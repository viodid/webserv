#include "../include/Socket.hpp"

Socket::Socket()
{
    create_bind_listen_("127.0.0.1");
}

Socket::Socket(const std::string& addr)
{
    create_bind_listen_(addr);
}

Socket::~Socket()
{
    freeaddrinfo(m_addrinfo);
    if (m_sfd != -1 && close(m_sfd) != 0) {
        std::stringstream ss;
        ss << "[Error] closing sfd " << m_sfd;
        std::cerr << ss.str() << ": " << std::strerror(errno) << std::endl;
    }
    if (m_cfd != -1 && close(m_cfd) != 0) {
        std::stringstream ss;
        ss << "[Error] closing cfd " << m_cfd;
        std::cerr << ss.str() << ": " << std::strerror(errno) << std::endl;
    }
#if DEBUG
    else 
        std::cout << "[Debug] success close sfd " << m_sfd << std::endl;
#endif
}

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

void Socket::start() {
    m_cfd = accept(m_sfd, m_curraddr->ai_addr, &m_curraddr->ai_addrlen); // blocks
	if (m_cfd == -1)
        throw std::runtime_error(std::strerror(errno));
#if DEBUG
    std::cout << "[Debug] success accept on sfd " << m_sfd << std::endl;
#endif
}

