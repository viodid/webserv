#include "../include/Connection.hpp"
#include <iostream>
#include <stdexcept>

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

const Socket& Connection::getSocket() const
{
    return *socket_;
}

const VirtualHost& Connection::getConfig() const
{
    return config_;
}

int Connection::read(char buffer[], int len)
{
    // TODO: rm
    if (len == 0)
        throw std::runtime_error("buffer len shouldn't be 0\n");
    int read_n = recv(socket_->getFd(), buffer, len, MSG_DONTWAIT);
    std::cout << "connection#read_n: " << read_n << "\n";
    if (read_n == -1) {
        std::cout << "read on an empty socket\n";
        return 0;
    }
    if (read_n == 0)
        throw ExceptionClientCloseConn("");
    return read_n;
}
