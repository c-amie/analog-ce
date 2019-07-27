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

/*** outhtml.c; HTML output ***/

#include "anlghea3.h"

/* Page width */
unsigned int html_pagewidth(Outchoices *od) {
  return od->htmlpagewidth;
}

/* The top of the output if we are in CGI mode */
void html_cgihead(FILE *outf, Outchoices *od) {
  fprintf(outf, "Content-Type: text/html; charset=%s\n\n",
	  od->lngstr[charset_]);
}

/* Stuff this output style needs in the page header */
void html_stylehead(FILE *outf, Outchoices *od) {
  fputs("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n", outf);
  fputs("<html>\n<head>\n", outf);
  fprintf(outf, "<meta http-equiv=\"Content-Type\" "
	  "content=\"text/html; charset=%s\">\n", od->lngstr[charset_]);
  if (od->norobots)
    fputs("<meta name=\"robots\" content=\"noindex,nofollow\">\n", outf);
  fprintf(outf, "<meta name=\"GENERATOR\" content=\"analog %s\">\n", VERSION);
  fputs("<title>", outf);
  htmlputs(outf, od, od->hostname, FROM_CFG);
  fprintf(outf, " %s", od->lngstr[webstatsfor_]);
  fputs("</title>\n", outf);
  if (!strcaseeq(od->stylesheet, "none")) {
    fputs("<link href=\"", outf);
    htmlputs(outf, od, od->stylesheet, IN_HREF);
    fputs("\" rel=\"stylesheet\">\n", outf);
  }
  fputs("</head>\n", outf);
  fputs("<body>\n", outf);
}

/* The title of the page, plus the user's HEADERFILE */
void html_pagetitle(FILE *outf, Outchoices *od) {
  fputs("<h1><a NAME=\"Top\">", outf);
  if (!strcaseeq(od->logo, "none")) {
    fputs("<IMG src=\"", outf);
    if (od->logo[0] != '/' && strstr(od->logo, "://") == NULL)
      htmlputs(outf, od, od->imagedir, IN_HREF);
    htmlputs(outf, od, od->logo, IN_HREF);
    if (STREQ(od->logo, "analogo"))
      fprintf(outf, ".%s", od->pngimages?"png":"gif");
    /* Above: '.' not EXTSEP even on RISC OS */
    fputs("\" alt=\"\"> ", outf);
  }
  if (strcaseeq(od->hosturl, "none")) {
    fprintf(outf, "%s</a> ", od->lngstr[webstatsfor_]);
    htmlputs(outf, od, od->hostname, FROM_CFG);
  }
  else {
    fprintf(outf, "%s</a> <a HREF=\"", od->lngstr[webstatsfor_]);
    htmlputs(outf, od, od->hosturl, IN_HREF);
    fputs("\">", outf);
    htmlputs(outf, od, od->hostname, FROM_CFG);
    fputs("</a>", outf);
  }
  fputs("</h1>\n\n", outf);

  if (!strcaseeq(od->headerfile, "none"))
    html_includefile(outf, od, od->headerfile, 'h');
}

/* Program start time, and logfile start and end times */
void html_timings(FILE *outf, Outchoices *od, Dateman *dman) {
  extern timecode_t starttimec;

  char **lngstr = od->lngstr;

  double t0;
  int t1, t2;

  if (od->runtime)
    fprintf(outf, "%s %s.\n<br>", lngstr[progstart_],
	    timesprintf(od, lngstr[datefmt2_], starttimec, UNSET));

  if (dman->firsttime <= dman->lasttime) {
    fprintf(outf, "%s %s ", lngstr[reqstart_],
	    timesprintf(od, lngstr[datefmt2_], dman->firsttime, UNSET));
    fprintf(outf, "%s %s", lngstr[to_],
	    timesprintf(od, lngstr[datefmt2_], dman->lasttime, UNSET));
    t0 = (dman->lasttime - dman->firsttime) / 1440.0 + 0.005;
    t1 = (int)t0;
    t2 = (int)(100 * (t0 - (double)t1));
    fprintf(outf, " (%d", t1);
    html_putch(outf, od->decpt);
    fprintf(outf, "%02d %s).\n", t2, lngstr[days_]);
  }
}

/* Finishing the top of the page */
void html_closehead(FILE *outf, Outchoices *od) {
  if (od->gotos == FEW)
    html_gotos(outf, od, -1);

  html_hrule(outf, od);
}

/* Starting the bottom of the page */
void html_pagebotstart(FILE *outf, Outchoices *od) {
}

/* The credit line at the bottom of the page */
void html_credit(FILE *outf, Outchoices *od) {
  fprintf(outf, "<i>%s <a HREF=\"%s\" rel=\"follow\">Analog CE %s</a>.\n", od->lngstr[credit_],
	  ANALOGURL, VNUMBER);
}

/* The program run time */
void html_runtime(FILE *outf, Outchoices *od, long secs) {
  char **lngstr = od->lngstr;

  fprintf(outf, "<br><b>%s:</b> ", lngstr[runtime_]);

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
void html_pagefoot(FILE *outf, Outchoices *od) {
  fputs("</i>\n", outf);
  if (od->gotos != FALSE)
    html_gotos(outf, od, -1);

  if (!strcaseeq(od->footerfile, "none"))
    html_includefile(outf, od, od->footerfile, 'f');

  fputs("<p><a href=\"http://validator.w3.org/\">\n", outf);
  fputs("<img src=\"", outf);
  htmlputs(outf, od, od->imagedir, IN_HREF);
  fprintf(outf, "html2.%s\"\n", od->pngimages?"png":"gif");
  /* Above: '.' not EXTSEP even on RISC OS */
  fputs("alt=\"HTML 2.0 Conformant!\"></a>\n", outf);
}

/* Footer material for this output style */
void html_stylefoot(FILE *outf, Outchoices *od) {
  fputs("</body>\n</html>\n", outf);
}

/* Report title */
void html_reporttitle(FILE *outf, Outchoices *od, choice rep) {
  extern char *anchorname[];
  extern unsigned int *rep2lng;

  fprintf(outf, "<h2><a NAME=\"%s\">%s</a></h2>\n", anchorname[rep],
	  od->lngstr[rep2lng[rep]]);
  if (od->gotos == TRUE)
    html_gotos(outf, od, rep);
}

/* Report footer */
void html_reportfooter(FILE *outf, Outchoices *od, choice rep) {
}

/* Report description */
void html_reportdesc(FILE *outf, Outchoices *od, choice rep) {
  fprintf(outf, "<p><em>%s</em>\n", od->descstr[rep]);
}

/* The time period spanned by the report */
void html_reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd,
		     timecode_t mind) {
  /* NB Can't combine next two lines because timesprintf uses static buffer. */
  fprintf(outf, "<p><em>%s %s ", od->lngstr[repspan_],
	  timesprintf(od, od->lngstr[datefmt2_], mind, UNSET));
  fprintf(outf, "%s %s.</em>\n", od->lngstr[to_],
	  timesprintf(od, od->lngstr[datefmt2_], maxd, UNSET));
}

/* General Summary header */
void html_gensumhead(FILE *outf, Outchoices *od) {
}

/* General Summary footer */
void html_gensumfoot(FILE *outf, Outchoices *od) {
}

/* Single General Summary line, long data */
void html_gensumline(FILE *outf, Outchoices *od, int namecode,
		     unsigned long x, unsigned long x7, logical isaverage) {

  /* If this is the first Gen Sum line, and there is no seven-day data, then
     start a new paragraph. Otherwise the new paragraph has already been
     started in html_lastseven(). */
  if (namecode == succreqs_ && x7 == (unsigned long)UNSET)
    fprintf(outf, "<p><b>%s%s</b> ", od->lngstr[namecode], od->lngstr[colon_]);
  else
    fprintf(outf, "<br><b>%s%s</b> ",  od->lngstr[namecode],
	    od->lngstr[colon_]);

  f3printf(outf, od, (double)x, 0, od->sepchar);

  if (x7 != (unsigned long)UNSET) {
    fputs(" (", outf);
    f3printf(outf, od, (double)x7, 0, od->sepchar);
    putc(')', outf);
  }
  putc('\n', outf);
}

/* Single General Summary line, bytes data */
void html_gensumlineb(FILE *outf, Outchoices *od, int namecode, double x,
		      double x7, logical isaverage) {
  char **lngstr = od->lngstr;

  unsigned int bm;
  char *c;

  fprintf(outf, "<br><b>%s%s</b> ",  lngstr[namecode], lngstr[colon_]);

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
void html_lastseven(FILE *outf, Outchoices *od, timecode_t last7to) {
  fputs("<p>", outf);
  fprintf(outf, "(%s %s %s).\n", od->lngstr[brackets_],
	  od->lngstr[sevendaysto_],
	  timesprintf(od, od->lngstr[datefmt1_], last7to, UNSET));
}

/* Start of a <pre> section */
void html_prestart(FILE *outf, Outchoices *od) {
  fputs("<pre><tt>", outf);
}

/* End of a <pre> section */
void html_preend(FILE *outf, Outchoices *od) {
  fputs("</tt></pre>\n", outf);
}

/* A horizontal rule */
void html_hrule(FILE *outf, Outchoices *od) {
  fputs("<hr>\n", outf);
}

/* An en dash */
char *html_endash(void) {
  return "-";
  /* Should be &ndash; but not all browsers implement &ndash;, and when
     they do it's usually just a regular dash. */
}

/* putc with special characters escaped */
void html_putch(FILE *outf, char c) {
  if (c == '<')
    fputs("&lt;", outf);
  else if (c == '>')
    fputs("&gt;", outf);
  else if (c == '&')
    fputs("&amp;", outf);
  else if (c == '"')
    fputs("&quot;", outf);
  else
    putc(c, outf);
}

/* strlen for HTML strings. Assume string contains no &'s except as markup. */
/* NB This may not work well for multibyte charsets, but it's hard to know
   whether an & is markup or just a byte of a multibyte character. Special
   cases for particular charsets are given below and selected in init.c. */
/* NOTE html_strlengthgth_utf8 and _jis in outhtml only. Other outputters just
   have _strlengthgth */
size_t html_strlength(const char *s) {
  const char *t;
  logical f;
  size_t i;

  for (t = s, f = TRUE, i = 0; *t != '\0'; t++) {
    if (*t == '&')
      f = FALSE;
    else if (*t == ';')
      f = TRUE;
    if (f)
      i++;
  }
  return(f?i:strlen(s));
  /* If !f, something went wrong (eg multibyte). Maybe the & wasn't markup. */
}

size_t html_strlength_utf8(const char *s) {
  /* This only knows about jp chars in the range 1110xxxx 10xxxxxx 10xxxxxx.
     Other languages using UTF-8 would need new code. */
  const unsigned char *t;
  size_t i;

  for (i = 0, t = (const unsigned char *)s; *t != '\0'; t++) {
    if ((*t & 0xf0) == 0xe0 && (*(t + 1) & 0xc0) == 0x80 &&
	(*(t + 2) & 0xc0) == 0x80) {
      t += 2;  /* plus 1 in loop */
      i += 2;
      /* three-character jp sequence = one jp char = length 2 wrt ASCII */
    }
    else
      i++;
  }
  return(i);
}

size_t html_strlength_jis(const char *s) {
  size_t i;

  for (i = 0; *s != '\0'; s++) {
    if (*s == 0x1B && (*(s + 1) == '$' || *(s + 1) == '(') && *(s + 2) == 'B')
      s += 2; /* plus 1 in loop */   /* ignore ESC $ B and ESC ( B switches */
    else
      i++;
  }
  return(i);  /* returns length in bytes, because one jp char = two bytes and
		 width of one jp char = width of two ASCII chars */
}

/* Allow month in dates? DO NOT enable for human-readable text because of
   i18n problems. */
logical html_allowmonth(void) {
  return FALSE;
}

/* Calculate column widths */
void html_calcwidths(Outchoices *od, choice rep, unsigned int width[],
		     unsigned int *bmult, unsigned int *bmult7, double *unit,
		     unsigned long maxr, unsigned long maxr7,
		     unsigned long maxp, unsigned long maxp7, double maxb,
		     double maxb7, unsigned long howmany) {
  calcwidths(od, rep, width, bmult, bmult7, unit, maxr, maxr7, maxp, maxp7,
	     maxb, maxb7, howmany);
}

/* "Each unit represents" line */
void html_declareunit(FILE *outf, Outchoices *od, char graphby, double unit,
		      unsigned int bmult) {
  char **lngstr = od->lngstr;

  char *s;

  fputs("<p>\n", outf);
  fprintf(outf, "%s (", lngstr[eachunit_]);
  if (ISLOWER(graphby))
    fprintf(outf, "<tt>%c</tt>", od->markchar);
  else {
    fprintf(outf, "<img src=\"");
    htmlputs(outf, od, od->imagedir, IN_HREF);
    fprintf(outf, "bar%c1.%s\" alt=\"%c\">", od->barstyle,
	    od->pngimages?"png":"gif", od->markchar);
    /* Above: '.' not EXTSEP even on RISC OS */
  }
  fprintf(outf, ") %s ", lngstr[represents_]);

  if (graphby == 'R' || graphby == 'r') {
    f3printf(outf, od, unit, 0, od->sepchar);
    if (unit == 1.)
      fprintf(outf, " %s.", lngstr[request_]);
    else
      fprintf(outf, " %s %s.", lngstr[requests_], lngstr[partof_]);
  }
  else if (graphby == 'P' || graphby == 'p') {
    f3printf(outf, od, unit, 0, od->sepchar);
    if (unit == 1.)
      fprintf(outf, " %s.", lngstr[pagereq_]);
    else
      fprintf(outf, " %s %s.", lngstr[pagereqs_], lngstr[partof_]);
  }
  else {
    if (bmult > 0) {
      html_printdouble(outf, od, unit);
      s = strchr(lngstr[xbytes_], '?');  /* checked in initialisation */
      *s = '\0';
      fprintf(outf, " %s%s%s %s.", lngstr[xbytes_],
	      lngstr[byteprefix_ + bmult], s + 1, lngstr[partof_]);
      *s = '?';
    }
    else {
      f3printf(outf, od, unit, 0, od->sepchar);
      fprintf(outf, " %s %s.", lngstr[bytes_], lngstr[partof_]);
    }
  }
  putc('\n', outf);
}

/* Start of column header line */
void html_colheadstart(FILE *outf, Outchoices *od, choice rep) {
}

/* Column header line: individual column */
void html_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
		     unsigned int width, char *colname, logical unterminated) {
  if (unterminated)
    fputs(colname, outf);
  else
    fprintf(outf, "%*s: ",
	    width + strlen(colname) - od->outputter->strlength(colname),
	    colname);
}

/* End of column header line */
void html_colheadend(FILE *outf, Outchoices *od, choice rep) {
  putc('\n', outf);
}

/* Start of column header underlining line */
void html_colheadustart(FILE *outf, Outchoices *od, choice rep) {
}

/* Underlining of one column header. */
/* If column is terminated, set width and leave name blank; and conversely. */
void html_colheadunderline(FILE *outf, Outchoices *od, choice rep, choice col,
			   unsigned int width, char *name) {
  if (width > 0) {
    matchlengthn(outf, od, (size_t)width, '-');
    fputs(": ", outf);
  }
  else
    matchlength(outf, od, name, '-');
}

/* End of column header underlining line */
void html_colheaduend(FILE *outf, Outchoices *od, choice rep) {
  putc('\n', outf);
}

/* Start of a table row */
void html_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
		   int level, char *name, char *datefmt, char *timefmt) {
}

/* Print level in hierarchy represented by this row */
void html_levelcell(FILE *outf, Outchoices *od, choice rep, int level) {
}

/* Name column */
void html_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
		   choice source, unsigned int width, logical name1st,
		   logical isfirst, logical rightalign, Alias *aliashead,
		   Include *linkhead, logical ispage, unsigned int spaces,
		   char *baseurl) {
  extern char *workspace;

  choice savemultibyte;
  logical linked;
  int i;

  if (name1st != isfirst)
    return;

  if (isfirst)
    matchlengthn(outf, od, width - od->outputter->strlength(name), ' ');
  savemultibyte = od->multibyte;
  if (rep == REP_SIZE || rep == REP_PROCTIME)
    /* Kludge: for these two reports, we know the texts are things like
       "< 1" and we want to convert > and < */
    od->multibyte = FALSE;

  strcpy(workspace, name);
  do_aliasx(workspace, aliashead);

  if (!isfirst) {
    if (rightalign)
      i = (int)width - (int)od->outputter->strlength(workspace) - (int)spaces;
    else
      i = (int)spaces;
    matchlengthn(outf, od, i, ' ');
  }

  linked = (linkhead != NULL && included(name, ispage, linkhead));
  if (linked) {
    /* We link to the unaliased name, because the OUTPUTALIAS is usually in
       the nature of an annotation. */
    /* If the config file has a defined anonymization service provider
	   [ANONYMIZERURL], prefix with the provided string */
	if (strcaseeq(od->anonymizerurl, "none")) {
      fputs("<a href=\"", outf);
	} else {
      fputs("<a href=\"", outf);
	  htmlputs(outf, od, od->anonymizerurl, IN_HREF);
	}
    if (baseurl != NULL)
      htmlputs(outf, od, baseurl, IN_HREF);
    html_escfprintf(outf, name);
	/* If set, add rel="nofollow" */
	if (od->linknofollow) {
      fputs("\" rel=\"nofollow\">", outf);
	} else {
      fputs("\">", outf);
	}
  }

  htmlputs(outf, od, workspace, source);

  if (linked)
    fputs("</a>", outf);

  if (isfirst)
    fputs(": ", outf);

  od->multibyte = savemultibyte;  /* restore multibyte */
}

/* Single cell, unsigned long argument */
void html_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
		 unsigned long x, unsigned int width) {
  f3printf(outf, od, (double)x, width, od->repsepchar);
  fputs(": ", outf);
}

/* Single cell, TRUSTED string argument */
void html_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
		  char *s, unsigned int width) {
  matchlengthn(outf, od, width - od->outputter->strlength(s), ' ');
  htmlputs(outf, od, s, TRUSTED);
  fputs(": ", outf);
}

/* Single cell, listing bytes */
void html_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
		    double b, double bmult, unsigned int width) {
  printbytes(outf, od, b, bmult, width, od->repsepchar);
  fputs(": ", outf);
}

/* Single cell, listing percentage */
void html_pccell(FILE *outf, Outchoices *od, choice rep, choice col, double n,
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
    html_putch(outf, od->decpt);
    fprintf(outf, "%02d%%", pc2);
  }
  fputs(": ", outf);
}

/* Single cell, index */
void html_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
		    long index, unsigned int width) {
  /* If index is 0 (i.e. sub-item), just print spaces */
  if (index <= 0)
    matchlengthn(outf, od, width, ' ');
  else
    f3printf(outf, od, (double)index, width, od->repsepchar);
  fputs(": ", outf);
}

/* End of a table row */
void html_rowend(FILE *outf, Outchoices *od, choice rep) {
  putc('\n', outf);
}

/* Blank line in time reports */
void html_blankline(FILE *outf, Outchoices *od, choice *cols) {
  putc('\n', outf);
}

/* Barchart in time reports */
void html_barchart(FILE *outf, Outchoices *od, int y, char graphby) {
  int i, j;
  logical first = TRUE;

  if (ISLOWER(graphby)) {
    for (i = 0; i < y; i++)
      html_putch(outf, od->markchar);
    return;
  }

  for (j = 32; j >= 1; j /= 2) {
    while (y >= j) {
      fputs("<img src=\"", outf);
      htmlputs(outf, od, od->imagedir, IN_HREF);
      fprintf(outf, "bar%c%d.%s\" alt=\"", od->barstyle, j,
	      od->pngimages?"png":"gif"); /* '.' not EXTSEP even on RISC OS */
      if (first) {
	for (i = 0; i < y; i++)
	  html_putch(outf, od->markchar);
	first = FALSE;
      }
      fputs("\">", outf);
      y -= j;
    }
  }
}

/* "Busiest time period" line */
void html_busyprintf(FILE *outf, Outchoices *od, choice rep, char *datefmt,
		     unsigned long reqs, unsigned long pages, double bys,
		     datecode_t date, unsigned int hr, unsigned int min,
		     datecode_t newdate, unsigned int newhr,
		     unsigned int newmin, char graphby) {
  extern unsigned int *rep2busystr;

  char **lngstr = od->lngstr;
  char sepchar = od->sepchar;

  unsigned int bmult;
  char *s;

  fprintf(outf, "%s %s (", lngstr[rep2busystr[rep]],
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
void html_notlistedstr(FILE *outf, Outchoices *od, choice rep,
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
void html_whatincluded(FILE *outf, Outchoices *od, choice rep,
		       unsigned long n, Dateman *dman) {
  whatincluded(outf, od, rep, n, dman);
}

/* Spacing at the start of the whatincluded line */
void html_whatinchead(FILE *outf, Outchoices *od) {
  fputs("<p>\n", outf);
}

/* Finishing the whatincluded line */
void html_whatincfoot(FILE *outf, Outchoices *od) {
  putc('\n', outf);
}

/* Printing part of the whatincluded line */
void html_whatincprintstr(FILE *outf, Outchoices *od, char *s) {
  htmlputs(outf, od, s, TRUSTED);
}

/* Print a double to a nice number of decimal places */
void html_printdouble(FILE *outf, Outchoices *od, double x) {
  unsigned int prec;
  double d;

  /* first calculate how many decimal places we need */

  for (prec = 0, d = x - (double)((int)(x));
       d - (double)((int)(d + 0.000005)) > 0.00001; d *= 10)
    prec++;

  /* now print it */

  if (prec > 0) {
    fprintf(outf, "%d", (int)x);
    html_putch(outf, od->decpt);
    fprintf(outf, "%0*d", prec, (int)(d + EPSILON));
  }
  else
    fprintf(outf, "%d", (int)(x + EPSILON));
}

/* Include a header file or footer file */
void html_includefile(FILE *outf, Outchoices *od, char *name, char type) {
  FILE *inf;
  char buffer[BLOCKSIZE];
  size_t n;

  if ((inf = my_fopen(name, (type == 'h')?"header file":"footer file")) !=
      NULL) {
    od->html = FALSE;
    html_hrule(outf, od);
    while ((n = fread(buffer, 1, BLOCKSIZE, inf)))  /* single equals */
      fwrite((void *)buffer, 1, n, outf);
    if (type == 'h')
      html_hrule(outf, od);
    (void)my_fclose(inf, name, (type == 'h')?"header file":"footer file");
  }
}

/* Filetype for RISC OS */
unsigned int html_riscosfiletype(void) {
  return 0xfaf;
}

/* ======================================================================= */
/* Supporting functions for HTML */

/* Print "goto"s. Assume we've checked that we want gotos here. */
void html_gotos(FILE *outf, Outchoices *od, choice rep) {
  extern unsigned int *rep2lng;
  extern char *anchorname[];

  choice *reporder = od->reporder;
  char **lngstr = od->lngstr;
  int i;

  fprintf(outf, "<p>(<b>%s</b>", lngstr[goto_]);
  fprintf(outf, "%s <a HREF=\"#Top\">%s</a>", lngstr[colon_], lngstr[top_]);
  for (i = 0; reporder[i] != -1; i++) {
    if (reporder[i] == rep)
      fprintf(outf, "%s %s", lngstr[colon_], lngstr[rep2lng[reporder[i]]]);
    else if (od->repq[reporder[i]])
      fprintf(outf, "%s <a HREF=\"#%s\">%s</a>", lngstr[colon_],
	      anchorname[reporder[i]], lngstr[rep2lng[reporder[i]]]);
  }
  fputs(")\n", outf);
}

/* Escape names for use in hyperlinks. As with htmlputs(), don't try and
   print character by character. */
void html_escfprintf(FILE *outf, char *name) {
#ifdef EBCDIC
  extern unsigned char os_toascii[];
#endif
  char w1[64];
  char *w = w1;
  int len = 0;

  for ( ; *name != '\0'; name++) {
    if (ISALNUM(*name) || *name == '/' || *name == '.' || *name == ':' ||
	*name == '-' || *name == '~' || *name == '_' || *name == '?' ||
	*name == '%' || *name == '=' || *name == '+' ||
	*name == ';' ||	*name == '@' || *name == '$' || *name == ',') {
      /* All reserved and some unreserved chars from RFC 2396 Sec 2. */
      /* Reserved chars are not escaped because if they are in the logfile they
	 must have their special meanings (path delimiters etc.), and escaping
	 them would change the semantics of the URL. */
      PUTc(w, *name);
      len += 1;
    }
    else if (*name == '&') {
      PUTs(w, "&amp;", 0);
      len += 5;
    }
    else {
#ifdef EBCDIC
      sprintf(w, "%%%.2X", os_toascii[*name]);
#else
      sprintf(w, "%%%.2X", (unsigned char)(*name));
#endif
      w += 3;
      len += 3;
    }
    if (len > 58) {
      *w = '\0';
      fputs(w1, outf);
      w = w1;
      len = 0;
    }
  }
  *w = '\0';
  fputs(w1, outf);
}

/* htmlputs(): print a string with an appropriate amount of HTML encoding.
   Much quicker than using html_putch(). */

/** What to convert has SECURITY IMPLICATIONS. An attacker must not be allowed
 ** to insert abitrary data in the output.
 **
 ** So data is categorised according to its source, via an enum in anlghea3.h.
 ** In the following descriptions of the security levels, "convert" means
 ** converting e.g. < to &lt; and "escape" means using \< to prevent this
 ** happening. "unprintable" means characters set as unprintable in init.c:
 ** note that this is only known unprintable characters 0x00 - 0x1F, 0x7F,
 ** and in ISO-8859-* also 0x80-0x9F.
 **
 ** 1) AS_IS: Completely trusted string. Output as-is.
 ** 2) TRUSTED: E.g. a string from a language file. Also completely trusted. In
 **    single-byte mode, convert characters (for convenience not security), but
 **    allow any of the special characters to be escaped, even \< .
 **    In multibyte mode, output the string as-is.
 ** 3) FROM_CFG: An item read in from configuration. Unless we're in CGI mode,
 **    treat as case 2. In CGI mode, the input could have come from the form,
 **    so be more cautious to avoid cross-site scripting attacks. Specifically,
 **    convert all characters, allow only \& and \\ escapes, and use '?' in
 **    place of unprintable characters.
 ** 4) UNTRUSTED: E.g. data from the logfile. Do all conversions, don't allow
 **    any escapes, and use '?' in place of all unprintable characters.
 ** 5) IN_HREF: Special case for data from the config file which is being put
 **    inside an <a href=""> or <img src="">. As 3), but use %nm in place of
 **    unprintable characters. (NB data from the logfile which is put inside
 **    an href uses escfprintf() instead of this function.)
 **/
void htmlputs(FILE *outf, Outchoices *od, char *s, choice source) {
#ifdef EBCDIC
  extern unsigned char os_toascii[];
#endif
  extern logical cgi;
  extern logical unprintable[256];

  char w1[64];
  char *c;
  char *w = w1;
  int len = 0;

  if (source == FROM_CFG && !cgi)
    source = TRUSTED;

  if (source == TRUSTED && od->multibyte)
    source = AS_IS;

  if (source == AS_IS) {
    fputs(s, outf);
    return;
  }

  for (c = s; *c != '\0'; c++) {
    if (*c == '<') {
      PUTs(w, "&lt;", 0);
      len += 4;
    }
    else if (*c == '>') {
      PUTs(w, "&gt;", 0);
      len += 4;
    }
    else if (*c == '&') {
      PUTs(w, "&amp;", 0);
      len += 5;
    }
    else if (*c == '"') {
      PUTs(w, "&quot;", 0);
      len += 6;
    }
    else if (*c == '\\' &&  /* escape these chars in these circumstances: */
	     ((source == TRUSTED && (*(c + 1) == '<' || *(c + 1) == '>' ||
				     *(c + 1) == '&' || *(c + 1) == '"' ||
				     *(c + 1) == '\\'))
	      || (source == FROM_CFG && (*(c + 1) == '&' ||
					 *(c + 1) == '\\')))) {
      od->html = FALSE;
      PUTc(w, *(++c));
      len += 1;
    }
    else if (unprintable[(unsigned char)(*c)] && source != TRUSTED) {
      /* unprintable chars */
      if (source == IN_HREF) {
#ifdef EBCDIC
	sprintf(w, "%%%.2X", os_toascii[*c]);
#else
	sprintf(w, "%%%.2X", (unsigned char)(*c));
#endif
	w += 3;
	len += 3;
      }
      else {  /* source == FROM_CFG or UNTRUSTED */
	PUTc(w, '?');
	len += 1;
      }
    }
    else {  /* output non-special characters as-is */
      PUTc(w, *c);
      len += 1;
    }
    if (len > 56) {
      *w = '\0';
      fputs(w1, outf);
      w = w1;
      len = 0;
    }
  }
  *w = '\0';
  fputs(w1, outf);
}

/* ======================================================================= */
/* Pie charts */
/* Shared by HTML and XHTML */

#ifndef NOGRAPHICS
/* First some #defines and globals */
#define XSIZE (600)                 /* Size of whole graphic */
#define YSIZE (270)
#define SHORTXSIZE (2 * (XCENTRE))  /* Size if no text on picture */
#define SHORTYSIZE (2 * (YCENTRE))
#define XCENTRE (125)               /* Centre of pie */
#define YCENTRE (125)
#define DIAMETER (200)              /* Diameter of pie */
#define BORDER (4)                  /* Size of border */
#define BOXESLEFT (XCENTRE + DIAMETER / 2 + 25)    /* The coloured boxes */ 
#define BOXESTOP (YCENTRE - DIAMETER / 2 + 16)
#define BOXESSIZE (10)
#define TEXTLEFT ((BOXESLEFT) + 2 * (BOXESSIZE))   /* The labels */
#define TEXTOFFSET (-1)
#define TEXTGAP (16)     /* Vertical period between successive boxes/labels */
#define CAPTIONLEFT (XCENTRE - DIAMETER / 2)       /* Bottom caption */
#define CAPTIONTOP (YCENTRE + DIAMETER / 2 + 12)
#define NO_COLOURS (10)  /* Number of text strings, excluding "Other" */
#define MAXCHARS (54)    /* Max length of a label, INCLUDING \0. */
#define TWOPI (6.283185)
#define MINANGLE (0.01)  /* Min fraction of circle we are prepared to plot */
#define PIE_EPSILON (0.0001)
gdImagePtr im;
gdFontPtr font;
logical normalchart;
int white, black, grey, lightgrey, colours[NO_COLOURS], col, boxesy;
double totangle;

FILE *piechart_init(char *filename) {
  FILE *pieoutf;
  int xsize, ysize, b1, b2;

  if ((pieoutf = FOPENWB(filename)) == NULL) {
    warn('F', TRUE, "Failed to open pie chart file %s for writing: "
	 "ignoring it", filename);
    return(pieoutf);
  }
  debug('F', "Opening %s as pie chart file", filename);
#ifdef RISCOS
  _swix(OS_File, _INR(0,2), 18, filename, 0xb60);  /* set PNG filetype */
#endif

  xsize = normalchart?XSIZE:SHORTXSIZE;
  ysize = normalchart?YSIZE:SHORTYSIZE;
  im = gdImageCreate(xsize, ysize);
  /* The first colour allocated in a new image is the background colour. */
  white = gdImageColorAllocate(im, 255, 255, 255);           /* white */
  black = gdImageColorAllocate(im, 0, 0, 0);                 /* black */
  grey = gdImageColorAllocate(im, 128, 128, 128);            /* grey */
  lightgrey = gdImageColorAllocate(im, 217, 217, 217);       /* light grey */
  col = 0;
  /* Wedge colours. If these change, so must images/sq*. */
  colours[col++] = gdImageColorAllocate(im, 255, 0, 0);      /* red */
  colours[col++] = gdImageColorAllocate(im, 0, 0, 255);      /* mid blue */
  colours[col++] = gdImageColorAllocate(im, 0, 128, 0);      /* green */
  colours[col++] = gdImageColorAllocate(im, 255, 128, 0);    /* orange */
  colours[col++] = gdImageColorAllocate(im, 0, 0, 128);      /* navy blue */
  colours[col++] = gdImageColorAllocate(im, 0, 255, 0);      /* pale green */
  colours[col++] = gdImageColorAllocate(im, 255, 128, 128);  /* pink */
  colours[col++] = gdImageColorAllocate(im, 0, 255, 255);    /* cyan */
  colours[col++] = gdImageColorAllocate(im, 128, 0, 128);    /* purple */
  colours[col++] = gdImageColorAllocate(im, 255, 255, 0);    /* yellow */
  col = 0;
  totangle = 0.75;  /* starting at the top */
  boxesy = BOXESTOP;
  b1 = xsize - 1 - BORDER;
  b2 = ysize - 1 - BORDER;
  /* Plot outline of pie, and border of image */
  gdImageArc(im, XCENTRE, YCENTRE, DIAMETER + 2, DIAMETER + 2, 0, 360, black);
  gdImageRectangle(im, BORDER, BORDER, b1, b2, black);
  gdImageLine(im, xsize - 1, 0, b1, BORDER, black);
  gdImageLine(im, 0, ysize - 1, BORDER, b2, black);
  gdImageFill(im, 0, 0, lightgrey);
  gdImageFill(im, xsize - 1, ysize - 1, grey);
  gdImageLine(im, 0, 0, BORDER, BORDER, black);
  gdImageLine(im, xsize - 1, ysize - 1, b1, b2, black);
  return(pieoutf);
}

void findwedges(Wedge wedge[NO_COLOURS], choice rep, Hashindex *items,
		choice chartby, Strlist *expandlist, unsigned int level,
		Strlist *partname, unsigned long tot, double totb,
		double totb7) {
  /* Calculate which wedges we actually want, i.e. the ten with the biggest
     angles. But we also preserve the sort order of the "items" list. (Be
     careful between > and >= so as to use that order for breaking ties).
     Construction of name same as in printtree(). */
  static double smallestangle;
  static int smallestwedge;

  char *name;
  double angle;
  Strlist *pn, s;
  size_t need = (size_t)level + 3;
  Hashindex *p;
  int i;

  if (level == 0) {  /* not recursing: initialise wedges to 0 */
    for (i = 0; i < NO_COLOURS; i++) {
      wedge[i].angle = 0.0;
      wedge[i].name = NULL;
    }
    smallestangle = 0.0;
    smallestwedge = NO_COLOURS - 1;
  }

  for (pn = partname; pn != NULL; TO_NEXT(pn))
    need += strlen(pn->name);
  for (p = items; p != NULL; TO_NEXT(p)) {
    name = maketreename(partname, p, &pn, &s, need, rep, TRUE);
    if (incstrlist(name, expandlist) && p->other != NULL &&
	((Hashtable *)(p->other))->head[0] != NULL)
      /* then find wedges in lower level of tree instead. ->head[0] != NULL
	 must come after p->other != NULL; o/wise it may not be a treerep */
      findwedges(wedge, rep, ((Hashtable *)(p->other))->head[0], chartby,
		 expandlist, level + 1, pn, tot, totb, totb7);
    else {
      if (chartby == BYTES)
	angle = p->own->bytes / totb;
      else if (chartby == BYTES7)
	angle = p->own->bytes7 / totb7;
      else
	angle = ((double)(p->own->data[chartby])) / ((double)tot);
      if (angle > smallestangle) {/* remove smallest, move along, put p last */
	/* We probably don't do this very often so we don't bother with keeping
	   hold of the memory and reusing it later. */
	free(wedge[smallestwedge].name);
	for (i = smallestwedge; i < NO_COLOURS - 1; i++) {
	  wedge[i].name = wedge[i + 1].name;
	  wedge[i].angle = wedge[i + 1].angle;
	}
	COPYSTR(wedge[NO_COLOURS - 1].name, name);
	/* malloc's necessary space. Needed because next call to maketreename()
	   will overwrite name. */
	wedge[NO_COLOURS - 1].angle = angle;
	smallestangle = wedge[0].angle;  /* Recalculate smallest */
	smallestwedge = 0;
	for (i = 1; i < NO_COLOURS; i++) {
	  if (wedge[i].angle <= smallestangle) {
	    smallestangle = wedge[i].angle;
	    smallestwedge = i;
	  }
	}
      }
    }
  }
}

void piechart_caption(FILE *outf, choice rep, choice chartby, char **lngstr) {
  extern choice *rep2reqs, *rep2reqs7;
  extern unsigned int *method2sort;

  static char *caption = NULL;
  static size_t len = 0;

  choice requests = rep2reqs[G(rep)];
  choice requests7 = rep2reqs7[G(rep)];

  ENSURE_LEN(caption, len, strlen(lngstr[chartby_]) +
	     strlen(lngstr[method2sort[requests]]) +
	     strlen(lngstr[method2sort[requests7]]) +
	     strlen(lngstr[method2sort[chartby]]) + 3);
  /* More than we need, but that's OK. */
  strcpy(caption, lngstr[chartby_]);
  strcat(caption, " ");
  if (chartby == REQUESTS)
    strcat(caption, lngstr[method2sort[requests]]);
  else if (chartby == REQUESTS7)
    strcat(caption, lngstr[method2sort[requests7]]);
  else
    strcat(caption, lngstr[method2sort[chartby]]);
  strcat(caption, ".");
  if (normalchart) {
#ifdef EBCDIC
    (void)strtoascii(caption);
#endif
    gdImageString(im, font, CAPTIONLEFT, CAPTIONTOP, (unsigned char *)caption,
		  black);
  }
  else
    fprintf(outf, "<p><em>%s</em></p>\n", caption);
}

int piechart_wedge(FILE *outf, Outchoices *od, double angle, char *s) {
  /* The angle is expressed between 0 and 1. Returns col if wedge was big
     enough to be coloured in, NO_COLOURS for grey, else -1. */
  double x, y, newangle, medangle;
  int colour = black, rc = -1;
  char t[MAXCHARS];
  size_t len;

  if (angle < 0) {
    angle = 1.75 - totangle;  /* rest of the circle because started at 0.75 */
    colour = grey;
  }
  else if (col >= NO_COLOURS)
    angle = 0;  /* As a signal not to make a wedge. Can this happen? */
  else if (angle >= MINANGLE)
    colour = colours[col];

  if (angle >= MINANGLE || (colour == grey && angle > EPSILON)) {
    /* plot one edge of wedge */
    x = (double)XCENTRE + (double)DIAMETER * cos(totangle * TWOPI) / 2. +
      PIE_EPSILON;
    y = (double)YCENTRE + (double)DIAMETER * sin(totangle * TWOPI) / 2. +
      PIE_EPSILON;
    gdImageLine(im, XCENTRE, YCENTRE, (int)x, (int)y, black);

    /* plot other edge of wedge */
    newangle = totangle + angle;
    x = (double)XCENTRE + (double)DIAMETER * cos(newangle * TWOPI) / 2. +
      PIE_EPSILON;
    y = (double)YCENTRE + (double)DIAMETER * sin(newangle * TWOPI) / 2. +
      PIE_EPSILON;
    gdImageLine(im, XCENTRE, YCENTRE, (int)x, (int)y, black);

    /* Fill wedge */
    medangle = totangle + angle / 2.;
    x = (double)XCENTRE + (double)DIAMETER * cos(medangle * TWOPI) / 2.5;
    y = (double)YCENTRE + (double)DIAMETER * sin(medangle * TWOPI) / 2.5;
    if (gdImageGetPixel(im, (int)x, (int)y) == white) {  /* room to colour */
      gdImageFill(im, (int)x, (int)y, colour);
      /* Make label for wedge. If !normalchart, this is done in piechart_key()
	 below instead. (See long comment near bottom of piechart().) */
      if (normalchart) {
	gdImageFilledRectangle(im, BOXESLEFT, boxesy, BOXESLEFT + BOXESSIZE,
			       boxesy + BOXESSIZE, colour);
	if ((len = strlen(s)) <= MAXCHARS - 1)
	  strcpy(t, s);
	else {
	  strncpy(t, s, (MAXCHARS - 3) / 2);
	  strcpy(t + (MAXCHARS - 3) / 2, "...");
	  strncpy(t + (MAXCHARS + 3) / 2, s + len - (MAXCHARS - 4) / 2,
		  (MAXCHARS - 4) / 2);
	  t[MAXCHARS - 1] = '\0';
	}
#ifdef EBCDIC
	strtoascii(t);
#endif
	gdImageString(im, font, TEXTLEFT, boxesy + TEXTOFFSET,
		      (unsigned char *)t, black);
	boxesy += TEXTGAP;
      }
      rc = (colour == grey)?NO_COLOURS:col;
    }   /* end if (room to colour) */
    totangle = newangle;
    col++;
  }
  return(rc);
}

void piechart_key(FILE *outf, Outchoices *od, int col, char *name,
		  char *extension, Alias *aliashead) {
  extern char *workspace;

  /* Only called if !normalchart and wedge was included on chart */
  if (od->outstyle == HTML)
    fputs("<br><img src=\"", outf);
  else /* od->outstyle == XHTML */
    fputs("<br /><img src=\"", outf);
  htmlputs(outf, od, od->imagedir, IN_HREF);
  if (col == NO_COLOURS)
    fprintf(outf, "sqg.%s", extension);
  else         /* Above and below: '.' not EXTSEP even on RISC OS */
    fprintf(outf, "sq%d.%s", col, extension);
  if (od->outstyle == HTML)
    fputs("\" alt=\"\"> ", outf);
  else /* od->outstyle == XHTML */
    fputs("\" alt=\"\" /> ", outf);
  strcpy(workspace, name);
  do_aliasx(workspace, aliashead);
  htmlputs(outf, od, workspace, UNTRUSTED);
  fputs("\n", outf);
}

void piechart_write(FILE *pieoutf, char *filename, logical jpegcharts) {
#ifdef HAVE_GD
  if (jpegcharts)
    gdImageJpeg(im, pieoutf, 100);
  else
#endif
  gdImagePng(im, pieoutf);
  debug('F', "Closing %s", filename);
  fclose(pieoutf);
  gdImageDestroy(im);
}

void piechart_cleanup(Wedge wedge[NO_COLOURS]) {
  int i;

  /* free the wedge names allocated in findwedges() */
  for (i = 0; i < NO_COLOURS; i++)
    free(wedge[i].name);
}

void piechart(FILE *outf, Outchoices *od, choice rep, Hashindex *items,
	      choice requests, choice requests7, choice pages, choice pages7,
	      unsigned long totr, unsigned long totr7, unsigned long totp,
	      unsigned long totp7, double totb, double totb7) {
  extern unsigned int *rep2lng;
  extern char *repname[];
  extern char *workspace;
  extern char *anchorname[];
  static char *filename = NULL;

  char **lngstr = od->lngstr;
  choice chartby = od->chartby[G(rep)];
  Strlist *expandlist = od->expandhead[G(rep)];
  char gender = lngstr[rep2lng[rep] + 3][0];
  char *extension = (od->jpegcharts)?"jpg":"png";

  Wedge wedge[NO_COLOURS];
  FILE *pieoutf;
  int key[NO_COLOURS], keyg;
  double largestangle;
  unsigned long tot = 1;
  char *otherstr;
  int i;

  /* Sort out what the chartby really means */
  if (chartby == CHART_NONE)
    return;  /* We didn't want a pie chart after all */
  if (chartby == REQUESTS) {
    chartby = requests;
    tot = totr;
  }
  else if (chartby == REQUESTS7) {
    chartby = requests7;
    tot = totr7;
  }
  else if (chartby == PAGES) {
    chartby = pages;
    tot = totp;
  }
  else if (chartby == PAGES7) {
    chartby = pages7;
    tot = totp7;
  }
  if (tot == 0 || (chartby == BYTES && totb < 0.5) ||
      (chartby == BYTES7 && totb7 < 0.5)) {
    warn('R', TRUE, "In %s, turning off empty pie chart", repname[rep]);
    return;
  }

  /* Calculate which wedges to include */
  findwedges(wedge, rep, items, chartby, expandlist, 0, NULL, tot, totb,
	     totb7);

  /* Check whether we still want a chart */
  largestangle = wedge[0].angle;
  for (i = 1; i < NO_COLOURS; i++)
    largestangle = MAX(wedge[i].angle, largestangle);
  if (largestangle >= 1 - EPSILON) {
    warn('R', TRUE, "In %s, turning off pie chart of only one wedge",
	 repname[rep]);
    return;
  }
  if (largestangle == 0.) {
    warn('R', TRUE, "In %s, turning off pie chart with no wedges",
	 repname[rep]);
    return;
  }
  if (largestangle < MINANGLE) {
    warn('R', TRUE, "In %s, turning off pie chart because no wedge "
	 "large enough", repname[rep]);
    return;
  }

  /* font and normalchart are the same for every chart, but calculating them
     here allows us to keep the variables only in this file */
  normalchart = TRUE;
  if (strcaseeq(lngstr[charset_], "ISO-8859-2"))
    font = gdFontSmall;
  else {
    font = gdFontFixed;
    if (!strcaseeq(lngstr[charset_], "ISO-8859-1") &&
	!strcaseeq(lngstr[charset_], "US-ASCII"))
      normalchart = FALSE;
  }

  if (filename == NULL)
    filename = (char *)xmalloc(strlen(od->localchartdir) + 13);
  /* max poss length = localchartdir + anchorname ( <= 8 ) + ".png\0" */
  sprintf(filename, "%s%s%c%s", od->localchartdir, anchorname[rep], EXTSEP,
	  extension);
  if ((pieoutf = piechart_init(filename)) == NULL)
    return;  /* Warning message is given in piechart_init() */

  /* Now we can finally get round to plotting the chart */
  for (i = 0; i < NO_COLOURS; i++) {
    key[i] = -1;
    if (wedge[i].name != NULL) {
      strcpy(workspace, wedge[i].name);
      do_aliasx(workspace, od->aliashead[G(rep)]);
      key[i] = piechart_wedge(outf, od, wedge[i].angle, workspace);
      /* retain i -> colour mapping for calling piechart_key() below */
    }
  }
  if (normalchart)
    piechart_caption(outf, rep, od->chartby[G(rep)], lngstr);

  /* Plot the catch-all wedge and close the file */
  if (gender == 'm')
    otherstr = lngstr[otherm_];
  else if (gender == 'f')
    otherstr = lngstr[otherf_];
  else
    otherstr = lngstr[othern_];
  keyg = piechart_wedge(outf, od, -1., otherstr);
  piechart_write(pieoutf, filename, od->jpegcharts);

  /* Now the text on the page. In CGI mode, this must be done _after_ the image
     is closed, or the browser may fail to find the image. This is why printing
     the caption and key must be done twice; above here if normalchart, below
     here otherwise. */
  if (od->outstyle == HTML)
    fprintf(outf, "<p><img src=\"%s%s.%s\" alt=\"\">\n", od->chartdir,
	    anchorname[rep], extension);   /* '.' not EXTSEP even on RISC OS */
  else /* od->outstyle == XHTML */
    fprintf(outf, "<p><img src=\"%s%s.%s\" alt=\"\" /></p>\n", od->chartdir,
	    anchorname[rep], extension);

  if (!normalchart) {
    piechart_caption(outf, rep, od->chartby[G(rep)], lngstr);
    for (i = 0; i < NO_COLOURS; i++) {
      if (key[i] != -1)
	piechart_key(outf, od, key[i], wedge[i].name, extension,
		     od->aliashead[G(rep)]);
    }
    if (keyg != -1)
      piechart_key(outf, od, keyg, otherstr, extension, NULL);
  }

  piechart_cleanup(wedge);
}
#endif  /* NOGRAPHICS */
