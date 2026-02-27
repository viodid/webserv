#include "../include/Webserver.hpp"
#include <cstdlib>

int main()
{
    Webserver webserv(create_mock_config().getVirtualHosts());
    webserv.init();
    webserv.run();
    return EXIT_SUCCESS;
}
