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

class ExceptionErrorConnectionSocket : public std::exception {
public:
    ExceptionErrorConnectionSocket(const std::string&);
    virtual ~ExceptionErrorConnectionSocket() throw() { };

    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionParserError : public std::exception {
public:
    ExceptionParserError(const std::string& msg);
    virtual ~ExceptionParserError() throw();
    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionUnsupportedFileType : public std::exception {
public:
    ExceptionUnsupportedFileType(const std::string& msg);
    virtual ~ExceptionUnsupportedFileType() throw();
    const char* what() const throw();

private:
    const std::string msg_;
};

class ExceptionParentRootDirectory : public std::exception {
public:
    ExceptionParentRootDirectory(const std::string& msg);
    virtual ~ExceptionParentRootDirectory() throw();
    const char* what() const throw();

private:
    const std::string msg_;
};
