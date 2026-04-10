#include "../../include/HttpResponse/HttpResponse.hpp"

HttpResponse::HttpResponse(const StatusLine& status_line,
    const FieldLines& field_lines,
    const Body& body)
    : status_line_(status_line)
    , field_lines_(field_lines)
    , body_(body)
{
}
