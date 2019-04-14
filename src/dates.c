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

/*** dates.c; anything to do with dates. ***/

#include "anlghea3.h"

/* Analog stores times internally in minutes since midnight 30-31/12/1969.
   (Not in "Unix time" because it works better if all valid times are > 0).
   Note also that months are numbered Jan = 0 to Dec = 11 internally. */

unsigned int daysbefore[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273,
			       304, 334};  /* in non-leap year */
unsigned int monthlength[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,
				31};

void code2date(datecode_t code, unsigned int *date, unsigned int *month,
	       unsigned int *year) {
  /* not most efficient possible, but only used in output so doesn't need to
     be super-fast. NB DATE2CODE() is in anlghea3.h. */
  code += 364;    /* so 0 -> 1/1/69 */
  *year = 1969 + 4 * (code / 1461);
  code %= 1461;
  *year += MIN(code / 365, 3); /* no leap days for a while after 1/1/69! */
  if (code == 1460) {
    *month = DEC;
    *date = 31;
  }
  else {
    code %= 365;    /* so 0 -> 1/Jan, any year */
    for (*month = DEC;
	 daysbefore[*month] + (IS_LEAPYEAR(*year) && *month > FEB) > code;
	 (*month)--)
      ;   /* run to right month */
    *date = code - daysbefore[*month] + 1 -
      (IS_LEAPYEAR(*year) && *month > FEB);
  }
}

time_t shifttime(time_t timer, int diff) {
  /* Surprisingly, there is no ANSI C function to do this */
  struct tm *lt = localtime(&timer);
  lt->tm_min += diff;
  timer = mktime(lt);
  return(timer);
}

choice parsedate(time_t starttime, char *s, timecode_t *date, logical from,
		 logical unixtime) {
  struct tm *st;
  int y, m, d, h, n;
  char *p;

#ifndef NOGMTIME
  if (unixtime)
    st = gmtime(&starttime);
  else
#endif
    st = localtime(&starttime);
  if (ISDIGIT(*s) && ISDIGIT(*(s + 1))) {
    y = 10 * (*s - '0') + (*(s + 1) - '0');
    s += 2;
    y += 1900;
    if (y < 1970)
      y += 100;
  }
  else if (*s == '+' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
    y = st->tm_year + 1900 + 10 * (*(s + 1) - '0') + (*(s + 2) - '0');
    s += 3;
  }
  else if (*s == '-' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
    y = st->tm_year + 1900 - 10 * (*(s + 1) - '0') - (*(s + 2) - '0');
    s += 3;
  }
  else
    return(ERR);

  if (ISDIGIT(*s) && ISDIGIT(*(s + 1))) {
    m = 10 * (*s - '0') + (*(s + 1) - '0') - 1;
    if (m > 11 || m < 0)
      return(ERR);
    s += 2;
  }
  else if (*s == '+' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
    m = st->tm_mon + 10 * (*(s + 1) - '0') + (*(s + 2) - '0');
    s += 3;
  }
  else if (*s == '-' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
    m = st->tm_mon - 10 * (*(s + 1) - '0') - (*(s + 2) - '0');
    s += 3;
  }
  else
    return(ERR);
  while (m < 0) {   /* need to do this now because about to use monthlength */
    m += 12;
    y--;
  }
  while (m > 11) {
    m -= 12;
    y++;
  }

  if (ISDIGIT(*s) && ISDIGIT(*(s + 1))) {
    d = (int)strtol(s, &p, 10);
    if (d > 31 || d <= 0)
      return(ERR);
    else if (d > (int)(monthlength[m]) + (m == FEB && IS_LEAPYEAR(y)))
      d = monthlength[m] + (m == FEB && IS_LEAPYEAR(y));
  }                   /* relative dates must be >= 2 digits but can be more */
  else if (*s == '+' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2)))
    d = st->tm_mday + (int)strtol(s + 1, &p, 10);
  else if (*s == '-' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2)))
    d = st->tm_mday - (int)strtol(s + 1, &p, 10);
  else
    return(ERR);

  if (*p == ':') {  /* parse hour & minute */
    s = p + 1;
    if (ISDIGIT(*s) && ISDIGIT(*(s + 1))) {
      h = 10 * (*s - '0') + (*(s + 1) - '0');
      if (h > 23)
	return(ERR);
      s += 2;
    }
    else if (*s == '+' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
      h = st->tm_hour + 10 * (*(s + 1) - '0') + (*(s + 2) - '0');
      s += 3;
    }
    else if (*s == '-' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
      h = st->tm_hour - 10 * (*(s + 1) - '0') - (*(s + 2) - '0');
      s += 3;
    }
    else
      return(ERR);

    if (ISDIGIT(*s) && ISDIGIT(*(s + 1))) {
      n = 10 * (*s - '0') + (*(s + 1) - '0');
      if (n > 59)
	return(ERR);
      s += 2;
    }
    else if (*s == '+' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
      n = st->tm_min + 10 * (*(s + 1) - '0') + (*(s + 2) - '0');
      s += 3;
    }
    else if (*s == '-' && ISDIGIT(*(s + 1)) && ISDIGIT(*(s + 2))) {
      n = st->tm_min - 10 * (*(s + 1) - '0') - (*(s + 2) - '0');
      s += 3;
    }
    else
      return(ERR);

    if (*s != '\0')
      return(ERR);
  }                 /* end *p == ':' */
  else if (*p == '\0' || *p == 'd' || *p == 'D' || *p == 'e' || *p == 'E') {
    /* d, e can come from FLOOR */
    if (from) {
      h = 0;
      n = 0;
    }
    else {
      h = 23;
      n = 59;
    }
  }
  else
    return(ERR);

  while (n < 0) {
    n += 60;
    h--;
  }
  while (n > 59) {
    n -= 60;
    h++;
  }
  while (h < 0) {
    h += 24;
    d--;
  }
  while (h > 23) {
    h -= 24;
    d++;
  }
  while (d < 0) {
    m--;
    if (m < 0) {   /* NB already adjusted m once above */
      m += 12;
      y--;
    }
    d += monthlength[m] + (m == FEB && IS_LEAPYEAR(y));
  }
  while (d > (int)(monthlength[m]) + (m == FEB && IS_LEAPYEAR(y))) {
    d -= monthlength[m] + (m == FEB && IS_LEAPYEAR(y));
    m++;
    if (m > 11) {
      m -= 12;
      y++;
    }
  }

  *date = TIMECODE(DATE2CODE(y, m, d), h, n);
#ifndef NOGMTIME
  if (unixtime)
    *date -= 1440;   /* Unix time is one day different from analog time */
#endif
  return(OK);
}

choice wantdate(timecode_t *timecode, Dateman *dman, unsigned int hour,
		unsigned int minute, unsigned int date, unsigned int month,
		unsigned int year, int tz) {
  static unsigned int oldy = 0, oldm = 0, oldd = 0, oldh = 25;
  static timecode_t oldtime = 0;

  /* first check integrity of date */
  if (month > 11 || hour > 23 || minute > 59 || date == 0 ||
      date > monthlength[month] + (month == FEB && IS_LEAPYEAR(year)) ||
      year < 1970 || year > 2069)
    return(ERR);

  if (hour != oldh || date != oldd || month != oldm || year != oldy) {
    oldtime = TIMECODE(DATE2CODE(year, month, date), hour, 0);
    oldh = hour;                        /* only calculate once an hour */
    oldd = date;
    oldm = month;
    oldy = year;
  }

  *timecode =  oldtime + minute + tz;
  if (*timecode < dman->from || *timecode > dman->to)
    return(FALSE);
  return(TRUE);
}

choice wantunixtime(timecode_t *timecode, Dateman *dman,
		    unsigned long unixtime, int tz) {
  /* no need to check range because parseunixtime() catches it */
  *timecode = UXTIME2CODE(unixtime) + tz;
  if (*timecode < dman->from || *timecode > dman->to)
    return(FALSE);
  return(TRUE);
}

Daysdata *newday(unsigned int granularity) {
  extern Memman *xmemman;

  Daysdata *dp;
  unsigned int i;

  dp = (Daysdata *)submalloc(xmemman, sizeof(Daysdata));
  dp->reqs = (unsigned long *)submalloc(xmemman,
					granularity * sizeof(unsigned long));
  dp->pages = (unsigned long *)submalloc(xmemman,
					 granularity * sizeof(unsigned long));
  dp->bytes = (double *)submalloc(xmemman, granularity * sizeof(double));

  for (i = 0; i < granularity; i++) {
    dp->reqs[i] = 0;
    dp->pages[i] = 0;
    dp->bytes[i] = 0.0;
  }

  dp->prev = NULL;
  dp->next = NULL;

  return(dp);
}

void dmaninit(Dateman *dman, datecode_t datecode, unsigned int granularity) {
  dman->firstdate = datecode;
  dman->lastdate = datecode;
  dman->currdate = datecode;
  dman->firstdp = newday(granularity);
  dman->lastdp = dman->firstdp;
  dman->currdp = dman->firstdp;
}

void datehash(timecode_t timecode, Dateman *dman, unsigned long reqs,
	      unsigned long pages, double bytes, unsigned int granularity) {
  int i;
  Daysdata *dp;

  datecode_t datecode = (datecode_t)(timecode/1440);

  if (dman->currdp == NULL)
    dmaninit(dman, datecode, granularity);
  if (datecode >= dman->lastdate) {
    for (i = datecode - dman->lastdate; i > 0; i--) {
      dp = newday(granularity);
      dman->lastdp->next = dp;
      dp->prev = dman->lastdp;
      dman->lastdp = dp;
      dman->lastdate = datecode;
    }
    dman->currdp = dman->lastdp;
  }
  else if (datecode >= dman->currdate) {
    for (i = datecode - dman->currdate; i > 0; i--)
      dman->currdp = dman->currdp->next;
  }
  else if (datecode <= dman->firstdate) {
    for (i = dman->firstdate - datecode; i > 0; i--) {
      dp = newday(granularity);
      dman->firstdp->prev = dp;
      dp->next = dman->firstdp;
      dman->firstdp = dp;
      dman->firstdate = datecode;
    }
    dman->currdp = dman->firstdp;
  }
  else {
    dman->currdp = dman->firstdp;
    for (i = datecode - dman->firstdate; i > 0; i--) {
      dman->currdp = dman->currdp->next;
    }
  }

  dman->currdate = datecode;

  i = ((timecode % 1440) * granularity) / 1440;
  dman->currdp->reqs[i] += reqs;
  dman->currdp->pages[i] += pages;
  dman->currdp->bytes[i] += bytes;
}
