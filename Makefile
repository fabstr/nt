CC=clang
CFLAGS=-Wall -g -I/usr/local/include
CLIBS=-lm -L/usr/local/lib -lcurl 
VALGRINDFLAGS=--leak-check=full --log-file=valgrind.log


all: bencode.o testBencode.o viewTorrent

test: clean testBencode

%.o: %.c %.h
	${CC} ${CFLAGS} ${CLIBS} -c $<

tracker: tracker.o bencode.o sha1.o
	${CC} ${CLAGS} ${CLIBS} -o tracker tracker.o bencode.o sha1.o

testBencode: bencode.o testBencode.o sha1.o
	${CC} ${CFLAGS} ${CLIBS} -o testBencode bencode.o testBencode.o sha1.o
	valgrind ${VALGRINDFLAGS} ./testBencode
	grep lost valgrind.log

testSHA1: sha1.o testSHA1.o
	${CC} ${CFLAGS} ${CLIBS} -o testSHA1 $?
	./testSHA1

viewTorrent: viewTorrent.o bencode.o sha1.o
	${CC} ${CLAGS} ${CLIBS} -o viewTorrent viewTorrent.o bencode.o sha1.o 

clean: 
	rm -f *.o *.core testBencode testSHA1 valgrind.log*
