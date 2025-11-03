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
        std::sstream ss;
        ss << "[Error] closing sfd " << m_sfd;
        cerr << ss.str() << ": " << std::strerr(errno) << std::endl;
    }
#if DEGBUG
    std::cout << [Debug] success close sfd " << m_sfd << std::endl;
#endif
}

// TODO: raise exceptions instead of exit
Socket::create_bind_listen_(const std::string& addr) {

    struct addrinfo	hints, *result, *rp;

    memset(&hints, 0, sizeof(hints));
	hints.ai_family =  AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(add.c_str(), "5555", &hints, &result) != 0)
	{
		perror("getaddrinfo");
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		close(sfd);
	}
	if (rp == NULL)
	{
		perror("Could not bind");
		exit(EXIT_FAILURE);
	}
	if (listen(sfd, 0) == -1)
	{
		perror("listen");
		close(sfd);
        freeaddrinfo(result);
		exit(EXIT_FAILURE);
	}
}
