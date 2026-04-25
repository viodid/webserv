#pragma once
#include "../Interfaces/IBodySink.hpp"
#include <cstddef>
#include <string>

// RFC 9112 §7.1 Transfer-Encoding: chunked decoder.
// Stream-friendly: feed bytes as they arrive across multiple calls.
class ChunkedDecoder {
public:
    ChunkedDecoder();

    // Consumes up to `len` bytes from `buf`, writing decoded body bytes to
    // `sink`. Returns the number of bytes consumed from `buf`. Throws
    // ExceptionBadFraming on malformed framing.
    size_t feed(const char* buf, size_t len, IBodySink& sink);

    bool isDone() const;

private:
    enum State {
        SizeLine,
        Data,
        DataCrlf,
        Trailer,
        Done_
    };

    State state_;
    size_t chunk_remaining_;
    std::string partial_line_;
    int crlf_seen_;

    void parseSize_();
};
