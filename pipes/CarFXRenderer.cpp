//
// CarFXRenderer - custom vehicle renderer for d3d9;
//
//	15/10/2004	- JohnW:	- early port to PC; 
//	25/01/2005	- Andrzej:	- initial;
//	28/01/2005	- Andrzej:	- class renamed to CCarFXRenderer;
//	02/02/2005	- Andrzej:	- PreRenderUpdate() added;
//	01/03/2005	- Andrzej:	- added support for GTA_XBOX;
//
//
//
//
#include "dma.h"
#include "rwPlugin.h"
#include "MemoryMgr.h"
#include "TxdStore.h"
#include "ClothesBuilder.h"
#include "VehicleModelInfo.h"
#include "PipelinePlugin.h"
#include "CarFXRenderer.h"

#ifdef GTA_PC
	#include "CustomCarEnvMapPipeline.h"
#endif
#ifdef GTA_XBOX
	#include "CustomCarEnvMapPipeline_Xbox.h"
#endif

//
//
//
//
RwTexture*	CCarFXRenderer::ms_aDirtTextures[CARFX_NUM_DIRT_LEVELS] = {NULL};


//
//
//
//
RwBool CCarFXRenderer::Initialise()
{
	if(!CCustomCarEnvMapPipeline::CreatePipe())
		return(FALSE);

	return(TRUE);
}

//
//
// clean up the dirt textures etc.
//
void CCarFXRenderer::Shutdown()
{

	for(int32 i=0; i<CARFX_NUM_DIRT_LEVELS; i++)
	{
		if (ms_aDirtTextures[i])
		{
			RwTextureDestroy(ms_aDirtTextures[i]);
			ms_aDirtTextures[i] = NULL;
		}
	}

	CCustomCarEnvMapPipeline::DestroyPipe();

}// end of CVuCarFXRenderer::Shutdown()...

//
//
//
//
RwBool CCarFXRenderer::RegisterPlugins()
{
	if(!CCustomCarEnvMapPipeline::RegisterPlugin())
	{
		return(FALSE);
	}

	return(TRUE);
}


//
//
//
//
void CCarFXRenderer::PreRenderUpdate()
{
	CCustomCarEnvMapPipeline::PreRenderUpdate();
}


//
//
//
//
//
RpAtomic* CCarFXRenderer::CustomCarPipeAtomicSetup(RpAtomic *pAtomic)
{
	// force linear filtering on all atomic's textures:
	//	SetFilterModeOnAtomicsTextures(pAtomic, rwFILTERLINEAR);
	return(CCustomCarEnvMapPipeline::CustomPipeAtomicSetup(pAtomic));
}

//
//
//
//
//
static
RpAtomic* customCarPipelineSetupAtomicCB(RpAtomic *pAtomic, void *data __RWUNUSED__)
{
	return(CCarFXRenderer::CustomCarPipeAtomicSetup(pAtomic));
}

//
//
//
//
RpClump* CCarFXRenderer::CustomCarPipeClumpSetup(RpClump *pClump)
{
	RpClumpForAllAtomics(pClump, customCarPipelineSetupAtomicCB, NULL);
	return(pClump);
}


#ifndef FINAL
	// temp pointer to name of model being loaded (see below):
	char *carfxDbgLoadedModelName = NULL;
#endif

//
//
//
//
RpAtomic* CCarFXRenderer::SetCustomFXAtomicRenderPipelinesVMICB(RpAtomic* pAtomic, void* pData)
{
	ASSERT(pAtomic);

#ifndef FINAL
	// check for proper pipeline ID (loaded model should be properly exported):
	if(!CCarFXRenderer::IsCCPCPipelineAttached(pAtomic))
	{
		char buf[128];
		if(carfxDbgLoadedModelName)
			::sprintf(buf, "Loaded car model '%s' had no proper pipeline ID. See PaulK to fix this.", carfxDbgLoadedModelName);
		else
			::sprintf(buf, "Loaded car model had no proper pipeline ID. See PaulK to fix this.");
		ASSERTMSG(FALSE, buf);
	}
#endif//FINAL

	// set CustomCarFX pipeline:
	CCarFXRenderer::CustomCarPipeAtomicSetup(pAtomic);

	return(pAtomic);
}


//
//
//
//
RwBool CCarFXRenderer::IsCCPCPipelineAttached(RpAtomic *pAtomic)
{
	uint32 pipeID = GetPipelineID(pAtomic);

	if(pipeID == PDSPC_CarFXPipe_AtmID)
		return(TRUE);

	return(FALSE);
}


//
//
//
//
void CCarFXRenderer::SetFxEnvMapLightMult(float m)
{
	CCustomCarEnvMapPipeline::SetEnvMapLightingMult(m);
}

float CCarFXRenderer::GetFxEnvMapLightMult()
{
	return(CCustomCarEnvMapPipeline::GetEnvMapLightingMult());
}





/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DIRT TEXTURE:
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
RwBool CCarFXRenderer::InitialiseDirtTexture()
{
//!XBOX - can't do this yet on xbox
#if defined (GTA_XBOX)
	return(FALSE);
#endif //GTA_XBOX

	// Load texture dictionary
	int32 vehicleTxdId = CTxdStore::FindTxdSlot("vehicle");
	ASSERT(vehicleTxdId != -1);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(vehicleTxdId);

	RwTexture* pDirtTex;
	pDirtTex = (RwTextureRead(RWSTRING(CARFX_DIRT_TEXTURE_NAME),NULL));
	RwTextureSetFilterMode(pDirtTex, rwFILTERLINEAR);
	RwRaster* pOldDirtRaster = RwTextureGetRaster(pDirtTex);

	const int32 drDepth = RwRasterGetDepth(pOldDirtRaster);
	const int32 drWidth = RwRasterGetWidth(pOldDirtRaster);
	const int32 drHeight= RwRasterGetHeight(pOldDirtRaster);

	ASSERTMSG(drDepth == 32, "CarFXRenderer: Bad dirt texture depth!");
	{	
		const int32 format = RwRasterGetFormat(pOldDirtRaster);
		ASSERT(format != -1);
		
		const int32 colorFormat = format&rwRASTERFORMATPIXELFORMATMASK;

		// allow only RGBA8888 palette:
		ASSERTMSG((colorFormat == rwRASTERFORMAT888 || colorFormat == rwRASTERFORMAT8888), "CarFXRenderer: Bad dirt format (not RGB888)!");
	}

	for(int32 i=0; i<CARFX_NUM_DIRT_LEVELS;i++)
	{
		// get a copy of the dirt texture into our array of dirt textures
		ms_aDirtTextures[i] = CClothesBuilder::CopyTexture(pDirtTex);
		RwTextureSetName(ms_aDirtTextures[i], CARFX_DIRT_TEXTURE_NAME);

		// prepare it for editting
		RwRaster* pDirtRaster = RwTextureGetRaster(ms_aDirtTextures[i]);
		RwUInt8* pPixelData = RwRasterLock(pDirtRaster, 0, rwRASTERLOCKWRITE | rwRASTERLOCKREAD);
		ASSERT(pPixelData);
		UInt32	pixelIndex = 0;
		float	scale, invScale;

		// modify the pixels in the raster to the correct dirt level for this texture
		for(int j = 0; j< drHeight; j++)
		{
			for(int k = 0; k< drWidth; k++)
			{
				pixelIndex = ((j*drWidth) + k)*4;

				pPixelData[pixelIndex] = ((pPixelData[pixelIndex] * i / CARFX_NUM_DIRT_LEVELS) + 
											(0xff * (CARFX_NUM_DIRT_LEVELS - i) / CARFX_NUM_DIRT_LEVELS));		//red
				pPixelData[pixelIndex+1] = ((pPixelData[pixelIndex+1] * i / CARFX_NUM_DIRT_LEVELS) + 
											(0xff * (CARFX_NUM_DIRT_LEVELS - i) / CARFX_NUM_DIRT_LEVELS));		//green
				pPixelData[pixelIndex+2] = ((pPixelData[pixelIndex+2] * i / CARFX_NUM_DIRT_LEVELS) + 
											(0xff * (CARFX_NUM_DIRT_LEVELS - i) / CARFX_NUM_DIRT_LEVELS));		//blue
				pPixelData[pixelIndex+3] = pPixelData[pixelIndex+3];		//alpha

			}
		}

		RwRasterUnlock(pDirtRaster);
	}

	CTxdStore::PopCurrentTxd();

	return(TRUE);
}

//
//
//
//
RpMaterial* CCarFXRenderer::MaterialRemapDirtCB(RpMaterial* pMaterial, void* pData)
{
	UInt32 dirtLevel = (UInt32) pData;
	RwTexture* pTexture = RpMaterialGetTexture(pMaterial);

	if (pTexture)
	{
		//if this is a dirt texture then remap it
		if (rwstrcmp(RwTextureGetName(pTexture), CARFX_DIRT_TEXTURE_NAME) == 0)
		{
			RpMaterialSetTexture(pMaterial, ms_aDirtTextures[dirtLevel]);
		}
	}
	return(pMaterial);
}

//
//
//
//
RpAtomic* CCarFXRenderer::AtomicRemapDirtCB(RpAtomic* pAtomic, void* pData)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);

	RpGeometryForAllMaterials(pGeom, MaterialRemapDirtCB, pData);

	return(pAtomic);
}

//
//
// remap the car textures which use vehicleGrunge256 to use the right dirt level texture
//
void CCarFXRenderer::RemapDirt(CVehicleModelInfo* pModelInfo, UInt32 dirtLevel)
{
	ASSERT(dirtLevel <= CARFX_NUM_DIRT_LEVELS);
	ASSERT(pModelInfo);

//	RpClump *pClump = pModelInfo->GetClump();
//	RpClumpForAllAtomics(pClump, AtomicRemapDirtCB, (void*)dirtLevel);
	
	for(int32 i=0; i<MAX_PER_VEHICLE_DIRT_MATERIALS; i++)
	{
		RpMaterial* pMaterial = pModelInfo->GetDirtMaterial(i);
		if (pMaterial != NULL)
		{
			RpMaterialSetTexture(pMaterial, ms_aDirtTextures[dirtLevel]);
		}
	}

}







