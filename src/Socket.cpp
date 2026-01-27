#include "../include/Socket.hpp"
#include <netdb.h>
#include <sys/socket.h>
#include <vector>

Socket::Socket(const Config config)
{
    bindToVirtualHosts(config);
}

Socket::~Socket()
{
    for (std::vector<std::pair<VirtualHostConfig, pollfd>>::iterator it = vh_config.begin();
        it != vh_config.end();
        ++it) {
        freeaddrinfo(it->first.addrinf);
        if (it->first.socket != -1 && close(it->first.socket) != 0)
            std::cerr << "[Error] closing sfd " << it->first.socket << ": " << std::strerror(errno) << std::endl;
#if DEBUG
        else
            std::cout << "[Debug] success close sfd " << it->first.socket << std::endl;
#endif
    }
}

Socket::Socket(const Socket& cp)
    : vh_config(cp.vh_config)
{
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

void Socket::bindToVirtualHosts(const Config& conf)
{
    for (std::vector<VirtualHost>::const_iterator vh = conf.virtual_hosts.begin();
        vh != conf.virtual_hosts.end();
        vh++) {
        createBindListen(*vh);
    }
}

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

    vh_config.push_back({ vh_c, { vh_c.socket, POLLIN, 0 } });
#if DEBUG
    std::cout << "[Debug] success listen on sfd " << vh_c.socket << std::endl;
#endif
}
