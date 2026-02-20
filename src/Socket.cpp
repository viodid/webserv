#include "../include/Socket.hpp"

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
        std::cerr << "[Error] closing sfd " << fd_ << ": " << std::strerror(errno) << std::endl;
#if DEBUG
    else
        std::cout << "[Debug] success close sfd " << fd_ << std::endl;
#endif
}

// TODO: C++ it
const char* inet_ntop2(void* addr, char* buf, size_t size)
{
    struct sockaddr_storage* sas = (sockaddr_storage*)addr;
    struct sockaddr_in* sa4;
    struct sockaddr_in6* sa6;
    void* src;

    switch (sas->ss_family) {
    case AF_INET:
        sa4 = (sockaddr_in*)addr;
        src = &(sa4->sin_addr);
        break;
    case AF_INET6:
        sa6 = (sockaddr_in6*)addr;
        src = &(sa6->sin6_addr);
        break;
    default:
        return NULL;
    }

    return inet_ntop(sas->ss_family, src, buf, size);
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

    for (curraddr_ = addrinf_; curraddr_ != nullptr; curraddr_ = curraddr_->ai_next)
        std::cout << inet_ntop2((void*)curraddr_->ai_addr, std::string().data(), curraddr_->ai_addrlen) << "\n";

    for (curraddr_ = addrinf_; curraddr_ != nullptr; curraddr_ = curraddr_->ai_next) {
        fd_ = socket(curraddr_->ai_family, curraddr_->ai_socktype, curraddr_->ai_protocol);
        if (fd_ == -1)
            continue;
        int yes = 1;
        if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            close(fd_);
            throw std::runtime_error(std::strerror(errno));
        }
        if (bind(fd_, curraddr_->ai_addr, curraddr_->ai_addrlen) == 0)
            break;
        close(fd_);
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
    std::cout << "[Debug] success listen on sfd " << fd_ << std::endl;
#endif
}
