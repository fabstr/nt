#include <stdio.h>
#include <stdlib.h>

#include "bencode.h"

void doFile(char *file)
{
	printf("%s (", file);
	unsigned char *infohash = calculateInfoHash(file);
	for (int i=0; i<20; ++i) {
		unsigned char c = infohash[i];
		printf("%s%x", (c <= 0xF ? "0" : ""), c);
	}
	printf("): ");


	FILE *f = fopen(file, "rb");

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buff = (char *) malloc(size+1);
	fread(buff, size, 1, f);
	buff[size] = '\0';

	value *v = (value *) malloc(sizeof(value));
	int consumed = decode(buff, size, v);
	if (consumed == 0) {
		printf("0 consumed\n");
	}
	printValue(v);
	freeValue(v);
	printf("\n");
	free(buff);
}

int main(int argc, char **argv)
{
	for (int i=1; i<argc; i++) {
		doFile(argv[i]);
	}
}
