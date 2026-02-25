#pragma once
#include <string>
#include <utility>
#include <vector>

enum AllowedMethods {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE
};

enum ErrorPages {
    E_400,
    E_403,
    E_404,
    E_500
};

struct Location {
    std::vector<AllowedMethods> allowed_methods;
    std::string redirection;
    std::string root;
    std::string default_file;
    bool directory_listing;
};

struct VirtualHost {
    const std::string hostname;
    const std::string port;
    const size_t socket_size;
    const std::vector<std::pair<ErrorPages, std::string> > error_pages;
    const std::vector<Location> locations;
};

struct Config {
    std::vector<VirtualHost> virtual_hosts;
};
