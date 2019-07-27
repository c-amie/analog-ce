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

/*** anlghea2.h: second header file ***/

/*** This file is a convenient place to store defaults for various options.
 *** You can redefine them if you want, but you probably don't want to because
 *** they have sensible defaults and it's easier to change them at run-time.
 *** See anlghead.h for the options you are likely to want to change. ***/

#ifndef ANLGHEA2_H
#define ANLGHEA2_H

#ifndef ANLGHEAD
#include "anlghead.h"
#endif

/*** The first few options are compiled into the program. They can't be
     changed without re-compiling the program. ***/

#define BLOCKSIZE (32768)
/* Analog allocates memory in blocks of this many bytes. Also any logfile
   lines this long or longer will be rejected. */

#define MAX_CONFIGS (50)
/* The maximum number of configuration files that are allowed. */

#define MAXPAGEWIDTH (240)
/* the maximum allowed value of the PAGEWIDTH variable */

#ifndef ALIGNSIZE
#define ALIGNSIZE (8)
/* All objects can align on boundaries that are multiples of this. This
   is system-dependent, but I expect 8 to be big enough for almost everyone. */
#endif

/*** The rest of the options can be overridden when the program is run. ***/

#ifndef CACHEOUTFILE
#define CACHEOUTFILE "none"
/* The default file for cache output. Use "none" for no cache file. */
#endif

#ifndef CACHEFILE
#define CACHEFILE "none"
/* The file for cache input. Again, can be "none". */
#endif

#ifndef NODNS
#define DNSFILE "dnscache"
/* where to store resolved addresses so we don't have to look them up next
   time */

#define DNSLOCKFILE "dnslock"
/* A lock file to stop two processes trying to write the DNSFILE at once */ 

#define DNS (DNS_NONE)
/* level of DNS activity. Can be DNS_NONE, DNS_READ, DNS_LOOKUP or
   DNS_WRITE. */

#define DNSGOODHOURS (100000)
/* How many hours lookups stay fresh for before we have to look them
   up again. 100000 = 11.4 years! */

#define DNSBADHOURS (336)
/* After how many hours to retry failed lookups. 336 = two weeks. */

#define DNSTIMEOUT (0)
/* How many seconds to wait for the DNS server to reply before giving up. This
   is only used on some platforms. It is set to 0 by default, which means off,
   because it breaks DNS lookups on many platforms. */
#endif

#define DIRSUFFIX "index.html"
/* the default filename tried if a directory is requested. The program
   combines statistics from /dir/ and /dir/DIRSUFFIX. If you don't wish
   this combining to occur, define DIRSUFFIX to be "". The usual value
   is "index.html". */

#define CASE_INSENSITIVE (TRUE)
#ifdef UNIX
#undef CASE_INSENSITIVE
#define CASE_INSENSITIVE (FALSE)
#endif
#ifdef OSX
#undef CASE_INSENSITIVE
#define CASE_INSENSITIVE (FALSE)
#endif
#ifdef BEOS
#undef CASE_INSENSITIVE
#define CASE_INSENSITIVE (FALSE)
#endif
#ifdef NEXTSTEP
#undef CASE_INSENSITIVE
#define CASE_INSENSITIVE (FALSE)
#endif
/* Do you have a case sensitive or case insensitive file system? */

#define USERCASE_INSENSITIVE (TRUE)
/* The same for user names */

#define SEARCHCHARCONVERT (UNSET)
/* Whether to convert %nm's from the top half of the character set in the
   search reports. UNSET means yes for single-byte character sets, no for
   multibyte character sets. */

#define REPORTORDER "x1QmWDdHwh4657oZSlLujJkKfsNnBbpvRMcPztiEIYyr"
/* A string like "x1QmWDdHwh4657oZSlLujJkKfsNnBbpvRMcPztiEIYyr" giving the
   order in which the reports will be output. You should include each of the
   above letters exactly once. See docs/output.html for which report has which
   code letter.*/

/* Whether we want each of the reports by default */
#define GENERAL (TRUE)      /* General Summary */
#define YEARLY (FALSE)      /* Yearly Report */
#define QUARTERLY (FALSE)   /* Quarterly Report */
#define MONTHLY (TRUE)      /* Monthly Report */
#define DAILYSUM (TRUE)     /* Daily Summary */
#define DAILYREP (FALSE)    /* Daily Report */
#define WEEKLY (FALSE)      /* Weekly Report */
#define HOURLYSUM (TRUE)    /* Hourly Summary */
#define WEEKHOUR (FALSE)    /* Hour of the Week Summary */
#define HOURLYREP (FALSE)   /* Hourly Report */
#define QUARTERREP (FALSE)  /* Quarter-Hour Report */
#define QUARTERSUM (FALSE)  /* Quarter-Hour Summary */
#define FIVEREP (FALSE)     /* Five-Minute Report */
#define FIVESUM (FALSE)     /* Five-Minute Summary */
#define DOMAINREP (TRUE)    /* Domain Report */
/* Not called DOMAIN because would clash with <math.h> on some systems */
#define ORGANISATION (TRUE) /* Organisation Report */
#define DIRECTORY (TRUE)    /* Directory Report */
#define FILETYPE (TRUE)     /* File Type Report */
#define REQUEST (TRUE)      /* Request Report */
#define REDIRREP (FALSE)    /* Redirection Report */
#define FAILREP (FALSE)     /* Failure Report */
#define SIZEREP (TRUE)      /* File Size Report */
#define PROCTIME (FALSE)    /* Processing Time Report */
#define HOST (FALSE)        /* Host Report */
#define REDIRHOST (FALSE)   /* Host Report */
#define FAILHOST (FALSE)    /* Host Report */
#define REFERRER (FALSE)    /* Referrer Report */
#define REFSITE (FALSE)     /* Referring Site Report */
#define REDIRREF (FALSE)    /* Redirected Referrer Report */
#define FAILREF (FALSE)     /* Failed Referrer Report */
#define VHOST (FALSE)       /* Virtual Host Report */
#define REDIRVHOST (FALSE)  /* Virtual Host Redirection Report */
#define FAILVHOST (FALSE)   /* Virtual Host Failure Report */
#define USER (FALSE)        /* User Report */
#define REDIRUSER (FALSE)   /* User Report */
#define FAILUSER (FALSE)    /* Failed User Report */
#define SEARCHQUERY (FALSE) /* Search Query Report */
#define SEARCHWORD (TRUE)   /* Search Word Report */
#define INTSEARCHQUERY (FALSE) /* Internal Search Query Report */
#define INTSEARCHWORD (FALSE)  /* Internal Search Word Report */
#define BROWSERSUM (FALSE)  /* Browser Summary */
#define BROWSERREP (FALSE)  /* Browser Report */
#define OSREP (TRUE)        /* Operating System Report */
#define STATUS (TRUE)       /* Status Code Report */

#define GOTOS (TRUE)        /* "Go To" lines: TRUE, FALSE or FEW */
#define RUNTIME (TRUE)      /* "Running time" line: TRUE or FALSE */
#define DESCRIPTIONS (TRUE) /* Report descriptions: TRUE or FALSE */

#define REPORTSPAN (FALSE)
/* Announce time period which each report spans. This is FALSE by default
   because it uses a lot of memory to calculate. */
#define REPORTSPANTHRESHOLD (60)
/* Only announce an individual report span if it differs from the overall span
   of the whole report by at least this many minutes at one end. So set to 0 to
   list each individual one regardless. */

#define DOMCOLS "Rb"
/* Which columns we want to appear in the domain report, and in which order.
   The string can contain any of the following letters:
   R   Number of requests from each domain
   r   Percentage of the requests from each domain
   P   Number of requests for pages from each domain
   p   Percentage of the requests for pages from each domain
   B   Total number of bytes transferred to each domain
   b   The percentage of traffic to each domain
   d   Date of last access from each domain
   D   Date and time of last access
   c   Date of first access
   C   Date and time of first access
   N   Number in list */
/* And the same for the other reports */
#define ORGCOLS "Rb"
#define HOSTCOLS "Rb"
#define REDIRHOSTCOLS "R"
#define FAILHOSTCOLS "R"
#define DIRCOLS "Rb"
#define TYPECOLS "Rb"
#define REQCOLS "RbD"
#define REDIRCOLS "R"
#define FAILCOLS "R"
#define YEARCOLS "RP"
#define QUARTERLYCOLS "RP"
#define MONTHCOLS "RP"
#define DAYSUMCOLS "RP"
#define DAYREPCOLS "RP"
#define WEEKCOLS "RP"
#define HOURSUMCOLS "RP"
#define WEEKHOURCOLS "RP"
#define HOURREPCOLS "RP"
#define QUARTERSUMCOLS "RP"
#define QUARTERREPCOLS "RP"
#define FIVESUMCOLS "RP"
#define FIVEREPCOLS "RP"
#define REFCOLS "R"
#define REFSITECOLS "R"
#define REDIRREFCOLS "R"
#define FAILREFCOLS "R"
#define BROWSUMCOLS "NRP"
#define BROWREPCOLS "RP"
#define OSCOLS "NRP"
#define SIZECOLS "Rb"
#define PROCTIMECOLS "R"
#define VHOSTCOLS "Rb"
#define REDIRVHOSTCOLS "R"
#define FAILVHOSTCOLS "R"
#define USERCOLS "Rb"
#define REDIRUSERCOLS "R"
#define FAILUSERCOLS "R"
#define SEARCHQUERYCOLS "R"
#define SEARCHWORDCOLS "R"
#define INTSEARCHQUERYCOLS "R"
#define INTSEARCHWORDCOLS "R"
#define STATUSCOLS "R"

/* Should the time graphs be calculated by requests 'R', bytes 'B' or
   pages 'P': or use 'r', 'b', 'p' for graphic-free barcharts. */
#define YEARGRAPH 'P'
#define QUARTERLYGRAPH 'P'
#define MONTHGRAPH 'P'
#define DAYSUMGRAPH 'P'
#define DAYREPGRAPH 'P'
#define HOURSUMGRAPH 'P'
#define WEEKHOURGRAPH 'P'
#define HOURREPGRAPH 'P'
#define WEEKGRAPH 'P'
#define QUARTERSUMGRAPH 'P'
#define QUARTERREPGRAPH 'P'
#define FIVESUMGRAPH 'P'
#define FIVEREPGRAPH 'P'

/* Should they go forwards (oldest entries at top) or backwards? */
#define YEARBACK (FALSE)
#define QUARTERLYBACK (FALSE)
#define MONTHBACK (FALSE)
#define DAYBACK (FALSE)
#define HOURBACK (FALSE)
#define WEEKBACK (FALSE)
#define QUARTERBACK (FALSE)
#define FIVEBACK (FALSE)

/* The maximum number of rows in each (0 for "show all time") */
#define YEARROWS (0)
#define QUARTERLYROWS (0)
#define MONTHROWS (0)
#define WEEKROWS (0)
#define DAYROWS (42)
#define HOURROWS (72)
#define QUARTERROWS (96)
#define FIVEROWS (288)

/* The "floor" for each report. See docs/othreps.html for the possible formats
   for these strings. */
#define REQFLOOR "20r"
#define REQARGSFLOOR "10r"
#define REDIRFLOOR "-30r"
#define REDIRARGSFLOOR "10r"
#define FAILFLOOR "-30r"
#define FAILARGSFLOOR "10r"
#define DOMFLOOR "0b"
#define SUBDOMFLOOR "0.5%b"
#define ORGFLOOR "-20r"
#define SUBORGFLOOR "0.5%r"
#define DIRFLOOR "0.01%b"
#define SUBDIRFLOOR "0.01%b"
#define TYPEFLOOR "0.1%b"
#define SUBTYPEFLOOR "0.1%b"
#define HOSTFLOOR "-50r"
#define REDIRHOSTFLOOR "-20r"
#define FAILHOSTFLOOR "-20r"
#define REFFLOOR "20r"
#define REFARGSFLOOR "10r"
#define REFSITEFLOOR "-30r"
#define REFDIRFLOOR "50r"
#define REDIRREFFLOOR "-30r"
#define REDIRREFARGSFLOOR "10r"
#define FAILREFFLOOR "-30r"
#define FAILREFARGSFLOOR "10r"
#define BROWSUMFLOOR "-20p"
#define SUBBROWFLOOR "-10p"
#define BROWREPFLOOR "-40p"
#define OSFLOOR "0p"
#define SUBOSFLOOR "0p"
#define VHOSTFLOOR "-20b"
#define REDIRVHOSTFLOOR "-20r"
#define FAILVHOSTFLOOR "-20r"
#define USERFLOOR "-50r"
#define REDIRUSERFLOOR "-20r"
#define FAILUSERFLOOR "-20r"
#define SEARCHQUERYFLOOR "-30r"
#define SEARCHWORDFLOOR "-30r"
#define INTSEARCHQUERYFLOOR "-30r"
#define INTSEARCHWORDFLOOR "-30r"
#define STATUSFLOOR "0r"

/* How to sort each report: REQUESTS, REQUESTS7, PAGES, PAGES7, BYTES, BYTES7,
   ALPHABETICAL, DATESORT, FIRSTDATE or RANDOM */
#define REQSORTBY (REQUESTS)
#define REDIRSORTBY (REQUESTS)
#define FAILSORTBY (REQUESTS)
#define DOMSORTBY (BYTES)
#define ORGSORTBY (REQUESTS)
#define DIRSORTBY (BYTES)
#define TYPESORTBY (BYTES)
#define HOSTSORTBY (ALPHABETICAL)
#define REDIRHOSTSORTBY (REQUESTS)
#define FAILHOSTSORTBY (REQUESTS)
#define REFSORTBY (REQUESTS)
#define REFSITESORTBY (REQUESTS)
#define REDIRREFSORTBY (REQUESTS)
#define FAILREFSORTBY (REQUESTS)
#define BROWSUMSORTBY (PAGES)
#define BROWREPSORTBY (PAGES)
#define OSSORTBY (PAGES)
#define VHOSTSORTBY (BYTES)
#define REDIRVHOSTSORTBY (REQUESTS)
#define FAILVHOSTSORTBY (REQUESTS)
#define USERSORTBY (REQUESTS)
#define REDIRUSERSORTBY (REQUESTS)
#define FAILUSERSORTBY (REQUESTS)
#define SEARCHQUERYSORTBY (REQUESTS)
#define SEARCHWORDSORTBY (REQUESTS)
#define INTSEARCHQUERYSORTBY (REQUESTS)
#define INTSEARCHWORDSORTBY (REQUESTS)
#define STATUSSORTBY (ALPHABETICAL)

/* The variable to use to plot pie charts for each report. REQUESTS, REQUESTS7,
   PAGES, PAGES7, BYTES, BYTES7, CHART_SORTBY (meaning, the same as the SORTBY
   above) or CHART_NONE (no pie chart). */ 
#define REQCHART (CHART_SORTBY)
#define REDIRCHART (CHART_SORTBY)
#define FAILCHART (CHART_SORTBY)
#define TYPECHART (CHART_SORTBY)
#define SIZECHART (CHART_SORTBY)
#define PROCTIMECHART (CHART_SORTBY)
#define DIRCHART (CHART_SORTBY)
#define HOSTCHART (CHART_SORTBY)
#define REDIRHOSTCHART (CHART_SORTBY)
#define FAILHOSTCHART (CHART_SORTBY)
#define DOMCHART (CHART_SORTBY)
#define ORGCHART (CHART_SORTBY)
#define REFCHART (CHART_SORTBY)
#define REFSITECHART (CHART_SORTBY)
#define REDIRREFCHART (CHART_SORTBY)
#define FAILREFCHART (CHART_SORTBY)
#define BROWREPCHART (CHART_SORTBY)
#define BROWSUMCHART (CHART_SORTBY)
#define OSCHART (CHART_SORTBY)
#define VHOSTCHART (CHART_SORTBY)
#define REDIRVHOSTCHART (CHART_SORTBY)
#define FAILVHOSTCHART (CHART_SORTBY)
#define USERCHART (CHART_SORTBY)
#define REDIRUSERCHART (CHART_SORTBY)
#define FAILUSERCHART (CHART_SORTBY)
#define SEARCHQUERYCHART (CHART_SORTBY)
#define SEARCHWORDCHART (CHART_SORTBY)
#define INTSEARCHQUERYCHART (CHART_SORTBY)
#define INTSEARCHWORDCHART (CHART_SORTBY)
#define STATUSCHART (CHART_SORTBY)

#define WEEKBEGINSON (SUNDAY)
/* The 1st day of the week. I think it's SUNDAY, but you can choose any day. */

#define LASTSEVEN (TRUE)
/* Whether to include statistics for the last seven days */

#define COMPSEP "\t"
/* A separator string between fields in "computer" output style */

#define RAWBYTES (FALSE)
/* Whether bytes should be quoted in full (e.g., 1,021,453 bytes) as opposed
   to in kilobytes, megabytes etc. (e.g., 997.5 kilobytes). */

#define BYTESDP (2)
/* If RAWBYTES is off, how many decimal places to display in the number of
   bytes. */

#define NOROBOTS (TRUE)
/* Exclude SOME robots from indexing the output page or following its links. */

#define LINKNOFOLLOW (TRUE)
/* Appends rel="nofollow" to outbound URLs to reduce SEO SPAM targeting */

#define HEADERFILE "none"
/* an extra file (with HTML markup if desired) placed between the page title
   and the start of the statistics. Use "none" for none. */

#define FOOTERFILE "none"
/* And one placed at the bottom of the page. */

#define OUTPUT (XHTML)
/* Default output type: HTML, XHTML, PLAIN, ASCII, LATEX, COMPUTER or
   OUT_NONE. */

#ifndef LANGUAGE
#define LANGUAGE (ENGLISH)
/* Default language for the output page:
   ARMENIAN, BASQUE, BULGARIAN, BULGARIAN_MIK, CATALAN, SIMP_CHINESE (GB2312),
   TRAD_CHINESE (Big5), CZECH, CZECH_1250, DANISH, DUTCH, ENGLISH, US_ENGLISH,
   FINNISH, FRENCH, GERMAN, HUNGARIAN, INDONESIAN, ITALIAN,
   JAPANESE_EUC, JAPANESE_JIS, JAPANESE_SJIS, JAPANESE_UTF,
   KOREAN, LATVIAN, NORWEGIAN (Bokmal), NYNORSK, POLISH, PORTUGUESE,
   BR_PORTUGUESE, RUSSIAN, RUSSIAN_1251, SERBIAN, SLOVAK, SLOVAK_1250,
   SLOVENE, SLOVENE_1250, SPANISH, SWEDISH, SWEDISH_ALT, TURKISH or UKRAINIAN.
   *
   The following languages were available for previous versions of analog, but
   have not yet been translated for version 5. As they are translated, they
   will be added to the analog home page:
   BOSNIAN, CROATIAN, GREEK, ICELANDIC, LITHUANIAN, ROMANIAN and SLOVAK. */
#endif

#define MARKCHAR '+'
/* a character for the graphical displays */

#define BARSTYLE 'b'
/* The default bar style for the displays -- 'a' to 'h' */

#define PNGIMAGES (TRUE)
/* Whether the graphics should be png's (TRUE) or gif's (FALSE). */

#define HTMLPAGEWIDTH (65)
#define PLAINPAGEWIDTH (76)
#define LATEXPAGEWIDTH (65)
/* The width of the output for HTML, ASCII/PLAIN, and LaTeX */

#define MINGRAPHWIDTH (15)
/* bar charts must be at least this many characters wide */

#define CSSPREFIX ""
/* A prefix to put in front of all CSS class names in XHTML mode.
   Use "" for no prefix. */

#define ERRLINELENGTH (78)
#define STDERRWIDTH (ERRLINELENGTH)
/* the maximum width for error and warning messages */

#endif  /* ANLGHEA2_H */
