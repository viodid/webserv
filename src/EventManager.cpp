#include "../include/EventManager.hpp"

EventManager::EventManager(const std::vector<Connection>& connections)
    : connections_(connections)
{
}

int EventManager::manage()
{
    nfds_t nfds = connections_.size();
    for (size_t i = 0; i < nfds; ++i)
        addPollFds(connections_[i].getSocket().getFd());
    int poll_count = poll(fds_.data(), nfds, -1);
    if (poll_count == -1)
        throw std::runtime_error(std::strerror(errno));

#if DEBUG
    std::cout << "poll_count: " << poll_count << std::endl
              << "pfds.size(): " << nfds << std::endl;
#endif

    return poll_count;
}

const std::vector<pollfd>& EventManager::getPollFds() const
{
    return fds_;
}

void EventManager::addPollFds(int fd)
{
    pollfd pfd = { fd, POLLIN, 0 };
    fds_.push_back(pfd);
}

void EventManager::removePollFds(int fd)
{
    for (size_t i = 0; i < connections_.size(); ++i) {
        if (connections_[i].getSocket().getFd() == fd) {
            if (i >= fds_.size()) {
                std::stringstream s;
                s << "Index '" << i << "' out of bounds of fds_\n";
                throw std::runtime_error(s.str());
            }
            fds_.erase(fds_.begin() + i);
            return;
        }
    }
    std::stringstream s;
    s << "File descriptor '" << fd << "' not found within fds_ vector\n";
    throw std::runtime_error(s.str());
}
