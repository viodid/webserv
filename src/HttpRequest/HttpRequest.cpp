#include "../../include/HttpRequest/HttpRequest.hpp"
#include <cstdlib>

HttpRequest::HttpRequest()
    : start_time_(currTimeMs())
    , curr_state_(RequestLineState)
    , cursor_(0)
{
    buffer_.resize(Settings::PARSER_BUFFER_SIZE);
}
HttpRequest::~HttpRequest()
{
}
const RequestLine& HttpRequest::getRequestLine() const
{
    return request_line_;
}
const FieldLines& HttpRequest::getFieldLines() const
{
    return field_lines_;
}
const Body& HttpRequest::getBody() const
{
    return body_;
}

void HttpRequest::setRequestLine(const std::string& method,
    const std::string& target,
    const std::string& http_version)
{
    request_line_.setMethod(method);
    request_line_.setRequestTarget(target);
    request_line_.setHttpVersion(http_version);
}
void HttpRequest::setFieldLines(const std::string& field_name,
    const std::string& field_value)
{
    field_lines_.set(field_name, field_value);
}
void HttpRequest::setBody(const std::string& body)
{
    body_.parse(body.data(), body.size());
}

void HttpRequest::parseFromReader(IReader& reader)
{
    // Header phase: tight idle timeout (defense against slowloris). Body
    // phase: generous idle timeout so long uploads (and clients that wait on
    // 100-Continue before sending) can complete.
    unsigned long timeout_ms = (curr_state_ == BodyState)
        ? Settings::TIMEOUT_BODY_MS
        : Settings::TIMEOUT_REQUEST_MS;
    if (currTimeMs() - start_time_ > timeout_ms)
        throw ExceptionRequestTimeout("request timeout");

    // Header phase only: cap the parse buffer so a slow client can't pin
    // arbitrary memory before sending CRLF-CRLF. Once body parsing begins,
    // bytes are drained as they arrive and the read buffer stays small.
    if (curr_state_ != BodyState
        && buffer_.size() > Settings::PARSER_MAX_BUFFER_SIZE)
        throw ExceptionConnLenExceeded("overrun MAX_BUFFER_SIZE");

    int bytes_read = reader.read(buffer_.data() + cursor_,
        buffer_.size() - cursor_);

    if (bytes_read > 0)
        start_time_ = currTimeMs();

    cursor_ += bytes_read;

    if (curr_state_ != BodyState && cursor_ >= buffer_.size())
        buffer_.resize(buffer_.size() * 2);

    parseBuffered();
}

void HttpRequest::parseBuffered()
{
    int bytes_parsed = parse_(buffer_.data(), cursor_);

    if (bytes_parsed) {
        int buffer_size = buffer_.size();
        buffer_.erase(buffer_.begin(), buffer_.begin() + bytes_parsed);
        buffer_.resize(buffer_size);
        cursor_ -= bytes_parsed;
    }
}

void HttpRequest::resolveFraming_()
{
    const std::string& cl = field_lines_.get("content-length");
    const std::string& te = field_lines_.get("transfer-encoding");

    if (!cl.empty() && !te.empty())
        throw ExceptionBadFraming("both Content-Length and Transfer-Encoding present");

    if (!te.empty()) {
        if (te != "chunked")
            throw ExceptionBadFraming("unsupported Transfer-Encoding");
        body_.setChunked();
        curr_state_ = HeadersDoneState;
        return;
    }

    if (!cl.empty()) {
        char* end = NULL;
        unsigned long n = std::strtoul(cl.c_str(), &end, 10);
        if (end == cl.c_str() || *end != '\0')
            throw ExceptionBodyLength("malformed 'Content-Length' header");
        if (n == 0) {
            curr_state_ = Done;
            return;
        }
        body_.setContentLength(n);
        curr_state_ = HeadersDoneState;
        return;
    }

    curr_state_ = Done;
}

void HttpRequest::configureBody(const std::string& file_path, size_t max_bytes)
{
    if (curr_state_ != HeadersDoneState)
        return;
    body_.setMaxBytes(max_bytes);
    if (!file_path.empty())
        body_.useFile(file_path);
    curr_state_ = BodyState;
}

bool HttpRequest::needsBodyConfig() const
{
    return curr_state_ == HeadersDoneState;
}

HttpRequestParseState HttpRequest::getState() const
{
    return curr_state_;
}

/*
 * https://www.rfc-editor.org/rfc/rfc9112#name-message-parsing
 */
int HttpRequest::parse_(const char* buffer, int length)
{
    int parsed = 0;
    bool keep = true;

    while (keep) {
        switch (curr_state_) {
        case RequestLineState: {
            int n = request_line_.parse(buffer + parsed, length - parsed);
            if (n == 0) {
                keep = false;
                break;
            }
            curr_state_ = FieldLinesState;
            parsed += n;
            break;
        }
        case FieldLinesState: {
            int n = field_lines_.parse(buffer + parsed, length - parsed);
            if (n == 0) {
                keep = false;
                break;
            }
            parsed += n;
            if (n == 2) {
                resolveFraming_();
                // Pause so the caller can configure the body destination.
                keep = false;
            }
            break;
        }
        case HeadersDoneState: {
            // No explicit configureBody() call: default to in-memory,
            // unlimited. Keeps direct-use callers (tests) working without
            // ceremony.
            curr_state_ = BodyState;
            break;
        }
        case BodyState: {
            int n = body_.parse(buffer + parsed, length - parsed);
            parsed += n;
            if (body_.isComplete()) {
                curr_state_ = Done;
                keep = false;
                break;
            }
            if (n == 0) {
                keep = false;
                break;
            }
            break;
        }
        case Done: {
            keep = false;
            break;
        }
        }
    }
    return parsed;
}

bool HttpRequest::isDone() const
{
    return curr_state_ == Done;
}
