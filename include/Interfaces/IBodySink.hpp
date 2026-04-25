#pragma once
#include <cstddef>
#include <string>

class IBodySink {
public:
    virtual ~IBodySink() { }

    // Append `len` bytes to the sink. Throws ExceptionPayloadTooLarge if the
    // total bytes written would exceed the configured limit.
    virtual void write(const char* data, size_t len) = 0;

    // Flush / close the sink. Idempotent.
    virtual void finalize() = 0;

    virtual size_t bytesWritten() const = 0;

    // Path on disk where the body was stored, or "" for in-memory sinks.
    virtual const std::string& getStoredPath() const = 0;
};
