#include "../include/Utils.hpp"
#include <cctype>

// Converts all characters in str to lowercase
std::string toLower(const std::string& str)
{
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = std::tolower(static_cast<unsigned char>(result[i]));
    return result;
}

// Removes leading and trailing whitespace from str
std::string trim(const std::string& str)
{
    size_t start = 0;
    size_t end = str.size();
    while (start < end && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;
    return str.substr(start, end - start);
}
