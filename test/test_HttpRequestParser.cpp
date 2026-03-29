#include "../include/HttpRequestParser.hpp"
#include <exception>
#include <gtest/gtest.h>

TEST(HttpParserTest, ParseRequestLineCorrect)
{
    try {
        HttpRequest request = HttpRequestParser("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n").parse();
        EXPECT_EQ("GET", request.getRequestLine().getMethod());
        EXPECT_EQ("/", request.getRequestLine().getRequestTarget());
        EXPECT_EQ("1.1", request.getRequestLine().getHttpVersion());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpParserTest, ParseRequestLineCorrectWithPath)
{
    try {
        HttpRequest request = HttpRequestParser("GET /coffee HTTP/1.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse();
        EXPECT_EQ("GET", request.getRequestLine().getMethod());
        EXPECT_EQ("/coffee", request.getRequestLine().getRequestTarget());
        EXPECT_EQ("1.0", request.getRequestLine().getHttpVersion());
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpParserTest, ParseRequestLineInvalid)
{
    EXPECT_THROW(HttpRequestParser("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET HTTP/1.1 /coffee\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET coffee 1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET coffee HTTP/1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET /coffee  HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET  /coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET /coffee HTTP/1.1\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
}
