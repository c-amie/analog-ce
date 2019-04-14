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

/*** input.c; parsing the logfiles */

#include "anlghea3.h"

extern char *block_start, *block_end, *block_bell, *record_start, *pos;
extern logical termchar[];
static logical stdin_used = FALSE;
static char gzmagic[2] = {(char)0x1f, (char)0x8b};
static char zipmagic[4] = {'P', 'K', 0x03, 0x04};
static char bz2magic[3] = {'B', 'Z', 'h'};

/* Open logfile for reading. NB Even if open fails, may need to call
   my_lfclose() to clear up */
logical my_lfopen(Logfile *lf, char *filetype)
{
#ifndef NOPIPES
  extern Strpairlist *uncompresshead;
  char *cmd;
  Strpairlist *up;
#endif
#ifndef LINE_PARSER
  unz_file_info info;
  char magic[5];
  size_t n, len;
  int bze;
#endif
  int i;

  /* Initialise logfile */
  lf->from = LAST_TIME;
  lf->to = FIRST_TIME;
  for (i = 0; i < LOGDATA_NUMBER; i++)
    lf->data[i] = 0;
  lf->bytes = 0;
  lf->bytes7 = 0;

#ifndef LINE_PARSER
  /* Previously opened zip file: open next member.
     (Last member closed in my_lfclose()). */
  if (lf->type == LF_ZIP) {
    if (unzGoToNextFile((unzFile)(lf->file)) == UNZ_OK &&
	unzOpenCurrentFile((unzFile)(lf->file)) == UNZ_OK) {
      duplogfile(lf); /* One logfile for this member and 1 for the whole zip */
      /* Calculate the name of the member, in the form "foo.zip:bar.log" */
      /* Same code below */
      len = strlen(lf->name);
      unzGetCurrentFileInfo((unzFile)(lf->file), &info,
			    NULL, 0, NULL, 0, NULL, 0);
      lf->name = (char *)xmalloc(len + info.size_filename + 2);
      strcpy(lf->name, lf->next->name);  /* lf->next is a clone of lf */
      lf->name[len] = ':';
      unzGetCurrentFileInfo((unzFile)(lf->file), NULL,
			    lf->name + len + 1, info.size_filename + 1,
			    NULL, 0, NULL, 0);
      debug('F', "Opening %s as zipped %s", lf->name, filetype);
      lf->type = LF_ZIPMEMBER;
      return TRUE;
    }
    return FALSE;  /* no more members */
  }
  else
#endif  /* LINE_PARSER */
    if (lf->type != LF_NOTOPENED)
    return FALSE;  /* other previously opened file: shouldn't happen */

  /* Open the file */
  lf->file = NULL;

  if (IS_STDIN(lf->name)) {
    if (stdin_used) {
      warn('F', TRUE, "stdin already used; cannot use it as %s", filetype);
      return FALSE;
    }
    else {
      lf->file = (void *)stdin;
      stdin_used = TRUE;
      debug('F', "Opening stdin as %s", filetype);
      lf->type = LF_NORMAL;
      return TRUE;
    }
  }

  lf->file = (void *)FOPENR(lf->name);
  if (lf->file == NULL) {
    warn('F', TRUE, "Failed to open %s %s: ignoring it", filetype, lf->name);
    return FALSE;
  }

  /* First see if it matches an UNCOMPRESS command: if so pass to a pipe */
#ifndef NOPIPES
  for (up = uncompresshead; up != NULL; TO_NEXT(up)) {
    if (strpbrk(lf->name, "*?") == NULL && MATCHES(lf->name, up->name)) {
      /* * and ? might be a security risk from the form: actually probably
	 not because (i) they shouldn't get this far -- they are interpreted by
	 configlogfile() unless they don't match anything; (ii) they probably
	 wouldn't cause any harm anyway: but let's be over-cautious.
      */
      fclose((FILE *)(lf->file));
      cmd = (char *)xmalloc(strlen(up->data) + strlen(lf->name) + 4);
      if (strchr(lf->name, ' ') != NULL)
	sprintf(cmd, "%s \"%s\"", up->data, lf->name);
      else
	sprintf(cmd, "%s %s", up->data, lf->name);
      lf->file = (void *)POPENR(cmd);
      free((void *)cmd);
      debug('F', "Opening %s as %s", lf->name, filetype);
      debug('F', "  Using %s to uncompress it", up->data);
      lf->type = LF_PIPE;
      return TRUE;
    }
  }
#endif
#ifndef LINE_PARSER   /* Mac version has its own zip/gzip/bzip decompression */
  /* If it didn't match an UNCOMPRESS, read in the first four bytes to see if
     it's a known decompression scheme. */
  n = fread((void *)magic, 1, 4, (FILE *)(lf->file));
  rewind((FILE *)(lf->file));
  if (n >= 3 && strncmp(magic, bz2magic, 3) == 0) {
    lf->file2 = (void *)BZ2_bzReadOpen(&bze, (FILE *)(lf->file),
				       0, 0, NULL, 0);
    if (lf->file2 == NULL) {
      
      warn('F', TRUE, "Failed to open %s %s as bzip2 file: ignoring it",
	   filetype, lf->name);
      return FALSE;
    }
    debug('F', "Opening %s as bzip2'ed %s", lf->name, filetype);
    lf->type = LF_BZ2;
    return TRUE;
  }
  else if (n >= 2 && strncmp(magic, gzmagic, 2) == 0) {
    fclose((FILE *)(lf->file));
    lf->file = (void *)gzopen(lf->name, "rb");
    if (lf->file == NULL) {
      warn('F', TRUE, "Failed to open %s %s as gzip file: ignoring it",
	   filetype, lf->name);
      return FALSE;
    }
    debug('F', "Opening %s as gzipped %s", lf->name, filetype);
    lf->type = LF_GZIP;
    return TRUE;
  }
  else if (n >= 4 && strncmp(magic, zipmagic, 4) == 0) {
    fclose((FILE *)(lf->file));
    lf->file = (void *)unzOpen(lf->name);
    /* Check file was opened, and first member can be opened */
    if (lf->file == NULL || unzGoToFirstFile((unzFile)(lf->file)) != UNZ_OK ||
	unzOpenCurrentFile((unzFile)(lf->file)) != UNZ_OK) {
      warn('F', TRUE, "Failed to open %s %s as zip file: ignoring it",
	   filetype, lf->name);
      return FALSE;
    }
    debug('F', "Opening %s as zip file", lf->name);
    lf->type = LF_ZIP;
    duplogfile(lf); /* One logfile for this member and one for the whole zip */
    /* Calculate the name of the member, in the form "foo.zip:bar.log" */
    /* Same code above */
    len = strlen(lf->name);
    unzGetCurrentFileInfo((unzFile)(lf->file), &info,
			  NULL, 0, NULL, 0, NULL, 0);
    lf->name = (char *)xmalloc(len + info.size_filename + 2);
    strcpy(lf->name, lf->next->name);  /* lf->next is a clone of lf */
    lf->name[len] = ':';
    unzGetCurrentFileInfo((unzFile)(lf->file), NULL,
			  lf->name + len + 1, info.size_filename + 1,
			  NULL, 0, NULL, 0);
    debug('F', "Opening %s as zipped %s", lf->name, filetype);
    lf->type = LF_ZIPMEMBER;
    return TRUE;
  }
#endif  /* LINE_PARSER */ 
  /* The normal case: just an ordinary file */
  debug('F', "Opening %s as %s", lf->name, filetype);
  lf->type = LF_NORMAL;
  return TRUE;
}

FILE *my_fopen(char *name, char *filetype)
{             /* open for reading */
  FILE *f;

  if (IS_STDIN(name)) {
    if (stdin_used) {
      warn('F', TRUE, "stdin already used; cannot use it as %s", filetype);
      f = NULL;
    }
    else {
      f = stdin;
      stdin_used = TRUE;
      debug('F', "Opening stdin as %s", filetype);
    }
  }
  else {
    if ((f = FOPENR(name)) == NULL)
      warn('F', TRUE, "Failed to open %s %s: ignoring it", filetype, name);
    else
      debug('F', "Opening %s as %s", name, filetype);
  }
  return(f);
}

#define LFCLOSE_DATEFMT "%d/%m/%y:%H%n"
void my_lfclose(Logfile *p, Logfile *allc, Logfile *allp,
		char *filetype, Dateman *dman, unsigned long *totdata,
		double *bys, double *bys7) {
  extern FILE *errfile;
  extern char *warn_args;

  unsigned long *data = p->data;
  Logfile *ap, *nextap;
  Inputformatlist *fmt;
  choice count[INPUT_NUMBER];
  logical done;
  int i;
#ifndef LINE_PARSER
  int bze;
#endif

  if (p->type == LF_NOTOPENED)
    return;

#ifndef LINE_PARSER
  /* zip container: don't do diagnostics etc. */
  if (p->type == LF_ZIP) {
    debug('F', "Closing zip file %s", IS_STDIN(p->name)?"stdin":(p->name));
    unzClose((unzFile)(p->file));
    return;
  }
#endif

  debug('F', "Closing %s %s", filetype,	IS_STDIN(p->name)?"stdin":(p->name));
  debug('S', "Successful requests: %lu", data[LOGDATA_SUCC]);
  debug('S', "Redirected requests: %lu", data[LOGDATA_REDIR]);
  debug('S', "Failed requests: %lu", data[LOGDATA_FAIL]);
  debug('S', "Requests returning informational status code: %lu",
	data[LOGDATA_INFO]);
  debug('S', "Status code not given: %lu", data[LOGDATA_UNKNOWN]); 
  if (allp != NULL) {  /* i.e. logfile not cache file */
   debug('S', "Unwanted lines: %lu", data[LOGDATA_UNWANTED]);
    debug('S', "Corrupt lines: %lu", data[LOGDATA_CORRUPT]);
    if (data[LOGDATA_CORRUPT] >
	(data[LOGDATA_SUCC] + data[LOGDATA_REDIR] + data[LOGDATA_FAIL] +
	 data[LOGDATA_INFO] + data[LOGDATA_UNKNOWN] +
	 data[LOGDATA_UNWANTED]) / 10 && data[LOGDATA_CORRUPT] > 10) {
      warn('L', TRUE, "Large number of corrupt lines in %s %s: "
	   "turn debugging on or try different LOGFORMAT", filetype,
	   IS_STDIN(p->name)?"stdin":(p->name));
      if (strchr(warn_args, 'L') != NULL)
	report_logformat(errfile, p->format, TRUE);
    }
  }
  for (i = 0; i < LOGDATA_NUMBER; i++)
    totdata[i] += data[i];
  *bys += p->bytes;
  *bys7 += p->bytes7;
  if (p->from <= p->to) {
    dman->firsttime = MIN(dman->firsttime, p->from);
    dman->lasttime = MAX(dman->lasttime, p->to);
    if (p->tz > 0)
      debug('S', "Times in %s offset by +%d minutes", filetype, p->tz);
    else if (p->tz < 0)
      debug('S', "Times in %s offset by %d minutes", filetype, p->tz);
    debug('S', "Earliest entry in %s: %s", filetype,
	  timesprintf(NULL, LFCLOSE_DATEFMT, p->from, FALSE));
    debug('S', "Latest entry in %s: %s", filetype,
	  timesprintf(NULL, LFCLOSE_DATEFMT, p->to, FALSE));
    if (allp == NULL) {
      p->format->count[ITEM_FILE] = 2;  /* good enough for date check */
      count[ITEM_FILE] = 2;
      p->from += 4;  /* avoid false alerts */
    }
    else {
      for (i = 0; i < INPUT_NUMBER; i++)
	count[i] = 0;
      for (fmt = p->format; fmt != NULL; TO_NEXT(fmt)) {
	for (i = 0; i < INPUT_NUMBER; i++)
	  count[i] = MAX(count[i], fmt->count[i]);
      }
    }
    for (ap = (allc == NULL)?allp:allc; ap != p; ap = nextap) {
      if (ap->from < p->to && p->from < ap->to) {
	for (done = FALSE, fmt = ap->format; fmt != NULL && !done;
	     TO_NEXT(fmt)) {
	  for (i = 0; i < INPUT_NUMBER && !done; i++) {
	    if (fmt->count[i] == 2 && count[i] == 2) {
	      warn('L', TRUE, "%ss %s and %s overlap: "
		   "possible double counting", filetype, ap->name, p->name);
	      done = TRUE;
	    }
	  }
	}
      }
      nextap = ap->next;
      if (nextap == NULL)
	nextap = allp;  /* run through allc then through allp */
    }
  }
  else if (data[LOGDATA_SUCC] + data[LOGDATA_REDIR] + data[LOGDATA_FAIL] +
	   data[LOGDATA_INFO] + data[LOGDATA_UNKNOWN] > 0)
    debug('S', "No times in %s", filetype);

  /* Actually close the file */
  switch (p->type) {
  case LF_NORMAL:
    fclose((FILE *)(p->file));
    break;
#ifndef LINE_PARSER
  case LF_GZIP:
    gzclose((gzFile)(p->file));
    break;
  case LF_ZIPMEMBER:
    unzCloseCurrentFile((unzFile)(p->file));
    break;
  case LF_BZ2:
    BZ2_bzReadClose(&bze, (BZFILE *)(p->file2));
    fclose((FILE *)(p->file));
    break;
#endif
#ifndef NOPIPES
  case LF_PIPE:
    if (feof((FILE *)(p->file))) {
      if (pclose((FILE *)(p->file)) != 0)
	warn('F', TRUE, "Problems uncompressing %s %s", filetype, p->name);
    }
    else
      pclose((FILE *)(p->file)); /* not reached EOF: pclose will return
				    broken pipe */
    break;
#endif
  }
}

int my_fclose(FILE *f, char *name, char *filetype)
{
  debug('F', "Closing %s %s", filetype, IS_STDIN(name)?"stdin":name);
  return(fclose(f));
}

logical logfile_init(Logfile *lf) {
  extern unsigned int year, code;
  extern double bytes;

  if (lf->format == NULL) {
    debug('F', "Ignoring logfile %s, which contains no items being analysed",
	  lf->name);
    return FALSE;
  }
  pos = NULL;
  year = 0;
  bytes = 0;
  code = 0;
  return(my_lfopen(lf, "logfile"));
}

/* Duplicate a logfile in the logfile list. Used to expand a zip file into many
   individual files. NB copies data member pointers not contents. */
void duplogfile(Logfile *lf) {
  Logfile *nextlf = lf->next;
  Logfile *newlf = (Logfile *)xmalloc(sizeof(Logfile));

  memcpy(newlf, lf, sizeof(Logfile));
  lf->next = newlf;
  newlf->next = nextlf;
}

choice getmoredata(Logfile *lf, char *start, size_t length) {
#ifndef LINE_PARSER
  int bze;
#endif
  int n;

  switch (lf->type) {
  case LF_NORMAL:
  case LF_PIPE:
    n = fread((void *)start, 1, length, (FILE *)(lf->file));
    break;
#ifndef LINE_PARSER
  case LF_GZIP:
    n = gzread((gzFile)(lf->file), (void *)start, length);
    if (n < 0)
      n = 0;  /* gzread() returns -1 on error */
    break;
  case LF_ZIPMEMBER:
    n = unzReadCurrentFile((unzFile)(lf->file), (void *)start, length);
    if (n < 0)
      n = 0;  /* unzReadCurrentFile() returns -1 on error */
    break;
  case LF_BZ2:
    n = BZ2_bzRead(&bze, (BZFILE *)(lf->file2), (void *)start, length);
    if (bze != BZ_OK && bze != BZ_STREAM_END)
      n = 0;  /* return value from BZ2_bzRead() undefined on error */
    break;
#endif
  default:
    n = 0;  /* shouldn't happen, but if it does return EOF */
  }
  block_end = start + n;
  block_bell = block_end - BLOCK_EPSILON;  /* saves repeating this calc. */

  /* need to set block_end and block_bell even if returning EOF because still
     have to parse the rest of the last line of the file */
  if (block_end == start)
    return(EOF);
  return(OK);
}

choice getnextline(FILE *f, char *start, size_t used) {
  /* not very efficient: only for use during initialisation */
  char *s = start;

  if ((*s = (char)getc(f)) == (char)EOF && feof(f))  /* in case char == 255 */
    return(EOF);
  while (*s == '\r' || *s == '\n')
    *s = (char)getc(f);    /* run past any new line */
  if (*s == (char)EOF && feof(f))
    return(EOF);
  for (*(++s) = (char)getc(f); *s != '\r' && *s != '\n' && !feof(f) &&
	 s < start + BLOCKSIZE - used - 1; *(++s) = (char)getc(f))
    ;  /* read in to next new line */
  *s = '\0';
  if (s == start + BLOCKSIZE - 1)
    return(FALSE);
  return(TRUE);
}

void shunt_data(Logfile *lf) {
  memmove((void *)block_start, (void *)record_start,
	  (size_t)(block_end - record_start));
  (void)getmoredata(lf, block_start + (block_end - record_start),
		    (size_t)(BLOCKSIZE - (block_end - record_start)));
  pos -= record_start - block_start;
  record_start = block_start;
}

int strtomonth(char *m)  /* convert 3 letter month abbrev. to int */
{
  int monthno = ERR;

  switch (m[0]) {
  case 'A':
  case 'a':
    if ((m[1] == 'p' || m[1] == 'P') && (m[2] == 'r' || m[2] == 'R'))
      monthno = 3;
    else if ((m[1] == 'u' || m[1] == 'U') && (m[2] == 'g' || m[2] == 'G'))
      monthno = 7;
    break;
  case 'D':
  case 'd':
    if ((m[1] == 'e' || m[1] == 'E') && (m[2] == 'c' || m[2] == 'C'))
      monthno = 11;
    break;
  case 'F':
  case 'f':
    if ((m[1] == 'e' || m[1] == 'E') && (m[2] == 'b' || m[2] == 'B'))
      monthno = 1;
    break;
  case 'J':
  case 'j':
    if ((m[1] == 'a' || m[1] == 'A') && (m[2] == 'n' || m[2] == 'N'))
      monthno = 0;
    if ((m[1] == 'u' || m[1] == 'U')) {
      if ((m[2] == 'l' || m[2] == 'L'))
	monthno = 6;
      else if ((m[2] == 'n' || m[2] == 'N'))
	monthno = 5;
    }
    break;
  case 'M':
  case 'm':
    if ((m[1] == 'a' || m[1] == 'A')) {
      if ((m[2] == 'r' || m[2] == 'R'))
	monthno = 2;
      else if ((m[2] == 'y' || m[2] == 'Y'))
	monthno = 4;
    }
    break;
  case 'N':
  case 'n':
    if ((m[1] == 'o' || m[1] == 'O') && (m[2] == 'v' || m[2] == 'V'))
      monthno = 10;
    break;
  case 'O':
  case 'o':
    if ((m[1] == 'c' || m[1] == 'C') && (m[2] == 't' || m[2] == 'T'))
      monthno = 9;
    break;
  case 'S':
  case 's':
    if ((m[1] == 'e' || m[1] == 'E') && (m[2] == 'p' || m[2] == 'P'))
      monthno = 8;
    break;
  }
  return(monthno);
}

choice parsemonth(Logfile *lf, void *arg, char c) {
  unsigned int *m = (unsigned int *)arg;

  *block_end = '\0';
  *m = (unsigned int)strtomonth(pos);
  if (*m == (unsigned int)ERR)
    return(FALSE);
  else {
    pos += 3;
    return(TRUE);
  }
}

choice parseuint2(Logfile *lf, void *arg, char c) {
  unsigned int *x = (unsigned int *)arg;

  *block_end = '\0';
  if (!ISDIGIT(*pos)) {
    if (*pos == ' ')
      *x = 0;
    else
      return(FALSE);
  }
  else
    *x = *pos - '0';
  pos++;

  if (!ISDIGIT(*pos))
    return(TRUE);
  *x *= 10;
  *x += *pos - '0';
  pos++;
  return(TRUE);
}

choice parsenmonth(Logfile *lf, void *arg, char c) { /* nearly same as uint2 */
  unsigned int *x = (unsigned int *)arg;

  *block_end = '\0';
  if (!ISDIGIT(*pos))
    return(FALSE);
  *x = *pos - '0';
  pos++;

  if (!ISDIGIT(*pos)) {
    if (*x == 0) {
      pos--;
      return(FALSE);
    }
    (*x)--;   /* to convert to internal month representation */
    return(TRUE);
  }
  *x *= 10;
  *x += *pos - '0';
  if (*x == 0)
    return(FALSE);
  pos++;
  (*x)--;
  return(TRUE);
}

choice parseulong(Logfile *lf, void *arg, char c) {
  unsigned long *x = (unsigned long *)arg;
  register char *p = pos;

  *block_end = '\0';
  if (!ISDIGIT(*p))
    return(FALSE);

  *x = 0;
  while (ISDIGIT(*p)) {
    if (*x >= 429496729)   /* (usual) max for unsigned long, divided by 10 */
      return(FALSE);  /* best to keep pos at beginning of number this time */
    *x *= 10;
    *x += (*p - '0');
    p++;
  }

  pos = p;
  return(TRUE);
}

choice parseunixtime(Logfile *lf, void *arg, char c) {
  unsigned long *x = (unsigned long *)arg;
  register char *p = pos;

  *block_end = '\0';
  if (!ISDIGIT(*p))
    return(FALSE);

  *x = 0;
  while (ISDIGIT(*p)) {
    if (*x >= 214748364)   /* max for Unix time, divided by 10 */
      return(FALSE);  /* best to keep pos at beginning of number this time */
    *x *= 10;
    *x += (*p - '0');
    p++;
  }

  pos = p;
  return(TRUE);
}

/* Parse processing time in microseconds, convert to milliseconds */
choice parseproctimeu(Logfile *lf, void *arg, char c) {
  unsigned long *x;
  choice rc;

  rc = parseulong(lf, arg, c);
  if (rc == FALSE)
    return(FALSE);

  x = (unsigned long *)arg;
  *x /= 1000;

  return(rc);
}

choice parseproctime(Logfile *lf, void *arg, char c) {
  /* parse processing time in decimal seconds, converting to milliseconds */
  unsigned long *x = (unsigned long *)arg;
  register char *p = pos;

  *block_end = '\0';
  if (!ISDIGIT(*p))  /* NB Decimals must begin "0." not just "." */
    return(FALSE);

  if (ISDIGIT(*(p + 3)) && ISDIGIT(*(p + 2)) && ISDIGIT(*(p + 1))) {
    *x = 1000000;   /* signifying at least 1000 seconds */
    while (ISDIGIT(*p))
      p++;
    if (*p == '.') {
      p++;
      while (ISDIGIT(*p))
	p++;
    }
    pos = p;
    return(TRUE);
  }

  for (*x = 0; ISDIGIT(*p); p++) {
    *x *= 10;
    *x += (*p - '0');
  }
  *x *= 1000;

  if (*p == '.') {    /* decimal follows; only read first 3 places */
    p++;
    if (ISDIGIT(*p)) {
      *x += 100 * (*(p++) - '0');
      if (ISDIGIT(*p)) {
	*x += 10 * (*(p++) - '0');
	if (ISDIGIT(*p)) {
	  *x += (*(p++) - '0');
	  while (ISDIGIT(*p))
	    p++;
	}
      }
    }
  }

  pos = p;
  return(TRUE);
}

choice parseuint2strict(Logfile *lf, void *arg, char c) {
  unsigned int *x = (unsigned int *)arg;

  *block_end = '\0';
  if (!ISDIGIT(*pos))
    return(FALSE);
  *x = 10 * (*pos - '0');
  pos++;

  if (!ISDIGIT(*pos))
    return(FALSE);
  *x += *pos - '0';
  pos++;
  return(TRUE);
}

choice parsescode(Logfile *lf, void *arg, char c) {
  /* parseuint3strict() with twiddles */
  unsigned int *x = (unsigned int *)arg;

  *block_end = '\0';
  if (*pos < '1' || *pos > '5') {  /* only accept codes 100 - 599, or - */
    if (*pos == '-') {      /* see also MIN_SC, SC_NUMBER in anlghea3.h */
      pos++;
      *x = 200;
      return(TRUE);
    }
    else
      return(FALSE);
  }
  *x = 100 * (*pos - '0');
  pos++;

  if (!ISDIGIT(*pos))
    return(FALSE);
  *x += 10 * (*pos - '0');
  pos++;

  if (!ISDIGIT(*pos))
    return(FALSE);
  *x += *pos - '0';
  pos++;
  return(TRUE);
}

choice parseuint4strict(Logfile *lf, void *arg, char c) {
  unsigned int *x = (unsigned int *)arg;

  *block_end = '\0';
  if (!ISDIGIT(*pos))
    return(FALSE);
  *x = 1000 * (*pos - '0');
  pos++;

  if (!ISDIGIT(*pos))
    return(FALSE);
  *x += 100 * (*pos - '0');
  pos++;

  if (!ISDIGIT(*pos))
    return(FALSE);
  *x += 10 * (*pos - '0');
  pos++;

  if (!ISDIGIT(*pos))
    return(FALSE);
  *x += *pos - '0';
  pos++;
  return(TRUE);
}

choice parseudint(Logfile *lf, void *arg, char c) {
  double *x = (double *)arg;

  *block_end = '\0';
  if (*pos == '-') {
    *x = 0.0;  /* because used for bytes (only) */
    pos++;
    return(TRUE);
  }
  if (!ISDIGIT(*pos))
    return(FALSE);
  *x = *pos - '0';
  pos++;

  while (ISDIGIT(*pos)) {
    *x *= 10;
    *x += *pos - '0';
    pos++;
  }
  return(TRUE);
}

choice parseyear2(Logfile *lf, void *arg, char c) {
  unsigned int *y = (unsigned int *)arg;
  logical rc;

  rc = parseuint2strict(lf, arg, c);
  if (*y >= 70)
    *y += 1900;
  else
    *y += 2000;
  return(rc);
}

choice parseyear2or4(Logfile *lf, void *arg, char c) {
  if (*pos == '\0' || (*(pos + 1) == '\0'))
    return(FALSE);
  else if (ISDIGIT(*(pos + 2)))
    return(parseuint4strict(lf, arg, c));
  else
    return(parseyear2(lf, arg, c));
}

choice parseam(Logfile *lf, void *arg, char c) {
  char *d = (char *)arg;

  *block_end = '\0';
  *d = TOLOWER(*pos);
  if (*d == 'a' || *d == 'p') {
    pos++;
    return(TRUE);
  }
  else
    return(FALSE);
}

choice parsecode(Logfile *lf, void *arg, char c) {
  unsigned int *x = (unsigned int *)arg;
  char *d = pos, e, *g;

  *block_end = '\0';
  if (parsejunk(lf, NULL, c) == FALSE || pos == d)
    return(FALSE);
  g = (c == WHITESPACE)?pos:(pos - 1);
  /* parsejunk normally leaves pos AFTER delimiter, but ON delimiter if
     delimiter == WHITESPACE */
  e = *g;
  *g = '\0';
  *x = IGNORE_CODE;
  if (d[0] == 'O' && d[1] == 'K')
    *x = 200;
  else if (STREQ(d, "ERR!"))
    *x = 404;
  else if (STREQ(d, "PRIV"))
    *x = 401;
  else if (STREQ(d, "not modified"))
    *x = 304;
  else if (*(d++) == 'g' && *(d++) == 'e' && *(d++) == 't' && *(d++) == ' ' &&
	   *(d++) == 'f') {
    if (STREQ(d, "ile"))
      *x = 200;
    else if (STREQ(d, "ailed"))
      *x = 499;
  }
  *g = e;
  return(TRUE);
}

choice parsejunk(Logfile *lf, void *arg, char c) { /* NB allow empty strings */
  register char *p = pos;

  if (c == WHITESPACE) {
    termchar[(unsigned char)' '] = TRUE;
    termchar[(unsigned char)'\t'] = TRUE;
    *block_end = ' ';
  }
  else {
    termchar[(unsigned char)c] = TRUE;
    *block_end = c;
  }
  while (!termchar[(unsigned char)(*p)])
    p++;

  if (p == block_end) {
    if (record_start == block_start) {
      if (c == WHITESPACE) {
	termchar[(unsigned char)' '] = FALSE;
	termchar[(unsigned char)'\t'] = FALSE;
      }
      else if (c != '\r' && c != '\n' && c != '\0')
	termchar[(unsigned char)c] = FALSE;
      pos = p;
      return(FALSE);
    }
    pos = p;
    shunt_data(lf);
    p = pos;
    *block_end = (c == WHITESPACE)?' ':c;
    while (!termchar[(unsigned char)(*p)])
      p++;
    if (p == block_end) {
      if (c == WHITESPACE) {
	termchar[(unsigned char)' '] = FALSE;
	termchar[(unsigned char)'\t'] = FALSE;
      }
      else if (c != '\r' && c != '\n' && c != '\0')
	termchar[(unsigned char)c] = FALSE;
      pos = p;
      return(FALSE);
    }
  }
  if (c == WHITESPACE) {
    termchar[(unsigned char)' '] = FALSE;
    termchar[(unsigned char)'\t'] = FALSE;
  }
  else if (c != '\r' && c != '\n' && c != '\0')
    termchar[(unsigned char)c] = FALSE;
  if (*p != c && !(c == '\n' && *p == '\r') &&
      !(c == WHITESPACE && (*p == ' ' || *p == '\t'))) {
    pos = p;
    return(FALSE);
  }
  if (c == '\n' || c == '\r') {
    while ((*p == '\n' || *p == '\r') && p < block_end)
      p++;
  }
  else if (c != WHITESPACE)
    p++;
  /* Leave pos AFTER delimiter, except ON delimiter if delim == WHITESPACE */
  pos = p;
  return(TRUE);
}

choice parsespace(Logfile *lf, void *arg, char c) {

  *block_end = '\0';
  if (pos == block_end && record_start != block_start)
    shunt_data(lf);
  if (*pos != '\t' && *pos != ' ')
    return(FALSE);
  while (*pos == '\t' || *pos == ' ')
    pos++;
  if (pos == block_end && record_start != block_start) {
    shunt_data(lf);
    while (*pos == '\t' || *pos == ' ')
      pos++;
  }
  return(TRUE);
}

choice parseoptspace(Logfile *lf, void *arg, char c) {

  *block_end = '\0';
  if (pos == block_end && record_start != block_start)
    shunt_data(lf);
  while (*pos == '\t' || *pos == ' ')
    pos++;
  if (pos == block_end && record_start != block_start) {
    shunt_data(lf);
    while (*pos == '\t' || *pos == ' ')
      pos++;
  }
  return(TRUE);
}

choice parsenewline(Logfile *lf, void *arg, char c) {
  /* allow new line to be preceded by white space */
  *block_end = 'a';
  if (*pos != '\n' && *pos != ' ' && *pos != '\r' && *pos != '\t')
    return(FALSE);
  while (*pos == ' ' || *pos == '\t')
    pos++;
  if (pos == block_end && record_start != block_start) {
    shunt_data(lf);
    *block_end = 'a';
    while (*pos == ' ' || *pos == '\t')
      pos++;
  }

  if (*pos != '\n' && *pos != '\r')
    return(FALSE);
  termchar[(unsigned char)'\0'] = FALSE;
  while (termchar[(unsigned char)(*pos)])
    pos++;
  if (pos == block_end && record_start != block_start) {
    shunt_data(lf);
    *block_end = 'a';
    while (termchar[(unsigned char)(*pos)])
      pos++;
  }
  termchar[(unsigned char)'\0'] = TRUE;
  return(TRUE);
}

void parsenonnewline(Logfile *lf) {

  *block_end = '\n';
  termchar[(unsigned char)'\0'] = FALSE;
  while (!termchar[(unsigned char)(*pos)])
    pos++;

  if (pos == block_end) {
    if (record_start == block_start) {
      termchar[(unsigned char)'\0'] = TRUE;
      return;
    }
    shunt_data(lf);
    *block_end = '\n';
    while (!termchar[(unsigned char)(*pos)])
      pos++;
  }
  termchar[(unsigned char)'\0'] = TRUE;
}

choice parsestring(Logfile *lf, void *arg, char c) { /*NB allow empty strings*/
  register char *p = pos;
  Memman *m = (Memman *)arg;
  size_t length = 0;

  char *string_start;
  string_start = p;

  if (c == WHITESPACE) {
    termchar[(unsigned char)' '] = TRUE;
    termchar[(unsigned char)'\t'] = TRUE;
    *block_end = ' ';
  }
  else {
    termchar[(unsigned char)c] = TRUE;
    *block_end = c;
  }
  while (!termchar[(unsigned char)(*p)]) {
    p++;
    length++;
  }

  if (p == block_end) {
    if (record_start == block_start) {
      if (c == WHITESPACE) {
	termchar[(unsigned char)' '] = FALSE;
	termchar[(unsigned char)'\t'] = FALSE;
      }
      else if (c != '\r' && c != '\n' && c != '\0')
	termchar[(unsigned char)c] = FALSE;
      pos = p;
      return(FALSE);
    }
    string_start -= record_start - block_start;
    pos = p;
    shunt_data(lf);
    p = pos;
    *block_end = (c == WHITESPACE)?' ':c;
    while (!termchar[(unsigned char)(*p)]) {
      p++;
      length++;
    }
    if (p == block_end) {
      if (c == WHITESPACE) {
	termchar[(unsigned char)' '] = FALSE;
	termchar[(unsigned char)'\t'] = FALSE;
      }
      else if (c != '\r' && c != '\n' && c != '\0')
	termchar[(unsigned char)c] = FALSE;
      pos = p;
      return(FALSE);
    }
  }

  if (c == WHITESPACE) {
    termchar[(unsigned char)' '] = FALSE;
    termchar[(unsigned char)'\t'] = FALSE;
  }
  else if (c != '\r' && c != '\n' && c != '\0')
    termchar[(unsigned char)c] = FALSE;
  if (*p != c && !(c == '\n' && *p == '\r') &&
      !(c == WHITESPACE && (*p == ' ' || *p == '\t'))) {
    pos = p;
    return(FALSE);
  }

  memcpy(submalloc(m, length + 1), (void *)string_start, length);
  *((char *)(m->next_pos) - 1) = '\0'; /* = curr_pos + length */
  if (c == '\n' || c == '\r') {
    while ((*p == '\n' || *p == '\r') && p < block_end)
      p++;
  }
  else if (c != WHITESPACE)
    p++;
  pos = p;
  return(TRUE);
}

choice parsemsbrow(Logfile *lf, void *arg, char c) {
  Memman *m = (Memman *)arg;
  char *d;

  if (parsestring(lf, arg, c) == FALSE)
    return(FALSE);
  for (d = m->curr_pos; *d != '\0'; d++) {
    if (*d == '+')
      *d = ' ';
  }
  return(TRUE);
}

choice parseref(Logfile *lf, void *arg, char c) {
  /* For referrer in old referrer logs: as parsestring except also checks ->
     immediately after delimiting character (presumably space: may not be \n
     or \r: also WHITESPACE wouldn't work so is disallowed in strtoinfmt()) */
  register char *p = pos;
  Memman *m = (Memman *)arg;
  size_t length = 0;

  char *string_start;
  string_start = p;

  *block_end = '\r';
  termchar[(unsigned char)c] = TRUE;
  /* p + 1 or p + 2 could be past block_end, but this doesn't matter because
     they will never be read; *block_end == '\r' and there is no way to set
     c == '\r'. */
  while (!termchar[(unsigned char)(*p)] ||
	 (*p == c && (*(p + 1) != '-' || *(p + 2) != '>'))) {
    p++;
    length++;
  }

  if (p == block_end) {
    if (record_start == block_start) {
      if (c != '\r' && c != '\n' && c != '\0')
	termchar[(unsigned char)c] = FALSE;
      pos = p;
      return(FALSE);
    }
    string_start -= record_start - block_start;
    pos = p;
    shunt_data(lf);
    p = pos;
    *block_end = '\r';
    while (!termchar[(unsigned char)(*p)] ||
	   (*p == c && (*(p + 1) != '-' || *(p + 2) != '>'))) {
      p++;
      length++;
    }
    if (p == block_end) {
      if (c != '\r' && c != '\n' && c != '\0')
	termchar[(unsigned char)c] = FALSE;
      pos = p;
      return(FALSE);
    }
  }

  if (c != '\r' && c != '\n' && c != '\0')
    termchar[(unsigned char)c] = FALSE;
  if (*p != c) {
    pos = p;
    return(FALSE);
  }

  memcpy(submalloc(m, length + 1), (void *)string_start, length);
  *((char *)(m->next_pos) - 1) = '\0';
  p++;

  pos = p;
  return(TRUE);
}

choice checkchar(Logfile *lf, void *arg, char c) {
  if (*pos == c) {
    pos++;
    return(TRUE);
  }
  else
    return(FALSE);
}

void resolveconflicts(unsigned int n, char *delims, ...) {
  /* Used in parselogfmt(). Munges a format header line, which is in pos, so
     that there are no "repeated" fields. n is the number of alternatives. */
  va_list ap;
  char *arg, *s, *t;
  logical found;
  unsigned int i;

  va_start(ap, delims);
  for (i = 0, found = FALSE; i < n; i++) {
    arg = va_arg(ap, char *);
    for (s = strstr(pos, arg); s != NULL; s = strstr(s + 1, arg)) {
      t = s + strlen(arg);
      if ((s == pos || strchr(delims, *(s - 1)) != NULL) &&
	  (*t == '\0' || strchr(delims, *t) != NULL)) {  /* true match */
	if (found) { /* found an earlier occurrence */
	  *s = 'X';
	  *(s + 1) = 'X';
	}
	else  /* no earlier occurrence: leave this one alone */
	  found = TRUE;
      }
    }
  }
  va_end(ap);
}

choice parselogfmt(Logfile *lf, void *arg, char c) {
  extern Inputformatlist *logformat;
  static char *fmt = NULL;
  static size_t fmtlen = 0;
  char *d, *e;
  unsigned int x, y;

  logformat->used = TRUE;  /* so as to start afresh */
  *block_end = '\n';
  d = strpbrk(pos, "\r\n");
  if (d == block_end) {
    shunt_data(lf);
    *block_end = '\n';
    d = strpbrk(pos, "\r\n");
    if (d == block_end)
      return(FALSE);
  }
  if (d == NULL)
    return(BADLOGFMT);
  else
    *d = '\0';
  switch (c) {
  case '0':  /* DEFAULT format. These are caught and translated earlier: this
		is just in case we have specified DEFAULTLOGFORMAT DEFAULT */
  case '1':  /* AUTO format */
    if (*pos == '!' && *(pos + 1) == '!') {
      debug('F', "  Detect that it's in WebSTAR format");
      configlogfmt((void *)&logformat, NULL, "WEBSTAR", NULL, -1);
    }
    else if (chrn(pos, ',') == 15) {
      e = strchr(strchr(pos, ',') + 1, ',') + 2;
      x = (*(e++) - '0');
      if (ISDIGIT(*e))
	x = x * 10 + (*(e++) - '0');
      if (*(e++) != '/') {
	warn('F', TRUE, "Logfile seems to be in Microsoft format "
	     "but I can't parse the date: use LOGFORMAT command");
	return(BADLOGFMT);
      }
      y = (*(e++) - '0');
      if (ISDIGIT(*e))
	y = y * 10 + (*(e++) - '0');
      if (*e != '/') {
	warn('F', TRUE, "Logfile seems to be in Microsoft format "
	     "but I can't parse the date: use LOGFORMAT command");
	return(BADLOGFMT);
      }
      if (x <= 12 && y > 12) {
	debug('F',
	      "  Detect that it's in Microsoft format (North American dates)");
	configlogfmt((void *)&logformat, NULL, "MICROSOFT-NA", NULL, -1);
      }
      else if (x > 12 && y <= 12) {
	debug('F',
	      "  Detect that it's in Microsoft format (international dates)");
	configlogfmt((void *)&logformat, NULL, "MICROSOFT-INT", NULL, -1);
      }
      else if (x > 12 && y > 12) {
	warn('F', TRUE, "Logfile seems to be in Microsoft format "
	     "but I can't parse the date: use LOGFORMAT command");
	return(BADLOGFMT);
      }
      else {
	warn('F', TRUE, "Microsoft logfile with ambiguous dates: "
	     "use LOGFORMAT MICROSOFT-NA or LOGFORMAT MICROSOFT-INT");
	*d = '\n';
	return(BADLOGFMT);
      }
    }
    else if (*pos == 'f' && *(pos + 1) == 'o' && *(pos + 2) == 'r' &&
	     *(pos + 3) == 'm' && *(pos + 4) == 'a' && *(pos + 5) == 't' &&
	     *(pos + 6) == '=') {
      debug('F', "  Detect that it's in Netscape format");
      configlogfmt((void *)&logformat, NULL, "NETSCAPE", NULL, -1);
    }
    else if (ISDIGIT(*pos) && ISDIGIT(*(pos + 3)) && *(pos + 5) == '/' &&
	     *(pos + 14) == ':' && ISDIGIT(*(pos + 16))) {
      if (chrn(pos, '\t') == 12) {
	x = (*pos - '0') * 10 + (*(pos + 1) - '0');
	y = (*(pos + 3) - '0') * 10 + (*(pos + 4) - '0');
	if (x <= 12 && y > 12) {
	  debug('F',
		"  Detect that it's in WebSite format (North American dates)");
	  configlogfmt((void *)&logformat, NULL, "WEBSITE-NA", NULL, -1);
	}
	else if (x > 12 && y <= 12) {
	  debug('F',
		"  Detect that it's in WebSite format (international dates)");
	  configlogfmt((void *)&logformat, NULL, "WEBSITE-INT", NULL, -1);
	}
	else {
	  warn('F', TRUE, "WebSite logfile with ambiguous dates: "
	       "use LOGFORMAT WEBSITE-NA or LOGFORMAT WEBSITE-INT");
	  *d = '\n';
	  return(BADLOGFMT);
	}
      }
      else if (chrn(pos, '\t') == 5) {
	debug('F', "  Detect that it's in MacHTTP format");
	configlogfmt((void *)&logformat, NULL, "MACHTTP", NULL, -1);
      }
      else {
	*d = '\n';
	return(BADLOGFMT);
      }
    }
    else if (strstr(pos, " -> ") != NULL) {
      debug('F', "  Detect that it's in referrer log format");
      configlogfmt((void *)&logformat, NULL, "REFERRER", NULL, -1);
    }
    else if (*pos == '[' && (*(pos + 21) == ']' || *(pos + 27) == ']')) {
      debug('F', "  Detect that it's in browser log format");
      configlogfmt((void *)&logformat, NULL, "BROWSER", NULL, -1);
    }
    else if (*pos == '#') {
      /* look at first the two lines to determine vendor, although we expect
	 Microsoft string on first line, and WebSTAR on second. */
      for (e = d + 1; *e == '\n' || *e == '\r'; e++)
	;
      if (headmatch(pos + 1,
		    "Software: Microsoft Internet Information Serv") ||
	  headmatch(e, "#Software: Microsoft Internet Information Serv")) {
	debug('F',
	      "  Detect that it's in Microsoft's version of extended format");
	configlogfmt((void *)&logformat, NULL, "MS-EXTENDED", NULL, -1);
      }
      else if (headmatch(e, "#Software: WebSTAR") ||
	       headmatch(pos + 1, "Software: WebSTAR")) {
	debug('F', "  Detect that it's in extended format (WebSTAR server)");
	configlogfmt((void *)&logformat, NULL, "WEBSTAR-EXTENDED", NULL, -1);
      }
      else {
	debug('F', "  Detect that it's in W3 extended format");
	configlogfmt((void *)&logformat, NULL, "EXTENDED", NULL, -1);
      }
    }
    else if ((e = strchr(pos + 6, '[')) != NULL && *(e + 27) == ']' &&
	     strchr(pos, '"') == e + 29) {
      x = chrn(e + 23, '"');
      if (x == 2) {
	debug('F', "  Detect that it's in common log format");
	configlogfmt((void *)&logformat, NULL, "COMMON", NULL, -1);
      }
      else if (x == 3) {
	debug('F',
	      "  Detect that it's in Microsoft's broken common log format");
	configlogfmt((void *)&logformat, NULL, "MS-COMMON", NULL, -1);
      }
      else if (x == 6) {
	debug('F', "  Detect that it's in NCSA combined format");
	configlogfmt((void *)&logformat, NULL, "COMBINED", NULL, -1);
      }
      else {
	*d = '\n';
	return(BADLOGFMT);
      }
    }
    else {
      *d = '\n';
      return(BADLOGFMT);
    }
    *d = '\n';
    break;
  case '2':  /* WebSTAR format */
    /* pos is already start of format proper */
    strtoupper(pos);
    resolveconflicts(5, " \t", "SC-STATUS", "RESULT_CODE", "STATUS",
		     "CS-STATUS",  "RESULT");
    resolveconflicts(3, " \t", "CS-URI-STEM", "CS-URI", "URL");
    resolveconflicts(2, " \t", "BYTES", "BYTES_SENT");
    resolveconflicts(4, " \t", "HOSTNAME", "C-DNS", "CS-HOST", "CS-DNS");
    resolveconflicts(2, " \t", "C-IP", "CS-IP");
    resolveconflicts(2, " \t", "REFERER", "CS(REFERER)");
    resolveconflicts(2, " \t", "AGENT", "CS(USER-AGENT)");
    resolveconflicts(4, " \t", "HOSTFIELD", "CS(HOST)", "HOST", "CS-SIP");
    resolveconflicts(2, " \t", "SEARCH_ARGS", "CS-URI-QUERY");
    ENSURE_LEN(fmt, fmtlen, 32);   /* Arbitrary amount to get started */
    fmt[0] = '\0';
    pos = strtok(pos, " \t");
    while (pos != NULL) {
      ENSURE_LEN(fmt, fmtlen, strlen(fmt) + 12);
      /* Enough to add \t + any next string. Not v efficient, but that's OK. */
      if (!IS_EMPTY_STRING(fmt))
	strcat(fmt, "\t");
      if (STREQ(pos, "COMMON_LOG_FORMAT"))
	strcpy(fmt, "COMMON");
      else if (STREQ(pos, "DATE"))
	strcat(fmt, "%m/%d/%Z");
      else if (STREQ(pos, "TIME"))
	strcat(fmt, "%W%h:%n:%j");
      else if (STREQ(pos, "RESULT"))
	strcat(fmt, "%C");
      else if (STREQ(pos, "SC-STATUS") || STREQ(pos, "CS-STATUS") ||
	       STREQ(pos, "RESULT_CODE") || STREQ(pos, "STATUS"))
	strcat(fmt, "%c");
      else if (STREQ(pos, "URL") || STREQ(pos, "CS-URI-STEM") ||
	       STREQ(pos, "CS-URI"))
	strcat(fmt, "%r");
      else if (STREQ(pos, "BYTES") || STREQ(pos, "BYTES_SENT"))
	strcat(fmt, "%b");
      else if (STREQ(pos, "HOSTNAME") || STREQ(pos, "CS-HOST") ||
	       STREQ(pos, "C-DNS") || STREQ(pos, "CS-DNS"))
	strcat(fmt, "%S");
      else if (STREQ(pos, "CS-IP") || STREQ(pos, "C-IP"))
	strcat(fmt, "%s");
      else if (STREQ(pos, "REFERER"))
	strcat(fmt, "%f");
      else if (STREQ(pos, "CS(REFERER)"))
	strcat(fmt, "\"%f\"");
      else if (STREQ(pos, "AGENT"))
	strcat(fmt, "%B");
      else if (STREQ(pos, "CS(USER-AGENT)"))
	strcat(fmt, "\"%B\"");
      else if (STREQ(pos, "CS(HOST)") || STREQ(pos, "HOSTFIELD"))
	strcat(fmt, "\"%v\"");
      else if (STREQ(pos, "HOST") || STREQ(pos, "CS-SIP"))
	strcat(fmt, "%v");
      else if (STREQ(pos, "SEARCH_ARGS") || STREQ(pos, "CS-URI-QUERY"))
	strcat(fmt, "%q");
      else if (STREQ(pos, "USER"))
	strcat(fmt, "%u");
      else if (STREQ(pos, "TRANSFER_TIME"))
	strcat(fmt, "%t");
      else
	strcat(fmt, "%j");
      pos = strtok((char *)NULL, " \t");
    }
    configlogfmt((void *)&logformat, NULL, "WEBSTAR", NULL, -1);
    configlogfmt((void *)&logformat, NULL, fmt, NULL, -3);
    pos = d + 1;  /* start at next line */
    (void)parsenewline(lf, NULL, '\0');
    break;
  case '3':  /* W3 extended format */
  case '5':  /* Microsoft's attempt at same */
  case '6':  /* WebSTAR's (much closer :-) attempt at same */
    strtoupper(pos);
    resolveconflicts(2, " \t", "BYTES", "SC-BYTES");
    resolveconflicts(2, " \t", "C-IP", "CS-IP");
    if (c == '6') {  /* WebSTAR */
      resolveconflicts(3, " \t", "C-DNS", "CS-DNS", "CS-HOST");
      resolveconflicts(6, " \t", "CS(HOST)", "S-DNS", "S-IP", "CS-SIP",
		       "S-SITENAME", "S-COMPUTERNAME");
    }
    else {
      resolveconflicts(2, " \t", "C-DNS", "CS-DNS");
      resolveconflicts(7, " \t", "CS(HOST)", "CS-HOST", "S-DNS", "S-IP",
		       "S-SITENAME", "S-COMPUTERNAME", "CS-SIP");
    }
    resolveconflicts(2, " \t", "CS-URI-STEM", "CS-URI");
    resolveconflicts(3, " \t", "CS-USERNAME", "CS(FROM)", "CS(COOKIE)");
    ENSURE_LEN(fmt, fmtlen, 32);   /* See above */
    fmt[0] = '\0';
    pos = strtok(pos, " \t");
    while (pos != NULL) {
      ENSURE_LEN(fmt, fmtlen, strlen(fmt) + 11);  /* See above */
      if (!IS_EMPTY_STRING(fmt))
	strcat(fmt, "%w");
      if (STREQ(pos, "DATE"))
	strcat(fmt, "%Y-%m-%d");
      else if (STREQ(pos, "TIME"))
	strcat(fmt, "%h:%n:%j");
      else if (STREQ(pos, "BYTES") || STREQ(pos, "SC-BYTES"))
	strcat(fmt, "%b");
      else if (STREQ(pos, "SC-STATUS"))
	strcat(fmt, "%c");
      else if (STREQ(pos, "C-DNS") || STREQ(pos, "CS-DNS"))
	strcat(fmt, "%S");
      else if (STREQ(pos, "CS-IP") || STREQ(pos, "C-IP"))
	strcat(fmt, "%s");
      else if (STREQ(pos, "CS-URI-STEM") || STREQ(pos, "CS-URI"))
	strcat(fmt, "%r");
      else if (STREQ(pos, "CS(REFERER)")) {
	if (c == '5')  /* Microsoft */
	  strcat(fmt, "%f");
	else
	  strcat(fmt, "\"%f\"");
      }
      else if (STREQ(pos, "CS(USER-AGENT)")) {
	if (c == '5')  /* Microsoft */
	  strcat(fmt, "%A");
	else
	  strcat(fmt, "\"%B\"");
      }
      else if (STREQ(pos, "CS-HOST")) {
	if (c == '6')  /* WebSTAR */
	  strcat(fmt, "%S");
	else
	  strcat(fmt, "%v");
      }
      else if (STREQ(pos, "CS(HOST)"))
	strcat(fmt, "\"%v\"");
      else if (STREQ(pos, "S-IP") ||
	       STREQ(pos, "S-DNS") || STREQ(pos, "CS-SIP") ||
	       STREQ(pos, "S-SITENAME") || STREQ(pos, "S-COMPUTERNAME"))
	strcat(fmt, "%v");
      else if (STREQ(pos, "CS-URI-QUERY"))
	strcat(fmt, "%q");
      else if (STREQ(pos, "CS(FROM)"))
	strcat(fmt, "\"%u\"");
      else if (STREQ(pos, "CS-USERNAME") || STREQ(pos, "CS(COOKIE)"))
	strcat(fmt, "%u");
      else if (STREQ(pos, "TIME-TAKEN")) {
	if (c == '5')  /* Microsoft */
	  strcat(fmt, "%T");
	else
	  strcat(fmt, "%t");
      }
      else
	strcat(fmt, "%j");
      pos = strtok((char *)NULL, " \t");
    }
    if (c == '3')
      configlogfmt((void *)&logformat, NULL, "EXTENDED", NULL, -1);
    else if (c == '5')
      configlogfmt((void *)&logformat, NULL, "MS-EXTENDED", NULL, -1);
    else /* c == '6' */
      configlogfmt((void *)&logformat, NULL, "WEBSTAR-EXTENDED", NULL, -1);
    configlogfmt((void *)&logformat, NULL, fmt, NULL, -3);
    pos = d + 1;
    (void)parsenewline(lf, NULL, '\0');
    break;
  case '4':  /* Netscape format */
    resolveconflicts(2, "%", "Req->vars.auth-user", "Req->vars.pauth-user");
    resolveconflicts(2, "%", "Req->reqpb.clf-request",
		     "Req->reqpb.proxy-request");
    resolveconflicts(3, "%", "Req->srvhdrs.content-length",
		     "Req->headers.content-length", "Req->vars.p2c-cl");
    resolveconflicts(2, "%", "Req->vars.xfer-time",
		     "Req->vars.xfer-time-total");
    ENSURE_LEN(fmt, fmtlen, 32);   /* See above */
    fmt[0] = '\0';
    while (*pos != '\0') {
      ENSURE_LEN(fmt, fmtlen, strlen(fmt) + 18);  /* See above */
      if (*pos != '%') {
	e = strchr(fmt, '\0');
	*e = *(pos++);
	*(e + 1) = '\0';
      }
      else {
	if ((e = strchr(++pos, '%')) == NULL)
	  return(BADLOGFMT);
	else *e = '\0';
	if (STREQ(pos, "Ses->client.ip"))
	  strcat(fmt, "%S");
	else if (STREQ(pos, "Req->vars.auth-user") ||
		 STREQ(pos, "Req->vars.pauth-user"))
	  strcat(fmt, "%u");
	else if (STREQ(pos, "SYSDATE"))
	  strcat(fmt, "%d/%M/%Y:%h:%n:%j");
	else if (STREQ(pos, "Req->reqpb.clf-request") ||
		 STREQ(pos, "Req->reqpb.proxy-request"))
	  strcat(fmt, "%j%w%r%wHTTP%j");
	else if (STREQ(pos, "Req->srvhdrs.clf-status"))
	  strcat(fmt, "%c");
	else if (STREQ(pos, "Req->srvhdrs.content-length") ||
		 STREQ(pos, "Req->headers.content-length") ||
		 STREQ(pos, "Req->vars.p2c-cl"))
	  strcat(fmt, "%b");
	else if (STREQ(pos, "Req->headers.referer"))
	  strcat(fmt, "%f");
	else if (STREQ(pos, "Req->headers.user-agent"))
	  strcat(fmt, "%B");
	else if (STREQ(pos, "Req->headers.host"))
	  strcat(fmt, "%v");
	else if (STREQ(pos, "Req->vars.xfer-time") ||
		 STREQ(pos, "Req->vars.xfer-time-total"))
	  strcat(fmt, "%t");
	else
	  strcat(fmt, "%j");
	pos = e + 1;
      }
    }
    configlogfmt((void *)&logformat, NULL, "NETSCAPE", NULL, -1);
    configlogfmt((void *)&logformat, NULL, fmt, NULL, -3);
    pos = d + 1;
    (void)parsenewline(lf, NULL, '\0');
    break;
  }
  return(NEWLOGFMT);
}

choice parsenextrecord(Logfile *lf, Inputformat *format) {
  Inputformat *ipf, *ipf2;
  logical rc;

  if (pos == NULL) {   /* new logfile */
    if (getmoredata(lf, block_start, BLOCKSIZE) == EOF)
      return(EOF);
    pos = block_start;
    if (*pos == '\n' || *pos == '\r')
      parsenewline(lf, NULL, '\0');  /* ignore completely blank lines at top */
    record_start = pos;
  }

  pos = record_start;

  for (ipf = format; ipf->inpfns != NULL; TO_NEXT(ipf)) {

    if (pos > block_bell) {
      if (record_start != block_start)
	shunt_data(lf);
      if (pos == block_end && block_end - block_start < BLOCKSIZE)
	return(EOF);
    }

    rc = ipf->inpfns->fn(lf, ipf->inpfns->opt, ipf->sep);
    if (rc != TRUE) {
      for (ipf2 = format; ipf2 != ipf; TO_NEXT(ipf2)) {
	if (ipf2->inpfns->fn == &parsestring ||
	    ipf2->inpfns->fn == &parseref)
	  ((Memman *)(ipf2->inpfns->opt))->next_pos =
	    ((Memman *)(ipf2->inpfns->opt))->curr_pos;
	/* reset strings; NB ipf returned !TRUE so didn't allocate */
      }
      return(rc);
    }
  }
  return(TRUE);
}

int parseconfline(char *s, char **cmd, char **arg1, char **arg2) {
  char *c, d;

  for (c = s; *c == ' ' || *c == '\t'; c++)
    ;   /* run past white space */
  if (*c == '\0' || *c == '#')
    return(-1);
  *cmd = c;
  while (*c != ' ' && *c != '\t' && *c != '\0' && *c != '#')
    c++;
  if (*c == '\0' || *c == '#') {
    *c = '\0';
    return(0);
  }
  *c = '\0';
  c++;
  for ( ; *c == ' ' || *c == '\t'; c++)
    ;   /* run past white space again */
  if (*c == '\0' || *c == '#')
    return(0);
  *arg1 = c;
  if (*c == '\'' || *c == '"' || *c == '(') {
    d = (*c == '(')?')':(*c);  /* terminating character for next string */
    *arg1 = (++c);
    while (*c != d && *c != '\0')
      c++;
  }
  else while (*c != ' ' && *c != '\t' && *c != '\0' && *c != '#')
    c++;
  if (*c == '\0' || *c == '#') {
    *c = '\0';
    return(1);
  }
  *c = '\0';
  c++;
  for ( ; *c == ' ' || *c == '\t'; c++)
    ;
  if (*c == '\0' || *c == '#')
    return(1);
  *arg2 = c;
  if (*c == '\'' || *c == '"' || *c == '(') {
    d = (*c == '(')?')':(*c);
    *arg2 = (++c);
    while (*c != d && *c != '\0')
      c++;
  }
  else while (*c != ' ' && *c != '\t' && *c != '\0' && *c != '#')
    c++;
  if (*c == '\0' || *c == '#') {
    *c = '\0';
    return(2);
  }
  *c = '\0';
  c++;
  for ( ; *c == ' ' || *c == '\t'; c++)
    ;
  if (*c == '\0' || *c == '#')
    return(2);
  return(3);
}

int nextconfline(FILE *f, char **cmd, char **arg1, char **arg2) {
  /* if 255 increased, so must u[] be in confline() */
  size_t l;
  int rc;

  *cmd = NULL;
  *arg1 = NULL;
  *arg2 = NULL;
  while (TRUE) {
    if (getnextline(f, block_start, 0) == EOF)
      return(EOF);
    while ((l = strlen(block_start)) > 0 && *(block_start + l - 1) == '\\') {
      if (getnextline(f, block_start + l - 1, l - 1) == EOF)
	return(EOF);
    }
    if (l >= 255) {
      *(block_start + 70) = '\0';
      warn('C', TRUE, "Ignoring long configuration line starting\n%s",
	   block_start);
    }
    else if ((rc = parseconfline(block_start, cmd, arg1, arg2)) != -1)
      return(rc);  /* o/wise line was blank or a comment: go round again */
  }
}

char *nextlngstr(FILE *f, char *name, logical want) {

  while (TRUE) {
    if (getnextline(f, block_start, 0) == EOF) {  /* EOF reached */
      if (want)
	error("language file %s too short", name);
      else
	return((char *)NULL);
    }
    if (strlen(block_start) >= 255)
      error("language file %s contains excessively long lines", name);
    if (block_start[0] != '#' || block_start[1] != '#') {  /* found a line */
      if (want)
	return(block_start);
      else
	error("language file %s too long", name);
    }   /* otherwise just found a comment; go round again */
  }
}

choice nextdnsline(FILE *f, timecode_t *timec, char **name, char **alias) {
  char *timestr = NULL;

  *name = NULL;
  *alias = NULL;
  if (getnextline(f, block_start, 0) == EOF)
    return(EOF);
  if (strlen(block_start) >= 255) {
    *(block_start + 70) = '\0';
    warn('G', TRUE, "Ignoring long line in DNS file starting\n%s",
	 block_start);
    return(FALSE);
  }
  if ((timestr = strtok(block_start, " ")) == NULL ||
      (*name = strtok((char *)NULL, " ")) == NULL ||
      (*alias = strtok((char *)NULL, " ")) == NULL ||
      strtok((char *)NULL, " ") != NULL || !ISDIGIT(*timestr) ||
      (*timec = strtoul(timestr, (char **)NULL, 10)) == 0) {
    warn('G', TRUE, "Ignoring corrupt line in DNS file\n%s %s %s", block_start,
	 (*name == NULL)?"":(*name), (*alias == NULL)?"":(*alias));
    return(FALSE);
  }
  return(TRUE);
}

void process_domainsfile(FILE *f, Options *op) {
  /* size of v must be bounded because u in confline is */
  char *s, *t, *c, *u, v[256];

  while (TRUE) {
    if (getnextline(f, block_start, 0) == EOF)
      return;
    if (*block_start == '#')
      ;  /* ignore comment line */
    else if (strlen(block_start) >= 250) {
      *(block_start + 70) = '\0';
      warn('G', TRUE, "Ignoring long line in domains file starting\n%s",
	   block_start);
    }
    else {
      for (s = block_start; *s == ' ' || *s == '\t'; s++)
	;
      for (t = s; *t != ' ' && *t != '\t' && *t != '\0'; t++)
	;
      if (*t == '\0')
	warn('G', TRUE, "Ignoring incomplete line in domains file\n%s",
	     block_start);
      else {
	for (c = t + 1; *c == ' ' || *c == '\t'; c++)
	  ;
	if (*c < '1' || *c > '9' || (*(c + 1) != ' ' && *(c + 1) != '\t'))
	  warn('G', TRUE, "Ignoring corrupt line in domains file\n%s",
	       block_start);
	else {
	  *t = '\0';
	  *(c + 1) = '\0';
	  for (t = c + 2; *t == ' ' || *t == '\t'; t++)
	    ;
	  for (u = strchr(block_start, '\0') - 1;
	       (*u == ' ' || *u == '\t') && u > t; u--)
	    ;
	  if (u > t)
	    *(u + 1) = '\0';
	  confline(op, "DOMLEVEL", s, c, -1);
	  sprintf(v, "PLAIN:.%s (%s)", s, t);
	  confline(op, "DOMOUTPUTALIAS", s, v, -1);
	}
      }
    }
  }
}

void process_descfile(FILE *f, char *name, Options *op) {
  int i;

  for (i = 0; i < REP_NUMBER; i++) {
    if (getnextline(f, block_start, 0) == EOF) {
      warn ('F', TRUE, "report descriptions file %s too short: ignoring it",
	    name);
      op->outopts.descriptions = FALSE;
      return;
    }
    if (*block_start == '#')
      i--;  /* ignore comment line */
    else {
      if (strlen(block_start) > 500) {
	*(block_start + 500) = '\0';
	warn('G', TRUE, "Long line in report descriptions file: truncating it "
	     "at 500 characters");
	/* Limit is because of prettyprintf() */
      }
      COPYSTR(op->outopts.descstr[i], block_start);
    }
  }
  while (getnextline(f, block_start, 0) != EOF) {
    if (*block_start != '#') {
      warn ('F', TRUE, "report descriptions file %s too long: ignoring it",
	    name);
      op->outopts.descriptions = FALSE;
      return;
    }
  }
}
