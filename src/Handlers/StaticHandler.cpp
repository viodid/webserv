#include "../../include/Handlers/StaticHandler.hpp"

StaticHandler::StaticHandler(const Location& conf,const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
{
}


HttpResponse StaticHandler::handle()
{
    return HttpResponse(StatusLine("",Location::E_400), FieldLines(), Body());
}
