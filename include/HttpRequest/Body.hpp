#pragma once
#include "../Exceptions.hpp"
#include <stdexcept>
#include <string>

class Body {
public:
    Body() { };

    const std::string& get() const;
    void set(const std::string&);

    int parse(const char* buffer, int buf_len, const std::string& content_len);

private:
    std::string body_;
};
