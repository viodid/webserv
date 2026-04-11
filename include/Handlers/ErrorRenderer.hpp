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

    /*
     * Returns the corresponding error HTML page as a string
     */
    std::string render(Location::ErrorPages status_code);
    /*
     * Returns a HttpResponse with a body error page
     */
    HttpResponse generateHttpResponse(const std::string http_version);

private:
    std::map<Location::ErrorPages, std::string> error_path_;

};
