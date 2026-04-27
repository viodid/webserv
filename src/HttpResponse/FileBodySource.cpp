#include "../../include/HttpResponse/FileBodySource.hpp"
#include "../../include/Exceptions.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

FileBodySource::FileBodySource(const std::string& path)
    : fd_(-1)
    , eof_(false)
{
    fd_ = open(path.c_str(), O_RDONLY);
    if (fd_ == -1)
        throw std::runtime_error(std::strerror(errno));
}

FileBodySource::~FileBodySource()
{
    if (fd_ != -1)
        close(fd_);
}

std::string FileBodySource::nextChunk()
{
    if (eof_)
        return std::string();

    char buf[Settings::RESPONSE_BUFFER_SIZE];
    ssize_t n = read(fd_, buf, sizeof(buf));
    if (n == -1)
        throw std::runtime_error(std::strerror(errno));
    if (n == 0) {
        eof_ = true;
        return std::string();
    }
    return std::string(buf, n);
}

bool FileBodySource::isEmpty()
{
    return eof_;
}
