// TODO: primeagen 43:59
#include "../include/Webserver.hpp"
#include <sys/socket.h>

Webserver::Webserver(const std::vector<VirtualHost>& config)
    : config_(config)
{
}

void Webserver::init()
{
    for (std::vector<VirtualHost>::const_iterator it = config_.begin();
        it != config_.end(); it++) {
        Socket socket(it->hostname, it->port);
        Connection conn = { Connection::LISTENER, socket, *it, "", "" };
        connections_.push_back(conn);
    }
}

void Webserver::run()
{
    EventManager notifier(connections_);
    notifier.manage();
    const std::vector<pollfd>& pollfds = notifier.getPollFds();
    for (size_t i = 0; i < connections_.size(); i++) {
        if (pollfds[i].revents & POLLIN) {
            if (connections_[i].type == Connection::CLIENT)
                handleNewConnection_(notifier, connections_[i]);
            else
                handleClientData_(notifier, connections_[i]);
        }
    }
}

void Webserver::handleNewConnection_(EventManager& notifier, const Connection& connection)
{
    int cfd = connection.socket.acceptConn();
    notifier.addPollFds(cfd);
    Connection newConn = { Connection::CLIENT, Socket(cfd), connection.config, "", "" };
    connections_.push_back(newConn);
}

void Webserver::handleClosedConn_(EventManager& manager, const Connection& connection)
{
    manager.removePollFds(connection.socket.getFd());
    std::cout << "closed conn fd: " << connection.socket.getFd() << std::endl;
    for (size_t i = 0; i < connections_.size(); i++) {
        if (&connections_[i] == &connection) {
            std::cout << "connection erased from connections_\n";
            connections_.erase(connections_.begin() + i);
            break;
        }
    }
}

void Webserver::handleClientData_(EventManager& notifier, const Connection& connection)
{
    std::vector<char> buf(READ_SOCKET_SIZE);
    std::vector<char> data;
    data.reserve(READ_SOCKET_SIZE);
    int count = 0;
    while ((count = recv(connection.socket.getFd(), buf.data(), READ_SOCKET_SIZE, MSG_DONTWAIT))) {
        std::cout << "count = " << count << " - errno: " << errno << std::endl;
        if (count == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
                throw std::runtime_error(std::strerror(errno));
        }
        data.insert(data.end(), buf.begin(), buf.end());
    }
    if (count == 0) // conn closed by client
        return handleClosedConn_(notifier, connection);
    std::cout << data.data();
    std::cout << "here\n";
}
