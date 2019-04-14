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

/*** The contents of this file is copyright (c) Jason Linhart 1997. ***/

/*** macdir.h
 *
 *      Directory Operations for Mac based on BSD 4.3   <macdir.h>
 *      By Jason Linhart, January 1997
 ***/

#ifndef _MACDIR_H
#define _MACDIR_H       1

#include <stat.h>

#define NAME_MAX                255             /* # chars in a file name */

struct dirent {
#ifdef COMMENT
        long            d_ino;
        off_t           d_off;
        unsigned short  d_reclen;
#endif
        char            d_name[NAME_MAX+1];
};

/* The internal is hidden from the user. */
typedef void DIR;

/* Open a directory stream on NAME.
   Return a DIR stream on the directory, or NULL if it could not be opened.  */extern DIR *opendir (const char *name);

/* Close the directory stream DIRP.
   Return 0 if successful, -1 if not.  */
extern int closedir (DIR * dirp);

/* Read a directory entry from DIRP.
   Return a pointer to a `struct dirent' describing the entry,
   or NULL for EOF or error.  The storage returned may be overwritten
   by a later readdir call on the same DIR stream.  */
extern struct dirent *readdir (DIR * dirp);

/* Rewind DIRP to the beginning of the directory.  */
extern void rewinddir (DIR * dirp);

/* Seek to position POS on DIRP.  */
extern void seekdir (DIR * dirp, off_t pos);

/* Return the current position of DIRP.  */
extern off_t telldir (DIR * dirp);

/* Fake stat to work with current directory entry */

#define stat(file_name,buf)             dirstat(file_name,buf)

extern int dirstat(const char *file_name, struct stat *buf);

#endif /* macdir.h  */
