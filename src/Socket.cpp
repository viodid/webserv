#include "../include/Socket.hpp"

Socket::Socket()
{
    create_bind_listen_("127.0.0.1");
}

Socket::~Socket()
{
    freeaddrinfo(m_addrinfo);
    if (m_sfd != -1 && close(m_sfd) != 0)
        std::cerr << "[Error] closing sfd " << m_sfd << ": " << std::strerror(errno) << std::endl;
#if DEBUG
    else 
        std::cout << "[Debug] success close sfd " << m_sfd << std::endl;
#endif
}

Socket::Socket(const Socket& cp) {
    m_sfd = cp.m_sfd;
    m_addrinfo = cp.m_addrinfo;
    m_curraddr = cp.m_curraddr;
}

Socket& Socket::operator=(const Socket& cp) {
    if (this != &cp) {
        m_sfd = cp.m_sfd;
        m_addrinfo = cp.m_addrinfo;
        m_curraddr = cp.m_curraddr;
    }
    return *this;
}

void Socket::start() {
    pollfd pfd {m_sfd, POLLIN, 0};
    // initialize with just the socket fd
    std::vector<pollfd> pfds {pfd};
    
    for (;;) {

        nfds_t nfds = pfds.size();
        int poll_count = poll(pfds.data(), nfds, -1);
        if (poll_count == -1) throw std::runtime_error(std::strerror(errno));

        // check if event/s are from a new connection (main socket fd is hit)
        // or is an already opened one
        for (int i = 0; i < static_cast<int>(nfds); i++) {
            pollfd tmp_pfd = pfds[i];
            if (tmp_pfd.revents & POLLIN) {
                if (tmp_pfd.fd == m_sfd)
                    handle_new_conn_(pfds);
                else
                    handle_existing_conn_(tmp_pfd.fd, pfds);
            } else if (tmp_pfd.revents & POLLHUP)
                handle_closed_conn_(tmp_pfd.fd, pfds);
        }

        std::cout << "poll_count: " << poll_count << std::endl
            << "pfds.size(): " << pfds.size() << std::endl;
    }
}

// handle new connections
// connect (returns new socket)
// create another slot for new connection
void Socket::handle_new_conn_(std::vector<pollfd>& pfds) {
    int cfd = accept(m_sfd, m_curraddr->ai_addr, &m_curraddr->ai_addrlen); // blocks
	if (cfd == -1)
        throw std::runtime_error(std::strerror(errno));
    pfds.push_back(pollfd{cfd, POLLIN, 0});
}

void Socket::handle_existing_conn_(int fd, std::vector<pollfd>& pfds) {
    char buf[SOCKET_MSG_BUFFER+1]; // +1 for null terminator if buffer is full
    int count = recv(fd, buf, SOCKET_MSG_BUFFER, 0);
    if (count == -1)
        throw std::runtime_error(std::strerror(errno));
    if (!count) // conn closed by client
        return handle_closed_conn_(fd, pfds);
    buf[count] = '\0';
    std::cout << buf << std::endl;
    // TODO: implement complete send (not all the bytes may be send through the wire)
    if (send(fd, "hi! i'm a web server :)\n", 24, 0) == -1)
        throw std::runtime_error(std::strerror(errno));

}

void Socket::handle_closed_conn_(int cfd, std::vector<pollfd>& pfds) {
    if (close(cfd) != 0) {
        std::cerr << "[Error] closing client fd " << cfd << ": "
            << std::strerror(errno) << std::endl;
        throw std::runtime_error(std::strerror(errno));
    }

    for (std::vector<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it)
        if ((*it).fd == cfd) {
            pfds.erase(it);
            break;
        }
    std::cout << "closed conn fd: " << cfd << std::endl;
}

// creates a socket and assigns it to member variable m_sfd
// TODO: hardcode loopback addr?
void Socket::create_bind_listen_(const std::string& addr) {

    struct addrinfo	hints;

    memset(&hints, 0, sizeof(hints));
	hints.ai_family =  AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(addr.c_str(), SOCKET_PORT, &hints, &m_addrinfo) != 0)
        throw std::runtime_error(std::strerror(errno));

	for (m_curraddr = m_addrinfo; m_curraddr != nullptr; m_curraddr = m_curraddr->ai_next)
	{
		m_sfd = socket(m_curraddr->ai_family, m_curraddr->ai_socktype, m_curraddr->ai_protocol);
		if (m_sfd == -1)
			continue;
        int yes = 1;
        if (setsockopt(m_sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            close(m_sfd);
            throw std::runtime_error(std::strerror(errno));
        }
		if (bind(m_sfd, m_curraddr->ai_addr, m_curraddr->ai_addrlen) == 0)
		    break;
		close(m_sfd);
        throw std::runtime_error(std::strerror(errno));
	}
	if (m_curraddr == nullptr)
        throw std::runtime_error(std::strerror(errno));
	if (listen(m_sfd, 0) == -1) {
		close(m_sfd);
        freeaddrinfo(m_addrinfo);
        throw std::runtime_error(std::strerror(errno));
	}
#if DEBUG
    std::cout << "[Debug] success listen on sfd " << m_sfd << std::endl;
#endif
}

