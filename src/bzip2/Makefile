# Makefile for analog 6.0, zlib subdirectory
# Please read docs/Readme.html, or http://www.analog.cx/

HEADERS = bzlib.h bzlib_private.h
OBJS = bzlib.o blocksort.o compress.o crctable.o decompress.o huffman.o \
	randtable.o

all: $(OBJS)

bzlib.o: bzlib.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c bzlib.c

blocksort.o: blocksort.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c blocksort.c

compress.o: compress.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c compress.c

crctable.o: crctable.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c crctable.c

decompress.o: decompress.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c decompress.c

huffman.o: huffman.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c huffman.c

randtable.o: randtable.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c randtable.c

clean:
	rm -f *.o *~
