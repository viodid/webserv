#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"
#include "File.hpp"
#include "handler_utils.hpp"

class StaticHandler : public IRequestHandler {
public:
    StaticHandler(const Location& conf, const ErrorRenderer& error_renderer);
    ~StaticHandler() { }

    virtual HttpResponse* handle(const HttpRequest& request);

private:
    const Location conf_;
    const ErrorRenderer error_renderer_;

    HttpResponse* buildOkResponse_(const HttpRequest& request,
        const std::string& mime,
        size_t content_length,
        IBodySource* body);
};
