
/**********************************************************************
 *
 * File :     handy.cpp
 *
 * Abstract : 
 *
 *	
 *	2002/03/08	-	Andrzej: RpClumpGetBoundingSphere() added;
 *	2002/06/06	-	Andrzej: FindIntersectionLineSphere() added;
 *	2004/02/18	-	Andrzej: RpClump/AtomicConvertGeometryToTL/TS() added;
 *	2004/08/27	-	Andrzej: RpGeometryReplaceOldMaterialWithNewMaterial() added;
 *
 *
 *
 **********************************************************************
 *
 *
 **********************************************************************/

#include "handy.h"
#include "rtcharse.h"
#include "debug.h"
#include "timecycle.h"
#include "2deffect.h"

//!PC required extra headers to compile...
#if defined(GTA_PC) && !defined(OSW)
	#include "windows.h"
	#include "win.h"
	#include <d3d9.h>
#endif

#ifdef GTA_XBOX
	#include "xboxglobals.h"
#endif

#include "rpskin.h"


#ifdef GTA_TOON
	#include "rptoon.h"
	#include "txdstore.h"
#endif//GTA_TOON

#include "KeyGen.h"


// The global pointer to the global font.
static RtCharset			*gpTheFont;

///////////////////////////////////////////////////////////////////////////
// NAME       : CreateDebugFont()
// PURPOSE    : This function does exactly what it says on the tin. Has to
//				be called before stuff is printed to screen.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CreateDebugFont()
{
#ifndef FINALBUILD
	static RwRGBA       fore = { 0xff, 0xff, 0x80, 0xff };
	static RwRGBA       back = { 0, 0, 0, 0 };
	
	RtCharsetOpen();
	gpTheFont = RtCharsetCreate(&fore, &back);

	ASSERT(gpTheFont);
#endif	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : DestroyDebugFont()
// PURPOSE    : This function does exactly what it says on the tin. 
///////////////////////////////////////////////////////////////////////////

void DestroyDebugFont()
{
#ifndef FINALBUILD
	RtCharsetDestroy(gpTheFont);
	RtCharsetClose();
#endif	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : ObrsPrintfString()
// PURPOSE    : Prints stuff. Better name needed.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void ObrsPrintfString(const char *pString, short X, short Y)
{
#ifndef FINALBUILD
	RtCharsetPrintBuffered(gpTheFont, pString, X * 8, (Y) * 14, TRUE);
#endif
}

void FlushObrsPrintfs()
{
#ifndef FINALBUILD
	RtCharsetBufferFlush();
#endif
}

///////////////////////////////////////////////////////////////////////////
// NAME       : DefinedState()
// PURPOSE    : Sets the rendering pipeline to a defined state (handy for debugging).
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void DefinedState(void)
{
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSWRAP);
    RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);		// This mode currently results in the models being rendered translucently. This should be
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);	// fixed in the power pipe version of RW. We'll leave it for now.
    RwRenderStateSet(rwRENDERSTATEBORDERCOLOR, (void *)RWRGBALONG( 0, 0, 0, 255));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)RWRGBALONG( CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));
	RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPELINEAR);
#if (defined GTA_PC || defined GTA_XBOX)
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);

    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONGREATER);
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void *)0x00000002);
#endif	
//	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);	This one seems very slow.
}

void DefinedState2d(void)
{
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSWRAP);
    RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);		// This mode currently results in the models being rendered translucently. This should be
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);	// fixed in the power pipe version of RW. We'll leave it for now.
    RwRenderStateSet(rwRENDERSTATEBORDERCOLOR, (void *)RWRGBALONG( 0, 0, 0, 255));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);

	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
#if (defined GTA_PC || defined GTA_XBOX)
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *)rwALPHATESTFUNCTIONGREATER);
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void *)0x00000002);
#endif	
//	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);	This one seems very slow.
}


//
// GetFirstAtomicCallback: callback to return the first atomic found in a
// clump
//
RpAtomic *GetFirstAtomicCallback(RpAtomic* pAtomic, void *pData)
{
    *((RpAtomic **) pData) = pAtomic;
    return NULL;
}

RpAtomic* GetFirstAtomic(RpClump* pClump)
{
	RpAtomic* pAtomic=NULL;
	
	RpClumpForAllAtomics(pClump, &GetFirstAtomicCallback, &pAtomic);
	return pAtomic;	
}


//
// GetFirstAtomicCallback: callback to return the first atomic found in a
// clump
//
RpAtomic *Get2DEffectAtomicCallback(RpAtomic* pAtomic, void *pData)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeom);	
	if (RpGeometryGetNum2dEffects(pGeom) > 0)
	{
   		*((RpAtomic **) pData) = pAtomic;
   		return NULL;
	}
    return pAtomic;
}

RpAtomic* Get2DEffectAtomic(RpClump* pClump)
{
	RpAtomic* pAtomic=NULL;
	
	RpClumpForAllAtomics(pClump, &Get2DEffectAtomicCallback, &pAtomic);
	return pAtomic;	
}



//
// GetFirstAtomicCallback: callback to return the first atomic found in a
// clump
//
RwObject *GetFirstObjectCallback(RwObject* pObject, void *pData)
{
    *((RwObject **) pData) = pObject;
    return NULL;
}

RwObject* GetFirstObject(RwFrame* pFrame)
{
	RwObject* pObject=NULL;
	
	RwFrameForAllObjects(pFrame, &GetFirstObjectCallback, &pObject);
	return pObject;	
}

//
// GetFirstFrameCallback: callback to return the first frame in a children list
//
RwFrame *GetFirstFrameCallback(RwFrame* pFrame, void *pData)
{
    *((RwFrame **) pData) = pFrame;
    return NULL;
}

RwFrame* GetFirstChild(RwFrame* pFrame)
{
	RwFrame* pChild=NULL;
	
	RwFrameForAllChildren(pFrame, &GetFirstFrameCallback, &pChild);
	return pChild;
}

//
// GetFirstTextureCallback: callback to return the first texture found
//
RwTexture *GetFirstTextureCallback(RwTexture* pTexture, void *pData)
{
    *((RwTexture **) pData) = pTexture;
    return NULL;
}

RwTexture* GetFirstTexture(RwTexDictionary* pTxd)
{
	RwTexture* pTexture=NULL;
	
	RwTexDictionaryForAllTextures(pTxd, &GetFirstTextureCallback, &pTexture);
	return pTexture;	
}





//
// flag to decide which matrix to use: LTM (TRUE) or Modeling (FALSE):
//
static
bool b_cbsUseLTM=TRUE;


//
//
//
//
static
RpAtomic* cbsCalcMeanBSphereCenterCB(RpAtomic *pAtomic, void *data)
{

	CVector *pCenter = (CVector*)data;
	RpClump *pClump = RpAtomicGetClump(pAtomic);
	
	CVector AtomicCenter;
	if(b_cbsUseLTM)
	{
		RwV3dTransformPoints((RwV3d*)&AtomicCenter, 
    	                     &RpAtomicGetBoundingSphere(pAtomic)->center, 1,
    	                     RwFrameGetLTM(RpClumpGetFrame(pClump)));
	}
	else
	{
    	RwV3dTransformPoints((RwV3d*)&AtomicCenter,
        	                 &RpAtomicGetBoundingSphere(pAtomic)->center, 1,
        	                 RwFrameGetMatrix(RpClumpGetFrame(pClump)));
	}
	
	// cumulative center in world coordinates:
	(*pCenter) += AtomicCenter;
	return(pAtomic);
}


//
//
//
//
static
RpAtomic* cbsCalcMeanBSphereRadiusCB(RpAtomic *pAtomic, void *data)
{
	
	RwSphere	*TestSphere		= (RwSphere*)data;
	RpClump		*pClump			= RpAtomicGetClump(pAtomic);
	RwSphere	*AtomicSphere	= RpAtomicGetBoundingSphere(pAtomic);
	
	
	CVector AtomicCenter;
    if(b_cbsUseLTM)
    {
	    RwV3dTransformPoints((RwV3d*)&AtomicCenter,
	                         &RpAtomicGetBoundingSphere(pAtomic)->center, 1,
	                         RwFrameGetLTM(RpClumpGetFrame(pClump)));
    }
    else
    {
	    RwV3dTransformPoints((RwV3d*)&AtomicCenter,
	                         &RpAtomicGetBoundingSphere(pAtomic)->center, 1,
	                         RwFrameGetMatrix(RpClumpGetFrame(pClump)));
	}
	
	CVector PQ(AtomicCenter - TestSphere->center);
	const float PQlen = PQ.Magnitude();
	
	if(TestSphere->radius < (PQlen+AtomicSphere->radius))
	{
		TestSphere->radius = PQlen+AtomicSphere->radius;
	}
	
	return(pAtomic);
}





//
// Calculates bounding sphere for given clump;
//
// Note!
// 1. Returned pSphere is in clump's LOCAL coordinate system
// (if flag bUseLTM==TRUE, then sphere properties are calculated
// using inverted clump's LTM matrix instead of inverted standard
// Modeling Matrix);
//
// 2. All calculations are based on internal atomics' bounding spheres
// and their positions/sizes in WORLD coordinate system;
//
//
RpClump* RpClumpGetBoundingSphere(RpClump *pClump, RwSphere *pSphere, bool bUseLTM)
{
	b_cbsUseLTM = bUseLTM;

	if((!pClump) || (!pSphere))
		return(NULL);

	pSphere->radius = 0;
	pSphere->center = CVector(0,0,0);


	float NumAtomics = RpClumpGetNumAtomics(pClump);
	if(NumAtomics < 1.0f)
		return(NULL);

	CVector CenterOfSphere(0,0,0);
	RpClumpForAllAtomics(pClump, &cbsCalcMeanBSphereCenterCB, &CenterOfSphere);
	CenterOfSphere /= NumAtomics;

	RwSphere TestSphere;
	TestSphere.center = CenterOfSphere;
	TestSphere.radius = 0;
	RpClumpForAllAtomics(pClump, &cbsCalcMeanBSphereRadiusCB, &TestSphere);

	// world->local matrix:
	RwMatrix matW2L;
	// for any clump LTM and Modeling Matrix should be equal, but anyway:
	if(b_cbsUseLTM)
		RwMatrixInvert(&matW2L, RwFrameGetLTM(RpClumpGetFrame(pClump)));
	else	
		RwMatrixInvert(&matW2L, RwFrameGetMatrix(RpClumpGetFrame(pClump)));
	
	// now bounding sphere is in clump's local coordinate system: 
   	RwV3dTransformPoints(&TestSphere.center,
   						&TestSphere.center, 1,
   						&matW2L);
	
	pSphere->radius = TestSphere.radius;
	pSphere->center = TestSphere.center;
	return(pClump);
	
}// end of RpClumpGetBoundingSphere()...




#pragma mark -

//
//
//
//
static
RpAtomic* GetSkinHierarchy(RpAtomic *pAtomic, void *data)
{
    *(void**)data = (void*)RpSkinAtomicGetHAnimHierarchy(pAtomic);
    return(NULL);
}

//
// FASTER version of extracting RpHAnimHierarchy
// (note: skin atomic must be initialized first with proper data):
//
RpHAnimHierarchy* GetAnimHierarchyFromSkinClump(RpClump *pClump)
{
    RpHAnimHierarchy *pHierarchy = NULL;
    RpClumpForAllAtomics(pClump, &GetSkinHierarchy, (void*)&pHierarchy);
	return(pHierarchy);
}





///////////////////////////////////////////////////////////////////


//
//
//
//
static
RwFrame* GetChildFrameHierarchy(RwFrame *pFrame, void *data)
{    
    RpHAnimHierarchy *hierarchy = NULL;//*(RpHAnimHierarchy**)data;
    /*
     * Return the first hierarchy found that is attached to one of the atomic
     * frames...
     */
    hierarchy = RpHAnimFrameGetHierarchy(pFrame);
    if(!hierarchy)
    {
        RwFrameForAllChildren(pFrame, &GetChildFrameHierarchy, data);
        return(pFrame);
    }
    *(void**)data = (void*)hierarchy;

    return(NULL);
}



//
// SLOWER version of extacting RpHAnimHierarchy;
// (it should be used first time to get initialization data
// for atomic)
//
RpHAnimHierarchy* GetAnimHierarchyFromFrame(RwFrame* pFrame)
{
    RpHAnimHierarchy *pHierarchy = NULL;
    
    GetChildFrameHierarchy(pFrame, (void*)&pHierarchy);
    if(pHierarchy)
    	return pHierarchy;
    /*
     * Return the hierarchy for this model...
     */
    RwFrameForAllChildren(pFrame, &GetChildFrameHierarchy, (void*)&pHierarchy);

    return(pHierarchy);
}

RpHAnimHierarchy* GetAnimHierarchyFromClump(RpClump *pClump)
{
	return GetAnimHierarchyFromFrame(RpClumpGetFrame(pClump));
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////


//
//
//
//
static
void SkinGetSkinToBoneMatrix(RwMatrix *target, RpSkin *pSkin, int32 index)
{                                                       
    const RwMatrix* const SkinToBoneMatrices = RpSkinGetSkinToBoneMatrices(pSkin);

    *target = SkinToBoneMatrices[index];
}


//
//
//
//
static
void SkinGetBoneToSkinMatrix(RwMatrix *target, RpSkin *pSkin, int32 index)
{                                                       
    RwMatrix SkinToBone;
    SkinGetSkinToBoneMatrix(&SkinToBone, pSkin, index);
    RwMatrixInvert(target, &SkinToBone);
}




#define MAXNUMBONES         	(64)


//
//
//
//
struct _boneTranslation
{
    RwUInt32            parent;
    RwV3d               offset;
};


static
_boneTranslation BoneTranslations[MAXNUMBONES];



//
//
//
//
void SkinGetBonePositions(RpClump *pClump)
{

static
bool bBoneTranslationsExtracted = FALSE;


	if(bBoneTranslationsExtracted)
		return;
	bBoneTranslationsExtracted = TRUE;



	RpAtomic		*pAtomic 	= GETATOMICFROMCLUMP(pClump);
	RpSkin			*pSkin		= GETSKINFROMATOMIC(pAtomic);
	RpHAnimHierarchy *pHierarchy= GETANIMHIERARCHYFROMSKINCLUMP(pClump);


    uint32	stack[MAXNUMBONES >> 1];
    uint32	nStack = 0;
    int32	parentIndex = 0;

	//
	// Assume root bone is at origin:
	//
	BoneTranslations[0].offset.x = 0.0f;
    BoneTranslations[0].offset.y = 0.0f;
    BoneTranslations[0].offset.z = 0.0f;
    BoneTranslations[0].parent = -1;

	//
	// Get offset for each bone from its parent:
	//
    const int32 numBones = RpSkinGetNumBones(pSkin);

	for(int32 currentIndex=1; currentIndex<numBones; currentIndex++)
    {
        RwMatrix	currentBoneToSkin;
        RwMatrix	parentSkinToBone;

        //
        // Invert bone-to-skin matrix to get currentBoneToSkin.pos,
        // which will be the position of this bone's origin in skin space.
        //
        SkinGetBoneToSkinMatrix(&currentBoneToSkin, pSkin, currentIndex);

        //
        // Transform to get the position of this bone in the local space
        // of it's parent bone, which we store for later use.
        //
        SkinGetSkinToBoneMatrix(&parentSkinToBone, pSkin, parentIndex);

        RwV3dTransformPoints(&BoneTranslations[currentIndex].offset,
                             &currentBoneToSkin.pos, 1, &parentSkinToBone);
        
        //
        // Store the index of the parent bone also:
        //
        BoneTranslations[currentIndex].parent = parentIndex;

        //
        // Handle hierarchy structure:
        //
        uint32 flags = pHierarchy->pNodeInfo[currentIndex].flags;

        if (flags & rpHANIMPUSHPARENTMATRIX)
        {
            stack[++nStack] = parentIndex;
        }

        if (flags & rpHANIMPOPPARENTMATRIX)
        {
            parentIndex = stack[nStack--];
        }
        else
        {
            parentIndex = currentIndex;
        }
        
    }//for(int32 currentIndex=1; currentIndex<numBones; currentIndex++)...


}//end of SkinGetBonePositions()...



//
//
//
//
void SkinSetBonePositions(RpClump *pClump)
{

		RpAtomic		 *pAtomic	= GETATOMICFROMCLUMP(pClump);
		RpSkin			 *pSkin		= GETSKINFROMATOMIC(pAtomic);
		RpHAnimHierarchy *pHierarchy= GETANIMHIERARCHYFROMSKINCLUMP(pClump);


        //
        // Now add translations to bones by transforming with the
        // stored offset relative to each bone's parent.
        //
        RwMatrix* const matrices = RpHAnimHierarchyGetMatrixArray(pHierarchy);

		const int32 numBones = RpSkinGetNumBones(pSkin);

        for(int32 i=1; i<numBones; i++)
        {
        	//RwV3d outpos;	
            //RwV3dTransformPoints(&outpos,
            //                   &BoneTranslations[i].offset, 1,
            //                   &matrices[BoneTranslations[i].parent]);
            //outpos.x += matrices[i].pos.x;
            //outpos.y += matrices[i].pos.y;
            //outpos.z += matrices[i].pos.z;
            //matrices[i].pos = outpos;
            
            RwV3dTransformPoints(&matrices[i].pos,
                               &BoneTranslations[i].offset, 1,
                               &matrices[BoneTranslations[i].parent]);
        }


}//end of SkinSetBonePositions()...





//
//
//
//
void SkinGetBonePositionsToTable(RpClump *pClump, RwV3d *TransTable)
{

	if(!TransTable)
		return;



	RpAtomic		*pAtomic 	= GETATOMICFROMCLUMP(pClump);
	RpSkin			*pSkin		= GETSKINFROMATOMIC(pAtomic);
	RpHAnimHierarchy *pHierarchy= GETANIMHIERARCHYFROMSKINCLUMP(pClump);


    uint32	stack[MAXNUMBONES >> 1];
    uint32	nStack = 0;
    int32	parentIndex = 0;

	//
	// Assume root bone is at origin:
	//
	TransTable[0].x = 0.0f;
    TransTable[0].y = 0.0f;
    TransTable[0].z = 0.0f;

	//
	// Get offset for each bone from its parent:
	//
    int32 numBones = RpSkinGetNumBones(pSkin);

	for(int32 currentIndex=1; currentIndex<numBones; currentIndex++)
    {
        RwMatrix	currentBoneToSkin;
        RwMatrix	parentSkinToBone;

        //
        // Invert bone-to-skin matrix to get currentBoneToSkin.pos,
        // which will be the position of this bone's origin in skin space.
        //
        SkinGetBoneToSkinMatrix(&currentBoneToSkin, pSkin, currentIndex);

        //
        // Transform to get the position of this bone in the local space
        // of it's parent bone, which we store for later use.
        //
        SkinGetSkinToBoneMatrix(&parentSkinToBone, pSkin, parentIndex);

        RwV3dTransformPoints(&TransTable[currentIndex],
                             &currentBoneToSkin.pos, 1, &parentSkinToBone);
        

        //
        // Handle hierarchy structure:
        //
        uint32 flags = pHierarchy->pNodeInfo[currentIndex].flags;

        if (flags & rpHANIMPUSHPARENTMATRIX)
        {
            stack[++nStack] = parentIndex;
        }

        if (flags & rpHANIMPOPPARENTMATRIX)
        {
            parentIndex = stack[nStack--];
        }
        else
        {
            parentIndex = currentIndex;
        }


    }//for(int32 currentIndex=1; currentIndex<numBones; currentIndex++)...


}//SkinGetBonePositionsToTable()...

//
//
//
//
RpAtomic* AtomicRemoveAnimFromSkinCB(RpAtomic *pAtomic, void *data)
{
	
	RpSkin *pSkin = RpSkinAtomicGetSkin(pAtomic);
	if(pSkin)
	{
        RpHAnimHierarchy *pHierarchy = GETANIMHIERARCHYFROMATOMIC(pAtomic);
        if(pHierarchy)
        {
        	if(pHierarchy->currentAnim->pCurrentAnim)
				RpHAnimAnimationDestroy(pHierarchy->currentAnim->pCurrentAnim);
			pHierarchy->currentAnim->pCurrentAnim = NULL;
        }
	}
	
	

	return(pAtomic);
}




#pragma mark -


//
//
//
// converts given atomic's geometry to TriList:
//
bool8 RpAtomicConvertGeometryToTL(RpAtomic *pAtomic)
{
	ASSERT(pAtomic);

	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeom);

	uint32 flags = RpGeometryGetFlags(pGeom);

	// don't touch already pre-instanced models (they're in platform specific format):
	if(flags & rpGEOMETRYNATIVE)
		return(FALSE);
	
	// detect tri-stripped geometry:
	if(flags & rpGEOMETRYTRISTRIP)
	{
		pGeom = RpGeometryLock(pGeom, rpGEOMETRYLOCKALL); 
		ASSERT(pGeom);

		flags &= (~rpGEOMETRYTRISTRIP);
		RpGeometrySetFlags(pGeom, flags);
		
		RpGeometryUnlock(pGeom);
		return(TRUE);
	}	

	return(FALSE);
}

//
//
//
// converts given atomic's geometry to TriStrip:
//
bool8 RpAtomicConvertGeometryToTS(RpAtomic *pAtomic)
{
	ASSERT(pAtomic);

	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeom);

	uint32 flags = RpGeometryGetFlags(pGeom);

	// don't touch already pre-instanced models (they're in platform specific format):
	if(flags & rpGEOMETRYNATIVE)
		return(FALSE);
	
	// detect tri-listed geometry:
	if(!(flags & rpGEOMETRYTRISTRIP))
	{
		pGeom = RpGeometryLock(pGeom, rpGEOMETRYLOCKALL); 
		ASSERT(pGeom);

		flags |= rpGEOMETRYTRISTRIP;
		RpGeometrySetFlags(pGeom, flags);
		
		RpGeometryUnlock(pGeom);
		return(TRUE);
	}	

	return(FALSE);
}


//
//
//
//
static
RpAtomic* atomicConvertToTLcb(RpAtomic *atomic, void *pData)
{
bool8 *pretval = (bool8*)(pData);

	if(!RpAtomicConvertGeometryToTL(atomic))
	{
		*pretval = FALSE;
	}

	return(atomic);
} 


//
//
//
//
bool8 RpClumpConvertGeometryToTL(RpClump *pClump)
{
	ASSERT(pClump);

bool8 retval = TRUE;
	
	RpClumpForAllAtomics(pClump,  atomicConvertToTLcb, &retval);
	return(retval);
}


//
//
//
//
static
RpAtomic* atomicConvertToTScb(RpAtomic *atomic, void *pData)
{
bool8 *pretval = (bool8*)(pData);

	if(!RpAtomicConvertGeometryToTS(atomic))
	{
		*pretval = FALSE;
	}

	return(atomic);
} 


//
//
//
//
bool8 RpClumpConvertGeometryToTS(RpClump *pClump)
{
	ASSERT(pClump);

bool8 retval = TRUE;
	
	RpClumpForAllAtomics(pClump,  atomicConvertToTScb, &retval);
	return(retval);
}


#pragma mark -


//
//
//
//
//
RpMaterial* forceLinearFilteringMatTexturesCB(RpMaterial *pMaterial, void *data)
{
	RwTexture *pTexture = pMaterial->texture;
	if(pTexture)
	{
		RwTextureSetFilterMode(pTexture, (RwTextureFilterMode)((uint32)data));
	}	
	return(pMaterial);
}


//
//
// forces selected filter mode on all textures of given atomic
//
bool8 SetFilterModeOnAtomicsTextures(RpAtomic *pAtomic, RwTextureFilterMode filterMode)
{
	ASSERT(pAtomic);
	
	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);
	
	RpGeometryForAllMaterials(pGeometry, forceLinearFilteringMatTexturesCB, (void*)filterMode);
	return(TRUE);
}

RpAtomic* forceLinearFilteringAtomicsCB(RpAtomic *pAtomic, void* data)
{
	ASSERT(pAtomic);
	
	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);
	
	RpGeometryForAllMaterials(pGeometry, forceLinearFilteringMatTexturesCB, data);
	return(pAtomic);
}

//
// forces selected filter mode on all textures of given atomic
//
bool8 SetFilterModeOnClumpsTextures(RpClump *pClump, RwTextureFilterMode filterMode)
{
	ASSERT(pClump);
	
	RpClumpForAllAtomics(pClump, forceLinearFilteringAtomicsCB, (void*)filterMode);
	return(TRUE);
}
#pragma mark -


//
//
// completely replaces old material with new one in given RpGeometry:
//
bool8 RpGeometryReplaceOldMaterialWithNewMaterial(RpGeometry *pGeometry, RpMaterial *pOldMaterial, RpMaterial *pNewMaterial)
{	
	ASSERT(pGeometry);
	ASSERT(pOldMaterial);
	ASSERT(pNewMaterial);
	
	RpMesh* pMesh = (RpMesh*)(pGeometry->mesh + 1);
	ASSERT(pMesh);

bool8 retval = FALSE;


	const int32 numMeshes = pGeometry->mesh->numMeshes;
	for(int32 i=0; i<numMeshes; i++)
	{
		ASSERT(pMesh->material->refCount>0);

		if(pMesh->material == pOldMaterial)
		{
			const int32 index = _rpMaterialListFindMaterialIndex(&pGeometry->matList, pMesh->material);
			ASSERT(index != pGeometry->matList.numMaterials);
			ASSERT(pMesh->material == pGeometry->matList.materials[index]);

			// destroy old material
			RpMaterialDestroy(pMesh->material);

			// setup new material pointers
			pGeometry->matList.materials[index]	= pNewMaterial;
			pMesh->material						= pNewMaterial;
				
			// reference material so it isn't deleted at the wrong time
			RpMaterialAddRef(pNewMaterial);
			
			retval = TRUE; // material found & replaced
		}

		pMesh++;
	}

	return(retval);
}

#pragma mark -



#ifdef GTA_TOON


//
//
//
//
CToon::CToon()
{
}


//
//
//
//
CToon::~CToon()
{
}





//
//
//
//
#define MAX_TOON_COUNT	(16)


//
//
//
//
static RpToonInk	*silhouetteInk[MAX_TOON_COUNT]	= {NULL},
					*creaseInk[MAX_TOON_COUNT]		= {NULL};
static RpToonPaint	*paint[MAX_TOON_COUNT]			= {NULL};

static RpLight *DirLight = NULL;
static RwFrame *LightFrame = NULL;



//
//
//
//
int CToon::Initialise()
{

	// make a silhouette ink for all models:
    if(!silhouetteInk[0])
    {
    	for(int i=0; i<MAX_TOON_COUNT; i++)
    	{
	        RwRGBA color = {0,0,0,255};//{30,70,70,255};//{192,192,192,255};//{128,128,128,255};//{255,255,255,255};
	        silhouetteInk[i] = RpToonInkCreate();
	        RpToonInkSetOverallThickness(silhouetteInk[i], 3.0);//silThickness);
	        RpToonInkSetColor(silhouetteInk[i], color );
	        char name[64];
	        ::sprintf(name, "silhouette%d", i);
	        RpToonInkSetName(silhouetteInk[i], name);//"silhouette" );
		}
    }

    // make a crease ink for all models:
    if(!creaseInk[0])
    {
    	for(int i=0; i<MAX_TOON_COUNT; i++)
    	{
	        RwRGBA color = {0,0,0,255};//{30,70,70,255};//{192,192,192,255};//{128,128,128,255};
	        creaseInk[i] = RpToonInkCreate();
	        RpToonInkSetOverallThickness(creaseInk[i], 2.0f);//creaseThickness);
	        RpToonInkSetColor(creaseInk[i], color );
			char name[64];
			::sprintf(name, "crease%d", i);
	        RpToonInkSetName(creaseInk[i], name);//"crease" );
    	}
    }
    
    if(!paint[0])
    {	
    	for(int i=0; i<MAX_TOON_COUNT; i++)
    	{
	    	paint[i] = RpToonPaintCreate();

			// Get the pointers to the textures
			CTxdStore::PushCurrentTxd();
			CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));
				//RwTexture *pTex = RwTextureRead("blueyellow", NULL);
				//RwTexture *pTex = RwTextureRead("bluemetallic", NULL);
				RwTexture *pTex = RwTextureRead("smooth", NULL);
			CTxdStore::PopCurrentTxd();
			RpToonPaintSetGradientTexture(paint[i], pTex);
			
	//		RpToonPaintSetType(paint, RPTOON_PAINTTYPE_FLAT);
		}
	}

	
	//
    // Here's the directional light that controls ALL the shading
    //
    if(!DirLight)
    {
	    DirLight = RpLightCreate(rpLIGHTDIRECTIONAL);
	    LightFrame = RwFrameCreate();
			{
		        RwV3d   xAxis = {1,0,0},
		                yAxis = {0,1,0};
		        RwFrameRotate(LightFrame, &xAxis, 150.0f, rwCOMBINEREPLACE);
		        RwFrameRotate(LightFrame, &yAxis, -150.0f, rwCOMBINEPOSTCONCAT);
			}

	    RpLightSetFrame(DirLight, LightFrame);
	    RpWorldAddLight(Scene.world, DirLight);
    }


	return(TRUE);
}


//
//
//
//
int CToon::Shutdown()
{

    if(silhouetteInk[0])
    {
    	for(int i=0; i<MAX_TOON_COUNT; i++)
    	{
	    	RpToonInkDestroy(silhouetteInk[i]);
	    	silhouetteInk[i] = NULL;
	    }
    }

    if(creaseInk[0])
    {
    	for(int i=0; i<MAX_TOON_COUNT; i++)
    	{
	    	RpToonInkDestroy(creaseInk[i]);
	    	creaseInk[i] = NULL;
	    }
    }
    
    if(paint[0])
    {	
    	for(int i=0; i<MAX_TOON_COUNT; i++)
    	{
	    	RpToonPaintDestroy(paint[i]);
	    	paint[i] = NULL;
    	}
    }
	
	
	
    if(DirLight)
    {
	    RwFrame *frame = RpLightGetFrame(DirLight);
	    RpLightSetFrame(DirLight, NULL);
	    RwFrameDestroy(frame);
	    frame = NULL;
	    
	    RpLightDestroy(DirLight);
	    DirLight = NULL;
    }

	return(TRUE);
}

	




//
//
//
//
//
static
float ProcessTexCoord(float v, int opt)
{

	switch(opt)
	{
		case(1):
		{
			return(v*1.0f);//0.8f);//1.0f);
			
			// for peds:
			if(v<0.1f)			return(0.1f);
			#define COND_STAT(VAL)	else if(v<VAL)	return(VAL)
				COND_STAT(0.2f);
				//COND_STAT(0.3f);
				COND_STAT(0.4f);
				//COND_STAT(0.5f);
				COND_STAT(0.6f);
				//COND_STAT(0.7f);
				COND_STAT(0.8f);
				//COND_STAT(0.9f);
				COND_STAT(1.0f);
				//COND_STAT(1.1f);
				COND_STAT(1.2f);
				//COND_STAT(1.3f);
				COND_STAT(1.4f);
				//COND_STAT(1.5f);
				COND_STAT(1.6f);
				//COND_STAT(1.7f);
				COND_STAT(1.8f);
				//COND_STAT(1.9f);
				COND_STAT(2.0f);
				//COND_STAT(2.1f);
				COND_STAT(2.2f);
				//COND_STAT(2.3f);
				COND_STAT(2.4f);
				//COND_STAT(2.5f);
				COND_STAT(2.6f);
				//COND_STAT(2.7f);
				COND_STAT(2.8f);
				//COND_STAT(2.9f);
				COND_STAT(3.0f);
				//COND_STAT(3.1f);
			#undef COND_STAT
			else return(v);
		}
		break;
		
		case(0):
		default:
		{
			return(v*0.4f);//1.0f);
			
			if(v<0.1f)			return(0.1f);
			#define COND_STAT(VAL)	else if(v<VAL)	return(VAL)
				COND_STAT(0.2f);
				COND_STAT(0.3f);
				COND_STAT(0.4f);
				COND_STAT(0.5f);
				COND_STAT(0.6f);
				COND_STAT(0.7f);
				COND_STAT(0.8f);
				COND_STAT(0.9f);
				COND_STAT(1.0f);
				COND_STAT(1.1f);
				COND_STAT(1.2f);
				COND_STAT(1.3f);
				COND_STAT(1.4f);
				COND_STAT(1.5f);
				COND_STAT(1.6f);
				COND_STAT(1.7f);
				COND_STAT(1.8f);
				COND_STAT(1.9f);
				COND_STAT(2.0f);
				COND_STAT(2.1f);
				COND_STAT(2.2f);
				COND_STAT(2.3f);
				COND_STAT(2.4f);
				COND_STAT(2.5f);
				COND_STAT(2.6f);
				COND_STAT(2.7f);
				COND_STAT(2.8f);
				COND_STAT(2.9f);
				COND_STAT(3.0f);
				COND_STAT(3.1f);
			#undef COND_STAT
			else return(v);
		}
		break;
	}

}

//
//
//
//
RpAtomic* CToon::ChangeAtomicMappingCB(RpAtomic *pAtomic, int opt)
{

	RpGeometry *pGeometry		= RpAtomicGetGeometry(pAtomic);

	const int numVerts			= RpGeometryGetNumVertices(pGeometry);
	const int numTriangles		= RpGeometryGetNumTriangles(pGeometry);
	const int numTexCoordSets	= RpGeometryGetNumTexCoordSets(pGeometry);
	const int numTexCoords		= numVerts;
	
	const int geoFlags = RpGeometryGetFlags(pGeometry);
	
	if((geoFlags&rpGEOMETRYTEXTURED)==0)
		return(pAtomic);
		
	
	RpGeometryLock(pGeometry, rpGEOMETRYLOCKTEXCOORDSALL);	
	for(int j=0; j<numTexCoordSets; j++ )
	{
		RwTexCoords *pTexCoords = RpGeometryGetVertexTexCoords(pGeometry, RwTextureCoordinateIndex(j + rwTEXTURECOORDINATEINDEX0));
		if(pTexCoords)
		{
			//const RwTexCoords tex = {0,0};
			for(int i=0; i<numTexCoords; i++)
			{
				pTexCoords[i].u = ProcessTexCoord(pTexCoords[i].u, opt);// *= getrand01();//0.5f;//0.0f;
				pTexCoords[i].v = ProcessTexCoord(pTexCoords[i].v, opt);//getrand01();//0.5f;//0.0f;
			}
		}
	}
	RpGeometryUnlock(pGeometry);

	return(pAtomic);
}



//
//
//
//
static
RpAtomic* ccmCB(RpAtomic *pAtomic, void *data)
{

	CToon::ChangeAtomicMappingCB(pAtomic, 0x01);
	return(pAtomic);
}

//
//
//
//
void CToon::ChangeClumpMapping(RpClump *pClump)
{
	RpClumpForAllAtomics(pClump, ccmCB, 0);
}



//
//
//
//
void CToon::ChangeAtomicMapping(RpAtomic *pAtomic)
{
	CToon::ChangeAtomicMappingCB(pAtomic, 0x00);
}




//
//
//
//
static
void Toonify(RpToonGeo *g)
{

static int count=0;

//	count++;
//	count &= (MAX_TOON_COUNT-1);

	
    // If there was no match, set some by hand:
    if(!RpToonGeoGetSilhouetteInk(g))
    {
        RpToonGeoSetSilhouetteInk(g, silhouetteInk[count]);
    } 
    
    if(!RpToonGeoGetCreaseInk(g))
    {
        RpToonGeoSetCreaseInk(g, creaseInk[count]);
    }

    RpToonGeoSetPaint(g, paint[count]);
}




//
//
//
//
static
RpAtomic* ToonifyAtomicMainCB(RpAtomic *a, void *data)
{

	const int bIsClump = *((int*)data);

    if(!RpToonAtomicEnable(a))
    {
		// error toonifying?
    	ASSERT(FALSE);
    	printf("RpToonAtomicEnable(): atomic not toonified correctly !!!\n");
    	return(a);	
	}



	if(bIsClump) // clump?
	{
	
		if(ISCLUMPSKINNED(RpAtomicGetClump(a)))
		{
			RpSkinAtomicSetType(a, rpSKINTYPETOON);		
		}
	}


	RpGeometry *pGeometry = RpAtomicGetGeometry(a);
	if(!pGeometry)
	{
		// error toonifying?
    	ASSERT(FALSE);
		return(a);
    }
    
    RpToonGeo *pToonGeo = RpToonGeometryGetToonGeo(pGeometry);
    if(!pToonGeo)
    {
		// error toonifying?
    	ASSERT(FALSE);
		return(a);
    }
    

    Toonify(pToonGeo);


    return(a);
}



//
//
//
//
void CToon::ToonifyClump(RpClump *pClump)
{
	const int bIsClump = TRUE;	//clump

	RpClumpForAllAtomics(pClump, ToonifyAtomicMainCB, (void*)&bIsClump); 
}


//
//
//
//
void CToon::ToonifyAtomic(RpAtomic *pAtomic)
{
	const int bIsClump = FALSE;	//atomic

	ToonifyAtomicMainCB(pAtomic, (void*)&bIsClump); 
}




#endif//GTA_TOON



RwTexture* RwTexDictionaryFindHashNamedTexture(RwTexDictionary* pTxd, uint32 hashKey)
{
    RwTexture          *result;
    RwLLLink           *cur, *end;

    cur = rwLinkListGetFirstLLLink(&pTxd->texturesInDict);
    end = rwLinkListGetTerminator(&pTxd->texturesInDict);

    while (cur != end)
    {
        result = rwLLLinkGetData(cur, RwTexture, lInDictionary);

        if (result->name)
        {
            if (CKeyGen::GetUppercaseKey(result->name) == hashKey)
                 return result;
        }

        cur = rwLLLinkGetNext(cur);
    }

    /* Not found */
    return NULL;
}

RwBool  RwIm2DRenderPrimitive_BUGFIX(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices)
{
#ifdef GTA_XBOX
	if (RsXboxGlobalInfo.deviceConfig.multiSampleType != D3DMULTISAMPLE_NONE) 
		Modify2DPrimitiveVertices(vertices, numVertices);
#endif
	return RwIm2DRenderPrimitive(primType, vertices, numVertices);
}

RwBool  RwIm2DRenderIndexedPrimitive_BUGFIX(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices, RwImVertexIndex *indices, RwInt32 numIndices)
{
#ifdef GTA_XBOX
	if (RsXboxGlobalInfo.deviceConfig.multiSampleType != D3DMULTISAMPLE_NONE) 
		Modify2DPrimitiveVertices(vertices, numVertices);
#endif
	return RwIm2DRenderIndexedPrimitive(primType, vertices, numVertices, indices, numIndices);
}

RwBool  RwIm2DRenderTriangle_BUGFIX(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2, RwInt32 vert3)
{
#ifdef GTA_XBOX
	if (RsXboxGlobalInfo.deviceConfig.multiSampleType != D3DMULTISAMPLE_NONE) 
		Modify2DPrimitiveVertices(vertices, numVertices);
#endif
	return RwIm2DRenderTriangle(vertices, numVertices, vert1, vert2, vert3);
}

RwBool  RwIm2DRenderLine_BUGFIX(RwIm2DVertex *vertices, RwInt32 numVertices, RwInt32 vert1, RwInt32 vert2)
{
#ifdef GTA_XBOX
	if (RsXboxGlobalInfo.deviceConfig.multiSampleType != D3DMULTISAMPLE_NONE) 
		Modify2DPrimitiveVertices(vertices, numVertices);
#endif
	return RwIm2DRenderLine(vertices, numVertices, vert1, vert2);
}





