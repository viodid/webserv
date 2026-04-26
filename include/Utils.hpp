#pragma once
#include "Config.hpp"
#include <cctype>
#include <string>
#include <vector>

std::string toLower(const std::string& str);
std::string trim(const std::string& str);
size_t currTimeMs();
const Location* matchLocation(const std::string& target,
    const std::vector<Location>& locations);
