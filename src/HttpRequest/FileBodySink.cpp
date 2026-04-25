#include "../../include/HttpRequest/FileBodySink.hpp"
#include "../../include/Exceptions.hpp"
#include <fcntl.h>
#include <unistd.h>

FileBodySink::FileBodySink(const std::string& dir, const std::string& filename, size_t max_bytes)
    : path_(dir + "/" + filename)
    , fd_(-1)
    , bytes_written_(0)
    , max_bytes_(max_bytes)
{
    fd_ = ::open(path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_ == -1)
        throw ExceptionErrorConnectionSocket("FileBodySink: open failed for " + path_);
}

FileBodySink::~FileBodySink()
{
    closeFd_();
}

void FileBodySink::write(const char* data, size_t len)
{
    if (max_bytes_ != 0 && bytes_written_ + len > max_bytes_) {
        unlinkFile_();
        throw ExceptionPayloadTooLarge("body exceeds client_max_body_size");
    }
    size_t total = 0;
    while (total < len) {
        ssize_t n = ::write(fd_, data + total, len - total);
        if (n <= 0) {
            unlinkFile_();
            throw ExceptionErrorConnectionSocket("FileBodySink: write failed");
        }
        total += static_cast<size_t>(n);
    }
    bytes_written_ += len;
}

void FileBodySink::finalize()
{
    closeFd_();
}

size_t FileBodySink::bytesWritten() const
{
    return bytes_written_;
}

const std::string& FileBodySink::getStoredPath() const
{
    return path_;
}

void FileBodySink::closeFd_()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

void FileBodySink::unlinkFile_()
{
    closeFd_();
    ::unlink(path_.c_str());
}
