#include "../include/Socket.hpp"
#include <netdb.h>
#include <vector>

Socket::Socket(const Config config)
{
    bindToVirtualHosts(config);
}

Socket::~Socket()
{
    for (std::vector<VirtualHostConfig>::iterator it = vh_config_.begin();
        it != vh_config_.end();
        ++it) {
        freeaddrinfo(it->addrinf);
        if (it->socket != -1 && close(it->socket) != 0)
            std::cerr << "[Error] closing sfd " << it->socket << ": " << std::strerror(errno) << std::endl;
    }

#if DEBUG
    else std::cout << "[Debug] success close sfd " << m_sfd << std::endl;
#endif
}

Socket::Socket(const Socket& cp) { vh_config_ = cp.vh_config_; }

Socket& Socket::operator=(const Socket& cp)
{
    if (this != &cp)
        vh_config_ = cp.vh_config_;
    return *this;
}

void Socket::start()
{
    std::vector<pollfd> pfds;
    for (std::vector<VirtualHostConfig>::const_iterator it = vh_config_.begin();
        it != vh_config_.end();
        ++it) {
        if (it->socket != -1) {
            pfds.push_back(pollfd { it->socket, POLLIN, 0 });
        }
    }

    for (;;) {
        nfds_t nfds = pfds.size();
        int poll_count = poll(pfds.data(), nfds, -1);
        if (poll_count == -1)
            throw std::runtime_error(std::strerror(errno));

        // check if event/s are from a new connection (main socket from VH is hit)
        // or is an already opened one
        for (int i = 0; i < static_cast<int>(nfds); i++) {
            pollfd tmp_pfd = pfds[i];
            if (tmp_pfd.revents & POLLIN) {
                bool is_new_conn = false;
                for (std::vector<VirtualHostConfig>::const_iterator it = vh_config_.begin();
                    it != vh_config_.end();
                    ++it) {
                    if (tmp_pfd.fd == it->socket) {
                        handleNewConn(pfds, *it);
                        is_new_conn = true;
                        break;
                    }
                }
                if (!is_new_conn)
                    handleExistingConn(tmp_pfd.fd, pfds);
            } else if (tmp_pfd.revents & POLLHUP)
                handleClosedConn(tmp_pfd.fd, pfds);
        }
        std::cout << "poll_count: " << poll_count << std::endl
                  << "pfds.size(): " << pfds.size() << std::endl;
    }
}

/* creates another slot (socket) for a new connection */
void Socket::handleNewConn(std::vector<pollfd>& pfds, const VirtualHostConfig& vh_c) const
{
    int cfd = accept(vh_c.socket, vh_c.curraddr->ai_addr, &vh_c.curraddr->ai_addrlen); // blocks
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
    struct VirtualHostConfig vh_c { vh, -1, nullptr, nullptr };
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(vh.hostname.data(), vh.port.data(), &hints, &vh_c.addrinf) != 0)
        throw std::runtime_error(std::strerror(errno));

    for (vh_c.curraddr = vh_c.addrinf; vh_c.curraddr != nullptr; vh_c.curraddr = vh_c.curraddr->ai_next)
        std::cout << inet_ntop2((void*)vh_c.curraddr->ai_addr, std::string().data(), vh_c.curraddr->ai_addrlen) << "\n";

    for (vh_c.curraddr = vh_c.addrinf; vh_c.curraddr != nullptr; vh_c.curraddr = vh_c.curraddr->ai_next) {
        vh_c.socket = socket(vh_c.curraddr->ai_family, vh_c.curraddr->ai_socktype, vh_c.curraddr->ai_protocol);
        if (vh_c.socket == -1)
            continue;
        int yes = 1;
        if (setsockopt(vh_c.socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            close(vh_c.socket);
            throw std::runtime_error(std::strerror(errno));
        }
        if (bind(vh_c.socket, vh_c.curraddr->ai_addr, vh_c.curraddr->ai_addrlen) == 0)
            break;
        close(vh_c.socket);
        throw std::runtime_error(std::strerror(errno));
    }
    if (vh_c.curraddr == nullptr)
        throw std::runtime_error(std::strerror(errno));
    if (listen(vh_c.socket, 0) == -1) {
        close(vh_c.socket);
        freeaddrinfo(vh_c.addrinf);
        throw std::runtime_error(std::strerror(errno));
    }

    vh_config_.push_back(vh_c);
#if DEBUG
    std::cout << "[Debug] success listen on sfd " << m_sfd << std::endl;
#endif
}
