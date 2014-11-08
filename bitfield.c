#include "bitfield.h"

int setBit(bitfield *bitfield, size_t bit, int value)
{
	if (bit > bitfield -> nbits) {
		// invalid bit number
		return -1;
	}

	// get position of byte and bit (in byte)
	size_t bytepos = bit / 8;
	bit -= bytepos*8;

	// get bitmask and set the byte to the new value 
	if (value == 0) {
		// set the bit number bit to 0
		unsigned char mask = ~(1<<(7-bit));
		bitfield -> bytes[bytepos] &= mask;
	} else {
		// set the bit number bit to 1
		unsigned char mask = 1<<(7-bit);
		bitfield -> bytes[bytepos] |= mask;
	}

	return 0;
}

int isSet(bitfield *b, size_t pos)
{
	size_t bytepos = pos / 8;
	pos -= bytepos*8;

	unsigned char byte = b -> bytes[bytepos];
	unsigned char mask = 1<<(7-pos);

	if (byte & mask) {
		return 1;
	}
	return 0;
}

void getSet(bitfield *bitfield, unsigned int **pieces, size_t *count)
{
	// how long should the array be?
	int nbitsset = countSetBits(bitfield);

	// allocate memory
	*pieces = (unsigned int *) malloc(nbitsset * sizeof(unsigned int));
	*count = nbitsset;

	size_t arrPos = 0;
	for (size_t bytePos=0; bytePos < bitfield -> nbytes; ++bytePos) {
		unsigned char byte = bitfield -> bytes[bytePos];
		for (int bitPos=0; bitPos<8 && bytePos*8+bitPos<bitfield->nbits; 
				++bitPos) {
			int bitIndex = bytePos*8 + bitPos;
			unsigned char mask = 1<<(7-bitPos);
			if (byte & mask) {
				(*pieces)[arrPos++] = bitIndex;
			}
		}
	}
}

int countSetBits(bitfield *bitfield)
{
	int count = 0;
	for (size_t i=0; i<bitfield -> nbytes; ++i) {
		unsigned char byte = bitfield -> bytes[i];
		for (int i=0; i<8; ++i) {
			unsigned char mask = 1<<(7-i);
			if (byte & mask) {
				count++;
			}
		}
	}

	return count;
}

bitfield* newBitfield(size_t nbits)
{
	double bits = nbits;
	size_t size = (int) ceil(bits / 8);

	bitfield *b = (bitfield *) malloc(sizeof(bitfield));

	// allocate memory for bytes and set all to 0
	b -> bytes = (unsigned char *) malloc(size);
	memset(b -> bytes, 0, size);

	b -> nbytes = size;
	b -> nbits = nbits;

	return b;
}

void freeBitfield(bitfield *bitfield)
{
	free(bitfield -> bytes);
	free(bitfield);
}
