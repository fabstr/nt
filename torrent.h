#ifndef TORRENT_H
#define TORRENT_H

#include <stdio.h>
#include "bencode.h"
#include "net.h"
#include "tracker.h"

typedef struct torrent torrent;
struct torrent {
	// the trackers we are talking with
	size_t ntrackers;
	tracker *trackers;

	// the peers given by the tracker
	size_t npeers;
	peer *peers;

	// our peerid
	char peerid[20];

	// the infohash of the torrent
	char infohash[20];

	// the torrent file
	value *metainfo;
};

#endif // TORRENT_H
