#include "../../include/HttpRequest/Body.hpp"
#include "../../include/HttpRequest/MemoryBodySink.hpp"
#include <algorithm>

Body::Body()
    : mode_(NoBody)
    , content_length_(0)
    , bytes_received_(0)
    , sink_(NULL)
    , complete_(false)
{
}

Body::~Body()
{
    delete sink_;
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

void Body::installSink(IBodySink* sink)
{
    delete sink_;
    sink_ = sink;
}

bool Body::hasSink() const
{
    return sink_ != NULL;
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
    if (complete_ || sink_ == NULL || mode_ == NoBody)
        return 0;

    if (mode_ == ContentLengthMode) {
        size_t remaining = content_length_ - bytes_received_;
        size_t take = std::min(remaining, buf_len);
        if (take > 0)
            sink_->write(buffer, take);
        bytes_received_ += take;
        if (bytes_received_ == content_length_) {
            complete_ = true;
            sink_->finalize();
        }
        return static_cast<int>(take);
    }

    // Chunked mode
    size_t consumed = chunked_.feed(buffer, buf_len, *sink_);
    bytes_received_ += consumed;
    if (chunked_.isDone()) {
        complete_ = true;
        sink_->finalize();
    }
    return static_cast<int>(consumed);
}

const std::string& Body::get() const
{
    static const std::string empty;
    if (sink_ == NULL)
        return empty;
    const MemoryBodySink* mem = dynamic_cast<const MemoryBodySink*>(sink_);
    if (mem == NULL)
        return empty;
    return mem->getBody();
}

const IBodySink* Body::getSink() const
{
    return sink_;
}

std::string Body::format() const
{
    return get();
}
