#pragma once
#include <string>
#include <vector>

class HttpRequestLine {
    /*
     * Composed of:
     * mehthod
     * request-target
     * http-version
    */
public:
    HttpRequestLine(const std::string&, const std::string&, const std::string&);
};

class HttpFieldLine {
    /*
     * Composed of:
     * field-name
     * field-value
    */
public:
    HttpFieldLine(const std::string&, const std::string&);
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
    HttpRequest(const HttpFieldLine&, const std::vector<HttpFieldLine>&, const std::string&);
};
