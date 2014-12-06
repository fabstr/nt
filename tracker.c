#include "tracker.h"

size_t recdata(void *buff, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize = size*nmemb;

	tracker *t = (tracker *) userdata;

	// reallocate memory
	t -> buff = realloc(t -> buff, t -> buffsize + realsize + 1);
	if (t -> buff == NULL) {
		// out of memory
		return 0;
	}

	// copy from buff to t->buff and set null byte
	memcpy(&(t->buff[t->buffsize]), buff, realsize);
	t -> buffsize += realsize;
	t -> buff[t->buffsize] = '\0';

	return realsize;
}

tracker* openTracker(char *url)
{
	tracker *t = (tracker*) malloc(sizeof(tracker));
	if (t == NULL) {
		return NULL;
	}

	// copy the url
	size_t urllen = strlen(url);
	t -> url = (char*) malloc(urllen+1);
	strcpy(t -> url, url);

	// prepare the buffer
	t -> buff = malloc(1);
	t -> buffsize = 0;

	// start curl
	t -> curlhandle = curl_easy_init();
	curl_easy_setopt(t -> curlhandle, CURLOPT_WRITEFUNCTION, recdata);
	curl_easy_setopt(t -> curlhandle, CURLOPT_WRITEDATA, t);
	curl_easy_setopt(t -> curlhandle, CURLOPT_URL, t -> url);

	return t;
}

void closeTracker(tracker *t)
{
	curl_easy_cleanup(t -> curlhandle);
	free(t -> url);
	free(t -> buff);
}

void freeTracker(tracker *t)
{
	closeTracker(t);
	free(t);
}

char *trackerRequestToGetString(trackerRequest *req)
{
	char *str;
	size_t size;
	FILE *f = open_memstream(&str, &size);

	if (req -> info_hash != NULL) {
		fprintf(f, "info_hash=");
		for (int i=0; i<20; ++i) {
			unsigned char c = req -> info_hash[i];
			fprintf(f, "%%%s%X", (c <= 0xF ? "0" : ""), c);
		}
	}

	if (req -> peer_id != NULL)  {
		fprintf(f, "&peer_id=");
		for (int i=0; i<20; ++i) {
			unsigned char c = req -> peer_id[i];
			fprintf(f, "%%%s%X", (c <= 0xF ? "0" : ""), c);
		}
	}

	if (req -> port != 0) {
		fprintf(f, "&port=%d", req -> port);
	}

	if (req -> uploaded != -1) {
		fprintf(f, "&uploaded=%ld", req -> uploaded);
	}

	if (req -> downloaded != -1) {
		fprintf(f, "&downloaded=%ld", req -> downloaded);
	}

	if (req -> left != -1) {
		fprintf(f, "&left=%ld", req -> left);
	}

	if (req -> compact != -1) {
		if (req -> compact == 0) {
			fprintf(f, "&compact=0");
		} else {
			fprintf(f, "&compact=1");
		}
	}

	if (req -> no_peer_id != -1 && req -> compact > 0) {
		fprintf(f, "&no_peer_id");
	}

	if (req -> event != UNSPECIFIED) {
		if (req -> event == STARTED) {
			fprintf(f, "&event=started");
		} else if (req -> event == STOPPED) {
			fprintf(f, "&event=stopped");
		} else if (req -> event == COMPLETED) {
			fprintf(f, "&event=completed");
		}
	}

	if (req -> ip != NULL) {
		fprintf(f, "&ip=%s", req -> ip);
	}

	if (req -> numwant != -1) {
		fprintf(f, "&numwant=%d", req -> numwant);
	}

	if (req -> key != NULL) {
		fprintf(f, "&key=%s", req -> key);
	}

	if (req -> tracker_id != NULL) {
		fprintf(f, "&trackerid=%s", req -> tracker_id);
	}

	fclose(f);
	str[size] = '\0';
	return str;
}


trackerResponse* sendRequest(tracker *tracker, trackerRequest *request)
{
	char *urlToUse;
	size_t size;
	FILE *f = open_memstream(&urlToUse, &size);
	char *get = trackerRequestToGetString(request);
	fprintf(f, "%s?%s", tracker -> url, get);
	fclose(f);
	urlToUse[size] = '\0';

	curl_easy_setopt(tracker -> curlhandle, CURLOPT_URL, urlToUse);

	free(get);
	free(urlToUse);

	int result = curl_easy_perform(tracker -> curlhandle);
	if (result != CURLE_OK) {
		return NULL;
	}

	// decode the bencode string in tracker -> buff
	value *response = (value *) malloc(sizeof(value));
	int consumed = decode(tracker -> buff, tracker -> buffsize, response);
	if (consumed == 0) {
		freeValue(response);
		return NULL;
	}

	trackerResponse *tresp = parseResponse(response);
	freeValue(response);
	return tresp;
}

trackerResponse* parseResponse(value *response) 
{
	trackerResponse *tresp = newResponse();
	if (tresp == NULL) {
		return NULL;
	}

	value *v;
	if ((v = dictionaryGetByKey(response, "failure reason")) != NULL) {
		size_t msglen = strlen(v -> v.s);
		tresp -> failure_reason = (unsigned char *) malloc(msglen+1);
		strcpy(v -> v.s, (char *) tresp -> failure_reason);
		return tresp;
	}

	if ((v = dictionaryGetByKey(response, "warning message")) != NULL) {
		size_t msglen = strlen(v -> v.s);
		tresp -> warning_message = (unsigned char *) malloc(msglen+1);
		strcpy(v -> v.s, (char *) tresp -> warning_message);
	}

	if ((v = dictionaryGetByKey(response, "interval")) != NULL) {
		tresp -> interval = v -> v.i;
	}

	if ((v = dictionaryGetByKey(response, "min interval")) != NULL) {
		tresp -> min_interval = v -> v.i;
	}

	if ((v = dictionaryGetByKey(response, "tracker_id")) != NULL) {
		size_t msglen = strlen(v -> v.s);
		tresp -> tracker_id = (unsigned char *) malloc(msglen+1);
		strcpy(v -> v.s, (char *) tresp -> warning_message);
	}

	if ((v = dictionaryGetByKey(response, "complete")) != NULL) {
		tresp -> complete = v -> v.i;
	}

	if ((v = dictionaryGetByKey(response, "incomplete")) != NULL) {
		tresp -> incomplete = v -> v.i;
	}

	if ((v = dictionaryGetByKey(response, "peers")) != NULL) {
		if (v -> type == STRING) {
			tresp -> peers_list = NULL;

			char *str = v -> v.s;
			size_t len = v -> strlen;
			size_t npeers = len % 6;

			tresp -> peers = (peer *) malloc(sizeof(peer));
			tresp -> n_peers = npeers;

			for (int i=0; i<npeers; ++i) {
				peer *p = &(tresp -> peers[i]);
				uint32_t ip;
				unsigned short int port;

				memcpy(&ip, (str+i*6), 4);
				memcpy(&port, (str+i*10), 2);

				ip = ntohl(ip);
				port = ntohs(port);

				size_t ipsize, portsize;

				FILE *ipStr = open_memstream(&(p -> host), &ipsize);
				unsigned char bytes[4];
				bytes[0] = ip & 0xFF;
				bytes[1] = (ip >> 8) & 0xFF;
				bytes[2] = (ip >> 16) & 0xFF;
				bytes[3] = (ip >> 24) & 0xFF;	
				fprintf(ipStr, "%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
				fclose(ipStr);

				FILE *portStr = open_memstream(&(p -> host), &portsize);
				fprintf(portStr, "%d", port);
				fclose(portStr);
			}
		} else if (v -> type == LIST) {
			// steal the dictionary to avoid it beeing free()'d
			tresp -> peers_list = v -> v.l;
			v -> v.d = NULL;
		}
	}

	return tresp;
}

trackerRequest* newTrackerRequest()
{
	trackerRequest *t = (trackerRequest *) malloc(sizeof(trackerRequest));
	if (t == NULL) {
		return NULL;
	}

	t -> info_hash = NULL;
	t -> peer_id = NULL;
	t -> port  = 0;
	t -> uploaded = -1;
	t -> downloaded = -1;
	t -> left = -1;
	t -> compact = -1;
	t -> no_peer_id = -1;
	t -> event = UNSPECIFIED;
	t -> ip  = NULL;
	t -> numwant = -1;
	t -> key  = NULL;
	t -> tracker_id  = NULL;
	return t;
}

trackerResponse* newResponse()
{
	trackerResponse *t = (trackerResponse *) malloc(sizeof(trackerResponse));
	if (t == NULL) {
		return NULL;
	}

	t -> failure_reason = NULL;
	t -> warning_message = NULL;
	t -> interval = -1;
	t -> min_interval = -1;
	t -> tracker_id = NULL;
	t -> complete = -1;
	t -> incomplete = -1;
	t -> peers_list = NULL;
	t -> n_peers = 0; 
	t -> peers = NULL;

	return t;
}

void freeRequest(trackerRequest *t)
{
	if (t -> info_hash != NULL) free(t -> info_hash);
	if (t -> peer_id != NULL) free(t -> peer_id);
	if (t -> ip != NULL) free(t -> ip);
	if (t -> key != NULL) free(t -> key);
	if (t -> tracker_id != NULL) free(t -> tracker_id);
	free(t);
}

void freeResponse(trackerResponse *t)
{
	if (t -> failure_reason != NULL) free(t -> failure_reason);
	if (t -> warning_message != NULL) free(t -> warning_message);
	if (t -> tracker_id != NULL) free(t -> tracker_id);
	if (t -> peers_list != NULL) {
		value *v = (value *) malloc(sizeof(value));
		v -> type = LIST;
		v -> v.l = t -> peers_list;
		freeValue(v);
	}
	if (t -> peers != NULL) free(t -> peers);
	free(t);
}

#ifdef TRACKERTEST
int main()
{
	curl_global_init(CURL_GLOBAL_ALL);

	tracker *t = openTracker("http://www.lan.tallr.se/tracker/index.php");
	trackerRequest *tr = newTrackerRequest();
	unsigned char hash[21] = {0x7d, 0x6f, 0x70, 0x9d, 0x5f, 0x17, 0x3a, 0x20, 0x2c, 0x56, 0xff, 0x00, 0xc5, 0xaa, 0xbc, 0x8d, 0x80, 0xce, 0xc3, 0x0f, 0x0};
	tr -> info_hash = hash;
	tr -> peer_id = (unsigned char *) "123456789abcdefqwer";
	tr -> port = 1234;
	tr -> uploaded = 0;
	tr -> downloaded = 0;
	tr -> left = 12345;
	tr -> compact = 1;
	tr -> no_peer_id = 1;
	tr -> event = STARTED;
	tr -> ip = "192.168.1.20";
	tr -> numwant = 20;
	tr -> key = "asdf";
	tr -> tracker_id = "dfgblk";
	trackerResponse *resp = sendRequest(t, tr);
	if (resp == NULL) {
		printf("resp is null\n");
	} else {
		if (resp -> n_peers == 0 && resp -> peers_list != NULL) {
			printf("Peers: ");
			value v = {.type=LIST, .v.l = resp -> peers_list};
			printValue(&v);
			printf("\n");
		}

		freeResponse(resp);
		free(tr);
		freeTracker(t);
	}




	curl_global_cleanup();
}
#endif // TRACKERTEST
