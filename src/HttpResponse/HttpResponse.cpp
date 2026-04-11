#include "../../include/HttpResponse/HttpResponse.hpp"

HttpResponse::HttpResponse(const StatusLine& status_line,
    const FieldLines& field_lines,
    const Body& body)
    : status_line_(status_line)
    , field_lines_(field_lines)
    , body_(body)
{
}

std::string generateResponseStatusMsg(ResponseStatusCode status_code) {
    switch (status_code) {
        case R_200:
        return "OK";
        case R_201:
        return "Created";
        case R_301:
        return "Moved Permanently";
    }
}
