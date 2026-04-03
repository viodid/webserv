#include "../include/HttpRequest/HttpRequest.hpp"
#include "../include/IReader.hpp"
#include <exception>
#include <iostream>
#include <gtest/gtest.h>

class ChunkReader : public IReader {
public:
    ChunkReader(const std::string& data, int nb)
        : data_(data)
        , bytes_per_read_(nb)
        , pos_(0) { };

    int read(char buffer[], int len)
    {
        int to_read = bytes_per_read_;
        if (pos_ >= data_.size())
            return 0;

        if (len < bytes_per_read_)
            to_read = len;

        int i { 0 };
        for (auto it { data_.begin() + pos_ };
            i < to_read && it != data_.end();
            it++) {
            buffer[i] = *it;
            i++;
        }
        pos_ += i;
        return i;
    };

private:
    const std::string data_;
    const int bytes_per_read_;
    size_t pos_;
};

/*
 * REQUEST LINE TESTS
 */
TEST(HttpRequestTest, ParseRequestLineCorrectSmallBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("GET", request.getRequestLine().getMethod());
        EXPECT_EQ("/", request.getRequestLine().getRequestTarget());
        EXPECT_EQ("1.1", request.getRequestLine().getHttpVersion());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseRequestLineCorrectBigBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1024);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("GET", request.getRequestLine().getMethod());
        EXPECT_EQ("/", request.getRequestLine().getRequestTarget());
        EXPECT_EQ("1.1", request.getRequestLine().getHttpVersion());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseRequestLineCorrectWithPath)
{
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1024);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("GET", request.getRequestLine().getMethod());
        EXPECT_EQ("/coffee", request.getRequestLine().getRequestTarget());
        EXPECT_EQ("1.0", request.getRequestLine().getHttpVersion());
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseRequestLineMissingMethod)
{
    ChunkReader reader("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequest parser;
    EXPECT_THROW(parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

TEST(HttpRequestTest, ParseRequestLineSwapOrder)
{
    ChunkReader reader("GET HTTP/1.1 /coffee\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequest parser;
    EXPECT_THROW(parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

TEST(HttpRequestTest, ParseRequestLineWrongHttpVersion)
{
    ChunkReader reader("GET /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequest parser;
    EXPECT_THROW(parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

TEST(HttpRequestTest, ParseRequestLineMultipleWS)
{
    ChunkReader reader("GET  /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequest parser;
    EXPECT_THROW(parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

/*
 * FIELD LINES TESTS
 */
TEST(HttpRequestTest, ParseFieldLineCorrect)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent:  curl/7.81.0\r\nAccept:   text/html\r\n\r\n", 1);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("localhost:42069", request.getFieldLines().get("Host"));
        EXPECT_EQ("curl/7.81.0", request.getFieldLines().get("User-Agent"));
        EXPECT_EQ("text/html", request.getFieldLines().get("Accept"));
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseFieldLineRepeatedHeader)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nHost:  0.0.0.0\r\nAccept:   text/html\r\n\r\n", 1);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("localhost:42069,0.0.0.0", request.getFieldLines().get("Host"));
        EXPECT_EQ("text/html", request.getFieldLines().get("Accept"));
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseFieldLineWrongMissingColon)
{
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost  \r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequest parser;
    EXPECT_THROW(parser.parseFromReader(reader),
        ExceptionMalformedFieldLine);
}

TEST(HttpRequestTest, ParseFieldLineWrongFormat)
{
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost : localhost\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequest parser;
    EXPECT_THROW(parser.parseFromReader(reader),
        ExceptionMalformedFieldLine);
}

TEST(HttpRequestTest, ParseFieldLineWrongFieldNameToken)
{
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost: localhost\r\nUser-@gent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequest parser;
    EXPECT_THROW(parser.parseFromReader(reader),
        ExceptionMalformedFieldLine);
}

/*
 * BODY TESTS
 */
TEST(HttpRequestTest, ParseBodyCorrect)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 16\r\n\r\nthis is the body", 1);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("this is the body", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseBodyCorrectBigBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 16\r\n\r\nthis is the body", 1024);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("this is the body", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseEmptyBody)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 0\r\n\r\n", 1);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseNoContentLength)
{
    ChunkReader reader("GET / HTTP/1.1\r\n\r\n", 1);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseNoContentLengthButBody)
{
    ChunkReader reader("GET / HTTP/1.1\r\n\r\nbatman", 1);
    HttpRequest request;
    try {
        request.parseFromReader(reader);
        EXPECT_EQ("", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseBiggerContentLengthThanBody)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 100\r\n\r\nbatman", 1);
    HttpRequest request;
    EXPECT_THROW(request.parseFromReader(reader),
        ExceptionBodyLength);
}
