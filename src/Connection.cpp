#include "../include/Connection.hpp"

Connection::Connection(Type type, const Socket& socket, const VirtualHost& vh)
    : type_(type)
    , socket_(socket)
    , config_(vh)
{
}

Connection::Type Connection::getType() const
{
    return type_;
}

const Socket& Connection::getSocket() const
{
    return socket_;
}

const VirtualHost& Connection::getConfig() const
{
    return config_;
}
const std::string& Connection::getInputBuffer() const
{
    return input_buffer_;
}
const std::string& Connection::getOutputBuffer() const
{
    return output_buffer_;
}
void Connection::setInputBuffer(const std::string& s)
{
    input_buffer_ = s;
}
void Connection::setOutputBuffer(const std::string& s)
{
    output_buffer_ = s;
}
