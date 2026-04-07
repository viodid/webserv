#pragma once
#include "Exceptions.hpp"
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

/**
 * @class Socket
 * @brief A simple RAII wrapper for a TCP socket.
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
     * @brief Initializes a Socket object with an already created socket FD (client)
     *
     * @param fd The socket FD.
     */
    Socket(int fd);
    /**
     * @brief Initializes a Socket object and creates a POSIX socket ready to connect
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
    // assignment operator is implicitly deleted bc const member variables
    /**
     * @brief Opens a new socket connection. This call will block if no connection
     * is present in the listener socket.
     */
    int getFd() const;
    int acceptConn() const;
    ssize_t sendMsg(const std::string& msg) const;

private:
    int fd_;
    std::string hostname_;
    std::string port_;
    struct addrinfo* addrinf_; // head of the linked list
    struct addrinfo* curraddr_;

    void bindAndListen_();
};
