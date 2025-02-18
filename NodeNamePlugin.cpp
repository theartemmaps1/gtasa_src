//
//
//    Filename: NodeNamePlugin.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Plugin to add node name to RwFrame in RenderWare DFF exporter
//
//
// C headers
#include <string.h>
// Game headers
#include "NodeNamePlugin.h"
#include "rwplugin.h"

#define NODENAME_LEN	24
#define NODENAMEEXTFROMFRAME(frame)	((char*)(((RwUInt8*)frame) + gPluginOffset))

//global variables
static RwInt32 gPluginOffset;

#ifndef RWASSERT
#ifndef FINAL
#define RWASSERT(CONDITION)										\
		if (!(CONDITION))											\
		{															\
			printf("ASSERT File:%s Line:%d\n", __FILE__, __LINE__);	\
			asm volatile {"breakc 1"};								\
		}
#else
#define RWASSERT(CONDITION)	
#endif RWASSERT(CONDITION)
#endif

static void* NodeNameConstructor(void* frame, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	if(gPluginOffset > 0)
	{
		char *pExtData = NODENAMEEXTFROMFRAME(frame);
		// frame has no name to start with
		*pExtData = '\0';
	}
	return frame;
}

static void* NodeNameDestructor(void* frame, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	return frame;
}

static void* NodeNameCopy(void* pDst, const void* pSrc, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	strncpy(NODENAMEEXTFROMFRAME(pDst), NODENAMEEXTFROMFRAME(pSrc), NODENAME_LEN-1);
	return pDst;
}

static RwStream* NodeNameStreamWrite(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size)
{
	char *pExtData = NODENAMEEXTFROMFRAME(pData);

	RwStreamWrite(pStream, pExtData, len);

	return pStream;
}

static RwStream* NodeNameStreamRead(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size)
{
	char *pExtData = NODENAMEEXTFROMFRAME(pData);

#ifdef GTA_PS2
	RWASSERT(len<=NODENAME_LEN);
#endif
	
	RwStreamRead(pStream, pExtData, len);
	
	*(pExtData + len) = '\0';


	return pStream;
}

static RwInt32 NodeNameStreamGetSize(const void* pData, RwInt32 offset, RwInt32 size)
{
	char *pExtData = NODENAMEEXTFROMFRAME(pData);

	if(pExtData)
	{
		return rwstrlen(pExtData);
	}
	return 0;
}

//
//        name: NodeNamePluginAttach
// description: Register Max NodeName plugin
//         out: successful
//
RwBool NodeNamePluginAttach()
{
	gPluginOffset = RwFrameRegisterPlugin(NODENAME_LEN, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwNODENAME_ID),
											NodeNameConstructor,
											NodeNameDestructor,
											NodeNameCopy);
	RwFrameRegisterPluginStream(MAKECHUNKID(rwVENDORID_ROCKSTAR, rwNODENAME_ID), 
											NodeNameStreamRead,
											NodeNameStreamWrite,
											NodeNameStreamGetSize);
	return (gPluginOffset != -1);
}

//
//        name: SetAtomicNodeName
// description: Export node name into atomic extension plugin space
//          in: pNode = pointer to node
//				pAtomic = pointer to atomic
//
void SetFrameNodeName(RwFrame *pFrame, const char* pName)
{
	char *pExtData = NODENAMEEXTFROMFRAME(pFrame);

	if(gPluginOffset > 0)
	{
		strncpy(pExtData, pName, NODENAME_LEN - 1);
		*(pExtData + NODENAME_LEN - 1) = '\0';
	}
}

const char* GetFrameNodeName(RwFrame *pFrame)
{
	char *pExtData = NODENAMEEXTFROMFRAME(pFrame);

	if(gPluginOffset > 0)
		return pExtData;
	return NULL;
}



