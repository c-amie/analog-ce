/***             analog 6.0             http://www.analog.cx/             ***/
/*** This program is copyright (c) Stephen R. E. Turner 1995 - 2004 except as
 *** stated otherwise.
 ***
 *** This program is free software. You can redistribute it and/or modify it
 *** under the terms of version 2 of the GNU General Public License, which you
 *** should have received with it.
 ***
 *** This program is distributed in the hope that it will be useful, but
 *** without any warranty, expressed or implied.   ***/

/*** sort.c; sort a hashindex tree into a list by some criterion ***/

#include "anlghea3.h"

#define FINDNAME(p) ((rep >= 0)?(maketreename(partname, (p), newpn, space, need, rep, FALSE)):((p)->name))

/* See commentary in my_sort() below */
void calctotmax(Hashindex **gooditems, Hashindex **baditems, Strlist *partname,
		Strlist **newpn, Strlist *space, size_t need, choice rep,
		Include *wanthead, choice requests, choice requests7,
		choice pages, choice pages7, choice date, choice firstd,
		unsigned long *totr, unsigned long *totr7, unsigned long *totp,
		unsigned long *totp7, double *totb, double *totb7,
		unsigned long *maxr, unsigned long *maxr7, unsigned long *maxp,
		unsigned long *maxp7, double *maxb, double *maxb7,
		timecode_t *maxd, timecode_t *mind, logical donetots,
		logical nosort, Hashindex **ans3) {
  Hashindex *p, *nextp, *goods = NULL;
  logical done;

  if (!donetots) {
    *totr = *totr7 = *totp = *totp7 = *maxr = *maxr7 = *maxp = *maxp7 = 0;
    *totb = *totb7 = *maxb = *maxb7 = 0.;
    *maxd = FIRST_TIME;
    *mind = LAST_TIME;
  }
  if (*gooditems != NULL) {
    p = *gooditems;
    done = FALSE;
  }
  else {
    p = *baditems;
    done = TRUE;
  }

  for ( ; p != NULL; p = nextp) {
    /* Calculate the next p immediately because we will mess with p->next */
    nextp = p->next;
    if (nextp == NULL && !done) {
      nextp = *baditems;
      done = TRUE;
    }

    /* Is this item wanted? */
    if (!nosort &&
	(p->own == NULL || p->own->data[requests] == 0 ||
	 !included(FINDNAME(p), p->own->ispage, wanthead))) {
      /* not wanted; add to ans3 */
      p->next = *ans3;
      *ans3 = p;
    }
    else {
      /* wanted; add to goods, and include in tot and max */
      if (!nosort) {
	p->next = goods;
	goods = p;
      }
      if (!donetots) {
	*totr += p->own->data[requests];
	*maxr = MAX(*maxr, p->own->data[requests]);
	if (pages >= 0) {
	  /* This caution is probably unnecessary: if pages == -1, totp will
	     not be used. But technically, data[-1] is undefined behaviour, and
	     the program is then within its rights to crash. :)
	     'requests' is never -1, by the way: see wantcol[] in init.c. */
	  *totp += p->own->data[pages];
	  *maxp = MAX(*maxp, p->own->data[pages]);
	}
	if (requests7 >= 0) {
	  *totr7 += p->own->data[requests7];
	  *maxr7 = MAX(*maxr7, p->own->data[requests7]);
	}
	if (pages7 >= 0) {
	  *totp7 += p->own->data[pages7];
	  *maxp7 = MAX(*maxp7, p->own->data[pages7]);
	}
	*totb += p->own->bytes;
	*totb7 += p->own->bytes7;
	*maxb = MAX(*maxb, p->own->bytes);
	*maxb7 = MAX(*maxb7, p->own->bytes7);
	if (date >= 0)
	  *maxd = MAX(*maxd, p->own->data[date]);
	if (firstd >= 0 && p->own->data[firstd] != 0)
	  *mind = MIN(*mind, p->own->data[firstd]);
      }  /* !donetots */
    }  /* item wanted */
  }  /* p loop */

  /* Make gooditems into the resultant list of wanted things */
  if (!nosort) {
    *gooditems = goods;
    *baditems = NULL;
  }
}

/* See commentary in my_sort() below */
void calcfloor(unsigned long *min, double *dmin, choice *filterby,
	       Floor *floor, choice requests, choice requests7, choice pages,
	       choice pages7, choice date, choice firstd, unsigned long totr,
	       unsigned long totr7, unsigned long totp, unsigned long totp7,
	       double totb, double totb7, unsigned long maxr,
	       unsigned long maxr7, unsigned long maxp, unsigned long maxp7,
	       double maxb, double maxb7) {
  extern char *byteprefix;
  char *b;

  *min = 0;
  *dmin = floor->min;
  *filterby = floor->floorby;
  if (*filterby == REQUESTS) {
    *filterby = requests;
    if (floor->qual == '%')
      *dmin *= (double)totr / 100.0;  /* NB dmin < 0 => qual == '\0' */
    else if (floor->qual == ':')
      *dmin *= (double)maxr / 100.0;
    /* negative doubles may not convertable to unsigned integral types --
       C spec 6.2.1.3 */
    *min = (*dmin >= 0)?(unsigned long)(*dmin + 1 - EPSILON):0;
  }
  else if (*filterby == REQUESTS7) {
    *filterby = requests7;
    if (floor->qual == '%')
      *dmin *= (double)totr7 / 100.0;
    else if (floor->qual == ':')
      *dmin *= (double)maxr7 / 100.0;
    *min = (*dmin >= 0)?(unsigned long)(*dmin + 1 - EPSILON):0;
  }
  else if (*filterby == PAGES) {
    *filterby = pages;
    if (floor->qual == '%')
      *dmin *= (double)totp / 100.0;
    else if (floor->qual == ':')
      *dmin *= (double)maxp / 100.0;
    *min = (*dmin >= 0)?(unsigned long)(*dmin + 1 - EPSILON):0;
  }
  else if (*filterby == PAGES7) {
    *filterby = pages7;
    if (floor->qual == '%')
      *dmin *= (double)totp7 / 100.0;
    else if (floor->qual == ':')
      *dmin *= (double)maxp7 / 100.0;
    *min = (*dmin >= 0)?(unsigned long)(*dmin + 1 - EPSILON):0;
  }
  else if (*filterby == BYTES) {
    if (floor->qual == '%')
      *dmin *= totb / 100.0;
    else if (floor->qual == ':')
      *dmin *= maxb / 100.0;
    else if (floor->qual != '\0' &&
	     strchr(byteprefix + 1, floor->qual) != NULL) {
      for (b = byteprefix; *b != floor->qual; b++)
	*dmin *= 1024.0;
    }
  }
  else if (*filterby == BYTES7) {
    if (floor->qual == '%')
      *dmin *= totb7 / 100.0;
    else if (floor->qual == ':')
      *dmin *= maxb7 / 100.0;
    else if (floor->qual != '\0' &&
	     strchr(byteprefix + 1, floor->qual) != NULL) {
      for (b = byteprefix; *b != floor->qual; b++)
	*dmin *= 1024.0;
    }
  }
  else if (*filterby == DATESORT) {
    *filterby = date;
    *min = (*dmin >= 0)?(unsigned long)(*dmin + 1 - EPSILON):0;
  }
  else { /* *filterby == FIRSTDATE */
    *filterby = firstd;
    *min = (*dmin >= 0)?(unsigned long)(*dmin + 1 - EPSILON):0;
  }
}

/* See commentary in my_sort() below */
void makebadpn(Hashentry **badp, unsigned long *badn, Hashindex *ans2,
	       choice requests, choice requests7, choice pages, choice pages7,
	       choice date, choice firstd, unsigned long *maxr,
	       unsigned long *maxr7, unsigned long *maxp, unsigned long *maxp7,
	       double *maxb, double *maxb7, logical donetots) {
  /* Assume already tested badp != NULL */
  Hashindex *p;

  *badp = newhashentry(DATA_NUMBER, FALSE);
  *badn = 0;
  for (p = ans2; p != NULL; TO_NEXT(p)) {
    (*badp)->data[requests] += p->own->data[requests];
    if (pages >= 0)  /* see comment in calctotmax() */
      (*badp)->data[pages] += p->own->data[pages];
    if (requests7 >= 0)
      (*badp)->data[requests7] += p->own->data[requests7];
    if (pages7 >= 0)
      (*badp)->data[pages7] += p->own->data[pages7];
    (*badp)->bytes += p->own->bytes;
    (*badp)->bytes7 += p->own->bytes7;
    if (date >= 0)
      (*badp)->data[date] = MAX((*badp)->data[date], p->own->data[date]);
    if (firstd >= 0) {
      if ((*badp)->data[firstd] == 0)
	(*badp)->data[firstd] = p->own->data[firstd];
      else
	(*badp)->data[firstd] =
	  MIN((*badp)->data[firstd], p->own->data[firstd]);
    }
    (*badn)++;
  }
  if (!donetots) {
    *maxr = MAX(*maxr, (*badp)->data[requests]);
    *maxp = (pages >= 0)?(MAX(*maxp, (*badp)->data[pages])):0;
    *maxr7 = (requests7 >= 0)?(MAX(*maxr7, (*badp)->data[requests7])):0;
    *maxp7 = (pages7 >= 0)?(MAX(*maxp7, (*badp)->data[pages7])):0;
    *maxb = MAX(*maxb, (*badp)->bytes);
    *maxb7 = MAX(*maxb7, (*badp)->bytes7);
  }
}

/* Sorting routine. We do a different sort depending whether the floor is +ve
   (merge sort) or -ve (insertion sort) */
void my_sort(Hashindex **gooditems, Hashindex **baditems, Strlist *partname,
	     Strlist **newpn, Strlist *space, size_t need, choice rep,
	     Floor *floor, choice sortby, logical alphaback, Include *wanthead,
	     choice requests, choice requests7, choice pages, choice pages7,
	     choice date, choice firstd, unsigned long *totr,
	     unsigned long *totr7, unsigned long *totp, unsigned long *totp7,
	     double *totb, double *totb7, unsigned long *maxr,
	     unsigned long *maxr7, unsigned long *maxp, unsigned long *maxp7,
	     double *maxb, double *maxb7, timecode_t *maxd, timecode_t *mind,
	     logical donetots, Hashentry **badp, unsigned long *badn,
	     logical nosort) {
  /* NB floor and sortby could be a SUBFLOOR and SUBSORTBY (see sorttree()) */
  /* Call with rep set to -1 for a flat report, only >= 0 for tree (see
     FINDNAME() above) */
  Hashindex *ans = NULL, *ans2 = NULL, *ans3 = NULL;
  /* wanted items; items below floor; items not wanted even as "not listed" */
  Hashindex *p, *nextp, *q, *lastq = NULL;
  unsigned long min;
  double dmin;
  choice filterby;
  unsigned long count;

  /* Run through all the items discarding unwanted ones into ans3; putting all
     the rest into gooditems; and (if !donetots) finding totals and maxima of
     the gooditems for calculating floors. */
  calctotmax(gooditems, baditems, partname, newpn, space, need, rep, wanthead,
	     requests, requests7, pages, pages7, date, firstd, totr, totr7,
	     totp, totp7, totb, totb7, maxr, maxr7, maxp, maxp7, maxb, maxb7,
	     maxd, mind, donetots, nosort, &ans3);

  if (nosort)
    return;

  /* Calculate the value of the floor (min, dmin and filterby) */
  calcfloor(&min, &dmin, &filterby, floor, requests, requests7, pages, pages7,
	    date, firstd, *totr, *totr7, *totp, *totp7, *totb, *totb7, *maxr,
	    *maxr7, *maxp, *maxp7, *maxb, *maxb7);

  /* Now do the actual flooring and sorting. */
  /* Case I; -ve floor; insertion sort (small end first for speed) */
  if (dmin < 0) {

    /* First work out which items wanted; count represents the remaining number
       of items we want to find */
    count = (unsigned long)(-dmin + EPSILON);

    for (p = *gooditems; p != NULL; p = nextp) {
      nextp = p->next;  /* need this because will overwrite p->next */

      if ((filterby != BYTES && filterby != BYTES7 &&
	   p->own->data[filterby] == 0) ||
	  (filterby == BYTES && p->own->bytes == 0) ||
	  (filterby == BYTES7 && p->own->bytes7 == 0)) {
	/* if it scored 0 we don't want it even if it is in the top n */
	p->next = ans2;
	ans2 = p;
      }
      else {   /* provisionally wanted; check if in top n */
	if (filterby == BYTES) {
	  for (q = ans; q != NULL && p->own->bytes > q->own->bytes;
	       TO_NEXT(q))
	    lastq = q;   /* run to right place in emerging list */
	}
	else if (filterby == BYTES7) {
	  for (q = ans; q != NULL && p->own->bytes7 > q->own->bytes7;
	       TO_NEXT(q))
	    lastq = q;
	}
	else {
	  for (q = ans; q != NULL &&
	       p->own->data[filterby] > q->own->data[filterby];
	       TO_NEXT(q))
	    lastq = q;
	}
	if (q == ans) {  /* at beginning of list */
	  if (count > 0) {     /* list still not big enough */
	    p->next = ans;
	    ans = p;
	    count--;
	  }
	  else {    /* not in top n */
	    p->next = ans2;
	    ans2 = p;
	  }
	}
	else {  /* not at beginning of list, so in top n */
	  p->next = lastq->next;
	  lastq->next = p;
	  if (count == 0) { /* list already full, so shunt smallest item off */
	    lastq = ans; /* temporarily re-using lastq for old ans */
	    ans = ans->next;
	    lastq->next = ans2;
	    ans2 = lastq;
	  }
	  else
	    count--;
	}
      }
    }

    /* Then sort them */
    if (sortby == floor->floorby || sortby == RANDOM) {
      /* easy case; swap the list round so that the largest item is first */
      q = NULL;  /* q is last p */
      for (p = ans; p != NULL; p = nextp) {
	nextp = p->next;
	p->next = q;
	q = p;
      }
      ans = q;
    }
    else {  /* sortby != floorby, so have to sort properly */
      count = 0;
      for (p = ans; p != NULL; TO_NEXT(p))
	count++;
      if (count > 0) {
	if (sortby == BYTES)
	  ans = my_mergesort(ans, count, sortby, &mergeb);
	else if (sortby == BYTES7)
	  ans = my_mergesort(ans, count, sortby, &mergeb7);
	else if (sortby == ALPHABETICAL) {
	  if (alphaback)
	    reversenames(ans);
	  ans = my_mergesort(ans, count, sortby, &mergea);
	}
	else if (sortby == REQUESTS)
	  ans = my_mergesort(ans, count, requests, &merge);
	else if (sortby == REQUESTS7)
	  ans = my_mergesort(ans, count, requests7, &merge);
	else if (sortby == PAGES)
	  ans = my_mergesort(ans, count, pages, &merge);
	else if (sortby == PAGES7)
	  ans = my_mergesort(ans, count, pages7, &merge);
	else if (sortby == DATESORT)
	  ans = my_mergesort(ans, count, date, &merge);
	else /* sortby == FIRSTDATE */
	  ans = my_mergesort(ans, count, firstd, &merge);
	/* it's possible that the list has acquired some trailing junk which
	   we have to lop off */
	for (p = ans; count > 1; TO_NEXT(p))
	  count--;
	p->next = NULL;
	if (alphaback && sortby == ALPHABETICAL)
	  reversenames(ans);
      }
    }
  }

  /* Case II; +ve floor; mergesort */
  else {   /* dmin >= 0 */

    /* First find the wanted ones */
    count = 0;
    if (dmin <= 0.5) {
      filterby = requests;
      min = 1;  /* so as to include exactly those items with any requests */
    }
    for (p = *gooditems ; p != NULL; p = nextp) {
      nextp = p->next;
      if ((filterby != BYTES && filterby != BYTES7 &&
	   p->own->data[filterby] < min) ||
	  (filterby == BYTES && p->own->bytes < dmin) ||
	  (filterby == BYTES7 && p->own->bytes7 < dmin)) {
	p->next = ans2;   /* below floor */
	ans2 = p;
      }
      else {  /* wanted */
	p->next = ans;
	ans = p;
	count++;
      }
    }

    /* Then sort them */
    if (sortby == RANDOM)
      ;  /* do nothing: we're done */
    else if (count > 0) {
      if (sortby == BYTES)
	ans = my_mergesort(ans, count, sortby, &mergeb);
      else if (sortby == BYTES7)
	ans = my_mergesort(ans, count, sortby, &mergeb7);
      else if (sortby == ALPHABETICAL) {
	if (alphaback)
	  reversenames(ans);
	ans = my_mergesort(ans, count, sortby, &mergea);
      }
      else if (sortby == REQUESTS)
	ans = my_mergesort(ans, count, requests, &merge);
      else if (sortby == REQUESTS7)
	ans = my_mergesort(ans, count, requests7, &merge);
      else if (sortby == PAGES)
	ans = my_mergesort(ans, count, pages, &merge);
      else if (sortby == PAGES7)
	ans = my_mergesort(ans, count, pages7, &merge);
      else if (sortby == DATESORT)
	ans = my_mergesort(ans, count, date, &merge);
      else /* sortby == FIRSTDATE */
	ans = my_mergesort(ans, count, firstd, &merge);
      /* it's possible that the list has acquired some trailing junk which
	 we have to lop off */
      for (p = ans; count > 1; TO_NEXT(p))
	count--;
      p->next = NULL;
      if (alphaback && sortby == ALPHABETICAL)
	reversenames(ans);
    }
    else
      ans = NULL;
  }

  /* Accumulate all unwanted items into a single entry. Also recalculate maxima
     to take this one big item into account. */
  if (badp != NULL)
    makebadpn(badp, badn, ans2, requests, requests7, pages, pages7, date,
	      firstd, maxr, maxr7, maxp, maxp7, maxb, maxb7, donetots);

  /* Tack ans3 onto the end of ans2 */
  if (ans2 == NULL)
    ans2 = ans3;
  else if (ans3 != NULL) {
    for (p = ans2; p->next != NULL; TO_NEXT(p))
      ;  /* run to end of ans2 */
    p->next = ans3;
  }

  /* Assign the lists of wanted and unwanted items to be returned */
  *gooditems = ans;
  *baditems = ans2;
}

Hashindex *my_mergesort(Hashindex *list, unsigned long length, choice sortby,
			mergefnp mergefn) {

  Hashindex *ans, *p, *p1, *p2;
  unsigned long a, count;

  if (length <= 1)
    return(list);

  a = length / 2;
  count = a;
  for (p = list; count > 0; TO_NEXT(p))
    count--;    /* find the halfway point of the list */
  p1 = my_mergesort(list, a, sortby, mergefn);
  p2 = my_mergesort(p, length - a, sortby, mergefn);
  ans = mergefn(p1, p2, a, length - a, sortby);
  return(ans);
  /* NB may have trailing junk, but we can lop that off when we've returned,
     so as not to do it for each sub-sort */
}

Hashindex *merge(Hashindex *list1, Hashindex *list2, unsigned long length1,
		 unsigned long length2, choice sortby) {
  Hashindex *ans, *p1, *p2, *p3;

  if (list1->own->data[sortby] > list2->own->data[sortby]) {
    ans = list1;
    p1 = list1->next;
    p2 = list2;
    length1--;
  }
  else {
    ans = list2;
    p2 = list2->next;
    p1 = list1;
    length2--;
  }
  p3 = ans;

  while (length1 > 0 && length2 > 0) {
    if (p1->own->data[sortby] > p2->own->data[sortby]) {
      p3->next = p1;
      TO_NEXT(p3);
      TO_NEXT(p1);
      length1--;
    }
    else {
      p3->next = p2;
      TO_NEXT(p3);
      TO_NEXT(p2);
      length2--;
    }
  }

  if (length1 > 0)
    p3->next = p1;
  else
    p3->next = p2;

  return(ans);
}

Hashindex *mergea(Hashindex *list1, Hashindex *list2, unsigned long length1,
		  unsigned long length2, choice sortby) {
  Hashindex *ans, *p1, *p2, *p3;

  if (strcmp(list1->name, list2->name) < 0) {
    ans = list1;
    p1 = list1->next;
    p2 = list2;
    length1--;
  }
  else {
    ans = list2;
    p2 = list2->next;
    p1 = list1;
    length2--;
  }
  p3 = ans;

  while (length1 > 0 && length2 > 0) {
    if (strcmp(p1->name, p2->name) < 0) {
      p3->next = p1;
      TO_NEXT(p3);
      TO_NEXT(p1);
      length1--;
    }
    else {
      p3->next = p2;
      TO_NEXT(p3);
      TO_NEXT(p2);
      length2--;
    }
  }

  if (length1 > 0)
    p3->next = p1;
  else
    p3->next = p2;

  return(ans);
}

Hashindex *mergeb(Hashindex *list1, Hashindex *list2, unsigned long length1,
		  unsigned long length2, choice sortby) {
  /* see also mergeb7() below */
  Hashindex *ans, *p1, *p2, *p3;

  if (list1->own->bytes > list2->own->bytes) {
    ans = list1;
    p1 = list1->next;
    p2 = list2;
    length1--;
  }
  else {
    ans = list2;
    p2 = list2->next;
    p1 = list1;
    length2--;
  }
  p3 = ans;

  while (length1 > 0 && length2 > 0) {
    if (p1->own->bytes > p2->own->bytes) {
      p3->next = p1;
      TO_NEXT(p3);
      TO_NEXT(p1);
      length1--;
    }
    else {
      p3->next = p2;
      TO_NEXT(p3);
      TO_NEXT(p2);
      length2--;
    }
  }

  if (length1 > 0)
    p3->next = p1;
  else
    p3->next = p2;

  return(ans);
}

Hashindex *mergeb7(Hashindex *list1, Hashindex *list2, unsigned long length1,
		   unsigned long length2, choice sortby) {
  /* This is exactly the same as mergeb() above, except it sorts by bytes7
     instead of bytes. It's just implemented as a second function for speed. */
  Hashindex *ans, *p1, *p2, *p3;

  if (list1->own->bytes7 > list2->own->bytes7) {
    ans = list1;
    p1 = list1->next;
    p2 = list2;
    length1--;
  }
  else {
    ans = list2;
    p2 = list2->next;
    p1 = list1;
    length2--;
  }
  p3 = ans;

  while (length1 > 0 && length2 > 0) {
    if (p1->own->bytes7 > p2->own->bytes7) {
      p3->next = p1;
      TO_NEXT(p3);
      TO_NEXT(p1);
      length1--;
    }
    else {
      p3->next = p2;
      TO_NEXT(p3);
      TO_NEXT(p2);
      length2--;
    }
  }

  if (length1 > 0)
    p3->next = p1;
  else
    p3->next = p2;

  return(ans);
}
