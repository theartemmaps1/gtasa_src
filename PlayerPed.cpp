// Title	:	PlayerPed.cpp
// Author	:	Gordon Speirs
// Started	:	10/12/99

#include "PlayerPed.h"
#include "Ped.h"
#include "PedType.h"
#include "Physical.h"
#include "Maths.h"
#include "Main.h"
#include "General.h"
#include "World.h"
#include "Game.h"
#include "Range2D.h"
#include "Camera.h"
#include "Collision.h"
#include "Pad.h"
#include "PathFind.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "WeaponEffects.h"
#include "BulletInfo.h"
#include "Timer.h"
#include "PtrList.h"
#include "Wanted.h"
#include "Crime.h"
#include "Frontend.h"
#include "Debug.h"
#include "Profile.h"
#include "AnimManager.h"
#include "RpAnimBlendClump.h"
#include "RpAnimBlend.h"
#include "modelIndices.h"
#include "darkel.h"
#include "pools.h"
#include "carctrl.h"
#include "messages.h"
#ifdef GTA_NETWORK
#include "gamenet.h"
#endif
#include "network_allprojects.h"
#include "pedplacement.h"
#include "modelindices.h"
#include "2dEffect.h"
#include "streaming.h"
#include "VarConsole.h"
#include "CutsceneMgr.h"
#include "Script.h"
#include "mblur.h"
#include "sprite.h"
#include "replay.h"
#include "Task.h"
#include "PedIntelligence.h"
#include "TaskPlayer.h"
#include "Replay.h"
#include "localisation.h"
#include "TaskSecondary.h"
#include "TaskFlee.h"
#include "TaskManager.h"
#include "events.h"
#include "stats.h"
#include "VehicleModelInfo.h"
#include "gamelogic.h"
#include "clothes.h"
#include "tag.h"
#include "PedGroup.h"
#include "atomicmodelinfo.h"
#include "taskbasic.h"
#include "TaskAttack.h"
#include "PedIntelligence.h"
#include "font.h"
#include "taskgangs.h"
#include "PostEffects.h"
#include "Events.h"
#include "WaterLevel.h"
#include "VisibilityPlugins.h"
#include "WeaponModelInfo.h"
#include "Events.h"
#include "radar.h"
#include "globalspeechcontexts.h"
#include "cheat.h"
#include "EntryExits.h"
#include "PedType.h"

#ifdef USE_AUDIOENGINE
#include "AudioEngine.h"
#include "GameAudioEvents.h"
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

#ifndef FINAL
#define DEBUGPLAYERANIM
//#define DEBUGPLAYERHITCOL
//#define PLAYER_INVINCIBLE
//#define PLAYER_WANTED_2
#endif

#ifndef FINAL // just incase I forget to comment this out!
//	#pragma optimization_level 0
//	#pragma dont_inline on
//	#pragma auto_inline off
#endif

#pragma optimize_for_size on

#define MIN_SPRINT_ENERGY	(-150.0f)
//#define SPRINT_LEVELUP_INTERVAL	(500.0f)
//#define SPRINT_LEVELUP_JUMP (10.0f)
//#define MAX_SPRINT_ENERGY_LIMIT (1000.0f)

#define PLAYER_STAND_STILL_LIMIT (500)
#ifdef FINAL
	#define WEAPON_SPRAY_DELTA  (EIGHTH_PI/16.0f)
	#define WEAPON_SPRAY_SMALL_DELTA  (EIGHTH_PI/22.0f)
	#define WEAPON_SPRAY_LARGE_DELTA  (EIGHTH_PI/14.0f)	// made these small cause gonna multiply
	#define WEAPON_SPRAY_FLAME_DELTA  (EIGHTH_PI/10.0f) // by the timestep to equalise over framerate
	#define WEAPON_SPRAY_CHAINSAWRUN_DELTA	(EIGHTH_PI/16.0f)
#else
	float WEAPON_SPRAY_DELTA  		= (EIGHTH_PI/16.0f);
	float WEAPON_SPRAY_SMALL_DELTA  = (EIGHTH_PI/22.0f);
	float WEAPON_SPRAY_LARGE_DELTA  = (EIGHTH_PI/14.0f);
	float WEAPON_SPRAY_FLAME_DELTA  = (EIGHTH_PI/10.0f);
	float WEAPON_SPRAY_CHAINSAWRUN_DELTA	= (EIGHTH_PI/16.0f);

#endif

#define SOLICIT_FOOT_DISTANCE (3.0f)
#define SOLICIT_VEHICLE_DISTANCE (5.0f)
#define SOLICIT_PAUSE_TIME (10000)

#define ADRENALINE_BOOST_FACTOR (2.0f)
// only let the player be in die state for a max of 4sec (incase something goes wrong)
#define PLAYER_DIE_TIMELIMIT (4000)

TweakFloat CHAINGUN_SPINUP_RATE		= 0.025f;
TweakFloat CHAINGUN_SPINDOWN_RATE	= 0.003f;
TweakFloat CHAINGUN_SPIN_MAXSPEED	= 0.45f;

// don't delete this - we might have to put player ped 1st person anims back in
// for network games at some later stage
//#define PLAYER_1ST_PERSON_USEANIMS

#define DRUNK_FADE_STEP	(1)

static int32 playerIdlesAnimBlock = 0;
#ifndef FINAL
bool CPlayerPed::bDebugPlayerInfo = false;
bool CPlayerPed::bDebugTargetting = false;
#endif
#ifndef MASTER
bool CPlayerPed::bDebugPlayerInvincible = false;
#endif

// Some test stuff so that we can fuck about with the targetting
bool	bUseOldTargetting = false;			// Use the standard (old) targetting that was there before
float	TargetAngleCone			= 90.0f;	// What can the angle be?
float	WiderTargetAngleCone	= 140.0f;	// What can the angle be?
float	ConeShift				= 3.0f;		// At what distance can the target be in the wider cone

bool CPlayerPed::bHasDisplayedPlayerQuitEnterCarHelpText=false;

// Name			:	Constructor
// Purpose		:	Default constructor for CPlayerPed class
// Parameters	:	None
// Returns		:	Nothing

CPlayerPed::CPlayerPed(int32 nPlayerInfoIndex, bool bForReplay)
: CPed(PEDTYPE_PLAYER1)
{
	// need to set up ped class to point to playerped data in player structure
	m_pPlayerData = &CWorld::Players[nPlayerInfoIndex].PlayerPedData;
	// also use this opportunity to create wanted data
	m_pPlayerData->AllocateData();

	SetModelIndex(MODELID_PLAYER_PED);
	SetInitialState(bForReplay);
	
	SetWeaponLockOnTarget(NULL);

	SetPedState(PED_IDLE);
	
	// initialise player idles anim block index
	playerIdlesAnimBlock = CAnimManager::GetAnimationBlockIndex("playidles");
	
	// Create a group with the player being the leader.
	if (!bForReplay)
	{
		GetPlayerData()->m_PlayerGroup = CPedGroups::AddGroup();
		CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupIntelligence()->SetDefaultTaskAllocatorType(CPedGroupDefaultTaskAllocatorTypes::DEFAULT_TASK_ALLOCATOR_RANDOM);	// was DEFAULT_TASK_ALLOCATOR_FOLLOW_LIMITED
		CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].SetGroupCreatedBy(CPedGroup::MISSION_GROUP);
		CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->SetLeader(this);
		CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].Process();
		GetPlayerData()->m_GroupStuffDisabled = false;
		GetPlayerData()->m_GroupAlwaysFollow = false;
		GetPlayerData()->m_GroupNeverFollow = false;
	}

		// Have to update the maxhealth here so that it is correct for the 2nd player in 2-player games.
	m_nMaxHealth = CStats::GetFatAndMuscleModifier(STAT_MODIFIER_MAX_HEALTH);
	m_nHealth = m_nMaxHealth;

	m_nExtraMeleeCombo = MCOMBO_KICK_STD;
	m_nExtraMeleeComboFlags = 0x0f;

#ifdef GTA_PC
	m_pMouseLockOnRecruitPed=0;
	m_iMouseLockOnRecruitTimer=0;
#endif

#ifdef USE_AUDIOENGINE
	// clear out the audio entity and pass a reference to this ped
	m_PedSpeechAudioEntity.Initialise(this);
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

	GetPedIntelligence()->SetInformRespectedFriends(30.0f, 2);

#ifdef GTA_REPLAY
	m_nPedFlags.bUsedForReplay = bForReplay;
#endif
}

/*
// The following constructor is used in the pc replays only.
#ifdef GTA_PC
CPlayerPed::CPlayerPed(int32 nPlayerInfoIndex, CPlayerPedData *pPlayerPedData)
: CPed(PEDTYPE_PLAYER1)
{
	// need to set up ped class to point to playerped data in player structure
	m_pPlayerData = pPlayerPedData;
	// also use this opportunity to create wanted data
	m_pPlayerData->AllocateData();

	SetModelIndex(MODELID_PLAYER_PED);
	SetInitialState(true);
	
	SetWeaponLockOnTarget(NULL);

	SetPedState(PED_IDLE);
	
	// initialise player idles anim block index
	playerIdlesAnimBlock = CAnimManager::GetAnimationBlockIndex("playidles");
	
	// Create a group with the player being the leader.
	GetPlayerData()->m_PlayerGroup = CPedGroups::AddGroup();
	CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupIntelligence()->SetDefaultTaskAllocatorType(CPedGroupDefaultTaskAllocatorTypes::DEFAULT_TASK_ALLOCATOR_RANDOM);	// was DEFAULT_TASK_ALLOCATOR_FOLLOW_LIMITED
	CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].SetGroupCreatedBy(CPedGroup::MISSION_GROUP);
	CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->SetLeader(this);
	CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].Process();
	GetPlayerData()->m_GroupStuffDisabled = false;
	GetPlayerData()->m_GroupAlwaysFollow = false;
	GetPlayerData()->m_GroupNeverFollow = false;

		// Have to update the maxhealth here so that it is correct for the 2nd player in 2-player games.
	m_nMaxHealth = CStats::GetFatAndMuscleModifier(STAT_MODIFIER_MAX_HEALTH);
	m_nHealth = m_nMaxHealth;

	m_nExtraMeleeCombo = MCOMBO_KICK_STD;
	m_nExtraMeleeComboFlags = 0x0f;

#ifdef GTA_PC
	m_pMouseLockOnRecruitPed=0;
	m_iMouseLockOnRecruitTimer=0;
#endif

#ifdef USE_AUDIOENGINE
	// clear out the audio entity and pass a reference to this ped
	m_PedSpeechAudioEntity.Initialise(this);
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

	GetPedIntelligence()->SetInformRespectedFriends(30.0f, 2);	
}
#endif
*/

// Name			:	Destructor
// Purpose		:	Default destructor for CPlayerPed class
// Parameters	:	None
// Returns		:	Nothing

CPlayerPed::~CPlayerPed()
{
//!PC - need to remove any animblock refs that there may be for shutdown.
//#if defined (GTA_PC)
	// de-reference weapon combo if no longer required
	if(GetPlayerData()->m_MeleeWeaponAnimReferenced!=ANIM_STD_PED)
	{
		CAnimBlock *pAnimBlock = CAnimManager::GetAnimBlendAssoc(GetPlayerData()->m_MeleeWeaponAnimReferenced)->GetAnimBlock();
		int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
		CAnimManager::RemoveAnimBlockRef(blockIndex);
		GetPlayerData()->m_MeleeWeaponAnimReferenced = ANIM_STD_PED;
	}
	// de-reference extra melee combo if no longer required
	if(GetPlayerData()->m_MeleeWeaponAnimReferencedExtra!=ANIM_STD_PED)
	{
		CAnimBlock *pAnimBlock = CAnimManager::GetAnimBlendAssoc(GetPlayerData()->m_MeleeWeaponAnimReferencedExtra)->GetAnimBlock();
		int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
		CAnimManager::RemoveAnimBlockRef(blockIndex);
		GetPlayerData()->m_MeleeWeaponAnimReferencedExtra = ANIM_STD_PED;
	}
/*
	// de-reference skateboard anims if no longer needed
	if(GetPlayerData()->m_SkateboardAnimsReferenced==ANIM_SKATEBOARD_PED && (GetWeapon()->GetWeaponType()!=WEAPONTYPE_SKATEBOARD || m_nPedFlags.bInVehicle))
	{
		CAnimBlock *pAnimBlock = CAnimManager::GetAnimBlendAssoc(ANIM_SKATEBOARD_PED)->GetAnimBlock();
		int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
		CAnimManager::RemoveAnimBlockRef(blockIndex);
		GetPlayerData()->m_SkateboardAnimsReferenced = ANIM_STD_PED;
	}
*/
//#endif //GTA_PC
	
	// Remove the group that the player is the leader of.
#ifdef GTA_PC
	if (!m_nPedFlags.bUsedForReplay)
#endif
	{
		CPedGroups::RemoveGroup(GetPlayerData()->m_PlayerGroup);
	}
} // end - CPlayerPed::~CPlayerPed




// Name			:	SetupPlayerPed
// Purpose		:	Sets up the specified player ped
// Parameters	:	nPlayerNum - which player to set up 
// Returns		:	Nothing

void CPlayerPed::SetupPlayerPed(int32 nPlayerNum)
{
	ASSERTMSG(CWorld::Players[nPlayerNum].pPed == NULL, "You have already created this player");
	
	CWorld::Players[nPlayerNum].pPed = new CPlayerPed(nPlayerNum);
	ASSERT(CWorld::Players[nPlayerNum].pPed);
	
	switch(nPlayerNum)
	{
		case 0:
			break;
		case 1:
			CWorld::Players[nPlayerNum].pPed->SetPedType(PEDTYPE_PLAYER2);
			break;
		default:
			ASSERT(0);
			break;
	}
	
	CWorld::Players[nPlayerNum].pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
	CWorld::Add(CWorld::Players[nPlayerNum].pPed);
	CWorld::Players[nPlayerNum].pPed->SetShootingAccuracy(100); 
	CWorld::Players[nPlayerNum].PlayerState = CPlayerInfo::PLAYERSTATE_PLAYING;

#ifndef MASTER
	VarConsole.Add("Use old (not test) targetting",&bUseOldTargetting, true, VC_TARGETTING);
	VarConsole.Add("Cone angle (degrees full width)",&TargetAngleCone, 1.0f, 1.0f, 180.0f, true, VC_TARGETTING);
	VarConsole.Add("Wider cone angle",&WiderTargetAngleCone, 1.0f, 1.0f, 180.0f, true, VC_TARGETTING);
	VarConsole.Add("Cone shift (meters)",&ConeShift, 0.25f, 1.0f, 180.0f, true, VC_TARGETTING);

	VarConsole.Add("Invincible PlayerPed",&CPlayerPed::bDebugPlayerInvincible, true, VC_PEDS);
#ifndef FINAL
	VarConsole.Add("Debug PlayerPed",&CPlayerPed::bDebugPlayerInfo, true, VC_PEDS);
	VarConsole.Add("Apply Air Resistance In City",&CVehicle::m_fAirResistanceMult, 0.1f, 1.0f, 5.0f, true, VC_VEHICLES);
	VarConsole.Add("Tweak Vehicle Handling",&CVehicle::m_bDisplayHandlingInfo, true, VC_VEHICLES);
	VarConsole.Add("Display Vehicle Performance", &CVehicle::m_bDisplayPerformanceInfo, true, VC_VEHICLES);
	VarConsole.Add("Give Vehicle Hydraulics", &CVehicle::m_bGivePlayerHydraulics, true, VC_VEHICLES);
#endif
#endif
} // end - CPlayerPed::SetupPlayerPed




// Name			:	RemovePlayerPed
// Purpose		:	Removes the specified player ped
// Parameters	:	nPlayerNum - which player to remove
// Returns		:	Nothing

void CPlayerPed::RemovePlayerPed(int32 nPlayerNum)
{
	CPed *pPed = CWorld::Players[nPlayerNum].pPed;

	if (pPed)
	{
		// If this player was driving a car (like in 2player mode) we have to tidy up
		if (pPed->m_pMyVehicle && pPed->m_pMyVehicle->pDriver == pPed)
		{
			pPed->m_pMyVehicle->SetStatus(STATUS_PHYSICS);
			pPed->m_pMyVehicle->SetGasPedal(0.0f);
			pPed->m_pMyVehicle->SetBrakePedal(0.1f);
		}
	
	
		CWorld::Remove(CWorld::Players[nPlayerNum].pPed);
		delete CWorld::Players[nPlayerNum].pPed;
		CWorld::Players[nPlayerNum].pPed = NULL;
	}
} // end - CPlayerPed::RemovePlayerPed




// Name			:	DeactivatePlayerPed
// Purpose		:	Temporarily removes the specified player ped from the world
// Parameters	:	nPlayerNum - which player to remove (0 - 3)
// Returns		:	Nothing

void CPlayerPed::DeactivatePlayerPed(int32 nPlayerNum)
{
	CWorld::Remove(CWorld::Players[nPlayerNum].pPed);
} // end - CPlayerPed::DeactivatePlayerPed




// Name			:	ReactivatePlayerPed
// Purpose		:	Puts the specified player ped back into the world
// Parameters	:	nPlayerNum - which player to add (0 - 3)
// Returns		:	Nothing
void CPlayerPed::ReactivatePlayerPed(int32 nPlayerNum)
{
	CWorld::Add(CWorld::Players[nPlayerNum].pPed);
} // end - CPlayerPed::ReactivatePlayerPed



// Name			:	SetInitialState
// Purpose		:	Processes control for player ped object
// Parameters	:	None
// Returns		:	Nothing
void CPlayerPed::SetInitialState(bool bReplay)
{
//	ClearAll();
//	m_nDrunkenness = 0;
//	m_bFadeDrunkenness = false;
	CMBlur::ClearDrunkBlur();
//	m_nDrugLevel = 0;

//	m_bAdrenaline = false;
//	m_AdrenalineEndTime = 0;
	CTimer::SetTimeScale(1.0f);

	//m_vecTargetOffset = CVector2D(0.0f, 0.0f);
//	TIDYREF(m_pTargetEntity, &m_pTargetEntity);
	//m_pTargetEntity = NULL;
	//m_vecTargetPosition = CVector(0.0f, 0.0f, 0.0f);

	//m_vecFleePosition = CVector2D(0.0f, 0.0f);
	//m_pFleeEntity = NULL;
	//m_nFleeTimer = 0;
	//m_Objective = NO_OBJ;
	//m_StoredObjective = NO_OBJ;
	
	SetUsesCollision(true);
	// script command that sets collision off also sets gravity off
	m_nPhysicalFlags.bDoGravity = true;

	ClearAimFlag();
	ClearLookFlag();

	//m_nPedFlags.bIsContinuingAim = FALSE;
	m_nPedFlags.bRenderPedInCar = true;
		// Extinguish fire if we're on fire (before SetInitialState)
	if (m_pFire) 
	{
		m_pFire->Extinguish();
	}
//	SetIdle();
	SetPedState(PED_IDLE);
	SetMoveState(PEDMOVE_STILL);
	/*SetStoredPedState(PED_NONE);*/
	m_nPedFlags.bIsBeingArrested=false;
	m_nPedFlags.bIsDucking=false;
	m_nPedFlags.bCanExitCar=true;
	m_nPedFlags.bDontRender=false;
	
//////////////////////////
// NEW clear all PedIntelligence stuff
	GetPedIntelligence()->FlushIntelligence();
// need to do this AFTER we've flushed all the tasks, otherwise anims get deleted
// without calling their callbacks and leaving bad pointers in the tasks
	RpAnimBlendClumpRemoveAllAssociations((RpClump*)m_pRwObject);
// Add default player task	
	GetPedIntelligence()->AddTaskDefault(new CTaskSimplePlayerOnFoot());
///////////////////////////

	m_motionAnimGroup = ANIM_PLAYER_PED;
//	m_vecFightMovement = CVector2D(0.0f,0.0f);
//	m_moveBlendRatio = 0.0f;
//	m_nChosenWeapon = 0;
//	m_nCarDangerCounter = 0;
//	m_pDangerCar = NULL;
	m_nPedFlags.bIsPedDieAnimPlaying = false;
	if(GetPlayerData()) GetPlayerData()->m_bAdrenaline = false;		// This HAS to be called before SetRealMoveAnim or SetRealMoveAnim will slow time down whilst loading a game. (m_bAdrenaline has not been cleared at this point)
	SetRealMoveAnim();
//	bCanBeDamaged = true;
	m_pPedStats->m_nTemper = 50;
//	GiveWeapon(WEAPONTYPE_BASEBALLBAT, 0);
//	m_fFPSMoveHeading = 0.0f;
	
//	GiveWeapon(WEAPONTYPE_TEARGAS, 50);
	if(m_pAttachToEntity && !m_nFlags.bUsesCollision)
		m_nFlags.bUsesCollision = true;
	m_pAttachToEntity = NULL;
	m_nOriginalWeaponAmmo = 0;

	// player hasn't had this set up yet - do it now
	// other peds get this set up in the ped constructor
	// player can't as his intelligence is later flushed
 	m_pPedIntelligence->AddTaskSecondaryFacialComplex(new CTaskComplexFacial);
  	
	if (!bReplay)
	{
  		TellGroupToStartFollowingPlayer(true, false, true);
	}

  	if(GetPlayerData())
  		GetPlayerData()->SetInitialState();
}

//
// name:		GetPadFromPlayer
// description:	Returns a pointer to the pad class that is used by this player
CPad* CPlayerPed::GetPadFromPlayer()
{
	if (m_nPedType == PEDTYPE_PLAYER1)
	{
		return CPad::GetPad(0);
	}
	else if (m_nPedType == PEDTYPE_PLAYER2)		// Player on second controller
	{
		return CPad::GetPad(1);
	}
	else
	{
		return NULL;		// Don't take controls into acount if this player is controlled from another machine
	}
}


Bool8 CPlayerPed::CanPlayerStartMission()
{
	if (CGameLogic::GameState != CGameLogic::GAMESTATE_PLAYING || CGameLogic::IsCoopGameGoingOn()) return false;	// Obbe: put in to fix the problem with ending a 2 player game on a mission trigger.

	if (IsPedInControl() == false && GetPedState() != PED_DRIVING)
	{
		return false;
	}
	
	if(GetPedIntelligence()->GetNonTempTaskEventResponse() != NULL ||
		(GetPedIntelligence()->GetTaskPrimary() != NULL && GetPedIntelligence()->GetTaskPrimary()->GetTaskType() != CTaskTypes::TASK_SIMPLE_CAR_DRIVE) ||
		GetPedIntelligence()->GetTaskSecondaryAttack() != NULL ||
		!IsAlive())
	{
		return false;
	}

	if(GetPedIntelligence()->GetEventOfType(CEventTypes::EVENT_SCRIPT_COMMAND))
		return false;
	return true;
}

// Name			:	ProcessControl
// Purpose		:	Processes control for player ped object
// Parameters	:	None
// Returns		:	Nothing

#define ADAMS_THIS_IS_AN_INVALID_LOOK_HEADING (999999.0f)
//
//
void CPlayerPed::ProcessControl()
{
#ifdef DEBUGPLAYERANIM
if(bDebugPlayerInfo)
{
//	this can be useful for debugging nasty collision bugs, please leave
//	printf("\nPlayer%d Pos:%g,%g,%g Heading:%g,%g,%g\nSpd:%g,%g,%g TSpd: %g,%g,%g", 
//	CTimer::m_FrameCounter, GetPosition().x, GetPosition().y, GetPosition().z, GetHeading(), GetCurrentHeading(), m_fDesiredHeading,
//	GetMoveSpeed().x, GetMoveSpeed().y, GetMoveSpeed().z, GetTurnSpeed().x, GetTurnSpeed().y, GetTurnSpeed().z);

	CAnimBlendClumpData *pAnimBlendClump = ANIMBLENDCLUMPFROMCLUMP((RpClump*)m_pRwObject);

	// loop through all the attached associations and pause each
	ListLink* pList = LISTFROMCLASS(pAnimBlendClump)->pNext;
	int32 count = 7;

    CTask* pActiveTask=m_pPedIntelligence->GetTaskActive();
    if(pActiveTask)
    {
        CTask* pSimplestActiveTask=m_pPedIntelligence->GetTaskActiveSimplest();
        ASSERT(pSimplestActiveTask);
    	sprintf(gString, "ActiveTask:%s",pActiveTask->GetName().c_str());
	    CDebug::PrintToScreenCoors(gString, 35,count++);
    	sprintf(gString, "SimpleTask:%s\n",pSimplestActiveTask->GetName().c_str());    
	    CDebug::PrintToScreenCoors(gString, 35,count++);
    }
    else
    {
    	sprintf(gString, "ActiveTask:None");
	    CDebug::PrintToScreenCoors(gString, 35,count++);
    	sprintf(gString, "SimpleTask:None");    
	    CDebug::PrintToScreenCoors(gString, 35,count++);
    }	
//	sprintf(gString, "Velocity: %3.2f,%3.2f,%3.2f  Speed: %5.4f\n", m_vecMoveSpeed.x, m_vecMoveSpeed.y, m_vecMoveSpeed.z, m_vecMoveSpeed.Magnitude());
//	sprintf(gString, "Sprint Energy: %f", m_fSprintEnergy);
	sprintf(gString, "SurfaceType: %d", m_LastMaterialToHaveBeenStandingOn);
	CDebug::PrintToScreenCoors(gString, 35,count++);

	while(pList)
	{
		CAnimBlendAssociation* pAnim = CLASSFROMLIST(CAnimBlendAssociation, pList);
		sprintf(gString, "%s: %f,%f,%f\n", pAnim->GetAnimName(), pAnim->GetCurrentTime(), pAnim->GetBlendAmount(), pAnim->GetSpeed());
		CDebug::PrintToScreenCoors(gString, 35,count++);
//		CDebug::DebugLog(gString);
		pList = pList->pNext;
	}
	for(int i=0; i<m_nNoOfCollisionRecords; i++)
	{
		CBaseModelInfo *pModelInfo = CModelInfo::GetModelInfo(m_aCollisionRecordPtrs[i]->GetModelIndex());
		if(pModelInfo)
		{
			sprintf(gString, "Model:%s Index:%d\n", pModelInfo->GetModelName(), m_aCollisionRecordPtrs[i]->GetModelIndex());
			CDebug::PrintToScreenCoors(gString, 35,count++);
		}
	}
	
	sprintf(gString, "Blocked RArm:%d LArm:%d RMid:%d", m_nPedFlags.bRightArmBlocked, m_nPedFlags.bLeftArmBlocked, m_nPedFlags.bDuckRightArmBlocked);
	CDebug::PrintToScreenCoors(gString, 35,count++);
/*
	if(m_motionAnimGroup==ANIM_SKATEBOARD_PED && GetMoveState()>=PEDMOVE_WALK)
	{
		sprintf(gString, "Skateboard speed = %f", GetPlayerData()->m_fSkateBoardSpeed);
		CDebug::PrintToScreenCoors(gString, 35,count++);
	}
*/	
	sprintf(gString, "Sprint %d = %f (%f)", GetButtonSprintResults(SPRINT_ON_FOOT)>1.0f, GetPlayerData()->m_fSprintControlCounter, GetButtonSprintResults(SPRINT_ON_FOOT));
	CDebug::PrintToScreenCoors(gString, 35,count++);
	
	
	static bool bModMeleeNotGuns = true;
	static uint8 nModSkillWeapon = WEAPONTYPE_FIRST_SKILLWEAPON;
	static uint8 nModMeleeFlag = 1;
	if(bModMeleeNotGuns)
	{
		sprintf(gString, "Melee combo %s moves = %x\n", CTaskSimpleFight::saComboNames[m_nExtraMeleeCombo - MCOMBO_UNARMED_1], m_nExtraMeleeComboFlags);
		CDebug::PrintToScreenCoors(gString, 35,count+=3);

		// change selected melee combo
		if(CPad::GetPad(0)->DPadDownJustDown())		{	m_nExtraMeleeCombo++;	if(m_nExtraMeleeCombo > MCOMBO_UNARMED_4)	m_nExtraMeleeCombo = MCOMBO_UNARMED_4;	else 	nModSkillWeapon=0; }
		else if(CPad::GetPad(0)->DPadUpJustDown())	{	m_nExtraMeleeCombo--;	if(m_nExtraMeleeCombo < MCOMBO_UNARMED_1)	m_nExtraMeleeCombo = MCOMBO_UNARMED_1;	else	nModSkillWeapon=0; }
		// change available moves
		if(CPad::GetPad(0)->DPadRightJustDown())	{	if(nModMeleeFlag < MFLAG_ATTACK_ALL){	m_nExtraMeleeComboFlags += nModMeleeFlag;	nModMeleeFlag *= 2; }	}
		else if(CPad::GetPad(0)->DPadLeftJustDown()){	m_nExtraMeleeComboFlags = 0;	nModMeleeFlag = 1;	}
	}
	else
	{
		// display info
		sprintf(gString, "Weapon skill %s = %s\n", CWeaponInfo::ms_aWeaponNames[nModSkillWeapon], CWeaponInfo::ms_aWeaponSkillNames[GetWeaponSkill(eWeaponType(nModSkillWeapon))]);
		CDebug::PrintToScreenCoors(gString, 35,count+=3);
		// change selected weapon type
		if(CPad::GetPad(0)->DPadDownJustDown())	nModSkillWeapon++;	if(nModSkillWeapon > WEAPONTYPE_LAST_SKILLWEAPON)	nModSkillWeapon = WEAPONTYPE_FIRST_SKILLWEAPON;
		else if(CPad::GetPad(0)->DPadUpJustDown())	nModSkillWeapon--;	if(nModSkillWeapon < WEAPONTYPE_FIRST_SKILLWEAPON)	nModSkillWeapon = WEAPONTYPE_LAST_SKILLWEAPON;
		// change skill level
//		if(CPad::GetPad(0)->DPadRightJustDown())	SetWeaponSkill(eWeaponType(nModSkillWeapon), MIN(WEAPONSKILL_PRO, GetWeaponSkill(eWeaponType(nModSkillWeapon))+1));
//		if(CPad::GetPad(0)->DPadLeftJustDown())	SetWeaponSkill(eWeaponType(nModSkillWeapon), MAX(WEAPONSKILL_POOR, GetWeaponSkill(eWeaponType(nModSkillWeapon))-1));
	}
	
	if(CPad::GetPad(0)->ButtonCrossJustDown())	bModMeleeNotGuns = !bModMeleeNotGuns;
}
	static bool TEST_PRINT_SPEEDS = false;
	static float TEST_OLD_SPEED_2D = 0.0f;
	static float TEST_OLD_SPEED_Z = 0.0f;
	if(TEST_PRINT_SPEEDS)
	{
		printf("\n%d Dspd2D: %f DspdZ: %f Ts:%f\n", CTimer::m_FrameCounter, m_vecMoveSpeed.Magnitude2D() - TEST_OLD_SPEED_2D, m_vecMoveSpeed.z - TEST_OLD_SPEED_Z, CTimer::GetTimeStep());
		TEST_OLD_SPEED_2D = m_vecMoveSpeed.Magnitude2D();
		TEST_OLD_SPEED_Z = m_vecMoveSpeed.z;
	}
#endif

	ASSERTMSG(CharCreatedBy == MISSION_CHAR || CharCreatedBy == REPLAY_CHAR, "CPlayerPed::ProcessControl - Player Ped should always be a Mission character");


	// decrement CarDangerCounter (set by cars if they're gonna hit player)
	if(GetPlayerData()->m_nCarDangerCounter > 0) 
		GetPlayerData()->m_nCarDangerCounter--;
	// if CarDangerCounter down to zero then remove car pointer
	if(GetPlayerData()->m_nCarDangerCounter == 0)
		GetPlayerData()->m_pDangerCar = NULL;

// debug stuff/////

#ifdef PLAYER_INVINCIBLE
m_nHealth = 100;
#endif
//if (GetWantedLevel() < WANTED_LEVEL2)
#ifdef PLAYER_WANTED_2
	SetWantedLevel(WANTED_LEVEL2);
#endif


/////////

//	if (GetWantedLevel() > WANTED_CLEAN)
//	{
//	// commented out for testing- will put the frame thing back in to speed it up
//		if( !((CTimer::m_FrameCounter + RandomSeed) &15) )
//			FindNewAttackPoints();
//	}
	
	// DON'T do a frame counter based thing cause it's done inside the function
//	UpdateMeleeAttackers();

/*
	// if we're standing on (or near) a boat, want rescued from the water if we fall in
	if(m_pGroundPhysical && m_pGroundPhysical->GetIsTypeVehicle() && ((CVehicle *)m_pGroundPhysical)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
		m_nPedFlags.bTryingToReachDryLand = true;
	else if( !((CTimer::m_FrameCounter + RandomSeed) &15) )
	{
		CVehicle *pFindBoat = (CVehicle *) CWorld::TestSphereAgainstWorld(GetPosition(), 7.0f, NULL, false, true, false, false, false);
		if(pFindBoat && pFindBoat->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
			m_nPedFlags.bTryingToReachDryLand = true;
		else
			m_nPedFlags.bTryingToReachDryLand = false;
	}
*/
/*	
	if(GetWeapon()->GetFirstPersonWeaponMode() || TheCamera.Cams[0].Mode == CCam::MODE_HELICANNON_1STPERSON)
	{
		// try and rotate the ped's body to follow the looking up/down of the camera
		LimbOrientation tempTorsoOrientation;
		tempTorsoOrientation.yaw = 0.0f;
		tempTorsoOrientation.pitch = -1.0f*MIN(QUARTER_PI, MAX(-QUARTER_PI, TheCamera.Cams[0].Alpha));
		m_ik.RotateTorso(m_aPedFrames[PED_TORSO], tempTorsoOrientation);
	}
*/
//////////////////////////////////////////////////////
///// DEBUG STUFF FOR TESTING PEDIK RESPONSES ////////
//////////////////////////////////////////////////////
/*
#ifdef GTA_PC	
	static LimbOrientation tempHeadOrientation;
	static LimbOrientation tempArmOrientation;
	static LimbOrientation tempTorsoOrientation;
	static int8 nMouseMoveLimb = 1;
	
	if(GetWeapon()->GetWeaponType() == WEAPONTYPE_UZI && nMouseMoveLimb)
	{
		// add the aiming anim
		CWeaponInfo* pWeaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType());
		CAnimBlendAssociation *pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, (AnimationId)pWeaponInfo->GetAnimation());	
		if(!pAnim)
			pAnim = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, (AnimationId)pWeaponInfo->GetAnimation(), 8.0f);
			
		if(pAnim->GetCurrentTime() > pWeaponInfo->GetAnimFireTime() && pAnim->IsFlagSet(ABA_FLAG_ISPLAYING))
			pAnim->ClearFlag(ABA_FLAG_ISPLAYING);


		// move limb with mouse
		if(nMouseMoveLimb==1){
			tempHeadOrientation.yaw += 0.004f*CPad::GetPad(0)->GetAmountMouseMoved().x;
			tempHeadOrientation.yaw = MIN(PI, MAX(-PI, tempHeadOrientation.yaw));
		
			tempHeadOrientation.pitch -= 0.004f*CPad::GetPad(0)->GetAmountMouseMoved().y;
			tempHeadOrientation.pitch = MIN(HALF_PI, MAX(-HALF_PI, tempHeadOrientation.pitch));
		}
		else if(nMouseMoveLimb==2){
			tempArmOrientation.yaw += 0.004f*CPad::GetPad(0)->GetAmountMouseMoved().x;
			tempArmOrientation.yaw = MIN(PI, MAX(-PI, tempArmOrientation.yaw));
		
			tempArmOrientation.pitch -= 0.004f*CPad::GetPad(0)->GetAmountMouseMoved().y;
			tempArmOrientation.pitch = MIN(HALF_PI, MAX(-HALF_PI, tempArmOrientation.pitch));
		}
		else if(nMouseMoveLimb==3){
			tempTorsoOrientation.yaw += 0.004f*CPad::GetPad(0)->GetAmountMouseMoved().x;
			tempTorsoOrientation.yaw = MIN(PI, MAX(-PI, tempArmOrientation.yaw));
		
			tempTorsoOrientation.pitch -= 0.004f*CPad::GetPad(0)->GetAmountMouseMoved().y;
			tempTorsoOrientation.pitch = MIN(HALF_PI, MAX(-HALF_PI, tempArmOrientation.pitch));
		}
		
		// move torso
		m_ik.RotateTorso(m_aPedFrames[PED_TORSO], tempTorsoOrientation);

		// move actual limb (upper arm)
		RpHAnimStdKeyFrame *iFrame = m_aPedFrames[PED_UPPERARMR]->pStdKeyFrame;
		RtQuatRotate(&iFrame->q, &CVector(1.0f, 0.0f, 0.0f), -RADTODEG(tempHeadOrientation.pitch), rwCOMBINEPOSTCONCAT);
		RtQuatRotate(&iFrame->q, &CVector(0.0f, 0.0f, 1.0f), -RADTODEG(tempHeadOrientation.yaw), rwCOMBINEPOSTCONCAT);
		m_nPedFlags.bUpdateMatricesRequired = true;

		// move actual limb (lower arm)		
		RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pRwObject);
		int32 bone = RpHAnimIDGetIndex(pHierarchy, m_aPedFrames[PED_LOWERARMR]->boneTag);
		AnimBlendFrameData *pAnimFrame = (ANIMBLENDCLUMPFROMCLUMP((RpClump*)m_pRwObject)->GetFrameDataArray() + bone);
		iFrame = pAnimFrame->pStdKeyFrame;
		RtQuatRotate(&iFrame->q, &CVector(0.0f, 0.0f, 1.0f), -RADTODEG(tempArmOrientation.yaw), rwCOMBINEPOSTCONCAT);
		RtQuatRotate(&iFrame->q, &CVector(1.0f, 0.0f, 0.0f), -RADTODEG(tempArmOrientation.pitch), rwCOMBINEPOSTCONCAT);

		sprintf(gString, "Yaw: %f Pitch: %f", tempHeadOrientation.yaw, tempHeadOrientation.pitch);
		CDebug::PrintToScreenCoors(gString, 10,5);
		sprintf(gString, "Yaw: %f Pitch: %f", tempArmOrientation.yaw, tempArmOrientation.pitch);
		CDebug::PrintToScreenCoors(gString, 10,6);
	}
#endif
*/
	if (GetPlayerData()->m_bFadeDrunkenness)
	{
		if ( (GetPlayerData()->m_nDrunkenness - DRUNK_FADE_STEP) <= 0)
		{
			GetPlayerData()->m_nDrunkenness = 0;
			CMBlur::ClearDrunkBlur();
			GetPlayerData()->m_bFadeDrunkenness = false;
		}
		else
		{
			GetPlayerData()->m_nDrunkenness -= DRUNK_FADE_STEP;
		}
	}
	if (GetPlayerData()->m_nDrunkenness)
	{
		CMBlur::SetDrunkBlur(GetPlayerData()->m_nDrunkenness / 255.0f);
	
	}
	
	// if handle player breath didn't get called last frame then call it now
	// (need to do this order because it might be getting called from the player's vehicle process control)
	if(GetPlayerData()->m_bRequireHandleBreath)
		HandlePlayerBreath(false, 1.0f);
	
	GetPlayerData()->m_bRequireHandleBreath = true;

	CPed::ProcessControl();
	// try forcing this extra test on (for the player only)
	m_nPedFlags.bCheckColAboveHead = true;
	
    //SetNearbyPedsToInteractWithPlayer();

//	ProcessVendingMachines();

// test animated hit col model on playerped///////////////
#ifdef DEBUGPLAYERHITCOL
	{
		CPedModelInfo *pModelInfo = (CPedModelInfo *)CModelInfo::GetModelInfo(m_nModelIndex);
//		CColModel *pColModel = pModelInfo->AnimatePedColModelSkinnedWorld((RpClump *)m_pRwObject);
		CColModel *pColModel = pModelInfo->AnimatePedColModelSkinned((RpClump *)m_pRwObject);
		CVector vecPos;
		if(pColModel)
		for(int32 i=0; i<pColModel->m_nNoOfSpheres; i++)
		{
			vecPos = GetMatrix()*pColModel->m_pSphereArray[i].m_vecCentre;
			CDebug::RenderDebugSphere3D(vecPos.x, vecPos.y, vecPos.z, pColModel->m_pSphereArray[i].m_fRadius);
		}
		
		if( (CTimer::m_FrameCounter &15)==0)
		{
			vecPos = GetMatrix()*pColModel->m_sphereBound.m_vecCentre;
			CDebug::RenderDebugSphere3D(vecPos.x, vecPos.y, vecPos.z, pColModel->m_sphereBound.m_fRadius);
		}
	}
#endif
//////////////////////////////////////////////////////////

	if (GetWasPostponed())
	{
		return;
	}

	CPad* pPad = GetPadFromPlayer();
	
	DEBUGPROFILE(CProfile::ResumeProfile(PROFILE_PED_AI_TIME);)


/*
	if (pPad->LastBriefJustDown())
	{
		CMessages::DisplayPreviousBrief();
	}
*/
	GetPlayerWanted()->Update();
	
	PruneReferences();		// Any expired references need to go
	
	if(GetWeapon()->GetWeaponType()==WEAPONTYPE_MINIGUN)
	{
		CWeaponInfo* pWeaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType());
		CAnimBlendAssociation *pAnim = NULL;
		if(GetPedIntelligence()->GetTaskUseGun())
			pAnim = GetPedIntelligence()->GetTaskUseGun()->GetAnim();

		if(pAnim && (pAnim->GetCurrentTime() - pAnim->GetTimeStep() < pWeaponInfo->GetAnimLoopEnd()))
		{
			if(GetPlayerData()->m_fGunSpinSpeed < CHAINGUN_SPIN_MAXSPEED)
			{
				GetPlayerData()->m_fGunSpinSpeed += CHAINGUN_SPINUP_RATE*CTimer::GetTimeStep();
				if(GetPlayerData()->m_fGunSpinSpeed > CHAINGUN_SPIN_MAXSPEED)
					GetPlayerData()->m_fGunSpinSpeed = CHAINGUN_SPIN_MAXSPEED;
			}
			
			if(pPad->GetWeapon(this) && GetWeapon()->m_nAmmoTotal>0	&& pAnim->GetCurrentTime() >= pWeaponInfo->GetAnimLoopStart())
			{
#ifdef USE_AUDIOENGINE
				m_PedWeaponAudioEntity.AddAudioEvent(AUDIO_EVENT_WEAPON_FIRE_MINIGUN_AMMO);
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot( AudioHandle, AE_MINIGUN_FIRE_WITH_AMMO_LOOP, 0.0f);
#endif //USE_AUDIOENGINE
			}
			else
			{
#ifdef USE_AUDIOENGINE
				m_PedWeaponAudioEntity.AddAudioEvent(AUDIO_EVENT_WEAPON_FIRE_MINIGUN_NO_AMMO);
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot( AudioHandle, AE_MINIGUN_FIRE_WITHOUT_AMMO_LOOP, GetPlayerData()->m_fGunSpinSpeed/CHAINGUN_SPIN_MAXSPEED);
#endif //USE_AUDIOENGINE
			}
		}
		else if(GetPlayerData()->m_fGunSpinSpeed > 0.0f)
		{
			// spinning down sound
			if(GetPlayerData()->m_fGunSpinSpeed>=CHAINGUN_SPIN_MAXSPEED)
			{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
				DMAudio.PlayOneShot( AudioHandle, AE_MINIGUN_STOPPING, 0.0f);
#endif //USE_AUDIOENGINE
			}
			
			GetPlayerData()->m_fGunSpinSpeed -= CHAINGUN_SPINDOWN_RATE*CTimer::GetTimeStep();
			if(GetPlayerData()->m_fGunSpinSpeed < 0.0f)
				GetPlayerData()->m_fGunSpinSpeed = 0.0f;
		}
	}

	// Raymond added for chainsaw idle
	if(GetWeapon()->GetWeaponType()== WEAPONTYPE_CHAINSAW && GetPedState() !=PED_ATTACK
		&& !m_nPedFlags.bInVehicle && !GetPedIntelligence()->GetTaskSwim())
	{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
		DMAudio.PlayOneShot(AudioHandle, AE_CHAINSAW_IDLE, 0.0f);
#endif //USE_AUDIOENGINE
	}

	if(GetWeaponLockOn())
	{	
#ifdef GTA_PC
		Clear3rdPersonMouseTarget();
#endif
		CVector vecDrawTargetPos(0.0f,0.0f,0.0f);
		float targetHealth = 1.0f;
		if(GetWeaponLockOnTarget()->GetIsTypePed())
		{
			CWeaponInfo *pWeaponInfo =  CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill());
			CPed *pTargetPed = (CPed *)GetWeaponLockOnTarget();
			// peds max health = 1000.0f
			targetHealth = pTargetPed->m_nHealth / pTargetPed->m_nMaxHealth;
			float fTargetHeadRange = pWeaponInfo->GetTargetHeadRange();
			
			if(pTargetPed->IsAlive()
			&& CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetWeaponFireType()==FIRETYPE_INSTANT_HIT
			&& (pTargetPed->GetPosition() - GetPosition()).MagnitudeSqr() < fTargetHeadRange*fTargetHeadRange)
			{
				GetPlayerData()->m_nTargetBone = BONETAG_HEAD;
				GetPlayerData()->m_vecTargetBoneOffset.x = 0.05f;
			}
			else
			{
				GetPlayerData()->m_nTargetBone = BONETAG_SPINE1;
				GetPlayerData()->m_vecTargetBoneOffset.x = 0.2f;
			}
			vecDrawTargetPos = GetPlayerData()->m_vecTargetBoneOffset;
			pTargetPed->GetTransformedBonePosition(vecDrawTargetPos, GetPlayerData()->m_nTargetBone);

			// add current speed to compensate for being a frame behind
		  	if(targetHealth > 0.0f && !pTargetPed->m_nPedFlags.bInVehicle && pTargetPed->m_eMoveState != PEDMOVE_STILL)
				vecDrawTargetPos +=pTargetPed->GetMoveSpeed()*CTimer::GetTimeStep();
			else if (pTargetPed->m_nPedFlags.bInVehicle && pTargetPed->m_pMyVehicle != NULL)
			{
				vecDrawTargetPos +=(pTargetPed->m_pMyVehicle->GetMoveSpeed() + pTargetPed->m_pMyVehicle->GetTurnSpeed())*CTimer::GetTimeStep();
			}
			// (might be better to pass an entity pointer to MarkTarget 
			//  and update position right before it gets drawn?)
		}
		else if(GetWeaponLockOnTarget()->GetIsTypeVehicle())
		{
			CVehicle *pTargetVehicle = (CVehicle *)GetWeaponLockOnTarget();
			// vehicle's max health == 1000.0f;
			targetHealth = pTargetVehicle->m_nHealth / 1000.0f;
			vecDrawTargetPos = pTargetVehicle->GetPosition() + (pTargetVehicle->GetMoveSpeed() + pTargetVehicle->GetTurnSpeed())*CTimer::GetTimeStep();
		}
		else if(GetWeaponLockOnTarget()->GetIsTypeObject())
		{
			CObject *pTargetObj = (CObject *)GetWeaponLockOnTarget();
			// object's max health == 1000.0f;
			targetHealth = pTargetObj->m_fHealth / 1000.0f;
			vecDrawTargetPos = pTargetObj->GetPosition() + pTargetObj->GetMoveSpeed()*CTimer::GetTimeStep();
		}
		else
			vecDrawTargetPos = GetWeaponLockOnTarget()->GetPosition();
		
		//
		// ped targetted:
		//

		uint8 Red, Green, Blue;
		if (targetHealth > 0.0f)  // if not dead then colour target based on health
		{
			if(targetHealth > 1.0f)	targetHealth = 1.0f;
			Red = targetHealth * TARGET_GOOD_R + (1.0f - targetHealth) * TARGET_DANGER_R;
			Green = targetHealth * TARGET_GOOD_G + (1.0f - targetHealth) * TARGET_DANGER_G;
			Blue = targetHealth * TARGET_GOOD_B + (1.0f - targetHealth) * TARGET_DANGER_B;		
		}
		else  // if dead then its just black
		{
			Red = Green = Blue = 0;
		}
		
		const float dist = 1.0f - (vecDrawTargetPos - GetPosition()).Magnitude() / 50.0f;

		CWeaponEffects::MarkTarget(GetPedType() - PEDTYPE_PLAYER1, vecDrawTargetPos, Red,Green,Blue,255, dist);
	}
//#ifdef GTA_PC
//	else
//	{	
//		Compute3rdPersonMouseTarget();
//	}
//#endif

	if(m_eMoveState!=PEDMOVE_NONE)
	{
		// Restore sprint energy if not running or sprinting
		if(m_eMoveState != PEDMOVE_RUN && m_eMoveState != PEDMOVE_SPRINT)
			HandleSprintEnergy(false, 1.0f);
		// Restore sprint energy at a lower rate if running but not sprinting
		else if(m_eMoveState == PEDMOVE_RUN)
			HandleSprintEnergy(false, 0.3f);
	}
	else if(m_nPedFlags.bInVehicle && m_pMyVehicle && m_pMyVehicle->GetVehicleType()!=VEHICLE_TYPE_BMX)
		HandleSprintEnergy(false, 1.0f);
	
	// moved here so the weapons are still updated after death (for stopping effect)
#ifdef USE_AUDIOENGINE
		GetWeapon()->Update((CPed *)this);
#else //USE_AUDIOENGINE
		GetWeapon()->Update(AudioHandle, (CPed *)this);
#endif //USE_AUDIOENGINE
				
	if (GetPedState() == PED_DEAD)
	{
		ClearWeaponTarget();
		return;
	}
	else if(GetPedState() == PED_DIE)
	{
		ClearWeaponTarget();
//		CalculateNewVelocity();
//		CalculateNewOrientation();
		
//		if(CTimer::GetTimeInMilliseconds() > m_nTimeOfDeath + PLAYER_DIE_TIMELIMIT)
//			SetDead();
		return;
	}
	/*
	//Removed because all enter/exit car is now done as a task.
	else if((GetPedState() == PED_DRIVING)&&(m_Objective != LEAVE_CAR)
#ifdef GTA_NETWORK	
					&& gGameNet.NetWorkStatus == NETSTAT_SINGLEPLAYER	// Don't close door in network game (causes m_nGettingOutFlags to remain set)
#endif
	)
	{
		ASSERT(CReplay::ReplayGoingOn() || m_pMyVehicle);
		if ( (!CReplay::ReplayGoingOn()) || m_pMyVehicle)
		{
			if(m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR && ((CAutomobile*)m_pMyVehicle)->Damage.GetDoorStatus(FRONT_LEFT_DOOR)== DT_DOOR_SWINGING_FREE)
			{
				CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS);
				if(!(m_pMyVehicle->m_nGettingOutFlags & 1))
				{	
					if(!pAnim)
					{
						pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS);

						if(!pAnim)// start new anim here
						{
							// are we not accelerating and not turning?
							if( (!pPad) || (pPad->GetAccelerate()==0.0f && pPad->GetSteeringLeftRight()==0.0f && pPad->GetBrake() == 0.0f))
							{
								// this line is just to prevent anyone else trying to open the door from the outside
								// while I'm closing it from the inside. It gets cleared in the callback
							
//								m_pMyVehicle->m_nGettingOutFlags |= 1;
								m_pMyVehicle->SetGettingInFlags(1);
								
								
								if(m_pMyVehicle->m_bLowVehicle)
									pAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS);
								else
									pAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS);

								pAnim->SetFinishCallback(PedAnimDoorCloseRollingCB, this);
								return;
							}
						}
					}
				}
				// anim is playing, process open door here
				if(pAnim)
					m_pMyVehicle->ProcessOpenDoor(CAR_DOOR_LF, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS, pAnim->GetCurrentTime());
			}
		}
		return; 
	}*/
	else
	{
		// Set the move state to still. This may be changed later on in the function. Most likely 
		// by SetRealMoveAnim()
//		if(m_Objective == NO_OBJ) // only set this if player is not under AI control (cutscenes etc)
//			m_eMoveState = PEDMOVE_STILL;

		DEBUGPLAYERPEDVISIBLEPEDS
		(
			DebugMarkVisiblePeds();
		)

		//float fPow = CMaths::Pow(0.8f, CTimer::GetTimeStep());
		//m_moveBlendRatio *= fPow;
		//if(m_nPedFlags.bIsLanding)
		//	RunningLand(pPad);

		// Some weapons cannot be used now and should produce a negative sound
		if( pPad && pPad->WeaponJustDown(this) )
		{
			eWeaponType WepType = GetWeapon()->GetWeaponType();

			if( (!TheCamera.Using1stPersonWeaponMode() ||
				(GetWeapon()->m_eState == WEAPONSTATE_OUT_OF_AMMO)) &&
				!GetPedIntelligence()->GetTaskSwim() )
			{
				if( WepType==WEAPONTYPE_SNIPERRIFLE )
				{
#ifdef USE_AUDIOENGINE
					AudioEngine.ReportFrontendAudioEvent(AUDIO_EVENT_FRONTEND_FIRE_FAIL_SNIPERRIFFLE);
#else //USE_AUDIOENGINE
					DMAudio.PlayFrontEndSound(AE_SNIPERRIFFLE_FIRE_FAIL, 0.0f);
#endif //USE_AUDIOENGINE
				}
				else if( WepType==WEAPONTYPE_ROCKETLAUNCHER || WepType==WEAPONTYPE_ROCKETLAUNCHER_HS)
				{
#ifdef USE_AUDIOENGINE
					AudioEngine.ReportFrontendAudioEvent(AUDIO_EVENT_FRONTEND_FIRE_FAIL_ROCKET);
#else //USE_AUDIOENGINE
					DMAudio.PlayFrontEndSound(AE_ROCKET_FIRE_FAIL, 0.0f);
#endif //USE_AUDIOENGINE
				}
			}		
		}

// this pedstate stuff will have to go. The new task stuff makes it redundant.
/*
		switch(GetPedState())
		{
		case PED_ARRESTED: // keep the ped lined up while arrested
		{
			if((GetStoredPedState() == PED_DRAGGED_FROM_CAR) && m_pAnim)
			{
				BeingDraggedFromCar(); // continue to keep ped tumbling from car
			}
			break;
		}
		case PED_SNIPER_MODE:
			eWeaponType nWeap = GetWeapon()->GetWeaponType();
			if( nWeap!=WEAPONTYPE_SNIPERRIFLE && nWeap!=WEAPONTYPE_LASERSCOPE )
			{
				if (pPad) PlayerControlM16(pPad);
			}
			else
			{
				if (pPad) PlayerControlSniper(pPad);
			}
			break;
			
		case PED_SEEK_ENTITY:
			AssertEntityPointerValid(m_pTargetEntity);
			m_vecTargetPosition = m_pTargetEntity->GetPosition();// keep track of entity position
		case PED_SEEK_POSITION:
			{
//				CVector2D vec2DDiff = CVector2D(m_vecTargetPosition.x-GetPosition().x, m_vecTargetPosition.y-GetPosition().y);
			
				switch(GetMoveState())
				{
					case PEDMOVE_SPRINT:
						m_moveBlendRatio = 2.5f;break;
					case PEDMOVE_RUN:
						m_moveBlendRatio = 1.8f;break;
					case PEDMOVE_WALK:
						m_moveBlendRatio = 1.0f;break;
					default:
						m_moveBlendRatio = 0.0f;
				}
												
				SetRealMoveAnim();

				if(Seek())
				{
					RestorePreviousState();
					SetMoveState(PEDMOVE_STILL);
				}
			}
			break;
		
		case PED_SEEK_BOAT_POSITION:
		case PED_SEEK_CAR:
			{
				CVector2D vec2DDiff = CVector2D(m_vecTargetPosition.x-GetPosition().x, m_vecTargetPosition.y-GetPosition().y);
			
				if((m_nPedFlags.bIsWaitingToGetInCar)||(m_nPedFlags.bStayInSamePlace))
				{
					m_moveBlendRatio = 0.0f;
				}
				else
				{
					m_moveBlendRatio = vec2DDiff.Magnitude() * 2.0f;
					if(m_moveBlendRatio > 2.0f)
					{
						m_moveBlendRatio = 2.0f;
					}
				}
			}	
			// allow player to break out of automatic control
			if(pPad &&
				!pPad->DisablePlayerControls && 
				(pPad->GetTarget() ||
				pPad->LeftStickXJustActivated() ||
				pPad->LeftStickYJustActivated() ||
				pPad->DPadUpJustDown() ||
				pPad->DPadDownJustDown() ||
				pPad->DPadLeftJustDown() ||
				pPad->DPadRightJustDown()))
			{
				RestorePreviousState();
				if((m_Objective == ENTER_CAR_AS_PASSENGER)||(m_Objective == ENTER_CAR_AS_DRIVER))
				{
					RestorePreviousObjective();
				}
			}
			if(pPad && pPad->GetSprint())
			{
				m_eMoveState = PEDMOVE_SPRINT;
			}	
			SetRealMoveAnim();
			
			break;
			
		case PED_ENTER_TRAIN:
		case PED_EXIT_TRAIN:
		case PED_ENTER_CAR:
		case PED_CARJACK:
		case PED_STEAL_CAR:
		case PED_DRAGGED_FROM_CAR:
		case PED_EXIT_CAR:
		case PED_GETUP:
		case PED_FALL:
			ClearWeaponTarget();
			break;
			
		case PED_IDLE:
		case PED_FLEE_ENTITY:
		case PED_FLEE_POSITION:
		case PED_NONE:
		case PED_AIMGUN:
		case PED_ATTACK:
		case PED_FIGHT:
		case PED_ANSWER_MOBILE:
			CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)m_pRwObject, ABA_PEDFLAG_DONTINTERRUPT);
			if(pAnim == NULL) // don't interrupt certain anims
			{
				if(m_pAttachToEntity != NULL)
				{
					;	// don't do any moving about cause we're attached to something
				}
				else if(TheCamera.Using1stPersonWeaponMode())
				{
					if(pPad) PlayerControlSniper(pPad);
				}
				else if(TheCamera.Cams[0].Using3rdPersonMouseCam())
				{
					if(pPad) PlayerControl1stPersonRunAround(pPad);
				}
				else if(GetPedState()==PED_FIGHT)
				{
					if(pPad) PlayerControlFighter(pPad);
				}
				else
				{
					if(pPad) PlayerControlZelda(pPad);
				}
			}
			if(IsPedInControl() && GetPedState() != PED_ANSWER_MOBILE)
			{
				if(pPad) ProcessPlayerWeapon(pPad);
			}
			break;

		case PED_JUMP:
			if (pPad) PlayerControlZelda(pPad);
			if(!m_nPedFlags.bIsLanding)
				return;
			else
				break;
							
		default:
			break;	
		}
*/		

		if(pPad && IsPedShootable() && GetPedState()!=PED_ANSWER_MOBILE 
		&& /*GetStoredPedState()!=PED_ANSWER_MOBILE &&*/
		!CWorld::Players[CWorld::FindPlayerSlotWithPedPointer(this)].pRemoteVehicle)
		{
			ProcessWeaponSwitch(pPad);
/* // moved further up the function so weapons are updated even if the ped is dying (to turn effects off)
#ifdef USE_AUDIOENGINE
		GetWeapon()->Update((CPed *)this);
#else //USE_AUDIOENGINE
		GetWeapon()->Update(AudioHandle, (CPed *)this);
#endif //USE_AUDIOENGINE
*/
		}
		
		// if player IS using 3rd person weapon controls need to continuously update the animation
		// motion groups for strafing etc. and rotate legs etc if required.
//		if( (TheCamera.Cams[TheCamera.ActiveCam].GetWeaponFirstPersonOn() || TheCamera.Cams[0].Using3rdPersonMouseCam())
//		&& CanStrafeOrMouseControl() )
		// never mind that - actually better to do it always incase of switching control mode!
		{
			ProcessAnimGroups();
		}

		
		if (pPad)		// Only if we're the playerinFocus on this machine
		{
			if ((TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_FOLLOWPED)&&(TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking==LOOKING_BEHIND))
			{
				//we are looking behind
				m_nLookTimer = 0;
				float fCamHeading = CMaths::ATan2(-TheCamera.Cams[TheCamera.ActiveCam].Front.x, TheCamera.Cams[TheCamera.ActiveCam].Front.y);
				fCamHeading = CGeneral::LimitRadianAngle(fCamHeading);
				
				float fAbsDiff = CMaths::Abs(fCamHeading - m_fCurrentHeading);
				
				if(GetPedState()==PED_ATTACK)
					ClearLookFlag();
				else if(fAbsDiff > DEGTORAD(30) && fAbsDiff < DEGTORAD(330))
				{
					if(fAbsDiff > DEGTORAD(150) && fAbsDiff < DEGTORAD(210))
					{
						float fLimitAngle1 = CGeneral::LimitRadianAngle(m_fCurrentHeading - DEGTORAD(150));
						float fLimitAngle2 = CGeneral::LimitRadianAngle(m_fCurrentHeading + DEGTORAD(150));
					
						if(m_fLookHeading == ADAMS_THIS_IS_AN_INVALID_LOOK_HEADING || m_nPedFlags.bIsDucking)
							fCamHeading = fLimitAngle1;
						else if(CMaths::Abs(fLimitAngle1 - m_fLookHeading) < CMaths::Abs(fLimitAngle2 - m_fLookHeading))
							fCamHeading = fLimitAngle1;
						else
							fCamHeading = fLimitAngle2;
					}
				
					SetLookFlag(fCamHeading, true);
					SetLookTimer(5.0f * CTimer::GetTimeElapsedInMilliseconds());
				}
				else
					ClearLookFlag();
			} 
		}

//		CalculateNewVelocity();
//		CalculateNewOrientation();

		if(m_eMoveState==PEDMOVE_SPRINT && m_nPedFlags.bIsLooking){
			ClearLookFlag();
			SetLookTimer(250);	// otherwise won't look again for 4000msec
		}

		CVector2D vec2DMoveSpeed;
		vec2DMoveSpeed = CVector2D::ConvertTo2D(GetMoveSpeed());
		if(vec2DMoveSpeed.Magnitude() < 0.1f)
		{
			if(GetPlayerData()->m_nStandStillTimer == 0)
			{
				// have stopped moving
				GetPlayerData()->m_nStandStillTimer = CTimer::GetTimeInMilliseconds() + PLAYER_STAND_STILL_LIMIT;
			}
			else // remaining stopped
			{
				if(CTimer::GetTimeInMilliseconds() > GetPlayerData()->m_nStandStillTimer) // not zero but we have passed the time limit
				{
					GetPlayerData()->m_bStoppedMoving = true;
				}
			}
		}
		else // moving again
		{
			GetPlayerData()->m_nStandStillTimer = 0;
			GetPlayerData()->m_bStoppedMoving = false;			
		}

		DEBUGPROFILE(CProfile::SuspendProfile(PROFILE_PED_AI_TIME);)
	}
	
	if (GetPlayerData()->m_bDontAllowWeaponChange && 
//		this == FindPlayerPed() && 
		this->IsPlayer() &&
		!CPad::GetPad(0)->GetTarget()) 
			GetPlayerData()->m_bDontAllowWeaponChange = false;
	
	
	if(GetPedState()!=PED_SNIPER_MODE && (GetWeapon()->m_eState == WEAPONSTATE_FIRING))
	{
		GetPlayerData()->m_LastTimeFiring = CTimer::GetTimeInMilliseconds();
	}
	
	
	ProcessGroupBehaviour(pPad);
	
	if (m_nPedFlags.bInVehicle) CCarCtrl::RegisterVehicleOfInterest(m_pMyVehicle);
	
	// This is normally done in PreRender, but if player ped isn't visible
	// (ie because we're in FPS mode, PreRender won't get called, so call it here instead
	if(!m_nFlags.bIsVisible)
	{
		UpdateRpHAnim();
	}
	
	// update whether the players in car gang are active or passive
	if (m_nPedFlags.bInVehicle)
	{
		if (CPad::GetPad(0)->DPadDownJustDown())
	    {
	    	GetPlayerData()->m_playersGangActive = false;
	    }
	    else if (CPad::GetPad(0)->DPadUpJustDown())
	    {
	    	GetPlayerData()->m_playersGangActive = true;
	    }
	}
	
	// calc how submerged the player is and store water height
	if (m_nPhysicalFlags.bIsInWater)
	{
		CVector pedPos = GetPosition();
		// position might have got moved since we did the water check?
		if(CWaterLevel::GetWaterLevel(pedPos.x, pedPos.y, pedPos.z+1.5f, &GetPlayerData()->m_waterHeight, true))
		{
			float playerHeightMin = pedPos.z + GetColModel().GetBoundBoxMin().z;
			float playerHeightMax = pedPos.z + GetColModel().GetBoundBoxMax().z;
	
			if (GetPlayerData()->m_waterHeight>=playerHeightMax)
			{
				GetPlayerData()->m_waterCoverPerc = 100;
			}
			else if (GetPlayerData()->m_waterHeight<=playerHeightMin)
			{
				GetPlayerData()->m_waterCoverPerc = 0;
			}
			else
			{
				GetPlayerData()->m_waterCoverPerc = (uint8)(100*((GetPlayerData()->m_waterHeight-playerHeightMin)/(playerHeightMax-playerHeightMin)));
			}
		}
		else
			m_nPhysicalFlags.bIsInWater = false;
	}
	else
	{
		GetPlayerData()->m_waterCoverPerc = 0;
	}
	
	
	// Occasionally we might trigger some samples here.
	// Ones that have to do with the gang stuff.
	if ((CTimer::m_FrameCounter & 127) == 0)
	{
		if (!FindPlayerVehicle())
		{
				// Make sure the group is in follow mode.
			if (CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].m_followLeader)
			{
				Int32	NumMembers = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader();
			
				if (NumMembers > 0 &&  CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].FindDistanceToNearestMember() > 20.0f &&
					CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].FindDistanceToNearestMember() < 100.0f &&
					CGame::currArea == AREA_MAIN_MAP)
				{
					if (NumMembers == 1)
					{
						Say(CONTEXT_GLOBAL_ORDER_KEEP_UP_ONE);
					}
					else
					{
						Say(CONTEXT_GLOBAL_ORDER_KEEP_UP_MANY);
					}
					
					// Gang members reply.
					for (Int32 M = 0; M < CPedGroupMembership::LEADER; M++)
					{
						CPed *pPed = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->GetMember(M);
						if (pPed && CGeneral::GetRandomNumberInRange(0.0f, 1.0f) < 0.5f)
						{
							pPed->Say(CONTEXT_GLOBAL_FOLLOW_REPLY, CGeneral::GetRandomNumberInRange(3000, 4500));
						}
					}
				}
			}
// we now trigger CHASED from TaskPolice, to make sure it ties in with real live, nearby cops better			
//			if (FindPlayerWanted()->m_nCopsInPursuit > 0 && (GetMoveState() == PEDMOVE_JOG || GetMoveState() == PEDMOVE_RUN || GetMoveState() == PEDMOVE_SPRINT))
//			{
//				Say(CONTEXT_GLOBAL_CHASED);
//			}
		}
	}
	
	if(IsHidden() && !CEntryExitManager::WeAreInInteriorTransition())//!GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_GOTO_DOOR_AND_OPEN))
	{
		//Ped's total light level is low and isn't going through an entry exit.
		Say(CONTEXT_GLOBAL_BREATHING);
	}
	
} // end - CPlayerPed::ProcessControl

bool CPlayerPed::IsHidden() const
{
	#define HIDDEN_THRESHOLD (0.05f)
	return (!m_nPedFlags.bInVehicle && GetLightingTotal() <= HIDDEN_THRESHOLD);
}


#ifndef MASTER
// Name			:	PutPlayerAtCoords
// Purpose		:	Places player at passed coords (used in varconsole)
void CPlayerPed::PutPlayerAtCoords(float x, float y, float z)
{
	CPed* pPlayer = FindPlayerPed();
	pPlayer->Teleport(CVector(x,y,z));
}
#endif  // MASTER



// Name			:	ReApplyMoveAnims
// Purpose		:	Remove old animations related to movement and add in new ones
void CPlayerPed::ReApplyMoveAnims()
{
////////////////////////////////////////////////////////	
// Be carefull where this is called from, can't have any BlendAnim() calls
// for non-partial anims after this or the old move anims getting blended out may not be removed
////////////////////////////////////////////////////////
	AnimationId moveAnims[5] = {ANIM_STD_WALK, ANIM_STD_RUN, ANIM_STD_RUNFAST, ANIM_STD_IDLE, ANIM_STD_STARTWALK};
	CAnimBlendAssociation* pAnim;
	CAnimBlendStaticAssociation* pNeededAnim;
	
	for(int32 i=0; i<5; i++)
	{
		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, moveAnims[i]);
		if(pAnim)
		{
			pNeededAnim = CAnimManager::GetAnimAssociation(m_motionAnimGroup, moveAnims[i]);
			// only want to re-apply anims if anim names differ
			if(pAnim->GetAnimHashKey() != pNeededAnim->GetAnimHashKey())
			{
				CAnimBlendAssociation* pNewAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, moveAnims[i]);
				pNewAnim->SetBlendDelta(pAnim->GetBlendDelta());  
				pNewAnim->SetBlendAmount(pAnim->GetBlendAmount());
//				pNewAnim->ClearFlag(0xffffffff);
//				pNewAnim->SetFlag(pAnim->GetFlags());
				pAnim->SetBlendDelta(-1000.0f);
				pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
//////////////////////////////////////////////////
//				printf("Replaced anim %s with %s\n",pAnim->GetAnimName(), pNewAnim->GetAnimName());
//////////////////////////////////////////////////
			}
		}	
	}
}


// NOTE: Not using 1st person stuff at the moment, but useful to keep this function incase
// we want to put it back in again, cause fn's a bit tricky (not really but can't be assed doing it again)
#ifdef FPS_MODE_AVAILABLE

#define SWITCH_ALL_FPS
void CPlayerPed::SwitchFPSMode(bool FPS_OnOff)
{
	CAnimBlendAssociation* pAnim;
	CAnimBlendAssociation* pNewAnim;
	CWeaponInfo *pWeaponInfo;
	AnimationId nOldAnim = ANIM_STD_NUM;
	
	// if we're already in the correct mode - don't need to do anything, so return
	if(GetWeapon()->GetFirstPersonWeaponMode() == FPS_OnOff)
		return;

#ifdef SWITCH_ALL_FPS
	for(uint16 i = WEAPONTYPE_UNARMED; i<WEAPONTYPE_LAST_WEAPONTYPE; i++)
	{
		pWeaponInfo = CWeaponInfo::GetWeaponInfo(eWeaponType(i));
		nOldAnim = AnimationId(pWeaponInfo->GetAnimation(m_wepWeapon[i].GetFirstPersonWeaponMode()));
		pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, nOldAnim);

		m_wepWeapon[i].SetFirstPersonWeaponMode(FPS_OnOff);
	
		if(pAnim && nOldAnim != pWeaponInfo->GetAnimation(GetWeapon()->GetFirstPersonWeaponMode()))
		{
			pNewAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, AnimationId(pWeaponInfo->GetAnimation(GetWeapon()->GetFirstPersonWeaponMode())) );
			pNewAnim->SetBlendDelta(pAnim->GetBlendDelta());  
			pNewAnim->SetBlendAmount(pAnim->GetBlendAmount());
			pNewAnim->ClearFlag(0xffffffff);
			pNewAnim->SetFlag(pAnim->GetFlags());
			pAnim->SetBlendDelta(-1000.0f);
			pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
//////////////////////////////////////////////////
//			printf("Replaced anim %s with %s\n",pAnim->GetAnimName(), pNewAnim->GetAnimName());
//////////////////////////////////////////////////
		}
	}
#else
	pWeaponInfo = GetWeaponInfo(GetWeapon()->GetWeaponType());
	nOldAnim = pWeaponInfo->GetAnimation(GetWeapon()->GetFirstPersonWeaponMode());
	pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, nOldAnim);

	GetWeapon()->SetFirstPersonWeaponMode(!FPS_OnOff);
	
	if(pAnim && nOldAnim != pWeaponInfo->GetAnimation(GetWeapon()->GetFirstPersonWeaponMode()))
	{
		pNewAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, pWeaponInfo->GetAnimation(GetWeapon()->GetFirstPersonWeaponMode()));
		pNewAnim->SetBlendDelta(pAnim->GetBlendDelta());  
		pNewAnim->SetBlendAmount(pAnim->GetBlendAmount());
		pNewAnim->ClearFlag(0xffffffff);
		pNewAnim->SetFlag(pAnim->GetFlags());
		pAnim->SetBlendDelta(-1000.0f);
		pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
//////////////////////////////////////////////////
//		printf("Replaced anim %s with %s\n",pAnim->GetAnimName(), pNewAnim->GetAnimName());
//////////////////////////////////////////////////
	}
#endif
}

#endif	// #ifdef FPS_MODE_AVAILABLE


// Name			:	UpdateMoveAnims
// Purpose		:	Update the animations player ped uses to move around
/*
float PLAYER_SKATEBOARD_MAXSPEED_SPRINT = 0.3f;
float PLAYER_SKATEBOARD_MAXSPEED_ABS = 2.0f;
float PLAYER_SKATEBOARD_MINSPEED_RUN = -0.2f;
float PLAYER_SKATEBOARD_MINSPEED_ABS = -0.5f;
float PLAYER_SKATEBOARD_DOWNHILL_MULT = 0.10f;
float PLAYER_SKATEBOARD_DOWNHILL_DAMP = 0.99f;
float PLAYER_SKATEBOARD_UPHILL_MULT = 0.025f;
float PLAYER_SKATEBOARD_UPHILL_DAMP = 0.97f;
float PLAYER_SKATEBOARD_DOWNHILL_SKIDSTOP = 0.95f;
*/
//
void CPlayerPed::SetRealMoveAnim()
{
	CAnimBlendAssociation* pWalk = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_WALK);
	CAnimBlendAssociation* pRun = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_RUN);
	CAnimBlendAssociation* pSprint = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_RUNFAST);
	CAnimBlendAssociation* pStartWalk = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_STARTWALK);
	CAnimBlendAssociation* pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);
	CAnimBlendAssociation* pRunStop1 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_RUNSTOP1);
	CAnimBlendAssociation* pRunStop2 = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_RUNSTOP2);
	CAnimBlendAssociation *pLeft = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_TURN_L);
	CAnimBlendAssociation *pRight = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_TURN_R);
		

	CAnimBlendAssociation* pIdleTired = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE_TIRED);
	
	if(m_nPedFlags.bResetWalkAnims)
	{
		if(pWalk)
			pWalk->SetCurrentTime(0.0f);
		if(pRun)
			pRun->SetCurrentTime(0.0f);
		if(pSprint)
			pSprint->SetCurrentTime(0.0f);
		m_nPedFlags.bResetWalkAnims = false;
	}
			
//	if(pIdle == NULL)
//	{
//		pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE_TIRED);
//	}
//	if(pIdle == NULL)
//	{
//		pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FIGHT_IDLE);
//		if(pIdle==NULL)	pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_ATTACK_EXTRA2);
//	}
	
	if( (pRunStop1 && pRunStop1->IsFlagSet(ABA_FLAG_ISPLAYING)) ||
		(pRunStop2 && pRunStop2->IsFlagSet(ABA_FLAG_ISPLAYING)) )
	{
//		if(m_motionAnimGroup==ANIM_SKATEBOARD_PED && pRunStop2)
//			m_eMoveState = PEDMOVE_STILL;
//		else
		{
			m_eMoveState = PEDMOVE_RUN;	// don't do anything till stopped
			
//			if(GetPlayerData()->m_fSkateBoardSpeed > 0.1f)
//			{
//				if(pRunStop1 && pRunStop1->IsFlagSet(ABA_FLAG_ISPLAYING)
//				&& pRunStop1->GetCurrentTime() > 0.5f*pRunStop1->GetTotalTime())
//					pRunStop1->SetCurrentTime(pRunStop1->GetCurrentTime() - pRunStop1->GetTimeStep());
//			}
//			else
			if(pRunStop1 && !pRunStop1->IsFlagSet(ABA_FLAG_ISPLAYING)
			&& pRunStop1->GetCurrentTime() < pRunStop1->GetTotalTime())
			{
				pRunStop1->SetFlag(ABA_FLAG_ISPLAYING);
			}
		}
	}
	else if( (pRunStop1 && pRunStop1->GetBlendDelta()>=0.0f) || 
			 (pRunStop2 && pRunStop2->GetBlendDelta()>=0.0f))
	{
		if(pRunStop1){
			pRunStop1->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			pRunStop1->SetBlendAmount(1.0f);
			pRunStop1->SetBlendDelta(-8.0f);
		}
		else if(pRunStop2){
			pRunStop2->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			pRunStop2->SetBlendAmount(1.0f);
			pRunStop2->SetBlendDelta(-8.0f);
		}
		RestoreHeadingRate();
		if(!pIdle){
			// if sprint energy is less than zero and not aiming gun and there isn't a building in the way
			// play tired idle
//			if(GetPlayerData()->m_fSprintEnergy < 0 && !m_nPedFlags.bIsAimingGun && !CWorld::TestSphereAgainstWorld(GetPosition(), 0.5f, NULL, true, false, false, false, false))
//			{
//				pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_IDLE_TIRED, 8.0f);
//			}
//			else	
				pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 8.0f);
//			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2500,4000);	// for alt idle anim switch
		}
/*
		if(m_motionAnimGroup==ANIM_SKATEBOARD_PED && pRunStop1 && pRunStop1->GetAnimGrp()==ANIM_SKATEBOARD_PED)
		{
			CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUNSTOP2, 1000.0f);
			// blend animation on skateboard
			if(m_pWeaponClump)
			{
				// get rid of any existing anims since we can't blend this one in smoothly
				RpAnimBlendClumpSetBlendDeltas(m_pWeaponClump, 0, -100.0f);
				CAnimBlendStaticAssociation* pBlendStaticAssoc = CAnimManager::GetAnimAssociation(ANIM_FLIP_SKATEBOARD, ANIM_FLIPUP_SKATEBOARD);
				CAnimBlendHierarchy* pBlendHier = pBlendStaticAssoc->GetAnimHierarchy();
				CAnimBlendAssociation *pBoardAnim = CAnimManager::AddAnimation(m_pWeaponClump, pBlendHier, ABA_FLAG_ISBLENDAUTOREMOVE);
				pBoardAnim->SetAnimId(ANIM_FLIPUP_SKATEBOARD);
				pBoardAnim->SetAnimGrp(ANIM_FLIP_SKATEBOARD);
			}
		}
*/
		pIdle->SetBlendAmount(0.0f);
		pIdle->SetBlendDelta(8.0f);
	}
	else if(GetPlayerData()->m_moveBlendRatio == 0.0f && pSprint==NULL)
	{
		if(GetPadFromPlayer()->ForceCameraBehindPlayer() && GetPadFromPlayer()->AimWeaponLeftRight(NULL)
		&& TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_FOLLOWPED)
		{
			static float PLAYER_TURN_ON_SPOT_ANIM_SPEED_MULT = 1.0f;
			if(GetPadFromPlayer()->AimWeaponLeftRight(NULL) < 0.0)
			{
				if(pLeft==NULL || pLeft->GetBlendDelta() < 0.0f || (pLeft->GetBlendAmount() < 1.0f && pLeft->GetBlendDelta()<=0.0f))
					pLeft = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_TURN_L, 16.0f);
				
				pLeft->SetSpeed(PLAYER_TURN_ON_SPOT_ANIM_SPEED_MULT*CMaths::Abs(GetPadFromPlayer()->AimWeaponLeftRight(NULL))/128.0f);
			}
			else
			{
				if(pRight==NULL || pRight->GetBlendDelta() < 0.0f || (pRight->GetBlendAmount() < 1.0f && pRight->GetBlendDelta()<=0.0f))
					pRight = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_TURN_R, 16.0f);
			
				pRight->SetSpeed(PLAYER_TURN_ON_SPOT_ANIM_SPEED_MULT*CMaths::Abs(GetPadFromPlayer()->AimWeaponLeftRight(NULL))/128.0f);
			}
			if(pIdle && pIdle->GetBlendAmount()<=0.01f)
				delete pIdle;
		}
		else if(pIdle==NULL)
			pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 4.0f);



		if(GetPlayerData()->m_fSprintEnergy < 0
		&& !GetPedIntelligence()->GetTaskFighting()	
		&& !GetPedIntelligence()->GetTaskUseGun()
		&& !GetPedIntelligence()->GetTaskDuck() 
		&& !GetPedIntelligence()->GetTaskThrow()
		&& !GetPedIntelligence()->GetTaskJetPack()
		&& !CWorld::TestSphereAgainstWorld(GetPosition(), 0.5f, NULL, true, false, false, false, false))
		{
			if(pIdleTired==NULL)
			{
				if (CClothes::GetDefaultPlayerMotionGroup() == ANIM_PLAYER_FAT_PED)
					pIdleTired = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_PLAYER_FAT_TIRED_PED, ANIM_STD_IDLE_TIRED, 4.0f);
				else
					pIdleTired = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_IDLE_TIRED, 4.0f);
				pIdleTired->SetFlag(ABA_FLAG_ISPLAYING);
			}
			else
			{
				//Every fourth frame breathe in our out in sequence (in out in out in out etc).
				const int freq=4;
				const int freqDouble=2*freq;
				if(0==(CTimer::m_FrameCounter%freq))
				{
					if(0==(CTimer::m_FrameCounter%freqDouble))
					{
						//Breathe in.
						//Say(CONTEXT_GLOBAL_PAIN_CJ_PANT_IN);
					}
					else
					{
						//Say(CONTEXT_GLOBAL_PAIN_CJ_PANT_OUT);
					}
				}
			}
		}
		else if(pIdleTired && pIdleTired->GetBlendAmount() > 0.0f && pIdleTired->GetBlendDelta() >= 0.0f)
		{
			pIdleTired->ClearFlag(ABA_FLAG_ISPLAYING);
			pIdleTired->SetBlendDelta(-2.0f);
		}

/*		if(!pIdle)
		{
			if(GetPlayerData()->m_fSprintEnergy < 0 && !m_nPedFlags.bIsAimingGun && !CWorld::TestSphereAgainstWorld(GetPosition(), 0.5f, NULL, true, false, false, false, false))
			{
				pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_IDLE_TIRED, 4.0f);
			}
			else	
				pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 4.0f);
			m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2500,4000);	// for alt idle anim switch
		}

		// if sprint energy is greater than zero or aiming gun and current anim is tired anim then blend in standard idle anim
		if((GetPlayerData()->m_fSprintEnergy > 0.0f || m_nPedFlags.bIsAimingGun) && pIdle->GetAnimId() == ANIM_STD_IDLE_TIRED)
			pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 4.0f);
		// else if not in fight state, and current anim is fight idle anim then blend in standard idle anim
		else if(GetPedState() != PED_FIGHT)
		{
			if(GetPlayerData()->m_fSprintEnergy < 0 && !m_nPedFlags.bIsAimingGun && pIdle->GetAnimId()!=ANIM_STD_IDLE_TIRED&& !CWorld::TestSphereAgainstWorld(GetPosition(), 0.5f, NULL, true, false, false, false, false))
				pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_IDLE_TIRED, 4.0f);
			else if(pIdle->GetAnimId()!=ANIM_STD_IDLE)
				pIdle = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_IDLE, 4.0f);
		}
*/
/*
		if(m_motionAnimGroup==ANIM_SKATEBOARD_PED 
		&& ((pRun && pRun->GetAnimGrp()==ANIM_SKATEBOARD_PED)// && pRun->GetBlendAmount()>0.5f) 
		|| (pWalk && pWalk->GetAnimGrp()==ANIM_STD_RUNSTOP1)))// && pWalk->GetBlendAmount()>0.5f)))
		{
			if(pRun && pRun->GetAnimGrp()==ANIM_SKATEBOARD_PED && pRun->GetBlendAmount()>0.5f)
			{
				pRunStop1 = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUNSTOP1, 8.0);
				pRunStop1->SetDeleteCallback(RestoreHeadingRateCB, this);
				m_eMoveState = PEDMOVE_RUN;
			}
			else
			{
				CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUNSTOP2, 1000.0);
				m_eMoveState = PEDMOVE_STILL;
				// blend animation on skateboard
				if(m_pWeaponClump)
				{
					// get rid of any existing anims since we can't blend this one in smoothly
					RpAnimBlendClumpSetBlendDeltas(m_pWeaponClump, 0, -100.0f);
					CAnimBlendStaticAssociation* pBlendStaticAssoc = CAnimManager::GetAnimAssociation(ANIM_FLIP_SKATEBOARD, ANIM_FLIPUP_SKATEBOARD);
					CAnimBlendHierarchy* pBlendHier = pBlendStaticAssoc->GetAnimHierarchy();
					CAnimBlendAssociation *pBoardAnim = CAnimManager::AddAnimation(m_pWeaponClump, pBlendHier, ABA_FLAG_ISBLENDAUTOREMOVE);
					pBoardAnim->SetAnimId(ANIM_FLIPUP_SKATEBOARD);
					pBoardAnim->SetAnimGrp(ANIM_FLIP_SKATEBOARD);
				}
			}
		}
		else
*/		
			m_eMoveState = PEDMOVE_STILL;
	}
	else
	{
		// If an idle anim is playing then remove animation set walk/run blend deltas to zero and add a
		// start walk animation
		if(pIdle)
		{
			if(pStartWalk == NULL)
			{
				pStartWalk = CAnimManager::AddAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_STARTWALK);
/*
				// blend animation on skateboard
				if(m_motionAnimGroup==ANIM_SKATEBOARD_PED && m_pWeaponClump)
				{
					// get rid of any existing anims since we can't blend this one in smoothly
					RpAnimBlendClumpSetBlendDeltas(m_pWeaponClump, 0, -100.0f);
					CAnimBlendStaticAssociation* pBlendStaticAssoc = CAnimManager::GetAnimAssociation(ANIM_FLIP_SKATEBOARD, ANIM_PUTDOWN_SKATEBOARD);
					CAnimBlendHierarchy* pBlendHier = pBlendStaticAssoc->GetAnimHierarchy();
					CAnimBlendAssociation *pBoardAnim = CAnimManager::AddAnimation(m_pWeaponClump, pBlendHier, ABA_FLAG_ISBLENDAUTOREMOVE);
					pBoardAnim->SetAnimId(ANIM_PUTDOWN_SKATEBOARD);
					pBoardAnim->SetAnimGrp(ANIM_FLIP_SKATEBOARD);
				}
*/
			}
			else
			{
				pStartWalk->SetBlendAmount(1.0f);
				pStartWalk->SetBlendDelta(0.0f);	
			}
			if(pWalk)
				pWalk->SetCurrentTime(0.0f);
			if(pRun)
				pRun->SetCurrentTime(0.0f);
			delete pIdle;
			
			//Also remove idle tired if both idles are being used
//			pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE_TIRED);		
//			if(pIdle)
//				delete pIdle;
			if(pIdleTired)
				pIdleTired->SetBlendDelta(-4.0f);

			//Also remove fight idle if it's there
//			pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FIGHT_IDLE);		
//			if(!pIdle) pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_ATTACK_EXTRA2);		
//			if(pIdle)
//				delete pIdle;
			//Also remove sprint if it's here (can't be sprinting and starting to walk)
			if(pSprint)
				delete pSprint;
			
			// reset the pointers so we know they're not there anymore
			pSprint = NULL;
			pIdle = NULL;
							
			m_eMoveState = PEDMOVE_WALK;
		}
		
		if(pRunStop1){
			delete pRunStop1;
			pRunStop1 = NULL;
			RestoreHeadingRate();	
		}
		if(pRunStop2){
			delete pRunStop2;
			pRunStop2 = NULL;
			RestoreHeadingRate();
		}
		if(pLeft){
			delete pLeft;
			pLeft = NULL;
		}
		if(pRight){
			delete pRight;
			pRight = NULL;
		}

		// Add run and walk anims if they don't exist. Set their blend amounts to zero
		if(pWalk == NULL)
		{
			pWalk = CAnimManager::AddAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_WALK);
			pWalk->SetBlendAmount(0.0f);
		}	
		if(pRun == NULL)
		{
			pRun = CAnimManager::AddAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUN);
			pRun->SetBlendAmount(0.0f);
		}	
			
		// If there is no start walk animation or it has stopped playing then start walking or running
		if(pStartWalk && (!pStartWalk->IsFlagSet(ABA_FLAG_ISPLAYING)
		|| pStartWalk->GetCurrentTime() + pStartWalk->GetTimeStep() >= pStartWalk->GetTotalTime()))
		{
			delete pStartWalk;
			pStartWalk = NULL;
			pWalk->SetFlag(ABA_FLAG_ISPLAYING);
			pRun->SetFlag(ABA_FLAG_ISPLAYING);
		}

		// If Sprint button pressed then check if can still sprint
		if(m_eMoveState == PEDMOVE_SPRINT)
		{
//			if(GetPlayerData()->m_fSprintEnergy < 0.0f && 
//				(GetPlayerData()->m_fSprintEnergy <= MIN_SPRINT_ENERGY || (pSprint==NULL||pSprint->GetBlendDelta()<0.0f) ))
//				m_eMoveState = PEDMOVE_STILL;
			if(pStartWalk)
				m_eMoveState = PEDMOVE_STILL;
//			if(GetPlayerData()->m_fSkateBoardSpeed > PLAYER_SKATEBOARD_MAXSPEED_SPRINT)
//				m_eMoveState = PEDMOVE_STILL;
		}
//		else
//		{
//			if(GetPlayerData()->m_fSkateBoardSpeed < PLAYER_SKATEBOARD_MINSPEED_RUN)
//				GetPlayerData()->m_moveBlendRatio = 1.0f;
//		}
		
		// If not sprinting and sprint anim exists delete it
		if(pSprint && (m_eMoveState!=PEDMOVE_SPRINT || GetPlayerData()->m_moveBlendRatio<0.40f) )
		{
			// if the sprint anim is there but not used, make sure it get's removed
			if(pSprint->GetBlendAmount() == 0.0f){
				pSprint->SetBlendDelta(-1000.0f);
				pSprint->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			}
			// if sprint in process of being blended out, or isn't playing -> do nothing
			else if(pSprint->GetBlendDelta() < 0.0f && pSprint->GetBlendAmount() < 0.8f){// || !pSprint->IsFlagSet(ABA_FLAG_ISPLAYING))
				if(GetPlayerData()->m_moveBlendRatio < 1.0f){
					pSprint->SetBlendDelta(-8.0f);
					pRun->SetBlendDelta(8.0f);
				}
			}
			// if wanting to stop completely (and no partials playing)
			else if(GetPlayerData()->m_moveBlendRatio < 0.40f)
			{
/*			
				if(m_motionAnimGroup==ANIM_SKATEBOARD_PED && pSprint->GetAnimGrp()==ANIM_SKATEBOARD_PED){
					pRunStop1 = CAnimManager::AddAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUNSTOP1);
					pRunStop1->SetBlendAmount(1.0f);
					pRunStop1->SetDeleteCallback(RestoreHeadingRateCB, this);
				}
				else
*/
				if((pSprint->GetCurrentTime() / pSprint->GetTotalTime()) < 0.5){
					pRunStop1 = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_RUNSTOP1);
					pRunStop1->SetBlendAmount(1.0f);
					pRunStop1->SetDeleteCallback(RestoreHeadingRateCB, this);
				}
				else{
					pRunStop2 = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_RUNSTOP2);
					pRunStop2->SetBlendAmount(1.0f);
					pRunStop2->SetDeleteCallback(RestoreHeadingRateCB, this);
				}
				SetHeadingRate(0.0f);
				pSprint->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				pSprint->SetBlendDelta(-1000.0f);
				pWalk->ClearFlag(ABA_FLAG_ISPLAYING);
				pRun->ClearFlag(ABA_FLAG_ISPLAYING);
				pWalk->SetBlendAmount(0.0f);
				pRun->SetBlendAmount(0.0f);
				pWalk->SetBlendDelta(0.0f);
				pRun->SetBlendDelta(0.0f);
			}
			else if(pSprint->GetBlendDelta() >= 0.0f){
				pSprint->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
				pSprint->SetBlendDelta(-1.0f);
				pRun->SetBlendDelta(1.0f);
			}

			if(GetPlayerData()->m_moveBlendRatio > 1.0f)
				m_eMoveState = PEDMOVE_RUN;
			else
				m_eMoveState = PEDMOVE_WALK;
		}
		// if start walk anim is playing stop walk and run playing and set their blend amounts to zero
		else if(pStartWalk)
		{
			pWalk->ClearFlag(ABA_FLAG_ISPLAYING);
			pRun->ClearFlag(ABA_FLAG_ISPLAYING);
			pWalk->SetBlendAmount(0.0f);
			pRun->SetBlendAmount(0.0f);
		}
		// if sprint state is set then if sprint doesn't exist add sprint anim, reduce sprint energy value
		else if(m_eMoveState == PEDMOVE_SPRINT)
		{
			if(pSprint == NULL)
			{
				if(pRun->GetBlendAmount() < 1.0f)
				{
					if(pWalk->GetBlendAmount()==0.0f && pRun->GetBlendAmount()==0.0f)
						pWalk->SetBlendAmount(1.0f);
				
					if(pRun->GetBlendDelta() <= 0.0f)
						pRun = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUN, 4.0f);

					GetPlayerData()->m_moveBlendRatio = 1.0f + pRun->GetBlendDelta();
				}
				else
				{
					pSprint = CAnimManager::BlendAnimation((RpClump*)m_pRwObject, m_motionAnimGroup, ANIM_STD_RUNFAST, 2.0f);
				}			
			}
			else if(pSprint->GetBlendDelta() < 0.0f)
			{
				pSprint->SetBlendDelta(2.0f);
				pRun->SetBlendDelta(-2.0f);
			}	

			if(pSprint)
			{
//				UseSprintEnergy();
				CStats::UpdateStatsWhenSprinting();
			}
		}
		// if walking
		else if(GetPlayerData()->m_moveBlendRatio < 1.0f)
		{
			pWalk->SetBlendAmount(1.0f);
			pRun->SetBlendAmount(0.0f);
			pWalk->SetBlendDelta(0.0f);
			pRun->SetBlendDelta(0.0f);
			m_eMoveState = PEDMOVE_WALK;
		}
		// if between walking and running
		else if(GetPlayerData()->m_moveBlendRatio < 2.0f)
		{
			pWalk->SetBlendAmount(2.0f - GetPlayerData()->m_moveBlendRatio);
			pRun->SetBlendAmount(GetPlayerData()->m_moveBlendRatio - 1.0f);
			pWalk->SetBlendDelta(0.0f);
			pRun->SetBlendDelta(0.0f);
			m_eMoveState = PEDMOVE_RUN;
		}
		// if running
		else
		{
			pWalk->SetBlendAmount(0.0f);
			pRun->SetBlendAmount(1.0f);
			pWalk->SetBlendDelta(0.0f);
			pRun->SetBlendDelta(0.0f);
			m_eMoveState = PEDMOVE_RUN;
			
			CStats::UpdateStatsWhenRunning();
		}
	}
	
	// do stuff to increase speed with Adrenaline
	if(GetPlayerData()->m_bAdrenaline)
	{
		if (CTimer::GetTimeInMilliseconds() > GetPlayerData()->m_AdrenalineEndTime && !CCheat::IsCheatActive(CCheat::ADRENALINE_CHEAT))
		{
			GetPlayerData()->m_bAdrenaline = false;
			CTimer::ms_fTimeScale = 1.0f;
			if(pStartWalk)	pStartWalk->SetSpeed(1.0f);
			if(pWalk)		pWalk->SetSpeed(1.0f);
			if(pRun)		pRun->SetSpeed(1.0f);
			if(pSprint)		pSprint->SetSpeed(1.0f);
		}
		else
		{
			CTimer::ms_fTimeScale = 1.0f/3.0f;
			if(pStartWalk)	pStartWalk->SetSpeed(ADRENALINE_BOOST_FACTOR);
			if(pWalk)		pWalk->SetSpeed(ADRENALINE_BOOST_FACTOR);
			if(pRun)		pRun->SetSpeed(ADRENALINE_BOOST_FACTOR);
			if(pSprint)		pSprint->SetSpeed(ADRENALINE_BOOST_FACTOR);
		}
	}
	if(pSprint)
	{
		if(TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_FIXED
#ifndef FINALBUILD		
			|| TheCamera.WorldViewerBeingUsed
#endif
		)
			pSprint->SetSpeed(0.7f);
		else	
		{
			pSprint->SetSpeed(CMaths::Max(1.0f, GetButtonSprintResults(SPRINT_ON_FOOT)));
/*	
			// set speed of anim depending on the counter modified by the speed the pad X button is tapped 
			if (GetPedIntelligence()->GetTaskDefault() && GetPedIntelligence()->GetTaskDefault()->GetTaskType()==CTaskTypes::TASK_SIMPLE_PLAYER_ON_FOOT)
			{
				float sprintSpeed = ((CTaskSimplePlayerOnFoot *)GetPedIntelligence()->GetTaskDefault())->m_fControlSprint;
		
				if (sprintSpeed < CTaskSimplePlayerOnFoot::SPRINT_THRESHOLD)
				{
					pSprint->SetSpeed(1.0f);
				}
				else
				{
					sprintSpeed -= CTaskSimplePlayerOnFoot::SPRINT_THRESHOLD;
					sprintSpeed /= CTaskSimplePlayerOnFoot::SPRINT_MAX - CTaskSimplePlayerOnFoot::SPRINT_THRESHOLD;

					pSprint->SetSpeed(1.0f + sprintSpeed*0.3f);
				}
			}
*/
		}
	}
/*
	if(m_motionAnimGroup==ANIM_SKATEBOARD_PED)
	{
		if(m_eMoveState >= PEDMOVE_WALK && pRunStop1==NULL)
		{
			if(m_ik.GetSlopeAngle() > 0.0f)
				GetPlayerData()->m_fSkateBoardSpeed += m_ik.GetSlopeAngle()*PLAYER_SKATEBOARD_DOWNHILL_MULT*CTimer::GetTimeStep();
			else
				GetPlayerData()->m_fSkateBoardSpeed += m_ik.GetSlopeAngle()*PLAYER_SKATEBOARD_UPHILL_MULT*CTimer::GetTimeStep();

			if(GetPlayerData()->m_fSkateBoardSpeed < 0.0f)
				GetPlayerData()->m_fSkateBoardSpeed *= CMaths::Pow(PLAYER_SKATEBOARD_UPHILL_DAMP, CTimer::GetTimeStep());
			else
				GetPlayerData()->m_fSkateBoardSpeed *= CMaths::Pow(PLAYER_SKATEBOARD_DOWNHILL_DAMP, CTimer::GetTimeStep());
		}
		else{
			GetPlayerData()->m_fSkateBoardSpeed *= CMaths::Pow(PLAYER_SKATEBOARD_DOWNHILL_SKIDSTOP, CTimer::GetTimeStep());
		}
		
		// limits
		if(GetPlayerData()->m_fSkateBoardSpeed > PLAYER_SKATEBOARD_MAXSPEED_ABS)			GetPlayerData()->m_fSkateBoardSpeed = PLAYER_SKATEBOARD_MAXSPEED_ABS;
		else if(GetPlayerData()->m_fSkateBoardSpeed < PLAYER_SKATEBOARD_MINSPEED_ABS)		GetPlayerData()->m_fSkateBoardSpeed = PLAYER_SKATEBOARD_MINSPEED_ABS;
		
		if(pRun && pRun->GetBlendAmount() > 0.5f)
		{
			pRun->SetSpeed(1.0f + GetPlayerData()->m_fSkateBoardSpeed);
		}
		else if(pWalk && pWalk->GetBlendAmount() > 0.5f)
		{
			pWalk->SetSpeed(1.0f + GetPlayerData()->m_fSkateBoardSpeed - PLAYER_SKATEBOARD_MINSPEED_RUN);
		}
	}
	else
		GetPlayerData()->m_fSkateBoardSpeed = 0.0f;
*/
}


float CPlayerPed::DoWeaponSmoothSpray()
{
/*
	if((GetPedIntelligence()->GetTaskSecondaryWeaponRanged())&&(!m_pEntLockOnTarget))
	{
		CWeaponInfo* pWeaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType());
		switch(GetWeapon()->GetWeaponType())
		{
			case WEAPONTYPE_MICRO_UZI:
			case WEAPONTYPE_MP5:
				return WEAPON_SPRAY_LARGE_DELTA;
				break;
				
//			case WEAPONTYPE_PISTOL:
//			case WEAPONTYPE_TEC9:
//			case WEAPONTYPE_SILENCED_INGRAM:
//				return WEAPON_SPRAY_FLAME_DELTA;
//				break;

			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_SPAS12_SHOTGUN:
//			case WEAPONTYPE_STUBBY_SHOTGUN:
				return WEAPON_SPRAY_LARGE_DELTA;
				break;
				
			case WEAPONTYPE_DESERT_EAGLE:
				return WEAPON_SPRAY_LARGE_DELTA;
				break;
				
			case WEAPONTYPE_M4:
			case WEAPONTYPE_AK47:
				return WEAPON_SPRAY_LARGE_DELTA;
				break;
				
			case WEAPONTYPE_MINIGUN:
//			case WEAPONTYPE_M60:
//			case WEAPONTYPE_HELICANNON:
				return WEAPON_SPRAY_SMALL_DELTA;
				break;
				
			case WEAPONTYPE_FLAMETHROWER:
				return WEAPON_SPRAY_FLAME_DELTA;
				break;
				
			case WEAPONTYPE_CHAINSAW:
				if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_MELEE_RUNNING))
					return WEAPON_SPRAY_CHAINSAWRUN_DELTA;
				else if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_MELEE_GROUND))
					return WEAPON_SPRAY_SMALL_DELTA;
				else
					return WEAPON_SPRAY_FLAME_DELTA;
				break;
			
			case WEAPONTYPE_BASEBALLBAT:
			case WEAPONTYPE_GOLFCLUB:
			case WEAPONTYPE_NIGHTSTICK:
				if(RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_MELEE_GROUND))
					return WEAPON_SPRAY_SMALL_DELTA;
				else
					return -1.0f;
				break;
		}
	}
	else if(m_nPedFlags.bIsDucking)
		return WEAPON_SPRAY_LARGE_DELTA;
*/
	return -1.0f;	
}

// Name			:	PlayerControlZelda
// Purpose		:	Player control routines, Zelda style
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing
/*
void CPlayerPed::PlayerControlZelda(CPad* pPad)
{
	float fStickX, fStickY, fStickAngle;
	float moveBlendRatio;
	float fSmoothWeapon = DoWeaponSmoothSpray();
	bool bTurn = false;
	float CamOrient=TheCamera.Orientation;


	DEBUGPEDVISION(DebugDrawVisionRangeSimple(GetPosition(), RADTODEG(GetPedHeading()));)
	DEBUGPEDLOOK(DebugDrawLook();)

	fStickX = pPad->GetPedWalkLeftRight();
	fStickY = pPad->GetPedWalkUpDown();
	
		// For the 2 handed weapons (the ones that track the target) we disallow
		// the players movement. This would break the targetting (very annoying)
	if (MovementDisabledBecauseOfTargeting())
	{
		fStickX = fStickY = 0.0f;
	}
	
	CVector2D vec2DStickVector = CVector2D(fStickX, fStickY);
	// all this is so that you can't double back when running and firing a handgun
	// camera should have automatically moved behind the player since they started firing
	if( fSmoothWeapon > 0.0f && fStickY > 0.0f)
	{
		vec2DStickVector.x = 0;
		vec2DStickVector.y = 0;
		moveBlendRatio = 0.0f;
		bTurn = true;
	}
	else
		moveBlendRatio = vec2DStickVector.Magnitude() / 60.0f;
	
	if((moveBlendRatio > 0.0f)||(bTurn))
	{
		fStickAngle = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -fStickX, fStickY);
		fStickAngle = fStickAngle - CamOrient;
		fStickAngle = CGeneral::LimitRadianAngle(fStickAngle);

		// if weapon type requires precision control spray
		if(fSmoothWeapon > 0.0f)
		{
			m_fDesiredHeading = m_fCurrentHeading - (fStickX/128.0f) * fSmoothWeapon * CTimer::GetTimeStep();
		}
		else
		{
			m_fDesiredHeading = fStickAngle;
		}
		//if (CPad::GetPad(0)->GetLookBehind()){CamOrient+=PI;}//for occasions when the player is looking behind
									                        //if we don't do this then the player will run backwards.
		//moveBlendRatio += 0.5f;
		// Don't accelerate too fast
		float maxAccel = 0.07f * CTimer::GetTimeStep();
		if(moveBlendRatio - m_moveBlendRatio > maxAccel)
			m_moveBlendRatio += maxAccel;
		else
			m_moveBlendRatio = moveBlendRatio;
	}
	else
		m_moveBlendRatio = 0.0f;

	// special case - want to use directional stuff but not change dirn or apply anim
	if(GetPedState() == PED_JUMP)
	{
		if( m_nPedFlags.bIsInTheAir ){
			if( GetUsesCollision() && !m_nPedFlags.bHitSteepSlope && (!m_nPedFlags.bHitSomethingLastFrame || m_vecDamageNormal.z>0.6f) &&
				GetDistanceTravelled()<0.02*CTimer::GetTimeStep() && m_vecMoveSpeed.MagnitudeSqr()<0.1f*0.1f)
			{
				CVector vecPush;
				vecPush.x = 3*ANGLE_TO_VECTOR_X(RADTODEG(m_fCurrentHeading));
				vecPush.y = 3*ANGLE_TO_VECTOR_Y(RADTODEG(m_fCurrentHeading));
				vecPush.z = 0.05f;
				ApplyMoveForce(vecPush);
			}
			//return;	// always return if pedstate == jump and are the air
		}
		else if(m_nPedFlags.bIsLanding)
			m_moveBlendRatio = 0.0f;
	}
	
	if(GetPedState() == PED_ANSWER_MOBILE)
	{
		SetRealMoveAnim();
		return;
	}
		
	// Has sprint button been pressed
	if(!CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_HEAVY) &&
	pPad->GetSprint() && !(m_pGroundPhysical && m_pGroundPhysical->InfiniteMass && !m_pGroundPhysical->InfiniteMassFixed))
	{
		m_eMoveState = PEDMOVE_SPRINT;
	}	

	if(GetPedState() != PED_FIGHT)	
		SetRealMoveAnim();

	// Has jump button been pressed
	if (!m_nPedFlags.bIsInTheAir &&
		!CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_HEAVY) &&
		pPad->JumpJustDown() &&
		GetPedState() != PED_JUMP)
	{
		ClearAttack();
		ClearWeaponTarget();
		
		if(m_nCarDangerCounter>0 && m_pDangerCar){
			SetEvasiveDive((CPhysical *)m_pDangerCar, true);
			m_nCarDangerCounter=0;
			m_pDangerCar = NULL;
		}
		else
			SetJump();
	}
	
	// play idles
	PlayIdleAnimations(pPad);
}
*/

#define NUM_IDLES_ANIMS						5
#define TIME_BEFORE_FIRST_IDLE_IS_PLAYED	30000
#define TIME_BETWEEN_IDLES					25000
//
// name:		PlayIdleAnimations
// description:	Play idle animations if the player hasn't been doing anything for a while
/*
void CPlayerPed::PlayIdleAnimations(CPad* pPad)
{
	CAnimBlendAssociation* pAnim;
	
	IFWEAREINNETWORKGAME(return;)

	if(TheCamera.m_WideScreenOn || m_nPedFlags.bIsDucking)
		return;
		
	const int32 aIdleAnims[NUM_IDLES_ANIMS][2] = {
		ANIM_PLAYER_IDLE1, ANIM_PLAYER_IDLES_PED, 
		ANIM_PLAYER_IDLE2, ANIM_PLAYER_IDLES_PED, 
		ANIM_PLAYER_IDLE3, ANIM_PLAYER_IDLES_PED, 
		ANIM_PLAYER_IDLE4, ANIM_PLAYER_IDLES_PED, 
//		ANIM_STD_ROADCROSS, ANIM_STD_PED,
		ANIM_STD_XPRESS_SCRATCH, ANIM_STD_PED};
	
	static int32 timeLastIdlePlayed = 0;
	static int32 lastIdlePlayed = -1;
	CAnimBlock* pAnimBlock = CAnimManager::GetAnimationBlock(playerIdlesAnimBlock);
	bool bFoundAnim = false;
	int32 howLongAgo = pPad->InputHowLongAgo();

	if(howLongAgo > TIME_BEFORE_FIRST_IDLE_IS_PLAYED)
	{
		CStreaming::RequestAnimations(playerIdlesAnimBlock, STRFLAG_DONTDELETE);
		if(pAnimBlock->m_loaded)
		{
			// if there is an idle anim playing then set it to blend out and set flag
			pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)GetRwObject());
			while(pAnim)
			{
				CAnimBlendHierarchy* pHier = pAnim->GetAnimHierarchy();
				int32 index = pHier - CAnimManager::GetAnimation(0);
				if(index >= pAnimBlock->m_animIndex && index < pAnimBlock->m_animIndex+pAnimBlock->m_numAnims)
				{
					bFoundAnim = true;
					break;
				}
				pAnim = RpAnimBlendGetNextAssociation(pAnim);
			}
			

			if(!bFoundAnim && !(m_nPedFlags.bIsLooking || m_nPedFlags.bIsRestoringLook))
			{
				
				if(howLongAgo - timeLastIdlePlayed > TIME_BETWEEN_IDLES)
				{
					{
						int32 anim;
						// choose a random idle anim but not the last one that was played
						do{
							anim = CGeneral::GetRandomNumberInRange(0, NUM_IDLES_ANIMS);
						}while(lastIdlePlayed == anim);
						
						pAnim = CAnimManager::BlendAnimation((RpClump*)GetRwObject(), (AssocGroupId)aIdleAnims[anim][1], 
							(AnimationId)aIdleAnims[anim][0], 8.0f);
						pAnim->SetFlag(ABA_PEDFLAG_EXPRESS);	
						timeLastIdlePlayed = howLongAgo;
						lastIdlePlayed = anim;
					}	
				}	
			}
		}
	}
	else if(pAnimBlock->m_loaded)
	{
		// if there is an idle anim playing then set it to blend out and set flag
		pAnim = RpAnimBlendClumpGetFirstAssociation((RpClump*)GetRwObject());
		while(pAnim)
		{
			if(pAnim->IsFlagSet(ABA_PEDFLAG_EXPRESS))
			{
				pAnim->SetBlendDelta(-8.0f);
				bFoundAnim = true;
			}
			pAnim = RpAnimBlendGetNextAssociation(pAnim);
		}
		// only remove animblock if flag isnt set
		if(!bFoundAnim)
		{
			CStreaming::RemoveAnimations(playerIdlesAnimBlock);
		}	
	}
	else
		timeLastIdlePlayed = 0;
}
*/

void CPlayerPed::PlayerControlFighter(CPad* pPad)
{
	/*
	float fStickX, fStickY, fStickAngle;
	float moveBlendRatio;

	fStickX = pPad->GetPedWalkLeftRight();
	fStickY = pPad->GetPedWalkUpDown();
	fStickAngle = CGeneral::GetAngleBetweenPoints(0.0f, 0.0f, -fStickX, fStickY);

	CVector2D vec2DStickVector = CVector2D(fStickX, fStickY);
	
	moveBlendRatio = vec2DStickVector.Magnitude();
	
	if(moveBlendRatio > 0.0f)
	{
		float CamOrient=TheCamera.Orientation;
		m_fDesiredHeading = DEGTORAD(fStickAngle) - CamOrient;
		
		if(moveBlendRatio > 120.0f)
			m_nFightShuffle = 1;
		else
			m_nFightShuffle = 0;

		// is sprint button pressed	
		if(pPad->GetSprint() && moveBlendRatio > 60.0f)
			m_nPedFlags.bIsContinuingAttack = false;
	}
	
	// Has jump button been pressed
	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_HEAVY) &&
		pPad->JumpJustDown())
	{
		if(m_nCarDangerCounter>0 && m_pDangerCar){
			SetEvasiveDive((CPhysical *)m_pDangerCar, true);
			m_nCarDangerCounter=0;
			m_pDangerCar = NULL;
		}
		else
			SetJump();
	}
	*/
}

/*
// Name			:	PlayerControl1stPersonRunAround
// Purpose		:	Player control routines, M16 Mode on Pc.
//				:	Mean you can run around.
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing


void CPlayerPed::PlayerControl1stPersonRunAround(CPad* pPad)
{
//	TheCamera.PlayerExhaustion = 0.1f + 0.9f * (1.0f - (m_fSprintEnergy - MIN_SPRINT_ENERGY) / (MAX_SPRINT_ENERGY - MIN_SPRINT_ENERGY));

	float fStickX=0.0f, fStickY=0.0f, fStickAngle=0.0f;
	float moveBlendRatio;
	float CamOrient=TheCamera.Orientation;

	DEBUGPEDVISION(DebugDrawVisionRangeSimple(GetPosition(), RADTODEG(GetPedHeading()));)
	DEBUGPEDLOOK(DebugDrawLook();)

	fStickX = pPad->GetPedWalkLeftRight();
	fStickY = pPad->GetPedWalkUpDown();
		
	CVector2D vec2DStickVector = CVector2D(fStickX, fStickY);
	// all this is so that you can't double back when running and firing a handgun
	// camera should have automatically moved behind the player since they started firing
	moveBlendRatio = vec2DStickVector.Magnitude() / 60.0f;
		
	if (moveBlendRatio > 0.0f)
	{
		fStickAngle = CamOrient;
		fStickAngle = CGeneral::LimitRadianAngle(fStickAngle);
		m_fDesiredHeading = fStickAngle;
		float maxAccel = 0.07f * CTimer::GetTimeStep();
		if(moveBlendRatio - m_moveBlendRatio > maxAccel)
			m_moveBlendRatio += maxAccel;
		else
			m_moveBlendRatio = moveBlendRatio;
	}
	else
		m_moveBlendRatio = 0.0f;

	// special case - want to use directional stuff but not change dirn or apply anim
	if(GetPedState() == PED_JUMP)
	{
		if( m_nPedFlags.bIsInTheAir ){
			if( GetUsesCollision() && !m_nPedFlags.bHitSteepSlope && (!m_nPedFlags.bHitSomethingLastFrame || m_vecDamageNormal.z>0.6f) &&
				GetDistanceTravelled()<0.02*CTimer::GetTimeStep() && m_vecMoveSpeed.MagnitudeSqr()<0.1f*0.1f)
			{
				CVector vecPush;
				vecPush.x = 3*ANGLE_TO_VECTOR_X(RADTODEG(m_fCurrentHeading));
				vecPush.y = 3*ANGLE_TO_VECTOR_Y(RADTODEG(m_fCurrentHeading));
				vecPush.z = 0.05f;
				ApplyMoveForce(vecPush);
			}
			//return;	// always return if pedstate == jump and are the air
		}
		else if(m_nPedFlags.bIsLanding)
			m_moveBlendRatio = 0.0f;
	}
	
	if(GetPedState() == PED_ANSWER_MOBILE)
	{
		SetRealMoveAnim();
		return;
	}
			
	// Has sprint button been pressed
	if(!CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_HEAVY) &&
		pPad->GetSprint())
	{
		if(!m_bPlayerSprintDisabled)
		{
			m_eMoveState = PEDMOVE_SPRINT;
		}
		else
		{
			m_eMoveState = PEDMOVE_RUN;
		}
	}	

	if(GetPedState() != PED_FIGHT)	
		SetRealMoveAnim();

	// Has jump button been pressed
	if (!m_nPedFlags.bIsInTheAir && 
		!CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_HEAVY) &&
		pPad->JumpJustDown() &&
		GetPedState() != PED_JUMP)
	{
		ClearAttack();
		ClearWeaponTarget();

		if(m_nCarDangerCounter>0 && m_pDangerCar){
			SetEvasiveDive((CPhysical *)m_pDangerCar, true);
			m_nCarDangerCounter=0;
			m_pDangerCar = NULL;
		}
		else
			SetJump();
	}
//	
//	// try and rotate the ped's body to follow the looking up/down of the camera
//	LimbOrientation tempTorsoOrientation;
//	tempTorsoOrientation.yaw = 0.0f;
//	tempTorsoOrientation.pitch = -1.0f*MIN(QUARTER_PI, MAX(-QUARTER_PI, TheCamera.Cams[0].Alpha));
//	m_ik.RotateTorso(m_aPedFrames[PED_TORSO], tempTorsoOrientation);
//
//	
//	// is weapon button pressed
//	if (pPad->GetWeapon())
//	{
//		CVector offset(0.0f,0.0f,0.6f);
//		offset = GetMatrix() * offset;
//		GetWeapon()->Fire(this, &offset);
//	}	
//
//	GetWeapon()->Update(AudioHandle);
//
}
*/


/*
// Name			:	PlayerControlSniper
// Purpose		:	Player control routines, sniper mode
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing
//
void CPlayerPed::PlayerControlSniper(CPad* pPad)
{
	// NOTE: all this might well have to change to get weapon firing anims working for network games ok!
	bool bExitSniperMode = false;
	ProcessWeaponSwitch(pPad);
	TheCamera.PlayerExhaustion = 0.1f + 0.9f * (1.0f - (m_fSprintEnergy - MIN_SPRINT_ENERGY) / (MAX_SPRINT_ENERGY - MIN_SPRINT_ENERGY));
	
	if(pPad->DuckJustDown() && m_nPedFlags.bIsDucking==false && m_eMoveState != PEDMOVE_SPRINT)
	{
		m_nPedFlags.bCrouchWhenShooting = true;
		SetDuck(60000, true);
	}
	else if(m_nPedFlags.bIsDucking && (pPad->DuckJustDown() || m_eMoveState == PEDMOVE_SPRINT))
	{
		ClearDuck(true);
		m_nPedFlags.bCrouchWhenShooting = false;
	}

#ifdef PLAYER_1ST_PERSON_USEANIMS
	// Has target button been released
	if(!pPad->GetTarget())
	{
		ClearPointGunAt();

		TheCamera.ClearPlayerWeaponMode();
		bExitSniperMode = true;
	}
#else
	// Has target button been released
	if(!pPad->GetTarget() && m_pAttachToEntity==NULL)
	{
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
		return;
	}

	int32 fReverbTimer = (1000*8)/30;
	if(GetWeapon()->GetWeaponType()==WEAPONTYPE_LASERSCOPE)
		fReverbTimer = (1000*10)/30;

	// has weapon just been pressed
	if(pPad->WeaponJustDown() && CTimer::GetTimeInMilliseconds() > GetWeapon()->m_nTimer)
	{
		CVector offset(0.0f,0.0f,0.6f);
		offset = GetMatrix() * offset;
		GetWeapon()->Fire(this, &offset, NULL, NULL, NULL);
		m_LastTimeFiring = CTimer::GetTimeInMilliseconds();
	}
	else if(CTimer::GetTimeInMilliseconds() > m_LastTimeFiring+fReverbTimer
	&& CTimer::GetTimeInMilliseconds() - CTimer::GetTimeElapsedInMilliseconds() < m_LastTimeFiring+fReverbTimer)
	{
		// only want sound if we have any ammo...
		if(pPad->GetWeapon() && GetWeapon()->m_nAmmoTotal>0) DMAudio.PlayOneShot(AudioHandle, AE_WEAPON_REVERB_END, (float)(GetWeapon()->GetWeaponType()));
	}
	
	GetWeapon()->Update(AudioHandle);
#endif
}
*/

/*
// Name			:	PlayerControlM16
// Purpose		:	Player control routines, M16 1st person mode
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing

void CPlayerPed::PlayerControlM16(CPad* pPad)
{
	ProcessWeaponSwitch(pPad);
	TheCamera.PlayerExhaustion = 0.1f + 0.9f * (1.0f - (m_fSprintEnergy - MIN_SPRINT_ENERGY) / (MAX_SPRINT_ENERGY - MIN_SPRINT_ENERGY));

	if(pPad->DuckJustDown() && m_nPedFlags.bIsDucking==false && m_eMoveState != PEDMOVE_SPRINT)
	{
		m_nPedFlags.bCrouchWhenShooting = true;
		SetDuck(60000, true);
	}
	else if(m_nPedFlags.bIsDucking && (pPad->DuckJustDown() || m_eMoveState == PEDMOVE_SPRINT))
	{
		ClearDuck(true);
		m_nPedFlags.bCrouchWhenShooting = false;
	}

	// Has exit mode button been pressed
	if(!pPad->GetTarget() && m_pAttachToEntity==NULL)
	{
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}

	// is weapon button pressed
	if(pPad->GetWeapon() && CTimer::GetTimeInMilliseconds() > GetWeapon()->m_nTimer)
	{
		if(GetWeapon()->m_eState==WEAPONSTATE_OUT_OF_AMMO)
		{
			DMAudio.PlayFrontEndSound(AE_SNIPERRIFFLE_FIRE_FAIL, 0.0f);
			GetWeapon()->m_nTimer = CTimer::GetTimeInMilliseconds() + CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetWeaponFiringRate();
		}
		else
		{
			CVector offset(0.0f,0.0f,0.6f);
			offset = GetMatrix() * offset;
			GetWeapon()->Fire(this, &offset, NULL, NULL, NULL);
			m_LastTimeFiring = CTimer::GetTimeInMilliseconds();
		}
	}
	else if(CTimer::GetTimeInMilliseconds() > GetWeapon()->m_nTimer
	&& CTimer::GetTimeInMilliseconds() - CTimer::GetTimeElapsedInMilliseconds() < GetWeapon()->m_nTimer
	&& GetWeapon()->m_eState!=WEAPONSTATE_OUT_OF_AMMO)
	{
		DMAudio.PlayOneShot(AudioHandle, AE_WEAPON_REVERB_END, (float)(GetWeapon()->GetWeaponType()));
	}

	GetWeapon()->Update(AudioHandle);
}
*/




// Name			:	PlayerControlRocketLauncher
// Purpose		:	Player control routines, rocket launcher mode
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing

/*void CPlayerPed::PlayerControlRocketLauncher(CPad* pPad)
{
	ProcessWeaponSwitch(pPad);

	RestoreSprintEnergy();
	TheCamera.PlayerExhaustion = 0.25f;

	if (pPad->WeaponJustDown())//GetButtonCross())
	{
		CVector		ShotOrigin;
		ShotOrigin = TheCamera.GetPosition();
		GetWeapon()->Fire(this, &ShotOrigin);
	}

	GetWeapon()->Update();
} // end - CPlayerPed::PlayerControlRocketLauncher
*/

//
// name:		DoesPlayerWantNewWeapon
// dscription:	Does the player want this weapon
bool CPlayerPed::DoesPlayerWantNewWeapon(eWeaponType newWeapon, bool bInCar)
{
	CPad* pPad = GetPadFromPlayer();
	int32 slot = CWeaponInfo::GetWeaponInfo(newWeapon)->m_nWeaponSlot;
	eWeaponType playersWeapon = m_WeaponSlots[slot].m_eWeaponType;
	
	// if already have the same weapon or don't have a weapon in that slot can pick it up
	if(playersWeapon == newWeapon || playersWeapon == WEAPONTYPE_UNARMED)
		return true;
	
	// don't give weapon if in car
	if(bInCar)
		return false;
	
	// for now don't let the player pick up a weapon while using the jetpack
	// that you can't fire while using the jetpack, replacing one that you can!
	// (if that doesn't make any sense speak to me (sandy))
	if(GetPedIntelligence()->GetTaskJetPack() && m_nCurrentWeapon==slot
	&& CWeaponInfo::GetWeaponInfo(playersWeapon, GetWeaponSkill(playersWeapon))->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM)
	&& !CWeaponInfo::GetWeaponInfo(playersWeapon, GetWeaponSkill(playersWeapon))->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
	{
		return false;
	}
	
	// if player is moving while in pickup and already have a weapon in that slot then can't pick weapon up
	//if(pPad && (pPad->GetPedWalkLeftRight() != 0.0f || pPad->GetPedWalkUpDown() != 0.0f))
	//	return false;
		
	// if attacking and using weapon from same slot then can't pick up weapon	
	if((GetPedState() == PED_ATTACK || GetPedState() == PED_AIMGUN) &&
		slot == m_nCurrentWeapon)
		return false;
		
	return true;	
}

// Name			:	ProcessPlayerWeapon
// Purpose		:	Processes control for player ped weapons
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing
int nTestPlayerWeaponAccuracy = 95;
float fTestPlayerWeaponReAimRate = 0.94f;
//
void CPlayerPed::ProcessPlayerWeapon(CPad* pPad)
{

} // end - CPlayerPed::ProcessPlayerWeapon


// Name			:	ProcessWeaponSwitch
// Purpose		:	Sorts out switching between weapons
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing

void CPlayerPed::ProcessWeaponSwitch(CPad* pPad)
{
	if(!CDarkel::FrenzyOnGoing() && m_pAttachToEntity==NULL
	&& !GetPedIntelligence()->GetTaskJetPack())
	{
		if(!GetWeaponLockOn() && !GetPlayerData()->m_bFreeAiming &&
			!GetPlayerData()->m_bDontAllowWeaponChange &&
			!GetPlayerData()->m_bInVehicleDontAllowWeaponChange)// && GetWeapon()->GetWeaponType() != WEAPONTYPE_DETONATOR)
		{	
			if (pPad->CycleWeaponRightJustDown())
			{
				if( TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_M16_1STPERSON && TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_M16_1STPERSON_RUNABOUT &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_SNIPER && TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_SNIPER_RUNABOUT &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_ROCKETLAUNCHER && TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_ROCKETLAUNCHER_RUNABOUT &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_ROCKETLAUNCHER_HS && TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_CAMERA)
				{
					GetPlayerData()->m_nChosenWeapon = m_nCurrentWeapon+1;
					while(true)
					{
						if(GetPlayerData()->m_nChosenWeapon >= PED_MAX_WEAPON_SLOTS){
							GetPlayerData()->m_nChosenWeapon = 0; // deals with unarmed
							break;
						}
						// otherwise, any weapon that is not unarmed is a used slot
						if((m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].m_eWeaponType != WEAPONTYPE_UNARMED &&
							m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].HasWeaponAmmoToBeUsed()) &&
						   (!CGameLogic::IsCoopGameGoingOn() || m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].CanBeUsedFor2Player()) )
							break;

						GetPlayerData()->m_nChosenWeapon++;
					}
				}
			}
			// choose previous weapon in list
			else if (pPad->CycleWeaponLeftJustDown())
			{
				if( TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_M16_1STPERSON &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_SNIPER &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_ROCKETLAUNCHER &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_ROCKETLAUNCHER_HS &&
					TheCamera.PlayerWeaponMode.Mode!=CCam::MODE_CAMERA)
				{
					GetPlayerData()->m_nChosenWeapon = m_nCurrentWeapon-1;
					while(true)
					{
						if(GetPlayerData()->m_nChosenWeapon < 0){
							GetPlayerData()->m_nChosenWeapon = PED_MAX_WEAPON_SLOTS - 1;
						}
						
						if(GetPlayerData()->m_nChosenWeapon == 0)
							break; // deals with unarmed
						
						if((m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].m_eWeaponType != WEAPONTYPE_UNARMED &&
							m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].HasWeaponAmmoToBeUsed()) &&
						   (!CGameLogic::IsCoopGameGoingOn() || m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].CanBeUsedFor2Player()) )
							break;

						GetPlayerData()->m_nChosenWeapon--;

//						if(GetPlayerData()->m_nChosenWeapon == 0)
//							break;
						
					}
				}
			}
		}	
		if(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetWeaponFireType() != FIRETYPE_MELEE
		&& (!pPad->GetWeapon(this) || GetWeapon()->GetWeaponType()!=WEAPONTYPE_MINIGUN)
		&& GetWeapon()->m_nAmmoTotal<=0)
		{
			if( TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON || 
				TheCamera.PlayerWeaponMode.Mode == CCam::MODE_SNIPER ||
				TheCamera.PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER ||
				TheCamera.PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER_HS)
//				TheCamera.PlayerWeaponMode.Mode == CCam::MODE_CAMERA)
			{
				// Don't change mode while we're in 1st person mode.
				// Only applicable for the PS2 that doesn't allow you to run around in 1st person 
				return;		
			}

			if(GetWeapon()->GetWeaponType() == WEAPONTYPE_DETONATOR &&
				m_WeaponSlots[WEAPONSLOT_TYPE_THROWN].m_eWeaponType == WEAPONTYPE_REMOTE_SATCHEL_CHARGE)
				GetPlayerData()->m_nChosenWeapon = WEAPONSLOT_TYPE_THROWN;
			else
				GetPlayerData()->m_nChosenWeapon = m_nCurrentWeapon-1;
			while(true)
			{
				if(GetPlayerData()->m_nChosenWeapon < 0){
					GetPlayerData()->m_nChosenWeapon = 0;
					break;
				}
				else if(GetPlayerData()->m_nChosenWeapon == WEAPONTYPE_BASEBALLBAT && m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].m_eWeaponType == eWeaponType(GetPlayerData()->m_nChosenWeapon))
					break;
				
				if(m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].m_nAmmoTotal > 0 
				&& GetPlayerData()->m_nChosenWeapon!=WEAPONTYPE_MOLOTOV 
				&& GetPlayerData()->m_nChosenWeapon!=WEAPONTYPE_TEARGAS 
				&& GetPlayerData()->m_nChosenWeapon!=WEAPONTYPE_GRENADE)
					break;
					
				GetPlayerData()->m_nChosenWeapon--;
			}		
		}
	}	
	// If chosen weapon has changed and not attacking then change weapons
	if(GetPlayerData()->m_nChosenWeapon != m_nCurrentWeapon
	&& !(GetPedIntelligence()->GetTaskUseGun() && (GetPedIntelligence()->GetTaskUseGun()->GetIsFiring() || GetPedIntelligence()->GetTaskUseGun()->GetIsReloading())))
	{
		RemoveWeaponAnims(m_nCurrentWeapon);
		MakeChangesForNewWeapon(GetPlayerData()->m_nChosenWeapon);
	}
	
} // end - CPlayerPed::ProcessWeaponSwitch


// Name			:	PickWeaponAllowedFor2Player
// Purpose		:	If the player happens to have a weapon he isn't allowed during 2 player games we pick unarmed
// Parameters	:
// Returns		:	Nothing

void CPlayerPed::PickWeaponAllowedFor2Player()
{
	if (!m_WeaponSlots[GetPlayerData()->m_nChosenWeapon].CanBeUsedFor2Player())
	{
		GetPlayerData()->m_nChosenWeapon = 0;
	}
}


void  CPlayerPed::UpdateCameraWeaponModes(CPad *pPad)
{
	unsigned int CurrentPlayerWeapon=0; 
	CurrentPlayerWeapon=GetWeapon()->GetWeaponType();

	switch (GetWeapon()->GetWeaponType())
	{
		case WEAPONTYPE_ROCKETLAUNCHER:
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_ROCKETLAUNCHER, 0, 0);
			break;
		case WEAPONTYPE_ROCKETLAUNCHER_HS:
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_ROCKETLAUNCHER_HS, 0, 0);
			break;
//		case WEAPONTYPE_LASERSCOPE:
		case WEAPONTYPE_SNIPERRIFLE:
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_SNIPER, 0, 0);
			break;
		case WEAPONTYPE_M4:
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_M16_1STPERSON, 0, 0);
			break;
		case WEAPONTYPE_CAMERA:
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_CAMERA, 0, 0);
			break;
//		case WEAPONTYPE_HELICANNON:
//			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_HELICANNON_1STPERSON, 0, 0);
//			break;
		default:
//			RestorePreviousState(); Obr/Sandy removed this (Broke grenade throwing)
			TheCamera.ClearPlayerWeaponMode();
			break;
	}
}


// Name			:	MakeChangesForNewWeapon
// Purpose		:	Makes the necessary changes for a new weapon beging selected.
// Parameters	:	pPad - ptr to control pad
// Returns		:	Nothing

void CPlayerPed::MakeChangesForNewWeapon(int weaponSlot)
{
	if(weaponSlot != -1)
	{
		MakeChangesForNewWeapon(m_WeaponSlots[weaponSlot].m_eWeaponType);
	}
}

void CPlayerPed::MakeChangesForNewWeapon(eWeaponType NewWeaponType)
{
	// stop any effect on the current weapon
	CWeapon* pCurrWeapon = GetWeapon();
	if (pCurrWeapon)
	{
		pCurrWeapon->StopWeaponEffect();
	}


	if(GetPedState()==PED_SNIPER_MODE)
	{
		//RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}

	SetCurrentWeapon(NewWeaponType);
	GetPlayerData()->m_nChosenWeapon = m_nCurrentWeapon;
	GetPlayerData()->m_fAttackButtonCounter = 0;
	
	CWeaponInfo *pWeaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill());

	// Make sure we've got maximum ammo in clip
	GetWeapon()->m_nAmmoInClip = MIN(GetWeapon()->m_nAmmoTotal, pWeaponInfo->GetWeaponAmmo());

	if (!pWeaponInfo->IsWeaponFlagSet(WEAPONTYPE_CANAIM))
	{
		ClearWeaponTarget();
	}
	if (!pWeaponInfo->IsWeaponFlagSet(WEAPONTYPE_CANFREEAIM))
	{
		GetPlayerData()->m_bFreeAiming = false;
	}

/*	// ignore the next comments - decided to do it later all the time incase of switching control modes

	// only want to do AnimationGroups stuff if we're not using 3rdperson mouse control
	// if we are then it will be done every frame anyway - elsewhere (after weapon processing)
	if( !((TheCamera.Cams[TheCamera.ActiveCam].GetWeaponFirstPersonOn() || TheCamera.Cams[0].Using3rdPersonMouseCam())
		&& CanStrafeOrMouseControl()) )
	{	
		// If rocket launcher has been chosen
		if(GetWeapon()->GetWeaponType() == WEAPONTYPE_ROCKET)
		{
			m_motionAnimGroup = ANIM_PLAYER_ROCKET_PED;
			ReApplyMoveAnims();
		}
		else if(GetWeapon()->GetWeaponType() == WEAPONTYPE_BASEBALLBAT)
		{
			m_motionAnimGroup = ANIM_PLAYER_BBBAT_PED;
			ReApplyMoveAnims();
		}
		else if(GetWeapon()->GetWeaponType()==WEAPONTYPE_PISTOL ||
				GetWeapon()->GetWeaponType()==WEAPONTYPE_UZI)
		{
			if(m_motionAnimGroup != ANIM_PLAYER_1ARMED_PED){
				m_motionAnimGroup = ANIM_PLAYER_1ARMED_PED;
				ReApplyMoveAnims();
			}
		}
		else if(GetWeapon()->IsType2Handed())
		{
			if(m_motionAnimGroup != ANIM_PLAYER_2ARMED_PED){
				m_motionAnimGroup = ANIM_PLAYER_2ARMED_PED;
				ReApplyMoveAnims();
			}
		}
		else if(m_motionAnimGroup != ANIM_PLAYER_PED)
		{
			m_motionAnimGroup = ANIM_PLAYER_PED;
			ReApplyMoveAnims();
		}
	}
*/	

	CAnimBlendAssociation *pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_WEAPON_ATTACK);
	if(pAnim)
	{
		pAnim->SetFlag(ABA_FLAG_ISPLAYING);
		pAnim->SetFlag(ABA_FLAG_ISFINISHAUTOREMOVE);
	}
	
	// Otherwise clear weapon camera mode and set idle
	TheCamera.ClearPlayerWeaponMode();
	
//	UpdateCameraWeaponModes();
	
//		SetIdle();
}

//
// name:		ProcessAnimGroups
// description: Apply anim motion group if it has changed
//
void CPlayerPed::ProcessAnimGroups()
{
	AssocGroupId nGroupChoice = ANIM_STD_PED;
	AssocGroupId requiredMotionGroup = m_motionAnimGroup;
	bool bMotionHasFatAnims = true;
	
	// using std running anims
	if((GetPlayerData()->m_fFPSMoveHeading > DEGTORAD(-50.0f) && GetPlayerData()->m_fFPSMoveHeading < DEGTORAD(50.0f))
	|| TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam() == false || !CanStrafeOrMouseControl())
	{
//		int32 weaponType = GetWeapon()->GetWeaponType();
// Instead of using the current weapon use the current weapon model
		int32 weaponType = WEAPONTYPE_UNARMED;
		if(m_pWeaponClump)
		{
			CClumpModelInfo* pModelInfo = CVisibilityPlugins::GetClumpModelInfo(m_pWeaponClump);
			if(pModelInfo && pModelInfo->GetModelType() == MI_TYPE_WEAPON)
				weaponType = ((CWeaponModelInfo*)pModelInfo)->GetWeaponInfo();
		}			
		// If rocket launcher has been chosen
		if(weaponType == WEAPONTYPE_ROCKETLAUNCHER || 
			weaponType == WEAPONTYPE_ROCKETLAUNCHER_HS)
		{
			requiredMotionGroup = ANIM_PLAYER_ROCKET_PED;
/*			if(m_motionAnimGroup != ANIM_PLAYER_ROCKET_PED){
				m_motionAnimGroup = ANIM_PLAYER_ROCKET_PED;
				ReApplyMoveAnims();
			}*/
		}
/*
		else if(weaponType==WEAPONTYPE_SKATEBOARD && !m_nPedFlags.bInVehicle)
		{
/*
			if(GetPlayerData()->m_SkateboardAnimsReferenced==ANIM_STD_PED)
			{
				CAnimBlock* pAnimBlock = CAnimManager::GetAnimBlendAssoc(ANIM_SKATEBOARD_PED)->GetAnimBlock();
				if(pAnimBlock==NULL)
					pAnimBlock =  CAnimManager::GetAnimationBlock(CAnimManager::GetAnimBlockName(ANIM_SKATEBOARD_PED));

				int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
				if(pAnimBlock->m_loaded != true)
					CStreaming::RequestAnimations(blockIndex, STRFLAG_FORCE_LOAD);
				else{
					CAnimManager::AddAnimBlockRef(blockIndex);
					GetPlayerData()->m_SkateboardAnimsReferenced = ANIM_SKATEBOARD_PED;
				}
			}
*//*
			CAnimBlendAssociation* pIdle = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_IDLE);

			if(GetMoveState()==PEDMOVE_STILL && pIdle && pIdle->GetBlendAmount() > 0.5f
			&& GetWeaponLockOn())
			{
				requiredMotionGroup = ANIM_PLAYER_PED;
				bMotionHasFatAnims = true;
			}
			else if(m_motionAnimGroup != ANIM_SKATEBOARD_PED && GetMoveState()==PEDMOVE_STILL
			&& pIdle && pIdle->GetBlendAmount() > 0.5f && GetPlayerData()->m_MeleeWeaponAnimReferenced==ANIM_MELEE_SKATEBOARD)
			{
				requiredMotionGroup = ANIM_SKATEBOARD_PED;
				bMotionHasFatAnims = false;
			}
			else if(requiredMotionGroup==ANIM_SKATEBOARD_PED)
				bMotionHasFatAnims = false;
			else
				requiredMotionGroup = ANIM_PLAYER_PED;
		}
*/
		else if(GetPedIntelligence()->GetTaskJetPack())
		{
			requiredMotionGroup = ANIM_PLAYER_JETPACK_PED;
			bMotionHasFatAnims = false;
		}
		else if(weaponType == WEAPONTYPE_BASEBALLBAT 
		     || weaponType == WEAPONTYPE_SHOVEL 
		     || weaponType == WEAPONTYPE_POOL_CUE)
//		||		GetWeapon()->GetWeaponType() == WEAPONTYPE_MACHETE)
		{
			requiredMotionGroup = ANIM_PLAYER_BBBAT_PED;
//			if(m_motionAnimGroup != ANIM_PLAYER_BBBAT_PED){
//				m_motionAnimGroup = ANIM_PLAYER_BBBAT_PED;
//				ReApplyMoveAnims();
//			}
		}
/*		// REMOVED 1ARMED GROUP AS THE ANIMS ARE NOW THE SAME ANYWAY
		else if(weaponType==WEAPONTYPE_PISTOL
		||		weaponType==WEAPONTYPE_PISTOL_SILENCED
		||		weaponType==WEAPONTYPE_DESERT_EAGLE
		||		weaponType==WEAPONTYPE_SAWNOFF_SHOTGUN
		||		weaponType==WEAPONTYPE_TEC9
		||		weaponType==WEAPONTYPE_MICRO_UZI
		||		weaponType==WEAPONTYPE_MP5
		||		weaponType == WEAPONTYPE_GOLFCLUB
		||		weaponType == WEAPONTYPE_KATANA
		||		weaponType == WEAPONTYPE_CAMERA)
		{
			requiredMotionGroup = ANIM_PLAYER_1ARMED_PED;
//			if(m_motionAnimGroup != ANIM_PLAYER_1ARMED_PED){
//				m_motionAnimGroup = ANIM_PLAYER_1ARMED_PED;
//				ReApplyMoveAnims();
//			}
		}
*/
		else if(weaponType==WEAPONTYPE_CHAINSAW
		||		weaponType==WEAPONTYPE_FLAMETHROWER
		||		weaponType==WEAPONTYPE_MINIGUN)
		{
			requiredMotionGroup = ANIM_PLAYER_CHAINSAW_PED;
//			if(m_motionAnimGroup != ANIM_PLAYER_CHAINSAW_PED){
//				m_motionAnimGroup = ANIM_PLAYER_CHAINSAW_PED;
//				ReApplyMoveAnims();
//			}
		}
		else if(weaponType==WEAPONTYPE_M4 || 
				weaponType==WEAPONTYPE_AK47 ||
				weaponType==WEAPONTYPE_SPAS12_SHOTGUN || 
				weaponType==WEAPONTYPE_SHOTGUN ||
				weaponType==WEAPONTYPE_SNIPERRIFLE ||
		 		weaponType==WEAPONTYPE_COUNTRYRIFLE)

		{
			requiredMotionGroup = ANIM_PLAYER_2ARMED_PED;
//			if(m_motionAnimGroup != ANIM_PLAYER_2ARMED_PED){
//				m_motionAnimGroup = ANIM_PLAYER_2ARMED_PED;
//				ReApplyMoveAnims();
//			}
		}
		else if(GetPlayerData()->m_pClothes->GetIsWearingBalaclava())
		{
			requiredMotionGroup = ANIM_PLAYER_SNEAK_PED;
			bMotionHasFatAnims = false;
		}
		else
		{
			requiredMotionGroup = ANIM_PLAYER_PED;
//			AssocGroupId defaultGroup = CClothes::GetDefaultPlayerMotionGroup();
//			if(m_motionAnimGroup != defaultGroup)
//			{
//				m_motionAnimGroup = defaultGroup;
//				ReApplyMoveAnims();
//			}	
		}
	}
//////////////////////////////////////////////
// strafing ANIMS and anim groups are only in the GTA_PC version at the moment
//!PC - strafing removed for PC build just now
//#if defined (GTA_PC)// && defined (GTA_LIBERTY)
#if 0
	// moving backwards
	else if( m_pPlayerData->m_fFPSMoveHeading < DEGTORAD(-130) || m_pPlayerData->m_fFPSMoveHeading > DEGTORAD(130) )
	{
		if(GetWeapon()->GetWeaponType() == WEAPONTYPE_ROCKETLAUNCHER || GetWeapon()->GetWeaponType() == WEAPONTYPE_ROCKETLAUNCHER_HS)
			requiredMotionGroup = ANIM_ROCKET_STRAFEBACK;
		else if(GetWeapon()->GetWeaponType() == WEAPONTYPE_CHAINSAW
		|| GetWeapon()->GetWeaponType() == WEAPONTYPE_FLAMETHROWER
		|| GetWeapon()->GetWeaponType() == WEAPONTYPE_MINIGUN)
			requiredMotionGroup = ANIM_CSAW_STRAFEBACK;
		else
			requiredMotionGroup = ANIM_PLAYER_STRAFEBACK;
		bMotionHasFatAnims = false;	
	}
	// moving erm.. left maybe
	else if( m_pPlayerData->m_fFPSMoveHeading > 0.0f)
	{
		if(GetWeapon()->GetWeaponType() == WEAPONTYPE_ROCKETLAUNCHER || GetWeapon()->GetWeaponType() == WEAPONTYPE_ROCKETLAUNCHER_HS)
			requiredMotionGroup = ANIM_ROCKET_STRAFELEFT;
		else if(GetWeapon()->GetWeaponType() == WEAPONTYPE_CHAINSAW
		|| GetWeapon()->GetWeaponType() == WEAPONTYPE_FLAMETHROWER
		|| GetWeapon()->GetWeaponType() == WEAPONTYPE_MINIGUN)
			requiredMotionGroup = ANIM_CSAW_STRAFELEFT;
		else
			requiredMotionGroup = ANIM_PLAYER_STRAFELEFT;
		bMotionHasFatAnims = false;	
	}
	// and right
	else
	{
		if(GetWeapon()->GetWeaponType() == WEAPONTYPE_ROCKETLAUNCHER || GetWeapon()->GetWeaponType() == WEAPONTYPE_ROCKETLAUNCHER_HS)
			requiredMotionGroup = ANIM_ROCKET_STRAFERIGHT;
		else if(GetWeapon()->GetWeaponType() == WEAPONTYPE_CHAINSAW
		|| GetWeapon()->GetWeaponType() == WEAPONTYPE_FLAMETHROWER
		|| GetWeapon()->GetWeaponType() == WEAPONTYPE_MINIGUN)
			requiredMotionGroup = ANIM_CSAW_STRAFERIGHT;
		else
			requiredMotionGroup = ANIM_PLAYER_STRAFERIGHT;
		bMotionHasFatAnims = false;	
	}
#endif
//////////////////////////////////////////////
	
	// if motion has fat and muscular anims then add on offset to animgroup id
	if(requiredMotionGroup==ANIM_PLAYER_PED && GetPedType()==PEDTYPE_PLAYER2
	&& ((CPedModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->GetMotionAnimGroup()!=ANIM_PLAYER_PED)
	{
		requiredMotionGroup = ((CPedModelInfo *)CModelInfo::GetModelInfo(GetModelIndex()))->GetMotionAnimGroup();
	}
	else if(bMotionHasFatAnims)
	{
		int32 motionGroup = CClothes::GetDefaultPlayerMotionGroup();
		requiredMotionGroup = AssocGroupId(requiredMotionGroup + motionGroup - ANIM_PLAYER_PED);
	}
	// apply new motion anims if they have changed
	if(m_motionAnimGroup != requiredMotionGroup)
	{
		m_motionAnimGroup = requiredMotionGroup;
		ReApplyMoveAnims();
	}
	

	CWeaponInfo *pWeaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill());
	bool bRemoveStdMeleeCombo = false;
	bool bRemoveExtraMeleeCombo = false;
	AssocGroupId nAnimGrp = ANIM_STD_PED;
	if(pWeaponInfo->GetWeaponFireType()==FIRETYPE_MELEE && !m_nPedFlags.bInVehicle)
	{
		if(pWeaponInfo->GetMeleeCombo()!=MCOMBO_UNARMED_1)
		{
			nAnimGrp = CTaskSimpleFight::m_aComboData[pWeaponInfo->GetMeleeCombo() - MCOMBO_UNARMED_1].nAnimGroup;
			if(nAnimGrp != GetPlayerData()->m_MeleeWeaponAnimReferenced)
			{
				if(GetPlayerData()->m_MeleeWeaponAnimReferenced != ANIM_STD_PED)
					bRemoveStdMeleeCombo = true;
			
				CAnimBlock* pAnimBlock = CAnimManager::GetAnimBlendAssoc(nAnimGrp)->GetAnimBlock();
				if(pAnimBlock==NULL)
					pAnimBlock =  CAnimManager::GetAnimationBlock(CAnimManager::GetAnimBlockName(nAnimGrp));

				int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
				if(pAnimBlock->m_loaded != true)
					CStreaming::RequestAnimations(blockIndex, STRFLAG_FORCE_LOAD);
				else if(GetPlayerData()->m_MeleeWeaponAnimReferenced == ANIM_STD_PED){
					CAnimManager::AddAnimBlockRef(blockIndex);
					GetPlayerData()->m_MeleeWeaponAnimReferenced = nAnimGrp;
				}
			}
		}
		else if(GetPlayerData()->m_MeleeWeaponAnimReferenced!=ANIM_STD_PED)
			bRemoveStdMeleeCombo = true;
		
		if(m_nExtraMeleeCombo!=MCOMBO_UNARMED_1)
		{
			nAnimGrp = CTaskSimpleFight::m_aComboData[m_nExtraMeleeCombo - MCOMBO_UNARMED_1].nAnimGroup;
			if(nAnimGrp != GetPlayerData()->m_MeleeWeaponAnimReferencedExtra)
			{
				if(GetPlayerData()->m_MeleeWeaponAnimReferencedExtra != ANIM_STD_PED)
					bRemoveExtraMeleeCombo = true;
			
				CAnimBlock* pAnimBlock = CAnimManager::GetAnimBlendAssoc(nAnimGrp)->GetAnimBlock();
				if(pAnimBlock==NULL)
					pAnimBlock =  CAnimManager::GetAnimationBlock(CAnimManager::GetAnimBlockName(nAnimGrp));

				int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
				if(pAnimBlock->m_loaded != true)
					CStreaming::RequestAnimations(blockIndex, STRFLAG_FORCE_LOAD);
				else if(GetPlayerData()->m_MeleeWeaponAnimReferencedExtra == ANIM_STD_PED){
					CAnimManager::AddAnimBlockRef(blockIndex);
					GetPlayerData()->m_MeleeWeaponAnimReferencedExtra = nAnimGrp;
				}
			}
		
		}
		else if(GetPlayerData()->m_MeleeWeaponAnimReferencedExtra!=ANIM_STD_PED)
			bRemoveExtraMeleeCombo = true;
	}
	else
	{
		bRemoveStdMeleeCombo = true;
		bRemoveExtraMeleeCombo = true;
	}
	
	// de-reference weapon combo if no longer required
	if(bRemoveStdMeleeCombo && GetPlayerData()->m_MeleeWeaponAnimReferenced!=ANIM_STD_PED)
	{
		CAnimBlock *pAnimBlock = CAnimManager::GetAnimBlendAssoc(GetPlayerData()->m_MeleeWeaponAnimReferenced)->GetAnimBlock();
		int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
		CAnimManager::RemoveAnimBlockRef(blockIndex);
		GetPlayerData()->m_MeleeWeaponAnimReferenced = ANIM_STD_PED;
	}
	// de-reference extra melee combo if no longer required
	if(bRemoveExtraMeleeCombo && GetPlayerData()->m_MeleeWeaponAnimReferencedExtra!=ANIM_STD_PED)
	{
		CAnimBlock *pAnimBlock = CAnimManager::GetAnimBlendAssoc(GetPlayerData()->m_MeleeWeaponAnimReferencedExtra)->GetAnimBlock();
		int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
		CAnimManager::RemoveAnimBlockRef(blockIndex);
		GetPlayerData()->m_MeleeWeaponAnimReferencedExtra = ANIM_STD_PED;
	}
/*
	// de-reference skateboard anims if no longer needed
	if(GetPlayerData()->m_SkateboardAnimsReferenced==ANIM_SKATEBOARD_PED && (GetWeapon()->GetWeaponType()!=WEAPONTYPE_SKATEBOARD || m_nPedFlags.bInVehicle))
	{
		CAnimBlock *pAnimBlock = CAnimManager::GetAnimBlendAssoc(ANIM_SKATEBOARD_PED)->GetAnimBlock();
		int32 blockIndex = (pAnimBlock - CAnimManager::GetAnimationBlock(0));
		CAnimManager::RemoveAnimBlockRef(blockIndex);
		GetPlayerData()->m_SkateboardAnimsReferenced = ANIM_STD_PED;
	}
*/	
}
//
//#pragma optimization_level reset

void CPlayerPed::ClearWeaponTarget()
{
	if (GetPedType() == PEDTYPE_PLAYER1 || GetPedType() == PEDTYPE_PLAYER2)
	{
		SetWeaponLockOnTarget(NULL);
		TheCamera.ClearPlayerWeaponMode();
		CWeaponEffects::ClearCrossHair(GetPedType() - PEDTYPE_PLAYER1);
	}
//	ClearPointGunAt();
}

float CPlayerPed::GetWeaponRadiusOnScreen()
{
	eWeaponType weaponType = GetWeapon()->GetWeaponType();
	CWeaponInfo *pWeaponInfo = CWeaponInfo::GetWeaponInfo(weaponType, GetWeaponSkill());
	float fScopeSizeMult = 0.5f / pWeaponInfo->GetAccuracy();
	
	if(pWeaponInfo->GetWeaponFireType()==FIRETYPE_MELEE)
		return 0.0f;
				
	if(weaponType==WEAPONTYPE_SHOTGUN || weaponType==WEAPONTYPE_SAWNOFF_SHOTGUN || weaponType==WEAPONTYPE_SPAS12_SHOTGUN)
	{
		static float SCOPE_SHOTGUN_MULT = 1.0f;
		fScopeSizeMult *= SCOPE_SHOTGUN_MULT;
	}
	else
	{
		static float SCOPE_RANGE_BASE = 15.0f;
		static float SCOPE_BUTTON_COUNTER_MULT = 0.5f;
		fScopeSizeMult *= MIN(1.0f, SCOPE_RANGE_BASE / pWeaponInfo->GetWeaponRange());
		fScopeSizeMult *= 1.0f + SCOPE_BUTTON_COUNTER_MULT*GetPlayerData()->m_fAttackButtonCounter;

		if(m_nPedFlags.bIsDucking)
			fScopeSizeMult *= 0.5f;
	}
	
	// made the crosshair have a bigger min value and max out
	if(fScopeSizeMult < WEAPON_RADIUS_MIN)	fScopeSizeMult = WEAPON_RADIUS_MIN;
//	else if(fScopeSizeMult > WEAPON_RADIUS_MAX)	fScopeSizeMult = WEAPON_RADIUS_MAX;

	return fScopeSizeMult;
}


bool CPlayerPed::PedCanBeTargettedVehicleWise(CPed *pPed)
{
	if (!pPed->m_nPedFlags.bInVehicle) return true;
	
	if (pPed->m_pMyVehicle)
	{
		if (pPed->m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE) return true;
		if (pPed->m_pMyVehicle->m_nVehicleFlags.bVehicleCanBeTargetted) return true;
//		if (!pPed->m_pMyVehicle->CarHasRoof()) return true;	This makes the collision on cars look shit
	}
	
	return false;
}

bool LOSBlockedBetweenPeds(CEntity *pPed1, CEntity *pPed2)
{
	// now process line of sight
	CColPoint	colPoint;
	CEntity*	pHitEntity;
	CVector 	VecPos1, VecPos2;

/*
	CVector VecPos1 = pPed1->GetPosition();
	if (pPed1->GetType() == ENTITY_TYPE_PED) VecPos1.z += 0.7f;

	CVector VecPos2 = pPed2->GetPosition();
	if (pPed2->GetType() == ENTITY_TYPE_PED) VecPos2.z += 0.7f;
*/
	if (pPed1->GetType() == ENTITY_TYPE_PED)
	{
		((CPed*)pPed1)->GetBonePosition(VecPos1, BONETAG_NECK);
		if (((CPed*)pPed1)->m_nPedFlags.bIsDucking)
		{		// Raise point a bit so that player can target people when crouching behind little fences
			VecPos1.z += 0.35f;
		}
	}
	else
	{
		VecPos1 = pPed1->GetPosition();
	}

	if (pPed2->GetType() == ENTITY_TYPE_PED)
	{
		((CPed*)pPed2)->GetBonePosition(VecPos2, BONETAG_NECK);
	}
	else
	{
		VecPos2 = pPed2->GetPosition();
	}

			// check buildings only. Make sure we can target stuff behind bShootThroughStuff.
	INC_PEDAI_LOS_COUNTER;
	bool	bResult = CWorld::ProcessLineOfSight(VecPos1, VecPos2, colPoint, pHitEntity, true, false, false, true, false, false, false, true);

	if (!bResult) return false;				// LOS is clear
	if (pHitEntity == pPed2) return false;	// We hit the second entity. As good as a clear shot.

	return (true);		// We had a genuine collision.
}

// Name			:	FindWeaponLockOnTarget
// Purpose		:	Searches for the nearest available weapon lock on target. If a
//					suitable target is found, lock on mode is set to active and
//					the target is set.
// Parameters	:	None
// Returns		:	TRUE if a target is found, FALSE otherwise

bool CPlayerPed::FindWeaponLockOnTarget()
{
	float fWeaponRange, fPedDistance;
	float TargetValue;
//	CPed* pVisPed = NULL;
	CEntity* pTargetEntity = NULL;
	CVector2D vec2DPedDistance;
	float	OrientationOfPed;
	CWeaponInfo *pWeaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill());
	fWeaponRange = pWeaponInfo->GetTargetRange();

	// check if we're already locked on
	if (GetWeaponLockOn())
	{
		ASSERT(m_pEntLockOnTarget);
		// we are - find out if the target is still valid
		vec2DPedDistance.x = m_pEntLockOnTarget->GetPosition().x - GetPosition().x;
		vec2DPedDistance.y = m_pEntLockOnTarget->GetPosition().y - GetPosition().y;
		fPedDistance = vec2DPedDistance.Magnitude();

		// check if ped is in this weapon's range
		if (fPedDistance <= pWeaponInfo->GetTargetRange() * CWeapon::TargetWeaponRangeMultiplier(m_pEntLockOnTarget, this))
		{
			return TRUE;
		}
		else
		{
			SetWeaponLockOnTarget(NULL);
			return FALSE;
		}
	}

	pTargetEntity = 0;
	TargetValue = -10000.0f;
	OrientationOfPed = CGeneral::GetATanOfXY(GetMatrix().GetForward().x, GetMatrix().GetForward().y);

	CPad *pPad = GetPadFromPlayer();
	if(CMaths::Abs(pPad->GetPedWalkLeftRight()) > 60 || CMaths::Abs(pPad->GetPedWalkUpDown()) > 60)
	{
		OrientationOfPed = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -pPad->GetPedWalkLeftRight(), pPad->GetPedWalkUpDown());
		OrientationOfPed = OrientationOfPed - TheCamera.Orientation;

		// bloody "GetATanOfXY" does it in the wrong order
		OrientationOfPed = OrientationOfPed + HALF_PI;

		OrientationOfPed = CGeneral::LimitRadianAngle(OrientationOfPed);
	}

	if(GetWeapon()->GetWeaponType()==WEAPONTYPE_SPRAYCAN)
	{
		// is orientationOfPed the same as heading?!
		float fPlayerHeading = GetHeading();
	
		float fRange = 8.0f;
		float fTestAngle, fBestAngle = PI;
		CVector vecSearchPos = GetPosition() + fRange*GetMatrix().GetForward();
		CVector vecTargetOffset;
		CEntity *pFoundEntity = NULL;
		CEntity *ppResults[16];
		int16 nNum;
		// search for buildings
		CWorld::FindObjectsInRange(vecSearchPos, fRange, false, &nNum, 15, ppResults, true, false, false, false, false);

		for(int32 i=0; i<nNum; i++)
		{
			if(CTagManager::IsTag(ppResults[i]) && CTagManager::GetAlpha(ppResults[i]) < 255)
			{
				vecTargetOffset = ppResults[i]->GetPosition() - GetPosition();
				if(vecTargetOffset.MagnitudeSqr() < fWeaponRange*fWeaponRange)
				{
					fTestAngle = CMaths::ATan2(-vecTargetOffset.x, vecTargetOffset.y);
					fTestAngle -= fPlayerHeading;
					if(fTestAngle < -PI)	fTestAngle += TWO_PI;
					else if(fTestAngle > PI)	fTestAngle -= TWO_PI;
					
					if(fTestAngle < fBestAngle || pFoundEntity==NULL)
					{
						fBestAngle = fTestAngle;
						pFoundEntity = ppResults[i];
					}
				}
			}
		}
		
		if(pFoundEntity && !LOSBlockedBetweenPeds(this, pFoundEntity))	// Do this here because it could be expensive (line check)
		{
			SetWeaponLockOnTarget(pFoundEntity);
			GetPlayerData()->m_bDontAllowWeaponChange = true;
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			Say(AE_PED_AIM_GUN);		
#endif //USE_AUDIOENGINE
			return TRUE;
		}
		
//		return false;
	}

	CPedPool& pool = CPools::GetPedPool();
	CPed* pPed;
	CPed* targetPed;
	int32 i=pool.GetSize();


//	float	Orientation;
//	Orientation = CGeneral::GetATanOfXY(GetMatrix().GetForward().x, GetMatrix().GetForward().y);

	while(i--)
	{
		pPed = pool.GetSlot(i);
		if(pPed && pPed != this)
		{
			// Is this ped a candidate
			if (pPed->GetPedState() != PED_DIE && pPed->GetPedState() != PED_DEAD 
			&& PedCanBeTargettedVehicleWise(pPed)
			&& !pPed->m_nPedFlags.bNeverEverTargetThisPed && //(pPed->pLeader != this) &&
			(!((pPed->GetPedType() == PEDTYPE_PLAYER1 || pPed->GetPedType() == PEDTYPE_PLAYER2) && CGameLogic::bPlayersCannotTargetEachother)) )
			{
				if(!CPedGroups::AreInSameGroup(pPed,this))
				{
//					if (CanIKReachThisTarget(pPed->GetPosition(), GetWeapon(), true))
					{
						float	Orientation2 = CGeneral::GetATanOfXY(pPed->GetPosition().x - this->GetPosition().x, pPed->GetPosition().y - this->GetPosition().y);
						float	OrientationDiff = Orientation2 - OrientationOfPed;
						while (OrientationDiff > PI) OrientationDiff -= 2.0f * PI;
						while (OrientationDiff < -PI) OrientationDiff += 2.0f * PI;

						if (ABS(OrientationDiff) < (WiderTargetAngleCone * ((PI/180.0f) / 2.0f)))
						{
								// Now we do a second test for the angle relative to a point a few meters behind the player.
							CVector	Forward = this->GetMatrix().GetForward();
							Forward.Normalise();
							CVector TestPos = this->GetPosition() - Forward * ConeShift;
						
							float	Orientation3 = CGeneral::GetATanOfXY(pPed->GetPosition().x - TestPos.x, pPed->GetPosition().y - TestPos.y);
							float	OrientationDiff = Orientation3 - OrientationOfPed;
						
							while (OrientationDiff > PI) OrientationDiff -= 2.0f * PI;
							while (OrientationDiff < -PI) OrientationDiff += 2.0f * PI;

							if (ABS(OrientationDiff) < (TargetAngleCone * ((PI/180.0f) / 2.0f)))
							{
								float	Distance = (pPed->GetPosition() - this->GetPosition()).Magnitude();
					
								float	Range = pWeaponInfo->GetTargetRange() * CWeapon::TargetWeaponRangeMultiplier(pPed, this);
								if (Distance < Range)
								{
									EvaluateTarget(pPed, &pTargetEntity, &TargetValue, Range, OrientationOfPed);
								}
							}
						}
					}
				}
			}
		}
	}

	CObjectPool& object_pool = CPools::GetObjectPool();
	CObject* pObj;
	int32 j=object_pool.GetSize();
	while(j--)
	{
		pObj = object_pool.GetSlot(j);
		
		if (pObj && pObj->CanBeTargetted() && (!pObj->m_nObjectFlags.bHasExploded) && pObj->GetRwObject() && CanIKReachThisTarget(pObj->GetPosition(), GetWeapon(), true))
		{
			EvaluateTarget(pObj, &pTargetEntity, &TargetValue, pWeaponInfo->GetTargetRange(), OrientationOfPed, true);
		}
	}

	if (CGameLogic::IsCoopGameGoingOn())
	{
		CVehiclePool& vehicle_pool = CPools::GetVehiclePool();
		CVehicle* pVeh;
		int32 j=vehicle_pool.GetSize();
		while(j--)
		{
			pVeh = vehicle_pool.GetSlot(j);
		
			if (pVeh && (!pVeh->m_nPhysicalFlags.bRenderScorched) && pVeh->GetVehicleType() != VEHICLE_TYPE_BMX && CanIKReachThisTarget(pVeh->GetPosition(), GetWeapon(), true))
			{
				EvaluateTarget(pVeh, &pTargetEntity, &TargetValue, pWeaponInfo->GetTargetRange(), OrientationOfPed, true);
			}
		}
	}

	
	if (pTargetEntity)
	{
		if(pTargetEntity->GetIsTypePed())
		{
			
			targetPed = (CPed *)pTargetEntity;
			// we found a ped within weapon range - lock on to him/her
			SetWeaponLockOnTarget(pTargetEntity);
			
			/*				
			This code is duplicated in TaskPlayer.cpp
			
			if (pWeaponInfo->GetWeaponFireType() != FIRETYPE_MELEE)
			{
				// send the event to the individual or the group
				CPedGroup* pPedGroup = CPedGroups::GetPedsGroup(targetPed);
				if (pPedGroup)
				{
					// add a group event if the target ped is part of a different group
					if(!CPedGroups::AreInSameGroup(targetPed,(CPed*)this))
					{
						CEventGroupEvent groupEvent(targetPed, new CEventGunAimedAt((CPed *)this));
						pPedGroup->GetGroupIntelligence()->AddEvent(groupEvent);
					}
				}
				else
				{
					// add the individual event
					CEventGunAimedAt event((CPed *)this);
					targetPed->GetPedIntelligence()->AddEvent(event);
				}
			}
			*/			
			
			GetPlayerData()->m_bDontAllowWeaponChange = true;
//			SetPointGunAt(pTargetEntity);
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			Say(AE_PED_AIM_GUN);		
#endif //USE_AUDIOENGINE
			return TRUE;	
		}
		else
		{
			// we found an object within weapon range - lock onto it
			SetWeaponLockOnTarget(pTargetEntity);
			GetPlayerData()->m_bDontAllowWeaponChange = true;
//			SetPointGunAt(pTargetEntity);
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			Say(AE_PED_AIM_GUN);		
#endif //USE_AUDIOENGINE
			return TRUE;		
		}

	}
	else
	{
		// no peds in weapon range - can't lock on
		return FALSE;
	}
}



#ifndef FINAL
void CPlayerPed::DisplayTargettingDebug()
{
	CPedPool& pool = CPools::GetPedPool();
	CPed* pPed;
//	CPed* targetPed;
	int32 i=pool.GetSize();
	CEntity	*pTargetEntity = NULL, *pTargetEntityTemp = NULL;
	float	TargetValue = 0.0f, TargetValueTemp = 0.0f;
	float	OrientationOfPed = CGeneral::GetATanOfXY(GetMatrix().GetForward().x, GetMatrix().GetForward().y);

		// First find out which ped would be targetted
	while(i--)
	{
		pPed = pool.GetSlot(i);
		if(pPed && !pPed->IsPlayer())
		{
			// Is this ped a candidate
			if (pPed->GetPedState() != PED_DIE && pPed->GetPedState() != PED_DEAD 
			&& PedCanBeTargettedVehicleWise(pPed)
			&& /*(pPed->pLeader != this) &&*/ !pPed->m_nPedFlags.bNeverEverTargetThisPed &&
			(!((pPed->GetPedType() == PEDTYPE_PLAYER1 || pPed->GetPedType() == PEDTYPE_PLAYER2) && CGameLogic::bPlayersCannotTargetEachother)) )
			{
				if(!CPedGroups::AreInSameGroup(pPed,this))
				{
					if (CanIKReachThisTarget(pPed->GetPosition(), GetWeapon(), true))
					{
						EvaluateTarget(pPed, &pTargetEntity, &TargetValue, 100.0f, OrientationOfPed);
					}
				}
			}
		}
	}

	DefinedState2d();
	
	i=pool.GetSize();
	while(i--)
	{
		pPed = pool.GetSlot(i);
		if(pPed && !pPed->IsPlayer())
		{
			// Is this ped a candidate
			if (pPed->GetPedState() != PED_DIE && pPed->GetPedState() != PED_DEAD 
			&& PedCanBeTargettedVehicleWise(pPed)
			&& /*(pPed->pLeader != this) &&*/ !pPed->m_nPedFlags.bNeverEverTargetThisPed &&
			(!((pPed->GetPedType() == PEDTYPE_PLAYER1 || pPed->GetPedType() == PEDTYPE_PLAYER2) && CGameLogic::bPlayersCannotTargetEachother)) )
			{
				if(!CPedGroups::AreInSameGroup(pPed,this))
				{
					if (CanIKReachThisTarget(pPed->GetPosition(), GetWeapon(), true))
					{
						TargetValueTemp = 0.0f;
						EvaluateTarget(pPed, &pTargetEntityTemp, &TargetValueTemp, 100.0f, OrientationOfPed);
						sprintf(gString, "%.2f Pri:%.2f Pos:%.2f\n", TargetValueTemp, FindTargetPriority(pPed), TargetValueTemp / FindTargetPriority(pPed));
					}
//					else
//					{
//						sprintf(gString, "IK can't reach\n");
//					}
				}
				else
				{
					sprintf(gString, "In same group\n");
				}
			}
			else
			{
				sprintf(gString, "XXX\n");
			}
			
			float	ScaleX, ScaleY;
			CVector	ScreenCoors, In = pPed->GetPosition() + CVector(0.0f, 0.0f, 1.5f);
			if (CSprite::CalcScreenCoors(In, &ScreenCoors, &ScaleX, &ScaleY))
			{
				// setup fonts for rendering
				CFont::SetProportional(TRUE);
				CFont::SetBackground(TRUE);
				CFont::SetScale( MIN (0.7f, ScaleX / 60.0f), MIN (0.7f,ScaleY / 60.0f));
				CFont::SetOrientation(FO_CENTRE);
				CFont::SetCentreSize(SCREEN_WIDTH);
				if (pPed == pTargetEntity)
				{
					CFont::SetColor( CRGBA(255, 128, 0, 255) );
				}
				else
				{
					CFont::SetColor( CRGBA(255, 255, 0, 255) );
				}
		        CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);
        
		    	AsciiToGxtChar(gString, gGxtString);
				CFont::PrintString( ScreenCoors.x, ScreenCoors.y, gGxtString);
			}
		}	
	}
}



#endif

void CPlayerPed::RotatePlayerToTrackTarget()
{
	return;

	// This only happens for 2 handed weapons
	if(!CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill())->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM)
	|| m_nPedFlags.bIsDucking)
	{
		// If the player is still changing his direction (using the left analogue stick)
		// we don't do the tracking thing.
//		if (!CPad::GetPad(0)->GetPedWalkLeftRight())
		{
			float	ExtremeAngle = MAX_TORSO_ROTATION * (PI / 180.0f) * 0.5f;
			float Yaw = CGeneral::GetRadianAngleBetweenPoints(GetWeaponLockOnTarget()->GetPosition().x, GetWeaponLockOnTarget()->GetPosition().y, this->GetMatrix().tx, this->GetMatrix().ty);

			// don't bother with rotation torso, just rotate whole ped for now (SA)
			m_fDesiredHeading = Yaw;
/*
			float Delta = this->m_fCurrentHeading - Yaw;
			Delta = CGeneral::LimitRadianAngle(Delta);

			if (Delta < -ExtremeAngle)
			{
				this->m_fCurrentHeading -= ExtremeAngle + Delta;
				this->m_fDesiredHeading -= ExtremeAngle + Delta;
			}
			else if (Delta > ExtremeAngle)
			{
				this->m_fCurrentHeading -= Delta - ExtremeAngle;
				this->m_fDesiredHeading -= Delta - ExtremeAngle;
			}
*/
		}
	}
}

/*
//
float PLAYER_AIM_STICK_SENS = 0.003f;
//
void CPlayerPed::RotatePlayerManualAiming(CPad *pPad)
{
	return;

	if(!GetPlayerData()->m_bFreeAiming)
	{
		GetPlayerData()->m_fLookPitch = 0.0f;
		GetPlayerData()->m_bFreeAiming =true;
	}
	
	float fStickX= 	-(pPad->LookAroundLeftRight());
	float fStickY=	pPad->LookAroundUpDown();
	
	fStickX *= PLAYER_AIM_STICK_SENS * (0.25f/3.5f) * CTimer::GetTimeStep();
	fStickY *= PLAYER_AIM_STICK_SENS * (0.15f/3.5f) * CTimer::GetTimeStep();

	m_fDesiredHeading += fStickX;

	GetPlayerData()->m_fLookPitch -= fStickY;
	if(GetPlayerData()->m_fLookPitch < -QUARTER_PI)	GetPlayerData()->m_fLookPitch = -QUARTER_PI;
	else if(GetPlayerData()->m_fLookPitch > QUARTER_PI)	GetPlayerData()->m_fLookPitch = QUARTER_PI;
}
*/

// returns true if the movement of the player should be disabled.
bool CPlayerPed::MovementDisabledBecauseOfTargeting()
{
	return false;
/*	
	if(!CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
	{
		if (GetWeaponLockOnTarget())
		{
			return true;
		}
	}
	return false;
*/
}




// Name			:	FindNextWeaponLockOnTarget
// Purpose		:	This function finds a neighbouring target. Clockwise or
//					counterclockwise from the current target
// Parameters	:	None
// Returns		:	TRUE if a target is found, FALSE otherwise

bool CPlayerPed::FindNextWeaponLockOnTarget(CEntity *pOldTarget, bool bDirection)
{
	float fWeaponRange;
	float TargetValue;
	CEntity* pTargetEntity = NULL;
	CVector2D vec2DPedDistance;
	float	OrientationOfPed;
	float	OrientationToTarget;

//	ASSERT(pOldTarget);		// We don't have an old target if we're doing free aiming at the moment
//	ASSERT(GetWeaponLockOn());

//	if(GetWeapon()->GetWeaponType()==WEAPONTYPE_SPRAYCAN)	Got a bug about L1/R1 not working for spraycan to removed this line (Obbe)
//		return FindWeaponLockOnTarget();

	fWeaponRange = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill())->GetTargetRange();

	pTargetEntity = 0;
	TargetValue = -10000.0f;
	OrientationOfPed = CGeneral::GetATanOfXY(GetMatrix().GetForward().x, GetMatrix().GetForward().y);

	if (pOldTarget)
	{
		OrientationToTarget = CGeneral::GetATanOfXY(pOldTarget->GetPosition().x - TheCamera.GetPosition().x, pOldTarget->GetPosition().y - TheCamera.GetPosition().y);
	}
	else
	{	// If we don't have an old target the player is doing a bit of free-aiming.
		// Use the orientation of the camera instead.
		OrientationToTarget = CGeneral::GetATanOfXY(TheCamera.GetMatrix().GetForward().x, TheCamera.GetMatrix().GetForward().y);	
	}

	// Go through all of the peds instead of just the close ones. The close ones
	// only go up to 30 meters or so which might not be enough.
	CPedPool& pool = CPools::GetPedPool();
	CPed* pPed;
	int32 i=pool.GetSize();	
	while(i--)
	{
		pPed = pool.GetSlot(i);
		if(pPed && pPed != this && pPed != pOldTarget)
		{
			// Is this ped a candidate
			if(pPed->GetPedState() != PED_DIE && pPed->GetPedState() != PED_DEAD 
			&& PedCanBeTargettedVehicleWise(pPed)
			&& /*pPed->pLeader != this &&*/ !pPed->m_nPedFlags.bNeverEverTargetThisPed)
			{
				if(!CPedGroups::AreInSameGroup(pPed,this))
				{
					if (!((pPed->GetPedType() == PEDTYPE_PLAYER1 || pPed->GetPedType() == PEDTYPE_PLAYER2) && CGameLogic::bPlayersCannotTargetEachother))
					{
						if ( (!LOSBlockedBetweenPeds(this, pPed)) && CanIKReachThisTarget(pPed->GetPosition(), GetWeapon(), true))
						{
							EvaluateNeighbouringTarget(pPed, &pTargetEntity, &TargetValue, fWeaponRange * CWeapon::TargetWeaponRangeMultiplier(pPed, this), OrientationToTarget, bDirection);
						}
					}
				}
			}
		}
	}

	CObjectPool& object_pool = CPools::GetObjectPool();
	CObject* pObj;
	int32 j=object_pool.GetSize();
	while(j--)
	{
		pObj = object_pool.GetSlot(j);
		
		if (pObj && pObj->CanBeTargetted() && (!pObj->m_nObjectFlags.bHasExploded) && pObj->GetRwObject() && CanIKReachThisTarget(pObj->GetPosition(), GetWeapon(), true))
		{
			EvaluateNeighbouringTarget(pObj, &pTargetEntity, &TargetValue, fWeaponRange * CWeapon::TargetWeaponRangeMultiplier(pPed, this), OrientationToTarget, bDirection);
		}
	}

	if (CGameLogic::IsCoopGameGoingOn())
	{
		CVehiclePool& vehicle_pool = CPools::GetVehiclePool();
		CVehicle* pVeh;
		int32 j=vehicle_pool.GetSize();
		while(j--)
		{
			pVeh = vehicle_pool.GetSlot(j);
		
			if (pVeh && (!pVeh->m_nPhysicalFlags.bRenderScorched) && pVeh->GetVehicleType() != VEHICLE_TYPE_BMX && CanIKReachThisTarget(pVeh->GetPosition(), GetWeapon(), true))
			{
				EvaluateNeighbouringTarget(pVeh, &pTargetEntity, &TargetValue, fWeaponRange * CWeapon::TargetWeaponRangeMultiplier(pPed, this), OrientationToTarget, bDirection);
			}
		}
	}


	if (pTargetEntity)
	{
		if (pTargetEntity->GetIsTypePed())
		{
			CPed* pPed = (CPed*)pTargetEntity;
			if (CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetWeaponFireType() != FIRETYPE_MELEE)
			{
				// send the event to the individual or the group
				CPedGroup* pPedGroup = CPedGroups::GetPedsGroup(pPed);
				if (pPedGroup)
				{
					if(!CPedGroups::AreInSameGroup(pPed,(CPed*)this))
					{
						// add a group event if the target ped is part of a group
						CEventGroupEvent groupEvent(pPed, new CEventGunAimedAt((CPed *)this));
						pPedGroup->GetGroupIntelligence()->AddEvent(groupEvent);
					}
				}
				else
				{
					// add the individual event
					CEventGunAimedAt event((CPed *)this);
					pPed->GetPedIntelligence()->AddEvent(event);
				}
			}
		}
		
		// we found a ped within weapon range - lock on to him/her
		SetWeaponLockOnTarget(pTargetEntity);
		GetPlayerData()->m_bDontAllowWeaponChange = true;
//		SetPointGunAt(pTargetEntity);
		return TRUE;
	}
	else
	{
		// no peds in weapon range - can't lock on
		return FALSE;
	}
}


// Name			:	FindTargetPriority
// Purpose		:	Finds a number between 0 and 1 that represents the target priority for this ped.
//					This has nothing to do with the distance to the target; just how much of a threat it is.
// Parameters	:	None
// Returns		:

#define PRIO_HARMLESS			(0.0f)
#define PRIO_NEUTRAL 			(0.1f)
#define PRIO_INGANGORFRIEND		(0.05f)
#define PRIO_GROVE				(0.06f)
#define PRIO_POTENTIALTHREAT	(0.25f)
#define PRIO_DIRECTTHREAT		(0.8f)
#define PRIO_ISTARGETPRIORITY	(1.0f)

float CPlayerPed::FindTargetPriority(CEntity *pTarget)
{
	float	Result = PRIO_NEUTRAL;		// Minimum priority

	switch (pTarget->GetType())
	{
		case ENTITY_TYPE_PED:
			{
				CPed *pPed = (CPed *)pTarget;

				if (pPed->GetCharCreatedBy() == MISSION_CHAR)
				{
					Result = PRIO_POTENTIALTHREAT;
				}

				if (pPed->GetPedType() == PEDTYPE_PLAYER_GANG)
				{
					Result = PRIO_GROVE;
				}

				if (CPedGroups::AreInSameGroup(this, pPed))
				{
					Result = PRIO_INGANGORFRIEND;
				}

				if (pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_KILL_PED_ON_FOOT) ||
					pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ARREST_PED))
				{
					Result = PRIO_DIRECTTHREAT;
				}
		
				if (pPed->m_nPedFlags.bThisPedIsATargetPriority)
				{
					Result = PRIO_ISTARGETPRIORITY;
				}
			}
			break;

		case ENTITY_TYPE_OBJECT:
			{
				CObject *pObject = (CObject *)pTarget;
				if ( (pObject->ObjectCreatedBy != MISSION_OBJECT) && (pObject->ObjectCreatedBy != MISSION_BRAIN_OBJECT) )
				{
					Result = PRIO_HARMLESS;
				}
			}
			break;

		case ENTITY_TYPE_VEHICLE:		// Only happens in multiplayer games.
			Result = PRIO_NEUTRAL;
			break;
			
		default:
			ASSERT(0);
			break;
	}

	return Result;
}



// Name			:	EvaluateTarget
// Purpose		:	Tests whether this target is better than the ones already found
// Parameters	:	None
// Returns		:

/*
void CPlayerPed::EvaluateTarget(CEntity *pTarget, CEntity **ppOldTarget, float *pOldTargetValue, float fWeaponRange, float OrientationOfPed, bool bIsThisAThreat)
{
	float		fPedDistance;

		// Is this candidate within range
	fPedDistance = (pTarget->GetPosition() - GetPosition()).Magnitude();

	// check if ped is in this weapon's range
	if (fPedDistance <= fWeaponRange)
	{

		if (!DoesTargetHaveToBeBroken(pTarget, GetWeapon()))
		{
			// Calculate target value (higher = more likely to be a candidate)
//			TestTargetValue = -fPedDistance;

//			float OrientationToPed = CGeneral::GetATanOfXY( (pTarget->GetPosition().x - this->GetPosition().x), (pTarget->GetPosition().y - this->GetPosition().y) );
//			float OrientationDiff = OrientationToPed - OrientationOfPed;
//			while (OrientationDiff >  PI) OrientationDiff -= 2.0f * PI;
//			while (OrientationDiff < -PI) OrientationDiff += 2.0f * PI;

			// Try taking the orientation relative to the camera into account.
//			float OrientationToCam = CGeneral::GetATanOfXY( (pTarget->GetPosition().x - TheCamera.GetPosition().x), (pTarget->GetPosition().y - TheCamera.GetPosition().y) );
//			float OrientationOfCam = CGeneral::GetATanOfXY( TheCamera.GetMatrix().GetForward().x, TheCamera.GetMatrix().GetForward().y );
//			float OrientationDiff = OrientationToPed - OrientationOfCam;
//			while (OrientationDiff >  PI) OrientationDiff -= 2.0f * PI;
//			while (OrientationDiff < -PI) OrientationDiff += 2.0f * PI;
//			float TestTargetValue = PI - ABS(OrientationDiff);

			// Test: Calculate the sprite onscreen coordinates for this target.
#define VALBEHINDCAM	 	(0.0f)
#define VALBEHINDPLAYER	 	(0.15f)
#define VALATSIDEOFSCREEN 	(0.5f)
#define VALATCENTEROFSCREEN (1.0f)
			CVector	ScreenCoors;
			float	ScaleX, ScaleY;
			float	TestTargetValue = VALBEHINDCAM;

			if (CSprite::CalcScreenCoors(pTarget->GetPosition(), &ScreenCoors, &ScaleX, &ScaleY, false))
			{

				float ScreenValue = ABS( (ScreenCoors.x/SCREEN_WIDTH) - 0.5f);
				if (ScreenValue > 0.5f)
				{
					TestTargetValue = VALATSIDEOFSCREEN;
				}
				else
				{
					ScreenValue = 2.0f * (0.5f - ScreenValue);	// Runs from 0.0f to 1.0f
					TestTargetValue = VALATSIDEOFSCREEN + ScreenValue * (VALATCENTEROFSCREEN - VALATSIDEOFSCREEN);
				}
				
				// If the potential target is actually behind the player we consider it 'off screen'.
				if (DotProduct(pTarget->GetPosition() - this->GetPosition(), TheCamera.GetMatrix().GetForward()) < 0.0f)
				{
					TestTargetValue = MIN(VALBEHINDPLAYER, TestTargetValue);
				}
				if (DotProduct(pTarget->GetPosition() - TheCamera.GetPosition(), TheCamera.GetMatrix().GetForward()) < 0.0f)
				{
					TestTargetValue = MIN(VALBEHINDCAM, TestTargetValue);
				}
			}
			
			// Now take distance into account.
			if (fPedDistance > 1.0f)
			{
				TestTargetValue *= 1.0f / (CMaths::Sqrt(fPedDistance));
//				TestTargetValue *= 1.0f / (CMaths::Sqrt(CMaths::Sqrt(fPedDistance)));
			}
				
			TestTargetValue *= FindTargetPriority(pTarget);

			if (TestTargetValue > *pOldTargetValue)
			{
				if (!LOSBlockedBetweenPeds(this, pTarget))	// Do this here because it could be expensive (line check)
				{
					*ppOldTarget = pTarget;
					*pOldTargetValue = TestTargetValue;
				}
			}
		}
	}
}
*/


// Name			:	EvaluateTarget
// Purpose		:	Tests whether this target is better than the ones already found
// Parameters	:	None
// Returns		:

void CPlayerPed::EvaluateTarget(CEntity *pTarget, CEntity **ppOldTarget, float *pOldTargetValue, float fWeaponRange, float OrientationOfPed, bool bIsThisAThreat)
{
	float		fPedDistance;

		// Is this candidate within range
	fPedDistance = (pTarget->GetPosition() - GetPosition()).Magnitude();

	// check if ped is in this weapon's range
	if (fPedDistance <= fWeaponRange)
	{
		if (!DoesTargetHaveToBeBroken(pTarget, GetWeapon()))
		{
			float OrientationToPed = CGeneral::GetATanOfXY( (pTarget->GetPosition().x - this->GetPosition().x), (pTarget->GetPosition().y - this->GetPosition().y) );
			float OrientationDiff = OrientationToPed - OrientationOfPed;
			while (OrientationDiff >  PI) OrientationDiff -= 2.0f * PI;
			while (OrientationDiff < -PI) OrientationDiff += 2.0f * PI;

			// Try taking the orientation relative to the camera into account.
//			float TestTargetValue = PI - ABS(OrientationDiff);
			float	TestTargetValue = 1.0f - ABS(OrientationDiff * (180.0f / PI)) / WiderTargetAngleCone;

			// Calculate target value (higher = more likely to be a candidate)
			if (fPedDistance > 1.0f)
			{
				TestTargetValue *= 1.0f / CMaths::Sqrt(CMaths::Sqrt(fPedDistance));
			}
				
			TestTargetValue *= FindTargetPriority(pTarget);

			if (TestTargetValue > *pOldTargetValue)
			{
				if (!LOSBlockedBetweenPeds(this, pTarget))	// Do this here because it could be expensive (line check)
				{
					*ppOldTarget = pTarget;
					*pOldTargetValue = TestTargetValue;
				}
			}
		}
	}
}




// Name			:	EvaluateNeighbouringTarget
// Purpose		:	Tests whether this target is better than the ones already found
// Parameters	:	None
// Returns		:

void CPlayerPed::EvaluateNeighbouringTarget(CEntity *pTarget, CEntity **ppOldTarget, float *pOldTargetValue, float fWeaponRange, float OrientationToTarget, bool bDirection)
{
	float		TestTargetValue, fPedDistance;
	float 		OrientationToPed, OrientationDiff;

		// Is this candidate within range
	fPedDistance = (pTarget->GetPosition() - GetPosition()).Magnitude();

	// check if ped is in this weapon's range
	if (fPedDistance <= fWeaponRange)
	{
		if (!DoesTargetHaveToBeBroken(pTarget, GetWeapon()))
		{
//			OrientationToPed = CGeneral::GetATanOfXY( (pTarget->GetPosition().x - this->GetPosition().x), (pTarget->GetPosition().y - this->GetPosition().y) );
			OrientationToPed = CGeneral::GetATanOfXY( (pTarget->GetPosition().x - TheCamera.GetPosition().x), (pTarget->GetPosition().y - TheCamera.GetPosition().y) );
			OrientationDiff = OrientationToPed - OrientationToTarget;
			while (OrientationDiff >  PI) OrientationDiff -= 2.0f * PI;
			while (OrientationDiff < -PI) OrientationDiff += 2.0f * PI;

			if (ABS(OrientationDiff) < 50.0f * (PI / 180.0f))
			{
				if (bDirection)
				{
					if (OrientationDiff <= 0.0f)
					{
						TestTargetValue = -100000.0f;
					}
					else
					{
						TestTargetValue = -ABS(OrientationDiff);
					}
				}
				else
				{
					if (OrientationDiff >= 0.0f)
					{
						TestTargetValue = -100000.0f;
					}
					else
					{
						TestTargetValue = -ABS(OrientationDiff);
					}
				}

	//			if (bIsThisAThreat)		// If this guy is a threat we make it more likely to be a target
	//			{
	//				TestTargetValue += 15.0f;
	//			}

				if (TestTargetValue > *pOldTargetValue)
				{
					*ppOldTarget = pTarget;
					*pOldTargetValue = TestTargetValue;
				}
			}
		}
	}
}

#ifdef GTA_PC

void CPlayerPed::Clear3rdPersonMouseTarget()
{
	if(m_pMouseLockOnRecruitPed)
	{
		TIDYREF(m_pMouseLockOnRecruitPed,(CEntity**)&m_pMouseLockOnRecruitPed);
		m_pMouseLockOnRecruitPed=0;
	}
}

void CPlayerPed::Compute3rdPersonMouseTarget(bool bUseCrosshair = true)
{
	CPed* pTarget=0;
	if(TheCamera.m_bUseMouse3rdPerson)
	{
		const float fRange = CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill())->GetTargetRange();//30.0f;
		CVector vecGunMuzzle=GetPosition();
		CVector vecSource;
		CVector vecTarget;

		// if we're using an aimed weapon then we want to choose the ped in the crosshairs
		if(bUseCrosshair)
		{
			TheCamera.Find3rdPersonCamTargetVector(fRange,vecGunMuzzle,vecSource,vecTarget);
		}
		// otherwise just pick the ped straight ahead
		else
		{
			vecSource = TheCamera.Cams[TheCamera.ActiveCam].Source;
			vecTarget = TheCamera.Cams[TheCamera.ActiveCam].Front;

			CVector vecSourceOffset =  vecSource - vecGunMuzzle;
			if(DotProduct(vecSourceOffset, vecTarget) < 0.0f)
				vecSource -= vecTarget*DotProduct(vecSourceOffset, vecTarget);

			vecTarget *= fRange;
			vecTarget += vecSource;
		}

		{
			CColPoint colPoint;
			CEntity* pHitEntity=0;
			CWorld::pIgnoreEntity = this;
			CWorld::bIncludeBikers = true;
			if(CWorld::ProcessLineOfSight(vecSource,vecTarget,colPoint,pHitEntity,false,false,true,false,false))
			{
				ASSERT(pHitEntity);
				ASSERT(pHitEntity->GetType()==ENTITY_TYPE_PED);
				CPed* pHitPed=(CPed*)pHitEntity;
				if((pHitEntity!=this) && (pHitPed->IsAlive()))
				{
					pTarget=pHitPed;							
				}
			}
			CWorld::ResetLineTestOptions();
		}
	}

	if(pTarget)
	{
		if(pTarget!=m_pMouseLockOnRecruitPed)
		{
			if(m_pMouseLockOnRecruitPed)
			{
				TIDYREF(m_pMouseLockOnRecruitPed,(CEntity**)&m_pMouseLockOnRecruitPed);
			}
			m_pMouseLockOnRecruitPed=pTarget;
			REGREF(m_pMouseLockOnRecruitPed,(CEntity**)&m_pMouseLockOnRecruitPed);
		}
		m_iMouseLockOnRecruitTimer=CTimer::GetTimeInMilliseconds()+1000;

		// this is considered the same as locking on to someone, so we want to make them put up their hands
		if(CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType())->GetWeaponFireType() != FIRETYPE_MELEE
		&& CWeaponInfo::GetWeaponInfo(GetWeapon()->GetWeaponType(), GetWeaponSkill())->IsWeaponFlagSet(WEAPONTYPE_CANAIM))
		{
			if(pTarget->GetPedIntelligence()->IsInSeeingRange(this->GetPosition()))
			{
				CTask * task = pTarget->GetPedIntelligence()->GetTaskEventResponse();
				if(!task || task->GetTaskType() != CTaskTypes::TASK_COMPLEX_REACT_TO_GUN_AIMED_AT)
				{
					if ((GetWeapon()->GetWeaponType()) != WEAPONTYPE_PISTOL_SILENCED)
						Say(CONTEXT_GLOBAL_PULL_GUN);

					// send the event to the individual or the group
					CPedGroup* pPedGroup = CPedGroups::GetPedsGroup(pTarget);
					if (pPedGroup)
					{
						// add a group event if the target ped is part of a different group
						if(!CPedGroups::AreInSameGroup(pTarget, this))
						{
							CEventGroupEvent groupEvent(pTarget, new CEventGunAimedAt(this));
							pPedGroup->GetGroupIntelligence()->AddEvent(groupEvent);
						}
					}
					else
					{
						// add the individual event
						CEventGunAimedAt event(this);
						pTarget->GetPedIntelligence()->AddEvent(event);
					}					
				}
			}
		}
	}
	else if(m_pMouseLockOnRecruitPed && m_iMouseLockOnRecruitTimer<CTimer::GetTimeInMilliseconds())
	{
		m_pMouseLockOnRecruitPed=0;
	}
}

void CPlayerPed::DrawTriangleForMouseRecruitPed() const
{
	if(!m_pMouseLockOnRecruitPed)
	{
		return;
	}

	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);	
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,  NULL); 
#if (defined GTA_PC || defined GTA_XBOX)
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)rwALPHATESTFUNCTIONALWAYS); //ignore alpha test just now
#endif

	UInt8 Red, Green, Blue;
	float targetHealth = m_pMouseLockOnRecruitPed->m_nHealth / m_pMouseLockOnRecruitPed->m_nMaxHealth;
	if(targetHealth > 1.0f)	targetHealth = 1.0f;
	if (targetHealth > 0.0f)  // if not dead then colour target based on health
	{	
		Red = targetHealth * TARGET_GOOD_R + (1.0f - targetHealth) * TARGET_DANGER_R;
		Green = targetHealth * TARGET_GOOD_G + (1.0f - targetHealth) * TARGET_DANGER_G;
		Blue = targetHealth * TARGET_GOOD_B + (1.0f - targetHealth) * TARGET_DANGER_B;		
	}
	else  // if dead then its just black
	{
		Red = Green = Blue = 0;
	}
	UInt8 Alpha=255;

	const float minScale=0.175f;
	const float maxScale=1.0f;
	const float distance=(m_pMouseLockOnRecruitPed->GetPosition()-GetPosition()).Magnitude();
	const float minDistance=10.0f;
	const float beta=0.02f;
	const float scale=minScale + CMaths::Min(1.0f,beta*CMaths::Max(distance - minDistance,0))*(maxScale-minScale);
	CVector right=TheCamera.GetRight();
	right*=scale;
	CVector up=CVector(0,0,1);
	up*=scale;

	CVector pos;
	CVector pos0,pos1,pos2;
	if(m_pMouseLockOnRecruitPed->GetPedType()==PEDTYPE_GANG1)
	{
		//For gang types that can be recruited draw the triangle with horizontal line at top.
		pos=m_pMouseLockOnRecruitPed->GetPosition()+CVector(0,0,1.0f);
		pos0=pos;
		pos1=pos-right;
		pos1+=up;
		pos2=pos+right;
		pos2+=up;
	}
	else
	{
		//For non-gang types draw the triangle with horizontal line at bottom
		pos=m_pMouseLockOnRecruitPed->GetPosition()+CVector(0,0,1.0f);
		pos0=pos+up;
		pos1=pos+right;
		pos2=pos-right;
	}
	
	// bring closer to the camera
	CVector ptToCam = TheCamera.GetPosition() - pos0;
	ptToCam.Normalise();
	pos0 += ptToCam;

	ptToCam = TheCamera.GetPosition() - pos1;
	ptToCam.Normalise();
	pos1 += ptToCam;

	ptToCam = TheCamera.GetPosition() - pos2;
	ptToCam.Normalise();
	pos2 += ptToCam;

	RwIm3DVertexSetPos(&TempVertexBuffer.m_3d[0], pos0.x, pos0.y, pos0.z);
	RwIm3DVertexSetRGBA(&TempVertexBuffer.m_3d[0], Red, Green, Blue, 255);

	RwIm3DVertexSetPos(&TempVertexBuffer.m_3d[1], pos1.x, pos1.y, pos1.z);
	RwIm3DVertexSetRGBA(&TempVertexBuffer.m_3d[1], Red, Green, Blue, 0);

	RwIm3DVertexSetPos(&TempVertexBuffer.m_3d[2], pos2.x, pos2.y, pos2.z);
	RwIm3DVertexSetRGBA(&TempVertexBuffer.m_3d[2], Red, Green, Blue, 0);

	TempBufferRenderIndexList[0] = 0;
	TempBufferRenderIndexList[1] = 1;
	TempBufferRenderIndexList[2] = 2;

	if (RENDERIMMEDIATEMODESTUFF)
	{
		if (RwIm3DTransform(&TempVertexBuffer.m_3d[0], 3, NULL, rwIM3D_VERTEXRGBA | rwIM3D_VERTEXXYZ))
		{
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, &TempBufferRenderIndexList[0], 3);
			RwIm3DEnd();
		}
	}

	// Done. Set stuff back
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);	
//	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
#if (defined GTA_PC || defined GTA_XBOX)
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)rwALPHATESTFUNCTIONGREATER); //ignore alpha test just now
#endif
}

#endif

// Name			:	IsThisPedAttackingPlayer
// Purpose		:	Work out whether this ped is attacking this particular player
// Parameters	:
// Returns		:

/*
bool CPlayerPed::IsThisPedAnAimingPriority(CPed *pPed)
{
	ASSERT(pPed);

	if(!pPed->m_nPedFlags.bPlayerFriend)
		return true;
		
	if (pPed->m_pEntLockOnTarget == this) return true;

	switch (pPed->m_Objective)
	{
		case KILL_CHAR_ON_FOOT:
		case KILL_CHAR_ANY_MEANS:
			if (pPed->m_pObjectivePed == this) return true;
			break;	
		default:
			break;
	}
	if (pPed->m_nPedState == PED_ABSEIL_FROM_HELI) return true;
	
	return false;
}
*/




// Name			:	Busted
// Purpose		:	Puts the player into 'arrested' state
// Parameters	:	None
// Returns		:	Nothing

void CPlayerPed::Busted()
{
	GetPlayerWanted()->m_nWantedLevel = 0;
} // end - CPlayerPed::Busted

void CPlayerPed::SetWantedLevel(Int32 NewLevel)
{
	ASSERT(GetPlayerWanted());
	GetPlayerWanted()->SetWantedLevel(NewLevel);
}

void CPlayerPed::SetWantedLevelNoDrop(Int32 NewLevel)
{
	ASSERT(GetPlayerWanted());
	GetPlayerWanted()->SetWantedLevelNoDrop(NewLevel);
}

void CPlayerPed::CheatWantedLevel(Int32 NewLevel)
{
	ASSERT(GetPlayerWanted());
	GetPlayerWanted()->CheatWantedLevel(NewLevel);
}


// Name			:	DebugMarkVisiblePeds
// Purpose		:	Marks peds currently visible to this player ped
// Parameters	:	None
// Returns		:	Nothing

void CPlayerPed::DebugMarkVisiblePeds()
{
//	int32 i = 0;

	// make sure the ped isn't already dying/dead
//	for (i = 0; i < m_NumClosePeds; i++)
//	{
//		if (OurPedCanSeeThisOne(m_apClosePeds[i]))
	
//		CWeaponEffects::MarkTarget(m_apClosePeds[i]->GetPosition(), 56,0,16,255, 1.0f);
//	}
} //end - CPlayerPed::DebugMarkVisiblePeds


/*
void CPlayerPed::RunningLand(CPad* pPad)
{
	CAnimBlendAssociation* pAnim = RpAnimBlendClumpGetAssociation((RpClump*)m_pRwObject, ANIM_STD_FALL_LAND);

	// If just started to play landing anim, check if can play land from jump anim. Will play it if the ped was almost
	// running and the controller is still being pushed
	if(pAnim && pAnim->GetCurrentTime() == 0.0f)
	{
		if(m_moveBlendRatio > 1.5f && 
			(pPad && (pPad->GetPedWalkLeftRight() != 0.0f || pPad->GetPedWalkUpDown() != 0.0f)))
		{
			pAnim->SetBlendDelta(-1000.0f);
			pAnim->SetFlag(ABA_FLAG_ISBLENDAUTOREMOVE);
			pAnim = CAnimManager::AddAnimation((RpClump*)m_pRwObject, ANIM_STD_PED, ANIM_STD_JUMP_LAND);
			pAnim->SetFinishCallback(FinishJumpCB, this);
			
			if(GetPedState() == PED_JUMP)
				RestorePreviousState();
		}
	}
}
*/

/*
void CPlayerPed::ClearObjectTargettable(Int32 Object)
{
	if (nExtraTargetEntity1 == Object)
	{
		nExtraTargetEntity1 = -1;
	}

	if (nExtraTargetEntity2 == Object)
	{
		nExtraTargetEntity2 = -1;
	}

	if (nExtraTargetEntity3 == Object)
	{
		nExtraTargetEntity3 = -1;
	}

	if (nExtraTargetEntity4 == Object)
	{
		nExtraTargetEntity4 = -1;
	}
}
*/

// Name			:	MakeObjectTargettable
// Purpose		:	Registers this object to be targettable by this player. Only one object at a time.
// Parameters	:	None
// Returns		:	Nothing
/*
void CPlayerPed::MakeObjectTargettable(Int32 Object)
{
	CObject	*pObj;

	pObj = CPools::GetObjectPool().GetAt(nExtraTargetEntity1);
	if (!pObj)
	{
		nExtraTargetEntity1 = Object;
		return;
	}

	pObj = CPools::GetObjectPool().GetAt(nExtraTargetEntity2);
	if (!pObj)
	{
		nExtraTargetEntity2 = Object;
		return;
	}

	pObj = CPools::GetObjectPool().GetAt(nExtraTargetEntity3);
	if (!pObj)
	{
		nExtraTargetEntity3 = Object;
		return;
	}

	pObj = CPools::GetObjectPool().GetAt(nExtraTargetEntity4);
	if (!pObj)
	{
		nExtraTargetEntity4 = Object;
		return;
	}

	return;		// Couldn't store this target I'm afraid
}
*/





// Name			:	CWeapon::DoesTargetHaveToBeBroken
// Purpose		:	True if this weapon breaks targetting if target is out of range.
// Parameters	:	
// Returns		:	

bool CPlayerPed::DoesTargetHaveToBeBroken(CEntity *pTargetEnt, CWeapon *pWeapon)
{
	if (!pTargetEnt->m_nFlags.bIsVisible) return true;		// When barrels blow up they turn invisible and should not be targettable.

	float RangeMult = CWeapon::TargetWeaponRangeMultiplier(pTargetEnt, this);

	CVector	Diff = pTargetEnt->GetPosition() - GetPosition();
	float	Length = Diff.Magnitude();
	if(Length > RangeMult * CWeaponInfo::GetWeaponInfo(pWeapon->GetWeaponType(), GetWeaponSkill(pWeapon->GetWeaponType()))->GetTargetRange())
		return true;

	if(pWeapon->GetWeaponType()==WEAPONTYPE_SPRAYCAN && pTargetEnt->GetIsTypeBuilding()
	&& CTagManager::IsTag(pTargetEnt) && CTagManager::GetAlpha(pTargetEnt)==255)
		return true;

	if (!CanIKReachThisTarget(pTargetEnt->GetPosition(), pWeapon, false))
	{
		return true;
	}

	return false;
}

// Name			:	CWeapon::CanIKReachThisTarget
// Purpose		:	Decides whether the ik is able to hit this target with this weapon.
// Parameters	:	
// Returns		:	

bool CPlayerPed::CanIKReachThisTarget(CVector Target, CWeapon *pWeapon, bool bTestYaw)
{


/*
	if (bTestYaw)
	{
		float Yaw = CGeneral::GetRadianAngleBetweenPoints(Target.x, Target.y, this->GetMatrix().tx, this->GetMatrix().ty);
		Yaw -= this->m_fCurrentHeading;
		Yaw = CGeneral::LimitRadianAngle(Yaw);
	
		if (!CWeaponInfo::GetWeaponInfo(pWeapon->GetWeaponType())->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
		{
			if (ABS(Yaw) > PI*0.5f) return false;
		}
	}
*/

		// For the weapons the don't allow aiming with arm we use a 45 degree cone that the target has to be in.
	if (!CWeaponInfo::GetWeaponInfo(pWeapon->GetWeaponType(), GetWeaponSkill(pWeapon->GetWeaponType()))->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
	{
		float	FlatDist = CMaths::Sqrt( (Target.x - this->GetMatrix().tx)*(Target.x - this->GetMatrix().tx) + (Target.y - this->GetMatrix().ty)*(Target.y - this->GetMatrix().ty) );
	
		if ( this->GetMatrix().tz - Target.z > FlatDist) return false;
	}

	return true;
}




// Name			:	KeepAreaAroundPlayerClear
// Purpose		:	When a player is in a cutscene, tries to keep the area around him clear of peds and cars
// Parameters	:	
// Returns		:	

void CPlayerPed::KeepAreaAroundPlayerClear()
{
	// check peds
	
	CEntity** ppNearbyPeds=GetPedIntelligence()->GetNearbyPeds();
	const int N=GetPedIntelligence()->GetMaxNumPedsInRange();
	int i;
	for(i=0;i<N;i++)
	{
		CEntity* pNearbyEntity=ppNearbyPeds[i];
		if(pNearbyEntity)
		{
			ASSERT(pNearbyEntity->GetType()==ENTITY_TYPE_PED);
			CPed* pNearbyPed=(CPed*)pNearbyEntity;
			
			if(pNearbyPed->GetCharCreatedBy()==RANDOM_CHAR && 
			   !pNearbyPed->m_nPedFlags.bInVehicle && //pNearbyPed->GetPedState()!=PED_DRIVING &&
			   pNearbyPed->IsAlive() && //pNearbyPed->GetPedState()!=PED_DEAD && pNearbyPed->GetPedState()!=PED_DIE)
			   !CPedGroups::ms_groups[0].GetGroupMembership()->IsMember(pNearbyPed))
			{
				if(pNearbyPed->GetIsOnScreen() && !pNearbyPed->m_nPedFlags.bKeepTasksAfterCleanUp)
				{							
					//Get the nearby ped to flee the player.
					//Work out first if the ped is already fleeing the player or has already been told to flee the player.
					bool bHasAddedEvent=false;
					if(!bHasAddedEvent)
					{
						CTask* pTask=pNearbyPed->GetPedIntelligence()->FindTaskByType(CTaskTypes::TASK_COMPLEX_SMART_FLEE_ENTITY);
						if(pTask)
						{
							CTaskComplexSmartFleeEntity* pTaskFlee=(CTaskComplexSmartFleeEntity*)pTask;
							if(pTaskFlee->GetFleeEntity()==this)
							{
								bHasAddedEvent=true;
							}
						}
					}
					if(!bHasAddedEvent)
					{
						CEvent* pEvent=pNearbyPed->GetPedIntelligence()->GetEventOfType(CEventTypes::EVENT_SCRIPT_COMMAND);
					   	if(pEvent)
					   	{				   	
						   	CEventScriptCommand* pEventScriptCommand=(CEventScriptCommand*)pEvent;
							if(pEventScriptCommand->GetTask() && pEventScriptCommand->GetTask()->GetTaskType()==CTaskTypes::TASK_COMPLEX_SMART_FLEE_ENTITY)
							{
								bHasAddedEvent=true;
							}
						}
					}
						
					if(!bHasAddedEvent)
					{						   
					   //Make the ped get out of the way by fleeing the player.
						const float fSafeDistance=1000.0f;
						const int iFleeTime=100000;
						CTaskComplexSmartFleeEntity* pTask=new CTaskComplexSmartFleeEntity(this,false,fSafeDistance,iFleeTime);
						pTask->SetMoveState(PEDMOVE_WALK);
						CEventScriptCommand event(CTaskManager::TASK_PRIORITY_PRIMARY,pTask);
						pNearbyPed->GetPedIntelligence()->AddEvent(event);
					}
				}
				else
				{
					pNearbyPed->FlagToDestroyWhenNextProcessed();
				}
			}
		}
	}
	
	/*
	int32 i = 0;
	CVector vecDistance, vecPosition;



	//BuildPedLists();
	for (i = 0; i < m_NumClosePeds; i++)
	{
		if( m_apClosePeds[i]->GetCharCreatedBy() == RANDOM_CHAR && m_apClosePeds[i]->GetPedState()!=PED_DRIVING
		&&	m_apClosePeds[i]->GetPedState()!=PED_DEAD && m_apClosePeds[i]->GetPedState()!=PED_DIE)
		{
			if(m_apClosePeds[i]->GetIsOnScreen())
			{		
				if(m_apClosePeds[i]->m_Objective != NO_OBJ)
				{
					if((m_apClosePeds[i]->GetPedState() == PED_CARJACK)||
					(m_apClosePeds[i]->GetPedState() == PED_ENTER_CAR))
					{
						m_apClosePeds[i]->QuitEnteringCar();
					}
					// use this rather than set idle
					// it will wait till it is safe for the ped to be set to idle
					m_apClosePeds[i]->ClearObjective();
				}
				else
				{
					// if I can't see this ped, delete him
					// otherwise make hime flee away from me
					m_apClosePeds[i]->SetFlee(this,5000);
					m_apClosePeds[i]->m_nPedFlags.bUseSmartSeekAndFlee = true;
					m_apClosePeds[i]->m_NextWanderNode.SetEmpty();
					m_apClosePeds[i]->SetMoveState(PEDMOVE_WALK);
				}
			}
			else
			{
				m_apClosePeds[i]->FlagToDestroyWhenNextProcessed();
			}
		}
	}
	*/

	CEntity *ppResults[8]; // whatever.. temp list
	CVehicle *pVehicle;
	int16 Num;
	CVector vecDistance, vecPosition;

	
	if(m_nPedFlags.bInVehicle && m_pMyVehicle)
		vecPosition = m_pMyVehicle->GetPosition();
	else
		vecPosition = GetPosition();
	
	// check for threat peds in cars
	CWorld::FindObjectsInRange(CVector(GetPosition()), 15.0f, true, &Num, 6, ppResults, 0, 1, 0, 0, 0);	// Just Cars we're interested in
	// cars in range stored in ppResults

	for(i=0; i<Num; i++) // find cars close by
	{
		pVehicle = (CVehicle *)ppResults[i];

		if(pVehicle->GetVehicleCreatedBy() != MISSION_VEHICLE && 
		pVehicle->GetStatus() != STATUS_PLAYER &&
		pVehicle->GetStatus() != STATUS_PLAYER_DISABLED)
		{
			vecDistance = pVehicle->GetPosition() - vecPosition;		
			
			// cars are far enough away, stop them moving
			if(vecDistance.MagnitudeSqr() > 5.0f*5.0f)
			{
				// Tell cars to stop for 5 secs so that they will automatically start driving again after a while
				pVehicle->AutoPilot.TempAction = CAutoPilot::TEMPACT_WAIT;
				pVehicle->AutoPilot.TempActionFinish = CTimer::GetTimeInMilliseconds() + 5000;
			}
			else
			{
				// Depending on whether the car is facing away or towards the player we will reverse or go foward a bit
				if ( (pVehicle->GetMatrix().GetForward().x * (vecPosition.x - pVehicle->GetPosition().x)) +
					 (pVehicle->GetMatrix().GetForward().y * (vecPosition.y - pVehicle->GetPosition().y)) > 0.0f)
				{	// reverse away
					pVehicle->AutoPilot.TempAction = CAutoPilot::TEMPACT_REVERSE;
					pVehicle->AutoPilot.TempActionFinish = CTimer::GetTimeInMilliseconds() + 2000;
				}
				else
				{	// go forward
					pVehicle->AutoPilot.TempAction = CAutoPilot::TEMPACT_GOFORWARD;
					pVehicle->AutoPilot.TempActionFinish = CTimer::GetTimeInMilliseconds() + 2000;
				}

			}
			CCarCtrl::PossiblyRemoveVehicle(pVehicle);
		}
		
	}


}

CPlayerInfo *CPlayerPed::GetPlayerInfoForThisPlayerPed(void)
{
	Int32 Loop;
#ifdef GTA_NETWORK

	if (CGameNet::NetWorkStatus != NETSTAT_SINGLEPLAYER)
	{
		for (Loop = 0; Loop < MAX_NUM_PLAYERS; Loop++)
		{
			if (CWorld::Players[Loop].bInUse && CWorld::Players[Loop].pPed == this)
			{
				return (&CWorld::Players[Loop]);
			}
		}
	}
#else
	for (Loop = 0; Loop < NUM_COOPERATIVE_PLAYERS; Loop++)
	{
		if (CWorld::Players[Loop].pPed == this)
		{
			return (&CWorld::Players[Loop]);
		}
	}
#endif
	
	ASSERTMSG(0, "CPlayerPed::GetPlayerInfoForThisPlayerPed - couldn't find player ped");
	return NULL;
}

void CPlayerPed::DoStuffToGoOnFire()
{
	if (GetPedState() == PED_SNIPER_MODE)
	{
		TheCamera.ClearPlayerWeaponMode();
	}
}


void CPlayerPed::AnnoyPlayerPed(bool bUsingVees)
{
	if(m_pPedStats->m_nTemper < 52)
		m_pPedStats->m_nTemper += 1;
	else if(bUsingVees && m_pPedStats->m_nTemper < 55)
		m_pPedStats->m_nTemper += 1;
	else if(bUsingVees)
		m_pPedStats->m_nTemper = 46;
}

void CPlayerPed::ClearAdrenaline()
{
	if(GetPlayerData()->m_bAdrenaline && GetPlayerData()->m_AdrenalineEndTime)
	{
		// don't clear flag directly, it will get cleared in SetRealMoveAnim once anims have been tidyed up
		GetPlayerData()->m_AdrenalineEndTime = 0;
		CTimer::SetTimeScale(1.0f);
	}
}


/*
// find up to NUM_ATTACK_POINTS positions around the ped, which would be good
// to shoot from (behind lampposts, walls, objects etc)
// if no objects are near, set the points equally around the player so that
// pusuing police who use the points stay out of each others way when they all attack at once
// only call this every so often, or police might never reach the points before they change

#define DISTANCE_AWAY_FROM_PED	(3.0f)
#define HIDE_RADIUS_AROUND_PLAYER	(18.0f)

void CPlayerPed::FindNewAttackPoints()
{
	CEntity *ppResults[8]; // whatever.. temp list
	int16 nNum,i;
	CVector vecNewPos, vecAddPos, CollPoint;//, CenterPoint;
	CEntity *pEntity;
	float fAngle;
	CRect rect;
	CPed *pPed;


	for(i=0; i<NUM_ATTACK_POINTS; i++) // find close car
	{
		if(m_pEntUsingAttackPoint[i])
		{
			pPed = ((CPed *)m_pEntUsingAttackPoint[i]);
			
			if(pPed->GetPedState() == PED_DEAD || pPed->m_pObjectivePed != this)
			{
				m_vecAttackPoints[i].x = 0.0f;
				m_vecAttackPoints[i].y = 0.0f;
				m_vecAttackPoints[i].z = 0.0f;
				m_pEntUsingAttackPoint[i] = NULL;
			}
		}
		else
		{
			m_vecAttackPoints[i].x = 0.0f;
			m_vecAttackPoints[i].y = 0.0f;
			m_vecAttackPoints[i].z = 0.0f;
		}
	}


	// initialise with rough points in a clear area around ped

	fAngle = 0;
////
//	for(i = 0; i<NUM_ATTACK_POINTS; i++)
//	{
//	
//		vecNewPos.x=GetPosition().x + (DISTANCE_AWAY_FROM_PED*CMaths::Cos(fAngle));
//		vecNewPos.y=GetPosition().y + (DISTANCE_AWAY_FROM_PED*CMaths::Sin(fAngle));
//		vecNewPos.z=GetPosition().z;
//
//		CPedPlacement::FindZCoorForPed(&vecNewPos);
//
//		if(CPedPlacement::IsPositionClearForPed(&vecNewPos))
//		{
//			m_vecAttackPoints[i] = vecNewPos;
//		}
//	
//		fAngle += ATTACK_POINT_ANGLE;
//	
//	}
///


	uint8 nNumNodes = 0;
	CBaseModelInfo* pModelInfo = NULL;
	C2dEffect *p2dEffect = NULL;


	CWorld::FindObjectsInRange(CVector(GetPosition()), HIDE_RADIUS_AROUND_PLAYER, true, &nNum, NUM_ATTACK_POINTS, ppResults, 1, 0, 0, 1, 0);	// Just Objects we're interested in (but have to check for small buildings too- as trees are buildings)

// new stuff for when the hide nodes go in
////
//	for(i=0; i<nNum; i++) // find close object
//	{
//		pEntity = ppResults[i];
//		pModelInfo = CModelInfo::GetModelInfo(pEntity->m_nModelIndex);
//		
//		if( (nNumNodes = pModelInfo->GetNum2dEffects()) )
//		{
//			for(j=0; j<nNumNodes; j++)
//			{
//				p2dEffect = pModelInfo->Get2dEffect(j);
//				
//				// currently using the (previously unused Ice cream setting for look-at points)
//				// could change this to LT_HIDEPOINT or something
//				if(p2dEffect->m_type == ET_LOOKATPOINT && p2dEffect->attr.k.m_type == LT_ICECREAM)
//				{
//					// work out the point's position at the base of the model which we will hide behind
//					CollPoint = pEntity->GetPosition();
//					CollPoint.x += p2dEffect->m_posn.x * pEntity->m_mat.GetForward().x + p2dEffect->m_posn.x * pEntity->m_mat.GetRight().x;
//					CollPoint.y += p2dEffect->m_posn.y * pEntity->m_mat.GetForward().y + p2dEffect->m_posn.x * pEntity->m_mat.GetRight().y;
//				
//					// work out point behind it
//					vecNewPos = GetPosition()-CollPoint;
//					vecAddPos = vecNewPos;
//					vecAddPos.Normalise();
//					vecAddPos *=(2.0f);// can use the radius specified by the hide point attribute
//					vecNewPos = CollPoint-vecAddPos;
//
//					CPedPlacement::FindZCoorForPed(&vecNewPos);
//					// is this point ok?
//					if(CPedPlacement::IsPositionClearForPed(&vecNewPos))
//					{
//						m_vecAttackPoints[i] = vecNewPos;
//					}
//					
//				}
//				else // wrong type of node (need a hide one)
//				{
//					continue;
//				}
//			}
//		}
//		else// dont want to hide behind this (no nodes, so no potential hide points specified)
//		{
//			continue;
//		}		
//
//	}		
///
// old system, checks specific models to determine where the base is
// this works for Liberty, but system above is new for Miami

CVector CenterPoint;

	for(i=0; i<nNum; i++) // find close object
	{
		pEntity = ppResults[i];
		if(	(pEntity->GetIsTypeObject() && ((CObject *)pEntity)->m_nCollisionSpecial!=OB_COL_FENCE)||
			( ((CAtomicModelInfo*)CModelInfo::GetModelInfo(pEntity->GetModelIndex()))->GetIsAnyTree()) )
		{
			if(pEntity->GetModelIndex() == MI_TRAFFICLIGHTS)// these are liberty city models, but still used
			{
						CenterPoint.x = 2.957f;
						CenterPoint.y = 0.147f;
			}
			else
			if(pEntity->GetModelIndex() == MI_SINGLESTREETLIGHTS1)
			{
						CenterPoint.x = 0.744f;
						CenterPoint.y = 0.0f;
			}
			else
			if(pEntity->GetModelIndex() == MI_SINGLESTREETLIGHTS2)
			{
						CenterPoint.x = 0.043f;
						CenterPoint.y = 0.0f;
			}
			else
			if(pEntity->GetModelIndex() == MI_SINGLESTREETLIGHTS3)
			{
						CenterPoint.x = 1.143f;
						CenterPoint.y = 0.145f;
			}
			else
			if(pEntity->GetModelIndex() == MI_DOUBLESTREETLIGHTS)
			{
						CenterPoint.x = 0.744f;
						CenterPoint.y = 0.0f;
			}
			else
			if(pEntity->GetModelIndex() == MI_LAMPPOST1) 
			{
				CenterPoint.x = 0.744f;
				CenterPoint.y = 0.0f;
			}
			else
			if(pEntity->GetModelIndex() == MI_TRAFFICLIGHT01) 
			{
				CenterPoint.x = 2.957f;
				CenterPoint.y = 0.147f;
			}
			else
			if(pEntity->GetModelIndex() == MI_LITTLEHA_POLICE) 
			{
				CenterPoint.x = 0.0f;
				CenterPoint.y = 0.0f;
			}
			else
			if(pEntity->GetModelIndex() == MI_PARKBENCH) 
			{
				CenterPoint.x = 0.0f;
				CenterPoint.y = 0.0f;
			}
			else
			if ( ((CAtomicModelInfo*)CModelInfo::GetModelInfo(pEntity->GetModelIndex()))->GetIsAnyTree())
			{
				CenterPoint.x = 0.0f;
				CenterPoint.y = 0.0f;
			}
			else// dont want to hide behind this
			{
				continue;
			}		

			CollPoint = pEntity->GetPosition();
			CollPoint.x += CenterPoint.y * pEntity->GetMatrix().GetForward().x + CenterPoint.x * pEntity->GetMatrix().GetRight().x;
			CollPoint.y += CenterPoint.y * pEntity->GetMatrix().GetForward().y + CenterPoint.x * pEntity->GetMatrix().GetRight().y;
		
			// work out point behind it
			vecNewPos = GetPosition()-CollPoint;
			vecAddPos = vecNewPos;
			vecAddPos.Normalise();
			vecAddPos *=(2.0f);// can use the radius specified by the hide point attribute
			vecNewPos = CollPoint-vecAddPos;

			CPedPlacement::FindZCoorForPed(&vecNewPos);
			// is this point ok?
			if(CPedPlacement::IsPositionClearForPed(vecNewPos))
			{
				m_vecAttackPoints[i] = vecNewPos;
			}
		}
	}
	


	
//#ifdef DEBUG
	// draw the points (THIS IS ACTUALLY VERY SLOW)
//	for(i = 0; i<NUM_ATTACK_POINTS; i++)
//	{
//		CDebug::DebugLine3D(m_vecAttackPoints[i].x, m_vecAttackPoints[i].y, m_vecAttackPoints[i].z,
//		m_vecAttackPoints[i].x, m_vecAttackPoints[i].y, m_vecAttackPoints[i].z+10.0f,
//		0x00ff00ff, 0x00ff00ff);
//		CDebug::DebugLine3D(m_vecAttackPoints[i].x-1.0f, m_vecAttackPoints[i].y, m_vecAttackPoints[i].z+1.0f,
//		m_vecAttackPoints[i].x+1.0f, m_vecAttackPoints[i].y, m_vecAttackPoints[i].z+1.0f,
//		0x00ff00ff, 0x00ff00ff);
//		CDebug::DebugLine3D(m_vecAttackPoints[i].x, m_vecAttackPoints[i].y-1.0f, m_vecAttackPoints[i].z+1.0f,
//		m_vecAttackPoints[i].x, m_vecAttackPoints[i].y+1.0f, m_vecAttackPoints[i].z+1.0f,
//		0x00ff00ff, 0x00ff00ff);
//	}
//#endif
	
}
*/

/*
void CPlayerPed::UpdateMeleeAttackers()
{
	int16 i = -1;
	CVector vecAttackPos;
	
	// check for attack points around the player that may be unusable because of
	// obstacles on the map (e.g. in a building or car)
	if( ((CTimer::m_FrameCounter + RandomSeed + 7) &3) == 0)
	{
		GetMeleeAttackCoords(vecAttackPos, GetPlayerData()->m_nMeleeAttackCheckCol, 2.0f);
		
		// if no line of sight from player to point, flag point as unusable
		if(!CWorld::GetIsLineOfSightClear(GetPosition(), vecAttackPos, true, true, false, true, false, false, false)
		||  CWorld::TestSphereAgainstWorld(vecAttackPos, 0.4f, GetPlayerData()->m_pMeleeAttackers[GetPlayerData()->m_nMeleeAttackCheckCol], true, true, false, true, false, false) )
			GetPlayerData()->m_pMeleeAttackers[GetPlayerData()->m_nMeleeAttackCheckCol] = this;
		// else clear, so make sure flagged as available
		else if(GetPlayerData()->m_pMeleeAttackers[GetPlayerData()->m_nMeleeAttackCheckCol] == this)
			GetPlayerData()->m_pMeleeAttackers[GetPlayerData()->m_nMeleeAttackCheckCol] = NULL;
			
		if(++GetPlayerData()->m_nMeleeAttackCheckCol >= NUM_MELEE_ATTACK_POINTS)
			GetPlayerData()->m_nMeleeAttackCheckCol = 0;
			
			
		INC_PEDAI_LOS_COUNTER;
	}

// commented out for testing- will put the frame thing back in to speed it up
//	if( ((CTimer::m_FrameCounter + RandomSeed + 3) &15) )
//		return;

	// check if attackers are still valid
	for(i=0; i<NUM_MELEE_ATTACK_POINTS; i++)
	{
		if(GetPlayerData()->m_pMeleeAttackers[i] && GetPlayerData()->m_pMeleeAttackers[i]!=this)
		{
			CTaskComplexKillPedOnFoot* pTaskKill=
				(CTaskComplexKillPedOnFoot*)
					GetPlayerData()->m_pMeleeAttackers[i]->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_KILL_PED_ON_FOOT);
			if(0==pTaskKill || pTaskKill->GetTargetPed()!=this)
			{
				GetPlayerData()->m_pMeleeAttackers[i] = NULL;				
			}
			else
			{
				GetMeleeAttackCoords(vecAttackPos, i, 2.0f);
				if( (vecAttackPos - GetPosition()).MagnitudeSqr() > 3.5f*3.5f )
				{
					GetPlayerData()->m_pMeleeAttackers[i] = NULL;
				}
			}
			
		}
	}
	
	// check here if all attack points happen to be blocked, let attackers ignore pts and attack straight ahead
	if(GetPlayerData()->m_pMeleeAttackers[0]==this && GetPlayerData()->m_pMeleeAttackers[1]==this && GetPlayerData()->m_pMeleeAttackers[2]==this
	&& GetPlayerData()->m_pMeleeAttackers[2]==this && GetPlayerData()->m_pMeleeAttackers[4]==this && GetPlayerData()->m_pMeleeAttackers[5]==this)
		GetPlayerData()->bAllMeleeAttackPtsBlocked = true;
	else
		GetPlayerData()->bAllMeleeAttackPtsBlocked = false;
}
*/
/*
int16 CPlayerPed::FindMeleeAttackPoint(CPed *pPed, CVector &vecDelta, uint32 &nLastAttackTime)
{
	int16 i, nAttackPointIndex = -1;
	bool bFreeAttackPoints = false;
		
	nLastAttackTime = 0;
	// search if we are in the attacking list already, or look for free spaces
	for(i=0; i<NUM_MELEE_ATTACK_POINTS; i++)
	{
		if( GetPlayerData()->m_pMeleeAttackers[i] == NULL )	// free (no ped at this point)
			bFreeAttackPoints = true;
		else if( GetPlayerData()->m_pMeleeAttackers[i] == pPed )	// the ped we're looking for!
			nAttackPointIndex = i;
		else if( GetPlayerData()->m_pMeleeAttackers[i]->m_nAttackTimer > nLastAttackTime )
			nLastAttackTime = GetPlayerData()->m_pMeleeAttackers[i]->m_nAttackTimer;
	}
	
	// ok ped's not on list but there are free points - look for closest
	if( nAttackPointIndex == -1 && bFreeAttackPoints )
	{
		float fDirectionToMe = CMaths::ATan2(-vecDelta.x, -vecDelta.y);
		fDirectionToMe += 0.5f*MELEE_ATTACK_POINT_ANGLE;
		if(fDirectionToMe < 0.0f)
			fDirectionToMe += TWO_PI;
		
		int16 nBestAttackPoint = CMaths::Floor( fDirectionToMe / MELEE_ATTACK_POINT_ANGLE );
		// hopefully we can go to our first pick of attack points
		if( GetPlayerData()->m_pMeleeAttackers[nBestAttackPoint] == NULL )
			nAttackPointIndex = nBestAttackPoint;
		// nope need to look for best free one
		else
		{
			int16 nNextBestAttackPoint = -99;
			for(i=0; i<NUM_MELEE_ATTACK_POINTS; i++)
			{
				if( GetPlayerData()->m_pMeleeAttackers[i] == NULL 
				&& CMaths::Abs(i-nBestAttackPoint) < CMaths::Abs(nNextBestAttackPoint-nBestAttackPoint))
				{
					nNextBestAttackPoint = i;
				}
			}
			
			if(nNextBestAttackPoint > 0)
			{
				nAttackPointIndex = nNextBestAttackPoint;
			}
		}
		
		// we found a free attack point so need to set that in the playerped array
		if(nAttackPointIndex != -1)
		{
			GetPlayerData()->m_pMeleeAttackers[nAttackPointIndex] = pPed;
			REGREF(pPed, (CEntity**)&(GetPlayerData()->m_pMeleeAttackers[nAttackPointIndex]));
			
			// set up timer so we don't attack until it's our turn
			if(nLastAttackTime > CTimer::GetTimeInMilliseconds())
				pPed->m_nAttackTimer = nLastAttackTime + CGeneral::GetRandomNumberInRange(1000, 2000);
			else
				pPed->m_nAttackTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(500, 1000);
			
		}
	}
	
	return nAttackPointIndex;
}
*/
/*
#define MELEE_ATTACK_POINT_X (0.866025403f)	// sqrt(3/4)
#define MELEE_ATTACK_POINT_Y (0.5f)			// 1/2
#define MELEE_ATTACK_ANGLE   (PI/3.0f)

void CPlayerPed::GetMeleeAttackCoords(CVector &vecResult, int8 nAttackPoint, float fRange)
{
	ASSERT( nAttackPoint > -1 && nAttackPoint < NUM_MELEE_ATTACK_POINTS);
	vecResult = GetPosition();
	switch(nAttackPoint)
	{
		case 0:
			vecResult.y += fRange;
			break;
			
		case 1:
			vecResult.x += fRange*MELEE_ATTACK_POINT_X;
			vecResult.y += fRange*MELEE_ATTACK_POINT_Y;
			break;
			
		case 2:
			vecResult.x += fRange*MELEE_ATTACK_POINT_X;
			vecResult.y -= fRange*MELEE_ATTACK_POINT_Y;
			break;
			
		case 3:
			vecResult.y -= fRange;
			break;
			
		case 4:
			vecResult.x -= fRange*MELEE_ATTACK_POINT_X;
			vecResult.y -= fRange*MELEE_ATTACK_POINT_Y;
			break;
			
		case 5:
			vecResult.x -= fRange*MELEE_ATTACK_POINT_X;
			vecResult.y += fRange*MELEE_ATTACK_POINT_Y;
			break;
	}
}
*/
/*
float CPlayerPed::GetMeleeAttackAngle(int8 nAttackPoint)
{
	return (float)(nAttackPoint*MELEE_ATTACK_ANGLE);
}
*/
/*
void CPlayerPed::RemovePedFromMeleeList(CPed *pPed)
{
	for(int16 i=0; i<NUM_MELEE_ATTACK_POINTS; i++)
	{
		if(GetPlayerData()->m_pMeleeAttackers[i] == pPed)
		{
			GetPlayerData()->m_pMeleeAttackers[i]=NULL;
			// reset attack timer so ped doesn't get confused when they get a new attack point
			pPed->m_nAttackTimer = 0;
			// hopefully only in the list once!
			return;
		}
	}
}
*/
/*
void CPlayerPed::AdvanceMeleeListAttackTimers(CPed *pPed)
{
	bool bFoundTime = false;
	uint32 nSmallestTimer = -1;
	for(int16 i=0; i<NUM_MELEE_ATTACK_POINTS; i++)
	{
		if(GetPlayerData()->m_pMeleeAttackers[i] && GetPlayerData()->m_pMeleeAttackers[i]!=pPed
		&& GetPlayerData()->m_pMeleeAttackers[i]->m_nAttackTimer < nSmallestTimer)
		{
			nSmallestTimer = GetPlayerData()->m_pMeleeAttackers[i]->m_nAttackTimer;
			bFoundTime = true;
		}
	}

	if(bFoundTime)
	{
		if(nSmallestTimer > pPed->m_nAttackTimer)
		{
			if( (nSmallestTimer - pPed->m_nAttackTimer) > 800)
				return;
			else
				nSmallestTimer = 800 - (nSmallestTimer - pPed->m_nAttackTimer);
		}
		else
			nSmallestTimer = 800 + (pPed->m_nAttackTimer - nSmallestTimer);
		
		for(int16 i=0; i<NUM_MELEE_ATTACK_POINTS; i++)
		{
			if(GetPlayerData()->m_pMeleeAttackers[i] && GetPlayerData()->m_pMeleeAttackers[i]!=pPed)
			{
				GetPlayerData()->m_pMeleeAttackers[i]->m_nAttackTimer += nSmallestTimer;
			}
		}
	}
}
*/
/*
void CPlayerPed::SetNearbyPedsToInteractWithPlayer()
{
	// only want to do this bit of code if Prostitutes are allowed in this version
	if(!CLocalisation::Prostitutes())
		return;

    int i;
    for(i=0;i<m_NumClosePeds;i++)
    {
        CPed* pClosePed=m_apClosePeds[i];
        if(pClosePed)
        {
            if( 
                //(pClosePed->CharCreatedBy != MISSION_CHAR)&&
			    (pClosePed->m_nObjectiveTimer < CTimer::GetTimeInMilliseconds())&&
			    (!CTheScripts::IsPlayerOnAMission())
			  )
			{
                const int iModelIndex=pClosePed->GetModelIndex();
                
                if(CPopulation::CanSolicitPlayerOnFoot(iModelIndex))
                {
                    CVector vDirToPed = pClosePed->GetPosition() - GetPosition();
                    const CVector& vClosePedDir=pClosePed->m_mat.GetForward();
                    const CVector& vDir=m_mat.GetForward();
                    CVector vBA=GetPosition()-pClosePed->GetPosition();
                    vBA.Normalise();
                    const float f1=DotProduct(vBA,vClosePedDir);
                    const float f2=DotProduct(vClosePedDir,vDir);
                    if((DotProduct(vBA,vClosePedDir)>0.707)&&(DotProduct(vClosePedDir,vDir)<-0.707))               
                    {
                        if(vDirToPed.MagnitudeSqr() < SOLICIT_FOOT_DISTANCE*SOLICIT_FOOT_DISTANCE)
                        {        
                            if(NO_OBJ==pClosePed->m_Objective)
                            {            
                                pClosePed->SetObjective(SOLICIT_FOOT,this);
                                pClosePed->m_nObjectiveTimer=CTimer::GetTimeInMilliseconds()+SOLICIT_PAUSE_TIME;
                                pClosePed->Say(AE_PED_SOLICIT);
                            }
                        }
                    }
                }
                else if(CPopulation::CanSolicitPlayerInCar(iModelIndex))
                {
                    if(m_nPedFlags.bInVehicle&&m_pMyVehicle)
                    {
                        if(m_pMyVehicle->IsVehicleNormal() && m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR)
	        		    {			
					        CVector vDirToVehicle = pClosePed->GetPosition()-m_pMyVehicle->GetPosition();
					        if(vDirToVehicle.MagnitudeSqr() < SOLICIT_VEHICLE_DISTANCE*SOLICIT_VEHICLE_DISTANCE)
					        {
						        // make sure there is at least one door available to get in
						        if(m_pMyVehicle->IsRoomForPedToLeaveCar(CAR_DOOR_LF, NULL))
						        {
						            if(NO_OBJ==pClosePed->m_Objective)
						            {
                                        pClosePed->SetObjective(SOLICIT_VEHICLE,m_pMyVehicle);
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
*/    

void CPlayerPed::SetPlayerMoveBlendRatio(CVector *pvecTarget)
{
	if(pvecTarget)
	{
		CVector2D vec2DDiff = CVector2D(pvecTarget->x-GetPosition().x, pvecTarget->y-GetPosition().y);

		GetPlayerData()->m_moveBlendRatio = vec2DDiff.Magnitude() * 2.0f;
		if(GetPlayerData()->m_moveBlendRatio > 2.0f)
		{
			GetPlayerData()->m_moveBlendRatio = 2.0f;
		}
	}
	else
	{
		switch(GetMoveState())
		{
			case PEDMOVE_SPRINT:
				GetPlayerData()->m_moveBlendRatio = 2.5f;break;
			case PEDMOVE_RUN:
				GetPlayerData()->m_moveBlendRatio = 1.8f;break;
			case PEDMOVE_WALK:
				GetPlayerData()->m_moveBlendRatio = 1.0f;break;
			default:
				GetPlayerData()->m_moveBlendRatio = 0.0f;
		}
	}

	SetRealMoveAnim();
}

//
// Name			:	ProcessVendingMachines
// Purpose		:	Now and then look for objects that are vending machines.
//
#define PRICEJUNKFOOD (2)
#define HEALTHJUNKFOOD (25)
#define FATJUNKFOOD (10)

/*
void CPlayerPed::ProcessVendingMachines()
{
	if ((CTimer::m_FrameCounter & 15) != 12) return;

	if (GetPedIntelligence()->GetTaskPrimary())
	{
static	Int32 TT = GetPedIntelligence()->GetTaskPrimary()->GetTaskType();
	}
	
//	if (GetPedIntelligence()->GetTaskPrimary()->GetTaskType() != CTaskTypes::TASK_SIMPLE_PLAYER_ON_FOOT)
//	{
		
//	}

			CObjectPool& 	pool = CPools::GetObjectPool();
			CObject* 		pObject;
			CObject* 		pSuitableObject = NULL;
			int32 			i=pool.GetSize();
			CVector			Coors;
			
			while(i--)
			{
				pObject = pool.GetSlot(i);
				if(pObject)
				{
					if (  pObject->GetMatrix().GetUp().z > 0.8f)
					{
						CBaseModelInfo *pBaseModel;
						pBaseModel = CModelInfo::GetModelInfo(pObject->GetModelIndex());
						for (Int32 C = 0; C < pBaseModel->GetNum2dEffects(); C++)
						{
							CVector		Pos;
							C2dEffect	*pEffect = pBaseModel->Get2dEffect(C);
		
							switch (pEffect->m_type)
							{
								case ET_VENDINGMACHINE:
									Coors = pObject->GetRwMatrix() * pEffect->m_posn;

									if ((Coors - GetPosition()).Magnitude() < 1.0f)
									{
										pSuitableObject = pObject;
									}
									break;

							}
						}
					}
				}	
			}

	if (GetPlayerData()->m_JustBeenSnacking)
	{
		if (!pSuitableObject)
		{
			GetPlayerData()->m_JustBeenSnacking = false;
		}
	}
	else
	{
		if (pSuitableObject)
		{
			GetPlayerData()->m_JustBeenSnacking = true;

			Int32 PlayerSlot = CWorld::FindPlayerSlotWithPedPointer(this);
			// Take some money off.
			if (CWorld::Players[PlayerSlot].Score >= PRICEJUNKFOOD)
			{
				CWorld::Players[PlayerSlot].Score -= PRICEJUNKFOOD;
				CStats::IncrementStat(FAT, FATJUNKFOOD);
				CWorld::Players[PlayerSlot].AddHealth(HEALTHJUNKFOOD);
			}
		}
	}
}
*/


//
// Name			:	FindPedToAttack
// Purpose		:	Go through all the peds and find a good ped to attack.
//
CPed *CPlayerPed::FindPedToAttack()
{
	CPedPool& pool = CPools::GetPedPool();
	CPed* 	pTargetPed = NULL;
	float	TargetDist = 99999.9f;
	int32 i=pool.GetSize();
	CVector	CoorsPoint;
	CVector	Point1 = FindPlayerCoors();
	CVector Point2 = Point1 + TheCamera.GetMatrix().GetForward() * 100.0f;
	Point1.z = Point2.z = 0.0f;

		// We test the distance of each ped to the line from the player ped in the direction
		// of the camera.

	while(i--)
	{
		CPed *pPed = pool.GetSlot(i);
		if(pPed && !pPed->IsPlayer() && pPed->IsAlive() && !CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->IsMember(pPed)
			&& pPed->GetPedType() != PEDTYPE_PLAYER_GANG)
		{
			CoorsPoint = pPed->GetPosition();
			CoorsPoint.z = 0.0f;

			float Dist = CCollision::DistToLine( &Point1, &Point2, &CoorsPoint);
			float RealDist = (CoorsPoint - Point1).Magnitude2D();
			if (RealDist > 20.0f) Dist += (RealDist - 20.0f) * 0.2f;

			// Gang members get their distance slashed (are more likely to get targetted)
			if (pPed->GetPedType() >= PEDTYPE_GANG1 && pPed->GetPedType() <= PEDTYPE_GANG10)
			{
				Dist = MAX(Dist * 0.5f - 2.0f, 0.0f);
			}

			if (Dist < TargetDist)
			{
				TargetDist = Dist;
				pTargetPed = pPed;
			}
		}	
	}
	return (pTargetPed);
}

//
// Name			:	ProcessGroupBehaviour
// Purpose		:	Process everything having to do with getting the player group to attack targets
//

// These variables have to be moved to the player structure
//enum { GROUPATT_STAYINGBEHIND = 0, GROUPATT_FOLLOWINGCLOSELY };
//static	UInt8	m_GroupAttackStatus = GROUPATT_FOLLOWINGCLOSELY;

void CPlayerPed::ProcessGroupBehaviour(CPad *pPad)
{
	//I've removed this line to ensure that the gather command to the group can be processed
	//even when the group stuff is disabled.  In the rest of the function each group command/say() function/etc
	//is only processed if !m_GroupStuffDisabled.  The exception is the function 
	//TellGroupToStartFollowingPlayer(true,true) that remains active all the time and is independent of 
	//the flag m_GroupStuffDisabled.  After the line that contains TellGroupToStartFollowingPlayer the function can 
	//return if !m_GroupStuffDisabled  - GY 03/08/04.	
	//if (GetPlayerData()->m_GroupStuffDisabled) return;

	CEntity *pTargetEnt = GetWeaponLockOnTarget();
#ifdef GTA_PC
	if(TheCamera.m_bUseMouse3rdPerson && !pTargetEnt)
		pTargetEnt = m_pMouseLockOnRecruitPed;
#endif

	// ATTACK command.
#define MILLISFORLONGPRESS (1200)
	if (!FindPlayerVehicle())	// Only control group when on foot.
	{
		if (pPad->GetGroupControlForward())
		{
			GetPlayerData()->m_DPadUpPressedInMilliseconds+=CTimer::GetTimeElapsedInMillisecondsNonClipped();
			if (GetPlayerData()->m_DPadUpPressedInMilliseconds == MILLISFORLONGPRESS)
			{	
				if (!GetPlayerData()->m_GroupStuffDisabled) //added by GY (see comment above)
				{			
					// Long press of the DPad up button -> Disband the group
					DisbandPlayerGroup();
				}
			}
		}	
		else
		{
			if (GetPlayerData()->m_DPadUpPressedInMilliseconds && GetPlayerData()->m_DPadUpPressedInMilliseconds < MILLISFORLONGPRESS)
			{
				if (pTargetEnt && pTargetEnt->GetIsTypePed() && ((((CPed *)pTargetEnt)->GetPedType() == PEDTYPE_PLAYER_GANG) || CCheat::IsRecruitCheatActive()))
				{
					if (!GetPlayerData()->m_GroupStuffDisabled) //added by GY (see comment above)
					{
						MakeThisPedJoinOurGroup((CPed*)pTargetEnt);
					}
				}
				else
				{
					// Player wants the group members to go into follow mode.
					TellGroupToStartFollowingPlayer(true, true, false);
				}
			}
			GetPlayerData()->m_DPadUpPressedInMilliseconds = 0;
		}
	}
	
	 //added by GY (see comment above)
	if (GetPlayerData()->m_GroupStuffDisabled) return;


	if (!FindPlayerVehicle())	// Only control group when on foot.
	{
		if (pPad->GetGroupControlBack())
		{
			GetPlayerData()->m_DPadDownPressedInMilliseconds+=CTimer::GetTimeElapsedInMillisecondsNonClipped();

			if (GetPlayerData()->m_DPadDownPressedInMilliseconds >= MILLISFORLONGPRESS)
			{	// Long press of the DPad down button -> Disband the group

				DisbandPlayerGroup();
			}			
		}
		else
		{
			if (GetPlayerData()->m_DPadDownPressedInMilliseconds && GetPlayerData()->m_DPadDownPressedInMilliseconds < MILLISFORLONGPRESS)
			{	// Fast press of the DPad down button
				// If we have a ped targetted we are trying to get him to follow the gang
				if (pTargetEnt && pTargetEnt->GetIsTypePed() && ((CPed *)pTargetEnt)->GetPedType() == PEDTYPE_PLAYER_GANG)
				{
					MakeThisPedJoinOurGroup((CPed *)pTargetEnt);
				}
				else
				{		// Get peds to gather and stay behind.
					TellGroupToStartFollowingPlayer(false, true, false);
				}
			}
			GetPlayerData()->m_DPadDownPressedInMilliseconds = 0;
		}
	}

	// If the player has a wanted level we make the cops hated. This will cause the
	// group members to attack the police.
	if ((CTimer::m_FrameCounter & 31) == 6)
	{
		// Go through all the peds in the world and tell the members of the players'
		// gang (GANG2) to hate the player or not depending on the wanted level.
		CPedPool& pool = CPools::GetPedPool();
		CPed* pPed;
		int32 i=pool.GetSize();

		while(i--)
		{
			pPed = pool.GetSlot(i);
			if(pPed && pPed->GetPedType() == PEDTYPE_PLAYER_GANG)
			{
				if (FindPlayerPed()->GetWantedLevel() > WANTED_CLEAN)
				{
					pPed->m_acquaintances.SetAsAcquaintance(ACQUAINTANCE_TYPE_PED_HATE, CPedType::GetPedFlag(PEDTYPE_COP));
				}
				else
				{
					pPed->m_acquaintances.ClearAsAcquaintance(ACQUAINTANCE_TYPE_PED_HATE, CPedType::GetPedFlag(PEDTYPE_COP));
				}
			}
		}
/*
		// leader is last member 
		for (Int32 Member = 0; Member < CPedGroupMembership::MAX_NUM_MEMBERS-1; Member++)
		{
			CPed *pPed = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->GetMember(Member);
			if(pPed)
			{
				if (FindPlayerPed()->GetWantedLevel() > WANTED_CLEAN)
				{
					pPed->m_acquaintances.SetAsAcquaintance(ACQUAINTANCE_TYPE_PED_HATE, CPedType::GetPedFlag(PEDTYPE_COP));
				}
				else
				{
					pPed->m_acquaintances.ClearAsAcquaintance(ACQUAINTANCE_TYPE_PED_HATE, CPedType::GetPedFlag(PEDTYPE_COP));
				}
			}	
		}
*/
	}
}

void CPlayerPed::ForceGroupToAlwaysFollow(bool bOn)
{
	if (bOn)
	{
		GetPlayerData()->m_GroupAlwaysFollow = true;
		TellGroupToStartFollowingPlayer(true, false, true);
	}
	else
	{
		GetPlayerData()->m_GroupAlwaysFollow = false;
	}
}

void CPlayerPed::ForceGroupToNeverFollow(bool bOn)
{
	if (bOn)
	{
		GetPlayerData()->m_GroupNeverFollow = true;
		TellGroupToStartFollowingPlayer(false, false, true);
	}
	else
	{
		GetPlayerData()->m_GroupNeverFollow = false;
	}
}

//
// Name			:	DisbandPlayerGroup
// Purpose		:	dumps the members of the players' group
//

void CPlayerPed::DisbandPlayerGroup()
{
	Int32	NumMembers = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader();

	if (NumMembers > 0)
	{
		if (NumMembers <= 1)
		{
			this->Say(CONTEXT_GLOBAL_ORDER_DISBAND_ONE);
		}
		else
		{
			this->Say(CONTEXT_GLOBAL_ORDER_DISBAND_MANY);
		}
	}

	// First remove the radar blips of the peds
/*	This now happens in the RemoveMember code.
	for (Int32 C = 0; C < CPedGroupMembership::MAX_NUM_MEMBERS; C++)
	{
		CPed *pPed = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->GetMember(C);
		if (pPed && pPed->GetCharCreatedBy() != MISSION_CHAR)
		{
			if (m_nPedFlags.bClearRadarBlipOnDeath)
			{
				CRadar::ClearBlipForEntity(BLIPTYPE_CHAR, CPools::GetPedPool().GetIndex(this));
				m_nPedFlags.bClearRadarBlipOnDeath = false;
			}
		}
	}
*/

	CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->RemoveAllFollowers(true);
}

//
// Name			:	MakeThisPedJoinOurGroup
// Purpose		:	If the player has a target selected and presses the right button we will
//					try to add this ped to the players' group.
//

void CPlayerPed::MakeThisPedJoinOurGroup(CPed *pCandidate)
{
	//CPed *pCandidate = (CPed *)GetWeaponLockOnTarget();
	ASSERT(pCandidate);

	// If this ped is drugged up he can't be recruited.
	if (pCandidate->m_nPedFlags.bDruggedUp)
	{
		this->Say(CONTEXT_GLOBAL_DRUGGED_IGNORE);
		return;
	}

	// If this ped is attacking the player we can't recruit him either.
//	CTask *pActiveTask = pCandidate->GetPedIntelligence()->GetTaskManager()->GetActiveTask();
	CTask *pActiveTask = pCandidate->GetPedIntelligence()->GetTaskManager()->FindActiveTaskByType(CTaskTypes::TASK_COMPLEX_KILL_PED_ON_FOOT);
	if (pActiveTask)
	{
		return;
	}

			// Make sure this guy belongs to the players gang type.
	if ((pCandidate->GetPedType() == PEDTYPE_PLAYER_GANG || CCheat::IsRecruitCheatActive()) && (!CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->IsMember(pCandidate)))
	{
		// Go into gangbanging voice mood
		CAEPedSpeechAudioEntity::SetCJMood(-1, 10000, TRUE);
		this->Say(CONTEXT_GLOBAL_JOIN_ME_ASK, 0, 1.0f, TRUE);		// Player invites gang dude to join

		Int32 NumThatCanJoin = CStats::FindMaxNumberOfGroupMembers();
		NumThatCanJoin = MIN(NumThatCanJoin, this->GetPlayerData()->m_nScriptLimitToGangSize);
		if (CStats::GetStatValue(CITIES_PASSED) == 1 || CStats::GetStatValue(CITIES_PASSED) == 2)
		{
			NumThatCanJoin = 0;
		}

		if ((CCheat::IsRecruitCheatActive() && CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader() < CPedGroupMembership::LEADER) ||
			(CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader() < NumThatCanJoin &&
			CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader() < NumThatCanJoin ))
		{		// There is room for an additional member.
					
			// Remove this guy from any group he might be in at the moment.
			CPedGroup* pCandidatesGroup=CPedGroups::GetPedsGroup(pCandidate);
			if(pCandidatesGroup)
			{
				pCandidatesGroup->GetGroupMembership()->RemoveMember(pCandidate);
			}
			ASSERT(0==CPedGroups::GetPedsGroup(pCandidate));
								
			CEventScriptCommand event(CTaskManager::TASK_PRIORITY_PRIMARY,new CTaskComplexBeInGroup(FindPlayerPed()->GetPlayerData()->m_PlayerGroup,false));
			pCandidate->GetPedIntelligence()->AddEvent(event);
			CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->AddFollower(pCandidate);
			CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].Process();

			pCandidate->GiveWeaponWhenJoiningGang();

			CEventNewGangMember* pNewGangMemberEvent=new CEventNewGangMember(pCandidate);
			CEventGroupEvent groupEvent(this,pNewGangMemberEvent);
			CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupIntelligence()->AddEvent(groupEvent);

			// this was so your buddies can swim along behind you without drowning
			// but think we're gonna force it for peds in the players group all the time so it's not required
			pCandidate->m_nPedFlags.bDrownsInWater = false;

			CStats::IncrementStat(NUMBER_GANG_MEMBERS_RECRUITED, 1);
			CStats::DisplayScriptStatUpdateMessage(INCREMENT_STAT, GANG_MEMBER_COUNT, 1);

			// Add a radar blip for this guy. Nice to be able to keep track of our guys.
			UInt32	Colour = (gaGangColoursR[PEDTYPE_PLAYER_GANG-PEDTYPE_GANG1] << 24) | (gaGangColoursG[PEDTYPE_PLAYER_GANG-PEDTYPE_GANG1] << 16) | (gaGangColoursB[PEDTYPE_PLAYER_GANG-PEDTYPE_GANG1] << 8) | 0x000000ff;
			Int32 Blip = CRadar::SetEntityBlip(BLIPTYPE_CHAR, CPools::GetPedPool().GetIndex(pCandidate), Colour, BLIPDISPLAY_BLIPONLY, "CODEPLR");
			
			CRadar::ChangeBlipScale(Blip, 2);
			CRadar::ChangeBlipColour(Blip, Colour);
			CRadar::SetBlipFriendly(Blip, TRUE);  // if in our gang then he is friendly
			pCandidate->m_nPedFlags.bClearRadarBlipOnDeath = true;
			
			CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->SetMaxSeparation(CPedGroupMembership::ms_fPlayerGroupMaxSeparation);	// Peds in player group can wander for 120 meters before being removed from group

			pCandidate->Say(CONTEXT_GLOBAL_JOIN_GANG_YES, 2500, 1.0f, TRUE);
			return;
		}
		else
		{
			pCandidate->Say(CONTEXT_GLOBAL_JOIN_GANG_NO, 2500, 1.0f, TRUE);
		
			// this guy didn't join the player group - do response	
			//CPed* pPed = (CPed*)GetWeaponLockOnTarget();
			
			CEventDontJoinPlayerGroup event(this);
			pCandidate->GetPedIntelligence()->AddEvent(event);
		}
	}
}


//
// Name			:	PlayerHasJustAttackedSomeone
// Purpose		:	Called when the player attacks another ped so that we can get the group to
//					attack the same ped/group.
//

void CPlayerPed::PlayerHasJustAttackedSomeone()
{
	PlayerWantsToAttack();
}


//
// Name			:	PlayerWantsToAttack
// Purpose		:	Tell the group to attack if it is not already doing so.
//

void CPlayerPed::PlayerWantsToAttack()
{
	if (CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader() >= 1)
	{
		if (CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].m_followLeader)
		{
			CEntity *pTargetEnt = NULL;
			CPed *pTargetPed = NULL;
			CPedGroupIntelligence* pGroupIntelligence = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupIntelligence();
			pGroupIntelligence->ReportAllBarScriptTasksFinished();

			pTargetEnt = GetWeaponLockOnTarget();
#ifdef GTA_PC
			if(TheCamera.m_bUseMouse3rdPerson && !pTargetEnt)
				pTargetEnt = m_pMouseLockOnRecruitPed;
#endif

			// If the player has someone targetted we will attack that ped.
			if (pTargetEnt && pTargetEnt->GetIsTypePed())
			{
				//Found a ped so just use this one.
				pTargetPed = (CPed *)pTargetEnt;
			}
			else if(pTargetEnt && CTagManager::IsTag(pTargetEnt))
			{
				//Just spraying a tag so don't tell group to join in attack.
				pTargetPed = 0;
			}
			else if(pTargetEnt && pTargetEnt->GetIsTypeObject() && ((CObject*)pTargetEnt)->CanBeTargetted())
			{
				//Just shooting an object (maybe a bottle) so don't tell group to join in attack.
				pTargetPed = 0;
			}
			else
			{
				// Find the nearest ped to the line from the player away from the camera that doesn't belong to the players' gang.
				pTargetPed = FindPedToAttack();
			}
			
			if(pTargetPed)
			{
				CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].PlayerGaveCommand_Attack(this, pTargetPed);
			}
		}
	}
}

//
// Name			:	MakeGroupRespondToPlayerTakingDamage
// Purpose		:	If the player is taking damage from an attack this function will
//					make the idle peds in his group retaliate.
//

void CPlayerPed::MakeGroupRespondToPlayerTakingDamage(const CEventDamage& eventDamage)
{
	CPed* pCulprit=(CPed*)eventDamage.GetInflictor();
	ASSERT(0==pCulprit || pCulprit->GetType()==ENTITY_TYPE_PED);
	if (pCulprit && CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader() >= 1)
	{
//		if (m_GroupAttackStatus == GROUPATT_FOLLOWINGCLOSELY)
		if (CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].m_followLeader)
		{
			CPedGroupIntelligence* pGroupIntelligence = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupIntelligence();
			CEventGroupEvent groupEvent(this,eventDamage.Clone());
			pGroupIntelligence->AddEvent(groupEvent);			
		}
	}
}


//
// Name			:	TellGroupWhetherToFollowPlayer
// Purpose		:	This goes through the peds in the group and will tell each individual
//					ped to follow the player 
//


void CPlayerPed::TellGroupToStartFollowingPlayer(bool bFollow, bool bPlaySound, bool bChangeFollowEvenWhenEmpty)
{
	if (GetPlayerData()->m_GroupAlwaysFollow && !bFollow) return;
	if (GetPlayerData()->m_GroupNeverFollow && bFollow) return;

	//If there is no one in the group then don't modify the follow leader flag.
	//It can be confusing if the user presses the d-pad with no one in the group and then later recruits
	//a gang member only to find that the buddy doesn't follow the leader.
	if (!bChangeFollowEvenWhenEmpty)
	{
		if(0==CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader()) return;
	}

	CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].m_followLeader = bFollow;
	CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupIntelligence()->SetDefaultTaskAllocatorType(CPedGroupDefaultTaskAllocatorTypes::DEFAULT_TASK_ALLOCATOR_RANDOM);


	if(bFollow)
	{
		CEventPlayerCommandToGroupAttack event;
		event.ComputeResponseTaskType(&CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup]);
		if(event.WillRespond())
		{		
			CEventPlayerCommandToGroupGather* pEvent=new CEventPlayerCommandToGroupGather();
			pEvent->SetResponseTaskType(event.GetResponseTaskType());
			CEventGroupEvent eventGroupEvent(this,pEvent);
			CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupIntelligence()->AddEvent(eventGroupEvent);
		}
	}
	
	if (bPlaySound)
	{
		Int32	NumInGroup = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->CountMembersExcludingLeader();
	
		if (NumInGroup > 0)
		{
			float	FurthestDistance = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].FindDistanceToFurthestMember();
		
			if (bFollow)
			{
				if (NumInGroup <= 1)
				{
					if (FurthestDistance < 3.0f)
					{
						Say(CONTEXT_GLOBAL_ORDER_FOLLOW_VNEAR_ONE);
					}
					else if (FurthestDistance < 10.0f)
					{
						Say(CONTEXT_GLOBAL_ORDER_FOLLOW_NEAR_ONE);
					}
					else
					{
						Say(CONTEXT_GLOBAL_ORDER_FOLLOW_FAR_ONE);
					}
				}
				else
				{
					if (FurthestDistance < 3.0f)
					{
						Say(CONTEXT_GLOBAL_ORDER_FOLLOW_VNEAR_MANY);
					}
					else if (FurthestDistance < 10.0f)
					{
						Say(CONTEXT_GLOBAL_ORDER_FOLLOW_NEAR_MANY);
					}
					else
					{
						Say(CONTEXT_GLOBAL_ORDER_FOLLOW_FAR_MANY);
					}
				}
			}
			else
			{
				if (NumInGroup <= 1)
				{
					Say(CONTEXT_GLOBAL_ORDER_WAIT_ONE);
				}
				else
				{
					Say(CONTEXT_GLOBAL_ORDER_WAIT_MANY);
				}
			}
	
		}
	}
}


//
// Name			:	MakePlayerGroupDisappear
// Purpose		:	Makes the player group invisible for a cutscene. 
//
bool abTempNeverLeavesGroup[CPedGroupMembership::MAX_NUM_MEMBERS];

void CPlayerPed::MakePlayerGroupDisappear()
{
	for (Int32 Member = 0; Member < CPedGroupMembership::MAX_NUM_MEMBERS-1; Member++)
	{
		CPed *pPed = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->GetMember(Member);

		if (pPed && pPed->GetCharCreatedBy() != MISSION_CHAR)
		{
			pPed->m_nFlags.bIsVisible = false;
			pPed->SetUsesCollision(false);
			abTempNeverLeavesGroup[Member] = pPed->m_nPedFlags.bNeverLeavesGroup;
			pPed->m_nPedFlags.bNeverLeavesGroup = true;
		}
	}
}

//
// Name			:	MakePlayerGroupReappear
// Purpose		:	Makes the player group visible again (after a cutscene).
//

void CPlayerPed::MakePlayerGroupReappear()
{
	for (Int32 Member = 0; Member < CPedGroupMembership::MAX_NUM_MEMBERS-1; Member++)
	{
		CPed *pPed = CPedGroups::ms_groups[GetPlayerData()->m_PlayerGroup].GetGroupMembership()->GetMember(Member);

		if (pPed && pPed->GetCharCreatedBy() != MISSION_CHAR)
		{
			pPed->m_nFlags.bIsVisible = true;
			if(!pPed->m_nPedFlags.bInVehicle)
			{
				pPed->SetUsesCollision(true);
			}
			pPed->m_nPedFlags.bNeverLeavesGroup = abTempNeverLeavesGroup[Member];
		}
	}
}


/*
// Name			:	UseSprintEnergy
// Purpose		:	use sprint energy
void CPlayerPed::UseSprintEnergy()
{
	if(GetPlayerData()->m_fSprintEnergy > MIN_SPRINT_ENERGY && (!CWorld::Players[CWorld::PlayerInFocus].DoesNotGetTired) && !GetPlayerData()->m_bAdrenaline)
	{
		float ts = CTimer::GetTimeStep();
		
		// if the player is tapping X, let him sprint for longer
		if (GetPedIntelligence()->GetTaskDefault() && GetPedIntelligence()->GetTaskDefault()->GetTaskType()==CTaskTypes::TASK_SIMPLE_PLAYER_ON_FOOT)
		{
			if (((CTaskSimplePlayerOnFoot *)GetPedIntelligence()->GetTaskDefault())->m_fControlSprint > CTaskSimplePlayerOnFoot::SPRINT_THRESHOLD)
				ts *= 0.25f;
		}
	
		GetPlayerData()->m_fSprintEnergy -= ts;
	}
}

// Name			:	RestoreSprintEnergy
// Purpose		:	
// sprint energy
void CPlayerPed::RestoreSprintEnergy(float fRate)
{
	// increase sprint energy
	if(GetPlayerData()->m_fSprintEnergy < CStats::GetFatAndMuscleModifier(STAT_MODIFIER_SPRINT_ENERGY))
		GetPlayerData()->m_fSprintEnergy += fRate * CTimer::GetTimeStep() / 2.0f;
}
*/

void CPlayerPed::ResetSprintEnergy()
{
	GetPlayerData()->m_fSprintEnergy = CStats::GetFatAndMuscleModifier(STAT_MODIFIER_SPRINT_ENERGY);
}

bool CPlayerPed::HandleSprintEnergy(bool bSprint, float fRate)
{
	float ts = CTimer::GetTimeStep()*fRate;
	if(bSprint)
	{
		if(CWorld::Players[CWorld::PlayerInFocus].DoesNotGetTired || GetPlayerData()->m_bAdrenaline || fRate==0.0f)
		{
			// can sprint but doesn't use any energy
			return true;
		}
		else if(GetPlayerData()->m_fSprintEnergy > MIN_SPRINT_ENERGY)
		{
			GetPlayerData()->m_fSprintEnergy = CMaths::Max(MIN_SPRINT_ENERGY, GetPlayerData()->m_fSprintEnergy - ts);
			return true;
		}
	}
	else 
	{
		// increase sprint energy
		if(GetPlayerData()->m_fSprintEnergy < CStats::GetFatAndMuscleModifier(STAT_MODIFIER_SPRINT_ENERGY))
			GetPlayerData()->m_fSprintEnergy += fRate * CTimer::GetTimeStep() / 2.0f;
	}
	
	return false;
}

float PLAYER_SPRINT_TAPADD = 4.0f;
float PLAYER_SPRINT_HOLDSUB = 0.7f;
float PLAYER_SPRINT_RELEASESUB = 0.2f;
float PLAYER_SPRINT_THRESHOLD = 5.0f;

struct SprintControlData
{
	float fTapAdd;
	float fHoldSub;
	float fReleaseSub;
	float fThreshhold;
	float fMaxLimit;
	
	float fEnergyRateStd;		// rate loose energy normally
	float fEnergyRateBashing;	// rate loose energy when bashing button
	
	float fResultMult;
};

SprintControlData PLAYER_SPRINT_SET[SPRINT_UNDER_WATER+1] =
{
//	tapAdd	holdSub	relSub	threshold	max		eRate	eBashRate	resultMult
	{4.0f,	0.7f,	0.2f,	5.0f,		10.0f,	1.0f, 	0.5f,		0.3f},	// onfoot
	{4.0f,	0.7f,	0.2f,	5.0f,		10.0f,	0.0f, 	0.4f,		1.0f},	// bmx
	{4.0f,	0.7f,	0.2f,	5.0f,		10.0f,	1.0f, 	0.3f,		0.3f},	// onwater
	{4.0f,	0.7f,	0.2f,	5.0f,		10.0f,	0.0f, 	0.0f,		1.0f}	// underwater
};

float CPlayerPed::ControlButtonSprint(eSprintType nSprintType)
{
	if(!GetPlayerData())
		return 0.0f;
		
	CPad *pPad = GetPadFromPlayer();
	bool bCanSprint = false;
	if(GetPlayerData()->m_bPlayerSprintDisabled)
		bCanSprint = false;
	else if(GetPlayerData()->m_fSprintControlCounter > 0.0f || GetPlayerData()->m_fSprintEnergy > 0.0f)
		bCanSprint = true;
	
	if(pPad->SprintJustDown() && bCanSprint)
		GetPlayerData()->m_fSprintControlCounter = CMaths::Min(PLAYER_SPRINT_SET[nSprintType].fMaxLimit, GetPlayerData()->m_fSprintControlCounter + PLAYER_SPRINT_SET[nSprintType].fTapAdd);
	else if(pPad->GetSprint() && bCanSprint)
		GetPlayerData()->m_fSprintControlCounter = CMaths::Max(1.0f, GetPlayerData()->m_fSprintControlCounter - PLAYER_SPRINT_SET[nSprintType].fHoldSub*CTimer::GetTimeStep());
	else if(GetPlayerData()->m_fSprintControlCounter > 0.0f)
		GetPlayerData()->m_fSprintControlCounter = CMaths::Max(0.0f, GetPlayerData()->m_fSprintControlCounter - PLAYER_SPRINT_SET[nSprintType].fReleaseSub*CTimer::GetTimeStep());

	float fSprintMult = 0.0f;
	float fUseEnergyMult = 1.0f;
	if(GetPlayerData()->m_fSprintControlCounter > PLAYER_SPRINT_SET[nSprintType].fThreshhold)
	{
		fSprintMult = GetPlayerData()->m_fSprintControlCounter / PLAYER_SPRINT_SET[nSprintType].fThreshhold;
		fUseEnergyMult = PLAYER_SPRINT_SET[nSprintType].fEnergyRateBashing;
	}
	else if(GetPlayerData()->m_fSprintControlCounter > 0.0f && bCanSprint)
	{
		fSprintMult = 1.0f;
		fUseEnergyMult = PLAYER_SPRINT_SET[nSprintType].fEnergyRateStd;
	}
	
	if(fSprintMult > 0.0f)
	{
		// trying to sprint so use energy up
		if(HandleSprintEnergy(true, fUseEnergyMult))
			return 1.0f + PLAYER_SPRINT_SET[nSprintType].fResultMult*CMaths::Max(0.0f, fSprintMult-1.0f);
		// ran out of energy so reset sprint counter
		else
			GetPlayerData()->m_fSprintControlCounter = 0.0f;
	}

	return 0.0f;
}

float CPlayerPed::GetButtonSprintResults(eSprintType nSprintType)
{
	if(GetPlayerData()->m_fSprintControlCounter > PLAYER_SPRINT_THRESHOLD)
		return 1.0f + PLAYER_SPRINT_SET[nSprintType].fResultMult*CMaths::Max(0.0f, (GetPlayerData()->m_fSprintControlCounter / PLAYER_SPRINT_THRESHOLD) - 1.0f);
	else if(GetPlayerData()->m_fSprintControlCounter > 0.0f)
		return 1.0f;

	return 0.0f;		
}



//
// Name			:	HandlePlayerBreath
// Purpose		:	Called when player is in the water on his own or in a vehicle
//					Handles breath and damage once breath has run out 
//
//#define SWIM_BREATH_COUNT_DEC (120)  // times for breath underwater
//#define SWIM_BREATH_COUNT_INC (10)

void CPlayerPed::ResetPlayerBreath()
{
//	GetPlayerData()->m_nBreath = 100.0f;  // make sure breath is at 100 when we go in water
//	GetPlayerData()->m_BreathCounter = CTimer::GetTimeInMilliseconds();
	GetPlayerData()->m_fBreath = CStats::GetFatAndMuscleModifier(STAT_MODIFIER_BREATH_UNDERWATER);
	GetPlayerData()->m_bRequireHandleBreath = false;
}

void CPlayerPed::HandlePlayerBreath(bool bUnderwater, float fRate)
{
	if(bUnderwater && !CCheat::IsCheatActive(CCheat::SCUBA_CHEAT))
	{
		float ts = CTimer::GetTimeStep()*fRate;
		if(GetPlayerData()->m_fBreath > 0.0f && m_nPedFlags.bDrownsInWater)
			GetPlayerData()->m_fBreath = CMaths::Max(0.0f, GetPlayerData()->m_fBreath - ts);
		else
		{
			CWeapon::GenerateDamageEvent(this, this, WEAPONTYPE_DROWNING, int32(3*ts), PED_SPHERE_CHEST, 0);
		}
	}
	else
	{
		if(GetPlayerData()->m_fBreath < CStats::GetFatAndMuscleModifier(STAT_MODIFIER_BREATH_UNDERWATER))
			GetPlayerData()->m_fBreath += 2.0f*fRate*CTimer::GetTimeStep();
	}
		
	GetPlayerData()->m_bRequireHandleBreath = false;
}