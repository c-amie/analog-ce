#ifndef NOGRAPHICS
#ifdef HAVE_GD
#include <gd_io.h>
#else
#ifndef GD_IO_H
#define GD_IO_H 1

#include <stdio.h>
 
typedef struct gdIOCtx {
	int	(*getC)(struct gdIOCtx*);
	int	(*getBuf)(struct gdIOCtx*, void*, int);

        void     (*putC)(struct gdIOCtx*, int);
	int	(*putBuf)(struct gdIOCtx*, const void*, int);

	int	(*seek)(struct gdIOCtx*, const int);
	long	(*tell)(struct gdIOCtx*);

	void    (*free)(struct gdIOCtx*);

} gdIOCtx;

typedef struct gdIOCtx	*gdIOCtxPtr;

int gdPutBuf(const void *, int, gdIOCtx*);

#endif  /* GD_IO_H */
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
