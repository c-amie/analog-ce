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

/*** globals.c; declaration and initialisation of globals, and defaults ***/
/* See also init.c and init2.c */

#include "anlghea3.h"

/* Program time */
time_t origstarttime, starttime;
timecode_t starttimec, starttimeuxc;
char *starttimestr;
/* NB starttime and starttimec are displaced by TIMEOFFSET in correct(), but
   starttimestr (used in warnings) and starttimeuxc (used in DNS) are not. */

/* What to read from input lines */
unsigned int year, month, date, hour, minute, code;
unsigned long unixtime, proctime;
double bytes;
char am;

/* Input processing */
char *block_start, *block_end, *block_bell, *record_start, *pos;
logical termchar[256];

/* Memory allocation */
Memman *xmemman, *amemman, mm[ITEM_NUMBER], mmq, mms;
char *workspace; /* scratch space for aliasing: see note at top of alias.c */
/* NB Be careful not to nest two usages of workspace. */

/* Are we running from form? */
logical cgi = FALSE;

/* DNS lookups: externs so that no worries about whether to pass them around */
#ifndef NODNS
choice dnslevel;
char *dnsfile, *dnslockfile;
FILE *dnsfilep = NULL, *dnslock = NULL;
Hashtable *dnstable;
unsigned int dnsgoodhrs, dnsbadhrs;
#ifndef NOALARM
unsigned int dnstimeout;
JMP_BUF jumpbuf;
#endif
#endif

/* Signal entries for unwanted hash entries (NULL is already used for unset) */
Hashentry *unwanted_entry, *blank_entry;
Hashindex *dummy_item;

/* Translating reports to other arrays */
choice *rep2type, *rep2reqs, *rep2reqs7, *rep2date, *rep2firstd;
unsigned int *rep2gran, *rep2lng, *rep2datefmt, *rep2colhead, *rep2busystr;
unsigned int *col2colhead, *method2sing, *method2pl, *method2date;
unsigned int *method2pc, *method2relpc, *method2sort;
logical *repistree;
char *byteprefix = " kMGTPEZY";  /* Room for growth :)  Note initial space */
/* NB Output versions of the byte prefixes are in the language files */

/* Domain levels; making this global is messy but easiest */
Strpairlist **domlevels;

/* Log formats */
Inputformatlist *logformat = NULL, *deflogformat;
logical newloglist = TRUE, iscache = FALSE;
int tz = 0, stz = 0;
choice wantitem[INPUT_NUMBER];
#ifndef NOPIPES
Strpairlist *uncompresshead = NULL;
#endif

/* Debugging, warnings, pretty printing */
FILE *errfile;
char *commandname = NULL, *commandpath = NULL;
char *debug_args = NULL, *warn_args = NULL;
logical anywarns = FALSE, errmess = FALSE, vblesonly = FALSE;
unsigned int errwidth = ERRLINELENGTH;
unsigned long progressfreq = 0;
unsigned int ppcol = 0;

/* What's recorded by cache files. Array size V5_DATA_NUMBER in anlghea3.h. */
/* don't record "last 7" stuff because it won't be correct at read time */
choice cache_records[V5_DATA_NUMBER] = {REQUESTS, PAGES, REDIR, FAIL, SUCCDATE,
					REDIRDATE, FAILDATE, SUCCFIRSTD,
					REDIRFIRSTD, FAILFIRSTD};

/* Date names for unlocalised text. See also ENGMONTHLEN etc. in anlghea3.h. */
char *engmonths[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
		       "Sep", "Oct", "Nov", "Dec"};
char *englongdays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
			"Friday", "Saturday"};
char *engshortdays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/* Names for things, always in order of enums in anlghea3.h */
char *item_type[] = {"virtual hosts", "files", "users", "referrers",
		     "browsers", "hosts"};
/* In methodname, we don't need them all, because some only occur internally */
char *methodname[] = {"requests","7-day requests", "pages", "7-day pages", "",
		      "", "", "", "last date", "", "", "first date", "", "",
		      "bytes", "7-day bytes", "alphabetical", "random"};
/* Filenames for each language, in order of enum in anlghea3.h */
char *country[] = {"am", "baq", /*"ba",*/ "bg", "bgm", "cat", "cn", "tw",
		   /*"hr",*/ "cz", "cz1", "dk", "nl", "uk", "us", "fi", "fr",
		   "de", /*"gr",*/ "hu", /*"is",*/ "id", "it", "jpe", "jpj",
		   "jps", "jpu", "kr", "lv", /*"lt",*/ "no", "no2", "pl", "pt",
		   "br", /*"ro",*/ "ru", "ru1", "yu", "sk", "sk1", "si", "si1",
		   "es", "se", "se2", "tr", "ua"};
/* Report code letters in order of enum in anlghea3.h */
/* Any new ones must also be added to reportorder in anlghea2.h */
char repcodes[] = "1QmWDH45dhw67xrEItiSlLofskKBvRMujJZNnYybpzcP";

/* Column code letters in order of enum in anlghea3.h */
char colcodes[] = "xRSPQBCrspqbcdDeEN";

/* Names for the anchors within the report, in the same order */
/* At most eight letters, because we use them as filenames too. (This limit is
   assumed in piechart()). */
char *anchorname[] = {"year", "quartly", "month", "week", "dayrep", "hourrep",
		      "quartrep", "fiverep",  "daysum", "hoursum", "weekhour",
		      "quartsum", "fivesum", "gensum", "req", "redir", "fail",
		      "type", "dir", "host", "redihost", "failhost", "dom",
		      "ref", "refsite", "redirref", "failref", "browrep",
		      "vhost", "redirvh", "failvh", "user", "rediuser",
		      "failuser", "org", "searchq", "searchw", "isearchq",
		      "isearchw", "browsum", "os", "size", "code", "proctime"};
/* These report names are in English, so not the same as in langfile */
/* Again in order of enum in anlghea3.h */
char *repname[] = {"Yearly Report", "Quarterly Report", "Monthly Report",
		   "Weekly Report", "Daily Report", "Hourly Report",
		   "Quarter-Hour Report", "Five-Minute Report",
		   "Daily Summary", "Hourly Summary",
		   "Hour of the Week Summary", "Quarter-Hour Summary",
		   "Five-Minute Summary", "General Summary", "Request Report",
		   "Redirection Report", "Failure Report", "File Type Report",
		   "Directory Report", "Host Report",
		   "Host Redirection Report", "Host Failure Report",
		   "Domain Report", "Referrer Report", "Referring Site Report",
		   "Redirected Referrer Report", "Failed Referrer Report",
		   "Browser Report", "Virtual Host Report",
		   "Virtual Host Redirection Report",
		   "Virtual Host Failure Report", "User Report",
		   "User Redirection Report", "User Failure Report",
		   "Organisation Report", "Search Query Report",
		   "Search Word Report", "Internal Search Query Report",
		   "Internal Search Word Report", "Browser Summary",
		   "Operating System Report", "File Size Report",
		   "Status Code Report", "Processing Time Report"};

#ifdef EBCDIC
/* EBCDIC-to-ASCII tables:
  These tables are bijective - there are no ambigous or duplicate characters.
 */
const unsigned char os_toascii[256] = {
#ifdef _OSD_POSIX /* Fujitsu-Siemens' EDF04 character set on BS2000: */
/*00*/  0x00, 0x01, 0x02, 0x03, 0x85, 0x09, 0x86, 0x7f,
        0x87, 0x8d, 0x8e, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /*................*/
/*10*/  0x10, 0x11, 0x12, 0x13, 0x8f, 0x0a, 0x08, 0x97,
        0x18, 0x19, 0x9c, 0x9d, 0x1c, 0x1d, 0x1e, 0x1f, /*................*/
/*20*/  0x80, 0x81, 0x82, 0x83, 0x84, 0x92, 0x17, 0x1b,
        0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x05, 0x06, 0x07, /*................*/
/*30*/  0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04,
        0x98, 0x99, 0x9a, 0x9b, 0x14, 0x15, 0x9e, 0x1a, /*................*/
/*40*/  0x20, 0xa0, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5,
        0xe7, 0xf1, 0x60, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, /* .........`.<(+|*/
/*50*/  0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef,
        0xec, 0xdf, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x9f, /*&.........!$*);.*/
/*60*/  0x2d, 0x2f, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5,
        0xc7, 0xd1, 0x5e, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, /*-/........^,%_>?*/
/*70*/  0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf,
        0xcc, 0xa8, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, /*..........:#@'="*/
/*80*/  0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1, /*.abcdefghi......*/
/*90*/  0xb0, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
        0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8, 0xc6, 0xa4, /*.jklmnopqr......*/
/*a0*/  0xb5, 0xaf, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0xdd, 0xde, 0xae, /*..stuvwxyz......*/
/*b0*/  0xa2, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc,
        0xbd, 0xbe, 0xac, 0x5b, 0x5c, 0x5d, 0xb4, 0xd7, /*...........[\]..*/
/*c0*/  0xf9, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5, /*.ABCDEFGHI......*/
/*d0*/  0xa6, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
        0x51, 0x52, 0xb9, 0xfb, 0xfc, 0xdb, 0xfa, 0xff, /*.JKLMNOPQR......*/
/*e0*/  0xd9, 0xf7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5, /*..STUVWXYZ......*/
/*f0*/  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0xb3, 0x7b, 0xdc, 0x7d, 0xda, 0x7e  /*0123456789.{.}.~*/

#elif defined(OS390) || defined(AS400) || defined(TPF)  /* IBM's OS/390 and TPF systems: */
/*
Bijective EBCDIC (character set IBM-1047) to US-ASCII table:
*/
    0x00, 0x01, 0x02, 0x03, 0x85, 0x09, 0x86, 0x7f, /* 00-0f:           */
    0x87, 0x8d, 0x8e, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* ................ */
    0x10, 0x11, 0x12, 0x13, 0x8f, 0x0a, 0x08, 0x97, /* 10-1f:           */
    0x18, 0x19, 0x9c, 0x9d, 0x1c, 0x1d, 0x1e, 0x1f, /* ................ */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x92, 0x17, 0x1b, /* 20-2f:           */
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x05, 0x06, 0x07, /* ................ */
    0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04, /* 30-3f:           */
    0x98, 0x99, 0x9a, 0x9b, 0x14, 0x15, 0x9e, 0x1a, /* ................ */
    0x20, 0xa0, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5, /* 40-4f:           */
    0xe7, 0xf1, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, /*  ...........<(+| */
    0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef, /* 50-5f:           */
    0xec, 0xdf, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x5e, /* &.........!$*);^ */
    0x2d, 0x2f, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5, /* 60-6f:           */
    0xc7, 0xd1, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, /* -/.........,%_>? */
    0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf, /* 70-7f:           */
    0xcc, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, /* .........`:#@'=" */
    0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 80-8f:           */
    0x68, 0x69, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1, /* .abcdefghi...... */
    0xb0, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, /* 90-9f:           */
    0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8, 0xc6, 0xa4, /* .jklmnopqr...... */
    0xb5, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, /* a0-af:           */
    0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0x5b, 0xde, 0xae, /* .~stuvwxyz...[.. */
    0xac, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc, /* b0-bf:           */
    0xbd, 0xbe, 0xdd, 0xa8, 0xaf, 0x5d, 0xb4, 0xd7, /* .............].. */
    0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, /* c0-cf:           */
    0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5, /* {ABCDEFGHI...... */
    0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, /* d0-df:           */
    0x51, 0x52, 0xb9, 0xfb, 0xfc, 0xf9, 0xfa, 0xff, /* }JKLMNOPQR...... */
    0x5c, 0xf7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, /* e0-ef:           */
    0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5, /* \.STUVWXYZ...... */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* f0-ff:           */
    0x38, 0x39, 0xb3, 0xdb, 0xdc, 0xd9, 0xda, 0x9f  /* 0123456789...... */
#else
#error Unimplemented EBCDIC platform. Please send information about your system to <martin@apache.org>!
#endif
};
/* Bijective ascii-to-ebcdic table: */
const unsigned char os_toebcdic[256] = {
#ifdef _OSD_POSIX /* Fujitsu-Siemens' EDF04 character set on BS2000: */
/*00*/  0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f,
        0x16, 0x05, 0x15, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,  /*................*/
/*10*/  0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26,
        0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f,  /*................*/
/*20*/  0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d,
        0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,  /* !"#$%&'()*+,-./ */
/*30*/  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,  /*0123456789:;<=>?*/
/*40*/  0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
        0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,  /*@ABCDEFGHIJKLMNO*/
/*50*/  0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
        0xe7, 0xe8, 0xe9, 0xbb, 0xbc, 0xbd, 0x6a, 0x6d,  /*PQRSTUVWXYZ[\]^_*/
/*60*/  0x4a, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,  /*`abcdefghijklmno*/
/*70*/  0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
        0xa7, 0xa8, 0xa9, 0xfb, 0x4f, 0xfd, 0xff, 0x07,  /*pqrstuvwxyz{|}~.*/
/*80*/  0x20, 0x21, 0x22, 0x23, 0x24, 0x04, 0x06, 0x08,
        0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x09, 0x0a, 0x14,  /*................*/
/*90*/  0x30, 0x31, 0x25, 0x33, 0x34, 0x35, 0x36, 0x17,
        0x38, 0x39, 0x3a, 0x3b, 0x1a, 0x1b, 0x3e, 0x5f,  /*................*/
/*a0*/  0x41, 0xaa, 0xb0, 0xb1, 0x9f, 0xb2, 0xd0, 0xb5,
        0x79, 0xb4, 0x9a, 0x8a, 0xba, 0xca, 0xaf, 0xa1,  /*................*/
/*b0*/  0x90, 0x8f, 0xea, 0xfa, 0xbe, 0xa0, 0xb6, 0xb3,
        0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab,  /*................*/
/*c0*/  0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68,
        0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,  /*................*/
/*d0*/  0xac, 0x69, 0xed, 0xee, 0xeb, 0xef, 0xec, 0xbf,
        0x80, 0xe0, 0xfe, 0xdd, 0xfc, 0xad, 0xae, 0x59,  /*................*/
/*e0*/  0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48,
        0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,  /*................*/
/*f0*/  0x8c, 0x49, 0xcd, 0xce, 0xcb, 0xcf, 0xcc, 0xe1,
        0x70, 0xc0, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0xdf   /*................*/
#elif defined(OS390) || defined(AS400) || defined(TPF)  /* IBM's OS/390 and TPF systems: */
/*
The US-ASCII to EBCDIC (character set IBM-1047) table:
This table is bijective (no ambiguous or duplicate characters)
*/
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2d, 0x2e, 0x2f, /* 00-0f:           */
    0x16, 0x05, 0x15, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* ................ */
    0x10, 0x11, 0x12, 0x13, 0x3c, 0x3d, 0x32, 0x26, /* 10-1f:           */
    0x18, 0x19, 0x3f, 0x27, 0x1c, 0x1d, 0x1e, 0x1f, /* ................ */
    0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d, /* 20-2f:           */
    0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61, /*  !"#$%&'()*+,-./ */
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* 30-3f:           */
    0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f, /* 0123456789:;<=>? */
    0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, /* 40-4f:           */
    0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, /* @ABCDEFGHIJKLMNO */
    0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, /* 50-5f:           */
    0xe7, 0xe8, 0xe9, 0xad, 0xe0, 0xbd, 0x5f, 0x6d, /* PQRSTUVWXYZ[\]^_ */
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 60-6f:           */
    0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, /* `abcdefghijklmno */
    0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, /* 70-7f:           */
    0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x07, /* pqrstuvwxyz{|}~. */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x04, 0x06, 0x08, /* 80-8f:           */
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x09, 0x0a, 0x14, /* ................ */
    0x30, 0x31, 0x25, 0x33, 0x34, 0x35, 0x36, 0x17, /* 90-9f:           */
    0x38, 0x39, 0x3a, 0x3b, 0x1a, 0x1b, 0x3e, 0xff, /* ................ */
    0x41, 0xaa, 0x4a, 0xb1, 0x9f, 0xb2, 0x6a, 0xb5, /* a0-af:           */
    0xbb, 0xb4, 0x9a, 0x8a, 0xb0, 0xca, 0xaf, 0xbc, /* ................ */
    0x90, 0x8f, 0xea, 0xfa, 0xbe, 0xa0, 0xb6, 0xb3, /* b0-bf:           */
    0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab, /* ................ */
    0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68, /* c0-cf:           */
    0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77, /* ................ */
    0xac, 0x69, 0xed, 0xee, 0xeb, 0xef, 0xec, 0xbf, /* d0-df:           */
    0x80, 0xfd, 0xfe, 0xfb, 0xfc, 0xba, 0xae, 0x59, /* ................ */
    0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48, /* e0-ef:           */
    0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57, /* ................ */
    0x8c, 0x49, 0xcd, 0xce, 0xcb, 0xcf, 0xcc, 0xe1, /* f0-ff:           */
    0x70, 0xdd, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0xdf  /* ................ */
#else
#error Unimplemented EBCDIC platform. Please send information about your system to <martin@apache.org>!
#endif
};
#endif

logical convertchar[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 00 - 0F */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 10 - 1F */
  1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,   /* 20 - 2F */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,   /* 30 - 3F */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   /* 40 - 4F */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   /* 50 - 5F */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   /* 60 - 6F */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,   /* 70 - 7F */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 80 - 8F */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* 90 - 9F */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* A0 - AF */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* B0 - BF */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* C0 - CF */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* D0 - DF */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   /* E0 - EF */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  /* F0 - FF */
/* %nm escapes to convert in do_alias(r|f). We convert all ASCII printable
   characters (0x20 - 0x7E) with the exception of % (0x25) [to distinguish
   %25ab from %ab]; and ? (0x3f), & (0x26), ; (0x3b) and = (0x3d) [to split
   query strings up correctly]. Could argue for + (0x2b) in query strings, but
   messes search reports up if we do this. */

logical unprintable[256];
/* Unprintable chars for the reports. This is charset dependent. But NB not the
   same as isctrl() or !isprint() even for ASCII range. */

/* The outputters */
Outputter cro_outputter =
  {cro_pagewidth, cro_cgihead, cro_stylehead, cro_pagetitle, cro_timings,
   cro_closehead, cro_pagebotstart, cro_credit, cro_runtime, cro_pagefoot,
   cro_stylefoot, cro_reporttitle, cro_reportfooter, cro_reportdesc,
   cro_reportspan, cro_gensumhead, cro_gensumfoot, cro_gensumline,
   cro_gensumlineb, cro_lastseven, cro_prestart, cro_preend, cro_hrule,
   cro_endash, cro_putch, cro_strlength, cro_allowmonth, cro_calcwidths,
   cro_declareunit, cro_colheadstart, cro_colheadcol, cro_colheadend,
   cro_colheadustart, cro_colheadunderline, cro_colheaduend, cro_rowstart,
   cro_levelcell, cro_namecell, cro_ulcell, cro_strcell, cro_bytescell,
   cro_pccell, cro_indexcell, cro_rowend, cro_blankline, cro_barchart,
   cro_busyprintf, cro_notlistedstr, cro_whatincluded, NULL, NULL, NULL, NULL,
   cro_includefile, cro_riscosfiletype};
Outputter html_outputter =
  {html_pagewidth, html_cgihead, html_stylehead, html_pagetitle, html_timings,
   html_closehead, html_pagebotstart, html_credit, html_runtime, html_pagefoot,
   html_stylefoot, html_reporttitle, html_reportfooter, html_reportdesc,
   html_reportspan, html_gensumhead, html_gensumfoot, html_gensumline,
   html_gensumlineb, html_lastseven, html_prestart, html_preend, html_hrule,
   html_endash, html_putch, html_strlength, html_allowmonth, html_calcwidths,
   html_declareunit, html_colheadstart, html_colheadcol, html_colheadend,
   html_colheadustart, html_colheadunderline, html_colheaduend, html_rowstart,
   html_levelcell, html_namecell, html_ulcell, html_strcell, html_bytescell,
   html_pccell, html_indexcell, html_rowend, html_blankline, html_barchart,
   html_busyprintf, html_notlistedstr, html_whatincluded, html_whatinchead,
   html_whatincfoot, html_whatincprintstr, html_printdouble, html_includefile,
   html_riscosfiletype};
Outputter latex_outputter =
  {latex_pagewidth, latex_cgihead, latex_stylehead, latex_pagetitle,
   latex_timings, latex_closehead, latex_pagebotstart, latex_credit,
   latex_runtime, latex_pagefoot, latex_stylefoot, latex_reporttitle,
   latex_reportfooter, latex_reportdesc, latex_reportspan, latex_gensumhead,
   latex_gensumfoot, latex_gensumline, latex_gensumlineb, latex_lastseven,
   latex_prestart, latex_preend, latex_hrule, latex_endash, latex_putch,
   latex_strlength, latex_allowmonth, latex_calcwidths, latex_declareunit,
   latex_colheadstart, latex_colheadcol, latex_colheadend, latex_colheadustart,
   latex_colheadunderline, latex_colheaduend, latex_rowstart, latex_levelcell,
   latex_namecell, latex_ulcell, latex_strcell, latex_bytescell, latex_pccell,
   latex_indexcell, latex_rowend, latex_blankline, latex_barchart,
   latex_busyprintf, latex_notlistedstr, latex_whatincluded, latex_whatinchead,
   latex_whatincfoot, latex_whatincprintstr, latex_printdouble,
   latex_includefile, latex_riscosfiletype};
Outputter plain_outputter =
  {plain_pagewidth, plain_cgihead, plain_stylehead, plain_pagetitle,
   plain_timings, plain_closehead, plain_pagebotstart, plain_credit,
   plain_runtime, plain_pagefoot, plain_stylefoot, plain_reporttitle,
   plain_reportfooter, plain_reportdesc, plain_reportspan, plain_gensumhead,
   plain_gensumfoot, plain_gensumline, plain_gensumlineb, plain_lastseven,
   plain_prestart, plain_preend, plain_hrule, plain_endash, plain_putch,
   plain_strlength, plain_allowmonth, plain_calcwidths, plain_declareunit,
   plain_colheadstart, plain_colheadcol, plain_colheadend, plain_colheadustart,
   plain_colheadunderline, plain_colheaduend, plain_rowstart, plain_levelcell,
   plain_namecell, plain_ulcell, plain_strcell, plain_bytescell, plain_pccell,
   plain_indexcell, plain_rowend, plain_blankline, plain_barchart,
   plain_busyprintf, plain_notlistedstr, plain_whatincluded, plain_whatinchead,
   plain_whatincfoot, plain_whatincprintstr, plain_printdouble,
   plain_includefile, plain_riscosfiletype};
Outputter xhtml_outputter =
  {xhtml_pagewidth, xhtml_cgihead, xhtml_stylehead, xhtml_pagetitle,
   xhtml_timings, xhtml_closehead, xhtml_pagebotstart, xhtml_credit,
   xhtml_runtime, xhtml_pagefoot, xhtml_stylefoot, xhtml_reporttitle,
   xhtml_reportfooter, xhtml_reportdesc, xhtml_reportspan, xhtml_gensumhead,
   xhtml_gensumfoot, xhtml_gensumline, xhtml_gensumlineb, xhtml_lastseven,
   xhtml_prestart, xhtml_preend, xhtml_hrule, xhtml_endash, xhtml_putch,
   xhtml_strlength, xhtml_allowmonth, xhtml_calcwidths, xhtml_declareunit,
   xhtml_colheadstart, xhtml_colheadcol, xhtml_colheadend, xhtml_colheadustart,
   xhtml_colheadunderline, xhtml_colheaduend, xhtml_rowstart, xhtml_levelcell,
   xhtml_namecell, xhtml_ulcell, xhtml_strcell, xhtml_bytescell, xhtml_pccell,
   xhtml_indexcell, xhtml_rowend, xhtml_blankline, xhtml_barchart,
   xhtml_busyprintf, xhtml_notlistedstr, xhtml_whatincluded, xhtml_whatinchead,
   xhtml_whatincfoot, xhtml_whatincprintstr, xhtml_printdouble,
   xhtml_includefile, xhtml_riscosfiletype};
Outputter xml_outputter =
  {xml_pagewidth, xml_cgihead, xml_stylehead, xml_pagetitle, xml_timings,
   xml_closehead, xml_pagebotstart, xml_credit, xml_runtime, xml_pagefoot,
   xml_stylefoot, xml_reporttitle, xml_reportfooter, xml_reportdesc,
   xml_reportspan, xml_gensumhead, xml_gensumfoot, xml_gensumline,
   xml_gensumlineb, xml_lastseven, xml_prestart, xml_preend, xml_hrule,
   xml_endash, xml_putch, xml_strlength, xml_allowmonth, xml_calcwidths,
   xml_declareunit, xml_colheadstart, xml_colheadcol, xml_colheadend,
   xml_colheadustart, xml_colheadunderline, xml_colheaduend, xml_rowstart,
   xml_levelcell, xml_namecell, xml_ulcell, xml_strcell, xml_bytescell,
   xml_pccell, xml_indexcell, xml_rowend, xml_blankline, xml_barchart,
   xml_busyprintf, xml_notlistedstr, xml_whatincluded, xml_whatinchead,
   xml_whatincfoot, xml_whatincprintstr, xml_printdouble, xml_includefile,
   xml_riscosfiletype};

Inputfns inpfns[] = { /* first the ITEM fns in any order */
  /* See also ITEMFNS_NUMBER in anlghea3.c */
  /* Also, some letters, namely those with &parsestring and other codes that
     take following separators (see list in strtoinfmt() in init.c), need to be
     included in the list in report_logfmt() in settings.c. */
  {'v', ITEM_VHOST, &parsestring, &mm[ITEM_VHOST]},
  {'r', ITEM_FILE, &parsestring, &mm[ITEM_FILE]},
  {'u', ITEM_USER, &parsestring, &mm[ITEM_USER]},
  {'f', ITEM_REFERRER, &parsestring, &mm[ITEM_REFERRER]},
  {'B', ITEM_BROWSER, &parsestring, &mm[ITEM_BROWSER]},
  {'S', ITEM_HOST, &parsestring, &mm[ITEM_HOST]},
  {'F', ITEM_REFERRER, &parseref, &mm[ITEM_REFERRER]},
  {'A', ITEM_BROWSER, &parsemsbrow, &mm[ITEM_BROWSER]},
  {'q', INP_QUERY, &parsestring, &mmq},
  {'s', INP_IP, &parsestring, &mms},
  {'n', INP_MIN, &parseuint2strict, (void *)&minute},
  {'h', INP_HOUR, &parseuint2, (void *)&hour},
  {'a', INP_AM, &parseam, (void *)&am},
  {'d', INP_DATE, &parseuint2, (void *)&date},
  {'m', INP_MONTH, &parsenmonth, (void *)&month},
  {'M', INP_MONTH, &parsemonth, (void *)&month},
  {'y', INP_YEAR, &parseyear2, (void *)&year},
  {'Y', INP_YEAR, &parseuint4strict, (void *)&year},
  {'Z', INP_YEAR, &parseyear2or4, (void *)&year},
  {'U', INP_UNIXTIME, &parseunixtime, (void *)&unixtime},
  {'T', INP_PROCTIME, &parseunixtime, (void *)&proctime},
  {'t', INP_PROCTIME, &parseproctime, (void *)&proctime},
  {'D', INP_PROCTIME, &parseproctimeu, (void *)&proctime},
  {'c', INP_CODE, &parsescode, (void *)&code},
  {'C', INP_CODE, &parsecode, (void *)&code},
  {'b', INP_BYTES, &parseudint, (void *)&bytes},
  {'w', UNSET, &parsespace, NULL},
  {'W', UNSET, &parseoptspace, NULL},
  {'j', UNSET, &parsejunk, NULL},
  {'x', UNSET, &parselogfmt, NULL},
  {'\0', UNSET, &checkchar, NULL}  /* this one must be last as marker */
};
Inputfns pnlinpfn = {'\n', UNSET, &parsenewline, NULL};
Inputfns ccinpfn = {'\0', UNSET, &checkchar, NULL};
Inputfns pjinpfn = {'j', UNSET, &parsejunk, NULL};

Options opts;
/* So that in future we can have several sets of options, and yet still
   declare cf[] neatly as below, we shunt them in and out of opts */
Configfns cf[] = {
  {"TIMECOLS", &configallcols, (void *)(opts.outopts.cols)},
  {"YEARCOLS", &configcols, (void *)(opts.outopts.cols[REP_YEAR])},
  {"QUARTERLYCOLS", &configcols, (void *)(opts.outopts.cols[REP_QUARTERLY])},
  {"MONTHCOLS", &configcols, (void *)(opts.outopts.cols[REP_MONTH])},
  {"WEEKCOLS", &configcols, (void *)(opts.outopts.cols[REP_WEEK])},
  {"DAYREPCOLS", &configcols, (void *)(opts.outopts.cols[REP_DAYREP])},
  {"FULLDAYCOLS", &configcols, (void *)(opts.outopts.cols[REP_DAYREP])},
  {"DAYSUMCOLS", &configcols, (void *)(opts.outopts.cols[REP_DAYSUM])},
  {"DAYCOLS", &configcols, (void *)(opts.outopts.cols[REP_DAYSUM])},
  {"HOURREPCOLS", &configcols, (void *)(opts.outopts.cols[REP_HOURREP])},
  {"FULLHOURCOLS", &configcols, (void *)(opts.outopts.cols[REP_HOURREP])},
  {"HOURSUMCOLS", &configcols, (void *)(opts.outopts.cols[REP_HOURSUM])},
  {"HOURCOLS", &configcols, (void *)(opts.outopts.cols[REP_HOURSUM])},
  {"WEEKHOURCOLS", &configcols,
   (void *)(opts.outopts.cols[REP_WEEKHOUR])},
  {"QUARTERREPCOLS", &configcols, (void *)(opts.outopts.cols[REP_QUARTERREP])},
  {"QUARTERCOLS", &configcols, (void *)(opts.outopts.cols[REP_QUARTERREP])},
  {"QUARTERSUMCOLS", &configcols, (void *)(opts.outopts.cols[REP_QUARTERSUM])},
  {"FIVEREPCOLS", &configcols, (void *)(opts.outopts.cols[REP_FIVEREP])},
  {"FIVECOLS", &configcols, (void *)(opts.outopts.cols[REP_FIVEREP])},
  {"FIVESUMCOLS", &configcols, (void *)(opts.outopts.cols[REP_FIVESUM])},
  {"REQCOLS", &configcols, (void *)(opts.outopts.cols[REP_REQ])},
  {"REDIRCOLS", &configcols, (void *)(opts.outopts.cols[REP_REDIR])},
  {"FAILCOLS", &configcols, (void *)(opts.outopts.cols[REP_FAIL])},
  {"TYPECOLS", &configcols, (void *)(opts.outopts.cols[REP_TYPE])},
  {"SIZECOLS", &configcols, (void *)(opts.outopts.cols[REP_SIZE])},
  {"PROCTIMECOLS", &configcols, (void *)(opts.outopts.cols[REP_PROCTIME])},
  {"DIRCOLS", &configcols, (void *)(opts.outopts.cols[REP_DIR])},
  {"HOSTCOLS", &configcols, (void *)(opts.outopts.cols[REP_HOST])},
  {"REDIRHOSTCOLS", &configcols, (void *)(opts.outopts.cols[REP_REDIRHOST])},
  {"FAILHOSTCOLS", &configcols, (void *)(opts.outopts.cols[REP_FAILHOST])},
  {"DOMCOLS", &configcols, (void *)(opts.outopts.cols[REP_DOM])},
  {"ORGCOLS", &configcols, (void *)(opts.outopts.cols[REP_ORG])},
  {"REFCOLS", &configcols, (void *)(opts.outopts.cols[REP_REF])},
  {"REFSITECOLS", &configcols, (void *)(opts.outopts.cols[REP_REFSITE])},
  {"REDIRREFCOLS", &configcols, (void *)(opts.outopts.cols[REP_REDIRREF])},
  {"FAILREFCOLS", &configcols, (void *)(opts.outopts.cols[REP_FAILREF])},
  {"BROWREPCOLS", &configcols, (void *)(opts.outopts.cols[REP_BROWREP])},
  {"FULLBROWCOLS", &configcols, (void *)(opts.outopts.cols[REP_BROWREP])},
  {"BROWSUMCOLS", &configcols, (void *)(opts.outopts.cols[REP_BROWSUM])},
  {"BROWCOLS", &configcols, (void *)(opts.outopts.cols[REP_BROWSUM])},
  {"OSCOLS", &configcols, (void *)(opts.outopts.cols[REP_OS])},
  {"VHOSTCOLS", &configcols, (void *)(opts.outopts.cols[REP_VHOST])},
  {"REDIRVHOSTCOLS", &configcols, (void *)(opts.outopts.cols[REP_REDIRVHOST])},
  {"FAILVHOSTCOLS", &configcols, (void *)(opts.outopts.cols[REP_FAILVHOST])},
  {"USERCOLS", &configcols, (void *)(opts.outopts.cols[REP_USER])},
  {"REDIRUSERCOLS", &configcols, (void *)(opts.outopts.cols[REP_REDIRUSER])},
  {"FAILUSERCOLS", &configcols, (void *)(opts.outopts.cols[REP_FAILUSER])},
  {"SEARCHQUERYCOLS", &configcols, (void *)(opts.outopts.cols[REP_SEARCHREP])},
  {"SEARCHWORDCOLS", &configcols, (void *)(opts.outopts.cols[REP_SEARCHSUM])},
  {"INTSEARCHQUERYCOLS", &configcols,
   (void *)(opts.outopts.cols[REP_INTSEARCHREP])},
  {"INTSEARCHWORDCOLS", &configcols,
   (void *)(opts.outopts.cols[REP_INTSEARCHSUM])},
  {"STATUSCOLS", &configcols, (void *)(opts.outopts.cols[REP_CODE])},
  {"YEARGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_YEAR])},
  {"QUARTERLYGRAPH", &configgraph,
   (void *)&(opts.outopts.graph[REP_QUARTERLY])},
  {"MONTHGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_MONTH])},
  {"WEEKGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_WEEK])},
  {"DAYREPGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_DAYREP])},
  {"FULLDAYGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_DAYREP])},
  {"DAYSUMGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_DAYSUM])},
  {"DAYGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_DAYSUM])},
  {"HOURREPGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_HOURREP])},
  {"FULLHOURGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_HOURREP])},
  {"HOURSUMGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_HOURSUM])},
  {"HOURGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_HOURSUM])},
  {"WEEKHOURGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_WEEKHOUR])},
  {"QUARTERREPGRAPH", &configgraph,
   (void *)&(opts.outopts.graph[REP_QUARTERREP])},
  {"QUARTERGRAPH", &configgraph,
   (void *)&(opts.outopts.graph[REP_QUARTERREP])},
  {"QUARTERSUMGRAPH", &configgraph,
   (void *)&(opts.outopts.graph[REP_QUARTERSUM])},
  {"FIVEREPGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_FIVEREP])},
  {"FIVEGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_FIVEREP])},
  {"FIVESUMGRAPH", &configgraph, (void *)&(opts.outopts.graph[REP_FIVESUM])},
  {"ALLGRAPH", &configallgraph, (void *)(opts.outopts.graph)},
  {"ALLGRAPHS", &configallgraph, (void *)(opts.outopts.graph)},
#ifndef NOGRAPHICS
  {"REQCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_REQ)])},
  {"REDIRCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_REDIR)])},
  {"FAILCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_FAIL)])},
  {"TYPECHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_TYPE)])},
  {"SIZECHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_SIZE)])},
  {"PROCTIMECHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_PROCTIME)])},
  {"DIRCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_DIR)])},
  {"HOSTCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_HOST)])},
  {"REDIRHOSTCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_REDIRHOST)])},
  {"FAILHOSTCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_FAILHOST)])},
  {"DOMCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_DOM)])},
  {"ORGCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_ORG)])},
  {"REFCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_REF)])},
  {"REFSITECHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_REFSITE)])},
  {"REDIRREFCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_REDIRREF)])},
  {"FAILREFCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_FAILREF)])},
  {"BROWREPCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_BROWREP)])},
  {"BROWSUMCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_BROWSUM)])},
  {"OSCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_OS)])},
  {"VHOSTCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_VHOST)])},
  {"REDIRVHOSTCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_REDIRVHOST)])},
  {"FAILVHOSTCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_FAILVHOST)])},
  {"USERCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_USER)])},
  {"REDIRUSERCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_REDIRUSER)])},
  {"FAILUSERCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_FAILUSER)])},
  {"SEARCHQUERYCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_SEARCHREP)])},
  {"SEARCHWORDCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_SEARCHSUM)])},
  {"INTSEARCHQUERYCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_INTSEARCHREP)])},
  {"INTSEARCHWORDCHART", &configchoice,
   (void *)&(opts.outopts.chartby[G(REP_INTSEARCHSUM)])},
  {"STATUSCHART", &configchoice, (void *)&(opts.outopts.chartby[G(REP_CODE)])},
  {"ALLCHART", &configallchart, (void *)(opts.outopts.chartby)},
  {"ALLCHARTS", &configallchart, (void *)(opts.outopts.chartby)},
  {"REQCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_REQ)])},
  {"REDIRCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_REDIR)])},
  {"FAILCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_FAIL)])},
  {"TYPECHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_TYPE)])},
  {"DIRCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_DIR)])},
  {"DOMCHARTEXPAND", &configstrlist,  /* NB special case in configstrlist */
   (void *)&(opts.outopts.expandhead[G(REP_DOM)])},
  {"ORGCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_ORG)])},
  {"REFCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_REF)])},
  {"REFSITECHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_REFSITE)])},
  {"REDIRREFCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_REDIRREF)])},
  {"FAILREFCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_FAILREF)])},
  {"BROWSUMCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_BROWSUM)])},
  {"BROWCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_BROWSUM)])},
  {"OSCHARTEXPAND", &configstrlist,
   (void *)&(opts.outopts.expandhead[G(REP_OS)])},
  {"CHARTDIR", &configstr, (void *)&(opts.outopts.chartdir)},
  {"LOCALCHARTDIR", &configstr, (void *)&(opts.outopts.localchartdir)},
#ifdef HAVE_GD
  {"JPEGCHARTS", &configchoice, (void *)&(opts.outopts.jpegcharts)},
#endif
#endif  /* NOGRAPHICS */
  {"PNGIMAGES", &configchoice, (void *)&(opts.outopts.pngimages)},
  {"BARSTYLE", &configbarstyle, (void *)&(opts.outopts.barstyle)},
  {"COMPSEP", &configstr, (void *)&(opts.outopts.compsep)},
  {"PRESEP", &configstr, (void *)&(opts.outopts.compsep)},
  {"HOSTNAME", &configstr, (void *)&(opts.outopts.hostname)},
  {"XMLDTD", &configstr, (void *)&(opts.outopts.xmldtd)},
  {"HOSTURL", &configstr, (void *)&(opts.outopts.hosturl)},
  {"LOGO", &configstr, (void *)&(opts.outopts.logo)},
  {"LOGOURL", &configstr, (void *)&(opts.outopts.logourl)},
  {"STYLESHEET", &configstr, (void *)&(opts.outopts.stylesheet)},
  {"CSSPREFIX", &configstr, (void *)&(opts.outopts.cssprefix)},
  {"BASEURL", &configstr, (void *)&(opts.outopts.baseurl)},
  {"ANONYMIZERURL", &configstr, (void *)&(opts.outopts.anonymizerurl)},
  {"LOGFORMAT", &configlogfmt, (void *)&logformat},
  {"DEFAULTLOGFORMAT", &configlogfmt, (void *)&deflogformat},
  {"APACHELOGFORMAT", &configapachelogfmt, (void *)&logformat},
  {"APACHEDEFAULTLOGFORMAT", &configapachelogfmt, (void *)&deflogformat},
  {"TIMEOFFSET", &configoffset, (void *)&stz},
  {"LOGTIMEOFFSET", &configoffset, (void *)&tz},
  {"LOGFILE", &configlogfile, (void *)(opts.miscopts.logfile)},
  {"CACHEFILE", &configcachefile, (void *)(opts.miscopts.logfile)},
  {"IMAGEDIR", &configstr, (void *)&(opts.outopts.imagedir)},
  {"HEADERFILE", &configstr, (void *)&(opts.outopts.headerfile)},
  {"FOOTERFILE", &configstr, (void *)&(opts.outopts.footerfile)},
  {"LANGUAGE", &configlang, (void *)&(opts.outopts.lang)},
  {"LANGFILE", &configstr, (void *)&(opts.outopts.lang.file)},
  {"OUTFILE", &configoutfile, (void *)&(opts.outopts.outfile)},
  {"CACHEOUTFILE", &configstr, (void *)&(opts.outopts.cacheoutfile)},
  {"ERRFILE", &configerrfile, (void *)&errfile},
  {"DOMAINSFILE", &configstr, (void *)&(opts.outopts.domainsfile)},
  {"DESCFILE", &configstr, (void *)&(opts.outopts.descfile)},
  {"DIRSUFFIX", &configstr, (void *)&(opts.miscopts.dirsuffix)},
  {"SEPCHAR", &configchar, (void *)&(opts.outopts.sepchar)},
  {"REPSEPCHAR", &configchar, (void *)&(opts.outopts.repsepchar)},
  {"DECPOINT", &configchar, (void *)&(opts.outopts.decpt)},
  {"DECPT", &configchar, (void *)&(opts.outopts.decpt)},
  {"MARKCHAR", &configchar, (void *)&(opts.outopts.markchar)},
  {"REQFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_REQ)])},
  {"REQARGSFLOOR", &configfloor, (void *)&(opts.outopts.subfloor[G(REP_REQ)])},
  {"REDIRFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_REDIR)])},
  {"REDIRARGSFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_REDIR)])},
  {"FAILFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_FAIL)])},
  {"FAILARGSFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_FAIL)])},
  {"TYPEFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_TYPE)])},
  {"SUBTYPEFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_TYPE)])},
  {"DIRFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_DIR)])},
  {"SUBDIRFLOOR", &configfloor, (void *)&(opts.outopts.subfloor[G(REP_DIR)])},
  {"HOSTFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_HOST)])},
  {"REDIRHOSTFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_REDIRHOST)])},
  {"FAILHOSTFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_FAILHOST)])},
  {"DOMFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_DOM)])},
  {"SUBDOMFLOOR", &configfloor, (void *)&(opts.outopts.subfloor[G(REP_DOM)])},
  {"ORGFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_ORG)])},
  {"SUBORGFLOOR", &configfloor, (void *)&(opts.outopts.subfloor[G(REP_ORG)])},
  {"REFFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_REF)])},
  {"REFARGSFLOOR", &configfloor, (void *)&(opts.outopts.subfloor[G(REP_REF)])},
  {"REFSITEFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_REFSITE)])},
  {"REFDIRFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_REFSITE)])},
  {"SUBREFSITEFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_REFSITE)])},
  {"REDIRREFFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_REDIRREF)])},
  {"REDIRREFARGSFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_REDIRREF)])},
  {"FAILREFFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_FAILREF)])},
  {"FAILREFARGSFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_FAILREF)])},
  {"BROWREPFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_BROWREP)])},
  {"FULLBROWFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_BROWREP)])},
  {"BROWSUMFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_BROWSUM)])},
  {"BROWFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_BROWSUM)])},
  {"SUBBROWFLOOR", &configfloor,
   (void *)&(opts.outopts.subfloor[G(REP_BROWSUM)])},
  {"OSFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_OS)])},
  {"SUBOSFLOOR", &configfloor, (void *)&(opts.outopts.subfloor[G(REP_OS)])},
  {"VHOSTFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_VHOST)])},
  {"REDIRVHOSTFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_REDIRVHOST)])},
  {"FAILVHOSTFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_FAILVHOST)])},
  {"USERFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_USER)])},
  {"REDIRUSERFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_REDIRUSER)])},
  {"FAILUSERFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_FAILUSER)])},
  {"SEARCHQUERYFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_SEARCHREP)])},
  {"SEARCHWORDFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_SEARCHSUM)])},
  {"INTSEARCHQUERYFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_INTSEARCHREP)])},
  {"INTSEARCHWORDFLOOR", &configfloor,
   (void *)&(opts.outopts.floor[G(REP_INTSEARCHSUM)])},
  {"STATUSFLOOR", &configfloor, (void *)&(opts.outopts.floor[G(REP_CODE)])},
  {"FILEALIAS", &configalias, (void *)&(opts.aliashead[ITEM_FILE])},
  {"HOSTALIAS", &configalias, (void *)&(opts.aliashead[ITEM_HOST])},
  {"REFALIAS", &configalias, (void *)&(opts.aliashead[ITEM_REFERRER])},
  {"BROWALIAS", &configalias, (void *)&(opts.aliashead[ITEM_BROWSER])},
  {"USERALIAS", &configalias, (void *)&(opts.aliashead[ITEM_USER])},
  {"VHOSTALIAS", &configalias, (void *)&(opts.aliashead[ITEM_VHOST])},
  {"SEARCHENGINE", &configstrpair2list, (void *)&(opts.searchengines)},
  {"INTSEARCHENGINE", &configstrpair2list, (void *)&(opts.intsearchengines)},
  {"REQALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_REQ)])},
  {"REQOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REQ)])},
  {"REDIRALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIR)])},
  {"REDIROUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIR)])},
  {"FAILALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_FAIL)])},
  {"FAILOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAIL)])},
  {"TYPEALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_TYPE)])},
  {"TYPEOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_TYPE)])},
  {"DIRALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_DIR)])},
  {"DIROUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_DIR)])},
  {"HOSTREPALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_HOST)])},
  {"HOSTOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_HOST)])},
  {"REDIRHOSTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRHOST)])},
  {"REDIRHOSTOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRHOST)])},
  {"FAILHOSTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILHOST)])},
  {"FAILHOSTOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILHOST)])},
  {"DOMALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_DOM)])},
  {"DOMOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_DOM)])},
  {"ORGALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_ORG)])},
  {"ORGOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_ORG)])},
  {"REFREPALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_REF)])},
  {"REFOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REF)])},
  {"REFSITEALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REFSITE)])},
  {"REFSITEOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REFSITE)])},
  {"REDIRREFALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRREF)])},
  {"REDIRREFOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRREF)])},
  {"FAILREFALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILREF)])},
  {"FAILREFOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILREF)])},
  {"BROWREPALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_BROWREP)])},
  {"BROWREPOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_BROWREP)])},
  {"FULLBROWOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_BROWREP)])},
  {"BROWSUMALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_BROWSUM)])},
  {"BROWSUMOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_BROWSUM)])},
  {"BROWOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_BROWSUM)])},
  {"OSALIAS", &configalias, (void *)&(opts.outopts.aliashead[G(REP_OS)])},
  {"OSOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_OS)])},
  {"VHOSTREPALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_VHOST)])},
  {"VHOSTOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_VHOST)])},
  {"REDIRVHOSTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRVHOST)])},
  {"REDIRVHOSTOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRVHOST)])},
  {"FAILVHOSTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILVHOST)])},
  {"FAILVHOSTOUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILVHOST)])},
  {"USERREPALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_USER)])},
  {"USEROUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_USER)])},
  {"REDIRUSERALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRUSER)])},
  {"REDIRUSEROUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_REDIRUSER)])},
  {"FAILUSERALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILUSER)])},
  {"FAILUSEROUTPUTALIAS", &configalias,
   (void *)&(opts.outopts.aliashead[G(REP_FAILUSER)])},
#ifndef NOPIPES
  {"UNCOMPRESS", &configstrpairlist, (void *)&uncompresshead},
#endif
  {"REQARGS", &configtree, (void *)&(opts.outopts.tree[G(REP_REQ)])},
  {"REDIRARGS", &configtree, (void *)&(opts.outopts.tree[G(REP_REDIR)])},
  {"FAILARGS", &configtree, (void *)&(opts.outopts.tree[G(REP_FAIL)])},
  {"REFARGS", &configtree, (void *)&(opts.outopts.tree[G(REP_REF)])},
  {"REDIRREFARGS", &configtree, (void *)&(opts.outopts.tree[G(REP_REDIRREF)])},
  {"FAILREFARGS", &configtree, (void *)&(opts.outopts.tree[G(REP_FAILREF)])},
  {"SUBDOMAIN", &configtree, (void *)&(opts.outopts.tree[G(REP_DOM)])},
  {"SUBDOM", &configtree, (void *)&(opts.outopts.tree[G(REP_DOM)])},
  {"SUBDIR", &configtree, (void *)&(opts.outopts.tree[G(REP_DIR)])},
  {"SUBTYPE", &configtree, (void *)&(opts.outopts.tree[G(REP_TYPE)])},
  {"REFDIR", &configtree, (void *)&(opts.outopts.tree[G(REP_REFSITE)])},
  {"SUBREFSITE", &configtree, (void *)&(opts.outopts.tree[G(REP_REFSITE)])},
  {"SUBBROW", &configtree, (void *)&(opts.outopts.tree[G(REP_BROWSUM)])},
  {"SUBOS", &configtree, (void *)&(opts.outopts.tree[G(REP_OS)])},
  {"SUBORG", &configstrlist, (void *)&(opts.outopts.suborgs)},
  {"SUBORG2", &configtree, (void *)&(opts.outopts.tree[G(REP_ORG)])},
  /* have to postpone SUBORG's until domains file has been read */
  {"FILEINCLUDE", &configinc, (void *)&(opts.wanthead[ITEM_FILE])},
  {"FILEEXCLUDE", &configexc, (void *)&(opts.wanthead[ITEM_FILE])},
  {"HOSTINCLUDE", &confighostinc, (void *)&(opts.wanthead[ITEM_HOST])},
  {"HOSTEXCLUDE", &confighostexc, (void *)&(opts.wanthead[ITEM_HOST])},
  {"REFINCLUDE", &configinc, (void *)&(opts.wanthead[ITEM_REFERRER])},
  {"REFEXCLUDE", &configexc, (void *)&(opts.wanthead[ITEM_REFERRER])},
  {"BROWINCLUDE", &configinc, (void *)&(opts.wanthead[ITEM_BROWSER])},
  {"BROWEXCLUDE", &configexc, (void *)&(opts.wanthead[ITEM_BROWSER])},
  {"USERINCLUDE", &configinc, (void *)&(opts.wanthead[ITEM_USER])},
  {"USEREXCLUDE", &configexc, (void *)&(opts.wanthead[ITEM_USER])},
  {"VHOSTINCLUDE", &configinc, (void *)&(opts.wanthead[ITEM_VHOST])},
  {"VHOSTEXCLUDE", &configexc, (void *)&(opts.wanthead[ITEM_VHOST])},
  {"STATUSINCLUDE", &configscinc, (void *)(opts.code2type)},
  {"STATUSEXCLUDE", &configscexc, (void *)(opts.code2type)},
  {"304ISSUCCESS", &configchoice, (void *)&(opts.succ304)},
  {"REQINCLUDE", &configinc, (void *)&(opts.outopts.wanthead[G(REP_REQ)])},
  {"REQEXCLUDE", &configexc, (void *)&(opts.outopts.wanthead[G(REP_REQ)])},
  {"REDIRINCLUDE", &configinc, (void *)&(opts.outopts.wanthead[G(REP_REDIR)])},
  {"REDIREXCLUDE", &configexc, (void *)&(opts.outopts.wanthead[G(REP_REDIR)])},
  {"FAILINCLUDE", &configinc, (void *)&(opts.outopts.wanthead[G(REP_FAIL)])},
  {"FAILEXCLUDE", &configexc, (void *)&(opts.outopts.wanthead[G(REP_FAIL)])},
  {"TYPEINCLUDE", &configincd, (void *)&(opts.outopts.wanthead[G(REP_TYPE)])},
  {"TYPEEXCLUDE", &configexcd, (void *)&(opts.outopts.wanthead[G(REP_TYPE)])},
  {"DIRINCLUDE", &configincs, (void *)&(opts.outopts.wanthead[G(REP_DIR)])},
  {"DIREXCLUDE", &configexcs, (void *)&(opts.outopts.wanthead[G(REP_DIR)])},
  {"HOSTREPINCLUDE", &confighostinc,
   (void *)&(opts.outopts.wanthead[G(REP_HOST)])},
  {"HOSTREPEXCLUDE", &confighostexc,
   (void *)&(opts.outopts.wanthead[G(REP_HOST)])},
  {"REDIRHOSTINCLUDE", &confighostinc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRHOST)])},
  {"REDIRHOSTEXCLUDE", &confighostexc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRHOST)])},
  {"FAILHOSTINCLUDE", &confighostinc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILHOST)])},
  {"FAILHOSTEXCLUDE", &confighostexc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILHOST)])},
  {"DOMINCLUDE", &configincd, (void *)&(opts.outopts.wanthead[G(REP_DOM)])},
  {"DOMEXCLUDE", &configexcd, (void *)&(opts.outopts.wanthead[G(REP_DOM)])},
  {"ORGINCLUDE", &configinc, (void *)&(opts.outopts.wanthead[G(REP_ORG)])},
  {"ORGEXCLUDE", &configexc, (void *)&(opts.outopts.wanthead[G(REP_ORG)])},
  {"REFREPINCLUDE", &configinc, (void *)&(opts.outopts.wanthead[G(REP_REF)])},
  {"REFREPEXCLUDE", &configexc, (void *)&(opts.outopts.wanthead[G(REP_REF)])},
  {"REFSITEINCLUDE", &configincs,
   (void *)&(opts.outopts.wanthead[G(REP_REFSITE)])},
  {"REFSITEEXCLUDE", &configexcs,
   (void *)&(opts.outopts.wanthead[G(REP_REFSITE)])},
  {"REDIRREFINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRREF)])},
  {"REDIRREFEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRREF)])},
  {"FAILREFINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILREF)])},
  {"FAILREFEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILREF)])},
  {"BROWSUMINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_BROWSUM)])},
  {"BROWSUMEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_BROWSUM)])},
  {"BROWREPINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_BROWREP)])},
  {"BROWREPEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_BROWREP)])},
  {"FULLBROWINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_BROWREP)])},
  {"FULLBROWEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_BROWREP)])},
  {"OSINCLUDE", &configinc, (void *)&(opts.outopts.wanthead[G(REP_OS)])},
  {"OSEXCLUDE", &configexc, (void *)&(opts.outopts.wanthead[G(REP_OS)])},
  {"VHOSTREPINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_VHOST)])},
  {"VHOSTREPEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_VHOST)])},
  {"REDIRVHOSTINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRVHOST)])},
  {"REDIRVHOSTEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRVHOST)])},
  {"FAILVHOSTINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILVHOST)])},
  {"FAILVHOSTEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILVHOST)])},
  {"USERREPINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_USER)])},
  {"USERREPEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_USER)])},
  {"REDIRUSERINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRUSER)])},
  {"REDIRUSEREXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_REDIRUSER)])},
  {"FAILUSERINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILUSER)])},
  {"FAILUSEREXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_FAILUSER)])},
  {"SEARCHQUERYINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_SEARCHREP)])},
  {"SEARCHQUERYEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_SEARCHREP)])},
  {"SEARCHWORDINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_SEARCHSUM)])},
  {"SEARCHWORDEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_SEARCHSUM)])},
  {"INTSEARCHQUERYINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_INTSEARCHREP)])},
  {"INTSEARCHQUERYEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_INTSEARCHREP)])},
  {"INTSEARCHWORDINCLUDE", &configinc,
   (void *)&(opts.outopts.wanthead[G(REP_INTSEARCHSUM)])},
  {"INTSEARCHWORDEXCLUDE", &configexc,
   (void *)&(opts.outopts.wanthead[G(REP_INTSEARCHSUM)])},
  {"LINKINCLUDE", &configinc, (void *)&(opts.outopts.link[G(REP_REQ)])},
  {"LINKEXCLUDE", &configexc, (void *)&(opts.outopts.link[G(REP_REQ)])},
  {"REQLINKINCLUDE", &configinc, (void *)&(opts.outopts.link[G(REP_REQ)])},
  {"REQLINKEXCLUDE", &configexc, (void *)&(opts.outopts.link[G(REP_REQ)])},
  {"REDIRLINKINCLUDE", &configinc, (void *)&(opts.outopts.link[G(REP_REDIR)])},
  {"REDIRLINKEXCLUDE", &configexc, (void *)&(opts.outopts.link[G(REP_REDIR)])},
  {"FAILLINKINCLUDE", &configinc, (void *)&(opts.outopts.link[G(REP_FAIL)])},
  {"FAILLINKEXCLUDE", &configexc, (void *)&(opts.outopts.link[G(REP_FAIL)])},
  {"REFLINKINCLUDE", &configinc, (void *)&(opts.outopts.link[G(REP_REF)])},
  {"REFLINKEXCLUDE", &configexc, (void *)&(opts.outopts.link[G(REP_REF)])},
  {"REDIRREFLINKINCLUDE", &configinc,
   (void *)&(opts.outopts.link[G(REP_REDIRREF)])},
  {"REDIRREFLINKEXCLUDE", &configexc,
   (void *)&(opts.outopts.link[G(REP_REDIRREF)])},
  {"FAILREFLINKINCLUDE", &configinc,
   (void *)&(opts.outopts.link[G(REP_FAILREF)])},
  {"FAILREFLINKEXCLUDE", &configexc,
   (void *)&(opts.outopts.link[G(REP_FAILREF)])},
  {"ARGSINCLUDE", &configinc, (void *)&(opts.argshead)},
  {"ARGSEXCLUDE", &configexc, (void *)&(opts.argshead)},
  {"REFARGSINCLUDE", &configinc, (void *)&(opts.refargshead)},
  {"REFARGSEXCLUDE", &configexc, (void *)&(opts.refargshead)},
  {"PAGEINCLUDE", &configinc, (void *)&(opts.ispagehead)},
  {"PAGEEXCLUDE", &configexc, (void *)&(opts.ispagehead)},
  {"ROBOTINCLUDE", &configinc, (void *)&(opts.robots)},
  {"ROBOTEXCLUDE", &configexc, (void *)&(opts.robots)},
  {"REQSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_REQ)])},
  {"REQARGSSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_REQ)])},
  {"REDIRSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_REDIR)])},
  {"REDIRARGSSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_REDIR)])},
  {"FAILSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_FAIL)])},
  {"FAILARGSSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_FAIL)])},
  {"TYPESORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_TYPE)])},
  {"SUBTYPESORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_TYPE)])},
  {"DIRSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_DIR)])},
  {"SUBDIRSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_DIR)])},
  {"HOSTSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_HOST)])},
  {"REDIRHOSTSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_REDIRHOST)])},
  {"FAILHOSTSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_FAILHOST)])},
  {"DOMSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_DOM)])},
  {"SUBDOMSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_DOM)])},
  {"ORGSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_ORG)])},
  {"SUBORGSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_ORG)])},
  {"REFSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_REF)])},
  {"REFARGSSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_REF)])},
  {"REFSITESORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_REFSITE)])},
  {"REFDIRSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_REFSITE)])},
  {"SUBREFSITESORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_REFSITE)])},
  {"REDIRREFSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_REDIRREF)])},
  {"REDIRREFARGSSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_REDIRREF)])},
  {"FAILREFSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_FAILREF)])},
  {"FAILREFARGSSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_FAILREF)])},
  {"BROWREPSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_BROWREP)])},
  {"FULLBROWSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_BROWREP)])},
  {"BROWSUMSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_BROWSUM)])},
  {"BROWSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_BROWSUM)])},
  {"SUBBROWSORTBY", &configchoice,
   (void *)&(opts.outopts.subsortby[G(REP_BROWSUM)])},
  {"OSSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_OS)])},
  {"SUBOSSORTBY", &configchoice, (void *)&(opts.outopts.subsortby[G(REP_OS)])},
  {"STATUSSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_CODE)])},
  {"VHOSTSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_VHOST)])},
  {"REDIRVHOSTSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_REDIRVHOST)])},
  {"FAILVHOSTSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_FAILVHOST)])},
  {"USERSORTBY", &configchoice, (void *)&(opts.outopts.sortby[G(REP_USER)])},
  {"REDIRUSERSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_REDIRUSER)])},
  {"FAILUSERSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_FAILUSER)])},
  {"SEARCHQUERYSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_SEARCHREP)])},
  {"SEARCHWORDSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_SEARCHSUM)])},
  {"INTSEARCHQUERYSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_INTSEARCHREP)])},
  {"INTSEARCHWORDSORTBY", &configchoice,
   (void *)&(opts.outopts.sortby[G(REP_INTSEARCHSUM)])},
  {"ALL", &configall, (void *)(opts.outopts.repq)},
  {"ALLBACK", &configallback, (void *)(opts.outopts.back)},
  {"YEARLY", &configchoice, (void *)&(opts.outopts.repq[REP_YEAR])},
  {"QUARTERLY", &configchoice, (void *)&(opts.outopts.repq[REP_QUARTERLY])},
  {"MONTHLY", &configchoice, (void *)&(opts.outopts.repq[REP_MONTH])},
  {"WEEKLY", &configchoice, (void *)&(opts.outopts.repq[REP_WEEK])},
  {"DAILYREP", &configchoice, (void *)&(opts.outopts.repq[REP_DAYREP])},
  {"FULLDAILY", &configchoice, (void *)&(opts.outopts.repq[REP_DAYREP])},
  {"DAILYSUM", &configchoice, (void *)&(opts.outopts.repq[REP_DAYSUM])},
  {"DAILY", &configchoice, (void *)&(opts.outopts.repq[REP_DAYSUM])},
  {"HOURLYREP", &configchoice, (void *)&(opts.outopts.repq[REP_HOURREP])},
  {"FULLHOURLY", &configchoice, (void *)&(opts.outopts.repq[REP_HOURREP])},
  {"HOURLYSUM", &configchoice, (void *)&(opts.outopts.repq[REP_HOURSUM])},
  {"HOURLY", &configchoice, (void *)&(opts.outopts.repq[REP_HOURSUM])},
  {"WEEKHOUR", &configchoice, (void *)&(opts.outopts.repq[REP_WEEKHOUR])},
  {"QUARTERREP", &configchoice, (void *)&(opts.outopts.repq[REP_QUARTERREP])},
  {"QUARTER", &configchoice, (void *)&(opts.outopts.repq[REP_QUARTERREP])},
  {"QUARTERSUM", &configchoice, (void *)&(opts.outopts.repq[REP_QUARTERSUM])},
  {"FIVEREP", &configchoice, (void *)&(opts.outopts.repq[REP_FIVEREP])},
  {"FIVE", &configchoice, (void *)&(opts.outopts.repq[REP_FIVEREP])},
  {"FIVESUM", &configchoice, (void *)&(opts.outopts.repq[REP_FIVESUM])},
  {"GENERAL", &configchoice, (void *)&(opts.outopts.repq[REP_GENSUM])},
  {"LASTSEVEN", &configchoice, (void *)&(opts.outopts.last7)},
  {"RUNTIME", &configchoice, (void *)&(opts.outopts.runtime)},
  {"GOTOS", &configchoice, (void *)&(opts.outopts.gotos)},
  {"REPORTSPAN", &configchoice, (void *)&(opts.outopts.repspan)},
  {"REPORTSPANTHRESHOLD", &configuint, (void *)&(opts.outopts.rsthresh)},
  {"RSTHRESH", &configuint, (void *)&(opts.outopts.rsthresh)},
  {"DESCRIPTIONS", &configchoice, (void *)&(opts.outopts.descriptions)},
  {"REQUEST", &configchoice, (void *)&(opts.outopts.repq[REP_REQ])},
  {"REDIR", &configchoice, (void *)&(opts.outopts.repq[REP_REDIR])},
  {"REDIRECTION", &configchoice, (void *)&(opts.outopts.repq[REP_REDIR])},
  {"FAILURE", &configchoice, (void *)&(opts.outopts.repq[REP_FAIL])},
  {"FILETYPE", &configchoice, (void *)&(opts.outopts.repq[REP_TYPE])},
  {"SIZE", &configchoice, (void *)&(opts.outopts.repq[REP_SIZE])},
  {"PROCTIME", &configchoice, (void *)&(opts.outopts.repq[REP_PROCTIME])},
  {"DIRECTORY", &configchoice, (void *)&(opts.outopts.repq[REP_DIR])},
  {"HOST", &configchoice, (void *)&(opts.outopts.repq[REP_HOST])},
  {"REDIRHOST", &configchoice, (void *)&(opts.outopts.repq[REP_REDIRHOST])},
  {"FAILHOST", &configchoice, (void *)&(opts.outopts.repq[REP_FAILHOST])},
  {"DOMAIN", &configchoice, (void *)&(opts.outopts.repq[REP_DOM])},
  {"ORGANISATION", &configchoice, (void *)&(opts.outopts.repq[REP_ORG])},
  {"ORGANIZATION", &configchoice, (void *)&(opts.outopts.repq[REP_ORG])},
  {"REFERRER", &configchoice, (void *)&(opts.outopts.repq[REP_REF])},
  {"REFERER", &configchoice, (void *)&(opts.outopts.repq[REP_REF])},
  {"REFSITE", &configchoice, (void *)&(opts.outopts.repq[REP_REFSITE])},
  {"REDIRREF", &configchoice, (void *)&(opts.outopts.repq[REP_REDIRREF])},
  {"FAILREF", &configchoice, (void *)&(opts.outopts.repq[REP_FAILREF])},
  {"BROWSERREP", &configchoice, (void *)&(opts.outopts.repq[REP_BROWREP])},
  {"BROWREP", &configchoice, (void *)&(opts.outopts.repq[REP_BROWREP])},
  {"FULLBROWSER", &configchoice, (void *)&(opts.outopts.repq[REP_BROWREP])},
  {"BROWSERSUM", &configchoice, (void *)&(opts.outopts.repq[REP_BROWSUM])},
  {"BROWSUM", &configchoice, (void *)&(opts.outopts.repq[REP_BROWSUM])},
  {"BROWSER", &configchoice, (void *)&(opts.outopts.repq[REP_BROWSUM])},
  {"OSREP", &configchoice, (void *)&(opts.outopts.repq[REP_OS])},
  {"VHOST", &configchoice, (void *)&(opts.outopts.repq[REP_VHOST])},
  {"REDIRVHOST", &configchoice, (void *)&(opts.outopts.repq[REP_REDIRVHOST])},
  {"FAILVHOST", &configchoice, (void *)&(opts.outopts.repq[REP_FAILVHOST])},
  {"USER", &configchoice, (void *)&(opts.outopts.repq[REP_USER])},
  {"REDIRUSER", &configchoice, (void *)&(opts.outopts.repq[REP_REDIRUSER])},
  {"FAILUSER", &configchoice, (void *)&(opts.outopts.repq[REP_FAILUSER])},
  {"SEARCHQUERY", &configchoice, (void *)&(opts.outopts.repq[REP_SEARCHREP])},
  {"SEARCHWORD", &configchoice, (void *)&(opts.outopts.repq[REP_SEARCHSUM])},
  {"INTSEARCHQUERY", &configchoice,
   (void *)&(opts.outopts.repq[REP_INTSEARCHREP])},
  {"INTSEARCHWORD", &configchoice,
   (void *)&(opts.outopts.repq[REP_INTSEARCHSUM])},
  {"STATUS", &configchoice, (void *)&(opts.outopts.repq[REP_CODE])},
  {"YEARBACK", &configchoice, (void *)&(opts.outopts.back[REP_YEAR])},
  {"QUARTERLYBACK", &configchoice,
   (void *)&(opts.outopts.back[REP_QUARTERLY])},
  {"MONTHBACK", &configchoice, (void *)&(opts.outopts.back[REP_MONTH])},
  {"WEEKBACK", &configchoice, (void *)&(opts.outopts.back[REP_WEEK])},
  {"DAYBACK", &configchoice, (void *)&(opts.outopts.back[REP_DAYREP])},
  {"DAYREPBACK", &configchoice, (void *)&(opts.outopts.back[REP_DAYREP])},
  {"FULLDAYBACK", &configchoice, (void *)&(opts.outopts.back[REP_DAYREP])},
  {"HOURBACK", &configchoice, (void *)&(opts.outopts.back[REP_HOURREP])},
  {"HOURREPBACK", &configchoice, (void *)&(opts.outopts.back[REP_HOURREP])},
  {"FULLHOURBACK", &configchoice, (void *)&(opts.outopts.back[REP_HOURREP])},
  {"QUARTERBACK", &configchoice, (void *)&(opts.outopts.back[REP_QUARTERREP])},
  {"QUARTERREPBACK", &configchoice,
   (void *)&(opts.outopts.back[REP_QUARTERREP])},
  {"FIVEBACK", &configchoice, (void *)&(opts.outopts.back[REP_FIVEREP])},
  {"FIVEREPBACK", &configchoice, (void *)&(opts.outopts.back[REP_FIVEREP])},
  {"RAWBYTES", &configchoice, (void *)&(opts.outopts.rawbytes)},
  {"BYTESDP", &configuint, (void *)&(opts.outopts.bytesdp)},
  {"NOROBOTS", &configchoice, (void *)&(opts.outopts.norobots)},
  {"LINKNOFOLLOW", &configchoice, (void *)&(opts.outopts.linknofollow)},
  {"SETTINGS", &configchoice, (void *)&vblesonly},
  {"CGI", &configchoice, (void *)&cgi},
  {"OUTPUT", &configchoice, (void *)&(opts.outopts.outstyle)},
  {"PDFLATEX", &configchoice, (void *)&(opts.outopts.pdflatex)},
  {"CASE", &configchoice, (void *)&(opts.miscopts.case_insensitive)},
  {"USERCASE", &configchoice, (void *)&(opts.miscopts.usercase_insensitive)},
  {"SEARCHCHARCONVERT", &configchoice, (void *)&(opts.outopts.searchconv)},
  {"YEARROWS", &configuint, (void *)&(opts.outopts.rows[REP_YEAR])},
  {"QUARTERLYROWS", &configuint, (void *)&(opts.outopts.rows[REP_QUARTERLY])},
  {"MONTHROWS", &configuint, (void *)&(opts.outopts.rows[REP_MONTH])},
  {"WEEKROWS", &configuint, (void *)&(opts.outopts.rows[REP_WEEK])},
  {"DAYROWS", &configuint, (void *)&(opts.outopts.rows[REP_DAYREP])},
  {"DAYREPROWS", &configuint, (void *)&(opts.outopts.rows[REP_DAYREP])},
  {"FULLDAYROWS", &configuint, (void *)&(opts.outopts.rows[REP_DAYREP])},
  {"HOURROWS", &configuint, (void *)&(opts.outopts.rows[REP_HOURREP])},
  {"HOURREPROWS", &configuint, (void *)&(opts.outopts.rows[REP_HOURREP])},
  {"FULLHOURROWS", &configuint, (void *)&(opts.outopts.rows[REP_HOURREP])},
  {"QUARTERROWS", &configuint, (void *)&(opts.outopts.rows[REP_QUARTERREP])},
  {"QUARTERREPROWS", &configuint,
   (void *)&(opts.outopts.rows[REP_QUARTERREP])},
  {"FIVEROWS", &configuint, (void *)&(opts.outopts.rows[REP_FIVEREP])},
  {"FIVEREPROWS", &configuint, (void *)&(opts.outopts.rows[REP_FIVEREP])},
  {"FILELOWMEM", &configlowmem, (void *)&(opts.miscopts.lowmem[ITEM_FILE])},
  {"HOSTLOWMEM", &configlowmem, (void *)&(opts.miscopts.lowmem[ITEM_HOST])},
  {"REFLOWMEM", &configlowmem, (void *)&(opts.miscopts.lowmem[ITEM_REFERRER])},
  {"BROWLOWMEM", &configlowmem, (void *)&(opts.miscopts.lowmem[ITEM_BROWSER])},
  {"USERLOWMEM", &configlowmem, (void *)&(opts.miscopts.lowmem[ITEM_USER])},
  {"VHOSTLOWMEM", &configlowmem, (void *)&(opts.miscopts.lowmem[ITEM_VHOST])},
  {"ASCIIPAGEWIDTH", &configuint, (void *)&(opts.outopts.plainpagewidth)},
  {"PLAINPAGEWIDTH", &configuint, (void *)&(opts.outopts.plainpagewidth)},
  {"HTMLPAGEWIDTH", &configuint, (void *)&(opts.outopts.htmlpagewidth)},
  {"LATEXPAGEWIDTH", &configuint, (void *)&(opts.outopts.latexpagewidth)},
  {"MINGRAPHWIDTH", &configuint, (void *)&(opts.outopts.mingraphwidth)},
  {"ERRLINELENGTH", &configuint, (void *)&errwidth},
  {"PROGRESSFREQ", &configulong, (void *)&progressfreq},
  {"WEEKBEGINSON", &configchoice, (void *)&(opts.outopts.weekbeginson)},
  {"FROM", &configstr, (void *)&(opts.dman.fromstr)},
  {"TO", &configstr, (void *)&(opts.dman.tostr)},
  {"REPORTORDER", &configrepord, (void *)(opts.outopts.reporder)},
  {"DEBUG", &configdebug, (void *)&debug_args},
  {"WARNINGS", &configdebug, (void *)&warn_args},
  {"GENSUMLINES", &configdebug, (void *)&(opts.outopts.gensumlines)},
  {"DOMLEVEL", &configdomlevel, (void *)&domlevels},
#ifndef NODNS
  {"DNS", &configchoice, (void *)&dnslevel},
  {"DNSFILE", &configstr, (void *)&dnsfile},
  {"DNSLOCKFILE", &configstr, (void *)&dnslockfile},
  {"DNSGOODHOURS", &configuint, (void *)&dnsgoodhrs},
  {"DNSBADHOURS", &configuint, (void *)&dnsbadhrs},
#ifndef NOALARM
  {"DNSTIMEOUT", &configuint, (void *)&dnstimeout},
#endif
#endif
  {"CONFIGFILE", &configcall, NULL},
  {"", NULL, NULL}  /* at end as marker */
};

void globals(char *comname) {
#ifndef NOFOLLOW
  struct stat buf;
  char *path, *pathcp, *s, *t = NULL;
  size_t l = 0;
  logical done;
#endif
  int i;

  time(&starttime);
  starttimestr = ctime(&starttime);
  starttimestr[24] = '\0';
  (void)parsedate(starttime, "-00-00-00:-00-00", &starttimec, FALSE, FALSE);
  (void)parsedate(starttime, "-00-00-00:-00-00", &starttimeuxc, FALSE, TRUE);
  /* parsedate() is needed because we don't know the encoding of time() */
  errfile = stderr;
  setvbuf(stderr, NULL, ERRBUFMODE, BUFSIZ);
  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);
#ifdef SIGHUP
  signal(SIGHUP, sighandler);
#endif
#ifdef SIGQUIT
  signal(SIGQUIT, sighandler);
#endif
#ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif
  workspace = (char *)xmalloc(BLOCKSIZE);
  block_start = (char *)xmalloc(BLOCKSIZE + 1);  /* 1 for terminating char */
  for (i = 0; i < ITEM_NUMBER; i++) {
    mm[i].curr_pos = NULL;
    mm[i].first = NULL;
    mm[i].alignment = 1;
  }
  mmq.curr_pos = NULL;
  mmq.first = NULL;
  mmq.alignment = 1;
  mms.curr_pos = NULL;
  mms.first = NULL;
  mms.alignment = 1;
  xmemman = (Memman *)xmalloc(sizeof(Memman));
  xmemman->curr_pos = NULL;
  xmemman->first = NULL;
  xmemman->alignment = ALIGNSIZE;
  amemman = (Memman *)xmalloc(sizeof(Memman));
  amemman->curr_pos = NULL;
  amemman->first = NULL;
  amemman->alignment = ALIGNSIZE;
  for (i = 0; i < 256; i++)
    termchar[i] = FALSE;
  termchar[(unsigned char)'\r'] = TRUE;
  termchar[(unsigned char)'\n'] = TRUE;
  termchar[(unsigned char)'\0'] = TRUE;

  if (IS_EMPTY_STRING(comname))
    COPYSTR(commandname, "analog")  /* no semicolon by defn of COPYSTR */
  else
    COPYSTR(commandname, comname);
#ifndef NOFOLLOW
  commandpath = (char *)xmalloc(PATH_MAX);
  done = FALSE;
  if (strpbrk(commandname, PATHSEPS) == NULL &&
      (path = getenv("PATH")) != NULL) {
    pathcp = (char *)xmalloc(strlen(path) + 1);
    strcpy(pathcp, path);
    for (s = strtok(pathcp, ":"); !done && s != NULL; s = strtok(NULL, ":")) {
      ENSURE_LEN(t, l, strlen(s) + strlen(commandname) + 2);
      sprintf(t, "%s%c%s", s, DIRSEP, commandname);
      if (realpath(t, commandpath) != NULL && access(commandpath, X_OK) == 0 &&
	  stat(commandpath, &buf) == 0 && S_ISREG(buf.st_mode))
	done = TRUE;
    }
  }
  else if (realpath(commandname, commandpath) != NULL)
    done = TRUE;
  if (!done)
    strcpy(commandpath, commandname);
  commandpath = (char *)xrealloc((void *)commandpath, strlen(commandpath) + 1);
#else
  commandpath = commandname;
#endif

  unwanted_entry = newhashentry(DATA_NUMBER, FALSE);
  blank_entry = newhashentry(DATA_NUMBER, FALSE);
  dummy_item = (Hashindex *)submalloc(xmemman, sizeof(Hashindex));
  dummy_item->own = newhashentry(DATA_NUMBER, FALSE);
  rep2lng = (unsigned int *)xmalloc(REP_NUMBER * sizeof(unsigned int));
  rep2lng[REP_GENSUM] = gensum_;
  rep2lng[REP_YEAR] = yearrep_;
  rep2lng[REP_QUARTERLY] = quarterlyrep_;
  rep2lng[REP_MONTH] = monthrep_;
  rep2lng[REP_WEEK] = weekrep_;
  rep2lng[REP_DAYREP] = dayrep_;
  rep2lng[REP_DAYSUM] = daysum_;
  rep2lng[REP_HOURREP] = hourrep_;
  rep2lng[REP_HOURSUM] = hoursum_;
  rep2lng[REP_WEEKHOUR] = weekhoursum_;
  rep2lng[REP_QUARTERREP] = quarterrep_;
  rep2lng[REP_QUARTERSUM] = quartersum_;
  rep2lng[REP_FIVEREP] = fiverep_;
  rep2lng[REP_FIVESUM] = fivesum_;
  rep2lng[REP_REQ] = reqrep_;
  rep2lng[REP_REDIR] = redirrep_;
  rep2lng[REP_FAIL] = failrep_;
  rep2lng[REP_TYPE] = typerep_;
  rep2lng[REP_SIZE] = sizerep_;
  rep2lng[REP_PROCTIME] = proctimerep_;
  rep2lng[REP_DIR] = dirrep_;
  rep2lng[REP_HOST] = hostrep_;
  rep2lng[REP_REDIRHOST] = redirhostrep_;
  rep2lng[REP_FAILHOST] = failhostrep_;
  rep2lng[REP_DOM] = domrep_;
  rep2lng[REP_ORG] = orgrep_;
  rep2lng[REP_REF] = refrep_;
  rep2lng[REP_REFSITE] = refsiterep_;
  rep2lng[REP_REDIRREF] = redirrefrep_;
  rep2lng[REP_FAILREF] = failrefrep_;
  rep2lng[REP_BROWREP] = browrep_;
  rep2lng[REP_BROWSUM] = browsum_;
  rep2lng[REP_OS] = osrep_;
  rep2lng[REP_VHOST] = vhostrep_;
  rep2lng[REP_REDIRVHOST] = redirvhostrep_;
  rep2lng[REP_FAILVHOST] = failvhostrep_;
  rep2lng[REP_USER] = userrep_;
  rep2lng[REP_REDIRUSER] = rediruserrep_;
  rep2lng[REP_FAILUSER] = failuserrep_;
  rep2lng[REP_SEARCHREP] = searchrep_;
  rep2lng[REP_SEARCHSUM] = searchsum_;
  rep2lng[REP_INTSEARCHREP] = intsearchrep_;
  rep2lng[REP_INTSEARCHSUM] = intsearchsum_;
  rep2lng[REP_CODE] = statrep_;
  rep2type = (choice *)xmalloc(REP_NUMBER * sizeof(choice));
  rep2type[REP_YEAR] = INP_DATE;
  rep2type[REP_QUARTERLY] = INP_DATE;
  rep2type[REP_MONTH] = INP_DATE;
  rep2type[REP_WEEK] = INP_DATE;
  rep2type[REP_DAYREP] = INP_DATE;
  rep2type[REP_DAYSUM] = INP_DATE;
  rep2type[REP_HOURREP] = INP_DATE;
  rep2type[REP_HOURSUM] = INP_DATE;
  rep2type[REP_WEEKHOUR] = INP_DATE;
  rep2type[REP_QUARTERREP] = INP_DATE;
  rep2type[REP_QUARTERSUM] = INP_DATE;
  rep2type[REP_FIVEREP] = INP_DATE;
  rep2type[REP_FIVESUM] = INP_DATE;
  rep2type[REP_GENSUM] = UNSET;
  rep2type[REP_REQ] = ITEM_FILE;
  rep2type[REP_TYPE] = ITEM_FILE;
  rep2type[REP_DIR] = ITEM_FILE;
  rep2type[REP_REDIR] = ITEM_FILE;
  rep2type[REP_FAIL] = ITEM_FILE;
  rep2type[REP_HOST] = ITEM_HOST;
  rep2type[REP_REDIRHOST] = ITEM_HOST;
  rep2type[REP_FAILHOST] = ITEM_HOST;
  rep2type[REP_DOM] = ITEM_HOST;
  rep2type[REP_ORG] = ITEM_HOST;
  rep2type[REP_USER] = ITEM_USER;
  rep2type[REP_REDIRUSER] = ITEM_USER;
  rep2type[REP_FAILUSER] = ITEM_USER;
  rep2type[REP_REF] = ITEM_REFERRER;
  rep2type[REP_REFSITE] = ITEM_REFERRER;
  rep2type[REP_REDIRREF] = ITEM_REFERRER;
  rep2type[REP_FAILREF] = ITEM_REFERRER;
  rep2type[REP_SEARCHREP] = ITEM_REFERRER;
  rep2type[REP_SEARCHSUM] = ITEM_REFERRER;
  rep2type[REP_INTSEARCHREP] = ITEM_FILE;
  rep2type[REP_INTSEARCHSUM] = ITEM_FILE;
  rep2type[REP_BROWREP] = ITEM_BROWSER;
  rep2type[REP_BROWSUM] = ITEM_BROWSER;
  rep2type[REP_OS] = ITEM_BROWSER;
  rep2type[REP_VHOST] = ITEM_VHOST;
  rep2type[REP_REDIRVHOST] = ITEM_VHOST;
  rep2type[REP_FAILVHOST] = ITEM_VHOST;
  rep2type[REP_SIZE] = INP_BYTES;
  rep2type[REP_PROCTIME] = INP_PROCTIME;
  rep2type[REP_CODE] = INP_CODE;
  repistree = (logical *)xmalloc(GENREP_NUMBER * sizeof(logical));
  for (i = 0; i < GENREP_NUMBER; i++)
    repistree[i] = FALSE;
  repistree[G(REP_REQ)] = TRUE;
  repistree[G(REP_REDIR)] = TRUE;
  repistree[G(REP_FAIL)] = TRUE;
  repistree[G(REP_REF)] = TRUE;
  repistree[G(REP_REDIRREF)] = TRUE;
  repistree[G(REP_FAILREF)] = TRUE;
  repistree[G(REP_TYPE)] = TRUE;
  repistree[G(REP_DIR)] = TRUE;
  repistree[G(REP_DOM)] = TRUE;
  repistree[G(REP_ORG)] = TRUE;
  repistree[G(REP_REFSITE)] = TRUE;
  repistree[G(REP_BROWSUM)] = TRUE;
  repistree[G(REP_OS)] = TRUE;
  rep2reqs = (choice *)xmalloc(GENREP_NUMBER * sizeof(choice));
  rep2reqs7 = (choice *)xmalloc(GENREP_NUMBER * sizeof(choice));
  rep2date = (choice *)xmalloc(GENREP_NUMBER * sizeof(choice));
  rep2firstd = (choice *)xmalloc(GENREP_NUMBER * sizeof(choice));
  for (i = 0; i < GENREP_NUMBER; i++)
    rep2reqs[i] = REQUESTS;
  rep2reqs[G(REP_REDIR)] = REDIR;
  rep2reqs[G(REP_FAIL)] = FAIL;
  rep2reqs[G(REP_REDIRHOST)] = REDIR;
  rep2reqs[G(REP_FAILHOST)] = FAIL;
  rep2reqs[G(REP_REDIRUSER)] = REDIR;
  rep2reqs[G(REP_FAILUSER)] = FAIL;
  rep2reqs[G(REP_REDIRREF)] = REDIR;
  rep2reqs[G(REP_FAILREF)] = FAIL;
  rep2reqs[G(REP_REDIRVHOST)] = REDIR;
  rep2reqs[G(REP_FAILVHOST)] = FAIL;
  for (i = 0; i < GENREP_NUMBER; i++) {
    rep2reqs7[i] = (rep2reqs[i] == REDIR)?REDIR7:\
      ((rep2reqs[i] == FAIL)?FAIL7:REQUESTS7);
    rep2date[i] = (rep2reqs[i] == REDIR)?REDIRDATE:\
      ((rep2reqs[i] == FAIL)?FAILDATE:SUCCDATE);
    rep2firstd[i] = (rep2reqs[i] == REDIR)?REDIRFIRSTD:\
      ((rep2reqs[i] == FAIL)?FAILFIRSTD:SUCCFIRSTD);
  }
  rep2gran = (unsigned int *)xmalloc(DATEREP_NUMBER * sizeof(unsigned int));
  rep2gran[REP_DAYREP] = 1;
  rep2gran[REP_HOURREP] = 24;
  rep2gran[REP_QUARTERREP] = 96;
  rep2gran[REP_FIVEREP] = 288;
  rep2gran[REP_YEAR] = 1;
  rep2gran[REP_QUARTERLY] = 1;
  rep2gran[REP_MONTH] = 1;
  rep2gran[REP_WEEK] = 1;
  rep2gran[REP_DAYSUM] = 1;
  rep2gran[REP_HOURSUM] = 24;
  rep2gran[REP_WEEKHOUR] = 24;
  rep2gran[REP_QUARTERSUM] = 96;
  rep2gran[REP_FIVESUM] = 288;
  rep2datefmt = (unsigned int *)xmalloc(DATEREP_NUMBER * sizeof(unsigned int));
  rep2datefmt[REP_DAYREP] = dayrepfmt_;
  rep2datefmt[REP_DAYSUM] = daysumfmt_;
  rep2datefmt[REP_HOURREP] = hourrepfmt_;
  rep2datefmt[REP_QUARTERREP] = quarterrepfmt_;
  rep2datefmt[REP_FIVEREP] = quarterrepfmt_;
  rep2datefmt[REP_WEEK] = weekfmt_;
  rep2datefmt[REP_MONTH] = monthfmt_;
  rep2datefmt[REP_QUARTERLY] = quarterlyfmt_;
  rep2datefmt[REP_YEAR] = yearfmt_;
  rep2datefmt[REP_HOURSUM] = hoursumfmt_;
  rep2datefmt[REP_WEEKHOUR] = weekhoursumfmt_;
  rep2datefmt[REP_QUARTERSUM] = quartersumfmt_;
  rep2datefmt[REP_FIVESUM] = quartersumfmt_;
  rep2colhead = (unsigned int *)xmalloc(REP_NUMBER * sizeof(unsigned int));
  rep2colhead[REP_DAYREP] = date_;
  rep2colhead[REP_DAYSUM] = day_;
  rep2colhead[REP_HOURREP] = time_;
  rep2colhead[REP_HOURSUM] = hr_;
  rep2colhead[REP_WEEKHOUR] = hr_;
  rep2colhead[REP_QUARTERREP] = time_;
  rep2colhead[REP_QUARTERSUM] = time_;
  rep2colhead[REP_FIVEREP] = time_;
  rep2colhead[REP_FIVESUM] = time_;
  rep2colhead[REP_WEEK] = weekbeg_;
  rep2colhead[REP_MONTH] = month_;
  rep2colhead[REP_QUARTERLY] = quarteryr_;
  rep2colhead[REP_YEAR] = year_;
  rep2colhead[REP_REQ] = file_;
  rep2colhead[REP_REDIR] = file_;
  rep2colhead[REP_FAIL] = file_;
  rep2colhead[REP_TYPE] = ext_;
  rep2colhead[REP_SIZE] = size_;
  rep2colhead[REP_PROCTIME] = seconds_;
  rep2colhead[REP_DIR] = dir_;
  rep2colhead[REP_HOST] = host_;
  rep2colhead[REP_REDIRHOST] = host_;
  rep2colhead[REP_FAILHOST] = host_;
  rep2colhead[REP_DOM] = dom_;
  rep2colhead[REP_ORG] = org_;
  rep2colhead[REP_REF] = url_;
  rep2colhead[REP_REFSITE] = site_;
  rep2colhead[REP_REDIRREF] = url_;
  rep2colhead[REP_FAILREF] = url_;
  rep2colhead[REP_BROWREP] = browser_;
  rep2colhead[REP_BROWSUM] = browser_;
  rep2colhead[REP_OS] = os_;
  rep2colhead[REP_VHOST] = vhost_;
  rep2colhead[REP_REDIRVHOST] = vhost_;
  rep2colhead[REP_FAILVHOST] = vhost_;
  rep2colhead[REP_USER] = user_;
  rep2colhead[REP_REDIRUSER] = user_;
  rep2colhead[REP_FAILUSER] = user_;
  rep2colhead[REP_SEARCHREP] = search_;
  rep2colhead[REP_SEARCHSUM] = search_;
  rep2colhead[REP_INTSEARCHREP] = search_;
  rep2colhead[REP_INTSEARCHSUM] = search_;
  rep2colhead[REP_CODE] = code_;
  rep2busystr = (unsigned int *)xmalloc(DATEREPORTS_NUMBER * sizeof(unsigned int));
  rep2busystr[REP_DAYREP] = busyday_;
  rep2busystr[REP_HOURREP] = busyhour_;
  rep2busystr[REP_QUARTERREP] = busyquarterhr_;
  rep2busystr[REP_FIVEREP] = busyfive_;
  rep2busystr[REP_WEEK] = busyweek_;
  rep2busystr[REP_MONTH] = busymonth_;
  rep2busystr[REP_QUARTERLY] = busyquarteryr_;
  rep2busystr[REP_YEAR] = busyyear_;
  col2colhead = (unsigned int *)xmalloc(COL_NUMBER * sizeof(unsigned int));
  col2colhead[COL_REQS] = nreqs_;
  col2colhead[COL_PAGES] = npgs_;
  col2colhead[COL_BYTES] = nbytes_;
  col2colhead[COL_PREQS] = preqs_;
  col2colhead[COL_PPAGES] = ppgs_;
  col2colhead[COL_PBYTES] = pbytes_;
  col2colhead[COL_REQS7] = nreqs7_;
  col2colhead[COL_PAGES7] = npgs7_;
  col2colhead[COL_BYTES7] = nbytes7_;
  col2colhead[COL_PREQS7] = preqs7_;
  col2colhead[COL_PPAGES7] = ppgs7_;
  col2colhead[COL_PBYTES7] = pbytes7_;
  col2colhead[COL_DATE] = lastdate_;
  col2colhead[COL_TIME] = lasttime_;
  col2colhead[COL_FIRSTD] = firstdate_;
  col2colhead[COL_FIRSTT] = firsttime_;
  col2colhead[COL_INDEX] = index_;
  method2sing = (unsigned int *)xmalloc(METHOD_NUMBER * sizeof(unsigned int));
  method2sing[REQUESTS] = request_;
  method2sing[REQUESTS7] = request7_;
  method2sing[PAGES] = pagereq_;
  method2sing[PAGES7] = pagereq7_;
  method2sing[REDIR] = redirreq_;
  method2sing[REDIR7] = redirreq7_;
  method2sing[FAIL] = failreq_;
  method2sing[FAIL7] = failreq7_;
  method2sing[BYTES] = bytetraffic_;
  method2sing[BYTES7] = bytetraffic7_;
  method2pl = (unsigned int *)xmalloc(METHOD_NUMBER * sizeof(unsigned int));
  method2pl[REQUESTS] = requests_;
  method2pl[REQUESTS7] = requests7_;
  method2pl[PAGES] = pagereqs_;
  method2pl[PAGES7] = pagereqs7_;
  method2pl[REDIR] = redirreqs_;
  method2pl[REDIR7] = redirreqs7_;
  method2pl[FAIL] = failreqs_;
  method2pl[FAIL7] = failreqs7_;
  method2pl[BYTES] = bytestraffic_;
  method2pl[BYTES7] = bytestraffic7_;
  method2date = (unsigned int *)xmalloc(METHOD_NUMBER * sizeof(unsigned int));
  method2date[SUCCDATE] = sincedate_;
  method2date[REDIRDATE] = sinceredirdate_;
  method2date[FAILDATE] = sincefaildate_;
  method2date[SUCCFIRSTD] = sincefirstd_;
  method2date[REDIRFIRSTD] = sinceredirfirstd_;
  method2date[FAILFIRSTD] = sincefailfirstd_;
  method2pc = (unsigned int *)xmalloc(METHOD_NUMBER * sizeof(unsigned int));
  method2pc[REQUESTS] = prequests_;
  method2pc[REQUESTS7] = prequests7_;
  method2pc[PAGES] = ppages_;
  method2pc[PAGES7] = ppages7_;
  method2pc[REDIR] = predirs_;
  method2pc[REDIR7] = predirs7_;
  method2pc[FAIL] = pfails_;
  method2pc[FAIL7] = pfails7_;
  method2pc[BYTES] = ptraffic_;
  method2pc[BYTES7] = ptraffic7_;
  method2relpc = (unsigned int *)xmalloc(METHOD_NUMBER * sizeof(unsigned int));
  method2relpc[REQUESTS] = pmrequests_;
  method2relpc[REQUESTS7] = pmrequests7_;
  method2relpc[PAGES] = pmpages_;
  method2relpc[PAGES7] = pmpages7_;
  method2relpc[REDIR] = pmredirs_;
  method2relpc[REDIR7] = pmredirs7_;
  method2relpc[FAIL] = pmfails_;
  method2relpc[FAIL7] = pmfails7_;
  method2relpc[BYTES] = pmtraffic_;
  method2relpc[BYTES7] = pmtraffic7_;
  method2sort = (unsigned int *)xmalloc(METHOD_NUMBER * sizeof(unsigned int));
  method2sort[REQUESTS] = nrequests_;
  method2sort[REQUESTS7] = nrequests7_;
  method2sort[PAGES] = npages_;
  method2sort[PAGES7] = npages7_;
  method2sort[REDIR] = nredirs_;
  method2sort[REDIR7] = nredirs7_;
  method2sort[FAIL] = nfails_;
  method2sort[FAIL7] = nfails7_;
  method2sort[SUCCDATE] = succdate_;
  method2sort[REDIRDATE] = redirdate_;
  method2sort[FAILDATE] = faildate_;
  method2sort[SUCCFIRSTD] = succfirstd_;
  method2sort[REDIRFIRSTD] = redirfirstd_;
  method2sort[FAILFIRSTD] = failfirstd_;
  method2sort[BYTES] = traffic_;
  method2sort[BYTES7] = traffic7_;
}

void defaults(Options *op) {
  double codethresh[] = {100., 101., 199., 200., 201., 202., 203., 204., 205.,
			 206., 299., 300., 301., 302., 303., 304., 305., 306.,
			 307., 399., 400., 401., 402., 403., 404., 405., 406.,
			 407., 408., 409., 410., 411., 412., 413., 414., 415.,
			 416., 417., 499., 500., 501., 502., 503.,
			 504., 505., 506., 599., -1.};
  double sizethresh[] = {0., 10., 100., 1024., 10240., 102400., 1048576.,
			 10485760., 104857600., 1073741824., -1.};
  double ptthresh[] = {0., 10., 20., 50., 100., 200., 500., 1000., 2000.,
		       5000., 10000., 20000., 60000., 120000., 300000.,
		       600000., -1.};
  /* These thresholds have to be coordinated with the names in arrayrep().
     The max codethresh & ptthresh also feature near the end of readcache(). */
  double *thresh;
  int i, j, k;

  op->conffilelist = NULL;
  confline(op, "DEBUG", "OFF", NULL, -1);
  confline(op, "WARNINGS", "ON", NULL, -1);
  op->outopts.gensumlines = NULL;
  confline(op, "GENSUMLINES", "ON", NULL, -1);
  deflogformat = (Inputformatlist *)submalloc(xmemman,
					      sizeof(Inputformatlist));
  logformat = (Inputformatlist *)submalloc(xmemman, sizeof(Inputformatlist));
  deflogformat->used = TRUE;
  logformat->used = TRUE;
  configlogfmt((void *)&deflogformat, "DEFAULTLOGFORMAT", "AUTO", NULL, -1);
  configlogfmt((void *)&logformat, "LOGFORMAT", "DEFAULT", NULL, -1);
  deflogformat->used = TRUE;
  op->miscopts.logfile[0] = NULL;
  op->miscopts.logfile[1] = NULL;
  confline(op, "LOGFILE", LOGFILE, NULL, -1);
  confline(op, "CACHEFILE", CACHEFILE, NULL, -1);
  op->outopts.domainsfile = NULL;
  op->outopts.descfile = NULL;
  COPYSTR(op->miscopts.dirsuffix, DIRSUFFIX);
#ifndef NODNS
  dnslevel = DNS;
  dnsgoodhrs = DNSGOODHOURS;
  dnsbadhrs = DNSBADHOURS;
  COPYSTR(dnsfile, DNSFILE);
  COPYSTR(dnslockfile, DNSLOCKFILE);
#ifndef NOALARM
  dnstimeout = DNSTIMEOUT;
#endif
#endif
  op->miscopts.case_insensitive = CASE_INSENSITIVE;
  op->miscopts.usercase_insensitive = USERCASE_INSENSITIVE;
  op->outopts.searchconv = SEARCHCHARCONVERT;
  op->outopts.outstyle = OUTPUT;
  op->outopts.html = TRUE;
  op->outopts.pdflatex = TRUE;
  COPYSTR(op->outopts.headerfile, HEADERFILE);
  COPYSTR(op->outopts.footerfile, FOOTERFILE);
  COPYSTR(op->outopts.outfile, OUTFILE);
  COPYSTR(op->outopts.cacheoutfile, CACHEOUTFILE);
  COPYSTR(op->outopts.imagedir, IMAGEDIR);
  op->outopts.chartdir = NULL;
  op->outopts.localchartdir = NULL;
  op->outopts.jpegcharts = FALSE;
  op->outopts.pngimages = PNGIMAGES;
  confline(op, "YEARCOLS", YEARCOLS, NULL, -1);
  confline(op, "QUARTERLYCOLS", QUARTERLYCOLS, NULL, -1);
  confline(op, "MONTHCOLS", MONTHCOLS, NULL, -1);
  confline(op, "WEEKCOLS", WEEKCOLS, NULL, -1);
  confline(op, "DAYREPCOLS", DAYREPCOLS, NULL, -1);
  confline(op, "DAYSUMCOLS", DAYSUMCOLS, NULL, -1);
  confline(op, "HOURREPCOLS", HOURREPCOLS, NULL, -1);
  confline(op, "HOURSUMCOLS", HOURSUMCOLS, NULL, -1);
  confline(op, "WEEKHOURCOLS", WEEKHOURCOLS, NULL, -1);
  confline(op, "QUARTERREPCOLS", QUARTERREPCOLS, NULL, -1);
  confline(op, "QUARTERSUMCOLS", QUARTERSUMCOLS, NULL, -1);
  confline(op, "FIVEREPCOLS", FIVEREPCOLS, NULL, -1);
  confline(op, "FIVESUMCOLS", FIVESUMCOLS, NULL, -1);
  confline(op, "REQCOLS", REQCOLS, NULL, -1);
  confline(op, "REDIRCOLS", REDIRCOLS, NULL, -1);
  confline(op, "FAILCOLS", FAILCOLS, NULL, -1);
  confline(op, "TYPECOLS", TYPECOLS, NULL, -1);
  confline(op, "SIZECOLS", SIZECOLS, NULL, -1);
  confline(op, "PROCTIMECOLS", PROCTIMECOLS, NULL, -1);
  confline(op, "DIRCOLS", DIRCOLS, NULL, -1);
  confline(op, "HOSTCOLS", HOSTCOLS, NULL, -1);
  confline(op, "REDIRHOSTCOLS", REDIRHOSTCOLS, NULL, -1);
  confline(op, "FAILHOSTCOLS", FAILHOSTCOLS, NULL, -1);
  confline(op, "DOMCOLS", DOMCOLS, NULL, -1);
  confline(op, "ORGCOLS", ORGCOLS, NULL, -1);
  confline(op, "REFCOLS", REFCOLS, NULL, -1);
  confline(op, "REFSITECOLS", REFSITECOLS, NULL, -1);
  confline(op, "REDIRREFCOLS", REDIRREFCOLS, NULL, -1);
  confline(op, "FAILREFCOLS", FAILREFCOLS, NULL, -1);
  confline(op, "BROWREPCOLS", BROWREPCOLS, NULL, -1);
  confline(op, "BROWSUMCOLS", BROWSUMCOLS, NULL, -1);
  confline(op, "OSCOLS", OSCOLS, NULL, -1);
  confline(op, "VHOSTCOLS", VHOSTCOLS, NULL, -1);
  confline(op, "REDIRVHOSTCOLS", REDIRVHOSTCOLS, NULL, -1);
  confline(op, "FAILVHOSTCOLS", FAILVHOSTCOLS, NULL, -1);
  confline(op, "USERCOLS", USERCOLS, NULL, -1);
  confline(op, "REDIRUSERCOLS", REDIRUSERCOLS, NULL, -1);
  confline(op, "FAILUSERCOLS", FAILUSERCOLS, NULL, -1);
  confline(op, "SEARCHQUERYCOLS", SEARCHQUERYCOLS, NULL, -1);
  confline(op, "SEARCHWORDCOLS", SEARCHWORDCOLS, NULL, -1);
  confline(op, "INTSEARCHQUERYCOLS", INTSEARCHQUERYCOLS, NULL, -1);
  confline(op, "INTSEARCHWORDCOLS", INTSEARCHWORDCOLS, NULL, -1);
  confline(op, "STATUSCOLS", STATUSCOLS, NULL, -1);
  op->outopts.repq[REP_YEAR] = YEARLY;
  op->outopts.repq[REP_QUARTERLY] = QUARTERLY;
  op->outopts.repq[REP_MONTH] = MONTHLY;
  op->outopts.repq[REP_WEEK] = WEEKLY;
  op->outopts.repq[REP_DAYREP] = DAILYREP;
  op->outopts.repq[REP_DAYSUM] = DAILYSUM;
  op->outopts.repq[REP_HOURREP] = HOURLYREP;
  op->outopts.repq[REP_HOURSUM] = HOURLYSUM;
  op->outopts.repq[REP_WEEKHOUR] = WEEKHOUR;
  op->outopts.repq[REP_QUARTERREP] = QUARTERREP;
  op->outopts.repq[REP_QUARTERSUM] = QUARTERSUM;
  op->outopts.repq[REP_FIVEREP] = FIVEREP;
  op->outopts.repq[REP_FIVESUM] = FIVESUM;
  op->outopts.repq[REP_GENSUM] = GENERAL;
  op->outopts.repq[REP_REQ] = REQUEST;
  op->outopts.repq[REP_REDIR] = REDIRREP;
  op->outopts.repq[REP_FAIL] = FAILREP;
  op->outopts.repq[REP_TYPE] = FILETYPE;
  op->outopts.repq[REP_SIZE] = SIZEREP;
  op->outopts.repq[REP_PROCTIME] = PROCTIME;
  op->outopts.repq[REP_DIR] = DIRECTORY;
  op->outopts.repq[REP_HOST] = HOST;
  op->outopts.repq[REP_REDIRHOST] = REDIRHOST;
  op->outopts.repq[REP_FAILHOST] = FAILHOST;
  op->outopts.repq[REP_DOM] = DOMAINREP;
  op->outopts.repq[REP_ORG] = ORGANISATION;
  op->outopts.repq[REP_REF] = REFERRER;
  op->outopts.repq[REP_REFSITE] = REFSITE;
  op->outopts.repq[REP_REDIRREF] = REDIRREF;
  op->outopts.repq[REP_FAILREF] = FAILREF;
  op->outopts.repq[REP_BROWREP] = BROWSERREP;
  op->outopts.repq[REP_BROWSUM] = BROWSERSUM;
  op->outopts.repq[REP_OS] = OSREP;
  op->outopts.repq[REP_VHOST] = VHOST;
  op->outopts.repq[REP_REDIRVHOST] = REDIRVHOST;
  op->outopts.repq[REP_FAILVHOST] = FAILVHOST;
  op->outopts.repq[REP_USER] = USER;
  op->outopts.repq[REP_REDIRUSER] = REDIRUSER;
  op->outopts.repq[REP_FAILUSER] = FAILUSER;
  op->outopts.repq[REP_SEARCHREP] = SEARCHQUERY;
  op->outopts.repq[REP_SEARCHSUM] = SEARCHWORD;
  op->outopts.repq[REP_INTSEARCHREP] = INTSEARCHQUERY;
  op->outopts.repq[REP_INTSEARCHSUM] = INTSEARCHWORD;
  op->outopts.repq[REP_CODE] = STATUS;
  op->outopts.last7 = LASTSEVEN;
  op->outopts.runtime = RUNTIME;
  op->outopts.gotos = GOTOS;
  op->outopts.repspan = REPORTSPAN;
  op->outopts.rsthresh = REPORTSPANTHRESHOLD;
  op->outopts.descriptions = DESCRIPTIONS;
  op->outopts.lang.code = LANGUAGE;
  op->outopts.lang.file = NULL;
  op->outopts.graph[REP_YEAR] = YEARGRAPH;
  op->outopts.graph[REP_QUARTERLY] = QUARTERLYGRAPH;
  op->outopts.graph[REP_MONTH] = MONTHGRAPH;
  op->outopts.graph[REP_WEEK] = WEEKGRAPH;
  op->outopts.graph[REP_DAYREP] = DAYREPGRAPH;
  op->outopts.graph[REP_DAYSUM] = DAYSUMGRAPH;
  op->outopts.graph[REP_HOURREP] = HOURREPGRAPH;
  op->outopts.graph[REP_HOURSUM] = HOURSUMGRAPH; 
  op->outopts.graph[REP_WEEKHOUR] = WEEKHOURGRAPH;
  op->outopts.graph[REP_QUARTERREP] = QUARTERREPGRAPH;
  op->outopts.graph[REP_QUARTERSUM] = QUARTERSUMGRAPH;
  op->outopts.graph[REP_FIVEREP] = FIVEREPGRAPH;
  op->outopts.graph[REP_FIVESUM] = FIVESUMGRAPH;
  op->outopts.back[REP_YEAR] = YEARBACK;
  op->outopts.back[REP_QUARTERLY] = QUARTERLYBACK;
  op->outopts.back[REP_MONTH] = MONTHBACK;
  op->outopts.back[REP_WEEK] = WEEKBACK;
  op->outopts.back[REP_DAYREP] = DAYBACK;
  op->outopts.back[REP_HOURREP] = HOURBACK;
  op->outopts.back[REP_QUARTERREP] = QUARTERBACK;
  op->outopts.back[REP_FIVEREP] = FIVEBACK;
  op->outopts.rows[REP_YEAR] = YEARROWS;
  op->outopts.rows[REP_QUARTERLY] = QUARTERLYROWS;
  op->outopts.rows[REP_MONTH] = MONTHROWS;
  op->outopts.rows[REP_WEEK] = WEEKROWS;
  op->outopts.rows[REP_DAYREP] = DAYROWS;
  op->outopts.rows[REP_HOURREP] = HOURROWS;
  op->outopts.rows[REP_QUARTERREP] = QUARTERROWS;
  op->outopts.rows[REP_FIVEREP] = FIVEROWS;
#ifndef NOGRAPHICS
  op->outopts.chartby[G(REP_REQ)] = REQCHART;
  op->outopts.chartby[G(REP_REDIR)] = REDIRCHART;
  op->outopts.chartby[G(REP_FAIL)] = FAILCHART;
  op->outopts.chartby[G(REP_TYPE)] = TYPECHART;
  op->outopts.chartby[G(REP_DIR)] = DIRCHART;
  op->outopts.chartby[G(REP_HOST)] = HOSTCHART;
  op->outopts.chartby[G(REP_REDIRHOST)] = REDIRHOSTCHART;
  op->outopts.chartby[G(REP_FAILHOST)] = FAILHOSTCHART;
  op->outopts.chartby[G(REP_DOM)] = DOMCHART;
  op->outopts.chartby[G(REP_REF)] = REFCHART;
  op->outopts.chartby[G(REP_REFSITE)] = REFSITECHART;
  op->outopts.chartby[G(REP_REDIRREF)] = REDIRREFCHART;
  op->outopts.chartby[G(REP_FAILREF)] = FAILREFCHART;
  op->outopts.chartby[G(REP_BROWREP)] = BROWREPCHART;
  op->outopts.chartby[G(REP_VHOST)] = VHOSTCHART;
  op->outopts.chartby[G(REP_REDIRVHOST)] = REDIRVHOSTCHART;
  op->outopts.chartby[G(REP_FAILVHOST)] = FAILVHOSTCHART;
  op->outopts.chartby[G(REP_USER)] = USERCHART;
  op->outopts.chartby[G(REP_REDIRUSER)] = REDIRUSERCHART;
  op->outopts.chartby[G(REP_FAILUSER)] = FAILUSERCHART;
  op->outopts.chartby[G(REP_ORG)] = ORGCHART;
  op->outopts.chartby[G(REP_SEARCHREP)] = SEARCHQUERYCHART;
  op->outopts.chartby[G(REP_SEARCHSUM)] = SEARCHWORDCHART;
  op->outopts.chartby[G(REP_INTSEARCHREP)] = INTSEARCHQUERYCHART;
  op->outopts.chartby[G(REP_INTSEARCHSUM)] = INTSEARCHWORDCHART;
  op->outopts.chartby[G(REP_BROWSUM)] = BROWSUMCHART;
  op->outopts.chartby[G(REP_OS)] = OSCHART;
  op->outopts.chartby[G(REP_SIZE)] = SIZECHART;
  op->outopts.chartby[G(REP_CODE)] = STATUSCHART;
  op->outopts.chartby[G(REP_PROCTIME)] = PROCTIMECHART;
#else
  for (i = 0; i < GENREP_NUMBER; i++)
    op->outopts.chartby[i] = CHART_NONE;/* to avoid recording spurious stuff */
#endif  /* NOGRAPHICS */
  for (i = 0; i < GENREP_NUMBER; i++) {
    /* initialise all SUBSORTBYs to UNSET. Then we can use this later as a test
       for whether the report is a hierarchical report. Also set all SUBFLOOR
       methods to UNSET so that we don't count any unneeded data by mistake. */
    op->outopts.subsortby[i] = UNSET;
    op->outopts.subfloor[i].floorby = UNSET;
  }
  op->outopts.sortby[G(REP_REQ)] = REQSORTBY;
  op->outopts.subsortby[G(REP_REQ)] = REQSORTBY;
  op->outopts.sortby[G(REP_REDIR)] = REDIRSORTBY;
  op->outopts.subsortby[G(REP_REDIR)] = REDIRSORTBY;
  op->outopts.sortby[G(REP_FAIL)] = FAILSORTBY;
  op->outopts.subsortby[G(REP_FAIL)] = FAILSORTBY;
  op->outopts.sortby[G(REP_TYPE)] = TYPESORTBY;
  op->outopts.subsortby[G(REP_TYPE)] = TYPESORTBY;
  op->outopts.sortby[G(REP_DIR)] = DIRSORTBY;
  op->outopts.subsortby[G(REP_DIR)] = DIRSORTBY;
  op->outopts.sortby[G(REP_HOST)] = HOSTSORTBY;
  op->outopts.sortby[G(REP_REDIRHOST)] = REDIRHOSTSORTBY;
  op->outopts.sortby[G(REP_FAILHOST)] = FAILHOSTSORTBY;
  op->outopts.sortby[G(REP_DOM)] = DOMSORTBY;
  op->outopts.subsortby[G(REP_DOM)] = DOMSORTBY;
  op->outopts.sortby[G(REP_ORG)] = ORGSORTBY;
  op->outopts.subsortby[G(REP_ORG)] = ORGSORTBY;
  op->outopts.sortby[G(REP_REF)] = REFSORTBY;
  op->outopts.subsortby[G(REP_REF)] = REFSORTBY;
  op->outopts.sortby[G(REP_REFSITE)] = REFSITESORTBY;
  op->outopts.subsortby[G(REP_REFSITE)] = REFSITESORTBY;
  op->outopts.sortby[G(REP_REDIRREF)] = REDIRREFSORTBY;
  op->outopts.subsortby[G(REP_REDIRREF)] = REDIRREFSORTBY;
  op->outopts.sortby[G(REP_FAILREF)] = FAILREFSORTBY;
  op->outopts.subsortby[G(REP_FAILREF)] = FAILREFSORTBY;
  op->outopts.sortby[G(REP_BROWREP)] = BROWREPSORTBY;
  op->outopts.sortby[G(REP_BROWSUM)] = BROWSUMSORTBY;
  op->outopts.subsortby[G(REP_BROWSUM)] = BROWSUMSORTBY;
  op->outopts.sortby[G(REP_OS)] = OSSORTBY;
  op->outopts.subsortby[G(REP_OS)] = OSSORTBY;
  op->outopts.sortby[G(REP_VHOST)] = VHOSTSORTBY;
  op->outopts.sortby[G(REP_REDIRVHOST)] = REDIRVHOSTSORTBY;
  op->outopts.sortby[G(REP_FAILVHOST)] = FAILVHOSTSORTBY;
  op->outopts.sortby[G(REP_USER)] = USERSORTBY;
  op->outopts.sortby[G(REP_REDIRUSER)] = REDIRUSERSORTBY;
  op->outopts.sortby[G(REP_FAILUSER)] = FAILUSERSORTBY;
  op->outopts.sortby[G(REP_SEARCHREP)] = SEARCHQUERYSORTBY;
  op->outopts.sortby[G(REP_SEARCHSUM)] = SEARCHWORDSORTBY;
  op->outopts.sortby[G(REP_INTSEARCHREP)] = INTSEARCHQUERYSORTBY;
  op->outopts.sortby[G(REP_INTSEARCHSUM)] = INTSEARCHWORDSORTBY;
  op->outopts.sortby[G(REP_CODE)] = STATUSSORTBY;
  op->outopts.sortby[G(REP_SIZE)] = ALPHABETICAL;
  op->outopts.sortby[G(REP_PROCTIME)] = ALPHABETICAL;
  confline(op, "REQFLOOR", REQFLOOR, NULL, -1);
  confline(op, "REQARGSFLOOR", REQARGSFLOOR, NULL, -1);
  confline(op, "REDIRFLOOR", REDIRFLOOR, NULL, -1);
  confline(op, "REDIRARGSFLOOR", REDIRARGSFLOOR, NULL, -1);
  confline(op, "FAILFLOOR", FAILFLOOR, NULL, -1);
  confline(op, "FAILARGSFLOOR", FAILARGSFLOOR, NULL, -1);
  confline(op, "TYPEFLOOR", TYPEFLOOR, NULL, -1);
  confline(op, "SUBTYPEFLOOR", SUBTYPEFLOOR, NULL, -1);
  confline(op, "DIRFLOOR", DIRFLOOR, NULL, -1);
  confline(op, "SUBDIRFLOOR", SUBDIRFLOOR, NULL, -1);
  confline(op, "HOSTFLOOR", HOSTFLOOR, NULL, -1);
  confline(op, "REDIRHOSTFLOOR", REDIRHOSTFLOOR, NULL, -1);
  confline(op, "FAILHOSTFLOOR", FAILHOSTFLOOR, NULL, -1);
  confline(op, "DOMFLOOR", DOMFLOOR, NULL, -1);
  confline(op, "SUBDOMFLOOR", SUBDOMFLOOR, NULL, -1);
  confline(op, "ORGFLOOR", ORGFLOOR, NULL, -1);
  confline(op, "SUBORGFLOOR", SUBORGFLOOR, NULL, -1);
  confline(op, "REFFLOOR", REFFLOOR, NULL, -1);
  confline(op, "REFARGSFLOOR", REFARGSFLOOR, NULL, -1);
  confline(op, "REFSITEFLOOR", REFSITEFLOOR, NULL, -1);
  confline(op, "REFDIRFLOOR", REFDIRFLOOR, NULL, -1);
  confline(op, "REDIRREFFLOOR", REDIRREFFLOOR, NULL, -1);
  confline(op, "REDIRREFARGSFLOOR", REDIRREFARGSFLOOR, NULL, -1);
  confline(op, "FAILREFFLOOR", FAILREFFLOOR, NULL, -1);
  confline(op, "FAILREFARGSFLOOR", FAILREFARGSFLOOR, NULL, -1);
  confline(op, "BROWREPFLOOR", BROWREPFLOOR, NULL, -1);
  confline(op, "BROWSUMFLOOR", BROWSUMFLOOR, NULL, -1);
  confline(op, "SUBBROWFLOOR", SUBBROWFLOOR, NULL, -1);
  confline(op, "OSFLOOR", OSFLOOR, NULL, -1);
  confline(op, "SUBOSFLOOR", SUBOSFLOOR, NULL, -1);
  confline(op, "VHOSTFLOOR", VHOSTFLOOR, NULL, -1);
  confline(op, "REDIRVHOSTFLOOR", REDIRVHOSTFLOOR, NULL, -1);
  confline(op, "FAILVHOSTFLOOR", FAILVHOSTFLOOR, NULL, -1);
  confline(op, "USERFLOOR", USERFLOOR, NULL, -1);
  confline(op, "REDIRUSERFLOOR", REDIRUSERFLOOR, NULL, -1);
  confline(op, "FAILUSERFLOOR", FAILUSERFLOOR, NULL, -1);
  confline(op, "SEARCHQUERYFLOOR", SEARCHQUERYFLOOR, NULL, -1);
  confline(op, "SEARCHWORDFLOOR", SEARCHWORDFLOOR, NULL, -1);
  confline(op, "INTSEARCHQUERYFLOOR", INTSEARCHQUERYFLOOR, NULL, -1);
  confline(op, "INTSEARCHWORDFLOOR", INTSEARCHWORDFLOOR, NULL, -1);
  confline(op, "STATUSFLOOR", STATUSFLOOR, NULL, -1);
  confline(op, "REPORTORDER", REPORTORDER, NULL, -1);
  op->outopts.barstyle = BARSTYLE;
  op->outopts.markchar = MARKCHAR;
  op->outopts.rawbytes = RAWBYTES;
  op->outopts.bytesdp = BYTESDP;
  op->outopts.norobots = NOROBOTS;
  op->outopts.linknofollow = LINKNOFOLLOW;
  COPYSTR(op->outopts.hostname, HOSTNAME);
  COPYSTR(op->outopts.xmldtd, XMLDTD);
  COPYSTR(op->outopts.hosturl, HOSTURL);
  COPYSTR(op->outopts.anonymizerurl, ANONYMIZERURL);
  op->outopts.baseurl = NULL;
  COPYSTR(op->outopts.logo, LOGO);
  COPYSTR(op->outopts.logourl, LOGOURL);
  COPYSTR(op->outopts.stylesheet, STYLESHEET);
  COPYSTR(op->outopts.cssprefix, CSSPREFIX);
  op->outopts.weekbeginson = WEEKBEGINSON;
  op->outopts.plainpagewidth = PLAINPAGEWIDTH;
  op->outopts.htmlpagewidth = HTMLPAGEWIDTH;
  op->outopts.latexpagewidth = LATEXPAGEWIDTH;
  op->outopts.mingraphwidth = MINGRAPHWIDTH;
  op->outopts.sepchar = (char)UNSET;
  op->outopts.repsepchar = (char)UNSET;
  op->outopts.decpt = (char)UNSET;
  COPYSTR(op->outopts.compsep, COMPSEP);
  for (i = 0; i < GENREP_NUMBER; i++) {
    op->outopts.link[i] = NULL;
    op->outopts.wanthead[i] = NULL;
    op->outopts.aliashead[i] = NULL;
    op->outopts.expandhead[i] = NULL;
    op->outopts.tree[i] = (Tree *)xmalloc(sizeof(Tree));
    op->outopts.tree[i]->tree = NULL;
    op->outopts.tree[i]->space = (Memman *)xmalloc(sizeof(Memman));
    op->outopts.tree[i]->space->curr_pos = NULL;
    op->outopts.tree[i]->space->first = NULL;
    op->outopts.tree[i]->space->alignment = ALIGNSIZE;
  }
  domlevels = (Strpairlist **)xmalloc(DOMLEVEL_NUMBER * sizeof(Strpairlist *));
  for (i = 0; i < DOMLEVEL_NUMBER; i++)
    domlevels[i] = NULL;
  op->outopts.tree[G(REP_REQ)]->cutfn = &rnextname;
  op->outopts.tree[G(REP_REDIR)]->cutfn = &rnextname;
  op->outopts.tree[G(REP_FAIL)]->cutfn = &rnextname;
  op->outopts.tree[G(REP_REF)]->cutfn = &rnextname;
  op->outopts.tree[G(REP_REDIRREF)]->cutfn = &rnextname;
  op->outopts.tree[G(REP_FAILREF)]->cutfn = &rnextname;
  op->outopts.tree[G(REP_TYPE)]->cutfn = &tnextname;
  op->outopts.tree[G(REP_DIR)]->cutfn = &inextname;
  op->outopts.tree[G(REP_DOM)]->cutfn = &onextname;
  op->outopts.tree[G(REP_REFSITE)]->cutfn = &snextname;
  op->outopts.tree[G(REP_BROWSUM)]->cutfn = &bnextname;
  op->outopts.tree[G(REP_OS)]->cutfn = &pnextname;
  op->outopts.tree[G(REP_ORG)]->cutfn = &Znextname;
  for (i = 0; i < DERVREP_NUMBER; i++) {
    op->outopts.derv[i] = (Derv *)xmalloc(sizeof(Derv));
    op->outopts.derv[i]->table = rehash(NULL, HASHSIZE, NULL);
    op->outopts.derv[i]->space = (Memman *)xmalloc(sizeof(Memman));
    op->outopts.derv[i]->space->curr_pos = NULL;
    op->outopts.derv[i]->space->first = NULL;
    op->outopts.derv[i]->space->alignment = 1;
  }
  op->outopts.derv[REP_SEARCHREP - FIRST_DERVREP]->cutfn = &Nnextname;
  op->outopts.derv[REP_SEARCHSUM - FIRST_DERVREP]->cutfn = &nnextname;
  op->outopts.derv[REP_INTSEARCHREP - FIRST_DERVREP]->cutfn = &Nnextname;
  op->outopts.derv[REP_INTSEARCHSUM - FIRST_DERVREP]->cutfn = &nnextname;
  op->outopts.derv[REP_BROWSUM - FIRST_DERVREP]->cutfn = &Bnextname;
  op->outopts.derv[REP_OS - FIRST_DERVREP]->cutfn = &Pnextname;
  /* derv[*]->arg is set in finalinit so that lists of search engines and
     robots have already been read in */
  op->outopts.suborgs = NULL;
  for (i = 0; i < LOGDATA_NUMBER; i++)
    op->miscopts.data[i] = 0;
  op->miscopts.bytes = 0.0;
  op->miscopts.bytes7 = 0.0;
  confline(op, "REQARGS", "*?*", NULL, -1);
  confline(op, "REDIRARGS", "*?*", NULL, -1);
  confline(op, "FAILARGS", "*?*", NULL, -1);
  confline(op, "REFARGS", "*?*", NULL, -1);
  confline(op, "REDIRREFARGS", "*?*", NULL, -1);
  confline(op, "FAILREFARGS", "*?*", NULL, -1);
  confline(op, "SUBDOMAIN", "*", NULL, -1);
  confline(op, "SUBORG2", "*", NULL, -1);
  confline(op, "SUBDIR", "*", NULL, -1);
  confline(op, "SUBTYPE", "*", NULL, -1);
  confline(op, "REFDIR", "*", NULL, -1);
  confline(op, "SUBBROW", "*", NULL, -1);
  confline(op, "SUBOS", "*:*", NULL, -1);
  for (i = 0; i < ITEM_NUMBER; i++) {
    op->wanthead[i] = NULL;
    op->aliashead[i] = NULL;
    op->hash[i] = rehash(NULL, HASHSIZE, NULL);
    op->miscopts.lowmem[i] = 0;
  }
  for (i = 0; i < MIN_SC; i++)
    op->code2type[i] = UNWANTED;
  for ( ; i < SC_NUMBER; i++)
    op->code2type[i] = UNSET;
  op->code2type[0] = UNSET;  /* code2type[0] used as a special marker in ... */
  op->succ304 = TRUE;    /* ... configsinex(), correctlogfmt() & finalinit() */
  for (i = 0; i < ARRAYREP_NUMBER; i++) {
    if (i == REP_CODE - FIRST_ARRAYREP)
      thresh = codethresh;
    else if (i == REP_SIZE - FIRST_ARRAYREP)
      thresh = sizethresh;
    else
      thresh = ptthresh;
    for (k = 0; thresh[k] > -0.5; k++)
      ;  /* find size of array */
    k++;
    op->arraydata[i] = (Arraydata *)xmalloc(k * sizeof(Arraydata));
    for (j = 0; j < k; j++) {
      op->arraydata[i][j].threshold = thresh[j];
      op->arraydata[i][j].reqs = 0;
      op->arraydata[i][j].pages = 0;
      op->arraydata[i][j].bytes = 0.0;
      op->arraydata[i][j].reqs7 = 0;
      op->arraydata[i][j].pages7 = 0;
      op->arraydata[i][j].bytes7 = 0.0;
      op->arraydata[i][j].lastdate = 0;
      op->arraydata[i][j].firstdate = LAST_TIME;
    }
  }
  op->searchengines = NULL;
  op->intsearchengines = NULL;
  op->robots = NULL;
  op->ispagehead = NULL;
  op->argshead = NULL;
  op->refargshead = NULL;
  confline(op, "PAGEINCLUDE", "*/", NULL, -1);
  confline(op, "PAGEINCLUDE", "REGEXPI:\\.html?$", NULL, -1);
  op->dman.fromstr = NULL;
  op->dman.tostr = NULL;
  op->dman.currdp = NULL;
  op->dman.firstdp = NULL;
  op->dman.lastdp = NULL;
  op->dman.firsttime = LAST_TIME;
  op->dman.lasttime = FIRST_TIME;
}
