#ifndef PEER_H
#define PEER_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "net.h"

typedef struct message message;
struct message {
	int priority;

	int length_prefix; // must be set
	enum {
		KEEPALIVE=-1, CHOKE=0, UNCHOKE=1, INTERESTED=2, NOTINTERESTED=3,
		HAVE=4, BITFIELD=5, REQUEST=6, PIECE=7, CANCEL=8, PORT=9
	} id;
	unsigned char messageid; // -1 if not set
	void *payload; // NULL if not set
	size_t payloadlen; // 0 if payload is not set
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

message *newKeepAlive();
message *newChoke();
message *newUnchoke();
message *newInterested();
message *newNotInterested();
message *newHave(uint32_t pieceIndex);
message *newBitfield(unsigned char *bytes, size_t nbytes);
message *newRequest(uint32_t pieceIndex, uint32_t offset, uint32_t length);
message *newPiece(uint32_t pieceIndex, uint32_t offset, void *block, 
		size_t blockLength);
message *newCancel(uint32_t pieceIndex, uint32_t offset, uint32_t length);
message *newPort(uint16_t port);
void freeMessage(message *m);

peer* connectToPeer(char *host, char *port);
void closePeer(peer *p); // close the socket 
void freePeer(peer *p); // free the messages and p

int sendHandshake(peer *peer, char *infohash, char *peerid);
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
