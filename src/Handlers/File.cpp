#include "../../include/Handlers/File.hpp"
#include <cstring>
#include <dirent.h>
#include <stdexcept>
#include <unistd.h>
#include <vector>

static File::Type mapFileType(const std::string& path);

File::File(const std::string& path)
    : path_(path)
    , type_(mapFileType(path))
{
}

const std::string& File::getPath() const
{
    return path_;
}
const File::Type& File::getType() const
{
    return type_;
}

std::string File::getTypeFormat() const
{
    if (type_ == DIRECTORY)
        throw std::runtime_error("directory does not have a file format");
    switch (type_) {
    case TEXT_HTML:
        return "text/html";
    case TEXT_CSS:
        return "text/css";
    case TEXT_JS:
        return "text/javascript";
    case APP_JSON:
        return "application/json";
    case IMAGE_PNG:
        return "image/png";
    case IMAGE_JPEG:
        return "image/jpeg";
    case IMAGE_ICO:
        return "image/vnd.microsoft.icon";
    default:
        std::cerr << "fallthrough default file format 'text/plan'\n";
        return "text/plain";
    }
}

// TODO: use stat to get file size - chunk the response if size > 32KiB (change buffer size settings)
const std::string& File::readFile()
{
    if (type_ == DIRECTORY)
        throw std::runtime_error("cannot read a directory");
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

        char buffer[Settings::RESPONSE_BUFFER_SIZE];
        size_t bytes = read(fd, buffer, Settings::RESPONSE_BUFFER_SIZE);
        if (fd == -1)
            throw std::runtime_error(std::strerror(errno));
        if (bytes < Settings::RESPONSE_BUFFER_SIZE)
            buffer[bytes] = '\0';
        close(fd);
        content_ = std::string(buffer);
    }
    return content_;
}

bool File::isReadable() const
{
    if (access(path_.c_str(), R_OK) != 0) {
        std::cerr << std::strerror(errno) << " - " << path_ << '\n';
        return false;
    }
    return true;
}

bool File::isWritable() const
{
    if (access(path_.c_str(), W_OK) != 0) {
        std::cerr << std::strerror(errno) << " - " << path_ << '\n';
        return false;
    }
    return true;
}

bool File::isExecutable() const
{
    if (access(path_.c_str(), X_OK) != 0) {
        std::cerr << std::strerror(errno) << " - " << path_ << '\n';
        return false;
    }
    return true;
}

static bool isDirectory(const std::string& path)
{
    struct stat s;
    if (stat(path.c_str(), &s) != 0)
        throw std::runtime_error(std::strerror(errno));
    if (S_ISDIR(s.st_mode))
        return true;
    return false;
}

static File::Type mapFileType(const std::string& path)
{
    if (isDirectory(path))
        return File::DIRECTORY;

    std::string extension = path.substr(path.rfind('.', path.size() - 1) + 1);
    std::cout << "extension: " << extension << '\n';
    if (extension == "html" || extension == "htm")
        return File::TEXT_HTML;
    if (extension == "css")
        return File::TEXT_CSS;
    if (extension == "js" || extension == "mjs")
        return File::TEXT_JS;
    if (extension == "json")
        return File::APP_JSON;
    if (extension == "png")
        return File::IMAGE_PNG;
    if (extension == "jpg" || extension == "jpeg")
        return File::IMAGE_PNG;
    if (extension == "ico")
        return File::IMAGE_ICO;
    throw ExceptionUnsupportedFileType(extension);
}

