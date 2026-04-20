#pragma once

class IReader {
public:
    virtual ~IReader() { };
    virtual int read(char buffer[], int len) = 0;
};
