#include "../../include/HttpRequest/RequestLine.hpp"

static bool isSupportedHTTPVersion(const std::string& str);

const std::string& RequestLine::getMethod() const
{
    return method_;
}
const std::string& RequestLine::getRequestTarget() const
{
    return request_target_;
}
const std::string& RequestLine::getHttpVersion() const
{
    return http_version_;
}

void RequestLine::setMethod(const std::string& method)
{
    method_ = method;
}

void RequestLine::setRequestTarget(const std::string& rt)
{
    request_target_ = rt;
}
void RequestLine::setHttpVersion(const std::string& http_v)
{
    http_version_ = http_v;
}

int RequestLine::parse(const char* buffer, int length)
{
    std::string FIELD_DELIMETER = " ";
    const std::string str_stream(buffer, length);
    size_t delimeter_pos = str_stream.find(Settings::LINE_DELIMETER);

    if (delimeter_pos == std::string::npos)
        return 0;

    // request line parts break up
    size_t cursor = 0;
    std::vector<std::string> parts;
    while (parts.size() < 3) {

        if (parts.size() == 2)
            FIELD_DELIMETER = Settings::LINE_DELIMETER;

        size_t curr_deli = str_stream.find(FIELD_DELIMETER, cursor);
        if (curr_deli > delimeter_pos)
            throw ExceptionMalformedRequestLine("delimeter not found");

        parts.push_back(str_stream.substr(cursor, curr_deli - cursor));
        cursor = curr_deli + FIELD_DELIMETER.size();
    }

    // check METHOD
    for (std::string::iterator it = parts[0].begin(); it != parts[0].end(); it++) {
        if (!std::isupper(*it) || !std::isalpha(*it))
            throw ExceptionMalformedRequestLine("method is not uppercase or alphabetical");
    }

    if (parts[2].size() != 8 || !isSupportedHTTPVersion(parts[2]))
        throw ExceptionMalformedRequestLine("HTTP version unsupported");

    parts[2] = parts[2].substr(5);
    method_ = parts[0];
    request_target_ = parts[1];
    http_version_ = parts[2];

    return cursor;
}

static bool isSupportedHTTPVersion(const std::string& str)
{
    std::set<std::string> supported_versions;
    supported_versions.insert("HTTP/1.0");
    supported_versions.insert("HTTP/1.1");
    return supported_versions.find(str) != supported_versions.end();
}
