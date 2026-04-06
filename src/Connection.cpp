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

int Connection::getFd() const
{
    return socket_->getFd();
}

int Connection::acceptNewConnection() const
{
    if (type_ != LISTENER)
        throw std::runtime_error("somehow I've programed it poorly");
    return socket_->acceptConn();
}

int Connection::sendMsg(const std::string& msg)
{
    return socket_->sendMsg(msg);
}

const VirtualHost& Connection::getConfig() const
{
    return config_;
}

int Connection::read(char buffer[], int len)
{
    int read_n = recv(socket_->getFd(), buffer, len, MSG_DONTWAIT);
    if (read_n == -1)
        return 0;
    if (read_n == 0)
        throw ExceptionClientCloseConn("");
    return read_n;
}
