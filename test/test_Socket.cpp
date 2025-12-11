#include <gtest/gtest.h>
#include "../include/Socket.hpp"

TEST(SocketTest, SocketInitializes) {
    try {
        Socket s("localhost");
        s.start();
    } catch (std::exception& e) {
        FAIL() << e.what();
    }
    std::cout << "hey" << std::endl;
    SUCCEED() << "Socket is initialized";
}
    
