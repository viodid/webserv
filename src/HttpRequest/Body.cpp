#include "../../include/HttpRequest/Body.hpp"

const std::string& Body::get() const
{
    return body_;
}

void Body::set(const std::string& body)
{
    body_ = body;
}

int Body::parse(const char* buffer, int buf_len, const std::string& content_len)
{
    int i_cont_len;
    try {
        i_cont_len = std::stoi(content_len);
    } catch (std::invalid_argument& e) {
        return 0;
    }
    if (buf_len == 0 && body_.size() + buf_len != static_cast<size_t>(i_cont_len))
        throw ExceptionMalformedBody("Content-Length does not reflect body's length");

    body_ = body_.append(std::string(buffer, buf_len));
    return buf_len;
}
