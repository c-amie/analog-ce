# Makefile for analog 6.0, zlib subdirectory
# Please read docs/Readme.html, or http://www.analog.cx/

HEADERS = deflate.h infblock.h infcodes.h inffast.h inffixed.h inftrees.h \
	infutil.h trees.h zconf.h zlib.h zutil.h
OBJS = adler32.o compress.o crc32.o deflate.o gzio.o infblock.o infcodes.o \
	inffast.o inflate.o inftrees.o infutil.o trees.o uncompr.o zutil.o

all: $(OBJS)

adler32.o: adler32.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c adler32.c

compress.o: compress.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c compress.c

crc32.o: crc32.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c crc32.c

deflate.o: deflate.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c deflate.c

gzio.o: gzio.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c gzio.c

infblock.o: infblock.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c infblock.c

infcodes.o: infcodes.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c infcodes.c

inffast.o: inffast.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c inffast.c

inflate.o: inflate.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c inflate.c

inftrees.o: inftrees.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c inftrees.c

infutil.o: infutil.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c infutil.c

trees.o: trees.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c trees.c

uncompr.o: uncompr.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c uncompr.c

zutil.o: zutil.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c zutil.c

clean:
	rm -f *.o *~
