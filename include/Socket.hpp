// socket -> bind -> listen -> accept
//
#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <cerrno>
#include <string>
#include <sstream>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

// TODO: document
#define SOCKET_PORT     "5555"
#define SOCKET_BACKLOG  4096

/**
 * @class Socket
 * @brief A simple RAII wrapper for a network socket.
 * 
 * This class encapsulates a low-level socket file descriptor and manages its
 * lifetime. When a Socket object is created, it opens a socket. When it is
 * destroyed (goes out of scope), it automatically closes the socket.
 * 
 * @see https://man7.org/linux/man-pages/man2/socket.2.html
 */
class Socket {
    int m_sfd, m_cfd = -1;
    struct addrinfo *m_addrinfo, *m_curraddr = nullptr;

    void create_bind_listen_(const std::string&);

public:
    /**
     * @brief Default initializes a Socket object and creates a POSIX socket ready to connect
     * to the default value **127.0.0.1**.
     */
    Socket();
    /**
     * @brief Constructs a Socket object and creates a POSIX socket ready to connect
     *
     * @param addr The addres to listen to.
     */
    Socket(const std::string& addr);
    /**
     * @brief Destroys the Socket object, and frees its resources.
     *
     * Deallocates the dynamic memory assigned to m_addrinfo and closes
     * the sockets file descriptors in a safe manner.
     */
    ~Socket();
    Socket(const Socket&);
    Socket& operator=(const Socket&);

    // TODO: document
    void start();
};
#endif
