#include "../include/HttpRequestParser.hpp"
#include <unordered_map>

HttpRequestParser::HttpRequestParser(const std::string& s)
    : stream_(s)
{
}

static HttpRequestLine parseRequestLine(std::string::const_iterator& it, size_t len);
static void parseFieldLine(std::unordered_map<std::string, std::string>& field_lines,
    std::string::const_iterator& it, size_t len);
static void parseBody(std::string& buf, std::string::const_iterator& it, size_t len);

/*
 * https://www.rfc-editor.org/rfc/rfc9112#name-message-parsing
 */
void HttpRequestParser::parse() const
{
    const std::string DELIMETER = "\r\n";
    const size_t END = stream_.find("\r\n\r\n");

    if (END == std::string::npos)
        throw std::runtime_error("registered nurse not present");

    size_t cursor = stream_.find(DELIMETER);
    HttpRequestLine request_line = parseRequestLine(stream_.begin(), cursor);
    std::unordered_map<std::string, std::string> field_lines;
    while (cursor < END) {
        size_t curr_delimeter = stream_.find(DELIMETER, cursor);
        parseFieldLine(field_lines, stream_.begin() + cursor, curr_delimeter);
        cursor = curr_delimeter;
    }
    // we've got a body to parse
    std::string body_buf;
    if (field_lines.find("content-lenght") != field_lines.end()) {
        parseBody(body_buf,
            stream_.begin() + cursor,
            field_lines.find("content-lenght"));
    }
}
