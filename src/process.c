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

/*** process.c; process some data ***/

#include "anlghea3.h"

void process_data(Logfile *logfilep, Hashtable **hash,
		  Arraydata **arraydata, choice *count, choice *code2type,
		  choice datacols[ITEM_NUMBER][OUTCOME_NUMBER][DATACOLS_NUMBER][2],
		  choice data2cols[ITEM_NUMBER][DATA_NUMBER],
		  unsigned int *no_cols, Include **wanthead,
		  Include *ispagehead, Alias **aliashead, Include *argshead,
		  Include *refargshead, Dateman *dman, Tree **tree,
		  Derv **derv, choice *alltrees, choice *alldervs,
		  choice *lowmem, logical case_insensitive,
		  logical usercase_insensitive, unsigned char convfloor,
		  logical multibyte, char *dirsuffix,
		  unsigned int dirsufflength, unsigned int granularity) {
  extern unsigned int year, month, date, hour, minute, code;
  extern unsigned long unixtime, proctime;
  extern char am;
  extern double bytes;
  extern Memman mm[], mmq, mms, *amemman;
  extern choice *rep2type;
  extern Hashentry *unwanted_entry, *blank_entry;
  extern Hashindex *dummy_item;

  static Hashindex *gp[ITEM_NUMBER];
  unsigned long data[DATA2_NUMBER];
  Hashentry *item[ITEM_NUMBER];
  logical wanttree[ITEM_NUMBER];
  logical isitpage, last7;
  choice ispage = UNSET;
  choice wanted = TRUE, rc, outcome;
  timecode_t timecode = FIRST_TIME;
  char *name, *namestart, *nameend;
  size_t len;
  choice i, j, k;

  /*** check whether this line is wanted ***/

  if (count[INP_CODE] != 0) {
    if (code == IGNORE_CODE) {
      for (j = 0; j < ITEM_NUMBER; j++) {   /* reset strings */
	if (count[j] != 0)
	  mm[j].next_pos = mm[j].curr_pos;
      }
      mmq.next_pos = mmq.curr_pos;
      mms.next_pos = mms.curr_pos;
      logfilep->data[LOGDATA_UNKNOWN]++;
      return;
    }
    else if (code2type[code] == UNWANTED)
      wanted = FALSE;
  }
  if (wanted && count[INP_DATE] > 0) {
    if (count[INP_UNIXTIME])
      wanted = wantunixtime(&timecode, dman, unixtime, logfilep->tz);
    else {
      if (count[INP_AM]) {
	if (hour > 12) {
	  corrupt_line(logfilep, "Hour greater than 12", -1);
	  return;
	}
	else if (hour == 12)
	  hour = 0;
	if (am == 'p')
	  hour += 12;
      }
      wanted = wantdate(&timecode, dman, hour, minute, date, month, year,
			logfilep->tz);
    }
    if (wanted == ERR) { /* corrupt date */
      corrupt_line(logfilep, "Corrupt date or time", -1);
      return;
    }
  }  /* end count[INP_DATE] > 0 */
  for (i = 0; i < ITEM_NUMBER; i++) {
    wanttree[i] = FALSE;
    if (!wanted) {
      for (j = i; j < ITEM_NUMBER; j++) {  /* reset not-yet-hashed strings */
	if (count[j] != 0)                 /* NB i is now (unwanted i) + 1 */
	  mm[j].next_pos = mm[j].curr_pos;
      }
      mmq.next_pos = mmq.curr_pos;
      mms.next_pos = mms.curr_pos;
      logfilep->data[LOGDATA_UNWANTED]++;
      return;
    }
    if (i == ITEM_HOST)
      prealiasS(&(mm[ITEM_HOST]), &mms);
    name = (char *)(mm[i].curr_pos);
    if (count[i] == 0 || IS_EMPTY_STRING(name) ||
	(name[0] == '-' && name[1] == '\0')) {
      item[i] = blank_entry; /* or unwanted_; but we get wanted right anyway */
      wanted = (wanthead[i] == NULL || included("", FALSE, wanthead[i]));
      /* wanthead[i] == NULL is tested again in included() but it often saves
	 a call to that function, because blankness is common. */
    }
    else {
      if (i == ITEM_FILE || i == ITEM_REFERRER) {
	if ((j = prealias(&(mm[i]), &(mm[ITEM_VHOST]), item[ITEM_VHOST], &mmq,
			  (logical)((i == ITEM_FILE)?case_insensitive:FALSE),
			  (i == ITEM_FILE)?(logfilep->prefix):NULL,
			  logfilep->prefixlen, logfilep->pvpos,
			  (i == ITEM_FILE)?argshead:refargshead)) < 0) {
	  if (j == -1)
	    corrupt_line(logfilep,
			 "%v in file prefix but no VHOST in line", -1);
	  else
	    corrupt_line(logfilep, "Filename too long", -1);
	  return;
	}
      }
      if (lowmem[i] == 0) {
	if (gp[i] == NULL || !STREQ(name, gp[i]->name)) {
	  gp[i] = hashfind(&mm[i], &(hash[i]), no_cols[i], wanthead[i], UNSET,
			   ispagehead, aliashead[i], dirsuffix, dirsufflength,
			   usercase_insensitive, 0, FALSE, i, FALSE);
	}     /* if name the same as last time, don't need */
	else  /* to hashfind again, or save the name */
	  mm[i].next_pos = mm[i].curr_pos;
	item[i] = (Hashentry *)(gp[i]->other);	  
	wanted = (choice)(ENTRY_WANTED(item[i]));
      }

      else if (lowmem[i] == 1) {
	if ((rc = do_alias(name, amemman, aliashead[i], dirsuffix,
			   dirsufflength, usercase_insensitive, 0, FALSE, i))
	    == FALSE) {
	  item[i] = hashfind(&mm[i], &(hash[i]), no_cols[i], wanthead[i],
			     UNSET, ispagehead, NULL, dirsuffix, dirsufflength,
			     usercase_insensitive, 0, FALSE, i, TRUE)->own;
	}
	else if (rc == TRUE) {
	  mm[i].next_pos = mm[i].curr_pos;  /* don't save string */
	  item[i] = hashfind(amemman, &(hash[i]), no_cols[i], wanthead[i],
			     UNSET, ispagehead, NULL, dirsuffix, dirsufflength,
			     usercase_insensitive, 0, FALSE, i, TRUE)->own;
	}
	else { /* rc == ERR */
	  mm[i].next_pos = mm[i].curr_pos;
	  if (included("", FALSE, wanthead[i]))
	    item[i] = blank_entry;
	  else
	    item[i] = unwanted_entry;
	}
	wanted = (choice)(ENTRY_WANTED(item[i]));
      }

      else { /* lowmem[i] >= 2 */
	if ((rc = do_alias(name, amemman, aliashead[i], dirsuffix,
			   dirsufflength, usercase_insensitive, 0, FALSE, i))
	    == TRUE) {
	  mm[i].next_pos = mm[i].curr_pos;  /* don't save old string */
	  len = strlen((char *)(amemman->curr_pos));
	  memcpy(submalloc(&(mm[i]), len + 1), amemman->curr_pos, len + 1);
	  name = (char *)(mm[i].curr_pos); /* which might have changed */
	  amemman->next_pos = amemman->curr_pos;
	}
	if (rc == ERR) {
	  if (included("", FALSE, wanthead[i])) {
	    item[i] = blank_entry;
	    if (i == ITEM_FILE)
	      ispage = FALSE;
	  }
	  else
	    wanted = FALSE;
	  mm[i].next_pos = mm[i].curr_pos;
	}
	else {
	  isitpage = pageq(name, ispagehead, i);
	  if (i == ITEM_FILE)
	    ispage = (choice)isitpage;
	  if (included(name, isitpage, wanthead[i])) {
	    if (lowmem[i] == 2) {
	      item[i] = hashfind(&(mm[i]), &(hash[i]), no_cols[i], wanthead[i],
				 isitpage, ispagehead, NULL, dirsuffix,
				 dirsufflength, usercase_insensitive, 0, FALSE,
				 i, TRUE)->own;
	    }
	    else {
	      item[i] = blank_entry;
	      wanttree[i] = TRUE;
	      mm[i].next_pos = mm[i].curr_pos;
	    }
	  }
	  else {
	    wanted = FALSE;
	    mm[i].next_pos = mm[i].curr_pos;
	  }
	}
      }  /* end lowmem[i] >= 2 */
    }
  }      /* end for i */
  if (!wanted) {
    logfilep->data[LOGDATA_UNWANTED]++;
    return;
  }

  /*** now add it to the hash tables ***/

  /* add to logfile from and to if wanted, whatever status code */
  if (timecode != FIRST_TIME)
    logfilep->from = MIN(logfilep->from, timecode);
  logfilep->to = MAX(logfilep->to, timecode);
  last7 = (timecode > dman->last7from && timecode <= dman->last7to);
  if (ispage == UNSET)            /* NB blank_entry has ispage FALSE */
    ispage = (choice)(item[ITEM_FILE]->ispage);
  if (count[INP_BYTES] == 0)
    bytes = 0;
  if (count[INP_CODE] == 0) {
    outcome = SUCCESS;
    if (count[ITEM_FILE] == 2) {
      logfilep->data[LOGDATA_SUCC]++;
      logfilep->data[LOGDATA_SUCC7] += (unsigned long)last7;
      logfilep->data[LOGDATA_PAGES] += (unsigned long)ispage;
      logfilep->data[LOGDATA_PAGES7] +=
	(unsigned long)((logical)ispage && last7);
    }
    else {
      logfilep->data[LOGDATA_UNKNOWN]++;
      logfilep->data[LOGDATA_UNKNOWN7] += (unsigned long)last7;
    }
  }
  else if (code <= 199) {
    outcome = INFO;
    logfilep->data[LOGDATA_INFO]++;
    logfilep->data[LOGDATA_INFO7] += (unsigned long)last7;
  }
  else switch (outcome = code2type[code]) {
  case SUCCESS:
    logfilep->data[LOGDATA_SUCC]++;
    logfilep->data[LOGDATA_SUCC7] += (unsigned long)last7;
    logfilep->data[LOGDATA_PAGES] += (unsigned long)ispage;
    logfilep->data[LOGDATA_PAGES7] +=
      (unsigned long)((logical)ispage && last7);
    break;
  case FAILURE:
    logfilep->data[LOGDATA_FAIL]++;
    logfilep->data[LOGDATA_FAIL7] += (unsigned long)last7;
    break;
  case REDIRECT:
    logfilep->data[LOGDATA_REDIR]++;
    logfilep->data[LOGDATA_REDIR7] += (unsigned long)last7;
    break;
  case INFO:
    logfilep->data[LOGDATA_INFO]++;
    logfilep->data[LOGDATA_INFO7] += (unsigned long)last7;
    break;
  }

  /* NB any change in what to count when will require corresponding change to
     end of strtoinfmt() and to fmt munching in correct() */
  if (count[INP_CODE] == 2)
    arrayscore(arraydata[REP_CODE - FIRST_ARRAYREP], code, 1,
	       (unsigned long)last7, 0, 0, 0., 0., timecode);
  if (outcome != INFO) {
    if (outcome == SUCCESS) {
      if (count[INP_DATE] == 2)  /* only if file present: see strtoinfmt() */
	datehash(timecode, dman, 1, (unsigned long)ispage, bytes, granularity);
      if (count[INP_BYTES] == 2) {
	arrayscore(arraydata[REP_SIZE - FIRST_ARRAYREP], bytes, 1,
		   (unsigned long)last7, (unsigned long)ispage,
		   (unsigned long)((logical)ispage && last7), bytes,
		   last7?bytes:0., timecode);
	logfilep->bytes += bytes;
	if (last7)
	  logfilep->bytes7 += bytes;
      }
      if (count[INP_PROCTIME] == 2)
	arrayscore(arraydata[REP_PROCTIME - FIRST_ARRAYREP], proctime, 1,
		   (unsigned long)last7, (unsigned long)ispage,
		   (unsigned long)((logical)ispage && last7), bytes,
		   last7?bytes:0., timecode);

      if (alltrees[0] != REP_NUMBER || alldervs[0] != REP_NUMBER) {
	/* for LOWMEM 3, run through alltrees then alldervs */
	/* NB these (POSSTREE/POSSDERV in init.c) only count successes */
	for (k = 0; k <= 1; k++) {
	  for (i = 0; (k?(alldervs[i]):(alltrees[i])) != REP_NUMBER; i++) {
	    j = rep2type[k?(alldervs[i]):(alltrees[i])];
	    if (wanttree[j]) {
	      dummy_item->name = mm[j].curr_pos;
	      /* mm.curr_pos is marked for deletion, but still intact at
		 present */
	      dummy_item->own->data[data2cols[j][REQUESTS]] = 1;
	      if (data2cols[j][REQUESTS7] >= 0)  /* see comment in genrep() */
		dummy_item->own->data[data2cols[j][REQUESTS7]] =
		  (unsigned long)last7;
	      if (data2cols[j][PAGES] >= 0)
		dummy_item->own->data[data2cols[j][PAGES]] =
		  (unsigned long)ispage;
	      if (data2cols[j][PAGES7] >= 0)
		dummy_item->own->data[data2cols[j][PAGES7]] =
		  (unsigned long)((logical)ispage && last7);
	      if (data2cols[j][SUCCDATE] >= 0)
		dummy_item->own->data[data2cols[j][SUCCDATE]] = timecode;
	      if (data2cols[j][SUCCFIRSTD] >= 0)
		dummy_item->own->data[data2cols[j][SUCCFIRSTD]] = timecode;
	      dummy_item->own->bytes = bytes;
	      dummy_item->own->bytes7 = last7?bytes:0.;
	      if (k)
		makederived(derv[alldervs[i] - FIRST_DERVREP], dummy_item,
			    NULL, convfloor, multibyte, alldervs[i],
			    datacols[j], no_cols[j]);
	      else {
		namestart = NULL;
		tree[G(alltrees[i])]->cutfn(&namestart, &nameend,
					    dummy_item->name, FALSE);
		(void)treefind(namestart, nameend,
			       &(tree[G(alltrees[i])]->tree), dummy_item,
			       tree[G(alltrees[i])]->cutfn, FALSE, TRUE, FALSE,
			       tree[G(alltrees[i])]->space, datacols[j],
			       no_cols[j]);
	      }
	    }
	  }
	}
      }  /* there are trees or dervs */
    }    /* outcome == SUCCESS */
    data[REQUESTS2] = 1;
    data[REQUESTS72] = (unsigned long)last7;
    data[PAGES2] = (unsigned long)ispage;
    data[PAGES72] = (unsigned long)((logical)ispage && last7);
    data[DATE2] = timecode;
    data[FIRSTD2] = timecode;
    for (i = 0; i < ITEM_NUMBER; i++) {
      if (count[i] == 2 && !ENTRY_BLANK(item[i]))
	hashscore(item[i], data, datacols[i][outcome], outcome, bytes);
    }
  }      /* end if outcome != INFO */
}

void corrupt_line(Logfile *logfilep, char *message, ptrdiff_t n) {
  extern FILE *errfile;
  extern char *debug_args, *record_start, *pos;

  pos = record_start;
  parsenonnewline(logfilep);  /* skip to after next newline */
  *pos = '\0';
  if (pos - record_start > 500)  /* debug might only handle 509 chars */
    *(record_start + 500) = '\0';
  debug('C', "%s", record_start);
  if (n < 0)
    debug('C', "  (%s)", message);
  else if (strchr(debug_args, 'C')) {
    fprintf(errfile, "C: ");
    for ( ; n >= 10; n -= 10)
      fprintf(errfile, "          ");
    for ( ; n > 0; n--)
      fputc(' ', errfile);
    fprintf(errfile, "*\n");
  }
  logfilep->data[LOGDATA_CORRUPT]++;
  *pos = '\n';
  (void)parsenewline(logfilep, NULL, '\0');
}

void arrayscore(Arraydata *array, double amount, unsigned long reqs,
		unsigned long reqs7, unsigned long pages, unsigned long pages7,
		double bytes, double bytes7, timecode_t timecode) {
  choice i;
  
  for (i = 0; ; i++) {  /* last threshold must be -1 to ensure termination */
    if (amount <= array[i].threshold || array[i].threshold < -0.5) {
      array[i].reqs += reqs;
      array[i].reqs7 += reqs7;
      array[i].pages += pages;
      array[i].pages7 += pages7;
      array[i].bytes += bytes;
      array[i].bytes7 += bytes7;
      array[i].lastdate = MAX(array[i].lastdate, timecode);
      array[i].firstdate = MIN(array[i].firstdate, timecode);
      return;
    }
  }
}
