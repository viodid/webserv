#include "../include/Socket.hpp"
#include <sys/socket.h>

Socket::Socket(const std::string& hostname, const std::string& port)
    : hostname_(hostname)
    , port_(port)
{
    bindAndListen_();
}

Socket::~Socket()
{
    freeaddrinfo(addrinf_);
    if (close(fd_) != 0)
        std::cerr << "[Error] closing lfd " << fd_ << ": " << std::strerror(errno) << std::endl;
#if DEBUG
    else
        std::cout << "[Debug] success close lfd " << fd_ << std::endl;
#endif
}

int Socket::acceptConn() const
{
    int cfd = accept(fd_, curraddr_->ai_addr, &curraddr_->ai_addrlen); // blocks
    if (cfd == -1)
        throw std::runtime_error(std::strerror(errno));
    return cfd;
}

void Socket::bindAndListen_()
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(hostname_.data(), port_.data(), &hints, &addrinf_) != 0)
        throw std::runtime_error(std::strerror(errno));

    for (curraddr_ = addrinf_; curraddr_ != nullptr; curraddr_ = curraddr_->ai_next) {
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
    if (curraddr_ == nullptr)
        throw std::runtime_error(std::strerror(errno));
    if (listen(fd_, 0) == -1) {
        close(fd_);
        freeaddrinfo(addrinf_);
        throw std::runtime_error(std::strerror(errno));
    }

#if DEBUG
    std::cout << "[Debug] success listen on lfd " << fd_ << std::endl;
#endif
}
