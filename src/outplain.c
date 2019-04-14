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

/*** outplain.c; plain text output ***/

#include "anlghea3.h"

/* Page width */
unsigned int plain_pagewidth(Outchoices *od) {
  return od->plainpagewidth;
}

/* The top of the output if we are in CGI mode */
void plain_cgihead(FILE *outf, Outchoices *od) {
  fprintf(outf, "Content-Type: text/plain\n\n");
}

/* Stuff this output style needs in the page header */
void plain_stylehead(FILE *outf, Outchoices *od) {
}

/* The title of the page, plus the user's HEADERFILE */
void plain_pagetitle(FILE *outf, Outchoices *od) {
  fprintf(outf, "%s %s\n", od->lngstr[webstatsfor_], od->hostname);
  matchlength(outf, od, od->hostname, '=');
  matchlength(outf, od, od->lngstr[webstatsfor_], '=');
  fputs("=\n\n", outf);

  if (!strcaseeq(od->headerfile, "none"))
    plain_includefile(outf, od, od->headerfile, 'h');
}

/* Program start time, and logfile start and end times */
void plain_timings(FILE *outf, Outchoices *od, Dateman *dman) {
  extern timecode_t starttimec;

  char **lngstr = od->lngstr;

  double t0;
  int t1, t2;

  if (od->runtime)
    fprintf(outf, "%s %s.\n", lngstr[progstart_],
	    timesprintf(od, lngstr[datefmt2_], starttimec, UNSET));

  if (dman->firsttime <= dman->lasttime) {
    mprintf(outf, od->plainpagewidth, "%s %s ", lngstr[reqstart_],
	    timesprintf(od, lngstr[datefmt2_], dman->firsttime, UNSET));
    mprintf(outf, od->plainpagewidth, "%s %s", lngstr[to_],
	    timesprintf(od, lngstr[datefmt2_], dman->lasttime, UNSET));
    t0 = (dman->lasttime - dman->firsttime) / 1440.0 + 0.005;
    t1 = (int)t0;
    t2 = (int)(100 * (t0 - (double)t1));
    mprintf(outf, od->plainpagewidth, " (%d", t1);
    putc(od->decpt, outf);
    mprintf(outf, od->plainpagewidth, "%02d %s).", t2, od->lngstr[days_]);
    mprintf(outf, 0, NULL);
  }
}

/* Finishing the top of the page */
void plain_closehead(FILE *outf, Outchoices *od) {
  plain_hrule(outf, od);
}

/* Starting the bottom of the page */
void plain_pagebotstart(FILE *outf, Outchoices *od) {
}

/* The credit line at the bottom of the page */
void plain_credit(FILE *outf, Outchoices *od) {
  fprintf(outf, "%s analog %s.\n", od->lngstr[credit_], VNUMBER);
}

/* The program run time */
void plain_runtime(FILE *outf, Outchoices *od, long secs) {
  char **lngstr = od->lngstr;

  fprintf(outf, "%s: ", lngstr[runtime_]);

  if (secs == 0)
    fprintf(outf, "%s %s.\n", lngstr[lessone_], lngstr[second_]);
  else if (secs < 60)
    fprintf(outf, "%ld %s.\n", secs, 
	    (secs == 1)?lngstr[second_]:lngstr[seconds_]);
  else
    fprintf(outf, "%ld %s, %ld %s.\n", secs / 60,
	    (secs < 120)?lngstr[minute_]:lngstr[minutes_], secs % 60,
	    (secs % 60 == 1)?lngstr[second_]:lngstr[seconds_]);
}

/* The page footer, including the user's FOOTERFILE */
void plain_pagefoot(FILE *outf, Outchoices *od) {
  if (!strcaseeq(od->footerfile, "none"))
    plain_includefile(outf, od, od->footerfile, 'f');
}

/* Footer material for this output style */
void plain_stylefoot(FILE *outf, Outchoices *od) {
}

/* Report title */
void plain_reporttitle(FILE *outf, Outchoices *od, choice rep) {
  extern unsigned int *rep2lng;

  fprintf(outf, "%s\n", od->lngstr[rep2lng[rep]]);
  matchlength(outf, od, od->lngstr[rep2lng[rep]], '-');
  putc('\n', outf);
}

/* Report footer */
void plain_reportfooter(FILE *outf, Outchoices *od, choice rep) {
}

/* Report description */
void plain_reportdesc(FILE *outf, Outchoices *od, choice rep) {
  mprintf(outf, od->plainpagewidth, "%s", od->descstr[rep]);
  mprintf(outf, 0, NULL);
  if (rep != REP_SIZE && rep != REP_PROCTIME)
    putc('\n', outf);
  /* These two reports probably have no further header text.
      If this is wrong, it's corrected in plain_reportspan() later. */
}

/* The time period spanned by the report */
void plain_reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd,
		      timecode_t mind) {
  if ((rep == REP_SIZE || rep == REP_PROCTIME) && od->descriptions &&
      od->descstr[rep] != NULL)
    putc('\n', outf);
  /* We were wrong when we assumed in plain_reportdesc() that these reports
     had no further header text. So correct for it now. */

  /* NB Can't combine next two lines because timesprintf uses static buffer. */
  mprintf(outf, od->plainpagewidth, "%s %s ", od->lngstr[repspan_],
	  timesprintf(od, od->lngstr[datefmt2_], mind, UNSET));
  mprintf(outf, od->plainpagewidth, "%s %s.", od->lngstr[to_],
	  timesprintf(od, od->lngstr[datefmt2_], maxd, UNSET));
  mprintf(outf, 0, NULL);

  if (rep != REP_SIZE && rep != REP_PROCTIME)
    putc('\n', outf);
  /* This time these reports really do have no further header text. */
}

/* General Summary header */
void plain_gensumhead(FILE *outf, Outchoices *od) {
}

/* General Summary footer */
void plain_gensumfoot(FILE *outf, Outchoices *od) {
}

/* Single General Summary line, long data */
void plain_gensumline(FILE *outf, Outchoices *od, int namecode,
		      unsigned long x, unsigned long x7, logical isaverage) {

  fprintf(outf, "%s%s ",  od->lngstr[namecode], od->lngstr[colon_]);

  f3printf(outf, od, (double)x, 0, od->sepchar);

  if (x7 != (unsigned long)UNSET) {
    fputs(" (", outf);
    f3printf(outf, od, (double)x7, 0, od->sepchar);
    putc(')', outf);
  }
  putc('\n', outf);
}

/* Single General Summary line, bytes data */
void plain_gensumlineb(FILE *outf, Outchoices *od, int namecode, double x,
		       double x7, logical isaverage) {
  char **lngstr = od->lngstr;

  unsigned int bm;
  char *c;

  fprintf(outf, "%s%s ",  lngstr[namecode], lngstr[colon_]);

  bm = (od->rawbytes)?0:findbmult(x, od->bytesdp);
  printbytes(outf, od, x, bm, 0, od->sepchar);

  if (bm > 0) {
    c = strchr(lngstr[xbytes_], '?');
    *c = '\0';
    fprintf(outf, " %s%s%s", lngstr[xbytes_], lngstr[byteprefix_ + bm], c + 1);
    *c = '?';
  }
  else
    fprintf(outf, " %s", lngstr[bytes_]);

  if (x7 != UNSET) {
    fputs(" (", outf);
    bm = (od->rawbytes)?0:findbmult(x7, od->bytesdp);
    printbytes(outf, od, x7, bm, 0, od->sepchar);
    if (bm > 0) {
      c = strchr(lngstr[xbytes_], '?');
      *c = '\0';
      fprintf(outf, " %s%s%s)", lngstr[xbytes_], lngstr[byteprefix_ + bm],
	      c + 1);
      *c = '?';
    }
    else
      fprintf(outf, " %s)", lngstr[bytes_]);
  }

  putc('\n', outf);
}

/* "Last seven" explanation line */
void plain_lastseven(FILE *outf, Outchoices *od, timecode_t last7to) {
  mprintf(outf, od->plainpagewidth, "(%s %s %s).\n", od->lngstr[brackets_],
	  od->lngstr[sevendaysto_],
	  timesprintf(od, od->lngstr[datefmt1_], last7to, UNSET));
  mprintf(outf, 0, NULL);
}

/* Start of a <pre> section */
void plain_prestart(FILE *outf, Outchoices *od) {
  putc('\n', outf);
}

/* End of a <pre> section */
void plain_preend(FILE *outf, Outchoices *od) {
}

/* A horizontal rule */
void plain_hrule(FILE *outf, Outchoices *od) {
  unsigned int i;

  for (i = 0; i < od->plainpagewidth; i++)
    putc('-', outf);
  fputs("\n\n", outf);
}

/* An en dash */
char *plain_endash(void) {
  return "-";
}

/* putc with special characters escaped */
void plain_putch(FILE *outf, char c) {
  putc(c, outf);
}

/* strlen */
size_t plain_strlength(const char *s) {
  return strlen(s);
}

/* Allow month in dates? DO NOT enable for human-readable text because of
   i18n problems. */
logical plain_allowmonth(void) {
  return FALSE;
}

/* Calculate column widths */
void plain_calcwidths(Outchoices *od, choice rep, unsigned int width[],
		      unsigned int *bmult, unsigned int *bmult7, double *unit,
		      unsigned long maxr, unsigned long maxr7,
		      unsigned long maxp, unsigned long maxp7, double maxb,
		      double maxb7, unsigned long howmany) {
  calcwidths(od, rep, width, bmult, bmult7, unit, maxr, maxr7, maxp, maxp7,
	     maxb, maxb7, howmany);
}

/* "Each unit represents" line */
void plain_declareunit(FILE *outf, Outchoices *od, char graphby, double unit,
		       unsigned int bmult) {
  /* NB Number can still overflow pagewidth, but only if pagewidth is small,
     & will wrap straight afterwards. pagewidth is never guaranteed anyway. */
  extern unsigned int ppcol;
  char **lngstr = od->lngstr;
  unsigned int pagewidth = od->plainpagewidth;

  char *s;

  mprintf(outf, pagewidth, "%s (%c) %s ", lngstr[eachunit_], od->markchar,
	  lngstr[represents_]);

  if (graphby == 'R' || graphby == 'r') {
    ppcol += f3printf(outf, od, unit, 0, od->sepchar);
    if (unit == 1.)
      mprintf(outf, pagewidth, " %s.", lngstr[request_]);
    else
      mprintf(outf, pagewidth, " %s %s.", lngstr[requests_],
	      lngstr[partof_]);
  }
  else if (graphby == 'P' || graphby == 'p') {
    ppcol += f3printf(outf, od, unit, 0, od->sepchar);
    if (unit == 1.)
      mprintf(outf, pagewidth, " %s.", lngstr[pagereq_]);
    else
      mprintf(outf, pagewidth, " %s %s.", lngstr[pagereqs_],
	      lngstr[partof_]);
  }
  else {
    if (bmult > 0) {
      plain_printdouble(outf, od, unit);
      s = strchr(lngstr[xbytes_], '?');  /* checked in initialisation */
      *s = '\0';
      mprintf(outf, pagewidth, " %s%s%s %s.", lngstr[xbytes_],
	      lngstr[byteprefix_ + bmult], s + 1, lngstr[partof_]);
      *s = '?';
    }
    else {
      ppcol += f3printf(outf, od, unit, 0, od->sepchar);
      mprintf(outf, pagewidth, " %s %s.", lngstr[bytes_], lngstr[partof_]);
    }
  }
  mprintf(outf, 0, NULL);
}

/* Start of column header line */
void plain_colheadstart(FILE *outf, Outchoices *od, choice rep) {
}

/* Column header line: individual column */
void plain_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
		      unsigned int width, char *colname,
		      logical unterminated) {
  if (unterminated)
    fputs(colname, outf);
  else
    fprintf(outf, "%*s: ", width, colname);
}

/* End of column header line */
void plain_colheadend(FILE *outf, Outchoices *od, choice rep) {
  putc('\n', outf);
}

/* Start of column header underlining line */
void plain_colheadustart(FILE *outf, Outchoices *od, choice rep) {
}

/* Underlining of one column header. */
/* If column is terminated, set width and leave name blank; and conversely. */
void plain_colheadunderline(FILE *outf, Outchoices *od, choice rep, choice col,
			    unsigned int width, char *name) {
  if (width > 0) {
    matchlengthn(outf, od, (size_t)width, '-');
    fputs(": ", outf);
  }
  else
    matchlength(outf, od, name, '-');
}

/* End of column header underlining line */
void plain_colheaduend(FILE *outf, Outchoices *od, choice rep) {
  putc('\n', outf);
}

/* Start of a table row */
void plain_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
		    int level, char *name, char *datefmt, char *timefmt) {
}

/* Print level in hierarchy represented by this row */
void plain_levelcell(FILE *outf, Outchoices *od, choice rep, int level) {
}

/* Name column */
void plain_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
		    choice source, unsigned int width, logical name1st,
		    logical isfirst, logical rightalign, Alias *aliashead,
		    Include *linkhead, logical ispage, unsigned int spaces,
		    char *baseurl) {
  extern char *workspace;

  int i;

  if (name1st != isfirst)
    return;

  if (isfirst)
    matchlengthn(outf, od, width - strlen(name), ' ');

  strcpy(workspace, name);
  do_aliasx(workspace, aliashead);

  if (!isfirst) {
    if (rightalign)
      i = (int)width - (int)strlen(workspace) - (int)spaces;
    else
      i = (int)spaces;
    matchlengthn(outf, od, i, ' ');
  }
    
  fputs(workspace, outf);

  if (isfirst)
    fputs(": ", outf);
}

/* Single cell, unsigned long argument */
void plain_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
		  unsigned long x, unsigned int width) {
  f3printf(outf, od, (double)x, width, od->repsepchar);
  fputs(": ", outf);
}

/* Single cell, TRUSTED string argument */
void plain_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
		   char *s, unsigned int width) {
  matchlengthn(outf, od, width - strlen(s), ' ');
  fputs(s, outf);
  fputs(": ", outf);
}

/* Single cell, listing bytes */
void plain_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
		     double b, double bmult, unsigned int width) {
  printbytes(outf, od, b, bmult, width, od->repsepchar);
  fputs(": ", outf);
}

/* Single cell, listing percentage */
void plain_pccell(FILE *outf, Outchoices *od, choice rep, choice col, double n,
		  double tot, unsigned int width) {
  double pc;
  unsigned int pc1, pc2;

  matchlengthn(outf, od, width - 6, ' ');
  if (tot == 0)
    pc = 0.0;
  else
    pc = n * 10000.0 / tot;
  if (pc >= 9999.5)
    fputs("  100%", outf);
  else if (pc < 0.5)
    fputs("      ", outf);
  else {
    pc1 = ((int)(pc + 0.5)) / 100;
    pc2 = ((int)(pc + 0.5)) % 100;
    fprintf(outf, "%2d", pc1);
    putc(od->decpt, outf);
    fprintf(outf, "%02d%%", pc2);
  }
  fputs(": ", outf);
}

/* Single cell, index */
void plain_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
		    long index, unsigned int width) {
  /* If index is 0 (i.e. sub-item), just print spaces */
  if (index <= 0)
    matchlengthn(outf, od, width, ' ');
  else
    f3printf(outf, od, (double)index, width, od->repsepchar);
  fputs(": ", outf);
}

/* End of a table row */
void plain_rowend(FILE *outf, Outchoices *od, choice rep) {
  putc('\n', outf);
}

/* Blank line in time reports */
void plain_blankline(FILE *outf, Outchoices *od, choice *cols) {
  putc('\n', outf);
}

/* Barchart in time reports */
void plain_barchart(FILE *outf, Outchoices *od, int y, char graphby) {
  int i;

  for (i = 0; i < y; i++)
    putc(od->markchar, outf);
}

/* "Busiest time period" line */
void plain_busyprintf(FILE *outf, Outchoices *od, choice rep, char *datefmt,
		      unsigned long reqs, unsigned long pages, double bys,
		      datecode_t date, unsigned int hr, unsigned int min,
		      datecode_t newdate, unsigned int newhr,
		      unsigned int newmin, char graphby) {
  extern unsigned int *rep2busystr;

  char **lngstr = od->lngstr;
  char sepchar = od->sepchar;

  unsigned int bmult;
  char *s;

  fprintf(outf, "\n%s %s (", lngstr[rep2busystr[rep]],
	  datesprintf(od, datefmt, date, hr, min, newdate, newhr, newmin,
		      TRUE, UNSET));
  if (TOLOWER(graphby) == 'r') {
    f3printf(outf, od, (double)reqs, 0, sepchar);
    fprintf(outf, " %s).\n", (reqs == 1)?lngstr[request_]:lngstr[requests_]);
  }
  else if (TOLOWER(graphby) == 'p') {
    f3printf(outf, od, (double)pages, 0, sepchar);
    fprintf(outf, " %s).\n",
	    (pages == 1)?lngstr[pagereq_]:lngstr[pagereqs_]);
  }
  else /* TOLOWER(graphby) == 'b' */ {
    if (od->rawbytes)
      bmult = 0;
    else
      bmult = findbmult(bys, od->bytesdp);
    printbytes(outf, od, bys, bmult, 0, sepchar);
    putc(' ', outf);
    if (bmult >= 1) {
      s = strchr(lngstr[xbytes_], '?');  /* checked in initialisation */
      *s = '\0';
      fprintf(outf, "%s%s%s).\n", lngstr[xbytes_],
	      lngstr[byteprefix_ + bmult], s + 1);
      *s = '?';
    }
    else
      fprintf(outf, "%s).\n", lngstr[bytes_]);
  }
}

/* End of "Not listed" line. */
void plain_notlistedstr(FILE *outf, Outchoices *od, choice rep,
			unsigned long badn) {
  extern unsigned int *rep2lng, *rep2colhead;

  char **lngstr = od->lngstr;
  char *colhead = lngstr[rep2colhead[rep]];
  char *colheadp = lngstr[rep2colhead[rep] + 1];
  char gender = lngstr[rep2lng[rep] + 3][0];

  char *notlistedstr;

  if (gender == 'm')
    notlistedstr = lngstr[notlistedm_];
  else if (gender == 'f')
    notlistedstr = lngstr[notlistedf_];
  else
    notlistedstr = lngstr[notlistedn_];

  fprintf(outf, "[%s: ", notlistedstr);
  f3printf(outf, od, (double)badn, 0, od->sepchar);
  fprintf(outf, " %s]", (badn == 1)?colhead:colheadp);
}

/* The line declaring the floor and sort for a report */
void plain_whatincluded(FILE *outf, Outchoices *od, choice rep,
			unsigned long n, Dateman *dman) {
  whatincluded(outf, od, rep, n, dman);
}

/* Spacing at the start of the whatincluded line */
void plain_whatinchead(FILE *outf, Outchoices *od) {
}

/* Finishing the whatincluded line */
void plain_whatincfoot(FILE *outf, Outchoices *od) {
  mprintf(outf, 0, NULL);
}

/* Printing part of the whatincluded line */
void plain_whatincprintstr(FILE *outf, Outchoices *od, char *s) {
  mprintf(outf, od->plainpagewidth, "%s", s);
}

/* Print a double to a nice number of decimal places */
/* plain_printdouble always uses mprintf */
void plain_printdouble(FILE *outf, Outchoices *od, double x) {
  unsigned int pagewidth;
  char decpt;
  unsigned int prec;
  double d;

  if (od == NULL) {
    pagewidth = 1;
    decpt = '.';
  }
  else {
    pagewidth = od->plainpagewidth;
    decpt = od->decpt;
  }

  /* first calculate how many decimal places we need */

  for (prec = 0, d = x - (double)((int)(x));
       d - (double)((int)(d + 0.000005)) > 0.00001; d *= 10)
    prec++;

  /* now print it */

  if (prec > 0)
    mprintf(outf, pagewidth, "%d%c%0*d", (int)x, decpt, prec,
	    (int)(d + EPSILON));
  else
    mprintf(outf, pagewidth, "%d", (int)(x + EPSILON));
}

/* Include a header file or footer file */
void plain_includefile(FILE *outf, Outchoices *od, char *name, char type) {
  FILE *inf;
  char buffer[BLOCKSIZE];
  size_t n;

  if ((inf = my_fopen(name, (type == 'h')?"header file":"footer file")) !=
      NULL) {
    if (type == 'f')
      plain_hrule(outf, od);
    while ((n = fread(buffer, 1, BLOCKSIZE, inf)))  /* single equals */
      fwrite((void *)buffer, 1, n, outf);
    if (type == 'h')
      plain_hrule(outf, od);
    (void)my_fclose(inf, name, (type == 'h')?"header file":"footer file");
  }
}

/* Filetype for RISC OS */
unsigned int plain_riscosfiletype(void) {
  return 0xfff;
}
