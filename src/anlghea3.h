/***             analog 6.0             http://www.analog.cx/             ***/
/*** This program is copyright (c) Stephen R. E. Turner 1995 - 2004 except as
 *** stated otherwise.
 ***
 *** This program is free software. You can redistribute it and/or modify it
 *** under the terms of version 2 of the GNU General Public License, which you
 *** should have received with it.
 ***
 *** This program is distributed in the hope that it will be useful, but
 *** without any warranty, expressed or implied.   ***/

/*** anlghea3.h: third header file ***/

/*** The things in this file are not user-definable; user-definable options
 *** can be found in anlghead.h ***/

#ifndef ANLGHEA3_H
#define ANLGHEA3_H

/*** OS specific things ***/

#define VNUMBER "6.0.17"    /* the version number of this program */

#undef VERSION /* If no OS set, compilation will fail by not knowing VERSION */

#ifdef __MWERKS__
#undef MAC
#define MAC        /* So MWERKS compiler automatically switches to Mac */
#endif

#ifdef __BEOS__
#undef MAC
#undef BEOS
#define BEOS
#endif

#ifdef _WIN32
#ifndef UNIX  /* So as to signal CYGWIN */
#undef WIN32
#define WIN32
#endif
#endif

#ifdef MAC
#define MAC_EVENTS
#define NOPIPES
/* #define MACDIRENT */   /* No longer needed and POSIX dirent is better */
#define NOGLOB
#define NOFOLLOW
#define NOOPEN
#define NOALARM
#define LINE_PARSER
#define USE_ZLIB
#if !TARGET_API_MAC_CARBON
#include <MacHeaders.h>
#endif
#include <sioux.h>
#include <siouxglobals.h>
#define PATHSEPS ":"
#define DIRSEP ':'
#define EXTSEP '.'
#define VERSION VNUMBER"/Mac"
#endif

#ifdef DOS
#define NODNS
#define NOALARM
#define NODIRENT
#define NOFOLLOW
#define PATHSEPS ":\\/"
#define DIRSEP '\\'
#define EXTSEP '.'
#define VERSION VNUMBER"/DOS"
#endif

#ifdef VMS
#define NOPIPES
#define NOALARM
#define VMSDIRENT
#define NOFOLLOW
#ifdef __VMS_VER
#if (__VMS_VER < 70000000)
#define NOGMTIME
#endif
#endif
#define PATHSEPS ":>#]/" /* first four are real VMS, last for Unix emulation */
#define DIRSEP '/'
#define EXTSEP '.'
#include <unixio.h>
/* old DEC C (v4.x?) <stat.h> was missing these two defines */
#ifndef S_IWUSR
#define S_IWUSR 0000200 /* write permission, owner */
#endif
#ifndef S_IRUSR
#define S_IRUSR 0000400 /* read permission, owner */
#endif
#define cma$tis_errno_get_addr CMA$TIS_ERRNO_GET_ADDR
#define FOPENR(s) fopen(s, "r", "shr=upd")
#define VERSION VNUMBER"/OpenVMS"
#endif

#ifdef BEOS
#define NOALARM
#define NOFOLLOW
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#define VERSION VNUMBER"/BeOS"
#endif

#ifdef OSX
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#define VERSION VNUMBER"/OS X"
#endif

#ifdef UNIX
#ifndef OSX
#ifndef BEOS
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#define VERSION VNUMBER"/Unix"
#endif
#endif
#endif

#ifdef WIN32
#define popen _popen
#define pclose _pclose
extern void Win32Init(void);
extern void Win32Cleanup(void);
#define WIN32DIRENT  /* can comment out if POSIX dirent present, eg Borland */
#define NOFOLLOW
#define NOALARM
#define FOPENR_BINARY
#define POPENR_BINARY
#define PATHSEPS ":\\/"
#define DIRSEP '\\'
#define EXTSEP '.'
#define VERSION VNUMBER"/Win32"
#endif

#ifdef OS2
#define NOFOLLOW
#define NOALARM
#define PATHSEPS "\\/"
#define DIRSEP '\\'
#define EXTSEP '.'
#define VERSION VNUMBER"/OS2"
#endif

#ifdef RISCOS
#define NOOPEN
#define NOPIPES
#define RISCOSDIRENT
#define NOFOLLOW
#define NOALARM
#define PATHSEPS "."
#define DIRSEP '.'
#define EXTSEP '/'
#define VERSION VNUMBER MINORVERSION"/RISCOS"
#include <swis.h>
#endif

#ifdef NEXTSTEP
#define NOFOLLOW
#define NOALARM
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#ifndef S_IWUSR
#define S_IWUSR 0000200 /* write permission, owner */
#endif
#ifndef S_IRUSR
#define S_IRUSR 0000400 /* read permission, owner */
#endif
#define VERSION VNUMBER"/NeXTSTEP"
#endif

#ifdef MPEIX
#define NOFOLLOW
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#define VERSION VNUMBER"/iX"
#endif

#ifdef BS2000
#define EBCDIC
#define NOALARM
#define NOFOLLOW
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#define VERSION VNUMBER"/BS2000/OSD"
#endif

#ifdef OS390
#define EBCDIC
#define NOALARM
#define NOFOLLOW
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#define VERSION VNUMBER"/OS390"
#endif

#ifdef AS400
#define EBCDIC
#define PATHSEPS "/"
#define DIRSEP '/'
#define EXTSEP '.'
#define ALIGNSIZE (16)
#define NOPIPES
#define NOALARM
#define NOFOLLOW
#define FOPENR(s) fopen(s, "r,ccsid=37")
#define FOPENW(s) fopen(s, "w,ccsid=37")
#define FOPENWB(s) fopen(s, "wb,ccsid=37")
#define FOPENA(s) fopen(s, "a,ccsid=37")
#define VERSION VNUMBER"/AS400"
#endif

#if defined(EBCDIC) && !defined(OS390) && !defined(AS400) && !defined(BS2000)
/* Seems to be a not-yet-supported platform. */
#error Unsupported EBCDIC platform. Please port Analog and contribute your changes back to <analog-author@lists.meer.net>!
/* You might also want to have a look at:
 *   - globals.c  (system dependent EBCDIC translation tables)
 *   - settings.c (identification string for your OS)
 *   - Makefile   (extra Define)
 */
#endif

/*** ANSI header files ***/

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#ifndef NOGRAPHICS
#include <math.h>
#endif
#ifndef NOALARM
#include <setjmp.h>
#endif

/*** system dependent header files ***/

#ifndef LINE_PARSER
#include "zlib/zlib.h"
#include "unzip/unzip.h"
#include "bzip2/bzlib.h"
#endif

#include "pcre/pcre.h"

#include "libgd/gd.h"
#include "libgd/gdfontf.h"
#include "libgd/gdfonts.h"

#ifndef NOFOLLOW
#include <unistd.h>
#ifndef PATH_MAX
#include <sys/param.h>
#define PATH_MAX MAXPATHLEN
#endif
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef MAC
#ifdef LINE_PARSER
#ifndef REAL_STDIO
#define fopen(name,mode) mac_fopen((name),(mode))
#define fread(ptr,siz,num,file) mac_fread(ptr,siz,num,file)
#undef getc
#define getc(file) mac_getc(file)
#define fclose(file) mac_fclose((file))
#undef feof
#define feof(file) mac_feof(file)
#endif
extern FILE *mac_fopen(const char *name,const char *mode);
extern size_t mac_fread(void *ptr, size_t size, size_t num, FILE *file);
extern int mac_getc(FILE *file);
extern int mac_fclose(FILE *file);
extern int mac_feof(FILE *file);
#endif
#ifndef REAL_ALLOC
#define malloc(size) mac_malloc((size))
#define calloc(num,size) mac_calloc((num),(size))
#define realloc(ptr,size) mac_realloc((ptr),(size))
#define free(ptr) mac_free((ptr))
#endif
extern void *mac_malloc(size_t size);
extern void *mac_calloc(size_t num, size_t size);
extern void *mac_realloc(void *ptr, size_t size);
extern void mac_free(void *ptr);
#endif

#ifdef NEED_FLOATINGPOINT_H
#include <floatingpoint.h>
#endif

#ifndef NOOPEN
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#define OPEN_MODE (S_IREAD | S_IWRITE)
#else
#define OPEN_MODE (S_IRUSR | S_IWUSR)
/* Most systems will accept the above too, but this is POSIXly correct. */
#endif
#endif

/* I assume the header files are well enough behaved that we can include them
   twice without harm, so that we can just do !NOSTAT and !NODIRENT indep. */
/* See init2.c for an explanation of the DIRENT #define logic */
#ifndef NODIRENT
#ifndef VMSDIRENT
#ifndef MACDIRENT
#ifndef WIN32DIRENT
#ifndef RISCOSDIRENT
#ifndef NOGLOB
#include <glob.h>            /* POSIX.2 glob */
#else
#ifdef MAC
#include <stat.h>            /* Mac POSIX dirent */
#include <dirent.h>
#else
#include <sys/types.h>       /* Normal POSIX dirent */
#include <dirent.h>
#include <sys/stat.h>
#endif  /* MAC */
#endif  /* NOGLOB */
#endif  /* RISCOSDIRENT */
#else
#include <io.h>              /* Win32 dirent */
#include <sys/types.h>
#include <sys/stat.h>
#endif  /* WIN3DIRENT */
#else
#include "macdir.h"          /* Old Mac dirent (no longer used) */
#define NOGLOB
#endif  /* MACDIRENT */
#else
#define VMS_FSPEC_MAX 256    /* VMS dirent */
#include <descrip.h>
#endif  /* VMSDIRENT */
#endif  /* NODIRENT */

#ifndef NODNS
#ifdef MAC
#if !TARGET_API_MAC_CARBON
#include <Gestalt.h>         /* Mac DNS */
#include <MacTCP.h>
#include <OpenTransport.h>
#include <OpenTptInternet.h>
#include <AddressXlation.h>
#endif
#else                        /* Win32 and normal DNS */
#ifdef HAVE_ADDR_T   /* define in Makefile if in_addr_t != unsigned long */
#define IN_ADDR_T in_addr_t
#else
typedef unsigned long IN_ADDR_T;
#endif
#ifdef WIN32    /* Win32 DNS: NB windows.h above includes winsock.h */
#define INET_ADDR_ERR ((IN_ADDR_T)(INADDR_NONE))
#else
#ifdef RISCOS
#include <sys/types.h>
#endif
#include <sys/socket.h>      /* Normal DNS */
#include <netinet/in.h>
#include <netdb.h>
#ifndef BEOS
#include <arpa/inet.h>
#else
#ifdef BONE_VERSION
#include <arpa/inet.h>
#endif
#endif  /* BEOS */
#define INET_ADDR_ERR ((IN_ADDR_T)(-1))
#endif  /* !WIN32 */
#endif  /* !MAC */
#ifndef NOALARM              /* All DNS */
#include <unistd.h>
#endif
#ifndef SIGALRM
#define NOALARM
#endif
#endif  /* !NODNS */

#ifdef NODNS
#define NOALARM
#endif
#ifndef SIGALRM
#define NOALARM
#endif
#ifndef NOALARM
#ifdef USE_PLAIN_SETJMP
#define SETJMP setjmp
#define LONGJMP longjmp
#define JMP_BUF jmp_buf
#else
#define SETJMP(b) sigsetjmp((b), 1)
#define LONGJMP siglongjmp
#define JMP_BUF sigjmp_buf
#endif
#endif

/*** User-definable header file ***/

#include "anlghead.h"

/*** definitions ***/

#undef TRUE
#define TRUE (1)
#undef FALSE
#define FALSE (0)
#undef OK
#define OK (0)
#undef ERR
#define ERR (-1)
#undef UNSET
#define UNSET (-1)
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS (0)   /* ANSI, but some broken implementations lack it. */
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE (1)
#endif
#define EXIT_SIGNAL ((EXIT_FAILURE) + 1)
#ifdef WIN32
#define ERRBUFMODE (_IONBF)  /* on Windows, _IOLBF causes full buffering */
#else
#define ERRBUFMODE (_IOLBF)
#endif
#define FEW (2)  /* used for GOTOS and REPORTSPAN */
#define EXCLUDED (2)  /* in my_sort() */
#define NEWLOGFMT (2)
#define BADLOGFMT (3)
#define CONTINUATION (-1)  /* used in warn() in utils.c */
#define IPADDR_INC (3)   /* next six used in (struct include).type */
#define REGEX_INC (2) /* see also IS_REGEXT(), IS_IPADDRT(), IS_INC() below */
#define NORMAL_INC (1)
#define NORMAL_EXC (-1)
#define REGEX_EXC (-2)
#define IPADDR_EXC (-3)
#define BLOCK_EPSILON (30)
#define MIN_SC (100)   /* see parsescode() in input.c */
#define SC_NUMBER (600)
#define IGNORE_CODE (1000)
#define EPSILON (1e-8)
#define MINS_IN_WEEK (10080)
#define HASHSIZE (8)      /* initial size of hash table; a power of 2 */
#define TREEHASHSIZE (3)  /* a power of 2 plus 1 */
#define SIZEBINS (11)     /* in sizescore() */
#define ENGMONTHLEN (3)   /* length of longest of engmonths in globals.c */
#define ENGSHORTDAYLEN (3)/* length of longest of engshortdays in globals.c */
#define ITEMFNS_NUMBER ((ITEM_NUMBER) + 2) /* in inpfns[] in globals.c */
#define ANALOGURL "https://www.c-amie.co.uk/"
#define CAMIERSSURL "https://www.c-amie.co.uk/qlink/?id=103"
#define FIRST_TIME (0)
#define LAST_TIME (ULONG_MAX)
#define LAST_DATE ((ULONG_MAX) / 1440)
#define WHITESPACE ((char)(-1))
#define DOMLEVEL_NUMBER (677)  /* 26^2 + 1 */
#define PMATCH_SIZE (30)  /* This allows PCRE to capture 9 subexpressions */
/* Amount of stuff recorded by cache files: see cache_records in globals.c */
#define V34_DATA_NUMBER (7)
#define V5_DATA_NUMBER (10)
#ifdef MAC_EVENTS
#define MAC_IDLE_FREQ (200)
#endif

/* Special codes for certain reports. We collect them here just to avoid
   changing several instances if they change (although see Pnenxtname() too).
   *
   It would be nice to let the user use localised versions of these in e.g.
   *EXCLUDE commands, but that doesn't scale to the case of reports in two
   languages in one pass, because not all trees are built as late as output
   time. */
#define LNGSTR_UNRESOLVED "[unresolved numerical addresses]"
#define LNGSTR_NODOMAIN "[domain not given]"
#define LNGSTR_UNKDOMAIN "[unknown domain]"
#define LNGSTR_ROOTDIR "[root directory]"
#define LNGSTR_NODIR "[no directory]"
#define LNGSTR_NOEXT "[no extension]"
#define LNGSTR_BRKDIRS "[directories]"
#define LNGSTR_UNKWIN "Unknown Windows"
#define LNGSTR_UNKUX "Other Unix"
#define LNGSTR_ROBOTS "Robots"
#define LNGSTR_UNKOS "OS unknown"

/*** logfile line types ***/

#define LOG_COMMON1 "%S %j %u [%d/%M/%Y:%h:%n:%j] \"%j%w%r%wHTTP%j\" %c %b"
#define LOG_COMMON2 "%S %j %u [%d/%M/%Y:%h:%n:%j] \"%j%w%r\" %c %b"
#define LOG_COMMON3 "%S %j %u [%d/%M/%Y:%h:%n:%j] \"%r\" %c %b"
#define LOG_MS_COMMON1 "%S %j %u [%d/%M/%Y:%h:%n:%j] \"%j%w%r%w\"HTTP%j\" %c %b"
#define LOG_COMBINED1 \
  "%S %j %u [%d/%M/%Y:%h:%n:%j] \"%j%w%r%wHTTP%j\" %c %b \"%f\" \"%B\""
#define LOG_COMBINED2 \
  "%S %j %u [%d/%M/%Y:%h:%n:%j] \"%j%w%r\" %c %b \"%f\" \"%B\""
#define LOG_COMBINED3 \
  "%S %j %u [%d/%M/%Y:%h:%n:%j] \"%r\" %c %b \"%f\" \"%B\""
#define LOG_MS_INT1 "%S, %u, %d/%m/%Z, %h:%n:%j, W3SVC%j, %j, %v, %T, %j, %b, %c, %j, %j, %r, %q,"
#define LOG_MS_INT2 "%*S, %*u, %d/%m/%Z, %h:%n:%j, %j"
#define LOG_MS_NA1 "%S, %u, %m/%d/%Z, %h:%n:%j, W3SVC%j, %j, %v, %T, %j, %b, %c, %j, %j, %r, %q,"
#define LOG_MS_NA2 "%*S, %*u, %m/%d/%Z, %h:%n:%j, %j"
#define LOG_WEBSTAR1 "!!LOG_FORMAT%x2"
#define LOG_WEBSTAR2 "!!%j"
#define LOG_EXTENDED1 "#Fields:%x3"
#define LOG_EXTENDED2 "#%j"
#define LOG_MS_EXTENDED1 "#Fields:%x5"
#define LOG_WEBSTAR_EXTENDED1 "#Fields:%x6"
#define LOG_WEBSITE_NA \
  "%m/%d/%y %h:%n:%j\t%S\t%v\t%j\t%u\t%j\t%r\t%f\t%j\t%B\t%c\t%b\t%T"
#define LOG_WEBSITE_INT \
  "%d/%m/%y %h:%n:%j\t%S\t%v\t%j\t%u\t%j\t%r\t%f\t%j\t%B\t%c\t%b\t%T"
#define LOG_MACHTTP "%m/%d/%y\t%h:%n:%j \t%C%w%S\t%r\t%b"
#define LOG_NETSCAPE "format=%x4"
#define LOG_REFERRER1 "[%d/%M/%Y:%h:%n:%j] %F -> %*r"
#define LOG_REFERRER2 "%F -> %*r"
#define LOG_BROWSER "[%d/%M/%Y:%h:%n:%j] %B"

/*** enums ***/

/* Types of item and other inputs: must have ITEMs in process_data() order:
   code, bytes, proctime, unixtime, am, date in some order (these are
   INPUT_NUMBER, not to be confused with INP_NUMBER): then rest in some order.
   NB I don't know if changing process_data() order would have side-effects. */
/* See also item_type in globals.c */
enum {ITEM_VHOST, ITEM_FILE, ITEM_USER, ITEM_REFERRER, ITEM_BROWSER, ITEM_HOST,
      INP_CODE, INP_BYTES, INP_PROCTIME, INP_UNIXTIME, INP_AM, INP_DATE,
      INP_YEAR, INP_MONTH, INP_HOUR, INP_MIN, INP_QUERY, INP_IP, INP_NUMBER};
#define ITEM_NUMBER (INP_CODE)
#define INPUT_NUMBER (INP_YEAR)  /* NB There is INP_NUMBER and INPUT_NUMBER */

/* Types of report. They must be in the following order: date Reports;
   date Summaries; GENSUM; general reports; dervreps; arrayreps.
   (The order is insignificant within each section.) */
/* See also repcodes, anchorname and repname in globals.c */
/* Also report descriptions files have these entries in this order */
enum {REP_YEAR, REP_QUARTERLY, REP_MONTH, REP_WEEK, REP_DAYREP, REP_HOURREP,
      REP_QUARTERREP, REP_FIVEREP, REP_DAYSUM, REP_HOURSUM, REP_WEEKHOUR,
      REP_QUARTERSUM, REP_FIVESUM, REP_GENSUM, REP_REQ, REP_REDIR, REP_FAIL,
      REP_TYPE, REP_DIR, REP_HOST, REP_REDIRHOST, REP_FAILHOST, REP_DOM,
      REP_REF, REP_REFSITE, REP_REDIRREF, REP_FAILREF, REP_BROWREP, REP_VHOST,
      REP_REDIRVHOST, REP_FAILVHOST, REP_USER, REP_REDIRUSER, REP_FAILUSER,
      REP_ORG, REP_SEARCHREP, REP_SEARCHSUM, REP_INTSEARCHREP,
      REP_INTSEARCHSUM, REP_BROWSUM, REP_OS, REP_SIZE, REP_CODE, REP_PROCTIME,
      REP_NUMBER};
#define DATEREPORTS_NUMBER (REP_DAYSUM)   /* just the eight "Reports" */
#define DATEREP_NUMBER (REP_GENSUM)   /* including the five "Summaries" too */
#define FIRST_GENREP (REP_GENSUM + 1)
#define GENREP_NUMBER (REP_NUMBER - FIRST_GENREP)
#define FIRST_DERVREP (REP_SEARCHREP)
#define DERVREP_NUMBER (6)
#define FIRST_ARRAYREP (REP_SIZE)
#define ARRAYREP_NUMBER (3)
#define LAST_NORMALREP (REP_OS)

/* Types of logfile line. Order insignificant. */
enum {SUCCESS, FAILURE, REDIRECT, INFO, UNWANTED};
#define OUTCOME_NUMBER (INFO)  /* only first 3 needed for datacols[] */

/* Counts then dates collected about each item, and then other floor/sort
   criteria: order otherwise insignificant. See also methodname in globals.c
   and cache_records in cache.c. */
enum {REQUESTS, REQUESTS7, PAGES, PAGES7, REDIR, REDIR7, FAIL, FAIL7, SUCCDATE,
      REDIRDATE, FAILDATE, SUCCFIRSTD, REDIRFIRSTD, FAILFIRSTD, BYTES, BYTES7,
      ALPHABETICAL, RANDOM, METHOD_NUMBER};
#define COUNT_NUMBER (SUCCDATE)
#define DATESORT (SUCCDATE)
#define FIRSTDATE (SUCCFIRSTD)
#define DATA_NUMBER (BYTES)
#define CHART_NONE (SUCCDATE)
#define CHART_SORTBY (CHART_NONE + 1)

/* abbreviated version ignoring HTTP status code */
enum {REQUESTS2, REQUESTS72, PAGES2, PAGES72, DATE2, FIRSTD2, DATA2_NUMBER,
      DATACOLS_NUMBER};

/* Type of logfile by compression */
enum {LF_NOTOPENED, LF_NORMAL, LF_PIPE, LF_GZIP, LF_ZIP, LF_ZIPMEMBER, LF_BZ2};

/* Data collected about total requests etc. in each logfile */
enum {LOGDATA_UNWANTED, LOGDATA_UNKNOWN, LOGDATA_UNKNOWN7, LOGDATA_INFO,
      LOGDATA_INFO7, LOGDATA_SUCC, LOGDATA_SUCC7, LOGDATA_PAGES,
      LOGDATA_PAGES7, LOGDATA_REDIR, LOGDATA_REDIR7, LOGDATA_FAIL,
      LOGDATA_FAIL7, LOGDATA_CORRUPT, LOGDATA_NUMBER};

/* Poss. columns: COL_TITLE must be first; see also colcodes[] in globals.c */
enum {COL_TITLE, COL_REQS, COL_REQS7, COL_PAGES, COL_PAGES7, COL_BYTES,
      COL_BYTES7, COL_PREQS, COL_PREQS7, COL_PPAGES, COL_PPAGES7, COL_PBYTES,
      COL_PBYTES7, COL_DATE, COL_TIME, COL_FIRSTD, COL_FIRSTT, COL_INDEX,
      COL_NUMBER};

/* Levels of DNS lookup. Keep in order. */
enum {DNS_NONE, DNS_READ, DNS_LOOKUP, DNS_WRITE};

/* Output types: start with XHTML then PLAIN */
enum {XHTML, PLAIN, ASCII, HTML, LATEX, COMPUTER, XML, OUT_NONE};

/* days of the week */
enum {SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY};

/* languages; see also country in globals.c */
enum {ARMENIAN, BASQUE, /*BOSNIAN,*/ BULGARIAN, BULGARIAN_MIK, CATALAN,
      SIMP_CHINESE, TRAD_CHINESE, /*CROATIAN,*/ CZECH, CZECH_1250, DANISH,
      DUTCH, ENGLISH, US_ENGLISH, FINNISH, FRENCH, GERMAN, /*GREEK,*/
      HUNGARIAN, /*ICELANDIC,*/ INDONESIAN, ITALIAN, JAPANESE_EUC,
      JAPANESE_JIS,
      JAPANESE_SJIS, JAPANESE_UTF, KOREAN, LATVIAN, /*LITHUANIAN,*/
      NORWEGIAN, NYNORSK, POLISH, PORTUGUESE, BR_PORTUGUESE, /*ROMANIAN,*/
      RUSSIAN, RUSSIAN_1251, SERBIAN, SLOVAK, SLOVAK_1250, SLOVENE,
      SLOVENE_1250, SPANISH, SWEDISH, SWEDISH_ALT, TURKISH, UKRAINIAN};
#define OLDLANG (-2)

/* source of data passed to htmlputs(): see comments there. These represent
   roughly increasing levels of security. */
enum {AS_IS, TRUSTED, FROM_CFG, UNTRUSTED, IN_HREF};

/* lngstrs; charset_ must be first */
enum {charset_, weekbeg_, year_, quarteryr_, month_, day_, days_, hr_, minute_,
      minutes_, second_, seconds_, byte_, bytes_, xbytes_, request_, requests_,
      date_, datetime_, time_, firstdate_, firsttime_, lastdate_, lasttime_,
      file_, files_, host_, hosts_, vhost_, vhosts_, dir_, dirs_, dom_, doms_,
      org_, orgs_, ext_, exts_, url_, urls_, browser_, browsers_, os_, oss_,
      size_, search_, searches_, site_, sites_, user_, users_, code_, codes_,
      webstatsfor_, kilo_, mega_, giga_, tera_, peta_, exa_, zeta_, yotta_,
      kiloabbr_, megaabbr_, gigaabbr_, teraabbr_, petaabbr_, exaabbr_,
      zetaabbr_, yottaabbr_, filesize0_, filesize1_, filesize2_, filesize3_,
      filesize4_, filesize5_, filesize6_, filesize7_, filesize8_, filesize9_,
      filesize10_, gensum_, yearrep_, busyyear_, quarterlyrep_,
      busyquarteryr_, monthrep_, busymonth_, weekrep_, busyweek_, daysum_,
      dayrep_, busyday_, hourrep_, hoursum_, weekhoursum_, busyhour_,
      quarterrep_, quartersum_, busyquarterhr_, fiverep_, fivesum_, busyfive_,
      hostrep_, hostgs_, hostgp_, hostgen_, redirhostrep_, redirhostgs_,
      redirhostgp_, redirhostgen_, failhostrep_, failhostgs_, failhostgp_,
      failhostgen_, dirrep_, dirgs_, dirgp_, dirgen_, typerep_, extgs_, extgp_,
      extgen_, reqrep_, filegs_, filegp_, filegen_, redirrep_, redirgs_,
      redirgp_, redirgen_, failrep_, failgs_, failgp_, failgen_, refrep_,
      refgs_, refgp_, refgen_, refsiterep_, refsitegs_, refsitegp_,
      refsitegen_, redirrefrep_, redirrefgs_, redirrefgp_, redirrefgen_,
      failrefrep_, failrefgs_, failrefgp_, failrefgen_, searchrep_,
      searchrepgs_, searchrepgp_, searchrepgen_, searchsum_, searchsumgs_,
      searchsumgp_, searchsumgen_, intsearchrep_, intsearchrepgs_,
      intsearchrepgp_, intsearchrepgen_, intsearchsum_, intsearchsumgs_,
      intsearchsumgp_, intsearchsumgen_, vhostrep_, vhostgs_, vhostgp_,
      vhostgen_, redirvhostrep_, redirvhostgs_, redirvhostgp_, redirvhostgen_,
      failvhostrep_, failvhostgs_, failvhostgp_, failvhostgen_, userrep_,
      usergs_, usergp_, usergen_, rediruserrep_, redirusergs_, redirusergp_,
      redirusergen_, failuserrep_, failusergs_, failusergp_, failusergen_,
      browsum_, browgs_, browgp_, browgen_, browrep_, browrepgs_, browrepgp_,
      browrepgen_, osrep_, osgs_, osgp_, osgen_, domrep_, domgs_, domgp_,
      domgen_, orgrep_, orggs_, orggp_, orggen_, statrep_, codegs_, codegp_,
      codegen_, proctimerep_, sizerep_, credit_, runtime_, lessone_, eachunit_,
      represents_, partof_, pagereq_, pagereqs_, notlistedm_, notlistedf_,
      notlistedn_, otherm_, otherf_, othern_, progstart_, reqstart_, to_,
      succreqs_, avereqs_, totpages_, avepages_, totunknown_, totfails_,
      totredirs_, inforeqs_, distfiles_, disthosts_, corrupt_, unwanted_,
      totdata_, avedata_, brackets_, sevendaysto_, lastsevendays_, goto_, top_,
      unresolved_, nodomain_, unkdomain_, rootdir_, nodir_, noext_, brkdirs_,
      unkwin_, unkux_, robots_, unkos_, nreqs_, nreqs7_, preqs_,
      preqs7_, npgs_, npgs7_, ppgs_, ppgs7_, nbytes_, nxbytes_, nbytes7_,
      nxbytes7_, pbytes_, pbytes7_, index_, firstsm_, firstsf_, firstsn_,
      firstdsm_, firstdsf_, firstdsn_, allsm_, allsf_, allsn_, floorby_,
      atleast_, request7_, requests7_, pagereq7_, pagereqs7_, redirreq_,
      redirreqs_, redirreq7_, redirreqs7_, failreq_, failreqs_, failreq7_,
      failreqs7_, ptraffic_, ptraffic7_, pmtraffic_, pmtraffic7_, bytetraffic_,
      bytestraffic_, xbytestraffic_, bytetraffic7_, bytestraffic7_,
      xbytestraffic7_, sincedate_, sinceredirdate_, sincefaildate_,
      sincefirstd_, sinceredirfirstd_, sincefailfirstd_, sortedm_, sortedf_,
      sortedn_, repspan_, chartby_, traffic_, traffic7_,
      prequests_, prequests7_, pmrequests_, pmrequests7_, nrequests_,
      nrequests7_, ppages_, ppages7_, pmpages_, pmpages7_, npages_, npages7_,
      predirs_, predirs7_, pmredirs_, pmredirs7_, nredirs_, nredirs7_, pfails_,
      pfails7_, pmfails_, pmfails7_, nfails_, nfails7_, succdate_, redirdate_,
      faildate_, succfirstd_, redirfirstd_, failfirstd_, alphasortm_,
      alphasortf_, alphasortn_, numsortm_, numsortf_, numsortn_, unsortedm_,
      unsortedf_, unsortedn_, sepchar_, decpoint_, colon_, am_, pm_, datefmt1_,
      datefmt2_, dayrepfmt_, daysumfmt_, hourrepfmt_, hoursumfmt_,
      weekhoursumfmt_, quarterrepfmt_, quartersumfmt_, weekfmt_, monthfmt_,
      quarterlyfmt_, yearfmt_, genrepdate_, genreptime_,
      whatincfmt_, code100_, code101_, code199_, code200_, code201_, code202_,
      code203_, code204_, code205_, code206_, code299_, code300_, code301_,
      code302_, code303_, code304_, code_305, code306_, code307_, code399_,
      code400_, code401_, code402_, code403_, code404_, code405_, code406_,
      code407_, code408_, code409_, code410_, code411_, code412_, code413_,
      code414_, code415_, code416_, code417_, code499_, code500_, code501_,
      code502_, code503_, code504_, code505_, code506_, code599_, code999_,
      LNGSTR_NUMBER};
#define byteprefix_ (kilo_ - 1)
#define byteprefixabbr_ (kiloabbr_ - 1)

/* return codes from strtoinfmt() */
/* see also configlogfmt() in init2.c */
enum {FMT_OK, FMT_NOPC, FMT_DUP, FMT_BADCHAR, FMT_NOTERM, FMT_BADBUILTIN,
      FMT_BADPC, FMT_PARTTIME, FMT_QBUTNOR};

/*** typedefs ***/

typedef unsigned char logical;
typedef signed char choice;
typedef unsigned int datecode_t;
typedef unsigned long timecode_t;

/*** typedef function pointers ***/

struct hashindex;
struct logfile;

typedef void (*cutfnp)(char **, char **, char *, logical);
typedef void (*dcutfnp)(char **, char **, char *, void *);
typedef struct hashindex *(*mergefnp)(struct hashindex *, struct hashindex *,
				      unsigned long, unsigned long, choice);
typedef void (*conffnp)(void *, char *, char *, char *, int);
typedef choice (*inputfnp)(struct logfile *lf, void *, char);

/*** structs */

typedef struct {
  double bytes, bytes7;
  unsigned long *data;  /* some are really timecode_t */
  logical reused;  /* for reused, see newtreedata() in tree.c */
  choice ispage;
} Hashentry;

typedef struct hashindex {
  char *name;
  Hashentry *own;
  void *other;
  struct hashindex *next;
} Hashindex;

typedef struct aliasto {
  char *string;
  choice after;
  struct aliasto *next;
} AliasTo;
/* The target of an alias alternates a "string" and possibly a part of the
   original name indexed by "after" (else after == -1) (see printalias()). */

typedef struct alias {
  char *from;
  pcre *pattern;
  AliasTo *to;
  logical isregex;
  struct alias *next;
} Alias;
/* For a normal pattern, nmatch is no. of *'s. For a regular expression, it's
   -3 * (no. of capturing subexpressions): this is what pcre_exec needs. */

typedef struct strpairlist {
  char *name, *data;
  struct strpairlist *next;
} Strpairlist;

typedef struct {
  char *name;
  conffnp fn;
  void *opt;
} Configfns;

typedef struct {
  char *name;
  choice arg;
} Choices;

typedef struct {
  char code;
  choice type;
  inputfnp fn;
  void *opt;
} Inputfns;

typedef struct inputformat {
  Inputfns *inpfns;
  char sep;
  struct inputformat *next;
} Inputformat;

typedef struct inputformatlist {
  Inputformat *form;
  choice count[INPUT_NUMBER]; /* 0 = not present, 1 = starred, 2 = unstarred */
  logical used;
  struct inputformatlist *next;
} Inputformatlist;

typedef struct logfile {
  char *name;
  void *file;
  void *file2;  /* bzip2 needs the (FILE *) and the (BZFILE *) */
  choice type;
  Inputformatlist *format;
  timecode_t from, to;
  unsigned long data[LOGDATA_NUMBER];
  double bytes, bytes7;
  char *prefix;
  size_t prefixlen;
  int pvpos, tz;
  struct logfile *next;
} Logfile;

typedef struct mmlist {
  void *pos;
  struct mmlist *next;
} Mmlist;

typedef struct {
  void *block_end, *curr_pos, *next_pos;
  Mmlist *first, *last;
  unsigned short alignment;
} Memman;

typedef struct daysdata {
  unsigned long *reqs, *pages;
  double *bytes;
  struct daysdata *prev, *next;
} Daysdata;

typedef struct {
  char *fromstr, *tostr;
  timecode_t from, to, last7from, last7to, firsttime, lasttime;
  datecode_t firstdate, lastdate, currdate;
  /* perhaps bad name: currdate is date of current logfile entry */
  Daysdata *firstdp, *lastdp, *currdp;
} Dateman;

typedef struct timerep {
  unsigned long reqs, pages;
  double bytes;
  datecode_t date;
  unsigned int time;
  struct timerep *prev, *next;
} Timerep;

typedef struct {
  double threshold;
  unsigned long reqs, reqs7, pages, pages7;
  double bytes, bytes7;
  timecode_t firstdate, lastdate;
} Arraydata;

typedef struct {
  Hashindex **head;
  unsigned long size;
  unsigned long n;
} Hashtable;

typedef struct {
  Hashtable *tree;
  cutfnp cutfn;
  Memman *space;
} Tree;

typedef struct {
  Hashtable *table;
  dcutfnp cutfn;
  Memman *space;
  void *arg;
} Derv;

typedef struct strlist {
  char *name;
  struct strlist *next;
} Strlist;

typedef struct {
  char *name;
  double angle;
} Wedge;

typedef struct include {
  char *name;
  pcre *pattern;
  unsigned long minaddr, maxaddr;
  choice type;
  struct include *next;
} Include;

typedef struct {
  double min;
  char qual;
  choice floorby;
} Floor;

typedef struct {
  choice code;
  char *file;
} Lang;

struct outchoices;

/* The actual outputters are defined in globals.c */
typedef struct {
  unsigned int (*pagewidth)(struct outchoices *);
  void (*cgihead)(FILE *, struct outchoices *);
  void (*stylehead)(FILE *, struct outchoices *);
  void (*pagetitle)(FILE *, struct outchoices *);
  void (*timings)(FILE *, struct outchoices *, Dateman *);
  void (*closehead)(FILE *, struct outchoices *);
  void (*pagebotstart)(FILE *, struct outchoices *);
  void (*credit)(FILE *, struct outchoices *);
  void (*runtime)(FILE *, struct outchoices *, long);
  void (*pagefoot)(FILE *, struct outchoices *);
  void (*stylefoot)(FILE *, struct outchoices *);
  void (*reporttitle)(FILE *, struct outchoices *, choice);
  void (*reportfooter)(FILE *, struct outchoices *, choice);
  void (*reportdesc)(FILE *, struct outchoices *, choice);
  void (*reportspan)(FILE *, struct outchoices *, choice, timecode_t,
		     timecode_t);
  void (*gensumhead)(FILE *, struct outchoices *);
  void (*gensumfoot)(FILE *, struct outchoices *);
  void (*gensumline)(FILE *, struct outchoices *, int, unsigned long,
		     unsigned long, logical);
  void (*gensumlineb)(FILE *, struct outchoices *, int, double, double,
		      logical);
  void (*lastseven)(FILE *, struct outchoices *, timecode_t);
  void (*prestart)(FILE *, struct outchoices *);
  void (*preend)(FILE *, struct outchoices *);
  void (*hrule)(FILE *, struct outchoices *);
  char *(*endash)(void);
  void (*putch)(FILE *, char);
  size_t (*strlength)(const char *);
  logical (*allowmonth)(void);
  void (*calcwidths)(struct outchoices *, choice, unsigned int [],
		     unsigned int *, unsigned int *, double *, unsigned long,
		     unsigned long, unsigned long, unsigned long, double,
		     double, unsigned long);
  void (*declareunit)(FILE *, struct outchoices *, char, double, unsigned int);
  void (*colheadstart)(FILE *, struct outchoices *, choice);
  void (*colheadcol)(FILE *, struct outchoices *, choice, choice, unsigned int,
		     char *, logical);
  void (*colheadend)(FILE *, struct outchoices *, choice);
  void (*colheadustart)(FILE *, struct outchoices *, choice);
  void (*colheadunderline)(FILE *, struct outchoices *, choice,
			   choice, unsigned int, char *);
  void (*colheaduend)(FILE *, struct outchoices *, choice);
  void (*rowstart)(FILE *, struct outchoices *, choice, choice *, int, char *,
		   char *, char *);
  void (*levelcell)(FILE *, struct outchoices *, choice, int);
  void (*namecell)(FILE *, struct outchoices *, choice, char *, choice,
		   unsigned int, logical, logical, logical, Alias *, Include *,
		   logical, unsigned int, char *);
  void (*ulcell)(FILE *, struct outchoices *, choice, choice, unsigned long,
		 unsigned int);
  void (*strcell)(FILE *, struct outchoices *, choice, choice, char *,
		  unsigned int);
  void (*bytescell)(FILE *, struct outchoices *, choice, choice, double,
		    double, unsigned int);
  void (*pccell)(FILE *, struct outchoices *, choice, choice, double, double,
		 unsigned int);
  void (*indexcell)(FILE *, struct outchoices *, choice, choice, long,
		    unsigned int);
  void (*rowend)(FILE *, struct outchoices *, choice);
  void (*blankline)(FILE *, struct outchoices *, choice *);
  void (*barchart)(FILE *, struct outchoices *, int, char);
  void (*busyprintf)(FILE *, struct outchoices *, choice, char *,
		     unsigned long, unsigned long, double, datecode_t,
		     unsigned int, unsigned int, datecode_t, unsigned int,
		     unsigned int, char);
  void (*notlistedstr)(FILE *, struct outchoices *, choice, unsigned long);
  void (*whatincluded)(FILE *, struct outchoices *, choice, unsigned long,
		       Dateman *);
  void (*whatinchead)(FILE *, struct outchoices *);
  void (*whatincfoot)(FILE *, struct outchoices *);
  void (*whatincprintstr)(FILE *, struct outchoices *, char *);
  void (*printdouble)(FILE *, struct outchoices *, double);
  void (*includefile)(FILE *, struct outchoices *, char *, char);
  unsigned int (*riscosfiletype)(void);
} Outputter;

typedef struct outchoices {
  Outputter *outputter;
  char *outfile, *cacheoutfile;
  char *domainsfile, *descfile, *headerfile, *footerfile;
  choice outstyle, gotos;
  logical descriptions, multibyte, html, repspan, pdflatex;
  unsigned int rsthresh;
  choice reporder[REP_NUMBER + 1];
  logical repq[REP_NUMBER], last7, runtime;
  choice cols[REP_NUMBER][COL_NUMBER];
  unsigned int rows[DATEREPORTS_NUMBER];
  char graph[DATEREP_NUMBER];
  logical back[DATEREPORTS_NUMBER];
  choice chartby[GENREP_NUMBER];
  logical jpegcharts, pngimages;
  choice sortby[GENREP_NUMBER], subsortby[GENREP_NUMBER];
  Floor floor[GENREP_NUMBER], subfloor[GENREP_NUMBER];
  Lang lang;
  char *dayname[7], *monthname[12], **lngstr, *descstr[REP_NUMBER];
  size_t daylen, plaindaylen, monthlen, plainmonthlen, ampmlen, plainampmlen;
  char *hostname, *hosturl, *logo, *logourl, *imagedir;
  char *stylesheet,*cssprefix, *chartdir, *localchartdir;
  char *xmldtd;
  char markchar;
  logical norobots;
  logical linknofollow;
  logical rawbytes;
  unsigned int bytesdp;
  choice weekbeginson;
  unsigned int plainpagewidth, htmlpagewidth, latexpagewidth;
  unsigned int mingraphwidth;
  char sepchar, repsepchar, decpt, barstyle;
  char *compsep, *gensumlines;
  Tree *tree[GENREP_NUMBER];
  Strlist *suborgs;
  Derv *derv[DERVREP_NUMBER];
  choice *alltrees, *alldervs;
  Include *wanthead[GENREP_NUMBER];
  Alias *aliashead[GENREP_NUMBER];
  Include *link[GENREP_NUMBER];
  Strlist *expandhead[GENREP_NUMBER];
  char *baseurl;
  char *anonymizerurl;
  choice searchconv;
  unsigned char convfloor;
} Outchoices;

typedef struct {
  Logfile *logfile[2];  /* [0] are logfiles, [1] are cache files */
  unsigned long data[LOGDATA_NUMBER];
  double bytes, bytes7;
  char *dirsuffix;
  unsigned int dirsufflength;
  choice lowmem[ITEM_NUMBER];
  logical case_insensitive, usercase_insensitive;
  unsigned int granularity;
} Miscoptions;

typedef struct {
  Hashtable *hash[ITEM_NUMBER];
  Arraydata *arraydata[ARRAYREP_NUMBER];
  choice datacols[ITEM_NUMBER][OUTCOME_NUMBER][DATACOLS_NUMBER][2];
  /* given item type i, outcome j, the kth thing to do is to store data of
     data2-type dc[i][j][k][1] in structure's column index dc[i][j][k][0]. */
  choice data2cols[ITEM_NUMBER][DATA_NUMBER];
  /* item type i, data type j, is stored in this column (or -1 if unwanted) */
  unsigned int no_cols[ITEM_NUMBER];
  Hashindex **gooditems, **baditems;
  Include *wanthead[ITEM_NUMBER], *ispagehead, *argshead, *refargshead;
  choice code2type[SC_NUMBER];
  logical succ304;
  Alias *aliashead[ITEM_NUMBER];
  Strpairlist *searchengines, *intsearchengines;
  Include *robots;
  Dateman dman;
  Outchoices outopts;
  Miscoptions miscopts;
  Strlist *conffilelist;
} Options;

/*** macros ***/

#undef MAX
#define MAX(a, b) (((a) > (b))?(a):(b))
#undef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
/* a whole set of macros to cope with to...() and is...() commands if
   char is unsigned. */
#define ISALNUM(c) isalnum((unsigned char)(c))
#define ISALPHA(c) isalpha((unsigned char)(c))
#define ISDIGIT(c) isdigit((unsigned char)(c))
#define ISLOWER(c) islower((unsigned char)(c))
#define ISSPACE(c) isspace((unsigned char)(c))
#define ISUPPER(c) isupper((unsigned char)(c))
#define TOLOWER(c) ((char)tolower((unsigned char)(c)))
#define TOUPPER(c) ((char)toupper((unsigned char)(c)))
#ifdef NEED_STRCMP
#define strcmp my_strcmp
#endif
#ifdef NEED_DIFFTIME
#define difftime(t, s) ((double)((t) - (s)))
#endif
#ifdef NEED_STRTOUL
#define strtoul(a, b, c) ((unsigned long)(strtol(a, b, c)))
/* This is not quite right because of the different ranges of long and
   unsigned long, but it's probably OK for our purposes */
#endif
#define STREQ(s, t) (strcmp(s, t) == 0)
#define MATCHES(s, p) (matchq(s, (void *)p, FALSE, NULL))
/* Date conversion functions. See dates.c on the representation of dates. */
#define FEB (1)
#define DEC (11)
#define IS_LEAPYEAR(y) ((y) % 4 == 0) /* This is not Year 2100 compatible but
					 is Year 2000 compatible! */
#define DATE2CODE(y, m, d) (((y) - 1970) * 365 + ((y) - 1968) / 4 + \
		 daysbefore[m] + (d) - (IS_LEAPYEAR(y) && (m) <= FEB))
#define TIMECODE(datecode, hr, min) ((timecode_t)((datecode) * 1440 + (datecode_t)((hr) * 60 + (min))))
#define UXTIME2CODE(t) ((timecode_t)(((t) / 60) + 1440))
#define DAYOFWEEK(d) ((choice)(((d) + 10) % 7))
#define IS_EMPTY_STRING(s) ((s)[0] == '\0')
#define IS_STDIN(s) (STREQ((s), "-") || strcaseeq((s), "stdin"))
#define IS_STDOUT(s) (STREQ((s), "-") || strcaseeq((s), "stdout"))
#define TO_NEXT(x) (x) = (x)->next
#ifndef FOPENR
#ifdef FOPENR_BINARY
#define FOPENR(s) fopen(s, "rb")
#else
#define FOPENR(s) fopen(s, "r")
#endif
#endif
#ifndef FOPENW
#define FOPENW(s) fopen(s, "w")
#endif
#ifndef FOPENWB
#define FOPENWB(s) fopen(s, "wb")
#endif
#ifndef FOPENA
#define FOPENA(s) fopen(s, "a")
#endif
#ifndef POPENR
#ifdef POPENR_BINARY
#define POPENR(s) popen(s, "rb")
#else
#define POPENR(s) popen(s, "r")
#endif
#endif
#define ENTRY_WANTED(e) ((e) != unwanted_entry)
#define ENTRY_BLANK(e) ((e) == blank_entry)
#define LEN3(n, c) ((unsigned int)((c == '\0')?(n):((n) + (((n) - 1) / 3))))
/* The length of an n digit number with seps; e.g. 56789 [5] -> 56,789 [6] */
#define COPYSTR(s, n) {char *z = n; s = (char *)xmalloc(strlen(z) + 1); \
                       strcpy(s, z);}  /* z needed so n only evaluated once */
/* Note memory leak in COPYSTR, so only used in initialisation */
#define TOO_FULL(n, z) ((n) > (z))   /* defn for hash table too full */
#define TOO_FULL_TREE TOO_FULL
#define NEW_SIZE(z) (2 * (z))        /* new size of the hash table */
#define NEW_SIZE_TREE(z) (2 * (z) - 1)
/* Next three: see IPADDR_INC etc. above */
#define IS_IPADDRT(type) ((type) == 3 || (type) == -3)  /* for includes */
#define IS_REGEXT(type) ((type) == 2 || (type) == -2)  /* for includes */
#define IS_INC(type) ((type) > 0)     /* for includes */
/* static char *s = NULL; static size_t len = 0; ENSURE_LEN(s, len, req_len) */
#define ENSURE_LEN(s, l, n) if ((n) > (l)) { \
                              s = (char *)xrealloc((void *)s, n); \
			      l = n;}
/* These PUT's are used in datesprintf() etc.: quicker than using sprintf() */
#define PUT02d(s, d) {*((s)++) = ((d) / 10) + '0'; \
			*((s)++) = ((d) % 10) + '0'; }
#define PUT04d(s, d) {*((s)++) = ((d) / 1000) + '0'; \
			*((s)++) = ((d) / 100) % 10 + '0'; \
			*((s)++) = ((d) / 10) % 10 + '0'; \
			*((s)++) = ((d) % 10) + '0'; }
#define PUT2d(s, d) {*((s)++) = ((d) < 10)?' ':(((d) / 10) + '0'); \
		       *((s)++) = ((d) % 10) + '0'; }
#define PUT1d(s, d) {*((s)++) = (d) + '0'; }
#define PUTc(s, c) {*((s)++) = (c); }
#define PUTs(s, t, n) {int z; size_t l = strlen(t); \
			  memcpy((void *)s, (void *)t, l); s += l; \
                          for (z = n; z > 0; z--) *((s)++) = ' '; }
/* Some simple command line processing parts implemented as macros */
#define CLSHORTWARN(arg) warn('C', TRUE, "No argument specified after %s command line option (or space wrongly left before argument)", arg)
#define CLLONGWARN(arg) warn('C', TRUE, "Command line argument %s too long: ignoring end of it", arg)
#define CLSHORTCHECK(command) if (argv[i][2] == '\0') CLSHORTWARN(argv[i]); \
                              else command
#define CLLONGCHECK(command) if (argv[i][2] != '\0') CLLONGWARN(argv[i]); \
                            command
#define CLTOGGLE(f) CLLONGCHECK(f = (argv[i][0] == '+')?TRUE:FALSE)
#define CLREPTOGGLE(n) CLTOGGLE(op->outopts.repq[n])
#define CLOUTSTYLE CLTOGGLE
#define CLGOTOS CLTOGGLE
/* these work because PLAIN == TRUE == ON and XHTML == FALSE == OFF */
/* an abbreviation for arrays with size GENREP_NUMBER */
#define G(r) ((r) - FIRST_GENREP)
/* magicno as macro not fn. for speed: x is unsigned long to contain answer,
   s is (char *) to be magicked,
   b is unsigned long base (no. of hash bins); power of 2 in practice.
   Formula is (\sum_i 9^i c_i) [mod (ULONG_MAX + 1)] mod b */
#define MAGICNO(x, s, b) {register unsigned char *r; \
		    for(x = 0, r = (unsigned char *)(s); *r != '\0'; r++) { \
		    x = (x << 3) + x + (long)(*r); } \
		    x %= b; }
/* for tree magic number, t is end of string, bins are power of 2 plus 1,
   and formula has 33 in place of 9 */
#define MAGICNOTREE(x, s, t, b) {register unsigned char *r; \
		    for(x = 0, r = (unsigned char *)(s); \
			r < (unsigned char *)(t); r++) { \
		    x = (x << 5) + x + (long)(*r); } \
		    x %= (b - 1); x++; }

#include "anlghea4.h"  /* import function declarations */

#endif  /* ANLGHEA3_H */
