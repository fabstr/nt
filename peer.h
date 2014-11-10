#ifndef PEER_H
#define PEER_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "net.h"
#include "message.h"

typedef struct handshake handshake;
struct handshake {
	unsigned char *infohash;
	unsigned char *peerid;
	unsigned char *zeros;
};

typedef struct peer peer;
struct peer {
	char *host;
	char *port;
	int sockfd;

	message *messageQueue;
	size_t nmessages;

	int am_choking;
	int am_interested;
	int peer_choking;
	int peer_interested;
};

peer* connectToPeer(char *host, char *port);
void closePeer(peer *p); // close the socket 
void freePeer(peer *p); // free the messages and p

int sendHandshake(peer *peer, char *infohash, char *peerid);
handshake* recieveHandshake(peer *peer);

int sendMessageToPeer(peer *p, message *m);
message *recieveMessageFromPeer(peer *p);

void parseBitfield(message *m, unsigned char **bytes, size_t *nbytes);
void parseRequest(message *m, uint32_t *pieceIndex, uint32_t *offset, 
		uint32_t *length);
void parsePiece(message *m, uint32_t *pieceIndex, uint32_t *offset, 
		unsigned char **block, size_t *length);
void parseCancel(message *m, uint32_t *pieceIndex, uint32_t *offset, 
		uint32_t *length);
void parsePort(message *m, uint16_t *port);

#endif // PEER_H
