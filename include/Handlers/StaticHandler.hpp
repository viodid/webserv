#pragma once
#include "../Interfaces/IRequestHandler.hpp"
#include "ErrorRenderer.hpp"

class StaticHandler : public IRequestHandler {
public:
    StaticHandler(const Location& conf,const ErrorRenderer& error_renderer);

    virtual HttpResponse handle();

private:
    const Location conf_;
    const ErrorRenderer error_renderer_;
};
