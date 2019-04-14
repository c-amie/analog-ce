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

/*** output.c; controls the output, mostly calling fns from output2.c ***/

#include "anlghea3.h"

extern choice *rep2type, *rep2reqs, *rep2reqs7, *rep2date, *rep2firstd;
extern unsigned int *rep2colhead, *rep2gran, *rep2datefmt;

void pagetop(FILE *outf, Outchoices *od, Dateman *dman) {
  extern logical cgi;

  /* CGI header */
  if (cgi)
    od->outputter->cgihead(outf, od);

  /* Header material for this output style */
  od->outputter->stylehead(outf, od);

  /* The page title, plus the user's HEADERFILE */
  od->outputter->pagetitle(outf, od);

  /* Program start time, and logfile start and end times */
  od->outputter->timings(outf, od, dman);

  /* The bottom of the top of the page */
  od->outputter->closehead(outf, od);
}

void pagebot(FILE *outf, Outchoices *od) {
  extern time_t origstarttime;

  long secs;

  /* Start of the bottom of the page */
  od->outputter->pagebotstart(outf, od);

  /* The credit line */
  od->outputter->credit(outf, od);

  /* The program run time */
  if (od->runtime) {
    secs = (long)difftime(time((time_t *)NULL), origstarttime);
    od->outputter->runtime(outf, od, secs);
  }

  /* The page footer, including the user's FOOTERFILE */
  od->outputter->pagefoot(outf, od);

  /* Footer material for this output style */
  od->outputter->stylefoot(outf, od);
}

#define GENSUM_RATE(x, m) (((m) < 30 || (x) <= 1)?((unsigned long)UNSET):\
  ((unsigned long)(((double)((x) - 1) * 1440.0) / (double)(m))))
void gensum(FILE *outf, Outchoices *od,
	    unsigned long *data, double bys, double bys7,
	    Hashindex **gooditems, Hashindex **baditems, Dateman *dman,
	    choice data2cols[ITEM_NUMBER][DATA_NUMBER]) {
  timecode_t totmins, totmins7;
  logical q7 = (od->last7 && dman->firsttime < dman->last7from &&
		dman->last7from < dman->lasttime);
  unsigned long tot, tot7;

  od->outputter->gensumhead(outf, od);

  totmins = dman->lasttime - dman->firsttime;
  totmins7 = q7?MINS_IN_WEEK:0;
  if (q7)
    od->outputter->lastseven(outf, od, dman->last7to);
  gensumline(outf, od, '\0', succreqs_, data[LOGDATA_SUCC],
	     q7?data[LOGDATA_SUCC7]:(unsigned long)UNSET, FALSE);
  /* \0 above means always printed */
  gensumline(outf, od, 'B', avereqs_,
	     GENSUM_RATE(data[LOGDATA_SUCC], totmins),
	     GENSUM_RATE(data[LOGDATA_SUCC7], totmins7), TRUE);
  gensumline(outf, od, 'C', totunknown_, data[LOGDATA_UNKNOWN],
	     q7?data[LOGDATA_UNKNOWN7]:(unsigned long)UNSET, FALSE);
  gensumline(outf, od, 'D', totpages_, data[LOGDATA_PAGES],
	     q7?data[LOGDATA_PAGES7]:(unsigned long)UNSET, FALSE);
  gensumline(outf, od, 'E', avepages_,
	     GENSUM_RATE(data[LOGDATA_PAGES], totmins),
	     GENSUM_RATE(data[LOGDATA_PAGES7], totmins7), TRUE);
  gensumline(outf, od, 'F', totfails_, data[LOGDATA_FAIL],
	     q7?data[LOGDATA_FAIL7]:(unsigned long)UNSET, FALSE);
  gensumline(outf, od, 'G', totredirs_, data[LOGDATA_REDIR],
	     q7?data[LOGDATA_REDIR7]:(unsigned long)UNSET, FALSE);
  gensumline(outf, od, 'H', inforeqs_, data[LOGDATA_INFO],
	     q7?data[LOGDATA_INFO7]:(unsigned long)UNSET, FALSE);
  distcount(gooditems[ITEM_FILE], baditems[ITEM_FILE],
	    data2cols[ITEM_FILE][REQUESTS], data2cols[ITEM_FILE][REQUESTS7],
	    &tot, &tot7);
  gensumline(outf, od, 'I', distfiles_, tot, q7?tot7:(unsigned long)UNSET,
	     FALSE);
  distcount(gooditems[ITEM_HOST], baditems[ITEM_HOST],
	    data2cols[ITEM_HOST][REQUESTS], data2cols[ITEM_HOST][REQUESTS7],
	    &tot, &tot7);
  gensumline(outf, od, 'J', disthosts_, tot, q7?tot7:(unsigned long)UNSET,
	     FALSE);
  gensumline(outf, od, 'K', corrupt_, data[LOGDATA_CORRUPT],
	     (unsigned long)UNSET, FALSE);
  gensumline(outf, od, 'L', unwanted_, data[LOGDATA_UNWANTED],
	     (unsigned long)UNSET, FALSE);
  gensumlineb(outf, od, 'M', totdata_, bys, q7?bys7:UNSET, FALSE);
  gensumlineb(outf, od, 'N', avedata_,
	      (totmins < 30)?UNSET:((bys * 1440.0) / (double)totmins),
	      (q7 && totmins7 >= 30)?\
	      ((bys7 * 1440.0) / (double)totmins7):UNSET, TRUE);

  od->outputter->gensumfoot(outf, od);
}

void timerep(FILE *outf, Outchoices *od, choice rep, Dateman *dman,
	     unsigned int granularity) {
  extern unsigned int daysbefore[12];

  char **lngstr = od->lngstr;
  logical back = od->back[rep];
  unsigned int rows = od->rows[rep];
  char graphby = od->graph[rep];
  unsigned int repgran = rep2gran[rep];
  char *datefmt = od->lngstr[rep2datefmt[rep]];

  Daysdata *dp;
  Timerep *trhead, *trp, *oldtrp;
  unsigned int hr, min, newhr, newmin = 59, busytime = 0;
  datecode_t busydate = 0, i;
  int j, firsttime, lasttime;
  unsigned int k;
  unsigned int relgran = granularity / repgran; /* guaranteed to be integer */
  unsigned long reqs = 0, pages = 0, totr = 0, totp = 0;
  unsigned long maxr = 0, maxp = 0, busyr = 0, busyp = 0;
  double bys = 0.0, totb = 0.0, maxb = 0.0, busyb = 0.0, unit = 0.0;
  unsigned int width[COL_NUMBER], bmult, bmult7, date, month, year;
  unsigned int accum = 0, intlength = 0, rowsdone = 0;
  logical save, first = TRUE, leavegap;

  /* First work out what will be listed and shove them in an array */

  if (rows == 0)
    rows = INT_MAX;
  if (rep == REP_YEAR) {
    accum = 12;
    intlength = 364;
    /* The intlength is for calculating the date at the end of a time interval.
       Of course years, quarters and months differ in length, and we don't
       bother to calculate the date exactly. We assume that the language file
       only asks for the month at the end, not the precise date. */
  }
  else if (rep == REP_QUARTERLY) {
    accum = 3;
    intlength = 89;
  }
  else if (rep == REP_MONTH)
    accum = 1;
  else if (rep == REP_WEEK)
    intlength = 6;

  trhead = (Timerep *)xmalloc(sizeof(Timerep));
  trp = trhead;
  for (firsttime = 0; firsttime < (int)granularity &&
	 dman->firstdp->reqs[firsttime] == 0; firsttime++)
    ;    /* run to first time */
  for (lasttime = granularity - 1;
       lasttime >= 0 && dman->lastdp->reqs[lasttime] == 0; lasttime--)
    ;
  for (i = dman->lastdate, dp = dman->lastdp; i >= dman->firstdate;
       i--, dp = dp->prev) {
    for (j = (int)(granularity - relgran); j >= 0; j -= relgran) {
      for (k = 0; k < relgran; k++) {
	reqs += dp->reqs[j + k];
	pages += dp->pages[j + k];
	bys += dp->bytes[j + k];
      }
      if (accum > 0) {  /* REP_YEAR, REP_QUARTERLY, REP_MONTH */
	code2date(i, &date, &month, &year);
	save = (date == 1 && month % accum == 0) || (i == dman->firstdate);
      }
      else if (rep == REP_WEEK)
	save = (DAYOFWEEK(i) == od->weekbeginson) || (i == dman->firstdate);
      else  /* REP_DAYREP or below */
	save = ((dp != dman->lastdp || j <= lasttime) &&
		(dp != dman->firstdp || j + (int)relgran > firsttime));
      if (save) {
	while (rep == REP_WEEK && DAYOFWEEK(i) != od->weekbeginson && i > 1)
	  i--;  /* earliest week: account under first day of week */
	/* (i > 1 above stops us crashing by going back further than 1/1/70) */
	if (rep == REP_QUARTERLY && i == dman->firstdate)
	  i = DATE2CODE(year, (month / 3) * 3, 1);
	/* sim. first date of quarter: month, year were calculated above */
	totr += reqs;
	totp += pages;
	totb += bys;
	if (((graphby == 'R' || graphby == 'r') && reqs >= busyr) ||
	    ((graphby == 'P' || graphby == 'p') && pages >= busyp) ||
	    ((graphby == 'B' || graphby == 'b') && bys >= busyb)) {
	  busyr = reqs;
	  busyp = pages;
	  busyb = bys;
	  busydate = i;   /* busydate always set coz busyr was init. to 0 */
	  busytime = (1440 * j) / granularity;
	}
	if (rowsdone < rows) {
	  maxr = MAX(maxr, reqs);
	  maxp = MAX(maxp, pages);
	  maxb = MAX(maxb, bys);
	  trp->prev = (Timerep *)xmalloc(sizeof(Timerep));
	  trp->prev->next = trp;
	  trp = trp->prev;
	  trp->reqs = reqs;
	  trp->pages = pages;
	  trp->bytes = bys;
	  trp->date = i;
	  trp->time = (1440 * j) / granularity;
	  trp->prev = NULL;
	  rowsdone++;
	}
	reqs = 0;
	pages = 0;
	bys = 0.0;
      }
    }
  }

  /* Now print the stuff we've saved */

  width[COL_TITLE] = MAX(datefmtlen(od, datefmt),
			 od->outputter->strlength(lngstr[rep2colhead[rep]]));
  od->outputter->calcwidths(od, rep, width, &bmult, &bmult7, &unit,
			maxr, 0, maxp, 0, maxb, 0, 0);
  od->outputter->declareunit(outf, od, graphby, unit, bmult);
  od->outputter->prestart(outf, od);
  colheads(outf, od, rep, width, bmult, bmult7, TRUE);
  for (trp = back?(trhead->prev):trp; trp != (back?NULL:trhead); ) {
    hr = trp->time / 60;
    min = trp->time % 60;
    if (rep == REP_QUARTERLY || rep == REP_MONTH) {
      code2date(trp->date, &date, &month, &year);
      leavegap = (month == (unsigned int)(back?(12 - accum):0));
    }
    else if (rep == REP_DAYREP)
      leavegap = (DAYOFWEEK(trp->date) ==
		  (back?((od->weekbeginson + 6) % 7):(od->weekbeginson)));
    else if (rep == REP_HOURREP)
      leavegap = (hr == (unsigned int)(back?23:0));
    else if (rep == REP_QUARTERREP)
      leavegap = (min == (unsigned int)(back?45:0) &&
		  (hr % 4) == (unsigned int)(back?3:0));
    else if (rep == REP_FIVEREP)
      leavegap = (min == (unsigned int)(back?55:0));
    else /* REP_YEAR or REP_WEEK */
      leavegap = FALSE;
    if (leavegap && !first)
      od->outputter->blankline(outf, od, od->cols[rep]);
    first = FALSE;
    if (rep == REP_QUARTERREP)
      newmin = (min + 15) % 60;
    else if (rep == REP_FIVEREP)
      newmin = (min + 5) % 60;
    else if (rep == REP_HOURREP)
      newmin = 0;
    if (newmin == 59)  /* REP_DAYREP or above */
      newhr = 23;
    else
      newhr = (newmin == 0)?(hr + 1):(hr);
    printcols(outf, od, rep, trp->reqs, 0, trp->pages, 0, trp->bytes, 0., -1,
	      -1, totr, 0, totp, 0, totb, 0, width, bmult, bmult7, unit, TRUE,
	      FALSE, NULL, FALSE, 0, NULL, NULL, datefmt, NULL, trp->date, hr,
	      min, trp->date + intlength, newhr, newmin);
    oldtrp = trp;
    trp = back?(trp->prev):(trp->next);
    free((void *)oldtrp);
  }
  od->outputter->preend(outf, od);
  hr = busytime / 60;
  min = busytime % 60;
  if (rep == REP_QUARTERREP)
    newmin = (min + 15) % 60;
  else if (rep == REP_FIVEREP)
    newmin = (min + 5) % 60;
  else if (rep == REP_HOURREP)
    newmin = 0;
  if (newmin == 59)  /* REP_DAYREP or above */
    newhr = 23;
  else
    newhr = (newmin == 0)?(hr + 1):(hr);
  od->outputter->busyprintf(outf, od, rep, datefmt, busyr, busyp, busyb,
			    busydate, hr, min, busydate + intlength, newhr,
			    newmin, graphby);
}

void timesum(FILE *outf, Outchoices *od, choice rep, Dateman *dman,
	     unsigned int granularity) {
  unsigned int repgran = rep2gran[rep];
  unsigned int repspan = (rep == REP_DAYSUM || rep == REP_WEEKHOUR)?7:1;
  char *datefmt = od->lngstr[rep2datefmt[rep]];

  Daysdata *dp;
  unsigned int relgran = granularity / repgran; /* guaranteed to be integer */
  unsigned int bins = repspan * repgran;
  unsigned long *reqs, *pages, totr = 0, totp = 0;
  double *bys, totb = 0.0, unit = 0.0;
  unsigned int width[COL_NUMBER], bmult, bmult7;
  logical first = TRUE;
  datecode_t date;
  unsigned int weekday, offset, entry, hr, min, newmin, i, j;

  reqs = (unsigned long *)xmalloc(bins * sizeof(unsigned long));
  pages = (unsigned long *)xmalloc(bins * sizeof(unsigned long));
  bys = (double *)xmalloc(bins * sizeof(double));
  for (i = 0; i < bins; i++) {
    reqs[i] = 0;
    pages[i] = 0;
    bys[i] = 0.0;
  }
  
  for (date = dman->firstdate, dp = dman->firstdp; date <= dman->lastdate;
       date++, TO_NEXT(dp)) {
    offset = (repspan == 1)?0:(DAYOFWEEK(date) * repgran);
    for (i = 0; i < granularity; i++) {
      entry = offset + i / relgran;
      reqs[entry] += dp->reqs[i];
      totr += dp->reqs[i];
      pages[entry] += dp->pages[i];
      totp += dp->pages[i];
      bys[entry] += dp->bytes[i];
      totb += dp->bytes[i];
    }
  }

  width[COL_TITLE] = MAX(datefmtlen(od, datefmt),
			 od->outputter->strlength(od->lngstr[rep2colhead[rep]]));
  od->outputter->calcwidths(od, rep, width, &bmult, &bmult7, &unit,
			arraymaxl(reqs, bins), 0, arraymaxl(pages, bins), 0,
			arraymaxd(bys, bins), 0, 0);
  od->outputter->declareunit(outf, od, od->graph[rep], unit, bmult);
  od->outputter->prestart(outf, od);
  colheads(outf, od, rep, width, bmult, bmult7, TRUE);
  for (i = 0; i < repspan; i++) {
    weekday = ((od->weekbeginson + i) % 7); 
    offset = (repspan == 1)?0:(weekday * repgran);
    for (j = 0; j < repgran; j++) {
      entry = offset + j;
      hr = j * 24 / repgran;
      min = (j * 1440 / repgran) % 60;
      newmin = ((j + 1) * 1440 / repgran) % 60;
      if (!first && min == 0 &&
	  ((repgran == 24 && hr == 0) || (repgran == 96 && hr % 4 == 0) ||
	   repgran == 288))
	od->outputter->blankline(outf, od, od->cols[rep]);
      printcols(outf, od, rep, reqs[entry], 0, pages[entry], 0, bys[entry], 0.,
		-1, -1, totr, 0, totp, 0, totb, 0, width, bmult, bmult7, unit,
		TRUE, FALSE, NULL, FALSE, 0, NULL, NULL, datefmt, NULL,
		weekday + 4, hr, min, weekday + 4, (newmin == 0)?(hr + 1):(hr),
		newmin);
      /* weekday + 4 is an arbitrary (internal) date that is that weekday. */
      first = FALSE;
    }
  }
  od->outputter->preend(outf, od);
}

void printtree(FILE *outf, Outchoices *od, choice rep, Hashtable *tree,
	       choice requests, choice requests7, choice pages, choice pages7,
	       choice date, choice firstd, unsigned int level,
	       Strlist *partname, unsigned long totr, unsigned long totr7,
	       unsigned long totp, unsigned long totp7, double totb,
	       double totb7, unsigned int width[], logical possrightalign,
	       unsigned int bmult, unsigned int bmult7, double unit) {
  /* level is 0 at the top level in this function */
  char **lngstr = od->lngstr;
  Include *linkhead = od->link[G(rep)];
  char *baseurl = (rep2type[rep] == ITEM_FILE)?(od->baseurl):NULL;

  char *name;
  size_t need = (size_t)level + 3;
  logical rightalign;
  unsigned long datar, datar7, datap, datap7, datad, datafd;
  Hashindex *p;
  Strlist *pn, s;
  unsigned long goodn = 0;

  if (tree != NULL) {
    for (pn = partname; pn != NULL; TO_NEXT(pn))
      need += strlen(pn->name);
    for (p = tree->head[0]; p != NULL; TO_NEXT(p)) {
      name = maketreename(partname, p, &pn, &s, need, rep, TRUE);
      /* name construction also in findwedges() */
      if (STREQ(name, LNGSTR_NODOMAIN) || STREQ(name, LNGSTR_UNKDOMAIN) ||
	  ISDIGIT(name[strlen(name) - 1]))
	rightalign = FALSE;
      else
	rightalign = possrightalign;
      datar = p->own->data[requests];
      datar7 = (requests7 >= 0)?(p->own->data[requests7]):0;
      datap = (pages >= 0)?(p->own->data[pages]):0;
      datap7 = (pages7 >= 0)?(p->own->data[pages7]):0;
      datad = (date >= 0)?(p->own->data[date]):0;
      datafd = (firstd >= 0)?(p->own->data[firstd]):0;
      printcols(outf, od, rep, datar, datar7, datap, datap7, p->own->bytes,
		p->own->bytes7, (level == 0)?((long)(++goodn)):(-1),
		(int)level + 1, totr, totr7, totp, totp7, totb, totb7, width,
		bmult, bmult7, unit, FALSE, rightalign, name,
		(logical)(p->own->ispage), 2 * level, linkhead, baseurl,
		lngstr[genrepdate_], lngstr[genreptime_],
		(datecode_t)(datad / 1440),
		(unsigned int)((datad % 1440) / 60),
		(unsigned int)(datad % 60), (datecode_t)(datafd / 1440),
		(unsigned int)((datafd % 1440) / 60),
		(unsigned int)(datafd % 60));
      printtree(outf, od, rep, (Hashtable *)(p->other), requests, requests7,
		pages, pages7, date, firstd, level + 1, pn, totr, totr7, totp,
		totp7, totb, totb7, width, possrightalign, bmult, bmult7,
		unit);
    }
  }
}

void genrep(FILE *outf, Outchoices *od, choice rep, Hashindex **gooditems,
	    Hashindex **baditems,
	    choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2],
	    choice *data2cols, unsigned int data_number, Dateman *dman) {
  extern logical *repistree;
  extern char *workspace;

  logical istree = repistree[G(rep)];
  char **lngstr = od->lngstr;
  char *colhead = lngstr[rep2colhead[rep]];
  logical alphaback = (logical)(rep == REP_ORG || rep == REP_HOST ||
				rep == REP_VHOST);
  Tree *treex = od->tree[G(rep)];
  /* data2cols == NULL for arrayreps */
  choice requests = (data2cols == NULL)?REQUESTS:data2cols[rep2reqs[G(rep)]];
  choice requests7 =
    (data2cols == NULL)?REQUESTS7:data2cols[rep2reqs7[G(rep)]];
  choice pages = (data2cols == NULL)?PAGES:data2cols[PAGES];
  choice pages7 = (data2cols == NULL)?PAGES7:data2cols[PAGES7];
  choice date = (data2cols == NULL)?SUCCDATE:data2cols[rep2date[G(rep)]];
  choice firstd = (data2cols == NULL)?SUCCFIRSTD:data2cols[rep2firstd[G(rep)]];

  Hashtable *tree = NULL;  /* Just to keep compiler happy */
  Hashindex *p;
  Hashentry *badp;
  unsigned long datar, datar7, datap, datap7, datad, datafd;
  unsigned long totr, totr7, totp, totp7, i = 0;
  unsigned long maxr, maxr7, maxp, maxp7, goodn, badn;
  double totb, totb7, maxb, maxb7, unit = 1.0;
  timecode_t maxd, mind;
  unsigned int width[COL_NUMBER], bmult, bmult7, tw = 0;
  logical possrightalign = FALSE, rightalign, templ = FALSE;

  /* If istree, construct the tree. (Otherwise we shall use *gooditems.)
     (These two cases used to be separate functions, and it may still help to
     think of them that way.) */
  if (istree) {
    for (i = 0; od->alltrees[i] != REP_NUMBER; i++)
      if (rep == od->alltrees[i])
	templ = TRUE;   /* tree already constructed */
    if (!templ)
      maketree(treex, *gooditems, *baditems, datacols, data_number);
    tree = treex->tree;

  /* Apply the sort. This also sets tot*, max* and bad*. */
    tree->head[0] = sorttree(od, tree, rep, &(od->floor[G(rep)]),
			     od->sortby[G(rep)], &(od->subfloor[G(rep)]),
			     od->subsortby[G(rep)], alphaback, 0, NULL,
			     (rep == REP_DOM)?(od->aliashead[G(rep)]):NULL,
			     requests, requests7, pages, pages7, date, firstd,
			     &totr, &totr7, &totp, &totp7, &totb, &totb7,
			     &maxr, &maxr7, &maxp, &maxp7, &maxb, &maxb7,
			     &maxd, &mind, &badp, &badn, treex->space,
			     datacols);
  }  /* istree */
  else
    my_sort(gooditems, baditems, NULL, NULL, NULL, 0, -1, &(od->floor[G(rep)]),
	    od->sortby[G(rep)], alphaback, od->wanthead[G(rep)], requests,
	    requests7, pages, pages7, date, firstd, &totr, &totr7, &totp,
	    &totp7, &totb, &totb7, &maxr, &maxr7, &maxp, &maxp7, &maxb, &maxb7,
	    &maxd, &mind, FALSE, &badp, &badn,
	    (logical)(rep == REP_SIZE || rep == REP_PROCTIME));
  if (rep == REP_SIZE || rep == REP_PROCTIME) {
    /* These not sorted so as not to get rid of 0's and to preserve order */
    badp = newhashentry(DATA_NUMBER, FALSE);
    badn = 0;
  }

  /* Now calculate column sizes */
  for (p = istree?(tree->head[0]):(*gooditems), goodn = 0; p != NULL;
       TO_NEXT(p))
    goodn++;
  if (rep == REP_SIZE || rep == REP_PROCTIME) {
    width[COL_TITLE] = od->outputter->strlength(colhead);
    for (p = *gooditems; p != NULL; TO_NEXT(p))
      width[COL_TITLE] = MAX(strlen(p->name), /* no HTML codes used */
			     width[COL_TITLE]);
  }
  else
    width[COL_TITLE] = 0;
  od->outputter->calcwidths(od, rep, width, &bmult, &bmult7, &unit, maxr,
			    maxr7, maxp, maxp7, maxb, maxb7, goodn);
  if (alphaback && od->sortby[G(rep)] == ALPHABETICAL) {
    if (istree)
      tw = alphatreewidth(od, rep, tree, 0, NULL);
    else for (p = *gooditems; p != NULL; TO_NEXT(p)) {
      strcpy(workspace, p->name);
      do_aliasx(workspace, od->aliashead[G(rep)]);
      tw = MAX(tw, od->outputter->strlength(workspace));
    }
    width[COL_TITLE] = MIN(tw, width[COL_TITLE]);
    possrightalign = TRUE;
  }

  /* Print header material */
  if (od->repspan)
    reportspan(outf, od, rep, maxd, mind, dman);
#ifndef NOGRAPHICS
  if (od->outstyle == HTML || od->outstyle == XHTML)
    piechart(outf, od, rep, istree?(tree->head[0]):(*gooditems), requests,
	     requests7, pages, pages7, totr, totr7, totp, totp7, totb, totb7);
#endif
  if (rep != REP_SIZE && rep != REP_PROCTIME)
    od->outputter->whatincluded(outf, od, rep, goodn, dman);
  od->outputter->prestart(outf, od);
  colheads(outf, od, rep, width, bmult, bmult7,
	   (logical)(rep == REP_SIZE || rep == REP_PROCTIME));

  /* Print the items. Reuse goodn here. */
  if (istree)
    printtree(outf, od, rep, tree, requests, requests7, pages, pages7, date,
	      firstd, 0, NULL, totr, totr7, totp, totp7,
	      totb, totb7, width, possrightalign, bmult, bmult7, unit);
  else {
    for (goodn = 0, p = *gooditems; p != NULL; TO_NEXT(p)) {
      if (possrightalign &&
	  (!ISDIGIT(p->name[0]) || !ISDIGIT(p->name[strlen(p->name) - 1])))
	rightalign = TRUE;
      else
	rightalign = FALSE;
      datar = p->own->data[requests];
      datar7 = (requests7 >= 0)?(p->own->data[requests7]):0;
      datap = (pages >= 0)?(p->own->data[pages]):0;
      datap7 = (pages7 >= 0)?(p->own->data[pages7]):0;
      datad = (date >= 0)?(p->own->data[date]):0;
      datafd = (firstd >= 0)?(p->own->data[firstd]):0;
      printcols(outf, od, rep, datar, datar7, datap, datap7, p->own->bytes,
		p->own->bytes7, (long)(++goodn), 0, totr, totr7, totp, totp7,
		totb, totb7, width, bmult, bmult7, unit,
		(logical)(rep == REP_SIZE || rep == REP_PROCTIME),
		rightalign, p->name, p->own->ispage, 0, NULL, NULL,
		lngstr[genrepdate_], lngstr[genreptime_],
		(datecode_t)(datad / 1440),
		(unsigned int)((datad % 1440) / 60),
		(unsigned int)(datad % 60), (datecode_t)(datafd / 1440),
		(unsigned int)((datafd % 1440) / 60),
		(unsigned int)(datafd % 60));
    }
  }

  /* Print the "not listed" line and wind up. */
  if (badn > 0) {
    datar = badp->data[requests];
    datar7 = (requests7 >= 0)?(badp->data[requests7]):0;
    datap = (pages >= 0)?(badp->data[pages]):0;
    datap7 = (pages7 >= 0)?(badp->data[pages7]):0;
    datad = (date >= 0)?(badp->data[date]):0;
    datafd = (firstd >= 0)?(badp->data[firstd]):0;
    printcols(outf, od, rep, datar, datar7, datap, datap7, badp->bytes,
	      badp->bytes7, -1, istree, totr, totr7, totp, totp7, totb, totb7,
	      width, bmult, bmult7, unit,
	      (logical)(rep == REP_SIZE || rep == REP_PROCTIME), FALSE, NULL,
	      FALSE, 0, NULL, NULL, lngstr[genrepdate_], lngstr[genreptime_],
	      (datecode_t)(datad / 1440), (unsigned int)((datad % 1440) / 60),
	      (unsigned int)(datad % 60), (datecode_t)(datafd / 1440),
	      (unsigned int)((datafd % 1440) / 60),
	      (unsigned int)(datafd % 60));
    od->outputter->notlistedstr(outf, od, rep, badn);
    od->outputter->rowend(outf, od, rep);
  }
  od->outputter->preend(outf, od);
  if (istree)
    freemm(treex->space);
}

void dervrep(FILE *outf, Outchoices *od, choice rep, Hashindex *gooditems,
	     Hashindex *baditems,
	     choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2],
	     choice *data2cols, unsigned int data_number, Dateman *dman) {
  Derv *derv = od->derv[rep - FIRST_DERVREP];

  Hashindex *good = NULL, *bad = NULL;
  choice i;
  logical templ = FALSE;
  /* strategy: build list of items then hand off to genrep() */

  for(i = 0; od->alldervs[i] != REP_NUMBER; i++)
    if (rep == od->alldervs[i])
      templ = TRUE;
  if (!templ)
    makederived(derv, gooditems, baditems, od->convfloor, od->multibyte, rep,
		datacols, data_number);
  unhash(derv->table, &good, &bad);
  genrep(outf, od, rep, &good, &bad, datacols, data2cols, data_number, dman);
}

/* names for each bucket in arrayrep: see thresholds at top of defaults()...
   in globals.c. (Names for status codes and file sizes are in the language
   files). */

char *ptnames1[] =
{"0", "<= 0.01", "0.01-0.02", "0.02-0.05", "0.05-0.1 ", "0.1 -0.2 ",
 "0.2 -0.5 ", "0.5 -1   ", "1-  2 ", "2-  5 ", "5- 10 ", "10- 20 ",
 "20- 60 ", "60-120 ", "120-300 ", "300-600 ", "> 600 "};

char *ptnames2[] =
{"0", "", "", "", "", "", "", "1", "2", "3-  5", "5- 10", "10- 20", "20- 60",
 "60-120", "120-300", "300-600", "> 600"};
/* empty string signifies don't use */

void arrayrep(FILE *outf, Outchoices *od, choice rep, Arraydata *array,
	      Dateman *dman) {
  extern Memman *xmemman;

  Hashindex *good = NULL, *bad = NULL, *gp = NULL;
  char **names;
  choice i, lasti;
  logical done;
  /* strategy: construct list of (Hashindex *), and pass to genrep() */

  if (rep == REP_CODE)
    names = &(od->lngstr[code100_]);
  else if (rep == REP_SIZE)
    names = &(od->lngstr[filesize0_]);
  else if (array[1].reqs + array[2].reqs + array[3].reqs + array[4].reqs +
	   array[5].reqs + array[6].reqs == 0)  /* assume %t not %T */
    names = ptnames2;
  else
    names = ptnames1;

  /* calculate lasti */
  for (lasti = 0; array[lasti].threshold >= -0.5; lasti++)
    ;
  if (rep != REP_CODE) {
    for ( ; array[lasti].reqs == 0; lasti--)
      ;
  }
  for (i = 0, done = FALSE; !done; i++) {
    if (array[i].reqs > 0 || (rep != REP_CODE && !IS_EMPTY_STRING(names[i]))) {
      if (good == NULL) {
	gp = (Hashindex *)submalloc(xmemman, sizeof(Hashindex));
	good = gp;
      }
      else {
	gp->next = (Hashindex *)submalloc(xmemman, sizeof(Hashindex));
	TO_NEXT(gp);
      }
      gp->name = names[i];
      gp->own = newhashentry(DATA_NUMBER, FALSE);
      gp->own->data[REQUESTS] = array[i].reqs;
      gp->own->data[REQUESTS7] = array[i].reqs7;
      gp->own->data[PAGES] = array[i].pages;
      gp->own->data[PAGES7] = array[i].pages7;
      gp->own->data[SUCCDATE] = array[i].lastdate;
      gp->own->data[SUCCFIRSTD] = array[i].firstdate;
      gp->own->bytes = array[i].bytes;
      gp->own->bytes7 = array[i].bytes7;
      gp->next = NULL;
    }
    if (i == lasti)
      done = TRUE;
  }
  genrep(outf, od, rep, &good, &bad, NULL, NULL, 0, dman);
}

void output(Outchoices *od, Hashindex **gooditems, Hashindex **baditems,
	    Dateman *dman, Arraydata **arraydata, unsigned long *sumdata,
	    double totbytes, double totbytes7,
	    choice datacols[ITEM_NUMBER][OUTCOME_NUMBER][DATACOLS_NUMBER][2],
	    choice data2cols[ITEM_NUMBER][DATA_NUMBER], unsigned int *no_cols,
	    unsigned int granularity) {
  char *outfile = od->outfile;

  FILE *outf;
  int ro;
  choice rep;

  /* first open output file */
  
  if (IS_STDOUT(outfile)) {
    outf = stdout;
    debug('F', "Opening stdout as output file");
  }
  else {
    if ((outf = FOPENW(outfile)) == NULL)
      error("failed to open output file %s for writing", outfile);
    else {
      debug('F', "Opening %s as output file", outfile);
#ifdef RISCOS
      _swix(OS_File, _INR(0,2), 18, outfile, od->outputter->riscosfiletype());
      /* set filetype */
#endif
    }
  }

  /* remove any reports not wanted */

  checkreps(od, dman, gooditems, arraydata, data2cols);

  /* page header */

  pagetop(outf, od, dman);
    
  /* Now the main reports */

  for (ro = 0; od->reporder[ro] != -1; ro++) {
    rep = od->reporder[ro];

    if (od->repq[rep]) {
      report_title(outf, od, rep);
      switch(rep) {
      case (REP_GENSUM):
	gensum(outf, od, sumdata, totbytes, totbytes7, gooditems, baditems,
	       dman, data2cols);
	break;
      case (REP_YEAR):
      case (REP_QUARTERLY):
      case (REP_MONTH):
      case (REP_WEEK):
      case (REP_DAYREP):
      case (REP_HOURREP):
      case (REP_QUARTERREP):
      case (REP_FIVEREP):
	timerep(outf, od, rep, dman, granularity);
	break;
      case (REP_DAYSUM):
      case (REP_HOURSUM):
      case (REP_WEEKHOUR):
      case (REP_QUARTERSUM):
      case (REP_FIVESUM):
	timesum(outf, od, rep, dman, granularity);
	break;
      case (REP_HOST):
      case (REP_REDIRHOST):
      case (REP_FAILHOST):
      case (REP_BROWREP):
      case (REP_VHOST):
      case (REP_REDIRVHOST):
      case (REP_FAILVHOST):
      case (REP_USER):
      case (REP_REDIRUSER):
      case (REP_FAILUSER):
      case (REP_REQ):
      case (REP_REDIR):
      case (REP_FAIL):
      case (REP_REF):
      case (REP_REDIRREF):
      case (REP_FAILREF):
      case (REP_TYPE):
      case (REP_DIR):
      case (REP_DOM):
      case (REP_ORG):
      case (REP_REFSITE):
	genrep(outf, od, rep, &(gooditems[rep2type[rep]]),
	       &(baditems[rep2type[rep]]), datacols[rep2type[rep]],
	       data2cols[rep2type[rep]], no_cols[rep2type[rep]], dman);
	break;
      case (REP_SEARCHREP):
      case (REP_SEARCHSUM):
      case (REP_INTSEARCHREP):
      case (REP_INTSEARCHSUM):
      case (REP_BROWSUM):
      case (REP_OS):
	dervrep(outf, od, rep, gooditems[rep2type[rep]],
		baditems[rep2type[rep]], datacols[rep2type[rep]],
		data2cols[rep2type[rep]], no_cols[rep2type[rep]], dman);
	break;
      case (REP_SIZE):
      case (REP_CODE):
      case (REP_PROCTIME):
	arrayrep(outf, od, rep, arraydata[rep - FIRST_ARRAYREP], dman);
	break;
      }  /* end switch rep */
      report_footer(outf, od, rep);
    }    /* end if rep wanted */
  }      /* end for ro */

  /*** Bit at the bottom of the page ***/

  pagebot(outf, od);
  if (!IS_STDOUT(outfile)) {
    debug('F', "Closing %s", outfile);
    fclose(outf);
  }
}
