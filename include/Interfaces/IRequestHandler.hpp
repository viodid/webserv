#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"

class IRequestHandler {
public:
    virtual ~IRequestHandler() { };

    virtual HttpResponse* handle(const HttpRequest& request) = 0;
    virtual bool isDone() const = 0;
    virtual bool hasFailed() const = 0;
};
