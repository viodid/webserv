#pragma once
#include <string>

class Body {
public:
    Body() { };

    const std::string& get() const;

    void set(const std::string&);

private:
    std::string body_;
};
