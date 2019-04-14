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

/*** settings.c; report variable settings */

#include "anlghea3.h"

void report_vbles(Options *op) {
  extern char *item_type[];
  extern int stz;
  extern char *debug_args, *warn_args;
  extern unsigned long progressfreq;
  extern logical cgi, anywarns;
#ifndef NOPIPES
  extern Strpairlist *uncompresshead;
#endif
  int i;

  if (cgi)
    printf("Content-Type: text/plain\n\n");

  printf("This is analog version %s\n", VERSION);
  printf("For more information on analog see docs/Readme.html or %s\n\n",
	 ANALOGURL);

  anywarns = TRUE;
  report_compile();
  report_conffiles(op->conffilelist);
  report_debug("Warning types", warn_args, "CDEFLMR");
  report_debug("Debugging types", debug_args, "CDFSUV");
  if (progressfreq == 1)
    printf("Reporting progress every line\n");
  else if (progressfreq > 0)
    printf("Reporting progress every %lu lines\n", progressfreq);
  report_logfiles(op->miscopts.logfile[1], TRUE);
  report_logfiles(op->miscopts.logfile[0], FALSE);
#ifndef NOPIPES
  report_strpairlist(uncompresshead, "Uncompressing files", "by ");
#endif
  report_fromto(&(op->dman));
  if (stz != 0)
    printf("Local computer time offset by %s%d minutes\n", (stz > 0)?"+":"",
	   stz);
  for (i = 0; i < ITEM_NUMBER; i++)
    report_want(op->wanthead[i], item_type[i], "");
  report_scwant(op->code2type);
  report_want(op->ispagehead, "as pages", "");
  report_want(op->argshead, "search arguments", "");
  report_want(op->refargshead, "referrer search arguments", "");
  if (op->outopts.repq[REP_OS])
    report_want(op->robots, "as robots", "");
  if (op->outopts.repq[REP_SEARCHREP] || op->outopts.repq[REP_SEARCHSUM])
    report_strpairlist(op->searchengines, "Search engines", "arg=");
  if (op->outopts.repq[REP_INTSEARCHREP] || op->outopts.repq[REP_INTSEARCHSUM])
    report_strpairlist(op->intsearchengines, "Internal search engines",
		       "arg=");
  for (i = 0; i < ITEM_NUMBER; i++)
    report_alias(op->aliashead[i], item_type[i], "");
  report_lowmem(op->miscopts.lowmem);
#ifndef NODNS
  report_dns();
#endif
  printf("Filenames are case %s\n",
	 (op->miscopts.case_insensitive)?"insensitive":"sensitive");
  if (op->miscopts.dirsuffix[0] == '\0')
    printf("No DIRSUFFIX\n");
  else
    printf("DIRSUFFIX %s\n", op->miscopts.dirsuffix);
  report_outopts(&(op->outopts));
}

void report_compile(void) {
  char *temps;

  printf("Compile-time optional code:\n");
#ifdef NOGRAPHICS
  printf("  No pie charts\n");
#else
#ifdef HAVE_GD
  printf("  Pie chart code included -- using your libgd\n");
#else
  printf("  Pie chart code included\n");
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
#ifdef NOPIPES
  printf("  No pipes [so no UNCOMPRESS command]\n");
#else
  printf("  Pipes included\n");
#endif
#ifdef NODNS
  printf("  No DNS lookups\n");
#else
  printf("  DNS lookup code included\n");
#ifdef NOALARM
  printf("  No DNS timeouts\n");
#else
  printf("  DNS timeouts possible\n");
#endif
#endif /* !NODNS */
#ifdef NODIRENT
  printf("  No dirent [so no wildcards in logfile names]\n");
#else
#ifdef MACDIRENT
  printf("  Mac dirent included\n");
#else
#ifdef VMSDIRENT
  printf("  VMS dirent included\n");
#else
#ifdef WIN32DIRENT
  printf("  Win32 dirent included\n");
#else
#ifdef RISCOSDIRENT
  printf("  RISC OS dirent included\n");
#else
#ifdef NOGLOB
  printf("  POSIX.1 dirent included\n");
#else
  printf("  POSIX.2 globbing included\n");
#endif
#endif
#endif
#endif
#endif
#endif
#ifdef NOOPEN
  printf("  No open() function\n");
#else
  printf("  Using open() function\n");
#endif
#ifdef EBCDIC
  printf("  EBCDIC character set on ");
#if defined(BS2000)
  printf("BS2000/OSD\n");
#elif defined(AS400)
  printf("AS/400\n");
#elif defined(OS390)
  printf("OS/390\n");
#else
  printf("unknown platform\n");
#endif
#endif
#ifdef NEED_STRCMP
  printf("  My strcmp()\n");
#endif
#ifdef NEED_MEMMOVE
  printf("  My memmove()\n");
#endif
#ifdef NEED_STRTOUL
  printf("  My strtoul()\n");
#endif
#ifdef NEED_DIFFTIME
  printf("  My difftime()\n");
#endif
#ifdef NEED_FLOATINGPOINT_H
  printf("  <floatingpoint.h> header file\n");
#endif
  printf("Compile-time variables:\n");
  if (strcaseeq(DEFAULTCONFIGFILE, "none") || IS_STDIN(DEFAULTCONFIGFILE))
    temps = DEFAULTCONFIGFILE;
  else
    temps = buildfilename(CONFIGDIR, "", DEFAULTCONFIGFILE);
  printf("  Default configuration file: %s\n", temps);
  if (!strcaseeq(DEFAULTCONFIGFILE, "none"))
    check_file(temps);
  if (strcaseeq(MANDATORYCONFIGFILE, "none") || IS_STDIN(MANDATORYCONFIGFILE))
    temps = MANDATORYCONFIGFILE;
  else
    temps = buildfilename(CONFIGDIR, "", MANDATORYCONFIGFILE);
  printf("  Madatory configuration file: %s\n", temps);
  if (!strcaseeq(MANDATORYCONFIGFILE, "none"))
    check_file(temps);
}

void report_conffiles(Strlist *list) {
  printf("Configuration files read:\n");
  if (list == NULL)
    printf("  none\n");
  for ( ; list != NULL; TO_NEXT(list)) {
    if (IS_STDIN(list->name))
      printf("  standard input\n");
    else
      printf("  %s\n", list->name);
  }
}

void report_debug(char *type, char *arg, char *fullset) {
  printf("%s on: ", type);
  if (strpbrk(fullset, arg) == NULL)
    printf("none\n");
  else if (STREQ(arg, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"))
    printf("all\n");
  /* See configdebug() in init2.c. 'All' should really mean 'including the
     whole of fullset' so it can still sometimes fail this test and be spelled
     out below instead, but that's not a problem. */
  else {
    for ( ; *arg != '\0'; arg++) {
      if (strchr(fullset, *arg) != NULL)
	printf("%c", *arg);
    }
    putchar('\n');
  }
}

void report_logfiles(Logfile *logfile, logical cache) {
  Logfile *lf;

  printf("Reading %sfiles:\n", cache?"cache ":"log");
  if (logfile == NULL)
    printf("  none\n");
  for (lf = logfile; lf != NULL; TO_NEXT(lf)) {
    printf("  %s\n", lf->name);
    check_file(lf->name);
    if (!cache) {
      report_logformat(stdout, lf->format, FALSE);
      if (lf->tz != 0)
	printf("    Times in logfile offset by %s%d minutes\n",
		     (lf->tz > 0)?"+":"", lf->tz);
    }
  }
}

void report_logformat(FILE *outf, Inputformatlist *format, logical inwarn) {
  Inputformatlist *fp;
  Inputformat *p;

  if (inwarn)
    fprintf(outf, "    Current logfile format:\n");
  else
    fprintf(outf, "    Logfile format:\n");
  for (fp = format; fp != NULL; TO_NEXT(fp)) {
    fprintf(outf, "      ");
    for (p = fp->form; p->inpfns != NULL; TO_NEXT(p)) {
      if (p->inpfns->code == 'x') {
	if (p->sep == '0' || p->sep == '1')
	  fprintf(outf, "<Automatic detection>");
	else if (p->sep == '2')
	  fprintf(outf, "<WebSTAR format string>");
	else if (p->sep == '3')
	  fprintf(outf, "<W3 extended format string>");
	else if (p->sep == '4')
	  fprintf(outf, "<Netscape format string>");
      }
      else if (p->inpfns->code == '\n')
	fprintf(outf, "\\n");
      else if (p->inpfns->code != '\0')
	fprintf(outf, "%%%c", p->inpfns->code);
      if (p->inpfns->code == 'r' || p->inpfns->code == 'R' ||
	  p->inpfns->code == 'u' || p->inpfns->code == 'f' ||
	  p->inpfns->code == 'B' || p->inpfns->code == 'S' ||
	  p->inpfns->code == 'F' || p->inpfns->code == 'q' ||
	  p->inpfns->code == 'A' || p->inpfns->code == 's' ||
	  p->inpfns->code == 'C' || p->inpfns->code == 'j' ||
	  p->inpfns->code == 'v' || p->inpfns->code == '\0') {
	if (p->sep == '\t')
	  fprintf(outf, "\\t");
	else if (p->sep == '\n')
	  fprintf(outf, "\\n");
	else if (p->sep == '\\')
	  fprintf(outf, "\\\\");
	else if (p->sep == '%')
	  fprintf(outf, "%%%%");
	else if (p->sep != WHITESPACE)   /* whitespace has following %w */
	  putc(p->sep, outf);
      }
    }
    putc('\n', outf);
  }
}

void report_fromto(Dateman *dman) {
  extern char *engmonths[];

  unsigned int date, month, year;
  if (dman->from == FIRST_TIME && dman->to == LAST_TIME)
    printf("Analysing all dates\n");
  else {
    printf("Analysing dates");
    if (dman->from != FIRST_TIME) {
      code2date(dman->from / 1440, &date, &month, &year);
      printf(" from %02d/%s/%d at %02d:%02d", date, engmonths[month], year,
	     (int)((dman->from % 1440) / 60), (int)(dman->from % 60));
    }
    if (dman->to != LAST_TIME) {
      code2date(dman->to / 1440, &date, &month, &year);
      printf(" to %02d/%s/%d at %02d:%02d", date, engmonths[month], year,
	     (int)((dman->to % 1440) / 60), (int)(dman->to % 60));
    }
    putchar('\n');
  }
}

void report_want(Include *wanthead, char *type, char *indent) {
  static char *pre0 = "";
  static char *pre1 = "REGEXP:";
  static char *pre2 = "REGEXPI:";
  static char *blanks = "[blanks]";

  char *pre, *name;
  Include *wp, *lwp, *nextwp;

  if (wanthead != NULL) {
    /* We first have to turn all the links round to get the list in the right
       order. But we won't use them again. */
    for (wp = wanthead, lwp = NULL; wp != NULL; wp = nextwp) {
      nextwp = wp->next;
      wp->next = lwp;
      lwp = wp;
    }
    /* after this, lwp is the new head */
    printf("%sIncluding (+) and excluding (-) the following %s:\n",
	   indent, type);
    printf("%s  All %scluded, then\n", indent, IS_INC(lwp->type)?"ex":"in");
    for (wp = lwp; wp != NULL; TO_NEXT(wp)) {
      name = wp->name;
      pre = pre0;
      if (IS_REGEXT(wp->type)) {
	if (name[6] == ':') {
	  pre = pre1;
	  name += 7;
	}
	else {
	  pre = pre2;
	  name += 8;
	}
      }
      else if (IS_EMPTY_STRING(name))
	name = blanks;
      printf("%s  %c %s%s\n", indent, IS_INC(wp->type)?'+':'-', pre, name);
    }
  }
}

void report_scwant(choice *code2type) {
  int i, j;
  logical f;

  for (i = MIN_SC, f = FALSE; i < SC_NUMBER && !f; i++) {
    if (code2type[i] == UNWANTED)
      f = TRUE;
  }
  if (f) {
    printf("Including only the following status codes:\n  ");
    for (i = MIN_SC, j = 0; i <= SC_NUMBER; i++) {
      if (j != 0 && (i == SC_NUMBER || code2type[i] == UNWANTED)) {
	if (f)  /* reusing f for first printing */
	  f = FALSE;
	else
	  printf(",");
	if (i == j + 1)
	  printf("%d", j);
	else
	  printf("%d-%d", j, i - 1);
	j = 0;
      }
      else if (j == 0 && i != SC_NUMBER && code2type[i] != UNWANTED)
	j = i;
    }
    if (f)
      printf("none");
    printf("\n");
  }
}

void report_alias(Alias *aliashead, char *type, char *indent) {
  static char *pre0 = "";
  static char *pre1 = "REGEXP:";
  static char *pre2 = "REGEXPI:";

  char *pre, *from;
  Alias *ap;
  AliasTo *tp;

  if (aliashead != NULL) {
    printf("%sAliasing %s as follows:\n", indent, type);
    for (ap = aliashead; ap != NULL; TO_NEXT(ap)) {
      from = ap->from;
      pre = pre0;
      if (ap->isregex) {
	if (from[6] == ':') {
	  pre = pre1;
	  from += 7;
	}
	else {
	  pre = pre2;
	  from += 8;
	}
      }
      printf("%s  %s%s -> ", indent, pre, from);
      for (tp = ap->to; tp != NULL; TO_NEXT(tp)) {
	printf("%s", tp->string);
	if (tp->after >= 0)
	  printf("$%d", (tp->after) / 2 + !(ap->isregex));
	/* internal representation for !regex is one out */
      }
      putchar('\n');
    }
  }
}

void report_strpairlist(Strpairlist *head, char *type, char *connector) {
  Strpairlist *ap;

  if (head != NULL) {
    printf("%s as follows:\n", type);
    for (ap = head; ap != NULL; TO_NEXT(ap))
      printf("  %s %s%s\n", ap->name, connector, ap->data);
  }
}

void report_lowmem(choice lowmem[]) {
  extern char *item_type[];
  int i, j = 0;

  for (i = 0; i < ITEM_NUMBER; i++)
    j += (int)(lowmem[i]);
  if (j == 0)
    printf("No LOWMEM commands in operation\n");
  else {
    printf("LOWMEM settings are:\n");
    for (i = 0; i < ITEM_NUMBER; i++)
      printf("  %s: %d\n", item_type[i], lowmem[i]);
  }
}

#ifndef NODNS
void report_dns(void) {
  extern choice dnslevel;
  extern char *dnsfile, *dnslockfile;
  extern unsigned int dnsgoodhrs, dnsbadhrs;
#ifndef NOALARM
  extern unsigned int dnstimeout;
#endif

  printf("DNS level is ");
  if (dnslevel == DNS_NONE)
    printf("NONE\n");
  else if (dnslevel == DNS_READ)
    printf("READ\n");
  else if (dnslevel == DNS_LOOKUP)
    printf("LOOKUP\n");
  else
    printf("WRITE\n");
  if (dnslevel != DNS_NONE) {
    printf("DNS file is %s\n", dnsfile);
    if (dnslevel == DNS_WRITE)
      printf("DNS lock file is %s\n", dnslockfile);
    printf("Resolved DNS entries rechecked if older than %u hours\n",
	   dnsgoodhrs);
    printf("Unresolved DNS entries rechecked if older than %u hours\n",
	   dnsbadhrs);
#ifndef NOALARM
    if (dnslevel >= DNS_LOOKUP) {
      if (dnstimeout > 0)
	printf("Abort DNS lookups after %u seconds\n", dnstimeout);
      else
	printf("No timeout on DNS lookups\n");
    }
#endif
  }
}
#endif

void report_outopts(Outchoices *od) {
  extern char *repname[];
  extern char *englongdays[];
  extern logical cgi;

#ifndef NOGRAPHICS
  unsigned int numcharts = 0;
#endif
  int i;
  choice j;

  if (od->outstyle == OUT_NONE) {
    printf("Output style is NONE\n");
    if (strcaseeq(od->cacheoutfile, "none")) { /* same as below */
      printf("No cache output file\n");
      printf("NB OUTPUT NONE and no cache output file is an error!\n");
    }
    else
      printf("Cache output file is %s\n",
	     IS_STDOUT(od->cacheoutfile)?"stdout":(od->cacheoutfile));
  }
  else {
    printf("Report order is as follows:\n");
    for (i = 0; od->reporder[i] != -1; i++) {
      j = od->reporder[i];
      printf("  %s [%s]\n", repname[j], (od->repq[j])?"ON":"OFF");
      if (od->repq[j]) {
	if (j < DATEREP_NUMBER)
	  report_daterep(od, j);
	else if (j == REP_GENSUM)
	  report_debug("    General Summary lines", od->gensumlines,
		       "BCDEFGHIJKLMN");
	else if (j <= LAST_NORMALREP || j == REP_CODE) {
	  report_genrep(od, j);
#ifndef NOGRAPHICS
	  numcharts += (od->chartby[G(j)] != CHART_NONE);
#endif
	}
	else { /* REP_SIZE || REP_PROCTIME */
	  report_cols(od->cols[j]);
#ifndef NOGRAPHICS
	  if (od->outstyle == HTML || od->outstyle == XHTML)
	    report_chart(od->chartby[G(j)]);
	  numcharts += (od->chartby[G(j)] != CHART_NONE);
#endif
	}
      }
    }

    printf("Output file is %s\n",
	   IS_STDOUT(od->outfile)?"stdout":(od->outfile));
    if (strcaseeq(od->cacheoutfile, "none"))  /* same as above */
      printf("No cache output file\n");
    else
      printf("Cache output file is %s\n",
	     IS_STDOUT(od->cacheoutfile)?"stdout":(od->cacheoutfile));
    /* Don't check writeability because would overwrite */
    printf("Language file is %s\n", od->lang.file);
    check_file(od->lang.file);
    printf("Domains file is %s\n", od->domainsfile);
    check_file(od->domainsfile);
    if (od->outstyle == HTML || od->outstyle == XHTML)
      printf("Go To lines are %s\n",
	     (od->gotos == TRUE)?"on":((od->gotos == FALSE)?"off":
				       "on at top and bottom only"));
    if (!od->descriptions)
      puts("Report descriptions are off");
    else if (od->descfile == NULL)
      puts("Report descriptions are off (no descriptions file)");
    else {
      printf("Report descriptions file is %s\n", od->descfile);
      check_file(od->descfile);
    }
    printf("Report spans are %s\n", (od->repspan)?"ON":"OFF");
    if (od->repspan)
      printf("Report span threshold is %u minutes\n", od->rsthresh);
    printf("Output style is ");
    if (od->outstyle == HTML) {
      printf("HTML\n");
      if (strcaseeq(od->stylesheet, "none"))
	printf("No style sheet\n");
      else
	printf("Style sheet is %s\n", od->stylesheet);
    }
    else if (od->outstyle == XHTML) {
      printf("XHTML\n");
      if (strcaseeq(od->stylesheet, "none"))
	printf("Default (inline) style sheet\n");
      else
	printf("Style sheet is %s\n", od->stylesheet);
      if (strcaseeq(od->stylesheet, "none"))
	printf("No CSS prefix\n");
      else
	printf("CSS prefix is %s\n", od->cssprefix);
    }
    else if (od->outstyle == ASCII)
      printf("ASCII\n");
    else if (od->outstyle == PLAIN)
      printf("PLAIN\n");
    else if (od->outstyle == LATEX)
      printf("LATEX (and PDFLATEX is %s)\n", (od->pdflatex)?"ON":"OFF");
    else {  /* OUT_NONE is reported above */
      printf("COMPUTER\n");
      printf("The field separator is \"%s\"\n", od->compsep);
    }
    if (cgi)
      printf("Including CGI header lines\n");
    if (strcaseeq(od->headerfile, "none"))
      printf("No header file\n");
    else {
      printf("Header file is %s\n", od->headerfile);
      check_file(od->headerfile);
    }
    if (strcaseeq(od->footerfile, "none"))
      printf("No footer file\n");
    else {
      printf("Footer file is %s\n", od->footerfile);
      check_file(od->footerfile);
    }
    printf("Host name for title is %s\n", od->hostname);
    if (od->outstyle == HTML || od->outstyle == XHTML) {
      if (strcaseeq(od->hosturl, "none"))
	printf("Host name unlinked\n");
      else
	printf("Host name linked to %s\n", od->hosturl);
      if (strcaseeq(od->logo, "none"))
	printf("No logo for title line\n");
      else {
	printf("Logo for title line is %s", od->logo);
	if (STREQ(od->logo, "analogo"))
	  printf(".%s", od->pngimages?"png":"gif");
	putchar('\n');
	if (od->outstyle == XHTML) {
	  if (strcaseeq(od->logourl, "none"))
	    printf("Logo unlinked\n");
	  else
	    printf("Logo linked to %s\n", od->logourl);
	}
      }
      printf("URL of image directory is %s\n", od->imagedir);
      printf("Using %s images\n", od->pngimages?"PNG":"GIF");
#ifndef NOGRAPHICS
      if (numcharts != 0) {
	if (!IS_EMPTY_STRING(od->chartdir))  /* Should never be NULL: init.c */
	  printf("URL of pie chart directory is %s\n", od->chartdir);
	printf("Local location of pie chart directory is %s\n",
	       od->localchartdir);
#ifdef HAVE_GD
	printf("Charts in %s format\n", (od->jpegcharts)?"JPEG":"PNG");
#endif
      }
#endif
    }
    else if (od->outstyle == COMPUTER) {
      if (strcaseeq(od->hosturl, "none"))
	printf("No host URL\n");
      else
	printf("Host URL is %s\n", od->hosturl);
    }
    if (od->rawbytes)
      printf("Displaying exact bytes\n");
    else
      printf("Displaying bytes rounded to %u decimal places\n", od->bytesdp);
    if (od->outstyle != COMPUTER) {
      printf("The character for graph plotting is: %c\n", od->markchar);
      report_sep(od->sepchar, "thousands separator");
      report_sep(od->repsepchar, "thousands separator in tables");
      report_sep(od->decpt, "decimal point");
      printf("The page width is %d\n", od->outputter->pagewidth(od));
      printf("The minimum graph width is %d\n", od->mingraphwidth);
    }
    printf("Weeks begin on %s\n", englongdays[od->weekbeginson]);
  }
}

void report_sep(char c, char *type) {
  if (c == (char)UNSET)
    printf("The %s is: [from language file]\n", type);
  else if (c == ' ')
    printf("The %s is: [space]\n", type);
  else if (c == '\0')
    printf("The %s is: [none]\n", type);
  else
    printf("The %s is: %c\n", type, c);
}

void report_daterep(Outchoices *od, choice j) {
  report_cols(od->cols[j]);
  if (j < DATEREPORTS_NUMBER) {
    printf("    Maximum number of rows: ");
    if (od->rows[j] == 0)
      printf("unlimited\n");
    else
      printf("%u\n", od->rows[j]);
  }
  printf("    Graphing by ");
  if (od->graph[j] == 'R' || od->graph[j] == 'r')
    printf("requests\n");
  else if (od->graph[j] == 'P' || od->graph[j] == 'p')
    printf("requests for pages\n");
  else
    printf("bytes\n");
  if (j < DATEREPORTS_NUMBER)
    printf("    Displaying %s dates at top\n",
	   (od->back[j])?"latest":"earliest");
}

void report_genrep(Outchoices *od, choice j) {
  report_cols(od->cols[j]);
  report_sortby(od->sortby[G(j)], FALSE);
  report_floor(&(od->floor[G(j)]), FALSE);
  if (od->subsortby[G(j)] != UNSET) {  /* report is hierarchical */
    report_sortby(od->subsortby[G(j)], TRUE);
    report_floor(&(od->subfloor[G(j)]), TRUE);
  }
#ifndef NOGRAPHICS
  if (od->outstyle == HTML || od->outstyle == XHTML)
    report_chart(od->chartby[G(j)]);
#endif
  report_want(od->wanthead[G(j)], "in the report", "    ");
  report_alias(od->aliashead[G(j)], "in the report", "    ");
  report_want(od->link[G(j)], "links in the report", "    ");
}

void report_cols(choice *cols) {
  choice c;

  printf("    Columns:");
  if (cols[0] == COL_NUMBER)
    printf(" <none>\n");
  else for (c = 0; cols[c] != COL_NUMBER; c++) {
    switch (cols[c]) {
    case COL_REQS:
      printf(" #requests");
      break;
    case COL_REQS7:
      printf(" #7-day-requests");
      break;
    case COL_PREQS:
      printf(" %%requests");
      break;
    case COL_PREQS7:
      printf(" %%7-day-requests");
      break;
    case COL_PAGES:
      printf(" #pages");
      break;
    case COL_PAGES7:
      printf(" #7-day-pages");
      break;
    case COL_PPAGES:
      printf(" %%pages");
      break;
    case COL_PPAGES7:
      printf(" %%7-day-pages");
      break;
    case COL_BYTES:
      printf(" #bytes");
      break;
    case COL_BYTES7:
      printf(" #7-day-bytes");
      break;
    case COL_PBYTES:
      printf(" %%bytes");
      break;
    case COL_PBYTES7:
      printf(" %%7-day-bytes");
      break;
    case COL_DATE:
      printf(" last-date");
      break;
    case COL_FIRSTD:
      printf(" first-date");
      break;
    case COL_TIME:
      printf(" last-time");
      break;
    case COL_FIRSTT:
      printf(" first-time");
      break;
    case COL_INDEX:
      printf(" number");
      break;
    }
  }
  putchar('\n');
}

void report_sortby(choice sortby, logical sub) {
  if (sub)
    printf("    Sorting sub-items ");
  else
    printf("    Sorting ");
  if (sortby == ALPHABETICAL)
    printf("alphabetically\n");
  else if (sortby == RANDOM)
    printf("randomly\n");
  else if (sortby == REQUESTS)
    printf("by number of requests\n");
  else if (sortby == REQUESTS7)
    printf("by number of requests in the last 7 days\n");
  else if (sortby == PAGES)
    printf("by number of requests for pages\n");
  else if (sortby == PAGES7)
    printf("by number of requests for pages in the last 7 days\n");
  else if (sortby == DATESORT)
    printf("by date of last request\n");
  else if (sortby == FIRSTDATE)
    printf("by date of first request\n");
  else if (sortby == BYTES)
    printf("by number of bytes\n");
  else /* sortby == BYTES7 */
    printf("by number of bytes in the last 7 days\n");
}

void report_floor(Floor *floor, logical sub) {
  /* this is copied (simplified) from whatincluded() in output2.c */
  unsigned long temp;
  timecode_t tempd;
  char *datefmt = "%d/%m/%y at %H:%n";

  if (sub)
    printf("    Floor for sub-items is: ");
  else
    printf("    Floor is: ");

  if (floor->min < 0) {
    temp = (unsigned long)(-floor->min + EPSILON);
    printf("Top %lu item%s by ", temp, (temp == 1)?"":"s");
    if (floor->floorby == REQUESTS)
      printf("number of requests\n");
    else if (floor->floorby == REQUESTS7)
      printf("number of requests in the last 7 days\n");
    else if (floor->floorby == DATESORT)
      printf("date of last request\n");
    else if (floor->floorby == FIRSTDATE)
      printf("date of first request\n");
    else if (floor->floorby == PAGES)
      printf("number of requests for pages\n");
    else if (floor->floorby == PAGES7)
      printf("number of requests for pages in the last 7 days\n");
    else if (floor->floorby == BYTES)
      printf("number of bytes\n");
    else /* floor->floorby == BYTES7 */
      printf("number of bytes in the last 7 days\n");
  }
  else {   /* floor->min >= 0 */
    if (floor->floorby == DATESORT || floor->floorby == FIRSTDATE) {
      tempd = (timecode_t)(floor->min + EPSILON);
      if (floor->floorby == DATESORT)
	printf("items with requests since ");
      else
	printf("items with first request since ");
      printf("%s", timesprintf(NULL, datefmt, tempd, FALSE));
    }
    else if (floor->min > EPSILON) {
      if (floor->qual == '\0') {
	temp = (unsigned long)(floor->min + EPSILON);
	printf("%lu ", temp);
	if (floor->floorby == REQUESTS)
	  printf((temp == 1)?"request\n":"requests\n");
	else if (floor->floorby == REQUESTS7)
	  printf("%s in the last 7 days\n", (temp == 1)?"request":"requests");
	else if (floor->floorby == PAGES)
	  printf((temp == 1)?"request for a page\n":"requests for pages\n");
	else if (floor->floorby == PAGES7)
	  printf("%s in the last 7 days\n",
		 (temp == 1)?"request for a page":"requests for pages");
	else if (floor->floorby == BYTES)
	  printf((temp == 1)?"byte\n":"bytes\n");
	else /* floor->floorby == BYTES7 */
	  printf("%s in the last 7 days\n", (temp == 1)?"byte":"bytes");
      }
      else {  /* floor->qual != '\0' */
	plain_printdouble(stdout, NULL, floor->min);
	if (floor->qual == '%') {
	  if (floor->floorby == REQUESTS)
	    printf("%% of the requests\n");
	  else if (floor->floorby == REQUESTS7)
	    printf("%% of the requests in the last 7 days\n");
	  else if (floor->floorby == PAGES)
	    printf("%% of the requests for pages\n");
	  else if (floor->floorby == PAGES7)
	    printf("%% of the requests for pages in the last 7 days\n");
	  else if (floor->floorby == BYTES)
	    printf("%% of the bytes\n");
	  else /* floor->floorby == BYTES7 */
	    printf("%% of the bytes in the last 7 days\n");
	}
	else if (floor->qual == ':') {
	  if (floor->floorby == REQUESTS)
	    printf("%% of the maximum number of requests\n");
	  else if (floor->floorby == REQUESTS7)
	    printf("%% of the maximum number of requests in the last 7 days\n");
	  else if (floor->floorby == PAGES)
	    printf("%% of the maximum number of requests for pages\n");
	  else if (floor->floorby == PAGES7)
	    printf("%% of the maximum number of requests for pages in the last 7 days\n");
	  else if (floor->floorby == BYTES)
	    printf("%% of the maximum number of bytes\n");
	  else if (floor->floorby == BYTES7)
	    printf("%% of the maximum number of bytes in the last 7 days\n");
	}
	else /* if qual is anything else, must be (k|M|G|T)bytes[7] */
	  printf(" %cbytes%s\n", floor->qual,
		 (floor->floorby == BYTES)?"":" in the last 7 days");
      }   /* end floor->qual != '\0' */
    }     /* end floor->min > EPSILON */
    else
      printf("[all items]\n");
  }       /* end floor->min > 0 */
}

#ifndef NOGRAPHICS
void report_chart(choice chartby) {
  if (chartby == CHART_NONE) {
    printf("    No pie chart\n");
    return;
  }
  printf("    Plotting pie chart ");
  if (chartby == REQUESTS)
    printf("by number of requests\n");
  else if (chartby == REQUESTS7)
    printf("by number of requests in the last 7 days\n");
  else if (chartby == PAGES)
    printf("by number of requests for pages\n");
  else if (chartby == PAGES7)
    printf("by number of requests for pages in the last 7 days\n");
  else if (chartby == BYTES)
    printf("by number of bytes\n");
  else /* chartby == BYTES7 */
    printf("by number of bytes in the last 7 days\n");
}
#endif

void check_file(char *name) {
  FILE *f = NULL;

  if (!IS_STDIN(name) && (f = FOPENR(name)) == NULL)
    printf("    Warning: cannot open that file\n");
  if (f != NULL)
    fclose(f);
}
