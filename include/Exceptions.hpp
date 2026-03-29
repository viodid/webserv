#pragma once
#include <cerrno>
#include <cstring>
#include <exception>
#include <string>

class ExceptionMalformedRequestLine : public std::exception {
public:
    ExceptionMalformedRequestLine(const std::string&);
    virtual const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionMalformedFieldLine : public std::exception {
public:
    ExceptionMalformedFieldLine(const std::string&);
    virtual const char* what() const throw();

private:
    const std::string msg_;
};
