#include "../include/Connection.hpp"

Connection::Connection(Type type, Socket* socket, const VirtualHost& vh)
    : config_(vh)
    , type_(type)
    , socket_(socket)
    , response_(NULL)
{
}

Connection::~Connection()
{
    delete socket_;
    delete response_;
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

HttpRequest& Connection::getRequest()
{
    return request_;
}

int Connection::acceptNewConnection() const
{
    if (type_ != LISTENER)
        throw ExceptionErrorConnectionSocket("acceptNewConnection() called on non-listener socket");
    return socket_->acceptConn();
}

size_t Connection::sendBytes() const
{
    return socket_->sendBytes(out_buf_);
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
        throw ExceptionClientCloseConn("client connection closed");
    return read_n;
}
