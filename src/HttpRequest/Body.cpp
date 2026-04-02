#include "../../include/HttpRequest/Body.hpp"

const std::string& Body::get() const
{
    return body_;
}

void Body::set(const std::string& body)
{
    body_ = body;
}

int Body::parse(const char* buffer, int length)
{
    body_.append(std::string(buffer, length));
    return length;
}
