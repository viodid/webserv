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

/*
 * Returns the absolute file path the request body should be streamed to,
 * or an empty string if the body should be buffered in memory.
 *
 * Currently selects a destination only for POST to a route with `upload_store`.
 * Throws ExceptionBadFraming if the request target is rejected (e.g., path
 * traversal).
 */
std::string selectUploadPath(const HttpRequest& request, const VirtualHost& vh);
