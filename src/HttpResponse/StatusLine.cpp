#include "../../include/HttpResponse/StatusLine.hpp"
#include <sstream>

std::string StatusLine::format() const
{
    std::stringstream ss;
    ss << "HTTP/" << http_version_ << " " << status_code_
       << " " << reason_phrase_ << Settings::LINE_DELIMETER;
    return ss.str();
}
