#pragma once
#include "../IReader.hpp"
#include "Body.hpp"
#include "FieldLines.hpp"
#include "RequestLine.hpp"
#include <map>
#include <stdexcept>
#include <string>

enum HttpRequestState {
    RequestLineState,
    FieldLinesState,
    Done
};

// represents an HTTP 1.0 or 1.1 request
/* An HTTP request is composed of:
 * A request-line: GET /foo HTTP/1.1
 * Field lines: Host: example.com
 * And an optional message body followed by a new line '\r\n'
 * https://www.rfc-editor.org/rfc/rfc9112#name-field-syntax
 */
class HttpRequest {
public:
    HttpRequest();

    const RequestLine& getRequestLine() const;
    const FieldLines& getFieldLines() const;
    const Body& getBody() const;

    void setRequestLine(const std::string&, const std::string&, const std::string&);
    void setFieldLines(const std::string&, const std::string&);
    void setBody(const std::string&);

    void parseFromReader(IReader& reader);

private:
    HttpRequestState curr_state_;

    RequestLine request_line_;
    FieldLines field_lines_;
    Body body_;

    int parse_(const char* buffer, int length);
};
