#pragma once
#include <string>
#include <vector>

class HttpFieldLine {
public:
    HttpFieldLine(const std::string&, const std::string&);

    const std::string& getFieldName() const;
    const std::string& getFieldValue() const;

private:
    std::string field_name_;
    std::string field_value_;
};

class HttpRequestLine {
public:
    HttpRequestLine(const std::string&, const std::string&, const std::string&);

    const std::string& getMethod() const;
    const std::string& getRequestTarget() const;
    const std::string& getHttpVersion() const;

private:
    std::string method_;
    std::string request_target_;
    std::string http_version_;
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

    const HttpRequestLine& getRequestLine() const;
    const std::vector<HttpFieldLine>& getFieldLines() const;
    const std::string& getBody() const;

private:
    HttpRequestLine request_line_;
    std::vector<HttpFieldLine> field_lines_;
    std::string body_;
};
