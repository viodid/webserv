#pragma once
#include "../Interfaces/IReader.hpp"
#include "../Utils.hpp"
#include "Body.hpp"
#include "FieldLines.hpp"
#include "RequestLine.hpp"
#include <map>
#include <stdexcept>
#include <string>

enum HttpRequestParseState {
    RequestLineState,
    FieldLinesState,
    BodyState,
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
    ~HttpRequest();

    const RequestLine& getRequestLine() const;
    const FieldLines& getFieldLines() const;
    const Body& getBody() const;

    void setRequestLine(const std::string&, const std::string&, const std::string&);
    void setFieldLines(const std::string&, const std::string&);
    void setBody(const std::string&);

    void parseFromReader(IReader& reader);

    bool isDone() const;

private:
    // Request state
    RequestLine request_line_;
    FieldLines field_lines_;
    Body body_;

    // Parsing state
    size_t start_time_;
    HttpRequestParseState curr_state_;
    size_t cursor_;
    std::vector<char> buffer_;

    int parse_(const char* buffer, int length);
};
