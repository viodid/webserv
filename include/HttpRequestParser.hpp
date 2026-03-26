#pragma once
#include "HttpRequest.hpp"
#include <stdexcept>

class HttpRequestParser {
public:
    HttpRequestParser(const std::string&);

    void parse() const;

private:
    const std::string stream_;
};
