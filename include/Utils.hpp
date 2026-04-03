#pragma once
#include <cctype>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

std::string toLower(const std::string& str);
std::string trim(const std::string& str);
