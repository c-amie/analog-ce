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

/*** outxhtml.c; XHTML output ***/

#include "anlghea3.h"

/* Page width */
unsigned int xhtml_pagewidth(Outchoices *od) {
  return od->htmlpagewidth;  /* only used to guide the bar chart width */
}

/* The top of the output if we are in CGI mode */
void xhtml_cgihead(FILE *outf, Outchoices *od) {
  fprintf(outf, "Content-Type: application/xhtml+xml; charset=%s\n\n",
	  od->lngstr[charset_]);
}

/* Stuff this output style needs in the page header */
void xhtml_stylehead(FILE *outf, Outchoices *od) {
  fprintf(outf, "<?xml version=\"1.0\" encoding=\"%s\"?>\n",
	  od->lngstr[charset_]);
  if (strcaseeq(od->stylesheet, "none"))
    fputs("<?xml-stylesheet href=\"#internalStyle\" type=\"text/css\"?>\n",
	  outf);
  else {
    fputs("<?xml-stylesheet href=\"", outf);
    xhtmlputs(outf, od, od->stylesheet, IN_HREF);
    fputs("\" type=\"text/css\"?>\n", outf);
  }
  fputs("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"", outf);
  fputs(" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n", outf);
  fputs("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n", outf);
  fputs("<head>\n", outf);
  fputs("<title>", outf);
  xhtmlputs(outf, od, od->hostname, FROM_CFG);
  fprintf(outf, " %s", od->lngstr[webstatsfor_]);
    fputs("</title>\n", outf);
  fprintf(outf, "<meta http-equiv=\"Content-Type\" "
	  "content=\"text/html; charset=%s\" />\n", od->lngstr[charset_]);
  if (od->norobots)
    fputs("<meta name=\"robots\" content=\"noindex,nofollow\" />\n", outf);
  fprintf(outf, "<meta name=\"generator\" content=\"analog %s\" />\n",
	  VERSION);
  if (!strcaseeq(od->stylesheet, "none")) {
    fputs("<link href=\"", outf);
    xhtmlputs(outf, od, od->stylesheet, IN_HREF);
    fputs("\" rel=\"stylesheet\" />\n", outf);
  }
  else { /* default style sheet inline if no external style sheet specified */
    fprintf(outf, "<style type=\"text/css\" id=\"internalStyle\">\n");
    fprintf(outf, "h2 {\n\tbackground-color: #A0C0F0;\n\twidth: 98%%;\n\t"
	  "padding: 3px 6px;\n}\n");
    fprintf(outf, "table {\n\ttext-align: right;\n\tmargin-left: 30px;\n\t"
	    "background-color: #D0E0F0;\n\tborder-collapse: collapse;\n}\n");
    fprintf(outf, "th {\n\tborder-bottom: 1px solid #404050;\n\t"
	  "border-right: 1px dotted #606070;\n\tborder-top: none;\n\t"
	  "border-left: none;\n\tpadding: 0px 5px 1px 5px;\n\t"
	  "font-weight: bold;\n\tbackground-color: #A0C0F0;\n}\n");
    fprintf(outf, "td {\n\tpadding: 0px 5px 1px 5px;\n\t"
	  "border-right: 1px dotted #606070;\n\tborder-left: none;\n\t"
	  "border-bottom: none;\n\tborder-top:none;\n}\n");
    fprintf(outf,
	    "td.%sx {\n\tfont-family: monospace;\n\twhite-space: pre;\n}\n",
	    od->cssprefix);
    fprintf(outf, ".%sxl {\n\ttext-align: left;\n\tborder-right: none;\n}\n",
	    od->cssprefix);
    fprintf(outf, ".%sxr {\n\ttext-align: right;\n\tborder-right: none;\n}\n",
	    od->cssprefix);
    fprintf(outf, ".%sbar {\n\ttext-align: left;\n\tborder-right: none;\n}\n",
	    od->cssprefix);
    fprintf(outf,
	    "tr.%ssub {background-color: #C0C0FF;\n\tfont-style: italic;\n}\n",
	    od->cssprefix);
    fprintf(outf, ".%srepdesc {\n\tfont-style: italic;\n}\n", od->cssprefix);
    fprintf(outf, ".%srepspan {\n\tfont-style: italic;\n}\n", od->cssprefix);
    fprintf(outf, ".%sgoto {\n\tfont-size: small;\n}\n", od->cssprefix);
    fprintf(outf, ".%sgensumtitle {\n\tfont-weight: bold;\n}\n",
	    od->cssprefix);
    fprintf(outf, ".%sgototitle {\n\tfont-weight: bold;\n}\n", od->cssprefix);
    fprintf(outf, ".%sruntimetitle {\n\tfont-weight: bold;\n}\n",
	    od->cssprefix);
    fprintf(outf, "img {\n\tborder-style: none;\n}\n");
    fprintf(outf, "a:link {\n\tcolor: blue;\n\ttext-decoration: none;\n}\n");
    fprintf(outf,
	    "a:visited {\n\tcolor: purple;\n\ttext-decoration: none;\n}\n");
    fprintf(outf, "a:link:hover {\n\ttext-decoration: underline;\n}\n");
    fprintf(outf, "a:visited:hover {\n\ttext-decoration: underline;\n}\n");
    fprintf(outf, "a:link:active {\n\tcolor: red;\n\t"
	    "text-decoration: underline;\n}\n");
    fprintf(outf, "a:visited:active {\n\tcolor: red;\n\t"
	  "text-decoration: underline;\n}\n");
    fprintf(outf, "</style>\n");
  }  /* end of style sheet */
  fputs("</head>\n", outf);
  fputs("<body>\n", outf);
}

/* The title of the page, plus the user's HEADERFILE */
void xhtml_pagetitle(FILE *outf, Outchoices *od) {
  fprintf(outf, "<div class=\"%sheader\">", od->cssprefix);
  fputs("<h1><a name=\"Top\" id=\"Top\"", outf);
  if (!strcaseeq(od->logourl, "none")) {
    fputs(" href=\"", outf);
    xhtmlputs(outf, od, od->logourl, IN_HREF);
    fputs("\"", outf);
  }
  fputs(">", outf);
  
  if (!strcaseeq(od->logo, "none")) {
    fputs("<img src=\"", outf);
    if (od->logo[0] != '/' && strstr(od->logo, "://") == NULL)
      xhtmlputs(outf, od, od->imagedir, IN_HREF);
    xhtmlputs(outf, od, od->logo, IN_HREF);
    if (STREQ(od->logo, "analogo"))
      fprintf(outf, ".%s", od->pngimages?"png":"gif");
    /* Above: '.' not EXTSEP even on RISC OS */
    fputs("\" alt=\"\" /></a> ", outf);  /* close <a> here if logo */
  }
  if (strcaseeq(od->hosturl, "none")) {
    fprintf(outf, "%s ", od->lngstr[webstatsfor_]);
    xhtmlputs(outf, od, od->hostname, FROM_CFG);
    if (strcaseeq(od->logo, "none"))
      fputs("</a>", outf);  /* close <a> here if no logo */
  }
  else {
    fputs(od->lngstr[webstatsfor_], outf);
    if (strcaseeq(od->logo, "none"))
      fputs("</a>", outf);  /* close <a> here if no logo */
    fputs(" <a href=\"", outf);
    xhtmlputs(outf, od, od->hosturl, IN_HREF);
    fputs("\">", outf);
    xhtmlputs(outf, od, od->hostname, FROM_CFG);
    fputs("</a>", outf);
  }
  fputs("</h1>\n\n", outf);

  if (!strcaseeq(od->headerfile, "none"))
    xhtml_includefile(outf, od, od->headerfile, 'h');
}

/* Program start time, and logfile start and end times */
void xhtml_timings(FILE *outf, Outchoices *od, Dateman *dman) {
  extern timecode_t starttimec;

  char **lngstr = od->lngstr;

  double t0;
  int t1, t2;

  if (!od->runtime && dman->firsttime > dman->lasttime)
    return; /* Nothing to show, avoid printing <p></p> */

  fprintf(outf, "<p class=\"%sanalysisspan\">", od->cssprefix);
  if (od->runtime)
    fprintf(outf, "%s %s.", lngstr[progstart_],
	    timesprintf(od, lngstr[datefmt2_], starttimec, UNSET));

  if (dman->firsttime <= dman->lasttime) {
    if (od->runtime)
      fputs("\n<br />", outf);
    fprintf(outf, "%s %s ", lngstr[reqstart_],
	    timesprintf(od, lngstr[datefmt2_], dman->firsttime, UNSET));
    fprintf(outf, "%s %s", lngstr[to_],
	    timesprintf(od, lngstr[datefmt2_], dman->lasttime, UNSET));
    t0 = (dman->lasttime - dman->firsttime) / 1440.0 + 0.005;
    t1 = (int)t0;
    t2 = (int)(100 * (t0 - (double)t1));
    fprintf(outf, " (%d", t1);
    xhtml_putch(outf, od->decpt);
    fprintf(outf, "%02d %s).</p>\n", t2, lngstr[days_]);
  }
  else
    fputs("</p>\n", outf);
}

/* Finishing the top of the page */
void xhtml_closehead(FILE *outf, Outchoices *od) {
  fputs("</div>\n", outf);
  if (od->gotos == FEW)
    xhtml_gotos(outf, od, -1);
}

/* Starting the bottom of the page */
void xhtml_pagebotstart(FILE *outf, Outchoices *od) {
  fprintf(outf, "<div class=\"%sfooter\">", od->cssprefix);
}

/* The credit line at the bottom of the page */
void xhtml_credit(FILE *outf, Outchoices *od) {
  fprintf(outf, "<p class=\"%scredit\">%s <a href=\"%s\" rel=\"follow\">CE %s</a>.\n",
	  od->cssprefix, od->lngstr[credit_], ANALOGURL, VNUMBER);
}

/* The program run time */
void xhtml_runtime(FILE *outf, Outchoices *od, long secs) {
  char **lngstr = od->lngstr;

  fprintf(outf, "<br /><span class=\"%sruntimetitle\">%s:</span> ",
	  od->cssprefix, lngstr[runtime_]);

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
void xhtml_pagefoot(FILE *outf, Outchoices *od) {
  fputs("</p>\n", outf);
  if (od->gotos != FALSE)
    xhtml_gotos(outf, od, -1);
  fputs("</div>\n", outf);

  if (!strcaseeq(od->footerfile, "none"))
    xhtml_includefile(outf, od, od->footerfile, 'f');
}

/* Footer material for this output style */
void xhtml_stylefoot(FILE *outf, Outchoices *od) {
  fputs("</body>\n</html>\n", outf);
}

/* Report title */
void xhtml_reporttitle(FILE *outf, Outchoices *od, choice rep) {
  extern char *anchorname[];
  extern unsigned int *rep2lng;

  fprintf(outf, "<div class=\"%s%s\">", od->cssprefix, anchorname[rep]);
  fprintf(outf, "<h2><a name=\"%s\" id=\"%s\">%s</a></h2>\n", anchorname[rep],
	  anchorname[rep], od->lngstr[rep2lng[rep]]);
  if (od->gotos == TRUE)
    xhtml_gotos(outf, od, rep);
}

/* Report footer */
void xhtml_reportfooter(FILE *outf, Outchoices *od, choice rep) {
  fputs("</div>", outf);
}

/* Report description */
void xhtml_reportdesc(FILE *outf, Outchoices *od, choice rep) {
  fprintf(outf, "<p class=\"%srepdesc\">%s</p>\n", od->cssprefix,
	  od->descstr[rep]);
}

/* The time period spanned by the report */
void xhtml_reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd,
		     timecode_t mind) {
  /* NB Can't combine next two lines because timesprintf uses static buffer. */
  fprintf(outf, "<p class=\"%srepspan\">%s %s ", od->cssprefix,
	  od->lngstr[repspan_],
	  timesprintf(od, od->lngstr[datefmt2_], mind, UNSET));
  fprintf(outf, "%s %s.</p>\n", od->lngstr[to_],
	  timesprintf(od, od->lngstr[datefmt2_], maxd, UNSET));
}

/* General Summary header */
void xhtml_gensumhead(FILE *outf, Outchoices *od) {
}

/* General Summary footer */
void xhtml_gensumfoot(FILE *outf, Outchoices *od) {
  fputs("</p>\n", outf);
}

/* Single General Summary line, long data */
void xhtml_gensumline(FILE *outf, Outchoices *od, int namecode,
		      unsigned long x, unsigned long x7, logical isaverage) {

  /* If this is the first Gen Sum line, open the paragraph.
     (succreqs_ is always printed, and is always first). */
  if (namecode == succreqs_)
    fprintf(outf, "<p class=\"%sgensumlines\">\n", od->cssprefix);
  /* If it isn't, then put a line break. */
  else
    fputs("<br />", outf);
  fprintf(outf, "<span class=\"%sgensumtitle\">%s%s</span> ", od->cssprefix,
	  od->lngstr[namecode], od->lngstr[colon_]);

  f3printf(outf, od, (double)x, 0, od->sepchar);

  if (x7 != (unsigned long)UNSET) {
    fputs(" (", outf);
    f3printf(outf, od, (double)x7, 0, od->sepchar);
    putc(')', outf);
  }
  putc('\n', outf);
}

/* Single General Summary line, bytes data */
void xhtml_gensumlineb(FILE *outf, Outchoices *od, int namecode, double x,
		      double x7, logical isaverage) {
  char **lngstr = od->lngstr;

  unsigned int bm;
  char *c;

  fprintf(outf, "<br /><span class=\"%sgensumtitle\">%s%s</span> ",
	  od->cssprefix, lngstr[namecode], lngstr[colon_]);

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
void xhtml_lastseven(FILE *outf, Outchoices *od, timecode_t last7to) {
  fprintf(outf, "<p class=\"%slastseven\">", od->cssprefix);
  fprintf(outf, "%s %s %s.</p>\n", od->lngstr[brackets_],
	  od->lngstr[sevendaysto_],
	  timesprintf(od, od->lngstr[datefmt1_], last7to, UNSET));
}

/* Start of a <pre> section */
void xhtml_prestart(FILE *outf, Outchoices *od) {
  fputs("<table>\n", outf);
}

/* End of a <pre> section */
void xhtml_preend(FILE *outf, Outchoices *od) {
  fputs("</tbody></table>\n", outf);
}

/* A horizontal rule */
void xhtml_hrule(FILE *outf, Outchoices *od) {
}

/* An en dash */
char *xhtml_endash(void) {
  return "&ndash;";
}

/* putc with special characters escaped */
void xhtml_putch(FILE *outf, char c) {
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

/* strlen for XHTML strings. Assume string contains no &'s except as markup.
   May be switched to html_strlength_utf8 or html_strlength_jis in outhtml.c,
   depending on the output character set. */
size_t xhtml_strlength(const char *s) {
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

/* Allow month in dates? DO NOT enable for human-readable text because of
   i18n problems. */
logical xhtml_allowmonth(void) {
  return FALSE;
}

/* Calculate column widths */
void xhtml_calcwidths(Outchoices *od, choice rep, unsigned int width[],
		      unsigned int *bmult, unsigned int *bmult7, double *unit,
		      unsigned long maxr, unsigned long maxr7,
		      unsigned long maxp, unsigned long maxp7, double maxb,
		      double maxb7, unsigned long howmany) {
  /* widths don't really have much meaning, but they will still be used for the
     bar charts, and it's easier just to call the existing function. */
  calcwidths(od, rep, width, bmult, bmult7, unit, maxr, maxr7, maxp, maxp7,
	     maxb, maxb7, howmany);
}

/* "Each unit represents" line */
void xhtml_declareunit(FILE *outf, Outchoices *od, char graphby, double unit,
		       unsigned int bmult) {
  char **lngstr = od->lngstr;

  char *s;

  fprintf(outf, "<p class=\"%seachunit\">", od->cssprefix);
  fprintf(outf, "%s (", lngstr[eachunit_]);
  if (ISLOWER(graphby))
    fprintf(outf, "%c", od->markchar);
  else {
    fprintf(outf, "<img src=\"");
    xhtmlputs(outf, od, od->imagedir, IN_HREF);
    fprintf(outf, "bar%c1.%s\" alt=\"%c\" />", od->barstyle,
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
      xhtml_printdouble(outf, od, unit);
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
  fputs("</p>\n", outf);
}

/* Start of column header line */
void xhtml_colheadstart(FILE *outf, Outchoices *od, choice rep) {
  fputs("<thead><tr>", outf);
}

/* Column header line: individual column */
void xhtml_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
		     unsigned int width, char *colname, logical unterminated) {
  extern char colcodes[];

  char code[3];

  code[0] = colcodes[col];
  if (unterminated) {
    code[1] = 'l';  /* i.e. "xl" */
    code[2] = '\0';
  }
  else
    code[1] = '\0';

  fprintf(outf, "<th class=\"%s%s\">%s</th>", od->cssprefix, code, colname);
}

/* End of column header line */
void xhtml_colheadend(FILE *outf, Outchoices *od, choice rep) {

  if (rep < DATEREP_NUMBER)  /* time report: extra col for bar chart */
    fprintf(outf, "<th class=\"%sbar\">&nbsp;</th>", od->cssprefix);

  fputs("</tr></thead>\n", outf);
}

/* Start of column header underlining line */
void xhtml_colheadustart(FILE *outf, Outchoices *od, choice rep) {
}

/* Underlining of one column header. */
void xhtml_colheadunderline(FILE *outf, Outchoices *od, choice rep, choice col,
			   unsigned int width, char *name) {
}

/* End of column header underlining line */
void xhtml_colheaduend(FILE *outf, Outchoices *od, choice rep) {
  fputs("<tbody>", outf);
}

/* Start of a table row */
void xhtml_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
		   int level, char *name, char *datefmt, char *timefmt) {
  if (level >= 2)
    fprintf(outf, "<tr class=\"%ssub %slevel%d\">",
	    od->cssprefix, od->cssprefix, level);
  else
    fprintf(outf, "<tr>");
}

/* Print level in hierarchy represented by this row */
void xhtml_levelcell(FILE *outf, Outchoices *od, choice rep, int level) {
}

/* Name column */
void xhtml_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
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

  savemultibyte = od->multibyte;
  if (rep == REP_SIZE || rep == REP_PROCTIME)
    /* Kludge: for these two reports, we know the texts are things like
       "< 1" and we want to convert > and < */
    od->multibyte = FALSE;

  if (isfirst)
    fprintf(outf, "<td class=\"%sx\">", od->cssprefix);
  else if (rightalign)
    fprintf(outf, "<td class=\"%sxr\">", od->cssprefix);
  else
    fprintf(outf, "<td class=\"%sxl\">", od->cssprefix);

  strcpy(workspace, name);
  do_aliasx(workspace, aliashead);

  if (!isfirst && !rightalign) {
    for (i = 0; i < spaces; i++)
      fputs("&nbsp;", outf);
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
	  xhtmlputs(outf, od, od->anonymizerurl, IN_HREF);
	}
    if (baseurl != NULL)
      xhtmlputs(outf, od, baseurl, IN_HREF);
    xhtml_escfprintf(outf, name);
	/* If set, add rel="nofollow" */
	if (od->linknofollow) {
      fputs("\" rel=\"nofollow\">", outf);
	} else {
      fputs("\">", outf);
	}
  }

  xhtmlputs(outf, od, workspace, source);

  if (linked)
    fputs("</a>", outf);

  if (!isfirst && rightalign) {
    for (i = 0; i < spaces; i++)
      fputs("&nbsp;", outf);
  }

  fputs("</td>", outf);

  od->multibyte = savemultibyte;  /* restore multibyte */
}

/* Single cell, unsigned long argument */
void xhtml_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
		  unsigned long x, unsigned int width) {
  extern char colcodes[];

  fprintf(outf, "<td class=\"%s%c\">", od->cssprefix, colcodes[col]);
  f3printf(outf, od, (double)x, 0, od->repsepchar);
  fprintf(outf, "</td>");
}

/* Single cell, TRUSTED string argument */
void xhtml_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
		   char *s, unsigned int width) {
  extern char colcodes[];

  fprintf(outf, "<td class=\"%s%c\">", od->cssprefix, colcodes[col]);
  xhtmlputs(outf, od, s, TRUSTED);
  fprintf(outf, "</td>");
}

/* Single cell, listing bytes */
void xhtml_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
		     double b, double bmult, unsigned int width) {
  extern char colcodes[];

  fprintf(outf, "<td class=\"%s%c\">", od->cssprefix, colcodes[col]);
  printbytes(outf, od, b, bmult, 0, od->repsepchar);
  fprintf(outf, "</td>");
}

/* Single cell, listing percentage */
void xhtml_pccell(FILE *outf, Outchoices *od, choice rep, choice col, double n,
		  double tot, unsigned int width) {
  double pc;
  unsigned int pc1, pc2;

  extern char colcodes[];

  fprintf(outf, "<td class=\"%s%c\">", od->cssprefix, colcodes[col]);
  if (tot == 0)
    pc = 0.0;
  else
    pc = n * 10000.0 / tot;
  if (pc >= 9999.5)
    fputs("100%", outf);
  else if (pc < 0.5)
    fputs("&nbsp;", outf);
  else {
    pc1 = ((int)(pc + 0.5)) / 100;
    pc2 = ((int)(pc + 0.5)) % 100;
    fprintf(outf, "%2d", pc1);
    xhtml_putch(outf, od->decpt);
    fprintf(outf, "%02d%%", pc2);
  }
  fputs("</td>", outf);
}

/* Single cell, index */
void xhtml_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
		     long index, unsigned int width) {
  extern char colcodes[];

  fprintf(outf, "<td class=\"%s%c\">", od->cssprefix, colcodes[col]);

  /* If index is 0 (i.e. sub-item), just print space */
  if (index <= 0)
    fputs("&nbsp;", outf);
  else
    f3printf(outf, od, (double)index, 0, od->repsepchar);
  fputs("</td>", outf);
}

/* End of a table row */
void xhtml_rowend(FILE *outf, Outchoices *od, choice rep) {
  fputs("</tr>\n", outf);
}

/* Blank line in time reports */
void xhtml_blankline(FILE *outf, Outchoices *od, choice *cols) {
  extern char colcodes[];
  unsigned int c;

  fputs("<tr>", outf);
  fprintf(outf, "<td class=\"%sx\">&nbsp;</td>", od->cssprefix);
  for (c = 0; cols[c] != COL_NUMBER; c++)
    fprintf(outf, "<td class=\"%s%c\">&nbsp;</td>", od->cssprefix,
	    colcodes[cols[c]]);
  fprintf(outf, "<td class=\"%sbar\">&nbsp;</td>", od->cssprefix);
  fputs("</tr>\n", outf);
}

/* Barchart in time reports */
void xhtml_barchart(FILE *outf, Outchoices *od, int y, char graphby) {
  int i, j;
  logical first = TRUE;

  fprintf(outf, "<td class=\"%sbar\">", od->cssprefix);

  if (ISLOWER(graphby)) {
    for (i = 0; i < y; i++)
      xhtml_putch(outf, od->markchar);
  }
  else {
    for (j = 32; j >= 1; j /= 2) {
      while (y >= j) {
	fputs("<img src=\"", outf);
	xhtmlputs(outf, od, od->imagedir, IN_HREF);
	fprintf(outf, "bar%c%d.%s\" alt=\"", od->barstyle, j,
		od->pngimages?"png":"gif");/* '.' not EXTSEP even on RISC OS */
	if (first) {
	  for (i = 0; i < y; i++)
	    xhtml_putch(outf, od->markchar);
	  first = FALSE;
	}
	fputs("\" />", outf);
	y -= j;
      }
    }
  }

  fputs("</td>", outf);
}

/* "Busiest time period" line */
void xhtml_busyprintf(FILE *outf, Outchoices *od, choice rep, char *datefmt,
		     unsigned long reqs, unsigned long pages, double bys,
		     datecode_t date, unsigned int hr, unsigned int min,
		     datecode_t newdate, unsigned int newhr,
		     unsigned int newmin, char graphby) {
  extern unsigned int *rep2busystr;

  char **lngstr = od->lngstr;
  char sepchar = od->sepchar;

  unsigned int bmult;
  char *s;

  fprintf(outf, "<p class=\"%sbusiesttime\">%s %s (", od->cssprefix,
	  lngstr[rep2busystr[rep]],
	  datesprintf(od, datefmt, date, hr, min, newdate, newhr, newmin,
		      TRUE, UNSET));
  if (TOLOWER(graphby) == 'r') {
    f3printf(outf, od, (double)reqs, 0, sepchar);
    fprintf(outf, " %s", (reqs == 1)?lngstr[request_]:lngstr[requests_]);
  }
  else if (TOLOWER(graphby) == 'p') {
    f3printf(outf, od, (double)pages, 0, sepchar);
    fprintf(outf, " %s",
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
      fprintf(outf, "%s%s%s", lngstr[xbytes_],
	      lngstr[byteprefix_ + bmult], s + 1);
      *s = '?';
    }
    else
      fprintf(outf, "%s", lngstr[bytes_]);
  }
  fputs(").</p>\n", outf);
}

/* End of "Not listed" line. */
void xhtml_notlistedstr(FILE *outf, Outchoices *od, choice rep,
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

  fprintf(outf, "<td class=\"%sxl\">[%s: ", od->cssprefix, notlistedstr);
  f3printf(outf, od, (double)badn, 0, od->sepchar);
  fprintf(outf, " %s]</td>", (badn == 1)?colhead:colheadp);
}

/* The line declaring the floor and sort for a report */
void xhtml_whatincluded(FILE *outf, Outchoices *od, choice rep,
			unsigned long n, Dateman *dman) {
  whatincluded(outf, od, rep, n, dman);
}

/* Spacing at the start of the whatincluded line */
void xhtml_whatinchead(FILE *outf, Outchoices *od) {
  fprintf(outf, "<p class=\"%swhatinc\">\n", od->cssprefix);
}

/* Finishing the whatincluded line */
void xhtml_whatincfoot(FILE *outf, Outchoices *od) {
  fputs("</p>\n", outf);
}

/* Printing part of the whatincluded line */
void xhtml_whatincprintstr(FILE *outf, Outchoices *od, char *s) {
  xhtmlputs(outf, od, s, TRUSTED);
}

/* Print a double to a nice number of decimal places */
void xhtml_printdouble(FILE *outf, Outchoices *od, double x) {
  unsigned int prec;
  double d;

  /* first calculate how many decimal places we need */

  for (prec = 0, d = x - (double)((int)(x));
       d - (double)((int)(d + 0.000005)) > 0.00001; d *= 10)
    prec++;

  /* now print it */

  if (prec > 0) {
    fprintf(outf, "%d", (int)x);
    xhtml_putch(outf, od->decpt);
    fprintf(outf, "%0*d", prec, (int)(d + EPSILON));
  }
  else
    fprintf(outf, "%d", (int)(x + EPSILON));
}

/* Include a header file or footer file */
void xhtml_includefile(FILE *outf, Outchoices *od, char *name, char type) {
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
unsigned int xhtml_riscosfiletype(void) {
  return 0xfaf;
}

/* ======================================================================= */
/* Supporting functions for XHTML */

/* Print "goto"s. Assume we've checked that we want gotos here. */
void xhtml_gotos(FILE *outf, Outchoices *od, choice rep) {
  extern unsigned int *rep2lng;
  extern char *anchorname[];

  choice *reporder = od->reporder;
  char **lngstr = od->lngstr;
  int i;

  fprintf(outf, "<p class=\"%sgoto\">(<span class=\"%sgototitle\">%s</span>",
	  od->cssprefix, od->cssprefix, lngstr[goto_]);
  fprintf(outf, "%s <a href=\"#Top\">%s</a>", lngstr[colon_], lngstr[top_]);
  for (i = 0; reporder[i] != -1; i++) {
    if (reporder[i] == rep)
      fprintf(outf, " | %s", lngstr[rep2lng[reporder[i]]]);
    else if (od->repq[reporder[i]])
      fprintf(outf, " | <a href=\"#%s\">%s</a>", anchorname[reporder[i]],
	      lngstr[rep2lng[reporder[i]]]);
  }
  fputs(")</p>\n", outf);
}

/* Escape names for use in hyperlinks. */
void xhtml_escfprintf(FILE *outf, char *name) {
  html_escfprintf(outf, name);
}

/* Print a string with an appropriate amount of HTML encoding. */
void xhtmlputs(FILE *outf, Outchoices *od, char *s, choice source) {
  htmlputs(outf, od, s, source);
}
