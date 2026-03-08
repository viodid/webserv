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

bool HttpRequestParser::parseHeaderSection_(const std::string& buffer, size_t first_line_end,
                                            size_t header_end, HttpRequest& req)
{
	size_t headers_start = first_line_end + 2;
    std::string        headers_str = buffer.substr(headers_start, header_end - headers_start);
    std::istringstream stream(headers_str);
    std::string        line;

    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r")
            break;
        if (!parseHeader(line, req)) {
            req.state     = PARSE_BAD_REQUEST;
            req.error_msg = "Invalid header format";
            return false;
        }
    }
    return true;
}

bool HttpRequestParser::parseBody_(const std::string& body_data, bool is_complete,
                                   size_t max_body_size, HttpRequest& req)
{
	// Chunked body: divided
    if (req.hasHeader("transfer-encoding") &&
        toLower(req.getHeader("transfer-encoding")).find("chunked") != std::string::npos) {
        if (!parseChunkedBody(body_data, req, max_body_size)) {
            if (!is_complete)
                req.state = PARSE_INCOMPLETE;
            return false;
        }
        req.state = PARSE_SUCCESS;
        return true;
    }
	// Content-Length body: all at once
    if (req.hasHeader("content-length"))
        return parseContentLengthBody_(body_data, is_complete, max_body_size, req);
	// No body or unsupported Transfer-Encoding
    if (req.method == Location::POST || req.method == Location::PUT)
        req.body = body_data;
    req.state = PARSE_SUCCESS;
    return true;
}

// Parses the body when Content-Length is specified (all at once).
bool HttpRequestParser::parseContentLengthBody_(const std::string& body_data, bool is_complete,
                                                size_t max_body_size, HttpRequest& req)
{
    char* endptr;
    long  clen = std::strtol(req.getHeader("content-length").c_str(), &endptr, 10);
    if (*endptr != '\0' || clen < 0) {
        req.state     = PARSE_BAD_REQUEST;
        req.error_msg = "Invalid Content-Length";
        return false;
    }
    if (static_cast<size_t>(clen) > max_body_size) {
        req.state     = PARSE_ENTITY_TOO_LARGE;
        req.error_msg = "Body exceeds client_max_body_size";
        return false;
    }
    if (body_data.size() < static_cast<size_t>(clen)) {
        req.state     = is_complete ? PARSE_BAD_REQUEST : PARSE_INCOMPLETE;
        req.error_msg = is_complete ? "Incomplete body" : "";
        return false;
    }
    req.body  = body_data.substr(0, clen);
    req.state = PARSE_SUCCESS;
    return true;
}

// ==================== HttpRequestParser Private Methods ====================

bool HttpRequestParser::isValidVersion(const std::string& version)
{
    return version == "HTTP/1.0" || version == "HTTP/1.1";
}
