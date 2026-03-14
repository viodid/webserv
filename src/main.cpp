#include "../include/ConfigParser.hpp"
#include "../include/Webserver.hpp"
#include <cstdlib>
#include <iostream>

#define DEFAULT_CONFIG "resources/default.conf"

int main(int argc, char* argv[])
{
    std::string config_path = DEFAULT_CONFIG;
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
        return EXIT_FAILURE;
    }
    if (argc == 2)
        config_path = argv[1];

    try {
        ConfigParser parser(config_path);
        Config config = parser.parse();

        std::cout << "Config loaded: " << config.getVirtualHosts().size()
                  << " server(s) from " << config_path << std::endl;

        Webserver webserv(config.getVirtualHosts());
        webserv.init();
        webserv.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
