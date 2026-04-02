#include "../include/Connection.hpp"

Connection::Connection(Type type, Socket* socket, const VirtualHost& vh)
    : type_(type)
    , socket_(socket)
    , config_(vh)
{
    buffer_.reserve(Settings::CONNECTION_BUFFER_SIZE);
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

int Connection::read(char buffer[], int len)
{
    int read_n = recv(socket_->getFd(), buffer, len, MSG_DONTWAIT);
    if (read_n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            throw std::runtime_error(std::strerror(errno));
    }
    // if (read_n == 0)
    //     throw ExceptionClientCloseConn("");
    return read_n;
}
