#include "../include/Utils.hpp"

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

size_t currTimeMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (static_cast<unsigned long>(tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

std::string mapStatusCode(Location::ErrorPages status_code)
{
    switch (status_code) {
    case Location::E_400:
        return "Bad Request";
    case Location::E_403:
        return "Forbidden";
    case Location::E_404:
        return "Not Found";
    case Location::E_405:
        return "Method Not Allowed";
    case Location::E_408:
        return "Request Timeout";
    case Location::E_413:
        return "Content Too Large";
    case Location::E_414:
        return "URI Too Long";
    case Location::E_500:
        return "Internal Server Error";
    case Location::E_501:
        return "Not implemented";
    case Location::E_502:
        return "Bad Wategay";
    case Location::E_503:
        return "Service Unavailable";
    case Location::E_504:
        return "Gateway Timeout";
    case Location::_ERROR_COUNT:
        return "Not Implemented Error";
    }
}
