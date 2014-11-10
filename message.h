#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>

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



#endif // MESSAGE_H
