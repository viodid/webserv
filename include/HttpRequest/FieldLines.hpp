#pragma once
#include "../Exceptions.hpp"
#include "../Settings.hpp"
#include "../Utils.hpp"
#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/*
 * field-line   = field-name ":" OWS field-value OWS
 *
 * Host: localhost:5555
 * User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:148.0)
 *
 * https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax
 */
class FieldLines {
public:
    FieldLines() { };
    FieldLines(const std::map<std::string, std::string>& field_lines);

    const std::string& get(const std::string&) const;

    void set(const std::string&, const std::string&);

    int parse(const char* buffer, int length);
    void forEach(void (*f)(const std::string&, const std::string&)) const;

    std::string format() const;

private:
    std::map<std::string, std::string> field_lines_;
};
