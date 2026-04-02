#pragma once
#include <string>

class Body {
public:
    Body() { };

    const std::string& get() const;
    void set(const std::string&);

    int parse(const char* buffer, int length);

private:
    std::string body_;
};
