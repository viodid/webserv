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

    VirtualHost              parseServerBlock();
    Location                 parseLocationBlock();
};
