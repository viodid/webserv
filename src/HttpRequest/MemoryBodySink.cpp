#include "../../include/HttpRequest/MemoryBodySink.hpp"
#include "../../include/Exceptions.hpp"

MemoryBodySink::MemoryBodySink(size_t max_bytes)
    : max_bytes_(max_bytes)
{
}

MemoryBodySink::~MemoryBodySink() { }

void MemoryBodySink::write(const char* data, size_t len)
{
    if (max_bytes_ != 0 && body_.size() + len > max_bytes_)
        throw ExceptionPayloadTooLarge("body exceeds client_max_body_size");
    body_.append(data, len);
}

void MemoryBodySink::finalize() { }

size_t MemoryBodySink::bytesWritten() const
{
    return body_.size();
}

const std::string& MemoryBodySink::getStoredPath() const
{
    return empty_path_;
}

const std::string& MemoryBodySink::getBody() const
{
    return body_;
}
