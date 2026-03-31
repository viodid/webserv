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

// #define BUFFER_SIZE 1 << 8 // 1B
#define BUFFER_SIZE 8
#define MAX_BUFFER_SIZE 1 << 10 // 10KiB

class HttpRequestParser {
public:
    enum RequestState {
        INIT,
        DONE,
    };

    HttpRequestParser(IReader& reader);

    const HttpRequest getRequest() const;
    void parseFromReader();

private:
    IReader& reader_;
    HttpRequest request_;
    RequestState current_state_;

    int parse_(const char* buffer, int length);
    int parseRequestLine_(const char* buffer, int length);

    bool done() const;
};
