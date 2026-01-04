#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <utility>
#include <vector>

enum AllowedMethods {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
};

struct Location {
    std::vector<AllowedMethods> allowed_methods;
    std::string redirection;
    std::string root;
    std::string default_file;
    bool directory_listing;
};

struct VirtualHost {
    std::string hostname;
    std::string port;
    size_t socket_size;
    std::vector<std::pair<int, std::string>> error_pages;
    std::vector<Location> locations;
};

struct Config {
    const std::vector<VirtualHost> virutal_hosts;
};
#endif
