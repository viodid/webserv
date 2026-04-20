#pragma once
#include "../Exceptions.hpp"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

class Body {
public:
    Body();
    Body(const std::string& body);

    const std::string& get() const;
    void set(const std::string&);

    int parse(const char* buffer, size_t buf_len, const std::string& content_len);

    std::string format() const;

private:
    std::string body_;
    size_t content_length_;
};
