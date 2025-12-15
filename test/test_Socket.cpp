#include <gtest/gtest.h>
#include "../include/Socket.hpp"

TEST(SocketTest, SocketInitializes) {
    try {
        Socket s;
        s.start();
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
    SUCCEED() << "Socket is initialized";
}
    
