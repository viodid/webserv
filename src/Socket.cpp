#include "../include/Socket.hpp"
#include <netdb.h>
#include <vector>

Socket::Socket(const Config config)
{
    bindToVirtualHosts(config);
}

Socket::~Socket()
{
    freeaddrinfo(addrinfo_);
    if (sfd_ != -1 && close(sfd_) != 0)
        std::cerr << "[Error] closing sfd " << sfd_ << ": " << std::strerror(errno) << std::endl;
#if DEBUG
    else
        std::cout << "[Debug] success close sfd " << m_sfd << std::endl;
#endif
}

Socket::Socket(const Socket& cp)
{
    sfd_ = cp.sfd_;
    addrinfo_ = cp.addrinfo_;
    curraddr = cp.curraddr;
}

Socket& Socket::operator=(const Socket& cp)
{
    if (this != &cp) {
        sfd_ = cp.sfd_;
        addrinfo_ = cp.addrinfo_;
        curraddr = cp.curraddr;
    }
    return *this;
}

void Socket::start()
{
    // TODO: initialize with all the virutal hosts sockets
    pollfd pfd { sfd_, POLLIN, 0 };
    // initialize with just the socket fd
    std::vector<pollfd> pfds { pfd };

    for (;;) {
        nfds_t nfds = pfds.size();
        int poll_count = poll(pfds.data(), nfds, -1);
        if (poll_count == -1)
            throw std::runtime_error(std::strerror(errno));

        // check if event/s are from a new connection (main socket fd is hit)
        // or is an already opened one
        for (int i = 0; i < static_cast<int>(nfds); i++) {
            pollfd tmp_pfd = pfds[i];
            if (tmp_pfd.revents & POLLIN) {
                if (tmp_pfd.fd == sfd_)
                    handleNewConn(pfds);
                else
                    handleExistingConn(tmp_pfd.fd, pfds);
            } else if (tmp_pfd.revents & POLLHUP)
                handleClosedConn(tmp_pfd.fd, pfds);
        }
        std::cout << "poll_count: " << poll_count << std::endl
                  << "pfds.size(): " << pfds.size() << std::endl;
    }
}

/* creates another slot (socket) for a new connection */
void Socket::handleNewConn(std::vector<pollfd>& pfds) const
{
    int cfd = accept(sfd_, curraddr->ai_addr, &curraddr->ai_addrlen); // blocks
    if (cfd == -1)
        throw std::runtime_error(std::strerror(errno));
    pfds.push_back(pollfd { cfd, POLLIN, 0 });
}

void Socket::handleExistingConn(int fd, std::vector<pollfd>& pfds) const
{
    char buf[SOCKET_MSG_BUFFER + 1]; // +1 for null terminator if buffer is full
    int count = recv(fd, buf, SOCKET_MSG_BUFFER, 0);
    if (count == -1)
        throw std::runtime_error(std::strerror(errno));
    if (!count) // conn closed by client
        return handleClosedConn(fd, pfds);
    buf[count] = '\0';
    std::cout << buf << std::endl;
    // TODO: implement complete send (not all the bytes may be send through the wire)
    if (send(fd, "hi! i'm a web server :)\n", 24, 0) == -1)
        throw std::runtime_error(std::strerror(errno));
}

void Socket::handleClosedConn(int cfd, std::vector<pollfd>& pfds) const
{
    if (close(cfd) != 0) {
        std::cerr << "[Error] closing client fd " << cfd << ": "
                  << std::strerror(errno) << std::endl;
        throw std::runtime_error(std::strerror(errno));
    }

    for (std::vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it) {
        if ((*it).fd == cfd) {
            pfds.erase(it);
            break;
        }
    }
    std::cout << "closed conn fd: " << cfd << std::endl;
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

// TODO: handle different virtual hosts (every VH should be a listening posix socket with an interface:port pair)
// this setup comes from a configuration file
//
// creates a socket and assigns it to member variable m_sfd

void Socket::bindToVirtualHosts(const Config& conf)
{
    for (std::vector<VirtualHost>::const_iterator vh = conf.virtual_hosts.begin();
        vh != conf.virtual_hosts.end();
        vh++) {
        createBindListen(*vh);
    }
}

// TODO: test refactor
void Socket::createBindListen(const VirtualHost& vh)
{
    struct VirtualHostConfig vh_config { vh, -1, nullptr, nullptr };
    struct addrinfo hints;
    struct addrinfo* curraddr = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(vh.hostname.data(), vh.port.data(), &hints, &vh.addrinfo) != 0)
        throw std::runtime_error(std::strerror(errno));

    for (curraddr = vh.addrinfo; curraddr != nullptr; curraddr = curraddr->ai_next)
        std::cout << inet_ntop2((void*)curraddr->ai_addr, std::string().data(), curraddr->ai_addrlen) << "\n";

    for (curraddr = vh.addrinfo; curraddr != nullptr; curraddr = curraddr->ai_next) {
        vh_config.socket = socket(curraddr->ai_family, curraddr->ai_socktype, curraddr->ai_protocol);
        if (vh_config.socket == -1)
            continue;
        int yes = 1;
        if (setsockopt(vh_config.socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            close(vh_config.socket);
            throw std::runtime_error(std::strerror(errno));
        }
        if (bind(vh_config.socket, curraddr->ai_addr, curraddr->ai_addrlen) == 0)
            break;
        close(vh_config.socket);
        throw std::runtime_error(std::strerror(errno));
    }
    if (curraddr == nullptr)
        throw std::runtime_error(std::strerror(errno));
    if (listen(vh_config.socket, 0) == -1) {
        close(vh_config.socket);
        freeaddrinfo(vh.addrinfo);
        throw std::runtime_error(std::strerror(errno));
    }

    vh_config_.push_back(vh_config);
#if DEBUG
    std::cout << "[Debug] success listen on sfd " << m_sfd << std::endl;
#endif
}
