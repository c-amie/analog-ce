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

/*** analog.c; the main function ***/

#include "anlghea3.h"

int main(int argc, char *argv[])
{
  extern char *pos, *record_start, *commandname;
  extern unsigned long progressfreq;
  extern Inputformatlist *logformat;
  extern logical vblesonly;
  extern FILE *errfile;
#ifndef NODNS
  extern char *dnsfile, *dnslockfile;
  extern FILE *dnsfilep, *dnslock;
#endif

  Logfile *logfilep;
  Options *op;
  Inputformatlist *fmt;
  unsigned long linesread = 0, nextreport;
  ptrdiff_t corrupt_pos;
  time_t progfreqtime;
  struct tm *pflt;
  choice rc;

  op = (Options *)xmalloc(sizeof(Options));
  initialise(argc, argv, op);
  if (vblesonly) {
    report_vbles(op);
    my_exit(EXIT_SUCCESS);
  }
  nextreport = progressfreq;

  for (logfilep = op->miscopts.logfile[1]; logfilep != NULL;
       TO_NEXT(logfilep)) {
    if (my_lfopen(logfilep, "cache file")) {
      readcache(logfilep, op->hash, op->no_cols, op->datacols,
		op->data2cols, op->arraydata, op->wanthead, op->ispagehead,
		&(op->dman), op->outopts.tree, op->outopts.derv,
		op->outopts.alltrees, op->outopts.alldervs,
		op->miscopts.lowmem, op->miscopts.dirsuffix,
		op->miscopts.dirsufflength, op->miscopts.usercase_insensitive,
		op->outopts.convfloor, op->outopts.multibyte,
		op->miscopts.granularity);
    }
    my_lfclose(logfilep, op->miscopts.logfile[1], NULL, "cache file",
	       &(op->dman), op->miscopts.data, &(op->miscopts.bytes),
	       &(op->miscopts.bytes7));
  }

  for (logfilep = op->miscopts.logfile[0]; logfilep != NULL;
       TO_NEXT(logfilep)) {
    if (logfile_init(logfilep)) {
      rc = FALSE;
      while (rc != EOF) {
	corrupt_pos = -1;
	record_start = pos;
	for (fmt = logfilep->format, rc = FALSE; rc == FALSE && fmt != NULL;
	     TO_NEXT(fmt)) {
	  rc = parsenextrecord(logfilep, fmt->form);
	  if (rc == TRUE) {
	    process_data(logfilep, op->hash, op->arraydata, fmt->count,
			 op->code2type, op->datacols, op->data2cols,
			 op->no_cols, op->wanthead, op->ispagehead,
			 op->aliashead, op->argshead, op->refargshead,
			 &(op->dman), op->outopts.tree, op->outopts.derv,
			 op->outopts.alltrees, op->outopts.alldervs,
			 op->miscopts.lowmem, op->miscopts.case_insensitive,
			 op->miscopts.usercase_insensitive,
			 op->outopts.convfloor, op->outopts.multibyte,
			 op->miscopts.dirsuffix, op->miscopts.dirsufflength,
			 op->miscopts.granularity);
	  }
	  else if (rc == FALSE)
	    corrupt_pos = MAX(pos - record_start, corrupt_pos);
	  else if (rc == NEWLOGFMT) {
	    logfilep->format = logformat;
	    if (correctlogfmt(logfilep, op->wanthead, op->code2type,
			      &(op->dman)) == NULL) {
	      debug('F', "Ignoring logfile %s, which contains no items "
		    "being analysed", logfilep->name);
	      rc = EOF;
	    }
	  }
	  else if (rc == BADLOGFMT) {
	    warn('F', TRUE,
		 "Can't auto-detect format of logfile %s: ignoring it",
		 logfilep->name);
	    rc = EOF;
	  }
	}   /* end for(fmt) */
	if (rc == FALSE)
	  corrupt_line(logfilep, "", corrupt_pos);
	if ((++linesread) == nextreport) {
	  time(&progfreqtime);
	  pflt = localtime(&progfreqtime);
	  fprintf(errfile, "%s: [%02d:%02d:%02d] %lu lines read\n",
		  commandname, pflt->tm_hour, pflt->tm_min, pflt->tm_sec,
		  linesread);
	  if (errfile != stderr)
	    fprintf(stderr, "%s: [%02d:%02d:%02d] %lu lines read\n",
		    commandname, pflt->tm_hour, pflt->tm_min, pflt->tm_sec,
		    linesread);
	  nextreport += progressfreq;
	}
#ifdef MAC_EVENTS
	if ((linesread % MAC_IDLE_FREQ) == 0)
	  MacIdle();
#endif
      }   /* end while (rc != EOF) */
    }   /* if (logfile_init(logfilep)) */
    my_lfclose(logfilep, op->miscopts.logfile[1], op->miscopts.logfile[0],
	       "logfile", &(op->dman), op->miscopts.data,
	       &(op->miscopts.bytes), &(op->miscopts.bytes7));
  }   /* for (logfilep) */

#ifndef NODNS
  if (dnsfilep != NULL) {
    fclose(dnsfilep);
    debug('F', "Closing DNS output file %s", dnsfile);
  }
  if (dnslock != NULL) {
    fclose(dnslock);
    dnslock = NULL;
    if (remove(dnslockfile) != 0)
      warn('F', TRUE, "Trouble deleting DNS lock file %s", dnslockfile);
    else
      debug('F', "Deleting DNS lock file %s", dnslockfile);
  }
#endif
  unhashall(op->hash, &(op->gooditems), &(op->baditems));
  if (!STREQ(op->outopts.cacheoutfile, "none"))
    outputcache(op->outopts.cacheoutfile, op->gooditems, &(op->dman),
		op->arraydata, op->miscopts.data, op->miscopts.bytes,
		op->data2cols);
  if (op->outopts.outstyle != OUT_NONE)
    output(&(op->outopts), op->gooditems, op->baditems, &(op->dman),
	   op->arraydata, op->miscopts.data, op->miscopts.bytes,
	   op->miscopts.bytes7, op->datacols, op->data2cols, op->no_cols,
	   op->miscopts.granularity);
  my_exit(EXIT_SUCCESS);
  return(EXIT_SUCCESS);  /* just to stop compiler complaining */
}
