#pragma once
#include <cstddef>
#include <string>
#include <vector>

// RFC 9112 §7.1 Transfer-Encoding: chunked decoder.
class ChunkedDecoder {
public:
    ChunkedDecoder();

    /* Consumes up to `len` bytes from `buf`, appending decoded body bytes to
     * `out`. Returns the number of bytes consumed from `buf`. Throws
     * ExceptionBadFraming on malformed framing.
     */
    size_t feed(const char* buf, size_t len, std::vector<char>& out);

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
