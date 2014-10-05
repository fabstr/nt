CC=clang
CFLAGS=-Wall -g
CLIBS=-lm
VALGRINDFLAGS=--leak-check=full --log-file=valgrind.log

test: clean testBencode

%.o: %.c %.h
	${CC} ${CFLAGS} ${CLIBS} -c $<

testBencode: bencode.o testBencode.o
	${CC} ${CFLAGS} ${CLIBS} -o testBencode $?
	valgrind ${VALGRINDFLAGS} ./testBencode
	grep lost valgrind.log

testSHA1: sha1.o testSHA1.o
	${CC} ${CFLAGS} ${CLIBS} -o testSHA1 $?
	./testSHA1


clean: 
	rm -f *.o *.core testBencode testSHA1 valgrind.log*
