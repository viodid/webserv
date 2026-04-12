#include "../../include/Handlers/handler_utils.hpp"

size_t readFile(char* buffer, size_t len, const std::string& path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
        throw std::runtime_error(std::strerror(errno));

    size_t bytes = read(fd, buffer, len);
    if (fd == -1)
        throw std::runtime_error(std::strerror(errno));
    close(fd);

    return bytes;
}

HttpResponse constructHttpErrorResponse(const HttpRequest& request,
    const ErrorRenderer& error_renderer,
    Location::ErrorPages error_no)
{
    std::string body_html = error_renderer.render(error_no);
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Content-Type", "text/html; charset=utf-8");
    std::strstream ss;
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
