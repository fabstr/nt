#include "net.h"

void sigchld_handler(int s)
{
	while (waitpid(-1, NULL, WNOHANG) > 0) ;
}

void* getInAddr(struct sockaddr *sa)
{
	if (sa -> sa_family == AF_INET) {
		return &(((struct sockaddr_in *) sa) -> sin_addr);
	}

	return &(((struct sockaddr_in6 *) sa) -> sin6_addr);
}

void setAddrinfoHints(struct addrinfo *hints)
{
	memset(hints, 0, sizeof(struct addrinfo));
	hints -> ai_family = AF_UNSPEC;
	hints -> ai_socktype = SOCK_STREAM;
	hints -> ai_flags = AI_PASSIVE;
}

int bindTo(char *host, char *port)
{
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *p;

	setAddrinfoHints(&hints);

	int rv = getaddrinfo(host, port, &hints, &servinfo);
	if (rv != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	int sockfd;
	int yes = 1;
	for (p = servinfo; p != NULL; p = p -> ai_next) {
		sockfd = socket(p -> ai_family, p -> ai_socktype, p -> ai_protocol);
		if (sockfd == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			continue;
		}

		if (bind(sockfd, p -> ai_addr, p -> ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;

	}

	freeaddrinfo(servinfo);

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return -1;
	}

	return sockfd;
}

// bind and listen, return the socket
int openListenSocket(char *host, char *port)
{
	int sock = bindTo(host, port);
	if (listen(sock, BACKLOG) == -1) {
		close(sock);
		return -1;
	}

	return sock;
}

/*int main()*/
/*{*/
	/*struct sockaddr_storage their_addr;*/
	/*socklen_t sin_size;*/
	/*struct sigaction sa;*/
	/*char s[INET6_ADDRSTRLEN];*/

	/*int sockfd = openListenSocket("192.168.1.20", "1234");*/

	/*sa.sa_handler = sigchld_handler;*/
	/*sigemptyset(&sa.sa_mask);*/
	/*sa.sa_flags = SA_RESTART;*/
	/*if (sigaction(SIGCHLD, &sa, NULL) == -1) {*/
		/*perror("sigaction");*/
		/*exit(1);*/
	/*}*/

	/*printf("server: waiting for connections...\n");*/

	/*while (1) {*/
		/*sin_size = sizeof(their_addr);*/
		/*int new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);*/
		/*if (new_fd == -1) {*/
			/*perror("accept");*/
			/*continue;*/
		/*}*/

		/*inet_ntop(their_addr.ss_family, getInAddr((struct sockaddr *) &their_addr), s, sizeof(s));*/
		/*printf("server: got connection from %s\n", s);*/

		/*if (!fork()) {*/
			/*// child*/
			/*close(sockfd);*/
			/*if (send(new_fd, "Hello, world!\n", 14, 0) == -1) {*/
				/*perror("send");*/
			/*}*/
			/*close(new_fd);*/
			/*exit(0);*/
		/*}*/
		/*close(new_fd);*/
	/*}*/
/*}*/
