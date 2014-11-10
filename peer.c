#include "peer.h"

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

handshake* recieveHandshake(peer *peer)
{
	uint32_t strlen;
	char str[19];

	handshake *hs = (handshake *) malloc(sizeof(handshake));
	hs -> infohash = malloc(21);
	hs -> peerid = malloc(21);
	hs -> zeros = malloc(8);

	int success = 1;
	if (4 != recv(peer -> sockfd, &strlen, 4, 0)) {
		success = 0;
	} else if (19 != recv(peer -> sockfd, &str, 19, 0)) {
		success = 0;
	} else if (8 != recv(peer -> sockfd, hs -> zeros, 8, 0)) {
		success = 0;
	} else if (20 != recv(peer -> sockfd, hs -> infohash, 20, 0)) {
		success = 0;
	} else if (20 != recv(peer -> sockfd, hs -> peerid, 20, 0)) {
		success = 0;
	}

	if (success == 0) {
		free(hs->infohash);
		free(hs->peerid);
		free(hs->zeros);
		free(hs);
		return NULL;
	}
	
	return hs;
}
