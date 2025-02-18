//
//
//    Filename: AtomicModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a simple model( consists of one atomic)
//
//
#include "AtomicModelInfo.h"
#include "VehicleModelInfo.h"
#include "ModelInfo.h"
#include "TimeModelInfo.h"
#include "VisibilityPlugins.h"
#include "HierarchyIds.h"
#include "TxdStore.h"
#include "camera.h"
#include "general.h"
#include "globals.h"
#include "renderer.h"
#include "AnimManager.h"
#include "Camera.h"

#include "tag.h"

#include "2DEffect.h"

//!PC - stuff which PC doesn't support
#if defined (GTA_PS2)
	#include "ps2all.h"
	#include "VuCarFXRenderer.h"
	#include "VuCustomBuildingRenderer.h"
	#include "VuCustomSkinPedRenderer.h"
#else //GTA_PS2
	#include "CustomBuildingRenderer.h"
#endif //GTA_PS2


//void 	CheckAtomicForDuplicateVertices	(char* name, RpAtomic* pAtomic);
bool CDamageAtomicModelInfo::m_bCreateDamagedVersion = false;

//
// name:		SetMatfxFrameCB
// description:	Callback to set frame for MatFX environment maps
/*RpMaterial* SetMatfxFrameCB(RpMaterial* pMaterial, void* param)
{
	if(RpMatFXMaterialGetEffects(pMaterial) == rpMATFXEFFECTENVMAP)
		RpMatFXMaterialSetEnvMapFrame(pMaterial, (RwFrame*)param);
	return pMaterial;
}*/



//
// name:		SetAtomic
// description:	Set atomic for modelinfo
void CAtomicModelInfo::SetAtomic(RpAtomic* pAtomic) 
{
	//sort out the 2dfx counter
	RpGeometry* pGeom = NULL;

	//take off the current value if there is an atomic
	if(m_pRwObject)
	{
		pGeom = RpAtomicGetGeometry((RpAtomic*)m_pRwObject);
		ASSERT(pGeom);
		m_num2dEffects -= RpGeometryGetNum2dEffects(pGeom);	
	}
	
	//set the atomic
	
	m_pRwObject = (RwObject*)pAtomic;
	
	//add on the new value of the atomic

	pGeom = RpAtomicGetGeometry((RpAtomic*)m_pRwObject);
	ASSERT(pGeom);
	m_num2dEffects += RpGeometryGetNum2dEffects(pGeom);	
	
//	CheckAtomicForDuplicateVertices((char*)GetModelName(), m_pAtomic);

	// reference objects this model info requires
	AddTexDictionaryRef();
	if(GetAnimFileIndex() != -1)
		CAnimManager::AddAnimBlockRef(GetAnimFileIndex());


	{
#if (defined (GTA_PC) || defined (GTA_XBOX))
	if(CCustomBuildingRenderer::IsCBPCPipelineAttached(pAtomic))
	{
		// do CVB setup on loaded atomic (usually animated countryside objects):
		CCustomBuildingRenderer::AtomicSetup(pAtomic);
		//ASSERTMSG(FALSE, "CVBuildingPipe: Buildings shouldn't be loaded as clumps! See Andrzej.");
	}
	else if(CCarFXRenderer::IsCCPCPipelineAttached(pAtomic))
	{
		// do CustomCarFX setup on loaded atomic (usually they are car mods):
		//CCarFXRenderer::CustomCarPipeAtomicSetup(pAtomic);
		#ifndef FINAL
			// set name of the model we're going to load (this is for printing debug/assert info purposes only):
			extern char *carfxDbgLoadedModelName;
			carfxDbgLoadedModelName = this->m_modelName;
		#endif
		// extended VehicleModelInfo-type setup on loaded car mod:
		CCarFXRenderer::SetCustomFXAtomicRenderPipelinesVMICB(pAtomic, NULL);
	}
#endif //GTA_PC


#if defined (GTA_PS2)
		if(CVuCustomBuildingRenderer::IsCVBPipelineAttached(pAtomic))
		{
			// do CVB setup on loaded atomic:
			CVuCustomBuildingRenderer::AtomicSetup(pAtomic);
		}
		else if(CVuCarFXRenderer::IsCVCPipelineAttached(pAtomic))
		{
			// do CVC setup on loaded atomic (car mods usually):
			//CVuCarFXRenderer::CustomCVCarPipeAtomicSetup(pAtomic);

			#ifndef FINAL
				extern char *dbgLoadedModelName;
				// set name of the model we're going to load (this is for printing debug/assert info purposes only):
				dbgLoadedModelName = this->m_modelName;
			#endif

			// extended VehicleModelInfo-type setup on loaded car mod:
			CVuCarFXRenderer::SetCustomFXAtomicRenderPipelinesVMICB(pAtomic, NULL);
		}
		else if(CVuCustomSkinPedRenderer::IsCVSPPipelineAttached(pAtomic))
		{
			// do CVSP setup on loaded atomic:
			// CVuCustomSkinPedRenderer::AtomicSetup(pAtomic);
			ASSERTMSG(FALSE, "CVSkinPedPipe: Peds shouldn't be loaded as atomics! See Andrzej.");
		}
		else
		{
			PS2AllSetAtomicPipe(pAtomic);
		}
#endif //GTA_PS2
	}

	if(!GetIsUpgrade() && GetIsTag())
		CTagManager::SetupAtomic(pAtomic);

	// Use this to define that an object has been prerendered
	SetHasBeenPreRendered(true);
}

//
//        name: CAtomicModelInfo::Init
// description: Initialise Simple object values
//
void CAtomicModelInfo::Init()
{
	CBaseModelInfo::Init();
	
//	m_pRelated = NULL;
//	m_relatedModel = -1;
}

//
//        name: CAtomicModelInfo::DeleteRwObject
// description: Remove atomics used
//
void CAtomicModelInfo::DeleteRwObject()
{
	if(m_pRwObject)
	{
		//sort out the 2dfx count
		RpGeometry* pGeom = RpAtomicGetGeometry((RpAtomic*)m_pRwObject);
		ASSERT(pGeom);
		m_num2dEffects -= RpGeometryGetNum2dEffects(pGeom);	
	
		RwFrame* pFrame = RpAtomicGetFrame((RpAtomic*)m_pRwObject);
		RpAtomicDestroy((RpAtomic*)m_pRwObject);
		RwFrameDestroy(pFrame);
		m_pRwObject = NULL;
		// deference objects this modelinfo uses
		RemoveTexDictionaryRef();
		if(GetAnimFileIndex() != -1)
			CAnimManager::RemoveAnimBlockRef(GetAnimFileIndex());
	}
}

//
// name:		GetLodDistance()
// description:	Return the lod distance of an object
/*float CAtomicModelInfo::GetLodDistance(int32 lod)
{
	float dist;
	ASSERT(lod >= 0 && lod < NUM_SIMPLE_LODS); 
	dist = m_lodDistances[lod] * TheCamera.LODDistMultiplier;
	//if(!GetIsBigBuilding())
	//	dist = MIN(dist, MAX_LOD_DISTANCE);

	// Aaron wants to do this thing whereby the LOD moves out as the camera is higher up.
	// This way flying won't look so ugly.

	return dist;	
}*/

//
//        name: CAtomicModelInfo::GetAtomicFromDistance
// description: Return a the lod model from a distance
//          in: dist = distance from model
RpAtomic* CAtomicModelInfo::GetAtomicFromDistance(float dist)
{
	if(IsVisibleFromDistance(dist))
		return (RpAtomic*)m_pRwObject;
	return NULL;
}

//
//        name: CAtomicModelInfo::CreateInstance
// description: Create an instance of this object
//
RwObject* CAtomicModelInfo::CreateInstance(RwMatrix* pMatrix)
{
	if(m_pRwObject == NULL)
		return NULL;

	AddRef();
	
	RpAtomic *pAtomic = RpAtomicClone((RpAtomic*)m_pRwObject);
	RwFrame* pFrame = RwFrameCreate();
	RwMatrixCopy(RwFrameGetMatrix(pFrame), pMatrix);
	RpAtomicSetFrame(pAtomic, pFrame);

	RemoveRef();

	return (RwObject*)pAtomic;
}

//
//        name: CAtomicModelInfo::CreateInstance
// description: Create an instance of this object
//
RwObject* CAtomicModelInfo::CreateInstance()
{
	if(m_pRwObject == NULL)
		return NULL;

	AddRef();
	
	RpAtomic *pAtomic = RpAtomicClone((RpAtomic*)m_pRwObject);
	RwFrame* pFrame = RwFrameCreate();
	RpAtomicSetFrame(pAtomic, pFrame);

	RemoveRef();
	
	return &(pAtomic->object.object);
}

//
// name:		SetupVehicleUpgradeFlags
// description:	Setup flags for vehicle upgrade based on its name
void CAtomicModelInfo::SetupVehicleUpgradeFlags(const char* pName)
{
	// have we been thru this function before
	if(GetIsUpgrade())
		return;
		
	typedef struct {
		char *pPrefix;
		int32 id;
	} UpgradePrefix;
	
	
	const UpgradePrefix pUpgradePrefixes[] = {
		"bnt_", 	VEH_UPGRADE_BONNET,
		"bntl_",	VEH_UPGRADE_BONNET_LEFT,
		"bntr_",	VEH_UPGRADE_BONNET_RIGHT,
		"spl_",		VEH_UPGRADE_SPOILER,
		"wg_l_",	VEH_UPGRADE_WING_LEFT,
		"wg_r_",	VEH_UPGRADE_WING_RIGHT,
		"fbb_",		VEH_UPGRADE_FRONTBULLBAR,
		"bbb_",		VEH_UPGRADE_BACKBULLBAR,
		"lgt_",		VEH_UPGRADE_FRONTLIGHTS,
		"rf_",		VEH_UPGRADE_ROOF,
		"nto_",		VEH_UPGRADE_NITRO,
		"hydralics",	VEH_UPGRADE_HYDRAULICS,
		"stereo",		VEH_UPGRADE_STEREO,
		NULL
	};
	const UpgradePrefix pReplacementPrefixes[] = {
		"chss_", 	CAR_CHASSIS,
		"wheel_",	CAR_WHEEL_RF,
		"exh_",		CAR_EXHAUST,
		"fbmp_",	CAR_BUMPFRONT,
		"rbmp_",	CAR_BUMPREAR,
		"misc_a_",	CAR_MISC_A,
		"misc_b_",	CAR_MISC_B,
		"misc_c_",	CAR_MISC_C,
		NULL
	};
	
	// clear any flags in top half of short
	m_flags &= 0xff;
	
	int32 i=0;
	
	while(pUpgradePrefixes[i].pPrefix != NULL)
	{
		// If model has prefix set flags
		if(!strncmp(pUpgradePrefixes[i].pPrefix, pName, strlen(pUpgradePrefixes[i].pPrefix)))
		{
			m_flags |= ATOMIC_IS_UPGRADE;
			m_flags |= pUpgradePrefixes[i].id << ATOMIC_UPGRADE_ID_SHIFT;
			return;
		}
		i++;
	}

	i=0;
	while(pReplacementPrefixes[i].pPrefix != NULL)
	{
		// If model has prefix set flags
		if(!strncmp(pReplacementPrefixes[i].pPrefix, pName, strlen(pReplacementPrefixes[i].pPrefix)))
		{
			m_flags |= ATOMIC_IS_UPGRADE|ATOMIC_REPLACEMENT_UPGRADE;
			m_flags |= pReplacementPrefixes[i].id << ATOMIC_UPGRADE_ID_SHIFT;
			return;
		}
		i++;
	}
	
	ASSERTOBJ(0, pName, "Unrecognised upgrade name");
}

//
// name:		CAtomicModelInfo::SetupBigBuilding()
// description:	Do all the big building setup
//
/*void CAtomicModelInfo::SetupBigBuilding(int32 minIndex, int32 maxIndex)
{
	CAtomicModelInfo* pOtherModel;
	
	if(m_lodDistance <= ISBIGBUILDINGDIST)
		return;
		
	if(GetRelatedModel())
		return;
		
	SetIsBigBuilding(true);
	
	FindRelatedModel(minIndex, maxIndex);
	pOtherModel = GetRelatedModel();
	if(pOtherModel != NULL)
	{
		//SetNearDistance(pOtherModel->GetLodDistance() / TheCamera.LODDistMultiplier);
		// don't have draw last big buildings if they have a related high LOD
		if(GetDrawLast())
		{
			SetDrawLast(false);
			DEBUGLOG1("%s was draw last\n",  GetModelName());
		}
	}
	// this is different from GTA3. Now big buildings that don't have a related hi detail building
	// don't have a near distance
	else				
		SetNearDistance(0.0f);
}*/

//
// name:		CAtomicModelInfo::FindRelatedModel()
// description:	Find the model related to this one. Used specifically for
//				big buildings. So they know which buildings they are replacing
//
/*void CAtomicModelInfo::FindRelatedModel(int32 minIndex, int32 maxIndex)
{
	for(int32 i=minIndex; i<=maxIndex; i++)
	{
		CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(i);
		if(pModelInfo == this)
			continue;
		if(pModelInfo &&
			!CGeneral::faststrcmp(GetModelName()+3, pModelInfo->GetModelName()+3))
		{
			if(pModelInfo->AsAtomicModelInfoPtr())
				m_pRelated = pModelInfo->AsAtomicModelInfoPtr();
//			ASSERT(((CAtomicModelInfo*)pModelInfo)->GetNumberLOD() < 3);
			//m_relatedModel = i;
			break;
		}
	}	
}*/


#pragma mark --- CTimeModelInfo ---

// --- CTimeModelInfo ----------------------------------------------------------

CTimeInfo* CTimeInfo::FindOtherTimeModel(const char* pName)
{
	char otherName[24];
	bool isNightTimeObj = false;
	char* pIdentifier;
	int32 i;
	CBaseModelInfo* pModelInfo;
	CTimeInfo* pTimeInfo;
	
	strcpy(otherName, pName);
	if((pIdentifier = strstr(otherName, "_nt")))
	{
		strncpy(pIdentifier, "_dy", 4);
	}
	else if((pIdentifier = strstr(otherName, "_dy")))
	{
		strncpy(pIdentifier, "_nt", 4);
	}
	else
		return NULL;
	
#ifdef USE_MODELINFO_HASHKEY
	uint32 key = CKeyGen::GetUppercaseKey(otherName);
#endif	
	// Go through list of models and compare name with model names
	for(i=0; i<NUM_MODEL_INFOS; i++)
	{
		pModelInfo = CModelInfo::GetModelInfo(i);
		if(pModelInfo)
		{
			pTimeInfo = pModelInfo->GetTimeInfo();
			if(pTimeInfo)
			{
#ifdef USE_MODELINFO_HASHKEY
				if(pModelInfo->GetHashKey() == key)
#else
				if(!CGeneral::faststrncmp(pModelInfo->GetModelName(), otherName, MODEL_NAME_LEN))
#endif				
				{
					break;
				}
			}	
		}	
	}

	if(i != NUM_MODEL_INFOS)
	{
		ASSERTOBJ(pTimeInfo->m_on == m_off &&
				m_on == pTimeInfo->m_off, pName, 
				"Times do not correspond with the other time model");
		m_otherModel = i;
		return pTimeInfo;
	}
	return NULL;
}


#pragma mark --- CDamageAtomicInfo ---

//
//        name: CAtomicModelInfo::Init
// description: Initialise Simple object values
//
void CDamageAtomicModelInfo::Init()
{
	CAtomicModelInfo::Init();
	
	m_pDamageAtomic = NULL;
}

//
// name:		SetDamageAtomic
// description:	Set atomic for damaged version of object
void CDamageAtomicModelInfo::SetDamagedAtomic(RpAtomic* pAtomic)
{
	m_pDamageAtomic = pAtomic;
	
	{
#if defined (GTA_PC) || defined (GTA_XBOX)
	if(CCustomBuildingRenderer::IsCBPCPipelineAttached(pAtomic))
	{
		// do CVB setup on loaded atomic (usually animated countryside objects):
		CCustomBuildingRenderer::AtomicSetup(pAtomic);
		//ASSERTMSG(FALSE, "CVBuildingPipe: Buildings shouldn't be loaded as clumps! See Andrzej.");
	}
	else if(CCarFXRenderer::IsCCPCPipelineAttached(pAtomic))
	{
		// do CustomCarFX setup on loaded atomic (usually they are car mods):
		//CCarFXRenderer::CustomCarPipeAtomicSetup(pAtomic);
		#ifndef FINAL
			// set name of the model we're going to load (this is for printing debug/assert info purposes only):
			extern char *carfxDbgLoadedModelName;
			carfxDbgLoadedModelName = this->m_modelName;
		#endif
		// extended VehicleModelInfo-type setup on loaded car mod:
		CCarFXRenderer::SetCustomFXAtomicRenderPipelinesVMICB(pAtomic, NULL);
	}
#endif //GTA_PC

#if defined (GTA_PS2)
		if(CVuCustomBuildingRenderer::IsCVBPipelineAttached(pAtomic))
		{
			// do CVB setup on loaded atomic:
			CVuCustomBuildingRenderer::AtomicSetup(pAtomic);
		}
		else if(CVuCarFXRenderer::IsCVCPipelineAttached(pAtomic))
		{
			// do CVC setup on loaded atomic (car mods usually):
			//CVuCarFXRenderer::CustomCVCarPipeAtomicSetup(pAtomic);

			#ifndef FINAL
				extern char *dbgLoadedModelName;
				// set name of the model we're going to load (this is for printing debug/assert info purposes only):
				dbgLoadedModelName = this->m_modelName;
			#endif

			// extended VehicleModelInfo-type setup on loaded car mod:
			CVuCarFXRenderer::SetCustomFXAtomicRenderPipelinesVMICB(pAtomic, NULL);
		}
		else if(CVuCustomSkinPedRenderer::IsCVSPPipelineAttached(pAtomic))
		{
			// do CVSP setup on loaded atomic:
			// CVuCustomSkinPedRenderer::AtomicSetup(pAtomic);
			ASSERTMSG(FALSE, "CVSkinPedPipe: Peds shouldn't be loaded as atomics! See Andrzej.");
		}
		else
		{
			PS2AllSetAtomicPipe(pAtomic);
		}
#endif //GTA_PS2
	}
}

//
//        name: CAtomicModelInfo::CreateInstance
// description: Create an instance of this object
//
RwObject* CDamageAtomicModelInfo::CreateInstance(RwMatrix* pMatrix)
{
	if(m_bCreateDamagedVersion)
	{
		if(m_pDamageAtomic == NULL)
			return NULL;

		RpAtomic *pAtomic = RpAtomicClone(m_pDamageAtomic);
		RwFrame* pFrame = RwFrameCreate();
		RwMatrixCopy(RwFrameGetMatrix(pFrame), pMatrix);
		RpAtomicSetFrame(pAtomic, pFrame);

		return (RwObject*)pAtomic;
	}
	else
		return CAtomicModelInfo::CreateInstance(pMatrix);			
}

//
//        name: CAtomicModelInfo::CreateInstance
// description: Create an instance of this object
//
RwObject* CDamageAtomicModelInfo::CreateInstance()
{
	if(m_bCreateDamagedVersion)
	{
		if(m_pDamageAtomic == NULL)
			return NULL;

		RpAtomic *pAtomic = RpAtomicClone(m_pDamageAtomic);
		RwFrame* pFrame = RwFrameCreate();
		RpAtomicSetFrame(pAtomic, pFrame);

		return &(pAtomic->object.object);
	}
	else
		return CAtomicModelInfo::CreateInstance();
}	

//
//        name: CDamageAtomicModelInfo::DeleteRwObject
// description: Remove atomics used
//
void CDamageAtomicModelInfo::DeleteRwObject()
{
	if(m_pDamageAtomic)
	{
		RwFrame* pFrame = RpAtomicGetFrame(m_pDamageAtomic);
		RpAtomicDestroy(m_pDamageAtomic);
		RwFrameDestroy(pFrame);
		m_pDamageAtomic = NULL;
	}
	CAtomicModelInfo::DeleteRwObject();
}

