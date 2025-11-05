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
    if (close(m_sfd) != 0) {
        std::stringstream ss;
        ss << "[Error] closing sfd " << m_sfd;
        std::cerr << ss.str() << ": " << std::strerror(errno) << std::endl;
    }
#if DEBUG
    else 
        std::cout << "[Debug] success close sfd " << m_sfd << std::endl;
#endif
}

void Socket::create_bind_listen_(const std::string& addr) {

    struct addrinfo	hints, *result, *rp;

    memset(&hints, 0, sizeof(hints));
	hints.ai_family =  AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(addr.c_str(), "5555", &hints, &result) != 0)
        throw std::runtime_error(std::strerror(errno));

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		m_sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (m_sfd == -1)
			continue;
		if (bind(m_sfd, rp->ai_addr, rp->ai_addrlen) == 0)
		    break;
		close(m_sfd);
        throw std::runtime_error(std::strerror(errno));
	}
	if (rp == NULL)
        throw std::runtime_error(std::strerror(errno));
	if (listen(m_sfd, 0) == -1)
	{
		close(m_sfd);
        freeaddrinfo(result);
        throw std::runtime_error(std::strerror(errno));
	}
#if DEBUG
    std::cout << "[Debug] success listen on sfd " << m_sfd << std::endl;
#endif
}
