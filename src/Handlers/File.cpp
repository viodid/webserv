#include "../../include/Handlers/File.hpp"

File::File(const std::string& path)
{
    path_ = path;
    type_ = mapFileType_();
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
    case PDF:
        return "application/pdf";
    default:
        std::cerr << "fallthrough default file format 'text/plan'\n";
        return "text/plain";
    }
}

const std::string& File::readFile()
{
    if (type_ == DIRECTORY)
        throw std::runtime_error("cannot read a directory");
    if (content_.empty()) {
        int fd = open(path_.c_str(), O_RDONLY);
        if (fd == -1)
            throw std::runtime_error(std::strerror(errno));

        char buffer[Settings::RESPONSE_BUFFER_SIZE];
        int bytes = read(fd, buffer, sizeof(buffer));
        if (bytes == -1) {
            close(fd);
            throw std::runtime_error(std::strerror(errno));
        }
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

bool File::fileExists(const std::string& path)
{
    if (access(path.c_str(), F_OK) != 0) {
        std::cerr << std::strerror(errno) << " - " << path << '\n';
        return false;
    }
    return true;
}

bool File::isDirectory_() const
{
    struct stat s;
    if (stat(path_.c_str(), &s) != 0)
        throw std::runtime_error(std::strerror(errno));
    if (S_ISDIR(s.st_mode))
        return true;
    return false;
}

File::Type File::mapFileType_() const
{

    if (isDirectory_())
        return File::DIRECTORY;

    std::string extension = path_.substr(path_.rfind('.', path_.size() - 1) + 1);
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
        return File::IMAGE_JPEG;
    if (extension == "ico")
        return File::IMAGE_ICO;
    if (extension == "pdf")
        return File::PDF;
    throw ExceptionUnsupportedFileType(extension);
}
