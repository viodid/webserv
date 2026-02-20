// socket -> bind -> listen -> accept
//
#pragma once
#include "Config.hpp"
#include "Utils.hpp"
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
#include <utility>
#include <vector>

#define SOCKET_BACKLOG 4096
#define READ_SOCKET_SIZE 1 << 24 // 16MiB

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
     * to the given hostname and port.
     *
     * @param hostname The hostname or address to bind the socket to
     * @param port The port to listen to
     */
    Socket(const std::string& hostname, const std::string& port);
    /**
     * @brief Destroys the Socket object, and frees its resources.
     *
     * Deallocates the dynamic memory assigned to addrinfo and closes
     * the socket file descriptor in a safe manner.
     */
    ~Socket();
    Socket(const Socket&) = delete;
    // assignment operator is implicitly deleted bc const member variables
    /**
     * @brief Opens a new socket connection. This call will block if no connection
     * is present in the listener socket.
     */
    int accept() const;

private:
    int fd_;
    const std::string hostname_;
    const std::string port_;
    struct addrinfo* addrinf_;
    struct addrinfo* curraddr_;

    void Socket::bindAndListen_();
};
