#include "../include/Exceptions.hpp"

const char* ExceptionGetAddrInfo::What() const throw() {
    return std::strerror(errno);
}

