// Title	:	Automobile.cpp
// Author	:	Richard Jobling/William Henderson
// Started	:	25/08/99
//
//
//
//
//
//
#define CANT_DEAL_WITH_INLINED_ASM

// Game headers
#include "Automobile.h"
#include "Collision.h"
#include "Profile.h"
#include "General.h"
#include "modelinfo.h"
#include "TempColModels.h"
#include "HandlingMgr.h" 
#include "carctrl.h"
#include "carai.h"
#include "Floater.h"
#include "globals.h"
#include "Pools.h"
#include "shadows.h"
#include "explosion.h"
#include "pointlights.h"
#include "SurfaceTable.h"
#include "timer.h"
#include "camera.h"
#include "coronas.h"
#include "weather.h"
#include "skidmarks.h"
#include "modelIndices.h"
#include "playerped.h"
#include "stats.h"
#include "component.h"
#include "localisation.h"
#include "VehicleModelInfo.h"
#include "MonsterTruck.h"
#include "Planes.h"
#include "Bike.h"

#include "Pad.h"	// BILL - TEST: when start pressed display debug text
#include "renderer.h" // for getting camera position, for LOD temp frig
#include "world.h"
#include "particle.h"
#include "clock.h"
#include "population.h"
#include "darkel.h"
#include "garages.h"
#include "rubbish.h"
#include "remote.h"
#include "watercannon.h"
#include "pickups.h"
#include "VisibilityPlugins.h"
#include "game.h"
#include "handy.h"
#include "specialfx.h"
#include "antennas.h"
#include "record.h"
#include "zones.h"
#include "general.h"	//GetRandomNumberInRange()
#include "AnimManager.h"
#include "Glass.h"
#include "replay.h"
#include "cheat.h"

#include "ColTriangle.h"
#include "TimeCycle.h"		//CTimeCycle
#include "ParticleObject.h"	//CParticleObject::AddObject()
#include "WaterLevel.h"		//CWaterLevel::GetWaterLevel()...
#include "ZoneCull.h"		//CCullZone::PlayerNoRain(), CCullZone::CamNoRain()...
#ifdef GTA_NETWORK
#include "gamenet.h"
#include "netgames.h"
#endif
#ifndef MASTER	
#include "carcols.h"
#endif	

#include "PedIntelligence.h"
#include "Events.h"
#include "fx.h"
#include "MemoryMgr.h"
#include "gamelogic.h"
#include "streaming.h"
#include "PedDebugVisualiser.h"

#ifdef USE_AUDIOENGINE
#include "AudioEngine.h"
#include "GlobalSpeechContexts.h"
#include "gameaudioevents.h"
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

#include "TouchInterface.h"


//!PC - no custom renderer for PC - yet
#if defined (GTA_PS2)
#include "VuCarFXRenderer.h"		// SetCustomCVCarMatPipeForClump()...
#endif // GTA_PS2

#define CAR_COMPONENT_REMOVE_LIFESPAN 	(20000)

#ifndef FINAL // just incase I forget to comment this out!
//	#pragma optimization_level 0
//	#pragma dont_inline on
//	#pragma auto_inline off
#endif

//#define IGNORE_HIERARCHY

const float FIRST_LOD_DISTANCE = 60.0f;
const float SECOND_LOD_DISTANCE = 150.0f;
const int COPY_SIZE = sizeof(float)*3.0f;
const float BRAKE_TO_REVERSE_DURATION = 20.0f; // less than half a second
const float DAMAGED_DOOR_MAX_ANGLE = 0.15;
const float DAMAGED_PANEL_MAX_ANGLE = 0.1;

// BILL
extern CAutomobile* gpCar;
//extern tTerrain G_nTerrainType;
#define WHEEL_LOD


//#define TEST_DAMAGE
#define NEW_MODEL_HIERARCHY

const float AUTO_WHEEL_DIFF_FOR_CAM=0.05f;
float CAR_BALANCE_MULT = 0.08f;

// static variables in CAutomobile
Bool8 CAutomobile::m_sAllTaxiLights = FALSE;

#define JIMMY_TRANSFORM_TOTAL_TIME			(250.0f)

uint16 TOWTRUCK_HOIST_DOWN_LIMIT	= (20000);
uint16 TOWTRUCK_HOIST_UP_LIMIT		= (10000);
float TOWTRUCK_ROT_ANGLE			= (0.4f);
float TOWTRUCK_WIRE_LENGTH			= 1.0f;

//#define CAR_RAILTRACK_SURFACETYPE (SURFACE_TYPE_GRAVEL)
#define CAR_RAILTRACK_SURFACETYPE (SURFACE_TYPE_RAILTRACK)

//#define AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION

//
// GetCurrentAtomicObjectCB: Object callback function called for all atomics attached to a frame.
//							 Objective - to find and return currently drawn atomic model for vehicle
//							 componenent so that atomic may be cloned and transformed into a separate
//							 physical object
//
RwObject * GetCurrentAtomicObjectCB(RwObject *pObject, void *data)
{
	// if this atomic object is the one being rendered, then copy pointer into data
	if( RpAtomicGetFlags((RpAtomic*)pObject) & rpATOMICRENDER )
		*( (RpAtomic **) data ) = (RpAtomic*) pObject;
	// else do nothing
	return pObject;
}

//
//
//
CAutomobile::CAutomobile(int nModelIndex, UInt8 CreatedBy, UInt8 DoSuspensionLines)
: CVehicle(CreatedBy)
{
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(nModelIndex);
	ASSERT(pModelInfo);
	ASSERT(pModelInfo->GetModelType() == MI_TYPE_VEHICLE);
	ASSERT(nModelIndex != MODELID_BIKE_POLICE);
	
	int32 nHandlingIndex = pModelInfo->GetHandlingId();

	m_baseVehicleType = m_vehicleType = VEHICLE_TYPE_CAR;
	m_BlowUpTimer = 0;
	nBrakesOn = 0;

	if (m_sAllTaxiLights)
	{
		m_nAutomobileFlags.bTaxiLight = true;
	}
	else
	{
		m_nAutomobileFlags.bTaxiLight = false;		// Taxi light switched off initially
	}
	m_nAutomobileFlags.bShouldNotChangeColour = false;
	m_nAutomobileFlags.bWaterTight = false;
	m_nAutomobileFlags.bDoesNotGetDamagedUpsideDown = false;
	m_nAutomobileFlags.bCanBeVisiblyDamaged = true;
	m_nAutomobileFlags.bTankExplodesCars = true;
	m_nAutomobileFlags.bIsBoggedDownInSand = false;
//	m_nAutomobileFlags.bHeliIsCrashing = false;
	m_nAutomobileFlags.bIsMonsterTruck = false;
	
	SetModelIndex(nModelIndex);

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	// Special - some cars should use certain stations as startup condition (default condition already set in vehicle constructor)
	switch(nModelIndex)
	{
		case MODELID_BIKE_HARLEY:
//		case MODELID_BIKE_HELLS:
		case MODELID_HELI_HUNTER:
			m_nRadioStation = RADIO_5;					// VRock	(4)
			break;

		case MODELID_CAR_TOYZ:
		case MODELID_CAR_RCBARON:
		case MODELID_CAR_RCCOPTER:	
		case MODELID_CAR_RCGOBLIN:	
		case MODELID_CAR_RCBANDIT:
		case MODELID_CAR_GOLFCART:
		case MODELID_CAR_BAGGAGE:
			m_nRadioStation = AMBIENCE_CITY;			// Radio is off on these vehicles by default
			break;
	}
#endif //USE_AUDIOENGINE

	pHandling = mod_HandlingManager.GetPointer(nHandlingIndex);
	// copy handling flags across into Automobile structure so they can be modified
	hFlagsLocal = pHandling->hFlags;
	
	ASSERTMSG(!(hFlagsLocal &HF_HYDRAULICS_INSTALLED), "Hydraulics installed by handling");
	// randomly give some lowriders hydraulics
	if(hFlagsLocal &HF_HYDRAULICS_GEOMETRY && !(CGeneral::GetRandomNumber() &3))
	{
		//hFlagsLocal |= HF_HYDRAULICS_INSTALLED;
		AddVehicleUpgrade(MI_HYDRAULICS);
	}
		
	// get flying handling data (defaults to seaplane if no specific data)
	pFlyingHandling = mod_HandlingManager.GetFlyingPointer(nHandlingIndex);

	fBrakeCount = BRAKE_TO_REVERSE_DURATION;
	m_fGasPedalAudioRevs = 0.0f;
	m_fEngineRevs = 0.0f;
	m_fEngineForce = 0.0f;
	
//	nPadNum = 3;
	pModelInfo->ChooseVehicleColour(m_colour1, m_colour2, m_colour3, m_colour4);

	m_nVehicleFlags.bIsVan = false;
	m_nVehicleFlags.bIsBig = false;
	m_nVehicleFlags.bIsBus = false;
	m_nVehicleFlags.bLowVehicle = false;
	
	if (pHandling->mFlags & MF_IS_VAN)
		m_nVehicleFlags.bIsVan = true;
	if (pHandling->mFlags & MF_IS_BIG)
		m_nVehicleFlags.bIsBig = true;
	if (pHandling->mFlags & MF_IS_BUS)
		m_nVehicleFlags.bIsBus = true;
	if (pHandling->mFlags & MF_IS_LOW)
		m_nVehicleFlags.bLowVehicle = true;
		
	// initialise doors
	if(m_nVehicleFlags.bIsBus)
	{
		Door[FRONT_LEFT_DOOR].Init(-HALF_PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
		Door[FRONT_RIGHT_DOOR].Init(HALF_PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
	}
	else
	{
		Door[FRONT_LEFT_DOOR].Init(-0.4f*PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
		Door[FRONT_RIGHT_DOOR].Init(0.4f*PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
	}

	if(GetModelIndex()==MODELID_CAR_RHINO)
	{
		Door[REAR_LEFT_DOOR].SetExtraWheelPositions(1.0f, 1.0f, 1.0f, 1.0f);
		Door[REAR_RIGHT_DOOR].SetExtraWheelPositions(1.0f, 1.0f, 1.0f, 1.0f);

		RwObject *pRwObject;
		// don't render middle wheels for the moment
		pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_LM]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_RM]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
	}
	else if(m_nVehicleFlags.bIsVan) // allow vans rear doors to open fully 90 degrees
	{
		Door[REAR_LEFT_DOOR].Init(-HALF_PI, 0.0f, DOOR_AXIS_X, DOOR_AXIS_Z, DOOR_WILL_LOCK);
		Door[REAR_RIGHT_DOOR].Init(HALF_PI, 0.0f, DOOR_AXIS_NEG_X, DOOR_AXIS_Z, DOOR_WILL_LOCK);
	}
//	else if(pHandling->mFlags &MF_SUICIDE_REAR_DOORS)
//	{
//		Door[REAR_LEFT_DOOR].Init(0.4f*PI, 0.0f, DOOR_AXIS_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
//		Door[REAR_RIGHT_DOOR].Init(-0.4f*PI, 0.0f, DOOR_AXIS_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
//	}
	else
	{	
		Door[REAR_LEFT_DOOR].Init(-0.4f*PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
		Door[REAR_RIGHT_DOOR].Init(0.4f*PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_WILL_LOCK);
	}

 	// parameters are (max angle, min angle, sign, axes)
	if (pHandling->mFlags & MF_REVERSE_BONNET)
//		Door[BONNET].Init(0.0f, -0.3f*PI, TRUE, 0); // BANSHEE reverse bonnet
		Door[BONNET].Init(-0.3f*PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_X, DOOR_LOWGRAVITY); // BANSHEE reverse bonnet
	else
//		Door[BONNET].Init( 0.3f*PI, 0.0f, TRUE, 0);
		Door[BONNET].Init( 0.3f*PI, 0.0f, DOOR_AXIS_Y, DOOR_AXIS_X, DOOR_LOWGRAVITY);

	if (pHandling->mFlags &MF_HANGING_BOOT)
//		Door[BOOT].Init(0.0f, -0.4f*PI, FALSE, 0); // Boot hangs down and opens upwards
		Door[BOOT].Init(-0.4f*PI, 0.0f, DOOR_AXIS_NEG_Z, DOOR_AXIS_X, DOOR_WILL_LOCK); // Boot hangs down and opens upwards
	else if(pHandling->mFlags &MF_TAILGATE_BOOT)
//		Door[BOOT].Init(0.5f*PI, 0.0f, TRUE, 0);	// Boot is really a tailgate that opens down the way
		Door[BOOT].Init(0.5f*PI, 0.0f, DOOR_AXIS_Z, DOOR_AXIS_X, DOOR_WILL_LOCK);	// Boot is really a tailgate that opens down the way
	else
//		Door[BOOT].Init(0.0f, -0.3f*PI, TRUE, 0);
		Door[BOOT].Init(-0.3f*PI, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_X, DOOR_WILL_LOCK);

	if (pHandling->mFlags &MF_NO_DOORS)
	{
		Damage.SetDoorStatus(FRONT_LEFT_DOOR, DT_DOOR_MISSING);
		Damage.SetDoorStatus(FRONT_RIGHT_DOOR, DT_DOOR_MISSING);
		Damage.SetDoorStatus(REAR_LEFT_DOOR, DT_DOOR_MISSING);
		Damage.SetDoorStatus(REAR_RIGHT_DOOR, DT_DOOR_MISSING);
	}

	for (int i =0; i< MAX_DOORS; i++)
		DoorRotation[i] = CGeneral::GetRandomNumberInRange(-DAMAGED_DOOR_MAX_ANGLE, DAMAGED_DOOR_MAX_ANGLE);
	
	// make the tow rope swing around on the towtruck	
	if(GetModelIndex()==MODELID_CAR_TOWTRUCK && m_aCarNodes[CAR_MISC_B])
	{
		if(BouncingPanels[0].GetCompIndex()==-1)
			BouncingPanels[0].SetPanel(CAR_MISC_B, 2, 1.0f);
	}
	else if(GetModelIndex()==MODELID_CAR_TRACTOR && m_aCarNodes[CAR_BOOT])
	{
		if(BouncingPanels[0].GetCompIndex()==-1)
			BouncingPanels[0].SetPanel(CAR_BOOT, 1, 1.0f);
	}
/*
	// make some cars exhaust pipes bounce around a bit
	// IS it worth doing 
	else if(m_aCarNodes[CAR_EXHAUST] && !(pHandling->mFlags &MF_NO_PANEL_BOUNCING)
	&& !(CGeneral::GetRandomNumber() &3))
	{
		if(BouncingPanels[0].GetCompIndex()==-1)
			BouncingPanels[0].SetPanel(CAR_EXHAUST, 1, CGeneral::GetRandomNumberInRange(-0.05f, -0.15f));
	}
*/
	if(hFlagsLocal &HF_SWINGING_CHASSIS)
	{
		ChassisDoor.m_nDoorState = DT_DOOR_SWINGING_FREE;
		
		float fMaxAngle = 0.02f;
		if(GetModelIndex()==MODELID_CAR_COPCAR_LV || GetModelIndex()==MODELID_CAR_ESPERANTO)
			fMaxAngle = 0.03f;
		else if(GetModelIndex()==MODELID_CAR_STRETCH)
			fMaxAngle = 0.01f;
		
		ChassisDoor.Init(fMaxAngle*PI, -fMaxAngle*PI, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_CHASSIS|DOOR_FIXEDSTATE);
	}
	else if(GetModelIndex()==MODELID_CAR_FIRETRUCK_LA)
	{
		ChassisDoor.Init(0.1f*PI, -0.1f*PI, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_FIXEDSTATE|DOOR_LIMIT_SPD);
		ChassisDoor.m_nDoorState = DT_DOOR_SWINGING_FREE;
	}

	m_vecOldMoveSpeed = CVector(0.0f,0.0f,0.0f);
	m_vecOldTurnSpeed = CVector(0.0f,0.0f,0.0f);

	// these are defined in cPhysical	
	m_fMass = pHandling->fMass;
	m_fTurnMass = pHandling->fTurnMass;
	m_vecCOM = pHandling->CentreOfMass;
	m_fElasticity = 0.05f; // was 0.1
	m_fBuoyancyConstant = pHandling->fBuoyancyConstant;

	//m_fAirResistance = 0.9994f;
	if(pHandling->fDragCoeff > 0.01f)
		m_fAirResistance = 0.5f * pHandling->fDragCoeff / GAME_AIR_RESISTANCE_MASS;
	else
		m_fAirResistance = pHandling->fDragCoeff;


	ZRot = ZRotSpeed = 0.0f;
	m_nBusDoorTimer = 0;
	m_nBusDoorStart = 0;

	m_fSteerAngle = 0.0f;
	m_fGasPedal = 0.0f;
	m_fBrakePedal = 0.0f;

	pEntityThatSetUsOnFire = NULL;

	m_fGasPedalAudioRevs = 0.0f;	// Required for audio
//	m_bSirenOrAlarm = FALSE;
//	m_cHorn = 0;
//	m_nHornPattern = 0;

//	m_nSpeakTime = CTimer::GetTimeInMilliseconds();

	fPrevSpeed = 0.0f;
	m_nSuspensionHydraulics = 0;
	m_nOldSuspensionHydraulics = 0;
	// for each wheel
	for (int32 i = 0; i < 4; i++)
	{
		m_aGroundPhysicalPtrs[i] = NULL;
		m_aGroundOffsets[i] = CVector(0.0f, 0.0f, 0.0f);
		m_aWheelRatios[i] = 1.0f;
		m_aRatioHistory[i] = m_aWheelRatios[i];
		m_aWheelCounts[i] = 0.0f;
		m_aWheelPitchAngles[i] = DEGTORAD(0.0f);
		m_aWheelAngularVelocity[i] = 0.0f;
		m_aWheelLongitudinalSlip[i] = 0.0f;
		m_aWheelState[i] = WS_ROLLING;
		
		aWheelSkidmarkType[i]	= SKIDMARKTYPE_DEFAULT;
		bWheelBloody[i]			= FALSE;
	}

	nNoOfContactWheels = 0;			// Put declaration into automobile class as audio uses it
	m_nDriveWheelsOnGround = 0;		// Reset
	m_nDriveWheelsOnGroundLastFrame = 0;
	m_fHeightAboveRoad = 0.0f;
	m_fRearHeightAboveRoad = 0.0f;
	m_fExtraTractionMult = 1.0f;	// traction multiplier used by physical cars only
	m_fTyreTemp = 1.0f;

	// if vehicle collision model doesn't have any suspension lines that calculate them. Otherwise
	// just set the suspension heights (might want to leave to derrived classes)
	if(DoSuspensionLines)
		SetupSuspensionLines();

	SetStatus(STATUS_SIMPLE);
	m_nNumPassengers = 0;

	// Some cars have their door locked initially
	if (m_eDoorLockState == CARLOCK_UNLOCKED)
	{
		if (IsLawEnforcementVehicle())
		{
			m_eDoorLockState = CARLOCK_LOCKED_INITIALLY;
		}
	}
	
	GunOrientation = 0.0f;
	GunElevation = 0.05f;
	PropRotate = 0.0f;
	HeliRequestedOrientation = -1.0f;
	m_HarvesterTimer = 0;
	LeftDoorOpenForDriveBys = RightDoorOpenForDriveBys = 0.0f;
	
/*	// JIMMY BOND CODE	
	m_fTransformPosition = 0.0f;
	m_nTransformState = 0;
	// initialise base positions so code knows they need to be set
	// before components are moved
	for(int16 n=0; n<4; n++)
		m_fBasePositions[n] = -99999.9f;
		
	if(m_nModelIndex == MODELID_CAR_SENTINEL)
	{
		RwObject *pRwObject;
		// don't render covers
		pRwObject = GetFirstObject(m_aCarNodes[CAR_COVER_L]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		pRwObject = GetFirstObject(m_aCarNodes[CAR_COVER_R]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);

		// don't render fins
		pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_LF]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_LR]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_RF]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_RR]);
		RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
	}
*/
	// if Dodo (plane) don't want to draw front left wheel
	if(m_nModelIndex == MODELID_CAR_DODO)
	{
		//RwFrameForAllObjects(m_aCarNodes[CAR_WHEEL_LF], &SetVehicleAtomicVisibilityCB, (void*)VEHICLE_ATOMIC_NONE);
		RwObject* pWheel = GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]);
		RpAtomicSetFlags((RpAtomic*)pWheel, 0);
		
		
		if(m_nModelIndex == MODELID_CAR_DODO)
		{
			CMatrix matrix;
			matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
			CMatrix mat2 = RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]);
			CVector vecFLeft = mat2.GetTranslate();
			vecFLeft.y = 0.0f;
			vecFLeft.x += 0.1f;
			matrix.SetTranslate(vecFLeft);
			matrix.UpdateRW();
		}
	}
	else if(m_nModelIndex == MODELID_CAR_RHINO)
	{
		this->m_nPhysicalFlags.bIgnoresExplosions = true;
		this->m_nPhysicalFlags.bNotDamagedByBullets = true;
	}
	
	m_pFxSysNitro1 = NULL;
	m_pFxSysNitro2 = NULL;


#ifdef USE_AUDIOENGINE
	// initialise the VehicleAudioEntit-y
	m_VehicleAudioEntity.Initialise(this);
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

#ifndef FINAL
	Damage.m_pParentCar = this;
#endif

	m_heliDustRatio = 0.0f;
	m_statusWhenEngineBlown = EBS_NOT_SET;

	// for testing the nitro effect quickly	
//	hFlagsLocal = hFlagsLocal | HF_NOS_INSTALLED;
}


//////////////////////////////////////////////////////////////////////

//
//
//
CAutomobile::~CAutomobile()
{
	if (m_fxSysEngFire != NULL)
	{
		m_fxSysEngFire->Kill();
		m_fxSysEngFire = NULL;
	}
	
	StopNitroEffect();

#ifdef USE_AUDIOENGINE
	m_VehicleAudioEntity.Terminate();
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

}

/*#include "rpenvm.h"
RpAtomic* SetRenderPipeCB(RpAtomic* pAtomic, void* pData)
{
	RpEnvMAtomicSetup(pAtomic);
	
	return pAtomic;	
}

*/
void CAutomobile::SetModelIndex(uint32 index)
{
	CVehicle::SetModelIndex(index);
	SetupModelNodes();
	
	
//	RpClumpForAllAtomics((RpClump*)m_pRwObject, &SetRenderPipeCB, NULL);
}



//
//
//
void CAutomobile::SetupModelNodes()
{
#ifdef IGNORE_HIERARCHY
	return;
#endif

	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	ASSERT(pModelInfo->GetModelType() == MI_TYPE_VEHICLE);
	
	for(int32 i=0; i<MAX_CAR_NODES; i++)
		m_aCarNodes[i] = NULL;
	pModelInfo->FillFrameArray((RpClump*)m_pRwObject, &m_aCarNodes[0]);
}


//
//
//
void CAutomobile::SetupSuspensionLines()
{
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	CColModel& colModel = GetColModel();
	CCollisionData* pColData = colModel.GetCollisionData();
	
	ASSERT(!colModel.m_useSingleAlloc);
	ASSERT(pColData);
	
	bool bFirstTime = false;
	if(pColData->m_pLineArray == NULL)
	{
		if(GetModelIndex()==MODELID_CAR_RHINO)
			pColData->m_nNoOfLines = 12;
		else
			pColData->m_nNoOfLines = 4;

		pColData->m_pLineArray = (CColLine*)GtaMalloc(pColData->m_nNoOfLines*sizeof(CColLine));
		bFirstTime = true;
	}
	
	int16 i=0;
	bool bFrontWheel = true;
	for(i=0; i<4; i++)
	{
		CVector posn;

//		// temp hack until seaplane gets wheel nodes
//		if(GetModelIndex()==MODELID_BOAT_SEAPLANE)
//		{
//			CVector vecMin = colModel.GetBoundBoxMin();
//			CVector vecMax = colModel.GetBoundBoxMax();
//			if(i==0)
//				posn = CVector(vecMin.x, vecMax.y, vecMin.z);
//			else if(i==1)
//				posn = CVector(vecMin.x, vecMin.y, vecMin.z);
//			else if(i==2)
//				posn = CVector(vecMax.x, vecMax.y, vecMin.z);
//			else
//				posn = CVector(vecMax.x, vecMin.y, vecMin.z);
//		}
//		else

		pModelInfo->GetWheelPosn(i, posn);

		if(GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
		{
			if(posn.x > 0.0f)	posn.x += 0.15f;
			else posn.x -= 0.15f;
		}

		m_aWheelSuspensionHeights[i] = posn.z;
		
		posn.z += pHandling->fSuspensionUpperLimit;
		pColData->m_pLineArray[i].m_vecStart = posn;
		posn.z += -pHandling->fSuspensionUpperLimit + pHandling->fSuspensionLowerLimit - pModelInfo->GetWheelScale((i==0||i==2))*0.5f;
		pColData->m_pLineArray[i].m_vecEnd = posn;
//			colModel.m_pLineArray[i].m_vecStart = posn;
//			posn.z -= pHandling->fSuspensionUpperLimit + pModelInfo->GetWheelScale()*0.5f;
//			colModel.m_pLineArray[i].m_vecEnd = posn;

		m_fSuspensionLength[i] = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
		m_fLineLength[i] = pColData->m_pLineArray[i].m_vecStart.z - pColData->m_pLineArray[i].m_vecEnd.z;
	}

	// checks wheel positions are valid relative to each other, with exceptions
	if(GetVehicleAppearance() != APR_PLANE && GetVehicleAppearance() != APR_HELI)
	{
		ASSERTOBJ(pColData->m_pLineArray[0].m_vecStart.x < pColData->m_pLineArray[2].m_vecStart.x, pModelInfo->GetModelName(), "Front left and right wheels are the wrong way round");
		ASSERTOBJ(pColData->m_pLineArray[1].m_vecStart.x < pColData->m_pLineArray[3].m_vecStart.x, pModelInfo->GetModelName(), "Rear left and right wheels are the wrong way round");
		ASSERTOBJ(pColData->m_pLineArray[0].m_vecStart.y > pColData->m_pLineArray[1].m_vecStart.y, pModelInfo->GetModelName(), "Left front and rear wheels are the wrong way round");
		ASSERTOBJ(pColData->m_pLineArray[2].m_vecStart.y > pColData->m_pLineArray[3].m_vecStart.y, pModelInfo->GetModelName(), "Right front and rear wheels are the wrong way round");
	}
	// want to calculate default height above road here, for cars on rails	
	float fStart = pColData->m_pLineArray[0].m_vecStart.z;
	float fLength = m_fSuspensionLength[0];
	fLength *= 1.0f - 1.0f/(4.0f*pHandling->fSuspensionForce);
	m_fHeightAboveRoad = -fStart + fLength + pModelInfo->GetWheelScale(true)*0.5f;
	fStart = pColData->m_pLineArray[3].m_vecStart.z;
	fLength = m_fSuspensionLength[3];
	fLength *= 1.0f - 1.0f/(4.0f*pHandling->fSuspensionForce);
	m_fRearHeightAboveRoad = -fStart + fLength + pModelInfo->GetWheelScale(false)*0.5f;
#ifndef FINAL
	if(GetVehicleAppearance()!=APR_PLANE)
		ASSERTMSG(m_fHeightAboveRoad > 0.0f && m_fHeightAboveRoad < 3.0f, "Invalid car height above road");
#endif

	// also initialise wheel positions so they're rendered at the correct height
	for(i=0;i<4;i++)
		m_aWheelSuspensionHeights[i] = -m_fHeightAboveRoad + pModelInfo->GetWheelScale((i==0||i==2))*0.5f;

	// make sure bounding box still encloses all collision
	if(pColData->m_pLineArray[0].m_vecEnd.z < colModel.GetBoundBox().m_vecMin.z)
		colModel.GetBoundBox().m_vecMin.z = pColData->m_pLineArray[0].m_vecEnd.z;
	// resize bound sphere to enclose new bound box
	fLength = MAX(colModel.GetBoundBox().m_vecMin.Magnitude(), colModel.GetBoundBox().m_vecMax.Magnitude());
	if(colModel.GetBoundSphere().m_fRadius < fLength)
		colModel.GetBoundSphere().m_fRadius = fLength;

	//special case - want to increase size of bound sphere for rcbandit to make sure to enclose suspension lines		
	if(m_nModelIndex==MODELID_CAR_RCBANDIT){
		colModel.GetBoundSphere().m_fRadius = 2.0f;
		// temp //
		for(i=0;i<pColData->m_nNoOfSpheres;i++)
			pColData->m_pSphereArray[i].m_fRadius = 0.3f;
	}

#ifdef FINAL
	if(pHandling->mFlags &MF_FORCE_GRND_CLEARANCE && bFirstTime)
#else
	if(pHandling->mFlags &MF_FORCE_GRND_CLEARANCE)
#endif
	{
		float MIN_CLEARANCE_ON_MODEL = 0.25f;
		if(GetModelIndex()==MODELID_CAR_KART)
			MIN_CLEARANCE_ON_MODEL = 0.12f;
		
		float fMinPos = -m_fHeightAboveRoad + MIN_CLEARANCE_ON_MODEL;
		for(int i=0; i<pColData->m_nNoOfSpheres; i++)
		{
			if(pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius < fMinPos)
			{
				if(pColData->m_pSphereArray[i].m_fRadius > 0.4f)
					pColData->m_pSphereArray[i].m_fRadius = CMaths::Max(0.4f, pColData->m_pSphereArray[i].m_vecCentre.z - fMinPos);
			
				pColData->m_pSphereArray[i].m_vecCentre.z = fMinPos + pColData->m_pSphereArray[i].m_fRadius;
			}
		}
	}
	
	if(GetModelIndex()==MODELID_CAR_RHINO)
	{
		for(int nSide = 0; nSide < 2; nSide++)
		{
			for(int nLine=0; nLine<4; nLine++)
			{
				float fMinMult = 1.0f - (nLine + 1)*0.2f;
				float fMaxMult = (nLine + 1)*0.2f;
				pColData->m_pLineArray[nLine + nSide*4 + 4].m_vecStart = fMinMult*pColData->m_pLineArray[nSide*2].m_vecStart
																	  + fMaxMult*pColData->m_pLineArray[nSide*2 + 1].m_vecStart;

				pColData->m_pLineArray[nLine + nSide*4 + 4].m_vecEnd = fMinMult*pColData->m_pLineArray[nSide*2].m_vecEnd
																	  + fMaxMult*pColData->m_pLineArray[nSide*2 + 1].m_vecEnd;
			}
		}
	}
}



//
//
//
TweakFloat MAX_RESTING_SPEED		= (0.02f);
TweakFloat MAX_LOWER_SPEED			= (0.01f);
TweakInt32 CRAWL_COUNTER			= (20);
TweakInt32 ACTIVE_COUNTER			= (60);
TweakInt32 JACKED_COUNTER			= (500);

TweakFloat HYDRAULIC_CRAWL_LENGTH_MULT	= (0.7f);
TweakFloat HYDRAULIC_MIN_OVERLAP 		= (0.1f);
TweakFloat HYDRAULIC_MIN_JACKED_OVERLAP	= (0.0f);
TweakFloat HYDRAULIC_STICK_CONTROL_MULT	= (1.5f);
TweakFloat HYDRAULIC_DAMPING			= (0.10f);

TweakFloat HYDRAULIC_FORCE_MULT 		= (1.0f);
TweakFloat HYDRAULIC_JACKED_MOVE_DIFF	= (0.2f);
TweakFloat LOWRIDER_HYDRAULIC_FORCE_MULT 		= (1.5f);
TweakFloat LOWRIDER_HYDRAULIC_JACKED_MOVE_DIFF	= (0.4f);
//
void CAutomobile::HydraulicControl()
{
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	CColModel& colModelDefault = pModelInfo->GetColModel();
	float fWheelRad = pModelInfo->GetWheelScale(true)*0.5f;
	float aWheelColPts[4];
	if(GetStatus()!=STATUS_PLAYER)
	{
		if(GetStatus()!=STATUS_PHYSICS || VehicleCreatedBy!=MISSION_VEHICLE
		|| m_nSpecialColModel < 0)
			return;
	}
	
	if(hFlagsLocal &HF_HYDRAULICS_NOT_AVAILABLE)
		return;

	float fStdUpper, fStdLower, fStdLength;		// standard driving settings
	float fJackedUpper, fJackedLower, fJackedLength;	// fully jacked up settings
	uint16 i = 0;
	uint16 setting = 0;
	CVector posn;
	
	// check incase player is dying or something?!
	CPlayerPed *pPlayerPed = NULL;
	if(GetStatus()==STATUS_PLAYER)
	{
		if(pDriver && pDriver->IsPlayer())
			pPlayerPed = (CPlayerPed *)pDriver;

//		if(CWorld::Players[CWorld::PlayerInFocus].PlayerState != CPlayerInfo::PLAYERSTATE_PLAYING)
//			return;
		if(CGameLogic::GameState != CGameLogic::GAMESTATE_PLAYING)
			return;
	}
		
	if(!GetSpecialColModel())
		return;

	CColModel &colModel = CVehicle::m_aSpecialColModel[m_nSpecialColModel];
	CCollisionData* pColData = colModel.GetCollisionData();
	
	// need to work out hydraulic settings for this vehicle
	if(m_aSpecialHydraulicData[m_nSpecialColModel].fJackedUpper >= 100.0f)
	{
		// need to know where the bottom of this vehicle's collision model is so we never
		// move the top of the suspension line below there
		float fLowestSphereZ = 0.0f;
		for(int i=0; i<pColData->m_nNoOfSpheres; i++)
		{
			if(pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius < fLowestSphereZ)
				fLowestSphereZ = pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius;
		}
		// doing all this relative to the original wheel posisions
		pModelInfo->GetWheelPosn(0, posn);
		fLowestSphereZ -= posn.z;

		// the standard position will be modified depending on vehicle speed
		fStdUpper = pHandling->fSuspensionUpperLimit;
		fStdLower = pHandling->fSuspensionLowerLimit;
		if(hFlagsLocal &HF_HYDRAULICS_GEOMETRY)
			fStdLength = (fStdUpper - fStdLower)*LOWRIDER_HYDRAULIC_FORCE_MULT;
		else
			fStdLength = (fStdUpper - fStdLower)*HYDRAULIC_FORCE_MULT;
		fStdUpper = fStdLower + fStdLength;
		
		m_aSpecialHydraulicData[m_nSpecialColModel].fStdUpper = fStdUpper;
		m_aSpecialHydraulicData[m_nSpecialColModel].fStdLower = fStdLower;

		if(hFlagsLocal &HF_HYDRAULICS_GEOMETRY)
		{
			fJackedUpper = fStdUpper - LOWRIDER_HYDRAULIC_JACKED_MOVE_DIFF;
			fJackedLower = fStdLower - LOWRIDER_HYDRAULIC_JACKED_MOVE_DIFF;
		}
		else
		{
			fJackedUpper = fStdUpper - HYDRAULIC_JACKED_MOVE_DIFF;
			fJackedLower = fStdLower - HYDRAULIC_JACKED_MOVE_DIFF;
		}
		
		// make sure we don't push the top of the sus-line outside the col model
		if(fJackedUpper < fLowestSphereZ + HYDRAULIC_MIN_JACKED_OVERLAP)
			fJackedUpper = fLowestSphereZ + HYDRAULIC_MIN_JACKED_OVERLAP;
		
		fJackedLength = fJackedUpper - fJackedLower;
		
		m_aSpecialHydraulicData[m_nSpecialColModel].fJackedUpper = fJackedUpper;
		m_aSpecialHydraulicData[m_nSpecialColModel].fJackedLower = fJackedLower;
		
		
		m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedUpper = fStdUpper - 0.5f*(1.0f - HYDRAULIC_CRAWL_LENGTH_MULT)*fStdLength;
		m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedLower = fStdLower + 0.5f*(1.0f - HYDRAULIC_CRAWL_LENGTH_MULT)*fStdLength;
		
		fLowestSphereZ += fWheelRad;
		if(m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedLower > fLowestSphereZ - HYDRAULIC_MIN_OVERLAP)
		{
			float fMoveDown = m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedLower - (fLowestSphereZ - HYDRAULIC_MIN_OVERLAP);
			m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedUpper -= fMoveDown;
			m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedLower -= fMoveDown;
		}
	}
	else
	{
		fStdUpper = m_aSpecialHydraulicData[m_nSpecialColModel].fStdUpper;
		fStdLower = m_aSpecialHydraulicData[m_nSpecialColModel].fStdLower;
		fStdLength = fStdUpper - fStdLower;
		fJackedUpper = m_aSpecialHydraulicData[m_nSpecialColModel].fJackedUpper;
		fJackedLower = m_aSpecialHydraulicData[m_nSpecialColModel].fJackedLower;
		fJackedLength = fJackedUpper - fJackedLower;
	}
	
	bool bResetHistory = false;
	if(m_nSuspensionHydraulics < CRAWL_COUNTER && fPrevSpeed > MAX_RESTING_SPEED
	&& GetGasPedal())
	{
		if(m_nSuspensionHydraulics == 0){
			m_nSuspensionHydraulics = CRAWL_COUNTER;
//			for(i=0; i<4; i++)
//				m_aWheelSuspensionHeights[i] -= fSusHeight1;
#ifdef USE_AUDIOENGINE
			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_ON);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_JACKUP_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
			bResetHistory = true;
		}
		else
			m_nSuspensionHydraulics++;
	}
	else if(m_nSuspensionHydraulics > 0 && m_nSuspensionHydraulics < ACTIVE_COUNTER+1 && fPrevSpeed < MAX_LOWER_SPEED)
	{
		if(--m_nSuspensionHydraulics == 0)
#ifdef USE_AUDIOENGINE
			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_OFF);
			
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_RESET_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
	}

	// do player pad controls for the suspension	
	if(pPlayerPed && pPlayerPed->GetPadFromPlayer()->HornJustDown())
	{
		// raise or lower car
		if(m_nSuspensionHydraulics < JACKED_COUNTER)
			m_nSuspensionHydraulics = JACKED_COUNTER;	// jack up
		// don't go straight from jacked all the way up to all the way down
		else// if(fPrevSpeed > MAX_LOWER_SPEED)
			m_nSuspensionHydraulics = ACTIVE_COUNTER;	// lower to moving setting
//		else
//			m_nSuspensionHydraulics = 0;		// lower to crawling setting
		
		// raise up
		if(m_nSuspensionHydraulics >= JACKED_COUNTER)
		{
			for(i=0; i<4; i++)
			{
				// save the current position of the wheel
				aWheelColPts[i] = pColData->m_pLineArray[i].m_vecStart.z - m_aWheelRatios[i]*m_fLineLength[i];
			
				pModelInfo->GetWheelPosn(i, posn);

				posn.z += fJackedUpper;
				pColData->m_pLineArray[i].m_vecStart = posn;
				posn.z -= fJackedLength + fWheelRad;
				pColData->m_pLineArray[i].m_vecEnd = posn;
				m_fSuspensionLength[i] = fJackedLength;
				m_fLineLength[i] = fJackedLength + fWheelRad;
				
				// set wheel ratios so that wheel will remain in the same position
				if(m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
				{
					m_aWheelRatios[i] = (pColData->m_pLineArray[i].m_vecStart.z - aWheelColPts[i]) / m_fLineLength[i];
					if(m_aWheelRatios[i] > BILLS_EXTENSION_LIMIT)
						m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;
				}

				// reset history so code doesn't think wheel has hitsomething (plays bump sound)
				bResetHistory = true;
				// move the wheels down a bit
//				m_aWheelSuspensionHeights[i] -= fSusHeight2;
			}

#ifdef USE_AUDIOENGINE
			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_TRIGGER);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_JACKUP_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
		}
		// lower down
		else
		{
//			if(m_nSuspensionHydraulics == 0)	// lower to slow speed v-low setting
//			{
//				fStdUpper = m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedUpper;
//				fStdLower = m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedLower;
//				fStdLength = fStdUpper - fStdLower;
//			}
			
			for(i=0; i<4; i++)
			{
				// save the current position of the wheel
				aWheelColPts[i] = pColData->m_pLineArray[i].m_vecStart.z - m_aWheelRatios[i]*m_fLineLength[i];
			
				pModelInfo->GetWheelPosn(i, posn);

				posn.z += fStdUpper;
				pColData->m_pLineArray[i].m_vecStart = posn;
				posn.z -= fStdLength + fWheelRad;
				pColData->m_pLineArray[i].m_vecEnd = posn;

				m_fSuspensionLength[i] = fStdLength;
				m_fLineLength[i] = fStdLength + fWheelRad;

				// set wheel ratios so that wheel will remain in the same position
				if(m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
				{
					m_aWheelRatios[i] = (pColData->m_pLineArray[i].m_vecStart.z - aWheelColPts[i]) / m_fLineLength[i];
					if(m_aWheelRatios[i] > BILLS_EXTENSION_LIMIT)
						m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;
				}
			}
			
#ifdef USE_AUDIOENGINE
				m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_HYDRAULIC);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_RESET_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
		}
	}
	else
	{
		float fControlArray[4] = {1.0f, 1.0f, 1.0f, 1.0f};
		float fNewUpper, fNewLength;
		float fOldLowerPos;
		// want to count the max change in suspension line length
		float fMaxLineEndDelta = 0.0f;

		if(GetStatus()==STATUS_PHYSICS)
		{
			fControlArray[0] = m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[0];	// FL
			fControlArray[1] = m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[1];	// BL
			fControlArray[2] = m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[2];	// FR
			fControlArray[3] = m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[3];	// BR
		}
		else if(pPlayerPed && !pPlayerPed->GetPadFromPlayer()->GetHydraulicJump())
		{
			float fGunLR = pPlayerPed->GetPadFromPlayer()->GetCarGunLeftRight(true);
			float fGunUD = pPlayerPed->GetPadFromPlayer()->GetCarGunUpDown(true);
			
			float fStickMag = HYDRAULIC_STICK_CONTROL_MULT*CMaths::Sqrt(fGunLR*fGunLR + fGunUD*fGunUD)/128.0f;
			static float HYDRAULIC_STICK_ROTATE_ANGLE = -QUARTER_PI;
			float fStickAngle = CMaths::ATan2(fGunLR, fGunUD) + HYDRAULIC_STICK_ROTATE_ANGLE;
			
			float fControl_3_0 = fStickMag*CMaths::Cos(fStickAngle);
			float fControl_2_1 = fStickMag*CMaths::Sin(fStickAngle);
			
			fControlArray[0] = CMaths::Max(0.0f, -fControl_3_0);
			fControlArray[1] = CMaths::Max(0.0f, -fControl_2_1);
			fControlArray[2] = CMaths::Max(0.0f, fControl_2_1);
			fControlArray[3] = CMaths::Max(0.0f, fControl_3_0);
			
//			fControlArray[0] = HYDRAULIC_STICK_CONTROL_MULT*MAX(0.0f, -1.0f*fGunUD/128.0f - fGunLR/128.0f);	// FL
//			fControlArray[1] = HYDRAULIC_STICK_CONTROL_MULT*MAX(0.0f, fGunUD/128.0f - fGunLR/128.0f);		// BL
//			fControlArray[2] = HYDRAULIC_STICK_CONTROL_MULT*MAX(0.0f, -1.0f*fGunUD/128.0f + fGunLR/128.0f);	// FR
//			fControlArray[3] = HYDRAULIC_STICK_CONTROL_MULT*MAX(0.0f, fGunUD/128.0f + fGunLR/128.0f);		// BR
		}
		// otherwise they're all initialised to 1.0

		// suspension already jacked up, so controls raise individual wheels
		if(m_nSuspensionHydraulics >= JACKED_COUNTER)
		{
			// try and help suspension push wheels into the ground when jacking up
			if(m_nSuspensionHydraulics < JACKED_COUNTER+4){
				m_nSuspensionHydraulics++;
//				for(i=0; i<4; i++)
//					m_aWheelSuspensionHeights[i] -= fSusHeight3;
			}
			
			// use slow speed v-low setting
//			if(fPrevSpeed < MAX_LOWER_SPEED)
//			{
//				fStdUpper += CRAWL_UPPER_DIFF;
//				fStdLower += CRAWL_LOWER_DIFF;
//				fStdLength = fStdUpper - fStdLower;
//			}
		
			for(i=0; i<4; i++)
			{
				// limit how much the suspension can be pulled up so it doesn't go through the bodywork
				if(fControlArray[i] > 1.0f)	fControlArray[i] = 1.0f;
				
				fNewUpper = fJackedUpper + fControlArray[i]*(fStdUpper - fJackedUpper);
				fNewLength = fJackedLength + fControlArray[i]*(fStdLength - fJackedLength);
				// save the current position of the wheel
				aWheelColPts[i] = pColData->m_pLineArray[i].m_vecStart.z - m_aWheelRatios[i]*m_fLineLength[i];
			
				pModelInfo->GetWheelPosn(i, posn);

				posn.z += fNewUpper;
				pColData->m_pLineArray[i].m_vecStart = posn;
				posn.z -= fNewLength + fWheelRad;

				// calculate maximum absolute change in Suspension end position before it's applied
				if(CMaths::Abs(posn.z - pColData->m_pLineArray[i].m_vecEnd.z) > CMaths::Abs(fMaxLineEndDelta))
					fMaxLineEndDelta = posn.z - pColData->m_pLineArray[i].m_vecEnd.z;

				pColData->m_pLineArray[i].m_vecEnd = posn;
				
				m_fSuspensionLength[i] = fNewLength;
				m_fLineLength[i] = fNewLength + fWheelRad;

				// set wheel ratios so that wheel will remain in the same position
				if(m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
				{
					m_aWheelRatios[i] = (pColData->m_pLineArray[i].m_vecStart.z - aWheelColPts[i]) / m_fLineLength[i];
					if(m_aWheelRatios[i] > BILLS_EXTENSION_LIMIT)
						m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;
				}
			}
		}
		// suspension sitting low down, so controls lower individual wheels
		else
		{
			if(fControlArray[0]!=0.0f || fControlArray[1]!=0.0f || fControlArray[2]!=0.0f || fControlArray[3]!=0.0f)
			{
				if(m_nSuspensionHydraulics==0)
					fControlArray[0] = fControlArray[1] = fControlArray[2] = fControlArray[3] = 0.0f;

				m_nSuspensionHydraulics = ACTIVE_COUNTER;
			}
			else if(m_nSuspensionHydraulics==0)
			{
				fStdUpper = m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedUpper;
				fStdLower = m_aSpecialHydraulicData[m_nSpecialColModel].fStoppedLower;
				fStdLength = fStdUpper - fStdLower;			
			}
			
			for(i=0; i<4; i++)
			{
				// limit how much the suspension can be pushed down so it doesn't look bad
				if(fControlArray[i] > 1.0f)	fControlArray[i] = 1.0f;

				// save where the bottom of the suspension was last frame
				fOldLowerPos = pColData->m_pLineArray[i].m_vecEnd.z;

				fNewUpper = fStdUpper + fControlArray[i]*(fJackedUpper - fStdUpper);
				fNewLength = fStdLength + fControlArray[i]*(fJackedLength - fStdLength);
				// save the current position of the wheel
				aWheelColPts[i] = pColData->m_pLineArray[i].m_vecStart.z - m_aWheelRatios[i]*m_fLineLength[i];;
			
				pModelInfo->GetWheelPosn(i, posn);

				posn.z += fNewUpper;
				pColData->m_pLineArray[i].m_vecStart = posn;
				posn.z -= fNewLength + fWheelRad;

				// calculate maximum absolute change in Suspension end position before it's applied
				if(CMaths::Abs(posn.z - pColData->m_pLineArray[i].m_vecEnd.z) > CMaths::Abs(fMaxLineEndDelta))
					fMaxLineEndDelta = posn.z - pColData->m_pLineArray[i].m_vecEnd.z;
				// now apply position
				pColData->m_pLineArray[i].m_vecEnd = posn;

				m_fSuspensionLength[i] = fNewLength;
				m_fLineLength[i] = fNewLength + fWheelRad;

				// set wheel ratios so that wheel will remain in the same position
				if(m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
				{
					m_aWheelRatios[i] = (pColData->m_pLineArray[i].m_vecStart.z - aWheelColPts[i]) / m_fLineLength[i];
					if(m_aWheelRatios[i] > BILLS_EXTENSION_LIMIT)
						m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;

//					m_aWheelSuspensionHeights[i] -= fSusHeight4*(fOldLowerPos - colModel.m_pLineArray[i].m_vecEnd.z);
				}
			}
		}
		
		// check for fJackLower-fStdLower != 0.0f to make sure no divide by zero
		if( fJackedLower-fStdLower != 0.0f && CMaths::Abs(fMaxLineEndDelta/(fJackedLower-fStdLower)) > 0.01)
		{
			fMaxLineEndDelta = 0.5f*( (fJackedLower-fStdLower) + fMaxLineEndDelta)/(fJackedLower-fStdLower);
			if(fMaxLineEndDelta < 0.0f)
				fMaxLineEndDelta = 0.0f;
			else if(fMaxLineEndDelta > 1.0f)
				fMaxLineEndDelta = 1.0f;

#ifdef USE_AUDIOENGINE
//			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_TRIGGER);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_SLOW_SUSPENSION_MOVEMENT_LOOP, fMaxLineEndDelta);
#endif //USE_AUDIOENGINE
			
//			sprintf(gString, "AE_SLOW_SUSPENSION_MOVEMENT_LOOP: %f", fMaxLineDelta);
//			CDebug::PrintToScreenCoors(gString, 25,30);
//			printf("Sus: %f\n", fMaxLineDelta);
			
			if(fMaxLineEndDelta < 0.4f || fMaxLineEndDelta > 0.6f)
				bResetHistory = true;

			static float TRIGGER_MOVEMENT_DOWN = 0.05f;
			static float TRIGGER_MOVEMENT_UP = 0.05f;
			static float TRIGGER_MOVEMENT_UP_SLOW = 0.025f;
			
			if(fMaxLineEndDelta < 0.5f - TRIGGER_MOVEMENT_DOWN*CTimer::GetTimeStep())//0.25f)
			{
#ifdef USE_AUDIOENGINE
				m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_HYDRAULIC);
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_RESET_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
			}
			else if(fMaxLineEndDelta > 0.5f + TRIGGER_MOVEMENT_UP*CTimer::GetTimeStep())//0.75f)
			{
#ifdef USE_AUDIOENGINE
				m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_TRIGGER);
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_JACKUP_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
			}
			else if(fMaxLineEndDelta > 0.5f + TRIGGER_MOVEMENT_UP_SLOW*CTimer::GetTimeStep())//0.75f)
			{
#ifdef USE_AUDIOENGINE
				m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_HYDRAULIC);
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_JACKUP_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
			}
		}
	}

	if(bResetHistory)
	{
		// reset history so code doesn't think wheel has hitsomething (plays bump sound)
		float tempRatio = 0.0f;
		for(i=0; i<4; i++){
			tempRatio = 1.0f - m_fSuspensionLength[i] / m_fLineLength[i];
			m_aRatioHistory[i] = (m_aWheelRatios[i] - tempRatio) / (1.0f - tempRatio);
		}
	}

	m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[0] = 0.0f;
	m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[1] = 0.0f;
	m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[2] = 0.0f;
	m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[3] = 0.0f;
}


TweakFloat DEFAULT_COLLISION_MOVERATE	= 10.0f;
TweakFloat DEFAULT_COLLISION_RETURNRATE	= 20.0f;
int16 DEFAULT_COLLISION_EXTENDLIMIT = 2500;
//
CVector PACKER_COL_PIVOT(0.0f, 0.0f, 2.0f);
TweakFloat 	PACKER_COL_ANGLEMULT = -0.0001f;
//
CVector DOZER_COL_PIVOT(0.0f, 1.0f, -0.5f);
TweakFloat	DOZER_COL_ANGLEMULT = 0.0002f;
//
TweakFloat FORKLIFT_MOVE_MULT = 0.0006f;
TweakFloat FORKLIFT_COLLISION_MOVERATE = -10.0f;
TweakFloat FORKLIFT_COLLISION_RETURNRATE = -20.0f;
//
bool CAutomobile::UpdateMovingCollision(float fScriptControl)
{
	CPad *pPad = NULL;

	// Need OldSusHyd == SusHyd or the moving sound will play constantly
	if( (GetModelIndex()==MODELID_CAR_PACKER || GetModelIndex()==MODELID_CAR_DOZER
		|| GetModelIndex()==MODELID_CAR_DUMPER || GetModelIndex()==MODELID_CAR_CEMENT
		|| GetModelIndex()==MODELID_PLANE_ANDROM || GetModelIndex()==MODELID_CAR_FORKLIFT))

		m_nOldSuspensionHydraulics = m_nSuspensionHydraulics;

	if(GetStatus()!=STATUS_PLAYER)
	{
		// only make changes when the script is directly controlling it
		if(VehicleCreatedBy!=MISSION_VEHICLE || fScriptControl < 0.0f)
			return false;
		// only allow script control for valid vehicle types
		else if( !(GetModelIndex()==MODELID_CAR_PACKER || GetModelIndex()==MODELID_CAR_DOZER
		|| GetModelIndex()==MODELID_CAR_DUMPER || GetModelIndex()==MODELID_CAR_CEMENT
		|| GetModelIndex()==MODELID_PLANE_ANDROM || GetModelIndex()==MODELID_CAR_FIRETRUCK_LA
		|| GetModelIndex()==MODELID_CAR_FORKLIFT))
			return false;
		
		ASSERTMSG(fScriptControl <= 1.0f, "Script control must be between 0.0 - 1.0");
	}
	else if(pDriver && pDriver->IsPlayer())
	{
		// check incase player is dying or something?!
//		if(CWorld::Players[CWorld::PlayerInFocus].PlayerState != CPlayerInfo::PLAYERSTATE_PLAYING)
//			return false;
		if(CGameLogic::GameState != CGameLogic::GAMESTATE_PLAYING)
			return false;

		pPad = ((CPlayerPed *)pDriver)->GetPadFromPlayer();
	}
	else
		return false;
		
	if(GetModelIndex()==MODELID_CAR_CEMENT || GetModelIndex()==MODELID_CAR_FIRETRUCK_LA)
	{
		m_nOldSuspensionHydraulics = m_nSuspensionHydraulics;
		// script controls first
		if(fScriptControl >= 0.0f)
			m_nSuspensionHydraulics = fScriptControl*DEFAULT_COLLISION_EXTENDLIMIT;
		else if(pPad)
		{
			if(pPad->GetCarGunUpDown() < -10.0f)
				m_nSuspensionHydraulics = CMaths::Min(DEFAULT_COLLISION_EXTENDLIMIT, m_nSuspensionHydraulics - (int16)(pPad->GetCarGunUpDown()*2*DEFAULT_COLLISION_MOVERATE*CTimer::GetTimeStep() / 128.0f));
			else if(m_nSuspensionHydraulics > 0)
				m_nSuspensionHydraulics = CMaths::Max(0, m_nSuspensionHydraulics - (int16)((pPad->GetCarGunUpDown() + 100)*DEFAULT_COLLISION_MOVERATE*CTimer::GetTimeStep() / 128.0f));
		}
		return false;
	}
	
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	CColModel& colModelDefault = pModelInfo->GetColModel();
	CCollisionData* pColDataDefault = colModelDefault.GetCollisionData();
	
	ASSERT(pColDataDefault);

	// make sure special col model is available and allocated
	if(!GetSpecialColModel())
	{
		ASSERTMSG(false, "Moving collision - run out of col models");
		return false;
	}
		
	CColModel &colModel = CVehicle::m_aSpecialColModel[m_nSpecialColModel];
	CCollisionData* pColData = colModel.GetCollisionData();

	int32 i;
	bool bMovedStuff = false;
	CVector vecTempVertex;
	CVector vecPivot;
	if(fScriptControl >= 0.0f || (pPad && CMaths::Abs( pPad->GetCarGunUpDown() ) > 10.0f))
	{
		m_nOldSuspensionHydraulics = m_nSuspensionHydraulics;
		
		if(fScriptControl >= 0.0f)
			m_nSuspensionHydraulics = fScriptControl*DEFAULT_COLLISION_EXTENDLIMIT;
		else
		{
			float fMoveRate = DEFAULT_COLLISION_MOVERATE;
			if(GetModelIndex()==MODELID_CAR_FORKLIFT)
			{
				if(pPad->GetCarGunUpDown() < 0.0f)
					fMoveRate = FORKLIFT_COLLISION_MOVERATE;
				else
					fMoveRate = FORKLIFT_COLLISION_RETURNRATE;
			}
		
			m_nSuspensionHydraulics = MAX(0, m_nSuspensionHydraulics + (int16)(pPad->GetCarGunUpDown()*fMoveRate*CTimer::GetTimeStep() / 128.0f));
			if(m_nSuspensionHydraulics > DEFAULT_COLLISION_EXTENDLIMIT)
				m_nSuspensionHydraulics = DEFAULT_COLLISION_EXTENDLIMIT;
		}
		
		CMatrix matRot;
		
		if(GetModelIndex()==MODELID_CAR_DOZER && m_aCarNodes[CAR_MISC_A])
		{
			matRot.SetRotateX(DOZER_COL_ANGLEMULT*m_nSuspensionHydraulics);
			vecPivot = RwFrameGetMatrix(m_aCarNodes[CAR_MISC_A])->pos;
		}
		else if(GetModelIndex()==MODELID_CAR_DUMPER && GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK
		&& m_aCarNodes[TRUCK_MISC_A])
		{
			matRot.SetRotateX(CMonsterTruck::DUMPER_COL_ANGLEMULT*m_nSuspensionHydraulics);
			vecPivot = RwFrameGetMatrix(m_aCarNodes[TRUCK_MISC_A])->pos;
		}
		else if(GetModelIndex()==MODELID_PLANE_ANDROM && m_aCarNodes[PLANE_MISC_B])
		{
			matRot.SetRotateX(CPlane::ANDROM_COL_ANGLE_MULT*m_nSuspensionHydraulics);
			vecPivot = RwFrameGetMatrix(m_aCarNodes[PLANE_MISC_B])->pos;
		}
		else if(GetModelIndex()==MODELID_CAR_PACKER && m_aCarNodes[CAR_MISC_A])
		{
			matRot.SetRotateX(PACKER_COL_ANGLEMULT*m_nSuspensionHydraulics);
			vecPivot = RwFrameGetMatrix(m_aCarNodes[CAR_MISC_A])->pos;//PACKER_COL_PIVOT;
		}
		else if(GetModelIndex()==MODELID_CAR_FORKLIFT && m_aCarNodes[CAR_MISC_A])
		{
			matRot.SetTranslate(0.0f, 0.0f, FORKLIFT_MOVE_MULT*m_nSuspensionHydraulics);
			vecPivot = CVector(0.0f,0.0f,0.0f);
		}
//		matRot.Translate(-vecPivot);

		float fMaxMovedColZ = -1000.0f;
		float fMinMovedColZ = 1000.0f;
		for(i=0; i<pColData->m_nNoOfTriangles; i++)
		{
			if(pColData->m_pTriangleArray[i].m_nSurfaceType==SURFACE_TYPE_CAR_MOVINGCOMPONENT)// || GetModelIndex()==MODELID_CAR_PACKER)
			{
				// index1
				DecompressTranslationVector(vecTempVertex, pColDataDefault->m_pTriCompressedVectorArray[pColDataDefault->m_pTriangleArray[i].m_nIndex1]);
				vecTempVertex = matRot*(vecTempVertex - vecPivot) + vecPivot;
				CompressTranslationVector(pColData->m_pTriCompressedVectorArray[pColData->m_pTriangleArray[i].m_nIndex1], vecTempVertex);
				if(vecTempVertex.z > fMaxMovedColZ)
					fMaxMovedColZ = vecTempVertex.z;
				else if(vecTempVertex.z < fMinMovedColZ)
					fMinMovedColZ = vecTempVertex.z;
				// index2
				DecompressTranslationVector(vecTempVertex, pColDataDefault->m_pTriCompressedVectorArray[pColDataDefault->m_pTriangleArray[i].m_nIndex2]);
				vecTempVertex = matRot*(vecTempVertex - vecPivot) + vecPivot;
				CompressTranslationVector(pColData->m_pTriCompressedVectorArray[pColData->m_pTriangleArray[i].m_nIndex2], vecTempVertex);
				if(vecTempVertex.z > fMaxMovedColZ)
					fMaxMovedColZ = vecTempVertex.z;
				else if(vecTempVertex.z < fMinMovedColZ)
					fMinMovedColZ = vecTempVertex.z;
				// index3
				DecompressTranslationVector(vecTempVertex, pColDataDefault->m_pTriCompressedVectorArray[pColDataDefault->m_pTriangleArray[i].m_nIndex3]);
				vecTempVertex = matRot*(vecTempVertex - vecPivot) + vecPivot;
				CompressTranslationVector(pColData->m_pTriCompressedVectorArray[pColData->m_pTriangleArray[i].m_nIndex3], vecTempVertex);
				if(vecTempVertex.z > fMaxMovedColZ)
					fMaxMovedColZ = vecTempVertex.z;
				else if(vecTempVertex.z < fMinMovedColZ)
					fMinMovedColZ = vecTempVertex.z;
			}
			
			// packer is using test method since collision types not setup properly
			//if(GetModelIndex()==MODELID_CAR_PACKER)
			//	colModel.m_pTriangleArray[i].m_nSurfaceType = COLPOINT_SURFACETYPE_CAR_MOVINGCOMPONENT;
		}
		if(pColData->m_pTrianglePlaneArray)
		{
			for(i=0; i<pColData->m_nNoOfTriangles; i++)
			{
				pColData->m_pTrianglePlaneArray[i].Set(pColData->m_pTriCompressedVectorArray, pColData->m_pTriangleArray[i]);
			}
		}
		
		for(i=0; i<pColData->m_nNoOfSpheres; i++)
		{
			if(pColData->m_pSphereArray[i].m_data.m_nSurfaceType==SURFACE_TYPE_CAR_MOVINGCOMPONENT)
			{
				pColData->m_pSphereArray[i].m_vecCentre = matRot*(pColDataDefault->m_pSphereArray[i].m_vecCentre - vecPivot) + vecPivot;
				if(pColData->m_pSphereArray[i].m_vecCentre.z + pColData->m_pSphereArray[i].m_fRadius > fMaxMovedColZ)
					fMaxMovedColZ = pColData->m_pSphereArray[i].m_vecCentre.z + pColData->m_pSphereArray[i].m_fRadius;
				else if(pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius < fMinMovedColZ)
					fMinMovedColZ = pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius;
			}
		}
		
		// update bounding box to enclose moved collision
		if(fMaxMovedColZ > colModelDefault.GetBoundBoxMax().z)
			colModel.SetBoundBoxMax(CVector(colModel.GetBoundBoxMax().x, colModel.GetBoundBoxMax().y, fMaxMovedColZ));
		if(fMinMovedColZ < colModelDefault.GetBoundBoxMin().z)
			colModel.SetBoundBoxMin(CVector(colModel.GetBoundBoxMin().x, colModel.GetBoundBoxMin().y, fMinMovedColZ));

#ifdef USE_AUDIOENGINE
		// don't use CPad::GetPad... stuff use (m_nSuspensionHydraulics - m_nOldSuspensionHydraulics) to determine movement
		//m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_BOUNCE);
#else //USE_AUDIOENGINE
		DMAudio.PlayOneShot(AudioHandle, AE_SLOW_SUSPENSION_MOVEMENT_LOOP, 0.3f*CPad::GetPad(0)->GetCarGunUpDown()/128.0f);
#endif //USE_AUDIOENGINE
		bMovedStuff = true;
	}
	else
	{
		m_nOldSuspensionHydraulics = m_nSuspensionHydraulics;
	}

#ifndef FINAL
	if(GetModelIndex()==MODELID_CAR_PACKER && CVehicle::m_bDisplayPerformanceInfo)
	{
		CVector vecRearVert(0.0f,1000.0f,0.0f);
		for(i=0; i<pColData->m_nNoOfTriangles; i++)
		{
			// index1
			DecompressTranslationVector(vecTempVertex, pColData->m_pTriCompressedVectorArray[pColData->m_pTriangleArray[i].m_nIndex1]);
			if(vecTempVertex.y < vecRearVert.y)
				vecRearVert = vecTempVertex;
			// index2
			DecompressTranslationVector(vecTempVertex, pColData->m_pTriCompressedVectorArray[pColData->m_pTriangleArray[i].m_nIndex2]);
			if(vecTempVertex.y < vecRearVert.y)
				vecRearVert = vecTempVertex;
			// index3
			DecompressTranslationVector(vecTempVertex, pColData->m_pTriCompressedVectorArray[pColData->m_pTriangleArray[i].m_nIndex3]);
			if(vecTempVertex.y < vecRearVert.y)
				vecRearVert = vecTempVertex;
		}
		vecRearVert = GetMatrix()*vecRearVert;
		CDebug::RenderDebugSphere3D(vecRearVert.x, vecRearVert.y, vecRearVert.z, 0.5f);
	}
#endif

	return bMovedStuff;
}


CVector CAutomobile::AddMovingCollisionSpeed(CVector &vecOffset)
{
	CVector vecResult(0.0f,0.0f,0.0f);
	if(GetStatus()==STATUS_PLAYER || GetStatus()==STATUS_PLAYER_DISABLED
	|| (VehicleCreatedBy==MISSION_VEHICLE && (m_nSuspensionHydraulics!=0 || m_nOldSuspensionHydraulics!=0)))
	{
		float fAngleMult = 0.0f;
		float fAngleSpeed = 0.0f;
		CVector vecPivot(0.0f,0.0f,0.0f);
		
		// if component has moved too fast, eg the script has switched it instantaneously
		if(m_nSuspensionHydraulics - m_nOldSuspensionHydraulics > 100
		|| m_nSuspensionHydraulics - m_nOldSuspensionHydraulics < -100)
			return vecResult;
		
		if(GetModelIndex()==MODELID_CAR_DUMPER && m_aCarNodes[TRUCK_MISC_A])
		{
			fAngleMult = CMonsterTruck::DUMPER_COL_ANGLEMULT;
			vecPivot = RwFrameGetMatrix(m_aCarNodes[TRUCK_MISC_A])->pos;
		}
		else if(GetModelIndex()==MODELID_CAR_PACKER)
		{
			fAngleMult = PACKER_COL_ANGLEMULT;
			vecPivot = PACKER_COL_PIVOT;
		}
		else if(GetModelIndex()==MODELID_CAR_DOZER && m_aCarNodes[CAR_MISC_A])
		{
			fAngleMult = DOZER_COL_ANGLEMULT;
			vecPivot = RwFrameGetMatrix(m_aCarNodes[CAR_MISC_A])->pos;
		}
		else if(GetModelIndex()==MODELID_PLANE_ANDROM && m_aCarNodes[PLANE_MISC_B])
		{
			fAngleMult = CPlane::ANDROM_COL_ANGLE_MULT;
			vecPivot = RwFrameGetMatrix(m_aCarNodes[PLANE_MISC_B])->pos;
		}
				
		if(GetModelIndex()==MODELID_CAR_FORKLIFT)
		{
			static float TEST_FORKLIFT_VEL_MULT = 2.0f;
			fAngleSpeed = (float)m_nSuspensionHydraulics - (float)m_nOldSuspensionHydraulics;
			fAngleSpeed *= TEST_FORKLIFT_VEL_MULT*FORKLIFT_MOVE_MULT/CTimer::GetTimeStep();
			vecResult = fAngleSpeed*GetMatrix().GetUp();
		}
		else if(fAngleMult != 0.0f)
		{
			fAngleSpeed = (float)m_nSuspensionHydraulics - (float)m_nOldSuspensionHydraulics;
			fAngleSpeed *= fAngleMult/CTimer::GetTimeStep();
			
			vecResult.x = fAngleSpeed;
			vecResult = Multiply3x3(GetMatrix(), vecResult);
			vecResult = CrossProduct(vecResult, vecOffset - Multiply3x3(GetMatrix(), vecPivot));
		}
	}
	return vecResult;
}

float CAutomobile::GetMovingCollisionOffset()
{
	if(m_nSuspensionHydraulics > 0)
	{
		if(GetModelIndex()==MODELID_CAR_DUMPER && m_aCarNodes[TRUCK_MISC_A])
		{
			return (CMonsterTruck::DUMPER_COL_ANGLEMULT*m_nSuspensionHydraulics);
		}
		else if(GetModelIndex()==MODELID_CAR_PACKER)
		{
			return (PACKER_COL_ANGLEMULT*m_nSuspensionHydraulics);
		}
		else if(GetModelIndex()==MODELID_CAR_DOZER && m_aCarNodes[CAR_MISC_A])
		{
			return (DOZER_COL_ANGLEMULT*m_nSuspensionHydraulics);
		}
		else if(GetModelIndex()==MODELID_PLANE_ANDROM && m_aCarNodes[PLANE_MISC_B])
		{
			return (CPlane::ANDROM_COL_ANGLE_MULT*m_nSuspensionHydraulics);
		}
		else if(GetModelIndex()==MODELID_CAR_FORKLIFT)
		{
			return (FORKLIFT_MOVE_MULT*m_nSuspensionHydraulics);
		}
	}
	
	return 0.0f;
}


//
//
// Return the position of a component
//
void CAutomobile::GetComponentWorldPosition(int32 component, CVector& posn)
{
	RwMatrix* pMatrix;
	
	ASSERTMSG(component < MAX_CAR_NODES, "component Id is too high");
	if(m_aCarNodes[component]!=NULL){
		pMatrix = RwFrameGetLTM(m_aCarNodes[component]);
		posn = *RwMatrixGetPos(pMatrix);
	}
//	else
//		printf("CarNode missing: %d %d\n", m_nModelIndex, component);
}


//
//
//
bool CAutomobile::IsComponentPresent(int32 component) const
{
	ASSERTMSG(component < MAX_CAR_NODES, "component Id is too high");
	if (m_aCarNodes[component] == NULL) return FALSE;
	return(TRUE);
}
	
/*
//
//
//
void CAutomobile::SetComponentRotation(int32 component , CVector rot) 
{
	CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[component]));
	CVector posn(matrix.GetTranslate());
	
	matrix.SetRotateX(DEGTORAD(rot.x));		
	matrix.SetRotateY(DEGTORAD(rot.y));		
	matrix.SetRotateZ(DEGTORAD(rot.z));		
	matrix.Translate(posn);
	matrix.UpdateRW();

}
*/

//
//
//
void CAutomobile::OpenDoor(CPed* pPed, int32 index, eDoors DoorID, float timeRatio, bool bPlaySoundSample) 
{
	if(0==m_aCarNodes[index])
	{
		ASSERTMSG(false,"Attempting to open door that doesn't exist");
		return;
	}
	
	CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
	CVector posn(matrix.GetTranslate());
	float orien[3] = {0.0f, 0.0f, 0.0f};
	bool bClosedAtStart = FALSE;
	
	if(Door[DoorID].IsClosed())
	{
		// ensure swinging doors aren't culled
		RwFrameForAllObjects(m_aCarNodes[index], &CVehicleModelInfo::ClearAtomicFlagCB, (void*)VEHICLE_ATOMIC_DONTCULL);
		// set this otherwise car accelleration will move the door and make it impossible to close
		bClosedAtStart = TRUE;
	}
	
	Door[DoorID].Open(timeRatio);

	if((bClosedAtStart)&&(Door[DoorID].RetDoorAngle() != Door[DoorID].RetAngleWhenClosed()))
	{
		HideAllComps();
		// ensure swinging doors can be culled
		RwFrameForAllObjects(m_aCarNodes[index], &CVehicleModelInfo::SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_DONTCULL);
		// door began to open here
		
		if(bPlaySoundSample)
		{
#ifdef USE_AUDIOENGINE
			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_CAR_BONNET_OPEN + DoorID);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, (AE_CAR_BONNET_OPEN + DoorID), 0.0f);
#endif //USE_AUDIOENGINE

			// add sound event when door is opened:
			if(pPed)
			{
				CEventSoundQuiet eventFootstepNoise(pPed, SOUNDVOL_MEDIUM);
				GetEventGlobalGroup()->Add(eventFootstepNoise);		
			}
		}
	}
	
	if((!bClosedAtStart)&&(timeRatio == 0.0f))
	{
		if(Damage.GetDoorStatus(DoorID)== DT_DOOR_SWINGING_FREE)
			Damage.SetDoorStatus(DoorID, DT_DOOR_INTACT);
		else if(Damage.GetDoorStatus(DoorID)== DT_DOOR_BASHED_AND_SWINGING_FREE)
			Damage.SetDoorStatus(DoorID, DT_DOOR_BASHED);
			
		ShowAllComps();
		// door shut here
		
		if(bPlaySoundSample)
		{
#ifdef USE_AUDIOENGINE
			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_CAR_BONNET_CLOSE + DoorID);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, (AE_CAR_BONNET_CLOSE + DoorID), 0.0f);
#endif //USE_AUDIOENGINE
		
			// add sound event when door is slammed shut:
			if(pPed)
			{
				CEventSoundQuiet eventFootstepNoise (pPed, SOUNDVOL_VERY_LOUD);  // bit louder for slamming door
				GetEventGlobalGroup()->Add(eventFootstepNoise);
			}
		}
	}

	orien[Door[DoorID].RetAxes()] = Door[DoorID].RetDoorAngle();

	matrix.SetRotate(orien[0], orien[1], orien[2]);		
	matrix.Translate(posn);
	matrix.UpdateRW();

}

float CAutomobile::GetDooorAngleOpenRatio(const eDoors DoorId) const
{
	return (Door[DoorId].GetAngleOpenRatio());
}


#ifdef USE_ANIM_GROUPS
/*
void CAutomobile::ProcessOpenDoor(uint32 nDoor, uint32 nAnimGroupId, uint32 nAnimId, float fCurrTime)
{
	eDoors DoorId;

	switch(nDoor)
	{
		case CAR_DOOR_LF:	DoorId = FRONT_LEFT_DOOR; break;
		case CAR_DOOR_LR:	DoorId = REAR_LEFT_DOOR; break;
		case CAR_DOOR_RF:	DoorId = FRONT_RIGHT_DOOR; break;
		case CAR_DOOR_RR:	DoorId = REAR_RIGHT_DOOR; break;
		default:			ASSERT(0);	break;
	}

	if(IsDoorMissing(DoorId))
		return;

	float fTimeToBeginOpenDoor;
	float fTimeToEndOpenDoor;
	float fDurationOpenDoor;
	float fTimeToBeginCloseDoor;
	float fTimeToEndCloseDoor;
	float fDurationCloseDoor;
	float fTimeRatio;
	
	switch(nAnimId)
	{
		case ANIM_VEH_OPEN_OUTSIDE_FRONT_LHS:
		case ANIM_VEH_OPEN_OUTSIDE_FRONT_RHS:
		case ANIM_VEH_OPEN_OUTSIDE_REAR_LHS:
		case ANIM_VEH_OPEN_OUTSIDE_REAR_RHS:	
		
			CVehicleAnimGroupData::ComputeProcessDoorTimes(pHandling->AnimGroup,CVehicleAnimGroup::DOOR_TIME_OPEN_OUTSIDE,fTimeToBeginOpenDoor,fTimeToEndOpenDoor);
			
			if((fCurrTime > fTimeToBeginOpenDoor) && (fCurrTime < fTimeToEndOpenDoor))
			{ 
				fDurationOpenDoor = fTimeToEndOpenDoor - fTimeToBeginOpenDoor;
				fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor) / fDurationOpenDoor;

				if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
				{
					OpenDoor(nDoor, DoorId, fTimeRatio);
				}
			}
			else if(fCurrTime > fTimeToEndOpenDoor)
			{
				OpenDoor(nDoor, DoorId, 1.0f);
			}
			else if(fCurrTime < fTimeToBeginOpenDoor)
			{
			    OpenDoor(nDoor, DoorId, 0.0f);
			}			
			break;		
		
		case ANIM_VEH_CLOSE_INSIDE_FRONT_LHS:
		case ANIM_VEH_CLOSE_INSIDE_FRONT_RHS:
		case ANIM_VEH_CLOSE_INSIDE_REAR_LHS:
		case ANIM_VEH_CLOSE_INSIDE_REAR_RHS:
			
			CVehicleAnimGroupData::ComputeProcessDoorTimes(pHandling->AnimGroup,CVehicleAnimGroup::DOOR_TIME_CLOSE_INSIDE,fTimeToBeginCloseDoor,fTimeToEndCloseDoor);

			if((fCurrTime > fTimeToBeginCloseDoor) && (fCurrTime < fTimeToEndCloseDoor))
			{ 
				fDurationCloseDoor = fTimeToEndCloseDoor - fTimeToBeginCloseDoor;
				fTimeRatio = 1.0f-((fCurrTime-fTimeToBeginCloseDoor) / fDurationCloseDoor);
		
				if(Door[DoorId].GetAngleOpenRatio() > fTimeRatio) 
					OpenDoor(nDoor, DoorId, fTimeRatio);
			}
			else if(fCurrTime > fTimeToEndCloseDoor)
			{
				OpenDoor(nDoor, DoorId, 0.0f);
			}
			else if(fCurrTime < fTimeToBeginCloseDoor)
			{
			    OpenDoor(nDoor, DoorId, 1.0f);
			}
			break;

		case ANIM_VEH_GETOUT_FRONT_LHS:
		case ANIM_VEH_GETOUT_FRONT_RHS:
		case ANIM_VEH_GETOUT_REAR_LHS:
		case ANIM_VEH_GETOUT_REAR_RHS:	
			
			CVehicleAnimGroupData::ComputeProcessDoorTimes(pHandling->AnimGroup,CVehicleAnimGroup::DOOR_TIME_OPEN_INSIDE,fTimeToBeginOpenDoor,fTimeToEndOpenDoor);

			if((fCurrTime > fTimeToBeginOpenDoor) && (fCurrTime < fTimeToEndOpenDoor))
			{ 
				fDurationOpenDoor = fTimeToEndOpenDoor - fTimeToBeginOpenDoor;
				fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor) / fDurationOpenDoor;
	
				if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
				{
					OpenDoor(nDoor, DoorId, fTimeRatio);
				}
			}
			else if(fCurrTime > fTimeToEndOpenDoor)
			{
				OpenDoor(nDoor, DoorId, 1.0f);
			}
			else if(fCurrTime < fTimeToBeginOpenDoor)
			{
			    OpenDoor(nDoor, DoorId, 0.0f);
			}
			break;
		
		case ANIM_VEH_JACKED_LHS:
		case ANIM_VEH_JACKED_RHS:
		
		 	fTimeToBeginOpenDoor = 0.10f;// need to adjust
			fTimeToEndOpenDoor = 0.40f;
				
			if((fCurrTime > fTimeToBeginOpenDoor) && (fCurrTime < fTimeToEndOpenDoor))
			{ 
				fDurationOpenDoor = fTimeToEndOpenDoor - fTimeToBeginOpenDoor;
				fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor) / fDurationOpenDoor;
	
				OpenDoor(nDoor, DoorId, fTimeRatio);
			}
			else if(fCurrTime > fTimeToEndOpenDoor)
			{
				OpenDoor(nDoor, DoorId, 1.0f);
			}
			else if(fCurrTime < fTimeToBeginOpenDoor)
			{
			    OpenDoor(nDoor, DoorId, 0.0f);
			}
			break;
	
		case ANIM_VEH_CLOSE_OUTSIDE_FRONT_LHS:
		case ANIM_VEH_CLOSE_OUTSIDE_FRONT_RHS:
		case ANIM_VEH_CLOSE_OUTSIDE_REAR_LHS:
		case ANIM_VEH_CLOSE_OUTSIDE_REAR_RHS:	
			
			CVehicleAnimGroupData::ComputeProcessDoorTimes(pHandling->AnimGroup,CVehicleAnimGroup::DOOR_TIME_CLOSE_OUTSIDE,fTimeToBeginCloseDoor,fTimeToEndCloseDoor);

			if((fCurrTime > fTimeToBeginCloseDoor) && (fCurrTime < fTimeToEndCloseDoor))
			{ 
				fDurationCloseDoor = fTimeToEndCloseDoor - fTimeToBeginCloseDoor;
				fTimeRatio = 1.0f-((fCurrTime-fTimeToBeginCloseDoor) / fDurationCloseDoor);
	
				OpenDoor(nDoor, DoorId, fTimeRatio);
	
			}
			else if(fCurrTime > fTimeToEndCloseDoor)
			{
				OpenDoor(nDoor, DoorId, 0.0f);
			}
			else if(fCurrTime < fTimeToBeginCloseDoor)
			{
			    OpenDoor(nDoor, DoorId, 1.0f);
			}
			break;

		case ANIM_VEH_PULLOUT_LHS:
		case ANIM_VEH_PULLOUT_RHS:
		case ANIM_VEH_PULLOUT_WINDOW:
			
			switch(nAnimGroupId)
			{			
				case ANIM_VEH_STD:
				case ANIM_VEH_LOW:
				case ANIM_VEH_TRK:
				case ANIM_VEH_COACH:
				case ANIM_VEH_BUS:
				case ANIM_VEH_CONVERTIBLE:
					// door should be open
					OpenDoor(nDoor, DoorId, 1.0f);
					break;
					
				default:
					ASSERT(false);
					break;
			}
			break;

	    case ANIM_VEH_JUMPOUT_LHS:
		case ANIM_VEH_JUMPOUT_RHS:

			switch(nAnimGroupId)
			{
				case ANIM_VEH_STD:
				case ANIM_VEH_LOW:
				case ANIM_VEH_TRK:
				case ANIM_VEH_COACH:
				case ANIM_VEH_RUSTLER:	
				case ANIM_VEH_CONVERTIBLE:		
//					fTimeToBeginOpenDoor = 0.1f;
//					fTimeToEndOpenDoor = 0.4f;					
					fTimeToBeginOpenDoor = 0.01f;
					fTimeToEndOpenDoor = 0.1f;					
					break;		
				default:
					ASSERT(false);
					break;	
			}
			if((fCurrTime > fTimeToBeginOpenDoor) && (fCurrTime < fTimeToEndOpenDoor))
			{ 
				fDurationOpenDoor = fTimeToEndOpenDoor - fTimeToBeginOpenDoor;
				fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor) / fDurationOpenDoor;
	
				if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
				{
					OpenDoor(nDoor, DoorId, fTimeRatio);
				}
///						OpenDoor(nDoor, DoorId, fTimeRatio);
			}
			else if(fCurrTime > fTimeToEndOpenDoor)
			{
				OpenDoor(nDoor, DoorId, 1.0f);
			}
			else if(fCurrTime < fTimeToBeginOpenDoor)
			{
			    OpenDoor(nDoor, DoorId, 0.0f);
			}		
			break;
			
		case ANIM_VEH_CLOSE_DOOR_ROLLING:
			
			switch(nAnimGroupId)
			{
				case ANIM_VEH_STD:
				case ANIM_VEH_LOW:
				case ANIM_VEH_TRK:
				case ANIM_VEH_COACH:		
				case ANIM_VEH_RUSTLER:		
				case ANIM_VEH_CONVERTIBLE:	
					fTimeToBeginOpenDoor = 0.05f;//0.10f;//0.41f;opening as hand goes out
					fTimeToEndOpenDoor = 0.20f;//0.40f;//0.89f;	
					fTimeToBeginCloseDoor = 0.30f;//0.60f;//0.30f;// closing as hand goes in
					fTimeToEndCloseDoor = 0.475f;//0.95f;//0045f;		
					break;		
				default:
					ASSERT(false);
					break;			
			}
			if((fCurrTime < fTimeToBeginCloseDoor) && (fCurrTime > fTimeToBeginOpenDoor))
			{ 
				fDurationOpenDoor = fTimeToEndOpenDoor - fTimeToBeginOpenDoor;
				fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor) / fDurationOpenDoor;
		
				if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
					OpenDoor(nDoor, DoorId, fTimeRatio);
			}
			else if((fCurrTime > fTimeToBeginCloseDoor) && (fCurrTime < fTimeToEndCloseDoor))
			{ 
				fDurationCloseDoor = fTimeToEndCloseDoor - fTimeToBeginCloseDoor;
				fTimeRatio = 1.0f-((fCurrTime-fTimeToBeginCloseDoor) / fDurationCloseDoor);
		
				if(Door[DoorId].GetAngleOpenRatio() > fTimeRatio) 
					OpenDoor(nDoor, DoorId, fTimeRatio);
			}
			else if(fCurrTime > fTimeToEndCloseDoor)
			{
				OpenDoor(nDoor, DoorId, 0.0f);
			}
			break;
		
		case ANIM_VEH_FALL_OUT_LHS:
		case ANIM_VEH_FALL_OUT_RHS:

			fTimeToBeginOpenDoor = 0.10f;// need to adjust
			fTimeToEndOpenDoor = 0.40f;
				
			if((fCurrTime > fTimeToBeginOpenDoor) && (fCurrTime < fTimeToEndOpenDoor))
			{ 
				fDurationOpenDoor = fTimeToEndOpenDoor - fTimeToBeginOpenDoor;
				fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor) / fDurationOpenDoor;
	
				OpenDoor(nDoor, DoorId, fTimeRatio);
			}
			else if(fCurrTime > fTimeToEndOpenDoor)
			{
				OpenDoor(nDoor, DoorId, 1.0f);
			}
			else if(fCurrTime < fTimeToBeginOpenDoor)
			{
			    OpenDoor(nDoor, DoorId, 0.0f);
			}
			break;
									
		default:
			ASSERT(false);
			break;
	}
}
*/
#else
//
//
//
void CAutomobile::ProcessOpenDoor(uint32 nDoor, uint32 nAnimID, float fCurrTime)
{
	eDoors DoorId;

//	uint32 nAnimID = pPed->m_pAnim->GetAnimId();
//	float fCurrTime = pPed->m_pAnim->GetCurrentTime();
	switch(nDoor)
	{
		case CAR_DOOR_LF:	DoorId = FRONT_LEFT_DOOR; break;
		case CAR_DOOR_LR:	DoorId = REAR_LEFT_DOOR; break;
		case CAR_DOOR_RF:	DoorId = FRONT_RIGHT_DOOR; break;
		case CAR_DOOR_RR:	DoorId = REAR_RIGHT_DOOR; break;
		default:			ASSERT(0);	break;
	}

	if(IsDoorMissing(DoorId))
		return;

//	float fTimeToBeginOpenDoor;
//	float fTimeToEndOpenDoor;
	float fDurationOpenDoor;
//	float fTimeToBeginCloseDoor;
//	float fTimeToEndCloseDoor;
	float fDurationCloseDoor;
	float fTimeRatio;
		
	switch (nAnimID)
	{
		case ANIM_STD_COACH_GET_OUT_LHS:
//		case ANIM_STD_COACH_GET_OUT_RHS:
					static float fTimeToBeginOpenDoor1 = 0.0f;
					static float fTimeToEndOpenDoor1 = 0.3f;
					if((fCurrTime > fTimeToBeginOpenDoor1) && (fCurrTime < fTimeToEndOpenDoor1))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor1 - fTimeToBeginOpenDoor1;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor1) / fDurationOpenDoor;
						OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndOpenDoor1)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor1)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					break;

		case ANIM_STD_VAN_OPEN_DOOR_REAR_LHS:
		case ANIM_STD_VAN_OPEN_DOOR_REAR_RHS:
					
					static float fTimeToBeginOpenDoor2 = 0.37f;
					static float fTimeToEndOpenDoor2 = 0.55f;
				
					if((fCurrTime > fTimeToBeginOpenDoor2) && (fCurrTime < fTimeToEndOpenDoor2))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor2 - fTimeToBeginOpenDoor2;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor2) / fDurationOpenDoor;
						OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndOpenDoor2)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor2)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					
					break;
		case ANIM_STD_QUICKJACK:
		case ANIM_STD_CAR_OPEN_DOOR_LHS:
		case ANIM_STD_CAR_OPEN_DOOR_RHS:

					static float fTimeToBeginOpenDoor3 = 0.50f;//0.66f;
					static float fTimeToEndOpenDoor3 = 0.80f;
				
					if((fCurrTime > fTimeToBeginOpenDoor3) && (fCurrTime < fTimeToEndOpenDoor3))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor3 - fTimeToBeginOpenDoor3;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor3) / fDurationOpenDoor;
			
						if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
						{
							OpenDoor(nDoor, DoorId, fTimeRatio);
						}
///						OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndOpenDoor3)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor3)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					break;		
	    
	    case ANIM_STD_ROLLOUT_LHS:
		case ANIM_STD_ROLLOUT_RHS:

					static float fTimeToBeginOpenDoor4 = 0.1f;
					static float fTimeToEndOpenDoor4 = 0.4f;
				
					if((fCurrTime > fTimeToBeginOpenDoor4) && (fCurrTime < fTimeToEndOpenDoor4))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor4 - fTimeToBeginOpenDoor4;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor4) / fDurationOpenDoor;
			
						if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
						{
							OpenDoor(nDoor, DoorId, fTimeRatio);
						}
///						OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndOpenDoor4)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor4)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					break;		
		
		case ANIM_STD_CAR_PULL_OUT_PED_RHS:
		case ANIM_STD_CAR_PULL_OUT_PED_LO_RHS:
		
					// door should be open
					OpenDoor(nDoor, DoorId, 1.0f);
					break;

				
		case ANIM_STD_COACH_OPEN_LHS:
		case ANIM_STD_COACH_OPEN_RHS:

					static float fTimeToBeginOpenDoor5 = 0.66f;//0.41f;
					static float fTimeToEndOpenDoor5 = 0.80f;//0.89f;
				
					if((fCurrTime > fTimeToBeginOpenDoor5) && (fCurrTime < fTimeToEndOpenDoor5))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor5 - fTimeToBeginOpenDoor5;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor5) / fDurationOpenDoor;
			
						OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndOpenDoor5)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor5)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					break;		

		case ANIM_STD_CAR_CLOSE_DOOR_LHS:
		case ANIM_STD_CAR_CLOSE_DOOR_LO_LHS:
		case ANIM_STD_CAR_CLOSE_DOOR_RHS:
		case ANIM_STD_CAR_CLOSE_DOOR_LO_RHS:

					static float fTimeToBeginCloseDoor6 = 0.25f;
					static float fTimeToEndCloseDoor6 = 0.50f;
				
					if((fCurrTime > fTimeToBeginCloseDoor6) && (fCurrTime < fTimeToEndCloseDoor6))
					{ 
						fDurationCloseDoor = fTimeToEndCloseDoor6 - fTimeToBeginCloseDoor6;
						fTimeRatio = 1.0f-((fCurrTime-fTimeToBeginCloseDoor6) / fDurationCloseDoor);
				
						if(Door[DoorId].GetAngleOpenRatio() > fTimeRatio) 
							OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndCloseDoor6)
					{
						OpenDoor(nDoor, DoorId, 0.0f);
					}
					else if(fCurrTime < fTimeToBeginCloseDoor6)
					{
					    OpenDoor(nDoor, DoorId, 1.0f);
					}
					break;

		case ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS:
		case ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS:

					static float fTimeToBeginOpenDoor7 = 0.05f;//0.10f;//0.41f;opening as hand goes out
					static float fTimeToEndOpenDoor7 = 0.20f;//0.40f;//0.89f;
					
					static float fTimeToBeginCloseDoor7 = 0.30f;//0.60f;//0.30f;// closing as hand goes in
					static float fTimeToEndCloseDoor7 = 0.475f;//0.95f;//0045f;
				
					if((fCurrTime < fTimeToBeginCloseDoor7) && (fCurrTime > fTimeToBeginOpenDoor7))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor7 - fTimeToBeginOpenDoor7;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor7) / fDurationOpenDoor;
				
						if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
							OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if((fCurrTime > fTimeToBeginCloseDoor7) && (fCurrTime < fTimeToEndCloseDoor7))
					{ 
						fDurationCloseDoor = fTimeToEndCloseDoor7 - fTimeToBeginCloseDoor7;
						fTimeRatio = 1.0f-((fCurrTime-fTimeToBeginCloseDoor7) / fDurationCloseDoor);
				
						if(Door[DoorId].GetAngleOpenRatio() > fTimeRatio) 
							OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndCloseDoor7)
					{
						OpenDoor(nDoor, DoorId, 0.0f);
					}
					break;
		
		case ANIM_STD_GETOUT_LHS:
		case ANIM_STD_GETOUT_LO_LHS:
		case ANIM_STD_GETOUT_RHS:
		case ANIM_STD_GETOUT_LO_RHS:

					static float fTimeToBeginOpenDoor8 = 0.15f;//0.0f;// need to adjust
					static float fTimeToEndOpenDoor8 = 0.50f;//0.50f;
				
					if((fCurrTime > fTimeToBeginOpenDoor8) && (fCurrTime < fTimeToEndOpenDoor8))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor8 - fTimeToBeginOpenDoor8;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor8) / fDurationOpenDoor;
			
						if(Door[DoorId].GetAngleOpenRatio() < fTimeRatio) 
						{
							OpenDoor(nDoor, DoorId, fTimeRatio);
						}
					}
					else if(fCurrTime > fTimeToEndOpenDoor8)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor8)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					break;

		case ANIM_STD_CAR_CLOSE_LHS:
		case ANIM_STD_CAR_CLOSE_RHS:

					static float fTimeToBeginCloseDoor9 = 0.25f;//0.21f;// need to adjust
					static float fTimeToEndCloseDoor9 = 0.40f;//0.40f;
				
					if((fCurrTime > fTimeToBeginCloseDoor9) && (fCurrTime < fTimeToEndCloseDoor9))
					{ 
						fDurationCloseDoor = fTimeToEndCloseDoor9 - fTimeToBeginCloseDoor9;
						fTimeRatio = 1.0f-((fCurrTime-fTimeToBeginCloseDoor9) / fDurationCloseDoor);
			
						OpenDoor(nDoor, DoorId, fTimeRatio);
			
					}
					else if(fCurrTime > fTimeToEndCloseDoor9)
					{
						OpenDoor(nDoor, DoorId, 0.0f);
					}
					else if(fCurrTime < fTimeToBeginCloseDoor9)
					{
					    OpenDoor(nDoor, DoorId, 1.0f);
					}
					break;

		case ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS:
		case ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS:

					static float fTimeToBeginCloseDoor10 = 0.50f;// need to adjust
					static float fTimeToEndCloseDoor10 = 0.80f;
				
					if((fCurrTime > fTimeToBeginCloseDoor10) && (fCurrTime < fTimeToEndCloseDoor10))
					{ 
						fDurationCloseDoor = fTimeToEndCloseDoor10 - fTimeToBeginCloseDoor10;
						fTimeRatio = 1.0f-((fCurrTime-fTimeToBeginCloseDoor10) / fDurationCloseDoor);
			
						OpenDoor(nDoor, DoorId, fTimeRatio);
			
					}
					else if(fCurrTime > fTimeToEndCloseDoor10)
					{
						OpenDoor(nDoor, DoorId, 0.0f);
					}
					else if(fCurrTime < fTimeToBeginCloseDoor10)
					{
					    OpenDoor(nDoor, DoorId, 1.0f);
					}
					break;

		case ANIM_STD_VAN_GET_OUT_REAR_LHS:
		case ANIM_STD_VAN_GET_OUT_REAR_RHS:

					static float fTimeToBeginOpenDoor11 = 0.50f;// need to adjust
					static float fTimeToEndOpenDoor11 = 0.60f;
				
					if((fCurrTime > fTimeToBeginOpenDoor11) && (fCurrTime < fTimeToEndOpenDoor11))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor11 - fTimeToBeginOpenDoor11;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor11) / fDurationOpenDoor;
			
						OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndOpenDoor11)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor11)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					break;

        case ANIM_STD_JACKEDCAR_LO_LHS:
        case ANIM_STD_JACKEDCAR_LHS:
        case ANIM_STD_JACKEDCAR_LO_RHS:
        case ANIM_STD_JACKEDCAR_RHS:
            
                    static float fTimeToBeginOpenDoor12 = 0.10f;// need to adjust
					static float fTimeToEndOpenDoor12 = 0.40f;
				
					if((fCurrTime > fTimeToBeginOpenDoor12) && (fCurrTime < fTimeToEndOpenDoor12))
					{ 
						fDurationOpenDoor = fTimeToEndOpenDoor12 - fTimeToBeginOpenDoor12;
						fTimeRatio = (fCurrTime-fTimeToBeginOpenDoor12) / fDurationOpenDoor;
			
						OpenDoor(nDoor, DoorId, fTimeRatio);
					}
					else if(fCurrTime > fTimeToEndOpenDoor12)
					{
						OpenDoor(nDoor, DoorId, 1.0f);
					}
					else if(fCurrTime < fTimeToBeginOpenDoor12)
					{
					    OpenDoor(nDoor, DoorId, 0.0f);
					}
					
					break;

		case ANIM_STD_NUM:
					OpenDoor(nDoor, DoorId, fCurrTime);
					break;
		
	}
	

}
#endif


bool CAutomobile::IsDoorReady(eDoors DoorID) const
{
	if(Door[DoorID].IsClosed() || IsDoorMissing(DoorID))
	{
		return(TRUE);
	}
	else
	{
	    return (FALSE);
	}
}


bool CAutomobile::IsDoorFullyOpen(eDoors DoorID) const
{
	if(Door[DoorID].IsFullyOpen() || IsDoorMissing(DoorID))
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

bool CAutomobile::IsDoorClosed(eDoors DoorID) const
{
	if(Door[DoorID].IsClosed())
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}


bool CAutomobile::IsDoorMissing(eDoors DoorID) const
{
	uint8 Status = Damage.GetDoorStatus(DoorID);

	if ((Status == DT_DOOR_MISSING))
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

bool CAutomobile::IsOpenTopCar() const
{
	if( (GetModelIndex()==MODELID_CAR_COMET && m_comp1!=0 && m_comp2!=0)
	||	(GetModelIndex()==MODELID_CAR_STALLION && m_comp1!=0 && m_comp2!=0) )
//	||	GetModelIndex()==MODELID_CAR_STINGER )
		return true;

	return false;
}



//
// helper routine for dmgDrawCarCollidingParticles():
//
/*
inline
CVector	dccpGetRandomSmokeStartPosition(const CVector &DamagePos)
{
		CVector pos = DamagePos;
		//pos.x += CGeneral::GetRandomNumberInRange(-0.8f, 0.8f);
		//pos.y += CGeneral::GetRandomNumberInRange(-0.8f, 0.8f);
		pos.x += CGeneral::GetRandomNumberInRange(-0.3f, 0.3f);
		pos.y += CGeneral::GetRandomNumberInRange(-0.3f, 0.32f);
		//pos.z += 0.25f;
		return(pos);
}
*/

//
// helper routine for dmgDrawCarCollidingParticles():
//
/*
inline
CVector	dccpGetRandomSparkSpeed()
{
		CVector	speed;
		speed.x = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
		speed.y = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
		speed.z	= 0.006f;
		return(speed);
}
*/

//
//
// used in CAutomobile::VehicleDamage():
//
void CAutomobile::dmgDrawCarCollidingParticles(const CVector& DamagePosition, float fImpulse, eWeaponType weaponType)
{
	// car collision
	if (this->GetIsOnScreen())
	{	
		// MN - FX SYSTEM -------------------------------
		// car collision
		int32 iImpulse = int32(fImpulse);
	
		// sparks
		if (weaponType == WEAPONTYPE_UNARMED ||
			weaponType == WEAPONTYPE_FLOWERS)
		{
		}
		else
		{
//			CVector vel = -10.0f * m_vecMoveSpeed;//dccpGetRandomSparkSpeed();
			CVector sparkDir = m_vecMoveSpeed;
			float sparkSpeed = sparkDir.NormaliseAndMag();
			sparkSpeed *= -10.0f;
			CVector pos = DamagePosition;
			g_fx.AddSparks(pos, sparkDir, sparkSpeed, (4+iImpulse/10) & 63, m_vecMoveSpeed, true, 0.3f);
		}
				
		// dust		  	  	red   green blue  alpha size  rot   life		
		FxPrtMult_c fxMults(0.4f, 0.4f, 0.4f, 0.6f, 0.4f, 1.0f, 0.1f); 
//		FxPrtMult_c fxMults(1.0f, 0.0f, 0.0f, 0.6f, 0.3f, 1.0f, 0.1f); 
		
		CVector vel(0.0f, 0.0f, 0.0f);
		
		// bring the damage pos slightly closer to the center of the car
		// so we can see the dust and debris particles better
		CVector carToDamage = DamagePosition - GetPosition();
		CVector pos = GetPosition() + carToDamage*0.7f;

//		RwV3d pos = DamagePosition;//(RwV3d)dccpGetRandomSmokeStartPosition(DamagePosition);

		CVector moveVec = m_vecMoveSpeed * CTimer::GetTimeStep();
		float vehSpeed = moveVec.Magnitude();
		int32 numParticles = MAX(1, (int32)(vehSpeed*4.0f));
		
		for (int32 i=0; i<numParticles; i++)
		{
			CVector pos2 = pos - (moveVec*(1.0f-(i/(float)numParticles)));
			g_fx.m_fxSysSmokeHuge->AddParticle(&pos, &vel, 0.0f, &fxMults);	
		}

		// debris
		if (m_vecMoveSpeed.MagnitudeSqr()>0.25f*0.25f)
		{
			RwRGBA col = CVehicleModelInfo::GetVehicleColour(m_colour1);
			col.red *= m_lightingFromCollision;
			col.green *= m_lightingFromCollision;
			col.blue *= m_lightingFromCollision;
			g_fx.AddDebris(pos, col, 0.06f, 1+(iImpulse/100)); 
		}
	}	
}


void CAutomobile::ProcessCarOnFireAndExplode(bool8 forceExplode)
{
	uint8 EngineDamage = Damage.GetEngineStatus();
/*	CVector DamagePosition(((CVehicleModelInfo*)CModelInfo::GetModelInfo(this->GetModelIndex()))->GetPosnOnVehicle(VEHICLE_HEADLIGHTS_POSN));

	//
	// offset for engine position is not constant:
	//
	switch(Damage.GetDoorStatus(BONNET))
	{
		// small damage: bonnet still in place, we render smokes just at the edge of engine:
		case(DT_DOOR_INTACT):
		case(DT_DOOR_BASHED):
			DamagePosition += vecDAMAGE_ENGINE_POS_SMALL;
			break;
		// big damage: bonner is lost or swinging, we render smoke in the centre of engine:
		case(DT_DOOR_SWINGING_FREE):
		case(DT_DOOR_MISSING):
			DamagePosition += vecDAMAGE_ENGINE_POS_BIG;
			break;
	}


	//
	// if we are in FirstPerson mode, and car is on fire, make it visible for player:
	// (in this camera view, we move it a bit forward):
	//
	if((this==FindPlayerVehicle()) && (TheCamera.GetLookingForwardFirstPerson()))
	{
		if(	(this->m_nHealth < CAR_ON_FIRE_HEALTH) && (this->GetStatus() != STATUS_WRECKED) )
		{
			if (this->GetModelIndex() == MODELID_CAR_FIRETRUCK)
			{		// Special case for firetruck (Move point forward a little bit)
				DamagePosition += CVector(0.0f, 3.0f, -0.2f);
			}
			else
			{		// Move point forward a little bit
				DamagePosition += CVector(0.0f, 1.2f, -0.8f);
			}
		}
	}
	
//	DamagePosition = GetMatrix() * DamagePosition;
	DamagePosition.z += 0.15f;//0.3f;
*/	
	// if general health below certain level fire comes out of engine
	if(m_nHealth < CAR_ON_FIRE_HEALTH && GetStatus() != STATUS_WRECKED && !m_nVehicleFlags.bIsDrowning)
	{
		int32 triggerFire = 0;
	
		if (m_fxSysEngFire==NULL && (GetVehicleType()!=VEHICLE_TYPE_PLANE && GetVehicleType()!=VEHICLE_TYPE_HELI))
		{
			triggerFire = 1;
		}

#ifdef GTA_NETWORK
		if (gGameNet.NetWorkStatus != NETSTAT_CLIENTSTARTINGUP && gGameNet.NetWorkStatus != NETSTAT_CLIENTRUNNING)
#endif
		{
			if (GetVehicleType()==VEHICLE_TYPE_PLANE || GetVehicleType()==VEHICLE_TYPE_HELI)
			{
				if (m_statusWhenEngineBlown==EBS_NOT_SET)
				{
					// hasn't been set - set it
					if (IsInAir())
					{
						m_statusWhenEngineBlown = EBS_IN_AIR;
					}
					else
					{
						m_statusWhenEngineBlown = EBS_ON_GROUND;
					}
				}
			
				if (forceExplode)
				{
					BlowUpCar(pEntityThatSetUsOnFire);
				}
				else 
				{
					bool8 isRC = false;
					if (GetModelIndex()==MODELID_CAR_RCBARON || GetModelIndex()==MODELID_CAR_RCCOPTER || GetModelIndex()==MODELID_CAR_RCGOBLIN ||
						GetModelIndex()==MODELID_CAR_RCBANDIT || GetModelIndex()==MODELID_CAR_RCTIGER || GetModelIndex()==MODELID_CAR_REMOTE)
					{
						isRC = true;
					}
				
					// blow up slower than usual if in air when fucked
					if (m_statusWhenEngineBlown==EBS_ON_GROUND || isRC)
					{
						this->m_BlowUpTimer += CTimer::GetTimeElapsedInMilliseconds();
					}
					else
					{				
						this->m_BlowUpTimer += CTimer::GetTimeElapsedInMilliseconds()*0.2f;
					}

					if (this->m_BlowUpTimer > DT_ENGINE_BLOW_UP_TIME * 1000 || m_nVehicleFlags.bIsDrowning)
					{
						BlowUpCar(pEntityThatSetUsOnFire);
					}
					else
					{
						//			 	    red   green blue  alpha size  rot   life
						FxPrtMult_c fxMults(0.0f, 0.0f, 0.0f, 0.4f, 1.0f, 1.0f, 0.3f);	
						
						if (isRC)
						{
							if (GetStatus()==STATUS_PLAYER_REMOTE)
							{
								fxMults.m_alpha = 0.5f;
								fxMults.m_size = 0.1f;
								fxMults.m_life = 0.1f;
							}
							else
							{
								fxMults.m_alpha = 0.15f;
								fxMults.m_size = 0.3f;
								fxMults.m_life = 0.3f;
							}
						}	
									
						if (CTimer::m_FrameCounter%2==0)
						{
							CVector	pos = GetPosition();
							CVector	vel;
							
							if (isRC)
							{
								vel.x = CGeneral::GetRandomNumberInRange(-0.5f, 0.5f);
								vel.y = CGeneral::GetRandomNumberInRange(-0.5f, 0.5f);
								vel.z = CGeneral::GetRandomNumberInRange(0.0f, 0.4f);
								pos.x += CGeneral::GetRandomNumberInRange(-0.7f, 0.7f);
								pos.y += CGeneral::GetRandomNumberInRange(-0.7f, 0.7f);
							}
							else
							{
								vel.x = CGeneral::GetRandomNumberInRange(-1.5f, 1.5f);
								vel.y = CGeneral::GetRandomNumberInRange(-1.5f, 1.5f);
								vel.z = CGeneral::GetRandomNumberInRange(0.0f, 1.0f);
								pos.x += CGeneral::GetRandomNumberInRange(-2.0f, 2.0f);
								pos.y += CGeneral::GetRandomNumberInRange(-2.0f, 2.0f);
							}
							g_fx.m_fxSysSmokeHuge->AddParticle(&pos, &vel, 0.0f, &fxMults);	
						}	
					}
					
					// add random explosions to out of control ai planes/helis
					if (GetStatus()!=STATUS_PLAYER && GetStatus()!=STATUS_PLAYER_REMOTE)
					{
						if (GetModelIndex()!=MODELID_CAR_RCBARON && GetModelIndex()!=MODELID_CAR_RCCOPTER && GetModelIndex()!=MODELID_CAR_RCGOBLIN && 
						GetModelIndex()!=MODELID_CAR_RCBANDIT && GetModelIndex()!=MODELID_CAR_RCTIGER && GetModelIndex()!=MODELID_CAR_REMOTE)
						{
							if (CGeneral::GetRandomNumberInRange(0, 250)<3)
							{	
								CExplosion::AddExplosion(this, pEntityThatSetUsOnFire, EXP_TYPE_ROCKET, this->GetPosition());
							}
						}	
					}
					else
					{
						if (m_fxSysEngFire==NULL && m_BlowUpTimer>DT_ENGINE_BLOW_UP_TIME*1000*0.5f)
						{
							triggerFire = 2;
						}
					}
				}
			}
			else
			{
				this->m_BlowUpTimer += CTimer::GetTimeElapsedInMilliseconds();
				if (this->m_BlowUpTimer > DT_ENGINE_BLOW_UP_TIME * 1000)
				{
//					CWorld::Players[CWorld::PlayerInFocus].AwardMoneyForExplosion(this);	
					BlowUpCar(pEntityThatSetUsOnFire);
				}
			}
		}
		
		// MN - FX SYSTEM -------------------------------
		// car on fire
		if (triggerFire)
		{
			// car engine fire triggered
			CVehicleModelInfo *pModelInfo = (CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex());
			CVector engineOffset = pModelInfo->GetPosnOnVehicle(VEHICLE_ENGINE_POSN);
			
			RwMatrix* pParentMat = GetRwMatrix();

			if (pParentMat)
			{
				if (triggerFire==1)
				{
					m_fxSysEngFire = g_fxMan.CreateFxSystem("fire_car", (RwV3d*)&engineOffset, pParentMat);
				}
				else if (triggerFire==2)
				{
					m_fxSysEngFire = g_fxMan.CreateFxSystem("fire_large", (RwV3d*)&engineOffset, pParentMat);
				}
			}
			
			if (m_fxSysEngFire)
			{
				m_fxSysEngFire->Play();
				CEventVehicleOnFire event(this);
				GetEventGlobalGroup()->Add(event);
			}
		}
		
		if (m_fxSysEngFire)
		{
			RwV3d velAdd = m_vecMoveSpeed*50.0f;
			m_fxSysEngFire->SetVelAdd(&velAdd);
		}
		// ----------------------------------------------
	}
	else
	{
		m_BlowUpTimer=0;
		
		// MN - FX SYSTEM -------------------------------
		if (m_fxSysEngFire != NULL)
		{
			// stop the car engine fire
			m_fxSysEngFire->Kill();
			m_fxSysEngFire = NULL;
		}
		// ----------------------------------------------
	}
	
	if(EngineDamage > DT_ENGINE_ON_FIRE && m_nHealth > CAR_ON_FIRE_HEALTH)
	{
		// decrease overall vehicle health until fire starts
		m_nHealth -= 2.0f;	// 2 is probably quite a lot per frame?
	}
	
	ProcessDelayedExplosion();
}



//
//
//
#define VEHICLEDAMAGEMULTIPLIER (0.6f)	// Try to decrease the damage a wee bit. (was too easy to blow up)
#define CAR_DAMAGE_THRESHOLD (25.0f)
#define CAR_DAMAGE_SCALE_MASS (1500.0f)

#define FRONT_DAMAGE_BONNET 	(0x01)
#define FRONT_DAMAGE_WSCREEN 	(0x02)
#define FRONT_DAMAGE_L_WING 	(0x04)
#define FRONT_DAMAGE_R_WING 	(0x08)

#define REAR_DAMAGE_TRUNK 		(0x10)
#define REAR_DAMAGE_DOOR_LR 	(0x20)
#define REAR_DAMAGE_DOOR_RR		(0x40)

float FRONT_DAMAGE_MAG_MIN_BONNET = 0.3f;
float FRONT_DAMAGE_MAG_MIN_WSCREEN = 0.2f;
float FRONT_DAMAGE_MAG_MIN_WING = 0.35f;

#define TANK_DAMAGE_MULTIPLIER (15.0f)

void CAutomobile::VehicleDamage(float fImpulse, uint16 nPieceType, CEntity *pDamageEntity, CVector *pVecDamagePos, CVector *pVecDamageNormal, eWeaponType weaponType)
{
	float fPanelDamageMultiplier = 1.0f;
	bool bDamageDone = false;
	float fAffectHealthMult = 0.333f;
	float fDamageThreshhold = CAR_DAMAGE_THRESHOLD;
	
	// If the flag is set for don't damage this car (usually during a cutscene)
	if (!m_nVehicleFlags.bCanBeDamaged)
		return;

	if(!fImpulse)
	{
		// if no impulse passed directly to function then get damage from previous collision
		// (alternative is that impulse came directly from melee weapon code
		fImpulse = GetDamageImpulseMagnitude();
		nPieceType = GetDamagedPieceType();
		fAffectHealthMult = 1.0f;
		fDamageThreshhold *= (m_fMass / CAR_DAMAGE_SCALE_MASS);
		
		pDamageEntity = GetDamageEntity();
		pVecDamagePos = &m_vecDamagePos;
		pVecDamageNormal = &m_vecDamageNormal;
		
		// Car upside down (will explode).
		if (GetMatrix().GetUp().z < 0.0f && this != FindPlayerVehicle())
		{
			if(m_nAutomobileFlags.bDoesNotGetDamagedUpsideDown || GetStatus()==STATUS_PLAYER_REMOTE || m_nPhysicalFlags.bIsInWater)
				return;
			// Make sure computer cars alway explode when upside down.
			else if(GetStatus()!=STATUS_WRECKED)
				m_nHealth = MAX(0.0f, m_nHealth - CTimer::GetTimeStep() * 4.0f);
		}
		
		if(fImpulse==0.0f)
			return;
		
		if(m_nPhysicalFlags.bNotDamagedByCollisions)
			return;
		
		if(GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
		{
			if(CBike::DamageKnockOffRider(this, GetDamageImpulseMagnitude(), GetDamagedPieceType(), GetDamageEntity(), m_vecDamagePos, m_vecDamageNormal))
				return;
		}

		// if we've just had a head on collision, want to apply the brakes for a bit
		// so we don't just roll backwards in an unrealistic fashion
		static float VEH_DAMAGE_TEMPACT_DOTLIM = 0.4f;
		static float VEH_DAMAGE_TEMPACT_IMPLIM = 0.1f;
		float fDamageFwd = DotProduct(*pVecDamageNormal, GetMatrix().GetForward());
		
		if(GetStatus()==STATUS_PHYSICS && VehicleCreatedBy != MISSION_VEHICLE
		&& fDamageFwd < -VEH_DAMAGE_TEMPACT_DOTLIM
		&& fImpulse/m_fMass > VEH_DAMAGE_TEMPACT_IMPLIM)
		{
			AutoPilot.TempAction = CAutoPilot::TEMPACT_HEADON_COLLISION;
			AutoPilot.TempActionFinish = CTimer::GetTimeInMilliseconds() + CAR_HEADON_COLLISION_ACTION_TIME;
		}
		
		// this modifies impulse so you take less damage if you deliberately ram another car
		// for arena oddjobs such as destruction derby and sumo stuff
		if(CGame::currArea!=AREA_MAIN_MAP && CMaths::Abs(fDamageFwd) > VEH_DAMAGE_TEMPACT_DOTLIM
		&& pDamageEntity && pDamageEntity->GetIsTypeVehicle())
			fImpulse *= 0.333f;

		// if we've collided with a ped, want to subtract the damage of them running into us
		// so that ped's dont damage cars by sprinting in to them, cause that's silly
		if(pDamageEntity && pDamageEntity->GetIsTypePed() && ((CPed *)pDamageEntity)->GetIsStanding())
		{
			CPed *pHitPed = (CPed *)pDamageEntity;
			float fPedVelAlongCol = DotProduct( pHitPed->m_extractedVelocity.y*pHitPed->GetMatrix().GetForward(), *pVecDamageNormal);
			if(fPedVelAlongCol < 0.0f)
			fImpulse = MAX(0.0f, fImpulse + fPedVelAlongCol*pHitPed->m_fMass);
		}
	}
	else
	{
		bool8 bIgnore = false;
		if (CanVehicleBeDamaged(pDamageEntity, weaponType, &bIgnore) == false)
		{
			return;
		}
		ASSERT(pVecDamagePos);
		ASSERT(pVecDamageNormal);
	}

	// Collisions from the road don't do any damage.
	if(pDamageEntity!=NULL && pDamageEntity->GetIsTypeBuilding() && DotProduct(*pVecDamageNormal, GetMatrix().GetUp())>0.6f )
		return;
	
	// Depending on the flags that have been set for this vehicle we might
	// decide not to do damage.
	if(GetStatus()!=STATUS_PLAYER)
	{	//	Ignore the bOnlyDamagedByPlayer flag if the player is driving this car
		if(this->m_nPhysicalFlags.bOnlyDamagedByPlayer && pDamageEntity && (pDamageEntity!=FindPlayerPed() && pDamageEntity!=FindPlayerVehicle()))
			return;	// If we can only be damaged by player and this is not him.
	}
	
	// player's vehicles take less damage once 100% reached
	if(GetStatus()==STATUS_PLAYER && CStats::GetPercentageProgress() >= 100.0f)
		fImpulse *= 0.5f;
	
	// initialise the damage position to zero in case GetComponentPosition fails (shouldn't happen)
	CVector DamagePosition(0.0f, 0.0f, 0.0f);
	UInt32 LightDamage[MAX_LIGHTS];

	// trailer can't damage tractor, and vice-versa
	if(pDamageEntity && (pDamageEntity==m_pTowingVehicle || pDamageEntity==m_pVehicleBeingTowed))
		return;

	if(fImpulse > fDamageThreshhold && GetStatus()!=STATUS_WRECKED)
	{
#ifdef GTA_NETWORK
		if (gGameNet.NetWorkStatus == NETSTAT_SERVER) CNetGames::CarCollided(this, fImpulse);
#endif

		// If we are a police car and the player collided with us we go to a minimum
		// wanted level of 1
		if (GetIsLawEnforcer() && FindPlayerVehicle() && pDamageEntity==FindPlayerVehicle() && GetStatus() != STATUS_ABANDONED)
		{
			// If police car is moving slower than player car
			if(GetMoveSpeed().Magnitude() <= FindPlayerVehicle()->GetMoveSpeed().Magnitude() &&
					FindPlayerVehicle()->GetMoveSpeed().Magnitude() > 0.1f)
			{
				FindPlayerPed()->SetWantedLevelNoDrop(WANTED_LEVEL1);
			}
		}

		if(GetStatus()==STATUS_PLAYER && fImpulse>50.0f)
		{
			// Add little jolt.
			uint8 nFreq = (uint8) MIN(250.0f, 100 + 0.4f*fImpulse*2000.0f/m_fMass);
			uint16 nLength = 40000/nFreq;
//			printf("Crash Car: %d - %d\n", nLength, nFreq);
			CPad::GetPad(0)->StartShake(nLength, nFreq, 2000);
		}
		
		if(pDamageEntity)
		{
			if(pDamageEntity->GetIsTypeVehicle())
			{
				LastDamagedWeaponType = WEAPONTYPE_RAMMEDBYCAR;
				pLastDamageEntity = pDamageEntity;
//				pLastDamageEntity->RegisterReference((CEntity **)&pLastDamageEntity);
				REGREF(pLastDamageEntity, (CEntity **)&pLastDamageEntity);
			}
		}
		
		// Record light damage before damage applied
		for(int32 iLight=0; iLight<MAX_LIGHTS; iLight++)
		{
			LightDamage[iLight] = Damage.GetLightStatus((eLights)iLight);
		}

		// reset vehicle timer for driving skill stat as we have hit something:
		if (GetStatus()==STATUS_PLAYER)  // check its the player controlling
		{
			CWorld::Players[CWorld::PlayerInFocus].vehicle_time_counter = CTimer::GetTimeInMilliseconds();
		}

		float speedSqr = m_vecMoveSpeed.MagnitudeSqr();
		
		if (speedSqr>0.02f*0.02f)
		{
			this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
		}

		// only want to do particles if car is right way up or moving at a decent speed
		if(GetMatrix().GetUp().z > 0.0f || speedSqr>0.3f)
		{
			fPanelDamageMultiplier = 4.0f;
			uint32 nDamageChoices = 0;
			float fSideForce = 0.0f;
			float fSideOffset = 0.0f;
			
			
			switch(nPieceType)
			{
///////////////////////////////////////
// All of FrontBumper, Bonnet, FLWing, FRWing, Windscreen are grouped together
// with conditional breaks based on the choices made if the FrontBumper collided
				case(COLPOINT_PIECETYPE_FRONTBUMPER):
//					this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);

					if(Damage.ApplyDamage(this, CT_PANEL_FRONT_BUMPER, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
						bDamageDone = true;
					
					// need to make choices for front bumper about passing damage on
					fSideForce = DotProduct(*pVecDamageNormal, GetMatrix().GetRight());
					fSideOffset = DotProduct(*pVecDamagePos - GetPosition(), GetMatrix().GetRight());
					ASSERT(GetColModel().GetBoundBoxMax().x > 0.0f);
					fSideOffset /= GetColModel().GetBoundBoxMax().x;
					
					if(fSideOffset > 0.7f || (fSideOffset > 0.5f && fSideForce < -0.5f) && fImpulse > FRONT_DAMAGE_MAG_MIN_WING*m_fMass)
						nDamageChoices |= FRONT_DAMAGE_R_WING;
					else if(fSideOffset < -0.7f || (fSideOffset < -0.5f && fSideForce > 0.5f) && fImpulse > FRONT_DAMAGE_MAG_MIN_WING*m_fMass)
						nDamageChoices |= FRONT_DAMAGE_L_WING;
					
					if(Damage.GetPanelStatus(FRONT_BUMPER) >= DT_PANEL_BASHED && (fImpulse > FRONT_DAMAGE_MAG_MIN_BONNET*m_fMass
					|| (weaponType < WEAPONTYPE_LAST_WEAPONTYPE && 0==(CGeneral::GetRandomNumber() &3))))
						nDamageChoices |= FRONT_DAMAGE_BONNET;
					if(Damage.GetPanelStatus(FRONT_BUMPER) >= DT_PANEL_BASHED2 && fImpulse > FRONT_DAMAGE_MAG_MIN_WSCREEN*m_fMass)
						nDamageChoices |= FRONT_DAMAGE_WSCREEN;

					if(nDamageChoices==0)
						break;
						
				case(COLPOINT_PIECETYPE_BONNET):
					if(nDamageChoices==0 || nDamageChoices &FRONT_DAMAGE_BONNET)
						if(Damage.ApplyDamage(this, CT_DOOR_BONNET, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
							bDamageDone = true;
					
					if(nDamageChoices==0)
					{
//						this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
						break;
					}

				case(COLPOINT_PIECETYPE_FRONTLEFTWING):
					if(nDamageChoices==0 || nDamageChoices &FRONT_DAMAGE_L_WING)
						if(Damage.ApplyDamage(this, CT_PANEL_FRONT_LEFT, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
							bDamageDone = true;

					if(nDamageChoices==0)
					{
//						this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
						break;
					}

				case(COLPOINT_PIECETYPE_FRONTRIGHTWING):
					if(nDamageChoices==0 || nDamageChoices &FRONT_DAMAGE_R_WING)
						if(Damage.ApplyDamage(this, CT_PANEL_FRONT_RIGHT, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
							bDamageDone = true;

					if(nDamageChoices==0)
					{
//						this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
						break;
					}

				case(COLPOINT_PIECETYPE_WINDSCREEN):
					if(nDamageChoices==0 || nDamageChoices &FRONT_DAMAGE_WSCREEN)
						if(Damage.ApplyDamage(this, CT_PANEL_WINDSCREEN, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
							bDamageDone = true;
					// last one of group so definately break out!
					break;
				
///////////////////////////////////////
// RearBumper and Trunk (and rear doors for vans only) are grouped together 
// so damage is passed onto the boot through the bumper
				case(COLPOINT_PIECETYPE_REARBUMPER):
//					this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);

					if(Damage.ApplyDamage(this, CT_PANEL_REAR_BUMPER, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
						bDamageDone = true;

					// need to make choices for front bumper about passing damage on
					fSideForce = DotProduct(*pVecDamageNormal, GetMatrix().GetRight());
					fSideOffset = DotProduct(*pVecDamagePos - GetPosition(), GetMatrix().GetRight());
					ASSERT(GetColModel().GetBoundBoxMax().x > 0.0f);
					fSideOffset /= GetColModel().GetBoundBoxMax().x;
					
					// if this is a van then pass damage to rear doors
					if(m_nVehicleFlags.bIsVan && fSideOffset > 0.1f && fImpulse > FRONT_DAMAGE_MAG_MIN_WING*m_fMass)
						nDamageChoices |= REAR_DAMAGE_DOOR_RR;
					else if(m_nVehicleFlags.bIsVan && fSideOffset < -0.1f && fImpulse > FRONT_DAMAGE_MAG_MIN_WING*m_fMass)
						nDamageChoices |= REAR_DAMAGE_DOOR_LR;

					// if bumper is missing then damage trunk if it exists
					if(Damage.GetPanelStatus(REAR_BUMPER)<DT_PANEL_BASHED2)
						nDamageChoices |= REAR_DAMAGE_TRUNK;

					if(nDamageChoices==0)
						break;
					
				case(COLPOINT_PIECETYPE_TRUNK):
					if(nDamageChoices==0 || nDamageChoices &REAR_DAMAGE_TRUNK)
						if(Damage.ApplyDamage(this, CT_DOOR_BOOT, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
							bDamageDone = true;
					
					if(nDamageChoices==0)
					{
//						this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
						break;
					}

				case(COLPOINT_PIECETYPE_REARLEFTDOOR):
					if(nDamageChoices==0 || nDamageChoices &REAR_DAMAGE_DOOR_LR)
						if(Damage.ApplyDamage(this, CT_DOOR_REAR_LEFT, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
							bDamageDone = true;

					if(nDamageChoices==0)
					{
//						this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
						break;
					}

				case(COLPOINT_PIECETYPE_REARRIGHTDOOR):
					if(nDamageChoices==0 || nDamageChoices &REAR_DAMAGE_DOOR_RR)
						if(Damage.ApplyDamage(this, CT_DOOR_REAR_RIGHT, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
							bDamageDone = true;

					if(nDamageChoices==0)
					{
//						this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
					}
					
					// last one of group so definately break out!
					break;

				case(COLPOINT_PIECETYPE_FRONTLEFTDOOR):
//					this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);

					if(Damage.ApplyDamage(this, CT_DOOR_FRONT_LEFT, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
						bDamageDone = true;
					break;

				case(COLPOINT_PIECETYPE_FRONTRIGHTDOOR):
//					this->dmgDrawCarCollidingParticles(*pVecDamagePos, fImpulse*fAffectHealthMult, weaponType);
					
					if(Damage.ApplyDamage(this, CT_DOOR_FRONT_RIGHT, fImpulse * fPanelDamageMultiplier, pHandling->fCollisionDamageMultiplier))
						bDamageDone = true;
					break;

				/*
				case COLPOINT_PIECETYPE_FRONTLEFTWHEEL:
					Damage.ApplyDamage(this, CT_WHEEL_FRONT_LEFT, fImpulse, pHandling->fCollisionDamageMultiplier);
				break;

				case COLPOINT_PIECETYPE_FRONTRIGHTWHEEL:
					Damage.ApplyDamage(this, CT_WHEEL_FRONT_RIGHT, fImpulse, pHandling->fCollisionDamageMultiplier);
				break;

				case COLPOINT_PIECETYPE_REARLEFTWHEEL:
					Damage.ApplyDamage(this, CT_WHEEL_REAR_LEFT, fImpulse, pHandling->fCollisionDamageMultiplier);
				break;

				case COLPOINT_PIECETYPE_REARRIGHTWHEEL:
					Damage.ApplyDamage(this, CT_WHEEL_REAR_RIGHT, fImpulse, pHandling->fCollisionDamageMultiplier);
				break;
				*/
			}//switch (GetDamagedPieceType())...

/*
			if (GetDamageEntity() && GetDamageEntity() == FindPlayerVehicle())
			{
				if (fImpulse > 10.0f)
				{
					Int32	Amount;
					
					if (bDamageDone)
					{
						Amount = pHandling->nMonetaryValue * fImpulse / 1500000.0f;
					}
					else
					{
						Amount = pHandling->nMonetaryValue * fImpulse / 3000000.0f;
					}
					
					Amount = MIN(Amount, 40);
				
					if (Amount > 1)
					{
						sprintf(gString, "$%d", Amount);
//						CMoneyMessages::RegisterOne(DamagePosition, gString, 200, 200, 200, 1.0f, 0.4f);
						CWorld::Players[CWorld::PlayerInFocus].Score += Amount;
					}
				}
			}
*/

		}// if car's not upside down

		// Do damage to the general health of the car.
		fImpulse -= fDamageThreshhold;
		fImpulse *= pHandling->fCollisionDamageMultiplier;
		fImpulse *= VEHICLEDAMAGEMULTIPLIER;
		
		// new one to reduce damage done to global health of car by melee weapons
		fImpulse *= fAffectHealthMult;
		
		// For car ramming armoured car missions
		if(	m_nModelIndex == MODELID_CAR_SECURICAR && 
			GetDamageEntity() && 
			GetDamageEntity()->GetStatus() == STATUS_PLAYER)
			fImpulse *= 7.0f;

		if(m_nModelIndex == MODELID_CAR_RCCOPTER || m_nModelIndex == MODELID_CAR_RCGOBLIN || m_nModelIndex == MODELID_CAR_RCTIGER)
			fImpulse *= 30.0f;
	
		// tanks do lots more damage
		if (GetDamageEntity() && GetDamageEntity()->m_nModelIndex == MODELID_CAR_RHINO)
		{
			fImpulse *= TANK_DAMAGE_MULTIPLIER;
		}
		
		if (fImpulse > 0.0f)
		{
			//If inflictor of damage is a vehicle and the inflictor has a driver then give driver of this 
			//vehicle a CEventVehicleDamage
			if(GetDamageEntity() && GetDamageEntity()->GetIsTypeVehicle() && ((CVehicle*)GetDamageEntity())->pDriver && pDriver)
			{			
				// react to the collision 
				if (fImpulse > 5.0f)
				{
					CVehicle* pVehicle=this;
					CVehicle* pOtherVehicle=(CVehicle*)GetDamageEntity();
					CVector v;
					v.FromSubtract(pOtherVehicle->GetMoveSpeed(),pVehicle->GetMoveSpeed());
					const CVector& n=m_vecDamageNormal;
					
					if(DotProduct(n,v)<0)
					{
						pOtherVehicle=this;
						pVehicle=(CVehicle*)GetDamageEntity();
					}

					//pVehicle has rammed into pOtherVehicle.
					CEventVehicleDamageCollision event(pOtherVehicle,pVehicle,WEAPONTYPE_RAMMEDBYCAR);
					pOtherVehicle->pDriver->GetPedIntelligence()->AddEvent(event);	
					CVehicle::ReactToVehicleDamage(pOtherVehicle->pDriver);

					/*
					// only pass event to the driver of the car being bashed into by another (compare their speeds)
					if(GetMoveSpeed().Magnitude() <= ((CVehicle*)GetDamageEntity())->GetMoveSpeed().Magnitude())
					{
						CEventVehicleDamageCollision event(this,GetDamageEntity(),WEAPONTYPE_RAMMEDBYCAR);
						pDriver->GetPedIntelligence()->AddEvent(event);	
						CVehicle::ReactToVehicleDamage(((CVehicle*)GetDamageEntity())->pDriver);	
					}
					*/
				}
			}

			// Shout Oi!

			if (fImpulse > 5.0f) //35.0f)
			{		// Shout abuse for bad collisions
				CEntity *pEnt;
				pEnt = GetDamageEntity();

				if (this->pDriver)
				{
					if(pEnt)
					{
						if(pEnt->GetIsTypeVehicle())
						{
							if(!((FindPlayerVehicle() == this) && (((CVehicle*)(pEnt))->GetVehicleCreatedBy() == MISSION_VEHICLE)))
							{
								if(((CVehicle*)(pEnt))->pDriver) // only say something when the other driver can hear it
								{
									switch (m_VehicleAudioEntity.GetVehicleTypeForAudio())
									{
										case AE_VEHICLE_AUDIO_TYPE_CAR:
											this->pDriver->Say(CONTEXT_GLOBAL_CRASH_CAR, 0, 0.66f);
											break;
										case AE_VEHICLE_AUDIO_TYPE_BIKE:
											this->pDriver->Say(CONTEXT_GLOBAL_CRASH_BIKE, 0, 0.66f);
											break;
										default:
											this->pDriver->Say(CONTEXT_GLOBAL_CRASH_GENERIC, 0, 0.66f);
											break;
									}
								}
							}
						}
					}
				}
				
				// The passengers might have something to say too.
				if (this == FindPlayerVehicle())
				{
					if(pEnt)
					{
						CPed *pPassenger = this->PickRandomPassenger();
						if (pPassenger)
						{
							if (pEnt->GetIsTypePed())
							{
								pPassenger->Say(CONTEXT_GLOBAL_CAR_HIT_PED);
							}
							else
							{
								pPassenger->Say(CONTEXT_GLOBAL_CAR_CRASH);
							}
						}
					}
				}

				//Register this as an 'interesting event'
				g_InterestingEvents.Add(CInterestingEvents::ECarCrash, this);

			}



			Int16	OldHealth;
			OldHealth = m_nHealth;
			
			if (this == FindPlayerVehicle())
			{
				if (!m_nVehicleFlags.bTakeLessDamage)
				{
					m_nHealth -= (fImpulse) / 2.0f;
				}
				else
				{
					m_nHealth -= (fImpulse) / 6.0f;
				}

//				CPad::GetPad(0)->StartShake(110, 110); // dur freq
			}
			else
			{
/*				if (fImpulse > 35.0f)
				{		// Shout abuse for bad collisions
					if (this->pDriver)
					{
						this->pDriver->Say(AE_DRIVER_SHOUT_ABUSE);
					}
				}
*/				

				if (!m_nVehicleFlags.bTakeLessDamage)
				{
					if( GetDamageEntity() && GetDamageEntity() == FindPlayerVehicle())
						m_nHealth -= (fImpulse) / 1.5f;
					else
						m_nHealth -= (fImpulse) / 4.0f;
				}
				else
				{
					m_nHealth -= (fImpulse) / 12.0f;
				}
			}
							
			if (CCheat::IsCheatActive(CCheat::VEHICLEOFDEATH_CHEAT) && 
				GetDamageEntity() && 
				GetDamageEntity() == FindPlayerVehicle())
			{
				BlowUpCar(GetDamageEntity());
			}
			else if (m_nHealth <= 0 && OldHealth > 0)	// First number is zero
			{
				m_nHealth = 1.0f;		// Have to go on fire first to give player chance to get out.

				if (GetDamageEntity() && GetDamageEntity()->m_nModelIndex == MODELID_CAR_RHINO && FindPlayerVehicle() != this)
				{	
					BlowUpCar(GetDamageEntity());
				}
			}
		}
	
		// Compare Light damage from before damage applied
		for(int32 iLight=0; iLight < MAX_LIGHTS; iLight++)
		{
			if((LightDamage[iLight] != DT_LIGHT_SMASHED) && (Damage.GetLightStatus((eLights)iLight) == DT_LIGHT_SMASHED))
			{
#ifdef USE_AUDIOENGINE
		m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_LIGHT_SMASH);
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_CAR_LIGHT_SMASHED, (float)(iLight));
#endif //USE_AUDIOENGINE
				break;	// only need to record on light smash
			}
		}
	}
//	else
//	{
//		if (fImpulse > 10.0f)
//		{
//			if (this == FindPlayerVehicle()) CPad::GetPad(0)->StartShake(90, 70); // dur freq
//		}
//	}

	if (m_nHealth < CAR_ON_FIRE_HEALTH)
	{
		// Make the engine go on fire (only if it not already is though)
		if (Damage.GetEngineStatus() < DT_ENGINE_ON_FIRE)
		{
			Damage.SetEngineStatus(DT_ENGINE_ON_FIRE);
			m_BlowUpTimer=0;
//			m_nFlags.bOnFire = true;
			pEntityThatSetUsOnFire = GetDamageEntity();
			if (pEntityThatSetUsOnFire)
			{
				REGREF(((CEntity*)pEntityThatSetUsOnFire), ((CEntity**)&pEntityThatSetUsOnFire));
			}
			
			// Passenger might want to comment on this.
			CPed *pPassenger = PickRandomPassenger();
			if (pPassenger)
			{
				pPassenger->Say(CONTEXT_GLOBAL_CAR_FIRE, 1500);
			}

			
			
		}
	}
	// for buggy - there's no locational damage so need to force engine damage to get smoke
	// 			   so there is some visible indication of the cars health
	else if(m_nModelIndex==MODELID_CAR_BUGGY)
	{
		if(m_nHealth < 400.0f)
			Damage.SetEngineStatus(DT_ENGINE_ENGINE_PIPES_BURST);
		else if(m_nHealth < 600.0f)
			Damage.SetEngineStatus(DT_ENGINE_RADIATOR_BURST);
	}
}

//
// name:		TellHeliToGoToCoors
// description:	This function will tell a script generated heli to fly to specific
//				coordinates.

void CAutomobile::TellHeliToGoToCoors(float TargetX, float TargetY, float TargetZ, float MinHeightAboveTerrain, float LowestFlightHeight)
{
	// Make sure this indeed IS a heli
	ASSERTMSG(this->GetVehicleAppearance() == APR_HELI, "TellHeliToGoToCoors - Vehicle does not look like a heli");
	ASSERTMSG(GetVehicleType() == VEHICLE_TYPE_HELI, "TellHeliToGoToCoors - Vehicle is not a heli");

	AutoPilot.Mission = CAutoPilot::MISSION_HELI_FLYTOCOORS;
	AutoPilot.TargetCoors.x = TargetX;
	AutoPilot.TargetCoors.y = TargetY;
	AutoPilot.TargetCoors.z = TargetZ;
	AutoPilot.CruiseSpeed = 100;
	((CHeli *) this)->m_MinHeightAboveTerrain = MinHeightAboveTerrain;
	((CHeli *) this)->m_LowestFlightHeight = LowestFlightHeight;

	SetStatus(STATUS_PHYSICS);

	// Make sure the rotation is set properly first time we call it.
	if (ZRot == 0.0f)
	{
		ZRot = CGeneral::GetATanOfXY(GetMatrix().GetForward().x, GetMatrix().GetForward().y) + PI;
		while (ZRot > 2.0f*PI) ZRot -= 2.0f*PI;
	}
}

//
// name:		SetHeliOrientation
// description:	This function will tell a script generated heli to fly to specific
//				coordinates.

void CAutomobile::SetHeliOrientation(float Orientation)
{
	HeliRequestedOrientation = Orientation;
}

//
// name:		ClearHeliOrientation
// description:	Releases the heli orientation. Allows it to pick its own.

void CAutomobile::ClearHeliOrientation()
{
	HeliRequestedOrientation = -1.0f;
}


//
// name:		TellPlaneToGoToCoors
// description:	This function will tell a script generated plane to fly to specific
//				coordinates.

void CAutomobile::TellPlaneToGoToCoors(float TargetX, float TargetY, float TargetZ, float MinHeightAboveTerrain, float LowestFlightHeight)
{
	// Make sure this indeed IS a plane
	ASSERTMSG(this->GetVehicleAppearance() == APR_PLANE, "TellPlaneToGoToCoors - Vehicle does not look like a plane");
	ASSERTMSG(GetVehicleType() == VEHICLE_TYPE_PLANE, "TellPlaneToGoToCoors - Vehicle is not a plane");

	AutoPilot.Mission = CAutoPilot::MISSION_PLANE_FLYTOCOORS;
	AutoPilot.TargetCoors.x = TargetX;
	AutoPilot.TargetCoors.y = TargetY;
	AutoPilot.TargetCoors.z = TargetZ;
	AutoPilot.CruiseSpeed = 0;
	((CPlane *) this)->m_MinHeightAboveTerrain = MinHeightAboveTerrain;
	((CPlane *) this)->m_LowestFlightHeight = MAX(LowestFlightHeight, TargetZ);

	SetStatus(STATUS_PHYSICS);

	// Make sure the rotation is set properly first time we call it.
	if (ZRot == 0.0f)
	{
		ZRot = CGeneral::GetATanOfXY(GetMatrix().GetForward().x, GetMatrix().GetForward().y);
	}

	// Obbe:For some reason switching the engine on has become necessary.
	// Doing it here so that it doesn't affect anything else. (Just before SA final build)
	m_nVehicleFlags.bEngineOn = true;

}



//
// name:		ProcessFlyingCarStuff
// description:	All the code to handle the control process of flying vehicles
//				that used to reside in ProcessControl
//
void CAutomobile::ProcessFlyingCarStuff()
{
	if(GetStatus()==STATUS_PLAYER || GetStatus()==STATUS_PLAYER_REMOTE || GetStatus()==STATUS_PHYSICS)
	{
		if (CCheat::IsCheatActive(CCheat::FLYINGCARS_CHEAT) && m_vecMoveSpeed.Magnitude() > 0.0f && CTimer::GetTimeStep() > 0.0f)
		{
			FlyingControl(FLIGHTMODEL_PLANE);
		}
	}
}



//
// name:		HideAllComps
// description:	Hide the allcomps atomic and show all the other atomics
void CAutomobile::HideAllComps()
{
/*	if(m_aCarNodes[CAR_ALLCOMPS] && !bIsDamaged)
	{
		// hide all damaged atomics and display all other atomics
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::HideAllComponentsAtomicCB, (void*)VEHICLE_ATOMIC_DAMAGED);
		// hide allcomps atomic
		RwFrameForAllObjects(m_aCarNodes[CAR_ALLCOMPS], &SetVehicleAtomicVisibilityCB, (void*)VEHICLE_ATOMIC_DAMAGED);
	}*/
}

//
// name:		ShowAllComps
// description:	show the allcomps atomic and hide all the other atomics
void CAutomobile::ShowAllComps()
{
/*	if(m_aCarNodes[CAR_ALLCOMPS] && !bIsDamaged)
	{
		// hide all damaged/ok atomics and display all other atomics
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::HideAllComponentsAtomicCB, (void*)(VEHICLE_ATOMIC_DAMAGED|VEHICLE_ATOMIC_OK));
		// show allcomps atomic
		RwFrameForAllObjects(m_aCarNodes[CAR_ALLCOMPS], &SetVehicleAtomicVisibilityCB, (void*)VEHICLE_ATOMIC_NONE);
	}*/
}

//
// SetBumperDamage: Change model dependent on how it is damaged. This is only called whenever the
//					damage state changes
//
void CAutomobile::SetBumperDamage(ePanels PanelID, bool bDontSpawnStuff)
{
	uint8 Status = Damage.GetPanelStatus(PanelID);
	int32 index = CDamageManager::GetCarNodeIndexFromPanel(PanelID);
	
	if(m_aCarNodes[index] == NULL)
	{
		DEBUGLOG2("Trying to damage component %d of %s\n", index, CModelInfo::GetModelInfo(m_nModelIndex)->GetModelName());
		return;
	}
	else if(!((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
	{
		DEBUGLOG2("No damage atomic on component %d of %s\n", index, CModelInfo::GetModelInfo(m_nModelIndex)->GetModelName());
		return;
	}
	// used for rotating & translating -> m_aCarNodes[wing_lf_dummy]
/*	if (Status == DT_PANEL_SHIFTED)
	{
		CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
		CVector posn(matrix.GetTranslate());

		// rotate panel by (-/+0.3 radians)
		//matrix.SetRotateY(CGeneral::GetRandomNumberInRange(-DAMAGED_PANEL_MAX_ANGLE, DAMAGED_PANEL_MAX_ANGLE));
		//matrix.SetRotateX(CGeneral::GetRandomNumberInRange(-DAMAGED_PANEL_MAX_ANGLE, DAMAGED_PANEL_MAX_ANGLE));
		matrix.SetRotateX(CGeneral::GetRandomNumberInRange(-DAMAGED_PANEL_MAX_ANGLE, DAMAGED_PANEL_MAX_ANGLE));
		matrix.Translate(posn);
		matrix.UpdateRW();
	}
	else*/
	if(Status == DT_PANEL_BASHED2 && !(pHandling->mFlags &MF_NO_PANEL_BOUNCING))
	{
		for(int i=0; i<=MAX_BOUNCING_PANELS; i++)
		{
			if(BouncingPanels[i].GetCompIndex()==-1)
			{
				BouncingPanels[i].SetPanel(index, 0, CGeneral::GetRandomNumberInRange(-0.2f, -0.5f));
				break;
			}
			else if(BouncingPanels[i].GetCompIndex()==index)
				break;	// already set that panel to bounce
		}
	}
	else if(Status == DT_PANEL_BASHED)
	{
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
	}
	else if(Status == DT_PANEL_MISSING)
	{
		if (!bDontSpawnStuff) SpawnFlyingComponent(index, CG_BUMPER);
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_NONE);
	}
}


//
// SetPanelDamage: Change model dependent on how it is damaged. This is only called whenever the
//					damage state changes
//
void CAutomobile::SetPanelDamage(ePanels PanelID, bool bDontSpawnStuff)
{
	uint8 Status = Damage.GetPanelStatus(PanelID);

	int32 index = CDamageManager::GetCarNodeIndexFromPanel(PanelID);

	if(m_aCarNodes[index] == NULL)
		return;
	else if(!((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
	{
		DEBUGLOG2("No damage atomic on component %d of %s\n", index, CModelInfo::GetModelInfo(m_nModelIndex)->GetModelName());
		return;
	}

	// used for rotating & translating -> m_aCarNodes[wing_lf_dummy]
	/*if (Status == DT_PANEL_SHIFTED && PanelID != WINDSCREEN_PANEL)
	{
		CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
		CVector posn(matrix.GetTranslate());

		// rotate panel by (-/+0.3 radians)
		//matrix.SetRotateX(CGeneral::GetRandomNumberInRange(-DAMAGED_PANEL_MAX_ANGLE, DAMAGED_PANEL_MAX_ANGLE));
		//matrix.SetRotateY(CGeneral::GetRandomNumberInRange(-DAMAGED_PANEL_MAX_ANGLE, DAMAGED_PANEL_MAX_ANGLE));
		matrix.SetRotateZ(CGeneral::GetRandomNumberInRange(-DAMAGED_PANEL_MAX_ANGLE, DAMAGED_PANEL_MAX_ANGLE));
		matrix.Translate(posn);
		matrix.UpdateRW();
	}
	else*/
	if(Status == DT_PANEL_BASHED2 && !(pHandling->mFlags &MF_NO_PANEL_BOUNCING))
	{
		for(int i=0; i<=MAX_BOUNCING_PANELS; i++)
		{
			if(BouncingPanels[i].GetCompIndex()==-1)
			{
				if(index==CAR_WINDSCREEN || index==CAR_WING_LF || index==CAR_WING_RF)
					;
				else
					BouncingPanels[i].SetPanel(index, 1, CGeneral::GetRandomNumberInRange(-0.2f, -0.5f));
				break;
			}
			else if(BouncingPanels[i].GetCompIndex()==index)
				break;	// already set that panel to bounce
		}
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
	}
	else if(Status == DT_PANEL_BASHED)
	{
		if(PanelID == WINDSCREEN_PANEL)
		{
#ifdef USE_AUDIOENGINE
		m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_WINDSCREEN_SHATTER);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_CAR_WINDSCREEN_SHATTER, 0.0f);
#endif //USE_AUDIOENGINE
		}

		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
	}
	else if(Status == DT_PANEL_MISSING)
	{
		if (!bDontSpawnStuff) SpawnFlyingComponent(index, CG_PANEL);
		else if(PanelID==WINDSCREEN_PANEL)	CGlass::CarWindscreenShatters(this, false);
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_NONE);
	}
}


//
// SetDoorDamage: Change model dependent on how it is damaged. This is only called whenever the
//					damage state changes
//
void CAutomobile::SetDoorDamage(eDoors DoorID, bool bDontSpawnStuff)
{
	uint8 Status = Damage.GetDoorStatus(DoorID);
	
	int32 index = CDamageManager::GetCarNodeIndexFromDoor(DoorID);
	
	if(m_aCarNodes[index] == NULL)
	{
		DEBUGLOG2("Trying to damage component %d of %s\n", index, CModelInfo::GetModelInfo(m_nModelIndex)->GetModelName());
		return;
	}

	// don't let locked doors be knocked off
	if(!CanDoorsBeDamaged() && Status > DT_DOOR_BASHED && DoorID != BONNET && DoorID != BOOT)
	{
		Door[DoorID].Open(0.0f);
		Damage.SetDoorStatus(DoorID, DT_DOOR_BASHED);
		return;
	}	
		
	if(DoorID==BOOT && pHandling->mFlags &MF_NOSWING_BOOT)
	{
		if(Status==DT_DOOR_SWINGING_FREE)
		{
			if(((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
				Status = DT_DOOR_BASHED;
			else
				Status = DT_DOOR_MISSING;
		}
		else if(Status==DT_DOOR_BASHED_AND_SWINGING_FREE)
			Status = DT_DOOR_MISSING;
		
		Damage.SetDoorStatus(BOOT, Status);
	}

	if(Status == DT_DOOR_BASHED)
	{
		if(((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
			SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
		else
			DEBUGLOG2("No damage atomic on component %d of %s\n", index, CModelInfo::GetModelInfo(m_nModelIndex)->GetModelName());
		
		// door wasn't already shut so we need to force it now!
		if(Door[DoorID].m_fPrevAngle != 0.0f)
		{
			Door[DoorID].m_fAngle = 0.0f;
			Door[DoorID].m_fPrevAngle = 0.0f;
			Door[DoorID].m_fAngVel = 0.0f;
		
			CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
			CVector posn(matrix.GetTranslate());
			matrix.SetRotate(0.0f, 0.0f, 0.0f);		
			matrix.Translate(posn);
			matrix.UpdateRW();

			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_CAR_BONNET_CLOSE + DoorID);
		}
	}
	else if(Status == DT_DOOR_MISSING)
	{
		if (!bDontSpawnStuff)
		{
			if(DoorID==BONNET)
			{
#ifdef USE_AUDIOENGINE
				m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_BONNET_FLUBBER_FLUBBER, (CEntity*)SpawnFlyingComponent(index, CG_BONNET));
#endif //USE_AUDIOENGINE
			}
			else if(DoorID==BOOT)
				SpawnFlyingComponent(index, CG_BOOT);
			else
				SpawnFlyingComponent(index, CG_DOOR);
		}

		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_NONE);
	}
	else if(Status==DT_DOOR_SWINGING_FREE || Status==DT_DOOR_BASHED_AND_SWINGING_FREE)
	{
		// ensure swinging doors aren't culled
		RwFrameForAllObjects(m_aCarNodes[index], &CVehicleModelInfo::SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_DONTCULL);
		// make sure bonnet pops up when it springs free
		if(DoorID==BONNET)
			Door[BONNET].m_fAngVel = 0.2f;
	}
}

//
// SetRandomDamage: Give car some random damage like a bashed door or bumper - used
//					when generating random vehicles in poor areas, etc
//
#define MAX_RANDOM_DAMAGED_DOORS	MAX_DOORS
#define MAX_RANDOM_DAMAGED_PANELS	MAX_PANELS-3

void CAutomobile::SetRandomDamage(bool bLotsOfDamage)
{
	int32 numDamagedDoors, numDamagedPanels, door, panel, rand, nonDamaged, i, index;
	bool DamagedDoors[MAX_DOORS];
	bool DamagedPanels[MAX_PANELS];
	eDoors DoorID;
	ePanels PanelID;
	
	for (i=0; i<MAX_DOORS; i++)
		DamagedDoors[i] = FALSE;
		
	for (i=0; i<MAX_PANELS; i++)
	{
		// ignore rear panels and windscreen
		if (i==REAR_LEFT_PANEL || i==REAR_RIGHT_PANEL || i==WINDSCREEN_PANEL)
			DamagedPanels[i] = TRUE;
		else
			DamagedPanels[i] = FALSE;
	}
		
	// how many components are damaged?
	if (bLotsOfDamage)
	{
		numDamagedDoors = CGeneral::GetRandomNumberInRange(0, MAX_DOORS);

		if (numDamagedDoors == 0)
			numDamagedPanels = CGeneral::GetRandomNumberInRange(1, MAX_PANELS-3);
		else
			numDamagedPanels = CGeneral::GetRandomNumberInRange(0, MAX_PANELS-3);
	}
	else
	{
		numDamagedDoors = CGeneral::GetRandomNumberInRange(0, 1);
		
		if (numDamagedDoors == 0)
			numDamagedPanels = 1;
		else
			numDamagedPanels = CGeneral::GetRandomNumberInRange(0, 1);
	}
	
	// work out which doors are damaged
	for (door=0; door<numDamagedDoors; door++)
	{
		// pick a random non-damaged door
		rand = CGeneral::GetRandomNumberInRange(0, MAX_DOORS-door);
		
		if (door==0)
		{
			// no doors damaged yet, so just use random number
			i = rand;
		}
		else
		{	
			i = nonDamaged = -1;
			
			// count non-damaged doors until we get to the random non-damaged door 
			// we want
			while (nonDamaged < rand && i < MAX_DOORS)
			{
				if (!DamagedDoors[++i])
					nonDamaged++;
			}
			
			ASSERT(i < MAX_DOORS);
		}
					
		DoorID = (eDoors)i;
		index = CDamageManager::GetCarNodeIndexFromDoor(DoorID);
		
		if (Damage.GetDoorStatus(DoorID) == DT_DOOR_INTACT && ((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
		{
			Damage.SetDoorStatus(DoorID, DT_DOOR_BASHED);
			SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
		}
						
		DamagedDoors[i] = TRUE;
	}
		
	// work out which panels are damaged
	for (panel=0; panel<numDamagedPanels; panel++)
	{
		// pick a random non-damaged panel
		rand = CGeneral::GetRandomNumberInRange(0, MAX_PANELS-3-panel);
		
		i = nonDamaged = -1;
			
		// count non-damaged panels until we get to the random non-damaged panel 
		// we want
		while (nonDamaged < rand && i < MAX_PANELS)
		{
			if (!DamagedPanels[++i])
				nonDamaged++;
		}
			
		ASSERT(i < MAX_PANELS);
					
		PanelID = (ePanels)i;
		
		index = CDamageManager::GetCarNodeIndexFromPanel(PanelID);
		
		if (Damage.GetPanelStatus(PanelID) == DT_PANEL_INTACT && ((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
		{
			Damage.SetPanelStatus(PanelID, DT_PANEL_BASHED);
			SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
		}
				
		DamagedPanels[i] = TRUE;
	}
}

//
// SetTotalDamage: Damages all doors and panels on a car
//
void CAutomobile::SetTotalDamage(bool bRemoveStuff)
{
	int32 door, panel, index;
	eDoors DoorID;
	ePanels PanelID;
			
	for (door=0; door<MAX_DOORS; door++)
	{
		DoorID = (eDoors)door;
		index = CDamageManager::GetCarNodeIndexFromDoor(DoorID);
		
		if (m_aCarNodes[index])
		{
			if (bRemoveStuff && CGeneral::GetRandomNumberInRange(0,3) == 0)
			{
				Damage.SetDoorStatus(DoorID, DT_DOOR_MISSING);
				SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_NONE);
			}
			else if (Damage.GetDoorStatus(DoorID) == DT_DOOR_INTACT && ((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
			{
				Damage.SetDoorStatus(DoorID, DT_DOOR_BASHED);
				SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
			}
		}
	}
		
	for (panel=0; panel<MAX_PANELS; panel++)
	{
		if (panel != REAR_LEFT_PANEL && panel != REAR_RIGHT_PANEL)
		{
			PanelID = (ePanels)panel;
			index = CDamageManager::GetCarNodeIndexFromPanel(PanelID);
			
			if (m_aCarNodes[index])
			{
				if (bRemoveStuff && CGeneral::GetRandomNumberInRange(0,3) == 0)
				{
					Damage.SetPanelStatus(PanelID, DT_PANEL_MISSING);
					SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_NONE);
				}
				else if (Damage.GetPanelStatus(PanelID) == DT_PANEL_INTACT && ((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->IsCompDamageModelAvailable(index))
				{
					Damage.SetPanelStatus(PanelID, DT_PANEL_BASHED);
					SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_DAMAGED);
				}
			}
		}
	}
/*	
	if (bRemoveStuff)
	{
		for (int32 wheel = 0; wheel < MAX_WHEELS; wheel++)
		{
			if (CGeneral::GetRandomNumberInRange(0,3) == 0)
			{
				Damage.SetWheelStatus(wheel, DT_WHEEL_BURST);
			}
		}
	}
*/
}

RpMaterial* DisableMatFx(RpMaterial* pMaterial, void* data)
{
	RpMatFXMaterialSetEffects(pMaterial, rpMATFXEFFECTNULL);
	return pMaterial;
}

//
//
TweakFloat CAR_DEBRIS_TOWARDCAM_MULT = 5.0f;
//
CPhysical *CAutomobile::SpawnFlyingComponent(int32 index, uint32 type)
{
	if(CObject::nNoTempObjects >= TEMPOBJ_MAX_NO)
		return NULL;	// have already hit max allowable number of spawned objects
		
	if(m_aCarNodes[index]==NULL)
		return NULL;

	RpAtomic *pCurrentAtomic = NULL;
	RpAtomic **ppAtomicData = &pCurrentAtomic;  // extra step for clarity
	
	// this callback should now find the diplayed atomic and return a pointer in pCurrentAtomic
	RwFrameForAllObjects(m_aCarNodes[index], &GetCurrentAtomicObjectCB, (void *)ppAtomicData);
	if(pCurrentAtomic == NULL)
		return NULL;	// if no atomic is displayed then exit without spawning anything!
	
	CPools::GetObjectPool().SetCanDealWithNoMemory(true);
	CObject *pComponent = new CObject();
	CPools::GetObjectPool().SetCanDealWithNoMemory(false);

	// If can't allocate space for an object return
	if(pComponent == NULL)
		return NULL;

	RwMatrix *pMatrix = RwFrameGetLTM(m_aCarNodes[index]);

	// Set model index
	if(index==CAR_WINDSCREEN)
	{
		pComponent->SetModelIndexNoCreate(MODELID_COMPONENT_BONNET);
	}
	else switch(type)
	{
		case CG_WHEEL:
			pComponent->SetModelIndexNoCreate(MODELID_COMPONENT_WHEEL);
			break;
		case CG_BONNET:
			pComponent->SetModelIndexNoCreate(MODELID_COMPONENT_BONNET);
			pComponent->SetCentreOfMass(0.0f, 0.4f, 0.0f);
			break;
		case CG_BOOT:
			pComponent->SetModelIndexNoCreate(MODELID_COMPONENT_BOOT);
			pComponent->SetCentreOfMass(0.0f, -0.3f, 0.0f);
			break;	
		case CG_DOOR:
			pComponent->SetModelIndexNoCreate(MODELID_COMPONENT_DOOR);
			pComponent->SetCentreOfMass(0.0f, -0.5f, 0.0f);
			pComponent->m_nFlags.bDrawLast = true;
			break;
		case CG_BUMPER:
			{
				pComponent->SetModelIndexNoCreate(MODELID_COMPONENT_BUMPER);
				// because bouncing bumpers are attached at the side we need to move their Centre Of Mass
				// back to the middle otherwise they'll react really strangely when put into the world
				CMatrix matBumperTemp(pMatrix);
				float fSidePos = DotProduct(matBumperTemp.GetTranslate() - this->GetPosition(), this->GetMatrix().GetRight());
				pComponent->SetCentreOfMass(-fSidePos, 0.0f, 0.0f);
			}
			break;
		case CG_PANEL:
		default:
			pComponent->SetModelIndexNoCreate(MODELID_COMPONENT_PANEL);
			break;
	}
	// reference vehicles TXD so it isn't deleted before the component
	pComponent->RefModelInfo(m_nModelIndex);

	// Create copy of atomic
	RwFrame* pFrame = RwFrameCreate();

	pCurrentAtomic = RpAtomicClone(pCurrentAtomic);
	RwMatrixCopy(RwFrameGetMatrix(pFrame), pMatrix);
	RpAtomicSetFrame(pCurrentAtomic, pFrame);
	
	CVisibilityPlugins::SetAtomicRenderCallback(pCurrentAtomic,NULL);
	// switch off environment mapping for objects because of a bug where they screw up the render
	// state when fog is enabled
//	RpGeometryForAllMaterials(RpAtomicGetGeometry(pCurrentAtomic), &DisableMatFx, NULL);
	pComponent->AttachToRwObject( (RwObject*) pCurrentAtomic );
	pComponent->m_nFlags.bDontStream = true;
	pComponent->SetMass(10.0f);
	pComponent->SetTurnMass(25.0f);
	pComponent->SetAirResistance(0.97f);
	pComponent->SetElasticity(0.1f);
	pComponent->SetBuoyancyConstant( pComponent->m_fMass * PHYSICAL_GRAVITYFORCE * (100.0f / 75.0f) );
		
	pComponent->ObjectCreatedBy = TEMP_OBJECT;
	pComponent->SetIsStatic(false);
	pComponent->m_nObjectFlags.bIsPickUp = false;
	pComponent->m_nObjectFlags.bParentIsACar = true;
	pComponent->m_colour1 = m_colour1;
	pComponent->m_colour2 = m_colour2;
	pComponent->SetRemapTexture(m_pRemapTexture, m_remapTxdSlot);

	CObject::nNoTempObjects++;
	if(CObject::nNoTempObjects>TEMPOBJ_2ND_LIMIT)
		pComponent->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + CAR_COMPONENT_REMOVE_LIFESPAN*TEMPOBJ_2ND_LIFE_MULT;
	else if(CObject::nNoTempObjects>TEMPOBJ_1ST_LIMIT)
		pComponent->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + CAR_COMPONENT_REMOVE_LIFESPAN*TEMPOBJ_1ST_LIFE_MULT;
	else
		pComponent->m_nEndOfLifeTime = CTimer::GetTimeInMilliseconds() + CAR_COMPONENT_REMOVE_LIFESPAN;
	
	pComponent->m_vecMoveSpeed = m_vecMoveSpeed;
	
	if(pComponent->m_vecMoveSpeed.z > 0.0f)
		pComponent->m_vecMoveSpeed.z *= 1.5f;
	else if(GetMatrix().zz>0.0 && (type==CG_BONNET || type==CG_BOOT || index==CAR_WINDSCREEN)){
		pComponent->m_vecMoveSpeed.z *= -1.5f;
		pComponent->m_vecMoveSpeed.z += 0.04f;
	}
	else
		pComponent->m_vecMoveSpeed.z *= 0.25f;

	pComponent->m_vecMoveSpeed.x *= 0.75f;
	pComponent->m_vecMoveSpeed.y *= 0.75f;
	
	pComponent->m_vecTurnSpeed = m_vecTurnSpeed*2;
	
	// try and do addition shove of component away from car
	CVector vecTemp = pComponent->GetPosition() - GetPosition();
	vecTemp.Normalise();
	
	// if component is on top of car (car upright) add up to shove direction
	if(type==CG_BONNET || type==CG_BOOT || index==CAR_WINDSCREEN){
		vecTemp += GetMatrix().GetUp();
		if(GetMatrix().zz>0.0f){
			float fTemp = CMaths::Sqrt(m_vecMoveSpeed.x*m_vecMoveSpeed.x + m_vecMoveSpeed.y*m_vecMoveSpeed.y);
			pComponent->GetMatrix().Translate(fTemp*GetMatrix().GetUp());
		}
	}
	
	pComponent->ApplyMoveForce(vecTemp);
	
	if(type==CG_WHEEL)
	{
		pComponent->SetTurnMass(5.0f);
		pComponent->m_vecTurnSpeed.x = 0.5f;
		pComponent->SetAirResistance(0.99f);
	}
	
	if( GetStatus()==STATUS_WRECKED && IsVisible() 
	&& DotProduct(vecTemp, TheCamera.GetPosition() - GetPosition()) > -0.5f )
	{
		vecTemp = TheCamera.GetPosition() - GetPosition();
		vecTemp.Normalise();
		vecTemp.z += 0.3f;
		
		pComponent->ApplyMoveForce(CAR_DEBRIS_TOWARDCAM_MULT*vecTemp);
	}
	
	// make sure to call process col models with the spawned object 1st followed by the automobile
	// don't want the automobile collision line to get proccessed
	if(CCollision::ProcessColModels(pComponent->GetMatrix(), CModelInfo::GetColModel(pComponent->GetModelIndex()), GetMatrix(), this->GetColModel(), CWorld::m_aTempColPts)>0)
	{
		// component is going to be spawned colliding with it's parent car, 
		// need to make sure it doesn't process the collision
		pComponent->m_pNOCollisionVehicle = this;
	}	
	
	if(this->m_nPhysicalFlags.bRenderScorched==TRUE)
		pComponent->m_nPhysicalFlags.bRenderScorched=TRUE;
	
	CWorld::Add(pComponent);
	
	return (CPhysical *)pComponent;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : GetHeightAboveRoad
// PURPOSE :  Finds out how high above the actual road surface this car should
//			  be floating. (For cars in rails really)
//			  This should probably be refined to include the physics of the
//			  springs of the wheels (doesn't do that at the moment)
/////////////////////////////////////////////////////////////////////////////////
// NOW DONE IN SetupSuspensionLines() AND SAVED in m_fHeightAboveRoad
/*float CAutomobile::GetHeightAboveRoad()
{
	CColModel& colModel = CModelInfo::GetColModel(m_nModelIndex);

	float fStart = colModel.m_pLineArray[0].m_vecStart.z;
	float fLength = colModel.m_pLineArray[0].m_vecEnd.z - fStart;

	fLength *= 1 - 1/(8*pHandling->fSuspensionForce);
	
	return -(fStart + fLength);
}
*/


void CAutomobile::ReduceHornCounter(void)
{
	// Reduce horn counter if not player (if horn has been triggered)

	if(m_cHorn > 0)
	{
		--m_cHorn;
	}
}


// moved all the automobile buoyancy processing here from process control
// because it was all getting quite big, so it's more managble to be in a seperate
// function of it's own
//
float MOVE_UNDERWATER_CAR_COB = -0.30f;	// hack to make car float horizontally
float MOVE_UNDERWATER_BUBBLE_MOVESPDSQR = 0.02f;
//
void CAutomobile::ProcessBuoyancy()
{
	CVector CentreOfBuoyancy;
	CVector BuoyancyForce;
	bool bMessingAboutInWater = false;
	
	//
	// Car water splash:
	//
	// calculate current water color here: 
	//
	const RwRGBA dummyColor = {0,0,0,0};
	const float COLOR_SCALE = 0.45f; // 0.7f;//0.60f;
	uint8 Red	= COLOR_SCALE * 255.0f * (CTimeCycle::GetAmbientRed_Obj()	+ CTimeCycle::GetDirectionalRed() * 0.5f);
	uint8 Green	= COLOR_SCALE * 255.0f * (CTimeCycle::GetAmbientGreen_Obj()+ CTimeCycle::GetDirectionalGreen() * 0.5f);
	uint8 Blue	= COLOR_SCALE * 255.0f * (CTimeCycle::GetAmbientBlue_Obj()	+ CTimeCycle::GetDirectionalBlue() * 0.5f);
//	const RwRGBA rgbaSplashWaterColor = {225, 225, 255, 180};
	uint8 Alpha = CGeneral::GetRandomNumberInRange(128, 160);
	const RwRGBA rgbaSplashWaterColor = {Red, Green, Blue, Alpha};

	if(mod_Buoyancy.ProcessBuoyancy(this, m_fBuoyancyConstant, &CentreOfBuoyancy, &BuoyancyForce))
	{
		// MN - FX SYSTEM -------------------------------
//		if (m_nPhysicalFlags.bForceFullWaterCheck==false && m_vecMoveSpeed.z<-0.01f)
//		{		
//			// splash when car goes in deep water
//			g_fx.TriggerWaterSplash(this->GetPosition());
//		}
				
//		g_fx.TriggerWaterRipples(this->GetPosition());
		// ----------------------------------------------
		
		// MN - make planes explode if they hit water at high speed
		if (GetVehicleType()==VEHICLE_TYPE_PLANE && m_vecMoveSpeed.z<-1.0f)
		{
			BlowUpCar(this, false);
		}
		
		m_nPhysicalFlags.bForceFullWaterCheck = true;
		float fBuoyancyFraction = BuoyancyForce.z/(m_fMass*PHYSICAL_GRAVITYFORCE*MAX(MIN_TIMESTEP, CTimer::GetTimeStep()) );
		// sometimes buoyancy is set so it won't support the full weight of the vehicle and it will sink
		if(m_fBuoyancyConstant < m_fMass * PHYSICAL_GRAVITYFORCE)
			fBuoyancyFraction *= 1.05f*(m_fMass * PHYSICAL_GRAVITYFORCE) / m_fBuoyancyConstant;
		// the SET_CAR_HEAVY command doesnt set the buoyancyConstant as high as the mass so that car will sink
		if(this->m_nPhysicalFlags.bExtraHeavy)
			fBuoyancyFraction *= 3.0f/2.0f;
		
		float fPow = CMaths::Pow(CMaths::Max(0.5f, (1.0f-0.05f*fBuoyancyFraction)), CTimer::GetTimeStep());

/*		// JIMMY BOND CODE
		if(GetModelIndex()==MODELID_CAR_SENTINEL)
		{
			bIsDrowning = false;
			IsInWater = false;
			CentreOfBuoyancy += MOVE_UNDERWATER_CAR_COB*GetMatrix().GetForward();
			
			if(fBuoyancyFraction > 0.4f && m_fTransformPosition==JIMMY_TRANSFORM_TOTAL_TIME)
			{
				FlyingControl(FLIGHTMODEL_PLANE);
				bMessingAboutInWater = true;
			}
			
			if(fBuoyancyFraction > 0.8f && m_nTransformState==0 && m_fTransformPosition<JIMMY_TRANSFORM_TOTAL_TIME)
			{
				m_nTransformState = 1;
			}
			else if(fBuoyancyFraction < 0.6f && m_nTransformState==0 && m_fTransformPosition>0.0f
			&& (m_aRatioHistory[0] < 1.0f || m_aRatioHistory[1] < 1.0f || m_aRatioHistory[2] < 1.0f || m_aRatioHistory[3] < 1.0f))
			{
				m_nTransformState = -1;
			}
			
			if(bMessingAboutInWater && GetStatus()==STATUS_PLAYER)
			{
				if(m_vecMoveSpeed.MagnitudeSqr() > MOVE_UNDERWATER_BUBBLE_MOVESPDSQR
				&& (CMaths::Abs(CPad::GetPad(0)->CPad::GetPad(0)->GetSteeringLeftRight()) > 100.0f
				|| CMaths::Abs(CPad::GetPad(0)->CPad::GetPad(0)->GetSteeringUpDown()) > 100.0f) )
				{
					if( (CGeneral::GetRandomNumber() &7) > 3 )
					{
						// do trails from fins
/*						CVector vecFinPos(0.0f,0.0f,0.0f);
						GetComponentWorldPosition(CAR_FIN_RF, vecFinPos);
						vecFinPos += 0.25f*GetMatrix().GetRight();
						CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecFinPos, 0.75f*m_vecMoveSpeed, NULL, 0.2f , rgbaSplashWaterColor, CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f), 0, CGeneral::GetRandomNumberInRange(200, 600));

						vecFinPos = CVector(0.0f,0.0f,0.0f);
						GetComponentWorldPosition(CAR_FIN_RR, vecFinPos);
						vecFinPos += 0.25f*GetMatrix().GetRight();
						CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecFinPos, 0.75f*m_vecMoveSpeed, NULL, 0.2f , rgbaSplashWaterColor, CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f), 0, CGeneral::GetRandomNumberInRange(200, 600));

						vecFinPos = CVector(0.0f,0.0f,0.0f);
						GetComponentWorldPosition(CAR_FIN_LF, vecFinPos);
						vecFinPos -= 0.25f*GetMatrix().GetRight();
						CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecFinPos, 0.75f*m_vecMoveSpeed, NULL, 0.2f , rgbaSplashWaterColor, CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f), 0, CGeneral::GetRandomNumberInRange(200, 600));

						vecFinPos = CVector(0.0f,0.0f,0.0f);
						GetComponentWorldPosition(CAR_FIN_LR, vecFinPos);
						vecFinPos -= 0.25f*GetMatrix().GetRight();
						CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecFinPos, 0.75f*m_vecMoveSpeed, NULL, 0.2f , rgbaSplashWaterColor, CGeneral::GetRandomNumberInRange(0.0f, 0.4f), CGeneral::GetRandomNumberInRange(0.0f, 45.0f), 0, CGeneral::GetRandomNumberInRange(200, 600));
*//*					}
				}
				
				if( (CTimer::GetTimeInMilliseconds()&15) < (CGeneral::GetRandomNumber() &7) )
				{
					// do bubbles from cockpit
					
				}
				
				if((CGeneral::GetRandomNumber() &255) > 240)
				{
					AudioManager.ProcessUnderWaterCar(2000);
				}
			}
		}
*/
		if(GetModelIndex()==MODELID_CAR_VORTEX && GetMatrix().GetUp().z > 0.3f
		&& m_nVehicleFlags.bIsDrowning==false)
		{
			bMessingAboutInWater = true;
		}
		
		if(!bMessingAboutInWater)
		{
			m_vecMoveSpeed *= fPow;
			m_vecTurnSpeed *= fPow;
		}
		
		bool bDoBigHeliSplash = false;
		if( (pHandling->mFlags &MF_IS_HELI) && m_aWheelAngularVelocity[1] > MIN_ROT_SPEED_HELI_CONTROL)
		{
			if(GetModelIndex()!=MODELID_HELI_SEASPARROW && GetModelIndex()!=MODELID_HELI_LEVIATHN)
			{
				float fSuckDownMult = MAX(1.0f, 8.0f*fBuoyancyFraction);
				ApplyMoveForce(-2.0f*BuoyancyForce / fSuckDownMult);
				
				// do some extra damping
				//m_vecMoveSpeed *= fPow;
				m_vecTurnSpeed *= fPow;
				
//				ApplyTurnForce(-BuoyancyForce / fSuckDownMult, CentreOfBuoyancy);
				if(fBuoyancyFraction > 0.9f)
				{
					m_aWheelAngularVelocity[1] = 0.0f;
					bDoBigHeliSplash = true;
				}
				else
					return;
			}
			else if(fBuoyancyFraction > 3.0f)
			{
				m_aWheelAngularVelocity[1] = 0.0f;
				bDoBigHeliSplash = true;
			}
		}
	
		m_nPhysicalFlags.bForceFullWaterCheck = true;
		ApplyMoveForce(BuoyancyForce);
		ApplyTurnForce(BuoyancyForce, CentreOfBuoyancy);
		// some info for splashes
		CVector vecOldMoveSpeed = m_vecMoveSpeed;
		

		// sea sparrow can float, so don't take damage unless it really piles into the water (or is upsidedown)
		if((GetModelIndex()==MODELID_HELI_SEASPARROW || GetModelIndex()==MODELID_HELI_LEVIATHN)
		&& fBuoyancyFraction < 3.0f	&& (GetMatrix().GetUp().z > -0.5f || fBuoyancyFraction < 0.6f) )
		{
			m_nPhysicalFlags.bIsInWater = false;
			m_nVehicleFlags.bIsDrowning = false;
		}
/*
		// JIMMY BOND STUFF
		else if( GetModelIndex()==MODELID_CAR_SENTINEL && GetStatus()==STATUS_PLAYER)
		{
			IsInWater = false;
			bIsDrowning = false;
		}
*/
		else if((CCheat::IsCheatActive(CCheat::BACKTOTHEFUTURE_CHEAT) || GetModelIndex()==MODELID_CAR_VORTEX)
		&& GetStatus()==STATUS_PLAYER && GetMatrix().GetUp().z > 0.3f && m_nVehicleFlags.bIsDrowning==false)
		{
			m_nPhysicalFlags.bIsInWater = false;
			m_nVehicleFlags.bIsDrowning = false;
		}
		// cars only take damage if buoyancy force is enough to lift it off the ground
		// OR one of it's wheels is already off the ground and the force is greater than 60% of its weight
		else if(bDoBigHeliSplash || fBuoyancyFraction >= 1.0f || (fBuoyancyFraction > 0.6f && 
		(  m_aWheelRatios[0]==BILLS_EXTENSION_LIMIT || m_aWheelRatios[1]==BILLS_EXTENSION_LIMIT
		|| m_aWheelRatios[2]==BILLS_EXTENSION_LIMIT || m_aWheelRatios[3]==BILLS_EXTENSION_LIMIT)) )
		{
			m_nPhysicalFlags.bIsInWater = TRUE;
			m_nVehicleFlags.bIsDrowning = true;
			if(m_vecMoveSpeed.z < -0.1f)
				m_vecMoveSpeed.z = -0.1f;
				
			if(m_fBuoyancyConstant > m_fMass*PHYSICAL_GRAVITYFORCE*100.0f/125.0f)
			{
				m_fBuoyancyConstant -= 0.001f*m_fMass*PHYSICAL_GRAVITYFORCE;
			}
			// car is sunk - turn the engine off!
			if(m_fBuoyancyConstant < m_fMass*PHYSICAL_GRAVITYFORCE)
			{
				m_nVehicleFlags.bEngineOn = false;
			}

			// Inflict damage to the guys in the car
			if (pDriver) 
			{
				AssertEntityPointerValid_NotInWorld(pDriver);
				// make sure driver does full water check when he gets out
				pDriver->m_nPhysicalFlags.bForceFullWaterCheck = true;
				if(pDriver->IsPlayer() || !m_nAutomobileFlags.bWaterTight)
				{	//	If the car is watertight then non-player characters are safe inside
					if(GetVehicleType()==VEHICLE_TYPE_QUADBIKE && m_aWheelRatios[0]==BILLS_EXTENSION_LIMIT && m_aWheelRatios[1]==BILLS_EXTENSION_LIMIT
					&& m_aWheelRatios[2]==BILLS_EXTENSION_LIMIT && m_aWheelRatios[3]==BILLS_EXTENSION_LIMIT)
					{
						CVector vecNorm = -m_vecMoveSpeed;
						vecNorm.Normalise();
						CEventKnockOffBike event(this, m_vecMoveSpeed, vecNorm, m_vecMoveSpeed.Magnitude()*m_fMass, 0, WEAPONTYPE_DROWNING, 0);
						pDriver->GetPedIntelligence()->AddEvent(event);
						
						if (pDriver->IsPlayer())
							((CPlayerPed*)pDriver)->HandlePlayerBreath(TRUE);

					
						m_nVehicleFlags.bEngineOn = false;
					}
					else
					{
						// damage ped
						if (pDriver->IsPlayer())
						{
							((CPlayerPed*)pDriver)->HandlePlayerBreath(TRUE);
						}
						else
						{
							CPedDamageResponseCalculator damageResponseCalculator(this, CTimer::GetTimeStep(), WEAPONTYPE_DROWNING, PED_SPHERE_CHEST, false);
							CEventDamage event(this, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_DROWNING, PED_SPHERE_CHEST, 0, false, true);
							if(event.AffectsPed(pDriver))
								damageResponseCalculator.ComputeDamageResponse(pDriver, event.GetDamageResponseData());
							else
								event.GetDamageResponseData().SetDamageCalculated();
							pDriver->GetPedIntelligence()->AddEvent(event);
						}
					}
				}
				// WARNING! InflictDamage may have cleared the pDriver pointer
			}
			for(int32 i = 0; i < m_nMaxPassengers; i++)
			{
				if(pPassengers[i])
				{
					AssertEntityPointerValid_NotInWorld(pPassengers[i]);
					// make sure passengers do full water check when they get out
					pPassengers[i]->m_nPhysicalFlags.bForceFullWaterCheck = true;

					if ((pPassengers[i]->IsPlayer()) || (!m_nAutomobileFlags.bWaterTight))
					{	//	If the car is watertight then non-player characters are safe inside
						if(GetVehicleType()==VEHICLE_TYPE_QUADBIKE && m_aWheelRatios[0]==BILLS_EXTENSION_LIMIT && m_aWheelRatios[1]==BILLS_EXTENSION_LIMIT
						&& m_aWheelRatios[2]==BILLS_EXTENSION_LIMIT && m_aWheelRatios[3]==BILLS_EXTENSION_LIMIT)
						{
							CVector vecNorm = -m_vecMoveSpeed;
							vecNorm.Normalise();
							CEventKnockOffBike event(this, m_vecMoveSpeed, vecNorm, m_vecMoveSpeed.Magnitude()*m_fMass, 0, WEAPONTYPE_DROWNING, 0);
							pPassengers[i]->GetPedIntelligence()->AddEvent(event);

							if (pPassengers[i]->IsPlayer())
								((CPlayerPed*)pPassengers[i])->HandlePlayerBreath(TRUE);
						}
						else
						{
							// damage passengers
							if (pPassengers[i]->IsPlayer())
							{
								((CPlayerPed*)pPassengers[i])->HandlePlayerBreath(TRUE);
							}
							else
							{
								CPedDamageResponseCalculator damageResponseCalculator(this, CTimer::GetTimeStep(), WEAPONTYPE_DROWNING, PED_SPHERE_CHEST, false);
								CEventDamage event(this, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_DROWNING, PED_SPHERE_CHEST, 0, false, true);
								if(event.AffectsPed(pPassengers[i]))
									damageResponseCalculator.ComputeDamageResponse(pPassengers[i], event.GetDamageResponseData());
								else
									event.GetDamageResponseData().SetDamageCalculated();
								pPassengers[i]->GetPedIntelligence()->AddEvent(event);
							}
						}
					}
					// WARNING! InflictDamage may have cleared the pPassenger pointer
				}
			}
			
		}
		else{
			m_nPhysicalFlags.bIsInWater = false;	
			m_nVehicleFlags.bIsDrowning = false;
		}

/*
#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (ParticleObjects)	
// car water splash - add water splash particle objects

		// this variable tells us when to start generate raindrops:
		static
		uint32 nGenerateRaindrops = 0, nGenerateWaterCircles = 0;




#define ZPOSMIN			(-0.1f)
#define ZPOSMAX			(-0.2f)

#define SPLASH_VEL_RAD	(0.01f)
//#define SPLASH_POS_RAD	(1.3f)
//#define SPLASH_POS_RAD_DEF	(4.5f)
#define SPLASH_POS_RAD	(4.5f)

#define ZVELMIN			(0.15f)//(0.25f)
#define ZVELMAX			(0.45f)//(0.45f)


		//
		// Car water splash (deep water):
		//
		if(( (vecOldMoveSpeed.z < -0.1f && fBuoyancyFraction > 0.3f)
		|| bDoBigHeliSplash) && !bMessingAboutInWater)
		{
			//
			// particles - big splash
			//
			//printf(" ** Automobile Splash!\n\n");
				
			const float fVelMag		= vecOldMoveSpeed.Magnitude();
			
			//CVector vecSplashVel;//3f*vecOldMoveSpeed);
			//vecSplashVel.z *= -0.6f*fVelMag;
			
			//CVector vecSplashPos = this->GetPosition() + CentreOfBuoyancy + vecOldMoveSpeed*CTimer::GetTimeStep();
			const CVector vecSplashPos = this->GetPosition();// + CentreOfBuoyancy + vecOldMoveSpeed*2.0f;//CTimer::GetTimeStep();

			//const RwRGBA dummyColor = {0,0,0,0};
			//CVector vecPosAdd, vecVelAdd;
			
			
//6			CVector bbox_min(CModelInfo::GetColModel(this->GetModelIndex()).GetBoundBoxMin());
//6			CVector bbox_max(CModelInfo::GetColModel(this->GetModelIndex()).GetBoundBoxMax());
//6			printf(" y_min=%.2f, y_max=%.2f, y=%.2f \n", bbox_min.y, bbox_max.y, CMaths::Abs(bbox_min.y)+CMaths::Abs(bbox_max.y));
			 
//6			float SPLASH_POS_RAD = CMaths::Abs(bbox_min.y);//+CMaths::Abs(bbox_max.y)-2.5f;
//6			if(SPLASH_POS_RAD < SPLASH_POS_RAD_DEF)
//6				SPLASH_POS_RAD = SPLASH_POS_RAD_DEF;
			
//6			int32 PIECE_NUM = int32(SPLASH_POS_RAD);//-1.5f);
//6			if(PIECE_NUM < 3)
//6				PIECE_NUM = 3;

//6			printf(" SPLASH_POS_RAD = %.2f, PIECE_NUM = %d\n", SPLASH_POS_RAD, PIECE_NUM);



//			for(float Cir=0.0f; Cir < 360.0f; Cir += 4.0f)
//			{
//				const float Xoff = CMaths::Sin(Cir);
//				const float Yoff = CMaths::Cos(Cir);
//				
//				const float ZVel = CGeneral::GetRandomNumberInRange(ZVELMIN, ZVELMAX);
//				//const float ZPos = CGeneral::GetRandomNumberInRange(ZPOSMIN, ZPOSMAX);
//
//				CVector vecSplashVel= CVector(Xoff * SPLASH_VEL_RAD, Yoff * SPLASH_VEL_RAD, ZVel);
//				CVector vecPosAdd	= CVector(Xoff * SPLASH_POS_RAD, Yoff * SPLASH_POS_RAD, 0);
//				
//				CParticle::AddParticle(	PARTICLE_CAR_SPLASH,
//										vecSplashPos+vecPosAdd,
//										vecSplashVel,		
//										NULL, 0.0f, rgbaSplashWaterColor);//dummyColor );
//
//			
//				// add some splashes inside the circle:
//				const int32 PIECE_NUM = 3;
//				
//				for(int32 i=0; i<PIECE_NUM; i++)
//				{
//					const float len = (i+1)*(SPLASH_POS_RAD/float(PIECE_NUM));// CGeneral::GetRandomNumberInRange(0.0f, SPLASH_POS_RAD);
//
//					CVector vecPosAdd	= CVector(Xoff*len, Yoff*len, 0);
//					CParticle::AddParticle(	PARTICLE_CAR_SPLASH,
//											vecSplashPos+vecPosAdd,
//											vecSplashVel,		
//											NULL, 0.0f, rgbaSplashWaterColor);//dummyColor );
//				}				
//
//			}
//
	
			CVector vecSplashVel(0.0f, 0.0f, CGeneral::GetRandomNumberInRange(ZVELMIN, ZVELMAX));
			CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, vecSplashPos, vecSplashVel, 0.0f,
								 		75, rgbaSplashWaterColor, TRUE);
	

			nGenerateRaindrops		= CTimer::GetTimeInMilliseconds() + 300;//1200;
			nGenerateWaterCircles	= CTimer::GetTimeInMilliseconds() + 60;

			// do more big splashes for heli crashing into the water!
			if(bDoBigHeliSplash)
			{
				CVector vecSide = CrossProduct(GetMatrix().GetForward(), CVector(0.0f,0.0f,1.0f));
				CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, vecSplashPos + vecSide, vecSplashVel, 0.0f,
										75, rgbaSplashWaterColor, TRUE);
				CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, vecSplashPos - vecSide, vecSplashVel, 0.0f,
										75, rgbaSplashWaterColor, /TRUE);
			}
						
				
				
			// apply sharp change in velocity
//			fPow = CMaths::Pow(0.3f, CTimer::GetTimeStep());
			if(m_vecMoveSpeed.z < -0.2f)
				m_vecMoveSpeed.z = -0.2f;	// was -0.01
//			vecOldMoveSpeed.x *= fPow;
//			vecOldMoveSpeed.y *= fPow;
			
//			ApplyTurnForce( (CVector) (5*BuoyancyForce), (CVector) (5*CentreOfBuoyancy) );
				
			// printf("SPLASHHHHHH !!!\n");
			DMAudio.PlayOneShot(AudioHandle, AE_CAR_SPLASH, 0.0f);
		}//if(vecOldMoveSpeed.z < -0.3f)...


//			//
//			// calculate current water color here: 
//			//
//			const RwRGBA dummyColor = {0,0,0,0};
//			const float COLOR_SCALE = 0.6f;//0.60f;
//			uint8 Red	= COLOR_SCALE * 255.0f * (CTimeCycle::m_fCurrentAmbientRed	+ CTimeCycle::m_fCurrentDirectionalRed	* 0.8f);
//			uint8 Green	= COLOR_SCALE * 255.0f * (CTimeCycle::m_fCurrentAmbientGreen+ CTimeCycle::m_fCurrentDirectionalGreen* 0.8f);
//			uint8 Blue	= COLOR_SCALE * 255.0f * (CTimeCycle::m_fCurrentAmbientBlue	+ CTimeCycle::m_fCurrentDirectionalBlue * 0.8f);
//			const RwRGBA rgbaSplashWaterColor = {Red, Green, Blue, 255};
//				
//			const float fVelMag		= vecOldMoveSpeed.Magnitude();
//				
//			CVector vecSplashVel(0.2f * vecOldMoveSpeed);
//			vecSplashVel.z = 0.35f;//12f;//0.08f;//0.16f;//*= -0.9f*fVelMag;//-0.2f*fVelMag;
//				
//				
//			// get position in ~2.0 sec:
//			CVector vecSplashPos = this->GetPosition() + CentreOfBuoyancy + vecOldMoveSpeed*2.0f;//CTimer::GetTimeStep();
//			//vecSplashPos.z -= 1.0f;
//			
//			float zet = 0.0f;
//			if(CWaterLevel::GetWaterLevel(vecSplashPos.x, vecSplashPos.y, vecSplashPos.z, &zet))
//				vecSplashPos.z = zet + 0.5f;
			//
//			//
//			// big water splash:
//			//
//			//printf(" ** Automobile Splash!\n\n");
//			CParticleObject::AddObject(POBJECT_CAR_WATER_SPLASH, vecSplashPos, vecSplashVel,
//							0.0f, 750, rgbaSplashWaterColor, TRUE);
				
				
			

#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (AddParticle)
// car water splash

		if(nGenerateWaterCircles)
		if(CTimer::GetTimeInMilliseconds() >= nGenerateWaterCircles)
		{
			CVector vecSplashPos2(this->GetPosition());
			
			float zet = 0.0f;
			if(CWaterLevel::GetWaterLevel(vecSplashPos2.x, vecSplashPos2.y, vecSplashPos2.z, &zet))
				vecSplashPos2.z = zet;
					
			if(vecSplashPos2.z != 0.0f)
			{
				nGenerateWaterCircles = 0;
				vecSplashPos2.z += 1.0f;

				const RwRGBA dummyColor = {0,0,0,0};

				for(int32 i=0; i<4; i++)
				{
					const float RANGE = 2.5f;
					CVector pos(vecSplashPos2);
					pos.x += CGeneral::GetRandomNumberInRange(-RANGE, RANGE);
					pos.y += CGeneral::GetRandomNumberInRange(-RANGE, RANGE);
						
					CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW, pos, CVector(0,0,0), NULL, 0.0f, dummyColor);
				}
			}
		}
#endif // MN - OLD FX SYSTEM (AddParticle)


		if(nGenerateRaindrops)
		if(CTimer::GetTimeInMilliseconds() >= nGenerateRaindrops)
		{
			//const float speed_abs = CMaths::Abs(vecOldMoveSpeed.x) + CMaths::Abs(vecOldMoveSpeed.y);
			//if(speed_abs < 0.1f)
			{
				CVector vecSplashPos2(this->GetPosition());
					
				float zet = 0.0f;
				if(CWaterLevel::GetWaterLevel(vecSplashPos2.x, vecSplashPos2.y, vecSplashPos2.z, &zet))
					vecSplashPos2.z = zet;
							
				if(vecSplashPos2.z >= 0.0f)
				{
					//printf(" ** Splash zet = %.2f\n", vecSplashPos2.z);

					nGenerateRaindrops = 0;

					vecSplashPos2.z += 0.5f;
			
					const RwRGBA dummyColor = {0,0,0,0};
					//
					// rain splashes around (radius = 6.5m):
					//
					CParticleObject::AddObject(POBJECT_SPLASHES_AROUND, vecSplashPos2, CVector(0,0,0),
									6.5f, 2500, dummyColor, TRUE);

				}
			}
		}//if(nGenerateRaindrops)...
#endif // MN - OLD FX SYSTEM (ParticleObjects)	
*/
#if 0
/*
/////////////////////////////////////////////////////////////////////////////////////////
		//
		// Car water splash:
		//
		if(vecOldMoveSpeed.z < -0.3f)
		{
		
			//
			// calculate current water color here: 
			//
			const RwRGBA dummyColor = {0,0,0,0};
			const float COLOR_SCALE = 0.7f;//0.60f;
			uint8 Red	= COLOR_SCALE * 255.0f * (CTimeCycle::m_fCurrentAmbientRed	+ CTimeCycle::m_fCurrentDirectionalRed	* 0.5f);
			uint8 Green	= COLOR_SCALE * 255.0f * (CTimeCycle::m_fCurrentAmbientGreen+ CTimeCycle::m_fCurrentDirectionalGreen* 0.5f);
			uint8 Blue	= COLOR_SCALE * 255.0f * (CTimeCycle::m_fCurrentAmbientBlue	+ CTimeCycle::m_fCurrentDirectionalBlue * 0.5f);
			const RwRGBA rgbaSplashWaterColor = {Red, Green, Blue, 255};

			//
			// particles - big splash
			//
			printf(" ** Automobile Splash!\n\n");
				
			const float fVelMag		= vecOldMoveSpeed.Magnitude();
			
			CVector vecSplashVel(0.6f*vecOldMoveSpeed);
			vecSplashVel.z *= -0.2f*fVelMag;
				
			const CVector vecSplashPos = this->GetPosition() + CentreOfBuoyancy + vecOldMoveSpeed*CTimer::GetTimeStep();

			//const RwRGBA dummyColor = {0,0,0,0};
			CVector vecPosAdd, vecVelAdd;
				
				
			for(int32 i=-3; i<4; i++)
			{
				const float SPLASH_RAD_POS	= 0.5f;
				const float SPLASH_RAD_VEL	= 0.05f;
				const float fi = float(i);
				
				const float MIN_SPD = 0.03f;
				const float MAX_SPD = 0.06f;
				
				const float MIN_SPDZ = 0.6f;//0.05f;
				const float MAX_SPDZ = 0.8f;//0.10f;

				vecPosAdd = CVector(-3.0f*SPLASH_RAD_POS,	fi*SPLASH_RAD_POS,		0.0f);
				//vecVelAdd = CVector(-3.0f*SPLASH_RAD_VEL,	fi*SPLASH_RAD_VEL,	0.1f);
				vecVelAdd.x = -3.0f	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.y = fi	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.z = CGeneral::GetRandomNumberInRange(MIN_SPDZ, MAX_SPDZ);//0.1f);
				CParticle::AddParticle(PARTICLE_CAR_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 0.0f, rgbaSplashWaterColor);//dummyColor );



				vecPosAdd = CVector(3.0f*SPLASH_RAD_POS,	fi*SPLASH_RAD_POS,		0.0f);
				//vecVelAdd = CVector(3.0f*SPLASH_RAD_VEL,	fi*SPLASH_RAD_VEL,	0.1f);
				vecVelAdd.x = 3.0f	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.y = fi	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.z = CGeneral::GetRandomNumberInRange(MIN_SPDZ, MAX_SPDZ);//0.1f);
				CParticle::AddParticle(PARTICLE_CAR_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 0.0f, rgbaSplashWaterColor);//dummyColor );


				vecPosAdd = CVector(fi*SPLASH_RAD_POS,		-3.0f*SPLASH_RAD_POS,	0.0f);
				//vecVelAdd = CVector(fi*SPLASH_RAD_VEL*vecSPEED_Z.x,	-3.0f*SPLASH_RAD_VEL*vecSPEED_Z.y,	vecSPEED_Z.z);//0.1f);
				vecVelAdd.x = fi	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.y = -3.0f	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.z = CGeneral::GetRandomNumberInRange(MIN_SPDZ, MAX_SPDZ);//0.1f);
				CParticle::AddParticle(PARTICLE_CAR_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 0.0f, rgbaSplashWaterColor);//dummyColor );



				vecPosAdd = CVector(fi*SPLASH_RAD_POS,		3.0f*SPLASH_RAD_POS,	0.0f);
				//vecVelAdd = CVector(fi*SPLASH_RAD_VEL*vecSPEED_Z.x,	3.0f*SPLASH_RAD_VEL*vecSPEED_Z.y,	vecSPEED_Z.z);//0.1f);
				vecVelAdd.x = fi	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.y = 3.0f	* CGeneral::GetRandomNumberInRange(MIN_SPD, MAX_SPD);
				vecVelAdd.z = CGeneral::GetRandomNumberInRange(MIN_SPDZ, MAX_SPDZ);
				CParticle::AddParticle(PARTICLE_CAR_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 0.0f, rgbaSplashWaterColor);//dummyColor );
			}
				
			// apply sharp change in velocity
//			fPow = CMaths::Pow(0.3f, CTimer::GetTimeStep());
			if(m_vecMoveSpeed.z < -0.2f)
				m_vecMoveSpeed.z = -0.2f;	// was -0.01
//			m_vecMoveSpeed.x *= fPow;
//			m_vecMoveSpeed.y *= fPow;
				
//			ApplyTurnForce( (CVector) (5*BuoyancyForce), (CVector) (5*CentreOfBuoyancy) );
				
			// printf("SPLASHHHHHH !!!\n");
			DMAudio.PlayOneShot(AudioHandle, AE_CAR_SPLASH, 0.0f);
		}//if(vecOldMoveSpeed.z < -0.3f)...

/////////////////////////////////////////////////////////////////////////////////////////
*/
#endif//#if 0...


#if 0
/*
/////////////////////////////////////////////////////////////////////////////////////////
		if(vecOldMoveSpeed.z < -0.3)
		{
			// particles - big splash
			float fVelMag = vecOldMoveSpeed.Magnitude();
			CVector vecSplashVel = 0.6*vecOldMoveSpeed;
			vecSplashVel.z *= -0.2*fVelMag;
			CVector vecSplashPos = GetPosition() + CentreOfBuoyancy + vecOldMoveSpeed*CTimer::GetTimeStep();

			CVector vecPosAdd, vecVelAdd;
			for(int i=-3;i<4;i++)
			{
				vecPosAdd = CVector(-3*0.5f, i*0.5f, 0.0f);
				vecVelAdd = CVector(-3*0.05f, i*0.05f, 0.0f);
				CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 2.5f );

				vecPosAdd = CVector(3*0.5f, i*0.5f, 0.0f);
				vecVelAdd = CVector(3*0.05f, i*0.05f, 0.0f);
				CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 2.5f );

				vecPosAdd = CVector(i*0.5f, -3*0.5f, 0.0f);
				vecVelAdd = CVector(i*0.05f, -3*0.05f, 0.0f);
				CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 2.5f );

				vecPosAdd = CVector(i*0.5f, 3*0.5f, 0.0f);
				vecVelAdd = CVector(i*0.05f, 3*0.05f, 0.0f);
				CParticle::AddParticle(PARTICLE_BOAT_SPLASH,  vecSplashPos+vecPosAdd, vecSplashVel+vecVelAdd, NULL, 2.5f );
			}
				
			// apply sharp change in velocity
//			fPow = CMaths::Pow(0.3f, CTimer::GetTimeStep());
			if(m_vecMoveSpeed.z < -0.2f)
				m_vecMoveSpeed.z = -0.2;	// was -0.01
//			m_vecMoveSpeed.x *= fPow;
//			m_vecMoveSpeed.y *= fPow;
				
//			ApplyTurnForce( (CVector) (5*BuoyancyForce), (CVector) (5*CentreOfBuoyancy) );
				
			DMAudio.PlayOneShot(AudioHandle, AE_CAR_SPLASH, 0.0f);
			// printf("SPLASHHHHHH !!!\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
*/
#endif//0...


	}
	else
	{
	
		//
		// car is splashing in shallow water:
		//
		m_nPhysicalFlags.bIsInWater = false;
		m_nVehicleFlags.bIsDrowning = false;
		m_nPhysicalFlags.bForceFullWaterCheck = false;

		// reset buoyancy once car has left the water		
		m_fBuoyancyConstant = pHandling->fBuoyancyConstant;


/*		// JIMMY BOND STUFF
		if(GetModelIndex()==MODELID_CAR_SENTINEL
		&& (m_aRatioHistory[0]<1.0f || m_aRatioHistory[1]<1.0f || m_aRatioHistory[2]<1.0f || m_aRatioHistory[3]<1.0f))
		{
			if(m_fTransformPosition > 0.0f)
				m_nTransformState = -1;
			else if(m_nTransformState!=0)
				m_nTransformState = 0;
		}
*/
		
//		const RwRGBA rgbaSplashWaterColor = {155, 185, 155, 255};
		const RwRGBA rgbaSplashWaterColor = {155, 155, 185, 196};
		const RwRGBA rgbaSplashWaterColor2 = {255, 255, 255, 255};
		CVector vecWheelSpeed;
		CVector vecWheelPos;
		float fTemp = 0.0f;
		float radius = 0.0f;
//		float fVehicleFront = CModelInfo::GetColModel(m_nModelIndex).GetBoundBoxMax().y;
		
		for(int32 i=0; i<4; i++)
		{
			if( m_aWheelRatios[i]<BILLS_EXTENSION_LIMIT	&&
//				CColPoint::IsWater(m_aWheelColPoints[i].GetSurfaceTypeB()))
				g_surfaceInfos.IsWater(m_aWheelColPoints[i].GetSurfaceTypeB()))
			{
				vecWheelPos = m_aWheelColPoints[i].GetPosition() + 0.3f*GetMatrix().GetUp() - this->GetPosition();
				vecWheelSpeed = GetSpeed(vecWheelPos);
				vecWheelSpeed.z = 0.0f;
//				ApplyMoveForce(-0.003f*vecWheelSpeed*m_fMass*CTimer::GetTimeStep());
//				ApplyTurnForce(-0.0001f*vecWheelSpeed*m_fMass*CTimer::GetTimeStep(), vecWheelPos-GetPosition());

				if(/* !((CTimer::m_FrameCounter+i)&3)	&&*/
					(fTemp = vecWheelSpeed.MagnitudeSqr()) > 0.05f*0.05f)
				{
					fTemp = CMaths::Sqrt(fTemp);
					if (fTemp < 0.15f)
					  radius = 0.25f * fTemp;
					else
					  radius = 0.75f * fTemp;
					if (radius > 0.6f)
					 radius = 0.6f;
					vecWheelSpeed = 0.2f*vecWheelSpeed + GetMatrix().GetRight() * 0.2f * fTemp;
//					vecWheelSpeed.z = 0.2f*fTemp;
/*					
#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (AddParticle)
// car shallow water splash

					// do a thrustjet particle from 1 wheel per frame
					//CParticle::AddParticle(PARTICLE_BOAT_THRUSTJET,  vecWheelPos + GetPosition() + 0.3f*GetMatrix().GetUp(), vecWheelSpeed, NULL, fTemp, rgbaSplashWaterColor);
					CParticle::AddParticle(PARTICLE_PED_SPLASH,  vecWheelPos + GetPosition(), -0.50f*vecWheelSpeed, NULL, radius, rgbaSplashWaterColor, CGeneral::GetRandomNumberInRange(0.0f, 10.0f), CGeneral::GetRandomNumberInRange(0.0f, 90.0f), 1);
					CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,  vecWheelPos + GetPosition(), -0.6f*vecWheelSpeed, NULL, radius, rgbaSplashWaterColor2);

#endif // MN - OLD FX SYSTEM (AddParticle)
*/


					// MN - FX SYSTEM -------------------------------
					// TODO: splash when car goes in shallow water		
					// ----------------------------------------------
		
		

					// only do splash circles half as often
//					if( !((CTimer::m_FrameCounter+i)&7) )
//						CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW, vecWheelPos + GetPosition() + 0.5f*GetMatrix().GetUp(), CVector(0,0,0), NULL, 0.0f, rgbaSplashWaterColor);
/*
					switch(i)
					{
						// front left (water spray from the front of the vehicle)
						case(0):
							//CParticle::AddParticle(PARTICLE_CAR_SPLASH,  GetPosition() + fVehicleFront*GetMatrix().GetForward() - 0.5f*GetMatrix().GetRight(), 0.75f*m_vecMoveSpeed, NULL, 0.0f, rgbaSplashWaterColor);
							CParticle::AddParticle(PARTICLE_PED_SPLASH,  GetPosition() + fVehicleFront*GetMatrix().GetForward() - 0.5f*GetMatrix().GetRight(), 0.75f*m_vecMoveSpeed, NULL, 0.0f, rgbaSplashWaterColor);
							break;
											
						// front right
						case(2):
							//CParticle::AddParticle(PARTICLE_CAR_SPLASH,  GetPosition() + fVehicleFront*GetMatrix().GetForward() + 0.5f*GetMatrix().GetRight(), 0.75f*m_vecMoveSpeed, NULL, 0.0f, rgbaSplashWaterColor);
							CParticle::AddParticle(PARTICLE_PED_SPLASH,  GetPosition() + fVehicleFront*GetMatrix().GetForward() + 0.5f*GetMatrix().GetRight(), 0.75f*m_vecMoveSpeed, NULL, 0.0f, rgbaSplashWaterColor);
							break;
					}//switch(i)...					
*/					
					if(!(CTimer::m_FrameCounter&15))
					{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
						DMAudio.PlayOneShot(AudioHandle, AE_BOAT_HIT_WAVE, (float)(2000.0f*fTemp));
#endif //USE_AUDIOENGINE
					}
				}
			}
		
		}//for(int32 i=0; i<4; i++)...
		
	}
	
}//end of void CAutomobile::ProcessBuoyancy()...



//
// BREAK THIS FUNCTION DOWN INTO SMALL TIDY BITS
// most of this has been kept the way Richard wrote it.
// 
TweakFloat BURNOUT_STEER_MULT = -0.002f;
TweakFloat GEARCHANGE_MULT = -0.008f;
TweakUInt32 GEARCHANGE_INTERVAL = 1000;
//
TweakFloat SAND_WHEEL_MOVERES_MULT = (0.005f);
TweakFloat SAND_WHEEL_TURNRES_MULT = (0.993f);
TweakFloat SAND_WHEEL_BOGDOWN_LIMIT = (0.3f);
//
//
bool bHeliControl = false;
//
void CAutomobile::ProcessControl()
{
//////////////////////////////
// Do some debug stuff first (not that useful since rarely play Debug build)
#ifdef DEBUG
	if (pBombOwner)
	{
		AssertEntityPointerValid(pBombOwner);
	}
	
	UInt32 loop;
	for (loop = 0; loop < 4; loop++)
	{
		if (m_aGroundPhysicalPtrs[loop])
		{
			AssertEntityPointerValid(m_aGroundPhysicalPtrs[loop]);
		}
	}
	
	if (pDelayedExplosionInflictor)
	{
		AssertEntityPointerValid(pDelayedExplosionInflictor);
	}
	
	if (pDriver)
	{
		AssertEntityPointerValid_NotInWorld(pDriver);
	}
	
	for (loop = 0; loop < 3; loop++)
	{
		if (pPassengers[loop])
		{
			AssertEntityPointerValid_NotInWorld(pPassengers[loop]);
		}
	}
#endif
///////////////////////////////
	bool8 forceExplode=false;
	static float fThrust;
	float fDriveForce = 0.0f;
	float fFriction = 0.0f;
	uint32 nCarFlags = 0;
	
	if(m_nVehicleFlags.bUseCarCheats)
		nCarFlags |= PROCCONT_FLAG_CAR_CHEATS;
		
	CColModel *pColModel = &GetColModel();
	CCollisionData *pColData = pColModel->GetCollisionData();
	
	ASSERT(pColData);
	
	bool bSkipPhysics = false;
	// reset flag for has car warned peds of it's approach
	m_nVehicleFlags.bWarnedPeds = false;
	nBrakesOn = false;
	m_nAutomobileFlags.bIsBoggedDownInSand = false;
	m_nVehicleFlags.bRestingOnPhysical = false;
	
#ifdef USE_AUDIOENGINE
	// service the audio entity - this should be done once a frame 
	// we want to ALWAYS do it, even if car is totally stationary and inert
	m_VehicleAudioEntity.Service();
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE


	if(ShouldWeSkipProcessControl())
		return;
		
				
	if ((GetVehicleType()==VEHICLE_TYPE_PLANE || GetVehicleType()==VEHICLE_TYPE_HELI) &&
		(AutoPilot.Mission==CAutoPilot::MISSION_PLANE_CRASH_AND_BURN ||
		 AutoPilot.Mission==CAutoPilot::MISSION_HELI_CRASH_AND_BURN ||
		 (GetStatus()==STATUS_PLAYER&&m_nHealth<CAR_ON_FIRE_HEALTH&&m_statusWhenEngineBlown==EBS_IN_AIR)) && GetStatus()!=STATUS_WRECKED)
	{
		if ((GetDamageImpulseMagnitude()>0.0f && m_vecDamageNormal.z>0.0f) || 
			!IsInAir())
		{
			if (GetMoveSpeed().z>=0.0f || m_nPhysicalFlags.bIsInWater)
			{
				forceExplode = true;
			}
		}
	}

	if(CCheat::IsCheatActive(CCheat::TAXINITRO_CHEAT) && GetStatus()==STATUS_PLAYER && (GetModelIndex()==MODELID_CAR_TAXI || GetModelIndex()==MODELID_CAR_CABBIE))
	{
		nCarFlags += PROCCONT_FLAG_TAXI_WITHNITRO;
	}

	if (CCheat::IsCheatActive(CCheat::NITRO_CHEAT) || (nCarFlags &PROCCONT_FLAG_TAXI_WITHNITRO))
	{
		hFlagsLocal |= HF_NOS_INSTALLED;
		m_nNitroBoosts = 101;
	}

	// and moving cars stir up any rubbish around them
	//CRubbish::StirUp(this);
	
	// need to process a few things now in case fn returns e.g. for simple cars
	if(m_nVehicleFlags.bIsBus)	ProcessAutoBusDoors();
	ProcessCarAlarm();
	
	// do blending in of cars in the distance
	UpdateClumpAlpha();

	//////////////////////////////////
	// now do some logic for the peds in the car
	UpdatePassengerList();

	if(pDriver)
	{
		AssertEntityPointerValid(pDriver);
		// New driver and we have a bomb on board -> BOOOM!
		if(!m_nVehicleFlags.bDriverLastFrame && BombOnBoard==5)
		{
			DelayedExplosion = 1000;
			pDelayedExplosionInflictor = pBombOwner;
			if (pDelayedExplosionInflictor) REGREF(pDelayedExplosionInflictor, &pDelayedExplosionInflictor);
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_WARNING_IGNITION_ACTIVATED_CAR_BOMB, 1.0f);
#endif //USE_AUDIOENGINE
		}
		m_nVehicleFlags.bDriverLastFrame = true;

		/*		
		//Done in car tasks now.
		if(IsUpsideDown() && CanPedEnterCar())
		{
			if(!pDriver->IsPlayer() && pDriver->GetCharCreatedBy()!=MISSION_CHAR
			&& (!pDriver->pLeader || !pDriver->pLeader->m_nPedFlags.bInVehicle))
			{
				pDriver->SetObjective(LEAVE_CAR, this);
			}
		}
		*/
	}
	else
	{
		m_nVehicleFlags.bDriverLastFrame = false;
	}
	if(m_nNumPassengers > 0 && IsUpsideDown() && CanPedEnterCar())
	{
		/*
		//Done in car tasks now.
		for(int32 i=0; i<m_nMaxPassengers; i++)
		{
			if(pPassengers[i])
			{
				AssertEntityPointerValid(pPassengers[i]);
				if((!pPassengers[i]->IsPlayer())&&((!pPassengers[i]->pLeader)||(!pPassengers[i]->pLeader->m_nPedFlags.bInVehicle))&&(pPassengers[i]->GetCharCreatedBy()!=MISSION_CHAR))
				{
					pPassengers[i]->SetObjective(LEAVE_CAR, this);
				}
			}
		}
		*/
	}

	//////////////////////////////////////////////////////
	// this calls all the general AI stuff for steering and controlling cars
	// either by player control inputs or actual AI control
	// (only returns true if wanting to return from ProcessControl straight away)
	if( ProcessAI(nCarFlags) )
		return;

	//////////////////////////////////////
	// Abandoned cars (parked and stuff) will be set to static after a wee while
	bSkipPhysics = false;
	if((GetStatus() == STATUS_ABANDONED || GetStatus()==STATUS_WRECKED))// && !GetIsStuck())
	{
		bool bForceStatic = false;
		if(m_nVehicleFlags.bVehicleColProcessed==false && m_vecMoveSpeed.x==0.0f && m_vecMoveSpeed.y==0.0f && m_vecMoveSpeed.z==0.0f
		&& !(m_aRatioHistory[3]==1.0f && m_aRatioHistory[3]==1.0f && m_aRatioHistory[3]==1.0f && m_aRatioHistory[3]==1.0f) )
			bForceStatic = true;
	
		float fMoveSpeedLimit, fTurnSpeedLimit, fDistanceLimit;
		if(GetVehicleType()==VEHICLE_TYPE_PLANE){
			if(GetDamageImpulseMagnitude()>0.0f && GetDamageEntity() && GetDamageEntity()->GetIsTypeVehicle())
				fMoveSpeedLimit = 1.0f*PHYSICAL_MAXMOVESPEEDBEFOREBECOMESSTATIC;
			else
				fMoveSpeedLimit = 3.0f*PHYSICAL_MAXMOVESPEEDBEFOREBECOMESSTATIC;
			fTurnSpeedLimit = 1.0f*PHYSICAL_MAXTURNSPEEDBEFOREBECOMESSTATIC;
			fDistanceLimit = 0.10f;
		}
		else if(GetStatus()==STATUS_WRECKED){
			fMoveSpeedLimit = 2.0f*PHYSICAL_MAXMOVESPEEDBEFOREBECOMESSTATIC;
			fTurnSpeedLimit = 0.5f*PHYSICAL_MAXTURNSPEEDBEFOREBECOMESSTATIC;
			fDistanceLimit = 0.015f;
		}
		else{
			fMoveSpeedLimit = PHYSICAL_MAXMOVESPEEDBEFOREBECOMESSTATIC;
			fTurnSpeedLimit = 0.3f*PHYSICAL_MAXTURNSPEEDBEFOREBECOMESSTATIC;
			fDistanceLimit = 0.005f;
		}
		
		m_vecAverageMoveSpeed = (m_vecAverageMoveSpeed + m_vecMoveSpeed) / 2.0f;
		m_vecAverageTurnSpeed = (m_vecAverageTurnSpeed + m_vecTurnSpeed) / 2.0f;
		if( ( (m_vecAverageMoveSpeed.MagnitudeSqr() <= CMaths::Sqr(fMoveSpeedLimit * CTimer::GetTimeStep()))
		&& ( m_vecAverageTurnSpeed.MagnitudeSqr() <= CMaths::Sqr(fTurnSpeedLimit * CTimer::GetTimeStep())) 
		&& GetDistanceTravelled() < fDistanceLimit
		&& !(GetDamageImpulseMagnitude()>0.0f && GetDamageEntity() && GetDamageEntity()->GetIsTypePed()))// && GetVehicleType()!=VEHICLE_TYPE_PLANE))
		&& !(m_nPhysicalFlags.bIsInWater && (GetPosition() - TheCamera.GetPosition()).MagnitudeSqr() <50.0f*50.0f)
		|| bForceStatic)
		{
			m_nNoOfStaticFrames++;
			if (m_nNoOfStaticFrames > PHYSICAL_MAXNOOFSTATICFRAMES || bForceStatic)
			{
				if (!CCarCtrl::MapCouldMoveInThisArea(GetPosition().x, GetPosition().y))
				{
					// want to let the static counter build up naturally if bForceStatic flag set
					if(!bForceStatic || m_nNoOfStaticFrames > PHYSICAL_MAXNOOFSTATICFRAMES)
						m_nNoOfStaticFrames = PHYSICAL_MAXNOOFSTATICFRAMES;
				
					bSkipPhysics = true;
				
					// If we skip the physics we have to make sure the guy doesn't move at all.
					// Otherwise the car would just sort of float
					m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
					m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
				}
			}
		}
		else
			m_nNoOfStaticFrames = 0;
			
		if( (pHandling->mFlags &MF_IS_HELI) && m_aWheelAngularVelocity[1] > 0.0f)
		{
			bSkipPhysics = false;
			m_nNoOfStaticFrames = 0;
		}
		else if((GetModelIndex()==MODELID_HELI_SEASPARROW || GetModelIndex()==MODELID_HELI_LEVIATHN)
		&& nNoOfContactWheels==0 && GetDamageImpulseMagnitude() <= 0.0f)
		{
			bSkipPhysics = false;
			m_nNoOfStaticFrames = 0;
		}
		else if((GetModelIndex()==MODELID_CAR_VORTEX || CCheat::IsCheatActive(CCheat::BACKTOTHEFUTURE_CHEAT)
		&& ((m_aWheelRatios[0] < 1.0f && m_aWheelColPoints[0].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW)
		|| (m_aWheelRatios[1] < 1.0f && m_aWheelColPoints[1].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW)
		|| (m_aWheelRatios[2] < 1.0f && m_aWheelColPoints[2].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW)
		|| (m_aWheelRatios[3] < 1.0f && m_aWheelColPoints[3].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW))))
		{
			bSkipPhysics = false;
			m_nNoOfStaticFrames = 0;
		}
	}
	
	for(int32 i=0; i<4; i++)
	{
		// ground physical hasn't been processed yet so postpone (unless final loop)
		if (m_aGroundPhysicalPtrs[i] != NULL)
		{
			m_nVehicleFlags.bRestingOnPhysical = true;
			if(!CWorld::bForceProcessControl)
			{
				AssertEntityPointerValid(m_aGroundPhysicalPtrs[i]);
				if (m_aGroundPhysicalPtrs[i]->GetIsInSafePosition())
				{
					SetWasPostponed(true);
					return;
				}
			}
		}
	}
	// don't want to skip physical if we're sitting on something that might move
	if(m_nVehicleFlags.bRestingOnPhysical)
	{
		bSkipPhysics = false;
		m_nNoOfStaticFrames = 0;
	}

#ifndef FINAL
	// Bills DEBUG CODE
	if(CVehicle::m_bDisplayHandlingInfo)
		DebugCode();
#endif

	// check for an apply vehicle damage
	VehicleDamage();

	// Control the watercannon for player or AI
	if (GetStatus()==STATUS_PLAYER && (GetModelIndex()==MODELID_CAR_FIRETRUCK || GetModelIndex()==MODELID_CAR_SWATVAN))
	{
		FireTruckControl();	
	}
	// Control for Tank weapons and smashing through cars
	else if(GetModelIndex() == MODELID_CAR_RHINO || GetModelIndex() == MODELID_CAR_RCTIGER)
	{
		TankControl();		// Control the cannon for tank
//		if(GetModelIndex() == MODELID_CAR_RHINO)
//			BlowUpCarsInPath();
	}
	else if(GetModelIndex()==MODELID_CAR_PACKER || GetModelIndex()==MODELID_CAR_DOZER
	|| GetModelIndex()==MODELID_CAR_DUMPER || GetModelIndex()==MODELID_CAR_CEMENT
	|| GetModelIndex()==MODELID_PLANE_ANDROM || GetModelIndex()==MODELID_CAR_FORKLIFT)
	{
		if(UpdateMovingCollision())
		{
			bSkipPhysics = false;
		}
	}
	else if(GetModelIndex()==MODELID_CAR_TOWTRUCK || GetModelIndex()==MODELID_CAR_TRACTOR)
	{
		TowTruckControl();
	}
	// Control for cars with Hydraulic suspension
	else if(hFlagsLocal &HF_HYDRAULICS_INSTALLED)
	//GetModelIndex() == MODELID_CAR_HAITIAN || GetModelIndex() == MODELID_CAR_SAVANNA)
	{
		HydraulicControl();
	}
	// Control to do boost jump when cheat or bonus is activated
	else if(( CCheat::IsCheatActive(CCheat::STRONGGRIP_CHEAT) || (nCarFlags &PROCCONT_FLAG_TAXI_WITHNITRO))&& GetStatus()==STATUS_PLAYER && m_vecMoveSpeed.MagnitudeSqr() > 0.2f*0.2f)
	{
		BoostJumpControl();
	}
	
	if(hFlagsLocal &HF_NOS_INSTALLED)
	{
		NitrousControl(0);
	}
	else
	{
		StopNitroEffect();
	}
	
	// Control for player arming car bombs
	if(FindPlayerVehicle()==this && CPad::GetPad(0)->CarGunJustDown(true))
	{
		if(BombOnBoard==1)	// Bomb activated. It's gonna blow.
		{		
			BombOnBoard = 4;
			DelayedExplosion = 7000;	// 7 secs
			pDelayedExplosionInflictor = FindPlayerPed();	
			CGarages::TriggerMessage("GA_12", -1, 3000);
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_ARM_TIMED_CAR_BOMB, 1.0f);
#endif //USE_AUDIOENGINE
		}
		else if(BombOnBoard==2)	// Bomb activated. It's gonna blow next time somebody enters
		{
			BombOnBoard = 5;
			CGarages::TriggerMessage("GA_12", -1, 3000);
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_ARM_IGNITION_ACTIVATED_CAR_BOMB, 1.0f);
#endif //USE_AUDIOENGINE
		}
	}

	// skip default physics update, gravity and air resistance applied here
	if (bSkipPhysics)
	{
		// Do some stuff that would otherwise be done in CPhysical::ProcessControl
		CPhysical::SkipPhysics();

		m_nVehicleFlags.bVehicleColProcessed = false;
		m_nVehicleFlags.bAudioChangingGear = false;
		m_fTyreTemp = 1.0f;
	}
	else
	{
		// if speed is Zero collision will not have been processed -> need to force if not skipping physics
		if(m_nVehicleFlags.bVehicleColProcessed==false)
		{
			ProcessControlCollisionCheck(true);
		}
					
		bool bPreviouslyInWater = m_nPhysicalFlags.bIsInWater;
		bool bHasHitWall = GetHasHitWall();
		bool bGrounded = false;
		if(GetDamageImpulseMagnitude()>0.0f && m_vecDamageNormal.z > 0.1f)
			bGrounded = true;
	
		CPhysical::ProcessControl();
		
		if(GetModelIndex()==MODELID_BOAT_SEAPLANE)
		{
			CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
			int32 nHandlingIndex = pModelInfo->GetHandlingId();
			ProcessBoatControl(mod_HandlingManager.GetBoatPointer(nHandlingIndex), GunOrientation, bHasHitWall, bGrounded);
			if(m_nPhysicalFlags.bIsInWater)
				m_nPhysicalFlags.bForceFullWaterCheck = true;
			else
				m_nPhysicalFlags.bForceFullWaterCheck = false;
		}
		else
		{
			ProcessBuoyancy();
		}
	
		if (!bPreviouslyInWater && m_nPhysicalFlags.bIsInWater)
		{
			// reset player breath if the vehicle just entered water
			if (pDriver && pDriver->IsPlayer())
				((CPlayerPed*)pDriver)->ResetPlayerBreath();
			else for(int32 i = 0; i < m_nMaxPassengers; i++)
			{
				if(pPassengers[i] && pPassengers[i]->IsPlayer())
					((CPlayerPed*)pPassengers[i])->ResetPlayerBreath();
			}
		}
		
		DEBUGPROFILE(CProfile::ResumeProfile(PROFILE_PHYSICS_TIME);)

		if(GetModelIndex()==MODELID_CAR_RCBARON)
			ProcessFlyingCarStuff();

		// scale wheel ratio values
		// assume suspension lines in collision model are all the same length
//		float lineLength = (pColModel->GetLine(0).m_vecStart - pColModel->GetLine(0).m_vecEnd).z;
//		float suspensionLength = pHandling->fSuspensionUpperLimit - pHandling->fSuspensionLowerLimit;
		float ratio;// = 1.0f - suspensionLength / lineLength;
		if(!m_nAutomobileFlags.bIsMonsterTruck)
		for(int32 i=0; i<4; i++)
		{
			ratio = 1.0f - m_fSuspensionLength[i] / m_fLineLength[i];
			//if(m_aWheelRatios[i] < ratio)
			//	m_aWheelRatios[i] = 0.0f;
			//else
			m_aWheelRatios[i] = (m_aWheelRatios[i] - ratio) / (1.0f - ratio);
		}

		// process suspension
		CVector aWheelSpeeds[4];
		CVector aWheelOffsets[4];
		
		// apply suspension spring/damper forces
		ProcessSuspension();
		
		for(int32 i=0; i<4; i++)
		{
			if (m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
			{
				bMoreSkidMarks[i] = FALSE;

/*				// Also make wheel muddy if it has to be
				switch(m_aWheelColPoints[i].GetSurfaceTypeB())
				{
					case(COLPOINT_SURFACETYPE_SAND_DEEP):
					case(COLPOINT_SURFACETYPE_SAND_MEDIUM):
					case(COLPOINT_SURFACETYPE_SAND_COMPACT):
					case(COLPOINT_SURFACETYPE_SAND_ARID):
					case(COLPOINT_SURFACETYPE_SAND_MORE):
					case(COLPOINT_SURFACETYPE_SAND_BEACH):
						aWheelSkidmarkType[i]	= SKIDMARKTYPE_SANDY;
						bMoreSkidMarks[i]		= TRUE;
						break;
				
					case(COLPOINT_SURFACETYPE_GRASS_SHORT_LUSH):
					case(COLPOINT_SURFACETYPE_GRASS_MEDIUM_LUSH):
					case(COLPOINT_SURFACETYPE_GRASS_LONG_LUSH):
					case(COLPOINT_SURFACETYPE_GRASS_SHORT_DRY):
					case(COLPOINT_SURFACETYPE_GRASS_MEDIUM_DRY):
					case(COLPOINT_SURFACETYPE_GRASS_LONG_DRY):
					case(COLPOINT_SURFACETYPE_GOLFGRASS_ROUGH):
					case(COLPOINT_SURFACETYPE_GOLFGRASS_SMOOTH):
					case(COLPOINT_SURFACETYPE_MUD_WET):
					case(COLPOINT_SURFACETYPE_MUD_DRY):
						aWheelSkidmarkType[i]	= SKIDMARKTYPE_MUDDY;
						break;

					default:
						aWheelSkidmarkType[i]	= SKIDMARKTYPE_DEFAULT;
						break;					
				}
*/
			
				aWheelSkidmarkType[i] = g_surfaceInfos.GetSkidmarkType(m_aWheelColPoints[i].GetSurfaceTypeB());
				if (aWheelSkidmarkType[i] == SKIDMARKTYPE_SANDY)
				{
					bMoreSkidMarks[i] = true;
				}
			
				aWheelOffsets[i] = m_aWheelColPoints[i].GetPosition() - GetPosition();
			}
			else if(pColData->m_useDisksNotLines)
			{
				aWheelOffsets[i] = pColData->GetDisk(i).m_vecCentre;
				aWheelOffsets[i].z -= pColData->GetDisk(i).m_fRadius;
				aWheelOffsets[i] = Multiply3x3(GetMatrix(), aWheelOffsets[i]);			
			}
			else
			{
				aWheelOffsets[i] = Multiply3x3(GetMatrix(), pColData->GetLine(i).m_vecEnd);			
			}
		}

		// We have to recalculate the velocities. Otherwise there will be silly stuff going on when
		// the car is not moving.
		for(int32 i=0; i<4; i++)
		{
			aWheelSpeeds[i] = GetSpeed(aWheelOffsets[i]);

			if (m_aGroundPhysicalPtrs[i] != NULL)
			{
				AssertEntityPointerValid(m_aGroundPhysicalPtrs[i]);
				aWheelSpeeds[i] -= m_aGroundPhysicalPtrs[i]->GetSpeed(m_aGroundOffsets[i]);
				m_aGroundPhysicalPtrs[i] = NULL;
			}
		}

		// compute current forwards speed
		float fSpeed = DotProduct(m_vecMoveSpeed, GetMatrix().GetForward());

		if( m_nVehicleFlags.bAudioChangingGear && GetGasPedal() > 0.4f && GetBrakePedal() < 0.1f && fSpeed > 0.15f && this == FindPlayerVehicle() && TheCamera.Cams[TheCamera.ActiveCam].Mode!=CCam::MODE_1STPERSON)
		{
//			if(GetStatus()==STATUS_PLAYER){
//				sprintf(gString, "Changing Gear");
//				CDebug::PrintToScreenCoors(gString, 20,20);
//			}
			bool bOkToChange = true;
			if(GetStatus()==STATUS_PLAYER && !(pHandling->mFlags &MF_IS_BUS))
			{
				if(m_nBusDoorTimer > 0){
					bOkToChange = false;
					if(m_nBusDoorTimer > CTimer::GetTimeElapsedInMilliseconds())
						m_nBusDoorTimer -= CTimer::GetTimeElapsedInMilliseconds();
					else
						m_nBusDoorTimer = 0;
				}
				else
				{
					bOkToChange = true;
					m_nBusDoorTimer = GEARCHANGE_INTERVAL;
				}
			}
			
			if( (m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT || m_aWheelRatios[2]<BILLS_EXTENSION_LIMIT)
			&&  (m_aWheelRatios[1]<BILLS_EXTENSION_LIMIT || m_aWheelRatios[3]<BILLS_EXTENSION_LIMIT) )
				ApplyTurnForce( GEARCHANGE_MULT*CMaths::Min(m_fTurnMass, 2500.0f)*GetMatrix().GetUp(), -1.0f*GetMatrix().GetForward());
		}

		// brake force - taking into account time step
		fFriction = m_fBrakePedal * pHandling->fBrakeDeceleration  * CTimer::GetTimeStep();

//		const bool  bNeutralHandling = (GetStatus()!=STATUS_PLAYER && GetStatus()!=STATUS_PLAYER_REMOTE && (pHandling->hFlags &HF_NPC_NEUTRAL_HANDLING));
		// if NeutralHandling flag is set - grip and braking to be equal front and back
//		const float FRONT_BIAS = (bNeutralHandling ? 1.0f : 2.0f * pHandling->fBrakeBias);
//		const float REAR_BIAS = (bNeutralHandling ? 1.0f : 2.0f - pHandling->fBrakeBias);
//		const float FRONT_TRAC_BIAS = (bNeutralHandling ? 1.0f : 2.0f * pHandling->fTractionBias);
//		const float REAR_TRAC_BIAS = (bNeutralHandling ? 1.0f : 2.0f - FRONT_TRAC_BIAS);

		nNoOfContactWheels = 0;			// Put declaration into automobile class as audio uses it
		m_nDriveWheelsOnGroundLastFrame = m_nDriveWheelsOnGround;	// Record last frame's value
		m_nDriveWheelsOnGround = 0;		// Reset

		for(int32 i=0; i<4; i++)
		{
			if (m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
			{
				m_aWheelCounts[i] = AUTOMOBILE_NOOFFRAMESFORCONTACTTEST;
			}
			else
			{
				m_aWheelCounts[i] = MAX(m_aWheelCounts[i] - CTimer::GetTimeStep(), 0.0f);
			}

			if (m_aWheelCounts[i] > 0)
			{
				nNoOfContactWheels++;
	
				// Audio code - for revving when drive wheels are off the ground
				switch(pHandling->Transmission.m_nDriveType)
				{
					case '4':
						m_nDriveWheelsOnGround++;
						break;
						
					case 'R':
						if((i == REAR_LEFT_WHEEL) || (i == REAR_RIGHT_WHEEL))
						{
							m_nDriveWheelsOnGround++;
						}
						break;
						
					case 'F':
						if((i == FRONT_LEFT_WHEEL) || (i == FRONT_RIGHT_WHEEL))
						{
							m_nDriveWheelsOnGround++;
						}
						break;
				}
			}
		}
		
/*		// JIMMY BOND STUFF
		if( GetModelIndex()==MODELID_CAR_SENTINEL && m_fTransformPosition > 0.0f)
		{
			fDriveForce = 0.0f;
			fFriction = 1.0f * pHandling->fBrakeDeceleration  * CTimer::GetTimeStep();
			m_nDriveWheelsOnGround = 4;
			bIsBoggedDownInSand = true;
		}
*/

		// calculate drive force available to drive wheels
		uint8 nDriveForceCheat = 0;
		if(hFlagsLocal &HF_NOS_INSTALLED && m_fTyreTemp < 0.0f)
			nDriveForceCheat = 2;
		else if((nCarFlags &PROCCONT_FLAG_CAR_CHEATS) || CCheat::IsCheatActive(CCheat::STRONGGRIP_CHEAT))
			nDriveForceCheat = 1;
		
		fDriveForce = 0.0f;
		if(m_nVehicleFlags.bEngineOn && !(pHandling->mFlags &MF_IS_HELI) && !(pHandling->mFlags &MF_IS_PLANE))
		{
			fDriveForce = pHandling->Transmission.CalculateDriveAcceleration(m_fGasPedal, m_nCurrentGear, m_fGearChangeCount, fSpeed, &m_fEngineRevs, &m_fEngineForce, m_nDriveWheelsOnGround, nDriveForceCheat);
			fDriveForce /= m_fMassMultiplier;
		}

		// should be gravity vector.terrain_normal times friction
		float fAdhesiveScalar;
		if(GetStatus()==STATUS_PHYSICS)	// give a bit extra grip for stable handling
			fAdhesiveScalar = m_fExtraTractionMult * G_fGravity * pHandling->fTractionMultiplier / (float)nNumWheels;
		else
			fAdhesiveScalar = G_fGravity * pHandling->fTractionMultiplier / (float)nNumWheels;
		fAdhesiveScalar /= m_fMassMultiplier;	// If the car is carrying stuff the adhesive limit gets decreased. (It is a max speedchange not a max force)
		
		if( CCheat::IsCheatActive(CCheat::STRONGGRIP_CHEAT))
			fAdhesiveScalar *= 4.0f;
//		static float CAR_NOS_EXTRA_TRACTION_LOSS = 0.85f;
//		if(hFlagsLocal &HF_NOS_INSTALLED && m_fTyreTemp < 0.0f)
//			fAdhesiveScalar *= CAR_NOS_EXTRA_TRACTION_LOSS;
		else if(GetModelIndex()==MODELID_CAR_VORTEX && ((CPlane *)this)->m_fThrottleControl==0.0)
			fAdhesiveScalar *= 4.0f;
	
		if(this != FindPlayerVehicle())
		{
			if ((nCarFlags &PROCCONT_FLAG_CAR_CHEATS) ||  CCheat::IsCheatActive(CCheat::STRONGGRIP_CHEAT))
			{
				fAdhesiveScalar *= 1.2f;	// Cheat a little bit to make chasing easier
				fDriveForce *= 1.4f;		// obr
				if((nCarFlags &PROCCONT_FLAG_EXTRA_CARCHEATS) ||  CCheat::IsCheatActive(CCheat::STRONGGRIP_CHEAT))
				{
					fAdhesiveScalar *= 1.3f;	// Cheat a little bit to make chasing easier
					fDriveForce *= 1.4f;		// obr
				}
			}
		}
		
		static float fDoSpeepSteerFrac = 4.0f;
		if(fDoSpeepSteerFrac > 0.0f)
		{
			float fSteerLimitNotSkid;
			// work out the proper adhesive limit
			if(fSpeed > 0.01f && (m_aWheelCounts[0] > 0.0f || m_aWheelCounts[1] > 0.0f) && GetStatus()==STATUS_PLAYER)
			{
				float fRightDotProd = DotProduct(m_vecMoveSpeed, GetMatrix().GetRight());
				// base speedsteer on tarmac traction values only
				CColPoint colTempPoint;
				colTempPoint.SetSurfaceTypeA(SURFACE_TYPE_WHEELBASE);
				colTempPoint.SetSurfaceTypeB(SURFACE_TYPE_TARMAC);
			 	float fAdhesiveLimit = fDoSpeepSteerFrac * nNumWheels * fAdhesiveScalar * g_surfaceInfos.GetAdhesiveLimit(colTempPoint);

				fSteerLimitNotSkid = CMaths::Min(1.0f, fAdhesiveLimit/(fSpeed*fSpeed));
				fSteerLimitNotSkid = CMaths::ASin(fSteerLimitNotSkid)/DEGTORAD(pHandling->fSteeringLock);
				
				if((m_fSteerAngle < 0.0 && fRightDotProd > 0.05f) || (m_fSteerAngle > 0.0f && fRightDotProd < -0.05f)
				|| GetIsHandbrakeOn())
					fSteerLimitNotSkid = 1.0f;

				fSteerLimitNotSkid = CMaths::Min(1.0f, fSteerLimitNotSkid);
			}
			else
			{
				fSteerLimitNotSkid = 1.0f;
			}
			
			m_fSteerAngle *= fSteerLimitNotSkid;
		}

//////////////////////////////////
// FRONT WHEELS (1st chance to process them!)
//////////////////////////////////
#ifdef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION
		float fAveRatioL = 0.5f*(m_aWheelRatios[0] + m_aWheelRatios[1]);
		float fAveRatioR = 0.5f*(m_aWheelRatios[2] + m_aWheelRatios[3]);
#endif
		bool bDoRearWheelsFirst = hFlagsLocal &HF_PROCESS_REARWHEEL_1ST;
		float fUseSteerAngle = m_fSteerAngle;
		if(!bDoRearWheelsFirst)// && GetStatus()!=STATUS_TRAILER)
		{
			if(hFlagsLocal &HF_STEER_REARWHEELS)
				fUseSteerAngle = -999.0f;
			else
				fUseSteerAngle = m_fSteerAngle;
				
#ifdef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION
			ProcessCarWheelPair(0, 2, fUseSteerAngle, aWheelSpeeds, aWheelOffsets, fAveRatioL, fAveRatioR, fAdhesiveScalar, fDriveForce, fFriction, true);
#else
			ProcessCarWheelPair(0, 2, fUseSteerAngle, aWheelSpeeds, aWheelOffsets, fAdhesiveScalar, fDriveForce, fFriction, true);
#endif
		}
////////////////////////////////
// REAR WHEELS
////////////////////////////////
		if(hFlagsLocal &HF_STEER_REARWHEELS)
			fUseSteerAngle = -m_fSteerAngle;
		else if(hFlagsLocal &HF_HANDBRAKE_REARWHEELSTEER)
			fUseSteerAngle = m_f2ndSteerAngle;
		else
			fUseSteerAngle = -999.0f;

#ifdef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION
		ProcessCarWheelPair(1, 3, fUseSteerAngle, aWheelSpeeds, aWheelOffsets, fAveRatioL, fAveRatioR, fAdhesiveScalar, fDriveForce, fFriction, false);
#else
		ProcessCarWheelPair(1, 3, fUseSteerAngle, aWheelSpeeds, aWheelOffsets, fAdhesiveScalar, fDriveForce, fFriction, false);
#endif
//////////////////////////////////
// FRONT WHEELS (2nd chance to process them!)
//////////////////////////////////
		if(bDoRearWheelsFirst)// && GetStatus()!=STATUS_TRAILER)
		{
			if(hFlagsLocal &HF_STEER_REARWHEELS)
				fUseSteerAngle = -999.0f;
			else
				fUseSteerAngle = m_fSteerAngle;

#ifdef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION
			ProcessCarWheelPair(0, 2, fUseSteerAngle, aWheelSpeeds, aWheelOffsets, fAveRatioL, fAveRatioR, fAdhesiveScalar, fDriveForce, fFriction, true);
#else
			ProcessCarWheelPair(0, 2, fUseSteerAngle, aWheelSpeeds, aWheelOffsets, fAdhesiveScalar, fDriveForce, fFriction, true);
#endif
		}
/*
		// process wheel distance from car body
		CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
		if(!bIsMonsterTruck)
		for(int32 i=0; i<4; i++)
		{
			float fHeight = pColModel->m_pLineArray[i].m_vecStart.z;

			if(m_aWheelRatios[i] > 0.0f)
				fHeight -= m_aWheelRatios[i] * m_fSuspensionLength[i];

			// don't dampen movement of wheels for hydraulic suspension car!
			if( fHeight > m_aWheelSuspensionHeights[i]
			|| (m_nFlags.bUsingSpecialColModel && hFlagsLocal &HF_HYDRAULICS_INSTALLED))
			{
				m_aWheelSuspensionHeights[i] = fHeight;
			}
			// dampens the speed of the wheel moving down
			else
			{
				m_aWheelSuspensionHeights[i] += (fHeight - m_aWheelSuspensionHeights[i]) * AUTOMOBILE_WHEELSUSPENSIONINERTIA;
			}
		}
*/		
		if(GetStatus() == STATUS_PLAYER)
		{
	//		tVehicleType nModel;
	//		nModel = ((CVehicle*)(this))->pHandling->nVehicleID;
	//		if((nModel == VT_POLICE) || (nModel == VT_SWAT) || (nModel == VT_BOAT_POLICE1) || (nModel == VT_FIRETRUCK) || (nModel == VT_AMBULAN) || (nModel == VT_ICECREAM))
//			if((UsesSiren(GetModelIndex())) || IsLawEnforcer)


/*			
			// COMMENTED OUT FOR ICE CREAM VAN - ENABLE LATER WHEN NEW MODEL INDEX IS KNOWN
			if (GetModelIndex() == MODELID_CAR_MRWHOOPEE)
			{
				// Since MrWhoopee doesn't use the horn we will just make it so that
				// pressing the horn toggles it. Never mind the history.
				if (Pads[0].bHornHistory[Pads[0].iCurrHornHistory] &&
					!Pads[0].bHornHistory[(Pads[0].iCurrHornHistory + MAX_HORN_HISTORY - 1) % MAX_HORN_HISTORY])
				{
					m_bSirenOrAlarm = !m_bSirenOrAlarm;
					
					printf("m_bSirenOrAlarm toggled to %d\n", m_bSirenOrAlarm);
				}

			}
			else */
			
			ProcessSirenAndHorn(!(hFlagsLocal &HF_HYDRAULICS_INSTALLED) && ! CCheat::IsCheatActive(CCheat::STRONGGRIP_CHEAT) && !(nCarFlags &PROCCONT_FLAG_TAXI_WITHNITRO));
			
		}
		else
		{
			if(!IsAlarmActivated())
			{
				ReduceHornCounter();
			}
		}
		
		if(GetModelIndex()!=MODELID_CAR_RCBARON)
			ProcessFlyingCarStuff();
	}	

	ProcessCarOnFireAndExplode(forceExplode);

	// If this vehicle has its siren switched on other cars should get out of the way
	if (m_nVehicleFlags.bSirenOrAlarm && ((CTimer::m_FrameCounter & 7) == 5) && UsesSiren() && GetModelIndex() != MODELID_CAR_MRWHOOPEE && FindPlayerVehicle() == this)
	{
		CCarAI::MakeWayForCarWithSiren(this);
	}

	// finally copy wheel ratio history across, for audio use
	float fBumpShakePad = 0.0;
	float fSurfaceShakePad = 0.0f;
	float fVelSqr = m_vecMoveSpeed.MagnitudeSqr();
	float fCountWheelSpin = 0.0f;
	for(int32 i=0; i<4; i++)
	{
		// Find out if wheel bumps to be played
		float fWheelRatioDiff;
		fWheelRatioDiff = m_aRatioHistory[i] - m_aWheelRatios[i];
		
		if(fWheelRatioDiff > 0.3f && !(nCarFlags &PROCCONT_FLAG_DRIVING_ONSAND) && fVelSqr>0.2f*0.2f)
		{	
			if(Damage.GetWheelStatus(i) == DT_WHEEL_BURST)
			{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_CAR_WHEEL_BURST_BUMP, fWheelRatioDiff);
#endif //USE_AUDIOENGINE
			}
			else
			{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_CAR_WHEEL_BUMP, fWheelRatioDiff);
#endif //USE_AUDIOENGINE
			}

			// save this wheel hit for shaking the pad
			if(fWheelRatioDiff > fBumpShakePad)
				fBumpShakePad = fWheelRatioDiff;
		}
		
		if(m_aWheelRatios[i] < 1.0f && GetStatus()==STATUS_PLAYER)
		{
			float fShake = 0.1f*g_surfaceInfos.GetRoughness(m_aWheelColPoints[i].GetSurfaceTypeB());
			fSurfaceShakePad = CMaths::Max(fSurfaceShakePad, fShake);
/*
			// also check for driving on very rough surfaces	
			if (g_surfaceInfos.GetRoughness(m_aWheelColPoints[i].GetSurfaceTypeB()) == ROUGHNESS_VERY)
			{
				if(0.2f > fSurfaceShakePad)
					fSurfaceShakePad = 0.3f;
			}
			// or bit rough surfaces
			else if (g_surfaceInfos.GetRoughness(m_aWheelColPoints[i].GetSurfaceTypeB()) == ROUGHNESS_NORM)
			{
				if(0.1f > fSurfaceShakePad)
					fSurfaceShakePad = 0.2f;
			}
			// or grass
			else if (g_surfaceInfos.GetRoughness(m_aWheelColPoints[i].GetSurfaceTypeB()) == ROUGHNESS_QUITE)
			{
				if(0.05f > fSurfaceShakePad)
					fSurfaceShakePad = 0.1f;
			}
*/
		}
				
		// count wheels spinning or in air under power
		if(m_aWheelCounts[i] <= 0.0f && (GetGasPedal() > 0.5f || GetGasPedal() < -0.5f)
		&& (((i==0 || i==2) && mod_HandlingManager.IsFrontWheelDrive(pHandling->nVehicleID))
		||  ((i==1 || i==3) && mod_HandlingManager.IsRearWheelDrive(pHandling->nVehicleID))))
			fCountWheelSpin += WHEELSPIN_INAIR_TARGET_RATE;
		else if(m_aWheelState[i]==WS_SPINNING)
			fCountWheelSpin += WHEELSPIN_TARGET_RATE;
		
		m_aRatioHistory[i] = m_aWheelRatios[i];
		m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;
	}
	
	// want to smoothly count wheels doing powered wheelspins or spinning in air under acceleration
	if(pHandling->Transmission.m_nDriveType == '4')
		fCountWheelSpin /= 4.0f;
	else
		fCountWheelSpin /= 2.0f;
	
	float fDampRate = 0.0f;
	if(fCountWheelSpin < m_fWheelSpinForAudio)
		fDampRate = CMaths::Pow(WHEELSPIN_FALL_RATE, CTimer::GetTimeStep());
	else
		fDampRate = CMaths::Pow(WHEELSPIN_RISE_RATE, CTimer::GetTimeStep());
		
	m_fWheelSpinForAudio = fDampRate*m_fWheelSpinForAudio + (1.0f - fDampRate)*fCountWheelSpin;
	

	// Set the surface shake pad to 0 most of the time to avoind trc issue with prolonged shaking.
	// Set the surface shake pad to 0 most of the time to avoind trc issue with prolonged shaking.
	if ((CTimer::GetTimeInMilliseconds() & 2047) > 800)
	{
		// when on very bumpy surfaces surface shake will be overriding bumpshake anyway
		// and in the case of railtracks bumpshake will be happening all the time from the surface too!
		if(fSurfaceShakePad >= 0.29f)
			fBumpShakePad = 0.0f;
	
		fSurfaceShakePad = 0.0f;
	}
	
	float fTempMoveSpeed = 0.0f;
	if(!(nCarFlags &PROCCONT_FLAG_DRIVING_ONSAND) && (fBumpShakePad>0.0f || fSurfaceShakePad>0.0f) && GetStatus()==STATUS_PLAYER && (fTempMoveSpeed = m_vecMoveSpeed.MagnitudeSqr()) > 0.1f*0.1f)
	{
		// Add little jolt.
		fTempMoveSpeed = CMaths::Sqrt(fTempMoveSpeed);
		if(fBumpShakePad > 0.0f){
			uint8 nFreq = (uint8) MIN(250.0f, 100 + 200.0f*fBumpShakePad*fTempMoveSpeed*2000.0f/m_fMass);
			uint16 nLength = CTimer::GetTimeStep()*20000/nFreq;
			CPad::GetPad(0)->StartShake(nLength, nFreq);

//			printf("BumpShake: %d,%d\n", nLength, nFreq);
		}
		else{
			uint8 nFreq = (uint8) MIN(150.0f, 40 + 200.0f*fSurfaceShakePad*fTempMoveSpeed*2000.0f/m_fMass);
			uint16 nLength = CTimer::GetTimeStep()*5000/nFreq;
			CPad::GetPad(0)->StartShake(nLength, nFreq);

//			sprintf(gString, "SurfaceShake: %d", nFreq);
//			CDebug::PrintToScreenCoors(gString, 25,30);
		}
	}
	
	// wheel ratios have been reset, so ProcessEntityColision need to be 
	// called again before wheel ratios can been used
	m_nVehicleFlags.bVehicleColProcessed = false;
	// also clear changing gear flag, to be updated by audio process if required
	m_nVehicleFlags.bAudioChangingGear = false;


#ifndef FINAL
	// draw a speedo for player car
	if(CVehicle::m_bDisplayPerformanceInfo && (GetStatus() == STATUS_PLAYER || GetStatus() == STATUS_PLAYER_REMOTE))
	{
		float fSpeed = DotProduct(m_vecMoveSpeed, GetMatrix().GetForward())/GAME_VELOCITY_CONST;
		float fMaxSpeed = pHandling->Transmission.m_fMaxVelocity/GAME_VELOCITY_CONST;
		int nGear = m_nCurrentGear;
		sprintf(gString, "Spd: %f (%f), Gear: %d (%f) Ws: %f", fSpeed, fMaxSpeed, nGear, 100.0f*fDriveForce, m_fWheelSpinForAudio);
		CDebug::PrintToScreenCoors(gString, 18,27);
		
		float fLowestSphereZ = 0.0f;
		for(int i=0; i<pColData->m_nNoOfSpheres; i++)
		{
			if(pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius < fLowestSphereZ)
				fLowestSphereZ = pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius;
		}
		
		// CALCULATE GROUND CLEARANCE
		float fModelGroundClearance = (pHandling->fSuspensionLowerLimit - pColData->m_pLineArray[2].m_vecEnd.z) + fLowestSphereZ;
		float fSetupGroundClearance = GetHeightAboveRoad() + fLowestSphereZ;
		sprintf(gString, "Ground Clearance: Model(%1.4f) With Suspension(%1.4f)", fModelGroundClearance, fSetupGroundClearance);
		CDebug::PrintToScreenCoors(gString, 18,28);
		// CALCULATE WEIGHT DISTRIBUTION
		float fBaseWeightDistribution, fHandlingWeightDistribution;
		if(GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK)
		{
			fBaseWeightDistribution = -100.0f*pColData->m_pDiskArray[1].m_vecCentre.y / (pColData->m_pDiskArray[0].m_vecCentre.y - pColData->m_pDiskArray[1].m_vecCentre.y);
			fHandlingWeightDistribution = -100.0f*(pColData->m_pDiskArray[1].m_vecCentre.y - pHandling->CentreOfMass.y) / (pColData->m_pDiskArray[0].m_vecCentre.y - pColData->m_pDiskArray[1].m_vecCentre.y);
		}
		else
		{
			fBaseWeightDistribution = -100.0f*pColData->m_pLineArray[1].m_vecStart.y / (pColData->m_pLineArray[0].m_vecStart.y - pColData->m_pLineArray[1].m_vecStart.y);
			fHandlingWeightDistribution = -100.0f*(pColData->m_pLineArray[1].m_vecStart.y - pHandling->CentreOfMass.y) / (pColData->m_pLineArray[0].m_vecStart.y - pColData->m_pLineArray[1].m_vecStart.y);
		}
		sprintf(gString, "Weight Balance %2.1f:%2.1f With COG Moved(%2.1f:%2.1f)", fBaseWeightDistribution, (100.0f - fBaseWeightDistribution), fHandlingWeightDistribution, (100.0f - fHandlingWeightDistribution));
		CDebug::PrintToScreenCoors(gString, 15,29);
		// CALCULATE STABILITY FACTOR (SFF)
		float fModelSFF, fBaseSFF, fHandlingSFF;
		if(GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK)
		{
			float fWheelRadius = 0.5f*((CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->GetWheelScale(true);
			fModelSFF = 0.5f*(pColData->m_pDiskArray[2].m_vecCentre.x + pColData->m_pDiskArray[3].m_vecCentre.x) / (fWheelRadius - pColData->m_pDiskArray[2].m_vecCentre.z);
			fBaseSFF = 0.5f*(pColData->m_pDiskArray[2].m_vecCentre.x + pColData->m_pDiskArray[3].m_vecCentre.x) / GetHeightAboveRoad();
			fHandlingSFF = 0.5f*(pColData->m_pDiskArray[2].m_vecCentre.x + pColData->m_pDiskArray[3].m_vecCentre.x) / (GetHeightAboveRoad() + pHandling->CentreOfMass.z);
		}
		else
		{
			fModelSFF = 0.5f*(pColData->m_pLineArray[2].m_vecStart.x + pColData->m_pLineArray[3].m_vecStart.x) / (pHandling->fSuspensionLowerLimit - pColData->m_pLineArray[2].m_vecEnd.z);
			fBaseSFF = 0.5f*(pColData->m_pLineArray[2].m_vecStart.x + pColData->m_pLineArray[3].m_vecStart.x) / GetHeightAboveRoad();
			fHandlingSFF = 0.5f*(pColData->m_pLineArray[2].m_vecStart.x + pColData->m_pLineArray[3].m_vecStart.x) / (GetHeightAboveRoad() + pHandling->CentreOfMass.z);
		}
		sprintf(gString, "SFF: Model(%1.3f) With Suspension(%1.3f) With COG Moved(%1.3f)", fModelSFF, fBaseSFF, fHandlingSFF);
		CDebug::PrintToScreenCoors(gString, 10,30);

		if(CCullZones::DoExtraAirResistanceForPlayer())
		{
			sprintf(gString, "SLOOOOOOOW");
			CDebug::PrintToScreenCoors(gString, 30,25);
		}
		
		//sprintf(gString, "Wetness: %f, Surface: %d", CWeather::WetRoads, m_aWheelColPoints[1].GetSurfaceTypeB());
		//sprintf(gString, "Health: %f, VisDamage: %s", m_nHealth, (bIsDamaged ? "true":"false"));
//		sprintf(gString, "m_vecMoveSpeed: %f,%f,%f", m_vecMoveSpeed.x, m_vecMoveSpeed.y, m_vecMoveSpeed.z);
//		CDebug::PrintToScreenCoors(gString, 25,30);
	}
#endif


	//Now done as ped scanning for vehicle threat events.

	// if peds haven't been warned of this cars approach by now - do it!
	// JB : changed this so peds will avoid planes & helis when on the ground too
	if((!m_nVehicleFlags.bWarnedPeds))// && GetVehicleAppearance() != APR_HELI && GetVehicleAppearance() != APR_PLANE)
	{
		CCarCtrl::ScanForPedDanger(this);
	}


	DEBUGPROFILE(CProfile::SuspendProfile(PROFILE_PHYSICS_TIME);)
//	SetPedPositionInCar();

/* Allow stuff to leave the map	
	float fTempHeading = 0.0f;
	if(GetMatrix().tx > CAR_LIMIT_XMAX)
	{
		if(m_vecMoveSpeed.x > 0.0f)
			m_vecMoveSpeed.x *= -1.0f;
			
		if( (fTempHeading = GetHeading()) < 0.0f )
		{
			SetHeading( -fTempHeading );
		}
	}
	else if( GetMatrix().tx < CAR_LIMIT_XMIN)
	{
		if(m_vecMoveSpeed.x < 0.0f)
			m_vecMoveSpeed.x *= -1.0f;

		if( (fTempHeading = GetHeading()) > 0.0f )
		{
			SetHeading( -fTempHeading );
		}
	}

	if(GetMatrix().ty > CAR_LIMIT_YMAX)
	{
		if(m_vecMoveSpeed.y > 0.0f)
			m_vecMoveSpeed.y *= -1.0f;
	
		if( (fTempHeading = GetHeading()) < HALF_PI && fTempHeading > 0.0f)
		{
			SetHeading( PI - fTempHeading );
		}
		else if( fTempHeading < -HALF_PI && fTempHeading < 0.0f)
		{
			SetHeading( -PI - fTempHeading );
		}
	}
	else if( GetMatrix().ty < CAR_LIMIT_YMIN)
	{
		if(m_vecMoveSpeed.y < 0.0f)
			m_vecMoveSpeed.y *= -1.0f;
	
		if( (fTempHeading = GetHeading()) > HALF_PI)
		{
			SetHeading( PI - fTempHeading );
		}
		else if( fTempHeading < -HALF_PI )
		{
			SetHeading( -PI - fTempHeading );
		}
	}
*/

	// for cars with hydraulics we want to apply some additional damping around the y-axis
	if(hFlagsLocal &HF_HYDRAULICS_INSTALLED && m_vecMoveSpeed.Magnitude() < 0.2f)
	{
		bool bDoDamping = false;
		if(GetStatus()==STATUS_PHYSICS
		&& ((m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[0] > 0.5f && m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[1] > 0.5f)
		||  (m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[2] > 0.5f && m_aSpecialHydraulicData[m_nSpecialColModel].aScriptControl[3] > 0.5f)))
			bDoDamping = true;
		else if(GetStatus()==STATUS_PLAYER && pDriver && pDriver->IsPlayer())
		{
			CPad *pPad = ((CPlayerPed *)pDriver)->GetPadFromPlayer();
			if(CMaths::Abs(pPad->GetCarGunLeftRight()) > 50 && CMaths::Abs(pPad->GetCarGunUpDown()) < 50)
				bDoDamping = true;
		}
			
		if(bDoDamping)
		{
			static float HYDRAULIC_Y_ROTATION_DAMPING = 0.985f;
			static float HYDRAULIC_Y_ROTATION_SPEED_DAMPING = 5.0f;
			float fYRotationResistance = CMaths::Pow(HYDRAULIC_Y_ROTATION_DAMPING, CTimer::GetTimeStep());
			float fYRotationSpeed = DotProduct(m_vecTurnSpeed, GetMatrix().GetForward());
			float fYRotationDampForce = fYRotationResistance / (1 + HYDRAULIC_Y_ROTATION_SPEED_DAMPING*fYRotationSpeed*fYRotationSpeed);
			fYRotationDampForce = CMaths::Pow(fYRotationDampForce, CTimer::GetTimeStep());
			fYRotationDampForce = fYRotationSpeed * fYRotationDampForce - fYRotationSpeed;
			
			CVector vecTurnForce = -1.0f*GetMatrix().GetUp()*fYRotationDampForce*m_fTurnMass;
			CVector vecTurnOffset = GetMatrix().GetRight() + Multiply3x3(GetMatrix(), m_vecCOM);
			ApplyTurnForce(vecTurnForce, vecTurnOffset);
		}
	}

	if(m_pTowingVehicle)
	{
		if(GetStatus()==STATUS_TRAILER)
		{
			bool bApplyToTractor = false;
			// this applies velocity delta between towbar and towhitch
			if(m_pTowingVehicle->m_vecMoveSpeed.x!=0.0f
			|| m_pTowingVehicle->m_vecMoveSpeed.y!=0.0f
			|| m_pTowingVehicle->m_vecMoveSpeed.z!=0.0f
			|| this->m_vecMoveSpeed.MagnitudeSqr() > 0.1f*0.1f)
			{
				bApplyToTractor = true;
			}

			if(bApplyToTractor)
				m_pTowingVehicle->UpdateTractorLink();
			UpdateTrailerLink();
			
			// now apply velocity derived from positional difference between towbar and towhitch
			// (had to separate the two because velocity being applied to the trailer was incorporating
			//  the full delta pos velocity applied to the tractor, making the system unstable)
			if(m_pTowingVehicle && GetStatus()==STATUS_TRAILER)
			{
				if(bApplyToTractor)
					m_pTowingVehicle->UpdateTractorLink(false, true);
				UpdateTrailerLink(false, true);
			}
		}
		else
			BreakTowLink();
	}
	else if(m_pVehicleBeingTowed)
	{
		if(m_pVehicleBeingTowed->GetStatus()!=STATUS_TRAILER)
		{
			m_pVehicleBeingTowed->CleanUpOldReference((CEntity**)&m_pVehicleBeingTowed);
			m_pVehicleBeingTowed = NULL;
		}
		else if(m_pVehicleBeingTowed->m_pTowingVehicle==this)
		{
			// remove and add tractor to moving list to ensure it gets processed before the trailer!
			this->RemoveFromMovingList();
			this->AddToMovingList();
		}
	}
	
	if(m_nPhysicalFlags.bInfiniteMass && m_nPhysicalFlags.bInfiniteMassFixed){
		m_vecMoveSpeed = CVector(0.0f,0.0f,0.0f);
		m_vecTurnSpeed = CVector(0.0f,0.0f,0.0f);
		m_vecMoveFriction = CVector(0.0f,0.0f,0.0f);
		m_vecTurnFriction = CVector(0.0f,0.0f,0.0f);
	}
	else if(!bSkipPhysics && ((GetGasPedal()==0.0f/* && fFriction==0.0f*/) || GetStatus()==STATUS_WRECKED)
	&& CMaths::Abs(m_vecMoveSpeed.x) < 0.005f && CMaths::Abs(m_vecMoveSpeed.y) < 0.005f 
	&& CMaths::Abs(m_vecMoveSpeed.z) < 0.005f && !(GetDamageImpulseMagnitude()>0.0f && GetDamageEntity()==FindPlayerPed()))
	{
		if(GetVehicleType()==VEHICLE_TYPE_PLANE && ((CPlane *)this)->m_fThrottleControl!=0.0f)
			;// don't do it
		else if(GetVehicleType()==VEHICLE_TYPE_HELI && ((CHeli *)this)->m_fThrottleControl!=0.0f)
			;// don't do it
		else if(m_nSuspensionHydraulics!=0.0f && m_nSuspensionHydraulics!=m_nOldSuspensionHydraulics)
			;// don't do it
		else if(m_nPhysicalFlags.bIsInWater)
			;// don't do it
		else if(m_pTowingVehicle && (m_pTowingVehicle->m_vecMoveSpeed.x!=0.0f || m_pTowingVehicle->m_vecMoveSpeed.y!=0.0f || m_pTowingVehicle->m_vecMoveSpeed.z!=0.0f))
			;// don't do it
		// don't fix velocity if we're in mid-air (especially bad for helicopters!)
		else if(m_aRatioHistory[0] < BILLS_EXTENSION_LIMIT || m_aRatioHistory[1] < BILLS_EXTENSION_LIMIT
		|| m_aRatioHistory[2] < BILLS_EXTENSION_LIMIT || m_aRatioHistory[3] < BILLS_EXTENSION_LIMIT)
		{
			if((GetModelIndex()==MODELID_CAR_VORTEX || CCheat::IsCheatActive(CCheat::BACKTOTHEFUTURE_CHEAT)
			&& ((m_aRatioHistory[0] < 1.0f && m_aWheelColPoints[0].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW)
			|| (m_aRatioHistory[1] < 1.0f && m_aWheelColPoints[1].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW)
			|| (m_aRatioHistory[2] < 1.0f && m_aWheelColPoints[2].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW)
			|| (m_aRatioHistory[3] < 1.0f && m_aWheelColPoints[3].GetSurfaceTypeB()==SURFACE_TYPE_WATER_SHALLOW))))
			{
				bSkipPhysics = false;
				m_nNoOfStaticFrames = 0;
			}
			else
			{
				m_vecMoveSpeed = CVector(0.0f,0.0f,0.0f);
				m_vecTurnSpeed.z = 0.0f;	// let it bounce on suspension but not slide round
			}
		}
	}

		// Helis in the process of crashing are dealt with here
/*	if( (pHandling->mFlags &MF_IS_HELI) && ((CAutomobile *)this)->m_nAutomobileFlags.bHeliIsCrashing && !m_nPhysicalFlags.bRenderScorched)
	{
		this->ApplyMoveForce(0.0f, 0.0f, CTimer::GetTimeStep() * -4.0f);
		this->m_vecTurnSpeed.z += CTimer::GetTimeStep() * -0.004f;
		this->m_vecTurnSpeed.x += CTimer::GetTimeStep() * -0.0004f;

		// MN - FX SYSTEM -------------------------------
		// TODO: smoke from crashing helicopter
		// ----------------------------------------------


		if ( (CWorld::TestSphereAgainstWorld(GetPosition(), 8.0f, this, true, false, false, false, false, false) || GetPosition().z < WATERLEVEL_CLUDGEHEIGHT) &&
				!m_nPhysicalFlags.bRenderScorched)
		{
			CExplosion::AddExplosion(this, NULL, EXP_TYPE_CAR, GetPosition(), 0);
			m_nPhysicalFlags.bRenderScorched = true;
		}
	}
*/	// if Earthquake, the vehicle is a bit shaking by the earthquake
	if (CWeather::Earthquake > 0.0f)
	{
		CVector EPos = GetPosition();
		
		EPos.z += CGeneral::GetRandomNumberInRange(0.0f, CWeather::Earthquake * 0.5f);
		SetPosition(EPos);
	}	

	// If the car is fucked we create oil spils behind it.
/*	if (NumOilSpillsToDo)
	{
		float Dist = (GetPosition() - CVector(OilSpillLastX, OilSpillLastY, 0.0f)).Magnitude2D();
		if (Dist > 1.0f)
		{
			OilSpillLastX = GetPosition().x;
			OilSpillLastY = GetPosition().y;
			
			bool	bGroundFound;
			CVector	TempCoors = GetPosition();
			TempCoors.x += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f);
			TempCoors.y += CGeneral::GetRandomNumberInRange(-0.5f, 0.5f);
			TempCoors.z = CWorld::FindGroundZFor3DCoord(TempCoors.x, TempCoors.y, TempCoors.z + 1.0f, &bGroundFound);
			if(bGroundFound)
			{
				float	Angle = CGeneral::GetRandomNumberInRange(0.0f, 6.28f);
				float	Size = CGeneral::GetRandomNumberInRange(0.3f, 0.8f);
				
				CShadows::AddPermanentShadow(SHAD_TYPE_OIL, gpBloodPoolTex, &TempCoors, Size*CMaths::Cos(Angle), Size*CMaths::Sin(Angle),
															Size*CMaths::Sin(Angle), -Size*CMaths::Cos(Angle), 255, 0, 0, 0, 5.0f, 15000 + (CGeneral::GetRandomNumber() % 5000));
			}
			NumOilSpillsToDo--;
		}
	}
	else
	{
		if (GetVehicleType() != VEHICLE_TYPE_HELI && GetVehicleType() != VEHICLE_TYPE_PLANE)
		{
			if (m_nHealth < 500.0f && (CGeneral::GetRandomNumber() & 511) == 100)
			{
				NumOilSpillsToDo = CGeneral::GetRandomNumber() & 15;
			}
		}
	}*/

	if (GetModelIndex() == MODELID_CAR_COMBINE)
	{
		ProcessHarvester();
	}
	
	if(GetModelIndex()==MODELID_CAR_RHINO && (m_vecMoveSpeed.x != 0.0f || m_vecMoveSpeed.y != 0.0f || m_vecMoveSpeed.z != 0.0f
	|| m_vecTurnSpeed.x != 0.0f || m_vecTurnSpeed.y != 0.0f || m_vecTurnSpeed.z != 0.0f))
	{
		Door[REAR_LEFT_DOOR].SetExtraWheelPositions(1.0f, 1.0f, 1.0f, 1.0f);
		Door[REAR_RIGHT_DOOR].SetExtraWheelPositions(1.0f, 1.0f, 1.0f, 1.0f);
	}
}//end of CAutomobile::ProcessControl()...


/////////////////////////////////////////////////////////
//	ProcessAI():	moved all the AI and Player control inputs and logic
//					here from ProcessControl() to separate stuff a bit
//
//	Returns true only if want to return directly from ProcessControl after
//
TweakFloat CAR_INAIR_ROTF = 0.0007f;
TweakFloat MTRUCK_INAIR_ROTF = 0.0025f;
TweakFloat CAR_INAIR_ROTLIM = 0.02f;

TweakFloat CAR_INAIR_MOUSE_MULT = 0.02f;
//
bool CAutomobile::ProcessAI(uint32 &nProcContFlags)
{
	CColModel *pColModel = &GetColModel();

	// reset these flags
	AutoPilot.SlowingDownForCar = false;
	AutoPilot.SlowingDownForPed = false;

	if (AutoPilot.RecordingNumber >= 0 && !CVehicleRecording::bUseCarAI[AutoPilot.RecordingNumber]) return false;		// If there is a playback active on this bike we don't do ai.

	// do we need to do BOOST cheats for certain AI cars
	if( FindPlayerVehicle() && this != FindPlayerVehicle() && FindPlayerPed()->GetWantedLevel() > WANTED_LEVEL3
	&& (this->AutoPilot.Mission == CAutoPilot::MISSION_RAMPLAYER_FARAWAY || this->AutoPilot.Mission == CAutoPilot::MISSION_BLOCKPLAYER_FARAWAY
	||  this->AutoPilot.Mission == CAutoPilot::MISSION_RAMPLAYER_CLOSE || this->AutoPilot.Mission == CAutoPilot::MISSION_BLOCKPLAYER_CLOSE) 
	&& FindPlayerSpeed().Magnitude() > 0.3f)
	{
		nProcContFlags |= PROCCONT_FLAG_CAR_CHEATS;
		// Give us extra boost if going MUCH slower than player, or FAR away
		if((FindPlayerSpeed().Magnitude() > 0.4f &&	this->GetMoveSpeed().Magnitude() < 0.3f)
		|| (this->GetPosition() - FindPlayerCoors()).Magnitude() > 50.0f)
		{
			nProcContFlags |= PROCCONT_FLAG_EXTRA_CARCHEATS;
		}
	}
	else if(GetModelIndex()==MODELID_CAR_RCBANDIT && GetStatus()!=STATUS_PLAYER_REMOTE)
	{
		nProcContFlags |= PROCCONT_FLAG_CAR_CHEATS;
	}

	// do cheats that involve moving the COM around to stabilise the car
	if((nProcContFlags &PROCCONT_FLAG_CAR_CHEATS) ||  CCheat::IsCheatActive(CCheat::STRONGGRIP_CHEAT))
	{
		m_vecCOM.z = -1.0f*m_fHeightAboveRoad + 0.3f*m_fSuspensionLength[0];
	}
	else if(GetStatus()==STATUS_PHYSICS)
	{
		if(hFlagsLocal &HF_HYDRAULICS_GEOMETRY)
		{
			if(AutoPilot.Mission != CAutoPilot::MISSION_NONE && pColModel->GetCollisionData()
			&& pColModel->GetCollisionData()->m_nNoOfLines > 0)
			{
				m_vecCOM.y = 0.5f*(pColModel->GetCollisionData()->m_pLineArray[0].m_vecStart.y + pColModel->GetCollisionData()->m_pLineArray[1].m_vecStart.y);
				hFlagsLocal |= HF_NPC_NEUTRAL_HANDLING;
			}
			else
			{
				m_vecCOM = pHandling->CentreOfMass;
				hFlagsLocal &= ~HF_NPC_NEUTRAL_HANDLING;
			}
		}
		
		if(hFlagsLocal &HF_NPC_ANTI_ROLL)
			m_vecCOM.z = pHandling->CentreOfMass.z + (pColModel->GetBoundBoxMin().z - pHandling->CentreOfMass.z)*0.4f;
	}
	else // Must set it back or player can steal special 'stable' car
	{
		m_vecCOM = pHandling->CentreOfMass;
	}
	
	// Policecars and such will report crimes immediately
	// (No need for peds to use the phone)
	if (GetStatus() != STATUS_ABANDONED && GetStatus() != STATUS_WRECKED && GetStatus() != STATUS_PLAYER
		&& GetStatus() != STATUS_PLAYER_REMOTE && GetStatus() != STATUS_PLAYER_DISABLED)	
	{
		if (GetIsLawEnforcer())
		{
			ScanForCrimes();
		}
	}
	
	if(m_nVehicleFlags.bCanPark && !m_nVehicleFlags.bParking && VehicleCreatedBy!=MISSION_VEHICLE && AutoPilot.Mission==CAutoPilot::MISSION_CRUISE)
	{
	    if(((CTimer::m_FrameCounter + RandomSeed) &15) == 0)
	    {
    		if(GetModelIndex()!=MODELID_CAR_TAXI && GetModelIndex()!=MODELID_CAR_CABBIE)
//    		&& GetModelIndex()!=MODELID_CAR_BORGNINE && GetModelIndex()!=MODELID_CAR_KAUFMAN)
    		{
    	        //if(CTimer::GetTimeInMilliseconds()-m_nCreationTime>10000)
        	    {
            	    const CVector& vPos=GetPosition();
            	    const CVector& vForward=GetMatrix().GetForward();
            	    const CVector& vRight=GetMatrix().GetRight();
            	    const float fForwardDist=10;
            	    const float fRightDist=3.0f;
            	    CVector vTargetPos=vPos+vForward*fForwardDist+vRight*fRightDist;
            	    
            	    CColPoint colPoint;
            	    CEntity* pHitEntity=0;
            	    if(CWorld::ProcessLineOfSight(vPos,vTargetPos,colPoint,pHitEntity,true,true,true,false,false,false,false))
            	    {
            	        if(pHitEntity==this)
            	        {
//            	            pHitEntity=0;
//							CWorld::ProcessVerticalLine(vTargetPos+CVector(0,0,2),vTargetPos.z-2,colPoint,pHitEntity,true,false,false,false,false,false,NULL);
//							if(pHitEntity)
//							{
//								if(COLPOINT_SURFACETYPE_PAVEMENT==colPoint.GetSurfaceTypeB())
//								{
									CCarAI::GetCarToParkAtCoors(this,&vTargetPos);	                	        
//								}
//							}
						}
					}
            	    else
            	    {
//						pHitEntity=0;
//						CWorld::ProcessVerticalLine(vTargetPos+CVector(0,0,2),vTargetPos.z-2,colPoint,pHitEntity,true,false,false,false,false,false,NULL);
//						if(pHitEntity)
//						{
//							if(COLPOINT_SURFACETYPE_PAVEMENT==colPoint.GetSurfaceTypeB())
//							{
								CCarAI::GetCarToParkAtCoors(this,&vTargetPos);	                	        
//							}
//						}
					}
				}
			}
		}
	}

	bool bPlayerRemote = false; // only used in this wee section
	Int32 IndexOfRemotePlayer = -1;
	switch( GetStatus() )
	{
		case STATUS_PLAYER_REMOTE:
			// Test whether the player has activated the bomb on this remote controlled car
			if(CPad::GetPad(0)->CarGunJustDown() && !bDisableRemoteDetonation)
			{	// Make dude explode
				BlowUpCar(FindPlayerPed());
				CRemote::TakeRemoteControlledCarFromPlayer();
			}
			if(GetModelIndex()==MODELID_CAR_RCBANDIT && !bDisableRemoteDetonationOnContact)
			{
				CVector	Temp = GetPosition(), Temp2 = FindPlayerCoors();
//				if(RcbanditCheckHitWheels() || IsInWater || CTheZones::GetLevelFromPosition(&Temp) != CTheZones::GetLevelFromPosition(&Temp2) )
				if(RcbanditCheckHitWheels() || m_nPhysicalFlags.bIsInWater/* || CPopulation::IsPointInSafeZone(&Temp) */)
				{
					// Print a message if buggy exploded cause it went into wrong level
//					if (CTheZones::GetLevelFromPosition(&Temp) != CTheZones::GetLevelFromPosition(&Temp2))
					/*if (CPopulation::IsPointInSafeZone(&Temp))
					{
					    CGarages::TriggerMessage("HM2_5", -1, 5000);
					}*/
					CRemote::TakeRemoteControlledCarFromPlayer();
					BlowUpCar(FindPlayerPed());
				}
			}

			if(CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle == this)
				bPlayerRemote = true;

		// Fall through intended
		case STATUS_PLAYER:
			if(bPlayerRemote || (pDriver && pDriver->GetPedState()!=PED_EXIT_CAR && pDriver->GetPedState()!=PED_DRAGGED_FROM_CAR && pDriver->GetPedState()!=PED_ARRESTED) )
			{
				CPad *pPad = NULL;
				if(pDriver)	pPad = ((CPlayerPed *)pDriver)->GetPadFromPlayer();
				
				// Any expired references need to go
				PruneReferences();
				
				// Only do controls if this is the player in control on this machine
				if(bPlayerRemote)
				{
					IndexOfRemotePlayer = CWorld::FindPlayerSlotWithRemoteVehiclePointer(this);
					ASSERTMSG(IndexOfRemotePlayer >= 0, "ProcessAI - Can't find the player who is controlling this RC vehicle");
					if (IndexOfRemotePlayer >= 0)
					{
						ProcessControlInputs(IndexOfRemotePlayer);
					}
				}
				else if (pDriver->GetPedType() == PEDTYPE_PLAYER1 || pDriver->GetPedType() == PEDTYPE_PLAYER2)
				{
					ProcessControlInputs(pDriver->GetPedType() - PEDTYPE_PLAYER1);
				}
					
				// Also only do driveby's for the player
				if(GetStatus() == STATUS_PLAYER && GetVehicleType()!=VEHICLE_TYPE_HELI 
				&& (GetModelIndex()==MODELID_CAR_VORTEX || GetVehicleType()!=VEHICLE_TYPE_PLANE)
				&& GetModelIndex()!=MODELID_CAR_SWATVAN && GetModelIndex()!=MODELID_CAR_RHINO)
					DoDriveByShootings();
				
				// do some balancing for players car on 2 wheels by moving COM around
				if( CWorld::Players[CWorld::PlayerInFocus].nCarLess3WheelCounter > 500 
				&& GetVehicleType()!=VEHICLE_TYPE_HELI && GetVehicleType()!=VEHICLE_TYPE_PLANE )
				{
					float fCountScale = MIN(CWorld::Players[CWorld::PlayerInFocus].nCarLess3WheelCounter - 500, 1000)/(float)500.0f;
					if(GetMatrix().GetUp().z > -0.4f && pPad)
					{
						if(GetMatrix().GetRight().z > 0.0f)
							fCountScale *= 1.0f;
						else
							fCountScale *= -1.0f;
						m_vecCOM.z = pHandling->CentreOfMass.z + CAR_BALANCE_MULT*fCountScale*(pPad->GetSteeringLeftRight()/128.0f)*pColModel->GetBoundBoxMax().z;
					}
// only doing COM balance when upright, cause the switch gets too confusing
					else
					{
						m_vecCOM.z = pHandling->CentreOfMass.z - 0.5f*CAR_BALANCE_MULT*pColModel->GetBoundBoxMax().z;
					}
//					{
//						if(GetMatrix().GetRight().z > 0.0f)
//							fCountScale *= -1.0f;
//						else
//							fCountScale *= 1.0f;
//					}
//					m_vecCOM.z = pHandling->CentreOfMass.z + CAR_BALANCE_MULT*fCountScale*(CPad::GetPad(0)->GetSteeringLeftRight()/128.0f)*pColModel->GetBoundBoxMax().z;
				}
				else
				{
					m_vecCOM.z = pHandling->CentreOfMass.z;
				}
				
				if(nNoOfContactWheels==0 && pPad
				&& GetVehicleType()!=VEHICLE_TYPE_HELI && GetVehicleType()!=VEHICLE_TYPE_PLANE)
				{
					float fRotSpeed;
					float fRotForce = CAR_INAIR_ROTF;
					if(GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK)
						fRotForce = MTRUCK_INAIR_ROTF;
					else
						fRotForce *= CStats::GetFatAndMuscleModifier(STAT_MODIFIER_CAR_INAIR_BALANCE);
					
					// want to limit the scaling by mass so it doesn't look dumb for very big vehicles
					fRotForce *= m_fTurnMass*CMaths::Min(1.0f, 3000.0f / m_fTurnMass);

					float fStickX = pPad->GetSteeringLeftRight()/128.0f;
					float fStickY = pPad->GetSteeringUpDown()/128.0f;
#ifdef GTA_PC
					if(TheCamera.m_bUseMouse3rdPerson && CMaths::Abs(fStickX)< 0.05f && CMaths::Abs(fStickY)< 0.05f)
					{
						fStickX = CMaths::Min(1.5f, CMaths::Max(-1.5f, CAR_INAIR_MOUSE_MULT*pPad->GetAmountMouseMoved().x));
						fStickY = CMaths::Min(1.5f, CMaths::Max(-1.5f, CAR_INAIR_MOUSE_MULT*pPad->GetAmountMouseMoved().y));
					}
#endif
						
					// Rotation about Z
					if(pPad->GetHandBrake())
					{
						fRotSpeed = DotProduct(m_vecTurnSpeed, GetMatrix().GetUp());
						if( (fRotSpeed < CAR_INAIR_ROTLIM && fStickX < 0.0f)
						||	(fRotSpeed > -CAR_INAIR_ROTLIM && fStickX > 0.0f) )
							ApplyTurnForce( fStickX*CTimer::GetTimeStep()*fRotForce*GetMatrix().GetRight(), m_vecCOM + GetMatrix().GetForward() );
					}
					// Rotation about Y
					else if(!pPad->GetAccelerate())
					{
						fRotSpeed = DotProduct(m_vecTurnSpeed, GetMatrix().GetForward());
						if( (fRotSpeed < CAR_INAIR_ROTLIM && fStickX < 0.0f)
						||	(fRotSpeed > -CAR_INAIR_ROTLIM && fStickX > 0.0f) )
							ApplyTurnForce( fStickX*CTimer::GetTimeStep()*fRotForce*GetMatrix().GetRight(), m_vecCOM + GetMatrix().GetUp() );
					}
					
					// Rotation about X
					if(!pPad->GetAccelerate())
					{
						fRotSpeed = DotProduct(m_vecTurnSpeed, GetMatrix().GetRight());
						if( (fRotSpeed < CAR_INAIR_ROTLIM && fStickY < 0.0f)
						||	(fRotSpeed > -CAR_INAIR_ROTLIM && fStickY > 0.0f) )
							ApplyTurnForce( fStickY*CTimer::GetTimeStep()*fRotForce*GetMatrix().GetUp(), m_vecCOM + GetMatrix().GetForward() );
					}
				}

				DoSoftGroundResistance(nProcContFlags);
			}
#ifndef FINAL
			// cheat on debug console to give hydraulics to player's car	
			if(CVehicle::m_bGivePlayerHydraulics && GetVehicleType()==VEHICLE_TYPE_CAR && GetStatus()==STATUS_PLAYER)
			{
				hFlagsLocal |= HF_HYDRAULICS_INSTALLED;
				CVehicle::m_bGivePlayerHydraulics = false;
			}
#endif
			break;
			
		// This is here for the initial chase scene
		case STATUS_PLAYER_PLAYBACKFROMBUFFER:
		
//			if (CRecordDataForChase::Status != CRecordDataForChase::PLAYBACK_FINAL_DATA)
//			{
//				ProcessControlInputs(GetStatus() - STATUS_PLAYER_PRERECORD_1 + 2);
//			}
//			else
//			{	// Don't do a thing. (The inputs have already been read in from recorded data)
//			}
			break;
			
		case STATUS_SIMPLE:
			CCarAI::UpdateCarAI(this);
			CPhysical::ProcessControl();
			CCarCtrl::UpdateCarOnRails(this);
			// Audio needs these 3 variables. Should be able to take these out once Raymond has written special code for simple cars.
			nNoOfContactWheels=4;
			m_nDriveWheelsOnGroundLastFrame = m_nDriveWheelsOnGround;	// Record last frame's value
			m_nDriveWheelsOnGround=4;
			
			// Calculate what gear we're in so that sound still works.
			pHandling->Transmission.CalculateGearForSimpleCar(AutoPilot.ActualSpeed / 50.0f, m_nCurrentGear);	// 50 to convert to per frame speed rather than sensible per sec
			
			// Do wheels turning
			{
				float wheelRot = ProcessWheelRotation(WS_ROLLING, GetMatrix().GetForward(), m_vecMoveSpeed);
				m_aWheelPitchAngles[0] += wheelRot;
				m_aWheelPitchAngles[1] += wheelRot;
				m_aWheelPitchAngles[2] += wheelRot;
				m_aWheelPitchAngles[3] += wheelRot;
			}	
			PlayHornIfNecessary();
			ReduceHornCounter();
			m_nVehicleFlags.bVehicleColProcessed = false;
			m_nVehicleFlags.bAudioChangingGear = false;
			return true; // skip complex stuff
			break;
			
		case STATUS_PHYSICS:
		case STATUS_GHOST:
			CCarAI::UpdateCarAI(this);
			CCarCtrl::SteerAICarWithPhysics(this);
			CCarCtrl::ReconsiderRoute(this);
			PlayHornIfNecessary();
			// if being jacked -> make sure car comes to a stop
			if(m_nVehicleFlags.bIsBeingCarJacked)
			{
				SetGasPedal(0.0f);
				SetBrakePedal(1.0f);
				SetIsHandbrakeOn(true);
			}
			
			if(((m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[0].GetSurfaceTypeB())==ADHESION_GROUP_SAND)
			||	(m_aWheelRatios[1]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[1].GetSurfaceTypeB())==ADHESION_GROUP_SAND)
			||	(m_aWheelRatios[2]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[2].GetSurfaceTypeB())==ADHESION_GROUP_SAND)
			||	(m_aWheelRatios[3]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[3].GetSurfaceTypeB())==ADHESION_GROUP_SAND))
			&& GetModelIndex()!=MODELID_CAR_RCBANDIT && GetModelIndex()!=MODELID_CAR_SANDKING && GetModelIndex()!=MODELID_CAR_BUGGY)
			{
				m_nAutomobileFlags.bIsBoggedDownInSand = true;
				// apply move resistance
				if(CWeather::WetRoads > 0.0f)
					ApplyMoveForce(-SAND_WHEEL_MOVERES_MULT*(1.0f-CWeather::WetRoads)*m_fMass*CTimer::GetTimeStep()*m_vecMoveSpeed);
				else
					ApplyMoveForce(-SAND_WHEEL_MOVERES_MULT*m_fMass*CTimer::GetTimeStep()*m_vecMoveSpeed);
			}
			
			break;
			
		case STATUS_ABANDONED:
			for(int w=0; w<4; w++)
			{
				// might also want to check it's sitting on a proper transport vehicle
				if(m_aGroundPhysicalPtrs[w])
					m_nVehicleFlags.bRestingOnPhysical = true;
			}
			if(GetVehicleType()==VEHICLE_TYPE_HELI || (GetVehicleType()==VEHICLE_TYPE_PLANE && m_vecMoveSpeed.MagnitudeSqr() < 0.1f))
				SetBrakePedal(1.0f);
			if(m_nVehicleFlags.bRestingOnPhysical)
				SetBrakePedal(0.5f);
			else if(m_vecMoveSpeed.MagnitudeSqr() < 0.01f)
				SetBrakePedal(0.2f);
			else
				SetBrakePedal(0.0f);
				
			SetIsHandbrakeOn(false);
			SetSteerAngle(0.0f);
			SetGasPedal(0.0f);
			
			if(!IsAlarmActivated())
			{
				m_cHorn = 0;
			}
			// if being jacked -> make sure car comes to a stop
			if(m_nVehicleFlags.bIsBeingCarJacked)
			{
				SetGasPedal(0.0f);
				SetIsHandbrakeOn(true);
				SetBrakePedal(1.0f);
			}
			break;
			
		case STATUS_PLAYER_DISABLED:
MUST_FIX_THIS(gordon)
			/*
			if(m_vecMoveSpeed.MagnitudeSqr() < 0.01f || (pDriver && pDriver->IsPlayer()
			&& (pDriver->GetPedState()==PED_ARRESTED || pDriver->GetPedState()==PED_DRAGGED_FROM_CAR
			|| ((pDriver->GetPedState()==PED_EXIT_CAR) && !CanPedJumpOutCar())) ))
			*/
			if(m_vecMoveSpeed.MagnitudeSqr() < 0.01f || (pDriver && pDriver->IsPlayer()
			&& (pDriver->GetPedState()==PED_ARRESTED || 
			    pDriver->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_CAR_SLOW_BE_DRAGGED_OUT) ||
			    pDriver->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_CAR_QUICK_BE_DRAGGED_OUT) ||
			    pDriver->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_CAR_WAIT_TO_SLOW_DOWN))))
			{
				SetIsHandbrakeOn(true);
				SetBrakePedal(1.0f);
				SetGasPedal(0.0f);
			}
			else
			{
				SetBrakePedal(0.0f);
				SetIsHandbrakeOn(false);
			}
			SetSteerAngle(0.0f);
			SetGasPedal(0.0f);
			if(!IsAlarmActivated())
			{
				m_cHorn = 0;
			}
			break;
		case STATUS_WRECKED:
			SetBrakePedal(0.05f);
			SetIsHandbrakeOn(true);
			SetSteerAngle(0.0f);
			SetGasPedal(0.0f);
			if(!IsAlarmActivated())
			{
				m_cHorn = 0;
			}
			// do nothing
			break;
			
		case STATUS_TRAILER:
			SetBrakePedal(0.0f);
			SetIsHandbrakeOn(false);
			SetSteerAngle(0.0f);
			SetGasPedal(0.0f);

			if(!m_pTowingVehicle)
				BreakTowLink();

			break;
			
		default:
			ASSERT(0);
			break;
	}

	return false;
}

//
// this is the stuff that needs done to set up a vehicles collision stuff
// (mainly suspension ratios) because the vehicle is going to be processed
// but it was static last frame and didn't get it's collision done
//
void CAutomobile::ProcessControlCollisionCheck(bool bDoMovement)
{
	CMatrix matOld = GetMatrix();

	SetIsStuck(false);
	// this sets all the flags, and also flushes the col lists (which we want)
	SkipPhysics();
//	SetHasContacted(false);
//	SetIsInSafePosition(false);
//	SetWasPostponed(false);
//	SetHasHitWall(false);
	
	m_fTrueDistanceTravelled = 0.0f;
	m_nPhysicalFlags.bHalfSpeedCollision = false;
	m_nPhysicalFlags.bSkipLineCol = false;

	// reset suspension to avoid using invalid col points
	for(int i=0; i<4; i++)
		m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;

	if(bDoMovement)
	{
		ApplyMoveSpeed();
		ApplyTurnSpeed();
		uint16 nCounter = 0;
		while(CheckCollision() && nCounter++<5)
		{
			GetMatrix() = matOld;
			ApplyMoveSpeed();
			ApplyTurnSpeed();
		}
	}
	else
	{
		bool bUsesCollision = GetUsesCollision();
		SetUsesCollision(false);
		CheckCollision();
		SetUsesCollision(bUsesCollision);
	}
	
	SetIsInSafePosition(true);
	SetIsStuck(false);
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	ProcessHarvester
// DOES:		This function is called for the combine harvester.
//				It deals with killing people and destroying objects in
//				front of it.
///////////////////////////////////////////////////////////////////////////

void CAutomobile::ProcessHarvester()
{
	if(GetStatus()!=STATUS_PLAYER)
		return;
	
	CStreaming::m_bStreamHarvesterModelsThisFrame = true;
	
	const CMatrix& mat = GetMatrix();
	if (GetMoveSpeed().Magnitude2D() > 0.01f)
	{
		if (CTimer::m_FrameCounter & 1)
		{	// Go through the peds in the world to see whether they are so close to the harvester they should be killed.
			CPedPool& pool = CPools::GetPedPool();
			CPed* pPed;
			int32 i=pool.GetSize();
			
			while(i--)
			{
				pPed = pool.GetSlot(i);
				if(pPed && !pPed->IsPlayer())	// && pPed->CanBeDeleted())
				{
					// Calculate the coordinates of the ped in the system of the harvester.
					CVector	RelCoors = pPed->GetPosition() - this->GetPosition();
					float Forward = DotProduct(RelCoors, mat.GetForward());
					if (Forward > 4.0f && Forward < 5.0f)
					{
						float Side = DotProduct(RelCoors, mat.GetRight());
						if (ABS(Side) < 4.0f)
						{
							float Up = DotProduct(RelCoors, mat.GetUp());
							if (ABS(Up) < 4.0f)
							{
//								CWorld::Remove(pPed);	Doing this causes a crash as the ped could be directly after
//								delete pPed;			the harvester in the moving entity list and the CWorld::process code doesn't deal with this.
								pPed->FlagToDestroyWhenNextProcessed();
								m_HarvesterTimer = 20;
								m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_BODY_HARVEST);
							}
						}
					}
				}	
			}
		}
		else
		{		// Test for objects to destroy
			CObjectPool& pool = CPools::GetObjectPool();
			CObject* pObject;
			int32 i=pool.GetSize();
			
			while(i--)
			{
				pObject = pool.GetSlot(i);
				if(pObject && (pObject->GetModelIndex() == MI_GRASSHOUSE || pObject->GetModelIndex() == MI_GRASSPLANT) && pObject->m_nFlags.bIsVisible && !pObject->m_nFlags.bRenderDamaged)
				{
					// Calculate the coordinates of the ped in the system of the harvester.
					CVector	RelCoors = pObject->GetPosition() - this->GetPosition();
					float Forward = DotProduct(RelCoors, mat.GetForward());
					if (Forward > 4.0f && Forward < 5.0f)
					{
						float Side = DotProduct(RelCoors, mat.GetRight());
						if (ABS(Side) < 4.0f)
						{
							float Up = DotProduct(RelCoors, mat.GetUp());
							if (ABS(Up) < 4.0f)
							{
								pObject->ObjectDamage(99999.0f, NULL, NULL, this, WEAPONTYPE_RUNOVERBYCAR);
							}
						}
					}
				}	
			}
		}
	}
	
	if (m_HarvesterTimer)
	{
		Int32	MI = -1;

		CVector ObjectCoors = mat * (CVector(-1.2, -3.8, 1.5));
		CVector ObjectSpeed = mat.GetForward() * -0.1f;
		ObjectSpeed.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
		ObjectSpeed.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
	
		switch(m_HarvesterTimer)
		{
			case 1:
				MI = MI_HARVESTERBODYPART1;
				break;
			case 2:
				MI = MI_HARVESTERBODYPART2;
				break;
			case 3:
				MI = MI_HARVESTERBODYPART3;
				break;
			case 4:
				MI = MI_HARVESTERBODYPART2;
				break;
			case 6:
				MI = MI_HARVESTERBODYPART4;
				break;
			case 7:
				MI = MI_HARVESTERBODYPART1;
				break;
		}
		
		if (!CLocalisation::ShootLimbs())
		{
			MI = -1;
		}
		
		
		if (MI >= 0)
		{
			CObject *pObj = new CObject(MI, true);
			pObj->SetMatrix(mat);
			pObj->SetPosition(ObjectCoors);
			pObj->SetMoveSpeed(ObjectSpeed);
			pObj->SetTurnSpeed(CGeneral::GetRandomNumberInRange(-0.04f, 0.04f), CGeneral::GetRandomNumberInRange(-0.04f, 0.04f), CGeneral::GetRandomNumberInRange(-0.04f, 0.04f));
			pObj->ObjectCreatedBy = TEMP_OBJECT;
			pObj->UpdateRwMatrix();
			pObj->UpdateRwFrame();
			pObj->SetIsStatic(false);

			CObject::nNoTempObjects++;
			
			CWorld::Add(pObj);
		}
		m_HarvesterTimer--;
	
		if (CLocalisation::Blood())
		{
			if ((m_HarvesterTimer % 3) == 0)
			{
					//		 red   green blue  alpha size  rot   life
				FxPrtMult_c fxMults;
				fxMults.SetUp(0.15f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		
				g_fx.m_fxSysSmoke2->AddParticle(&ObjectCoors, &ObjectSpeed, 0.0f, &fxMults);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	Teleport
// DOES:		Moves this thing to different coordinates doing the things
//				that need to be done to stop the game from crashing.
///////////////////////////////////////////////////////////////////////////

void CAutomobile::Teleport(CVector NewCoors, Bool8 bClearOrientation)
{
	CWorld::Remove(this);
	this->SetPosition(NewCoors);

	if (bClearOrientation)
	{
		SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
	}
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);

/*
	this->m_aWheelRatios[0] = 99999.9f;
	this->m_aWheelRatios[1] = 99999.9f;
	this->m_aWheelRatios[2] = 99999.9f;
	this->m_aWheelRatios[3] = 99999.9f;
*/
	ResetSuspension();
	CWorld::Add(this);
}



void CAutomobile::ProcessSwingingDoor(int32 index, eDoors DoorID)
{
	uint8 Status = Damage.GetDoorStatus(DoorID);

//	if (GAMEISPAUSED) return;	// Don't want this to happen if everything else is frozen

	if(m_aCarNodes[index] && (Status==DT_DOOR_SWINGING_FREE || Status==DT_DOOR_BASHED_AND_SWINGING_FREE
	|| (Status < DT_DOOR_BASHED_AND_SWINGING_FREE && GetDamageImpulseMagnitude() > 100.0f && DoorID >= BOOT 
	&& (GetStatus()==STATUS_PLAYER || GetStatus()==STATUS_PHYSICS))))
	{
		CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
		CVector posn(matrix.GetTranslate());
		float orien[3] = {0.0f, 0.0f, 0.0f};
		if((Status==DT_DOOR_INTACT || Status==DT_DOOR_BASHED) && CanDoorsBeDamaged())
		{
			CVector vecDoorOffset = Multiply3x3(GetMatrix(), posn);
			if(Door[DoorID].ProcessImpact((CVehicle *)this, m_vecOldMoveSpeed, m_vecOldTurnSpeed, vecDoorOffset))
			{
				Status++;
				Damage.SetDoorStatus(DoorID, Status);
			}
		}
		
		if(Status==DT_DOOR_SWINGING_FREE || Status==DT_DOOR_BASHED_AND_SWINGING_FREE)
		{
			CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
			CVector posn(matrix.GetTranslate());
			float orien[3] = {0.0f, 0.0f, 0.0f};
			// bonnet gets blown up from wind
			if(DoorID==BONNET && (Door[DoorID].m_nDirn &DOOR_DIRN_MASK)==DOOR_AXIS_Y)
			{
				static float BONNET_DRAG_MULT = 0.05f;
				Door[DoorID].m_fAngVel += DotProduct(m_vecMoveSpeed, BONNET_DRAG_MULT*(0.1f+CMaths::Sin(Door[DoorID].m_fAngle))*GetMatrix().GetForward());
			}
		
			CVector vecDoorOffset = Multiply3x3(GetMatrix(), posn);
			if(Door[DoorID].Process((CVehicle *)this, m_vecOldMoveSpeed, m_vecOldTurnSpeed, vecDoorOffset))
			{
				Status--;
				Damage.SetDoorStatus(DoorID, Status);
#ifdef USE_AUDIOENGINE
		m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_CAR_BONNET_CLOSE + DoorID);
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, (AE_CAR_BONNET_CLOSE + DoorID), 0.0f);
#endif //USE_AUDIOENGINE
			}
			
			orien[Door[DoorID].RetAxes()] = Door[DoorID].RetDoorAngle();
			matrix.SetRotate(orien[0], orien[1], orien[2]);		
			matrix.Translate(posn);
			matrix.UpdateRW();
		
			if(DoorID==BONNET && Door[DoorID].m_nDoorState==DOOR_HIT_MAX_END
			&& DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()) > 0.4f)
			{
				CPhysical *pBonnet = SpawnFlyingComponent(CAR_BONNET ,CG_DOOR);
#ifdef USE_AUDIOENGINE
				m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_BONNET_FLUBBER_FLUBBER, (CEntity*)pBonnet);
#endif //USE_AUDIOENGINGE
				SetComponentVisibility(m_aCarNodes[CAR_BONNET], VEHICLE_ATOMIC_NONE);
				Damage.SetDoorStatus(BONNET, DT_DOOR_MISSING);
			
				if(pBonnet){
					if((CGeneral::GetRandomNumber() &1))
						pBonnet->SetMoveSpeed(0.4f*GetMoveSpeed() + 0.1f*GetMatrix().GetRight() + 0.5f*GetMatrix().GetUp());
					else
						pBonnet->SetMoveSpeed(0.4f*GetMoveSpeed() - 0.1f*GetMatrix().GetRight() + 0.5f*GetMatrix().GetUp());
					pBonnet->ApplyTurnForce(10.0f*GetMatrix().GetUp(), GetMatrix().GetForward());
				}
			}
		}
	}
}

CPhysical *CAutomobile::RemoveBonnetInPedCollision()
{
	if(m_aCarNodes[CAR_BONNET] && (Damage.GetDoorStatus(BONNET) == DT_DOOR_SWINGING_FREE || Damage.GetDoorStatus(BONNET) == DT_DOOR_BASHED_AND_SWINGING_FREE))
	{
		if(Door[BONNET].RetDoorAngle() > Door[BONNET].RetAngleWhenOpen()*0.4f)
		{
			CPhysical *pBonnet = SpawnFlyingComponent(CAR_BONNET ,CG_DOOR);
#ifdef USE_AUDIOENGINE
			m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_BONNET_FLUBBER_FLUBBER, (CEntity*)pBonnet);
#endif // USE_AUDIOENGINE
			
			SetComponentVisibility(m_aCarNodes[CAR_BONNET], VEHICLE_ATOMIC_NONE);
			Damage.SetDoorStatus(BONNET, DT_DOOR_MISSING);
			return pBonnet;
		}
	}
	return NULL;
}


// Name			:	CAutomobile::ResetSuspension
// Purpose		:	I think this sets the car wheels as if the car was in the air
//						with no wheels touching the ground. This should be called 
//						when a car is teleported around the map.
// Parameters	:	none
// Returns		:	nothing
void CAutomobile::ResetSuspension(void)
{
	for (int32 i = 0; i < 4; i++)
	{
		m_aWheelRatios[i] = 1.0f;
		m_aWheelCounts[i] = 0.0f;
		m_aWheelPitchAngles[i] = DEGTORAD(0.0f);
		m_aWheelState[i] = WS_ROLLING;
	}
}


//
//
TweakFloat PLAYERCAR_WHEELTILT_MULT 	= -1.0f;
TweakFloat PLAYERCAR_WHEELSTEER_MULT 	= 0.6f;
TweakFloat PLAYERCAR_WHEELTILT_HIATIAN 	= 0.6f;
TweakFloat KART_WHEEL_WIDTH_MULT = 3.0f;
//
void CAutomobile::UpdateWheelMatrix(int32 nWheelIndex, int32 nOptionFlags)
{
	if(m_aCarNodes[nWheelIndex] == NULL)
		return;
		
	bool bFrontWheel = false;
	bool bRearWheel = false;
	bool bSteeringWheel = false;
	int32 nWheelNumber;
	float fRhsWheel;
	float fAckermanSteer = 0.0f;
	CMatrix matrix;
	CVector posn, pos;
	CVehicleModelInfo *pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(this->GetModelIndex());
	const RwRGBA rgbaSplashWaterColor = {255, 255, 255, 32};

	switch(nWheelIndex)
	{
		case CAR_WHEEL_LF:
			nWheelNumber = 0;
			fRhsWheel = -1.0f;
			bFrontWheel = true;
			if(!(hFlagsLocal &HF_STEER_REARWHEELS))
				fAckermanSteer = m_fSteerAngle;
			break;
		case CAR_WHEEL_RF:
			nWheelNumber = 2;
			fRhsWheel = 1.0f;
			bFrontWheel = true;
			if(!(hFlagsLocal &HF_STEER_REARWHEELS))
				fAckermanSteer = m_fSteerAngle;
			break;
		case CAR_WHEEL_LR:
		case CAR_WHEEL_LM:
			nWheelNumber = 1;
			fRhsWheel = -1.0f;
			bRearWheel = true;
			if(hFlagsLocal &HF_STEER_REARWHEELS)
				fAckermanSteer = -m_fSteerAngle;
			else if(hFlagsLocal &HF_HANDBRAKE_REARWHEELSTEER)
				fAckermanSteer = m_f2ndSteerAngle;
			break;
		case CAR_WHEEL_RR:
		case CAR_WHEEL_RM:
			nWheelNumber = 3;
			fRhsWheel = 1.0f;
			bRearWheel = true;
			if(hFlagsLocal &HF_STEER_REARWHEELS)
				fAckermanSteer = -m_fSteerAngle;
			else if(hFlagsLocal &HF_HANDBRAKE_REARWHEELSTEER)
				fAckermanSteer = m_f2ndSteerAngle;
			break;
		default:
			ASSERT(0);
			return;
	}
	
	if(fAckermanSteer!=0.0f && !(nOptionFlags &WHEEL_OPTION_NOSTEER))
	{
		if(fRhsWheel < 0.0f)
		{
			if(bRearWheel)
			{
				if(fAckermanSteer > 0.0f)
					fAckermanSteer *= PLAYERCAR_WHEELSTEER_MULT;
			}
			else
			{
				if(fAckermanSteer < 0.0f)
					fAckermanSteer *= PLAYERCAR_WHEELSTEER_MULT;
			}
			// for Lhs wheels need to switch rotation angle
			fAckermanSteer += PI;
		}
		else
		{
			if(bRearWheel)
			{
				if(fAckermanSteer < 0.0f)
					fAckermanSteer *= PLAYERCAR_WHEELSTEER_MULT;
			}
			else
			{
				if(fAckermanSteer > 0.0f)
					fAckermanSteer *= PLAYERCAR_WHEELSTEER_MULT;
			}
		}
	}
	else
	{
		if(fRhsWheel < 0.0f)
			fAckermanSteer = PI;
		else
			fAckermanSteer = 0.0f;
	}

	matrix.Attach(RwFrameGetMatrix(m_aCarNodes[nWheelIndex]));
	posn = matrix.GetTranslate();
	posn.z = m_aWheelSuspensionHeights[nWheelNumber];

	float fScale = m_wheelScale;
	float fWidth = 1.0f;
	if(pModelInfo->GetWheelModelId() == -1) // wheels are part of the model and already scaled
	{
		if(bRearWheel)
			fScale *= pModelInfo->GetWheelScale(false) / pModelInfo->GetWheelScale(true);
		
		fWidth = pModelInfo->GetWheelScale(true) / (STD_CAR_WHEEL_SCALE*m_wheelScale);
	}
	else
	{
		if(bRearWheel)
			fScale = pModelInfo->GetWheelScale(false);
		else
			fScale = pModelInfo->GetWheelScale(true);
	}
	
	if(GetModelIndex()==MODELID_CAR_COMBINE && (nWheelIndex==CAR_WHEEL_LM || nWheelIndex==CAR_WHEEL_RM))
	{
		static float COMBINE_MIDDLE_WHEEL_SIZE = 1.7f;
		static float COMBINE_MIDDLE_WHEEL_POS = 0.45f;
		
		fScale = COMBINE_MIDDLE_WHEEL_SIZE / pModelInfo->GetWheelScale(true);
		posn.z +=COMBINE_MIDDLE_WHEEL_POS*(COMBINE_MIDDLE_WHEEL_SIZE - pModelInfo->GetWheelScale(false));
		if(fRhsWheel < 0.0f)
			fAckermanSteer -= PI;
		fAckermanSteer *= 0.5f;
		if(fRhsWheel < 0.0f)
			fAckermanSteer += PI;
	}
	else if(GetModelIndex()==MODELID_PLANE_STUNT && nWheelIndex==CAR_WHEEL_LF)
		fAckermanSteer = 0.0f;

	
	if(GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK || GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
		fWidth = 1.0f;
	else if(bRearWheel && hFlagsLocal &0x0000F000)// && !(pHandling->mFlags &MF_DOUBLE_REAR_WHEELS))
	{
		if(hFlagsLocal &HF_WHEEL_R_NARROW_X2)
			fWidth *= 0.65f;
		else if(hFlagsLocal &HF_WHEEL_R_NARROW)
			fWidth *= 0.8f;
		else if(hFlagsLocal &HF_WHEEL_R_WIDE)
			fWidth *= 1.1f;
		else if(hFlagsLocal &HF_WHEEL_R_WIDE_X2)
			fWidth *= 1.25f;
	}
	else if(bFrontWheel && hFlagsLocal &0x00000F00)
	{
		if(hFlagsLocal &HF_WHEEL_F_NARROW_X2)
			fWidth *= 0.65f;
		else if(hFlagsLocal &HF_WHEEL_F_NARROW)
			fWidth *= 0.8f;
		else if(hFlagsLocal &HF_WHEEL_F_WIDE)
			fWidth *= 1.1f;
		else if(hFlagsLocal &HF_WHEEL_F_WIDE_X2)
			fWidth *= 1.25f;
	}

	
	if(GetModelIndex()==MODELID_CAR_KART)
		fWidth *= KART_WHEEL_WIDTH_MULT;

	matrix.SetScale(fWidth*fScale, fScale, fScale);
	
	if(GetVehicleType()==VEHICLE_TYPE_HELI)
		matrix.Rotate(0.0f, 0.0f, fAckermanSteer);
	else if(Damage.GetWheelStatus(nWheelNumber) == DT_WHEEL_BURST)
		matrix.Rotate(fRhsWheel*m_aWheelPitchAngles[nWheelNumber], 0.0f, fAckermanSteer + 0.3f*CMaths::Sin(fRhsWheel*m_aWheelPitchAngles[nWheelNumber]));
	else	
		matrix.Rotate(fRhsWheel*m_aWheelPitchAngles[nWheelNumber], 0.0f, fAckermanSteer);

	if(m_nAutomobileFlags.bIsMonsterTruck || (hFlagsLocal &HF_HYDRAULICS_INSTALLED) 
	|| ((pHandling->mFlags &MF_AXLE_F_SOLID) && bFrontWheel) || ((pHandling->mFlags &MF_AXLE_R_SOLID) && bRearWheel))
	{
		int32 nOppositeWheel;
		if(nWheelNumber > 1)	nOppositeWheel = nWheelNumber - 2;
		else					nOppositeWheel = nWheelNumber + 2;
		
		float fTiltAngle = m_aWheelSuspensionHeights[nWheelNumber] - m_aWheelSuspensionHeights[nOppositeWheel];
		fTiltAngle = CMaths::ATan2(-1.0f*fRhsWheel*fTiltAngle, 2.0f*CMaths::Abs(posn.y));
		matrix.RotateY(fTiltAngle);
	}
	else// if(GetStatus()==STATUS_PLAYER)
	{
		// CHEAT /////////////////////////////		
		if((CCheat::IsCheatActive(CCheat::BACKTOTHEFUTURE_CHEAT) || GetModelIndex()==MODELID_CAR_VORTEX)
		&& m_aRatioHistory[nWheelNumber] < BILLS_EXTENSION_LIMIT && !m_nVehicleFlags.bIsDrowning)
		{
			if( !(nOptionFlags &WHEEL_OPTION_NOHOVERTILT) )
				matrix.RotateY( -HALF_PI*fRhsWheel );

			if( !(nOptionFlags &WHEEL_OPTION_NOHOVERPARTICLES) )
			{
/*			
#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (AddParticle)
// hover particles

				if( (CTimer::m_FrameCounter + 3) &1 )
					CParticle::AddParticle(PARTICLE_STEAM_NY_SLOWMOTION,  m_aWheelColPoints[nWheelNumber].GetPosition(), 0.5f*m_vecMoveSpeed + fRhsWheel*0.1f*GetMatrix().GetRight(), NULL, 0.4f, rgbaSplashWaterColor);
				else
					CParticle::AddParticle(PARTICLE_CAR_SPLASH,  m_aWheelColPoints[nWheelNumber].GetPosition(), 0.3f*m_vecMoveSpeed + fRhsWheel*0.15f*GetMatrix().GetRight() + CVector(0.0f, 0.0f, 0.1f), NULL, 0.15f, rgbaSplashWaterColor, CGeneral::GetRandomNumberInRange(0.0f, 10.0f), CGeneral::GetRandomNumberInRange(0.0f, 90.0f), 1);

#endif // MN - OLD FX SYSTEM (AddParticle)
*/
	
				// MN - FX SYSTEM -------------------------------
				// hover particles
				//		 				 red   green blue  alpha size  rot   life
				FxPrtMult_c fxMultsWater(1.0f, 1.0f, 1.0f, 0.2f, 0.5f, 0.0f, 0.05f);
		
				CVector pos = m_aWheelColPoints[nWheelNumber].GetPosition();
				CVector vel = 0.5f*m_vecMoveSpeed + fRhsWheel*1.0f*GetMatrix().GetRight();

				if (g_surfaceInfos.IsWater(m_aWheelColPoints[nWheelNumber].GetSurfaceTypeB()))
				{	
					if ((CTimer::m_FrameCounter+3) & 1)
					{
						vel.z += 2.0f;
						g_fx.m_fxSysWaterSplash->AddParticle(&pos, &vel, 0.0f, &fxMultsWater); 
					}
					else
					{
						g_fx.m_fxSysSmokeHuge->AddParticle(&pos, &vel, 0.0f, &fxMultsWater); 
					}
				}
				else if (g_surfaceInfos.IsSand(m_aWheelColPoints[nWheelNumber].GetSurfaceTypeB()))
				{
					//		 				red    green  blue   alpha  size  rot   life
					FxPrtMult_c fxMultsSand(0.81f, 0.67f, 0.57f, 0.35f, 0.5f, 0.0f, 0.05f);
					
					g_fx.m_fxSysSand->AddParticle(&pos, &vel, 0.0f, &fxMultsSand); 
				}
				
				// ----------------------------------------------

			}
		}
		else if(!(nOptionFlags &WHEEL_OPTION_NOTILT)
		&& !(bFrontWheel && pHandling->mFlags &MF_AXLE_F_NOTILT)
		&& !(bRearWheel && pHandling->mFlags &MF_AXLE_R_NOTILT))
		{
			float fNewOffset = posn.z + m_fHeightAboveRoad - pModelInfo->GetWheelScale(bFrontWheel)*0.5f;
			
			if((bFrontWheel && pHandling->mFlags &MF_AXLE_F_REVERSETILT) || (bRearWheel && pHandling->mFlags &MF_AXLE_R_REVERSETILT))
				matrix.RotateY( -CMaths::ASin(MIN(1.0f, MAX(-1.0f, fRhsWheel*PLAYERCAR_WHEELTILT_MULT*fNewOffset))) );
			else
				matrix.RotateY( CMaths::ASin(MIN(1.0f, MAX(-1.0f, fRhsWheel*PLAYERCAR_WHEELTILT_MULT*fNewOffset))) );
		}
	}
			
	matrix.Translate(posn);
	matrix.UpdateRW();
}



//
float fTestRotateAngle = 0.0f;
int16 nTestRotateAxis = 1;
//
#define PARTICLE_DIST		(70.0f * TheCamera.GenerationDistMultiplier)
#define PARTICLE_DIST_SQ 	PARTICLE_DIST*PARTICLE_DIST
//
//
//
//
TweakFloat PHEONIX_FLUTTER_PERIOD	= 70.0f;
TweakFloat PHEONIX_FLUTTER_AMP		= 0.13f;
TweakFloat CEMENT_TILT_ANGLE		= DEGTORAD(-10.0f);
TweakFloat CEMENT_ROT_SPEED_MULT	= 0.02f;
//
void CAutomobile::PreRender()
{
	CVector		SirenCoorsL, SirenCoorsR, SirenCoors;
	UInt8		SirenState, SirenNum;
	UInt8		Red1, Green1, Blue1, Red2, Green2, Blue2;

	CVehicle::PreRender();
	// debug. (obr)
#ifdef GTA_NETWORK
//m_nFlags.bRenderBlack = bControlledByServer; netobr
//if (m_nGettingOutFlags || m_nGettingInFlags) m_nFlags.bRenderBlack = CTimer::m_FrameCounter & 1;
#endif

		// Perhaps we should have some antennas for this car ?
	if (this->GetModelIndex() == MODELID_CAR_RCBANDIT)
	{
		//CAntennas::RegisterOne( (UInt32)this, GetMatrix().GetUp(), GetMatrix()*CVector(0.218f, -0.444f, 0.391f), 1.0f);
	}

	if( CCheat::IsCheatActive(CCheat::BACKTOTHEFUTURE_CHEAT))
		DoHoverSuspensionRatios();

	// process wheel distance from car body
	CColModel &colModel = GetColModel();
	CCollisionData* pColData = colModel.GetCollisionData();
	
	if(m_nVehicleFlags.bVehicleColProcessed && GetVehicleType()!=VEHICLE_TYPE_MONSTERTRUCK)
	{
		DoBurstAndSoftGroundRatios();
	
		for(int32 i=0; i<4; i++)
		{
			float ratio = 1.0f - m_fSuspensionLength[i] / m_fLineLength[i];
			ratio = (m_aWheelRatios[i] - ratio) / (1.0f - ratio);

			float fHeight = pColData->m_pLineArray[i].m_vecStart.z;
			if(ratio > 0.0f)
				fHeight -= ratio * m_fSuspensionLength[i];

			// don't dampen movement of wheels for hydraulic suspension car!
			if(m_aWheelRatios[i] < 1.0f && fHeight > m_aWheelSuspensionHeights[i])
			//|| (m_nPhysicalFlags.bUsingSpecialColModel && hFlagsLocal &HF_HYDRAULICS_INSTALLED)))
			{
				m_aWheelSuspensionHeights[i] = fHeight;
			}
			// dampens the speed of the wheel moving down
			else
			{
				m_aWheelSuspensionHeights[i] += (fHeight - m_aWheelSuspensionHeights[i]) * AUTOMOBILE_WHEELSUSPENSIONINERTIA;
			}
		}
	}

//	if(bVisibleForCamera)
	{
		// current speed of the car:
		float	fAutomobileSpeed;
		fAutomobileSpeed = DotProduct(this->m_vecMoveSpeed, this->GetMatrix().GetForward())/GAME_VELOCITY_CONST;
		// second version in game units
		float fTempSpeed = m_vecMoveSpeed.Magnitude();

		// Do skidmarks etc here since the position has been updated for this frame.
		if(GetModelIndex() == MODELID_CAR_DODO || GetModelIndex()==MODELID_CAR_VORTEX)
		{
			// don't do skidmarks/rubber smoke for 3 wheeled aeroplane
		}
		else if(GetModelIndex() == MODELID_CAR_RCBANDIT)
		{
/*
#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (AddParticle)
// radio controlled car skids

			// for radio-control model car just do extra small rubber smoke for skids
			for(int32 i=0; i<4; i++)
			{
				switch(this->m_aWheelState[i])
				{
					case WS_SPINNING:
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, m_aWheelColPoints[i].GetPosition()+CVector (0.0f,0.0f,0.05f), CVector(0.0f,0.0f,0.0f), NULL,0.1f);
					break;
					
					case WS_SKIDDING:
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, m_aWheelColPoints[i].GetPosition()+CVector (0.0f,0.0f,0.05f), CVector(0.0f,0.0f,0.0f), NULL,0.1f);
					break;
					
					case WS_LOCKED:
						CParticle::AddParticle(PARTICLE_RUBBER_SMOKE, m_aWheelColPoints[i].GetPosition()+CVector (0.0f,0.0f,0.05f), CVector(0.0f,0.0f,0.0f), NULL,0.1f);
					break;
				} // end switch
			}

#endif // MN - OLD FX SYSTEM (AddParticle)
*/


			// MN - FX SYSTEM -------------------------------
			// TODO: radio controlled car skids
			// ----------------------------------------------

		}
		else	
		{
			//CColPoint m_aWheelColPoints[4];

			// recalculate some simple physics for wheels in SIMPLE vehicles:
			if(this->GetStatus() == STATUS_SIMPLE)
			{
				const float WHEEL_SCALE = 1.5f;// 1.5f;

				CMatrix matrix;
				CVector posn, pos;
				
				// rear right:
				matrix.Attach(RwFrameGetMatrix(this->m_aCarNodes[CAR_WHEEL_RR]));
				posn = matrix.GetTranslate();
				posn.z = WHEEL_SCALE*m_aWheelSuspensionHeights[3];
				//posn.z *= pModelInfo->GetWheelScale();
				pos = this->GetMatrix() * posn;
				this->m_aWheelColPoints[3].SetPosition(pos);
				this->m_aWheelColPoints[3].SetSurfaceTypeB(SURFACE_TYPE_DEFAULT);
				
				// rear left:
				matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LR]));
				posn = matrix.GetTranslate();
				posn.z = WHEEL_SCALE*m_aWheelSuspensionHeights[1];
				//posn.z *= pModelInfo->GetWheelScale();
				pos = this->GetMatrix() * posn;
				this->m_aWheelColPoints[1].SetPosition(pos);
				this->m_aWheelColPoints[1].SetSurfaceTypeB(SURFACE_TYPE_DEFAULT);

				// front right
				matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
				posn = matrix.GetTranslate();
				posn.z = WHEEL_SCALE*m_aWheelSuspensionHeights[2];
				//posn.z *= pModelInfo->GetWheelScale();
				pos = this->GetMatrix() * posn;
				this->m_aWheelColPoints[2].SetPosition(pos);
				this->m_aWheelColPoints[2].SetSurfaceTypeB(SURFACE_TYPE_DEFAULT);

				// front left
				matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
				posn = matrix.GetTranslate();
				posn.z = WHEEL_SCALE*m_aWheelSuspensionHeights[0];
				//posn.z *= pModelInfo->GetWheelScale();
				pos = this->GetMatrix() * posn;
				this->m_aWheelColPoints[0].SetPosition(pos);
				this->m_aWheelColPoints[0].SetSurfaceTypeB(SURFACE_TYPE_DEFAULT);
			}


			// flag: don't produce wheel drops if car is going too fast...
			bool32 bProduceWheelDrops = (ABS(fAutomobileSpeed)<90.f)?TRUE:FALSE;
			
			
			// stupid check?
			if(GetStatus()==STATUS_PHYSICS	|| GetStatus()==STATUS_PLAYER
			|| GetStatus()==STATUS_PLAYER_PLAYBACKFROMBUFFER || GetStatus()==STATUS_SIMPLE)
			{
				// try only allowing smoke from skidding front wheels if rear wheels are also skidding
				// want to avoid front tyre smoke from steering left/right when moving in a pretty straight line
				bool8 bDoFrontSkidSmoke = false;
				if(m_aWheelState[1] == WS_SKIDDING || m_aWheelState[3] == WS_SKIDDING)
					bDoFrontSkidSmoke = true;
				
				float fOutsideVector = 1.0f;
				uint32 nWheelParticleFlags;
				// Loop for all 4 wheels:
				for(int32 i=0; i<4; i++)
				{
					nWheelParticleFlags = 0;
					if((i==0 || i==2) && bDoFrontSkidSmoke==false)
						nWheelParticleFlags += WHEEL_PARTICLE_NOSKIDSMOKE;
					
					if(bWheelBloody[i])
						nWheelParticleFlags += WHEEL_PARTICLE_BLOODY;
						
					if(bMoreSkidMarks[i])
						nWheelParticleFlags += WHEEL_PARTICLE_EXTRASKIDS;
						
					if(i==0 || i==1)
						fOutsideVector = -1.0f;
					else
						fOutsideVector = 1.0f;
				
					AddSingleWheelParticles(m_aWheelState[i], Damage.GetWheelStatus(i), m_aRatioHistory[i], fTempSpeed, &m_aWheelColPoints[i], &(m_aWheelColPoints[i].GetPosition()), fOutsideVector, i, aWheelSkidmarkType[i], &bWheelBloody[i], nWheelParticleFlags);
				}//for(int32 i=0; i<4; i++)...

			} // if(this->GetStatus() == ...
					
		} // end else
		
		// if car has a middle right wheel -> assume 6wheeler
		// want to add skidmarks for second 2 rear tyres (wheels 1 & 3)
		if(m_aCarNodes[CAR_WHEEL_RM]) 
		{	
			uint32 nWheelParticleFlags = 0;
			if(bWheelBloody[1])
				nWheelParticleFlags += WHEEL_PARTICLE_BLOODY;
			if(bMoreSkidMarks[1])
				nWheelParticleFlags += WHEEL_PARTICLE_EXTRASKIDS;
				
			CVector vecColPosition = m_aWheelColPoints[1].GetPosition() + GetMatrix().GetForward()*2.0f;
			AddSingleWheelParticles(m_aWheelState[1], Damage.GetWheelStatus(1), m_aRatioHistory[1], fTempSpeed, &m_aWheelColPoints[1], &vecColPosition, 1.0f, 5, aWheelSkidmarkType[1], &bWheelBloody[1], nWheelParticleFlags);


			nWheelParticleFlags = 0;
			if(bWheelBloody[3])
				nWheelParticleFlags += WHEEL_PARTICLE_BLOODY;
			if(bMoreSkidMarks[3])
				nWheelParticleFlags += WHEEL_PARTICLE_EXTRASKIDS;
				
			vecColPosition = m_aWheelColPoints[3].GetPosition() + GetMatrix().GetForward()*2.0f;
			AddSingleWheelParticles(m_aWheelState[3], Damage.GetWheelStatus(3), m_aRatioHistory[3], fTempSpeed, &m_aWheelColPoints[3], &vecColPosition, 1.0f, 6, aWheelSkidmarkType[3], &bWheelBloody[3], nWheelParticleFlags);
		}//if(m_aCarNodes[CAR_WHEEL_RM])...


		// do splashes on car roofs
		if(	!CCullZones::CamNoRain() &&	!CCullZones::PlayerNoRain()
		&& CMaths::Abs(fAutomobileSpeed) < 20.0f &&	CWeather::Rain > 0.02f && CGame::currArea == AREA_MAIN_MAP)
		{
			AddWaterSplashParticles();
		}	

		// car exhaust fumes stuff:
		const float SPEED_LIMIT_NO_EXHAUSTS = 130.0f;
		// engine is being heard, so we also produce exhausts... up to certain speed
		if( this->m_nVehicleFlags.bEngineOn && !(pHandling->mFlags &MF_NO_EXHAUST)
		&& fAutomobileSpeed < SPEED_LIMIT_NO_EXHAUSTS && !m_nVehicleFlags.bIsDrowning &&
		!m_nVehicleFlags.bDisableParticles)
		{
			AddExhaustParticles();
		}

		AddDamagedVehicleParticles();

	}//if(bVisibleForCamera)....
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Do possible specials
	switch (GetModelIndex())
	{
		case MODELID_CAR_COPCAR_LA:
		case MODELID_CAR_COPCAR_SF:
		case MODELID_CAR_COPCAR_LV:
		case MODELID_CAR_COPCAR_RURAL:
		case MODELID_CAR_ENFORCER:
		case MODELID_CAR_AMBULANCE:
		case MODELID_CAR_FIRETRUCK:
					// Put in a test whether siren is on or not
			if (m_nVehicleFlags.bSirenOrAlarm)
			{	// Render lights on ground
				Red1 = Green1 = Blue1 = Red2 = Green2 = Blue2 = 0;
				#define LIGHTVAL (255)
				switch (GetModelIndex())
				{
					case MODELID_CAR_COPCAR_RURAL:
						SirenCoorsL = CVector(0.7f, -0.1f, 1.2f);
						SirenCoorsR = CVector(-0.7f, -0.1f, 1.2f);
						Red1 = LIGHTVAL;
						Blue2 = LIGHTVAL;
						break;
					case MODELID_CAR_COPCAR_LA:
					case MODELID_CAR_COPCAR_SF:
					case MODELID_CAR_COPCAR_LV:
						SirenCoorsL = CVector(0.7f, -0.4f, 1.0f);
						SirenCoorsR = CVector(-0.7f, -0.4f, 1.0f);
						Red1 = LIGHTVAL;
						Blue2 = LIGHTVAL;
						break;
					case MODELID_CAR_ENFORCER:
						SirenCoorsL = CVector(0.55f, 1.1f, 1.4f);
						SirenCoorsR = CVector(-0.55f, 1.1f, 1.4f);
						Red1 = LIGHTVAL;
						Blue2 = LIGHTVAL;
						break;
					case MODELID_CAR_AMBULANCE:
						SirenCoorsL = CVector(0.6f, 0.9f, 1.2f);
						SirenCoorsR = CVector(-0.6f, 0.9f, 1.2f);
						Red1 = LIGHTVAL;
						Red2 = LIGHTVAL;
						Green2 = LIGHTVAL;
						Blue2 = LIGHTVAL;
						break;
					case MODELID_CAR_FIRETRUCK:
						SirenCoorsL = CVector(0.9f, 3.2f, 1.3f);
						SirenCoorsR = CVector(-0.9f, 3.2f, 1.3f);
						Red1 = LIGHTVAL;
						Red2 = LIGHTVAL;
						Green2 = LIGHTVAL;
						Blue2 = 0;
						break;
					default:
						ASSERT(0);
						break;
				}

				UInt32	TimeVal;
				UInt8	Red, Green, Blue;
				TimeVal = CTimer::GetTimeInMilliseconds() & 1023;
				if (TimeVal < 512)
				{
					Red = Red1 / 6;
					Green = Green1 / 6;
					Blue = Blue1 / 6;
				}
				else
				{
					Red = Red2 / 6;
					Green = Green2 / 6;
					Blue = Blue2 / 6;
				}
				
				TimeVal = TimeVal & 511;
				if (TimeVal < 100)
				{
					Red *= TimeVal / 100.0f;
					Green *= TimeVal / 100.0f;
					Blue *= TimeVal / 100.0f;
				}
				else
				if (TimeVal > 412)
				{
					Red *= (512-TimeVal) / 100.0f;
					Green *= (512-TimeVal) / 100.0f;
					Blue *= (512-TimeVal) / 100.0f;
				}
	
				CVector Temp = this->GetPosition();
				float Angle = (CTimer::GetTimeInMilliseconds() & 1023) * (6.28f / 1024.0f);
				float Sinus = 8.0f * CMaths::Sin(Angle);
				float Cosinus = 8.0f * CMaths::Cos(Angle);

				//CShadows::StoreCarLightShadow(this, ((UInt32)this)+21, gpShadowHeadLightsTex, &Temp, Cosinus, Sinus,
				//								Sinus, -Cosinus, Red, Green, Blue, 8.0f);
												
				// Register a light as well.
				CPointLights::AddLight(CPointLights::PLTYPE_POINTLIGHT, Temp + (2.0f * this->GetMatrix().GetUp()), CVector(0.0f, 0.0f, 0.0f), 10.0f, Red/200.0f, Green/200.0f, Blue/200.0f, CPointLights::FOGEFF_OFF);
				
				// Do actual coronas for the lights
				{					
//					SirenCoorsL = GetMatrix() * SirenCoorsL;
//					SirenCoorsR = GetMatrix() * SirenCoorsR;

					SirenCoorsL = SirenCoorsL;
					SirenCoorsR = SirenCoorsR;
					
					// During the day the lights are less noticable
					Red1 *= CTimeCycle::GetSpriteBrightness();
					Green1 *= CTimeCycle::GetSpriteBrightness();
					Blue1 *= CTimeCycle::GetSpriteBrightness();
					Red2 *= CTimeCycle::GetSpriteBrightness();
					Green2 *= CTimeCycle::GetSpriteBrightness();
					Blue2 *= CTimeCycle::GetSpriteBrightness();


					for (SirenNum = 0; SirenNum < 4; SirenNum++)
					{
						SirenState = ((CTimer::GetTimeInMilliseconds() + (SirenNum<<6)) >> 8) & 3;
				
						SirenCoors = ((SirenCoorsL * ((float)SirenNum)) + (SirenCoorsR * (3.0f-SirenNum))) * 0.333333333f;
						switch (SirenState)
						{
							case 0:
								CCoronas::RegisterCorona(((UInt32)this)+21+SirenNum, this, Red1, Green1, Blue1, 255, SirenCoors, 0.4f, 150.0f*TheCamera.LODDistMultiplier, CCoronas::CORONATYPE_SHINYSTAR, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF, CCoronas::TRAIL_OFF, 0.0f, false, 1.5f, false, 15.0f, false, true);						
								break;
							case 2:
								CCoronas::RegisterCorona(((UInt32)this)+21+SirenNum, this, Red2, Green2, Blue2, 255, SirenCoors, 0.4f, 150.0f*TheCamera.LODDistMultiplier, CCoronas::CORONATYPE_SHINYSTAR, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF, CCoronas::TRAIL_OFF, 0.0f, false, 1.5f, false, 15.0f, false, true);						
								break;
//							default:
//								CCoronas::UpdateCoronaCoors(((UInt32)this)+21+SirenNum, SirenCoors, 50.0f);
//								break;
						}
					}
				}		
			}
			break;
		case MODELID_CAR_TAXI:
			if (m_nAutomobileFlags.bTaxiLight)
			{
				// Render a little light above the taxi if the light is switched on.
				CVector offset(0.0f, -0.4f, 0.95f);
				CVector posWld = GetMatrix() * offset;
				
				CCoronas::RegisterCorona(((UInt32)this)+17, this, 100 * CTimeCycle::GetSpriteBrightness(), 100 * CTimeCycle::GetSpriteBrightness(), 0, 255, offset, 0.8f, 150.0f*TheCamera.LODDistMultiplier, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_SIMPLE, CCoronas::LOSCHECK_OFF, CCoronas::TRAIL_OFF, 0.0f, false, 1.5f, false, 15.0f, false, true);
				CPointLights::AddLight(CPointLights::PLTYPE_POINTLIGHT, posWld, CVector(0.0f, 0.0f, 0.0f), 10.0f, 0.1f, 0.1f, 0.05f, CPointLights::FOGEFF_OFF);
			}
			break;
		
		case MODELID_CAR_CABBIE:
			if (m_nAutomobileFlags.bTaxiLight)
			{
				// Render a little light above the taxi if the light is switched on.
				CVector offset(0.0f, 0.0f, 0.85f);
				CVector posWld = GetMatrix() * offset;
				
				CCoronas::RegisterCorona(((UInt32)this)+17, this, 100 * CTimeCycle::GetSpriteBrightness(), 100 * CTimeCycle::GetSpriteBrightness(), 0, 255, offset, 0.8f, 150.0f*TheCamera.LODDistMultiplier, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_SIMPLE, CCoronas::LOSCHECK_OFF, CCoronas::TRAIL_OFF, 0.0f, false, 1.5f, false, 15.0f, false, true);
				CPointLights::AddLight(CPointLights::PLTYPE_POINTLIGHT, posWld, CVector(0.0f, 0.0f, 0.0f), 10.0f, 0.1f, 0.1f, 0.05f, CPointLights::FOGEFF_OFF);
			}
			break;
			
		case MODELID_CAR_FBI:
			if (m_nVehicleFlags.bSirenOrAlarm)
			{
				CVector offset(0.0f, 1.2f, 0.5f);
				if ( (CTimer::GetTimeInMilliseconds() & 256) && (DotProduct(this->GetMatrix().GetForward(), TheCamera.GetForward()) < 0.0f))
				{
					CCoronas::RegisterCorona(((UInt32)this)+21, this, 0, 0, 255, 255, offset, 0.4f, 150.0f*TheCamera.LODDistMultiplier, CCoronas::CORONATYPE_SHINYSTAR, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF, CCoronas::TRAIL_OFF, 0.0f, false, 1.5f, false, 15.0f, false, true);
				}
			}
			
			break;
	}


	// A couple of cars just don't have lights at all
	if (GetModelIndex() != MODELID_CAR_REMOTE && GetModelIndex() != MODELID_CAR_DODO && GetModelIndex() != MODELID_CAR_RHINO && (!m_nVehicleFlags.bIsRCVehicle) && GetVehicleAppearance() != APR_HELI)
	{
		if (GetModelIndex()==MODELID_CAR_COMBINE || GetModelIndex()==MODELID_CAR_QUADBIKE)
		{
			DoVehicleLights(GetMatrix(), 0);
		}
		else
		{ 	
			DoVehicleLights(GetMatrix(), LIGHTS_DO_LHS|LIGHTS_DO_REVERSE);
		}
	}
	
	if ((this==FindPlayerVehicle())	&& (TheCamera.GetLookingForwardFirstPerson()))
	{
		// no shadows on player car if in 1st person view
	}
	else
	{
		if(this->m_nVehicleFlags.bIsRCVehicle)
		{
			if(this->GetVehicleType()==VEHICLE_TYPE_CAR)
			{
				// special case for RC tank & bandit (mission "New Model Army"):
				CShadows::StoreShadowForVehicle(this, VEHICLE_SHADOW_CAR);
			}
			else
			{
				CShadows::StoreShadowForVehicle(this, VEHICLE_SHADOW_RCBARON);
			}
		}
		else
		{
			CShadows::StoreShadowForVehicle(this, VEHICLE_SHADOW_CAR);
		}
	}

//	DoSunGlare();
	
		
//////////////////////////////////////////////////////////////////
///////Matrix stuff that was in Render()//////////////////////////
//////////////////////////////////////////////////////////////////
// All this code has been moved back from Render() to PreRender()
// so that it is executed before the Renderware Matricies are calculated
// otherwise the effects of matrix rotations won't be displayed
// until 1 frame later than requested

	CMatrix matrix;
	CVector posn;
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	CVector aWheelDirections[4];
	CVector aWheelSpeeds[4];
	CVector aWheelOffsets[4];

	// Rotate gun on top of tank
	if (GetModelIndex() == MODELID_CAR_RHINO)
		SetComponentRotation(m_aCarNodes[CAR_BONNET], ROT_AXIS_Z, GunOrientation);

	// Rotate radar dish on top of van
	// Apparently we're not using this now.
/*
	if (GetModelIndex() == MODELID_CAR_NEWSVAN && this == FindPlayerVehicle())
	{
		if(m_aCarNodes[CAR_MISC_A])
		{
			CVector	PickupCoors;
			if (CPickups::GetCoordsOfClosestCollectable1Pickup(this, 500.0f, PickupCoors))
			{		// Close to a pickup
				float ReqOrient = CMaths::ATan2( (PickupCoors.x - this->GetPosition().x),(PickupCoors.y - this->GetPosition().y) );
				float OrientDiff = ReqOrient - GunOrientation;
				while (OrientDiff > PI) OrientDiff -= 2.0f * PI;
				while (OrientDiff < -PI) OrientDiff += 2.0f * PI;
				float MaxChange = CTimer::GetTimeStep() * 0.1f;
				if (ABS(OrientDiff) < MaxChange)
				{
					GunOrientation = OrientDiff;
				}
				else if (OrientDiff > 0.0f)
				{
					GunOrientation += MaxChange;
				}
				else
				{
					GunOrientation -= MaxChange;
				}
			
			}
			else
			{		// Far from a pickup
				GunOrientation += CTimer::GetTimeStep() * 0.05f;
				if (GunOrientation > 2.0f * PI) GunOrientation -= 2.0f * PI;
			}

			SetComponentRotation(m_aCarNodes[CAR_BONNET], ROT_AXIS_Z, GunOrientation);
		}
	}
*/



	bool bIgnoreRearWheels = false;

	// MN - quick fix to get helicopters working
	if(pHandling->mFlags &MF_IS_HELI)
		bIgnoreRearWheels = true;
	// -----------------------------------------	
	
	
	CVector vecFrontForward = Multiply3x3(GetMatrix(), CVector(-CMaths::Sin(m_fSteerAngle), CMaths::Cos(m_fSteerAngle), 0.0f));
	CVector vecRearForward = GetMatrix().GetForward();
	
	if(hFlagsLocal &HF_STEER_REARWHEELS)
	{
		vecFrontForward = GetMatrix().GetForward();
		vecRearForward = Multiply3x3(GetMatrix(), CVector(CMaths::Sin(m_fSteerAngle), CMaths::Cos(m_fSteerAngle), 0.0f));
	}
	else if(hFlagsLocal &HF_HANDBRAKE_REARWHEELSTEER && m_f2ndSteerAngle!=0.0f)
	{
		vecRearForward = Multiply3x3(GetMatrix(), CVector(-CMaths::Sin(m_f2ndSteerAngle), CMaths::Cos(m_f2ndSteerAngle), 0.0f));
	}

	for(uint16 i=0;i<4;i++)
	{
		if(m_aWheelCounts[i] > 0.0f && (!bIgnoreRearWheels || i==0 || i==2))	// are wheels on ground
		{
			aWheelOffsets[i] = m_aWheelColPoints[i].GetPosition() - GetPosition();
			aWheelSpeeds[i] = GetSpeed(aWheelOffsets[i]);
			if(i==0 || i==2)
				m_aWheelAngularVelocity[i] = ProcessWheelRotation(m_aWheelState[i], vecFrontForward, aWheelSpeeds[i], pModelInfo->GetWheelScale((i==0||i==2))*0.5f);
			else
				m_aWheelAngularVelocity[i] = ProcessWheelRotation(m_aWheelState[i], vecRearForward, aWheelSpeeds[i], pModelInfo->GetWheelScale((i==0||i==2))*0.5f);

			m_aWheelPitchAngles[i] += m_aWheelAngularVelocity[i]*CTimer::GetTimeStep();
		}
	}

	int nFlags = 0;
	if(GetModelIndex()==MODELID_CAR_RHINO)
		nFlags = (WHEEL_OPTION_NOTILT|WHEEL_OPTION_NOSTEER|WHEEL_OPTION_NOHOVERTILT);

	// Set wheel positions 
	// front right
	UpdateWheelMatrix(CAR_WHEEL_RF, nFlags);
	// front left
	UpdateWheelMatrix(CAR_WHEEL_LF, nFlags);
	// rear right
	UpdateWheelMatrix(CAR_WHEEL_RR, nFlags);
	// rear left
	UpdateWheelMatrix(CAR_WHEEL_LR, nFlags);

	// middle right
	if(m_aCarNodes[CAR_WHEEL_RM])
		UpdateWheelMatrix(CAR_WHEEL_RM, nFlags);
	// middle left
	if(m_aCarNodes[CAR_WHEEL_LM])
		UpdateWheelMatrix(CAR_WHEEL_LM, nFlags);

	if(GetModelIndex()!=MODELID_CAR_RHINO)
	{
		
		ProcessSwingingDoor(CAR_DOOR_LF, FRONT_LEFT_DOOR );
		ProcessSwingingDoor(CAR_DOOR_RF, FRONT_RIGHT_DOOR );
		ProcessSwingingDoor(CAR_DOOR_LR, REAR_LEFT_DOOR );
		ProcessSwingingDoor(CAR_DOOR_RR, REAR_RIGHT_DOOR );
		ProcessSwingingDoor(CAR_BONNET, BONNET );
		ProcessSwingingDoor(CAR_BOOT, BOOT );
		
		for(int i=0; i<MAX_BOUNCING_PANELS; i++)
		if(BouncingPanels[i].GetCompIndex()>-1)
		{
			BouncingPanels[i].ProcessPanel(this, m_aCarNodes[BouncingPanels[i].GetCompIndex()], m_vecOldMoveSpeed, m_vecOldTurnSpeed);
		}
		
		uint32 nChassisComponent = CAR_CHASSIS;
		if(GetModelIndex()==MODELID_CAR_FIRETRUCK_LA)
			nChassisComponent = CAR_MISC_B;
		
		if(ChassisDoor.m_nDoorState==DT_DOOR_SWINGING_FREE && m_aCarNodes[nChassisComponent]!=NULL)
		{
			matrix.Attach(RwFrameGetMatrix(m_aCarNodes[nChassisComponent]));
			posn = matrix.GetTranslate();

			float orien[3] = {0.0f, 0.0f, 0.0f};
			CVector vecDoorOffset = Multiply3x3(GetMatrix(), posn);
			ChassisDoor.Process((CVehicle *)this, m_vecOldMoveSpeed, m_vecOldTurnSpeed, vecDoorOffset);
			orien[ChassisDoor.RetAxes()] = ChassisDoor.RetDoorAngle();

			matrix.SetRotate(orien[0], orien[1], orien[2]);
			// test let the player tilt the ladder up the way
			if(GetModelIndex()==MODELID_CAR_FIRETRUCK_LA)
				matrix.RotateX(PACKER_COL_ANGLEMULT*m_nSuspensionHydraulics);
			
			matrix.Translate(posn);
			matrix.UpdateRW();
		}
	}
	
	// now these have been used by all the swinging/bouncing stuff we can update them
	m_vecOldMoveSpeed = m_vecMoveSpeed + m_vecMoveFriction;
	m_vecOldTurnSpeed = m_vecTurnSpeed + m_vecTurnFriction;
	
	if(GetModelIndex()==MODELID_CAR_BUGGY)
	{
		float fRotSpeed = 0.0f;
		if(CMaths::Abs(GetGasPedal()) > 0.0f)
			fRotSpeed = 0.5f*CTimer::GetTimeStep();
		else
			fRotSpeed = 0.3f*CTimer::GetTimeStep();
//		if(PropRotate > TWO_PI)
//			PropRotate -= TWO_PI;
		// rotate flywheel on rear of engine
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_Y, fRotSpeed, false);
		SetComponentRotation(m_aCarNodes[CAR_MISC_E], ROT_AXIS_Y, 1.5f*fRotSpeed, false);
		// flex front suspension components
		static uint32 aBuggySuspensionComponents[3] = {CAR_MISC_B, 0, CAR_MISC_C};
		CColModel &colModel = GetColModel();
		for(int i=0; i<3; i++)
		{
			if(aBuggySuspensionComponents[i] && m_aCarNodes[aBuggySuspensionComponents[i]])
			{
				matrix.Attach(RwFrameGetMatrix(m_aCarNodes[aBuggySuspensionComponents[i]]));
				posn = matrix.GetTranslate();
				float fSuspensionWidth = pColData->m_pLineArray[i].m_vecStart.y - posn.y;
				float fSuspensionHeight = m_aWheelSuspensionHeights[i] - (pColData->m_pLineArray[i].m_vecStart.z - pHandling->fSuspensionUpperLimit);

				matrix.GetForward().z = fSuspensionHeight / fSuspensionWidth;
				matrix.UpdateRW();
			}
		}
		// rear transmission geometry tilts about centre to match up with rear wheels
		pModelInfo->GetWheelPosn(1, posn);
		SetTransmissionRotation(m_aCarNodes[CAR_MISC_D], m_aWheelSuspensionHeights[1], m_aWheelSuspensionHeights[3], posn, false);
	}
	else if(GetModelIndex()==MODELID_CAR_DOZER)
	{
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, DOZER_COL_ANGLEMULT*m_nSuspensionHydraulics);
	}
	else if(GetModelIndex()==MODELID_CAR_CEMENT)
	{
		if(GetStatus()!=STATUS_WRECKED)
		{
			// need to tilt barrel back down from horizontal
			SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, CEMENT_TILT_ANGLE);
			// then we can rotate child barrel along it's major axis
			PropRotate += (0.5f + float(m_nSuspensionHydraulics)/float(DEFAULT_COLLISION_EXTENDLIMIT))*CEMENT_ROT_SPEED_MULT*CTimer::GetTimeStep();
			SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_Y, PropRotate, true);
			SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_X, CEMENT_TILT_ANGLE, false);
		}
	}
	else if(GetModelIndex()==MODELID_CAR_PACKER)
	{
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, PACKER_COL_ANGLEMULT*m_nSuspensionHydraulics);
	}
	else if(GetModelIndex()==MODELID_CAR_TOWTRUCK)
	{
		float fRotAngle =  TOWTRUCK_ROT_ANGLE*(float)m_nSuspensionHydraulics/(float)TOWTRUCK_HOIST_DOWN_LIMIT;
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, fRotAngle);
		if(m_pVehicleBeingTowed)
		{
			fRotAngle *= -1.0f;
			CVector vecDelta, vecTemp;
			if(m_pVehicleBeingTowed->GetTowHitchPos(vecDelta, true, this)
			&& GetTowBarPos(vecTemp, true, m_pVehicleBeingTowed))
			{
				vecDelta -= vecTemp;
				float fTempDist = DotProduct(vecDelta, GetMatrix().GetForward());
				fRotAngle += CMaths::ASin(fTempDist / TOWTRUCK_WIRE_LENGTH);
				SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_X, fRotAngle, true);
				
				fTempDist = DotProduct(vecDelta, GetMatrix().GetRight());
				fRotAngle = CMaths::ASin(-fTempDist / TOWTRUCK_WIRE_LENGTH);
				SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_Y, fRotAngle, false);
			}
		}
		else
		{
			SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_X, -fRotAngle, true);
			if(BouncingPanels[0].GetCompIndex()==CAR_MISC_B)
			{
				matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_MISC_B]));
				matrix.yz += BouncingPanels[0].m_vecBounceAngle.x;
				matrix.xz += BouncingPanels[0].m_vecBounceAngle.y;
				matrix.UpdateRW();
			}
		}
	}
	else if(GetModelIndex()==MODELID_CAR_TRACTOR)
	{
		float fRotAngle =  TOWTRUCK_ROT_ANGLE*(float)m_nSuspensionHydraulics/(float)TOWTRUCK_HOIST_DOWN_LIMIT;
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, fRotAngle);
	}
	else if(GetModelIndex()==MODELID_CAR_FORKLIFT && m_aCarNodes[CAR_MISC_A])
	{
		CVehicleModelInfo *pModelInfo = (CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex());
		RwFrame *pTempFrame = CClumpModelInfo::GetFrameFromId((RpClump*)pModelInfo->GetRwObject(), CAR_MISC_A);
		CVector vecMovedPos = RwFrameGetMatrix(pTempFrame)->pos;
		vecMovedPos.z += FORKLIFT_MOVE_MULT*m_nSuspensionHydraulics;
		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_MISC_A]));
		matrix.SetTranslateOnly(vecMovedPos);
		matrix.UpdateRW();
	}
	else if(GetModelIndex()==MODELID_CAR_COMBINE)
	{
		float fRotCarbAngle = 0.0f;
		if(PropRotate > TWO_PI)	PropRotate -= TWO_PI;

		fRotCarbAngle = PropRotate = PropRotate - ( MAX(0.0f, MIN(0.1f, DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()))) )*CTimer::GetTimeStep();
		SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_X, fRotCarbAngle);	

		// now do the front bit that's attached to the two front wheels
		CVector wheelPosn;
		pModelInfo->GetWheelPosn(0, wheelPosn);
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_MISC_A]));
		posn = matrix.GetTranslate();

		float fAveWheelZ = (m_aWheelSuspensionHeights[0] + m_aWheelSuspensionHeights[2])/2.0f;
		float fDiffWheelZ = m_aWheelSuspensionHeights[0] - m_aWheelSuspensionHeights[2];

		float fAngX = -CMaths::ATan2(fAveWheelZ - wheelPosn.z, posn.y - wheelPosn.y);
		float fAngY = CMaths::ATan2(fDiffWheelZ, 2.0f*CMaths::Abs(wheelPosn.x));

		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, fAngX+PI, true);
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_Y, fAngY, false);
		
		float fChassisRollAngle = 0.0f;
		if(GetMatrix().GetUp().z > 0.0f && m_nGettingInFlags==0 && m_nGettingOutFlags==0)
		{
			static float COMBINE_CHASSIS_ROLL_LIMIT = DEGTORAD(30.0f);
			fChassisRollAngle = CMaths::ASin(GetMatrix().GetRight().z);
			if(fChassisRollAngle > COMBINE_CHASSIS_ROLL_LIMIT)
				fChassisRollAngle = COMBINE_CHASSIS_ROLL_LIMIT;
			else if(fChassisRollAngle < -COMBINE_CHASSIS_ROLL_LIMIT)
				fChassisRollAngle = -COMBINE_CHASSIS_ROLL_LIMIT;
		}
		
		static float COMBINE_CHASSIS_ROLL_SPEED = 0.95f;
		float fRate = CMaths::Pow(COMBINE_CHASSIS_ROLL_SPEED, CTimer::GetTimeStep());
		GunOrientation = fRate*GunOrientation + (1.0f - fRate)*fChassisRollAngle;
		
		SetComponentRotation(m_aCarNodes[CAR_CHASSIS], ROT_AXIS_Y, GunOrientation);
	}
	else if(GetModelIndex()==MODELID_CAR_BANDITO || GetModelIndex()==MODELID_CAR_HOTKNIFE)
	{
		static uint32 aBanditoSuspensionComponents[4] = {CAR_MISC_A, CAR_MISC_C, CAR_MISC_B, CAR_MISC_D};
		CColModel &colModel = GetColModel();
		for(int i=0; i<4; i++)
		{
			if(m_aCarNodes[aBanditoSuspensionComponents[i]])
			{
				matrix.Attach(RwFrameGetMatrix(m_aCarNodes[aBanditoSuspensionComponents[i]]));
				posn = matrix.GetTranslate();
				float fSuspensionWidth = pColData->m_pLineArray[i].m_vecStart.x - posn.x;
				float fSuspensionHeight = m_aWheelSuspensionHeights[i] - (pColData->m_pLineArray[i].m_vecStart.z - pHandling->fSuspensionUpperLimit);

				matrix.GetRight().z = fSuspensionHeight / fSuspensionWidth;
				matrix.UpdateRW();
			}
		}

		float fRotSpeed = 0.0f;
		if(CMaths::Abs(GetGasPedal()) > 0.0f)
			fRotSpeed = 0.5f*CTimer::GetTimeStep();
		else
			fRotSpeed = 0.3f*CTimer::GetTimeStep();
		// rotate flywheel on rear of engine
		SetComponentRotation(m_aCarNodes[CAR_MISC_E], ROT_AXIS_Y, fRotSpeed, false);
	}
	else if(GetModelIndex()==MODELID_CAR_RHINO || GetModelIndex()==MODELID_CAR_SWATVAN)
	{
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_Z, GunOrientation);
		SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_X, GunElevation, true);
		//SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_Z, GunOrientation, false);
	}
	else if(GetModelIndex()==MODELID_CAR_FIRETRUCK)
	{
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, GunElevation, true);
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_Z, GunOrientation, false);
	}
	else if(GetModelIndex()==MODELID_CAR_ZR350)
	{
		SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_X, PropRotate, true);
	}
//	else if(GetModelIndex()==MODELID_CAR_SWEEPER)
//	{
//		if(GetStatus()==STATUS_PLAYER || GetStatus()==STATUS_PHYSICS || GetStatus()==STATUS_SIMPLE)
//		{
//			static float SWEEPER_BRUSH_SPEED = 0.3f; 
//			SetComponentRotation(m_aCarNodes[CAR_MISC_A], ROT_AXIS_Z, SWEEPER_BRUSH_SPEED*CTimer::GetTimeStep(), false);
//			SetComponentRotation(m_aCarNodes[CAR_MISC_B], ROT_AXIS_Z, -SWEEPER_BRUSH_SPEED*CTimer::GetTimeStep(), false);
//		}
//	}

	if(GetModelIndex()==MODELID_CAR_SANDKING || (hFlagsLocal &HF_HYDRAULICS_GEOMETRY
	&& hFlagsLocal &HF_HYDRAULICS_INSTALLED && !GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK))
	{
		pModelInfo->GetWheelPosn(0, posn);
		SetTransmissionRotation(m_aCarNodes[CAR_MISC_A], m_aWheelSuspensionHeights[0], m_aWheelSuspensionHeights[2], posn, true);
		pModelInfo->GetWheelPosn(1, posn);
		SetTransmissionRotation(m_aCarNodes[CAR_MISC_B], m_aWheelSuspensionHeights[1], m_aWheelSuspensionHeights[3], posn, false);
	}
	
/*	// JIMMY BOND STUFF
	if(GetModelIndex()==MODELID_CAR_SENTINEL && GetStatus()==STATUS_PLAYER)
	{
		AnimateJimmyTransform();
	}
*/


}// end of CAutomobile::PreRender()...

//
//
//
//
void CAutomobile::Render()
{
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	DontUseSmallerRemovalRange	= CTimer::GetTimeInMilliseconds() + NEEDS_TO_BE_OFFSCREEN_TO_BE_REMOVED;

	UInt32 alphaTestRef= 1;
	RwRenderStateGet(rwRENDERSTATEALPHATESTFUNCTIONREF, &alphaTestRef);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)1);

//	pModelInfo->SetEnvironmentMap();
#ifndef MASTER	
#ifdef GTA_PS2
//	if(bCarColsEditor && IsCarColsRGBEditCar(this))// messes up the car colour editor
//		pModelInfo->SetVehicleColourOnTheFly(gCarCols_r,gCarCols_g,gCarCols_b);
//	else
#endif	
#endif	


//	this is done in CRenderer::RenderOneNonRoad():
//	this->CustomCarPlate_BeforeRenderingStart(pModelInfo);

	if( CCheat::IsCheatActive(CCheat::ONLYRENDERWHEELS_CHEAT))
	{
		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RR]));
		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LR]));

		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RF]));
		RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]));

		if(m_aCarNodes[CAR_WHEEL_RM])
			RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RM]));
		if(m_aCarNodes[CAR_WHEEL_LM])
			RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LM]));
	}
	else
	{
		CVehicle::Render();
	}
	
	if(GetModelIndex()==MODELID_CAR_RHINO)
	{
		CColModel &rhinoCol = GetColModel();
		CCollisionData* pRhinoColData = rhinoCol.GetCollisionData();
		
		ASSERT(pRhinoColData);
		
		float aLineRatios[12];
		Door[REAR_LEFT_DOOR].GetExtraWheelPositions(aLineRatios[4], aLineRatios[5], aLineRatios[6], aLineRatios[7]);
		Door[REAR_RIGHT_DOOR].GetExtraWheelPositions(aLineRatios[8], aLineRatios[9], aLineRatios[10], aLineRatios[11]);
		
		for(int nExtraWheel = 4; nExtraWheel < 12; nExtraWheel++)
		{
			float ratio = CMaths::Max(0.0f, aLineRatios[nExtraWheel] - (m_fLineLength[0] - m_fSuspensionLength[0])/m_fLineLength[0]);
			CVector vecTempWheelOffset = ratio*pRhinoColData->m_pLineArray[nExtraWheel].m_vecEnd + (1.0f - ratio)*pRhinoColData->m_pLineArray[nExtraWheel].m_vecStart;

			if(nExtraWheel < 8)
			{
				vecTempWheelOffset -= CVector(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LR])->pos);
				RwMatrixTranslate(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LR]), &vecTempWheelOffset, rwCOMBINEPOSTCONCAT);
				RwFrameUpdateObjects(m_aCarNodes[CAR_WHEEL_LR]);
				RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_LR]));
				vecTempWheelOffset *= -1.0f;
				RwMatrixTranslate(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LR]), &vecTempWheelOffset, rwCOMBINEPOSTCONCAT);
			}
			else
			{
				vecTempWheelOffset -= CVector(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RR])->pos);
				RwMatrixTranslate(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RR]), &vecTempWheelOffset, rwCOMBINEPOSTCONCAT);
				RwFrameUpdateObjects(m_aCarNodes[CAR_WHEEL_RR]);
				RpAtomicRender((RpAtomic*)GetFirstObject(m_aCarNodes[CAR_WHEEL_RR]));
				vecTempWheelOffset *= -1.0f;
				RwMatrixTranslate(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RR]), &vecTempWheelOffset, rwCOMBINEPOSTCONCAT);
			}
		}	
	}

	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)alphaTestRef);

//	this is done in CRenderer::RenderOneNonRoad():
//	this->CustomCarPlate_AfterRenderingStop(pModelInfo);

	// vehicle headlight beams
	if (GetModelIndex() != MODELID_CAR_REMOTE && GetModelIndex() != MODELID_CAR_DODO && GetModelIndex() != MODELID_CAR_RHINO && (!m_nVehicleFlags.bIsRCVehicle) && GetVehicleAppearance() != APR_HELI)
	{
		if (m_bLightOnFR)
		{
			DoHeadLightBeam(0, GetMatrix(), true);
		}
		
		if (m_bLightOnFL)
		{
			DoHeadLightBeam(0, GetMatrix(), false);
		}
	}
}



//
//
//
static RwTexture *pOldMatTexture = NULL;

//
// set custom car plate texture:	
//
void CAutomobile::CustomCarPlate_BeforeRenderingStart(CVehicleModelInfo *pModelInfo)
{
	if(pModelInfo->CustomCarPlatingAvailable())
	{
		ASSERTMSG(pOldMatTexture==NULL, "CustomCarPlate_BeforeRenderingStart() called twice in a row!");	// this should be NULL!!!

		RpMaterial *pMaterial = pModelInfo->GetCustomPlateMaterial();
		pOldMatTexture = RpMaterialGetTexture(pMaterial);
		RwTextureAddRef(pOldMatTexture);	// increase texture's reference counter (so it won't get deleted in RpMaterialSetTexture()): 
		RpMaterialSetTexture(pMaterial, this->pCustomPlateTexture);
	}
}

//
// restore old car plate texture:
//
void CAutomobile::CustomCarPlate_AfterRenderingStop(CVehicleModelInfo *pModelInfo)
{
	if(pModelInfo->CustomCarPlatingAvailable())
	{
		RpMaterial *pMaterial = pModelInfo->GetCustomPlateMaterial();
		RpMaterialSetTexture(pMaterial, pOldMatTexture);
		RwTextureDestroy(pOldMatTexture);	// decrease texture's reference counter
		pOldMatTexture = NULL;
	}
}



//
//
//
CColPoint aAutomobileColPoints[12];
//
int32 CAutomobile::ProcessEntityCollision(CEntity* pEntity, CColPoint* aColPoints)
{
	// yay the suspension lines are gonna get done for this car 
	// even if they might be 1.0 still -> but it might be in the air so thats ok.
	if(GetStatus()!=STATUS_SIMPLE)
		m_nVehicleFlags.bVehicleColProcessed = true;

//	if (GetIsTouching(pEntity))
	{
		CColModel &ColModel = GetColModel();
		CCollisionData* pColData = ColModel.GetCollisionData();
			
		int32 nNoOfLines = pColData->m_nNoOfLines;
		int32 nNoOfCollisions = 0;
		int32 nNoOfWheelCollisions = 0;
		
		float aWheelRatios[12];
		float aOldWheelRatios[12];
		for(int i=0; i<4; i++)
		{
			aWheelRatios[i] = m_aWheelRatios[i];
			aOldWheelRatios[i] = m_aWheelRatios[i];
		}
		if(GetModelIndex()==MODELID_CAR_RHINO)
		{
			Door[REAR_LEFT_DOOR].GetExtraWheelPositions(aWheelRatios[4], aWheelRatios[5], aWheelRatios[6], aWheelRatios[7]);
			Door[REAR_RIGHT_DOOR].GetExtraWheelPositions(aWheelRatios[8], aWheelRatios[9], aWheelRatios[10], aWheelRatios[11]);
			for(int e=4; e<12; e++)
				aOldWheelRatios[e] = aWheelRatios[e];
		}
		
		if(m_nPhysicalFlags.bHalfSpeedCollision || m_nPhysicalFlags.bSkipLineCol || pEntity->GetIsTypePed() ||
			(m_nModelIndex == MODELID_CAR_DODO && pEntity->GetIsTypeVehicle()) )
			pColData->m_nNoOfLines = 0;
		
		int32 nNoOfTris = -1;
		int32 nOtherNoOfTris = -1;
		if(m_pTowingVehicle==pEntity || m_pVehicleBeingTowed==pEntity)
		{
			nNoOfTris = pColData->m_nNoOfTriangles;
			pColData->m_nNoOfTriangles = 0;
			nOtherNoOfTris = pEntity->GetColModel().GetCollisionData()->m_nNoOfTriangles;
			pEntity->GetColModel().GetCollisionData()->m_nNoOfTriangles = 0;
		}
			
		DEBUGPROFILE(CProfile::ResumeProfile(PROFILE_COLLISION_TIME);)
		if (GetStatus() != STATUS_GHOST)
		{
			nNoOfCollisions = CCollision::ProcessColModels(GetMatrix(), 
					ColModel, 
					pEntity->GetMatrix(),
					pEntity->GetColModel(),
					aColPoints, 
					aAutomobileColPoints, 
					aWheelRatios);
		}
		else
		{
			nNoOfCollisions = 0;	// Don't do collisions for ghost cars.
		}
		DEBUGPROFILE(CProfile::SuspendProfile(PROFILE_COLLISION_TIME);)
		
		// restore triangles we turned off for trailers
		if(nOtherNoOfTris >= 0)
			pEntity->GetColModel().GetCollisionData()->m_nNoOfTriangles = nOtherNoOfTris;
		if(nNoOfTris >= 0)
			pColData->m_nNoOfTriangles = nNoOfTris;

		// if currently doing a 1/2 speed collision calculation
		// and no collisions were found, want to pospone suspension settings
		// to subsequent fullspeed ProcessEntityCollision step
		/*if(m_nHalfSpeedCollision || m_bSkipLineCol || (m_nModelIndex == MODELID_CAR_DODO &&
			(pEntity->GetIsTypePed() || pEntity->GetIsTypeVehicle()) ))
		{
			for (int32 i = 0; i < 4; i++)
			{
				m_aWheelRatios[i] = aOldWheelRatios[i];
			}
		}*/
		if(pColData->m_nNoOfLines == 0)
			pColData->m_nNoOfLines = nNoOfLines;
		else	
		{			
			//float lighting = 0.0f;
			//float wheelLighting;
			
			//StoredCollPolys[0].lighting = m_aWheelColPoints[0].GetLightingB();
			//StoredCollPolys[1].lighting = m_aWheelColPoints[3].GetLightingB();
			
			for (int32 i = 0; i < 4; i++)
			{
				//wheelLighting = m_lightingFromCollision;
				if ((aWheelRatios[i] < BILLS_EXTENSION_LIMIT) && (aWheelRatios[i] < m_aWheelRatios[i]))
				{
					m_aWheelRatios[i] = aWheelRatios[i];
					m_aWheelColPoints[i] = aAutomobileColPoints[i];
					
					nNoOfWheelCollisions++;
					// get lighting for this wheel
					//wheelLighting = CColTriangle::CalculateLighting(m_aWheelColPoints[i].GetLightingB());
					
					m_storedCollisionLighting[i] = m_aWheelColPoints[i].GetLightingB();
					if (pEntity->GetIsTypeVehicle() || pEntity->GetIsTypeObject())
					{
						m_aGroundPhysicalPtrs[i] = (CPhysical*) pEntity;
						REGREF(pEntity, (CEntity **)&m_aGroundPhysicalPtrs[i]);
						m_aGroundOffsets[i] = m_aWheelColPoints[i].GetPosition() - pEntity->GetPosition();
						
						if(pEntity->GetIsTypeVehicle())
							m_storedCollisionLighting[i] = ((CVehicle*) pEntity)->m_storedCollisionLighting[i];
/* bodycast has been removed in Miami
						if(pEntity->m_nModelIndex == MI_BODYCAST && GetStatus()==STATUS_PLAYER)
						{
							float fTemp = 0.0f;
							if((fTemp = m_vecMoveSpeed.MagnitudeSqr()) > 0.1f)
							{
								CObject::nBodyCastHealth -= 0.1f*m_fMass*fTemp;
//								PlayOneShotScriptObject(ASE_CARDBOARD_BOX_SMASH, GetPosition());
								DMAudio.PlayOneShot(AudioHandle, AE_PLASTER_BLOKE_CRY, 0.0f);
							}
							if(pEntity->GetIsStatic())
							{
								pEntity->SetIsStatic(false);
								((CPhysical *)pEntity)->ApplyMoveForce(m_vecMoveSpeed / CMaths::Sqrt(fTemp));
								((CPhysical *)pEntity)->m_nNoOfStaticFrames = 0;
								((CPhysical *)pEntity)->AddToMovingList();
							}
						}
*/
					}
					// For sound purposes record the material type we're standing on
					m_LastMaterialToHaveBeenStandingOn = m_aWheelColPoints[i].GetSurfaceTypeB();
					
					if (pEntity->GetIsTypeBuilding())
					{	// Use this for culling (cullzones)
						this->pEntityWeAreOnForVisibilityCheck = pEntity;
						this->m_nFlags.bTunnel = pEntity->m_nFlags.bTunnel;
						this->m_nFlags.bTunnelTransition = pEntity->m_nFlags.bTunnelTransition;
					}
				}
				
				//lighting += wheelLighting;
			}
			
			if(GetModelIndex()==MODELID_CAR_RHINO)
			{
				for(int f=4; f<12; f++)
				aWheelRatios[f] = CMaths::Min(aWheelRatios[f], aOldWheelRatios[f]);

				Door[REAR_LEFT_DOOR].SetExtraWheelPositions(aWheelRatios[4], aWheelRatios[5], aWheelRatios[6], aWheelRatios[7]);
				Door[REAR_RIGHT_DOOR].SetExtraWheelPositions(aWheelRatios[8], aWheelRatios[9], aWheelRatios[10], aWheelRatios[11]);
			}
			//m_lightingFromCollision = lighting / 4.0f;
		}// end of else
//if (this == FindPlayerVehicle())
//{
//	printf("@@@@@@@@@@ Collided with:%d (Up:%f %f %f)\n",
//		pEntity->GetModelIndex(), pEntity->GetMatrix().GetUp().x, pEntity->GetMatrix().GetUp().y, pEntity->GetMatrix().GetUp().z);
//}
		if(GetModelIndex()==MODELID_CAR_FORKLIFT && nNoOfCollisions > 0 && pEntity->GetIsTypeObject()
		&& m_nSuspensionHydraulics==0 && m_nOldSuspensionHydraulics==0)
		{
			for(int i=0; i<nNoOfCollisions; i++)
			{
				if(aColPoints[i].GetSurfaceTypeA()==SURFACE_TYPE_CAR_MOVINGCOMPONENT)
				{
					if(i < nNoOfCollisions - 1)
					{
						for(int j=i; j<nNoOfCollisions-1; j++)
							aColPoints[j] = aColPoints[j+1];
							
						// step back one so we don't miss the next collision point
						i -= 1;
					}
					// remove this collision from total because its not a physical collision
					nNoOfCollisions -= 1;
				}
			}
		}

		// record collision
		if ((nNoOfCollisions > 0) || (nNoOfWheelCollisions > 0))
		{
			AddCollisionRecord(pEntity);
			if (!pEntity->GetIsTypeBuilding())
				((CPhysical*) pEntity)->AddCollisionRecord(this);

			if(nNoOfCollisions > 0 && (pEntity->GetIsTypeBuilding() || (pEntity->GetIsTypeObject() && ((CObject *)pEntity)->m_nPhysicalFlags.bInfiniteMass) ))
				SetHasHitWall(true);
		}
//testing this (31/03/2004)
		if(nNoOfWheelCollisions > 0 && pEntity->GetIsTypeBuilding() && pHandling->fSuspensionHighSpdComDamp > 0.0f)
		{
			static float TEST_CAR_LINE_COL_MIN_CHANGE = 0.1f;
			static float TEST_CAR_LINE_COL_SUB_CHANGE = 0.0f;
			static float TEST_CAR_LINE_COL_MAX_CHANGE = 0.2f;
			static float TEST_CAR_LINE_COL_DEPTH_MULT = 200.0f;
			for (int32 i = 0; i < 4; i++)
			{
				float oldRatio = 1.0f - m_fSuspensionLength[i] / m_fLineLength[i];
				oldRatio = m_aRatioHistory[i]*(1.0f - oldRatio) + oldRatio;
				if(aOldWheelRatios[i] < oldRatio)
					oldRatio = aOldWheelRatios[i];
			
				if((oldRatio - m_aWheelRatios[i])*m_fLineLength[i] > TEST_CAR_LINE_COL_MIN_CHANGE
				&& Damage.GetWheelStatus(i)==DT_WHEEL_INTACT
				&& g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[i].GetSurfaceTypeB())!=ADHESION_GROUP_SAND
				&& m_aWheelColPoints[i].GetSurfaceTypeB()!=CAR_RAILTRACK_SURFACETYPE)
				{
					aColPoints[nNoOfCollisions] = m_aWheelColPoints[i];

					float fChange = MIN(TEST_CAR_LINE_COL_MAX_CHANGE, (oldRatio - m_aWheelRatios[i])*m_fLineLength[i] - TEST_CAR_LINE_COL_SUB_CHANGE);
					fChange *= CMaths::Abs(DotProduct(aColPoints[nNoOfCollisions].GetNormal(), GetMatrix().GetUp()))
							*  MIN(0.3f, CMaths::Abs(DotProduct(m_vecMoveSpeed, GetMatrix().GetForward())))/0.3f;
					aColPoints[nNoOfCollisions].SetDepth(fChange*pHandling->fSuspensionHighSpdComDamp);

					if(i==0)
						aColPoints[nNoOfCollisions].SetPieceTypeA(COLPOINT_PIECETYPE_FRONTLEFTWHEEL);
					else if(i==1)
						aColPoints[nNoOfCollisions].SetPieceTypeA(COLPOINT_PIECETYPE_REARLEFTWHEEL);
					else if(i==2)
						aColPoints[nNoOfCollisions].SetPieceTypeA(COLPOINT_PIECETYPE_FRONTRIGHTWHEEL);
					else
						aColPoints[nNoOfCollisions].SetPieceTypeA(COLPOINT_PIECETYPE_REARRIGHTWHEEL);
					
					// set surface types so we can identify this as purely a wheel bump collision
					aColPoints[nNoOfCollisions].SetSurfaceTypeA(SURFACE_TYPE_WHEELBASE);
					aColPoints[nNoOfCollisions].SetSurfaceTypeB(SURFACE_TYPE_WHEELBASE);
					if(nNoOfCollisions < PHYSICAL_MAXNOOFCOLLISIONPOINTS - 1)
						nNoOfCollisions++;
				}
			}
		}		
//testing this
		return nNoOfCollisions;
	}

	return 0;

}

//
//
TweakFloat CAR_STEER_SMOOTH_RATE = 0.2f;

TweakFloat CAR_MOUSE_STEER_SENS = -0.0035f;
TweakFloat CAR_MOUSE_CENTRE_RANGE = 0.7f;
TweakFloat CAR_MOUSE_CENTRE_MULT = 0.975f;
//
void CAutomobile::ProcessControlInputs(uint8 CtrlNum)
{
	float	Forwardness;

//	nPadNum = CtrlNum; // for handling multi player
	float fSpeed = DotProduct(m_vecMoveSpeed, GetMatrix().GetForward());

	ASSERT(CtrlNum <= 22);

	bool bForceBrake = false;
	if(pDriver && NULL==pDriver->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_CAR_JUMP_OUT)
	&& (pDriver->GetPedState()==PED_ARRESTED
	|| pDriver->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_CAR_SLOW_BE_DRAGGED_OUT)
	|| pDriver->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_CAR_QUICK_BE_DRAGGED_OUT)
	|| pDriver->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_CAR_WAIT_TO_SLOW_DOWN)
	|| CPad::GetPad(CtrlNum)->GetExitVehicle()))
	{
		SetIsHandbrakeOn(true);
		bForceBrake = true;
	}
	else
		SetIsHandbrakeOn(CPad::GetPad(CtrlNum)->GetHandBrake());


	//Auto E-Brake : occurs when the vehicle is moving forward at a fast enough speed based off the vehicles max velocity.
	static bool bCheckingHandBrake = false;
	static int nOldSteeringDir = 0;
	static float fTurningTime = 0.0f;
	char txt[128];

	float fSpeedPercentage = pHandling->Transmission.m_fMaxVelocity*0.5f;
	float fHighSpeedPercentage = pHandling->Transmission.m_fMaxVelocity*0.8f;
	if(!bCheckingHandBrake && fSpeed > fSpeedPercentage  && FindPlayerPed()->GetPadFromPlayer()->GetSteeringLeftRight() && !FindPlayerPed()->GetPadFromPlayer()->GetBrake()){
		if( fSpeed < fHighSpeedPercentage || ( fSpeed >= fHighSpeedPercentage && !FindPlayerPed()->GetPadFromPlayer()->GetAccelerate()) )
			bCheckingHandBrake = true;
	}

	if(bCheckingHandBrake){
		fSpeedPercentage = pHandling->Transmission.m_fMaxVelocity*0.24f;
		
		int nSteeringDir = FindPlayerPed()->GetPadFromPlayer()->GetSteeringLeftRight();
		if(fSpeed > 0.28f && nSteeringDir && (nSteeringDir > 0 && nOldSteeringDir > 0 || nSteeringDir < 0 && nOldSteeringDir < 0 || nOldSteeringDir == 0)){
			fTurningTime += CTimer::GetTimeStepInSeconds()*pHandling->fTractionLoss;	//pHandling->fTractionMultiplier;			//fSuspensionBias;
			sprintf(txt, "TurningTimer :: %f", fTurningTime);
			//CDebug::PrintToScreenCoors(txt, 25, 10);
			nOldSteeringDir = nSteeringDir;
		}else{

			//CDebug::PrintToScreenCoors("Reseting ~~~~~", 25, 50);
			fTurningTime = 0.0f;
			SetIsHandbrakeOn(false);
			bCheckingHandBrake = false;
			nOldSteeringDir = 0;

		}

		//speed is still good and we have been turning for more than enough time
		if(fTurningTime > 0.42f){
			sprintf(txt, "HandBraking turningtime::%f	tacktionloss::%f", fTurningTime, pHandling->fTractionLoss);
			//CDebug::PrintToScreenCoors(txt, 25,30);
			SetIsHandbrakeOn(true);
		}

		if(fTurningTime > 0.88f)
			SetIsHandbrakeOn(false);
	}

	
	//	bool bReversing = false;

	// progressive braking
/*
	fBrake += (((float) CPad::GetPad(CtrlNum)->GetBrake() / 255.0f) - fBrake) * 0.1f * CTimer::GetTimeStep(); // was 0.2
	fBrake = MIN(MAX(fBrake, 0.0f), 1.0f);

	// dead zone required (5%)
	if (fBrake < 0.05f)
	{
//		fBrake = 0.0f;
		fGas += (((float) CPad::GetPad(CtrlNum)->GetAccelerate() / 255.0f) - fGas) * 0.2f * CTimer::GetTimeStep(); // was 0.4
		fGas = MIN(MAX(fGas, 0.0f), 1.0f);
	}
 	else
	{
		fGas = 0.0f;
	}
*/

#ifdef GTA_PC
	// progressive steering
	if(TheCamera.m_bUseMouse3rdPerson && m_bEnableMouseSteering)
	{
		if(CPad::GetPad(CtrlNum)->GetAmountMouseMoved().x || (CMaths::Abs(fSteer) > 0.0f && m_nLastControlInput==INPUT_MOUSE && !CPad::GetPad(CtrlNum)->GetSteeringLeftRight()))
		{
			if(!CPad::GetPad(CtrlNum)->GetVehicleMouseLook())
				fSteer += CAR_MOUSE_STEER_SENS * CPad::GetPad(CtrlNum)->GetAmountMouseMoved().x;

			m_nLastControlInput = INPUT_MOUSE;

			if(CMaths::Abs(fSteer) < CAR_MOUSE_CENTRE_RANGE || CPad::GetPad(CtrlNum)->GetVehicleMouseLook())
				fSteer *= CMaths::Pow(CAR_MOUSE_CENTRE_MULT, CTimer::GetTimeStep());
		}
		else if(CPad::GetPad(CtrlNum)->GetSteeringLeftRight() || m_nLastControlInput!=INPUT_MOUSE)
		{
			fSteer += (((float) -CPad::GetPad(CtrlNum)->GetSteeringLeftRight() / 128.0f) - fSteer) * CAR_STEER_SMOOTH_RATE * CTimer::GetTimeStep();	// was 0.4f
			m_nLastControlInput = INPUT_JOYSTICK;
		}
	}
	else
#endif//GTA_PC
	{
	    fSteer += (((float) -CPad::GetPad(CtrlNum)->GetSteeringLeftRight() / 128.0f) - fSteer) * CAR_STEER_SMOOTH_RATE * CTimer::GetTimeStep();	// was 0.4f
		m_nLastControlInput = INPUT_JOYSTICK;
	}
	
	// for experimental steering, etc
	fSteer += GetNewSteeringAmt();
		

	fSteer = CMaths::Clamp(fSteer, -1.0f, 1.0f);

	// Forwardness between -1 and 1
	Forwardness = (CPad::GetPad(CtrlNum)->GetAccelerate() - CPad::GetPad(CtrlNum)->GetBrake()) / 255.0f;
	// when driving AND doing driveby at the same time do brake and accel with shoulder buttons
	if(Forwardness==0.0f && pDriver && pDriver->GetPedIntelligence()->GetTaskActiveSimplest()
	&& pDriver->GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_GANG_DRIVEBY
	&& TheCamera.Cams[0].Mode==CCam::MODE_AIMWEAPON_FROMCAR)
	{
		Forwardness = (CPad::GetPad(CtrlNum)->GetRightShoulder2() - CPad::GetPad(CtrlNum)->GetLeftShoulder2()) / 255.0f;
	}
	
	// want to limit reverse acceleration for plane
	if(m_nModelIndex == MODELID_CAR_DODO)
	{
		if(Forwardness<0.0f)
			Forwardness *= 0.3f;
	}
	
	if(!m_nVehicleFlags.bEngineOn && m_nVehicleFlags.bIsDrowning)
	{
		Forwardness = 0.0f;
	}

//	if(CPad::GetPad(CtrlNum)->GetBrake() == 0 )
//		nBrakesOn = FALSE;
	if(bForceBrake)
	{
		SetBrakePedal(1.0f);
		SetGasPedal(0.0f);
	}
	else if (ABS(fSpeed) < 0.01f)
	{		// We've stopped. Go where we want to go.
	/*	if(nBrakesOn){
			SetGasPedal(0.0f);
			SetBrakePedal(-Forwardness);
			// Leave in if a timer between brake and reverse is required
			nBrakesOn -= CTimer::GetTimeStep();
		}
		else{
	*/
		if(CPad::GetPad(CtrlNum)->GetAccelerate() > 150.0f && CPad::GetPad(CtrlNum)->GetBrake() > 150.0f)
		{
			SetGasPedal(CPad::GetPad(CtrlNum)->GetAccelerate() / 255.0f);
			SetBrakePedal(CPad::GetPad(CtrlNum)->GetBrake() / 255.0f);
			nBrakesOn = true;
		}
		else
		{
			SetGasPedal(Forwardness);
			SetBrakePedal(0.0f);
		}
	//	}
	}
	else
	{		
		//OS_ReleaseLog("Speed %f", fSpeed);
		// We're goig at a certain speed already. Either apply brake or apply gas.
		if (fSpeed >= 0.0f)
		{			// we're going forward
			if (Forwardness >= 0.0f)
			{		// go faster
				SetGasPedal(Forwardness);
				SetBrakePedal(0.0f);
			}
			else
			{		// brake
				SetGasPedal(0.0f);
				SetBrakePedal(-Forwardness);
//				nBrakesOn = 10;		// set timer period, or just any positive number if no timer
			}
		}
		else
		{			// we're going backwards
			if (Forwardness >= 0.0f)
			{
				// let us sit on the gas pedal if we're trying to get up a slope but sliding back
				if(GetGasPedal() > 0.5f && fSpeed > -0.15f)
				{
					SetGasPedal(Forwardness);
					SetBrakePedal(0.0f);
				}
				// oh dear we're sliding back too fast, go for the brakes
				else
				{
					SetGasPedal(0.0f);
					SetBrakePedal(Forwardness);
				}
			}
			else
			{		// check if brakes were on from going forwards
//				if(nBrakesOn){
//					SetGasPedal(0.0f);
//					SetBrakePedal(-Forwardness);
//				}
//				else{	// go faster
					SetGasPedal(Forwardness);
					SetBrakePedal(0.0f);
//				}
			}
		}
	}


	// Do some stuff here to stop the player from moving too far from the other player in cooperative mode.
	if (GetGasPedal() > 0.0f && this->pDriver && !CGameLogic::IsPlayerAllowedToGoInThisDirection((CPlayerPed*)this->pDriver, this->GetMatrix().GetForward()))
	{
		SetGasPedal(0.0f);
	}	
	if (GetGasPedal() < 0.0f && this->pDriver && !CGameLogic::IsPlayerAllowedToGoInThisDirection((CPlayerPed*)this->pDriver, this->GetMatrix().GetForward() * -1.0f))
	{
		SetGasPedal(0.0f);
	}	

	static float fValue; // static for speed
	fValue = 0.0f;
	// use non-linear steering - squared
	if (fSteer >= 0)
		fValue = fSteer * fSteer;
	else
		fValue = -fSteer * fSteer;

	SetSteerAngle(fValue * DEGTORAD(pHandling->fSteeringLock));

	if(pHandling->hFlags &HF_HANDBRAKE_REARWHEELSTEER)
	{
		static float HB_REARSTEER_TURN_FRAC = 0.9f;
		static float HB_REARSTEER_RETURN_FRAC = 0.9f;
		if(GetIsHandbrakeOn())
			Set2ndSteerAngle(HB_REARSTEER_TURN_FRAC*Get2ndSteerAngle() - (1.0f-HB_REARSTEER_TURN_FRAC)*GetSteerAngle());
		else
			Set2ndSteerAngle(HB_REARSTEER_RETURN_FRAC*Get2ndSteerAngle());
	}
	
	// If this car is fucked we'll use comedy controls (just buggered really)
	if (m_nVehicleFlags.bComedyControls)
	{
//		if (((CTimer::GetTimeInMilliseconds() >> 10) &15) < 12) SetGasPedal(1.0f);
/*
		if ((((CTimer::GetTimeInMilliseconds() >> 10) + 6) &15) < 12)
		{
			SetBrakePedal(0.0f);
		}
*/
//		SetIsHandbrakeOn(false);
/*
		if ((CTimer::GetTimeInMilliseconds() & 2048))
		{
			m_fSteerAngle += 0.08f;
		}
		else
		{
			m_fSteerAngle -= 0.03f;
		}
*/

		UInt16 RandNum = CGeneral::GetRandomNumber() % 10;
		
		switch (ComedyControlsState)
		{
			case 0 :
				if (RandNum < 2)
				{
					ComedyControlsState = 1;
				}
				else if (RandNum < 4)
				{
					ComedyControlsState = 2;
				}
				
				break;
				
			case 1 :
				m_fSteerAngle += 0.05f;
				
				if (RandNum < 2)
				{
					ComedyControlsState = 0;
				}
				break;
				
			case 2 :
				m_fSteerAngle -= 0.05f;
				
				if (RandNum < 2)
				{
					ComedyControlsState = 0;
				}
				break;
		}
	}
	else
	{
		ComedyControlsState = 0;
	}

	// If the player has no control (cut scene for instance) we will hit the brakes.
	if ( (CPad::GetPad(0)->DisablePlayerControls && CGameLogic::SkipState != CGameLogic::SK_FADEOUT) || (CPad::GetPad(0)->bApplyBrakes) ) //	|| Cutscene))
	{
		SetBrakePedal(1.0f);
		SetIsHandbrakeOn(true);
		SetGasPedal(0.0f);
		FindPlayerPed()->KeepAreaAroundPlayerClear();
		// Limit the velocity
		float Speed = GetMoveSpeed().Magnitude();
		#define MAXSPEEDNOW (0.28f)	// 60 km/h
		if (Speed > MAXSPEEDNOW)
		{
			SetMoveSpeed( GetMoveSpeed() * (MAXSPEEDNOW / Speed) );
		}
	}
	
	// if the engine is broken we reset te controls
	if (m_nVehicleFlags.bEngineBroken)
	{
		SetBrakePedal(0.05f);
		SetIsHandbrakeOn(false);
		SetGasPedal(0.0f);
	}
}



/* Bills old version
void CAutomobile::ProcessControlInputs(uint8 CtrlNum)
{

	nPadNum = CtrlNum; // for handling multi player
	float fSpeed = 0.0f;

	if (nPadNum > 3)
		nPadNum = 3;

	SetIsHandbrakeOn(CPad::GetPad(CtrlNum)->GetHandBrake());
	bool bReversing = false;

	// progressive braking
	fBrake += (((float) CPad::GetPad(CtrlNum)->GetBrake() / 255.0f) - fBrake) * 0.1f * CTimer::GetTimeStep(); // was 0.2
	fBrake = MIN(MAX(fBrake, 0.0f), 1.0f);

	// dead zone required (5%)
	if (fBrake < 0.05f)
	{
		 fBrake = 0.0f;
		fGas += (((float) CPad::GetPad(CtrlNum)->GetAccelerate() / 255.0f) - fGas) * 0.2f * CTimer::GetTimeStep(); // was 0.4
		fGas = MIN(MAX(fGas, 0.0f), 1.0f);
	}
	else
	{
		fGas = 0.0f;
	}

	// progressive steering
	fSteer += (((float) -CPad::GetPad(CtrlNum)->GetSteeringLeftRight() / 128.0f) - fSteer) * 0.2f * CTimer::GetTimeStep();	// was 0.4f
	fSteer = MIN(MAX(fSteer, -1.0f), 1.0f);

	if (fBrake)
	{

		fSpeed = DotProduct(m_vecMoveSpeed, GetMatrix().GetForward());

		if (fSpeed < 0.01f)
		{
			if (fBrakeCount < 0.0f)
			{
				fGas = -fBrake * 0.75f;
				bReversing = true;
			}
			else
				fBrakeCount -=  CTimer::GetTimeStep();
		}
		else
			fBrakeCount = BRAKE_TO_REVERSE_DURATION;
	}

	static float fValue; // static for speed
	// use non-linear steering - squared
	if (fSteer >= 0)
		fValue = fSteer * fSteer;
	else
		fValue = -fSteer * fSteer;

	SetSteerAngle(fValue * DEGTORAD(pHandling->fSteeringLock)); 
	SetGasPedal(fGas);
	if (bReversing)
		SetBrakePedal(0.0f);
	else
		SetBrakePedal(fBrake);

}
*/


//
//
//
Bool8 CAutomobile::GetAllWheelsOffGround(void) const
{
	if (m_nDriveWheelsOnGround==0)
	{
 		return(TRUE); 				
	}
	else
		return(FALSE);

}



#define HANDLING_EDITING_PAD	0
void CAutomobile::DebugCode()
{
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	
	if((FindPlayerVehicle() == this && !CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle)
	|| CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle == this )
	{
		int32 nHandlingIndex = pModelInfo->GetHandlingId();

		tHandlingData* pHandling = mod_HandlingManager.GetPointer(nHandlingIndex);
//		SetupModelNodes();

		// Stuff to punt data into physical as well.
		m_fMass = pHandling->fMass;
		m_fTurnMass = pHandling->fTurnMass;
		m_fBuoyancyConstant = pHandling->fBuoyancyConstant;

		if(pHandling->fDragCoeff > 0.01f)
			m_fAirResistance = 0.5f * pHandling->fDragCoeff / GAME_AIR_RESISTANCE_MASS;// * FRAMES_PER_SECOND (no need becuase speed in m/frame)
		else
			m_fAirResistance = pHandling->fDragCoeff;


#ifdef EDITABLE_HANDLING
		mod_HandlingManager.DisplayHandlingData(this, pHandling, 0, 1);
		// modify handling values
		if (CPad::GetPad(HANDLING_EDITING_PAD)->DPadRightJustDown())
			mod_HandlingManager.ModifyHandlingValue(this, this->pHandling->nVehicleID, (tField)0, true);

		if (CPad::GetPad(HANDLING_EDITING_PAD)->DPadLeftJustDown())
			mod_HandlingManager.ModifyHandlingValue(this, this->pHandling->nVehicleID, (tField)0, false);

		if (CPad::GetPad(HANDLING_EDITING_PAD)->DPadUpJustDown())
			mod_HandlingManager.PrevHandlingField();

		if (CPad::GetPad(HANDLING_EDITING_PAD)->DPadDownJustDown())
			mod_HandlingManager.NextHandlingField();
#endif // EDITABLE_HANDLING
	}
}

///////////////////////////////////////////////////////////////////////////////////
// FUNCTION : BlowUpCar
// PURPOSE :  Does everything needed to destroy a car
///////////////////////////////////////////////////////////////////////////////////

void CAutomobile::BlowUpCar( CEntity *pCulprit, Bool8 bInACutscene )
{
#ifdef GTA_NETWORK
	ASSERT(gGameNet.NetWorkStatus != NETSTAT_CLIENTRUNNING && gGameNet.NetWorkStatus != NETSTAT_CLIENTSTARTINGUP);	// Client shouldn't be blowing up cars
#endif

	if (!m_nVehicleFlags.bCanBeDamaged)
	{
		return;	//	If the flag is set for don't damage this car (usually during a cutscene)
	}


		// Only server can blow cars up.
#ifdef GTA_NETWORK
	if (gGameNet.NetWorkStatus != NETSTAT_SINGLEPLAYER && gGameNet.NetWorkStatus != NETSTAT_SERVER)
	{
		return;
	}
#endif




	if (pCulprit == FindPlayerPed() || pCulprit == FindPlayerVehicle())
	{
		CWorld::Players[CWorld::PlayerInFocus].HavocCaused += HAVOC_BLOWUPCAR;
		CWorld::Players[CWorld::PlayerInFocus].CurrentChaseValue += CHASE_BLOWUPCAR;
		CStats::IncrementStat(PROPERTY_DESTROYED, 4000 + (CGeneral::GetRandomNumber() % 6000));
	}

/*
	// MN - commented out as planes go into their own BlowUpCar() - this would never have any effect
		// If this is a flying ai plane we get it to crash and burn. Looks better.
	if (this->GetVehicleType() == VEHICLE_TYPE_PLANE && GetStatus() != STATUS_PLAYER)
	{
		switch(this->AutoPilot.Mission)
		{
			case CAutoPilot::MISSION_PLANE_FLYTOCOORS:
			case CAutoPilot::MISSION_PLANE_ATTACK_PLAYER:
			case CAutoPilot::MISSION_PLANE_DOG_FIGHT_ENTITY:
			case CAutoPilot::MISSION_PLANE_DOG_FIGHT_PLAYER:
			case CAutoPilot::MISSION_PLANE_ATTACK_PLAYER_POLICE:
			case CAutoPilot::MISSION_PLANE_FLYINDIRECTION:
			case CAutoPilot::MISSION_PLANE_FOLLOW_ENTITY:
				this->AutoPilot.Mission = CAutoPilot::MISSION_PLANE_CRASH_AND_BURN;
				m_nHealth = 0.0f;
				return;
				break;
		}
	}			
*/
		// If this is a news heli that got destroyed we expect the news people to wisen up. Only police helis from now on.
	if (GetModelIndex() == MODELID_HELI_VCNMAV)
	{
		CWanted::bUseNewsHeliInAdditionToPolice = false;
	}
	


	m_vecMoveSpeed.z += 0.13f;
	SetStatus(STATUS_WRECKED);
	m_nPhysicalFlags.bRenderScorched = TRUE;  // need to make Scorched BEFORE components blow off
	m_nTimeOfDeath = CTimer::GetTimeInMilliseconds();
	

	// do it before SpawnFlyingComponent() so this bit is propagated to all vehicle parts before they go flying:
	// let know pipeline, that we don't want any extra passes visible for this clump anymore:
	CVisibilityPlugins::SetClumpForAllAtomicsFlag((RpClump*)this->m_pRwObject, VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES);
//#if defined(GTA_PC)
//	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
//	pModelInfo->DisableEnvMap();
//#endif //GTA_PS2


	// Make all the bits fall off. (So that it looks even more wrecked)
	Damage.FuckCarCompletely();
	// this only sets status flags though, so still need to remove components
	if(m_nModelIndex != MODELID_CAR_RCTIGER && m_nModelIndex != MODELID_CAR_REMOTE)
	{
		SetBumperDamage(FRONT_BUMPER);
		SetBumperDamage(REAR_BUMPER);
		
		SetDoorDamage(BONNET);
		SetDoorDamage(BOOT);

		SetDoorDamage(FRONT_LEFT_DOOR);
		SetDoorDamage(FRONT_RIGHT_DOOR);
		SetDoorDamage(REAR_LEFT_DOOR);
		SetDoorDamage(REAR_RIGHT_DOOR);
		
		SpawnFlyingComponent(CAR_WHEEL_LF, CG_WHEEL);

		RpAtomic *pCurrentAtomic = NULL;
		RpAtomic **ppAtomicData = &pCurrentAtomic;  // extra step for clarity
		// this callback should now find the diplayed atomic and return a pointer in pCurrentAtomic
		RwFrameForAllObjects(m_aCarNodes[CAR_WHEEL_LF], &GetCurrentAtomicObjectCB, (void *)ppAtomicData);
		if(pCurrentAtomic)
			RpAtomicSetFlags(pCurrentAtomic, 0);

//		SetComponentVisibility(m_aCarNodes[CAR_WHEEL_LF], VEHICLE_ATOMIC_NONE);
	}
	
	m_nHealth = 0;	// Make sure this happens before AddExplosion or it will blow up twice
	DelayedExplosion = 0;	// Cancel any pending explosions.
	BombOnBoard = 0;		// Make sure player can't blow up wrecks.

	TheCamera.CamShake(0.4f, GetPosition().x, GetPosition().y, GetPosition().z);
	
	CVector	Temp = GetPosition();

	KillPedsInVehicle();
	KillPedsGettingInVehicle();
	
	// Switch off the engine. (For sound purposes)
	this->m_nVehicleFlags.bEngineOn = FALSE;
	this->OverrideLights = NO_CAR_LIGHT_OVERRIDE;
	this->m_nVehicleFlags.bLightsOn = FALSE;
	this->m_nVehicleFlags.bSirenOrAlarm = FALSE;
	this->m_nAutomobileFlags.bTaxiLight = FALSE;

	if (m_nVehicleFlags.bIsAmbulanceOnDuty)
	{
		m_nVehicleFlags.bIsAmbulanceOnDuty = false;
		CCarCtrl::NumAmbulancesOnDuty--;
	}
	if (m_nVehicleFlags.bIsFireTruckOnDuty)
	{
		m_nVehicleFlags.bIsFireTruckOnDuty = false;
		CCarCtrl::NumFireTrucksOnDuty--;
	}
	
	ChangeLawEnforcerState(FALSE);
	
	gFireManager.StartFire((CEntity *)this, pCulprit, DEFAULT_FIRE_PARTICLE_SIZE, true, FIRE_AVERAGE_BURNTIME, 0);	// Make the car burn for a while
	
	CDarkel::RegisterCarBlownUpByPlayer(this);	// We assume every car exploding has been caused by player

	CVector vecExplosionPos = GetPosition();
	float fOffsetRange = 0.1f;
	if(GetVehicleType()==VEHICLE_TYPE_CAR || GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
		fOffsetRange = 0.75f;
	
	vecExplosionPos += CGeneral::GetRandomNumberInRange(-fOffsetRange, fOffsetRange)*GetColModel().GetBoundBoxMax().x*GetMatrix().GetRight();
	vecExplosionPos += CGeneral::GetRandomNumberInRange(-fOffsetRange, fOffsetRange)*GetColModel().GetBoundBoxMax().y*GetMatrix().GetForward();
	vecExplosionPos.z -= CGeneral::GetRandomNumberInRange(0.5f, 1.5f)*GetColModel().GetBoundBoxMax().z;
	
	// Car explodes. (BOOM)
	if (m_nModelIndex == MODELID_CAR_RCTIGER || m_nModelIndex == MODELID_CAR_REMOTE)
	{
		CExplosion::AddExplosion(this, pCulprit, EXP_TYPE_CAR_QUICK, vecExplosionPos, 0, true, -1, bInACutscene);
	}
	else
	{
		CExplosion::AddExplosion(this, pCulprit, EXP_TYPE_CAR, vecExplosionPos, 0, true, -1, bInACutscene);
	}

		// If we are the server we have to make sure this update gets used by the other machines
#ifdef GTA_NETWORK
	if (gGameNet.NetWorkStatus == NETSTAT_SERVER)
	{
		this->ForceUpdateToClient();
		if (pDriver) pDriver->ForceUpdateToClient();
		for (Int32 Pass = 0; Pass < m_nMaxPassengers; Pass++) if (pPassengers[Pass]) pPassengers[Pass]->ForceUpdateToClient();
	}
#endif

}

///////////////////////////////////////////////////////////////////////////////////
// FUNCTION	: BlowUpCarCutSceneNoExtras()
// PURPOSE	: Modified version of Function above (BlowUpCar(...))
//			: Seems to be for a very specialised script command 
//			: EXPLODE_CAR_FOR_CUTSCENE_NO_SHAKE_OR_BITS
//			: so have given it a function of its own. (easy to remove if script no longer uses it)	
///////////////////////////////////////////////////////////////////////////////////

void CAutomobile::BlowUpCarCutSceneNoExtras(bool bDontShakeCam, bool bDontSpawnStuff, bool bNoExplosion, bool bMakeSound/*=true*/)
{
#ifdef GTA_NETWORK
	ASSERT(gGameNet.NetWorkStatus != NETSTAT_CLIENTRUNNING && gGameNet.NetWorkStatus != NETSTAT_CLIENTSTARTINGUP);	// Client shouldn't be blowing up cars
#endif

	if (!m_nVehicleFlags.bCanBeDamaged)
	{
		return;	//	If the flag is set for don't damage this car (usually during a cutscene)
	}


		// Only server can blow cars up.
#ifdef GTA_NETWORK
	if (gGameNet.NetWorkStatus != NETSTAT_SINGLEPLAYER && gGameNet.NetWorkStatus != NETSTAT_SERVER)
	{
		return;
	}
#endif

	// If this is a flying ai plane we get it to crash and burn. Looks better.
	if (this->GetVehicleType() == VEHICLE_TYPE_PLANE && GetStatus() != STATUS_PLAYER)
	{
		switch(this->AutoPilot.Mission)
		{
			case CAutoPilot::MISSION_PLANE_FLYTOCOORS:
			case CAutoPilot::MISSION_PLANE_ATTACK_PLAYER:
			case CAutoPilot::MISSION_PLANE_DOG_FIGHT_ENTITY:
			case CAutoPilot::MISSION_PLANE_DOG_FIGHT_PLAYER:
			case CAutoPilot::MISSION_PLANE_ATTACK_PLAYER_POLICE:
			case CAutoPilot::MISSION_PLANE_FLYINDIRECTION:
			case CAutoPilot::MISSION_PLANE_FOLLOW_ENTITY:
				this->AutoPilot.Mission = CAutoPilot::MISSION_PLANE_CRASH_AND_BURN;
				return;
				break;
		}
	}			

		// If this is a news heli that got destroyed we expect the news people to wisen up. Only police helis from now on.
	if (GetModelIndex() == MODELID_HELI_VCNMAV)
	{
		CWanted::bUseNewsHeliInAdditionToPolice = false;
	}

	if (!bNoExplosion)
		m_vecMoveSpeed.z += 0.13f;

		
	SetStatus(STATUS_WRECKED);
	m_nPhysicalFlags.bRenderScorched = TRUE;  // need to make Scorched BEFORE components blow off
	m_nTimeOfDeath = CTimer::GetTimeInMilliseconds();

	// do it before SpawnFlyingComponent() so this bit is propagated to all vehicle parts before they go flying:
	// let know pipeline, that we don't want any extra passes visible for this clump anymore:
	CVisibilityPlugins::SetClumpForAllAtomicsFlag((RpClump*)this->m_pRwObject, VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES);

	
	// Make all the bits fall off. (So that it looks even more wrecked)
	if (bNoExplosion)
		Damage.FuckCarCompletely(true); // keep wheels
	else
		Damage.FuckCarCompletely(false); // remove only 1 wheel
		
	// this only sets status flags though, so still need to remove components
	if(m_nModelIndex != MODELID_CAR_RCBANDIT)
	{
		SetBumperDamage(FRONT_BUMPER, bDontSpawnStuff);
		SetBumperDamage(REAR_BUMPER, bDontSpawnStuff);
		
		SetDoorDamage(BONNET, bDontSpawnStuff);
		SetDoorDamage(BOOT, bDontSpawnStuff);

		SetDoorDamage(FRONT_LEFT_DOOR, bDontSpawnStuff);
		SetDoorDamage(FRONT_RIGHT_DOOR, bDontSpawnStuff);
		SetDoorDamage(REAR_LEFT_DOOR, bDontSpawnStuff);
		SetDoorDamage(REAR_RIGHT_DOOR, bDontSpawnStuff);
		
		if(bDontSpawnStuff== false)
		{
			SpawnFlyingComponent(CAR_WHEEL_LF, CG_WHEEL);
		}
		
		RpAtomic *pCurrentAtomic = NULL;
		RpAtomic **ppAtomicData = &pCurrentAtomic;  // extra step for clarity
		
		if (!bNoExplosion)
		{
			// this callback should now find the diplayed atomic and return a pointer in pCurrentAtomic
			RwFrameForAllObjects(m_aCarNodes[CAR_WHEEL_LF], &GetCurrentAtomicObjectCB, (void *)ppAtomicData);
			if(pCurrentAtomic)
				RpAtomicSetFlags(pCurrentAtomic, 0);	
		}

//		SetComponentVisibility(m_aCarNodes[CAR_WHEEL_LF], VEHICLE_ATOMIC_NONE);
	}
	
	m_nHealth = 0;	// Make sure this happens before AddExplosion or it will blow up twice
	DelayedExplosion = 0;	// Cancel any pending explosions.
	BombOnBoard = 0;		// Make sure player can't blow up wrecks.

	if(bDontShakeCam==false)
	{
		TheCamera.CamShake(0.4f, GetPosition().x, GetPosition().y, GetPosition().z);
	}
	
	CVector	Temp = GetPosition();

	KillPedsInVehicle();
	
	// Switch off the engine. (For sound purposes)
	this->m_nVehicleFlags.bEngineOn = FALSE;
	this->OverrideLights = NO_CAR_LIGHT_OVERRIDE;
	this->m_nVehicleFlags.bLightsOn = FALSE;
	this->m_nVehicleFlags.bSirenOrAlarm = FALSE;
	this->m_nAutomobileFlags.bTaxiLight = FALSE;

	if (m_nVehicleFlags.bIsAmbulanceOnDuty)
	{
		m_nVehicleFlags.bIsAmbulanceOnDuty = false;
		CCarCtrl::NumAmbulancesOnDuty--;
	}
	if (m_nVehicleFlags.bIsFireTruckOnDuty)
	{
		m_nVehicleFlags.bIsFireTruckOnDuty = false;
		CCarCtrl::NumFireTrucksOnDuty--;
	}
	
	ChangeLawEnforcerState(FALSE);
	
	// Car explodes. (BOOM)
	// $Shaun Using Molotov doesnt shake the camera etc.
	if (!bNoExplosion)
	{
		// no explosion is used for gang roadblocks, so line below is moved in here
		CDarkel::RegisterCarBlownUpByPlayer(this);	// We assume every car exploding has been caused by player
	
		gFireManager.StartFire((CEntity *)this, NULL, DEFAULT_FIRE_PARTICLE_SIZE, true, FIRE_AVERAGE_BURNTIME, 0);	// Make the car burn for a while
		CExplosion::AddExplosion(this, NULL, EXP_TYPE_MOLOTOV, this->GetPosition(), 0, bMakeSound);
	}
	
	// If we are the server we have to make sure this update gets used by the other machines
#ifdef GTA_NETWORK
	if (gGameNet.NetWorkStatus == NETSTAT_SERVER)
	{
		this->ForceUpdateToClient();
		if (pDriver) pDriver->ForceUpdateToClient();
		for (Int32 Pass = 0; Pass < m_nMaxPassengers; Pass++) if (pPassengers[Pass]) pPassengers[Pass]->ForceUpdateToClient();
	}
#endif

	
}



///////////////////////////////////////////////////////////////////////////////////
// FUNCTION : RemoveRefsToVehicle
// PURPOSE :  If this car has references to a certain entity they will get removed here.
///////////////////////////////////////////////////////////////////////////////////

void CAutomobile::RemoveRefsToVehicle(CEntity *pEntity)
{

	for (int32 i = 0; i < 4; i++)
	{
		if (m_aGroundPhysicalPtrs[i] == (CPhysical *)pEntity)
		{
			m_aGroundPhysicalPtrs[i] = NULL;
		}
	}
}


bool CAutomobile::SetUpWheelColModel(CColModel *pColModel)
{
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	CColModel& colModel = GetColModel();
	CCollisionData* pColData = colModel.GetCollisionData();
	CCollisionData* pColData2 = pColModel->GetCollisionData();
	
	// don't let tyres burst on certain vehicles
	if(GetModelIndex()==MODELID_CAR_TRACTOR	|| GetModelIndex()==MODELID_CAR_COMBINE
	|| GetModelIndex()==MODELID_CAR_KART)
		return false;

	pColModel->GetBoundSphere() = colModel.GetBoundSphere();
	pColModel->GetBoundBox() = colModel.GetBoundBox();

	CMatrix matrix;
	// front left wheel
	matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
	pColData2->m_pSphereArray[0].Set(0.5f*pModelInfo->GetWheelScale(true), matrix.GetTranslate(), SURFACE_TYPE_RUBBER, COLPOINT_PIECETYPE_FRONTLEFTWHEEL);
	// rear left wheel
	matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LR]));
	pColData2->m_pSphereArray[1].Set(0.5f*pModelInfo->GetWheelScale(false), matrix.GetTranslate(), SURFACE_TYPE_RUBBER, COLPOINT_PIECETYPE_REARLEFTWHEEL);

	// front right wheel
	matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
	pColData2->m_pSphereArray[2].Set(0.5f*pModelInfo->GetWheelScale(true), matrix.GetTranslate(), SURFACE_TYPE_RUBBER, COLPOINT_PIECETYPE_FRONTRIGHTWHEEL);
	// rear right wheel
	matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RR]));
	pColData2->m_pSphereArray[3].Set(0.5f*pModelInfo->GetWheelScale(false), matrix.GetTranslate(), SURFACE_TYPE_RUBBER, COLPOINT_PIECETYPE_REARRIGHTWHEEL);
	
	// middle wheels
	if(m_aCarNodes[CAR_WHEEL_LM] && m_aCarNodes[CAR_WHEEL_RM]){
		// left middle wheel
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LM]));
		pColData2->m_pSphereArray[4].Set(0.5f*pModelInfo->GetWheelScale(false), matrix.GetTranslate(), SURFACE_TYPE_RUBBER, COLPOINT_PIECETYPE_REARLEFTWHEEL);
		// right middle wheel
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RM]));
		pColData2->m_pSphereArray[5].Set(0.5f*pModelInfo->GetWheelScale(false), matrix.GetTranslate(), SURFACE_TYPE_RUBBER, COLPOINT_PIECETYPE_REARRIGHTWHEEL);
		// this vehicle has six wheels to test
		pColData2->m_nNoOfSpheres = 6;
	}
	else	// must only have 4 wheels I guess
		pColData2->m_nNoOfSpheres = 4;

	return true;
}


float fBurstForceMult = 0.03f;

bool CAutomobile::BurstTyre(uint8 nWheelPieceType, bool bApplyForce)
{
	// can't burst tyres on a tank!
	if(GetModelIndex()==MODELID_CAR_RHINO)
		return false;
		
	if (m_nVehicleFlags.bTyresDontBurst || m_nPhysicalFlags.bRenderScorched)
		return false;

	// unfortunately the PieceTypes are ordered differently to the car no.s
	switch(nWheelPieceType)
	{
		case COLPOINT_PIECETYPE_FRONTLEFTWHEEL:
			nWheelPieceType = 0;
			break;
		case COLPOINT_PIECETYPE_REARLEFTWHEEL:
			nWheelPieceType = 1;
			break;
		case COLPOINT_PIECETYPE_FRONTRIGHTWHEEL:
			nWheelPieceType = 2;
			break;
		case COLPOINT_PIECETYPE_REARRIGHTWHEEL:
			nWheelPieceType = 3;
			break;
	}
	// check the wheel's there to burst before bursting it.
	if(Damage.GetWheelStatus(nWheelPieceType) == DT_WHEEL_INTACT)
	{
		Damage.SetWheelStatus(nWheelPieceType, DT_WHEEL_BURST);

		// check whether the tyre has been burst (gunfire) or slashed with sharp object:
/*		if ((FindPlayerPed()->CPed::GetWeapon()->GetWeaponType() >= WEAPONTYPE_BRASSKNUCKLE) && (FindPlayerPed()->CPed::GetWeapon()->GetWeaponType() <= WEAPONTYPE_CHAINSAW))
		{
			CStats::TyresSlashed++; // increment this in the stats
		}
		else*/
		{
			CStats::IncrementStat(TYRES_POPPED, 1);
		}
		
#ifdef USE_AUDIOENGINE
		m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_TYRE_BURST);
#else //USE_AUDIOENGINE
		DMAudio.PlayOneShot(AudioHandle, AE_CAR_WHEEL_BURST, 0.0f);	// boom
#endif //USE_AUDIOENGINE
		
		if(GetStatus() == STATUS_SIMPLE){
			CCarCtrl::SwitchVehicleToRealPhysics(this);
		}
		if (bApplyForce)
		{
			ApplyMoveForce(CGeneral::GetRandomNumberInRange(-fBurstForceMult,fBurstForceMult)*m_fMass*GetMatrix().GetRight());
			ApplyTurnForce(CGeneral::GetRandomNumberInRange(-fBurstForceMult,fBurstForceMult)*m_fTurnMass*GetMatrix().GetRight(), GetMatrix().GetForward());
			m_nNoOfStaticFrames = 0;
		}

		return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////////
// FUNCTION : Fix
// PURPOSE :  Revert all damage back to OK 
///////////////////////////////////////////////////////////////////////////////////
void CAutomobile::Fix()
{
	Damage.ResetDamageStatus();
	
	// if the vehicle didn't have any doors to start with
	//(BF Injection) then remove the ghost ones it's just been given
	if (pHandling->mFlags &MF_NO_DOORS)
	{
		Damage.SetDoorStatus(FRONT_LEFT_DOOR, DT_DOOR_MISSING);
		Damage.SetDoorStatus(FRONT_RIGHT_DOOR, DT_DOOR_MISSING);
		Damage.SetDoorStatus(REAR_LEFT_DOOR, DT_DOOR_MISSING);
		Damage.SetDoorStatus(REAR_RIGHT_DOOR, DT_DOOR_MISSING);
	}

	m_nVehicleFlags.bIsDamaged = false;
/*	if(m_aCarNodes[CAR_ALLCOMPS])
	{
		ShowAllComps();
	}
	else*/
	{
		// hide all damaged atomics and display all other atomics
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::HideAllComponentsAtomicCB, (void*)VEHICLE_ATOMIC_DAMAGED);
	}
	
	for(int32 i=CAR_DOOR_RF; i<MAX_CAR_NODES; i++)
	{
		// If component exists
		if(m_aCarNodes[i])
		{
			// Set OK atomics to be visible
			//RwFrameForAllObjects(m_aCarNodes[i], &SetVehicleAtomicVisibilityCB, (void*)VEHICLE_ATOMIC_OK);

			// reset door rotation
			CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[i]));
			matrix.SetTranslate(matrix.GetTranslate());		
			matrix.UpdateRW();
		}
	}
	
	// Fix the tyres that may have been burst
	Damage.SetWheelStatus(FRONT_LEFT_WHEEL, DT_WHEEL_INTACT);
	Damage.SetWheelStatus(REAR_LEFT_WHEEL, DT_WHEEL_INTACT);
	Damage.SetWheelStatus(FRONT_RIGHT_WHEEL, DT_WHEEL_INTACT);
	Damage.SetWheelStatus(REAR_RIGHT_WHEEL, DT_WHEEL_INTACT);

	// remove bouncing panels
	for(int i=0; i<=MAX_BOUNCING_PANELS; i++)
		BouncingPanels[i].m_nComponentIndex = -1;
}

void CAutomobile::FixTyre(eWheels tyre)
{
	Damage.SetWheelStatus(tyre, DT_WHEEL_INTACT);	
}

void CAutomobile::FixDoor(int32 index, eDoors iDoor)
{
	
	// if the car has no doors, no need to fix them.
	if (pHandling->mFlags &MF_NO_DOORS) return;
	
	
	Door[iDoor].Open(0.0f);
	Damage.SetDoorStatus(iDoor, DT_DOOR_INTACT);
	
	// If component exists
	if(m_aCarNodes[index])
	{
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_OK);
		
		// reset door rotation
		CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
		matrix.SetTranslate(matrix.GetTranslate());		
		matrix.UpdateRW();
	}
}



void CAutomobile::PopDoor(int32 index, eDoors iDoor, bool bSpawnStuff)
{
	uint8 Status = Damage.GetDoorStatus(iDoor);
	
	if(Status != DT_DOOR_MISSING)
	{
		if(bSpawnStuff) 
		{
			switch(index)
			{
				case BOOT:
					SpawnFlyingComponent(index, CG_BOOT);
					break;
				case BONNET:
#ifdef USE_AUDIOENGINE
					m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_BONNET_FLUBBER_FLUBBER, (CEntity*)SpawnFlyingComponent(index, CG_BONNET));
#endif //USE_AUDIOENGINE
					break;
				default:
					SpawnFlyingComponent(index, CG_DOOR);
					break;	
			}
			
		}
		
		Damage.SetDoorStatus(iDoor, DT_DOOR_MISSING);
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_NONE);
	}	
	
}

void CAutomobile::FixPanel(int32 index, ePanels iPanel)
{
	
	Damage.SetPanelStatus(iPanel, DT_PANEL_INTACT);
	
	// remove bouncing panels
	for(int i=0; i<=MAX_BOUNCING_PANELS; i++)
		if(BouncingPanels[i].m_nComponentIndex == index)
			BouncingPanels[i].m_nComponentIndex = -1;

	// If component exists
	if(m_aCarNodes[index])
	{
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_OK);
		
		// reset panel rotation
		CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[index]));
		matrix.SetTranslate(matrix.GetTranslate());		
		matrix.UpdateRW();
	}
}



void CAutomobile::PopPanel(int32 index, ePanels iPanel, bool bSpawnStuff)
{
	uint8 Status = Damage.GetPanelStatus(iPanel);
	
	ASSERTMSG(m_aCarNodes[index] != NULL, "Cannot Pop panel that does not exist");
	if(Status != DT_PANEL_MISSING)
	{
		if(bSpawnStuff) 
		{
			switch(index)
			{
				case FRONT_BUMPER:
				case REAR_BUMPER: 
					SpawnFlyingComponent(index, CG_BUMPER);
					break;
				default:
					SpawnFlyingComponent(index, CG_PANEL);	
					break;
			}
		}
		
		Damage.SetPanelStatus(iPanel, DT_PANEL_MISSING);
		SetComponentVisibility(m_aCarNodes[index], VEHICLE_ATOMIC_NONE);
	}	
	
}
		
///////////////////////////////////////////////////////////////////////////////////
// FUNCTION : Fix
// PURPOSE :  Revert all damage back to OK 
///////////////////////////////////////////////////////////////////////////////////
void CAutomobile::SetupDamageAfterLoad()
{
	if(m_aCarNodes[CAR_BUMPFRONT])
		SetBumperDamage(FRONT_BUMPER);
	if(m_aCarNodes[CAR_BONNET])
		SetDoorDamage(BONNET);
	if(m_aCarNodes[CAR_BUMPREAR])
		SetBumperDamage(REAR_BUMPER);
	if(m_aCarNodes[CAR_BOOT])
		SetDoorDamage(BOOT);
	if(m_aCarNodes[CAR_DOOR_LF])
		SetDoorDamage(FRONT_LEFT_DOOR);
	if(m_aCarNodes[CAR_DOOR_RF])
		SetDoorDamage(FRONT_RIGHT_DOOR);
	if(m_aCarNodes[CAR_DOOR_LR])
		SetDoorDamage(REAR_LEFT_DOOR);
	if(m_aCarNodes[CAR_DOOR_RR])
		SetDoorDamage(REAR_RIGHT_DOOR);
	if(m_aCarNodes[CAR_WING_LF])
		SetPanelDamage(FRONT_LEFT_PANEL);
	if(m_aCarNodes[CAR_WING_RF])
		SetPanelDamage(FRONT_RIGHT_PANEL);
/* these components are no longer used
	if(m_aCarNodes[CAR_WING_LR])
		SetPanelDamage(REAR_LEFT_PANEL);
	if(m_aCarNodes[CAR_WING_RR])
		SetPanelDamage(REAR_RIGHT_PANEL);
*/
}

///////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetTaxiLight
// PURPOSE :  Switched taxilight on or off 
///////////////////////////////////////////////////////////////////////////////////
void CAutomobile::SetTaxiLight(bool State)
{
	m_nAutomobileFlags.bTaxiLight = State;
}

///////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetAllTaxiLights
// PURPOSE :  If m_sAllTaxiLights is TRUE then all new taxis will have their 
//				lights switched on
///////////////////////////////////////////////////////////////////////////////////
void CAutomobile::SetAllTaxiLights(bool State)
{
	m_sAllTaxiLights = State;
}



/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : PlayCarHorn
// PURPOSE :  Play the car horn if not already triggered.  Also shouts abuse
/////////////////////////////////////////////////////////////////////////////////

#define MIN_SPEAK_TIME			1500
#define SPEAK_TIME_OFFSET		3000

#define MIN_NO_HORN_TIME		150

void CAutomobile::PlayCarHorn(void)
{
	UInt32 r;

	if(!IsAlarmActivated())
	{
		if(m_cHorn == 0) 
		{
			if(m_NoHornCount > 0)
			{
				m_NoHornCount--;
				return;
			}

			m_NoHornCount = MIN_NO_HORN_TIME + (CGeneral::GetRandomNumber() & 0x7f);	
			
	//		r = CGeneral::GetRandomNumber() & 0xf;	// 
			
			r = m_NoHornCount & 0x7;


			if(r < 2)
			{
				// Horn only
				
				m_cHorn = HORN_TIME;
			}
			else if(r < 4)
			{
				// Horn and Abuse
				
				if(pDriver != NULL && AutoPilot.SlowingDownForCar)
				{
#ifdef USE_AUDIOENGINE
					pDriver->Say(CONTEXT_GLOBAL_BLOCKED/*AE_DRIVER_SHOUT_ABUSE*/);
#else //USE_AUDIOENGINE
					pDriver->Say(AE_DRIVER_SHOUT_ABUSE);
#endif //USE_AUDIOENGINE
				}
				m_cHorn = HORN_TIME;
			}
			else
			{
				// Abuse only 
				
				if(pDriver != NULL)
				{
#ifdef USE_AUDIOENGINE
					pDriver->Say(CONTEXT_GLOBAL_BLOCKED/*AE_DRIVER_SHOUT_ABUSE*/);
#else //USE_AUDIOENGINE
					pDriver->Say(AE_DRIVER_SHOUT_ABUSE);
#endif //USE_AUDIOENGINE
				}
			}			
		}

/*		switch(CGeneral::GetRandomNumber() & 0x7)
		{
			case 1:
				m_cHorn = HORN_TIME;		
				break;
	
			case 2:
				if(pDriver != NULL)
				{
					pDriver->Say(AE_DRIVER_SHOUT_ABUSE);
				}
				break;
					
			case 3:
				m_cHorn = HORN_TIME;		

				if(ShoutsInsult())
				{
					if(pDriver != NULL)
					{
						pDriver->Say(AE_DRIVER_SHOUT_ABUSE);
					}
				}
				break;
								
			default: // if 0 ignore
				break;
		}
*/
	}
}

/*
Bool8 CAutomobile::ShoutsInsult()
{
	// Not all cars allow drivers to shout insults - police, gang, etc

	switch(GetModelIndex())
	{
		case MODELID_CAR_POLICE:
		case MODELID_CAR_ENFORCER:
		case MODELID_CAR_BOAT_PREDATOR:
		case MODELID_CAR_FIRETRUCK:
		case MODELID_CAR_AMBULANCE:
		case MODELID_CAR_FBI:
		case MODELID_CAR_BARRACKS:
		case MODELID_CAR_RHINO:
		case MODELID_CAR_RCBANDIT:
		case MODELID_CAR_BELLYUP:
		case MODELID_CAR_MRWONGS:
		case MODELID_CAR_MAFIA:
		case MODELID_CAR_YARDIE:
		case MODELID_CAR_YAKUZA:
		case MODELID_CAR_DIABLOS:
		case MODELID_CAR_COLUMBIANS:
		case MODELID_CAR_HOODS:
			return(FALSE);
		default:
			return(TRUE);
	}
}
*/


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : PlayHornIfNecessary
// PURPOSE :  Checks to see if reason to play car horn / shout abuse
/////////////////////////////////////////////////////////////////////////////////

void CAutomobile::PlayHornIfNecessary(void)
{
	// Only do for simple and physics cars - not player, abandoned etc

	if((AutoPilot.SlowingDownForPed || AutoPilot.SlowingDownForCar) && (!HasCarStoppedBecauseOfLight()))
	{
		PlayCarHorn();
	}
}

// Name			:	ScanForCrimes
// Purpose		:	Checks area surrounding car for crimes being commited. These are reported immediately
// Parameters	:	None
// Returns		:	Nothing

#define COP_CAR_CRIME_DETECTION_RANGE (20.0f)

void CAutomobile::ScanForCrimes()
{
//	float fDistance;
	CVector vecEvent;
//	int32 i;
//	eEventType EventType = EVENT_NULL;
//	eCrimeType	CrimeWitnessed;

	
	// If the player drives a cop with car alarm activated we set the wanted status
	// to a set minimum in one go.
	if (FindPlayerVehicle() && FindPlayerVehicle()->GetBaseVehicleType() == VEHICLE_TYPE_CAR)
	{
		if ( ((CAutomobile *)FindPlayerVehicle())->IsAlarmActivated() )
		{
			// Test whether we are close enough to notice player
			CVector	Dist;
			Dist = FindPlayerVehicle()->GetPosition() - this->GetPosition();
			if (Dist.MagnitudeSqr() < COP_CAR_CRIME_DETECTION_RANGE*COP_CAR_CRIME_DETECTION_RANGE)
			{
				CWorld::Players[CWorld::PlayerInFocus].pPed->SetWantedLevelNoDrop(WANTED_LEVEL1);
			}
		}
	}
}


// Name			:	CanPedLeaveCar
// Purpose		:	Returns true if a ped is allowed to leave this car through a specific door.
//					Things that could prevent this are other cars and buildings.
//					A line check is performed as well as a volume type check.
// Parameters	:	None
// Returns		:	Nothing

bool CAutomobile::IsRoomForPedToLeaveCar(uint32 nDoor, CVector *pvecCarJackOffset)
{
/*
	CVector vecLocalSeatPosition, Seat, ExitPoint, AnimExitPoint, ExtendedExitPoint, Diff, vecAnimPos;
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CColPoint TestColPoint;
	CEntity* TestEntity = NULL;
	float	Length, ZCeiling, ZFloor;
	bool	bReturnValue;

	TestColPoint.SetPosition(0.0f,0.0f,0.0f);

	switch(nDoor)
	{
		case CAR_DOOR_LF:
		 	vecLocalSeatPosition = pModelInfo->GetFrontSeatPosn();
			vecLocalSeatPosition.x = -vecLocalSeatPosition.x;
			break;	
		case CAR_DOOR_RF:
		 	vecLocalSeatPosition = pModelInfo->GetFrontSeatPosn();
			break;	
		case CAR_DOOR_LR:
		 	vecLocalSeatPosition = pModelInfo->GetBackSeatPosn();
			vecLocalSeatPosition.x = -vecLocalSeatPosition.x;
			break;	
		case CAR_DOOR_RR:
		 	vecLocalSeatPosition = pModelInfo->GetBackSeatPosn();
			break;	
	}

	Seat = this->GetMatrix() * vecLocalSeatPosition;

	ExitPoint = CPed::GetPositionToOpenCarDoor(this, nDoor);

	if(pvecCarJackOffset)
	{
		vecAnimPos = *pvecCarJackOffset;
		if((nDoor == CAR_DOOR_RF)||(nDoor == CAR_DOOR_RR))
			vecAnimPos.x = -vecAnimPos.x;

		AnimExitPoint = Multiply3x3(GetMatrix(), vecAnimPos);
		ExitPoint += AnimExitPoint;
#ifdef DEBUG
		CDebug::DebugLine3D(ExitPoint.x-0.5f, ExitPoint.y, ExitPoint.z,
			ExitPoint.x+0.5f, ExitPoint.y, ExitPoint.z, 0x00ff00ff, 0x00ff00ff);
		CDebug::DebugLine3D(ExitPoint.x, ExitPoint.y-0.5f, ExitPoint.z,
			ExitPoint.x, ExitPoint.y+0.5f, ExitPoint.z, 0x00ff00ff, 0x00ff00ff);
		CDebug::DebugLine3D(ExitPoint.x, ExitPoint.y, ExitPoint.z-0.5f,
			ExitPoint.x, ExitPoint.y, ExitPoint.z+0.5f, 0x00ff00ff, 0x00ff00ff);
#endif		
	}

	// If the car is upside down we move the lines up a little bit (Otherwise we would
	// collide with the road..
	if (this->GetMatrix().zz < 0.0f)
	{
		Seat.z += 0.5f;
		ExitPoint.z += 0.5f;
	}


	Diff = ExitPoint - Seat;
	
	// appears we need to move the z up anyway (always colliding with the road)
	ExitPoint.z += 0.5f;

	Length = Diff.Magnitude();

	// Extend line a wee bit	
	ExtendedExitPoint = Seat + (Diff) * ((Length + 0.6f) / Length);

	// Draw a line between the two.
	// If testing objects is too strict we have to do something special since some
	// objects have to be taken into account (like the crushers' body)
	if (CWorld::GetIsLineOfSightClear(Seat, ExtendedExitPoint, true, false, false, true, false))
	{
//		CDebug::DebugLine3D(ExtendedExitPoint.x, ExtendedExitPoint.y, ExtendedExitPoint.z,
//								Seat.x, Seat.y, Seat.z, 0xffffffff, 0xffffffff);
	}
	else
	{
//		CDebug::DebugLine3D(ExtendedExitPoint.x, ExtendedExitPoint.y, ExtendedExitPoint.z,
//								Seat.x, Seat.y, Seat.z, 0xff00ffff, 0xff00ffff);
		return false;
	}
	
	// Now we do a bubble collision type thing to test whether the area is clear for the
	// ped. We only want to test against other cars and buildings (not roads) (check for peds too???????)

	if ( CWorld::TestSphereAgainstWorld(ExitPoint, 0.6f, this, true, true, false, true, false))
	{
//		CDebug::DebugLine3D(ExitPoint.x, ExitPoint.y, ExitPoint.z,
//								ExitPoint.x, ExitPoint.y, ExitPoint.z + 0.5f, 0xff0000ff, 0xff0000ff);	
		return false;
	}
	else
	{
//		CDebug::DebugLine3D(ExitPoint.x, ExitPoint.y, ExitPoint.z,
//								ExitPoint.x, ExitPoint.y, ExitPoint.z + 0.5f, 0xffffffff, 0xffffffff);
	}

	// Also make sure there is enough headroom.
	bReturnValue = CWorld::ProcessVerticalLine(ExitPoint, 1000.0f, TestColPoint, TestEntity, true, false, false, true, false);
	ZCeiling = TestColPoint.GetPosition().z;
	if (bReturnValue && ZCeiling > ExitPoint.z && ZCeiling < ExitPoint.z+0.6f)
	{
		return false;	// ceiling too low
	}
	bReturnValue = CWorld::ProcessVerticalLine(ExitPoint, -1000.0f, TestColPoint, TestEntity, true, false, false, true, false);
	if (!bReturnValue)
	{
		return false;	// No ground below us (BAD)
	}
	ZFloor = TestColPoint.GetPosition().z;
	
//	ASSERT(ZCeiling >= ZFloor)
		
//	if (ZCeiling - ZFloor < 1.8f)
	if (ZCeiling && ZCeiling < ZFloor)
	{
		return false;	// Not enough headroom for the player
	}
*/
	ASSERT(false);
	return	true;
}




bool16 	CAutomobile::RcbanditCheckHitWheels() const
{
	float	MinX, MaxX, MinY, MaxY;
	Int32	nLeft, nBottom, nRight, nTop;
	Int32	LoopX, LoopY;

	// Find the cars in this area
	// Identify scan area. (Block around car)
	MinX = this->GetPosition().x - 2.0f;
	MaxX = this->GetPosition().x + 2.0f;
	MinY = this->GetPosition().y - 2.0f;
	MaxY = this->GetPosition().y + 2.0f;

	// Find the sectorlists we have to go through
	nLeft = MAX(WORLD_WORLDTOSECTORX(MinX), 0);
	nBottom = MAX(WORLD_WORLDTOSECTORY(MinY), 0);
	nRight = MIN(WORLD_WORLDTOSECTORX(MaxX), (WORLD_WIDTHINSECTORS-1));
	nTop = MIN(WORLD_WORLDTOSECTORY(MaxY), (WORLD_DEPTHINSECTORS-1));

	CWorld::AdvanceCurrentScanCode();
	for (LoopY = nBottom; LoopY <= nTop; LoopY++)
	{
		for (LoopX = nLeft; LoopX <= nRight; LoopX++)
		{
			CRepeatSector& rsector = CWorld::GetRepeatSector(LoopX, LoopY);
			// First we check for cars
// Obbe: removed this. The non-overlap lists don't exist anymore.
//			if(RcbanditCheck1CarWheels(sector.GetVehiclePtrList()))
//				return true;
				
			if(RcbanditCheck1CarWheels(rsector.GetOverlapVehiclePtrList()))
				return true;
		}
	}
	return false;
}

bool16 CAutomobile::RcbanditCheck1CarWheels(CPtrList& list) const
{
	CColModel& colModel = CModelInfo::GetColModel(GetModelIndex());
	CPtrNode* pNode = list.GetHeadPtr();
	CEntity* pEntity;
	CAutomobile* pCar;
	CVehicleModelInfo *pModelInfo;
	CVector	WheelOffset;
	Int32	i;
	CColSphere WheelSphere;
	static CMatrix matW2B;

	while (pNode != NULL)
	{
		pEntity = (CEntity*) pNode->GetPtr();
		pNode = pNode->GetNextPtr();

		if((pEntity!=(CAutomobile*)this) && ((CVehicle *)pEntity)->GetBaseVehicleType() == VEHICLE_TYPE_CAR && pEntity->GetModelIndex() != MODELID_CAR_RCBANDIT)
		{			
			if ((pEntity->GetScanCode() != CWorld::GetCurrentScanCode()))
			{
				pEntity->SetScanCode(CWorld::GetCurrentScanCode());

					// Make sure the car is not too far away.
				if (ABS(this->GetPosition().x - pEntity->GetPosition().x) < 10.0f &&
					ABS(this->GetPosition().y - pEntity->GetPosition().y) < 10.0f)
				{
					pCar = (CAutomobile *)pEntity;
					pModelInfo = (CVehicleModelInfo*) CModelInfo::GetModelInfo(pCar->m_nModelIndex);
					// Find the coordinates of the wheels.
					for (i = 0; i < 4; i++)
					{
						if (pCar->m_aRatioHistory[i] < 1.0f || pCar->GetStatus()==STATUS_SIMPLE)	// Wheel is touching the ground
						{
							// get wheel position
							pModelInfo->GetWheelPosn(i, WheelOffset);

							// want to transform wheel pos into rcbandit frame
							matW2B = Invert(GetMatrix(), matW2B);
							WheelSphere.m_vecCentre = matW2B * (pCar->GetMatrix()*WheelOffset);
							WheelSphere.m_fRadius = 0.25f*pModelInfo->GetWheelScale(true);
							if(CCollision::TestSphereBox(WheelSphere, colModel.GetBoundBox()))
								return true;
						}
					}
				}
			}
		}
	}

	return false;
}


// Name			:	DeadPedMakesTyresBloody
// Purpose		:	A dead ped will call this function to make the tyres of a car muddy.
//					Also to make the car jump up a bit whilst driving over a ped..
// Parameters	:	None
// Returns		:	Nothing

void CPed::DeadPedMakesTyresBloody()
{
	float	MinX, MaxX, MinY, MaxY;
	Int32	nLeft, nBottom, nRight, nTop;
	Int32	LoopX, LoopY;

	// Find the cars in this area
	// Identify scan area. (Block around car)
	MinX = this->GetPosition().x - 2.0f;
	MaxX = this->GetPosition().x + 2.0f;
	MinY = this->GetPosition().y - 2.0f;
	MaxY = this->GetPosition().y + 2.0f;

	// Find the sectorlists we have to go through
	nLeft = MAX(WORLD_WORLDTOSECTORX(MinX), 0);
	nBottom = MAX(WORLD_WORLDTOSECTORY(MinY), 0);
	nRight = MIN(WORLD_WORLDTOSECTORX(MaxX), (WORLD_WIDTHINSECTORS-1));
	nTop = MIN(WORLD_WORLDTOSECTORY(MaxY), (WORLD_DEPTHINSECTORS-1));

	CWorld::AdvanceCurrentScanCode();
	for (LoopY = nBottom; LoopY <= nTop; LoopY++)
	{
		for (LoopX = nLeft; LoopX <= nRight; LoopX++)
		{
			CRepeatSector& rsector = CWorld::GetRepeatSector(LoopX, LoopY);
			// First we check for cars
// Obbe: removed this. The non-overlap lists don't exist anymore.
//			MakeTyresMuddySectorList(sector.GetVehiclePtrList());
			MakeTyresMuddySectorList(rsector.GetOverlapVehiclePtrList());
		}
	}
}


// Name			:	MakeTyresMuddySectorList
// Purpose		:	A dead ped will call this function to make the tyres of a car muddy.
//					Also to make the car jump up a bit whilst driving over a ped..
// Parameters	:	None
// Returns		:	Nothing

void CPed::MakeTyresMuddySectorList(CPtrList& list)
{
	CPtrNode* pNode = list.GetHeadPtr();
	CEntity* pEntity;
	CVehicle *pVehicle = NULL;
	CAutomobile *pCar = NULL;
	CBike *pBike = NULL;
	CVector	WheelOffset, WheelOffsetXFormed;
	Int32	i;
	float	DistSqr;

	while (pNode != NULL)
	{
		pEntity = (CEntity*) pNode->GetPtr();
		pNode = pNode->GetNextPtr();
		if((pEntity->GetScanCode() != CWorld::GetCurrentScanCode()))
		{
			pEntity->SetScanCode(CWorld::GetCurrentScanCode());

				// Make sure the car is not too far away.
			if (ABS(this->GetPosition().x - pEntity->GetPosition().x) < 10.0f &&
				ABS(this->GetPosition().y - pEntity->GetPosition().y) < 10.0f)
			{
				pVehicle = (CVehicle *)pEntity;
				if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR){
					pCar = (CAutomobile *)pEntity;
					pBike = NULL;
				}
				else if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE){
					pCar = NULL;
					pBike = (CBike *)pEntity;
				}
					// Make sure the car is going at a certain speed					
				if (pVehicle->GetMoveSpeed().x * pVehicle->GetMoveSpeed().x + pVehicle->GetMoveSpeed().y * pVehicle->GetMoveSpeed().y > 0.05f)
				{
					// Find the coordinates of the wheels.
					if(pCar)
					for (i = 0; i < 4; i++)
					{
						if (!pCar->bWheelBloody[i])	// Make sure wheel isn't already muddy
						{
							if (pCar->m_aWheelRatios[i] < 1.0f)	// Wheel is touching the ground
							{
								// Fine test coordinate
								switch (i)
								{
									case FRONT_LEFT_WHEEL:
										WheelOffset = CVector(-CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMax().x, CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMax().y, 0.0f);
										break;
									case REAR_LEFT_WHEEL:
										WheelOffset = CVector(-CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMax().x, CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMin().y, 0.0f);
										break;
									case FRONT_RIGHT_WHEEL:
										WheelOffset = CVector( CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMax().x, CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMax().y, 0.0f);
										break;
									case REAR_RIGHT_WHEEL:
										WheelOffset = CVector( CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMax().x, CModelInfo::GetColModel(pCar->GetModelIndex()).GetBoundBoxMin().y, 0.0f);
										break;
								}
								WheelOffsetXFormed = pCar->GetMatrix()*WheelOffset;

								// Check distance.
								if (ABS(WheelOffsetXFormed.z - this->GetPosition().z) < 2.0f)
								{
									DistSqr = (WheelOffsetXFormed.x - this->GetPosition().x) * (WheelOffsetXFormed.x - this->GetPosition().x) +
											(WheelOffsetXFormed.y - this->GetPosition().y) * (WheelOffsetXFormed.y - this->GetPosition().y);
									if (DistSqr < 1.0f)
									{
										if(CLocalisation::Blood())  // is there blood in this version?
										{
											pCar->bWheelBloody[i] = true;
											// Raymond: add crunchy sound here
#ifdef USE_AUDIOENGINE
											pCar->m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_DRIVE_OVER);
#else //USE_AUDIOENGINE
											DMAudio.PlayOneShot(pCar->AudioHandle, AE_PED_CRUNCH, 0.0f);
#endif //USE_AUDIOENGINE
										}
										
										if(pCar->m_fMass > 500.0f)
										{
											// Also apply an upward force here to make the car bump up
											pCar->ApplyMoveForce(0.0f, 0.0f, 50.0f*MIN(1.0f, m_fMass/1000.0f));
											pCar->ApplyTurnForce(0.0f, 0.0f, 50.0f*MIN(1.0f, m_fTurnMass/2000.0f), WheelOffsetXFormed.x - pCar->GetPosition().x, WheelOffsetXFormed.y - pCar->GetPosition().y, WheelOffsetXFormed.z - pCar->GetPosition().z);

											// Add little jolt.
											if(pCar == FindPlayerVehicle())
												CPad::GetPad(0)->StartShake(300, 70);
										}
									}
								}
							}
						}
					}
					else if(pBike)
					for (i = 0; i < 2; i++)
					{
						if(!pBike->bWheelBloody[i])	// Make sure wheel isn't already muddy
						{
							if(pBike->m_aWheelRatios[i*2] < 1.0f)	// Wheel is touching the ground
							{
								// Fine test coordinate
								switch (i)
								{
									case 0:
										WheelOffset = CVector(0.0f, 0.8f*CModelInfo::GetColModel(pBike->GetModelIndex()).GetBoundBoxMax().y, 0.0f);
										break;
									case 1:
										WheelOffset = CVector(0.0f, 0.8f*CModelInfo::GetColModel(pBike->GetModelIndex()).GetBoundBoxMin().y, 0.0f);
										break;
								}
								WheelOffsetXFormed = pBike->GetMatrix()*WheelOffset;

								// Check distance.
								if (ABS(WheelOffsetXFormed.z - this->GetPosition().z) < 2.0f)
								{
									DistSqr = (WheelOffsetXFormed.x - this->GetPosition().x) * (WheelOffsetXFormed.x - this->GetPosition().x) +
											(WheelOffsetXFormed.y - this->GetPosition().y) * (WheelOffsetXFormed.y - this->GetPosition().y);
									if (DistSqr < 1.0f)
									{
										if(CLocalisation::Blood())
										{
											pBike->bWheelBloody[i] = true;
											// Raymond: add crunchy sound here
#ifdef USE_AUDIOENGINE
											pBike->m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_DRIVE_OVER);
#else //USE_AUDIOENGINE
											DMAudio.PlayOneShot(pBike->AudioHandle, AE_PED_CRUNCH, 0.0f);
#endif //USE_AUDIOENGINE
										}
										
										if(pBike->m_fMass > 100.0f)
										{
											// Also apply an upward force here to make the car bump up
											pBike->ApplyMoveForce(0.0f, 0.0f, 10.0f);
											pBike->ApplyTurnForce(0.0f, 0.0f, 10.0f, WheelOffsetXFormed.x - pBike->GetPosition().x, WheelOffsetXFormed.y - pBike->GetPosition().y, WheelOffsetXFormed.z - pBike->GetPosition().z);

											// Add little jolt.
											if(pBike == FindPlayerVehicle())
												CPad::GetPad(0)->StartShake(300, 70);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


#define BUS_DOOR_TIME_TO_OPEN	(500)
#define BUS_DOOR_TIME_TO_CLOSE	BUS_DOOR_TIME_TO_OPEN
#define OPEN_THEN_CLOSE	(0)
#define CLOSE_ONLY		(1)

void CAutomobile::SetBusDoorTimer(uint32 nTime, uint8 nStatus)
{
	if(nTime < (BUS_DOOR_TIME_TO_OPEN + BUS_DOOR_TIME_TO_CLOSE))
	{
		nTime = BUS_DOOR_TIME_TO_OPEN + BUS_DOOR_TIME_TO_CLOSE;
	}
	
	if(nStatus == OPEN_THEN_CLOSE)
	{
		m_nBusDoorStart = CTimer::GetTimeInMilliseconds();
	}
	else
		m_nBusDoorStart = CTimer::GetTimeInMilliseconds()-BUS_DOOR_TIME_TO_OPEN;
	
	m_nBusDoorTimer = m_nBusDoorStart + nTime;
}

void CAutomobile::ProcessAutoBusDoors()
{
	if(m_nBusDoorTimer > CTimer::GetTimeInMilliseconds())
	{
		float fRatioOpen;// fully open
		
//		m_nHealth = 1000;
		// keep opening
		if((m_nBusDoorTimer > 0)&&(CTimer::GetTimeInMilliseconds() > m_nBusDoorTimer-BUS_DOOR_TIME_TO_CLOSE))
		{
			// keep going till closed
			if(!IsDoorMissing(FRONT_LEFT_DOOR) && !(m_nGettingInFlags & 1))
			{
				if(!IsDoorClosed(FRONT_LEFT_DOOR))
					fRatioOpen = 1.0f - ((float)(CTimer::GetTimeInMilliseconds()-(m_nBusDoorTimer-BUS_DOOR_TIME_TO_CLOSE))) / BUS_DOOR_TIME_TO_CLOSE;
				else
				{
					m_nBusDoorTimer = CTimer::GetTimeInMilliseconds();
					fRatioOpen = 0.0f;
				}
	
				OpenDoor(0, CAR_DOOR_LF, FRONT_LEFT_DOOR, fRatioOpen);
			}
			// keep going till closed
			if(!IsDoorMissing(FRONT_RIGHT_DOOR) && !(m_nGettingInFlags & 4))
			{
				if(!IsDoorClosed(FRONT_RIGHT_DOOR))
					fRatioOpen = 1.0f - ((float)(CTimer::GetTimeInMilliseconds()-(m_nBusDoorTimer-BUS_DOOR_TIME_TO_CLOSE))) / BUS_DOOR_TIME_TO_CLOSE;
				else
				{
					m_nBusDoorTimer = CTimer::GetTimeInMilliseconds();
					fRatioOpen = 0.0f;
				}
	
				OpenDoor(0, CAR_DOOR_RF, FRONT_RIGHT_DOOR, fRatioOpen);
			}
		}
	}
	else
	{
		if(m_nBusDoorStart != 0)
		{
			if(!IsDoorMissing(FRONT_LEFT_DOOR) && !(m_nGettingInFlags & 1))
				OpenDoor(0, CAR_DOOR_LF, FRONT_LEFT_DOOR, 0.0f);
			if(!IsDoorMissing(FRONT_RIGHT_DOOR)&& !(m_nGettingInFlags & 4))
				OpenDoor(0, CAR_DOOR_RF, FRONT_RIGHT_DOOR, 0.0f);
			
			m_nBusDoorStart = 0;
			m_nBusDoorTimer = 0;
		}
		// door should be closed		
	}
}



/////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION : TankControl
// PURPOSE :  Called from ProcessControl in Automobile.cpp
//
/////////////////////////////////////////////////////////////////////////////////
//
const CVector	vecTURRET_CENTRE	= CVector(0.0f,		-1.394f,	2.296f);
const CVector	vecBARREL_END		= CVector(0.0f,		 2.950f,	2.970f);
const CVector	vecTURBAR			= vecBARREL_END - vecTURRET_CENTRE;
//
CVector TIGER_GUN_POS		= CVector(0.0f, 0.5f, 0.2f);
uint32 TIGER_GUNFIRE_RATE	= 60;
//
TweakFloat TANK_GUN_ELEVATION = DEGTORAD(15.0f);
TweakFloat TANK_ROTATE_RATE = 0.015f;
TweakFloat TANK_ELEVATE_RATE = 0.005f;
TweakFloat TANK_ELEVATE_MAX = DEGTORAD(35.0f);
TweakFloat TANK_ELEVATE_MIN = DEGTORAD(-7.0f);
TweakFloat TANK_ELEVATE_MIN_REVERSE = DEGTORAD(-3.0f);

TweakFloat TANK_RECOIL_FORCE_MULT = -0.1f;
//
void CAutomobile::TankControl()
{
	if(GetModelIndex()==MODELID_CAR_RCTIGER && GetStatus()==STATUS_PLAYER_REMOTE)
	{
		if(CPad::GetPad(0)->CarGunJustDown() && CTimer::GetTimeInMilliseconds() > TimeOfLastShotFired + TIGER_GUNFIRE_RATE)
		{
			CWeapon tempWeapon(WEAPONTYPE_MINIGUN, 5000);
			CVector vecFirePos = TIGER_GUN_POS;
			vecFirePos = GetMatrix()*vecFirePos + m_vecMoveSpeed*CTimer::GetTimeStep();
			
			tempWeapon.FireInstantHit(this, &vecFirePos, &vecFirePos, NULL, NULL, NULL);
			tempWeapon.AddGunshell(this, vecFirePos, CVector2D(0.0f,0.1f), 0.025f);

#ifdef USE_AUDIOENGINE
			AudioEngine.ReportWeaponEvent(AUDIO_EVENT_WEAPON_FIRE, WEAPONTYPE_MINIGUN, this);
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(((CPhysical*)(this))->AudioHandle, AE_FIRE_WEAPON, 0.0f);
#endif //USE_AUDIOENGINE
			TimeOfLastShotFired = CTimer::GetTimeInMilliseconds();
		}
	}
	else if(GetStatus()==STATUS_PLAYER && GetModelIndex()==MODELID_CAR_RHINO)
	{
		// Make sure player isn't dead or anything
//		if (CWorld::Players[CWorld::PlayerInFocus].PlayerState == CPlayerInfo::PLAYERSTATE_PLAYING
		if (CGameLogic::GameState == CGameLogic::GAMESTATE_PLAYING
		&& pDriver && pDriver->IsPlayer())
		{
			CPad *pPad = ((CPlayerPed *)pDriver)->GetPadFromPlayer();
			if(!pPad)
				return;

			bool bRotating = pPad->RotateGun();

			// Fire Gun
			float OldGunOrientation = GunOrientation;
			if(TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING)
			{
				CVector vecForward = TheCamera.Cams[TheCamera.ActiveCam].Front;
				vecForward = Multiply3x3(vecForward, GetMatrix());
				
				float fDesGunOrientation = CMaths::ATan2(-vecForward.x, vecForward.y);
				float fDesGunElevation = CMaths::ATan2(vecForward.z, vecForward.Magnitude2D()) + TANK_GUN_ELEVATION;
				
				if(fDesGunOrientation > GunOrientation + PI)
					fDesGunOrientation -= TWO_PI;
				else if(fDesGunOrientation < GunOrientation - PI)
					fDesGunOrientation += TWO_PI;
				
				float fMove = fDesGunOrientation - GunOrientation;
				if(fMove > TANK_ROTATE_RATE*CTimer::GetTimeStep())
					GunOrientation += TANK_ROTATE_RATE*CTimer::GetTimeStep();
				else if(fMove < -TANK_ROTATE_RATE*CTimer::GetTimeStep())
					GunOrientation -= TANK_ROTATE_RATE*CTimer::GetTimeStep();
				else
					GunOrientation = fDesGunOrientation;
				
				fMove = fDesGunElevation - GunElevation;
				if(fMove > TANK_ELEVATE_RATE*CTimer::GetTimeStep())
					GunElevation += TANK_ELEVATE_RATE*CTimer::GetTimeStep();
				else if(fMove < -TANK_ELEVATE_RATE*CTimer::GetTimeStep())
					GunElevation -= TANK_ELEVATE_RATE*CTimer::GetTimeStep();
				else
					GunElevation = fDesGunElevation;
			}
			else
			{
				GunOrientation -= TANK_ROTATE_RATE*CTimer::GetTimeStep() * pPad->GetCarGunLeftRight()/128.0f;
				GunElevation += TANK_ELEVATE_RATE*CTimer::GetTimeStep() * pPad->GetCarGunUpDown()/128.0f;
			}
			
			if(GunOrientation < -PI)
				GunOrientation += TWO_PI;
			else if(GunOrientation > PI)
				GunOrientation -= TWO_PI;
			
			if(GunElevation > TANK_ELEVATE_MAX)
				GunElevation = TANK_ELEVATE_MAX;
			else if(GunOrientation > HALF_PI || GunOrientation < -HALF_PI)
			{
				float fSmoothRearMin = TANK_ELEVATE_MIN - CMaths::Max(-1.0f, 1.3f*CMaths::Cos(GunOrientation))*(TANK_ELEVATE_MIN_REVERSE - TANK_ELEVATE_MIN);
				if(GunElevation < fSmoothRearMin)
					GunElevation = fSmoothRearMin;
			}
			else if(GunElevation < TANK_ELEVATE_MIN)
				GunElevation = TANK_ELEVATE_MIN;

			if(OldGunOrientation != GunOrientation)
			{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_TANK_GUN_ROTATE, CMaths::Abs(GunOrientation - OldGunOrientation));	// actually a loop
#endif //USE_AUDIOENGINE
			}

			if(pPad->CarGunJustDown())
			{
				if(CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].LastTimeBigGunFired + 800)
				{
					CWorld::Players[CWorld::PlayerInFocus].LastTimeBigGunFired = CTimer::GetTimeInMilliseconds();

					//CVector Offset = CVector(0.0f, 0.0f, 2.2f);
					//CVector ShotVector = CVector(CMaths::Sin(-GunOrientation), CMaths::Cos(-GunOrientation), 0.0f);
					//Offset += ShotVector * 2.0f;
					//CVector ShotOrigin = this->GetMatrix() * Offset;
					//ShotVector = Multiply3x3(GetMatrix(), ShotVector);

					CVector ShotVector = CVector(CMaths::Sin(-GunOrientation), CMaths::Cos(GunOrientation), CMaths::Sin(GunElevation));
					ShotVector = Multiply3x3(this->GetMatrix(), ShotVector);

					CVector ShotOrigin;
					CVector Offset;
					if(m_aCarNodes[CAR_MISC_C])
					{
						ShotOrigin = RwFrameGetLTM(m_aCarNodes[CAR_MISC_C])->pos;
			
						// add on the speed of the nozzle point on the firetruck so we're not a frame behind
						Offset = ShotOrigin - GetPosition();
						Offset = GetSpeed(Offset);
						ShotOrigin += Offset*CTimer::GetTimeStep();
					}
					else
					{
						const float sinx = CMaths::Sin(GunOrientation);
						const float cosx = CMaths::Cos(GunOrientation);
						Offset.x = vecTURBAR.x*cosx - vecTURBAR.y*sinx;
						Offset.y = vecTURBAR.x*sinx + vecTURBAR.y*cosx;
						Offset.z = vecTURBAR.z /*correction?*/- 1.0f;
						Offset += vecTURRET_CENTRE;
						CVector ShotOrigin = this->GetMatrix() * Offset;
					}
//					m_vecMoveSpeed -= (ShotVector * 0.06f);
//					m_vecMoveSpeed.z += 0.05f;

					Offset = ShotOrigin - GetPosition();
					ApplyForce(TANK_RECOIL_FORCE_MULT*m_fMass*ShotVector, Offset);

					CVector ShotTarget = ShotOrigin + (ShotVector * 60.0f);

					// Find out where the impact is going to be.
//					CWeapon::DoTankDoomAiming(FindPlayerVehicle(), FindPlayerPed(), &ShotOrigin, &ShotTarget);
					CWeapon::DoTankDoomAiming(this, pDriver, &ShotOrigin, &ShotTarget);

					// Now we do a line check to find out where we hit stuff
					CColPoint	colPoint;
					CEntity		*pHitEntity = NULL;
					CWorld::pIgnoreEntity = this;
					CWorld::ProcessLineOfSight(ShotOrigin, ShotTarget, colPoint, pHitEntity, true, true, true, true, true, true);
					CWorld::pIgnoreEntity = NULL;

					if(pHitEntity)
					{		// Generate explosion at the coordinates of the gun
						ShotTarget = colPoint.GetPosition();
							// Move target back a little bit so that the explosion looks better
						ShotTarget -= (ShotTarget - ShotOrigin) * 0.04f;
						CExplosion::AddExplosion(NULL, FindPlayerPed(), EXP_TYPE_TANK_GRENADE, ShotTarget);
					}
					else
					{
						// Generate an explosion in mid air
						CExplosion::AddExplosion(NULL, FindPlayerPed(), EXP_TYPE_TANK_GRENADE, ShotTarget);
					}
				
					// MN - FX SYSTEM -------------------------------
					// tank firing			
					CVector fireDir = ShotTarget - ShotOrigin;
					g_fx.TriggerTankFire(ShotOrigin, fireDir);
					// ----------------------------------------------
	
					
				}//if(CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].LastTimeBigGunFired + 800)...
				
			}//if (CPad::GetPad(0)->WeaponJustDown())...
/*			
			// Make the turret rotate properly
			if(m_aCarNodes[CAR_WINDSCREEN])
			{
				CMatrix matrix;
				CVector posn;

				matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WINDSCREEN]));
				posn = matrix.GetTranslate();
				matrix.SetRotateZ(GunOrientation);
				matrix.Translate(posn);
				matrix.UpdateRW();
			}
*/			
		}//if (CWorld::Players[CWorld::PlayerInFocus].PlayerState == CPlayerInfo::PLAYERSTATE_PLAYING)...
		
	}//if(this == FindPlayerVehicle())...
	
	
}//end of CAutomobile::TankControl()....




/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : BlowUpCarsInPath
// PURPOSE :  This function will blow up cars that are touched by this vehicle
//			  Used for the likes of the tank.
/////////////////////////////////////////////////////////////////////////////////

void CAutomobile::BlowUpCarsInPath()
{
	Int32	Collision;

	// Make sure we're going fast enough.
	#define MINSPEEDTOCRUSH (0.1f)
	if ( (this->GetMoveSpeed().Magnitude() > MINSPEEDTOCRUSH) && m_nAutomobileFlags.bTankExplodesCars)
	{
		// Find the objects that we collided with recently
		for (Collision = 0; Collision < m_nNoOfCollisionRecords /*PHYSICAL_MAXNOOFCOLLISIONRECORDS*/; Collision++)
		{
			if (m_aCollisionRecordPtrs[Collision])
			{
				if (m_aCollisionRecordPtrs[Collision]->GetIsTypeVehicle())
				{
					if (m_aCollisionRecordPtrs[Collision]->GetModelIndex() != MODELID_CAR_RHINO)
					{
						if (!((CVehicle*)m_aCollisionRecordPtrs[Collision])->m_nPhysicalFlags.bRenderScorched)
						{		// This guy can still blow up

							// Make sure we get a wanted level for it eventually
							if (this == FindPlayerVehicle())
							{
								CCrime::ReportCrime(CRIME_CAUSE_EXPLOSION, (CVehicle *)m_aCollisionRecordPtrs[Collision], FindPlayerPed());
							}
							// Blow it up AFTER registering event or wanted level won't go up
							((CVehicle *)m_aCollisionRecordPtrs[Collision])->BlowUpCar(this);

						}
					}
				}
			}
		}	
	}
}




void CAutomobile::BoostJumpControl()
{
	if(pDriver && pDriver->IsPlayer() && ((CPlayerPed *)pDriver)->GetPadFromPlayer()
	&& ((CPlayerPed *)pDriver)->GetPadFromPlayer()->HornJustDown()
	&& m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT && m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT
	&& m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT && m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT)
	{
#ifdef USE_AUDIOENGINE
		//m_VehicleAudioEntity.AddAudioEvent(AUDIO_EVENT_SUSPENSION_TRIGGER);
#else //USE_AUDIOENGINE
		DMAudio.PlayOneShot(AudioHandle, AE_JACKUP_SUSPENSION, 0.0f);
		DMAudio.PlayOneShot(AudioHandle, AE_CAR_WHEEL_BUMP, 1.0f);
#endif //USE_AUDIOENGINE
/*		
#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (AddParticle)
// car boost jump

		CParticle::AddParticle(PARTICLE_ENGINE_STEAM,  m_aWheelColPoints[0].GetPosition() + 0.5f*GetMatrix().GetUp(), 1.3f*m_vecMoveSpeed, NULL, 2.5f);
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,  m_aWheelColPoints[0].GetPosition() + 0.5f*GetMatrix().GetUp(), 1.2f*m_vecMoveSpeed, NULL, 2.0f);
		
		CParticle::AddParticle(PARTICLE_ENGINE_STEAM,  m_aWheelColPoints[2].GetPosition() + 0.5f*GetMatrix().GetUp(), 1.3f*m_vecMoveSpeed, NULL, 2.5f);
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,  m_aWheelColPoints[2].GetPosition() + 0.5f*GetMatrix().GetUp(), 1.2f*m_vecMoveSpeed, NULL, 2.0f);

		CParticle::AddParticle(PARTICLE_ENGINE_STEAM,  m_aWheelColPoints[0].GetPosition() + 0.5f*GetMatrix().GetUp() - GetMatrix().GetForward(), 1.3f*m_vecMoveSpeed, NULL, 2.5f);
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,  m_aWheelColPoints[0].GetPosition() + 0.5f*GetMatrix().GetUp() - GetMatrix().GetForward(), 1.2f*m_vecMoveSpeed, NULL, 2.0f);
		
		CParticle::AddParticle(PARTICLE_ENGINE_STEAM,  m_aWheelColPoints[2].GetPosition() + 0.5f*GetMatrix().GetUp() - GetMatrix().GetForward(), 1.3f*m_vecMoveSpeed, NULL, 2.5f);
		CParticle::AddParticle(PARTICLE_ENGINE_SMOKE,  m_aWheelColPoints[2].GetPosition() + 0.5f*GetMatrix().GetUp() - GetMatrix().GetForward(), 1.2f*m_vecMoveSpeed, NULL, 2.0f);
		
#endif // MN - OLD FX SYSTEM (AddParticle)
*/


		// MN - FX SYSTEM -------------------------------
		// TODO: car boost jump
		// ----------------------------------------------


		ApplyMoveForce( 0.15f*m_fMass*CVector(0.0f,0.0f,1.0f) );
		ApplyTurnForce( 0.01f*m_fTurnMass*GetMatrix().GetUp(), 1.0f*GetMatrix().GetForward() );
	}
}

void CAutomobile::DoNitroEffect(float keyframeTime)
{
	// get exhaust pos
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());
	CVector exhaustPos = pModelInfo->GetPosnOnVehicle(VEHICLE_EXHAUST_POSN);
	CVector exhaustPos2;
	
	bool8 inWater1 = false;
	bool8 inWater2 = false;
	if (m_nPhysicalFlags.bForceFullWaterCheck)
	{
		CVector exhaustPosWld = GetMatrix()*exhaustPos;
		float waterLevel;
		if(CWaterLevel::GetWaterLevel(exhaustPosWld.x, exhaustPosWld.y, exhaustPosWld.z, &waterLevel, true))
		{
			if (waterLevel>=exhaustPosWld.z)
			{
				inWater1 = true;
			}
		}
	}

	// have we double exhast output?
	if (pHandling->mFlags &MF_DOUBLE_EXHAUST)
	{
		exhaustPos2 = exhaustPos;
		exhaustPos2.x *= -1.0f;
		
		if (m_nPhysicalFlags.bForceFullWaterCheck)
		{
			CVector exhaustPos2Wld = GetMatrix()*exhaustPos2;
			float waterLevel;
			if(CWaterLevel::GetWaterLevel(exhaustPos2Wld.x, exhaustPos2Wld.y, exhaustPos2Wld.z, &waterLevel, true))
			{
				if (waterLevel>=exhaustPos2Wld.z)
				{
					inWater2 = true;
				}
			}
		}
	}

	RwMatrix* pParentMat = GetRwMatrix();
	
	// nitro effect	
	if (m_pFxSysNitro1==NULL && inWater1==false && pParentMat)
	{
		// trigger nitro effect
		m_pFxSysNitro1 = g_fxMan.CreateFxSystem("nitro", (RwV3d*)&exhaustPos, pParentMat, true);
//		ASSERT(m_pFxSysNitro1);
		if (m_pFxSysNitro1)
		{
			m_pFxSysNitro1->SetLocalParticles(true);
			m_pFxSysNitro1->Play();
		}
	}
	else
	{
		if (m_pFxSysNitro1)
		{
			m_pFxSysNitro1->SetConstTime(true, CMaths::Abs(keyframeTime));	
			
			if (m_pFxSysNitro1->GetPlayStatus()==PS_PLAYING && inWater1)
			{
				m_pFxSysNitro1->Stop();
			}
			else if (m_pFxSysNitro1->GetPlayStatus()==PS_STOPPED && inWater1==false)
			{
				m_pFxSysNitro1->Play();
			}
		}
	}
		
	if (pHandling->mFlags &MF_DOUBLE_EXHAUST)
	{
		if (m_pFxSysNitro2==NULL && inWater1==false && pParentMat)
		{
			// trigger nitro effect
			m_pFxSysNitro2 = g_fxMan.CreateFxSystem("nitro", (RwV3d*)&exhaustPos2, pParentMat, true);
//			ASSERT(m_pFxSysNitro2);
			if (m_pFxSysNitro2)
			{
				m_pFxSysNitro2->SetLocalParticles(true);
				m_pFxSysNitro2->Play();
			}
		}
		else
		{
			if (m_pFxSysNitro2)
			{
				m_pFxSysNitro2->SetConstTime(true, CMaths::Abs(keyframeTime));	
					
				if (m_pFxSysNitro2->GetPlayStatus()==PS_PLAYING && inWater2)
				{
					m_pFxSysNitro2->Stop();
				}
				else if (m_pFxSysNitro2->GetPlayStatus()==PS_STOPPED && inWater2==false)
				{
					m_pFxSysNitro2->Play();
				}
			}
		}
	}
}

void CAutomobile::StopNitroEffect()
{
	// stop nitro effect
	if (m_pFxSysNitro1 != NULL)
	{
		m_pFxSysNitro1->Kill();
		m_pFxSysNitro1 = NULL;
	}
	
	if (m_pFxSysNitro2 != NULL)
	{
		m_pFxSysNitro2->Kill();
		m_pFxSysNitro2 = NULL;
	}
}

TweakFloat NOS_USE_RATE		= (0.001f);
TweakFloat NOS_COOL_RATE		= (0.001f);
//
void CAutomobile::NitrousControl(int8 nGiveBoosts)
{	
	CPad *pPad = NULL;
	if(GetStatus()==STATUS_PLAYER && pDriver->IsPlayer())
		pPad = ((CPlayerPed *)pDriver)->GetPadFromPlayer();
	
	bool8 killNitroEffect = true;
	
	if(nGiveBoosts > 0)
	{
		hFlagsLocal |= HF_NOS_INSTALLED;
		m_fTyreTemp = 1.0f;
		m_nNitroBoosts = nGiveBoosts;
	}
	else if(nGiveBoosts < 0)
	{
		hFlagsLocal &= ~HF_NOS_INSTALLED;
		m_fTyreTemp = 1.0f;
		m_nNitroBoosts = 0;
		killNitroEffect = true;
	}
	else if(m_fTyreTemp==1.0f && m_nNitroBoosts > 0)
	{
		if(GetStatus()==STATUS_PHYSICS || (pPad && pPad->GetCarGunFired() && !pPad->GetLookLeft() && !pPad->GetLookRight() && !pPad->GetLookBehindForCar()))
		{
			m_fTyreTemp = -0.000001f;
			if(m_nNitroBoosts < 101)
				m_nNitroBoosts--;
		}
	}
	else if(m_fTyreTemp < 0.0) // using nitrogen
	{
		// deplete gas
		m_fTyreTemp -= NOS_USE_RATE*CTimer::GetTimeStep();
			
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
		DMAudio.PlayOneShot(AudioHandle, AE_SLOW_SUSPENSION_MOVEMENT_LOOP, MIN(1.0f, DotProduct(m_vecMoveSpeed, GetMatrix().GetForward())/0.4f));
#endif //USE_AUDIOENGINE
		// if all used up
		if(m_fTyreTemp < -1.0f)
		{
			m_fTyreTemp = 0.000001f;
			// run out so remove bottle (if there is one)
			if(m_nNitroBoosts==0)
			{
				hFlagsLocal &= ~HF_NOS_INSTALLED;
				RemoveUpgrade(VEH_UPGRADE_NITRO);
				m_fTyreTemp = 1.0f;
				killNitroEffect = true;
			}
		}
			
		// nitro effect
		if (m_fGasPedal>0.0f)
		{
			float keyTime = 0.5f+(CMaths::Abs(m_fGasPedal)*0.5f);
			ASSERT(keyTime>=0.5f && keyTime<=1.0f);
			DoNitroEffect(keyTime);
		}
		else
		{
			DoNitroEffect(0.5f);
		}
		killNitroEffect = false;
	}
	else	// recharging / cooling off
	{
		// cools faster if moving
		//m_fTyreTemp += NOS_COOL_RATE*CTimer::GetTimeStep()*MIN(1.0f, MAX(3.0f, DotProduct(m_vecMoveSpeed, GetMatrix().GetForward())*4.0f));
		// scratch that - now cools faster if not accelerating!
		m_fTyreTemp += NOS_COOL_RATE*CTimer::GetTimeStep()*CMaths::Max(0.25, 1.0f - m_fGasPedal);

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
		DMAudio.PlayOneShot(AudioHandle, AE_RESET_SUSPENSION, 0.0f);
#endif //USE_AUDIOENGINE
		// ready to use again
		if(m_fTyreTemp > 1.0f)
			m_fTyreTemp = 1.0f;
			
		// nitro cooling effect
		float keyTime = (1.0f-m_fTyreTemp)*0.5f;
		ASSERT(keyTime>=0.0f && keyTime<=0.5f);
		DoNitroEffect(keyTime);
		killNitroEffect = false;
	}
	
	if (killNitroEffect)
	{
		StopNitroEffect();
	}
}

//
TweakFloat TOWTRUCK_HOIST_SPEED_DOWN 	= 6.0f;
TweakFloat TOWTRUCK_HOIST_SPEED_UP 		= 2.0f;
TweakFloat TOWTRUCK_PICKUP_LIMIT_XY		= 0.5f;
TweakFloat TOWTRUCK_PICKUP_LIMIT_Z 		= 1.0f;
//
void CAutomobile::TowTruckControl()
{
	if(GetStatus()!=STATUS_PLAYER)
		return;
		
	CPad *pPad = NULL;
	if( !(pDriver && pDriver->IsPlayer() && (pPad = ((CPlayerPed *)pDriver)->GetPadFromPlayer())) )
		return;

	if(CMaths::Abs( pPad->GetCarGunUpDown() ) > 10.0f)
	{
		uint16 nUpLimit = 0;
		if(m_pVehicleBeingTowed)
			nUpLimit = TOWTRUCK_HOIST_UP_LIMIT;
	
		if(pPad->GetCarGunUpDown() > 0.0f)
			m_nSuspensionHydraulics = MAX(nUpLimit, m_nSuspensionHydraulics - (int16)(pPad->GetCarGunUpDown()*TOWTRUCK_HOIST_SPEED_UP*CTimer::GetTimeStep()));
		else if(m_nSuspensionHydraulics < TOWTRUCK_HOIST_DOWN_LIMIT)
			m_nSuspensionHydraulics = MIN(TOWTRUCK_HOIST_DOWN_LIMIT, m_nSuspensionHydraulics - (int16)(pPad->GetCarGunUpDown()*TOWTRUCK_HOIST_SPEED_DOWN*CTimer::GetTimeStep()));
	}
	
#ifndef FINAL
	if(CPedDebugVisualiser::GetDebugDisplay())
	{
		CVector vecBarPos;
		GetTowBarPos(vecBarPos, true, this);
		CDebug::DebugLine3D(vecBarPos.x+0.5f,vecBarPos.y,vecBarPos.z, vecBarPos.x-0.5f,vecBarPos.y,vecBarPos.z, 0xffffffff,0xffffffff);
		CDebug::DebugLine3D(vecBarPos.x,vecBarPos.y+0.5f,vecBarPos.z, vecBarPos.x,vecBarPos.y-0.5f,vecBarPos.z, 0xffffffff,0xffffffff);
		CDebug::DebugLine3D(vecBarPos.x,vecBarPos.y,vecBarPos.z+0.5f, vecBarPos.x,vecBarPos.y,vecBarPos.z-0.5f, 0xffffffff,0xffffffff);
	}
#endif
	
	// if hoist is fully down, then scan for pickups
	if(m_nSuspensionHydraulics == TOWTRUCK_HOIST_DOWN_LIMIT
	&& m_pVehicleBeingTowed==NULL)
	{
		CVector vecTowBarPos(0.0f,0.0f,0.0f), vecTowHitchPos(0.0f,0.0f,0.0f);

		// this is a towtruck
		if(!GetTowBarPos(vecTowBarPos, false, this))
			return;

		CEntity *ppResults[16];
		CEntity *pFoundEntity = NULL;
		int16	nNum, i;
		CWorld::FindObjectsInRange(vecTowBarPos, 10.0f, true, &nNum, 16, ppResults, false, true, false, false, false);

		CVehicle *pVehicle = NULL;
		CVector vecDiff;
		for(i=0; i<nNum; i++)
		{
			pVehicle = (CVehicle *)ppResults[i];
			if(pVehicle!=this && pVehicle->GetTowHitchPos(vecTowHitchPos, true, this) && (pVehicle->m_nVehicleFlags.bIsLocked==false))
			{
#ifndef FINAL
if(CPedDebugVisualiser::GetDebugDisplay())
{
	CDebug::DebugLine3D(vecTowHitchPos.x+0.5f,vecTowHitchPos.y,vecTowHitchPos.z, vecTowHitchPos.x-0.5f,vecTowHitchPos.y,vecTowHitchPos.z, 0x00ff00ff,0x00ff00ff);
	CDebug::DebugLine3D(vecTowHitchPos.x,vecTowHitchPos.y+0.5f,vecTowHitchPos.z, vecTowHitchPos.x,vecTowHitchPos.y-0.5f,vecTowHitchPos.z, 0x00ff00ff,0x00ff00ff);
	CDebug::DebugLine3D(vecTowHitchPos.x,vecTowHitchPos.y,vecTowHitchPos.z+0.5f, vecTowHitchPos.x,vecTowHitchPos.y,vecTowHitchPos.z-0.5f, 0x00ff00ff,0x00ff00ff);
}
#endif
				CVector vecDiff = vecTowHitchPos - vecTowBarPos;
				if(vecDiff.Magnitude2D() < TOWTRUCK_PICKUP_LIMIT_XY
				&& CMaths::Abs(vecDiff.z) < TOWTRUCK_PICKUP_LIMIT_Z)
				{
					pVehicle->SetTowLink(this, false);
					m_nSuspensionHydraulics -= 100;
					return;
				}
			}
		}	
	}
}

//
#define CAR_HITCH_OFFSET_Y (-0.5f)
//
bool CAutomobile::GetTowHitchPos(CVector &vecResult, bool bGenericPosOk, CVehicle *pVeh)
{
	if(bGenericPosOk)
	{
		vecResult.x = 0.0f;
		vecResult.y = CModelInfo::GetColModel(GetModelIndex()).GetBoundBoxMax().y + CAR_HITCH_OFFSET_Y;
		vecResult.z = TOW_LINK_DEFAULT_Z - m_fHeightAboveRoad;
		
		vecResult = GetMatrix() * vecResult;
		return true;
	}
	return false;
}

//
TweakFloat CAR_TOWBAR_OFFSET_Y	= -0.5f;
TweakFloat TOWTRUCK_OFFSET_Y 	= -1.05f;
TweakFloat TOWTRUCK_OFFSET_Z 	= 1.0f;
TweakFloat TRACTOR_OFFSET_Y		= -0.6f;
TweakFloat TRACTOR_OFFSET_Z		= 0.5f;
//
bool CAutomobile::GetTowBarPos(CVector &vecResult, bool bGenericPosOk, CVehicle *pVeh)
{
	if(GetModelIndex()==MODELID_CAR_TOWTRUCK || GetModelIndex()==MODELID_CAR_TRACTOR)
	{
		float fOffsetY = TOWTRUCK_OFFSET_Y;
		float fOffsetZ = TOWTRUCK_OFFSET_Z;
		if(GetModelIndex()==MODELID_CAR_TRACTOR)
		{
			if(pVeh && pVeh->GetVehicleType()==VEHICLE_TYPE_TRAILER	&& pVeh->GetModelIndex()!=MODELID_TRAILER_FARM_1)
				return false;
				
			fOffsetY = TRACTOR_OFFSET_Y;
			fOffsetZ = TRACTOR_OFFSET_Z;
		}
		else if(pVeh && pVeh->GetVehicleType()==VEHICLE_TYPE_TRAILER)
			return false;
			
	
		vecResult.x = 0.0f;
		vecResult.y = CModelInfo::GetColModel(GetModelIndex()).GetBoundBoxMin().y + fOffsetY;
		vecResult.z = (1.0f - m_nSuspensionHydraulics/(float)TOWTRUCK_HOIST_DOWN_LIMIT)*TRACTOR_OFFSET_Z + TOW_LINK_DEFAULT_Z - m_fHeightAboveRoad;
		
		vecResult = GetMatrix() * vecResult;
		return true;
	}
	else
	{
		bool bUseMiscA = false;
		if(GetModelIndex()==MODELID_CAR_PETROL || GetModelIndex()==MODELID_CAR_RDTRAIN || GetModelIndex()==MODELID_CAR_LINERUNNER || GetModelIndex()==MODELID_TRAILER_ARTICT3)
			bUseMiscA = true;
		else if(GetModelIndex()==MODELID_CAR_UTILITY && pVeh && pVeh->GetModelIndex()==MODELID_TRAILER_UTIL_1)
			bUseMiscA = true;
		else if((GetModelIndex()==MODELID_CAR_BAGGAGE || GetModelIndex()==MODELID_CAR_TUG || GetModelIndex()==MODELID_TRAILER_BAGBOX_A || GetModelIndex()==MODELID_TRAILER_BAGBOX_B)
		&& pVeh && (pVeh->GetModelIndex()==MODELID_TRAILER_BAGBOX_A || pVeh->GetModelIndex()==MODELID_TRAILER_BAGBOX_B || pVeh->GetModelIndex()==MODELID_TRAILER_TUGSTAIR))
			bUseMiscA = true;
		
		if(bUseMiscA && m_aCarNodes[CAR_MISC_A])
		{
			vecResult = RwFrameGetLTM(m_aCarNodes[CAR_MISC_A])->pos;
			return true;
		}
		else if(bGenericPosOk)
		{
			vecResult.x = 0.0f;
			vecResult.y =  CModelInfo::GetColModel(GetModelIndex()).GetBoundBoxMin().y + CAR_TOWBAR_OFFSET_Y;
			vecResult.z = TOW_LINK_DEFAULT_Z - m_fHeightAboveRoad;
			
			vecResult = GetMatrix() * vecResult;
			return true;
		}
	}
	
	return false;
}

bool CAutomobile::SetTowLink(CVehicle *pTractor, bool bWarpPosition)
{
	if(pTractor==NULL || m_pTowingVehicle)
		return false;
	
	if(GetStatus()==STATUS_PHYSICS || GetStatus()==STATUS_TRAILER || GetStatus()==STATUS_ABANDONED
	|| GetStatus()==STATUS_SIMPLE)
	{
		if(GetStatus()==STATUS_SIMPLE)
			CCarCtrl::SwitchVehicleToRealPhysics(this);
	
		SetStatus(STATUS_TRAILER);
	
		m_pTowingVehicle = pTractor;
//		m_pTowingVehicle->RegisterReference((CEntity **)&m_pTowingVehicle);
		REGREF(m_pTowingVehicle, ((CEntity **)&m_pTowingVehicle));
		
		pTractor->m_pVehicleBeingTowed = this;
//		pTractor->m_pVehicleBeingTowed->RegisterReference((CEntity **)&pTractor->m_pVehicleBeingTowed);
		REGREF(pTractor->m_pVehicleBeingTowed, ((CEntity **)&pTractor->m_pVehicleBeingTowed));
		
		// remove and add to moving list
		// to make sure we process the tractor before the trailer
		this->RemoveFromMovingList();
		m_pTowingVehicle->RemoveFromMovingList();
		
		this->AddToMovingList();
		m_pTowingVehicle->AddToMovingList();
		
		if(bWarpPosition)
		{
			CVector vecTowBarPos(0.0f,0.0f,0.0f), vecTowHitchPos(0.0f,0.0f,0.0f);
			// if directly linking to a towtruck then set controls to fully lifted
			if(pTractor->GetModelIndex()==MODELID_CAR_TOWTRUCK || pTractor->GetModelIndex()==MODELID_CAR_TRACTOR)
				((CAutomobile *)m_pTowingVehicle)->m_nSuspensionHydraulics = TOWTRUCK_HOIST_UP_LIMIT;
			
			SetHeading(pTractor->GetHeading());
			if(!GetTowHitchPos(vecTowHitchPos, true, this))
				return false;
				
			if(!pTractor->GetTowBarPos(vecTowBarPos, true, this))
				return false;
			
			vecTowHitchPos -= GetPosition();
			SetPosition(vecTowBarPos - vecTowHitchPos);
			// now make sure it's sitting on the road properly
			PlaceOnRoadProperly();
		}
		else
		{
			// this is getting picked up by some towing vehicle, so change our velocity to match theirs
			UpdateTrailerLink(true);
		}

		return true;
	}
	
	return false;
}

bool CAutomobile::BreakTowLink()
{
	// make sure the pointers get cleared whether the status is trailer or not
	if(m_pTowingVehicle)
	{
		if(m_pTowingVehicle->m_pVehicleBeingTowed)
		{
			m_pTowingVehicle->m_pVehicleBeingTowed->CleanUpOldReference((CEntity**)&m_pTowingVehicle->m_pVehicleBeingTowed);
			m_pTowingVehicle->m_pVehicleBeingTowed = NULL;
		}
		m_pTowingVehicle->CleanUpOldReference((CEntity**)&m_pTowingVehicle);
		m_pTowingVehicle = NULL;
	}

	// only break link if still being towed
	if(GetStatus()!=STATUS_TRAILER && GetStatus()!=STATUS_SIMPLE_TRAILER)
		return false;
	
	if(pDriver)
	{
		if(pDriver->IsPlayer())
			SetStatus(STATUS_PLAYER);
		else
			SetStatus(STATUS_PHYSICS);
	}
	else if(m_nHealth < 1.0f)
		SetStatus(STATUS_WRECKED);
	else
		SetStatus(STATUS_ABANDONED);
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : PlaceOnRoadProperly
// PURPOSE :  If the car is placed kinda on the road this function will
//			  do it properly
/////////////////////////////////////////////////////////////////////////////////

void CAutomobile::PlaceOnRoadProperly()
{


	CVector	FrontPoint, RearPoint;
	CVector	VecAlongLength, ResultCoors;
	CColPoint	TestColPoint;
	CEntity*	TestEntity;
	CVector		Up;
	bool		bHeightFound;
	float		HeightFound;
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	CColModel& colModel = GetColModel();
	CCollisionData* pColData = colModel.GetCollisionData();

	float	CarLengthFront = pColData->m_pLineArray[0].m_vecStart.y;
	float	CarLengthRear = -pColData->m_pLineArray[3].m_vecStart.y;
	

#define HEIGHTSAMPLERANGEUP		(5.0f)
#define HEIGHTSAMPLERANGEDOWN	(5.0f)
	// Calculate front point
	VecAlongLength = this->GetMatrix().GetForward();

	// Calculate rear point
	RearPoint.x = this->GetPosition().x - (VecAlongLength.x * CarLengthRear);
	RearPoint.y = this->GetPosition().y - (VecAlongLength.y * CarLengthRear);

	// Set new front point
	FrontPoint.x = this->GetPosition().x + (VecAlongLength.x * CarLengthFront);
	FrontPoint.y = this->GetPosition().y + (VecAlongLength.y * CarLengthFront);

	// Read the heights of the map for the front and rear points
	FrontPoint.z = this->GetPosition().z;
	bHeightFound = false;

	if (CWorld::ProcessVerticalLine(FrontPoint, FrontPoint.z + HEIGHTSAMPLERANGEUP, TestColPoint, TestEntity, true, false, false, false, false, false, NULL))
	{
		bHeightFound = true;
		HeightFound = TestColPoint.GetPosition().z;
		this->pEntityWeAreOnForVisibilityCheck = TestEntity;
		ASSERT(TestEntity);
		this->m_nFlags.bTunnel = TestEntity->m_nFlags.bTunnel;
		this->m_nFlags.bTunnelTransition = TestEntity->m_nFlags.bTunnelTransition;
	}
	if (CWorld::ProcessVerticalLine(FrontPoint, FrontPoint.z - HEIGHTSAMPLERANGEDOWN, TestColPoint, TestEntity, true, false, false, false, false, false, NULL))
	{
		if (bHeightFound)
		{	// Take the nearest of the 2 heights
			if ( CMaths::Abs(FrontPoint.z - HeightFound) > CMaths::Abs(FrontPoint.z - TestColPoint.GetPosition().z) )
			{
				HeightFound = TestColPoint.GetPosition().z;
				this->pEntityWeAreOnForVisibilityCheck = TestEntity;
				ASSERT(TestEntity);
				this->m_nFlags.bTunnel = TestEntity->m_nFlags.bTunnel;
				this->m_nFlags.bTunnelTransition = TestEntity->m_nFlags.bTunnelTransition;
			}
		}
		else
		{
			HeightFound = TestColPoint.GetPosition().z;
			this->pEntityWeAreOnForVisibilityCheck = TestEntity;
			ASSERT(TestEntity);
			this->m_nFlags.bTunnel = TestEntity->m_nFlags.bTunnel;
			this->m_nFlags.bTunnelTransition = TestEntity->m_nFlags.bTunnelTransition;
		}
		bHeightFound = true;
	}
	if (bHeightFound)
	{
		// If we didn't find anything we keep the value we already had. We've probably
		// found a hole in the map.
		FrontPoint.z = HeightFound;
		StoredCollPolys[0].lighting = TestColPoint.GetLightingB();
	}




	RearPoint.z = this->GetPosition().z;
	bHeightFound = false;
	if (CWorld::ProcessVerticalLine(RearPoint, RearPoint.z + HEIGHTSAMPLERANGEUP, TestColPoint, TestEntity, true, false, false, false, false, false, NULL))
	{
		bHeightFound = true;
		HeightFound = TestColPoint.GetPosition().z;
		this->pEntityWeAreOnForVisibilityCheck = TestEntity;
		ASSERT(TestEntity);
		this->m_nFlags.bTunnel = TestEntity->m_nFlags.bTunnel;
		this->m_nFlags.bTunnelTransition = TestEntity->m_nFlags.bTunnelTransition;
	}
	if (CWorld::ProcessVerticalLine(RearPoint, RearPoint.z - HEIGHTSAMPLERANGEDOWN, TestColPoint, TestEntity, true, false, false, false, false, false, NULL))
	{
		if (bHeightFound)
		{	// Take the nearest of the 2 heights
			if ( CMaths::Abs(RearPoint.z - HeightFound) > CMaths::Abs(RearPoint.z - TestColPoint.GetPosition().z) )
			{
				HeightFound = TestColPoint.GetPosition().z;
				this->pEntityWeAreOnForVisibilityCheck = TestEntity;
				ASSERT(TestEntity);
				this->m_nFlags.bTunnel = TestEntity->m_nFlags.bTunnel;
				this->m_nFlags.bTunnelTransition = TestEntity->m_nFlags.bTunnelTransition;
			}
		}
		else
		{
			HeightFound = TestColPoint.GetPosition().z;
			this->pEntityWeAreOnForVisibilityCheck = TestEntity;
			ASSERT(TestEntity);
			this->m_nFlags.bTunnel = TestEntity->m_nFlags.bTunnel;
			this->m_nFlags.bTunnelTransition = TestEntity->m_nFlags.bTunnelTransition;
		}
		bHeightFound = true;
	}
	if (bHeightFound)
	{
		// If we didn't find anything we keep the value we already had. We've probably
		// found a hole in the map.
		RearPoint.z = HeightFound;
		StoredCollPolys[1].lighting = TestColPoint.GetLightingB();
	}

	FrontPoint.z += GetHeightAboveRoad();
	RearPoint.z += GetRearHeightAboveRoad();
	
	// Generate the new coordinates and matrix
	// Side vector
	this->GetMatrix().xx = (FrontPoint.y - RearPoint.y) / (CarLengthFront + CarLengthRear);
	this->GetMatrix().yx = -(FrontPoint.x - RearPoint.x) / (CarLengthFront + CarLengthRear);
	this->GetMatrix().zx = 0.0f;
	// Front vector
	CVector	FrontVec = FrontPoint - RearPoint;
	FrontVec.Normalise();
	this->GetMatrix().xy = FrontVec.x;
	this->GetMatrix().yy = FrontVec.y;
	this->GetMatrix().zy = FrontVec.z;
	// Up vector
	Up = CrossProduct(this->GetMatrix().GetRight(), this->GetMatrix().GetForward());
	this->GetMatrix().xz = Up.x;
	this->GetMatrix().yz = Up.y;
	this->GetMatrix().zz = Up.z;

	// Write the new coordinates and matrix
	ResultCoors = (FrontPoint * CarLengthRear + RearPoint * CarLengthFront) / (CarLengthRear + CarLengthFront);
	
	this->SetPosition( ResultCoors );

	// For planes we also set the take-off direction
	if (this->GetVehicleType() == VEHICLE_TYPE_PLANE)
	{
		((CPlane *)this)->m_TakeOffDirection = CGeneral::GetATanOfXY(GetMatrix().GetForward().x, GetMatrix().GetForward().y);
	}
}



CPed *CAutomobile::KnockPedOutCar(eWeaponType nHitType, uint16 nDoor, CPed *pPed)
{
	/*
	AnimationId nAnim = ANIM_STD_KO_FRONT;
		
	if(pPed)
	{
		pPed->m_nDoor = nDoor;
		pPed->SetPedState(PED_IDLE);
		CAnimManager::BlendAnimation((RpClump*)pPed->m_pRwObject, pPed->m_motionAnimGroup, ANIM_STD_IDLE, 100.0f);

		// call the CallBack fn directly cause we're not using a getout anim
		pPed->PedSetOutCarCB(NULL,pPed);
		pPed->SetMoveState(PEDMOVE_STILL);
		if(GetMatrix().GetUp().z < 0.0f)
			pPed->SetHeading(CGeneral::LimitRadianAngle(this->GetHeading() + PI));
		else
			pPed->SetHeading(this->GetHeading());

		switch(nHitType)
		{
			case WEAPONTYPE_FALL:
			case WEAPONTYPE_RAMMEDBYCAR:
			case WEAPONTYPE_BASEBALLBAT:
				nAnim = ANIM_STD_SPINFORWARD_LEFT;
				pPed->SetMoveSpeed(m_vecMoveSpeed);
				pPed->ApplyMoveForce(8.0f*GetMatrix().GetRight() + 4.0f*GetMatrix().GetUp());
				break;
				
			case WEAPONTYPE_UNIDENTIFIED:
			case WEAPONTYPE_UNARMED:
				nAnim = ANIM_STD_NUM;
				pPed->SetMoveSpeed(m_vecMoveSpeed);
				pPed->m_pNOCollisionVehicle = this;
				break;
				
		}

		//  might have jumped rather than fallen off
		if(nHitType!=WEAPONTYPE_UNARMED)
		{
			pPed->SetFall(1000, nAnim);
			pPed->SetIsStanding(false);
			pPed->SetHeadingRate(0.0f);
		}
		pPed->m_pMyVehicle = NULL;
	}
	*/
	return pPed;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : PopBoot
// PURPOSE :  If this vehicle has a boot we will make it open
/////////////////////////////////////////////////////////////////////////////////

void CAutomobile::PopBoot()
{

	switch(Damage.GetDoorStatus(BOOT))
	{
		case DT_DOOR_INTACT:
		case DT_DOOR_BASHED:
//			Damage.SetDoorStatus(BOOT, DT_DOOR_SWINGING_FREE);
			Door[BOOT].m_fAngle = Door[BOOT].m_fOpenAngle;

			CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
			CVector posn(matrix.GetTranslate());
			float orien[3] = {0.0f, 0.0f, 0.0f};
		
			orien[Door[BOOT].RetAxes()] = Door[BOOT].RetDoorAngle();

			matrix.SetRotate(orien[0], orien[1], orien[2]);		
			matrix.Translate(posn);
			matrix.UpdateRW();

			break;
	}

	// Give the boot a bit of momentum so that it swings open.
//	Door[BOOT].m_fAngVel = -2.0f;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : PopBootUsingPhysics
// PURPOSE :  If this vehicle has a boot we will make it open
/////////////////////////////////////////////////////////////////////////////////

void CAutomobile::PopBootUsingPhysics()
{
	switch(Damage.GetDoorStatus(BOOT))
	{
		case DT_DOOR_INTACT:
			Damage.SetDoorStatus(BOOT, DT_DOOR_SWINGING_FREE);
		case DT_DOOR_BASHED:
			Damage.SetDoorStatus(BOOT, DT_DOOR_BASHED_AND_SWINGING_FREE);
			break;
	}

	// Give the boot a bit of momentum so that it swings open.
	Door[BOOT].m_fAngVel = -2.0f;
}



/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CloseBoot
// PURPOSE :  If this vehicle has a boot we will make it open
/////////////////////////////////////////////////////////////////////////////////

void CAutomobile::CloseBoot()
{

	Door[BOOT].m_fAngle = Door[BOOT].m_fClosedAngle;

	CMatrix matrix(RwFrameGetMatrix(m_aCarNodes[CAR_BOOT]));
	CVector posn(matrix.GetTranslate());
	float orien[3] = {0.0f, 0.0f, 0.0f};
		
		//Door[DoorID].Process((CVehicle *)this);
	orien[Door[BOOT].RetAxes()] = Door[BOOT].RetDoorAngle();

	matrix.SetRotate(orien[0], orien[1], orien[2]);		
	matrix.Translate(posn);
	matrix.UpdateRW();
}

void CAutomobile::CloseAllDoors()
{
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());

	if(!IsDoorMissing(FRONT_LEFT_DOOR))
		OpenDoor(0, CAR_DOOR_LF, FRONT_LEFT_DOOR, 0.0f);

	if(pModelInfo->GetNumberOfDoors() > 1)
	{
		if(!IsDoorMissing(FRONT_RIGHT_DOOR))
			OpenDoor(0, CAR_DOOR_RF, FRONT_RIGHT_DOOR, 0.0f);

		if(pModelInfo->GetNumberOfDoors() > 2)
		{
			if(!IsDoorMissing(REAR_LEFT_DOOR))
				OpenDoor(0, CAR_DOOR_LR, REAR_LEFT_DOOR, 0.0f);

			if(!IsDoorMissing(REAR_RIGHT_DOOR))
				OpenDoor(0, CAR_DOOR_RR, REAR_RIGHT_DOOR, 0.0f);
		}
	}
}


//
//
//
void CAutomobile::DoHoverSuspensionRatios()
{
	if(GetMatrix().GetUp().z < 0.3f || m_nVehicleFlags.bIsDrowning)
		return;

	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	CColModel& colModel = GetColModel();
	CCollisionData* pColData = colModel.GetCollisionData();

	CVector vecLineStartPos, vecLineEndPos;
	float fWaterLevel, fGroundLevel;
	
	for(int32 i=0; i<4; i++)
	{
		vecLineStartPos = GetMatrix()*pColData->m_pLineArray[i].m_vecStart;
		vecLineEndPos = GetMatrix()*pColData->m_pLineArray[i].m_vecEnd;
		
		if(m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
			fGroundLevel = m_aWheelColPoints[i].GetPosition().z;
		else
			fGroundLevel = -100.0f;
	
		if(CWaterLevel::GetWaterLevel(vecLineEndPos.x, vecLineEndPos.y, vecLineEndPos.z, &fWaterLevel, false)
		&& fWaterLevel > fGroundLevel && fWaterLevel > vecLineEndPos.z - 1.0f)
		{
			if(fWaterLevel <= vecLineEndPos.z)
				m_aWheelRatios[i] = HOVER_DAMPING_EXTENSION_LIMIT;
			else if(fWaterLevel > vecLineStartPos.z)
				m_aWheelRatios[i] = 0.0f;
			else
				m_aWheelRatios[i] = (vecLineStartPos.z - fWaterLevel)/(vecLineStartPos.z - vecLineEndPos.z);
				
			vecLineEndPos = vecLineStartPos + m_aWheelRatios[i]*(vecLineEndPos - vecLineStartPos);
			vecLineEndPos.z = fWaterLevel;
			m_aWheelColPoints[i].SetPosition(vecLineEndPos);
			m_aWheelColPoints[i].SetNormal(0.0f, 0.0f, 1.0f);//CWaterLevel::GetWaterNormal(vecLineEndPos.x, vecLineEndPos.y));
			m_aWheelColPoints[i].SetSurfaceTypeB(SURFACE_TYPE_WATER_SHALLOW);
		}
	}
}

//
//
//
// all these are fractions of the wheel radius
TweakFloat WHEEL_RATIO_BURST_MULT = 0.25f;
TweakFloat WHEEL_RATIO_SINK_IN_SAND = 0.30f;
TweakFloat WHEEL_RATIO_SINK_IN_SAND_OA1 = 0.20f;
TweakFloat WHEEL_RATIO_SINK_IN_SAND_OA2 = 0.15f;

TweakFloat WHEEL_RATIO_RAILTRACK_BUMP_MULT = 0.3f;
TweakFloat WHEEL_RATIO_RAILTRACK_BUMP_DIST = 1.5f;
TweakFloat WHEEL_RATIO_RAILTRACK_BUMP_DIST_SPEED = 0.3f;
//
void CAutomobile::DoBurstAndSoftGroundRatios()
{
#ifndef FINAL	
	if(CTimer::bSkipProcessThisFrame)
		return;
#endif

	CVehicleModelInfo *pModelInfo = (CVehicleModelInfo *)CModelInfo::GetModelInfo(GetModelIndex());

	// need the vehicles speed for burst tyres and driving in sand
	float fForwardSpeed = CMaths::Abs(DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()));

	for(int32 i=0; i<4; i++)
	{
		// simulate missing wheel by forcing a skip over the wheels processing code
		if(Damage.GetWheelStatus(i) == DT_WHEEL_MISSING)
			m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;
		else if(Damage.GetWheelStatus(i) == DT_WHEEL_BURST)
		{
			// now to time*speed based thing to bump wheel as car moves along
			if( CGeneral::GetRandomNumberInRange(0, 98 + (uint16)(fForwardSpeed*40)) < 100)
			{
				m_aWheelRatios[i] +=  WHEEL_RATIO_BURST_MULT*(m_fLineLength[i] - m_fSuspensionLength[i]) / m_fLineLength[i];
				if(m_aWheelRatios[i] > BILLS_EXTENSION_LIMIT)
					m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;
			}
		}
		// make wheels sink a bit into sand (rises out at speed)
		else if(m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT
		&& g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[i].GetSurfaceTypeB())==ADHESION_GROUP_SAND
		&& GetModelIndex()!=MODELID_CAR_RHINO)
		{
			float fSinkRatio = WHEEL_RATIO_SINK_IN_SAND;
			if(hFlagsLocal &HF_OFFROAD_ABILITIES_X2)
				fSinkRatio = WHEEL_RATIO_SINK_IN_SAND_OA2;
			else if(hFlagsLocal &HF_OFFROAD_ABILITIES)
				fSinkRatio = WHEEL_RATIO_SINK_IN_SAND_OA1;
		
			m_aWheelRatios[i] +=  fSinkRatio*MAX(0.4f, 1.0f - 0.7f*fForwardSpeed/SAND_WHEEL_BOGDOWN_LIMIT - 0.7f*CWeather::WetRoads)*(m_fLineLength[i] - m_fSuspensionLength[i]) / m_fLineLength[i];
			if(m_aWheelRatios[i] > BILLS_EXTENSION_LIMIT)
				m_aWheelRatios[i] = BILLS_EXTENSION_LIMIT;
		}
		else if(m_aWheelRatios[i] < 1.0f && m_aWheelColPoints[i].GetSurfaceTypeB()==CAR_RAILTRACK_SURFACETYPE)
		{
			float fBumpAngle = WHEEL_RATIO_RAILTRACK_BUMP_DIST / (0.5f*pModelInfo->GetWheelScale(i==0 || i==2));
			// stretch distance between sleepers when we're going really fast so wheels don't go too mental!
			if(fForwardSpeed > WHEEL_RATIO_RAILTRACK_BUMP_DIST_SPEED)
				fBumpAngle *= (fForwardSpeed / WHEEL_RATIO_RAILTRACK_BUMP_DIST_SPEED);
			
			float fWheelAngleFrac = m_aWheelPitchAngles[i] / fBumpAngle;
			fWheelAngleFrac -= CMaths::Floor(fWheelAngleFrac);
			
			float fNewWheelAngleFrac = (m_aWheelPitchAngles[i] + m_aWheelAngularVelocity[i]*CTimer::GetTimeStep()) / fBumpAngle;
			fNewWheelAngleFrac -= CMaths::Floor(fNewWheelAngleFrac);
			  
			if((m_aWheelAngularVelocity[i] > 0.0f && fNewWheelAngleFrac < fWheelAngleFrac)
			|| (m_aWheelAngularVelocity[i] < 0.0f && fNewWheelAngleFrac > fWheelAngleFrac))
			{
				float fWheelRatioAdd = WHEEL_RATIO_RAILTRACK_BUMP_MULT*(m_fLineLength[i] - m_fSuspensionLength[i]) / m_fLineLength[i];
			
				m_aWheelRatios[i] = CMaths::Max(m_aWheelRatios[i] - fWheelRatioAdd, 0.2f);
			}
		}

	}
}


//
//
//TweakFloat RAILTRACK_WHEEL_MOVERES_MULT	= 0.003f;
//
void CAutomobile::DoSoftGroundResistance(uint32 &nProcContFlags)
{
	if(((m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[0].GetSurfaceTypeB())==ADHESION_GROUP_SAND)
	||	(m_aWheelRatios[1]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[1].GetSurfaceTypeB())==ADHESION_GROUP_SAND)
	||	(m_aWheelRatios[2]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[2].GetSurfaceTypeB())==ADHESION_GROUP_SAND)
	||	(m_aWheelRatios[3]<BILLS_EXTENSION_LIMIT && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[3].GetSurfaceTypeB())==ADHESION_GROUP_SAND))
	&& GetModelIndex()!=MODELID_CAR_RCBANDIT && GetModelIndex()!=MODELID_CAR_RHINO)
	{
		CVector vecThisSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetMatrix().GetUp())*GetMatrix().GetUp();
		float fSpeedDampingMult = vecThisSpeed.MagnitudeSqr();
		if(fSpeedDampingMult > SAND_WHEEL_BOGDOWN_LIMIT*SAND_WHEEL_BOGDOWN_LIMIT)
		{
			fSpeedDampingMult = CMaths::Sqrt(fSpeedDampingMult);
			vecThisSpeed = SAND_WHEEL_BOGDOWN_LIMIT*vecThisSpeed/fSpeedDampingMult;
			fSpeedDampingMult = SAND_WHEEL_MOVERES_MULT * MAX(0.2f, 1.0f - 2.0f*fSpeedDampingMult);
		}
		else
		{
			m_nAutomobileFlags.bIsBoggedDownInSand = true;
			fSpeedDampingMult = SAND_WHEEL_MOVERES_MULT;
		}
		
		if(hFlagsLocal &HF_OFFROAD_ABILITIES_X2)
			fSpeedDampingMult *= 0.3f;
		else if(hFlagsLocal &HF_OFFROAD_ABILITIES)
			fSpeedDampingMult *= 0.6f;
		
		if(CWeather::WetRoads > 0.2f)
			fSpeedDampingMult *= (1.2f - CWeather::WetRoads);

		// apply move resistance
		ApplyMoveForce(-fSpeedDampingMult*m_fMass*CTimer::GetTimeStep()*vecThisSpeed);
		// save the fact we're driving on sand as a flag now rather than a dedicated bool
		nProcContFlags |= PROCCONT_FLAG_DRIVING_ONSAND;
	}
	else if(((m_aWheelRatios[0]<BILLS_EXTENSION_LIMIT && m_aWheelColPoints[0].GetSurfaceTypeB()==CAR_RAILTRACK_SURFACETYPE)
	||	(m_aWheelRatios[1]<BILLS_EXTENSION_LIMIT && m_aWheelColPoints[1].GetSurfaceTypeB()==CAR_RAILTRACK_SURFACETYPE)
	||	(m_aWheelRatios[2]<BILLS_EXTENSION_LIMIT && m_aWheelColPoints[2].GetSurfaceTypeB()==CAR_RAILTRACK_SURFACETYPE)
	||	(m_aWheelRatios[3]<BILLS_EXTENSION_LIMIT && m_aWheelColPoints[3].GetSurfaceTypeB()==CAR_RAILTRACK_SURFACETYPE))
	&& GetModelIndex()!=MODELID_CAR_RCBANDIT && GetModelIndex()!=MODELID_CAR_RHINO)
	{
		// apply move resistance
		CVector vecThisSpeed = m_vecMoveSpeed - DotProduct(m_vecMoveSpeed, GetMatrix().GetUp())*GetMatrix().GetUp();
		
		//if(vecThisSpeed.MagnitudeSqr() > BIKE_RAILTRACK_MOVERES_SPEED_CAP)
		//	vecThisSpeed *= BIKE_RAILTRACK_MOVERES_SPEED_CAP / vecThisSpeed.Magnitude();
		
		ApplyMoveForce(-CVehicle::ms_fRailTrackResistance*m_fMass*CTimer::GetTimeStep()*vecThisSpeed);
	}

}


//
//
//
float ROLL_ONTO_WHEELS_FORCE = 0.0025f;
//
void CAutomobile::ProcessSuspension()
{
	// Need to store some tempory vectors
	float aWheelSpringForces[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float aWheelRatios[4] = {m_aWheelRatios[0], m_aWheelRatios[1], m_aWheelRatios[2], m_aWheelRatios[3]};
	CVector aWheelSpeeds[4];
	CVector aWheelDirections[4];
	CVector aWheelOffsets[4];
	// need the vehicles speed for burst tyres and driving in sand
	float fForwardSpeed = CMaths::Abs(DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()));
	// gonna loop through this more than once for the rhino
	int32 nLoops = 1;
	bool bRealWheel = true;
	if(GetModelIndex()==MODELID_CAR_RHINO && !CReplay::ReplayGoingOn())
		nLoops = 3;
	
	while(nLoops > 0)
	{
		for(int32 i=0; i<4; i++)
		{
			// no point doing proper stuff to get wheel directions anymore since
			// they're always pointing directly down the way
			aWheelDirections[i] = -1.0f*GetMatrix().GetUp();

			if (aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
			{
				// if not real wheel, assume this has been set at end of previous loop
				if(bRealWheel)
					aWheelOffsets[i] = m_aWheelColPoints[i].GetPosition() - GetPosition();
			}
			// not gonna used Offset if Ratio !< BILLS_EXTENSION_LIMIT, but 2 avoid compiler warning better initialise
			else
				aWheelOffsets[i] = CVector(0.0f,0.0f,0.0f);
		}

			
		// apply suspension forces
		for(int32 i=0; i<4; i++)
		{
			if (aWheelRatios[i] < BILLS_EXTENSION_LIMIT)
			{
				// suspension may require fine tuning between front and back if CM is not dead centre
				float fSusBias = pHandling->fSuspensionBias;
				if(i==1 || i==3)
					fSusBias = 1.0f - fSusBias;
				
				// centre of mass is moved to directly between the wheels for AI lowriders
				// so need to adjust the suspension balance accordingly
				if(hFlagsLocal &HF_HYDRAULICS_GEOMETRY && hFlagsLocal &HF_HYDRAULICS_INSTALLED
				&& hFlagsLocal &HF_NPC_NEUTRAL_HANDLING && GetStatus()==STATUS_PHYSICS)
					fSusBias = 0.5f;

				float fSusForce = pHandling->fSuspensionForce;
				if(hFlagsLocal &HF_HYDRAULICS_INSTALLED && CMaths::Abs(fForwardSpeed) < 0.15f)
				{
					if(hFlagsLocal &HF_HYDRAULICS_GEOMETRY)
						fSusForce *= LOWRIDER_HYDRAULIC_FORCE_MULT;
					else
						fSusForce *= HYDRAULIC_FORCE_MULT;
				}
				else if(GetModelIndex()==MODELID_CAR_VORTEX)
					fSusForce *= 1.0f + 0.25f*CMaths::Abs(((CPlane *)this)->m_fThrottleControl);

				// Don't multiply by TimeStep as this is done inside the ApplySpringCollision function
				if( CCheat::IsCheatActive(CCheat::BACKTOTHEFUTURE_CHEAT) || GetModelIndex()==MODELID_CAR_VORTEX || !bRealWheel)
					ApplySpringCollision(fSusForce, aWheelDirections[i], aWheelOffsets[i], aWheelRatios[i], fSusBias, aWheelSpringForces[i]);
				else
					ApplySpringCollisionAlt(fSusForce, aWheelDirections[i], aWheelOffsets[i], aWheelRatios[i], fSusBias, m_aWheelColPoints[i].GetNormal(), aWheelSpringForces[i]);
			}
		}
		
		for(int32 i=0; i<4; i++)
		{
			aWheelSpeeds[i] = GetSpeed(aWheelOffsets[i]);
			if (m_aGroundPhysicalPtrs[i] != NULL && bRealWheel)	// BUG: this is always NULL at the moment. Standing on moving things won't work
			{
				AssertEntityPointerValid(m_aGroundPhysicalPtrs[i]);
				aWheelSpeeds[i] -= m_aGroundPhysicalPtrs[i]->GetSpeed(m_aGroundOffsets[i]);
	// Don't clear the pointer quite yet cause WheelSpeeds need to be recalculated after damping is applied
	//			m_aGroundPhysicalPtrs[i] = NULL;
			}

			// here we have the opportunity to improve the wheel directions with respect to the ground
			// important for damping to determine magnitude of speed to be damped
			if(m_aWheelRatios[i] < BILLS_EXTENSION_LIMIT && m_aWheelColPoints[i].GetNormal().z > 0.35f && bRealWheel)
				aWheelDirections[i] = -m_aWheelColPoints[i].GetNormal();
		}
		// need to do this as a separate loop so can get WheelSpeeds for ALL wheels
		// BEFORE any damping force is applied
		for(int32 i=0; i<4; i++)
		{
			float fSusDamping = pHandling->fSuspensionDamping;
			// set lower damping values when hydraulic suspension is in use
			if(hFlagsLocal &HF_HYDRAULICS_INSTALLED && HYDRAULIC_DAMPING < fSusDamping
			&& CMaths::Abs(fForwardSpeed) < 0.15f)
				fSusDamping = HYDRAULIC_DAMPING;
		
			if(aWheelRatios[i] < HOVER_DAMPING_EXTENSION_LIMIT && !m_nAutomobileFlags.bIsMonsterTruck)
				ApplySpringDampening(fSusDamping, aWheelSpringForces[i], aWheelDirections[i], aWheelOffsets[i], aWheelSpeeds[i]);
		}
	
		nLoops--;
	
		if(GetModelIndex()==MODELID_CAR_RHINO && nLoops > 0)
		{
			CColModel &colModel = GetColModel();
			CCollisionData* pColData = colModel.GetCollisionData();
			float fDummy1, fDummy2;
			int nLineIndex[4] = {4, 7, 8, 11};
			// do outside extra set
			if(nLoops==2)
			{
				Door[REAR_LEFT_DOOR].GetExtraWheelPositions(aWheelRatios[0], fDummy1, fDummy2, aWheelRatios[1]);
				Door[REAR_RIGHT_DOOR].GetExtraWheelPositions(aWheelRatios[2], fDummy1, fDummy2, aWheelRatios[3]);
			}
			// do inside extra set
			else if(nLoops==1)
			{
				Door[REAR_LEFT_DOOR].GetExtraWheelPositions(fDummy1, aWheelRatios[0], aWheelRatios[1], fDummy2);
				Door[REAR_RIGHT_DOOR].GetExtraWheelPositions(fDummy1, aWheelRatios[2], aWheelRatios[3], fDummy2);
				nLineIndex[0] = 5;
				nLineIndex[1] = 6;
				nLineIndex[2] = 9;
				nLineIndex[3] = 10;
			}
			else
				continue;
			
			for(int i=0; i<4; i++)
			{
				aWheelOffsets[i] = pColData->m_pLineArray[nLineIndex[i]].m_vecStart + CVector(0.0f,0.0f,-aWheelRatios[i]*m_fLineLength[i]);
				aWheelOffsets[i] = Multiply3x3(GetMatrix(), aWheelOffsets[i]);
				fDummy1 = 1.0f - m_fSuspensionLength[i] / m_fLineLength[i];
				aWheelRatios[i] = (aWheelRatios[i] - fDummy1) / (1.0f - fDummy1);
			}
		}
	}
	
	float fRoll = -1111.1f;
	float fSpeedLimit = 0.02f;
	float fRollForce = ROLL_ONTO_WHEELS_FORCE;
	// be a bit more aggressive rolling AI cars back onto their wheels!
	if(GetStatus()!=STATUS_PLAYER && GetStatus()!=STATUS_PLAYER_REMOTE)
	{
		fSpeedLimit *= 2.0f;
		fRollForce *= 2.0f;
	}
	
	if(fForwardSpeed < fSpeedLimit && m_aWheelRatios[0]==1.0f && m_aWheelRatios[1]==1.0f
	&& (m_aWheelRatios[2] < 1.0f || m_aWheelRatios[3] < 1.0f))
	{
		fRoll = 1.0f;
	}
	else if(fForwardSpeed < fSpeedLimit && m_aWheelRatios[2]==1.0f && m_aWheelRatios[3]==1.0f
	&& (m_aWheelRatios[0] < 1.0f || m_aWheelRatios[1] < 1.0f))
	{
		fRoll = -1.0f;
	}
	
	if(fRoll > -1000.0f)
	{
		CVector vecTempRight = CrossProduct(GetMatrix().GetForward(), CVector(0.0f,0.0f,1.0f));
		if(CMaths::Abs(DotProduct(GetMatrix().GetRight(), vecTempRight)) < 0.6f)
		{
			CVector vecRollForce = fRoll*fRollForce*m_fTurnMass * GetMatrix().GetUp();
			CVector vecRollPoint = CModelInfo::GetBoundingBox(GetModelIndex()).GetBoundBoxMax().x * GetMatrix().GetRight();
			ApplyTurnForce(vecRollForce, vecRollPoint);
			vecRollForce = fRoll*fRollForce*m_fMass * CrossProduct(CVector(0.0f,0.0f,1.0f), GetMatrix().GetForward());
			ApplyMoveForce(vecRollForce);
		}
	}
}

#pragma optimize ( "g", off )
//
//
//
#ifdef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION
// 2 extra parameters for average wheel ratios
void CAutomobile::ProcessCarWheelPair(int32 nWheel1, int32 nWheel2, float fSteerAngle, CVector aWheelSpeeds[], CVector aWheelOffsets[], float fWheelGripRatio1, float fWheelGripRatio2, float fAdhesiveScalar, float fDriveForce, float fFrictionForce, bool bFrontWheels)
#else
// uses wheel specific wheel ratios
void CAutomobile::ProcessCarWheelPair(int32 nWheel1, int32 nWheel2, float fSteerAngle, CVector aWheelSpeeds[], CVector aWheelOffsets[], float fAdhesiveScalar, float fDriveForce, float fFrictionForce, bool bFrontWheels)
#endif
{
	bool bDriveWheels = false;
	if(bFrontWheels && mod_HandlingManager.IsFrontWheelDrive(pHandling->nVehicleID))
		bDriveWheels = true;
	else if(!bFrontWheels && mod_HandlingManager.IsRearWheelDrive(pHandling->nVehicleID))
		bDriveWheels = true;
	
#ifndef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION // !! NOT DEFINED !!
	float fTempSuspensionBias = 2.0f * pHandling->fSuspensionBias;
	if(!bFrontWheels)
		fTempSuspensionBias = 2.0f - fTempSuspensionBias;
#endif

#ifdef MAGIC_NUMBER_TRACTION

#else
	// need to do special stuff for rear wheels such as handbrake and burnouts
	if(!bFrontWheels)
	{
		if (GetIsHandbrakeOn() && !(hFlagsLocal &HF_HANDBRAKE_REARWHEELSTEER))
		{
			// special value represent handbrake friction, i.e. locked wheel
			fFrictionForce = 20000.0f;
/*TEST
			// heat tyres from skidding
			if(!(hFlagsLocal &HF_NOS_INSTALLED) && (pHandling->flags &HF_HANDBRAKE_TYREWARM)
			&& m_vecMoveSpeed.MagnitudeSqr() >  0.01f)
			{
				m_fTyreTemp += 0.005f*CTimer::GetTimeStep();
				if(m_fTyreTemp > 2.0f)	m_fTyreTemp = 2.0f;
			}
*/
		}
		else if(bDriveWheels && (nBrakesOn))// || (GetStatus()==STATUS_PLAYER && 
//		(m_aWheelState[nWheel1]==WS_SPINNING || m_aWheelState[nWheel2]==WS_SPINNING)))
//		&& m_aWheelRatios[nWheel1]<1.0f && m_aWheelRatios[nWheel2]<1.0f)
		{
			// rear wheels, gas overrides brakes (burnouts)
//			if(nBrakesOn){
				fFrictionForce = 0.0f;
				fAdhesiveScalar = 0.0f;
//			}
			float fBurnoutRotForce = BURNOUT_STEER_MULT*m_fTurnMass*CMaths::Min(1.0f, 3000.0f/m_fTurnMass);
			ApplyTurnForce(aWheelOffsets[nWheel1], m_fSteerAngle*fBurnoutRotForce*GetMatrix().GetRight());
		}
		else if(!(hFlagsLocal &HF_NOS_INSTALLED) //&& m_fTyreTemp > 1.0f 
		&& mod_HandlingManager.IsRearWheelDrive(pHandling->nVehicleID))
		{
			fAdhesiveScalar *= m_fTyreTemp;
		}
	}
#endif // not MAGIC_NUMBER_TRACTION
		
	if(m_aWheelCounts[nWheel1]>0.0f || m_aWheelCounts[nWheel2] > 0.0f)
	{
		float fSin, fCos;
		float fThrust, fAdhesiveLimit;
		float BRAKE_BIAS, TRACTION_BIAS;
		CVector vecForward, vecRight, vecTempStore;
		bool bSteeringWheels = false;
		tWheelState fTempWheelState;
		
		if(fSteerAngle > -100.0f)
		{
			bSteeringWheels = true;
			fSin = CMaths::Sin(fSteerAngle);
			fCos = CMaths::Cos(fSteerAngle);
		}
		
		// if NeutralHandling flag is set - grip and braking to be equal front and back
		bool bNeutralHandling = (GetStatus()!=STATUS_PLAYER && GetStatus()!=STATUS_PLAYER_REMOTE && (hFlagsLocal &HF_NPC_NEUTRAL_HANDLING));
		if(bFrontWheels)
		{
			BRAKE_BIAS = (bNeutralHandling ? 1.0f : 2.0f * pHandling->fBrakeBias);
			TRACTION_BIAS = (bNeutralHandling ? 1.0f : 2.0f * pHandling->fTractionBias);
		}
		else
		{
			BRAKE_BIAS = (bNeutralHandling ? 1.0f : 2.0f - 2.0f*pHandling->fBrakeBias);
			TRACTION_BIAS = (bNeutralHandling ? 1.0f : 2.0f - 2.0f*pHandling->fTractionBias);
		}

		if (m_aWheelCounts[nWheel1] > 0.0f)
		{
			if(bDriveWheels)
				fThrust = fDriveForce;
			else
				fThrust = 0.0f;

			vecForward = GetMatrix().GetForward();
			float fAlongNormal = DotProduct(vecForward, m_aWheelColPoints[nWheel1].GetNormal());
			vecForward -= fAlongNormal * m_aWheelColPoints[nWheel1].GetNormal();
			vecForward.Normalise();
			vecRight = CrossProduct(vecForward, m_aWheelColPoints[nWheel1].GetNormal());
			vecRight.Normalise();
			
			if(bSteeringWheels && GetModelIndex()!=MODELID_PLANE_HARRIER)
			{
				vecTempStore = fCos * vecForward - fSin * vecRight;
				vecRight = fSin * vecForward + fCos * vecRight;
				vecForward = vecTempStore;
			}

			m_aWheelColPoints[nWheel1].SetSurfaceTypeA(SURFACE_TYPE_WHEELBASE);
		 	fAdhesiveLimit = fAdhesiveScalar * g_surfaceInfos.GetAdhesiveLimit(m_aWheelColPoints[nWheel1]);
		 	if(TreatAsPlayerForCollisions())
		 	{
		 		fAdhesiveLimit *= g_surfaceInfos.GetWetMultiplier(m_aWheelColPoints[nWheel1].GetSurfaceTypeB());
#ifdef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION
		 		fAdhesiveLimit *= CMaths::Min(2.0f, (1.0f - fWheelGripRatio1)*4.0f*pHandling->fSuspensionForce);
#else
				fAdhesiveLimit *= CMaths::Min(2.0f, (1.0f - m_aWheelRatios[nWheel1])*4.0f*pHandling->fSuspensionForce*fTempSuspensionBias);
#endif
			 	if((hFlagsLocal &HF_OFFROAD_ABILITIES_X2) && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[nWheel1].GetSurfaceTypeB())>ADHESION_GROUP_ROAD)
			 		fAdhesiveLimit *= 1.4f;
			 	else if((hFlagsLocal &HF_OFFROAD_ABILITIES) && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[nWheel1].GetSurfaceTypeB())>ADHESION_GROUP_ROAD)
			 		fAdhesiveLimit *= 1.15f;
			}
			fTempWheelState = m_aWheelState[nWheel1];	// should get previous wheel state
			// damage effect
			if (Damage.GetWheelStatus(nWheel1) == DT_WHEEL_BURST)
				ProcessWheel(vecForward, vecRight, aWheelSpeeds[nWheel1], aWheelOffsets[nWheel1], nNoOfContactWheels, fThrust, fFrictionForce*BRAKE_BIAS, fAdhesiveLimit*Damage.fWheelDamageEffect*TRACTION_BIAS, nWheel1, &m_aWheelLongitudinalSlip[nWheel1], &fTempWheelState, DT_WHEEL_BURST);
			else
				ProcessWheel(vecForward, vecRight, aWheelSpeeds[nWheel1], aWheelOffsets[nWheel1], nNoOfContactWheels, fThrust, fFrictionForce*BRAKE_BIAS, fAdhesiveLimit*TRACTION_BIAS, nWheel1, &m_aWheelLongitudinalSlip[nWheel1], &fTempWheelState, DT_WHEEL_INTACT);
			
			// don't want to wheelspin going backwards
			if(bDriveWheels && m_fGasPedal < 0 && fTempWheelState==WS_SPINNING)
				m_aWheelState[nWheel1] = WS_ROLLING;
			else
				m_aWheelState[nWheel1] = fTempWheelState;
		}
		
		if (m_aWheelCounts[nWheel2] > 0.0f)
		{
			if(bDriveWheels)
				fThrust = fDriveForce;
			else
				fThrust = 0.0f;

			vecForward = GetMatrix().GetForward();
			float fAlongNormal = DotProduct(vecForward, m_aWheelColPoints[nWheel2].GetNormal());
			vecForward -= fAlongNormal * m_aWheelColPoints[nWheel2].GetNormal();
			vecForward.Normalise();
			vecRight = CrossProduct(vecForward, m_aWheelColPoints[nWheel2].GetNormal());
			vecRight.Normalise();

			if(bSteeringWheels)
			{
				vecTempStore = fCos * vecForward - fSin * vecRight;
				vecRight = fSin * vecForward + fCos * vecRight;
				vecForward = vecTempStore;
			}

			m_aWheelColPoints[nWheel2].SetSurfaceTypeA(SURFACE_TYPE_WHEELBASE);
		 	fAdhesiveLimit = fAdhesiveScalar * g_surfaceInfos.GetAdhesiveLimit(m_aWheelColPoints[nWheel2]);

		 	if(TreatAsPlayerForCollisions())
		 	{
		 		fAdhesiveLimit *= g_surfaceInfos.GetWetMultiplier(m_aWheelColPoints[nWheel2].GetSurfaceTypeB());
#ifdef AVERAGE_FRONT_REAR_RATIOS_FOR_TRACTION
		 		fAdhesiveLimit *= MIN(2.0f, (1.0f - fWheelGripRatio2)*4.0f*pHandling->fSuspensionForce);
#else
		 		fAdhesiveLimit *= MIN(2.0f, (1.0f - m_aWheelRatios[nWheel2])*4.0f*pHandling->fSuspensionForce*fTempSuspensionBias);
#endif
			 	if((hFlagsLocal &HF_OFFROAD_ABILITIES_X2) && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[nWheel2].GetSurfaceTypeB())>ADHESION_GROUP_ROAD)
			 		fAdhesiveLimit *= 1.4f;
			 	else if((hFlagsLocal &HF_OFFROAD_ABILITIES) && g_surfaceInfos.GetAdhesionGroup(m_aWheelColPoints[nWheel2].GetSurfaceTypeB())>ADHESION_GROUP_ROAD)
			 		fAdhesiveLimit *= 1.15f;
			}
			fTempWheelState = m_aWheelState[nWheel2];	// should get previous wheel state
			// damage effect
			if (Damage.GetWheelStatus(nWheel2) == DT_WHEEL_BURST)
				ProcessWheel(vecForward, vecRight, aWheelSpeeds[nWheel2], aWheelOffsets[nWheel2], nNoOfContactWheels, fThrust, fFrictionForce*BRAKE_BIAS, fAdhesiveLimit*Damage.fWheelDamageEffect*TRACTION_BIAS, nWheel2, &m_aWheelLongitudinalSlip[nWheel2], &fTempWheelState, DT_WHEEL_BURST);
			else
				ProcessWheel(vecForward, vecRight, aWheelSpeeds[nWheel2], aWheelOffsets[nWheel2], nNoOfContactWheels, fThrust, fFrictionForce*BRAKE_BIAS, fAdhesiveLimit*TRACTION_BIAS, nWheel2, &m_aWheelLongitudinalSlip[nWheel2], &fTempWheelState, DT_WHEEL_INTACT);

			// don't want to wheelspin going backwards
			if(bDriveWheels && m_fGasPedal < 0 && fTempWheelState==WS_SPINNING)
				m_aWheelState[nWheel2] = WS_ROLLING;
			else
				m_aWheelState[nWheel2] = fTempWheelState;
		}

	}
	
	// need to process some more stuff now specific to rear wheels
	if(!bFrontWheels && !(hFlagsLocal &HF_NOS_INSTALLED))
	{
		if(nBrakesOn && bDriveWheels && (m_aWheelState[1]==WS_SPINNING || m_aWheelState[3]==WS_SPINNING))
		{
			// rear wheels spinning burnout -> increase tyre temp
			m_fTyreTemp += 0.001f * CTimer::GetTimeStep();
			if(m_fTyreTemp > 3.0f)	m_fTyreTemp = 3.0f;
		}
		else if(m_fTyreTemp > 1.0f)
		{
			// not spinning, cool down
			m_fTyreTemp = 1.0f + (m_fTyreTemp - 1.0f)*CMaths::Pow( 0.995f, CTimer::GetTimeStep());
		}
	}
	
#ifdef MAGIC_NUMBER_TRACTION
	if( m_aWheelCounts[nWheel1] <= 0.0f)
	{
		m_aWheelLongitudinalSlip[nWheel1] = 0.0f;
	}
	if( m_aWheelCounts[nWheel2] <= 0.0f)
	{
		m_aWheelLongitudinalSlip[nWheel2] = 0.0f;
	}
#else
	// not for choppers
	if( !(pHandling->mFlags &MF_IS_HELI) )
	{
		// now do rotations for front wheels off the ground
		if( m_aWheelCounts[nWheel1] <= 0.0f)
		{
			if(bDriveWheels && fDriveForce)
			{
				// if wheel is off the ground, but there is drive through it, want to spin wheel
				if(fDriveForce > 0){
					if(m_aWheelAngularVelocity[nWheel1]<1.0f)
						m_aWheelAngularVelocity[nWheel1] -= 0.1f;
				}
				else{
					if(m_aWheelAngularVelocity[nWheel1]>-1.0f)
						m_aWheelAngularVelocity[nWheel1] += 0.05f;
				}
				
				m_aWheelPitchAngles[nWheel1] += m_aWheelAngularVelocity[nWheel1]*CTimer::GetTimeStep();
			}
			else
			{
				m_aWheelAngularVelocity[nWheel1] *= 0.95f;
				m_aWheelPitchAngles[nWheel1] += m_aWheelAngularVelocity[nWheel1]*CTimer::GetTimeStep();
			}	
		}
	
		if( m_aWheelCounts[nWheel2] <= 0.0f)
		{
			if(bDriveWheels && fDriveForce)
			{
				// if wheel is off the ground, but there is drive through it, want to spin wheel
				if(fDriveForce > 0){
					if(m_aWheelAngularVelocity[nWheel2]<1.0f)
						m_aWheelAngularVelocity[nWheel2] -= 0.1f;
				}
				else{
					if(m_aWheelAngularVelocity[nWheel2]>-1.0f)
						m_aWheelAngularVelocity[nWheel2] += 0.05f;
				}
			
				m_aWheelPitchAngles[nWheel2] += m_aWheelAngularVelocity[nWheel2]*CTimer::GetTimeStep();
			}
			else
			{
				m_aWheelAngularVelocity[nWheel2] *= 0.95f;
				m_aWheelPitchAngles[nWheel2] += m_aWheelAngularVelocity[nWheel2]*CTimer::GetTimeStep();
			}
		}
	}
#endif	
}
#pragma optimize ("", on)

float CAutomobile::GetCarRoll()
{
	float fRoll = GetMatrix().GetRight().Magnitude2D();
 
	if(GetMatrix().GetUp().z < 0.0f)      fRoll *= -1.0f; 
	fRoll = CMaths::ATan2(GetMatrix().GetRight().z, fRoll); 
	fRoll = RADTODEG(fRoll); 
	
	return fRoll;
}

float CAutomobile::GetCarPitch()
{
	float fPitch = GetMatrix().GetForward().Magnitude2D();
	
	if(GetMatrix().GetUp().z < 0.0f)	fPitch *= -1.0f;
	fPitch = CMaths::ATan2(GetMatrix().GetForward().z, fPitch);
	
	return fPitch;
}

//
// FUNCTION: FindWheelWidth
// PURPOSE:  Finds out what the wheel width of this vehicle is (used by the skidmarks)
//
TweakFloat KART_WHEEL_SKID_MULT = 1.5f;
//
float CAutomobile::FindWheelWidth(bool bRear)
{
	float fWidth = STD_CAR_WHEEL_WIDTH;
	fWidth *= ((CVehicleModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex))->GetWheelScale(!bRear) / STD_CAR_WHEEL_SCALE;
	
	if(bRear && hFlagsLocal &0x0000F000)
	{
		if(hFlagsLocal &HF_WHEEL_R_NARROW_X2)
			fWidth *= 0.65f;
		else if(hFlagsLocal &HF_WHEEL_R_NARROW)
			fWidth *= 0.8f;
		else if(hFlagsLocal &HF_WHEEL_R_WIDE)
			fWidth *= 1.1f;
		else if(hFlagsLocal &HF_WHEEL_R_WIDE_X2)
			fWidth *= 1.25f;
	}
	else if(!bRear && hFlagsLocal &0x00000F00 )
	{
		if(hFlagsLocal &HF_WHEEL_F_NARROW_X2)
			fWidth *= 0.65f;
		else if(hFlagsLocal &HF_WHEEL_F_NARROW)
			fWidth *= 0.8f;
		else if(hFlagsLocal &HF_WHEEL_F_WIDE)
			fWidth *= 1.1f;
		else if(hFlagsLocal &HF_WHEEL_F_WIDE_X2)
			fWidth *= 1.25f;
	}

	if(GetModelIndex()==MODELID_CAR_KART)
		fWidth *= KART_WHEEL_SKID_MULT;

	return fWidth;
}






/*
//
#define JIMMY_TRANSFORM_WHEEL_END_TIME		(100.0f)
#define JIMMY_TRANSFORM_COVER_START_TIME	(90.0f)
#define JIMMY_TRANSFORM_COVER_END_TIME		(190.0f)
#define JIMMY_TRANSFORM_FIN_START_TIME		(200.0f)
float JIMMY_TRANSFORM_FIN_SLIDE_DIST = 0.3f;
float JIMMY_TRANSFORM_WHEEL_SLIDE_DIST = 0.7f;
//
CVector vecLotusRocketPos(0.7f, 2.5f, 0.0f);
CVector vecLotusGrenadePos(0.0f, -1.9f, 0.1f);
//
void CAutomobile::AnimateJimmyTransform()
{
	RwObject* pRwObject = NULL;
	CColModel *pColModel = &GetColModel();
	CVehicleModelInfo *pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(this->GetModelIndex());
	bool bSaveInitialPositions = false;
	if(m_fBasePositions[0] == -99999.9f)
	{
		bSaveInitialPositions = true;
	}
	
	// Rotate NumberPlate
	if(CPad::GetPad(0)->ShockButtonRJustDown())
	{
		if(PropRotate == 0.0f)
			PropRotate = 0.001f;
		else if(PropRotate == 1.0f)
			PropRotate = -1.0f;
	}
	else
	{
		if(PropRotate > 0.0f && PropRotate < 1.0f)
		{
			PropRotate += 0.06f*CTimer::GetTimeStep();
			if(PropRotate > 1.0f)	PropRotate = 1.0f;
		}
		else if(PropRotate < 0.0f)
		{
			PropRotate += 0.06f*CTimer::GetTimeStep();
			if(PropRotate > 0.0f)	PropRotate = 0.0f;
		}
	}
	
	// fire correct weapon
	if(CPad::GetPad(0)->CarGunJustDown() && CTimer::GetTimeInMilliseconds() > TimeOfLastShotFired + 400)
	{
		if(PropRotate == 1.0f)
		{
			CWeapon tempWeapon(WEAPONTYPE_GRENADE, 100);
			
			CVector vecFirePos = vecLotusGrenadePos;
			vecFirePos = GetMatrix()*vecFirePos + MAX(0.0f, DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()))*GetMatrix().GetForward()*CTimer::GetTimeStep();
			tempWeapon.FireProjectile(this, &vecFirePos, 0.5f);

			DMAudio.PlayOneShot(((CPhysical*)(this))->AudioHandle, AE_FIRE_WEAPON, 0.0f);
			TimeOfLastShotFired = CTimer::GetTimeInMilliseconds();
		}
		else
		{
			CWeapon tempWeapon(WEAPONTYPE_ROCKETLAUNCHER, 100);
			
			CVector vecFirePos = vecLotusRocketPos;
			vecFirePos = GetMatrix()*vecFirePos + MAX(0.0f, DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()))*GetMatrix().GetForward()*CTimer::GetTimeStep();
			tempWeapon.FireProjectile(this, &vecFirePos, 0);
			
			vecFirePos = vecLotusRocketPos;
			vecFirePos.x = -vecFirePos.x;
			vecFirePos = GetMatrix()*vecFirePos + MAX(0.0f, DotProduct(m_vecMoveSpeed, GetMatrix().GetForward()))*GetMatrix().GetForward()*CTimer::GetTimeStep();
			tempWeapon.FireProjectile(this, &vecFirePos, 0);

			DMAudio.PlayOneShot(((CPhysical*)(this))->AudioHandle, AE_FIRE_WEAPON, 0.0f);
			TimeOfLastShotFired = CTimer::GetTimeInMilliseconds();
		}
	}
	
	// don't want to get stuck midway between transformation positions
	if(m_nTransformState==0 && m_fTransformPosition!=0.0f
	&& m_fTransformPosition!=JIMMY_TRANSFORM_TOTAL_TIME)
	{
		m_nTransformState = -1;
	}
	
	// need to save old position to see if crosses start or end of sections
	float fOldPosition = m_fTransformPosition;
	if(m_nTransformState==1)
	{
		m_fTransformPosition += CTimer::GetTimeStep();
		if(m_fTransformPosition > JIMMY_TRANSFORM_TOTAL_TIME)
		{
			m_fTransformPosition = JIMMY_TRANSFORM_TOTAL_TIME;
			m_nTransformState = 0;
		}
		
		// if pass end of wheel anim, stop rendering wheels
		if(m_fTransformPosition > JIMMY_TRANSFORM_WHEEL_END_TIME && fOldPosition < JIMMY_TRANSFORM_WHEEL_END_TIME)
		{
			// don't render wheels
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_LR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_RF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_RR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		}
		
		// if pass start of cover anim, start rendering covers
		if(m_fTransformPosition > JIMMY_TRANSFORM_COVER_START_TIME && fOldPosition < JIMMY_TRANSFORM_COVER_START_TIME)
		{
			// render covers
			pRwObject = GetFirstObject(m_aCarNodes[CAR_COVER_L]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_COVER_R]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
		}
		
		// if pass start of fin anim, start rendering fins
		if(m_fTransformPosition > JIMMY_TRANSFORM_FIN_START_TIME && fOldPosition < JIMMY_TRANSFORM_FIN_START_TIME)
		{
			// render fins
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_LF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_LR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_RF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_RR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
		}
	}
	else if(m_nTransformState==-1)
	{
		m_fTransformPosition -= CTimer::GetTimeStep();
		if(m_fTransformPosition < 0.0f)
		{
			m_fTransformPosition = 0.0f;
			m_nTransformState = 0;
		}
		
		// if pass end of wheel anim, stop rendering wheels
		if(m_fTransformPosition < JIMMY_TRANSFORM_WHEEL_END_TIME && fOldPosition > JIMMY_TRANSFORM_WHEEL_END_TIME)
		{
			// render wheels
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_LF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_LR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_RF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_WHEEL_RR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, rpATOMICRENDER);
		}
		
		// if pass start of cover anim, stop rendering covers
		if(m_fTransformPosition < JIMMY_TRANSFORM_COVER_START_TIME && fOldPosition > JIMMY_TRANSFORM_COVER_START_TIME)
		{
			// don't render covers
			pRwObject = GetFirstObject(m_aCarNodes[CAR_COVER_L]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_COVER_R]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		}
		
		// if pass start of fin anim, stop rendering fins
		if(m_fTransformPosition < JIMMY_TRANSFORM_FIN_START_TIME && fOldPosition > JIMMY_TRANSFORM_FIN_START_TIME)
		{
			// don't render fins
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_LF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_LR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_RF]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
			pRwObject = GetFirstObject(m_aCarNodes[CAR_FIN_RR]);
			RpAtomicSetFlags((RpAtomic*)pRwObject, 0);
		}
	}
	
	// now set positions of components based on TransformationPosition
	CMatrix matrix;
	CVector posn;
	float angle;

	// wheels
	if(m_fTransformPosition > 0.0f)
	{
		angle = m_fTransformPosition / JIMMY_TRANSFORM_WHEEL_END_TIME;
		if(angle < 0.0f) angle = 0.0f;
		else if(angle > 1.0f) angle = 1.0f;
		// want a second angle to slide the wheels in the way later in sequence
		float angle2 = 2.0f*angle - 1.0f;
		if(angle2 < 0.0f)	angle2 = 0.0f;

		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LF]));
		posn = matrix.GetTranslate();
		posn.z = pColModel->m_pLineArray[0].m_vecStart.z - (1.0f - angle)*m_fSuspensionLength[0];
		posn.x = pColModel->m_pLineArray[0].m_vecStart.x + angle2*JIMMY_TRANSFORM_WHEEL_SLIDE_DIST;
		matrix.SetRotateY(PI + angle*HALF_PI);
		matrix.Translate(posn);
		matrix.Scale(pModelInfo->GetWheelScale());
		matrix.UpdateRW();
		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_LR]));
		posn = matrix.GetTranslate();
		posn.z = pColModel->m_pLineArray[1].m_vecStart.z - (1.0f - angle)*m_fSuspensionLength[1];
		posn.x = pColModel->m_pLineArray[1].m_vecStart.x + angle2*JIMMY_TRANSFORM_WHEEL_SLIDE_DIST;
		matrix.SetRotateY(PI + angle*HALF_PI);
		matrix.Translate(posn);
		matrix.Scale(pModelInfo->GetWheelScale());
		matrix.UpdateRW();
		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RF]));
		posn = matrix.GetTranslate();
		posn.z = pColModel->m_pLineArray[2].m_vecStart.z - (1.0f - angle)*m_fSuspensionLength[2];
		posn.x = pColModel->m_pLineArray[2].m_vecStart.x - angle2*JIMMY_TRANSFORM_WHEEL_SLIDE_DIST;
		matrix.SetRotateY(-angle*HALF_PI);
		matrix.Translate(posn);
		matrix.Scale(pModelInfo->GetWheelScale());
		matrix.UpdateRW();
		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WHEEL_RR]));
		posn = matrix.GetTranslate();
		posn.z = pColModel->m_pLineArray[3].m_vecStart.z - (1.0f - angle)*m_fSuspensionLength[3];
		posn.x = pColModel->m_pLineArray[3].m_vecStart.x - angle2*JIMMY_TRANSFORM_WHEEL_SLIDE_DIST;
		matrix.SetRotateY(-angle*HALF_PI);
		matrix.Translate(posn);
		matrix.Scale(pModelInfo->GetWheelScale());
		matrix.UpdateRW();
	}

	// covers
	{
		angle = (m_fTransformPosition - JIMMY_TRANSFORM_COVER_START_TIME) / (JIMMY_TRANSFORM_COVER_END_TIME - JIMMY_TRANSFORM_COVER_START_TIME);
		if(angle < 0.0f) angle = 0.0f;
		else if(angle > 1.0f) angle = 1.0f;
		angle = 1.0f - angle;
		// left cover
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_COVER_L]));
		posn = matrix.GetTranslate();
		matrix.SetRotateY(-HALF_PI*angle);
		matrix.Translate(posn);
		matrix.UpdateRW();
		// right cover
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_COVER_R]));
		posn = matrix.GetTranslate();
		matrix.SetRotateY(HALF_PI*angle);
		matrix.Translate(posn);
		matrix.UpdateRW();
	}
	
	
	// fins
	{
		angle = (m_fTransformPosition - JIMMY_TRANSFORM_FIN_START_TIME) / (JIMMY_TRANSFORM_TOTAL_TIME - JIMMY_TRANSFORM_FIN_START_TIME);
		if(angle < 0.0f) angle = 0.0f;
		else if(angle > 1.0f) angle = 1.0f;
		angle = 1.0f - angle;

		// left front fin		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_FIN_LF]));
		posn = matrix.GetTranslate();
		// need to save inital x posn of fins so we can slide them into the car
		if(bSaveInitialPositions)
			m_fBasePositions[0] = posn.x;
		
		posn.x = m_fBasePositions[0] + angle * JIMMY_TRANSFORM_FIN_SLIDE_DIST;
		matrix.SetRotateX(-m_fSteerAngle + CPad::GetPad(0)->GetSteeringUpDown() / 256.0f);
		matrix.Translate(posn);
		matrix.UpdateRW();
		
		// left rear fin		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_FIN_LR]));
		posn = matrix.GetTranslate();
		posn.x = m_fBasePositions[0] + angle * JIMMY_TRANSFORM_FIN_SLIDE_DIST;
		matrix.SetRotateX(-m_fSteerAngle - CPad::GetPad(0)->GetSteeringUpDown() / 256.0f);
		matrix.Translate(posn);
		matrix.UpdateRW();

		// right front fin		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_FIN_RF]));
		posn = matrix.GetTranslate();
		if(bSaveInitialPositions)
			m_fBasePositions[1] = posn.x;

		posn.x = m_fBasePositions[1] - angle * JIMMY_TRANSFORM_FIN_SLIDE_DIST;
		matrix.SetRotateX(m_fSteerAngle + CPad::GetPad(0)->GetSteeringUpDown() / 256.0f);
		matrix.Translate(posn);
		matrix.UpdateRW();

		// right rear fin		
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_FIN_RR]));
		posn = matrix.GetTranslate();
		posn.x = m_fBasePositions[1] - angle * JIMMY_TRANSFORM_FIN_SLIDE_DIST;
		matrix.SetRotateX(m_fSteerAngle - CPad::GetPad(0)->GetSteeringUpDown() / 256.0f);
		matrix.Translate(posn);
		matrix.UpdateRW();
	}
	
	// Number Plate
	if(m_aCarNodes[CAR_WING_LR]!=NULL)
	{
		matrix.Attach(RwFrameGetMatrix(m_aCarNodes[CAR_WING_LR]));
		posn = matrix.GetTranslate();
		matrix.SetRotateX(CMaths::Abs(PropRotate)*PI);
		matrix.Translate(posn);
		matrix.UpdateRW();
	}
}
//
*/

void CAutomobile::DoHeliDustEffect(float power, float radiusMult)
{
	if ((GetVehicleType()==VEHICLE_TYPE_HELI && m_aWheelAngularVelocity[1] > 0.75f*MIN_ROT_SPEED_HELI_CONTROL) ||
		(GetVehicleType()==VEHICLE_TYPE_PLANE))
	{
		bool bGround = false;
		float waterlevel = -1000.0f;

		CColPoint testColPoint;
		CEntity* testEntity;
		bool returnVal = CWorld::ProcessVerticalLine(GetPosition(), -1000.0f, testColPoint, testEntity, true, false, false, false, false);
		float landlevel = testColPoint.GetPosition().z;
		uint8 surfaceType = testColPoint.GetSurfaceTypeB();
		bool8 isSand = g_surfaceInfos.IsSand(surfaceType);		
		
		bool8 isWater = false;
		if (CWaterLevel::GetWaterLevel(GetPosition().x, GetPosition().y, GetPosition().z, &waterlevel, false))
		{
			if (waterlevel > landlevel)
			{
				landlevel = waterlevel;
				isWater = true;
			}
		}
			
		float fRadius = 30.0f;
		if(GetModelIndex()==MODELID_CAR_RCGOBLIN || GetModelIndex()==MODELID_CAR_RCCOPTER)
			fRadius = 3.0f;

		fRadius *= radiusMult;

		float distToCamSqr = (GetPosition()-TheCamera.GetPosition()).MagnitudeSqr();
		if ((distToCamSqr< 50.0f*50.0f) && ((GetPosition().z - landlevel) < fRadius))
		{
			// create adanger event if heli is low
			if (CTimer::m_FrameCounter%20 == 0)
			{
				float fleeRange = 20.0f;
				if (GetModelIndex()==MODELID_PLANE_HARRIER)
				{
					fleeRange = 30.0f;
				}
			
				if (GetPosition().z - landlevel < fleeRange)
				{
					CEventDanger event(this, fleeRange);
					GetEventGlobalGroup()->Add(event);
				}
			}
		
			if (m_fxSysHeliDust == NULL)
			{	
				// create the system
				CVector offsetPos(0.0f, 0.0f, 0.0f);
				m_fxSysHeliDust = g_fxMan.CreateFxSystem("heli_dust", (RwV3d*)&offsetPos, NULL, true);
				if (m_fxSysHeliDust)
				{
					m_fxSysHeliDust->SetLocalParticles(true);
					m_fxSysHeliDust->Play();
				}
			}
		
			if(m_fxSysHeliDust)
			{
				float playTime = (fRadius - (GetPosition().z - landlevel)) / fRadius;
				playTime *= power;
				playTime *= playTime;
				
				
				if (playTime>m_heliDustRatio)
				{
					m_heliDustRatio = MIN(m_heliDustRatio+0.04f, playTime);
				}
				else if (playTime<m_heliDustRatio)
				{
					m_heliDustRatio = MAX(m_heliDustRatio-0.04f, playTime);
				}
				
				m_fxSysHeliDust->SetConstTime(true, m_heliDustRatio);
				
				CVector effectPos = GetPosition();
				effectPos.z = landlevel;
				
				m_fxSysHeliDust->SetOffsetPos(&effectPos);
				
				if (isWater)
				{
					m_fxSysHeliDust->EnablePrim(0, false);
					m_fxSysHeliDust->EnablePrim(1, false);
					m_fxSysHeliDust->EnablePrim(2, true);
				}
				else if (isSand)
				{
					m_fxSysHeliDust->EnablePrim(0, false);
					m_fxSysHeliDust->EnablePrim(1, true);
					m_fxSysHeliDust->EnablePrim(2, false);
				}
				else
				{
					m_fxSysHeliDust->EnablePrim(0, true);
					m_fxSysHeliDust->EnablePrim(1, false);
					m_fxSysHeliDust->EnablePrim(2, false);
				}
			}
		}
		else
		{
			if (m_fxSysHeliDust)	
			{
				// kill the system
				m_fxSysHeliDust->Kill();
				m_fxSysHeliDust = NULL;
				m_heliDustRatio = 0.0f;
			}
		}
	}
	else
	{		
		if (m_fxSysHeliDust)	
		{
			// kill the system
			m_fxSysHeliDust->Kill();
			m_fxSysHeliDust = NULL;
			m_heliDustRatio = 0.0f;
		}
	}
}


bool8 CAutomobile::IsInAir()
{
	if (m_nPhysicalFlags.bCoorsFrozenByScript)
	{
		return true;
	}

	if (m_nPhysicalFlags.bIsInWater || 
		m_aWheelRatios[0]<1.0f || 
		m_aWheelRatios[1]<1.0f || 
		m_aWheelRatios[2]<1.0f || 
		m_aWheelRatios[3]<1.0f ||
		(m_vecMoveSpeed.x==0.0f && m_vecMoveSpeed.y==0.0f && m_vecMoveSpeed.z==0.0f))
	{
		return false;
	}
	
	return true;
}		
