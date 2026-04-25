#pragma once
#include "../Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IBodySink.hpp"
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
 * Selects the body sink for a request based on its target/method and the
 * matching location:
 *   - POST to a route with `upload_store` -> FileBodySink under upload_store
 *   - everything else                    -> MemoryBodySink
 *
 * Returns a heap-allocated sink; caller takes ownership (or hands it off to
 * HttpRequest::installBodySink). May throw ExceptionBadFraming if the request
 * target attempts path traversal.
 */
IBodySink* createBodySink(const HttpRequest& request, const VirtualHost& vh);
