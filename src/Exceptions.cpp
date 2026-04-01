#include "../include/Exceptions.hpp"

ExceptionMalformedRequestLine::ExceptionMalformedRequestLine(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionMalformedRequestLine::what() const throw()
{
    return msg_.c_str();
}

ExceptionMalformedFieldLine::ExceptionMalformedFieldLine(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionMalformedFieldLine::what() const throw()
{
    return msg_.c_str();
}

ExceptionClientCloseConn::ExceptionClientCloseConn(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionClientCloseConn::what() const throw()
{
    return msg_.c_str();
}
