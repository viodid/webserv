#include "../include/Exceptions.hpp"
#include "../include/HttpRequest/ChunkedDecoder.hpp"
#include "../include/HttpRequest/MemoryBodySink.hpp"
#include <gtest/gtest.h>
#include <string>

namespace {
// Drives the decoder one byte at a time across the whole input. Returns
// total bytes consumed (should equal input.size() if input contains a complete
// chunked stream).
size_t feedByteByByte(ChunkedDecoder& d, IBodySink& sink, const std::string& input)
{
    size_t total = 0;
    for (size_t i = 0; i < input.size() && !d.isDone(); ++i) {
        size_t n = d.feed(input.data() + i, 1, sink);
        // 1-byte feeds may return 0 if the byte didn't complete a state.
        total += n;
    }
    // After byte-by-byte feed, decoder may have consumed all bytes spread
    // across multiple states; sum of returned sizes should be ≤ input size.
    return total;
}
}

TEST(ChunkedDecoder, SingleChunkBigBuffer)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    // "5\r\nhello\r\n0\r\n\r\n"
    std::string input = "5\r\nhello\r\n0\r\n\r\n";
    size_t n = d.feed(input.data(), input.size(), sink);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ(input.size(), n);
    EXPECT_EQ("hello", sink.getBody());
}

TEST(ChunkedDecoder, MultipleChunks)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    std::string input = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
    size_t n = d.feed(input.data(), input.size(), sink);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ(input.size(), n);
    EXPECT_EQ("hello world", sink.getBody());
}

TEST(ChunkedDecoder, HexSizesUpperLower)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    // 0x10 = 16
    std::string input = "10\r\n0123456789abcdef\r\nA\r\nABCDEFGHIJ\r\n0\r\n\r\n";
    d.feed(input.data(), input.size(), sink);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("0123456789abcdefABCDEFGHIJ", sink.getBody());
}

TEST(ChunkedDecoder, ByteByByte)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    std::string input = "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
    feedByteByByte(d, sink, input);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("hello world", sink.getBody());
}

TEST(ChunkedDecoder, EmptyBody)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    std::string input = "0\r\n\r\n";
    size_t n = d.feed(input.data(), input.size(), sink);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ(input.size(), n);
    EXPECT_EQ("", sink.getBody());
}

TEST(ChunkedDecoder, ChunkExtensionsIgnored)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    std::string input = "5;name=value\r\nhello\r\n0\r\n\r\n";
    d.feed(input.data(), input.size(), sink);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("hello", sink.getBody());
}

TEST(ChunkedDecoder, TrailerHeadersIgnored)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    std::string input = "5\r\nhello\r\n0\r\nX-Trailer: foo\r\n\r\n";
    d.feed(input.data(), input.size(), sink);
    EXPECT_TRUE(d.isDone());
    EXPECT_EQ("hello", sink.getBody());
}

TEST(ChunkedDecoder, MalformedSizeThrows)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    std::string input = "ZZ\r\nfoo\r\n";
    EXPECT_THROW(d.feed(input.data(), input.size(), sink), ExceptionBadFraming);
}

TEST(ChunkedDecoder, MissingCrlfAfterDataThrows)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    // 5 bytes "hello" then "XX" instead of CRLF
    std::string input = "5\r\nhelloXX0\r\n\r\n";
    EXPECT_THROW(d.feed(input.data(), input.size(), sink), ExceptionBadFraming);
}

TEST(ChunkedDecoder, IsNotDoneUntilTerminator)
{
    ChunkedDecoder d;
    MemoryBodySink sink(0);
    std::string input = "5\r\nhello\r\n";
    d.feed(input.data(), input.size(), sink);
    EXPECT_FALSE(d.isDone());
    EXPECT_EQ("hello", sink.getBody());

    std::string rest = "0\r\n\r\n";
    d.feed(rest.data(), rest.size(), sink);
    EXPECT_TRUE(d.isDone());
}

TEST(ChunkedDecoder, EnforcesSinkLimit)
{
    ChunkedDecoder d;
    MemoryBodySink sink(3); // tight cap
    std::string input = "5\r\nhello\r\n0\r\n\r\n";
    EXPECT_THROW(d.feed(input.data(), input.size(), sink), ExceptionPayloadTooLarge);
}
