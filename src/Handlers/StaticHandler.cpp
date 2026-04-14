#include "../../include/Handlers/StaticHandler.hpp"

StaticHandler::StaticHandler(const Location& conf, const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
{
}

HttpResponse StaticHandler::handle(const HttpRequest& request)
{
    if (!isMethodAllowed(request, conf_))
        return constructHttpErrorResponse(request, error_renderer_, Location::S_405);
    // construct path (after aliases)
    std::string absolute_path = constructPath(request, conf_);
    // if path is a file -> render and return
    if (isFileReadable(absolute_path))
        return constructHttpOKResponse_(request, absolute_path);
    // if path is a directory
    //// if index is set -> render index file and return
    //// if autoindex is false -> return HttpError
    //// else render directory listing and return
    std::cout << "here\n";
    return constructHttpOKResponse_(request, absolute_path);
}

HttpResponse constructHttpOKResponse_(const HttpRequest& request,
    const std::string& path)
{
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Content-Type", "text/html; charset=utf-8");

    char buf[Settings::PARSER_MAX_BUFFER_SIZE];
    size_t bytes = readFile(buf, Settings::PARSER_MAX_BUFFER_SIZE, path);
    if (bytes < Settings::PARSER_MAX_BUFFER_SIZE)
        buf[bytes + 1] = '\0';
    std::string html = buf;
    std::stringstream ss;
    ss << html.size();
    field_lines.set("Content-Length", ss.str());
    return HttpResponse(
        StatusLine(request.getRequestLine().getHttpVersion(), Location::S_200),
        field_lines,
        Body(html));
}
