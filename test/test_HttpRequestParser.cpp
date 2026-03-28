/*
 *
 * // Test: Good GET Request line
 * r, err := RequestFromReader(strings.NewReader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n "))
 * require.NoError(t, err)
 * require.NotNil(t, r)
 * assert.Equal(t, "GET", r.RequestLine.Method)
 * assert.Equal(t, "/", r.RequestLine.RequestTarget)
 * assert.Equal(t, "1.1", r.RequestLine.HttpVersion)
 * // Test: Good GET Request line with path
 * r,err = RequestFromReader(strings.NewReader("GET /coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n "))
 * require.NoError(t, err)
 * require.NotNil(t, r)
 * assert.Equal(t, "GET", r.RequestLine.Method)
 * assert.Equal(t, "/coffee", r.RequestLine.RequestTarget)
 * assert.Equal(t, "1.1", r.RequestLine.HttpVersion)
 *
 *  // Test: Invalid number of parts in request line
 * err = RequestFromReader(strings.NewReader("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n "))
 * require.Error(t, err)
 */

#include "../include/HttpRequestParser.hpp"
#include <gtest/gtest.h>

TEST(HttpParserTest, ParseRequestLineCorrect)
{
    HttpRequest request = HttpRequestParser("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n").parse();
    EXPECT_EQ("GET", request.getRequestLine().getMethod());
    EXPECT_EQ("/", request.getRequestLine().getRequestTarget());
    EXPECT_EQ("1.1", request.getRequestLine().getHttpVersion());
}

TEST(HttpParserTest, ParseRequestLineCorrectWithPath)
{
    HttpRequest request = HttpRequestParser("GET /coffee HTTP/1.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse();
    EXPECT_EQ("GET", request.getRequestLine().getMethod());
    EXPECT_EQ("/coffee", request.getRequestLine().getRequestTarget());
    EXPECT_EQ("1.0", request.getRequestLine().getHttpVersion());
}

TEST(HttpParserTest, ParseRequestLineInvalid)
{
    EXPECT_THROW(HttpRequestParser("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET /coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET coffee 1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET coffee HTTP/1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
    EXPECT_THROW(HttpRequestParser("GET /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ").parse(),
        ExceptionMalformedRequestLine);
}
