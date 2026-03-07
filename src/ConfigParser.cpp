#include "../include/ConfigParser.hpp"
#include <cstdlib>
#include <iostream>

// Constructor
ConfigParser::ConfigParser(const std::string& filepath)
    : filepath_(filepath)
    , pos_(0)
{
}

// Read File
void ConfigParser::readFile()
{
    std::ifstream file(filepath_.c_str());
    if (!file.is_open())
        throw std::runtime_error("ConfigParser: cannot open file: " + filepath_);
    std::stringstream ss;
    ss << file.rdbuf();
    content_ = ss.str();
    file.close();
}

// Skip Whitespace and comments
void ConfigParser::skipWhitespaceAndComments()
{
    while (pos_ < content_.size()) {
        // Skip whitespace
        if (std::isspace(static_cast<unsigned char>(content_[pos_]))) {
            ++pos_;
            continue;
        }
        // Skip line comments starting with #
        if (content_[pos_] == '#') {
            while (pos_ < content_.size() && content_[pos_] != '\n')
                ++pos_;
            continue;
        }
        break;
    }
}

// Tokenization
bool isSingleCharToken(char c) {
    return c == '{' || c == '}' || c == ';';
}

std::string ConfigParser::nextToken()
{
    skipWhitespaceAndComments();
    if (pos_ >= content_.size())
        return "";

    // Single-character tokens: { } ;
    char c = content_[pos_];
    if (isSingleCharToken(c)) {
        ++pos_;
        return std::string(1, c);
    }

    // Word token (everything until whitespace or special char)
    size_t start = pos_;
    while (pos_ < content_.size()) {
        char ch = content_[pos_];
        if (std::isspace(static_cast<unsigned char>(ch))
            || isSingleCharToken(ch) || ch == '#')
            break;
        ++pos_;
    }
    return content_.substr(start, pos_ - start);
}

std::string ConfigParser::peekToken()
{
    size_t saved = pos_;
    std::string tok = nextToken();
    pos_ = saved;
    return tok;
}

void ConfigParser::expect(const std::string& expected)
{
    std::string tok = nextToken();
    if (tok != expected) {
        std::stringstream ss;
        ss << "ConfigParser: expected '" << expected
           << "' but got '" << tok << "' at position " << pos_;
        throw std::runtime_error(ss.str());
    }
}

// ==================== Directive helpers (server) ====================

void ConfigParser::skipUnknownDirective(const std::string& tok, const std::string& block)
{
    std::cerr << "ConfigParser: warning: unknown directive '" << tok
              << "' in " << block << " block, skipping\n";
    if (peekToken() == "{") {
        nextToken();
        int depth = 1;
        while (depth > 0) {
            std::string t = nextToken();
            if (t.empty()) break;
            if (t == "{") ++depth;
            else if (t == "}") --depth;
        }
    } else {
		std::string peek = peekToken();
		while (!peek.empty() && peek != "}") {
			std::string tok = nextToken();
			if (tok == ";")
				break;
			peek = peekToken();
		}
    }
}

void ConfigParser::parseListen(std::string& hostname, std::string& port)
{
    std::string addr = nextToken();
    size_t colon = addr.rfind(':');
    if (colon == std::string::npos) { // not found
        hostname = "0.0.0.0";
        port = addr;
    } else {
        hostname = addr.substr(0, colon);
        port = addr.substr(colon + 1);
    }
    expect(";");
}

void ConfigParser::parseClientMaxBodySize(size_t& size)
{
    std::string size_str = nextToken();
    char* endptr;
    long val = std::strtol(size_str.c_str(), &endptr, 10);
    if (*endptr != '\0' || val < 0)
        throw std::runtime_error("ConfigParser: invalid client_max_body_size: " + size_str);
    size = static_cast<size_t>(val);
    expect(";");
}

void ConfigParser::parseErrorPage(std::vector<std::pair<Location::ErrorPages, std::string> >& pages)
{
    std::string code = nextToken();
    std::string path = nextToken();
    pages.push_back(std::make_pair(Location::errorPageFromCode(code), path));
    expect(";");
}

// ==================== Directive helpers (Location) ====================

void ConfigParser::parseAutoindex(bool& dir_listing)
{
    std::string val = nextToken();
    if (val == "on")
        dir_listing = true;
    else if (val == "off")
        dir_listing = false;
    else
        throw std::runtime_error("ConfigParser: autoindex must be 'on' or 'off', got: " + val);
    expect(";");
}

void ConfigParser::parseAllowedMethods(std::vector<Location::AllowedMethods>& methods)
{
    for (std::string m = peekToken(); m != ";" && !m.empty(); m = peekToken())
        methods.push_back(Location::methodFromString(nextToken()));
    expect(";");
    if (methods.empty())
        throw std::runtime_error("ConfigParser: allowed_methods requires at least one method");
}

void ConfigParser::parseReturn(std::string& code, std::string& path)
{
    std::string first = nextToken();
    if (!first.empty() && std::isdigit(static_cast<unsigned char>(first[0]))) {
        code = first;
        path = nextToken();
    } else {
        path = first;
    }
    expect(";");
}

void ConfigParser::parseCgiPass(std::map<std::string, std::string>& cgi_map)
{
    cgi_map[nextToken()] = nextToken();
    expect(";");
}
