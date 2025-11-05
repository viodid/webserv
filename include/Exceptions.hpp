#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <exception>
#include <cerrno>
#include <cstring>

class ExceptionGetAddrInfo : public std::exception {
public:
    virtual const char* What() const throw();
};

#endif


