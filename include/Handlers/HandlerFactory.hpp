#pragma once
#include "../Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

class Connection;

/*
 * Picks the location matching the request target (longest path-prefix wins)
 * and instantiates the appropriate handler for it.
 *
 * Returns a heap-allocated handler; caller takes ownership.
 *
 * The connection pointer is forwarded to handlers that need to attach
 * out-of-band state (e.g. CGIHandler, which spawns a process and stores it
 * on the connection). May be NULL for handlers that do not need it.
 */
IRequestHandler* createHandler(const HttpRequest& request,
    const VirtualHost& vh,
    const ErrorRenderer& error_renderer,
    Connection* conn);
