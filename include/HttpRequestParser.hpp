#pragma once
#include "Exceptions.hpp"
#include "HttpRequest.hpp"
#include <stdexcept>

class HttpRequestParser {
public:
    HttpRequestParser(const std::string&);

    HttpRequest parse();

private:
    std::string stream_;

    HttpRequestLine parseRequestLine_();
};
