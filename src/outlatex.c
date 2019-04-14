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

/*** outlatex.c; LaTeX output ***/

#include "anlghea3.h"

/* Supporting global variable for LaTeX */
char verbchar;

/* Page width */
unsigned int latex_pagewidth(Outchoices *od) {
  return od->latexpagewidth;
}

/* The top of the output if we are in CGI mode */
void latex_cgihead(FILE *outf, Outchoices *od) {
  fprintf(outf, "Content-Type: text/plain\n\n");
}

/* Stuff this output style needs in the page header */
void latex_stylehead(FILE *outf, Outchoices *od) {
  fputs("\\documentclass{article}\n", outf);
  if (!strcaseeq(od->lngstr[charset_], "US-ASCII")) {
    /* Charset US-ASCII or ISO-8859-[12] enforced in init.c */
    fprintf(outf, "\\usepackage[latin%c]{inputenc}\n",
	    od->lngstr[charset_][9]);
    fputs("\\usepackage[T1]{fontenc}\n", outf);
  }
  if (od->pdflatex)
    fputs("\\usepackage[pdftex]{color}\n", outf);
  else
    fputs("\\usepackage[dvips]{color}\n", outf);
  fputs("\\definecolor{barcolour}{rgb}{0.75,0.2,0.2}\n", outf);
  fputs("\\newsavebox{\\ttchar}\n", outf);
  fputs("\\sbox{\\ttchar}{\\mbox{\\tt -}}\n", outf);
  fputs("\\newlength{\\ttwidth}\n", outf);
  fputs("\\setlength{\\ttwidth}{\\wd\\ttchar}\n", outf);
  fputs("\\newcommand{\\barchart}[1]{{\\tt\\color{barcolour}\\rule[0.2ex]{#1\\ttwidth}{0.5ex}}}\n", outf);
  fputs("\\makeatletter\n", outf);
  fputs("\\def\\@maketitle{\\begin{center}{\\Large\\bf\\@title}\\end{center}}\n", outf);
  fputs("\\makeatother\n", outf);
  fputs("\\pagestyle{empty}\n", outf);
  fputs("\\setlength{\\parindent}{0pt}\n\n", outf);
  fputs("\\begin{document}\n", outf);
}

/* The title of the page, plus the user's HEADERFILE */
void latex_pagetitle(FILE *outf, Outchoices *od) {
  fprintf(outf, "\\title{%s ", od->lngstr[webstatsfor_]);
  latexputs(outf, od->hostname);
  fputs("}\n\\maketitle\\thispagestyle{empty}\n", outf);

  if (!strcaseeq(od->headerfile, "none"))
    latex_includefile(outf, od, od->headerfile, 'h');
}

/* Program start time, and logfile start and end times */
void latex_timings(FILE *outf, Outchoices *od, Dateman *dman) {
  extern timecode_t starttimec;

  char **lngstr = od->lngstr;

  double t0;
  int t1, t2;

  if (od->runtime)
    fprintf(outf, "%s %s.\n\n", lngstr[progstart_],
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
    latex_putch(outf, od->decpt);
    fprintf(outf, "%02d %s).\n", t2, lngstr[days_]);
  }
}

/* Finishing the top of the page */
void latex_closehead(FILE *outf, Outchoices *od) {
  latex_hrule(outf, od);
}

/* Starting the bottom of the page */
void latex_pagebotstart(FILE *outf, Outchoices *od) {
}

/* The credit line at the bottom of the page */
void latex_credit(FILE *outf, Outchoices *od) {
  fprintf(outf, "\\smallskip\n%s analog %s.\n", od->lngstr[credit_], VNUMBER);
}

/* The program run time */
void latex_runtime(FILE *outf, Outchoices *od, long secs) {
  char **lngstr = od->lngstr;

  fprintf(outf, "\n{\\bf %s:} ", lngstr[runtime_]);

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
void latex_pagefoot(FILE *outf, Outchoices *od) {
  if (!strcaseeq(od->footerfile, "none"))
    latex_includefile(outf, od, od->footerfile, 'f');
}

/* Footer material for this output style */
void latex_stylefoot(FILE *outf, Outchoices *od) {
  fputs("\\end{document}\n", outf);
}

/* Report title */
void latex_reporttitle(FILE *outf, Outchoices *od, choice rep) {
  extern unsigned int *rep2lng;

  fprintf(outf, "\\section*{%s}\n", od->lngstr[rep2lng[rep]]);
}

/* Report footer */
void latex_reportfooter(FILE *outf, Outchoices *od, choice rep) {
}

/* Report description */
void latex_reportdesc(FILE *outf, Outchoices *od, choice rep) {
  fprintf(outf, "{\\em %s}\n\n", od->descstr[rep]);
  if (rep != REP_SIZE && rep != REP_PROCTIME)
    fprintf(outf, "\\smallskip\n");
  /* These two reports probably have no further header text.
     If this is wrong, it's corrected in latex_reportspan() later. */
}

/* The time period spanned by the report */
void latex_reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd,
		      timecode_t mind) {
  if ((rep == REP_SIZE || rep == REP_PROCTIME) && od->descriptions &&
      od->descstr[rep] != NULL)
    fputs("\\smallskip\n", outf);
  /* We were wrong when we assumed in latex_reportdesc() that these reports
     had no further header text. So correct for it now. */

  /* NB Can't combine next two lines because timesprintf uses static buffer. */
  fprintf(outf, "{\\em %s %s ", od->lngstr[repspan_],
	  timesprintf(od, od->lngstr[datefmt2_], mind, UNSET));
  fprintf(outf, "%s %s.}\n\n", od->lngstr[to_],
	  timesprintf(od, od->lngstr[datefmt2_], maxd, UNSET));

  if (rep != REP_SIZE && rep != REP_PROCTIME)
    fprintf(outf, "\\smallskip\n");
  /* This time these reports really do have no further header text. */
}

/* General Summary header */
void latex_gensumhead(FILE *outf, Outchoices *od) {
  fputs("\\begin{obeylines}\n", outf);
}

/* General Summary footer */
void latex_gensumfoot(FILE *outf, Outchoices *od) {
  fputs("\\end{obeylines}\n", outf);
}

/* Single General Summary line, long data */
void latex_gensumline(FILE *outf, Outchoices *od, int namecode,
		      unsigned long x, unsigned long x7, logical isaverage) {

  fprintf(outf, "{\\bf %s%s} ", od->lngstr[namecode], od->lngstr[colon_]);

  f3printf(outf, od, (double)x, 0, od->sepchar);

  if (x7 != (unsigned long)UNSET) {
    fputs(" (", outf);
    f3printf(outf, od, (double)x7, 0, od->sepchar);
    putc(')', outf);
  }
  putc('\n', outf);
}

/* Single General Summary line, bytes data */
void latex_gensumlineb(FILE *outf, Outchoices *od, int namecode, double x,
		       double x7, logical isaverage) {
  char **lngstr = od->lngstr;

  unsigned int bm;
  char *c;

  fprintf(outf, "{\\bf %s%s} ",  lngstr[namecode], lngstr[colon_]);

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
void latex_lastseven(FILE *outf, Outchoices *od, timecode_t last7to) {
  fprintf(outf, "(%s %s %s).\n", od->lngstr[brackets_],
	  od->lngstr[sevendaysto_],
	  timesprintf(od, od->lngstr[datefmt1_], last7to, UNSET));
}

/* Start of a <pre> section */
void latex_prestart(FILE *outf, Outchoices *od) {
  fputs("\\smallskip\n\\begin{obeylines}\n", outf);
}

/* End of a <pre> section */
void latex_preend(FILE *outf, Outchoices *od) {
  fputs("\\end{obeylines}\n", outf);
}

/* A horizontal rule */
void latex_hrule(FILE *outf, Outchoices *od) {
  fputs("\\medskip\\hrule\n", outf);
}

/* An en dash */
char *latex_endash(void) {
  return "--";
}

/* putc with special characters escaped */
void latex_putch(FILE *outf, char c) {
  if (c == '&' || c == '$' || c == '%' || c == '&' || c == '#' || c == '_')
    fprintf(outf, "\\%c", c);
  else if (c == '|')
    fputs("$|$", outf);
  else if (c == '\\')
    fputs("$\\backslash$", outf);
  else if (c == '{' || c == '}')
    fprintf(outf, "$\\%c$", c);
  else if (c == '^' || c == '~')
    fprintf(outf, "\\%c{}", c);
  else
    putc(c, outf);
}

/* strlen */
size_t latex_strlength(const char *s) {
  return strlen(s);
}

/* Allow month in dates? DO NOT enable for human-readable text because of
   i18n problems. */
logical latex_allowmonth(void) {
  return FALSE;
}

/* Calculate column widths */
void latex_calcwidths(Outchoices *od, choice rep, unsigned int width[],
		      unsigned int *bmult, unsigned int *bmult7, double *unit,
		      unsigned long maxr, unsigned long maxr7,
		      unsigned long maxp, unsigned long maxp7, double maxb,
		      double maxb7, unsigned long howmany) {
  calcwidths(od, rep, width, bmult, bmult7, unit, maxr, maxr7, maxp, maxp7,
	     maxb, maxb7, howmany);
}

/* "Each unit represents" line */
void latex_declareunit(FILE *outf, Outchoices *od, char graphby, double unit,
		       unsigned int bmult) {
  char **lngstr = od->lngstr;

  char *s;

  fprintf(outf, "%s (", lngstr[eachunit_]);
  if (ISLOWER(graphby)) {
    if (od->markchar == '|')
      fprintf(outf, "\\verb+%c+", od->markchar);
    else
      fprintf(outf, "\\verb|%c|", od->markchar);
  }
  else
    fprintf(outf, "\\barchart{1}");
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
      latex_printdouble(outf, od, unit);
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
void latex_colheadstart(FILE *outf, Outchoices *od, choice rep) {
  verbchar = '|';  /* assume this never occurs in a column heading */
  /* Note no multibyte problem with this assignment of verbchar because
     LATEX output doesn't work in multibyte anyway. */
  fprintf(outf, "\\verb%c", verbchar);
}

/* Column header line: individual column */
void latex_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
		      unsigned int width, char *colname,
		      logical unterminated) {
  if (unterminated)
    fputs(colname, outf);
  else
    fprintf(outf, "%*s: ", width, colname);
}

/* End of column header line */
void latex_colheadend(FILE *outf, Outchoices *od, choice rep) {
  /* verbchar was set in latex_colheadstart */
  fprintf(outf, "%c\n", verbchar);
}

/* Start of column header underlining line */
void latex_colheadustart(FILE *outf, Outchoices *od, choice rep) {
  /* verbchar was set in latex_colheadstart */
  fprintf(outf, "\\verb%c", verbchar);
}

/* Underlining of one column header. */
/* If column is terminated, set width and leave name blank; and conversely. */
void latex_colheadunderline(FILE *outf, Outchoices *od, choice rep, choice col,
			    unsigned int width, char *name) {
  if (width > 0) {
    matchlengthn(outf, od, (size_t)width, '-');
    fputs(": ", outf);
  }
  else
    matchlength(outf, od, name, '-');
}

/* End of column header underlining line */
void latex_colheaduend(FILE *outf, Outchoices *od, choice rep) {
  /* verbchar was set in latex_colheadstart */
  fprintf(outf, "%c\n", verbchar);
}

/* Start of a table row */
void latex_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
		    int level, char *name, char *datefmt, char *timefmt) {
  /* Choice of a verbchar: can't be number, letter, colon, space, percent
     (LaTeX special characters are OK though); or the markchar, the decpt,
     or any character in a compiled date or in the name. Also we try only to
     use ASCII printable characters (32-127) for legible LaTeX code. That
     leaves the following choices, in order of preference. We return the
     verbchar at the end for the cases in which calling functions need to
     terminate the \verb */
  char *verbchars = "|+/!^#-=?_\\~'$\"()<>[]{}.,&;";
  char lastditch[] = {(char)(0xF7)};   /* the division sign in ISO-8859-[12] */
  
  for ( ;
       (*verbchars == od->markchar || *verbchars == od->decpt ||
	(name != NULL && strchr(name, *verbchars) != NULL) ||
	(datefmt != NULL && strchr(datefmt, *verbchars) != NULL) ||
	(timefmt != NULL && strchr(timefmt, *verbchars) != NULL))
	 && *verbchars != '\0';
       verbchars++)
    ;  /* Run to suitable verbchar */
  if (*verbchars == '\0')   /* no suitable char: last guess */
    verbchars = lastditch;
  verbchar = *verbchars;
  fprintf(outf, "\\verb%c", verbchar);

  /* Set the putc to plain_putch temporarily, because we are now in \verb. */
  od->outputter->putch = &plain_putch;
  od->outputter->endash = &plain_endash;
}

/* Print level in hierarchy represented by this row */
void latex_levelcell(FILE *outf, Outchoices *od, choice rep, int level) {
}

/* Name column */
void latex_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
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
  /* plain strlen because in \verb */

  strcpy(workspace, name);
  do_aliasx(workspace, aliashead);

  if (!isfirst) {
    if (rightalign)
      i = (int)width - (int)strlen(workspace) - (int)spaces;
    else
      i = (int)spaces;
    matchlengthn(outf, od, i, ' ');
  }

  fputs(workspace, outf);   /* and plain puts because in \verb */

  if (isfirst)
    fputs(": ", outf);
}

/* Single cell, unsigned long argument */
void latex_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
		  unsigned long x, unsigned int width) {
  f3printf(outf, od, (double)x, width, od->repsepchar);
  fputs(": ", outf);
}

/* Single cell, TRUSTED string argument */
void latex_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
		   char *s, unsigned int width) {
  matchlengthn(outf, od, width - strlen(s), ' ');  /* plain strlen: in \verb */
  fputs(s, outf);
  fputs(": ", outf);
}

/* Single cell, listing bytes */
void latex_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
		     double b, double bmult, unsigned int width) {
  printbytes(outf, od, b, bmult, width, od->repsepchar);
  fputs(": ", outf);
}

/* Single cell, listing percentage */
void latex_pccell(FILE *outf, Outchoices *od, choice rep, choice col, double n,
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
    putc(od->decpt, outf);       /* plain putc, because we are in \verb */
    fprintf(outf, "%02d%%", pc2);
  }
  fputs(": ", outf);
}

/* Single cell, index */
void latex_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
		    long index, unsigned int width) {
  /* If index is 0 (i.e. sub-item), just print spaces */
  if (index <= 0)
    matchlengthn(outf, od, width, ' ');
  else
    f3printf(outf, od, (double)index, width, od->repsepchar);
  fputs(": ", outf);
}

/* End of a table row */
void latex_rowend(FILE *outf, Outchoices *od, choice rep) {
  if (rep >= DATEREP_NUMBER || ISLOWER(od->graph[rep]))
    /* if timerep and u.c. graphby, closed \verb earlier */
    putc(verbchar, outf);
  putc('\n', outf);

  /* Undo temporary allocation of putc in _rowstart */
  od->outputter->putch = &latex_putch;
  od->outputter->endash = &latex_endash;
}

/* Blank line in time reports */
void latex_blankline(FILE *outf, Outchoices *od, choice *cols) {
  fputs("\\verb||\n", outf);
}

/* Barchart in time reports */
void latex_barchart(FILE *outf, Outchoices *od, int y, char graphby) {
  int i;

  if (ISLOWER(graphby)) {
    for (i = 0; i < y; i++)
      putc(od->markchar, outf);  /* plain putc, because we are in \verb */
  }
  else {
    putc(verbchar, outf);
    od->outputter->putch = &latex_putch;  /* now out of verb */
    od->outputter->endash = &latex_endash;
    fprintf(outf, "\\barchart{%d}", y);
  }
}

/* "Busiest time period" line */
void latex_busyprintf(FILE *outf, Outchoices *od, choice rep, char *datefmt,
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
void latex_notlistedstr(FILE *outf, Outchoices *od, choice rep,
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
  fprintf(outf, " %s]%c", (badn == 1)?colhead:colheadp, verbchar);
}

/* The line declaring the floor and sort for a report */
void latex_whatincluded(FILE *outf, Outchoices *od, choice rep,
			unsigned long n, Dateman *dman) {
  whatincluded(outf, od, rep, n, dman);
}

/* Spacing at the start of the whatincluded line */
void latex_whatinchead(FILE *outf, Outchoices *od) {
}

/* Finishing the whatincluded line */
void latex_whatincfoot(FILE *outf, Outchoices *od) {
  putc('\n', outf);
}

/* Printing part of the whatincluded line */
void latex_whatincprintstr(FILE *outf, Outchoices *od, char *s) {
  latexputs(outf, s);
}

/* Print a double to a nice number of decimal places */
void latex_printdouble(FILE *outf, Outchoices *od, double x) {
  unsigned int prec;
  double d;

  /* first calculate how many decimal places we need */

  for (prec = 0, d = x - (double)((int)(x));
       d - (double)((int)(d + 0.000005)) > 0.00001; d *= 10)
    prec++;

  /* now print it */

  if (prec > 0) {
    fprintf(outf, "%d", (int)x);
    latex_putch(outf, od->decpt);
    fprintf(outf, "%0*d", prec, (int)(d + EPSILON));
  }
  else
    fprintf(outf, "%d", (int)(x + EPSILON));
}

/* Include a header file or footer file */
void latex_includefile(FILE *outf, Outchoices *od, char *name, char type) {
  FILE *inf;
  char buffer[BLOCKSIZE];
  size_t n;

  if ((inf = my_fopen(name, (type == 'h')?"header file":"footer file")) !=
      NULL) {
    if (type == 'f') {
      latex_hrule(outf, od);
      fputs("\\smallskip\n", outf);
    }
    while ((n = fread(buffer, 1, BLOCKSIZE, inf)))  /* single equals */
      fwrite((void *)buffer, 1, n, outf);
    if (type == 'h') {
      latex_hrule(outf, od);
      fputs("\\smallskip\n", outf);
    }
    (void)my_fclose(inf, name, (type == 'h')?"header file":"footer file");
  }
}

/* Filetype for RISC OS */
unsigned int latex_riscosfiletype(void) {
  return 0xaca;
}

/* ======================================================================= */
/* Supporting functions for LaTeX */

void latexputs(FILE *outf, char *s) {
  char w1[64];
  char *c;
  char *w = w1;
  int len = 0;

  for (c = s; *c != '\0'; c++) {
    if (*c == '&' || *c == '$' || *c == '%' || *c == '&' || *c == '#' ||
	*c == '_') {
      PUTc(w, '\\');
      PUTc(w, *c);
      len += 2;
    }
    else if (*c == '|') {
      PUTs(w, "$|$", 0);
      len += 3;
    }
    else if (*c == '\\') {
      PUTs(w, "$\\backslash$", 0);
      len += 12;
    }
    else if (*c == '{' || *c == '}') {
      PUTs(w, "$\\", 0);
      PUTc(w, *c);
      PUTc(w, '$');
      len += 4;
    }
    else if (*c == '^' || *c == '~') {
      PUTc(w, '\\');
      PUTc(w, *c);
      PUTs(w, "{}", 0);
      len += 4;
    }
    else {
      PUTc(w, *c);
      len += 1;
    }
    if (len > 50) {
      *w = '\0';
      fputs(w1, outf);
      w = w1;
      len = 0;
    }
  }
  *w = '\0';
  fputs(w1, outf);
}
