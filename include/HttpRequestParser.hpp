#pragma once
#include "Exceptions.hpp"
#include "HttpRequest.hpp"
#include "IReader.hpp"
#include <cctype>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#define BUFFER_SIZE 1 << 10 // 1kib

class HttpRequestParser {
public:
    enum RequestState {
        INIT,
        DONE,
    };

    HttpRequestParser(IReader& reader);

    HttpRequest parse();

private:
    IReader& reader_;
    std::string buffer_;
    RequestState state_;

    HttpRequestLine parseRequestLine_();
};
