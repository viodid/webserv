#include <gtest/gtest.h>
#include "../include/Socket.hpp"

TEST(SocketTest, SocketInitializes) {
    try {
        Socket s("localhost");
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
    SUCCEED() << "Socket is initialized";
}
    
