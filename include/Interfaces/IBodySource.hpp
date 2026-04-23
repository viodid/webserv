#pragma once
#include <string>

class IBodySource {
public:
    virtual ~IBodySource();

    virtual std::string nextChunk() = 0;
    virtual bool isEmpty() = 0;
};
