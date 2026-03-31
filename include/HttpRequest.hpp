#pragma once
#include <map>
#include <string>

struct HttpRequestLine {
    std::string method;
    std::string request_target;
    std::string http_version;
};

struct HttpFieldLine {
    std::string field_name;
    std::string field_value;
};

// represents an HTTP 1.0 or 1.1 request
/* An HTTP request is composed of:
 * A request-line: GET /foo HTTP/1.1
 * Field lines: Host: example.com
 * And an optional message body followed by a new line '\r\n'
 * https://www.rfc-editor.org/rfc/rfc9112#name-field-syntax
 */
struct HttpRequest {
    HttpRequestLine request_line;
    std::map<std::string, std::string> field_lines;
    std::string body;
};
