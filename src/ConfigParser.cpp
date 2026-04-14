#include "../include/ConfigParser.hpp"
#include "../include/Exceptions.hpp"
#include <cstdlib>
#include <iostream>
#include <cctype>
#include <cerrno>

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
        throw ExceptionParserError("ConfigParser: cannot open file: " + filepath_);
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
static bool isSingleCharToken(char c) {
    return c == '{' || c == '}' || c == ';';
}

std::string ConfigParser::nextToken()
{
    skipWhitespaceAndComments();
    if (pos_ >= content_.size()) // EOF
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
        throw ExceptionParserError(ss.str());
    }
}

// ==================== Directive helpers (server) ====================

void ConfigParser::skipUnknownDirective(const std::string& tok, const std::string& block)
{
    #ifdef DEBUG
        std::cerr << "ConfigParser: warning: unknown directive '" << tok
                  << "' in " << block << " block, skipping\n";
    #else
        (void)tok;
        (void)block;
    #endif
    std::string t;
    while (true) {
        t = nextToken();
        if (t.empty()) {
			break;
		}
        if (t == ";") {
			break;
		}
        if (t == "{") {
            int depth = 1;
            while (depth > 0) {
                t = nextToken();
                if (t.empty()) {
					break;
				}
                if (t == "{") {
					++depth;
				}
                else if (t == "}") {
					--depth;
				}
            }
            break;
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
	errno = 0;
    long val = std::strtol(size_str.c_str(), &endptr, 10);
    if (errno == ERANGE || *endptr != '\0' || val < 0)
        throw ExceptionParserError("ConfigParser: invalid client_max_body_size: " + size_str);
    size = static_cast<size_t>(val);
    expect(";");
}

void ConfigParser::parseStatusCode(std::vector<std::pair<Location::StatusCodes, std::string> >& pages)
{
    std::string code = nextToken();
    std::string path = nextToken();
    pages.push_back(std::make_pair(Location::statusCodeFromCode(code), path));
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
        throw ExceptionParserError("ConfigParser: autoindex must be 'on' or 'off', got: " + val);
    expect(";");
}

void ConfigParser::parseAllowedMethods(std::vector<Location::AllowedMethods>& methods)
{
    for (std::string m = peekToken(); m != ";" && m != "}" && !m.empty(); m = peekToken())
        methods.push_back(Location::methodFromString(nextToken()));
    expect(";");
    if (methods.empty())
        throw ExceptionParserError("ConfigParser: allowed_methods requires at least one method");
}

void ConfigParser::parseReturn(std::string& code, std::string& path)
{
    std::string first = nextToken();
	bool is_numeric = !first.empty();
    for (std::string::size_type i = 0; i < first.size() && is_numeric; ++i) {
    	if (!std::isdigit(static_cast<unsigned char>(first[i]))) {
        	is_numeric = false;
    	}
    }
	if (is_numeric) {
        code = first;
        path = nextToken();
    } else {
        path = first;
    }
    expect(";");
}

void ConfigParser::parseCgiPass(std::map<std::string, std::string>& cgi_map)
{
    std::string ext     = nextToken();
    std::string handler = nextToken();
    cgi_map[ext] = handler;
    expect(";");
}

// Main parse
Config ConfigParser::parse()
{
    readFile();
    pos_ = 0;
    std::vector<VirtualHost> virtual_hosts;
    skipWhitespaceAndComments();
    while (pos_ < content_.size()) {
        std::string tok = nextToken();
        if (tok.empty())
            break;
        if (tok != "server") {
            std::stringstream ss;
            ss << "ConfigParser: expected 'server' but got '" << tok
               << "' at position " << pos_;
            throw ExceptionParserError(ss.str());
        }
        virtual_hosts.push_back(parseServerBlock());
        skipWhitespaceAndComments();
    }
    if (virtual_hosts.empty())
        throw ExceptionParserError("ConfigParser: no server blocks found in " + filepath_);
    return Config(virtual_hosts);
}

// ==================== Server block ====================
VirtualHost ConfigParser::parseServerBlock()
{
    expect("{");

    std::string hostname;
    std::string port;
    size_t      client_max_body_size = 1048576; // 1 MiB default
    std::vector<std::pair<Location::StatusCodes, std::string> > status_codes;
    std::vector<Location> locations;

    for (std::string tok = peekToken(); tok != "}"; tok = peekToken()) {
        if (tok.empty())
            throw ExceptionParserError("ConfigParser: unexpected end of file inside server block");
        tok = nextToken();
        if (tok == "listen")
            parseListen(hostname, port);
        else if (tok == "client_max_body_size")
            parseClientMaxBodySize(client_max_body_size);
        else if (tok == "error_page")
            parseStatusCode(status_codes);
        else if (tok == "location")
            locations.push_back(parseLocationBlock());
        else
            skipUnknownDirective(tok, "server");
    }
    nextToken(); // consume '}'

    if (hostname.empty() || port.empty())
        throw ExceptionParserError("ConfigParser: server block missing 'listen' directive");

    char* endptr;
    long port_num = std::strtol(port.c_str(), &endptr, 10);
    if (*endptr != '\0' || port_num < 1 || port_num > 65535)
        throw ExceptionParserError("ConfigParser: invalid port number: " + port);

    return VirtualHost(hostname, port, client_max_body_size, status_codes, locations);
}

// ==================== Location block ====================

Location ConfigParser::parseLocationBlock()
{
    std::string path = nextToken();
    if (path.empty() || path == "{")
        throw ExceptionParserError("ConfigParser: location missing path");
    expect("{");

    std::string root;
    std::string default_file;
    std::string redirection_code;
    std::string redirection_path;
    bool        dir_listing = false;
    std::string upload_store;
    std::map<std::string, std::string> cgi_map;
    std::vector<Location::AllowedMethods> methods;

    for (std::string tok = peekToken(); tok != "}"; tok = peekToken()) {
        if (tok.empty())
            throw ExceptionParserError("ConfigParser: unexpected end of file inside location block");
        tok = nextToken();
        if (tok == "root")
            { root = nextToken(); expect(";"); }
        else if (tok == "index")
            { default_file = nextToken(); expect(";"); }
        else if (tok == "autoindex")
            parseAutoindex(dir_listing);
        else if (tok == "allowed_methods")
            parseAllowedMethods(methods);
        else if (tok == "return")
            parseReturn(redirection_code, redirection_path);
        else if (tok == "upload_store")
            { upload_store = nextToken(); expect(";"); }
        else if (tok == "cgi_pass")
            parseCgiPass(cgi_map);
        else
            skipUnknownDirective(tok, "location");
    }
    nextToken(); // consume '}'

	// If no allowed_methods specified, default to GET
    if (methods.empty())
        methods.push_back(Location::GET);
    // If POST is allowed, either cgi_pass or upload_store must be present
    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i] == Location::POST) {
            if (cgi_map.empty() && upload_store.empty()) {
                throw ExceptionParserError("ConfigParser: location with POST method must have 'cgi_pass' or 'upload_store'");
            }
            break;
        }
    }
    return Location(path, methods, redirection_code, redirection_path, root, default_file, dir_listing, upload_store, cgi_map);
}
