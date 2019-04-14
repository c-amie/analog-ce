# Makefile for analog 6.0, RISC OS
# Please read docs/Readme.html, or http://www.analog.cx/
#
CC = cc
# which compiler to use: eg cc, acc, gcc. NB Different
# compilers need different CFLAGS, e.g., -O instead of -O2.

CFLAGS = -throwback -Wp -ansi -IC:,TCPIPLibs: -UCOMPAT_INET4 -DMINORVERSION="\"-sb1\""
# options, e.g. for optimisation or ANSI compilation.

DEFS = -DNOUNCRYPT -DBZ_STRICT_ANSI
# -DDEBUGSAFE=1024
# ... -DNOOPEN -DNOPIPES -DNODNS -DNODIRENT 
# ... -DEBCDIC -DNOGMTIME -DNEED_STRCMP -DNEED_MEMMOVE ...
# ... -DNEED_STRTOUL -DNEED_DIFFTIME -DHAVE_ADDR_T ...
# ... -DNEED_FLOATINGPOINT_H
# All the options are explained at the bottom of this file.

OS = RISCOS
# Operating system: UNIX, DOS, WIN32, MAC, OS2, OSX, VMS
#                   RISCOS, BEOS, NEXTSTEP, MPEIX, BS2000, AS400

LIBS = C:o.stubs TCPIPLibs:o.Socklib5 TCPIPLibs:o.Inetlib TCPIPLibs:o.Unixlib
DEPEND=-depend !Depend

OBJS = alias.o analog.o cache.o dates.o globals.o hash.o init.o init2.o \
	input.o output.o output2.o process.o settings.o sort.o tree.o \
	utils.o outcro.o outhtml.o outlatex.o outplain.o outxhtml.o \
	outxml.o

GD_OBJS = libgd.o.gd  libgd.o.gd_io libgd.o.gd_io_file libgd.o.gd_png \
	libgd.o.gdfontf libgd.o.gdfonts libgd.o.gdtables

PNG_OBJS = libpng.o.png libpng.o.pngerror libpng.o.pngmem libpng.o.pngset \
	libpng.o.pngtrans libpng.o.pngwio libpng.o.pngwrite \
	libpng.o.pngwtran libpng.o.pngwutil

ZLIB_OBJS = zlib.o.adler32 zlib.o.compress zlib.o.crc32 zlib.o.deflate \
	zlib.o.gzio zlib.o.infblock zlib.o.infcodes zlib.o.inffast \
	zlib.o.inflate zlib.o.inftrees zlib.o.infutil zlib.o.trees \
	zlib.o.uncompr zlib.o.zutil

PCRE_OBJS = pcre.o.pcre

BZIP2_OBJS = bzip2.o.blocksort bzip2.o.bzlib bzip2.o.compress \
	bzip2.o.crctable bzip2.o.decompress bzip2.o.huffman \
	bzip2.o.randtable

UNZIP_OBJS = unzip.o.ioapi unzip.o.unzip	

.INIT:
	@cdir ^.!Analog
	@cdir o
	@cdir bzip2.o
	@cdir libgd.o
	@cdir libpng.o
	@cdir pcre.o
	@cdir unzip.o
	@cdir zlib.o

all: ^.!Analog.analog

^.!Analog.analog: $(OBJS) $(GD_OBJS) $(PNG_OBJS) $(ZLIB_OBJS) $(PCRE_OBJS) $(BZIP2_OBJS) $(UNZIP_OBJS)
	link -aif -o ^.!Analog.analog $(OBJS) $(GD_OBJS) $(PNG_OBJS) $(ZLIB_OBJS) $(PCRE_OBJS) $(BZIP2_OBJS) $(UNZIP_OBJS) $(LIBS)
	@echo '***'
	@echo '***IMPORTANT: You must read the licence before using analog'
	@echo '***'

.SUFFIXES: .o .c
.c.o:;  $(CC) $(CFLAGS) $(DEPEND) -c $(DEFS) -D$(OS) -o $@ $<

clean:
	-remove ^.!Analog.analog
	-wipe o.* ~v~c

cleanall:
	-remove ^.!Analog.analog
	-wipe o.* ~v~c
	-wipe libgd.o.* ~v~c
	-wipe libpng.o.* ~v~c
	-wipe pcre.o.* ~v~c
	-wipe zlib.o.* ~v~c
	-wipe bzip2.o.* ~v~c
	-wipe unzip.o.* ~v~c

# OK, here's the details on all the DEFS options, in case anyone needs to know.
# NOPIPES excludes non-ANSI functions popen() and pclose(). You will lose the
#   UNCOMPRESS command.
# NODNS excludes all the DNS lookup code. This is platform-dependent, but
#   typically involves header files <net*.h> and functions like inet_addr()
#   and gethostbyaddr().
# NODIRENT stops you using wildcards in logfile names. Again, the code is
#   platform-dependent, but usually the header file <dirent.h> and functions
#   stat(), opendir(), readdir() and closedir() are involved.
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
# NOGRAPHICS omits the code for generating pie charts, including all of libgd,
#   libpng and zlib.
# HAVE_GD means you already have libgd, libpng and zlib, and you want to use
#   your version not mine. This is likely to reduce the size of the binary.
#   You will also need to include "-lgd -lpng -lz" in the LIBS.
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

# Dynamic dependencies:
