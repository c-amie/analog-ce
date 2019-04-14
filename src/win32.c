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

/*** win32.c; stuff only required for the Win32 port ***/
/* This stuff is due to Magnus Hagander (mha@edu.sollentuna.se) */
#include "anlghea3.h"
#ifdef WIN32

/*
 * Initialize the required Win32 structures and routines
 */

void Win32Init(void) {
#ifndef NODNS
  WSADATA wsaData;

  if (WSAStartup(MAKEWORD(1,1),&wsaData))
    error("unable to initialise winsock.dll");
#endif
#ifndef NOPRIORITY
  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
#endif
}

/*
 * Cleanup Win32 structures and routines
 */
void Win32Cleanup(void) {
#ifndef NODNS
  WSACleanup();
#endif
}
#endif
