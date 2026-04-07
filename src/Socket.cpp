#include "../include/Socket.hpp"

Socket::Socket(int fd)
    : fd_(fd)
    , hostname_("")
    , port_("")
    , addrinf_(NULL)
    , curraddr_(NULL)
{
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

ssize_t Socket::sendMsg(const std::string& msg) const
{
    ssize_t total_sent = 0;
    while (total_sent < static_cast<ssize_t>(msg.size())) {
        ssize_t send_n = send(fd_, msg.c_str(), msg.size(), MSG_NOSIGNAL);
        if (send_n == -1)
            throw ExceptionErrorConnectionSocket(std::strerror(errno));
        total_sent += send_n;
    }
    return total_sent;
}

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
    if (listen(fd_, 0) == -1) {
        close(fd_);
        freeaddrinfo(addrinf_);
        throw std::runtime_error(std::strerror(errno));
    }

#if DEBUG
    char buf[100];

    struct addrinfo* addr;
    for (addr = addrinf_; addr != NULL; addr = addr->ai_next) {
        std::cout << inet_ntop2((void*)addr->ai_addr, buf, addr->ai_addrlen)
                  << ":" << ((struct sockaddr_in*)addr->ai_addr)->sin_port << "\n";
    }
    std::cout << "[Debug] success listen on lfd " << fd_ << std::endl;
#endif
}
