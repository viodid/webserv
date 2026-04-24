#include "../../include/Handlers/HandlerFactory.hpp"
#include "../../include/Handlers/RedirectHandler.hpp"
#include "../../include/Handlers/StaticHandler.hpp"

static const Location* matchLocation(const std::string& target,
    const std::vector<Location>& locations)
{
    const Location* best = NULL;
    size_t best_len = 0;
    for (size_t i = 0; i < locations.size(); ++i) {
        const std::string& path = locations[i].getPath();
        if (target.compare(0, path.size(), path) == 0) {
            if (best == NULL || path.size() > best_len) {
                best = &locations[i];
                best_len = path.size();
            }
        }
    }
    if (best == NULL && !locations.empty())
        best = &locations[0];
    return best;
}

IRequestHandler* createHandler(const HttpRequest& request,
    const VirtualHost& vh,
    const ErrorRenderer& error_renderer)
{
    const std::string& target = request.getRequestLine().getRequestTarget();
    const Location* loc = matchLocation(target, vh.getLocations());

    if (loc == NULL)
        return new StaticHandler(vh.getLocations()[0], error_renderer);

    if (!loc->getRedirectionPath().empty())
        return new RedirectHandler(*loc, error_renderer);

    return new StaticHandler(*loc, error_renderer);
}
