# Makefile for analog 6.0, libpng subdirectory
# Please read docs/Readme.html, or http://www.analog.cx/

HEADERS = png.h pngconf.h ../zlib/zlib.h
OBJS = png.o pngerror.o pngmem.o pngset.o pngtrans.o pngwio.o pngwrite.o \
	pngwtran.o pngwutil.o

all: $(OBJS)

png.o: png.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c png.c

pngerror.o: pngerror.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngerror.c

pngmem.o: pngmem.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngmem.c

pngset.o: pngset.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngset.c

pngtrans.o: pngtrans.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngtrans.c

pngwio.o: pngwio.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngwio.c

pngwrite.o: pngwrite.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngwrite.c

pngwtran.o: pngwtran.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngwtran.c

pngwutil.o: pngwutil.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) $(INCDIRS) -c pngwutil.c

clean:
	rm -f *.o *~
