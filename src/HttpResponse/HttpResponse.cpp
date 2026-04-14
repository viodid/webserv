#include "../../include/HttpResponse/HttpResponse.hpp"

HttpResponse::HttpResponse(const StatusLine& status_line,
    const FieldLines& field_lines,
    const Body& body)
    : status_line_(status_line)
    , field_lines_(field_lines)
    , body_(body)
{
}

std::string HttpResponse::format() const
{
    std::stringstream ss;
    ss << status_line_.format() << field_lines_.format()
       << Settings::LINE_DELIMETER << body_.format();
    return ss.str();
}
