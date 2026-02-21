#pragma once
#include "Config.hpp"
#include "Socket.hpp"

/**
 * @struct Connection
 * @brief State holder of a socket.
 *
 * This struct holds the session state for a given TCP socket (client or
 * listener). Ties the VitualHost configuration to a socket and holds I/O
 * buffers.
 */
struct Connection {
    enum Type {
        LISTENER,
        CLIENT,
    };

    const Type type;
    const Socket& socket;
    const VirtualHost& config;
    std::string input_buffer;
    std::string output_buffer;
};
