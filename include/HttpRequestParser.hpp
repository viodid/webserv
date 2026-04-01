#pragma once
#include "Exceptions.hpp"
#include "HttpRequest.hpp"
#include "IReader.hpp"
#include "Settings.hpp"
#include <cctype>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

class HttpRequestParser {
public:
    enum RequestState {
        RequestLine,
        FieldLine,
        Done
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
    int parseFieldLine_(const char* buffer, int length);

    bool done() const;
};
