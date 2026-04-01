// TODO: primeagen 1:04:00
#include "../include/Webserver.hpp"
#include <map>

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
#if DEBUG
            std::cout << "Connection no: " << i + 1 << "\n";
#endif
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

void Webserver::handleClientData_(EventManager& notifier, Connection& connection)
{
    HttpRequestParser parser(connection);
    try {
        parser.parseFromReader();
        HttpRequest request = parser.getRequest();
        std::cout << "Request line:\n"
                  << "- Method: " << request.request_line.method << "\n"
                  << "- Target: " << request.request_line.request_target << "\n"
                  << "- Version: " << request.request_line.http_version << "\n"
                  << "Field line:\n";
        for (std::map<std::string, std::string>::const_iterator it = request.field_lines.begin();
            it != request.field_lines.end();
            it++)
            std::cout << it->first << ": " << it->second << "\n";
    } catch (ExceptionClientCloseConn& e) {
        return handleClosedConn_(notifier, connection);
    }
}
