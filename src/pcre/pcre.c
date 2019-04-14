/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* This file contains code from Philip Hazel's PCRE library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details. */

/* Copyright (c) 1997-2000 University of Cambridge */

#ifndef HAVE_PCRE
#include "pcreintl.h"

#ifdef __cplusplus
#define class pcre_class
#endif

#define BRASTACK_SIZE 200

#ifdef SUPPORT_UTF8
#define MAXLIT 250
#else
#define MAXLIT 255
#endif

static const char rep_min[] = { 0, 0, 1, 1, 0, 0 };
static const char rep_max[] = { 0, 0, 0, 0, 1, 1 };

static const short int escapes[] = {
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,    ':',    ';',    '<',    '=',    '>',    '?',
  '@', -ESC_A, -ESC_B,      0, -ESC_D,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0, -ESC_S,      0,      0,      0, -ESC_W,
    0,      0, -ESC_Z,    '[',   '\\',    ']',    '^',    '_',
  '`',      7, -ESC_b,      0, -ESC_d,     27,   '\f',      0,
    0,      0,      0,      0,      0,      0,   '\n',      0,
    0,      0,   '\r', -ESC_s,   '\t',      0,      0, -ESC_w,
    0,      0, -ESC_z
};

static const char *posix_names[] = {
  "alpha", "lower", "upper",
  "alnum", "ascii", "cntrl", "digit", "graph",
  "print", "punct", "space", "word",  "xdigit" };

static const uschar posix_name_lengths[] = {
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 6, 0 };

static const int posix_class_maps[] = {
  cbit_lower, cbit_upper, -1,
  cbit_lower, -1,         -1,
  cbit_upper, -1,         -1,
  cbit_digit, cbit_lower, cbit_upper,
  cbit_print, cbit_cntrl, -1,
  cbit_cntrl, -1,         -1,
  cbit_digit, -1,         -1,
  cbit_graph, -1,         -1,
  cbit_print, -1,         -1,
  cbit_punct, -1,         -1,
  cbit_space, -1,         -1,
  cbit_word,  -1,         -1,
  cbit_xdigit,-1,         -1
};


static BOOL
  compile_regex(int, int, int *, uschar **, const uschar **, const char **,
    BOOL, int, int *, int *, compile_data *);

typedef struct eptrblock {
  struct eptrblock *prev;
  const uschar *saved_eptr;
} eptrblock;

#define match_condassert   0x01
#define match_isgroup      0x02

void *(*pcre_malloc)(size_t) = malloc;
void  (*pcre_free)(void *) = free;

#ifndef SUPPORT_UTF8
#define GETCHARINC(c, eptr) c = *eptr++;
#define GETCHARLEN(c, eptr, len) c = *eptr;
#define BACKCHAR(eptr)

#else   /* SUPPORT_UTF8 */

#define GETCHARINC(c, eptr) \
  c = *eptr++; \
  if (md->utf8 && (c & 0xc0) == 0xc0) \
    { \
    int a = utf8_table4[c & 0x3f]; \
    int s = 6 - a; \
    c &= utf8_table3[a]; \
    while (a-- > 0) \
      { \
      c |= (*eptr++ & 0x3f) << s; \
      s += 6; \
      } \
    }

#define GETCHARLEN(c, eptr, len) \
  c = *eptr; \
  len = 1; \
  if (md->utf8 && (c & 0xc0) == 0xc0) \
    { \
    int i; \
    int a = utf8_table4[c & 0x3f]; \
    int s = 6 - a; \
    c &= utf8_table3[a]; \
    for (i = 1; i <= a; i++) \
      { \
      c |= (eptr[i] & 0x3f) << s; \
      s += 6; \
      } \
    len += a; \
    }

#define BACKCHAR(eptr) while((*eptr & 0xc0) == 0x80) eptr--;

#endif

#include "pcrechar.h"


#ifdef SUPPORT_UTF8

static int utf8_table1[] = { 0x7f, 0x7ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff};

static int utf8_table2[] = { 0,    0xc0, 0xe0, 0xf0, 0xf8, 0xfc};
static int utf8_table3[] = { 0xff, 0x1f, 0x0f, 0x07, 0x03, 0x01};

static uschar utf8_table4[] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5 };

static int
ord2utf8(int cvalue, uschar *buffer)
{
register int i, j;
for (i = 0; i < sizeof(utf8_table1)/sizeof(int); i++)
  if (cvalue <= utf8_table1[i]) break;
*buffer++ = utf8_table2[i] | (cvalue & utf8_table3[i]);
cvalue >>= 6 - i;
for (j = 0; j < i; j++)
  {
  *buffer++ = 0x80 | (cvalue & 0x3f);
  cvalue >>= 6;
  }
return i + 1;
}
#endif

int
pcre_fullinfo(const pcre *external_re, const pcre_extra *study_data, int what,
  void *where)
{
const real_pcre *re = (const real_pcre *)external_re;
const real_pcre_extra *study = (const real_pcre_extra *)study_data;

if (re == NULL || where == NULL) return PCRE_ERROR_NULL;
if (re->magic_number != MAGIC_NUMBER) return PCRE_ERROR_BADMAGIC;

switch (what)
  {
  case PCRE_INFO_OPTIONS:
  *((unsigned long int *)where) = re->options & PUBLIC_OPTIONS;
  break;

  case PCRE_INFO_SIZE:
  *((size_t *)where) = re->size;
  break;

  case PCRE_INFO_CAPTURECOUNT:
  *((int *)where) = re->top_bracket;
  break;

  case PCRE_INFO_BACKREFMAX:
  *((int *)where) = re->top_backref;
  break;

  case PCRE_INFO_FIRSTCHAR:
  *((int *)where) =
    ((re->options & PCRE_FIRSTSET) != 0)? re->first_char :
    ((re->options & PCRE_STARTLINE) != 0)? -1 : -2;
  break;

  case PCRE_INFO_FIRSTTABLE:
  *((const uschar **)where) =
    (study != NULL && (study->options & PCRE_STUDY_MAPPED) != 0)?
      study->start_bits : NULL;
  break;

  case PCRE_INFO_LASTLITERAL:
  *((int *)where) =
    ((re->options & PCRE_REQCHSET) != 0)? re->req_char : -1;
  break;

  default: return PCRE_ERROR_BADOPTION;
  }

return 0;
}

static int
check_escape(const uschar **ptrptr, const char **errorptr, int bracount,
  int options, BOOL isclass, compile_data *cd)
{
const uschar *ptr = *ptrptr;
int c, i;

c = *(++ptr);
if (c == 0) *errorptr = ERR1;

else if (c < '0' || c > 'z') {}

else if ((i = escapes[c - '0']) != 0) c = i;

else
  {
  const uschar *oldptr;
  switch (c)
    {

    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':

    if (!isclass)
      {
      oldptr = ptr;
      c -= '0';
      while ((cd->ctypes[ptr[1]] & ctype_digit) != 0)
        c = c * 10 + *(++ptr) - '0';
      if (c < 10 || c <= bracount)
        {
        c = -(ESC_REF + c);
        break;
        }
      ptr = oldptr;
      }

    if ((c = *ptr) >= '8')
      {
      ptr--;
      c = 0;
      break;
      }

    case '0':
    c -= '0';
    while(i++ < 2 && (cd->ctypes[ptr[1]] & ctype_digit) != 0 &&
      ptr[1] != '8' && ptr[1] != '9')
        c = c * 8 + *(++ptr) - '0';
    c &= 255;
    break;

    case 'x':
#ifdef SUPPORT_UTF8
    if (ptr[1] == '{' && (options & PCRE_UTF8) != 0)
      {
      const uschar *pt = ptr + 2;
      register int count = 0;
      c = 0;
      while ((cd->ctypes[*pt] & ctype_xdigit) != 0)
        {
        count++;
        c = c * 16 + cd->lcc[*pt] -
          (((cd->ctypes[*pt] & ctype_digit) != 0)? '0' : 'W');
        pt++;
        }
      if (*pt == '}')
        {
        if (c < 0 || count > 8) *errorptr = ERR34;
        ptr = pt;
        break;
        }
      }
#endif

    c = 0;
    while (i++ < 2 && (cd->ctypes[ptr[1]] & ctype_xdigit) != 0)
      {
      ptr++;
      c = c * 16 + cd->lcc[*ptr] -
        (((cd->ctypes[*ptr] & ctype_digit) != 0)? '0' : 'W');
      }
    break;

    case 'c':
    c = *(++ptr);
    if (c == 0)
      {
      *errorptr = ERR2;
      return 0;
      }

    if (c >= 'a' && c <= 'z') c = cd->fcc[c];
    c ^= 0x40;
    break;

    default:
    if ((options & PCRE_EXTRA) != 0) switch(c)
      {
      default:
      *errorptr = ERR3;
      break;
      }
    break;
    }
  }

*ptrptr = ptr;
return c;
}

static BOOL
is_counted_repeat(const uschar *p, compile_data *cd)
{
if ((cd->ctypes[*p++] & ctype_digit) == 0) return FALSE;
while ((cd->ctypes[*p] & ctype_digit) != 0) p++;
if (*p == '}') return TRUE;

if (*p++ != ',') return FALSE;
if (*p == '}') return TRUE;

if ((cd->ctypes[*p++] & ctype_digit) == 0) return FALSE;
while ((cd->ctypes[*p] & ctype_digit) != 0) p++;
return (*p == '}');
}

static const uschar *
read_repeat_counts(const uschar *p, int *minp, int *maxp,
  const char **errorptr, compile_data *cd)
{
int min = 0;
int max = -1;

while ((cd->ctypes[*p] & ctype_digit) != 0) min = min * 10 + *p++ - '0';

if (*p == '}') max = min; else
  {
  if (*(++p) != '}')
    {
    max = 0;
    while((cd->ctypes[*p] & ctype_digit) != 0) max = max * 10 + *p++ - '0';
    if (max < min)
      {
      *errorptr = ERR4;
      return p;
      }
    }
  }

if (min > 65535 || max > 65535)
  *errorptr = ERR5;
else
  {
  *minp = min;
  *maxp = max;
  }
return p;
}

static int
find_fixedlength(uschar *code, int options)
{
int length = -1;

register int branchlength = 0;
register uschar *cc = code + 3;

for (;;)
  {
  int d;
  register int op = *cc;
  if (op >= OP_BRA) op = OP_BRA;

  switch (op)
    {
    case OP_BRA:
    case OP_ONCE:
    case OP_COND:
    d = find_fixedlength(cc, options);
    if (d < 0) return -1;
    branchlength += d;
    do cc += (cc[1] << 8) + cc[2]; while (*cc == OP_ALT);
    cc += 3;
    break;

    case OP_ALT:
    case OP_KET:
    case OP_KETRMAX:
    case OP_KETRMIN:
    case OP_END:
    if (length < 0) length = branchlength;
      else if (length != branchlength) return -1;
    if (*cc != OP_ALT) return length;
    cc += 3;
    branchlength = 0;
    break;

    case OP_ASSERT:
    case OP_ASSERT_NOT:
    case OP_ASSERTBACK:
    case OP_ASSERTBACK_NOT:
    do cc += (cc[1] << 8) + cc[2]; while (*cc == OP_ALT);
    cc += 3;
    break;

    case OP_REVERSE:
    cc++;
    /* Fall through */

    case OP_CREF:
    case OP_OPT:
    cc++;
    /* Fall through */

    case OP_SOD:
    case OP_EOD:
    case OP_EODN:
    case OP_CIRC:
    case OP_DOLL:
    case OP_NOT_WORD_BOUNDARY:
    case OP_WORD_BOUNDARY:
    cc++;
    break;

    case OP_CHARS:
    branchlength += *(++cc);
#ifdef SUPPORT_UTF8
    for (d = 1; d <= *cc; d++)
      if ((cc[d] & 0xc0) == 0x80) branchlength--;
#endif
    cc += *cc + 1;
    break;

    case OP_EXACT:
    case OP_TYPEEXACT:
    branchlength += (cc[1] << 8) + cc[2];
    cc += 4;
    break;

    case OP_NOT_DIGIT:
    case OP_DIGIT:
    case OP_NOT_WHITESPACE:
    case OP_WHITESPACE:
    case OP_NOT_WORDCHAR:
    case OP_WORDCHAR:
    case OP_ANY:
    branchlength++;
    cc++;
    break;

    case OP_CLASS:
    cc += (*cc == OP_REF)? 2 : 33;

    switch (*cc)
      {
      case OP_CRSTAR:
      case OP_CRMINSTAR:
      case OP_CRQUERY:
      case OP_CRMINQUERY:
      return -1;

      case OP_CRRANGE:
      case OP_CRMINRANGE:
      if ((cc[1] << 8) + cc[2] != (cc[3] << 8) + cc[4]) return -1;
      branchlength += (cc[1] << 8) + cc[2];
      cc += 5;
      break;

      default:
      branchlength++;
      }
    break;

    default:
    return -1;
    }
  }
}

static BOOL
check_posix_syntax(const uschar *ptr, const uschar **endptr, compile_data *cd)
{
int terminator;
terminator = *(++ptr);
if (*(++ptr) == '^') ptr++;
while ((cd->ctypes[*ptr] & ctype_letter) != 0) ptr++;
if (*ptr == terminator && ptr[1] == ']')
  {
  *endptr = ptr;
  return TRUE;
  }
return FALSE;
}

static int
check_posix_name(const uschar *ptr, int len)
{
register int yield = 0;
while (posix_name_lengths[yield] != 0)
  {
  if (len == posix_name_lengths[yield] &&
    strncmp((const char *)ptr, posix_names[yield], len) == 0) return yield;
  yield++;
  }
return -1;
}

static BOOL
compile_branch(int options, int *brackets, uschar **codeptr,
  const uschar **ptrptr, const char **errorptr, int *optchanged,
  int *reqchar, int *countlits, compile_data *cd)
{
int repeat_type, op_type;
int repeat_min, repeat_max;
int bravalue, length;
int greedy_default, greedy_non_default;
int prevreqchar;
int condcount = 0;
int subcountlits = 0;
register int c;
register uschar *code = *codeptr;
uschar *tempcode;
const uschar *ptr = *ptrptr;
const uschar *tempptr;
uschar *previous = NULL;
uschar class[32];

greedy_default = ((options & PCRE_UNGREEDY) != 0);
greedy_non_default = greedy_default ^ 1;

*reqchar = prevreqchar = -1;
*countlits = 0;

for (;; ptr++)
  {
  BOOL negate_class;
  int class_charcount;
  int class_lastchar;
  int newoptions;
  int condref;
  int subreqchar;

  c = *ptr;
  if ((options & PCRE_EXTENDED) != 0)
    {
    if ((cd->ctypes[c] & ctype_space) != 0) continue;
    if (c == '#')
      {
      while ((c = *(++ptr)) != 0 && c != '\n') ;
      continue;
      }
    }

  switch(c)
    {

    case 0:
    case '|':
    case ')':
    *codeptr = code;
    *ptrptr = ptr;
    return TRUE;

    case '^':
    previous = NULL;
    *code++ = OP_CIRC;
    break;

    case '$':
    previous = NULL;
    *code++ = OP_DOLL;
    break;

    case '.':
    previous = code;
    *code++ = OP_ANY;
    break;

    case '[':
    previous = code;
    *code++ = OP_CLASS;

    if ((c = *(++ptr)) == '^')
      {
      negate_class = TRUE;
      c = *(++ptr);
      }
    else negate_class = FALSE;

    class_charcount = 0;
    class_lastchar = -1;

    memset(class, 0, 32 * sizeof(uschar));

    do
      {
      if (c == 0)
        {
        *errorptr = ERR6;
        goto FAILED;
        }

      if (c == '[' &&
          (ptr[1] == ':' || ptr[1] == '.' || ptr[1] == '=') &&
          check_posix_syntax(ptr, &tempptr, cd))
        {
        BOOL local_negate = FALSE;
        int posix_class, i;
        register const uschar *cbits = cd->cbits;

        if (ptr[1] != ':')
          {
          *errorptr = ERR31;
          goto FAILED;
          }

        ptr += 2;
        if (*ptr == '^')
          {
          local_negate = TRUE;
          ptr++;
          }

        posix_class = check_posix_name(ptr, tempptr - ptr);
        if (posix_class < 0)
          {
          *errorptr = ERR30;
          goto FAILED;
          }

        if ((options & PCRE_CASELESS) != 0 && posix_class <= 2)
          posix_class = 0;

        posix_class *= 3;
        for (i = 0; i < 3; i++)
          {
          int taboffset = posix_class_maps[posix_class + i];
          if (taboffset < 0) break;
          if (local_negate)
            for (c = 0; c < 32; c++) class[c] |= ~cbits[c+taboffset];
          else
            for (c = 0; c < 32; c++) class[c] |= cbits[c+taboffset];
          }

        ptr = tempptr + 1;
        class_charcount = 10;
        continue;
        }

      if (c == '\\')
        {
        c = check_escape(&ptr, errorptr, *brackets, options, TRUE, cd);
        if (-c == ESC_b) c = '\b';
        else if (c < 0)
          {
          register const uschar *cbits = cd->cbits;
          class_charcount = 10;
          switch (-c)
            {
            case ESC_d:
            for (c = 0; c < 32; c++) class[c] |= cbits[c+cbit_digit];
            continue;

            case ESC_D:
            for (c = 0; c < 32; c++) class[c] |= ~cbits[c+cbit_digit];
            continue;

            case ESC_w:
            for (c = 0; c < 32; c++) class[c] |= cbits[c+cbit_word];
            continue;

            case ESC_W:
            for (c = 0; c < 32; c++) class[c] |= ~cbits[c+cbit_word];
            continue;

            case ESC_s:
            for (c = 0; c < 32; c++) class[c] |= cbits[c+cbit_space];
            continue;

            case ESC_S:
            for (c = 0; c < 32; c++) class[c] |= ~cbits[c+cbit_space];
            continue;

            default:
            *errorptr = ERR7;
            goto FAILED;
            }
          }

#ifdef SUPPORT_UTF8
        if (c > 255)
          {
          *errorptr = ERR33;
          goto FAILED;
          }
#endif
        }

      if (ptr[1] == '-' && ptr[2] != ']')
        {
        int d;
        ptr += 2;
        d = *ptr;

        if (d == 0)
          {
          *errorptr = ERR6;
          goto FAILED;
          }

        if (d == '\\')
          {
          const uschar *oldptr = ptr;
          d = check_escape(&ptr, errorptr, *brackets, options, TRUE, cd);

#ifdef SUPPORT_UTF8
          if (d > 255)
            {
            *errorptr = ERR33;
            goto FAILED;
            }
#endif
          if (d < 0)
            {
            if (d == -ESC_b) d = '\b'; else
              {
              ptr = oldptr - 2;
              goto SINGLE_CHARACTER;
              }
            }
          }

        if (d < c)
          {
          *errorptr = ERR8;
          goto FAILED;
          }

        for (; c <= d; c++)
          {
          class[c/8] |= (1 << (c&7));
          if ((options & PCRE_CASELESS) != 0)
            {
            int uc = cd->fcc[c];
            class[uc/8] |= (1 << (uc&7));
            }
          class_charcount++;
          class_lastchar = c;
          }
        continue;
        }

      SINGLE_CHARACTER:

      class [c/8] |= (1 << (c&7));
      if ((options & PCRE_CASELESS) != 0)
        {
        c = cd->fcc[c];
        class[c/8] |= (1 << (c&7));
        }
      class_charcount++;
      class_lastchar = c;
      }

    while ((c = *(++ptr)) != ']');

    if (class_charcount == 1 && class_lastchar >= 0)
      {
      if (negate_class)
        {
        code[-1] = OP_NOT;
        }
      else
        {
        code[-1] = OP_CHARS;
        *code++ = 1;
        }
      *code++ = class_lastchar;
      }

    else
      {
      if (negate_class)
        for (c = 0; c < 32; c++) code[c] = ~class[c];
      else
        memcpy(code, class, 32);
      code += 32;
      }
    break;

    case '{':
    if (!is_counted_repeat(ptr+1, cd)) goto NORMAL_CHAR;
    ptr = read_repeat_counts(ptr+1, &repeat_min, &repeat_max, errorptr, cd);
    if (*errorptr != NULL) goto FAILED;
    goto REPEAT;

    case '*':
    repeat_min = 0;
    repeat_max = -1;
    goto REPEAT;

    case '+':
    repeat_min = 1;
    repeat_max = -1;
    goto REPEAT;

    case '?':
    repeat_min = 0;
    repeat_max = 1;

    REPEAT:
    if (previous == NULL)
      {
      *errorptr = ERR9;
      goto FAILED;
      }

    if (ptr[1] == '?')
      { repeat_type = greedy_non_default; ptr++; }
    else repeat_type = greedy_default;

    if (*previous == OP_CHARS)
      {
      int len = previous[1];

      if (repeat_min == 0) *reqchar = prevreqchar;
      *countlits += repeat_min - 1;

      if (len == 1)
        {
        c = previous[2];
        code = previous;
        }
      else
        {
        c = previous[len+1];
        previous[1]--;
        code--;
        }
      op_type = 0;
      goto OUTPUT_SINGLE_REPEAT;
      }

    else if ((int)*previous == OP_NOT)
      {
      op_type = OP_NOTSTAR - OP_STAR;
      c = previous[1];
      code = previous;
      goto OUTPUT_SINGLE_REPEAT;
      }

    else if ((int)*previous < OP_EODN || *previous == OP_ANY)
      {
      op_type = OP_TYPESTAR - OP_STAR;
      c = *previous;
      code = previous;

      OUTPUT_SINGLE_REPEAT:

      if (repeat_max == 0) goto END_REPEAT;

      repeat_type += op_type;

      if (repeat_min == 0)
        {
        if (repeat_max == -1) *code++ = OP_STAR + repeat_type;
          else if (repeat_max == 1) *code++ = OP_QUERY + repeat_type;
        else
          {
          *code++ = OP_UPTO + repeat_type;
          *code++ = repeat_max >> 8;
          *code++ = (repeat_max & 255);
          }
        }

      else if (repeat_min == 1 && repeat_max == -1)
        *code++ = OP_PLUS + repeat_type;

      else
        {
        if (repeat_min != 1)
          {
          *code++ = OP_EXACT + op_type;
          *code++ = repeat_min >> 8;
          *code++ = (repeat_min & 255);
          }

        else if (*previous == OP_CHARS)
          {
          if (code == previous) code += 2; else previous[1]++;
          }

        else if (*previous == OP_NOT) code++;

        if (repeat_max < 0)
          {
          *code++ = c;
          *code++ = OP_STAR + repeat_type;
          }

        else if (repeat_max != repeat_min)
          {
          *code++ = c;
          repeat_max -= repeat_min;
          *code++ = OP_UPTO + repeat_type;
          *code++ = repeat_max >> 8;
          *code++ = (repeat_max & 255);
          }
        }

      *code++ = c;
      }

    else if (*previous == OP_CLASS || *previous == OP_REF)
      {
      if (repeat_max == 0)
        {
        code = previous;
        goto END_REPEAT;
        }
      if (repeat_min == 0 && repeat_max == -1)
        *code++ = OP_CRSTAR + repeat_type;
      else if (repeat_min == 1 && repeat_max == -1)
        *code++ = OP_CRPLUS + repeat_type;
      else if (repeat_min == 0 && repeat_max == 1)
        *code++ = OP_CRQUERY + repeat_type;
      else
        {
        *code++ = OP_CRRANGE + repeat_type;
        *code++ = repeat_min >> 8;
        *code++ = repeat_min & 255;
        if (repeat_max == -1) repeat_max = 0;
        *code++ = repeat_max >> 8;
        *code++ = repeat_max & 255;
        }
      }

    else if ((int)*previous >= OP_BRA || (int)*previous == OP_ONCE ||
             (int)*previous == OP_COND)
      {
      register int i;
      int ketoffset = 0;
      int len = code - previous;
      uschar *bralink = NULL;

      if (repeat_max == -1)
        {
        register uschar *ket = previous;
        do ket += (ket[1] << 8) + ket[2]; while (*ket != OP_KET);
        ketoffset = code - ket;
        }

      if (repeat_min == 0)
        {

        if (subcountlits > 0)
          {
          *reqchar = prevreqchar;
          *countlits -= subcountlits;
          }

        if (repeat_max == 0)
          {
          code = previous;
          goto END_REPEAT;
          }

        if (repeat_max <= 1)
          {
          memmove(previous+1, previous, len);
          code++;
          *previous++ = OP_BRAZERO + repeat_type;
          }

        else
          {
          int offset;
          memmove(previous+4, previous, len);
          code += 4;
          *previous++ = OP_BRAZERO + repeat_type;
          *previous++ = OP_BRA;

          offset = (bralink == NULL)? 0 : previous - bralink;
          bralink = previous;
          *previous++ = offset >> 8;
          *previous++ = offset & 255;
          }

        repeat_max--;
        }

      else
        {
        for (i = 1; i < repeat_min; i++)
          {
          memcpy(code, previous, len);
          code += len;
          }
        if (repeat_max > 0) repeat_max -= repeat_min;
        }

      if (repeat_max >= 0)
        {
        for (i = repeat_max - 1; i >= 0; i--)
          {
          *code++ = OP_BRAZERO + repeat_type;

          if (i != 0)
            {
            int offset;
            *code++ = OP_BRA;
            offset = (bralink == NULL)? 0 : code - bralink;
            bralink = code;
            *code++ = offset >> 8;
            *code++ = offset & 255;
            }

          memcpy(code, previous, len);
          code += len;
          }

        while (bralink != NULL)
          {
          int oldlinkoffset;
          int offset = code - bralink + 1;
          uschar *bra = code - offset;
          oldlinkoffset = (bra[1] << 8) + bra[2];
          bralink = (oldlinkoffset == 0)? NULL : bralink - oldlinkoffset;
          *code++ = OP_KET;
          *code++ = bra[1] = offset >> 8;
          *code++ = bra[2] = (offset & 255);
          }
        }

      else code[-ketoffset] = OP_KETRMAX + repeat_type;
      }

    else
      {
      *errorptr = ERR11;
      goto FAILED;
      }

    END_REPEAT:
    previous = NULL;
    break;

    case '(':
    newoptions = options;
    condref = -1;

    if (*(++ptr) == '?')
      {
      int set, unset;
      int *optset;

      switch (*(++ptr))
        {
        case '#':
        ptr++;
        while (*ptr != ')') ptr++;
        continue;

        case ':':
        bravalue = OP_BRA;
        ptr++;
        break;

        case '(':
        bravalue = OP_COND;
        if ((cd->ctypes[*(++ptr)] & ctype_digit) != 0)
          {
          condref = *ptr - '0';
          while (*(++ptr) != ')') condref = condref*10 + *ptr - '0';
          if (condref == 0)
            {
            *errorptr = ERR35;
            goto FAILED;
            }
          ptr++;
          }
        else ptr--;
        break;

        case '=':
        bravalue = OP_ASSERT;
        ptr++;
        break;

        case '!':
        bravalue = OP_ASSERT_NOT;
        ptr++;
        break;

        case '<':
        switch (*(++ptr))
          {
          case '=':
          bravalue = OP_ASSERTBACK;
          ptr++;
          break;

          case '!':
          bravalue = OP_ASSERTBACK_NOT;
          ptr++;
          break;

          default:
          *errorptr = ERR24;
          goto FAILED;
          }
        break;

        case '>':
        bravalue = OP_ONCE;
        ptr++;
        break;

        case 'R':
        *code++ = OP_RECURSE;
        ptr++;
        continue;

        default:
        set = unset = 0;
        optset = &set;

        while (*ptr != ')' && *ptr != ':')
          {
          switch (*ptr++)
            {
            case '-': optset = &unset; break;

            case 'i': *optset |= PCRE_CASELESS; break;
            case 'm': *optset |= PCRE_MULTILINE; break;
            case 's': *optset |= PCRE_DOTALL; break;
            case 'x': *optset |= PCRE_EXTENDED; break;
            case 'U': *optset |= PCRE_UNGREEDY; break;
            case 'X': *optset |= PCRE_EXTRA; break;

            default:
            *errorptr = ERR12;
            goto FAILED;
            }
          }

        newoptions = (options | set) & (~unset);

        if (*ptr == ')')
          {
          if ((options & PCRE_INGROUP) != 0 &&
              (options & PCRE_IMS) != (newoptions & PCRE_IMS))
            {
            *code++ = OP_OPT;
            *code++ = *optchanged = newoptions & PCRE_IMS;
            }
          options = newoptions;
          previous = NULL;
          continue;
          }

        bravalue = OP_BRA;
        ptr++;
        }
      }

    else
      {
      if (++(*brackets) > EXTRACT_MAX)
        {
        *errorptr = ERR13;
        goto FAILED;
        }
      bravalue = OP_BRA + *brackets;
      }

    previous = (bravalue >= OP_ONCE)? code : NULL;
    *code = bravalue;
    tempcode = code;

    if (!compile_regex(
         options | PCRE_INGROUP,
         ((options & PCRE_IMS) != (newoptions & PCRE_IMS))?
           newoptions & PCRE_IMS : -1,
         brackets,
         &tempcode,
         &ptr,
         errorptr,
         (bravalue == OP_ASSERTBACK ||
          bravalue == OP_ASSERTBACK_NOT),
         condref,
         &subreqchar,
         &subcountlits,
         cd))
      goto FAILED;

    if (bravalue == OP_COND)
      {
      uschar *tc = code;
      condcount = 0;

      do {
         condcount++;
         tc += (tc[1] << 8) | tc[2];
         }
      while (*tc != OP_KET);

      if (condcount > 2)
        {
        *errorptr = ERR27;
        goto FAILED;
        }
      }

    if (subreqchar > 0 &&
         (bravalue >= OP_BRA || bravalue == OP_ONCE || bravalue == OP_ASSERT ||
         (bravalue == OP_COND && condcount == 2)))
      {
      prevreqchar = *reqchar;
      *reqchar = subreqchar;
      if (bravalue != OP_ASSERT) *countlits += subcountlits;
      }

    code = tempcode;

    if (*ptr != ')')
      {
      *errorptr = ERR14;
      goto FAILED;
      }
    break;

    case '\\':
    tempptr = ptr;
    c = check_escape(&ptr, errorptr, *brackets, options, FALSE, cd);

    if (c < 0)
      {
      if (-c >= ESC_REF)
        {
        previous = code;
        *code++ = OP_REF;
        *code++ = -c - ESC_REF;
        }
      else
        {
        previous = (-c > ESC_b && -c < ESC_Z)? code : NULL;
        *code++ = -c;
        }
      continue;
      }

    ptr = tempptr;
    c = '\\';

    NORMAL_CHAR:
    default:
    previous = code;
    *code = OP_CHARS;
    code += 2;
    length = 0;

    do
      {
      if ((options & PCRE_EXTENDED) != 0)
        {
        if ((cd->ctypes[c] & ctype_space) != 0) continue;
        if (c == '#')
          {
          while ((c = *(++ptr)) != 0 && c != '\n') ;
          if (c == 0) break;
          continue;
          }
        }

      if (c == '\\')
        {
        tempptr = ptr;
        c = check_escape(&ptr, errorptr, *brackets, options, FALSE, cd);
        if (c < 0) { ptr = tempptr; break; }

#ifdef SUPPORT_UTF8
        if (c > 127 && (options & PCRE_UTF8) != 0)
          {
          uschar buffer[8];
          int len = ord2utf8(c, buffer);
          for (c = 0; c < len; c++) *code++ = buffer[c];
          length += len;
          continue;
          }
#endif
        }

      *code++ = c;
      length++;
      }

    while (length < MAXLIT && (cd->ctypes[c = *(++ptr)] & ctype_meta) == 0);

    prevreqchar = (length > 1)? code[-2] : *reqchar;
    *reqchar = code[-1];
    *countlits += length;

    previous[1] = length;
    if (length < MAXLIT) ptr--;
    break;
    }
  }

FAILED:
*ptrptr = ptr;
return FALSE;
}

static BOOL
compile_regex(int options, int optchanged, int *brackets, uschar **codeptr,
  const uschar **ptrptr, const char **errorptr, BOOL lookbehind, int condref,
  int *reqchar, int *countlits, compile_data *cd)
{
const uschar *ptr = *ptrptr;
uschar *code = *codeptr;
uschar *last_branch = code;
uschar *start_bracket = code;
uschar *reverse_count = NULL;
int oldoptions = options & PCRE_IMS;
int branchreqchar, branchcountlits;

*reqchar = -1;
*countlits = INT_MAX;
code += 3;

if (condref >= 0)
  {
  *code++ = OP_CREF;
  *code++ = condref;
  }

for (;;)
  {
  int length;

  if (optchanged >= 0)
    {
    *code++ = OP_OPT;
    *code++ = optchanged;
    options = (options & ~PCRE_IMS) | optchanged;
    }

  if (lookbehind)
    {
    *code++ = OP_REVERSE;
    reverse_count = code;
    *code++ = 0;
    *code++ = 0;
    }

  if (!compile_branch(options, brackets, &code, &ptr, errorptr, &optchanged,
      &branchreqchar, &branchcountlits, cd))
    {
    *ptrptr = ptr;
    return FALSE;
    }

  length = code - last_branch;
  last_branch[1] = length >> 8;
  last_branch[2] = length & 255;

  if (*reqchar != -2)
    {
    if (branchreqchar >= 0)
      {
      if (*reqchar == -1) *reqchar = branchreqchar;
      else if (*reqchar != branchreqchar) *reqchar = -2;
      }
    else *reqchar = -2;
    }

  if (branchcountlits < *countlits) *countlits = branchcountlits;

  if (lookbehind)
    {
    *code = OP_END;
    length = find_fixedlength(last_branch, options);
    if (length < 0)
      {
      *errorptr = ERR25;
      *ptrptr = ptr;
      return FALSE;
      }
    reverse_count[0] = (length >> 8);
    reverse_count[1] = length & 255;
    }

  if (*ptr != '|')
    {
    length = code - start_bracket;
    *code++ = OP_KET;
    *code++ = length >> 8;
    *code++ = length & 255;
    if (optchanged >= 0)
      {
      *code++ = OP_OPT;
      *code++ = oldoptions;
      }
    *codeptr = code;
    *ptrptr = ptr;
    return TRUE;
    }

  *code = OP_ALT;
  last_branch = code;
  code += 3;
  ptr++;
  }
}

static const uschar*
first_significant_code(const uschar *code, int *options, int optbit,
  BOOL optstop)
{
for (;;)
  {
  switch ((int)*code)
    {
    case OP_OPT:
    if (optbit > 0 && ((int)code[1] & optbit) != (*options & optbit))
      {
      if (optstop) return code;
      *options = (int)code[1];
      }
    code += 2;
    break;

    case OP_CREF:
    code += 2;
    break;

    case OP_WORD_BOUNDARY:
    case OP_NOT_WORD_BOUNDARY:
    code++;
    break;

    case OP_ASSERT_NOT:
    case OP_ASSERTBACK:
    case OP_ASSERTBACK_NOT:
    do code += (code[1] << 8) + code[2]; while (*code == OP_ALT);
    code += 3;
    break;

    default:
    return code;
    }
  }
}

static BOOL
is_anchored(register const uschar *code, int *options)
{
do {
   const uschar *scode = first_significant_code(code + 3, options,
     PCRE_MULTILINE, FALSE);
   register int op = *scode;
   if (op >= OP_BRA || op == OP_ASSERT || op == OP_ONCE || op == OP_COND)
     { if (!is_anchored(scode, options)) return FALSE; }
   else if ((op == OP_TYPESTAR || op == OP_TYPEMINSTAR) &&
            (*options & PCRE_DOTALL) != 0)
     { if (scode[1] != OP_ANY) return FALSE; }
   else if (op != OP_SOD &&
           ((*options & PCRE_MULTILINE) != 0 || op != OP_CIRC))
     return FALSE;
   code += (code[1] << 8) + code[2];
   }
while (*code == OP_ALT);
return TRUE;
}

static BOOL
is_startline(const uschar *code)
{
do {
   const uschar *scode = first_significant_code(code + 3, NULL, 0, FALSE);
   register int op = *scode;
   if (op >= OP_BRA || op == OP_ASSERT || op == OP_ONCE || op == OP_COND)
     { if (!is_startline(scode)) return FALSE; }
   else if (op == OP_TYPESTAR || op == OP_TYPEMINSTAR)
     { if (scode[1] != OP_ANY) return FALSE; }
   else if (op != OP_CIRC) return FALSE;
   code += (code[1] << 8) + code[2];
   }
while (*code == OP_ALT);
return TRUE;
}

static int
find_firstchar(const uschar *code, int *options)
{
register int c = -1;
do {
   int d;
   const uschar *scode = first_significant_code(code + 3, options,
     PCRE_CASELESS, TRUE);
   register int op = *scode;

   if (op >= OP_BRA) op = OP_BRA;

   switch(op)
     {
     default:
     return -1;

     case OP_BRA:
     case OP_ASSERT:
     case OP_ONCE:
     case OP_COND:
     if ((d = find_firstchar(scode, options)) < 0) return -1;
     if (c < 0) c = d; else if (c != d) return -1;
     break;

     case OP_EXACT:       /* Fall through */
     scode++;

     case OP_CHARS:       /* Fall through */
     scode++;

     case OP_PLUS:
     case OP_MINPLUS:
     if (c < 0) c = scode[1]; else if (c != scode[1]) return -1;
     break;
     }

   code += (code[1] << 8) + code[2];
   }
while (*code == OP_ALT);
return c;
}

pcre *
pcre_compile(const char *pattern, int options, const char **errorptr,
  int *erroroffset, const unsigned char *tables)
{
real_pcre *re;
int length = 3;
int runlength;
int c, reqchar, countlits;
int bracount = 0;
int top_backref = 0;
int branch_extra = 0;
int branch_newextra;
unsigned int brastackptr = 0;
size_t size;
uschar *code;
const uschar *ptr;
compile_data compile_block;
int brastack[BRASTACK_SIZE];
uschar bralenstack[BRASTACK_SIZE];

#ifndef SUPPORT_UTF8
if ((options & PCRE_UTF8) != 0)
  {
  *errorptr = ERR32;
  return NULL;
  }
#endif

if (errorptr == NULL) return NULL;
*errorptr = NULL;

if (erroroffset == NULL)
  {
  *errorptr = ERR16;
  return NULL;
  }
*erroroffset = 0;

if ((options & ~PUBLIC_OPTIONS) != 0)
  {
  *errorptr = ERR17;
  return NULL;
  }

if (tables == NULL) tables = pcre_default_tables;
compile_block.lcc = tables + lcc_offset;
compile_block.fcc = tables + fcc_offset;
compile_block.cbits = tables + cbits_offset;
compile_block.ctypes = tables + ctypes_offset;

ptr = (const uschar *)(pattern - 1);
while ((c = *(++ptr)) != 0)
  {
  int min, max;
  int class_charcount;

  if ((options & PCRE_EXTENDED) != 0)
    {
    if ((compile_block.ctypes[c] & ctype_space) != 0) continue;
    if (c == '#')
      {
      while ((c = *(++ptr)) != 0 && c != '\n') ;
      continue;
      }
    }

  switch(c)
    {
    case '\\':
      {
      const uschar *save_ptr = ptr;
      c = check_escape(&ptr, errorptr, bracount, options, FALSE, &compile_block);
      if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
      if (c >= 0)
        {
        ptr = save_ptr;
        c = '\\';
        goto NORMAL_CHAR;
        }
      }
    length++;

    if (c <= -ESC_REF)
      {
      int refnum = -c - ESC_REF;
      if (refnum > top_backref) top_backref = refnum;
      length++;
      if (ptr[1] == '{' && is_counted_repeat(ptr+2, &compile_block))
        {
        ptr = read_repeat_counts(ptr+2, &min, &max, errorptr, &compile_block);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        if ((min == 0 && (max == 1 || max == -1)) ||
          (min == 1 && max == -1))
            length++;
        else length += 5;
        if (ptr[1] == '?') ptr++;
        }
      }
    continue;

    case '^':
    case '.':
    case '$':
    case '*':
    case '+':
    case '?':
    length++;
    continue;

    case '{':
    if (!is_counted_repeat(ptr+1, &compile_block)) goto NORMAL_CHAR;
    ptr = read_repeat_counts(ptr+1, &min, &max, errorptr, &compile_block);
    if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
    if ((min == 0 && (max == 1 || max == -1)) ||
      (min == 1 && max == -1))
        length++;
    else
      {
      length--;
      if (min == 1) length++; else if (min > 0) length += 4;
      if (max > 0) length += 4; else length += 2;
      }
    if (ptr[1] == '?') ptr++;
    continue;

    case '|':
    length += 3 + branch_extra;
    continue;

    case '[':
    class_charcount = 0;
    if (*(++ptr) == '^') ptr++;
    do
      {
      if (*ptr == '\\')
        {
        int ch = check_escape(&ptr, errorptr, bracount, options, TRUE,
          &compile_block);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        if (-ch == ESC_b) class_charcount++; else class_charcount = 10;
        }
      else class_charcount++;
      ptr++;
      }
    while (*ptr != 0 && *ptr != ']');

    if (class_charcount == 1) length += 3; else
      {
      length += 33;

      if (*ptr != 0 && ptr[1] == '{' && is_counted_repeat(ptr+2, &compile_block))
        {
        ptr = read_repeat_counts(ptr+2, &min, &max, errorptr, &compile_block);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        if ((min == 0 && (max == 1 || max == -1)) ||
          (min == 1 && max == -1))
            length++;
        else length += 5;
        if (ptr[1] == '?') ptr++;
        }
      }
    continue;

    case '(':
    branch_newextra = 0;

    if (ptr[1] == '?')
      {
      int set, unset;
      int *optset;

      switch (c = ptr[2])
        {
        case '#':
        ptr += 3;
        while (*ptr != 0 && *ptr != ')') ptr++;
        if (*ptr == 0)
          {
          *errorptr = ERR18;
          goto PCRE_ERROR_RETURN;
          }
        continue;

        case ':':
        case '=':
        case '!':
        case '>':
        ptr += 2;
        break;

        case 'R':
        if (ptr[3] != ')')
          {
          *errorptr = ERR29;
          goto PCRE_ERROR_RETURN;
          }
        ptr += 3;
        length += 1;
        break;

        case '<':
        if (ptr[3] == '=' || ptr[3] == '!')
          {
          ptr += 3;
          branch_newextra = 3;
          length += 3;
          break;
          }
        *errorptr = ERR24;
        goto PCRE_ERROR_RETURN;

        case '(':
        if ((compile_block.ctypes[ptr[3]] & ctype_digit) != 0)
          {
          ptr += 4;
          length += 2;
          while ((compile_block.ctypes[*ptr] & ctype_digit) != 0) ptr++;
          if (*ptr != ')')
            {
            *errorptr = ERR26;
            goto PCRE_ERROR_RETURN;
            }
          }
        else
          {
          ptr++;
          if (ptr[2] != '?' ||
             (ptr[3] != '=' && ptr[3] != '!' && ptr[3] != '<') )
            {
            ptr += 2;
            *errorptr = ERR28;
            goto PCRE_ERROR_RETURN;
            }
          }
        break;

        default:
        set = unset = 0;
        optset = &set;
        ptr += 2;

        for (;; ptr++)
          {
          c = *ptr;
          switch (c)
            {
            case 'i':
            *optset |= PCRE_CASELESS;
            continue;

            case 'm':
            *optset |= PCRE_MULTILINE;
            continue;

            case 's':
            *optset |= PCRE_DOTALL;
            continue;

            case 'x':
            *optset |= PCRE_EXTENDED;
            continue;

            case 'X':
            *optset |= PCRE_EXTRA;
            continue;

            case 'U':
            *optset |= PCRE_UNGREEDY;
            continue;

            case '-':
            optset = &unset;
            continue;

            case ')':
            if (brastackptr == 0)
              {
              options = (options | set) & (~unset);
              set = unset = 0;
              }
            /* Fall through */

            case ':':
            if (((set|unset) & PCRE_IMS) != 0)
              {
              length += 4;
              branch_newextra = 2;
              if (((set|unset) & PCRE_CASELESS) != 0) options |= PCRE_ICHANGED;
              }
            goto END_OPTIONS;

            default:
            *errorptr = ERR12;
            goto PCRE_ERROR_RETURN;
            }
          }

        END_OPTIONS:
        if (c == ')')
          {
          if (branch_newextra == 2 && (branch_extra == 0 || branch_extra == 3))
            branch_extra += branch_newextra;
          continue;
          }
        }
      }
    else bracount++;

    if (brastackptr >= sizeof(brastack)/sizeof(int))
      {
      *errorptr = ERR19;
      goto PCRE_ERROR_RETURN;
      }

    bralenstack[brastackptr] = branch_extra;
    branch_extra = branch_newextra;

    brastack[brastackptr++] = length;
    length += 3;
    continue;

    case ')':
    length += 3;
      {
      int minval = 1;
      int maxval = 1;
      int duplength;

      if (brastackptr > 0)
        {
        duplength = length - brastack[--brastackptr];
        branch_extra = bralenstack[brastackptr];
        }
      else duplength = 0;

      if ((c = ptr[1]) == '{' && is_counted_repeat(ptr+2, &compile_block))
        {
        ptr = read_repeat_counts(ptr+2, &minval, &maxval, errorptr,
          &compile_block);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        }
      else if (c == '*') { minval = 0; maxval = -1; ptr++; }
      else if (c == '+') { maxval = -1; ptr++; }
      else if (c == '?') { minval = 0; ptr++; }

      if (minval == 0)
        {
        length++;
        if (maxval > 0) length += (maxval - 1) * (duplength + 7);
        }
      else
        {
        length += (minval - 1) * duplength;
        if (maxval > minval)
          length += (maxval - minval) * (duplength + 7) - 6;
        }
      }
    continue;

    NORMAL_CHAR:
    default:
    length += 2;
    runlength = 0;
    do
      {
      if ((options & PCRE_EXTENDED) != 0)
        {
        if ((compile_block.ctypes[c] & ctype_space) != 0) continue;
        if (c == '#')
          {
          while ((c = *(++ptr)) != 0 && c != '\n') ;
          continue;
          }
        }

      if (c == '\\')
        {
        const uschar *saveptr = ptr;
        c = check_escape(&ptr, errorptr, bracount, options, FALSE,
          &compile_block);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        if (c < 0) { ptr = saveptr; break; }

#ifdef SUPPORT_UTF8
        if (c > 127 && (options & PCRE_UTF8) != 0)
          {
          int i;
          for (i = 0; i < sizeof(utf8_table1)/sizeof(int); i++)
            if (c <= utf8_table1[i]) break;
          runlength += i;
          }
#endif
        }

      runlength++;
      }

    while (runlength < MAXLIT &&
      (compile_block.ctypes[c = *(++ptr)] & ctype_meta) == 0);

    ptr--;
    length += runlength;
    continue;
    }
  }

length += 4;

if (length > 65539)
  {
  *errorptr = ERR20;
  return NULL;
  }

size = length + offsetof(real_pcre, code[0]);
re = (real_pcre *)(pcre_malloc)(size);

if (re == NULL)
  {
  *errorptr = ERR21;
  return NULL;
  }

re->magic_number = MAGIC_NUMBER;
re->size = size;
re->options = options;
re->tables = tables;

ptr = (const uschar *)pattern;
code = re->code;
*code = OP_BRA;
bracount = 0;
(void)compile_regex(options, -1, &bracount, &code, &ptr, errorptr, FALSE, -1,
  &reqchar, &countlits, &compile_block);
re->top_bracket = bracount;
re->top_backref = top_backref;

if (*errorptr == NULL && *ptr != 0) *errorptr = ERR22;

*code++ = OP_END;

if (top_backref > re->top_bracket) *errorptr = ERR15;

if (*errorptr != NULL)
  {
  (pcre_free)(re);
  PCRE_ERROR_RETURN:
  *erroroffset = ptr - (const uschar *)pattern;
  return NULL;
  }

if ((options & PCRE_ANCHORED) == 0)
  {
  int temp_options = options;
  if (is_anchored(re->code, &temp_options))
    re->options |= PCRE_ANCHORED;
  else
    {
    int ch = find_firstchar(re->code, &temp_options);
    if (ch >= 0)
      {
      re->first_char = ch;
      re->options |= PCRE_FIRSTSET;
      }
    else if (is_startline(re->code))
      re->options |= PCRE_STARTLINE;
    }
  }

if (reqchar >= 0 && (countlits > 1 || (re->options & PCRE_FIRSTSET) == 0))
  {
  re->req_char = reqchar;
  re->options |= PCRE_REQCHSET;
  }

return (pcre *)re;
}

static BOOL
match_ref(int offset, register const uschar *eptr, int length, match_data *md,
  unsigned long int ims)
{
const uschar *p = md->start_subject + md->offset_vector[offset];

if (length > md->end_subject - eptr) return FALSE;

if ((ims & PCRE_CASELESS) != 0)
  {
  while (length-- > 0)
    if (md->lcc[*p++] != md->lcc[*eptr++]) return FALSE;
  }
else
  { while (length-- > 0) if (*p++ != *eptr++) return FALSE; }

return TRUE;
}

static BOOL
match(register const uschar *eptr, register const uschar *ecode,
  int offset_top, match_data *md, unsigned long int ims, eptrblock *eptrb,
  int flags)
{
unsigned long int original_ims = ims;   /* Save for resetting on ')' */
eptrblock newptrb;

if ((flags & match_isgroup) != 0)
  {
  newptrb.prev = eptrb;
  newptrb.saved_eptr = eptr;
  eptrb = &newptrb;
  }

for (;;)
  {
  int op = (int)*ecode;
  int min, max, ctype;
  register int i;
  register int c;
  BOOL minimize = FALSE;

  if (op > OP_BRA)
    {
    int number = op - OP_BRA;
    int offset = number << 1;

    if (offset < md->offset_max)
      {
      int save_offset1 = md->offset_vector[offset];
      int save_offset2 = md->offset_vector[offset+1];
      int save_offset3 = md->offset_vector[md->offset_end - number];

      md->offset_vector[md->offset_end - number] = eptr - md->start_subject;

      do
        {
        if (match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup))
          return TRUE;
        ecode += (ecode[1] << 8) + ecode[2];
        }
      while (*ecode == OP_ALT);

      md->offset_vector[offset] = save_offset1;
      md->offset_vector[offset+1] = save_offset2;
      md->offset_vector[md->offset_end - number] = save_offset3;
      return FALSE;
      }

    else op = OP_BRA;
    }

  switch(op)
    {
    case OP_BRA:
    do
      {
      if (match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup))
        return TRUE;
      ecode += (ecode[1] << 8) + ecode[2];
      }
    while (*ecode == OP_ALT);
    return FALSE;

    case OP_COND:
    if (ecode[3] == OP_CREF)
      {
      int offset = ecode[4] << 1;
      return match(eptr,
        ecode + ((offset < offset_top && md->offset_vector[offset] >= 0)?
          5 : 3 + (ecode[1] << 8) + ecode[2]),
        offset_top, md, ims, eptrb, match_isgroup);
      }

    else
      {
      if (match(eptr, ecode+3, offset_top, md, ims, NULL,
          match_condassert | match_isgroup))
        {
        ecode += 3 + (ecode[4] << 8) + ecode[5];
        while (*ecode == OP_ALT) ecode += (ecode[1] << 8) + ecode[2];
        }
      else ecode += (ecode[1] << 8) + ecode[2];
      return match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup);
      }
    case OP_CREF:
    ecode += 2;
    break;

    case OP_END:
    if (md->notempty && eptr == md->start_match) return FALSE;
    md->end_match_ptr = eptr;
    md->end_offset_top = offset_top;
    return TRUE;

    case OP_OPT:
    ims = ecode[1];
    ecode += 2;
    break;

    case OP_ASSERT:
    case OP_ASSERTBACK:
    do
      {
      if (match(eptr, ecode+3, offset_top, md, ims, NULL, match_isgroup)) break;
      ecode += (ecode[1] << 8) + ecode[2];
      }
    while (*ecode == OP_ALT);
    if (*ecode == OP_KET) return FALSE;

    if ((flags & match_condassert) != 0) return TRUE;

    do ecode += (ecode[1] << 8) + ecode[2]; while (*ecode == OP_ALT);
    ecode += 3;
    offset_top = md->end_offset_top;
    continue;

    case OP_ASSERT_NOT:
    case OP_ASSERTBACK_NOT:
    do
      {
      if (match(eptr, ecode+3, offset_top, md, ims, NULL, match_isgroup))
        return FALSE;
      ecode += (ecode[1] << 8) + ecode[2];
      }
    while (*ecode == OP_ALT);

    if ((flags & match_condassert) != 0) return TRUE;

    ecode += 3;
    continue;

    case OP_REVERSE:
#ifdef SUPPORT_UTF8
    c = (ecode[1] << 8) + ecode[2];
    for (i = 0; i < c; i++)
      {
      eptr--;
      BACKCHAR(eptr)
      }
#else
    eptr -= (ecode[1] << 8) + ecode[2];
#endif

    if (eptr < md->start_subject) return FALSE;
    ecode += 3;
    break;

    case OP_RECURSE:
      {
      BOOL rc;
      int *save;
      int stacksave[15];

      c = md->offset_max;

      if (c < 16) save = stacksave; else
        {
        save = (int *)(pcre_malloc)((c+1) * sizeof(int));
        if (save == NULL)
          {
          save = stacksave;
          c = 15;
          }
        }

      for (i = 1; i <= c; i++)
        save[i] = md->offset_vector[md->offset_end - i];
      rc = match(eptr, md->start_pattern, offset_top, md, ims, eptrb,
        match_isgroup);
      for (i = 1; i <= c; i++)
        md->offset_vector[md->offset_end - i] = save[i];
      if (save != stacksave) (pcre_free)(save);
      if (!rc) return FALSE;

      offset_top = md->end_offset_top;
      eptr = md->end_match_ptr;
      ecode++;
      }
    break;

    case OP_ONCE:
      {
      const uschar *prev = ecode;
      const uschar *saved_eptr = eptr;

      do
        {
        if (match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup))
          break;
        ecode += (ecode[1] << 8) + ecode[2];
        }
      while (*ecode == OP_ALT);

      if (*ecode != OP_ONCE && *ecode != OP_ALT) return FALSE;

      do ecode += (ecode[1] << 8) + ecode[2]; while (*ecode == OP_ALT);

      offset_top = md->end_offset_top;
      eptr = md->end_match_ptr;

      if (*ecode == OP_KET || eptr == saved_eptr)
        {
        ecode += 3;
        break;
        }

      if (ecode[3] == OP_OPT)
        {
        ims = (ims & ~PCRE_IMS) | ecode[4];
        }

      if (*ecode == OP_KETRMIN)
        {
        if (match(eptr, ecode+3, offset_top, md, ims, eptrb, 0) ||
            match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup))
              return TRUE;
        }
      else
        {
        if (match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup) ||
            match(eptr, ecode+3, offset_top, md, ims, eptrb, 0)) return TRUE;
        }
      }
    return FALSE;

    case OP_ALT:
    do ecode += (ecode[1] << 8) + ecode[2]; while (*ecode == OP_ALT);
    break;

    case OP_BRAZERO:
      {
      const uschar *next = ecode+1;
      if (match(eptr, next, offset_top, md, ims, eptrb, match_isgroup))
        return TRUE;
      do next += (next[1] << 8) + next[2]; while (*next == OP_ALT);
      ecode = next + 3;
      }
    break;

    case OP_BRAMINZERO:
      {
      const uschar *next = ecode+1;
      do next += (next[1] << 8) + next[2]; while (*next == OP_ALT);
      if (match(eptr, next+3, offset_top, md, ims, eptrb, match_isgroup))
        return TRUE;
      ecode++;
      }
    break;

    case OP_KET:
    case OP_KETRMIN:
    case OP_KETRMAX:
      {
      const uschar *prev = ecode - (ecode[1] << 8) - ecode[2];
      const uschar *saved_eptr = eptrb->saved_eptr;

      eptrb = eptrb->prev;

      if (*prev == OP_ASSERT || *prev == OP_ASSERT_NOT ||
          *prev == OP_ASSERTBACK || *prev == OP_ASSERTBACK_NOT ||
          *prev == OP_ONCE)
        {
        md->end_match_ptr = eptr;
        md->end_offset_top = offset_top;
        return TRUE;
        }

      if (*prev != OP_COND)
        {
        int number = *prev - OP_BRA;
        int offset = number << 1;

        if (number > 0)
          {
          if (offset >= md->offset_max) md->offset_overflow = TRUE; else
            {
            md->offset_vector[offset] =
              md->offset_vector[md->offset_end - number];
            md->offset_vector[offset+1] = eptr - md->start_subject;
            if (offset_top <= offset) offset_top = offset + 2;
            }
          }
        }

      ims = original_ims;

      if (*ecode == OP_KET || eptr == saved_eptr)
        {
        ecode += 3;
        break;
        }

      if (*ecode == OP_KETRMIN)
        {
        if (match(eptr, ecode+3, offset_top, md, ims, eptrb, 0) ||
            match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup))
              return TRUE;
        }
      else
        {
        if (match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup) ||
            match(eptr, ecode+3, offset_top, md, ims, eptrb, 0)) return TRUE;
        }
      }
    return FALSE;

    case OP_CIRC:
    if (md->notbol && eptr == md->start_subject) return FALSE;
    if ((ims & PCRE_MULTILINE) != 0)
      {
      if (eptr != md->start_subject && eptr[-1] != '\n') return FALSE;
      ecode++;
      break;
      }

    case OP_SOD:
    if (eptr != md->start_subject) return FALSE;
    ecode++;
    break;

    case OP_DOLL:
    if ((ims & PCRE_MULTILINE) != 0)
      {
      if (eptr < md->end_subject) { if (*eptr != '\n') return FALSE; }
        else { if (md->noteol) return FALSE; }
      ecode++;
      break;
      }
    else
      {
      if (md->noteol) return FALSE;
      if (!md->endonly)
        {
        if (eptr < md->end_subject - 1 ||
           (eptr == md->end_subject - 1 && *eptr != '\n')) return FALSE;

        ecode++;
        break;
        }
      }

    case OP_EOD:
    if (eptr < md->end_subject) return FALSE;
    ecode++;
    break;

    case OP_EODN:
    if (eptr < md->end_subject - 1 ||
       (eptr == md->end_subject - 1 && *eptr != '\n')) return FALSE;
    ecode++;
    break;

    case OP_NOT_WORD_BOUNDARY:
    case OP_WORD_BOUNDARY:
      {
      BOOL prev_is_word = (eptr != md->start_subject) &&
        ((md->ctypes[eptr[-1]] & ctype_word) != 0);
      BOOL cur_is_word = (eptr < md->end_subject) &&
        ((md->ctypes[*eptr] & ctype_word) != 0);
      if ((*ecode++ == OP_WORD_BOUNDARY)?
           cur_is_word == prev_is_word : cur_is_word != prev_is_word)
        return FALSE;
      }
    break;

    case OP_ANY:
    if ((ims & PCRE_DOTALL) == 0 && eptr < md->end_subject && *eptr == '\n')
      return FALSE;
    if (eptr++ >= md->end_subject) return FALSE;
#ifdef SUPPORT_UTF8
    if (md->utf8)
      while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
#endif
    ecode++;
    break;

    case OP_NOT_DIGIT:
    if (eptr >= md->end_subject ||
       (md->ctypes[*eptr++] & ctype_digit) != 0)
      return FALSE;
    ecode++;
    break;

    case OP_DIGIT:
    if (eptr >= md->end_subject ||
       (md->ctypes[*eptr++] & ctype_digit) == 0)
      return FALSE;
    ecode++;
    break;

    case OP_NOT_WHITESPACE:
    if (eptr >= md->end_subject ||
       (md->ctypes[*eptr++] & ctype_space) != 0)
      return FALSE;
    ecode++;
    break;

    case OP_WHITESPACE:
    if (eptr >= md->end_subject ||
       (md->ctypes[*eptr++] & ctype_space) == 0)
      return FALSE;
    ecode++;
    break;

    case OP_NOT_WORDCHAR:
    if (eptr >= md->end_subject ||
       (md->ctypes[*eptr++] & ctype_word) != 0)
      return FALSE;
    ecode++;
    break;

    case OP_WORDCHAR:
    if (eptr >= md->end_subject ||
       (md->ctypes[*eptr++] & ctype_word) == 0)
      return FALSE;
    ecode++;
    break;

    case OP_REF:
      {
      int length;
      int offset = ecode[1] << 1;
      ecode += 2;

      length = (offset >= offset_top || md->offset_vector[offset] < 0)?
        md->end_subject - eptr + 1 :
        md->offset_vector[offset+1] - md->offset_vector[offset];

      switch (*ecode)
        {
        case OP_CRSTAR:
        case OP_CRMINSTAR:
        case OP_CRPLUS:
        case OP_CRMINPLUS:
        case OP_CRQUERY:
        case OP_CRMINQUERY:
        c = *ecode++ - OP_CRSTAR;
        minimize = (c & 1) != 0;
        min = rep_min[c];
        max = rep_max[c];
        if (max == 0) max = INT_MAX;
        break;

        case OP_CRRANGE:
        case OP_CRMINRANGE:
        minimize = (*ecode == OP_CRMINRANGE);
        min = (ecode[1] << 8) + ecode[2];
        max = (ecode[3] << 8) + ecode[4];
        if (max == 0) max = INT_MAX;
        ecode += 5;
        break;

        default:
        if (!match_ref(offset, eptr, length, md, ims)) return FALSE;
        eptr += length;
        continue;
        }

      if (length == 0) continue;

      for (i = 1; i <= min; i++)
        {
        if (!match_ref(offset, eptr, length, md, ims)) return FALSE;
        eptr += length;
        }

      if (min == max) continue;

      if (minimize)
        {
        for (i = min;; i++)
          {
          if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
          if (i >= max || !match_ref(offset, eptr, length, md, ims))
            return FALSE;
          eptr += length;
          }
        }

      else
        {
        const uschar *pp = eptr;
        for (i = min; i < max; i++)
          {
          if (!match_ref(offset, eptr, length, md, ims)) break;
          eptr += length;
          }
        while (eptr >= pp)
          {
          if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
          eptr -= length;
          }
        return FALSE;
        }
      }

    case OP_CLASS:
      {
      const uschar *data = ecode + 1;
      ecode += 33;

      switch (*ecode)
        {
        case OP_CRSTAR:
        case OP_CRMINSTAR:
        case OP_CRPLUS:
        case OP_CRMINPLUS:
        case OP_CRQUERY:
        case OP_CRMINQUERY:
        c = *ecode++ - OP_CRSTAR;
        minimize = (c & 1) != 0;
        min = rep_min[c];
        max = rep_max[c];
        if (max == 0) max = INT_MAX;
        break;

        case OP_CRRANGE:
        case OP_CRMINRANGE:
        minimize = (*ecode == OP_CRMINRANGE);
        min = (ecode[1] << 8) + ecode[2];
        max = (ecode[3] << 8) + ecode[4];
        if (max == 0) max = INT_MAX;
        ecode += 5;
        break;

        default:
        min = max = 1;
        break;
        }

      for (i = 1; i <= min; i++)
        {
        if (eptr >= md->end_subject) return FALSE;
        GETCHARINC(c, eptr)

#ifdef SUPPORT_UTF8
        if (c > 255) return FALSE;
#endif

        if ((data[c/8] & (1 << (c&7))) != 0) continue;
        return FALSE;
        }

      if (min == max) continue;

      if (minimize)
        {
        for (i = min;; i++)
          {
          if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
          if (i >= max || eptr >= md->end_subject) return FALSE;
          GETCHARINC(c, eptr)

#ifdef SUPPORT_UTF8
          if (c > 255) return FALSE;
#endif
          if ((data[c/8] & (1 << (c&7))) != 0) continue;
          return FALSE;
          }
        }

      else
        {
        const uschar *pp = eptr;
        int len = 1;
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject) break;
          GETCHARLEN(c, eptr, len)

#ifdef SUPPORT_UTF8
          if (c > 255) break;
#endif
          if ((data[c/8] & (1 << (c&7))) == 0) break;
          eptr += len;
          }

        while (eptr >= pp)
          {
          if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;

#ifdef SUPPORT_UTF8
          BACKCHAR(eptr)
#endif
          }
        return FALSE;
        }
      }

    case OP_CHARS:
      {
      register int length = ecode[1];
      ecode += 2;

      if (length > md->end_subject - eptr) return FALSE;
      if ((ims & PCRE_CASELESS) != 0)
        {
        while (length-- > 0)
          if (md->lcc[*ecode++] != md->lcc[*eptr++])
            return FALSE;
        }
      else
        {
        while (length-- > 0) if (*ecode++ != *eptr++) return FALSE;
        }
      }
    break;

    case OP_EXACT:
    min = max = (ecode[1] << 8) + ecode[2];
    ecode += 3;
    goto REPEATCHAR;

    case OP_UPTO:
    case OP_MINUPTO:
    min = 0;
    max = (ecode[1] << 8) + ecode[2];
    minimize = *ecode == OP_MINUPTO;
    ecode += 3;
    goto REPEATCHAR;

    case OP_STAR:
    case OP_MINSTAR:
    case OP_PLUS:
    case OP_MINPLUS:
    case OP_QUERY:
    case OP_MINQUERY:
    c = *ecode++ - OP_STAR;
    minimize = (c & 1) != 0;
    min = rep_min[c];
    max = rep_max[c];
    if (max == 0) max = INT_MAX;

    REPEATCHAR:
    if (min > md->end_subject - eptr) return FALSE;
    c = *ecode++;

    if ((ims & PCRE_CASELESS) != 0)
      {
      c = md->lcc[c];
      for (i = 1; i <= min; i++)
        if (c != md->lcc[*eptr++]) return FALSE;
      if (min == max) continue;
      if (minimize)
        {
        for (i = min;; i++)
          {
          if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
          if (i >= max || eptr >= md->end_subject ||
              c != md->lcc[*eptr++])
            return FALSE;
          }
        }
      else
        {
        const uschar *pp = eptr;
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || c != md->lcc[*eptr]) break;
          eptr++;
          }
        while (eptr >= pp)
          if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
        return FALSE;
        }
      }
    else
      {
      for (i = 1; i <= min; i++) if (c != *eptr++) return FALSE;
      if (min == max) continue;
      if (minimize)
        {
        for (i = min;; i++)
          {
          if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
          if (i >= max || eptr >= md->end_subject || c != *eptr++) return FALSE;
          }
        }
      else
        {
        const uschar *pp = eptr;
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || c != *eptr) break;
          eptr++;
          }
        while (eptr >= pp)
         if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
           return TRUE;
        return FALSE;
        }
      }

    case OP_NOT:
    if (eptr >= md->end_subject) return FALSE;
    ecode++;
    if ((ims & PCRE_CASELESS) != 0)
      {
      if (md->lcc[*ecode++] == md->lcc[*eptr++]) return FALSE;
      }
    else
      {
      if (*ecode++ == *eptr++) return FALSE;
      }
    break;

    case OP_NOTEXACT:
    min = max = (ecode[1] << 8) + ecode[2];
    ecode += 3;
    goto REPEATNOTCHAR;

    case OP_NOTUPTO:
    case OP_NOTMINUPTO:
    min = 0;
    max = (ecode[1] << 8) + ecode[2];
    minimize = *ecode == OP_NOTMINUPTO;
    ecode += 3;
    goto REPEATNOTCHAR;

    case OP_NOTSTAR:
    case OP_NOTMINSTAR:
    case OP_NOTPLUS:
    case OP_NOTMINPLUS:
    case OP_NOTQUERY:
    case OP_NOTMINQUERY:
    c = *ecode++ - OP_NOTSTAR;
    minimize = (c & 1) != 0;
    min = rep_min[c];
    max = rep_max[c];
    if (max == 0) max = INT_MAX;

    REPEATNOTCHAR:
    if (min > md->end_subject - eptr) return FALSE;
    c = *ecode++;

    if ((ims & PCRE_CASELESS) != 0)
      {
      c = md->lcc[c];
      for (i = 1; i <= min; i++)
        if (c == md->lcc[*eptr++]) return FALSE;
      if (min == max) continue;
      if (minimize)
        {
        for (i = min;; i++)
          {
          if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
          if (i >= max || eptr >= md->end_subject ||
              c == md->lcc[*eptr++])
            return FALSE;
          }
        }
      else
        {
        const uschar *pp = eptr;
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || c == md->lcc[*eptr]) break;
          eptr++;
          }
        while (eptr >= pp)
          if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
        return FALSE;
        }
      }

    else
      {
      for (i = 1; i <= min; i++) if (c == *eptr++) return FALSE;
      if (min == max) continue;
      if (minimize)
        {
        for (i = min;; i++)
          {
          if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
            return TRUE;
          if (i >= max || eptr >= md->end_subject || c == *eptr++) return FALSE;
          }
        }
      else
        {
        const uschar *pp = eptr;
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || c == *eptr) break;
          eptr++;
          }
        while (eptr >= pp)
         if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
           return TRUE;
        return FALSE;
        }
      }

    case OP_TYPEEXACT:
    min = max = (ecode[1] << 8) + ecode[2];
    minimize = TRUE;
    ecode += 3;
    goto REPEATTYPE;

    case OP_TYPEUPTO:
    case OP_TYPEMINUPTO:
    min = 0;
    max = (ecode[1] << 8) + ecode[2];
    minimize = *ecode == OP_TYPEMINUPTO;
    ecode += 3;
    goto REPEATTYPE;

    case OP_TYPESTAR:
    case OP_TYPEMINSTAR:
    case OP_TYPEPLUS:
    case OP_TYPEMINPLUS:
    case OP_TYPEQUERY:
    case OP_TYPEMINQUERY:
    c = *ecode++ - OP_TYPESTAR;
    minimize = (c & 1) != 0;
    min = rep_min[c];
    max = rep_max[c];
    if (max == 0) max = INT_MAX;

    REPEATTYPE:
    ctype = *ecode++;

    if (min > md->end_subject - eptr) return FALSE;
    if (min > 0) switch(ctype)
      {
      case OP_ANY:
#ifdef SUPPORT_UTF8
      if (md->utf8)
        {
        for (i = 1; i <= min; i++)
          {
          if (eptr >= md->end_subject ||
             (*eptr++ == '\n' && (ims & PCRE_DOTALL) == 0))
            return FALSE;
          while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
          }
        break;
        }
#endif
      if ((ims & PCRE_DOTALL) == 0)
        { for (i = 1; i <= min; i++) if (*eptr++ == '\n') return FALSE; }
      else eptr += min;
      break;

      case OP_NOT_DIGIT:
      for (i = 1; i <= min; i++)
        if ((md->ctypes[*eptr++] & ctype_digit) != 0) return FALSE;
      break;

      case OP_DIGIT:
      for (i = 1; i <= min; i++)
        if ((md->ctypes[*eptr++] & ctype_digit) == 0) return FALSE;
      break;

      case OP_NOT_WHITESPACE:
      for (i = 1; i <= min; i++)
        if ((md->ctypes[*eptr++] & ctype_space) != 0) return FALSE;
      break;

      case OP_WHITESPACE:
      for (i = 1; i <= min; i++)
        if ((md->ctypes[*eptr++] & ctype_space) == 0) return FALSE;
      break;

      case OP_NOT_WORDCHAR:
      for (i = 1; i <= min; i++)
        if ((md->ctypes[*eptr++] & ctype_word) != 0)
          return FALSE;
      break;

      case OP_WORDCHAR:
      for (i = 1; i <= min; i++)
        if ((md->ctypes[*eptr++] & ctype_word) == 0)
          return FALSE;
      break;
      }

    if (min == max) continue;

    if (minimize)
      {
      for (i = min;; i++)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0)) return TRUE;
        if (i >= max || eptr >= md->end_subject) return FALSE;

        c = *eptr++;
        switch(ctype)
          {
          case OP_ANY:
          if ((ims & PCRE_DOTALL) == 0 && c == '\n') return FALSE;
#ifdef SUPPORT_UTF8
          if (md->utf8)
            while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
#endif
          break;

          case OP_NOT_DIGIT:
          if ((md->ctypes[c] & ctype_digit) != 0) return FALSE;
          break;

          case OP_DIGIT:
          if ((md->ctypes[c] & ctype_digit) == 0) return FALSE;
          break;

          case OP_NOT_WHITESPACE:
          if ((md->ctypes[c] & ctype_space) != 0) return FALSE;
          break;

          case OP_WHITESPACE:
          if  ((md->ctypes[c] & ctype_space) == 0) return FALSE;
          break;

          case OP_NOT_WORDCHAR:
          if ((md->ctypes[c] & ctype_word) != 0) return FALSE;
          break;

          case OP_WORDCHAR:
          if ((md->ctypes[c] & ctype_word) == 0) return FALSE;
          break;
          }
        }
      }

    else
      {
      const uschar *pp = eptr;
      switch(ctype)
        {
        case OP_ANY:

#ifdef SUPPORT_UTF8
        if (md->utf8 && max < INT_MAX)
          {
          if ((ims & PCRE_DOTALL) == 0)
            {
            for (i = min; i < max; i++)
              {
              if (eptr >= md->end_subject || *eptr++ == '\n') break;
              while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
              }
            }
          else
            {
            for (i = min; i < max; i++)
              {
              eptr++;
              while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
              }
            }
          break;
          }
#endif
        if ((ims & PCRE_DOTALL) == 0)
          {
          for (i = min; i < max; i++)
            {
            if (eptr >= md->end_subject || *eptr == '\n') break;
            eptr++;
            }
          }
        else
          {
          c = max - min;
          if (c > md->end_subject - eptr) c = md->end_subject - eptr;
          eptr += c;
          }
        break;

        case OP_NOT_DIGIT:
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_digit) != 0)
            break;
          eptr++;
          }
        break;

        case OP_DIGIT:
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_digit) == 0)
            break;
          eptr++;
          }
        break;

        case OP_NOT_WHITESPACE:
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_space) != 0)
            break;
          eptr++;
          }
        break;

        case OP_WHITESPACE:
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_space) == 0)
            break;
          eptr++;
          }
        break;

        case OP_NOT_WORDCHAR:
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_word) != 0)
            break;
          eptr++;
          }
        break;

        case OP_WORDCHAR:
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_word) == 0)
            break;
          eptr++;
          }
        break;
        }

      while (eptr >= pp)
        {
        if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
#ifdef SUPPORT_UTF8
        if (md->utf8)
          while (eptr > pp && (*eptr & 0xc0) == 0x80) eptr--;
#endif
        }
      return FALSE;
      }

    default:
    md->errorcode = PCRE_ERROR_UNKNOWN_NODE;
    return FALSE;
    }
  }
}

int
pcre_exec(const pcre *external_re, const pcre_extra *external_extra,
  const char *subject, int length, int start_offset, int options, int *offsets,
  int offsetcount)
{
int resetcount, ocount;
int first_char = -1;
int req_char = -1;
int req_char2 = -1;
unsigned long int ims = 0;
match_data match_block;
const uschar *start_bits = NULL;
const uschar *start_match = (const uschar *)subject + start_offset;
const uschar *end_subject;
const uschar *req_char_ptr = start_match - 1;
const real_pcre *re = (const real_pcre *)external_re;
const real_pcre_extra *extra = (const real_pcre_extra *)external_extra;
BOOL using_temporary_offsets = FALSE;
BOOL anchored = ((re->options | options) & PCRE_ANCHORED) != 0;
BOOL startline = (re->options & PCRE_STARTLINE) != 0;

if ((options & ~PUBLIC_EXEC_OPTIONS) != 0) return PCRE_ERROR_BADOPTION;

if (re == NULL || subject == NULL ||
   (offsets == NULL && offsetcount > 0)) return PCRE_ERROR_NULL;
if (re->magic_number != MAGIC_NUMBER) return PCRE_ERROR_BADMAGIC;

match_block.start_pattern = re->code;
match_block.start_subject = (const uschar *)subject;
match_block.end_subject = match_block.start_subject + length;
end_subject = match_block.end_subject;

match_block.endonly = (re->options & PCRE_DOLLAR_ENDONLY) != 0;
match_block.utf8 = (re->options & PCRE_UTF8) != 0;

match_block.notbol = (options & PCRE_NOTBOL) != 0;
match_block.noteol = (options & PCRE_NOTEOL) != 0;
match_block.notempty = (options & PCRE_NOTEMPTY) != 0;

match_block.errorcode = PCRE_ERROR_NOMATCH;

match_block.lcc = re->tables + lcc_offset;
match_block.ctypes = re->tables + ctypes_offset;

ims = re->options & (PCRE_CASELESS|PCRE_MULTILINE|PCRE_DOTALL);

ocount = offsetcount - (offsetcount % 3);

if (re->top_backref > 0 && re->top_backref >= ocount/3)
  {
  ocount = re->top_backref * 3 + 3;
  match_block.offset_vector = (int *)(pcre_malloc)(ocount * sizeof(int));
  if (match_block.offset_vector == NULL) return PCRE_ERROR_NOMEMORY;
  using_temporary_offsets = TRUE;
  }
else match_block.offset_vector = offsets;

match_block.offset_end = ocount;
match_block.offset_max = (2*ocount)/3;
match_block.offset_overflow = FALSE;

resetcount = 2 + re->top_bracket * 2;
if (resetcount > offsetcount) resetcount = ocount;

if (match_block.offset_vector != NULL)
  {
  register int *iptr = match_block.offset_vector + ocount;
  register int *iend = iptr - resetcount/2 + 1;
  while (--iptr >= iend) *iptr = -1;
  }

if (!anchored)
  {
  if ((re->options & PCRE_FIRSTSET) != 0)
    {
    first_char = re->first_char;
    if ((ims & PCRE_CASELESS) != 0) first_char = match_block.lcc[first_char];
    }
  else
    if (!startline && extra != NULL &&
      (extra->options & PCRE_STUDY_MAPPED) != 0)
        start_bits = extra->start_bits;
  }

if ((re->options & PCRE_REQCHSET) != 0)
  {
  req_char = re->req_char;
  req_char2 = ((re->options & (PCRE_CASELESS | PCRE_ICHANGED)) != 0)?
    (re->tables + fcc_offset)[req_char] : req_char;
  }

do
  {
  int rc;
  register int *iptr = match_block.offset_vector;
  register int *iend = iptr + resetcount;

  while (iptr < iend) *iptr++ = -1;

  if (first_char >= 0)
    {
    if ((ims & PCRE_CASELESS) != 0)
      while (start_match < end_subject &&
             match_block.lcc[*start_match] != first_char)
        start_match++;
    else
      while (start_match < end_subject && *start_match != first_char)
        start_match++;
    }

  else if (startline)
    {
    if (start_match > match_block.start_subject + start_offset)
      {
      while (start_match < end_subject && start_match[-1] != '\n')
        start_match++;
      }
    }

  else if (start_bits != NULL)
    {
    while (start_match < end_subject)
      {
      register int c = *start_match;
      if ((start_bits[c/8] & (1 << (c&7))) == 0) start_match++; else break;
      }
    }

  if (req_char >= 0)
    {
    register const uschar *p = start_match + ((first_char >= 0)? 1 : 0);

    if (p > req_char_ptr)
      {
      if (req_char == req_char2)
        {
        while (p < end_subject)
          {
          if (*p++ == req_char) { p--; break; }
          }
        }
      else
        {
        while (p < end_subject)
          {
          register int pp = *p++;
          if (pp == req_char || pp == req_char2) { p--; break; }
          }
        }
      if (p >= end_subject) break;

      req_char_ptr = p;
      }
    }

  match_block.start_match = start_match;
  if (!match(start_match, re->code, 2, &match_block, ims, NULL, match_isgroup))
    continue;

  if (using_temporary_offsets)
    {
    if (offsetcount >= 4)
      {
      memcpy(offsets + 2, match_block.offset_vector + 2,
        (offsetcount - 2) * sizeof(int));
      }
    if (match_block.end_offset_top > offsetcount)
      match_block.offset_overflow = TRUE;

    (pcre_free)(match_block.offset_vector);
    }

  rc = match_block.offset_overflow? 0 : match_block.end_offset_top/2;

  if (match_block.offset_end < 2) rc = 0; else
    {
    offsets[0] = start_match - match_block.start_subject;
    offsets[1] = match_block.end_match_ptr - match_block.start_subject;
    }

  return rc;
  }

while (!anchored &&
       match_block.errorcode == PCRE_ERROR_NOMATCH &&
       start_match++ < end_subject);

if (using_temporary_offsets)
  {
  (pcre_free)(match_block.offset_vector);
  }

return match_block.errorcode;
}
#endif  /* HAVE_PCRE */
