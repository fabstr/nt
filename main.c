#include <pthread.h>
#include <stdio.h>
#include "torrent.h"

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("nt metainfo-file ...\n");
		return 1;
	}

	size_t nfiles = argc-1;
	pthread_t threads[nfiles];

	torrentdata data[nfiles];

	// start a thread for each torrent file
	short unsigned int port = 1200;
	for (size_t i=0; i<nfiles; ++i) {
		data[i].filename = argv[i+1];
		data[i].host = "0.0.0.0";
		sprintf(data[i].port, "%d", port++);

		int val = pthread_create(&(threads[i]), NULL, handleTorrent, &(data[i]));
		if (val != 0) {
			fprintf(stderr, "Could not create thread %ld.\n", i);
			return 1;
		}
	}

	// wait for all the threads to finish
	for (size_t i=0; i<nfiles; ++i) {
		pthread_join(threads[i], NULL);
	}
}
