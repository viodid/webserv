#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"
#include <string>

class Connection;

/**
 * @class CGIHandler
 * @brief Forks a CGI interpreter and attaches a CgiProcess to the connection.
 *
 * On success, handle() returns NULL: the CgiProcess is owned by the connection
 * and the main loop is responsible for polling its stdout pipe and finalizing
 * the response. On synchronous failure (script missing, fork failure, etc.)
 * handle() returns a complete error response and no CgiProcess is attached.
 */
class CGIHandler : public IRequestHandler {
public:
    CGIHandler(const Location& conf,
        const VirtualHost& vh,
        const ErrorRenderer& error_renderer,
        const std::string& interpreter,
        Connection* conn);
    ~CGIHandler();

    virtual HttpResponse* handle(const HttpRequest& request);

private:
    const Location conf_;
    const VirtualHost vh_;
    const ErrorRenderer error_renderer_;
    const std::string interpreter_;
    Connection* conn_;

    CGIHandler(const CGIHandler&);
    CGIHandler& operator=(const CGIHandler&);
};
