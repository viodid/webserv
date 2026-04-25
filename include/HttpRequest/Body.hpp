#pragma once
#include "../Exceptions.hpp"
#include "../Interfaces/IBodySink.hpp"
#include "ChunkedDecoder.hpp"
#include <cstddef>
#include <string>

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

    // Take ownership of the sink. Subsequent decoded bytes are written to it.
    void installSink(IBodySink* sink);
    bool hasSink() const;

    Mode getMode() const;
    bool isComplete() const;

    // Feed buffer bytes; returns bytes consumed. Throws on framing/payload
    // errors.
    int parse(const char* buffer, size_t buf_len);

    // Returns the decoded body iff a MemoryBodySink is installed; otherwise an
    // empty string. Kept for compatibility with existing callers/tests.
    const std::string& get() const;

    const IBodySink* getSink() const;

    std::string format() const;

private:
    Mode mode_;
    size_t content_length_;
    size_t bytes_received_;
    ChunkedDecoder chunked_;
    IBodySink* sink_;
    bool complete_;

    Body(const Body&);
    Body& operator=(const Body&);
};
