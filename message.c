#include "message.h"

message*newKeepAlive()
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 0; // keep alive
	m -> id = KEEPALIVE;
	m -> payload = NULL;
	m -> payloadlen = 0;
	return m;
}

message *newChoke()
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 1; 
	m -> id = CHOKE;
	m -> payload = NULL;
	m -> payloadlen = 0;
	return m;
}

message *newUnchoke()
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 1;
	m -> id = UNCHOKE;
	m -> payload = NULL;
	m -> payloadlen = 0;
	return m;
}

message *newInterested()
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 1;
	m -> id = INTERESTED;
	m -> payload = NULL;
	m -> payloadlen = 0;
	return m;
}

message *newNotInterested()
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 1;
	m -> id = NOTINTERESTED;
	m -> payload = NULL;
	m -> payloadlen = 0;
	return m;
}

message *newHave(uint32_t pieceIndex)
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 1;
	m -> id = HAVE;
	m -> payload = malloc(4);
	memcpy(m -> payload, &pieceIndex, 4);
	m -> payloadlen = 4;
	return m;
}

message *newBitfield(unsigned char *bytes, size_t nbytes)
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 1+nbytes;
	m -> id = BITFIELD;
	m -> payload = malloc(nbytes);
	memcpy(m -> payload, bytes, nbytes);
	m -> payloadlen = nbytes;
	return m;
}

message *newRequest(uint32_t pieceIndex, uint32_t offset, uint32_t length)
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 13;
	m -> id = REQUEST;
	m -> payload = malloc(12);
	pieceIndex = htonl(pieceIndex);
	offset = htonl(offset);
	length = htonl(length);
	memcpy(m -> payload, &pieceIndex, 4);
	memcpy(m -> payload+4, &offset, 4);
	memcpy(m -> payload+8, &length, 4);
	m -> payloadlen = 12;
	return m;
}

message *newPiece(uint32_t pieceIndex, uint32_t offset, void *block, 
		size_t blockLength)
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 9+blockLength;
	m -> id = PIECE;
	m -> payload = malloc(blockLength);
	memcpy(m -> payload, block, blockLength);
	m -> payloadlen = blockLength;
	return m;
}

message *newCancel(uint32_t pieceIndex, uint32_t offset, uint32_t length)
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 13;
	m -> id = CANCEL;
	m -> payload = malloc(12);
	pieceIndex = htonl(pieceIndex);
	offset = htonl(offset);
	length = htonl(length);
	memcpy(m -> payload, &pieceIndex, 4);
	memcpy(m -> payload+4, &offset, 4);
	memcpy(m -> payload+8, &length, 4);
	m -> payloadlen = 12;
	return m;
}

message *newPort(uint16_t port)
{
	message *m = (message *) malloc(sizeof(message));
	m -> priority = 0;
	m -> length_prefix = 3;
	m -> id = PORT;
	m -> payload = malloc(2);
	port = htons(port);
	memcpy(m -> payload, &port, 2);
	m -> payloadlen = 2;
	return m;
}

void freeMessage(message *m)
{
	if (m -> payload != NULL) {
		free(m -> payload);
	}
	free(m);
}


