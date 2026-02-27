#include "../include/Config.hpp"
#include <gtest/gtest.h>

TEST(ConfigTest, ConfigWorks)
{
    const Config conf = create_mock_config();
    SUCCEED() << "Config file created";
}
