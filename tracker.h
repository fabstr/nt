#ifndef TRACKER_H
#define TRACKER_H

#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>
#include "bencode.h"
#include "peer.h"

typedef struct tracker tracker;
struct tracker {
	char *url;
	CURL *curlhandle;
	char *buff;
	size_t buffsize;
};

enum trackerEvent {
	UNSPECIFIED, STARTED, STOPPED, COMPLETED
};

typedef struct trackerRequest trackerRequest;
struct trackerRequest {
	unsigned char* info_hash; // null if not set
	unsigned char* peer_id;  // null of not set
	unsigned short int port;  // 0 if not set
	long int uploaded; // -1 if not set
	long int downloaded; // -1 if not set
	long int left; // -1 if not set
	int compact; // -1 if not set, else 0 or 1
	int no_peer_id; // -1 if not set, else 0 or 1
	enum trackerEvent event; // UNSPECIFIED if not set
	char *ip; // null if not set
	int numwant; // -1 if not set
	char* key; // null if not set
	char* tracker_id;// null if not set
};

typedef struct trackerResponse trackerResponse;
struct trackerResponse {
	unsigned char* failure_reason; // null if not set
	unsigned char* warning_message; // null if not set
	int interval; // -1 if not set
	int min_interval; // -1 if not set
	unsigned char* tracker_id; // null if not set
	long complete; // -1 if not set
	long incomplete; // -1 if not set
	list* peers_list; // null if not set
	size_t n_peers; // if not 0, peers_dictionary should be null and n_peers
	                // is the number of peers in the peers array
	peer *peers;
};

size_t recdata(void *buff, size_t size, size_t nmemb, void *userdata);
tracker* openTracker(char *url);
void closeTracker(tracker *t);
char *trackerRequestToGetString(trackerRequest *req);
trackerResponse* sendRequest(tracker *tracker, trackerRequest *request);
trackerResponse* parseResponse(value *response);
trackerRequest *newTrackerRequest();
trackerResponse* newResponse();
void freeRequest(trackerRequest *t);
void freeResponse(trackerResponse *t);

#endif // TRACKER_H
