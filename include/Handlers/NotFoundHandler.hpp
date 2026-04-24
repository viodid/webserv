#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

class NotFoundHandler : public IRequestHandler {
public:
    NotFoundHandler(const ErrorRenderer& error_renderer);
    ~NotFoundHandler() { }

    virtual HttpResponse* handle(const HttpRequest& request);

private:
    const ErrorRenderer error_renderer_;
};
