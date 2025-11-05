#include <gtest/gtest.h>
#include "../include/Socket.hpp"

TEST(SocketTest, SocketInitializes) {
    Socket s("localhost");
    SUCCEED() << "Socket is initialized";
}
    
