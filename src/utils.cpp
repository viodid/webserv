#include "../include/Utils.hpp"
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/time.h>

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

// Gets the current time as ms since epoch
size_t currTimeMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (static_cast<unsigned long>(tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

const Location* matchLocation(const std::string& target,
    const std::vector<Location>& locations)
{
    const Location* best = NULL;
    size_t best_len = 0;
    for (size_t i = 0; i < locations.size(); ++i) {
        const std::string& path = locations[i].getPath();
        if (target.compare(0, path.size(), path) != 0)
            continue;
        bool on_boundary = target.size() == path.size()
            || path[path.size() - 1] == '/'
            || target[path.size()] == '/'
            || target[path.size()] == '?'
            || target[path.size()] == '#';
        if (!on_boundary)
            continue;
        if (best == NULL || path.size() > best_len) {
            best = &locations[i];
            best_len = path.size();
        }
    }
    return best;
}
