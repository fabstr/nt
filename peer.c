#include "peer.h"

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

peer* connectToPeer(char *host, char *port)
{
	peer *p = (peer *) malloc(sizeof(peer));
	p -> host = host;
	p -> port = port;
	p -> sockfd = connectTo(host, port);

	p -> messageQueue = NULL;
	p -> nmessages = 0;

	p -> am_choking = 1;
	p -> am_interested = 0;
	p -> peer_choking = 1;
	p -> peer_interested = 0;

	return p;
}

void closePeer(peer *p)
{
	close(p -> sockfd);
}

void freePeer(peer *p)
{
	free(p -> messageQueue);
	free(p);
}

int sendHandshake(peer *peer, char *infohash, char *peerid)
{
	char *message;
	size_t size;
	FILE *f = open_memstream(&message, &size);

	unsigned char prococol_name_length = 0x13; // 19
	char *protocol_name = "BitTorrent protocol";

	// write 0x13
	fwrite(&prococol_name_length, 1, 1, f);

	// write "BitTorrent protocol" (no null char)
	fwrite(protocol_name, 19, 1, f);

	// write 8 bytes of zero
	char zero[8] = {0,0,0,0,0,0,0,0};
	fwrite(zero, 8, 1, f);

	// write infohash and peerid
	fwrite(infohash, 20, 1, f);
	fwrite(peerid, 20, 1, f);

	fclose(f);

	// send the message
	int toreturn = send(peer -> sockfd, message, size, 0);

	free(message);
	return toreturn;
}

int sendMessageToPeer(peer *p, message *m)
{
	uint32_t lengthprefix;

	if (m -> id == KEEPALIVE) {
		lengthprefix = htonl(0);
		return send(p -> sockfd, &lengthprefix, 4, 0);
	}

	char *message;
	size_t size;
	FILE *f = open_memstream(&message, &size);

	lengthprefix = htonl(m -> length_prefix);
	fwrite(&lengthprefix, 4, 1, f);

	unsigned char id = m -> id;
	fwrite(&id, 1, 1, f);

	if (m -> payloadlen != 0 && m -> payload != NULL) {
		fwrite(m -> payload, m -> payloadlen, 1, f);
	}

	fclose(f);

	int toreturn = send(p -> sockfd, message, size, 0);

	free(message);
	return toreturn;
}

message *recieveMessageFromPeer(peer *p)
{
	message *m = (message *) malloc(sizeof(message));
	if (m == NULL) {
		return NULL;
	}

	// get the length of the message
	uint32_t len;
	int read = recv(p -> sockfd, &len, 4, 0);
	if (read != 4) {
		free(m);
		return NULL;
	}
	len = ntohl(len);


	// was it a keep-alive?
	if (len == 0) {
		m -> payload = NULL;
		m -> payloadlen = 0;
		return m;
	}

	// allocate memory for data
	m -> payload = malloc(len);
	m -> payloadlen = len;

	// read data
	read = recv(p -> sockfd, m -> payload, len, 0);
	if (read == -1) {
		free(m -> payload);
		free(m);
		return NULL;
	}

	// get the id (the first byte)
	char *data = m -> payload;
	char id = data[0];
	switch (id) {
		case 0: m -> id = CHOKE; 
		case 1: m -> id = UNCHOKE;
		case 2: m -> id = INTERESTED;
		case 3: m -> id = NOTINTERESTED;
		case 4: m -> id = HAVE;
		case 5: m -> id = BITFIELD;
		case 6: m -> id = REQUEST;
		case 7: m -> id = PIECE;
		case 8: m -> id = CANCEL;
		case 9: m -> id = PORT;
	}

	return m;
}
