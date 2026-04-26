#include "../../include/Handlers/HandlerFactory.hpp"
#include "../../include/Handlers/DeleteHandler.hpp"
#include "../../include/Handlers/NotFoundHandler.hpp"
#include "../../include/Handlers/RedirectHandler.hpp"
#include "../../include/Handlers/StaticHandler.hpp"
#include "../../include/Handlers/UploadHandler.hpp"
#include "../../include/Utils.hpp"

IRequestHandler* createHandler(const HttpRequest& request,
    const VirtualHost& vh,
    const ErrorRenderer& error_renderer)
{
    const std::string& target = request.getRequestLine().getRequestTarget();
    const std::string& method = request.getRequestLine().getMethod();
    const Location* loc = matchLocation(target, vh.getLocations());

    if (loc == NULL)
        return new NotFoundHandler(error_renderer);

    if (!loc->getRedirectionPath().empty())
        return new RedirectHandler(*loc, error_renderer);

    if (method == "POST" && !loc->getUploadStore().empty())
        return new UploadHandler(*loc, error_renderer);

    if (method == "DELETE")
        return new DeleteHandler(*loc, error_renderer);

    return new StaticHandler(*loc, error_renderer);
}

const Location* matchLocation(const std::string& target,
    const std::vector<Location>& locations)
{
    const Location* best = NULL;
    size_t best_len = 0;
    for (size_t i = 0; i < locations.size(); ++i) {
        const std::string& path = locations[i].getPath();
        if (target.compare(0, path.size(), path) != 0)
            continue;
        bool on_boundary = target.size() == path.size()
            || path[path.size() - 1] == '/'
            || target[path.size()] == '/';
        if (!on_boundary)
            continue;
        if (best == NULL || path.size() > best_len) {
            best = &locations[i];
            best_len = path.size();
        }
    }
    return best;
}
