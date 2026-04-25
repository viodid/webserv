#include "../../include/Handlers/HandlerFactory.hpp"
#include "../../include/Exceptions.hpp"
#include "../../include/Handlers/DeleteHandler.hpp"
#include "../../include/Handlers/NotFoundHandler.hpp"
#include "../../include/Handlers/RedirectHandler.hpp"
#include "../../include/Handlers/StaticHandler.hpp"
#include "../../include/Handlers/UploadHandler.hpp"
#include "../../include/HttpRequest/FileBodySink.hpp"
#include "../../include/HttpRequest/MemoryBodySink.hpp"
#include "../../include/Utils.hpp"
#include <sstream>

namespace {

const Location* matchLocation(const std::string& target,
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
    return best;
}

bool containsTraversal(const std::string& s)
{
    // Reject any ".." path segment.
    size_t start = 0;
    while (start < s.size()) {
        size_t end = s.find('/', start);
        if (end == std::string::npos)
            end = s.size();
        if (end - start == 2 && s[start] == '.' && s[start + 1] == '.')
            return true;
        start = end + 1;
    }
    return false;
}

std::string pickFilename(const std::string& target, const std::string& route)
{
    std::string tail = target;
    if (route.size() > 1 && tail.compare(0, route.size(), route) == 0)
        tail.erase(0, route.size());
    while (!tail.empty() && tail[0] == '/')
        tail.erase(0, 1);

    // basename = last path component
    std::string basename = tail;
    size_t slash = tail.rfind('/');
    if (slash != std::string::npos)
        basename = tail.substr(slash + 1);

    if (basename.empty() || basename == "." || basename == "..") {
        std::stringstream ss;
        ss << "upload-" << currTimeMs();
        basename = ss.str();
    }
    return basename;
}

}

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

IBodySink* createBodySink(const HttpRequest& request, const VirtualHost& vh)
{
    const std::string& target = request.getRequestLine().getRequestTarget();
    const std::string& method = request.getRequestLine().getMethod();
    const Location* loc = matchLocation(target, vh.getLocations());
    size_t max_body = vh.getSocketSize();

    if (loc != NULL && method == "POST" && !loc->getUploadStore().empty()) {
        if (containsTraversal(target))
            throw ExceptionBadFraming("path traversal in upload target");
        std::string filename = pickFilename(target, loc->getPath());
        return new FileBodySink(loc->getUploadStore(), filename, max_body);
    }

    return new MemoryBodySink(max_body);
}
