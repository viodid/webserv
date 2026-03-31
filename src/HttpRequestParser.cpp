#include "../include/HttpRequestParser.hpp"
#include <stdexcept>

static const char DELIMETER[] = "\r\n";
static const char END_MESSAGE[] = "\r\n\r\n";

HttpRequestParser::HttpRequestParser(IReader& reader)
    : reader_(reader)
{
    current_state_ = HttpRequestParser::RequestState::INIT;
}

const HttpRequest HttpRequestParser::getRequest() const
{
    return request_;
}

void HttpRequestParser::parseFromReader()
{
    int read_idx = 0;
    std::vector<char> buffer;

    int buffer_size = BUFFER_SIZE;
    buffer.resize(buffer_size);

    while (!done()) {

        if (buffer.size() > MAX_BUFFER_SIZE)
            throw std::runtime_error("overrun MAX_BUFFER_SIZE");

        int bytes_read = reader_.read(buffer.data() + read_idx,
            buffer.capacity() - read_idx);

        read_idx += bytes_read;

        if (bytes_read >= buffer_size) {
            buffer_size *= 2;
            buffer.resize(buffer_size);
        }

        int bytes_parsed = parse_(buffer.data(), read_idx);

        // remove already parsed content from buffer
        if (bytes_parsed) {
            buffer.erase(buffer.begin(), buffer.begin() + bytes_parsed);
            buffer.resize(buffer_size);
        }
    }
}

/*
 * https://www.rfc-editor.org/rfc/rfc9112#name-message-parsing
 */
int HttpRequestParser::parse_(const char* buffer, int length)
{
    int parsed = 0;
    bool keep = true;

    while (keep) {
        switch (current_state_) {
        case HttpRequestParser::RequestState::INIT: {
            int n = parseRequestLine_(buffer + parsed, length - parsed);
            if (n == 0) {
                keep = false;
                break;
            }
            current_state_ = HttpRequestParser::RequestState::DONE;
            parsed += n;
            break;
        }
        case HttpRequestParser::RequestState::DONE: {
            keep = false;
            break;
        }
        }
    }
    return parsed;
}

/*
 * request-line = method SP request-target SP HTTP-version
 * GET /foo HTTP/1.1
 * https://www.rfc-editor.org/rfc/rfc9112#name-request-line
 */
static bool isSupportedHTTPVersion(const std::string& str);

int HttpRequestParser::parseRequestLine_(const char* buffer, int length)
{
    std::string FIELD_DELIMETER = " ";
    const std::string str_stream(buffer, length);
    size_t delimeter_pos = str_stream.find(DELIMETER);

    if (delimeter_pos == std::string::npos)
        return 0;

    // request line parts break up
    size_t cursor = 0;
    std::vector<std::string> parts;
    while (parts.size() < 3) {

        if (parts.size() == 2)
            FIELD_DELIMETER = DELIMETER;

        size_t curr_deli = str_stream.find(FIELD_DELIMETER, cursor);
        if (curr_deli == std::string::npos)
            throw ExceptionMalformedRequestLine("delimeter not found");

        parts.push_back(str_stream.substr(cursor, curr_deli - cursor));
        cursor = curr_deli + FIELD_DELIMETER.size();
    }

    // check METHOD
    for (std::string::iterator it = parts[0].begin(); it != parts[0].end(); it++) {
        if (!std::isupper(*it) || !std::isalpha(*it))
            throw ExceptionMalformedRequestLine("method is not uppercase or alphabetical");
    }

    if (parts[2].size() != 8 || !isSupportedHTTPVersion(parts[2]))
        throw ExceptionMalformedRequestLine("HTTP version unsopported");

    parts[2] = parts[2].substr(5);
    request_.request_line.method = parts[0];
    request_.request_line.request_target = parts[1];
    request_.request_line.http_version = parts[2];

    return cursor;
}

static bool isSupportedHTTPVersion(const std::string& str)
{
    std::set<std::string> supported_versions;
    supported_versions.insert("HTTP/1.0");
    supported_versions.insert("HTTP/1.1");
    return supported_versions.find(str) != supported_versions.end();
}

bool HttpRequestParser::done() const
{
    return current_state_ == HttpRequestParser::RequestState::DONE;
}
