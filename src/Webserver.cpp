#include "../include/Webserver.hpp"

Webserver::Webserver(const std::vector<VirtualHost>& config)
    : config_(config)
{
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
                handleWrite_(*c, dead_fds);
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
    manager.removeFd(fd);
    for (size_t i = 0; i < connections_.size(); i++) {
        if (connections_[i]->getFd() == fd) {
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

void Webserver::handleWrite_(Connection& c, std::set<int>& dead_fds)
{
    if (!c.getRequest().isDone())
        return;

    try {
        if (!c.hasResponse()) {
            ErrorRenderer error_renderer(c.getConfig().getStatusCodes());
            IRequestHandler* handler = createHandler(c.getRequest(), c.getConfig(), error_renderer);
            HttpResponse* response = handler->handle(c.getRequest());
            delete handler;
            c.setResponse(response);
        }

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
