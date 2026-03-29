#pragma once
#include "Exceptions.hpp"
#include "HttpRequest.hpp"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class HttpRequestParser {
public:
    HttpRequestParser(const std::string&);

    HttpRequest parse();

private:
    std::string stream_;

    HttpRequestLine parseRequestLine_();
};
