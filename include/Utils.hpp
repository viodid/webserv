#pragma once
#include "Config.hpp"
#include <cctype>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

std::string toLower(const std::string& str);
std::string trim(const std::string& str);
size_t currTimeMs();
std::string mapStatusCode(Location::ErrorPages status_code);
