#include "../../include/HttpRequest/Body.hpp"

Body::Body()
    : content_length_(0)
{
}
const std::string& Body::get() const
{
    return body_;
}

void Body::set(const std::string& body)
{
    body_ = body;
}

int Body::parse(const char* buffer, size_t buf_len, const std::string& content_len)
{

    if (content_length_ == 0) {
        content_length_ = std::atoi(content_len.c_str());
        if (content_length_ == 0)
            throw ExceptionMalformedFieldLine("malformed 'Content-Length' header");
    }

    if (buf_len >= content_length_)
        body_ = std::string(buffer, std::min(static_cast<size_t>(buf_len), content_length_));

    return body_.size();
}
