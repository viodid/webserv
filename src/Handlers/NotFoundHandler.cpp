#include "../../include/Handlers/NotFoundHandler.hpp"
#include "../../include/Handlers/handler_utils.hpp"

NotFoundHandler::NotFoundHandler(const ErrorRenderer& error_renderer)
    : error_renderer_(error_renderer)
{
}

HttpResponse* NotFoundHandler::handle(const HttpRequest& request)
{
    return constructHttpErrorResponse(request, error_renderer_, Location::S_404);
}
