#include "../include/Utils.hpp"
#include <cctype>
#include <algorithm>

// Takes a file descriptor and a buffer (as a vector)
// and returns the number of bytes read or -1 in case of error
size_t readFromFile(int fd, std::vector<char>& buf)
{
    const size_t BUFFER_SIZE = 1 << 12; // 4KiB
    char buf_chunk[BUFFER_SIZE];
    size_t total = 0;
    int bytes_read;
    while ((bytes_read = read(fd, buf_chunk, BUFFER_SIZE)) > 0) {
        buf.insert(buf.end(), buf_chunk, buf_chunk + bytes_read);
        total += bytes_read;
    }
    if (bytes_read == -1)
        return -1;
    return total;
}

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
