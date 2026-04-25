#include "../include/Exceptions.hpp"
#include "../include/HttpRequest/MemoryBodySink.hpp"
#include <gtest/gtest.h>

TEST(MemoryBodySink, AcceptsBytesUpToLimit)
{
    MemoryBodySink sink(100);
    sink.write("hello", 5);
    sink.write(" world", 6);
    sink.finalize();
    EXPECT_EQ(11U, sink.bytesWritten());
    EXPECT_EQ("hello world", sink.getBody());
}

TEST(MemoryBodySink, ZeroLimitMeansUnlimited)
{
    MemoryBodySink sink(0);
    std::string big(1024 * 1024, 'x');
    sink.write(big.data(), big.size());
    sink.finalize();
    EXPECT_EQ(big.size(), sink.bytesWritten());
}

TEST(MemoryBodySink, ThrowsWhenLimitExceeded)
{
    MemoryBodySink sink(5);
    sink.write("abc", 3);
    EXPECT_THROW(sink.write("xyz", 3), ExceptionPayloadTooLarge);
}

TEST(MemoryBodySink, ExactLimitOk)
{
    MemoryBodySink sink(5);
    sink.write("hello", 5);
    sink.finalize();
    EXPECT_EQ("hello", sink.getBody());
}

TEST(MemoryBodySink, GetStoredPathEmpty)
{
    MemoryBodySink sink(0);
    EXPECT_EQ("", sink.getStoredPath());
}
