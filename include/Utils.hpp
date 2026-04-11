#pragma once
#include <cctype>
#include <string>

std::string toLower(const std::string& str);
std::string trim(const std::string& str);
size_t currTimeMs();
size_t readFile(char* buffer, size_t len, const std::string& path);
