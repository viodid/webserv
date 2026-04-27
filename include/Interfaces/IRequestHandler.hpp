#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"

class IRequestHandler {
public:
    virtual ~IRequestHandler() { };

    virtual HttpResponse* handle(const HttpRequest& request) = 0;
};
