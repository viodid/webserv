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

const VirtualHost& Connection::getConfig() const
{
    return config_;
}

void Connection::setResponse(HttpResponse* response)
{
    delete response_;
    response_ = response;
    if (response_ != NULL)
        appendToBuffer_(response_->getBytesHeader());
}

bool Connection::hasResponse() const
{
    return response_ != NULL;
}

void Connection::pullBodyChunk()
{
    if (response_ == NULL || !response_->bodyHasMore())
        return;
    appendToBuffer_(response_->nextBodyChunk());
}

size_t Connection::writeBufferSize() const
{
    return out_buf_.size();
}

bool Connection::isWriteDone() const
{
    if (!out_buf_.empty())
        return false;
    if (response_ == NULL)
        return false;
    return !response_->bodyHasMore();
}

size_t Connection::sendBytes()
{
    size_t n = socket_->sendBytes(out_buf_);
    if (n > 0)
        out_buf_.erase(out_buf_.begin(), out_buf_.begin() + n);
    return n;
}

int Connection::read(char buffer[], int len)
{
    int read_n = recv(socket_->getFd(), buffer, len, 0);
    if (read_n == -1)
        throw ExceptionErrorConnectionSocket(std::strerror(errno));
    if (read_n == 0)
        throw ExceptionClientCloseConn("client connection closed");
    return read_n;
}

void Connection::appendToBuffer_(const std::string& s)
{
    out_buf_.insert(out_buf_.end(), s.begin(), s.end());
}
