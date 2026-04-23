#pragma once
#include "Config.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"
#include "Handlers/ErrorRenderer.hpp"
#include "Handlers/StaticHandler.hpp"
#include "HttpRequest/HttpRequest.hpp"

class Webserver {
public:
    Webserver(const std::vector<VirtualHost>& config);
    ~Webserver();

    void init();
    void run();

private:
    const std::vector<VirtualHost>& config_;
    std::vector<Connection*> connections_;

    void handleRead_(EventManager& notifier, Connection& c);
    void handleWrite_(EventManager& notifier, Connection& c);
    void handleNewClient_(EventManager& manager, const Connection& conn);
    void handleClosedConn_(EventManager& manager, const Connection& conn);
};
