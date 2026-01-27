#include "../include/Webserver.hpp"

Webserver::Webserver(Socket& socket)
    : socket_(socket)
{
}

void Webserver::start()
{
    for (;;) {
        nfds_t nfds = socket_.vh_config.size();
        std::vector<pollfd> fds;
        for (size_t i = 0; i < nfds; ++i)
            fds.push_back(socket_.vh_config[i].second);
        int poll_count = poll(fds.data(), nfds, -1);
        if (poll_count == -1)
            throw std::runtime_error(std::strerror(errno));

        // check if event/s are from a new connection (main socket from VH is hit)
        // or is an alrea_cpy opened one
        for (size_t i = 0; i < fds.size(); i++) {
            if (fds[i].revents == 0 || fds[i].fd == -1)
                continue;
            std::pair<VirtualHostConfig, pollfd>& tmp_pair = socket_.vh_config[i];
            if (fds[i].revents & POLLIN) {
                if (tmp_pair.first.is_vh_socket)
                    handleNewConn(tmp_pair.first);
                else
                    handleClientData(tmp_pair);
            } else if (fds[i].revents & POLLHUP)
                handleClosedConn(tmp_pair);
        }
        std::cout << "poll_count: " << poll_count << std::endl
                  << "pfds.size(): " << socket_.vh_config.size() << std::endl;
    }
}

/* creates another slot (socket) for a new connection */
void Webserver::handleNewConn(const VirtualHostConfig& vh)
{
    int cfd = accept(vh.socket, vh.curraddr->ai_addr, &vh.curraddr->ai_addrlen); // blocks
    if (cfd == -1)
        throw std::runtime_error(std::strerror(errno));
    socket_.vh_config.push_back(
        { VirtualHostConfig {
              vh.vh,
              cfd,
              false,
              nullptr,
              nullptr },
            pollfd { cfd, POLLIN, 0 } });
}

// TODO: primeagen 43:59
void Webserver::handleClientData(std::pair<VirtualHostConfig, pollfd>& tmp_pair)
{
    std::vector<char> buf(READ_SOCKET_SIZE);
    std::vector<char> data;
    data.reserve(READ_SOCKET_SIZE);
    int count = 0;
    while ((count = recv(tmp_pair.first.socket, buf.data(), READ_SOCKET_SIZE, MSG_DONTWAIT))) {
        std::cout << "count = " << count << " - errno: " << errno << std::endl;
        if (count == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
                throw std::runtime_error(std::strerror(errno));
        }
        data.insert(data.end(), buf.begin(), buf.end());
    }
    if (count == 0) // conn closed by client
        return handleClosedConn(tmp_pair);
    std::cout << data.data();
    std::cout << "here\n";

    // POST /potato HTTP/1.1
    // Host: localhost:5555
    // User-Agent: curl/8.5.0
    // Accept: */
    // Content-Type: application/patatas
    // Keep-Alive: 1
    // Content-Length: 10

    // {"hey": 4}here

    // parse header

    // parse body (if any)
    // stash rest of the message (if any)

    // TODO: implement complete send (not all the bytes may be send through the wire)
    if (send(tmp_pair.first.socket, "200 OK\nAllow: GET\n\n", 20, 0) == -1)
        throw std::runtime_error(std::strerror(errno));
}

void Webserver::handleClosedConn(std::pair<VirtualHostConfig, pollfd>& tmp_pair)
{
    int cfd = tmp_pair.first.socket;
    if (close(cfd) != 0) {
        std::cerr << "[Error] closing client fd " << cfd << ": "
                  << std::strerror(errno) << std::endl;
        throw std::runtime_error(std::strerror(errno));
    }
    tmp_pair.first.socket = -1;
    tmp_pair.second.fd = -1;
    std::cout << "closed conn fd: " << cfd << std::endl;
}
