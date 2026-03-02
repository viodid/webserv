#pragma once
#include "Config.hpp"
#include "Socket.hpp"

/**
 * @class Connection
 * @brief State holder of a socket.
 *
 * This class holds the session state for a given TCP socket (client or
 * listener). Ties the VitualHost configuration to a socket and holds I/O
 * buffers.
 */
class Connection {
public:
    enum Type {
        LISTENER,
        CLIENT
    };
    Connection(Type, Socket*, const VirtualHost&);
    ~Connection();

    Type getType() const;
    const Socket& getSocket() const;
    const VirtualHost& getConfig() const;
    const std::string& getInputBuffer() const;
    const std::string& getOutputBuffer() const;
    void setInputBuffer(const std::string&);
    void setOutputBuffer(const std::string&);

private:
    Type type_;
    Socket* socket_;
    VirtualHost config_;
    std::string input_buffer_;
    std::string output_buffer_;
};
