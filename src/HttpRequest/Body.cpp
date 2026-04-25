#include "../../include/HttpRequest/Body.hpp"
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>

Body::Body()
    : mode_(NoBody)
    , content_length_(0)
    , bytes_received_(0)
    , max_bytes_(0)
    , file_fd_(-1)
    , complete_(false)
{
}

Body::~Body()
{
    closeFd_();
}

void Body::setContentLength(size_t n)
{
    mode_ = ContentLengthMode;
    content_length_ = n;
    if (n == 0)
        complete_ = true;
}

void Body::setChunked()
{
    mode_ = ChunkedMode;
}

void Body::setMaxBytes(size_t n)
{
    max_bytes_ = n;
}

void Body::useFile(const std::string& path)
{
    file_path_ = path;
    file_fd_ = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd_ == -1)
        throw ExceptionErrorConnectionSocket("Body: open failed for " + path);
}

Body::Mode Body::getMode() const
{
    return mode_;
}

bool Body::isComplete() const
{
    return complete_;
}

int Body::parse(const char* buffer, size_t buf_len)
{
    if (complete_ || mode_ == NoBody)
        return 0;

    if (mode_ == ContentLengthMode) {
        size_t remaining = content_length_ - bytes_received_;
        size_t take = std::min(remaining, buf_len);
        if (take > 0)
            writeBytes_(buffer, take);
        if (bytes_received_ == content_length_) {
            complete_ = true;
            closeFd_();
        }
        return static_cast<int>(take);
    }

    // ChunkedMode
    decode_buf_.clear();
    size_t consumed = chunked_.feed(buffer, buf_len, decode_buf_);
    if (!decode_buf_.empty())
        writeBytes_(decode_buf_.data(), decode_buf_.size());
    if (chunked_.isDone()) {
        complete_ = true;
        closeFd_();
    }
    return static_cast<int>(consumed);
}

const std::string& Body::get() const
{
    return body_;
}

const std::string& Body::getStoredPath() const
{
    return file_path_;
}

std::string Body::format() const
{
    return body_;
}

void Body::writeBytes_(const char* data, size_t len)
{
    if (max_bytes_ != 0 && bytes_received_ + len > max_bytes_) {
        abortFile_();
        throw ExceptionPayloadTooLarge("body exceeds client_max_body_size");
    }
    if (file_fd_ != -1) {
        size_t total = 0;
        while (total < len) {
            ssize_t n = ::write(file_fd_, data + total, len - total);
            if (n <= 0) {
                abortFile_();
                throw ExceptionErrorConnectionSocket("Body: file write failed");
            }
            total += static_cast<size_t>(n);
        }
    } else {
        body_.append(data, len);
    }
    bytes_received_ += len;
}

void Body::closeFd_()
{
    if (file_fd_ != -1) {
        ::close(file_fd_);
        file_fd_ = -1;
    }
}

void Body::abortFile_()
{
    if (file_fd_ != -1) {
        closeFd_();
        ::unlink(file_path_.c_str());
    }
}
