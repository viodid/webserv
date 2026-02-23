#pragma once
#include "Connection.hpp"
#include <strstream>
#include <sys/poll.h>
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
     *
     * @return the number of connections ready to be handled
     */
    int manage();
    /**
     * @brief Returns the pollfd vector member variable as read only
     */
    const std::vector<pollfd>& getPollFds() const;
    /**
     * @brief Mutate the state of pollfd vector member viariable.
     * It adds one new pollfd struct to the vector initialized with POLLIN.
     */
    void addPollFds(int fd);
    /**
     * @brief Mutate the state of pollfd vector member viariable.
     * It removes a pollfd struct instance from the vector given a FD.
     */
    void removePollFds(int fd);

private:
    const std::vector<Connection>& connections_;
    std::vector<pollfd> fds_;
};
