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
    std::string path = constructPath(request, conf_);
    Location::StatusCodes status_code = Location::S_200;
    try {
        File file(path);
        if (file.getType() == File::DIRECTORY) {
            if (!conf_.getDefaultFile().empty())
                path.append("/" + conf_.getDefaultFile());
            // if autoindex is false -> return HttpError
            else if (conf_.isDirectoryListing())
                return renderDirListing(path);
            // else render directory listing and return
        }
        // if path is a file -> render and return
        if (file.isReadable())
            return constructHttpOKResponse_(request, file);
    } catch (const ExceptionUnsupportedFileType& e) {
        std::cerr << e.what() << '\n';
        status_code = Location::S_415;
    }
    status_code = Location::S_404;
    return constructHttpErrorResponse(request, error_renderer_, status_code);
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
    std::cout << "FILE: \n"
              << "- type: " << file.getTypeFormat() << '\n'
              << "- path: " << file.getPath() << '\n';
    return HttpResponse(
        StatusLine(request.getRequestLine().getHttpVersion(), Location::S_200),
        field_lines,
        Body(file.readFile()));
}
