#include "../../include/HttpResponse/StatusLine.hpp"

static std::string mapReasonPhrase(Location::ErrorPages status_code);

StatusLine::StatusLine(const std::string& http_version, Location::ErrorPages status_code)
    : http_version_(http_version)
    , status_code_(status_code)
    , reason_phrase_(mapReasonPhrase(status_code))
{
}
