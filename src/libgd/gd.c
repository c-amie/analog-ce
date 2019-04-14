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
#include "../zlib/zlib.h"
#include "gd.h"

extern int gdCosT[];
extern int gdSinT[];

gdImagePtr gdImageCreate(int sx, int sy)
{
	int i;
	gdImagePtr im;
	im = (gdImage *) malloc(sizeof(gdImage));
	im->pixels = (unsigned char **) malloc(sizeof(unsigned char *) * sy);
	im->polyInts = 0;
	im->polyAllocated = 0;
	im->brush = 0;
	im->tile = 0;
	im->style = 0;
	for (i=0; (i<sy); i++) {
		im->pixels[i] = (unsigned char *) calloc(
			sx, sizeof(unsigned char));
	}	
	im->sx = sx;
	im->sy = sy;
	im->colorsTotal = 0;
	im->transparent = (-1);
	im->interlace = 0;

        for (i=0; (i < gdMaxColors); i++) {
           im->open[i] = 1;
	   im->red[i] = 0;
           im->green[i] = 0;
           im->blue[i] = 0;
	};

	return im;
}

void gdImageDestroy(gdImagePtr im)
{
	int i;
	for (i=0; (i<im->sy); i++) {
		free(im->pixels[i]);
	}	
	free(im->pixels);
	if (im->polyInts) {
			free(im->polyInts);
	}
	if (im->style) {
		free(im->style);
	}
	free(im);
}

int gdImageColorAllocate(gdImagePtr im, int r, int g, int b)
{
	int i;
	int ct = (-1);
	for (i=0; (i<(im->colorsTotal)); i++) {
		if (im->open[i]) {
			ct = i;
			break;
		}
	}	
	if (ct == (-1)) {
		ct = im->colorsTotal;
		if (ct == gdMaxColors) {
			return -1;
		}
		im->colorsTotal++;
	}
	im->red[ct] = r;
	im->green[ct] = g;
	im->blue[ct] = b;
	im->open[ct] = 0;
	return ct;
}

void gdImageColorDeallocate(gdImagePtr im, int color)
{
	im->open[color] = 1;
}

void gdImageColorTransparent(gdImagePtr im, int color)
{
	im->transparent = color;
}

void gdImageSetPixel(gdImagePtr im, int x, int y, int color)
{
  /* all other cases removed for analog S.R.E.T. */
  if (gdImageBoundsSafe(im, x, y))
    im->pixels[y][x] = color;
}

int gdImageGetPixel(gdImagePtr im, int x, int y)
{
	if (gdImageBoundsSafe(im, x, y)) {
		return im->pixels[y][x];
	} else {
		return 0;
	}
}

void gdImageLine(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;
	dx = abs(x2-x1);
	dy = abs(y2-y1);
	if (dy <= dx) {
		d = 2*dy - dx;
		incr1 = 2*dy;
		incr2 = 2 * (dy - dx);
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}
		gdImageSetPixel(im, x, y, color);
		if (((y2 - y1) * ydirflag) > 0) {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y++;
					d+=incr2;
				}
				gdImageSetPixel(im, x, y, color);
			}
		} else {
			while (x < xend) {
				x++;
				if (d <0) {
					d+=incr1;
				} else {
					y--;
					d+=incr2;
				}
				gdImageSetPixel(im, x, y, color);
			}
		}		
	} else {
		d = 2*dx - dy;
		incr1 = 2*dx;
		incr2 = 2 * (dx - dy);
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}
		gdImageSetPixel(im, x, y, color);
		if (((x2 - x1) * xdirflag) > 0) {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x++;
					d+=incr2;
				}
				gdImageSetPixel(im, x, y, color);
			}
		} else {
			while (y < yend) {
				y++;
				if (d <0) {
					d+=incr1;
				} else {
					x--;
					d+=incr2;
				}
				gdImageSetPixel(im, x, y, color);
			}
		}
	}
}

int gdImageBoundsSafe(gdImagePtr im, int x, int y)
{
	return (!(((y < 0) || (y >= im->sy)) ||
		((x < 0) || (x >= im->sx))));
}

void gdImageChar(gdImagePtr im, gdFontPtr f, int x, int y, 
	int c, int color)
{
	int cx, cy;
	int px, py;
	int fline;
	cx = 0;
	cy = 0;
	if ((c < f->offset) || (c >= (f->offset + f->nchars))) {
		return;
	}
	fline = (c - f->offset) * f->h * f->w;
	for (py = y; (py < (y + f->h)); py++) {
		for (px = x; (px < (x + f->w)); px++) {
			if (f->data[fline + cy * f->w + cx]) {
				gdImageSetPixel(im, px, py, color);	
			}
			cx++;
		}
		cx = 0;
		cy++;
	}
}

void gdImageString(gdImagePtr im, gdFontPtr f, 
	int x, int y, unsigned char *s, int color)
{
	int i;
	int l;
	l = strlen((char *) s);
	for (i=0; (i<l); i++) {
		gdImageChar(im, f, x, y, s[i], color);
		x += f->w;
	}
}

void gdImageArc(gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color)
{
	int i;
	int lx = 0, ly = 0;
	int w2, h2;
	w2 = w/2;
	h2 = h/2;
	while (e < s) {
		e += 360;
	}
	for (i=s; (i <= e); i++) {
		int x, y;
		x = ((long)gdCosT[i % 360] * (long)w2 / 1024) + cx; 
		y = ((long)gdSinT[i % 360] * (long)h2 / 1024) + cy;
		if (i != s) {
			gdImageLine(im, lx, ly, x, y, color);	
		}
		lx = x;
		ly = y;
	}
}

void gdImageFill(gdImagePtr im, int x, int y, int color)
{
	int lastBorder;
	int old;
	int leftLimit, rightLimit;
	int i;
	old = gdImageGetPixel(im, x, y);
	if (color == gdTiled) {
		int p, tileColor;	
		int srcx, srcy;
		if (!im->tile) {
			return;
		}
		if (gdImageGetTransparent(im->tile) != (-1)) {
			return;
		}	
		srcx = x % gdImageSX(im->tile);
		srcy = y % gdImageSY(im->tile);
		p = gdImageGetPixel(im->tile, srcx, srcy);
		tileColor = im->tileColorMap[p];
		if (old == tileColor) {
			return;
		}
	} else {
		if (old == color) {
			return;
		}
	}
	leftLimit = (-1);
	for (i = x; (i >= 0); i--) {
		if (gdImageGetPixel(im, i, y) != old) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		leftLimit = i;
	}
	if (leftLimit == (-1)) {
		return;
	}
	rightLimit = x;
	for (i = (x+1); (i < im->sx); i++) {	
		if (gdImageGetPixel(im, i, y) != old) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		rightLimit = i;
	}
	if (y > 0) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = gdImageGetPixel(im, i, y-1);
			if (lastBorder) {
				if (c == old) {	
					gdImageFill(im, i, y-1, color);		
					lastBorder = 0;
				}
			} else if (c != old) {
				lastBorder = 1;
			}
		}
	}
	if (y < ((im->sy) - 1)) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = gdImageGetPixel(im, i, y+1);
			if (lastBorder) {
				if (c == old) {
					gdImageFill(im, i, y+1, color);		
					lastBorder = 0;
				}
			} else if (c != old) {
				lastBorder = 1;
			}
		}
	}
}

void gdImageRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	gdImageLine(im, x1, y1, x2, y1, color);		
	gdImageLine(im, x1, y2, x2, y2, color);		
	gdImageLine(im, x1, y1, x1, y2, color);
	gdImageLine(im, x2, y1, x2, y2, color);
}

void gdImageFilledRectangle(gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int x, y;
	for (y=y1; (y<=y2); y++) {
		for (x=x1; (x<=x2); x++) {
			gdImageSetPixel(im, x, y, color);
		}
	}
}
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
