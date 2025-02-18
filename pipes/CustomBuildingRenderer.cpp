//
// CustomBuildingRenderer - custom building renderer for d3d9;
//
//	15/10/2004	-	JohnW:		- early port to PC;
//	03/02/2005	-	Andrzej:	- initial;
//
//
//
//
//
#include "dma.h"
#include "MemoryMgr.h"
#include "rwPlugin.h"
#include "PipelinePlugin.h"
#ifdef GTA_XBOX
	#include "hdstream.h"
#endif

#ifdef GTA_PC
	#include "CustomBuildingPipeline.h"
	#include "CustomBuildingDNPipeline.h"
#endif
#ifdef GTA_XBOX
	#include "CustomBuildingPipeline_Xbox.h"
	#include "CustomBuildingDNPipeline_Xbox.h"
#endif

#include "CustomBuildingRenderer.h"
#include "clock.h"

#ifdef GTA_XBOX
	#define PDS_CustomXBoxColourNormal_AtmID	(rpPDSPC_MAKEPIPEID(rwVENDORID_ROCKSTAR,	rwCUSTPDS_CVBDNUVAXBOX_ATM_PIPE_ID))
#endif



//
//
//
//
CCustomBuildingRenderer::CCustomBuildingRenderer()
{
	// do nothing
}

//
//
//
//
CCustomBuildingRenderer::~CCustomBuildingRenderer()
{
	// do nothing
}




//
//
//
//
RwBool CCustomBuildingRenderer::Initialise()
{
	if(!CCustomBuildingPipeline::CreatePipe())
	{
		return(FALSE);
	}
	
	if(!CCustomBuildingDNPipeline::CreatePipe())
	{
		return(FALSE);
	}
	
	return(TRUE);
}

//
//
//
//
void CCustomBuildingRenderer::Shutdown()
{
	CCustomBuildingPipeline::DestroyPipe();
	CCustomBuildingDNPipeline::DestroyPipe();
}


//
//
//
//
RwBool CCustomBuildingRenderer::PluginAttach()
{
	if(!CCustomBuildingDNPipeline::ExtraVertColourPluginAttach())
		return(FALSE);

	return(TRUE);
}


//
//
//
//
RpAtomic* CCustomBuildingRenderer::AtomicSetup(RpAtomic *pAtomic)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeom);

#if defined(GTA_XBOX)
	if(CCustomBuildingRenderer::IsCBPCPipelineAttached(pAtomic))
	{
		// DN lighting:
		return(CCustomBuildingDNPipeline::CustomPipeAtomicSetup(pAtomic));
	}
	else
	{
		// standard lighting:
		return(CCustomBuildingPipeline::CustomPipeAtomicSetup(pAtomic));
	}
#endif //GTA_XBOX

#if defined(GTA_PC)
	if(	CCustomBuildingDNPipeline::GetExtraVertColourPtr(pGeom) &&
		RpGeometryGetPreLightColors(pGeom)						)
	{
		// DN lighting:
		return(CCustomBuildingDNPipeline::CustomPipeAtomicSetup(pAtomic));
	}
	else
	{
		// standard lighting:
		return(CCustomBuildingPipeline::CustomPipeAtomicSetup(pAtomic));
	}
#else
	return NULL;
#endif

}

//
//
// returns TRUE, if atomic contains DN data
//
RwBool CCustomBuildingRenderer::IsCBPCPipelineAttached(RpAtomic *pAtomic)
{
#ifdef GTA_XBOX

	#ifdef HDSTREAM_DEBUG
		return(TRUE);	// this is for colouring the map red or green to show how the hard drive streaming is working
	#endif

		// XBox atomics have a plugin attached containing a pipeline ID which signifies that this
		// atomic uses the DN stuff and has its night colours are held in the vertex normals
		const uint32 pipelineID = GetPipelineID(pAtomic);
		if(pipelineID == PDS_CustomXBoxColourNormal_AtmID)
		{
			return(TRUE);
		}

#else // GTA_XBOX

	// GTA_PC:
	const uint32 pipelineID = GetPipelineID(pAtomic);
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);

	// standard way of detecting these pipelines:
	if(	(pipelineID == PDSPC_CBuildingEnvMap_AtmID)		||
		(pipelineID == PDSPC_CBuildingDNEnvMap_AtmID)	)
	{
		return(TRUE);
	}
	// GTA_PC: other way to detect whether something is a building:
	else if(CCustomBuildingDNPipeline::GetExtraVertColourPtr(pGeom) &&
			RpGeometryGetPreLightColors(pGeom)						)
	{
		return(TRUE);
	}

#endif

    return(FALSE);
}





//
// updates Day&Night balance up to current in-game time;
// doesn't need to be called every frame;
// 
//
void CCustomBuildingRenderer::UpdateDayNightBalanceParam()
{

//
// note: N2D ("Night-2-Day") start hour must be SMALLER than D2N ("Day-2-Night") start hour:
//
#define	FADE_N2D_START_HOUR		(6  * 60.0f)	// night -> day hour fade start (in minutes)
#define FADE_N2D_LEN			(60.0f)			// fadelen (in minutes)
#define FADE_N2D_STOP_HOUR		(FADE_N2D_START_HOUR+FADE_N2D_LEN)

#define	FADE_D2N_START_HOUR		(20 * 60.0f)	// day -> night hour fade start (in minutes)
#define FADE_D2N_LEN			(60.0f)			// fadelen (in minutes)
#define FADE_D2N_STOP_HOUR		(FADE_D2N_START_HOUR+FADE_D2N_LEN)


 
	//
	// "Day&Night" Balance (fBDN) parameter:
	//
	// fDBN describes how day&night color components are mixed:
	// 0.0	->	full day color
	// 0.5	->	half day / half night color
	// 1.0	->	full night color
	//
	float fBDN = 0.0f;
	const float minutes = CClock::GetMinutesSinceMidnight();

	if(minutes < FADE_N2D_START_HOUR)
	{
		// "night" verts:
		fBDN = 1.0f;
	}
	else if(minutes < FADE_N2D_STOP_HOUR)
	{
		// "night->day" fading (1.0f -> 0.0f):
		fBDN = (FADE_N2D_STOP_HOUR - minutes) / (FADE_N2D_LEN);
	}
	else if(minutes < FADE_D2N_START_HOUR)
	{
		// "day" verts:
		fBDN = 0.0f;
	}
	else if(minutes < FADE_D2N_STOP_HOUR) 
	{
		// "day->"night" fading (0.0f -> 1.0f):
		//...
		fBDN = 1.0f - ((FADE_D2N_STOP_HOUR - minutes) / (FADE_D2N_LEN));
	}
	else
	{
		// "night" verts:
		fBDN = 1.0f;
	}



	ASSERT(fBDN >= 0.0f);
	ASSERT(fBDN <= 1.0f);
	//if(fBDN < 0.0f)		
	//	fBDN = 0.0f;
	//else if(fBDN > 1.0f)
	//	fBDN = 1.0f;


	CCustomBuildingRenderer::SetDayNightBalanceParam(fBDN);
}


//
//
//
//
void CCustomBuildingRenderer::Update()
{
	CCustomBuildingRenderer::UpdateDayNightBalanceParam();

#ifndef OSW
	CCustomBuildingDNPipeline::Update(FALSE);
#endif
}



