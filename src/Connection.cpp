#include "../include/Connection.hpp"

Connection::Connection(Type type, Socket* socket, const VirtualHost& vh)
    : type_(type)
    , socket_(socket)
    , config_(vh)
{
}

Connection::~Connection()
{
    delete socket_;
#if DEBUG
    std::cout << "[Debug] Connection destructor called " << std::endl;
#endif
}

Connection::Type Connection::getType() const
{
    return type_;
}

int Connection::getFd() const
{
    return socket_->getFd();
}

int Connection::acceptNewConnection() const
{
    if (type_ != LISTENER)
        throw ExceptionErrorConnectionSocket("acceptNewConnection() called on non-listener socket");
    return socket_->acceptConn();
}

ssize_t Connection::sendMsg(const std::string& msg) const
{
    return socket_->sendMsg(msg);
}

const VirtualHost& Connection::getConfig() const
{
    return config_;
}

int Connection::read(char buffer[], int len)
{
    int read_n = recv(socket_->getFd(), buffer, len, 0);
    std::cout << "recv on socket: " << socket_->getFd() << " - read_n: " << read_n << "\n";
    if (read_n == -1)
        throw ExceptionErrorConnectionSocket(std::strerror(errno));
    if (read_n == 0)
        throw ExceptionClientCloseConn("");
    return read_n;
}
