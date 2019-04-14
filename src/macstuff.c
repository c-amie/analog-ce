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

/*** macstuff.c; stuff only required for the Mac port ***/
/*** The contents of this file is copyright various authors as follows. ***/

/* The functions in this file inside #ifdef MAC_EVENTS are due to, and
   copyright, Jason Linhart (jason@summary.net), 1996. */
/* The functions in this file inside #ifdef MACDIRENT are due to
   Jason Linhart, January 1997. */
/* The functions in this file inside #ifdef MAC are due to Jason Linhart,
   March 1998. */
/* Code for OS X Carbon by Jason Linhart, March 2001. */
/* The functions in this file inside #ifndef NODNS are due to, and
   copyright, Jason Linhart and Stephan Somogyi, 1996, 1997.
   Version history:
   950531  SCS      First release
   960716  JTL      Switched to async OT calls and improved failure cases
                    to not recheck
   960927  JTL      Added MacTCP support and combined check and open into
                    OpenNetwork
*/

#define REAL_ALLOC
#include "anlghea3.h"
#ifdef MAC_EVENTS

static char *cmd_vect[64] = {
	"analog",
	"-G",
	"+g~~~",
	0
	};

extern char mac_do_script[256];
extern char TryAEOpen(void);

static int
CmdParse(char **vect,char *cmd_line,int limit)
{
        int count;

        for (count=0; count<limit && *cmd_line; ++count) {
                if (*cmd_line=='"') {
                        ++cmd_line;
                        *vect++ = cmd_line;
                        while (*cmd_line && *cmd_line!='"') ++cmd_line;
                        if (*cmd_line) *cmd_line++ = 0;
                        }
                else {
                        *vect++ = cmd_line;
                        while (*cmd_line && *cmd_line!=' ' && *cmd_line!='\t') ++cmd_line;
                        if (*cmd_line) *cmd_line++ = 0;
                        }
                while (*cmd_line==' ' || *cmd_line=='\t') ++cmd_line;
                }
        return(count);
        }


void MacInit(int *argc,char **argv[])
{
  char c;

  SIOUXSettings.asktosaveonclose=false;
  (void)printf("Processing...\n");
  setvbuf(stdout,NULL,_IOLBF,BUFSIZ);
  if ((c=TryAEOpen())!=0) {
    if (c!='#') {
      cmd_vect[1]="~~~";
      *argc=2;
    }
    else *argc=3;
  }
  else if (mac_do_script[0]) 
        *argc=1+CmdParse(cmd_vect+1,mac_do_script,63);
  else *argc=1;
  *argv=cmd_vect;
}

void mac_temp_free(void);

void MacFini(void)
{
  extern logical anywarns;

  if (!anywarns)
    SIOUXSettings.autocloseonquit=true;
  SIOUXSettings.sleep=20;
#ifndef NODNS
  ResolverCleanup();
#endif
  (void)fprintf(stderr, "Complete!\n");
  mac_temp_free();
}

extern int mac_quit_now;

void MacIdle(void)
{
  extern logical anywarns;
  static long time = 0;

  EventRecord myEvent;
  WindowPtr whichWindow;
#if TARGET_API_MAC_CARBON
  Rect tempRect;
#endif
  char theChar;

  if (TickCount()<time) return;
  if (mac_quit_now) {
    anywarns = FALSE;  /* kludge so that window doesn't sit around */
    my_exit(1);
  }
#if !TARGET_API_MAC_CARBON
  SystemTask();
#endif
  if (WaitNextEvent(everyEvent, &myEvent, 1, nil)) {

    if (!SIOUXHandleOneEvent(&myEvent)) switch (myEvent.what) {

    case mouseDown:
      switch (FindWindow(myEvent.where,&whichWindow)) {

      case inMenuBar:
	MenuSelect(myEvent.where);
	break;
#if !TARGET_API_MAC_CARBON
      case inSysWindow:
	SystemClick(&myEvent,whichWindow);
	break;
#endif
      case inContent:
	SelectWindow(whichWindow);
	break;
      case inDrag:
#if TARGET_API_MAC_CARBON
	GetRegionBounds(GetGrayRgn(),&tempRect);
	DragWindow(whichWindow,myEvent.where,&tempRect);
#else
	DragWindow(whichWindow,myEvent.where,&qd.screenBits.bounds);
#endif
	break;
      }
      break;
    case keyDown:
      theChar = myEvent.message & charCodeMask;
      break;
    case updateEvt:
      BeginUpdate((WindowPtr) myEvent.message);
      EndUpdate((WindowPtr) myEvent.message);
      break;
    }
  }
  time=TickCount()+20;
}
#endif   /* (ifdef MAC_EVENTS) */

#ifdef MACDIRENT
/* Assume #ifdef MAC, so MacHeaders already loaded */
static int indx=0;					/* offset to current entry, >0 is open, 0 is closed, -1 is at end */
static unsigned char fname[257];	// The current file name
static short dir_volume;			// The open dir's volume
static long dir_ID;					// The open dir's dir ID
static char last_attrib;			// The attributes of the last entry read

static void
CToPCpy(unsigned char *pstr, const char *cstr)
{                 /* Convert a C string to a Pascal string */
        register char *dptr, len;

        len=0;
        dptr=(char *)pstr+1;
        while (len<255 && (*dptr++ = *cstr++)!=0) ++len;
        *pstr=len;
        }

DIR *
opendir (const char *name)           /* Open a directory stream on NAME. */
/* Return a DIR stream on the directory, or NULL if it could not be opened.  */
{
	CInfoPBRec cat_info;
	FSSpec spec;
	Boolean targetIsFolder, wasAliased;

	if (indx) {											// We don't support more than one dir open at a time
		closedir((DIR *)1);								// Fail hard
		return((DIR *)0);
		}
	dir_volume=0;										// Default to the application directory
	dir_ID=0;
	while (name[0]=='.') ++name;						// Don't allow leading '.', avoids device names
		CToPCpy(fname,name);
		if (!FSMakeFSSpec(0,0,fname,&spec)) {			// Figure out where user is pointing
			ResolveAliasFile(&spec, true, &targetIsFolder, &wasAliased);
			cat_info.dirInfo.ioCompletion=(IOCompletionUPP)0;
			cat_info.dirInfo.ioNamePtr=spec.name;
			cat_info.dirInfo.ioVRefNum=spec.vRefNum;
			cat_info.dirInfo.ioFDirIndex = 0;			// Use full spec
			cat_info.dirInfo.ioDrDirID=spec.parID;
			cat_info.dirInfo.ioFVersNum=0;
			if (!PBGetCatInfoSync(&cat_info) && (cat_info.dirInfo.ioFlAttrib&16)) 
				spec.parID=cat_info.dirInfo.ioDrDirID;	// Found it, save it's volume and dirID
			dir_volume=spec.vRefNum;
			dir_ID=spec.parID;
			}
		else return ((DIR *)0);
	indx=1;
	return((DIR *)1);
	}

int
closedir (DIR * dirp)
/* Close the directory stream DIRP. Return 0 if successful, -1 if not.  */
{
	if (indx) {
		indx=0;
		return(0);
		}
	return(-1);
	}

struct dirent *
readdir (DIR * dirp)                 /* Read a directory entry from DIRP. */
/* Return a pointer to a `struct dirent' describing the entry, or NULL for EOF
   or error.  The storage returned may be overwritten by a later readdir call
   on the same DIR stream.  */
{
	CInfoPBRec finfo;

	finfo.hFileInfo.ioCompletion=(IOCompletionUPP)0;	// No completion routine
	finfo.hFileInfo.ioNamePtr=fname;					// Where to return file names
	finfo.hFileInfo.ioVRefNum=dir_volume;				// Saved volume and directory
	finfo.hFileInfo.ioDirID=dir_ID;
	finfo.hFileInfo.ioFDirIndex=indx;					// Appropriate entry in dir
	if (indx>0 && !PBGetCatInfoSync(&finfo)) {			// Get info about the next dir entry
		last_attrib=finfo.hFileInfo.ioFlAttrib;
		fname[fname[0]+1]=0;
		++indx;
		return((struct dirent *)(fname+1));
		}
	if (indx) indx = -1;								// Must be at end of dir
	return((struct dirent *)0);
	}

void
rewinddir (DIR * dirp)				/* Rewind DIRP to the beginning of the directory. */
{
	if (indx) indx=1;
	}

void
seekdir (DIR * dirp, off_t pos) /* Seek to position POS on DIRP.  */
{
	if (indx && pos>0) indx=pos;
	}

off_t
telldir (DIR * dirp)                 /* Return the current position of DIRP. */
{
	return((indx>0)?indx:-1);
	}

int
dirstat(const char *file_name, struct stat *buf)
/* Special version of stat that only works for most recent dir entry */
{
	if (indx) {
		if (last_attrib&0x10) buf->st_mode=S_IFDIR;
		else buf->st_mode=S_IFREG;
		return(0);
		}
	return(-1);
	}


#endif

#ifdef MAC
#ifndef NODNS
static long OpenNetwork(void);

/* Takes a string of an IP address in *hostname, resolves it to a domain name,
   and returns the name in *hostname.
   Returns nil if unresolvable (or something else went wrong), otherwise
   returns 1. */

/* URL processing and host lookup */

static long slNetChecked = 0, slNetPresent = 0, slNetSvcOpen = 0;
static OTNotifyUPP svcnotifyproc = 0;

typedef struct {         /* Open Transport Internet services provider info */
  InetSvcRef ref;        /* provider reference */
  Boolean done;          /* true when asynch operation has completed */
  OTResult result;       /* result code */
  void *cookie;          /* cookie */
} SvcInfo;

static SvcInfo sSvcRef;

#if !TARGET_API_MAC_CARBON
static ResultUPP gMacTCPDNRResultProcUPP = 0;

static pascal void
MacTCPDNRResultProc (struct hostInfo *hInfoPtr, char *userDataPtr)
{
  *(Boolean*)userDataPtr = true;
}
#endif

int IpAddr2Name(char *hostname)
{
#if !TARGET_API_MAC_CARBON
  struct hostInfo hInfoMacTCP;
  OSErr err;
  int cnt, tmp;
  char *cptr;
  Boolean done;
#endif
  OSStatus lStatus;
  InetHost lHostAddr;

  if (!slNetChecked) {
    slNetPresent = OpenNetwork();
    slNetChecked = 1;
  }

  if (slNetPresent == 1) {

    /* turn ascii with periods into a long */
    lStatus = OTInetStringToHost(hostname, &lHostAddr);
    if (lStatus != noErr) return 0;

    /* turn the long into a reverse-resolved name */
    sSvcRef.done=false;
    lStatus=OTInetAddressToName(sSvcRef.ref,lHostAddr,hostname);
    if (!lStatus) {
      do {
	MacIdle();
      } while (!sSvcRef.done);
      lStatus=sSvcRef.result;
    }
    if (!lStatus) {
      if (hostname[strlen(hostname)-1]=='.') hostname[strlen(hostname)-1]=0;
      return(1);
    }
  }
#if !TARGET_API_MAC_CARBON
  else if (slNetPresent==2) {
    lHostAddr=0;
    cptr=hostname;
    for (cnt=0; cnt<4; ++cnt) {
      if (!ISDIGIT(*cptr)) return(0);
      tmp=atoi(cptr);
      if (tmp<0 || tmp>255) return(0);
      lHostAddr=(lHostAddr<<8)|tmp;
      while (ISDIGIT(*cptr)) ++cptr;
      if (cnt!=3 && *cptr!='.') return(0);
      ++cptr;
    }
    memset(&hInfoMacTCP, 0, sizeof(hInfoMacTCP));
    done=false;
    err = AddrToName(lHostAddr, &hInfoMacTCP, gMacTCPDNRResultProcUPP,
		     (char*)&done);
    if (err == cacheFault) {
      while (!done) MacIdle();
      err = hInfoMacTCP.rtnCode;
    }
    if (err == noErr) {
      hInfoMacTCP.cname[254] = 0;
      (void)strcpy(hostname, hInfoMacTCP.cname);
      if (hostname[strlen(hostname)-1]=='.') hostname[strlen(hostname)-1]=0;
      return(1);
    }
  }
#endif
  return 0;
} /* end IpAddr2Name() */


/*      Must call this before quitting app
*/
void
ResolverCleanup(void)
{

  if (slNetChecked && slNetSvcOpen) {
    if (slNetPresent==1) OTCloseProvider(sSvcRef.ref);
#if !TARGET_API_MAC_CARBON
    else if (slNetPresent==2) CloseResolver();
#endif
  }
} /* end ResolverCleanup() */

/* #pragma mark - */

/*
        Check for availbility of OT/TCP 1.1 or later,
        or MacTCP and open the service.
        Return nil if it isn't.
*/

static pascal void
SvcNotifyProc(void *dataPtr,OTEventCode code,OTResult result,void *cookie)
{
  register SvcInfo *svcInfo;

  svcInfo=(SvcInfo *)dataPtr;
  svcInfo->done=true;
  svcInfo->result=result;
  svcInfo->cookie=cookie;
}

static Boolean
OpenInetServices(SvcInfo *svcInfo)
{
  OSStatus result;

  svcInfo->done=false;
  if (!svcnotifyproc) svcnotifyproc=NewOTNotifyUPP(&SvcNotifyProc);
#ifdef __CARBON__
  
  result=OTAsyncOpenInternetServicesInContext(kDefaultInternetServicesPath, 0,
					      svcnotifyproc, svcInfo, NULL);
#else
  result=OTAsyncOpenInternetServices(kDefaultInternetServicesPath, 0,
				     svcnotifyproc, svcInfo);
#endif
  if (!result) {
    do {
      MacIdle();
    } while (!svcInfo->done);
    result=svcInfo->result;
  }
  if (result) return(false);
  svcInfo->ref=(InetSvcRef)svcInfo->cookie;
  return(true);
}

static long
OpenNetwork(void)
{
  OSStatus lStatus;
  OSErr err;
  long lResponse, lCriteria;

  err = Gestalt(gestaltOpenTpt, &lResponse);
  if (err == noErr)       {
    /* Older OpenTransport Headers require that thenext line read:
       lCriteria = gestaltOpenTptPresent + gestaltOpenTptTCPPresent; */
    lCriteria = gestaltOpenTptPresentMask + gestaltOpenTptTCPPresentMask;
    lResponse = lCriteria & lResponse;
    if (lResponse == lCriteria)     {
#ifdef __CARBON__
      lStatus=InitOpenTransportInContext(kInitOTForApplicationMask,NULL);
#else
      lStatus = InitOpenTransport();
#endif
      if (lStatus == noErr) {
	if (OpenInetServices(&sSvcRef)) {
	  slNetSvcOpen=1;
	  return(1);
	}
      }
      return(0);
      /* OT present, but won't open */
    }
  }
#if !TARGET_API_MAC_CARBON
  else {
    gMacTCPDNRResultProcUPP = NewResultProc(&MacTCPDNRResultProc);
    err = OpenResolver(nil);
    if (err == noErr) {
      slNetSvcOpen=1;
      return(2);
    }
  }
#endif
  return(0);
} /* end OpenNetwork() */
#endif
#endif

#ifdef MAC

#define MY_MAGIC_ONE		0x3748596A
#define MY_MAGIC_TWO		0xA7958473

#define MY_BIG_SIZE			(32*1024)
#define MY_BLOCK_SIZE		(4*(MY_BIG_SIZE+3*sizeof(long)))

#if MY_BIG_SIZE!=BLOCKSIZE
Look into this! They should be the same, but changes would
require other adjustments.
#endif

static long my_cur_avail = 0;
static char *my_cur_block;
static struct my_block {
	struct my_block **next;
	char storage[MY_BLOCK_SIZE];
	} **temp_mem_blocks = nil;

static struct my_free {
	struct my_free *next;
	char storage[MY_BIG_SIZE-sizeof(struct my_free *)];
	} *my_free_list = nil;

void mac_temp_free(void);

void
mac_temp_free(void)
{
	OSErr result;
	struct my_block **block, **next;

	block=temp_mem_blocks;
	while (block) {
		next=(*block)->next; 
		TempDisposeHandle((Handle)block,&result);
		if (result) break;
		block=next;
		}
	}

static void *
my_temp_malloc(size_t size)
{
	OSErr result;
	struct my_block **block;
	long version;
	char *ret_ptr;

	size=(size+3)&~3;
	if (Gestalt(gestaltSystemVersion,&version) || version<0x700) return(nil);
	if (size>MY_BLOCK_SIZE) {
		if (TempFreeMem()<1024*1024+size) return(nil);
		block=(struct my_block **)TempNewHandle(size+sizeof(struct my_block **),&result);
		if (!block || !*block || result) return(nil);
		TempHLock((Handle)block,&result);
		(*block)->next=temp_mem_blocks;
		temp_mem_blocks=block;
		return((void *)&(*block)->storage);
		}
	if (size>my_cur_avail) {
		if (TempFreeMem()<1024*1024+MY_BLOCK_SIZE) return(nil);
		block=(struct my_block **)TempNewHandle(sizeof(struct my_block),&result);
		if (!block || !*block || result) return(nil);
		TempHLock((Handle)block,&result);
		(*block)->next=temp_mem_blocks;
		temp_mem_blocks=block;
		my_cur_block=(*block)->storage;
		my_cur_avail=MY_BLOCK_SIZE;
		}
	ret_ptr=my_cur_block;
	my_cur_block+=size;
	my_cur_avail-=size;
	return((void *)ret_ptr);
	}

#define MAC_LEAVE_FREE          (64*1024)

static void *
sub_mac_malloc(size_t size)
{
	char *ret_ptr;

	size=(size+3)&~3;
	if (size>=MY_BIG_SIZE) {
		if (MaxBlock()<=MAC_LEAVE_FREE+size) return(my_temp_malloc(size));
		return(malloc(size));
		}
	if (size>my_cur_avail) {
		if (MaxBlock()<MAC_LEAVE_FREE+MY_BLOCK_SIZE) return(my_temp_malloc(size));
		my_cur_block=malloc(MY_BLOCK_SIZE);
		my_cur_avail=MY_BLOCK_SIZE;
		}
	ret_ptr=my_cur_block;
	my_cur_block+=size;
	my_cur_avail-=size;
	return((void *)ret_ptr);
	}

void *
mac_malloc(size_t size)
{
	long *ptr;

	if (size>=MY_BIG_SIZE) {
		if (size==MY_BIG_SIZE && my_free_list) {
			ptr=(long *)my_free_list;
			my_free_list=my_free_list->next;
			}
		else {
			size+=(size/MY_BIG_SIZE)*3*sizeof(long);
			ptr=(long *)sub_mac_malloc(size);
			if (ptr) {
				*ptr++ = size;
				*ptr++ = MY_MAGIC_ONE;
				*ptr++ = MY_MAGIC_TWO;
				}
			}
		}
	else ptr=(long *)sub_mac_malloc(size);
	return((void *)ptr);
	}

void *
mac_calloc(size_t num,size_t size)
{
	return(mac_malloc(num*size));
	}

void *
mac_realloc(void *ptr,size_t size)
{
	void *new_blk;

	if (!size || !(new_blk=mac_malloc(size))) return(nil);
	memcpy(new_blk,ptr,size);
	mac_free(ptr);
	return(new_blk);
	}

void
mac_free(void *ptr)
{
	struct my_free *blk;
	long *lptr, size;

	lptr=(long *)ptr;
	if (lptr) {
		lptr-=3;
		if (lptr[1]==MY_MAGIC_ONE && lptr[2]==MY_MAGIC_TWO) {
			size = *lptr + 3*sizeof(long);
			while (size>=MY_BIG_SIZE+3*sizeof(long)) {
				blk=(struct my_free *)(lptr+3);
				blk->next=my_free_list;
				my_free_list=blk;
				*lptr=MY_BIG_SIZE+3*sizeof(long);
				lptr[1]=MY_MAGIC_ONE;
				lptr[2]=MY_MAGIC_TWO;
				lptr+=(MY_BIG_SIZE/sizeof(long))+3;
				size-=MY_BIG_SIZE+3*sizeof(long);
				}
			}
		}
	}
#endif
