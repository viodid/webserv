#include "../../include/Handlers/StaticHandler.hpp"
#include "../../include/HttpResponse/FileBodySource.hpp"
#include "../../include/HttpResponse/StringBodySource.hpp"
#include <sys/stat.h>

StaticHandler::StaticHandler(const Location& conf, const ErrorRenderer& error_renderer)
    : conf_(conf)
    , error_renderer_(error_renderer)
{
}

HttpResponse* StaticHandler::handle(const HttpRequest& request)
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
            const std::string& target = request.getRequestLine().getRequestTarget();
            size_t qpos = target.find('?');
            std::string target_path = (qpos == std::string::npos) ? target : target.substr(0, qpos);
            if (target_path.empty() || target_path[target_path.size() - 1] != '/') {
                std::string new_target = target_path + "/";
                if (qpos != std::string::npos)
                    new_target.append(target, qpos, std::string::npos);
                return constructHttpRedirectResponse(request, new_target, Location::S_301);
            }
            if (!conf_.getDefaultFile().empty()) {
                if (!path.empty() && path[path.size() - 1] != '/')
                    path.append("/");
                path.append(conf_.getDefaultFile());
                file = File(path);
            } else if (conf_.isDirectoryListing()) {
                std::string html = renderDirListing(path, request.getRequestLine().getRequestTarget());
                return buildOkResponse_(request, "text/html", html.size(),
                    new StringBodySource(html));
            } else {
                return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
            }
        }

        struct stat st;
        if (stat(path.c_str(), &st) == -1)
            return constructHttpErrorResponse(request, error_renderer_, Location::S_500);

        return buildOkResponse_(request, file.getTypeFormat(),
            static_cast<size_t>(st.st_size),
            new FileBodySource(path));
    } catch (const ExceptionUnsupportedFileType& e) {
        std::cerr << "StaticHandler -  unsupported file type: " << e.what() << '\n';
        return constructHttpErrorResponse(request, error_renderer_, Location::S_415);
    } catch (const ExceptionParentRootDirectory& e) {
        std::cerr << "StaticHandler -  exception parent root dir: " << e.what() << '\n';
        return constructHttpErrorResponse(request, error_renderer_, Location::S_403);
    } catch (const std::exception& e) {
        std::cerr << "StaticHandler -  general exception: " << e.what() << '\n';
        return constructHttpErrorResponse(request, error_renderer_, Location::S_500);
    }
}

HttpResponse* StaticHandler::buildOkResponse_(const HttpRequest& request,
    const std::string& mime,
    size_t content_length,
    IBodySource* body)
{
    HttpResponse* response = new HttpResponse;
    response->setStatusLine(StatusLine(request.getRequestLine().getHttpVersion(), Location::S_200));

    FieldLines field_lines;
    field_lines.set("Server", "42webserv/0.1.0");
    field_lines.set("Content-Type", mime);
    field_lines.set("Connection", "close");

    std::stringstream ss;
    ss << content_length;
    field_lines.set("Content-Length", ss.str());

    response->setFieldLines(field_lines);
    response->setBodySource(body);
    return response;
}
