#include "../../include/HttpResponse/StatusLine.hpp"

StatusLine::StatusLine(const std::string& http_version, Location::ErrorPages status_code)
    : http_version_(http_version)
    , status_code_(status_code)
    , reason_phrase_(mapStatusCode(status_code))
{
}
