// TODO: primeagen 1:04:00
#include "../include/Webserver.hpp"
#include "../include/HttpRequestParser.hpp"
#include <fcntl.h>

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
    for (std::vector<VirtualHost>::const_iterator it = config_.begin(); it != config_.end(); it++) {
        Socket* socket_ptr = new Socket(it->getHostname(), it->getPort());
        int flags = fcntl(socket_ptr->getFd(), F_GETFL, 0);
        if (flags != -1)
            fcntl(socket_ptr->getFd(), F_SETFL, flags | O_NONBLOCK);

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

    int flags = fcntl(cfd, F_GETFL, 0);
    if (flags != -1)
        fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

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
            delete connections_[i];
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

    int fd = connection.getSocket().getFd();
    ssize_t count = recv(fd, buf.data(), READ_SOCKET_SIZE, 0);

    if (count > 0) {
        data.insert(data.end(), buf.begin(), buf.begin() + count);
    } else {
        return handleClosedConn_(notifier, connection);
    }

    if (data.empty())
        return;

    std::string raw(data.begin(), data.end());
    HttpRequest req = HttpRequestParser::parseIncremental(
        raw, false, connection.getConfig().getSocketSize());

    if (req.state == PARSE_SUCCESS) {
        std::cout << "[Parser] Method:  " << req.method << "\n";
        std::cout << "[Parser] Path:    " << req.path << "\n";
        std::cout << "[Parser] Version: " << req.version << "\n";
        if (!req.query_string.empty())
            std::cout << "[Parser] Query:   " << req.query_string << "\n";
        if (!req.body.empty())
            std::cout << "[Parser] Body (" << req.body.size() << " bytes)\n";
    } else {
        std::cout << "[Parser] state=" << req.state
                  << " msg=" << req.error_msg << "\n";
    }
}