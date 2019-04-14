/********************************
*      GD Graphics Library      *
********************************/

/* This file contains code from Thomas Boutell's gd library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifndef HAVE_GD
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"

int gdPutBuf(const void *buf, int size, gdIOCtx* ctx)
{
	return (ctx->putBuf)(ctx, buf, size);
}
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
