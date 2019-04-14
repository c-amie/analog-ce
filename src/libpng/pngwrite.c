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
png_write_info_before_PLTE(png_structp png_ptr, png_infop info_ptr)
{
   png_debug(1, "in png_write_info_before_PLTE\n");
   if (!(png_ptr->mode & PNG_WROTE_INFO_BEFORE_PLTE))
   {
   png_write_sig(png_ptr);

   png_write_IHDR(png_ptr, info_ptr->width, info_ptr->height,
      info_ptr->bit_depth, info_ptr->color_type, info_ptr->compression_type,
      info_ptr->filter_type,
      0);
      png_ptr->mode |= PNG_WROTE_INFO_BEFORE_PLTE;
   }
}

void
png_write_info(png_structp png_ptr, png_infop info_ptr)
{
   png_debug(1, "in png_write_info\n");

   png_write_info_before_PLTE(png_ptr, info_ptr);

   if (info_ptr->valid & PNG_INFO_PLTE)
      png_write_PLTE(png_ptr, info_ptr->palette,
         (png_uint_32)info_ptr->num_palette);
   else if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_error(png_ptr, "Valid palette required for paletted images\n");

#if defined(PNG_WRITE_tRNS_SUPPORTED)
   if (info_ptr->valid & PNG_INFO_tRNS)
      {
      png_write_tRNS(png_ptr, info_ptr->trans, &(info_ptr->trans_values),
         info_ptr->num_trans, info_ptr->color_type);
      }
#endif
}

void
png_write_end(png_structp png_ptr, png_infop info_ptr)
{
   png_debug(1, "in png_write_end\n");
   if (!(png_ptr->mode & PNG_HAVE_IDAT))
      png_error(png_ptr, "No IDATs written into file");

   png_ptr->mode |= PNG_AFTER_IDAT;
   png_write_IEND(png_ptr);
}


png_structp
png_create_write_struct(png_const_charp user_png_ver, png_voidp error_ptr,
   png_error_ptr error_fn, png_error_ptr warn_fn)
{
   png_structp png_ptr;
#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
   jmp_buf jmpbuf;
#endif
#endif
   png_debug(1, "in png_create_write_struct\n");
   if ((png_ptr = (png_structp)png_create_struct(PNG_STRUCT_PNG)) == NULL)
   {
      return ((png_structp)NULL);
   }

#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
   if (setjmp(jmpbuf))
#else
   if (setjmp(png_ptr->jmpbuf))
#endif
   {
      png_free(png_ptr, png_ptr->zbuf);
      png_destroy_struct(png_ptr);
      return ((png_structp)NULL);
   }
#ifdef USE_FAR_KEYWORD
   png_memcpy(png_ptr->jmpbuf,jmpbuf,sizeof(jmp_buf));
#endif
#endif

   png_set_error_fn(png_ptr, error_ptr, error_fn, warn_fn);

   if (user_png_ver == NULL || user_png_ver[0] != png_libpng_ver[0] ||
       (png_libpng_ver[0] == '0' && user_png_ver[2] < '9'))
   {
      png_error(png_ptr,
         "Incompatible libpng version in application and library");
   }

   png_ptr->zbuf_size = PNG_ZBUF_SIZE;
   png_ptr->zbuf = (png_bytep)png_malloc(png_ptr,
      (png_uint_32)png_ptr->zbuf_size);

   png_set_write_fn(png_ptr, NULL, NULL, NULL);

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   png_set_filter_heuristics(png_ptr, PNG_FILTER_HEURISTIC_DEFAULT,
      1, NULL, NULL);
#endif
   return ((png_structp)png_ptr);
}

void
png_write_image(png_structp png_ptr, png_bytepp image)
{
   png_uint_32 i;
   int pass, num_pass;
   png_bytepp rp;

   png_debug(1, "in png_write_image\n");
   num_pass = 1;

   for (pass = 0; pass < num_pass; pass++)
   {
      for (i = 0, rp = image; i < png_ptr->height; i++, rp++)
      {
         png_write_row(png_ptr, *rp);
      }
   }
}

void
png_write_row(png_structp png_ptr, png_bytep row)
{
   png_debug2(1, "in png_write_row (row %ld, pass %d)\n",
      png_ptr->row_number, png_ptr->pass);

   if (png_ptr->row_number == 0 && png_ptr->pass == 0)
   {
      png_write_start_row(png_ptr);
   }

   png_ptr->row_info.color_type = png_ptr->color_type;
   png_ptr->row_info.width = png_ptr->usr_width;
   png_ptr->row_info.channels = png_ptr->usr_channels;
   png_ptr->row_info.bit_depth = png_ptr->usr_bit_depth;
   png_ptr->row_info.pixel_depth = (png_byte)(png_ptr->row_info.bit_depth *
      png_ptr->row_info.channels);
   png_ptr->row_info.rowbytes = ((png_ptr->row_info.width *
      (png_uint_32)png_ptr->row_info.pixel_depth + 7) >> 3);
   png_debug1(3, "row_info->color_type = %d\n", png_ptr->row_info.color_type);
   png_debug1(3, "row_info->width = %d\n", png_ptr->row_info.width);
   png_debug1(3, "row_info->channels = %d\n", png_ptr->row_info.channels);
   png_debug1(3, "row_info->bit_depth = %d\n", png_ptr->row_info.bit_depth);
   png_debug1(3, "row_info->pixel_depth = %d\n", png_ptr->row_info.pixel_depth);
   png_debug1(3, "row_info->rowbytes = %d\n", png_ptr->row_info.rowbytes);

   png_memcpy_check(png_ptr, png_ptr->row_buf + 1, row,
      png_ptr->row_info.rowbytes);
   if (png_ptr->transformations)
      png_do_write_transformations(png_ptr);
   png_write_find_filter(png_ptr, &(png_ptr->row_info));

   if (png_ptr->write_row_fn != NULL)
      (*(png_ptr->write_row_fn))(png_ptr, png_ptr->row_number, png_ptr->pass);
}

#if defined(PNG_WRITE_FLUSH_SUPPORTED)
void
png_write_flush(png_structp png_ptr)
{
   int wrote_IDAT;

   png_debug(1, "in png_write_flush\n");
   if (png_ptr->row_number >= png_ptr->num_rows)
     return;

   do
   {
      int ret;

      ret = deflate(&png_ptr->zstream, Z_SYNC_FLUSH);
      wrote_IDAT = 0;
      if (ret != Z_OK)
      {
         if (png_ptr->zstream.msg != NULL)
            png_error(png_ptr, png_ptr->zstream.msg);
         else
            png_error(png_ptr, "zlib error");
      }
      if (!(png_ptr->zstream.avail_out))
      {
         png_write_IDAT(png_ptr, png_ptr->zbuf,
                        png_ptr->zbuf_size);
         png_ptr->zstream.next_out = png_ptr->zbuf;
         png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
         wrote_IDAT = 1;
      }
   } while(wrote_IDAT == 1);

   if (png_ptr->zbuf_size != png_ptr->zstream.avail_out)
   {
      png_write_IDAT(png_ptr, png_ptr->zbuf,
                     png_ptr->zbuf_size - png_ptr->zstream.avail_out);
      png_ptr->zstream.next_out = png_ptr->zbuf;
      png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
   }
   png_ptr->flush_rows = 0;
   png_flush(png_ptr);
}
#endif /* PNG_WRITE_FLUSH_SUPPORTED */

void
png_destroy_write_struct(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr)
{
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;

   png_debug(1, "in png_destroy_write_struct\n");
   if (png_ptr_ptr != NULL)
   {
      png_ptr = *png_ptr_ptr;
   }

   if (info_ptr_ptr != NULL)
      info_ptr = *info_ptr_ptr;

   if (info_ptr != NULL)
   {
      png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);

      png_destroy_struct((png_voidp)info_ptr);
      *info_ptr_ptr = (png_infop)NULL;
   }

   if (png_ptr != NULL)
   {
      png_write_destroy(png_ptr);
      png_destroy_struct((png_voidp)png_ptr);
      *png_ptr_ptr = (png_structp)NULL;
   }
}

void
png_write_destroy(png_structp png_ptr)
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf tmp_jmp;
#endif
   png_error_ptr error_fn;
   png_error_ptr warning_fn;
   png_voidp error_ptr;

   png_debug(1, "in png_write_destroy\n");
   deflateEnd(&png_ptr->zstream);
   png_free(png_ptr, png_ptr->zbuf);
   png_free(png_ptr, png_ptr->row_buf);
   png_free(png_ptr, png_ptr->prev_row);
   png_free(png_ptr, png_ptr->sub_row);
   png_free(png_ptr, png_ptr->up_row);
   png_free(png_ptr, png_ptr->avg_row);
   png_free(png_ptr, png_ptr->paeth_row);
#if defined(PNG_TIME_RFC1123_SUPPORTED)
   png_free(png_ptr, png_ptr->time_buffer);
#endif

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   png_free(png_ptr, png_ptr->prev_filters);
   png_free(png_ptr, png_ptr->filter_weights);
   png_free(png_ptr, png_ptr->inv_filter_weights);
   png_free(png_ptr, png_ptr->filter_costs);
   png_free(png_ptr, png_ptr->inv_filter_costs);
#endif
#ifdef PNG_SETJMP_SUPPORTED
   png_memcpy(tmp_jmp, png_ptr->jmpbuf, sizeof (jmp_buf));
#endif
   error_fn = png_ptr->error_fn;
   warning_fn = png_ptr->warning_fn;
   error_ptr = png_ptr->error_ptr;
   png_memset(png_ptr, 0, sizeof (png_struct));
   png_ptr->error_fn = error_fn;
   png_ptr->warning_fn = warning_fn;
   png_ptr->error_ptr = error_ptr;
#ifdef PNG_SETJMP_SUPPORTED
   png_memcpy(png_ptr->jmpbuf, tmp_jmp, sizeof (jmp_buf));
#endif
}

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
void
png_set_filter_heuristics(png_structp png_ptr, int heuristic_method,
   int num_weights, png_doublep filter_weights,
   png_doublep filter_costs)
{
   int i;

   png_debug(1, "in png_set_filter_heuristics\n");
   if (heuristic_method >= PNG_FILTER_HEURISTIC_LAST)
   {
      png_warning(png_ptr, "Unknown filter heuristic method");
      return;
   }
   if (heuristic_method == PNG_FILTER_HEURISTIC_DEFAULT)
   {
      heuristic_method = PNG_FILTER_HEURISTIC_UNWEIGHTED;
   }
   if (num_weights < 0 || filter_weights == NULL ||
      heuristic_method == PNG_FILTER_HEURISTIC_UNWEIGHTED)
   {
      num_weights = 0;
   }

   png_ptr->num_prev_filters = (png_byte)num_weights;
   png_ptr->heuristic_method = (png_byte)heuristic_method;

   if (num_weights > 0)
   {
      if (png_ptr->prev_filters == NULL)
      {
         png_ptr->prev_filters = (png_bytep)png_malloc(png_ptr,
            (png_uint_32)(sizeof(png_byte) * num_weights));


         for (i = 0; i < num_weights; i++)
         {
            png_ptr->prev_filters[i] = 255;
         }
      }

      if (png_ptr->filter_weights == NULL)
      {
         png_ptr->filter_weights = (png_uint_16p) png_malloc(png_ptr,
            (png_uint_32)(sizeof(png_uint_16) * num_weights));

         png_ptr->inv_filter_weights = (png_uint_16p) png_malloc(png_ptr,
            (png_uint_32)(sizeof(png_uint_16) * num_weights));

         for (i = 0; i < num_weights; i++)
         {
            png_ptr->inv_filter_weights[i] =
            png_ptr->filter_weights[i] = PNG_WEIGHT_FACTOR;
         }
      }

      for (i = 0; i < num_weights; i++)
      {
         if (filter_weights[i] < 0.0)
         {
            png_ptr->inv_filter_weights[i] =
            png_ptr->filter_weights[i] = PNG_WEIGHT_FACTOR;
         }
         else
         {
            png_ptr->inv_filter_weights[i] =
               (png_uint_16)((double)PNG_WEIGHT_FACTOR*filter_weights[i]+0.5);
            png_ptr->filter_weights[i] =
               (png_uint_16)((double)PNG_WEIGHT_FACTOR/filter_weights[i]+0.5);
         }
      }
   }

   if (png_ptr->filter_costs == NULL)
   {
      png_ptr->filter_costs = (png_uint_16p) png_malloc(png_ptr,
         (png_uint_32)(sizeof(png_uint_16) * PNG_FILTER_VALUE_LAST));

      png_ptr->inv_filter_costs = (png_uint_16p) png_malloc(png_ptr,
         (png_uint_32)(sizeof(png_uint_16) * PNG_FILTER_VALUE_LAST));

      for (i = 0; i < PNG_FILTER_VALUE_LAST; i++)
      {
         png_ptr->inv_filter_costs[i] =
         png_ptr->filter_costs[i] = PNG_COST_FACTOR;
      }
   }

   for (i = 0; i < PNG_FILTER_VALUE_LAST; i++)
   {
      if (filter_costs == NULL || filter_costs[i] < 0.0)
      {
         png_ptr->inv_filter_costs[i] =
         png_ptr->filter_costs[i] = PNG_COST_FACTOR;
      }
      else if (filter_costs[i] >= 1.0)
      {
         png_ptr->inv_filter_costs[i] =
            (png_uint_16)((double)PNG_COST_FACTOR / filter_costs[i] + 0.5);
         png_ptr->filter_costs[i] =
            (png_uint_16)((double)PNG_COST_FACTOR * filter_costs[i] + 0.5);
      }
   }
}
#endif  /* PNG_WRITE_WEIGHTED_FILTER_SUPPORTED */

#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
