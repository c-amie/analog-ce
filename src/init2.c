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

/*** init2.c; subsiduary functions for parsing config files */
/* See also init.c */

#include "anlghea3.h"

/*** All config commands have the following structure:
  void config.....(void *opt, char *cmd, char *arg1, char *arg2, int rc)
  opt is the variable to be changed, "cmd arg1 arg2" is the config command
  issued, rc is the number of arguments retrieved from the config line (0..3),
  or -1 if the config command was issued by the program internally,
  or -2 if it was issued by the program in parsing command line (for those
  -2's which don't go through confline(), cmd is command line option). -3 is
  occasionally used for special cases. ***/

/* First some warnings of things that can go wrong in config commands.
   NB rc = -1 announced to be because anlghead.h got messed up. But it
   could be an internal error caused by a bug. */

void shortwarn(char *cmd, char *arg1, int rc) {
  char *s = "Not enough arguments for configuration command: ignoring it";
  if (rc == -1)
    error("Default given for %s in anlghead.h too short");
  else if (arg1 == NULL)
    warn('C', TRUE, "%s:\n%s", s, cmd);
  else
    warn('C', TRUE, "%s:\n%s %s", s, cmd, delimit(arg1));
}

void longwarn(char *cmd, char *arg1, char *arg2, int rc) {
  char *s = "Too many arguments for configuration command: "
    "ignoring end of line starting";
  if (rc == -1)
    warn('C', TRUE, "Default given for %s in anlghead.h too long: "
	 "ignoring end of it", cmd);
  else if (arg2 == NULL)
    warn('C', TRUE, "%s:\n%s %s", s, cmd, delimit(arg1));
  else
    warn('C', TRUE, "%s:\n%s %s %s", s, cmd, delimit(arg1), delimit(arg2));
}

void badwarn(char *cmd, choice domess, char *arg1, char *arg2, int rc) {
  char *s = "Bad argument in configuration command: ignoring it";
  if (rc == -2)
    warn('C', TRUE, "Bad argument in command line argument %s: ignoring it",
	 cmd);
  else if (rc == -1)
    error("Incorrect default given for %s in anlghead.h", cmd);
  else if (arg2 == NULL)
    warn('C', domess, "%s:\n%s %s", s, cmd, delimit(arg1));
  else
    warn('C', domess, "%s:\n%s %s %s", s, cmd, delimit(arg1), delimit(arg2));
}

void unknownwarn(char *cmd, char *arg1, char *arg2) {
  char *s = "Unknown configuration command: ignoring it";
  if (arg1 == NULL)
    warn('C', TRUE, "%s:\n%s", s, cmd);  
  else if (arg2 == NULL)
    warn('C', TRUE, "%s:\n%s %s", s, cmd, delimit(arg1));
  else
    warn('C', TRUE, "%s:\n%s %s %s", s, cmd, delimit(arg1), delimit(arg2));
}

void configcall(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  /* calling function will then call config() */
}

void configcols(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  extern char colcodes[];

  choice *cols = (choice *)opt;  /* see also configallcols() */

  logical warn1 = FALSE, warn2 = FALSE, warn3;
  char *c;
  int i = 0, j, k;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  for (c = arg1; *c != '\0' && i < COL_NUMBER - 1; c++, i++) {
    for (cols[i] = COL_NUMBER, k = 0;
	 cols[i] == COL_NUMBER && colcodes[k] != '\0';
	 k++) {
      if (*c == colcodes[k])
	cols[i] = k;
    }
    if (cols[i] == COL_NUMBER) {
      i--;
      if (!warn1) {
	warn('C', TRUE, "Ignoring unknown columns in\n%s %s", cmd, arg1);
	warn1 = TRUE;
      }
    }
    /* check for repeated columns */
    else if ((cols[i] == COL_DATE && strchr(arg1, 'D') != NULL) ||
	     (cols[i] == COL_FIRSTD && strchr(arg1, 'E') != NULL)) {
      i--;      /* cancel i++ in for loop, so overwriting this column */
      if (!warn2) {
	warn('C', TRUE,
	     "Ignoring repeated columns in configuration command\n%s %s",
	     cmd, arg1);
	warn2 = TRUE;
      }
    }
    else {
      for (j = 0, warn3 = FALSE; j < i && !warn3; j++) { 
	if (cols[j] == cols[i]) {         /* shortest code, not least work */
	  warn3 = TRUE;
	  i--;   /* see above */
	  if (!warn2) {
	    warn('C', TRUE,
		 "Ignoring repeated columns in configuration command\n%s %s",
		 cmd, arg1);
	    warn2 = TRUE;
	  }
	}
      }
    }
  }  /* for c */
  if (*c != '\0' /* can this happen? */ || rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  cols[i] = COL_NUMBER;
}

void configallcols(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  choice *cols = (choice *)opt;
  int j;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  for (j = 0; j < DATEREP_NUMBER; j++)    /* possibility of dup. warns */
    configcols((void *)&(cols[j * COL_NUMBER]), cmd, arg1, NULL, -3);
}

/* A set of symbolic word choices for use with configchoice() below. Must all
   end with "". */
Choices sortbychoices[] = {{"REQUESTS", REQUESTS}, {"REQUESTS7", REQUESTS7},
  {"PAGES", PAGES}, {"PAGES7", PAGES7}, {"BYTES", BYTES}, {"BYTES7", BYTES7},
  {"LASTDATE", DATESORT}, {"DATE", DATESORT}, {"FIRSTDATE", FIRSTDATE},
  {"ALPHABETICAL", ALPHABETICAL}, {"RANDOM", RANDOM}, {"", 0}};
#ifndef NOGRAPHICS
Choices chartchoices[] = {{"ON", CHART_SORTBY}, {"TRUE", CHART_SORTBY},
  {"SORTBY", CHART_SORTBY}, {"OFF", CHART_NONE}, {"FALSE", CHART_NONE},
  {"NONE", CHART_NONE}, {"REQUESTS", REQUESTS}, {"REQUESTS7", REQUESTS7},
  {"PAGES", PAGES}, {"PAGES7", PAGES7}, {"BYTES", BYTES}, {"BYTES7", BYTES7},
  {"", 0}};
#endif
Choices outstylechoices[] = {{"HTML", HTML}, {"XHTML", XHTML},
  {"PLAIN", PLAIN}, {"ASCII", ASCII}, {"LATEX", LATEX}, {"COMPUTER", COMPUTER},
  {"PREFORMATTED", COMPUTER}, {"CRO", COMPUTER}, {"XML", XML},
  {"CACHE", OUT_NONE}, {"NONE", OUT_NONE}, {"", 0}};
#ifndef NODNS
Choices dnschoices[] = {{"NONE", DNS_NONE}, {"OFF", DNS_NONE},
  {"READ", DNS_READ}, {"LOOKUP", DNS_LOOKUP}, {"WRITE", DNS_WRITE}, {"", 0}};
#endif
Choices gotochoices[] = {{"ON", TRUE}, {"TRUE", TRUE}, {"ALL", TRUE},
  {"OFF", FALSE}, {"FALSE", FALSE}, {"NONE", FALSE}, {"FEW", FEW},
  {"SOME", FEW}, {"", 0}};
Choices casechoices[] = {{"INSENSITIVE", TRUE}, {"SENSITIVE", FALSE}, {"", 0}};
Choices daychoices[] = {{"SUNDAY", SUNDAY}, {"MONDAY", MONDAY},
  {"TUESDAY", TUESDAY}, {"WEDNESDAY", WEDNESDAY}, {"THURSDAY", THURSDAY},
  {"FRIDAY", FRIDAY}, {"SATURDAY", SATURDAY}, {"", 0}};
Choices langchoices[] = {{"ARMENIAN", ARMENIAN}, {"BASQUE", BASQUE},
  {"BOSNIAN", OLDLANG},
  {"BULGARIAN", BULGARIAN}, {"BULGARIAN-MIK", BULGARIAN_MIK},
  {"CATALAN", CATALAN},
  {"SIMP-CHINESE", SIMP_CHINESE}, {"CHINESE", SIMP_CHINESE},
  {"TRAD-CHINESE", TRAD_CHINESE}, {"TAIWANESE", TRAD_CHINESE},
  {"CROATIAN", OLDLANG}, {"CZECH", CZECH}, {"CZECH-1250", CZECH_1250},
  {"DANISH", DANISH}, {"DUTCH", DUTCH}, {"FLEMISH", DUTCH},
  {"ENGLISH", ENGLISH}, {"US-ENGLISH", US_ENGLISH}, {"FINNISH", FINNISH},
  {"FRENCH", FRENCH}, {"GERMAN", GERMAN}, {"GREEK", OLDLANG},
  {"HUNGARIAN", HUNGARIAN}, {"ICELANDIC", OLDLANG}, {"INDONESIAN", INDONESIAN},
  {"ITALIAN", ITALIAN},
  {"JAPANESE-EUC", JAPANESE_EUC}, {"JAPANESE-JIS", JAPANESE_JIS},
  {"JAPANESE-SJIS", JAPANESE_SJIS}, {"JAPANESE-UTF", JAPANESE_UTF},
  {"JAPANESE", JAPANESE_JIS}, {"KOREAN", KOREAN}, {"LATVIAN", LATVIAN},
  {"LITHUANIAN", OLDLANG}, {"NORWEGIAN", NORWEGIAN}, {"BOKMAL", NORWEGIAN},
  {"BOKMAAL", NORWEGIAN}, {"NYNORSK", NYNORSK}, {"POLISH", POLISH},
  {"PORTUGUESE", PORTUGUESE}, {"PORTUGESE", PORTUGUESE},
  {"BR-PORTUGUESE", BR_PORTUGUESE}, {"BR-PORTUGESE", BR_PORTUGUESE},
  {"BRAZILIAN", BR_PORTUGUESE}, {"ROMANIAN", OLDLANG}, {"RUSSIAN", RUSSIAN},
  {"RUSSIAN-1251", RUSSIAN_1251},
  {"SERBIAN", SERBIAN}, {"SERBOCROATIAN", SERBIAN}, {"SERBOCROAT", SERBIAN},
  {"SLOVAK", SLOVAK}, {"SLOVAK-1250", SLOVAK_1250},
  {"SLOVENE", SLOVENE}, {"SLOVENIAN", SLOVENE},
  {"SLOVENE-1250", SLOVENE_1250}, {"SLOVENIAN-1250", SLOVENE_1250},
  {"SPANISH", SPANISH},
  {"SWEDISH", SWEDISH}, {"SWEDISH-ALT", SWEDISH_ALT}, {"TURKISH", TURKISH},
  {"UKRAINIAN", UKRAINIAN}, {"UKRANIAN", UKRAINIAN}, {"", 0}};
Choices onoffchoices[] = {{"ON", TRUE}, {"TRUE", TRUE}, {"OFF", FALSE},
  {"FALSE", FALSE}, {"", 0}};

void configchoice(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  Choices *choices;
  choice *ans = (choice *)opt;
  logical *ansl = (logical *)opt;
  char *cmdend = strchr(cmd, '\0');
  int i;
  logical done, islog = FALSE;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }

  if (STREQ(cmdend - 6, "SORTBY"))
    choices = sortbychoices;
#ifndef NOGRAPHICS
  else if (STREQ(cmdend - 5, "CHART"))
    choices = chartchoices;
#endif
  else if (STREQ(cmd, "OUTPUT"))
    choices = outstylechoices;
#ifndef NODNS
  else if (STREQ(cmd, "DNS"))
    choices = dnschoices;
#endif
  else if (STREQ(cmd, "LANGUAGE"))
    choices = langchoices;
  else if (STREQ(cmd, "GOTOS"))
    choices = gotochoices;
  else if (STREQ(cmdend - 4, "CASE")) {
    choices = casechoices;
    islog = TRUE;
  }
  else if (STREQ(cmd, "WEEKBEGINSON"))
    choices = daychoices;
  else if (STREQ(cmd, "SEARCHCHARCONVERT"))
    choices = onoffchoices;
  else {
    choices = onoffchoices;
    islog = TRUE;
  }

  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  for (i = 0, done = FALSE; !done && !IS_EMPTY_STRING(choices[i].name); i++) {
    if (strcaseeq(arg1, choices[i].name)) {
      if (islog)
	*ansl = (logical)(choices[i].arg);
      else
	*ans = choices[i].arg;
      /* I'm sure islog is unnecessary -- that we can safely pass a logical in
	 to this function and pretend it's a choice in here. But let's pretend
	 we're in a strongly-typed language. :-) */
      done = TRUE;
    }
  }
  if (!done)
    badwarn(cmd, TRUE, arg1, arg2, rc);
}

#ifndef NOGRAPHICS
void configallchart(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  /* only takes ON and OFF to avoid invalid choices */
  choice *q = (choice *)opt;
  choice p = UNSET;
  int i;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (strcaseeq(arg1, "ON") || strcaseeq(arg1, "TRUE") ||
      strcaseeq(arg1, "SORTBY"))
    p = CHART_SORTBY;
  else if (strcaseeq(arg1, "OFF") || strcaseeq(arg1, "FALSE") ||
	   strcaseeq(arg1, "NONE"))
    p = CHART_NONE;
 
 if (p == UNSET)
    badwarn(cmd, TRUE, arg1, arg2, rc);
  else for (i = 0; i < GENREP_NUMBER; i++)
    q[i] = p;
}
#endif  /* NOGRAPHICS */

void configdebug(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char **args = (char **)opt;
  char *a, *c;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  if (strcaseeq(arg1, "ON") || strcaseeq(arg1, "TRUE") ||
      strcaseeq(arg1, "ALL"))
    configstr(args, NULL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", NULL, -1);
  else if (strcaseeq(arg1, "OFF") || strcaseeq(arg1, "FALSE") ||
	   strcaseeq(arg1, "NONE"))
    configstr(args, NULL, "", NULL, -1);
  else {
    strtoupper(arg1);
    if (arg1[0] == '-') {
      while ((c = strpbrk(*args, arg1 + 1)) != NULL)
	memmove((void *)c, (void *)(c + 1), strlen(c));
    }
    else if (arg1[0] == '+') {  /* c.f. configstr() */
      *args = (char *)xrealloc((void *)(*args), strlen(arg1) + strlen(*args));
      for (a = strchr(*args, '\0'), c = arg1 + 1; *c != '\0'; c++) {
	if (!strchr(*args, *c)) {
	  *a = *c;
	  *(++a) = '\0';
	}
      }
    }
    else
      configstr(args, NULL, arg1, NULL, -1);
  }
}

void configall(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  logical *q = (logical *)opt;
  choice result = UNSET;
  int i;

  configchoice((void *)(&result), cmd, arg1, arg2, rc);
  if (result != UNSET) {
    for (i = 0; i < REP_NUMBER; i++) {
      if (i != REP_GENSUM)
	q[i] = result;
    }
  }
}

void configallback(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  logical *q = (logical *)opt;  /* same as configall(), but only DATEREPORTs */
  choice result = UNSET;
  int i;

  configchoice((void *)(&result), cmd, arg1, arg2, rc);
  if (result != UNSET) {
    for (i = 0; i < DATEREPORTS_NUMBER; i++)
      q[i] = (logical)result;
  }
}

void configlang(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  Lang *lang = (Lang *)opt;
  choice code = UNSET;

  configchoice((void *)(&code), cmd, arg1, arg2, rc);
  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  if (code == OLDLANG) {
    badwarn(cmd, FALSE, arg1, arg2, rc);
    warn('C', CONTINUATION, "  (language not yet translated for version 5)");
  }
  else if (code != UNSET) {  /* code == UNSET warning in configchoice() */
    lang->code = code;
    lang->file = NULL;
  }
}

void selectlang(char *country, Outchoices *op) {
  /* Select localisation files with country code. If outstyle is HTML/XHTML
     or ASCII, look first for special files for those styles. */
  char partname[13];
  /* The longest name we might want to build is 3 for the country code,
     1 for h or a, 8 for "desc.txt" and terminating \0. */
  char *filename = NULL;  /* Just to keep the compiler happy */
  FILE *f;

  if (op->outstyle == HTML || op->outstyle == XHTML || op->outstyle == ASCII) {
    sprintf(partname, "%s%c%clng", country, (op->outstyle == ASCII)?'a':'h',
	    EXTSEP);
    filename = buildfilename(LANGDIR, "lang", partname);
  }
  if ((op -> outstyle != HTML && op->outstyle != XHTML &&
       op->outstyle != ASCII) || (f = FOPENR(filename)) == NULL) {
    sprintf(partname, "%s%clng", country, EXTSEP);
    filename = buildfilename(LANGDIR, "lang", partname);
  }
  else
    fclose(f);
  COPYSTR(op->lang.file, filename);

  if (op->domainsfile == NULL) {
    if (op->outstyle == HTML || op->outstyle == XHTML ||
	op->outstyle == ASCII) {
      sprintf(partname, "%s%cdom%ctab", country,
	      (op->outstyle == ASCII)?'a':'h', EXTSEP);
      filename = buildfilename(LANGDIR, "lang", partname);
    }
    if ((op->outstyle == HTML || op->outstyle == XHTML ||
	 op->outstyle == ASCII) && (f = FOPENR(filename)) != NULL)
      fclose(f);
    else {
      sprintf(partname, "%sdom%ctab", country, EXTSEP);
      filename = buildfilename(LANGDIR, "lang", partname);
      if ((f = FOPENR(filename)) == NULL)
	filename = NULL;  /* and will get set to ukdom.tab on return */
      else
	fclose(f);
    }
    if (filename != NULL)
      COPYSTR(op->domainsfile, filename);
  }

  if (op->descriptions && op->descfile == NULL) {
    if (op->outstyle == HTML || op->outstyle == XHTML ||
	op->outstyle == ASCII) {
      sprintf(partname, "%s%cdesc%ctxt", country,
	      (op->outstyle == ASCII)?'a':'h', EXTSEP);
      filename = buildfilename(LANGDIR, "lang", partname);
    }
    if ((op->outstyle == HTML || op->outstyle == XHTML ||
	 op->outstyle == ASCII) && (f = FOPENR(filename)) != NULL)
      fclose(f);
    else {
      sprintf(partname, "%sdesc%ctxt", country, EXTSEP);
      filename = buildfilename(LANGDIR, "lang", partname);
      if ((f = FOPENR(filename)) == NULL)
	filename = NULL;
      else
	fclose(f);
    }
    if (filename != NULL)
      COPYSTR(op->descfile, filename);
  }
}

void configlogfmt(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  extern Memman *xmemman;
  Inputformatlist **logfmt = (Inputformatlist **)opt;
  Inputformatlist *fp;
  Inputformat *form;
  choice count[INPUT_NUMBER];
  choice rc2;
  char code;
  int i;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (strcaseeq(arg1, "DEFAULT")) {
    (*logfmt)->used = TRUE;
    configlogfmt(opt, NULL, "%x0", NULL, -1);
    (*logfmt)->used = TRUE;
  }
  else if (strcaseeq(arg1, "AUTO")) {
    (*logfmt)->used = TRUE;
    configlogfmt(opt, NULL, "%x1", NULL, -1);
    (*logfmt)->used = TRUE;
  }
  else if (strcaseeq(arg1, "COMMON")) {
    configlogfmt(opt, NULL, LOG_COMMON1, NULL, -1);
    configlogfmt(opt, NULL, LOG_COMMON2, NULL, -1);
    configlogfmt(opt, NULL, LOG_COMMON3, NULL, -1);
  }
  else if (strcaseeq(arg1, "MS-COMMON")) {
    configlogfmt(opt, NULL, LOG_MS_COMMON1, NULL, -1);
    configlogfmt(opt, NULL, LOG_COMMON2, NULL, -1);
    configlogfmt(opt, NULL, LOG_COMMON3, NULL, -1);
  }
  else if (strcaseeq(arg1, "COMBINED")) {
    configlogfmt(opt, NULL, LOG_COMBINED1, NULL, -1);
    configlogfmt(opt, NULL, LOG_COMBINED2, NULL, -1);
    configlogfmt(opt, NULL, LOG_COMBINED3, NULL, -1);
  }
  else if (strcaseeq(arg1, "MICROSOFT-NA") ||
	   strcaseeq(arg1, "MICROSOFT-NA2") ||
	   strcaseeq(arg1, "MICROSOFT-NA4")) {  /* NA2 and NA4 are legacy */
    configlogfmt(opt, NULL, LOG_MS_NA1, NULL, -1);
    configlogfmt(opt, NULL, LOG_MS_NA2, NULL, -1);
  }
  else if (strcaseeq(arg1, "MICROSOFT-INT") ||
	   strcaseeq(arg1, "MICROSOFT-INT2") ||
	   strcaseeq(arg1, "MICROSOFT-INT4")) {
    configlogfmt(opt, NULL, LOG_MS_INT1, NULL, -1);
    configlogfmt(opt, NULL, LOG_MS_INT2, NULL, -1);
  }
  else if (strcaseeq(arg1, "WEBSITE-NA"))
    configlogfmt(opt, NULL, LOG_WEBSITE_NA, NULL, -1);
  else if (strcaseeq(arg1, "WEBSITE-INT"))
    configlogfmt(opt, NULL, LOG_WEBSITE_INT, NULL, -1);
  else if (strcaseeq(arg1, "WEBSTAR")) {
    configlogfmt(opt, NULL, LOG_WEBSTAR1, NULL, -1);
    configlogfmt(opt, NULL, LOG_WEBSTAR2, NULL, -1);
  }
  else if (strcaseeq(arg1, "EXTENDED")) {
    configlogfmt(opt, NULL, LOG_EXTENDED1, NULL, -1);
    configlogfmt(opt, NULL, LOG_EXTENDED2, NULL, -1);
  }
  else if (strcaseeq(arg1, "MS-EXTENDED")) {
    configlogfmt(opt, NULL, LOG_MS_EXTENDED1, NULL, -1);
    configlogfmt(opt, NULL, LOG_EXTENDED2, NULL, -1);
  }
  else if (strcaseeq(arg1, "WEBSTAR-EXTENDED")) {
    configlogfmt(opt, NULL, LOG_WEBSTAR_EXTENDED1, NULL, -1);
    configlogfmt(opt, NULL, LOG_EXTENDED2, NULL, -1);
  }
  else if (strcaseeq(arg1, "MACHTTP"))
    configlogfmt(opt, NULL, LOG_MACHTTP, NULL, -1);
  else if (strcaseeq(arg1, "NETSCAPE"))
    configlogfmt(opt, NULL, LOG_NETSCAPE, NULL, -1);
  else if (strcaseeq(arg1, "BROWSER")) 
    configlogfmt(opt, NULL, LOG_BROWSER, NULL, -1);
  else if (strcaseeq(arg1, "REFERRER") || strcaseeq(arg1, "REFERER")) {
    configlogfmt(opt, NULL, LOG_REFERRER1, NULL, -1);
    configlogfmt(opt, NULL, LOG_REFERRER2, NULL, -1);
  }
  else if ((rc2 = strtoinfmt(&form, arg1, count)) != FMT_OK) {
    code = (rc == -3)?'F':'C';
    if (rc == -3)
      warn(code, FALSE, "Ignoring corrupt format line in logfile");
    else
      badwarn(cmd, FALSE, arg1, arg2, rc);
    if (rc2 == FMT_DUP)
      warn(code, CONTINUATION, "  (reason: one item occurs twice in format)");
    else if (rc2 == FMT_PARTTIME)
      warn(code, CONTINUATION, "  (reason: time without date or vice versa)");
    else if (rc2 == FMT_BADPC)
      warn(code, CONTINUATION, "  (reason: an unknown item code is present)");
    else if (rc2 == FMT_NOPC)
      warn(code, CONTINUATION, "  (reason: no item codes are present)");
    else if (rc2 == FMT_BADCHAR)
      warn(code, CONTINUATION,
	   "  (reason: an unknown escape sequence is present)");
    else if (rc2 == FMT_NOTERM)
      warn(code, CONTINUATION,
	   "  (reason: an unterminated string is present)");
    else if (rc2 == FMT_BADBUILTIN)
      warn(code, CONTINUATION, "  (reason: non-existent built-in format)");
    else if (rc2 == FMT_QBUTNOR)
      warn(code, CONTINUATION, "  (reason: query string without filename)");
    /* despite finishing "else if", that should be all the rc2's */
    return;
  }
  else {
    if ((*logfmt)->used) {
      fp = (Inputformatlist *)submalloc(xmemman, sizeof(Inputformatlist));
      *logfmt = fp;
      (*logfmt)->used = FALSE;
    }
    else {
      for (fp = *logfmt; fp->next != NULL; TO_NEXT(fp))
	;
      fp->next = (Inputformatlist *)submalloc(xmemman,
					      sizeof(Inputformatlist));
      TO_NEXT(fp);
    }
    fp->form = form;
    for (i = 0; i < INPUT_NUMBER; i++)
      fp->count[i] = count[i];
    fp->next = NULL;
  }
}

void configapachelogfmt(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char *p;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if ((p = apachelogfmt(arg1)) == NULL)
    warn('C', TRUE,
	 "Sorry, can't parse \"%%...{format}t\" in %s: ignoring whole line",
	 cmd);

  else if (STREQ(cmd, "APACHELOGFORMAT"))
    configlogfmt(opt, "APACHELOGFORMAT -> LOGFORMAT", p, NULL, 1);
  else
    configlogfmt(opt, "APACHEDAFAULTLOGFORMAT -> DEFAULTLOGFORMAT", p,
		 NULL, 1);
}

void configrepord(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  extern char repcodes[];
  char *s1 = "REPORTORDER";
  char *s2 = "REPORTORDER in anlghead.h";
  char *s = (rc == -1)?s2:s1;
  choice *order = (choice *)opt;
  logical used[REP_NUMBER];
  int i, j, k;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }

  for (i = 0; i < REP_NUMBER; i++)
    used[i] = 0;
  for (i = 0, k = 0; k < REP_NUMBER && arg1[i] != '\0'; i++) {
    if (ISALNUM(arg1[i])) {  /* else ignore */
      for (j = 0; repcodes[j] != arg1[i] && repcodes[j] != '\0'; j++)
	;
      if (repcodes[j] == '\0')
	warn('C', TRUE, "Spurious character %c in %s: ignoring it", arg1[i],
	     s);
      else if (used[j] == 0) {
	order[k++] = j;
	used[j] = 1;
      }
      else if (used[j] == 1) {
	warn('C', TRUE, "Character %c used more than once in %s: "
	     "ignoring later occurrences", arg1[i], s);
	used[j] = 2;
      }
    }
  }

  if (rc > 1 || arg1[i] != '\0')
    longwarn(cmd, arg1, arg2, rc);

  if (k < REP_NUMBER) {
    for (i = 0; i < REP_NUMBER && k < REP_NUMBER; i++) {
      if (used[i] == 0) {     /* k should stay < R_N automatically, but... */
	warn('C', TRUE, "Character %c not used in %s: adding it at end",
	     repcodes[i], s);
	order[k++] = i;
      }
    }
  }
  order[k] = -1;
}

void configstr(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char **s = (char **)opt;
  size_t len;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  len = strlen(arg1);
  *s = (char *)xrealloc((void *)(*s), len + 1);
  strcpy(*s, arg1);
}

void configstrlist(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  Strlist **list = (Strlist **)opt;
  Strlist *lp = NULL;
  char *t;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc == -1)  /* have to be careful because can't strtok a const string */
    t = arg1;
  else if ((t = strtok(arg1, ",")) == NULL) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (*list != NULL) {
    for (lp = *list; lp->next != NULL; TO_NEXT(lp))
      ;  /* find end of list */
  }
  for ( ; t != NULL; t = ((rc == -1)?NULL:strtok((char *)NULL, ","))) {
    if (strcaseeq(t, "none"))
      *list = NULL;
    else {
      if (*list == NULL) {
	lp = (Strlist *)xmalloc(sizeof(Strlist));
	*list = lp;
      }
      else {
	lp->next = (Strlist *)xmalloc(sizeof(Strlist));
	TO_NEXT(lp);
      }
      if (STREQ(cmd, "DOMCHARTEXPAND") && *t == '.')  /* ugly special case */
	t++;
      COPYSTR(lp->name, t);
      lp->next = NULL;
    }
  }
  return;
}

void configerrfile(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  /* NB Don't use freopen(), coz fatal errors sent to both errfile & stderr. */
  FILE **err = (FILE **)opt;
  FILE *tmp;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (strcaseeq(arg1, "stderr")) {
    if (*err != stderr) {
      debug('F', "Opening stderr as new ERRFILE");
      warn('E', TRUE, "Redirecting future diagnostic messages to stderr");
      fclose(*err);
      *err = stderr;
    }
  }
  else {
    arg1 = buildfilename(ERRDIR, "", arg1);
    if ((tmp = FOPENW(arg1)) == NULL)
      warn('F', TRUE, "Failed to open ERRFILE %s: ignoring it", arg1);
    else {
      debug('F', "Opening %s as new ERRFILE", arg1);
      warn('E', TRUE, "Redirecting future diagnostic messages to %s", arg1);
      if (*err != stderr)
	fclose(*err);
      *err = tmp;
      setvbuf(*err, NULL, ERRBUFMODE, BUFSIZ);
    }
  }
}

/* There are several versions of expandwildlogs(), according to the following
   flags:
     NODIRENT excludes all wildcards in logfile names.
     Otherwise, if VMSDIRENT, MACDIRENT, WIN32DIRENT or RISCOSDIRENT is
     defined, use that.
     Otherwise use POSIX.2 glob, unless NOGLOB is defined when use POSIX.1
     stuff, not allowing wildcards in directory names.
     MACDIRENT does actually include the POSIX.1 dirent code here, but
     implements all the functions itself in macstuff.c.
   In all cases, expandwildlogs() takes an argument, Logfile *lp, and clones
   it within the list (using clonelogs()) as it expands the wildcards. It
   returns a pointer to the last clone. E.g. expandwildlogs(log2*) turns
   log1 -> log2* -> log3 into log1 -> log2a -> log2b -> log3 returning log2b */
#ifndef NODIRENT
Logfile *clonelogs(Logfile *from, char *name) {
  Logfile *ans = (Logfile *)xmalloc(sizeof(Logfile));

  memcpy(ans, from, sizeof(Logfile));
  COPYSTR(ans->name, name);
  ans->next = from->next;
  from->next = ans;
  return(ans);
} 

#ifndef VMSDIRENT
#ifndef WIN32DIRENT
#ifndef RISCOSDIRENT
#ifndef NOGLOB
/* Glob code is due to Owen Cliffe Feb 2001, slightly modified by
   Stephen Turner */
Logfile *expandwildlogs(Logfile *lp, Logfile **pter) {
  Logfile *ans = lp;
  glob_t globbuf;
  struct stat statbuf;
  int i;

  globbuf.gl_offs = 0;
  if(glob(lp->name,GLOB_NOCHECK,NULL,&globbuf) != 0){
    warn('F', TRUE, "Failed to expand wildcards %s", lp->name);
    return(ans);
  }
   
   for(i=0;i<globbuf.gl_pathc;i++){
      char * fname= globbuf.gl_pathv[i];
      if (stat(fname, &statbuf) == 0 && S_ISREG(statbuf.st_mode))
	ans = clonelogs(ans, fname);
   }
   globfree(&globbuf);
   if (ans != lp)  /* remove expanded wildcard lp from list */
     *pter = lp->next;
   return(ans);
}
#else   /* not GLOB; use POSIX dirent */
Logfile *expandwildlogs(Logfile *lp, Logfile **pter) {
  Logfile *ans = lp;
  struct dirent *filep;
  struct stat buf;
  DIR *dirp;
  char *dirname, *filename, *c;
  size_t dirlen, len = 0;

  if ((c = strrpbrk(lp->name, PATHSEPS)) == NULL) {
    filename = lp->name;
    dirname = (char *)xmalloc(3);
#ifdef MAC
    dirname[0] = '\0';
#else
    sprintf(dirname, ".%c", DIRSEP);
#endif
  }
  else {
    filename = c + 1;
    dirname = (char *)xmalloc((size_t)(c - lp->name) + 2);
    memcpy((void *)dirname, (void *)(lp->name), (size_t)(c - lp->name) + 1);
    dirname[(c - lp->name) + 1] = '\0';
  }

  if ((dirp = opendir(dirname)) == NULL) {
    if (strchr(dirname, '*') != NULL || strchr(dirname, '?') != NULL)
      warn('F', TRUE, "Cannot open directory %s: won't expand wildcards %s\n"
	   "(wildcards not allowed in directory name)", dirname, lp->name);
    else
      warn('F', TRUE, "Cannot open directory %s: won't expand wildcards %s",
	   dirname, lp->name);
    return(ans);
  }

  dirlen = strlen(dirname);
  while ((filep = readdir(dirp)) != NULL) {
    if (MATCHES(filep->d_name, filename)) {
      if (strlen(filep->d_name) > len) {
	len = strlen(filep->d_name);  /* d_namlen is not portable */
	dirname = (char *)xrealloc((void *)dirname, dirlen + len + 1);
      }
      memcpy((void *)(dirname + dirlen), (void *)(filep->d_name), len + 1);
      stat(dirname, &buf);  /* dirname now contains complete filename */
      if (S_ISREG(buf.st_mode))
	ans = clonelogs(ans, dirname);
    }
  }
  closedir(dirp);
  free((void *)dirname);
  if (ans != lp)
    *pter = lp->next;
  return(ans);
}
#endif   /* NOGLOB */
#else    /* RISCOSDIRENT */
/* The RISC OS dirent is due to Stefan Bellon (sbellon@sbellon.de) */
static struct {
    unsigned int load, exec, length, attributes, type;
    char name[255]; /* WARNING: arbitrary limit */
} direntry;

Logfile *expandwildlogs(Logfile *lp, Logfile **pter) {
  Logfile *ans = lp;
  char *dirname, *leafname, *filename, *c;
  int count, context = 0;
  _kernel_oserror *e;
  size_t dirlen, path = 0, len = 0;

  if ((c = strrpbrk(lp->name, PATHSEPS)) == NULL) {
    path = 0;
    dirname = (char *)xmalloc(1);
    dirname[0] = '\0';
    leafname = lp->name;
  }
  else {
    path = 1;
    dirname = (char *)xmalloc((size_t)(c - lp->name) + 1);
    memcpy((void *)dirname, (void *)(lp->name), (size_t)(c - lp->name));
    dirname[(c - lp->name)] = '\0';
    leafname = c + 1;
  }
  dirlen = strlen(dirname);
  filename = (char *)xmalloc(dirlen + path + 1);
  memcpy(filename, dirname, dirlen);
  filename[dirlen] = DIRSEP;
  filename[dirlen + path] = '\0'; /* may overwrite the DIRSEP again */

  while (context != -1) {
    e = _swix(OS_GBPB, _INR(0,6) | _OUTR(3,4),
              10, dirname, &direntry, 1, context, sizeof(direntry), leafname,
              &count, &context);
    if (e != NULL)
      error("%s\n", e->errmess);

    if (count > 0 && direntry.type == 1) {
      if (strlen(direntry.name) > len) {
        len = strlen(direntry.name);
        filename = (char *)xrealloc((void *)filename, dirlen + path + len + 1);
      }
      memcpy((void *)(filename + dirlen + path),
	     (void *)(direntry.name), len + 1);
      ans = clonelogs(ans, filename);
    }
  }

  free((void *)dirname);
  free((void *)filename);
  if (ans != lp)
    *pter = lp->next;
  return(ans);
}
#endif   /* RISCOSDIRENT */
#else    /* WIN32DIRENT */
Logfile *expandwildlogs(Logfile *lp, Logfile **pter) {
  Logfile *ans = lp;
  struct _stat buf;
  char *dirname, *c;
  struct _finddata_t file;
  long code;
  int rc2;
  size_t dirlen, len = 0;

  if ((code = _findfirst(lp->name, &file)) != (long)(-1)) {
    if ((c = strrpbrk(lp->name, PATHSEPS)) == NULL) {
      dirname = (char *)xmalloc(1);
      dirname[0] = '\0';
    }
    else {
      dirname = (char *)xmalloc((size_t)(c - lp->name) + 2);
      memcpy((void *)dirname, (void *)(lp->name), (size_t)(c - lp->name) + 1);
      dirname[(c - lp->name) + 1] = '\0';
    }
    dirlen = strlen(dirname);
    for (rc2 = 0; rc2 == 0; rc2 = _findnext(code, &file)) {
      if (strlen(file.name) > len) {
	len = strlen(file.name);
	dirname = (char *)xrealloc((void *)dirname, dirlen + len + 1);
      }
      memcpy((void *)(dirname + dirlen), (void *)(file.name), len + 1);
      _stat(dirname, &buf);  /* dirname now contains complete filename */
      if (_S_IFREG & buf.st_mode)    /* bitwise & */
	ans = clonelogs(ans, dirname);
    }
    _findclose(code);
    free((void *)dirname);
  }
  if (ans != lp)
    *pter = lp->next;
  return(ans);
}
#endif   /* WIN32DIRENT */
#else    /* VMSDIRENT */
/* This function is due to Dave Jones (except for any errors I introduced when
   converting from old configwildlogs() to new expandwildlogs()) */
Logfile *expandwildlogs(Logfile *lp, Logfile **pter) {
  Logfile *ans = lp;
  static char fspec[VMS_FSPEC_MAX], related[VMS_FSPEC_MAX];
  static char result[VMS_FSPEC_MAX];
  static $DESCRIPTOR(fspec_dx,fspec);
  static $DESCRIPTOR(related_dx,"");
  static $DESCRIPTOR(default_dx,".log");
  static $DESCRIPTOR(result_dx,result);
  char *space, *ques;
  long int context;
  int status, stsval, length, LIB$FIND_FILE(), LIB$FIND_FILE_END();
  
  length = strlen (lp->name);
  if ( length >= VMS_FSPEC_MAX ) length = VMS_FSPEC_MAX - 1;
  strncpy ( fspec, lp->name, length ); fspec[length] = '\0';
  while ( ques = strchr(fspec,'?') ) *ques = '%';
  fspec_dx.dsc$w_length = length;
  for ( context = 0; 
	1&(status=LIB$FIND_FILE ( &fspec_dx, &result_dx, &context, &default_dx,
				  &related_dx, &stsval, (long *) 0 )); ) {
    space = strchr ( result, ' ' );
    if ( !space ) space = &result[VMS_FSPEC_MAX-1];
    *space = '\0';
    ans = clonelogs(ans, result);
    /* Save last result to use as default for next lookup */
    strcpy ( related, result );
    related_dx.dsc$w_length = strlen(result);
    related_dx.dsc$a_pointer = related;
  }
  if ( context ) LIB$FIND_FILE_END ( &context );
  if (ans != lp)
    *pter = lp->next;
  return(ans);
}
#endif  /* VMSDIRENT */
#endif  /* NODIRENT */

void configlogfile(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  extern Inputformatlist *logformat;
  extern logical newloglist, iscache;
  /* see note in configcachefile() below re iscache */
  extern int tz;

  Logfile **logfile = (Logfile **)opt;
  /* logfile[0] is logfiles, logfile[1] is cache files */
  Logfile *lf;
  char *s, *t;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 2)
    longwarn(cmd, arg1, arg2, rc);

  if (strchr(arg1, ',') != NULL) {
    s = (char *)xmalloc(strlen(arg1) + 1);
    strcpy(s, arg1);  /* can't strtok arg1 directly in case it's const */
    if ((t = strtok(s, ",")) == NULL) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      free((void *)s);
      return;
    }
    for ( ; t != NULL; t = strtok((char *)NULL, ","))
      configlogfile(opt, "LOGFILE", t, arg2, rc);
    free((void *)s);
    return;
  }
  if (strcaseeq(arg1, "NONE")) {
    /* from LOGFILE not CACHEFILE because CACHEFILE NONE caught earlier */
    configlogfmt((void *)&logformat, "LOGFORMAT", "DEFAULT", NULL, -1);
    logfile[iscache] = NULL;
    return;
  }
  if (newloglist) {
    logfile[0] = NULL;
    logfile[1] = NULL;
    newloglist = FALSE;
  }
  if (logfile[iscache] == NULL) {
    logfile[iscache] = (Logfile *)xmalloc(sizeof(Logfile));
    lf = logfile[iscache];
  }
  else {
    for (lf = logfile[iscache]; lf->next != NULL; TO_NEXT(lf))
      ;
    lf->next = (Logfile *)xmalloc(sizeof(Logfile));
    TO_NEXT(lf);
  }
  if (IS_STDIN(arg1) || rc == -2) {
    COPYSTR(lf->name, arg1);
  }
  else
    lf->name = buildfilename(iscache?(char *)CACHEDIR:(char *)LOGSDIR, "",
			     arg1);

  /* Non-configuration options initialised in my_lfopen */
  lf->type = LF_NOTOPENED;
  lf->format = logformat;
  lf->tz = tz;
  if (rc >= 2) {  /* not from CACHEFILE: those are caught earlier */
    COPYSTR(lf->prefix, arg2);
    lf->prefixlen = strlen(lf->prefix);
    if (strstr(arg2, "%v") == NULL)
      lf->pvpos = UNSET;
    else
      lf->pvpos = strstr(arg2, "%v") - arg2;
  }
  else
    lf->prefix = NULL;
  lf->next = NULL;
  if (!iscache)
    logformat->used = TRUE;
}

void configcachefile(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  /* a wrapper to configlogfile(), but we have to catch a few cases first */
  Logfile **logfile = (Logfile **)opt;
  extern logical iscache;
  /* iscache tells configlogfile() to use logfile[1] instead of logfile[0],
     not to change logformat->used, and to use CACHEDIR instead of LOGSDIR. */

  if (rc > 1) {
    longwarn(cmd, arg1, arg2, rc);
    rc = 1;
  }
  if (strcaseeq(arg1, "NONE")) {
    logfile[1] = NULL;
    return;
  }
  iscache = TRUE;
  configlogfile(opt, cmd, arg1, NULL, rc);
  iscache = FALSE;
}

void configoutfile(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char **file = (char **)opt;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (IS_STDOUT(arg1) || rc == -2) {
    COPYSTR(*file, arg1);
  }
  else
    *file = buildfilename(OUTDIR, "", arg1);
}

void configchar(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char *c = (char *)opt;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  if (IS_EMPTY_STRING(arg1) || strcaseeq(arg1, "none"))
    *c = '\0';
  else {
    if (arg1[1] != '\0')
      longwarn(cmd, arg1, arg2, rc);
    *c = arg1[0];
  }
}

void configbarstyle(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char *g = (char *)opt;
  char h;

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1 || arg1[1] != '\0')
    longwarn(cmd, arg1, arg2, rc);
  h = TOLOWER(arg1[0]);
  if (h != 'a' && h != 'b' && h != 'c' && h != 'd' && h != 'e' && h != 'f' &&
      h != 'g' && h != 'h' && h != 'i' && h != 'j')
    badwarn(cmd, TRUE, arg1, arg2, rc);
  else
    *g = h;
}

void configgraph(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char *g = (char *)opt;  /* see also configallgraph() below */

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1 || arg1[1] != '\0')
    longwarn(cmd, arg1, arg2, rc);
  if (arg1[0] != 'R' && arg1[0] != 'P' && arg1[0] != 'B' && arg1[0] != 'r' &&
      arg1[0] != 'p' && arg1[0] != 'b')
    badwarn(cmd, TRUE, arg1, arg2, rc);
  else
    *g = arg1[0];
}

void configallgraph(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char *g = (char *)opt;   /* same as configgraph() */
  int i;

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1 || arg1[1] != '\0')
    longwarn(cmd, arg1, arg2, rc);
  if (arg1[0] != 'R' && arg1[0] != 'P' && arg1[0] != 'B' && arg1[0] != 'r' &&
      arg1[0] != 'p' && arg1[0] != 'b')
    badwarn(cmd, TRUE, arg1, arg2, rc);
  else {
    for (i = 0; i < DATEREP_NUMBER; i++)
      g[i] = arg1[0];
  }
}

void configfloor(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  /* This function parses the following formats for floor
     r = requests, p = pages, b = bytes,
     s = 7-day requests, q = 7-day pages, c = 7-day bytes,
     d = last date, e = first date.
     -100(r|s|p|q|b|c|d|e)      // top 100
     1000(r|s|p|q|b|c)          // at least 1000
     9.5(k|M|G|T|P|E|Z|Y)(b|c)  // ditto
     0.5%(r|s|p|q|b|c)          // at least 0.5% of total
     0.5:(r|s|p|q|b|c)          // at least 0.5% of largest
     970701(d|e)                // last/first access since
     -00-0201(d|e)              // same with a relative date
     Upper case letters for r|p|b|c|d are also acceptable. */
  extern time_t starttime;
  extern char *byteprefix;

  Floor *floor = (Floor *)opt;
  char **b, *c, d;
  choice floorby;
  char qual = '\0';
  double mn;
  timecode_t t;

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);
  c = arg1 + strlen(arg1) - 1;
  d = TOUPPER(*c);
  if (d == 'R')
    floorby = REQUESTS;
  else if (d == 'S')
    floorby = REQUESTS7;
  else if (d == 'P')
    floorby = PAGES;
  else if (d == 'Q')
    floorby = PAGES7;
  else if (d == 'B')
    floorby = BYTES;
  else if (d == 'C')
    floorby = BYTES7;
  else if (d == 'D')
    floorby = DATESORT;
  else if (d == 'E')
    floorby = FIRSTDATE;
  else {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  if (floorby != DATESORT && floorby != FIRSTDATE) {
    b = (char **)xmalloc(sizeof(char *));
    mn = strtod(arg1, b);
    if (*b == arg1) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      return;
    }
    else if (*b == c - 1) {
      if (mn < 0) {
	badwarn(cmd, TRUE, arg1, arg2, rc);
	return;
      }
      else if (**b == '%' || **b == ':' ||
	       ((floorby == BYTES || floorby == BYTES7) &&
		strchr(byteprefix + 1, **b)))
	qual = **b;
      else {
	badwarn(cmd, TRUE, arg1, arg2, rc);
	return;
      }
    }
    free((void *)b);
  }
  else { /* floorby == DATESORT or FIRSTDATE */
    if (arg1[0] == '-' && arg1[1] != '0' && ISDIGIT(arg1[1]))
      mn = atof(arg1);
    else if (parsedate(starttime, arg1, &t, TRUE, FALSE) == ERR) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      return;
    }
    else
      mn = (double)t;
  }
  floor->min = mn;
  floor->qual = qual;
  floor->floorby = floorby;
}

void configtree(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  Tree **treex = (Tree **)opt;
  Hashindex item;
  Hashentry *own;
  char *name, *nameend, *t;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (strchr(arg1, ',') != NULL) {
    if ((t = strtok(arg1, ",")) == NULL) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      return;
    }
    for ( ; t != NULL; t = strtok((char *)NULL, ","))
      configtree(opt, cmd, t, NULL, -1);
    return;
  }
  own = newhashentry(DATA_NUMBER, FALSE);
  item.own = own;
  item.name = arg1;
  name = NULL;
  (*treex)->cutfn(&name, &nameend, item.name, TRUE);
  (void)treefind(name, nameend, &((*treex)->tree), &item, (*treex)->cutfn,
		 TRUE, TRUE, FALSE, (*treex)->space, NULL, DATA_NUMBER);
}

void configulong(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  unsigned int *x = (unsigned int *)opt;
  char **c;

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (!ISDIGIT(arg1[0])) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  c = (char **)xmalloc(sizeof(char *));
  *x = strtoul(arg1, c, 10);
  if (rc > 1 || **c != '\0')
    longwarn(cmd, arg1, arg2, rc);
  free((void *)c);
}

void configuint(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  unsigned int *x = (unsigned int *)opt;
  char **c;

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (!ISDIGIT(arg1[0])) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  c = (char **)xmalloc(sizeof(char *));
  *x = (unsigned int)strtoul(arg1, c, 10);
  if (rc > 1 || **c != '\0')
    longwarn(cmd, arg1, arg2, rc);
  free((void *)c);
}

void configoffset(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  /* configint with extra checks */
  int *x = (int *)opt;
  char **c;
  int y;

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (arg1[0] == '+') {
    if (!ISDIGIT(arg1[1])) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      return;
    }
    arg1++;
  }
  else if (!ISDIGIT(arg1[0]) && !(arg1[0] == '-' && ISDIGIT(arg1[1]))) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  c = (char **)xmalloc(sizeof(char *));
  y = (int)strtol(arg1, c, 10);
  if (rc > 1 || **c != '\0')
    longwarn(cmd, arg1, arg2, rc);
  if (y > 40320 || y < -40320)
    warn('C', TRUE, "Ignoring offset of more than 28 days in configuration "
	 "command\n%s %s", cmd, arg1);
  else {
    *x = y;
    if (*x % 30 != 0)
      warn('D', TRUE,
	   "Offset not a multiple of 30 in configuration command\n%s %s",
	   cmd, arg1);
    else if (*x > 1500 || *x < -1500)
      warn('D', TRUE,
	   "Offset more than 25 hours in configuration command\n%s %s",
	   cmd, arg1);
  }
  free((void *)c);
}

void configlowmem(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  choice *x = (choice *)opt;

  if (rc == 0 || IS_EMPTY_STRING(arg1)) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (!(arg1[0] >= '0' && arg1[0] <= '3')) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  if (rc > 1 || arg1[1] != '\0')
    longwarn(cmd, arg1, arg2, rc);
  *x = (choice)(arg1[0] - '0');
}

int aliastocount(char *s) {
  int n;

  if (headcasematch(s, "PLAIN:"))
    return(0);

  n = (strchr(s, '*') != NULL);
  while ((s = strchr(s, '$')) != NULL) {
    s++;
    if (*s < '1' || *s > '9')
      s++;
    else
      n = MAX(n, (*s - '0'));
  }
  return(n);
}

AliasTo *configaliasto(char *s, logical is_regex) {
  /* Can prob be optimised somewhat, but with loss of any remaining clarity. */
  AliasTo *ans, *atp;
  char *t, next;

  ans = (AliasTo *)xmalloc(sizeof(AliasTo));

  if (headcasematch(s, "PLAIN:")) { 
    ans->string = (char *)xmalloc(strlen(s) - 5);
    strcpy(ans->string, s + 6);
    ans->after = -1;
    ans->next = NULL;
    return(ans);
  }

  /* else the normal case */
  atp = ans;
  while (TRUE) {
    t = strpbrk(s, "*$");
    while (t != NULL && *t == '$' &&
	   (*(t + 1) < '1' || *(t + 1) > '9'))  /* ignore ${non-digit} */
      t = strpbrk(t + 2, "*$");
    if (t == NULL)
      t = strchr(s, '\0');
    atp->string = (char *)xmalloc(t - s + 1);
    memcpy(atp->string, s, t - s);
    atp->string[t - s] = '\0';
    for (s = atp->string; (s = strstr(s, "$$")) != NULL; s++)
      memmove(s + 1, s + 2, strlen(s + 1));    /* "$$"->"$"   (reuse s) */
    if (*t == '\0')
      atp->after = -1;
    else {
      if (*t == '*')
	next = '1';
      else  /* *t == '$' */
	next = *(++t);
      atp->after = 2 * ((int)next - (int)(is_regex?'0':'1'));
      /* internal index of *'s is from 0; of regex brackets is from 1 */
    }
    if (*t == '\0' || *(t + 1) == '\0') {
      atp->next = NULL;
      return(ans);
    }
    else {
      atp->next = (AliasTo *)xmalloc(sizeof(AliasTo));
      TO_NEXT(atp);
    }
    s = t + 1;  /* set s to rest of original string */
  }
}

void configalias(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  Alias **alias = (Alias **)opt;
  Alias *ap;
  unsigned int leftstars;
  int maxrightstar;
  char starchar;
  logical is_regex = FALSE;
  pcre *pattern = NULL;
  char *errstr;
  int erroffset;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc == 1) {
    if (strcaseeq(arg1, "none"))
      *alias = NULL;
    else
      shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 2)
    longwarn(cmd, arg1, arg2, rc);

  if (headcasematch(arg1, "REGEXP:") || headcasematch(arg1, "REGEXPI:")) {
    if ((pattern =
	 pcre_compile(arg1 + 7 + (arg1[6] != ':'),
		      PCRE_DOTALL | ((arg1[6] == ':')?0:PCRE_CASELESS),
		      (const char **)(&errstr), &erroffset, NULL)) == NULL) {
      badwarn(cmd, FALSE, arg1, arg2, rc);
      warn('C', CONTINUATION, "  (%s in regular expression)", errstr);
      return;
    }
    starchar = '(';
    pcre_fullinfo(pattern, NULL, PCRE_INFO_CAPTURECOUNT, (void *)&leftstars);
    is_regex = TRUE;
  }
  else {
    starchar = '*';
    leftstars = chrdistn(arg1, starchar);
  }
  if (leftstars >= 2 && strchr(arg2, '*') != NULL &&
      !headcasematch(arg2, "PLAIN:")) {
    badwarn(cmd, FALSE, arg1, arg2, rc);
    warn('C', CONTINUATION,
	 "  (Can't have * on RHS with two %c's on LHS: use $1, $2 etc.)",
	 starchar);
    return;
  }
  if ((maxrightstar = aliastocount(arg2)) > (int)leftstars) {
    badwarn(cmd, FALSE, arg1, arg2, rc);
    warn('C', CONTINUATION, "  (More substitutions on RHS than %c's on LHS)",
	 starchar);
    return;
  }

  /* add new one to front for speed; swap round in reversealias() later */
  ap = (Alias *)xmalloc(sizeof(Alias));
  COPYSTR(ap->from, arg1);   /* save string even for regex: might l.c. */
  ap->to = configaliasto(arg2, is_regex);
  ap->isregex = is_regex;
  if (is_regex)
    ap->pattern = pattern;
  ap->next = *alias;
  *alias = ap;
}

void configstrpair(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  Strpairlist **pair = (Strpairlist **)opt;
  Strpairlist *ap;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc == 1) {
    if (strcaseeq(arg1, "none"))
      *pair = NULL;
    else
      shortwarn(cmd, arg1, rc);
    return;
  }
  if (chrn(arg1, '*') >= 2 && strchr(arg2, '*') != NULL) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  if (rc > 2)
    longwarn(cmd, arg1, arg2, rc);

  if (*pair == NULL) {
    ap = (Strpairlist *)xmalloc(sizeof(Strpairlist));
    *pair = ap;
  }
  else {
    for (ap = *pair; ap->next != NULL; TO_NEXT(ap))
      ;  /* find end of list */
    ap->next = (Strpairlist *)xmalloc(sizeof(Strpairlist));
    TO_NEXT(ap);
  }
  COPYSTR(ap->name, arg1);
  COPYSTR(ap->data, arg2);
  ap->next = NULL;
}

void configstrpairlist(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  char *t;

  if (arg1 == NULL) {
    configstrpair(opt, cmd, arg1, arg2, rc); /* will warn about args if nec. */
    return;
  }
  if ((t = strtok(arg1, ",")) == NULL) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  for ( ; t != NULL; t = strtok((char *)NULL, ","))
    configstrpair(opt, cmd, t, arg2, rc);
  return;
}

void configstrpair2list(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  /* comma-separated list in arg2 instead of arg1 */
  char *t;

  if (arg2 == NULL) {
    configstrpair(opt, cmd, arg1, arg2, rc); /* will warn about args if nec. */
    return;
  }
  if ((t = strtok(arg2, ",")) == NULL) {
    badwarn(cmd, TRUE, arg1, arg2, rc);
    return;
  }
  for ( ; t != NULL; t = strtok((char *)NULL, ","))
    configstrpair(opt, cmd, arg1, t, rc);
  return;
}

void configdomlevel(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  Strpairlist ***levels = (Strpairlist ***)opt;
  unsigned int c;

  c = 26 * ((int)(*arg1 - 'a'));
  if (*arg1 != '\0')
    c += (int)(*(arg1 + 1) - 'a');
  if (c >= DOMLEVEL_NUMBER)  /* this shouldn't happen */
    c = DOMLEVEL_NUMBER - 1;
  configstrpair((void *)(&((*levels)[c])), cmd, arg1, arg2, rc);
}

void configinex(void *opt, char *cmd, char *arg1, char *arg2, int rc,
		logical in, logical omitinitdot, logical omittrailslash) {
  Include **include = (Include **)opt;
  Include *ip;
  char *errstr;
  int erroffset;
  char *t;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (strchr(arg1, ',') != NULL && !headcasematch(arg1, "REGEXP:") &&
      !headcasematch(arg1, "REGEXPI:")) {
    /* This still allows a REGEXP in a comma-separated list if it's not the
       first item and doesn't contain a comma. The docs don't admit this. :) */
    if ((t = strtok(arg1, ",")) == NULL) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      return;
    }
    for ( ; t != NULL; t = strtok((char *)NULL, ","))
      configinex(opt, cmd, t, NULL, -1, in, omitinitdot, omittrailslash);
    return;
  }

  /* NB Put new include on front of list (so will test it earlier) */
  ip = *include;
  *include = (Include *)xmalloc(sizeof(Include));
  if (omitinitdot && arg1[0] == '.')
    arg1++;
  if (omittrailslash && !IS_EMPTY_STRING(arg1) &&
      arg1[strlen(arg1) - 1] == '/')
    arg1[strlen(arg1) - 1] = '\0';
  COPYSTR((*include)->name, arg1);
  (*include)->next = ip;   /* save name even for regex: might l.c. */
  if (headcasematch(arg1, "REGEXP:") || headcasematch(arg1, "REGEXPI:")) {
    if (((*include)->pattern =
	 pcre_compile(arg1 + 7 + (arg1[6] != ':'), 
		      PCRE_DOTALL | ((arg1[6] == ':')?0:PCRE_CASELESS),
		      (const char **)(&errstr), &erroffset, NULL)) == NULL) {
      *include = ip;
      badwarn(cmd, FALSE, arg1, arg2, rc);
      warn('C', CONTINUATION, "  (%s in regular expression)", errstr);
      return;
    }
    (*include)->type = in?(REGEX_INC):(REGEX_EXC);
  }
  else {
    if (strcaseeq((*include)->name, "pages"))
      strcpy((*include)->name, "pages");
    (*include)->type = in?(NORMAL_INC):(NORMAL_EXC);
  }
}

void configinc(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  configinex(opt, cmd, arg1, arg2, rc, TRUE, FALSE, FALSE);
}

void configexc(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  configinex(opt, cmd, arg1, arg2, rc, FALSE, FALSE, FALSE);
}

/* same, omitting initial dot */
void configincd(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
    configinex(opt, cmd, arg1, arg2, rc, TRUE, TRUE, FALSE);
}

void configexcd(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
    configinex(opt, cmd, arg1, arg2, rc, FALSE, TRUE, FALSE);
}

/* same, omitting trailing slash */
void configincs(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  configinex(opt, cmd, arg1, arg2, rc, TRUE, FALSE, TRUE);
}

void configexcs(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  configinex(opt, cmd, arg1, arg2, rc, FALSE, FALSE, TRUE);
}

/* Try and interpret a string representing a range of IP addresses or a subnet
   mask. Return whether successful. cf matchiprange() in utils.c.
*/
logical parseiprange(char *arg1, unsigned long *minaddr,
		     unsigned long *maxaddr) {
  unsigned long addr = 0, mask;
  int n1, n2, octet, i;
  char *arg, *tok, *sep, *c;

  /* We only interpret ranges and subnet masks here, not single addresses or
     wildcards, for example. Everything else is rejected so that it gets
     submitted to normal string matching, which is more efficient.
  */
  if (!ISDIGIT(*arg1) || (strchr(arg1, '-') == NULL &&
			  strchr(arg1, '/') == NULL))
    return FALSE;

  /* We can't use strtok here because it's already used in an outer loop in
     confighostinex(), so we do it by hand. We first copy arg1 into arg so that
     we don't have to worry about restoring arg1 afterwards.
  */
  COPYSTR(arg, arg1);
  for (octet = 0, tok = arg; octet < 4 && tok != NULL; octet++, tok = sep) {

    if ((sep = strchr(tok, '.')) != NULL) {
      *sep = '\0';
      sep++;  /* sep is now NULL if no more tokens, else start of next token */
    }

    /* Case 1: 131.111-114 or 131.111-114.* */
    if ((c = strchr(tok, '-')) != NULL) {
      *c = 0;
      n1 = atoi255(tok);
      n2 = atoi255(c + 1);
      if (n1 < 0 || n2 <= n1)  /* includes n2 < 0 */
	return FALSE;
      /* Check just dots and stars left in the string. We don't bother checking
	 for too many dots though. */
      for (c = sep; c != NULL && *c != '\0'; c++) {
	if (*c != '.' && *c != '*')
	  return FALSE;
      }
      addr <<= 8 * (4 - octet);
      *minaddr = addr + (n1 << 8 * (3 - octet));
      *maxaddr = addr + (((n2 + 1) << 8 * (3 - octet)) - 1);
      /* brackets essential to avoid underflow or overflow */
      return TRUE;
    }

    /* Case 2: 131.111.20.18/24 */
    if ((c = strchr(tok, '/')) != NULL) {
      if (octet != 3)  /* slash only valid after last octet */
	return FALSE;
      if (sep != NULL && *sep != '\0')  /* check end of string */
	return FALSE;
      *c = 0;
      n1 = atoi255(tok);
      n2 = atoi255(c + 1);
      if (n1 < 0 || n2 < 1 || n2 > 32)
	return FALSE;
      addr <<= 8;
      addr += n1;
      /* calculate subnet mask */
      mask = 0;
      for (i = 0; i < 32; i++) {
	mask <<= 1;
	mask += (i < n2);
      }
      addr &= mask;
      *minaddr = addr;
      *maxaddr = addr + ((1 << (32 - n2)) - 1);
      /* brackets essential to avoid underflow or overflow */
      return TRUE;
    }

    /* Case 3: The normal case: just the next number */
    n1 = atoi255(tok);
    if (n1 < 0)
      return FALSE;
    addr <<= 8;
    addr += n1;

  }  /* for octet */

  /* We can only get here if we've read four octets but still not encountered
     - or / . In that case, the string is corrupt. */
  return FALSE;
}

/* For hosts, use confighostinex(). */
/* First try parsing as a contiguous range of IP addresses, using
   parseiprange() above. If that fails, pass to configinex for normal
   processing. */
void confighostinex(void *opt, char *cmd, char *arg1, char *arg2, int rc,
		    logical in) {
  Include **include = (Include **)opt;
  Include *ip;
  unsigned long minaddr, maxaddr;
  char *c;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (strchr(arg1, ',') != NULL) {
    if ((c = strtok(arg1, ",")) == NULL) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      return;
    }
    for ( ; c != NULL; c = strtok((char *)NULL, ","))
      confighostinex(opt, cmd, c, NULL, 1, in);
    return;
  }

  /* First try parsing the argument as a range of IP addresses. */
  if (parseiprange(arg1, &minaddr, &maxaddr)) {
    ip = *include;
    *include = (Include *)xmalloc(sizeof(Include));
    COPYSTR((*include)->name, arg1);
    (*include)->minaddr = minaddr;
    (*include)->maxaddr = maxaddr;
    (*include)->next = ip;
    (*include)->type = in?(IPADDR_INC):(IPADDR_EXC);
  }
  /* If that fails, parse to the regular string-like processing. */
  else
    configinex(opt, cmd, arg1, arg2, rc, in, FALSE, FALSE);
}

void confighostinc(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  confighostinex(opt, cmd, arg1, arg2, rc, TRUE);
}

void confighostexc(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  confighostinex(opt, cmd, arg1, arg2, rc, FALSE);
}

/* for status codes */
void configscinex(void *opt, char *cmd, char *arg1, char *arg2, int rc,
		  logical in) {
  choice *code2type = (choice *)opt;
  unsigned int bottom, top, i;
  char *c, *d;
  logical f = FALSE;

  if (rc == 0) {
    shortwarn(cmd, arg1, rc);
    return;
  }
  if (rc > 1)
    longwarn(cmd, arg1, arg2, rc);

  if (strchr(arg1, ',') != NULL) {
    if ((c = strtok(arg1, ",")) == NULL) {
      badwarn(cmd, TRUE, arg1, arg2, rc);
      return;
    }
    for ( ; c != NULL; c = strtok((char *)NULL, ","))
      configscinex(opt, cmd, c, NULL, 1, in);
    return;
  }

  if (STREQ(arg1, "*")) {
    bottom = MIN_SC;
    top = SC_NUMBER - 1;
  }
  else {
    if (*arg1 == '-') {
      bottom = MIN_SC;
      c = arg1 + 1;
      if (!ISDIGIT(*c)) {
	badwarn(cmd, TRUE, arg1, arg2, rc);
	return;
      }
    }
    else {
      bottom = (unsigned int)strtoul(arg1, &c, 10);
      if (*c == '-') {
	c++;
	f = TRUE;
      }
      if (bottom < MIN_SC || bottom >= SC_NUMBER ||
	  (*c != '\0' && !ISDIGIT(*c))) {
	badwarn(cmd, TRUE, arg1, arg2, rc);
	return;
      }
    }
    if (*c == '\0') {
      if (f)
	top = SC_NUMBER - 1;
      else
	top = bottom;
    }
    else {
      top = (unsigned int)strtoul(c, &d, 10);
      if (top < bottom || top >= SC_NUMBER || *d != '\0') {
	badwarn(cmd, TRUE, arg1, arg2, rc);
	return;
      }
    }
  }

  if (code2type[0] == UNSET)
    code2type[0] = in?SUCCESS:UNWANTED; /* mark first in/ex: see finalinit() */
  for (i = bottom; i <= top; i++)
    code2type[i] = in?SUCCESS:UNWANTED; /* SUCCESS set more carefully there */
}

void configscinc(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  configscinex(opt, cmd, arg1, arg2, rc, TRUE);
}

void configscexc(void *opt, char *cmd, char *arg1, char *arg2, int rc) {
  configscinex(opt, cmd, arg1, arg2, rc, FALSE);
}
