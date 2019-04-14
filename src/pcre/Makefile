# Makefile for analog 6.0, pcre subdirectory
# Please read docs/Readme.html, or http://www.analog.cx/

HEADERS = pcre.h pcrechar.h pcreintl.h
OBJS = pcre.o

all: $(OBJS)

pcre.o: pcre.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c pcre.c

clean:
	rm -f *.o *~
