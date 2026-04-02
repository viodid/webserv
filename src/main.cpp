#include "../include/Webserver.hpp"
#include <cstdlib>

int main()
{
    Config config = create_mock_config();
    Webserver webserv(config.getVirtualHosts());
    webserv.init();
    webserv.run();
    return EXIT_SUCCESS;
}