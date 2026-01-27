#pragma once
#include "HttpParserRequest.hpp"
#include "Socket.hpp"

class Webserver {
public:
    Webserver(Socket&); // TODO: remove
    Webserver(Socket&, HttpParserRequest&);

    void start();

private:
    Socket& socket_;
    // const HttpParserRequest httpParser_;

    void handleNewConn(const VirtualHostConfig& vh);
    void handleClientData(std::pair<VirtualHostConfig, pollfd>& tmp_pair);
    void handleClosedConn(std::pair<VirtualHostConfig, pollfd>& tmp_pair);
};
