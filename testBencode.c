#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "bencode.h"

int testBencodeEncoding() 
{
	char *str = "hej";
	printf("3:hej:\n");
	encodeString(str, stdout);
	printf("\n");

	int i = 1234;
	printf("i1234e:\n");
	encodeInteger(i, stdout);
	printf("\n");

	long foo = 6123456789;
	printf("i6123456789e:\n");
	encodeInteger(foo, stdout);
	printf("\n");

	value vv1 = {.type = INTEGER, .v.i = 1};
	value vv2 = {.type = INTEGER, .v.i = 2};
	value vv3 = {.type = INTEGER, .v.i = 3};
	list ll1 = {.v=&vv1};
	list ll2 = {.v=&vv2};
	list ll3 = {.v=&vv3};
	ll1.next = &ll2;
	ll2.next = &ll3;
	ll3.next = NULL;

	value v1 = {.type=STRING, .v.s="hej"};
	value v2 = {.type=INTEGER, .v.i=21345};
	value v3 = {.type=LIST, .v.l=&ll1};
	list l1 = {.v = &v1};
	list l2 = {.v = &v2};
	list l3 = {.v = &v3};
	l1.next = &l2;
	l2.next = &l3;
	l3.next = NULL;

	printf("l3:heji21345eli1ei2ei3eee:\n");
	encodeList(&l1, stdout);
	printf("\n");

	value dv1 = {.type=INTEGER, .v.i=1234};
	value dv2 = {.type=STRING, .v.s="or foo?"};
	value dv3 = {.type=LIST, .v.l = &ll1};
	dictionary d1 = {.key="foo", .v = &dv1};
	dictionary d2 = {.key="bar", .v = &dv2};
	dictionary d3 = {.key="cat", .v = &dv3};
	d1.next = &d2;
	d2.next = &d3;
	d3.next = NULL;

	printf("d3:fooi1234e3:bar7:or foo?3:catli1ei2ei3eee:\n");
	encodeDictionary(&d1, stdout);
	printf("\n");

	return 0;
}

int testDecodeString()
{
	printf("Testing decoding strings\n");

	char *str1 = "3:hej";
	char *str2 = "11:asdfghjkloi";
	char *str1Correct = "hej";
	char *str2Correct = "asdfghjkloi";
	value* v1 = (value *) malloc(sizeof(value));
	value* v2 = (value *) malloc(sizeof(value));
	int consumed;
	int passed = 1;

	consumed = decodeString(str1, strlen(str1)+1, v1);
	if (consumed != 5) {
		passed = 0;
		printf("consumed=%d should be 5\n", consumed);
	} else if (v1-> type != STRING) {
		passed = 0;
		printf("wrong type: %d should be STRING\n", v1 -> type);
	} else if (strcmp(str1Correct, v1->v.s) != 0) {
		passed = 0;
		printf("Expected '%s'\n", str1Correct);
		printf("Got      '%s'\n", v1->v.s);
	}

	consumed = decodeString(str2, strlen(str2)+1, v2);
	if (consumed != 14) {
		passed = 0;
		printf("consumed=%d should be 14\n", consumed);
	} else if (v2-> type != STRING) {
		passed = 0;
		printf("wrong type: %d should be STRING\n", v2 -> type);
	} else if (strcmp(str2Correct, v2->v.s) != 0) {
		passed = 0;
		printf("Expected '%s'\n", str2Correct);
		printf("Got      '%s'\n", v2->v.s);
	}
	freeValue(v1);
	freeValue(v2);

	return passed;
}

int testDecodeInteger() 
{
	printf("Testing decoding integers\n");

	int passed = 1;
	value *v1 = (value *) malloc(sizeof(value));
	value *v2 = (value *) malloc(sizeof(value));
	value *v3 = (value *) malloc(sizeof(value));
	value *v4 = (value *) malloc(sizeof(value));
	value *v11 = (value *) malloc(sizeof(value));
	int consumed;

	int i1 = 1;
	char *c1 = "i1e";
	consumed = decodeInteger(c1, strlen(c1)+1, v1);
	if (consumed != 3) {
		passed = 0;
		printf("consumed=%d should be 3\n", consumed);
	} else if (v1 -> type != INTEGER) {
		passed = 0;
		printf("wrong type: %d should be INTEGER\n", v1 -> type);
	} else if (v1 -> v.i != i1) {
		passed = 0;
		printf("Expected %d\n", i1);
		printf("Got      %ld\n", v1 -> v.i);
	}

	int i2 = 12;
	char* c2 = "i12e";
	consumed = decodeInteger(c2, strlen(c2)+1, v2);
	if (consumed != 4) {
		passed = 0;
		printf("consumed=%d should be 4\n", consumed);
	} else if (v2 -> type != INTEGER) {
		passed = 0;
		printf("wrong type: %d should be INTEGER\n", v2 -> type);
	} else if (v2 -> v.i != i2) {
		passed = 0;
		printf("Expected %d\n", i2);
		printf("Got      %ld\n", v2 -> v.i);
	}


	int i3 = 123;
	char* c3 = "i123e";
	consumed = decodeInteger(c3, strlen(c3)+1, v3);
	if (consumed != 5) {
		passed = 0;
		printf("consumed=%d should be 5\n", consumed);
	} else if (v3 -> type != INTEGER) {
		passed = 0;
		printf("wrong type: %d should be INTEGER\n", v3 -> type);
	} else if (v3 -> v.i != i3) {
		passed = 0;
		printf("Expected %d\n", i3);
		printf("Got      %ld\n", v3 -> v.i);
	}


	int i4 = 1234;
	char* c4 = "i1234e";
	consumed = decodeInteger(c4, strlen(c4)+1, v4);
	if (consumed != 6) {
		passed = 0;
		printf("consumed=%d should be 3\n", consumed);
	} else if (v4 -> type != INTEGER) {
		passed = 0;
		printf("wrong type: %d should be INTEGER\n", v4 -> type);
	} else if (v4 -> v.i != i4) {
		passed = 0;
		printf("Expected %d\n", i4);
		printf("Got      %ld\n", v4 -> v.i);
	}


	long int i11 = 71234567890;
	char* c11 = "i71234567890e";
	consumed = decodeInteger(c11, strlen(c1)+1, v11);
	if (consumed != 13) {
		passed = 0;
		printf("consumed=%d should be 3\n", consumed);
	} else if (v11 -> type != INTEGER) {
		passed = 0;
		printf("wrong type: %d should be INTEGER\n", v11 -> type);
	} else if (v11 -> v.i != i11) {
		passed = 0;
		printf("Expected %ld\n", i11);
		printf("Got      %ld\n", v11 -> v.i);
	}


	freeValue(v1);
	freeValue(v2);
	freeValue(v3);
	freeValue(v4);
	freeValue(v11);

	return passed;
}

int testDecode() 
{
	printf("Testing decode()\n");
	int passed = 1;
	int consumed;

	char *str = "3:hej";
	char *i = "i123e";

	value *vStr = (value *) malloc(sizeof(value));
	value *vI = (value *) malloc(sizeof(value));

	consumed = decode(str, strlen(str)+1, vStr);
	if (consumed != 5) {
		passed = 0;
		printf("consumed=%d should be 5\n", consumed);
	} else if (vStr->type != STRING) {
		passed = 0;
		printf("wrong type: %d should be STRING\n", vStr->type);
	} else if (strcmp("hej", vStr->v.s) != 0) {
		passed = 0;
		printf("Expected 'hej'\n");
		printf("Got      '%s'\n", vStr->v.s);
	}

	consumed = decode(i, strlen(i)+1, vI);
	if (consumed != 5) {
		passed = 0;
		printf("consumed=%d should be 5\n", consumed);
	} else if (vI->type != INTEGER) {
		passed = 0;
		printf("wrong type: %d should be INTEGER\n", vStr->type);
	} else if (vI->v.i != 123) {
		passed = 0;
		printf("Expected 123\n");
		printf("Got      '%ld'\n", vI->v.i);
	}


	freeValue(vStr);
	freeValue(vI);
	return passed;
}

int testDecodeList()
{
	printf("Testing decoding lists\n");
	int passed = 1;
	int consumed;

	char *lststr = "li1ei2ei3ei4ee";
	value* listDec = (value *) malloc(sizeof(value));
	consumed = decode(lststr, strlen(lststr)+1, listDec); 
	if (consumed != 14) {
		printf("consumed=%d expected 14\n", consumed);
		passed = 0;
	} else {
		list *l = listDec -> v.l;
		int i=1;
		while (l != NULL) {
			list *next = l -> next;
			if (l->v -> type != INTEGER || l->v -> v.i != i++) {
				passed = 0;
				break;
			}
			l = next;
		}
		if (passed == 0) {
			printf("decode list failure: got ");
			printValue(listDec);
			printf("\n");
		}
	}
	freeValue(listDec);

	char *lststr2 = "le";
	value *list2Dec = (value *) malloc(sizeof(value));
	consumed = decode(lststr2, strlen(lststr2)+1, list2Dec);
	if (consumed != 2) {
		printf("consumed=%d expected 2\n", consumed);
		passed = 0;
	} else {
		if (list2Dec -> type != LIST) {
			printf("wrong type: %d should be LIST\n", list2Dec->type);
			passed = 0;
		} else if (list2Dec -> v.l != NULL) {
			printf("list not null: %p\n", list2Dec->v.l);
			passed = 0;
		}
	}
	freeValue(list2Dec);


	char *lststr3 = "l3:hej4:idag3:apae";
	value *list3Dec = (value *) malloc(sizeof(value));
	consumed = decode(lststr3, strlen(lststr3)+1, list3Dec);
	if (consumed != 18) {
		printf("consumed=%d expected 18\n", consumed);
		passed = 0;
	} else {
		value strings[3];
		strings[0].type = STRING; strings[0].v.s = "hej";
		strings[1].type = STRING; strings[1].v.s = "idag";
		strings[2].type = STRING; strings[2].v.s = "apa";
		list *l = list3Dec -> v.l;
		int i=0;
		while (l != NULL) {
			list *next = l -> next;
			if (l -> v -> type != STRING) {
				printf("wrong type: %d should be STRING\n", l -> v -> type);
				passed = 0;
			} else if (strcmp(strings[i].v.s, l -> v -> v.s) != 0) {
				printf("wrong value: %s should be %s\n", l -> v -> v.s, strings[i].v.s);
				passed = 0;
			}
			++i;
			l = next;
		}
		if (i != 3) {
			printf("wrong number of elements in list, %d should be 3\n", i);
			passed = 0;
		}
	}
	freeValue(list3Dec);
	
	char *lststr4 = "ld3:foo3:bar3:apa5:bananee";
	value *list4Dec = (value *) malloc(sizeof(value));
	fprintf(stderr, "FOOOOO\n");
	consumed = decode(lststr4, strlen(lststr4)+1, list4Dec);
	fprintf(stderr, "FOOOOO\n");
	if (consumed != 26) {
		printf("consumed=%d expected 26\n", consumed);
		passed = 0;
	} else {
		printf("lista: ");
		printValue(list4Dec);
		printf("\n");
	}
	freeValue(list4Dec);

	return passed;
}

int testDecodeDictionary()
{
	printf("Testing decoding dictionary\n");

	int passed = 1;
	char *dictStr = "d3:fooi1234e3:bar7:or foo?3:catli1ei2ei3eee";
	value* dictDec = (value *) malloc(sizeof(value));
	if (dictDec == NULL) return 0;
	decode(dictStr, strlen(dictStr)+1, dictDec);


	value vl1 = {.type=INTEGER, .v.i=1};
	value vl2 = {.type=INTEGER, .v.i=2};
	value vl3 = {.type=INTEGER, .v.i=3};
	list l3 = {.next = NULL, .v = &vl3};
	list l2 = {.next = &l3, .v = &vl2};
	list l1 = {.next = &l2, .v = &vl1};
	value v1 = {.type=INTEGER, .v.i=1234};
	value v2 = {.type=STRING, .v.s="or foo?"};
	value v3 = {.type=LIST, .v.l = &l1};
	dictionary d3 = {.key="cat", .next=NULL, .v=&v3};
	dictionary d2 = {.key="bar", .next=&d3, .v=&v2};
	dictionary d1 = {.key="foo", .next=&d2, .v=&v1}; 
	value correctDict = {.type = DICTIONARY, .v.d = &d1};
	if (!valueEqual(dictDec, &correctDict)) {
		passed = 0;
		printf("dictionaries not equal\n");
	}

	freeValue(dictDec);

	return passed;
}

int testBencodeDecoding()
{
	int passed = 1;
	if (testDecodeString() == 0) {
		passed = 0;
		printf("decodeString FAILED\n");
	}

	if (testDecodeInteger() == 0) {
		passed = 0;
		printf("decodeInteger FAILED\n");
	}

	if (testDecode() == 0) {
		passed = 0;
		printf("decode FAILED\n");
	}

	if (testDecodeList() == 0) {
		passed = 0;
		printf("decodeList FAILED\n");
	}

	if (testDecodeDictionary() == 0) {
		passed = 0;
		printf("decodeDictionary FAILED\n");
	}

	return passed;
}

int testDigitsRequired()
{
	int i0=0, c0=1;
	int i1=1, c1=1;
	int i2=12, c2=2;
	int i3=123, c3=3;
	int i4=1234, c4=4;
	long int i11=61234567890, c11=11;

	int passed = 1;

	if (getDigitsRequiredInString(i0) != c0) {
		passed = 0;
		printf("i0=%d should be %d\n", i0, c0);
	}
	
	if (getDigitsRequiredInString(i1) != c1) {
		passed = 0;
		printf("i1=%d should be %d\n", i1, c1);
	}
	
	if (getDigitsRequiredInString(i2) != c2) {
		passed = 0;
		printf("i2=%d should be %d\n", i2, c2);
	}
	
	if (getDigitsRequiredInString(i3) != c3) {
		passed = 0;
		printf("i3=%d should be %d\n", i3, c3);
	}
	
	if (getDigitsRequiredInString(i4) != c4) {
		passed = 0;
		printf("i4=%d should be %d\n", i4, c4);
	}
	
	if (getDigitsRequiredInString(i11) != c11) {
		passed = 0;
		printf("i11=%ld should be %ld\n", i11, c11);
	}

	return passed;
}

int testValueEqual()
{
	int passed = 1;

	printf("Testing Strings equal\n");
	value v1 = {.type=STRING, .v.s = "hej"};
	value v2 = {.type=STRING, .v.s = "hej"};
	if (!valueEqual(&v1, &v2)) {
		passed = 0;
		printf("Values not equal: ");
		printValue(&v1);
		printf(" ");
		printValue(&v2);
		printf("\n");
	}

	printf("Testing Ints equal\n");
	value v3 = {.type=INTEGER, .v.i = 123};
	value v4 = {.type=INTEGER, .v.i = 123};
	if (!valueEqual(&v3, &v4)) {
		passed = 0;
		printf("Values not equal: ");
		printValue(&v3);
		printf(" ");
		printValue(&v4);
		printf("\n");
	}

	printf("Testing Ints not equal\n");
	value v5 = {.type=INTEGER, .v.i = 123};
	value v6 = {.type=INTEGER, .v.i = 124};
	if (valueEqual(&v5, &v6)) {
		passed = 0;
		printf("Values equal: ");
		printValue(&v3);
		printf(" ");
		printValue(&v4);
		printf("\n");
	}

	printf("Testing Int and string not equal\n");
	value v7 = {.type=INTEGER, .v.i = 124};
	value v8 = {.type=STRING, .v.s = "hej"};
	if (valueEqual(&v7, &v8)) {
		passed = 0;
		printf("Values equal: ");
		printValue(&v3);
		printf(" ");
		printValue(&v4);
		printf("\n");
	}

	printf("Testing Lists equal\n");
	value v11 = {.type=INTEGER, .v.i=3};
	value v12 = {.type=INTEGER, .v.i=6};
	value v13 = {.type=INTEGER, .v.i=9};
	value v21 = {.type=INTEGER, .v.i=3};
	value v22 = {.type=INTEGER, .v.i=6};
	value v23 = {.type=INTEGER, .v.i=9};
	list l3 = {.v = &v11, .next=NULL};
	list l2 = {.v = &v12, .next=&l3};
	list l1 = {.v = &v13, .next=&l2};
	list k3 = {.v = &v21, .next=NULL};
	list k2 = {.v = &v22, .next=&k3};
	list k1 = {.v = &v23, .next=&k2};
	value list1 = {.type=LIST, .v.l = &l1};
	value list2 = {.type=LIST, .v.l = &k1};
	if (!valueEqual(&list1, &list2)) {
		passed = 0;
		printf("Lists not equal:\n");
		printValue(&v11);
		printf("\n");
		printValue(&v21);
	}

	printf("Testing Lists not equal\n");
	value w11 = {.type=INTEGER, .v.i=3};
	value w12 = {.type=INTEGER, .v.i=6};
	value w13 = {.type=INTEGER, .v.i=9};
	value w21 = {.type=INTEGER, .v.i=4};
	value w22 = {.type=INTEGER, .v.i=7};
	value w23 = {.type=INTEGER, .v.i=10};
	list m3 = {.v = &w11, .next=NULL};
	list m2 = {.v = &w12, .next=&m3};
	list m1 = {.v = &w13, .next=&m2};
	list n3 = {.v = &w21, .next=NULL};
	list n2 = {.v = &w22, .next=&n3};
	list n1 = {.v = &w23, .next=&n2};
	value list3 = {.type=LIST, .v.l = &m1};
	value list4 = {.type=LIST, .v.l = &n1};
	if (valueEqual(&list3, &list4)) {
		passed = 0;
		printf("Lists equal:\n");
		printValue(&w11);
		printf("\n");
		printValue(&w21);
	}

	return passed;
}

int main() 
{
	int passed = 1;
	if (testDigitsRequired() == 0) passed = 0;

	printf("\n\nENCODING:\n");
	if (testBencodeEncoding() != 0) passed = 0;

	printf("\n\nDECODING:\n");
	if (testBencodeDecoding() == 0) passed = 0;

	printf("\n\nVALUEEQUAL:\n");
	if (testValueEqual() == 0) passed = 0;
	
	printf("\n");

	if (passed == 1) printf("All tests passed\n");
	if (passed == 0) printf("There were errors\n");
	return !passed;
}
