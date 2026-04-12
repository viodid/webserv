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

#if DEBUG
    std::cout << "constructPath: " << alias << "\n";
#endif

    return alias;
}

bool isFileReadable(const std::string& path)
{
    if (access(path.c_str(), R_OK) != 0) {
        std::cerr << std::strerror(errno) << "\n";
        return false;
    }
    return true;
}
