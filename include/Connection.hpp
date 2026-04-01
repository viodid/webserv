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
    const Socket& getSocket() const;
    const VirtualHost& getConfig() const;
    const std::string& getInputBuffer() const;
    const std::string& getOutputBuffer() const;
    void setInputBuffer(const std::string&);
    void setOutputBuffer(const std::string&);

    virtual int read(char buffer[], int len);

private:
    Type type_;
    Socket* socket_;
    VirtualHost config_;
    std::vector<char> buffer_;
    std::string input_buffer_;
    std::string output_buffer_;
};
