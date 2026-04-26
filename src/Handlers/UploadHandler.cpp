#include "../../include/Handlers/UploadHandler.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include "../../include/HttpResponse/EmptyBodySource.hpp"

UploadHandler::UploadHandler(const Location& conf, const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
{
}

HttpResponse* UploadHandler::handle(const HttpRequest& request)
{
    const std::string& path = request.getBody().getStoredPath();
    if (path.empty())
        return constructHttpErrorResponse(request, error_renderer_, Location::S_400);

    std::string basename = path;
    size_t slash = path.rfind('/');
    if (slash != std::string::npos)
        basename = path.substr(slash + 1);

    std::string route = conf_.getPath();
    if (!route.empty() && route[route.size() - 1] == '/')
        route.erase(route.size() - 1);
    std::string location = route + "/" + basename;

    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Location", location);
    field_lines.set("Content-Length", "0");
    field_lines.set("Connection", "close");

    HttpResponse* response = new HttpResponse;
    response->setStatusLine(StatusLine(request.getRequestLine().getHttpVersion(), Location::S_201));
    response->setFieldLines(field_lines);
    response->setBodySource(new EmptyBodySource);
    return response;
}
