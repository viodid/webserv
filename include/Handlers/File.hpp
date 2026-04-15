#pragma once
#include "../Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>

class File {
public:
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/MIME_types/Common_types
    enum Type {
        TEXT_HTML,
        TEXT_CSS,
        TEXT_JS,
        APP_JSON,
        IMAGE_JPEG,
        IMAGE_PNG,
        IMAGE_ICO
    };
    File(const std::string& path);
    File(const HttpRequest& request, const Location& location);

    const std::string& getPath() const;
    const Type& getType() const;
    std::string getTypeFormat() const;

    const std::string& readFile();
    bool isReadable() const;

private:
    const std::string path_;
    const Type type_;
    std::string content_;
};
