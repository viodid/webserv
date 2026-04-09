#pragma once
#include "Config.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"
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
    std::vector<HttpRequest*> requests_;

    void handleNewConnection_(EventManager& manager, const Connection& conn);
    void handleClientData_(EventManager& notifier, Connection& connection, HttpRequest& request);
    void handleClosedConn_(EventManager& manager, const Connection& conn);
};
