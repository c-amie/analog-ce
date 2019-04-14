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

void
png_set_IHDR(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 width, png_uint_32 height, int bit_depth,
   int color_type, int interlace_type, int compression_type,
   int filter_type)
{
   int rowbytes_per_pixel;
   png_debug1(1, "in %s storage function\n", "IHDR");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->width = width;
   info_ptr->height = height;
   info_ptr->bit_depth = (png_byte)bit_depth;
   info_ptr->color_type =(png_byte) color_type;
   info_ptr->compression_type = (png_byte)compression_type;
   info_ptr->filter_type = (png_byte)filter_type;
   info_ptr->interlace_type = (png_byte)interlace_type;
   if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      info_ptr->channels = 1;
   else if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
      info_ptr->channels = 3;
   else
      info_ptr->channels = 1;
   if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
      info_ptr->channels++;
   info_ptr->pixel_depth = (png_byte)(info_ptr->channels * info_ptr->bit_depth);

   rowbytes_per_pixel = (info_ptr->pixel_depth + 7) >> 3;
   if (( width > PNG_MAX_UINT/rowbytes_per_pixel))
   {
      png_warning(png_ptr,
         "Width too large to process image data; rowbytes will overflow.");
      info_ptr->rowbytes = (png_size_t)0;
   }
   else
      info_ptr->rowbytes = (info_ptr->width * info_ptr->pixel_depth + 7) >> 3;
}

void
png_set_PLTE(png_structp png_ptr, png_infop info_ptr,
   png_colorp palette, int num_palette)
{

   png_debug1(1, "in %s storage function\n", "PLTE");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->palette = palette;

   info_ptr->num_palette = (png_uint_16)num_palette;
   info_ptr->valid |= PNG_INFO_PLTE;
}

#if defined(PNG_tRNS_SUPPORTED)
void
png_set_tRNS(png_structp png_ptr, png_infop info_ptr,
   png_bytep trans, int num_trans, png_color_16p trans_values)
{
   png_debug1(1, "in %s storage function\n", "tRNS");
   if (png_ptr == NULL || info_ptr == NULL)
      return;

   if (trans != NULL)
      info_ptr->trans = trans;

   if (trans_values != NULL)
   {
      png_memcpy(&(info_ptr->trans_values), trans_values,
         sizeof(png_color_16));
      if (num_trans == 0)
        num_trans = 1;
   }
   info_ptr->num_trans = (png_uint_16)num_trans;
   info_ptr->valid |= PNG_INFO_tRNS;
}
#endif

#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
