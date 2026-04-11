#include "../include/Webserver.hpp"
#include "../include/ConfigParser.hpp"
#include "../include/Settings.hpp"

int main()
{
    Config config = ConfigParser(Settings::DEFAULT_CONFIG_PATH).parse();
    Webserver webserv(config.getVirtualHosts());
    webserv.init();
    webserv.run();
    return EXIT_SUCCESS;
}
