#include "../include/HttpRequestParser.hpp"
#include "../include/IReader.hpp"
#include <exception>
#include <gtest/gtest.h>

class ChunkReader : public IReader {
public:
    ChunkReader(const std::string& data, int nb)
        : data_(data)
        , bytes_per_read_(nb)
        , pos_(0) { };

    int read(char buffer[], int len)
    {
        if (pos_ >= data_.size())
            return 0;

        if (len < bytes_per_read_)
            bytes_per_read_ = len;

        int i { 0 };
        for (auto it { data_.begin() + pos_ };
            i < bytes_per_read_ && it != data_.end();
            it++) {
            buffer[i] = *it;
            i++;
        }
        pos_ += i;
        return i;
    };

private:
    const std::string data_;
    int bytes_per_read_;
    size_t pos_;
};

TEST(HttpParserTest, ParseRequestLineCorrectSmallBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1);
    HttpRequestParser parser(reader);
    try {
        parser.parseFromReader();
        HttpRequest request = parser.getRequest();
        EXPECT_EQ("GET", request.request_line.method);
        EXPECT_EQ("/", request.request_line.request_target);
        EXPECT_EQ("1.1", request.request_line.http_version);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpParserTest, ParseRequestLineCorrectBigBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1024);
    HttpRequestParser parser(reader);
    try {
        parser.parseFromReader();
        HttpRequest request = parser.getRequest();
        EXPECT_EQ("GET", request.request_line.method);
        EXPECT_EQ("/", request.request_line.request_target);
        EXPECT_EQ("1.1", request.request_line.http_version);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpParserTest, ParseRequestLineCorrectWithPath)
{
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 11);
    HttpRequestParser parser(reader);
    try {
        parser.parseFromReader();
        HttpRequest request = parser.getRequest();
        EXPECT_EQ("GET", request.request_line.method);
        EXPECT_EQ("/coffee", request.request_line.request_target);
        EXPECT_EQ("1.0", request.request_line.http_version);
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpParserTest, ParseRequestLineMissingMethod)
{
    ChunkReader reader("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequestParser parser(reader);
    EXPECT_THROW(parser.parseFromReader(),
        ExceptionMalformedRequestLine);
}

TEST(HttpParserTest, ParseRequestLineSwapOrder)
{
    ChunkReader reader("GET HTTP/1.1 /coffee\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequestParser parser(reader);
    EXPECT_THROW(parser.parseFromReader(),
        ExceptionMalformedRequestLine);
}

TEST(HttpParserTest, ParseRequestLineWrongHttpVersion)
{
    ChunkReader reader("GET /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequestParser parser(reader);
    EXPECT_THROW(parser.parseFromReader(),
        ExceptionMalformedRequestLine);
}

TEST(HttpParserTest, ParseRequestLineMultipleWS)
{
    ChunkReader reader("GET  /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1);
    HttpRequestParser parser(reader);
    EXPECT_THROW(parser.parseFromReader(),
        ExceptionMalformedRequestLine);
}
