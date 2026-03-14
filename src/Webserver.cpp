// TODO: primeagen 1:04:00
#include "../include/Webserver.hpp"
#include "../include/HttpRequestParser.hpp"

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

    ssize_t count = 0;
    do {
        count = recv(connection.getSocket().getFd(), buf.data(), READ_SOCKET_SIZE, 0);
        if (count > 0)
            data.insert(data.end(), buf.begin(), buf.begin() + count);
        else if (count == 0)
            return handleClosedConn_(notifier, connection);
        else if (count == -1)
            break;
    } while (count > 0);

    if (data.empty())
        return;

    std::string raw(data.begin(), data.end());
    HttpRequest req = HttpRequestParser::parseIncremental(
        raw, false, connection.getConfig().getSocketSize());

    if (req.state == PARSE_SUCCESS) {
        std::cout << "[Parser] Method:  " << req.method << "\n";
        std::cout << "[Parser] Path:    " << req.path   << "\n";
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
