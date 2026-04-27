#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

class DeleteHandler : public IRequestHandler {
public:
    DeleteHandler(const Location& conf, const ErrorRenderer& error_renderer);
    ~DeleteHandler() { }

    virtual HttpResponse* handle(const HttpRequest& request);

private:
    const Location conf_;
    const ErrorRenderer error_renderer_;
};
