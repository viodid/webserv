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
    HeadersDoneState,
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

    // Recv from `reader` and advance the parser. Pauses at HeadersDoneState
    // so the caller can configure the body destination.
    void parseFromReader(IReader& reader);

    // Advance the parser using only bytes already in the internal buffer
    // (no recv). Used after configureBody() to consume body bytes that
    // arrived alongside the headers.
    void parseBuffered();

    bool isDone() const;

    // True after headers are parsed and the body destination has not been
    // configured yet.
    bool needsBodyConfig() const;

    HttpRequestParseState getState() const;

    // Configure the body destination. file_path == "" means in-memory; non-
    // empty means stream the decoded body into that file. max_bytes == 0
    // means unlimited. Transitions the parser into the body phase.
    void configureBody(const std::string& file_path, size_t max_bytes);

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
    void resolveFraming_();

    HttpRequest(const HttpRequest&);
    HttpRequest& operator=(const HttpRequest&);
};
