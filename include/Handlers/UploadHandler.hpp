#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

class UploadHandler : public IRequestHandler {
public:
    UploadHandler(const Location& conf, const ErrorRenderer& error_renderer);
    ~UploadHandler() { }

    virtual HttpResponse* handle(const HttpRequest& request);

private:
    const Location conf_;
    const ErrorRenderer error_renderer_;
};
