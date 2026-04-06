#pragma once
#include <cerrno>
#include <cstring>
#include <exception>
#include <string>

class ExceptionMalformedRequestLine : public std::exception {
public:
    ExceptionMalformedRequestLine(const std::string&);
    virtual ~ExceptionMalformedRequestLine() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionMalformedFieldLine : public std::exception {
public:
    ExceptionMalformedFieldLine(const std::string&);
    virtual ~ExceptionMalformedFieldLine() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionBodyLength : public std::exception {
public:
    ExceptionBodyLength(const std::string&);
    virtual ~ExceptionBodyLength() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionClientCloseConn : public std::exception {
public:
    ExceptionClientCloseConn(const std::string&);
    virtual ~ExceptionClientCloseConn() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionClientErrConn : public std::exception {
public:
    ExceptionClientErrConn(const std::string&);
    virtual ~ExceptionClientErrConn() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionConnLenExceeded : public std::exception {
public:
    ExceptionConnLenExceeded(const std::string&);
    virtual ~ExceptionConnLenExceeded() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionRequestTimeout : public std::exception {
public:
    ExceptionRequestTimeout(const std::string&);
    virtual ~ExceptionRequestTimeout() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};
