#pragma once
#include "../Interfaces/IBodySink.hpp"

class MemoryBodySink : public IBodySink {
public:
    // max_bytes == 0 means unlimited.
    explicit MemoryBodySink(size_t max_bytes);
    virtual ~MemoryBodySink();

    virtual void write(const char* data, size_t len);
    virtual void finalize();
    virtual size_t bytesWritten() const;
    virtual const std::string& getStoredPath() const;

    const std::string& getBody() const;

private:
    std::string body_;
    size_t max_bytes_;
    std::string empty_path_;
};
