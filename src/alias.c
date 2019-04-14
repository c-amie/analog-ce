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

/*** alias.c; functions to cope with aliasing. ***/

#include "anlghea3.h"

/* Throughout this file, to avoid a buffer overflow, we must be careful when we
   do any operation which can increase the length of the name. Such operations
   are thankfully rare, except for user aliases in wildalias().
   *
   To ensure this, most functions assume that char *name can hold an aliased
   name of length at least BLOCKSIZE - 1. (In practice we always copy the name
   into workspace and call the aliasing function with argument workspace.) */

choice do_alias(char *name, Memman *mp, Alias *aliashead, char *dirsuffix,
		unsigned int dirsufflength, logical usercase_insensitive,
		unsigned char convfloor, logical multibyte, choice type) {
  /* return ERR for corrupt/unwanted, else whether any alias done */
  extern char *workspace;
  logical rc = TRUE;
  size_t len;

  strcpy(workspace, name);

  if (type == ITEM_FILE)
    rc = do_aliasr(workspace, dirsuffix, dirsufflength);
  else if (type == ITEM_HOST)
    rc = do_aliasS(workspace);
  else if (type == ITEM_REFERRER)
    rc = do_aliasf(workspace);
  else if (type == ITEM_BROWSER)
    rc = do_aliasB(workspace);
  else if (type == ITEM_VHOST)
    rc = do_aliasv(workspace);
  else if (type == ITEM_USER)
    rc = do_aliasu(workspace, usercase_insensitive);
  else if (type == REP_SEARCHREP || type == REP_INTSEARCHREP)
    /* nasty mix of ITEM_ & REP_, but it works because they are all distinct */
    rc = do_aliasN(workspace, convfloor, multibyte);
  else if (type == REP_SEARCHSUM || type == REP_INTSEARCHSUM)
    rc = do_aliasn(workspace, convfloor, multibyte);
  else if (type == REP_BROWSUM)
    rc = do_aliasb(workspace);
  if (rc == FALSE)
    return(ERR);

  (void)do_aliasx(workspace, aliashead);

  if (!STREQ(workspace, name)) {
    len = strlen(workspace);
    memcpy((void *)submalloc(mp, len + 1), (void *)workspace, len);
    *((char *)(mp->curr_pos) + len) = '\0';
    return(TRUE);
  }
  else
    return(FALSE);
}

/* User-specified aliases.
 *
 * Name is changed if necessary. Name MUST be at least BLOCKSIZE long.
 *
 * The function returns whether any alias matched.
 */
logical do_aliasx(char *name, Alias *aliashead) {
  int pmatch[PMATCH_SIZE];
  Alias *ap;
  logical done;

  for (ap = aliashead, done = FALSE; ap != NULL && !done; TO_NEXT(ap)) {
    if (matchq(name,
	       (ap->isregex)?((void *)(ap->pattern)):((void *)(ap->from)),
	       ap->isregex, pmatch)) {
      done = TRUE;
      wildalias(name, ap->to, pmatch);
    }
  }
  return(done);
}

logical do_aliasr(char *name,  char *dirsuffix, unsigned int dirsufflength) {
#ifdef EBCDIC
  extern unsigned char os_toebcdic[];
#endif
  extern logical convertchar[256];
  char *c, *d, *e1, *e2;
  unsigned int tempint = 0;

  /* NB Case insensitivity has been done (if necessary) in prealias() */

  /* Zerothly, strip off #'s. These shouldn't get in the request, but do
     for some broken agents (particularly spiders). */
  if ((c = strchr(name, '#')) != NULL)
    *c = '\0';

  /* Halfthly, strip from semicolon to the end of the URL stem.
     (e.g. jsessionid). */

  if ((c = strchr(name, ';')) != NULL) {
    d = strchr(name, '?');
    if (d == NULL)
      d = strchr(c + 1, '\0');
    if (c < d)
      memmove(c, d, strlen(d) + 1);
  }

  /* First, change %7E to ~, etc. */

  c = name;
  while((c = strchr(c, '%')) != NULL) {
    sscanf(c + 1, "%2x", &tempint);
    if (tempint <= 0xFF && convertchar[tempint]) { /* could have been %-1 */
#ifdef EBCDIC
      tempint = os_toebcdic[tempint];
#endif
      *c = (char)tempint;
      memmove((void *)(c + 1), (void *)(c + 3), strlen(c + 3) + 1);
    }
    c++;
  }

  /* Secondly, if it begins with a colon, assume it's a Mac file and convert
     colons to slashes. */

  c = name;
  if (*c == ':') {
    for ( ; *c != '\0'; c++) {
      if (*c == ':')
	*c = '/';
    }
  }

  /* Otherwise, change backslashes to forward slashes. This is not strictly
     correct, but it's usually the right thing to do (to make the Directory
     Report correct). */

  for ( ; *c != '\0' && *c != '?'; c++) {
    if (*c == '\\')
      *c = '/';
  }

  /* Thirdly, if it ends with DIRSUFFIX (typically index.html), strip it */

  e1 = strchr(name, '\0');
  if ((e2 = strchr(name, '?')) != NULL)
    *e2 = '\0';
  else
    e2 = e1;
  if (dirsufflength > 0 && dirsufflength < (unsigned int)(e2 - name) &&
      *(e2 - dirsufflength - 1) == '/' &&
      STREQ(e2 - dirsufflength, dirsuffix)) {
    memcpy((void *)(e2 - dirsufflength), (void *)e2, (size_t)(e1 - e2) + 1);
    if (e2 != e1)
      e2 -= dirsufflength;
  }
  if (e2 != e1)
    *e2 = '?';

  /* Fourthly, // -> /  ;  /./ -> /  ;  /spam/../ -> /  */
  /* Used to use 3 strstr()s. More intuitive but MUCH slower. */

  c = name;
  while ((c = strchr(c, '/')) != NULL && c < e2) {
    if (*(c + 1) == '/') {
      if (c != name && *(c - 1) == ':')
	c++;   /* Don't translate http:// ; just skip to next /  */
      else
	memmove((void *)(c + 1), (void *)(c + 2), strlen(c + 2) + 1);
    }
    else if (*(c + 1) == '.') {
      if (*(c + 2) == '/')
	memmove((void *)(c + 1), (void *)(c + 3), strlen(c + 3) + 1);
      else if (*(c + 2) == '.' && *(c + 3) == '/') {
	d = c + 4;
	/* go back to prev slash (but not past // or start of name) */
        if (c != name && *(c - 1) != '/') {
	  while (*(--c) != '/' && c != name)
	    ;
	}
	memmove((void *)(c + 1), (void *)d, strlen(d) + 1);
	if (c == name)
	  *c = '/';
      }
      else
	c++;
    }
    else
      c++;
  }

  return(TRUE);
}

logical do_aliasS(char *name) {
#ifndef NODNS
  extern choice dnslevel;
#endif

  char *c;
  int len;

  len = (int)strlen(name) - 1;  /* NB offset by 1 */

  /* DNS lookup */

#ifndef NODNS
  if (dnslevel != DNS_NONE && ISDIGIT(name[len])) {
    do_dns(name, NULL, dnslevel);
    len = (int)strlen(name) - 1;
  }
#endif

  /* remove trailing dot */

  if (name[len] == '.')
    name[len] = '\0';

  /* convert to lower case */

  for (c = name + len; c >= name; c--)
    *c = TOLOWER(*c);

  return(TRUE);
}

logical do_aliasf(char *name) {
  /* name must be at least BLOCKSIZE long */
#ifdef EBCDIC
  extern unsigned char os_toebcdic[];
#endif
  extern logical convertchar[256];
  int defaultport = UNSET;
  char *c, *d;
  unsigned int tempint = 0;

  /* First, strip off #'s */
  if ((c = strchr(name, '#')) != NULL)
    *c = '\0';

  /* Strip from semicolon to the end of the URL stem. (e.g. jsessionid). */

  if ((c = strchr(name, ';')) != NULL) {
    d = strchr(name, '?');
    if (d == NULL)
      d = strchr(c + 1, '\0');
    if (c < d)
      memmove(c, d, strlen(d) + 1);
  }

  /* Next, change %7E to ~, etc. */
  c = name;
  while((c = strchr(c, '%')) != NULL) {
    sscanf(c + 1, "%2x", &tempint);
    if (tempint <= 0xFF && convertchar[tempint]) {
#ifdef EBCDIC
      tempint = os_toebcdic[tempint];
#endif
      *c = (char)tempint;
      memmove((void *)(c + 1), (void *)(c + 3), strlen(c + 3) + 1);
    }
    c++;
  }

  /* Coerce method to lower case */
  for (c = name; *c != ':' && *c != '\0'; c++)
    *c = TOLOWER(*c);
  if (*c != ':')
    return(FALSE);

  /* find out what sort of URL it is */
  *c = '\0';  /* *c was already set to the first colon */

  if (STREQ(name, "http"))
    defaultport = 80;
  else if (STREQ(name, "ftp"))
    defaultport = 21;
  else if (STREQ(name, "file"))
    return(FALSE);  /* don't count file: URL's */
  else if (STREQ(name, "https"))
    defaultport = 443;
  else if (STREQ(name, "news"))
    defaultport = 0;
  /* The rest probably never happen but they don't slow it down either because
     execution almost never gets to them. */
  else if (STREQ(name, "gopher"))
    defaultport = 70;
  else if (STREQ(name, "telnet"))
    defaultport = 23;
  else if (STREQ(name, "wais"))
    defaultport = 210;
  else if (STREQ(name, "nntp"))
    defaultport = 119;
  else if (STREQ(name, "prospero"))
    defaultport = 1525;
  else if (STREQ(name, "mailto"))
    defaultport = 0;
  *c = ':';

  /* Change backslash -> slash for the reason given in do_aliasr() */

  c++;
  for (d = c; *d != '\0' && *d != '?'; d++) {
    if (*d == '\\')
      *d = '/';
  }

  /* If "news:" or "mailto:" or unknown, that's all we do.
     Otherwise, check it has the // next and coerce hostname to lower case */

  if (defaultport > 0) {
    if (*c != '/' || *(c + 1) != '/')
      return(FALSE);
    else for (c += 2; *c != '/' && *c != ':' && *c != '\0' && *c != '?'; c++)
      *c = TOLOWER(*c);

    /* strip trailing .'s from hostname */

    for (d = c - 1; *d == '.'; d--)
      ;  /* run back to before any dots */
    if (d != c - 1) {
      memmove((void *)(d + 1), (void *)c, strlen(c) + 1);
      c = d + 1;
    }

    /* strip leading 0s from port numbers and cross out default port numbers */

    if (*c == ':') {
      for (d = c + 1; *d == '0'; d++)
	;  /* run forward to after any 0's */
      if (d != c + 1)
	memmove((void *)(c + 1), (void *)d, strlen(d) + 1);

      if (defaultport == atoi(c + 1)) {
	/* run forward to after all digits */
	for (d = c + 2; ISDIGIT(*d); d++)
	  ;
	memmove((void *)c, (void *)d, strlen(d) + 1);
      }
    }
    
    /* We don't want to change /./ -> / etc. even in http protocol, because we
       don't want to make assumptions about other people's file systems. */
    /* So finally, trailing slash if no directory name given */

    for ( ; *c != '/' && *c != '\0' && *c != '?'; c++)
      ;   /* run to next slash or end of string */
    if (*c != '/' && strlen(name) <= BLOCKSIZE - 2) {
      memmove((void *)(c + 1), (void *)c, strlen(c) + 1);
      *c = '/';
    }
  }

  return(TRUE);
}

logical do_aliasB(char *name) {
  /* NB aliasB is at parse time, aliasb for compiling browsum */
  char *c;

  /* cut (illegal) "via"s (e.g., via certain proxy or cache) */
  if ((c = strstr(name, " via ")) != NULL)
    *c = '\0';

  /* cut trailing spaces */
  for (c = name + strlen(name) - 1; *c == ' ' && c > name; c--)
    *c = '\0';
  if (c == name)
    return(FALSE);

  return(TRUE);
}

logical do_aliasu(char *name, logical case_insensitive) {

  if (case_insensitive) {
    for ( ; *name != '\0'; name++) {
      /* Some usernames have backslashes in which need to change to slashes */
      if (*name == '\\')
	*name = '/';
      else      /* convert to lower case */
	*name = TOLOWER(*name);
    }
  }
  else for ( ; *name != '\0'; name++) {  /* separate loop for speed */
    if (*name == '\\')
      *name = '/';
  }

  return(TRUE);
}

logical do_aliasv(char *name) {

  /* just convert to lower case */
  for ( ; *name != '\0'; name++)
    *name = TOLOWER(*name);

  return(TRUE);
}

logical do_aliasN(char *name, unsigned char convfloor, logical multibyte) {

  char *c, *d;

  /* lower case and convert %nm's by calling do_aliasn() */
  do_aliasn(name, convfloor, multibyte);

  /* change unwanted characters to spaces */
  /* 'unwanted' should be coordinated with nnextname() */
  for (c = name; *c != '\0'; c++) {
    if (*c == '-') {  /* remove except at beginning of word */
      if ((c != name && *(c - 1) != ' ') ||
	  *(c + 1) == '+' || *(c + 1) == '-' || *(c + 1) == '"' ||
	  *(c + 1) == ',' || *(c + 1) == '(' ||
	  *(c + 1) == ')' || *(c + 1) == '.' || *(c + 1) == '\0' ||
	  ISSPACE(*(c + 1)))
	*c = ' ';
    }
    else if (*c == '.') {  /* remove except between letters/numbers */
      if (c == name || !ISALNUM(*(c - 1)) || !ISALNUM(*(c + 1)))
	*c = ' ';
    }
    else if (*c == '+' || *c == '"' || *c == ',' ||
	     *c == '(' || *c == ')' || ISSPACE(*c))
      *c = ' ';
  }

  /* remove initial spaces */
  if (ISSPACE(*name)) {
    for (c = name + 1; ISSPACE(*c); c++)
      ;
    memmove(name, c, strlen(c) + 1);
  }

  /* remove trailing spaces */
  if (*name == '\0')
    return(FALSE);
  for (c = strchr(name, '\0') - 1; ISSPACE(*c); c--)
    ;
  *(c + 1) = '\0';

  /* concatenate spaces */
  c = name;
  for (c = strstr(c, "  "); c != NULL; c = strstr(c + 1, "  ")) {
    for (d = c + 2; *d == ' '; d++)
      ;  /* run past contiguous spaces */
    memmove(c + 1, d, strlen(d) + 1);
  }

  return(TRUE);
}

logical do_aliasn(char *name, unsigned char convfloor, logical multibyte) {
  /* NB Most of the filtering has already been done by nnextname() */
  /* NB2 do_aliasN() calls this function, so if it's extended, we need to
     think about whether we want do_aliasN() to include the new stuff too. */
#ifdef EBCDIC
  extern unsigned char os_toebcdic[];
#endif
  unsigned int tempint = 0;
  char *c;

  /* lower case */
  if (!multibyte) {
    for (c = name; *c != '\0'; c++)
      *c = TOLOWER(*c);
  }

  /* If required, change %nm's from the top half of character set */
  /* NB convfloor is ASCII value, applied before conversion to EBCDIC */
  /* Also change %25, %26, %3D and %3F here: see convertchar[] in globals.c */
  c = name;
  while((c = strchr(c, '%')) != NULL) {
    sscanf(c + 1, "%2x", &tempint);
    if (tempint <= 0xFF && ((convfloor > 0 && tempint >= convfloor) ||
			    tempint == 0x25 || tempint == 0x26 ||
			    tempint == 0x3D || tempint == 0x3F)) {
#ifdef EBCDIC
      tempint = os_toebcdic[tempint];
#endif
      *c = (char)tempint;
      memmove((void *)(c + 1), (void *)(c + 3), strlen(c + 3) + 1);
    }
    c++;
  }

  return(TRUE);
}

logical do_aliasb(char *name) {
  /* NB aliasB is at parse time, aliasb for compiling Browser Summary. */
  /* See Bnextname() for the role of this function. */
  char *s, *t;
  logical done = FALSE;

  if (headmatch(name, "Mozilla")) {
    if (*(name + 7) == '/' && *(name + 8) == '5') {

      /* Mozilla/5 is messy. The real version number should be written at the
	 end of the brackets. Furthermore, there are (so far) three different
	 version number formats, "m18", "0.8" and "rv:0.9.1+".

	 To add to the confusion, we should spot those which are Netscape 6+
	 branded, which have the normal Mozilla version number but also the
	 brand name on the end.
	 
	 We've filtered out Mozilla/5.0 (compatible) in Bnextname() so these
	 should all be genuine Mozilla. But if we can't find a version number
	 we'll just leave it as Mozilla, not Mozilla/5.0, to avoid confusing
	 the two different numbering schemes. */

      if ((t = strstr(name, ") Gecko/")) != NULL) {
	/* There should only be one ')', but let's be cautious. */

	/* First try pulling off Netscape version number. There are two cases:
	   Netscape/7.x, or Netscape6/6.x which should be translated to just
	   Netscape/6.x. */
	if (headmatch(t + 17 /* after "Gecko/YYYYMMDD" */, "Netscape")) {
	  memmove(name, t + 17, 8);
	  if (*(t + 25) == '6' && *(t + 26) == '/')
	    memmove(name + 8, t + 26, strlen(t + 26) + 1);
	  else
	    memmove(name + 8, t + 25, strlen(t + 25) + 1);
	  done = TRUE;
	}

	/* Otherwise look for normal Mozilla version number */
	else {
	  for (s = t - 1; *s != ' ' && s != name; s--)
	    ;  /* run back to space before last element inside brackets */
	  if (s != name && *(s - 1) == ';') {
	    s++;
	    if (headmatch(s, "rv:"))
	      s += 3;
	    if (ISDIGIT(*s) || (*s == 'm' && ISDIGIT(*(s + 1)))) {/* Found it*/
	      if (*s == 'm')
		*s = 'M';
	      memcpy(name + 8, s, t - s);
	      *(name + 8 + (t - s)) = '\0';
	      done = TRUE;
	    }
	  }
	}
      }
      if (!done)
	*(name + 7) = '\0';
      return(TRUE);
    }

    /* All other Mozilla's are reckoned to be Netscape */
    memmove(name + 8, name + 7, strlen(name + 7) + 1);
    strncpy(name, "Netscape", 8);
  }

  /* Convert "Galeon; n.m" to "Galeon/n.m"; n a digit tested in Bnextname() */
  else if (headmatch(name, "Galeon; ")) {
    *(name + 6) = '/';
    memmove(name + 7, name + 8, strlen(name + 8) + 1);
    return(TRUE);
  }

  /* In general, convert "Name n.m" to "Name/n.m" if n a digit */
  for (s = name; ISALNUM(*s); s++)
    ;
  if (*s == ' ' && ISDIGIT(*(s + 1)))
    *s = '/';
  return(TRUE);
}

#ifndef NODNS
logical dnsresolve(char *name, choice level) {
  /* name must be at least BLOCKSIZE long */
#ifndef NOALARM
  extern unsigned int dnstimeout;
  extern JMP_BUF jumpbuf;
#endif
  volatile logical done = FALSE;
#ifndef MAC
  IN_ADDR_T addr;
#ifdef VMS
  char *addrp;
#else
#ifdef AS400
  unsigned char *addrp;
#else
  const char *addrp;
#endif
#endif
  struct hostent *tempp;
#endif

  if (level < DNS_LOOKUP)
    return(FALSE);
  debug('D', "Looking up %s:", name);
#ifdef MAC
  done = IpAddr2Name(name);
  /* Doesn't currently include code to use SIGALRM */
#else
  addr = inet_addr(name);
  if (addr != INET_ADDR_ERR) {
    addrp = (char *) &addr;
#ifndef NOALARM
    if (SETJMP(jumpbuf))
      signal(SIGALRM, SIG_DFL);
    /* reset to default handling in case SIGALRM is raised exogenously */
    else {
      signal(SIGALRM, sighandler);
      alarm(dnstimeout);
#endif
      tempp = gethostbyaddr(addrp, sizeof(IN_ADDR_T), AF_INET);
#ifndef NOALARM
      alarm(0);
      signal(SIGALRM, SIG_DFL);
#endif
      if (tempp != NULL && tempp->h_name != NULL && tempp->h_name[0] != '\0') {
	/* hostnames shouldn't be 32,000 characters long, but... */
	strncpy(name, tempp->h_name, BLOCKSIZE - 1);
	name[BLOCKSIZE - 1] = '\0';
	done = TRUE;
      }
#ifndef NOALARM
    }
#endif
  }
#endif  /* not MAC */
  if (done)
    debug('D', "  resolved to %s", name);
  else
    debug('D', "  can't resolve");
  return(done);
}

void do_dns(char *name, char *alias, choice level) {
  /* This is simplified from hashfind(). It is called in two situations:
     @ During reading of DNS file. In this case alias != NULL.
     @ During do_aliasS(). Then alias == NULL, and name can be changed. In this
     case, dnsresolve() assumes that name is BLOCKSIZE long. */
  extern Hashtable *dnstable;
  extern timecode_t starttimeuxc;
  extern FILE *dnsfilep;
  extern Memman *xmemman;

  Hashindex *lp, *lastlp;
  unsigned long magic;
  logical done = FALSE;

  if (TOO_FULL(dnstable->n, dnstable->size))
   dnstable = rehash(dnstable, NEW_SIZE(dnstable->size), NULL);
  MAGICNO(magic, name, dnstable->size);

  lp = dnstable->head[magic];
  lastlp = NULL;

  while (!done) {
    if (lp == NULL) {  /* need a new index entry */
      lp = (Hashindex *)submalloc(xmemman, sizeof(Hashindex));
      if (lastlp == NULL)
	dnstable->head[magic] = lp;
      else
	lastlp->next = lp;
      lp->name = (char *)submalloc(xmemman, strlen(name) + 1);
      strcpy(lp->name, name);   /* NB don't use lp->own */
      if (alias != NULL) {   /* initial build */
	if (STREQ(alias, "*"))
	  lp->other = NULL;
	else {
	  lp->other = submalloc(xmemman, strlen(alias) + 1);
	  strcpy((char *)(lp->other), alias);
	}
      }
      else if (dnsresolve(name, level)) {  /* potentially changes name */
	lp->other = submalloc(xmemman, strlen(name) + 1);
	strcpy((char *)(lp->other), name);
	if (dnsfilep != NULL) {
	  if (strchr(lp->name, ' ') == NULL && strchr(name, ' ') == NULL &&
	      strlen(lp->name) + strlen(name) < 240) {
	    /* otherwise will be corrupt at read time */
	    fprintf(dnsfilep, "%ld %s %s\n", starttimeuxc, lp->name, name);
	    fflush(dnsfilep);
	  }
	}
      }
      else {
	lp->other = NULL;
	if (dnsfilep != NULL) {
	  if (strchr(lp->name, ' ') == NULL && strlen(lp->name) < 240) {
	    /* otherwise will be corrupt at read time */
	    fprintf(dnsfilep, "%ld %s *\n", starttimeuxc, lp->name);
	    fflush(dnsfilep);
	  }
	}
      }
      lp->next = NULL;
      (dnstable->n)++;
      done = TRUE;
    }
    else if (STREQ(lp->name, name)) {
      if (alias != NULL) {   /* initial build: overwrite */
	if (STREQ(alias, "*"))
	  lp->other = NULL;
	else {
	  lp->other = submalloc(xmemman, strlen(alias) + 1);
	  strcpy((char *)(lp->other), alias);
	}
      }
      else if (lp->other != NULL)
	strcpy(name, (char *)(lp->other));
      done = TRUE;
    }
    else {
      lastlp = lp;
      TO_NEXT(lp);
    }
  }
}
#endif

/* Change name to its alias. Matching has already been checked.
   name MUST have length >= BLOCKSIZE. */
void wildalias(char *name, AliasTo *to, int pmatch[]) {
  char *s, *ans;
  int *pm;
  size_t len, tolen;

  len = strlen(name);
  ans = name + len;
  /* The aliased name is built AFTER the orig name. */
  for (s = ans; to != NULL; TO_NEXT(to)) {
    pm = &(pmatch[to->after]);
    tolen = strlen(to->string);
    len += tolen;
    if (to->after >= 0 && pm[0] >= 0)
      len += (size_t)(pm[1] - pm[0]);
    if (len >= BLOCKSIZE - 1) {
      ans[0] = '\0';  /* so we just return unaliased name */
      return;
    }
    strcpy(s, to->string);
    s += tolen;
    if (to->after >= 0 && pm[0] >= 0) {
      memcpy((void *)s, (void *)(name + pm[0]), (size_t)(pm[1] - pm[0]));
      s += pm[1] - pm[0];
    }
  }
  *s = '\0';
  memmove((void *)name, (void *)ans, strlen(ans) + 1);
}

/* Now preliminary file/referrer aliasing, to add prefix & query string */
choice prealias(Memman *mm, Memman *mmv, Hashentry *vhost, Memman *mmq,
		logical case_insensitive, char *prefix, size_t prefixlen,
		int pvpos, Include *argshead) {
  extern Hashentry *blank_entry;
  size_t len, len2 = 0;
  char *curr, *next, *mmqstart, *c;

  /* First, fold to lower case */
  if (case_insensitive) {
    for (c = mm->curr_pos; *c != '\0' && *c != '?'; c++)
      *c = TOLOWER(*c);
  }

  /* Then add prefix */
  if (prefix != NULL) {
    len = prefixlen;
    if (pvpos >= 0) {
      if (ENTRY_BLANK(vhost))
	return(-1); /* %v in prefix but no vhost => corrupt */
      else {
	len2 = strlen((char *)(mmv->curr_pos));
	len += len2 - 2;
      /* NB vhost may be marked for deletion (next_pos == curr_pos), but has
	 not yet overwritten. */
      }
    }
    curr = (char *)(mm->curr_pos);
    next = (char *)(mm->next_pos);
    if ((char *)(mm->block_end) - (char *)(mm->next_pos) < (ptrdiff_t)len) {
      c = (char *)submalloc(mm, len);
      mm->next_pos = (void *)((char *)(mm->curr_pos) + (next - curr));
      if ((char *)(mm->block_end) - (char *)(mm->next_pos) < (ptrdiff_t)len)
	return(-2); /* filename + prefix too long => corrupt */
    }
    memmove((void *)((char *)(mm->curr_pos) + len), (void *)curr,
	    (size_t)(next - curr));
    if (pvpos < 0)
      memmove(mm->curr_pos, (void *)prefix, prefixlen);
    else {
      memmove(mm->curr_pos, (void *)prefix, (size_t)pvpos);
      memmove((void *)((char *)(mm->curr_pos) + pvpos), mmv->curr_pos, len2);
      memmove((void *)((char *)(mm->curr_pos) + pvpos + len2),
	      (void *)(prefix + pvpos + 2), prefixlen - (size_t)pvpos - 2);
    }
    mm->next_pos = (void *)((char *)(mm->next_pos) + len);
  }  /* end prefix != NULL */

  /* Finally, add or delete query string */
  if ((char *)(mmq->next_pos) - (char *)(mmq->curr_pos) > 1 &&
      (((char *)(mmq->curr_pos))[0] != '-' ||
       ((char *)(mmq->curr_pos))[1] != '\0')) {
    /* length >= 1 and not just "-" (no need to call STREQ/strcmp) */
    if (included(mm->curr_pos, FALSE, argshead) &&
	strchr(mm->curr_pos, '?') == NULL) {
      mmqstart = (char *)(mmq->curr_pos);
      if (*mmqstart == '?') /* assume initial ? included in %q, Apache-style */
	mmqstart++;
      len = (size_t)((char *)(mmq->next_pos) - mmqstart);
      if ((char *)(mm->block_end) - (char *)(mm->next_pos) < (ptrdiff_t)len) {
	curr = (char *)(mm->curr_pos);
	next = (char *)(mm->next_pos);
	memmove(submalloc(mm, len), (void *)curr, (size_t)(next - curr));
	mm->next_pos = (void *)((char *)(mm->curr_pos) + (next - curr));
	if ((char *)(mm->block_end) - (char *)(mm->next_pos) < (ptrdiff_t)len)
	  return(-2); /* filename + query string too long => corrupt */
      }
      *((char *)(mm->next_pos) - 1) = '?';
      memmove(mm->next_pos, (void *)mmqstart, len);
      mm->next_pos = (void *)((char *)(mm->next_pos) + len);
    }
  }
  else if ((c = strchr((char *)(mm->curr_pos) + 1, '?')) != NULL) {
    *c = '\0';
    if (included(mm->curr_pos, FALSE, argshead))
      *c = '?';
    else
      mm->next_pos = c + 1;
  }
  mmq->next_pos = mmq->curr_pos;  /* always reset mmq */
  return(OK);
}

void prealiasS(Memman *mm, Memman *mms) {
  /* Choose between %S and %s tokens */
  size_t len;

  if ((char *)(mms->next_pos) - (char *)(mms->curr_pos) > 1 &&
      (((char *)(mms->curr_pos))[0] != '-' ||
       ((char *)(mms->curr_pos))[1] != '\0')) {
    /* length >= 1 and not just "-", i.e. mms exists */
    if ((char *)(mm->next_pos) - (char *)(mm->curr_pos) <= 1 ||
	(((char *)(mm->curr_pos))[0] == '-' &&
	 ((char *)(mm->curr_pos))[1] == '\0')) {
      /* mm has the opposite property: so copy mms into mm */
      len = strlen((char *)(mms->curr_pos));
      mm->next_pos = mm->curr_pos;
      memcpy(submalloc(mm, len + 1), mms->curr_pos, len + 1);
    }
  }
  mms->next_pos = mms->curr_pos;  /* in any case, reset mms */
  return;
}

void reverseonename(Hashindex *p) {
  extern Memman *xmemman;
  static char *s = NULL;
  static size_t len = 0;
  char *t1, *t2, *t3;
  size_t l, m = 0;
  logical done;

  /* reversing done in own space, so pull off const char * cases first */
  if (STREQ(p->name, LNGSTR_NODOMAIN) || STREQ(p->name, LNGSTR_UNKDOMAIN))
    return;
  l = strlen(p->name);                             /* numerical host */
  if (ISDIGIT(*(p->name + l - 1))  && ISDIGIT(*(p->name))) {
    t1 = p->name;
    t2 = t1;
    t3 = t1;
    while (*t3 == '0')
      t3++;
    for (done = FALSE; !done; t2++) {
      if (*t2 == '.') {
	ENSURE_LEN(s, len, m + (t2 - t1) + 3);
	for (l = t2 - t1; l < 3; l++)
	  *(s + (m++)) = '0';
	memcpy((void *)(s + m), (void *)t3, (size_t)(t2 - t3 + 1));
	m += t2 - t3 + 1;
	t2++;
	t1 = t2;
	t3 = t2;
	while (*t3 == '0')
	  t3++;
      }
      else if (*t2 == '\0') {
	ENSURE_LEN(s, len, m + (t2 - t1) + 3);
	for (l = t2 - t1; l < 3; l++)
	  *(s + (m++)) = '0';
	memcpy((void *)(s + m), (void *)t3, (size_t)(t2 - t3 + 1));
	m += t2 - t3 + 1;
	if (strlen(p->name) < (l = strlen(s)))
	  p->name = (char *)submalloc(xmemman, l + 1);
	memcpy((void *)(p->name), (void *)s, m);
	done = TRUE;
      }
    }
  }
  else {
    ENSURE_LEN(s, len, l + 1);
    for (t1 = p->name + l, t2 = t1, done = FALSE; !done;
	 t1--) {
      if (*t1 == '.') {
	memcpy((void *)(s + m), (void *)(t1 + 1), (size_t)(t2 - t1 - 1));
	m += t2 - t1;  /* including the dot below */
	t2 = t1;
	*(s + m - 1) = '.';
      }
      if (t1 == p->name) {
	memcpy((void *)(s + m), (void *)t1, (size_t)(t2 - t1));
	m += t2 - t1 + 1;
	*(s + m - 1) = '\0';
	memcpy((void *)(p->name), (void *)s, m);
	done = TRUE;
      }
    }
  }
}

void reversenames(Hashindex *ans) {
  Hashindex *p;
  for (p = ans; p != NULL; TO_NEXT(p))
    reverseonename(p);
}
