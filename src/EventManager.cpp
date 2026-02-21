#include "../include/EventManager.hpp"

EventManager::EventManager(const std::vector<Connection>& connections)
    : connections_(connections)
{
}

EventManager::~EventManager() { }

int EventManager::manage()
{
    nfds_t nfds = connections_.size();
    std::vector<pollfd> fds;
    for (size_t i = 0; i < nfds; ++i)
        fds.push_back(connections_[i].socket.getFd());
    int poll_count = poll(fds.data(), nfds, -1);
    if (poll_count == -1)
        throw std::runtime_error(std::strerror(errno));

#if DEBUG
    std::cout << "poll_count: " << poll_count << std::endl
              << "pfds.size(): " << nfds << std::endl;
#endif

    return poll_count;
}
