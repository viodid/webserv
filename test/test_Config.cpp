#include "../include/Config.hpp"
#include <gtest/gtest.h>
#include <vector>

const Config create_config()
{
    return Config {
        std::vector<VirtualHost> {
            VirtualHost {
                "localhost",
                "5555",
                100000,
                std::vector<std::pair<ErrorPages, std::string>> {},
                std::vector<Location> {
                    Location {
                        std::vector { GET, HEAD, POST, PUT, DELETE },
                        "",
                        "/var/www/html",
                        "/var/www/html/403.html",
                        false,
                    },
                },
            },
            VirtualHost {
                "localhost",
                "42069",
                100000,
                std::vector<std::pair<ErrorPages, std::string>> {},
                std::vector<Location> {
                    Location {
                        std::vector { GET, POST },
                        "",
                        "/var/www/html",
                        "/var/www/html/403.html",
                        false,
                    },
                },
            },
        },
    };
}

TEST(ConfigTest, ConfigWorks)
{
    const Config conf = create_config();
    SUCCEED() << "Config file created";
}
