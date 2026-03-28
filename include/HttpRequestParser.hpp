#pragma once
#include "HttpRequest.hpp"
#include <stdexcept>

class HttpRequestParser {
public:
    HttpRequestParser(const std::string&);

    HttpRequest parse() const;

private:
    const std::string stream_;
};
