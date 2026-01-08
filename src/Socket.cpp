#include "../include/Socket.hpp"
#include <netdb.h>
#include <vector>

Socket::Socket(const Config config)
{
    bindToVirtualHosts(config);
}

Socket::~Socket()
{
    for (std::vector<std::pair<VirtualHostConfig, pollfd>>::iterator it = vh_config_.begin();
        it != vh_config_.end();
        ++it) {
        freeaddrinfo(it->first.addrinf);
        if (it->first.socket != -1 && close(it->first.socket) != 0)
            std::cerr << "[Error] closing sfd " << it->first.socket << ": " << std::strerror(errno) << std::endl;
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
    for (;;) {
        nfds_t nfds = vh_config_.size();
        std::vector<pollfd> arr_fds;
        for (size_t i = 0; i < nfds; ++i)
            arr_fds.push_back(vh_config_[i].second);
        int poll_count = poll(arr_fds.data(), nfds, -1);
        if (poll_count == -1)
            throw std::runtime_error(std::strerror(errno));

        // check if event/s are from a new connection (main socket from VH is hit)
        // or is an already opened one
        for (std::vector<std::pair<VirtualHostConfig, pollfd>>::const_iterator tmp_pair = vh_config_.begin();
            tmp_pair != vh_config_.end();
            ++tmp_pair) {
            if (tmp_pair->second.revents & POLLIN) {
                if (tmp_pair->first.is_vh_socket)
                    handleNewConn(tmp_pair->first);
                else
                    // TODO: from here
                    handleExistingConn(*tmp_pair);
            } else if (tmp_pair->second.revents & POLLHUP)
                handleClosedConn(tmp_pair->first.socket);
        }
        std::cout << "poll_count: " << poll_count << std::endl
                  << "pfds.size(): " << vh_config_.size() << std::endl;
    }
}

/* creates another slot (socket) for a new connection */
void Socket::handleNewConn(const VirtualHostConfig& vh)
{
    int cfd = accept(vh.socket, vh.curraddr->ai_addr, &vh.curraddr->ai_addrlen); // blocks
    if (cfd == -1)
        throw std::runtime_error(std::strerror(errno));
    vh_config_.push_back(
        { VirtualHostConfig {
              vh.vh,
              cfd,
              false,
              nullptr,
              nullptr },
            pollfd { cfd, POLLIN, 0 } });
}

void Socket::handleExistingConn(const std::pair<VirtualHostConfig, pollfd>& tmp_pair)
{
    char buf[tmp_pair.first.vh.socket_size + 1]; // +1 for null terminator if buffer is full
    int count = recv(tmp_pair.first.socket, buf, tmp_pair.first.vh.socket_size, 0);
    if (count == -1)
        throw std::runtime_error(std::strerror(errno));
    if (!count) // conn closed by client
        return handleClosedConn(tmp_pair.first.socket);
    buf[count] = '\0';
    std::cout << buf << std::endl;
    // TODO: implement complete send (not all the bytes may be send through the wire)
    if (send(tmp_pair.first.socket, "hi! i'm a web server :)\n", 24, 0) == -1)
        throw std::runtime_error(std::strerror(errno));
}

void Socket::handleClosedConn(int cfd)
{
    if (close(cfd) != 0) {
        std::cerr << "[Error] closing client fd " << cfd << ": "
                  << std::strerror(errno) << std::endl;
        throw std::runtime_error(std::strerror(errno));
    }
    for (std::vector<std::pair<VirtualHostConfig, pollfd>>::iterator it = vh_config_.begin();
        it != vh_config_.end();
        ++it) {
        if ((*it).first.socket == cfd) {
            vh_config_.erase(it);
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
    struct VirtualHostConfig vh_c { vh, -1, true, nullptr, nullptr };
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

    vh_config_.push_back({ vh_c, { vh_c.socket, POLLIN, 0 } });
#if DEBUG
    std::cout << "[Debug] success listen on sfd " << m_sfd << std::endl;
#endif
}
