#pragma once
#include "Connection.hpp"
#include <vector>

/**
 * @class EventManager
 * @brief Demultiplexer notifier for sockets
 *
 * This class manages a vector of connections and calls the `poll` system call
 * on all the connections file descriptors.
 *
 * @see https://man7.org/linux/man-pages/man2/poll.2.html
 */
class EventManager {
public:
    /**
     * @brief Initializes an EventManger object.
     *
     * @param connections The connections vector to manage by this obj.
     */
    EventManager(const std::vector<Connection>& connections);

    /**
     * @brief Calls `poll` system call and waits for one or more socket
     * connections to happen.
     */
    int manage();

private:
    const std::vector<Connection>& connections_;
};
