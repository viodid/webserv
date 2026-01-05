#include "../include/Config.hpp"
#include "../include/Socket.hpp"
#include <gtest/gtest.h>

TEST(SocketTest, SocketInitializes)
{
    try {
        Socket s;
        //s.start();
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
    SUCCEED() << "Socket is initialized";
}
