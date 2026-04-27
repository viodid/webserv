#pragma once
#include "Config.hpp"
#include "Exceptions.hpp"
#include "HttpRequest/HttpRequest.hpp"
#include "HttpResponse/HttpResponse.hpp"
#include "Interfaces/IReader.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

class CgiProcess; // forward decl

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
    int getFd() const;
    HttpRequest& getRequest();
    int acceptNewConnection() const;
    const VirtualHost& getConfig() const;

    /*
     * Takes ownership of the response. Replaces any prior response.
     * Serializes headers into the write buffer immediately.
     */
    void setResponse(HttpResponse* response);
    bool hasResponse() const;
    /*
     * Pulls one body chunk from the response and appends it to the write
     * buffer. No-op if the body source is exhausted.
     */
    void pullBodyChunk();
    /*
     * Number of bytes currently waiting in the write buffer.
     */
    size_t writeBufferSize() const;
    /*
     * Returns true once the buffer is empty and the body source is exhausted.
     */
    bool isWriteDone() const;
    /*
     * Send as many bytes as the kernel accepts; advance the buffer by that
     * amount. Returns the number of bytes sent.
     */
    size_t sendBytes();

    virtual int read(char buffer[], int len);

    /*
     * Optional CGI process attached to this connection. NULL when no CGI is
     * in flight. Connection owns the pointer and deletes it.
     */
    CgiProcess* getCgi() const;
    void setCgi(CgiProcess* cgi);
    void clearCgi();

private:
    const VirtualHost config_;
    Type type_;
    Socket* socket_;
    HttpRequest request_;
    HttpResponse* response_;
    std::vector<char> out_buf_;
    CgiProcess* cgi_;

    void appendToBuffer_(const std::string& s);

    Connection(const Connection&);
    Connection& operator=(const Connection&);
};
