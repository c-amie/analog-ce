/********************************
*      GD Graphics Library      *
********************************/

/* This file contains code from Thomas Boutell's gd library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifdef HAVE_GD
#include <gd.h>
#else
#ifndef GD_H
#define GD_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "gd_io.h"

#define gdMaxColors 256

typedef struct gdImageStruct {
	unsigned char ** pixels;
	int sx;
	int sy;
	int colorsTotal;
	int red[gdMaxColors];
	int green[gdMaxColors];
	int blue[gdMaxColors]; 
	int open[gdMaxColors];
	int transparent;
	int *polyInts;
	int polyAllocated;
	struct gdImageStruct *brush;
	struct gdImageStruct *tile;	
	int brushColorMap[gdMaxColors];
	int tileColorMap[gdMaxColors];
	int styleLength;
	int stylePos;
	int *style;
	int interlace;
} gdImage;

typedef gdImage * gdImagePtr;

typedef struct {
	int nchars;
	int offset;
	int w;
	int h;
	char *data;
} gdFont;

typedef gdFont *gdFontPtr;
#define gdDashSize 4

#define gdStyled (-2)
#define gdBrushed (-3)
#define gdStyledBrushed (-4)
#define gdTiled (-5)
#define gdTransparent (-6)

gdImagePtr gdImageCreate(int sx, int sy);
void gdImageDestroy(gdImagePtr im);
void gdImageSetPixel(gdImagePtr im, int x, int y, int color);
int gdImageGetPixel(gdImagePtr im, int x, int y);
void gdImageLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
void gdImageRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
void gdImageFilledRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
int gdImageBoundsSafe(gdImagePtr im, int x, int y);
void gdImageChar(gdImagePtr im, gdFontPtr f, int x, int y, int c, int color);
void gdImageString(gdImagePtr im, gdFontPtr f, int x, int y, unsigned char *s, int color);

int gdImageColorAllocate(gdImagePtr im, int r, int g, int b);
void gdImageColorDeallocate(gdImagePtr im, int color);
void gdImageColorTransparent(gdImagePtr im, int color);
void gdImagePng(gdImagePtr im, FILE *out);
void gdImagePngCtx(gdImagePtr im, gdIOCtx *out);
void gdImageArc(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color);
void gdImageFill(gdImagePtr im, int x, int y, int color);
void gdImageInterlace(gdImagePtr im, int interlaceArg);
#define gdImageSX(im) ((im)->sx)
#define gdImageSY(im) ((im)->sy)
#define gdImageColorsTotal(im) ((im)->colorsTotal)
#define gdImageRed(im, c) ((im)->red[(c)])
#define gdImageGreen(im, c) ((im)->green[(c)])
#define gdImageBlue(im, c) ((im)->blue[(c)])
#define gdImageGetTransparent(im) ((im)->transparent)
#define gdImageGetInterlaced(im) ((im)->interlace)

gdIOCtx* gdNewFileCtx(FILE*);
gdIOCtx* gdNewDynamicCtx(int, void*);

#ifdef __cplusplus
}
#endif

#endif  /* GD_H */
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
