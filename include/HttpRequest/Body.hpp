#pragma once
#include "../Exceptions.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <string>

class Body {
public:
    Body();

    const std::string& get() const;
    void set(const std::string&);

    int parse(const char* buffer, size_t buf_len, const std::string& content_len);

private:
    std::string body_;
    size_t content_length_;
};
