#include "../../include/Handlers/DeleteHandler.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include "../../include/HttpResponse/EmptyBodySource.hpp"
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

DeleteHandler::DeleteHandler(const Location& conf, const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
{
}

HttpResponse* DeleteHandler::handle(const HttpRequest& request)
{
    std::string path;
    try {
        path = constructPath(request, conf_);
    } catch (const std::exception&) {
        return constructHttpErrorResponse(request, error_renderer_, Location::S_400);
    }

    struct stat st;
    if (stat(path.c_str(), &st) == -1) {
        if (errno == ENOENT)
            return constructHttpErrorResponse(request, error_renderer_, Location::S_404);
        return constructHttpErrorResponse(request, error_renderer_, Location::S_403);
    }
    if (S_ISDIR(st.st_mode))
        return constructHttpErrorResponse(request, error_renderer_, Location::S_403);

    if (std::remove(path.c_str()) != 0)
        return constructHttpErrorResponse(request, error_renderer_, Location::S_403);

    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Content-Length", "0");
    field_lines.set("Connection", "close");

    HttpResponse* response = new HttpResponse;
    response->setStatusLine(StatusLine(request.getRequestLine().getHttpVersion(), Location::S_200));
    response->setFieldLines(field_lines);
    response->setBodySource(new EmptyBodySource);
    return response;
}
