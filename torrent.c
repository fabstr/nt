#include "torrent.h"

void freeTorrent(torrent *t)
{
	pthread_mutex_destroy(t -> trackersMutex);
	pthread_mutex_destroy(t -> peersMutex);

	if (t -> trackers != NULL) {
		for (size_t i=0; i<t -> ntrackers; ++i) {
			tracker *tr = &(t -> trackers[i]);
			closeTracker(tr);
		}

		free(t -> trackers);
	}

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

size_t countAnnounce(value *metainfo)
{
	value *announce = dictionaryGetByKey(metainfo, "announce");
	char *announceStr = announce -> v.s;

	value *announceList = dictionaryGetByKey(metainfo, "announce-list");

	size_t count = 1;
	if (announceList != NULL) {
		list *l = announceList -> v.l;
		while (l != NULL) {
			char *str = l -> v -> v.s;

			if (strcmp(announceStr, str) != 0) {
				count++;
			}

			l = l -> next;
		}
	}

	freeValue(announce);
	freeValue(announceList);

	return count;
}

void setTrackers(tracker *trackers, size_t count, value *metainfo)
{
	if (count < 1) return;

	size_t pos = 0;

	value *announce = dictionaryGetByKey(metainfo, "announce");
	char *announceStr = announce -> v.s;

	tracker *t = openTracker(announceStr);
	trackers[pos].url = t -> url;
	trackers[pos].curlhandle = t -> curlhandle;
	trackers[pos].buff = t -> buff;
	trackers[pos].buffsize = t -> buffsize;
	pos++;

	value *announceList = dictionaryGetByKey(metainfo, "announce-list");

	if (announceList != NULL) {
		list *l = announceList -> v.l;
		while (l != NULL && pos < count) {
			char *str = l -> v -> v.s;

			if (strcmp(announceStr, str) != 0) {
				tracker *t = openTracker(announceStr);
				trackers[pos].url = t -> url;
				trackers[pos].curlhandle = t -> curlhandle;
				trackers[pos].buff = t -> buff;
				trackers[pos].buffsize = t -> buffsize;
				pos++;
			}

			l = l -> next;
		}
	}

	freeValue(announce);
	freeValue(announceList);
}

torrent* newTorrent(char *filename)
{
	// get the metainfo
	torrent *t = (torrent *) malloc(sizeof(torrent));
	t -> metainfo = decodeFile(filename);

	// get the info hash
	t -> infohash = calculateInfoHash(filename);

	t -> ntrackers = countAnnounce(t -> metainfo);
	t -> trackers = (tracker *) malloc(t -> ntrackers * sizeof(tracker));
	setTrackers(t -> trackers, t -> ntrackers, t -> metainfo);

	t -> peerid = NULL;
	t -> peers = NULL;

	t -> npeers = 0;
	t -> peersSize = 30;
	t -> peers = (peer *) malloc(t->peersSize * sizeof(peer));

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

	printf("in:  ");
	printInfoHash(t -> infohash);
	printf("\n");

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
	torrent *t = (torrent *) data;

	printf("out: ");
	printInfoHash(t -> infohash);
	printf("\n");

	printf("Trackers (%ld):\n", t -> ntrackers);
	for (int i=0; i<t->ntrackers; ++i) {
		printf("%s\n", t -> trackers[i].url);
	}

	int looping = 1;
	while (looping) {
		for (size_t i=0; i<t -> ntrackers; ++i) {
			tracker *tr = &(t -> trackers[i]);
			peer *p = askTrackerForPeers(tr, t);
			mergePeers(t, p);
		}
		looping = 0;
	}

	return NULL;
}

// wait handshake and send ours
void handleIncomingConnection(int sock, struct sockaddr addr, torrent *t)
{
	printf("Handling connection from %d\n", sock);

	// create the peer object
	peer *p = (peer *) malloc(sizeof(peer));

	// set the socket
	p -> sockfd = sock;

	// wait for the handshake 
	handshake *hs = recieveHandshake(p);

	// check the info hash
	if (strncmp((char *) hs -> infohash, (char *) t -> infohash, 20) != 0) {
		// the info hash does not match, close the connection 
		close(sock);
	} else {
		// send our own handshake
		int result = sendHandshake(p, (char *) t -> infohash, (char *) t -> peerid);
		if (result > 0) {
			addPeer(t, p);
		}
	}

	// TODO what to do with hs -> peerid?
	free(p);
	free(hs->infohash);
	free(hs->peerid);
	free(hs->zeros);
	free(hs);
}

void addPeer(torrent *t, peer *p)
{
	// lock the peer mutex and add this peer to the array
	pthread_mutex_lock(t -> peersMutex);

	if (t -> npeers == t -> peersSize) {
		// need to increase the peers array
		t -> peers = realloc(t->peers, t->peersSize*2);
		if (t -> peers == NULL) {
			// we have a problem
			// TODO
			pthread_mutex_unlock(t -> peersMutex);
			return;
		}
	}

	// get the current peer to write to
	peer *pp = &(t -> peers[t -> npeers]);

	// increase the number of peers we have
	t -> npeers += 1;

	// set host/port to null since we are already connected
	pp -> host = NULL;
	pp -> port = NULL;

	// copy the socket
	pp -> sockfd = p -> sockfd;

	// allocate memory for 0 messages
	pp -> nmessages = 0;
	pp -> messageQueue = (message *) malloc(pp->nmessages * sizeof(message));

	// set the connection to choking and not interested
	pp -> am_choking = 1;
	pp -> am_interested = 0;
	pp -> peer_choking = 1;
	pp -> peer_interested = 0;

	// we're done, unlock the mutex
	pthread_mutex_unlock(t -> peersMutex);
}
