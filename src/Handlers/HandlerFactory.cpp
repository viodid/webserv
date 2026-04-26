#include "../../include/Handlers/HandlerFactory.hpp"
#include "../../include/Handlers/CGIHandler.hpp"
#include "../../include/Handlers/DeleteHandler.hpp"
#include "../../include/Handlers/NotFoundHandler.hpp"
#include "../../include/Handlers/RedirectHandler.hpp"
#include "../../include/Handlers/StaticHandler.hpp"
#include "../../include/Handlers/UploadHandler.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include "../../include/Utils.hpp"
#include <sys/stat.h>

namespace {

// Strip the query string and return the file extension (with leading dot).
// Returns an empty string if no extension is found.
std::string extensionOf(const std::string& target)
{
    std::string path = target;
    size_t q = path.find('?');
    if (q != std::string::npos)
        path.erase(q);
    size_t slash = path.rfind('/');
    size_t dot = path.rfind('.');
    if (dot == std::string::npos)
        return "";
    if (slash != std::string::npos && dot < slash)
        return "";
    return path.substr(dot);
}

} // namespace

IRequestHandler* createHandler(const HttpRequest& request,
    const VirtualHost& vh,
    const ErrorRenderer& error_renderer,
    Connection* conn)
{
    const std::string& target = request.getRequestLine().getRequestTarget();
    const std::string& method = request.getRequestLine().getMethod();
    const Location* loc = matchLocation(target, vh.getLocations());

    if (loc == NULL)
        return new NotFoundHandler(error_renderer);

    if (!loc->getRedirectionPath().empty())
        return new RedirectHandler(*loc, error_renderer);

    // CGI: extension match against the location's cgi_map. Runs before
    // upload/static so a CGI script in a location that also accepts uploads
    // is dispatched to CGI.
    if (!loc->getCgiMap().empty()) {
        std::string ext = extensionOf(target);
        std::map<std::string, std::string>::const_iterator it
            = loc->getCgiMap().find(ext);
        if (it != loc->getCgiMap().end())
            return new CGIHandler(*loc, vh, error_renderer, it->second, conn);
    }

    if (method == "POST" && !loc->getUploadStore().empty())
        return new UploadHandler(*loc, error_renderer);

    if (method == "DELETE")
        return new DeleteHandler(*loc, error_renderer);

    return new StaticHandler(*loc, error_renderer);
}
