#ifndef TESTBENCODE_H
#define TESTBENCODE_H

#include "bencode.h"

int testDecodeString();
int testDecodeInteger();
int testDecodeList();
int testDecodeDictionary();
int testValueEqual();
int testCalculateInfoHash();

int testDigitsRequired();
int testBencodeDecoding();
int testBencodeEncoding();

#endif // TESTBENCODE_H
