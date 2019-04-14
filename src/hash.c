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

/*** hash.c; the functions which do all the work in the hash tables. ***/

#include "anlghea3.h"

Hashtable *rehash(Hashtable *old, unsigned long size, Memman *treespace) {
  /* Construct a new hash table out of an old one, using same buckets */
  unsigned long i, magic;
  Hashindex *p, *nextp;
  Hashtable *ans;
  char *c;

  if (treespace != NULL && size * sizeof(Hashindex *) < BLOCKSIZE) {
    ans = (Hashtable *)submalloc(treespace, sizeof(Hashtable));
    ans->head = (Hashindex **)submalloc(treespace, size * sizeof(Hashindex *));
  }
  else {
    ans = (Hashtable *)xmalloc(sizeof(Hashtable));
    ans->head = (Hashindex **)xmalloc(size * sizeof(Hashindex *));
  }
  ans->size = size;
  ans->n = 0;
  for (i = 0; i < size; i++)
    ans->head[i] = NULL;

  if (old != NULL) {
    if (treespace != NULL)    /* i.e. is tree */
      ans->head[0] = old->head[0];
    for (i = (unsigned long)(treespace != NULL); i < old->size; i++) {
      /* run through items in old table */
      for (p = old->head[i]; p != NULL; p = nextp) {
	nextp = p->next;    /* rehash p into new table */
	if (treespace != NULL) {
	  c = strchr(p->name, '\0');
	  MAGICNOTREE(magic, p->name, c, size);
	}
	else
	  MAGICNO(magic, p->name, size);
	p->next = ans->head[magic];
	ans->head[magic] = p;
	ans->n++;
      }
    }
    if (treespace == NULL || old->size * sizeof(Hashindex *) >= BLOCKSIZE) {
      /* free old table, if originally xmalloc'ed */
      free((void *)(old->head));
      free((void *)old);
    }
  }
  return(ans);
}

Hashindex *hashfind(Memman *mp, Hashtable **table, unsigned int data_number,
		    Include *wanthead, choice ispage, Include *ispagehead,
		    Alias *aliashead, char *dirsuffix,
		    unsigned int dirsufflength, logical usercase_insensitive,
		    unsigned char convfloor, logical multibyte, choice type,
		    logical aliased) {
  /* NB The calling function will normally consult (lp->own) if aliased,
     (Hashentry *)(lp->other) if !aliased */
  /* NB2 ispage != UNSET only if aliasing and in/excluding already done too */
  /* If aliased, aliashead isn't used */
  /* convfloor and multibyte only passed to do_alias, and only used if we are
     constructing the Search Word Report or Search Query Report */
  extern Hashentry *unwanted_entry, *blank_entry;
  extern Memman *amemman, *xmemman;

  unsigned long magic;  /* register'ing this made it slower on my machine */
  Hashindex *lp, *lastlp;
  char *name, *c;
  choice rc;

  if (TOO_FULL((*table)->n, (*table)->size))
    *table = rehash(*table, NEW_SIZE((*table)->size), NULL);
  name = (char *)(mp->curr_pos);
  MAGICNO(magic, name, (*table)->size);

  lp = (*table)->head[magic];
  lastlp = NULL;

  while (TRUE) {
    if (lp == NULL) {  /* need a new index entry */
      lp = (Hashindex *)submalloc(xmemman, sizeof(Hashindex));
      if (lastlp == NULL)
	(*table)->head[magic] = lp;
      else
	lastlp->next = lp;
      lp->name = name;
      lp->own = NULL;
      lp->other = NULL;
      lp->next = NULL;
      ((*table)->n)++;

      if (aliased) {
	if (ispage == UNSET) {
	  ispage = (choice)pageq(name, ispagehead, type);
	  if (included(name, (logical)ispage, wanthead))
	    lp->own = newhashentry(data_number, (logical)ispage);
	  else
	    lp->own = unwanted_entry;
	}
	else
	  lp->own = newhashentry(data_number, (logical)ispage);
      }
      else {
	if ((rc = do_alias(lp->name, amemman, aliashead, dirsuffix,
			   dirsufflength, usercase_insensitive, convfloor,
			   multibyte, type)) == TRUE)
	  lp->other = (void *)hashfind(amemman, table, data_number, wanthead,
				       UNSET, ispagehead, NULL, dirsuffix,
				       dirsufflength, usercase_insensitive,
				       convfloor, multibyte, type, TRUE)->own;
	else if (rc == ERR) {
	  if (included("", FALSE, wanthead)) {
	    lp->own = blank_entry;
	    lp->other = (void *)blank_entry;
	  }
	  else {
	    lp->own = unwanted_entry;
	    lp->other = (void *)unwanted_entry;
	  }
	}
	else {
	  ispage = (choice)pageq(name, ispagehead, type);
	  if (included(name, (logical)ispage, wanthead)) {
	    lp->own = newhashentry(data_number, (logical)ispage);
	    lp->other = (void *)(lp->own);
	  }
	  else {
	    lp->own = unwanted_entry;
	    lp->other = (void *)unwanted_entry;
	  }
	}
      }    /* end !aliased */
      return(lp);
    }      /* end need new entry */
    else if (STREQ(lp->name, name)) {  /* found it */
      mp->next_pos = mp->curr_pos;   /* overwrites old name in mp */
      if (aliased) {
	if (lp->own == NULL) {   /* haven't calculated lp->own yet */
	  if (ispage == UNSET) {
	    if (type == ITEM_FILE) {
	      if ((c = strchr(name, '?')) != NULL) {
		*c = '\0';
		ispage = (choice)included(name, FALSE, ispagehead);
		*c = '?';
	      }
	      else
		ispage = (choice)included(name, FALSE, ispagehead);
	    }
	    else
	      ispage = FALSE;
	    if (included(name, (logical)ispage, wanthead))
	      lp->own = newhashentry(data_number, (logical)ispage);
	    else
	      lp->own = unwanted_entry;
	  }
	  else
	    lp->own = newhashentry(data_number, (logical)ispage);
	}
      }
      else {   /* !aliased */
	if (lp->other == NULL) {
	  if ((rc = do_alias(lp->name, amemman, aliashead, dirsuffix,
			     dirsufflength, usercase_insensitive, convfloor,
			     multibyte, type)) == FALSE)
	    lp->other = (void *)(lp->own);
	    /* own must have been set because alias hasn't */
	  else if (rc == TRUE)
	    lp->other = (void *)hashfind(amemman, table, data_number, wanthead,
					 UNSET, ispagehead, NULL, dirsuffix,
					 dirsufflength, usercase_insensitive,
					 convfloor, multibyte, type,
					 TRUE)->own;
	  else /* rc == ERR */ if (included("", FALSE, wanthead))
	    lp->other = (void *)blank_entry;
	  else
	    lp->other = (void *)unwanted_entry;
	}
      }    /* end !aliased */
      return(lp);
    }      /* end found it */
    else {
      lastlp = lp;
      TO_NEXT(lp);
    }
  }
}

Hashentry *newhashentry(unsigned int data_number, logical ispage) {
  extern Memman *xmemman;

  Hashentry *ans;
  unsigned int i;

  ans = (Hashentry *)submalloc(xmemman, sizeof(Hashentry));
  ans->data = (unsigned long *)submalloc(xmemman,
					 data_number * sizeof(unsigned long));
  for (i = 0; i < data_number; i++)
    ans->data[i] = 0;
  ans->bytes = 0.0;
  ans->bytes7 = 0.0;
  ans->ispage = ispage;
  ans->reused = TRUE;    /* only used in tree.c: see newtreedata() there */
  return(ans);
}

void hashscore(Hashentry *ep, unsigned long data[DATA2_NUMBER],
	       choice datacols[DATACOLS_NUMBER][2], choice outcome,
	       double bytes) {

  for ( ; **datacols >= 0; datacols++) {
    if ((*datacols)[1] == DATE2)
      ep->data[(*datacols)[0]] = MAX(ep->data[(*datacols)[0]], data[DATE2]);
    else if ((*datacols)[1] == FIRSTD2 && ep->data[(*datacols)[0]] != 0 &&
	     data[FIRSTD2] != 0)
	/* kludge for first entry: too hard to initialise the correct columns
	   to LAST_TIME in newhashentry */
	ep->data[(*datacols)[0]] = MIN(ep->data[(*datacols)[0]],
				       data[FIRSTD2]);
    else
      ep->data[(*datacols)[0]] += data[(*datacols)[1]];
  }
  if (outcome == SUCCESS) {
    ep->bytes += bytes;
    if (data[REQUESTS72] > 0) {
      ep->bytes7 += bytes;
    }
  }
}

void unhash(Hashtable *hash, Hashindex **gooditems, Hashindex **baditems) {
  Hashindex *p, *nextp;
  unsigned long j;

  *gooditems = NULL;
  *baditems = NULL;
  for (j = 0; j < hash->size; j++) {  /* run through all items */
    for (p = hash->head[j]; p != NULL; p = nextp) {
      nextp = p->next;
      p->next = *gooditems;  /* compiling backwards is easier */
      *gooditems = p;
    }
  }
}

void unhashall(Hashtable **hash, Hashindex ***gooditems,
	       Hashindex ***baditems) {
  unsigned int i;

  *gooditems = (Hashindex **)xmalloc(ITEM_NUMBER * sizeof(Hashindex *));
  *baditems = (Hashindex **)xmalloc(ITEM_NUMBER * sizeof(Hashindex *));
  for (i = 0; i < ITEM_NUMBER; i++) {
    unhash(hash[i], &((*gooditems)[i]), &((*baditems)[i]));
  }
}
