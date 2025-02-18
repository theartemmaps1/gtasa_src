//
//
//    Filename: ModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class containing all the lists of models
//
//
#ifndef INC_MODELINFO_H_
#define INC_MODELINFO_H_

// Game headers
#include "BaseModelInfo.h"
//#include "Instance.h"
#include "Debug.h"
#include "Store.h"

// number of model info pointers in array
#define NUM_MODEL_INFOS			(20000)

// number of pedestrian object types
#define NUM_PED_MODEL_INFOS		(278)
// number of 2d effects
#define NUM_2D_EFFECTS			(100)

class CBaseModelInfo;
class CAtomicModelInfo;
class CDamageAtomicModelInfo;
class CLodAtomicModelInfo;
class CTimeModelInfo;
class CLodTimeModelInfo;
class CWeaponModelInfo;
class CClumpModelInfo;
class CVehicleModelInfo;
class CPedModelInfo;
class C2dEffect;

//
//   Class Name: CModelInfo
//  Description: Class holding all the lists of model types
//
class CModelInfo
{
public:
	static void Initialise();
	static void ReInit2dEffects();
	static void ShutDown();

	// return a model info pointer
	static inline CBaseModelInfo* GetModelInfo(int32 index);
	// this function is very slow should not be used in time critical code
	static CBaseModelInfo* GetModelInfoUInt16(const char* pName, UInt16* pIndex=NULL);
	static CBaseModelInfo* GetModelInfo(const char* pName, int32* pIndex=NULL);
	static CBaseModelInfo* GetModelInfo(const char* pName, int32 start, int32 end);
	static CBaseModelInfo* GetModelInfoFromHashKey(uint32 key, int32* pIndex=NULL);
	static CBaseModelInfo* GetModelInfoFast(class CModelInfoAccelerator *pAccel, const char *pName, int32* pIndex);
	static void ResetModelInfoSearch() {ms_lastPositionSearched = 0;}
	
	static CAtomicModelInfo* AddAtomicModel(int32 index);
	static CDamageAtomicModelInfo* AddDamageAtomicModel(int32 index);
	static CLodAtomicModelInfo* AddLodAtomicModel(int32 index);
	//static CMloModelInfo* AddMloModel(int32 index);
	static CTimeModelInfo* AddTimeModel(int32 index);
	static CLodTimeModelInfo* AddLodTimeModel(int32 index);
	static CWeaponModelInfo* AddWeaponModel(int32 index);
	static CClumpModelInfo* AddClumpModel(int32 index);
	static CVehicleModelInfo* AddVehicleModel(int32 index);
	static CPedModelInfo* AddPedModel(int32 index);

	static void PrintModelInfoStoreUsage();

	// Construct MLO RenderWare Clumps from instance info
	static void ConstructMloClumps();

	// access to MLO instance store
	//static CStore<CInstance, NUM_MLO_INSTANCES>& GetMloInstanceStore();
	static CStore<C2dEffect, NUM_2D_EFFECTS>& Get2dEffectStore();

	// access functions
	static inline CColModel& GetColModel(int32 i) {return CModelInfo::GetModelInfo(i)->GetColModel();}
	static CBoundingBox& GetBoundingBox(int32 i) {return CModelInfo::GetModelInfo(i)->GetBoundingBox();}

	static Bool8 IsCarModel(int32 index);
	static Bool8 IsHeliModel(int32 index);
	static Bool8 IsPlaneModel(int32 index);
	static Bool8 IsBoatModel(int32 index);
	static Bool8 IsTrainModel(int32 index);
	static Bool8 IsBikeModel(int32 index);
//	static Bool8 IsFakeHeliModel(int32 index);
	static Bool8 IsFakePlaneModel(int32 index);
	static Bool8 IsMonsterTruckModel(int32 index);
	static Bool8 IsQuadBikeModel(int32 index);
	static Bool8 IsBmxModel(int32 index);
	static Bool8 IsTrailerModel(int32 index);

	// this returns vehicle type or -1 if model is not a vehicle
	static int32 IsVehicleModelType(int32 index);

private:
	static inline void SetModelInfo(int32 index, CBaseModelInfo* pModelInfo) { ASSERT(ms_modelInfoPtrs[index]==NULL); ms_modelInfoPtrs[index] = pModelInfo; }
	static inline void RemoveModelInfo(int32 index) {ms_modelInfoPtrs[index] = NULL;}

	static CBaseModelInfo*	ms_modelInfoPtrs[NUM_MODEL_INFOS];
	static int32 ms_lastPositionSearched;
};


inline CBaseModelInfo* CModelInfo::GetModelInfo(int32 index) 
{ 	
	ASSERTMSG(index >= 0 && index < NUM_MODEL_INFOS, "modelinfo doesnt exist");
	return ms_modelInfoPtrs[index]; 
}

#endif // INC_MODELINFO_H_