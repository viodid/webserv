#pragma once
#include "../Exceptions.hpp"
#include "ChunkedDecoder.hpp"
#include <cstddef>
#include <string>
#include <vector>

class Body {
public:
    enum Mode {
        NoBody,
        ContentLengthMode,
        ChunkedMode
    };

    Body();
    ~Body();

    void setContentLength(size_t n);
    void setChunked();

    // 0 = unlimited.
    void setMaxBytes(size_t n);

    // Redirect body bytes to a file (truncates, mode 0644). Default storage
    // is an in-memory string. Throws on open failure. Must be called before
    // parse() consumes any body bytes.
    void useFile(const std::string& path);

    // Returns bytes consumed from buffer. Throws on framing/payload errors.
    int parse(const char* buffer, size_t buf_len);

    Mode getMode() const;
    bool isComplete() const;

    // In-memory body (empty when useFile() was called).
    const std::string& get() const;

    // Path on disk where the body was stored (empty for in-memory).
    const std::string& getStoredPath() const;

    std::string format() const;

private:
    Mode mode_;
    size_t content_length_;
    size_t bytes_received_;
    size_t max_bytes_;
    ChunkedDecoder chunked_;
    std::vector<char> decode_buf_;

    std::string body_;
    int file_fd_;
    std::string file_path_;
    bool complete_;

    Body(const Body&);
    Body& operator=(const Body&);

    void writeBytes_(const char* data, size_t len);
    void closeFd_();
    void abortFile_();
};
