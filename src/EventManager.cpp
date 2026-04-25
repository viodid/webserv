#include "../include/EventManager.hpp"
#include <sys/poll.h>

EventManager::EventManager(std::vector<Connection*>& connections)
    : connections_(connections)
{
    for (size_t i = 0; i < connections_.size(); ++i)
        addFd(connections_[i]->getFd());
}

int EventManager::manage()
{
    int poll_count = poll(fds_.data(), connections_.size(), -1);
    if (poll_count == -1)
        throw std::runtime_error(std::strerror(errno));

    return poll_count;
}

const std::vector<pollfd>& EventManager::getPollFds() const
{
    return fds_;
}

void EventManager::addFd(int fd)
{
    pollfd pfd = { fd, POLLIN, 0 };
    fds_.push_back(pfd);
}

void EventManager::enableWrite(int fd)
{
    for (size_t i = 0; i < fds_.size(); ++i) {
        if (fds_[i].fd == fd) {
            fds_[i].events |= POLLOUT;
            return;
        }
    }
}

void EventManager::disableWrite(int fd)
{
    for (size_t i = 0; i < fds_.size(); ++i) {
        if (fds_[i].fd == fd) {
            fds_[i].events &= ~POLLOUT;
            return;
        }
    }
}

void EventManager::removeFd(int fd)
{
    for (size_t i = 0; i < connections_.size(); ++i) {
        if (connections_[i]->getFd() == fd) {
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

Connection* EventManager::getConnectionFor(int fd) const
{
    for (std::vector<Connection*>::const_iterator it = connections_.begin();
        it != connections_.end();
        it++) {
        if ((*it)->getFd() == fd)
            return *it;
    }
    throw std::runtime_error("connection not found");
}
