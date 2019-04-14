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

/* This paragraph added for analog by Jason Linhart */
#ifdef MACOS
extern void *mac_png_malloc(unsigned long size);
extern void mac_png_free(void *ptr);
#define malloc(size)            mac_png_malloc(size)
#define free(ptr)                       mac_png_free(ptr)
#endif

#if defined(__TURBOC__) && !defined(_Windows) && !defined(__FLAT__)

png_voidp
png_create_struct(int type)
{
   png_size_t size;
   png_voidp struct_ptr;

   if (type == PNG_STRUCT_INFO)
     size = sizeof(png_info);
   else if (type == PNG_STRUCT_PNG)
     size = sizeof(png_struct);
   else
     return ((png_voidp)NULL);

   if ((struct_ptr = (png_voidp)farmalloc(size)) != NULL)
   {
      png_memset(struct_ptr, 0, size);
   }
   return (struct_ptr);
}


void
png_destroy_struct(png_voidp struct_ptr)
{
   if (struct_ptr != NULL)
   {
      farfree (struct_ptr);
   }
}

png_voidp
png_malloc(png_structp png_ptr, png_uint_32 size)
{
   png_voidp ret;
   if (png_ptr == NULL || size == 0)
      return ((png_voidp)NULL);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      png_error(png_ptr, "Cannot Allocate > 64K");
#endif

   if (size == (png_uint_32)65536L)
   {
      if (png_ptr->offset_table == NULL)
      {
         ret = farmalloc(size);
         if (ret == NULL || ((png_size_t)ret & 0xffff))
         {
            int num_blocks;
            png_uint_32 total_size;
            png_bytep table;
            int i;
            png_byte huge * hptr;

            if (ret != NULL)
            {
               farfree(ret);
               ret = NULL;
            }

            if(png_ptr->zlib_window_bits > 14)
               num_blocks = (int)(1 << (png_ptr->zlib_window_bits - 14));
            else
               num_blocks = 1;
            if (png_ptr->zlib_mem_level >= 7)
               num_blocks += (int)(1 << (png_ptr->zlib_mem_level - 7));
            else
               num_blocks++;

            total_size = ((png_uint_32)65536L) * (png_uint_32)num_blocks+16;

            table = farmalloc(total_size);

            if (table == NULL)
            {
               png_error(png_ptr, "Out Of Memory.");
            }

            if ((png_size_t)table & 0xfff0)
            {
               png_error(png_ptr, "Farmalloc didn't return normalized pointer");
            }

            png_ptr->offset_table = table;
            png_ptr->offset_table_ptr = farmalloc(num_blocks *
               sizeof (png_bytep));

            if (png_ptr->offset_table_ptr == NULL)
            {
               png_error(png_ptr, "Out Of memory.");
            }

            hptr = (png_byte huge *)table;
            if ((png_size_t)hptr & 0xf)
            {
               hptr = (png_byte huge *)((long)(hptr) & 0xfffffff0L);
               hptr = hptr + 16L;
            }
            for (i = 0; i < num_blocks; i++)
            {
               png_ptr->offset_table_ptr[i] = (png_bytep)hptr;
               hptr = hptr + (png_uint_32)65536L;
            }

            png_ptr->offset_table_number = num_blocks;
            png_ptr->offset_table_count = 0;
            png_ptr->offset_table_count_free = 0;
         }
      }

      if (png_ptr->offset_table_count >= png_ptr->offset_table_number)
         png_error(png_ptr, "Out of Memory.");

      ret = png_ptr->offset_table_ptr[png_ptr->offset_table_count++];
   }
   else
      ret = farmalloc(size);

   if (ret == NULL)
   {
      png_error(png_ptr, "Out of memory.");
   }

   return (ret);
}

void
png_free(png_structp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL || ptr == NULL)
      return;

   if (png_ptr->offset_table != NULL)
   {
      int i;

      for (i = 0; i < png_ptr->offset_table_count; i++)
      {
         if (ptr == png_ptr->offset_table_ptr[i])
         {
            ptr = NULL;
            png_ptr->offset_table_count_free++;
            break;
         }
      }
      if (png_ptr->offset_table_count_free == png_ptr->offset_table_count)
      {
         farfree(png_ptr->offset_table);
         farfree(png_ptr->offset_table_ptr);
         png_ptr->offset_table = NULL;
         png_ptr->offset_table_ptr = NULL;
      }
   }

   if (ptr != NULL)
   {
      farfree(ptr);
   }
}

#else /* Not the Borland DOS special memory handler */

png_voidp
png_create_struct(int type)
{
   png_size_t size;
   png_voidp struct_ptr;

   if (type == PNG_STRUCT_INFO)
      size = sizeof(png_info);
   else if (type == PNG_STRUCT_PNG)
      size = sizeof(png_struct);
   else
      return ((png_voidp)NULL);

#if defined(__TURBOC__) && !defined(__FLAT__)
   if ((struct_ptr = (png_voidp)farmalloc(size)) != NULL)
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
   if ((struct_ptr = (png_voidp)halloc(size,1)) != NULL)
# else
   if ((struct_ptr = (png_voidp)malloc(size)) != NULL)
# endif
#endif
   {
      png_memset(struct_ptr, 0, size);
   }

   return (struct_ptr);
}

void
png_destroy_struct(png_voidp struct_ptr)
{
   if (struct_ptr != NULL)
   {
#if defined(__TURBOC__) && !defined(__FLAT__)
      farfree(struct_ptr);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
      hfree(struct_ptr);
# else
      free(struct_ptr);
# endif
#endif
   }
}

png_voidp
png_malloc(png_structp png_ptr, png_uint_32 size)
{
   png_voidp ret;
   if (png_ptr == NULL || size == 0)
      return ((png_voidp)NULL);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
      png_error(png_ptr, "Cannot Allocate > 64K");
#endif

#if defined(__TURBOC__) && !defined(__FLAT__)
   ret = farmalloc(size);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
   ret = halloc(size, 1);
# else
   ret = malloc((size_t)size);
# endif
#endif

   if (ret == NULL)
   {
      png_error(png_ptr, "Out of Memory");
   }

   return (ret);
}

void
png_free(png_structp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL || ptr == NULL)
      return;

#if defined(__TURBOC__) && !defined(__FLAT__)
   farfree(ptr);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
   hfree(ptr);
# else
   free(ptr);
# endif
#endif
}

#endif /* Not Borland DOS special memory handler */

png_voidp
png_memcpy_check (png_structp png_ptr, png_voidp s1, png_voidp s2,
   png_uint_32 length)
{
   png_size_t size;

   size = (png_size_t)length;
   if ((png_uint_32)size != length)
      png_error(png_ptr,"Overflow in png_memcpy_check.");

   return(png_memcpy (s1, s2, size));
}
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
