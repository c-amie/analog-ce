# Makefile for analog 6.0, unzip subdirectory
# Please read docs/Readme.html, or http://www.analog.cx/

HEADERS = unzip.h ioapi.h ../zlib/zlib.h
OBJS = ioapi.o unzip.o

all: $(OBJS)

ioapi.o: ioapi.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c ioapi.c

unzip.o: unzip.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -DNOUNCRYPT $(INCDIRS) -c unzip.c

clean:
	rm -f *.o *~
