#pragma once
#include "../Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

/*
 * Picks the location matching the request target (longest path-prefix wins)
 * and instantiates the appropriate handler for it.
 *
 * Returns a heap-allocated handler; caller takes ownership.
 */
IRequestHandler* createHandler(const HttpRequest& request,
    const VirtualHost& vh,
    const ErrorRenderer& error_renderer);
