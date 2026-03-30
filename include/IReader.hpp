#pragma once

class IReader {
public:
    virtual ~IReader();
    virtual int read(char bufffer[], int len) = 0;
};
