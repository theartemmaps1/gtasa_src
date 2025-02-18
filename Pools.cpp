// Title	:	Pools.cpp
// Author	:	Richard Jobling
// Started	:	04/12/98

#include "Pools.h"
#include "PtrList.h"
#include "EntryInfoNode.h"
#include "MemInfo.h"
#include "playerped.h"
#include "general.h"
#include "PlayerPed.h"
#include "Streaming.h"
#include "ProjectileInfo.h"
#include "Task.h"
#include "TaskAllocator.h"
#ifdef NEW_CAMERA
#include "cam.h"
#endif

//
// Do not change the size of any of the following Pools unless you have 
// spoken to Adam
//
#define POOLS_MAXNOOFPTRNODES_SINGLELINK 	70000 // was 55000 but needed to increase - neilf.
#define POOLS_MAXNOOFPTRNODES_DOUBLELINK	3200	// was 2800 for ps2 but got assert
#define POOLS_MAXNOOFENTRYINFONODES	500
#define POOLS_MAXNOOFBUILDINGS 		13000	// was 12000 (for ps2? - had to put up for pc anyway)
//#define POOLS_MAXNOOFMULTIBUILDINGS 1
#define POOLS_MAXNOOFOBJECTS		350
#define POOLS_MAXNOOFDUMMYS			2500	// was 2300 for ps2 but got assert
#define POOLS_MAXNOOFCOLMODELS		(10150)//was 10200


#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
#define POOLS_MAXNOOFAUDIOSCRIPTOBJECTS	(192)
#endif //USE_AUDIOENGINE

#define POOLS_MAXNOOFTASKS			(500)	//was 450 for ps2 but got assert
#define POOLS_MAXNOOFEVENTS			(200)
#define POOLS_MAXNOOFPOINTROUTES	(64)
#define POOLS_MAXNOOFPATROLROUTES	(32)
#define POOLS_MAXNOOFNODEROUTES		(64)
#define POOLS_MAXNOOFTASKALLOCATORS	(16)

#ifdef NEW_CAMERA
#define POOLS_MAXNOOFCAMS			(MAX_CAMERA_POOL_SIZE) // just a guess for now...  DW
#endif

CPtrNodeSingleLinkPool* CPools::ms_pPtrNodeSingleLinkPool = NULL;
CPtrNodeDoubleLinkPool* CPools::ms_pPtrNodeDoubleLinkPool = NULL;
CEntryInfoNodePool* CPools::ms_pEntryInfoNodePool = NULL;
CPedPool* CPools::ms_pPedPool = NULL;
CVehiclePool* CPools::ms_pVehiclePool = NULL;
CBuildingPool* CPools::ms_pBuildingPool = NULL;
//CMultiBuildingPool* CPools::ms_pMultiBuildingPool = NULL;
CObjectPool* CPools::ms_pObjectPool = NULL;
CDummyPool* CPools::ms_pDummyPool = NULL;

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
CAudioScriptObjectPool* CPools::ms_pAudioScriptObjectPool = NULL;
#endif //USE_AUDIOENGINE

CColModelPool* CPools::ms_pColModelPool = NULL;
CTaskPool* CPools::ms_pTaskPool = NULL;
CEventPool* CPools::ms_pEventPool = NULL;
CPointRoutePool* CPools::ms_pPointRoutePool=NULL;
CPatrolRoutePool* CPools::ms_pPatrolRoutePool=NULL;
CNodeRoutePool* CPools::ms_pNodeRoutePool=NULL;
CTaskAllocatorPool* CPools::ms_pTaskAllocatorPool=NULL;
CPedIntelligencePool* CPools::ms_pPedIntelligencePool=NULL;
CPedAttractorPool* CPools::ms_pPedAttractorPool=NULL;

#ifdef NEW_CAMERA
CCamPool* CPools::ms_pCamPool = NULL;
#endif

#pragma dont_inline on
#pragma optimize_for_size on

void CPools::Initialise()
{
	CMemInfo::EnterBlock(POOLS_MEM_ID);
	
	ms_pPtrNodeSingleLinkPool = new CPtrNodeSingleLinkPool(POOLS_MAXNOOFPTRNODES_SINGLELINK, "PtrNode Single");
	ms_pPtrNodeDoubleLinkPool = new CPtrNodeDoubleLinkPool(POOLS_MAXNOOFPTRNODES_DOUBLELINK, "PtrNode Double");
	ms_pEntryInfoNodePool = new CEntryInfoNodePool(POOLS_MAXNOOFENTRYINFONODES, "EntryInfoNode");
	ms_pPedPool = new CPedPool(POOLS_MAXNOOFPEDS, "Peds");
	ms_pVehiclePool = new CVehiclePool(POOLS_MAXNOOFVEHICLES, "Vehicles");
	ms_pBuildingPool = new CBuildingPool(POOLS_MAXNOOFBUILDINGS, "Buildings");
//	ms_pMultiBuildingPool = new CMultiBuildingPool(POOLS_MAXNOOFMULTIBUILDINGS, "Buildings");
	ms_pObjectPool = new CObjectPool(POOLS_MAXNOOFOBJECTS, "Objects");
	ms_pDummyPool = new CDummyPool(POOLS_MAXNOOFDUMMYS, "Dummys");

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	ms_pAudioScriptObjectPool = new CAudioScriptObjectPool(POOLS_MAXNOOFAUDIOSCRIPTOBJECTS, "AudioScriptObj");
#endif //USE_AUDIOENGINE

	ms_pColModelPool = new CColModelPool(POOLS_MAXNOOFCOLMODELS, "ColModel");
	ms_pTaskPool = new CTaskPool(POOLS_MAXNOOFTASKS, "Task");
	ms_pEventPool = new CEventPool(POOLS_MAXNOOFEVENTS, "Event");
	ms_pPointRoutePool = new CPointRoutePool(POOLS_MAXNOOFPOINTROUTES, "PointRoute");
	ms_pPatrolRoutePool = new CPatrolRoutePool(POOLS_MAXNOOFPATROLROUTES, "PatrolRoute");
	ms_pNodeRoutePool = new CNodeRoutePool(POOLS_MAXNOOFNODEROUTES, "NodeRoute");
	ms_pTaskAllocatorPool = new CTaskAllocatorPool(POOLS_MAXNOOFTASKALLOCATORS, "TaskAllocator");
	ms_pPedIntelligencePool = new CPedIntelligencePool(POOLS_MAXNOOFPEDS, "PedIntelligence");
	ms_pPedAttractorPool = new CPedAttractorPool(NUM_PED_ATTRACTORS, "PedAttractors");

#ifdef NEW_CAMERA
	ms_pCamPool = new CCamPool(POOLS_MAXNOOFCAMS, "Cam");
#endif

//printf("pool sizeofs %d %d %d %d %d %d %d %d %d\n",
//		sizeof(CPtrNode), sizeof(CEntryInfoNode), sizeof(CPed), sizeof(CVehicle),
//		sizeof(CBuilding), sizeof(CTreadable), sizeof(CObject), sizeof(CDummy),
//		sizeof(cAudioScriptObject) );
		

	CMemInfo::ExitBlock();
}

void CPools::ShutDown()
{
	int32 spaces = ms_pPtrNodeSingleLinkPool->GetNoOfUsedSpaces();
	DEBUGLOG1("PtrNodes SingleLink left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pPtrNodeDoubleLinkPool->GetNoOfUsedSpaces();
	DEBUGLOG1("PtrNodes DoubleLink left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pEntryInfoNodePool->GetNoOfUsedSpaces();
	DEBUGLOG1("EntryInfoNodes left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pPedPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Peds left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pVehiclePool->GetNoOfUsedSpaces();
	DEBUGLOG1("Vehicles left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pBuildingPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Buildings left %d\n", spaces);
//	ASSERT(spaces == 0);
//	spaces = ms_pMultiBuildingPool->GetNoOfUsedSpaces();
//	DEBUGLOG1("Multi Buildings left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pObjectPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Objects left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pDummyPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Dummys left %d\n", spaces);
//	ASSERT(spaces == 0);

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	spaces = ms_pAudioScriptObjectPool->GetNoOfUsedSpaces();
	DEBUGLOG1("AudioScriptObjects left %d\n", spaces);
//	ASSERT(spaces == 0);
#endif //USE_AUDIOENGINE

	spaces = ms_pColModelPool->GetNoOfUsedSpaces();
	DEBUGLOG1("ColModels left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pTaskPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Tasks left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pEventPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Events left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pPointRoutePool->GetNoOfUsedSpaces();
	DEBUGLOG1("Points left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pPatrolRoutePool->GetNoOfUsedSpaces();
	DEBUGLOG1("Patrols left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pNodeRoutePool->GetNoOfUsedSpaces();
	DEBUGLOG1("Nodes left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pTaskAllocatorPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Nodes left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pPedIntelligencePool->GetNoOfUsedSpaces();
	DEBUGLOG1("PedIntelligences left %d\n", spaces);
//	ASSERT(spaces == 0);
	spaces = ms_pPedAttractorPool->GetNoOfUsedSpaces();
	DEBUGLOG1("PedAttractors left %d\n", spaces);
//	ASSERT(spaces == 0);
#ifdef NEW_CAMERA
	spaces = ms_pCamPool->GetNoOfUsedSpaces();
	DEBUGLOG1("Cams left %d\n", spaces);
#endif
//	ASSERT(spaces == 0);


printf("Shutdown pool started\n");
	delete ms_pPtrNodeSingleLinkPool;
	delete ms_pPtrNodeDoubleLinkPool;
	delete ms_pEntryInfoNodePool;
	delete ms_pPedPool;
	delete ms_pVehiclePool;
	delete ms_pBuildingPool;
//	delete ms_pMultiBuildingPool;
	delete ms_pObjectPool;
	delete ms_pDummyPool;

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	delete ms_pAudioScriptObjectPool;
#endif //USE_AUDIOENGINE

	delete ms_pColModelPool;
	delete ms_pTaskPool;
	delete ms_pEventPool;
	delete ms_pPointRoutePool;
	delete ms_pPatrolRoutePool;
	delete ms_pNodeRoutePool;
	delete ms_pTaskAllocatorPool;
	delete ms_pPedIntelligencePool;
	delete ms_pPedAttractorPool;

#ifdef NEW_CAMERA
	delete ms_pCamPool;
#endif

printf("Shutdown pool done\n");
}

/*
FunctionName: CheckPoolsEmpty
What it does: duh
Paramters: Nothing  
Returns: Nothing 
*/
void CPools::CheckPoolsEmpty()
{
	/*
	int32 spaces = ms_pPtrNodePool->GetNoOfUsedSpaces();
	
	int32 SpacesUsedByBuildingsAndRoads=0;
	
	for (int32 i = 0; i < WORLD_WIDTHINSECTORS * WORLD_DEPTHINSECTORS; i++)
	{
		CSector& sector = ms_aSectors[i];

		pNode = sector.GetBuildingPtrList().GetHeadPtr();
		while (pNode != NULL)
		{
			SpacesUsedByBuildingsAndRoads++;
			pNode = pNode->GetNextPtr();
		}
		
		pNode = sector.GetOverlapBuildingPtrList().GetHeadPtr();
		while (pNode != NULL)
		{
			SpacesUsedByBuildingsAndRoads++;
			pNode = pNode->GetNextPtr();
		}
	}
	
	ASSERT(spaces == SpacesUsedByBuildingsAndRoads);
	spaces = ms_pEntryInfoNodePool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	*/
	int32	spaces = ms_pPedPool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	spaces = ms_pVehiclePool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	spaces = ms_pPedIntelligencePool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	spaces = ms_pPedAttractorPool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	/*
	spaces = ms_pBuildingPool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	spaces = ms_pTreadablePool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
    
	spaces = ms_pObjectPool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	/*
	spaces = ms_pDummyPool->GetNoOfUsedSpaces();
	ASSERT(spaces == 0);
	*/	
#ifdef GTA_PS2		// as this moment, no audio implement for PC yet

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	spaces = ms_pAudioScriptObjectPool->GetNoOfUsedSpaces();
#endif //USE_AUDIOENGINE

#endif	
	ASSERT(spaces == 0);
	
	spaces = ms_pTaskPool->GetNoOfUsedSpaces();

#ifdef NEW_CAMERA // what does this code do?? - DW
	spaces = ms_pCamPool->GetNoOfUsedSpaces();
#endif

	UInt8 stop_var = 1;
	
	if (spaces > 0)
	{
/*
		int32 i=ms_pTaskPool->GetSize();
		while(i--)
		{
			CTask* pTask = ms_pTaskPool->GetSlot(i);
			if(pTask)
			{
				int iTaskType=pTask->GetTaskType();
			}			
		}
*/
		stop_var = 2;
	}
	
	spaces = ms_pEventPool->GetNoOfUsedSpaces();

	if (spaces > 0)
	{
		stop_var = 3;
	}

		// Make sure there are no game_objects left	
	CObjectPool& pool = CPools::GetObjectPool();
	CObject* pObject;
	int32 i=pool.GetSize();
			
	while(i--)
	{
		pObject = pool.GetSlot(i);
		if(pObject && pObject->ObjectCreatedBy == GAME_OBJECT)
		{
			printf("Offending object: MI:%d Coors:%f %f %f\n", pObject->GetModelIndex(), pObject->GetPosition().x, pObject->GetPosition().y, pObject->GetPosition().z);
			ASSERTMSG(0, "Game object left in object pool\n");
		}
	}
	
	
printf("pools have been cleared \n");
}

int32 CPools::GetPedRef(class CPed* pPed)
{
	return ms_pPedPool->GetIndex(pPed);
}

class CPed* CPools::GetPed(int32 nRef)
{
	return (CPed*) ms_pPedPool->GetAt(nRef);
}

int32 CPools::GetVehicleRef(class CVehicle* pVehicle)
{
	return ms_pVehiclePool->GetIndex(pVehicle);
}

class CVehicle* CPools::GetVehicle(int32 nRef)
{
	return (CVehicle*) ms_pVehiclePool->GetAt(nRef);
}

int32 CPools::GetObjectRef(class CObject* pObject)
{
	return ms_pObjectPool->GetIndex(pObject);
}

class CObject* CPools::GetObject(int32 nRef)
{
	return (CObject*) ms_pObjectPool->GetAt(nRef);
}

/*int32 CPools::GetMultiBuildingRef(class CMultiBuilding* pBuilding)
{
	return ms_pMultiBuildingPool->GetIndex(pBuilding);
}

class CMultiBuilding* CPools::GetMultiBuilding(int32 nRef)
{
	return (CMultiBuilding*) ms_pMultiBuildingPool->GetAt(nRef);
}*/


// NAME: MakeSureSlotInObjectPoolIsEmpty
// FUNCTION: If this slot (just slot not including the 8 bit of poolinfo)
//           is not empty it will be emptied by moving the contents into
//           another slot.

void CPools::MakeSureSlotInObjectPoolIsEmpty(Int32 Slot)
{
	if (!CPools::GetObjectPool().GetIsFree(Slot))
	{		// This slot isn't free. We have to move the contents to a different slot first.
		CObject *pOldObject = CPools::GetObjectPool().GetSlot(Slot);

		ASSERT(pOldObject);

			// Temp objects can simply be removed.
		if (pOldObject->ObjectCreatedBy == TEMP_OBJECT)
		{
			CWorld::Remove(pOldObject);
			delete pOldObject;
			return;				// Slot is free; our work is done
		}


			// There are some objects that shouldn't be moved (something has gone wrong)
		ASSERT(!pOldObject->m_nObjectFlags.bIsPickUp);

		// If it so happens this is a projectile we simply get rid of it (should be pretty rare)
		if (!CProjectileInfo::RemoveIfThisIsAProjectile(pOldObject))
		{
			CObject *pNewObject = new CObject(pOldObject->GetModelIndex(), false);
				
			ASSERT(pNewObject);
				
			CWorld::Remove(pOldObject);
				
				// Copy old object into new object slot.
			memcpy(pNewObject, pOldObject, CPools::GetObjectPool().GetStorageSize());
				
			CWorld::Add(pNewObject);
			pOldObject->m_pRwObject = NULL;		// Don't want the rw object to be deleted. (it is now used by pNewObject).
			delete pOldObject;
			pNewObject->pReferences = NULL;	// These have all been removed with the 'delete pOldObject' above
		}
	}
}

//DEBUG STUFF
Int32 CPools::CheckBuildingAtomics(void)
{
	// remove bigbuildings from above the club
	CBuildingPool& pool = CPools::GetBuildingPool();
	CEntity* pEntity;

	UInt32 i=pool.GetSize();
	while(i--)
	{
		pEntity = pool.GetSlot(i);
		if(pEntity && pEntity->GetRwObject())
		{

			// should never have an atomic ptr with no geometry...
			if((RwObjectGetType(pEntity->m_pRwObject) == rpATOMIC))
			{
				RpAtomic* pAtomic = (RpAtomic*)(pEntity->m_pRwObject);
				ASSERT(RpAtomicGetGeometry(pAtomic));
			}
		}
	}
	return(-1);
}



