#pragma once
#include <cctype>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

size_t readFromFile(int fd, std::vector<char>& buf);
std::string toLower(const std::string& str);
