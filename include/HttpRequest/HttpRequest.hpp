#pragma once
#include "../Interfaces/IBodySink.hpp"
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
    // so the caller can inspect headers and install a body sink.
    void parseFromReader(IReader& reader);

    // Advance the parser using only bytes already in the internal buffer
    // (no recv). Useful after installing a sink to consume body bytes that
    // arrived in the same packet as the headers.
    void parseBuffered();

    bool isDone() const;

    // True after headers have been parsed but before body parsing has begun
    // and no sink has been installed yet.
    bool needsBodySink() const;

    HttpRequestParseState getState() const;

    // Transfers ownership of `sink` to the body. Must be called while
    // `needsBodySink()` is true (or never, in which case a default in-memory
    // sink is auto-installed before body parsing).
    void installBodySink(IBodySink* sink);

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
