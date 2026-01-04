#include "../include/Config.hpp"
#include "../include/Socket.hpp"
#include <gtest/gtest.h>

TEST(SocketTest, SocketInitializes)
{
    VirtualHost vh { "hey", "there", 42 };
    VirtualHost vh_c = vh;
    Config c { std::vector<VirtualHost> { vh, vh_c } };
    try {
        Socket s;
        s.start();
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
    SUCCEED() << "Socket is initialized";
}
