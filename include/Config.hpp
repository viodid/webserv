#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <utility>
#include <vector>

struct VirtualHost {
    const std::string hostname;
    const std::string port;
    const size_t socket_size;
};

struct Config {
    const std::vector<VirtualHost> virutal_hosts;
};
#endif
