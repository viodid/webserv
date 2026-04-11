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
        requests_.push_back(NULL);
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
                    handleClientData_(notifier, *connections_[i], *requests_[i]);
                break;
            }
        }
    }
}

void Webserver::handleNewConnection_(EventManager& notifier, const Connection& connection)
{
    int cfd = connection.acceptNewConnection();
    notifier.addFd(cfd);
    Socket* socket_ptr = new Socket(cfd);
    Connection* connection_ptr = new Connection(Connection::CLIENT, socket_ptr, connection.getConfig());
    connections_.push_back(connection_ptr);
    requests_.push_back(new HttpRequest);
}

void Webserver::handleClosedConn_(EventManager& manager, const Connection& connection)
{
    manager.removeFd(connection.getFd());
    std::cout << "closed conn fd: " << connection.getFd() << std::endl;
    for (size_t i = 0; i < connections_.size(); i++) {
        if (connections_[i] == &connection) {
            delete connections_[i];
            connections_.erase(connections_.begin() + i);
            delete requests_[i];
            requests_.erase(requests_.begin() + i);
#if DEBUG
            std::cout << "connection erased from connections_: " << i + 1 << "\n";
            std::cout << "request erased from requests_: " << i + 1 << "\n";
#endif
            break;
        }
    }
}

static void print_field_lines(const std::string& fn, const std::string& fv)
{
    std::cout << fn << ": " << fv << "\n";
}

void Webserver::handleClientData_(EventManager& notifier, Connection& connection, HttpRequest& request)
{
    try {
        request.parseFromReader(connection);
#if DEBUG
        std::cout << "Request line:\n"
                  << "- Method: " << request.getRequestLine().getMethod() << "\n"
                  << "- Target: " << request.getRequestLine().getRequestTarget() << "\n"
                  << "- Version: " << request.getRequestLine().getHttpVersion() << "\n"
                  << "Field line:\n";
        request.getFieldLines().forEach(&print_field_lines);
        std::cout << "Body:\n"
                  << request.getBody().get() << "\n";

#endif

        // TODO: catch malformed requests and return appropriate response
    } catch (ExceptionClientCloseConn& e) {
        std::cerr << e.what();
        return handleClosedConn_(notifier, connection);
    } catch (ExceptionErrorConnectionSocket& e) {
        std::cerr << e.what();
        return handleClosedConn_(notifier, connection);
    }

    if (request.isDone()) {
        // connection.sendMsg("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\nConnection: keep-alive\r\n\r\nHello World!");
        /*
         * 1. Event handler factory:
         * Based on the requests, returns the corresponding handler (Static, CGI...)
         *  - The request handler implements the interface (IRequestHandler)
         */
    }
}
