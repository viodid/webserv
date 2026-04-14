#include "../../include/HttpResponse/StatusLine.hpp"
#include <sstream>

StatusLine::StatusLine(const std::string& http_version, Location::StatusCodes status_code)
    : http_version_(http_version)
    , status_code_(status_code)
    , reason_phrase_(generateDefaultStatusMsg(status_code).first)
{
}

std::string StatusLine::format() const
{
    std::stringstream ss;
    ss << "HTTP/" << http_version_ << " " << status_code_
       << " " << reason_phrase_ << Settings::LINE_DELIMETER;
    return ss.str();
}
