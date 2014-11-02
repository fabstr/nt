#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct data {
	char *mem;
	size_t size;
};

size_t recdata(void *buff, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize = size*nmemb;
	struct data *d = (struct data *) userdata;
	d -> mem = realloc(d->mem, d->size+realsize+1);
	if (d -> mem == NULL) {
		printf("out of memory\n");
		return 0;
	}

	memcpy(&(d->mem[d->size]), buff, realsize);
	d -> size += realsize;
	d -> mem[d->size] = '\0';
	return realsize;
}


int main()
{
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *curl = curl_easy_init();

	struct data d;
	d.mem = (char *) malloc(1);
	d.size = 0;

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recdata);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &d);
	curl_easy_setopt(curl, CURLOPT_URL, "https://tallr.se/ip.php");
	curl_easy_perform(curl);


	printf("Got string: '%s'\n", d.mem);

	free(d.mem);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}
