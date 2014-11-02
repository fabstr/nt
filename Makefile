SOURCES= bencode.c curltest.c net.c sha1.c testBencode.c testSHA1.c tracker.c \
	 viewTorrent.c
OBJECTS=${SOURCES:.c=.o}
BINS= viewTorrent tracker testBencode viewTorrent net testSHA1 torrent

CC=clang
CFLAGS=-Wall -g -I/usr/local/include
CLIBS=-lm -L/usr/local/lib -lcurl 
VALGRINDFLAGS=--leak-check=full --log-file=valgrind.log

all: torrent

# binaries
torrent: torrent.o tracker.o bencode.o net.o sha1.o
	${CC} ${CFLAGS} ${CLIBS} -o torrent torrent.o tracker.o bencode.o\
		net.o sha1.o

tracker: tracker.o bencode.o sha1.o
	${CC} ${CFLAGS} ${CLIBS} -o tracker tracker.o bencode.o sha1.o

testBencode: bencode.o testBencode.o sha1.o
	${CC} ${CFLAGS} ${CLIBS} -o testBencode bencode.o testBencode.o sha1.o

sha1: sha1.o testSHA1.o
	${CC} ${CFLAGS} ${CLIBS} -o testSHA1 $?

testSHA1: sha1
	./testSHA1

viewTorrent: viewTorrent.o bencode.o sha1.o
	${CC} ${CLAGS} ${CLIBS} -o viewTorrent viewTorrent.o bencode.o sha1.o 

net: net.o 
	${CC} ${CLAGS} ${CLIBS} -o net net.o

# object files
%.o: %.c %.h
	${CC} ${CFLAGS} ${CLIBS} -c $<

# various stuff
clean: 
	rm -f *.o *.core ${BINS} valgrind.log*

test: clean testBencode
	valgrind ${VALGRINDFLAGS} ./testBencode
	grep lost valgrind.log
