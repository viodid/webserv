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

ExceptionBodyLength::ExceptionBodyLength(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionBodyLength::what() const throw()
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

ExceptionConnLenExceeded::ExceptionConnLenExceeded(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionConnLenExceeded::what() const throw()
{
    return msg_.c_str();
}

ExceptionRequestTimeout::ExceptionRequestTimeout(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionRequestTimeout::what() const throw()
{
    return msg_.c_str();
}

ExceptionErrorConnectionSocket::ExceptionErrorConnectionSocket(const std::string& str)
    : msg_(str)
{
}

const char* ExceptionErrorConnectionSocket::what() const throw()
{
    return msg_.c_str();
}

ExceptionParserError::ExceptionParserError(const std::string& msg)
    : msg_(msg)
{
}

ExceptionParserError::~ExceptionParserError() throw() { }

const char* ExceptionParserError::what() const throw()
{
    return msg_.c_str();
}

ExceptionUnsupportedFileType::ExceptionUnsupportedFileType(const std::string& msg)
    : msg_(msg)
{
}

ExceptionUnsupportedFileType::~ExceptionUnsupportedFileType() throw() { }

const char* ExceptionUnsupportedFileType::what() const throw()
{
    return msg_.c_str();
}
