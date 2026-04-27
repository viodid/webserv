#include "../../include/Handlers/HandlerFactory.hpp"
#include "../../include/Handlers/CGIHandler.hpp"
#include "../../include/Handlers/DeleteHandler.hpp"
#include "../../include/Handlers/NotFoundHandler.hpp"
#include "../../include/Handlers/RedirectHandler.hpp"
#include "../../include/Handlers/StaticHandler.hpp"
#include "../../include/Handlers/UploadHandler.hpp"
#include "../../include/Handlers/handler_utils.hpp"
#include "../../include/Utils.hpp"

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
