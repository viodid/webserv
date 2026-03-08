#include "../include/HttpRequestParser.hpp"
#include "../include/Utils.hpp"
#include <sstream>
#include <cstdlib>

// ==================== HttpFieldLine Methods ====================

HttpFieldLine::HttpFieldLine(const std::string& name, const std::string& value)
    : field_name_(name)
    , field_value_(value)
{
}

const std::string& HttpFieldLine::getFieldName() const  { return field_name_; }
const std::string& HttpFieldLine::getFieldValue() const { return field_value_; }

// ==================== HttpRequestLine Methods ====================

HttpRequestLine::HttpRequestLine(const std::string& method,
                                 const std::string& target,
                                 const std::string& version)
    : method_(method)
    , request_target_(target)
    , http_version_(version)
{
}

const std::string& HttpRequestLine::getMethod()        const { return method_; }
const std::string& HttpRequestLine::getRequestTarget() const { return request_target_; }
const std::string& HttpRequestLine::getHttpVersion()   const { return http_version_; }

