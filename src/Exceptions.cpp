#include "../include/Exceptions.hpp"

ExceptionMalformedFieldLine::ExceptionMalformedFieldLine(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionMalformedRequestLine::what() const throw()
{
    return msg_.c_str();
}

ExceptionMalformedRequestLine::ExceptionMalformedRequestLine(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionMalformedFieldLine::what() const throw()
{
    return msg_.c_str();
}
