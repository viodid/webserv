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
    File file(request, conf_);
    // if path is a file -> render and return
    if (file.isReadable())
        return constructHttpOKResponse_(request, file);
    // if path is a directory
    //// if index is set -> render index file and return
    //// if autoindex is false -> return HttpError
    //// else render directory listing and return
    std::cout << "here\n";
    return constructHttpOKResponse_(request, file);
}

HttpResponse StaticHandler::constructHttpOKResponse_(const HttpRequest& request, File& file)
{
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    std::string file_type = file.getTypeFormat();
    file_type.append("; charset=utf-8");
    field_lines.set("Content-Type", file_type);

    std::stringstream ss;
    ss << file.readFile().size();
    field_lines.set("Content-Length", ss.str());
    return HttpResponse(
        StatusLine(request.getRequestLine().getHttpVersion(), Location::S_200),
        field_lines,
        Body(file.readFile()));
}
