#include "../include/Socket.hpp"
#include <netinet/tcp.h>

Socket::Socket(int fd)
    : fd_(fd)
    , hostname_("")
    , port_("")
    , addrinf_(NULL)
    , curraddr_(NULL)
{
    int yes = 1;
    if (setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1)
        std::cerr << "[Warn] TCP_NODELAY on fd " << fd_ << ": " << std::strerror(errno) << '\n';
}

Socket::Socket(const std::string& hostname, const std::string& port)
    : fd_(-1)
    , hostname_(hostname)
    , port_(port)
    , addrinf_(NULL)
    , curraddr_(NULL)
{
    bindAndListen_();
}

Socket::~Socket()
{
    if (addrinf_)
        freeaddrinfo(addrinf_);
    if (close(fd_) != 0)
        std::cerr << "[Error] closing lfd " << fd_ << ": " << std::strerror(errno) << std::endl;
#if DEBUG
    std::cout << "[Debug] destructor socket: success close fd " << fd_ << std::endl;
#endif
}

int Socket::getFd() const
{
    return fd_;
}

int Socket::acceptConn() const
{
    int cfd = accept(fd_, curraddr_->ai_addr, &curraddr_->ai_addrlen); // blocks
    if (cfd == -1)
        throw std::runtime_error(std::strerror(errno));
    return cfd;
}

size_t Socket::sendBytes(const std::vector<char>& bytes) const
{
    if (bytes.empty())
        return 0;
    ssize_t n = send(fd_, bytes.data(), bytes.size(), MSG_NOSIGNAL);
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        throw ExceptionErrorConnectionSocket(std::strerror(errno));
    }
    return static_cast<size_t>(n);
}

void Socket::bindAndListen_()
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(hostname_.data(), port_.data(), &hints, &addrinf_);
    if (ret != 0)
        throw std::runtime_error("getaddrinfo error\n");

    for (curraddr_ = addrinf_; curraddr_ != NULL; curraddr_ = curraddr_->ai_next) {
        fd_ = socket(curraddr_->ai_family, curraddr_->ai_socktype, curraddr_->ai_protocol);
        if (fd_ == -1)
            continue;
        int yes = 1;
        if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            close(fd_);
            freeaddrinfo(addrinf_);
            throw std::runtime_error(std::strerror(errno));
        }
        if (bind(fd_, curraddr_->ai_addr, curraddr_->ai_addrlen) == 0)
            break;
        close(fd_);
        freeaddrinfo(addrinf_);
        throw std::runtime_error(std::strerror(errno));
    }
    if (curraddr_ == NULL)
        throw std::runtime_error(std::strerror(errno));
    if (listen(fd_, SOMAXCONN) == -1) {
        close(fd_);
        freeaddrinfo(addrinf_);
        throw std::runtime_error(std::strerror(errno));
    }

#if DEBUG
    std::cout << "[Debug] success listen on lfd " << fd_ << std::endl;
#endif
}
