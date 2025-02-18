//
//
//    Filename: ModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: 
//
//
// Game headers
#include "ModelInfo.h"
#include "BaseModelInfo.h"
#include "AtomicModelInfo.h"
#include "TimeModelInfo.h"
#include "WeaponModelInfo.h"
#include "VehicleModelInfo.h"
#include "PedModelInfo.h"
#include "2dEffect.h"
#include "FileMgr.h"
#include "Maths.h"
#include "general.h"
#include "ModelInfoAccel.h"

#include "tempColModels.h"
#include "modelindices.h"
#include "World.h"

#include "ModelInfoAccel.h"

// number of simple model types
#define NUM_ATOMIC_MODEL_INFOS	(14000)
// number of simple model types
#define NUM_DAMAGE_MODEL_INFOS	(70)
// number of LOD model types
#define NUM_LOD_MODEL_INFOS		(1)
// number of time object types
#define NUM_TIME_MODEL_INFOS	(169)
// number of time object types
#define NUM_LOD_TIME_MODEL_INFOS	(1)
// number of weapon object types
#define NUM_WEAPON_MODEL_INFOS	(51)
// number of clump object types
#define NUM_CLUMP_MODEL_INFOS	(92)
// number of vehicle object types
#define NUM_VEHICLE_MODEL_INFOS	(212)

// static variables
static CStore<CAtomicModelInfo, NUM_ATOMIC_MODEL_INFOS> ms_atomicModelStore;
static CStore<CDamageAtomicModelInfo, NUM_DAMAGE_MODEL_INFOS> ms_damageAtomicModelStore;
static CStore<CLodAtomicModelInfo, NUM_LOD_MODEL_INFOS> ms_lodModelStore;
static CStore<CTimeModelInfo, NUM_TIME_MODEL_INFOS> ms_timeModelStore;
static CStore<CLodTimeModelInfo, NUM_LOD_TIME_MODEL_INFOS> ms_lodTimeModelStore;
static CStore<CWeaponModelInfo, NUM_WEAPON_MODEL_INFOS> ms_weaponModelStore;
static CStore<CClumpModelInfo, NUM_CLUMP_MODEL_INFOS> ms_clumpModelStore;
static CStore<CVehicleModelInfo, NUM_VEHICLE_MODEL_INFOS> ms_vehicleModelStore;
static CStore<CPedModelInfo, NUM_PED_MODEL_INFOS> ms_pedModelStore;
static CStore<C2dEffect, NUM_2D_EFFECTS> ms_2dEffectStore;
// static member variables
CBaseModelInfo*	CModelInfo::ms_modelInfoPtrs[NUM_MODEL_INFOS];
int32 CModelInfo::ms_lastPositionSearched = 0;

//
//        name: CModelInfoLists::Initialise
// description: Initialise all the model lists
//
void CModelInfo::Initialise()
{
	int32 i;

	DEBUGLOG1("sizeof AtomicModelStore %d\n", sizeof(ms_atomicModelStore));
	DEBUGLOG1("sizeof LodModelStore %d\n", sizeof(ms_lodModelStore));
	DEBUGLOG1("sizeof TimeModelStore %d\n", sizeof(ms_timeModelStore));
	DEBUGLOG1("sizeof LodTimeModelStore %d\n", sizeof(ms_lodTimeModelStore));
	DEBUGLOG1("sizeof WeaponModelStore %d\n", sizeof(ms_weaponModelStore));
	DEBUGLOG1("sizeof ClumpModelStore %d\n", sizeof(ms_clumpModelStore));
	DEBUGLOG1("sizeof VehicleModelStore %d\n", sizeof(ms_vehicleModelStore));
	DEBUGLOG1("sizeof PedModelStore %d\n", sizeof(ms_pedModelStore));
	DEBUGLOG1("sizeof 2deffectsModelStore %d\n", sizeof(ms_2dEffectStore));
	
	for(i=0; i<NUM_MODEL_INFOS; i++)
		ms_modelInfoPtrs[i] = NULL;

	ms_atomicModelStore.Init();
	ms_damageAtomicModelStore.Init();
	ms_lodModelStore.Init();
	//ms_mloModelStore.Init();
	ms_timeModelStore.Init();
	ms_lodTimeModelStore.Init();
	ms_weaponModelStore.Init();
	ms_clumpModelStore.Init();
	ms_vehicleModelStore.Init();
	ms_pedModelStore.Init();
	//ms_xtraCompsModelStore.Init();
	//ms_mloInstanceStore.Init();
	ms_2dEffectStore.Init();
	
	// Add in model data (primarily collision models) for car components
	// to be used to generate component objects falling off cars
	// Also need to set these to be loaded in CStreaming::Init() to stop the game crashing
	CAtomicModelInfo* pModelInfo1 = AddAtomicModel(MODELID_COMPONENT_DOOR);
	pModelInfo1->SetColModel(&CTempColModels::ms_colModelDoor1);
	pModelInfo1->SetTexDictionary("generic");
	pModelInfo1->SetLodDistance(80);
	
	CAtomicModelInfo* pModelInfo2 = AddAtomicModel(MODELID_COMPONENT_BUMPER);
	pModelInfo2->SetColModel(&CTempColModels::ms_colModelBumper1);
	pModelInfo2->SetTexDictionary("generic");
	pModelInfo2->SetLodDistance(80);
	
	CAtomicModelInfo* pModelInfo3 = AddAtomicModel(MODELID_COMPONENT_PANEL);
	pModelInfo3->SetColModel(&CTempColModels::ms_colModelPanel1);
	pModelInfo3->SetTexDictionary("generic");
	pModelInfo3->SetLodDistance(80);
	
	CAtomicModelInfo* pModelInfo4 = AddAtomicModel(MODELID_COMPONENT_BONNET);
	pModelInfo4->SetColModel(&CTempColModels::ms_colModelBonnet1);
	pModelInfo4->SetTexDictionary("generic");
	pModelInfo4->SetLodDistance(80);
	
	CAtomicModelInfo* pModelInfo5 = AddAtomicModel(MODELID_COMPONENT_BOOT);
	pModelInfo5->SetColModel(&CTempColModels::ms_colModelBoot1);
	pModelInfo5->SetTexDictionary("generic");
	pModelInfo5->SetLodDistance(80);
	
	CAtomicModelInfo* pModelInfo6 = AddAtomicModel(MODELID_COMPONENT_WHEEL);
	pModelInfo6->SetColModel(&CTempColModels::ms_colModelWheel1);
	pModelInfo6->SetTexDictionary("generic");
	pModelInfo6->SetLodDistance(80);
	
	CAtomicModelInfo* pModelInfo7 = AddAtomicModel(MODELID_BODYPART_A);
	pModelInfo7->SetColModel(&CTempColModels::ms_colModelBodyPart1);
	pModelInfo7->SetTexDictionary("generic");
	pModelInfo7->SetLodDistance(80);

	CAtomicModelInfo* pModelInfo8 = AddAtomicModel(MODELID_BODYPART_B);
	pModelInfo8->SetColModel(&CTempColModels::ms_colModelBodyPart2);
	pModelInfo8->SetTexDictionary("generic");
	pModelInfo8->SetLodDistance(80);
}

//
// name:		PrintModelInfoStoreUsage
// description:	Print how many of each modelinfo there are
void CModelInfo::PrintModelInfoStoreUsage()
{
	DEBUGLOG1("Atomic ModelInfo %d\n", ms_atomicModelStore.GetItemsUsed());
	DEBUGLOG1("Damage ModelInfo %d\n", ms_damageAtomicModelStore.GetItemsUsed());
	DEBUGLOG1("Lod ModelInfo %d\n", ms_lodModelStore.GetItemsUsed());
	DEBUGLOG1("Time ModelInfo %d\n", ms_timeModelStore.GetItemsUsed());
	DEBUGLOG1("Lod time ModelInfo %d\n", ms_lodTimeModelStore.GetItemsUsed());
	DEBUGLOG1("Weapon ModelInfo %d\n", ms_weaponModelStore.GetItemsUsed());
	DEBUGLOG1("Clump ModelInfo %d\n", ms_clumpModelStore.GetItemsUsed());
	DEBUGLOG1("Vehicle ModelInfo %d\n", ms_vehicleModelStore.GetItemsUsed());
	DEBUGLOG1("Ped ModelInfo %d\n", ms_pedModelStore.GetItemsUsed());
	DEBUGLOG1("2dEffect ModelInfo %d\n", ms_2dEffectStore.GetItemsUsed());
}

void CModelInfo::ReInit2dEffects() 
{
	ms_2dEffectStore.Init();
	for(int32 i=0; i<NUM_MODEL_INFOS; i++)
	{
		if(ms_modelInfoPtrs[i])
			ms_modelInfoPtrs[i]->Init2dEffects();
	}	
}

//
//        name: CModelInfoLists::Shutdown
// description: Remove all allocated memory after game has finished
//
void CModelInfo::ShutDown()
{
	ms_atomicModelStore.ForAllItemsUsed(&CAtomicModelInfo::Shutdown);
	ms_damageAtomicModelStore.ForAllItemsUsed(&CDamageAtomicModelInfo::Shutdown);
	ms_lodModelStore.ForAllItemsUsed(&CLodAtomicModelInfo::Shutdown);
	//ms_mloModelStore.ForAllItemsUsed(&CMloModelInfo::Shutdown);
	ms_timeModelStore.ForAllItemsUsed(&CTimeModelInfo::Shutdown);
	ms_lodTimeModelStore.ForAllItemsUsed(&CLodTimeModelInfo::Shutdown);
	ms_weaponModelStore.ForAllItemsUsed(&CWeaponModelInfo::Shutdown);
	ms_clumpModelStore.ForAllItemsUsed(&CClumpModelInfo::Shutdown);
	ms_vehicleModelStore.ForAllItemsUsed(&CVehicleModelInfo::Shutdown);
	ms_pedModelStore.ForAllItemsUsed(&CPedModelInfo::Shutdown);
	//ms_xtraCompsModelStore.ForAllItemsUsed(&CXtraCompsModelInfo::Shutdown);
	//ms_mloInstanceStore.ForAllItemsUsed(&CInstance::Shutdown);
	ms_2dEffectStore.ForAllItemsUsed(&C2dEffect::Shutdown);
	
	ms_atomicModelStore.SetItemsUsed(0);
	ms_damageAtomicModelStore.SetItemsUsed(0);
	ms_lodModelStore.SetItemsUsed(0);
	//ms_mloModelStore.SetItemsUsed(0);
	ms_timeModelStore.SetItemsUsed(0);
	ms_lodTimeModelStore.SetItemsUsed(0);
	ms_weaponModelStore.SetItemsUsed(0);
	ms_clumpModelStore.SetItemsUsed(0);
	ms_vehicleModelStore.SetItemsUsed(0);
	ms_pedModelStore.SetItemsUsed(0);
	//ms_xtraCompsModelStore.SetItemsUsed(0);
	//ms_mloInstanceStore.SetItemsUsed(0);
	ms_2dEffectStore.SetItemsUsed(0);
}

//
//        name: CFileLoader::GetModelInfo
// description: Get a pointer to a modelInfo class from a name
//
CBaseModelInfo* CModelInfo::GetModelInfo(const char *pName, int32* pIndex)
{
#ifdef USE_MODELINFO_HASHKEY
	uint32 hashKey = CKeyGen::GetUppercaseKey(pName);
#endif
	CBaseModelInfo* pModel;

#define DW_MODELINFO_CACHE_LAST_POSITION_SEARCHED
#ifdef DW_MODELINFO_CACHE_LAST_POSITION_SEARCHED

	// Go through list of models and compare name with model names
	for(int32 i=ms_lastPositionSearched; i<NUM_MODEL_INFOS; i++)
	{
		pModel = ms_modelInfoPtrs[i];
#ifdef USE_MODELINFO_HASHKEY
		if(pModel && pModel->GetHashKey() == hashKey)
#else
		if(pModel && !stricmp(pModel->GetModelName(), pName))
#endif
		{
			if(pIndex)
				*pIndex = i;
				
			ms_lastPositionSearched = i;
			return pModel;
		}
	}


	for(int32 i=ms_lastPositionSearched; i>=0; i--)
	{
		pModel = ms_modelInfoPtrs[i];
#ifdef USE_MODELINFO_HASHKEY
		if(pModel && pModel->GetHashKey() == hashKey)
#else
		if(pModel && !stricmp(pModel->GetModelName(), pName))
#endif
		{
			if(pIndex)
				*pIndex = i;
				
			ms_lastPositionSearched = i;
			return pModel;
		}
	}


#else


	// Go through list of models and compare name with model names
	for(int32 i=0; i<NUM_MODEL_INFOS; i++)
	{
		pModel = ms_modelInfoPtrs[i];
#ifdef USE_MODELINFO_HASHKEY
		if(pModel && pModel->GetHashKey() == hashKey)
#else
		if(pModel && !stricmp(pModel->GetModelName(), pName))
#endif
		{
			if(pIndex)
				*pIndex = i;
			return pModel;
		}
	}
#endif	
	return NULL;
}

//
// name:		GetModelInfoFromHashKey
// description:	Get modelinfo pointer from a hash key identifier
CBaseModelInfo* CModelInfo::GetModelInfoFromHashKey(uint32 key, int32* pIndex)
{
	CBaseModelInfo* pModel;
	// Go through list of models and compare name with model names
	for(int32 i=0; i<NUM_MODEL_INFOS; i++)
	{
		pModel = ms_modelInfoPtrs[i];
		if(pModel && pModel->GetHashKey() == key)
		{
			if(pIndex)
				*pIndex = i;
			return pModel;
		}
	}
	return NULL;
}

//
//        name: CFileLoader::GetModelInfo
// description: Get a pointer to a modelInfo class from a name
// DW - Uint16 version
CBaseModelInfo* CModelInfo::GetModelInfoUInt16(const char *pName, UInt16* pIndex)
{
	int32 val = 0;
	CBaseModelInfo *pData = GetModelInfo(pName,&val);
	if (pIndex)
		*pIndex = val;
	return pData;
}


//
// name:		GetModelInfo
// description:	Get modelinfo from a name and a range of model indices
CBaseModelInfo* CModelInfo::GetModelInfo(const char* pName, int32 start, int32 end)
{
#ifdef USE_MODELINFO_HASHKEY
	uint32 hashKey = CKeyGen::GetUppercaseKey(pName);
#endif
	CBaseModelInfo* pModel;
	// Go through list of models and compare name with model names
	for(int32 i=start; i<=end; i++)
	{
		pModel = ms_modelInfoPtrs[i];
#ifdef USE_MODELINFO_HASHKEY
		if(pModel && pModel->GetHashKey() == hashKey)
#else
		if(pModel && !stricmp(pModel->GetModelName(), pName))
#endif
		{
			return pModel;
		}
	}
	return NULL;
}




#ifdef MODEL_INFO_ACCELERATOR
//
//        name: CFileLoader::GetModelInfoFast
// description: Get a pointer to a modelInfo class from a name
// DEREKS FAST VERSION.
CBaseModelInfo* CModelInfo::GetModelInfoFast(CModelInfoAccelerator *pAccel, const char *pName, int32* pIndex)
{
	uint16 id = pAccel->GetNextModelInfoId();
	
	if(id == INVALID_MODELID)
		return NULL;

	CBaseModelInfo* pModel = ms_modelInfoPtrs[id];

#ifdef USE_MODELINFO_HASHKEY
	if(pModel && pModel->GetHashKey() == CKeyGen::GetUppercaseKey(pName))
#else
	if(pModel && !stricmp(pModel->GetModelName(), pName))
#endif
	{
		if(pIndex)
			*pIndex = id;
		return pModel;
	}
	return NULL;
}
#endif


//
//        name: CModelInfo::AddAtomicModel
// description: Allocate a new model from one of the model stores and give it an index
//          in: index = index model will have in model array
//
CAtomicModelInfo* CModelInfo::AddAtomicModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CAtomicModelInfo *pModel = ms_atomicModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
CDamageAtomicModelInfo* CModelInfo::AddDamageAtomicModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CDamageAtomicModelInfo *pModel = ms_damageAtomicModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
CLodAtomicModelInfo* CModelInfo::AddLodAtomicModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CLodAtomicModelInfo *pModel = ms_lodModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
/*CMloModelInfo* CModelInfo::AddMloModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CMloModelInfo *pModel = ms_mloModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}*/
CTimeModelInfo* CModelInfo::AddTimeModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CTimeModelInfo *pModel = ms_timeModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
CLodTimeModelInfo* CModelInfo::AddLodTimeModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CLodTimeModelInfo *pModel = ms_lodTimeModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
CWeaponModelInfo* CModelInfo::AddWeaponModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CWeaponModelInfo *pModel = ms_weaponModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
CClumpModelInfo* CModelInfo::AddClumpModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CClumpModelInfo *pModel = ms_clumpModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
CVehicleModelInfo* CModelInfo::AddVehicleModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CVehicleModelInfo *pModel = ms_vehicleModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
CPedModelInfo* CModelInfo::AddPedModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CPedModelInfo *pModel = ms_pedModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}
/*CXtraCompsModelInfo* CModelInfo::AddXtraCompsModel(int32 index)
{
	ASSERT(index >= 0 && index < NUM_MODEL_INFOS);

	CXtraCompsModelInfo *pModel = ms_xtraCompsModelStore.GetNextItem();
	pModel->Init();
	SetModelInfo(index, pModel);
	return pModel;
}

CStore<CInstance, NUM_MLO_INSTANCES>& CModelInfo::GetMloInstanceStore() 
{
	return ms_mloInstanceStore;
}*/
CStore<C2dEffect, NUM_2D_EFFECTS>& CModelInfo::Get2dEffectStore() 
{
	return ms_2dEffectStore;
}


//
//        name: CModelInfo::ConstructMloClumps
// description: Construct the RenderWare clumps used to display the MLO objects
//
/*void CModelInfo::ConstructMloClumps()
{
	ms_mloModelStore.ForAllItemsUsed(&CMloModelInfo::ConstructClump);
}*/

//
// name: CModelInfo::IsBoatModel
// description: Returns TRUE if the given model is a boat
//
Bool8 CModelInfo::IsBoatModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_BOAT)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsCarModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_CAR)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsTrainModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_TRAIN)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsHeliModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_HELI)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsPlaneModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_PLANE)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsBikeModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_BIKE)
			return TRUE;
	}
	return FALSE;
}

/*
Bool8 CModelInfo::IsFakeHeliModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_FAKE_HELI)
			return TRUE;
	}
	return FALSE;
}
*/

Bool8 CModelInfo::IsFakePlaneModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_FAKE_PLANE)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsMonsterTruckModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_MONSTERTRUCK)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsQuadBikeModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_QUADBIKE)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsBmxModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_BMX)
			return TRUE;
	}
	return FALSE;
}

Bool8 CModelInfo::IsTrailerModel(int32 index)
{
	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return FALSE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		pVehicleInfo = (CVehicleModelInfo*) GetModelInfo(index);
		if (pVehicleInfo->GetVehicleClass() == VEHICLE_TYPE_TRAILER)
			return TRUE;
	}
	return FALSE;
}


// IsVehicleModelType() gives vehicle type or -1 if not vehicle
int32 CModelInfo::IsVehicleModelType(int32 index)
{
	if (index >= NUM_MODEL_INFOS) return VEHICLE_TYPE_NONE;

	CVehicleModelInfo *pVehicleInfo = 0;
	CBaseModelInfo *pBaseInfo = GetModelInfo(index);
	if(!pBaseInfo)
	    return VEHICLE_TYPE_NONE;
	
	if (pBaseInfo->GetModelType() == MI_TYPE_VEHICLE)
	{
		return ((CVehicleModelInfo *)pBaseInfo)->GetVehicleClass();
	}
	
	return VEHICLE_TYPE_NONE;
}



