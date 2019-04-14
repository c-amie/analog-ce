# Makefile for analog 6.0, libgd subdirectory
# Please read docs/Readme.html, or http://www.analog.cx/

HEADERS = gd.h gd_io.h gdfontf.h gdfonts.h ../zlib/zlib.h ../libpng/png.h
OBJS = gd.o gd_io.o gd_io_file.o gd_png.o gdfontf.o gdfonts.o gdtables.o

all: $(OBJS)

gd.o: gd.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c gd.c

gd_io.o: gd_io.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c gd_io.c

gd_io_file.o: gd_io_file.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c gd_io_file.c

gd_png.o: gd_png.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c gd_png.c

gdfontf.o: gdfontf.c $(HEADERS) gdfontf.h Makefile
	$(CC) $(ALLCFLAGS) -c gdfontf.c

gdfonts.o: gdfonts.c $(HEADERS) gdfonts.h Makefile
	$(CC) $(ALLCFLAGS) -c gdfonts.c

gdtables.o: gdtables.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c gdtables.c

clean:
	rm -f *.o *~
