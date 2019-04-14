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

/*** The contents of this file is copyright (c) Jason Linhart 1998 - 2001. ***/

/*** macinput.c; input functions for Mac */

#define REAL_STDIO	/* Define these to suppress defines in anlghea3.h */
/* #define DEBUG_VERSION */

#include "anlghea3.h"

#ifdef MAC
#ifdef USE_ZLIB
#include <unix.h>
#include <unistd.h>
#include <fcntl.h>
#include "my_bzlib.h"
#include "my_zlib.h"
#include "unzip.jtl.h"
#endif

#ifdef LINE_PARSER
/*
	Input file parsing
*/

#define LOC_BUFF_SIZE		(16*1024)

#ifndef USE_ZLIB
typedef long gzFile;
typedef long unzFile;
typedef long BZFILE;
#endif

static FILE *in_fp = 0;
static gzFile gz_fp;
static unzFile zip_fp;
static BZFILE *bz_fp;
static int zip_eof;
static int in_mode = 0;			// 0-plain, 1-GZip, 2-PKZip, 3-BZip2
static long line_num;
static int buff_cnt, buff_offset;
static char *buff = nil;

static void
FillBuff(int drop_cnt)
{
	buff_offset+=drop_cnt;
	buff_cnt-=drop_cnt;
	if (buff_cnt<LOC_BUFF_SIZE/2) {
		if (buff_cnt>0) memmove(buff,buff+buff_offset,buff_cnt);
		buff_offset=0;
		switch (in_mode) {

		case 0:
			buff_cnt+=fread(buff+buff_cnt,1,LOC_BUFF_SIZE-buff_cnt,in_fp);
			break;
#ifdef USE_ZLIB
		case 1:
			buff_cnt+=gzread(gz_fp,(voidp)(buff+buff_cnt),LOC_BUFF_SIZE-buff_cnt);
			break;
		case 2:
			while (buff_cnt<LOC_BUFF_SIZE && !zip_eof) {
				long part;

				part=LOC_BUFF_SIZE-buff_cnt;
				part=unzReadCurrentFile(zip_fp,buff+buff_cnt,part);
				if (part<0) {
					unzCloseCurrentFile(zip_fp);
					zip_eof=true;
					break;
					}
				else if (part==0) {
					unzCloseCurrentFile(zip_fp);
					if (unzGoToNextFile(zip_fp)!=UNZ_OK || 
						 unzOpenCurrentFile(zip_fp)!=UNZ_OK) {
						zip_eof=true;
						break;
						}
					}
				else buff_cnt+=part;
				}
			break;
		case 3:
			buff_cnt+=BZ2_bzread(bz_fp,buff+buff_cnt,LOC_BUFF_SIZE-buff_cnt);
			break;
#endif
			}
		}
	}

/*
	Get file from HighLevel Apple Event
*/

static Boolean haveFSS = false;
static FSSpec myFSS;

static OSErr
GotRequiredParams(const AppleEvent *theAppleEvent)
{
	DescType typeCode;
	Size actualSize;
	OSErr retErr, err;

	err=AEGetAttributePtr(theAppleEvent,keyMissedKeywordAttr,typeWildCard,&typeCode,
		NULL,0,&actualSize);
	
	if (err==errAEDescNotFound)	retErr = noErr;		// we got all the required params: all is ok
	else if (err==noErr) retErr = errAEEventNotHandled;
	else retErr = err;
	return retErr;
	}

static pascal OSErr					/* Used for Print and Close events */
DoAENoop(const AppleEvent *theAppleEvent,AppleEvent *reply,long handlerRefcon)
{
#pragma unused ( handlerRefcon )
	OSErr err;
	
	err=GotRequiredParams(theAppleEvent);
	if (!err && reply->dataHandle)				/*	a reply is sought */
		err=AEPutParamPtr(reply,'errs','TEXT',"Ignoring",8);	
	return(err);
	}

static pascal OSErr
DoAEOpenDoc(const AppleEvent *theAppleEvent,AppleEvent *reply,long handlerRefcon)
{
#pragma unused ( reply )
#pragma unused ( handlerRefcon )
	OSErr err;
	AEDescList docList;
	Size actualSize;
	AEKeyword keywd;
	DescType typeCode;
	long itemsInList;
	
	err=AEGetParamDesc(theAppleEvent,keyDirectObject,typeAEList,&docList);
	if (!err) {
		err=GotRequiredParams(theAppleEvent);
		if (!err) {
			err=AECountItems(&docList,&itemsInList);
			if (!err && itemsInList>0) {
				err=AEGetNthPtr(&docList,1,typeFSS,&keywd,&typeCode,(Ptr)&myFSS,
					 sizeof(myFSS),&actualSize);
				if (!err) haveFSS=true;
				}
			}
		AEDisposeDesc(&docList);
		}
	return(err);
	}

char mac_do_script[256] = "";

static pascal OSErr
DoAEDo(const AppleEvent *theAppleEvent,AppleEvent *reply,long handlerRefcon)
{
#pragma unused ( reply )
#pragma unused ( handlerRefcon )
  OSErr err;
  AEDesc desc;
  Size actualSize;
  DescType typeCode;
        
  err=AEGetParamDesc(theAppleEvent,keyDirectObject,typeChar,&desc);
  if (!err) {
    err=GotRequiredParams(theAppleEvent);
    if (!err) {
      err=AEGetParamPtr(theAppleEvent,keyDirectObject,typeChar,
			&typeCode,(Ptr)mac_do_script,256,&actualSize);
      if (err) mac_do_script[0]=0;
      else {
	mac_do_script[255]=0;
	if (actualSize>=0 && actualSize<256) mac_do_script[actualSize]=0;
      }
    }
    AEDisposeDesc(&desc);
  }
  return(err);
}

int mac_quit_now = 0;

static pascal OSErr                                     /* Used for Quit event 
*/
DoAEQuit(const AppleEvent *theAppleEvent,AppleEvent *reply,long handlerRefcon)
{
#pragma unused ( handlerRefcon )
	OSErr err;

	err=GotRequiredParams(theAppleEvent);
	if (!err && reply->dataHandle)						/* a reply is sought */
	      err=AEPutParamPtr(reply,'errs','TEXT',"Ignoring",8);
	mac_quit_now=1;
	SIOUXQuitting=true;
	return(err);
}

#ifndef _MSL_USE_NEW_FILE_APIS				// These are Pro 7 concepts!!
#ifndef _MSL_USE_OLD_FILE_APIS				// If neither is defined, this is Pro 6
#define _MSL_USE_OLD_FILE_APIS		1		// In Pro 6 do it the old way
#endif
#endif

#if _MSL_USE_OLD_AND_NEW_FILE_APIS
extern _MSL_IMP_EXP_C char __msl_system_has_new_file_apis(void);
#endif

static OSErr
OpenFork(FSSpec *spec,char permission,short *refNum)
{
#if _MSL_USE_NEW_FILE_APIS
	FSRef theRef;
#endif
	OSErr err;

#if _MSL_USE_OLD_AND_NEW_FILE_APIS
	if (__msl_system_has_new_file_apis()) {
#endif /* _MSL_USE_OLD_AND_NEW_FILE_APIS */
#if _MSL_USE_NEW_FILE_APIS
		err=FSpMakeFSRef(spec,&theRef);
		if (!err) err=FSOpenFork(&theRef,0,NULL,permission,refNum);
#endif
#if _MSL_USE_OLD_AND_NEW_FILE_APIS
		}
	else {
#endif /* _MSL_USE_OLD_AND_NEW_FILE_APIS */
#if _MSL_USE_OLD_FILE_APIS
		err=FSpOpenDF(spec,permission,refNum);
#endif
#if _MSL_USE_OLD_AND_NEW_FILE_APIS
		}
#endif
	return(err);
	}

char TryAEOpen(void);

char
TryAEOpen(void)
{
	AEEventHandlerUPP HandlerUPP;
	EventRecord myEvent;
	OSErr result;
	long lvalue;
	int cnt;
	short refNum;
	char c[2];

	if (Gestalt(gestaltAppleEventsAttr,&lvalue)!=noErr) return(false);
	result=noErr;
	HandlerUPP=NewAEEventHandlerUPP(&DoAEOpenDoc);
	result=AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,HandlerUPP,0,false);
	if (result==noErr)	{
		HandlerUPP=NewAEEventHandlerUPP(&DoAENoop);
		result=AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,HandlerUPP,0,false);
		}
	if (result==noErr)	{
		HandlerUPP=NewAEEventHandlerUPP(&DoAENoop);
		result=AEInstallEventHandler(kCoreEventClass,kAEPrintDocuments,HandlerUPP,0,false);
		}
	if (result==noErr)	{
		HandlerUPP=NewAEEventHandlerUPP(&DoAEQuit);
		result=AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,HandlerUPP,0,false);
		}
        if (result==noErr)      {
                HandlerUPP=NewAEEventHandlerUPP(&DoAEDo);
                result=AEInstallEventHandler(kAEMiscStandards,kAEDoScript,HandlerUPP,0,false);
                }
	if (result==noErr) {
		for (cnt=0; cnt<40; ++cnt) {
			if (GetNextEvent(everyEvent,&myEvent)) {
				if (myEvent.what==kHighLevelEvent) AEProcessAppleEvent(&myEvent);
				else SIOUXHandleOneEvent(&myEvent);
				}
			}
		}
	if (haveFSS && !OpenFork(&myFSS,fsRdPerm,&refNum)) {
		read(refNum,c,2);
		close(refNum);
		return((c[0]=='#' && c[1]==' ')?'#':' ');
		}
        return(0);
        }

#ifdef USE_ZLIB

#define GZ_NUM_BLOCKS	16

static int gz_sizes[GZ_NUM_BLOCKS] = {
	16,		24,		28,		64,		76,			100,		108,
	140,	2048,	2048,	5816,	11520,		16*1024,	32*1024,
	64*1024,	64*1024 };
static void *gz_blocks[GZ_NUM_BLOCKS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int gz_bflags[GZ_NUM_BLOCKS] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };

static voidpf
my_gz_alloc_func(voidpf opaque,uInt items,uInt size)
{
	int indx;

	size*=items;
	if (size<512) {
		if (size<70) {
			if (size<=16) indx=0;
			else if (size==24) indx=1;
			else if (size==28) indx=2;
			else if (size==64) indx=3;
			else indx=GZ_NUM_BLOCKS;
			}
		else {
			if (size==76) indx=4;
			else if (size==100) indx=5;
			else if (size==108) indx=6;
			else if (size==140) indx=7;
			else indx=GZ_NUM_BLOCKS;
			}
		}
	else {
		if (size<=2048) indx=8;				/* There are two sets of these, indx */
											/* 8 and 9, special case in loop below */
		else if (size==5816) indx=10;
		else if (size==11520) indx=11;
		else if (size==16*1024) indx=12;
		else if (size==32*1024) indx=13;
		else if (size==64*1024) indx=14;	/* There are two sets of these, indx */
											/* 14 and 15, special case in loop below */
		else indx=GZ_NUM_BLOCKS;
		}
	if (indx>=GZ_NUM_BLOCKS) {
#ifdef DEBUG_VERSION
		printf("gz alloc: unusual size, %d bytes\n",size);
#endif
		return(malloc(size));
		}

	while (true) {
	    if (!gz_bflags[indx]) {
	    	gz_bflags[indx]=1;
	    	return(gz_blocks[indx]);
	    	}
	    else if (gz_bflags[indx]==2) {
	    	gz_blocks[indx]=malloc(gz_sizes[indx]);
			gz_bflags[indx]=1;
			return(gz_blocks[indx]);
			}
		if (indx==8 || indx==14) ++indx;
		else break;
		}
#ifdef DEBUG_VERSION
	printf("gz alloc: table in use already, %d bytes\n",size);
#endif
	return(malloc(size));
	}

static void
my_gz_free_func(voidpf opaque,voidpf address)
{
	void **ptr;
	int indx;

	if (!address) return;
	ptr=gz_blocks;
	for (indx=0; indx<GZ_NUM_BLOCKS; ++indx,++ptr) {
		if (address == *ptr) {
			gz_bflags[indx]=0;
			return;
			}
		}
	free(address);
    }

#define BZ_NUM_BLOCKS	4

static void *bz_blocks[BZ_NUM_BLOCKS] = { 0, 0, 0, 0 };
static int bz_bflags[BZ_NUM_BLOCKS] = { 2, 2, 2, 2 };

static void *
my_bz_alloc_func(void *opaque,int items,int size)
{
	int indx;

	size*=items;
	if (size==64116) {
		for (indx=0; indx<BZ_NUM_BLOCKS; ++indx) {
		    if (!bz_bflags[indx]) {
	    		bz_bflags[indx]=1;
	    		return(bz_blocks[indx]);
	    		}
		    else if (bz_bflags[indx]==2) {
		    	bz_blocks[indx]=malloc(64116);
				bz_bflags[indx]=1;
				return(bz_blocks[indx]);
				}
			}
#ifdef DEBUG_VERSION
		printf("bz alloc: all table slots in use already\n");
#endif
		}
#ifdef DEBUG_VERSION
	else printf("bz alloc: unusual block size, %d bytes\n",size);
#endif
	return(malloc(size));
	}

static void
my_bz_free_func(void *opaque,void *addr)
{
	void **ptr;
	int indx;

	if (!addr) return;
	ptr=bz_blocks;
	for (indx=0; indx<BZ_NUM_BLOCKS; ++indx,++ptr) {
		if (addr == *ptr) {
			bz_bflags[indx]=0;
			return;
			}
		}
	free(addr);
    }
#endif

FILE *
mac_fopen(const char *name,const char *mode)
{
	int len;
	short refNum;

	if (!strcmp(name,"~~~") && !strcmp(mode,"r") && haveFSS) {
		if (!OpenFork(&myFSS,fsRdPerm,&refNum)) {
			if (in_fp) return(fdopen(refNum,"r"));
			in_fp=fdopen(refNum,"rb");
			}
		else return(nil);
		}
	else {
		if (strcmp(mode,"r") || in_fp) {
			FILE *fp;
			int omode, fd;

			if (mode[0]=='r') omode=O_RDONLY|O_BINARY;
			else if (mode[0]=='w') omode=O_WRONLY|O_CREAT|O_TRUNC;
			else omode=O_WRONLY|O_CREAT|O_APPEND;
			_fcreator='R*ch';
			_ftype='TEXT';
			fd=open(name,omode);
			_fcreator=_ftype=0;
			if (fd<0) return(nil);
			fp=fdopen(fd,(char *)mode);
			return(fp);
			}
		in_fp=fopen(name,"rb");
		}
	if (in_fp) {
		line_num=buff_cnt=buff_offset=0;
		if (!buff) buff=(char *)malloc(LOC_BUFF_SIZE);
#ifdef USE_ZLIB
		len=fread(buff,1,16,in_fp);
		fseek(in_fp,0,SEEK_SET);
		if (len==16) {
											/* GZip file start with 0x1F, 0x8B, and mode must be DEFLATE(8) */
			if (buff[0]==0x1F && ((unsigned char *)buff)[1]==0x8B && buff[2]==8) {
        		gzsetalloc(my_gz_alloc_func,my_gz_free_func,nil);
				gz_fp=gzfopen(in_fp,"rb");
				in_mode=1;
				}
			else if (buff[0]=='P' && buff[1]=='K' && buff[2]==3 && buff[3]==4) {
				in_mode=0;
				zip_fp=unzOpen(in_fp,my_gz_alloc_func,my_gz_free_func,nil);
				if (zip_fp) {
					if (unzGoToFirstFile(zip_fp)==UNZ_OK) {
						if (unzOpenCurrentFile(zip_fp)==UNZ_OK) {
							in_mode=2;
							zip_eof=false;
							}
						else unzClose(zip_fp);
						}
					else unzClose(zip_fp);
					}
				if (!in_mode) fseek(in_fp,0,SEEK_SET);
				}
											/* BZip2 files start with "BZh<digit>1AY&SY" */
			else if (buff[0]=='B' && buff[1]=='Z' && buff[2]=='h' && 
				 isdigit(buff[3]) && !memcmp("1AY&SY",buff+4,6)) {
				bzsetalloc(my_bz_alloc_func,my_bz_free_func,0);
				bz_fp=BZ2_bzfopen(in_fp,"rb");
				in_mode=3;
				}							/* ZIP files start with "PK\03\04" - we only read deflated and stored items (nothing else tends to occur) */
			else in_mode=0;
			}
		else in_mode=0;
#else
		in_mode=0;
#endif
		FillBuff(0);
		return((FILE *)in_fp);
		}
	return(nil);
	}

int
mac_fclose(FILE *file)
{
	int result;

	if ((gzFile)file==in_fp) {
		switch (in_mode) {

		case 0:
			result=fclose(in_fp);
			break;
#ifdef USE_ZLIB
		case 1:
			result=gzclose(gz_fp);
			break;
		case 2:
			if (!zip_eof) unzCloseCurrentFile(zip_fp);
			unzClose(zip_fp);
			result=0;
			break;
		case 3:
			BZ2_bzclose(bz_fp);
			result=0;
			break;
#endif
			}
		in_fp=nil;
		}
	else result=fclose(file);
	return(result);
	}

size_t
mac_fread(void *ptr,size_t size,size_t num,FILE *file)
{
	size_t total;
	long len;

	if (in_fp!=(gzFile)file) return(fread(ptr,size,num,file));
	num*=size;
	total=0;
	while (num>0) {
		if (buff_cnt<=0) return(total);
		len=buff_cnt;
		if (len>num) len=num;
		memcpy(ptr,buff+buff_offset,len);
		((char *)ptr)+=len;
		FillBuff(len);
		num-=len;
		total+=len;
		}
	return(total/size);
	}

int
mac_getc(FILE *file)
{
	int result;

	if (in_fp!=(gzFile)file) return(getc(file));
	if (buff_cnt<=0) return(EOF);
	result = *((unsigned char *)buff+buff_offset);
	FillBuff(1);
	return(result);
	}

int
mac_feof(FILE *file)
{
	if (in_fp!=(gzFile)file) return(feof(file));
	return(buff_cnt<=0);
	}

#endif


#define PNG_NUM_BLOCKS	11

static int png_sizes[PNG_NUM_BLOCKS] = {
	10,		10,		80,		560,	700,			/* 80 used for 64, 700 used for 601 */
	5816,	8192,	65536,	65536,	65536,
	65536 };

void *png_blocks[PNG_NUM_BLOCKS] = { 0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0, 0, 0 };	/* address of the block */
int png_bflags[PNG_NUM_BLOCKS] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };	/* 0-available, 1-in use, 2-not allocated */

extern void *mac_png_malloc(unsigned long size);
extern void mac_png_free(void *ptr);

void *						/* Allocate memory - use cached block if size is right */
mac_png_malloc(unsigned long size)
{
	int indx;

#ifdef DEBUG_VERSION
//	printf("png alloc: %d bytes\n",size);
#endif
	if (size<1000) {
		if (size==10) indx=0;			/* There are two sets of these, indx */
										/* 0 and 1, special case in loop below */
		else if (size<=80) indx=2;
		else if (size==560) indx=3;
		else if (size<=700) indx=4;
		else indx=PNG_NUM_BLOCKS;
		}
	else {
		if (size==5816) indx=5;
		else if (size==8192) indx=6;
		else if (size==65536) indx=7;	/* There are four sets of these, indx */
										/* 7, 8, 9, and 10, special case in loop below */
		else indx=PNG_NUM_BLOCKS;
		}
	if (indx<PNG_NUM_BLOCKS) {
		while (true) {
			if (!png_bflags[indx]) {
		    	png_bflags[indx]=1;
		    	return(png_blocks[indx]);
				}
			else if (png_bflags[indx]==2) {
				png_blocks[indx]=malloc(png_sizes[indx]);
		    	png_bflags[indx]=1;
		    	return(png_blocks[indx]);
				}
			if (indx==0 || (indx>=7 && indx<=9)) ++indx;
			else break;
			}
#ifdef DEBUG_VERSION
		printf("png alloc: table in use already, %d bytes\n",size);
#endif
		return(malloc(size));
		}
#ifdef DEBUG_VERSION
	printf("png alloc: unusual size, %d bytes\n",size);
#endif
	return(malloc(size));
	}

void								/* Free memory - put blocks back in the cache */
mac_png_free(void *address)
{
	void **ptr;
	int indx;

	if (!address) return;
	ptr=png_blocks;
	for (indx=0; indx<PNG_NUM_BLOCKS; ++indx,++ptr) {
		if (address == *ptr) {
			png_bflags[indx]=0;
			return;
			}
		}
	free(address);
    }

#endif

/* END OF macinput.c - read input files */
