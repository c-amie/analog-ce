/*********************************
*      PNG Graphics Library      *
*********************************/

/* This file contains code from the PNG reference library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifndef HAVE_GD
#define PNG_INTERNAL
#include "png.h"

static void png_default_error PNGARG((png_structp png_ptr,
                                      png_const_charp message));
static void png_default_warning PNGARG((png_structp png_ptr,
                                        png_const_charp message));

void
png_error(png_structp png_ptr, png_const_charp message)
{
   if (png_ptr->error_fn != NULL)
      (*(png_ptr->error_fn))(png_ptr, message);

   png_default_error(png_ptr, message);
}

void
png_warning(png_structp png_ptr, png_const_charp message)
{
   if (png_ptr->warning_fn != NULL)
      (*(png_ptr->warning_fn))(png_ptr, message);
   else
      png_default_warning(png_ptr, message);
}

#define isnonalpha(c) ((c) < 41 || (c) > 122 || ((c) > 90 && (c) < 97))
static PNG_CONST char png_digit[16] = {
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static void
png_format_buffer(png_structp png_ptr, png_charp buffer, png_const_charp message)
{
   int iout = 0, iin = 0;

   while (iin < 4)
   {
      int c = png_ptr->chunk_name[iin++];
      if (isnonalpha(c))
      {
         buffer[iout++] = '[';
         buffer[iout++] = png_digit[(c & 0xf0) >> 4];
         buffer[iout++] = png_digit[c & 0x0f];
         buffer[iout++] = ']';
      }
      else
      {
         buffer[iout++] = (png_byte)c;
      }
   }

   if (message == NULL)
      buffer[iout] = 0;
   else
   {
      buffer[iout++] = ':';
      buffer[iout++] = ' ';
      png_memcpy(buffer+iout, message, 64);
      buffer[iout+63] = 0;
   }
}

void
png_chunk_error(png_structp png_ptr, png_const_charp message)
{
   char msg[16+64];
   png_format_buffer(png_ptr, msg, message);
   png_error(png_ptr, msg);
}

void
png_chunk_warning(png_structp png_ptr, png_const_charp message)
{
   char msg[16+64];
   png_format_buffer(png_ptr, msg, message);
   png_warning(png_ptr, msg);
}

static void
png_default_error(png_structp png_ptr, png_const_charp message)
{
#ifndef PNG_NO_CONSOLE_IO
   fprintf(stderr, "libpng error: %s\n", message);
#endif

#ifdef PNG_SETJMP_SUPPORTED
#  ifdef USE_FAR_KEYWORD
   {
      jmp_buf jmpbuf;
      png_memcpy(jmpbuf,png_ptr->jmpbuf,sizeof(jmp_buf));
      longjmp(jmpbuf, 1);
   }
#  else
   longjmp(png_ptr->jmpbuf, 1);
# endif
#else
   if (png_ptr == NULL)
   PNG_ABORT();
#endif
}

static void
png_default_warning(png_structp png_ptr, png_const_charp message)
{
#ifndef PNG_NO_CONSOLE_IO
   fprintf(stderr, "libpng warning: %s\n", message);
#endif
   if (png_ptr == NULL)
      return;
}

void
png_set_error_fn(png_structp png_ptr, png_voidp error_ptr,
   png_error_ptr error_fn, png_error_ptr warning_fn)
{
   png_ptr->error_ptr = error_ptr;
   png_ptr->error_fn = error_fn;
   png_ptr->warning_fn = warning_fn;
}


png_voidp
png_get_error_ptr(png_structp png_ptr)
{
   return ((png_voidp)png_ptr->error_ptr);
}
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
