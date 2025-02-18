//	===========================================================================
//	overlay.cpp			�1999-2000 Metrowerks Inc. All rights reserved.
//	===========================================================================
//
//	Metrowerks Utilities
//
//	Version.1.5.8
//
//  12/09/2001  rcampana,	modified mwDbgVifBreak() and mwDbgDmaBreak() to
//							assembly
//	06/07/2000	kashima,	modify mwDbgVifHandler() for Runtime Library 1.6
//	05/17/2000	kashima,	add VU debugging utility functions
//	01/25/2000	kashima,	change mwNotifyOverlayLoaded()
//	01/13/2000	kashima,	add overlay support
//  03/05/2003  adamf,		Changed to work with GTA engine
/*******************************************************************************
	INCLUDE
*******************************************************************************/
/*	CW header	*/
#include "mwUtils.h"
#include "mwUtils_PS2.h"

// Game headers
#include "skyfs.h"
#include "Debug.h"
#include "FileMgr.h"
#include "Timer.h"
#include "streaming.h"
#include "loadingscreen.h"
#include "frontend.h"

//#define USE_HOST_OVERLAYS

/*******************************************************************************
	DEFINE
*******************************************************************************/

#ifndef	WORD
#define	WORD	unsigned int
#endif
#ifndef	BYTE
#define	BYTE	unsigned char
#endif

/*******************************************************************************
	TYPEDEF
*******************************************************************************/

/*******************************************************************************
	PROTOTYPE
*******************************************************************************/
/*------------------------------------------------------------------------------
	mwOverlayHeader
------------------------------------------------------------------------------*/
typedef struct	mwOverlayHeader
{
	BYTE	identifier[3];	/*	"MWo"	*/
	BYTE	version;
	WORD	id;
	WORD 	address;
	WORD 	sz_text;
	WORD 	sz_data;
	WORD	sz_bss;
	WORD 	_static_init;
	WORD 	_static_init_end;
	BYTE	name[32];
}	mwOverlayHeader;

/*******************************************************************************
	GLOBALS
*******************************************************************************/
static bool gbOverlayInUse = false;
static char gOverlayLoaded[32] = "";

/*******************************************************************************
	EXTERN
*******************************************************************************/

#ifdef __cplusplus
extern	"C"	{
#endif

// override the notify debugger stuff so that it works in a release build as well
#undef mwOverlayNotifyToDebugger
#ifndef	MASTER
#define	mwOverlayNotifyToDebugger	MWNotifyOverlayLoaded
void	mwOverlayNotifyToDebugger(const void* pAddress);
#else
#define	mwOverlayNotifyToDebugger(pAddress)
#endif

/*------------------------------------------------------------------------------
 *	for static initializer
------------------------------------------------------------------------------*/
extern	void	__initialize_cpp_rts(void** si, void** sie, void* ets, void* ete);

/*------------------------------------------------------------------------------
 *	mwOverlayNotifyToDebugger
 *	notify to debugger
 *	--- CAUTION ---
 *	you have to call this function after the overlay file is loaded
------------------------------------------------------------------------------*/
#ifndef	MASTER
#pragma optimization_level 0
#pragma dont_inline on
#pragma	auto_inline	off
asm void mwOverlayNotifyToDebugger(const void*	pAddress)
{
#pragma	unused(pAddress)
	jr	ra
	nop
}
#pragma	auto_inline	reset
#pragma optimization_level reset
#pragma dont_inline reset
#endif

#pragma	mark	-

#ifdef __cplusplus
}
#endif	/* __cplusplus */

/*------------------------------------------------------------------------------
 *	mwOverlayInit
 *	initialize overlay
 *	--- CAUTION ---
 *	you have to call this function after the overlay file is loaded
------------------------------------------------------------------------------*/
void mwOverlayInit(const void* pAddress, int inFileSize)
{
	unsigned char*		pPtr	= (unsigned char*)pAddress;
	mwOverlayHeader*	pOH		= (mwOverlayHeader*)pAddress;
	const unsigned int	nSize	= pOH->sz_bss;

	/* invalidate i-cache */
	FlushCache(INVALIDATE_ICACHE);		/* invalid instruction cashe */

	/* clear bss section*/
	if (nSize > 0)
	{
		pPtr	+= inFileSize;
		memset(pPtr, 0, nSize);
	}

	/* call static initializer of overlay */
	__initialize_cpp_rts((void**)pOH->_static_init, (void**)pOH->_static_init_end, NULL, NULL);
}

/*------------------------------------------------------------------------------
 *	mwBload
 *	utility function to load binary file into memory
------------------------------------------------------------------------------*/
int mwBload(const char* pFilePath, const void* pAddress)
{
	int		rfd			= -1;
	void*	pBuffer		= NULL;
	int		size		= 0;
	int		readsize		= 0;

//	if(IsFileSystemInstalled())
//	{
//		return CFileMgr::LoadFile(pFilePath, (unsigned char*)pAddress);
//	}
#ifdef CDROM
#ifdef USE_HOST_OVERLAYS
	sprintf(gString, "host0:%s", pFilePath);
	strupr(gString + 6);
#else
	// If DVD is dual layer then assume overlay is on layer 1
	if(CFileMgr::IsDVDDualLayer())
		sprintf(gString, "cdrom1:\\%s", pFilePath);
	else	
		sprintf(gString, "cdrom0:\\%s", pFilePath);
	strupr(gString + 8);
#endif	
#else
	SkyPostProcessFilename(gString, pFilePath);
#endif
	
	/*	open file	*/
	rfd = sceOpen((char*)gString, SCE_RDONLY);
	
	if(rfd < 0)
	{
		CLoadingScreen::Pause();
		while(rfd < 0)
		{
			FrontEndMenuManager.MessageScreen("CDERROR", true);
			rfd = sceOpen((char*)gString, SCE_RDONLY);
		}
		CLoadingScreen::Continue();
	}
	
	size = sceLseek(rfd, 0, SCE_SEEK_END);
	sceLseek(rfd, 0, SCE_SEEK_SET);

	
	/*	read file into memory	*/
	if (size > 0)
	{
		readsize = sceRead(rfd, (void *)pAddress, size);
		if(readsize != size)
		{
			CLoadingScreen::Pause();
			while(readsize != size)
			{
				FrontEndMenuManager.MessageScreen("CDERROR", true);
				readsize = sceRead(rfd, (void *)pAddress, size);
			}
			CLoadingScreen::Continue();
		}
	}

    sceClose(rfd);

	return	size;
}

/*------------------------------------------------------------------------------
 *	mwLoadOverlay
 *	utility function to load overlay file
------------------------------------------------------------------------------*/
int mwLoadOverlay(const char* pFilePath, const void* pAddress)
{
	int	size	= 0;
	int	result	= FALSE;

	// If overlay is already loaded then return 
	if(!strcmp(pFilePath, gOverlayLoaded))
		return TRUE;

	if(CStreaming::IsInitialised())
		CStreaming::FlushChannels();
	
	ASSERTMSG(gbOverlayInUse == false, "An overlay is currently in use");

	printf("Loading code overlay %s\n", pFilePath);

	/* invalidate i-cache */
	FlushCache(INVALIDATE_ICACHE);		/* invalid instruction cashe */

	CTimer::Suspend();
		
	/* load overlay file */
	size	= mwBload(pFilePath, pAddress);
	ASSERTOBJ (size > 0, pFilePath, "Failed to load overlay");

	/* notify to debugger */
	mwOverlayNotifyToDebugger(pAddress);
	/* initialize overlay */
	mwOverlayInit(pAddress, size);

	// copy overlay name
	strcpy(gOverlayLoaded, pFilePath);

	result	= TRUE;

	CTimer::Resume();

	return	result;
}


//
// name:		mwFreeOverlay
// description:	Set the current overlay to be unused, so it is allowed to be overwritten
void mwFreeOverlay()
{
	gbOverlayInUse = false;
	gOverlayLoaded[0] = '\0';
}

const char* GetLoadedOverlayName()
{
	return gOverlayLoaded;
}