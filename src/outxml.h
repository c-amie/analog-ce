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

// outxml.h - XML output module for analog (http://www.analog.cx)
// written by and copyright Per Jessen, per@computer.org, February 2003. 


// leave this to build nicely readable xml output.
#define XMLDBG "\n"


#define max(a,b) ((a)>(b)?(a):(b))

//#ifdef __USE_ISOC99
#if 0
	#define XML_OUT( f... )	fprintf( outf, f )
#else
	#define XML_OUT	xml_printf
	#define _NEED_XML_PRINTF
	#define __func__ ""
	static int xml_printf( char*, ... );
#endif


#define XML_TRACE(x) if ( x ) if ( outf ) XML_OUT( XMLDBG "<!-- %s:%5d %s() -->", __FILE__, __LINE__, __func__ )


// we only need this when we don't have strptime().
#ifndef __USE_GNU
extern char *engmonths[12];

// note: when defined like this, strptime() provides no return value. 
// also, this is obviously not! a complete replacement for strptime()  
#define strptime( bfr, format, tm ) 								\
        sscanf( (bfr), "%s %d %d %d:%d:%*d", 							\
	buffer, &((tm)->tm_mday), &((tm)->tm_year), &((tm)->tm_hour), &((tm)->tm_min) );	\
        (tm)->tm_year-=1900;									\
        for( i=0; i<12 && strcmp(engmonths[i],buffer); i++) ;		\
        (tm)->tm_mon=i;	
#endif


static void all2lower( char** );
static char* strtolower( const char* );

static void xml_defaults( Options *opts );
static char* cleanprint( char* );

//#define OFFSET(s,m) (int)(&(((s*)0)->m))

static const char *column_name[COL_NUMBER], *report_name[REP_NUMBER], *criteria[METHOD_NUMBER];

