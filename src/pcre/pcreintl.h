/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* This file contains code from Philip Hazel's PCRE library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details. */

/* Copyright (c) 1997-2000 University of Cambridge */

#ifndef HAVE_PCRE
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcre.h"

#ifndef offsetof
#define offsetof(p_type,field) ((size_t)&(((p_type *)0)->field))
#endif

#define PCRE_IMS (PCRE_CASELESS|PCRE_MULTILINE|PCRE_DOTALL)

#define PCRE_FIRSTSET      0x40000000
#define PCRE_REQCHSET      0x20000000
#define PCRE_STARTLINE     0x10000000
#define PCRE_INGROUP       0x08000000
#define PCRE_ICHANGED      0x04000000

#define PCRE_STUDY_MAPPED   0x01

#define PUBLIC_OPTIONS \
  (PCRE_CASELESS|PCRE_EXTENDED|PCRE_ANCHORED|PCRE_MULTILINE| \
   PCRE_DOTALL|PCRE_DOLLAR_ENDONLY|PCRE_EXTRA|PCRE_UNGREEDY|PCRE_UTF8)

#define PUBLIC_EXEC_OPTIONS \
  (PCRE_ANCHORED|PCRE_NOTBOL|PCRE_NOTEOL|PCRE_NOTEMPTY)

#define PUBLIC_STUDY_OPTIONS 0

#define MAGIC_NUMBER  0x50435245UL

typedef int BOOL;

#undef FALSE
#define FALSE   0
#undef TRUE
#define TRUE    1

enum { ESC_A = 1, ESC_B, ESC_b, ESC_D, ESC_d, ESC_S, ESC_s, ESC_W, ESC_w,
       ESC_Z, ESC_z, ESC_REF };

enum {
  OP_END,
  OP_SOD,
  OP_NOT_WORD_BOUNDARY,
  OP_WORD_BOUNDARY,
  OP_NOT_DIGIT,
  OP_DIGIT,
  OP_NOT_WHITESPACE,
  OP_WHITESPACE,
  OP_NOT_WORDCHAR,
  OP_WORDCHAR,
  OP_EODN,
  OP_EOD,
  OP_OPT,
  OP_CIRC,
  OP_DOLL,
  OP_ANY,
  OP_CHARS,
  OP_NOT,
  OP_STAR,
  OP_MINSTAR,
  OP_PLUS,
  OP_MINPLUS,
  OP_QUERY,
  OP_MINQUERY,
  OP_UPTO,
  OP_MINUPTO,
  OP_EXACT,
  OP_NOTSTAR,
  OP_NOTMINSTAR,
  OP_NOTPLUS,
  OP_NOTMINPLUS,
  OP_NOTQUERY,
  OP_NOTMINQUERY,
  OP_NOTUPTO,
  OP_NOTMINUPTO,
  OP_NOTEXACT,
  OP_TYPESTAR,
  OP_TYPEMINSTAR,
  OP_TYPEPLUS,
  OP_TYPEMINPLUS,
  OP_TYPEQUERY,
  OP_TYPEMINQUERY,
  OP_TYPEUPTO,
  OP_TYPEMINUPTO,
  OP_TYPEEXACT,
  OP_CRSTAR,
  OP_CRMINSTAR,
  OP_CRPLUS,
  OP_CRMINPLUS,
  OP_CRQUERY,
  OP_CRMINQUERY,
  OP_CRRANGE,
  OP_CRMINRANGE,
  OP_CLASS,
  OP_REF,
  OP_RECURSE,
  OP_ALT,
  OP_KET,
  OP_KETRMAX,
  OP_KETRMIN,
  OP_ASSERT,
  OP_ASSERT_NOT,
  OP_ASSERTBACK,
  OP_ASSERTBACK_NOT,
  OP_REVERSE,
  OP_ONCE,
  OP_COND,
  OP_CREF,
  OP_BRAZERO,
  OP_BRAMINZERO,
  OP_BRA
};

#define EXTRACT_MAX  99

#define ERR1  "\\ at end of pattern"
#define ERR2  "\\c at end of pattern"
#define ERR3  "unrecognized character follows \\"
#define ERR4  "numbers out of order in {} quantifier"
#define ERR5  "number too big in {} quantifier"
#define ERR6  "missing terminating ] for character class"
#define ERR7  "invalid escape sequence in character class"
#define ERR8  "range out of order in character class"
#define ERR9  "nothing to repeat"
#define ERR10 "operand of unlimited repeat could match the empty string"
#define ERR11 "internal error: unexpected repeat"
#define ERR12 "unrecognized character after (?"
#define ERR13 "too many capturing parenthesized sub-patterns"
#define ERR14 "missing )"
#define ERR15 "back reference to non-existent subpattern"
#define ERR16 "erroffset passed as NULL"
#define ERR17 "unknown option bit(s) set"
#define ERR18 "missing ) after comment"
#define ERR19 "too many sets of parentheses"
#define ERR20 "regular expression too large"
#define ERR21 "failed to get memory"
#define ERR22 "unmatched parentheses"
#define ERR23 "internal error: code overflow"
#define ERR24 "unrecognized character after (?<"
#define ERR25 "lookbehind assertion is not fixed length"
#define ERR26 "malformed number after (?("
#define ERR27 "conditional group contains more than two branches"
#define ERR28 "assertion expected after (?("
#define ERR29 "(?p must be followed by )"
#define ERR30 "unknown POSIX class name"
#define ERR31 "POSIX collating elements are not supported"
#define ERR32 "this version of PCRE is not compiled with PCRE_UTF8 support"
#define ERR33 "characters with values > 255 are not yet supported in classes"
#define ERR34 "character value in \\x{...} sequence is too large"
#define ERR35 "invalid condition (?(0)"

typedef unsigned char uschar;

typedef struct real_pcre {
  unsigned long int magic_number;
  size_t size;
  const unsigned char *tables;
  unsigned long int options;
  uschar top_bracket;
  uschar top_backref;
  uschar first_char;
  uschar req_char;
  uschar code[1];
} real_pcre;

typedef struct real_pcre_extra {
  uschar options;
  uschar start_bits[32];
} real_pcre_extra;

typedef struct compile_data {
  const uschar *lcc;
  const uschar *fcc;
  const uschar *cbits;
  const uschar *ctypes;
} compile_data;

typedef struct match_data {
  int    errorcode;
  int   *offset_vector;
  int    offset_end;
  int    offset_max;
  const uschar *lcc;
  const uschar *ctypes;
  BOOL   offset_overflow;
  BOOL   notbol;
  BOOL   noteol;
  BOOL   utf8;
  BOOL   endonly;
  BOOL   notempty;
  const uschar *start_pattern;
  const uschar *start_subject;
  const uschar *end_subject;
  const uschar *start_match;
  const uschar *end_match_ptr;
  int    end_offset_top;
} match_data;

#define ctype_space   0x01
#define ctype_letter  0x02
#define ctype_digit   0x04
#define ctype_xdigit  0x08
#define ctype_word    0x10
#define ctype_meta    0x80

#define cbit_space     0
#define cbit_xdigit   32
#define cbit_digit    64
#define cbit_upper    96
#define cbit_lower   128
#define cbit_word    160
#define cbit_graph   192
#define cbit_print   224
#define cbit_punct   256
#define cbit_cntrl   288
#define cbit_length  320

#define lcc_offset      0
#define fcc_offset    256
#define cbits_offset  512
#define ctypes_offset (cbits_offset + cbit_length)
#define tables_length (ctypes_offset + 256)

#endif  /* HAVE_PCRE */
