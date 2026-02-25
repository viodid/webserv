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
                handleNewConnection(notifier, connections_[i]);
            else
                handleClientData(notifier, connections_[i]);
        }
    }
}

void Webserver::handleNewConnection(EventManager& notifier, const Connection& connection)
{
    int cfd = connection.socket.acceptConn();
    notifier.addPollFds(cfd);
    Connection newConn = { Connection::CLIENT, Socket(cfd), connection.config, "", "" };
    connections_.push_back(newConn);
}
