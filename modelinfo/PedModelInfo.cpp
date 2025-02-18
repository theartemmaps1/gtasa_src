//
//
//    Filename: PedModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a Pedestrian model
//
//
#include "PedModelInfo.h"
#include "MemoryMgr.h"
#include "ModelInfo.h"
#include "VisibilityPlugins.h"
#include "HierarchyIds.h"
#include "SurfaceTable.h"
#include "NodeNamePlugin.h"
#include "General.h"
#include "Game.h"

#include "RpAnimBlendClump.h"
#include "VuCustomSkinPedRenderer.h"


//
// Hierarchy descriptions
//
RwObjectNameIdAssocation CPedModelInfo::m_pPedIds[] = {
	{"Smid", 		PED_TORSO, 	0},
	{"Shead", 		PED_HEAD, 	0},
	{"Supperarml",	PED_UPPERARML,	0},
	{"Supperarmr", 	PED_UPPERARMR, 	0},
	{"SLhand",		PED_HANDL,	0},
	{"SRhand", 		PED_HANDR, 	0},
	{"Supperlegl",	PED_UPPERLEGL,	0},
	{"Supperlegr",	PED_UPPERLEGR,	0},
	{"Sfootl",		PED_FOOTL,	0},
	{"Sfootr", 		PED_FOOTR, 	0},
	{"Slowerlegl",	PED_KNEEL,	0},
	{"Slowerlegr",	PED_KNEER,	0},
	{NULL,NULL, 0}
};

// keep old values for ref (sandy)
//	{NULL,			BONETAG_HEAD,		PED_SPHERE_HEAD,		0.0f,	0.05f,	0.15f},
//	{NULL,			BONETAG_SPINE1,		PED_SPHERE_CHEST,		0.0f,	0.15f,	0.20f},
//	{NULL,			BONETAG_SPINE1,		PED_SPHERE_CHEST,		0.0f,	-0.05f,	0.25f},
//	{NULL,			BONETAG_SPINE,		PED_SPHERE_MIDSECTION,	0.0f,	-0.25f,	0.25f},
//	{NULL,			BONETAG_L_UPPERARM,	PED_SPHERE_UPPERARM_L,	0.03f,	-0.05f,	0.16f},
//	{NULL,			BONETAG_R_UPPERARM,	PED_SPHERE_UPPERARM_R,	0.03f,-0.05f,	0.16f},
//	{NULL,			BONETAG_L_CALF,		PED_SPHERE_LEG_L,		0.0f,	0.15f,	0.20f},
//	{NULL,			BONETAG_R_CALF,		PED_SPHERE_LEG_R,		0.0f,	0.15f,	0.20f},
//	{NULL,			BONETAG_L_FOOT,		PED_SPHERE_LEG_L,		0.0f,	0.15f,	0.15f},
//	{NULL,			BONETAG_R_FOOT,		PED_SPHERE_LEG_R,		0.0f,	0.15f,	0.15f}


colModelNodeInfo CPedModelInfo::m_pColNodeInfos[NUM_PED_COLMODEL_NODES] = {
	{NULL,			BONETAG_HEAD,		PED_SPHERE_HEAD,		 0.05f,	0.0f,	0.0f,	0.15f},
	{NULL,			BONETAG_SPINE1,		PED_SPHERE_CHEST,		 0.20f,	0.0f,	0.0f,	0.20f},
	{NULL,			BONETAG_SPINE1,		PED_SPHERE_CHEST,		 0.0f,	0.0f,	0.0f,	0.20f},
	{NULL,			BONETAG_SPINE,		PED_SPHERE_MIDSECTION,	-0.1f,	0.0f,	0.0f,	0.20f},
	{NULL,			BONETAG_L_UPPERARM,	PED_SPHERE_UPPERARM_L,	 0.06f,	0.0f,	0.0f,	0.14f},
	{NULL,			BONETAG_R_UPPERARM,	PED_SPHERE_UPPERARM_R,	 0.06f,	0.0f,	0.0f,	0.14f},
	{NULL,			BONETAG_L_FOREARM,	PED_SPHERE_UPPERARM_L,	 0.05f,	0.0f,	0.0f,	0.14f},
	{NULL,			BONETAG_R_FOREARM,	PED_SPHERE_UPPERARM_R,	 0.05f,	0.0f,	0.0f,	0.14f},
	{NULL,			BONETAG_L_CALF,		PED_SPHERE_LEG_L,		-0.10f,	0.0f,	0.0f,	0.18f},
	{NULL,			BONETAG_R_CALF,		PED_SPHERE_LEG_R,		-0.10f,	0.0f,	0.0f,	0.18f},
	{NULL,			BONETAG_L_FOOT,		PED_SPHERE_LEG_L,		-0.18f,	0.0f,	0.0f,	0.16f},
	{NULL,			BONETAG_R_FOOT,		PED_SPHERE_LEG_R,		-0.18f,	0.0f,	0.0f,	0.16f}
};



//RwTexture* CPedModelInfo::m_pDefaultFaceTexture = NULL;
//RwTexture* CPedModelInfo::m_pLastFaceTexture = NULL;


//
//        name: CPedModelInfo::SetClump
// description: Set pointer to clump and update render callbacks etc
//
void CPedModelInfo::SetClump(RpClump* pClump) 
{

#if defined (GTA_PS2)
#ifndef FINAL
	//
	// when we are here, we can be sure, that CVSP ped clump was loaded:
	//
	if(!CVuCustomSkinPedRenderer::IsCVSPPipelineAttached(pClump))
	{
	//	ASSERTMSG(FALSE, "CVSP pipeline not attached to loaded ped clump! See Toks to fix it.");
	}
#endif //FINAL
#endif //GTA_PS2

	// note: CVSP initialization is done in CClumpModelInfo::SetClump() -> SetAtomicPipeCB():
	//CVuCustomSkinPedRenderer::ClumpSetup(pClump);



	CClumpModelInfo::SetClump(pClump);
	SetFrameIds(m_pPedIds);
	if(m_pHitColModel==NULL)
		CreateHitColModelSkinned(pClump);

	RpClumpForAllAtomics((RpClump*)m_pRwObject, &CClumpModelInfo::SetAtomicRendererCB, (void*) &CVisibilityPlugins::RenderPedCB);

#if defined(GTA_PC) && !defined(FINALBUILD)
	// This will not work with Hash keys
	// if this is the player model then setup player render callback
	if(!strcmp(GetModelName(), "player"))
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CClumpModelInfo::SetAtomicRendererCB, (void*) &CVisibilityPlugins::RenderPlayerCB);
#endif	

	RpHAnimHierarchy *pHierarchy = GETANIMHIERARCHYFROMCLUMP(pClump);
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_ROOT) != -1, GetModelName(), "Cannot find root tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_PELVIS) != -1, GetModelName(), "Cannot find pelvis tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_SPINE) != -1, GetModelName(), "Cannot find spine tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_SPINE1) != -1, GetModelName(), "Cannot find spine1 tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_NECK) != -1, GetModelName(), "Cannot find neck tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_HEAD) != -1, GetModelName(), "Cannot find head tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_BROW) != -1, GetModelName(), "Cannot find l brow tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_BROW) != -1, GetModelName(), "Cannot find r brow tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_JAW) != -1, GetModelName(), "Cannot find jaw tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_CLAVICLE) != -1, GetModelName(), "Cannot find r clavicle tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_UPPERARM) != -1, GetModelName(), "Cannot find r upperarm tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_FOREARM) != -1, GetModelName(), "Cannot find r forearm tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_HAND) != -1, GetModelName(), "Cannot find r hand tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_FINGERS) != -1, GetModelName(), "Cannot find r finger tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_FINGER01) != -1, GetModelName(), "Cannot find r finger1 tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_CLAVICLE) != -1, GetModelName(), "Cannot find l clavicle tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_UPPERARM) != -1, GetModelName(), "Cannot find l upperarm tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_FOREARM) != -1, GetModelName(), "Cannot find l forearm tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_HAND) != -1, GetModelName(), "Cannot find l hand tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_FINGERS) != -1, GetModelName(), "Cannot find l finger tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_FINGER01) != -1, GetModelName(), "Cannot find l finger1 tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_THIGH) != -1, GetModelName(), "Cannot find l thigh tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_CALF) != -1, GetModelName(), "Cannot find l calf tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_FOOT) != -1, GetModelName(), "Cannot find l foot tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_TOE) != -1, GetModelName(), "Cannot find l toe tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_THIGH) != -1, GetModelName(), "Cannot find r thigh tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_CALF) != -1, GetModelName(), "Cannot find r calf tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_FOOT) != -1, GetModelName(), "Cannot find r foot tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_TOE) != -1, GetModelName(), "Cannot find r toe tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_BELLY) != -1, GetModelName(), "Cannot find belly tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_BREAST) != -1, GetModelName(), "Cannot find r breast tag");
	ASSERTOBJ(RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_BREAST) != -1, GetModelName(), "Cannot find l breast tag");
}

void CPedModelInfo::DeleteRwObject()
{
	CClumpModelInfo::DeleteRwObject();
	if(m_pHitColModel)
		delete m_pHitColModel;
	m_pHitColModel = NULL;	
}

RpAtomic* GetAtomicListCB(RpAtomic* pAtomic, void* pData)
{
	**((RpAtomic***)pData) = pAtomic;
	(*((RpAtomic***)pData))++;
	return pAtomic;
}

RpAtomic* AttachAtomicsFromListCB(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	RpAtomic* pLowAtomic = **((RpAtomic***)pData);
	RpClump* pClump = RpAtomicGetClump(pAtomic);
	RpClump* pOldClump = RpAtomicGetClump(pLowAtomic);
	
	RpAtomicSetFrame(pLowAtomic, pFrame);
	RpClumpRemoveAtomic(pOldClump, pLowAtomic);
	RpClumpAddAtomic(pClump, pLowAtomic);
	
	// next in list
	(*((RpAtomic***)pData))++;

	return pAtomic;
}

RpAtomic* CountAtomicsCB(RpAtomic* pAtomic, void* pData)
{
	(*(int32*)pData)++;
	return pAtomic;
}

RwFrame* FindPedFrameFromNameCB(RwFrame* pFrame, void* pData)
{
	if(!CGeneral::faststricmp(((RwObjectNameAssoc*)pData)->pName+1, GetFrameNodeName(pFrame)+1))
	{
		((RwObjectNameAssoc*)pData)->pObject = (RwObject*)pFrame;
		return NULL;
	}
	
	RwFrameForAllChildren(pFrame, &FindPedFrameFromNameCB, pData);
	if(((RwObjectNameAssoc*)pData)->pObject)
		return NULL;
	return pFrame;
}

//
//        name: CPedModelInfo::SetLowDetailClump
// description: Set pointer to clump and update render callbacks etc
//
/*void CPedModelInfo::SetLowDetailClump(RpClump* pClump) 
{
	RpAtomic* pLowDetailAtomicArray[16];
	RpAtomic** ppArray = pLowDetailAtomicArray;
	int32 count1=0, count2=0;
	
	RpClumpForAllAtomics(m_pClump, &CountAtomicsCB, &count1);
	RpClumpForAllAtomics(pClump, &CountAtomicsCB, &count2);
	
	RpClumpForAllAtomics(m_pClump, &CClumpModelInfo::SetAtomicRendererCB, &CVisibilityPlugins::RenderPedHiDetailCB);
	RpClumpForAllAtomics(pClump, &CClumpModelInfo::SetAtomicRendererCB, &CVisibilityPlugins::RenderPedLowDetailCB);
	
	// I am assuming the hierarchy parts come in the same order
	// get list of atomics
	RpClumpForAllAtomics(pClump, &GetAtomicListCB, &ppArray);
	// add atomics to frame hierarchy and clump
//	ppArray = pLowDetailAtomicArray;
//	RpClumpForAllAtomics(m_pClump, &AttachAtomicsFromListCB, &ppArray);

	for(int32 i=0; i<count2; i++)
	{
		const char* pAtomicName = GetFrameNodeName(RpAtomicGetFrame(pLowDetailAtomicArray[i]));
		RwObjectNameAssoc nameAssoc;

		nameAssoc.pObject = NULL;
		nameAssoc.pName = pAtomicName;
		RwFrameForAllChildren(RpClumpGetFrame(m_pClump), &FindPedFrameFromNameCB, &nameAssoc);

		ASSERTOBJ(nameAssoc.pObject, pAtomicName, "Can't find ped component");
		if(nameAssoc.pObject)
		{
			RpAtomicSetFrame(pLowDetailAtomicArray[i], (RwFrame*)nameAssoc.pObject);
			RpClumpRemoveAtomic(pClump, pLowDetailAtomicArray[i]);
			RpClumpAddAtomic(m_pClump, pLowDetailAtomicArray[i]);
		}	
	}
}
*/
//
//        name: CPedModelInfo::AddXtraAtomics
// description: Add in extra atomics like the head and teeshirt symbols
//
void CPedModelInfo::AddXtraAtomics(RpClump* pClump)
{
}

//
// CPedModelInfo::SetFaceTexture: set the texture for the peds skin colour
// 
void CPedModelInfo::SetFaceTexture(RwTexture* pTexture)
{
}


RwObject * FindHeadRadiusCB(RwObject *object, void *data)
{
	float* pFloat = (float *)data;
	*pFloat = (RpAtomicGetBoundingSphere((RpAtomic *)object))->radius;
	return NULL;
}



void CPedModelInfo::CreateHitColModelSkinned(RpClump *pClump)
{
	RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump(pClump);
	RwMatrix *pMatrix = NULL;
	CColModel* pColModel = new CColModel();
	CColSphere* pColSpheres;
	CVector zero(0.0f,0.0f,0.0f);
	CVector posn;
	int32 nBone = -1;
//	RwMatrix *pInvertMatrix = RwMatrixCreate();
//	RwMatrix *pResultantMatrix = RwMatrixCreate();
	RwMatrix *pInvertMatrix = CGame::m_pWorkingMatrix1;
	RwMatrix *pResultantMatrix = CGame::m_pWorkingMatrix2;

	pColModel->AllocateData(NUM_PED_COLMODEL_NODES, 0, 0, 0, 0);
	ASSERTOBJ(pHierarchy, GetModelName(), "Doesn't have an animation hierarchy");
	
	RwMatrixInvert(pInvertMatrix, RwFrameGetMatrix(RpClumpGetFrame(pClump)));

	pColSpheres = pColModel->m_pColData->m_pSphereArray;
	
	for(int32 i=0;i<NUM_PED_COLMODEL_NODES;i++)
	{
		// reset position vector
//		posn = zero;
		posn.x = m_pColNodeInfos[i].xOffset;
		posn.y = m_pColNodeInfos[i].yOffset;
		posn.z = m_pColNodeInfos[i].zOffset;

		// reset resulatant matrix
		RwMatrixCopy(pResultantMatrix, pInvertMatrix);

		// get the node matrix for this node/bone
//		nBone = ConvertPedNode2BoneTag(m_pColNodeInfos[i].boneId);
//		nBone = RpHAnimIDGetIndex(pHierarchy, nBone);
//		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);

		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, m_pColNodeInfos[i].boneId));

		RwMatrixTransform(pResultantMatrix, pMatrix, rwCOMBINEPRECONCAT);
		// transform node into ped frame
		RwV3dTransformPoints(&posn, &posn, 1, pResultantMatrix);
		
		pColSpheres[i].m_vecCentre = posn;// + CVector(m_pColNodeInfos[i].fHorzOffset,0.0f,m_pColNodeInfos[i].fVertOffset);
		pColSpheres[i].m_fRadius = m_pColNodeInfos[i].fRadius;
			
		pColSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_PED; 
		pColSpheres[i].m_data.m_nPieceType = m_pColNodeInfos[i].nPieceType;
	}

//	RwMatrixDestroy(pInvertMatrix);
//	RwMatrixDestroy(pResultantMatrix);
	
//	pColModel->m_pSphereArray = pColSpheres;
//	pColModel->m_nNoOfSpheres = NUM_PED_COLMODEL_NODES;
	pColModel->GetBoundSphere().Set(1.5f, CVector(0.0f, 0.0f, 0.0f));
	pColModel->GetBoundBox().Set(CVector(-0.5f, -0.5f, -1.2f), CVector(0.5f, 0.5f, 1.2f));
	pColModel->m_level = LEVEL_GENERIC;	
	m_pHitColModel = pColModel;
}


// Returns a pointer to the peds hit collision model
// which has been animated to match the peds pose in the peds local frame
//
// i.e. all collision sphere coordinates are in local coordinates
CColModel *CPedModelInfo::AnimatePedColModelSkinned(RpClump *pClump)
{
	if(m_pHitColModel == NULL)
	{
		CreateHitColModelSkinned(pClump);
		return m_pHitColModel;
	}
	
	CCollisionData* pHitColData = m_pHitColModel->GetCollisionData();
	RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump(pClump);
	RwMatrix *pMatrix = NULL;
	CVector zero(0.0f,0.0f,0.0f);
	CVector posn;
	int32 nBone = -1;
//	RwMatrix *pInvertMatrix = RwMatrixCreate();
//	RwMatrix *pResultantMatrix = RwMatrixCreate();
	RwMatrix *pInvertMatrix = CGame::m_pWorkingMatrix1;
	RwMatrix *pResultantMatrix = CGame::m_pWorkingMatrix2;

	RwMatrixInvert(pInvertMatrix, RwFrameGetMatrix(RpClumpGetFrame(pClump)));

	for(int32 i=0;i<NUM_PED_COLMODEL_NODES;i++)
	{
		// reset position vector
//		posn = zero;
		posn.x = m_pColNodeInfos[i].xOffset;
		posn.y = m_pColNodeInfos[i].yOffset;
		posn.z = m_pColNodeInfos[i].zOffset;

		// reset resulatant matrix
		RwMatrixCopy(pResultantMatrix, pInvertMatrix);

		// get the node matrix for this node/bone
//		nBone = ConvertPedNode2BoneTag(m_pColNodeInfos[i].frameId);
//		nBone = RpHAnimIDGetIndex(pHierarchy, nBone);
//		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);

		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, m_pColNodeInfos[i].boneId));

		RwMatrixTransform(pResultantMatrix, pMatrix, rwCOMBINEPRECONCAT);

		// transform node into ped frame
		RwV3dTransformPoints(&posn, &posn, 1, pResultantMatrix);
		
		pHitColData->m_pSphereArray[i].m_vecCentre = posn;// + CVector(m_pColNodeInfos[i].fHorzOffset,0.0f,m_pColNodeInfos[i].fVertOffset);
	}
	
	posn = zero;
	RwMatrixCopy(pResultantMatrix, pInvertMatrix);
	nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_SPINE1);
	pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	RwMatrixTransform(pResultantMatrix, pMatrix, rwCOMBINEPRECONCAT);
	// transform node into ped frame
	RwV3dTransformPoints(&posn, &posn, 1, pResultantMatrix);
	
	m_pHitColModel->GetBoundSphere().Set(1.5f, posn);
	m_pHitColModel->GetBoundBox().Set(posn - CVector(1.2f, 1.2f, 1.2f), posn + CVector(1.2f, 1.2f, 1.2f));
	
//	RwMatrixDestroy(pInvertMatrix);
//	RwMatrixDestroy(pResultantMatrix);

	return m_pHitColModel;
}



// Returns a pointer to the peds hit collision model
// which has been animated to match the peds pose in the world frame
//
// i.e. all collision sphere positions are in world coordinates
CColModel *CPedModelInfo::AnimatePedColModelSkinnedWorld(RpClump *pClump)
{
	if(m_pHitColModel == NULL)
	{
		CreateHitColModelSkinned(pClump);
		// this gives col model animated to local coordinates, so need to do it again
	}

	CCollisionData* pHitColData = m_pHitColModel->GetCollisionData();
	RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump(pClump);
	RwMatrix *pMatrix = NULL;
	CVector zero(0.0f,0.0f,0.0f);
	CVector posn;
	int32 nBone = -1;

	for(int32 i=0;i<NUM_PED_COLMODEL_NODES;i++)
	{
		// reset position vector
//		posn = zero;
		posn.x = m_pColNodeInfos[i].xOffset;
		posn.y = m_pColNodeInfos[i].yOffset;
		posn.z = m_pColNodeInfos[i].zOffset;

		// get the node matrix for this node/bone
//		nBone = ConvertPedNode2BoneTag(m_pColNodeInfos[i].frameId);
//		nBone = RpHAnimIDGetIndex(pHierarchy, nBone);
//		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);

		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, m_pColNodeInfos[i].boneId));
		
		// transform node into world
		RwV3dTransformPoints(&posn, &posn, 1, pMatrix);
		
		pHitColData->m_pSphereArray[i].m_vecCentre = posn;// + CVector(m_pColNodeInfos[i].fHorzOffset,0.0f,m_pColNodeInfos[i].fVertOffset);
	}

	posn = zero;
	nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_SPINE1);
	pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	// transform node into world
	RwV3dTransformPoints(&posn, &posn, 1, pMatrix);

	m_pHitColModel->GetBoundSphere().Set(1.5f, posn);
	m_pHitColModel->GetBoundBox().Set(posn - CVector(1.2f, 1.2f, 1.2f), posn + CVector(1.2f, 1.2f, 1.2f));

	return m_pHitColModel;
}


void CPedModelInfo::IncrementVoice(void)
{
	if((m_FirstVoice<0) || (m_LastVoice<0))
	{
		m_NextVoice = -1;
		return;
	}
	
	m_NextVoice++;
	if ((m_NextVoice>m_LastVoice) || (m_NextVoice<m_FirstVoice))
		m_NextVoice = m_FirstVoice;
	
	return;
}


