#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"
#include "File.hpp"
#include "handler_utils.hpp"

class StaticHandler : public IRequestHandler {
public:
    StaticHandler(const Location& conf, const ErrorRenderer& error_renderer);
    // TODO: rm
    ~StaticHandler() { std::cout << "StaticHandler destructor called\n"; };

    virtual HttpResponse* handle(const HttpRequest& request);

    bool isDone() const;
    bool hasFailed() const;

private:
    enum State {
        ERROR,
        DONE
    };
    const Location conf_;
    const ErrorRenderer error_renderer_;
    State state_;

    void buildHeaders_(HttpResponse* response);
    void buildFileSource(HttpResponse* response);

    HttpResponse constructHttpOKResponse_(const HttpRequest& request,
        const std::string& file_format,
        const std::string& file_content);
};
