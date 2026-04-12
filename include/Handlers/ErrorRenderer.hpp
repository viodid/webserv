#pragma once
#include "handler_utils.hpp"
#include "../Config.hpp"
#include "../Settings.hpp"
#include <cstring>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

class ErrorRenderer {
public:
    ErrorRenderer(const std::vector<std::pair<Location::StatusCodes, std::string> >& error_pages_);

    /*
     * Returns the corresponding error HTML page as a string
     */
    std::string render(Location::StatusCodes status_code) const;

private:
    std::map<Location::StatusCodes, std::string> error_path_;
};

