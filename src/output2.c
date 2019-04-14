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

/*** output2.c; subsiduary output functions ***/

#include "anlghea3.h"

extern unsigned int *rep2lng;
extern choice *rep2type, *rep2reqs, *rep2reqs7, *rep2date, *rep2firstd;
extern char *repname[];

void report_title(FILE *outf, Outchoices *od, choice rep) {

  /* The actual report title */
  od->outputter->reporttitle(outf, od, rep);

  /* The report description text */
  if (od->descriptions && od->descstr[rep] != NULL)
    od->outputter->reportdesc(outf, od, rep);
}

void report_footer(FILE *outf, Outchoices *od, choice rep) {
  od->outputter->reportfooter(outf, od, rep);
  od->outputter->hrule(outf, od);
}

/* The period represented by the report. At the moment, this is a function of
   the report, not the underlying item type. Either choice makes some sense,
   though, and it would just be a matter of changing the calculation of maxd &
   min before passing them into this function. */
/* Assume od->repspan already tested. */
void reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd,
		timecode_t mind, Dateman *dman) {
  if (maxd == FIRST_TIME || mind == LAST_TIME ||
      (mind - dman->firsttime < od->rsthresh && 
       dman->lasttime - maxd < od->rsthresh))
    return;

  od->outputter->reportspan(outf, od, rep, maxd, mind);
}

void matchlength(FILE *outf, Outchoices *od, char *s, char c) {
  size_t i;

  for (i = od->outputter->strlength(s); i > 0; i--)
    od->outputter->putch(outf, c);
}

void matchlengthn(FILE *outf, Outchoices *od, int width, char c) {
  for ( ; width > 0; width--)
    od->outputter->putch(outf, c);
}

/*** Date printing routine ***/

size_t datefmtlen(Outchoices *od, char *fmt) {
  /* Page width required for formatted date. All dates should be the same,
     so just format an arbitrary one and measure it. */
  return(od->outputter->strlength(datesprintf(od, fmt, 1, 23, 59, 1, 23, 59,
					   FALSE, UNSET)));
}

char *datesprintf(Outchoices *od, char *fmt, datecode_t date, unsigned int hr,
		  unsigned int min, datecode_t newdate, unsigned int newhr,
		  unsigned int newmin, logical running, choice allowmonth) {
  /* Formats date. Allocates space as necessary, but 2nd call will overwrite */
  /* If od is NULL, must have running == TRUE and allowmonth != UNSET.
     Otherwise, if allowmonth is UNSET, follow what the od->outputter says. */
  extern char *engmonths[], *engshortdays[];
  static char *ans = NULL;
  static size_t len = 0;

  size_t (*strlenp)(const char *);
  char * (*endashp)(void);
  char **monthname, **dayname, *compsep;
  size_t monthlen, daylen, ampmlen, plainmonthlen, plaindaylen, plainampmlen;
  size_t current, increment;
  unsigned int d, m, y, d2, m2, y2, n, i;
  char *s, *c, *am, *pm;

  if (od == NULL) {   /* Not in output routine */
    strlenp = &strlen;
    endashp = &plain_endash;
    monthname = engmonths;
    dayname = engshortdays;
    compsep = NULL;
    am = "am";
    pm = "pm";
    plainmonthlen = ENGMONTHLEN;
    plaindaylen = ENGSHORTDAYLEN;
    plainampmlen = 2;
  }
  else {
    strlenp = od->outputter->strlength;
    endashp = od->outputter->endash;
    monthname = od->monthname;
    dayname = od->dayname;
    compsep = od->compsep;
    am = od->lngstr[am_];
    pm = od->lngstr[pm_];
    plainmonthlen = od->plainmonthlen;
    plaindaylen = od->plaindaylen;
    plainampmlen = od->plainampmlen;
    if (allowmonth == UNSET)
      allowmonth = od->outputter->allowmonth();
  }
  if (running) {    /* Running text: no extra spacing to line things up */
    monthlen = 0;
    daylen = 0;
    ampmlen = 0;
  }
  else {
    monthlen = od->monthlen;
    daylen = od->daylen;
    ampmlen = od->ampmlen;
  }
  increment = monthlen + plainmonthlen + daylen + plaindaylen + ampmlen +
    plainampmlen + ((compsep == NULL)?0:strlen(compsep)) + 5;
  /* A (naive) upper bound on the amount by which the length of the answer
     might grow in one step; cf comment under plainmonthlen in init.c. */

  if (date == 0 || date == LAST_DATE) {
    n = chrn(fmt, '\b');
    ENSURE_LEN(ans, len, n * ((compsep == NULL)?0:strlen(compsep)) + 1);
    s = ans;
    if (compsep != NULL) {
      for (i = 0; i < n; i++)
	PUTs(s, compsep, 0);
    }
    *s = '\0';
    return(ans);
  }
  code2date(date, &d, &m, &y);
  code2date(newdate, &d2, &m2, &y2);
  ENSURE_LEN(ans, len, 1);  /* in case fmt is "" */
  for (c = fmt, s = ans; *c != '\0'; c++) {
    current = (ans == NULL)?0:(size_t)(s - ans);
    ENSURE_LEN(ans, len, current + increment);
    s = ans + current;   /* in case ans was moved when realloc'ed */
    if (*c == '%' && *(c + 1) != '\0') {
      c++;
      switch (*c) {
      case '%':
	PUTc(s, '%');
	break;
      case 'd':
	PUT2d(s, d);
	break;
      case 'D':
	PUT02d(s, d);
	break;
      case 'e':
	PUT2d(s, d2);
	break;
      case 'E':
	PUT02d(s, d2);
	break;
      case 'l':
	if (monthname != NULL)
	  PUTs(s, monthname[m2],
	       (int)monthlen - (int)strlenp(monthname[m2]));
	break;
      case 'L':
	if (allowmonth)
	  PUT02d(s, m2 + 1);
	break;
      case 'm':
	if (monthname != NULL)
	  PUTs(s, monthname[m],
	       (int)monthlen - (int)strlenp(monthname[m]));
	break;
      case 'M':
	if (allowmonth)
	  PUT02d(s, m + 1);
	break;
      case 'q':
	PUT1d(s, (m / 3) + 1);
	break;
      case '\b':  /* \b only used internally */
	if (compsep != NULL)
	  PUTs(s, compsep, 0);
	break;
      case 'y':
	PUT02d(s, y % 100);
	break;
      case 'Y':
	PUT04d(s, y);
	break;
      case 'z':
	PUT02d(s, y2 % 100);
	break;
      case 'Z':
	PUT04d(s, y2);
	break;
      case 'X':
	PUT04d(s, y);
	PUTc(s, '-');
	PUT02d(s, m + 1);
	PUTc(s, '-');
	PUT02d(s, d);
	break;
      case 'h':
	PUT2d(s, hr);
	break;
      case 'H':
	PUT02d(s, hr);
	break;
      case 'j':
	i = hr % 12;
	if (i == 0)
	  i = 12;
	PUT2d(s, i);
	break;
      case 'a':
	if (hr < 12 || hr == 24)
	  PUTs(s, am, (int)ampmlen - (int)strlenp(am))
	else       /* no semicolon above because of definition of PUTs */
	  PUTs(s, pm, (int)ampmlen - (int)strlenp(pm));
	break;
      case 'i':
	PUT2d(s, newhr);
	break;
      case 'I':
	PUT02d(s, newhr);
	break;
      case 'k':
	i = newhr % 12;
	if (i == 0)
	  i = 12;
	PUT2d(s, i);
	break;
      case 'b':
	if (newhr < 12 || newhr == 24)
	  PUTs(s, am, (int)ampmlen - (int)strlenp(am))
	else       /* no semicolon above because of definition of PUTs */
	  PUTs(s, pm, (int)ampmlen - (int)strlenp(pm));
	break;
      case 'n':
	PUT02d(s, min);
	break;
      case 'o':
	PUT02d(s, newmin);
	break;
      case 'w':
	if (dayname != NULL)
	  PUTs(s, dayname[DAYOFWEEK(date)],
	       (int)daylen - (int)strlenp(dayname[DAYOFWEEK(date)]));
	break;
      case 'x':
	PUTs(s, endashp(), 0);
	break;
      }  /* switch *c */
    }    /* if *c == '%' */
    else
      PUTc(s, *c);
  }  /* for c */
  *s = '\0';
  return(ans);
}

char *timesprintf(Outchoices *od, char *fmt, timecode_t t, choice allowmonth) {
  /* Just a wrapper for the most common case of datesprintf(). */
  return(datesprintf(od, fmt, t / 1440, (t % 1440) / 60, t % 60, 0, 0, 0, TRUE,
		     allowmonth));
}

int f3printf(FILE *outf, Outchoices *od, double x, unsigned int width,
	     char sepchar) {
  /* Return number of characters printed, but counting e.g. &amp; as 1. */
  /* NB The sepchar is sometimes repsepchar */
  int ans, i;

  x += EPSILON;   /* just to make sure rounding down works OK */
  if (sepchar == '\0')
    return(fprintf(outf, "%*.0f", width, x));

  for (i = 0; x >= 1000; i++)
    x /= 1000;  /* find out how big x is to get number of leading spaces */
  ans = fprintf(outf, "%*d", MAX((int)width - 4 * i, 0), (int)x);
  ans += 4 * i;
  /* now run down again, printing each clump */
  for ( ; i > 0; i--) {
    od->outputter->putch(outf, sepchar);
    x -= (int)x;
    x *= 1000;
    fprintf(outf, "%03d", (int)x);
  }
  return(ans);
}

void printbytes(FILE *outf, Outchoices *od, double bytes, unsigned int bmult,
		unsigned int width, char sepchar) {

  unsigned int dp = od->bytesdp;

  int by1;
  double by2, rounder;
  unsigned int j;

  if (bmult == 0)
    (void)f3printf(outf, od, bytes, width, sepchar);
  else {
    for (j = 0; j < bmult; j++)
      bytes /= 1024; /* divide bytes to get kilobytes, megabytes or whatever */

    /* Add some amount in order to round to the correct number of decimal
       places accurately: 0.5 for 0 d.p.s, 0.05 for 1 d.p. etc. */
    rounder = 0.5;
    for (j = 0; j < dp; j++)
      rounder /= 10.0;
    bytes += rounder;

    if (dp == 0) {  /* fractional part not wanted */
      fprintf(outf, "%*d", width, (int)bytes);
    }
    else {
      by1 = (int)bytes;    /* whole number of kilo/mega/etc. bytes */
      width -= MIN(width, dp + 1);  /* leave room for fractional part */
      fprintf(outf, "%*d", width, by1);
      by2 = (bytes - (double)by1);  /* fractional part */
      for (j = 0; j < dp; j++)
	by2 *= 10;
      od->outputter->putch(outf, od->decpt);
      fprintf(outf, "%0*d", dp, (int)by2);
    }
  }
}

double findunit(Outchoices *od, double n, unsigned int width[], choice *cols) {
  int w;
  double unit;
  int c;
  int i, j;

  w = (int)(od->outputter->pagewidth(od)) - (int)width[COL_TITLE] - 2;
  for (c = 0; cols[c] != COL_NUMBER; c++)
    w -= (int)width[cols[c]] + 2;
  w = MAX(w, (int)(od->mingraphwidth));
  /* unit must be nice amount: i.e., {1, 1.5, 2, 2.5, 3, 4, 5, 6, 8} * 10^n */
  unit = ((n - 1) / (double)w);
  j = 0;
  while (unit > 24.) {
    unit /= 10.;
    j++;
  }
  unit = (double)((int)unit);
  if (unit == 6.)
    unit = 7.;
  else if (unit == 8.)
      unit = 9.;
  else if (unit >= 20.)
    unit = 24.;
  else if (unit >= 15.)
    unit = 19.;
  else if (unit >= 10.)
    unit = 14.;
  unit += 1.;
  for (i = 0; i < j; i++) {
    unit *= 10.;
  }
  return(unit);
}

/* The widths which columns want to take. The od->outputters have their own
   version of this function, but usually they just call this one */
void calcwidths(Outchoices *od, choice rep, unsigned int width[],
		unsigned int *bmult, unsigned int *bmult7, double *unit,
		unsigned long maxr, unsigned long maxr7, unsigned long maxp,
		unsigned long maxp7, double maxb, double maxb7,
		unsigned long howmany) {
  /* width[COL_TITLE] should be set before calling this function. */
  /* width[COL_TITLE] == 0 signifies that the title is last and this function
     should calculate the remaining width. */
  /* *unit == 0 for timegraphs (and it's then set here); non-zero otherwise. */
  extern unsigned int *col2colhead;

  char repsepchar = od->repsepchar;
  char graphby = od->graph[rep];
  choice *cols = od->cols[rep];
  char **lngstr = od->lngstr;

  int w;
  unsigned int i;

  width[COL_REQS] = MAX(LEN3(log10i(maxr) + 1, repsepchar),
			od->outputter->strlength(lngstr[col2colhead[COL_REQS]]));
  width[COL_REQS7] = MAX(LEN3(log10i(maxr7) + 1, repsepchar),
			 od->outputter->strlength(lngstr[col2colhead[COL_REQS7]]));
  width[COL_PAGES] = MAX(LEN3(log10i(maxp) + 1, repsepchar),
			 od->outputter->strlength(lngstr[col2colhead[COL_PAGES]]));
  width[COL_PAGES7] = MAX(LEN3(log10i(maxp7) + 1, repsepchar),
			  od->outputter->strlength(lngstr[col2colhead[COL_PAGES7]]));
  if (od->rawbytes || maxb < 1024.0) {
    width[COL_BYTES] = MAX(LEN3(log10x(maxb) + 1, repsepchar),
			   od->outputter->strlength(lngstr[col2colhead[COL_BYTES]]));
    *bmult = 0;
  }
  else {
    *bmult = findbmult(maxb, od->bytesdp);
    width[COL_BYTES] =
      MAX(3 + od->bytesdp + (od->bytesdp != 0),
	  od->outputter->strlength(lngstr[col2colhead[COL_BYTES] + 1])
	  + od->outputter->strlength(lngstr[byteprefixabbr_ + *bmult]) - 1);
  }
  /* I have some misgivings about allowing the bmult7 to be different from
     the bmult. It's less immediately readable. But I think it's necessary,
     because maxb and maxb7 are quite often different orders of magnitude. */
  if (od->rawbytes || maxb7 < 1024.0) {
    width[COL_BYTES7] =
      MAX(LEN3(log10x(maxb7) + 1, repsepchar),
	  od->outputter->strlength(lngstr[col2colhead[COL_BYTES7]]));
    *bmult7 = 0;
  }
  else {
    *bmult7 = findbmult(maxb7, od->bytesdp);
    width[COL_BYTES7] =
      MAX(3 + od->bytesdp + (od->bytesdp != 0),
	  od->outputter->strlength(lngstr[col2colhead[COL_BYTES7] + 1])
	  + od->outputter->strlength(lngstr[byteprefixabbr_ + *bmult7]) - 1);
  }
  width[COL_PREQS] = MAX(6, od->outputter->strlength(lngstr[col2colhead[COL_PREQS]]));
  width[COL_PREQS7] =
    MAX(6, od->outputter->strlength(lngstr[col2colhead[COL_PREQS7]]));
  width[COL_PPAGES] =
    MAX(6, od->outputter->strlength(lngstr[col2colhead[COL_PPAGES]]));
  width[COL_PPAGES7] =
    MAX(6, od->outputter->strlength(lngstr[col2colhead[COL_PPAGES7]]));
  width[COL_PBYTES] =
    MAX(6, od->outputter->strlength(lngstr[col2colhead[COL_PBYTES]]));
  width[COL_PBYTES7] =
    MAX(6, od->outputter->strlength(lngstr[col2colhead[COL_PBYTES7]]));
  width[COL_DATE] = MAX(datefmtlen(od, lngstr[genrepdate_]),
			od->outputter->strlength(lngstr[col2colhead[COL_DATE]]));
  width[COL_TIME] = MAX(datefmtlen(od, lngstr[genreptime_]),
			od->outputter->strlength(lngstr[col2colhead[COL_TIME]]));
  width[COL_FIRSTD] = MAX(datefmtlen(od, lngstr[genrepdate_]),
			  od->outputter->strlength(lngstr[col2colhead[COL_FIRSTD]]));
  width[COL_FIRSTT] = MAX(datefmtlen(od, lngstr[genreptime_]),
			  od->outputter->strlength(lngstr[col2colhead[COL_FIRSTT]]));
  width[COL_INDEX] = MAX(LEN3(log10i(howmany) + 1, repsepchar),
			 od->outputter->strlength(lngstr[col2colhead[COL_INDEX]]));
  if (*unit == 0) { /* i.e. a timegraph */
    if (graphby == 'R' || graphby == 'r')
      *unit = findunit(od, (double)maxr, width, cols);
    else if (graphby == 'P' || graphby == 'p')
      *unit = findunit(od, (double)maxp, width, cols);
    else {
      for (i = 0; i < *bmult; i++)
	maxb /= 1024;
      if (*bmult > 0)
	maxb *= 1000;
      *unit = findunit(od, maxb, width, cols);
      if (*bmult > 0)
	*unit /= 1000;
    }
  }
  if (width[COL_TITLE] == 0) {
    w = (int)(od->outputter->pagewidth(od));
    for (i = 0; cols[i] != COL_NUMBER; i++)
      w -= (int)width[cols[i]] + 2;
    width[COL_TITLE] = (unsigned int)MAX(0, w);
  }

}

unsigned int alphatreewidth(Outchoices *od, choice rep, Hashtable *tree,
			    unsigned int level, Strlist *partname) {
  /* Calculate width needed for Organisation Report.
     Constructing the name is basically the same code as printtree(). */
  extern char *workspace;

  char *name;
  size_t need = (size_t)level + 3;
  Strlist *pn, s;
  Hashindex *p;
  unsigned int tw = 0, tmp;

  if (tree == NULL || tree->head[0] == NULL)
    return(0);
  for (p = tree->head[0]; p != NULL; TO_NEXT(p)) {
    name = maketreename(partname, p, &pn, &s, need, rep, TRUE);
    if (!STREQ(name, LNGSTR_NODOMAIN) && !STREQ(name, LNGSTR_UNKDOMAIN) &&
	!ISDIGIT(name[strlen(name) - 1])) { /* ignore left-aligned ones */
      strcpy(workspace, name);
      do_aliasx(workspace, od->aliashead[G(rep)]);
      tmp = od->outputter->strlength(workspace) + 2 * level;
                       /* will be printed with 2 trailing spaces per level */
      tw = MAX(tw, tmp);
      tmp = alphatreewidth(od, rep, (Hashtable *)(p->other), level + 1, pn);
      tw = MAX(tw, tmp);
      /* The second tmp will of course be bigger unless there are aliases
	 (if there are any children at all). */
    }
  }
  return(tw);
}

/* Declare the floor and sortby for a report. The od->outputters have their own
   version of this function, but most of them just call this one to sort out
   all the language strings. */
void whatincluded(FILE *outf, Outchoices *od, choice rep, unsigned long n,
		  Dateman *dman) {
  extern char *byteprefix;
  extern unsigned int *method2sing, *method2pl, *method2date, *method2pc;
  extern unsigned int *method2relpc, *method2sort;

  char **lngstr = od->lngstr;
  choice sortby = od->sortby[G(rep)];
  double floormin = od->floor[G(rep)].min;
  char floorqual = od->floor[G(rep)].qual;
  choice floorby = od->floor[G(rep)].floorby;
  char *gens = lngstr[rep2lng[rep] + 1];
  char *genp = lngstr[rep2lng[rep] + 2];
  char gender = lngstr[rep2lng[rep] + 3][0];
  choice requests = rep2reqs[G(rep)];
  choice requests7 = rep2reqs7[G(rep)];
  choice date = rep2date[G(rep)];
  choice firstd = rep2firstd[G(rep)];

  int firsts, firstds, alls, sorted, alphsort, unsort, bmult;
  char *lngs, *c;
  static char *t = NULL;
  static size_t tlen = 0;
  unsigned long temp = 0;
  unsigned long temp2;
  int i;
  timecode_t tempd;

  if (gender == 'm') {
    firsts = firstsm_;
    firstds = firstdsm_;
    alls = allsm_;
    sorted = sortedm_;
    alphsort = STREQ(gens, lngstr[codegs_])?numsortm_:alphasortm_;
    unsort = unsortedm_;            /* quickest kludge for only one report */
  }
  else if (gender == 'f') {
    firsts = firstsf_;
    firstds = firstdsf_;
    alls = allsf_;
    sorted = sortedf_;
    alphsort = STREQ(gens, lngstr[codegs_])?numsortf_:alphasortf_;
    unsort = unsortedf_;
  }
  else { /* gender == 'n' */
    firsts = firstsn_;
    firstds = firstdsn_;
    alls = allsn_;
    sorted = sortedn_;
    alphsort = STREQ(gens, lngstr[codegs_])?numsortn_:alphasortn_;
    unsort = unsortedn_;
  }

  /* see also report_floor() in settings.c */
  od->outputter->whatinchead(outf, od);
  if (floormin < 0 && n < (unsigned long)(-floormin + EPSILON))
    floormin = 1;  /* not enough items for requested -ve floor */
  /* floormin = 1 will work even for date sort because it will be before
     dman->firsttime. With very high probability. :) */
  if (floormin < 0) {
    temp = (unsigned long)(-floormin + EPSILON);
    if (temp == 1) {
      ENSURE_LEN(t, tlen, strlen(lngstr[firsts]) + strlen(gens) + 1);
      sprintf(t, lngstr[firsts], gens);
      od->outputter->whatincprintstr(outf, od, t);
    }
    else {
      for (temp2 = temp, i = 1; temp2 >= 10; temp2 /= 10)
	i++;
      ENSURE_LEN(t, tlen, strlen(lngstr[firstds]) + strlen(genp) + i + 1);
      sprintf(t, lngstr[firstds], temp, genp);
      od->outputter->whatincprintstr(outf, od, t);
    }
    od->outputter->whatincprintstr(outf, od, " ");
    od->outputter->whatincprintstr(outf, od, lngstr[floorby_]);
    od->outputter->whatincprintstr(outf, od, " ");
    if (floorby == REQUESTS)
      od->outputter->whatincprintstr(outf, od, lngstr[method2sort[requests]]);
    else if (floorby == REQUESTS7)
      od->outputter->whatincprintstr(outf, od, lngstr[method2sort[requests7]]);
    else if (floorby == DATESORT)
      od->outputter->whatincprintstr(outf, od, lngstr[method2sort[date]]);
    else if (floorby == FIRSTDATE)
      od->outputter->whatincprintstr(outf, od, lngstr[method2sort[firstd]]);
    else
      od->outputter->whatincprintstr(outf, od, lngstr[method2sort[floorby]]);
  }
  else {   /* floormin >= 0 */
    ENSURE_LEN(t, tlen, strlen(lngstr[alls]) + strlen(genp) + 1);
    sprintf(t, lngstr[alls], genp);
    od->outputter->whatincprintstr(outf, od, t);
    if (floormin < 2 - EPSILON && floorqual == '\0' && floorby == REQUESTS)
      floormin = 0;  /* Report 1r as 0r */
    if (floorby == DATESORT || floorby == FIRSTDATE) {
      tempd = (timecode_t)(floormin + EPSILON);
      if (tempd > dman->firsttime) {
	od->outputter->whatincprintstr(outf, od, " ");
	od->outputter->whatincprintstr(outf, od,
		   lngstr[method2date[(floorby == DATESORT)?date:firstd]]);
	od->outputter->whatincprintstr(outf, od, " ");
	od->outputter->whatincprintstr(outf, od, timesprintf(od, lngstr[whatincfmt_],
						     tempd, UNSET));
      }
    }
    else if (floormin > EPSILON) {
      od->outputter->whatincprintstr(outf, od, " ");
      od->outputter->whatincprintstr(outf, od, lngstr[atleast_]);
      od->outputter->whatincprintstr(outf, od, " ");
      if (floorqual == '\0') {
	temp = (unsigned long)(floormin + EPSILON);
	for (temp2 = temp, i = 1; temp2 >= 10; temp2 /= 10)
	  i++;
	ENSURE_LEN(t, tlen, i + 1);
	sprintf(t, "%lu", temp);
	od->outputter->whatincprintstr(outf, od, t);
	od->outputter->whatincprintstr(outf, od, " ");
	if (floorby == REQUESTS)
	  od->outputter->whatincprintstr(outf, od, (temp == 1)?\
		  lngstr[method2sing[requests]]:lngstr[method2pl[requests]]);
	else if (floorby == REQUESTS7)
	  od->outputter->whatincprintstr(outf, od, (temp == 1)?\
		  lngstr[method2sing[requests7]]:\
		  lngstr[method2pl[requests7]]);
	else
	  od->outputter->whatincprintstr(outf, od, (temp == 1)?\
		  lngstr[method2sing[floorby]]:lngstr[method2pl[floorby]]);
      }
      else {  /* floorqual != '\0' */
	od->outputter->printdouble(outf, od, floormin);
	if (floorqual == '%') {
	  if (floorby == REQUESTS)
	    c = lngstr[method2pc[requests]];
	  else if (floorby == REQUESTS7)
	    c = lngstr[method2pc[requests7]];
	  else
	    c = lngstr[method2pc[floorby]];
	  od->outputter->whatincprintstr(outf, od, c);
	}
	else if (floorqual == ':') {
	  if (floorby == REQUESTS)
	    c = lngstr[method2relpc[requests]];
	  else if (floorby == REQUESTS7)
	    c = lngstr[method2relpc[requests7]];
	  else
	    c = lngstr[method2relpc[floorby]];
	  od->outputter->whatincprintstr(outf, od, c);
	}
	else { /* if qual is anything else, must be (k|M|G|T|etc.)bytes */
	  lngs = (floorby == BYTES)?lngstr[xbytestraffic_]:\
	    lngstr[xbytestraffic7_];
	  if (strchr(byteprefix, floorqual) == NULL)  /* shouldn't happen */
	    bmult = 1;
	  else
	    bmult = strchr(byteprefix, floorqual) - byteprefix;
	  c = strchr(lngs, '?');  /* checked during initialisation */
	  *c = '\0';
	  od->outputter->whatincprintstr(outf, od, " ");
	  od->outputter->whatincprintstr(outf, od, lngs);
	  od->outputter->whatincprintstr(outf, od,
					 lngstr[byteprefix_ + bmult]);
	  od->outputter->whatincprintstr(outf, od, c + 1);
	  *c = '?';
	}
      }   /* end floorqual != '\0' */
    }     /* end floormin > EPSILON */
  }       /* end floormin > 0 */
  /* That completes the floor; now we are just left with the sortby */
  if (floormin >= 0 || temp != 1) { /* else only one item, so no sort */
    if (floormin < 0 && sortby == RANDOM)
      sortby = floorby;
    od->outputter->whatincprintstr(outf, od, ", ");
    if (sortby == ALPHABETICAL)
      od->outputter->whatincprintstr(outf, od, lngstr[alphsort]);
    else if (sortby == RANDOM)
      od->outputter->whatincprintstr(outf, od, lngstr[unsort]);
    else {
      od->outputter->whatincprintstr(outf, od, lngstr[sorted]);
      od->outputter->whatincprintstr(outf, od, " ");
      if (sortby == REQUESTS)
	od->outputter->whatincprintstr(outf, od,
				       lngstr[method2sort[requests]]);
      else if (sortby == REQUESTS7)
	od->outputter->whatincprintstr(outf, od,
				       lngstr[method2sort[requests7]]);
      else if (sortby == DATESORT)
	od->outputter->whatincprintstr(outf, od, lngstr[method2sort[date]]);
      else if (sortby == FIRSTDATE)
	od->outputter->whatincprintstr(outf, od, lngstr[method2sort[firstd]]);
      else
	od->outputter->whatincprintstr(outf, od, lngstr[method2sort[sortby]]);
    }
  }
  od->outputter->whatincprintstr(outf, od, ".");
  od->outputter->whatincfoot(outf, od);
}

void barchart(FILE *outf, Outchoices *od, char graphby, unsigned long reqs,
	      unsigned long pages, double bys, double unit) {
  double x;
  int y;

  if (graphby == 'P' || graphby == 'p')
    x = (double)pages - 0.5;
  else if (graphby == 'R' || graphby == 'r')
    x = (double)reqs - 0.5;
  else
    x = bys;
  x /= unit;
  x += 1;
  y = (int)x;

  od->outputter->barchart(outf, od, y, graphby);
}

void colheads(FILE *outf, Outchoices *od, choice rep, unsigned int width[],
	      unsigned int bmult, unsigned int bmult7, logical name1st) {
  extern unsigned int *col2colhead, *rep2colhead;
  static char *bcolname = NULL;
  static size_t bcolnamelen = 0;

  char **lngstr = od->lngstr;
  choice *cols = od->cols[rep];

  char *colname, *d;
  unsigned int c, bm;

  /* Column headers */

  od->outputter->colheadstart(outf, od, rep);

  if (name1st)
    od->outputter->colheadcol(outf, od, rep, COL_TITLE, width[COL_TITLE],
			      lngstr[rep2colhead[rep]], FALSE);
  for (c = 0; cols[c] != COL_NUMBER; c++) {
    if (cols[c] == COL_BYTES)
      bm = bmult;
    else if (cols[c] == COL_BYTES7)
      bm = bmult7;
    else
      bm = 0;

    if (bm == 0)
      colname = lngstr[col2colhead[cols[c]]];
    else {  /* special case for bytes: insert kilo, mega, etc. */
      colname = lngstr[col2colhead[cols[c]] + 1];
      d = strchr(colname, '?');  /* checked during initialisation */
      *d = '\0';
      ENSURE_LEN(bcolname, bcolnamelen,
		 strlen(colname) + strlen(lngstr[byteprefixabbr_ + bm]) +
		 strlen(d + 1) + 1);
      sprintf(bcolname, "%s%s%s", colname, lngstr[byteprefixabbr_ + bm],
	      d + 1);
      *d = '?';
      colname = bcolname;
    }
    od->outputter->colheadcol(outf, od, rep, cols[c], width[cols[c]], colname,
			      FALSE);
  }  /* for (c = 0) */
  if (!name1st)
    od->outputter->colheadcol(outf, od, rep, COL_TITLE, width[COL_TITLE],
			      lngstr[rep2colhead[rep]], TRUE);
      
  od->outputter->colheadend(outf, od, rep);

  /* Underlinings */

  od->outputter->colheadustart(outf, od, rep);
 
  if (name1st)
    od->outputter->colheadunderline(outf, od, rep, COL_TITLE,
				width[COL_TITLE], ""); 
  for (c = 0; cols[c] != COL_NUMBER; c++)
    od->outputter->colheadunderline(outf, od, rep, cols[c], width[cols[c]],
				    ""); 
  if (!name1st)
    od->outputter->colheadunderline(outf, od, rep, COL_TITLE, 0,
				lngstr[rep2colhead[rep]]); 

  od->outputter->colheaduend(outf, od, rep);
}

void printcols(FILE *outf, Outchoices *od, choice rep, unsigned long reqs,
	       unsigned long reqs7, unsigned long pages, unsigned long pages7,
	       double bys, double bys7, long index, int level,
	       unsigned long totr, unsigned long totr7, unsigned long totp,
	       unsigned long totp7, double totb, double totb7,
	       unsigned int width[], unsigned int bmult, unsigned int bmult7,
	       double unit, logical name1st, logical rightalign, char *name,
	       logical ispage, unsigned int spaces, Include *linkhead,
	       char *baseurl, char *datefmt, char *timefmt, datecode_t date,
	       unsigned int hr, unsigned int min, datecode_t date2,
	       unsigned int hr2, unsigned int min2) {
  /* 'level' is -1 for time reports, 0 for other non-hierarchical reports,
     and starts at 1 for hierarchical reports. */
  /* For time reps, date2, hr2 & min2 carry the end of the interval; for
     genreps, date2, hr2 & min2 carry the time of first request. */

  choice *cols = od->cols[rep];
  logical timerep = (rep < DATEREP_NUMBER);
  char graphby = timerep?(od->graph[rep]):'\0';

  choice source;
  char *datestr;
  unsigned int c, i;

  od->outputter->rowstart(outf, od, rep, cols, level, name, datefmt, timefmt);

  if (timerep) {
    name = datesprintf(od, datefmt, date, hr, min, date2, hr2, min2, FALSE,
		       UNSET);
    source = AS_IS;
  }
  else if (rep == REP_SIZE || rep == REP_PROCTIME)
    source = TRUSTED;
  else 
    source = UNTRUSTED;

  od->outputter->namecell(outf, od, rep, name, source, width[COL_TITLE],
			  name1st, TRUE, FALSE, NULL, NULL, FALSE, 0, NULL);

  od->outputter->levelcell(outf, od, rep, level);

  for (c = 0; cols[c] != COL_NUMBER; c++) {
    switch(cols[c]) {
    case COL_REQS:
      od->outputter->ulcell(outf, od, rep, cols[c], reqs, width[cols[c]]);
      break;
    case COL_REQS7:
      od->outputter->ulcell(outf, od, rep, cols[c], reqs7, width[cols[c]]);
      break;
    case COL_PREQS:
      od->outputter->pccell(outf, od, rep, cols[c], (double)reqs, (double)totr,
			width[cols[c]]);
      break;
    case COL_PREQS7:
      od->outputter->pccell(outf, od, rep, cols[c], (double)reqs7,
			    (double)totr7, width[cols[c]]);
      break;
    case COL_PAGES:
      od->outputter->ulcell(outf, od, rep, cols[c], pages, width[cols[c]]);
      break;
    case COL_PAGES7: 
      od->outputter->ulcell(outf, od, rep, cols[c], pages7, width[cols[c]]);
      break;
    case COL_PPAGES:
      od->outputter->pccell(outf, od, rep, cols[c], (double)pages,
			    (double)totp, width[cols[c]]);
      break;
    case COL_PPAGES7:
      od->outputter->pccell(outf, od, rep, cols[c], (double)pages7,
			    (double)totp7, width[cols[c]]);
      break;
    case COL_BYTES:
      od->outputter->bytescell(outf, od, rep, cols[c], bys, bmult,
			       width[cols[c]]);
      break;
    case COL_BYTES7:
      od->outputter->bytescell(outf, od, rep, cols[c], bys7, bmult7,
			   width[cols[c]]);
      break;
    case COL_PBYTES:
      od->outputter->pccell(outf, od, rep, cols[c], bys, totb, width[cols[c]]);
      break;
    case COL_PBYTES7:
      od->outputter->pccell(outf, od, rep, cols[c], bys7, totb7,
			    width[cols[c]]);
      break;
    case COL_DATE:
    case COL_TIME:
    case COL_FIRSTD:
    case COL_FIRSTT:
      if (cols[c] == COL_DATE || cols[c] == COL_TIME)
	datestr = datesprintf(od, (cols[c] == COL_DATE)?datefmt:timefmt,
			      date, hr, min, 0, 0, 0, FALSE, UNSET);
      else
	datestr = datesprintf(od, (cols[c] == COL_FIRSTD)?datefmt:timefmt,
			      date2, hr2, min2, 0, 0, 0, FALSE, UNSET);
      od->outputter->strcell(outf, od, rep, cols[c], datestr, width[cols[c]]);
      break;
    case COL_INDEX:
      od->outputter->indexcell(outf, od, rep, cols[c], index, width[cols[c]]);
      break;
    }
  }

  if (timerep) {
    for (i = 0; i < (int)bmult; i++)
      bys /= 1024;
    barchart(outf, od, graphby, reqs, pages, bys, unit);
  }

  if (name == NULL)
     return;   /* calling function supplies name and newline */

  if (rep == REP_CODE)
    source = TRUSTED;
  else 
    source = UNTRUSTED;
  od->outputter->namecell(outf, od, rep, name, source, width[COL_TITLE],
			  name1st, FALSE, rightalign,
			  G(rep)>=0?od->aliashead[G(rep)]:NULL, linkhead,
			  ispage, spaces, baseurl);

  od->outputter->rowend(outf, od, rep);
}

/*** Now some stuff for the General Summary ***/

void distcount(Hashindex *gooditems, Hashindex *baditems, choice requests,
	       choice requests7, unsigned long *tot, unsigned long *tot7) {
  Hashindex *p;

  for (p = gooditems, *tot = 0, *tot7 = 0; p != NULL; TO_NEXT(p)) {
    if (p->own != NULL) {
      if (p->own->data[requests] > 0)
	(*tot)++;
      if (p->own->data[requests7] > 0)
	(*tot7)++;
    }
  }
  for (p = baditems; p != NULL; TO_NEXT(p)) {
    if (p->own != NULL) {
      if (p->own->data[requests] > 0)
	(*tot)++;
      if (p->own->data[requests7] > 0)
	(*tot7)++;
    }
  }
}

void gensumline(FILE *outf, Outchoices *od, char codeletter, int namecode,
		unsigned long x, unsigned long x7, logical isaverage) {
  if (strchr(od->gensumlines, codeletter) == NULL)
    return;

  if (x == (unsigned long)UNSET)
    return;

  if (x == 0 && namecode != succreqs_)
    return;

  od->outputter->gensumline(outf, od, namecode, x, x7, isaverage);
}

void gensumlineb(FILE *outf, Outchoices *od, char codeletter, int namecode,
		 double x, double x7, logical isaverage) {
  /* same as gensumline() but for bytes */

  if (strchr(od->gensumlines, codeletter) == NULL)
    return;

  if (x <= 0)
    return;

  od->outputter->gensumlineb(outf, od, namecode, x, x7, isaverage);
}

logical checkonerep(Outchoices *od, Hashindex *gp, choice rep, choice requests,
		    cutfnp cutfn, dcutfnp dcutfn, void *darg) {
  extern Memman *amemman;

  static char *newname = NULL, *dnewname = NULL;
  static size_t len = 0, dlen = 0;

  char *namestart, *nameend, *name;
  choice rc;

  /* Procedure: go through all the entries. If the report will use that entry
     (it has any requests, and the name is included()) turn the report on.
     To calculate the name, we have to call cutfn if the report is a tree
     report, and/or (dcutfn and alias) if it is a dervrep. */
  for ( ; gp != NULL; TO_NEXT(gp)) {
    if (gp->own != NULL && gp->own->data[requests] > 0) {
      name = gp->name;
      if (cutfn == NULL && dcutfn != NULL) {
	/* The search reports. Here dcutfn can produce zero or multiple answers
	   for each name, and we have to check them all. */
	namestart = NULL;
	for (dcutfn(&namestart, &nameend, name, darg); namestart != NULL;
	     dcutfn(&namestart, &nameend, name, darg)) {
	  ENSURE_LEN(dnewname, dlen, (size_t)(nameend - namestart + 1));
	  memcpy((void *)dnewname, (void *)namestart,
		 (size_t)(nameend - namestart));
	  dnewname[nameend - namestart] = '\0';
	  if ((rc = do_alias(dnewname, amemman, NULL, NULL, 0, FALSE,
			     od->convfloor, od->multibyte, rep)) != ERR) {
	    name = rc?((char *)(amemman->curr_pos)):dnewname;
	    if (included(name, FALSE, od->wanthead[G(rep)]))
	      return(TRUE);
	  }
	}
      }
      else {
	/* otherwise each name produces just one answer to check; the name
	   itself, or if a tree report the name at the top level of the tree */
	if (cutfn != NULL) {  /* if it's a tree report */
	  if (dcutfn != NULL) {  /* if it's also a derv report */
	    /* Here we rely on the fact that if it's both a tree rep and a derv
	       rep, then dcutfn will produce exactly one name. (See comment on
	       dcutfnp in tree.c). */
	    namestart = NULL;
	    dcutfn(&namestart, &nameend, name, darg);
	    ENSURE_LEN(dnewname, dlen, (size_t)(nameend - namestart + 1));
	    memcpy((void *)dnewname, (void *)namestart,
		   (size_t)(nameend - namestart));
	    dnewname[nameend - namestart] = '\0';
	    if ((rc = do_alias(dnewname, amemman, NULL, NULL, 0, FALSE,
			       od->convfloor, od->multibyte, rep)) == ERR)
	      name = NULL;
	    else
	      name = rc?((char *)(amemman->curr_pos)):dnewname;
	  }
	  if (name != NULL) {
	    namestart = NULL;
	    cutfn(&namestart, &nameend, name, FALSE);
	    ENSURE_LEN(newname, len, (size_t)(nameend - namestart + 1));
	    memcpy((void *)newname, (void *)namestart,
		   (size_t)(nameend - namestart));
	    newname[nameend - namestart] = '\0';
	    name = newname;
	  }
	}
	if (name != NULL &&
	    included(name, gp->own->ispage, od->wanthead[G(rep)]))
	  return(TRUE);
      }
    }
  }
  return(FALSE);  /* nothing matched, so turn the report off */
}

logical checktreerep(Outchoices *od, Hashtable *tp, choice rep,
		     choice requests, cutfnp cutfn) {
  unsigned long i;

  for (i = 0; i < tp->size; i++) {
    if (checkonerep(od, tp->head[i], rep, requests, cutfn, NULL, NULL))
      return(TRUE);
  }
  return(FALSE);
}

logical checkarrayrep(Arraydata *array) {
  choice i;

  for (i = 0; ; i++) {
    if (array[i].reqs > 0)
      return(TRUE);
    if (array[i].threshold < -0.5)
      return(FALSE);
  }
}

void checkreps(Outchoices *od, Dateman *dman, Hashindex **gooditems,
	       Arraydata **arraydata,
	       choice data2cols[ITEM_NUMBER][DATA_NUMBER]) {
  extern logical *repistree;

  logical *repq = od->repq;

  cutfnp cutfn;
  dcutfnp dcutfn;
  void *darg;
  choice rep;
  int j;
  choice ok;

  if (dman->currdp == NULL) {
    for (rep = 0; rep < DATEREP_NUMBER; rep++) {
      if (repq[rep]) {
	warn('R', TRUE, "Turning off empty time reports");
	for ( ; rep < DATEREP_NUMBER; rep++)
	  repq[rep] = FALSE;
      }
    }
  }
  for (rep = FIRST_GENREP; rep <= LAST_NORMALREP; rep++) {
    cutfn = repistree[G(rep)]?(od->tree[G(rep)]->cutfn):NULL;
    dcutfn = (rep >= FIRST_DERVREP)?(od->derv[rep - FIRST_DERVREP]->cutfn):\
      NULL;
    darg = (rep >= FIRST_DERVREP)?(od->derv[rep - FIRST_DERVREP]->arg):NULL;
    for (ok = 0, j = 0; od->alltrees[j] != REP_NUMBER; j++) {
      if (rep == od->alltrees[j])
	ok = 1;
    }
    for (j = 0; od->alldervs[j] != REP_NUMBER; j++) {
      if (rep == od->alldervs[j])
	ok = 2;
    }
    if (ok) {
      if (!checktreerep(od, (ok == 1)?(od->tree[G(rep)]->tree):\
			(od->derv[rep - FIRST_DERVREP]->table), rep,
			data2cols[rep2type[rep]][rep2reqs[G(rep)]],
			(ok == 1)?NULL:cutfn)) {
	/* If ok == 1, tree made, so done cutfn already; if ok == 2, made derv
	   but not tree, so still need cutfn. NB i in alltrees or alldervs
	   implies repq so don't have to check that. */
	warn('R', TRUE, "Turning off empty %s", repname[rep]);
	repq[rep] = FALSE;
      }
    }
    else if (repq[rep]) {
      if (!checkonerep(od, gooditems[rep2type[rep]], rep,
		       data2cols[rep2type[rep]][rep2reqs[G(rep)]], cutfn,
		       dcutfn, darg)) {
	warn('R', TRUE, "Turning off empty %s", repname[rep]);
	repq[rep] = FALSE;
      }
    }
  }
  for ( ; rep < REP_NUMBER; rep++) {
    if (repq[rep] && !checkarrayrep(arraydata[rep - FIRST_ARRAYREP])) {
      warn('R', TRUE, "Turning off empty %s", repname[rep]);
      repq[rep] = FALSE;
    }
  }
}
