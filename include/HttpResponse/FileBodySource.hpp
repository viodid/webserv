#pragma once
#include "../Interfaces/IBodySource.hpp"
#include "../Settings.hpp"
#include <string>

class FileBodySource : public IBodySource {
public:
    FileBodySource(const std::string& path);
    virtual ~FileBodySource();

    virtual std::string nextChunk();
    virtual bool isEmpty();

private:
    int fd_;
    bool eof_;

    FileBodySource(const FileBodySource&);
    FileBodySource& operator=(const FileBodySource&);
};
