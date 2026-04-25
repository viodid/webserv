#include "../../include/Handlers/RedirectHandler.hpp"
#include "../../include/Handlers/handler_utils.hpp"

RedirectHandler::RedirectHandler(const Location& conf, const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
{
}

HttpResponse* RedirectHandler::handle(const HttpRequest& request)
{
    const std::string& code_str = conf_.getRedirectionCode();
    const std::string& target = conf_.getRedirectionPath();

    if (target.empty() || code_str.empty())
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);

    return constructHttpRedirectResponse(request, target,
        Location::statusCodeFromCode(code_str));
}
