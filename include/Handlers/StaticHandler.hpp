#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"
#include "File.hpp"
#include "handler_utils.hpp"

class StaticHandler : public IRequestHandler {
public:
    StaticHandler(const Location& conf, const ErrorRenderer& error_renderer);

    virtual HttpResponse handle(const HttpRequest& request);

private:
    const Location conf_;
    const ErrorRenderer error_renderer_;

    HttpResponse constructHttpOKResponse_(const HttpRequest& request, File& file);
};
