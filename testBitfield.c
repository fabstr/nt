#include "testBitfield.h"

void printBitfield(bitfield *bitfield)
{
	for (size_t i=0; i<bitfield -> nbytes; ++i) {
		unsigned char byte = bitfield -> bytes[i];
		for (int i=7; i>=0; --i) {
			if (byte & (1<<i)) {
				printf("1");
			} else {
				printf("0");
			}
		}
	}
}

void testSingleByte()
{
	bitfield *b = newBitfield(8);

	setBit(b, 0, 1);
	setBit(b, 1, 1);
	setBit(b, 2, 1);
	setBit(b, 3, 0);
	setBit(b, 4, 1);
	setBit(b, 5, 1);
	setBit(b, 6, 1);
	setBit(b, 7, 1);
	printf("11101111\n");
	printBitfield(b); 
	printf("\n");
	printf("\n");

	unsigned int *pieces;
	size_t npieces;
	getSet(b, &pieces, &npieces);

	printf("0 1 2 4 5 6 7\n");

	for (size_t i=0; i<npieces; ++i) {
		printf("%d ", pieces[i]);
	}
	printf("\n");
	printf("\n");
	
	if (!(isSet(b, 0) == 1)) printf("bit 0\n");
	if (!(isSet(b, 1) == 1)) printf("bit 1\n");
	if (!(isSet(b, 2) == 1)) printf("bit 2\n");
	if (!(isSet(b, 3) == 0)) printf("bit 3\n");
	if (!(isSet(b, 4) == 1)) printf("bit 4\n");
	if (!(isSet(b, 5) == 1)) printf("bit 5\n");
	if (!(isSet(b, 6) == 1)) printf("bit 6\n");
	if (!(isSet(b, 7) == 1)) printf("bit 7\n");

	free(pieces);
	freeBitfield(b);
}

void testThreeBytes()
{
	bitfield *b = newBitfield(24);
	for (int i=0; i<24; ++i) {
		setBit(b, i, i % 2);
	}
	printf("010101010101010101010101\n");
	printBitfield(b);
	printf("\n");
	printf("\n");

	for (int i=0; i<24; ++i) {
		setBit(b, i, 0);
	}
	printf("000000000000000000000000\n");
	printBitfield(b);
	printf("\n");
	printf("\n");

	for (int i=0; i<24; ++i) {
		setBit(b, i, 1);
	}
	printf("111111111111111111111111\n");
	printBitfield(b);
	printf("\n");
	printf("\n");

	freeBitfield(b);
}

void testEightBytes()
{
	bitfield *b = newBitfield(64);

	printf("00000000000000000000000000000000000000000000000000000"
			"00000000000\n");
	printBitfield(b);
	printf("\n\n");
	
	for (int i=0; i<64; ++i) {
		setBit(b, i, 1);
	}
	printf("11111111111111111111111111111111111111111111111111111"
			"11111111111\n");
	printBitfield(b);
	printf("\n\n");

	for (int i=0; i<64; ++i) {
		setBit(b, i, 0);
	}
	printf("00000000000000000000000000000000000000000000000000000"
			"00000000000\n");
	printBitfield(b);
	printf("\n\n");
}

void testTenBits()
{
	bitfield *b = newBitfield(10);
	
	for (int i=0; i<10; ++i) {
		setBit(b, i, i%2);
	}
	printf("0101010101000000\n");
	printBitfield(b);
	printf("\n");
	printf("\n");


	unsigned int *pieces;
	size_t npieces;
	getSet(b, &pieces, &npieces);
	
	printf("1 3 5 7 9\n");
	for (size_t i=0; i<npieces; ++i) {
		printf("%d ", pieces[i]);
	}
	printf("\n");
	printf("\n");


	free(pieces);
	freeBitfield(b);
}

int main()
{
	testSingleByte();
	testThreeBytes();
	testEightBytes();
	testTenBits();
}

