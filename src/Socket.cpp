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
    // TODO: handle m_cfd
    freeaddrinfo(m_addrinfo);
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

    struct addrinfo	hints;

    memset(&hints, 0, sizeof(hints));
	hints.ai_family =  AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(addr.c_str(), "5555", &hints, &m_adrrinfo) != 0)
        throw std::runtime_error(std::strerror(errno));

	for (m_curraddr = m_addrinfo; curraddr != NULL; curraddr = curraddr->ai_next)
	{
		m_sfd = socket(curraddr->ai_family, curraddr->ai_socktype, curraddr->ai_protocol);
		if (m_sfd == -1)
			continue;
		if (bind(m_sfd, curraddr->ai_addr, curraddr->ai_addrlen) == 0)
		    break;
		close(m_sfd);
        throw std::runtime_error(std::strerror(errno));
	}
	if (curraddr == NULL)
        throw std::runtime_error(std::strerror(errno));
	if (listen(m_sfd, 0) == -1)
	{
		close(m_sfd);
        freeaddrinfo(m_addrinfo);
        throw std::runtime_error(std::strerror(errno));
	}
#if DEBUG
    std::cout << "[Debug] success listen on sfd " << m_sfd << std::endl;
#endif
}

void Socket::connect() const {
    m_cfd = accept(m_sfd, m_curraddr->ai_addr, &m_curraddr->ai_addrlen);
    // TODO: from here
	if (m_cfd == -1)
	{
		perror("accept");
		close(sfd);
        freeaddrinfo(result);
		exit(EXIT_FAILURE);
	}
	write(cfd, "hey there!\n", 12);

}
