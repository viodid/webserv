#pragma once
#include "../Exceptions.hpp"
#include "ChunkedDecoder.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

class Body {
public:
    Body();
    Body(const std::string& body);

    const std::string& get() const;
    void set(const std::string&);

    int parse(const char* buffer, size_t buf_len, const std::string& content_len);
    int parseChunked(const char* buffer, size_t buf_len,
        const std::string& upload_store,
        const std::string& target);
    bool isChunkedDone() const;
    const std::string& getStoredPath() const;

    std::string format() const;

private:
    std::string body_;
    size_t content_length_;
    ChunkedDecoder decoder_;
    std::ofstream out_file_;
    std::string stored_path_;
    bool chunked_inited_;
};
