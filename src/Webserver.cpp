#include "../include/Webserver.hpp"
#include "../include/CgiProcess.hpp"
#include "../include/Handlers/handler_utils.hpp"
#include <csignal>

Webserver::Webserver(const std::vector<VirtualHost>& config)
    : config_(config)
{
    // Ignore SIGPIPE: writes to closed pipes (e.g. CGI child exiting early)
    // would otherwise terminate the server. send() already uses MSG_NOSIGNAL.
    std::signal(SIGPIPE, SIG_IGN);
}

Webserver::~Webserver()
{
    for (size_t i = 0; i < connections_.size(); i++)
        delete connections_[i];
}

void Webserver::init()
{
    for (std::vector<VirtualHost>::const_iterator it = config_.begin();
        it != config_.end(); it++) {
        Socket* socket_ptr = new Socket(it->getHostname(), it->getPort());
        Connection* connection_ptr = new Connection(Connection::LISTENER, socket_ptr, *it);
        connections_.push_back(connection_ptr);
    }
}

void Webserver::run()
{
    EventManager notifier(connections_);
    for (;;) {
        notifier.manage();
        std::vector<pollfd> pollfds = notifier.getPollFds();
        std::set<int> dead_fds;

        for (size_t i = 0; i < pollfds.size(); i++) {
            int fd = pollfds[i].fd;
            if (dead_fds.count(fd))
                continue;

            // CGI pipe fds are not connections.
            if (notifier.getCgiFor(fd) != NULL) {
                if (pollfds[i].revents & (POLLIN | POLLHUP))
                    handleCgiReadable_(notifier, fd);
                continue;
            }

            Connection* c = notifier.getConnectionFor(fd);
            if (!c)
                continue;

            if (pollfds[i].revents & POLLIN) {
#if DEBUG
                std::cout << "POLLIN - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleRead_(notifier, *c, dead_fds);
                if (dead_fds.count(fd))
                    continue;
            }
            if (pollfds[i].revents & POLLOUT) {
#if DEBUG
                std::cout << "POLLOUT - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleWrite_(notifier, *c, dead_fds);
                if (dead_fds.count(fd))
                    continue;
            }
            if (pollfds[i].revents & (POLLERR | POLLHUP)) {
#if DEBUG
                std::cout << "POLLERR - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                dead_fds.insert(fd);
            }
        }

        sweepCgiTimeouts_(notifier);

        for (std::set<int>::const_iterator it = dead_fds.begin();
            it != dead_fds.end(); ++it)
            destroyConnection_(notifier, *it);
    }
}

void Webserver::handleNewClient_(EventManager& notifier, const Connection& c)
{
    int cfd = c.acceptNewConnection();
    notifier.addFd(cfd);
    Socket* socket_ptr = new Socket(cfd);
    Connection* connection_ptr = new Connection(Connection::CLIENT, socket_ptr, c.getConfig());
    connections_.push_back(connection_ptr);
}

void Webserver::markClosed_(std::set<int>& dead_fds, const Connection& c)
{
#if DEBUG
    std::cout << "marking conn closed fd: " << c.getFd() << std::endl;
#endif
    dead_fds.insert(c.getFd());
}

void Webserver::destroyConnection_(EventManager& manager, int fd)
{
    for (size_t i = 0; i < connections_.size(); i++) {
        if (connections_[i]->getFd() == fd) {
            // Remove any CGI pipe fd this connection still holds before
            // tearing the connection down (which deletes the CgiProcess).
            CgiProcess* cgi = connections_[i]->getCgi();
            if (cgi != NULL && cgi->getStdoutFd() != -1) {
                try {
                    manager.removeCgiFd(cgi->getStdoutFd());
                } catch (const std::exception& e) {
                    std::cerr << "destroyConnection_: removeCgiFd: " << e.what() << '\n';
                }
            }
            try {
                manager.removeFd(fd);
            } catch (const std::exception& e) {
                std::cerr << "destroyConnection_: removeFd: " << e.what() << '\n';
            }
            delete connections_[i];
            connections_.erase(connections_.begin() + i);
            return;
        }
    }
}

void Webserver::handleRead_(EventManager& notifier, Connection& c, std::set<int>& dead_fds)
{
    if (c.getType() == Connection::LISTENER) {
        handleNewClient_(notifier, c);
        return;
    }

    try {
        c.getRequest().parseFromReader(c, c.getConfig().getLocations());
        if (c.getRequest().isDone())
            notifier.enableWrite(c.getFd());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        markClosed_(dead_fds, c);
    }
}

void Webserver::handleWrite_(EventManager& notifier, Connection& c, std::set<int>& dead_fds)
{
    if (!c.getRequest().isDone())
        return;

    try {
        if (!c.hasResponse() && c.getCgi() == NULL) {
            ErrorRenderer error_renderer(c.getConfig().getStatusCodes());
            IRequestHandler* handler = createHandler(c.getRequest(), c.getConfig(), error_renderer, &c);
            HttpResponse* response = handler->handle(c.getRequest());
            delete handler;
            if (response == NULL) {
                // Async path: a CgiProcess was attached to the connection.
                // Stop polling for write until the CGI completes.
                CgiProcess* cgi = c.getCgi();
                if (cgi == NULL) {
                    std::cerr << "handler returned NULL without attaching CGI\n";
                    markClosed_(dead_fds, c);
                    return;
                }
                notifier.addCgiFd(cgi->getStdoutFd(), cgi);
                notifier.disableWrite(c.getFd());
                return;
            }
            c.setResponse(response);
        }

        if (c.getCgi() != NULL)
            return; // CGI still in flight; nothing to write yet.

        if (c.writeBufferSize() == 0)
            c.pullBodyChunk();

        c.sendBytes();

        if (c.isWriteDone())
            markClosed_(dead_fds, c);
    } catch (const std::exception& e) {
        std::cerr << "handleWrite_: " << e.what() << '\n';
        markClosed_(dead_fds, c);
    }
}

void Webserver::handleCgiReadable_(EventManager& notifier, int fd)
{
    CgiProcess* cgi = notifier.getCgiFor(fd);
    if (cgi == NULL)
        return;
    cgi->onReadable();
    if (cgi->isTerminal())
        finalizeCgi_(notifier, *cgi->getOwner());
}

void Webserver::finalizeCgi_(EventManager& notifier, Connection& c)
{
    CgiProcess* cgi = c.getCgi();
    if (cgi == NULL)
        return;

    int cgi_fd = cgi->getStdoutFd();
    HttpResponse* response = NULL;
    try {
        ErrorRenderer error_renderer(c.getConfig().getStatusCodes());
        response = cgi->buildResponse(c.getRequest(), error_renderer);
    } catch (const std::exception& e) {
        std::cerr << "finalizeCgi_: buildResponse threw: " << e.what() << '\n';
    }

    if (cgi_fd != -1) {
        try {
            notifier.removeCgiFd(cgi_fd);
        } catch (const std::exception& e) {
            std::cerr << "finalizeCgi_: removeCgiFd: " << e.what() << '\n';
        }
    }

    c.clearCgi();

    if (response == NULL) {
        // Builder failed completely: synthesize a 500 so we never hang the client.
        ErrorRenderer error_renderer(c.getConfig().getStatusCodes());
        response = constructHttpErrorResponse(c.getRequest(), error_renderer, Location::S_500);
    }
    c.setResponse(response);
    notifier.enableWrite(c.getFd());
}

void Webserver::sweepCgiTimeouts_(EventManager& notifier)
{
    unsigned long now = nowMs();
    for (size_t i = 0; i < connections_.size(); ++i) {
        Connection* c = connections_[i];
        CgiProcess* cgi = c->getCgi();
        if (cgi == NULL)
            continue;
        if (cgi->checkTimeout(now))
            finalizeCgi_(notifier, *c);
    }
}
