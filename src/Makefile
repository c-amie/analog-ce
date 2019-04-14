# Makefile for analog 6.0
# Please read docs/Readme.html, or http://www.analog.cx/
# This is a general Unix-like Makefile: Makefiles for other OS's can be found
#   in the "build" directory.
CC = gcc           # which compiler to use: eg cc, acc, gcc. NB Different
#                    compilers need different CFLAGS, e.g., -O instead of -O2.
MAKE = make        # which "make" to use
CFLAGS = -O2    # options, e.g. for optimisation or ANSI compilation.
#                 Some OS's need -D_FILE_OFFSET_BITS=64 to support files > 2MB.
#                 HP/UX cc needs CFLAGS = -Aa (HP/UX 9) or -Ae (HP/UX 10)
#                 BeOS needs CFLAGS = -O2 -Wl,-L/boot/home/config/lib
#                 BS2000/OSD needs CFLAGS = -XLLML -XLLMK
#                 NeXTSTEP needs CFLAGS = -O2 -pipe -no-precomp 
DEFS =          # any combination of -DNOPIPES -DNODNS -DNODIRENT -DNOGLOB ...
#                 ... -DNOOPEN -DNOFOLLOW -DNOALARM -DNOGRAPHICS -DNOGMTIME ...
#                 ... -DEBCDIC -DUSE_PLAIN_SETJMP ...
#                 ... -DHAVE_GD -DHAVE_ZLIB -DHAVE_BZLIB -DHAVE_PCRE ...
#                 ... -DNEED_STRCMP -DNEED_MEMMOVE -DNEED_STRTOUL ...
#                 ... -DNEED_DIFFTIME -DHAVE_ADDR_T -DNEED_FLOATINGPOINT_H
#                 All the options are explained at the bottom of this file.
#
#                 Solaris 2 (SunOS 5+) might need DEFS = -DNEED_STRCMP
#                 SunOS 4 usually needs...
#   DEFS = -DNEED_MEMMOVE -DNEED_STRTOUL -DNEED_DIFFTIME -DNEED_FLOATINGPOINT_H
#                 64-bit Solaris and Tru64 version 5+ need DEFS = -DHAVE_ADDR_T
#                 SCO OpenServer 5 needs DEFS = -DPATH_MAX=2048
#                 BeOS 5.0.3 or below needs DEFS = -DNODNS
#                 DYNIX/ptx needs DEFS = -D_SOCKET_VERSION=11
#                 MPE/iX needs DEFS = -D_POSIX_SOURCE -D_SOCKET_SOURCE
#
OS = UNIX         # Operating system: UNIX, DOS, WIN32, MAC, OS2, OSX, VMS
#                   RISCOS, BEOS, NEXTSTEP, MPEIX, BS2000, AS400, OS390
LIBS = -lm  # extra libraries needed; most platforms (but not OS X or BeOS)
#               need -lm LAST
#             if you defined HAVE_GD above you also need -lgd -lpng -ljpeg -lz
#             if you defined HAVE_ZLIB above you also need -lz
#             if you defined HAVE_BZLIB above you also need -lbz2
#             if you defined HAVE_PCRE above you also need -lpcre
#        Also Solaris 2 (SunOS 5+) needs LIBS = -lnsl -lm
#             SCO Unix needs LIBS = -lsocket -lm
#             IRIX needs LIBS = -lc -lm
#             Windows compilers probably need LIBS = -lwsock32 -lm
#             BeOS 5.0.3 or below needs LIBS = -lnet -lglob; 5.0.4 or above
#             needs LIBS = -lbind -lglob; and install libglob from BeBits.com
#             OS/2 with EMX needs LIBS = -lsocket -lm
#             BS2000/OSD needs LIBS = -lsocket -lm
#             MPE/iX needs LIBS = -lsocket -lm
#             SVR4 needs LIBS = -lsocket -lnsl -lm
#             DYNIX/ptx needs LIBS = -L/usr/lib/bsd-socket -lsocket -lnsl -lm

PROGRAM = analog
TARGET = ../analog
OBJS = alias.o analog.o cache.o dates.o globals.o hash.o init.o init2.o \
	input.o macinput.o macstuff.o output.o output2.o outcro.o outhtml.o \
	outlatex.o outplain.o outxhtml.o outxml.o process.o settings.o sort.o \
	tree.o utils.o win32.o
SUBDIRS = bzip2 libgd libpng pcre unzip zlib
SUBDIROBJS = libgd/gd.o  libgd/gd_io.o libgd/gd_io_file.o libgd/gd_png.o \
	libgd/gdfontf.o libgd/gdfonts.o libgd/gdtables.o \
	libpng/png.o libpng/pngerror.o libpng/pngmem.o libpng/pngset.o \
	libpng/pngtrans.o libpng/pngwio.o libpng/pngwrite.o \
	libpng/pngwtran.o libpng/pngwutil.o pcre/pcre.o \
	zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/deflate.o \
	zlib/gzio.o zlib/infblock.o zlib/infcodes.o zlib/inffast.o \
	zlib/inflate.o zlib/inftrees.o zlib/infutil.o zlib/trees.o \
	zlib/uncompr.o zlib/zutil.o unzip/ioapi.o unzip/unzip.o \
	bzip2/bzlib.o bzip2/blocksort.o bzip2/compress.o bzip2/crctable.o \
	bzip2/decompress.o bzip2/huffman.o bzip2/randtable.o
HEADERS = anlghead.h anlghea2.h anlghea3.h anlghea4.h macdir.h \
	pcre/pcre.h libgd/gd.h libgd/gdfontf.h libgd/gdfonts.h unzip/unzip.h \
	zlib/zlib.h bzip2/bzlib.h
ALLCFLAGS = $(CFLAGS) $(DEFS) -D$(OS)
ALLOBJS = $(OBJS) $(SUBDIROBJS)

$(PROGRAM): $(OBJS) $(SUBDIRS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) -o $(TARGET) $(ALLOBJS) $(LIBS)

# There doesn't seem to be a good way to write all these rules in a generic
# form that works for all "make" programs
bzip2: ALWAYS
	cd bzip2 && $(MAKE) 'CC=$(CC)' 'ALLCFLAGS=$(ALLCFLAGS)'

libgd: ALWAYS
	cd libgd && $(MAKE) 'CC=$(CC)' 'ALLCFLAGS=$(ALLCFLAGS)'

libpng: ALWAYS
	cd libpng && $(MAKE) 'CC=$(CC)' 'ALLCFLAGS=$(ALLCFLAGS)'

pcre: ALWAYS
	cd pcre && $(MAKE) 'CC=$(CC)' 'ALLCFLAGS=$(ALLCFLAGS)'

unzip: ALWAYS
	cd unzip && $(MAKE) 'CC=$(CC)' 'ALLCFLAGS=$(ALLCFLAGS)'

zlib: ALWAYS
	cd zlib && $(MAKE) 'CC=$(CC)' 'ALLCFLAGS=$(ALLCFLAGS)'

alias.o: alias.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c alias.c

analog.o: analog.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c analog.c

cache.o: cache.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c cache.c

dates.o: dates.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c dates.c

globals.o: globals.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c globals.c

hash.o: hash.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c hash.c

init.o: init.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c init.c

init2.o: init2.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c init2.c

input.o: input.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c input.c

macinput.o: macinput.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c macinput.c

macstuff.o: macstuff.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c macstuff.c

output.o: output.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c output.c

output2.o: output2.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c output2.c

outcro.o: outcro.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c outcro.c

outhtml.o: outhtml.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c outhtml.c

outlatex.o: outlatex.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c outlatex.c

outplain.o: outplain.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c outplain.c

outxhtml.o: outxhtml.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c outxhtml.c

outxml.o: outxml.c outxml.h $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c outxml.c

process.o: process.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c process.c

settings.o: settings.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c settings.c

sort.o: sort.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c sort.c

tree.o: tree.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c tree.c

utils.o: utils.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c utils.c

win32.o: win32.c $(HEADERS) Makefile
	$(CC) $(ALLCFLAGS) -c win32.c

force:
	rm -f $(ALLOBJS)
	make $(PROGRAM) 'CFLAGS=$(CFLAGS)' 'LIBS=$(LIBS)' \
	'DEFS=$(DEFS)' 'CC=$(CC)'

clean:
	rm -f $(ALLOBJS) $(TARGET) *~ */*~

ALWAYS:

# OK, here's the details on all the DEFS options, in case anyone needs to know.
# NOPIPES excludes non-ANSI functions popen() and pclose(). You will lose the
#   UNCOMPRESS command.
# NODNS excludes all the DNS lookup code. This is platform-dependent, but
#   typically involves header files <net*.h> and functions like inet_addr()
#   and gethostbyaddr().
# NODIRENT stops you using wildcards in logfile names. This code is also
#   platform-dependent. But under Unix, the header file <glob.h> and function
#   glob() from POSIX.2 are used; _unless_ NOGLOB is defined, in which case
#   the header file <dirent.h> and functions stat(), opendir(), readdir() and
#   closedir() are used instead. In this latter case, you can't use wildcards
#   in directory names.
# NOOPEN excludes non-ANSI functions open() and fdopen(). This results in an
#   alternative, less safe locking mechanism for DNS files and cache files.
# NOFOLLOW omits code including the functions realpath(), access() and stat()
#   and the header file <unistd.h> and possibly <sys/param.h>. Analog won't
#   then follow symbolic links when guessing path names.
# NOALARM omits code including the function alarm() and the header file
#   <unistd.h>. You will lose the ability to set a DNS timeout.
#
# ** If all of the above are defined only features defined in ANSI C are used.
#
# NOGRAPHICS omits the code for generating pie charts, including all of libgd
#   and libpng.
# HAVE_GD means you already have libgd (at least version 1.8), libpng, libjpeg
#   and zlib, and you want to use your version of these libraries not
#   mine. This is likely to reduce the size of the binary. Also you will gain
#   the JPEGCHARTS command. You will also need to include
#   "-lgd -lpng -ljpeg -lz" in the LIBS above.
# HAVE_ZLIB means you already have zlib, and you want to use your version not
#   mine. This is likely to reduce the size of the binary. You will also need
#   to include "-lz" in the LIBS above.
# HAVE_BZLIB means you already have bzlib, and you want to use your version not
#   mine. This is likely to reduce the size of the binary. You will also need
#   to include "-lbz2" in the LIBS above.
# HAVE_PCRE means you already have libpcre, and you want to use your version
#   not mine. This is likely to reduce the size of the binary.
#   You will also need to include "-lpcre" in the LIBS.
# EBCDIC is needed if your server uses the EBCDIC character set, not ASCII.
# USE_PLAIN_SETJMP uses ANSI setjmp() instead of POSIX sigsetjmp().
# NEED_STRCMP, NEED_MEMMOVE, NEED_STRTOUL and NEED_DIFFTIME provide
#   replacements for standard functions missing on some systems.
# NOGMTIME doesn't use the gmtime() function.
# NEED_FLOATINGPOINT_H includes a header file called <floatingpoint.h>.
# HAVE_ADDR_T is needed if inet_addr() returns in_addr_t which is not the
#   same as unsigned long.
