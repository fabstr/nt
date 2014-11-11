#include "torrent.h"

void freeTorrent(torrent *t)
{
	pthread_mutex_destroy(t -> trackersMutex);
	pthread_mutex_destroy(t -> peersMutex);

	if (t -> trackers != NULL) free(t -> trackers);
	if (t -> peers != NULL) free(t -> peers);
	if (t -> peerid != NULL) free(t -> peerid);
	if (t -> infohash != NULL) free(t -> infohash);
	freeValue(t -> metainfo);
	free(t);
}

void printInfoHash(unsigned char *infohash)
{
	for (size_t i=0; i<20; ++i) {
		unsigned char c = infohash[i];
		if (c <= 0x10) {
			printf("0");
		}
		printf("%x", c);
	}
}

torrent* newTorrent(char *filename)
{
	// get the metainfo
	torrent *t = (torrent *) malloc(sizeof(torrent));
	t -> metainfo = decodeFile(filename);

	// get the info hash
	t -> infohash = calculateInfoHash(filename);

	t -> peerid = NULL;
	t -> trackers = NULL;
	t -> peers = NULL;

	t -> trackersMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	t -> peersMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

	pthread_mutex_init(t -> trackersMutex, NULL);
	pthread_mutex_init(t -> peersMutex, NULL);

	return t;
}

void* handleTorrent(void *data)
{
	torrentdata *tdata = (torrentdata *) data;
	printf("Opening %s on tcp://%s:%s\n", tdata -> filename, tdata -> host, tdata -> port);

	torrent *t = newTorrent(tdata -> filename);
	t -> tdata = tdata;
	t -> sockfd = openListenSocket(tdata -> host, tdata -> port);

	pthread_t inbound_thread;
	pthread_t outbound_thread;

	if (t -> sockfd == -1) {
		fprintf(stderr, "Could not open listening socket.\n");
		return NULL;
	}

	pthread_create(&inbound_thread, NULL, inboundLoop, t);
	pthread_create(&outbound_thread, NULL, outboundLoop, t);

	pthread_join(inbound_thread, NULL);
	pthread_join(outbound_thread, NULL);

	// close the listening socket
	close(t -> sockfd);

	freeTorrent(t);
	return NULL;
}

// wait for peer to connect and add them to the peer queue
void* inboundLoop(void *data) 
{
	torrent *t = (torrent *) data;
	int newfd;
	struct sockaddr addr;
	socklen_t addrsize = sizeof(struct sockaddr);
	int looping = 1;
	printf("Waiting for connections...\n");
	while (looping) {
		newfd = accept(t->sockfd, &addr, &addrsize);
		if (newfd != -1) {
			handleIncomingConnection(newfd, addr, t);
		}
	}

	return NULL;
}

// connect to peers and add them to the peer queue
void* outboundLoop(void *data)
{
	// torrent *t = (torrent *) data;
	printf("This is outbound loop talking!\n");
	return NULL;
}

// wait handshake and send ours
void handleIncomingConnection(int sock, struct sockaddr addr, torrent *t)
{
	printf("Handling connection from %d\n", sock);
	close(sock);

	// create the peer object
	peer *p = (peer *) malloc(sizeof(peer));

	// set the socket
	p -> sockfd = sock;

	// wait for the handshake 
	// check the info hash
	// send our own handshake
	// lock the peer mutex and add this peer to the array
	pthread_mutex_lock(t -> peersMutex);
	pthread_mutex_unlock(t -> peersMutex);
}
