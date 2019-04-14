#ifndef NOGRAPHICS
#ifdef HAVE_GD
#include <gdfonts.h>
#else
#ifndef _GDFONTS_H_
#define _GDFONTS_H_ 1

#include "gd.h"

extern gdFontPtr gdFontSmall;

#endif
#endif
#endif
