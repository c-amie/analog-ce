/********************************
*      GD Graphics Library      *
********************************/

/* This file contains code from Thomas Boutell's gd library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifndef HAVE_GD

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"
#include "../libpng/png.h"

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

typedef struct _jmpbuf_wrapper {
  jmp_buf jmpbuf;
} jmpbuf_wrapper;

static jmpbuf_wrapper gdPngJmpbufStruct;

static void gdPngErrorHandler(png_structp png_ptr, png_const_charp msg)
{
  jmpbuf_wrapper  *jmpbuf_ptr;

  fprintf(stderr, "gd-png:  fatal libpng error: %s\n", msg);
  fflush(stderr);

  jmpbuf_ptr = png_get_error_ptr(png_ptr);
  if (jmpbuf_ptr == NULL) {
    fprintf(stderr,
      "gd-png:  EXTREMELY fatal error: jmpbuf unrecoverable; terminating.\n");
    fflush(stderr);
    exit(99);
  }

  longjmp(jmpbuf_ptr->jmpbuf, 1);
}


static void gdPngWriteData(png_structp png_ptr,
	png_bytep data, png_size_t length)
{
	gdPutBuf(data, length, (gdIOCtx *)
		png_get_io_ptr(png_ptr));
}

static void gdPngFlushData(png_structp png_ptr)
{
}

void gdImagePng(gdImagePtr im, FILE *outFile)
{
	gdIOCtx *out = gdNewFileCtx(outFile);
	gdImagePngCtx(im, out);
	out->free(out);
}

void gdImagePngCtx(gdImagePtr im, gdIOCtx *outfile)
{
    int i, j, bit_depth, interlace_type;
    int width = im->sx;
    int height = im->sy;
    int colors = im->colorsTotal;
    int *open = im->open;
    int mapping[gdMaxColors];
    png_byte trans_value = 0;
    png_color palette[gdMaxColors];
    png_structp png_ptr;
    png_infop info_ptr;
    volatile int transparent = im->transparent;
    volatile int remap = FALSE;


    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
      &gdPngJmpbufStruct, gdPngErrorHandler, NULL);
    if (png_ptr == NULL) {
        fprintf(stderr, "gd-png error: cannot allocate libpng main struct\n");
        return;
    }

    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr, "gd-png error: cannot allocate libpng info struct\n");
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return;
    }

    if (setjmp(gdPngJmpbufStruct.jmpbuf)) {
        fprintf(stderr, "gd-png error: setjmp returns error condition\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_set_write_fn(png_ptr, (void *)outfile, gdPngWriteData, gdPngFlushData);

    if (transparent >= im->colorsTotal ||
       (transparent >= 0 && open[transparent])) 
        transparent = -1;

    for (i = 0;  i < gdMaxColors;  ++i)
        mapping[i] = -1;

    colors = 0;
    for (i = 0;  i < im->colorsTotal;  ++i) {
        if (!open[i]) {
            mapping[i] = colors;
            ++colors;
        }
    }
    if (colors < im->colorsTotal) {
        remap = TRUE;
        if (transparent >= 0)
            transparent = mapping[transparent];
    }

    if (colors <= 2)
        bit_depth = 1;
    else if (colors <= 4)
        bit_depth = 2;
    else if (colors <= 16)
        bit_depth = 4;
    else
        bit_depth = 8;

    interlace_type = im->interlace? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;

    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth,
      PNG_COLOR_TYPE_PALETTE, interlace_type,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (transparent >= 0) {
        if (transparent != 0) {
            if (!remap) {
                remap = TRUE;
                for (i = 0;  i < colors;  ++i)
                    mapping[i] = i;
            }
            mapping[transparent] = 0;
            mapping[0] = transparent;
        }
        png_set_tRNS(png_ptr, info_ptr, &trans_value, 1, NULL);
    }

    if (remap)
        for (i = 0;  i < im->colorsTotal;  ++i) {
            if (mapping[i] < 0)
                continue;
            palette[mapping[i]].red   = im->red[i];
            palette[mapping[i]].green = im->green[i];
            palette[mapping[i]].blue  = im->blue[i];
        }
    else
        for (i = 0;  i < colors;  ++i) {
            palette[i].red   = im->red[i];
            palette[i].green = im->green[i];
            palette[i].blue  = im->blue[i];
        }
    png_set_PLTE(png_ptr, info_ptr, palette, colors);


    png_write_info(png_ptr, info_ptr);

    png_set_packing(png_ptr);

    if (remap) {
        png_bytep *row_pointers;
	row_pointers = malloc(sizeof(png_bytep) * height);
        if (row_pointers == NULL) {
            fprintf(stderr, "gd-png error: unable to allocate row_pointers\n");
        }
        for (j = 0;  j < height;  ++j) {
            if ((row_pointers[j] = (png_bytep)malloc(width)) == NULL) {
                fprintf(stderr, "gd-png error: unable to allocate rows\n");
                for (i = 0;  i < j;  ++i)
                    free(row_pointers[i]);
                return;
            }
            for (i = 0;  i < width;  ++i)
                row_pointers[j][i] = mapping[im->pixels[j][i]];
        }

        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, info_ptr);

        for (j = 0;  j < height;  ++j)
            free(row_pointers[j]);
	free(row_pointers);
    } else {
        png_write_image(png_ptr, im->pixels);
        png_write_end(png_ptr, info_ptr);
    }
        png_destroy_write_struct(&png_ptr, &info_ptr);
}
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
