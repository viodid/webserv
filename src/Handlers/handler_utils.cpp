#include "../../include/Handlers/handler_utils.hpp"

HttpResponse constructHttpErrorResponse(const HttpRequest& request,
    const ErrorRenderer& error_renderer,
    Location::StatusCodes error_no)
{
    std::string body_html = error_renderer.render(error_no);
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Content-Type", "text/html; charset=utf-8");
    std::stringstream ss;
    ss << body_html.size();
    field_lines.set("Content-Length", ss.str());
    return HttpResponse(
        StatusLine(request.getRequestLine().getHttpVersion(), error_no),
        field_lines,
        Body(body_html));
}

bool isMethodAllowed(const HttpRequest& request, const Location& location)
{
    for (std::vector<Location::AllowedMethods>::const_iterator it = location.getAllowedMethods().begin();
        it != location.getAllowedMethods().end();
        it++) {
        if (*it == location.methodFromString(request.getRequestLine().getMethod()))
            return true;
    }
    return false;
}
