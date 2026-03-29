#include "../include/HttpRequestParser.hpp"

static const char DELIMETER[] = "\r\n";
static const char END_MESSAGE[] = "\r\n\r\n";

HttpRequestParser::HttpRequestParser(const std::string& s)
    : stream_(s)
{
}

/*
 * https://www.rfc-editor.org/rfc/rfc9112#name-message-parsing
 */
HttpRequest HttpRequestParser::parse()
{
    HttpRequestLine request_line = parseRequestLine_();
    std::map<std::string, std::string> field_lines;
    // while (cursor < END) {
    //     size_t curr_delimeter = stream_.find(DELIMETER, cursor);
    //     parseFieldLine(field_lines, stream_.begin() + cursor, curr_delimeter);
    //     cursor = curr_delimeter;
    // }

    // we've got a body to parse
    // std::string body;
    // if (field_lines.find("content-lenght") != field_lines.end()) {
    //     parseBody(body,
    //         stream_.begin() + cursor,
    //         field_lines.find("content-lenght"));
    // }
    return HttpRequest(request_line, field_lines, "");
}

/*
 * request-line = method SP request-target SP HTTP-version
 * GET /foo HTTP/1.1
 * https://www.rfc-editor.org/rfc/rfc9112#name-request-line
 */
// TODO: primeagen 1:04:00
static bool isSupportedHTTPVersion(const std::string& str);
HttpRequestLine HttpRequestParser::parseRequestLine_()
{
    std::string FIELD_DELIMETER = " ";
    size_t delimeter_pos = stream_.find(DELIMETER);
    if (delimeter_pos == std::string::npos)
        throw ExceptionMalformedRequestLine("delimeter not found");
    size_t cursor = 0;
    std::vector<std::string> parts;
    while (parts.size() < 3) {
        if (parts.size() == 2)
            FIELD_DELIMETER = DELIMETER;
        size_t curr_deli = stream_.find(FIELD_DELIMETER, cursor);
        if (curr_deli == std::string::npos)
            throw ExceptionMalformedRequestLine("space character delimeter not found");
        parts.push_back(stream_.substr(cursor, curr_deli - cursor));
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
    stream_ = stream_.substr(cursor);
    return HttpRequestLine(parts[0], parts[1], parts[2]);
}

static bool isSupportedHTTPVersion(const std::string& str)
{
    std::set<std::string> supported_versions;
    supported_versions.insert("HTTP/1.0");
    supported_versions.insert("HTTP/1.1");
    return supported_versions.find(str) != supported_versions.end();
}
