#include <stdlib.h>
#include <stdio.h>

#include "sha1.h"

int main()
{
	char*  output = (char *) malloc(20);
	char* input = "";
	sha1((const unsigned char *) input, 0, (unsigned char *) output);
	for (int i=0; i<20; i++) {
		unsigned char c = output[i];
		printf("%02x ", c);
	}
	printf("\n");
}
