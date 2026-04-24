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

    virtual bool isDone() const;
    virtual bool hasFailed() const;

private:
    enum State {
        PENDING,
        ERROR,
        DONE
    };
    const Location conf_;
    const ErrorRenderer error_renderer_;
    State state_;

    HttpResponse* buildOkResponse_(const HttpRequest& request,
        const std::string& mime,
        size_t content_length,
        IBodySource* body);
};
