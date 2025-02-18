//
//
//    Filename: ClumpModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a model with a RenderWare Clump attached
//
//
#include "handy.h"
#include "ClumpModelInfo.h"
#include "NodeNamePlugin.h"
#include "VisibilityPlugins.h"
#include "AnimManager.h"
#include "RpAnimBlend.h"
#include "2deffect.h"

//!PC - stuff not supported on PC
#ifdef GTA_PS2
	#include "PS2All.h"
	#include "eemacros.h"
	#include "VuCustomBuildingRenderer.h"
	#include "VuCustomSkinPedRenderer.h"
	#include "VuCarFXRenderer.h"
#else//GTA_PS2
	#include "CustomBuildingRenderer.h"
	#include "CarFXRenderer.h"
#endif //GTA_PS2

#include "general.h"
#include "TxdStore.h"

#include "RpAnimBlendClump.h"	//ISCLUMPSKINNED()...


void CheckBones(RpHAnimHierarchy* pHier)
{
#ifndef FINAL
    for(int32 i=0; i<pHier->numNodes; i++)
    {
        if(pHier->pNodeInfo[i].flags > 4)
        {
#if defined (GTA_PS2)
            printf("Problem with Hierarchy node %d\n", i);
            asm volatile ("breakc 1");
#else //GTA_PS2
			char errString [80];
			sprintf(errString, "Problem with Hierarchy node %d\n",i);
			ASSERTMSG(FALSE,errString);
#endif //GTA_PS2
        }           
    }
#endif    
}

 


void CClumpModelInfo::Init() 
{
	CBaseModelInfo::Init(); 

	m_animFileIndex = -1; 
}

void CClumpModelInfo::Shutdown()
{
	CBaseModelInfo::Shutdown(); 
}

//
//        name: CClumpModelInfo::DeleteRwObject
// description: Destroy associated RenderWare Clump 
//
void CClumpModelInfo::DeleteRwObject()
{
	if(m_pRwObject) 
	{
		RpAtomic* pAtomic = Get2DEffectAtomic((RpClump*)GetRwObject());
		if (pAtomic)
		{
			RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
			ASSERT(pGeom);
			m_num2dEffects -= RpGeometryGetNum2dEffects(pGeom);
		}

		RpClumpDestroy((RpClump*)m_pRwObject);
		m_pRwObject = NULL;
		
		RemoveTexDictionaryRef();
		if(GetAnimFileIndex() != -1)
			CAnimManager::RemoveAnimBlockRef(GetAnimFileIndex());

		// delete collision if it came in with RW model	
		if(GetWasCollisionStreamedWithModel())
			DeleteCollisionModel();
	}	
}

//
//
// name:		SetHierarchyForSkinAtomic
// description:	Set the skinning hierarchy for the atomics in a clump
static RpAtomic* SetHierarchyForSkinAtomic(RpAtomic *pAtomic, void *data)
{
	// If data is not null just set hierarchy to null
	if(data != NULL)
	{
	    RpSkinAtomicSetHAnimHierarchy(pAtomic, (RpHAnimHierarchy*)data);
	    return NULL;
	}
	// otherwise find hierarchy
	RpHAnimHierarchy* pHierarchy = GetAnimHierarchyFromFrame(RpAtomicGetFrame(pAtomic));
	ASSERTOBJ(pHierarchy, GetFrameNodeName(RpAtomicGetFrame(pAtomic)), "Cannot find animation hierarchy");
    RpSkinAtomicSetHAnimHierarchy(pAtomic, pHierarchy);
    
    return pAtomic;
}



/*
void 	CheckAtomicForDuplicateVertices	(char* name, RpAtomic* pAtomic);

RpAtomic* CheckDuplicateVertsCB(RpAtomic *pAtomic, void *data)
{
	CheckAtomicForDuplicateVertices((char*)data, pAtomic);
	
	return pAtomic;
}

*/
static
RpAtomic* SetAtomicPipeCB(RpAtomic *pAtomic, void *data)
{

CClumpModelInfo *clumpMI = (CClumpModelInfo*)data;
#if ((defined (GTA_PC) || defined(GTA_XBOX)))
	if(CCustomBuildingRenderer::IsCBPCPipelineAttached(pAtomic))
	{
		// do CVB setup on loaded atomic (usually animated countryside objects):
		CCustomBuildingRenderer::AtomicSetup(pAtomic);
		//ASSERTMSG(FALSE, "CVBuildingPipe: Buildings shouldn't be loaded as clumps! See Andrzej.");
	}
	else if(CCarFXRenderer::IsCCPCPipelineAttached(pAtomic))
	{
		CCarFXRenderer::CustomCarPipeAtomicSetup(pAtomic);
	}
#endif

#if defined (GTA_PS2)
	//setup all the ps2 custom renderer callbacks...
	if(CVuCustomSkinPedRenderer::IsCVSPPipelineAttached(pAtomic))
	{
		// do CVSP setup on loaded atomic:
		CVuCustomSkinPedRenderer::AtomicSetup(pAtomic);
	}
	else if(CVuCustomBuildingRenderer::IsCVBPipelineAttached(pAtomic))
	{
		// do CVB setup on loaded atomic (usually animated countryside objects):
		CVuCustomBuildingRenderer::AtomicSetup(pAtomic);
		//ASSERTMSG(FALSE, "CVBuildingPipe: Buildings shouldn't be loaded as clumps! See Andrzej.");
	}
	else if(CVuCarFXRenderer::IsCVCPipelineAttached(pAtomic))
	{
		// do CVC setup on loaded atomic:
		//CVuCarFXRenderer::CustomCVCarPipeAtomicSetup(pAtomic);
		//ASSERTMSG(FALSE, "CVCarPipe: Vehicle pipe initialisation shouldn't be done here! See Andrzej.");

		#ifndef FINAL
			extern char *dbgLoadedModelName;
			// set name of the model we're going to load (this is for printing debug/assert info purposes only):
			dbgLoadedModelName = (char*)clumpMI->GetModelName();
		#endif
		CVuCarFXRenderer::SetCustomFXAtomicRenderPipelinesVMICB(pAtomic, NULL);
	}
	else
	{
		PS2AllSetAtomicPipe(pAtomic);
	}
#endif //GTA_PS2


	return pAtomic;
}



//
//        name: CClumpModelInfo::SetClump
// description: Set pointer to clump
//
void CClumpModelInfo::SetClump(RpClump* pClump) 
{

	//sort out the 2dfx counter
	RpGeometry* pGeom = NULL;
	//take off the current value if there is an atomic
	if(m_pRwObject)
	{
		RpAtomic* pAtomic = Get2DEffectAtomic((RpClump*)m_pRwObject);
		if (pAtomic)
		{
			pGeom = RpAtomicGetGeometry(pAtomic);
			ASSERT(pGeom);
			m_num2dEffects -= RpGeometryGetNum2dEffects(pGeom);
		}
	}
	
	m_pRwObject = (RwObject*)pClump;


	//sort out the 2dfx counter
	//add the current value if there is an atomic
	if(m_pRwObject)
	{
		RpAtomic* pAtomic = Get2DEffectAtomic((RpClump*)m_pRwObject);
		if (pAtomic)
		{
			pGeom = RpAtomicGetGeometry(pAtomic);
			ASSERT(pGeom);
			m_num2dEffects += RpGeometryGetNum2dEffects(pGeom);
		}
	}


	CVisibilityPlugins::SetClumpModelInfo((RpClump*)m_pRwObject, this);

	AddTexDictionaryRef();
	if(GetAnimFileIndex() != -1)
		CAnimManager::AddAnimBlockRef(GetAnimFileIndex());
	
	
//	RpClumpForAllAtomics(pClump, CheckDuplicateVertsCB, (void*)GetModelName());


	// don't change vehicles pipeline as they need the matfx info to decide 
	// if they should have carfx pipeline attached
	if(TRUE)	//GetModelType() != MI_TYPE_VEHICLE)
	{
		RpClumpForAllAtomics(pClump, SetAtomicPipeCB, (void*)this);
	}

	
	
	if(ISCLUMPSKINNED(pClump))
	{ 
		if(GetHasComplexHierarchy())
		{
			RpClumpForAllAtomics(pClump, SetHierarchyForSkinAtomic, NULL);
		}
		else
		{
			RwFrame *pFrame = RpClumpGetFrame(pClump);
			// if there is no frame then file is invalid
			ASSERTMSG(pFrame, "Invalid Skinned Clump");

			// allow a little leeway for choosing clipping over non-clipping code
			RpAtomic* pAtomic = GetFirstAtomic(pClump);
			RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
			pGeom->morphTarget[0].boundingSphere.radius *= 1.2f;

		   	RpHAnimHierarchy *pHierarchy = GETANIMHIERARCHYFROMCLUMP(pClump);
			ASSERTMSG(pHierarchy, "No valid animation hierarchy detected in clump!");

			RpClumpForAllAtomics(pClump, SetHierarchyForSkinAtomic, (void*)pHierarchy);

			#ifdef GTA_PC
				// normalise all matrix weights
				pAtomic= GETATOMICFROMCLUMP(pClump);
				RpSkin		*pSkin	= RpSkinAtomicGetSkin(pAtomic);
				pGeom	= RpAtomicGetGeometry(pAtomic);
				for(int32 i=0; i<pGeom->numVertices; i++)
				{
					RwMatrixWeights *pW = (RwMatrixWeights*)&RpSkinGetVertexBoneWeights(pSkin)[i];
					const float w = pW->w0 + pW->w1 + pW->w2 + pW->w3;
					pW->w0 /= w;
					pW->w1 /= w;
					pW->w2 /= w;
					pW->w3 /= w;
				}
			#endif//GTA_PC


			RpHAnimHierarchySetFlags(pHierarchy, (RpHAnimHierarchyFlag)(rpHANIMHIERARCHYUPDATEMODELLINGMATRICES|rpHANIMHIERARCHYUPDATELTMS));
			
			CheckBones(pHierarchy);
		}
	}
}// end of CClumpModelInfo::SetClump()...


//
//        name: CClumpModelInfo::CreateInstance
// description: Create an instance of this object
//
RwObject* CClumpModelInfo::CreateInstance(RwMatrix* pMatrix)
{
	if(m_pRwObject == NULL)
		return NULL;

	RpClump* pClump = (RpClump*)CreateInstance();
	RwFrame* pFrame = RpClumpGetFrame(pClump);
	RwMatrixCopy(RwFrameGetMatrix(pFrame), pMatrix);

	return (RwObject*)pClump;
}



//
//        name: CClumpModelInfo::CreateInstance
// description: Create an instance of this object
//
RwObject* CClumpModelInfo::CreateInstance()
{
	if(m_pRwObject == NULL)
		return NULL;

	AddRef();
	
	RpClump *pClump = RpClumpClone((RpClump*)m_pRwObject);


	if(ISCLUMPSKINNED(pClump) && !GetHasComplexHierarchy())
	{
		RpHAnimHierarchy *pHierarchy = GETANIMHIERARCHYFROMCLUMP(pClump);
		ASSERTMSG(pHierarchy, "Invalid hierarchy in clump!");
	    RpClumpForAllAtomics(pClump, SetHierarchyForSkinAtomic, (void*)pHierarchy);

		RpHAnimAnimation *pAnim = RpAnimBlendCreateAnimationForHierarchy(pHierarchy);
		ASSERT(pAnim);
		RpHAnimHierarchySetCurrentAnim(pHierarchy, pAnim);

		RpHAnimHierarchySetFlags(pHierarchy, (RpHAnimHierarchyFlag)(rpHANIMHIERARCHYUPDATEMODELLINGMATRICES|rpHANIMHIERARCHYUPDATELTMS));

		CheckBones(pHierarchy);
	}//if(ISCLUMPSKINNED(pClump))...

	if(GetIsAnimated())
	{
		RpAnimBlendClumpInit(pClump);
		CAnimBlock* pAnimBlock = CAnimManager::GetAnimationBlock(m_animFileIndex);
#ifdef USE_MODELINFO_HASHKEY
		CAnimBlendHierarchy* pHier = CAnimManager::GetAnimation(GetHashKey(), pAnimBlock);
#else		
		CAnimBlendHierarchy* pHier = CAnimManager::GetAnimation(GetModelName(), pAnimBlock);
#endif		
		// If an animation is found
		if(pHier)
			CAnimManager::BlendAnimation(pClump, pHier, ABA_FLAG_ISLOOPED, 1.0f);
	}

	RemoveRef();
	
	return (RwObject*)pClump;

}// end of CClumpModelInfo::CreateInstance()...


//
// name:		SetAnimFile
// description:	Set anim file required when loading this model
void CClumpModelInfo::SetAnimFile(const char* pName)
{
	if(!stricmp(pName, "null"))
		return;
	// temporarily let animFileIndex be a pointer to the anim block
	m_animFileIndex = (int32)new char[strlen(pName)+1];
	strcpy((char*)m_animFileIndex, pName);
}

//
// name:		ConvertAnimFileIndex
// description:	Convert animfile name to an animfile index
void CClumpModelInfo::ConvertAnimFileIndex()
{
	if(m_animFileIndex == -1)
		return;
	int32 index = CAnimManager::GetAnimationBlockIndex((char*)m_animFileIndex);
	
	delete[] (char*)m_animFileIndex;
	m_animFileIndex = index;
}

//
// name:		SetAtomicRendererCB
// description:	Set the renderer for each atomic
RpAtomic* CClumpModelInfo::SetAtomicRendererCB(RpAtomic* pAtomic, void* pData)
{
	CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, (RpAtomicCallBackRender)pData);

	// DW a temporary hack to make sure peds are mip maped... to be removed and put somewhere better when we really know why they are not automatically through the streaming mip mapped.
	// JW mipmapping seems to cause a problem in the allocation of memory to player textures dicts - disabled for now.
	// MN removed totally and set in the correct place where all textures are loaded in
//	SetFilterModeOnAtomicsTextures(pAtomic, rwFILTERLINEAR);

	return pAtomic;
}

//
// Find Frame callback function
//
RwFrame* CClumpModelInfo::FindFrameFromNameCB(RwFrame* pFrame, void* pData)
{
	if(!CGeneral::faststricmp(((RwObjectNameAssoc*)pData)->pName, GetFrameNodeName(pFrame)))
	{
		((RwObjectNameAssoc*)pData)->pObject = (RwObject*)pFrame;
		return NULL;
	}
	
	RwFrameForAllChildren(pFrame, &FindFrameFromNameCB, pData);
	if(((RwObjectNameAssoc*)pData)->pObject)
		return NULL;
	return pFrame;
}

RwFrame* CClumpModelInfo::FindFrameFromNameWithoutIdCB(RwFrame* pFrame, void* pData)
{
	if(CVisibilityPlugins::GetFrameHierarchyId(pFrame) == 0 && 
		!CGeneral::faststricmp(((RwObjectNameAssoc*)pData)->pName, GetFrameNodeName(pFrame)))
	{
		((RwObjectNameAssoc*)pData)->pObject = (RwObject*)pFrame;
		return NULL;
	}
	
	RwFrameForAllChildren(pFrame, &FindFrameFromNameWithoutIdCB, pData);
	if(((RwObjectNameAssoc*)pData)->pObject)
		return NULL;
	return pFrame;
}

//
// Find Frame callback function
//
RwFrame* CClumpModelInfo::FindFrameFromIdCB(RwFrame* pFrame, void* pData)
{
	if(((RwObjectIdAssoc*)pData)->id == CVisibilityPlugins::GetFrameHierarchyId(pFrame))
	{
		((RwObjectIdAssoc*)pData)->pObject = (RwObject*)pFrame;
		return NULL;
	}
	
	RwFrameForAllChildren(pFrame, &FindFrameFromIdCB, pData);
	if(((RwObjectNameAssoc*)pData)->pObject)
		return NULL;
	return pFrame;
}

//
// Fill Frame array callback function
//
RwFrame* CClumpModelInfo::FillFrameArrayCB(RwFrame* pFrame, void* pData)
{
	int32 id = CVisibilityPlugins::GetFrameHierarchyId(pFrame);
	if(id > 0)
	{
		*(((RwFrame**)pData) + id) = pFrame;
	}
	
	RwFrameForAllChildren(pFrame, &FillFrameArrayCB, pData);
	return pFrame;
}

//
// CVehicleModelInfo::GetFrameFromId: Get the frame in this clump with the specified ID
//
RwFrame* CClumpModelInfo::GetFrameFromId(RpClump* pClump, int32 id)
{
	RwObjectIdAssoc assoc;
		
	assoc.pObject = NULL;
	assoc.id = id;
	
	RwFrameForAllChildren(RpClumpGetFrame(pClump), &FindFrameFromIdCB, &assoc);
	
	return (RwFrame*)assoc.pObject;
	
}

//
// CVehicleModelInfo::GetFrameFromName: Get the frame in this clump with the specified name
//
RwFrame* CClumpModelInfo::GetFrameFromName(RpClump* pClump, const char* pName)
{
	RwObjectNameAssoc assoc;
		
	assoc.pObject = NULL;
	assoc.pName = pName;
	
	RwFrameForAllChildren(RpClumpGetFrame(pClump), &FindFrameFromNameCB, &assoc);
	
	return (RwFrame*)assoc.pObject;
	
}

//
// CVehicleModelInfo::FillFrameArray: Fill an array with pointers to all the valid frames in 
//						this vehicle model
//
void CClumpModelInfo::FillFrameArray(RpClump* pClump, RwFrame** ppFrame)
{
	RwFrameForAllChildren(RpClumpGetFrame(pClump), &FillFrameArrayCB, ppFrame);
}

//
// SetFrameIds: set the Ids for all the members of the hierarchy
//
void CClumpModelInfo::SetFrameIds(RwObjectNameIdAssocation* pAssocArray)
{
	int32 i=0;
	
	while(pAssocArray[i].pName != NULL)
	{
		RwObjectNameAssoc nameAssoc;
		
		if(!(pAssocArray[i].flags & CLUMP_NO_FRAMEID))
		{
			nameAssoc.pObject = NULL;
			nameAssoc.pName = pAssocArray[i].pName;
			RwFrameForAllChildren(RpClumpGetFrame((RpClump*)m_pRwObject), &FindFrameFromNameWithoutIdCB, &nameAssoc);
			
			if(nameAssoc.pObject)
			{
				CVisibilityPlugins::SetFrameHierarchyId((RwFrame*)nameAssoc.pObject, pAssocArray[i].hierId);
			}
	#ifdef DEBUG
	/*		else
			{
				sprintf(gString, "Couldn't find component %s for %s\n", nameAssoc.pName, GetModelName());
				DEBUGLOG(gString);
			}*/
	#endif
		}
		i++;
	}
}

/*RwObject* SetAttachmentAtomicPtr(RwObject* pObject, void* data)
{
	if(RwObjectGetType(pObject) == rpATOMIC)
	{
		((RpClumpAttachment*)data)->pAtomic = (RpAtomic*)pObject;
		return NULL;
	}
	return pObject;
}*/

//
// CClumpModelInfo::AddXtraAtomic: Add an extra component to the clump from the list of indices in the AttachmentGroup 
//									into the list of attachments.
//
/*RpAtomic* CClumpModelInfo::AddXtraAtomic(RpClump* pClump, RpClumpAttachment* pAttachments, RpClumpAttachmentGroup& attachGroup)
{
	int32 num = CGeneral::GetRandomNumberInRange(0, attachGroup.num);
	
	num = attachGroup.indices[num];
	
	if(pAttachments[num].hierarchyId == -1)
		return NULL;
	RwFrame* pFrame = GetFrameFromId(pClump, pAttachments[num].hierarchyId);

	ASSERTOBJ(pAttachments[num].pAtomic != NULL, pAttachments[num].pName, "Cannot find clump attachment");
	ASSERTOBJ(pAttachments[num].pAtomic != NULL, pAttachments[num].pName, "Cannot find frame from hierarchy Id for clump attachment");

	RpAtomic* pAtomic = RpAtomicClone(pAttachments[num].pAtomic);
	RpAtomicSetFrame(pAtomic, pFrame);
	RpClumpAddAtomic(pClump, pAtomic);
	
	return pAtomic;
}*/
