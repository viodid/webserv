#include "../include/ConfigParser.hpp"
#include "../include/Settings.hpp"
#include "../include/Webserver.hpp"
#include <cstdlib>

int main()
{
    try {
        Config config = ConfigParser(Settings::DEFAULT_CONFIG_PATH).parse();
        Webserver webserv(config.getVirtualHosts());
        webserv.init();
        webserv.run();
    } catch (const ExceptionParserError& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
