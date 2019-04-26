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

/*** outcro.c; computer-readable output ***/

#include "anlghea3.h"

/* Page width -- unused in this output style */
unsigned int cro_pagewidth(Outchoices *od) {
  return 0;
}

/* The top of the output if we are in CGI mode */
void cro_cgihead(FILE *outf, Outchoices *od) {
  fprintf(outf, "Content-Type: text/plain\n\n");
}

/* Stuff this output style needs in the page header */
void cro_stylehead(FILE *outf, Outchoices *od) {
}

/* The title of the page, plus the user's HEADERFILE */
void cro_pagetitle(FILE *outf, Outchoices *od) {
  if (!strcaseeq(od->headerfile, "none"))
    cro_includefile(outf, od, od->headerfile, 'h');

  fprintf(outf, "x%sVE%s</a>Analog CE %s ( <a href=\"%s\">Analog CE</a> | <a href=\"%s\">Update Check</a> )\n", od->compsep, od->compsep, VNUMBER, ANALOGURL, CAMIERSSURL);
  fprintf(outf, "x%sHN%s%s\n", od->compsep, od->compsep, od->hostname);
  if (!strcaseeq(od->hosturl, "none"))
    fprintf(outf, "x%sHU%s%s\n", od->compsep, od->compsep, od->hosturl);
}

/* Program start time, and logfile start and end times */
void cro_timings(FILE *outf, Outchoices *od, Dateman *dman) {
  extern timecode_t starttimec;
  char *compsep = od->compsep;

  if (od->runtime)
    fprintf(outf, "x%sPS%s%s\n", compsep, compsep,
	    timesprintf(od, od->lngstr[datefmt2_], starttimec, UNSET));

  if (dman->firsttime <= dman->lasttime) {
    fprintf(outf, "x%sFR%s%s\n", compsep, compsep,
	    timesprintf(od, od->lngstr[datefmt2_], dman->firsttime, UNSET));
    fprintf(outf, "x%sLR%s%s\n", compsep, compsep,
	    timesprintf(od, od->lngstr[datefmt2_], dman->lasttime, UNSET));
  }
}

/* Finishing the top of the page */
void cro_closehead(FILE *outf, Outchoices *od) {
}

/* Starting the bottom of the page */
void cro_pagebotstart(FILE *outf, Outchoices *od) {
}

/* The credit line at the bottom of the page */
void cro_credit(FILE *outf, Outchoices *od) {
  /* Already printed at the top of the page in cro_pagehead() */
}

/* The program run time */
void cro_runtime(FILE *outf, Outchoices *od, long secs) {
}

/* The page footer, including the user's FOOTERFILE */
void cro_pagefoot(FILE *outf, Outchoices *od) {
  if (!strcaseeq(od->footerfile, "none"))
    cro_includefile(outf, od, od->footerfile, 'f');
}

/* Footer material for this output style */
void cro_stylefoot(FILE *outf, Outchoices *od) {
}

/* Report title */
void cro_reporttitle(FILE *outf, Outchoices *od, choice rep) {
}

/* Report footer */
void cro_reportfooter(FILE *outf, Outchoices *od, choice rep) {
}

/* Report description */
void cro_reportdesc(FILE *outf, Outchoices *od, choice rep) {
}

/* The time period spanned by the report */
void cro_reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd,
		    timecode_t mind) {
  extern char repcodes[];

  fprintf(outf, "%c%s*FR%s%s\n", repcodes[rep], od->compsep, od->compsep,
	  timesprintf(od, od->lngstr[datefmt2_], mind, UNSET));
  fprintf(outf, "%c%s*LR%s%s\n", repcodes[rep], od->compsep, od->compsep,
	  timesprintf(od, od->lngstr[datefmt2_], maxd, UNSET));
}

/* General Summary header */
void cro_gensumhead(FILE *outf, Outchoices *od) {
}

/* General Summary footer */
void cro_gensumfoot(FILE *outf, Outchoices *od) {
}

/* Single General Summary line, long data */
void cro_gensumline(FILE *outf, Outchoices *od, int namecode,
		    unsigned long x, unsigned long x7, logical isaverage) {
  char *compsep = od->compsep;
  char *name = od->lngstr[namecode];

  if (isaverage)
    return;   /* CRO doesn't display averages */

  fprintf(outf, "x%s%c%c%s%lu", compsep, name[0], name[1], compsep, x);
  if (x7 != (unsigned long)UNSET)
    fprintf(outf, "\nx%s%c%c%s%lu", compsep, name[2], name[3], compsep, x7);

  putc('\n', outf);
}

/* Single General Summary line, bytes data */
void cro_gensumlineb(FILE *outf, Outchoices *od, int namecode, double x,
		     double x7, logical isaverage) {
  char *compsep = od->compsep;
  char *name = od->lngstr[namecode];

  if (isaverage)
    return;   /* CRO doesn't display averages */

  fprintf(outf, "x%s%c%c%s%.0f", compsep, name[0], name[1], compsep, x);

  if (x7 != UNSET)
    fprintf(outf, "\nx%s%c%c%s%.0f", compsep, name[2], name[3], compsep, x7);

  putc('\n', outf);
}

/* "Last seven" explanation line */
void cro_lastseven(FILE *outf, Outchoices *od, timecode_t last7to) {
  fprintf(outf, "x%sE7%s%s\n", od->compsep, od->compsep,
	  timesprintf(od, od->lngstr[datefmt1_], last7to, UNSET));
}

/* Start of a <pre> section */
void cro_prestart(FILE *outf, Outchoices *od) {
}

/* End of a <pre> section */
void cro_preend(FILE *outf, Outchoices *od) {
}

/* A horizontal rule */
void cro_hrule(FILE *outf, Outchoices *od) {
}

/* An en dash */
char *cro_endash(void) {
  return "-";
}

/* putc with special characters escaped */
void cro_putch(FILE *outf, char c) {
  putc(c, outf);
}

/* strlen */
size_t cro_strlength(const char *s) {
  return strlen(s);
}

/* Allow month in dates? DO NOT enable for human-readable text because of
   i18n problems. */
logical cro_allowmonth(void) {
  return TRUE;
}

/* Calculate column widths */
void cro_calcwidths(Outchoices *od, choice rep, unsigned int width[],
		      unsigned int *bmult, unsigned int *bmult7, double *unit,
		      unsigned long maxr, unsigned long maxr7,
		      unsigned long maxp, unsigned long maxp7, double maxb,
		      double maxb7, unsigned long howmany) {
  unsigned int i;

  for (i = 0; i < COL_NUMBER; i++)
    width[i] = 0;

  *bmult = 0;
  *bmult7 = 0;
}

/* "Each unit represents" line */
void cro_declareunit(FILE *outf, Outchoices *od, char graphby, double unit,
		     unsigned int bmult) {
}

/* Start of column header line */
void cro_colheadstart(FILE *outf, Outchoices *od, choice rep) {
}

/* Column header line: individual column */
void cro_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
		    unsigned int width, char *colname, logical unterminated) {
}

/* End of column header line */
void cro_colheadend(FILE *outf, Outchoices *od, choice rep) {
}

/* Start of column header underlining line */
void cro_colheadustart(FILE *outf, Outchoices *od, choice rep) {
}

/* Underlining of one column header */
void cro_colheadunderline(FILE *ouf, Outchoices *od, choice rep, choice col,
			  unsigned int width, char *name) {
}

/* End of column header underlining line */
void cro_colheaduend(FILE *outf, Outchoices *od, choice rep) {
}

/* Start of a table row */
void cro_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
		  int level, char *name, char *datefmt, char *timefmt) {
  extern char repcodes[];
  extern char colcodes[];

  unsigned int c;

  fprintf(outf, "%c%s", repcodes[rep], od->compsep);
  if (level >= 1)
    putc('l', outf);
  for (c = 0; cols[c] != COL_NUMBER; c++)
    putc(colcodes[cols[c]], outf);
  fputs(od->compsep, outf);
}

/* Print level in hierarchy represented by this row */
void cro_levelcell(FILE *outf, Outchoices *od, choice rep, int level) {
  if (level >= 1)
        fprintf(outf, "%d%s", level, od->compsep);
}

/* Name column */
void cro_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
		  choice source, unsigned int width, logical name1st,
		  logical isfirst, logical rightalign, Alias *aliashead,
		  Include *linkhead, logical ispage, unsigned int spaces,
		  char *baseurl) {
  extern char *workspace;

  if (isfirst)
    return;

  strcpy(workspace, name);
  do_aliasx(workspace, aliashead);
  fputs(workspace, outf);
}

/* Single cell, unsigned long argument */
void cro_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
		unsigned long x, unsigned int width) {
  fprintf(outf, "%lu%s", x, od->compsep);
}

/* Single cell, TRUSTED string argument */
void cro_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
		 char *s, unsigned int width) {
  fprintf(outf, "%s%s", s, od->compsep);
}

/* Single cell, listing bytes */
void cro_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
		   double b, double bmult, unsigned int width) {
  fprintf(outf, "%.0f%s", b, od->compsep);
}

/* Single cell, listing percentage */
void cro_pccell(FILE *outf, Outchoices *od, choice rep, choice col, double n,
		double tot, unsigned int width) {
  fprintf(outf, "%.3f", (tot == 0)?0.0:(n * 100.0 / tot));
  fputs(od->compsep, outf);
}

/* Single cell, index */
void cro_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
		   long index, unsigned int width) {
  /* If index is 0 (i.e. sub-item), don't print anything */
  if (index > 0)
    fprintf(outf, "%ld", index);
  fputs(od->compsep, outf);
}

/* End of a table row */
void cro_rowend(FILE *outf, Outchoices *od, choice rep) {
  putc('\n', outf);
}

/* Blank line in time reports */
void cro_blankline(FILE *outf, Outchoices *od, choice *cols) {
}

/* Barchart in time reports */
void cro_barchart(FILE *outf, Outchoices *od, int y, char graphby) {
}

/* "Busiest time period" line */
void cro_busyprintf(FILE *outf, Outchoices *od, choice rep, char *datefmt,
		    unsigned long reqs, unsigned long pages, double bys,
		    datecode_t date, unsigned int hr, unsigned int min,
		    datecode_t newdate, unsigned int newhr,
		    unsigned int newmin, char graphby) {
  extern char repcodes[];

  char *compsep = od->compsep;

  putc(repcodes[rep], outf);
  if (TOLOWER(graphby) == 'r')
    fprintf(outf, "%s*BT%sR%s%lu", compsep, compsep, compsep, reqs);
  else if (TOLOWER(graphby) == 'p')
    fprintf(outf, "%s*BT%sP%s%lu", compsep, compsep, compsep, pages);
  else /* TOLOWER(graphby) == 'b' */
    fprintf(outf, "%.0f", bys);
  fprintf(outf, "%s%s\n", compsep,
	  datesprintf(od, datefmt, date, hr, min, newdate, newhr, newmin,
		      TRUE, UNSET));
}

/* End of "Not listed" line. */
void cro_notlistedstr(FILE *outf, Outchoices *od, choice rep,
		      unsigned long badn) {
  extern unsigned int *rep2lng;

  char **lngstr = od->lngstr;
  char gender = lngstr[rep2lng[rep] + 3][0];

  char *notlistedstr;

  if (gender == 'm')
    notlistedstr = lngstr[notlistedm_];
  else if (gender == 'f')
    notlistedstr = lngstr[notlistedf_];
  else
    notlistedstr = lngstr[notlistedn_];

  fprintf(outf, "[%s: %lu]", notlistedstr, badn);
}

/* The line declaring the floor and sort for a report */
void cro_whatincluded(FILE *outf, Outchoices *od, choice rep,
		      unsigned long n, Dateman *dman) {
  extern char repcodes[];

  choice sortby = od->sortby[G(rep)];
  double floormin = od->floor[G(rep)].min;
  char floorqual = od->floor[G(rep)].qual;
  choice floorby = od->floor[G(rep)].floorby;

  timecode_t tempd;

  fprintf(outf, "%c%s*f%s", repcodes[rep], od->compsep, od->compsep);
  if (floormin < 0)
    fprintf(outf, "-%lu", (unsigned long)(-floormin + EPSILON));
  else if (floorby == DATESORT || floorby == FIRSTDATE) {
    tempd = (timecode_t)(floormin + EPSILON);
    fputs(timesprintf(od, "%Y%M%D:%H%n", tempd, UNSET), outf);
  }
  else if (floorqual == '\0')
    fprintf(outf, "%lu", (unsigned long)(floormin + EPSILON));
  else
    fprintf(outf, "%f", floormin);
  if (floorqual != '\0')
    putc(floorqual, outf);
  if (floorby == REQUESTS)
    putc('R', outf);
  else if (floorby == REQUESTS7)
    putc('S', outf);
  else if (floorby == PAGES)
    putc('P', outf);
  else if (floorby == PAGES7)
    putc('Q', outf);
  else if (floorby == BYTES)
    putc('B', outf);
  else if (floorby == BYTES7)
    putc('C', outf);
  else if (floorby == DATESORT)
    putc('D', outf);
  else /* floorby == FIRSTDATE */
    putc('E', outf);

  /* now the sortby */
  if (floormin < 0 && sortby == RANDOM)
    sortby = floorby;
  fprintf(outf, "%s", od->compsep);
  if (sortby == ALPHABETICAL)
    putc('a', outf);
  else if (sortby == BYTES)
    putc('b', outf);
  else if (sortby == BYTES7)
    putc('c', outf);
  else if (sortby == DATESORT)
    putc('d', outf);
  else if (sortby == FIRSTDATE)
    putc('e', outf);
  else if (sortby == PAGES)
    putc('p', outf);
  else if (sortby == PAGES7)
    putc('q', outf);
  else if (sortby == REQUESTS)
    putc('r', outf);
  else if (sortby == REQUESTS7)
    putc('s', outf);
  else /* sortby == RANDOM */
    putc('x', outf);
  putc('\n', outf);
}

/* Include a header file or footer file */
void cro_includefile(FILE *outf, Outchoices *od, char *name, char type) {
  FILE *inf;
  char buffer[BLOCKSIZE];
  size_t n;

  if ((inf = my_fopen(name, (type == 'h')?"header file":"footer file")) !=
      NULL) {
    while ((n = fread(buffer, 1, BLOCKSIZE, inf)))  /* single equals */
      fwrite((void *)buffer, 1, n, outf);
    (void)my_fclose(inf, name, (type == 'h')?"header file":"footer file");
  }
}

/* Filetype for RISC OS */
unsigned int cro_riscosfiletype(void) {
  return 0xfff;
}
