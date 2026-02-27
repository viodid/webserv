#pragma once
#include "Config.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"
#include "Socket.hpp"

class Webserver {
public:
    Webserver(const std::vector<VirtualHost>& config);

    void init();
    void run();

private:
    const std::vector<VirtualHost>& config_;
    std::vector<Connection> connections_;

    void handleNewConnection_(EventManager& manager, const Connection& conn);
    void handleClientData_(EventManager& manager, const Connection& conn);
    void handleClosedConn_(EventManager& manager, const Connection& conn);
};
