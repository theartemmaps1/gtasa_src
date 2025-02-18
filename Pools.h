// Title	:	Pools.h
// Author	:	Richard Jobling
// Started	:	04/12/98

#ifndef _POOLS_H_
#define _POOLS_H_


#include "Pool.h"
#include "PtrList.h"
#include "EntryInfoNode.h"
#include "heli.h"
#include "CutsceneObject.h"
#include "Dummy.h"
#include "PlayerPed.h"
#include "Building.h"
#include "Route.h"
#include "TaskAllocator.h"
#include "timecycle.h"
#include "pedintelligence.h"
#include "camera.h"

#ifdef NEW_CAMERA
#include "cameramain.h"
#include "cam.h"
#endif


// Moved most of the Pools sizes to CPP file because it is a pain having to recompile 
// everything just because I changed a pool size
#ifdef GTA_PC
#define POOLS_MAXNOOFPEDS 			140		// Need larger pools on the PC to allow players to increase ped & car density
#define POOLS_MAXNOOFVEHICLES 		110
#else
#define POOLS_MAXNOOFPEDS 			74		
#define POOLS_MAXNOOFVEHICLES 		70
#endif


#define LARGESTPEDCLASS CCopPed	//CPlayerPed
#define LARGESTOBJECTCLASS CCutsceneObject
#define LARGESTVEHICLECLASS CHeli
#define LARGESTTASKALLOCATOR CTaskAllocatorPlayerCommandAttack

#ifdef NEW_CAMERA
typedef CPool<class CCamNew, char[MAX_CAM_SIZE]> CCamPool;
#endif

typedef CPool<class CPtrNodeSingleLink> CPtrNodeSingleLinkPool;
typedef CPool<class CPtrNodeDoubleLink> CPtrNodeDoubleLinkPool;
typedef CPool<class CEntryInfoNode> CEntryInfoNodePool;
typedef CPool<class CPed, LARGESTPEDCLASS> CPedPool;
typedef CPool<class CVehicle, LARGESTVEHICLECLASS> CVehiclePool;
typedef CPool<class CBuilding> CBuildingPool;
//typedef CPool<class CMultiBuilding> CMultiBuildingPool;
typedef CPool<class CObject, LARGESTOBJECTCLASS> CObjectPool;
typedef CPool<class CDummy> CDummyPool;

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
typedef CPool<class cAudioScriptObject> CAudioScriptObjectPool;
#endif //USE_AUDIOENGINE

// NB : define 'MAX_TASK_SIZE_CALCULATOR' in "TaskTypes.h" to find maximum task size
#ifdef FINAL
#define MAX_TASK_SIZE	128		// CTaskSimpleSlideToCoord
#else
#define MAX_TASK_SIZE	132		// CTaskSimpleSlideToCoord
#endif

// NB : define 'MAX_EVENT_SIZE_CALCULATOR' in "EventCounter.h" to find maximum event size
#ifdef FINAL
#define MAX_EVENT_SIZE	68		// CEventDamage
#else
#define MAX_EVENT_SIZE	72		// CEventDamage
#endif

#define MAX_ATTRACTOR_SIZE 196

typedef CPool<class CColModel> CColModelPool;
typedef CPool<class CTask, char[MAX_TASK_SIZE]> CTaskPool;	// NB : was 152
typedef CPool<class CEvent, char[MAX_EVENT_SIZE]> CEventPool;	// NB : was 76
typedef CPool<class CPointRoute> CPointRoutePool;	
typedef CPool<class CPatrolRoute> CPatrolRoutePool;	
typedef CPool<class CNodeRoute> CNodeRoutePool;
typedef CPool<class CTaskAllocator, LARGESTTASKALLOCATOR> CTaskAllocatorPool;
typedef CPool<class CPedIntelligence> CPedIntelligencePool;
typedef CPool<class CPedAttractor, char[MAX_ATTRACTOR_SIZE]> CPedAttractorPool;

#ifdef NEW_CAMERA
typedef CPool<class CCamNew, char[MAX_CAM_SIZE]> CCamPool;
#endif

class CPools
{
public:
	static CPtrNodeSingleLinkPool* ms_pPtrNodeSingleLinkPool;
	static CPtrNodeDoubleLinkPool* ms_pPtrNodeDoubleLinkPool;
	static CEntryInfoNodePool* ms_pEntryInfoNodePool;
	static CPedPool* ms_pPedPool;
	static CVehiclePool* ms_pVehiclePool;
	static CBuildingPool* ms_pBuildingPool;
//	static CMultiBuildingPool* ms_pMultiBuildingPool;
	static CObjectPool* ms_pObjectPool;
	static CDummyPool* ms_pDummyPool;

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	static CAudioScriptObjectPool* ms_pAudioScriptObjectPool;
#endif //USE_AUDIOENGINE	

	static CColModelPool* ms_pColModelPool;
	static CTaskPool* ms_pTaskPool;
	static CEventPool* ms_pEventPool;
	static CPointRoutePool* ms_pPointRoutePool;
	static CPatrolRoutePool* ms_pPatrolRoutePool;
	static CNodeRoutePool* ms_pNodeRoutePool;
	static CTaskAllocatorPool* ms_pTaskAllocatorPool;
	static CPedIntelligencePool* ms_pPedIntelligencePool;
	static CPedAttractorPool* ms_pPedAttractorPool;
#ifdef NEW_CAMERA
	static CCamPool* ms_pCamPool;
#endif

	static bool Save() 
	{ 
		if (!SavePedPool()) 
			return false; 
//		if (!SaveVehiclePool()) 
//			return false;
		return SaveObjectPool();
	}
	
	static bool Load() 
	{ 
		if (!LoadPedPool()) 
			return false; 
//		if (!LoadVehiclePool()) 
//			return false;
		return LoadObjectPool();
	}
	
	static bool SavePedPool();
	static bool LoadPedPool();
	static bool SaveVehiclePool();
	static bool LoadVehiclePool();
	static bool SaveObjectPool();
	static bool LoadObjectPool();
	
	static void MakeSureSlotInObjectPoolIsEmpty(Int32 Slot);

public:
	static Int32 CheckBuildingAtomics();
	static void Initialise();
	static void ShutDown();
	static void CheckPoolsEmpty();
	static CPtrNodeSingleLinkPool& GetPtrNodeSingleLinkPool() { return *ms_pPtrNodeSingleLinkPool; }
	static CPtrNodeDoubleLinkPool& GetPtrNodeDoubleLinkPool() { return *ms_pPtrNodeDoubleLinkPool; }
	static CEntryInfoNodePool& GetEntryInfoNodePool() { return *ms_pEntryInfoNodePool; }
	static CPedPool& GetPedPool() { return *ms_pPedPool; }
	static CVehiclePool& GetVehiclePool() { return *ms_pVehiclePool; }
	static CBuildingPool& GetBuildingPool() { return *ms_pBuildingPool; }
//	static CMultiBuildingPool& GetMultiBuildingPool() { return *ms_pMultiBuildingPool; }
	static CObjectPool& GetObjectPool() { return *ms_pObjectPool; }
	static CDummyPool& GetDummyPool() { return *ms_pDummyPool; }

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	static CAudioScriptObjectPool& GetAudioScriptObjectPool() { return *ms_pAudioScriptObjectPool; }
#endif //USE_AUDIOENGINE	

	static CColModelPool& GetColModelPool() { return *ms_pColModelPool; }
	static CTaskPool& GetTaskPool() { return *ms_pTaskPool; }
	static CEventPool& GetEventPool() { return *ms_pEventPool; }
	static CPointRoutePool& GetPointRoutePool() { return *ms_pPointRoutePool; }
	static CPatrolRoutePool& GetPatrolRoutePool() { return *ms_pPatrolRoutePool; }
	static CNodeRoutePool& GetNodeRoutePool() { return *ms_pNodeRoutePool; }
	static CTaskAllocatorPool& GetTaskAllocatorPool() { return *ms_pTaskAllocatorPool; }
	static CPedIntelligencePool& GetPedIntelligencePool() { return *ms_pPedIntelligencePool; }
	static CPedAttractorPool& GetPedAttractorPool() { return *ms_pPedAttractorPool; }
#ifdef NEW_CAMERA
	static CCamPool& GetCamPool() { return *ms_pCamPool; }
#endif

	// use these to get unique references to entities in the game
	static int32 GetPedRef(class CPed* pPed);
	static class CPed* GetPed(int32 nRef);
	static int32 GetVehicleRef(class CVehicle* pVehicle);
	static class CVehicle* GetVehicle(int32 nRef);
	static int32 GetObjectRef(class CObject* pObject);
	static class CObject* GetObject(int32 nRef);
//	static int32 GetMultiBuildingRef(class CMultiBuilding* pBuilding);
//	static class CMultiBuilding* GetMultiBuilding(int32 nRef);
};



#endif