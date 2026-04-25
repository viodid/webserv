#pragma once
#include "../Interfaces/IBodySource.hpp"

class EmptyBodySource : public IBodySource {
public:
    virtual std::string nextChunk();
    virtual bool isEmpty();
};
