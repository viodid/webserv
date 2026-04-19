#pragma once
#include "../Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

class File {
public:
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/MIME_types/Common_types
    enum Type {
        DIRECTORY,
        TEXT_HTML,
        TEXT_CSS,
        TEXT_JS,
        APP_JSON,
        IMAGE_JPEG,
        IMAGE_PNG,
        IMAGE_ICO
    };
    File(const std::string& path);

    const std::string& getPath() const;
    const Type& getType() const;
    std::string getTypeFormat() const;

    const std::string& readFile();
    bool isReadable() const;
    bool isWritable() const;
    bool isExecutable() const;

    static bool fileExists(const std::string& path);

private:
    std::string path_;
    Type type_;
    std::string content_;

    bool isDirectory_() const;
    File::Type mapFileType_() const;
};
