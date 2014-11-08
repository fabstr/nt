#ifndef BITFIELD_H
#define BITFIELD_H 

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <stdio.h>


/*
 *   Byte 0                                     1
 * +-------------------------------------------+-------------------------------+
 * | Bit 1    2    3    4    5    6    7    8  | 9   10   11   12   13  14  15 |
 * +-------------------------------------------+-------------------------------+
 * | ...                                                                       |
 * +-------------------------------------------+-------------------------------+
 * | Bit n-15 n-14 n-13 n-12 n-11 n-10 n-9 n-8 | n-7 n-6 n-5 n-4 n-3 n-2 n-1 n |
 * +-------------------------------------------+-------------------------------+
 *  Byte k-1                                    k
 */

typedef struct bitfield bitfield;
struct bitfield {
	unsigned char *bytes;
	size_t nbytes;
	size_t nbits;
};

// treat value as bool: non-zero is 1, zero is 0
// return 0 if success, // -1 if bit is an invalid bit number,
int setBit(bitfield *bitfield, size_t bit, int value);

// return the value of bit pos
int isSet(bitfield *b, size_t pos);

// each int in pieces denotes a piece set in bitfield (array length is count) 
void getSet(bitfield *bitfield, unsigned int **pieces, size_t *count);

int countSetBits(bitfield *bitfield);

bitfield* newBitfield(size_t nbits);
void freeBitfield(bitfield *bitfield);

#endif // BITFIELD_H
