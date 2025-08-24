# analog-ce
Analog CE

This is the official GIT repository for the Analog CE maintenance project of Stephen Turner's Analog log-file parser project.

## Project Future
Please note that I no longer have the ability to compile for Mac and have less access to dev tools than I did, so if someone else wants to take up maintenance, please get in touch.

## Binary / Executable Downloads
Pre-compiled binaries are now available here in the releases tab of GitHub.

## What is the difference between this and Stephen Turner’s version?
After Dr. Turner stopped maintaining Analog, I have stepped in to provide updates to the detection algorithm. Analog CE should be considered to be a maintenance release of Analog. The binaries are newer and contain a small number of optimisations however the fundamental functionality is identical.

## Do I need Analog CE?
Analog CE is primarily aimed at users who want to be able to detect newer operating systems and web browsers through the reporting functionality of Analog. If you don’t need this, then there is no specific need to use Analog CE unless you are:

1. Looking to use XML output formatting for which Analog CE has a number of bug fixes
2. Seeking to improve link privacy and reduce the risk of being victim to an SEO spam bot; this is especially important if your stats pages are accessible via the public Internet

## Guides
- [Quick Start Guide: Without Report Magic](https://www.c-amie.co.uk/software/analog/guides/quick-start-guide-using-analog-ce-without-report-magic/)
- [Quick Start Guide: With Report Magic on Windows](https://www.c-amie.co.uk/software/analog/guides/quick-start-guide-using-analog-ce-with-report-magic-for-windows/)
- [Using Analog CE to provide automatic statistics on a multi-site IIS server](http://www.c-amie.co.uk/qlink/?id=106)
- [Using Analog CE to generate live statistics for inclusion on web pages](http://www.c-amie.co.uk/qlink/?id=112)
- [The Search Query and Search Word reports are empty or show very little keyword content](https://www.c-amie.co.uk/technical/analogs-search-query-report-is-empty-or-sparsely-populated/)
- [Adding WordPress search to the Internal Search Query & Internal Search Word Reports](https://www.c-amie.co.uk/technical/how-to-enable-the-analog-internal-search-query-report-for-wordpress/)
- [Creating a Link Anonymiser Service for Analog CE’s ANONYMIZERURL setting](https://www.c-amie.co.uk/technical/creating-a-link-anonymiser-service-for-analog-ces-anonymizerurl-setting/)

## Building
If you receive the error:
```
WARNING: 'aclocal-1.16' is missing on your system.
         You should only need it if you modified 'acinclude.m4' or
         'configure.ac' or m4 files included by 'configure.ac'.
         The 'aclocal' program is part of the GNU Automake package:
         <https://www.gnu.org/software/automake>
         It also requires GNU Autoconf, GNU m4 and Perl in order to run:
         <https://www.gnu.org/software/autoconf>
         <https://www.gnu.org/software/m4/>
         <https://www.perl.org/>
```
Download pcre2-10.44.tar.gz from https://github.com/PCRE2Project/pcre2/releases/tag/pcre2-10.44 and extract over src/pcre2. Something is being lost in the clone to/from GitHub that is causing the error. If you know what and how to solve it, please file details of the fix as an issue. Thanks.
