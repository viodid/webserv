#pragma once
#include "../Interfaces/IBodySource.hpp"

class StringBodySource : public IBodySource {
public:
    StringBodySource(const std::string& content);

    virtual std::string nextChunk();
    virtual bool isEmpty();

private:
    std::string content_;
    bool consumed_;
};
