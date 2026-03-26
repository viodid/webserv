#pragma once
#include <string>
#include <vector>

class HttpFieldLine {
public:
    HttpFieldLine(const std::string&, const std::string&);

private:
    const std::string& field_name_;
    const std::string& field_value_;
};

class HttpRequestLine {
public:
    HttpRequestLine(const std::string&, const std::string&, const std::string&);

private:
    const std::string& method_;
    const std::string& request_target_;
    const std::string& http_version_;
};

/*
 * Data holder class - it represents an HTTP 1.0 or 1.1 request
 */
class HttpRequest {
public:
    /* An HTTP request is composed of:
     * A request-line: GET /foo HTTP/1.1
     * Field lines: Host: example.com
     * And an optional message body followed by a new line '\r\n'
     * https://www.rfc-editor.org/rfc/rfc9112#name-field-syntax
     */
    HttpRequest(const HttpRequestLine&, const std::vector<HttpFieldLine>&, const std::string&);

private:
    const HttpRequestLine& request_line_;
    const std::vector<HttpFieldLine>& field_lines_;
    const std::string& body_;
};
