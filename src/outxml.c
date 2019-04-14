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

// outxml.c - XML output module for analog (http://www.analog.cx)
// written by and copyright Per Jessen, per@computer.org, February 2003. 

#define _GNU_SOURCE

#define NDEBUG

#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "anlghea3.h"
#include "outxml.h"



// we use this to borrow a copy of 'outf' such that function that 
// don't have an 'outf' argument can use XML_TRACE() anyway.
static FILE *outf = 0; 
static FILE **outfp = &outf;

static choice this_report;

static Options *opts;

static int array_index;


// ==================================================================================

/* Page width */
unsigned int xml_pagewidth(Outchoices *od) { XML_TRACE(0); return 0; }

/* The top of the output if we are in CGI mode */
void xml_cgihead(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* Stuff this output style needs in the page header */
void xml_stylehead(FILE *outf, Outchoices *od)
{
	extern timecode_t starttimec;

        unsigned int year, month, day, i;
	struct tm temptime;

        char *xmlversion = "1.0";
        char *encoding = "UTF-8";
	time_t t;
	char buffer[256];

	// borrow a copy of the outfile handle
	*outfp=outf;

	// KLUDGE!!!!
	opts=(Options*)((int)od-offsetof(Options,outopts));	

	t=time(0); 
	strftime( buffer, 256, "%Y%m%d%H%M%S", gmtime(&t) );



	XML_OUT("<?xml version=\"%s\" encoding=\"%s\" standalone=\"no\" ?>"
		XMLDBG "<!DOCTYPE analog-data PUBLIC \"::timian/analog-data::\" \"%s\">"
		XMLDBG "<analog-data version=\"%s\" timestamp=\"%s\">"
		XMLDBG "<analog>",
		xmlversion, 
		encoding,
		od->xmldtd,
		VNUMBER,	// VERSION instead? VERSION includes platform it was built.
		buffer );

	// set up appropriate defaults for XML output.
	xml_defaults( opts );

	// extract a build timestamp from __DATE__ and __TIME__ -- locale dependent?! need to force "C" locale ?
	strptime( __DATE__" "__TIME__, "%b%n%d%n%Y%n%T", &temptime );

	sprintf( buffer, "%04d%02d%02d%02d%02d",
	temptime.tm_year+1900, temptime.tm_mon+1, temptime.tm_mday, temptime.tm_hour, temptime.tm_min );

        code2date( starttimec/1440, &day, &month, &year);
	
	XML_OUT( XMLDBG "<property name=\"version\" content=\"%s\"/>", VERSION );
	XML_OUT( XMLDBG "<property name=\"built\" content=\"%s\"/>", buffer );
	XML_OUT( XMLDBG "<property name=\"starttime\" content=\"%04d%02d%02d%02d%02d\"/> ",
                year, month+1, day, (int)((starttimec%1440)/60), (int)(starttimec%60) );

//	XML_OUT( XMLDBG "<property name=\"304issuccess\" content=\"%s\"/>", opts.succ304?"yes":"no" );
//	XML_OUT( XMLDBG "<property name=\"logtimeoffset\" content=\"%d\"/>", stz );

	i=i;	// mustn't upset the compiler :-)

}

/* The title of the page, plus the user's HEADERFILE */
void xml_pagetitle(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* Program start time, and logfile start and end times */
void xml_timings(FILE *outf, Outchoices *od, Dateman *dman)
{
	unsigned int year, month, day;

	XML_TRACE(0);

	code2date( dman->firsttime/1440, &day, &month, &year);
        XML_OUT( XMLDBG"<timespan from=\"%04d%02d%02d%02d%02d\" ",
                year, month+1, day, (int)((dman->firsttime%1440)/60), (int)(dman->firsttime%60) );

        code2date( dman->lasttime/1440, &day, &month, &year);
        XML_OUT( "to=\"%04d%02d%02d%02d%02d\" days=\"%f\"/>",
                year, month+1, day, (int)((dman->lasttime%1440)/60), (int)(dman->lasttime%60),
                (dman->lasttime-dman->firsttime)/1440.0+0.005 );

}

/* Finishing the top of the page */
void xml_closehead(FILE *outf, Outchoices *od) { XML_TRACE(0); XML_OUT(XMLDBG"</analog>"); }

/* Starting the bottom of the page */
void xml_pagebotstart(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* The credit line at the bottom of the page */
void xml_credit(FILE *outf, Outchoices *od)
{ 
	XML_TRACE(0);
}

/* The program run time */
void xml_runtime(FILE *outf, Outchoices *od, long secs)
{
	XML_TRACE(0);
//	XML_OUT( XMLDBG"<property name=\"%s\" contents=\"%u\"/>",
//		"processingtime", secs);
}

/* The page footer, including the user's FOOTERFILE */
void xml_pagefoot(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* Footer material for this output style */
void xml_stylefoot(FILE *outf, Outchoices *od)
{
	XML_TRACE(0);
	XML_OUT( XMLDBG"</analog-data>" );

}

/* Report title */
void xml_reporttitle(FILE *outf, Outchoices *od, choice rep)
{
	char *dir;

	XML_TRACE(0); 

	assert( ((char*)od-(char*)opts)==offsetof(Options,outopts) );

	array_index=0;

	XML_OUT( XMLDBG"<report name=\"%s\">", strtolower(report_name[rep]) );

	this_report=rep;

	switch (rep) {
	case REP_GENSUM:
		break;
	case REP_YEAR:
	case REP_QUARTERLY:
	case REP_MONTH:
	case REP_WEEK:
	case REP_DAYREP:
	case REP_HOURREP:
	case REP_QUARTERREP:
	case REP_FIVEREP:
		XML_OUT( XMLDBG "<rowlimit count=\"%d\"/>", opts->outopts.rows[rep] );
	case REP_DAYSUM:
	case REP_HOURSUM:
	case REP_WEEKHOUR:
	case REP_QUARTERSUM:
	case REP_FIVESUM:
		// only time reports (for the time being) have a 'back' setting to set their sort-order.
		XML_OUT( XMLDBG "<sort dir=\"%s\" by=\"\"/>", od->back[rep]? "descending" : "ascending" );
		break;
	case REP_SIZE:
	case REP_PROCTIME:
	case REP_HOST:
	case REP_REDIRHOST:
	case REP_FAILHOST:
	case REP_VHOST:
	case REP_REDIRVHOST:
	case REP_FAILVHOST:
	case REP_USER:
	case REP_REDIRUSER:
	case REP_FAILUSER:
	case REP_SEARCHREP:
	case REP_SEARCHSUM:
	case REP_INTSEARCHREP:
	case REP_INTSEARCHSUM:
	case REP_BROWSUM:
	case REP_CODE:
		od->subsortby[rep]=
		od->subfloor[rep].floorby=UNSET;	

	case REP_TYPE:		// the following reports can also have subfloor/sort setting
        case REP_DIR:
        case REP_REQ:
        case REP_REDIR:
        case REP_FAIL:
        case REP_DOM:
        case REP_REF:
        case REP_BROWREP:
        case REP_OS:
        case REP_REDIRREF:
        case REP_FAILREF:
        case REP_ORG:
        case REP_REFSITE:
		dir="ascending"; // analog will sort all of these in ascending order.

		XML_OUT( XMLDBG "<sort dir=\"%s\" by=\"%s\"/>", dir, od->sortby[rep]==UNSET?"":criteria[od->sortby[rep]] );

		// 5.90b1 there's something funky going on with subsortby
		// 5.90b2 ?
		assert( od->subsortby[rep]==UNSET || (od->subsortby[rep]>=REQUESTS && od->subsortby[rep]<=METHOD_NUMBER) );

		if ( od->sortby[rep]!=UNSET )
		if ( od->subsortby[rep]!=UNSET )	// is this dependable ??
//		XML_OUT( XMLDBG "<sort dir=\"%s\" by=\"%s\"/>", dir, criteria[od->subsortby[rep]] );
		XML_OUT( XMLDBG "<sort dir=\"%s\" by=\"%u\"/>", dir, od->subsortby[rep] );

		// the *FLOOR setting
		XML_OUT( XMLDBG "<floor level=\"%f\" ", od->floor[rep].min );
		if ( od->floor[rep].qual ) XML_OUT( "qual=\"%c\" ", od->floor[rep].qual );
	        XML_OUT( "by=\"%s\"/>", od->floor[rep].floorby==UNSET?"":criteria[od->floor[rep].floorby] );

		if ( od->subfloor[rep].floorby!=UNSET )	
		{
		XML_OUT( XMLDBG "<floor level=\"%f\" ", od->subfloor[rep].min );
//		if ( od->subfloor[rep].qual ) XML_OUT( "qual=\"%c\" ", od->subfloor[rep].qual )
                XML_OUT( "by=\"%s\"/>", criteria[od->floor[rep].floorby] );

		}

		break;
	default:	
		puts(report_name[rep]);
		assert(0);
		break;
	}

}

/* Report footer */
void xml_reportfooter(FILE *outf, Outchoices *od, choice rep) { XML_TRACE(0); }

/* Report description */
void xml_reportdesc(FILE *outf, Outchoices *od, choice rep) { XML_TRACE(0); }

/* The time period spanned by the report */
void xml_reportspan(FILE *outf, Outchoices *od, choice rep, timecode_t maxd, timecode_t mind)
{ 
	unsigned int year, month, day;

	XML_TRACE(0);

        code2date( mind/1440, &day, &month, &year);
        XML_OUT( XMLDBG"<timespan from=\"%04d%02d%02d%02d%02d\" ",
                year, month+1, day, (int)((mind%1440)/60), (int)(mind%60) );

        code2date( maxd/1440, &day, &month, &year);
        XML_OUT( "to=\"%04d%02d%02d%02d%02d\" days=\"%f\"/>",
                year, month+1, day, (int)((maxd%1440)/60), (int)(maxd%60),
                (maxd-mind)/1440.0+0.005 );

}

/* General Summary header */
void xml_gensumhead(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* General Summary footer */
void xml_gensumfoot(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* Single General Summary line, long data */
void xml_gensumline(FILE *outf, Outchoices *od, int namecode, unsigned long x, unsigned long x7, logical isaverage)
{
	if ( x7==(unsigned long)UNSET ) x7=0;
	xml_gensumlineb(outf,od,namecode,(double)x,(double)x7,isaverage);
}


/* Single General Summary line, bytes data */
void xml_gensumlineb(FILE *outf, Outchoices *od, int namecode, double x, double x7, logical isaverage)
{
	char *name;

	switch( namecode ) {
	case succreqs_:   name="succreqs_";	break;
	case avereqs_:    name="avereqs_";	break;
	case totpages_:	  name="totpages_";	break;
	case avepages_:	  name="avepages_";	break;
	case totunknown_: name="totunknown_";	break;
	case totfails_:   name="totfails_";	break;
	case totredirs_:  name="totredirs_";	break;
	case inforeqs_:   name="inforeqs_";	break;
	case distfiles_:  name="distfiles_";	break;
	case disthosts_:  name="disthosts_";	break;
	case corrupt_:    name="corrupt_";	break;	
	case unwanted_:   name="unwanted_";	break;
	case totdata_:	  name="totdata_";	break;
	case avedata_:	  name="avedata_";	break;
	default:	  name="__unknown__";	break;
	}


        XML_OUT( XMLDBG"<row>"
                "<col name=\"name\"><i18n key=\"%s\"/></col>",  name );
        XML_OUT( XMLDBG"<col name=\"%s\">%f</col>", strtolower(column_name[COL_REQS]), x );
        XML_OUT( XMLDBG"<col name=\"%s\">%f</col>", strtolower(column_name[COL_REQS7]), x7 );
        XML_OUT( XMLDBG"</row>" );

}


/* "Last seven" explanation line */
void xml_lastseven(FILE *outf, Outchoices *od, timecode_t last7to)
{
	unsigned int year, month, day;

	XML_TRACE(0);
	code2date( last7to/1440, &day, &month, &year);

	XML_OUT( XMLDBG "<!-- lastseven: %04d%02d%02d%02d%02d -->", 
                year, month+1, day, (int)((last7to%1440)/60), (int)(last7to%60) );

}

/* Start of a <pre> section */
void xml_prestart(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* strlen */
size_t xml_strlength(const char *s) { XML_TRACE(0); return strlen(s); }

/* End of a <pre> section */
void xml_preend(FILE *outf, Outchoices *od) { XML_TRACE(0); }

/* A horizontal rule */
void xml_hrule(FILE *outf, Outchoices *od)
{
	// an odd place to place this, but the only option.
	XML_OUT( XMLDBG"</report><!-- name=\"%s\" -->", strtolower(report_name[this_report]) );
}

/* An en dash */
char *xml_endash(void) { XML_TRACE(0); return "-"; }

/* putc with special characters escaped */
void xml_putch(FILE *outf, char c) { XML_TRACE(0); }

/* Allow month in dates? DO NOT enable for human-readable text because of
   i18n problems. */
logical xml_allowmonth(void) { XML_TRACE(0); return TRUE; }

/* Calculate column widths */
void xml_calcwidths(Outchoices *od, choice rep, unsigned int width[],
		     unsigned int *bmult, unsigned int *bmult7, double *unit,
		     unsigned long maxr, unsigned long maxr7,
		     unsigned long maxp, unsigned long maxp7, double maxb,
		     double maxb7, unsigned long howmany)
{
	XML_TRACE(0);
	// outxml has no need to do this, but analog will segfault otherwise.
	calcwidths(od, rep, width, bmult, bmult7, unit, maxr, maxr7, maxp, maxp7,
		maxb, maxb7, howmany);
}

/* "Each unit represents" line */
void xml_declareunit(FILE *outf, Outchoices *od, char graphby, double unit,
		      unsigned int bmult) { XML_TRACE(0); }

/* Start of column header line */
void xml_colheadstart(FILE *outf, Outchoices *od, choice rep) { XML_TRACE(0); }


/* Column header line: individual column */
void xml_colheadcol(FILE *outf, Outchoices *od, choice rep, choice col, unsigned int width, char *colname, logical unterminated) {XML_TRACE(0);}

/* End of column header line */
void xml_colheadend(FILE *outf, Outchoices *od, choice rep) {XML_TRACE(0);}

/* Start of column header underlining line */
void xml_colheadustart(FILE *outf, Outchoices *od, choice rep) {XML_TRACE(0);}

/* Underlining of one column header. */
/* If column is terminated, set width and leave name blank; and conversely. */
void xml_colheadunderline(FILE *outf, Outchoices *od, choice rep, choice col, unsigned int width, char *name) {XML_TRACE(0);}

/* End of column header underlining line */
void xml_colheaduend(FILE *outf, Outchoices *od, choice rep) { XML_TRACE(0); }

/* Start of a table row */
void xml_rowstart(FILE *outf, Outchoices *od, choice rep, choice *cols,
		   int level, char *name, char *datefmt, char *timefmt)
{
	XML_TRACE(0);

	if ( level!=UNSET )
	XML_OUT( XMLDBG"<row level=\"%d\">", level );
	else
	XML_OUT( XMLDBG"<row>" );
}

/* Print level in hierarchy represented by this row */
void xml_levelcell(FILE *outf, Outchoices *od, choice rep, int level)
{
	XML_TRACE(0);

//	XML_OUT( XMLDBG"<col name=\"COL_LEVEL\">%d</col>", level );
}

/* Name column */
void xml_namecell(FILE *outf, Outchoices *od, choice rep, char *name,
  choice source, unsigned int width, logical name1st, logical isfirst,
  logical rightalign, Alias *aliashead, Include *linkhead, logical ispage,
  unsigned int spaces, char *baseurl)
{
	char *p, *q;
	int n;

	XML_TRACE(0); 

	// no output aliasing is done yet. not sure it will be done either.


	// we don't care, but by testing it we avoid printing the column twice.
	if ( !isfirst && name )
	{
		XML_OUT( XMLDBG "<col name=\"name\">" );

		switch( rep) {
		case REP_MONTH:
		case REP_YEAR:
		case REP_WEEK:
		case REP_QUARTERLY:
		case REP_DAYREP:
		case REP_HOURREP:
		case REP_QUARTERREP:
		case REP_FIVEREP:
		case REP_DAYSUM:
		case REP_HOURSUM:
		case REP_QUARTERSUM:
		case REP_FIVESUM:
		case REP_WEEKHOUR:	
			// this is split because strtok needs to be called in the right order.
			XML_OUT( "<timespan from=\"%s\" ", strtok(name," ") ); 
			XML_OUT( "to=\"%s\" ", strtok( NULL," ") );
			switch( rep ) {
			case REP_DAYSUM:
				XML_OUT( "dayofweek=\"%s\" />", strtok( NULL," ") );
				break;
			case REP_HOURSUM:
				XML_OUT( "hour=\"%s\" />", strtok( NULL," ") );
				break;
			case REP_QUARTERSUM:
				XML_OUT( "minute=\"%s\" />", strtok( NULL," ") );
				break;
			case REP_FIVESUM:
				XML_OUT( "minute=\"%s\" />", strtok( NULL," ") );
				break;
			case REP_WEEKHOUR:
				XML_OUT( "dayofweek=\"%s\" ", strtok( NULL," ") );
				XML_OUT( "hour=\"%s\" />", strtok( NULL," ") );
				break;
			default:
				XML_OUT( "/>" );
			}
			break;
		case REP_CODE:
			strtok(name, " ");	// assumption: name = "nnn text"
			XML_OUT( name );
			break;
//			n=array_index++;
//			XML_OUT( "%d", (int)(opts->arraydata[rep-FIRST_ARRAYREP][n].threshold) );
//			break;
		case REP_OS:
			// kludge: the hardcoded OS names from tree.c are turned into i18n
			// keys - hardly a well thought out interface, but the best we can do
			// for the moment.
			p=strtolower(name);			// all lowercase
			while ( (q=strchr(p,' ')) ) *q='-';	// replace ' ' with '-'
			while ( (q=strchr(p,'/')) ) *q='-';
			
			XML_OUT( "<i18n key=\"os-%s\"/>", p );
			break;
		case REP_SIZE:
		case REP_PROCTIME:
			n=array_index++;
			XML_OUT( "%f", opts->arraydata[rep-FIRST_ARRAYREP][n].threshold );
			break; 
		case REP_DOM:
		case REP_TYPE:
			// kludge: if the string supplied is delimited by square brackets, 
			// we turn it into an i18n key.
			if ( *name=='[' && *(name+strlen(name)-1)==']' )
			{
				p=strtolower(name+1); 
				*(p+strlen(p)-1)=0;
				while ( (q=strchr(p,' ')) ) *q='-';
				XML_OUT( "<i18n key=\"%s-%s\"/>", rep==REP_DOM?"domain":"type", p );
				break;
			}
			// this is a delibrate fall-thru.
		default:
			XML_OUT( "<![CDATA[%s]]>", cleanprint(name) );
			break;
		}
		XML_OUT( "</col>" );
	}

#if 0
extern char *workspace;

  choice savemultibyte;
  logical linked;
  int i;

  if (name1st != isfirst)
    return;

  if (isfirst)
    matchlengthn(outf, od, width - od->outputter->strlen(name), ' ');
  savemultibyte = od->multibyte;
  if (rep == REP_SIZE || rep == REP_PROCTIME)
    /* Kludge: for these two reports, we know the texts are things like
       "< 1" and we want to convert > and < */
    od->multibyte = FALSE;

  strcpy(workspace, name);
  do_aliasx(workspace, aliashead);

  if (!isfirst) {
    if (rightalign)
      i = (int)width - (int)od->outputter->strlen(workspace) - (int)spaces;
    else
      i = (int)spaces;
    matchlengthn(outf, od, i, ' ');
  }

  linked = (linkhead != NULL && included(name, ispage, linkhead));
  if (linked) {
    /* We link to the unaliased name, because the OUTPUTALIAS is usually in
       the nature of an annotation. */
    fputs("<a href=\"", outf);
    if (baseurl != NULL)
      htmlputs(outf, od, baseurl, IN_HREF);
//    xml_escfprintf(outf, name);
    fputs("\">", outf);
  }

  htmlputs(outf, od, workspace, UNTRUSTED);

  if (linked)
    fputs("</a>", outf);

  if (isfirst)
    fputs(": ", outf);

  od->multibyte = savemultibyte;  /* restore multibyte */
#endif
}


/* Single cell, unsigned long argument */
void xml_ulcell(FILE *outf, Outchoices *od, choice rep, choice col, unsigned long x, unsigned int width)
{
	XML_TRACE(0);
	XML_OUT( XMLDBG"<col name=\"%s\">%lu</col>", strtolower(column_name[col]), x ); 
}

/* Single cell, TRUSTED string argument */
void xml_strcell(FILE *outf, Outchoices *od, choice rep, choice col, char *s, unsigned int width)
{
	XML_TRACE(0);

	XML_OUT( XMLDBG "<col name=\"%s\">", strtolower(column_name[col]) );
	switch( col )
	{
	case COL_TIME:
	case COL_FIRSTT:
	case COL_DATE:
	case COL_FIRSTD:
		if ( strlen(s) )
		XML_OUT( "<timespan from=\"%s\"/>", strtok(s, " ") );	
		break;
	default:
		XML_OUT( s );
		break;
	}
	XML_OUT( "</col>" );

}

/* Single cell, listing bytes */
void xml_bytescell(FILE *outf, Outchoices *od, choice rep, choice col, double b, double bmult, unsigned int width)
{
	XML_TRACE(0);
	XML_OUT( XMLDBG"<col name=\"%s\">%f</col>", strtolower(column_name[col]), b );
}

/* Single cell, listing percentage */
void xml_pccell(FILE *outf, Outchoices *od, choice rep, choice col, double n, double tot, unsigned int width)
{
	XML_TRACE(0);
	XML_OUT( XMLDBG"<col name=\"%s\">%f</col>", strtolower(column_name[col]),
	tot>0 ? 100*n/tot : 0.0 );
}

/* Single cell, index */
void xml_indexcell(FILE *outf, Outchoices *od, choice rep, choice col, long index, unsigned int width)
{
	XML_TRACE(0);
	XML_OUT( XMLDBG"<col name=\"%s\">%ld</col>", strtolower(column_name[col]), index );
}

/* End of a table row */
void xml_rowend(FILE *outf, Outchoices *od, choice rep) { XML_TRACE(0); XML_OUT(XMLDBG"</row>"); }

/* Blank line in time reports */
void xml_blankline(FILE *outf, Outchoices *od, choice *cols) { XML_TRACE(0); }

/* Barchart in time reports */
void xml_barchart(FILE *outf, Outchoices *od, int y, char graphby) { XML_TRACE(0); }

/* "Busiest time period" line */
void xml_busyprintf(FILE *outf, Outchoices *od, choice rep, char *datefmt,
		     unsigned long reqs, unsigned long pages, double bys,
		     datecode_t date, unsigned int hr, unsigned int min,
		     datecode_t newdate, unsigned int newhr,
		     unsigned int newmin, char graphby)
{
	unsigned int year, month, day;

	XML_TRACE(0);

	code2date( date, &day, &month, &year);
	XML_OUT( XMLDBG"<busiest time=\"%04d%02d%02d%02d%02d\" ", 
		year, month+1, day, hr, min );

	switch( graphby ) {
	case 'R':
	case 'r':
		XML_OUT( "reqs=\"%lu\"", reqs );
		break;
	case 'P':
	case 'p':
		XML_OUT( "pages=\"%lu\"", pages );
		break;
	case 'B':
	case 'b':
		XML_OUT( "bytes=\"%f\"", bys );
		break;
	default:
		// what happens to busiest time when no graphby is set?
		assert(0); 
		break;
	}

	XML_OUT( "/>" );

#if 0
  extern unsigned int *rep2busystr;

  char **lngstr = od->lngstr;
  char sepchar = od->sepchar;

  unsigned int bmult;
  char *s;

  fprintf(outf, "%s %s (", lngstr[rep2busystr[rep]],
	  datesprintf(od, datefmt, date, hr, min, newdate, newhr, newmin,
		      TRUE, UNSET));

  if (TOLOWER(graphby) == 'r') {
    f3printf(outf, od, (double)reqs, 0, sepchar);
    fprintf(outf, " %s).\n", (reqs == 1)?lngstr[request_]:lngstr[requests_]);
  }
  else if (TOLOWER(graphby) == 'p') {
    f3printf(outf, od, (double)pages, 0, sepchar);
    fprintf(outf, " %s).\n",
	    (pages == 1)?lngstr[pagereq_]:lngstr[pagereqs_]);
  }
  else /* TOLOWER(graphby) == 'b' */ {
    if (od->rawbytes)
      bmult = 0;
    else
      bmult = findbmult(bys, od->bytesdp);
    printbytes(outf, od, bys, bmult, 0, sepchar);
    putc(' ', outf);
    if (bmult >= 1) {
      s = strchr(lngstr[xbytes_], '?');  /* checked in initialisation */
      *s = '\0';
      fprintf(outf, "%s%s%s).\n", lngstr[xbytes_],
	      lngstr[byteprefix_ + bmult], s + 1);
      *s = '?';
    }
    else
      fprintf(outf, "%s).\n", lngstr[bytes_]);
  }
#endif
}

/* End of "Not listed" line. */
void xml_notlistedstr(FILE *outf, Outchoices *od, choice rep, unsigned long badn)
{
	XML_TRACE(0);
	XML_OUT( XMLDBG "<col name=\"name\"><i18n key=\"notlisted-%s\" var=\"%lu\"/></col>", strtolower(report_name[rep]), badn );
#if 0
  extern unsigned int *rep2lng, *rep2colhead;

  char **lngstr = od->lngstr;
  char *colhead = lngstr[rep2colhead[rep]];
  char *colheadp = lngstr[rep2colhead[rep] + 1];
  char gender = lngstr[rep2lng[rep] + 3][0];

  char *notlistedstr;

  if (gender == 'm')
    notlistedstr = lngstr[notlistedm_];
  else if (gender == 'f')
    notlistedstr = lngstr[notlistedf_];
  else
    notlistedstr = lngstr[notlistedn_];

  fprintf(outf, "[%s: ", notlistedstr);
  f3printf(outf, od, (double)badn, 0, od->sepchar);
  fprintf(outf, " %s]\n", (badn == 1)?colhead:colheadp);
#endif
}

/* The line declaring the floor and sort for a report */
void xml_whatincluded(FILE *outf, Outchoices *od, choice rep,
		       unsigned long n, Dateman *dman) { 
	XML_TRACE(0);

//	floor_element( outf, od, rep );

	#ifndef NDEBUG
	XML_OUT( XMLDBG "<!-- a textual representation of the FLOOR and SORT settings. -->" );
	whatincluded(outf, od, rep, n, dman);
	#endif
}

/* Spacing at the start of the whatincluded line */
void xml_whatinchead(FILE *outf, Outchoices *od)
{
	XML_TRACE(0); XML_OUT( "\n<!-- whatincluded: \"" );
}

/* Finishing the whatincluded line */
void xml_whatincfoot(FILE *outf, Outchoices *od)
{
	XML_TRACE(0); XML_OUT( "\" -->" );
}

/* Printing part of the whatincluded line */
void xml_whatincprintstr(FILE *outf, Outchoices *od, char *s)
{ 
	XML_TRACE(0); XML_OUT( s );
}

/* Print a double */
void xml_printdouble(FILE *outf, Outchoices *od, double x)
{
	XML_OUT( "%g", x );
}

/* Include a header file or footer file */
void xml_includefile(FILE *outf, Outchoices *od, char *name, char type) { XML_TRACE(0); }

/* Filetype for RISC OS */
unsigned int xml_riscosfiletype(void) {
  return 0xf80;
}

// ======= miscellaneous helper functions ===============================================

#ifdef _NEED_XML_PRINTF
static int xml_printf( char *format, ... )
{
	va_list v;
	int r;

	va_start( v, format );

	r=vfprintf( outf, format, v );

	va_end( v );

	return r;
}
#endif

static char* strtolower( const char *s )
{
        static char *buffer=0;
        char *p;

        // buffer is freed upon program termination.
        buffer=realloc(buffer, max(strlen(s)+1,1024));

        for( p=buffer; *s; p++, s++ )
        {
                *p=tolower(*s);
        }
        *p=0;

        return buffer;
}

// converts a null-terminated array of null-terminated strings to lower case.
static void all2lower( char** strings )
{
	char *p;

	while( *strings )
	{
		for ( p=*strings; *p; p++ )
		{
			*p=tolower(*p);
		}
		strings++;
	}
}

// this function sets the output options as they are required for XML output. 
// it's a bit of a kludge, but will have to do till Stephen implements a method for
// letting an output module set its own defaults. 
static void xml_defaults( Options *op)
{
	extern logical cgi;
	Outchoices *od=&(opts->outopts);

	choice timerepcols[] = { COL_REQS, COL_PREQS, COL_PAGES, COL_PPAGES, COL_BYTES, COL_PBYTES, COL_NUMBER };
	choice genrepcols[] = { COL_REQS, COL_PREQS, COL_REQS7, COL_PREQS7, 
				COL_PAGES, COL_PPAGES, COL_PAGES7, COL_PPAGES7, 
				COL_BYTES, COL_PBYTES, COL_BYTES7, COL_PBYTES7,
				COL_TIME, COL_FIRSTT, 
				COL_INDEX, COL_NUMBER };
	int i;
	static char *numbers[]={"0","1","2","3","4","5","6","7","8","9","10","11","12",0};


	od->rawbytes=TRUE;				// we always want bytes listed as raw numbers.
	od->gensumlines="BCDEFGHIJKLMN";		// we always want all the general summary lines
							// always select english as output language ??

	for( i=REP_YEAR; i<DATEREP_NUMBER; i++ )	// timereports will always list all columns.
	memcpy( od->cols[i], timerepcols, sizeof(timerepcols) );


	for( i=REP_REQ; i<REP_NUMBER; i++ ) 		// all other reports will always list all columns.
	memcpy( od->cols[i], genrepcols, sizeof(genrepcols) );

	for( i=0; i<7; i++ )				// ignore i18n daynames, just use 0..6
	od->dayname[i]=numbers[i];	

        od->lngstr[genreptime_]=
        od->lngstr[genrepdate_]=
        od->lngstr[datefmt1_]=
        od->lngstr[datefmt2_]=
        od->lngstr[weekfmt_]=
        od->lngstr[yearfmt_]=
        od->lngstr[dayrepfmt_]=
        od->lngstr[quarterlyfmt_]=
        od->lngstr[hourrepfmt_]=
        od->lngstr[quarterrepfmt_]=
        od->lngstr[monthfmt_]="%Y%M%D%H%n %Z%L%E%I%o";

	od->lngstr[quartersumfmt_]="%Y%M%D%H%n %Z%L%E%I%o %n";
	od->lngstr[hoursumfmt_]="%Y%M%D%H%n %Z%L%E%I%o %h";
        od->lngstr[daysumfmt_]="%Y%M%D%H%n %Z%L%E%I%o %w";
	od->lngstr[weekhoursumfmt_]="%Y%M%D%H%n %Z%L%E%I%o %w %h";

	for( i=0; i<11; i++ )
	od->lngstr[filesize0_+i]=numbers[i];


	cgi=FALSE;					// not possible 

	for( i=0; i<REP_NUMBER; i++ ) 			// always list all reports in this order
		od->reporder[i]=i;			// (depends on ALL ON in analog.cfg)


        // override reports settings ??
//	opts.outopts.repspan=
//	od->repspan=TRUE;				// analogs manual says this could be memory consuming?
//	opts.outopts.rsthresh=-1;			// list reportspan on all reports

	od->last7=TRUE;

}

#if 0
static char* utf8( char* in )
{
        static char *out=NULL;
        static size_t outsz=4;

        size_t inlen, outlen; 
	char *inptr, *outptr;
	int rc, i;


	if ( out==NULL ) out=malloc(outsz);		// initial output buffer
	iconv( cd, NULL, NULL, NULL, NULL);		// reset conversion state

	outlen=outsz;					// initial outbuffer size
	inlen=strlen(in);				// input buffer length
	inptr=in;
	outptr=out;

	while( inlen )
	{
//		fprintf(stderr, "inlen=%d, string=\"%s\"\n", inlen, in);
		rc=iconv( cd, &inptr, &inlen, &outptr, &outlen );
		if ( rc==-1 )
		switch( errno ) {
		case E2BIG:	// buffer not big enough?
			outlen+=outsz;			// adding this much space to output buffer
			out=realloc( out, outsz+=outsz );	// allocate bigger buffer
//			fprintf(stderr, "outsz=%d, inlen=%d, outlen=%d\n", outsz, inlen, outlen);
			break;
		case EILSEQ:	// An  invalid  multibyte  sequence is encountered in the input.
//			fprintf( stderr, "%s:%d: error %d: %s\n", __FILE__, __LINE__, errno, strerror(errno) );
//			inptr++; inlen--;               // try just skipping one input char
//			break;
		case EINVAL:	// An incomplete multibyte sequence is encountered in the input.
			fprintf( stderr, "%s:%d: error %d: %s\n", __FILE__, __LINE__, errno, strerror(errno) );
			fprintf( stderr, "%s:%d: string=\"%s\"\n", __FILE__, __LINE__, in, strerror(errno) );
			inlen=0;			//terminate conversion here.
			break;
		default:
			fprintf( stderr, "%s:%d: error %d: %s\n", __FILE__, __LINE__, errno, strerror(errno) );
			my_exit(8);
		}
	}
	// the majority of conversions will be one-to-one, so don't bother telling us about them
	if ( strlen(in)!=(outptr-out) )
	fprintf( stderr, "%d chars in converted to %d chars out\n", strlen(in), outptr-out );

	*outptr=0;		// terminate output buffer

	return out;
}
#endif

char *cleanprint( char *name )
{
	static char *out=NULL;
	static size_t outsz=4;
	char *outptr, c; 


	if ( out==NULL ) out=malloc(outsz);
	outptr=out;

	// cleanprint: printable characters are printed straight, unprintable as hexadecimal codes.

	while( (c=*name++) )
	{
		if ( (outsz-(int)(outptr-out))<6 )
		{
			outptr=(char*)(outptr-out);	// save old
			out=realloc(out, outsz*=2); 	// reallocate buffer
			outptr=out+(int)outptr;		// repoint		
		}
		if ( isprint( c ) ) *outptr++=c;
		else outptr+=sprintf( outptr, "[0x%02X]", (unsigned char)c );
	}
	*outptr=0;

	return out;
}

// I am keeping the name constants below in upper case to make comparison with anlghea3.h easier.
// Before they're used in the output, they're converted to lower case.
// It would be better to convert them prior to use, but this will do for now.


// this is text representation of the column name enumeration from anlghea3.h
static const char *column_name[] = {
"COL_TITLE",
"COL_REQS",
"COL_REQS7",
"COL_PAGES",
"COL_PAGES7",
"COL_BYTES",
"COL_BYTES7",
"COL_PREQS",
"COL_PREQS7",
"COL_PPAGES",
"COL_PPAGES7",
"COL_PBYTES",
"COL_PBYTES7",
"COL_DATE",
"COL_TIME",
"COL_FIRSTD",
"COL_FIRSTT",
"COL_INDEX" };

// this is a text representation of the report name enumeration from anlghea3.h
static const char *report_name[] = {
"REP_YEAR",
"REP_QUARTERLY",
"REP_MONTH",
"REP_WEEK",
"REP_DAYREP",
"REP_HOURREP",
"REP_QUARTERREP",
"REP_FIVEREP",
"REP_DAYSUM",
"REP_HOURSUM",
"REP_WEEKHOUR",
"REP_QUARTERSUM",
"REP_FIVESUM",
"REP_GENSUM",
"REP_REQ",
"REP_REDIR",
"REP_FAIL",
"REP_TYPE",
"REP_DIR",
"REP_HOST",
"REP_REDIRHOST",
"REP_FAILHOST",
"REP_DOMAIN",		// "REP_DOM",
"REP_REF",
"REP_REFSITE",
"REP_REDIRREF",
"REP_FAILREF",
"REP_BROWREP",
"REP_VHOST",
"REP_REDIRVHOST",
"REP_FAILVHOST",
"REP_USER",
"REP_REDIRUSER",
"REP_FAILUSER",
"REP_ORG",
"REP_SEARCHREP",
"REP_SEARCHSUM",
"REP_INTSEARCHREP",
"REP_INTSEARCHSUM",
"REP_BROWSUM",
"REP_OS",
"REP_SIZE",
"REP_CODE",
"REP_PROCTIME" };

// this is a text representation of floor and sort settings .....
static const char *criteria[] = {
"REQUESTS", "REQUESTS7", "PAGES", "PAGES7", "REDIR", "REDIR7", "FAIL", "FAIL7", "SUCCDATE",
"REDIRDATE", "FAILDATE", "SUCCFIRSTD", "REDIRFIRSTD", "FAILFIRSTD", "BYTES", "BYTES7",
"ALPHABETICAL", "RANDOM" };


