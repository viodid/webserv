#pragma once
#include "Connection.hpp"
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <sys/poll.h>
#include <vector>

class CgiProcess; // forward decl: avoids circular include

/**
 * @class EventManager
 * @brief Demultiplexer notifier for sockets and CGI pipes.
 *
 * Manages a single pollfd array for both client/listener sockets and CGI
 * stdout pipes. Owners are looked up via two parallel registries: the
 * connections vector (referenced from Webserver) and a CGI-fd map.
 *
 * @see https://man7.org/linux/man-pages/man2/poll.2.html
 */
class EventManager {
public:
    EventManager(std::vector<Connection*>& connections);

    int manage();
    const std::vector<pollfd>& getPollFds() const;

    void addFd(int fd);
    void removeFd(int fd);
    void enableWrite(int fd);
    void disableWrite(int fd);

    /*
     * Returns the connection owning fd, or NULL if it is not a connection
     * (e.g. it is a CGI stdout pipe).
     */
    Connection* getConnectionFor(int fd) const;

    /*
     * Register/unregister a CGI stdout pipe and its owning process.
     * addCgiFd internally calls addFd; removeCgiFd internally calls removeFd.
     */
    void addCgiFd(int fd, CgiProcess* cgi);
    void removeCgiFd(int fd);
    CgiProcess* getCgiFor(int fd) const;
    bool hasCgi() const;

private:
    const std::vector<Connection*>& connections_;
    std::vector<pollfd> fds_;
    std::map<int, CgiProcess*> cgi_fds_;
};
