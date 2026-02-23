#include "../include/Config.hpp"
#include "../include/Socket.hpp"
// #include "../include/Webserver.hpp"
#include <gtest/gtest.h>

const Config create_config();

TEST(SocketTest, SocketInitializes)
{
    try {
        Socket s("localhost", "5555");
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
    try {
        Socket s("localhost", "5555");
        s.acceptConn();
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
}
