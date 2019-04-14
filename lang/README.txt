This directory contains all language files and other translations for analog.

Notes to translators:

Files are named after country codes (not language codes); or in some cases
country code plus an additional letter indicating the character set.

There is a variety of files which can be translated, but only one is
essential, namely the language file.

Here are all the possibilities. If the country code is xy, then the language
MUST have

  xy.lng   Language file in the language's usual character set

It MAY have

  xya.lng  Language file transliterated into US-ASCII, i.e. no accents etc.

Every language is ENCOURAGED to have

  xydom.tab                Domains file in local character set 
  xydesc.txt               Report descriptions file in local character set

and if it has an ASCII language file, it SHOULD then have corresponding

  xyadom.tab, xyadesc.txt

It MAY also have

  xyform.html  Form interface
  xy.cfg       Configuration file with any locale-specific commands: for
               example LANGUAGE and TYPEOUTPUTALIAS commands
