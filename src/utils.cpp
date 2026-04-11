#include "../include/Utils.hpp"
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

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

size_t readFile(char* buffer, size_t len, const std::string& path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1)
        throw std::runtime_error(std::strerror(errno));

    size_t bytes = read(fd, buffer, len);
    if (fd == -1)
        throw std::runtime_error(std::strerror(errno));
    close(fd);

    return bytes;
}
