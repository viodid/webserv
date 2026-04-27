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
    // Block indefinitely when idle, but cap the wait when CGI processes are
    // in flight so the timeout sweep can run between poll() returns.
    int timeout_ms = cgi_fds_.empty() ? -1 : 1000;
    int poll_count = poll(fds_.data(), fds_.size(), timeout_ms);
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
    for (size_t i = 0; i < fds_.size(); ++i) {
        if (fds_[i].fd == fd) {
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
    return NULL;
}

void EventManager::addCgiFd(int fd, CgiProcess* cgi)
{
    addFd(fd);
    cgi_fds_[fd] = cgi;
}

void EventManager::removeCgiFd(int fd)
{
    cgi_fds_.erase(fd);
    removeFd(fd);
}

CgiProcess* EventManager::getCgiFor(int fd) const
{
    std::map<int, CgiProcess*>::const_iterator it = cgi_fds_.find(fd);
    if (it == cgi_fds_.end())
        return NULL;
    return it->second;
}

bool EventManager::hasCgi() const
{
    return !cgi_fds_.empty();
}
