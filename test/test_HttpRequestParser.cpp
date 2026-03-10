#include "../include/HttpRequestParser.hpp"
#include <gtest/gtest.h>

// ============================================================
// Helper: builds a valid raw HTTP request
// ============================================================
static std::string makeRaw(const std::string& method,
                            const std::string& uri,
                            const std::string& version,
                            const std::string& extra_headers = "",
                            const std::string& body = "")
{
    std::string raw = method + " " + uri + " " + version + "\r\n";
    raw += "Host: localhost\r\n";
    if (!extra_headers.empty())
        raw += extra_headers;
    if (!body.empty())
        raw += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    raw += "\r\n";
    raw += body;
    return raw;
}

// ============================================================
// Request line: method, URI, version
// ============================================================

TEST(HttpRequestParser, ParsesGetRequest)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", "/index.html", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.method, Location::GET);
    EXPECT_EQ(req.path, "/index.html");
    EXPECT_EQ(req.version, "HTTP/1.1");
}

TEST(HttpRequestParser, ParsesHttp10)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", "/", "HTTP/1.0"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.version, "HTTP/1.0");
}

TEST(HttpRequestParser, ParsesDeleteMethod)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("DELETE", "/resource", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.method, Location::DELETE);
}

TEST(HttpRequestParser, ParsesHeadMethod)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("HEAD", "/", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.method, Location::HEAD);
}

TEST(HttpRequestParser, UnknownMethodReturns501)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("PATCH", "/", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_NOT_IMPLEMENTED);
}

TEST(HttpRequestParser, InvalidVersionReturns400)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", "/", "HTTP/2.0"), 0);
    EXPECT_EQ(req.state, PARSE_BAD_REQUEST);
}

TEST(HttpRequestParser, UriTooLongReturns414)
{
    std::string long_uri = "/" + std::string(HttpRequestParser::MAX_URI_LENGTH, 'a');
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", long_uri, "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_URI_TOO_LONG);
}

TEST(HttpRequestParser, ParsesQueryString)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", "/search?q=foo&page=2", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.path, "/search");
    EXPECT_EQ(req.query_string, "q=foo&page=2");
}

TEST(HttpRequestParser, QueryStringEmptyWhenNoQuestion)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", "/no-query", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_TRUE(req.query_string.empty());
}

// ============================================================
// Headers
// ============================================================

TEST(HttpRequestParser, HeadersAreLowercased)
{
    std::string raw = makeRaw("GET", "/", "HTTP/1.1", "X-Custom-Header: MyValue\r\n");
    HttpRequest req = HttpRequestParser::parse(raw, 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_TRUE(req.hasHeader("x-custom-header"));
    EXPECT_EQ(req.getHeader("x-custom-header"), "MyValue");
}

TEST(HttpRequestParser, GetHeaderIsCaseInsensitive)
{
    std::string raw = makeRaw("GET", "/", "HTTP/1.1", "Content-Type: text/html\r\n");
    HttpRequest req = HttpRequestParser::parse(raw, 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.getHeader("CONTENT-TYPE"), "text/html");
    EXPECT_EQ(req.getHeader("content-type"), "text/html");
}

TEST(HttpRequestParser, HasHeaderReturnsFalseForMissing)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", "/", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_FALSE(req.hasHeader("authorization"));
}

TEST(HttpRequestParser, MissingHeaderTerminatorReturns400)
{
    // No \r\n\r\n terminator
    std::string raw = "GET / HTTP/1.1\r\nHost: localhost\r\n";
    HttpRequest req = HttpRequestParser::parse(raw, 0);
    EXPECT_EQ(req.state, PARSE_BAD_REQUEST);
}

TEST(HttpRequestParser, MissingHeaderTerminatorIncompleteWhenNotComplete)
{
    std::string raw = "GET / HTTP/1.1\r\nHost: localhost\r\n";
    HttpRequest req = HttpRequestParser::parseIncremental(raw, false, 1024);
    EXPECT_EQ(req.state, PARSE_INCOMPLETE);
}

// ============================================================
// Body with Content-Length
// ============================================================

TEST(HttpRequestParser, PostBodyParsed)
{
    std::string body = "name=John&age=30";
    HttpRequest req = HttpRequestParser::parse(
        makeRaw("POST", "/submit", "HTTP/1.1", "Content-Type: application/x-www-form-urlencoded\r\n", body),
        1024);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.body, body);
}

TEST(HttpRequestParser, PutBodyParsed)
{
    std::string body = "{\"key\":\"value\"}";
    HttpRequest req = HttpRequestParser::parse(
        makeRaw("PUT", "/resource", "HTTP/1.1", "Content-Type: application/json\r\n", body),
        1024);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.body, body);
}

TEST(HttpRequestParser, BodyExceedsMaxSizeReturns413)
{
    std::string body(200, 'x');
    HttpRequest req = HttpRequestParser::parse(
        makeRaw("POST", "/upload", "HTTP/1.1", "", body),
        50);  // max_body_size = 50
    EXPECT_EQ(req.state, PARSE_ENTITY_TOO_LARGE);
}

TEST(HttpRequestParser, InvalidContentLengthReturns400)
{
    std::string raw = "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: abc\r\n\r\nbody";
    HttpRequest req = HttpRequestParser::parse(raw, 1024);
    EXPECT_EQ(req.state, PARSE_BAD_REQUEST);
}

TEST(HttpRequestParser, IncompleteBodyReturnsIncomplete)
{
    // Content-Length says 100 but body only has 5 bytes
    std::string raw = "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 100\r\n\r\nhello";
    HttpRequest req = HttpRequestParser::parseIncremental(raw, false, 1024);
    EXPECT_EQ(req.state, PARSE_INCOMPLETE);
}

TEST(HttpRequestParser, IncompleteBodyCompleteReturns400)
{
    std::string raw = "POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 100\r\n\r\nhello";
    HttpRequest req = HttpRequestParser::parse(raw, 1024);
    EXPECT_EQ(req.state, PARSE_BAD_REQUEST);
}

TEST(HttpRequestParser, GetWithNoBodyIsOk)
{
    HttpRequest req = HttpRequestParser::parse(makeRaw("GET", "/", "HTTP/1.1"), 0);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_TRUE(req.body.empty());
}

// ============================================================
// Chunked transfer
// ============================================================

TEST(HttpRequestParser, ChunkedBodyDecoded)
{
    // Chunked: "Hello " (6) + "World" (5) + final terminator 0
    std::string chunked_body = "6\r\nHello \r\n5\r\nWorld\r\n0\r\n";
    std::string raw =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n" +
        chunked_body;
    HttpRequest req = HttpRequestParser::parse(raw, 1024);
    EXPECT_EQ(req.state, PARSE_SUCCESS);
    EXPECT_EQ(req.body, "Hello World");
}

TEST(HttpRequestParser, ChunkedBodyExceedsMaxReturns413)
{
    std::string chunked_body = "6\r\nHello \r\n5\r\nWorld\r\n0\r\n";
    std::string raw =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n" +
        chunked_body;
    HttpRequest req = HttpRequestParser::parse(raw, 5);  // max = 5, body = 11
    EXPECT_EQ(req.state, PARSE_ENTITY_TOO_LARGE);
}

TEST(HttpRequestParser, IncompleteChunkedReturnsIncomplete)
{
    // Missing final terminator 0\r\n
    std::string raw =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "5\r\nHello\r\n";  // sin chunk final
    HttpRequest req = HttpRequestParser::parseIncremental(raw, false, 1024);
    EXPECT_EQ(req.state, PARSE_INCOMPLETE);
}

// ============================================================
// Empty buffer
// ============================================================

TEST(HttpRequestParser, EmptyBufferReturnsIncomplete)
{
    HttpRequest req = HttpRequestParser::parse("", 1024);
    EXPECT_EQ(req.state, PARSE_INCOMPLETE);
}

// ============================================================
// HttpFieldLine and HttpRequestLine (support wrappers)
// ============================================================

TEST(HttpFieldLine, StoresNameAndValue)
{
    HttpFieldLine f("Content-Type", "text/html");
    EXPECT_EQ(f.getFieldName(), "Content-Type");
    EXPECT_EQ(f.getFieldValue(), "text/html");
}

TEST(HttpRequestLine, StoresComponents)
{
    HttpRequestLine rl("GET", "/path", "HTTP/1.1");
    EXPECT_EQ(rl.getMethod(), "GET");
    EXPECT_EQ(rl.getRequestTarget(), "/path");
    EXPECT_EQ(rl.getHttpVersion(), "HTTP/1.1");
}
