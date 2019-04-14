/*********************************
*      PNG Graphics Library      *
*********************************/

/* This file contains code from the PNG reference library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifdef HAVE_GD
#include <pngconf.h>
#else
#ifndef PNGCONF_H
#define PNGCONF_H

#ifndef PNG_ZBUF_SIZE
#define PNG_ZBUF_SIZE 8192
#endif

#if defined(MAXSEG_64K) && !defined(PNG_MAX_MALLOC_64K)
#define PNG_MAX_MALLOC_64K
#endif

#  ifdef PNG_NO_STDIO
#    ifndef PNG_NO_CONSOLE_IO
#      define PNG_NO_CONSOLE_IO
#    endif
#    ifdef PNG_DEBUG
#      if (PNG_DEBUG > 0)
#        include <stdio.h>
#      endif
#    endif
#  else
#    include <stdio.h>
#  endif

#ifndef PNGARG

#ifdef OF
#define PNGARG(arglist) OF(arglist)
#else

#ifdef _NO_PROTO
#define PNGARG(arglist) ()
#else
#define PNGARG(arglist) arglist
#endif

#endif /* OF */
#endif /* PNGARG */

#ifndef MACOS
#if (defined(__MWERKS__) && defined(macintosh)) || defined(applec) || \
    defined(THINK_C) || defined(__SC__) || defined(TARGET_OS_MAC)
#define MACOS
#endif
#endif

#if !defined(MACOS) && !defined(RISCOS)
#include <sys/types.h>
#endif

#ifndef PNG_SETJMP_NOT_SUPPORTED
#  define PNG_SETJMP_SUPPORTED
#endif

#ifdef PNG_SETJMP_SUPPORTED
#  ifdef __linux__
#    ifdef _BSD_SOURCE
#      define _PNG_SAVE_BSD_SOURCE
#      undef _BSD_SOURCE
#    endif
#    ifdef _SETJMP_H
      __png.h__ already includes setjmp.h
      __dont__ include it again
#    endif
#endif /* __linux__ */


#include <setjmp.h>

#  ifdef __linux__
#    ifdef _PNG_SAVE_BSD_SOURCE
#      define _BSD_SOURCE
#      undef _PNG_SAVE_BSD_SOURCE
#    endif
#  endif /* __linux__ */
#endif /* PNG_SETJMP_SUPPORTED */

#ifdef BSD
#include <strings.h>
#else
#include <string.h>
#endif


#ifdef PNG_INTERNAL
#include <stdlib.h>

#define PNG_EXTERN

#if !defined(PNG_NO_FLOATING_POINT_SUPPORTED)
#if defined(MACOS)
#if !defined(__MATH_H__) && !defined(__MATH_H) && !defined(__cmath__)
#include <fp.h>
#endif
#else
#include <math.h>
#endif
#endif


#if (defined(__MWERKS__) && defined(WIN32)) || defined(__STDC__)
#define PNG_ALWAYS_EXTERN
#endif

#ifdef __TURBOC__
#include <mem.h>
#include "alloc.h"
#endif

#ifdef _MSC_VER
#include <malloc.h>
#endif

#ifndef PNG_DITHER_RED_BITS
#define PNG_DITHER_RED_BITS 5
#endif
#ifndef PNG_DITHER_GREEN_BITS
#define PNG_DITHER_GREEN_BITS 5
#endif
#ifndef PNG_DITHER_BLUE_BITS
#define PNG_DITHER_BLUE_BITS 5
#endif

#ifndef PNG_MAX_GAMMA_8
#define PNG_MAX_GAMMA_8 11
#endif

#ifndef PNG_GAMMA_THRESHOLD
#define PNG_GAMMA_THRESHOLD 0.05
#endif

#endif /* PNG_INTERNAL */

#ifndef PNG_NO_CONST
#  define PNG_CONST const
#else
#  define PNG_CONST
#endif

#define PNG_WRITE_PACK_SUPPORTED
#define PNG_tRNS_SUPPORTED

#ifndef PNG_NO_WRITE_WEIGHTED_FILTER
#define PNG_WRITE_WEIGHTED_FILTER_SUPPORTED
#endif

#ifndef PNG_NO_WRITE_FLUSH
#define PNG_WRITE_FLUSH_SUPPORTED
#endif

#ifndef PNG_NO_WRITE_EMPTY_PLTE
#define PNG_WRITE_EMPTY_PLTE_SUPPORTED
#endif

#ifndef PNG_NO_STDIO
#define PNG_TIME_RFC1123_SUPPORTED
#endif

#if defined(PNG_USE_PNGVCRD) || defined(PNG_USE_PNGGCCRD) && \
  !defined(PNG_NO_ASSEMBLER_CODE)
#define PNG_ASSEMBLER_CODE_SUPPORTED
#endif

#ifndef PNG_NO_FLOATING_POINT_SUPPORTED
#define PNG_FLOATING_POINT_SUPPORTED
#endif

#ifndef PNG_NO_FIXED_POINT_SUPPORTED
#define PNG_FIXED_POINT_SUPPORTED
#endif

#ifdef PNG_NO_GLOBAL_ARRAYS
#  define PNG_USE_LOCAL_ARRAYS
#else
#  if defined(__GNUC__) && defined(WIN32)
#    define PNG_NO_GLOBAL_ARRAYS
#    define PNG_USE_LOCAL_ARRAYS
#  else
#    define PNG_USE_GLOBAL_ARRAYS
#  endif
#endif

#ifndef PNG_NO_INFO_IMAGE
#  define PNG_INFO_IMAGE_SUPPORTED
#endif

#if defined(PNG_tIME_SUPPORTED)
#  include <time.h>
#endif

typedef unsigned long png_uint_32;
typedef long png_int_32;
typedef unsigned short png_uint_16;
typedef short png_int_16;
typedef unsigned char png_byte;

typedef size_t png_size_t;

#ifdef __BORLANDC__
#if defined(__LARGE__) || defined(__HUGE__) || defined(__COMPACT__)
#define LDATA 1
#else
#define LDATA 0
#endif

#if !defined(__WIN32__) && !defined(__FLAT__)
#define PNG_MAX_MALLOC_64K
#if (LDATA != 1)
#ifndef FAR
#define FAR __far
#endif
#define USE_FAR_KEYWORD
#endif   /* LDATA != 1 */

#endif  /* __WIN32__, __FLAT__ */

#endif   /* __BORLANDC__ */

#if defined(FAR)
#  if defined(M_I86MM)
#     define USE_FAR_KEYWORD
#     define FARDATA FAR
#     include <dos.h>
#  endif
#endif

#ifndef FAR
#   define FAR
#endif

#ifndef FARDATA
#define FARDATA
#endif

typedef png_int_32 png_fixed_point;

typedef void            FAR * png_voidp;
typedef png_byte        FAR * png_bytep;
typedef png_uint_32     FAR * png_uint_32p;
typedef png_int_32      FAR * png_int_32p;
typedef png_uint_16     FAR * png_uint_16p;
typedef png_int_16      FAR * png_int_16p;
typedef PNG_CONST char  FAR * png_const_charp;
typedef char            FAR * png_charp;
typedef png_fixed_point FAR * png_fixed_point_p;
#ifdef PNG_FLOATING_POINT_SUPPORTED
typedef double          FAR * png_doublep;
#endif


typedef png_byte        FAR * FAR * png_bytepp;
typedef png_uint_32     FAR * FAR * png_uint_32pp;
typedef png_int_32      FAR * FAR * png_int_32pp;
typedef png_uint_16     FAR * FAR * png_uint_16pp;
typedef png_int_16      FAR * FAR * png_int_16pp;
typedef PNG_CONST char  FAR * FAR * png_const_charpp;
typedef char            FAR * FAR * png_charpp;
typedef png_fixed_point FAR * FAR * png_fixed_point_pp;
#ifdef PNG_FLOATING_POINT_SUPPORTED
typedef double          FAR * FAR * png_doublepp;
#endif


typedef char            FAR * FAR * FAR * png_charppp;

typedef charf *         png_zcharp;
typedef charf * FAR *   png_zcharpp;
typedef z_stream FAR *  png_zstreamp;


#ifndef PNG_EXPORT

#  if defined(_MSC_VER) && defined(_DLL)
#    define PNG_EXPORT(type,symbol)        type __declspec(dllexport) symbol
#  endif


#  ifdef __WIN32DLL__	
#    define PNG_EXPORT(type,symbol) __declspec(dllexport) type symbol
#  endif


#  ifdef ALT_WIN32_DLL
#    define PNG_EXPORT(type,symbol) type __attribute__((dllexport)) symbol
#  endif


#  if defined(__BORLANDC__) && defined(_Windows) && defined(__DLL__)
#    define PNG_EXPORT(type,symbol) type _export symbol
#  endif


#  ifdef __BEOSDLL__
#    define PNG_EXPORT(type,symbol) __declspec(export) type symbol
#  endif
#endif

#ifndef PNG_EXPORT
#  define PNG_EXPORT(type,symbol) type symbol
#endif

#if defined(__MINGW32__) || defined(__CYGWIN32__)
#  define PNG_ATTR_DLLIMP
#endif

#ifndef PNG_EXPORT_VAR
#  if defined(_MSC_VER) && defined(_DLL)	
#    define PNG_EXPORT_VAR(type) extern type __declspec(dllexport)
#  endif
#  ifdef PNG_DECL_DLLEXP
#    define PNG_EXPORT_VAR(type) extern __declspec(dllexport) type
#  endif
#  ifdef PNG_ATTR_DLLEXP
#    define PNG_EXPORT_VAR(type) extern type __attribute__((dllexport))
#  endif
#  ifdef PNG_DECL_DLLIMP
#    define PNG_EXPORT_VAR(type) extern __declspec(dllimport) type
#  endif
#  ifdef PNG_ATTR_DLLIMP
#    define PNG_EXPORT_VAR(type) extern type __attribute__((dllimport))
#  endif
#endif

#ifndef PNG_EXPORT_VAR
#    define PNG_EXPORT_VAR(type) extern type
#endif

#ifndef PNG_ABORT
#   define PNG_ABORT() abort()
#endif

#ifdef PNG_SETJMP_SUPPORTED
#   define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#else
#   define png_jmpbuf(png_ptr) \
    (LIBPNG_WAS_COMPILED_WITH__PNG_SETJMP_NOT_SUPPORTED)
#endif

#if defined(USE_FAR_KEYWORD)

#   define CHECK   1
#   define NOCHECK 0
#   define CVT_PTR(ptr) (png_far_to_near(png_ptr,ptr,CHECK))
#   define CVT_PTR_NOCHECK(ptr) (png_far_to_near(png_ptr,ptr,NOCHECK))
#   define png_strcpy _fstrcpy
#   define png_strlen _fstrlen
#   define png_memcmp _fmemcmp
#   define png_memcpy _fmemcpy
#   define png_memset _fmemset
#else
#   define CVT_PTR(ptr)         (ptr)
#   define CVT_PTR_NOCHECK(ptr) (ptr)
#   define png_strcpy strcpy
#   define png_strlen strlen
#   define png_memcmp memcmp
#   define png_memcpy memcpy
#   define png_memset memset
#endif


#if (PNG_ZBUF_SIZE > 65536) && defined(PNG_MAX_MALLOC_64K)
#undef PNG_ZBUF_SIZE
#define PNG_ZBUF_SIZE 65536
#endif

#endif  /* PNGCONF_H */
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
