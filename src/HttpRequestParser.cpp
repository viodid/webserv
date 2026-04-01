#include "../include/HttpRequestParser.hpp"
#include <stdexcept>

HttpRequestParser::HttpRequestParser(IReader& reader)
    : reader_(reader)
{
    current_state_ = HttpRequestParser::RequestLine;
}

const HttpRequest HttpRequestParser::getRequest() const
{
    return request_;
}

void HttpRequestParser::parseFromReader()
{
    int read_idx = 0;
    int bytes_parsed = 0;
    std::vector<char> buffer;

    int buffer_size = Settings::PARSER_BUFFER_SIZE;
    buffer.resize(buffer_size);

    while (!done()) {

        if (buffer.size() > Settings::PARSER_MAX_BUFFER_SIZE)
            throw std::runtime_error("overrun MAX_BUFFER_SIZE");

        int bytes_read = reader_.read(buffer.data() + read_idx,
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
int HttpRequestParser::parse_(const char* buffer, int length)
{
    int parsed = 0;
    bool keep = true;

    while (keep) {
        switch (current_state_) {
        case HttpRequestParser::RequestLine: {
            int n = parseRequestLine_(buffer + parsed, length - parsed);
            if (n == 0) {
                keep = false;
                break;
            }
            current_state_ = HttpRequestParser::FieldLine;
            parsed += n;
            break;
        }
        case HttpRequestParser::FieldLine: {
            int n = parseFieldLine_(buffer + parsed, length - parsed);
            if (n == 0) {
                keep = false;
                break;
            }
            parsed += n;
            if (n == 2)
                current_state_ = HttpRequestParser::Done;
            break;
        }
        case HttpRequestParser::Done: {
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
    size_t delimeter_pos = str_stream.find(Settings::LINE_DELIMETER);

    if (delimeter_pos == std::string::npos)
        return 0;

    // request line parts break up
    size_t cursor = 0;
    std::vector<std::string> parts;
    while (parts.size() < 3) {

        if (parts.size() == 2)
            FIELD_DELIMETER = Settings::LINE_DELIMETER;

        size_t curr_deli = str_stream.find(FIELD_DELIMETER, cursor);
        if (curr_deli > delimeter_pos)
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

/*
 * field-line   = field-name ":" OWS field-value OWS
 *
 * Host: localhost:5555
 * User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:148.0)
 *
 * https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax
 */
int HttpRequestParser::parseFieldLine_(const char* buffer, int length)
{
    std::string FIELD_DELIMETER = ":";
    const std::string str_stream(buffer, length);
    size_t delimeter_pos = str_stream.find(Settings::LINE_DELIMETER);

    if (delimeter_pos == 0)
        return 2;

    if (delimeter_pos == std::string::npos)
        return 0;

    std::vector<std::string> parts;
    // field-name
    size_t fl_delimeter = str_stream.find(FIELD_DELIMETER);
    if (fl_delimeter > delimeter_pos)
        throw ExceptionMalformedFieldLine("delimeter ':' not found");
    for (size_t i = 0; i < fl_delimeter; i++) {
        if (str_stream[i] == ' ')
            throw ExceptionMalformedFieldLine("malformed key header");
    }
    parts.push_back(str_stream.substr(0, fl_delimeter));

    // field-value
    std::string value = str_stream.substr(fl_delimeter + 1);
    size_t i;
    for (i = 0; value[i] == ' ' && i < value.size(); i++) { }
    if (i == value.size())
        throw ExceptionMalformedFieldLine("header value not found");
    parts.push_back(value.substr(i, value.find(Settings::LINE_DELIMETER) - i));

    request_.field_lines[parts[0]] = parts[1];

    return delimeter_pos + std::string(Settings::LINE_DELIMETER).size();
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
    return current_state_ == HttpRequestParser::Done;
}
