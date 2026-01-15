#pragma once
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

size_t readFromFile(int fd, std::vector<char>& buf);
