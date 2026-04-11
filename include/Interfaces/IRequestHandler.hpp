#pragma once
#include "../HttpResponse/HttpResponse.hpp"

class IRequestHandler {
public:
    virtual ~IRequestHandler() { };
    virtual HttpResponse handle() = 0;
};
