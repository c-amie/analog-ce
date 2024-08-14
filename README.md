# analog-ce
Analog CE

This is the official GIT repository for the Analog CE maintenance project of Stephen Turner's Analog log-file parser project.

If you found this useful, please consider donating via https://www.c-amie.co.uk/


## Binary / Executable Downloads
If you are looking for binary downloads of pre-compiled executabled for Analog CE. Please visit https://www.c-amie.co.uk/software/analog/

## Building
If you receive the error:
'''
WARNING: 'aclocal-1.16' is missing on your system.
         You should only need it if you modified 'acinclude.m4' or
         'configure.ac' or m4 files included by 'configure.ac'.
         The 'aclocal' program is part of the GNU Automake package:
         <https://www.gnu.org/software/automake>
         It also requires GNU Autoconf, GNU m4 and Perl in order to run:
         <https://www.gnu.org/software/autoconf>
         <https://www.gnu.org/software/m4/>
         <https://www.perl.org/>
'''
Download pcre2-10.44.tar.gz from https://github.com/PCRE2Project/pcre2/releases/tag/pcre2-10.44 and extract over src/pcre2. Something is being lost in the clone to/from GitHub that is causing the error. If you know what and how to solve it, please file it as an issue.
