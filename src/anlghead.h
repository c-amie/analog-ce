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

/*** anlghead.h: first header file ***/

/*** This file contains the user-definable options which you are most likely to
 *** want to change. Edit any of the #define lines according to your desires.
 *** However, all of them except MANDATORYCONFIGFILE can be changed at run
 *** time as well. ***/ 

/*** These settings are overridden by any in your configuration file. ***/

/*** DON'T REMOVE THE #'S -- THEY'RE NOT COMMENTS! ***/

#ifndef ANLGHEAD_H
#define ANLGHEAD_H

#ifndef HOSTNAME
#define HOSTNAME "[my organisation]"
/* The name of your organisation or web server. This is used for printing
   at the top of the output. */
#endif

#ifndef HOSTURL
#define HOSTURL "none"
/* The URL of your host's home page, for linking to at the top of the
   output; use "none" for no linking. */
#endif

#ifndef XMLDTD
#define XMLDTD "http://timian.jessen.ch/dtd/analog-data.dtd"
/* The URL of your host's home page, for linking to at the top of the
   output; use "none" for no linking. */
#endif

#ifndef ANONYMIZERURL
#define ANONYMIZERURL "none"
/* Optional URL to prefix on the site and URL reports for link anonymity
   ; use "none" to disable anonymization. */
#endif

#ifndef LOGFILE
#define LOGFILE "logfile.log"
/* The name of the default logfile. Can be a list, separated by commas
   (but no spaces), and can include wild cards. */
#endif

#ifndef OUTFILE
#define OUTFILE "stdout"
/* The default file for output. Use "stdout" for stdout. */
#endif

#ifndef IMAGEDIR
#define IMAGEDIR "images/"
/* URL of the directory where the images for the graphical reports live.
   The URL can be absolute, or relative to the output page: e.g., just the
   empty string "" for the same directory as the output page. */
#endif

#ifndef LOGO
#define LOGO "analogo"
/* the file where the analog logo lives: within the IMAGEDIR unless it
   starts with / or contains :// . You can put another logo in here (your
   organisation's logo, for example -- but if you do, change the LOGOURL
   below too). Use "none" for no logo. */
#endif

#ifndef LOGOURL
#define LOGOURL "https://www.c-amie.co.uk/software/analog/"
/* Where the logo links to, in XHTML output. */
#endif

#ifndef STYLESHEET
#define STYLESHEET "none"
/* The URL of your style sheet. Use "none" for no style sheet. */
#endif

#ifndef DEFAULTCONFIGFILE
#define DEFAULTCONFIGFILE "analog.cfg"
/* The name of the default configuration file. */
#endif

#ifndef MANDATORYCONFIGFILE
#define MANDATORYCONFIGFILE "none"
/* A mandatory configuration file; read after all other arguments so
   overrides everything. Program exits if it is not found. Use "none"
   for none. */
#endif

/* Finally, some directories where particularly files are looked for (you
   MUST include the trailing slash in the directory name!). You can also use
   "" to mean "the current working directory at the time you run the program".
   *
   If any of these directories is defined to be NULL, analog will attempt to
   deduce the directory of the analog binary from the command name you used,
   and use that directory; or the current directory if it couldn't deduce a
   directory. This may or may not work, so it's better to specify a location
   explicitly here if you know where the files will be kept. */
#ifndef LANGDIR
#define LANGDIR NULL
#endif
/* Directory where the language files live. Actually, if this one is defined
   to be NULL, they will be looked for inside the "lang" subdirectory of the
   directory containing the analog binary. */
#ifndef CONFIGDIR
#define CONFIGDIR NULL
#endif
/* Directory containing configuration files. */
#ifndef LOGSDIR
#define LOGSDIR NULL
#endif
/* Directory containing logfiles. */
#ifndef CACHEDIR
#define CACHEDIR NULL
#endif
/* Directory containing cache files. */
#ifndef OUTDIR
#define OUTDIR NULL
#endif
/* Directory for writing output files: must already exist. */
#ifndef HEADERDIR
#define HEADERDIR NULL
#endif
/* Directory containing header and footer files. */
#ifndef DNSDIR
#define DNSDIR NULL
#endif
/* Directory containing DNS files. */
#ifndef LOCKDIR
#define LOCKDIR NULL
#endif
/* Directory containing the DNS lock file. */
#ifndef ERRDIR
#define ERRDIR NULL
#endif
/* Directory containing the ERRFILE. */

#include "anlghea2.h"
/* You can edit anlghea2.h as well. But the options in there are ones which
   you're unlikely to want to change now. */

#endif  /* ANLGHEAD_H */
