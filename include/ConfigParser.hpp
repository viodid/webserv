#pragma once
#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

// Parses an NGINX-style configuration file and returns a Config.
class ConfigParser {
public:
    ConfigParser(const std::string& filepath);
    Config parse();

private:
    std::string filepath_;
    std::string content_;
    size_t      pos_;

    void        readFile();
    void        skipWhitespaceAndComments();
    std::string nextToken();
    std::string peekToken();
    void        expect(const std::string& expected);
    void        skipUnknownDirective(const std::string& tok, const std::string& block);

    void        parseListen(std::string& hostname, std::string& port);
    void        parseClientMaxBodySize(size_t& size);
    void        parseErrorPage(std::vector<std::pair<Location::StatusCodes, std::string> >& pages);
    void        parseAutoindex(bool& dir_listing);
    void        parseAllowedMethods(std::vector<Location::AllowedMethods>& methods);
    void        parseReturn(std::string& code, std::string& path);
    void        parseCgiPass(std::map<std::string, std::string>& cgi_map);

    VirtualHost              parseServerBlock();
    Location                 parseLocationBlock();
};
