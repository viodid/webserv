#include "../include/HttpRequestParser.hpp"
#include "../include/IReader.hpp"
#include <exception>
#include <gtest/gtest.h>
#include <stdexcept>

class ChunkReader : IReader {
public:
    ChunkReader(const std::string& data, int nb)
        : data_(data)
        , bytes_per_read_(nb)
        , pos_(0) { };

    int read(char bufffer[], int len)
    {
        if (pos_ >= data_.size())
            return 0;

        if (len < bytes_per_read_)
            throw std::runtime_error("buffer size is smaller than chunk reader");

        int i { 0 };
        for (auto it { data_.begin() + pos_ };
            i < bytes_per_read_ || it != data_.end();
            it++) {
            bufffer[i] = *it;
            i++;
        }
        pos_ += i;
        return i;
    };

private:
    const std::string data_;
    const int bytes_per_read_;
    int pos_;
};

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
