#include "../include/Config.hpp"
#include "../include/Socket.hpp"
#include "../include/Webserver.hpp"
#include <gtest/gtest.h>

const Config create_config();

TEST(SocketTest, SocketInitializes)
{
    try {
        Socket s(create_config());
        Webserver ws(s);
        ws.start();
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
    SUCCEED() << "Socket is initialized";
}
