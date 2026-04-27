#pragma once
#include "Config.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"
#include "Handlers/ErrorRenderer.hpp"
#include "Handlers/HandlerFactory.hpp"
#include "Handlers/StaticHandler.hpp"
#include "HttpRequest/HttpRequest.hpp"
#include <set>

class Webserver {
public:
    Webserver(const std::vector<VirtualHost>& config);
    ~Webserver();

    void init();
    void run();

private:
    const std::vector<VirtualHost>& config_;
    std::vector<Connection*> connections_;

    void handleRead_(EventManager& notifier, Connection& c, std::set<int>& dead_fds);
    void handleWrite_(EventManager& notifier, Connection& c, std::set<int>& dead_fds);
    void handleNewClient_(EventManager& manager, const Connection& conn);
    void markClosed_(std::set<int>& dead_fds, const Connection& c);
    void destroyConnection_(EventManager& manager, int fd);
    void handleCgiReadable_(EventManager& notifier, int fd);
    void finalizeCgi_(EventManager& notifier, Connection& c);
    void sweepCgiTimeouts_(EventManager& notifier);
};
