/*********************************
*      PNG Graphics Library      *
*********************************/

/* This file contains code from the PNG reference library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifndef HAVE_GD
#define PNG_INTERNAL
#define PNG_NO_EXTERN
#include <assert.h>
#include "png.h"

typedef version_1_0_6 Your_png_h_is_not_version_1_0_6;

#ifdef PNG_USE_GLOBAL_ARRAYS
char png_libpng_ver[12] = "1.0.6";

png_byte FARDATA png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};

PNG_IHDR;
PNG_IDAT;
PNG_IEND;
PNG_PLTE;
PNG_bKGD;
PNG_cHRM;
PNG_gAMA;
PNG_hIST;
PNG_iCCP;
PNG_iTXt;
PNG_oFFs;
PNG_pCAL;
PNG_sCAL;
PNG_pHYs;
PNG_sBIT;
PNG_sRGB;
PNG_tEXt;
PNG_tIME;
PNG_tRNS;
PNG_zTXt;

int FARDATA png_pass_start[] = {0, 4, 0, 2, 0, 1, 0};
int FARDATA png_pass_inc[] = {8, 8, 4, 4, 2, 2, 1};
int FARDATA png_pass_ystart[] = {0, 0, 4, 0, 2, 0, 1};
int FARDATA png_pass_yinc[] = {8, 8, 8, 4, 4, 2, 2};
#ifdef PNG_HAVE_ASSEMBLER_COMBINE_ROW
int FARDATA png_pass_width[] = {8, 4, 4, 2, 2, 1, 1};
#endif
int FARDATA png_pass_mask[] = {0x80, 0x08, 0x88, 0x22, 0xaa, 0x55, 0xff};
int FARDATA png_pass_dsp_mask[] = {0xff, 0x0f, 0xff, 0x33, 0xff, 0x55, 0xff};

#endif

voidpf
png_zalloc(voidpf png_ptr, uInt items, uInt size)
{
   png_uint_32 num_bytes = (png_uint_32)items * size;
   png_voidp ptr = (png_voidp)png_malloc((png_structp)png_ptr, num_bytes);

   if (num_bytes > (png_uint_32)0x8000L)
   {
      png_memset(ptr, 0, (png_size_t)0x8000L);
      png_memset((png_bytep)ptr + (png_size_t)0x8000L, 0,
         (png_size_t)(num_bytes - (png_uint_32)0x8000L));
   }
   else
   {
      png_memset(ptr, 0, (png_size_t)num_bytes);
   }
   return ((voidpf)ptr);
}

void
png_zfree(voidpf png_ptr, voidpf ptr)
{
   png_free((png_structp)png_ptr, (png_voidp)ptr);
}

void
png_reset_crc(png_structp png_ptr)
{
   png_ptr->crc = crc32(0, Z_NULL, 0);
}

void
png_calculate_crc(png_structp png_ptr, png_bytep ptr, png_size_t length)
{
   int need_crc = 1;

   if (png_ptr->chunk_name[0] & 0x20)                     /* ancillary */
   {
      if ((png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_MASK) ==
          (PNG_FLAG_CRC_ANCILLARY_USE | PNG_FLAG_CRC_ANCILLARY_NOWARN))
         need_crc = 0;
   }
   else                                                    /* critical */
   {
      if (png_ptr->flags & PNG_FLAG_CRC_CRITICAL_IGNORE)
         need_crc = 0;
   }

   if (need_crc)
      png_ptr->crc = crc32(png_ptr->crc, ptr, (uInt)length);
}

png_infop
png_create_info_struct(png_structp png_ptr)
{
   png_infop info_ptr;

   png_debug(1, "in png_create_info_struct\n");
   if(png_ptr == NULL) return (NULL);
   if ((info_ptr = (png_infop)png_create_struct(PNG_STRUCT_INFO)) != NULL)
   {
      png_info_init(info_ptr);
   }

   return (info_ptr);
}

void
png_info_init(png_infop info_ptr)
{
   png_debug(1, "in png_info_init\n");
   png_memset(info_ptr, 0, sizeof (png_info));
}

void
png_free_data(png_structp png_ptr, png_infop info_ptr, png_uint_32 mask, int num)
{
   if (png_ptr == NULL || info_ptr == NULL)
      return;

#if defined(PNG_tRNS_SUPPORTED)
if (mask & PNG_FREE_TRNS)
{
   if (info_ptr->valid & PNG_INFO_tRNS)
   {
       if (info_ptr->free_me & PNG_FREE_TRNS)
         png_free(png_ptr, info_ptr->trans);
       info_ptr->valid &= ~PNG_INFO_tRNS;
   }
}
#endif

if (mask & PNG_FREE_PLTE)
{
   if (info_ptr->valid & PNG_INFO_PLTE)
   {
       if (info_ptr->free_me & PNG_FREE_PLTE)
          png_zfree(png_ptr, info_ptr->palette);
       info_ptr->valid &= ~(PNG_INFO_PLTE);
       info_ptr->num_palette = 0;
   }
}

#if defined(PNG_INFO_IMAGE_SUPPORTED)
if (mask & PNG_FREE_ROWS)
{
   if (info_ptr->free_me & PNG_FREE_ROWS)
   {
       int row;

       for (row = 0; row < (int)info_ptr->height; row++)
          png_free(png_ptr, info_ptr->row_pointers[row]);
       png_free(png_ptr, info_ptr->row_pointers);
   }
}
#endif
   if(num == -1)
     info_ptr->free_me &= ~mask;
}

png_voidp
png_get_io_ptr(png_structp png_ptr)
{
   return (png_ptr->io_ptr);
}

png_charp
png_get_header_ver(png_structp png_ptr)
{
   /* Version of *.h files used when building libpng */
   if(png_ptr != NULL) /* silence compiler warning about unused png_ptr */
      return(PNG_LIBPNG_VER_STRING);
   return(PNG_LIBPNG_VER_STRING);
}
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
