/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* This file contains code from Philip Hazel's PCRE library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details. */

/* Copyright (c) 1997-2000 University of Cambridge */

#ifdef HAVE_PCRE
#include <pcre.h>
#else
#ifndef _PCRE_H
#define _PCRE_H

#define PCRE_MAJOR 3
#define PCRE_MINOR 4
#define PCRE_DATE  22-Aug-2000

#define SUPPORT_UTF8

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCRE_CASELESS        0x0001
#define PCRE_MULTILINE       0x0002
#define PCRE_DOTALL          0x0004
#define PCRE_EXTENDED        0x0008
#define PCRE_ANCHORED        0x0010
#define PCRE_DOLLAR_ENDONLY  0x0020
#define PCRE_EXTRA           0x0040
#define PCRE_NOTBOL          0x0080
#define PCRE_NOTEOL          0x0100
#define PCRE_UNGREEDY        0x0200
#define PCRE_NOTEMPTY        0x0400
#define PCRE_UTF8            0x0800

#define PCRE_ERROR_NOMATCH        (-1)
#define PCRE_ERROR_NULL           (-2)
#define PCRE_ERROR_BADOPTION      (-3)
#define PCRE_ERROR_BADMAGIC       (-4)
#define PCRE_ERROR_UNKNOWN_NODE   (-5)
#define PCRE_ERROR_NOMEMORY       (-6)
#define PCRE_ERROR_NOSUBSTRING    (-7)

#define PCRE_INFO_OPTIONS         0
#define PCRE_INFO_SIZE            1
#define PCRE_INFO_CAPTURECOUNT    2
#define PCRE_INFO_BACKREFMAX      3
#define PCRE_INFO_FIRSTCHAR       4
#define PCRE_INFO_FIRSTTABLE      5
#define PCRE_INFO_LASTLITERAL     6

typedef void pcre;
typedef void pcre_extra;

extern void *(*pcre_malloc)(size_t);
extern void  (*pcre_free)(void *);

extern pcre *pcre_compile(const char *, int, const char **, int *,
              const unsigned char *);
extern int  pcre_exec(const pcre *, const pcre_extra *, const char *,
              int, int, int, int *, int);
extern int  pcre_fullinfo(const pcre *, const pcre_extra *, int, void *);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* _PCRE_H */
#endif  /* HAVE_PCRE */
