#include "../include/HttpRequest.hpp"

HttpRequestLine::HttpRequestLine(const std::string& method,
    const std::string& request_target,
    const std::string& http_v)
    : method_(method)
    , request_target_(request_target)
    , http_version_(http_v)
{
}

const std::string& HttpRequestLine::getMethod() const
{
    return method_;
}

const std::string& HttpRequestLine::getRequestTarget() const
{
    return request_target_;
}

const std::string& HttpRequestLine::getHttpVersion() const
{
    return http_version_;
}

HttpFieldLine::HttpFieldLine(const std::string& fn, const std::string& fv)
    : field_name_(fn)
    , field_value_(fv)
{
}

const std::string& HttpFieldLine::getFieldName() const
{
    return field_name_;
}

const std::string& HttpFieldLine::getFieldValue() const
{
    return field_value_;
}

HttpRequest::HttpRequest(const HttpRequestLine& request_line,
    const std::unordered_map<std::string, std::string>& field_lines,
    const std::string& body)
    : request_line_(request_line)
    , field_lines_(field_lines)
    , body_(body)
{
}

const HttpRequestLine& HttpRequest::getRequestLine() const
{
    return request_line_;
}

const std::unordered_map<std::string, std::string>& HttpRequest::getFieldNames() const
{
    return field_lines_;
}

const std::string& HttpRequest::getBody() const
{
    return body_;
}
