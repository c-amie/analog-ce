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

/*** cache.c; reading and writing cache files ***/

#include "anlghea3.h"

void outputcache(char *outfile, Hashindex **gooditems, Dateman *dman,
		 Arraydata **arraydata, unsigned long *data, double bys,
		 choice data2cols[ITEM_NUMBER][DATA_NUMBER]) {
  extern choice cache_records[];

  FILE *outf = NULL;
  Daysdata *dp;
  Hashindex *p;
  int i, j;
  datecode_t k;
  logical done;

  /* first open output file as in output.c */
  if (IS_STDOUT(outfile)) {
    outf = stdout;
    debug('F', "Opening stdout as cache output file");
  }
  else {
#ifdef NOOPEN
    /* The ANSI, but less preferred, option. There is a race problem. Also if
       we somehow have got overwrite access but not read, it will go wrong. */
    if ((outf = FOPENR(outfile)) != NULL) {
      fclose(outf);
      error("cache output file %s already exists: won't overwrite", outfile);
    }
    else if ((outf = FOPENW(outfile)) == NULL)
      error("failed to open cache output file %s for writing", outfile);
    else
      debug('F', "Opening %s as cache output file", outfile);
#else
    /* NFS is broken, so actually even this version can contain a race if the
       file is mounted over NFS. */
    if ((i = open(outfile, O_WRONLY | O_CREAT | O_EXCL, OPEN_MODE)) < 0) {
      if (errno == EEXIST)
	error("cache output file %s already exists: won't overwrite", outfile);
      else
	error("failed to open cache output file %s for writing", outfile);
    }
    else if ((outf = fdopen(i, "w")) == NULL)  /* can this happen? */
      error("failed to open cache output file %s for writing", outfile);
    else
      debug('F', "Opening %s as cache output file", outfile);
#endif
  }
  fprintf(outf,
	  "CACHE type 5 produced by analog %s. Do not modify or delete!\n",
	  VERSION);

  /* output the summary data */
  fprintf(outf, "T\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%.0f\n",
	  data[LOGDATA_UNKNOWN], data[LOGDATA_INFO], data[LOGDATA_SUCC],
	  data[LOGDATA_PAGES], data[LOGDATA_REDIR], data[LOGDATA_FAIL], bys);

  /* output the date info */
  if (dman->firsttime == LAST_TIME && dman->lasttime == FIRST_TIME)
    fprintf(outf, "D\t%lu\t%lu\n", dman->lasttime, dman->firsttime);
  else
    fprintf(outf, "D\t%lu\t%lu\n", dman->firsttime, dman->lasttime);
  for (k = dman->firstdate, dp = dman->firstdp; dp != NULL;
       k++, TO_NEXT(dp)) {          /* dp != NULL iff k <= dman->lastdate */
    for (i = 0; i < 288; i++) {
      if (dp->reqs[i] != 0)
	fprintf(outf, "%u\t%d\t%lu\t%lu\t%.0f\n", k, i, dp->reqs[i],
		dp->pages[i], dp->bytes[i]);
    }
  }

  /* output the items */
  for (i = 0; i < ITEM_NUMBER; i++) {
    for (p = gooditems[i]; p != NULL; TO_NEXT(p)) {
      if (p->own != NULL && (p->own->data[data2cols[i][REQUESTS]] != 0 ||
			     p->own->data[data2cols[i][FAIL]] != 0 ||
			     p->own->data[data2cols[i][REDIR]] != 0)) {
	fprintf(outf, "%d\t", i);
	for (j = 0; j < V5_DATA_NUMBER; j++)
	  fprintf(outf, "%lu\t", p->own->data[data2cols[i][cache_records[j]]]);
	fprintf(outf, "%.0f\t%s\n", p->own->bytes, p->name);
      }
    }
  }

  /* output the arraydata. For historical reasons, these all differ slightly */
  /* the sizes */
  for (i = 0, done = FALSE; !done; i++) {
    if (arraydata[REP_SIZE - FIRST_ARRAYREP][i].reqs > 0)
      fprintf(outf, "z\t%d\t%lu\t%lu\t%lu\t%.0f\n", i,
	      arraydata[REP_SIZE - FIRST_ARRAYREP][i].reqs,
	      arraydata[REP_SIZE - FIRST_ARRAYREP][i].pages,
	      arraydata[REP_SIZE - FIRST_ARRAYREP][i].lastdate,
	      arraydata[REP_SIZE - FIRST_ARRAYREP][i].bytes);
    if (arraydata[REP_SIZE - FIRST_ARRAYREP][i].threshold < -0.5)
      done = TRUE;
  }

  /* the status codes */
  for (i = 0, done = FALSE; !done; i++) {
    if (arraydata[REP_CODE - FIRST_ARRAYREP][i].reqs > 0)
      fprintf(outf, "c\t%u\t%lu\t%lu\n",
	      (unsigned int)(arraydata[REP_CODE - FIRST_ARRAYREP][i].threshold),
	      arraydata[REP_CODE - FIRST_ARRAYREP][i].reqs,
	      arraydata[REP_CODE - FIRST_ARRAYREP][i].lastdate);
    if (arraydata[REP_CODE - FIRST_ARRAYREP][i].threshold < -0.5)
      done = TRUE;
  }

  /* the processing times */
  for (i = 0, done = FALSE; !done; i++) {
    if (arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].reqs > 0)
      fprintf(outf, "P\t%lu\t%lu\t%lu\t%lu\t%.0f\n",
	      (arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].threshold < -0.5)?
	      ((unsigned long)(arraydata[REP_PROCTIME - FIRST_ARRAYREP][i - 1].threshold) + 1):
	      ((unsigned long)(arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].threshold)),
	      arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].reqs,
	      arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].pages,
	      arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].lastdate,
	      arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].bytes);
    if (arraydata[REP_PROCTIME - FIRST_ARRAYREP][i].threshold < -0.5)
      done = TRUE;
  }

  if (!IS_STDOUT(outfile)) {
    debug('F', "Closing %s", outfile);
    fclose(outf);
  }
}

void hashbuild(choice code, unsigned long *data, double bys, Memman *mm,
	       Hashtable **hash, unsigned int data_number,
	       choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2],
	       choice data2cols[], Include *wanthead, Include *ispagehead,
	       Tree **tree, Derv **derv, choice *alltrees,
	       choice *alldervs, choice lowmem, char *dirsuffix,
	       unsigned int dirsufflength, logical usercase_insensitive,
	       unsigned char convfloor, logical multibyte) {
  /* very simplified version of process_data(): don't do any more aliases */
  extern Hashentry *unwanted_entry;
  extern Hashindex *dummy_item;
  extern choice *rep2type;

  unsigned long data2[DATA2_NUMBER];
  Hashentry *item;
  char *namestart, *nameend;
  logical ispage, buildtree = FALSE;
  int i, k;
  
  if (lowmem <= 1)
    item = hashfind(mm, hash, data_number, wanthead, UNSET, ispagehead,
		    NULL, dirsuffix, dirsufflength, usercase_insensitive,
		    0, FALSE, code, TRUE)->own;
  else {
    ispage = pageq(mm->curr_pos, ispagehead, code);
    if (included(mm->curr_pos, ispage, wanthead)) {
      if (lowmem == 2)
	item = hashfind(mm, hash, data_number, wanthead, ispage, ispagehead,
			NULL, dirsuffix, dirsufflength, usercase_insensitive,
			0, FALSE, code, TRUE)->own;
      else {
	item = unwanted_entry;
	buildtree = TRUE;
      }
    }
    else {
      item = unwanted_entry;
      mm->next_pos = mm->curr_pos;
    }
  }

  if (ENTRY_WANTED(item)) {
    data2[REQUESTS2] = data[REQUESTS];
    data2[PAGES2] = data[PAGES];
    data2[DATE2] = data[SUCCDATE];
    data2[FIRSTD2] = data[SUCCFIRSTD];
    hashscore(item, data2, datacols[SUCCESS], SUCCESS, bys);
    data2[REQUESTS2] = data[REDIR];
    data2[DATE2] = data[REDIRDATE];
    data2[FIRSTD2] = data[REDIRFIRSTD];
    hashscore(item, data2, datacols[REDIRECT], REDIRECT, 0.0);
    data2[REQUESTS2] = data[FAIL];
    data2[DATE2] = data[FAILDATE];
    data2[FIRSTD2] = data[FAILFIRSTD];
    hashscore(item, data2, datacols[FAILURE], FAILURE, 0.0);
  }
  else if (buildtree) {  /* as in process.c, run through trees then dervs */
    for (k = 0; k <= 1; k++) {
      for (i = 0; (k?(alldervs[i]):(alltrees[i])) != REP_NUMBER; i++) {
	if (rep2type[k?(alldervs[i]):(alltrees[i])] == code) {
	  dummy_item->name = mm->curr_pos;
	  dummy_item->own->data[data2cols[REQUESTS]] = data[REQUESTS];
	  if (data2cols[PAGES] >= 0)
	    dummy_item->own->data[data2cols[PAGES]] = data[PAGES];
	  if (data2cols[SUCCDATE] >= 0)
	    dummy_item->own->data[data2cols[SUCCDATE]] = data[SUCCDATE];
	  if (data2cols[SUCCFIRSTD] >= 0)
	    dummy_item->own->data[data2cols[SUCCFIRSTD]] = data[SUCCFIRSTD];
	  dummy_item->own->bytes = bys;
	  if (k)
	    makederived(derv[alldervs[i] - FIRST_DERVREP], dummy_item, NULL,
			convfloor, multibyte, alldervs[i], datacols,
			data_number);
	  else {
	    namestart = NULL;
	    tree[G(alltrees[i])]->cutfn(&namestart, &nameend, dummy_item->name,
					FALSE);
	    (void)treefind(namestart, nameend, &(tree[G(alltrees[i])]->tree),
			   dummy_item, tree[G(alltrees[i])]->cutfn, FALSE,
			   TRUE, FALSE, tree[G(alltrees[i])]->space, datacols,
			   data_number);
	  }
	}
      }
    }
  }
}

void corruptcacheline(char *line, choice *rc, choice type) {
  extern char *pos;
  char *p;

  for (p = line; p < pos - 2 && p - line < 76; p++) {
    if (*p == '\0')
      *p = (type == 0 || type == 3)?'\t':':';
  }   /* new style : old style */
  if (pos - line > 76)
    *(line + 76) = '\0';
  if (type == 3)
    warn('F', TRUE, "Ignoring line in cache file with out-of-range dates looking like\n%s", line);
  else if (type == 2)
    warn('F', TRUE, "Incomplete line in cache file looking like\n%s", line);
  else  /* type == 1 or 0 */
    warn('F', TRUE, "Ignoring corrupt line in cache file looking like\n%s",
	 line);
  *rc = FALSE;
}

logical parsecacheline(char *linestart, char *parsestart, unsigned int *code,
		       unsigned int maxcode, int no_items, unsigned long *data,
		       double *bys, Memman *mm) {
  choice rc = TRUE;
  char *temps, *temps2;
  int i;

  /* Note that maxcode is greater than the greatest possible code */
  if (!ISDIGIT(*parsestart) ||
      (*code = (unsigned int)atoi(parsestart)) >= maxcode ||
      (temps2 = strchr(parsestart, '\t')) == NULL)
    corruptcacheline(linestart, &rc, 0);
  else {
    temps = strtok(temps2 + 1, "\t");
    for (i = 0; i < no_items; i++) {
      if (rc == TRUE) {
	if (temps == NULL || !ISDIGIT(*temps))
	  corruptcacheline(linestart, &rc, 0);
	else {
	  data[i] = strtoul(temps, (char **)NULL, 10);
	  temps = strtok((char *)NULL, "\t");
	}
      }
    }
    if (rc == TRUE && bys != NULL) {
      if (temps == NULL || !ISDIGIT(*temps))
	corruptcacheline(linestart, &rc, 0);
      else {
	*bys = strtod(temps, (char **)NULL);
	temps = strtok((char *)NULL, "\t");
      }
    }
    if (rc == TRUE && mm != NULL) {
      if (temps == NULL)
	corruptcacheline(linestart, &rc, 0);
      else {
	temps2 = strchr(temps, '\0');
	memcpy(submalloc(mm, (size_t)(temps2 - temps) + 1), (void *)temps,
	       (size_t)(temps2 - temps) + 1);
	temps = strtok((char *)NULL, "\t");
      }
    }
    if (rc == TRUE && temps != NULL)
      corruptcacheline(linestart, &rc, 0);
  }
  return((logical)rc);
}

void readoldcache(Logfile *cachefilep, Dateman *dman, choice type,
		  unsigned int granularity) {
  /* These very old caches only contained data over time */
  extern char *block_start, *block_end, *record_start, *pos;
  unsigned int y, m, d, h;
  unsigned long reqs = 0, pages = 0;
  double bys = 0.0;
  timecode_t timecode;
  choice rc = TRUE;
  char *temps;
  char tempc;
  int i;

  while (rc != EOF) {
    rc = TRUE;
    if (pos >= block_end) {
      if ((rc = getmoredata(cachefilep, block_start, BLOCKSIZE)) == OK)
	rc = TRUE;
      pos = block_start;
      record_start = pos;
    }
    if (rc == TRUE) {
      (void)parsenewline(cachefilep, NULL, '\0');
      record_start = pos;
      parsenonnewline(cachefilep);
      if (pos == record_start)
	rc = EOF;
      *(pos++) = '\0';
      if (rc == TRUE) {
	if (sscanf(record_start, "%4u%2u%2u%2u%c", &y, &m, &d, &h, &tempc) != 5
	    || tempc != ':' ||
	    wantdate(&timecode, dman, h, 0, d, m - 1, y, 0) == ERR)
	  corruptcacheline(record_start, &rc, 1);
	else {
	  strtok(record_start, ":");
	  for ( ; h < 24 && rc == TRUE; h++) {
	    if ((temps = strtok((char *)NULL, ":")) == NULL ||
		!ISDIGIT(*temps)) {
	      if (*temps == '*')
		rc = EOF;
	      else
		corruptcacheline(record_start, &rc, 2);
	    }
	    else {
	      reqs = strtoul(temps, (char **)NULL, 10);
	      if ((temps = strtok((char *)NULL, ":")) == NULL ||
		  !ISDIGIT(*temps))
		corruptcacheline(record_start, &rc, 2);
	      else if (type == 2) {
		pages = strtoul(temps, (char **)NULL, 10);
		if ((temps = strtok((char *)NULL, ":")) == NULL ||
		    !ISDIGIT(*temps))
		  corruptcacheline(record_start, &rc, 2);
		else
		  bys = strtod(temps, (char **)NULL);
	      }
	      else
		bys = strtod(temps, (char **)NULL);
	      for (i = 0; i < 12; i++) {
		if (wantdate(&timecode, dman, h, (unsigned int)(5 * i), d,
			     m - 1, y, 0))
		  datehash(timecode, dman,
			   ((i + 1) * reqs) / 12 - (i * reqs) / 12,
			   (type == 2)?\
			   (((i + 1) * pages) / 12 - (i * pages) / 12):0,
			   ((i + 1) * bys) / 12 - (i * bys) / 12, granularity);
		/* maybe shouldn't datehash unless rc == TRUE (cf readcache())
		   but I no longer understand it well enough to mess with it */
		cachefilep->from = MIN(cachefilep->from, timecode);
		cachefilep->to = MAX(cachefilep->to, timecode);
	      }
	      cachefilep->data[LOGDATA_SUCC] += reqs;
	      if (type == 2)
		cachefilep->data[LOGDATA_PAGES] += pages;
	      cachefilep->bytes += bys;
	      if (timecode > dman->last7from && timecode <= dman->last7to) {
		cachefilep->data[LOGDATA_SUCC7] += reqs;
		if (type == 2)
		  cachefilep->data[LOGDATA_PAGES7] += pages;
		cachefilep->bytes7 += bys;
	      }
	    }
	  }
	}
      }
    }
  }
}

void readcache(Logfile *cachefilep, Hashtable **hash, unsigned int *no_cols,
	       choice datacols[ITEM_NUMBER][OUTCOME_NUMBER][DATACOLS_NUMBER][2],
	       choice data2cols[ITEM_NUMBER][DATA_NUMBER],
	       Arraydata **arraydata, Include **wanthead, Include *ispagehead,
	       Dateman *dman, Tree **tree, Derv **derv,
	       choice *alltrees, choice *alldervs, choice *lowmem,
	       char *dirsuffix, unsigned int dirsufflength,
	       logical usercase_insensitive, unsigned char convfloor,
	       logical multibyte, unsigned int granularity) {
  /* compare with nextdnsline etc. in input.c */
  extern Memman mmq;  /* use mmq for another purpose here */
  extern choice cache_records[];
  extern char *block_start, *block_end, *record_start, *pos;
  extern unsigned int *rep2gran;

  unsigned long data[V5_DATA_NUMBER];
  unsigned long data2[DATA_NUMBER];
  int data_number;
  double bys;
  datecode_t datecode;
  timecode_t timecode, firsttime, lasttime;
  unsigned int code;
  choice rc = TRUE;
  logical rc2;
  char *temps;
  char type, tempc;
  int i;

  rc = getmoredata(cachefilep, block_start, BLOCKSIZE);
  pos = block_start;
  record_start = block_start;
  if (sscanf(pos, "CACHE type %c produced by analo%c", &type, &tempc) != 2
      || type < '1' || tempc != 'g') {
    warn('F', TRUE, "%s is not an analog cache file: ignoring it",
	 cachefilep->name);
    return;
  }
  else if (type > '5') {
    warn('F', TRUE, "Can't understand cache file %s written by a later version of analog: ignoring it", cachefilep->name);
    return;
  }
  parsenonnewline(cachefilep);

  if (type < '3')
    readoldcache(cachefilep, dman, (choice)(type - '0'), granularity);

  else { /* types 3 onwards are similar enough the same to use the same code */
    if (type <= '4')
      data_number = V34_DATA_NUMBER;
    else
      data_number = V5_DATA_NUMBER;
    (void)parsenewline(cachefilep, NULL, '\0');
    record_start = pos;
    parsenonnewline(cachefilep);
    if (sscanf(record_start, "T\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lf%c",
	       &(data[0]), &(data[1]), &(data[2]), &(data[3]), &(data[4]),
	       &(data[5]), &bys, &tempc) != 8 ||
	(tempc != '\r' && tempc != '\n')) {
      warn('F', TRUE, "Cache file %s is corrupt: ignoring it",
	   cachefilep->name); 
      return;
    }
    if (type <= '4') {
      for (i = 0, rc2 = FALSE; i < ITEM_NUMBER && !rc2; i++) {
	if (data2cols[i][SUCCFIRSTD] != -1 || data2cols[i][FAILFIRSTD] != -1 ||
	    data2cols[i][REDIRFIRSTD] != -1) {
	  warn('M', TRUE, "Old-style cache file %s doesn't contain data on "
	       "first-request times of items; so these may be overestimated",
	       cachefilep->name);
	  rc2 = TRUE;
	}
      }
    }
    if (type == '3') {
      firsttime = FIRST_TIME;
      lasttime = LAST_TIME;
    }
    else {
      (void)parsenewline(cachefilep, NULL, '\0');
      record_start = pos;
      parsenonnewline(cachefilep);
      if (sscanf(record_start, "D\t%lu\t%lu%c", &firsttime, &lasttime, &tempc)
	  != 3 || (tempc != '\r' && tempc != '\n') || firsttime > lasttime) {
	warn('F', TRUE, "Cache file %s is corrupt: ignoring it",
	     cachefilep->name); 
	return;
      }
    }
    if (lasttime > dman->last7from && lasttime != LAST_TIME)
      warn('M', TRUE, "Cache file %s doesn't contain last-seven-day "
	   "statistics; so these may be underestimated", cachefilep->name);
    cachefilep->data[LOGDATA_UNKNOWN] += data[0];
    cachefilep->data[LOGDATA_INFO] += data[1];
    cachefilep->data[LOGDATA_SUCC] += data[2];
    cachefilep->data[LOGDATA_PAGES] += data[3];
    cachefilep->data[LOGDATA_REDIR] += data[4];
    cachefilep->data[LOGDATA_FAIL] += data[5];
    cachefilep->bytes += bys;
    while (rc != EOF) {
      rc = TRUE;
      if (pos >= block_end) {
	if ((rc = getmoredata(cachefilep, block_start, BLOCKSIZE)) == OK)
	  rc = TRUE;
	pos = block_start;
	record_start = pos;
      }
      if (rc == TRUE) {
	(void)parsenewline(cachefilep, NULL, '\0');
	record_start = pos;
	parsenonnewline(cachefilep);
	if (pos - record_start >= 2048) {
	  *(record_start + 70) = '\0';
	  warn('F', TRUE, "Ignoring long line in cache file starting\n%s",
	       record_start);
	  rc = FALSE;
	}
	if (pos == record_start)
	  rc = EOF;
      }
      if (rc == TRUE) {
	*(pos++) = '\0';
	if (ISDIGIT(*record_start)) {
	  if (ISDIGIT(*(record_start + 1))) {
	    temps = strchr(record_start, '\t');
	    if (temps == NULL)
	      corruptcacheline(record_start, &rc, 0);
	    else {
	      datecode = (unsigned int)atoi(record_start);
	      if (parsecacheline(record_start, ++temps, &code,
				 rep2gran[REP_FIVEREP], 2, data, &bys, NULL)) {
		timecode = (timecode_t)datecode * 1440 + (timecode_t)code * 5;
		/* time rounded down to nearest 5 minutes, so... */
		if (timecode < firsttime) {
		  if (firsttime - timecode <= 4)
		    timecode = firsttime;
		  else
		    corruptcacheline(record_start, &rc, 3);
		}
		else if (timecode > lasttime)
		  corruptcacheline(record_start, &rc, 3);
		else if (lasttime - timecode <= 4)
		  timecode = lasttime;
		/* This doesn't cover the case where the first and last entries
		   in the cache file are in the same five-minute period, but
		   that's so unlikely that I can't be bothered to write special
		   code for it. Anyway, it's not the only remaining problem
		   with using only five-minute resolution in the cache file. */
		if (rc == TRUE && timecode >= dman->from &&
		    timecode <= dman->to) {
		  datehash(timecode, dman, data[0], data[1], bys, granularity);
		  cachefilep->from = MIN(cachefilep->from, timecode);
		  cachefilep->to = MAX(cachefilep->to, timecode);
		  if (timecode > dman->last7from &&
		      timecode <= dman->last7to) {
		    cachefilep->data[LOGDATA_SUCC7] += data[0];
		    cachefilep->data[LOGDATA_PAGES7] += data[1];
		    cachefilep->bytes7 += bys;
		  }
		}
	      }
	    }
	  }
	  else if (*(record_start + 1) == '\t') {
	    rc2 = parsecacheline(record_start, record_start, &code,
				 ITEM_NUMBER, data_number, data, &bys, &mmq);
	    if (rc2) {
	      /* Map fields which are recorded into full data structure. */
	      for (i = 0; i < DATA_NUMBER; i++)
		data2[i] = 0;
	      for (i = 0; i < V5_DATA_NUMBER; i++)
		data2[cache_records[i]] = data[i];
	      if (type <= '4') {
		/* In this case, firstd terms weren't recorded; use the
		   obvious bound on them. */
		data2[SUCCFIRSTD] = data2[SUCCDATE];
		data2[REDIRFIRSTD] = data2[REDIRDATE];
		data2[FAILFIRSTD] = data2[FAILDATE];
	      }
	      hashbuild((choice)code, data2, bys, &mmq, &(hash[code]),
			no_cols[code], datacols[code], data2cols[code],
			wanthead[code], ispagehead, tree, derv, alltrees,
			alldervs, lowmem[code], dirsuffix, dirsufflength,
			usercase_insensitive, convfloor, multibyte);
	    }
	  }
	  else
	    corruptcacheline(record_start, &rc, 0);
	}
	else if (*record_start == 'z' && *(record_start + 1) == '\t') {
	  if (parsecacheline(record_start, record_start + 2, &code, SIZEBINS,
			     3, data, &bys, NULL)) {
	    arrayscore(arraydata[REP_SIZE - FIRST_ARRAYREP],
		       arraydata[REP_SIZE - FIRST_ARRAYREP][code].threshold,
		       data[0], 0, data[1], 0, bys, 0., data[2]);
	  }
	}
	else if (*record_start == 'c' && *(record_start + 1) == '\t') {
	  if (parsecacheline(record_start, record_start + 2, &code, 600, 2,
			     data, NULL, NULL))
	    arrayscore(arraydata[REP_CODE - FIRST_ARRAYREP],
		       code, data[0], 0, 0, 0, 0., 0., data[1]);
	}
	else if (*record_start == 'P' && *(record_start + 1) == '\t') {
	  if (parsecacheline(record_start, record_start + 2, &code, 600002, 3,
			     data, &bys, NULL))
	    arrayscore(arraydata[REP_PROCTIME - FIRST_ARRAYREP], code,
		       data[0], 0, data[1], 0, bys, 0., data[2]);
	}
	else
	  corruptcacheline(record_start, &rc, 0);
      }
    }
  }
}
