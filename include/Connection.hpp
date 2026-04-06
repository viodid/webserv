#pragma once
#include "Config.hpp"
#include "Exceptions.hpp"
#include "IReader.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include <vector>

/**
 * @class Connection
 * @brief State holder of a socket.
 *
 * This class holds the session state for a given TCP socket (client or
 * listener). Ties the VitualHost configuration to a socket and holds I/O
 * buffers.
 */
class Connection : public IReader {
public:
    enum Type {
        LISTENER,
        CLIENT
    };
    Connection(Type, Socket*, const VirtualHost&);
    ~Connection();

    Type getType() const;
    /*
     * Gets the FD from the connection socket
     */
    int getFd() const;
    /*
     * Accepts a new connection a returns its FD (only for LISTENER connections)
     */
    int acceptNewConnection() const;
    /*
     * returns the connection Config
     */
    const VirtualHost& getConfig() const;

    /*
     * Send a message to the socket.
     *
     * It returns the number of bytes sent.
     */
    int sendMsg(const std::string& msg);

    /*
     * Reads from a socket and copies over the given input buffer
     *
     * Returns the number of bytes read or raises a ExceptionClientCloseConn
     */
    virtual int read(char buffer[], int len);

private:
    Type type_;
    Socket* socket_;
    VirtualHost config_;
};
