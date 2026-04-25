#pragma once
#include "../Interfaces/IBodySink.hpp"

class FileBodySink : public IBodySink {
public:
    // Opens dir + "/" + filename for writing (truncates). Throws on open
    // failure. max_bytes == 0 means unlimited.
    FileBodySink(const std::string& dir, const std::string& filename, size_t max_bytes);
    virtual ~FileBodySink();

    virtual void write(const char* data, size_t len);
    virtual void finalize();
    virtual size_t bytesWritten() const;
    virtual const std::string& getStoredPath() const;

private:
    std::string path_;
    int fd_;
    size_t bytes_written_;
    size_t max_bytes_;

    FileBodySink(const FileBodySink&);
    FileBodySink& operator=(const FileBodySink&);

    void closeFd_();
    void unlinkFile_();
};
