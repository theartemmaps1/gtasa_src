//
// name:		ps2all.cpp
// description:	Our own version of ps2all which should run a little faster 
//				than the generic one in RW
// written by:	Adam Fowler
// date:		3/7/02
//
//	2004/03/15	- Andrzej:	- some code cleanup applied;
//
//
//
//

// RW headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
//$DW$#include <rppds.h>

// game headers
#include "rwplugin.h"
#include "ps2all.h"
#include "matrix.h"
#include "lights.h"
#include "VehicleModelInfo.h"

#include "CustomPipelineCommon.h"
#include "VuCustomBuildingRenderer.h"
//#include "VuCustomSkinPedRenderer.h"




//
//
//
//
static bool8 bPS2AllSetupLighting	= TRUE;
static bool8 bLastGeomLit			= TRUE;


#ifndef FINALBUILD
	bool 	bDisplay32bitTextures = false;
	int32 	num32bitTextures;
	char 	textures32bit[16][48];

	int32 	numAtomicsRendered = 0;
#endif


extern RpLight *pAmbient;
extern RpLight *pDirect;
extern RpLight *pExtraDirectionals[NUM_EXTRA_DIRECTIONALS];
extern int32 NumExtraDirLightsInWorld;

// pipelines created
RxPipeline* pOptimisedLightPipe;
RxPipeline* pOptimisedLightSkinnedPipe;
RxPipeline* pVehicleColourMaterialPipe;


RwRGBA gPS2AllVehicleColour1;
RwRGBA gPS2AllVehicleColour2;
RwRGBA gPS2AllVehicleColour3;


//
// name:		ApplyAmbientLight
// description:	send ambient lighting info
//
static
void ApplyAmbientLight(RpLight* pLight)
{
    const RwRGBAReal *color = &pLight->color;
   
	// write colour 
   	*(_rwSkyLightFillPos+0)				= color->red;
	*(_rwSkyLightFillPos+1) 			= color->green;
	*(_rwSkyLightFillPos+2) 			= color->blue;
	// write type
	*((RwUInt32*)(_rwSkyLightFillPos+3))= rpLIGHTAMBIENT;

	_rwSkyLightFillPos += 4;

	_rwSkyLightQWordsWritten++; 
}

//
// name:		ApplyDirectionalLight
// description:	send directional lighting info
//
static
void ApplyDirectionalLight(RpLight* pLight)
{

const RwV3d* pAt = RwMatrixGetAt(&(RpLightGetFrame(pLight)->modelling));
const RwRGBAReal *color = &pLight->color;
   
	// write colour 
   	*(_rwSkyLightFillPos+0)				= color->red;
	*(_rwSkyLightFillPos+1) 			= color->green;
	*(_rwSkyLightFillPos+2) 			= color->blue;
	// write type
	*((RwUInt32*)(_rwSkyLightFillPos+3))= rpLIGHTDIRECTIONAL;
	_rwSkyLightFillPos += 4;

	*(_rwSkyLightFillPos++)				= pAt->x;
	*(_rwSkyLightFillPos++) 			= pAt->y;
	*(_rwSkyLightFillPos++) 			= pAt->z;
	*(_rwSkyLightFillPos++) 			= (RwReal)(0.0f);
	
	_rwSkyLightQWordsWritten += 2; 
}

//
// name:		WriteLightingMatrix
// description:	Send inverse matrix information
static
void WriteLightingMatrix(RwMatrix* pMatrix)
{
	*(_rwSkyLightFillPos++) = pMatrix->right.x;
	*(_rwSkyLightFillPos++) = pMatrix->right.y;
	*(_rwSkyLightFillPos++) = pMatrix->right.z;
	*(_rwSkyLightFillPos++) = 1.0f;

	*(_rwSkyLightFillPos++) = pMatrix->up.x;
	*(_rwSkyLightFillPos++) = pMatrix->up.y;
	*(_rwSkyLightFillPos++) = pMatrix->up.z;
	*(_rwSkyLightFillPos++) = 1.0f;

	*(_rwSkyLightFillPos++) = pMatrix->at.x;
	*(_rwSkyLightFillPos++) = pMatrix->at.y;
	*(_rwSkyLightFillPos++) = pMatrix->at.z;
	*(_rwSkyLightFillPos++) = 1.0f;

	*(_rwSkyLightFillPos++) = pMatrix->pos.x;
	*(_rwSkyLightFillPos++) = pMatrix->pos.y;
	*(_rwSkyLightFillPos++) = pMatrix->pos.z;
	*(_rwSkyLightFillPos++) = 1.0f;

    _rwSkyLightQWordsWritten += 4;
}

//
// name:		PS2AllApplyDirectionalLighting
// description:	Callback that applies lighting to an atomic
//
static
void PS2AllApplyDirectionalLighting(RpAtomic *pAtomic, RwSurfaceProperties *surface)
{

	if(RpGeometryGetFlags(pAtomic->geometry) & rpGEOMETRYLIGHT)
	{
		// Assume if pDirect isn't lighting the object then it isn't lit by any directional lighting
		if(pDirect->object.object.flags & rpLIGHTLIGHTATOMICS)
		{
			RwMatrix invMat;
			
			// assume ltm has been updated already
			RwMatrix *pFrameMat = &(((RwFrame*)rwObjectGetParent(pAtomic))->ltm);
			
			Invert(*((CMatrix*)pFrameMat), *((CMatrix*)&invMat));
			
			// This doesn't appear to do anything
			#if 0
				((RwReal*)&surfLightCoeffs)[0] = surface->ambient	* (RwReal)(255.0f);
				((RwReal*)&surfLightCoeffs)[1] = surface->specular  * (RwReal)(255.0f);
				((RwReal*)&surfLightCoeffs)[2] = surface->diffuse	* (RwReal)(255.0f);
			#endif

			WriteLightingMatrix(&invMat);
			
			// Add in ambient light
			ApplyAmbientLight(pAmbient);
		
			// Add in directional light
			ApplyDirectionalLight(pDirect);
			
			// Add in any extra directional lights
			for(int32 i=0; i<NumExtraDirLightsInWorld; i++)
			{
				ApplyDirectionalLight(pExtraDirectionals[i]);
			}
		}
		else
		{
			// skip matrix
			_rwSkyLightFillPos += 16;
			_rwSkyLightQWordsWritten += 4; 

			// Add in ambient light
			ApplyAmbientLight(pAmbient);
		}
	}
}



#pragma mark -

//
//
//
//
RwBool CustomPipelineObjSetupCB_RpAtomicPS2AllLightingSetup(RxPS2AllPipeData *ps2AllPipeData)
{
	RpAtomic *pAtomic = (RpAtomic*)(ps2AllPipeData->sourceObject);
	ASSERT(pAtomic);
	
	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);



	const bool8 geomLit = ((RpGeometryGetFlags(pGeometry)&rpGEOMETRYLIGHT) == rpGEOMETRYLIGHT);
		
	if(bPS2AllSetupLighting == FALSE && bLastGeomLit == geomLit && FALSE)
	{
		if(geomLit)
		{
			// if directional light is lighting objects
			if(pDirect->object.object.flags & rpLIGHTLIGHTATOMICS)
			{
				// assume ltm has been updated already
				RwMatrix invMat;
				Invert(*((CMatrix*)&(((RwFrame *)rwObjectGetParent(pAtomic))->ltm)), *((CMatrix*)&invMat));
		    	RpAtomicPS2AllLightingPersist(&invMat, 1.0f, 1.0f);
		    }
		    else
		    {
		  	    RpAtomicPS2AllLightingPersist(NULL, 1.0f, 1.0f);
			}
		}

        return(TRUE);
	}


	bPS2AllSetupLighting	= FALSE;
	bLastGeomLit			= geomLit;
	
    // Do da lightin', boss 
    PS2AllApplyDirectionalLighting(pAtomic, NULL);

	return(TRUE);
}



//
//
//
//
RwBool CustomPipelineObjSetupCB_RpSkinAtomicPS2AllLightingSetup(RxPS2AllPipeData *ps2AllPipeData)
{

	RpAtomic *pAtomic = (RpAtomic*)(ps2AllPipeData->sourceObject);
	ASSERT(pAtomic);	


	if(bPS2AllSetupLighting == FALSE && bLastGeomLit == TRUE)
	{
		// assume ltm has been updated already
		RwMatrix invMat;
		Invert(*((CMatrix*)&(((RwFrame *)rwObjectGetParent(pAtomic))->ltm)), *((CMatrix*)&invMat));
    	RpAtomicPS2AllLightingPersist(&invMat, 1.0f, 1.0f);
	}
	else
	{
		bPS2AllSetupLighting	= FALSE;
		bLastGeomLit			= TRUE;		// skinned atomics always require lighting

	    // Do da lightin', boss 
	    PS2AllApplyDirectionalLighting(pAtomic, NULL);
	}


	return(TRUE);
}




/* 
 this is the default PS2All ObjectSetup 
RwBool RpAtomicPS2AllObjectSetupCallBack(RxPS2AllPipeData *ps2AllPipeData, RwMatrix **transform)
{
    RpAtomic           *atomic;
    RwFrustumTestResult inFrustum;

    atomic = (RpAtomic *)(ps2AllPipeData->sourceObject);

    RpAtomicPS2AllGetMeshHeaderMeshCache(atomic, ps2AllPipeData);

    RpAtomicPS2AllGatherObjMetrics(atomic);

    RpAtomicPS2AllMorphSetup(atomic, ps2AllPipeData);

    RpAtomicPS2AllObjInstanceTest(atomic, ps2AllPipeData);

    RpAtomicPS2AllClear(atomic);

    RpAtomicPS2AllTransformSetup(atomic, transform);

    RpAtomicPS2AllFrustumTest(atomic, &inFrustum);

    RpAtomicPS2AllPrimTypeTransTypeSetup(ps2AllPipeData, inFrustum);

    RpAtomicPS2AllMatModulateSetup(atomic, ps2AllPipeData);

    RpAtomicPS2AllLightingSetup(ps2AllPipeData);

    RWRETURN(TRUE);
}*/




#pragma mark -


//
// name:		PS2AllObjectSetupCB
// description:	Optimised ps2all object setup callback
RwBool PS2AllObjectSetupCB(RxPS2AllPipeData * ps2AllPipeData, RwMatrix ** transform)
{
    RpAtomic* pAtomic = (RpAtomic *)(ps2AllPipeData->sourceObject);
    RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
    RwFrustumTestResult inFrustum;


#ifndef FINALBUILD
	CustomPipelineObjSetupCB_UpdateNumAtomicsRendered();
#endif

	//
    // We know that our RpAtomics have only one morphTarget, so
    // we can simplify RpAtomicPS2AllGetMeshHeaderMeshCache slightly
    // by unconditionally retrieving the RwMeshCache from the RpGeometry.
    // 
    //RpAtomicPS2AllGetMeshHeaderMeshCache(pAtomic, ps2AllPipeData);
    ps2AllPipeData->meshHeader = pGeom->mesh;
    ps2AllPipeData->meshCache = rpGeometryGetMeshCache(pGeom, pGeom->mesh->numMeshes);
    
    // Gather metrics:
    RpAtomicPS2AllGatherObjMetrics(pAtomic);

    // Set up numMorphTargets and spExtra
    //RpAtomicPS2AllMorphSetup(pAtomic, ps2AllPipeData);

    // Decide whether this geometry needs instancing or not 
    RpAtomicPS2AllObjInstanceTest(pAtomic, ps2AllPipeData);

    // Clear geometry and interpolator instance flags 
    RpAtomicPS2AllClear(pAtomic);

    // We need to cache the transform
    RpAtomicPS2AllTransformSetup(pAtomic, transform);

    // We need to to a frustum test here
    RpAtomicPS2AllFrustumTest(pAtomic, &inFrustum);

    // Set up primType and transtype
    RpAtomicPS2AllPrimTypeTransTypeSetup(ps2AllPipeData, inFrustum);
	
    // Do we modulate with material colours for this geometry?
    RpAtomicPS2AllMatModulateSetup(pAtomic, ps2AllPipeData);


#ifndef FINALBUILD
	CustomPipelineObjSetupCB_Display32bitTextures(pAtomic);
#endif

	// custom lighting setup
	CustomPipelineObjSetupCB_RpAtomicPS2AllLightingSetup(ps2AllPipeData);

    return (TRUE);
}





//
//
// Macros required to get skinning object setup function working 
//
#define _rpSkinVIFTransferMatricesSize(_numMatrices)                        \
    (1 + _numMatrices * 4)

#define _rpSkinDMAcntTransVIFTransferMatrices( _numMatrices, _matrices,     \
                                               _memory )                    \
    _rpSkinDMATransferMatrices( _numMatrices, _matrices, _memory,           \
                                ((1L << 28) | (/**/ _numMatrices * 4 /**/)) )

/*------------- (FAKE) MATFX ALLOCATIONS ------------------------------------- */
#define pipeASymbMatFXBaseAddress  (vuSDLightOffset-20)                          /* */
#define pipeASymbMatrixArraySize   (64)                                      /* */
#define pipeASymbSkinBaseAddress   (pipeASymbMatFXBaseAddress-pipeASymbMatrixArraySize*4)

/*****************************************************************************
 _rpSkinDMATransferMatrices

 Macro to upload the skinning matrices.
 */
#define _rpSkinDMATransferMatrices( _numMatrices, _matrices,                \
                                    _memory, _dmaTag )                      \
MACRO_START                                                                 \
{                                                                           \
    RwUInt128 ltmp = 0;                                                     \
    RwUInt64 tmp;                                                           \
    RwUInt64 tmp1;                                                          \
    RwUInt32 i;                                                             \
                                                                            \
    RWASSERT(64 >= _numMatrices);                                           \
    RWASSERT(NULL != _matrices);                                            \
                                                                            \
    /*------- VIF Transfer Information ------------------*/                 \
    tmp = _dmaTag;                                                          \
    tmp1 = ( ( (VIFCMD_UNPACK                                               \
               | (((/**/ _numMatrices * 4 /**/) & 0xFF ) << 16)             \
               | ((RwUInt64)(_memory))) << 32)                              \
             | (VIFCMD_CYCLE | (4 << 8) | (4)) );                           \
    MAKE128(ltmp, tmp1, tmp);                                               \
    RWDMA_ADD_TO_PKT(ltmp);                                                 \
    /*---------------------------------------------------*/                 \
                                                                            \
    /*-------- Upload Skin matrices ---------------------*/                 \
    for(i = 0; i < _numMatrices; i++, _matrices++)                          \
    {                                                                       \
        RwUInt128 ltmp1;                                                    \
        RwUInt128 ltmp2;                                                    \
        RwUInt128 ltmp3;                                                    \
        RwUInt128 ltmp4;                                                    \
                                                                            \
        ltmp1 = *((RwUInt128 *)&_matrices->right.x);                        \
        ltmp2 = *((RwUInt128 *)&_matrices->up.x);                           \
        ltmp3 = *((RwUInt128 *)&_matrices->at.x);                           \
        ltmp4 = *((RwUInt128 *)&_matrices->pos.x);                          \
                                                                            \
        RWDMA_ADD_TO_PKT(ltmp1);                                            \
        RWDMA_ADD_TO_PKT(ltmp2);                                            \
        RWDMA_ADD_TO_PKT(ltmp3);                                            \
        RWDMA_ADD_TO_PKT(ltmp4);                                            \
    }                                                                       \
    /*---------------------------------------------------*/                 \
}                                                                           \
MACRO_STOP




//
//
//
//
extern "C"
{
	RwMatrix *_rpSkinMatrixUpdating(RpAtomic *atomic, RpSkin *skin);
}



//
// name:		PS2AllSkinnnedShadowObjectSetupCB
// description:	Object setup for skinned object pipeline
//
RwBool PS2AllSkinnedObjectSetupCB(RxPS2AllPipeData *ps2AllPipeData, RwMatrix **transform)
{

    RpAtomic *pAtomic = (RpAtomic*)(ps2AllPipeData->sourceObject);
    RWASSERT(pAtomic);

    RpGeometry *pGeom = pAtomic->geometry;
    RWASSERT(pGeom);

    RpSkin *pSkin = RpSkinGeometryGetSkin(pGeom);
    RWASSERT(pSkin);
    
    const uint32 numMatrices = RpSkinGetNumBones(pSkin);
    RWASSERT(numMatrices > 0);
    RWASSERT(numMatrices < 64);

#ifndef FINALBUILD
	CustomPipelineObjSetupCB_UpdateNumAtomicsRendered();
#endif

    // Get the RwMeshCache from the atomic/geometry;
    // If the geometry has more than one morph target the resEntry
    // in the atomic is used else the resEntry in the geometry 
    //RpAtomicPS2AllGetMeshHeaderMeshCache(atomic, ps2AllPipeData);
    ps2AllPipeData->meshHeader = pGeom->mesh;                                                                   \
	ps2AllPipeData->meshCache = rpGeometryGetMeshCache(pGeom, pGeom->mesh->numMeshes);

    // Set up numMorphTargets and spExtra:
    //RpAtomicPS2AllMorphSetup(atomic, ps2AllPipeData);
	 
    // Decide whether this geometry needs instancing or not:
    RpAtomicPS2AllObjInstanceTest(pAtomic, ps2AllPipeData);

    // Clear geometry and interpolator instance flags:
    RpAtomicPS2AllClear(pAtomic);
	
    // We need to cache the transform:
    RpAtomicPS2AllTransformSetup(pAtomic, transform);


RwFrustumTestResult inFrustum;
    // We need to to a frustum test here:
    //RpAtomicPS2AllFrustumTest(pAtomic, &inFrustum);
	CustomPipelineObjSetupCB_PedFrustumTest(pAtomic, &inFrustum);
	
    // Set up primType and transtype:
    RpAtomicPS2AllPrimTypeTransTypeSetup(ps2AllPipeData, inFrustum);

    // Do we modulate with material colours for this geometry?
    RpAtomicPS2AllMatModulateSetup(pAtomic, ps2AllPipeData);


	// custom lighting setup
	CustomPipelineObjSetupCB_RpSkinAtomicPS2AllLightingSetup(ps2AllPipeData);

	
	
    // Grab the bones matrices (potentially updating them):
    RwMatrix *mba = _rpSkinMatrixUpdating(pAtomic, pSkin);
    RWASSERT(mba);

    // Open a DMA packet to upload the matrices:
    _rwDMAOpenVIFPkt(0, (1 + _rpSkinVIFTransferMatricesSize(numMatrices)));

    // Add the matrices to the packet (NOTE: as noted in the comment above and in
    // _rpSkinSkyMatricesAddressVerify below, we assume that pipeASymbSkinBaseAddress
    // is the same as pipeASymbSkinWithMatFXBaseAddress):
    RWDMA_LOCAL_BLOCK_BEGIN();
    {
	    _rpSkinDMAcntTransVIFTransferMatrices(numMatrices, mba, pipeASymbSkinBaseAddress);
    }
    RWDMA_LOCAL_BLOCK_END();

    return(TRUE);
}







//
// name:		Bridge callback for vehicle meshes with changing vehicle colours
// description:	Standard bridge callback with slight change when uploading vehicle colours
//
RwBool RpMeshPS2AllVehicleBridgeCallBack(RxPS2AllPipeData *ps2AllPipeData)
{
    // If you exit here, the mesh won't be rendered:

    
    // Setup colour and texture for car colours
	CVehicleModelInfo::UpdateVehicleColour(ps2AllPipeData);
	
    // Asynchronously upload the texture if nec/poss:
    RpMeshPS2AllAsyncTextureUpload(ps2AllPipeData);
    // If changed, does skyTexCacheAccessRaster and updates global renderstate:
    RpMeshPS2AllSyncTextureUpload(ps2AllPipeData);

    // Open a VIF transfer packet, with a header DMA tag:
    const uint32 numInitialQW =	rpMESHPS2ALLGIFTAGNUMINITIALQW +
                   				rpMESHPS2ALLMATCOLNUMINITIALQW +
                   				rpMESHPS2ALLSURFPROPSNUMINITIALQW +
                   				rpMESHPS2ALLCLIPINFONUMINITIALQW +
                   				rpMESHPS2ALLTEXTURESTATENUMINITIALQW;
    const uint32 numExtraQW	=	rpMESHPS2ALLVU1CODEUPLOADNUMEXTRAQW;
    RpMeshPS2AllStartVIFUploads(numInitialQW, numExtraQW);


    // Extra user VIF uploads would go here... VIF tag(s) necessary.
    // Would have called _rxPS2AllStartVIFUploads with FALSE if needed
    // DMA tags in the user uploads (would have had to fix up with a
    // terminal DMA tag so the following standard transfers work). 

    // Here follow the standard VIF uploads 

    // Upload a GIF tag for the code to submit geom to the GS under
    // (also does some renderstate setup based on alpha - that's why
    // this needs to be before the texture state setup func)
    RpMeshPS2AllGIFTagUpload(ps2AllPipeData);

    // Upload material colour, dependent on whether there's a texture 
	// TODO[5]: CHECK HELPER MACROS - IF CAN PUT IN PREDICATION PARAMETERS TO HELP
	//   CULL WORK, THEN DO SO - EG FOR RpMeshPS2AllMatColUpload, CAN SAY "YES/NO"
	//   IN THE PARAMETERS, FOR WHETHER THERE'S A TEXTURE (DEPENDING ON WHETHER
	//   YOU KNOW) AND THAT'LL BOIL AWAY IN RELEASE COS IT'S A MACRO! [SHOULD BE
	//   ABLE TO ADD SUCH PARAMETERS LATER WITH LEGACY-SUPPORT MACROS THAT CONTAIN
	//   THE BOILED-AWAY TESTS]
    RpMeshPS2AllMatColUpload(ps2AllPipeData);

    // Upload surface properties, including the 'extra' float in W
    RpMeshPS2AllSurfPropsUpload(ps2AllPipeData);

    // Sets up clipping info and J-C's switch QWs (NOTE: uses "transType&7")
    RpMeshPS2AllClipInfoUpload(ps2AllPipeData);

    // Upload texture renderstate to the GS (this sends stuff thru VIF
    // only, ergo can be combined with other VIF stuff! :) )
    RpMeshPS2AllTextureStateUpload(ps2AllPipeData);

    // Set up vu1CodeIndex 
    RpMeshPS2AllVU1CodeIndexSetup(ps2AllPipeData);
    // Upload the VU1 code (if it changed) last, since it uses a DMA tag (ref transfer of the code)
    RpMeshPS2AllVU1CodeUpload(ps2AllPipeData);

    // Kicks off geometry transfer, sets up refCnt/clrCnt
    RpMeshPS2AllEndVIFUploads(ps2AllPipeData);

    return (TRUE);
}





//
// name:		PS2AllRecalculateLighting
// description:	Set variable so that next atomic doesnt use the lighting from the previous atomic
//
void PS2AllRecalculateLighting()
{
	bPS2AllSetupLighting = TRUE;
}




//
// name:		PS2AllSetAtomicPipe
// description:	Set the pipeline for the atomic
void PS2AllSetAtomicPipe(RpAtomic* pAtomic)
{
	// if everything is OK, then CVB pipeline should not be in here already 
	// (check CAtomicModelInfo::SetAtomic() & CDamageAtomicModelInfo::SetDamagedAtomic());
	ASSERT(CVuCustomBuildingRenderer::IsCVBPipelineAttached(pAtomic)==FALSE);
	
	// if everything is OK, then CVSP pupeline should not in here alradt
	// check CClumpModelInfo::SetClump())
	//ASSERT(CVuCustomSkinPedRenderer::IsCVSPPipelineAttached(pAtomic)==FALSE);
	
    RpSkin* pSkin = RpSkinAtomicGetSkin(pAtomic);
    if(!pSkin)
    {
    	RpAtomicSetPipeline(pAtomic, pOptimisedLightPipe);
	}
}




//
// name:		PS2AllSetAtomicPipe
// description:	Set the pipeline for the atomic
//
void PS2AllSetVehicleColourMaterialPipe(RpMaterial* pMaterial)
{
	// don't change CVB pipeline (it may be already attached):
	if(CVuCustomBuildingRenderer::IsCVBPipelineAttached(pMaterial))
	{
		// do nothing. CVB setup is necessary for atomics only.
	}
	//else if(CVuCustomSkinPedRenderer::IsCVSPPipelineAttached(pMaterial))
	//{
	//	// do nothing
	//}
	else
	{
	   	RpMaterialSetPipeline(pMaterial, pVehicleColourMaterialPipe);
	}
}
