#include "../../include/Handlers/RedirectHandler.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include "../../include/HttpResponse/EmptyBodySource.hpp"

RedirectHandler::RedirectHandler(const Location& conf, const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
    , state_(PENDING)
{
}

bool RedirectHandler::isDone() const
{
    return state_ != PENDING;
}

bool RedirectHandler::hasFailed() const
{
    return state_ == ERROR;
}

HttpResponse* RedirectHandler::handle(const HttpRequest& request)
{
    const std::string& code_str = conf_.getRedirectionCode();
    const std::string& target = conf_.getRedirectionPath();

    if (target.empty() || code_str.empty()) {
        state_ = ERROR;
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
    }

    Location::StatusCodes code = Location::statusCodeFromCode(code_str);

    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Location", target);
    field_lines.set("Content-Length", "0");
    field_lines.set("Connection", "close");

    HttpResponse* response = new HttpResponse;
    response->setStatusLine(StatusLine(request.getRequestLine().getHttpVersion(), code));
    response->setFieldLines(field_lines);
    response->setBodySource(new EmptyBodySource);

    state_ = DONE;
    return response;
}
