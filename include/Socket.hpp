// socket -> bind -> listen -> accept
//
#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>

class Socket {
    int m_sfd;

    void create_bind_listen_(const std::string&);

public:
    Socket();
    Socket(const std::string&);
    ~Socket();
    Socket(const Socket&);
    Socket& operator=(const Socket&);
};
#endif
