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

/*** utils.c; lots of little functions to do odd little things ***/

#include "anlghea3.h"

void my_exit(int status) {
  extern FILE *errfile;
#ifndef NODNS
  extern FILE *dnslock;
  extern char *dnslockfile;
#endif

#ifdef MAC_EVENTS
  MacFini();
#endif
#ifdef WIN32
  Win32Cleanup();
#endif
#ifndef NODNS
  if (dnslock != NULL) {
    fclose(dnslock);
    if (remove(dnslockfile) != 0)
      warn('F', TRUE, "Trouble deleting DNS lock file %s", dnslockfile);
    else
      debug('F', "Deleting DNS lock file %s", dnslockfile);
  }
#endif
  if (errfile != stderr)
    fclose(errfile);
  exit(status);
}

void sighandler(int sig) {
  extern FILE *errfile;
  extern logical anywarns;
  extern char *commandname;

#ifndef NOALARM
  extern JMP_BUF jumpbuf;
  if (sig == SIGALRM)
    LONGJMP(jumpbuf, 1);
#endif
  fprintf(errfile, "%s: Received ", commandname);
  if (sig == SIGTERM)
    fprintf(errfile, "termination");
  else if (sig == SIGINT)
    fprintf(errfile, "interrupt");
#ifdef SIGHUP
  else if (sig == SIGHUP)
    fprintf(errfile, "HUP");
#endif
#ifdef SIGQUIT
  else if (sig == SIGQUIT)
    fprintf(errfile, "quit");
#endif
  fprintf(errfile, " signal: exiting\n");

  if (errfile != stderr) {
    fprintf(stderr, "Received ");
    if (sig == SIGTERM)
      fprintf(stderr, "termination");
    else if (sig == SIGINT)
      fprintf(stderr, "interrupt");
#ifdef SIGHUP
    else if (sig == SIGHUP)
      fprintf(stderr, "HUP");
#endif
#ifdef SIGQUIT
    else if (sig == SIGQUIT)
      fprintf(stderr, "quit");
#endif
    fprintf(stderr, " signal: exiting\n");
  }

  anywarns = TRUE;
  my_exit(EXIT_SIGNAL);
}

logical substrcaseeq(char *s, char *t) {
  /* match the beginning of s case insensitively against lower case t */
  for ( ; TOLOWER(*s) == *t && *t != '\0'; s++, t++)
    ;       /* *s != '\0' covered by above two cases */
  if (*t == '\0')
    return(TRUE);
  else
    return(FALSE);
}

char *strtoupper(char *n) {
  /* convert a string to upper case UP TO the first line break */
  char *c;
  for (c = n; *c != '\0' && *c != '\n' && *c != '\r'; c++)
    if (ISLOWER(*c))   /* needed to stop (known u.c.) const char *n crashing */
      *c = TOUPPER(*c);
  return(n);
}

void strtolowerx(char *n) { /* lower-case filename: only used in next 3 fns. */
  for ( ; *n != '\0' && *n != '?'; n++)
    *n = TOLOWER(*n);
}

void toloweralias(Alias *ap, logical both) {
  AliasTo *tp;
  char *errstr;
  int erroffset;

  for ( ; ap != NULL; TO_NEXT(ap)) {
    if (headcasematch(ap->from, "REGEXP:"))
      ap->pattern = pcre_compile(ap->from + 7, PCRE_CASELESS | PCRE_DOTALL,
				 (const char **)(&errstr), &erroffset, NULL);
    /* if original regcomp succeeded, this one should too */
    else if (!headcasematch(ap->from, "REGEXPI:"))
      /* nothing to do for REGEXPI: */
      strtolowerx(ap->from);
    if (both) {
      for (tp = ap->to; tp != NULL; TO_NEXT(tp))
	strtolowerx(tp->string);
    }
  }
}

void tolowerinc(Include *ip) {
  /* see comments in previous function */
  char *errstr;
  int erroffset;

  for ( ; ip != NULL; TO_NEXT(ip)) {
    if (headcasematch(ip->name, "REGEXP:"))
      ip->pattern = pcre_compile(ip->name + 7, PCRE_CASELESS | PCRE_DOTALL,
				 (const char **)(&errstr), &erroffset, NULL);
    else if (!headcasematch(ip->name, "REGEXPI:"))
      strtolowerx((char *)(ip->name));
  }
}

void tolowerse(Strpairlist *sp) {  /* for (internal) search engines */
  for ( ; sp != NULL; TO_NEXT(sp))
    strtolowerx(sp->name);
}

void reversealias(Alias **head) {
  /* aliases are first collated in reverse order */
  Alias *ap, *lastap = NULL, *nextap;

  for (ap = *head; ap != NULL; ap = nextap) {
    nextap = ap->next;
    ap->next = lastap;
    lastap = ap;
  }
  *head = lastap;
}

/* Swap an strpairlist round. Same as reversealias. Who needs templates?? */
void reversestrpairlist(Strpairlist **head) {
  Strpairlist *sp, *lastsp = NULL, *nextsp;

  for (sp = *head; sp != NULL; sp = nextsp) {
    nextsp = sp->next;
    sp->next = lastsp;
    lastsp = sp;
  }
  *head = lastsp;
}

logical strcaseeq(char *s, char *t) {
  for ( ; TOUPPER(*s) == TOUPPER(*t) && *s != '\0'; s++, t++)
    ;
  if (TOUPPER(*s) != TOUPPER(*t))
    return(FALSE);
  else
    return(TRUE);
}

char *strrpbrk(char *s, char *t) {
  /* locate the last occurence in s of any character in t */
  char *u;

  for (u = strchr(s, '\0') - 1; u >= s; u--) {
    if (strchr(t, *u) != NULL)
      return(u);
  }
  return(NULL);
}

#ifdef EBCDIC
char *strtoascii(char *s) {
  extern unsigned char os_toascii[];
  char *t;

  for (t = s; *t != '\0'; t++)
    *t = (char)os_toascii[(unsigned char)(*t)];
  return(s);
}
#endif

char *buildfilename(char *old, char *sub, char *name) {
  /* Here's how we construct many filenames. If the "name" contains one of the
     PATHSEPS, then just use the "name" (i.e., relative to the working dir).
     Otherwise, put it in the same directory as "old". If "old" is NULL, then
     use the subdirectory "sub" of the location of the analog binary as "old".
     We always malloc new space to hold the name. */
  /* The calling routine should check any special cases appropriate to that
     type of file, e.g., "none", "-", "stdin" or "stdout". */
  extern char *commandpath;
  size_t t = 0, u;
  /* t is the length of sub, u of the directory of old */
  char *s, *ans;

  if (strpbrk(name, PATHSEPS) != NULL) {
    ans = (char *)xmalloc(strlen(name) + 1);
    strcpy(ans, name);
    return(ans);
  }
  if (old == NULL) {
    old = commandpath;
    t = strlen(sub);
  }
  if ((s = strrpbrk(old, PATHSEPS)) == NULL)
    u = 0;
  else
    u = ((s - old) + 1);
  ans = (char *)xmalloc(t + u + strlen(name) + 2);
  if (u != 0)
    strncpy(ans, old, u);
  if (t != 0) {
    sprintf(ans + u, "%s%c", sub, DIRSEP);
    t++;
  }
  strcpy(ans + t + u, name);
  return(ans);
}

#ifdef NEED_STRCMP
int my_strcmp(char *s, char *t) {
  for ( ; *s == *t && *s != '\0'; s++, t++)
    ;
  if (*s < *t)
    return(-1);
  else if (*s > *t)
    return(1);
  else
    return(0);
}
#endif

unsigned int chrn(char *s, char c) {  /* no. of occurrences of c in s */
  unsigned int n = 0;
  for ( ; *s != '\0'; s++)
    n += (*s == c);
  return(n);
}

unsigned int chrdistn(char *s, char c) {/* ditto but consec c's count only 1 */
  unsigned int n = 0;
  for ( ; *s != '\0'; s++)
    n += (*s == c && *(s + 1) != c);
  return(n);
}

unsigned int log10i(unsigned long i) {
  unsigned int j;

  for (j = 0; i >= 10; i /= 10)
    j++;
  return(j);
}

unsigned int log10x(double d) {
  unsigned int j;

  for (j = 0; d >= 10; d /= 10)
    j++;
  return(j);
}

/* If 'd' bytes are displayed to 'dp' decimal places, should they be written
   as kilobytes, megabytes, or what? */
unsigned int findbmult(double d, unsigned int dp) {
  unsigned int j;
  double threshold;

  if (d < 1024)
    return(0);  /* see also in calcsizes() */

  /* Bytes will be rounded to 'dp' decimal places. So the theshold at which
     we move to a new multiplier is 999.5 if dp = 0, 999.95 if dp = 1 etc. */
  threshold = 0.5;
  for (j = 0; j < dp; j++)
    threshold /= 10.0;
  threshold = 1000.0 - threshold;

  /* Now we can find the multiplier */
  for (j = 0; d >= threshold; d /= 1024.0)
    j++;
  return(j);
}

/* Convert a string to integer between 0 and 255, with some error checking
   built in. Returns -1 if string empty, or not all digits, or greater than
   255. */
int atoi255(const char *s) {
  int ans;

  if (*s == '\0')
    return(-1);
  for (ans = 0; ISDIGIT(*s); s++) {
    ans *= 10;
    ans += *s - '0';
    if (ans > 255)
      return(-1);
  }
  if (*s != '\0')
    return(-1);
  return(ans);
}

unsigned long arraymaxl(unsigned long *x, unsigned int size) {
  unsigned long y;
  unsigned int i;

  y = 0;
  for (i = 0; i < size; i++)
    y = MAX(y, x[i]);
  return(y);
}

double arraymaxd(double *x, unsigned int size) {
  double y;       /* assuming doubles in x all positive */
  unsigned int i;

  y = 0.0;
  for (i = 0; i < size; i++)
    y = MAX(y, x[i]);
  return(y);
}

size_t arraymaxlen(char **s, unsigned int size, Outchoices *od) {
  unsigned int i;
  size_t y = 0;

  size_t (*strlenp)(const char *);

  if (od == NULL)
    strlenp = &strlen;
  else
    strlenp = od->outputter->strlength;

  for (i = 0; i < size; i++)
    y = MAX(strlenp(s[i]), y);

  return(y);
}

void prettyprintf(FILE *outf, unsigned int pagewidth, char *fmt, va_list ap) {
  /* NB Calling function must call mprintf with pagewidth = 0 after message. */
  char m[511];  /* longer than any message we want to produce */
  extern unsigned int ppcol;
  static logical spacedone = FALSE;
  char *m1, *m2;

  if (pagewidth == 0) {
    putc('\n', outf);
    ppcol = 0;
    spacedone = FALSE;
    return;
  }
  vsprintf(m, fmt, ap);
  for (m1 = m, m2 = m; *m1 != '\0'; m2++) {
    if (*m2 == ' ' || *m2 == '\0' || *m2 == '\n') {
      if ((m2 - m1) + ppcol > pagewidth && (*m1 == ' ' || spacedone)) {
	fputs("\n  ", outf);
	if (*m1 == ' ')
	  m1++;
	spacedone = FALSE;
	ppcol = 1;
      }
      while (m1 < m2) {
	putc(*m1, outf);
	spacedone = (*(m1++) == ' ');
	ppcol++;
      }
      if (*m1 == '\n') {
	fputs("\n  ", outf);
	m1++;
	spacedone = FALSE;
	ppcol = 2;
      }
    }
  }
}

void mprintf(FILE *outf, unsigned int pagewidth, char *fmt, ...) {
  /* wrapper to prettyprintf(): see note there */
  va_list ap;

  va_start(ap, fmt);
  prettyprintf(outf, pagewidth, fmt, ap);
  va_end(ap);
}

void print_progversion(FILE *f) {
  extern char *commandname, *starttimestr;
  extern logical cgi;
  static logical done = FALSE;

  if (!done) {
    if (cgi)
      fprintf(f, "[%s] ", starttimestr);
    fprintf(f, "%s: Analog CE version %s\n", commandname, VERSION);
    done = TRUE;
  }
}

void warn(char c, choice domess, char *s, ...) {
  extern FILE *errfile;
  extern char *warn_args;
  extern logical anywarns, errmess, cgi;
  extern unsigned int errwidth;
  extern char *commandname, *starttimestr;

  va_list ap;

  if (strchr(warn_args, c) != NULL) {
    if (errwidth == 0)
      errwidth = UINT_MAX;
    print_progversion(errfile);
    va_start(ap, s);
    if (cgi)
      mprintf(errfile, errwidth, "[%s] ", starttimestr);
    if (domess == CONTINUATION)
      mprintf(errfile, errwidth, "%s: ...cont..: ", commandname, c);
    else
      mprintf(errfile, errwidth, "%s: Warning %c: ", commandname, c);
    prettyprintf(errfile, errwidth, s, ap);
    mprintf(errfile, 0, NULL);
    if (!errmess && domess) {
      fputs("  (For help on all errors and warnings,", errfile);
      fputs(" see docs/errors.html)\n", errfile);
      errmess = TRUE;
    }
    va_end(ap);
    anywarns = TRUE;
  }
}

void error(char *s, ...) {
  extern FILE *errfile;
  extern logical anywarns, errmess, cgi;
  extern unsigned int errwidth;
  extern char *commandname, *starttimestr;

  va_list ap;

  if (errwidth == 0)
    errwidth = UINT_MAX;
  print_progversion(errfile);
  va_start(ap, s);
  if (cgi)
    mprintf(errfile, errwidth, "[%s] ", starttimestr);
  mprintf(errfile, errwidth, "%s: Fatal error: ", commandname);
  prettyprintf(errfile, errwidth, s, ap);
  mprintf(errfile, errwidth, ": exiting");
  mprintf(errfile, 0, NULL);
  if (!errmess) {
    fputs("  (For help on all errors and warnings,", errfile);
    fputs(" see docs/errors.html)\n", errfile);
    errmess = TRUE;
  }
  if (errfile != stderr) {
    if (cgi)
      mprintf(stderr, STDERRWIDTH, "[%s] ", starttimestr);
    mprintf(stderr, STDERRWIDTH, "%s: Fatal error: ", commandname);
    prettyprintf(stderr, STDERRWIDTH, s, ap);
    mprintf(stderr, STDERRWIDTH, ": exiting");
    mprintf(stderr, 0, NULL);
  }
  va_end(ap);
  anywarns = TRUE;

  my_exit(EXIT_FAILURE);
}

void debug(char c, char *s, ...) {
  extern FILE *errfile;
  extern char *debug_args;
  extern logical anywarns, cgi;
  extern char *commandname, *starttimestr;

  va_list ap;

  if (strchr(debug_args, c) != NULL) {
    print_progversion(errfile);
    va_start(ap, s);
    if (cgi)
      fprintf(errfile, "[%s] %s: Debug ", starttimestr, commandname);
    fprintf(errfile, "%c: ", c);
    vfprintf(errfile, s, ap);
    fputc('\n', errfile);
    va_end(ap);
    if (c != 'D')
      anywarns = TRUE;
  }
}

char *delimit(char *s) {
  /* This function delimits an argument s so that it could appear in a config
     command. It malloc's as necessary: only used in warning messages */
  char *ans;
  size_t len;

  if (*s != '\'' && *s != '"' && *s != '(' && strchr(s, ' ') == NULL &&
      strchr(s, '#') == NULL)
    return(s);

  len = strlen(s);
  ans = (char *)xmalloc(len + 3);
  strcpy(ans + 1, s);
  if (strchr(s, '"') == NULL) {
    *ans = '"';
    ans[len + 1] = '"';
  }
  else if (strchr(s, ')') == NULL) {
    *ans = '(';
    ans[len + 1] = ')';
  }
  else {
    *ans = '\'';
    ans[len + 1] = '\'';
  }
  ans[len + 2] = '\0';
  return(ans);
}

void *xmalloc(size_t size) {
  /* the same as malloc, only checks for out of memory */
  void *answer;

  if ((answer = malloc(size)) == NULL)
    error("Ran out of memory: cannot continue");

  /* Enable this for debugging: memset(answer, 35, size); */
  return(answer);
}

void *xrealloc(void *ptr, size_t size) {

  if (ptr == NULL)  /* Some broken systems don't allow realloc(NULL, ...) */
    ptr = malloc(size);
  else
    ptr = realloc(ptr, size);
  if (ptr == NULL)
    error("Ran out of memory: cannot continue");

  return(ptr);
}

void *submalloc(Memman *m, size_t size) {
  Mmlist *l;
  /* so memmans are initialised with curr_pos, first = NULL & alignment set */

  size = ((size + m->alignment - 1) / m->alignment) * m->alignment;
  if (m->curr_pos != NULL &&
      (size_t)((char *)(m->block_end) - (char *)(m->next_pos)) >= size)
    m->curr_pos = m->next_pos;
  else {
    m->curr_pos = xmalloc(BLOCKSIZE);
    m->block_end = (void *)((char *)(m->curr_pos) + BLOCKSIZE);
    if (m->first == NULL) {  /* initialisation only */
      m->first = (Mmlist *)xmalloc(sizeof(Mmlist));
      m->first->pos = m->curr_pos;
      m->first->next = NULL;
      m->last = m->first;
    }
    else {
      l = (Mmlist *)xmalloc(sizeof(Mmlist));
      l->pos = m->curr_pos;
      l->next = NULL;
      m->last->next = l;
      TO_NEXT(m->last);
    }
  }
  m->next_pos = (void *)((char *)(m->curr_pos) + size);
  return(m->curr_pos);
}

void freemm(Memman *m) {
  Mmlist *p;

  for (p = m->first; p != NULL; TO_NEXT(p))
    free(p->pos);
}

/* The next function is derived from the FreeBSD 2.2.6 source, and is subject
   to the copyright notice in docs/LicBSD.txt within the analog distribution.
 */

#ifdef NEED_MEMMOVE
/*
 * sizeof(word) MUST BE A POWER OF TWO
 * SO THAT wmask BELOW IS ALL ONES
 */
typedef	int word;		/* "word" used for optimal copy speed */
#define	wsize	sizeof(word)
#define	wmask	(wsize - 1)

void *memmove(void *dst0, const void *src0, size_t length) {
  char *dst = dst0;
  const char *src = src0;
  size_t t;

  if (length == 0 || dst == src)		/* nothing to do */
    goto done;

  /*
   * Macros: loop-t-times; and loop-t-times, t>0
   */
#define	TLOOP(s) if (t) TLOOP1(s)
#define	TLOOP1(s) do { s; } while (--t)

  if ((unsigned long)dst < (unsigned long)src) {
    /*
     * Copy forward.
     */
    t = (int)src;	/* only need low bits */
    if ((t | (int)dst) & wmask) {
      /*
       * Try to align operands.  This cannot be done
       * unless the low bits match.
       */
      if ((t ^ (int)dst) & wmask || length < wsize)
	t = length;
      else
	t = wsize - (t & wmask);
      length -= t;
      TLOOP1(*dst++ = *src++);
    }
    /*
     * Copy whole words, then mop up any trailing bytes.
     */
    t = length / wsize;
    TLOOP(*(word *)dst = *(word *)src; src += wsize; dst += wsize);
    t = length & wmask;
    TLOOP(*dst++ = *src++);
  } else {
    /*
     * Copy backwards.  Otherwise essentially the same.
     * Alignment works as before, except that it takes
     * (t&wmask) bytes to align, not wsize-(t&wmask).
     */
    src += length;
    dst += length;
    t = (int)src;
    if ((t | (int)dst) & wmask) {
      if ((t ^ (int)dst) & wmask || length <= wsize)
	t = length;
      else
	t &= wmask;
      length -= t;
      TLOOP1(*--dst = *--src);
    }
    t = length / wsize;
    TLOOP(src -= wsize; dst -= wsize; *(word *)dst = *(word *)src);
    t = length & wmask;
    TLOOP(*--dst = *--src);
  }
 done:
  return (dst0);
}
#endif

#define SETPMATCH(ptr1, ptr2) \
  if (nmatch > 0) { \
    pmatch[0] = (int)(ptr1 - whole); \
    pmatch[1] = (int)(ptr2 - whole); \
  }

/* match string s against pattern p */
logical wildmatch(char *s, char *p, char *whole, int nmatch, int *pmatch) {
  char *ss, *pp;

  /* First match head portions */
  while ((*s == *p || *p == '?') && *s != '\0' && *p != '*') {
    s++;        /* *p != '\0' is covered by (*s == *p) && (*s != '\0') */
    p++;
  }
  /* Unless we've reached a pattern *, we've finished now */
  if (*p != '*') {
    if (*s == '\0' && *p == '\0')
      return(TRUE);
    else
      return(FALSE);
  }
  /* Otherwise, match the tail portions. This is purely for speed reasons,
     so that patterns with only one star don't have to go through the loop
     below. */
  ss = strchr(s, '\0');
  pp = strchr(p, '\0');
  while ((*ss == *pp || *pp == '?') && ss >= s && *pp != '*') {
    ss--;       /* pp != p is covered because *p == '*' */
    pp--;
  }
  /* Again, we might have finished now */
  if (pp == p) {
    SETPMATCH(s, ss + 1);
    return(TRUE);
  }
  else if (*pp != '*')
    return(FALSE);
  for (p++; *p == '*' && p != pp; p++)
    ;
  if (p == pp) {    /* there was really only one *, disguised as several */
    SETPMATCH(s, ss + 1);
    return(TRUE);
  }
  if (ss < s)
    return(FALSE);
  /* this leaves only the difficult case, where p contains >= 2 *'s. Try and
     match ANY tail substring of s against the bit of the pattern after the
     first * (or consec. *'s) (reducing *'s in p, guaranteeing termination).
     (p was incremented above). We choose to match tail portion afresh each
     time (cheap), rather than insert null bytes into (poss. const) s and p */
  for (ss = s; *ss != '\0'; ss++) {   /* reuse ss */
    if (wildmatch(ss, p, whole, nmatch - 2, pmatch + 2)) {
      SETPMATCH(s, ss);
      return(TRUE);
    }
  }
  return(FALSE);
}

logical matchq(char *s, void *p, logical is_regex, int *pmatch) {
  if (is_regex)
    return(pcre_exec((pcre *)p, NULL, s, (int)strlen(s), 0, 0, pmatch,
		     (pmatch == NULL)?0:PMATCH_SIZE) >= 0);
  else
    return(wildmatch(s, (char *)p, s, (pmatch == NULL)?0:PMATCH_SIZE, pmatch));
}

logical genwildmatch(char *s, char *s2, char *p) {
  /* Exactly the same as wildmatch, except with doubly delimited strings,
     and we don't need to know the matched portions. */
  char *ss, *pp;

  while ((*s == *p || *p == '?') && s < s2 && *p != '*' && *p != '\0') {
    s++;
    p++;
  }
  if (*p != '*') {
    if (s == s2 && *p == '\0')
      return(TRUE);
    else
      return(FALSE);
  }
  ss = s2;
  pp = strchr(p, '\0');
  while ((*ss == *pp || *pp == '?' || ss == s2) && ss >= s && *pp != '*') {
    ss--;
    pp--;
  }
  if (pp == p)
    return(TRUE);
  else if (*pp != '*')
    return(FALSE);
  for (p++; *p == '*' && p != pp; p++)
    ;
  if (p == pp)
    return(TRUE);
  if (ss < s)
    return(FALSE);
  for ( ; s < s2; s++) {
    if (genwildmatch(s, s2, p))
      return(TRUE);
  }
  return(FALSE);
}

logical headmatch(char *s, char *p) {  /* does s start with p? */

  for ( ; *p != '\0'; s++, p++) {
    if (*s != *p)
      return(FALSE);
  }
  return(TRUE);
}

logical headcasematch(char *s, char *p) {  /* same, only case insensitive */

  for ( ; *p != '\0'; s++, p++) {
    if (TOLOWER(*s) != TOLOWER(*p))
      return(FALSE);
  }
  return(TRUE);
}

/* Does a hostname match an IP address range? */
logical matchiprange(char *s, unsigned long minaddr, unsigned long maxaddr)
{
  unsigned long addr;
  int n, octet;

  /* Turn this host into a long. This is basically just the simple case from
     parseiprange() (including atoi255()), but a much faster version. */

  for (addr = 0, octet = 0; octet < 4; octet++) {
    if (!ISDIGIT(*s))
      return(FALSE);
    for (n = 0; ISDIGIT(*s); s++) {
      n *= 10;
      n += *s - '0';
      if (n > 255)
	return(FALSE);
    }
    if (octet < 3) {
      if (*s == '.')
	s++;
      else
	return(FALSE);
    }
    addr <<= 8;
    addr += n;
  }

  if (*s != '\0')
    return(FALSE);

  /* Is it in the right range? */
  return (minaddr <= addr && addr <= maxaddr);
}

/* Is an item included according to an INCLUDE/EXCLUDE list? */
logical included(char *name, logical ispage, Include *listhead) {
  logical is_ipaddr, is_regex, matches;
  Include *lp, *lastlp;

  if (listhead == NULL)
    return(TRUE);

  for (lp = listhead; lp != NULL; TO_NEXT(lp)) {
    is_ipaddr = IS_IPADDRT(lp->type);
    is_regex = IS_REGEXT(lp->type);
    if (!is_regex && !is_ipaddr && STREQ((char *)(lp->name), "pages"))
      /* 'pages' is made case insensitive at config time */
      matches = ispage;
    else if (is_ipaddr)
      matches = matchiprange(name, lp->minaddr, lp->maxaddr);
    else
      matches = matchq(name,
		       is_regex?((void *)(lp->pattern)):((void *)(lp->name)),
		       is_regex, NULL);
    if (matches)
      return(IS_INC(lp->type));
    lastlp = lp;
  }

  /* not listed: return opposite of first inc/exc */
  return(!IS_INC(lastlp->type));
}

/* Is an item included in an Strlist? */
logical incstrlist(char *name, Strlist *listhead) {
  Strlist *lp;

  for (lp = listhead; lp != NULL; TO_NEXT(lp)) {
    if (STREQ(name, lp->name))
      return(TRUE);
  }
  return(FALSE);
}

logical pageq(char *name, Include *ispagehead, choice type) {
  char *c;
  logical ans;

  if (type == ITEM_REFERRER)
    return(TRUE);
  if (type != ITEM_FILE)
    return(FALSE);

  if ((c = strchr(name, '?')) != NULL) {
    *c = '\0';
    ans = (choice)included(name, FALSE, ispagehead);
    *c = '?';
  }
  else
    ans = (choice)included(name, FALSE, ispagehead);
  return(ans);
}
