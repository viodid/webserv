#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

class RedirectHandler : public IRequestHandler {
public:
    RedirectHandler(const Location& conf, const ErrorRenderer& error_renderer);
    ~RedirectHandler() { }

    virtual HttpResponse* handle(const HttpRequest& request);

private:
    const Location conf_;
    const ErrorRenderer error_renderer_;
};
