#include "../../include/HttpRequest/HttpRequest.hpp"

HttpRequest::HttpRequest()
    : start_time_(currTimeMs())
    , curr_state_(RequestLineState)
    , cursor_(0)
{
    buffer_.resize(Settings::PARSER_BUFFER_SIZE);
}
HttpRequest::~HttpRequest()
{
#if DEBUG
    std::cout << "HttpRequest destructor called\n";
#endif
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
    if (currTimeMs() - start_time_ > Settings::TIMEOUT_REQUEST_MS)
        throw ExceptionRequestTimeout("");

    if (buffer_.size() > Settings::PARSER_MAX_BUFFER_SIZE)
        throw ExceptionConnLenExceeded("overrun MAX_BUFFER_SIZE");

    int bytes_read = reader.read(buffer_.data() + cursor_,
        buffer_.size() - cursor_);

    cursor_ += bytes_read;

    if (cursor_ >= buffer_.size())
        buffer_.resize(buffer_.size() * 2);

    int bytes_parsed = parse_(buffer_.data(), cursor_);

    // remove already parsed content from buffer
    if (bytes_parsed) {
        int buffer_size = buffer_.size();
        buffer_.erase(buffer_.begin(), buffer_.begin() + bytes_parsed);
        buffer_.resize(buffer_size);
        cursor_ -= bytes_parsed;
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
                const std::string& content_length = field_lines_.get("content-length");
                if (!content_length.empty() && content_length != "0")
                    curr_state_ = BodyState;
                else
                    curr_state_ = Done;
            }
            break;
        }
        case BodyState: {
            int n = body_.parse(buffer + parsed, length - parsed,
                field_lines_.get("content-length"));

            if (n == 0) {
                keep = false;
                break;
            }

            parsed += n;
            curr_state_ = Done;
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

bool HttpRequest::isDone() const
{
    return curr_state_ == Done;
}
