#pragma once
#include "Exceptions.hpp"
#include "HttpRequest.hpp"
#include <cctype>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

class HttpRequestParser {
public:
    HttpRequestParser(const std::string&);

    HttpRequest parse();

private:
    std::string stream_;

    HttpRequestLine parseRequestLine_();
};
