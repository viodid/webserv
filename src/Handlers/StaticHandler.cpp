#include "../../include/Handlers/StaticHandler.hpp"

StaticHandler::StaticHandler(const Location& conf, const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
{
}

HttpResponse StaticHandler::handle(const HttpRequest& request)
{
    try {
        std::string path = constructPath(request, conf_);
        stripQueryURI(path);

        if (!File::fileExists(path))
            return constructHttpErrorResponse(request, error_renderer_, Location::S_404);
        if (!isMethodAllowed(request, conf_))
            return constructHttpErrorResponse(request, error_renderer_, Location::S_405);

        File file(path);
        if (!file.isReadable())
            return constructHttpErrorResponse(request, error_renderer_, Location::S_403);

        if (file.getType() == File::DIRECTORY) {
            if (!conf_.getDefaultFile().empty()) {
                if (!path.empty() && path[path.size() - 1] != '/')
                    path.append("/");
                file = File(path.append(conf_.getDefaultFile()));
            } else if (conf_.isDirectoryListing()) {
                std::string dir_list = renderDirListing(path, request.getRequestLine().getRequestTarget());
                return constructHttpOKResponse_(request, "text/html", dir_list);
            } else
                return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
        }
        return constructHttpOKResponse_(request, file.getTypeFormat(), file.readFile());
    } catch (const ExceptionUnsupportedFileType& e) {
        std::cerr << e.what() << '\n';
        return constructHttpErrorResponse(request, error_renderer_, Location::S_415);
    } catch (const ExceptionParentRootDirectory& e) {
        std::cerr << e.what() << '\n';
        return constructHttpErrorResponse(request, error_renderer_, Location::S_403);
    }
    return constructHttpErrorResponse(request, error_renderer_, Location::S_404);
}

HttpResponse StaticHandler::constructHttpOKResponse_(const HttpRequest& request,
    const std::string& file_format,
    const std::string& file_content)
{
    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    std::string file_type = file_format;
    file_type.append("; charset=utf-8");
    field_lines.set("Content-Type", file_type);
    field_lines.set("Connection", "close");

    std::stringstream ss;
    ss << file_content.size();
    field_lines.set("Content-Length", ss.str());
    return HttpResponse(
        StatusLine(request.getRequestLine().getHttpVersion(), Location::S_200),
        field_lines,
        Body(file_content));
}
