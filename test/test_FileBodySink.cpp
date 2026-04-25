#include "../include/Exceptions.hpp"
#include "../include/HttpRequest/FileBodySink.hpp"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace {
std::string slurp(const std::string& path)
{
    std::ifstream in(path.c_str(), std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::string mkTmpDir()
{
    char tmpl[] = "/tmp/wsv-fbs-XXXXXX";
    return std::string(mkdtemp(tmpl));
}
}

TEST(FileBodySink, WritesBytesToDisk)
{
    std::string dir = mkTmpDir();
    {
        FileBodySink sink(dir, "out.bin", 0);
        sink.write("hello", 5);
        sink.write(" world", 6);
        sink.finalize();
        EXPECT_EQ(11U, sink.bytesWritten());
        EXPECT_EQ(dir + "/out.bin", sink.getStoredPath());
    }
    EXPECT_EQ("hello world", slurp(dir + "/out.bin"));
    unlink((dir + "/out.bin").c_str());
    rmdir(dir.c_str());
}

TEST(FileBodySink, ZeroLimitMeansUnlimited)
{
    std::string dir = mkTmpDir();
    {
        FileBodySink sink(dir, "big.bin", 0);
        std::string chunk(8192, 'a');
        for (int i = 0; i < 8; ++i)
            sink.write(chunk.data(), chunk.size());
        sink.finalize();
        EXPECT_EQ(8U * 8192U, sink.bytesWritten());
    }
    struct stat st;
    ASSERT_EQ(0, stat((dir + "/big.bin").c_str(), &st));
    EXPECT_EQ(8 * 8192, st.st_size);
    unlink((dir + "/big.bin").c_str());
    rmdir(dir.c_str());
}

TEST(FileBodySink, ThrowsAndRemovesFileWhenLimitExceeded)
{
    std::string dir = mkTmpDir();
    std::string path;
    {
        FileBodySink sink(dir, "limited.bin", 5);
        sink.write("abc", 3);
        path = sink.getStoredPath();
        EXPECT_THROW(sink.write("xyz", 3), ExceptionPayloadTooLarge);
    }
    struct stat st;
    EXPECT_NE(0, stat(path.c_str(), &st)) << "file should be removed on limit error";
    rmdir(dir.c_str());
}

TEST(FileBodySink, OpenFailureThrows)
{
    EXPECT_THROW(FileBodySink("/nonexistent-dir-xyz-123", "x", 0), std::exception);
}
