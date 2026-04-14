#include "../../include/Handlers/File.hpp"
#include <unistd.h>

static std::string constructPath(const HttpRequest& request, const Location& location);
static File::Type mapFileType(const std::string& path);

File::File(const std::string& path)
    : path_(path)
    , type_(mapFileType(path))
{
}

File::File(const HttpRequest& request, const Location& location)
    : path_(constructPath(request, location))
    , type_(mapFileType(path_))
{
#if DEBUG
    std::cout << "File path_: " << path_ << "\n";
#endif
}

std::string File::getTypeFormat() const
{
    switch (type_) {
    case TEXT_HTML:
        return "text/html";
    case TEXT_CSS:
        return "text/css";
    case TEXT_JS:
        return "text/js";
    case IMAGE_PNG:
        return "image/png";
    case IMAGE_JPEG:
        return "image/jpeg";
    default:
        return "text/plain";
    }
}

const std::string& File::readFile()
{
#if DEBUG
    std::cout << "File.read()\n";
#endif
    if (content_.empty()) {
#if DEBUG
        std::cout << "File.read() cache miss\n";
#endif
        int fd = open(path_.c_str(), O_RDONLY);
        if (fd == -1)
            throw std::runtime_error(std::strerror(errno));

        char buffer[Settings::PARSER_MAX_BUFFER_SIZE];
        size_t bytes = read(fd, buffer, Settings::PARSER_MAX_BUFFER_SIZE);
        if (fd == -1)
            throw std::runtime_error(std::strerror(errno));
        if (bytes < Settings::PARSER_MAX_BUFFER_SIZE)
            buffer[bytes] = '\n';
        close(fd);
        content_ = std::string(buffer);
    }
    return content_;
}

bool File::isReadable() const
{
    if (access(path_.c_str(), R_OK) != 0) {
        std::cerr << std::strerror(errno) << "\n";
        return false;
    }
    return true;
}

static std::string constructPath(const HttpRequest& request, const Location& location)
{
    if (location.getRoot().empty() || location.getPath().empty())
        throw std::runtime_error("location.root or location.path cannot be empty");
    // "/" "/main.js" "/var/www" -> "/var/www/main.js"
    // "/statics" "/statics/main.css" "/var/www" -> "/var/www/main.css"
    // "/" "/statics/main.css" "/var/www" -> "/var/www/statics/main.css"
    std::string alias = location.getRoot();
    std::string request_target = request.getRequestLine().getRequestTarget();
    std::string route = location.getPath();
    if (route.size() > 1) // location route is not "/"
        request_target.erase(0, route.size());
    alias.append(request_target);

    return alias;
}

static File::Type mapFileType(const std::string& path)
{
    std::string extension = path.substr(path.rfind('.', path.size() - 1) + 1);
    std::cout << "extension: " << extension << '\n';
    if (extension == "html")
        return File::TEXT_HTML;
    if (extension == "css")
        return File::TEXT_CSS;
    if (extension == "js")
        return File::TEXT_JS;
    if (extension == "png")
        return File::IMAGE_PNG;
    if (extension == "jpg" || extension == "jpeg")
        return File::IMAGE_PNG;
    throw ExceptionUnsupportedFileType(extension);
}
