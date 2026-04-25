#include "../include/Exceptions.hpp"
#include "../include/HttpRequest/ChunkedDecoder.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace {
std::string str(const std::vector<char>& v)
{
    return std::string(v.begin(), v.end());
}
}

TEST(ChunkedDecoder, SingleChunkBigBuffer)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "5\r\nhello\r\n0\r\n\r\n";
    size_t n = d.feed(input.data(), input.size(), out);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ(input.size(), n);
    EXPECT_EQ("hello", str(out));
}

TEST(ChunkedDecoder, MultipleChunks)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
    size_t n = d.feed(input.data(), input.size(), out);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ(input.size(), n);
    EXPECT_EQ("hello world", str(out));
}

TEST(ChunkedDecoder, HexSizesUpperLower)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "10\r\n0123456789abcdef\r\nA\r\nABCDEFGHIJ\r\n0\r\n\r\n";
    d.feed(input.data(), input.size(), out);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("0123456789abcdefABCDEFGHIJ", str(out));
}

TEST(ChunkedDecoder, ByteByByte)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
    for (size_t i = 0; i < input.size() && !d.isDone(); ++i)
        d.feed(input.data() + i, 1, out);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("hello world", str(out));
}

TEST(ChunkedDecoder, EmptyBody)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "0\r\n\r\n";
    size_t n = d.feed(input.data(), input.size(), out);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ(input.size(), n);
    EXPECT_EQ("", str(out));
}

TEST(ChunkedDecoder, ChunkExtensionsIgnored)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "5;name=value\r\nhello\r\n0\r\n\r\n";
    d.feed(input.data(), input.size(), out);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("hello", str(out));
}

TEST(ChunkedDecoder, TrailerHeadersIgnored)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "5\r\nhello\r\n0\r\nX-Trailer: foo\r\n\r\n";
    d.feed(input.data(), input.size(), out);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("hello", str(out));
}

TEST(ChunkedDecoder, MalformedSizeThrows)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "ZZ\r\nfoo\r\n";
    EXPECT_THROW(d.feed(input.data(), input.size(), out), ExceptionBadFraming);
}

TEST(ChunkedDecoder, MissingCrlfAfterDataThrows)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "5\r\nhelloXX0\r\n\r\n";
    EXPECT_THROW(d.feed(input.data(), input.size(), out), ExceptionBadFraming);
}

TEST(ChunkedDecoder, IsNotDoneUntilTerminator)
{
    ChunkedDecoder d;
    std::vector<char> out;
    std::string input = "5\r\nhello\r\n";
    d.feed(input.data(), input.size(), out);
    EXPECT_FALSE(d.isDone());
    EXPECT_EQ("hello", str(out));

    std::string rest = "0\r\n\r\n";
    d.feed(rest.data(), rest.size(), out);
    EXPECT_TRUE(d.isDone());
}
