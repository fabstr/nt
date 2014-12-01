#ifndef TORRENT_H
#define TORRENT_H

#include <pthread.h>
#include <stdio.h>
#include "bencode.h"
#include "net.h"
#include "tracker.h"

typedef struct torrentdata torrentdata;
struct torrentdata {
	char *filename;
	char *host;
	char port[6];
};

typedef struct torrent torrent;
struct torrent {
	// the trackers we are talking with
	size_t ntrackers;
	tracker *trackers;
	pthread_mutex_t *trackersMutex;

	// the peers given by the tracker; peersMutex must be locked before
	// doint anything with this array
	size_t npeers;
	size_t peersSize;
	peer *peers;
	pthread_mutex_t *peersMutex;

	// our peerid
	unsigned char *peerid;

	// the infohash of the torrent
	unsigned char *infohash;

	// the torrent file
	value *metainfo;

	// the socket we are listening on
	int sockfd;

	// the torrent data (filename/host/port)
	torrentdata *tdata;
};

void printInfoHash(unsigned char *infohash);
void freeTorrent(torrent *t);
torrent* newTorrent(char *filename);
void* handleTorrent(void *filename);
void *outboundLoop(void *data);
void *inboundLoop(void *data);
void handleIncomingConnection(int sock, struct sockaddr addr, torrent *t);
void addPeer(torrent *t, peer *p);


#endif // TORRENT_H
