//
//
//    Filename: PipelinePlugin.c
//     Creator: John Gurney
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Plugin to add a pipeline id to atomics in RenderWare DFF exporter
//
//
#include "PipelinePlugin.h"
#include "rwplugin.h"
#include "Debug.h"

#define PIPELINE_LEN					(4)
#define PIPELINEEXTFROMATOMIC(atomic)	((uint32*)(((RwUInt8*)atomic) + gPluginOffset))


//global variables
static RwInt32 gPluginOffset=-1;

//
//
//
//
static void* PipelineConstructor(void* atomic, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	if(gPluginOffset > 0)
	{
		uint32 *pipelineID = PIPELINEEXTFROMATOMIC(atomic);
		*pipelineID = 0;
	}
	return atomic;
}

//
//
//
//
static void* PipelineCopy(void* pDst, const void* pSrc, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	*PIPELINEEXTFROMATOMIC(pDst) = *PIPELINEEXTFROMATOMIC(pSrc);
	return pDst;
}

//
//
//
//
static RwStream* PipelineStreamRead(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size)
{
	char* pipelineID = (char*) PIPELINEEXTFROMATOMIC(pData);

	ASSERT(len==PIPELINE_LEN);
	
	RwStreamRead(pStream, pipelineID, len);
	
	return pStream;
}

//
//
//
//
static RwInt32 PipelineStreamGetSize(const void* pData, RwInt32 offset, RwInt32 size)
{
	return PIPELINE_LEN;
}


//
//        name: PipelinePluginAttach
// description: Register Max pipeline ID plugin
//         out: successful
//
RwBool PipelinePluginAttach()
{
	gPluginOffset =  RpAtomicRegisterPlugin(	PIPELINE_LEN,
												MAKECHUNKID(rwVENDORID_ROCKSTAR, rwPIPELINE_ID),
												PipelineConstructor,
												NULL,
												PipelineCopy);
	
	if(gPluginOffset != -1)
	{
		if(RpAtomicRegisterPluginStream(	MAKECHUNKID(rwVENDORID_ROCKSTAR, rwPIPELINE_ID), 
											PipelineStreamRead,
											NULL,
											PipelineStreamGetSize) < 0)
			gPluginOffset = -1;
	}
		
	return (gPluginOffset != -1);
}

//
//
//
//
const uint32 GetPipelineID(RpAtomic *pAtomic)
{
	ASSERT(gPluginOffset > 0);

	uint32 pipelineID = *((uint32*)PIPELINEEXTFROMATOMIC(pAtomic));
	return(pipelineID);
}

//
//
//
//
uint32 SetPipelineID(RpAtomic *pAtomic, uint32 pipelineID)
{
	ASSERT(gPluginOffset > 0);

	*((uint32*)PIPELINEEXTFROMATOMIC(pAtomic)) = pipelineID;
	return(pipelineID);
}


