// socket -> bind -> listen -> accept
//
#ifndef SOCKET_HPP
#define SOCKET_HPP
#include "Config.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

// TODO: document
#define SOCKET_PORT "5555"
#define SOCKET_BACKLOG 4096
#define SOCKET_MSG_BUFFER 100000 // 100 KB

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
public:
    /**
     * @brief Default initializes a Socket object and creates a POSIX socket ready to connect
     * to the default value **127.0.0.1**.
     */
    Socket(const Config config);
    /**
     * @brief Constructs a Socket object and creates a POSIX socket ready to connect
     *
     * @param addr The addres to listen to.
     */
    // Socket(const std::string& addr);
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

private:
    const Config config_;
    int sfd_ = -1;
    struct addrinfo *addrinfo_, *curraddr_ = nullptr;

    void createBindListen();
    void handleNewConn(std::vector<pollfd>& pfds) const;
    void handleExistingConn(int fd, std::vector<pollfd>& pfds) const;
    void handleClosedConn(int cfd, std::vector<pollfd>& pfds) const;
};
#endif
