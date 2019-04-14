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

/*** anlghea4.h: fourth header file ***/

/*** Just function declarations: these are not user-definable; user-definable
 *** options can be found in anlghead.h ***/

#ifndef ANLGHEA4_H
#define ANLGHEA4_H

/* in alias.c */
extern choice do_alias(char *name, Memman *mp, Alias *aliashead,
  char *dirsuffix, unsigned int dirsufflength, logical usercase_insensitive,
  unsigned char convfloor, logical multibyte, choice type);
extern logical do_aliasx(char *name, Alias *aliashead);
extern logical do_aliasr(char *name, char *dirsuffix,
  unsigned int dirsufflength);
extern logical do_aliasS(char *name);
extern logical do_aliasf(char *name);
extern logical do_aliasB(char *name);
extern logical do_aliasu(char *name, logical case_insensitive);
extern logical do_aliasv(char *name);
extern logical do_aliasN(char *name, unsigned char convfloor,
  logical multibyte);
extern logical do_aliasn(char *name, unsigned char convfloor,
  logical multibyte);
extern logical do_aliasb(char *name);
#ifndef NODNS
extern logical dnsresolve(char *name, choice level);
extern void do_dns(char *name, char *alias, choice level);
#endif
extern void printalias(char *name, AliasTo *to, int pmatch[]);
extern void wildalias(char *name, AliasTo *to, int pmatch[]);
extern choice prealias(Memman *mm, Memman *mmv, Hashentry *vhost, Memman *mmq,
  logical case_insensitive, char *prefix, size_t prefixlen, int pvpos,
  Include *argshead);
extern void prealiasS(Memman *mm, Memman *mms);
extern void reverseonename(Hashindex *p);
extern void reversenames(Hashindex *ans);

/* in analog.c -- only main() */

/* in cache.c */
extern void outputcache(char *outfile, Hashindex **gooditems, Dateman *dman,
  Arraydata **arraydata, unsigned long *data, double bys,
  choice data2cols[ITEM_NUMBER][DATA_NUMBER]);
extern void hashbuild(choice code, unsigned long *data, double bys, Memman *mm,
  Hashtable **hash, unsigned int data_number,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2], choice data2cols[],
  Include *wanthead, Include *ispagehead, Tree **tree,
  Derv **derv, choice *alltrees, choice *alldervs, choice lowmem,
  char *dirsuffix, unsigned int dirsufflength, logical usercase_insensitive,
  unsigned char convfloor, logical multibyte);
extern void corruptcacheline(char *line, choice *rc, choice type);
extern logical parsecacheline(char *linestart, char *parsestart,
  unsigned int *code, unsigned int maxcode, int no_items, unsigned long *data,
  double *bys, Memman *mm);
extern void readoldcache(Logfile *cachefilep, Dateman *dman, choice type,
  unsigned int granularity);
extern void readcache(Logfile *cachefilep, Hashtable **hash,
  unsigned int *no_cols,
  choice datacols[ITEM_NUMBER][OUTCOME_NUMBER][DATACOLS_NUMBER][2],
  choice data2cols[ITEM_NUMBER][DATA_NUMBER],
  Arraydata **arraydata, Include **wanthead, Include *ispagehead,
  Dateman *dman, Tree **tree, Derv **derv, choice *alltrees,
  choice *alldervs, choice *lowmem, char *dirsuffix,
  unsigned int dirsufflength, logical usercase_insensitive,
  unsigned char convfloor, logical multibyte, unsigned int granularity);

/* in dates.c */
extern void code2date(datecode_t code, unsigned int *date, unsigned int *month,
  unsigned int *year);
extern time_t shifttime(time_t timer, int diff);
extern choice parsedate(time_t starttime, char *s, timecode_t *date,
  logical from, logical unixtime);
extern choice wantdate(timecode_t *timecode, Dateman *dman, unsigned int hr,
  unsigned int min, unsigned int date, unsigned int month, unsigned int year,
  int tz);
extern choice wantunixtime(timecode_t *timecode, Dateman *dman,
  unsigned long unixtime, int tz);
extern Daysdata *newday(unsigned int granularity);
extern void dmaninit(Dateman *dman, datecode_t datecode,
  unsigned int granularity);
extern void datehash(timecode_t timecode, Dateman *dman, unsigned long reqs,
  unsigned long pages, double bytes, unsigned int granularity);

/* in globals.c */
extern void globals(char *comnane);
extern void defaults(Options *op);

/* in hash.c */
extern Hashtable *rehash(Hashtable *old, unsigned long size,
  Memman *treespace);
extern Hashindex *hashfind(Memman *mp, Hashtable **table,
  unsigned int data_number, Include *wanthead,
  choice ispage, Include *ispagehead, Alias *aliashead, char *dirsuffix,
  unsigned int dirsufflength, logical usercase_insensitive,
  unsigned char convfloor, logical multibyte, choice type, logical aliased);
extern Hashentry *newhashentry(unsigned int data_number, logical ispage);
extern void hashscore(Hashentry *ep, unsigned long data[DATA2_NUMBER],
  choice datacols[DATACOLS_NUMBER][2], choice outcome, double bytes);
extern void unhash(Hashtable *hash, Hashindex **gooditems,
  Hashindex **baditems);
extern void unhashall(Hashtable **hash, Hashindex ***gooditems,
  Hashindex ***baditems);

/* in init.c */
extern void initialise(int argc, char *argv[], Options *op);
extern void confline(Options *op, char *cmd, char *arg1, char *arg2, int rc);
extern choice config(char *filename, Options *op, logical fromcommandline);
extern void settings(Options *op, int argc, char *argv[]);
extern Inputformatlist *correctlogfmt(Logfile *lp, Include **wanthead,
  choice *code2type, Dateman *dman);
extern void correct(Options *op);
extern void finalinit(Options *op);
extern choice strtoinfmt(Inputformat **ans, char *s, choice *count);
extern char *apachelogfmt(char *fmt);
extern void clconfline(Options *op, char *s);
extern void clgenrep(Options *op, choice rep, char *arg);
extern void cldebug(char **s, char *arg);
extern void clargs(Options *op, int argc, char *argv[]);

/* in init2.c */
extern void shortwarn(char *cmd, char *arg1, int rc);
extern void longwarn(char *cmd, char *arg1, char *arg2, int rc);
extern void badwarn(char *cmd, choice domess, char *arg1, char *arg2, int rc);
extern void unknownwarn(char *cmd, char *arg1, char *arg2);
extern void configcall(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configcols(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configallcols(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configchoice(void *opt, char *cmd, char *arg1, char *arg2, int rc);
#ifndef NOGRAPHICS
extern void configallchart(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
#endif
extern void configdebug(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configall(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configallback(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configlang(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void selectlang(char *country, Outchoices *op);
extern void configlogfmt(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configapachelogfmt(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configrepord(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configstr(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configstrlist(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configerrfile(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
#ifndef NODIRENT
extern Logfile *clonelogs(Logfile *from, char *name);
extern Logfile *expandwildlogs(Logfile *lp, Logfile **pter);
#endif
extern void configlogfile(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configcachefile(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configoutfile(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configchar(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configbarstyle(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configgraph(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configallgraph(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configfloor(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configtree(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configulong(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configuint(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configoffset(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configlowmem(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern int aliastocount(char *s);
extern AliasTo *configaliasto(char *s, logical is_regex);
extern void configalias(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configstrpair(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configstrpairlist(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configstrpair2list(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configdomlevel(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configinex(void *opt, char *cmd, char *arg1, char *arg2, int rc,
  logical in, logical omitinitdot, logical omittrailslash);
extern void configinc(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configexc(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configincd(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configexcd(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configincs(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configexcs(void *opt, char *cmd, char *arg1, char *arg2, int rc);
logical parseiprange(char *arg1, unsigned long *minaddr,
  unsigned long *maxaddr);
extern void confighostinex(void *opt, char *cmd, char *arg1, char *arg2,
  int rc, logical in);
extern void confighostinc(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void confighostexc(void *opt, char *cmd, char *arg1, char *arg2,
  int rc);
extern void configscinex(void *opt, char *cmd, char *arg1, char *arg2, int rc,
  logical in);
extern void configscinc(void *opt, char *cmd, char *arg1, char *arg2, int rc);
extern void configscexc(void *opt, char *cmd, char *arg1, char *arg2, int rc);

/* in input.c */
extern logical my_lfopen(Logfile *lf, char *logtype);
extern FILE *my_fopen(char *name, char *logtype);
extern void my_lfclose(Logfile *p, Logfile *allc, Logfile *allp,
  char *filetype, Dateman *dman, unsigned long *totdata, double *bys,
  double *bys7);
extern int my_fclose(FILE *f, char *name, char *filetype);
extern logical logfile_init(Logfile *lf);
extern void duplogfile(Logfile *lf);
extern choice getmoredata(Logfile *lf, char *start, size_t length);
extern choice getnextline(FILE *f, char *start, size_t used);
extern void shunt_data(Logfile *lf);
extern int strtomonth(char *month);
extern choice parsemonth(Logfile *lf, void *arg, char c);
extern choice parseuint2(Logfile *lf, void *arg, char c);
extern choice parsenmonth(Logfile *lf, void *arg, char c);
extern choice parseulong(Logfile *lf, void *arg, char c);
extern choice parseunixtime(Logfile *lf, void *arg, char c);
extern choice parseproctime(Logfile *lf, void *arg, char c);
extern choice parseproctimeu(Logfile *lf, void *arg, char c);
extern choice parseuint2strict(Logfile *lf, void *arg, char c);
extern choice parsescode(Logfile *lf, void *arg, char c);
extern choice parseuint4strict(Logfile *lf, void *arg, char c);
extern choice parseudint(Logfile *lf, void *arg, char c);
extern choice parseyear2(Logfile *lf, void *arg, char c);
extern choice parseyear2or4(Logfile *lf, void *arg, char c);
extern choice parseam(Logfile *lf, void *arg, char c);
extern choice parsecode(Logfile *lf, void *arg, char c);
extern choice parsejunk(Logfile *lf, void *arg, char c);
extern choice parsespace(Logfile *lf, void *arg, char c);
extern choice parseoptspace(Logfile *lf, void *arg, char c);
extern choice parsenewline(Logfile *lf, void *arg, char c);
extern void parsenonnewline(Logfile *lf);
extern choice parsestring(Logfile *lf, void *arg, char c);
extern choice parsemsbrow(Logfile *lf, void *arg, char c);
extern choice parseref(Logfile *lf, void *arg, char c);
extern choice checkchar(Logfile *lf, void *arg, char c);
extern void resolveconflicts(unsigned int n, char *delims, ...);
extern choice parselogfmt(Logfile *lf, void *arg, char c);
extern choice parsenextrecord(Logfile *lf, Inputformat *format);
extern int parseconfline(char *s, char **cmd, char **arg1, char **arg2);
extern int nextconfline(FILE *f, char **cmd, char **arg1, char **arg2);
extern char *nextlngstr(FILE *f, char *name, logical want);
extern choice nextdnsline(FILE *f, timecode_t *timec, char **name,
  char **alias);
extern void process_domainsfile(FILE *f, Options *op);
extern void process_descfile(FILE *f, char *name, Options *op);

/* in macstuff.c (See also macdir.h) */
#ifdef MAC_EVENTS
extern void MacInit(int *argc, char **argv[]);
extern void MacFini(void);
extern void MacIdle(void);
#endif
#ifdef MACDIRENT
extern void CToPCpy(unsigned char *pstr, const char *cstr);
#endif
#ifdef MAC
#ifndef NODNS
extern long OpenNetwork(void);
extern int IpAddr2Name(char *hostname);
extern void ResolverCleanup(void);
extern pascal void SvcNotifyProc(void *dataPtr, OTEventCode code,
  OTResult result, void *cookie);
extern pascal void MacTCPDNRResultProc(struct hostInfo *hInfoPtr,
  char *userDataPtr);
#endif
#endif

/* in output.c */
extern void pagetop(FILE *outf, Outchoices *od, Dateman *dman);
extern void pagebot(FILE *outf, Outchoices *od);
extern void gensum(FILE *outf, Outchoices *od, unsigned long *data, double bys,
  double bys7, Hashindex **gooditems, Hashindex **baditems, Dateman *dman,
  choice data2cols[ITEM_NUMBER][DATA_NUMBER]);
extern void timerep(FILE *outf, Outchoices *od, choice rep, Dateman *dman,
  unsigned int granularity);
extern void timesum(FILE *outf, Outchoices *od, choice rep, Dateman *dman,
  unsigned int granularity);
extern void printtree(FILE *outf, Outchoices *od, choice rep, Hashtable *tree,
  choice requests, choice requests7, choice pages, choice pages7, choice date,
  choice firstd, unsigned int level, Strlist *partname, unsigned long totr,
  unsigned long totr7, unsigned long totp, unsigned long totp7, double totb,
  double totb7, unsigned int width[], logical possrightalign,
  unsigned int bmult, unsigned int bmult7, double unit);
extern void genrep(FILE *outf, Outchoices *od, choice rep,
  Hashindex **gooditems, Hashindex **baditems,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2], choice *data2cols,
  unsigned int data_number, Dateman *dman);
extern void dervrep(FILE *outf, Outchoices *od, choice rep,
  Hashindex *gooditems, Hashindex *baditems,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2], choice *data2cols,
  unsigned int data_number, Dateman *dman);
extern void arrayrep(FILE *outf, Outchoices *od, choice rep, Arraydata *array,
  Dateman *dman);
extern void output(Outchoices *od, Hashindex **gooditems, Hashindex **baditems,
  Dateman *dman, Arraydata **arraydata, unsigned long *sumdata,
  double totbytes, double totbytes7,
  choice datacols[ITEM_NUMBER][OUTCOME_NUMBER][DATACOLS_NUMBER][2],
  choice data2cols[ITEM_NUMBER][DATA_NUMBER], unsigned int *no_cols,
  unsigned int granularity);

/* in output2.c */
extern void report_title(FILE *outf, Outchoices *od, choice rep);
extern void report_footer(FILE *outf, Outchoices *od, choice rep);
extern void reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd,
  timecode_t mind, Dateman *dman); 
extern void matchlength(FILE *outf, Outchoices *od, char *s, char c);
extern void matchlengthn(FILE *outf, Outchoices *od, int width, char c);
extern size_t datefmtlen(Outchoices *od, char *fmt);
extern char *datesprintf(Outchoices *od, char *fmt, datecode_t date,
  unsigned int hr, unsigned int min, datecode_t newdate, unsigned int newhr,
  unsigned int newmin, logical running, choice allowmonth); 
extern char *timesprintf(Outchoices *od, char *fmt, timecode_t t,
  choice allowmonth);
extern int f3printf(FILE *outf, Outchoices *od, double x, unsigned int width,
  char sepchar);
extern void printbytes(FILE *outf, Outchoices *od, double bytes,
  unsigned int bmult, unsigned int width, char sepchar);
extern void doublemprintf(FILE *outf, choice outstyle, unsigned int pagewidth,
  double x, char decpt);
extern double findunit(Outchoices *od, double n, unsigned int width[],
  choice *cols);
extern void calcwidths(Outchoices *od, choice rep, unsigned int width[],
  unsigned int *bmult, unsigned int *bmult7, double *unit, unsigned long maxr,
  unsigned long maxr7, unsigned long maxp, unsigned long maxp7, double maxb,
  double maxb7, unsigned long howmany);
extern unsigned int alphatreewidth(Outchoices *od, choice rep, Hashtable *tree,
  unsigned int level, Strlist *partname);
extern void declareunit(FILE *outf, Outchoices *od, char graphby, double unit,
  unsigned int bmult);
extern void whatincluded(FILE *outf, Outchoices *od, choice rep,
  unsigned long n, Dateman *dman);
extern void barchart(FILE *outf, Outchoices *od, char graphby,
  unsigned long reqs, unsigned long pages, double bys, double unit);
extern void colheads(FILE *outf, Outchoices *od, choice rep,
  unsigned int width[], unsigned int bmult, unsigned int bmult7,
  logical name1st);
extern void printcols(FILE *outf, Outchoices *od, choice rep,
  unsigned long reqs, unsigned long reqs7, unsigned long pages,
  unsigned long pages7, double bys, double bys7, long index, int level,
  unsigned long totr, unsigned long totr7, unsigned long totp,
  unsigned long totp7, double totb, double totb7, unsigned int width[],
  unsigned int bmult, unsigned int bmult7, double unit, logical name1st,
  logical rightalign, char *name, logical ispage, unsigned int spaces,
  Include *linkhead, char *baseurl, char *datefmt, char *timefmt,
  datecode_t date, unsigned int hr, unsigned int min, datecode_t date2,
  unsigned int hr2, unsigned int min2);
extern void distcount(Hashindex *gooditems, Hashindex *baditems,
  choice requests, choice requests7, unsigned long *tot, unsigned long *tot7);
extern void gensumline(FILE *outf, Outchoices *od, char codeletter,
  int namecode, unsigned long x, unsigned long x7, logical isaverage);
extern void gensumlineb(FILE *outf, Outchoices *od, char codeletter,
  int namecode, double x, double x7, logical isaverage);
extern logical checkonerep(Outchoices *od, Hashindex *gp, choice rep,
  choice requests, cutfnp cutfn, dcutfnp dcutfn, void *darg);
extern logical checktreerep(Outchoices *od, Hashtable *tp, choice rep,
  choice requests, cutfnp cutfn);
extern logical checkarrayrep(Arraydata *array);
extern void checkreps(Outchoices *od, Dateman *dman, Hashindex **gooditems,
  Arraydata **arraydata, choice data2cols[ITEM_NUMBER][DATA_NUMBER]);

/* in outcro.c */
extern unsigned int cro_pagewidth(Outchoices *od);
extern void cro_cgihead(FILE *outf, Outchoices *od);
extern void cro_stylehead(FILE *outf, Outchoices *od);
extern void cro_pagetitle(FILE *outf, Outchoices *od);
extern void cro_timings(FILE *outf, Outchoices *od, Dateman *dman);
extern void cro_closehead(FILE *outf, Outchoices *od);
extern void cro_pagebotstart(FILE *outf, Outchoices *od);
extern void cro_credit(FILE *outf, Outchoices *od);
extern void cro_runtime(FILE *outf, Outchoices *od, long secs);
extern void cro_pagefoot(FILE *outf, Outchoices *od);
extern void cro_stylefoot(FILE *outf, Outchoices *od);
extern void cro_reporttitle(FILE *outf, Outchoices *od, choice rep);
extern void cro_reportfooter(FILE *outf, Outchoices *od, choice rep);
extern void cro_reportdesc(FILE *outf, Outchoices *od, choice rep);
extern void cro_reportspan(FILE *outf, Outchoices *od, choice rep,
  timecode_t maxd, timecode_t mind);
extern void cro_gensumhead(FILE *outf, Outchoices *od);
extern void cro_gensumfoot(FILE *outf, Outchoices *od);
extern void cro_gensumline(FILE *outf, Outchoices *od, int namecode,
  unsigned long x, unsigned long x7, logical isaverage);
extern void cro_gensumlineb(FILE *outf, Outchoices *od, int namecode,
  double x, double x7, logical isaverage);
extern void cro_lastseven(FILE *outf, Outchoices *od, timecode_t last7to);
extern void cro_prestart(FILE *outf, Outchoices *od);
extern void cro_preend(FILE *outf, Outchoices *od);
extern void cro_hrule(FILE *outf, Outchoices *od);
extern char *cro_endash(void);
extern void cro_putch(FILE *outf, char c);
extern size_t cro_strlength(const char *s);
extern logical cro_allowmonth(void);
extern void cro_calcwidths(Outchoices *od, choice rep, unsigned int width[],
  unsigned int *bmult, unsigned int *bmult7, double *unit, unsigned long maxr,
  unsigned long maxr7, unsigned long maxp, unsigned long maxp7, double maxb,
  double maxb7, unsigned long howmany);
extern void cro_declareunit(FILE *outf, Outchoices *od, char graphby,
  double unit, unsigned int bmult);
extern void cro_colheadstart(FILE *outf, Outchoices *od, choice rep);
extern void cro_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned int width, char *colname, logical unterminated);
extern void cro_colheadend(FILE *outf, Outchoices *od, choice rep);
extern void cro_colheadustart(FILE *outf, Outchoices *od, choice rep);
extern void cro_colheadunderline(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *name);
extern void cro_colheaduend(FILE *outf, Outchoices *od, choice rep);
extern void cro_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
  int level, char *name, char *datefmt, char *timefmt);
extern void cro_levelcell(FILE *outf, Outchoices *od, choice rep, int level);
extern void cro_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
  choice source, unsigned int width, logical name1st, logical isfirst,
  logical rightalign, Alias *aliashead, Include *linkhead, logical ispage,
  unsigned int spaces, char *baseurl);
extern void cro_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned long x, unsigned int width);
extern void cro_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
  char *s, unsigned int width);
extern void cro_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
  double b, double bmult, unsigned int width);
extern void cro_pccell(FILE *outf, Outchoices *od, choice rep, choice col,
  double n, double tot, unsigned int width);
extern void cro_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
  long index, unsigned int width);
extern void cro_rowend(FILE *outf, Outchoices *od, choice rep);
extern void cro_blankline(FILE *outf, Outchoices *od, choice *cols);
extern void cro_barchart(FILE *outf, Outchoices *od, int y, char graphby);
extern void cro_busyprintf(FILE *outf, Outchoices *od, choice rep,
  char *datefmt, unsigned long reqs, unsigned long pages, double bys,
  datecode_t date, unsigned int hr, unsigned int min, datecode_t newdate,
  unsigned int newhr, unsigned int newmin, char graphby);
extern void cro_notlistedstr(FILE *outf, Outchoices *od, choice rep,
  unsigned long badn);
extern void cro_whatincluded(FILE *outf, Outchoices *od, choice rep,
  unsigned long n, Dateman *dman);
extern void cro_includefile(FILE *outf, Outchoices *od, char *name,
  char type);
extern unsigned int cro_riscosfiletype(void);

/* in outhtml.c */
extern unsigned int html_pagewidth(Outchoices *od);
extern void html_cgihead(FILE *outf, Outchoices *od);
extern void html_stylehead(FILE *outf, Outchoices *od);
extern void html_pagetitle(FILE *outf, Outchoices *od);
extern void html_timings(FILE *outf, Outchoices *od, Dateman *dman);
extern void html_closehead(FILE *outf, Outchoices *od);
extern void html_pagebotstart(FILE *outf, Outchoices *od);
extern void html_credit(FILE *outf, Outchoices *od);
extern void html_runtime(FILE *outf, Outchoices *od, long secs);
extern void html_pagefoot(FILE *outf, Outchoices *od);
extern void html_stylefoot(FILE *outf, Outchoices *od);
extern void html_reporttitle(FILE *outf, Outchoices *od, choice rep);
extern void html_reportfooter(FILE *outf, Outchoices *od, choice rep);
extern void html_reportdesc(FILE *outf, Outchoices *od, choice rep);
extern void html_reportspan(FILE *outf, Outchoices *od, choice rep,
  timecode_t maxd, timecode_t mind);
extern void html_gensumhead(FILE *outf, Outchoices *od);
extern void html_gensumfoot(FILE *outf, Outchoices *od);
extern void html_gensumline(FILE *outf, Outchoices *od, int namecode,
  unsigned long x, unsigned long x7, logical isaverage);
extern void html_gensumlineb(FILE *outf, Outchoices *od, int namecode,
  double x, double x7, logical isaverage);
extern void html_lastseven(FILE *outf, Outchoices *od, timecode_t last7to);
extern void html_prestart(FILE *outf, Outchoices *od);
extern void html_preend(FILE *outf, Outchoices *od);
extern void html_hrule(FILE *outf, Outchoices *od);
extern char *html_endash(void);
extern void html_putch(FILE *outf, char c);
extern size_t html_strlength(const char *s);
extern size_t html_strlength_utf8(const char *s);
extern size_t html_strlength_jis(const char *s);
extern logical html_allowmonth(void);
extern void html_calcwidths(Outchoices *od, choice rep, unsigned int width[],
  unsigned int *bmult, unsigned int *bmult7, double *unit, unsigned long maxr,
  unsigned long maxr7, unsigned long maxp, unsigned long maxp7, double maxb,
  double maxb7, unsigned long howmany);
extern void html_declareunit(FILE *outf, Outchoices *od, char graphby,
  double unit, unsigned int bmult);
extern void html_colheadstart(FILE *outf, Outchoices *od, choice rep);
extern void html_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned int width, char *colname, logical unterminated);
extern void html_colheadend(FILE *outf, Outchoices *od, choice rep);
extern void html_colheadustart(FILE *outf, Outchoices *od, choice rep);
extern void html_colheadunderline(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *name);
extern void html_colheaduend(FILE *outf, Outchoices *od, choice rep);
extern void html_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
  int level, char *name, char *datefmt, char *timefmt);
extern void html_levelcell(FILE *outf, Outchoices *od, choice rep, int level);
extern void html_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
  choice source, unsigned int width, logical name1st, logical isfirst,
  logical rightalign, Alias *aliashead, Include *linkhead, logical ispage,
  unsigned int spaces, char *baseurl);
extern void html_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned long x, unsigned int width);
extern void html_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
  char *s, unsigned int width);
extern void html_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
  double b, double bmult, unsigned int width);
extern void html_pccell(FILE *outf, Outchoices *od, choice rep, choice col,
  double n, double tot, unsigned int width);
extern void html_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
  long index, unsigned int width);
extern void html_rowend(FILE *outf, Outchoices *od, choice rep);
extern void html_blankline(FILE *outf, Outchoices *od, choice *cols);
extern void html_barchart(FILE *outf, Outchoices *od, int y, char graphby);
extern void html_busyprintf(FILE *outf, Outchoices *od, choice rep,
  char *datefmt, unsigned long reqs, unsigned long pages, double bys,
  datecode_t date, unsigned int hr, unsigned int min, datecode_t newdate,
  unsigned int newhr, unsigned int newmin, char graphby);
extern void html_notlistedstr(FILE *outf, Outchoices *od, choice rep,
  unsigned long badn);
extern void html_whatincluded(FILE *outf, Outchoices *od, choice rep,
  unsigned long n, Dateman *dman);
extern void html_whatinchead(FILE *outf, Outchoices *od);
extern void html_whatincfoot(FILE *outf, Outchoices *od);
extern void html_whatincprintstr(FILE *outf, Outchoices *od, char *s);
extern void html_printdouble(FILE *outf, Outchoices *od, double x);
extern void html_includefile(FILE *outf, Outchoices *od, char *name,
  char type);
extern unsigned int html_riscosfiletype(void);
extern void html_gotos(FILE *outf, Outchoices *od, choice rep);
extern void html_escfprintf(FILE *outf, char *name);
extern void htmlputs(FILE *outf, Outchoices *od, char *s, choice source);
#ifndef NOGRAPHICS
extern FILE *piechart_init(char *filename);
extern void findwedges(Wedge wedge[], choice rep, Hashindex *items,
  choice chartby, Strlist *expandlist, unsigned int level, Strlist *partname,
  unsigned long tot, double totb, double totb7);
extern void piechart_caption(FILE *outf, choice rep, choice chartby,
  char **lngstr);
extern int piechart_wedge(FILE *outf, Outchoices *od, double angle, char *s);
extern void piechart_key(FILE *outf, Outchoices *od, int col, char *name,
  char *extension, Alias *aliashead);
extern void piechart_write(FILE *pieoutf, char *filename, logical jpegcharts);
extern void piechart_cleanup(Wedge wedge[]);
extern void piechart(FILE *outf, Outchoices *od, choice rep, Hashindex *items,
  choice requests, choice requests7, choice pages, choice pages7,
  unsigned long totr, unsigned long totr7, unsigned long totp,
  unsigned long totp7, double totb, double totb7);
#endif

/* in outlatex.c */
extern unsigned int latex_pagewidth(Outchoices *od);
extern void latex_cgihead(FILE *outf, Outchoices *od);
extern void latex_stylehead(FILE *outf, Outchoices *od);
extern void latex_pagetitle(FILE *outf, Outchoices *od);
extern void latex_timings(FILE *outf, Outchoices *od, Dateman *dman);
extern void latex_closehead(FILE *outf, Outchoices *od);
extern void latex_pagebotstart(FILE *outf, Outchoices *od);
extern void latex_credit(FILE *outf, Outchoices *od);
extern void latex_runtime(FILE *outf, Outchoices *od, long secs);
extern void latex_pagefoot(FILE *outf, Outchoices *od);
extern void latex_stylefoot(FILE *outf, Outchoices *od);
extern void latex_reporttitle(FILE *outf, Outchoices *od, choice rep);
extern void latex_reportfooter(FILE *outf, Outchoices *od, choice rep);
extern void latex_reportdesc(FILE *outf, Outchoices *od, choice rep);
extern void latex_reportspan(FILE *outf, Outchoices *od, choice rep,
  timecode_t maxd, timecode_t mind);
extern void latex_gensumhead(FILE *outf, Outchoices *od);
extern void latex_gensumfoot(FILE *outf, Outchoices *od);
extern void latex_gensumline(FILE *outf, Outchoices *od, int namecode,
  unsigned long x, unsigned long x7, logical isaverage);
extern void latex_gensumlineb(FILE *outf, Outchoices *od, int namecode,
  double x, double x7, logical isaverage);
extern void latex_lastseven(FILE *outf, Outchoices *od, timecode_t last7to);
extern void latex_prestart(FILE *outf, Outchoices *od);
extern void latex_preend(FILE *outf, Outchoices *od);
extern void latex_hrule(FILE *outf, Outchoices *od);
extern char *latex_endash(void);
extern void latex_putch(FILE *outf, char c);
extern size_t latex_strlength(const char *s);
extern logical latex_allowmonth(void);
extern void latex_calcwidths(Outchoices *od, choice rep, unsigned int width[],
  unsigned int *bmult, unsigned int *bmult7, double *unit, unsigned long maxr,
  unsigned long maxr7, unsigned long maxp, unsigned long maxp7, double maxb,
  double maxb7, unsigned long howmany);
extern void latex_declareunit(FILE *outf, Outchoices *od, char graphby,
  double unit, unsigned int bmult);
extern void latex_colheadstart(FILE *outf, Outchoices *od, choice rep);
extern void latex_colheadcol(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *colname, logical unterminated);
extern void latex_colheadend(FILE *outf, Outchoices *od, choice rep);
extern void latex_colheadustart(FILE *outf, Outchoices *od, choice rep);
extern void latex_colheadunderline(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *name);
extern void latex_colheaduend(FILE *outf, Outchoices *od, choice rep);
extern void latex_rowstart(FILE *outf, Outchoices *od, choice rep,
  choice *cols, int level, char *name, char *datefmt, char *timefmt);
extern void latex_levelcell(FILE *outf, Outchoices *od, choice rep, int level);
extern void latex_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
  choice source, unsigned int width, logical name1st, logical isfirst,
  logical rightalign, Alias *aliashead, Include *linkhead, logical ispage,
  unsigned int spaces, char *baseurl);
extern void latex_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned long x, unsigned int width);
extern void latex_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
  char *s, unsigned int width);
extern void latex_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
  double b, double bmult, unsigned int width);
extern void latex_pccell(FILE *outf, Outchoices *od, choice rep, choice col,
  double n, double tot, unsigned int width);
extern void latex_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
  long index, unsigned int width);
extern void latex_rowend(FILE *outf, Outchoices *od, choice rep);
extern void latex_blankline(FILE *outf, Outchoices *od, choice *cols);
extern void latex_barchart(FILE *outf, Outchoices *od, int y, char graphby);
extern void latex_busyprintf(FILE *outf, Outchoices *od, choice rep,
  char *datefmt, unsigned long reqs, unsigned long pages, double bys,
  datecode_t date, unsigned int hr, unsigned int min, datecode_t newdate,
  unsigned int newhr, unsigned int newmin, char graphby);
extern void latex_notlistedstr(FILE *outf, Outchoices *od, choice rep,
  unsigned long badn);
extern void latex_whatincluded(FILE *outf, Outchoices *od, choice rep,
  unsigned long n, Dateman *dman);
extern void latex_whatinchead(FILE *outf, Outchoices *od);
extern void latex_whatincfoot(FILE *outf, Outchoices *od);
extern void latex_whatincprintstr(FILE *outf, Outchoices *od, char *s);
extern void latex_printdouble(FILE *outf, Outchoices *od, double x);
extern void latex_includefile(FILE *outf, Outchoices *od, char *name,
  char type);
extern unsigned int latex_riscosfiletype(void);
extern void latexputs(FILE *outf, char *s);

/* in outplain.c */
extern unsigned int plain_pagewidth(Outchoices *od);
extern void plain_cgihead(FILE *outf, Outchoices *od);
extern void plain_stylehead(FILE *outf, Outchoices *od);
extern void plain_pagetitle(FILE *outf, Outchoices *od);
extern void plain_timings(FILE *outf, Outchoices *od, Dateman *dman);
extern void plain_closehead(FILE *outf, Outchoices *od);
extern void plain_pagebotstart(FILE *outf, Outchoices *od);
extern void plain_credit(FILE *outf, Outchoices *od);
extern void plain_runtime(FILE *outf, Outchoices *od, long secs);
extern void plain_pagefoot(FILE *outf, Outchoices *od);
extern void plain_stylefoot(FILE *outf, Outchoices *od);
extern void plain_reporttitle(FILE *outf, Outchoices *od, choice rep);
extern void plain_reportfooter(FILE *outf, Outchoices *od, choice rep);
extern void plain_reportdesc(FILE *outf, Outchoices *od, choice rep);
extern void plain_reportspan(FILE *outf, Outchoices *od, choice rep,
  timecode_t maxd, timecode_t mind);
extern void plain_gensumhead(FILE *outf, Outchoices *od);
extern void plain_gensumfoot(FILE *outf, Outchoices *od);
extern void plain_gensumline(FILE *outf, Outchoices *od, int namecode,
  unsigned long x, unsigned long x7, logical isaverage);
extern void plain_gensumlineb(FILE *outf, Outchoices *od, int namecode,
  double x, double x7, logical isaverage);
extern void plain_lastseven(FILE *outf, Outchoices *od, timecode_t last7to);
extern void plain_prestart(FILE *outf, Outchoices *od);
extern void plain_preend(FILE *outf, Outchoices *od);
extern void plain_hrule(FILE *outf, Outchoices *od);
extern char *plain_endash(void);
extern void plain_putch(FILE *outf, char c);
extern size_t plain_strlength(const char *s);
extern logical plain_allowmonth(void);
extern void plain_calcwidths(Outchoices *od, choice rep, unsigned int width[],
  unsigned int *bmult, unsigned int *bmult7, double *unit, unsigned long maxr,
  unsigned long maxr7, unsigned long maxp, unsigned long maxp7, double maxb,
  double maxb7, unsigned long howmany);
extern void plain_declareunit(FILE *outf, Outchoices *od, char graphby,
  double unit, unsigned int bmult);
extern void plain_colheadstart(FILE *outf, Outchoices *od, choice rep);
extern void plain_colheadcol(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *colname, logical unterminated);
extern void plain_colheadend(FILE *outf, Outchoices *od, choice rep);
extern void plain_colheadustart(FILE *outf, Outchoices *od, choice rep);
extern void plain_colheadunderline(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *name);
extern void plain_colheaduend(FILE *outf, Outchoices *od, choice rep);
extern void plain_rowstart(FILE *outf, Outchoices *od, choice rep,
  choice *cols, int level, char *name, char *datefmt, char *timefmt);
extern void plain_levelcell(FILE *outf, Outchoices *od, choice rep, int level);
extern void plain_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
  choice source, unsigned int width, logical name1st, logical isfirst,
  logical rightalign, Alias *aliashead, Include *linkhead, logical ispage,
  unsigned int spaces, char *baseurl);
extern void plain_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned long x, unsigned int width);
extern void plain_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
  char *s, unsigned int width);
extern void plain_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
  double b, double bmult, unsigned int width);
extern void plain_pccell(FILE *outf, Outchoices *od, choice rep, choice col,
  double n, double tot, unsigned int width);
extern void plain_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
  long index, unsigned int width);
extern void plain_rowend(FILE *outf, Outchoices *od, choice rep);
extern void plain_blankline(FILE *outf, Outchoices *od, choice *cols);
extern void plain_barchart(FILE *outf, Outchoices *od, int y, char graphby);
extern void plain_busyprintf(FILE *outf, Outchoices *od, choice rep,
  char *datefmt, unsigned long reqs, unsigned long pages, double bys,
  datecode_t date, unsigned int hr, unsigned int min, datecode_t newdate,
  unsigned int newhr, unsigned int newmin, char graphby);
extern void plain_notlistedstr(FILE *outf, Outchoices *od, choice rep,
  unsigned long badn);
extern void plain_whatincluded(FILE *outf, Outchoices *od, choice rep,
  unsigned long n, Dateman *dman);
extern void plain_whatinchead(FILE *outf, Outchoices *od);
extern void plain_whatincfoot(FILE *outf, Outchoices *od);
extern void plain_whatincprintstr(FILE *outf, Outchoices *od, char *s);
extern void plain_printdouble(FILE *outf, Outchoices *od, double x);
extern void plain_includefile(FILE *outf, Outchoices *od, char *name,
  char type);
extern unsigned int plain_riscosfiletype(void);

/* in outxhtml.c */
extern unsigned int xhtml_pagewidth(Outchoices *od);
extern void xhtml_cgihead(FILE *outf, Outchoices *od);
extern void xhtml_stylehead(FILE *outf, Outchoices *od);
extern void xhtml_pagetitle(FILE *outf, Outchoices *od);
extern void xhtml_timings(FILE *outf, Outchoices *od, Dateman *dman);
extern void xhtml_closehead(FILE *outf, Outchoices *od);
extern void xhtml_pagebotstart(FILE *outf, Outchoices *od);
extern void xhtml_credit(FILE *outf, Outchoices *od);
extern void xhtml_runtime(FILE *outf, Outchoices *od, long secs);
extern void xhtml_pagefoot(FILE *outf, Outchoices *od);
extern void xhtml_stylefoot(FILE *outf, Outchoices *od);
extern void xhtml_reporttitle(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_reportfooter(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_reportdesc(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_reportspan(FILE *outf, Outchoices *od, choice rep,
  timecode_t maxd, timecode_t mind);
extern void xhtml_gensumhead(FILE *outf, Outchoices *od);
extern void xhtml_gensumfoot(FILE *outf, Outchoices *od);
extern void xhtml_gensumline(FILE *outf, Outchoices *od, int namecode,
  unsigned long x, unsigned long x7, logical isaverage);
extern void xhtml_gensumlineb(FILE *outf, Outchoices *od, int namecode,
  double x, double x7, logical isaverage);
extern void xhtml_lastseven(FILE *outf, Outchoices *od, timecode_t last7to);
extern void xhtml_prestart(FILE *outf, Outchoices *od);
extern void xhtml_preend(FILE *outf, Outchoices *od);
extern void xhtml_hrule(FILE *outf, Outchoices *od);
extern char *xhtml_endash(void);
extern void xhtml_putch(FILE *outf, char c);
extern size_t xhtml_strlength(const char *s);
extern logical xhtml_allowmonth(void);
extern void xhtml_calcwidths(Outchoices *od, choice rep, unsigned int width[],
  unsigned int *bmult, unsigned int *bmult7, double *unit, unsigned long maxr,
  unsigned long maxr7, unsigned long maxp, unsigned long maxp7, double maxb,
  double maxb7, unsigned long howmany);
extern void xhtml_declareunit(FILE *outf, Outchoices *od, char graphby,
  double unit, unsigned int bmult);
extern void xhtml_colheadstart(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_colheadcol(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *colname, logical unterminated);
extern void xhtml_colheadend(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_colheadustart(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_colheadunderline(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *name);
extern void xhtml_colheaduend(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_rowstart(FILE *outf, Outchoices *od, choice rep,
  choice *cols, int level, char *name, char *datefmt, char *timefmt);
extern void xhtml_levelcell(FILE *outf, Outchoices *od, choice rep, int level);
extern void xhtml_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
  choice source, unsigned int width, logical name1st, logical isfirst,
  logical rightalign, Alias *aliashead, Include *linkhead, logical ispage,
  unsigned int spaces, char *baseurl);
extern void xhtml_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned long x, unsigned int width);
extern void xhtml_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
  char *s, unsigned int width);
extern void xhtml_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
  double b, double bmult, unsigned int width);
extern void xhtml_pccell(FILE *outf, Outchoices *od, choice rep, choice col,
  double n, double tot, unsigned int width);
extern void xhtml_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
  long index, unsigned int width);
extern void xhtml_rowend(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_blankline(FILE *outf, Outchoices *od, choice *cols);
extern void xhtml_barchart(FILE *outf, Outchoices *od, int y, char graphby);
extern void xhtml_busyprintf(FILE *outf, Outchoices *od, choice rep,
  char *datefmt, unsigned long reqs, unsigned long pages, double bys,
  datecode_t date, unsigned int hr, unsigned int min, datecode_t newdate,
  unsigned int newhr, unsigned int newmin, char graphby);
extern void xhtml_notlistedstr(FILE *outf, Outchoices *od, choice rep,
  unsigned long badn);
extern void xhtml_whatincluded(FILE *outf, Outchoices *od, choice rep,
  unsigned long n, Dateman *dman);
extern void xhtml_whatinchead(FILE *outf, Outchoices *od);
extern void xhtml_whatincfoot(FILE *outf, Outchoices *od);
extern void xhtml_whatincprintstr(FILE *outf, Outchoices *od, char *s);
extern void xhtml_printdouble(FILE *outf, Outchoices *od, double x);
extern void xhtml_includefile(FILE *outf, Outchoices *od, char *name,
  char type);
extern unsigned int xhtml_riscosfiletype(void);
extern void xhtml_gotos(FILE *outf, Outchoices *od, choice rep);
extern void xhtml_escfprintf(FILE *outf, char *name);
extern void xhtmlputs(FILE *outf, Outchoices *od, char *s, choice source);

/* in outxml.c */
extern unsigned int xml_pagewidth(Outchoices *od);
extern void xml_cgihead(FILE *outf, Outchoices *od);
extern void xml_stylehead(FILE *outf, Outchoices *od);
extern void xml_pagetitle(FILE *outf, Outchoices *od);
extern void xml_timings(FILE *outf, Outchoices *od, Dateman *dman);
extern void xml_closehead(FILE *outf, Outchoices *od);
extern void xml_pagebotstart(FILE *outf, Outchoices *od);
extern void xml_credit(FILE *outf, Outchoices *od);
extern void xml_runtime(FILE *outf, Outchoices *od, long secs);
extern void xml_pagefoot(FILE *outf, Outchoices *od);
extern void xml_stylefoot(FILE *outf, Outchoices *od);
extern void xml_reporttitle(FILE *outf, Outchoices *od, choice rep);
extern void xml_reportfooter(FILE *outf, Outchoices *od, choice rep);
extern void xml_reportdesc(FILE *outf, Outchoices *od, choice rep);
extern void xml_reportspan(FILE *outf, Outchoices *od, choice rep,
  timecode_t maxd, timecode_t mind);
extern void xml_gensumhead(FILE *outf, Outchoices *od);
extern void xml_gensumfoot(FILE *outf, Outchoices *od);
extern void xml_gensumline(FILE *outf, Outchoices *od, int namecode,
  unsigned long x, unsigned long x7, logical isaverage);
extern void xml_gensumlineb(FILE *outf, Outchoices *od, int namecode,
  double x, double x7, logical isaverage);
extern void xml_lastseven(FILE *outf, Outchoices *od, timecode_t last7to);
extern void xml_prestart(FILE *outf, Outchoices *od);
extern void xml_preend(FILE *outf, Outchoices *od);
extern void xml_hrule(FILE *outf, Outchoices *od);
extern char *xml_endash(void);
extern void xml_putch(FILE *outf, char c);
extern size_t xml_strlength(const char *s);
extern logical xml_allowmonth(void);
extern void xml_calcwidths(Outchoices *od, choice rep, unsigned int width[],
  unsigned int *bmult, unsigned int *bmult7, double *unit, unsigned long maxr,
  unsigned long maxr7, unsigned long maxp, unsigned long maxp7, double maxb,
  double maxb7, unsigned long howmany);
extern void xml_declareunit(FILE *outf, Outchoices *od, char graphby,
  double unit, unsigned int bmult);
extern void xml_colheadstart(FILE *outf, Outchoices *od, choice rep);
extern void xml_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned int width, char *colname, logical unterminated);
extern void xml_colheadend(FILE *outf, Outchoices *od, choice rep);
extern void xml_colheadustart(FILE *outf, Outchoices *od, choice rep);
extern void xml_colheadunderline(FILE *outf, Outchoices *od, choice rep,
  choice col, unsigned int width, char *name);
extern void xml_colheaduend(FILE *outf, Outchoices *od, choice rep);
extern void xml_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
  int level, char *name, char *datefmt, char *timefmt);
extern void xml_levelcell(FILE *outf, Outchoices *od, choice rep, int level);
extern void xml_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
  choice source, unsigned int width, logical name1st, logical isfirst,
  logical rightalign, Alias *aliashead, Include *linkhead, logical ispage,
  unsigned int spaces, char *baseurl);
extern void xml_ulcell(FILE *outf, Outchoices *od, choice rep, choice col,
  unsigned long x, unsigned int width);
extern void xml_strcell(FILE *outf, Outchoices *od, choice rep, choice col,
  char *s, unsigned int width);
extern void xml_bytescell(FILE *outf, Outchoices *od, choice rep, choice col,
  double b, double bmult, unsigned int width);
extern void xml_pccell(FILE *outf, Outchoices *od, choice rep, choice col,
  double n, double tot, unsigned int width);
extern void xml_indexcell(FILE *outf, Outchoices *od, choice rep, choice col,
  long index, unsigned int width);
extern void xml_rowend(FILE *outf, Outchoices *od, choice rep);
extern void xml_blankline(FILE *outf, Outchoices *od, choice *cols);
extern void xml_barchart(FILE *outf, Outchoices *od, int y, char graphby);
extern void xml_busyprintf(FILE *outf, Outchoices *od, choice rep,
  char *datefmt, unsigned long reqs, unsigned long pages, double bys,
  datecode_t date, unsigned int hr, unsigned int min, datecode_t newdate,
  unsigned int newhr, unsigned int newmin, char graphby);
extern void xml_notlistedstr(FILE *outf, Outchoices *od, choice rep,
  unsigned long badn);
extern void xml_whatincluded(FILE *outf, Outchoices *od, choice rep,
  unsigned long n, Dateman *dman);
extern void xml_whatinchead(FILE *outf, Outchoices *od);
extern void xml_whatincfoot(FILE *outf, Outchoices *od);
extern void xml_whatincprintstr(FILE *outf, Outchoices *od, char *s);
extern void xml_printdouble(FILE *outf, Outchoices *od, double x);
extern void xml_includefile(FILE *outf, Outchoices *od, char *name,
  char type);
extern unsigned int xml_riscosfiletype(void);

/* in process.c */
extern void process_data(Logfile *logfilep, Hashtable **hash,
  Arraydata **arraydata, choice *count, choice *code2type,
  choice datacols[ITEM_NUMBER][OUTCOME_NUMBER][DATACOLS_NUMBER][2],
  choice data2cols[ITEM_NUMBER][DATA_NUMBER], unsigned int *no_cols,
  Include **wanthead, Include *ispagehead, Alias **aliashead,
  Include *argshead, Include *refargshead, Dateman *dman, Tree **tree,
  Derv **derv, choice *alltrees, choice *alldervs, choice *lowmem,
  logical case_insensitive, logical usercase_insensitive,
  unsigned char convfloor, logical multibyte, char *dirsuffix,
  unsigned int dirsufflength, unsigned int granularity);
extern void corrupt_line(Logfile *logfilep, char *message, ptrdiff_t n);
extern void arrayscore(Arraydata *array, double amount, unsigned long reqs,
  unsigned long reqs7, unsigned long pages, unsigned long pages7, double bytes,
  double bytes7, timecode_t timecode); 

/* in settings.c */
extern void report_vbles(Options *op);
extern void report_compile(void);
extern void report_debug(char *type, char *arg, char *fullset);
extern void report_conffiles(Strlist *list);
extern void report_logfiles(Logfile *logfile, logical cache);
extern void report_logformat(FILE *outf, Inputformatlist *format,
  logical inwarn);
extern void report_fromto(Dateman *dman);
extern void report_want(Include *wanthead, char *type, char *indent);
extern void report_scwant(choice *code2type);
extern void report_alias(Alias *aliashead, char *type, char *indent);
extern void report_strpairlist(Strpairlist *head, char *type, char *connector);
extern void report_lowmem(choice lowmem[]);
#ifndef NODNS
extern void report_dns(void);
#endif
extern void report_outopts(Outchoices *od);
extern void report_sep(char c, char *type);
extern void report_daterep(Outchoices *od, choice j);
extern void report_genrep(Outchoices *od, choice j);
extern void report_cols(choice *cols);
extern void report_sortby(choice sortby, logical sub);
extern void report_floor(Floor *floor, logical sub);
#ifndef NOGRAPHICS
extern void report_chart(choice chartby);
#endif
extern void check_file(char *name);

/* in sort.c */
extern void calctotmax(Hashindex **gooditems, Hashindex **baditems,
  Strlist *partname, Strlist **newpn, Strlist *space, size_t need, choice rep,
  Include *wanthead, choice requests, choice requests7, choice pages,
  choice pages7, choice date, choice firstd, unsigned long *totr,
  unsigned long *totr7, unsigned long *totp, unsigned long *totp7,
  double *totb, double *totb7, unsigned long *maxr, unsigned long *maxr7,
  unsigned long *maxp, unsigned long *maxp7, double *maxb, double *maxb7,
  timecode_t *maxd, timecode_t *mind, logical donetots, logical nosort,
  Hashindex **ans3);
extern void calcfloor(unsigned long *min, double *dmin, choice *filterby,
  Floor *floor, choice requests, choice requests7, choice pages, choice pages7,
  choice date, choice firstd, unsigned long totr, unsigned long totr7,
  unsigned long totp, unsigned long totp7, double totb, double totb7,
  unsigned long maxr, unsigned long maxr7, unsigned long maxp,
  unsigned long maxp7, double maxb, double maxb7);
extern void makebadpn(Hashentry **badp, unsigned long *badn, Hashindex *ans2,
  choice requests, choice requests7, choice pages, choice pages7, choice date,
  choice firstd, unsigned long *maxr, unsigned long *maxr7,
  unsigned long *maxp, unsigned long *maxp7, double *maxb, double *maxb7,
  logical donetots);
extern void my_sort(Hashindex **gooditems, Hashindex **baditems,
  Strlist *partname, Strlist **newpn, Strlist *space, size_t need, choice rep,
  Floor *floor, choice sortby, logical alphaback, Include *wanthead,
  choice requests, choice requests7, choice pages, choice pages7, choice date,
  choice firstd, unsigned long *totr, unsigned long *totr7,
  unsigned long *totp, unsigned long *totp7, double *totb, double *totb7,
  unsigned long *maxr, unsigned long *maxr7, unsigned long *maxp,
  unsigned long *maxp7, double *maxb, double *maxb7, timecode_t *maxd,
  timecode_t *mind, logical donetots, Hashentry **badp, unsigned long *badn,
  logical nosort);
extern Hashindex *my_mergesort(Hashindex *list, unsigned long length,
  choice sortby, mergefnp mergefn);
extern Hashindex *merge(Hashindex *list1, Hashindex *list2,
  unsigned long length1, unsigned long length2, choice sortby);
extern Hashindex *mergea(Hashindex *list1, Hashindex *list2,
  unsigned long length1, unsigned long length2, choice sortby);
extern Hashindex *mergeb(Hashindex *list1, Hashindex *list2,
  unsigned long length1, unsigned long length2, choice sortby);
extern Hashindex *mergeb7(Hashindex *list1, Hashindex *list2,
  unsigned long length1, unsigned long length2, choice sortby);

/* in tree.c */
extern Hashindex *treefind(char *name, char *nameend, Hashtable **tree,
  Hashindex *item, cutfnp cutfn, logical build, logical transient,
  logical reuse, Memman *space,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2],
  unsigned int data_number);
extern void graft(Hashtable **new, Hashtable *old, Memman *space,
  unsigned int data_number);
extern void allgraft(Hashtable *t, Memman *space, unsigned int data_number);
extern Hashindex *newtreeentry(char *name, char *nameend, Hashindex *item,
  logical transient, logical reuse, Memman *space, unsigned int data_number);
extern Hashentry *newtreedata(Hashentry *from, Memman *space,
  unsigned int data_number);
extern void treescore(Hashentry *to, Hashentry *from,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2]);
extern Hashindex *sorttree(Outchoices *od, Hashtable *tree, choice rep,
  Floor *floor, choice sortby, Floor *subfloor, choice subsortby,
  logical alphaback, unsigned int level, Strlist *partname, Alias *notcorrupt,
  choice requests, choice requests7, choice pages, choice pages7, choice date,
  choice firstd, unsigned long *totr, unsigned long *totr7,
  unsigned long *totp, unsigned long *totp7, double *totb, double *totb7,
  unsigned long *maxr, unsigned long *maxr7, unsigned long *maxp,
  unsigned long *maxp7, double *maxb, double *maxb7, timecode_t *maxd,
  timecode_t *mind, Hashentry **badp, unsigned long *badn, Memman *space,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2]);
extern void maketree(Tree *treex, Hashindex *gooditems, Hashindex *baditems,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2],
  unsigned int data_number);
extern void makederived(Derv *derv, Hashindex *gooditems, Hashindex *baditems,
  unsigned char convfloor, logical multibyte, choice rep,
  choice datacols[OUTCOME_NUMBER][DATACOLS_NUMBER][2],
  unsigned int data_number);
extern char *maketreename(Strlist *pn, Hashindex *p, Strlist **newpn,
  Strlist *space, size_t need, choice rep, logical delims);
extern logical genstreq(char *a, char *b, char *t);
extern void rnextname(char **name, char **nameend, char *whole, logical build);
extern void inextname(char **name, char **nameend, char *whole, logical build);
extern void onextname(char **name, char **nameend, char *whole, logical build);
extern void tnextname(char **name, char **nameend, char *whole, logical build);
extern void snextname(char **name, char **nameend, char *whole, logical build);
extern void Znextname(char **name, char **nameend, char *whole, logical build);
extern void bnextname(char **name, char **nameend, char *whole, logical build);
extern void pnextname(char **name, char **nameend, char *whole, logical build);
extern void Bnextname(char **name, char **nameend, char *whole, void *arg);
extern void Pnextname(char **name, char **nameend, char *whole, void *arg);
extern void Nnextname(char **name, char **nameend, char *whole, void *arg);
extern void nnextname(char **name, char **nameend, char *whole, void *arg);

/* in utils.c */
extern void my_exit(int status);
extern void sighandler(int sig);
extern logical substrcaseeq(char *s, char *t);
extern char *strtoupper(char *n);
extern void strtolowerx(char *n);
extern void toloweralias(Alias *ap, logical both);
extern void tolowerinc(Include *ip);
extern void tolowerse(Strpairlist *sp);
extern void reversealias(Alias **head);
extern void reversestrpairlist(Strpairlist **head);
extern logical strcaseeq(char *s, char *t);
extern char *strrpbrk(char *s, char *t);
#ifdef EBCDIC
extern char *strtoascii(char *s);
#endif
extern char *buildfilename(char *old, char *sub, char *name);
#ifdef NEED_STRCMP
extern int my_strcmp(char *s, char *t);
#endif
extern unsigned int chrn(char *s, char c);
extern unsigned int chrdistn(char *s, char c);
extern unsigned int log10i(unsigned long i);
extern unsigned int log10x(double d);
extern unsigned int findbmult(double d, unsigned int dp);
extern int atoi255(const char *s);
extern unsigned long arraymaxl(unsigned long *x, unsigned int size);
extern double arraymaxd(double *x, unsigned int size);
extern size_t arraymaxlen(char **s, unsigned int size, Outchoices *od);
extern void prettyprintf(FILE *outf, unsigned int pagewidth, char *fmt,
  va_list ap);
extern void mprintf(FILE *outf, unsigned int pagewidth, char *fmt, ...);
extern void print_progversion(FILE *f);
extern void warn(char c, choice domess, char *s, ...);
extern void error(char *s, ...);
extern void debug(char c, char *s, ...);
extern char *delimit(char *s);
extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern void *submalloc(Memman *m, size_t size);
extern void freemm(Memman *m);
#ifdef NEED_MEMMOVE
extern void *memmove(void *dst0, const void *src0, size_t length);
#endif
extern logical wildmatch(char *s, char *p, char *whole, int nmatch,
  int *pmatch);
extern logical matchq(char *s, void *p, logical is_regex, int *pmatch);
extern logical genwildmatch(char *s, char *s2, char *p);
extern logical headmatch(char *s, char *p);
extern logical headcasematch(char *s, char *p);
extern logical matchiprange(char *s, unsigned long minaddr,
  unsigned long maxaddr);
extern logical included(char *name, logical ispage, Include *listhead);
extern logical incstrlist(char *name, Strlist *listhead);
extern logical pageq(char *name, Include *ispagehead, choice type);

/* in win32.c */
#ifdef WIN32
extern void Win32Init(void);
extern void Win32Cleanup(void);
#endif  /* WIN32 */

#endif  /* ANLGHEA4_H */
