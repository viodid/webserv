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
        const std::vector<pollfd>& pollfds = notifier.getPollFds();
        for (size_t i = 0; i < connections_.size(); i++) {
            Connection* c = notifier.getConnectionFor(pollfds[i].fd);
            if (pollfds[i].revents & POLLIN) {
#if DEBUG
                std::cout << "POLLIN - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleRead_(notifier, *c);
            }
            if (pollfds[i].revents & POLLOUT) {
#if DEBUG
                std::cout << "POLLOUT - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleWrite_(notifier, *c);
            }
            if (pollfds[i].revents & POLLERR || pollfds[i].revents & POLLHUP) {
#if DEBUG
                std::cout << "POLLERR - connection: " << i << " - fd: " << c->getFd() << '\n';
#endif
                handleClosedConn_(notifier, *c);
            }
        }
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

void Webserver::handleClosedConn_(EventManager& manager, const Connection& c)
{
    manager.removeFd(c.getFd());
#if DEBUG
    std::cout << "closed conn fd: " << c.getFd() << std::endl;
#endif
    for (size_t i = 0; i < connections_.size(); i++) {
        if (connections_[i] == &c) {
            delete connections_[i];
            connections_.erase(connections_.begin() + i);
            break;
        }
    }
}

void Webserver::handleRead_(EventManager& notifier, Connection& c)
{
    if (c.getType() == Connection::LISTENER) {
        handleNewClient_(notifier, c);
        return;
    }

    try {
        c.getRequest().parseFromReader(c);
        if (c.getRequest().isDone())
            notifier.enableWrite(c.getFd());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return handleClosedConn_(notifier, c);
    }
}

void Webserver::handleWrite_(EventManager& notifier, Connection& c)
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
            handleClosedConn_(notifier, c);
    } catch (const std::exception& e) {
        std::cerr << "handleWrite_: " << e.what() << '\n';
        handleClosedConn_(notifier, c);
    }
}
