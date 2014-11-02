#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "bencode.h"
#include "sha1.h"

int getDigitsRequiredInString(long int i)
{
	if (i < 0) {
		return getDigitsRequiredInString(-i)+1;
	} else if (i <= 9) {
		return 1;
	}

	double log = log10(i);
	double intpart;
	modf(log, &intpart);

	int answer = (int) intpart + 1;

	return answer;
}

int encodeString(char *s, FILE *f)
{
	int len = strlen(s);
	fprintf(f, "%d:%s", len, s);
	return 0;
}

int encodeInteger(long int i, FILE *f)
{
	fprintf(f, "i%lde", i);
	return 0;
}

int encodeList(list *l, FILE *f)
{
	fprintf(f, "l");

	list *curr = l;
	while (curr != NULL) {
		encode(curr -> v, f);
		curr = curr -> next;
	}


	fprintf(f, "e");
	return 0;
}

int encodeDictionary(dictionary *d, FILE *f)
{
	fprintf(f, "d");

	dictionary *curr = d;
	while (curr != NULL) {
		encodeString(curr->key, f);
		encode(curr->v, f);
		curr = curr -> next;
	}

	fprintf(f, "e");
	return 0;
}

int encode(value *v, FILE *f) 
{
	int retval;
	switch (v -> type) {
		case STRING:
			retval = encodeString(v -> v.s, f);
			break;
		case INTEGER:
			retval = encodeInteger(v -> v.i, f);
			break;
		case LIST:
			retval = encodeList(v -> v.l, f);
			break;
		case DICTIONARY:
			retval = encodeDictionary(v -> v.d, f);
			break;
		default:
			return 1;
	}
	return retval;
}

int decodeString(char *str, size_t strlen, value *v)
{
	DEBUG("decoding string from '%s' (strlen=%ld)\n", str, strlen);
	long int size=0;
	int assigned = sscanf(str, "%ld:", &size);
	if (assigned != 1) {
		DEBUG("decodeString: didnt read size, assigned=%d\n", assigned);
		return 0;
	}

	char *decodedString = (char *) malloc(size+1);
	int offset = getDigitsRequiredInString(size)+1;
	DEBUG("have offset=%d\n", offset);
	DEBUG("apparent string to copy from: '%s'\n", str+offset);
	int offsetBegin = offset;
	int binary = 0;
	int i;
	for (i=0; i<size && i<=strlen-offsetBegin; i++, offset++) {
		decodedString[i] = str[offset];
		if (decodedString[i] < 8 || 
				(decodedString[i] > 13 && 
				 decodedString[i] < 32)) {
			binary = 1;
		}
	}

	decodedString[i] = '\0';
	v -> type = STRING;
	v -> v.s = decodedString;
	v -> binary_string = binary;
	v -> strlen = size;

	DEBUG("decoded string: '%s' offset=%d\n", v->v.s, offset);
	DEBUG("left after decoding: '%s'\n", str+offset);

	return offset;
}

int decodeInteger(char *str, size_t strlen, value *v)
{
	DEBUG("decoding integer from '%s'\n", str);
	int consumed = 0;
	long int i;
	int assigned = sscanf(str, "i%lde", &i);
	if (assigned != 1) {
		DEBUG("decodeInteger: didnt read size, assigned=%d\n", assigned);
		return 0;
	}

	v -> type = INTEGER;
	v -> v.i = i;

	consumed = getDigitsRequiredInString(v->v.i);
	consumed += 2;

	DEBUG("%d consumed, left: '%s'\n", consumed, str+consumed);
	return consumed;
}

int decodeList(char *str, size_t strlen, value *v)
{
	DEBUG("decoding list from '%s'\n", str);

	list *l = (list *) malloc(sizeof(list));
	v -> v.l = l;
	v -> type = LIST;
	int pos = 1;

	int count = 0;
	while (pos < strlen) {
		++count;
		l -> v = (value *) malloc(sizeof(value));
		int consumed = decode(str+pos, strlen-pos, l -> v);
		if (consumed == 0) {
			DEBUG("decodeList: 0 was consumed\n");
			l -> next = NULL;
			return 0;
		}

		pos += consumed;

		if (str[pos] == 'e') {
			l -> next = NULL;
			break;
		} else {
			list *next = (list *) malloc(sizeof(list));
			l->next = next;
			l = next;
		}
	}
	if (count == 0) {
		free(l->v);
		free(l);
		v -> v.l = NULL;
	}
	DEBUG("done decoding list, returning %d\n", pos+1);
	return pos+1;
}

int decodeDictionary(char *str, size_t strlen, value *v)
{
	DEBUG("enter decodeDictionary\n");
	dictionary *d = (dictionary *) malloc(sizeof(dictionary));
	v -> v.d = d;
	v -> type = DICTIONARY;
	int pos = 1;
	int level = 1;

	DEBUG("enter loop\n");
	while (pos < strlen && level > 0) {
		DEBUG("begin loop, getting key, pos=%d\n", pos);
		// get the key
		value* keyValue = (value *) malloc(sizeof(value));
		int consumed = decodeString(str+pos, strlen-pos, keyValue);
		if (consumed == 0) {
			DEBUG("decodeDictionary: 0 was consumed whilst reading key\n");
			d -> next = NULL;
			return 0;
		}

		pos += consumed;
		DEBUG("string decoded (%d consumed) pos=%d\n", consumed, pos);

		int length;
		for (length=0; keyValue->v.s[length] != '\0'; length++) ;
		char *keyString = (char *) malloc(length+1);
		for (int i=0; i<length; i++) {
			keyString[i] = keyValue->v.s[i];
		}
		keyString[length] = '\0';
		freeValue(keyValue);

		DEBUG("got key %s\n", keyString);

		d -> key = keyString;
		d -> v = (value *) malloc(sizeof(value));
		consumed = decode(str+pos, strlen-pos, d -> v);
		if (consumed == 0) {
			DEBUG("decodeDictionary: 0 was consumed whilst reading value\n");
			d -> next = NULL;
			return 0;
		}

		pos += consumed;

		DEBUG("value consumed=%d pos=%d left='%s'\n", consumed, pos, str+pos);

		if (str[pos] == 'e') {
			d -> next = NULL;
			break;
		} else {
			dictionary *next = (dictionary *) malloc(sizeof(dictionary));
			d->next = next;
			d = next;
		}
	}
	DEBUG("done decodeDictionary, returning %d\n", pos+1);
	return pos+1;
}

int decode(char *str, size_t strlen, value *v)
{
	char type = str[0];
	switch (type) {
		case 'i':
			DEBUG("decoding int\n");
			return decodeInteger(str, strlen, v);
			break;
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			DEBUG("decoding string\n");
			return decodeString(str, strlen, v);
			break;
		case 'l':
			DEBUG("decoding list\n");
			return decodeList(str, strlen, v);
			break;
		case 'd':
			DEBUG("decoding dictionary\n");
			return decodeDictionary(str, strlen, v);
			break;
		default: 
			return 0;
			break;
	}
}

value *dictionaryGetByKey(value *v, char *key)
{
	if (v == NULL || v -> type != DICTIONARY) {
		return NULL;
	}

	dictionary *dict = v -> v.d;

	while (dict != NULL) {
		if (strcmp(key, dict -> key) == 0) {
			return dict -> v;
		}

		dict = dict -> next;
	}

	return NULL;
}

unsigned char* calculateInfoHash(char *torrentfile)
{
	value* torrent = decodeFile(torrentfile);

	value *infovalue = dictionaryGetByKey(torrent, "info");
	if (infovalue == NULL) {
		return NULL;
	}

	// get the bencode string 
	char *buff;
	size_t size;
	FILE *f = open_memstream(&buff, &size);
	encode(infovalue, f);
	fclose(f);

	unsigned char *hash = (unsigned char *) malloc(20);
	sha1((const unsigned char *) buff, size, hash);

	free(buff);

	return hash;
}

value* decodeFile(char *filename)
{
	DEBUG("decoding file '%s'\n", filename);

	FILE *f = fopen(filename, "rb");

	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *str = (char *) malloc(len+1);
	fread(str, len, 1, f);
	str[len] = '\0';

	fclose(f);

	value *v = (value *) malloc(sizeof(value));
	int decoded = decode(str, len, v);
	if (decoded == 0) {
		DEBUG("decoded 0 in decodeFile\n");
		free(str);
		return NULL;
	}
	free(str);
	return v;
}

void printValue(value *v)
{
	if (v == NULL) {
		printf("NULL");
	} else if (v->type == INTEGER) {
		printf("%ld", v->v.i);
	} else if (v->type == STRING) {
		if (v -> binary_string == 1) {
			printf("\"");
			for (int i=0; i<v -> strlen; ++i) {
				unsigned char c = v -> v.s[i];
				if (c <= 0xF) {
					printf("0");
				}
				printf("%x", c);
			}
			printf("\"");
		} else {
			printf("\"%s\"", v->v.s);
		}
	} else if (v->type == LIST) {
		printf("[");
		list *l = v -> v.l;
		while (l != NULL) {
			printValue(l->v);
			if (l -> next != NULL) {
				printf(", ");
			}
			l = l -> next;
		}
		printf("]");
	} else if (v->type == DICTIONARY) {
		printf("{");
		dictionary *d = v -> v.d;
		while (d != NULL) {
			printf("\"%s\": ", d -> key);
			printValue(d->v);
			if (d -> next != NULL) {
				printf(", ");
			}
			d = d -> next;
		}
		printf("}");
	} else {
		printf("unknown type %d", v->type);
	}
}

void freeList(list *l)
{
	/*printf("freeList: l=%p\n", l);*/
	while (l != NULL) {
		list *next = l -> next;
		freeValue(l->v);
		free(l);
		l = next;
	}
}

void freeDictionary(dictionary *d)
{
	dictionary *next; 
	while (d != NULL) {
		next = d -> next;
		free(d -> key);
		freeValue(d -> v);
		free(d);
		d = next;
	}
}

void freeValue(value *v)
{
	if (v == NULL) return;

	/*printf("freeValue: v=%p ", v);*/
	if (v -> type == STRING) {
		/*printf("string v=%p", v);*/
		free(v -> v.s);
	} else if (v -> type == LIST) {
		/*printf("list v=%p:\n", v);*/
		freeList(v -> v.l);
	} else if (v -> type == INTEGER) {
		/*printf("integer v=%p", v);*/
	} else if (v -> type == DICTIONARY) {
		freeDictionary(v -> v.d);
	}
	/*printf(" FREEING\n");*/
	free(v);
}

// return 1 if v1 equals v2
// else 0
int valueEqual(value *v1, value* v2)
{
	if (v1 == NULL && v2 == NULL) {
		return 0;
	} else if (v1 == NULL || v2 == NULL) {
		return 1;
	}

	if (v1 -> type != v2 -> type) {
		return 0;
	} else if (v1 -> type == INTEGER && v1 -> v.i == v2 -> v.i) {
		return 1;
	} else if (v1 -> type == STRING && strcmp(v1 -> v.s, v2 -> v.s) == 0) {
		return 1;
	} else if (v1 -> type == LIST) {
		list *l1 = v1 -> v.l;
		list *l2 = v2 -> v.l;
		list *next1, *next2;

		while (l1 != NULL && l2 != NULL) {
			next1 = l1 -> next;
			next2 = l2 -> next;

			if (!valueEqual(l1->v, l2->v)) return 0;

			l1 = next1;
			l2 = next2;
		}

		if (l1 == NULL && l2 == NULL) return 1;
	} else if (v1 -> type == DICTIONARY) {
		dictionary *d1 = v1 -> v.d;
		dictionary *d2 = v2 -> v.d;
		dictionary *next1, *next2;

		while (d1 != NULL && d2 != NULL) {
			next1 = d1 -> next;
			next2 = d2 -> next;

			if (strcmp(d1->key, d2->key) != 0) return 0;
			if (!valueEqual(d1->v, d2->v)) return 0;

			d1 = next1;
			d2 = next2;
		}

		if (d1 == NULL && d2 == NULL) return 1;
	} 
	return 0;
}

