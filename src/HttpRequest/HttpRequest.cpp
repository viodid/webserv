#include "../../include/HttpRequest/HttpRequest.hpp"
#include <stdexcept>

static bool done(HttpRequestState state);

HttpRequest::HttpRequest()
{
    curr_state_ = RequestLineState;
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
    body_.set(body);
}

void HttpRequest::parseFromReader(IReader& reader)
{
    int read_idx = 0;
    int bytes_parsed = 0;
    std::vector<char> buffer;

    int buffer_size = Settings::PARSER_BUFFER_SIZE;
    buffer.resize(buffer_size);

    while (!done(curr_state_)) {

        if (buffer.size() > Settings::PARSER_MAX_BUFFER_SIZE)
            throw std::runtime_error("overrun MAX_BUFFER_SIZE");

        int bytes_read = reader.read(buffer.data() + read_idx,
            buffer.capacity() - read_idx);

        read_idx += bytes_read;

        if (bytes_read + read_idx >= buffer_size) {
            buffer_size *= 2;
            buffer.resize(buffer_size);
        }

        bytes_parsed = parse_(buffer.data(), read_idx);

        // remove already parsed content from buffer
        if (bytes_parsed) {
            buffer.erase(buffer.begin(), buffer.begin() + bytes_parsed);
            buffer.resize(buffer_size);
            read_idx -= bytes_parsed;
        }
    }
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
                if (!field_lines_.get("content-length").empty()) {
                    curr_state_ = BodyState;
                    keep = false;
                } else
                    curr_state_ = Done;
            }
            break;
        }
        case BodyState: {
            int n = body_.parse(buffer + parsed, length - parsed,
                field_lines_.get("content-length"));

            if (n == 0) {
                curr_state_ = Done;
                break;
            }

            parsed += n;
            keep = false;
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

static bool done(HttpRequestState state)
{
    return state == Done;
}
