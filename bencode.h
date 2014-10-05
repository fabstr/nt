#ifndef BENCODE_H
#define BENCODE_H

#define DEBUGGING 0

#define DEBUG(...) {if (DEBUGGING) fprintf(stderr, __VA_ARGS__);}

typedef struct list list;
typedef struct dictionary dictionary;
typedef struct value value;

struct value {
	enum {STRING, INTEGER, LIST, DICTIONARY} type;
	union {
		long int i;
		char *s;
		list *l;
		dictionary *d;
	} v;
};

struct dictionary {
	char *key;
	value *v;
	dictionary *next;
};

struct list {
	value *v;
	list *next;
};

int getDigitsRequiredInString(long int i);
int encodeString(char *s, FILE *f);
int encodeInteger(long int i, FILE *f);
int encodeList(list *i, FILE *f);
int encodeDictionary(dictionary *d, FILE *f);
int encode(value *v, FILE *f);

int decodeString(char *str, size_t len, value *v);
int decodeInteger(char *str, size_t strlen, value *v);
int decodeList(char *str, size_t strlen, value *v);
int decodeDictionary(char *str, size_t strlen, value *v);
int decode(char *str, size_t strlen, value *v);

void printValue(value *v);

void freeList(list *l);
void freeDictionary(dictionary *d);
void freeValue(value *v);

int valueEqual(value *v1, value *v2);

int testDecodeString();
int testDecodeInteger();
int testDecodeList();
int testDecodeDictionary();
int testDecodeDictionary();
int testValueEqual();

int testDigitsRequired();
int testBencodeDecoding();
int testBencodeEncoding();

#endif // BENCODE_H
