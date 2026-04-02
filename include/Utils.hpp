#pragma once
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <string>

size_t		readFromFile(int fd, std::vector<char>& buf);
std::string	toLower(const std::string& str);
std::string	trim(const std::string& str);
