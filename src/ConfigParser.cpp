#include "../include/ConfigParser.hpp"
#include "../include/Exceptions.hpp"
#include <cstdlib>
#include <iostream>
#include <cctype>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>

// ==================== Validation helpers ====================

static void validateIPv4(const std::string& host)
{
    size_t start = 0;
    int parts = 0;
    while (start <= host.size()) {
        size_t dot = host.find('.', start);
        std::string part = host.substr(start, (dot == std::string::npos ? host.size() : dot) - start);
        if (part.empty() || part.size() > 3)
            throw ExceptionParserError("ConfigParser: invalid IPv4 address: " + host);
        for (size_t i = 0; i < part.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(part[i])))
                throw ExceptionParserError("ConfigParser: invalid IPv4 address: " + host);
        }
        if (part.size() > 1 && part[0] == '0')
            throw ExceptionParserError("ConfigParser: invalid IPv4 address: " + host);
        int val = std::atoi(part.c_str());
        if (val < 0 || val > 255)
            throw ExceptionParserError("ConfigParser: invalid IPv4 address: " + host);
        ++parts;
        if (dot == std::string::npos)
            break;
        start = dot + 1;
    }
    if (parts != 4)
        throw ExceptionParserError("ConfigParser: invalid IPv4 address: " + host);
}

static void validatePathReadable(const std::string& path, const std::string& directive)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
        throw ExceptionParserError("ConfigParser: " + directive + ": file not found: " + path);
    if (access(path.c_str(), R_OK) != 0)
        throw ExceptionParserError("ConfigParser: " + directive + ": file not readable: " + path);
}

static void validateDirReadable(const std::string& path, const std::string& directive)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
        throw ExceptionParserError("ConfigParser: " + directive + ": directory not found: " + path);
    if (access(path.c_str(), R_OK) != 0)
        throw ExceptionParserError("ConfigParser: " + directive + ": directory not readable: " + path);
}

static void validateDirWritable(const std::string& path, const std::string& directive)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
        throw ExceptionParserError("ConfigParser: " + directive + ": directory not found: " + path);
    if (access(path.c_str(), W_OK) != 0)
        throw ExceptionParserError("ConfigParser: " + directive + ": directory not writable: " + path);
}

static void validateExecutable(const std::string& path, const std::string& directive)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
        throw ExceptionParserError("ConfigParser: " + directive + ": file not found: " + path);
    if (access(path.c_str(), X_OK) != 0)
        throw ExceptionParserError("ConfigParser: " + directive + ": not executable: " + path);
}

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
    validateIPv4(hostname);
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
    validatePathReadable(path, "error_page");
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

static void validateReturnPath(const std::string& path)
{
    if (path.empty() || path == ";" || path == "}")
        throw ExceptionParserError("ConfigParser: 'return' directive requires at least a path");
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
        validateReturnPath(path);
    } else {
        path = first;
        validateReturnPath(path);
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
    for (size_t i = 0; i < virtual_hosts.size(); ++i) {
        for (size_t j = i + 1; j < virtual_hosts.size(); ++j) {
            if (virtual_hosts[i].getPort() == virtual_hosts[j].getPort())
                throw ExceptionParserError(
                    "ConfigParser: duplicate port across server blocks: " + virtual_hosts[i].getPort());
        }
    }
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
    bool        listen_set = false;

    for (std::string tok = peekToken(); tok != "}"; tok = peekToken()) {
        if (tok.empty())
            throw ExceptionParserError("ConfigParser: unexpected end of file inside server block");
        tok = nextToken();
        if (tok == "listen") {
            if (listen_set)
                throw ExceptionParserError("ConfigParser: duplicate 'listen' directive in server block");
            parseListen(hostname, port);
            listen_set = true;
        }
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

    bool root_set = false;
    bool index_set = false;
    bool autoindex_set = false;
    bool methods_set = false;
    bool upload_store_set = false;
    bool cgi_map_set = false;
    bool return_set = false;

    for (std::string tok = peekToken(); tok != "}"; tok = peekToken()) {
        if (tok.empty())
            throw ExceptionParserError("ConfigParser: unexpected end of file inside location block");
        tok = nextToken();
        if (tok == "root") {
            root = nextToken();
            expect(";");
            root_set = true;
        }
        else if (tok == "index") {
            default_file = nextToken();
            expect(";");
            index_set = true;
        }
        else if (tok == "autoindex") {
            parseAutoindex(dir_listing);
            autoindex_set = true;
        }
        else if (tok == "allowed_methods") {
            parseAllowedMethods(methods);
            methods_set = true;
        }
        else if (tok == "return") {
            parseReturn(redirection_code, redirection_path);
            return_set = true;
        }
        else if (tok == "upload_store") {
            upload_store = nextToken();
            expect(";");
            upload_store_set = true;
        }
        else if (tok == "cgi_pass") {
            parseCgiPass(cgi_map);
            cgi_map_set = true;
        }
        else
            skipUnknownDirective(tok, "location");
    }
    nextToken(); // consume '}'

    if (return_set) {
        if (root_set || index_set || autoindex_set || methods_set || upload_store_set || cgi_map_set) {
            throw ExceptionParserError(
                "ConfigParser: location with 'return' cannot have other directives");
        }
    }

    if (!methods_set && !return_set)
        methods.push_back(Location::GET);

    bool needs_root = index_set || autoindex_set || upload_store_set || cgi_map_set;
    if (!needs_root && methods_set) {
        for (size_t i = 0; i < methods.size(); ++i) {
            if (methods[i] == Location::GET) {
                needs_root = true;
                break;
            }
        }
    }
    if (needs_root && !root_set) {
        throw ExceptionParserError(
            "ConfigParser: location with a handler must declare a 'root' directive");
    }

    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i] == Location::POST) {
            if (!cgi_map_set && !upload_store_set) {
                throw ExceptionParserError("ConfigParser: location with POST method must have 'cgi_pass' or 'upload_store'");
            }
            break;
        }
    }

    if (root_set)
        validateDirReadable(root, "root");
    if (upload_store_set)
        validateDirWritable(upload_store, "upload_store");
    if (cgi_map_set) {
        for (std::map<std::string, std::string>::const_iterator it = cgi_map.begin();
             it != cgi_map.end(); ++it) {
            validateExecutable(it->second, "cgi_pass");
        }
    }
    if (index_set && root_set)
        validatePathReadable(root + "/" + default_file, "index");

    return Location(path, methods, redirection_code, redirection_path, root, default_file, dir_listing, upload_store, cgi_map);
}
