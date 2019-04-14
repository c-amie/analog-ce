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

/*** init.c; initialisation routines ***/
/* See also init2.c and globals.c */

#include "anlghea3.h"

void initialise(int argc, char *argv[], Options *op) {
#ifdef MAC_EVENTS
  MacInit(&argc, &argv);
#endif
#ifdef WIN32
  Win32Init();
#endif
  globals(argv[0]);
  defaults(op);
  settings(op, argc, argv);
  correct(op);
  finalinit(op);
}

void confline(Options *op, char *cmd, char *arg1, char *arg2, int rc) {
  extern Options opts;
  extern Configfns cf[];
  extern char *pos;

  char u[265];   /* see nextconfline() and uses of u below */
  char *savepos;
  int i;
  logical done = FALSE;

  strtoupper(cmd);
  if (rc >= 2 && (STREQ(cmd, "SUBDOMAIN") || STREQ(cmd, "SUBDOM"))) {
    /* ugly but easy */
    if (strchr(arg1, '*') != NULL)
      warn('C', TRUE, "Can't have * in first of two arguments to SUBDOMAIN: "
	   "ignoring second argument");
    else {
      sprintf(u, "PLAIN:%s (%s)", arg1, arg2);
      configalias((void *)&(op->outopts.aliashead[G(REP_DOM)]), "SUBDOMAIN",
		  arg1, u, -1);
    }
    rc--;  /* to avoid error message */
  }
  for (i = 0; cf[i].fn != NULL && !done; i++) {
    if (STREQ(cmd, cf[i].name)) {
      /* I think memcpy is OK, but in case we call it recursively one day... */
      memmove((void *)&opts, (void *)op, sizeof(Options));
      cf[i].fn(cf[i].opt, cmd, arg1, arg2, rc);
      memmove((void *)op, (void *)&opts, sizeof(Options));
      if (cf[i].fn == &configcall && rc != '\0' && !IS_EMPTY_STRING(arg1)) {
	savepos = pos;
	strcpy(u, arg1); /* or name will get obliterated before fclose */
	(void)config(u, op, FALSE);
	pos = savepos;
      }
      done = TRUE;
    }
  }
  if (!done)
    unknownwarn(cmd, arg1, arg2);
}

choice config(char *filename, Options *op, logical fromcommandline) {
  extern Inputformatlist *logformat;
  extern char *pos;

  static int no_confs = 0;
  FILE *f;
  char *fullname, *cmd, *arg1, *arg2;
  int rc;

  if (fromcommandline || IS_STDIN(filename))
    fullname = filename;
  else
    fullname = buildfilename(CONFIGDIR, "", filename);
  if ((f = my_fopen(fullname, "configuration file")) == NULL)
    return(ERR);
  if (no_confs++ >= MAX_CONFIGS)
    error("Attempted to read more than %d configuration files", MAX_CONFIGS);
  configstrlist((void *)&(op->conffilelist), "", fullname, NULL, -1);
  pos = NULL;
  while ((rc = nextconfline(f, &cmd, &arg1, &arg2)) != EOF)
    confline(op, cmd, arg1, arg2, rc);
  (void)my_fclose(f, fullname, "configuration file");
  if (!(logformat->used))
    warn('D', TRUE,
	 "LOGFORMAT in configuration file %s with no subsequent LOGFILE",
	 filename);
  configlogfmt((void *)&logformat, "LOGFORMAT", "DEFAULT", NULL, -1);
  return(OK);
}

void settings(Options *op, int argc, char *argv[]) {
  extern Inputformatlist *logformat;
  extern logical newloglist;

  int i;
  logical done;

  /* once through command line arguments to see if we just want help */
  for (i = 0; i < argc; i++) {
    if (STREQ(argv[i], "-help") || STREQ(argv[i], "-version") ||
	STREQ(argv[i], "--help") || STREQ(argv[i], "--version")) {
      fprintf(stderr, "This is analog version %s\n", VERSION);
      fprintf(stderr, "For help see docs/Readme.html, "
#ifdef UNIX
	      "or man analog, "
#endif
	      "or %s\n", ANALOGURL);
      my_exit(EXIT_SUCCESS);
    }
  }

  /* once through command line arguments to see if default config wanted */
  newloglist = TRUE;
  if (!strcaseeq(DEFAULTCONFIGFILE, "none")) {
    for (i = argc - 1, done = FALSE; i >= 1 && !done; i--) {
      if (!IS_EMPTY_STRING(argv[i]) && argv[i][1] == 'G' &&
	  (argv[i][0] == '+' || argv[i][0] == '-')) {
	done = TRUE;
	CLLONGCHECK(if (argv[i][0] == '+') (void)config(DEFAULTCONFIGFILE, op, FALSE);)
      }
    }
    if (!done)
      (void)config(DEFAULTCONFIGFILE, op, FALSE);
  }

  /* now read in rest of command line arguments */
  newloglist = TRUE;
  clargs(op, argc, argv);
  if (!(logformat->used))
    warn('D', TRUE, "LOGFORMAT on command line with no subsequent logfile");
  configlogfmt((void *)&logformat, "LOGFORMAT", "DEFAULT", NULL, -1);

  /* finally, read in mandatory config file, aborting if not found */
  newloglist = TRUE;
  if (!strcaseeq(MANDATORYCONFIGFILE, "none") &&
      config(MANDATORYCONFIGFILE, op, FALSE) == ERR)
    error("Cannot ignore mandatory configuration file");
}

Inputformatlist *correctlogfmt(Logfile *lp, Include **wanthead,
			       choice *code2type, Dateman *dman) {
  /* Here's a summary of what happens, minus several bells and whistles. At
     logfile parse time (strtoinfmt() below), fmt->count[ITEM] was set to 2 for
     counted items, 1 for starred items. Now if the item isn't wanted, its
     count is reduced to 1. Then if an item with count 1 is not filtered, its
     count is reduced to 0 and its %x is changed to %j. Filenames are special
     because whether a lot of other things get counted depend on the presence
     of a filename. If there are no items left with count 2, the whole logfile
     is ignored. */
  extern choice wantitem[];
  extern Inputfns inpfns[], pjinpfn;
  extern char *item_type[];

  Inputformatlist *fmt;
  Inputformat *fns;
  logical noitem[ITEM_NUMBER], nodates, nocodes, nonames, nobytes;
  int count, i, j;

  for (count = 0, fmt = lp->format; fmt != NULL; TO_NEXT(fmt)) {
    if (!wantitem[INP_CODE] && fmt->count[INP_CODE] == 2)
      fmt->count[INP_CODE] = 1;
    if (!wantitem[INP_DATE] && fmt->count[INP_DATE] == 2)
      fmt->count[INP_DATE] = 1;  /* NB read date even if not filtered */
    for (fns = fmt->form; fns->inpfns != NULL; TO_NEXT(fns)) {
      for (i = 0; i < ITEMFNS_NUMBER; i++) {
	if (fns->inpfns == &inpfns[i]) {
	  j = inpfns[i].type;
	  if (!wantitem[j] && fmt->count[j] == 2 &&
	      !(j == ITEM_FILE && (fmt->count[INP_DATE] == 2 ||
				   fmt->count[INP_CODE] > 0 ||
				   fmt->count[INP_BYTES] > 0 ||
				   fmt->count[INP_PROCTIME] > 0)))
	    fmt->count[j] = 1;
	  if (wanthead[j] == NULL && fmt->count[j] == 1 && j != ITEM_FILE &&
	      !(j == ITEM_VHOST && lp->pvpos != UNSET)) {
	    if (inpfns[i].fn != &parseref)
	      fns->inpfns = &pjinpfn;
	    fmt->count[j] = 0;
	  }
	  count += (int)(fmt->count[j] == 2);
	}
      }     /* end for i through ITEMFNS */
      if (fns->inpfns->fn == &parselogfmt) {
	count++;
	for (i = 0; i < INPUT_NUMBER; i++)
	  fmt->count[i] = 1;   /* just so doesn't trigger warnings below */
      }
    }       /* end for fns in fmt */
  }         /* end for fmt through formats */

  if (count == 0)
    lp->format = NULL;  /* i.e., mark logfile to be ignored */
  else {
    for (i = 0; i < ITEM_NUMBER; i++)
      noitem[i] = FALSE;
    nodates = FALSE;
    nocodes = FALSE;
    nonames = FALSE;
    nobytes = FALSE;
    for (fmt = lp->format; fmt != NULL; TO_NEXT(fmt)) {
      for (count = 0, j = 0; j < INPUT_NUMBER; j++)
	count += (int)(fmt->count[j] == 2);
      if (count > 0) {
	for (i = 0; i < ITEM_NUMBER; i++) {
	  if (fmt->count[i] == 0)
	    noitem[i] = TRUE;
	}
	if (fmt->count[INP_DATE] == 0)
	  nodates = TRUE;
	if (fmt->count[INP_CODE] == 0)
	  nocodes = TRUE;
	if (fmt->count[ITEM_FILE] == 0)
	  nonames = TRUE;
	if (fmt->count[INP_BYTES] == 0)
	  nobytes = TRUE;
      }
    }
    for (i = 0; i < ITEM_NUMBER; i++) {
      if (wanthead[i] != NULL && noitem[i])
	warn('M', TRUE, "Logfile %s contains lines with no %s, which are "
	     "being filtered", lp->name, item_type[i]);
    }
    if (code2type[0] != UNSET && nocodes)
      warn('M', TRUE, "Logfile %s contains lines with no status codes, "
	   "which are being filtered", lp->name);
    if ((dman->from > FIRST_TIME || dman->to < LAST_TIME) && nodates)
      warn('M', TRUE, "Logfile %s contains lines with no dates, which are "
	   "being filtered", lp->name);
    if (nonames)
      warn('M', TRUE, "Logfile %s contains lines with no file names: "
	   "page counts may be low", lp->name);
    if (nobytes)
      warn('M', TRUE, "Logfile %s contains lines with no bytes: byte counts "
	   "may be low", lp->name);
  }
  return(lp->format);
}

void correct(Options *op) {
  extern time_t origstarttime, starttime;
  extern timecode_t starttimec;
  extern int stz;
  extern Inputformatlist *deflogformat;
  extern char *repname[], *methodname[];
  extern unsigned int *rep2gran;
  extern choice *rep2type, *rep2reqs;
  extern choice wantitem[];
  extern logical vblesonly;

  Outchoices *od = &(op->outopts);
  choice outstyle = od->outstyle;

  char *sf = "SUBFLOOR";
  char *af = "ARGSFLOOR";
  char *ss = "SUBSORTBY";
  char *as = "ARGSSORTBY";
  Logfile *lp;
  Include *incp, *lastincp;
  choice rep, *cols, floor, subfloor, sortby, subsortby, chartby;
  char *subf, *subs, *c;
  logical colsinc[COL_NUMBER], istree, templ;
  char graph;
  int i, j, k;

  /* NB some options, e.g. reportorder, corrected when parsed */
  origstarttime = starttime;
  starttime = shifttime(starttime, stz);
  starttimec += stz;
  if (od->markchar == '\0') {
    warn('C', TRUE, "MARKCHAR none not allowed. Using + instead.");
    od->markchar = '+';
  }
  if (od->decpt == '\0') {
    warn('C', TRUE, "DECPOINT none not allowed. "
	 "Getting it from language file instead.");
    od->decpt = (char)UNSET;  /* and will be read from lang file */
  }
  if (od->bytesdp > 6) {
    warn('C', TRUE, "The maximum allowed value of BYTESDP is 6. "
	 "For exact numbers, use RAWBYTES ON");
    od->bytesdp = 6;
  }
  op->dman.from = FIRST_TIME;
  if (op->dman.fromstr != NULL &&  /* so parse unless fromstr is NULL */
      parsedate(starttime, op->dman.fromstr, &(op->dman.from), TRUE, FALSE)
      == ERR)
    warn('C', TRUE, "Invalid FROM string %s: ignoring it", op->dman.fromstr);
  op->dman.to = LAST_TIME;
  if (op->dman.tostr != NULL &&
      parsedate(starttime, op->dman.tostr, &(op->dman.to), FALSE, FALSE)
      == ERR)
    warn('C', TRUE, "Invalid TO string %s: ignoring it", op->dman.tostr);
  if (op->dman.from > op->dman.to) {
    warn('C', TRUE, "FROM time is later than TO time: "
	 "would exclude everything so ignoring them");
    op->dman.from = FIRST_TIME;
    op->dman.to = LAST_TIME;
  }
  for (i = 0, lp = op->miscopts.logfile[0]; lp != NULL; TO_NEXT(lp))
    i = MAX(i, lp->tz);
  if (op->dman.from > starttimec + i + 60)  /* one hour's grace */
    warn('D', TRUE, "FROM time is later than the present");
  if (op->dman.to < LAST_TIME) {
    op->dman.last7from = op->dman.to - MINS_IN_WEEK;
    op->dman.last7to = op->dman.to;
  }
  else {
    op->dman.last7from = starttimec - MINS_IN_WEEK;
    op->dman.last7to = starttimec;
  }
  if (outstyle == HTML || outstyle == XHTML) {
    if (od->htmlpagewidth == 0)
      od->htmlpagewidth = 1;
    else if (od->htmlpagewidth > MAXPAGEWIDTH) {
      warn('C', TRUE, "HTMLPAGEWIDTH %u too large: using maximum allowed "
	   "value of %u", od->htmlpagewidth, MAXPAGEWIDTH);
      od->htmlpagewidth = MAXPAGEWIDTH;
    }
  }
  else if (outstyle == ASCII || outstyle == PLAIN) {
    if (od->plainpagewidth == 0)
      od->plainpagewidth = 1;
    else if (od->plainpagewidth > MAXPAGEWIDTH) {
      warn('C', TRUE, "PLAINPAGEWIDTH %u too large: using maximum allowed "
	   "value of %u", od->plainpagewidth, MAXPAGEWIDTH);
      od->plainpagewidth = MAXPAGEWIDTH;
    }
  }
  else if (outstyle == LATEX) {
    if (od->latexpagewidth == 0)
      od->latexpagewidth = 1;
    else if (od->latexpagewidth > MAXPAGEWIDTH) {
      warn('C', TRUE, "LATEXPAGEWIDTH %u too large: using maximum allowed "
	   "value of %u", od->latexpagewidth, MAXPAGEWIDTH);
      od->latexpagewidth = MAXPAGEWIDTH;
    }
  }
  for (lastincp = NULL, incp = op->argshead, templ = FALSE; incp != NULL;
       TO_NEXT(incp)) {
    if (STREQ((char *)(incp->name), "pages")) {
      if (!templ)
	warn('C', TRUE, "ARGSINCLUDE/EXCLUDE can't include 'pages'");
      templ = TRUE;
      if (lastincp == NULL)
	op->argshead = incp->next;
      else
	lastincp->next = incp->next;
    }
    else
      lastincp = incp;
  }
  for (lastincp = NULL, incp = op->refargshead, templ = FALSE; incp != NULL;
       TO_NEXT(incp)) {
    if (STREQ((char *)(incp->name), "pages")) {
      if (!templ)
	warn('C', TRUE, "REFARGSINCLUDE/EXCLUDE can't include 'pages'");
      templ = TRUE;
      if (lastincp == NULL)
	op->refargshead = incp->next;
      else
	lastincp->next = incp->next;
    }
    else
      lastincp = incp;
  }
  for (i = 0, j = 0, k = 0; i < DATEREPORTS_NUMBER; i++) {
    j += (int)(od->repq[i] && od->back[i]);
    k += (int)(od->repq[i]);
  }
  if (j != 0 && j != k)
    warn('D', TRUE, "Time reports have not all got same value of BACK");
  /* The next bit is totally foul, so here's a guide. Numbers are repeated
     in comments below. [C] problems are overridden, [D] are just warned.
    for (i through reports turned on, except GENSUM) {
      if (time report) {                *** (1) ***
        check COLS don't include C, c, D, d, E, e, Q, q, S, s or N   [C]
        check GRAPH included in COLS               [D]
      }
      else (non-time reports) {         *** (2) ***
        if (REP_REQ || REP_TYPE) {             *** (3) ***
          Check P, Q not in COLS or (sub)SORTBY or (sub)FLOOR   [C]
          For REP_TYPE, also check p, q not in COLS or CHART    [C]
        }
        else if (redirection/failure report || REP_CODE) {   *** (4) ***
          Check P, p, Q, q, B, b, C, c not in COLS or (sub)SORTBY or
             (sub)FLOOR or CHART  [C]
        }
        Interpret "CHART SORTBY"               *** (5) ***
        if (REP_SIZE || REP_PROCTIME) {        *** (6) ***
          Check N not in COLS    [C]
        }
        else {                                 *** (7) ***
          Check SORTBY matches SUBSORTBY   [D]
          Check FLOOR matches SUBFLOOR     [D]
          Check SORTBY matches FLOOR       [D]
          Check CHART matches SORTBY       [D]
          Check no column N if SORTBY ALPHABETICAL or RANDOM   [D]
          Check (sub)SORTBY and (sub)FLOOR and CHART in COLS   [D]
        }
      }
      Check COLS non-empty  [D]         *** (8) ***
    }
  */
  for (i = 0; od->reporder[i] != -1; i++) {
    rep = od->reporder[i];
    if (rep != REP_GENSUM && od->repq[rep]) {
      cols = od->cols[rep];
      for (j = 0; j < COL_NUMBER; j++)
	colsinc[j] = FALSE;
      for (j = 0; cols[j] != COL_NUMBER; j++)
	colsinc[cols[j]] = TRUE;
      if (rep < DATEREP_NUMBER) {   /* *** (1) ***   time reports */
	graph = od->graph[rep];
	if (colsinc[COL_FIRSTD] || colsinc[COL_FIRSTT] || colsinc[COL_DATE] ||
	    colsinc[COL_TIME] || colsinc[COL_INDEX] ||
	    colsinc[COL_REQS7] || colsinc[COL_PAGES7] || colsinc[COL_BYTES7] ||
	    colsinc[COL_PREQS7] || colsinc[COL_PPAGES7] ||
	    colsinc[COL_PBYTES7]) {
	  warn('C', TRUE, "In %s, C, c, D, d, E, e, Q, q, S, s and N not "
	       "allowed in COLS: ignoring them", repname[rep]);
	  for (j = 0; cols[j] != COL_NUMBER; j++) {
	    if (cols[j] == COL_FIRSTD || cols[j] == COL_FIRSTT ||
		cols[j] == COL_DATE || cols[j] == COL_TIME ||
		cols[j] == COL_REQS7 || cols[j] == COL_PAGES7 ||
		cols[j] == COL_BYTES7 || cols[j] == COL_PREQS7 ||
		cols[j] == COL_PPAGES7 || cols[j] == COL_PBYTES7 ||
		cols[j] == COL_INDEX) {
	      for (k = j; cols[k] != COL_NUMBER; k++)
		cols[k] = cols[k + 1];
	      j--;
	    }
	  }
	}
	if (cols[0] != COL_NUMBER) {  /* o/wise different warning below */
	  if (((graph == 'R' || graph == 'r') &&
	       !colsinc[COL_REQS] && !colsinc[COL_PREQS]) ||
	      ((graph == 'P' || graph == 'p') &&
	       !colsinc[COL_PAGES] && !colsinc[COL_PPAGES]) ||
	      ((graph == 'B' || graph == 'b') &&
	       !colsinc[COL_BYTES] && !colsinc[COL_PBYTES]))
	    warn('D', TRUE, "In %s, GRAPH (%c) isn't included in COLS",
		 repname[rep], graph);
	}
      }   /* end if time report */
      else {    /* *** (2) *** non-time reports */
	sortby = od->sortby[G(rep)];
	floor = od->floor[G(rep)].floorby;
	subsortby = od->subsortby[G(rep)];
	subfloor = od->subfloor[G(rep)].floorby;
	chartby = (outstyle == HTML || outstyle == XHTML)?\
	  (od->chartby[G(rep)]):CHART_NONE;
	if (rep == REP_REQ || rep == REP_FAIL || rep == REP_REDIR ||
	    rep == REP_REF || rep == REP_FAILREF || rep == REP_REDIRREF) {
	  subf = af;  /* Just to get the error messages right */
	  subs = as;
	}
	else {
	  subf = sf;
	  subs = ss;
	}
	istree = (od->subsortby[G(rep)] != UNSET);
	/* For istree, see comment on initialising SUBSORTBYs in globals.c */
	if (rep == REP_REQ || rep == REP_TYPE) {      /* *** (3) *** */
	  if (rep == REP_REQ) {
	    if (colsinc[COL_PAGES] || colsinc[COL_PAGES7]) {
	      warn('C', TRUE, "In %s, P and Q not allowed in COLS: "
		   "ignoring them", repname[rep]);
	      for (j = 0; cols[j] != COL_NUMBER; j++) {
		if (cols[j] == COL_PAGES || cols[j] == COL_PAGES7) {
		  for (k = j; cols[k] != COL_NUMBER; k++)
		    cols[k] = cols[k + 1];
		  j--;
		}
	      }
	    }
	  }
	  else /* REP_TYPE */if (colsinc[COL_PAGES] || colsinc[COL_PAGES7] ||
				 colsinc[COL_PPAGES] || colsinc[COL_PPAGES7]) {
	    warn('C', TRUE, "In %s, P, p, Q and q not allowed in COLS: "
		 "ignoring them", repname[rep]);
	    for (j = 0; cols[j] != COL_NUMBER; j++) {
	      if (cols[j] == COL_PAGES || cols[j] == COL_PAGES7 ||
		  cols[j] == COL_PPAGES|| cols[j] == COL_PPAGES7) {
		for (k = j; cols[k] != COL_NUMBER; k++)
		  cols[k] = cols[k + 1];
		j--;
	      }
	    }
	  }
	  if (sortby == PAGES || sortby == PAGES7) {
	    warn('C', TRUE, "In %s, illegal SORTBY (%s): will sort by "
		 "requests instead", repname[rep], methodname[sortby]);
	    od->sortby[G(rep)] = (sortby == PAGES)?REQUESTS:REQUESTS7;
	    sortby = (sortby == PAGES)?REQUESTS:REQUESTS7;
	  }
	  if (floor == PAGES || floor == PAGES7) {
	    warn('C', TRUE, "In %s, illegal FLOOR (%s): will use requests "
		 "instead", repname[rep], methodname[floor]);
	    od->floor[G(rep)].floorby = (floor == PAGES)?REQUESTS:REQUESTS7;
	    floor = (floor == PAGES)?REQUESTS:REQUESTS7;
	  }
	  if (istree) {
	    if (subsortby == PAGES || subsortby == PAGES7) {
	      warn('C', TRUE, "In %s, illegal %s (%s): will sort by requests "
		   "instead", repname[rep], subs, methodname[subsortby]);
	      od->subsortby[G(rep)] = (subsortby == PAGES)?REQUESTS:REQUESTS7;
	      subsortby = (subsortby == PAGES)?REQUESTS:REQUESTS7;
	    }
	    if (subfloor == PAGES || subfloor == PAGES7) {
	      warn('C', TRUE, "In %s, illegal %s (%s): will use requests "
		   "instead", repname[rep], subf, methodname[subfloor]);
	      od->subfloor[G(rep)].floorby =
		(subfloor == PAGES)?REQUESTS:REQUESTS7;
	      subfloor = (subfloor == PAGES)?REQUESTS:REQUESTS7;
	    }
	  }
	  if (rep == REP_TYPE && (chartby == PAGES || chartby == PAGES7)) {
	    warn('C', TRUE, "In %s, illegal CHART (%s): will chart by "
		 "requests instead", repname[rep], methodname[chartby]);
	    od->chartby[G(rep)] = REQUESTS;
	    chartby = REQUESTS;
	  }
	}    /* end rep == REP_REQ || rep == REP_TYPE */
	else if (rep2reqs[G(rep)] != REQUESTS || rep == REP_CODE) {
	  /* i.e. redirection/failure or REP_CODE */   /* *** (4) *** */
	  if (colsinc[COL_PAGES] || colsinc[COL_PPAGES] ||
	      colsinc[COL_PAGES7] || colsinc[COL_PPAGES7] ||
	      colsinc[COL_BYTES] || colsinc[COL_PBYTES] ||
	      colsinc[COL_BYTES7] || colsinc[COL_PBYTES7]) {
	    warn('C', TRUE, "In %s, P, p, Q, q B, b, C and c not allowed in "
		 "COLS: ignoring them", repname[rep]);
	    for (j = 0; cols[j] != COL_NUMBER; j++) {
	      if (cols[j] == COL_PAGES || cols[j] == COL_PPAGES ||
		  cols[j] == COL_PAGES7 || cols[j] == COL_PPAGES7 ||
		  cols[j] == COL_BYTES || cols[j] == COL_PBYTES	||
		  cols[j] == COL_BYTES7 || cols[j] == COL_PBYTES7) {
		for (k = j; cols[k] != COL_NUMBER; k++)
		  cols[k] = cols[k + 1];
		j--;
	      }
	    }
	  }
	  if (sortby == PAGES || sortby == PAGES7 || sortby == BYTES ||
	      sortby == BYTES7) {
	    warn('C', TRUE, "In %s, illegal SORTBY (%s): will sort by "
		 "requests instead", repname[rep], methodname[sortby]);
	    od->sortby[G(rep)] = REQUESTS;
	    sortby = REQUESTS;
	  }
	  if (floor == PAGES || floor == PAGES7 || floor == BYTES ||
	      floor == BYTES7) {
	    warn('C', TRUE, "In %s, illegal FLOOR (%s): will use -50r instead",
		 repname[rep], methodname[floor]);
	    od->floor[G(rep)].min = -50;
	    od->floor[G(rep)].qual = '\0';
	    od->floor[G(rep)].floorby = REQUESTS;
	    floor = REQUESTS;
	  }
	  if (chartby == PAGES || chartby == PAGES7 || chartby == BYTES ||
	      chartby == BYTES7) {
	    warn('C', TRUE, "In %s, illegal CHART (%s): will chart by "
		 "requests instead", repname[rep], methodname[chartby]);
	    od->chartby[G(rep)] = REQUESTS;
	    chartby = REQUESTS;
	  }
	  if (istree) {
	    if (subsortby == PAGES || subsortby == PAGES7 ||
		subsortby == BYTES || subsortby == BYTES7) {
	      warn('C', TRUE, "In %s, illegal %s (%s): will sort by requests "
		   "instead", repname[rep], subs, methodname[subsortby]);
	      od->subsortby[G(rep)] = REQUESTS;
	      subsortby = REQUESTS;
	    }
	    if (subfloor == PAGES || subfloor == PAGES7 || subfloor == BYTES ||
		subfloor == BYTES7) {
	      warn('C', TRUE, "In %s, illegal %s (%s): will use -1r instead",
		   repname[rep], subf, methodname[subfloor]);
	      od->subfloor[G(rep)].min = -1;
	      od->subfloor[G(rep)].qual = '\0';
	      od->subfloor[G(rep)].floorby = REQUESTS;
	      subfloor = REQUESTS;
	    }
	  }
	}   /* end redirection/failure || REP_CODE */
	if (chartby == CHART_SORTBY) {                 /* *** (5) *** */
	  if (sortby == REQUESTS || sortby == REQUESTS7 || sortby == PAGES ||
	      sortby == PAGES7 || sortby == BYTES || sortby == BYTES7)
	    od->chartby[G(rep)] = sortby;
	  else if (floor == REQUESTS || floor == REQUESTS7 || floor == PAGES ||
		   floor == PAGES7 || floor == BYTES || floor == BYTES7)
	    od->chartby[G(rep)] = floor;
	  else
	    od->chartby[G(rep)] = REQUESTS;
	  chartby = od->chartby[G(rep)];
	}
	/* "if" not "else if" is correct (to make next "else if" work) */
	if (rep == REP_SIZE || rep == REP_PROCTIME) {  /* *** (6) *** */
	  if (colsinc[COL_INDEX]) {
	    warn('C', TRUE, "In %s, N not allowed in COLS: ignoring it",
		 repname[rep]);
	    for (j = 0; cols[j] != COL_NUMBER; j++) {
	      if (cols[j] == COL_INDEX) {
		for (k = j; cols[k] != COL_NUMBER; k++)
		  cols[k] = cols[k + 1];
		j--;
	      }
	    }
	  }
	}
	else { /* *** (7) *** rep != REP_SIZE && rep != REP_PROCTIME */
	  /* check SORTBYs (SIZE & PROCTIME don't have SORTBY's) */
	  if (istree) {
	    if (sortby != subsortby && subsortby != ALPHABETICAL &&
		subsortby != RANDOM)
	      warn('D', TRUE, "In %s, SORTBY (%s) doesn't match %s (%s)",
		   repname[rep], methodname[sortby], subs,
		   methodname[subsortby]);
	    if (floor != subfloor)
	      warn('D', TRUE, "In %s, FLOOR (%s) doesn't match %s (%s)",
		   repname[rep], methodname[floor], subf,
		   methodname[subfloor]);
	  }
	  if (((sortby == REQUESTS || sortby == REQUESTS7 || sortby == PAGES ||
		sortby == PAGES7 || sortby == BYTES || sortby == BYTES7) &&
	       (floor == REQUESTS || floor == REQUESTS7 || floor == PAGES ||
		floor == PAGES7 || floor == BYTES || floor == BYTES7) &&
	       sortby != floor) ||
	      (sortby == DATESORT && floor == FIRSTDATE) ||
	      (sortby == FIRSTDATE && floor == DATESORT))
	    warn('D', TRUE, "In %s, SORTBY (%s) doesn't match FLOOR (%s)",
		 repname[rep], methodname[sortby], methodname[floor]);
	  if (chartby != CHART_NONE) {
	    if (sortby == REQUESTS || sortby == REQUESTS7 || sortby == PAGES ||
		sortby == PAGES7 || sortby == BYTES || sortby == BYTES7) {
	      if (sortby != chartby) {
		warn('D', TRUE, "In %s, CHART (%s) doesn't match SORTBY (%s)",
		     repname[rep], methodname[chartby], methodname[sortby]);
	      }
	    }
	    else if ((floor == REQUESTS || floor == REQUESTS7 ||
		      floor == PAGES || floor == PAGES7 || floor == BYTES ||
		      floor == BYTES7) && floor != chartby) {
		warn('D', TRUE, "In %s, CHART (%s) doesn't match FLOOR (%s)",
		     repname[rep], methodname[chartby], methodname[floor]);
	    }
	  }
	  if (sortby == ALPHABETICAL && colsinc[COL_INDEX])
	    warn('D', TRUE, "In %s, column N with SORTBY ALPHABETICAL",
		 repname[rep]);
	  else if (sortby == RANDOM && colsinc[COL_INDEX])
	    warn('D', TRUE, "In %s, column N with SORTBY RANDOM",
		 repname[rep]);
	  if (cols[0] != COL_NUMBER) {  /* o/wise different warning below */
	    if ((sortby == REQUESTS && !colsinc[COL_REQS] &&
		 !colsinc[COL_PREQS]) ||
		(sortby == REQUESTS7 && !colsinc[COL_REQS7] &&
		 !colsinc[COL_PREQS7]) ||
		(sortby == PAGES && !colsinc[COL_PAGES] &&
		 !colsinc[COL_PPAGES]) ||
		(sortby == PAGES7 && !colsinc[COL_PAGES7] &&
		 !colsinc[COL_PPAGES7]) ||
		(sortby == BYTES && !colsinc[COL_BYTES] &&
		 !colsinc[COL_PBYTES]) ||
		(sortby == BYTES7 && !colsinc[COL_BYTES7] &&
		 !colsinc[COL_PBYTES7]) ||
		(sortby == DATESORT && !colsinc[COL_DATE] &&
		 !colsinc[COL_TIME]) ||
		(sortby == FIRSTDATE && !colsinc[COL_FIRSTD] &&
		 !colsinc[COL_FIRSTT]))
	      warn('D', TRUE, "In %s, SORTBY (%s) isn't included in COLS",
		   repname[rep], methodname[sortby]);
	    if ((floor == REQUESTS && !colsinc[COL_REQS] &&
		 !colsinc[COL_PREQS]) ||
		(floor == REQUESTS7 && !colsinc[COL_REQS7] &&
		 !colsinc[COL_PREQS7]) ||
		(floor == PAGES && !colsinc[COL_PAGES] &&
		 !colsinc[COL_PPAGES]) ||
		(floor == PAGES7 && !colsinc[COL_PAGES7] &&
		 !colsinc[COL_PPAGES7]) ||
		(floor == BYTES && !colsinc[COL_BYTES] &&
		 !colsinc[COL_PBYTES]) ||
		(floor == BYTES7 && !colsinc[COL_BYTES7] &&
		 !colsinc[COL_PBYTES7]) ||
		(floor == DATESORT && !colsinc[COL_DATE] &&
		 !colsinc[COL_TIME]) ||
		(floor == FIRSTDATE && !colsinc[COL_FIRSTD] &&
		 !colsinc[COL_FIRSTT]))
	      warn('D', TRUE, "In %s, FLOOR (%s) isn't included in COLS",
		   repname[rep], methodname[floor]);
	    if (istree) {
	      if ((subsortby == REQUESTS && !colsinc[COL_REQS] &&
		   !colsinc[COL_PREQS]) ||
		  (subsortby == REQUESTS7 && !colsinc[COL_REQS7] &&
		   !colsinc[COL_PREQS7]) ||
		  (subsortby == PAGES && !colsinc[COL_PAGES] &&
		   !colsinc[COL_PPAGES]) ||
		  (subsortby == PAGES7 && !colsinc[COL_PAGES7] &&
		   !colsinc[COL_PPAGES7]) ||
		  (subsortby == BYTES && !colsinc[COL_BYTES] &&
		   !colsinc[COL_PBYTES]) ||
		  (subsortby == BYTES7 && !colsinc[COL_BYTES7] &&
		   !colsinc[COL_PBYTES7]) ||
		  (subsortby == DATESORT && !colsinc[COL_DATE] &&
		   !colsinc[COL_TIME]) ||
		  (subsortby == FIRSTDATE && !colsinc[COL_FIRSTD] &&
		   !colsinc[COL_FIRSTT]))
		warn('D', TRUE, "In %s, %s (%s) isn't included in COLS",
		     repname[rep], subs, methodname[subsortby]);
	      if ((subfloor == REQUESTS && !colsinc[COL_REQS] &&
		   !colsinc[COL_PREQS]) ||
		  (subfloor == REQUESTS7 && !colsinc[COL_REQS7] &&
		   !colsinc[COL_PREQS7]) ||
		  (subfloor == PAGES && !colsinc[COL_PAGES] &&
		   !colsinc[COL_PPAGES]) ||
		  (subfloor == PAGES7 && !colsinc[COL_PAGES7] &&
		   !colsinc[COL_PPAGES7]) ||
		  (subfloor == BYTES && !colsinc[COL_BYTES] &&
		   !colsinc[COL_PBYTES]) ||
		  (subfloor == BYTES7 && !colsinc[COL_BYTES7] &&
		   !colsinc[COL_PBYTES7]) ||
		  (subfloor == DATESORT && !colsinc[COL_DATE] &&
		   !colsinc[COL_TIME]) ||
		  (subfloor == FIRSTDATE && !colsinc[COL_FIRSTD] &&
		   !colsinc[COL_FIRSTT]))
		warn('D', TRUE, "In %s, %s (%s) isn't included in COLS",
		     repname[rep], subf, methodname[subfloor]);
	    }
	    if ((chartby == REQUESTS && !colsinc[COL_REQS] &&
		 !colsinc[COL_PREQS]) ||
		(chartby == REQUESTS7 && !colsinc[COL_REQS7] &&
		 !colsinc[COL_PREQS7]) ||
		(chartby == PAGES && !colsinc[COL_PAGES] &&
		 !colsinc[COL_PPAGES]) ||
		(chartby == PAGES7 && !colsinc[COL_PAGES7] &&
		 !colsinc[COL_PPAGES7]) ||
		(chartby == BYTES && !colsinc[COL_BYTES] &&
		 !colsinc[COL_PBYTES]) ||
		(chartby == BYTES7 && !colsinc[COL_BYTES7] &&
		 !colsinc[COL_PBYTES7]))
	      warn('D', TRUE, "In %s, CHART (%s) isn't included in COLS",
		   repname[rep], methodname[chartby]);
	  }
	}
      }
      if (cols[0] == COL_NUMBER)   /* *** (8) *** */
	warn('D', TRUE, "%s contains no COLS", repname[rep]);
    }
  }  /* end for i through reports */
  /* change logformats to ignore items which are not wanted (see 28/10/97) */
  for (lp = op->miscopts.logfile[0]; lp != NULL; TO_NEXT(lp)) {
    if (lp->format->form->inpfns->fn == &parselogfmt &&
	lp->format->form->sep == '0')  /* DEFAULT format */
      lp->format = deflogformat;
  }
  if (!vblesonly) {
    if (STREQ(od->cssprefix, "none"))
      od->cssprefix[0] = '\0';
    if (!STREQ(od->cacheoutfile, "none")) {
      if (outstyle != OUT_NONE) {
	if (IS_STDOUT(od->outfile) &&
	    IS_STDOUT(od->cacheoutfile))
	  error("OUTFILE and CACHEOUTFILE both set to stdout");
	else if (STREQ(od->cacheoutfile, od->outfile))
	  error("OUTFILE and CACHEOUTFILE are the same");
	/* won't catch same file under different names, but cache file opening
	   will still fail later in that case */
      }
      for (i = 0; i < INPUT_NUMBER; i++)
	wantitem[i] = TRUE;
      op->miscopts.granularity = rep2gran[REP_FIVEREP];
      for (i = 0, j = 0; i < ITEM_NUMBER; i++)
	j += (int)(op->miscopts.lowmem[i] >= 3);
      if (j != 0)
	warn('D', TRUE, "LOWMEM 3 prevents that item being cached");
    }
    else if (outstyle == OUT_NONE)
      error("OUTPUT NONE and CACHEOUTFILE none selected");
    else {  /* cachefile == none */
      for (i = 0; i < INPUT_NUMBER; i++)
	wantitem[i] = FALSE;
      for (op->miscopts.granularity = 1, i = 0;
	   od->reporder[i] != -1; i++) {
	rep = od->reporder[i];
	if (rep < DATEREP_NUMBER && od->repq[rep])
	  op->miscopts.granularity = MAX(op->miscopts.granularity,
					 rep2gran[rep]);
	if (od->repq[rep] && rep2type[rep] != UNSET)
	  wantitem[rep2type[rep]] = TRUE;
	if (strchr(od->gensumlines, 'I') != NULL)
	  wantitem[ITEM_FILE] = TRUE;
	if (strchr(od->gensumlines, 'J') != NULL)
	  wantitem[ITEM_HOST] = TRUE;
      }
    }  /* end cachefile == none */
    for (lp = op->miscopts.logfile[0], templ = FALSE; lp != NULL;
	 TO_NEXT(lp)) {
      (void)correctlogfmt(lp, op->wanthead, op->code2type, &(op->dman));
      if (op->miscopts.lowmem[ITEM_VHOST] >= 3 && lp->pvpos != UNSET) {
	if (!templ) {
	  warn('C', TRUE,
	       "Ignoring %%v in logfile prefixes because of VHOSTLOWMEM 3");
	  templ = TRUE;
	}
	lp->pvpos = UNSET;
      }
    }
  }  /* end if (!vblesonly) */
  if (outstyle == COMPUTER) {
    od->sepchar = '\0';
    od->repsepchar = '\0';
    od->decpt = '.';
    od->rawbytes = TRUE;
    od->lang.code = ENGLISH;  /* force English in CRO */
    od->lang.file = NULL;
    while ((c = strstr(od->compsep, "\\t")) != NULL) {
      *c = '\t';
      memmove((void *)(c + 1), (void *)(c + 2), strlen(c + 2) + 1);
    }
  }
  else if (od->repsepchar == (char)UNSET)
    od->repsepchar = '\0'; /* sepchar, decpt set from langfile below */
  /* lower case appropriate aliases and in/excludes */
  /* NB could send them through all fixed aliases, but others probably
     never needed and could cause confusion */
  toloweralias(op->aliashead[ITEM_HOST], TRUE);
  toloweralias(od->aliashead[G(REP_HOST)], FALSE);
  toloweralias(od->aliashead[G(REP_DOM)], FALSE);
  toloweralias(od->aliashead[G(REP_ORG)], FALSE);
  tolowerinc(op->wanthead[ITEM_HOST]);
  tolowerinc(od->wanthead[G(REP_HOST)]);
  tolowerinc(od->wanthead[G(REP_DOM)]);
  tolowerinc(od->wanthead[G(REP_ORG)]);
  toloweralias(op->aliashead[ITEM_VHOST], TRUE);
  toloweralias(od->aliashead[G(REP_VHOST)], FALSE);
  tolowerinc(op->wanthead[ITEM_VHOST]);
  tolowerinc(od->wanthead[G(REP_VHOST)]);
  tolowerinc(od->wanthead[G(REP_SEARCHREP)]);
  tolowerinc(od->wanthead[G(REP_SEARCHSUM)]);
  if (op->miscopts.usercase_insensitive) {
    toloweralias(op->aliashead[ITEM_USER], TRUE);
    toloweralias(od->aliashead[G(REP_USER)], FALSE);
    toloweralias(od->aliashead[G(REP_FAILUSER)], FALSE);
    tolowerinc(op->wanthead[ITEM_USER]);
    tolowerinc(od->wanthead[G(REP_USER)]);
    tolowerinc(od->wanthead[G(REP_FAILUSER)]);
  }
  if (op->miscopts.case_insensitive) {
    toloweralias(op->aliashead[ITEM_FILE], TRUE);
    toloweralias(od->aliashead[G(REP_REQ)], FALSE);
    toloweralias(od->aliashead[G(REP_REDIR)], FALSE);
    toloweralias(od->aliashead[G(REP_FAIL)], FALSE);
    toloweralias(od->aliashead[G(REP_TYPE)], FALSE);
    toloweralias(od->aliashead[G(REP_DIR)], FALSE);
    tolowerinc(op->wanthead[ITEM_FILE]);
    tolowerinc(op->ispagehead);
    tolowerinc(op->argshead);
    tolowerinc(od->wanthead[G(REP_REQ)]);
    tolowerinc(od->wanthead[G(REP_REDIR)]);
    tolowerinc(od->wanthead[G(REP_FAIL)]);
    tolowerinc(od->wanthead[G(REP_TYPE)]);
    tolowerinc(od->wanthead[G(REP_DIR)]);
    tolowerse(op->intsearchengines);
  }  
}

#define POSSTREE(r) if (op->miscopts.lowmem[rep2type[r]] >= 3 && od->repq[r]) \
		       od->alltrees[i++] = r
#define POSSDERV(r) if (op->miscopts.lowmem[rep2type[r]] >= 3 && od->repq[r]) \
		       od->alldervs[i++] = r
#define POSSDATACOL(t, a, b) if (op->data2cols[i][a] >= 0) { \
      op->datacols[i][t][k][0] = op->data2cols[i][a]; \
      op->datacols[i][t][k++][1] = b; }
#define CLOSECATEGORY(t) op->datacols[i][t][k][0] = -1; k = 0;
#define ALIASP(c, f, t) ENSURE_LEN(ss, len, strlen(t) + 7); \
                        sprintf(ss, "PLAIN:%s", t); confline(op, c, f, ss, -1)

void finalinit(Options *op) {
#ifndef NODNS
  extern choice dnslevel;
  extern char *dnsfile, *dnslockfile;
  extern FILE *dnsfilep, *dnslock;
  extern Hashtable *dnstable;
  extern unsigned int dnsgoodhrs, dnsbadhrs;
  extern Outputter cro_outputter, html_outputter, latex_outputter,
    plain_outputter, xhtml_outputter, xml_outputter;
  timecode_t timec, goodtimec, badtimec;
  char *name, *alias;
  choice rc;
#endif
#ifndef NOPIPES
  extern Strpairlist *uncompresshead;
#endif
  extern timecode_t starttimeuxc;
  extern char *country[];
  extern Memman *xmemman;
  extern choice *rep2type, *rep2reqs, *rep2reqs7, *rep2date, *rep2firstd;
  extern choice cache_records[];
  extern char *pos;
  extern logical vblesonly;
  extern logical unprintable[256];
  static char *ss = NULL;
  static size_t len = 0;

  Outchoices *od = &(op->outopts);
  choice outstyle = od->outstyle;

  Logfile *lp, *lastlp;
  logical wantcol[ITEM_NUMBER][DATA_NUMBER];
  choice *cols;
  char **lngstr = NULL;  /* just to keep compiler happy */
  FILE *tempf;
  char temps[10];  /* for "ukdom.tab" */
  char *s;
  Strlist *sp;
  int i, j, k;

  /* if only doing cache file, turn off all reports to avoid recording unwanted
     stuff inadvertently */
  if (outstyle == OUT_NONE) {
    for (i = 0; i < REP_NUMBER; i++)
      od->repq[i] = FALSE;
  }
  /* set code2type variable */
  for (i = MIN_SC; i < SC_NUMBER; i++) {
    if (op->code2type[i] == UNWANTED ||
	(op->code2type[i] == UNSET && op->code2type[0] == SUCCESS))
      op->code2type[i] = UNWANTED;
    else if (i <= 199)
      op->code2type[i] = INFO;
    else if (i <= 299 || (i == 304 && op->succ304))
      op->code2type[i] = SUCCESS;
    else if (i <= 399)
      op->code2type[i] = REDIRECT;
    else
      op->code2type[i] = FAILURE;
  }
  /* swap aliases round */
  for (i = 0; i < ITEM_NUMBER; i++)
    reversealias(&(op->aliashead[i]));
  for (i = 0; i < GENREP_NUMBER; i++)
    reversealias(&(od->aliashead[i]));
  /* and swap uncompress list round */
#ifndef NOPIPES
  reversestrpairlist(&uncompresshead);
#endif
  /* Translate filenames to put files in the correct directories */
  /* Also interpret %date codes and wild cards where appropriate */
  if (!strcaseeq(od->headerfile, "none") && !IS_STDIN(od->headerfile))
    od->headerfile = buildfilename(HEADERDIR, "", od->headerfile);
  if (!strcaseeq(od->footerfile, "none") && !IS_STDIN(od->footerfile))
    od->footerfile = buildfilename(HEADERDIR, "", od->footerfile);
  if (!STREQ(od->cacheoutfile, "none") && !IS_STDOUT(od->cacheoutfile)) {
    s = timesprintf(NULL, buildfilename(CACHEDIR, "", od->cacheoutfile),
		    op->dman.last7to, TRUE);
    COPYSTR(od->cacheoutfile, s);
  }
  if (!IS_STDOUT(od->outfile)) {
    s = timesprintf(NULL, od->outfile, op->dman.last7to, TRUE);
    /* outfile has already had buildfilename() in configoutfile() */
    COPYSTR(od->outfile, s);
  }
  for (i = 0; i <= 1; i++) {
    for (lp = op->miscopts.logfile[i], lastlp = NULL; lp != NULL;
	 lastlp = lp, TO_NEXT(lp)) {
      if (!IS_STDIN(lp->name)) {
	s = timesprintf(NULL, lp->name, op->dman.last7to, TRUE);
	COPYSTR(lp->name, s);
#ifndef NODIRENT
	if (strchr(lp->name, '*') != NULL || strchr(lp->name, '?') != NULL)
	  lp = expandwildlogs(lp, (lastlp == NULL)?\
			      (&(op->miscopts.logfile[i])):(&(lastlp->next)));
#endif
      }
    }
  }
#ifndef NOGRAPHICS
  if (outstyle == HTML || outstyle == XHTML) {
    if (od->chartdir == NULL || od->localchartdir == NULL) {
      /* If CHARTDIR and LOCALCHARTDIR both set, everything is OK */
      for (i = 0, j = 0; i < GENREP_NUMBER; i++)
	j += (od->chartby[i] != CHART_NONE && od->repq[i + FIRST_GENREP]);
      if (j != 0) {  /* j = number of wanted charts */
	if (od->chartdir != NULL)
	  warn('C', TRUE, "Ignoring CHARTDIR because LOCALCHARTDIR is unset");
	else if (od->localchartdir != NULL)
	  warn('C', TRUE, "Ignoring LOCALCHARTDIR because CHARTDIR is unset");
	if (IS_STDOUT(od->outfile)) {
	  warn('D', TRUE, "Turning all pie charts off because OUTFILE is "
	       "stdout and CHARTDIR is unset");
	  for (i = 0, j = 0; i < GENREP_NUMBER; i++)
	    od->chartby[i] = CHART_NONE;
	}
	else {
	  od->localchartdir = buildfilename(od->outfile, "", "");
	  od->chartdir = "";
	}
      }
    }
    /* convert any date codes in CHARTDIRs */
    if (od->chartdir != NULL && od->localchartdir != NULL) {
      s = timesprintf(NULL, od->localchartdir, op->dman.last7to, TRUE);
      COPYSTR(od->localchartdir, s);
      s = timesprintf(NULL, od->chartdir, op->dman.last7to, TRUE);
      COPYSTR(od->chartdir, s);
    }
  }
#endif
#ifndef NODNS
  if (!IS_STDIN(dnsfile))
    dnsfile = buildfilename(DNSDIR, "", dnsfile);
  dnslockfile = buildfilename(LOCKDIR, "", dnslockfile);
#endif
  if (od->domainsfile != NULL && !IS_STDIN(od->domainsfile))
    od->domainsfile = buildfilename(LANGDIR, "lang", od->domainsfile); 
  if (od->descfile != NULL && !IS_STDIN(od->descfile))
    od->descfile = buildfilename(LANGDIR, "lang", od->descfile); 
  if (od->lang.file == NULL)
    selectlang(country[od->lang.code], &(op->outopts));
  else if (!IS_STDIN(od->lang.file))
    od->lang.file = buildfilename(LANGDIR, "lang", od->lang.file); 
  if (od->domainsfile == NULL) {
    sprintf(temps, "ukdom%ctab", EXTSEP);
    od->domainsfile = buildfilename(LANGDIR, "lang", temps);
  }
  od->multibyte = FALSE;
  if (outstyle == PLAIN) {
    od->outstyle = ASCII;
    outstyle = ASCII;
    /* The only difference is the langfile, which we have just chosen */
  }
  if (outstyle == ASCII)
    od->outputter = &plain_outputter;
  else if (outstyle == HTML)
    od->outputter = &html_outputter;
  else if (outstyle == XHTML)
    od->outputter = &xhtml_outputter;
  else if (outstyle == LATEX)
    od->outputter = &latex_outputter;
  else if (outstyle == XML)
    od->outputter = &xml_outputter;
  else /* outstyle == COMPUTER */
    od->outputter = &cro_outputter;
  if (!vblesonly) {  /* persists to end of function */
    if (outstyle != OUT_NONE) {
      /* Read in language information */
      if ((tempf = my_fopen(od->lang.file, "language file")) == NULL)
	error("Can't read language file %s", od->lang.file);
      od->lngstr = (char **)xmalloc(LNGSTR_NUMBER * sizeof(char *));
      lngstr = od->lngstr;  /* just for conciseness */
      pos = NULL;
      COPYSTR(lngstr[charset_], nextlngstr(tempf, od->lang.file, TRUE));
      if (outstyle == LATEX && !strcaseeq(lngstr[charset_], "US-ASCII") &&
	  !(headcasematch(lngstr[charset_], "ISO-8859-") &&
	    (lngstr[charset_][9] == '1' || lngstr[charset_][9] == '2') &&
	    lngstr[charset_][10] == '\0'))
	error("OUTPUT LATEX only available with US-ASCII, ISO-8859-1 and "
	      "ISO-8859-2 character sets");
      /* NB Latin-3 and -5 have LaTeX .def files too but I have no such
	 language files, so no way to test them (and no need). Ditto some
	 Windows codepages. */
      if (*(lngstr[charset_]) == '*') {
	od->multibyte = TRUE;
	(lngstr[charset_])++;
	od->html = FALSE;
      }
      if (outstyle == HTML || outstyle == XHTML) {
	if (strcaseeq(lngstr[charset_], "UTF-8"))
	  od->outputter->strlength = &html_strlength_utf8;
	else if (strcaseeq(lngstr[charset_], "ISO-2022-JP"))
	  od->outputter->strlength = &html_strlength_jis;
      }
      for (i = 0; i < 7; i++)
	COPYSTR(od->dayname[i], nextlngstr(tempf, od->lang.file, TRUE));
      for (i = 0; i < 12; i++)
	COPYSTR(od->monthname[i], nextlngstr(tempf, od->lang.file, TRUE));
      for (i = 1; i < LNGSTR_NUMBER; i++)  /* charset_ was 0 */
	COPYSTR(lngstr[i], nextlngstr(tempf, od->lang.file, TRUE));
      (void)nextlngstr(tempf, od->lang.file, FALSE);
      /* This final nextlngstr() checks the language file isn't too long */
      if (strchr(lngstr[xbytes_], '?') == NULL ||
	  strchr(lngstr[nxbytes_], '?') == NULL ||
	  strchr(lngstr[nxbytes7_], '?') == NULL ||
	  strchr(lngstr[xbytestraffic_], '?') == NULL ||
	  strchr(lngstr[xbytestraffic7_], '?') == NULL)
	error("language file %s corrupt: translation of \"?bytes\" missing "
	      "a question mark", od->lang.file);
      /* Fatal error is harsh here: but it avoids all sorts of trouble later */
      od->plainmonthlen = arraymaxlen(od->monthname, 12, NULL);
      /* plain is longer: by pretending to be ASCII it includes whole strlen */
      /* But note that even this is not enough for formatted dates: could need
	 max_ascii_len + (max_html_len - min_html_len) to apply spacing */
      od->monthlen = arraymaxlen(od->monthname, 12, od);
      od->plaindaylen = arraymaxlen(od->dayname, 7, NULL);
      od->daylen = arraymaxlen(od->dayname, 7, od);
      od->plainampmlen = MAX(strlen(lngstr[am_]), strlen(lngstr[pm_]));
      od->ampmlen = MAX(od->outputter->strlength(lngstr[am_]),
			od->outputter->strlength(lngstr[pm_]));
      /* Set the convfloor (see do_alias(n|N)). We only do this approximately:
	 convert A0-FF for ISO-8859-*, 80-FF o/wise. This may still include
	 some non-printable characters! But we can't have a table for every
	 charset. */
      if (od->searchconv == UNSET)
	od->searchconv = !(od->multibyte);
      if (!od->searchconv || strcaseeq(lngstr[charset_], "US-ASCII"))
	od->convfloor = 0;
      else if (substrcaseeq(lngstr[charset_], "iso-8859-") ||
	       strcaseeq(lngstr[charset_], "armscii-8"))
	od->convfloor = 0xA0;
      else
	od->convfloor = 0x80;
      /* Similarly set the characters we shall consider as unprintable in the
	 reports. */
#ifdef EBCDIC
    for (i = 0; i <= 0x3F; i++)
      unprintable[i] = TRUE;
#else
      for (i = 0; i <= 0x1F; i++)
	unprintable[i] = TRUE;
      for (i = 0x20; i <= 0xFF; i++)
	unprintable[i] = FALSE;
      if (substrcaseeq(lngstr[charset_], "iso-8859-") ||
	  strcaseeq(lngstr[charset_], "armscii-8")) {
	for (i = 0x80; i <= 0x9F; i++)
	  unprintable[i] = TRUE;
      }
      unprintable[0x7F] = TRUE;
#endif
      unprintable[(unsigned char)'\0'] = FALSE;
      unprintable[(unsigned char)'\t'] = FALSE;
      if (od->baseurl != NULL && STREQ(od->baseurl, "none"))
	od->baseurl = NULL;
      if (outstyle == COMPUTER) {
	COPYSTR(lngstr[succreqs_], "SRS7");
	COPYSTR(lngstr[totunknown_], "NCC7");
	COPYSTR(lngstr[totpages_], "PRP7");
	COPYSTR(lngstr[totfails_], "FLF7");
	COPYSTR(lngstr[totredirs_], "RRR7");
	COPYSTR(lngstr[inforeqs_], "NII7");
	COPYSTR(lngstr[distfiles_], "NFN7");
	COPYSTR(lngstr[disthosts_], "NHH7");
	COPYSTR(lngstr[corrupt_], "CL");
	COPYSTR(lngstr[unwanted_], "UL");
	COPYSTR(lngstr[totdata_], "BTB7");
	COPYSTR(lngstr[dayrepfmt_], "%Y%\b%M%\b%D");
	COPYSTR(lngstr[daysumfmt_], "%w");
	COPYSTR(lngstr[hourrepfmt_], "%Y%\b%M%\b%D%\b%H");
	COPYSTR(lngstr[hoursumfmt_], "%H");
	COPYSTR(lngstr[weekhoursumfmt_], "%w%\b%H");
	COPYSTR(lngstr[quarterrepfmt_], "%Y%\b%M%\b%D%\b%H%\b%n");
	COPYSTR(lngstr[quartersumfmt_], "%H%\b%n");
	COPYSTR(lngstr[weekfmt_], "%Y%\b%M%\b%D");
	COPYSTR(lngstr[monthfmt_], "%Y%\b%M");
	COPYSTR(lngstr[quarterlyfmt_], "%Y%\b%q");
	COPYSTR(lngstr[yearfmt_], "%Y");
	COPYSTR(lngstr[genrepdate_], "%Y%\b%M%\b%D");
	COPYSTR(lngstr[genreptime_], "%Y%\b%M%\b%D%\b%H%\b%n");
	COPYSTR(lngstr[datefmt1_], "%Y%\b%M%\b%D%\b%H%\b%n");
	COPYSTR(lngstr[datefmt2_], "%Y%\b%M%\b%D%\b%H%\b%n");
      }
      /* repsepchar done above. Also OUTPUT COMPUTER versions done above. */
      if (od->sepchar == (char)UNSET)
	od->sepchar = *(lngstr[sepchar_]);
      if (od->decpt == (char)UNSET)
	od->decpt = *(lngstr[decpoint_]);
      (void)my_fclose(tempf, od->lang.file, "language file");

      if ((tempf = my_fopen(od->domainsfile, "domains file")) != NULL) {
	process_domainsfile(tempf, op);
	(void)my_fclose(tempf, od->domainsfile, "domains file");
      }
      if (od->descriptions && od->descfile != NULL &&
	  (tempf = my_fopen(od->descfile, "report descriptions file"))
	  != NULL) {
	process_descfile(tempf, od->descfile, op);
	(void)my_fclose(tempf, od->descfile, "report descriptions file");
      }
      else
	od->descriptions = FALSE;
    }  /* end outstyle != OUT_NONE */

    /* Which data we should collect about each item? */
    for (i = 0; i < ITEM_NUMBER; i++) {
      for (j = 0; j < DATA_NUMBER; j++)
	wantcol[i][j] = FALSE;
    }
    if (!strcaseeq(od->cacheoutfile, "none")) {
      /* If we are writing a cache file, must have everything it records. */
      for (i = 0; i < ITEM_NUMBER; i++)
	for (j = 0; j < V5_DATA_NUMBER; j++)
	  wantcol[i][cache_records[j]] = TRUE;
    }
    if (outstyle != OUT_NONE) {
      /* For a regular output file, check all reports to see what's wanted. */
      for (i = FIRST_GENREP; i <= LAST_NORMALREP; i++) { 
	if (od->repq[i]) {
	  /* always need REQUESTS */
	  wantcol[rep2type[i]][rep2reqs[G(i)]] = TRUE;
	  /* need (SUB)FLOOR and (SUB)SORTBY and CHART */
	  if (od->floor[G(i)].floorby == REQUESTS7 ||
	      od->sortby[G(i)] == REQUESTS7 ||
	      od->subfloor[G(i)].floorby == REQUESTS7 ||
	      od->subsortby[G(i)] == REQUESTS7 ||
	      od->chartby[G(i)] == REQUESTS7)
	    wantcol[rep2type[i]][rep2reqs7[G(i)]] = TRUE;
	  if (od->floor[G(i)].floorby == PAGES || od->sortby[G(i)] == PAGES ||
	      od->subfloor[G(i)].floorby == PAGES ||
	      od->subsortby[G(i)] == PAGES || od->chartby[G(i)] == PAGES)
	    wantcol[rep2type[i]][PAGES] = TRUE;
	  if (od->floor[G(i)].floorby == PAGES7 ||
	      od->sortby[G(i)] == PAGES7 ||
	      od->subfloor[G(i)].floorby == PAGES7 ||
	      od->subsortby[G(i)] == PAGES7 || od->chartby[G(i)] == PAGES7)
	    wantcol[rep2type[i]][PAGES7] = TRUE;
	  if (od->floor[G(i)].floorby == DATESORT ||
	      od->sortby[G(i)] == DATESORT ||
	      od->subfloor[G(i)].floorby == DATESORT ||
	      od->subsortby[G(i)] == DATESORT)
	    /* NB chartby can't be date or firstdate */
	    wantcol[rep2type[i]][rep2date[G(i)]] = TRUE;
	  if (od->floor[G(i)].floorby == FIRSTDATE ||
	      od->sortby[G(i)] == FIRSTDATE ||
	      od->subfloor[G(i)].floorby == FIRSTDATE ||
	      od->subsortby[G(i)] == FIRSTDATE)
	    wantcol[rep2type[i]][rep2firstd[G(i)]] = TRUE;
	  /* need COLS */
	  cols = od->cols[i];
	  for (j = 0; cols[j] != COL_NUMBER; j++) {
	    if (cols[j] == COL_REQS7 || cols[j] == COL_PREQS7)
	      wantcol[rep2type[i]][rep2reqs7[G(i)]] = TRUE;
	    else if (cols[j] == COL_PAGES || cols[j] == COL_PPAGES)
	      wantcol[rep2type[i]][PAGES] = TRUE;
	    else if (cols[j] == COL_PAGES7 || cols[j] == COL_PPAGES7)
	      wantcol[rep2type[i]][PAGES7] = TRUE;
	    else if (cols[j] == COL_DATE || cols[j] == COL_TIME)
	      wantcol[rep2type[i]][rep2date[G(i)]] = TRUE;
	    else if (cols[j] == COL_FIRSTD || cols[j] == COL_FIRSTT)
	      wantcol[rep2type[i]][rep2firstd[G(i)]] = TRUE;
	  }
	  /* if reportspan is on, need firsttime and lasttime */
	  if (od->repspan) {
	    wantcol[rep2type[i]][rep2date[G(i)]] = TRUE;
	    wantcol[rep2type[i]][rep2firstd[G(i)]] = TRUE;
	  }
	}
      }
      /* and General Summary needs count of files and hosts */
      wantcol[ITEM_FILE][REQUESTS] = TRUE;
      wantcol[ITEM_FILE][REQUESTS7] = TRUE;
      wantcol[ITEM_HOST][REQUESTS] = TRUE;
      wantcol[ITEM_HOST][REQUESTS7] = TRUE;
    }  /* end outstyle != OUT_NONE */
    /* now assemble wantcol into a data2cols structure */
    for (i = 0; i < ITEM_NUMBER; i++) {
      for (j = 0, k = 0; j < DATA_NUMBER; j++)
	op->data2cols[i][j] = (wantcol[i][j])?(k++):(-1);
      op->no_cols[i] = k;
    }
    /* and finally, turn data2cols into datacols */
    for (i = 0, k = 0; i < ITEM_NUMBER; i++) {
      POSSDATACOL(SUCCESS, REQUESTS, REQUESTS2);
      POSSDATACOL(SUCCESS, REQUESTS7, REQUESTS72);
      POSSDATACOL(SUCCESS, PAGES, PAGES2);
      POSSDATACOL(SUCCESS, PAGES7, PAGES72);
      POSSDATACOL(SUCCESS, SUCCDATE, DATE2);
      POSSDATACOL(SUCCESS, SUCCFIRSTD, FIRSTD2);
      CLOSECATEGORY(SUCCESS);
      POSSDATACOL(FAILURE, FAIL, REQUESTS2);
      POSSDATACOL(FAILURE, FAIL7, REQUESTS72);
      POSSDATACOL(FAILURE, FAILDATE, DATE2);
      POSSDATACOL(FAILURE, FAILFIRSTD, FIRSTD2);
      CLOSECATEGORY(FAILURE);
      POSSDATACOL(REDIRECT, REDIR, REQUESTS2);
      POSSDATACOL(REDIRECT, REDIR7, REQUESTS72);
      POSSDATACOL(REDIRECT, REDIRDATE, DATE2);
      POSSDATACOL(REDIRECT, REDIRFIRSTD, FIRSTD2);
      CLOSECATEGORY(REDIRECT);
    }

    op->miscopts.dirsufflength = strlen(op->miscopts.dirsuffix);
    for (i = 0; i < GENREP_NUMBER; i++)
      allgraft(od->tree[i]->tree, od->tree[i]->space,
	       op->no_cols[rep2type[i + FIRST_GENREP]]);
    for (sp = od->suborgs; sp != NULL; TO_NEXT(sp)) {
      if (strlen(sp->name) > 0 && !ISDIGIT(*(sp->name)) &&
	  sp->name[strlen(sp->name) - 1] == '*')  /* won't catch 3com.* */
	warn('C', TRUE, "SUBORG %s meaningless: must specify a domain because "
	     "organisations are at different levels in different domains",
	     sp->name);
      else
	confline(op, "SUBORG2", sp->name, NULL, 1);
    }
    od->alltrees = (choice *)submalloc(xmemman, 6 * sizeof(choice));
    i = 0;
    POSSTREE(REP_DIR);
    POSSTREE(REP_DOM);
    POSSTREE(REP_ORG);
    POSSTREE(REP_TYPE);
    POSSTREE(REP_REFSITE);  /* if adding more, change '6' above */
    od->alltrees[i] = REP_NUMBER;
    od->alldervs = (choice *)submalloc(xmemman, 5 * sizeof(choice));
    i = 0;
    POSSDERV(REP_SEARCHREP);
    POSSDERV(REP_SEARCHSUM);
    POSSDERV(REP_BROWSUM);
    POSSDERV(REP_OS);       /* if adding more, change '5' above */
    od->alldervs[i] = REP_NUMBER;
    od->derv[REP_SEARCHREP - FIRST_DERVREP]->arg = (void *)(op->searchengines);
    od->derv[REP_SEARCHSUM - FIRST_DERVREP]->arg = (void *)(op->searchengines);
    od->derv[REP_INTSEARCHREP - FIRST_DERVREP]->arg =
      (void *)(op->intsearchengines);
    od->derv[REP_INTSEARCHSUM - FIRST_DERVREP]->arg =
      (void *)(op->intsearchengines);
    od->derv[REP_OS - FIRST_DERVREP]->arg = (void *)(op->robots);
    od->derv[REP_BROWSUM - FIRST_DERVREP]->arg = NULL;
    if (outstyle != OUT_NONE) {
      ALIASP("DOMOUTPUTALIAS", LNGSTR_UNRESOLVED, lngstr[unresolved_]);
      ALIASP("DOMOUTPUTALIAS", LNGSTR_NODOMAIN, lngstr[nodomain_]);
      ALIASP("DOMOUTPUTALIAS", LNGSTR_UNKDOMAIN, lngstr[unkdomain_]);
      ALIASP("ORGOUTPUTALIAS", LNGSTR_NODOMAIN, lngstr[nodomain_]);
      ALIASP("ORGOUTPUTALIAS", LNGSTR_UNKDOMAIN, lngstr[unkdomain_]);
      ALIASP("DIROUTPUTALIAS", LNGSTR_ROOTDIR, lngstr[rootdir_]);
      ALIASP("DIROUTPUTALIAS", LNGSTR_NODIR, lngstr[nodir_]);
      ALIASP("TYPEOUTPUTALIAS", LNGSTR_NOEXT, lngstr[noext_]);
      ALIASP("TYPEOUTPUTALIAS", LNGSTR_BRKDIRS, lngstr[brkdirs_]);
      ALIASP("OSOUTPUTALIAS", LNGSTR_UNKWIN, lngstr[unkwin_]);
      ALIASP("OSOUTPUTALIAS", LNGSTR_UNKUX, lngstr[unkux_]);
      ALIASP("OSOUTPUTALIAS", LNGSTR_UNKOS, lngstr[unkos_]);
      ALIASP("OSOUTPUTALIAS", LNGSTR_ROBOTS, lngstr[robots_]);
    }
#ifndef NODNS
    if (dnslevel != DNS_NONE) {
      dnstable = rehash(NULL, HASHSIZE, NULL);
      if ((tempf = my_fopen(dnsfile, "DNS input file")) != NULL) {
	pos = NULL;
	goodtimec = (starttimeuxc > (timecode_t)dnsgoodhrs * 60)?\
	  (starttimeuxc - (timecode_t)dnsgoodhrs * 60):0;
	badtimec = (starttimeuxc > (timecode_t)dnsbadhrs * 60)?\
	  (starttimeuxc - (timecode_t)dnsbadhrs * 60):0;
	while((rc = nextdnsline(tempf, &timec, &name, &alias)) != EOF) {
	  if (rc == TRUE && ((STREQ(alias, "*") && timec >= badtimec) ||
			     (!STREQ(alias, "*") && timec >= goodtimec)) &&
	      !(timec > starttimeuxc + MINS_IN_WEEK))
	    do_dns(name, alias, dnslevel);
	}
	(void)my_fclose(tempf, dnsfile, "DNS input file");
      }
      if (dnslevel == DNS_WRITE) {
#ifdef NOOPEN
	/* The ANSI, but less preferred, option. There is a race problem. Also
	   if we have got overwrite access but not read, it will go wrong. */
	if ((tempf = FOPENR(dnslockfile)) == NULL) {
	  if ((dnslock = FOPENW(dnslockfile)) == NULL) {
	    warn('F', TRUE, "Failed to create DNS lock file %s: "
		 "backing off to DNS LOOKUP", dnslockfile);
	    dnslevel = DNS_LOOKUP;
	  }
	  else
	    debug('F', "Creating %s as DNS lock file", dnslockfile);
	}
	else {
	  fclose(tempf);
	  warn('F', TRUE,
	       "DNS lock file %s already exists: backing off to DNS LOOKUP",
	       dnslockfile);
	  dnslevel = DNS_LOOKUP;
	}
#else
	/* The following is not the strictly correct procedure on Unix. NFS is
	   broken, so there can still be a race condition. One should really
	   @ create a guaranteed unique file;
	   @ hard link the lock file to the unique file;
	   @ stat the unique file, testing for links == 2.
	   However, this has the disadvantage that it can leave files with
	   weird names lying around. Also it can't then share code with other
	   platforms. I think that the chance of a problem is so small, and the
	   consequences sufficiently non-serious, that this is good enough. */
	if ((i = open(dnslockfile, O_WRONLY | O_CREAT | O_EXCL, OPEN_MODE)) < 0) {
	  if (errno == EEXIST)
	    warn('F', TRUE,
		 "DNS lock file %s already exists: backing off to DNS LOOKUP",
		 dnslockfile);
	  else
	    warn('F', TRUE, "Failed to create DNS lock file %s: "
		 "backing off to DNS LOOKUP", dnslockfile);
	  dnslevel = DNS_LOOKUP;
	}
	else if ((dnslock = fdopen(i, "w")) == NULL) { /* can this happen? */
	  /* We don't actually write to the dnslock. But this is convenient
	     for compatibility with the #ifdef NOOPEN approach. */
	  warn('F', TRUE,
	       "Failed to create DNS lock file %s: backing off to DNS LOOKUP",
	       dnslockfile);
	  dnslevel = DNS_LOOKUP;
	}
	else
	  debug('F', "Creating %s as DNS lock file", dnslockfile);
#endif  /* NOOPEN */
	if (dnslevel == DNS_WRITE) {
	  if ((dnsfilep = FOPENA(dnsfile)) == NULL) {
	    warn('F', TRUE, "Failed to open DNS output file %s for writing: "
		 "backing off to DNS LOOKUP", dnsfile);
	    dnslevel = DNS_LOOKUP;
	    fclose(dnslock);
	    dnslock = NULL;
	    if (remove(dnslockfile) != 0)
	      warn('F', TRUE, "Trouble deleting DNS lock file %s",
		   dnslockfile);
	    else
	      debug('F', "Deleting DNS lock file %s", dnslockfile);
	  }
	  else
	    debug('F', "Opening %s as DNS output file", dnsfile);
	}
      }
    }  /* end if dnslevel != DNS_NONE */
#endif  /* NODNS */
  }  /* end if !vblesonly */
}

/* Now functions for turning strings into log formats. */
/* The actual mapping of codes to functions is at inpfns[] in globals.c. */
choice strtoinfmt(Inputformat **ans, char *s, choice *count) {
  /* For a discussion of count[], see correctlogfmt() above */
  extern Memman *xmemman;
  extern Inputfns inpfns[], pnlinpfn, ccinpfn;

  Inputformat *ifp;
  logical done, count_this, typedone[INP_NUMBER];
  char tempchar = '\0';
  char *c;
  int i;

  if (strchr(s, '%') == NULL)
    return(FMT_NOPC);
  for (i = 0; i < ITEM_NUMBER; i++)
    count[i] = 0;
  for (i = 0; i < INP_NUMBER; i++)
    typedone[i] = FALSE;
  *ans = (Inputformat *)submalloc(xmemman, sizeof(Inputformat));
  ifp = *ans;
  /* First a messy special case. If %s occurs but not %S, promote %s to %S */
  for (c = s, done = FALSE; *c != '\0' && !done; c++) {
    if (*c == '%') {
      c++;
      if (*c == '*')
	c++;
      if (*c == 'S')
	done = TRUE;
      c++;
    }
  }
  if (!done) {
    for (c = s; *c != '\0'; c++) {
      if (*c == '%') {
	c++;
	if (*c == '*')
	  c++;
	if (*c == 's')
	  *c = 'S';
	c++;
      }
    }
  }
  /* Now the main routine */
  for (c = s; *c != '\0'; c++) {
    if (*c == '%') {
      c++;
      if (*c == '%') {
	ifp->inpfns = &ccinpfn;
	ifp->sep = *c;
      }
      else {
	if (*c == '*') {
	  count_this = FALSE;
	  c++;
	}
	else
	  count_this = TRUE;
	done = FALSE;
	for (i = 0; !done && inpfns[i].code != '\0'; i++) {
	  if (*c == inpfns[i].code) {
	    if (inpfns[i].type != UNSET) {
	      if (typedone[inpfns[i].type])
		return(FMT_DUP);
	      typedone[inpfns[i].type] = TRUE;
	    }
	    ifp->inpfns = &inpfns[i];
	    if (inpfns[i].fn == &parsestring || inpfns[i].fn == &parseref ||
		inpfns[i].fn == &parsemsbrow || inpfns[i].fn == &parsejunk ||
		inpfns[i].fn == &parsecode) {
	      c++;
	      if (*c == '\0') {
		ifp->sep = '\n';
		c--;
	      }
	      else if (*c == '\\') {
		c++;
		if (*c == '\\')
		  ifp->sep = *c;
		else if (*c == 'n' || *c == 'r')
		  ifp->sep = '\n';
		else if (*c == 't')
		  ifp->sep = '\t';
		else
		  return(FMT_BADCHAR);
	      }
	      else if (*c == '%') {
		c++;
		if (*c == '%')
		  ifp->sep = *c;
		else if (*c == 'w' && inpfns[i].fn != &parseref) {
		  ifp->sep = WHITESPACE;      /* parseref can't take %w */
		  c -= 2;  /* need to parsespace() too */
		}
		else
		  return(FMT_NOTERM);
	      }
	      else
		ifp->sep = *c;
	    }
	    else if (inpfns[i].fn == &parselogfmt) {
	      c++;
	      if (*c < '0' || *c > '6')
		return(FMT_BADBUILTIN);
	      else
		ifp->sep = *c;
	    }
	    else  /* fn != parse(string|ref|msbrow|junk|code|logfmt) */
	      ifp->sep = '\0';
	    if (i < ITEMFNS_NUMBER)
	      count[inpfns[i].type] = count_this?2:1;
	    done = TRUE;
	  }     /* end if *c == inpfns[i].code */
	}       /* end for i */
	if (!done)
	  return(FMT_BADPC);
      }
    }    /* end if *c == '%' */
    else if (*c == '\\') {
      c++;
      if (*c == '\\') {
	ifp->inpfns = &ccinpfn;
	ifp->sep = *c;
      }
      else if (*c == 'n' || *c == 'r') {
	ifp->inpfns = &pnlinpfn;
	ifp->sep = '\n';
      }
      else if (*c == 't') {
	ifp->inpfns = &ccinpfn;
	ifp->sep = '\t';
      }
      else
	return(FMT_BADCHAR);
    }    /* end if *c == '\\' */
    else {
      ifp->inpfns = &ccinpfn;
      ifp->sep = *c;
    }
    ifp->next = (Inputformat *)submalloc(xmemman, sizeof(Inputformat));
    tempchar = ifp->sep;
    TO_NEXT(ifp);
  }
  if (tempchar != '\n') {
    ifp->inpfns = &pnlinpfn;
    ifp->next = (Inputformat *)submalloc(xmemman, sizeof(Inputformat));
    TO_NEXT(ifp);
  }
  ifp->inpfns = NULL;
  if (typedone[INP_YEAR] || typedone[INP_MONTH] || typedone[INP_DATE] ||
      typedone[INP_HOUR] || typedone[INP_MIN] || typedone[INP_AM]) {
    if (typedone[INP_UNIXTIME])
      return(FMT_DUP);
    if (!(typedone[INP_YEAR] && typedone[INP_MONTH] && typedone[INP_DATE] &&
	  typedone[INP_HOUR] && typedone[INP_MIN]))
      return(FMT_PARTTIME);  /* partial time info is corrupt */
    count[INP_DATE] = (count[ITEM_FILE] == 2)?2:1;
    count[INP_AM] = (choice)typedone[INP_AM];
    count[INP_UNIXTIME] = 0;
  }
  else if (typedone[INP_UNIXTIME]) {
    count[INP_DATE] = (count[ITEM_FILE] == 2)?2:1;
    count[INP_AM] = 0;
    count[INP_UNIXTIME] = 1;
  }
  else {
    count[INP_DATE] = 0;
    count[INP_AM] = 0;
    count[INP_UNIXTIME] = 0;
  }
  if (typedone[INP_QUERY] && !typedone[ITEM_FILE])
    return(FMT_QBUTNOR);
  count[INP_BYTES] = typedone[INP_BYTES]?((count[ITEM_FILE] == 2)?2:1):0;
  count[INP_PROCTIME] =
    2 * (choice)(typedone[INP_PROCTIME] && count[ITEM_FILE] == 2);
  if (typedone[INP_CODE] && count[ITEM_FILE] == 2)
    count[INP_CODE] = 2;
  else if (typedone[INP_CODE])
    count[INP_CODE] = 1;
  else
    count[INP_CODE] = 0;
  return(FMT_OK);
}

char *apachelogfmt(char *fmt) {
  static char *ans = NULL;
  static size_t len = 0;
  char *p, *q;

  ENSURE_LEN(ans, len, 32);   /* Arbitrary amount to get started */
  ans[0] = '\0';
  for (p = fmt; *p != '\0'; p++) {
    ENSURE_LEN(ans, len, strlen(ans) + 20);
    /* Enough to add any next string. Not v efficient, but that's OK. */
    if (*p == '\\' && (*(p + 1) == '"' || *(p + 1) == '\\'))
      p++; /* NB \t will stay as \t etc. and analog LOGFORMAT will interpret */
    if (*p == '%' && *(p + 1) != '\0') {
      p++;
      while (ISDIGIT(*p) || *p == ',' || *p == '!' || *p == '<' || *p == '>')
	p++;
      if (*p == 'b')
	strcat(ans, "%b");
      else if (*p == 'u')
	strcat(ans, "%u");
      else if (*p == 'h' || *p == 'a')
	strcat(ans, "%S");
      else if (*p == 's')
	strcat(ans, "%c");
      else if (*p == 'U')
	strcat(ans, "%r");
      else if (*p == 'r')
	strcat(ans, "%j%w%r%wHTTP%j");
      else if (*p == 't')
	strcat(ans, "[%d/%M/%Y:%h:%n:%j]");
      else if (*p == 'T')
	strcat(ans, "%t");
      else if (*p == 'D')
	strcat(ans, "%D");
      else if (*p == 'q')
	strcat(ans, "%q");
      else if (*p == 'A' || *p == 'v' || *p == 'V')
	strcat(ans, "%v");
      else if (substrcaseeq(p, "{host}") && *(p + 6) == 'i') {
	strcat(ans, "%v");
	p += 6;
      }
      else if (substrcaseeq(p, "{user-agent}") && *(p + 12) == 'i') {
	strcat(ans, "%B");
	p += 12;
      }
      else if (substrcaseeq(p, "{referer}") && *(p + 9) == 'i') {
	strcat(ans, "%f");
	p += 9;
      }
      else { /* unknown or unwanted code */
	if (*p == '{') {
	  while (*p != '}' && *p != '\0')
	    p++;
	  if (*p == '}')
	    p++;
	  if (*p == 't')
	    return(NULL);
	}
	strcat(ans, "%j");
      }
    }  /*  *p == '%' && ... */
    else {
      q = strchr(ans, '\0');
      *(q++) = *p;
      *q = '\0';
    }
  }
  return(ans);
}

/*** Finally we move on to the command line argument processing. ***/

void clconfline(Options *op, char *s) {
  char *cmd = NULL, *arg1 = NULL, *arg2 = NULL;
  int rc;

  if ((rc = parseconfline(s, &cmd, &arg1, &arg2)) != -1)
    confline(op, cmd, arg1, arg2, rc);
}

void clgenrep(Options *op, choice rep, char *arg) {
  size_t len;
  char c, *d;

  op->outopts.repq[rep] = (arg[0] == '+')?TRUE:FALSE;
  if (arg[0] == '-') {
    if (arg[2] != '\0')
      CLLONGWARN(arg);
  }
  else if (arg[2] != '\0') {   /* parse sort method */
    rep = G(rep);   /* future args are genargs only */
    if (!ISALPHA(arg[2]))
      d = arg + 2;
    else {
      d = arg + 3;
      c = TOLOWER(arg[2]);
      if (c == 'r')
	op->outopts.sortby[rep] = REQUESTS;
      else if (c == 's')
	op->outopts.sortby[rep] = REQUESTS7;
      else if (c == 'p')
	op->outopts.sortby[rep] = PAGES;
      else if (c == 'q')
	op->outopts.sortby[rep] = PAGES7;
      else if (c == 'b')
	op->outopts.sortby[rep] = BYTES;
      else if (c == 'c')
	op->outopts.sortby[rep] = BYTES7;
      else if (c == 'a')
	op->outopts.sortby[rep] = ALPHABETICAL;
      else if (c == 'd')
	op->outopts.sortby[rep] = DATESORT;
      else if (c == 'e')
	op->outopts.sortby[rep] = FIRSTDATE;
      else if (c == 'x')
	op->outopts.sortby[rep] = RANDOM;
      else {
	warn('C', TRUE, "Unknown sort method in command line option %s", arg);
	return;
      }
    }
    if (*d != '\0') {   /* parse floor */
      len = strlen(d);
      c = TOLOWER(*(d + len - 1));   /* final character */
      if (c != 'r' && c != 's' && c != 'p' && c != 'q' &&
	  c != 'b' && c != 'c' && c != 'd' && c != 'e') {
	if (d == arg + 2) {
	  warn('C', TRUE,
	       "No sort method or floor given in command line option %s", arg);
	  return;
	}     /* else deduce floor method */
	c = arg[2];
	memmove((void *)(arg + 2), (void *)(arg + 3), len + 1);
	if (c == 'a' || c == 'x')
	  *(d + len - 1) = 'r';
	else
	  *(d + len - 1) = c;
	configfloor((void *)&(op->outopts.floor[rep]), arg, arg + 2, NULL, -2);
      }
      else
	configfloor((void *)&(op->outopts.floor[rep]), arg, d, NULL, -2);
    }
  }
}

void cldebug(char **s, char *arg) {
  if (arg[0] == '-') {
    if (arg[2] != '\0')
      CLLONGWARN(arg);
    configdebug(s, arg, "FALSE", NULL, -2);
  }
  else if (arg[2] == '\0')
    configdebug(s, arg, "TRUE", NULL, -2);
  else
    configdebug(s, arg, arg + 2, NULL, -2);
}

void clargs(Options *op, int argc, char *argv[]) {
  extern char repcodes[];
  extern logical vblesonly;
  extern char *debug_args, *warn_args;
  int i;
  choice j;

  for (i = 1; i < argc; i++) {
    if (strlen(argv[i]) > 255) {
      argv[i][70] = '\0';
      warn('C', TRUE, "Ignoring long command line argument starting\n%s", argv[i]);
    }
    else if (!IS_EMPTY_STRING(argv[i])) {
      if (argv[i][0] != '+' && argv[i][0] != '-')
	configlogfile((void *)&(op->miscopts.logfile[0]), argv[i], argv[i],
		      NULL, -2);
      else switch (argv[i][1]) {
      case '\0':
	configlogfile((void *)&(op->miscopts.logfile[0]), argv[i], "stdin",
		      NULL, -2);
	break;
      case '1':
      case '4':
      case '5':
      case '6':
      case '7':
      case 'd':
      case 'D':
      case 'H':
      case 'h':
      case 'm':
      case 'P':
      case 'Q':
      case 'W':
      case 'w':
      case 'x':
      case 'z':
	for (j = 0; repcodes[j] != argv[i][1]; j++)
	  ;
	CLREPTOGGLE(j);
	break;
      case 'b':
      case 'B':
      case 'c':
      case 'E':
      case 'f':
      case 'i':
      case 'I':
      case 'j':
      case 'J':
      case 'k':
      case 'K':
      case 'l':
      case 'L':
      case 'M':
      case 'n':
      case 'N':
      case 'p':
      case 'o':
      case 'r':
      case 'R':
      case 'S':
      case 't':
      case 'u':
      case 'v':
      case 'y':
      case 'Y':
      case 'Z':
	for (j = 0; repcodes[j] != argv[i][1]; j++)
	  ;
	clgenrep(op, j, argv[i]);
	break;
      case 's':
	if (STREQ(argv[i] + 2, "ettings"))
	  vblesonly = TRUE;
	else
	  clgenrep(op, REP_REFSITE, argv[i]);
	break;
      case 'a':
	CLOUTSTYLE(op->outopts.outstyle);
	break;
      case 'A':
	CLLONGCHECK(configall((void *)(op->outopts.repq), argv[i],
			      (argv[i][0] == '+')?"ON":"OFF", NULL, -2));
	break;
      case 'C':
	CLSHORTCHECK(clconfline(op, argv[i] + 2));
	break;
      case 'F':
	if (argv[i][0] == '-') {
	  CLLONGCHECK(op->dman.fromstr = NULL);
	}
	else
	  CLSHORTCHECK(confline(op, "FROM", argv[i] + 2, NULL, -2));
	break;
      case 'g':
	CLSHORTCHECK((void)config(argv[i] + 2, op, TRUE));
	break;
      case 'G':  /* mandatory config file: already dealt with */
	break;
      case 'O':
	CLSHORTCHECK(confline(op, "OUTFILE", argv[i] + 2, NULL, -2));
	break;
      case 'q':
	cldebug(&warn_args, argv[i]);
	break;
      case 'T':
	if (argv[i][0] == '-') {
	  CLLONGCHECK(op->dman.tostr = NULL);
	}
	else
	  CLSHORTCHECK(confline(op, "TO", argv[i] + 2, NULL, -2));
	break;
      case 'U':
	CLSHORTCHECK(confline(op, "CACHEFILE", argv[i] + 2, NULL, -2));
	break;	
      case 'V':
	cldebug(&debug_args, argv[i]);
	break;
      case 'X':
	CLGOTOS(op->outopts.gotos);
	break;
      case '-':
	if (STREQ(argv[i] + 2, "settings"))
	  vblesonly = TRUE;
	else
	  warn('C', TRUE, "Ignoring unknown command line argument %s",
	       argv[i]);  /* --help and --version are detected earlier, */
	break;            /* in settings() */
      default:
	warn('C', TRUE, "Ignoring unknown command line argument %s", argv[i]);
	break;
      }
    }
  }
}
