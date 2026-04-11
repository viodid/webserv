#include "../../include/Handlers/StaticHandler.hpp"

StaticHandler::StaticHandler(const ErrorRenderer& error_renderer, const Location& conf)
    : error_renderer_(error_renderer)
    , conf_(conf)
{
}
