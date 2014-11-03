#ifndef NET_H
#define NET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define BACKLOG 10

void sigchld_handler(int s);
void* getInAddr(struct sockaddr *sa);
void setAddrinfoHints(struct addrinfo *hints);
int bindTo(char *host, char *port);
int openListenSocket(char *host, char *port);
int connectTo(char *host, char *port);


#endif // NET_H
