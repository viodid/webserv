#include "../../include/HttpResponse/StatusLine.hpp"

StatusLine::StatusLine(const std::string& http_version, Location::StatusCodes status_code)
    : http_version_(http_version)
    , status_code_(status_code)
    , reason_phrase_(generateDefaultStatusMsg(status_code).first)
{
}
