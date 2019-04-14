/********************************
*      GD Graphics Library      *
********************************/

/* This file contains code from Thomas Boutell's gd library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifndef HAVE_GD

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"

typedef struct fileIOCtx {
  gdIOCtx	ctx;
  FILE		*f;
} fileIOCtx;

struct fileIOCtx 	*fileIOCtxPtr;

gdIOCtx* newFileCtx(FILE *f);

static int fileGetbuf( gdIOCtx*, void *, int );
static int filePutbuf( gdIOCtx*, const void *, int );
static void filePutchar( gdIOCtx*, int );
static int fileGetchar( gdIOCtx* ctx);

static int fileSeek(struct gdIOCtx*, const int);
static long fileTell(struct gdIOCtx*);
static void freeFileCtx(gdIOCtx *ctx);

gdIOCtx* gdNewFileCtx (FILE *f) {
  fileIOCtx 	*ctx;

  ctx = (fileIOCtx*) malloc(sizeof(fileIOCtx));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->f = f;

  ctx->ctx.getC = fileGetchar;
  ctx->ctx.putC = filePutchar;

  ctx->ctx.getBuf = fileGetbuf;
  ctx->ctx.putBuf = filePutbuf;

  ctx->ctx.tell = fileTell;
  ctx->ctx.seek = fileSeek;

  ctx->ctx.free = freeFileCtx;

  return (gdIOCtx*)ctx;
}

static 
void freeFileCtx(gdIOCtx *ctx)
{
  free(ctx);
}


static int 
filePutbuf( gdIOCtx* ctx, const void *buf, int size )
{
  fileIOCtx  *fctx;
  fctx = (fileIOCtx*) ctx;

  return fwrite(buf, 1, size, fctx->f);

}

static int
fileGetbuf( gdIOCtx* ctx, void *buf, int size )
{
  fileIOCtx  *fctx;
  fctx = (fileIOCtx*) ctx;

  return (fread(buf, 1, size, fctx->f));

}

static void
filePutchar( gdIOCtx* ctx, int a )
{
  unsigned char b;
  fileIOCtx  *fctx;
  fctx = (fileIOCtx*) ctx;

  b = a;

  putc(b, fctx->f);
}

static int fileGetchar( gdIOCtx* ctx)
{
  fileIOCtx  *fctx;
  fctx = (fileIOCtx*) ctx;

  return getc(fctx->f);
}


static int fileSeek(struct gdIOCtx* ctx, const int pos)
{
  fileIOCtx  *fctx;
  fctx = (fileIOCtx*) ctx;

  return (fseek(fctx->f, pos, SEEK_SET) == 0);
}

static long fileTell(struct gdIOCtx* ctx)
{
  fileIOCtx  *fctx;
  fctx = (fileIOCtx*) ctx;

  return ftell(fctx->f);
}
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
