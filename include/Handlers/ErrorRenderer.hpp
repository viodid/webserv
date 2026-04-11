#pragma once
#include "../Config.hpp"
#include "../Settings.hpp"
#include "../Utils.hpp"
#include <cstring>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

class ErrorRenderer {
public:
    ErrorRenderer(const std::vector<std::pair<Location::ErrorPages, std::string> >& error_pages_);

    std::string render(Location::ErrorPages status_code);

private:
    std::map<Location::ErrorPages, std::string> error_path_;
};
