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

#if defined(PNG_READ_PACK_SUPPORTED) || defined(PNG_WRITE_PACK_SUPPORTED)
void
png_set_packing(png_structp png_ptr)
{
   png_debug(1, "in png_set_packing\n");
   if (png_ptr->bit_depth < 8)
   {
      png_ptr->transformations |= PNG_PACK;
      png_ptr->usr_bit_depth = 8;
   }
}
#endif

#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
