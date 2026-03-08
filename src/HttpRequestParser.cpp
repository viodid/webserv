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

// ==================== HttpRequest Methods ====================

// Returns the value of the header field with the given name (case-insensitive).
// Example: getHeader("Content-Type") returns "text/html" if the request has a header "Content-Type: text/html".
std::string HttpRequest::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(toLower(key));
    if (it != headers.end())
        return it->second;
    return "";
}

bool HttpRequest::hasHeader(const std::string& key) const
{
    return headers.find(toLower(key)) != headers.end();
}

// ==================== HttpRequestParser Public Methods ====================

HttpRequest HttpRequestParser::parse(const std::string& raw_request, size_t max_body_size)
{
    return parseIncremental(raw_request, true, max_body_size);
}

HttpRequest HttpRequestParser::parseIncremental(const std::string& buffer, bool is_complete,
                                                size_t max_body_size)
{
    HttpRequest req;

    if (buffer.empty()) {
        req.state = PARSE_INCOMPLETE;
        return req;
    }

    size_t header_end = buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        req.state     = is_complete ? PARSE_BAD_REQUEST : PARSE_INCOMPLETE;
        req.error_msg = is_complete ? "Missing header terminator" : "";
        return req;
    }

    size_t first_line_end = buffer.find("\r\n");
    if (!parseRequestLine(buffer.substr(0, first_line_end), req))
        return req;

    if (header_end > MAX_HEADER_SIZE) {
        req.state     = PARSE_ENTITY_TOO_LARGE;
        req.error_msg = "Headers too large";
        return req;
    }

    if (!parseHeaderSection_(buffer, first_line_end, header_end, req))
        return req;

    size_t body_start = header_end + 4; // 4 accounts for the length of "\r\n\r\n"
    if (body_start < buffer.size()) // There's body data after the headers
        parseBody_(buffer.substr(body_start), is_complete, max_body_size, req);
    else
        req.state = PARSE_SUCCESS;
    return req;
}

// ==================== HttpRequestParser Private Methods ====================

bool HttpRequestParser::isValidVersion(const std::string& version)
{
    return version == "HTTP/1.0" || version == "HTTP/1.1";
}
