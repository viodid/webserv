#include "../include/HttpRequest/HttpRequest.hpp"
#include <chrono>
#include <exception>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <thread>

class ChunkReader : public IReader {
public:
    ChunkReader(const std::string& data, int nb, size_t timeout_ms)
        : data_(data)
        , bytes_per_read_(nb)
        , timeout_(timeout_ms)
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
        if (timeout_ > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_));
        return i;
    };

private:
    const std::string data_;
    const int bytes_per_read_;
    const size_t timeout_;
    size_t pos_;
};

/*
 * REQUEST LINE TESTS
 */
TEST(HttpRequestTest, ParseRequestLineCorrectSmallBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
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
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1024, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
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
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n", 1024, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
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
    ChunkReader reader("/coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

TEST(HttpRequestTest, ParseRequestLineSwapOrder)
{
    ChunkReader reader("GET HTTP/1.1 /coffee\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

TEST(HttpRequestTest, ParseRequestLineWrongHttpVersion)
{
    ChunkReader reader("GET /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

TEST(HttpRequestTest, ParseRequestLineMultipleWS)
{
    ChunkReader reader("GET  /coffee HTTP/2.0\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionMalformedRequestLine);
}

TEST(HttpRequestTest, ParseRequestLineIncomplete)
{
    ChunkReader reader("GET  /coffee HTTP/2.0", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionRequestTimeout);
}

/*
 * FIELD LINES TESTS
 */
TEST(HttpRequestTest, ParseFieldLineCorrect)
{
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent:  curl/7.81.0\r\nAccept:   text/html\r\n\r\n", 1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
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
    ChunkReader reader("GET / HTTP/1.1\r\nHost: localhost:42069\r\nHost:  0.0.0.0\r\nAccept:   text/html\r\n\r\n", 1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
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
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost  \r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionMalformedFieldLine);
}

TEST(HttpRequestTest, ParseFieldLineWrongFormat)
{
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost : localhost\r\nUser-Agent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionMalformedFieldLine);
}

TEST(HttpRequestTest, ParseFieldLineWrongFieldNameToken)
{
    ChunkReader reader("GET /coffee HTTP/1.0\r\nHost: localhost\r\nUser-@gent: curl/7.81.0\r\nAccept: \r\n\r\n ", 1, 0);
    HttpRequest parser;
    EXPECT_THROW(while (!parser.isDone()) parser.parseFromReader(reader),
        ExceptionMalformedFieldLine);
}

/*
 * BODY TESTS
 */
TEST(HttpRequestTest, ParseBodyCorrectSmallBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 16\r\n\r\nthis is the body", 1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
            request.parseFromReader(reader);
        EXPECT_EQ("this is the body", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseBodyCorrectBigBuffer)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 16\r\n\r\nthis is the body", 1024, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
            request.parseFromReader(reader);
        EXPECT_EQ("this is the body", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseEmptyBody)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 0\r\n\r\n", 1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
            request.parseFromReader(reader);
        EXPECT_EQ("", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseNoContentLength)
{
    ChunkReader reader("GET / HTTP/1.1\r\n\r\n", 1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
            request.parseFromReader(reader);
        EXPECT_EQ("", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseNoContentLengthButBody)
{
    ChunkReader reader("GET / HTTP/1.1\r\n\r\nbatman", 1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
            request.parseFromReader(reader);
        EXPECT_EQ("", request.getBody().get());
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseBiggerContentLengthThanBody)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 100\r\n\r\nbatman", 1, 0);
    HttpRequest request;
    EXPECT_THROW(while (!request.isDone()) request.parseFromReader(reader),
        ExceptionRequestTimeout);
}

TEST(HttpRequestTest, ParseTimeoutRaisesException)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: 100\r\n\r\nbatman", 1, Settings::TIMEOUT_REQUEST_MS / 10);
    HttpRequest request;
    EXPECT_THROW(while (!request.isDone()) request.parseFromReader(reader),
        ExceptionRequestTimeout);
}

TEST(HttpRequestTest, ParseMalformedContentLength)
{
    ChunkReader reader("GET / HTTP/1.1\r\nContent-Length: L00\r\n\r\nbatman", 1, 0);
    HttpRequest request;
    EXPECT_THROW(while (!request.isDone()) request.parseFromReader(reader),
        ExceptionBodyLength);
}

/*
 * CHUNKED BODY TESTS
 */
static const char kChunkedTestDir[] = "/tmp/webserv_chunked_test";

static std::vector<Location> makeChunkedLocations()
{
    mkdir(kChunkedTestDir, 0755);
    std::vector<Location::AllowedMethods> methods;
    methods.push_back(Location::POST);
    std::vector<Location> locs;
    locs.push_back(Location("/upload", methods, "", "", "/tmp", "", false, kChunkedTestDir));
    return locs;
}

static std::string readFile(const std::string& path)
{
    std::ifstream in(path.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

TEST(HttpRequestTest, ParseChunkedBodyValid)
{
    std::vector<Location> locs = makeChunkedLocations();
    ChunkReader reader(
        "POST /upload/hello.bin HTTP/1.1\r\n"
        "Transfer-Encoding: chunked\r\n\r\n"
        "5\r\nhello\r\n"
        "6\r\n world\r\n"
        "0\r\n\r\n",
        1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
            request.parseFromReader(reader, locs);
        EXPECT_EQ("/tmp/webserv_chunked_test/hello.bin", request.getBody().getStoredPath());
        EXPECT_EQ("hello world", readFile(request.getBody().getStoredPath()));
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseChunkedMalformedHexThrows)
{
    std::vector<Location> locs = makeChunkedLocations();
    ChunkReader reader(
        "POST /upload/bad.bin HTTP/1.1\r\n"
        "Transfer-Encoding: chunked\r\n\r\n"
        "zz\r\nhello\r\n0\r\n\r\n",
        1, 0);
    HttpRequest request;
    EXPECT_THROW(while (!request.isDone()) request.parseFromReader(reader, locs),
        ExceptionBadFraming);
}

TEST(HttpRequestTest, ParseChunkedTEAndCLPrecedence)
{
    // Both TE: chunked and a misleading Content-Length are present. Per RFC 9112,
    // chunked must take precedence; otherwise the parser would pick BodyState and
    // treat the chunked framing as raw body.
    std::vector<Location> locs = makeChunkedLocations();
    ChunkReader reader(
        "POST /upload/precedence.bin HTTP/1.1\r\n"
        "Content-Length: 999\r\n"
        "Transfer-Encoding: chunked\r\n\r\n"
        "3\r\nabc\r\n0\r\n\r\n",
        1, 0);
    HttpRequest request;
    try {
        while (!request.isDone())
            request.parseFromReader(reader, locs);
        EXPECT_EQ("abc", readFile(request.getBody().getStoredPath()));
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        FAIL();
    }
}

TEST(HttpRequestTest, ParseChunkedSizeLineTooLong)
{
    std::vector<Location> locs = makeChunkedLocations();
    std::string head = "POST /upload/dos.bin HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n";
    std::string giant_line(Settings::PARSER_MAX_BUFFER_SIZE + 16, 'a');
    ChunkReader reader(head + giant_line, 1024, 0);
    HttpRequest request;
    EXPECT_THROW(while (!request.isDone()) request.parseFromReader(reader, locs),
        ExceptionPayloadTooLarge);
}
