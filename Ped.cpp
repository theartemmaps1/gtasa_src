// Title	:	Ped.cpp / AKA ADAM.CPP
// Author	:	Gordon Speirs
// Started	:	02/09/99

// This file contains code common to all ped types - the basic ped behaviours,
// animation, movement and other stuff like that.

// To do : Need to look at BuildPedLists - is it really needed?
// Too many timers and other shit like that in the ped class - need to tidy everything up
// RestorePreviousState needs to be expanded to cover all (or most) states and the other
// behaviours need to make sure they use RestorePreviousState
// Scanning stuff may be redundant now, could use event list instead, should be easier
// and more efficient
//
//
//
//
//
//
//
//
//

#include "Pools.h"
#include "Ped.h"
#include "PedAI.h"
#include "pedIntelligence.h"
#include "events.h"
#include "PedType.h"
#include "CivilianPed.h" 
#include "PlayerPed.h"
#include "DummyPed.h"
#include "Population.h"
#include "Physical.h"
#include "Maths.h"
#include "Main.h"
#include "General.h"
#include "ModelInfo.h"
#include "AtomicModelInfo.h"
#include "ModelIndices.h"
#include "World.h"
#include "Game.h"
#include "Range2D.h"
#include "Camera.h"
#include "Collision.h"
#include "Pad.h"
#include "PathFind.h"
#include "PedPath.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "WeaponEffects.h"
#include "Timer.h"
#include "PtrList.h"
#include "Debug.h"
#include "Profile.h"
#include "shadows.h"
#include "globals.h"
#include "Phones.h"
#include "draw.h"
#include "Pools.h"
#include "RpAnimBlend.h"
#include "RpAnimBlendClump.h"
#include "AnimManager.h"
#include "Wanted.h"
#include "HierarchyIds.h"
#include "pedplacement.h"
#include "Carctrl.h"
#include "Carai.h"
#include "VehicleModelInfo.h"
#include "AnimBlendHierarchy.h"
//#include "PedRoutes.h"
#include "Automobile.h"
#include "TrafficLights.h"
#include "Pools.h"
#include "floater.h"
#include "pickups.h"
#include "timecycle.h"
#include "waterlevel.h"
#include "particle.h"
#include "VisibilityPlugins.h"
#include "object.h"
#include "radar.h"
#include "weather.h"
#include "script.h"
#include "ParticleObject.h"		// CParticleObject::AddObject()....
#include "ZoneCull.h"			// CCullZones::CamNoRain(), CCullZones::PlayerNoRain()...
#include "console.h"
#include "replay.h"
#include "tempcolmodels.h"
#include "garages.h"
#include "ColTriangle.h"

#ifndef ANDROID
#include <limits>
#endif

#ifdef GTA_NETWORK
	#include "gamenet.h"
	#include "netgames.h"
#endif
#include "PedAttractor.h"
#include "WindModifiers.h"
#include "streaming.h"
#include "surfacetable.h"
#include "Clock.h"

#include "PedIntelligence.h"
#include "TaskCarUtils.h"
#include "TaskCar.h"
#include "Task.h"
#include "TaskGoto.h"
#include "TaskAttack.h"
#include "TaskTypes.h"
#include "TaskBasic.h"
#include "TaskSecondary.h"
#include "TaskPlayer.h"
#include "TaskJumpFall.h"
#include "Events.h"
#include "localisation.h"
#include "timer.h"
#include "VehicleModelInfo.h"
#include "MemInfo.h"
#include "Bmx.h"
#include "stats.h"
#include "fx.h"
#include "taskManager.h"
#include "specialfx.h"
#include "conversations.h"
#include "ObjectData.h"
#include "cover.h"
#include "PostEffects.h"
#include "EulerAng.h"
#include "globalspeechcontexts.h"
#include "cheat.h"

#ifdef USE_AUDIOENGINE
#include "AudioEngine.h"
#include "GameAudioEvents.h"
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

#ifndef FINAL
	#include "VarConsole.h"
	#include "pedDebugVisualiser.h"
#endif

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
#include "DMAudio.h"
#include "audioman.h"
#endif //USE_AUDIOENGINE

#define RECIPROCAL_256 (0.004f)
#define CAR_MINIMUM_VALUE_WORTH_STEALING	2000
#define CHAT_DISTANCE (1.8f)
// movement inertia settings
#define PED_MAXWALKINGSPEEDCHANGE (0.01f)
#define PED_MAXFLOATINGSPEEDCHANGE (0.002f)

#define PED_GROUNDCOLLINE_OFFSET (-0.25f)
#define PED_GROUNDCOLLINE_OFFSET_TIMESTEP (-0.15f)
#define PED_INTHEAIROFFSET (PED_GROUNDOFFSET + 0.5f)
#define PED_FALLINGOFFSET (PED_GROUNDOFFSET + 3.0f)

// ranges (to pursue ped on foot, find a car, etc)
//#define PED_CHASE_ON_FOOT_RANGE 15.0f
//#define PED_RESUME_CHASE_ON_FOOT_RANGE 8.0f
#define PED_DEFAULT_ARRIVE_RANGE 1.5f

#define PED_TIRED_MAXHEALTH	20

#define PED_NOTICE_DEAD_PED_RANGE 20.0f
#define PED_CAN_SEE_RANGE 40.0f
#define PED_RESTORE_WANDER_NODE_RANGE 7.0f
// time spans to apply collision avoidance after a ped collision
#define COLLISION_AVOID_TIME_FLEE 2500
#define COLLISION_AVOID_TIME_SEEK 5000
#define PED_MAX_STATIC_FRAMES 50

#define HOOKER_FEE 100

//#define PATH_FIND_AROUND_BUILDINGS_ON_FOLLOW_PATH
#define CLOSEST_NODE_SEARCH_SIZE (8)

#define STRIPPER_JEER_DISTANCE (10)

//CPed* gapTempPedList[PED_MAX_SORTEDPEDLISTSIZE];
//uint16 gnNumTempPedList;
//CPed* CPed::ms_apAllPeds[PED_MAX_SORTEDPEDLISTSIZE];
//uint16 CPed::ms_nNumAllPeds;
//CVector2D CPed::ms_vec2DTargetPosition;
//CVector2D CPed::ms_vec2DFleePosition;
//int32 CPed::ms_nUpdateIndex;
//int32 CPed::ms_nUpdateTimer;
//CVector	vecVehicleSeatPosOffset = CVector(0.0f, -0.30f, -0.30f); // offset from seat position - should be 0,0,0
static CVector vecVehicleSeatPosOffset = CVector(0.0f, 0.0f, -0.50f); // offset from seat position - should be 0,0,0
static 	CColPoint aTempPedColPts[PHYSICAL_MAXNOOFCOLLISIONPOINTS];

#ifndef FINAL
	//int32 CPed::nDebugTestPlayerAttacks = FIGHT_IDLE;
	int32 CPed::nDebugPedOption2 = 0;
	float CPed::fDebugPedOption3 = 0.0f;
	bool  CPed::bDebugSay = false;
#endif

int32 CPed::nCloseBlockedLosWaitTime = 1500;
int32 CPed::nFarBlockedLosWaitTime = 3000;

uint16 CPed::m_sGunFlashBlendStart = 10000;
uint16 CPed::m_sGunFlashBlendOutRate = 10;

extern bool8 g_bUsingAnimViewer;

RwObject *GetCurrentAtomicObjectCB(RwObject *pObject, void *data);

#ifndef FINAL // just incase I forget to comment this out!
//	#pragma optimization_level 0
//	#pragma dont_inline on
//	#pragma auto_inline off
#endif

#pragma optimize_for_size on

// Name			:	Constructor
// Purpose		:	Default constructor for CPed class
// Parameters	:	pedType: type of ped, Constructor requires this as it uses this variable in the function
// Returns		:	Nothing

CPed::CPed(uint32 pedType) : 
m_nPedType(pedType),
m_nMaxHealth(100),
m_nHealth(100),
m_nArmour(0),
m_extractedVelocity(0.0f,0.0f),
m_ik(this)
{
	SetTypePed();
	SetUsesCollisionRecords(TRUE);
	m_nPhysicalFlags.bPedPhysics = true;
	
	//m_Objective = NO_OBJ;
	//m_StoredObjective = NO_OBJ;
	
	CharCreatedBy = RANDOM_CHAR;
	
	//pLeader = NULL;
	//m_pObjectivePed = NULL;
	//m_pObjectiveVehicle = NULL;
	//m_fObjectiveHeading = 0;

	m_pMyVehicle = NULL;
	//m_pAnim = NULL;
	//m_vecInitialBlendVector = CVector(0.0f, 0.0f, 0.0f);
	
/*	
    m_pMyAttractor=0;
    m_iAttractorSlot=-1;
*/
	
	//m_FormationType = PED_FORMATION_NONE,
	
	//m_nCollisionTimer = 0;
	m_nAntiSpazTimer = 0;
	//m_vecAntiSpazVector = CVector2D(0.0f, 0.0f);
	//m_nPhoneTimer = 0;
	//m_bKeepActive = FALSE;
	//m_nPauseTimer = 0;
	m_nUnconsciousTimer = 0;
	m_nAttackTimer = 0;
	//m_nActiveTimer = 0;
	m_nLookTimer = 0;
	//m_nChatTimer = 0;
	//m_nShootTimer = 0;
	//m_nCarJackTimer = 0;
	//m_nObjectiveTimer = 0;
	//m_nDuckTimer = 0;
	//m_nDuckAndCoverTimer = 0;
	m_nTimeOfDeath = 0;
	m_vecCurrentVelocity = CVector2D(0.0f, 0.0f);
	m_fCurrentHeading = 0.0f;
	m_fHeadingChangeRate = 15.0f;
	m_fHeadingChangeRateAccel = 0.1f;
	m_fDesiredHeading=0.0f;
	//m_nDoor = CAR_DOOR_LF;
	//m_nAvoidCorner = AVOID2_NONE;
	m_pGroundPhysical = NULL;
	m_vecGroundOffset = CVector(0.0f, 0.0f, 0.0f);
	m_vecGroundNormal = CVector(0.0f, 0.0f, 1.0f);
	
	//m_pTargetEntity = NULL;
	//m_vecTargetPosition = CVector(0.0f, 0.0f, 0.0f);
	m_nShootRate = 40;
	//m_fArriveRange = 1.0f;
	//m_fHeadingRange = 0.1f;
	
	//m_vecGotoCoordDestination=CVector(0.0f,0.0f,0.0f);
	//m_fGotoCoordOnFootFromPathDistance=0.0f;
	//m_eGotoCoordOnFootMoveState=PEDMOVE_NONE;
	
	//m_nTypeOfSeek= SEEKTYPE_PATHNODE;
	//m_nPhoneToUse = -1;
	m_pAccident= NULL;

	//m_pFleeEntity = NULL;
	//m_vecFleePosition = CVector2D(0.0f, 0.0f);
	//m_nFleeTimer = 0;
    //m_pFleeEntityWhenStanding = NULL;

	//m_vecGuardPosition = CVector(0.0f, 0.0f, 0.0f);
	//m_fGuardRange = 0;

	//m_eWaitState = PEDWAIT_FALSE;
	//m_nWaitTimer = 0;
	m_pNOCollisionVehicle = NULL;

	m_nPedState = PED_IDLE;
	//m_nPedStoredState = PED_NONE;
	m_eMoveState = PEDMOVE_STILL;
	m_eMoveStateAnim = PEDMOVE_NONE;

	m_pFire = NULL;
	FireDamageMultiplier = 1.0f;
	
	m_pEntLockOnTarget = NULL;
	
	m_pEntLookEntity = NULL;
	m_fLookHeading = 0.0f;

	m_pEntityStandingOn = NULL;
	m_fHitHeadHeight = PED_HIT_HEAD_CLEAR;
	
	//m_pRange2D = NULL;

//	m_nPedRand = CGeneral::GetRandomNumberInRange(0, 1000);

	//for (int32 i = 0; i < PED_NUM_PATHNODES_LOOKAHEAD; i++)
	//{
	//	m_aPathNodeList[i].SetEmpty();
	//}

	//m_nNumPathNodes = 0;
	//m_nCurrentPathNode = 0;
	//m_nPathWanderDirection = 0;
	//m_LastWanderNode.SetEmpty();
	//m_NextWanderNode.SetEmpty();
    //m_pEntityToAvoidOnPathFollow=NULL;
    //m_pEntityToFollowOnPathFollow=NULL;
	//m_iPathFollowTimer = 0;
	//m_CurrentPathNode.SetEmpty();
	
	//m_iDelayedThreatType=0;
    //m_iDelayedThreatTimer=0;
    //m_iDelayedThreatResponseTime=CGeneral::GetRandomNumberInRange(MIN_THREAT_RESPONSE_TIME,MAX_THREAT_RESPONSE_TIME);

	//m_nCurrentRoute = -1;
	//m_nRouteStartIndex = 0;
	//m_nPositionOnRoute = 0;
	//m_nTypeOfRouteFollow = 0;

	m_fMass = 70.0f;
	m_fTurnMass = 100.0f;
	m_fAirResistance = 1.0f*0.4f/m_fMass;
	m_fElasticity = 0.05f;
	//m_nCannotSeeTargetTimer = 0;
	//m_nCannotSeeCeaseAttackTime = 0;
	m_nLimbRemoveIndex = -1;

	m_nPedFlags.bIsStanding = false;
	m_nPedFlags.bWasStanding = false;
	m_nPedFlags.bIsLooking = false;
	m_nPedFlags.bIsRestoringLook = false;
	m_nPedFlags.bIsAimingGun = false;	
	m_nPedFlags.bIsRestoringGun = false;
	m_nPedFlags.bCanPointGunAtTarget = false;	
	m_nPedFlags.bIsTalking = false;
	
	m_nPedFlags.bInVehicle = FALSE;
	m_nPedFlags.bIsInTheAir = false;
	m_nPedFlags.bIsLanding = false;
	m_nPedFlags.bHitSomethingLastFrame = false;
	m_nPedFlags.bIsNearCar = false;
	m_nPedFlags.bRenderPedInCar = true;
	m_nPedFlags.bUpdateAnimHeading = false;
	m_nPedFlags.bRemoveHead = false;
	m_nPedFlags.bFiringWeapon = false;
	
	m_nPedFlags.bHasACamera = (CGeneral::GetRandomNumber() &3) ? false : true;
	m_nPedFlags.bPedIsBleeding = false;
	m_nPedFlags.bStopAndShoot = false;
	m_nPedFlags.bIsPedDieAnimPlaying = false;	
	m_nPedFlags.bStayInSamePlace = false;	
	m_nPedFlags.bKindaStayInSamePlace = false;	
	m_nPedFlags.bBeingChasedByPolice = false;	
	m_nPedFlags.bNotAllowedToDuck = false;
	m_nPedFlags.bCrouchWhenShooting = false;

	m_nPedFlags.bIsDucking = false;
	m_nPedFlags.bGetUpAnimStarted = false;
	m_nPedFlags.bDoBloodyFootprints = false;	
//	m_nPedFlags.bIsLeader = false;
	m_nPedFlags.bDontDragMeOutCar = false;	
	m_nPedFlags.bStillOnValidPoly = false;
	m_nPedFlags.bAllowMedicsToReviveMe = true;
	m_nPedFlags.bResetWalkAnims = false;
	
	m_nPedFlags.bOnBoat = false;
	m_nPedFlags.bBusJacked = false;
	m_nPedFlags.bFadeOut = false;
	m_nPedFlags.bKnockedUpIntoAir = false;
	m_nPedFlags.bHitSteepSlope = false;
	m_nPedFlags.bCullExtraFarAway = false;
	m_nPedFlags.bTryingToReachDryLand = false;
	m_nPedFlags.bCollidedWithMyVehicle = false;
	
	m_nPedFlags.bRichFromMugging = false;
	m_nPedFlags.bChrisCriminal = false;	
	m_nPedFlags.bShakeFist = false;
	m_nPedFlags.bNoCriticalHits = false;
	m_nPedFlags.bHasAlreadyBeenRecorded = false;
	m_nPedFlags.bUpdateMatricesRequired = false;
	m_nPedFlags.bFleeWhenStanding = false;
	m_nPedFlags.bMiamiViceCop=false;

	m_nPedFlags.bMoneyHasBeenGivenByScript=false;
	m_nPedFlags.bHasBeenPhotographed=false;
	m_nPedFlags.bIsDrowning = false;
	m_nPedFlags.bDrownsInWater = true;
	m_nPedFlags.bHeadStuckInCollision = false;
	m_nPedFlags.bDeadPedInFrontOfCar = false;
	m_nPedFlags.bStayInCarOnJack = false;	
	m_nPedFlags.bDontFight = false;
	
	m_nPedFlags.bDoomAim = true;
	m_nPedFlags.bCanBeShotInVehicle = true;
	m_nPedFlags.bPushedAlongByCar = false;
	m_nPedFlags.bNeverEverTargetThisPed = false;
	m_nPedFlags.bThisPedIsATargetPriority = false;	
	m_nPedFlags.bCrouchWhenScared = false;
	m_nPedFlags.bKnockedOffBike = false;
	m_nPedFlags.bDonePositionOutOfCollision = false;
       
    m_nPedFlags.bDontRender = false;
    m_nPedFlags.bHasBeenAddedToPopulation = false;
    m_nPedFlags.bHasJustLeftCar = false;;
    m_nPedFlags.bIsInDisguise = false;
	m_nPedFlags.bDoesntListenToPlayerGroupCommands = false;
    m_nPedFlags.bIsBeingArrested = false;
    m_nPedFlags.bHasJustSoughtCover = false;
    m_nPedFlags.bKilledByStealth = false;
    m_nPedFlags.bDoesntDropWeaponsWhenDead = false;
    m_nPedFlags.bBloodPuddleCreated = false;
    
    m_nPedFlags.bCalledPreRender = false;
	m_nPedFlags.bPartOfAttackWave = false;
	m_nPedFlags.bClearRadarBlipOnDeath = false;
	m_nPedFlags.bNeverLeavesGroup = false;
	m_nPedFlags.bTestForBlockedPositions = false;
	m_nPedFlags.bRightArmBlocked = false;
	m_nPedFlags.bLeftArmBlocked = false;
	m_nPedFlags.bDuckRightArmBlocked = false;
	m_nPedFlags.bMidriffBlockedForJump = false;
	m_nPedFlags.bFallenDown = false;
	m_nPedFlags.bUseAttractorInstantly = false;
	m_nPedFlags.bDontAcceptIKLookAts = false;
	m_nPedFlags.bIsCached = false;
	
	m_nPedFlags.bHasAScriptBrain = false;
	m_nPedFlags.bWaitingForScriptBrainToLoad = false;
	m_nPedFlags.bHasGroupDriveTask = false;
	m_nPedFlags.bCanExitCar = true;
	m_nPedFlags.CantBeKnockedOffBike = KNOCKOFFBIKE_DEFAULT;
	m_nPedFlags.bPushOtherPeds = false;
	m_nPedFlags.bHasBulletProofVest = false;
	
	m_nPedFlags.bUsingMobilePhone = false;
	m_nPedFlags.bUpperBodyDamageAnimsOnly = false;
	m_nPedFlags.bStuckUnderCar = false;
	m_nPedFlags.bKeepTasksAfterCleanUp = false;
	m_nPedFlags.bIgnoreHeightCheckOnGotoPointTask = false;
	m_nPedFlags.bForceDieInCar = false;
	m_nPedFlags.bCheckColAboveHead = false;
	m_nPedFlags.bIgnoreWeaponRange = false;

	m_nPedFlags.bDruggedUp = false;
	m_nPedFlags.bWantedByPolice = false;
	m_nPedFlags.bSignalAfterKill = true;
	m_nPedFlags.bCanClimbOntoBoat = false;
	m_nPedFlags.bPedHitWallLastFrame = false;
	m_nPedFlags.bIgnoreHeightDifferenceFollowingNodes = false;
	m_nPedFlags.bMoveAnimSpeedHasBeenSetByTask = false;
	m_nPedFlags.bGetOutUpsideDownCar = true;
	
	m_nPedFlags.bJustGotOffTrain = false;
	m_nPedFlags.bDeathPickupsPersist = false;
	m_nPedFlags.bTestForShotInVehicle = false;
#ifdef GTA_REPLAY
	m_nPedFlags.bUsedForReplay = false;
#endif
	// Register this ped with the audio stuff
#ifdef USE_AUDIOENGINE
	m_PedWeaponAudioEntity.Initialise(this);
	m_PedAudioEntity.Initialise(this);
#else //USE_AUDIOENGINE
	AudioHandle = DMAudio.CreateEntity(AUDT_PHYSICAL, (void *)this);
	DMAudio.SetEntityStatus(AudioHandle, true);		// enable dude.
#endif //USE_AUDIOENGINE

	//m_nThreats = CPedType::GetPedTypeThreats(GetPedType());
	m_acquaintances = CPedType::GetPedTypeAcquaintances(GetPedType());
	
	//m_pThreatEntity = NULL;
	//m_vec2DThreatPosition = CVector2D(0.0f, 0.0f);
	//m_pInterestingEntity = NULL;
	//m_eInterestingEvent = EVENT_NULL;
	//m_fInterestingEventHeading = 0.0f;

//	m_cPedOnFireAudio = -1;
//	m_NumClosePeds = 0;
//	for (Int16 i = 0; i < PED_MAX_CLOSEPEDLISTSIZE; i++)
//	{
//		m_apClosePeds[i] = 0;
//	}
	
	
//	m_nNumWeapons = 0;
	m_nCurrentWeapon = 0;
	m_eStoredWeapon = WEAPONTYPE_UNIDENTIFIED;	// use this weapontype to denote No stored weapon
	m_eDelayedWeapon = WEAPONTYPE_UNIDENTIFIED;
// this will be removed when it's safe to do so/////////////
/*
	for (int loop = 0; loop < PED_MAX_WEAPONS; loop++)
	{
		m_wepWeapon[loop].m_eWeaponType = WEAPONTYPE_UNARMED;
		m_wepWeapon[loop].m_eState = WEAPONSTATE_READY;
		m_wepWeapon[loop].m_nAmmoInClip = 0;
		m_wepWeapon[loop].m_nAmmoTotal = 0;
		m_wepWeapon[loop].m_nTimer = 0;
	}
*/
///////////////////////////////////

	int loop;
	for(loop=0; loop < PED_MAX_WEAPON_SLOTS; loop++)
	{
		m_WeaponSlots[loop].m_eWeaponType = WEAPONTYPE_UNARMED;
		m_WeaponSlots[loop].m_eState = WEAPONSTATE_READY;
		m_WeaponSlots[loop].m_nAmmoInClip = 0;
		m_WeaponSlots[loop].m_nAmmoTotal = 0;
		m_WeaponSlots[loop].m_nTimer = 0;
	}
	
	m_nWeaponSkill = WEAPONSKILL_STD;
	m_nExtraMeleeCombo = MCOMBO_UNARMED_1;
	m_nExtraMeleeComboFlags = 0;
	
	//m_eOldFightState = FIGHT_NULL;	
	//m_eFightState = FIGHT_NULL;
	GiveWeapon(WEAPONTYPE_UNARMED, 0);

	SetShootingAccuracy(60);
	
	LastDamagedWeaponType = -1;
	pLastDamageEntity = NULL;
	LastDamagedTime = 0;
	m_pAttachToEntity = NULL;	// attach player to this entity
	m_nOriginalWeaponAmmo = 0;
	
	m_storedCollPoly.bValidPolyStored = false;
	m_distTravelledSinceLastHeightCheck = 0.0f;

	WeaponModelInHand = -1;
	m_MoneyCarried = 0;

	BleedingFrames = 0;

	DontUseSmallerRemovalRange = 0;
	
	m_pMyAccidentVehicle = 0;

/*
    m_pMyAttractor = 0;
    m_iAttractorSlot = -1;
*/
	m_pWeaponClump = NULL;
	m_pWeaponFlashFrame = NULL;
	m_pGogglesClump = NULL;
	m_pbGogglesEffect = NULL;
	m_nGunFlashBlendAmount = 0;
	m_nGunFlashBlendOutRate = 0;
	m_nGunFlashBlendAmount2 = 0;
	m_nGunFlashBlendOutRate2 = 0;

    //m_iDelayedPhrase=-1;
	//m_iDelayedPhraseTimer=0;
	m_pCoverPoint = NULL;
	m_pLastEntryExit = NULL;
	
	// Audio
	
	LastTalkSfx = 0xffffffff;	// last thing the ped said
	
#ifdef DEBUG_PED_STATE	
	m_states.clear();
	m_oldstates.clear();
	m_storedstates.clear();
	m_objectives.clear();
#endif	

	CMemInfo::EnterBlock(TASKEVENT_MEM_ID);	

    m_pPedIntelligence=new CPedIntelligence(this);
    m_pPlayerData = NULL;

	CMemInfo::ExitBlock();	


    if (!IsPlayer())
    {
	  	m_pPedIntelligence->AddTaskSecondaryFacialComplex(new CTaskComplexFacial);
	}	        	

	// at least give the ped a default task
	m_pPedIntelligence->AddTaskDefault(new CTaskSimpleStandStill(0,true));
	//m_pPedIntelligence->AddTaskDefault(CTaskComplexWander::GetWanderTaskByPedType(m_nPedType));
    
//    m_breatheCounter = 0.0f;
    m_wobble = 0.0f;

	fRemoveRangeMultiplier = 1.0f;
	StreamedScriptBrainToLoad = -1;
	
	CPopulation::UpdatePedCount(this, ADD_TO_POPULATION);
	
	m_nPedFlags.bHasBeenRendered = false;	
   	m_nPedFlags.bIsDyingStuck = false;

#ifndef FINAL
	m_SayWhat = -1;
	m_StartTimeSay = -1;
#endif

	if(CCheat::IsCheatActive(CCheat::EVERYBODYATTACKSPLAYER_CHEAT) && !IsPlayer())
	{
		m_acquaintances.SetAsAcquaintance(ACQUAINTANCE_TYPE_PED_HATE,CPedType::GetPedFlag(PEDTYPE_PLAYER1));
		CEventAcquaintancePedHate event(FindPlayerPed());
		event.SetResponseTaskType(CTaskTypes::TASK_COMPLEX_KILL_PED_ON_FOOT);
		GetPedIntelligence()->AddEvent(event);	
	}

} // end - CPed::CPed




// Name			:	Destructor
// Purpose		:	Default destructor for CPed class
// Parameters	:	None
// Returns		:	Nothing

CPed::~CPed()
{
	CReplay::RecordPedDeleted(this);

	if (m_nPedFlags.bWaitingForScriptBrainToLoad)
	{
		CStreaming::SetMissionDoesntRequireScript(CTheScripts::ScriptsForBrains.ScriptBrainArray[StreamedScriptBrainToLoad].StreamedScriptIndex);
		m_nPedFlags.bWaitingForScriptBrainToLoad = false;
		CTheScripts::RemoveFromWaitingForScriptBrainArray(this, StreamedScriptBrainToLoad);
		StreamedScriptBrainToLoad = -1;
	}

	CWorld::Remove(this);
	
	/*
	if(m_pMyAttractor)
	{
    	GetPedAttractorManager()->DeRegisterPed(this,m_pMyAttractor);
    }
    */
	
	CRadar::ClearBlipForEntity(BLIPTYPE_CHAR, CPools::GetPedPool().GetIndex(this));
	
	CConversations::RemoveConversationForPed(this);

	
//	If the character is in a vehicle then check whether the character
//	is the driver or a passenger and remove the relevant pointer to this character
//	in the vehicle class
/*
	if (m_nPedFlags.bInVehicle && m_pMyVehicle)
	{
		ASSERT(m_pMyVehicle);
		
		uint8 nFlagForMyDoor = 0;
		switch(m_nDoor)
		{
			case CAR_DOOR_LF:nFlagForMyDoor = 1;break;
			case CAR_DOOR_LR:nFlagForMyDoor = 2;break;
			case CAR_DOOR_RF:nFlagForMyDoor = 4;break;
			case CAR_DOOR_RR:nFlagForMyDoor = 8;break;
			default: break;
		}

		if (m_pMyVehicle->pDriver == this)
		{	//	This character is the driver of the vehicle
			TIDYREF(m_pMyVehicle->pDriver, (CEntity **) &m_pMyVehicle->pDriver);
			//m_pMyVehicle->pDriver = NULL;
			m_pMyVehicle->RemoveDriver();
		}
		else
		{
			for(int i =0; i < m_pMyVehicle->m_nMaxPassengers; i++)
			{
				if(m_pMyVehicle->pPassengers[i] == this)
				{
//					TIDYREF(m_pMyVehicle->pPassengers[i], ((CEntity**)&m_pMyVehicle->pPassengers[i]));
//					m_pMyVehicle->pPassengers[i] = NULL;
					m_pMyVehicle->RemovePassenger(this);
				}
			}
		}

		// If this ped is getting out of a car we have to tell the car about that
		if (m_nPedState == PED_EXIT_CAR || m_nPedState == PED_DRAGGED_FROM_CAR)
		{
			// clear appropriate flags in cars
			if (m_nPedState == PED_EXIT_CAR || m_nPedState == PED_DRAGGED_FROM_CAR)
			{
				m_pMyVehicle->m_nGettingOutFlags &= ~(nFlagForMyDoor);

//sprintf(gString, "m_nGettingOutFlags:%d (flag:%d)", m_pMyVehicle->m_nGettingOutFlags, nFlagForMyDoor);
//TheConsole.AddLine(gString);
//CDebug::DebugLog(gString);
			}
		}
		
//	Still need to check if this character is a passenger in the vehicle
		m_nPedFlags.bInVehicle = FALSE;
	}
*/
	
	TIDYREF(m_pMyVehicle, (CEntity**)&m_pMyVehicle);
	m_pMyVehicle = NULL;
	
	/*
	else
	{
		if(m_nPedState == PED_ENTER_CAR || m_nPedState == PED_CARJACK)
		{
			QuitEnteringCar();
		}
	}
	*/		

	if (m_pFire)
	{
		m_pFire->Extinguish();
	}
	
	ReleaseCoverPoint();

	// If ped is referenced by another ped, clear ped references
/*		This shouldn't be needed anymore.
	if(GetIsReferenced())
	{
		CPedPool& pool = CPools::GetPedPool();
		CPed* pPed;
		int32 i=pool.GetSize();
		
		while(i--)
		{
			pPed = pool.GetSlot(i);
			if(pPed == NULL)
				continue;
				
			if(pPed->m_pObjectivePed == this)
				pPed->SetObjective(NO_OBJ);
			if(pPed->pLeader == this)
				pPed->pLeader = NULL;
			// Not sure if you need this variable
			if(pPed->m_pThreatEntity == this)
				pPed->m_pThreatEntity = NULL;
			if(pPed->m_pTargetEntity == this)
			{
				switch(pPed->m_nPedState)
				{
				case PED_SEEK_ENTITY:
					pPed->ClearSeek();
					break;
				case PED_ATTACK:
					pPed->ClearAttack();
					break;
				case PED_MUG:
					pPed->ClearMug();
					break;
				}
				pPed->ClearPointGunAt();
			}
			if(pPed->m_pEntLookEntity == this)
			{
				pPed->ClearPointGunAt();
			}		
		}		
	}
*/	
	ClearWeapons();
	ASSERTMSG(m_pWeaponClump == NULL, "Weapon clump hasn't been deleted when deleting a ped");
	ASSERTMSG(m_pGogglesClump == NULL, "Goggles clump hasn't been deleted when deleting a ped");
	
	//If the ped was create as a passener then inform the 
	//population manager that a passenger has been deleted.
	UpdateStatLeavingVehicle();
	
	if (m_nPedFlags.bMiamiViceCop) CPopulation::NumMiamiViceCops--;

	CPopulation::UpdatePedCount(this, SUBTRACT_FROM_POPULATION);
	
#ifdef USE_AUDIOENGINE
	m_PedSpeechAudioEntity.Terminate();
	m_PedWeaponAudioEntity.Terminate();
	m_PedAudioEntity.Terminate();
#else //USE_AUDIOENGINE
	DMAudio.DestroyEntity(AudioHandle);
#endif //USE_AUDIOENGINE
	
	if(m_pPedIntelligence) delete m_pPedIntelligence;

	TIDYREF(m_pEntLookEntity, &m_pEntLookEntity);
	//TIDYREF(m_pTargetEntity, &m_pTargetEntity);
	
} // end - CPed::~CPed


//
// operator new and delete for CPed
//
void* CPed::operator new(size_t nSize) 
{ 
	ASSERT(nSize <= CPedPool::GetStorageSize());
	return ((void*) CPools::GetPedPool().New()); 
}


void* CPed::operator new (size_t nSize, int32 index)
{
	ASSERT(nSize <= CPedPool::GetStorageSize());
	return ((void*) CPools::GetPedPool().New(index)); 
}


void CPed::operator delete(void *pVoid) 
{ 
	CPools::GetPedPool().Delete((CPed*)pVoid); 
}

void CPed::operator delete(void *pVoid, int32 index) 
{ 
	CPools::GetPedPool().Delete((CPed*)pVoid); 
}




// Name			:	Initialise
// Purpose		:	Init CPed stuff
// Parameters	:	None
// Returns		:	Nothing

void CPed::Initialise()
{
	DEBUGLOG("Initialising CPed...\n");
	
	// set up ped type information
	CPedType::Initialise();
//	CTaskSimpleFight::LoadMeleeData();
	CCarEnterExit::SetAnimOffsetForEnterOrExitVehicle();

//	ms_nUpdateIndex = 0;
//	ms_nUpdateTimer = 0;
//#ifndef FINAL
//	VarConsole.Add("Debug Player Fight Move",&CPed::nDebugTestPlayerAttacks, ((int32)1), ((int32)FIGHT_A_ATTACK1), ((int32)FIGHT_GROUND_KICK), true);
//#endif

	DEBUGLOG("CPed ready\n");
} // end - CPed::Initialise

void CPed::SetCharCreatedBy(const UInt8 a) 
{	
	CharCreatedBy=a;
	SetPedDefaultDecisionMaker();
		
	if (CharCreatedBy == MISSION_CHAR)
	{
    	GetPedIntelligence()->SetSeeingRange(CPedIntelligence::ms_fSenseRangeOfMissionPeds);
		GetPedIntelligence()->SetHearingRange(CPedIntelligence::ms_fSenseRangeOfMissionPeds);		
		if(!IsPlayer())
		{
			GetPedIntelligence()->SetInformRespectedFriends(0,0);
		}
	}
	else
	{
		ASSERTMSG( (!IsPlayer()) || CharCreatedBy == REPLAY_CHAR, "CPed::SetCharCreatedBy - Player Ped should always be a Mission character");
	}
}

void CPed::SetModelIndex(uint32 index)
{
	m_nFlags.bIsVisible = true;

	ASSERTOBJ(CModelInfo::GetModelInfo(index)->GetRwObject(), CModelInfo::GetModelInfo(index)->GetModelName(), "Ped model is not loaded");
	CEntity::SetModelIndex(index);

	RpAnimBlendClumpInit((RpClump*)m_pRwObject);

	// Get Ped Nodes have to do this after RpAnimBlendClumpInit() has been called
	RpAnimBlendClumpFillFrameArray((RpClump*)m_pRwObject, &m_aPedFrames[0]);

	CPedModelInfo* pModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(m_nModelIndex);
	ASSERT(pModelInfo->GetModelType() == MI_TYPE_PED);
	
	// set ped stats pointer to default stored in type modelinfo
	SetPedStats(pModelInfo->GetDefaultPedStats());
	m_fHeadingChangeRate = m_pPedStats->m_fMaxHeadingChange;
	
	// set ped decision maker to the default (defined in pedstat.dat)
	SetPedDefaultDecisionMaker();


	// Rich peds carry more money
	if (CPopCycle::IsPedInGroup(GetModelIndex(), POPCYCLE_GROUP_BUSINESS) ||
		CPopCycle::IsPedInGroup(GetModelIndex(), POPCYCLE_GROUP_CASUAL_RICH))
	{
		m_MoneyCarried = 20 + CGeneral::GetRandomNumber() % 50;
	}
	else
	{
		m_MoneyCarried = CGeneral::GetRandomNumber() % 25;
	}
	if (CGeneral::GetRandomNumberInRange(0, 100) < 3) m_MoneyCarried = 400;


	// get MotionAnimGroup before calling SetMoveAnim()
	m_motionAnimGroup = pModelInfo->GetMotionAnimGroup();
	CAnimManager::AddAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE);

	if(!CanUseTorsoWhenLooking())
		m_ik.SetFlag(PEDIK_TORSO_USED);
		
	// Set extracted velocity vector
	ANIMBLENDCLUMPFROMCLUMP(m_pRwObject)->SetVelocityPtr(&m_extractedVelocity.x);

	if(pModelInfo->GetHitColModel()==NULL)
		pModelInfo->CreateHitColModelSkinned((RpClump *)m_pRwObject);

	UpdateRpHAnim();
}

void CPed::SetPedStats(ePedStats nIndex)
{
	m_pPedStats = CPedStats::GetPedStatInfo(nIndex);
}

void CPed::SetPedDefaultDecisionMaker()
{
	if(IsPlayer())
	{
		m_pPedIntelligence->SetPedDecisionMakerType(CDecisionMakerTypes::PLAYER_DECISION_MAKER);		
	}
	else if(!(MISSION_CHAR==CharCreatedBy))
	{
		//m_pPedIntelligence->SetPedDecisionMakerType(CPedStats::ms_apPedStats[m_pPedStats->m_ePedStatType]->m_iDefaultDecisionMaker);
		m_pPedIntelligence->SetPedDecisionMakerType(m_pPedStats->m_iDefaultDecisionMaker);
	}
	else
	{
		m_pPedIntelligence->SetPedDecisionMakerType(CDecisionMakerTypes::DEFAULT_DECISION_MAKER);
	}

}


// Name			:	Update
// Purpose		:	Updates CPed stuff
// Parameters	:	None
// Returns		:	Nothing

void CPed::Update()
{
	// This is used to determine if the ped will run a full update (building ped lists
	// and other stuff) or just a simple update
//	ms_nUpdateTimer++;
//	ms_nUpdateTimer %= PED_UPDATE_DELAY;
} // end - CPed::Update

void CPed::DeleteRwObject()
{
	// This is done in the destructor. 
/*	if(m_pWeaponAtomic)
	{
		RwFrame* pFrame = RpAtomicGetFrame(m_pWeaponAtomic);
		RpAtomicDestroy(m_pWeaponAtomic);
		RwFrameDestroy(pFrame);
		m_pWeaponAtomic = NULL;
	}*/

	CEntity::DeleteRwObject();
}

/*
// Name			:	BuildPedLists
// Purpose		:	This has all been simplified dramatically. Every ped keeps a list
//					of other peds that are nearby. The ped would have to check on the
//					fly whether peds are visible. The ClosePedList is only generated once
//					every 16 frames or so.
// Parameters	:	None
// Returns		:	Nothing

void CPed::BuildPedLists()
{
	float fSortedPedListRangeToUse = PED_SORTEDPEDLISTRANGE * ((float) nThreatReactionRangeMultiplier);
	float fPedSenseRangeToUse = PED_SENSERANGE * ((float) nThreatReactionRangeMultiplier);

	if ( ( (this->RandomSeed + CTimer::m_FrameCounter) & 15) == 0)
	{		// Build fresh list

		CVector vecCentre = GetBoundCentre();
		CRect rect;
		CPed *pPed;
		int32 i = 0;
		int32 j = 0;
		int	 nDeadPeds = 0;

		rect.left = vecCentre.x - fSortedPedListRangeToUse;
		rect.bottom = vecCentre.y - fSortedPedListRangeToUse;
		rect.right = vecCentre.x + fSortedPedListRangeToUse;
		rect.top = vecCentre.y + fSortedPedListRangeToUse;

		int32 nLeft = WORLD_WORLDTOSECTORX(rect.left);
		int32 nBottom = WORLD_WORLDTOSECTORY(rect.bottom);
		int32 nRight = WORLD_WORLDTOSECTORX(rect.right);
		int32 nTop = WORLD_WORLDTOSECTORY(rect.top);

		CVector2D vec2DPedVector, vec2DTargetPed;

		vec2DPedVector.x = GetPosition().x;
		vec2DPedVector.y = GetPosition().y;

		gnNumTempPedList = 0;

		CWorld::AdvanceCurrentScanCode();
		for (int32 y = nBottom; y <= nTop; y++)
		{
			for (int32 x = nLeft; x <= nRight; x++)
			{
				CPtrList& list = CWorld::GetRepeatSector(x, y).GetOverlapPedPtrList();
				CPtrNode* pNode = list.GetHeadPtr();

				while (pNode != NULL)
				{
					pPed = (CPed*)pNode->GetPtr();
					
					if (pPed->GetScanCode() != CWorld::GetCurrentScanCode())
					{
						pPed->SetScanCode(CWorld::GetCurrentScanCode());
						// ignore current ped and peds in cars
						if( pPed != this && (!pPed->m_nPedFlags.bInVehicle || (pPed->m_pMyVehicle && pPed->m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)) )
						{
							vec2DTargetPed.x = pPed->GetPosition().x - vec2DPedVector.x;
							vec2DTargetPed.y = pPed->GetPosition().y - vec2DPedVector.y;

							if (vec2DTargetPed.Magnitude() < fPedSenseRangeToUse)
							{					
								// this ped is in sensing range - add it to the list
								AssertPedPointerValid(pPed);
							
								// only want to store a couple of dead peds (for the ambulance guys)
								// otherwise they clog up the list and affect threat searches and stuff
								// for the mission peds
								if(pPed->m_nPedState == PED_DEAD)
								{
									if(nDeadPeds < 4)
									{
										nDeadPeds++;
										gapTempPedList[gnNumTempPedList] = pPed;
										gnNumTempPedList++;
									}
								}
								else
								{
									gapTempPedList[gnNumTempPedList] = pPed;
									gnNumTempPedList++;
								}
							}
						}
					}

					pNode = pNode->GetNextPtr();
				}
			}
		}

		// flag the ends of the list with a NULL
		gapTempPedList[gnNumTempPedList] = NULL;

		// Quicksort the list
		SortPeds(gapTempPedList, 0, gnNumTempPedList-1);


		m_NumClosePeds = 0;
		while (m_NumClosePeds != PED_MAX_CLOSEPEDLISTSIZE && gapTempPedList[m_NumClosePeds] != NULL)
		{
			m_apClosePeds[m_NumClosePeds] = gapTempPedList[m_NumClosePeds];
			m_NumClosePeds++;
		}

		// Fill the rest in with zeros
		for (Int32 nClosePed = m_NumClosePeds; nClosePed < PED_MAX_CLOSEPEDLISTSIZE; nClosePed++)
		{
			m_apClosePeds[nClosePed] = NULL;
		}
	}
	//else
	//{		// Prune old list

		// Go through the close ped lists and remove the entries that have disappeared or now are far away
		Int16	Peds = 0;
		Int16	ClearCount;
		bool	bRemove;
		float	DiffX, DiffY;
	
		while (Peds < PED_MAX_CLOSEPEDLISTSIZE)
		{
			bRemove = false;
			if (m_apClosePeds[Peds])
			{
		
				if (!m_apClosePeds[Peds]->IsPointerValid())//m_listEntryInfo.GetHeadPtr() == NULL)
				{
					bRemove = true;
				}
				else
				{
					DiffX = GetPosition().x - m_apClosePeds[Peds]->GetPosition().x;
					DiffY = GetPosition().y - m_apClosePeds[Peds]->GetPosition().y;
					if (DiffX*DiffX + DiffY*DiffY > fPedSenseRangeToUse*fPedSenseRangeToUse)
					{					
						bRemove = true;
					}
				}
			}
			if (bRemove)
			{
				// Shift the rest back one position
				for (ClearCount = Peds; ClearCount < (PED_MAX_CLOSEPEDLISTSIZE-1); ClearCount++)
				{
					m_apClosePeds[ClearCount] = m_apClosePeds[ClearCount+1];
				}
				// Set last to zero
				m_apClosePeds[PED_MAX_CLOSEPEDLISTSIZE-1] = NULL;
				m_NumClosePeds--;
				ASSERT(m_NumClosePeds >= 0);
				
#ifdef DEBUG
{		// Just a test to make sure removing one ped has happened correctly
	Int16	Bla;
	for (Bla = 0; Bla < m_NumClosePeds; Bla++)
	{
		ASSERT(m_apClosePeds[Bla]);
	}
	while (Bla < PED_MAX_CLOSEPEDLISTSIZE)
	{
		ASSERT(!m_apClosePeds[Bla]);
		Bla++;
	}

}
#endif				
				
			}
			else
			{
				Peds++;
			}
		}
	//}
	
#ifdef DEBUG
//	Last updated 28.11.00
	UInt32 loop;
	
	for (loop = 0; loop < PED_MAX_CLOSEPEDLISTSIZE; loop++)
	{
		if (m_apClosePeds[loop])
		{
			AssertEntityPointerValid(m_apClosePeds[loop]);
		}
	}

//	End last updated 28.11.00
#endif
	
} // end - CPed::BuildPedLists
*/

bool CPed::CanSeeEntity(CEntity *pEntity, float viewAngle)
{
	float fViewAngle, fHeadingAngle, AngleDiff;
	
	AssertEntityPointerValid(pEntity);
	
	fViewAngle = DEGTORAD(CGeneral::GetAngleBetweenPoints( pEntity->GetPosition().x, pEntity->GetPosition().y, GetPosition().x, GetPosition().y));

	if(fViewAngle > TWO_PI)
	{
		fViewAngle -= TWO_PI;
	}
	else
	if(fViewAngle < 0)
	{
		fViewAngle += TWO_PI;
	}

	fHeadingAngle = m_fCurrentHeading;

	if(fHeadingAngle > TWO_PI)
	{
		fHeadingAngle -= TWO_PI;
	}
	else
	if(fHeadingAngle < 0)
	{
		fHeadingAngle += TWO_PI;
	}

	AngleDiff = ABS(fViewAngle - fHeadingAngle);

	if((AngleDiff < viewAngle)||(AngleDiff > (TWO_PI-viewAngle))) // Entity is within my line of sight (120 deg)
	{
		return(TRUE);
	}
	
	return(FALSE);
}

// if a ped had got out of a car and finds itself in collision with a 
// building or object, we need to check the immediate area and find a place to put them
// out of that collision
// this will be expensive but should very rarely happen
// the ped checlks if its clear before getting out, but it may be the case that
// his car was hit by another and moved while he was getting out
#define OPTIMISED_POSITIONPEDOUTOFCOLLISION
//
bool CPed::PositionPedOutOfCollision(int nDoor, CVehicle* pVehicle, bool bUseNodes)
{
#ifndef FINAL
#ifdef CATCH_SUPERCOP
	m_bHasCalledPositionPedOOC = true;
#endif
#endif
	
#ifdef OPTIMISED_POSITIONPEDOUTOFCOLLISION

	CVehicle* pTestVehicle=pVehicle;
	if(0==pTestVehicle)
	{
		pTestVehicle=m_pMyVehicle;
	}
	
	ASSERT(pTestVehicle);
	if(!pTestVehicle) return false;
	if(m_nPedFlags.bDonePositionOutOfCollision) return true;

	bool bDone = false;
	CColModel &ColModel = pTestVehicle->GetColModel();
	CVector vecCarPosition = pTestVehicle->GetPosition();
	CVector vecOrigPedPos = GetPosition();
	CVector vecTestPedPos = vecOrigPedPos;
	CVector vecZero(0.0f,0.0f,0.0f);
	bool bUsesCollision = GetUsesCollision();
	
	//////////////////////////////////////////////	
	// REMEMBER TO CLEAR THIS OR EVERYTHING WILL BREAK!!
	CWorld::pIgnoreEntity = pTestVehicle;
	// do a special thing where we turn collision off
	SetUsesCollision(false);
	// but set flag so it returns if it detects ANY intersections
	m_nPhysicalFlags.bForceHitReturnFalse = true;
	// zero speed so we don't travel into collision (not really required given above flag)
	m_vecMoveSpeed = vecZero;
	
	// if vehicle's on it's side, try first putting the ped ontop of the side of the car
	if(pTestVehicle->IsOnItsSide() && pTestVehicle->GetBaseVehicleType()!=VEHICLE_TYPE_BIKE)
	{
		vecTestPedPos = vecCarPosition;
		vecTestPedPos.z += ColModel.GetBoundBoxMax().x + PED_GROUNDOFFSET;
		SetPosition(vecTestPedPos);
		
		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
		{
			bDone = true;
			INC_PEDAI_LOS_COUNTER;
		}
	}
	// else do a proper test on the peds exit position
	else if(nDoor!=0)
	{
		vecTestPedPos = CCarEnterExit::GetPositionToOpenCarDoor(*pTestVehicle, nDoor);
		SetPosition(vecTestPedPos);

		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false)) {
		
			bDone = true;	// no way 1st attempt worked!
			INC_PEDAI_LOS_COUNTER;
		}

		// try other door for bikes
		if(!bDone && pTestVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE && (nDoor==CAR_DOOR_LF || nDoor==CAR_DOOR_LR))
		{
			int nOtherDoor = CAR_DOOR_RF;
			if(nDoor==CAR_DOOR_LR)
				nOtherDoor = CAR_DOOR_RR;
		
			vecTestPedPos = CCarEnterExit::GetPositionToOpenCarDoor(*pTestVehicle, nOtherDoor);
			SetPosition(vecTestPedPos);
		
			if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false)) {
			
				bDone = true;	// opposite door is fine!
				INC_PEDAI_LOS_COUNTER;
			}
		}
	}
	
	
	float fCarWidth = ColModel.GetBoundBoxMin().x - 0.355f;

	// need to increase width if vehicle is rolled to side on slope because ped is put in the world standing up vertically
	float fUpDiff  = CMaths::Max(ColModel.GetBoundBoxMax().z, -ColModel.GetBoundBoxMin().z);
	fCarWidth += fUpDiff*CMaths::Abs(pTestVehicle->GetMatrix().GetRight().z);
	
	if(nDoor == CAR_DOOR_RF || nDoor == CAR_DOOR_RR)
	{
		fCarWidth = ColModel.GetBoundBoxMax().x + 0.355f;
	}

	// try moving the ped along to the desired edge of the vehicle's collision
	if(!bDone)
	{
		float fCurrentSide = DotProduct(vecOrigPedPos - vecCarPosition, pTestVehicle->GetMatrix().GetRight());
		vecTestPedPos = vecOrigPedPos + (-fCurrentSide + fCarWidth)*pTestVehicle->GetMatrix().GetRight();
		SetPosition(vecTestPedPos);
//		m_vecMoveSpeed = vecZero + vecTestSpeedSide + vecTestSpeedFwd;

		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
		{
			bDone = true;	// no way 1st attempt worked!
			INC_PEDAI_LOS_COUNTER;
		}
	}
		
	// next try 4 points down that side of the vehicle
	if(!bDone)
	{
		float fStartY = ColModel.GetBoundBoxMin().y;
		float fAddY = (ColModel.GetBoundBoxMax().y - fStartY) / 3.0f;
		for(int i=0; i<4; i++)
		{
			vecTestPedPos = vecCarPosition + fCarWidth*pTestVehicle->GetMatrix().GetRight() + (fStartY + i*fAddY)*pTestVehicle->GetMatrix().GetForward();
			SetPosition(vecTestPedPos);
//			m_vecMoveSpeed = vecZero + vecTestSpeedSide + vecTestSpeedFwd;

			if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
			{
				bDone = true;	// cool, this worked
				INC_PEDAI_LOS_COUNTER;
				break;
			}
		}
	}
	
	// try at the back of the vehicle
	if(!bDone)
	{
		vecTestPedPos = vecCarPosition + (ColModel.GetBoundBoxMin().y - 0.355f)*pTestVehicle->GetMatrix().GetForward();
		SetPosition(vecTestPedPos);
//		m_vecMoveSpeed = vecZero + vecTestSpeedSide - vecTestSpeedFwd;

		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
		{
			bDone = true;
			INC_PEDAI_LOS_COUNTER;
		}
	}

	// try at the front of the vehicle
	if(!bDone)
	{
		vecTestPedPos = vecCarPosition + (ColModel.GetBoundBoxMax().y + 0.355f)*pTestVehicle->GetMatrix().GetForward();
		SetPosition(vecTestPedPos);
//		m_vecMoveSpeed = vecZero + vecTestSpeedSide + vecTestSpeedFwd;

		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
		{
			bDone = true;
			INC_PEDAI_LOS_COUNTER;
		}
	}
	
	// try at other back corner
	if(!bDone)
	{
		vecTestPedPos = vecCarPosition - fCarWidth*pTestVehicle->GetMatrix().GetRight() + ColModel.GetBoundBoxMin().y*pTestVehicle->GetMatrix().GetForward();
		SetPosition(vecTestPedPos);
//		m_vecMoveSpeed = vecZero - vecTestSpeedSide - vecTestSpeedFwd;

		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
		{
			bDone = true;
			INC_PEDAI_LOS_COUNTER;
		}
	}

	// try at other front corner
	if(!bDone)
	{
		vecTestPedPos = vecCarPosition - fCarWidth*pTestVehicle->GetMatrix().GetRight() + ColModel.GetBoundBoxMax().y*pTestVehicle->GetMatrix().GetForward();
		SetPosition(vecTestPedPos);
//		m_vecMoveSpeed = vecZero - vecTestSpeedSide + vecTestSpeedFwd;

		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
		{
			bDone = true;
			INC_PEDAI_LOS_COUNTER;
		}
	}

	// try on top of car
	if(!bDone && pTestVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR)
	{
		vecTestPedPos = vecCarPosition + ColModel.GetBoundBoxMax().z*pTestVehicle->GetMatrix().GetUp();
		vecTestPedPos.z += PED_GROUNDOFFSET;
		SetPosition(vecTestPedPos);
//		m_vecMoveSpeed = vecZero - vecTestSpeedSide + vecTestSpeedFwd;

		if(!CheckCollision() && CWorld::GetIsLineOfSightClear(vecCarPosition, vecTestPedPos, true, false, false, true, false))
		{
			bDone = true;
			INC_PEDAI_LOS_COUNTER;
		}
	}

//	m_pNOCollisionVehicle = pTestVehicle;
	CWorld::pIgnoreEntity = NULL;
	SetUsesCollision(bUsesCollision);
	m_nPhysicalFlags.bForceHitReturnFalse = false;
	
	if(bDone)
	{
		// make sure we only go through this whole effort once per frame!
		m_nPedFlags.bDonePositionOutOfCollision = true;
		
		m_vecMoveSpeed = vecZero;
		m_vecTurnSpeed = vecZero;
		
		if(pTestVehicle->GetBaseVehicleType()!=VEHICLE_TYPE_BIKE || bUseNodes)
		{
			pTestVehicle->m_vecMoveSpeed = vecZero;
			pTestVehicle->m_vecTurnSpeed = vecZero;
			pTestVehicle->m_vecMoveSpeed.z -= 0.05f;
		}
/////////////////////////////////////////////
// not sure we want to place ped at ground height, cause that doesn't check for cars or objects
// so can get placed inside shit.  which is bad.  better use the original position CheckCollision was done at
/////////////////////////////////////////////
		return true;
/**************************************
		// remember to set car and ped's move and turn speed to zero
		CVector vecHighTestPedPos = vecTestPedPos;
		vecTestPedPos.z -= 0.1f;
		vecHighTestPedPos.z += 10.0f;
		float fTempTestWaterLevel = 100.0f;
		if(!CWaterLevel::GetWaterLevel(vecTestPedPos.x, vecTestPedPos.y, vecTestPedPos.z, &fTempTestWaterLevel, true))
			fTempTestWaterLevel = -1001.0f;
		
		if(CPedPlacement::FindZCoorForPed(&vecTestPedPos) && vecTestPedPos.z > fTempTestWaterLevel)
		{
			m_mat.SetTranslate(vecTestPedPos);
			SetHeading(m_pMyVehicle->GetHeading());
			return true;
		}
		else if(CPedPlacement::FindZCoorForPed(&vecHighTestPedPos) && vecHighTestPedPos.z > fTempTestWaterLevel)
		{
			m_mat.SetTranslate(vecHighTestPedPos);
			SetHeading(m_pMyVehicle->GetHeading());
			return true;
		}
		else if(fTempTestWaterLevel > -1000.0f)
		{
			m_mat.SetTranslate(vecTestPedPos);
			SetHeading(m_pMyVehicle->GetHeading());
			return true;
		}
		// no ground (or water) here so not a valid coordinate, need to look for node
		else
			bDone = false;
**************************************/
	}
	// uh oh, didn't find anywhere - look for a node?
	if(!bDone && bUseNodes)
	{
		CNodeAddress ResultPedNode = ThePaths.FindNodeClosestToCoors(vecCarPosition, PFGRAPH_PEDS, 999999.9f, false);
		CNodeAddress ResultCarNode = ThePaths.FindNodeClosestToCoors(vecCarPosition, PFGRAPH_CARS, 999999.9f, false);
		bool	bNearestCoors = false;

		if(!ResultPedNode.IsEmpty())
		{
			bNearestCoors = true;
			vecTestPedPos = ThePaths.FindNodePointer(ResultPedNode)->GetCoors();
		}
		if(!ResultCarNode.IsEmpty())
		{
			bNearestCoors = true;
			if ((!bNearestCoors) || (ThePaths.FindNodePointer(ResultCarNode)->GetCoors() - vecCarPosition).Magnitude2D() < (vecTestPedPos - vecCarPosition).Magnitude2D())
			{
				vecTestPedPos = ThePaths.FindNodePointer(ResultCarNode)->GetCoors();
			}
		}
			//ASSERT((vecTestPedPos - GetPosition()).Magnitude() < 20.0f);
		if (bNearestCoors)
		{
			CPedPlacement::FindZCoorForPed(&vecTestPedPos);
			SetPosition(vecTestPedPos);
			SetHeading(pTestVehicle->GetHeading());

			// make sure we only go through this whole effort once per frame!
			m_nPedFlags.bDonePositionOutOfCollision = true;
		
			m_vecMoveSpeed = vecZero;
			m_vecTurnSpeed = vecZero;
			pTestVehicle->m_vecTurnSpeed = vecZero;
			pTestVehicle->m_vecMoveSpeed = vecZero;
			pTestVehicle->m_vecMoveSpeed.z += 0.02f;
			return true;
		}
	}
	
	return false;
	
#else
	CVector vecPosition, vecCarPosition,vecDiff,vecExtendedExitPoint;
	CVector vecBestPosition, vecBestPositionCar;
	int fBestDistance, fBestDistanceCar;
	float fAmount = 0.5f, fLength, fDistance;
	bool bFound = false;
	bool bFoundCar = false;
	int nSteps = 20;// increased from 15 so that it works with the seaplane (big wide mother)
	CEntity *pCar;
	
	fBestDistance = 999.0f;
	fBestDistanceCar = 999.0f;
	
	ASSERT(m_pMyVehicle);
	if (!m_pMyVehicle) return false;

	vecCarPosition = m_pMyVehicle->GetPosition();
	
	vecPosition.y = GetPosition().y - fAmount*((float)(nSteps - 1))/2.0f;
	vecPosition.z = GetPosition().z;

	for(int y = 0; y < nSteps; y++)
	{
		vecPosition.x = GetPosition().x - fAmount*((float)(nSteps - 1))/2.0f;
		
		for(int x = 0; x < nSteps; x++)
		{
			CPedPlacement::FindZCoorForPed(&vecPosition);
//			vecPosition.z += 0.5f;
			vecDiff = vecPosition - vecCarPosition;
			
			fLength = vecDiff.Magnitude();

			// Extend line a wee bit	
			vecExtendedExitPoint = vecCarPosition + (vecDiff) * ((fLength + 0.6f) / fLength);

			if (CWorld::GetIsLineOfSightClear(vecCarPosition, vecExtendedExitPoint, true, false, false, true, false)
			&&(!CWorld::TestSphereAgainstWorld(vecPosition, 0.6f, this, true, false, false, true, false)))
			{
				INC_PEDAI_LOS_COUNTER;
				
				fDistance = CVector(vecPosition-GetPosition()).MagnitudeSqr();
				// does this spot collide with a car
				pCar = CWorld::TestSphereAgainstWorld(vecPosition, 0.6f, this, false, true, false, false, false);
				
				if(!pCar)
				{
					// no cars collided with
					if(fDistance < fBestDistance)
					{
						fBestDistance = fDistance;
						vecBestPosition = vecPosition;
						bFound = true;
					}
				}
				else
				{
					// this is inside a car (not a problem, but would rather have one not in a car)
					if(fDistance < fBestDistanceCar)
					{
						fBestDistanceCar = fDistance;
						vecBestPositionCar = vecPosition;
						bFoundCar = true;
					}
				}
								
			}

			vecPosition.x += fAmount;
		}

		vecPosition.y += fAmount;
	}
	
	if(bFound || bFoundCar)
	{
		// set to a non in car position if possible
		if(bFound)
			SetPosition(vecBestPosition);
		else
		{
			CVector bBoxVMax = pCar->GetColModel().GetBoundBoxMax();
			vecBestPositionCar.z += bBoxVMax.z;
			SetPosition(vecBestPositionCar);
		}
		return true;
	}	
	
	return false;
#endif
}

bool CPed::PositionAnyPedOutOfCollision()
{
	CVector vecPosition, vecOldPosition, vecDiff;
	CVector vecBestPosition, vecBestPositionCar;
	int fBestDistance, fBestDistanceCar;
	float fAmount = 0.5f, fDistance;
	bool bFound = false;
	bool bFoundCar = false;
	int nSteps = 15;
	CEntity *pCar;
	
	fBestDistance = 999.0f;
	fBestDistanceCar = 999.0f;
	
	vecOldPosition = GetPosition();
	
	vecPosition.y = GetPosition().y - fAmount*((float)(nSteps - 1))/2.0f;
	vecPosition.z = GetPosition().z;

	for(int y = 0; y < nSteps; y++)
	{
		vecPosition.x = GetPosition().x - fAmount*((float)(nSteps - 1))/2.0f;
		
		for(int x = 0; x < nSteps; x++)
		{
			CPedPlacement::FindZCoorForPed(&vecPosition);

			vecDiff = vecPosition - vecOldPosition;
			
			if (!CWorld::TestSphereAgainstWorld(vecPosition, 0.6f, this, true, false, false, true, false))
			{
				fDistance = CVector(vecPosition-GetPosition()).MagnitudeSqr();
				// does this spot collide with a car
				pCar = CWorld::TestSphereAgainstWorld(vecPosition, 0.6f, this, false, true, false, false, false);
				
				if(!pCar)
				{
					// no cars collided with
					if(fDistance < fBestDistance)
					{
						fBestDistance = fDistance;
						vecBestPosition = vecPosition;
						bFound = true;
					}
				}
				else
				{
					// this is inside a car (not a problem, but would rather have one not in a car)
					if(fDistance < fBestDistanceCar)
					{
						fBestDistanceCar = fDistance;
						vecBestPositionCar = vecPosition;
						bFoundCar = true;
					}
				}
								
			}

			vecPosition.x += fAmount;
		}

		vecPosition.y += fAmount;
	}
	
	if(bFound || bFoundCar)
	{
		// set to a non in car position if possible
		if(bFound)
			SetPosition(vecBestPosition);
		else
		{
			CVector bBoxVMax = pCar->GetColModel().GetBoundBoxMax();
			
			vecBestPositionCar.z += bBoxVMax.z;

			SetPosition(vecBestPositionCar);
		}
		return true;
	}	
	
	return false;
}


// Name			:	OurPedCanSeeThisEntity (formerly OurPedCanSeeThisOne)
// Purpose		:	Works out whether this ped can see the other entity
// Parameters	:	None
// Returns		:	Nothing
bool CPed::OurPedCanSeeThisEntity(CEntity *pEntity, bool bForTargetingPurposes)
{
	float	fDotProduct;
	CVector2D	vec2DTargetEntity;	//	vec2DPedHeading, 
	CVector vecMyPos, vecTargetPos;

//	vec2DPedHeading.x = ANGLE_TO_VECTOR_X(RADTODEG(GetPedHeading()));
//	vec2DPedHeading.y = ANGLE_TO_VECTOR_Y(RADTODEG(GetPedHeading()));

	vec2DTargetEntity.x = pEntity->GetPosition().x - GetPosition().x;
	vec2DTargetEntity.y = pEntity->GetPosition().y - GetPosition().y;

//	fDotProduct = vec2DPedHeading.x * vec2DTargetPed.x + vec2DPedHeading.y * vec2DTargetPed.y;
	fDotProduct = GetMatrix().GetForward().x * vec2DTargetEntity.x + GetMatrix().GetForward().y * vec2DTargetEntity.y;

	if(bForTargetingPurposes || ((fDotProduct >= 0.0f)&&(vec2DTargetEntity.Magnitude() < PED_CAN_SEE_RANGE)))
	{
		// now process line of sight
		CColPoint colPoint;
		CEntity* pHitEntity;

		vecMyPos = GetPosition();
		vecMyPos.z += 1.0f;// look from head
		
		vecTargetPos = pEntity->GetPosition();
		if (pEntity->GetType() == ENTITY_TYPE_PED)
		{
			vecTargetPos.z += 1.0f; // check to see if can shoot head (might be behind a wall, but still a visible target)
		}
		if (bForTargetingPurposes)
		{
			INC_PEDAI_LOS_COUNTER;
			
			// check buildings only. Make sure we can target stuff behind bShootThroughStuff.
			if(CWorld::ProcessLineOfSight(vecMyPos, vecTargetPos , colPoint, pHitEntity, 1,0,0,1 /*changed Obr 22 Aug. Might have to change back*/,0, false, false, true))
			{
				return false;
			}
		}
		else
		{
			INC_PEDAI_LOS_COUNTER;
			
			// check buildings only
			if(CWorld::ProcessLineOfSight(vecMyPos, vecTargetPos , colPoint, pHitEntity, 1,0,0,0,0))
			{
				return false;
			}
		}
		

		return true;
	}
	return false;
}



// Name			:	SortPeds
// Purpose		:	Quicksorts ped list into ascending order
// Parameters	:	apPeds - array of ptrs to peds we want to sort
//					nStartIndex, nEndIndex - start and end points to sort between
// Returns		:	Nothing

void CPed::SortPeds(CPed* apPeds[], int32 nStartIndex, int32 nEndIndex)
{
	if (nStartIndex < nEndIndex)
	{
		int32 i = nStartIndex;
		int32 j = nEndIndex;

		float fDistance = CVector(GetPosition() - apPeds[(i + j) / 2]->GetPosition()).Magnitude();

		do
		{
			while (CVector(GetPosition() - apPeds[i]->GetPosition()).Magnitude() < fDistance)
			{
				i++;
			}

			while (fDistance < CVector(GetPosition() - apPeds[j]->GetPosition()).Magnitude())
			{
				j--;
			}

			if (i <= j)
			{	
				CPed* pTempPed = apPeds[i];
				apPeds[i] = apPeds[j];
				apPeds[j] = pTempPed;
				i++;
				j--;
			}
		}
		while (i <= j);

		SortPeds(apPeds, nStartIndex, j);
		SortPeds(apPeds, i, nEndIndex);
	}
} // end - CPed::SortPeds



#define PED_MOVESTATE_BLENDDELTA	1.0f
void CPed::SetMoveState(eMoveState MoveState)
{ 
	m_eMoveState = MoveState;
}

void CPed::SetMoveAnim()
{
	if(PED_DIE==m_nPedState || PED_DEAD==m_nPedState)
	{
		return;
	}
	
	if(m_nPedFlags.bIsDucking || m_pAttachToEntity)
		return;

	CAnimBlendAssociation* pAnim = NULL;
	if(m_eMoveStateAnim == m_eMoveState)
	{
		if(m_eMoveState >= PEDMOVE_WALK && (m_ik.GetSlopeAngle() > 0.01f || m_ik.GetSlopeAngle() < -0.01f))
		{
			AnimationId nAnim = ANIM_STD_WALK;
			if(m_eMoveState==PEDMOVE_RUN)	nAnim = ANIM_STD_RUN;
			else if(m_eMoveState==PEDMOVE_SPRINT)	nAnim = ANIM_STD_RUNFAST;
			
			pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, nAnim);
			if(pAnim && !m_nPedFlags.bMoveAnimSpeedHasBeenSetByTask)
			{
				SetMoveAnimSpeed(pAnim);
				
				/*
				if(CharCreatedBy == MISSION_CHAR)
					pAnim->SetSpeed(1.0f + MAX(-0.3f, MIN(0.3f, m_ik.GetSlopeAngle())));
				else
					pAnim->SetSpeed(1.2f + MAX(-0.3f, MIN(0.3f, m_ik.GetSlopeAngle())) - (0.4f * RandomSeed) / RAND_MAX_16);
				*/
			}
		}
	   	return;
    }
	if(m_eMoveState==PEDMOVE_NONE){
		m_eMoveStateAnim = PEDMOVE_NONE;
		return;
	}

	AssocGroupId MotionAnimGroup;
//	CAnimBlendAssociation *pAnim2;

	
	/*if((pLeader)&&(pLeader->IsPlayer()))
		MotionAnimGroup = ANIM_PLAYER_PED;
	else*/
		MotionAnimGroup = m_motionAnimGroup;

	// check if any non-interrupt anims are running
	pAnim = NULL;//RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_PEDFLAG_DONTINTERRUPT);
	
/*	// if no non-interrupt anims, then check if FightIdle is running
	if(pAnim == NULL){
		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_MELEE_IDLE);
		// if FightIdle, must've not ended fight properly or don't want to end fight and wander off
		if(pAnim && GetPedIntelligence()->GetTaskFighting()){
			//EndFight(2);
			return;
		}
		// or state changed but anim wasn't removed
		else if(pAnim){
			pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);
//			printf("Fight Idle Still Here! %p\n", this);
			if(!pAnim2 || !(pAnim2->GetBlendDelta()>0))
			{
				// get rid of fight idle
				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				// blend normal idle over top
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_IDLE, 8.0f);
			}
		}
	}

	// if no non-interrupt anims, then check if WeaponStance anims are running
	if(pAnim == NULL){
		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_WEAPON_STANCE);
		if(!pAnim) pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_WEAPON_FWD);
		if(!pAnim) pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_WEAPON_LEFT);
		if(!pAnim) pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_WEAPON_BACK);
		if(!pAnim) pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_WEAPON_RIGHT);
		// if FightIdle, must've not ended fight properly or don't want to end fight and wander off
		if(pAnim && GetPedIntelligence()->GetTaskUseGun()){
			//EndFight(2);
			return;
		}
		// or state changed but anim wasn't removed
		else if(pAnim){
			pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);
//			printf("Fight Idle Still Here! %p\n", this);
			if(!pAnim2 || !(pAnim2->GetBlendDelta()>0))
			{
				// get rid of fight idle
				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				// blend normal idle over top
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_IDLE, 8.0f);
			}
		}
	}
*/
	/*
	// do the same for the 1 other non-partial anim - IDLE_TIRED!
	if(pAnim == NULL){
		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE_TIRED);
		// if IdleTired, check if valid cause to use anim
		if(pAnim && false){//(m_eWaitState == PEDWAIT_STUCK || m_eWaitState == PEDWAIT_FINISH_FLEE)){
			return;
		}
		// if not then maybe conditions changed but anim wasn't removed
		else if(pAnim){
			pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);
			if(!pAnim2 || !(pAnim2->GetBlendDelta()>0))
			{
				// get rid of fight idle
				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				// blend normal idle over top
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_IDLE, 4.0f);
			}
		}
	}
	*/
	/*
	// do the same for the 1 other non-partial anim - IDLE_TIRED!
	if(pAnim == NULL){
		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_HANDSUP);
		// if IdleTired, check if valid cause to use anim
		if(pAnim && m_eWaitState == PEDWAIT_PLAYANIM_HANDSUP){
			return;
		}
		// if not then maybe conditions changed but anim wasn't removed
		else if(pAnim){
			CAnimBlendAssociation *pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);
			if(!pAnim2 || !(pAnim2->GetBlendDelta()>0))
			{
				// get rid of fight idle
				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				// blend normal idle over top
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_IDLE, 4.0f);
			}
		}
	}
	// do the same for the 1 other non-partial anim - IDLE_TIRED!
	if(pAnim == NULL){
		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_HANDSCOWER);
		// if IdleTired, check if valid cause to use anim
		if(pAnim && m_eWaitState == PEDWAIT_PLAYANIM_HANDSCOWER){
			return;
		}
		// if not then maybe conditions changed but anim wasn't removed
		else if(pAnim){
			CAnimBlendAssociation *pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);
			if(!pAnim2 || !(pAnim2->GetBlendDelta()>0))
			{
				// get rid of fight idle
				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				// blend normal idle over top
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_IDLE, 4.0f);
			}
		}
	}
	*/
	if(pAnim == NULL)  // no anims running, can then add required move anim
	{
		m_eMoveStateAnim = m_eMoveState;
		
		// If moving remove all partial animations and stop looking/aiming at stuff
		if(m_eMoveState == PEDMOVE_WALK || m_eMoveState == PEDMOVE_RUN || m_eMoveState == PEDMOVE_SPRINT)
		{
			// RpAnimBlendClumpRemoveAssociations((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL);
			// lets try and remove partial anims a little more elegantly so they can still be seen a bit
			CAnimBlendAssociation *pTempAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL);
			while(pTempAnim)
			{
				if(!pTempAnim->IsFlagSet(ABA_FLAG_ISFINISHAUTOREMOVE) && !pTempAnim->IsFlagSet(ABA_PEDFLAG_UPPERBODYONLY))
				{
					pTempAnim->SetBlendDelta(-2.0f);
					pTempAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				}
				pTempAnim = RpAnimBlendGetNextAssociation(pTempAnim, ABA_FLAG_ISPARTIAL);
			}
		
			ClearAimFlag();
			ClearLookFlag();
		}
		
		switch(m_eMoveState)
		{
			case PEDMOVE_STILL:
				//if(m_nHealth < PED_TIRED_MAXHEALTH)
				//	pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_IDLE_TIRED, 4.0f);
				//else
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_IDLE, 4.0f);
				break;
			case PEDMOVE_TURN_L:
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_TURN_L, 16.0f);
				break;
			case PEDMOVE_TURN_R:
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_TURN_R, 16.0f);
				break;
			case PEDMOVE_WALK:
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_WALK, 1.0f);
				break;
			case PEDMOVE_RUN:
				{
					if(m_nPedState==PED_FLEE_ENTITY)
						pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_RUN, 3.0f);
					else
						pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_RUN, 1.0f);
				}
				break;
			case PEDMOVE_SPRINT:
				if(CPedGroups::IsInPlayersGroup(this) && CPedGroups::GetPedsGroup(this)->GetGroupMembership()->GetLeader()
				&& CPedGroups::GetPedsGroup(this)->GetGroupMembership()->GetLeader()->GetMoveState() >= PEDMOVE_RUN)
					pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_PLAYER_PED, ANIM_STD_RUNFAST, 1.0f);
				else
					pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, MotionAnimGroup, ANIM_STD_RUNFAST, 1.0f);
				break;
			default:
				break;
		}
		if(pAnim && !m_nPedFlags.bMoveAnimSpeedHasBeenSetByTask)
		{
			SetMoveAnimSpeed(pAnim);
			
			/*
			if(CharCreatedBy == MISSION_CHAR)
				pAnim->SetSpeed(1.0f + MAX(-0.3f, MIN(0.3f, m_ik.GetSlopeAngle())));
			else
				pAnim->SetSpeed(1.2f + MAX(-0.3f, MIN(0.3f, m_ik.GetSlopeAngle())) - (0.4f * RandomSeed) / RAND_MAX_16);
			*/
		}
	}
}

void
CPed::SetMoveAnimSpeed(CAnimBlendAssociation * pAnim)
{
	if(CharCreatedBy == MISSION_CHAR)
	{
		pAnim->SetSpeed(1.0f + MAX(-0.3f, MIN(0.3f, m_ik.GetSlopeAngle())));
		}
	else
	{
		pAnim->SetSpeed(1.2f + MAX(-0.3f, MIN(0.3f, m_ik.GetSlopeAngle())) - (0.4f * RandomSeed) / RAND_MAX_16);
	}
}

					
//
// name:		StopNonPartialAnims/RestartNonPartialAnims
// description:	Two functions to stop or restart non-partial anims running. There is no point 
// 				non partial anims running on a ped who is sitting in a car/dead etc
void CPed::StopNonPartialAnims()
{
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject);
	while(pAnim)
	{
		if(!pAnim->IsFlagSet(ABA_FLAG_ISPARTIAL))
			pAnim->ClearFlag(ABA_FLAG_ISPLAYING);
		pAnim = RpAnimBlendGetNextAssociation(pAnim);
	}
}
void CPed::RestartNonPartialAnims()
{
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject);
	while(pAnim)
	{
		if(!pAnim->IsFlagSet(ABA_FLAG_ISPARTIAL))
			pAnim->SetFlag(ABA_FLAG_ISPLAYING);
		pAnim = RpAnimBlendGetNextAssociation(pAnim);
	}
}

/*
// Name			:	SetStoredState
// Purpose		:	
// Parameters	:	None
// Returns		:	nothing

void CPed::SetStoredState()
{
	// if the current ped state is one which has already recorded the old state
	// and we are asked to change the state again, we don't set the stored state
	// to the current state, just keep the original one
	//
	// We might change that though
	//

	if((m_nPedStoredState != PED_NONE))
		return;

	// these are states that you can't go back to, so should not be stored
	if(!CanPedReturnToState())
		return;
	

//	if(m_nPedState==PED_WANDER_PATH){
//		m_nPedFlags.bRestorePathNodes = true;
//		if(m_eMoveState <= PEDMOVE_STILL)
//			m_eMoveState = PEDMOVE_WALK;	
//	}
	
	//Never return to the idle state. 
	if(m_nPedState==PED_IDLE)
	{
	    return;
	}
		
	m_nPedStoredState = m_nPedState;
	if(m_eMoveState >= m_eStoredMoveState)
		m_eStoredMoveState = m_eMoveState;

}
*/

/*
// Name			:	RestorePreviousState
// Purpose		:	Sets a ped back to its stored previous state. Not all states are covered here yet
// Parameters	:	None
// Returns		:	Nothing

void CPed::RestorePreviousState()
{
	if(!CanSetPedState() || m_nPedState==PED_FALL || (m_nPedState==PED_GETUP && !m_nPedFlags.bGetUpAnimStarted))
		return;
		
	ASSERT(m_nPedStoredState != PED_DRIVING);

	if(m_nPedFlags.bInVehicle && m_pMyVehicle)
	{
		ASSERT(m_pMyVehicle);
		SetPedState(PED_DRIVING);
		m_nPedStoredState = PED_NONE;
		return;
	}
	
 	if(m_nPedStoredState == PED_NONE)
	{
		if((!IsPlayer()) && (!(CharCreatedBy == MISSION_CHAR)) && (m_Objective == NO_OBJ))
		{
			if(!SetWanderPath(CGeneral::GetRandomNumber() % 8))
				SetIdle();	// fallback incase SetWanderPath doesn't work!
		}
		else
		{
			SetIdle();
		}
		return;
	}	
	//ASSERTMSG(m_nPedStoredState != PED_NONE, "Trying to restore null ped state");
#ifdef GTA_NETWORK
	if (m_nPedState == PED_DRIVING)
	{
		SetIdle();
		return;	
	}
#endif

	ASSERT(m_nPedState != PED_DRIVING);
	
	switch(m_nPedStoredState)
	{
	case PED_IDLE:
		SetIdle();
		break;
	case PED_WANDER_PATH:
		ASSERT(m_nPedState != PED_DRIVING);
		const ePedState redundantPedState=m_nPedState;
		SetPedState(PED_WANDER_PATH);
		m_nPedFlags.bForceRun = false;
		if( m_nPedFlags.bRestorePathNodes && (!m_NextWanderNode.IsEmpty()) && 
			(ThePaths.TakeWidthIntoAccountForWandering(m_NextWanderNode, RandomSeed) - GetPosition()).MagnitudeSqr() < PED_RESTORE_WANDER_NODE_RANGE*PED_RESTORE_WANDER_NODE_RANGE ){
			// just set walking again and use node that's already there
			SetMoveState(PEDMOVE_WALK);
		}
		else if(PED_FOLLOW_PATH==redundantPedState)
		{
		    //If the ped has just finished negotiating an obstacle
		    //then set it back onto the old wander path.
		    SetWanderPath(m_nPathWanderDirection);
		}
		else
		{
			SetWanderPath(CGeneral::GetRandomNumber() % 8);
		}
		break;
	default:
		SetPedState(m_nPedStoredState);
		SetMoveState(m_eStoredMoveState);
		break;
	}
	m_nPedStoredState = PED_NONE;

} // end - CPed::RestorePreviousState
*/


/*
// Name			:	ScanForThreats
// Purpose		:	Checks area surrounding ped for visible threats (e.g. other peds
//					with guns, gang members etc.) or audible threats (gunshots,explosions) and triggers flee behaviour if necessary
// Parameters	:	None
// Returns		:	Nothing

int32 CPed::ScanForThreats()
{
	int32 nPotentialThreat = 0;
	CVector2D vecThreatPos, vecToThreat;
	float fClosestThreatDistance;
	CPed *pThreatPed = NULL;

	// make temp copy of what this ped is scared of
	int32 nConsideredThreats = m_nThreats;//CPedType::GetPedTypeThreats(GetPedType());

	fClosestThreatDistance = 60.0f; // big distance away
	vecThreatPos = GetPosition();
	// scan through nearby visible peds and see if our ped is scared of
	// any of them ~dodgy~ need to consider cars, explosions etc. too	
	
	// check for explosions
	if(nConsideredThreats & PED_FLAG_EXPLOSION)
	{
		if(CheckForExplosions(vecThreatPos))
		{
			m_vec2DThreatPosition = vecThreatPos;

			return(PED_FLAG_EXPLOSION);
		}
	}
	// check for gun shots

	if(nConsideredThreats & PED_FLAG_GUN)
	{
		pThreatPed = CheckForGunShots();
		
		// if we found a threat ped, and he's not the same type as us (covers ourselves). Don't want same gang/cops shooting each other
		if(pThreatPed)
		{
			if(GetPedType() != pThreatPed->GetPedType() || GetPedType()==PEDTYPE_CIVMALE || GetPedType()==PEDTYPE_CIVFEMALE)
			{
				AssertPedPointerValid(pThreatPed);
				if (!IsGangMember())
				{
					// ok, react to closest gunshot threats
					m_pThreatEntity = pThreatPed;
					m_pThreatEntity->RegisterReference(&m_pThreatEntity);

					return(PED_FLAG_GUN);		
				}
				// gang peds don't consider gun shots threats unless the ped shooting is considered a threat
				//(except security guards- gang 5)
				if (nConsideredThreats & CPedType::GetPedFlag(pThreatPed->GetPedType()) || GetPedType() == PEDTYPE_GANG5)
				{
					// ok, react to ped who shot the gun
					TIDYREF(m_pThreatEntity, &m_pThreatEntity);
					m_pThreatEntity = pThreatPed;
					m_pThreatEntity->RegisterReference(&m_pThreatEntity);

					return CPedType::GetPedFlag(pThreatPed->GetPedType());		
				}
			}
		}
	}
	
	// check for any recently dead peds
	if((nConsideredThreats & PED_FLAG_DEADPEDS)&&(CharCreatedBy != MISSION_CHAR))
	{
		ASSERT(!IsGangMember());
		pThreatPed = CheckForDeadPeds();
		
		if(pThreatPed)
		{
			CVector vecRange = pThreatPed->GetPosition() - GetPosition();
			if(vecRange.MagnitudeSqr() < PED_NOTICE_DEAD_PED_RANGE*PED_NOTICE_DEAD_PED_RANGE)
			{
				m_pInterestingEntity = (CEntity *)pThreatPed;
				m_pInterestingEntity->RegisterReference(&m_pInterestingEntity);
		
				return(PED_FLAG_DEADPEDS);
			}
		}
	}
	
	// check cars
	
	//if no shots/explosions check for other peds which are considered a threat
	int32 i = 0;
	nPotentialThreat = 0;
	pThreatPed = NULL;
	bool bGuyAttackingMe = false;
	bool bGuyAttackingMeInSight = false;
	CPed *pAttackingPed = NULL;
	float fClosestAttackingPedDistance = 60.f;
	
	// only do this every 4 frames to avoid too many LOS checks
	if( (CTimer::m_FrameCounter + RandomSeed + 16) &4 )
	for (i = 0; i < m_NumClosePeds; i++)
	{
		if(CharCreatedBy == RANDOM_CHAR && m_apClosePeds[i]->CharCreatedBy == MISSION_CHAR && (m_apClosePeds[i]->IsPlayer() == FALSE) )
			continue;

		nPotentialThreat = CPedType::GetPedFlag(m_apClosePeds[i]->GetPedType());

		// don't check against our ped
		if ((nConsideredThreats & nPotentialThreat) && (m_apClosePeds[i]->m_nHealth > 0))
		{
			vecToThreat = (CVector2D::ConvertTo2D(m_apClosePeds[i]->GetPosition())) - GetPosition();

			if(OurPedCanSeeThisOne(m_apClosePeds[i],m_nPedFlags.bCheckObjectObstaclesOnThreatSearch))
			{
				// check for threats
				// he does, flag the threat
				// I can see this guy, and he is attacking me, top priority
				if(m_apClosePeds[i]->GetPedIntelligence()->GetTaskSecondaryAim()
					||m_apClosePeds[i]->GetPedIntelligence()->GetTaskSecondaryWeaponRanged()
					||m_apClosePeds[i]->GetPedIntelligence()->GetTaskSecondaryFight())
				{
					if(m_apClosePeds[i]->m_pTargetEntity == this)
					{
						if(vecToThreat.MagnitudeSqr() < fClosestAttackingPedDistance*fClosestAttackingPedDistance)
						{
							fClosestThreatDistance = vecToThreat.Magnitude();
							fClosestAttackingPedDistance = fClosestThreatDistance;
							pThreatPed = m_apClosePeds[i];
							AssertPedPointerValid(pThreatPed);
							bGuyAttackingMeInSight = true;
						}
					}
					else // I can see him, but I'm in no danger yet- not such a high priority
					if(vecToThreat.MagnitudeSqr() < fClosestThreatDistance*fClosestThreatDistance && !bGuyAttackingMeInSight)
					{
						fClosestThreatDistance = vecToThreat.Magnitude();
						pThreatPed = m_apClosePeds[i];
						AssertPedPointerValid(pThreatPed);
					}
				}
			}
			else // can't see the guy (facing wrong way or guy is hidden), but can we hear him?
			if(!bGuyAttackingMeInSight) // want to kill the guys I can see that are attacking me first
			{
				if(m_apClosePeds[i]->GetPedIntelligence()->GetTaskSecondaryAim()
					||m_apClosePeds[i]->GetPedIntelligence()->GetTaskSecondaryWeaponRanged()
					||m_apClosePeds[i]->GetPedIntelligence()->GetTaskSecondaryFight())
				{
					CColPoint colPoint;
					CEntity* pHitEntity;
					// if there is a line of sight available to him
					if(!CWorld::ProcessLineOfSight(GetPosition(), m_apClosePeds[i]->GetPosition(), colPoint, pHitEntity, 1,0,0,m_nPedFlags.bCheckObjectObstaclesOnThreatSearch,0))
					{			
						if(m_apClosePeds[i]->m_pTargetEntity == this)
						{
							// this guy is attacking me, he takes priority
							// go for the closest one
							if(vecToThreat.MagnitudeSqr() < fClosestAttackingPedDistance*fClosestAttackingPedDistance)
							{
								fClosestThreatDistance = vecToThreat.Magnitude();
								fClosestAttackingPedDistance = fClosestThreatDistance;
								pThreatPed = m_apClosePeds[i];
								AssertPedPointerValid(pThreatPed);
								bGuyAttackingMe = true;
							}
						}
						else // he's not attacking me but he's shooting a gun
						if(!m_apClosePeds[i]->GetWeapon()->IsTypeMelee() && !bGuyAttackingMe)
						{
							if(vecToThreat.MagnitudeSqr() < fClosestThreatDistance*fClosestThreatDistance)
							{
								fClosestThreatDistance = vecToThreat.Magnitude();
								pThreatPed = m_apClosePeds[i];
								AssertPedPointerValid(pThreatPed);
								bGuyAttackingMe = true;
							}
							
						}
					}
				}
			}
			
		}
	}
	CEntity *ppResults[8]; // whatever.. temp list
	CVehicle *pVehicle;
	int16 Num;

	// check for threat peds in cars
	CWorld::FindObjectsInRange(CVector(GetPosition()), 20.0f, true, &Num, 6, ppResults, 0, 1, 0, 0, 0);	// Just Cars we're interested in
	// cars in range stored in ppResults
	for(i=0; i<Num; i++) // find best car
	{
		pVehicle = (CVehicle *)ppResults[i];
		if(pVehicle->pDriver)
		{
			nPotentialThreat = CPedType::GetPedFlag(pVehicle->pDriver->GetPedType());
			if ((nConsideredThreats & nPotentialThreat) && (pVehicle->pDriver->m_nHealth > 0)
				 && OurPedCanSeeThisOne(pVehicle->pDriver))
			{
				vecToThreat = (CVector2D::ConvertTo2D(pVehicle->GetPosition())) - GetPosition();

				if(vecToThreat.MagnitudeSqr() < fClosestThreatDistance*fClosestThreatDistance)
				{
					fClosestThreatDistance = vecToThreat.Magnitude();
					pThreatPed = pVehicle->pDriver;
					AssertPedPointerValid(pThreatPed);
				}
			}
		}
	}
	
	
	m_pThreatEntity = pThreatPed;
	if (m_pThreatEntity) m_pThreatEntity->RegisterReference(&m_pThreatEntity);
	
	return(nPotentialThreat);

} // end - ScanForThreats
*/


/*
void CPed::ScanForDelayedResponseThreats()
{
    //If the ped isn't waiting to respond to a threat 
    //then look for a fresh threat.
    if(0==m_iDelayedThreatType)
    {
        m_pThreatEntity=0;
        m_pInterestingEntity=0;
        m_iDelayedThreatType=ScanForThreats();
        
        //If there is a threat then set the response timer.
        if(m_iDelayedThreatType)
        {
            //If there is a threat but no threat entity then there isn't really a threat.
            if((0==m_pThreatEntity)&&(0==m_pInterestingEntity))
            {
                //No threat entity so no delayed threat.
                m_iDelayedThreatType=0;
                m_iDelayedThreatTimer=0;
            }
            else
            {
                //Threat entity so set the delayed threat timer.
	            m_iDelayedThreatTimer=CTimer::GetTimeInMilliseconds()+m_iDelayedThreatResponseTime;
            }
        }
        else
        {
            m_iDelayedThreatTimer=0;
        }
    }
}
*/

/*
void CPed::CheckThreatValidity()
{
    //Check that the threat entity is still valid.
    if(m_pThreatEntity)
    {
        if(!IsEntityPointerValid(m_pThreatEntity))
        {
            m_iDelayedThreatType=0;
            m_pThreatEntity=0;
        }
    }
   
    //Check that the interesting entity is still valid.
    if(m_pInterestingEntity)
    {
        if(!IsEntityPointerValid(m_pInterestingEntity))
        {
            m_iDelayedThreatType=0;
            m_pInterestingEntity=0;
        }
    }
    
    if((0==m_pThreatEntity)&&(0==m_pInterestingEntity))
    {
        m_iDelayedThreatType=0;
    }
}
*/

// Name			:	ScanForAudibleThreats
// Purpose		:	Checks area surrounding ped for audible threats (e.g. gunshots,
//					explosions, screams etc.) and triggers flee behaviour if necessary
// Parameters	:	None
// Returns		:	Nothing
/*
void CPed::ScanForAudibleThreats()
{
	int32 nThreats = 0;
	CVector2D vecPedPos, vecThreatPos;
	eWeaponState PedWeaponState;

	if(!IsPedInControl())
		return;
		
	// make temp copy of what this ped is scared of
	int32 nScaredBy = CPedType::GetPedTypeThreats(GetPedType());

	if (nScaredBy == 0)
	{
		// this ped is hard as nails and fears nothing - finished here
		return;
	}

	// scan through nearby peds and see if our ped is scared of any
	// of them ~dodgy~ need to consider cars, explosions etc. too	
	for (Int32 i = 0; i < m_NumClosePeds; i++)
	{
		// don't check against our ped
		if (m_apClosePeds[i] != this)
		{
			nThreats = 0;
			
			// check for scary peds
			nThreats = CPedType::GetPedFlag(m_apClosePeds[i]->GetPedType());

			// check for scary peds with guns
			PedWeaponState = m_apClosePeds[i]->GetWeapon()->GetWeaponState();

			// if the current ped is firing a gun, add that to current threats
			if (PedWeaponState == WEAPONSTATE_FIRING)
			{
				nThreats |= PED_FLAG_GUN;
			}

			// check if our ped is scared of the current ped
			if (CPedType::GetPedTypeThreats(GetPedType()) & nThreats)
			{		
				// he is - run like fuck
				AssertPedPointerValid(m_apClosePeds[i]);
				SetFlee(CVector2D::ConvertTo2D(m_apClosePeds[i]->GetPosition()), 10000);
			}
		}
	}
} // end - ScanForAudibleThreats
*/

#define ADAMS_THIS_IS_AN_INVALID_LOOK_HEADING (999999.0f)

//
// name:		CanUseTorsoWhenLooking
// description:	Returns if ped can use its torso when looking
bool CPed::CanUseTorsoWhenLooking()
{
	if(m_nPedState == PED_DRIVING || m_nPedState==PED_DRAGGED_FROM_CAR ||
		m_nPedFlags.bIsDucking/* ||
		m_motionAnimGroup==ANIM_SEXY_WOMANPED || m_motionAnimGroup==ANIM_STD_WOMANPED*/)
		return false;
	return true;	
	
	
}

// Name			:	SetLookFlag
// Purpose		:	Sets ped look flag, look at position or entity
// Parameters	:	fLookHeading, pLookEntity - position or entity to look at
//					isPersistant - whether to continue looking if peds neck is strained
void CPed::SetLookFlag(float fLookHeading, bool isPersistant, bool bOverride)
{
	if(m_nLookTimer  < CTimer::GetTimeInMilliseconds() || bOverride)
	{
		m_nPedFlags.bIsLooking = true;
		m_nPedFlags.bIsRestoringLook = false;
		m_fLookHeading = fLookHeading;
		TIDYREF(m_pEntLookEntity, &m_pEntLookEntity);
		m_pEntLookEntity = NULL;
		m_nLookTimer = 0;
		//m_nPedFlags.bIsPersistantLooking = isPersistant;

		// this flag might've been cleared because ped was in a car
		if(CanUseTorsoWhenLooking())
			m_ik.ClearFlag(PEDIK_TORSO_USED);
	}
}

void CPed::SetLookFlag(CEntity* pLookEntity, bool bIsPersistant, bool bOverride)
{
	if(m_nLookTimer  < CTimer::GetTimeInMilliseconds() || bOverride)
	{
		m_nPedFlags.bIsLooking = true;
		m_nPedFlags.bIsRestoringLook = false;
		AssertEntityPointerValid(pLookEntity);
		TIDYREF(m_pEntLookEntity, &m_pEntLookEntity);
		m_pEntLookEntity = pLookEntity;
		REGREF(m_pEntLookEntity, &m_pEntLookEntity);
		m_fLookHeading = ADAMS_THIS_IS_AN_INVALID_LOOK_HEADING;
	//	m_pEntLookEntity->SetIsReferenced(true);
		m_nLookTimer = 0;
		//m_nPedFlags.bIsPersistantLooking = bIsPersistant;
		
		// this flag might've been cleared because ped was in a car
		if(CanUseTorsoWhenLooking())
			m_ik.ClearFlag(PEDIK_TORSO_USED);
	}
}

// Name			:	SetLookFlag
// Purpose		:	Stops ped looking
void CPed::ClearLookFlag()
{
	if(m_nPedFlags.bIsLooking)
	{
		m_nPedFlags.bIsLooking = false;
		m_nPedFlags.bIsRestoringLook = true;
		m_nPedFlags.bShakeFist = false;
		
		// this flag might've been cleared because ped was in a car
		if(CanUseTorsoWhenLooking())
			m_ik.ClearFlag(PEDIK_TORSO_USED);
		
//		m_pEntLookEntity = NULL;
//		m_fLookHeading = ADAMS_THIS_IS_AN_INVALID_LOOK_HEADING;
		if(IsPlayer())
			m_nLookTimer = (CTimer::GetTimeInMilliseconds() + 2000); // used to prevent ped from looking again
		else
			m_nLookTimer = (CTimer::GetTimeInMilliseconds() + 4000); // used to prevent ped from looking again
		
		if(m_nPedState == PED_LOOK_HEADING || m_nPedState == PED_LOOK_ENTITY)
			ClearLook();
	}		
}


// Callback to remove fingers after giving the Vees
/*void FinishFuckUCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed *) pData;
	if(pAnim->GetAnimId()==ANIM_STD_PARTIAL_FUCKU && pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_UNARMED)
		pPed->RemoveWeaponModel(MODELID_WEAPON_FINGERS);
}*/

// Name			:	MoveHeadToLook
// Purpose		:	Moves head to look at interesting stuff
// Parameters	:	None
// Returns		:	Nothing
#define FINGER_FIRE_TIME (4.0f/30.0f)
/*
void CPed::MoveHeadToLook()
{
	float fDesiredPitchDegrees = 0.0f;
	CVector vecLookPosition;
	CAnimBlendAssociation *pAnim = NULL;
		
	
	// Check head look timer
	if(m_nLookTimer && CTimer::GetTimeInMilliseconds() > m_nLookTimer)
	{
		ClearLookFlag();
	}
	// want to maintain this flag if bIsRestoringLook is to be used
	if((m_nPedFlags.bIsLooking || m_nPedFlags.bIsRestoringLook) && !CanUseTorsoWhenLooking()) 
	{
		m_ik.SetFlag(PEDIK_TORSO_USED);
	}
	
	// Get desired looking direction
	if(m_pEntLookEntity)
	{
		// check and see if FuckU anim is playing, if so, do we want to give the fingers?
//		if(!m_nPedFlags.bShakeFist && 
//			GetWeapon()->GetWeaponType()==WEAPONTYPE_UNARMED && 
//			(pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_PARTIAL_FUCKU)) )
//		{
//			if(pAnim->GetCurrentTime() > FINGER_FIRE_TIME && pAnim->GetCurrentTime() - pAnim->GetTimeStep() <= FINGER_FIRE_TIME)
//			{
//				// check if animation is directed at a cop or cop car
//				bool bIsCop = false;
//				if(m_pEntLookEntity->GetModelIndex()==MODELID_CAR_POLICE)
//					bIsCop = true;
//				else if(m_pEntLookEntity->GetIsTypePed() && ((CPed *)m_pEntLookEntity)->GetPedType()==PEDTYPE_COP)
//					bIsCop = true;
//					
//				if(IsPlayer() && (m_pPedStats->m_nTemper >= 52 || bIsCop)){
//					AddWeaponModel(MODELID_WEAPON_FINGERS);
//					((CPlayerPed *)this)->AnnoyPlayerPed(true);
//				}
//				else if(!(CGeneral::GetRandomNumber() &3))
//					AddWeaponModel(MODELID_WEAPON_FINGERS);
//			}
//		}
	
		if(m_pEntLookEntity->GetType()== ENTITY_TYPE_PED)
		{
			((CPed*)m_pEntLookEntity)->m_ik.GetComponentPosition(vecLookPosition,PED_TORSO);
		}
		else
		{
			vecLookPosition = m_pEntLookEntity->GetPosition();
		}

		if(m_ik.LookAtPosition(vecLookPosition) == false)
		{
			if(!m_nPedFlags.bIsPersistantLooking)
				ClearLookFlag();
		}
		// if ped is able to look at target, and shake fist flag has been set, might want to play an anim
		else if(m_nPedFlags.bShakeFist && !m_nPedFlags.bIsAimingGun && !m_nPedFlags.bIsRestoringGun && m_nPedState != PED_ANSWER_MOBILE)
		{

			if(m_nLookTimer - CTimer::GetTimeInMilliseconds() < 1000)
			{
				AnimationId nAnimChoice = ANIM_STD_NUM;
				CAnimBlendAssociation *pAnim = NULL;
				bool bValidWeapon =  !GetWeapon()->IsType2Handed() && GetWeapon()->GetWeaponType()!=WEAPONTYPE_ROCKETLAUNCHER;
				// choose which (if any) anim to play
				if(IsPlayer() && bValidWeapon){
					if(m_pEntLookEntity->GetIsTypePed()){
						if(m_pPedStats->m_nTemper < 49 || ((CPed *)m_pEntLookEntity)->GetPedType()==PEDTYPE_COP)
							nAnimChoice = ANIM_STD_PARTIAL_FUCKU;
						else if(m_pPedStats->m_nTemper < 47)
							nAnimChoice = ANIM_STD_PARTIAL_PUNCH;
					}
					else if(m_pPedStats->m_nTemper > 49 || 
							(m_pEntLookEntity->GetIsTypeVehicle() && ((CVehicle*)m_pEntLookEntity)->IsLawEnforcer))
						nAnimChoice = ANIM_STD_PARTIAL_FUCKU;
				}
				else if( bValidWeapon && (CGeneral::GetRandomNumber() &1) )
					nAnimChoice = ANIM_STD_PARTIAL_FUCKU;
				
				if(nAnimChoice != ANIM_STD_NUM)
					pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, nAnimChoice, 4.0);

				if(pAnim){
					pAnim->SetFlag(ABA_FLAG_ISFINISHAUTOREMOVE);
					pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
					// annoy player
					//if(pAnim->GetAnimId()==ANIM_STD_PARTIAL_FUCKU)
					//	((CPlayerPed *)this)->AnnoyPlayerPed(true);
				}
				
				m_nPedFlags.bShakeFist = false;
			}
		}

	}
	else if(m_fLookHeading != ADAMS_THIS_IS_AN_INVALID_LOOK_HEADING)
	{
		if(m_ik.LookInDirection(m_fLookHeading, 0.0f) == false)
		{
			if(!m_nPedFlags.bIsPersistantLooking)
				ClearLookFlag();
		}
	}
	else
		ClearLookFlag();
}
*/

/*
// Name			:	RestoreHeadPosition
// Purpose		:	Look ahead again
// Parameters	:	None
// Returns		:	Nothing

// temp hack to compile
void CPed::RestoreHeadPosition()
{
	if(!CanUseTorsoWhenLooking())
	{
		m_ik.SetFlag(PEDIK_TORSO_USED);
	}

	if(m_ik.RestoreLookAt())
	{
		m_nPedFlags.bIsRestoringLook = false;
		
		if(CanUseTorsoWhenLooking())
			m_ik.ClearFlag(PEDIK_TORSO_USED);
		
		return;
	}
}
*/
// Name			:	SetLookFlag
// Purpose		:	Sets ped look flag, look at position or entity
// Parameters	:	fLookHeading, pLookEntity - position or entity to look at
//					isPersistant - whether to continue looking if peds neck is strained
void CPed::SetAimFlag(float fLookHeading)
{
	m_nPedFlags.bIsAimingGun = true;
	m_nPedFlags.bIsRestoringGun = false;
	m_fLookHeading = fLookHeading;
	m_nLookTimer = 0;
	TIDYREF(m_pEntLookEntity, &m_pEntLookEntity);
	m_pEntLookEntity = NULL;
	//TIDYREF(m_pTargetEntity, &m_pTargetEntity);
	//m_pTargetEntity = NULL;

	if( m_nPedFlags.bIsDucking )
		m_ik.ClearFlag(PEDIK_USE_ARM);
	// If using pistol or uzi move arm
	if(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill())->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
		m_ik.SetFlag(PEDIK_USE_ARM);
	else
		m_ik.ClearFlag(PEDIK_USE_ARM);
}

void CPed::SetAimFlag(CEntity* pLookEntity)
{
	m_nPedFlags.bIsAimingGun = true;
	m_nPedFlags.bIsRestoringGun = false;
	TIDYREF(m_pEntLookEntity, &m_pEntLookEntity);
	m_pEntLookEntity = pLookEntity;
	REGREF(m_pEntLookEntity, &m_pEntLookEntity);
	//TIDYREF(m_pTargetEntity, &m_pTargetEntity);
	//m_pTargetEntity = pLookEntity;
	//REGREF(m_pTargetEntity, &m_pTargetEntity);
//	m_pTargetEntity->SetIsReferenced(true);
	m_nLookTimer = 0;

}

// Name			:	ClearAimFlag
// Purpose		:	Clear aim flag
void CPed::ClearAimFlag()
{
	if(m_nPedFlags.bIsAimingGun)
	{
		m_nPedFlags.bIsAimingGun = false;
		m_nPedFlags.bIsRestoringGun = true;
		m_ik.ClearFlag(PEDIK_USE_ARM);
		m_nLookTimer = 0;
	}
	
	if(GetPlayerData())
		GetPlayerData()->m_fLookPitch = 0.0f;
}

// Name			:	AimGun
// Purpose		:	Moves gun to point at position or entity
// Parameters	:	None
// Returns		:	Nothing
/*
void CPed::AimGun()
{
	if(IsPlayer() && m_nPedFlags.bIsDucking)
		m_ik.ClearFlag(PEDIK_USE_ARM);

	// Get desired looking direction
	if(m_pTargetEntity)
	{
		CVector posn;
		if(m_pTargetEntity->GetIsTypePed())
		{
			((CPed*)m_pTargetEntity)->GetBonePosition(posn, BONETAG_SPINE1);//PED_TORSO
		}	
		else
		{
			posn = m_pTargetEntity->GetPosition();
		}
		
		if(!IsPlayer())		// player says something when first aims
		{
			Say(AE_PED_COME_ON_DURING_FIGHT);
		}
		m_nPedFlags.bCanPointGunAtTarget = m_ik.PointGunAtPosition(posn);
		
		// If ped is looking at something different to what he is targeting
		if(m_pEntLookEntity != m_pTargetEntity)
			SetLookFlag(m_pTargetEntity, true, true);
	}
	else if(IsPlayer())
	{
		m_nPedFlags.bCanPointGunAtTarget = m_ik.PointGunInDirection(m_fLookHeading, ((CPlayerPed *)this)->m_fLookPitch);
	}
	else
	{
		m_nPedFlags.bCanPointGunAtTarget = m_ik.PointGunInDirection(m_fLookHeading, 0.0f);
	}
	

	// now control the animation so that it doesn't fire (but only if not running a fire ranged task)
//    if(GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ATTACK_RANGED))
//    	return;

	CWeaponInfo* pWeaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType());
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_WEAPON_ATTACK);
	float animLoopStart = pWeaponInfo->GetAnimLoopStart();

	if((pAnim == NULL || pAnim->GetBlendDelta() < 0.0f) && pWeaponInfo->GetCrouchFireAnim())
	{
	 	pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, (AnimationId)pWeaponInfo->GetCrouchFireAnim());
	 	animLoopStart = pWeaponInfo->GetAnim2LoopStart();
	}

	if(pAnim)
	{
		// If crossed fire loop start then stop anim
		if(pAnim->GetCurrentTime() > 0.4f*animLoopStart)
		{
		    if(!GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_GUN_CTRL))
		    {
				pAnim->SetCurrentTime(animLoopStart);
				pAnim->ClearFlag(ABA_FLAG_ISPLAYING);
			}
			// if crouching down have to aim with whole body
			if(m_nPedFlags.bIsDucking)
				m_ik.ClearFlag(PEDIK_USE_ARM);
			// If using pistol or uzi move arm
			if(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
				m_ik.SetFlag(PEDIK_USE_ARM);
			else
				m_ik.ClearFlag(PEDIK_USE_ARM);
		}	
	}
	
	
}

// Name			:	RestoreHeadPosition
// Purpose		:	Look ahead again
// Parameters	:	None
// Returns		:	Nothing
void CPed::RestoreGunPosition()
{
	if(m_nPedFlags.bIsLooking)
	{
		m_ik.StopUsingTorso();
		m_nPedFlags.bIsRestoringGun = false;
		return;
	}
	if(m_ik.RestoreGunPosn())
	{
		m_nPedFlags.bIsRestoringGun = false;
		return;
	}

	if(IsPlayer())
		((CPlayerPed *)this)->m_fLookPitch = 0.0f;
}
*/


bool CPed::CanWeRunAndFireWithWeapon(void)
{
	// if we are in one of those anims you can't run and fire
 	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill())->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
// Name			:	ScanForInterestingStuff
// Purpose		:	Checks area surrounding ped for visible interesting stuff
// Parameters	:	None
// Returns		:	Nothing

void CPed::ScanForInterestingStuff()
{
	int32 i = 0;

	if((!IsPedInControl())||(m_Objective != NO_OBJ)||(CharCreatedBy == MISSION_CHAR))
		return;

	CEntity *ppResults[8]; // whatever.. temp list
	CVehicle *pVehicle;
	int16 nNum;
		
	// scan for nice-looking women to letch at
	LookForSexyPeds();	
	LookForSexyCars();
	
	// scan for lookat nodes attached entities - and investigate if required
	if(LookForInterestingNodes())
		return;
	
	if(m_nPedType == PEDTYPE_CRIMINAL)
	{
		// insert ped muggings in here...(old stuff no longer valid)
		
		// carjacking below
		if(m_nCarJackTimer < CTimer::GetTimeInMilliseconds())
		{
			int32 nProbCarJack;

			nProbCarJack=10;

			if(CGeneral::GetRandomNumber() % 100 < nProbCarJack)
			{
				int16 nBest;
				UInt32 nHighestValue;
	
				nBest = -1;
				nHighestValue = 0;
	
				CWorld::FindObjectsInRange(CVector(GetPosition()), 10.0f, true, &nNum, 6, ppResults, 0, 1, 0, 0, 0);	// Just Cars we're interested in
				// cars in range stored in ppResults
				for(i=0; i<nNum; i++) // find best car
				{
					pVehicle = (CVehicle *)ppResults[i];

					// If vehicle is a mission vehicle or moving too fast ignore
					if(pVehicle->VehicleCreatedBy == MISSION_VEHICLE ||
						pVehicle->GetMoveSpeed().Magnitude() > 0.1f ||
						!pVehicle->IsVehicleNormal()) 
						continue;

					// Don't have random peds jack cars in network game
#ifdef GTA_NETWORK
					if (gGameNet.NetWorkStatus != NETSTAT_SINGLEPLAYER) return;
#endif

					if(pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_CAR)
					{
						if(nHighestValue < pVehicle->pHandling->nMonetaryValue)
						{
							nHighestValue = pVehicle->pHandling->nMonetaryValue;
							nBest = i;
						}
					}
				}
	
				if(nHighestValue > CAR_MINIMUM_VALUE_WORTH_STEALING && nBest != -1)
				{
					pVehicle = (CVehicle *)ppResults[nBest];
					if(pVehicle){
						//go to car
						SetObjective(ENTER_CAR_AS_DRIVER, pVehicle);
						m_nCarJackTimer = (CTimer::GetTimeInMilliseconds() + 5000);
						return;
					}
				}
	
				// set timer anyway... don't need to check all the time
				m_nCarJackTimer = (CTimer::GetTimeInMilliseconds() + 5000);
			}
			// not looking for cars to jack, so maybe look for peds to mug instead!
			else if(m_Objective!=MUG_CHAR && !(CGeneral::GetRandomNumber() &7) )
			{
				CPed *pPed = NULL;
				uint16 nTempType = PEDTYPE_LAST_PEDTYPE;
				for(uint16 i=0; i<m_NumClosePeds; i++)
				{
					// only look for peds to mug within a certain range
					if( (m_apClosePeds[i]->GetPosition() - GetPosition()).MagnitudeSqr() > 7.0f*7.0f)
						break;
				
					// only want to mug certain types of ped
					nTempType = m_apClosePeds[i]->GetPedType();
				
					if( (nTempType==PEDTYPE_CIVMALE || nTempType==PEDTYPE_CIVFEMALE || nTempType==PEDTYPE_CRIMINAL 
					|| nTempType==PEDTYPE_BUM || nTempType==PEDTYPE_PROSTITUTE) && m_apClosePeds[i]->CharCreatedBy!=MISSION_CHAR)
					{
						// make sure ped is in a state that they can be mugged (and not already trying to mug us)
						if(m_apClosePeds[i]->IsPedShootable() && m_apClosePeds[i]->m_Objective!=MUG_CHAR){
							pPed = m_apClosePeds[i];
							break;
						}
					}
				}
			
				if(pPed)
					SetObjective(MUG_CHAR, pPed);

				m_nCarJackTimer = (CTimer::GetTimeInMilliseconds() + 5000);			
			}
					
		}

	}
	
	//Scan for people to chat to
	if(PED_WANDER_PATH==m_nPedState)
	{
    	if(CGeneral::GetRandomNumberInRange(0.0f,1.0f) < 0.5f)
    	{
    	    if(CTimer::GetTimeInMilliseconds() > m_nChatTimer)
    	    {
    	        int k;    		
        		for (k=0;k<m_NumClosePeds;k++)	
        		{
        		    CPed* pClosePed=m_apClosePeds[k];
        		    if(pClosePed)
        		    {
        		        if(PED_WANDER_PATH==pClosePed->m_nPedState)
        		        {
            		        CVector vDiff=pClosePed->GetPosition()-GetPosition();
            		        const float fDiff=vDiff.Magnitude();
            		        if(fDiff<CHAT_DISTANCE)
            		        {
            		            if(CanSeeEntity(pClosePed)&&pClosePed->CanSeeEntity(this))
            		            {
            		                if(WillChat(pClosePed))
            		                {
                   						const uint32 nChatTime = 10000+(CGeneral::GetRandomNumber() % 4000);
                                        SetChat(pClosePed,nChatTime);
                                        pClosePed->SetChat(this,nChatTime);
            		                }
            		            }
            		        }
        		        }
        		    }
        		}
    		}
        }
		else
		{
		    m_nChatTimer=CTimer::GetTimeInMilliseconds()+200;
		}
	}
	
  

  

} // end - ScanForInterestingStuff
*/
//	#define IS_GANG_MEMBER(pedtype) ((pedtype == PEDTYPE_GANG1) ? (true) : (pedtype == PEDTYPE_GANG2) ? (true) : (pedtype == PEDTYPE_GANG3) ? (true) : (false))

/*
// Name			:	WillChat
// Purpose		:	will these two peds chat?
//
// Parameters	:	pointers two the 2 peds
// returns true/false

bool CPed::WillChat(CPed *pPed)
{
// seems not to notice roads and pavements as different
//	if(m_LastMaterialToHaveBeenStandingOn == 1)// road
//		return false;

	if((!m_LastWanderNode.IsEmpty())&&(!m_NextWanderNode.IsEmpty())&&(m_LastWanderNode != m_NextWanderNode))
	{
		if(ThePaths.TestCrossesRoad(m_LastWanderNode, m_NextWanderNode)) // we are crossing a road
		{
			return false;
		}
	}	
	
	if(m_LastMaterialToHaveBeenStandingOn == COLPOINT_SURFACETYPE_TARMAC)
	{
		return false;
	}		
	
		
	if(this == pPed) // I don't talk to myself
		return false;
	
	if(m_nPedType == pPed->m_nPedType) // ok, same pedtype can always chat
		return true;
		
	if(m_nPedType == PEDTYPE_CRIMINAL)
		return false;
			
	if(pPed->m_nPedType == PEDTYPE_COP)
		return false;
		
	if(pPed->IsPlayer())
	    return false;
			
	if( (IsGangMember() || pPed->IsGangMember()) && m_nPedType != pPed->m_nPedType) // one is a gang member, dont chat
		return false;

	return true;
}
*/


// Name			:	CalculateNewVelocity
// Purpose		:	Calculates a new position for a ped based on its current position
//					and steering force
// Parameters	:	None
// Returns		:	Nothing
//
#define ROTATE_LEG_PITCH_CORRECTION (0.1f)
//
//TweakFloat SKATEBOARD_HEADING_RATE_MULT = 0.3f;
TweakFloat PED_SLOPE_PITCH_RATE 		= 0.75f;
TweakFloat PED_SLOPE_PITCH_RATE_SLOW	= 0.9f;
TweakFloat PED_SLOPE_RETURN_RATE 		= 0.9f;
TweakFloat PED_BODY_LEAN_ADD_RATE 		= 0.2f;
TweakFloat PED_BODY_LEAN_RETURN_RATE 	= 0.95f;
TweakFloat PED_BODY_LEAN_LIMIT_RATE 	= 0.5f;
TweakFloat PED_HEADING_RATE_ACCEL_MIN 	= 0.1f;
TweakFloat PED_HEADING_RATE_ACCEL_ADD 	= 0.1f;
//
void CPed::CalculateNewVelocity()
{
	if(IsPedInControl())
	{
		float fBestHeading, fHeadingDiff;
		float fMaxHeadingChange; // = DEGTORAD(CPedType::GetPedTypeMaxHeadingChange(m_nPedType) * CTimer::GetTimeStep());
		bool bReverseDir;
	
		bReverseDir = FALSE;
		fBestHeading = m_fDesiredHeading;

		fMaxHeadingChange = DEGTORAD(m_fHeadingChangeRate) * CTimer::GetTimeStep();
/*
		if(m_motionAnimGroup==ANIM_SKATEBOARD_PED && GetMoveState()>=PEDMOVE_WALK)
		{
			fMaxHeadingChange *= SKATEBOARD_HEADING_RATE_MULT;
			if(GetPlayerData())
				fMaxHeadingChange /= 1.0f + 0.5f*GetPlayerData()->m_fSkateBoardSpeed;
		}
*/
		m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
		fBestHeading = CGeneral::LimitRadianAngle(fBestHeading);
		
		if (m_fCurrentHeading + PI < fBestHeading)
		{
			fBestHeading -= 2*PI;
		}
		else if (m_fCurrentHeading - PI > fBestHeading)
		{
			fBestHeading += 2*PI;
		}
/*
		if(IsPlayer() && (m_nPedState == PED_ATTACK))
		{
			fMaxHeadingChange/=4.0f;
		}
*/
		fHeadingDiff = fBestHeading - m_fCurrentHeading;
		
		if(IsPlayer())// && m_motionAnimGroup!=ANIM_SKATEBOARD_PED)
			m_fHeadingChangeRateAccel = 1.0f;
		else if(fHeadingDiff >= 0.0f && m_fHeadingChangeRateAccel < 0.0f)	
			m_fHeadingChangeRateAccel = PED_HEADING_RATE_ACCEL_MIN;
		else if(fHeadingDiff < 0.0f && m_fHeadingChangeRateAccel > 0.0f)
			m_fHeadingChangeRateAccel = -PED_HEADING_RATE_ACCEL_MIN;
		
		bool bTurningAFAP = false;
		if(fHeadingDiff > CMaths::Abs(m_fHeadingChangeRateAccel)*fMaxHeadingChange){
			m_fCurrentHeading += CMaths::Abs(m_fHeadingChangeRateAccel)*fMaxHeadingChange;
			m_fHeadingChangeRateAccel += PED_HEADING_RATE_ACCEL_ADD*CTimer::GetTimeStep();
			if(m_fHeadingChangeRateAccel > 1.0f)	m_fHeadingChangeRateAccel = 1.0f;
			bTurningAFAP = true;
		}
		else if(fHeadingDiff < -1.0f*CMaths::Abs(m_fHeadingChangeRateAccel)*fMaxHeadingChange){
			m_fCurrentHeading -= CMaths::Abs(m_fHeadingChangeRateAccel)*fMaxHeadingChange;
			m_fHeadingChangeRateAccel -= PED_HEADING_RATE_ACCEL_ADD*CTimer::GetTimeStep();
			if(m_fHeadingChangeRateAccel < -1.0f)	m_fHeadingChangeRateAccel = -1.0f;
			bTurningAFAP = true;
		}
		else
		{
			if(!IsPlayer() && CMaths::Abs(fHeadingDiff) > PED_HEADING_RATE_ACCEL_MIN*fMaxHeadingChange)
			{
				m_fCurrentHeading += 0.5f*fHeadingDiff;
				m_fHeadingChangeRateAccel *= 0.5f;
			}
			else
			{
				m_fCurrentHeading += fHeadingDiff;
				m_fHeadingChangeRateAccel = CMaths::Max(PED_HEADING_RATE_ACCEL_MIN, CMaths::Abs(fHeadingDiff) / fMaxHeadingChange);
			}
		}
		
		if(!IsPlayer() && (GetMoveState()==PEDMOVE_STILL || GetMoveState()==PEDMOVE_NONE)
		&& bTurningAFAP && GetPedIntelligence()->GetTaskUseGun()==NULL && GetPedIntelligence()->GetTaskFighting()==NULL)
		//CMaths::Abs(fHeadingDiff) > 2.0f*PED_HEADING_RATE_ACCEL_ADD*fMaxHeadingChange)
		{
			if(fHeadingDiff > 0.0f)
				SetMoveState(PEDMOVE_TURN_L);
			else
				SetMoveState(PEDMOVE_TURN_R);
		}
		else if(GetMoveState()==PEDMOVE_TURN_L || GetMoveState()==PEDMOVE_TURN_R)
		{
			SetMoveState(PEDMOVE_STILL);
		}

/*
		if(m_motionAnimGroup==ANIM_SKATEBOARD_PED)
		{
/*
			if(GetMoveState()==PEDMOVE_RUN && m_ik.IsFlagSet(PEDIK_SLOPE_PITCH))
			{
				float fDiff = -1.0f*fHeadingDiff;
				
				if(fDiff > PED_BODY_LEAN_LIMIT_RATE*fMaxHeadingChange)
					fBodyAngle += PED_BODY_LEAN_ADD_RATE*PED_BODY_LEAN_LIMIT_RATE*fMaxHeadingChange;
				else if(fDiff < -PED_BODY_LEAN_LIMIT_RATE*fMaxHeadingChange)
					fBodyAngle -= PED_BODY_LEAN_ADD_RATE*PED_BODY_LEAN_LIMIT_RATE*fMaxHeadingChange;
				else
					fBodyAngle += PED_BODY_LEAN_ADD_RATE*fDiff;
				
				fBodyAngle *= CMaths::Pow(PED_BODY_LEAN_RETURN_RATE, CTimer::GetTimeStep());
				m_ik.SetBodyRollAngle(fBodyAngle);
			}
			else
				m_ik.SetBodyRollAngle(0.0f);
#ifndef FINAL			
			sprintf(gString, "Skateboard Lean = %f", m_ik.GetBodyRollAngle());
			CDebug::PrintToScreenCoors(gString, 35,11);
#endif
*//*
			m_ik.SetBodyRollAngle(0.0f);
		}
		else
*/
			m_ik.SetBodyRollAngle(0.0f);
	}

	float fGroundNormalFwd = DotProduct(m_vecGroundNormal, GetMatrix().GetForward());
	float fGroundNormalRight = DotProduct(m_vecGroundNormal, GetMatrix().GetRight());
	
	// sometimes want to use ik to pitch body for slope
	if(m_ik.IsFlagSet(PEDIK_SLOPE_PITCH))
	{

//		if((m_eMoveState >= PEDMOVE_WALK || GetPedState()==PED_DIE || m_nPedFlags.bFallenDown) && !g_surfaceInfos.IsStairs(m_LastMaterialToHaveBeenStandingOn))//!=COLPOINT_SURFACETYPE_STAIRS)
//		{

		float fRate = 0.0f;
		// JB : rearranged this to allow IK slope & pitch to active on stairs if ped is dead or has fallen	
		if((GetPedState()==PED_DIE || m_nPedFlags.bFallenDown) || (m_eMoveState >= PEDMOVE_WALK 
		&& !g_surfaceInfos.IsStairs(m_LastMaterialToHaveBeenStandingOn) && !m_nPedFlags.bPedHitWallLastFrame))
		{
			if(GetPedState()==PED_DIE || m_nPedFlags.bFallenDown)// || m_motionAnimGroup==ANIM_SKATEBOARD_PED)
				m_ik.SetSlopeRollAngle(CMaths::ASin(MIN(1.0f, MAX(-1.0f, fGroundNormalRight))));
			else
				m_ik.SetSlopeRollAngle(0.0f);
			
			float fAngle = CMaths::ASin(MIN(1.0f, MAX(-1.0f, fGroundNormalFwd)));
			
			fRate = PED_SLOPE_PITCH_RATE_SLOW;
			if(GetPedState()==PED_DIE || m_nPedFlags.bFallenDown || m_eMoveState > PEDMOVE_WALK)
				fRate = PED_SLOPE_PITCH_RATE;
			
			m_ik.SetSlopeAngle(PED_SLOPE_PITCH_RATE*m_ik.GetSlopeAngle() + (1.0f-PED_SLOPE_PITCH_RATE)*fAngle);
		}
		else
		{
			if(m_ik.GetSlopeAngle()!=0.0 || m_ik.GetSlopeRollAngle()!=0.0f)
				fRate = CMaths::Pow(PED_SLOPE_RETURN_RATE, CTimer::GetTimeStep());
				
			if(CMaths::Abs(m_ik.GetSlopeAngle()) > 0.01f)
				m_ik.SetSlopeAngle(m_ik.GetSlopeAngle()*fRate);
			else
				m_ik.SetSlopeAngle(0.0f);
			
//			if(m_nPedFlags.bFallenDown)
//				m_ik.SetSlopeRollAngle(CMaths::ASin(MIN(1.0f, MAX(-1.0f, fGroundNormalRight))));
//			else
			if(CMaths::Abs(m_ik.GetSlopeRollAngle()) > 0.02f)
				m_ik.SetSlopeRollAngle(m_ik.GetSlopeRollAngle()*fRate);
			else
			m_ik.SetSlopeRollAngle(0.0f);
		}
	}
	else if(GetMoveState() >= PEDMOVE_WALK && !IsPlayer())
	{
		if(CMaths::Abs(m_ik.GetSlopeRollAngle()) > 0.02f)
		{
			float fRate = CMaths::Pow(PED_SLOPE_RETURN_RATE, CTimer::GetTimeStep());
			m_ik.SetSlopeRollAngle(m_ik.GetSlopeRollAngle()*fRate);
		}	
		m_ik.SetSlopeRollAngle(0.0f);
	}

	fGroundNormalFwd = CMaths::Sqrt(CMaths::Max(0.0f, 1.0f - fGroundNormalFwd*fGroundNormalFwd));
	fGroundNormalRight = CMaths::Sqrt(CMaths::Max(0.0f, 1.0f - fGroundNormalRight*fGroundNormalRight));

	m_vecCurrentVelocity = CVector2D(0.0f,0.0f);
	m_vecCurrentVelocity += m_extractedVelocity.y*fGroundNormalFwd*GetMatrix().GetForward();
	m_vecCurrentVelocity += m_extractedVelocity.x*fGroundNormalRight*GetMatrix().GetRight();

//	m_vecCurrentVelocity.x = -CMaths::Sin(m_fCurrentHeading)*m_extractedVelocity.y + CMaths::Cos(m_fCurrentHeading)*m_extractedVelocity.x;
//	m_vecCurrentVelocity.y = CMaths::Cos(m_fCurrentHeading)*m_extractedVelocity.y + CMaths::Sin(m_fCurrentHeading)*m_extractedVelocity.x;
	
	if(CTimer::GetTimeStep() < MIN_TIMESTEP && !CTimer::bSlowMotionActive)
		m_vecCurrentVelocity *= MIN_TIMESTEP;
	else
		m_vecCurrentVelocity /= CTimer::GetTimeStep();

//!PC - looks like this hack won't work any more...
//#ifdef GTA_PC
#if 0
	//hack that shall eventually be removed (MAYBE NOT!!)
	if( (TheCamera.Cams[TheCamera.ActiveCam].GetWeaponFirstPersonOn() || TheCamera.Cams[0].Using3rdPersonMouseCam())
	&& this==FindPlayerPed() && CanStrafeOrMouseControl() && GetPedState()!=PED_FIGHT)
    {
		float HeadingForFirstPerson = WorkOutHeadingForMovingFirstPerson(m_fCurrentHeading);	
		float CurrentVelocityMagnitude = m_vecCurrentVelocity.Magnitude(); 

		float fHeadingAngle = CGeneral::LimitRadianAngle(HeadingForFirstPerson - m_fCurrentHeading);
		if(fHeadingAngle < -HALF_PI)
			fHeadingAngle += PI;
		else if(fHeadingAngle > HALF_PI)
			fHeadingAngle -= PI;
			
		if(fHeadingAngle > DEGTORAD(-50.0f) && fHeadingAngle < DEGTORAD(50.0f))
		{
			TheCamera.Cams[TheCamera.ActiveCam].SetPlayerVelocityFor1stPerson(CurrentVelocityMagnitude);

			m_vecCurrentVelocity.x = -CMaths::Sin(HeadingForFirstPerson)*CurrentVelocityMagnitude;
			m_vecCurrentVelocity.y = CMaths::Cos(HeadingForFirstPerson)*CurrentVelocityMagnitude;
		}
    	
//////////////////////////////////
// TEST
		// need to check if player's standing still
		CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);
		// might be using alternative idle anims (or tired blended over idle)
		CAnimBlendAssociation* pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FIGHT_IDLE);
		if(!pAnim2)	pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE_TIRED);
		if(!pAnim2)	pAnim2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_ATTACK_EXTRA2);

		if((pAnim==NULL || pAnim->GetBlendAmount() < 0.5f) && pAnim2==NULL && !m_nPedFlags.bIsDucking)
		{
    		// try and rotate the ped's legs to follow the motion
			LimbOrientation tempTorsoOrientation;
			tempTorsoOrientation.yaw = CGeneral::LimitRadianAngle(HeadingForFirstPerson - m_fCurrentHeading);
			if(tempTorsoOrientation.yaw > HALF_PI + DEGTORAD(10.0f))
				tempTorsoOrientation.yaw -= PI;
			else if(tempTorsoOrientation.yaw < -HALF_PI - DEGTORAD(10.0f))
				tempTorsoOrientation.yaw += PI;
				
			if(tempTorsoOrientation.yaw > DEGTORAD(-50.0f) && tempTorsoOrientation.yaw < DEGTORAD(50.0f))
			{
				tempTorsoOrientation.pitch = ROTATE_LEG_PITCH_CORRECTION;

				// this bit of code was trying to find a bone between the root of the ped and the lower legs
				// so that is could effectively rotate the hips as one and change the legs orientation that way
				// (can't find a suitable bone to manipulate though unfortunately)
/*
				RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pRwObject);
				int32 bone = RpHAnimIDGetIndex(pHierarchy, nTestBoneTag);//ConvertPedNode2BoneTag(PED_PELVIS));
				AnimBlendFrameData *pAnimFrame = (ANIMBLENDCLUMPFROMCLUMP((RpClump*)m_pRwObject)->GetFrameDataArray() + bone);
				RpHAnimStdKeyFrame *iFrame = pAnimFrame->pStdKeyFrame;

				RtQuatRotate(&iFrame->q, &CPedIK::XaxisIK, RADTODEG(tempTorsoOrientation.yaw), rwCOMBINEPRECONCAT);
*/
				// this bit of code is trying to rotate the two legs individually in the same was as it was done for
				// the original non-skinned pc version

				RpHAnimStdKeyFrame *iFrame = m_aPedFrames[PED_UPPERLEGL]->pStdKeyFrame;
				RtQuatRotate(&iFrame->q, &CPedIK::ZaxisIK, RADTODEG(tempTorsoOrientation.pitch), rwCOMBINEPOSTCONCAT);
				RtQuatRotate(&iFrame->q, &CPedIK::XaxisIK, RADTODEG(tempTorsoOrientation.yaw), rwCOMBINEPOSTCONCAT);

				iFrame = m_aPedFrames[PED_UPPERLEGR]->pStdKeyFrame;
				RtQuatRotate(&iFrame->q, &CPedIK::ZaxisIK, RADTODEG(tempTorsoOrientation.pitch), rwCOMBINEPOSTCONCAT);
				RtQuatRotate(&iFrame->q, &CPedIK::XaxisIK, RADTODEG(tempTorsoOrientation.yaw), rwCOMBINEPOSTCONCAT);

				m_nPedFlags.bUpdateMatricesRequired = true;
			}
		}
////////////////////////////////////
    }
    else if(this==FindPlayerPed())
    {
    	FindPlayerPed()->m_fFPSMoveHeading = 0.0f;
    }
#endif    
} // end - CPed::CalculateNewVelocity


float CPed::WorkOutHeadingForMovingFirstPerson(float CurrentHeading) 
{
	bool WalkingForwards=false;
	bool WalkingBackWards=false;
	
	// only do 1st person stuff for players (make sure we've got an instance of CPlayerPed)
	if(!this->IsPlayer() || !GetPlayerData())
		return 0.0f;
		
	CPlayerPed *pPlayerPed = (CPlayerPed *)this;

	float fStickX, fStickY;
	fStickX = CPad::GetPad(0)->GetPedWalkLeftRight();
	fStickY = CPad::GetPad(0)->GetPedWalkUpDown();

	if(fStickY == 0.0f)
	{
		if(fStickX > 0.0f)
			GetPlayerData()->m_fFPSMoveHeading = -HALF_PI;
		else if(fStickX < 0.0f)
			GetPlayerData()->m_fFPSMoveHeading = HALF_PI;
		// else no control movement so leave FPSheading as it was
	}	
	else
		GetPlayerData()->m_fFPSMoveHeading = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -fStickX, fStickY);

	// add the heading change onto the true ped heading to get direction of movement
	CurrentHeading = CGeneral::LimitRadianAngle(CurrentHeading + GetPlayerData()->m_fFPSMoveHeading);

	return CurrentHeading;
}


// Name			:	UpdatePosition
// Purpose		:	Updates a peds position
// Parameters	:	None
// Returns		:	Nothing
TweakFloat PED_MAXFALLINGSPEEDCHANGE = 0.01f;
//
void CPed::UpdatePosition()
{
	CVector2D vecChange;

	if (CReplay::ReplayGoingOn()) return;

	// If ped is in the air ignore
	if(!GetIsStanding())
	{
		// still need to do rotation if swiming
		if(GetPedIntelligence()->GetTaskSwim() 
		|| GetPedIntelligence()->GetTaskJetPack()
		|| (GetPedIntelligence()->GetTaskPrimary() && GetPedIntelligence()->GetTaskPrimary()->GetTaskType()==CTaskTypes::TASK_COMPLEX_USE_SWAT_ROPE))
		{
			SetHeading(m_fCurrentHeading);
		}
		
		return;
	}
	
	// if the player is attached to a vehicle their position will be updated elsewhere
	if(m_pAttachToEntity)
		return;

	SetHeading(m_fCurrentHeading);

	// if the ped is standing on moving ground then
	// take into account the movement of the ground
	if (m_pGroundPhysical != NULL)
	{
		if(m_nPedFlags.bFallenDown && m_pGroundPhysical->GetIsTypeVehicle()
		&& !m_pGroundPhysical->m_nPhysicalFlags.bInfiniteMass
		&& ((CVehicle *)m_pGroundPhysical)->GetBaseVehicleType()!=VEHICLE_TYPE_BOAT)
		{
			vecChange.x = 0.0f;
			vecChange.y = 0.0f;
		}
	
		CVector vecGroundVelocity;
		// don't want to include friction components for boats (and do want to add vertical speed to ped)
		if(!IsPlayer() && m_pGroundPhysical->GetIsTypeVehicle() && ((CVehicle *)m_pGroundPhysical)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
		{
			CVector vecBoatOffset = GetPosition();
			vecBoatOffset.z -= PED_GROUNDOFFSET;
			vecBoatOffset -= m_pGroundPhysical->GetPosition();
			vecGroundVelocity = CrossProduct(m_pGroundPhysical->m_vecTurnSpeed, vecBoatOffset) + m_pGroundPhysical->m_vecMoveSpeed;
			
			vecGroundVelocity += -1.0f * m_pGroundPhysical->m_vecTurnSpeed.MagnitudeSqr() * vecBoatOffset * CTimer::GetTimeStep();
			
			m_vecMoveSpeed.z = vecGroundVelocity.z;
		}
		else
		{
			vecGroundVelocity = m_pGroundPhysical->GetSpeed(m_vecGroundOffset);
//			vecGroundVelocity += -1.0f * m_pGroundPhysical->m_vecTurnSpeed.MagnitudeSqr() * m_vecGroundOffset * CTimer::GetTimeStep();
		}
		
		vecChange.x = m_vecCurrentVelocity.x + vecGroundVelocity.x - m_vecMoveSpeed.x;
		vecChange.y = m_vecCurrentVelocity.y + vecGroundVelocity.y - m_vecMoveSpeed.y;

		m_fCurrentHeading += m_pGroundPhysical->m_vecTurnSpeed.z * CTimer::GetTimeStep();
		m_fDesiredHeading += m_pGroundPhysical->m_vecTurnSpeed.z * CTimer::GetTimeStep();
	}
	else if(g_surfaceInfos.IsSteepSlope(m_LastMaterialToHaveBeenStandingOn) &&
			(m_vecGroundNormal.x != 0.0f || m_vecGroundNormal.y != 0.0f) )
	{
		CVector2D vecPush(m_vecGroundNormal.x, m_vecGroundNormal.y);
		vecPush.Normalise();

		m_vecMoveSpeed.x = 0.0f;
		m_vecMoveSpeed.y = 0.0f;
		m_vecMoveSpeed.z = -0.001f;

		vecChange.x = 0.02f*vecPush.x + m_vecCurrentVelocity.x;
		vecChange.y = 0.02f*vecPush.y + m_vecCurrentVelocity.y;
		
		float fStopMove = DotProduct(vecPush, vecChange);
		
		if(fStopMove < 0.0f)
		{
			vecChange.x -= fStopMove*vecPush.x;
			vecChange.y -= fStopMove*vecPush.y;
		}
	}
	else
	{
		vecChange.x = m_vecCurrentVelocity.x - m_vecMoveSpeed.x;
		vecChange.y = m_vecCurrentVelocity.y - m_vecMoveSpeed.y;
	}

	// limit the force the ped applies in the change direction
	// this causes inertia and restricts grip when standing on moving ground
	// if the ped is in the air the change force is limited alot more

	//
	// AF: Only limit change in direction if standing on something or in the air
	//
	float fChangeMagnitude;
	float fMaxChangeMagnitude;
	
	if(m_pGroundPhysical && m_pGroundPhysical->m_pAttachToEntity==NULL
	&& !(m_pGroundPhysical->m_nPhysicalFlags.bInfiniteMass && !m_pGroundPhysical->m_nPhysicalFlags.bInfiniteMassFixed))
	{
		fChangeMagnitude = vecChange.Magnitude();
		fMaxChangeMagnitude = fChangeMagnitude;
		
		if(m_pGroundPhysical->GetIsTypeVehicle())
		{
			if(m_nPedState==PED_DIE)
				fMaxChangeMagnitude = PED_MAXFLOATINGSPEEDCHANGE * CTimer::GetTimeStep();
			else if(((CVehicle *)m_pGroundPhysical)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE
			&& m_pGroundPhysical->GetMoveSpeed().MagnitudeSqr() > 0.2f*0.2f)
				fMaxChangeMagnitude = 0.1f*PED_MAXFLOATINGSPEEDCHANGE * CTimer::GetTimeStep();
			else if(((CVehicle *)m_pGroundPhysical)->GetBaseVehicleType()==VEHICLE_TYPE_CAR)
				fMaxChangeMagnitude = PED_MAXWALKINGSPEEDCHANGE * CTimer::GetTimeStep();
		}
		else
			fMaxChangeMagnitude = PED_MAXWALKINGSPEEDCHANGE * CTimer::GetTimeStep();
			
		if (fChangeMagnitude > fMaxChangeMagnitude)
		{
			vecChange *= fMaxChangeMagnitude / fChangeMagnitude;
		}
	}
	else if(m_nPedFlags.bFallenDown && m_pGroundPhysical==NULL)
	{
		fChangeMagnitude = vecChange.Magnitude();
		fMaxChangeMagnitude = PED_MAXFALLINGSPEEDCHANGE*CTimer::GetTimeStep();
		if (fChangeMagnitude > fMaxChangeMagnitude)
		{
			vecChange *= fMaxChangeMagnitude / fChangeMagnitude;
		}
	}

	// apply speed change to movement speed
	m_vecMoveSpeed.x += vecChange.x;
	m_vecMoveSpeed.y += vecChange.y;
	
} // end - CPed::UpdatePosition




// Name			:	CalculateNewOrientation
// Purpose		:	Calculates the z orientation for a ped based on its current velocity
// Parameters	:	None
// Returns		:	Nothing

#define MAX_DIM_TREAT_BUILD_AS_OBJ (3.0f)

void CPed::CalculateNewOrientation()
{
	if (CReplay::ReplayGoingOn()) return;

	if(!IsPedInControl())
		return;
		
	SetOrientation(0.0f, 0.0f, m_fCurrentHeading);
} // end - CPed::CalculateNewOrientation


// Name			:	ClearAll
// Purpose		:	Clears all ped states
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearAll()
{
	if( IsPedInControl() || m_nPedState == PED_DEAD)
	{
		SetPedState(PED_NONE);
		m_eMoveState = PEDMOVE_NONE;
		//TIDYREF(m_pTargetEntity, &m_pTargetEntity);
		//m_pTargetEntity = NULL;
		//m_vecTargetPosition = CVector(0.0f, 0.0f, 0.0f);

		//m_vecFleePosition = CVector2D(0.0f, 0.0f);
		//m_pFleeEntity = NULL;
		//m_nFleeTimer = 0;
		//m_pFleeEntityWhenStanding=NULL;
		
//		SetUsesCollision(true);
//		ClearPointGunAt();
		
		//m_nPedFlags.bIsContinuingAim = FALSE;
		m_nPedFlags.bRenderPedInCar = true;
		m_nPedFlags.bKnockedUpIntoAir = false;
		m_nPedFlags.bKnockedOffBike = false;
		m_pNOCollisionVehicle = NULL;
	}
	
} // end - CPed::ClearAll



// Name			:	ProcessBuoyancy
// Purpose		:	Work out forces for ped in water
// Parameters	:	None
// Returns		:	Nothing
void CPed::ProcessBuoyancy()
{
	float fBuoyancyConstant = 1.1f;
	CVector CentreOfBuoyancy;
	CVector BuoyancyForce;

	// used for starting raindrops after splash:
	static
	uint32 nGenerateRaindrops = 0;
	static
	uint32 nGenerateWaterCircles = 0;

	if(m_nPedFlags.bInVehicle)
		return;
	
//	static bool TEST_NON_PLAYER_SKIP_BUOYANCY = true;
//	if(TEST_NON_PLAYER_SKIP_BUOYANCY && !GetPlayerData())
//		return;

	if(m_nPedState==PED_DEAD || m_nPedState==PED_DIE)
		fBuoyancyConstant = 1.8f;
	if (mod_Buoyancy.ProcessBuoyancy(this, m_fMass*PHYSICAL_GRAVITYFORCE*fBuoyancyConstant, &CentreOfBuoyancy, &BuoyancyForce))
	{

		if(GetIsStanding() && m_pEntityStandingOn && m_pEntityStandingOn->GetIsTypeVehicle()
		&& ((CVehicle *)m_pEntityStandingOn)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT && !((CVehicle*)m_pEntityStandingOn)->m_nPhysicalFlags.bRenderScorched)
		{
			m_nPhysicalFlags.bIsInWater = FALSE;
			if(GetPedIntelligence()->GetTaskSwim())
			{
				((CTaskSimpleSwim *)GetPedIntelligence()->GetTaskActiveSimplest())->AddToStopTimer(CTimer::GetTimeStep());
			}
			return;	// if on a boat -> not in water so return
		}
		else if(GetPlayerData())
		{
			// First - although player is in the water, want to check he's not standing on, or about to fall on a Boat
			CVector vecStart = GetPosition();
			float fEndZ = vecStart.z - 3.0f;
			CColPoint colPt;
			CEntity* pEntity;

			//INC_PEDAI_LOS_COUNTER;  this is a dead cheap line test because we're only testing vehicles
			if(CWorld::ProcessVerticalLine(vecStart, fEndZ, colPt, pEntity, false, true, false, false, false))
			{
				if(pEntity->GetIsTypeVehicle() && ((CVehicle *)pEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT
				&& !((CVehicle*)pEntity)->m_nPhysicalFlags.bRenderScorched && pEntity->GetMatrix().GetUp().z > 0.0f)
				{
					m_nPhysicalFlags.bIsInWater = FALSE;
					return;	// if on a boat -> not in water so return
				}
			}
		}

		const float COLOR_SCALE = 0.5f;//0.60f;
		uint8 Red	= (int32)(COLOR_SCALE * 255.0f * (CTimeCycle::GetAmbientRed() + CTimeCycle::GetDirectionalRed() * 0.5f));
		uint8 Green	= (int32)(COLOR_SCALE * 255.0f * (CTimeCycle::GetAmbientGreen() + CTimeCycle::GetDirectionalGreen() * 0.5f));
		uint8 Blue	= (int32)(COLOR_SCALE * 255.0f * (CTimeCycle::GetAmbientBlue() + CTimeCycle::GetDirectionalBlue() * 0.5f));
		uint8 Alpha = CGeneral::GetRandomNumberInRange(48, 96);
		const RwRGBA dummyColor = {Red, Green, Blue, Alpha};

		// MN - FX SYSTEM -------------------------------
		if (m_nPhysicalFlags.bForceFullWaterCheck==false && m_vecMoveSpeed.z<-0.01f)
		{
			// splash from ped falling in water	
			float waterLevel;
			CVector splashPos = this->GetPosition() + m_vecMoveSpeed*CTimer::GetTimeStep()*4.0f;
			if (CWaterLevel::GetWaterLevel(splashPos.x, splashPos.y, splashPos.z, &waterLevel, true))
			{
				splashPos.z = waterLevel;
				g_fx.TriggerWaterSplash(splashPos);
#ifdef USE_AUDIOENGINE
				AudioEngine.ReportWaterSplash(this, AE_SILENCE, TRUE);
#endif //USE_AUDIOENGINE
			}
		}
		// ----------------------------------------------

		m_nPhysicalFlags.bIsInWater = TRUE;

		m_nPhysicalFlags.bForceFullWaterCheck = true;
		
		ApplyMoveForce(BuoyancyForce);
		
//		if(m_nPedState == PED_DIE || m_nPedState == PED_DEAD)
//			return;
/*
		if(m_nPedFlags.bTryingToReachDryLand && !(m_nPedState == PED_DIE || m_nPedState == PED_DEAD))
		{
			if(BuoyancyForce.z/m_fMass > 0.4f*PHYSICAL_GRAVITYFORCE*CTimer::GetTimeStep())
			{
				m_nPedFlags.bTryingToReachDryLand = false;
				CVector vecHitWaterPos = GetPosition();
				if(PlacePedOnDryLand()){
					// inflict some damage as a penalty for falling in the water
					if(m_nHealth > 20.0f)
//						InflictDamage(NULL, WEAPONTYPE_DROWNING, 15.0f, PED_SPHERE_CHEST,0);
						GetPedIntelligence()->AddEvent(new CEventDamage(this, NULL, WEAPONTYPE_DROWNING, 15.0f, PED_SPHERE_CHEST, 0));
						

					// if the ped was in the air, don't want them to fall when they land (might fall in the water again)
					if(m_nPedFlags.bIsInTheAir){
						RpAnimBlendClumpSetBlendDeltas((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL, -1000.0f);
						m_nPedFlags.bIsInTheAir = 0;
					}
/*

					m_vecMoveSpeed = CVector(0.0f,0.0f,0.0f);
					SetPedState(PED_IDLE);			
					return;
				}
			}
		}
*/		
		float fPow = 0.0f;
		if(BuoyancyForce.z/m_fMass > PHYSICAL_GRAVITYFORCE*CTimer::GetTimeStep() || mod_Buoyancy.GetWaterLevel() > GetPosition().z + 0.6f)
		{
			bool bIsSwimming = false;

			if(IsPlayer())
			{
				if(GetPedIntelligence()->GetTaskSwim())
				{
					((CTaskSimpleSwim *)GetPedIntelligence()->GetTaskActiveSimplest())->SetStopTimer(0.0f);
					// don't want quite so much resistance, so we can swim a bit better
					bIsSwimming = true;
				}
				else if(GetPedIntelligence()->GetTaskActiveSimplest() && GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_CLIMB)
				{
					bIsSwimming = true;
				}
				else
				{
					CEventInWater event(BuoyancyForce.z/(m_fMass*PHYSICAL_GRAVITYFORCE*CTimer::GetTimeStep()));
					GetPedIntelligence()->AddEvent(event);
				}
				
				SetIsStanding(false);
				m_nPedFlags.bIsDrowning = true;
			}
			else
			{
				SetIsStanding(false);
				m_nPedFlags.bIsDrowning = true;
				
				/*
//				InflictDamage(NULL, WEAPONTYPE_DROWNING, 3.0f*CTimer::GetTimeStep(), PED_SPHERE_CHEST,0);
				CPedDamageResponseCalculator damageResponseCalculator(NULL, 3.0f*CTimer::GetTimeStep(), WEAPONTYPE_DROWNING, PED_SPHERE_CHEST, false);
				CEventDamage event(NULL, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_DROWNING, PED_SPHERE_CHEST, 0 , false);
				if(event.AffectsPed(this))
					damageResponseCalculator.ComputeDamageResponse(this,event.GetDamageResponseData());
				else
					event.GetDamageResponseData().SetDamageCalculated();
				*/
				CEventInWater event;	
				GetPedIntelligence()->AddEvent(event);
			}

			if(!bIsSwimming)
			{
				fPow = CMaths::Pow(0.9f, CTimer::GetTimeStep());
				m_vecMoveSpeed.x *= fPow;
				m_vecMoveSpeed.y *= fPow;
				if(m_vecMoveSpeed.z < 0.0f)	m_vecMoveSpeed.z *= fPow;
			}
		}
		// if not floating then increase timer to stop swimming task
		else if(GetIsStanding() && GetPedIntelligence()->GetTaskSwim())
		{
			((CTaskSimpleSwim *)GetPedIntelligence()->GetTaskActiveSimplest())->AddToStopTimer(CTimer::GetTimeStep());
		}
		else if(GetPlayerData())
		{
			CVector vecMouthPos(0.0f,0.0f,0.1f);
			GetTransformedBonePosition(vecMouthPos, BONETAG_HEAD);
			if(vecMouthPos.z < mod_Buoyancy.GetWaterLevel())
				((CPlayerPed *)this)->HandlePlayerBreath(true, 1.0f);
		}

/*		
#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (ParticleObjects AddParticle)	
// add water splash particle object
// add rain drops particle object
// water circles 



		if(BuoyancyForce.z/m_fMass > 0.25f*PHYSICAL_GRAVITYFORCE*CTimer::GetTimeStep())
		{
			if(fPow==0.0f)
				fPow = CMaths::Pow(0.9f, CTimer::GetTimeStep());
				
			m_vecMoveSpeed.x *= fPow;
			m_vecMoveSpeed.y *= fPow;
		
			//
			// ped water splash:
			//
			if(this->m_vecMoveSpeed.z < -0.1f)
			{
				m_vecMoveSpeed.z = -0.01f;
			
				DMAudio.PlayOneShot(AudioHandle, AE_PED_SPLASH, 0.0f);


				// position of our ped in ~1.5 sec:
				CVector vecSplashPos( this->GetPosition() + this->m_vecMoveSpeed*2.2f );
			
				float zet = 0.0f;
				if(CWaterLevel::GetWaterLevel(vecSplashPos.x, vecSplashPos.y, vecSplashPos.z, &zet))
					vecSplashPos.z = zet;// + 0.5f;
			
			
				CVector vecSplashVel(0.0f, 0.0f, 0.10f);//this->m_vecMoveSpeed*0.1f);// 0.0f, 0.0f, 0.1f);
//				vecSplashVel.z = 0.18f;//0.1f;
			
				CParticleObject::AddObject(POBJECT_PED_WATER_SPLASH, vecSplashPos, vecSplashVel, 0.0f,
											200, dummyColor, TRUE);

				nGenerateRaindrops		= CTimer::GetTimeInMilliseconds() + 80;//300;
				nGenerateWaterCircles	= CTimer::GetTimeInMilliseconds() + 100;//60;
			
			
			
			
//				#if 0
//				//
//				// old splash code:
//				//		
//				CVector vecPuffPos;
//				CVector vecPuffVel;
//				const float fAngStep = PI/16.0f;
//			
//				for(float fAng = 0.0f; fAng < 2.0f*PI; fAng += fAngStep)
//				{
//					vecPuffPos.x = 0.2f * CMaths::Cos(fAng);
//					vecPuffPos.y = 0.2f * CMaths::Sin(fAng);
//					vecPuffPos.z = 0.0f;
//		
//					vecPuffVel.x = 0.1f * vecPuffPos.x;
//					vecPuffVel.y = 0.1f * vecPuffPos.y;
//					vecPuffVel.z = 0.03f;
//		
//					vecPuffPos = vecPuffPos + this->GetPosition();
//					CParticle::AddParticle(PARTICLE_BOAT_SPLASH, vecPuffPos, vecPuffVel, NULL, 1.5f);
//				}
//				#endif//#if 0...
			}
			else if(m_vecMoveSpeed.z < -0.04f)
			{
				m_vecMoveSpeed.z = -0.02f;
			}
			
		}	// end if buoyancyforce > 25% gravityforce

		// Generate particles
		if(nGenerateWaterCircles)
		{
			if(CTimer::GetTimeInMilliseconds() >= nGenerateWaterCircles)
			{
				CVector vecSplashPos2( this->GetPosition() );
				
				float zet = 0.0f;
				if(CWaterLevel::GetWaterLevel(vecSplashPos2.x, vecSplashPos2.y, vecSplashPos2.z, &zet))
					vecSplashPos2.z = zet;
						
				if(vecSplashPos2.z != 0.0f)
				{
					nGenerateWaterCircles = 0;
		//			vecSplashPos2.z += 1.0f;
					for(int32 i=0; i<4; i++)
					{
						const float RANGE = 0.75;//2.5f;
						CVector pos(vecSplashPos2);
						pos.x += CGeneral::GetRandomNumberInRange(-RANGE, RANGE);
						pos.y += CGeneral::GetRandomNumberInRange(-RANGE, RANGE);
						CParticle::AddParticle(PARTICLE_RAIN_SPLASH_BIGGROW, pos, CVector(0,0,0), NULL, 0.0f, dummyColor);
					}
				}
			}//if(nGenerateWaterCircles)...
		}


		if(nGenerateRaindrops)
		{
			if(CTimer::GetTimeInMilliseconds() >= nGenerateRaindrops)
			{
				const RwRGBA dummyColor2 = {0, 0, 0, 0};
				CVector vecSplashPos2( this->GetPosition() );
				
				float zet = 0.0f;
				if(CWaterLevel::GetWaterLevel(vecSplashPos2.x, vecSplashPos2.y, vecSplashPos2.z, &zet))
					vecSplashPos2.z = zet;
						
				if(vecSplashPos2.z >= 0.0f)
				{
					//printf(" ** Splash zet = %.2f\n", vecSplashPos2.z);
					nGenerateRaindrops = 0;
					vecSplashPos2.z += 0.25f;//0.5f;
					// rain splashes around (radius = 3.5m):
					CParticleObject::AddObject(POBJECT_SPLASHES_AROUND, vecSplashPos2, CVector(0,0,0),
									4.5f, 1500, dummyColor2, TRUE);
				}
			}//if(nGenerateRaindrops)...
		}
		
#endif // MN - OLD FX SYSTEM (ParticleObjects)	
*/
	}
	else
	{
		m_nPhysicalFlags.bForceFullWaterCheck = false;
		
		// if ped is not in the water at all, but is trying to swim, force the end of the swim task fast!
		if(GetPedIntelligence()->GetTaskSwim())
		{
			((CTaskSimpleSwim *)GetPedIntelligence()->GetTaskActiveSimplest())->SetStopTimer(1000.0f);
		}

	}



}// CPed::ProcessBuoyancy()...


/*
void CPed::FixCollisionProblems()
{
	if(GetPedIntelligence()->GetTaskActive()
	&& GetPedIntelligence()->GetTaskActive()->GetTaskType()!=CTaskTypes::TASK_SIMPLE_CLIMB)
		return;

	// If anyPed has been hit by something or playerPed is stuck
	if(GetUsesCollision() && m_nPedState!=PED_DIE 
	&& ((GetDamageEntity() && GetDamageImpulseMagnitude()>0.0f)
	|| (IsPlayer() && GetIsStuck())))
	{
		float fPush;
		bool bDealtWith = FALSE;
		
		fPush = 1.0f;
		CEntity* pDamageEntity = GetDamageEntity();
		CVector vecPush;
        
		// Push the ped away from the object it collided with a little
		if(!m_nPedFlags.bIsInTheAir && m_nPedState!=PED_JUMP && GetDamageImpulseMagnitude()>0.0f)
		{
			vecPush = m_vecDamageNormal;
			vecPush.z = 0.0f;
			if(GetIsStanding())// || !IsPlayer())
				vecPush *= 4.0f;
			else	// if not IsStanding -> no control forces to resist, so smaller push
				vecPush *= 0.5f;
				
			ApplyMoveForce(vecPush);
		}
		
		if((m_nPedFlags.bIsInTheAir	&& m_nPedState!=PED_DIE && m_nPedState!=PED_DEAD)
		|| (!GetIsStanding() && !GetWasStanding() && m_nPedState==PED_FALL) )
		{
			CColPoint cCol1, cCol2;
			float fZ1 = 0.0f, fZ2 = 0.0f;
			CVector vecCheckDelta;
			CEntity *pEntity = NULL;
		
			if(m_nAntiSpazTimer > 1000 || m_nAntiSpazTimer==0){
				m_nAntiSpazTimer = 0;
				m_vecWeaponPrevPos = GetPosition();
				vecPush = CVector(0.0f,0.0f,0.0f);
			}
			else{
				vecPush = GetPosition() - m_vecWeaponPrevPos;
			}
			m_nAntiSpazTimer++;

			if( (m_nAntiSpazTimer > FRAMES_PER_SECOND/(4*MAX(MIN_TIMESTEP,CTimer::GetTimeStep())) && vecPush.MagnitudeSqr() < 0.01f*m_nAntiSpazTimer)
			||  (m_nNoOfCollisionRecords>1 && m_nAntiSpazTimer > FRAMES_PER_SECOND/(2*MAX(MIN_TIMESTEP,CTimer::GetTimeStep())) && CMaths::Abs(vecPush.z) < 0.004f*m_nAntiSpazTimer) )
			{
				if( ((3 + this->RandomSeed + CTimer::m_FrameCounter) & 7) == 0)
				{
					fZ1 = 1.0f;
					vecCheckDelta = CVector(GetMatrix().tx, GetMatrix().ty, GetMatrix().tz + fZ1);
					fZ2 = GetMatrix().tz - PED_GROUNDOFFSET;
					
					INC_PEDAI_LOS_COUNTER;
					
					if(CWorld::ProcessVerticalLine(vecCheckDelta, fZ2, cCol1, pEntity, true, true, false, true, false))
					{
						if(!m_nPedFlags.bHeadStuckInCollision || (cCol1.GetPosition().z + PED_GROUNDOFFSET) < GetMatrix().tz)
						{
							GetMatrix().tz = cCol1.GetPosition().z + PED_GROUNDOFFSET;
							UpdateRwMatrix();
							if(m_nPedFlags.bHeadStuckInCollision)	m_nPedFlags.bHeadStuckInCollision = false;
						}
//						SetLanding();
//						m_nPedFlags.bIsInTheAir = false;
						SetIsStanding(true);
					}
				}
				else if(IsPlayer() && m_nPedState != PED_JUMP && CPad::GetPad(0)->JumpJustDown())
				{
					fZ1 = CPad::GetPad(0)->GetPedWalkLeftRight();
					fZ2 = CPad::GetPad(0)->GetPedWalkUpDown();
					if(CMaths::Abs(fZ1)>0.0f || CMaths::Abs(fZ2)>0.0f){
						m_fDesiredHeading = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -fZ1, fZ2);
						m_fDesiredHeading = m_fDesiredHeading - TheCamera.Orientation;;
						m_fCurrentHeading = m_fDesiredHeading = CGeneral::LimitRadianAngle(m_fDesiredHeading);
						SetOrientation(0.0f, 0.0f, m_fCurrentHeading);
					}
					// let player get the ped out of this jam by jumping away
					SetJump();
					m_nAntiSpazTimer = 0;
					m_vecWeaponPrevPos = GetPosition();
					vecPush = CVector(0.0f,0.0f,0.0f);
				}
				else if(IsPlayer())// && m_nPedState!=PED_JUMP)
				{
					// ok player is stuck in the air, but give direction control to give
					// the impression that player still has control and encourage jump
					fZ1 = CPad::GetPad(0)->GetPedWalkLeftRight();
					fZ2 = CPad::GetPad(0)->GetPedWalkUpDown();
					if(CMaths::Abs(fZ1)>0.0f || CMaths::Abs(fZ2)>0.0f){
						m_fDesiredHeading = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -fZ1, fZ2);
						m_fDesiredHeading = m_fDesiredHeading - TheCamera.Orientation;;
						m_fCurrentHeading = m_fDesiredHeading = CGeneral::LimitRadianAngle(m_fDesiredHeading);
						SetOrientation(0.0f, 0.0f, m_fCurrentHeading);
					}
					// also remove glide anim by blending in idle so player doesn't look stoopid
					CAnimBlendAssociation *pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_JUMP_GLIDE);
					if(!pAnim)	pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL_GLIDE);
					if(pAnim){
						pAnim->SetBlendDelta(-3.0f);
						pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
					}
					// if we're stuck again make sure to clear the jump state so we can then jump again
					if(m_nPedState==PED_JUMP)
						SetPedState(PED_IDLE);
				}
				else
				{
					// for non-player peds just remove glide anim so they don't look completely stupid
					CAnimBlendAssociation *pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_JUMP_GLIDE);
					if(!pAnim)	pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL_GLIDE);
					if(pAnim){
						pAnim->SetBlendDelta(-3.0f);
						pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
					}
				}
			}
			else if( m_nNoOfCollisionRecords==1 && m_aCollisionRecordPtrs[0]!=NULL && m_aCollisionRecordPtrs[0]->GetIsTypeBuilding() &&
			m_nAntiSpazTimer > FRAMES_PER_SECOND/(2*MAX(MIN_TIMESTEP,CTimer::GetTimeStep())) && CMaths::Abs(vecPush.z) < 0.004f*m_nAntiSpazTimer )
			{
				vecCheckDelta.x = -vecPush.y;
				vecCheckDelta.y =  vecPush.x;
				vecCheckDelta.z = 1.0f;
				vecCheckDelta.Normalise();
				
				INC_PEDAI_LOS_COUNTER;
				
				if(CWorld::ProcessVerticalLine(GetPosition()+vecCheckDelta, -20.0f, cCol1, pEntity, true, false, false, false, false))
					fZ1 = cCol1.GetPosition().z;
				else
					fZ1 = 500.0f;
					
				INC_PEDAI_LOS_COUNTER;
				
				if(CWorld::ProcessVerticalLine(GetPosition()-vecCheckDelta, -20.0f, cCol2, pEntity, true, false, false, false, false))
					fZ2 = cCol2.GetPosition().z;
				else
					fZ2 = 501.0f;

				int16 nPlaceOnGround = 0;
				if((fZ1 > GetPosition().z - PED_GROUNDOFFSET && fZ2 < 500.0f)
				|| (fZ1 > GetPosition().z - PED_GROUNDOFFSET && fZ2 > GetPosition().z - PED_GROUNDOFFSET))
					nPlaceOnGround = 1;
				else if(fZ2 > GetPosition().z - PED_GROUNDOFFSET && fZ1<499.0f)
					nPlaceOnGround = 2;

				if(nPlaceOnGround > 0 && !m_nPedFlags.bHeadStuckInCollision)
				{
					if(nPlaceOnGround==2)	GetMatrix().SetTranslateOnly(cCol2.GetPosition());
					else					GetMatrix().SetTranslateOnly(cCol1.GetPosition());
					GetMatrix().tz += PED_GROUNDOFFSET;
					UpdateRwMatrix();
//					SetLanding();
					SetIsStanding(true);
				}

				if(fZ1 < fZ2)
					vecCheckDelta = -1*vecCheckDelta;
					
				vecCheckDelta.z = 1.0f;
				ApplyMoveForce(4.0f*vecCheckDelta);
				SetPosition(GetPosition() + 0.25f*vecCheckDelta);
				m_fCurrentHeading = CGeneral::GetRadianAngleBetweenPoints(vecCheckDelta.x, vecCheckDelta.y, 0.0f,0.0f);
				m_fDesiredHeading = m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
				SetOrientation(0.0f, 0.0f, m_fCurrentHeading);

				if(m_nPedState!=PED_FALL && !m_nPedFlags.bIsPedDieAnimPlaying)
					SetFall(1000, ANIM_STD_HIGHIMPACT_BACK, true);
					
				m_nPedFlags.bIsInTheAir = false;
			}
			else if(m_vecDamageNormal.z > 0.4f)
			{
				// ok nice reuse of variables here - if we're jumping and in the air won't be using waitstates!
				if(GetPedState()==PED_JUMP)
				{
					if(m_nWaitTimer > 2000)
						m_nWaitTimer = 0;
					else if( m_nWaitTimer < 1000)
						m_nWaitTimer += CTimer::GetTimeElapsedInMilliseconds();
				}
				
				vecPush = m_vecDamageNormal;
				vecPush.z = 0.0f;
				vecPush.Normalise();
				// check timer so we don't end up stuck and sliding up railings
				if(GetPedState()==PED_JUMP && m_nWaitTimer < 300)
					ApplyMoveForce(-4.0f*vecPush);
				else
					ApplyMoveForce(2.0f*vecPush);
			}
		}
		else if(m_nAntiSpazTimer < 1001)
			m_nAntiSpazTimer = 0;
			
	}// end if is in collision
	else
	{
		m_nPedFlags.bHitSomethingLastFrame = false;
		if(m_nAntiSpazTimer < 1001){
			if(m_nAntiSpazTimer > 500 || !m_nPedFlags.bIsInTheAir)
				m_nAntiSpazTimer = 0;
			else if(m_nAntiSpazTimer > 0)
				m_nAntiSpazTimer -= 1;
		}

	}
}
*/



//
// Name			:	ProcessControl
// Purpose		:	Work out forces for ped standing on ground
// Parameters	:	None
// Returns		:	Nothing
//
void CPed::ProcessControl()
{
	// This does pretty much nothing for non-player peds right now 
	m_PedAudioEntity.Service();
	
	// control the molotov effect
	if (IsPlayer())
	{	
		CWeapon* pWeapon = GetWeapon();
		if (pWeapon->GetWeaponType()==WEAPONTYPE_MOLOTOV)
		{
			if ((m_nPedFlags.bDontRender) || 
				(!m_nFlags.bIsVisible) ||
				(m_nPhysicalFlags.bIsInWater && GetPedIntelligence()->GetTaskSwim()) ||
				(m_pWeaponClump==NULL))
			{
				// kill the molotov effect
				if (pWeapon->m_pWeaponFxSys)
				{
					g_fxMan.DestroyFxSystem(pWeapon->m_pWeaponFxSys);
					pWeapon->m_pWeaponFxSys = NULL;
				}
			}
			else
			{
				if (pWeapon->m_pWeaponFxSys==NULL)
				{
					RpHAnimHierarchy* pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)GetRwObject());
					int32 boneId = RpHAnimIDGetIndex(pHierarchy, BONETAG_R_HAND);
					RwMatrix* pPedHandMat = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + boneId);
					
					CVector offsetPos(0.0f, 0.0f, 0.0f);
				
					pWeapon->m_pWeaponFxSys = g_fxMan.CreateFxSystem("molotov_flame", (RwV3d*)&offsetPos, pPedHandMat, false);
					
					if (pWeapon->m_pWeaponFxSys)
					{
						pWeapon->m_pWeaponFxSys->SetLocalParticles(true);
						pWeapon->m_pWeaponFxSys->CopyParentMatrix();
						pWeapon->m_pWeaponFxSys->Play();
					}			
				}	
			}			
		}	
	}

#ifndef FINAL
	if (this != FindPlayerPed())
	{
		if (CPedDebugVisualiserMenu::IsStopProcessingPedsExceptFocus())
		{	
			if (CPedDebugVisualiserMenu::GetFocus() != this)
			 	return;
		}
		else if (CPedDebugVisualiserMenu::IsStopProcessingPeds())
			return;
	}
#endif // FINAL

	if (ShouldWeSkipProcessControl()) return;

#ifdef GTA_REPLAY
	if (m_nPedFlags.bUsedForReplay) return;	// Replay peds don't need to process control. (leads to tasks using anims that have been removed by replay code)
#endif

#ifdef DEBUG_PED_STATE	
	{
    	int i;
    	const int N=m_objectives.size();
    	eObjective* pObjs=&(m_objectives[0]);
    	for(i=0;i<N;i++)
    	{
    	    const eObjective obj=m_objectives[i];
    	    int ppp=0;
    	}
    }
    
    {
    	int i;
    	const int N=m_states.size();
    	ePedState* pStates=&(m_states[0]);
    	for(i=0;i<N;i++)
    	{
    	    const ePedState state=m_states[i];
    	    int ppp=0;
    	}
    }
    
    {
        int i;
    	const int N=m_oldstates.size();
    	ePedState* pOldStates=&(m_oldstates[0]);
    	for(i=0;i<N;i++)
    	{
    	    const ePedState oldstate=m_oldstates[i];
    	    int ppp=0;
    	}
    }
    
    {
        int i;
    	const int N=m_storedstates.size();
    	ePedState* pStoredStates=&(m_storedstates[0]);
    	for(i=0;i<N;i++)
    	{
    	    const ePedState storedstate=m_storedstates[i];
    	    int ppp=0;
    	}
    }
    
    
#endif

#ifdef DEBUG
//	Last updated 24.11.00
	//if (m_pThreatEntity)
	//{
	//	AssertEntityPointerValid(m_pThreatEntity);
	//}
	
	//if (m_pTargetEntity)
	//{
	//	AssertEntityPointerValid(m_pTargetEntity);
	//}
	
	//if (m_pFleeEntity)
	//{
	//	AssertEntityPointerValid(m_pFleeEntity);
	//}
	
	if (m_pEntLockOnTarget)
	{
		AssertEntityPointerValid(m_pEntLockOnTarget);
	}
	
	if (m_pEntLookEntity)
	{
		AssertEntityPointerValid_NotInWorld(m_pEntLookEntity);
	}
	
	if (m_pGroundPhysical)
	{
		AssertEntityPointerValid(m_pGroundPhysical);
	}
	
	//if (pLeader)
	//{
	//	AssertEntityPointerValid(pLeader);
	//}
	
	//if (m_pObjectivePed)
	//{
	//	AssertEntityPointerValid(m_pObjectivePed);
	//}
	
	//if (m_pObjectiveVehicle)
	//{
	//	AssertEntityPointerValid(m_pObjectiveVehicle);
	//}
	
	if (m_nPedFlags.bInVehicle && m_pMyVehicle)
	{
		AssertEntityPointerValid(m_pMyVehicle);
	}
	
//	End last updated 24.11.00
#endif

		// Peds need to be frozen if they're part of a gang war and the player is in an interior.
	if (m_nPedFlags.bPartOfAttackWave && (CGame::currArea != AREA_MAIN_MAP || FindPlayerCoors().z > 950.0f))
	{
		return;
	}


	// Tidy up out-of-date references
	if (((CTimer::m_FrameCounter + RandomSeed) & 31) == 0)
	{
		PruneReferences();
	}
	
	// Fade in/out ped
	int32 alpha = CVisibilityPlugins::GetClumpAlpha((RpClump*)GetRwObject());
	if(m_nPedFlags.bFadeOut)
	{
		alpha -= 8;
		if(alpha < 0)	alpha = 0;
	}
	else if(alpha < 255)
	{
		alpha += 16;
		if(alpha > 255)	alpha = 255;
	}
	CVisibilityPlugins::SetClumpAlpha((RpClump*)GetRwObject(), alpha);

	// bKnockedOffBike turns off all collision between ped and the bike they just fell off 
	if(m_nPedFlags.bKnockedOffBike)
	{
		// want to clear it once it's safe to do so
		if((GetIsStanding() || m_nPedFlags.bIsDrowning) && !m_pGroundPhysical && !m_nPedFlags.bHeadStuckInCollision 
		&& m_fHitHeadHeight==PED_HIT_HEAD_CLEAR && m_vecMoveSpeed.MagnitudeSqr() < 0.01f)
		{
			if(m_pMyVehicle)
			{
				if(CCollision::ProcessColModels(GetMatrix(), CModelInfo::GetColModel(GetModelIndex()), m_pMyVehicle->GetMatrix(), CModelInfo::GetColModel(m_pMyVehicle->GetModelIndex()), aTempPedColPts)==0)
				{
					m_nPedFlags.bKnockedOffBike = false;
					m_pNOCollisionVehicle = NULL;
				}
			}
			else
				m_nPedFlags.bKnockedOffBike = false;
		}
	}
		
	if(m_nPedFlags.bHeadStuckInCollision && m_nPedFlags.bCheckColAboveHead && m_fHitHeadHeight > GetPosition().z + 1.5f)
		m_nPedFlags.bHeadStuckInCollision = false;
	
	// reset a load of flags before this ProcessControl loop
	m_nPedFlags.bFiringWeapon = false;
    m_nPedFlags.bDonePositionOutOfCollision = false;
	SetWasStanding(false);
	m_nPhysicalFlags.bIsInWater = false;
	m_nPedFlags.bIsDrowning = false;
	m_nPedFlags.bPushOtherPeds = false;
	m_nPedFlags.bCheckColAboveHead = false;
	m_nPedFlags.bPedHitWallLastFrame = false;
	m_nPedFlags.bTestForShotInVehicle = false;

	m_fHitHeadHeight = PED_HIT_HEAD_CLEAR;
	
	if(m_nGunFlashBlendAmount > 0)
	{
		if(m_nGunFlashBlendAmount > m_nGunFlashBlendOutRate*CTimer::GetTimeElapsedInMilliseconds())
			m_nGunFlashBlendAmount -= m_nGunFlashBlendOutRate*CTimer::GetTimeElapsedInMilliseconds();
		else
			m_nGunFlashBlendAmount = 0;
	}
	if(m_nGunFlashBlendAmount2 > 0)
	{
		if(m_nGunFlashBlendAmount2 > m_nGunFlashBlendOutRate2*CTimer::GetTimeElapsedInMilliseconds())
			m_nGunFlashBlendAmount2 -= m_nGunFlashBlendOutRate2*CTimer::GetTimeElapsedInMilliseconds();
		else
			m_nGunFlashBlendAmount2 = 0;
	}
	
	if(!GetIsStanding())
		m_nPedFlags.bHeadStuckInCollision = false;
	
	//Set this flag to help mission peds in player's group.
	if(GetCharCreatedBy()==MISSION_CHAR && 
	   FindPlayerPed()!=this && 
	   CPedGroups::ms_groups[FindPlayerPed()->GetPlayerData()->m_PlayerGroup].GetGroupMembership()->IsMember(this))
	{
		m_nPedFlags.bCheckColAboveHead = false;
	}
	
	ProcessBuoyancy();
	
	if (GetPlayerData())
	{
		if (m_nPhysicalFlags.bIsInWater && GetPlayerData()->m_waterCoverPerc>50)
		{ 
			if (GetPlayerData()->m_wetness<100)
			{
				GetPlayerData()->m_wetness++;
			}
		}
		else
		{
			if (GetPlayerData()->m_wetness>0)
			{
				GetPlayerData()->m_wetness--;
			}
		}
	}
	
	// If standing on something and it hasn't been processed then pospone process of this object
	if(GetIsStanding() && !CWorld::bForceProcessControl && m_pGroundPhysical)
	{
		// ground physical hasn't been processed yet so postpone
		if(m_pGroundPhysical->GetIsInSafePosition()
		|| (m_pGroundPhysical->GetIsTypeVehicle() && ((CVehicle*)m_pGroundPhysical)->GetVehicleType()==VEHICLE_TYPE_TRAIN))
		{
			SetWasPostponed(true);
			return;
		}
	}

/*
	if(m_nPedState == PED_ARRESTED)
	{
		ServiceTalking();
		return;
	}
*/	

	// AI stuff that needs to be done before CPhysical::ProcessControl()
	GetPedIntelligence()->ProcessFirst();
		
	//FixCollisionProblems();

	//if(m_nPedFlags.bIsDucking)			
	//	Duck();
	
	if(!GetIsStanding() && m_vecMoveSpeed.z > 0.25)
	{
		if(GetPlayerData())
			m_vecMoveSpeed.z = 0.25f;
		else
			m_vecMoveSpeed = m_vecMoveSpeed * CMaths::Pow(0.95f, CTimer::GetTimeStep());
	}

	// Process physical object (can skip under certain circumstances -
	// ie when ped is static and collision is getting skipped)
	if(!IsPlayer() && GetIsStanding() && m_vecMoveSpeed.x==0.0f && m_vecMoveSpeed.y==0.0f && m_vecMoveSpeed.z==0.0f
	&& (GetMoveState()==PEDMOVE_STILL || GetMoveState()==PEDMOVE_NONE) && m_extractedVelocity.x==0.0f && m_extractedVelocity.y==0.0f
	&& GetPedState()!=PED_JUMP && !m_nPedFlags.bIsInTheAir && m_pGroundPhysical==NULL)
	{
		// Do some stuff that would otherwise be done in CPhysical::ProcessControl
		CPhysical::SkipPhysics();
	}
	else
		CPhysical::ProcessControl();

	//FIX_AI
//	if(!Dying or Dead)
	{
		RequestDelayedWeapon();
	
		// tried moving this before calculating vel and stuff so can alter speeds for skaters
		PlayFootSteps();
	}

	// this flag should be set by tasks to say they want to do the check
	m_nPedFlags.bTestForBlockedPositions = false;
#ifndef FINAL
	if(GetPlayerData() && CPlayerPed::bDebugPlayerInfo)
		m_nPedFlags.bTestForBlockedPositions = true;
#endif
///////////////////////
// make secondary tasks
//	if(m_nPedFlags.bIsAimingGun)
//		AimGun();
//	else if(m_nPedFlags.bIsRestoringGun)
//		RestoreGunPosition();

	// reset this flag every frame - it will get set by tasks if valid
	m_nPedFlags.bFallenDown = false;

	// All of ped AI processing done here now!!
	GetPedIntelligence()->Process();

	// moved this after PedIntelligence->Process() to minimise lag
	// between Desired and Current heading (especially for player)
	if(m_nPedState!=PED_DEAD)
	{
		CalculateNewVelocity();
// This is done in UpdatePosition() anyway!
//		CalculateNewOrientation();
	}
	UpdatePosition();

	SetMoveAnim();	

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	ServiceTalking();
#endif //USE_AUDIOENGINE

	// clear these flags 
	m_nPedFlags.bRightArmBlocked = false;
	m_nPedFlags.bLeftArmBlocked = false;
	m_nPedFlags.bDuckRightArmBlocked = false;
	m_nPedFlags.bMidriffBlockedForJump = false;
	

////////////////////////////
// Misc services and tidy up
////////////////////////////
	
	if ((m_nPedFlags.bPedIsBleeding || BleedingFrames) && 
		CLocalisation::Blood() &&
		!m_nPedFlags.bInVehicle) //CGame::nastyGame)
	{
		if (BleedingFrames) 
			BleedingFrames--;
			// Generate some blood around the ped
		if ((CTimer::m_FrameCounter & 3) == 0 && (GetPosition() - TheCamera.GetPosition()).MagnitudeSqr() < 50.0f * 50.0f)
		{
			CVector vecTemp;
			float	fSize = 0.15f + (CGeneral::GetRandomNumber() & 127) * 0.0015f;
			vecTemp.x = GetMatrix().tx + ((CGeneral::GetRandomNumber() & 127) - 64) * 0.007f;
			vecTemp.y = GetMatrix().ty + ((CGeneral::GetRandomNumber() & 127) - 64) * 0.007f;
			vecTemp.z = GetMatrix().tz + 1.0f;
			CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpBloodPoolTex, &vecTemp, fSize, 0.0f,
									0.0f, -fSize, 255, FX_BLOOD_RED, 0, 0, PED_SHADOW_DEPTH, 2000 + (CGeneral::GetRandomNumber() & 4095) );		
		}
	}

	if(m_nPedFlags.bInVehicle)// the car we are in has been deleted and not destroyed us properly for some reason
	{
		if(m_pMyVehicle == NULL)
		{
			 // just tidy up
			m_nPedFlags.bInVehicle = false;
			this->UpdateStatLeavingVehicle();
//			CPopulation::UpdatePedCount(this, ADD_TO_POPULATION);
		}
		else
		{
			CPopulation::UpdatePedCount(this, SUBTRACT_FROM_POPULATION);
		}
	}
	else
	{
		CPopulation::UpdatePedCount(this, ADD_TO_POPULATION);
	}
	
	/*
	if((m_iDelayedPhrase>=0)&&(CTimer::GetTimeInMilliseconds()>m_iDelayedPhraseTimer))
	{
	    Say(m_iDelayedPhrase);  
   	    m_iDelayedPhrase=-1;
	    m_iDelayedPhraseTimer=0;
	}
	
	if (CCheat::IsCheatActive(CCheat::FANNYMAGNET_CHEAT) && m_pPedStats->m_nSexiness > 40 && m_nPedType == PEDTYPE_CIVFEMALE)
	{
		//if (!pLeader)	// Only if we don't already have a leader
		//{
		//	SetLeader(FindPlayerPed());
		//}
	}
	*/
	
	// Do we have anything to say?
	if ( ( (CTimer::m_FrameCounter + RandomSeed) & 16383) == 0)
	{
		if (m_nPedFlags.bDruggedUp)
		{
			Say(CONTEXT_GLOBAL_DRUGGED_CHAT);
		}
	}

#ifdef USE_AUDIOENGINE
	if((GetWeapon()->GetWeaponType() == WEAPONTYPE_CHAINSAW) && (GetPedState() != PED_ATTACK)
		&& !m_nPedFlags.bInVehicle && !GetPedIntelligence()->GetTaskSwim())
	{
		m_PedWeaponAudioEntity.AddAudioEvent(AUDIO_EVENT_WEAPON_CHAINSAW_IDLE);
	}

	m_PedWeaponAudioEntity.Service();
#endif //USE_AUDIOENGINE
}

/*
bool CPed::RecomputeFollowPathOnCollision(const CEntity* pHitEntity, const CVector& vNormal)
{
    //Only path find around buildings when the ped is following a static path.
    if(m_pEntityToAvoidOnPathFollow)
    {
        return false;  
    }
    
    const CVector& n=vNormal;
    const float d=-DotProduct(n,GetPosition());
    
    CNodeAddress Nodes[CLOSEST_NODE_SEARCH_SIZE];
    int i;
    for(i=0;i<CLOSEST_NODE_SEARCH_SIZE;i++)
    {
        Nodes[i].SetEmpty();
    }
    
    const int iNoOfNodes=ThePaths.RecordNodesInCircle(GetPosition(),20,PFGRAPH_PEDS,CLOSEST_NODE_SEARCH_SIZE,Nodes);   
    float fClosestDistanceSquared=FLT_MAX;
    CNodeAddress BestNode;
    for(i=0;i<iNoOfNodes;i++)
    {
        CNodeAddress PathNode=Nodes[i];
        if ((!PathNode.IsEmpty()) && ThePaths.IsRegionLoaded(PathNode))
        {
            CVector vNodePos=ThePaths.FindNodePointer(PathNode)->GetCoors();
            const float f=DotProduct(n,vNodePos)+d;
            
            //Test that the node is on the outside of the contact plane.  This ensures
            //that the ped walks away from the contact object.
            if(f>0.25f)
            {
                const float fDistanceSquared=(GetPosition()-vNodePos).MagnitudeSqr();
                
                //Test that the current node is closer than the best node found so far.
                if(fDistanceSquared<fClosestDistanceSquared)
                {
                     CVector w=vNodePos-GetPosition();
                     const float fLength=w.Magnitude();
                     w.Normalise();
                     w*=MAX(4.0f,fLength);
                    CVector vNewTargetPos=GetPosition()+w;
                    if(CWorld::GetIsLineOfSightClear(vNewTargetPos,GetPosition(),true,false,false,false,false,false,false))
                    {
                        fClosestDistanceSquared=fDistanceSquared;
                        BestNode=PathNode;        
                    }
                }
            }
        }   
    }
    
    if(!BestNode.IsEmpty())
    {
        //Head towards this node.
        ClearFollowPath();
        m_aPathNodeList[0]=BestNode;
        m_nNumPathNodes=1;
        return true;
    }
    return false;
}
*/

/*
void CPed::RecomputeFollowPathOnCollision(const CEntity* pHitEntity, const CVector& vNormal)
{
    //Only path find around buildings when the ped is following a static path.
    if(m_pEntityToAvoidOnPathFollow)
    {
        return;  
    }
    
    //Find nodes nearby the player that the ped might be able to get to.
    CPathNode* closeNodes[CLOSEST_NODE_SEARCH_SIZE];
    int i;
    for(i=0;i<CLOSEST_NODE_SEARCH_SIZE;i++)
    {
        closeNodes[i]=0;
    }
    ThePaths.RecordNodesClosestToCoors
        (GetPosition(), PFGRAPH_PEDS, CLOSEST_NODE_SEARCH_SIZE, &closeNodes[0]);
        
   
   //Construct the contact plane (ped can only go to nodes on the 
   //outside of the plane)
    const CVector& n=vNormal;
    const float d=-DotProduct(n,GetPosition());
        
    //Find a nearby node which is on the outside of the contact plane 
    //and the ped can walk a little way toward the node without hitting anything. 
    CPathNode* pClosestNode=0;
    float fClosestDistanceSquared=FLT_MAX;
    for(i=0;i<CLOSEST_NODE_SEARCH_SIZE;i++)
    {
        CPathNode* pPathNode=closeNodes[i];
        if(pPathNode)
        {
            CVector vNodePos=pPathNode->GetCoors();
            const float f=DotProduct(n,vNodePos)+d;
            
            //Test that the node is on the outside of the contact plane.  This ensures
            //that the ped walks away from the contact object.
            if(f>0.25f)
            {
                const float fDistanceSquared=(GetPosition()-vNodePos).MagnitudeSqr();
                
                //Test that the current node is closer than the best node found so far.
                if(fDistanceSquared<fClosestDistanceSquared)
                {
                    CVector w=vNodePos-GetPosition();
                    w.Normalise();
                    w*=4.0f;
                    CVector vNewTargetPos=GetPosition()+w;
                    
                    //Test that the ped can walk a little way towards the node without 
                    //getting stuck.
                    if(CWorld::IsWanderPathClear(GetPosition(),vNewTargetPos,0.5f,4))
                    {
                        fClosestDistanceSquared=fDistanceSquared;
                        pClosestNode=pPathNode;
                    }
                }        
            }
        }
    }
    
    //If we have found a suitable node then just go to it and forget the 
    //path that the ped is currently following.   
    if(pClosestNode)
    {
        //Head towards this node.
        ClearFollowPath();
        m_pPathNodeList[0]=pClosestNode;
        m_nNumPathNodes=1;
    }
}
*/


/*
void CPed::MovePathNodeToWalkAroundObject(const CEntity* pDamageEntity, const CVector& n)
{
#ifdef PATH_FIND_AROUND_BUILDINGS_ON_FOLLOW_PATH

    ASSERT((pDamageEntity->GetType()==ENTITY_TYPE_BUILDING)||(pDamageEntity->GetType()==ENTITY_TYPE_OBJECT));
    
    CVector w=m_vecTargetPosition-GetPosition();
    w.Normalise();   
    CVector t=w-n*DotProduct(w,n);
    t.Normalise();
    t*=2.0f;
    const CVector& vPedPos=GetPosition();
    CVector vNewTargetPos=vPedPos+t;
    m_vPathNodeOffset+=vNewTargetPos-m_vecTargetPosition;

#endif
}
*/

#pragma mark --- optimize for size off ---
#pragma optimize_for_size off

// Name			:	ProcessEntityCollision
// Purpose		:	Find collision points and leg heights
// Parameters	:	Entity pointer and colpoint array
// Returns		:	Number of colpoints and fills in the colpoint array
TweakFloat FALL_HURT_SPEED_H = 0.33f;
TweakFloat FALL_HURT_SPEED_V = -0.25f;
TweakFloat FALL_HURT_MULT_H  = 100.0f;
TweakFloat FALL_HURT_MULT_V  = 400.0f;
TweakFloat FALL_HURT_SPEED_DIE = -0.6f;
//
#define PED_LINE_IN_COL

#ifdef PED_LINE_IN_COL
//
int32 CPed::ProcessEntityCollision(CEntity* pEntity, class CColPoint* aColPoints)
{
	float aLegLineRatios[2] = {1.0f, 1.0f};
	CColPoint aLegColPoints[2];
	CVector vecHitHead_LegNormal(0.0f,0.0f,1.0f);
	bool bProcessingBoat = false;
	bool bDoLegLine = false;
	float fGroundColLineOffset = PED_GROUNDCOLLINE_OFFSET_TIMESTEP * CTimer::GetTimeStep();
	float fTopOfCol = 0.94f;
	CColModel *pPedCol = &CModelInfo::GetColModel(m_nModelIndex);
	CCollisionData* pColData = pPedCol->GetCollisionData();
	
	ASSERTMSG( &(CModelInfo::GetColModel(m_nModelIndex)) == &CTempColModels::ms_colModelPed1, "Wrong Ped ColModel: bugger");
	ASSERTMSG( pColData->m_nNoOfLines == 0, "Ped CoModel has lines??: oops");

	// setup how many of the peds col spheres we want to test
	float fOriginalHeading = -1001.0f;
	if(m_nPedFlags.bTestForBlockedPositions && GetUsesCollision())// && !pEntity->GetIsTypePed())
	{
		if(IsPlayer() && TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_AIMWEAPON
		&& GetPedIntelligence()->GetTaskUseGun() && GetPedIntelligence()->GetTaskUseGun()->GetWeaponInfo()
		&& GetPedIntelligence()->GetTaskUseGun()->GetWeaponInfo()->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
		{
			fOriginalHeading = GetHeading();
			SetHeading(CMaths::ATan2(-TheCamera.Cams[TheCamera.ActiveCam].Front.x, TheCamera.Cams[TheCamera.ActiveCam].Front.y));
		}
		
		pPedCol = &CTempColModels::ms_colModelPed2;
		pColData = pPedCol->GetCollisionData();
	}

	if(!GetUsesCollision() && !m_nPhysicalFlags.bForceHitReturnFalse)
		return 0;

	if(pEntity->GetIsTypeVehicle() && ((CVehicle *)pEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
		bProcessingBoat = true;

	if(!m_nPhysicalFlags.bHalfSpeedCollision && !m_nPhysicalFlags.bSkipLineCol && !m_nPhysicalFlags.bForceHitReturnFalse
	&& m_pAttachToEntity==NULL && !pEntity->GetIsTypePed())
	{
		// If not come across a ProcessEntityCollision function yet and ped is standing on the ground then 
		// clear flag but set was standing flag
		if (!GetCollisionProcessed())
		{
// MOVED THIS STUFF INTO CPhysical::CheckCollision to make sure it gets called
// otherwise we end up with peds jumping out of planes and walking around in mid-air
//			// reset ground physical pointer
//			m_pGroundPhysical = NULL;
//			if(GetIsStanding())
//			{
//				SetIsStanding(false);
//				SetWasStanding(true);
//			}
				
			SetCollisionProcessed(true);
		}
		
		bDoLegLine = true;
		pColData->m_pLineArray[0].m_vecStart.z = 0.0f;
		pColData->m_pLineArray[0].m_vecEnd.z = -PED_GROUNDOFFSET;

		if(GetWasStanding())
		{
			pColData->m_pLineArray[0].m_vecEnd.z += fGroundColLineOffset; //PED_GROUNDCOLLINE_OFFSET;
//			pColData->m_pLineArray[0].m_vecStart.z += PED_GROUNDCOLLINE_OFFSET; //fGroundColLineOffset;
		}

		fTopOfCol = pColData->m_pSphereArray[2].m_vecCentre.z + pColData->m_pSphereArray[2].m_fRadius;
		if(m_nPedFlags.bCheckColAboveHead)
		{
			float fDistGroundToCol = PED_GROUNDOFFSET + (pColData->m_pSphereArray[0].m_vecCentre.z - pColData->m_pSphereArray[0].m_fRadius);
		
			pColData->m_pLineArray[1].m_vecStart.z = fTopOfCol;
			pColData->m_pLineArray[1].m_vecStart.z -= fDistGroundToCol;
			pColData->m_pLineArray[1].m_vecEnd.z = fTopOfCol;
			pColData->m_pLineArray[1].m_vecEnd.z += fDistGroundToCol;
		}
	}
		
	if(bDoLegLine)
	{
		if(m_nPedFlags.bCheckColAboveHead)
		{
			pColData->m_nNoOfLines = 2;
			pPedCol->SetBoundRadius(pColData->m_pLineArray[1].m_vecEnd.z);
			pPedCol->SetBoundBoxMaxZ(pColData->m_pLineArray[1].m_vecEnd.z);
			pPedCol->SetBoundBoxMinZ(pColData->m_pLineArray[0].m_vecEnd.z);
		}
		else
		{
			pColData->m_nNoOfLines = 1;
			pPedCol->SetBoundRadius(CMaths::Abs(pColData->m_pLineArray[0].m_vecEnd.z));
			pPedCol->SetBoundBoxMaxZ(STD_PED_BOUND_BOX_MAX);
			pPedCol->SetBoundBoxMinZ(pColData->m_pLineArray[0].m_vecEnd.z);
		}
	}
	else
	{
		pColData->m_nNoOfLines = 0;
		pPedCol->SetBoundRadius(STD_PED_BOUND_RADIUS);
		pPedCol->SetBoundBoxMaxZ(STD_PED_BOUND_BOX_MAX);
		pPedCol->SetBoundBoxMinZ(STD_PED_BOUND_BOX_MIN);
	}
	
	bool bGetAllCollisions = false;
	if(GetIsStuck() && (pEntity->GetIsTypeBuilding() || ((CPhysical *)pEntity)->m_nPhysicalFlags.bInfiniteMass))
		bGetAllCollisions = true;
	
	int32 nNoOfCollisions = CCollision::ProcessColModels(GetMatrix(), *pPedCol, pEntity->GetMatrix(), pEntity->GetColModel(), aColPoints, aLegColPoints, aLegLineRatios, bGetAllCollisions) ;
	float fOldZPos = GetMatrix().tz;

	if(bDoLegLine)
	{
		pColData->m_nNoOfLines = 0;
		pPedCol->SetBoundRadius(STD_PED_BOUND_RADIUS);
		pPedCol->SetBoundBoxMinZ(STD_PED_BOUND_BOX_MIN);
		pPedCol->SetBoundBoxMaxZ(STD_PED_BOUND_BOX_MAX);

		if(aLegLineRatios[0] < 1.0f)
		{
			// If hasn't hit anything yet that was a ped or have hit something below the object you have hit
			// then set the ped z coord
			if(!GetIsStanding() || GetMatrix().tz < aLegColPoints[0].GetPosition().z + PED_GROUNDOFFSET || (bProcessingBoat && GetMatrix().tz < aLegColPoints[0].GetPosition().z + 3.0f*PED_GROUNDOFFSET))
			{
				bool bDownwardCollision = false;
				for(int c=0; c<nNoOfCollisions; c++)
				if(aColPoints[c].GetNormal().z < -0.867f)
					bDownwardCollision = true;
			
				if(IsPlayer())
				{
					static float PLAYER_LIGHTING_CHANGE_RATE = 0.1f;
					float fChangeRate = PLAYER_LIGHTING_CHANGE_RATE*CTimer::GetTimeStep();
					m_lightingFromCollision	= fChangeRate*CColTriangle::CalculateLighting(aLegColPoints[0].GetLightingB()) + (1.0f - fChangeRate)*m_lightingFromCollision;
				}
				else
					m_lightingFromCollision = CColTriangle::CalculateLighting(aLegColPoints[0].GetLightingB());
							
				// If hit a vehicle or object then store the object in the ped structure
				if(!GetIsStanding() && (pEntity->GetIsTypeVehicle() || pEntity->GetIsTypeObject()))
				{
					m_pGroundPhysical = (CPhysical*) pEntity;
					REGREF(pEntity, (CEntity **)&m_pGroundPhysical);
					m_vecGroundOffset = aLegColPoints[0].GetPosition() - pEntity->GetPosition();
					m_pEntityStandingOn = pEntity;
					REGREF(pEntity, (CEntity **)&m_pEntityStandingOn);

					// if we're standing on a boat, want rescued from the water if we fall in (done in playerped now)
					if(pEntity->GetIsTypeVehicle() && ((CVehicle *)pEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
						m_nPedFlags.bOnBoat = true;
					else
						m_nPedFlags.bOnBoat = false;
					
					// If we landed on a vehicle get the lighting from the vehicle	
					if(pEntity->GetIsTypeVehicle())
						m_lightingFromCollision = m_pGroundPhysical->m_lightingFromCollision;
				}
				else
				{
					m_pEntityStandingOn = pEntity;
					REGREF(pEntity, (CEntity **)&m_pEntityStandingOn);
					// if standing on the ground then must've reached safe ground from the boat
					m_nPedFlags.bTryingToReachDryLand = false;
					m_nPedFlags.bOnBoat = false;
				}

				// set body position
				if(m_nPedFlags.bCheckColAboveHead && aLegLineRatios[1] < 1.0f)
				{
					//if(aLegColPoints[1].GetPosition().z < m_fHitHeadHeight)
					if(aLegColPoints[1].GetPosition().z < m_fHitHeadHeight && (!pEntity->GetIsPhysical() || ((CPhysical *)pEntity)->m_nPhysicalFlags.bInfiniteMassFixed))
						m_fHitHeadHeight = aLegColPoints[1].GetPosition().z;
				}
				
				if((m_nPedFlags.bHeadStuckInCollision==false || (aLegColPoints[0].GetPosition().z + PED_GROUNDOFFSET) < GetMatrix().tz
				|| (pEntity->GetIsTypeBuilding() && DotProduct(aLegColPoints[0].GetNormal(), GetMoveSpeed()) > 0.0f))
				&& !bDownwardCollision)
				{
					if(m_fHitHeadHeight < PED_HIT_HEAD_CLEAR)
					{
						float fOldZ = GetPosition().z;
						if(aLegColPoints[0].GetPosition().z + PED_GROUNDOFFSET + fTopOfCol > m_fHitHeadHeight)
						{
							vecHitHead_LegNormal = aLegColPoints[0].GetNormal();
							GetMatrix().tz = CMaths::Min(m_fHitHeadHeight - fTopOfCol, aLegColPoints[0].GetPosition().z + PED_GROUNDOFFSET);
						}
						else
						{
							GetMatrix().tz = aLegColPoints[0].GetPosition().z + PED_GROUNDOFFSET;
						}						
					}
					else
					{
						GetMatrix().tz = aLegColPoints[0].GetPosition().z + PED_GROUNDOFFSET;
						if(m_nPedFlags.bHeadStuckInCollision)
							m_nPedFlags.bHeadStuckInCollision = false;
					}
				}
				else
				{
					if(m_nPedFlags.bHeadStuckInCollision)
					{
						if(!pEntity->GetIsTypeVehicle())
							vecHitHead_LegNormal = aLegColPoints[0].GetNormal();
					}
				}

				// Set the material of the ground (so that the sound code can use it)
				m_LastMaterialToHaveBeenStandingOn = aLegColPoints[0].GetSurfaceTypeB();
				m_vecGroundNormal = aLegColPoints[0].GetNormal();
				
				if(g_surfaceInfos.IsSteepSlope(m_LastMaterialToHaveBeenStandingOn))
				{
					m_nPedFlags.bHitSteepSlope = true;
//					m_vecDamageNormal = aLegColPoints[0].GetNormal();
				}
			}
			
			// stop z speed (hit ground sometimes)
			float fHitGround = 0.0f, fHurtSpdHorz = FALL_HURT_SPEED_H, fHurtSpdVert = FALL_HURT_SPEED_V;
			if(GetPedState()==PED_IDLE){
				fHurtSpdHorz *= 2.0f;
				fHurtSpdVert *= 1.5f;
			}
			// for player - may only want to inflict fall damage for actual falls, eg not after hit by car
			CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL);
			
			CVector vecLandSpeed = m_vecMoveSpeed;
			if(pEntity->GetIsPhysical())
				vecLandSpeed -= ((CPhysical *)pEntity)->m_vecMoveSpeed;
			
			if(!GetWasStanding() && ( ((fHitGround=vecLandSpeed.Magnitude2D()) > fHurtSpdHorz && !m_nPedFlags.bPushedAlongByCar) || vecLandSpeed.z < fHurtSpdVert)
			&& m_pNOCollisionVehicle!=pEntity && m_pMyVehicle!=pEntity)
			{
				float fHorzThreshHold = 0.25f;
				float fVertThreshHold = FALL_HURT_SPEED_V;
				if(g_surfaceInfos.IsSoftLanding(aLegColPoints[0].GetSurfaceTypeB()))
				{
					fHorzThreshHold *= 1.5f;
					fVertThreshHold *= 1.5f;
				}
				
				// want to save horizontal component for later
				float fHitGroundHorz = FALL_HURT_MULT_H*CMaths::Max(0.0f, fHitGround - fHorzThreshHold);
				fHitGround = fHitGroundHorz + FALL_HURT_MULT_V*CMaths::Max(0.0f, fVertThreshHold - vecLandSpeed.z);
				
				if(vecLandSpeed.z < FALL_HURT_SPEED_DIE)
					fHitGround = 500.0f;

				uint8 dir = 2;	// default dirn to 2 (fall on face)
				if(vecLandSpeed.x > 0.01f || vecLandSpeed.x < -0.01f || vecLandSpeed.y > 0.01f || vecLandSpeed.y < -0.01f)
					dir = GetLocalDirection(CVector2D::ConvertTo2D(-1.0f*vecLandSpeed));

				CEventDamage event(pEntity, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_FALL, PED_SPHERE_CHEST, dir, false, false);
				if(event.AffectsPed(this))
				{
					CPedDamageResponseCalculator damageResponseCalculator(pEntity, fHitGround, WEAPONTYPE_FALL, PED_SPHERE_CHEST, false);
					damageResponseCalculator.ComputeDamageResponse(this,event.GetDamageResponseData());
					GetPedIntelligence()->AddEvent(event);

					if(IsPlayer() && fHitGroundHorz > 5.0f)
					{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
						Say(AE_PLAYER_HIT_GROUND_GRUNT);
#endif //USE_AUDIOENGINE
					}
				}
										
			}
			// make sure if ped falls on their face they take at least 5 damage points
			else if(!GetWasStanding() && pAnim && vecLandSpeed.z<-2.0f*PHYSICAL_GRAVITYFORCE*CTimer::GetTimeStep() && m_pMyVehicle!=pEntity)
			{
				CEventDamage event(pEntity, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_FALL, PED_SPHERE_CHEST, 2, false, false);
				if(event.AffectsPed(this))
				{
					CPedDamageResponseCalculator damageResponseCalculator(pEntity,5.0f*3.0f,WEAPONTYPE_FALL, PED_SPHERE_CHEST,false);
					damageResponseCalculator.ComputeDamageResponse(this,event.GetDamageResponseData());
					GetPedIntelligence()->AddEvent(event);
				}
			}
			
			m_vecMoveSpeed.z = 0.0f;
			SetIsStanding(true);
		
			// reprocess collision collisions (not lines) if player has been moved up because of standing on something
			if(GetMatrix().tz > fOldZPos + 0.1f && pColData->m_nNoOfLines==0 && IsPlayer())
			{
				nNoOfCollisions = CCollision::ProcessColModels(GetMatrix(), *pPedCol, pEntity->GetMatrix(), pEntity->GetColModel(), aColPoints);
			}
		}
		else
		{
			if(m_nPedFlags.bCheckColAboveHead && aLegLineRatios[1] < 1.0f)
			{
				//if(aLegColPoints[1].GetPosition().z < m_fHitHeadHeight)
				if(aLegColPoints[1].GetPosition().z < m_fHitHeadHeight && (!pEntity->GetIsPhysical() || ((CPhysical *)pEntity)->m_nPhysicalFlags.bInfiniteMassFixed))
				{
					m_fHitHeadHeight = aLegColPoints[1].GetPosition().z;
				
					if(GetIsStanding() && m_fHitHeadHeight < GetPosition().z + fTopOfCol)
					{
						GetMatrix().tz = m_fHitHeadHeight - fTopOfCol;
					}
				}
			}
		
			m_nPedFlags.bOnBoat = false;
		}
	}

	for(int i=0; i<nNoOfCollisions; i++)
	{
		if(m_nPedFlags.bTestForBlockedPositions && GetUsesCollision() && aColPoints[i].GetPieceTypeA() > PED_COL_SPHERE_HEAD) 
		{
			bool bUseBlockedPoints = true;
			bool bUseUpperPoints = true;
			bool bUseLowerPoints = true;
			if(pEntity->GetIsTypePed())
			{
				bUseLowerPoints = false;
				if(((CPed *)pEntity)->m_nPedFlags.bFallenDown)
					bUseBlockedPoints = false;
				else if(((CPed *)pEntity)->m_nPedFlags.bIsDucking)
					bUseUpperPoints = false;
			}
		
			if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_UPPERARM_R && bUseBlockedPoints && bUseUpperPoints)
				m_nPedFlags.bRightArmBlocked = true;
			else if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_UPPERARM_L && bUseBlockedPoints && bUseUpperPoints)
				m_nPedFlags.bLeftArmBlocked = true;
			else if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_LEG_R && bUseBlockedPoints)
				m_nPedFlags.bDuckRightArmBlocked = true;
			else if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_MIDSECTION && bUseLowerPoints)
				m_nPedFlags.bMidriffBlockedForJump = true;
				
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
		// need to make sure trains don't push peds down through the ground
		else if(pEntity->GetIsTypeVehicle()	&& ((CVehicle*)pEntity)->GetVehicleType()==VEHICLE_TYPE_TRAIN
		&& ((CVehicle*)pEntity)->m_nPhysicalFlags.bInfiniteMass)
		{
			if(aColPoints[i].GetNormal().z < 0.0f)
			{
				aColPoints[i].GetNormal().z = 0.0f;
				aColPoints[i].GetNormal().Normalise();
			}
		}
	}

	// reset to 3 incase it's used outside of this function
//	pedCol.m_nNoOfSpheres = 3;
	// and set the peds heading back to what it should be if necessary
	if(fOriginalHeading > -1000.0)
		SetHeading(fOriginalHeading);
	
	DEBUGPROFILE(CProfile::SuspendProfile(PROFILE_COLLISION_TIME);)

	// change normals so none of them point down
	//
	// this is a fudge to avoid sticking under walls
	//
	if( (pEntity->GetIsTypeBuilding() || pEntity->GetIsStatic()) && GetWasStanding() )
	{
		for (int32 i = 0; i < nNoOfCollisions; i++)
		{
			CVector vecNormal = aColPoints[i].GetNormal();
			float mag2d = 0.0f;
			
			if(vecNormal.z < -0.99f && aColPoints[i].GetPieceTypeA()==PED_COL_SPHERE_HEAD && IsPlayer())
			{
//				printf("oww my head");
				mag2d = CMaths::Sqrt(m_vecMoveSpeed.x*m_vecMoveSpeed.x + m_vecMoveSpeed.y*m_vecMoveSpeed.y);
				if(mag2d < 0.001f)	mag2d = 0.001f;
				vecNormal.x = -m_vecMoveSpeed.x / mag2d;
				vecNormal.y = -m_vecMoveSpeed.y / mag2d;
				
				GetMatrix().tz -= 0.05f;
				m_nPedFlags.bHeadStuckInCollision = true;
				m_nPedFlags.bHitSteepSlope = true;
			}
			else
			{
				mag2d = CMaths::Sqrt(vecNormal.x*vecNormal.x + vecNormal.y*vecNormal.y);
				if(mag2d != 0.0f)
				{
					vecNormal.x /= mag2d;
					vecNormal.y /= mag2d;
				}
			}

// removing this seems to solve a lot of ped collision problems relating to
// overhanging building collisions - hope it doesn't break anything else.
//			if ((vecNormal.z < 0.0f) && (vecNormal.z > -1.0f))
//				vecNormal.z = 0.0f;
				
			vecNormal.Normalise();
			aColPoints[i].SetNormal(vecNormal);
			
			// want to save whether ped has collided with a steep slope (to make sure he slides down!)
			if(g_surfaceInfos.IsSteepSlope(aColPoints[i].GetSurfaceTypeB()))
				m_nPedFlags.bHitSteepSlope = true;
		}
	}
	
	if(vecHitHead_LegNormal.z < 1.0f)// && !CWorld::bSecondShift)
	{
		aColPoints[nNoOfCollisions].GetNormal().x = vecHitHead_LegNormal.x;
		aColPoints[nNoOfCollisions].GetNormal().y = vecHitHead_LegNormal.y;
		aColPoints[nNoOfCollisions].GetNormal().z = 0.0f;
		aColPoints[nNoOfCollisions].GetNormal().Normalise();
		
		aColPoints[nNoOfCollisions].GetPosition() = GetPosition() - PED_NOMINAL_RADIUS*aColPoints[nNoOfCollisions].GetNormal();
		nNoOfCollisions++;
		m_nPedFlags.bHitSteepSlope = true;
	}

	// record collision
	if(nNoOfCollisions > 0 || aLegLineRatios[0] < 1.0f)
	{
		AddCollisionRecord(pEntity);

		if (!pEntity->GetIsTypeBuilding())
			((CPhysical*) pEntity)->AddCollisionRecord(this);

		if( nNoOfCollisions > 0 && (pEntity->GetIsTypeBuilding() || pEntity->GetIsStatic()) )
			SetHasHitWall(true);
	}

	return nNoOfCollisions;

//	return 0;
}

#else // #ifdef PED_LINE_IN_COL
//
int32 CPed::ProcessEntityCollision(CEntity* pEntity, class CColPoint* aColPoints)
{
	CColPoint legsColPoint;
	CVector vecHitHead_LegNormal(0.0f,0.0f,1.0f);
	bool bLegsTouchEntity = false;
	bool bProcessingBoat = false;
	float fGroundColLineOffset = PED_GROUNDCOLLINE_OFFSET_TIMESTEP * CTimer::GetTimeStep();
	
	if(!GetUsesCollision() && !m_nPhysicalFlags.bForceHitReturnFalse)
		return 0;

	if(pEntity->GetIsTypeVehicle() && ((CVehicle *)pEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
		bProcessingBoat = true;

	if(!m_nPhysicalFlags.bHalfSpeedCollision && !m_nPhysicalFlags.bSkipLineCol
	&& m_pAttachToEntity==NULL && !pEntity->GetIsTypePed())
	{
		// If not come across a ProcessEntityCollision function yet and ped is standing on the ground then 
		// clear flag but set was standing flag
		if (!GetCollisionProcessed())
		{
			// reset ground physical pointer
			m_pGroundPhysical = NULL;

			if(GetIsStanding())
			{
				SetIsStanding(false);
				SetWasStanding(true);
			}
				
			SetCollisionProcessed(true);
			
			m_distTravelledSinceLastHeightCheck += m_vecMoveSpeed.Magnitude2D() * CTimer::GetTimeStep();
			m_nPedFlags.bStillOnValidPoly = false;
			// Don't do this for the player. If distance travelled since last fulll collision check is less than 1 metre or ped 
			// is wandering along a path and the last check is less than 2 metres just do a check against the stored polygon
			if(!IsPlayer() && 
				(m_distTravelledSinceLastHeightCheck < 1.0f || 
					(m_distTravelledSinceLastHeightCheck < 2.0f && m_nPedState == PED_WANDER_PATH)))
			{
				CVector lineStart(GetMatrix().tx, GetMatrix().ty, GetMatrix().tz);
				float lineEnd = GetMatrix().tz - PED_GROUNDOFFSET;
				if(GetWasStanding())
				{
					lineStart.z += PED_GROUNDCOLLINE_OFFSET; //fGroundColLineOffset;
					lineEnd += fGroundColLineOffset;//PED_GROUNDCOLLINE_OFFSET;
				}	
				if(CCollision::IsStoredPolyStillValidVerticalLine(lineStart, lineEnd, legsColPoint, &m_storedCollPoly))
				{
					m_nPedFlags.bStillOnValidPoly = true;
					// set body position
					if(!m_nPedFlags.bHeadStuckInCollision || (legsColPoint.GetPosition().z + PED_GROUNDOFFSET) < GetMatrix().tz)
					{
						GetMatrix().tz = legsColPoint.GetPosition().z + PED_GROUNDOFFSET;
						if(m_nPedFlags.bHeadStuckInCollision)	m_nPedFlags.bHeadStuckInCollision = false;
					}

					m_vecMoveSpeed.z = 0.0f;
					SetIsStanding(true);
				}
				else
				{
					m_storedCollPoly.bValidPolyStored = false;
					m_distTravelledSinceLastHeightCheck = 0.0f;
					m_nPedFlags.bHitSteepSlope = false;
				}
			}
			else
			{
				m_storedCollPoly.bValidPolyStored = false;
				m_distTravelledSinceLastHeightCheck = 0.0f;
				m_nPedFlags.bHitSteepSlope = false;
			}
		}
	

//		if(!m_storedCollPoly.bValidPolyStored)
		if(!m_nPedFlags.bStillOnValidPoly)
		{
			CVector vecCentre(GetMatrix().tx, GetMatrix().ty, GetMatrix().tz - PED_GROUNDOFFSET/2.0f);
			float radius = pEntity->GetBoundRadius() + (PED_GROUNDOFFSET/2.0f) - fGroundColLineOffset;//PED_GROUNDCOLLINE_OFFSET;
//			float radius = PED_GROUNDOFFSET/2.0f - fGroundColLineOffset;
			// If Ped was standing then extend the ped collision line by an extra 25cm to stop peds moving slightly
			// up and down and also to stop peds floating down stairs. If Ped is in the air don't want to do this
			// because it is hard to force peds off the ground
			if(GetWasStanding())
			{
				if(bProcessingBoat){
					vecCentre.z += 2.0f*fGroundColLineOffset;//PED_GROUNDCOLLINE_OFFSET;
					radius += CMaths::Abs(fGroundColLineOffset);//PED_GROUNDCOLLINE_OFFSET);
				}
				else
					vecCentre.z += fGroundColLineOffset;//PED_GROUNDCOLLINE_OFFSET;
			}

			if ((vecCentre - pEntity->GetBoundCentre()).MagnitudeSqr() < (radius*radius))
			{
				CColLine line;

				line.m_vecStart.x = GetMatrix().tx;
				line.m_vecStart.y = GetMatrix().ty;
				line.m_vecStart.z = GetMatrix().tz;
				line.m_vecEnd.x = GetMatrix().tx;
				line.m_vecEnd.y = GetMatrix().ty;
				line.m_vecEnd.z = GetMatrix().tz - PED_GROUNDOFFSET;

				if(GetWasStanding() && (!GetPedIntelligence()->GetTaskJetPack()
				|| !GetPedIntelligence()->GetTaskJetPack()->GetThrust()))
				{
					line.m_vecEnd.z += fGroundColLineOffset; //PED_GROUNDCOLLINE_OFFSET;
					line.m_vecStart.z += PED_GROUNDCOLLINE_OFFSET; //fGroundColLineOffset;
				}
				
				{
					DEBUGPROFILE(CProfile::ResumeProfile(PROFILE_COLLISION_TIME);)
					float fRatio = 1.0f;
					//bLegsTouchEntity = CCollision::ProcessLineOfSight(line, pEntity->GetMatrix(), pEntity->GetColModel(), legsColPoint, fRatio);
					bLegsTouchEntity = CCollision::ProcessVerticalLine(line, pEntity->GetMatrix(), pEntity->GetColModel(), legsColPoint, fRatio, false, false, &m_storedCollPoly);

					// if ped was standing on a boat last frame AND initial line of sight check failed
					// want to check further down incase the boat fell away from below ped standing on moving boat
					if(bProcessingBoat && GetWasStanding() && !bLegsTouchEntity){
						// move complete line down by 25cm
						line.m_vecStart.z = line.m_vecEnd.z;
						line.m_vecEnd.z += fGroundColLineOffset;//PED_GROUNDCOLLINE_OFFSET;
						// do line check again
						bLegsTouchEntity = CCollision::ProcessVerticalLine(line, pEntity->GetMatrix(), pEntity->GetColModel(), legsColPoint, fRatio, false, false, &m_storedCollPoly);
					}
					
					DEBUGPROFILE(CProfile::SuspendProfile(PROFILE_COLLISION_TIME);)

					// If line hit something that isn't a ped
					if (bLegsTouchEntity)
					{
						// If hasn't hit anything yet that was a ped or have hit something below the object you have hit
						// then set the ped z coord
						if(!GetIsStanding() || GetMatrix().tz < legsColPoint.GetPosition().z + PED_GROUNDOFFSET || (bProcessingBoat && GetMatrix().tz < legsColPoint.GetPosition().z + 3.0f*PED_GROUNDOFFSET))
						{
							if(IsPlayer())
							{
								static float PLAYER_LIGHTING_CHANGE_RATE = 0.1f;
								float fChangeRate = PLAYER_LIGHTING_CHANGE_RATE*CTimer::GetTimeStep();
								m_lightingFromCollision	= fChangeRate*CColTriangle::CalculateLighting(legsColPoint.GetLightingB()) + (1.0f - fChangeRate)*m_lightingFromCollision;
							}
							else
								m_lightingFromCollision = CColTriangle::CalculateLighting(legsColPoint.GetLightingB());
							
							// If hit a vehicle or object then store the object in the ped structure
							if (pEntity->GetIsTypeVehicle() || pEntity->GetIsTypeObject())
							{
								m_pGroundPhysical = (CPhysical*) pEntity;
								REGREF(pEntity, (CEntity **)&m_pGroundPhysical);
								m_vecGroundOffset = legsColPoint.GetPosition() - pEntity->GetPosition();
								m_pEntityStandingOn = pEntity;
								REGREF(pEntity, (CEntity **)&m_pEntityStandingOn);
								// don't use stored polygon if it belongs to a physical object
								m_storedCollPoly.bValidPolyStored = false;

								// if we're standing on a boat, want rescued from the water if we fall in (done in playerped now)
								if(pEntity->GetIsTypeVehicle() && ((CVehicle *)pEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
									m_nPedFlags.bOnBoat = true;
								else
									m_nPedFlags.bOnBoat = false;
								
								// If we landed on a vehicle get the lighting from the vehicle	
								if(pEntity->GetIsTypeVehicle())
									m_lightingFromCollision = m_pGroundPhysical->m_lightingFromCollision;
							}
							else
							{
								m_pEntityStandingOn = pEntity;
								REGREF(pEntity, (CEntity **)&m_pEntityStandingOn);
								// if standing on the ground then must've reached safe ground from the boat
								m_nPedFlags.bTryingToReachDryLand = false;
								m_nPedFlags.bOnBoat = false;
							}

							// set body position
							if(m_nPedFlags.bHeadStuckInCollision == false || (legsColPoint.GetPosition().z + PED_GROUNDOFFSET) < GetMatrix().tz)
							{
								GetMatrix().tz = legsColPoint.GetPosition().z + PED_GROUNDOFFSET;
								if(m_nPedFlags.bHeadStuckInCollision)	m_nPedFlags.bHeadStuckInCollision = false;
							}
							else
							{
								// we're not using this poly to position ped, so don't store it
								m_storedCollPoly.bValidPolyStored = false;
								
								if(m_nPedFlags.bHeadStuckInCollision)
									vecHitHead_LegNormal = legsColPoint.GetNormal();
							}

							// Set the material of the ground (so that the sound code can use it)
							m_LastMaterialToHaveBeenStandingOn = legsColPoint.GetSurfaceTypeB();
							m_vecGroundNormal = legsColPoint.GetNormal();
							
							if(g_surfaceInfos.IsSteepSlope(m_LastMaterialToHaveBeenStandingOn))
							{
								m_nPedFlags.bHitSteepSlope = true;
//								m_vecDamageNormal = legsColPoint.GetNormal();
							}
						}
						else
						{
							// we're not using this poly to position ped, so don't store it
							m_storedCollPoly.bValidPolyStored = false;
						}
						
						// stop z speed (hit ground sometimes)
						float fHitGround = 0.0f, fHurtSpdHorz = FALL_HURT_SPEED_H, fHurtSpdVert = FALL_HURT_SPEED_V;
						if(GetPedState()==PED_IDLE){
							fHurtSpdHorz *= 2.0f;
							fHurtSpdVert *= 1.5f;
						}
						// for player - may only want to inflict fall damage for actual falls, eg not after hit by car
						CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL);
						
						
						if(!GetWasStanding() && ( ((fHitGround=m_vecMoveSpeed.Magnitude2D()) > fHurtSpdHorz && !m_nPedFlags.bPushedAlongByCar) || m_vecMoveSpeed.z < fHurtSpdVert)
						&& m_pNOCollisionVehicle!=pEntity)
						{
							float fHitGroundHorz = fHitGround = FALL_HURT_MULT_H*MAX(0.0f, fHitGround - 0.25f);
							if(m_vecMoveSpeed.z < FALL_HURT_SPEED_V)
								fHitGround += FALL_HURT_MULT_V*(FALL_HURT_SPEED_V - m_vecMoveSpeed.z);
						
							uint8 dir = 2;	// default dirn to 2 (fall on face)
							if(m_vecMoveSpeed.x > 0.01f || m_vecMoveSpeed.x < -0.01f || m_vecMoveSpeed.y > 0.01f || m_vecMoveSpeed.y < -0.01f)
								dir = GetLocalDirection(CVector2D::ConvertTo2D(-1.0f*m_vecMoveSpeed));
							if (g_surfaceInfos.IsSoftLanding(legsColPoint.GetSurfaceTypeB())) fHitGround *= 0.5f;

							CEventDamage event(pEntity, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_FALL, PED_SPHERE_CHEST, dir, false);
							if(event.AffectsPed(this))
							{
								CPedDamageResponseCalculator damageResponseCalculator(pEntity,fHitGround,WEAPONTYPE_FALL, PED_SPHERE_CHEST,false);
								damageResponseCalculator.ComputeDamageResponse(this,event.GetDamageResponseData());

								GetPedIntelligence()->AddEvent(event);

								if(IsPlayer() && fHitGroundHorz > 5.0f)
								{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
									Say(AE_PLAYER_HIT_GROUND_GRUNT);
#endif //USE_AUDIOENGINE
								}
							}
													
						}
						else if(!GetWasStanding() && pAnim && m_vecMoveSpeed.z<-2.0f*PHYSICAL_GRAVITYFORCE*CTimer::GetTimeStep())	// make sure if ped falls on their face they take at least 5 damage points
						{
							CEventDamage event(pEntity, CTimer::GetTimeInMilliseconds(), WEAPONTYPE_FALL, PED_SPHERE_CHEST, 2, false);
							if(event.AffectsPed(this))
							{
								CPedDamageResponseCalculator damageResponseCalculator(pEntity,5.0f*3.0f,WEAPONTYPE_FALL, PED_SPHERE_CHEST,false);
								damageResponseCalculator.ComputeDamageResponse(this,event.GetDamageResponseData());
								GetPedIntelligence()->AddEvent(event);
							}
						}	
						m_vecMoveSpeed.z = 0.0f;

						SetIsStanding(true);
					}
					else
						m_nPedFlags.bOnBoat = false;
				}		
			}
		}	
	}
	
	DEBUGPROFILE(CProfile::ResumeProfile(PROFILE_COLLISION_TIME);)
	ASSERTMSG( &(CModelInfo::GetColModel(m_nModelIndex)) == &CTempColModels::ms_colModelPed1, "Wrong Ped ColModel: bugger");
	ASSERTMSG( CModelInfo::GetColModel(m_nModelIndex).m_nNoOfLines == 0, "Ped CoModel has lines??: oops");

	CColModel *pPedCol = &CModelInfo::GetColModel(m_nModelIndex);
	// setup how many of the peds col spheres we want to test
	float fOriginalHeading = -1001.0f;
	if(m_nPedFlags.bTestForBlockedPositions && GetUsesCollision())// && !pEntity->GetIsTypePed())
	{
		if(GetPedIntelligence()->GetTaskUseGun() && GetPedIntelligence()->GetTaskUseGun()->GetWeaponInfo()
		&& GetPedIntelligence()->GetTaskUseGun()->GetWeaponInfo()->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM)
		&& IsPlayer() && TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_AIMWEAPON)
		{
			fOriginalHeading = GetHeading();
			SetHeading(CMaths::ATan2(-TheCamera.Cams[TheCamera.ActiveCam].Front.x, TheCamera.Cams[TheCamera.ActiveCam].Front.y));
		}
		
		pPedCol = &CTempColModels::ms_colModelPed2;
//		pedCol.m_nNoOfSpheres = 9;
	}
//	else
//		pedCol.m_nNoOfSpheres = 3;

	int32 nNoOfCollisions = CCollision::ProcessColModels(GetMatrix(), *pPedCol, pEntity->GetMatrix(), pEntity->GetColModel(), aColPoints);
	for(int i=0; i<nNoOfCollisions; i++)
	{
		if(m_nPedFlags.bTestForBlockedPositions && GetUsesCollision() && aColPoints[i].GetPieceTypeA() > 0 
		&& aColPoints[i].GetPieceTypeA()!=PED_SPHERE_HEAD)
		{
			bool bUseBlockedPoint = true;
			if(pEntity->GetIsTypePed() && ((CPed *)pEntity)->m_nPedFlags.bFallenDown)
				bUseBlockedPoint = false;
		
			if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_UPPERARM_R && bUseBlockedPoint)
				m_nPedFlags.bRightArmBlocked = true;
			else if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_UPPERARM_L && bUseBlockedPoint)
				m_nPedFlags.bLeftArmBlocked = true;
			else if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_LEG_R && bUseBlockedPoint)
				m_nPedFlags.bDuckRightArmBlocked = true;
			else if(aColPoints[i].GetPieceTypeA()==PED_SPHERE_MIDSECTION && !pEntity->GetIsTypePed())
				m_nPedFlags.bMidriffBlockedForJump = true;
			else
				continue;
				
			// we might want to use these as soft collisions to push peds away from
			// weapons for example
//			static bool TEST_PED_ARMS_SOFT_COL = false;
//			if(TEST_PED_ARMS_SOFT_COL)
//			{
//				aColPoints[i].SetPieceTypeA(COLPOINT_PIECETYPE_FRONTLEFTWHEEL);
//			}
//			else			
			// but most likely we'll just use the results in pedIntelligence so remove collision
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

	// reset to 3 incase it's used outside of this function
//	pedCol.m_nNoOfSpheres = 3;
	// and set the peds heading back to what it should be if necessary
	if(fOriginalHeading > -1000.0)
		SetHeading(fOriginalHeading);
	
	DEBUGPROFILE(CProfile::SuspendProfile(PROFILE_COLLISION_TIME);)

	// record collision
	if(nNoOfCollisions > 0 || bLegsTouchEntity)
	{
		AddCollisionRecord(pEntity);

		if (!pEntity->GetIsTypeBuilding())
			((CPhysical*) pEntity)->AddCollisionRecord(this);

		if( nNoOfCollisions > 0 && (pEntity->GetIsTypeBuilding() || pEntity->GetIsStatic()) )
			SetHasHitWall(true);
	}
	
	// change normals so none of them point down
	//
	// this is a fudge to avoid sticking under walls
	//
	if( (pEntity->GetIsTypeBuilding() || pEntity->GetIsStatic()) && GetWasStanding() )
	{
		for (int32 i = 0; i < nNoOfCollisions; i++)
		{
			CVector vecNormal = aColPoints[i].GetNormal();
			float mag2d = 0.0f;
			
			if(vecNormal.z < -0.99f && aColPoints[i].GetPieceTypeA()==PED_COL_SPHERE_HEAD && IsPlayer())
			{
//				printf("oww my head");
				mag2d = CMaths::Sqrt(m_vecMoveSpeed.x*m_vecMoveSpeed.x + m_vecMoveSpeed.y*m_vecMoveSpeed.y);
				if(mag2d < 0.001f)	mag2d = 0.001f;
				vecNormal.x = -m_vecMoveSpeed.x / mag2d;
				vecNormal.y = -m_vecMoveSpeed.y / mag2d;
				
				GetMatrix().tz -= 0.05f;
				m_nPedFlags.bHeadStuckInCollision = true;
				m_nPedFlags.bHitSteepSlope = true;
			}
			else
			{
				mag2d = CMaths::Sqrt(vecNormal.x*vecNormal.x + vecNormal.y*vecNormal.y);
				if(mag2d != 0.0f)
				{
					vecNormal.x /= mag2d;
					vecNormal.y /= mag2d;
				}
			}

// removing this seems to solve a lot of ped collision problems relating to
// overhanging building collisions - hope it doesn't break anything else.
//			if ((vecNormal.z < 0.0f) && (vecNormal.z > -1.0f))
//				vecNormal.z = 0.0f;
				
			vecNormal.Normalise();
			aColPoints[i].SetNormal(vecNormal);
			
			// want to save whether ped has collided with a steep slope (to make sure he slides down!)
			if(g_surfaceInfos.IsSteepSlope(aColPoints[i].GetSurfaceTypeB()))
				m_nPedFlags.bHitSteepSlope = true;
		}
	}
	
	if(vecHitHead_LegNormal.z < 1.0f)
	{
		aColPoints[nNoOfCollisions].GetNormal().x = vecHitHead_LegNormal.x;
		aColPoints[nNoOfCollisions].GetNormal().y = vecHitHead_LegNormal.y;
		aColPoints[nNoOfCollisions].GetNormal().Normalise();
		
		aColPoints[nNoOfCollisions].GetPosition() = GetPosition() - PED_NOMINAL_RADIUS*aColPoints[nNoOfCollisions].GetNormal();
		nNoOfCollisions++;
		m_nPedFlags.bHitSteepSlope = true;
	}

	return nNoOfCollisions;

//	return 0;
}
#endif //#ifdef PED_LINE_IN_COL


#pragma optimize_for_size on
#pragma mark --- optimize for size on ---


static
void particleProduceFootSplash(CPed *ThePed, const CVector& Position, float Size=0.15f, int32 ParticleCount=4);
static
void particleProduceFootDust(CPed *ThePed, const CVector& Position, float Size=0.0f, int32 ParticleCount=4);
//
//
// helper function for CPed::PlayFootSteps():
//
static
void particleProduceFootSplash(CPed *ThePed, const CVector& Position, float Size, int32 ParticleCount)
{
	if (ThePed->GetIsOnScreen() == false)
	{
		return;
	}
	
	CVector vec = Position - TheCamera.GetPosition();
	vec.z = 0.0f;
	if (vec.MagnitudeSqr() > 10.0f*10.0f)
	{	
		return;
	}

	// MN - FX SYSTEM -------------------------------	
	// ped foot splashes	
	
	const float SPLASH_RADIUS = 0.1f;//0.2f;

	//					red   green blue  alpha size   rot   life
	FxPrtMult_c fxMults(1.0f, 1.0f, 1.0f, 0.1f, 0.15f, 0.0f, 0.15f);
	
	for(int32 i=0; i<ParticleCount; i++)
	{
		CVector pos(Position);
		pos.x += CGeneral::GetRandomNumberInRange(-SPLASH_RADIUS, SPLASH_RADIUS);
		pos.y += CGeneral::GetRandomNumberInRange(-SPLASH_RADIUS, SPLASH_RADIUS);
		
		RwV3d zeroVec = {0.0f, 0.0f, 0.0f};
		CVector speed = ThePed->GetMatrix().GetForward() * 1.5;
	
		g_fx.m_fxSysSplash->AddParticle(&pos, &zeroVec, 0.0f, &fxMults);
		g_fx.m_fxSysSplash->AddParticle(&pos, &speed, 0.0f, &fxMults);
	}
	
	// ----------------------------------------------
	
}// end of particleProduceFootSplash()...


//
//
// helper function for CPed::PlayFootSteps():
//
static
void particleProduceFootDust(CPed *ThePed, const CVector& Position, float Size, int32 ParticleCount)
{
	if (ThePed->GetIsOnScreen() == false)
	{
		return;
	}
	
	CVector vec = Position - TheCamera.GetPosition();
	vec.z = 0.0f;
	if (vec.MagnitudeSqr() > 10.0f*10.0f)
	{	
		return;
	}
	
	// MN - FX SYSTEM -------------------------------
	// player foot dust
	const float FOOTDUST_VAR_POS	= 0.1f;
	const float FOOTDUST_VAR_SPEED	= 0.15f;

	//					red   green blue  alpha size   rot   life
	FxPrtMult_c fxMults(1.0f, 1.0f, 1.0f, 0.1f, 0.15f, 0.0f, 0.15f);
/*
	if (g_bUsingAnimViewer)
	{
		fxMults.m_red = 1.0f;
		fxMults.m_green = 0.0f;
		fxMults.m_blue = 0.0f;
		fxMults.m_alpha = 1.0f;
		fxMults.m_size = 0.05f;
		fxMults.m_life = 0.01f;
	}
*/
/*		
	// TODO: put back in when materials are all set up
	
	switch(ThePed->m_LastMaterialToHaveBeenStandingOn)
	{
		// produce dust only for following material types:
		case(COLPOINT_SURFACETYPE_TARMAC):
		case(COLPOINT_SURFACETYPE_PAVEMENT):
		case(COLPOINT_SURFACETYPE_GRAVEL):
		case(COLPOINT_SURFACETYPE_FUCKED_CONCRETE):
		case(COLPOINT_SURFACETYPE_SAND_DEEP):
		case(COLPOINT_SURFACETYPE_SAND_MEDIUM):
		case(COLPOINT_SURFACETYPE_SAND_COMPACT):
		case(COLPOINT_SURFACETYPE_SAND_ARID):
		case(COLPOINT_SURFACETYPE_SAND_MORE):
		case(COLPOINT_SURFACETYPE_ROCK_DRY):
		case(COLPOINT_SURFACETYPE_CONCRETE_BEACH):
		{
*/		

		if (g_surfaceInfos.ProducesFootDust(ThePed->m_LastMaterialToHaveBeenStandingOn))
		{
			for(int32 i=0; i<ParticleCount; i++)
			{
				CVector pos(Position);
				pos.x += CGeneral::GetRandomNumberInRange(-FOOTDUST_VAR_POS, FOOTDUST_VAR_POS);
				pos.y += CGeneral::GetRandomNumberInRange(-FOOTDUST_VAR_POS, FOOTDUST_VAR_POS);

				CVector speed;
				speed.x = CGeneral::GetRandomNumberInRange(-FOOTDUST_VAR_SPEED, FOOTDUST_VAR_SPEED);
				speed.y = CGeneral::GetRandomNumberInRange(-FOOTDUST_VAR_SPEED, FOOTDUST_VAR_SPEED);
				speed.z = 0.0f;

	
				if (!g_bUsingAnimViewer)
				{
					g_fx.m_fxSysSmoke2->AddParticle(&pos, &speed, 0.0f, &fxMults);
				}
			}
		}
		
		
/*			
			break;
		}
		default:
		{
			break;				
		}
	}
*/	// ----------------------------------------------

}// end of particleProduceFootDust()...



// MN - does stuff that happens when a foot lands on the ground
//      e.g. footprints and wobble activation etc
void CPed::DoFootLanded(bool left, bool8 doWobble)
{
	// If the ped is off-screen don't do anything
	if( (!m_nPedFlags.bCalledPreRender) || (m_nFlags.bDontUpdateHierarchy))
		return;

	CVector vec;
	if(left)
		GetBonePosition(vec, BONETAG_L_FOOT);
	else
		GetBonePosition(vec, BONETAG_R_FOOT);

	{
		const CVector vecFwd(GetMatrix().GetForward());
		const CVector vecRgt(GetMatrix().GetRight());
		vec.z -= 0.1f;//0.15f;//0.1f
		vec += 0.20f*vecFwd;//0.15f*vecFwd;
		

		// bloody footprints:
		if(m_nPedFlags.bDoBloodyFootprints && CLocalisation::Blood())
		{
			CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpBloodPoolTex, &vec, 0.26f*vecFwd.x, 0.26f*vecFwd.y,
						0.14f*vecRgt.x, 0.14f*vecRgt.y, 255, 200, 0, 0, PED_SHADOW_DEPTH, 3000);
			if( m_nTimeOfDeath > 20)
				m_nTimeOfDeath -= 20;
			else
			{
				m_nTimeOfDeath = 0;
				m_nPedFlags.bDoBloodyFootprints = false;
			}
		}
		
		// footpaths on beaches:
		if( g_surfaceInfos.LeavesFootsteps(m_LastMaterialToHaveBeenStandingOn) && (TheCamera.GetPosition() - GetPosition()).Magnitude2D() < 10.0f)
		{
			CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpShadowPedTex, &vec, -0.26f*vecFwd.x, -0.26f*vecFwd.y,
						-0.10f*vecRgt.x, -0.10f*vecRgt.y, 120, 250, 250, 50, PED_SHADOW_DEPTH, (this->IsPlayer())? 5000 : 2000);
		}
		
		
		// footsplashes:
		if( (CWeather::Rain>0.1f) && (!CCullZones::CamNoRain()) && (!CCullZones::PlayerNoRain()) && CGame::currArea == AREA_MAIN_MAP)
		{
			//
			// it's raining: we produce rain splashes on player feet:
			//
			particleProduceFootSplash(this, vec);
			
		}

	//	if (this->m_LastMaterialToHaveBeenStandingOn == COLPOINT_SURFACETYPE_GRAVEL)  // dusty gravel
		{
			//
			// we produce dust on player feet
			//
		//	if(this->IsPlayer())
			particleProduceFootDust(this, vec);
		}
	}

	// set some wobble info to get bellys and breasticles going
	if (doWobble)
	{	
		m_wobble = 2*PI;	
		m_wobbleSpeed = 0.4f + 20.0f*(m_vecCurrentVelocity.x*m_vecCurrentVelocity.x + m_vecCurrentVelocity.y*m_vecCurrentVelocity.y);	
	}


	if (m_nPhysicalFlags.bIsInWater)
	{
		CVector pedPos = GetPosition();
		float waterHeight;
		bool bWaterFound = CWaterLevel::GetWaterLevel(pedPos.x, pedPos.y, pedPos.z+1.5f, &waterHeight, true);
		CVector ripplePos = pedPos + m_vecMoveSpeed*CTimer::GetTimeStep()*2.0f;
		ripplePos.z = waterHeight;	

		if (waterHeight<pedPos.z-0.4f)
		{
			g_fx.TriggerFootSplash(ripplePos);
#ifdef USE_AUDIOENGINE
			m_PedAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_FOOTSTEP_LEFT, 0.0f, 1.0f, NULL,
				SURFACE_TYPE_WATER_SHALLOW);
#endif //USE_AUDIOENGINE
		}
	}
	else if (g_surfaceInfos.IsShallowWater(m_LastMaterialToHaveBeenStandingOn))
	{
		CVector footPos = vec;
		static float yOffset = -0.2f;
		static float zOffset = 0.3f;
		footPos.z += zOffset;
		footPos += GetForward()*yOffset;
		g_fx.TriggerFootSplash(footPos);
#ifdef USE_AUDIOENGINE
		m_PedAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_FOOTSTEP_LEFT, 0.0f, 1.0f, NULL,
			SURFACE_TYPE_WATER_SHALLOW);
#endif //USE_AUDIOENGINE
	}
//	SET_FACIAL_EXPRESSION(this, CTaskSimpleFacial::FACIAL_TALKING, 20000, -1, 0);
}




#define PEDANIM_FOOTDOWNFRAME	(2.0f/30.0f)

//
//
// Name			:	PlayFootSteps
// Purpose		:	play footstep samples
//
void CPed::PlayFootSteps()
{

	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject);
	CAnimBlendAssociation* pWalk=NULL;
	CAnimBlendAssociation* pSkid=NULL;
	float partialBlend = 0.0f;
	float walkcycleBlend = 0.0f;
	
		
	// test if the anim is a standard walk, run, sprint - we want to do a wobble if so
	bool8 doWobble = false;
	int32 animId = pAnim->GetAnimId();
	if (animId==ANIM_STD_WALK || animId==ANIM_STD_RUN || animId==ANIM_STD_RUNFAST)
	{
		doWobble = true;
	}

	// is ped  a skater? (we don't want skating peds to leave footpaths on sand):
	bool bThisPedIsSkater = (this->m_pPedStats==CPedStats::GetPedStatInfo(PEDSTAT_SKATER));

	if(m_nPedFlags.bDoBloodyFootprints && m_nTimeOfDeath > 0 && m_nTimeOfDeath < 300)
	{
		if(--m_nTimeOfDeath == 0)
			m_nPedFlags.bDoBloodyFootprints = false;
	}
	
	// make sure ped is standing on something
	if(!GetIsStanding())
		return;
		
	while(pAnim)
	{
		if(pAnim->IsFlagSet(ABA_PEDFLAG_WALKCYCLE))
		{
			pWalk = pAnim;
			walkcycleBlend += pAnim->GetBlendAmount();
		}
		else if(!pAnim->IsFlagSet(ABA_PEDFLAG_UPPERBODYONLY) && (pAnim->GetAnimId() != ANIM_MELEE_IDLE) &&
			(pAnim->IsFlagSet(ABA_FLAG_ISPARTIAL) || !m_nPedFlags.bIsDucking))
		{
			partialBlend += pAnim->GetBlendAmount();
		}
			
		pAnim = RpAnimBlendGetNextAssociation(pAnim);
	}


	// produce footdust only for main player:	
//	if(this->IsPlayer())
	if(false)
	{
		pSkid = RpAnimBlendClumpGetAssociation((RpClump*)this->m_pRwObject, ANIM_STD_RUNSTOP1);
		if(pSkid==NULL)
			pSkid = RpAnimBlendClumpGetAssociation((RpClump*)this->m_pRwObject, ANIM_STD_RUNSTOP2);
	}

	//
	// is player skidding?
	//
	if(pSkid!=NULL && pSkid->GetBlendAmount() > 0.1f )
	{
		//printf(" ** Player is skidding!\n\n");

		//
		// left foot:
		//
		{
			CVector vec;
			GetBonePosition(vec, BONETAG_L_FOOT);

			const CVector vecFwd(GetMatrix().GetForward());
			const CVector vecRgt(GetMatrix().GetRight());
			vec.z -= 0.1f;//0.15f;//0.1f;
			vec += 0.20f*vecFwd;//0.15f*vecFwd;
			
			particleProduceFootDust(this, vec, 0.02f, 1);
		}
		
		//
		// right foot:
		//
		{
			CVector vec;
			GetBonePosition(vec, BONETAG_R_FOOT);
			const CVector vecFwd(GetMatrix().GetForward());
			const CVector vecRgt(GetMatrix().GetRight());
			vec.z -= 0.1f;//0.15f;//0.1f;
			vec += 0.20f*vecFwd;//0.15f*vecFwd;
			
			particleProduceFootDust(this, vec, 0.02f, 1);
		}
	}//if(is player skidding)?...
	

	
	
	if(pWalk && walkcycleBlend > 0.5f && partialBlend < 1.0f)
	{
		// MN - times when the feet are on the ground
		#define	INITIAL_FOOT_LANDED_RATIO (1.0f/15.0f)
		float leftFootLanded  = pWalk->GetTotalTime() * INITIAL_FOOT_LANDED_RATIO;
		float rightFootLanded = pWalk->GetTotalTime() / 2.0f + leftFootLanded;
	
		if(m_nPedFlags.bIsDucking)
		{
			#define DUCK_FOOTSTEP_MOVE_TIME (6.0f/30.0f);
			leftFootLanded  += DUCK_FOOTSTEP_MOVE_TIME;
			rightFootLanded += DUCK_FOOTSTEP_MOVE_TIME;
		}

		float rightFootDown = PEDANIM_FOOTDOWNFRAME + pWalk->GetTotalTime() / 2.0f;
		
		if( bThisPedIsSkater )
		{
			CVector vec;
			const RwRGBA colSand = {170, 165, 140, 255};
			float leftFootDown;

			static float fWalkLeft = 1.0f;
			static float fWalkRight = 15.0f;

			if(pWalk->GetAnimId()==ANIM_STD_WALK)
			{
				leftFootDown  = 0.0f;
				rightFootDown = 16.0f/30.0f;
			}
			else
			{
				leftFootDown  = 0.0f;
				rightFootDown = 10.0f/30.0f;
			}

			float fVolume = 1.0f;
			switch(g_surfaceInfos.GetAdhesionGroup(m_LastMaterialToHaveBeenStandingOn))
			{
				case ADHESION_GROUP_LOOSE:
					if( (CGeneral::GetRandomNumber() &127)==0 )
						NULL;//SetFall(0, ANIM_STD_HIGHIMPACT_BACK, false);
					else
						m_extractedVelocity *= 0.5f;

					fVolume = 0.5f;
					break;
				case ADHESION_GROUP_SAND:
					if( (CGeneral::GetRandomNumber() &63)==0 )
						NULL;//SetFall(0, ANIM_STD_HIGHIMPACT_BACK, false);
					else
						m_extractedVelocity *= 0.2f;
					
					fVolume = 0.2f;
					break;
				case ADHESION_GROUP_WET:
					m_extractedVelocity *= 0.3f;
					fVolume = 0.2f;
					break;
				case ADHESION_GROUP_RUBBER:
				case ADHESION_GROUP_HARD:
				case ADHESION_GROUP_ROAD:
				default:
					fVolume = 1.0f;
					break;
			}
			
		
			if(fVolume > 0.2f && pWalk->GetCurrentTime()>leftFootDown && pWalk->GetCurrentTime() - pWalk->GetTimeStep() <= leftFootDown)
			{
#ifdef USE_AUDIOENGINE
				if (m_PedAudioEntity.IsInitialised())
				{
					float fFrequencyScaling=1.0f;

					if(pWalk->GetAnimId() == ANIM_STD_WALK)
						fFrequencyScaling = 0.75f;

					m_PedAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_SKATE_LEFT, 20.0f *
						CMaths::Log10(fVolume), fFrequencyScaling);
				}
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_SKATE, (pWalk->GetAnimId() << 8) | ((UInt8)(fVolume * 127.0f)));
#endif //USE_AUDIOENGINE
			}
			else if(fVolume > 0.2f && pWalk->GetCurrentTime()>rightFootDown && pWalk->GetCurrentTime() - pWalk->GetTimeStep() <= rightFootDown)
			{
#ifdef USE_AUDIOENGINE
				if (m_PedAudioEntity.IsInitialised())
				{
					float fFrequencyScaling=1.0f;

					if(pWalk->GetAnimId() == ANIM_STD_WALK)
						fFrequencyScaling = 0.75f;

					m_PedAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_SKATE_RIGHT, 20.0f *
						CMaths::Log10(fVolume), fFrequencyScaling);
				}
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_SKATE, (pWalk->GetAnimId() << 8) | ((UInt8)(fVolume * 127.0f)));
#endif //USE_AUDIOENGINE
			}
		}
		//
		// footstep left:
		//
//		else if((pWalk->GetCurrentTime() >= PEDANIM_FOOTDOWNFRAME &&
//			pWalk->GetCurrentTime() - pWalk->GetTimeStep() < PEDANIM_FOOTDOWNFRAME))
		
		else if (pWalk->GetCurrentTime() >= leftFootLanded &&
				 pWalk->GetCurrentTime() - pWalk->GetTimeStep() < leftFootLanded)
		{	
			// find the volume that we will use for the sound event:
			if(IsPlayer() && GetPlayerData())  // sound event for left foot
			{
				float vol = 0.0f;  // sound event volume
				bool bDoingRobbery = false;
				if(GetPlayerData()->m_pClothes->GetIsWearingBalaclava())
					bDoingRobbery = true;
				
				switch (m_eMoveState)
				{
					case PEDMOVE_SPRINT :
						vol = SOUNDVOL_HEAVY_FOOTSTEPS;
						if(bDoingRobbery)	vol += 10.0f;
						break;
					case PEDMOVE_RUN:
					case PEDMOVE_JOG:
						if(GetPlayerData()->m_moveBlendRatio < 2.0f)
						{
							if(bDoingRobbery && GetPlayerData()->m_moveBlendRatio > 1.1f)
								vol = SOUNDVOL_BARELY_AUDIBLE + 20.0f*(GetPlayerData()->m_moveBlendRatio - 1.0f);
							else if(GetPlayerData()->m_moveBlendRatio > 1.5f)
								vol = SOUNDVOL_BARELY_AUDIBLE + 15.0f*(GetPlayerData()->m_moveBlendRatio - 1.0f);
							else
								vol = 0.0f;
						}
						else
						{
							if(bDoingRobbery)
								vol = SOUNDVOL_HEAVY_FOOTSTEPS;
							else
								vol = SOUNDVOL_NORM_FOOTSTEPS;
						}
						break;
					case PEDMOVE_WALK :
					default:
						vol = 0.0f;
						break;
				}

				if(vol > 0.0f)
				{						
					CEventSoundQuiet eventFootstepNoise (this, vol);
					GetEventGlobalGroup()->Add(eventFootstepNoise);
				}
			}

#ifdef USE_AUDIOENGINE
			float fVolumeOffsetdB=0.0f, fRelativeFrequency=1.0f;

			if(m_nPedFlags.bIsDucking)
			{
				fVolumeOffsetdB = -18.0f;
				fRelativeFrequency = 0.8f;
			}
			else
			{
				switch(m_eMoveState)
				{
					case PEDMOVE_RUN:
						fVolumeOffsetdB = -6.0f;
						fRelativeFrequency = 1.1f;
						break;
					case PEDMOVE_SPRINT:
						fRelativeFrequency = 1.2f;
						break;
					default:
						fVolumeOffsetdB = -12.0f;
						fRelativeFrequency = 0.9f;
						break;
				}

				if(m_motionAnimGroup == ANIM_PLAYER_SNEAK_PED)
				{
					fVolumeOffsetdB -= 6.0f;
					fRelativeFrequency -= 0.1f;
				}
			}

			if (m_PedAudioEntity.IsInitialised())
			{
				m_PedAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_FOOTSTEP_LEFT, fVolumeOffsetdB,
					fRelativeFrequency);
			}
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_PED_FOOTSTEP_LEFT, 1.0f);
#endif //USE_AUDIOENGINE

			// now do bloody flootprints or footsplashes if required
			//if(	(m_nPedFlags.bDoBloodyFootprints) || 
			//	(CWeather::Rain > 0.1f) )
			{
				DoFootLanded(true, doWobble);
				
			}//if(	(m_nPedFlags.bDoBloodyFootprints) || (CWeather::Rain > 0.1f) )...

		}//if((pWalk->GetCurrentTime() >= PEDANIM_FOOTDOWNFRAME && pWalk->GetCurrentTime() - pWalk->GetTimeStep() < PEDANIM_FOOTDOWNFRAME))...
		//
		// footstep right:
		//
//		else if((pWalk->GetCurrentTime() >= rightFootDown &&
//			pWalk->GetCurrentTime() - pWalk->GetTimeStep() < rightFootDown))

		else if (pWalk->GetCurrentTime() >= rightFootLanded &&
				 pWalk->GetCurrentTime() - pWalk->GetTimeStep() < rightFootLanded)
		{	
#ifdef USE_AUDIOENGINE
			float fVolumeOffsetdB=0.0f, fRelativeFrequency=1.0f;

			if(m_nPedFlags.bIsDucking)
			{
				fVolumeOffsetdB = -18.0f;
				fRelativeFrequency = 0.8f;
			}
			else
			{
				switch(m_eMoveState)
				{
					case PEDMOVE_RUN:
						fVolumeOffsetdB = -6.0f;
						fRelativeFrequency = 1.1f;
						break;
					case PEDMOVE_SPRINT:
						fRelativeFrequency = 1.2f;
						break;
					default:
						fVolumeOffsetdB = -12.0f;
						fRelativeFrequency = 0.9f;
						break;
				}

				if(m_motionAnimGroup == ANIM_PLAYER_SNEAK_PED)
				{
					fVolumeOffsetdB -= 6.0f;
					fRelativeFrequency -= 0.1f;
				}
			}

			if (m_PedAudioEntity.IsInitialised())
			{
				m_PedAudioEntity.AddAudioEvent(AUDIO_EVENT_PED_FOOTSTEP_RIGHT, fVolumeOffsetdB,
					fRelativeFrequency);
			}
#else //USE_AUDIOENGINE
			DMAudio.PlayOneShot(AudioHandle, AE_PED_FOOTSTEP_RIGHT, 1.0f);
#endif //USE_AUDIOENGINE
	
			DoFootLanded(false, doWobble);
			
		}//if((pWalk->GetCurrentTime() >= rightFootDown && pWalk->GetCurrentTime() - pWalk->GetTimeStep() < rightFootDown))...
		
		
	}//if(pWalk && partialBlend < 1.0f)...


	if (m_nPedFlags.bIsLanding)
	{
		CTask* taskPtr;
		taskPtr = GetPedIntelligence()->GetTaskActiveSimplest();
		if (taskPtr->GetTaskType() == CTaskTypes::TASK_SIMPLE_LAND)
		{
			if (((CTaskSimpleLand*)taskPtr)->RightFootLanded())
			{
				DoFootLanded(false, true);		
			}	
			else if (((CTaskSimpleLand*)taskPtr)->LeftFootLanded())
			{
			
				DoFootLanded(true, true);	
			}
		}
	}
/*
	//
	// add footsteps/bloodprints when Tommy is just about to land (and he was not walking):
	//
	if((this->IsPlayer()) && (!pWalk) && (m_nPedFlags.bIsLanding))
	{
		//
		// left foot:
		//
		{
			CVector vec;
			GET_CPED_FOOT_LEFT_POSITION(this, vec);

			const CVector vecFwd(m_mat.GetForward());
			const CVector vecRgt(m_mat.GetRight());
			vec.z -= 0.1f;
			vec += 0.20f*vecFwd;


			// bloody footprints:
			if(m_nPedFlags.bDoBloodyFootprints)
			{
				CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpBloodPoolTex, &vec, 0.26f*vecFwd.x, 0.26f*vecFwd.y,
							0.14f*vecRgt.x, 0.14f*vecRgt.y, 255, 255, 0, 0, PED_SHADOW_DEPTH, 3000);
				if( m_nTimeOfDeath > 20)
					m_nTimeOfDeath -= 20;
				else
				{
					m_nTimeOfDeath = 0;
					m_nPedFlags.bDoBloodyFootprints = false;
				}
			}


			// footpaths on beaches:
			if((!bThisPedIsSkater) && (CColPoint::LeaveFootSteps(m_LastMaterialToHaveBeenStandingOn)))
			//if(m_LastMaterialToHaveBeenStandingOn==COLPOINT_SURFACETYPE_DEFAULT)
			{
				CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpShadowPedTex, &vec, -0.26f*vecFwd.x, -0.26f*vecFwd.y,
							0.14f*vecRgt.x, 0.14f*vecRgt.y, 120, 250, 250, 50, PED_SHADOW_DEPTH, 5000);
			}
		
			// footsplashes:
			if( (CWeather::Rain>0.1f) && (!CCullZones::CamNoRain()) && (!CCullZones::PlayerNoRain()) )
			{
				//
				// it's raining - produce foot splashes:
				//
				particleProduceFootSplash(this, vec);
			}
			else
			{
				//
				// it's not raining - produce foot dust (only for the player):
				//
				if(this->IsPlayer())
					particleProduceFootDust(this, vec);
			}

			m_wobble = 2*PI;	
			m_wobbleSpeed = 1.0f + 50.0f*(m_vecCurrentVelocity.x*m_vecCurrentVelocity.x + m_vecCurrentVelocity.y*m_vecCurrentVelocity.y);	
		
		}


		//
		// right foot:
		//
		{
			CVector vec;
			GET_CPED_FOOT_RIGHT_POSITION(this, vec);

			const CVector vecFwd(m_mat.GetForward());
			const CVector vecRgt(m_mat.GetRight());
			vec.z -= 0.1f;
			vec += 0.20f*vecFwd;


			// bloody footprints:
			if(m_nPedFlags.bDoBloodyFootprints)
			{
				CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpBloodPoolTex, &vec, 0.26f*vecFwd.x, 0.26f*vecFwd.y,
							0.10f*vecRgt.x, 0.10f*vecRgt.y, 255, 255, 0, 0, PED_SHADOW_DEPTH, 3000);
				if( m_nTimeOfDeath > 20)
					m_nTimeOfDeath -= 20;
				else
				{
					m_nTimeOfDeath = 0;
					m_nPedFlags.bDoBloodyFootprints = false;
				}
			}


			// footpaths on beaches:
			if((!bThisPedIsSkater) && (CColPoint::LeaveFootSteps(m_LastMaterialToHaveBeenStandingOn)))
			//if(m_LastMaterialToHaveBeenStandingOn==COLPOINT_SURFACETYPE_DEFAULT)
			{
				CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpShadowPedTex, &vec, -0.26f*vecFwd.x, -0.26f*vecFwd.y,
							0.14f*vecRgt.x, 0.14f*vecRgt.y, 120, 250, 250, 50, PED_SHADOW_DEPTH, 5000);
			}
			
			
			// footsplashes:
			if( (CWeather::Rain>0.1f) && (!CCullZones::CamNoRain()) && (!CCullZones::PlayerNoRain()) )
			{
				//
				// it's raining - produce foot splashes:
				//
				particleProduceFootSplash(this, vec);
			}
			else
			{
				//
				// it's not raining - produce foot dust (only for the player):
				//
				if(this->IsPlayer())
					particleProduceFootDust(this, vec);
			}

			m_wobble = 2*PI;	
			m_wobbleSpeed = 1.0f + 50.0f*(m_vecCurrentVelocity.x*m_vecCurrentVelocity.x + m_vecCurrentVelocity.y*m_vecCurrentVelocity.y);	
		
		}
	
	}//if(m_nPedFlags.bIsLanding)...
*/



	//
	// ped splashes produced from puddles, pools, etc.:
	//
	

	// MN - FX SYSTEM -------------------------------
	// TODO: splash from ped walking thru puddle		
	// ----------------------------------------------	
					
/*					
#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (AddParticle)
// ped splashes from puddles etc

	if(CColPoint::IsWater(m_LastMaterialToHaveBeenStandingOn))
	{
//		const RwRGBA rgbaSplashWaterColor = {155, 155, 185, 128};
		const RwRGBA rgbaSplashWaterColor2 = {255, 255, 255, 196};
		
		const float fSpeed = m_vecMoveSpeed.x*m_vecMoveSpeed.x + m_vecMoveSpeed.y*m_vecMoveSpeed.y;
		float radius = 0.0f;
		
		
		if(fSpeed > 0.03f*0.03f)
		{
			// do some splash circles behind the ped
//			if( !(CTimer::m_FrameCounter&3) )
//				CParticle::AddParticle(PARTICLE_RAIN_SPLASH, GetPosition() - 0.2*m_mat.GetUp() - ((CGeneral::GetRandomNumber()&7 - 4)*0.05f)*m_mat.GetRight(), CVector(0,0,0), NULL, 0.2f, rgbaSplashWaterColor);
			// do a kind of bow wave of splashes in front of the ped
			if(	!(CTimer::m_FrameCounter&1)	&& (fSpeed > 0.13f*0.13f))
			{
				radius = CMaths::Sqrt(fSpeed) * 2.0;
				if (radius < 0.25f) radius = 0.25f;
				if (radius > 0.75f) radius = 0.75f;

				//CVector posSplash	(this->GetPosition() - 0.3f*m_mat.GetUp() + 0.3f*m_mat.GetForward());
				CVector	posSplash	(this->GetPosition() + 0.3f*this->m_mat.GetForward());
				posSplash.z -= 1.2f;

				CVector	speedSplash	(-0.75f*this->m_vecMoveSpeed);
				//speedSplash.z = CGeneral::GetRandomNumberInRange(0.03f, 0.05f);
				speedSplash.z = CGeneral::GetRandomNumberInRange(0.01f, 0.03f);

				//CParticle::AddParticle(PARTICLE_CAR_SPLASH,  posSplash, speedSplash, NULL, 0.5f, rgbaSplashWaterColor);
				//CParticle::AddParticle(PARTICLE_PED_SPLASH,  posSplash, speedSplash, NULL, 0, rgbaSplashWaterColor);
				CParticle::AddParticle(PARTICLE_CAR_SPLASH,  posSplash, speedSplash, NULL, radius * 0.5f, CRGBA(0,0,0,0));
				//speedSplash.z = CGeneral::GetRandomNumberInRange(0.03f, 0.05f);
				CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,  posSplash, speedSplash, NULL, radius, rgbaSplashWaterColor2);
			}
		}
		if ((this->GetPedState() == PED_JUMP))
		{
			CVector vecSplashPos( this->GetPosition());
			vecSplashPos.z -= 0.1f;
			CVector vecSplashVel(0.0f, 0.0f, 0.075f);
			CParticle::AddParticle(PARTICLE_CAR_SPLASH,  vecSplashPos, vecSplashVel, NULL, 0.005f, CRGBA(0,0,0,0));
			vecSplashVel.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
			vecSplashVel.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
			vecSplashVel.z -= CGeneral::GetRandomNumberInRange(0.025f, 0.05f);
			CParticle::AddParticle(PARTICLE_RUBBER_SMOKE,  vecSplashPos, vecSplashVel, NULL, 0.50f, rgbaSplashWaterColor2);
		}
	}//if(m_LastMaterialToHaveBeenStandingOn==COLPOINT_SURFACETYPE_SHALLOW_WATER)...

#endif // MN - OLD FX SYSTEM (AddParticle)
*/
}// end of CPed::PlayFootSteps()...





//
// GetLocalDirection: Returns the local direction. If vector is coming from the back, front,left,right
//			etc(0 = front, 1 = left, 2 = back, 3 = right
//
int32 CPed::GetLocalDirection(const CVector2D& dir)
{
	float theta = CMaths::ATan2(-dir.x, dir.y);
	//if(dir.x > 0.0f)
	//	theta = -theta;
		
	float angle = theta - m_fCurrentHeading;
	
	angle += QUARTER_PI;
	
	while(angle < 0)
		angle += TWO_PI;
		
	int32 quadrant = (angle / HALF_PI);
	
	while(quadrant > 3)
		quadrant -= 4;
		
	return quadrant;
}


//#define PED_NOMINAL_RADIUS (0.35f)
//#define PED_CORNER_CHECK (0.65f)

/*
enum { BLOCKED_BY_NONE = 0, BLOCKED_BY_BUILDING, BLOCKED_BY_CAR, BLOCKED_BY_OBJECT };

void CPed::SetDirectionToWalkAroundVehicle(CVehicle* pVehicle)
{
    int iPathFollowTimer;
    if(PEDMOVE_WALK==m_eMoveState)
    {
        iPathFollowTimer=2000.0f;
    }
    else
    {
        iPathFollowTimer=250.0f;
    }
    
    CEntity* pTargetEntity=m_pTargetEntity;
    
    //If the ped is trying to get to another ped then set the target to be 
    //the other ped.
    if((KILL_CHAR_ANY_MEANS==m_Objective)||
       (KILL_CHAR_ON_FOOT==m_Objective)||
       (MUG_CHAR==m_Objective)||
       (GOTO_CHAR_ON_FOOT==m_Objective)||
       (FLEE_CHAR_ON_FOOT_TILL_SAFE)||
       (FLEE_CHAR_ON_FOOT_ALWAYS==m_Objective)||
       (FOLLOW_CHAR_IN_FORMATION==m_Objective)||
       (GUARD_ATTACK==m_Objective))
    {
        pTargetEntity=m_pObjectivePed;   
    }
	
	m_pPedIntelligence->AddEvent(new CEventVehicleCollision(m_nDamagedPieceType,m_fDamageImpulseMagnitude,pVehicle,m_vecDamageNormal,m_vecDamagePos));
}
*/

/*
void CPed::SetDirectionToWalkAroundVehicle(CVehicle* pVehicle)
{
    //If there is a path to follow then don't compute a new one.
    if(m_nNumPathNodes)
    {
        return;
    }
    
    //Get the centre and radius of the vehicle's bounding circle.
    const float fPedRadius=0.7f;
    const CVector& vVehiclePos=pVehicle->GetPosition();
    const CVector vMin=
        pVehicle->m_mat*
            (CVector(-fPedRadius,-fPedRadius,0) + pVehicle->GetColModel().GetBoundBoxMin());
    const CVector vMax=
        pVehicle->m_mat*
            (CVector(fPedRadius,fPedRadius,0) + pVehicle->GetColModel().GetBoundBoxMax());
    const float fVehicleRadius=MAX((vMax-vVehiclePos).Magnitude(),(vMin-vVehiclePos).Magnitude());
    
    //Find a position on the vehicle's bounding box and on the 
    //line from ped to target that allows a clear path to the target.
    
    CVector pos[4]=
        {CVector(vMin.x,vMin.y,0),
         CVector(vMax.x,vMin.y,0),
         CVector(vMax.x,vMax.y,0),
         CVector(vMin.x,vMax.y,0)};
    CVector dirs[4]=
        {pVehicle->m_mat.GetForward()*-1,
         pVehicle->m_mat.GetRight()*-1, 
         pVehicle->m_mat.GetForward(),
         pVehicle->m_mat.GetRight()};
    float d[4]=
        {-DotProduct(pos[0],dirs[0]),
         -DotProduct(pos[1],dirs[1]),
         -DotProduct(pos[2],dirs[2]),
         -DotProduct(pos[3],dirs[3])};
    
    CVector v1=m_vecTargetPosition;
    CVector v2=GetPosition();
    CVector w=v2-v1;
    w.Normalise();
    bool b1=false;
    bool b2=false;
    
    int i;
    for(i=0;i<4;i++)
    {
        const float fSide1=DotProduct(dirs[i],v1)+d[i];
        const float fSide2=DotProduct(dirs[i],v2)+d[i];
        
        float fTest;
        if((fSide1>0)&&(fSide2<0))
        {
            const float t=-(d[i]+DotProduct(dirs[i],v1))/DotProduct(dirs[i],w);
            v1=v1+w*t;
            b1=true;
        }
        if((fSide1<0)&&(fSide2>0))
        {
            const float t=-(d[i]+DotProduct(dirs[i],v2))/DotProduct(dirs[i],w);
            v2=v2+w*t;
            b2=true;
        }
    }
    
    //Draw the target point.
    //if(b1&&b2)
    //{
    //    CDebug::RenderDebugSphere3D(v1.x,v1.y,v1.z,0.25f);
    //    CDebug::DebugLine3D(v1.x,v1.y,v1.z,v1.x,v1.y,v1.z+3.0,255,255);
    //}
    
    SetStoredState();
    m_iPathFollowTimer=CTimer::GetTimeInMilliseconds()+1000;
    SetFollowPath(m_vecTargetPosition,2*fVehicleRadius,PEDMOVE_RUN,USE_DYNAMIC_NODES);
}
*/

/*
void CPed::SetDirectionToWalkAroundObject(CEntity* pEntity)
{
	float fOldHeading;
	float fMaxDist = 8.0f;
//	Bool bSeekingCar;
	CVector bBoxVMin = pEntity->GetColModel().GetBoundBoxMin();
	CVector bBoxVMax = pEntity->GetColModel().GetBoundBoxMax();
	CVector bBoxCentre = (bBoxVMin + bBoxVMax) / 2.0f;
	CVector vecToCarCorner;
	CMatrix matBox = pEntity->m_mat;
	float fEntityHeading = pEntity->GetHeading();
	bool bIsEnteringThisCar = false;
	bool bIsUpsideDown = false;
	bool bCarIsVan = false;
	float fCornerCheckRadius = 0.1f*(bBoxVMax.y - bBoxVMin.y);
//	float fCornerCheckRadius = 0.4f*(bBoxVMax.z - bBoxVMin.z);
	CVector vecHeightAdjustedTargetPosition;

	if(GetMoveState() == PEDMOVE_STILL || GetMoveState() == PEDMOVE_NONE)
		return;

//	if(pEntity->GetIsTypeObject() && ((CPhysical*)pEntity)->InfiniteMass)
//	{
		if(CharCreatedBy!=MISSION_CHAR && pEntity->GetModelIndex()==MI_PHONEBOOTH1)
		{
			bool bAlreadyRunning = (GetMoveState()==PEDMOVE_RUN || GetMoveState()==PEDMOVE_SPRINT);
			SetFlee(pEntity, 5000);
			m_nPedFlags.bUseSmartSeekAndFlee = true;
			m_NextWanderNode.SetEmpty();
			if(!bAlreadyRunning)
				SetMoveState(PEDMOVE_WALK);
				
			return;
		}
//		return; // door- ignore
//	}


		// add on radius of the ped collision so ped doesn't end up going in circles
//	if(pEntity->GetIsTypePed()){
		bBoxVMin.x -= PED_NOMINAL_RADIUS;
		bBoxVMin.y -= PED_NOMINAL_RADIUS;
		bBoxVMax.x += PED_NOMINAL_RADIUS;
		bBoxVMax.y += PED_NOMINAL_RADIUS;
//	}

	if(fCornerCheckRadius < 0.5f)
		fCornerCheckRadius = 0.5f;
	if(fCornerCheckRadius > 0.5f*(bBoxVMax.z-bBoxVMin.z))
		fCornerCheckRadius = 0.5f*(bBoxVMax.z-bBoxVMin.z);
	if(fCornerCheckRadius > 0.5f*(bBoxVMax.x-bBoxVMin.x))
		fCornerCheckRadius = 0.5f*(bBoxVMax.x-bBoxVMin.x);
	
	if(matBox.GetUp().z < 0.0f)
		bIsUpsideDown = true;

	if((pEntity->GetModelIndex()==MI_TRAFFICLIGHTS) // special case for traffic lights
		||(pEntity->GetModelIndex()==MI_SINGLESTREETLIGHTS1)
		||(pEntity->GetModelIndex()==MI_SINGLESTREETLIGHTS2))
	{
		fCornerCheckRadius = 0.4f;
		if(matBox.GetUp().z > 0.57f){
			bBoxCentre.x = bBoxVMax.x - 0.25f;// this is close to the point of the base
			bBoxCentre = pEntity->m_mat * bBoxCentre;
			fMaxDist = 0.75f;
		}
		else{
			bBoxVMin.x = 1.2f*MIN(bBoxVMin.x,bBoxVMin.y);
			bBoxVMax.x = 1.2f*MAX(bBoxVMax.x,bBoxVMax.y);
			bBoxVMin.y = 1.2f*bBoxVMin.z;
			bBoxVMax.y = 1.2f*bBoxVMax.z;
			
			fEntityHeading = CMaths::ATan2(-1.0f*matBox.GetUp().x, matBox.GetUp().y);
			matBox.SetUnity();
			matBox.RotateZ(fEntityHeading);
			matBox.Translate(pEntity->GetPosition());
			
			bBoxCentre = pEntity->GetPosition();
		}
		bIsUpsideDown = false;
	}
	else{
		bBoxCentre = pEntity->m_mat * bBoxCentre;
	}
	
	fOldHeading = m_fDesiredHeading;
	bBoxCentre = bBoxCentre - GetPosition();

	// get angle line from ped to bbox centre 
	float theta = CMaths::ATan2(-bBoxCentre.x, bBoxCentre.y);
	float theta2 = CGeneral::LimitRadianAngle(fEntityHeading - theta);
	float cornerAngle = CMaths::ATan2(bBoxVMax.x - bBoxVMin.x, bBoxVMax.y - bBoxVMin.y);
	float fMoveSpeed;
	
	if(IsPlayer())
	{
		if(FindPlayerPed()->m_moveBlendRatio > 0.0f)
			fMoveSpeed = 2.0f/FindPlayerPed()->m_moveBlendRatio;
		else
			fMoveSpeed = 0.0f;
	}
	else
	{
		switch(GetMoveState())
		{
			case PEDMOVE_SPRINT: fMoveSpeed = 0.5f; break;
			case PEDMOVE_RUN: fMoveSpeed = 0.5f; break;
			case PEDMOVE_WALK: fMoveSpeed = 2.0f; break;
			default : fMoveSpeed = 0.0f; break;
		}
	}
	
	if(m_pTargetEntity == pEntity && pEntity->GetIsTypeVehicle())
	{
		if((m_Objective == ENTER_CAR_AS_DRIVER)||(m_Objective == ENTER_CAR_AS_PASSENGER)||(m_Objective == SOLICIT_VEHICLE))
		{
			bIsEnteringThisCar = true; // m_fDesiredHeading remains unchanged
			if(IsPlayer())
				fMoveSpeed=0.0f;
			if(((CVehicle *)pEntity)->m_bIsVan)
				bCarIsVan = true;
		}
	}
	
	// if first time we've collided with this entity or timer has run out
	// want to check 4 corners of bound box to find clear route around obstacle

	uint8 nIsPosBlocked[4];

	for(uint16 i=0;i<4;i++)
		nIsPosBlocked[i] = BLOCKED_BY_NONE;

	if(CTimer::GetTimeInMilliseconds()>m_nCollisionTimer || m_pCollisionEntity!=pEntity)
	{
		CEntity *pHitEntity = NULL;
		CVector vecTemp;
		CVector vecCorners[4];
		bool bDoAvoidCorner = true;

		// don't want to check corners for upside down vehicles cause it'll find lots of blocks when they're clear
		if(pEntity->GetIsTypeVehicle()  && !bIsUpsideDown)
		{
			// front left
			vecTemp.x = bBoxVMin.x+0.7f*fCornerCheckRadius;	vecTemp.y = bBoxVMax.y-0.7f*fCornerCheckRadius;	vecTemp.z = 0.0f;
			vecTemp = matBox * vecTemp;
			vecTemp.z += 0.6f; // the 0.6f increase allows the check not to be set off by steps (like if front of the fuzz ball)
								// but is still ok to detect short walls, and everything higher.
			vecCorners[0] = vecTemp;
	//CDebug::RenderDebugSphere3D(vecTemp.x, vecTemp.y, vecTemp.z, fCornerCheckRadius);
			if((pHitEntity=CWorld::TestSphereAgainstWorld(vecTemp, fCornerCheckRadius, pEntity, true, true, false, true, false))!=NULL){
				if(pHitEntity->GetIsTypeBuilding())
				{
					 	nIsPosBlocked[0] = BLOCKED_BY_BUILDING;
				}
				else if(pHitEntity->GetIsTypeVehicle()) nIsPosBlocked[0] = BLOCKED_BY_CAR;
				else	nIsPosBlocked[0] = BLOCKED_BY_OBJECT;
			}
			// front right
			vecTemp.x = bBoxVMax.x-0.7f*fCornerCheckRadius;	vecTemp.y = bBoxVMax.y-0.7f*fCornerCheckRadius;	vecTemp.z = 0.0f;
			vecTemp = matBox * vecTemp;
			vecTemp.z += 0.6f;
			vecCorners[1] = vecTemp;
	//CDebug::RenderDebugSphere3D(vecTemp.x, vecTemp.y, vecTemp.z, fCornerCheckRadius);
			if((pHitEntity=CWorld::TestSphereAgainstWorld(vecTemp, fCornerCheckRadius, pEntity, true, true, false, true, false))!=NULL){
				if(pHitEntity->GetIsTypeBuilding())
				{
					 nIsPosBlocked[1] = BLOCKED_BY_BUILDING;
				}
				else if(pHitEntity->GetIsTypeVehicle()) nIsPosBlocked[1] = BLOCKED_BY_CAR;
				else	nIsPosBlocked[1] = BLOCKED_BY_OBJECT;
			}
			// rear right
			vecTemp.x = bBoxVMax.x-0.7f*fCornerCheckRadius;	vecTemp.y = bBoxVMin.y+0.7f*fCornerCheckRadius;	vecTemp.z = 0.0f;
			vecTemp = matBox * vecTemp;
			vecTemp.z += 0.6f;
			vecCorners[2] = vecTemp;
	//CDebug::RenderDebugSphere3D(vecTemp.x, vecTemp.y, vecTemp.z, fCornerCheckRadius);
			if((pHitEntity=CWorld::TestSphereAgainstWorld(vecTemp, fCornerCheckRadius, pEntity, true, true, false, true, false))!=NULL){
				if(pHitEntity->GetIsTypeBuilding()) 
				{
					nIsPosBlocked[2] = BLOCKED_BY_BUILDING;
				}
				else if(pHitEntity->GetIsTypeVehicle()) nIsPosBlocked[2] = BLOCKED_BY_CAR;
				else	nIsPosBlocked[2] = BLOCKED_BY_OBJECT;
			}
			// rear left
			vecTemp.x = bBoxVMin.x+0.7f*fCornerCheckRadius;	vecTemp.y = bBoxVMin.y+0.7f*fCornerCheckRadius;	vecTemp.z = 0.0f;
			vecTemp = matBox * vecTemp;
			vecTemp.z += 0.6f;
			vecCorners[3] = vecTemp;
	//CDebug::RenderDebugSphere3D(vecTemp.x, vecTemp.y, vecTemp.z, fCornerCheckRadius);
			if((pHitEntity=CWorld::TestSphereAgainstWorld(vecTemp, fCornerCheckRadius, pEntity, true, true, false, true, false))!=NULL){
				if(pHitEntity->GetIsTypeBuilding()) 
				{
					 nIsPosBlocked[3] = BLOCKED_BY_BUILDING;
				}
				else if(pHitEntity->GetIsTypeVehicle()) nIsPosBlocked[3] = BLOCKED_BY_CAR;
				else	nIsPosBlocked[3] = BLOCKED_BY_OBJECT;
			}
			CColPoint colPoint;
			CEntity* pHitEntity;


			if(!nIsPosBlocked[0] && !nIsPosBlocked[1])
			{
				vecCorners[0].x = bBoxVMin.x-0.3f;	vecCorners[0].y = bBoxVMax.y+0.3f;	vecCorners[0].z = 0.0f;
				vecCorners[0] = matBox * vecCorners[0];

				vecCorners[1].x = bBoxVMax.x+0.3f;	vecCorners[1].y = bBoxVMax.y+0.3f;	vecCorners[1].z = 0.0f;
				vecCorners[1] = matBox * vecCorners[1];

				if(CWorld::ProcessLineOfSight(vecCorners[0], vecCorners[1] , colPoint, pHitEntity, 1,1,0,1,0))
				{
					if(pHitEntity->GetIsTypeVehicle())
					{
						nIsPosBlocked[0] = BLOCKED_BY_CAR;
						nIsPosBlocked[1] = BLOCKED_BY_CAR;
					}
					else
					if(pHitEntity->GetIsTypeBuilding())
					{
						nIsPosBlocked[0] = BLOCKED_BY_BUILDING;
						nIsPosBlocked[1] = BLOCKED_BY_BUILDING;
					}
					else
					if(pHitEntity->GetIsTypeObject())
					{
						nIsPosBlocked[0] = BLOCKED_BY_OBJECT;
						nIsPosBlocked[1] = BLOCKED_BY_OBJECT;
					}
					
				}			
			}

			if(!nIsPosBlocked[2] && !nIsPosBlocked[3])
			{
				vecCorners[2].x = bBoxVMax.x+0.3f;	vecCorners[2].y = bBoxVMin.y-0.3f;	vecCorners[2].z = 0.0f;
				vecCorners[2] = matBox * vecCorners[2];

				vecCorners[3].x = bBoxVMin.x-0.3f;	vecCorners[3].y = bBoxVMin.y-0.3f;	vecCorners[3].z = 0.0f;
				vecCorners[3] = matBox * vecCorners[3];

				if(CWorld::ProcessLineOfSight(vecCorners[2], vecCorners[3] , colPoint, pHitEntity, 1,1,0,1,0))
				{
					if(pHitEntity->GetIsTypeVehicle())
					{
						nIsPosBlocked[2] = BLOCKED_BY_CAR;
						nIsPosBlocked[3] = BLOCKED_BY_CAR;
					}
					else
					if(pHitEntity->GetIsTypeBuilding())
					{
						nIsPosBlocked[2] = BLOCKED_BY_BUILDING;
						nIsPosBlocked[3] = BLOCKED_BY_BUILDING;
					}
					else
					if(pHitEntity->GetIsTypeObject())
					{
						nIsPosBlocked[2] = BLOCKED_BY_OBJECT;
						nIsPosBlocked[3] = BLOCKED_BY_OBJECT;
					}
				}			
			}		
			
		}	// End of don't check if upside down car
		else
			bDoAvoidCorner = false;
		
		if(nIsPosBlocked[0] && nIsPosBlocked[1] && nIsPosBlocked[2] && nIsPosBlocked[3])
		{
			for(uint16 i=0;i<4;i++)
				nIsPosBlocked[i] = BLOCKED_BY_NONE;
			bDoAvoidCorner = false;
		}

		// want to miss out the rest of this if not an upright car or all corners blocked
		if(!bDoAvoidCorner){
			m_nAvoidCorner = AVOID2_NONE;
		}
		// ped at back of car
		else if(ABS(theta2) < cornerAngle){
			if( (bIsEnteringThisCar && (m_nDoor==CAR_DOOR_LF || m_nDoor==CAR_DOOR_LR)) ||
				(CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) > 0.0f) ){
				if(nIsPosBlocked[3]==BLOCKED_BY_BUILDING || (nIsPosBlocked[3] && !nIsPosBlocked[1] && !nIsPosBlocked[2]) )
					m_nAvoidCorner = AVOID2_FR;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
			else{
				if(nIsPosBlocked[2]==BLOCKED_BY_BUILDING || (nIsPosBlocked[2] && !nIsPosBlocked[0] && !nIsPosBlocked[3]) )
					m_nAvoidCorner = AVOID2_FL;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
		}
		// ped at front of car
		else if(ABS(theta2) > PI - cornerAngle){
			if( (bIsEnteringThisCar && (m_nDoor==CAR_DOOR_LF||m_nDoor==CAR_DOOR_LR)) ||
				(CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) < 0.0f) ){
				if(nIsPosBlocked[0]==BLOCKED_BY_BUILDING || (nIsPosBlocked[0] && !nIsPosBlocked[1] && !nIsPosBlocked[2]))
					m_nAvoidCorner = AVOID2_BR;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
			else{
				if(nIsPosBlocked[1]==BLOCKED_BY_BUILDING || (nIsPosBlocked[1] && !nIsPosBlocked[0] && !nIsPosBlocked[3]))
					m_nAvoidCorner = AVOID2_BL;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
		}
		// ped at left of car
		else if( (theta2 > 0.0f && !bIsUpsideDown) || (theta2 >= 0.0f && bIsUpsideDown) ){
			if((bIsEnteringThisCar)&&(m_nDoor==CAR_DOOR_LF||m_nDoor==CAR_DOOR_LR))
				m_nAvoidCorner = AVOID2_NONE;
			else if(CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) > 0.0f){
				if(nIsPosBlocked[0]==BLOCKED_BY_BUILDING || (nIsPosBlocked[0] && !nIsPosBlocked[2] && !nIsPosBlocked[3]))
					m_nAvoidCorner = AVOID2_BR;
				else
				if(nIsPosBlocked[1]==BLOCKED_BY_BUILDING || (nIsPosBlocked[1] && !nIsPosBlocked[2] && !nIsPosBlocked[3]))
					m_nAvoidCorner = AVOID2_BR;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
			else{
				if(nIsPosBlocked[3]==BLOCKED_BY_BUILDING || (nIsPosBlocked[3] && !nIsPosBlocked[0] && !nIsPosBlocked[1]))
					m_nAvoidCorner = AVOID2_FR;
				else
				if(nIsPosBlocked[2]==BLOCKED_BY_BUILDING || (nIsPosBlocked[2] && !nIsPosBlocked[0] && !nIsPosBlocked[1]))
					m_nAvoidCorner = AVOID2_FR;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
		}
		// ped at right of car
		else{
			if((bIsEnteringThisCar)&&((m_nDoor==CAR_DOOR_RF)||(m_nDoor==CAR_DOOR_RR)))
				m_nAvoidCorner = AVOID2_NONE;
			else if(CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) < 0.0f){
				if(nIsPosBlocked[1]==BLOCKED_BY_BUILDING || (nIsPosBlocked[1] && !nIsPosBlocked[2] && !nIsPosBlocked[3]))
					m_nAvoidCorner = AVOID2_BL;
				else
				if(nIsPosBlocked[0]==BLOCKED_BY_BUILDING || (nIsPosBlocked[0] && !nIsPosBlocked[2] && !nIsPosBlocked[3]))
					m_nAvoidCorner = AVOID2_BL;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
			else{
				if(nIsPosBlocked[2]==BLOCKED_BY_BUILDING || (nIsPosBlocked[2] && !nIsPosBlocked[0] && !nIsPosBlocked[1]))
					m_nAvoidCorner = AVOID2_FL;
				else
				if(nIsPosBlocked[3]==BLOCKED_BY_BUILDING || (nIsPosBlocked[3] && !nIsPosBlocked[0] && !nIsPosBlocked[1]))
					m_nAvoidCorner = AVOID2_FL;
				else
					m_nAvoidCorner = AVOID2_NONE;
			}
		}

		m_pCollisionEntity = pEntity;
		m_pCollisionEntity->RegisterReference(&m_pCollisionEntity);
		// set the timer so we don't do this again for a wee while
		m_nCollisionTimer = CTimer::GetTimeInMilliseconds() + 512 + (CGeneral::GetRandomNumber()&255);
	}
	
	// Ped hit back 
	if(ABS(theta2) < cornerAngle)
	{
		if(bIsEnteringThisCar)
		{
			// if car is a van and ped's heading for a rear door -> don't do anything
			if(bCarIsVan && (m_nDoor==CAR_DOOR_LR || m_nDoor==CAR_DOOR_RR))
				return;
			// head for the correct side
			else if((m_nDoor==CAR_DOOR_LF)||(m_nDoor==CAR_DOOR_LR)|| (nIsPosBlocked[2] && !nIsPosBlocked[3]))
			{
				m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + HALF_PI);
				vecToCarCorner.x = bBoxVMin.x;// BL
				vecToCarCorner.y = bBoxVMin.y;
				vecToCarCorner.z = 0;//GetPosition().z;
			}
			else
			{
				m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading - HALF_PI);
				vecToCarCorner.x = bBoxVMax.x;//BR
				vecToCarCorner.y = bBoxVMin.y;
				vecToCarCorner.z = 0;//GetPosition().z;
			}
			
		}
		// if desired heading is greater than angle 
		else if(m_nAvoidCorner==AVOID2_FL || m_nAvoidCorner==AVOID2_BL ||
		(m_nAvoidCorner==AVOID2_NONE && CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) > 0.0f) )
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + HALF_PI);
			vecToCarCorner.x = bBoxVMin.x;// BL
			vecToCarCorner.y = bBoxVMin.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
		else	
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading - HALF_PI);
			vecToCarCorner.x = bBoxVMax.x;//BR
			vecToCarCorner.y = bBoxVMin.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
	}
	// Ped hit front
	else if(ABS(theta2) > PI - cornerAngle)
	{
		// if a Van and peds aiming for a back door, leave direction choice up to standard loop
		if( bIsEnteringThisCar && !(bCarIsVan && (m_nDoor==CAR_DOOR_LR || m_nDoor==CAR_DOOR_RR)) )
		{
			// head for the correct side
			if((m_nDoor==CAR_DOOR_LF)||(m_nDoor==CAR_DOOR_LR) || (nIsPosBlocked[1] && !nIsPosBlocked[0]))
			{
				m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + HALF_PI);
				vecToCarCorner.x = bBoxVMin.x;//TL
				vecToCarCorner.y = bBoxVMax.y;
				vecToCarCorner.z = 0;//GetPosition().z;
			}
			else
			{
				m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading - HALF_PI);
				vecToCarCorner.x = bBoxVMax.x;//TR
				vecToCarCorner.y = bBoxVMax.y;
				vecToCarCorner.z = 0;//GetPosition().z;
			}
			
		}
		// if desired heading is greater than angle 
		else if(m_nAvoidCorner==AVOID2_FR || m_nAvoidCorner==AVOID2_BR ||
		(m_nAvoidCorner==AVOID2_NONE && CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) > 0.0f) )
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading - HALF_PI);
			vecToCarCorner.x = bBoxVMax.x;//TR
			vecToCarCorner.y = bBoxVMax.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
		else	
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + HALF_PI);
			vecToCarCorner.x = bBoxVMin.x;//TL
			vecToCarCorner.y = bBoxVMax.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
	}
	// Ped hit left
	else if( theta2 > 0.0f )
	{
		// if a Van and ped is aiming for a back door, go to back left corner
		if(bIsEnteringThisCar && bCarIsVan && (m_nDoor==CAR_DOOR_LR || m_nDoor==CAR_DOOR_RR))
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + PI);
			vecToCarCorner.x = bBoxVMin.x;// BL
			vecToCarCorner.y = bBoxVMin.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
		else if(bIsEnteringThisCar && (m_nDoor==CAR_DOOR_LF || (m_nDoor==CAR_DOOR_LR && !bCarIsVan))) 
			return;
		// if desired heading is greater than angle 
		else if(m_nAvoidCorner==AVOID2_FL || m_nAvoidCorner==AVOID2_FR ||
		(m_nAvoidCorner==AVOID2_NONE && CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) > 0.0f) )
		{
			m_fDesiredHeading = fEntityHeading;
			vecToCarCorner.x = bBoxVMin.x;//TL
			vecToCarCorner.y = bBoxVMax.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
		else	
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + PI);
			vecToCarCorner.x = bBoxVMin.x;// BL
			vecToCarCorner.y = bBoxVMin.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
	}
	// Ped hit right
	else		
	{
		// if a Van and ped is aiming for a back door, go to back right corner
		if(bIsEnteringThisCar && bCarIsVan && (m_nDoor==CAR_DOOR_RR || m_nDoor==CAR_DOOR_LR))
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + PI);
			vecToCarCorner.x = bBoxVMax.x;//BR
			vecToCarCorner.y = bBoxVMin.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
		else if(bIsEnteringThisCar && (m_nDoor==CAR_DOOR_RF || (m_nDoor==CAR_DOOR_RR && !bCarIsVan)))
			return;
		// if desired heading is greater than angle 
		else if(m_nAvoidCorner==AVOID2_BL || m_nAvoidCorner==AVOID2_BR ||
		(m_nAvoidCorner==AVOID2_NONE && CGeneral::LimitRadianAngle(m_fDesiredHeading - theta) > 0.0f) )
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + PI);
			vecToCarCorner.x = bBoxVMax.x;//BR
			vecToCarCorner.y = bBoxVMin.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
		else
		{
			m_fDesiredHeading = fEntityHeading;
			vecToCarCorner.x = bBoxVMax.x;//TR
			vecToCarCorner.y = bBoxVMax.y;
			vecToCarCorner.z = 0;//GetPosition().z;
		}
	}
	
	if(bIsUpsideDown)
		vecToCarCorner.x *= -1.0f;
	
	vecToCarCorner = matBox*vecToCarCorner;
	m_vecAntiSpazVector.x = vecToCarCorner.x;
	m_vecAntiSpazVector.y = vecToCarCorner.y;
	vecToCarCorner -= GetPosition();

//	if((bBoxVMax.x-bBoxVMin.x > 1.0f && bBoxVMax.x-bBoxVMin.x > 1.0f))
		m_fDesiredHeading = CGeneral::LimitRadianAngle( CMaths::ATan2(-vecToCarCorner.x, vecToCarCorner.y) );
//	else{
//		m_vecAntiSpazVector.x = 0.0f;
//		m_vecAntiSpazVector.y = 0.0f;
//	}

	if((m_fDesiredHeading == m_fCurrentHeading)&&(m_nPedFlags.bHitSomethingLastFrame))
	{ // this means we're not going around this object- try something else

		if(m_fDesiredHeading != fOldHeading) // The desired heading was being changed elsewhere, stick with it
		{
			m_fDesiredHeading = fOldHeading;
		}
		else
		{
			m_fDesiredHeading = CGeneral::LimitRadianAngle(fEntityHeading + PI);
		}
	}


	CVector2D vec2DCarCorner;
	
	vec2DCarCorner = CVector2D::ConvertTo2D(vecToCarCorner);
	
	float fDistToWalkToCorner = vec2DCarCorner.Magnitude();

	if(fDistToWalkToCorner<= 0.5f)
		fDistToWalkToCorner = 0.5f;

	if(fDistToWalkToCorner> fMaxDist)
		fDistToWalkToCorner = fMaxDist;

	m_nAntiSpazTimer = (CTimer::GetTimeInMilliseconds() +(280*fDistToWalkToCorner*fMoveSpeed));
}
*/

//
// IsPedInControl: Returns if ped is control of it own movement
//
bool CPed::IsPedInControl()
{
	if(m_nPedFlags.bIsInTheAir || 
		m_nPedFlags.bIsLanding || 
		!IsAlive() ||
		m_nPedState == PED_ARRESTED)
		return false;

	return true;	
}

bool CPed::IsPedShootable()
{
	if(m_nPedState > PED_STATES_CAN_SHOOT)
		return false;
		
	return true;
}

bool CPed::UseGroundColModel()
{
	if(m_nPedState == PED_FALL || m_nPedState == PED_EVADE_DIVE ||
		m_nPedState == PED_DIE || m_nPedState == PED_DEAD)
		return true;
	
	return false;
}

//
// CanPedReturnToState: Returns if ped can return to current state
//
bool CPed::CanPedReturnToState()
{
	if(m_nPedState >  PED_STATES_NO_AI || 
		m_nPedState == PED_AIMGUN ||
		m_nPedState == PED_ATTACK ||
		m_nPedState == PED_FIGHT ||
		m_nPedState == PED_EVADE_STEP ||
		m_nPedState == PED_SNIPER_MODE ||
		m_nPedState == PED_LOOK_ENTITY)
		return false;
	return true;	
}

//
// CanSetPedState: Returns if ped can change its state
//
bool CPed::CanSetPedState()
{
	if(m_nPedState == PED_DIE ||
		m_nPedState == PED_DEAD ||
		m_nPedState == PED_ARRESTED ||
		m_nPedState == PED_ENTER_CAR ||
		m_nPedState == PED_CARJACK ||
		m_nPedState == PED_STEAL_CAR)
		return false;
	return true;	
}

//
// CanPedReturnToState: Returns if ped can return to current state
//
bool CPed::CanBeArrested()
{
	if(m_nPedState == PED_DIE ||
		m_nPedState == PED_DEAD ||
		m_nPedState == PED_ARRESTED ||
		m_nPedState == PED_ENTER_CAR ||
		m_nPedState == PED_EXIT_CAR)
		return false;
	return true;	
}


bool CPed::CanStrafeOrMouseControl()
{
	if(	m_nPedState==PED_IDLE || m_nPedState==PED_FLEE_ENTITY
	||	m_nPedState==PED_FLEE_POSITION || m_nPedState==PED_NONE
	||	m_nPedState==PED_AIMGUN || m_nPedState==PED_ATTACK
	||	m_nPedState==PED_FIGHT || m_nPedState==PED_JUMP
	||  m_nPedState==PED_ANSWER_MOBILE)
		return true;
	return false;
}


/*
// Name			:	PedSetPreviousStateCB
// Purpose		:	This is called when certain animation finish to revert the ped state back to its
//					original state
// Parameters	:	None
// Returns		:	TRUE if the ped can be deleted or converted
void CPed::PedSetPreviousStateCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed*)pData;
	pPed->RestorePreviousState();
	pPed->m_pAnim = NULL;
}
*/

/*
// Name			:	PedGetupCB
// Purpose		:	This is called once basic getup has finished
// Parameters	:	None
void CPed::PedGetupCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed*)pData;
	// just incase, we don't want to blend out partials if die anim could be there
//	if(!pPed->m_nPedFlags.bIsPedDieAnimPlaying && pPed->m_nPedState!=PED_DIE && pPed->m_nPedState!=PED_DEAD)
	if(pPed->m_nPedState==PED_GETUP)
		RpAnimBlendClumpSetBlendDeltas((RpClump*)(pPed->m_pRwObject), ABA_FLAG_ISPARTIAL, -1000.0f);
//	CAnimManager::BlendAnimation((RpClump*)(pPed->m_pRwObject), pPed->m_motionAnimGroup, ANIM_STD_IDLE, 100.0f);

	//pPed->m_nPedFlags.bIsLyingDown = false;


//#ifdef DEBUG	
	if(	(pAnim->GetAnimId() != ANIM_STD_GET_UP) &&
		(pAnim->GetAnimId() != ANIM_STD_GET_UP_LEFT) &&
		(pAnim->GetAnimId() != ANIM_STD_GET_UP_RIGHT) &&
		(pAnim->GetAnimId() != ANIM_STD_GET_UP_FRONT) &&
		(pAnim->GetAnimId() != ANIM_SUNBATHE_UP) &&
		(pAnim->GetAnimId() != ANIM_SUNBATHE_ESCAPE))
	{
		ASSERT(0);
	}
//#endif	

	// remove getup anim
	pAnim->SetBlendDelta(-1000.0f);

	if(pPed->m_nPedType != PEDTYPE_PLAYER1 &&
		pPed->m_nPedType != PEDTYPE_PLAYER2 &&
		pPed->m_nPedType != PEDTYPE_PLAYER3 &&
		pPed->m_nPedType != PEDTYPE_PLAYER4)
	{
		// play a dust down anim
		//CAnimManager::AddAnimation((RpClump*)pPed->m_pRwObject, ANIM_STD_PED, ANIM_STD_XPRESS_DESPAIR);
	}
	if(pPed->m_nPedState == PED_GETUP)
		pPed->RestorePreviousState();
		
	if((pPed->m_nPedFlags.bFleeWhenStanding)&&(pPed->m_pFleeEntityWhenStanding))
	{
	    pPed->SetFlee(pPed->m_pFleeEntityWhenStanding,10000);
	    pPed->Say(AE_PED_PANIC_SCREAM);
    	pPed->m_nPedFlags.bFleeWhenStanding=false;
	    pPed->m_pFleeEntityWhenStanding=0;
	}
	else if(pPed->m_nPedFlags.bWanderWhenStanding)
	{
	    pPed->SetObjective(NO_OBJ);
	    pPed->SetWanderPath(CGeneral::GetRandomNumberInRange(0,8));
	    pPed->m_nPedFlags.bWanderWhenStanding=false;
	}
	else
	{
    	if(pPed->m_nPedState == PED_FLEE_POSITION || pPed->m_nPedState == PED_FLEE_ENTITY)
    	{
    		pPed->SetMoveState(PEDMOVE_RUN);
    	}
    	else
    	{
    		pPed->SetMoveState(PEDMOVE_STILL);
    	}
	    pPed->SetMoveAnim();
	}
	pPed->m_nPedFlags.bGetUpAnimStarted = false;
}
*/

/*
// Name			:	PedLandCB
// Purpose		:	This is called once landing anim has finished
// Parameters	:	None
void CPed::PedLandCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed*)pData;

	// remove landing anim	
	pAnim->SetBlendDelta(-1000.0f);
	pPed->m_nPedFlags.bIsLanding = 0;
	
	if(pPed->m_nPedState == PED_JUMP)
		pPed->RestorePreviousState();
}
*/

/*
// Name			:	PedStaggerCB
// Purpose		:	This is called once stagger anim has finished
// Parameters	:	None
void CPed::PedStaggerCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed*)pData;
	if(pPed->m_nPedState == PED_STAGGER)
		RestorePedState();
}
*/


/*
// Name			:	PedSetDraggedOutCarCB
// Purpose		:	This is called once a ped has been dragged out of a car and needs his position updated
// Parameters	:	None
void CPed::PedSetDraggedOutCarCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed*)pData;
	eDoors DoorId;
	bool bQuickJacked;
	uint8 nFlagForMyDoor = 0;
	CAnimBlendAssociation* pTempAnim = RpAnimBlendClumpGetAssociation((RpClump*)pPed->m_pRwObject, ANIM_STD_QUICKJACKED);

	if(pTempAnim)
		bQuickJacked = true;
	else
		bQuickJacked = false;

	if( pAnim && pAnim->GetAnimId() == ANIM_BIKE_HIT && pPed->m_pMyVehicle)
	{
		if(pPed->m_nDoor == CAR_DOOR_LF || pPed->m_nDoor == CAR_DOOR_RF){
			CAnimManager::BlendAnimation((RpClump*)(pPed->m_pRwObject), ANIM_STD_PED, ANIM_STD_BIKE_FALLOFF, 100.0f);
			nFlagForMyDoor = 5;
		}
		else{
			CAnimManager::BlendAnimation((RpClump*)(pPed->m_pRwObject), ANIM_STD_PED, ANIM_STD_BIKE_FALLBACK, 100.0f);
			nFlagForMyDoor = 10;
		}
		
		pPed->m_pMyVehicle->m_nGettingOutFlags &= ~(nFlagForMyDoor);
		((CBike *)(pPed->m_pMyVehicle))->KnockOffRider(WEAPONTYPE_UNIDENTIFIED, 0, pPed, true);
		return;
	}

	if(pPed->m_nPedState != PED_ARRESTED)
	{
		pPed->m_nPedStoredState = PED_NONE;
		if(pAnim)
			pAnim->SetBlendDelta(-1000.0f);
	}	
	
	pPed->RestartNonPartialAnims();
//	ASSERT(pPed->m_pMyVehicle);	m_pMyVehicle not always set on client in networkgame
	pPed->m_pAnim = NULL;
	pPed->m_pTargetEntity = NULL;
	if(pPed->m_pMyVehicle && pPed->m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
	switch(pPed->m_nDoor)
	{
		case CAR_WINDSCREEN:
		case CAR_DOOR_LF:nFlagForMyDoor = 5;DoorId = FRONT_LEFT_DOOR;break;
		case CAR_DOOR_LR:nFlagForMyDoor = 10;DoorId = REAR_LEFT_DOOR;break;
		case CAR_DOOR_RF:nFlagForMyDoor = 5;DoorId = FRONT_RIGHT_DOOR;break;
		case CAR_DOOR_RR:nFlagForMyDoor = 10;DoorId = REAR_RIGHT_DOOR;break;
		default: ASSERTMSG(1, "Ped->m_nDoor is invalid");
	}
	else	
	switch(pPed->m_nDoor)
	{
		case CAR_DOOR_LF:nFlagForMyDoor = 1;DoorId = FRONT_LEFT_DOOR;break;
		case CAR_DOOR_LR:nFlagForMyDoor = 2;DoorId = REAR_LEFT_DOOR;break;
		case CAR_DOOR_RF:nFlagForMyDoor = 4;DoorId = FRONT_RIGHT_DOOR;break;
		case CAR_DOOR_RR:nFlagForMyDoor = 8;DoorId = REAR_RIGHT_DOOR;break;
		default: ASSERTMSG(1, "Ped->m_nDoor is invalid");
	}
	
	// clear getting out flags
	if (pPed->m_pMyVehicle)
	{
		pPed->m_pMyVehicle->m_nGettingOutFlags &= ~(nFlagForMyDoor);

//sprintf(gString, "m_nGettingOutFlags:%d (flag:%d)", pPed->m_pMyVehicle->m_nGettingOutFlags, nFlagForMyDoor);
//TheConsole.AddLine(gString);
// CDebug::DebugLog(gString);
	}
	
//	if(!pPed->m_pMyVehicle->IsDoorMissing(DoorId))
//		((CAutomobile*)pPed->m_pMyVehicle)->Damage.SetDoorStatus(DoorId, DT_DOOR_SWINGING_FREE);

	if (pPed->m_pMyVehicle)
	{
		if(pPed->m_pMyVehicle->pDriver == pPed) // need this check because there may be a new driver already
		{
			pPed->m_pMyVehicle->RemoveDriver();

			if (pPed->m_pMyVehicle->m_eDoorLockState == CARLOCK_LOCKED_INITIALLY) pPed->m_pMyVehicle->m_eDoorLockState = CARLOCK_UNLOCKED;
			
			if ((pPed->GetPedType()==PEDTYPE_COP) && (pPed->m_pMyVehicle->IsLawEnforcementVehicle()))
			{
				pPed->m_pMyVehicle->ChangeLawEnforcerState(FALSE);
			}
	//		SetRadioStation(pPed);	// Set radio station ped was listening to when left car
		}
		else
		{
			for(int i=0; i< pPed->m_pMyVehicle->m_nMaxPassengers; i++)
			{
				if (pPed->m_pMyVehicle->pPassengers[i]==pPed)
				{
					pPed->m_pMyVehicle->pPassengers[i]=NULL;
					pPed->m_pMyVehicle->m_nNumPassengers--;
				}
			}
		}
	}
		
	pPed->m_nPedFlags.bInVehicle = FALSE;
	// reset infinite mass flag
//	pPed->m_pMyVehicle->InfiniteMass = false;

	if(pPed->IsPlayer())
	{
		AudioManager.PlayerJustLeftCar();
	}
	
	if(pPed->m_Objective==LEAVE_CAR_AND_DIE)
	{
		pAnim->SetDeleteCallback(PedSetDraggedOutCarPositionCB, pPed);

		pPed->m_nHealth = 0.0f;
		pPed->SetDie(ANIM_STD_HIT_FLOOR, 1000.0f, 0.5f);
#ifdef GTA_NETWORK
		if (gGameNet.NetWorkStatus == NETSTAT_SERVER && pPed->IsPlayer())
		{		// A player has died in a network game. We have to act upon it.
			gGameNet.PlayerPedHasBeenKilledInNetworkGame(pPed);
			CNetGames::RegisterPlayerKill(pPed, NULL);				
		}
#endif
		return;
	}
	else if(bQuickJacked)
	{
//		pPed->PedSetQuickDraggedOutCarPositionCB(pAnim, pPed);
		pAnim->SetDeleteCallback(PedSetQuickDraggedOutCarPositionCB, pPed);
	}
	else
	{
//		pPed->PedSetDraggedOutCarPositionCB(pAnim, pPed);
		pAnim->SetDeleteCallback(PedSetDraggedOutCarPositionCB, pPed);
		if(pPed->CanSetPedState())
			CAnimManager::BlendAnimation(((RpClump*)(pPed->m_pRwObject)), ANIM_STD_PED, ANIM_STD_GET_UP, 1000.0f);

	}
	
	pPed->ReplaceWeaponWhenExitingVehicle();
	
	pPed->m_eMoveStateAnim = PEDMOVE_NONE;
	pPed->m_nPedFlags.bWasWarpedIntoCar = false;
//#ifdef GTA_NETWORK
//	if (gGameNet.NetWorkStatus == NETSTAT_SERVER)
//	{
//		if (pPed->m_nPedState == PED_DRAGGED_FROM_CAR) pPed->m_nPedState = PED_IDLE;	// State is still PED_DRAGGED_FROM_CAR here. Put in this fix (Obr)
//		pPed->bControlledByServer = false;
//		pPed->ForceUpdateToClient();
//		if (pPed->m_pMyVehicle && pPed->m_pMyVehicle->bControlledByServer)
//		{
//			pPed->m_pMyVehicle->bControlledByServer = false;
//			pPed->m_pMyVehicle->ForceUpdateToClient();
//		}
//	}
//#endif
}
*/

RwObject* SetAtomicVisibilityCB(RwObject* pObject, void* data)
{
	if(CVisibilityPlugins::GetAtomicId((RpAtomic*)pObject) == (int32)data)
		RpAtomicSetFlags((RpAtomic*)pObject, rpATOMICRENDER);
	else	
		RpAtomicSetFlags((RpAtomic*)pObject, 0);
	return pObject;
}

// Name			:	CanBeDeleted
// Purpose		:	Checks whether the ped can be removed from the map or converted
//						into a dummy ped. Currently, Mission-created peds cannot be
//						removed or converted, but randomly-created peds can be.
// Parameters	:	None
// Returns		:	TRUE if the ped can be deleted or converted
Bool8 CPed::CanBeDeleted(void)
{

	// Now that peds tidy up after themselves they should be ok to be removed even
	// when entring car.
	// IF LOADS OF ASSERTS START TO HAPPEN WE WILL PUT THIS BACK IN.
//	if(m_nPedState == PED_ENTER_CAR || 
//		m_nPedState == PED_STEAL_CAR ||
//		m_nPedState == PED_CARJACK)// ||
//		m_nPedFlags.bInVehicle) // until we have a dummy ped model which is driving, we will not convert to dummy if in vehicle
//		return FALSE;

	if (m_nPedFlags.bInVehicle) return FALSE;

	// Peds in the players' gang should not be deleted.
	if (CPedGroups::ms_groups[FindPlayerPed()->GetPlayerData()->m_PlayerGroup].GetGroupMembership()->IsFollower(this))
	{
		return false;
	}
		
	switch (CharCreatedBy)
	{
		case RANDOM_CHAR :	//	This character was created by the code and can be deleted
			return TRUE;
			break;
		case MISSION_CHAR :	//	Mission created characters should exist for the duration
			return FALSE;	//	of the mission
			break;
		case REPLAY_CHAR :	//	Replay created characters existance is controlled by replay
			return FALSE;	
			break;
		case UNUSED_CHAR :	//	A character should never have this Creator once it is
			ASSERT(0);		// in use by the game
			break;
	}
	
	ASSERT(0);
	return TRUE;
}

// for this version, don't bother checking if the ped's in a vehicle
Bool8 CPed::CanBeDeletedEvenInVehicle(void)
{
	switch (CharCreatedBy)
	{
		case RANDOM_CHAR :	//	This character was created by the code and can be deleted
			return TRUE;
			break;
		case MISSION_CHAR :	//	Mission created characters should exist for the duration
			return FALSE;	//	of the mission
			break;
		case REPLAY_CHAR :	//	Replay created characters existance is controlled by replay
			return FALSE;	
			break;
		case UNUSED_CHAR :	//	A character should never have this Creator once it is
			ASSERT(0);		// in use by the game
			break;
	}
	
	ASSERT(0);
	return TRUE;
}


//
// Remove the ped weapon model
//
/*static RwObject* RemoveWeaponModelCB(RwObject* pObj, void* pData)
{
	ASSERT(RwObjectGetType(pObj) == rpATOMIC);
	if(CVisibilityPlugins::GetAtomicModelInfo((RpAtomic*)pObj) == (CBaseModelInfo*)pData)
	{
		RpClumpRemoveAtomic(RpAtomicGetClump((RpAtomic*)pObj), (RpAtomic*)pObj);
		RpAtomicDestroy((RpAtomic*)pObj);
		return NULL;
	}	
	return pObj;
}

static RwObject *RemoveAllModelCB(RwObject *pObj, void *pData)
{
	ASSERT(RwObjectGetType(pObj) == rpATOMIC);
	if(CVisibilityPlugins::GetAtomicModelInfo((RpAtomic*)pObj) != NULL)
	{
		RpClumpRemoveAtomic(RpAtomicGetClump((RpAtomic*)pObj), (RpAtomic*)pObj);
		RpAtomicDestroy((RpAtomic*)pObj);
	}
	return pObj;
}*/

//
// Add the ped weapon model
//
void CPed::AddWeaponModel(int32 weaponId)
{
	if(weaponId == -1)
		return;

//	if (weaponId == MI_PICKUP_PARACHUTE)
//		return;
	
	if (GetWeapon()->m_bDontPlaceInHand)
		return;


	
	CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(weaponId);

	ASSERT(pModelInfo);
//	ASSERT(pModelInfo->GetModelType() == MI_TYPE_WEAPON);
//	ASSERT(m_pWeaponAtomic == NULL);	//This happens a lot and isn't all that bad. Removed so that testers don't get it all the time.
	// just in case it happens in the final build
	if(m_pWeaponClump)
		RemoveWeaponModel(-1);
	
	m_pWeaponClump = (RpClump*)pModelInfo->CreateInstance();
	ASSERTMSG(m_pWeaponClump, "Weapon model not loaded!");
	if(m_pWeaponClump)
	{
		m_pWeaponFlashFrame = CClumpModelInfo::GetFrameFromName(m_pWeaponClump, "gunflash");
/*
		if(m_pWeaponFlashFrame)
		{
			RpAtomic *pFlashAtomic = NULL;
			RwFrameForAllObjects(m_pWeaponFlashFrame, &GetCurrentAtomicObjectCB, (void *)(&pFlashAtomic));
			if(pFlashAtomic != NULL)
			{
				// need to set render flag (should be off in pickup weapon models)
				uint32 nFlags = RpAtomicGetFlags(pFlashAtomic);
				nFlags |= rpATOMICRENDER;
				RpAtomicSetFlags(pFlashAtomic, nFlags);
			}
		}
*/
	}
	else
		m_pWeaponFlashFrame = NULL;
	
	pModelInfo->AddRef();
	
/*	
	// get skateboard clump ready to be animated
	if(weaponId==MODELID_WEAPON_SKATEBOARD && !RpAnimBlendClumpIsInitialized(m_pWeaponClump))
		RpAnimBlendClumpInit(m_pWeaponClump);
*/
	WeaponModelInHand = weaponId;
	
	if (IsPlayer())
	{
		CWeapon* pWeapon = GetWeapon();
		if (pWeapon->GetWeaponType()==WEAPONTYPE_MOLOTOV && weaponId==MODELID_WEAPON_MOLOTOV)
		{
			if (pWeapon->m_pWeaponFxSys==NULL)
			{
				RpHAnimHierarchy* pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)GetRwObject());
				int32 boneId = RpHAnimIDGetIndex(pHierarchy, BONETAG_R_HAND);
				RwMatrix* pPedHandMat = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + boneId);
				
				CVector offsetPos(0.0f, 0.0f, 0.0f);
			
				pWeapon->m_pWeaponFxSys = g_fxMan.CreateFxSystem("molotov_flame", (RwV3d*)&offsetPos, pPedHandMat, false);
				
				if (pWeapon->m_pWeaponFxSys)
				{
					pWeapon->m_pWeaponFxSys->SetLocalParticles(true);
					pWeapon->m_pWeaponFxSys->CopyParentMatrix();
					pWeapon->m_pWeaponFxSys->Play();
				}			
			}
		}		
	}	
	
	
//////////////////////////////////
// AF: remove this as we can do it by animating a clump
// special hack to let us animate minigun
/*	if(GetPlayerData() && weaponId==MODELID_WEAPON_MINIGUN)
	{
		ASSERT(GetPlayerData()->m_pSpecialAtomic == NULL);
		pModelInfo = CModelInfo::GetModelInfo(MODELID_WEAPON_MINIGUN2);
		ASSERT(pModelInfo);
		GetPlayerData()->m_pSpecialAtomic = (RpAtomic*)pModelInfo->CreateInstance();
	}*/
/////////////////////////////////
}

//
// name:		RemoveWeaponModel
// description: Remove weapon model
void CPed::RemoveWeaponModel(int32 weaponId)
{
	if (IsPlayer())
	{
		CWeapon* pWeapon = GetWeapon();
//		if (pWeapon->GetWeaponType()==WEAPONTYPE_MOLOTOV)
		{
			if (pWeapon->m_pWeaponFxSys)
			{
				g_fxMan.DestroyFxSystem(pWeapon->m_pWeaponFxSys);
				pWeapon->m_pWeaponFxSys = NULL;
			}
		}
	}

	if(m_pWeaponClump)
	{
		if(weaponId == -1 || 
		CVisibilityPlugins::GetClumpModelInfo(m_pWeaponClump) == CModelInfo::GetModelInfo(weaponId))
		{
			CClumpModelInfo* pModelInfo = CVisibilityPlugins::GetClumpModelInfo(m_pWeaponClump);
			pModelInfo->RemoveRef();
			
			if(ISCLUMPSKINNED(m_pWeaponClump))
				RpClumpForAllAtomics(m_pWeaponClump, AtomicRemoveAnimFromSkinCB, NULL);

			RpClumpDestroy(m_pWeaponClump);	
			m_pWeaponClump = NULL;
			m_pWeaponFlashFrame = NULL;
		}
	}
// AF: Remove this as we can do it by animating a clump	
/*	if(GetPlayerData() && (weaponId==-1 || weaponId==MODELID_WEAPON_MINIGUN) && GetPlayerData()->m_pSpecialAtomic)
	{
		RwFrame* pFrame = RpAtomicGetFrame(GetPlayerData()->m_pSpecialAtomic);
		RpAtomicDestroy(GetPlayerData()->m_pSpecialAtomic);
		RwFrameDestroy(pFrame);
		GetPlayerData()->m_pSpecialAtomic = NULL;
	}*/

	WeaponModelInHand = -1;
	m_nGunFlashBlendAmount = 0;
	m_nGunFlashBlendAmount2 = 0;
}

//
// Add the ped goggles model
//
void CPed::AddGogglesModel(int32 weaponId, bool *pbEffectFlag)
{
	if(weaponId == -1)
		return;

	CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(weaponId);

	ASSERT(pModelInfo);

	if(m_pGogglesClump)
		RemoveGogglesModel();
	
	m_pGogglesClump = (RpClump*)pModelInfo->CreateInstance();
	pModelInfo->AddRef();
	
	m_pbGogglesEffect = pbEffectFlag;
	
	// turn the effect on 
	*pbEffectFlag = true;
}

//
// name:		RemoveGogglesModel
// description: Remove goggles model
void CPed::RemoveGogglesModel()
{
	if(m_pGogglesClump)
	{
		CClumpModelInfo* pModelInfo = CVisibilityPlugins::GetClumpModelInfo(m_pGogglesClump);
		pModelInfo->RemoveRef();
			
		if(ISCLUMPSKINNED(m_pGogglesClump))
			RpClumpForAllAtomics(m_pGogglesClump, AtomicRemoveAnimFromSkinCB, NULL);

		RpClumpDestroy(m_pGogglesClump);	
		m_pGogglesClump = NULL;
		
		if (m_pbGogglesEffect)
		{
			*m_pbGogglesEffect = false;
			m_pbGogglesEffect = NULL;
		}
	}
}

void CPed::PutOnGoggles()
{
	CWeapon* pWeapon;
	CWeaponInfo* pWepInfo = CWeaponInfo::GetWeaponInfo(WEAPONTYPE_INFRARED);
	int32 modelId;
			
	pWeapon = &m_WeaponSlots[pWepInfo->m_nWeaponSlot];
	
	if (pWeapon && (pWeapon->GetWeaponType() == WEAPONTYPE_INFRARED || pWeapon->GetWeaponType() == WEAPONTYPE_NIGHTVISION))
	{
		pWepInfo = CWeaponInfo::GetWeaponInfo(pWeapon->GetWeaponType());
		modelId = pWepInfo->GetModelId();
		
		// attach goggles to players head
		if (pWeapon->GetWeaponType() == WEAPONTYPE_INFRARED)
			AddGogglesModel(modelId, &CPostEffects::m_bInfraredVision);
		else	
			AddGogglesModel(modelId, &CPostEffects::m_bNightVision);
			
		// remove from his hand
		pWeapon->m_bDontPlaceInHand = true;
		
		if (pWeapon == GetWeapon())
			RemoveWeaponModel(modelId);
	}
}

void CPed::TakeOffGoggles()
{
	CWeapon* pWeapon;
	CWeaponInfo* pWepInfo = CWeaponInfo::GetWeaponInfo(WEAPONTYPE_INFRARED);
	int32 modelId;
			
	pWeapon = &m_WeaponSlots[pWepInfo->m_nWeaponSlot];
	
	if (pWeapon && (pWeapon->GetWeaponType() == WEAPONTYPE_INFRARED || pWeapon->GetWeaponType() == WEAPONTYPE_NIGHTVISION))
	{
		pWepInfo = CWeaponInfo::GetWeaponInfo(pWeapon->GetWeaponType());
		modelId = pWepInfo->GetModelId();
		
		// remove goggles from player's head
		RemoveGogglesModel();
			
		// put goggles back in his hand
		pWeapon->m_bDontPlaceInHand = false;
		
		if (pWeapon == GetWeapon())
			AddWeaponModel(modelId);
	}
}

//
// name:		RequestDelayedWeapon
// description:	Request the delayed weapon. Give the weapon if it is already in memory
void CPed::RequestDelayedWeapon()
{
	if(m_eDelayedWeapon == WEAPONTYPE_UNIDENTIFIED)
		return;
		
	int32 id = CWeaponInfo::GetWeaponInfo(m_eDelayedWeapon)->GetModelId();
	int32 id2 = CWeaponInfo::GetWeaponInfo(m_eDelayedWeapon)->GetModelId2();
	// request models for weapon
	if(id != -1)
		CStreaming::RequestModel(id, STRFLAG_FORCE_LOAD);
	if(id2 != -1)
		CStreaming::RequestModel(id2, STRFLAG_FORCE_LOAD);
	// if they have loaded give the weapon	
	if((id == -1 || CStreaming::HasModelLoaded(id)) && 
		(id2 == -1 || CStreaming::HasModelLoaded(id2)))
	{
		GiveWeapon(m_eDelayedWeapon, m_delayedAmmo);
		m_eDelayedWeapon = WEAPONTYPE_UNIDENTIFIED;
	}
}

//
// name:		GiveDelayedWeapon
// description:	Request weapon from streaming
void CPed::GiveDelayedWeapon(eWeaponType weaponType, UInt32 ammo)
{
	if (!IsPlayer())
	{
		CTaskSimpleHoldEntity* pTask = GetPedIntelligence()->GetTaskHold();
	
		// make non-player peds drop anything they are holding in their right hand (bottles, etc)		
		if (pTask && pTask->GetEntityBeingHeld() && pTask->GetBoneId() == PED_HANDR)
		{	
			DropEntityThatThisPedIsHolding(true);
		}
	}

		// If we're already waiting on a weapon we don't accept the new request.
	if (m_eDelayedWeapon!=WEAPONTYPE_UNIDENTIFIED) return;

	m_eDelayedWeapon = weaponType;
	m_delayedAmmo = ammo;
	
	RequestDelayedWeapon();
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	GiveWeapon
// DOES:		Gives this ped a certain amount of ammo of a specific
//				type. This will effectively give this player that weapon.
///////////////////////////////////////////////////////////////////////////
int32 CPed::GiveWeapon(eWeaponType weaponType, UInt32 ammoQuantity, bool GenerateOldWeaponPickup)
{

	int32 nSlot = GetWeaponSlot(weaponType);
	
	ASSERT(nSlot >= 0 && nSlot < PED_MAX_WEAPON_SLOTS);
	

	
	
	// already have this weapon
	if(m_WeaponSlots[nSlot].m_eWeaponType == weaponType)
	{
		// only ever one item in gift slot:
		if (nSlot == WEAPONSLOT_TYPE_GIFT) return nSlot;
		
		m_WeaponSlots[nSlot].m_nAmmoTotal += ammoQuantity;
//		if((weaponType<WEAPONTYPE_LAST_WEAPONTYPE)&&(weaponType>WEAPONTYPE_UNARMED)&&(CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType]>=0))
//		{
//			m_WeaponSlots[nSlot].m_nAmmoTotal = MIN(m_WeaponSlots[nSlot].m_nAmmoTotal, CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType]);
//		}
//		else
		{
			m_WeaponSlots[nSlot].m_nAmmoTotal = MIN(m_WeaponSlots[nSlot].m_nAmmoTotal, MAX_AMMO);            
		}		
        m_WeaponSlots[nSlot].Reload(this);
		// clear weapon state if necessary
		if(m_WeaponSlots[nSlot].m_eState == WEAPONSTATE_OUT_OF_AMMO && m_WeaponSlots[nSlot].m_nAmmoTotal > 0)
			m_WeaponSlots[nSlot].m_eState = WEAPONSTATE_READY;
	}
	else
	{
		// different weapon already in this slot
		if(m_WeaponSlots[nSlot].m_eWeaponType != WEAPONTYPE_UNARMED)
		{
/*
			if (GenerateOldWeaponPickup)
			{
				CVector	PickUpCoors, Temp, Temp2, Offset;
				bool	bOnGround;

					// The ammo the player ends up with is the ammo he already had for this slot + the ammo given just now
					// This happens for pistols (3), shotguns(4), sub machine guns(5), rifles (6) 
				if (CWeapon::WeaponGroupSharesAmmo(nSlot))
				{
					ammoQuantity += m_WeaponSlots[nSlot].m_nAmmoTotal;
					m_WeaponSlots[nSlot].m_nAmmoTotal = 0;
				}

				if (m_WeaponSlots[nSlot].m_nAmmoTotal || !CWeapon::WeaponGroupDontCreatePickUpsWithoutAmmo(nSlot))
				{
				// drop the weapon that is already there and generate a pickup for this weapon
					Offset = TheCamera.GetMatrix().GetRight() * 2.6f;
				// Randomize coordinates a little bit so that multiple pickups don't end up at exactly the same coordinates
					Offset.x += 0.35f - 0.05f * (CGeneral::GetRandomNumber()&15);
					Offset.y += 0.35f - 0.05f * (CGeneral::GetRandomNumber()&15);

					if (FindPlayerPed()->m_mat.GetForward().x * Offset.x + FindPlayerPed()->m_mat.GetForward().y * Offset.y > 0.0f)
					{		// Make sure the pickup pops out behind and not in front of the player
						Offset = -Offset;
					}

					PickUpCoors = GetPosition();
					PickUpCoors += Offset;
				
					// Set the z coordinate to be above the ground a bit
					PickUpCoors.z = 1.0f + CWorld::FindGroundZFor3DCoord(PickUpCoors.x, PickUpCoors.y, PickUpCoors.z, &bOnGround);
				
					CColPoint	colPoint;
					CEntity		*pHitEntity;
					if (bOnGround && !CWorld::ProcessLineOfSight(FindPlayerPed()->GetPosition(), PickUpCoors, colPoint, pHitEntity, true, false, false, true, false, false))
					{
						// Generate the appropriate pickup
						CPickups::GenerateNewOne_WeaponType(PickUpCoors, m_WeaponSlots[nSlot].m_eWeaponType, CPickup::PICKUP_ONCE_TIMEOUT_SLOW, m_WeaponSlots[nSlot].m_nAmmoTotal, true);
					}
					else
					{	// Try on the other side
						PickUpCoors = GetPosition();
						PickUpCoors -= Offset;
					
						// Set the z coordinate to be above the ground a bit
						PickUpCoors.z = 1.0f + CWorld::FindGroundZFor3DCoord(PickUpCoors.x, PickUpCoors.y, PickUpCoors.z, &bOnGround);
					
						if (bOnGround)
						{
							// Generate the appropriate pickup
							CPickups::GenerateNewOne_WeaponType(PickUpCoors, m_WeaponSlots[nSlot].m_eWeaponType, CPickup::PICKUP_ONCE_TIMEOUT_SLOW, m_WeaponSlots[nSlot].m_nAmmoTotal, true);
						}
					}
				}
			}
*/

			// If the new weapon shares ammo with the current weapon we add the ammo up
			if (CWeapon::WeaponGroupSharesAmmo(nSlot))
			{
				ammoQuantity += m_WeaponSlots[nSlot].m_nAmmoTotal;
			}

			RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(m_WeaponSlots[nSlot].m_eWeaponType)->GetModelId());
			
			if (GetWeaponSlot(weaponType) == GetWeaponSlot(WEAPONTYPE_INFRARED))
				RemoveGogglesModel();
				
			m_WeaponSlots[nSlot].Shutdown();
		}
//		else // we have picked up an additional weapon
//		{
//			m_nNumWeapons++;
//		}
		
		m_WeaponSlots[nSlot].Initialise(weaponType, ammoQuantity, this);
		
		if(nSlot == m_nCurrentWeapon && !m_nPedFlags.bInVehicle)
		{
			AddWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
		}
		
	}
	
	if (m_WeaponSlots[nSlot].m_eState != WEAPONSTATE_OUT_OF_AMMO) 
	{
		m_WeaponSlots[nSlot].m_eState = WEAPONSTATE_READY;
	}
	
	return nSlot;
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	GetWeaponSlot
// DOES:		Returns the slot the ped has 
///////////////////////////////////////////////////////////////////////////
int32 CPed::GetWeaponSlot(eWeaponType weaponType)
{


	return ((CWeaponInfo::GetWeaponInfo(weaponType))->m_nWeaponSlot);

	
}


///////////////////////////////////////////////////////////////////////////
// FUNCTION:	SetCurrentWeapon
// DOES:		
///////////////////////////////////////////////////////////////////////////

void CPed::SetCurrentWeapon(int weaponSlot)
{
	if(weaponSlot != -1)
	{
		// remove current weapon model
		if(GetWeapon()->GetWeaponType() != WEAPONTYPE_UNARMED)
			RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());

		m_nCurrentWeapon = weaponSlot;

		if(GetPlayerData())
			GetPlayerData()->m_nChosenWeapon = m_nCurrentWeapon;
			
		if(m_WeaponSlots[weaponSlot].m_eWeaponType != WEAPONTYPE_UNARMED)
		{
			// Remove Weapon model and replace with new weapon model
			AddWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
		}	
	}

	
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	SetCurrentWeapon
// DOES:		

///////////////////////////////////////////////////////////////////////////
void CPed::SetCurrentWeapon(eWeaponType weaponType)
{
	int weaponSlot;
	
	weaponSlot = GetWeaponSlot(weaponType);
	
	SetCurrentWeapon(weaponSlot);
	
	
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	GrantAmmo
// DOES:		Gives this player a certain amount of ammo of a specific
//				type. This will effectively give this player that weapon.
///////////////////////////////////////////////////////////////////////////
void CPed::GrantAmmo(eWeaponType weaponType, UInt32 ammoQuantity)
{
	int32 slot = GetWeaponSlot(weaponType);
	// Found the weapon slot with the weapon we were looking for.
	if(slot != -1)
	{
		m_WeaponSlots[slot].m_nAmmoTotal += ammoQuantity;
		// clear weapon state if necessary
//	    if((weaponType<WEAPONTYPE_LAST_WEAPONTYPE)&&(weaponType>WEAPONTYPE_UNARMED)&&(CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType]>=0))
//		{
//			m_WeaponSlots[slot].m_nAmmoTotal = MIN(m_WeaponSlots[slot].m_nAmmoTotal, CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType]);
//		}
//		else
		{
			m_WeaponSlots[slot].m_nAmmoTotal = MIN(m_WeaponSlots[slot].m_nAmmoTotal, MAX_AMMO);            
		}	
		
		if(m_WeaponSlots[slot].m_eState == WEAPONSTATE_OUT_OF_AMMO && m_WeaponSlots[slot].m_nAmmoTotal > 0)
			m_WeaponSlots[slot].m_eState = WEAPONSTATE_READY;
	}

}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	SetAmmo
// DOES:		Gives this player a certain amount of ammo of a specific
//				type. This will effectively give this player that weapon.
//				Take the weapon away by setting the ammo to zero.
///////////////////////////////////////////////////////////////////////////
void CPed::SetAmmo(eWeaponType weaponType, UInt32 ammoQuantity)
{
	int32 slot = GetWeaponSlot(weaponType);
	// Found the weapon slot with the weapon we were looking for.
	if(slot != -1)
	{
		m_WeaponSlots[slot].m_nAmmoTotal = ammoQuantity;
//		if((weaponType<WEAPONTYPE_LAST_WEAPONTYPE)&&(weaponType>WEAPONTYPE_UNARMED)&&(CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType]>=0))
//		{
//			m_WeaponSlots[slot].m_nAmmoTotal = MIN(m_WeaponSlots[slot].m_nAmmoTotal, CWeaponInfo::ms_aMaxAmmoForWeapon[weaponType]);
//		}
//		else
		{
			m_WeaponSlots[slot].m_nAmmoTotal = MIN(m_WeaponSlots[slot].m_nAmmoTotal, MAX_AMMO);            
		}
        // Make sure we don't have more ammo in clip than in total	
		m_WeaponSlots[slot].m_nAmmoInClip = MIN(m_WeaponSlots[slot].m_nAmmoTotal, m_WeaponSlots[slot].m_nAmmoInClip);
		
		// clear weapon state if necessary
		if(m_WeaponSlots[slot].m_eState == WEAPONSTATE_OUT_OF_AMMO && m_WeaponSlots[slot].m_nAmmoTotal > 0)
			m_WeaponSlots[slot].m_eState = WEAPONSTATE_READY;
	}

}

void CPed::ClearWeapon(eWeaponType weaponType)
{
	int32 slot = GetWeaponSlot(weaponType);
	
	if(slot != -1)
	{	// Found the weapon slot that this type of weapon should be in...
		if (m_WeaponSlots[slot].GetWeaponType() == weaponType)
		{	//	...	and the current weapon for this slot is the correct weapon type
			if (m_nCurrentWeapon == slot)
			{	//	Have to clear this weapon if the ped is holding it
				SetCurrentWeapon(WEAPONTYPE_UNARMED);
			}
			
			m_WeaponSlots[slot].Shutdown();

			if (weaponType == WEAPONTYPE_NIGHTVISION || weaponType == WEAPONTYPE_INFRARED)
				RemoveGogglesModel();
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	ClearWeapons
// DOES:		Clear all of the weapons (on death and arrest) 
///////////////////////////////////////////////////////////////////////////
void CPed::ClearWeapons()
{
	Int16	i = 0;	

	// Find the weapon slot of this specific type.
	RemoveWeaponModel(-1);
	RemoveGogglesModel();
	
	for(i=0; i < PED_MAX_WEAPON_SLOTS; i++)
	{
		m_WeaponSlots[i].Shutdown();
	}
	SetCurrentWeapon(WEAPONTYPE_UNARMED);//m_nCurrentWeapon = 0;
}

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	DoWeHaveWeaponAvailable
// DOES:		Work out whether this ped can use this weapon. 
///////////////////////////////////////////////////////////////////////////

bool CPed::DoWeHaveWeaponAvailable(eWeaponType weaponType)
{
	int32 slot = GetWeaponSlot(weaponType);
	
	if(slot != -1)
	{	// Found the weapon slot that this type of weapon should be in...
		if (m_WeaponSlots[slot].GetWeaponType() == weaponType)
		{
			return true;
		}
	}
	return false;
}

//
// name:		RemoveWeaponWhenEnteringVehicle
// description:	Remove players weapon when he is entering a vehicle. Swap in SMG if player has one
//
// nVehicleType lets us have different weapon selection criteria based on the vehicle we're using
// eg. jetpacks can use pistols
//
void CPed::RemoveWeaponWhenEnteringVehicle(int32 nVehicleType)
{
	// disable weapon change after removing weapons for entering a car
	if(GetPlayerData())
		GetPlayerData()->m_bInVehicleDontAllowWeaponChange = true;
	
	if(m_eStoredWeapon!=WEAPONTYPE_UNIDENTIFIED)
	{
		//Already has stored weapon so don't do this again.
		//Could be in the middle of a 
		//HIDE_CHAR_WEAPON_FOR_SCRIPTED_CUTSCENE(true) and
		//HIDE_CHAR_WEAPON_FOR_SCRIPTED_CUTSCENE(false)
		return;
	}
	
	int32 nDoDrivebySlot = WEAPONTYPE_UNIDENTIFIED;
	// if is player and has an SMG (not currently selected)
	if(IsPlayer())
	{
		CPlayerInfo* pPlayer = ( (CPlayerPed *)this)->GetPlayerInfoForThisPlayerPed();
		ASSERTMSG(pPlayer, "CPed::RemoveWeaponWhenEnteringVehicle - Can't find PlayerInfo for Player Ped");
		if(pPlayer->bCanDoDriveBy)
		{
			if((m_WeaponSlots[WEAPONSLOT_TYPE_SMG].m_eWeaponType==WEAPONTYPE_MICRO_UZI
			||  m_WeaponSlots[WEAPONSLOT_TYPE_SMG].m_eWeaponType==WEAPONTYPE_TEC9
			|| (nVehicleType!=1 && m_WeaponSlots[WEAPONSLOT_TYPE_SMG].m_eWeaponType==WEAPONTYPE_MP5))
			&& m_WeaponSlots[WEAPONSLOT_TYPE_SMG].m_nAmmoTotal>0)
			{
				nDoDrivebySlot = WEAPONSLOT_TYPE_SMG;
			}
			// this is to let the player use other 1 handed weapons when flying a jetpack!
			else if(nVehicleType==1 && m_WeaponSlots[WEAPONSLOT_TYPE_SHOTGUN].m_eWeaponType==WEAPONTYPE_SAWNOFF_SHOTGUN
			&& m_WeaponSlots[WEAPONSLOT_TYPE_SHOTGUN].m_nAmmoTotal>0)
			{
				nDoDrivebySlot = WEAPONSLOT_TYPE_HANDGUN;
			}
			else if(nVehicleType==1 && m_WeaponSlots[WEAPONSLOT_TYPE_HANDGUN].m_eWeaponType==WEAPONTYPE_PISTOL
			&& m_WeaponSlots[WEAPONSLOT_TYPE_HANDGUN].m_nAmmoTotal>0)
			{
				nDoDrivebySlot = WEAPONSLOT_TYPE_HANDGUN;
			}
		}
	}
	
	if(nDoDrivebySlot!=WEAPONTYPE_UNIDENTIFIED)
	{
		// save current weapon so it can be resored after player leaves car
		if(m_eStoredWeapon == WEAPONTYPE_UNIDENTIFIED)
			m_eStoredWeapon = GetWeapon()->GetWeaponType();
		// change to Uzi for drivebys
		SetCurrentWeapon(m_WeaponSlots[nDoDrivebySlot].m_eWeaponType);
	}
	else
		RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
}

//
// name:		ReplaceWeaponWhenExitingVehicle
// description:	Give player his weapon back when he is exiting a vehicle. 
void CPed::ReplaceWeaponWhenExitingVehicle()
{
	// disable weapon change after removing weapons for entering a car
	if(GetPlayerData())
		GetPlayerData()->m_bInVehicleDontAllowWeaponChange = false;
	
	// made this more general so it works for a wider range of stored weapons
	if(IsPlayer())
	{
		if(m_eStoredWeapon != WEAPONTYPE_UNIDENTIFIED)
		{
			SetCurrentWeapon(m_eStoredWeapon);
			m_eStoredWeapon = WEAPONTYPE_UNIDENTIFIED;
		}
		else
			AddWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
	}
	else
		AddWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
/*
	// if this wasnt the player or the player didn't have an SMG then add weapon model otherwise change weapon
	if(!(IsPlayer() &&
		(GetWeaponSlot(GetWeapon()->GetWeaponType()) == WEAPONSLOT_TYPE_SMG)))
	{
		AddWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
	}
	else if(IsPlayer() && m_eStoredWeapon != WEAPONTYPE_UNIDENTIFIED)
	{
		SetCurrentWeapon(m_eStoredWeapon);
		m_eStoredWeapon = WEAPONTYPE_UNIDENTIFIED;
	}
*/
}

//
// name:		RemoveWeaponForScriptedCutscene
// description:	Called by the level designers, to remove weapons for scripted cutscenes.
void CPed::RemoveWeaponForScriptedCutscene()
{
	//
	// AF: Do not know why this cares if character is carrying an SMG
	//
	//if (m_WeaponSlots[WEAPONSLOT_TYPE_SMG].m_eWeaponType!=WEAPONTYPE_UNARMED && 
	//	m_WeaponSlots[WEAPONSLOT_TYPE_SMG].m_nAmmoTotal>0)
	{
		m_eStoredWeapon = GetWeapon()->GetWeaponType();
		SetCurrentWeapon(0);
		//RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
	}
}

//
// name:		ReplaceWeaponForScriptedCutscene
// description:	Called by the level designers, to replace weapons for scripted cutscenes. 
void CPed::ReplaceWeaponForScriptedCutscene()
{
	if(m_eStoredWeapon != WEAPONTYPE_UNIDENTIFIED)
	{
		SetCurrentWeapon(m_eStoredWeapon);
		m_eStoredWeapon = WEAPONTYPE_UNIDENTIFIED;
	}
}

bool CPed::DoGunFlash(int32 nDuration, bool bLHand)
{
	if(m_pWeaponClump && m_pWeaponFlashFrame)
	{
		if(bLHand)
		{
			m_nGunFlashBlendAmount2 = m_sGunFlashBlendStart;
			m_nGunFlashBlendOutRate2 = m_sGunFlashBlendStart / nDuration;
		}
		else
		{
			m_nGunFlashBlendAmount = m_sGunFlashBlendStart;
			m_nGunFlashBlendOutRate = m_sGunFlashBlendStart / nDuration;
		}
				
		float fAngle = CGeneral::GetRandomNumberInRange(-360.0f, 360.0f);
		RwMatrix *pFlashMatrix = RwFrameGetMatrix(m_pWeaponFlashFrame);
		RwV3d vPos = *RwMatrixGetPos(pFlashMatrix);
		
//		RwMatrixRotate(pFlashMatrix, pAxis, fAngle, rwCOMBINEREPLACE);
//		RwMatrixTranslate(pFlashMatrix, &vPos, rwCOMBINEPOSTCONCAT);

		RwMatrixRotate(pFlashMatrix, &CPedIK::XaxisIK, fAngle, rwCOMBINEPRECONCAT);
				
		return true;
	}
	
	return false;
}

void CPed::SetGunFlashAlpha(bool bLHand)
{
	int16 nBlend = bLHand ? m_nGunFlashBlendAmount2 : m_nGunFlashBlendAmount;

	if(m_pWeaponFlashFrame && (m_nGunFlashBlendAmount >= 0 || m_nGunFlashBlendAmount2 >= 0))
	{
		RpAtomic *pFlashAtomic = (RpAtomic*)GetFirstObject(m_pWeaponFlashFrame);
		if(pFlashAtomic != NULL)
		{
			static int32 FLASH_BLEND_MULT = 350;
			int32 nAlpha = CMaths::Min(255, FLASH_BLEND_MULT*nBlend/m_sGunFlashBlendStart);
			
			if(nBlend <= 0)
				nAlpha = 0;
			
			CVehicle::SetComponentAtomicAlpha(pFlashAtomic, nAlpha);

//			// need to set render flag (should be off in pickup weapon models)
//			uint32 nFlags = RpAtomicGetFlags(pFlashAtomic);
//			nFlags |= rpATOMICRENDER;
			RpAtomicSetFlags(pFlashAtomic, rpATOMICRENDER);
		}

		if(!bLHand && m_nGunFlashBlendAmount==0)
			m_nGunFlashBlendAmount = -1;
		else if(bLHand && m_nGunFlashBlendAmount2==0)
			m_nGunFlashBlendAmount2 = -1;
	}
}

void CPed::ResetGunFlashAlpha()
{
	if(m_pWeaponFlashFrame)
	{
		RpAtomic *pFlashAtomic = (RpAtomic*)GetFirstObject(m_pWeaponFlashFrame);

		if(pFlashAtomic != NULL)
		{
			// just stop it rendering instead of setting the alpha
			RpAtomicSetFlags((RpAtomic*)pFlashAtomic, 0);
			CVehicle::SetComponentAtomicAlpha(pFlashAtomic, 0);
		}
	}
}


int8 CPed::GetWeaponSkill()
{
	return GetWeaponSkill(GetWeapon()->GetWeaponType());
}


//
#define WEAPON_SKILL_LEVEL_MULT (10)
//
int8 CPed::GetWeaponSkill(eWeaponType weaponType)
{
	if(weaponType >= WEAPONTYPE_FIRST_SKILLWEAPON && weaponType <= WEAPONTYPE_LAST_SKILLWEAPON)
	{
		if(IsPlayer())
		{
			int32 nWeaponStatIndex = CWeaponInfo::GetSkillStatIndex(weaponType);
			ASSERT(nWeaponStatIndex > 0);

			if(CStats::GetStatValue(nWeaponStatIndex) >=  CWeaponInfo::GetRequiredSkillStat(weaponType, WEAPONSKILL_PRO))
				return WEAPONSKILL_PRO;
			else if(CStats::GetStatValue(nWeaponStatIndex) >=  CWeaponInfo::GetRequiredSkillStat(weaponType, WEAPONSKILL_STD))
				return WEAPONSKILL_STD;
			else
				return WEAPONSKILL_POOR;
		}
		else
		{
			if(weaponType==WEAPONTYPE_PISTOL && GetPedType()==PEDTYPE_COP)
				return WEAPONSKILL_SPECIAL;
		
			return m_nWeaponSkill;
		}
	}
	
	return WEAPONSKILL_STD;
}

void CPed::SetWeaponSkill(eWeaponType weaponType, int8 weaponSkill)
{
	if(IsPlayer())
		;	// need to do something with ped stats maybe?
	else
		m_nWeaponSkill = weaponSkill;
/*		
	if(weaponType >= WEAPONTYPE_FIRST_SKILLWEAPON && weaponType <= WEAPONTYPE_LAST_SKILLWEAPON
	&& weaponSkill >= WEAPONSKILL_POOR && weaponSkill <= WEAPONSKILL_PRO)
		m_WeaponSkills[weaponType - WEAPONTYPE_FIRST_SKILLWEAPON] = weaponSkill;
	else
		ASSERT(false);
*/
}

//
// Returns bike riding skill as a float between 0.0 and 1.0
//
const float BIKE_RIDING_SKILL_LIMIT = 1000.0f;
//
float CPed::GetBikeRidingSkill()
{
	float fBikeSkill = 0.0f;
	if(GetPlayerData())
		fBikeSkill = CMaths::Min(1.0f, CStats::GetStatValue(BIKE_SKILL) / BIKE_RIDING_SKILL_LIMIT);
	else if(CharCreatedBy==MISSION_CHAR)
		fBikeSkill = 1.0f;
	
	return fBikeSkill;
}

//
// name:		ShoulderBoneRotation
// description:	Code to rotate shoulder bone about the x-axis half as much as the upper
//				arm. This stops the skinning looking so shit
void CPed::ShoulderBoneRotation(RpClump* pClump)
{
#define PLAYER_SHOULDER
#ifdef PLAYER_SHOULDER		
	RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump(pClump);
	RwMatrix *pMatrix;
	int32 nBone;

	const int32 rotationOrder = EulOrdXYZr;
	static bool bPlayerShoulderRotation = true;
	float rx,ry,rz;
	RwMatrix* pMatrix2;
	
	nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_BREAST);
	pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_UPPERARM);
	pMatrix2 = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	RwMatrixCopy(pMatrix, pMatrix2);
	
	CMatrix mat(pMatrix);

	// Get clavicle matrix
	nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_CLAVICLE);
	pMatrix2 = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	CMatrix pedMat(pMatrix2);
	CMatrix invPedMat = Invert(pedMat);
	

	mat = invPedMat * mat;
	mat.ConvertToEulerAngles(rx,ry,rz, rotationOrder);

	//sprintf(gString, "Left  %f,%f,%f", rx,ry,rz);
	//VarConsole.AddDebugOutput(gString);
	
	if(bPlayerShoulderRotation)
		rx /= 2.0f;
	mat.ConvertFromEulerAngles(rx,ry,rz, rotationOrder);
	mat = pedMat * mat;
	mat.UpdateRW();
	
	nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_BREAST);
	pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_UPPERARM);
	pMatrix2 = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	RwMatrixCopy(pMatrix, pMatrix2);

	mat.Attach(pMatrix);

	// Get clavicle matrix
	nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_CLAVICLE);
	pMatrix2 = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	pedMat.Attach(pMatrix2);
	invPedMat = Invert(pedMat);
	
	mat = invPedMat * mat;
	mat.ConvertToEulerAngles(rx,ry,rz, rotationOrder);

	//sprintf(gString, "Right  %f,%f,%f", rx,ry,rz);
	//VarConsole.AddDebugOutput(gString);
	
	if(bPlayerShoulderRotation)
		rx /= 2.0f;
	mat.ConvertFromEulerAngles(rx,ry,rz, rotationOrder);
	mat = pedMat * mat;
	mat.UpdateRW();
	
	// ensure hierarchy doesn't get updated again
	//m_nFlags.bDontUpdateHierarchy = true;
#endif
}
//
//
//
static float fBikeNeckWobble = 0.2f;
static float fBikeBodyWobble = 0.1f;
static float fBikeUpperArmWobble = 0.2f;

void CPed::PreRender()
{
	// If this ped is in a car it will get its pre-render done through the vehicle.
	// This assures the prerender is always done for peds that get rendered.
	if (DoesRenderAndPreRenderThroughVehicle()) return;

	PreRenderAfterTest();
}

extern float MAX_DISTANCE_PED_SHADOWS_SQR;

void CPed::PreRenderAfterTest()
{
#ifndef FINAL
	if((CPlayerPed::bDebugPlayerInfo && IsPlayer())
	|| (CPedDebugVisualiser::GetDebugDisplay()>1 && !IsPlayer()))
	{
	//	CDebug::DebugLine3D(GetPosition().x, GetPosition().y, GetPosition().z-0.5f,
	//	 GetPosition().x, GetPosition().y, GetPosition().z+ 0.5f, 0xffffffff, 0xffffffff);
		CDebug::DebugLine3D(GetPosition().x-0.5f, GetPosition().y, GetPosition().z,
		 GetPosition().x+0.5f, GetPosition().y, GetPosition().z, 0xffffffff, 0xffffffff);
		CDebug::DebugLine3D(GetPosition().x, GetPosition().y-0.5f, GetPosition().z,
		 GetPosition().x, GetPosition().y+0.5f, GetPosition().z, 0xffffffff, 0xffffffff);
	}
	if(CPlayerPed::bDebugTargetting && IsPlayer())
	{
		CDebug::DebugLine3D(GetPosition().x, GetPosition().y, GetPosition().z + 0.5f,
		 GetPosition().x + 1.5f*GetMatrix().GetForward().x, GetPosition().y + 1.5f*GetMatrix().GetForward().y, GetPosition().z + 0.5f, 0xffffffff, 0xffffffff);
	}
#endif
	if(GetPedIntelligence()->GetTaskSwim())
	{
		GetPedIntelligence()->GetTaskSwim()->ApplyRollAndPitch(this);
		m_ik.ClearFlag(PEDIK_SLOPE_PITCH);
	}
	else if(GetPedIntelligence()->GetTaskJetPack())
	{
		GetPedIntelligence()->GetTaskJetPack()->ApplyRollAndPitch(this);
		m_ik.ClearFlag(PEDIK_SLOPE_PITCH);
	}

	if(GetPedIntelligence()->GetTaskInAir())
	{
		GetPedIntelligence()->GetTaskInAir()->ApplyRollAndPitch(this);
		m_ik.ClearFlag(PEDIK_SLOPE_PITCH);
	}
	else if(m_ik.IsFlagSet(PEDIK_SLOPE_PITCH) || (!IsPlayer() && m_ik.GetSlopeAngle()!=0.0f))
	{
		m_ik.PitchForSlope();
	}
	
	// flag basically lets pedIntelligence know that anims were updated last frame
	m_nPedFlags.bCalledPreRender=true;

	UpdateRpHAnim();
	
	if(!CTimer::bSkipProcessThisFrame)
	{
/*
		if(m_pWeaponClump && GetWeapon()->GetWeaponType()==WEAPONTYPE_SKATEBOARD
		&& RpAnimBlendClumpGetFirstAssociation(m_pWeaponClump))
		{
			RpAnimBlendClumpUpdateAnimations(m_pWeaponClump, CTimer::GetTimeStepInSeconds(), !m_nFlags.bOffscreen);
			if(ISCLUMPSKINNED(m_pWeaponClump))
				RpHAnimHierarchyUpdateMatrices(GETANIMHIERARCHYFROMSKINCLUMP(m_pWeaponClump));
			
			// don't want this anims to blend slowly back to default position
			CAnimBlendAssociation *pBoardAnim = RpAnimBlendClumpGetFirstAssociation(m_pWeaponClump);
			if(pBoardAnim && !pBoardAnim->IsFlagSet(ABA_FLAG_ISPLAYING))
	//		&& (pBoardAnim->GetAnimId()==ANIM_FLIPUP_SKATEBOARD || pBoardAnim->GetAnimId()==ANIM_PUTDOWN_SKATEBOARD))
			{
				pBoardAnim->SetBlendDelta(-1000.0f);
			}
		}
		else
*/		
		if(m_pWeaponClump && GetPlayerData() && GetWeapon()->GetWeaponType()==WEAPONTYPE_MINIGUN)
		{
//			GetPlayerData()->m_fGunSpinAngle += GetPlayerData()->m_fGunSpinSpeed*CTimer::GetTimeStep();
//			if(GetPlayerData()->m_fGunSpinAngle > TWO_PI)
//				GetPlayerData()->m_fGunSpinAngle -= TWO_PI;
				
			RwFrame *pBarrelFrame = CClumpModelInfo::GetFrameFromName(m_pWeaponClump, "minigun2");
			if(pBarrelFrame)
			{
				float fAddAngle = RADTODEG(GetPlayerData()->m_fGunSpinSpeed*CTimer::GetTimeStep());
				RwMatrixRotate(RwFrameGetMatrix(pBarrelFrame), &CPedIK::XaxisIK, fAddAngle, rwCOMBINEPRECONCAT);
			}
		}
	}

	// SHADOWS
	if (m_nFlags.bIsVisible)
	{	
		if (CTimeCycle::GetShadowStrength())	
		{
			// get some info about tasks
			bool8 enteringCar = false;
			bool8 leavingCar = false;
			CTaskComplexEnterCarAsDriver* pTaskEnterCar = NULL;

			if (m_nPedFlags.bInVehicle)
			{
				// in a vehicle - check if leaving or being dragged out
				CTaskComplexLeaveCar* pTaskLeaveCar = (CTaskComplexLeaveCar*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_LEAVE_CAR);
				if (pTaskLeaveCar)
				{
					leavingCar = true;
				}
				else
				{
					CTaskComplexDragPedFromCar* pTaskDragPedFromCar = (CTaskComplexDragPedFromCar*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_DRAG_PED_FROM_CAR);
					if (pTaskDragPedFromCar)
					{
						leavingCar = true;
					}
				}
			}
			else
			{
				// not in a vehicle - check if entering
				pTaskEnterCar = (CTaskComplexEnterCarAsDriver*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER);
				if (pTaskEnterCar)
				{
					enteringCar = true;
				}
			}

#if defined GTA_PC || defined GTA_XBOX
			
			bool8 shouldUseRealTimeShadow = g_fx.GetFxQuality()==FX_QUALITY_VERY_HIGH || (g_fx.GetFxQuality()==FX_QUALITY_HIGH && GetPedType()==PEDTYPE_PLAYER1);
			bool8 realTimeShadowFailed = false;

			if (shouldUseRealTimeShadow)
			{
				// test if ped is in range for a shadow
				CVector shadowPos;
				GetBonePosition(shadowPos, BONETAG_ROOT);

				float distSqr = (shadowPos.x - TheCamera.GetPosition().x) * (shadowPos.x - TheCamera.GetPosition().x) +
		 						(shadowPos.y - TheCamera.GetPosition().y) * (shadowPos.y - TheCamera.GetPosition().y);

				if (distSqr <= MAX_DISTANCE_PED_SHADOWS_SQR)	
				{
					// test if the shadow is to be drawn
					bool8 shadowAllowed = true;

					// no shadows if in water
					if (m_nPhysicalFlags.bIsInWater)
					{
						shadowAllowed = false;
					}
					else
					{
						// no shadow if in a vehicle
						if (m_nPedFlags.bInVehicle)
						{
							shadowAllowed = false;

							// exception if on a bike or bmx
							if (m_nPedFlags.bInVehicle && m_pMyVehicle && (m_pMyVehicle->GetVehicleType()==VEHICLE_TYPE_BMX || m_pMyVehicle->GetVehicleType()==VEHICLE_TYPE_BIKE || m_pMyVehicle->GetVehicleType()==VEHICLE_TYPE_QUADBIKE))
							{
								shadowAllowed = true;
							}
						}

						// no shadow allowed if entering a vehicle
						if (enteringCar)
						{
							shadowAllowed = false;
							
							// exception if entering a bike or bmx
							CVehicle* pTargetVehicle = pTaskEnterCar->GetTargetVehicle();
							if (pTargetVehicle && (pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_BMX || pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_BIKE || pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_QUADBIKE))
							{
								shadowAllowed = true;
							}
						}

						// no shadows if the peds torso is too close to the ground
						CVector vecTorsoPos;
						GetBonePosition(vecTorsoPos, BONETAG_SPINE1);
						if (!IsAlive() || vecTorsoPos.z < GetPosition().z - 0.2f)
						{
							shadowAllowed = false;

							// allow shadows if ducking though
							if (m_nPedFlags.bIsDucking)
							{
								shadowAllowed = true;
							}
						}
					}

					// do the shadow - if allowed
					if (shadowAllowed)
					{
						// render any real time ped shadows
						g_realTimeShadowMan.DoShadowThisFrame(this);
						if (m_pRealTimeShadow == NULL)
						{
							realTimeShadowFailed = true;
						}
					}	
				}
			}

			// render normal shadow if we shouldn't be using a real time shadow or
			//                         we should be but we couldn't create one
			if (!shouldUseRealTimeShadow || realTimeShadowFailed)
			{
				// old shadow code
				if (!m_nPedFlags.bInVehicle || leavingCar)
				{
					CShadows::StoreShadowForPedObject(this,CTimeCycle::m_fShadowDisplacementX[CTimeCycle::m_CurrentStoredValue],CTimeCycle::m_fShadowDisplacementY[CTimeCycle::m_CurrentStoredValue],
													CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue],CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue],
													CTimeCycle::m_fShadowSideX[CTimeCycle::m_CurrentStoredValue],CTimeCycle::m_fShadowSideY[CTimeCycle::m_CurrentStoredValue]);
				}
			}

#else

			// old shadow code
			if (!m_nPedFlags.bInVehicle || leavingCar)
			{
				CShadows::StoreShadowForPedObject(this,CTimeCycle::m_fShadowDisplacementX[CTimeCycle::m_CurrentStoredValue],CTimeCycle::m_fShadowDisplacementY[CTimeCycle::m_CurrentStoredValue],
												CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue],CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue],
												CTimeCycle::m_fShadowSideX[CTimeCycle::m_CurrentStoredValue],CTimeCycle::m_fShadowSideY[CTimeCycle::m_CurrentStoredValue]);
			}

#endif
		}
	}


	// AF: Some funky shit with the player's arms
	if(GetModelIndex() == MODELID_PLAYER_PED)
	{
		ShoulderBoneRotation((RpClump*)m_pRwObject);
		m_nFlags.bDontUpdateHierarchy = true;
	}	
	
//	if(GetPedIntelligence()->GetTaskHold())
//		GetPedIntelligence()->GetTaskHold()->PositionEntity(this);
	
	RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pRwObject);
	RwMatrix *pMatrix;
	int32 nBone;
	// make players clothes flap about from wind
	bool bWindyDay = false, bOnBike = false;
	float fWindMod = 1.0f;
	if(IsPlayer() && CWindModifiers::FindWindModifier(GetPosition(), &fWindMod, &fWindMod)
	&& !CCullZones::PlayerNoRain())
		bWindyDay = true;
	if(m_nPedState==PED_DRIVING && m_pMyVehicle && (m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE
	|| (m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR && ((CAutomobile *)m_pMyVehicle)->IsOpenTopCar())) )
		bOnBike = true;
	
	// don't do this effect if player has no top or a vest
	if(GetPlayerData() && (GetPlayerData()->m_pClothes->GetModel(CLOTHES_SHIRT) == CKeyGen::GetUppercaseKey("vest") ||
		GetPlayerData()->m_pClothes->GetModel(CLOTHES_SHIRT) == CKeyGen::GetUppercaseKey("torso")))
	{
		bWindyDay = bOnBike = false;
	}
	
	if(bWindyDay || bOnBike) 
	{
		float fForwardSpeed = 0.0f;
		if(bOnBike)
		{
			fForwardSpeed = DotProduct(m_pMyVehicle->m_vecMoveSpeed, m_pMyVehicle->GetMatrix().GetForward());
		
#define MIN_AUDIO_FLAP_SPEED		0.4f
#define AUDIO_FLAP_SPEED_RANGE		(1.0f - MIN_AUDIO_FLAP_SPEED)

			if(fForwardSpeed > MIN_AUDIO_FLAP_SPEED)
			{
				// range of 0.4
				
#ifdef USE_AUDIOENGINE
				// This gets done in the ped's audio entity Service() routine now
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot(AudioHandle, AE_SHIRT_FLAP, (fForwardSpeed - MIN_AUDIO_FLAP_SPEED) / AUDIO_FLAP_SPEED_RANGE);
#endif //USE_AUDIOENGINE
			}
		}

		
		if(bWindyDay && CMaths::Abs(fWindMod - 1.0f) > fForwardSpeed)
			fForwardSpeed = CMaths::Abs(fWindMod - 1.0f);

        RwV3d vecScale;
		vecScale.x = CGeneral::GetRandomNumberInRange(1.0f-fBikeNeckWobble*fForwardSpeed,1.0f+fBikeNeckWobble*fForwardSpeed);
		vecScale.y = CGeneral::GetRandomNumberInRange(1.0f-fBikeNeckWobble*fForwardSpeed,1.0f+fBikeNeckWobble*fForwardSpeed);
		vecScale.z = CGeneral::GetRandomNumberInRange(1.0f-fBikeNeckWobble*fForwardSpeed,1.0f+fBikeNeckWobble*fForwardSpeed);

		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_NECK);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vecScale, rwCOMBINEPRECONCAT);

		vecScale.x = CGeneral::GetRandomNumberInRange(1.0f-fBikeBodyWobble*fForwardSpeed,1.0f+fBikeBodyWobble*fForwardSpeed);
		vecScale.y = CGeneral::GetRandomNumberInRange(1.0f-fBikeBodyWobble*fForwardSpeed,1.0f+fBikeBodyWobble*fForwardSpeed);
		vecScale.z = CGeneral::GetRandomNumberInRange(1.0f-fBikeBodyWobble*fForwardSpeed,1.0f+fBikeBodyWobble*fForwardSpeed);
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_CLAVICLE);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vecScale, rwCOMBINEPRECONCAT);

		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_R_CLAVICLE);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vecScale, rwCOMBINEPRECONCAT);
		
		if(bOnBike || GetPedIntelligence()->GetTaskJetPack()==NULL)
		{
			nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_SPINE1);
			pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
			RwMatrixScale(pMatrix, &vecScale, rwCOMBINEPRECONCAT);
		}
			
		vecScale.x = CGeneral::GetRandomNumberInRange(1.0f-fBikeUpperArmWobble*fForwardSpeed,1.0f+fBikeUpperArmWobble*fForwardSpeed);
		vecScale.y = CGeneral::GetRandomNumberInRange(1.0f-fBikeUpperArmWobble*fForwardSpeed,1.0f+fBikeUpperArmWobble*fForwardSpeed);
		vecScale.z = CGeneral::GetRandomNumberInRange(1.0f-fBikeUpperArmWobble*fForwardSpeed,1.0f+fBikeUpperArmWobble*fForwardSpeed);
		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_L_UPPERARM);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vecScale, rwCOMBINEPRECONCAT);
		
		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_R_UPPERARM);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vecScale, rwCOMBINEPRECONCAT);

//		if(m_nPedFlags.bInVehicle && m_pMyVehicle && m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
//			((CBike *)m_pMyVehicle)->FixHandsToBars(this);
	}
	

#ifdef GTA_NETWORK
//m_nFlags.bRenderBlack = bControlledByServer;  // netobr
#endif

	// can't remove skinned body parts, but in the case of the head, can scale it to zero
	if(m_nPedFlags.bRemoveHead && m_nLimbRemoveIndex==PED_HEAD)
	{
		// get bone index
		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_HEAD);
		// get the head bone matrix and scale it to zero (turn the head off)
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		CVector vZero(0.0f, 0.0f, 0.0f);
		RwMatrixScale(pMatrix, &vZero, rwCOMBINEPRECONCAT);
		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_JAW);
		// get the head bone matrix and scale it to zero (turn the head off)
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vZero, rwCOMBINEPRECONCAT);
		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_L_BROW);
		// get the head bone matrix and scale it to zero (turn the head off)
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vZero, rwCOMBINEPRECONCAT);
		nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_R_BROW);
		// get the head bone matrix and scale it to zero (turn the head off)
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixScale(pMatrix, &vZero, rwCOMBINEPRECONCAT);
	}
	
	
	

	// MN - wobble belly and breasts 
	RwV3d axis = {0.0f, 0.0f, 1.0f};
	float angleWobble = 0.0f;	
	static float wobbleDepth = 5.0f;
	
	
	if (m_wobble>0.0f)
	{
		angleWobble = -CMaths::Sin(m_wobble) * wobbleDepth;
		m_wobble -= m_wobbleSpeed * CTimer::GetTimeStep();

		// don't wobble player tits. These bones are used elsewhere
		if(!IsPlayer())
		{
			nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_BREAST);
			pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
			RwMatrixRotate(pMatrix, &axis, angleWobble, rwCOMBINEPRECONCAT);
			
			nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_BREAST);
			pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
			RwMatrixRotate(pMatrix, &axis, angleWobble, rwCOMBINEPRECONCAT);
		}
				
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_BELLY);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, angleWobble, rwCOMBINEPRECONCAT);
	}		

/*	
	// MN - ped breathing
	float angleBreathe = sin(m_breatheCounter) * 4.0f;
	
	if (angleBreathe < 0.0f)
	{
		angleBreathe *= 0.3f;
	}

	m_breatheCounter += 0.14f;

	nBone = RpHAnimIDGetIndex(pHierarchy,  ConvertPedNode2BoneTag(PED_TORSO));
	pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
	RwMatrixRotate(pMatrix, &axis, angleBreathe, rwCOMBINEPRECONCAT);
*/	
	// MN - ped blinking
/*	if (CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.985f)
	{
		if (!IsPlayer() && IsAlive())
		{
			nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_BROW);
			if(nBone != -1)
			{
				pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
				RwMatrixRotate(pMatrix, &axis, 15.0f, rwCOMBINEPRECONCAT);
			}			
			nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_BROW);
			if(nBone != -1)
			{
				pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
				RwMatrixRotate(pMatrix, &axis, 15.0f, rwCOMBINEPRECONCAT);
			}	
		}
	}
*/
	// For earthquake, ped will be a bit shaking by that
	if (CWeather::Earthquake > 0.0f)
	{
		float EQuake = CGeneral::GetRandomNumberInRange(-CWeather::Earthquake, CWeather::Earthquake);
		
		EQuake *= 0.0025f;
		
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_CALF);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_CALF);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);

		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_FOREARM);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_FOREARM);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_UPPERARM);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_UPPERARM);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_FOOT);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_FOOT);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_L_HAND);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_R_HAND);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
		nBone = RpHAnimIDGetIndex(pHierarchy,  BONETAG_HEAD);
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwMatrixRotate(pMatrix, &axis, EQuake, rwCOMBINEPOSTCONCAT);
	
	}
	
// temporarily remove spurting blood since removing limbs is out (16/09/2001)
// maybe replace with pulsing spurts from intact but injured limbs

// use frameCounter to pulse blood every 8 frames
/*
	if(m_nPedFlags.bRemoveHead && m_nPedFlags.bIsPedDieAnimPlaying && m_nLimbRemoveIndex != -1
	&& (CTimer::m_FrameCounter &7) > 3 )
	{
		CVector vec(0.0f,0.0f,0.0f), vel(0.0f,0.0f,0.0f);

		RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pRwObject);
		int32 nBone = RpHAnimIDGetIndex(pHierarchy, m_aPedFrames[m_nLimbRemoveIndex]->boneTag);
		// hmm try this 1st
		RwMatrix *pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		RwV3dTransformPoints(&vec, &vec, 1, pMatrix);
		//vec = m_mat * vec;

		switch(m_nLimbRemoveIndex){
			case PED_HEAD:
				vel = 0.1f*GetMatrix().GetUp();
				break;
			case PED_UPPERARML:
				vel = -0.04f*GetMatrix().GetRight() + 0.04f*GetMatrix().GetUp();
				break;
			case PED_UPPERARMR:
				vel = 0.04f*GetMatrix().GetRight() + 0.04f*GetMatrix().GetUp();
				break;
			case PED_UPPERLEGL:
				vel = 0.05f*GetMatrix().GetForward() + 0.04f*GetMatrix().GetUp();
				break;
			case PED_UPPERLEGR:
				vel = 0.05f*GetMatrix().GetForward() + 0.04f*GetMatrix().GetUp();
				break;
			default:
				vel = CVector(0.0f,0.0f,0.0f);
				break;
		}
		
		// MN - FX SYSTEM -------------------------------
		// blood from bullet hitting ped
		g_fx.AddBlood(vec, vel, 4, this->m_lightingFromCollision);			
		// ----------------------------------------------	
		
	}
*/	
	// blood spurting from decapitaded head
	if (m_nPedFlags.bRemoveHead && m_nLimbRemoveIndex==PED_HEAD && GetPedState()!=PED_DEAD && !m_nPedFlags.bIsDyingStuck && (CTimer::m_FrameCounter&7)>3)
	{	
		RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pRwObject);
		int32 nBone = RpHAnimIDGetIndex(pHierarchy, BONETAG_HEAD);
		RwMatrix *pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
		
		CVector pos = *RwMatrixGetPos(pMatrix);
		CVector vel = 0.6f*GetMatrix().GetUp();
	
		// MN - FX SYSTEM -------------------------------
		// blood from bullet hitting ped
		g_fx.AddBlood(pos, vel, 16, this->m_lightingFromCollision);			
		// ----------------------------------------------
	}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// splashes on peds while it rains ( for more heavy rain):
	if((CWeather::Rain > 0.3f) &&  (TheCamera.SoundDistUp > CAM_DIST_SOUND_SCAN-5.0f) &&
		(!this->m_nPedFlags.bInVehicle) && CGame::currArea == AREA_MAIN_MAP &&
		this->GetPosition().z < 900.0f && !CCullZones::CamNoRain())
	{
		const float	CAM_DISTANCE_NOT_VISIBLE		= 25.0f;//66.0f;//45.0f;
		const float CAM_DISTANCE_NOT_VISIBLE_SNIPER = 50.0f;

		// current distance of the object to camera:
		float	fCamDistance = (TheCamera.GetPosition() - this->GetPosition()).Magnitude();

		bool32	bVisibleForCamera = FALSE;
	
	
		//if(TheCamera.LODDistMultiplier > 1.0f)
		//{
		//	// sniper rifle is zooming:
		//	bVisibleForCamera = (fCamDistance<CAM_DISTANCE_NOT_VISIBLE_SNIPER)?TRUE:FALSE;
		//}
		//else
		//{
			//normal mode:
			bVisibleForCamera = (fCamDistance<CAM_DISTANCE_NOT_VISIBLE)?TRUE:FALSE;
		//}


		if(bVisibleForCamera)
		{
			CPedModelInfo *pModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(this->GetModelIndex());
			pModelInfo->AnimatePedColModelSkinnedWorld((RpClump *)m_pRwObject);
			CColModel *pColModel = pModelInfo->GetHitColModel();
	
			// amount of time (~3 msecs) to calculate new gunflash
			// position when player is moving:
			//const float	GF_SCALE_TIME = 	1.5f;

			bool32 bDisplaySplashes = TRUE;
			
			
			CVector playerSpeed = FindPlayerSpeed();
			// if player is moving = don't generate splashes
			if( (ABS(playerSpeed.x) > 0.05f) ||
				(ABS(playerSpeed.y) > 0.05f)	)
					bDisplaySplashes = FALSE;
			else
			// ped is falling/dying?
			if(	(this->m_nPedState == PED_FALL) 	||
				(this->m_nPedState == PED_DIE)		||
				(this->m_nPedState == PED_DEAD)		||
				(this->m_nPedState == PED_ATTACK)	||
				(this->m_nPedState == PED_FIGHT)	)
					bDisplaySplashes = FALSE;
			else
			// is ped head under 1.3 meters from ground level? - suitable for first aid men;
			// (0.3 meters from center-of-ped)?
			if(!this->IsPedHeadAbovePos(0.3f))
					bDisplaySplashes = FALSE;
			else
			// if we use "tired-after-run" animation:
			if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE_TIRED))
					bDisplaySplashes = FALSE;
				

			CCollisionData* pColData = pColModel->GetCollisionData();
			int32 numSpheres = pColData->m_nNoOfSpheres;

			if(bDisplaySplashes)
			for(int32 i=0; i<numSpheres; i++)
			{
				CColSphere *sphere = &pColData->m_pSphereArray[i];

				switch(sphere->m_data.m_nPieceType)
				{
					// we want only head & upper arms to be splashed:
					case(PED_SPHERE_HEAD):
					case(PED_SPHERE_UPPERARM_L):
					case(PED_SPHERE_UPPERARM_R):
						break;
						
					// ignore all other spheres:
					default:
						continue;
						break;
				}


				// MN - FX SYSTEM -------------------------------	
				// rain splashes on players head and shoulders	
				const float SPREAD_RADIUS = 0.08f;

				//					red   green blue  alpha  size   rot   life
				FxPrtMult_c fxMults(1.0f, 1.0f, 1.0f, 0.35f, 0.01f, 0.0f, 0.03f);
	
				CVector pos = sphere->m_vecCentre;
				pos.x += CGeneral::GetRandomNumberInRange(-SPREAD_RADIUS, SPREAD_RADIUS);
				pos.y += CGeneral::GetRandomNumberInRange(-SPREAD_RADIUS, SPREAD_RADIUS);
				pos.z += sphere->m_fRadius * 0.75f;

				RwV3d vel = playerSpeed * 50.0f;
			
				g_fx.m_fxSysSplash->AddParticle(&pos, &vel, 0.0f, &fxMults);
		
			
				// ----------------------------------------------


			}//for(int32 i=0; i<numSpheres; i++)...
			
		}//if(bVisibleForCamera)...

	}//	if((CWeather::Rain > 0.3f) &&  (TheCamera.SoundDistUp > CAM_DIST_SOUND_SCAN-5.0f))...
/////////////////////////////////////////////////////////////////

	// water dripping from player if he is wet
	if (GetPlayerData() && GetPlayerData()->m_wetness>0 && GetPlayerData()->m_waterCoverPerc<30)
	{
		//						red   green blue  alpha  size  rot   life
		FxPrtMult_c fxMultsDrip(1.0f, 1.0f, 1.0f, 0.20f, 0.15f, 0.0f, 0.1f);
		
		CVector pos = GetPosition();
		pos.x += CGeneral::GetRandomNumberInRange(-0.3f, 0.3f);
		pos.y += CGeneral::GetRandomNumberInRange(-0.3f, 0.3f);
		pos.z += CGeneral::GetRandomNumberInRange(-0.8f, 0.2f);
		
		CVector vel(0.0f, 0.0f, 0.0f);
		
		fxMultsDrip.m_alpha *= GetPlayerData()->m_wetness/100.0f;
		
//		int32 randNum = CGeneral::GetRandomNumberInRange(0, 100); 
//		if (randNum < 25+GetPlayerData()->m_wetness)
		{
			g_fx.m_fxSysWaterSplash->AddParticle(&pos, &vel, 0.0f, &fxMultsDrip);
		}
	}


	// If ped is in a vehicle then get lighting from that vehicle 
    if( m_nPedFlags.bInVehicle && m_pMyVehicle )
    {
    	m_lightingFromCollision = m_pMyVehicle->m_lightingFromCollision;
    }

}//end of void CPed::PreRender()....

#pragma mark --- optimize for size off ---
#pragma optimize_for_size off

// Name			:	Render
// Purpose		:	Hardwired ped rendering
// Parameters	:	None
// Returns		:	Nothing
CVector vecTestTemp(-1.0f,-1.0f,-1.0f);
void CPed::Render()
{
	UInt32 alphaTestRef= 1;
	if (IsPlayer())
	{
		RwRenderStateGet(rwRENDERSTATEALPHATESTFUNCTIONREF, &alphaTestRef);
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)1);
	}

	if (m_nPedFlags.bDontRender)
	{
		return;
	}

	// If there is a reflection being rendered the playerped is put on the renderlist
	// even if his m_nFlags.bIsVisible is false. For the main render (not the reflection) we
	// jump out here.
	if ( (!m_nFlags.bIsVisible) && !(CMirrors::bRenderingReflection && !(CMirrors::TypeOfMirror == CMirrors::MIRTYPE_FLOOR)))
	{
		return;
	}

//	if(m_nPedFlags.bInVehicle && m_pMyVehicle && m_nPedState != PED_EXIT_CAR && m_nPedState != PED_DRAGGED_FROM_CAR)
	if(m_nPedFlags.bInVehicle && m_pMyVehicle && (!GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_LEAVE_CAR)) && (!GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_CAR_SLOW_BE_DRAGGED_OUT_AND_STAND_UP)))
	{
		 // used for coach so can't see player sitting beside door
		if(!m_nPedFlags.bRenderPedInCar)
			return;

		if(m_pMyVehicle->GetBaseVehicleType()!=VEHICLE_TYPE_BIKE
		&& m_pMyVehicle->GetVehicleType()!=VEHICLE_TYPE_QUADBIKE && !IsPlayer())
		{
			float distToCameraSqr = (TheCamera.GetPosition() - GetPosition()).MagnitudeSqr();
			if(m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
			{
				if(distToCameraSqr > BOATRIDER_HI_DETAIL_RANGE*BOATRIDER_HI_DETAIL_RANGE)
					return;
			}
			else if(distToCameraSqr > DRIVER_HI_DETAIL_RANGE*DRIVER_HI_DETAIL_RANGE)
				return;
		}		
	}


	#ifdef GTA_PS2
	// select FastCulling render mode for skinned peds (instead of expensive TrueClipping mode):
	const bool8 prevTSClipState = RpSkyGetTrueTSClipper();
	if((this->GetPedType() != PEDTYPE_PLAYER1) && (this->GetPedType() != PEDTYPE_PLAYER2))	// do not use fastculling for player peds 
	{	
		RpSkySelectTrueTSClipper(FALSE);
	}
	#endif

	// "Vision-FX" stuff ("night vision", "infrared vision", etc.)
	if( CPostEffects::IsVisionFXActive() )
	{
		CPostEffects::InfraredVisionStoreAndSetLightsForHeatObjects( this );
		CPostEffects::NightVisionSetLights();

		CEntity::Render();

		CPostEffects::InfraredVisionRestoreLightsForHeatObjects();
	} // if //
	else
	{
		CEntity::Render();
	} // else //

	#ifdef GTA_PS2
		// restore previous clip render mode:
		RpSkySelectTrueTSClipper(prevTSClipState);
	#endif
	
	bool8 bRenderWeapon = true;
	if (GetPlayerData())
	{
		if (GetPlayerData()->m_bRenderWeapon == false)
		{
			bRenderWeapon = false;
		}
	}

	if(m_pWeaponClump && bRenderWeapon
	&& !(m_nPhysicalFlags.bIsInWater && GetPedIntelligence()->GetTaskSwim())
	&& !GetPedIntelligence()->GetTaskHold(false))
	{
#if (defined(GTA_PC) || defined(GTA_XBOX))
		CVisibilityPlugins::AddWeaponPedForPC(this);
#else //GTA_PC
		RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pRwObject);
		int32 boneTag;
		bool bSkate = false;
		CAnimBlendAssociation *pSkateAnim = NULL;
/*		
		if(m_motionAnimGroup==ANIM_SKATEBOARD_PED)
		{
			CAnimBlendAssociation *pBoardAnim = RpAnimBlendClumpGetFirstAssociation(m_pWeaponClump);
			if(pBoardAnim)// && (pBoardAnim->GetAnimId()==ANIM_FLIPUP_SKATEBOARD || pBoardAnim->GetAnimId()==ANIM_PUTDOWN_SKATEBOARD))
			{
				boneTag = BONETAG_ROOT;
				pSkateAnim = RpAnimBlendClumpGetAssociation((RpClump *)m_pRwObject, ANIM_STD_STARTWALK);
			}
			else if(GetMoveState() >= PEDMOVE_WALK)
			{
				boneTag = BONETAG_L_FOOT;
				bSkate = true;
			}
			else
				boneTag = BONETAG_R_HAND;
		}
		else
*/		
		if(GetWeapon()->GetWeaponType()==WEAPONTYPE_PARACHUTE)
			boneTag = BONETAG_SPINE1;
		else
			boneTag = BONETAG_R_HAND;

		RwMatrix *pNodeMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, boneTag));
		if(boneTag==BONETAG_ROOT)
			pNodeMatrix = GetRwMatrix();
		
		RwFrame *pWeaponFrame = RpClumpGetFrame(m_pWeaponClump);
		RwMatrix *pWeaponMatrix = RwFrameGetMatrix(pWeaponFrame);
/*		
		if(bSkate)
		{
			CVector vec(0.0f,0.0f,0.0f);
			vec = Multiply3x3(GetMatrix(), vec);
			vec += pNodeMatrix->pos;
		
			// use ped's matrix directly to set rotations
			*pWeaponMatrix = *GetRwMatrix();
			// then override position
			pWeaponMatrix->pos.x = vec.x;
			pWeaponMatrix->pos.y = vec.y;
			pWeaponMatrix->pos.z = vec.z;
			
			if(RpAnimBlendClumpGetFirstAssociation(m_pWeaponClump)==NULL)
			{
				static RwV3d skateAxisX = {1.0f, 0.0f, 0.0f};
				static float skateRotX = 93.0f;
				static RwV3d skateAxisY = {0.0f, 1.0f, 0.0f};
				static float skateRotY = -7.0f;
				static RwV3d skateAxisZ = {0.0f, 0.0f, 1.0f};
				static float skateRotZ = -85.0f;
				RwMatrixRotate(pWeaponMatrix,  &skateAxisZ, skateRotZ, rwCOMBINEPRECONCAT);
				RwMatrixRotate(pWeaponMatrix,  &skateAxisY, skateRotY, rwCOMBINEPRECONCAT);
				RwMatrixRotate(pWeaponMatrix,  &skateAxisX, skateRotX - RADTODEG(m_ik.GetSlopeRollAngle()), rwCOMBINEPRECONCAT);
				static RwV3d skateTrans = {-0.22f, -0.18f, -0.05f};
				RwMatrixTranslate(pWeaponMatrix,  &skateTrans, rwCOMBINEPRECONCAT);
			}
		}
		else
*/
		*pWeaponMatrix = *pNodeMatrix;
		
		if(GetWeapon()->GetWeaponType()==WEAPONTYPE_PARACHUTE)
		{
			static RwV3d parachuteOffset = {0.1f, -0.15f, 0.0f};
			RwMatrixTranslate(pWeaponMatrix,  &parachuteOffset, rwCOMBINEPRECONCAT);
			RwMatrixRotate(pWeaponMatrix, &CPedIK::YaxisIK, 90.0f, rwCOMBINEPRECONCAT);
		}
/*
		else if(pSkateAnim && pSkateAnim->GetNode(0))
		{
			CVector vecTranslate;
			pSkateAnim->GetNode(0)->GetCurrentTranslation(vecTranslate, 1.0f);
			vecTranslate *= -1.0f;
			RwMatrixTranslate(pWeaponMatrix,  (RwV3d*)&vecTranslate, rwCOMBINEPRECONCAT);
		}
*/
		// need to update the alpha on the gunflash just before we render the weapon
		SetGunFlashAlpha(false);
	
		RwFrameUpdateObjects(pWeaponFrame);
		// now render the weapon

		RpClumpRender(m_pWeaponClump);
		
		// try and render the weapon again in the other hand!
		if(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill())->IsWeaponFlagSet(WEAPONTYPE_TWIN_PISTOLS))
		{
			boneTag = BONETAG_L_HAND;
			pNodeMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, boneTag));
			*pWeaponMatrix = *pNodeMatrix;

			static RwV3d axis = {1.0f, 0.0f, 0.0f};
			RwMatrixRotate(pWeaponMatrix,  &axis, 180.0f, rwCOMBINEPRECONCAT);
			static RwV3d trans = {0.04f, -0.05f, 0.0f};
			RwMatrixTranslate(pWeaponMatrix,  &trans, rwCOMBINEPRECONCAT);
			
			// need to update the alpha on the gunflash just before we render the weapon
			SetGunFlashAlpha(true);
			
			RwFrameUpdateObjects(pWeaponFrame);
			// now render the weapon
			RpClumpRender(m_pWeaponClump);
		}
#endif //GTA_PC

		if(m_nGunFlashBlendAmount > 0 || m_nGunFlashBlendAmount2 > 0)
			ResetGunFlashAlpha();

/*
		if(GetPlayerData() && GetPlayerData()->m_pSpecialAtomic)
		{
			pWeaponFrame = RpAtomicGetFrame(GetPlayerData()->m_pSpecialAtomic);
			pWeaponMatrix = RwFrameGetMatrix(pWeaponFrame);
			*pWeaponMatrix = *pNodeMatrix;

			GetPlayerData()->m_fGunSpinAngle += GetPlayerData()->m_fGunSpinSpeed*CTimer::GetTimeStep();
			if(GetPlayerData()->m_fGunSpinAngle > TWO_PI)
				GetPlayerData()->m_fGunSpinAngle -= TWO_PI;

			CMatrix matrix;
			matrix.Attach(pWeaponMatrix);
			CMatrix matrix2;
			matrix2.SetRotateX(GetPlayerData()->m_fGunSpinAngle );
			matrix2.Rotate(vecTestTemp.x*DEGTORAD(-4.477f), vecTestTemp.y*DEGTORAD(29.731f), vecTestTemp.z*DEGTORAD(1.064f));
			matrix2.Translate(CVector(0.829f, -0.001f, 0.226f));
			
			matrix = matrix*matrix2;
			matrix.UpdateRW();
			
			RwFrameUpdateObjects(pWeaponFrame);
			// now render the weapon
			RpAtomicRender(GetPlayerData()->m_pSpecialAtomic);
		}*/
	}

	if(m_pGogglesClump)
	{
		RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pRwObject);

		RwMatrix *pNodeMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, BONETAG_HEAD));
		
		RwFrame *pGogglesFrame = RpClumpGetFrame(m_pGogglesClump);
		RwMatrix *pGogglesMatrix = RwFrameGetMatrix(pGogglesFrame);
		
		*pGogglesMatrix = *pNodeMatrix;

		CVector vec = CVector(0.0f, 0.084f, 0.0f);
		
		// transform offset through bone matrix into world frame
		RwV3dTransformPoints(&vec, &vec, 1, pNodeMatrix);
		
		pGogglesMatrix->pos = vec;
		
		RwFrameUpdateObjects(pGogglesFrame);
		
		// now render the goggles
		RpClumpRender(m_pGogglesClump);	
	}

	if(GetPedIntelligence()->GetTaskJetPack())
		GetPedIntelligence()->GetTaskJetPack()->RenderJetPack(this);
		
	m_nPedFlags.bHasBeenRendered = true;

	if (IsPlayer())
	{
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)alphaTestRef);
	}
}

void CPed::SpecialEntityPreCollisionStuff(CPhysical *pPhysical, bool bDoingShift, bool &bSkipTestEntirely, bool &bSkipCol, bool& bForceBuildingCol, bool& bForceSoftCol)
{
	if(pPhysical->GetIsTypeVehicle() && m_nPedFlags.bKnockedOffBike && m_pMyVehicle==pPhysical)
	{
		bSkipTestEntirely = true;
	}
	else if(m_pNOCollisionVehicle==pPhysical
	|| pPhysical->m_pNOCollisionVehicle==this)
	{
		bSkipCol = true;
		if(!m_nPedFlags.bKnockedUpIntoAir || m_nPedFlags.bKnockedOffBike)
			this->m_nPhysicalFlags.bSkipLineCol = true;
	}
	else if(m_pAttachToEntity==pPhysical
	|| pPhysical->m_pAttachToEntity==this)
	{
		bSkipTestEntirely = true;	//bSkipCol = true;
	}
	else if(m_pAttachToEntity && pPhysical->m_pAttachToEntity)
	{
		bSkipTestEntirely = true;	//bSkipCol = true;
	}
	else if(pPhysical->m_nPhysicalFlags.bDoorPhysics)
	{
		if(pPhysical->m_nPhysicalFlags.bInfiniteMass || pPhysical->m_nPhysicalFlags.bDoorHitEndStop)
			bForceBuildingCol = true;
		else if(bDoingShift)
			bSkipTestEntirely = true;
		else if(GetIsStuck() || pPhysical->GetIsStuck())
			bForceSoftCol = true;

		this->m_nPhysicalFlags.bSkipLineCol = true;
	}
	else if(pPhysical->m_nPhysicalFlags.bHangingPhysics || pPhysical->m_nPhysicalFlags.bPoolBallPhysics)
	{
		if(bDoingShift)
			bSkipCol = true;
		else if(GetIsStuck() || pPhysical->GetIsStuck())
			bForceSoftCol = true;

		this->m_nPhysicalFlags.bSkipLineCol = true;
	}
	else if(pPhysical->GetIsTypeObject())
	{
		CObject *pObject = (CObject *)pPhysical;
		if(pObject->m_nObjectFlags.bLampPostCollision && pObject->GetMatrix().zz<0.66f)
		{
			bSkipCol = true;
			this->m_nPhysicalFlags.bSkipLineCol = true;
		}
		else if(pObject->GetModelIndex()==MODELID_WEAPON_GRENADE && pObject->GetMatrix().tz < GetMatrix().tz)
		{
			bSkipCol = true;
			this->m_nPhysicalFlags.bSkipLineCol = true;
		}
		else if((pObject->m_pObjectInfo->m_fUprootLimit > 0.0f || pObject->m_nPhysicalFlags.bInfiniteMass)
		&& CMaths::Abs(pObject->m_vecMoveSpeed.x) < 0.001f
		&& CMaths::Abs(pObject->m_vecMoveSpeed.y) < 0.001f
		&& CMaths::Abs(pObject->m_vecMoveSpeed.z) < 0.001f)
		{
			bForceBuildingCol = true;
		}
		else if(pObject->GetIsStuck())
		{
			bForceBuildingCol = true;
		}			
	}
	else if(pPhysical->GetModelIndex()==MODELID_CAR_RCBANDIT || pPhysical->GetModelIndex()==MODELID_CAR_RCTIGER || pPhysical->GetModelIndex()==MODELID_CAR_RCCAM)
	{
		bSkipCol = true;
		this->m_nPhysicalFlags.bSkipLineCol = true;
	}
	else if(pPhysical->GetIsStuck())
	{
		bForceBuildingCol = true;
	}
	
	if(IsPlayer() && GetPedIntelligence()->GetTaskClimb())
		this->m_nPhysicalFlags.bSkipLineCol = true;
}

#define MIN_RAD_PLAYER		(0.3f)
#define MIN_RAD_PED			(0.3f)
#define MIN_RAD_PED_SQR		(MIN_RAD_PED*MIN_RAD_PED)
#define HIGHSPEED_ELASTICITY_MULT_PED	(2.0f)
//
uint8 CPed::SpecialEntityCalcCollisionSteps(bool &bDoPreCheckAtFullSpeed, bool &bDoPreCheckAtHalfSpeed)
{
	uint8 nNumChecks = 1;
	
	if(m_pAttachToEntity==NULL && (GetPlayerData() ||
	m_vecMoveSpeed.MagnitudeSqr()*CTimer::GetTimeStep()*CTimer::GetTimeStep() >= MIN_RAD_PED_SQR))
	{
		float fStepDist = m_vecMoveSpeed.Magnitude()*CTimer::GetTimeStep();
		// want to make sure the player ped always does at least 2 checks per frame
		if(GetPlayerData())
		{
			if(m_pGroundPhysical)
				nNumChecks = (uint8) MAX(4, CMaths::Ceil(2.0f*fStepDist / MIN_RAD_PED));
			else
				nNumChecks = (uint8) MAX(2, CMaths::Ceil(fStepDist / MIN_RAD_PED));
		}
		else
			nNumChecks = (uint8) CMaths::Ceil(1.5f*fStepDist / MIN_RAD_PED);
		
		if(!GetPlayerData())
			m_fElasticity *= HIGHSPEED_ELASTICITY_MULT_PED;
	}
	
	return nNumChecks;
}

#pragma optimize_for_size on
#pragma mark --- optimize for size on ---


/*
void CPed::CheckAroundForPossibleCollisions()
{
	// need to get sphere from our ped, and enlarge it
	// then check it against list of objects in this sector
	// if there's a hit, we change ped heading by a small bit

	Int16	Num;
	CEntity *ppResults[8]; // whatever.. temp list
	CEntity *pEntity;
	CVector vecCentreA = GetBoundCentre();
	CVector vecCentreB;
	CVector vecAvoidVector;	
	float fRadiusA = 1.0f;// larger area around ped
	float fRadiusB;
//	float fAngleAvoid;
//	float fAvoidTime;

	if(CTimer::GetTimeInMilliseconds() > m_nAntiSpazTimer)
	{

		CWorld::FindObjectsInRange(vecCentreA, 10.0f, true, &Num, 6, ppResults, 0, 1, 0, 1, 0);	// Just Objects Cars and Buildings we're interested in

		for(int i = 0; i< Num; i++)
		{
			pEntity = (CEntity *)ppResults[i];//--Num]);

			if((m_nTypeOfSeek==SEEKTYPE_PHONE)&&
				(gPhoneInfo.PhoneAtThisPosition( pEntity->GetPosition() )))
			{
				break;
			}
			vecCentreB = pEntity->GetBoundCentre();
			fRadiusB = pEntity->GetBoundRadius();// + 0.5f;

			vecCentreB.z=vecCentreA.z; // don't take z into account

#ifdef DEBUG
//			{
//				char	Str[80];
//				sprintf(Str, "fRadiusB=%f",fRadiusB);
//				CDebug::DebugString(Str, 1, 16);
//			}
#endif

			if ((fRadiusB > 4.5f)||(fRadiusB < 1.0f))// total fudge until I can think of a better way
			{
				fRadiusB= 1.0f;
			}

			if((vecCentreA - vecCentreB).MagnitudeSqr() < CMaths::Sqr(fRadiusA + fRadiusB))
			{
				// collide... change heading
				// code below ok for moving around circular objcts

				m_fDesiredHeading += EIGHTH_PI;

			}

		}
	}
}
*/

// Name			:	SetIdle
// Purpose		:	Idle ped
// Parameters	:	None
// Returns		:	Nothing

void CPed::SetIdle() 
{
	ASSERT(m_nPedState != PED_DEAD && m_nPedState != PED_DIE);
 
	// AF: Why can't you go to idle from mug and flee?
	if((m_nPedState != PED_IDLE)&&(m_nPedState != PED_MUG)&&(m_nPedState != PED_FLEE_ENTITY))
	{
#ifndef GTA_NETWORK
		ASSERT(m_nPedState != PED_DRIVING);
#endif

		// this state needs to be cleared manually otherwise anims won't get removed
		if(GetPedState()==PED_AIMGUN)
//			ClearPointGunAt();		

		SetPedState(PED_IDLE);
		SetMoveState(PEDMOVE_STILL);
		// we're clearing stuff here, so better clear stored state too
		// or it might get restored at some later date
		//m_nPedStoredState = PED_NONE;
	}
	
	//if(m_eWaitState==PEDWAIT_FALSE)
	//	m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2000,4000);
}

/*
// Name			:	Idle
// Purpose		:	Ped stands about doing nothing
// Parameters	:	None
// Returns		:	Nothing

void CPed::Idle()
{
	DEBUGPEDSTATE
	(
		CDebug::DebugString("IDLE", 0, 3);
	)

	// if someone is getting out of my door and I'm blocking the way
	// then move out of the way
	if(m_pMyVehicle && m_pMyVehicle->m_nGettingOutFlags && m_nDoor)
	{
		uint8 nFlagForMyDoor;
		
		switch(m_nDoor)
		{
			case CAR_DOOR_LF:nFlagForMyDoor = 1;break;
			case CAR_DOOR_LR:nFlagForMyDoor = 2;break;
			case CAR_DOOR_RF:nFlagForMyDoor = 4;break;
			case CAR_DOOR_RR:nFlagForMyDoor = 8;break;
		}
		
		if((m_pMyVehicle->m_nGettingOutFlags & nFlagForMyDoor)&&(m_Objective != KILL_CHAR_ON_FOOT))
		{
			CVector vecExitPoint,vecDistancFromDoor;
			
			vecExitPoint = GetPositionToOpenCarDoor(m_pMyVehicle, m_nDoor);
			vecDistancFromDoor = GetPosition() - vecExitPoint;
			
			if(vecDistancFromDoor.MagnitudeSqr() < 0.5f*0.5f)
			{
				SetMoveState(PEDMOVE_WALK);
				return;
			}
		}
	} 

	if(m_eMoveState != PEDMOVE_STILL && !IsPlayer())
		SetMoveState(PEDMOVE_STILL);
	
	// stop all movement
	m_vecCurrentVelocity = CVector2D(0.0f, 0.0f);
} // end - CPed::Idle
*/


/*
// Name			:	SetPause
// Purpose		:	Sets ped pause state. Ped will pause for a short period
// Parameters	:	nPauseTime - base pause time in milliseconds
// Returns		:	Nothing

void CPed::SetPause(int32 nPauseTime)
{
	if (IsPedInControl() && !(m_nPedState == PED_PAUSE)
		&& !(m_nPedState == PED_FLEE_ENTITY))
	{
		SetStoredState();
		SetPedState(PED_PAUSE);
		SetMoveState(PEDMOVE_STILL);

		m_nPauseTimer = (CTimer::GetTimeInMilliseconds() + nPauseTime + (CGeneral::GetRandomNumber() & 1023) );// % MAX_TIMER_MILLISECONDS;
	}
} // end - CPed::SetPause
*/


/*
// Name			:	ClearPause
// Purpose		:	Clears ped pause state.
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearPause()
{
	//m_nPedState = 0;
	RestorePreviousState();
} // end - CPed::ClearPause
*/

/*
// Name			:	Pause
// Purpose		:	Ped waits for a short time
// Parameters	:	None
// Returns		:	Nothing

void CPed::Pause()
{
	DEBUGPEDSTATE
	(
		CDebug::DebugString("PAUSE", 0, 3);
	)

	// stop all movement
	m_vecCurrentVelocity = CVector2D(0.0f, 0.0f);

	if (CTimer::GetTimeInMilliseconds() > m_nPauseTimer)
	{
		// Pause has timed out - clear pause behaviour
		ClearPause();
		// dont just SetWanderPath(), restores old state in ClearPause()
//		SetWanderPath(m_nPedRand % 8);
	}
} // end - CPed::Pause
*/


/*
// Name			:	SetFall
// Purpose		:	Sets ped fall state. Ped will fall to the ground
// Parameters	:	nUnconsciousTime - time ped remains on the ground after falling in milliseconds
//					If set to -1 the ped will stay down indefinitely
//					pKnockDownEntity - ptr to the entity that caused the fall
// Returns		:	Nothing

void CPed::SetFall(int32 nUnconsciousTime, AnimationId anim, bool8 bForceFall)
{
	CAnimBlendAssociation *pAnim = NULL;

	// don't want to knock the player down when they're attached to a vehicle
	if(m_pAttachToEntity)
		return;

	if (IsPedInControl() || (bForceFall && m_nPedState!=PED_DIE && m_nPedState!=PED_DEAD))
	{
		ClearLookFlag();
		ClearAimFlag();
		SetStoredState();
		SetPedState(PED_FALL);
		// might want to set fall without specifying an anim (ie anim already started)
		if(anim!=ANIM_STD_NUM)
		{
			// better check if anim is already there first
			pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, anim);
			if(pAnim){	// if so - reset and set to blend in again
				pAnim->SetCurrentTime(0.0f);
				pAnim->SetBlendAmount(0.0f);
				pAnim->SetBlendDelta(8.0f);
				pAnim->SetFlag(ABA_FLAG_ISPLAYING);
			}
			else	// else blend in anim from scratch
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, anim, 8.0f);
// TEMP MEASURE TILL ANIM GETS BROKEN UP (miss out 1st bit of anim)			
			if(anim == ANIM_STD_BIKE_FALLBACK)
				pAnim->SetCurrentTime(0.4f);
		}
		else if(IsPlayer())
		{
			pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_ROLLOUT_LHS);
			if(pAnim==NULL)
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_ROLLOUT_RHS);
		}
		
		if (nUnconsciousTime == -1)
		{
			m_nUnconsciousTimer = -1;
		}
		else if( IsPlayer() && pAnim )	// keep the player down for 0.5sec after each knockdown
		{
			if(pAnim->GetAnimId()==ANIM_STD_ROLLOUT_LHS || pAnim->GetAnimId()==ANIM_STD_ROLLOUT_RHS)
				m_nUnconsciousTimer = CTimer::GetTimeInMilliseconds() + 1000*pAnim->GetTotalTime() - 1000*pAnim->GetCurrentTime() + 100;
			else
				m_nUnconsciousTimer = CTimer::GetTimeInMilliseconds() + 1000*pAnim->GetTotalTime() + 500;
		}
		else if(pAnim)
			m_nUnconsciousTimer = CTimer::GetTimeInMilliseconds() + 1000*pAnim->GetTotalTime() + nUnconsciousTime + ((RandomSeed + CTimer::m_FrameCounter)%1000);
		else
			m_nUnconsciousTimer = CTimer::GetTimeInMilliseconds() + 1000 + nUnconsciousTime + ((RandomSeed + CTimer::m_FrameCounter)%1000);
			
		//m_nPedFlags.bIsLyingDown = true;
	}
} // end - CPed::SetFall
*/


/*
// Name			:	ClearFall
// Purpose		:	Clears ped fall state.
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearFall()
{
	SetGetUp();
} // end - CPed::ClearFall
*/


/*
// Name			:	Fall
// Purpose		:	Ped falls to the ground
// Parameters	:	None
// Returns		:	Nothing
#ifdef FINAL
	#define FALLING_LOOP_START	(0.0f)
	#define FALLING_LOOP_END	(0.667f)
#else
	float FALLING_LOOP_START = (0.0f);
	float FALLING_LOOP_END = (0.667f);
#endif
//
void CPed::Fall()
{
	if (m_nUnconsciousTimer != -1 && GetIsStanding())
	{
		if (CTimer::GetTimeInMilliseconds() > m_nUnconsciousTimer)
		{
			// We've been on the ground for long enough - get up now
			ClearFall();
		}
	}
	
	if(IsPlayer() && (m_nPedFlags.bKnockedUpIntoAir || m_nPedFlags.bKnockedOffBike) && !GetIsStanding())
	{
		CAnimBlendAssociation *pFallAnim, *pFallingAnim;
		pFallAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL);

		if(pFallAnim && (pFallAnim->GetAnimId()==ANIM_STD_FALL_ONBACK || pFallAnim->GetAnimId()==ANIM_STD_FALL_ONFRONT))
			pFallingAnim = pFallAnim;
		else
			pFallingAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL_ONBACK);
		if(!pFallingAnim)
			pFallingAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL_ONFRONT);

		if(!pFallingAnim && pFallAnim && pFallAnim->GetCurrentTime() > 0.8f*pFallAnim->GetTotalTime() )
//		&&(	pFallAnim->GetAnimId()==ANIM_STD_HIGHIMPACT_FRONT || pFallAnim->GetAnimId()==ANIM_STD_HIGHIMPACT_BACK
//		||	pFallAnim->GetAnimId()==ANIM_STD_SPINFORWARD_LEFT || pFallAnim->GetAnimId()==ANIM_STD_SPINFORWARD_RIGHT))
		{
			if(pFallAnim->IsFlagSet(ABA_PEDFLAG_FORWARD_ANIM))
				CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_FALL_ONFRONT, 8.0f);
			else
				CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_FALL_ONBACK, 8.0f);
		}
		else if(pFallingAnim && pFallingAnim->GetBlendAmount() > 0.3f && pFallingAnim->GetBlendDelta()>=0.0f
		&& pFallingAnim->GetCurrentTime() > FALLING_LOOP_END
		&& pFallingAnim->GetCurrentTime() - pFallingAnim->GetTimeStep() <= FALLING_LOOP_END)
		{
			pFallingAnim->SetCurrentTime(FALLING_LOOP_START);
			pFallingAnim->SetFlag(ABA_FLAG_ISPLAYING);
		}
	}
	else if((m_nPedFlags.bKnockedUpIntoAir || m_nPedFlags.bKnockedOffBike) && GetIsStanding() && !GetWasStanding())
	{
		CAnimBlendAssociation *pAnim;
		if((pAnim=RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL_ONBACK))
		|| (pAnim=RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL_ONFRONT)) )
		{
			// stop doing all this stuff
			m_nPedFlags.bKnockedUpIntoAir = false;
			m_nPedFlags.bKnockedOffBike = false;
			// if the anim's playing want it to finish asap now ped has landed
			pAnim->SetSpeed(3.0f);
			// say OWW YA'BUGGER!
			if(IsPlayer())
				Say(AE_PLAYER_HIT_GROUND_GRUNT);
		}
		else if( (pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL))
		&& !pAnim->IsFlagSet(ABA_FLAG_ISPLAYING))
		{
			m_nPedFlags.bKnockedUpIntoAir = false;
			m_nPedFlags.bKnockedOffBike = false;
			// say oww!
		}
	}
} // end - CPed::Fall
*/

/*
// Name			:	CheckIfInTheAir
// Purpose		:	Check if ped should be set to the in the air state. 
// Parameters	:	None
// Returns		:	Nothing
bool CPed::CheckIfInTheAir()
{
	CVector vecStart = GetPosition();
	float fEndZ = m_mat.tz - PED_INTHEAIROFFSET;
	CColPoint colPt;
	CEntity* pEntity;

	if(!m_nPedFlags.bInVehicle)
	{
		bool8 bGetGround = CWorld::ProcessVerticalLine(vecStart, fEndZ, colPt, pEntity, true, true, false, true, false);
		if(!bGetGround && m_nPedState!=PED_JUMP)
		{	
			vecStart.z -= PED_GROUNDOFFSET;
			if(CWorld::TestSphereAgainstWorld(vecStart, 0.15f, this, true, false, false, false, false)!=NULL)
				bGetGround = true;
		}
		return !bGetGround;
	}
	else
	 return false;
}
*/

/*
// Name			:	SetInTheAir
// Purpose		:	Sets ped in the air state. Checks to see if ped should play panic falling anim
// Parameters	:	None
// Returns		:	Nothing
void CPed::SetInTheAir()
{
	if(!m_nPedFlags.bIsInTheAir)
	{
		m_nPedFlags.bIsInTheAir = 1;
	}
///
	if(!m_nPedFlags.bIsInTheAir)
	{
//		SetStoredState();
//		m_nPedState = PED_INTHEAIR;
		m_nPedFlags.bIsInTheAir = 1;
		CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_FALL_GLIDE, 4.0f);
		if(m_nPedState == PED_ATTACK)
		{
			ClearAttack();
			ClearPointGunAt();
		}
		else if(m_nPedState == PED_FIGHT)
		{
			EndFight();
		}
	}	
////
}
*/

/*
void CPed::InTheAir()
{
	CVector vecStart = CVector(m_mat.tx, m_mat.ty, m_mat.tz);
	CVector vecEnd = CVector(m_mat.tx, m_mat.ty, m_mat.tz - PED_FALLINGOFFSET);
	CColPoint colPt;
	CEntity* pEntity;

	// If moving downwards
	if(m_vecMoveSpeed.z < 0.0f && !m_nPedFlags.bIsPedDieAnimPlaying && m_nPedState!=PED_DIE && m_nPedState!=PED_DEAD)
	{

		// Check if ground is a long way below
		bool bHitGround = CWorld::ProcessLineOfSight(vecStart, vecEnd, colPt, pEntity, true, true, false, true, false);
		if(bHitGround == false)
		{
			if(m_nPedState == PED_ABSEIL_FROM_HELI)
			{	
				//need abseil anim to put in here
				// NOTE: it's now added from the start, but still need to stop fall anim getting added
			}
			else
			{
				CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL);
				if(pAnim == NULL && m_vecMoveSpeed.z<-0.1f)
				{
	//				RpAnimBlendClumpSetBlendDeltas((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL, -1000.0f);
					CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_FALL, 4.0f);
				}
			}	
		}
		else
		{
			// near enough ground and moving down
			if(m_mat.tz - colPt.GetPosition().z < 1.3f || GetIsStanding())
			{
				SetLanding();
			}
		}
	}	
}
*/

/*
void CPed::SetLanding()
{
	if(m_nPedState==PED_DIE || m_nPedState==PED_DEAD)
		return;

	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL);
//	m_nPedState = PED_LANDING;

	// don't want to roll and land under the water (and don't want to get rid of fall anim)
	if(pAnim && m_nPedFlags.bIsDrowning)
		return;
	
	RpAnimBlendClumpSetBlendDeltas((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL, -1000.0f);
	// If found falling anim then get ped to collapse, otherwise land gracefully
	if(pAnim || (GetPedType()==PEDTYPE_COP && m_nPedFlags.bKnockedUpIntoAir))
	{
		pAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_FALL_COLLAPSE);
		DMAudio.PlayOneShot(AudioHandle, AE_PED_COLLAPSE_AFTER_FALL, 1.0f);
		if(IsPlayer())
			Say(AE_PLAYER_HIT_GROUND_GRUNT);
		// this flag was only used temporarily to get swat cops to roll
		if(GetPedType()==PEDTYPE_COP && m_nPedFlags.bKnockedUpIntoAir)
			m_nPedFlags.bKnockedUpIntoAir = false;
	}
	else
	{
		pAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_FALL_LAND);
		DMAudio.PlayOneShot(AudioHandle, AE_PED_LAND_ON_FEET_AFTER_FALL, 1.0f);
	}
	pAnim->SetFinishCallback(PedLandCB, this);
	m_nPedFlags.bIsInTheAir = 0;
	m_nPedFlags.bIsLanding = 1;
}
*/

/*
// Name			:	SetGetUp
// Purpose		:	Sets ped get up state. Ped will get up from the ground
// Parameters	:	None
// Returns		:	Nothing

void CPed::SetGetUp()
{
	if( (m_nPedState!=PED_GETUP || m_nPedFlags.bGetUpAnimStarted==false) && CanSetPedState())
	{
		// don't want to let dead peds getup if possible
		if(m_nHealth < 1.0f && !IsPedHeadAbovePos(-0.3f))
		{
			m_nHealth = 0.0f;
			SetDie(ANIM_STD_NUM);
#ifdef GTA_NETWORK
		if (gGameNet.NetWorkStatus == NETSTAT_SERVER && this->IsPlayer())
		{		// A player has died in a network game. We have to act upon it.
			gGameNet.PlayerPedHasBeenKilledInNetworkGame(this);
			CNetGames::RegisterPlayerKill(this, NULL);				
		}
#endif

			return;
		}
	
		CAnimBlendAssociation *pAnim, *pFallAnim;
		
		if(m_nPedFlags.bUpdateAnimHeading)
		{
			m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
			m_fCurrentHeading -= 0.5*PI;
			m_nPedFlags.bUpdateAnimHeading = false;
		}
		if(m_nPedState != PED_GETUP){
			SetStoredState();
			SetPedState(PED_GETUP);
		}
		CVector vecPedPos = GetPosition();
		CVehicle *pOffendingVehicle = NULL;
		// check for cars around current position
		if( (pOffendingVehicle = (CVehicle *)CPedPlacement::IsPositionClearOfCars( &vecPedPos )) 
		&& pOffendingVehicle->GetBaseVehicleType()!=VEHICLE_TYPE_BIKE && pOffendingVehicle != m_pAttachToEntity
		||  (m_pNOCollisionVehicle && (m_pNOCollisionVehicle->GetIsTypeVehicle() && ((CVehicle *)m_pNOCollisionVehicle)->GetBaseVehicleType()!=VEHICLE_TYPE_BIKE) && ( ((CTimer::m_FrameCounter + RandomSeed + 5)&7)>0 ||
			CCollision::ProcessColModels(m_mat, CModelInfo::GetColModel(m_nModelIndex), m_pNOCollisionVehicle->GetMatrix(), m_pNOCollisionVehicle->GetColModel(), aTempPedColPts)>0 )) )
		{
			m_nPedFlags.bGetUpAnimStarted = false;
			if(IsPlayer())
//				InflictDamage(NULL, WEAPONTYPE_RUNOVERBYCAR, CTimer::GetTimeStep(), PED_SPHERE_CHEST, 0);
				GetPedIntelligence()->AddEvent(new CEventDamage(this, NULL, WEAPONTYPE_RUNOVERBYCAR,  CTimer::GetTimeStep(), PED_SPHERE_CHEST, 0));
			else if(CPad::GetPad(0)->DisablePlayerControls!=0)
//				InflictDamage(NULL, WEAPONTYPE_RUNOVERBYCAR, 1000, PED_SPHERE_CHEST, 0);
				GetPedIntelligence()->AddEvent(new CEventDamage(this, NULL, WEAPONTYPE_RUNOVERBYCAR,  1000, PED_SPHERE_CHEST, 0));
			
			return;	// don't add getup anim if under a car
		}
		else{
			m_nPedFlags.bGetUpAnimStarted = true;
			m_pNOCollisionVehicle = NULL;
			m_nPedFlags.bKnockedUpIntoAir = false;
			m_nPedFlags.bKnockedOffBike = false;
		}
//		RpAnimBlendClumpSetBlendDeltas((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL, -1000.0f);
		// also if sprint anim is there -> want to replace it with run anim to avoid runstops
		
		if((pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_RUNFAST)))
		{
			if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_RUN))
				CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUN, 8.0f);
			else
				CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 8.0f);
			// set sprint anim to autoremove
			pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
		}
		// check if ped is on their front or back		
		if(PEDWAIT_SUN_BATHE_IDLE==m_eWaitState)
		{
		    SetHeadingRate(0.0f);
		    if((m_nPedFlags.bFleeWhenStanding)&&(m_pFleeEntityWhenStanding))
		    {
		        //Awaiting fixed animation - just use standard get up anim for now.
        		//pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_SUNBATHE_PED, ANIM_SUNBATHE_ESCAPE, 4.0f);
    			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_GET_UP, 1000.0f);
	        }
	        else
	        {
	            //Awaiting fixed animation - just use standard get up anim for now.
        		//pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_SUNBATHE_PED, ANIM_SUNBATHE_UP, 4.0f);	            
    			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_GET_UP, 1000.0f);
	        }
		    pAnim->SetFinishCallback(PedGetupCB,this);
		}
		else
		{
    		pFallAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_PEDFLAG_FORWARD_ANIM);
    		if(pFallAnim)
    			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_GET_UP_FRONT, 1000.0f);
    		else
    			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_GET_UP, 1000.0f);
    		pAnim->SetFinishCallback(PedGetupCB, this);
        }


		//pAnim->SetFinishCallback(PedGetupCB, this);
	}
} // end - CPed::SetGetUp
*/

// Name			:	Mug
// Purpose		:	Mugging check
// Parameters	:	None
// Returns		:	Nothing

/*
void CPed::Mug()
{
//	if(!(m_nPedState & PED_ATTACK))

	if (m_pTargetEntity && m_pTargetEntity->GetIsTypePed())
	{
		if(CTimer::GetTimeInMilliseconds() > (m_nAttackTimer-2000))// give the mugger 2 secs to clear off
		{
			SetWanderPath(CGeneral::GetRandomNumber() % 8); // finished the call		
			SetFlee(m_pTargetEntity,20000);
		}
		else
		{
			// if 
			if(CVector(m_pTargetEntity->GetPosition() - GetPosition()).Magnitude() > 3.0f)
			{
				m_nShootRate = 50;
			}
			Say(AE_PED_MUGGING);									// Mugger
			((CPed*)(m_pTargetEntity))->Say(AE_PED_BEING_MUGGED);	// Muggee
		}
	}
	else
	{
		ClearMug();
	}
}
*/

// Name			:	SetLook
// Purpose		:	Sets ped look state, look at position
// Parameters	:	fLookHeading - position to seek
// Returns		:	Nothing

void CPed::SetLook(float fLookHeading)
{
	if(!IsPedInControl())
	{
		return;
	}
	//SetStoredState();

	SetPedState(PED_LOOK_HEADING);
	SetLookFlag(fLookHeading);

} // end - CPed::SetLook


// Name			:	SetLook
// Purpose		:	Sets ped look state, look at entity
// Parameters	:	pLookEntity - ptr to entity to look at
// Returns		:	Nothing

void CPed::SetLook(CEntity* pLookEntity)
{
	if(!IsPedInControl())
	{
		return;
	}
	//SetStoredState();
	SetPedState(PED_LOOK_ENTITY);
	SetLookFlag(pLookEntity);

} // end - CPed::SetLook


// Name			:	SetLookTimer
// Purpose		:	Sets the amount of time a ped looks
// Parameters	:	pLookEntity - ptr to entity to look at
// Returns		:	Nothing

void CPed::SetLookTimer(uint32 nTime)
{
	if(CTimer::GetTimeInMilliseconds() > m_nLookTimer)
		m_nLookTimer = (CTimer::GetTimeInMilliseconds() + nTime);
} // end - CPed::SetLookTimer

/*
// Name			:	SetAttackTimer
// Purpose		:	Sets the amount of time a ped waits before attacking again
// Parameters	:	
// Returns		:	Nothing
void CPed::SetAttackTimer(uint32 nTime)
{
	if(CTimer::GetTimeInMilliseconds() > m_nAttackTimer)
	{
		if(m_nShootTimer > CTimer::GetTimeInMilliseconds())
			m_nAttackTimer = m_nShootTimer + nTime;
		else
			m_nAttackTimer = CTimer::GetTimeInMilliseconds() + nTime;	
	}	
} // end - CPed::SetAttackTimer
*/

/*
// Name			:	SetShootTimer
// Purpose		:	Sets the amount of time a ped attacks for
// Parameters	:	
// Returns		:	Nothing
void CPed::SetShootTimer(uint32 nTime)
{
	if(CTimer::GetTimeInMilliseconds() > m_nShootTimer)
		m_nShootTimer = (CTimer::GetTimeInMilliseconds() + nTime);
} // end - CPed::SetShootTimer
*/

// Name			:	ClearLook
// Purpose		:	Clears ped look state
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearLook()
{
	//RestorePreviousState();
	ClearLookFlag();


} // end - CPed::ClearLook


// Name			:	Look
// Purpose		:	Turn Ped to face something
// Parameters	:	None
// Returns		:	Nothing

void CPed::Look()
{
    TurnBody();
}

bool CPed::TurnBody()
{
	float fDesiredHeading, fCurrentHeadingDegrees;
	bool rt = true;

	if(m_pEntLookEntity != NULL)
	{
		m_fLookHeading=CGeneral::
		    GetRadianAngleBetweenPoints
		            (m_pEntLookEntity->GetPosition().x, m_pEntLookEntity->GetPosition().y,
		             GetPosition().x, GetPosition().y);
	}
	fDesiredHeading = m_fLookHeading;
	fDesiredHeading=CGeneral::LimitRadianAngle(fDesiredHeading);
	fCurrentHeadingDegrees = m_fCurrentHeading;

	if (fCurrentHeadingDegrees + PI < fDesiredHeading)
	{
		fDesiredHeading -= 2*PI;
	}
	else
	{
		if (fCurrentHeadingDegrees - PI > fDesiredHeading)
		{
			fDesiredHeading += 2*PI;
		}
	}

	float fHeadingChange = fCurrentHeadingDegrees - fDesiredHeading;
	m_fDesiredHeading = fDesiredHeading;

	if (ABS(fHeadingChange) > 0.05f)
	{
		fCurrentHeadingDegrees -= fHeadingChange / 5;
		rt = false;
	}
	m_fCurrentHeading = fCurrentHeadingDegrees;
	
	m_fLookHeading = fDesiredHeading;
	
	return rt;

} // end - CPed::TurnBody

/*
// Name			:	SetSeek
// Purpose		:	Sets ped seek state, seek position
// Parameters	:	vecSeekPosition - position to seek
// Returns		:	Nothing

void CPed::SetSeek(CVector vecSeekPosition, float fRange)
{
	ASSERT(m_nPedState != PED_DEAD && m_nPedState != PED_DIE);
//	CVector vecToTarget;
	
//	vecToTarget = GetPosition()-vecSeekPosition;

	if(!IsPedInControl())
	{
		return;
	}

	if((m_nPedState == PED_SEEK_POSITION) 
	&& (m_vecTargetPosition.x == vecSeekPosition.x)
	&& (m_vecTargetPosition.y == vecSeekPosition.y))// z doesn't matter
	{
		return;
	}
	
	//If the ped is following a path then it will already 
	//call the seek() function.  We don't want to interrupt the path 
	//path following so don't change  the ped state.
	if(PED_FOLLOW_PATH==m_nPedState)
	{
	    return;
	}
	
	// if we are in one of those anims where you can't run and fire, then clear the aim flag
	if(CanWeRunAndFireWithWeapon()==false)
		ClearPointGunAt();
	
	if(m_nPedState != PED_SEEK_POSITION) // can follow one seek with another, but don't want to record last seek because the old position will be lost
	{
		SetStoredState();
	}
	SetPedState(PED_SEEK_POSITION);
	m_fArriveRange = fRange;
//	if((m_Objective == RUN_TO_AREA)||(m_nPedFlags.bForceRun))
//		SetMoveState(PEDMOVE_RUN);
//	else
//		SetMoveState(PEDMOVE_WALK);
	
	m_vecTargetPosition = vecSeekPosition;

	// allow Seek() to set the move state, we may already be inside the range
	// this movestate should only be set at the start of the seek
//	SetMoveState(PEDMOVE_STILL);
} // end - CPed::SetSeek
*/


/*
// Name			:	SetSeek
// Purpose		:	Sets ped seek state, seek entity
// Parameters	:	pSeekEntity - ptr to entity to seek
// Returns		:	Nothing

void CPed::SetSeek(CEntity* pSeekEntity, float fRange)
{
	ASSERT(m_nPedState != PED_DEAD && m_nPedState != PED_DIE);
	
	if(!IsPedInControl())
	{
		return;
	}
	
	
	if((m_nPedState == PED_SEEK_ENTITY) && (m_pTargetEntity == pSeekEntity))
	{
		return;
	}
	
	//If the ped is following a path then it will already 
	//call the seek() function.  Don't interrupt the flow 
	//of the path following so don't interrupt the ped state.
	if(m_nPedState==PED_FOLLOW_PATH)
	{
	    return;
	}

	if (pSeekEntity != NULL)
	{
		if(m_nPedState != PED_SEEK_ENTITY) // can follow one seek with another, but don't want to record last seek because the old entity will be lost
		{
			SetStoredState();
		}
		SetPedState(PED_SEEK_ENTITY);
		m_fArriveRange = fRange;
//		if((m_Objective == RUN_TO_AREA)||(m_nPedFlags.bForceRun))
//			SetMoveState(PEDMOVE_RUN);
//		else
//			SetMoveState(PEDMOVE_WALK);

		m_pTargetEntity = pSeekEntity;
		m_pTargetEntity->RegisterReference(&m_pTargetEntity);
//		m_pTargetEntity->SetIsReferenced(true);

		// allow Seek() to set the move state, we may already be inside the range
		SetMoveState(PEDMOVE_STILL);
	}
} // end - CPed::SetSeek
*/


/*
// Name			:	ClearSeek
// Purpose		:	Clears ped seek state
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearSeek()
{
	SetIdle();
	m_nTypeOfSeek=0;
} // end - CPed::ClearSeek
*/


/*
// Name			:	Seek
// Purpose		:	Calculates forces to apply to a ped so that it will move
//					towards its target position
// Parameters	:	None
// Returns		:	TRUE if within PED_TARGET_DISTANCE_THRESHOLD, FALSE otherwise
#define DISTANCE_TO_SLOW_FROM_RUN_TO_STILL (2.0f)
#define DISTANCE_TO_SLOW_FROM_WALK_TO_STILL (1.0f)

bool CPed::Seek()
{
	float fDistance, fDistanceToSpazPoint;
	CVector2D vecTargetVector;
	CVector vecRoughTarget;
	float fArrive = m_fArriveRange;
	float fTargetZ;
	eMoveState NewMoveState;
	CEntity *pHitEntity = NULL;

	vecRoughTarget = m_vecTargetPosition;
	fTargetZ = m_vecTargetPosition.z;
	
	if( m_Objective!=ENTER_CAR_AS_DRIVER && m_nPedState!=PED_EXIT_TRAIN && m_nPedState!=PED_ENTER_TRAIN 
	&&  m_nPedState!=PED_SEEK_BOAT_POSITION && m_Objective!=ENTER_CAR_AS_PASSENGER && m_Objective!=SOLICIT_VEHICLE
	&& !m_nPedFlags.bSeekingCover
	&&  !(m_pObjectivePed && m_pObjectivePed->m_nPedFlags.bInVehicle))
	{
	
		// this check only gets done every 8 (could be more) frames, cause it's costing quite a bit when there's lots of peds about
		if(((CTimer::m_FrameCounter + RandomSeed + 316)&31)==0)
		{
			pHitEntity = CWorld::TestSphereAgainstWorld(vecRoughTarget, 0.4f, NULL, false, true, false, false, false);

			if(pHitEntity)
			{
				// the position we are going to is blocked by a car and we aren't heading towards a car
				if(pHitEntity->GetIsTypeVehicle() && ((CVehicle*)pHitEntity)->GetBaseVehicleType() != VEHICLE_TYPE_BOAT)
				{
					fArrive = pHitEntity->GetColModel().GetBoundBoxMax().y 
							- pHitEntity->GetColModel().GetBoundBoxMin().y;
					fArrive *= 0.55f;
				}
				else
					fArrive = 2.5f;	
			}
		}

	}

	vecTargetVector.x = m_vecTargetPosition.x - GetPosition().x;
	vecTargetVector.y = m_vecTargetPosition.y - GetPosition().y;

	if(m_pTargetEntity == NULL && m_nPedState == PED_SEEK_ENTITY)
		ClearSeek();
	if(m_Objective == FOLLOW_CHAR_IN_FORMATION && m_pObjectivePed==NULL)
	{
		m_Objective = NO_OBJ;
		ClearObjective();
		SetWanderPath(0);//? what to do - better than standing around doing nothing
		return FALSE;
	}
		
	fDistance = vecTargetVector.Magnitude(); 
	if ((fDistance < DISTANCE_TO_SLOW_FROM_RUN_TO_STILL)||(m_Objective == GOTO_AREA_ON_FOOT))
	{
		if(m_Objective == FOLLOW_CHAR_IN_FORMATION)
		{
			if(m_pObjectivePed->GetMoveState() != PEDMOVE_STILL)
			{
				NewMoveState = m_pObjectivePed->GetMoveState();
			}
		}
		else
		{
			NewMoveState = PEDMOVE_WALK;
		}
	}
	else
	{
		if(m_Objective == FOLLOW_CHAR_IN_FORMATION)
		{
			if (fDistance > DISTANCE_TO_SLOW_FROM_RUN_TO_STILL)
			{
				NewMoveState = PEDMOVE_RUN;
			}
			else
			if(m_pObjectivePed->GetMoveState() != PEDMOVE_STILL)
			{
				NewMoveState = m_pObjectivePed->GetMoveState();
			}
		}
		else
		{
			if(m_Objective ==SPRINT_TO_AREA)
			{
				NewMoveState = PEDMOVE_SPRINT;
			}
			else
			if((m_Objective == KILL_CHAR_ANY_MEANS)||(m_Objective == KILL_CHAR_ON_FOOT)
				||(m_Objective == RUN_TO_AREA)||(m_nPedFlags.bForceRun))
			{
				NewMoveState = PEDMOVE_RUN;
			}
			else		
			{
				NewMoveState = PEDMOVE_WALK;
			}
		}
	}
	if((m_nPedState == PED_SEEK_ENTITY)&&(m_pTargetEntity->GetType()==ENTITY_TYPE_PED)&&(((CPed *)m_pTargetEntity)->m_nPedFlags.bInVehicle))
	{
		fArrive += 2.0f;
	}

	CVector *vecFollowPath = SeekFollowingPath();
	
	if(!vecFollowPath)
	{
		if (fDistance < fArrive)

		{
			// We're close enough to the target now
			if((m_Objective == FOLLOW_CHAR_IN_FORMATION)&&(m_pObjectivePed->GetMoveState() != PEDMOVE_STILL))
			{
				NewMoveState = m_pObjectivePed->GetMoveState();
			}
			else
			if(GetMoveState() != PEDMOVE_STILL)
			{
				NewMoveState = PEDMOVE_STILL;
				m_nAntiSpazTimer = 0;
				m_vecAntiSpazVector.x = 0.0f;
				m_vecAntiSpazVector.y = 0.0f;
			}

			
			if((m_Objective == GOTO_AREA_ON_FOOT)||
			   (m_Objective == RUN_TO_AREA)||
			   (m_Objective == SPRINT_TO_AREA)||
			   (m_Objective == GOTO_AREA_ANY_MEANS)||
			   (m_Objective == GOTO_SEAT_ON_FOOT)||
			   (m_Objective == GOTO_ATM_ON_FOOT)||
			   (m_Objective == GOTO_BUS_STOP_ON_FOOT)||
			   (m_Objective == GOTO_PIZZA_ON_FOOT)||
			   (m_Objective == GOTO_SHELTER_ON_FOOT)||	
			   (m_Objective == GOTO_ICE_CREAM_VAN_ON_FOOT)			   		   
			   )
			{
				if(m_NextWanderNode.IsEmpty())// we are aiming for the final postion
				{
					SetScriptObjectivePassedFlag(true);
				}
				else
					m_NextWanderNode.SetEmpty();
					
				m_nPedFlags.bUseSmartSeekAndFlee = true;// switch on smart seeking again
				
			}



			return TRUE;


		}


	}

	if(m_nPedFlags.bForceRun && NewMoveState != PEDMOVE_SPRINT)
		NewMoveState = PEDMOVE_RUN;
	

	
	if (CTimer::GetTimeInMilliseconds() > m_nAntiSpazTimer)
	{
		float fDiff;
		
		if(vecFollowPath)
			m_fDesiredHeading = CGeneral::GetRadianAngleBetweenPoints(vecFollowPath->x, vecFollowPath->y, GetPosition().x, GetPosition().y);
		else				
			m_fDesiredHeading = CGeneral::GetRadianAngleBetweenPoints(m_vecTargetPosition.x, m_vecTargetPosition.y, GetPosition().x, GetPosition().y);

		//why did I put this in??
		fDiff = ABS(m_fDesiredHeading-m_fCurrentHeading);
		if(fDiff > PI) 
			fDiff = TWO_PI - fDiff;
		if(fDiff > HALF_PI)
		{
			if((fDistance < DISTANCE_TO_SLOW_FROM_WALK_TO_STILL)||(fDiff > (HALF_PI+QUARTER_PI)))
			{
				NewMoveState = PEDMOVE_STILL;
			}
			else
			if(fDistance < DISTANCE_TO_SLOW_FROM_RUN_TO_STILL)
			{
				NewMoveState = PEDMOVE_WALK;
			}
			
		}
	}
	else
	{
		if((m_vecAntiSpazVector.x != 0.0f)&&(m_vecAntiSpazVector.y != 0.0f))
		{
			float fDiff;
			
			m_fDesiredHeading = CGeneral::GetRadianAngleBetweenPoints(m_vecAntiSpazVector.x, m_vecAntiSpazVector.y, GetPosition().x, GetPosition().y);

			//why did I put this in??
			fDiff = ABS(m_fDesiredHeading-m_fCurrentHeading);
			if(fDiff > PI) 
				fDiff = TWO_PI - fDiff;
			if(fDiff > HALF_PI)
			{
				if(fDistance < DISTANCE_TO_SLOW_FROM_WALK_TO_STILL)
				{
					NewMoveState = PEDMOVE_STILL;
				}
				else
				if(fDistance < DISTANCE_TO_SLOW_FROM_RUN_TO_STILL)
				{
					if(m_nPedFlags.bForceRun)
						NewMoveState = PEDMOVE_RUN;
					else
						NewMoveState = PEDMOVE_WALK;
				}
				
			}
			
			fDistanceToSpazPoint = CVector2D(CVector2D::ConvertTo2D(GetPosition())-m_vecAntiSpazVector).Magnitude();
			
			if((fDistanceToSpazPoint < 0.5f))//||(fDistance < fDistanceToSpazPoint))
			{
				m_nAntiSpazTimer = 0;
				m_vecAntiSpazVector.x = 0.0f;
				m_vecAntiSpazVector.y = 0.0f;
			}
		}	
	}
	
	if((m_nPedState == PED_FLEE_POSITION)||(m_nPedState == PED_FLEE_ENTITY))
	{
		if(GetMoveState() < NewMoveState)
			SetMoveState(NewMoveState);
	}
	else if(m_nPedState != PED_FOLLOW_PATH)//if we are following a path then the move state has been set.
	{
    	if(m_Objective != GOTO_CHAR_ON_FOOT && m_eWaitState==PEDWAIT_FALSE) // this sets up move speeds based on the char you are following
	    	SetMoveState(NewMoveState);
	}
		
	SetMoveAnim();

	return FALSE;

} // end - CPed::Seek
*/


/*
// Name			:	SetFlee
// Purpose		:	Sets ped flee state, flee position
// Parameters	:	vecFleePosition - position to flee
//					nFleeDuration - time to flee for
// Returns		:	Nothing

void CPed::SetFlee(const CVector2D& vecFleePosition, int32 nFleeDuration)
{
	if (CTimer::GetTimeInMilliseconds() < m_nAntiSpazTimer)
	return;
	
	if(!IsPedInControl() || m_nPedFlags.bStayInSamePlace)
		return;

#ifdef GTA_NETWORK
	if (IsPlayer()) return;
#endif

	if(m_nPedState != PED_FLEE_ENTITY)
	{
		SetStoredState();
	
		SetPedState(PED_FLEE_POSITION);
		SetMoveState(PEDMOVE_RUN);
	
		m_vecFleePosition = vecFleePosition;
	}
	m_nPedFlags.bUseSmartSeekAndFlee = true;
	m_NextWanderNode.SetEmpty();
	
	m_nFleeTimer = (CTimer::GetTimeInMilliseconds() + nFleeDuration);// % MAX_TIMER_MILLISECONDS;
	float Angle1 = CGeneral::GetRadianAngleBetweenPoints(GetPosition().x, GetPosition().y, vecFleePosition.x, vecFleePosition.y);

	m_fDesiredHeading=CGeneral::LimitRadianAngle(Angle1);

	if (m_fCurrentHeading + PI < m_fDesiredHeading)
	{
		m_fDesiredHeading -= 2*PI;
	}
	else
	{
		if (m_fCurrentHeading - PI > m_fDesiredHeading)
		{
			m_fDesiredHeading += 2*PI;
		}
	}

//	m_fCurrentHeading = m_fDesiredHeading;
} // end - CPed::SetFlee
*/


/*
// Name			:	SetFlee
// Purpose		:	Sets ped flee state, flee entity
// Parameters	:	pFleeEntity - ptr to entity to flee
//					nFleeDuration - time to flee for
// Returns		:	Nothing

void CPed::SetFlee(CEntity* pFleeEntity, int32 nFleeDuration)
{
	// debug ************
//	SetObjective(SET_LEADER, FindPlayerPed()); 
//	return;
	// debug ************

	if(!IsPedInControl() || m_nPedFlags.bStayInSamePlace)
		return;

#ifdef GTA_NETWORK
	if (IsPlayer()) return;
#endif

	ASSERT(m_nPedState != PED_DRIVING);

	if(pFleeEntity != NULL)
	{
		SetStoredState();
		
		SetPedState(PED_FLEE_ENTITY);
		m_nPedFlags.bUseSmartSeekAndFlee = true;
		
		SetMoveState(PEDMOVE_RUN);

		m_pFleeEntity = pFleeEntity;
		m_pFleeEntity->RegisterReference(&m_pFleeEntity);
		
		if(nFleeDuration > 0)
			m_nFleeTimer = (CTimer::GetTimeInMilliseconds() + nFleeDuration);
		else
			m_nFleeTimer = 0;
		
		float Angle1 = CGeneral::GetRadianAngleBetweenPoints(GetPosition().x, GetPosition().y, pFleeEntity->GetPosition().x, pFleeEntity->GetPosition().y);

		m_fDesiredHeading=CGeneral::LimitRadianAngle(Angle1);

		if (m_fCurrentHeading + PI < m_fDesiredHeading)
		{
			m_fDesiredHeading -= 2*PI;
		}
		else
		{
			if (m_fCurrentHeading - PI > m_fDesiredHeading)
			{
				m_fDesiredHeading += 2*PI;
			}
		}
	}
	
} // end - CPed::SetFlee
*/


/*
// Name			:	ClearFlee
// Purpose		:	Clears ped flee state
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearFlee()
{
	RestorePreviousState();
	m_nPedFlags.bUseSmartSeekAndFlee = false;
	m_nChatTimer = 0;	// used for node flee
	m_nFleeTimer = 0;
} // end - CPed::ClearFlee
*/

/*
// Name			:	Flee
// Purpose		:	Calculates forces to apply to a ped so that it will move
//					away from its target
// Parameters	:	None
// Returns		:	Nothing
#define SAFE_FLEE_DISTANCE (30.0f)
#define EXTRA_FLEE_TIME (5000)

void CPed::Flee()
{
	float Angle1;
//	Int32 nNode;
	
//	SetMoveState(PEDMOVE_RUN);
	// do end of flee if safe stuff

	if( CTimer::GetTimeInMilliseconds() > m_nFleeTimer && m_nFleeTimer > 0)
	{
		bool8 bSafe = TRUE;
		// if fleeing entity - check ped has reached safe range from threat entity
		if(m_nPedState == PED_FLEE_ENTITY){
			float range = (GetPosition().x - ms_vec2DFleePosition.x)*(GetPosition().x - ms_vec2DFleePosition.x) 
						+ (GetPosition().y - ms_vec2DFleePosition.y)*(GetPosition().y - ms_vec2DFleePosition.y);
			
			if(range < SAFE_FLEE_DISTANCE*SAFE_FLEE_DISTANCE)
				bSafe = FALSE;
		}
		// only clear flee if safe to do so	
		if(bSafe)
		{
		    m_nPedFlags.bScreamWhenFleeing=false;
		
			eMoveState oldMoveState = m_eMoveState;
			ClearFlee();
			if((m_Objective == FLEE_CHAR_ON_FOOT_TILL_SAFE)||(m_Objective == FLEE_CHAR_ON_FOOT_ALWAYS))
			{
				m_nPedFlags.bBeingChasedByPolice = false;
				RestorePreviousObjective();
			}
			
			// if I'm not doing anything important
			if( (m_nPedState==PED_IDLE || m_nPedState==PED_WANDER_PATH) && CGeneral::GetRandomNumber()&1)
			{
				if(oldMoveState > PEDMOVE_WALK)
					SetWaitState(PEDWAIT_FINISH_FLEE);	// have a rest
				else{
					uint32 nLookTime = 2000;
					SetWaitState(PEDWAIT_CROSS_ROAD_LOOK);	// look about briefly
				}
			}
			return;
		}
		else
		{	// else flee some more
			m_nFleeTimer = CTimer::GetTimeInMilliseconds() + EXTRA_FLEE_TIME;
		}
	}
	
	if(m_nPedFlags.bScreamWhenFleeing)
	{
    	if(((CTimer::m_FrameCounter + RandomSeed) & 7) == 0)
    	{
    	    Say(AE_PED_PANIC_SCREAM);
	        m_nPedFlags.bScreamWhenFleeing=false;
	    }
	}
	

// use a modified version of wander path code to flee via nodes
	if(m_nPedFlags.bUseSmartSeekAndFlee)
	{
		CNodeAddress TempPathNode;
		CNodeAddress PrevPathNode;
		int8 i = 0;
		bool8 bFound = FALSE;
		bool8 bReachedNode = FALSE;
		// get the path node direction we wish to flee in
		uint8 nFleeDirn = 9;
		uint8 nNewDirn = 0;
		
		if(m_nAntiSpazTimer < CTimer::GetTimeInMilliseconds() &&
			m_nCollisionTimer < CTimer::GetTimeInMilliseconds())
		{
			if( (!m_NextWanderNode.IsEmpty()) && ThePaths.IsRegionLoaded(m_NextWanderNode) && CTimer::GetTimeInMilliseconds() > m_nChatTimer)
			{
				nFleeDirn = CGeneral::GetNodeHeadingFromVector(GetPosition().x - ms_vec2DFleePosition.x, GetPosition().y - ms_vec2DFleePosition.y);
				if(m_nPathWanderDirection < nFleeDirn)
					m_nPathWanderDirection += 8;
				uint8 nDiff = m_nPathWanderDirection - nFleeDirn;
				if(nDiff > 2 && nDiff < 6){	// fleeing toward threat
					PrevPathNode.SetEmpty();
					m_LastWanderNode = m_NextWanderNode;
					m_NextWanderNode.SetEmpty();
					bReachedNode = FALSE;
				}
			}

			if(!m_NextWanderNode.IsEmpty())
			{
				// set our target position to the coords of the node
				m_vecTargetPosition = ThePaths.TakeWidthIntoAccountForWandering(m_NextWanderNode, RandomSeed);
				// set force run flag to avoid 'seek()' setting MoveState to walk if we want to run!
				if(GetMoveState() == PEDMOVE_RUN)
					m_nPedFlags.bForceRun = true;
				
				eMoveState FixMoveState = GetMoveState();
				if(Seek()){	// if arrived at node
					PrevPathNode = m_LastWanderNode;
					m_LastWanderNode = m_NextWanderNode;
					m_NextWanderNode.SetEmpty();
					bReachedNode = TRUE;
				}
				// reset flag
				m_nPedFlags.bForceRun = false;
				// restore MoveState incase seek has altered it
				SetMoveState(FixMoveState);
			}
		}

		if(m_NextWanderNode.IsEmpty())	// should only happen at start of flee
		{
		
			if(nFleeDirn == 9) // hasn't been calculated yet
				nFleeDirn = CGeneral::GetNodeHeadingFromVector(GetPosition().x - ms_vec2DFleePosition.x, GetPosition().y - ms_vec2DFleePosition.y);
			ThePaths.FindNextNodeWandering(PFGRAPH_PEDS, GetPosition(), &m_LastWanderNode, &m_NextWanderNode, nFleeDirn, &nNewDirn);

			if(nFleeDirn < nNewDirn)
				nFleeDirn += 8;
			uint8 nDiff = nFleeDirn - nNewDirn;

			// check for single nodes or fleeing back towards threat
			if(m_NextWanderNode.IsEmpty() || m_NextWanderNode==PrevPathNode || m_NextWanderNode==m_LastWanderNode || (nFleeDirn-nNewDirn)==4){
					m_nPedFlags.bUseSmartSeekAndFlee = false;
					SetMoveState(PEDMOVE_RUN);
					Flee();
					return;
			}

			// found a node - set it as start of the path
			m_nPathWanderDirection = nNewDirn;

			// give a minimum gap between checking direction is ok
			m_nChatTimer = CTimer::GetTimeInMilliseconds() + 2000;
		}	
	}
	else
	{
	// if fleeing from entity, need to keep changing position, otherwise keep fleeing in same direction
		if((m_nPedState == PED_FLEE_ENTITY || m_nPedState==PED_ON_FIRE) && m_nAntiSpazTimer < CTimer::GetTimeInMilliseconds())
		{
			Angle1 = CGeneral::GetRadianAngleBetweenPoints(GetPosition().x, GetPosition().y, ms_vec2DFleePosition.x, ms_vec2DFleePosition.y);

			m_fDesiredHeading=CGeneral::LimitRadianAngle(Angle1);
		
			if (m_fCurrentHeading + PI < m_fDesiredHeading)
			{
				m_fDesiredHeading -= 2*PI;
			}
			else
			{
				if (m_fCurrentHeading - PI > m_fDesiredHeading)
				{
					m_fDesiredHeading += 2*PI;
				}
			}
		}

	// if collision timer active, need to introduce obstacle avoidance to heading
	// new direction produced is a combination of original desired heading, vector from centre of
	// collision entity, and collision normal (because buildings are odd, often concave, shapes)
		if(CTimer::GetTimeInMilliseconds() < m_nCollisionTimer && m_pCollisionEntity)
		{
			// get fraction of obstacle avoidance direction to apply as function of remaining time
			float fAvoidFraction = 2.0*(m_nCollisionTimer - CTimer::GetTimeInMilliseconds())/(float)COLLISION_AVOID_TIME_FLEE;
			
			// for first 1/4 just use collision normal heading
			if(fAvoidFraction > 1.5){
				m_fDesiredHeading = CGeneral::GetRadianAngleBetweenPoints(m_vecDamageNormal.x, m_vecDamageNormal.y, 0.0f, 0.0f);
				m_fDesiredHeading = CGeneral::LimitRadianAngle(m_fDesiredHeading);
			}
			else
			{
				// desired heading to flee from collision entity Centre position
				float fEntityRetreatHeading = CGeneral::GetRadianAngleBetweenPoints(GetPosition().x, GetPosition().y, m_pCollisionEntity->GetPosition().x, m_pCollisionEntity->GetPosition().y);
				fEntityRetreatHeading=CGeneral::LimitRadianAngle(fEntityRetreatHeading);
				// desired heading to retreat from immediate collision
				float fColRetreatHeading = CGeneral::GetRadianAngleBetweenPoints(m_vecDamageNormal.x, m_vecDamageNormal.y, 0.0f, 0.0f);
				fColRetreatHeading=CGeneral::LimitRadianAngle(fColRetreatHeading);
				// get them in the same 180 deg
				if(fEntityRetreatHeading + PI < fColRetreatHeading)
				{
					fColRetreatHeading -= 2*PI;
				}
				else if(fEntityRetreatHeading - PI > fColRetreatHeading)
				{
					fColRetreatHeading += 2*PI;
				}
				
				// for second 1/4 use combination of collision normal and collision entity vector
				if(fAvoidFraction > 1.0)
				{
					fAvoidFraction = 2.0*(fAvoidFraction - 1.0);
					m_fDesiredHeading = fAvoidFraction*fColRetreatHeading + (1.0-fAvoidFraction)*fEntityRetreatHeading;
				}
				// for final 1/2 use balanced combination of all 3 headings
				else
				{
					fColRetreatHeading = 0.5*(fColRetreatHeading + fEntityRetreatHeading);
				
					if (m_fDesiredHeading + PI < fColRetreatHeading)
					{
						fColRetreatHeading -= 2*PI;
					}
					else if (m_fDesiredHeading - PI > fColRetreatHeading)
					{
						fColRetreatHeading += 2*PI;
					}
					
					m_fDesiredHeading = fAvoidFraction*fColRetreatHeading + (1-fAvoidFraction)*m_fDesiredHeading;
				}
			}
			
			// finally do that limiting thing to get desired and current heading in same +/- 180
			m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
			if (m_fCurrentHeading + PI < m_fDesiredHeading)
			{
				m_fDesiredHeading -= 2*PI;
			}
			else if (m_fCurrentHeading - PI > m_fDesiredHeading)
			{
				m_fDesiredHeading += 2*PI;
			}
		}
	} // end if(bUseSmartSeekAndFlee) else...
} // end - CPed::Flee
*/



/*
// Name			:	SetWanderPath
// Purpose		:	Sets ped wander path state
// Parameters	:	nDirection - general direction we want to move in (0 to 7)
// Returns		:	TRUE if successful, FALSE otherwise

bool CPed::SetWanderPath(int8 nDirection)
{
	uint8 nNewDirection;

	ASSERT(m_nPedState != PED_DEAD && m_nPedState != PED_DIE);
	ASSERT(!m_nPedFlags.bInVehicle);
#ifndef GTA_NETWORK
	ASSERT(!IsPlayer());
#endif
	if (IsPlayer())
	{
		return false;
	}
	
	if(!IsPedInControl())
	{
		// we can't interrupt the current state
		m_nPathWanderDirection = nDirection;
		m_nPedFlags.bWanderWhenPossible = true;
		return false;
	}

	if(m_nPedFlags.bStayInSamePlace)
	{
		SetIdle();
		return false;
	}
//	SetAnimSpeed(0.5f);
	m_nPathWanderDirection = nDirection;
	if(nDirection == 0)
	{
		nDirection = CGeneral::GetRandomNumberInRange(1, 7);
	}

	// try to find a node in the specified direction
	ThePaths.FindNextNodeWandering(PFGRAPH_PEDS, GetPosition(), &m_LastWanderNode, &m_NextWanderNode, m_nPathWanderDirection, &nNewDirection);
//	ASSERT(m_pNextWanderNode);	// Single path node caused this perhaps
	
// This stuff should be removed ASAP
	while (m_NextWanderNode.IsEmpty())
	{
		m_nPathWanderDirection++;
		m_nPathWanderDirection %= 8;

		// we sometimes get 'holes' in the map data at the moment which can cause a
		// nasty hard to find crash - if DEBUGPEDPATHFINDHACK is set we just set the
		// ped to idle state to avoid the crash for now, otherwise we assert
		if (m_nPathWanderDirection == nDirection)
		{
			DEBUGPEDPATHFINDHACK
			(
				ClearAll();
				SetIdle();
			)
			return FALSE;
			CDebug::DebugMessage("Can't find valid path node, SetWanderPath, Ped.cpp");
		}

		ThePaths.FindNextNodeWandering(PFGRAPH_PEDS, GetPosition(), &m_LastWanderNode, &m_NextWanderNode, m_nPathWanderDirection, &nNewDirection);
	}


	// found a node - set it as start of the path
	m_nPathWanderDirection = nNewDirection;

	ASSERT(m_nPedState != PED_DRIVING);
	SetPedState(PED_WANDER_PATH);
	SetMoveState(PEDMOVE_WALK);
	m_nPedFlags.bForceRun = false;
	return TRUE;
} // end CPed::SetWanderPath
*/

/*
// Name			:	ClearWanderPath
// Purpose		:	Clears ped wander path state
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearWanderPath()
{
	SetIdle();

	for (int32 i = 0; i < PED_NUM_PATHNODES_LOOKAHEAD; i++)
	{
		m_aPathNodeList[i].SetEmpty();
	}

	m_nNumPathNodes = 0;
	m_nCurrentPathNode = 0;
} // end CPed::ClearWanderPath
*/

/*
// Name			:	WanderPath
// Purpose		:	Wander a path supplied by route finding code
// Parameters	:	None
// Returns		:	Nothing

void CPed::WanderPath()
{
	int8 i = 0;
	bool8 bFound = FALSE;
	CNodeAddress TempPathNode;

	if(m_NextWanderNode.IsEmpty())
	{
#ifndef FINAL
		printf("THIS SHOULDN@T HAPPEN TOO OFTEN\n");
#endif
		SetIdle();
		return;
	}
	// try and avoid ped loosing motion
	if(!m_eWaitState && (GetMoveState()==PEDMOVE_STILL || GetMoveState()==PEDMOVE_NONE) )
		SetMoveState(PEDMOVE_WALK);
		
	// set our target position to the coords of the node
	m_vecTargetPosition = ThePaths.TakeWidthIntoAccountForWandering(m_NextWanderNode, RandomSeed);
	m_vecTargetPosition.z += 1.0f;// correct height

	// check if we've arrived at the node
	if (Seek())
	{
		TempPathNode = m_LastWanderNode;
		uint8 nTempRand = (RandomSeed + CTimer::m_FrameCounter*3) % 100;
		uint8 nOrigWanderDirn = m_nPathWanderDirection;
		uint8 nDoubleBackDirn = m_nPathWanderDirection + (m_nPathWanderDirection>3 ? -4 : 4);
		uint8 nNewDirn = 9;
		uint8 i=0;
		uint8 nBestDirn = 9;
		CNodeAddress BestPathNode;
	
		if (nTempRand > 90)
		{
			m_nPathWanderDirection -= 2;
			if (m_nPathWanderDirection < 0)
			{
				m_nPathWanderDirection += 8;
			}
		}
		else if (nTempRand > 80)
		{
			m_nPathWanderDirection += 2;
			m_nPathWanderDirection %= 8;
		}

		m_LastWanderNode = m_NextWanderNode;
		// try to find a node in the specified direction
		ThePaths.FindNextNodeWandering(PFGRAPH_PEDS, GetPosition(), &m_LastWanderNode, &m_NextWanderNode, m_nPathWanderDirection, &nNewDirn);
		
        if(PEDSTAT_SKATER==((CPedModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->GetDefaultPedStats())
        {
            if(!m_NextWanderNode.IsEmpty())
            {
                const CVector vNextNodePos=ThePaths.FindNodePointer(m_NextWanderNode)->GetCoors();
                if(!CPopulation::IsSkateable(vNextNodePos))
                {
                    m_NextWanderNode.SetEmpty();
                }
            }
        }
//		ASSERT(m_pNextWanderNode);	// Single path node caused this perhaps
// This stuff should be removed ASAP
		while (m_NextWanderNode.IsEmpty())
		{
			i++;
			m_nPathWanderDirection++;
			m_nPathWanderDirection %= 8;
			
			// if checked all 8 directions now decide what to do
			if (i>7)
			{
				if(BestPathNode.IsEmpty())
				{
					DEBUGPEDPATHFINDHACK
					(
						ClearAll();
						SetIdle();
					)
					CDebug::DebugMessage("Can't find valid path node, SetWanderPath, Ped.cpp");
					return;
				}
				else
				{
					m_NextWanderNode = BestPathNode;
					nNewDirn = nBestDirn;
				}
			}
			else
			{
				ThePaths.FindNextNodeWandering(PFGRAPH_PEDS, GetPosition(), &m_LastWanderNode, &m_NextWanderNode, m_nPathWanderDirection, &nNewDirn);
				if( (!m_NextWanderNode.IsEmpty()) && nNewDirn==nDoubleBackDirn){
					nBestDirn = nNewDirn;
					BestPathNode = m_NextWanderNode;
					m_NextWanderNode.SetEmpty();
				}
		        
		        if(PEDSTAT_SKATER==((CPedModelInfo*)CModelInfo::GetModelInfo(GetModelIndex()))->GetDefaultPedStats())
                {
                    if(!m_NextWanderNode.IsEmpty())
                    {
                        const CVector vNextNodePos= ThePaths.FindNodePointer(m_NextWanderNode)->GetCoors();
                        if(!CPopulation::IsSkateable(vNextNodePos))
                        {
                            m_NextWanderNode.SetEmpty();
                        }
                    }
                }
			}
		}

		// found a node - set it as start of the path
		m_nPathWanderDirection = nNewDirn;
		
		if(m_LastWanderNode != m_NextWanderNode)
		{
			if(ThePaths.TestForPedTrafficLight(m_LastWanderNode, m_NextWanderNode)) // traffic light here
			{
				SetWaitState(PEDWAIT_TRAFFIC_LIGHTS);
			}
			else if(ThePaths.TestCrossesRoad(m_LastWanderNode, m_NextWanderNode))
			{
				SetWaitState(PEDWAIT_CROSS_ROAD);
			}
			else if(m_NextWanderNode == TempPathNode)
			{
				// path has doubled back on itself
				SetWaitState(PEDWAIT_DOUBLEBACK);
				Say(AE_PED_WHERE_AM_I);
			}
		}
		else
		{	
			m_NextWanderNode = TempPathNode;
			// path has doubled back on itself
			SetWaitState(PEDWAIT_DOUBLEBACK);
			Say(AE_PED_WHERE_AM_I);
		}
	}
} // end - CPed::WanderPath
*/

/*

// Name			:	Avoid
// Purpose		:	Searches through list of close peds, finds closest one,
//					and takes evasive action if necessary
// Parameters	:	None
// Returns		:	Nothing

void CPed::Avoid()
{
	CVector2D vecAvoidVector;
	CPed* pPed = NULL;

	DEBUGPEDSTATE
	(
		CDebug::DebugString("AVOID", 0, 9);
	)
	if(m_pPedStats->m_nTemper > m_pPedStats->m_nFear && m_pPedStats->m_nTemper > 50)
		return;

	// If we haven't made an avoiding movement recently, go on
	if (CTimer::GetTimeInMilliseconds() > m_nAntiSpazTimer)
	{
		// Don't avoid stuff if ped is idle - let moving peds worry about it
		if (GetMoveState() != PEDMOVE_STILL && GetMoveState() != PEDMOVE_NONE)
		{

			// ClosePed list is already sorted every 1/2sec or so, so assume ped[0] is closest
			CPed *pNearestPed = m_apClosePeds[0];

			// Check if we found a close ped
			if ((pNearestPed != NULL)&&(!(pNearestPed->m_nPedState == PED_DEAD))&&(pNearestPed != m_pTargetEntity)&&(pNearestPed != m_pObjectivePed))
			{
				// Check what type of ped the closest ped is - do we want to avoid this ped
				// type (e.g. gang members might not avoid other ped types, instead just bang
				// into them 'cos they're hard as nails)
				if (CPedType::GetPedTypeAvoid(GetPedType()) & CPedType::GetPedFlag(pNearestPed->GetPedType()))
				{
					// OK, we've found a close ped and it's a ped type we want to avoid
					// Next, need to find out if it's close enough and in front of us
					CVector2D vecSpherePos = CVector2D(0.0f, 0.0f);
					CVector2D vecCollisionCheck = CVector2D(0.0f, 0.0f);

					vecSpherePos.x = ANGLE_TO_VECTOR_X(RADTODEG(m_fCurrentHeading));
					vecSpherePos.y = ANGLE_TO_VECTOR_Y(RADTODEG(m_fCurrentHeading));
					vecSpherePos.Normalise();

					// Do a sphere check a little in front of our ped to see if the close ped
					// is in front of us and close
					vecCollisionCheck = (CVector2D::ConvertTo2D(pNearestPed->GetPosition()) -
						CVector2D(GetPosition().x + (vecSpherePos.x * PED_AVOID_LOOKAHEAD_DISTANCE), GetPosition().y + (vecSpherePos.y * PED_AVOID_LOOKAHEAD_DISTANCE)));

					float CollisionMagnitude = vecCollisionCheck.Magnitude();

					if ((CollisionMagnitude <= PED_AVOID_THRESHOLD) && OurPedCanSeeThisOne(pNearestPed))
					{
						m_nAntiSpazTimer = (CTimer::GetTimeInMilliseconds() + PED_ANTI_SPAZ_TIMER + (((RandomSeed + CTimer::m_FrameCounter*3) % 1000) / 5));	// % MAX_TIMER_MILLISECONDS;
						m_fDesiredHeading += QUARTER_PI;//m_fCurrentHeading
						if(!m_nPedFlags.bIsLooking)
						{
							SetLookFlag(pNearestPed);
							SetLookTimer(CGeneral::GetRandomNumberInRange(500, 800));
						}	
						
					}
				}
			}
		}
	}
} // end - CPed::Avoid
*/

/*
CVector* CPed::SeekFollowingPath()
{
	static CVector vecTarg;
	
	if(m_nCurrentPathNode >= m_nNumPathNodes || m_nNumPathNodes == 0)
	{
		return NULL;
	}
	else
	{
	
		vecTarg = ThePaths.FindNodePointer(m_aPathNodeList[m_nCurrentPathNode])->GetCoors();
		
//		if(vecTarg)	This 'if' is old code and should never not happen (Obbe)
		{
			CVector2D vecToTarg;
			
			vecToTarg.x = vecTarg.x - GetPosition().x;
			vecToTarg.y = vecTarg.y - GetPosition().y;
			
			if(vecToTarg.Magnitude() < m_fArriveRange)
			{
				m_nCurrentPathNode++;
				if(m_nCurrentPathNode<m_nNumPathNodes)
				{
				    m_CurrentPathNode=m_aPathNodeList[m_nCurrentPathNode];
				}
			}
		}
		
		if(m_nCurrentPathNode==m_nNumPathNodes)
		{
		    return NULL;
		}

		return &vecTarg;
	}
}
*/

/*
void CPed::InsertPathNode(const int iNodeIndex, const CVector& vPos)
{
    const CVector vDiff=vPos-m_pPathNodeList[iNodeIndex]->GetCoors();
    const float fDistance2=vDiff.MagnitudeSqr();
    if(fDistance2<1.0f)
    {
        return;
    }
    
    int i;
    for(i=PED_NUM_PATHNODES_LOOKAHEAD-1;i>iNodeIndex;i--)
    {
        m_pPathNodeList[i]=m_pPathNodeList[i-1];
    }
    m_pPathNodeList[iNodeIndex]=new CPathNode;
    m_pPathNodeList[iNodeIndex]->SetCoors(vPos);
    if(m_nNumPathNodes<PED_NUM_PATHNODES_LOOKAHEAD)
    {
        m_nNumPathNodes++;
    } 
}
*/

/*
// Name			:	SetFollowPath
// Purpose		:	Sets ped follow path state
// Parameters	:	vecDestination - coord ped wants to get to
// Returns		:	TRUE if path successfully set, FALSE otherwise

bool CPed::SetFollowPath
(const CVector& vecDestination, const float fGotoCoordOnFootFromPathDistance, const eMoveState eGotoCoordOnFootMoveState, CEntity* pEntityToAvoidOnPathFollow, CEntity* pEntityToFollowOnPathFollow, const int iPathFollowTime)
{
  
    //If we are already following a path then only try to follow a new
    //path if either
    //(i)  the destination has moved by a threshold distance.
    //(ii) the timer is still ticking a previous path follow.
  
    if(PED_FOLLOW_PATH==m_nPedState)
    {
        //Decide whether to compute a new path or continue following the current path.
        bool bComputeNewPath=false;
        
        if((pEntityToAvoidOnPathFollow)&&(pEntityToAvoidOnPathFollow!=m_pEntityToAvoidOnPathFollow))
        {
            //If we are to avoid a new entity then compute a new path.
            bComputeNewPath=true;    
        }
        else if((0==pEntityToAvoidOnPathFollow)&&(m_pEntityToAvoidOnPathFollow))
        {
            //If we were previously avoiding an entity but aren't anymore then compute a new path.
            bComputeNewPath=true;
        }
        else if((pEntityToFollowOnPathFollow)&&(pEntityToFollowOnPathFollow!=m_pEntityToFollowOnPathFollow))
        {
            //If we are to follow a new entity then compute a new path.
            bComputeNewPath=true;
        }
        else if((0==pEntityToFollowOnPathFollow)&&(m_pEntityToFollowOnPathFollow))
        {
            //If we were previously following an entity but aren't anymore then compute a new path.
            bComputeNewPath=true;
        }
        else if((pEntityToFollowOnPathFollow)&&(PED_SEEK_POSITION!=m_nPedState))
        {
            //If we are following the same entity then test if the entity has moved.
            //If we are seeking a position then just keep following until the timer runs out.
            const float fDistanceSquared=(pEntityToFollowOnPathFollow->GetPosition()-m_vecGotoCoordDestination).MagnitudeSqr();
            if(fDistanceSquared>1.0f*1.0f)
            {
                bComputeNewPath=true;
            }
        }
        else if((0==pEntityToAvoidOnPathFollow)&&(0==pEntityToFollowOnPathFollow))
        {
            //We aren't avoiding an entity or following an entity.
            //If the final destination has moved then compute a new path. 
            const float fDistanceSquared=(vecDestination-m_vecGotoCoordDestination).MagnitudeSqr();
            if(fDistanceSquared>1.0f*1.0f)
            {
                bComputeNewPath=true;
            }
        }        
        
        if(!bComputeNewPath)
        {
            return false;
        }
    }
    
    //Set the timer.
    m_iPathFollowTimer=CTimer::GetTimeInMilliseconds()+iPathFollowTime;
    
    //Set the entity that is to be avoided by the path.
    m_pEntityToAvoidOnPathFollow=pEntityToAvoidOnPathFollow;
    
    //Set the entity that is to be followed by the path.
    m_pEntityToFollowOnPathFollow=pEntityToFollowOnPathFollow;
    
    //Set the arrive range to 0.5f just in case it has 
    //been set to a stupid number.
    m_fArriveRange=0.5f;
    
    //Take a note of the required final destination and
    //the distance from the destination where the ped will
    //break from the path and the move state during path follow.
    m_vecGotoCoordDestination = !( (pEntityToFollowOnPathFollow) && (ENTITY_TYPE_PED==pEntityToFollowOnPathFollow->GetType()))  ? vecDestination : pEntityToFollowOnPathFollow->GetPosition();
    if((pEntityToFollowOnPathFollow)&&(PED_SEEK_POSITION==m_nPedState))
    {
        m_vecGotoCoordDestination=m_vecTargetPosition;
    }
    m_fGotoCoordOnFootFromPathDistance=fGotoCoordOnFootFromPathDistance >0 ? fGotoCoordOnFootFromPathDistance : GOTO_COORD_ON_FOOT_FROM_PATH_DISTANCE;
    m_eGotoCoordOnFootMoveState=((eGotoCoordOnFootMoveState==PEDMOVE_RUN)||(eGotoCoordOnFootMoveState==PEDMOVE_WALK)) ? eGotoCoordOnFootMoveState : PEDMOVE_WALK; 
        
    //Call the appropriate path follow routine.
    if(!m_pEntityToAvoidOnPathFollow)
    {
        return SetFollowPathStatic();
    }
    else
    {
        return SetFollowPathDynamic();
    }
}
*/
  
/*  
bool CPed::SetFollowPathStatic()
{    
    //Clear the path nodes.
    ClearFollowPath();
    ASSERT(0==m_nNumPathNodes);

    //Test if the ped is near the target and there is a clear line of sight from the ped to the target.
    if((GetPosition()-m_vecGotoCoordDestination).MagnitudeSqr()<
            m_fGotoCoordOnFootFromPathDistance*m_fGotoCoordOnFootFromPathDistance)
    {
        const float fMaxHeightChange=0.5f;
        const int iMaxNoOfSamples=4;
        if(CWorld::IsWanderPathClear(GetPosition(),m_vecGotoCoordDestination,fMaxHeightChange,iMaxNoOfSamples))
        {
           //The ped can now just go straight to the target so we don't need
           //to follow the path any more.
           RestorePreviousState();
           
           //There needs to be some mechanism to get us to the target.
           //If there isn't then set one.
           if(NO_OBJ==m_Objective)
           {
               if(PEDMOVE_RUN==m_eGotoCoordOnFootMoveState)
               {
                   SetObjective(RUN_TO_AREA,m_vecGotoCoordDestination);
               }
               else 
               {
                   SetObjective(GOTO_AREA_ON_FOOT,m_vecGotoCoordDestination);
               }
           }
           
           SetPedState(PED_NONE);
           return true;
        }
    }    
    
    //Find the nodes that will take us toward the destination.
	ThePaths.DoPathSearch
	    (PFGRAPH_PEDS, 
	     GetPosition(),
	     EmptyNodeAddress, 
	     m_vecGotoCoordDestination,
		 &m_aPathNodeList[0], 
		 &m_nNumPathNodes, 
		 PED_NUM_PATHNODES_LOOKAHEAD
		 );
    
 
    
	//If the path search hasn't found any nodes then just
	//go straight to the goal.
    if(0==m_nNumPathNodes)
    {
        //The ped can now just go straight to the target so we don't need
        //to follow the path any more.
        RestorePreviousState();
        
        //There needs to be some mechanism to get us to the target.
        //If there isn't then set one.
        if(NO_OBJ==m_Objective)
        {
            if(PEDMOVE_RUN==m_eGotoCoordOnFootMoveState)
            {
               SetObjective(RUN_TO_AREA,m_vecGotoCoordDestination);
            }
            else 
            {
               SetObjective(GOTO_AREA_ON_FOOT,m_vecGotoCoordDestination);
            }
        }
        
        SetPedState(PED_NONE);
        return true;
    }
    
    if(m_nNumPathNodes>0)
    {
        if(m_aPathNodeList[0]!=m_CurrentPathNode)
        {
           //Remove zeroth node.
           int i;
           for(i=0;i<PED_NUM_PATHNODES_LOOKAHEAD-1;i++)
           {
               m_aPathNodeList[i]=m_aPathNodeList[i+1];
           } 
           m_nNumPathNodes--;
        }
    }
   
    
    //Test if there is a clear line of sight between each node 
    //and the target position.  If  
    //(i)  there is a clear line of sight
    //(ii) the target is within a threshold distance of the node pos
    //(iii)there is no water between the target and the node pos 
    //(iv) there are no sudden height changes between target and node pos.
    //then the ped will walk to the target on foot.
    int i;
    for(i=0;i<m_nNumPathNodes;i++)
    {
        const CVector& vNodePos= ThePaths.FindNodePointer(m_aPathNodeList[i])->GetCoors();
        const CVector& vEndPos=m_vecGotoCoordDestination;
        
        //Test that the node pos is close to the destination pos. 
        if((vNodePos-vEndPos).MagnitudeSqr()<
            m_fGotoCoordOnFootFromPathDistance*m_fGotoCoordOnFootFromPathDistance)
        {
            //Test that there is a clear line of sight from the node pos to the target pos.
            const float fMaxHeightChange=0.5f;
            const int iMaxNoOfSamples=4;
            if(CWorld::IsWanderPathClear(vNodePos, vEndPos, fMaxHeightChange,iMaxNoOfSamples))
            {  
                m_nNumPathNodes=i+1;
                break;
            }
        }
    }
    
   
        
    m_nCurrentPathNode = 0;  
    if(!m_CurrentPathNode.IsEmpty())
    {
        //Look for the current path node in the list
        //and start the path follow at this node.
        for(i=0;i<m_nNumPathNodes;i++)
        {       
            if(m_aPathNodeList[i]==m_CurrentPathNode)
            {
                m_nCurrentPathNode=i;
                break;
            }
        }
    }
  	m_CurrentPathNode=m_aPathNodeList[m_nCurrentPathNode];

 	//SetStoredState(); 
    //m_nPedStoredState=m_nPedState;
    ePedState storedState=m_nPedStoredState;
	m_nPedStoredState=PED_NONE;
	SetStoredState();
	if(PED_NONE==m_nPedStoredState)
	{
	    m_nPedStoredState=storedState;
	}
	SetPedState(PED_FOLLOW_PATH);
	SetMoveState(m_eGotoCoordOnFootMoveState);
	return true;
} // end - CPed::SetFollowPath()
*/

/*
bool CPed::SetFollowPathDynamic()
{    

    int i;
    
    //Get the bounding box in body space
    const CVector vBodyMin=
         (CVector(-PED_NOMINAL_RADIUS,-PED_NOMINAL_RADIUS,0) + m_pEntityToAvoidOnPathFollow->GetColModel().GetBoundBoxMin());
    const CVector vBodyMax=
         (CVector(+PED_NOMINAL_RADIUS,+PED_NOMINAL_RADIUS,0) + m_pEntityToAvoidOnPathFollow->GetColModel().GetBoundBoxMax());
  
    //Compute the four corners of the bounding box in the horizontal plane
    const float z=GetPosition().z;
    CVector bodyCorners[4]=
        {CVector(vBodyMin.x,vBodyMin.y,0),
         CVector(vBodyMax.x,vBodyMin.y,0),
         CVector(vBodyMax.x,vBodyMax.y,0),
         CVector(vBodyMin.x,vBodyMax.y,0)};
         
   
   //If the car is upside down then swap the corners.
   if(m_pEntityToAvoidOnPathFollow->GetIsTypeVehicle())
   {
       CVehicle* pVehicle=(CVehicle*)m_pEntityToAvoidOnPathFollow;
       if(pVehicle->IsUpsideDown())
       {
            CVector tmp;
            
            tmp=bodyCorners[1];
            bodyCorners[1]=bodyCorners[0];
            bodyCorners[0]=tmp;
            
            tmp=bodyCorners[3];
            bodyCorners[3]=bodyCorners[2];
            bodyCorners[2]=tmp;
       }
   }
  
    //Transform corners into world space.  
    const CMatrix& mat=m_pEntityToAvoidOnPathFollow->m_mat;
    CVector corners[4];
    for(i=0;i<4;i++)
    {
        corners[i]=mat*bodyCorners[i];
        corners[i].z=GetPosition().z;
    }
        
   //Compute the four bounding planes of the box 
   //(the ith plane contains (i-1)th and ith corners).
   CVector normals[4];
   float ds[4];   
   CVector v0=corners[3];
   for(i=0;i<4;i++)
   {
      const CVector& v1=corners[i];
      CVector w=v1-v0;
      w.Normalise();
      normals[i]=CVector(w.y,-w.x,0);
      ds[i]=-DotProduct(v0,normals[i]);
      v0=v1;    
   }
   
   //Intersect edge from start to target and see if there is a component 
   //lying inside or if the whole edge lies outside
   CVector vPos1;
   CVector vPos2;
   bool bLiesOutside=false;
   {
       vPos1=GetPosition();
       vPos2=m_vecGotoCoordDestination;
       CVector w=vPos2-vPos1;
       w.Normalise();
       CVector v=vPos1;
       const float fTolerance=0.1f;
      
       int i;
       for(i=0;i<4;i++)
       {
           const float fSide1=DotProduct(normals[i],vPos1)+ds[i];
           const int iSide1=fSide1 > fTolerance ? 1 : fSide1 < -fTolerance ? -1 : 0;
           const float fSide2=DotProduct(normals[i],vPos2)+ds[i];
           const int iSide2=fSide2 > fTolerance ? 1 : fSide2 < -fTolerance ? -1 : 0; 
           
           if((-1!=iSide1)&&(-1!=iSide2))
           {
              //Either 
              //(i)  both points lie outside the plane.
              //(ii) one point lies on the plane and one lies outside the plane.
              //(iii)both points lie on the plane.   
              //The entire edge can be said to lie outside.
              bLiesOutside=true; 
              
              //Move points on plane just outside the plane 
              if(0==iSide1)
              {
                  //Move vPos1 just onto the outside of the plane.   
                  vPos1+=normals[i]*(-fSide1+fTolerance);
                  const float fNewDist1=DotProduct(vPos1,normals[i])+ds[i];
              }
              if(0==iSide2)
              {
                 //Move vPos2 just onto the outside of the plane.,
                 vPos2+=normals[i]*(-fSide2+fTolerance);
                 const float fNewDist2=DotProduct(vPos2,normals[i])+ds[i];
              }          
           }
           else if((iSide1==-1)&&(iSide2==1))
           {
               const float t=-(DotProduct(normals[i],v)+ds[i])/DotProduct(normals[i],w);
               vPos2=v+w*t;
           }
           else if((iSide1==1)&&(iSide2==-1))
           {
               const float t=-(DotProduct(normals[i],v)+ds[i])/DotProduct(normals[i],w);
               vPos1=v+w*t;
           }      
       }
   }
   
   
   if(!bLiesOutside)
   {
       //Compute the bounding circle.
       CVector vCentre(0,0,0);
       for(i=0;i<4;i++)
       {
           vCentre+=corners[i];
       }
       vCentre*=0.25f;
       //float fRadius=FLT_MAX;
       float fRadius=0;
       for(i=0;i<4;i++)
       {
          const float fRadiusSquared=(corners[i]-vCentre).MagnitudeSqr();
          if(fRadiusSquared>fRadius)
          {
              fRadius=fRadiusSquared;
          }
       }
       fRadius=CMaths::Sqrt(fRadius);
       fRadius*=1.1f;
       CColSphere sphere;
       sphere.Set(fRadius,vCentre);
       
       //Intersect the ray from source to target with bounding circle.
       CVector w=m_vecGotoCoordDestination-GetPosition();
       w.z=0;
       if(w.Magnitude()==0)
       {
            return false;
       }
       w.Normalise();
       CVector v=GetPosition();
       if(!sphere.IntersectRay(v,w,vPos1,vPos2))
       {
           //There must be a clear path to the target so set the 
           //timer back to zero and restore the previous state if the 
           //ped is already following a path.
           m_iPathFollowTimer=0;
           if(PED_FOLLOW_PATH==m_nPedState)
           {
               RestorePreviousState();
           }
           return false;
       }
   }
   
   //Find the crossed planes between source and target positions.
   int iPlane1=-1;
   int iPlane2=-1;
   CVector v=vPos1;
   CVector w=vPos2-vPos1;
   w.Normalise();
   for(i=0;i<4;i++)
   {
       const float ffSide1=DotProduct(normals[i],vPos1)+ds[i];
       const float ffSide2=DotProduct(normals[i],vPos2)+ds[i];
       if((ffSide1>0)&&(ffSide2<0))
       {
           const float t=-(ds[i]+DotProduct(normals[i],v))/DotProduct(normals[i],w);
           vPos1=v+w*t;
           iPlane1=i;
       }
       else if((ffSide1<0)&&(ffSide2>0))
       {
           const float t=-(ds[i]+DotProduct(normals[i],v))/DotProduct(normals[i],w);
           vPos2=v+w*t;
           iPlane2=i;
       }
   }
   
   if((iPlane2<0)||(iPlane1<0))
   {
       //There must be a clear path to the target so set the 
       //timer back to zero and restore the previous state.
       m_iPathFollowTimer=0;
       if(PED_FOLLOW_PATH==m_nPedState)
       {
          RestorePreviousState();
       }
       return false;
   }
   
   //Compute the target and start corners.
   const int iTargetCorner1=(iPlane2+4-1)%4;
   const int iTargetCorner2=(iPlane2+4-0)%4;
   const int iStartCorner1=(iPlane1+4-1)%4;
   const int iStartCorner2=(iPlane1+4-0)%4;
   
   //The two paths are 
   //(i)  start pos -> start corner 1 -> target corner 2 -> target pos
   //(ii) start pos -> start corner 2 -> target corner 1 -> target pos 
   
   //Compute path 1.

   CVector vPath1[5];
   int iPath1Size=0;

   {
       vPath1[iPath1Size]=GetPosition();
       iPath1Size++;
       
       vPath1[iPath1Size]=corners[iStartCorner1];
       iPath1Size++;
       
       int iStart1=iStartCorner1;
       while(iStart1!=iTargetCorner2)
       {
           iStart1=(iStart1+4-1)%4;
           vPath1[iPath1Size]=corners[iStart1];
           iPath1Size++;
       }
       
       vPath1[iPath1Size]=m_vecGotoCoordDestination;
       iPath1Size++;
   }
 
   //Compute path 2
   
   CVector vPath2[5];
   int iPath2Size=0;
     
   {
       vPath2[iPath2Size]=GetPosition();
       iPath2Size++;
       
       vPath2[iPath2Size]=corners[iStartCorner2];
       iPath2Size++;
       
       int iStart2=iStartCorner2;
       while(iStart2!=iTargetCorner1)
       {
           iStart2=(iStart2+4+1)%4;
           vPath2[iPath2Size]=corners[iStart2];
           iPath2Size++;
       }
           
       vPath2[iPath2Size]=m_vecGotoCoordDestination;
       iPath2Size++;
   }
   
   //Test each path for obstructions.
   
   CVector vCentre(0,0,0);
   int k;
   for(k=0;k<4;k++)
   {
       vCentre+=corners[k];
   }
   vCentre*=0.25f;
   
   CEntity* pHitEntity1=0;
   int iPath1Block=iPath1Size;
   bool bIsPath1Clear=true;
   {
       CVector v0=vPath1[0];
       for(i=1;i<iPath1Size;i++)
       {
       
           const CVector& v1=vPath1[i];
           
           CVector vDir=v1-v0;
           vDir.z=0;
           vDir.Normalise();
           vDir*=0.5f;
           vDir=CVector(vDir.y,-vDir.x,0);
           if(DotProduct(vDir,v1-vCentre)<0)
           {
                vDir*=-1;
           }
           CVector v0prime=v0+vDir;
           CVector v1prime=v1+vDir;
                      
           CColPoint colPoint;
           bool bHit=CWorld::ProcessLineOfSight(v0,v1,colPoint,pHitEntity1,true,true,true,true,false,false,false);
           if(!bHit)
           {
                bHit=CWorld::ProcessLineOfSight(v0prime,v1prime,colPoint,pHitEntity1,true,true,true,true,false,false,false);
           }
           
           if(bHit)
           {
                if((pHitEntity1!=m_pEntityToAvoidOnPathFollow)&&(pHitEntity1!=this)&&(pHitEntity1!=m_pTargetEntity))
                {
                    if(ENTITY_TYPE_PED==pHitEntity1->GetType())
                    {
                        //The entity is a ped.
                        if(((CPed*)pHitEntity1)->GetPedState()==PED_IDLE)
                        {
                            //The hit ped is idle so the path is blocked.
                            iPath1Block=i;
                            bIsPath1Clear=false; 
                            break;  
                        }
                        else if(DotProduct(v1-v0,pHitEntity1->m_mat.GetForward())<0)
                        {
                            //The hit ped isn't idle but is walking towards this ped so the path is blocked.
                            iPath1Block=i;
                            bIsPath1Clear=false;   
                            break;
                        }
                        else if(((CPed*)pHitEntity1)->m_pObjectivePed==this)
                        {
                            //The hit ped is following this ped. Move away from hit ped.
                            iPath1Block=i;
                            bIsPath1Clear=false;   
                            break;
                        }
                    }
                    else
                    {
                         iPath1Block=i;
                         bIsPath1Clear=false;
                         break;
                    }
                }
                else
                {
                    pHitEntity1=0;
                }
           }
           
           v0=v1;    
       }
   }
   
   CEntity* pHitEntity2=0;
   int iPath2Block=iPath2Size;
   bool bIsPath2Clear=true;
   {
       CVector v0=vPath2[0];
       for(i=1;i<iPath2Size;i++)
       {
           const CVector& v1=vPath2[i];
           
           CVector vDir=v1-v0;
           vDir.z=0;
           vDir.Normalise();
           vDir*=0.5f;
           vDir=CVector(vDir.y,-vDir.x,0);
           if(DotProduct(vDir,v1-vCentre)<0)
           {
                vDir*=-1;
           }
           CVector v0prime=v0+vDir;
           CVector v1prime=v1+vDir;
           
           CColPoint colPoint;
           bool bHit=CWorld::ProcessLineOfSight(v0,v1,colPoint,pHitEntity2,true,true,true,true,false,false,false);
           if(!bHit)
           {
                bHit=CWorld::ProcessLineOfSight(v0prime,v1prime,colPoint,pHitEntity2,true,true,true,true,false,false,false);
           }
           
           if(bHit)
           {
                if((pHitEntity2!=m_pEntityToAvoidOnPathFollow)&&(pHitEntity2!=this)&&(pHitEntity2!=m_pTargetEntity))
                {
                    if(ENTITY_TYPE_PED==pHitEntity2->GetType())
                    {
                        //The entity is a ped.
                        if(((CPed*)pHitEntity2)->GetPedState()==PED_IDLE)
                        {
                            //The hit ped is idle so the path is blocked.
                            iPath2Block=i;
                            bIsPath2Clear=false;   
                            break;
                        }
                        else if(DotProduct(v1-v0,pHitEntity2->m_mat.GetForward())<0)
                        {
                            //The hit ped isn't idle but is walking towards this ped so the path is blocked.
                            iPath2Block=i;
                            bIsPath2Clear=false; 
                            break;  
                        }
                        else if(((CPed*)pHitEntity2)->m_pObjectivePed==this)
                        {
                            //The hit ped is following this ped. Move away from hit ped.
                            iPath2Block=i;
                            bIsPath2Clear=false;   
                            break;
                        }
                    }
                    else
                    {
                        iPath2Block=i;
                        bIsPath2Clear=false;
                        break;
                    }
                }
                else
                {
                    pHitEntity2=0;
                }
           }
           
           v0=v1;    
       }
   }
   
   //Compute the length of each path.
   float fLength1=0;
   for(i=0;i<iPath1Size-1;i++)
   {
       fLength1+=(vPath1[i+1]-vPath1[i+0]).Magnitude();
   }
   float fLength2=0;
   for(i=0;i<iPath2Size-1;i++)
   {
       fLength2+=(vPath2[i+1]-vPath2[i+0]).Magnitude();
   }    
   
   int iChosenPath=-1;
   if(bIsPath1Clear&&bIsPath2Clear)
   {
       //Both paths are clear so choose the shortest path.
       iChosenPath = fLength1 < fLength2 ? 1 : 2;
   }   
   else if(bIsPath1Clear)
   {
       //Path 1 clear but path 2 blocked.
       iChosenPath = 1;
   }
   else if(bIsPath2Clear)
   {
       //Path 2 clear but path 1 blocked.
       iChosenPath = 2;
   }
   else
   {
       if((1==iPath1Block)&&(iPath2Block>1))
       {
           //Path 1 has a block on the way to the first node
           //so choose path 2 which is clear at least to the first corner.
           iChosenPath = 2;
       }
       else if((iPath1Block>1)&&(1==iPath2Block))
       {
           //Path 2 has a block on the way to the first node
           //so choose path 1 which is clear at least to the first corner.
           iChosenPath = 1;
       }
       else if(pHitEntity1==pHitEntity2)
       {
           //The same entity blocks both paths so choose the shortest path.
           iChosenPath = fLength1 < fLength2 ? 1 : 2;
       }
       else
       {
           //Choose the path with the smallest blockage.
           const float fRadiusBlockage1=
               pHitEntity1->GetColModel().m_sphereBound.m_fRadius;
           const float fRadiusBlockage2=
               pHitEntity2->GetColModel().m_sphereBound.m_fRadius;       
           iChosenPath = fRadiusBlockage1 < fRadiusBlockage2 ? 1 : 2;
       }
   }
       
   if(1==iChosenPath)
   {
       ClearFollowPath();
       i=1;
       while((i<iPath1Size-1))
       {
ASSERT(0);		// This stuff needs some more code (Obbe)
           CPathNode& node=m_dynamicPathNodes[m_nNumPathNodes];
           CPathNode* pNode=&node;
           pNode->SetCoors(vPath1[i]);
 //          m_aPathNodeList[m_nNumPathNodes]=pNode;
           m_nNumPathNodes++;
           i++;
       }
   }
   else if(2==iChosenPath)
   {
       ClearFollowPath();
       i=1;
       while((i<iPath2Size-1))
       {
ASSERT(0);		// This stuff needs some more code (Obbe)
           CPathNode& node=m_dynamicPathNodes[m_nNumPathNodes];
           CPathNode* pNode=&node;
           pNode->SetCoors(vPath2[i]);
//           m_aPathNodeList[m_nNumPathNodes]=pNode;
           m_nNumPathNodes++;
           i++;
       }
   }
   
   if(0==m_nNumPathNodes)
   {
       m_iPathFollowTimer=0;
       if(PED_FOLLOW_PATH==m_nPedState)
       {
           RestorePreviousState();
       }
       return false;
   }
   
   //SetStoredState();
   //m_nPedStoredState=m_nPedState;
    ePedState storedState=m_nPedStoredState;
	m_nPedStoredState=PED_NONE;
	SetStoredState();
	if(PED_NONE==m_nPedStoredState)
	{
	    m_nPedStoredState=storedState;
	}
   SetPedState(PED_FOLLOW_PATH);
   SetMoveState(m_eGotoCoordOnFootMoveState);
   return true;
}
*/

/*
bool CPed::SetFollowPathDynamic
(const CVector& vecDestination, const float fSearchRadius, const eMoveState eGotoCoordOnFootMoveState)
{
    //Clear the wander nodes.
    ClearWanderPath();
    ASSERT(0==m_nNumPathNodes);
    
    //Test if we are close to the destination.  If the ped 
    //is near the destination then we have finished the 
    //path following.
    const float fDistanceSquared=(GetPosition()-vecDestination).MagnitudeSqr();
    if(fDistanceSquared<1.0f*1.0f)
    {
        RestorePreviousState();
        return;
    }
    
    //Compute the path nodes that we will follow.
    m_pFlowPathFinder->ComputePathNodes
        (GetPosition(),vecDestination,
         fSearchRadius,1.0f,
         m_pPathNodeList,m_nNumPathNodes);
    
    if(0==m_nNumPathNodes)
    {
        RestorePreviousState();
        return;
    }
    
    m_vecGotoCoordDestination=vecDestination;
    m_eGotoCoordOnFootMoveState=eGotoCoordOnFootMoveState; 
    m_nPedState=PED_FOLLOW_PATH;
	SetMoveState(m_eGotoCoordOnFootMoveState);  
}
*/

/*
// Name			:	ClearFollowPath
// Purpose		:	Clears ped follow path state
// Parameters	:	None
// Returns		:	Nothing

void CPed::ClearFollowPath()
{
  

	for (int32 i = 0; i < PED_NUM_PATHNODES_LOOKAHEAD; i++)
	{
		m_aPathNodeList[i].SetEmpty();
	}

	m_nNumPathNodes = 0;
	m_nCurrentPathNode = 0;
} // end - CPed::ClearFollowPath
*/

/*
// Name			:	FollowPath
// Purpose		:	Follows a path supplied by route finding code
// Parameters	:	None
// Returns		:	Nothing

void CPed::FollowPath()
{
//	CPathNode* pPathNode;

    m_CurrentPathNode=m_aPathNodeList[m_nCurrentPathNode];

    if((m_iPathFollowTimer!=0)&&(CTimer::GetTimeInMilliseconds()>m_iPathFollowTimer))
    {
        RestorePreviousState();
        ClearFollowPath();
        m_iPathFollowTimer=0;
        return;
    }

	DEBUGPEDSTATE
	(
		CDebug::DebugString("FOLLOW PATH", 0, 10);
	)

  
	if(m_aPathNodeList[m_nCurrentPathNode].IsEmpty())
	{
	    RestorePreviousState();
        ClearFollowPath();
        m_iPathFollowTimer=0;
        return;
	}

	// set our target position to the coords of the node
	m_vecTargetPosition.x = ThePaths.FindNodePointer(m_aPathNodeList[m_nCurrentPathNode])->GetCoors().x;
	m_vecTargetPosition.y = ThePaths.FindNodePointer(m_aPathNodeList[m_nCurrentPathNode])->GetCoors().y;
	m_vecTargetPosition.z = GetPosition().z;
	
	// check if we've arrived at the last node.
	if (Seek())
	{
	    //just another check that we are really at the last node.
		if (m_nCurrentPathNode == m_nNumPathNodes)
		{
            RestorePreviousState();
            ClearFollowPath();
		    const int iTimeLeft=m_iPathFollowTimer-CTimer::GetTimeInMilliseconds();
		    SetFollowPath(m_vecGotoCoordDestination,m_fGotoCoordOnFootFromPathDistance,m_eGotoCoordOnFootMoveState,m_pEntityToAvoidOnPathFollow,m_pEntityToFollowOnPathFollow,iTimeLeft);
		}
	}
	
    //m_pCurrentPathNode=m_pPathNodeList[m_nCurrentPathNode];

} // end - CPed::FollowPath
*/


/*
// Name			:	SetStagger
// Purpose		:	Sets ped stagger state
// Parameters	:	None
// Returns		:	Nothing

void CPed::SetStagger(AnimationId anim)
{
	if (CanSetPedState())
	{
		SetStoredState();
		SetPedState(PED_STAGGER);
		CAnimBlendAssociation* pAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, anim);
		pAnim->SetBlendAmount(0.0f);
		pAnim->SetBlendDelta(8.0f);
		
		pAnim->SetFinishCallback(PedStaggerCB, this);
	}

} // end - CPed::SetStagger
*/

/*
// Name			:	Stagger
// Purpose		:	Trigger stagger animations for peds
// Parameters	:	None
// Returns		:	Nothing

void CPed::Stagger()
{
} // end - CPed::Stagger
*/

/*
// Name			:	SetEvasiveStep
// Purpose		:	Sets ped to step out of way of car
// Parameters	:	pointer to danger car
// Returns		:	Nothing

void CPed::SetEvasiveStep(CPhysical *pThreat, uint8 nForceStep)
{
	if(m_nPedState == PED_EVADE_STEP)
		return;
		
	if (IsPedInControl() && ((!IsPlayer() && m_nPedFlags.bThreatReactionOverwritesObjective) || nForceStep))
	{
		// does player hear/notice car to dive?  add random factor
		float fPedHeading = CGeneral::GetRadianAngleBetweenPoints(pThreat->GetPosition().x, pThreat->GetPosition().y, GetPosition().x, GetPosition().y);
		fPedHeading = CGeneral::LimitRadianAngle(fPedHeading);
		m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
		float fDelta = ABS(fPedHeading-m_fCurrentHeading);
		bool bHorn = false;
		
		
		if(fDelta>PI) fDelta = 2*PI - fDelta;
		
		if((pThreat->GetType() == ENTITY_TYPE_VEHICLE) && 
		((CVehicle*)pThreat)->GetBaseVehicleType() == VEHICLE_TYPE_CAR &&
		((CAutomobile*)pThreat)->m_cHorn > 0)
		{
			bHorn = true;
			if(!IsPlayer())
				nForceStep = true;
		}
			
		if(fDelta>0.5*PI && pThreat->GetModelIndex()!=MODELID_CAR_RCBANDIT && !bHorn && !nForceStep){	// car is behind ped
			return;	// didn't hear/notice car
		}
		
		SetLookFlag(pThreat, true);

		if(!(CGeneral::GetRandomNumber()&1) || pThreat->GetModelIndex()==MODELID_CAR_RCBANDIT || nForceStep)
		{
			// want to change direction: Get Car Velocity heading
			float fCarVec = CGeneral::GetRadianAngleBetweenPoints(pThreat->m_vecMoveSpeed.x, pThreat->m_vecMoveSpeed.y, 0.0f, 0.0f);
			// reverse heading from ped to car into heading from car to ped
			if((fPedHeading += PI)>PI)
				fPedHeading -= 2*PI;
		
			fPedHeading = fPedHeading - fCarVec;
			fPedHeading = CGeneral::LimitRadianAngle(fPedHeading);
			if(fPedHeading > 0)
				fPedHeading = fCarVec - HALF_PI;
			else
				fPedHeading = fCarVec + HALF_PI;

			// finally start animation
			AnimationId nAnim = ANIM_STD_NUM;
			if(nForceStep==0 || nForceStep==PED_FORCE_EVADE_STEP)
				nAnim = ANIM_STD_EVADE_STEP;
			else if(nForceStep==PED_FORCE_EVADE_COWER)
				nAnim = ANIM_STD_HANDSCOWER;
			
			// if anim is already there, don't do EvadeStep (could cause animCB probs, and won't notice anyway)
			if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, nAnim))
				return;

			CAnimBlendAssociation* pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, nAnim, 8.0f);
			pAnim->ClearFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			pAnim->SetFinishCallback(PedEvadeCB, this);
		}
		else
		{
			// if anim is already there, don't do EvadeStep (could cause animCB probs, and won't notice anyway)
			if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_HAILTAXI))
				return;
		
			// use another anim to show disaproval
			CAnimBlendAssociation* pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_HAILTAXI, 8.0f);
			pAnim->ClearFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			pAnim->SetFinishCallback(PedEvadeCB, this);
			// Raymond -> say 'watch it buddy' or something
		}
		
		if(!nForceStep)
			Say(AE_PED_DODGE_CAR);
		
		// just switch to desired heading for now, don't worry about turn rates
		m_fCurrentHeading = CGeneral::LimitRadianAngle(fPedHeading);
		
		// then save and set state
		//ClearLookFlag();
		ClearAimFlag();
		//SetStoredState();
		//m_nPedStoredState=m_nPedState;
		ePedState storedState=m_nPedStoredState;
		m_nPedStoredState=PED_NONE;
		SetStoredState();
		if(PED_NONE==m_nPedStoredState)
		{
		    m_nPedStoredState=storedState;
		}
		SetPedState(PED_EVADE_STEP);
	}

}
*/

/*
// Name			:	SetEvasiveDive
// Purpose		:	Sets ped to Dive out of way of car
// Parameters	:	pointer to danger car
// Returns		:	Nothing

void CPed::SetEvasiveDive(CPhysical *pThreat, bool8 nForceDive)
{
	if (IsPedInControl() && m_nPedFlags.bThreatReactionOverwritesObjective)
	{
		float fPedHeading = m_fCurrentHeading;
		bool8 bDontDive = false;
		bool8 bHorn = false;
		
		if((pThreat->GetType() == ENTITY_TYPE_VEHICLE) && 
		((CVehicle*)pThreat)->GetBaseVehicleType() == VEHICLE_TYPE_CAR &&
		((CAutomobile*)pThreat)->m_cHorn > 0)
		{
			bHorn = true;
			if(!IsPlayer())
				nForceDive = true;
		}
		if(!nForceDive){	
			// don't want to make player dive unless forced, but save danger state
			if(IsPlayer()){
				((CPlayerPed *)this)->m_nCarDangerCounter = 5;	
				((CPlayerPed *)this)->m_pDangerCar = (CAutomobile *)pThreat;
				REGREF(pThreat, (CEntity **) &(((CPlayerPed *)this)->m_pDangerCar));
				return;	
			}
			// does player hear/notice car to dive?  add random factor
			fPedHeading = CGeneral::GetRadianAngleBetweenPoints(pThreat->GetPosition().x, pThreat->GetPosition().y, GetPosition().x, GetPosition().y);
			fPedHeading = CGeneral::LimitRadianAngle(fPedHeading);
			m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
			float fDelta = ABS(fPedHeading-m_fCurrentHeading);
			if(fDelta>PI) fDelta = 2*PI - fDelta;
				
			if(fDelta>HALF_PI){	// car is behind ped
				if(CGeneral::GetRandomNumber()&7)
					return;	// 7in8 don't dive (didn't hear/notice car)
			}
			else if(CGeneral::GetRandomNumber()&1)
				bDontDive = true;	// plus 1in2 of all peds who could dive, don't
			
			// want to dive perpendicular to vector between ped and threat, to which side is not important!
			fPedHeading += HALF_PI;
			if(CGeneral::GetRandomNumber()&1)
				fPedHeading -= PI;
				
			Say(AE_PED_DODGE_CAR);
		}
		else if(pThreat)
		{
			// want to change direction first
			fPedHeading = CGeneral::GetRadianAngleBetweenPoints(pThreat->m_vecMoveSpeed.x, pThreat->m_vecMoveSpeed.y, 0.0f, 0.0f);
			fPedHeading += -HALF_PI + PI*(CGeneral::GetRandomNumber()&1);
			fPedHeading = CGeneral::LimitRadianAngle(fPedHeading);
		}	
		
		if(bDontDive || (!IsPlayer() && m_pPedStats->m_nStatFlags &STAT_EVADE_NODIVE))
		{
			m_fCurrentHeading = fPedHeading;
		
			// then save and set state
			ClearLookFlag();
			ClearAimFlag();
			// look at threat in question
			SetLookFlag((CEntity *)pThreat, true);
			
			// if anim is already there, don't do EvadeStep (could cause animCB probs, and won't notice anyway)
			if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_HANDSUP))
				return;

			// finally start animation
			CAnimBlendAssociation* pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_HANDSUP, 8.0f);
			pAnim->ClearFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			pAnim->SetFinishCallback(PedEvadeCB, this);

        	//SetStoredState();
			//m_nPedStoredState=m_nPedState
			ePedState storedState=m_nPedStoredState;
			m_nPedStoredState=PED_NONE;
			SetStoredState();
			if(PED_NONE==m_nPedStoredState)
			{
			    m_nPedStoredState=storedState;
			}
			SetPedState(PED_EVADE_STEP);
		}
		else
		{
			// just switch to desired heading for now, don't worry about turn rates
			m_fCurrentHeading = fPedHeading;
		
			// then save and set state
			ClearLookFlag();
			ClearAimFlag();
			SetStoredState();
			SetPedState(PED_EVADE_DIVE);

			// finally start animation
			CAnimBlendAssociation* pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_EVADE_DIVE, 8.0f);
			pAnim->SetFinishCallback(PedEvadeCB, this);
		}
		
		if(pThreat->GetIsTypeVehicle() && (GetPedType() == PEDTYPE_COP) && (((CVehicle *)pThreat)->pDriver) && ((CVehicle *)pThreat)->pDriver->IsPlayer())
		{
			FindPlayerWanted()->RegisterCrime_Immediately(CRIME_RECKLESS_DRIVING, GetPosition(), (UInt32)this);
			FindPlayerWanted()->RegisterCrime_Immediately(CRIME_SPEEDING, GetPosition(), (UInt32)this);
		}
	}
}
*/

/*
// Callback function for Ped Evade animations -
// might want to use to add a pause after anim before get up

void CPed::PedEvadeCB(CAnimBlendAssociation* pAnim, void* pData)
{
	ASSERTMSG(pData, "EvadeCB - no data");
	CPed* pPed = (CPed*)pData;
	if(pAnim)
	{
		if(pAnim->GetAnimId()==ANIM_STD_EVADE_DIVE)
		{
			// anim will get removed by FINISHAUTOREMOVE flag
			pPed->m_nPedFlags.bUpdateAnimHeading = true;
			pPed->ClearLookFlag();
			if(pPed->m_nPedState==PED_EVADE_DIVE){
				// make the ped think he's fallen over, then allow that code to get him up
				// so that he can be arrested if the police are nearby
				pPed->m_nUnconsciousTimer = CTimer::GetTimeInMilliseconds() + 1;
				pPed->SetPedState(PED_FALL);
			}
			pAnim->ClearFlag(ABA_FLAG_ISFINISHAUTOREMOVE);
			pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
//			pPed->SetGetUp();
		}
		else if(pAnim->IsFlagSet(ABA_FLAG_ISFINISHAUTOREMOVE))
		{
			// if anim is dealt with, just need to clear look and restore state
			pPed->ClearLookFlag();
			if(pPed->m_nPedState==PED_EVADE_DIVE || pPed->m_nPedState==PED_EVADE_STEP)
				pPed->RestorePreviousState();
		}
		else
		if(pPed->m_nPedState != PED_ARRESTED)
		{
			// need to manually blend anims out and remove them
			pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			if(pAnim->GetBlendDelta() >= 0.0)
				pAnim->SetBlendDelta(-4.0f);

			pPed->ClearLookFlag();
			if(pPed->m_nPedState==PED_EVADE_DIVE || pPed->m_nPedState==PED_EVADE_STEP)
				pPed->RestorePreviousState();
		}
	}
	else{
		// how can this happen?... but just incase
		pPed->ClearLookFlag();
		if(pPed->m_nPedState==PED_EVADE_DIVE || pPed->m_nPedState==PED_EVADE_STEP)
			pPed->RestorePreviousState();
	}
}
*/


// Name			:	SetDie
// Purpose		:	Sets ped dying state
// Parameters	:	None
// Returns		:	Nothing
/*
void CPed::SetDie(AnimationId anim, float fBlendDelta, float fAnimSpeed)
{
   // if(m_pMyAttractor)
   // {
   //     GetPedAttractorManager()->DeRegisterPed(this,m_pMyAttractor);
   // }
    
#ifdef GTA_NETWORK
	if (gGameNet.NetWorkStatus == NETSTAT_CLIENTRUNNING || gGameNet.NetWorkStatus == NETSTAT_CLIENTSTARTINGUP) return;
#endif

	if (this == FindPlayerPed())
	{
		if (FindPlayerPed()->bCanBeDamaged == FALSE)
		{
			return;
		}
	}
	m_pThreatEntity = NULL;

	if(!(m_nPedState == PED_DEAD) && !(m_nPedState == PED_DIE))
	{
		ASSERTMSG(anim!=ANIM_STD_HIT_FRONT, "Can't die with Hit_Front");
		ASSERTMSG(anim!=ANIM_STD_HIT_LEFT, "Can't die with Hit_Left");
		ASSERTMSG(anim!=ANIM_STD_HIT_BACK, "Can't die with Hit_Back");
		ASSERTMSG(anim!=ANIM_STD_HIT_RIGHT, "Can't die with Hit_Right");
		ASSERTMSG(fBlendDelta > 1.0f, "Can't die without BLENDING IN an anim");
		
		CAnimBlendAssociation* pAnim = NULL;
		// use lower blend speed if ped is not in standing up state
		if(m_nPedState==PED_FALL || m_nPedState==PED_GETUP)
			fBlendDelta *= 0.5f;
		
		SetStoredState();
		ClearAll();
		m_nHealth = 0;

		if(m_nPedState == PED_DRIVING)
		{
			if(!IsPlayer() && (m_pMyVehicle==NULL || m_pMyVehicle->GetBaseVehicleType()!=VEHICLE_TYPE_BIKE))
				FlagToDestroyWhenNextProcessed();
		}
		else
		if(m_nPedFlags.bInVehicle)// in the process of getting out of car
		{
			if(m_pAnim)
				m_pAnim->SetBlendDelta(-1000.0f);
	
//			SetMoveState(PEDMOVE_NONE);
//			CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 100.0f);
//			RestartNonPartialAnims();
		}
		else
		if(m_nPedState == PED_ENTER_CAR || m_nPedState == PED_CARJACK)
		{
			QuitEnteringCar();
		}
		SetPedState(PED_DIE);
		
		if(anim != ANIM_STD_NUM)
		{
			// better check if anim is already there first
			//pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, anim);
			//if(pAnim){	// if so - reset and set to blend in again
			//	pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, anim, fBlendDelta);
			//	pAnim->SetCurrentTime(0.0f);
			//	pAnim->SetBlendAmount(0.0f);
			//	pAnim->SetFlag(ABA_FLAG_ISPLAYING);
			//}
			//else
				pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, anim, fBlendDelta);
			
////////////////////////////////////////////////////
			if(fAnimSpeed > 0.0f)
				pAnim->SetSpeed(fAnimSpeed);
////////////////////////////////////////////////////
			pAnim->ClearFlag(ABA_FLAG_ISFINISHAUTOREMOVE);
			if(pAnim->IsFlagSet(ABA_FLAG_ISPLAYING))
			{			
				pAnim->SetFinishCallback(FinishDieAnimCB, this);
				m_nPedFlags.bIsPedDieAnimPlaying = true;
			}
			ASSERT(pAnim);
		}
		else
		{
			m_nPedFlags.bIsPedDieAnimPlaying = false;
		}
		

		Say(AE_PED_DEATH_CRY);
		if((m_nPedStoredState == PED_ENTER_CAR)||(m_nPedStoredState == PED_CARJACK))
		{
			QuitEnteringCar();		
		}
		if(!m_nPedFlags.bInVehicle)
			StopNonPartialAnims();
		if(pAnim){
			ASSERTMSG(pAnim->GetBlendDelta()>0.0f || pAnim->GetBlendAmount()>0.99f, "Die anim not blending in!");
		}
		// time of death will get reset when the ped is actually 'dead'
		// but can use timer to check die doesn't go on too long (mainly for player, just incase)
		m_nTimeOfDeath = CTimer::GetTimeInMilliseconds();
		
		if(CLocalisation::Blood() && anim==ANIM_STD_HIT_FLOOR && pAnim)  // only if blood is allowed
		{
			pAnim->SetCurrentTime(pAnim->GetTotalTime() - 0.01f);
			pAnim->SetFlag(ABA_FLAG_ISPLAYING);
		}
	}
} // end - CPed::SetDie
*/

/*
void CPed::FinishDieAnimCB(CAnimBlendAssociation* pAnim, void* pData)
{	
	// just want to reset wait timer so Wait() will auto finish on next loop
	CPed *pPed = (CPed *) pData;
	if(pPed->m_nPedFlags.bIsPedDieAnimPlaying)
		pPed->m_nPedFlags.bIsPedDieAnimPlaying = false;
}
*/

/*
// Name			:	SetDead
// Purpose		:	Sets ped dead state
// Parameters	:	None
// Returns		:	Nothing

void CPed::SetDead()
{
	// want to check that this ped is actually lying on the ground (not drowned tho!)
//	ANIM_ASSERT(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_DROWN) ||
//	m_nPedFlags.bInVehicle || !IsPedHeadAbovePos(-0.3f), "Dead ped not lying on ground!", (RpClump*)m_pRwObject);

	if(!RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_DROWN))
		SetUsesCollision(false);
		
	m_nHealth = 0;
	if(m_nPedState == PED_DRIVING)
		m_nFlags.bIsVisible = FALSE;
	SetPedState(PED_DEAD);
	m_pAnim = NULL;
	m_pNOCollisionVehicle = NULL;
	RemoveWeaponModel(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetModelId());
	m_nCurrentWeapon = 0;
	

	if (this != FindPlayerPed())
	{
		RemoveWeaponAnims(WEAPONTYPE_UNARMED, -1000.0f);
		CreateDeadPedWeaponPickups();
		CreateDeadPedMoney();
	}
	m_nTimeOfDeath = CTimer::GetTimeInMilliseconds();
	bBloodPuddleCreated = false;
	m_nPedFlags.bDoBloodyFootprints = false;
	//m_nPedFlags.bWasWarpedIntoCar = false;
} // end - CPed::SetDead
*/


/*
// Name			:	Die
// Purpose		:	Trigger dying animations for peds
// Parameters	:	None
// Returns		:	Nothing

void CPed::Die()
{

} // end - CPed::Die
*/

/*
// Name			:	SetChat
// Purpose		:	Ped stops and chats to other ped
// Parameters	:	None
// Returns		:	nothing

void CPed::SetChat(CEntity* pPed2,uint32 time)
{
	if(m_nPedState!=PED_CHAT)
	{
	    m_nPedStoredState=PED_NONE;
		SetStoredState();
	}
		
	SetPedState(PED_CHAT);
	SetMoveState(PEDMOVE_STILL);

    m_nLookTimer=0;
	SetLookFlag(pPed2, true, true);
	
	if(time < 0)
		m_nChatTimer = 0;// chat for ever
	else
		m_nChatTimer = (CTimer::GetTimeInMilliseconds() + time);//% MAX_TIMER_MILLISECONDS; // used to prevent ped from looking again
	m_nLookTimer = (CTimer::GetTimeInMilliseconds() + 3000);//% MAX_TIMER_MILLISECONDS; // used to prevent ped from looking again
}
*/

/*
// Name			:	Chat
// Purpose		:	Ped stops and chats to other ped
// Parameters	:	None
// Returns		:	nothing

void CPed::Chat()
{
	// If still needing to turn to look at oother person in conversation
	if(m_nPedFlags.bIsLooking)
	{
		// Turn body, stop when facing other person
		if(TurnBody())
			ClearLookFlag();
	}	

	if((m_pEntLookEntity == NULL)|| !m_pEntLookEntity->GetIsTypePed())
	{
		ClearChat();
		return;
	}	

	CPed* pOtherPed = ((CPed*)m_pEntLookEntity);
	CAnimBlendAssociation* pAnim;

	if(pOtherPed->m_nPedState != PED_CHAT)
	{
		ClearChat();
		m_nChatTimer = (CTimer::GetTimeInMilliseconds() + 30000); // used to prevent ped from chatting again

		if(pOtherPed->m_pObjectivePed && (pOtherPed->m_Objective == KILL_CHAR_ON_FOOT || 
			pOtherPed->m_Objective == FLEE_CHAR_ON_FOOT_TILL_SAFE))
		{
			ReactToAttack((CEntity *)pOtherPed->m_pObjectivePed);
		}
		return;
	}	
		
	// if talking, randomly stop	
	if(m_nPedFlags.bIsTalking)
	{
		if(CGeneral::GetRandomNumber() < 512)
		{
			// See if chat animation is playing
			pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CHAT);
 			if(pAnim)
 			{
 				pAnim->SetBlendDelta(-4.0f);
 				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
 			}	
			m_nPedFlags.bIsTalking = 0; 
		}
		else
		{
			// Keep chatting
			Say(AE_PED_CHATTING_TO_ANOTHER_PED);
		}
	}
	else
	{
		// Randomly play an expression if one is not already playing
		if(CGeneral::GetRandomNumber() < 20 && 
			RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_PEDFLAG_EXPRESS) == NULL)
		{
			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_XPRESS_SCRATCH, 4.0f);
		}
		// If other ped is not talking and not playing an expression then start talking
		if(!pOtherPed->m_nPedFlags.bIsTalking &&
			RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_PEDFLAG_EXPRESS) == NULL)
		{
			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_CHAT, 4.0f);
			pAnim->SetCurrentTime(CGeneral::GetRandomNumberInRange(0.0f, 3.0f));
			m_nPedFlags.bIsTalking = 1;

			Say(AE_PED_CHATTING_TO_ANOTHER_PED);

		}	
	}	
	if(m_nChatTimer && CTimer::GetTimeInMilliseconds() > m_nChatTimer)
	{
		ClearChat();
		m_nChatTimer = (CTimer::GetTimeInMilliseconds() + 30000); // used to prevent ped from chatting again
	}

}
*/

/*
// Name			:	ClearChat
// Purpose		:	
// Parameters	:	None
// Returns		:	nothing

void CPed::ClearChat()
{
	// See if chat animation is playing and remove it
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CHAT);
	if(pAnim)
	{
		pAnim->SetBlendDelta(-8.0f);
		pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
	}
	
	m_nPedFlags.bIsTalking = 0;
	ClearLookFlag();
	RestorePreviousState();
		
	if(BUY_ICE_CREAM==m_Objective)
    {
        m_nPedFlags.bBoughtIceCream=true;
        SetObjective(NO_OBJ);
        SetWanderPath(CGeneral::GetRandomNumberInRange(0,8));
    }
}
*/

/*
// Name			:	FacePhone
// Purpose		:	Ped turns to face a phone
// Parameters	:	None
// Returns		:	TRUE if facing the phone, FALSE otherwise

bool CPed::FacePhone()
{
	float fDesiredHeading,fCurrentHeadingDegrees;
	float fPhonex,fPhoney;
	
	fCurrentHeadingDegrees = RADTODEG(m_fCurrentHeading);

	fPhonex=gPhoneInfo.ms_aPhone[m_nPhoneToUse].vecPosition.x;
	fPhoney=gPhoneInfo.ms_aPhone[m_nPhoneToUse].vecPosition.y;

	fDesiredHeading = CGeneral::GetRadianAngleBetweenPoints(fPhonex, fPhoney, GetPosition().x, GetPosition().y);
	SetLookFlag(fDesiredHeading, true, true);
	fDesiredHeading=CGeneral::LimitAngle(fDesiredHeading);

	m_vecCurrentVelocity = CVector2D(0.0f, 0.0f);


	if (fCurrentHeadingDegrees + 180 < fDesiredHeading)
	{
		fDesiredHeading -= 360;
	}
	else
	{
		if (fCurrentHeadingDegrees - 180 > fDesiredHeading)
		{
			fDesiredHeading += 360;
		}
	}

	float fHeadingChange = fCurrentHeadingDegrees - fDesiredHeading;

	if (ABS(fHeadingChange) > 0.75f)
	{
		fCurrentHeadingDegrees -= fHeadingChange / 5;
	}
	else
	{
		ClearFacePhone();
		ClearLookFlag();
		m_nPhoneTimer = (CTimer::GetTimeInMilliseconds() + PHONECALL_DURATION);// % MAX_TIMER_MILLISECONDS;
		return(TRUE);
	}

	m_fCurrentHeading = DEGTORAD(fCurrentHeadingDegrees);

	return(FALSE);

} // end - CPed::FacePhone
*/

/*
// Name			:	MakePhonecall
// Purpose		:	Ped chats away on the phone
// Parameters	:	None
// Returns		:	TRUE if finished the call, FALSE otherwise

bool CPed::MakePhonecall()
{
	if (CTimer::GetTimeInMilliseconds() > m_nPhoneTimer)
	{
		ClearMakePhoneCall();
		gPhoneInfo.ms_aPhone[m_nPhoneToUse].nState=VACANT;
		m_nPhoneToUse=-1;

		// Phone calls don't actually register crimes anymore. Still nice to see the peds
		// do it though.

		// temporary to test police pursuit
//		FindPlayerWanted()->RegisterCrime(CRIME_SHOOT_COP);
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}

} // end - CPed::MakePhonecall
*/

/*
// callback to play talking into phone idle
void StartTalkingOnMobileCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed*)pData;
	if(pPed->GetPedState() == PED_ANSWER_MOBILE)
		CAnimManager::BlendAnimation((RpClump*)pPed->m_pRwObject, ANIM_STD_PED, ANIM_STD_PHONE_TALK, 4.0f);
}

// Callback to remove mobile and replace with current weapon
void FinishTalkingOnMobileCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPed *) pData;

	// remove phone and replace with weapon
	if(pPed->m_eStoredWeapon != WEAPONTYPE_UNIDENTIFIED)
	{
		pPed->RemoveWeaponModel(MODELID_WEAPON_CELLPHONE);
		pPed->SetCurrentWeapon(pPed->m_eStoredWeapon);
		pPed->m_eStoredWeapon = WEAPONTYPE_UNIDENTIFIED;
	}
	
	// allow player to look at stuff again
	pPed->m_nLookTimer = 0;
}
*/

/*
//
// name:		SetAnswerMobile
// description:	Set ped to start answering his mobile phone
void CPed::SetAnswerMobile()
{
	// better check if dying first otherwise we can mess everything up and get stuck in Answer phone
	if(m_nPedState==PED_ANSWER_MOBILE || m_nPedState==PED_DIE || m_nPedState==PED_DEAD)
		return;
	SetPedState(PED_ANSWER_MOBILE);
	
	RemoveWeaponAnims(GetWeapon()->GetWeaponType(), -4.0f);
	
	// play start anim
	CAnimBlendAssociation* pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_PHONE_IN, 4.0f);
	pAnim->SetFinishCallback(&StartTalkingOnMobileCB, this);

	// stop player looking at other things while on the phone
	m_nLookTimer = 0x7fffffff;

	// save current weapon so it can be resored after player stops using mobile
	if(m_eStoredWeapon == WEAPONTYPE_UNIDENTIFIED)
		m_eStoredWeapon = GetWeapon()->GetWeaponType();
	//SetCurrentWeapon(0);
	RemoveWeaponModel(-1);
}

//
// name:		ClearAnswerMobile
// description:	Set ped to stop talking into his mobile phone
void CPed::ClearAnswerMobile()
{
	if(	m_nPedStoredState == PED_ANSWER_MOBILE)
		m_nPedStoredState = PED_NONE;
		
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_PHONE_TALK);
	if(pAnim)
	{
		pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_PHONE_OUT, 8.0f);
		pAnim->SetFinishCallback(&FinishTalkingOnMobileCB, this);
	}
	else
	{
		FinishTalkingOnMobileCB(NULL, this);
	}

	if(m_nPedState != PED_ANSWER_MOBILE)
		return;

	// just incase RestorePreviousState might be unable to get rid of PED_ANSWER_MOBILE
	SetPedState(PED_IDLE);
	RestorePreviousState();
	m_pAnim = NULL;
}
//
// name:		AnswerMobile
// description:	Check ped anims while in answering mobile phone state
void CPed::AnswerMobile()
{
#define PHONE_APPEAR_TIME		0.85f
#define PHONE_DISAPPEAR_TIME	0.5f
	if(IsPedInControl())
	{
		CAnimBlendAssociation* pStartAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_PHONE_IN);
		CAnimBlendAssociation* pEndAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_PHONE_OUT);
		CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_PHONE_TALK);

		// If can't find anims
		if(pStartAnim == NULL && pAnim == NULL && pEndAnim == NULL)
			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_PHONE_TALK, 4.0f);
		else if(pStartAnim)
		{
			if(pStartAnim->GetCurrentTime() >= PHONE_APPEAR_TIME && m_pWeaponAtomic == NULL)
			{
				AddWeaponModel(MODELID_WEAPON_CELLPHONE);
			}
		}	
		else if(pEndAnim)
		{
			if(pEndAnim->GetCurrentTime() >= PHONE_DISAPPEAR_TIME && 
				pEndAnim->GetCurrentTime() - pEndAnim->GetTimeStep() < PHONE_DISAPPEAR_TIME)
			{
				RemoveWeaponModel(MODELID_WEAPON_CELLPHONE);
				ASSERT(m_eStoredWeapon != WEAPONTYPE_UNIDENTIFIED);
				SetCurrentWeapon(m_eStoredWeapon);
				m_eStoredWeapon = WEAPONTYPE_UNIDENTIFIED;
			}
		}
	}
}
*/


//
// name:		AnswerMobile
// description:	Check anims while in answer mobile state

///////////////////////////////////////////////////////////////////////////
// FUNCTION:	Teleport
// DOES:		Moves this thing to different coordinates doing the things
//				that need to be done to stop the game from crashing.
///////////////////////////////////////////////////////////////////////////
void CPed::Teleport(CVector NewCoors, Bool8 bClearOrientation)	//	bClearOrientation is only used in the vehicle Teleport functions
{
	/*
	CWorld::Remove(this);
	this->SetPosition(NewCoors);
	this->SetIsStanding(false);
	//this->m_nAntiSpazTimer = 0;
	//this->m_vecAntiSpazVector = CVector2D(0.0f, 0.0f);
	TIDYREF(m_pDamageEntity,(CEntity**)&m_pDamageEntity);
	this->m_pDamageEntity = NULL;	
	CWorld::Add(this);
	
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);
	*/
	
	if(IsPlayer())
	{
		// need to save hold task otherwise player can't get through an interior entry/exit
		// while holding anything
		// MN - commented out as this is also done inside FlushImmediately
//		CTask *pRestoreHoldTask = NULL;
//		if(GetPedIntelligence()->GetTaskHold()
//		&& GetPedIntelligence()->GetTaskHold()->GetTaskType()==CTaskTypes::TASK_SIMPLE_HOLD_ENTITY)
//			pRestoreHoldTask = GetPedIntelligence()->GetTaskHold()->Clone();
	
		// remove all tasks and give back default task
		GetPedIntelligence()->FlushImmediately(true);
		
//		if(pRestoreHoldTask)
//			GetPedIntelligence()->AddTaskSecondaryPartialAnim(pRestoreHoldTask);
	}
	else
	{
		// remove all tasks if exiting a car and give back default task
		if(GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_LEAVE_CAR))
		{
			GetPedIntelligence()->FlushImmediately(true);
		}
	}
	
	CWorld::Remove(this);
	this->SetPosition(NewCoors);
	this->SetIsStanding(false);
	//this->m_nAntiSpazTimer = 0;
	//this->m_vecAntiSpazVector = CVector2D(0.0f, 0.0f);
	TIDYREF(m_pDamageEntity,(CEntity**)&m_pDamageEntity);
	this->m_pDamageEntity = NULL;	
	CWorld::Add(this);
	
	SetMoveSpeed(0.0f, 0.0f, 0.0f);
	SetTurnSpeed(0.0f, 0.0f, 0.0f);
}


/*
void CPed::SetSeekCar(CVehicle *pVehicle, uint32 nDoor)
{
	// check we're not already getting into a car, driving or dying or somthing first
	if(m_nPedState == PED_SEEK_CAR || !CanSetPedState() || m_nPedState==PED_DRIVING)
		return;

//TEST TEMP!!
//	if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE && m_pNOCollisionVehicle==NULL)
//		m_pNOCollisionVehicle = pVehicle;
/////////////

	SetStoredState();
	m_pTargetEntity = pVehicle;
	REGREF(m_pTargetEntity, (CEntity**)&m_pTargetEntity)
	m_pObjectiveVehicle = pVehicle;
	REGREF(m_pObjectiveVehicle, (CEntity**)&m_pObjectiveVehicle)
	m_pMyVehicle = pVehicle;
	REGREF(m_pMyVehicle, (CEntity**)&m_pMyVehicle)
	m_pTargetEntity->RegisterReference(&m_pTargetEntity);
	m_nDoor = nDoor;
	m_fArriveRange = 0.5f;

	SetPedState(PED_SEEK_CAR);
}
*/

/*
void CPed::SeekCar()
{
	CVehicle *pVehicle = m_pObjectiveVehicle;
	CVector vecDoorPosition = CVector(0.0f,0.0f,0.0f);
	CVector vecMyPos, vecToDoor;
	float fDist;
	bool bGoingForCorner = false;
	uint8 nFlagForMyDoor;

	if(pVehicle == NULL)
	{
		RestorePreviousState();
		return;
	}	
	ASSERT(pVehicle->GetType() == ENTITY_TYPE_VEHICLE);
		
	vecMyPos = GetPosition();
	
	if(m_Objective == ENTER_CAR_AS_PASSENGER)
	{
		if(m_nCarJackTimer > CTimer::GetTimeInMilliseconds())
		{
//			RestorePreviousState(); put this back as soon as tested
			SetMoveState(PEDMOVE_STILL);
			return;
		}

		if(pVehicle->GetModelIndex()==MODELID_CAR_COACH)// special case for coaches
		{
			GetNearestDoor(pVehicle, vecDoorPosition);
		}
		else
		if(pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_TRAIN)
		{
			if(pVehicle->GetStatus() == STATUS_TRAIN_NOT_MOVING)
			{
				if(!GetNearestTrainDoor(pVehicle, vecDoorPosition))
				{	
					// train is not ready to board
					RestorePreviousObjective();
					RestorePreviousState();
					return;
				}
			}
			else
			{
				RestorePreviousObjective();// train is moving away
				RestorePreviousState();
				return;
			}
		}
		else
		if(!GetNearestPassengerDoor(pVehicle, vecDoorPosition))
		{
			//Car is full
			if(pVehicle->m_nNumPassengers == pVehicle->m_nMaxPassengers)
			{
				RestorePreviousObjective();
				RestorePreviousState();
			}
			else// there might still be a chance to get in
			{
				SetMoveState(PEDMOVE_STILL);
			}
			m_nPedFlags.bIsWaitingToGetInCar = true;
			return;
		}
		else
		{
			m_nPedFlags.bIsWaitingToGetInCar = false;
		}
	}
	else
	{
		// don't have to carjack bikes from drivers side, so just go for closest
		if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
		{
			GetNearestDoor(pVehicle, vecDoorPosition);
		}
		else if((m_nDoor)&&(m_Objective != ENTER_CAR_AS_DRIVER))
		{
			if(IsRoomToBeCarJacked())
			{
				vecDoorPosition = GetPositionToOpenCarDoor(pVehicle, m_nDoor);// driver's door only
			}
			else
			{
				if(GetPedType()== PEDTYPE_COP)
				{
					vecDoorPosition = GetPositionToOpenCarDoor(pVehicle, CAR_DOOR_RF);// driver's door only
				}
				else
				{
					SetMoveState(PEDMOVE_STILL);
//					RestorePreviousState();
				}
			}
		}
		else
		{
			GetNearestDoor(pVehicle, vecDoorPosition);
		}

	
	}

	if(vecDoorPosition.x == 0.0f && vecDoorPosition.y == 0.0f)
	{
		// can't get in this car
		// don't give up just yet though
		//
		
		//if its a mission car we might need to get back in
		// so walk towards it
		if((IsPlayer()|| CharCreatedBy == MISSION_CHAR) && pVehicle->VehicleCreatedBy == MISSION_VEHICLE && !pVehicle->pDriver)
		{
			vecDoorPosition = pVehicle->GetPosition();
			
			// if we are touching it, then warp player into car

			if(m_nPedFlags.bCollidedWithMyVehicle)
			{
				WarpPedIntoCar(m_pMyVehicle);
				return;
			}
		}
		else
		{
			RestorePreviousState();
			if(IsPlayer())
			{
				ClearObjective();
			}
			else
			{
				if(CharCreatedBy == RANDOM_CHAR)
				{
					// car jacker/prozzer
					m_nCarJackTimer = (CTimer::GetTimeInMilliseconds() + 30000);			
				}
			}
			SetMoveState(PEDMOVE_STILL);
			// this has to be here, because we haven't been in SEEK_CAR pedstate long enough to alloow that code to clear any
			// sniper or rocket launcher camera modes that may be active
			TheCamera.ClearPlayerWeaponMode();
			//and allow this car to be deleted (we can't get in to move it and it may be blocking something)
			CCarCtrl::RemoveFromInterestingVehicleList(pVehicle);
			return;
		}
	}

	bGoingForCorner = PossiblyFindBetterPosToSeekCar(&vecDoorPosition, pVehicle);
	
	m_vecTargetPosition = vecDoorPosition;
	vecToDoor = m_vecTargetPosition - vecMyPos;

	fDist = vecToDoor.MagnitudeSqr();
	
	// if bForceRun OR car is driving away from ped and they're more than walk distance away from door
	if(m_nPedFlags.bForceRun || (pVehicle->pDriver && fDist>2.0f*2.0f
	&& (CMaths::Abs(pVehicle->m_vecMoveSpeed.x)>0.01f || CMaths::Abs(pVehicle->m_vecMoveSpeed.y)>0.01f)) )
	{
		SetMoveState(PEDMOVE_RUN);
	}
	else if(fDist < 2.0f*2.0f)//3.0f
	{
		SetMoveState(PEDMOVE_WALK);
	}

	if(fDist < 1.0f*1.0f)
	{
		float	OurBBWidth = pVehicle->GetColModel().GetBoundBoxMax().x * 2.0f;

		if(fDist < OurBBWidth)
		{
			m_nPedFlags.bIsNearCar = true;
		}
	}
	else
	{
		m_nPedFlags.bIsNearCar = false;
	}
		
	switch(m_nDoor)
	{
		case CAR_WINDSCREEN:	// do flags as Front Left door 
		case CAR_DOOR_LF:nFlagForMyDoor = 1; break;
		case CAR_DOOR_LR:nFlagForMyDoor = 2;break;
		case CAR_DOOR_RF:nFlagForMyDoor = 4;break;
		case CAR_DOOR_RR:nFlagForMyDoor = 8;break;
		default: nFlagForMyDoor = 0;
	}
	
	if(pVehicle->m_nGettingInFlags & nFlagForMyDoor) // someone is already getting in here
	{
		m_nPedFlags.bIsWaitingToGetInCar = true;
	}
	else
	{
		m_nPedFlags.bIsWaitingToGetInCar = false;
	}
	

	if(Seek()&&(!bGoingForCorner)
	&&(vecDoorPosition.z < (GetPosition().z+1.6f))
	&&(vecDoorPosition.z > (GetPosition().z-0.5f)))
	{
#ifdef GTA_TRAIN	
		if(pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_TRAIN)
		{
			SetEnterTrain(pVehicle,m_nDoor);
			return;
		}
#endif // GTA_TRAIN		
		m_fCurrentHeading = m_fDesiredHeading; // stop the twitching and turning, we are in the right location
		
		if(m_nPedFlags.bIsWaitingToGetInCar)
		{

			SetMoveState(PEDMOVE_STILL);
			return;// just keep waiting for now			
		}	

		pVehicle->SetIsStatic(false);

		if(m_Objective == SOLICIT_VEHICLE)
		{
			SetSolicit(1000);
		}
		else if(m_Objective == BUY_ICE_CREAM)
		{
			SetBuyIceCream();
		}
		else if(pVehicle->m_nNumGettingIn < (pVehicle->m_nMaxPassengers+1) && pVehicle->CanPedEnterCar())
		{
			switch (pVehicle->GetStatus())
			{
				case STATUS_PLAYER_DISABLED: 
				case STATUS_PLAYER: 
				case STATUS_SIMPLE:
				case STATUS_PHYSICS:
					if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
					{
						if( (!pLeader || pLeader != pVehicle->pDriver) && (
						((m_nDoor==CAR_DOOR_LF || m_nDoor==CAR_DOOR_RF || m_nDoor==CAR_WINDSCREEN) && pVehicle->pDriver) ||
						((m_nDoor==CAR_DOOR_LR || m_nDoor==CAR_DOOR_RR) && pVehicle->pPassengers[0])) )
						{
							SetCarJack(pVehicle);
						}
						else // no one in seat, OR THERE IS, BUT THEY ARE FRIENDS
						{
							SetEnterCar(pVehicle, m_nDoor);
						}
					}
					else if((!pVehicle->m_bIsBus)&&((!pLeader)||(pLeader != pVehicle->pDriver)) && // n.b. cars in these states shouldn't have a driver so even if pLeader == NULL, it will be ok
					( (m_nDoor == CAR_DOOR_LF && pVehicle->pDriver)
					||(m_nDoor == CAR_DOOR_RF && pVehicle->pPassengers[0])
					||(m_nDoor == CAR_DOOR_LR && pVehicle->pPassengers[1])
					||(m_nDoor == CAR_DOOR_RR && pVehicle->pPassengers[2])) )
					{
						SetCarJack(pVehicle);
						// if I'm dragging out a passenger, get driver out too
						//(should be taken care of in the setincarCB)
						if(m_Objective == ENTER_CAR_AS_DRIVER && m_nDoor != CAR_DOOR_LF) 
						{
							pVehicle->pDriver->m_nPedFlags.bFleeWhenOutCar = true;
						}
					}
					else // no one in seat, OR THERE IS, BUT THEY ARE FRIENDS
					{
						SetEnterCar(pVehicle, m_nDoor);
					}
					break;
	
				case STATUS_ABANDONED:
					if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
					{
						if( (m_nDoor==CAR_DOOR_LR || m_nDoor==CAR_DOOR_RR) && pVehicle->pPassengers[0])
						{
							if(pVehicle->pPassengers[0]->m_nPedFlags.bDontDragMeOutCar)
							{
								if(IsPlayer())
									SetEnterCar(pVehicle, m_nDoor);
									// else do nothing
							}
							else
								SetCarJack(pVehicle);
						}
						else
							SetEnterCar(pVehicle, m_nDoor); 
					}
					else if((m_nDoor == CAR_DOOR_RF)&&(pVehicle->pPassengers[0]))
					{
						if(pVehicle->pPassengers[0]->m_nPedFlags.bDontDragMeOutCar)
						{
							if(IsPlayer())
								SetEnterCar(pVehicle, m_nDoor);
								// else do nothing
						}
						else
							SetCarJack(pVehicle);
					}
					else
					{
						SetEnterCar(pVehicle, m_nDoor); 
					}
					break;
					
				case STATUS_WRECKED:
					SetIdle();
					break;
			}
			
			
		}
		// If number of peds getting in is greater than the number of passenger slots
		else
		{
			RestorePreviousState();
		}
	}
}
*/






bool CPed::IsPlayer() const
{
//#ifdef GTA_NETWORK
	if(GetPedType()==PEDTYPE_PLAYER1 || GetPedType()==PEDTYPE_PLAYER2)
//	|| GetPedType()==PEDTYPE_PLAYER_NETWORK)
//#else
//	if(GetPedType()==PEDTYPE_PLAYER1)
//#endif
	{
		return(true);
	}
	return(false);
}

/*
bool CPed::IsGangMember() const
{
	if ((GetPedType() >= PEDTYPE_GANG1) && (GetPedType() <= PEDTYPE_GANG9))
	{
		return(true);
	}
	return(false);
}
*/

//////////////////////////////////////////////////////////////////////////////
// NAME :     AssertPedPointerValid
// FUNCTION : Will assert if the ped pointer is not valid or the ped is not in
//            the world.
//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
void AssertPedPointerValid(CPed *pPed)
{
    ASSERT(IsPedPointerValid(pPed));
    /*
	AssertPedPointerValid_NotInWorld(pPed);
	
	if (pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle)
	{
		AssertEntityPointerValid(pPed->m_pMyVehicle);
	}
	else
	{
		// Make sure this ped is actually in the world at the moment
		ASSERT(pPed->m_listEntryInfo.GetHeadPtr() || pPed == FindPlayerPed());
	}
	*/
}
void AssertPedPointerValid_NotInWorld(CPed *pPed)
{
    ASSERT(IsPedPointerValid_NotInWorld(pPed));
    /*
	Int32	Index;

	ASSERT(pPed);	// Null pointers are not valid

	Index = (CPools::GetPedPool().GetIndex(pPed))>>8;

	ASSERT(Index >= 0 && Index <= POOLS_MAXNOOFPEDS);
	*/
}
#endif

bool IsPedPointerValid(CPed *pPed)
{
	if(!IsPedPointerValid_NotInWorld(pPed))
	{
	    return false;
	}
	
	if (pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle)
	{
		return IsEntityPointerValid(pPed->m_pMyVehicle);
	}
	else
	{
		// Make sure this ped is actually in the world at the moment
		return (pPed->m_listEntryInfo.GetHeadPtr() || pPed == FindPlayerPed());
	}
}

bool IsPedPointerValid_NotInWorld(CPed *pPed)
{
	return CPools::GetPedPool().IsValidPtr(pPed);
}

bool CPed::IsPointerValid()
{
	int32	Index;

	Index = (CPools::GetPedPool().GetIndex(this))>>8;

	if(Index < 0 || Index >= POOLS_MAXNOOFPEDS)
		return false;
	if(m_listEntryInfo.GetHeadPtr() == NULL && this != FindPlayerPed())
		return false;
	return true;	
}

float fTempBikeY = 0.7f;
float fTempBikeZ = 0.65f;

void CPed::SetPedPositionInCar()
{
	CVector vecLocalSeatPosition;
	CVector	vecFinalPosition;
	CVehicleModelInfo* pModelInfo;
	bool bFoundOldAnim = false;
	
	if (CReplay::ReplayGoingOn()) return;
	
	/*
	if(m_nPedFlags.bUpdateAnimPosition)
	{
		// See if ped sit in car anim is playing, sometimes it hangs on for an extra frame
		// sorry, this is a bit fudgy
		CAnimBlendAssociation* pAnim = NULL;
		if(m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
		{
			pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_BIKE_JUMPON_LHS);
			if(!pAnim)
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_BIKE_JUMPON_RHS);
			if(!pAnim)
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_BIKE_KICK);
				
			if(pAnim)
			{
				LineUpPedWithCar(POSITION_FIX_Z);				
				return;
			}
			else
				m_nPedFlags.bUpdateAnimPosition = false;
		}
		//
		else
		{
			pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_GET_IN_LHS);
			if(!pAnim)
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_GET_IN_LO_LHS);
			{
			if(!pAnim)
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_CLOSE_DOOR_LHS);
			{
			if(!pAnim)
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_CLOSE_DOOR_RHS);
			{
			if(!pAnim)
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_CLOSE_DOOR_LO_LHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_SHUFFLE_RHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_SHUFFLE_LO_RHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_VAN_GET_IN_REAR_LHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_VAN_GET_IN_REAR_RHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_COACH_GET_IN_LHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_COACH_GET_IN_RHS);
			{
			if(!pAnim)// not playing sit in car anim yet
				pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_JUMP_IN_LO_LHS);
			}}}}}}}}}}}} // phew, than goodness for bracket checking editors

			if(pAnim)// not playing sit in car anim yet
				bFoundOldAnim = true;
			else
				bFoundOldAnim = false;
		}// End if Not bike
			
		if(bFoundOldAnim)
		{
			LineUpPedWithCar(POSITION_FIX_Z);				
			m_nPedFlags.bUpdateAnimPosition = false;
			return;
		}
	}
	*/

#ifndef GTA_NETWORK
	ASSERT(m_pMyVehicle);
#endif
	ASSERT(m_nPedFlags.bInVehicle);


//	CAnimBlendClumpData *pAnimBlendClump = ANIMBLENDCLUMPFROMCLUMP((RpClump*)m_pRwObject);

//	AnimBlendFrameData* pFrameData = RpAnimBlendClumpFindFrame((RpClump*)m_pRwObject, "Smid");
//	CVector vecPosn = pFrameData->posn;

	pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(m_pMyVehicle->GetModelIndex());
	CMatrix Matrix = m_pMyVehicle->GetMatrix();
	CMatrix Matrix2;
	
	if(this == m_pMyVehicle->pDriver)
	{
		vecLocalSeatPosition = pModelInfo->GetFrontSeatPosn();
		MUST_FIX_THIS(sandy);
		if(m_pMyVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT && m_pMyVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BIKE)
			vecLocalSeatPosition.x = -vecLocalSeatPosition.x;
		
		if(m_pMyVehicle->GetVehicleType()==VEHICLE_TYPE_BMX)
			vecLocalSeatPosition.z -= 0.001f*CMaths::Abs(((CBmx *)m_pMyVehicle)->m_fControlJump);
	}
	else
	if(this == m_pMyVehicle->pPassengers[0])
	{
		if(m_pMyVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE || m_pMyVehicle->GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
			vecLocalSeatPosition = pModelInfo->GetBackSeatPosn();
		else
			vecLocalSeatPosition = pModelInfo->GetFrontSeatPosn();
	}
	else
	if(this == m_pMyVehicle->pPassengers[1])
	{
		vecLocalSeatPosition = pModelInfo->GetBackSeatPosn();
		vecLocalSeatPosition.x = -vecLocalSeatPosition.x;
	}
	else
	if(this == m_pMyVehicle->pPassengers[2])
	{
		vecLocalSeatPosition = pModelInfo->GetBackSeatPosn();
	}
	else
	{	// this is a bus, it doesn't matter where we calc position because they wont be rendered
		// but we need to keep it updated for other code
		vecLocalSeatPosition = pModelInfo->GetFrontSeatPosn();
	}

//	vecLocalSeatPosition.z += vecPosn.z;
	
	if(m_pMyVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
	{
		((CBike *)m_pMyVehicle)->CalculateLeanMatrix();
		Matrix = ((CBike *)m_pMyVehicle)->m_LeanMatrix;
	}
	else if(m_pMyVehicle->GetModelIndex()==MODELID_CAR_COMBINE)
	{
		float fChassisZPos = 0.0f;
		if(((CAutomobile *)m_pMyVehicle)->m_aCarNodes[CAR_CHASSIS])
		{
			Matrix2.Attach(RwFrameGetMatrix(((CAutomobile *)m_pMyVehicle)->m_aCarNodes[CAR_CHASSIS]));
			fChassisZPos = Matrix2.GetTranslate().z;
			Matrix2.Detach();
		}
		
		Matrix2.SetTranslate(0.0f,0.0f,-fChassisZPos);
		Matrix2.RotateY(((CAutomobile *)m_pMyVehicle)->GunOrientation);
		Matrix2.Translate(0.0f,0.0f,fChassisZPos);
		Matrix *= Matrix2;
	}
	
	vecFinalPosition = Multiply3x3(Matrix, vecLocalSeatPosition);
	Matrix.tx += vecFinalPosition.x;
	Matrix.ty += vecFinalPosition.y;
	Matrix.tz += vecFinalPosition.z;
	Matrix2.SetUnity();
	
	//if(m_pMyVehicle->m_nVehicleFlags.bIsVan)
	if(ANIM_VEH_VAN==(m_pMyVehicle->pHandling->AnimGroup+ANIM_VEH_STD))
	{
		if(this == m_pMyVehicle->pPassengers[1])
		{
			m_fCurrentHeading = m_pMyVehicle->GetHeading()-HALF_PI;
			Matrix2.SetTranslate(0,0,0);
			Matrix2.RotateZ(-HALF_PI);
			Matrix2.Translate(0.0f,0.6f,0.0f);
			Matrix *= Matrix2;
		}
		else
		if(this == m_pMyVehicle->pPassengers[2])
		{
			m_fCurrentHeading = m_pMyVehicle->GetHeading()+HALF_PI;
			Matrix2.SetTranslate(0,0,0);
			Matrix2.RotateZ(HALF_PI);
			Matrix *= Matrix2;
		}
		else
			m_fCurrentHeading = m_pMyVehicle->GetHeading(); 
	}
	else
		m_fCurrentHeading = m_pMyVehicle->GetHeading(); //to keep ped code aware of direction (for looking about etc)
	
	m_fDesiredHeading = m_fCurrentHeading;
	SetMatrix(Matrix);

#ifdef DEBUG
//	if(GetDebugDisplay()){
//		CDebug::DebugLine3D(Matrix.tx, Matrix.ty, Matrix.tz-0.5f,
//		 Matrix.tx, Matrix.ty, Matrix.tz+ 0.5f, 0xff0000ff, 0xff0000ff);
//		CDebug::DebugLine3D(Matrix.tx-0.5f, Matrix.ty, Matrix.tz,
//		 Matrix.tx+0.5f, Matrix.ty, Matrix.tz, 0xff0000ff, 0xff0000ff);
//		CDebug::DebugLine3D(Matrix.tx, Matrix.ty-0.5f, Matrix.tz,
//		 Matrix.tx, Matrix.ty+0.5f, Matrix.tz, 0xff0000ff, 0xff0000ff);
//	}
#endif
}
/*
void CPed::LookForSexyPeds()
{
	int32 i = 0;

	if(!IsPedInControl()&&(m_nPedState != PED_DRIVING))
		return;
	
	// scan for nice-looking women to letch at
	if((m_nLookTimer < CTimer::GetTimeInMilliseconds()) && (m_nPedType == PEDTYPE_CIVMALE))
	{
		for (i = 0; i < m_NumClosePeds; i++)
		{
			// don't check against our ped
//			if (OurPedCanSeeThisOne(m_apClosePeds[i]))
			if (CanSeeEntity(m_apClosePeds[i])) // use this: faster
			{
				if(	(CVector(GetPosition() - m_apClosePeds[i]->GetPosition()).Magnitude() < 10.0f)&&
					(m_apClosePeds[i]->m_pPedStats->m_nSexiness > m_pPedStats->m_nSexiness)&&
					(m_apClosePeds[i]->m_nPedType ==  PEDTYPE_CIVFEMALE))
				{
					SetLookFlag(m_apClosePeds[i],false); // not persistant looking
					m_nLookTimer = (CTimer::GetTimeInMilliseconds() + 4000);//% MAX_TIMER_MILLISECONDS;
					Say(AE_PED_EYING_UP_ANOTHER_PED);
					return;
				}
			}
		}
		m_nLookTimer = (CTimer::GetTimeInMilliseconds() + 10000);//Set look timer anyway
	}
}
*/

/*
void CPed::LookForSexyCars()
{
	CEntity *ppResults[8]; // whatever.. temp list
	CVehicle *pVehicle;
	int16 Num,i, nBest;
	UInt32 nHighestValue;

	if(!IsPedInControl()&&(m_nPedState != PED_DRIVING))
		return;

	nBest = 0;
	nHighestValue = 0;

	if(m_nLookTimer < CTimer::GetTimeInMilliseconds())
	{
		CWorld::FindObjectsInRange(CVector(GetPosition()), 10.0f, true, &Num, 6, ppResults, 0, 1, 0, 0, 0);	// Just Cars we're interested in
		// cars in range stored in ppResults
		for(i=0; i<Num; i++) // find best car
		{
			pVehicle = (CVehicle *)ppResults[i];
			if((pVehicle != m_pMyVehicle)&&(nHighestValue < pVehicle->pHandling->nMonetaryValue))
			{
				nHighestValue = pVehicle->pHandling->nMonetaryValue;
				nBest = i;
			}
		}
		if((Num > 0)&&(nHighestValue > 40000))
		{
			SetLookFlag(ppResults[nBest],false); // not persistant looking
		}
		m_nLookTimer = (CTimer::GetTimeInMilliseconds() + 10000);//Set look timer anyway
	}
}
*/

/*
#define INTEREST_NODE_SCAN_RANGE (15.0f)
#define INTEREST_NODE_APPROACH_RANGE (8.0f)
// function checks for lookat nodes attached to buildings/vehicles/objects
// the search ends with the 1st node within the defined ApproachRange
// the interest level of the node along with the ped's state and personality
// is then used to determine if the ped will investigate the node
bool8 CPed::LookForInterestingNodes()
{
	// check if ped is interested in interesting things -> may be on a mission

	// only do scan every 16 frames or so (should equate to every 0.5sec approx)
	if ( ( (this->RandomSeed + CTimer::m_FrameCounter) & 7) == 0// changed 7 from, 15
		 && CTimer::GetTimeInMilliseconds() > m_nChatTimer )
	{
		uint8 nNumNodes = 0;
		uint8 nRandInterestThreshold = CGeneral::GetRandomNumber() & 255;
		bool8 bFoundNode = false;
		CBaseModelInfo* pModelInfo = NULL;
		C2dEffect *p2dEffect = NULL;
		CEntity *pEntity = NULL;

		int32 nLeft = MAX(WORLD_WORLDTOSECTORX(GetPosition().x - INTEREST_NODE_SCAN_RANGE), 0);
		int32 nBottom = MAX(WORLD_WORLDTOSECTORY(GetPosition().y - INTEREST_NODE_SCAN_RANGE), 0);
		int32 nRight = MIN(WORLD_WORLDTOSECTORX(GetPosition().x + INTEREST_NODE_SCAN_RANGE), WORLD_WIDTHINSECTORS);
		int32 nTop = MIN(WORLD_WORLDTOSECTORY(GetPosition().y + INTEREST_NODE_SCAN_RANGE), WORLD_DEPTHINSECTORS);
		CWorld::AdvanceCurrentScanCode();

		for (int32 y = nBottom; y<=nTop && !bFoundNode; y++)
		{
			for (int32 x = nLeft; x<=nRight && !bFoundNode; x++)
			{
				CSector& sector = CWorld::GetSector(x, y);
				CRepeatSector& rsector = CWorld::GetRepeatSector(x, y);
				// scan list of vehicles 1st (ice-cream vans most desirable!)
				CPtrNode* pNode = rsector.GetOverlapVehiclePtrList().GetHeadPtr();
				while (pNode != NULL && !bFoundNode)
				{
					pEntity = (CEntity*)pNode->GetPtr();
					pNode = pNode->GetNextPtr();
					if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode())
					{
						pEntity->SetScanCode(CWorld::GetCurrentScanCode());
						pModelInfo = CModelInfo::GetModelInfo(pEntity->m_nModelIndex);
						if( (nNumNodes = pModelInfo->GetNum2dEffects()) )
						{
							for(int16 i=0; i<nNumNodes; i++){
								p2dEffect = pModelInfo->Get2dEffect(i);
								if( p2dEffect->m_type == ET_LOOKATPOINT && p2dEffect->attr.k.m_interest >= nRandInterestThreshold &&
								   (pEntity->m_mat*p2dEffect->m_posn - GetPosition()).MagnitudeSqr() < INTEREST_NODE_APPROACH_RANGE*INTEREST_NODE_APPROACH_RANGE){
									bFoundNode = true;
									break;
								}
							}
						}
					}
				}
				// scan list of objects next
				pNode = rsector.GetOverlapObjectPtrList().GetHeadPtr();
				while (pNode != NULL && !bFoundNode)
				{
					pEntity = (CEntity*)pNode->GetPtr();
					pNode = pNode->GetNextPtr();
					if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode())
					{
						pEntity->SetScanCode(CWorld::GetCurrentScanCode());
						pModelInfo = CModelInfo::GetModelInfo(pEntity->m_nModelIndex);
						if( (nNumNodes = pModelInfo->GetNum2dEffects()) )
						{
							for(int16 i=0; i<nNumNodes; i++){
								p2dEffect = pModelInfo->Get2dEffect(i);
								if( p2dEffect->m_type == ET_LOOKATPOINT && p2dEffect->attr.k.m_interest >= nRandInterestThreshold &&
								   (pEntity->m_mat*p2dEffect->m_posn - GetPosition()).MagnitudeSqr() < INTEREST_NODE_APPROACH_RANGE*INTEREST_NODE_APPROACH_RANGE){
									bFoundNode = true;
									break;
								}
							}
						}
					}
				}
				// scan list of buildings last (shop windows etc) USE OVERLAP LIST 1st - more likely to be near edges of buildings
				pNode = sector.GetOverlapBuildingPtrList().GetHeadPtr();
				while (pNode != NULL && !bFoundNode)
				{
					pEntity = (CEntity*)pNode->GetPtr();
					pNode = pNode->GetNextPtr();
					if (pEntity->GetScanCode() != CWorld::GetCurrentScanCode())
					{
						pEntity->SetScanCode(CWorld::GetCurrentScanCode());
						pModelInfo = CModelInfo::GetModelInfo(pEntity->m_nModelIndex);
						if( (nNumNodes = pModelInfo->GetNum2dEffects()) )
						{
							for(int16 i=0; i<nNumNodes; i++){
								p2dEffect = pModelInfo->Get2dEffect(i);
								if( p2dEffect->m_type == ET_LOOKATPOINT && p2dEffect->attr.k.m_interest >= nRandInterestThreshold &&
								   (pEntity->m_mat*p2dEffect->m_posn - GetPosition()).MagnitudeSqr() < INTEREST_NODE_APPROACH_RANGE*INTEREST_NODE_APPROACH_RANGE){
									bFoundNode = true;
									break;
								}
							}
						}
					}
				}				
				// scan list of buildings last (shop windows etc)
//				pNode =  sector.GetBuildingPtrList().GetHeadPtr();
//				while (pNode != NULL && !bFoundNode)
//				{
//					pEntity = (CEntity*)pNode->GetPtr();
//					pNode = pNode->GetNextPtr();
//					pModelInfo = CModelInfo::GetModelInfo(pEntity->m_nModelIndex);
//					if( (nNumNodes = pModelInfo->GetNum2dEffects()) )
//					{
//						for(int16 i=0; i<nNumNodes; i++){
//							p2dEffect = pModelInfo->Get2dEffect(i);
//							if( p2dEffect->m_type == ET_LOOKATPOINT && p2dEffect->attr.k.m_interest >= nRandInterestThreshold &&
//							   (pEntity->m_mat*p2dEffect->m_posn - GetPosition()).MagnitudeSqr() < INTEREST_NODE_APPROACH_RANGE*INTEREST_NODE_APPROACH_RANGE){
//								bFoundNode = true;
//								break;
//							}
//						}
//					}
//				}
				// end scan sector
			}
		}
		
		if(bFoundNode)	// found a look at node, go and look at it if want to
		{
			// need to transform heading, as given in relation to un-rotated object
			CVector vecHeading = pEntity->m_mat * p2dEffect->attr.k.m_dir - pEntity->GetPosition();
			float fAngle = CGeneral::GetRadianAngleBetweenPoints(vecHeading.x, vecHeading.y, 0.0f, 0.0f);

			if( (CGeneral::GetRandomNumber() & 255) > (this->RandomSeed & 255) )
			{
				CVector2D vec2dPos = CVector2D::ConvertTo2D(pEntity->m_mat*p2dEffect->m_posn);
				switch(p2dEffect->attr.k.m_type)
				{
					case QT_ATM:
						SetInvestigateEvent(EVENT_ATM, vec2dPos, 0.1f, 15000, fAngle);
						
						break;
					case LT_SHOPWINDOW: // changed time to add another 4 secs
						SetInvestigateEvent(EVENT_SHOPWINDOW, vec2dPos, 1.0f, CGeneral::GetRandomNumberInRange(8000, 8500 + p2dEffect->attr.k.m_interest*10), fAngle);

//			TheCamera.TakeControl(this, CCam::MODE_FOLLOWPED, 2);

						break;
					//case LT_SHOPSTALL:
					case LT_ICECREAM:
						// add calls here
						break;		
				}
				return true;
			}
			else
			{
				m_nChatTimer = CTimer::GetTimeInMilliseconds() + 2000;
				SetLookFlag(fAngle, true);
				SetLookTimer(1000);
			}
		}
	}
	return false;
}
*/

//
// name:		RemoveAnimsFromAnimationBlock
// description:	Remove any animations applied to this clump from the anim block indicated
void RemoveAnimsFromAnimationBlock(RpClump* pClump, const char *pAnimBlock)
{
	CAnimBlock* pBlock = CAnimManager::GetAnimationBlock(pAnimBlock);

	// find if there is a riot/stripper anim
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetFirstAssociation(pClump);
	while(pAnim)
	{
		CAnimBlendHierarchy* pHier = pAnim->GetAnimHierarchy();
		int32 index = pHier - CAnimManager::GetAnimation(0);
		if(index >= pBlock->m_animIndex && index < pBlock->m_animIndex+pBlock->m_numAnims)
			pAnim->SetBlendDelta(-1000.0f);
		pAnim = RpAnimBlendGetNextAssociation(pAnim);
	}
}

//
// name:		PlayRandomAnimationsFromAnimBlock
// description:	Play random animations from a group of anims (used by rioters and strippers)
void PlayRandomAnimationsFromAnimBlock(CPed* pPed, AssocGroupId grpId, uint32 animId, int32 numAnims)
{
/*
	if(!pPed->IsPedInControl())
		return;
	CAnimBlock* pBlock = CAnimManager::GetAnimationBlock(CAnimManager::GetAnimGroupName(grpId));

	// find if there is a riot anim
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)pPed->GetRwObject());
	while(pAnim)
	{
		CAnimBlendHierarchy* pHier = pAnim->GetAnimHierarchy();
		int32 index = pHier - CAnimManager::GetAnimation(0);
		if(index >= pBlock->m_animIndex && index < pBlock->m_animIndex+pBlock->m_numAnims)
			break;
		pAnim = RpAnimBlendGetNextAssociation(pAnim);
	}
	
	if(CTimer::GetTimeInMilliseconds() > pPed->m_nWaitTimer && pAnim)
		pAnim->ClearFlag(ABA_FLAG_ISLOOPED);
	// if wait timer has run out or there is a no riot anim blend in a new riot anim

	if(pAnim == NULL || pAnim->GetBlendDelta() < 0.0f)	
	{
		uint32 newAnim;
		
		// choose new anim but not the same as the previous one
		do{
			newAnim = animId+CGeneral::GetRandomNumberInRange(0, numAnims);
		} while(pAnim && pAnim->GetAnimId() == newAnim);
		
		// create new anim
		pAnim = CAnimManager::BlendAnimation((RpClump*)pPed->m_pRwObject, grpId, 
				(AnimationId)newAnim, 3.0f);
		pAnim->SetFinishCallback(CPed::FinishedWaitCB, pPed);
		if(pAnim->IsFlagSet(ABA_FLAG_ISLOOPED))
			pPed->m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(3000,8000);
		else
			pPed->m_nWaitTimer = 	CTimer::GetTimeInMilliseconds() + 8000;
	}
*/
}

void CPed::RestoreHeadingRate()
{ 
	m_fHeadingChangeRate = m_pPedStats->m_fMaxHeadingChange; 
}

void CPed::RestoreHeadingRateCB(CAnimBlendAssociation* pAnim, void* pData)
{
	((CPed *)pData)->m_fHeadingChangeRate = ((CPed *)pData)->m_pPedStats->m_fMaxHeadingChange;
}

void CPed::FlagToDestroyWhenNextProcessed(void)
{
	m_nFlags.bRemoveFromWorld = TRUE;

//	If the character is in a vehicle then check whether the character
//	is the driver or a passenger and remove the relevant pointer to this character
//	in the vehicle class
	if (m_nPedFlags.bInVehicle && m_pMyVehicle)
	{
		ASSERT(m_pMyVehicle);
		
		if (m_pMyVehicle->pDriver == this)
		{	//	This character is the driver of the vehicle
			TIDYREF_NOTINWORLD(m_pMyVehicle->pDriver, (CEntity **) &m_pMyVehicle->pDriver);
			m_pMyVehicle->pDriver = NULL;
			
			if (IsPlayer())
			{
				if (m_pMyVehicle->GetStatus() != STATUS_WRECKED)
				{
					m_pMyVehicle->SetStatus(STATUS_ABANDONED);
				}

				// must clean up any passengers in the player's car before RemoveReferencesToPlayer is called
				// Only do this for player's car? What is this for?
/*				
				for (Int32 Pass = 0; Pass < m_pMyVehicle->m_nMaxPassengers; Pass++)
				{
					if (m_pMyVehicle->pPassengers[Pass]) 
					{
						m_pMyVehicle->pPassengers[Pass]->m_nPedFlags.bInVehicle = FALSE;
						m_pMyVehicle->pPassengers[Pass]->m_pMyVehicle = NULL;
						m_pMyVehicle->pPassengers[Pass] = NULL;
						m_pMyVehicle->m_nNumPassengers--;
					}
				}
*/
			}
		}
		else
		{	// This character is a passenger
			m_pMyVehicle->RemovePassenger(this);
		}

		m_nPedFlags.bInVehicle = FALSE;
		if (IsVehiclePointerValid(m_pMyVehicle))
		{
			TIDYREF(m_pMyVehicle, (CEntity**)&m_pMyVehicle);
		}
		m_pMyVehicle = NULL;
		
		if (CharCreatedBy == MISSION_CHAR)
		{	// Willie needs mission peds in cars to be set to PED_DEAD when the car explodes
			SetPedState(PED_DEAD);
		}
		else
		{	// non-mission peds are set to PED_NONE so that they don't get a white chalk outline
			SetPedState(PED_NONE);
		}
		//m_pAnim = NULL;			// Only do this for Player Peds?
//		SetDead();
	}
}

/*
void CPed::SetSolicit(uint32 nTime)
{
	if((m_nPedState != PED_SOLICIT)&&
	  (IsPedInControl())&&
	  (m_pObjectiveVehicle)&&
	  (CharCreatedBy != MISSION_CHAR)&&
	  (m_pObjectiveVehicle->m_nNumGettingIn == 0)&&
	  (CTimer::GetTimeInMilliseconds() < m_nObjectiveTimer)
	  )
	{
		if(m_nDoor == CAR_DOOR_LF)
			m_fDesiredHeading = m_pObjectiveVehicle->GetHeading()-HALF_PI;
		else
			m_fDesiredHeading = m_pObjectiveVehicle->GetHeading()+HALF_PI;
		if(ABS(m_fDesiredHeading-m_fCurrentHeading) < HALF_PI)
		{
			m_nChatTimer = CTimer::GetTimeInMilliseconds() + nTime;// hack to force proz to turn physically around
			if((!m_pObjectiveVehicle->m_bIsVan)&&(!m_pObjectiveVehicle->m_bIsBus))
				m_pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_CAR_HOOKERTALK, 4.0f);
			SetPedState(PED_SOLICIT);
		}
	}
}
*/

/*
void CPed::Solicit()
{
	if((m_nChatTimer < CTimer::GetTimeInMilliseconds())||(!m_pObjectiveVehicle))
	{
		// finished the chatting, get in,
		if(!m_pObjectiveVehicle)
		{
			RestorePreviousState();
			RestorePreviousObjective();
			SetObjectiveTimer(10000);// stop prozzer from going back to car
		}
		else
		{
			if(CWorld::Players[CWorld::PlayerInFocus].Score > HOOKER_FEE)
			{
				m_pAnim = NULL; // clear link to HOOKERTALK anim
				SetLeader(m_pObjectiveVehicle->pDriver);
				Say(AE_PED_SOLICIT);
			}
			else
				m_pObjectiveVehicle = NULL;
		}
	}
	else
	{
		CVector vecDiff;
		CVector vecTalkPos;
		
		vecTalkPos = GetPositionToOpenCarDoor(m_pObjectiveVehicle, m_nDoor, 0.0f);
		
		Say(AE_PED_SOLICIT);
		
		if(FindPlayerVehicle() == m_pObjectiveVehicle)
		{
			CPed *pPlayerPed = (CPed*)FindPlayerPed();
			
			pPlayerPed->Say(AE_PED_SOLICIT);
		}
		
		
		SetMoveState(PEDMOVE_STILL);
		
		m_fDesiredHeading = DEGTORAD(CGeneral::GetAngleBetweenPoints( vecTalkPos.x, vecTalkPos.y, GetPosition().x, GetPosition().y));
		vecDiff = GetPosition() - vecTalkPos;

		if(m_fDesiredHeading > TWO_PI)
		{
			m_fDesiredHeading -= TWO_PI;
		}
		else
		if(m_fDesiredHeading < 0)
		{
			m_fDesiredHeading += TWO_PI;
		}
		
		if(vecDiff.MagnitudeSqr() > 1.0f*1.0f) 
		{
			CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_HOOKERTALK);
			if(pAnim)
			{
				pAnim->SetBlendDelta(-1000);
				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				
			}
			RestorePreviousState();
			RestorePreviousObjective();
			SetObjectiveTimer(10000);// stop prozzer from going back to car
		}
	}
}
*/

/*
void CPed::SetBuyIceCream()
{
	
	if((m_nPedState != PED_BUY_ICE_CREAM)&&(IsPedInControl())&&(m_pObjectiveVehicle))
	{
		//m_fDesiredHeading = m_pObjectiveVehicle->GetHeading()-HALF_PI;
		//if(ABS(m_fDesiredHeading-m_fCurrentHeading) < HALF_PI)
		{
			//m_nChatTimer = CTimer::GetTimeInMilliseconds() + 3000;// use chat timer to buy ice cream
			// would like to have an anim here for buying the ice cream if possible
			SetPedState(PED_BUY_ICE_CREAM);
		}
	}
}
*/


/*
void CPed::BuyIceCream()
{
	//if((m_nChatTimer < CTimer::GetTimeInMilliseconds())||(!m_pObjectiveVehicle))
	//{
	//	// finished the frozen dairy product transaction, we may now fuck off,
	//	RestorePreviousState();
	//	RestorePreviousObjective();
	//	SetObjectiveTimer(10000);// stop ped from buying another for a while
	//}
	//else
	{
		CVector vecDiff;
		
		
		//m_fDesiredHeading = m_pObjectiveVehicle->GetHeading()-HALF_PI;

		//vecDiff = GetPosition() - m_pObjectiveVehicle->GetPosition();

		//if(m_fDesiredHeading > TWO_PI)
		//{
		//	m_fDesiredHeading -= TWO_PI;
		//}
		//else
		//if(m_fDesiredHeading < 0)
		//{
		//	m_fDesiredHeading += TWO_PI;
		//}
		
		//if(CMaths::Abs(m_fDesiredHeading-m_fCurrentHeading)<0.5f)
		{
		    if(m_pObjectiveVehicle)
		    {
		        if((m_pObjectiveVehicle->pDriver)&&(CTimer::GetTimeInMilliseconds()>m_nChatTimer))
		        {
        		    SetChat(m_pObjectiveVehicle->pDriver,8000);
        		    m_pObjectiveVehicle->pDriver->SetChat(this,8000);
		        }
		        else
		        {
		            SetObjective(NO_OBJ);
		            SetWanderPath(CGeneral::GetRandomNumberInRange(0,8));
		        }
            }
            else
            {
                SetObjective(NO_OBJ);
	            SetWanderPath(CGeneral::GetRandomNumberInRange(0,8));
            }
		}
		
		
		//if(vecDiff.Magnitude() > 2.0f) 
		//{
		//	RestorePreviousState();
		//	RestorePreviousObjective();
		//	SetObjectiveTimer(10000);// stop ped from buying another for a while
		//}
	}
}
*/


/*
#define AREA_AROUND_CAR	(0.5f)

bool CPed::PossiblyFindBetterPosToSeekCar(CVector *vecDoorPos, CVehicle *pVehicle)
{
	CVector vecMyPos, vecMidwayToTarget, vecTestDoorPos;
	bool bNewPos = false;
	
	
	vecMyPos = GetPosition();
	vecMyPos.z = (vecDoorPos->z - 0.5f);
	
	vecTestDoorPos.x = vecDoorPos->x;
	vecTestDoorPos.y = vecDoorPos->y;
	vecTestDoorPos.z = (vecDoorPos->z - 0.5f);
	
	vecMidwayToTarget = (vecTestDoorPos + vecMyPos)/2.0f;
	
	//is our way to the door position blocked?
	if(CWorld::TestSphereAgainstWorld(vecMidwayToTarget, 0.25f, pVehicle, false, true, false, false, false))
	{
		//get four corner points around the car
		CVector vecCornerPos[4];
		float Angles[4];
		const CVector& bBoxVMin = pVehicle->GetColModel().GetBoundBoxMin();
		const CVector& bBoxVMax = pVehicle->GetColModel().GetBoundBoxMax();
		bool bClear[4];

		vecCornerPos[0].x = bBoxVMin.x-AREA_AROUND_CAR;//BL
		vecCornerPos[0].y = bBoxVMin.y-AREA_AROUND_CAR;
		vecCornerPos[0].z = 0;
		vecCornerPos[1].x = bBoxVMax.x+AREA_AROUND_CAR;//BR
		vecCornerPos[1].y = bBoxVMin.y-AREA_AROUND_CAR;
		vecCornerPos[1].z = 0;
		vecCornerPos[2].x = bBoxVMin.x-AREA_AROUND_CAR;//TL
		vecCornerPos[2].y = bBoxVMax.y+AREA_AROUND_CAR;
		vecCornerPos[2].z = 0;
		vecCornerPos[3].x = bBoxVMax.x+AREA_AROUND_CAR;//TR
		vecCornerPos[3].y = bBoxVMax.y+AREA_AROUND_CAR;
		vecCornerPos[3].z = 0;
		
		vecCornerPos[0] = pVehicle->GetMatrix()*vecCornerPos[0];
		vecCornerPos[1] = pVehicle->GetMatrix()*vecCornerPos[1];
		vecCornerPos[2] = pVehicle->GetMatrix()*vecCornerPos[2];
		vecCornerPos[3] = pVehicle->GetMatrix()*vecCornerPos[3];
		vecMyPos -= pVehicle->GetPosition();
		vecMyPos = pVehicle->GetMatrix()*vecMyPos;

		Angles[0] = CGeneral::LimitRadianAngle(CMaths::ATan2(vecCornerPos[0].x - vecMyPos.x, vecCornerPos[0].y - vecMyPos.y)-pVehicle->GetHeading());
		Angles[1] = CGeneral::LimitRadianAngle(CMaths::ATan2(vecCornerPos[1].x - vecMyPos.x, vecCornerPos[1].y - vecMyPos.y)-pVehicle->GetHeading());
		Angles[2] = CGeneral::LimitRadianAngle(CMaths::ATan2(vecCornerPos[2].x - vecMyPos.x, vecCornerPos[2].y - vecMyPos.y)-pVehicle->GetHeading());
		Angles[3] = CGeneral::LimitRadianAngle(CMaths::ATan2(vecCornerPos[3].x - vecMyPos.x, vecCornerPos[3].y - vecMyPos.y)-pVehicle->GetHeading());

		bClear[0] = (!(Angles[0] > -PI && Angles[0] < -HALF_PI));
		bClear[1] = (!(Angles[1] > HALF_PI && Angles[1] < PI));
		bClear[2] = (!(Angles[2] < 0 && Angles[2] > -HALF_PI));
		bClear[3] = (!(Angles[3] > 0 && Angles[3] < HALF_PI));
	
		// depending on which door we are going for, select the correct two
		if((m_nDoor == CAR_DOOR_LF)||(m_nDoor == CAR_DOOR_LR))
		{
			if(m_nDoor == CAR_DOOR_LF)
			{
				if(bClear[2])
				{
					vecTestDoorPos = vecCornerPos[2];
					bNewPos = true;
				}
				else
				if(bClear[0])
				{
					vecTestDoorPos = vecCornerPos[0];
					bNewPos = true;
				}
				else	// no, is 2 clear
				if(bClear[3])
				{
					vecTestDoorPos = vecCornerPos[3];
					bNewPos = true;
				}
				else
				if(bClear[1])
				{	// 0 is clear
					vecTestDoorPos = vecCornerPos[1];
					bNewPos = true;
				}
			}
			else
			{
				if(bClear[0])
				{
					vecTestDoorPos = vecCornerPos[0];
					bNewPos = true;
				}
				else	// no, is 2 clear
				if(bClear[2])
				{
					vecTestDoorPos = vecCornerPos[2];
					bNewPos = true;
				}
				else
				if(bClear[1])
				{	// 0 is clear
					vecTestDoorPos = vecCornerPos[1];
					bNewPos = true;
				}
				else
				if(bClear[3])
				{
					vecTestDoorPos = vecCornerPos[3];
					bNewPos = true;
				}
			}
		}
		else
		if((m_nDoor == CAR_DOOR_RF)||(m_nDoor == CAR_DOOR_RR))
		{
			if(m_nDoor == CAR_DOOR_RF)
			{
				if(bClear[3])
				{
					vecTestDoorPos = vecCornerPos[3];
					bNewPos = true;
				}
				else
				if(bClear[1])
				{
					vecTestDoorPos = vecCornerPos[1];
					bNewPos = true;
				}
				else
				if(bClear[2])
				{
					vecTestDoorPos = vecCornerPos[2];
					bNewPos = true;
				}
				else
				if(bClear[0])
				{	// 0 is clear
					vecTestDoorPos = vecCornerPos[0];
					bNewPos = true;
				}
			}			
			else
			{
				if(bClear[1])
				{
					vecTestDoorPos = vecCornerPos[1];
					bNewPos = true;
				}
				else
				if(bClear[3])
				{
					vecTestDoorPos = vecCornerPos[3];
					bNewPos = true;
				}
				else
				if(bClear[0])
				{	// 0 is clear
					vecTestDoorPos = vecCornerPos[0];
					bNewPos = true;
				}
				else
				if(bClear[2])
				{
					vecTestDoorPos = vecCornerPos[2];
					bNewPos = true;
				}
			}			
		}
	}
	
	if(bNewPos)
	{
		vecMyPos = GetPosition() - vecTestDoorPos;
		vecMyPos.z = 0.0f;
		
		if(vecMyPos.MagnitudeSqr() > 0.5f*0.5f)
		{
			vecDoorPos->x = vecTestDoorPos.x;
			vecDoorPos->y = vecTestDoorPos.y;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}
*/

/*
void CPed::SetLeader(CPed *Leader) 
{
	pLeader = Leader;
	if (pLeader)
	{	
	    pLeader->m_nPedFlags.bIsLeader=true;
	    
		// Leader is set to NULL for leaving groups
		AssertPedPointerValid(pLeader);
		
		REGREF(pLeader, (CEntity**)&pLeader);
	}
}
*/

/*
bool CPed::CanPedJumpThis(CEntity *pDamageEntity, CVector *pDamageNormal)
{
	CVector vecWaistPos, vecTestAheadPos;
	vecWaistPos = GetPosition();
	vecTestAheadPos = m_mat.GetForward();	// peds should always be vertical
	
	// always want to be able to jump out of swimming pools
	if(CColPoint::IsWater(m_LastMaterialToHaveBeenStandingOn))
		return true;

	if(pDamageNormal!=NULL)
	{
		if(pDamageNormal->z > 0.17f)	// 10deg upwards
		{
			// if collision is nearly flat, we shouldn't need to jump this
			if(pDamageNormal->z > 0.9f)
				return false;
		
			CColModel &PedCol = CModelInfo::GetColModel(GetModelIndex());
			ASSERTMSG(PedCol.m_nNoOfSpheres==3, "Someone has changed the default ped col model");
			// try and work out the exact height of the collision
			vecWaistPos.z += PedCol.m_pSphereArray[0].m_vecCentre.z - PedCol.m_pSphereArray[0].m_fRadius*pDamageNormal->z;
			// then add 10cm
			vecWaistPos.z += 0.05f;
			
			float fHorizontalNormalMag = pDamageNormal->Magnitude2D();
			// and for collisions with relatively flat planes, extend the line check
			if(pDamageNormal->z > 0.5f)
			{
				vecTestAheadPos.x = -pDamageNormal->x;
				vecTestAheadPos.y = -pDamageNormal->y;
				vecTestAheadPos.z = 0.0f;
				vecTestAheadPos /= fHorizontalNormalMag;
				vecTestAheadPos += vecTestAheadPos*fHorizontalNormalMag*PedCol.m_pSphereArray[0].m_fRadius;
				vecTestAheadPos *= MIN(4.0f, 2.0f/fHorizontalNormalMag);
			}
			else
			{
				// move collision test forward by where the collision occured in x,y plane
				vecTestAheadPos += vecTestAheadPos*fHorizontalNormalMag*PedCol.m_pSphereArray[0].m_fRadius;
			}
		}
		else
			vecWaistPos.z -= 0.15f;
	}
	else
		vecWaistPos.z -= 0.15f;
	
	//Do a test from the ped's waist to a metre in front of the ped at waist height.
	if (CWorld::GetIsLineOfSightClear(vecWaistPos, vecWaistPos + vecTestAheadPos, true, false, false, true, false))
	{ 
        return true;
   	}

	return false;
}
*/


/*
//
// Start the Ped jumping
//
#define JUMPOFF_RIGHTFOOT_STARTFRAME	(2.0f/30.0f)
#define JUMPOFF_RIGHTFOOT_ENDFRAME		(12.0f/30.0f)
void CPed::SetJump()
{
	if(m_nPedFlags.bInVehicle || m_nPedState == PED_JUMP) return;

	// if the launch anim is already there, need to wait till it's been removed to jump again
	// otherwise could get stuck in jump state with no anims (i.e. stuck stuck)
	CAnimBlendAssociation* pAnim;
	pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_JUMP_LAUNCH);
	if(pAnim)
		return;

	// don't want to let ped jump up steep hills (cause they're not supposed to be able to get up them!)	
	if(CColPoint::IsSteepSlope(m_LastMaterialToHaveBeenStandingOn))
	{
		if(DotProduct(m_mat.GetForward(), m_vecDamageNormal) < 0.0f)
			return;
	}
	
	float speed = m_vecCurrentVelocity.Magnitude() * FRAMES_PER_SECOND;
	
	SetStoredState();
	SetPedState(PED_JUMP);
	
	pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_JUMP_LAUNCH, 8.0f);
	pAnim->SetFinishCallback(FinishLaunchCB, this);
	
	m_fDesiredHeading = m_fCurrentHeading;
}
*/

/*
// called once launch anim has finished
#define MINJUMP_SPEED		(0.1f)	// standing/walking jump speed
#define RUNJUMP_SPEED		(0.17f)	// jump speed when running
#define SPRINTJUMP_SPEED	(0.22f)	// jump speed when sprinting
#define PLAYERPED_JUMPFORCE	(8.5f)
#define PED_JUMPFORCE	(4.5f)


void CPed::FinishLaunchCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CAnimBlendAssociation* pRunAnim = NULL;
	CPed* pPed = (CPlayerPed*)pData;
	
	// don't want to go on into jump if ped has been killed or something!
	if(pPed->m_nPedState!=PED_JUMP)
		return;

	CEntity *pHitHead = NULL;
	CVector vecTemp = pPed->GetPosition() + 0.09f*pPed->m_mat.GetForward();
	// collision model sphere 0 is head for peds
	vecTemp.z += CModelInfo::GetColModel(pPed->m_nModelIndex).m_pSphereArray[2].m_vecCentre.z + 0.35f;
	pHitHead = CWorld::TestSphereAgainstWorld(vecTemp, 0.25f, NULL, true, true, false, true, false);
	if(pHitHead==NULL){
		vecTemp = vecTemp + 0.15f*pPed->m_mat.GetForward();
		vecTemp.z += 0.15f;
		pHitHead = CWorld::TestSphereAgainstWorld(vecTemp, 0.25f, NULL, true, true, false, true, false);
	}
	
	// if ped is in a tightly constrained area (like a doorway) force no jumping allowed
	if( pHitHead==NULL && CCullZones::CamStairsForPlayer()
	&& CCullZones::FindZoneWithStairsAttributeForPlayer()!=NULL )
	{
		pHitHead = (CEntity *)pPed;
	}

	if(pHitHead)
	{
		pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
		pAnim = CAnimManager::BlendAnimation((RpClump*)pPed->m_pRwObject, ANIM_STD_PED, ANIM_STD_HIT_WALL, 8.0f);
		pAnim->ClearFlag(ABA_FLAG_ISFINISHAUTOREMOVE);
		pAnim->SetFinishCallback(FinishHitHeadCB, pPed);
		pPed->m_nPedFlags.bIsLanding = true;
		return;
	}
	
/////////////////////////////////////////
//	printf("Player Jump: Ini-Speed %f,%f,%f Mag:%f\n",pPed->m_vecMoveSpeed.x, pPed->m_vecMoveSpeed.y, pPed->m_vecMoveSpeed.z, pPed->m_vecMoveSpeed.Magnitude());
/////////////////////////////////////////
	
	float fMinJumpSpeed = MINJUMP_SPEED;
	if( (pRunAnim = RpAnimBlendClumpGetAssociation((RpClump*)(pPed->m_pRwObject), ANIM_STD_RUNFAST)) )
		fMinJumpSpeed = RUNJUMP_SPEED + pRunAnim->GetBlendAmount()*(SPRINTJUMP_SPEED - RUNJUMP_SPEED);
	else if( (pRunAnim = RpAnimBlendClumpGetAssociation((RpClump*)(pPed->m_pRwObject), ANIM_STD_RUN)) )
		fMinJumpSpeed = MINJUMP_SPEED + pRunAnim->GetBlendAmount()*(RUNJUMP_SPEED - MINJUMP_SPEED);


	if(pPed->IsPlayer()||(pPed->m_pObjectivePed && pPed->m_pObjectivePed->IsPlayer()))// might be hunting/seeking/killing player
		pPed->ApplyMoveForce(CVector(0, 0,PLAYERPED_JUMPFORCE));
	else
		pPed->ApplyMoveForce(CVector(0, 0,PED_JUMPFORCE));

	if(pPed->m_vecMoveSpeed.x * pPed->m_vecMoveSpeed.x + pPed->m_vecMoveSpeed.y * pPed->m_vecMoveSpeed.y < fMinJumpSpeed*fMinJumpSpeed
	|| pPed->m_pGroundPhysical)
	{
		float fJumpHeading = pPed->m_fCurrentHeading;
		if(TheCamera.Cams[0].Using3rdPersonMouseCam())
			fJumpHeading=pPed->WorkOutHeadingForMovingFirstPerson(pPed->m_fCurrentHeading);	
	
		if(pPed->m_pGroundPhysical)
		{
			pPed->m_vecMoveSpeed.x = -fMinJumpSpeed * CMaths::Sin(fJumpHeading) + pPed->m_pGroundPhysical->m_vecMoveSpeed.x;
			pPed->m_vecMoveSpeed.y = fMinJumpSpeed * CMaths::Cos(fJumpHeading) + pPed->m_pGroundPhysical->m_vecMoveSpeed.y;
		}	
		else
		{
			pPed->m_vecMoveSpeed.x = -fMinJumpSpeed * CMaths::Sin(fJumpHeading);
			pPed->m_vecMoveSpeed.y = fMinJumpSpeed * CMaths::Cos(fJumpHeading);
		}
	}
/////////////////////////////////////////
//	printf("Post-Speed: %f,%f,%f Mag:%f\n", pPed->m_vecMoveSpeed.x, pPed->m_vecMoveSpeed.y, pPed->m_vecMoveSpeed.z, pPed->m_vecMoveSpeed.Magnitude());
/////////////////////////////////////////
		
	pPed->SetIsStanding(false);
	pPed->m_nPedFlags.bIsInTheAir = 1;
	pAnim->SetBlendDelta(-1000.0f);
	CAnimManager::AddAnimation((RpClump*)pPed->m_pRwObject, ANIM_STD_PED, ANIM_STD_JUMP_GLIDE);
	
	if(pPed->m_nPedFlags.bDoBloodyFootprints)
	{
		// do footprint for left foot
		CVector vec(0,0,0);

		RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)(pPed->m_pRwObject));
		int32 bone = RpHAnimIDGetIndex(pHierarchy, pPed->m_aPedFrames[PED_FOOTL]->boneTag);
		// get the node matrix for this node/bone
		RwMatrix *pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + bone);
		// transform node into ped frame
		RwV3dTransformPoints(&vec, &vec, 1, pMatrix);
		// then transform into world frame
		//vec = pPed->m_mat * vec;

		const CVector vecFwd(pPed->m_mat.GetForward());
		const CVector vecRgt(pPed->m_mat.GetRight());
		vec.z -= 0.1f;//0.15f;//0.1f
		vec += 0.20f*vecFwd;//0.15f*vecFwd;
		// bloody footprints:			
		CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpBloodPoolTex, &vec, 0.26f*vecFwd.x, 0.26f*vecFwd.y,
					0.14f*vecRgt.x, 0.14f*vecRgt.y, 255, 255, 0, 0, PED_SHADOW_DEPTH, 3000);

		// do footprint for right foot
		vec = CVector(0,0,0);
		
		pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)(pPed->m_pRwObject));
		bone = RpHAnimIDGetIndex(pHierarchy, pPed->m_aPedFrames[PED_FOOTR]->boneTag);
		// get the node matrix for this node/bone
		pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + bone);
		// transform node into ped frame
		RwV3dTransformPoints(&vec, &vec, 1, pMatrix);
		// then transform into world frame
		//vec = pPed->m_mat * vec;

		vec.z -= 0.1f;//0.15f;//0.1f
		vec += 0.20f*vecFwd;//0.15f*vecFwd;
		// bloody footprints:			
		CShadows::AddPermanentShadow(SHAD_TYPE_DARK, gpBloodPoolTex, &vec, 0.26f*vecFwd.x, 0.26f*vecFwd.y,
					0.14f*vecRgt.x, 0.14f*vecRgt.y, 255, 255, 0, 0, PED_SHADOW_DEPTH, 3000);

		// decrement bloody footprint counters (twice as fast as for walking)
		if( pPed->m_nTimeOfDeath > 40)
			pPed->m_nTimeOfDeath -= 40;
		else{
			pPed->m_nTimeOfDeath = 0;
			pPed->m_nPedFlags.bDoBloodyFootprints = false;
		}
	}
}
*/

/*
// called once  anim has finished
void CPed::FinishJumpCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPlayerPed*)pData;

	pPed->m_nPedFlags.bResetWalkAnims = true;
	pPed->m_nPedFlags.bIsLanding = 0;
	pAnim->SetBlendDelta(-1000.0f);
}
*/

/*
void CPed::FinishHitHeadCB(CAnimBlendAssociation* pAnim, void* pData)
{
	CPed* pPed = (CPlayerPed*)pData;
	// blend out anim
	if(pAnim){
		pAnim->SetBlendDelta(-4.0f);
		pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
	}
	// restore previous state
	if(pPed->m_nPedState == PED_JUMP)
		pPed->RestorePreviousState();
		
	pPed->m_nPedFlags.bIsLanding = false;
}
*/

/*
bool CPed::CanPedDriveOff()
{
	if((m_nPedState != PED_DRIVING)||(m_nLookTimer > CTimer::GetTimeInMilliseconds()))
		return false;

	for (int16 i = 0; i < m_NumClosePeds; i++)
	{
		if(m_apClosePeds[i]->GetPedType() == GetPedType() &&
		  m_apClosePeds[i]->m_Objective == ENTER_CAR_AS_PASSENGER && 
		  m_apClosePeds[i]->m_pObjectiveVehicle == m_pObjectiveVehicle)
		{
			// found someone getting into my car
			m_nLookTimer = (CTimer::GetTimeInMilliseconds() + 1000);//Set look timer anyway
			return false;
		}
	}
		
	return true;
}
*/

	// PED RADIO STUFF
void CPed::SetRadioStation(void)
{
	CPedModelInfo* pModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(GetModelIndex());


	if(!IsPlayer())		// Don't bother with player
	{
		if(m_pMyVehicle)
		{
			if(m_pMyVehicle->pDriver == this)
			{
/*
				// MN - commented out as the ped defines have all changed
	
				if((GetModelIndex() == MODELID_GANG_PGa) || (GetModelIndex() == MODELID_GANG_PGb))
				{
					m_pMyVehicle->m_nRadioStation = DMAudio.GetFavouriteRadioStation();
				}
				else
*/				{
					// check if the radio is already on their fav stations:
//					if(m_pMyVehicle->m_nRadioStation == pModelInfo->GetFirstRadioStation() ||
//						m_pMyVehicle->m_nRadioStation == pModelInfo->GetSecondRadioStation())
//						return;
					
					// Radio wasn't tuned to something this ped liked, set to a station it likes
					if(CGeneral::GetRandomTrueFalse())
						m_pMyVehicle->m_VehicleAudioEntity.m_VehicleAudioSetting.RadioStation = pModelInfo->GetFirstRadioStation();
//						m_pMyVehicle->m_nRadioStation = pModelInfo->GetFirstRadioStation();
					else
						m_pMyVehicle->m_VehicleAudioEntity.m_VehicleAudioSetting.RadioStation = pModelInfo->GetSecondRadioStation();
//						m_pMyVehicle->m_nRadioStation = pModelInfo->GetSecondRadioStation();
				}

			}
		}
	}
}

/*
void CPed::WarpPedIntoCar(CVehicle *pVehicle)
{
	m_nPedFlags.bInVehicle = TRUE;
	m_pMyVehicle = pVehicle;
	REGREF(m_pMyVehicle, (CEntity **) &(m_pMyVehicle));
	m_pObjectiveVehicle = pVehicle;
	REGREF(m_pObjectiveVehicle, (CEntity **) &(m_pObjectiveVehicle));
	SetPedState(PED_DRIVING);
	SetUsesCollision(FALSE);
	m_nPedFlags.bIsInTheAir = false;
	m_nPedFlags.bWasWarpedIntoCar = true;
				
	if(m_Objective == ENTER_CAR_AS_DRIVER)
	{
		pVehicle->SetDriver(this);
		REGREF(pVehicle->pDriver, (CEntity **) &(pVehicle->pDriver));
	}
	else
	if(m_Objective == ENTER_CAR_AS_PASSENGER)
	{
		if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
		{
			if(!pVehicle->pPassengers[0])
			{
				pVehicle->pPassengers[0] = this;
				REGREF(pVehicle->pPassengers[0], (CEntity **) &(pVehicle->pPassengers[0]));
			}
		}
		if(!pVehicle->pPassengers[0])
		{
			pVehicle->pPassengers[0] = this;
			REGREF(pVehicle->pPassengers[0], (CEntity **) &(pVehicle->pPassengers[0]));
		}
		else
		if(!pVehicle->pPassengers[1])
		{
			pVehicle->pPassengers[1] = this;
			REGREF(pVehicle->pPassengers[1], (CEntity **) &(pVehicle->pPassengers[1]));
		}
		else
		if(!pVehicle->pPassengers[2])
		{
			pVehicle->pPassengers[2] = this;
			REGREF(pVehicle->pPassengers[2], (CEntity **) &(pVehicle->pPassengers[2]));
		}
		else
		if(!pVehicle->pPassengers[3])
		{
			pVehicle->pPassengers[3] = this;
			REGREF(pVehicle->pPassengers[3], (CEntity **) &(pVehicle->pPassengers[3]));
		}
	}
	else
		return;
	
	if(IsPlayer())
	{
		pVehicle->SetStatus(STATUS_PLAYER);
		AudioManager.PlayerJustGotInCar();
		CCarCtrl::RegisterVehicleOfInterest(pVehicle);
	}
	else
		pVehicle->SetStatus(STATUS_PHYSICS);
	
	CWorld::Remove(this);
	CVector TempCoors = pVehicle->GetPosition();
	SetPosition(TempCoors);
	CWorld::Add(this);

	if (pVehicle->IsAmbulanceOnDuty)
	{
		pVehicle->IsAmbulanceOnDuty = false;
		CCarCtrl::NumAmbulancesOnDuty--;
	}
	if (pVehicle->IsFireTruckOnDuty)
	{
		pVehicle->IsFireTruckOnDuty = false;
		CCarCtrl::NumFireTrucksOnDuty--;
	}

	// If engine is off, switch it on (playing starter motor too)
	if(!pVehicle->EngineOn)
	{
		pVehicle->EngineOn = TRUE;
		DMAudio.PlayOneShot(pVehicle->AudioHandle, AE_CAR_ENGINE_STARTER_MOTOR, 1.0f);
	}

	// make sure we get rid of any partial anims (like fall anims for example)
	RpAnimBlendClumpSetBlendDeltas((RpClump*)m_pRwObject, ABA_FLAG_ISPARTIAL, -1000.0f);

	AddInCarAnims(pVehicle, pVehicle->pDriver == this);

	RemoveWeaponWhenEnteringVehicle();
		
	if(pVehicle->m_bIsBus)
	{
		// dont render peds in buses
		m_nPedFlags.bRenderPedInCar = false;
	}	

	m_nPedFlags.bUpdateAnimPosition = true;

}
*/

/*
bool CPed::HasAttractor() const
{
    return (!(0==m_pMyAttractor));
}

void CPed::SetNewAttraction(CPedAttractor* pAttractor, const CVector& vNewAttractPos, const float fNewAttractHeading, const float fTime, const int iSlot)
{
    if(0==m_pMyAttractor)
    {
        m_pMyAttractor=pAttractor;
    }
    
    if(pAttractor==m_pMyAttractor)
    {
        switch(m_pMyAttractor->GetEffect()->attr.q.m_type)
        {
            m_iAttractorSlot=iSlot;
            
#ifndef FINALBUILD
            char s[255];
            sprintf(s,"%d \n", m_iAttractorSlot);
            CDebug::DebugLog(s);
#endif
            
            case QT_SEAT:
                SetObjective(GOTO_SEAT_ON_FOOT,fNewAttractHeading,vNewAttractPos);
                SetObjectiveTimer(fTime);
                m_iAttractorSlot=iSlot;
                break;
            case QT_ATM:
                SetObjective(GOTO_ATM_ON_FOOT,fNewAttractHeading,vNewAttractPos);
                SetObjectiveTimer(fTime);
                m_iAttractorSlot=iSlot;
                break;
            case QT_BUS_STOP:
                SetObjective(GOTO_BUS_STOP_ON_FOOT,fNewAttractHeading,vNewAttractPos);
                SetObjectiveTimer(fTime);
                m_iAttractorSlot=iSlot;
                break;
            case QT_PIZZA:
                SetObjective(GOTO_PIZZA_ON_FOOT,fNewAttractHeading,vNewAttractPos);
                SetObjectiveTimer(fTime);
                m_iAttractorSlot=iSlot;
                break;
            case QT_SHELTER:
                SetObjective(GOTO_SHELTER_ON_FOOT,fNewAttractHeading,vNewAttractPos);
                SetObjectiveTimer(fTime);
                m_iAttractorSlot=iSlot;
                break;
            case QT_ICE_CREAM_VAN:
                SetObjective(GOTO_ICE_CREAM_VAN_ON_FOOT,fNewAttractHeading,vNewAttractPos);
                SetObjectiveTimer(fTime);
                m_iAttractorSlot=iSlot;
                break;
            default:
                break;
        }
        
        //ASSERT(m_nObjectiveTimer>CTimer::GetTimeInMilliseconds());
        //ASSERT(m_nObjectiveTimer);
    }
}
*/



CEntity *CPed::AttachPedToEntity(CEntity *pEnt, CVector vecPos, uint16 nHeading, float fHeadingLimit, eWeaponType nWeaponType)
{
	if(!pEnt)
		return NULL;
	
	if(m_nPedFlags.bInVehicle)
		return NULL;
		
	m_pAttachToEntity = pEnt;
	REGREF(m_pAttachToEntity, &m_pAttachToEntity);
	
	m_vecAttachOffset = vecPos;
	m_nAttachLookDirn = nHeading;
	m_fAttachHeadingLimit = fHeadingLimit;
	
	if(IsPlayer())
		m_nFlags.bUsesCollision = false;
	else if(pEnt->GetIsTypeVehicle())
		m_pNOCollisionVehicle = pEnt;
//	else
//		m_nFlags.bUsesCollision = false;
	
	// make sure there's no objectives left over on the player when they're attached (eg enter_car)
	//if(IsPlayer())
	//{
	//	m_Objective = NO_OBJ;
	//	m_StoredObjective = NO_OBJ;
	//}	
	
	//SetStoredState();
	//SetPedState(PED_IDLE);
	

	if(m_eStoredWeapon == WEAPONTYPE_UNIDENTIFIED)
	{
		m_eStoredWeapon = GetWeapon()->GetWeaponType();
		m_nOriginalWeaponAmmo = GetWeapon()->GetWeaponAmmoTotal();
	}

	
	if(IsPlayer())
	{
		if(nWeaponType != WEAPONTYPE_UNARMED)
			GiveWeapon(nWeaponType, 30000);
		GetPlayerData()->m_nChosenWeapon = nWeaponType;
		((CPlayerPed *)this)->MakeChangesForNewWeapon(nWeaponType);
		if(nWeaponType==WEAPONTYPE_CAMERA)
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_CAMERA, 0, 0);
		else if(pEnt->GetModelIndex()==MODELID_WEAPON_POOLCUE && !CWeaponInfo::GetWeaponInfo(nWeaponType)->IsWeaponFlagSet(WEAPONTYPE_FIRSTPERSON))
		{
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_AIMWEAPON_ATTACHED, 0, 0);
			GetPlayerData()->m_bFreeAiming = true;
		}
		else
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_HELICANNON_1STPERSON, 0, 0);
			
		SetPedState(PED_SNIPER_MODE);
	}
	else
	{
		GiveWeapon(nWeaponType, 30000);
		SetCurrentWeapon(nWeaponType);
	}
	
	// before we return make sure to position the ped, otherwise might do
	// a process collision loop before they're positioned, clearing m_pNOCollisionVehicle
	PositionAttachedPed();
	return pEnt;
}

CEntity *CPed::AttachPedToBike(CEntity *pEnt, CVector vecPos, uint16 nHeading, float fHeadingLimit, float fVerticalLimit, eWeaponType nWeaponType)
{
	if(NULL==AttachPedToEntity(pEnt, vecPos, nHeading, fHeadingLimit, nWeaponType))
		return NULL;

	m_fAttachVerticalLimit = fVerticalLimit;
	return pEnt;
}

void CPed::DettachPedFromEntity()
{
	CEntity *pTempEnt = m_pAttachToEntity;
	m_pAttachToEntity = NULL;

	if(GetPedState()==PED_DIE)
	{
		// not sure we want to chuck dying peds off the back of boats?
//		if(pTempEnt->GetIsTypeVehicle() && ((CVehicle *)pTempEnt)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
//			return;
			
		m_pNOCollisionVehicle = pTempEnt;
		ApplyMoveForce( -4.0f*pTempEnt->GetMatrix().GetForward() + 4.0f*CVector(0.0f,0.0f,1.0f) );
		SetIsStanding(false);
	}
	else if( GetPedState()!=PED_DEAD)
	{
		//RestorePreviousState();
		CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 1000.0f);

		m_nFlags.bUsesCollision = true;
		
		if(m_eStoredWeapon != WEAPONTYPE_UNIDENTIFIED)
		{
			GetWeapon()->SetWeaponAmmoInClip(0);
			GetWeapon()->SetWeaponAmmoTotal(0);

			SetCurrentWeapon(m_eStoredWeapon);
			GetWeapon()->SetWeaponAmmoTotal(m_nOriginalWeaponAmmo);
			
			m_eStoredWeapon = WEAPONTYPE_UNIDENTIFIED;
		}
		
		if (IsPlayer())
		{
//			TheCamera.ClearPlayerWeaponMode();
			((CPlayerPed *)this)->ClearWeaponTarget();
		}
	}
}


void CPed::PositionAttachedPed()
{
	if(m_pAttachToEntity == NULL)
		return;

	CMatrix newMatrix, Matrix2;
	CVector vecAttachOffset;
	
	if(m_pAttachToEntity->GetModelIndex()==MODELID_CAR_FIRETRUCK_LA
	&& ((CAutomobile *)m_pAttachToEntity)->m_aCarNodes[CAR_MISC_B]
	&& m_vecAttachOffset.z < -900.0f)
	{
		newMatrix.Attach(RwFrameGetLTM(((CAutomobile *)m_pAttachToEntity)->m_aCarNodes[CAR_MISC_B]), false);
		newMatrix.Detach();
		vecAttachOffset = m_vecAttachOffset;
		vecAttachOffset.z += 1000.0f;
	}
	else
	{
		newMatrix = m_pAttachToEntity->GetMatrix();
		vecAttachOffset = m_vecAttachOffset;
	}
	
	CVector vecFinalPosition = Multiply3x3(newMatrix, vecAttachOffset) + newMatrix.GetTranslate();
	newMatrix.tx = vecFinalPosition.x;
	newMatrix.ty = vecFinalPosition.y;
	newMatrix.tz = vecFinalPosition.z;
	
	float fEntHeading = CMaths::ATan2(-newMatrix.xy, newMatrix.yy);//m_pAttachToEntity->GetHeading();
	if(!IsPlayer())
	{
		float fAttachedHeading = fEntHeading;
		switch(m_nAttachLookDirn)
		{
			case 0:	// forward (remember heading taken from x vector?)
				break;
			case 1:	// left
				fAttachedHeading += HALF_PI;
				break;
			case 2:	// back
				fAttachedHeading += PI;
				break;
			case 3: // right
				fAttachedHeading -= HALF_PI;
				break;
		}
		
		fAttachedHeading = CGeneral::LimitRadianAngle(fAttachedHeading);
		m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
		
		float fHeadingDiff = m_fCurrentHeading - fAttachedHeading;
		
		if(fHeadingDiff > PI)		fHeadingDiff -= TWO_PI;
		else if(fHeadingDiff < -PI)	fHeadingDiff += TWO_PI;
		
		if(fHeadingDiff > m_fAttachHeadingLimit)
			m_fCurrentHeading = fAttachedHeading + m_fAttachHeadingLimit;
		else if(fHeadingDiff < -m_fAttachHeadingLimit)
			m_fCurrentHeading = fAttachedHeading - m_fAttachHeadingLimit;
		
		m_fCurrentHeading = CGeneral::LimitRadianAngle(m_fCurrentHeading);
	}	

	Matrix2.SetRotateZ(m_fCurrentHeading - fEntHeading);
	newMatrix *= Matrix2;

	SetMatrix(newMatrix);
	
	if(m_pAttachToEntity->GetIsTypeVehicle() || m_pAttachToEntity->GetIsTypeObject())
	{
		m_vecMoveSpeed = ((CPhysical *)m_pAttachToEntity)->m_vecMoveSpeed;
		m_vecTurnSpeed = ((CPhysical *)m_pAttachToEntity)->m_vecTurnSpeed;
	}
	
	m_pGroundPhysical = NULL;
	SetIsStanding(true);
}



/*
void CPed::SunBathe()
{
	// just need to wait here, playing anim, then flee if there's a car about

}
*/


/*
void CPed::Flash()
{
	// wander about, look for women/other peds, then flash occasionally
			
	// do anim, if not already playing this anim
	{			
		CAnimBlendAssociation* pAnim = NULL;
//		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FLASH);
		if(!pAnim)
		{
//			pAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_FLASH);
			Say(AE_PED_FLASH);
		}

		if(m_nChatTimer && CTimer::GetTimeInMilliseconds() > m_nChatTimer)
		{
			m_nChatTimer = (CTimer::GetTimeInMilliseconds() + 30000); // used to prevent ped from flashing again for a while
		}
		
			
	}

}*/


void CPed::Undress(char *ModelFilename)
{
	int32 modelId;
	
	modelId = GetModelIndex();
	
	/*
	// special cludge code for mobile phones. If found hangup animation then remove phone
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_PHONE_OUT);
	if(pAnim)
		FinishTalkingOnMobileCB(pAnim, this);
	*/
	
	DeleteRwObject();
	if(IsPlayer())
		modelId = MODELID_PLAYER_PED;
	CStreaming::RequestSpecialModel(modelId, ModelFilename, STRFLAG_MISSION_REQUIRED|STRFLAG_FORCE_LOAD);
	
	CWorld::Remove(this);
}

void CPed::Dress(void)
{
	int32 modelId;
	
	modelId = m_nModelIndex;
	m_nModelIndex = -1;
	SetModelIndex(modelId);
	
	if(m_nPedState != PED_DRIVING)
	{
		m_nPedState = PED_IDLE;
	}
	//m_nPedStoredState = PED_NONE;
	//m_Objective = NO_OBJ;
	//m_StoredObjective = NO_OBJ;
	//m_eWaitState=PEDWAIT_FALSE;
	
	CWorld::Add(this);

//	if (this == FindPlayerPed())
//	{		
//		GetPlayerWanted()->m_nWantedLevelBeforeParole = FindPlayerPed()->m_Wanted->m_nWantedLevel;
//		GetPlayerWanted()->m_WantedLevelBeforeParole = FindPlayerPed()->m_Wanted->m_WantedLevel;
//		GetPlayerWanted()->m_TimeOfParole = CTimer::GetTimeInMilliseconds();
//		GetPlayerWanted()->m_nWantedLevel = 0;	
//		GetPlayerWanted()->m_WantedLevel = WANTED_CLEAN;	
//	}
	RestoreHeadingRate();
}

/*
void CPed::Say(tEvent Phrase, int iDelay)
{
    if(m_iDelayedPhrase!=-1)
    {
        return;
    }
    m_iDelayedPhrase=Phrase;
    m_iDelayedPhraseTimer=CTimer::GetTimeInMilliseconds()+iDelay;   
}
*/

bool CPed::IsAlive() const
{
	return (PED_DIE!=m_nPedState && PED_DEAD!=m_nPedState /*&& (m_nHealth>0)*/);
	
	/*
    CTask* pTaskActive=m_pPedIntelligence->GetTaskActive();
    if(pTaskActive)
    {
        if(pTaskActive->GetTaskType()==CTaskTypes::TASK_COMPLEX_LEAVE_CAR_AND_DIE)
        {
            return false;
        }
        
        CTask* pTaskActiveSimplest=m_pPedIntelligence->GetTaskActiveSimplest();
        ASSERT(pTaskActiveSimplest);
        const int iTaskType=pTaskActiveSimplest->GetTaskType();
        if((CTaskTypes::TASK_SIMPLE_DIE==iTaskType)||(CTaskTypes::TASK_SIMPLE_DEAD==iTaskType))
        {
            return false;
        }
    }
    
    return true;        
    */  
}


void CPed::ApplyDamageResponse(const CPedDamageResponse& d)
{
//	m_nHealth-=d.m_fDamageToApply;
//	m_nArmour-=d.m_fArmourToApply;
}

void CPed::UpdateStatEnteringVehicle()
{
//	CPopulation::UpdatePedCount(this, SUBTRACT_FROM_POPULATION);

//	if (!m_nPedFlags.bCountsAsVehiclePassenger)
//	{
//		m_nPedFlags.bCountsAsVehiclePassenger = true;
//		CPopulation::ms_nTotalCarPassengerPeds++;
//	}
}

void CPed::UpdateStatLeavingVehicle()
{
//	CPopulation::UpdatePedCount(this, ADD_TO_POPULATION);

//	if (m_nPedFlags.bCountsAsVehiclePassenger)
//	{
//		m_nPedFlags.bCountsAsVehiclePassenger = false;
//		CPopulation::ms_nTotalCarPassengerPeds--;
//	}
}

CVector aStdBonePosisions[BONETAG_L_BREAST+1] = 
{
	CVector(0.0f,0.0f,0.0f),	// BONETAG_ROOT
	CVector(0.0f,0.0f,0.0f),	// BONETAG_PELVIS
	CVector(0.0f,0.0f,0.0f),	// BONETAG_SPINE
	CVector(0.0f,0.0f,0.4f),	// BONETAG_SPINE1
	CVector(0.0f,0.0f,0.6f),	// BONETAG_NECK	
	CVector(0.0f,0.0f,0.7f),	// BONETAG_HEAD
	CVector(0.0f,0.0f,0.8f),	// BONETAG_L_BROW
	CVector(0.0f,0.0f,0.8f),	// BONETAG_R_BROW
	CVector(0.0f,0.0f,0.7f),	// BONETAG_JAW
	CVector(0.1f,0.0f,0.6f),	// BONETAG_R_CLAVICLE
	CVector(0.2f,0.0f,0.6f),	// BONETAG_R_UPPERARM
	CVector(0.2f,0.0f,0.4f),	// BONETAG_R_FOREARM
	CVector(0.2f,0.0f,0.0f),	// BONETAG_R_HAND
	CVector(0.2f,0.0f,0.0f),	// BONETAG_R_FINGERS
	CVector(0.2f,0.0f,0.0f),	// BONETAG_R_FINGER01
	CVector(-0.1f,0.0f,0.6f),	// BONETAG_L_CLAVICLE
	CVector(-0.2f,0.0f,0.6f),	// BONETAG_L_UPPERARM
	CVector(-0.2f,0.0f,0.4f),	// BONETAG_L_FOREARM
	CVector(-0.2f,0.0f,0.0f),	// BONETAG_L_HAND
	CVector(-0.2f,0.0f,0.0f),	// BONETAG_L_FINGERS
	CVector(-0.2f,0.0f,0.0f),	// BONETAG_L_FINGER01
	CVector(-0.1f,0.0f,-0.3f),	// BONETAG_L_THIGH
	CVector(-0.1f,0.0f,-0.5f),	// BONETAG_L_CALF
	CVector(-0.1f,0.0f,-0.9f),	// BONETAG_L_FOOT
	CVector(-0.1f,0.0f,-0.9f),	// BONETAG_L_TOE
	CVector(0.1f,0.0f,-0.3f),	// BONETAG_R_THIGH
	CVector(0.1f,0.0f,-0.5f),	// BONETAG_R_CALF
	CVector(0.1f,0.0f,-0.9f),	// BONETAG_R_FOOT
	CVector(0.1f,0.0f,-0.9f),	// BONETAG_R_TOE
	CVector(0.0f,0.0f,0.2f),	// BONETAG_BELLY
	CVector(0.1f,0.0f,0.5f),	// BONETAG_R_BREAST
	CVector(-0.1f,0.0f,0.5f)	// BONETAG_L_BREAST
};

void CPed::GetBonePosition(RwV3d& posn, uint32 boneTag, bool bCalledFromCamera)
{
	if(bCalledFromCamera)
	{
		if(!m_nPedFlags.bCalledPreRender)
		{
			UpdateRpHAnim();
			m_nPedFlags.bCalledPreRender = true;
		}
	}
	else if(!m_nPedFlags.bCalledPreRender)
	{
		posn = GetMatrix()*aStdBonePosisions[boneTag];
		return;
	}

	RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)(m_pRwObject));
	if(!pHierarchy)
	{
		// this sometimes may be NULL (esp. during cutscenes), so use safer code:
		posn = this->GetPosition();
		return;
	}
	
	int32 bone = RpHAnimIDGetIndex(pHierarchy, boneTag);//m_pPed->m_aPedFrames[compId]->boneTag);
	// get the node matrix for this node/bone
	RwMatrix *pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + bone);
	// JUST GET posn FROM MATRIX DIRECTLY (much quicker)
	posn = pMatrix->pos;
}

void CPed::GetTransformedBonePosition(RwV3d& posn, uint32 boneTag, bool bCalledFromCamera)
{
	if(bCalledFromCamera)
	{
		if(!m_nPedFlags.bCalledPreRender)
		{
			UpdateRpHAnim();
			m_nPedFlags.bCalledPreRender = true;
		}
	}
	else if(!m_nPedFlags.bCalledPreRender)
	{
		posn = GetMatrix()*aStdBonePosisions[boneTag];
		return;
	}

	RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)(m_pRwObject));
	int32 bone = RpHAnimIDGetIndex(pHierarchy, boneTag);//m_pPed->m_aPedFrames[compId]->boneTag);
	// get the node matrix for this node/bone
	RwMatrix *pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + bone);
	// transform vector into ped frame
	RwV3dTransformPoints(&posn, &posn, 1, pMatrix);
}

////////////////////////////////////////////////////
// FUNCTION: ReleaseCoverPoint
// DOES:     If this ped has a cover point 'reserved' we clear it here
////////////////////////////////////////////////////

void CPed::ReleaseCoverPoint()
{
	if (m_pCoverPoint)
	{
		m_pCoverPoint->ReleaseCoverPointForPed(this);
	
		m_pCoverPoint = NULL;
	}
}


////////////////////////////////////////////////////
// FUNCTION: GetHoldingTask
// DOES:    
////////////////////////////////////////////////////

CTaskSimpleHoldEntity* CPed::GetHoldingTask()
{
	CTaskSimpleHoldEntity* pTask;

	pTask = (CTaskSimpleHoldEntity*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_HOLD_ENTITY);
	if (pTask == NULL)
	{
		pTask = (CTaskSimpleHoldEntity*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_PICKUP_ENTITY);
		if (pTask == NULL)
		{
			pTask = (CTaskSimpleHoldEntity*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_PUTDOWN_ENTITY);
		}
	}
	
	return pTask;
}


/////////////////////////////////////////////////////
// FUNCTION :	GetEntityThatThisPedIsHolding
// DOES :		Returns a pointer to the entity that the ped is carrying
//				To be carrying something, the ped must be performing one of the following tasks
//				CTaskSimpleHoldEntity, CTaskSimplePickUpEntity, CTaskSimplePutDownEntity
//				or CTaskComplexGoPickUpEntity
/////////////////////////////////////////////////////
CEntity *CPed::GetEntityThatThisPedIsHolding(void)
{
	CTask *pTask = GetHoldingTask();
	CEntity *pEntityBeingHeld = NULL;
	
	if (pTask)
	{
		pEntityBeingHeld = ((CTaskSimpleHoldEntity*) pTask)->GetEntityBeingHeld();
	}
	else
	{
		pTask = GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_GO_PICKUP_ENTITY);
		if (pTask)
		{
			pEntityBeingHeld = ((CTaskComplexGoPickUpEntity*) pTask)->GetEntityBeingHeld();
		}
	}

	return pEntityBeingHeld;
}



/////////////////////////////////////////////////////
// FUNCTION :	DropEntityThatThisPedIsHolding
// DOES :		as it says
/////////////////////////////////////////////////////
void CPed::DropEntityThatThisPedIsHolding(bool8 deleteObj)
{
	CTaskSimpleHoldEntity* pTaskHold = GetHoldingTask();
	CEntity *pEntityBeingHeld = NULL;
	
	if (pTaskHold)
	{
		pEntityBeingHeld = ((CTaskSimpleHoldEntity*)pTaskHold)->GetEntityBeingHeld();
		pTaskHold->DropEntity(this,true);
		
		if (pEntityBeingHeld && deleteObj)
		{
			//Don't delete mission objects.
			if(!pEntityBeingHeld->GetIsTypeObject() || ((CObject*)pEntityBeingHeld)->ObjectCreatedBy!=MISSION_OBJECT)
			{
				pEntityBeingHeld->DeleteRwObject();
				CWorld::Remove(pEntityBeingHeld);
				delete pEntityBeingHeld;
				pEntityBeingHeld = NULL;
			}
		}
	}
}


/////////////////////////////////////////////////////
// FUNCTION :	CanThrowEntityThatThisPedIsHolding
// DOES :		as it says
/////////////////////////////////////////////////////
bool8 CPed::CanThrowEntityThatThisPedIsHolding()
{
	CTaskSimpleHoldEntity* pTask = GetHoldingTask();
	
	if (pTask)
	{
		return pTask->CanThrowEntity();
	}
	
	return false;
}


/////////////////////////////////////////////////////
// FUNCTION :	GiveObjectToPedToHold
// DOES :		as it says
/////////////////////////////////////////////////////
CEntity* CPed::GiveObjectToPedToHold(int32 modelIndex, bool8 dropCurrentObj)
{
	CTaskSimpleHoldEntity* pTask = (CTaskSimpleHoldEntity*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_HOLD_ENTITY);
	
	bool8 giveObject = false;
	if (GetEntityThatThisPedIsHolding())
	{	
		if (dropCurrentObj)
		{
			DropEntityThatThisPedIsHolding(true);
			giveObject = true;
		}
	}

	if (pTask==NULL || giveObject)
	{
		CEntity* pEntity = new CObject(modelIndex, false);
		pEntity->SetPosition(this->GetPosition());
		CWorld::Add(pEntity);
		
		CVector offset(0.0f, 0.0f, 0.0f);
		GetPedIntelligence()->AddTaskSecondary(new CTaskSimpleHoldEntity(pEntity, &offset, PED_HANDR, HOLD_ORIENTATE_BONE_FULL, ANIM_STD_NUM, ANIM_STD_PED, true), CTaskManager::TASK_SECONDARY_PARTIAL_ANIM);	
//		GetPedIntelligence()->AddInterestingEntity(pEntity);

		return pEntity;
	}
	
	return NULL;
}

/////////////////////////////////////////////////////
// FUNCTION :	GiveWeaponAtStartOfFight
// DOES :		Some peds (prostitutes, gang members) will pull a weapon
//				when getting involved in a fight.
/////////////////////////////////////////////////////

void CPed::GiveWeaponAtStartOfFight()
{
		// Don't do this for mission peds
	if (GetCharCreatedBy() == MISSION_CHAR) return;

		// Don't do this if ped already has weapon
	if (GetWeapon()->GetWeaponType() != WEAPONTYPE_UNARMED) return;
	
		// Only do this for certain types
	switch (GetPedType())
	{
		case PEDTYPE_PROSTITUTE:
		case PEDTYPE_DEALER:
		case PEDTYPE_CRIMINAL:
			if ((RandomSeed & 1023) < 200)
			{
				if (m_eDelayedWeapon==WEAPONTYPE_UNIDENTIFIED)
				{
					GiveDelayedWeapon(WEAPONTYPE_KNIFE, 50);
					SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_KNIFE)->m_nWeaponSlot);
				}
			}
			if ((RandomSeed & 1023) < 400)
			{
				if (m_eDelayedWeapon==WEAPONTYPE_UNIDENTIFIED)
				{
					GiveDelayedWeapon(WEAPONTYPE_PISTOL, 50);
					SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_PISTOL)->m_nWeaponSlot);
				}
			}
			break;
			
		case PEDTYPE_GANG1:
		case PEDTYPE_GANG2:
		case PEDTYPE_GANG3:
		case PEDTYPE_GANG4:
		case PEDTYPE_GANG5:
		case PEDTYPE_GANG6:
		case PEDTYPE_GANG7:
		case PEDTYPE_GANG8:
		case PEDTYPE_GANG9:
		case PEDTYPE_GANG10:
			if ((RandomSeed & 1023) < 400)
			{
				if (m_eDelayedWeapon==WEAPONTYPE_UNIDENTIFIED)
				{
					GiveDelayedWeapon(WEAPONTYPE_PISTOL, 50);
					SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_PISTOL)->m_nWeaponSlot);
				}
			}
			break;
	}
}

/////////////////////////////////////////////////////
// FUNCTION :	GiveWeaponWhenJoiningGang
// DOES :		When a ped joins the player gang we give him
//				a pistol or something.
/////////////////////////////////////////////////////

void CPed::GiveWeaponWhenJoiningGang()
{
	// If the ped already has a weapon (gangs now have weapons)
	// we don't bother.
	if (GetWeapon()->GetWeaponType() != WEAPONTYPE_UNARMED) return;
	
	if (m_eDelayedWeapon==WEAPONTYPE_UNIDENTIFIED)
	{
		if (CCheat::IsCheatActive(CCheat::RECRUITME_AK47_CHEAT))
		{
			GiveDelayedWeapon(WEAPONTYPE_AK47, 200);
			SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_AK47)->m_nWeaponSlot);
		}
		else if (CCheat::IsCheatActive(CCheat::RECRUITME_ROCKET_CHEAT))
		{
			GiveDelayedWeapon(WEAPONTYPE_ROCKETLAUNCHER, 200);
			SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_ROCKETLAUNCHER)->m_nWeaponSlot);
		}
		else
		{
			GiveDelayedWeapon(WEAPONTYPE_PISTOL, 200);
			SetCurrentWeapon(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_PISTOL)->m_nWeaponSlot);
		}
	}
}



/////////////////////////////////////////////////////
// FUNCTION :	IsPlayingHandSignal
/////////////////////////////////////////////////////

bool8 CPed::IsPlayingHandSignal()
{
	CTaskComplexPlayHandSignalAnim* pTask = (CTaskComplexPlayHandSignalAnim*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_HANDSIGNAL_ANIM);
	if (pTask)
	{
		return true;
	}
	
	return false;
}


/////////////////////////////////////////////////////
// FUNCTION :	StopPlayingHandSignal
/////////////////////////////////////////////////////

void CPed::StopPlayingHandSignal()
{
	CTaskComplexPlayHandSignalAnim* pTask = (CTaskComplexPlayHandSignalAnim*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_HANDSIGNAL_ANIM);
	if (pTask)
	{
		pTask->MakeAbortable(this, CTask::ABORT_PRIORITY_URGENT, 0);
	}
	
//	pTask = (CTaskComplexPlayHandSignalAnim*)GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_HANDSIGNAL_ANIM);
//	if (pTask)
//	{
//		ASSERT(0);
//	}
}

/////////////////////////////////////////////////////
// FUNCTION :	SetPedState
/////////////////////////////////////////////////////

void CPed::SetPedState(const ePedState pedState)
{
	if (pedState == PED_DEAD || pedState == PED_DIE)
	{
		ReleaseCoverPoint();
		if (m_nPedFlags.bClearRadarBlipOnDeath) CRadar::ClearBlipForEntity(BLIPTYPE_CHAR, CPools::GetPedPool().GetIndex(this));
	}
    m_nPedState=pedState;
}


// MN - should move this as part of the ped class eventually
//      it is exclusively checked out at the moment so i can't

float
CPed::GetWalkAnimSpeed(void)
{
	CAnimBlendStaticAssociation* pAnim = CAnimManager::GetAnimAssociation(m_motionAnimGroup, ANIM_STD_WALK);
	CAnimBlendHierarchy* pAnimBlendHierarchy = pAnim->GetAnimHierarchy();
	CAnimBlendSequence* pAnimBlendSequence = pAnimBlendHierarchy->GetAnimSequence(0);
	CAnimBlendKeyFrame *pKF;
	
	CAnimManager::UncompressAnimation(pAnimBlendHierarchy);
	if(pAnimBlendSequence->GetNumKeyFrames() > 0)
	{
		float dist, time;
		
		// get the last keyframe
		pKF = &pAnimBlendSequence->GetKeyFrame(pAnimBlendSequence->GetNumKeyFrames()-1);
		dist = pKF->m_vecTranslation.y;
		
		// get the first keyframe
		pKF = &pAnimBlendSequence->GetKeyFrame(0);
		dist -= pKF->m_vecTranslation.y;
		
		time = pAnimBlendHierarchy->GetTotalTime();
		
		return dist/time;
	}
	
	ASSERT(0);
	return 0;
}



