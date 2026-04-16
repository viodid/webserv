#include "../../include/Handlers/handler_utils.hpp"
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

bool isDirectory(const std::string& path)
{
    struct stat s;
    if (stat(path.c_str(), &s) != 0)
        throw std::runtime_error(std::strerror(errno));
    if S_ISDIR (s.st_mode)
        return true;
    return false;
}

std::string constructPath(const HttpRequest& request, const Location& location)
{
    if (location.getRoot().empty() || location.getPath().empty())
        throw std::runtime_error("location.root or location.path cannot be empty");
    // "/" "/main.js" "/var/www" -> "/var/www/main.js"
    // "/statics" "/statics/main.css" "/var/www" -> "/var/www/main.css"
    // "/" "/statics/main.css" "/var/www" -> "/var/www/statics/main.css"
    std::string alias = location.getRoot();
    std::string request_target = request.getRequestLine().getRequestTarget();
    std::string route = location.getPath();
    if (route.size() > 1) // location route is not "/"
        request_target.erase(0, route.size());
    alias.append(request_target);

    return alias;
}
