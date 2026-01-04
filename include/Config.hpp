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
};

struct VirtualHost {
    const std::string hostname;
    const std::string port;
    const size_t socket_size;
    const std::vector<std::pair<int, std::string>> error_pages;
};

struct Config {
    const std::vector<VirtualHost> virutal_hosts;
};
#endif
