// TODO: primeagen 54:00
#include "../include/Webserver.hpp"

Webserver::Webserver(const std::vector<VirtualHost>& config)
    : config_(config)
{
}

Webserver::~Webserver()
{
    for (size_t i = 0; i < connections_.size(); i++) 
        delete connections_[i];
#if DEBUG
    std::cout << "[Debug] Webserver destructor called " << std::endl;
#endif
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
            if (pollfds[i].revents & POLLIN) {
                if (connections_[i]->getType() == Connection::LISTENER)
                    handleNewConnection_(notifier, *connections_[i]);
                else
                    handleClientData_(notifier, *connections_[i]);
            }
        }
    }
}

void Webserver::handleNewConnection_(EventManager& notifier, const Connection& connection)
{
    int cfd = connection.getSocket().acceptConn();
    notifier.addPollFds(cfd);
    Socket* socket_ptr = new Socket(cfd);
    Connection* connection_ptr = new Connection(Connection::CLIENT, socket_ptr, connection.getConfig());
    connections_.push_back(connection_ptr);
}

void Webserver::handleClosedConn_(EventManager& manager, const Connection& connection)
{
    manager.removePollFds(connection.getSocket().getFd());
    std::cout << "closed conn fd: " << connection.getSocket().getFd() << std::endl;
    for (size_t i = 0; i < connections_.size(); i++) {
        if (connections_[i] == &connection) {
            std::cout << "connection erased from connections_: " << i << "\n";
            delete connections_[i];
            connections_.erase(connections_.begin() + i);
            break;
        }
    }
}

void Webserver::handleClientData_(EventManager& notifier, const Connection& connection)
{
    std::cout << "------------\n";
    std::vector<char> buf(READ_SOCKET_SIZE);
    std::vector<char> data;
    data.reserve(READ_SOCKET_SIZE);
    int count = 0;
    while ((count = recv(connection.getSocket().getFd(), buf.data(), READ_SOCKET_SIZE, MSG_DONTWAIT))) {
        std::cout << "count = " << count << " - errno: " << errno << std::endl;
        if (count == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
                throw std::runtime_error(std::strerror(errno));
        }
        data.insert(data.end(), buf.begin(), buf.end());
    }
    std::cout << "count = " << count << " - errno: " << errno << std::endl;
    if (count == 0) // conn closed by client
        return handleClosedConn_(notifier, connection);
    std::cout << data.data() << "\n";
}
