#pragma once
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

class StaticHandler : public IRequestHandler {
public:
    StaticHandler(const ErrorRenderer& error_renderer, const Location& conf);

    virtual HttpResponse handle();

private:
    const ErrorRenderer error_renderer_;
    const Location conf_;
};
