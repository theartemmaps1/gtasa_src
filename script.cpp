
/////////////////////////////////////////////////////////////////////////////////
//
// FILE :    script.cpp
// PURPOSE : Scripting and stuff.
// AUTHOR :  Graeme & Obbe. A big thanks to Brian Baird. Most of this stuff is based on GTA2's
//			 scripting.
//			 The GTA2 stuff looks pretty good although a couple of major changes
//			 have to be made for GTA3 (variable timestep, execution until a wait
//			 is encountered). It seems better to rewrite the code so that I know
//			 exactly what's going on. All of the commands have to be rewritten
//			 anyway)
// CREATED : 12-11-99
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SCRIPT_H_
	#include "script.h"
#endif


//
// Andrzej:	if defined, then aborting current mission is possible
//			with pressing "Cross"+"Circle" on Debug Pad (Pad(1));
//
//#define FAIL_CURRENT_MISSION_VIA_PAD


/*
#include "Debug.h"
*/
#include "FileMgr.h"
#include "camera.h"
/*
#include "Timer.h"
*/
#include "Commands.h"
#include "World.h"
#include "General.h"
/*
#include "Timer.h"
#include "PedType.h"
#include "Text.h"
*/
#include "Messages.h"
/*
//	#include "cdtimers.h"
*/
#include "clock.h"
/*
#include "curves.h"
*/
#include "civilianped.h"
/*
#include "autopilot.h"
#include "automobile.h"
*/
#include "game.h"
#include "carctrl.h"
/*
#include "pad.h"
*/
#include "population.h"
#include "object.h"
/*
#include "modelinfo.h"
#include "cargen.h"
#include "user.h"
*/
#include "Pools.h"
#include "PlayerPed.h"
#include "EmergencyPed.h"
/*
#include "radar.h"
#include "restart.h"
#include "shadows.h"
#include "handy.h"
#include "lines.h"
*/
#include "modelindices.h"
#include "font.h"
#include "weather.h"
#include "pickups.h"
//#include "pedroutes.h"
/*
#include "boat.h"
#include "cranes.h"
#include "darkel.h"
#include "explosion.h"
#include "garages.h"
#include "powerpoints.h"
#include "heli.h"
#include "gangs.h"
#include "dummyobject.h"
*/

#include "streaming.h"

#include "remote.h"
/*
#include "phones.h"
#include "coronas.h"
#include "pointlights.h"
*/
#include "animmanager.h"
/*
#include "collision.h"
#include "particleobject.h"
*/
#include "sprite.h"
#include "zones.h"
#ifdef USE_AUDIOENGINE
#include "AudioEngine.h"
#else //USE_AUDIOENGINE
#include "audioman.h"
#endif //USE_AUDIOENGINE
#include "cutscenemgr.h"
/*
#include "fire.h"
#include "waterlevel.h"
#include "projectileinfo.h"
#include "stats.h"
*/
#include "hud.h"
#include "replay.h"
#include "roadblocks.h"
#include "specialfx.h"
#include "AtomicModelInfo.h"
#include "mblur.h"
#include "timecycle.h"
#include "gamelogic.h"
#include "carcols.h"
#include "VehicleModelInfo.h"

#include "QuadBike.h"
#include "Heli.h"
#include "Planes.h"
#include "Bmx.h"
#include "Trailer.h"
#include "VarConsole.h"

#include "overlay.h"
#include "memorycard.h"
#include "VuTurboSprite.h"

#ifndef FINAL
	#include "scriptdebug.h"
#endif

#include "TaskInclude.h"
#include "PedIntelligence.h"
#include "TaskPlayer.h"
#include "TaskCarAccessories.h"
#include "TaskCar.h"
#include "PedGroup.h"
#include "fxSystem.h"
#include "Events.h"
#include "PedTaskRecord.h"
#include "heli.h"
#include "train.h"
#include "clothes.h"
#include "record.h"
#include "boat.h"
#include "gangwars.h"
#include "Events.h"
#include "Frontend.h"
#include "PostEffects.h"

#include "MenuSystem.h"
#include "EntryExits.h"
#include "Stats.h"
#include "ScriptVersion.h"
#include "shopping.h"
#include "loadingscreen.h"
#include "LoadMonitor.h"
#include "restart.h"
#include "garages.h"
#include "zonecull.h"

/////////////////////////////////////////////////////////////////////////////////
// Actual memory for the scripts.
/////////////////////////////////////////////////////////////////////////////////

CRunningScript	CTheScripts::ScriptsArray[MAX_NUM_SCRIPTS];
CRunningScript	*CTheScripts::pActiveScripts;
CRunningScript	*CTheScripts::pIdleScripts;
UInt8			CTheScripts::ScriptSpace[SIZE_SCRIPT_SPACE];
Int32			CTheScripts::LocalVariablesForCurrentMission[MAX_LOCAL_VARIABLES_FOR_A_MISSION_SCRIPT];		//	This memory is used for the local variables of the one currently-running mission

CMissionCleanup CTheScripts::MissionCleanUp;

CUpsideDownCarCheck CTheScripts::UpsideDownCars;

CStuckCarCheck CTheScripts::StuckCars;
CScriptsForBrains CTheScripts::ScriptsForBrains;

CScriptResourceManager CTheScripts::ScriptResourceManager;

CStreamedScripts CTheScripts::StreamedScripts;

entity_waiting_for_script_brain CTheScripts::EntitiesWaitingForScriptBrain[MAX_ENTITIES_WAITING_FOR_SCRIPT_BRAIN];

//	Bool8 CTheScripts::DeatharrestCheckEnabled;
//	Bool8 CTheScripts::DoneDeatharrest;

UInt32 CTheScripts::OnAMissionFlag;
//	Int32 *CTheScripts::OnAGangOneMissionFlag;
//	Int32 *CTheScripts::OnAGangTwoMissionFlag;
//	Int32 *CTheScripts::OnAGangThreeMissionFlag;
UInt32 CTheScripts::LastMissionPassedTime;

//	Int32 CTheScripts::GangOneBaseBriefId;
//	Int32 CTheScripts::GangTwoBaseBriefId;
//	Int32 CTheScripts::GangThreeBaseBriefId;

//	UInt32 CTheScripts::OnAMissionForContactFlag[NUMBER_OF_CONTACTS];
//	UInt32 CTheScripts::BaseBriefIdForContact[NUMBER_OF_CONTACTS];

#ifdef GTA_LIBERTY
collective_entry_struct CTheScripts::CollectiveArray[MAX_NUMBER_OF_COLLECTIVE_MEMBERS];
Int32 CTheScripts::NextFreeCollectiveIndex;
#endif

Int32 CTheScripts::LastRandomPedId;

UsedObjects CTheScripts::UsedObjectArray[MAX_USED_OBJECTS];
UInt16 CTheScripts::NumberOfUsedObjects;

script_sphere_struct CTheScripts::ScriptSphereArray[MAX_SCRIPT_SPHERES];

//	Stuff for displaying intro text
intro_text_line CTheScripts::IntroTextLines[SCRIPT_TEXT_NUM_LINES];
UInt16 CTheScripts::NumberOfIntroTextLinesThisFrame;

UInt8 CTheScripts::UseTextCommands;
//	End of stuff for displaying intro text

Bool8 CTheScripts::bUseMessageFormatting;
UInt16 CTheScripts::MessageCentre;
UInt16 CTheScripts::MessageWidth;


intro_script_rectangle CTheScripts::IntroRectangles[NUM_SCRIPT_RECTANGLES];
UInt16 CTheScripts::NumberOfIntroRectanglesThisFrame;
CSprite2d CTheScripts::ScriptSprites[NUM_SCRIPT_SPRITES];

building_swap_struct CTheScripts::BuildingSwapArray[MAX_NUM_BUILDING_SWAPS];
invisibility_setting_struct CTheScripts::InvisibilitySettingArray[MAX_NUM_INVISIBILITY_SETTINGS];

Int32 CTheScripts::SuppressedVehicleModels[MAX_SUPPRESSED_VEHICLE_MODELS];
Int32 CTheScripts::VehicleModelsBlockedByScript[MAX_VEHICLE_MODELS_BLOCKED_BY_SCRIPT];

anim_group_attached_to_char CTheScripts::ScriptAttachedAnimGroups[MAX_SCRIPT_CHAR_ATTACHED_ANIM_GROUPS];
connect_lods_objects CTheScripts::ScriptConnectLodsObjects[MAX_SCRIPT_CONNECT_LODS_OBJECTS];


	// Some debug info
Int16	CTheScripts::ScriptsUpdated;
Int16	CTheScripts::CommandsExecuted;

#ifndef FINAL
Int16	CTheScripts::NumScriptDebugLines;
sStoredDebugLine	CTheScripts::aStoredLines[MAXNUMSCRIPTDEBUGLINES];
#endif

#ifdef FINAL
const Bool8	CTheScripts::DbgFlag = FALSE;					// If true stuff gets printed out
#else
Bool8	CTheScripts::DbgFlag;					// If true stuff gets printed out
static bool bTimeScripts=false;
static bool bDisplayStreamedScriptInfo = false;

static bool bDisplayNonMissionEntities = false;

static bool bDisplayUsedScriptResources = false;
#endif

#ifndef MASTER
static bool bClosestDoorInfo = false;
#endif


Bool8 CTheScripts::StoreVehicleWasRandom;
Int32 CTheScripts::StoreVehicleIndex;

Bool8 CTheScripts::bUsingAMultiScriptFile;
	
multi_script_struct CTheScripts::MultiScriptArray[MAX_NUMBER_OF_MISSION_SCRIPT_FILES];
UInt32 CTheScripts::MainScriptSize;
UInt32 CTheScripts::LargestMissionScriptSize;
Int16 CTheScripts::NumberOfMissionScripts;
Int16 CTheScripts::NumberOfExclusiveMissionScripts;
UInt32 CTheScripts::LargestNumberOfMissionScriptLocalVariables;
Bool8 CTheScripts::bAlreadyRunningAMissionScript;

UInt8 CTheScripts::FailCurrentMission;
UInt32 CTheScripts::ScriptPickupCycleIndex;

Bool8 CTheScripts::bMiniGameInProgress;
Bool8 CTheScripts::bDisplayNonMiniGameHelpMessages;

Bool8 CTheScripts::bPlayerHasMetDebbieHarry;
UInt8 CTheScripts::RiotIntensity;
Bool8 CTheScripts::bPlayerIsOffTheMap;				//	Don't need to save this - player is never off the map when the game is saved
UInt8 CTheScripts::RadarZoomValue;					//	Don't need to save this
bool8 CTheScripts::RadarShowBlipOnAllLevels;		// dont save
bool8 CTheScripts::HideAllFrontEndMapBlips;			// dont save
Bool8 CTheScripts::bDisplayHud;						//	Don't need to save this
float CTheScripts::fCameraHeadingWhenPlayerIsAttached;	//	Don't need to save this
float CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;	//	Don't need to save this

Bool8 CTheScripts::bEnableCraneRaise;				//	Don't need to save this
Bool8 CTheScripts::bEnableCraneLower;				//	Don't need to save this
Bool8 CTheScripts::bEnableCraneRelease;				//	Don't need to save this

EScriptCrossHair	CTheScripts::bDrawCrossHair;	//	Don't need to save this

Int32 CTheScripts::ForceRandomCarModel;				//	Don't need to save this

Bool8 CTheScripts::bAddNextMessageToPreviousBriefs;	//	Don't need to save this
Bool8 CTheScripts::bScriptHasFadedOut;				//	Don't need to save this

Bool8 CTheScripts::bDrawOddJobTitleBeforeFade;		//	Don't need to save this - should be reset at the end of each mission
Bool8 CTheScripts::bDrawSubtitlesBeforeFade;		//	Don't need to save this - should be reset at the end of each mission


Int16 CTheScripts::CardStack[312];
Int16 CTheScripts::CardStackPosition;

script_effect_struct CTheScripts::ScriptEffectSystemArray[MAX_SCRIPT_EFFECT_SYSTEMS];
script_searchlight_struct CTheScripts::ScriptSearchLightArray[MAX_SCRIPT_SEARCHLIGHTS];
UInt16 CTheScripts::NumberOfScriptSearchLights;

script_checkpoint_struct CTheScripts::ScriptCheckpointArray[MAX_SCRIPT_CHECKPOINTS];
UInt16 CTheScripts::NumberOfScriptCheckpoints;

/*
script_dialogue_struct CTheScripts::ScriptDialogueQueue[SIZE_OF_SCRIPT_DIALOGUE_QUEUE];
Int16 CTheScripts::PlayingScriptDialogueIndex;
UInt16 CTheScripts::NumberOfElementsInScriptDialogueQueue;
*/

script_unique_thing_struct CTheScripts::ScriptSequenceTaskArray[CTaskSequences::MAX_NUM_SEQUENCE_TASKS];


//	Char CTheScripts::NextDieAnimName[ANIM_NAMELEN];
//	Char CTheScripts::NextDieAnimGroupName[ANIMBLOCK_NAMELEN];


#ifdef GTA_PC
#ifdef GTA_LIBERTY
	UInt8 CTheScripts::CountdownToMakePlayerUnsafe;
	UInt8 CTheScripts::DelayMakingPlayerUnsafeThisTime;
#endif
#endif

#ifndef FINAL
Char CTheScripts::ScriptErrorMessage[SCRIPT_ERROR_MESSAGE_MAX_LENGTH];
#endif


UInt16 CTheScripts::NumberOfEntriesStillToReadForSwitch;
Int32 CTheScripts::ValueToCheckInSwitchStatement;
Bool8 CTheScripts::SwitchDefaultExists;
Int32 CTheScripts::SwitchDefaultAddress;
UInt16 CTheScripts::NumberOfEntriesInSwitchTable;

jump_table_entry CTheScripts::SwitchJumpTable[MAX_ENTRIES_IN_SWITCH_JUMP_TABLE];

uint8 CTheScripts::m_nMenuIndex = -1;

Int32	ScriptParams[32];
#ifndef FINAL
static bool gbRecordScriptCommands = false;
#endif


extern bool gAllowScriptedFixedCameraCollision;	//	A camera flag that needs to be reset on mission cleanup and when a new game starts

//	static Int32 StoreVehicleIndex;

/*
static UInt32 CommandUsage[COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED + 1];
static UInt32 OrderedCommands[COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED + 1];
static UInt32 OrderedUsage[COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED + 1];
static UInt32 CommandTime[COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED + 1];
static UInt32 OrderedTime[COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED + 1];
*/

/*
void TestFunc(Int32 Num);

void TestFunc(Int32 Num)
{
	if (Num > 0)
	{
		TestFunc(Num-1);
	}
}
*/

/////////////////////////////////////////////////////////////////////////////////
// Mission Cleanup Code - copied from Brian's GTA2 code
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CMissionCleanup
// PURPOSE : Constructor
/////////////////////////////////////////////////////////////////////////////////
CMissionCleanup::CMissionCleanup(void)
{
	Init();
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CMissionCleanup::Init
// PURPOSE : Clears all the data in this class
/////////////////////////////////////////////////////////////////////////////////
void CMissionCleanup::Init(void)
{
	UInt16 loop;

	Count = 0;

	for (loop = 0; loop < MAX_CLEANUP; loop++)
	{
		cleanup_entities[loop].Type = CLEANUP_UNUSED;
		cleanup_entities[loop].Id = 0;
	}
	
	
#ifndef FINAL
	NonMissionCount = 0;
	
	for (loop = 0; loop < MAX_NON_MISSION_CLEANUP; loop++)
	{
		non_mission_entities[loop].Type = CLEANUP_UNUSED;
		non_mission_entities[loop].Id = 0;
		non_mission_entities[loop].script_name[0] = 0;
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CMissionCleanup::FindFree
// PURPOSE : Find space to store the details of a car, char or object created
//								by a mission script
// RETURNS : Pointer to a free entry
/////////////////////////////////////////////////////////////////////////////////
cleanup_entity_struct *CMissionCleanup::FindFree(void)
{
	UInt16 Index;
	cleanup_entity_struct *Curr;

	ASSERTMSG(Count < MAX_CLEANUP, "CMissionCleanup::FindFree - no space left");

	Index = 0;
	Curr = cleanup_entities;

	while(Index < MAX_CLEANUP)
	{
		if (Curr->Type == CLEANUP_UNUSED)
			return(Curr);

		Curr++;
		Index++;
	}

//	No space to store entity details
	return NULL;
}


#ifndef FINAL
non_mission_stuff *CMissionCleanup::FindNonMissionFree(void)
{
	UInt16 Index;
	non_mission_stuff *pCurr;
	
	ASSERTMSG(NonMissionCount < MAX_NON_MISSION_CLEANUP, "CMissionCleanup::FindNonMissionFree - no space left");
	
	Index = 0;
	pCurr = non_mission_entities;
	
	while (Index < MAX_NON_MISSION_CLEANUP)
	{
		if (pCurr->Type == CLEANUP_UNUSED)
			return pCurr;
			
		pCurr++;
		Index++;
	}

// No space to store entity details
	return NULL;
}
#endif

void CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects(void)
{
	UInt16 loop;
	cleanup_entity_struct *Curr;

	CVehicle* pVehicle;
	CPed *pPed;
	CObject *pObject;
	CVector2D EntityPosition2d;


	Curr = cleanup_entities;

	for (loop = 0; loop < MAX_CLEANUP; loop++)
	{
		if (Curr->Type != CLEANUP_UNUSED)
		{
			switch (Curr->Type)
			{
				case CLEANUP_CAR :
					pVehicle = CPools::GetVehiclePool().GetAt(Curr->Id);

					if (pVehicle)
					{
						ASSERTMSG(pVehicle->GetVehicleCreatedBy() == MISSION_VEHICLE, "CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects - Car is not a MISSION_VEHICLE");
						
						if ( (pVehicle->m_nFlags.bIsStaticWaitingForCollision))
						{
							if ( (CColStore::HasCollisionLoaded(pVehicle->GetPosition(), pVehicle->GetAreaCode()))
								&& (CIplStore::HaveIplsLoaded(pVehicle->GetPosition(), pVehicle->GetAreaCode())) )
							{
								pVehicle->m_nFlags.bIsStaticWaitingForCollision = false;
								
								if (!pVehicle->GetIsStatic())
								{
//									if (!pVehicle->InfiniteMass)
									{
										pVehicle->AddToMovingList();
									}
								}

								switch (pVehicle->GetBaseVehicleType())
								{
									case VEHICLE_TYPE_CAR :
										((CAutomobile *) pVehicle)->PlaceOnRoadProperly();
										break;
										
									case VEHICLE_TYPE_BIKE :
										((CBike *) pVehicle)->PlaceOnRoadProperly();
										break;

									case VEHICLE_TYPE_TRAILER :
										((CTrailer *) pVehicle)->PlaceOnRoadProperly();
										break;

								}
							}
						}
					}
					break;

				case CLEANUP_CHAR :
					pPed = CPools::GetPedPool().GetAt(Curr->Id);
					
					if (pPed)
					{
						ASSERTMSG(pPed->GetCharCreatedBy() == MISSION_CHAR, "CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects - Character is not a MISSION_CHAR");

						if ( (pPed->m_nFlags.bIsStaticWaitingForCollision))
						{
							if ( (CColStore::HasCollisionLoaded(pPed->GetPosition(), pPed->GetAreaCode()))
								&& (CIplStore::HaveIplsLoaded(pPed->GetPosition(), pPed->GetAreaCode())) )
							{
								pPed->m_nFlags.bIsStaticWaitingForCollision = false;
								
								if (!pPed->GetIsStatic())
								{
//									if (!pPed->InfiniteMass)
									{
										pPed->AddToMovingList();
									}
								}
							}
						}
					}

					break;

				case CLEANUP_OBJECT :
					pObject = CPools::GetObjectPool().GetAt(Curr->Id);

					if (pObject)
					{
						// Objects don't seem to ever be deleted - just turned into
						// dummy objects
						ASSERTMSG(pObject->ObjectCreatedBy == MISSION_OBJECT, "CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects - Object is not a MISSION_OBJECT");

						if ( (pObject->m_nFlags.bIsStaticWaitingForCollision))
						{
							if ( (CColStore::HasCollisionLoaded(pObject->GetPosition(), pObject->GetAreaCode()))
								&& (CIplStore::HaveIplsLoaded(pObject->GetPosition(), pObject->GetAreaCode())) )
							{
								pObject->m_nFlags.bIsStaticWaitingForCollision = false;
								
								if (!pObject->GetIsStatic())
								{
//									if (!pObject->InfiniteMass)
									{
										pObject->AddToMovingList();
									}
								}
							}
						}
					}
					break;
					
				case CLEANUP_ATTRACTOR :
					break;
				
				case CLEANUP_THREAT_LIST :
					break;
				
				case CLEANUP_SEQUENCE_TASK :			
					break;
					
				case CLEANUP_PEDGROUP :
					break;
					
				case CLEANUP_EFFECT_SYSTEM :
					break;
					
				case CLEANUP_DECISION_MAKER :
					break;

				case CLEANUP_FRIEND_LIST :
					break;
					
				case CLEANUP_SEARCHLIGHT :
					break;
					
				case CLEANUP_CHECKPOINT :
					break;
				
				case CLEANUP_TEXTURE_DICTIONARY :
					break;
					
				default :
					ASSERTMSG(0, "CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects - Unknown Type");
					break;
			}
		}

		Curr++;
	}
	
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CMissionCleanup::AddEntityToList
// PURPOSE : Add the details of a car, char or object created by a mission script
//				to the clean up array so that it can be deleted when the mission
//				is over. This should not be called for the main script.
// INPUTS : Id of entity, Type of entity(Car, Char, Object)
/////////////////////////////////////////////////////////////////////////////////
void CMissionCleanup::AddEntityToList(Int32 Id, UInt8 Type)
{
	cleanup_entity_struct *Curr;

	Curr = FindFree();

	if (Curr)
	{
		Curr->Id = Id;
		Curr->Type =Type;

		Count++;
	}
}

#ifndef FINAL
void CMissionCleanup::AddEntityToNonMissionList(Int32 Id, UInt8 Type, Char *pScriptName)
{
	non_mission_stuff *pCurr;
	
	pCurr = FindNonMissionFree();
	
	if (pCurr)
	{
		pCurr->Id = Id;
		pCurr->Type = Type;
		strncpy(pCurr->script_name, pScriptName, 8);
		
		NonMissionCount++;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CMissionCleanup::RemoveEntityFromList
// PURPOSE : Removes the details of a car, char or object from the clean up array.
//				It is assumed that any special tidying-up of the entity has already
//				been done.
// INPUTS :	Id of entity, Type of entity(Car, Char, Object)
/////////////////////////////////////////////////////////////////////////////////
void CMissionCleanup::RemoveEntityFromList(Int32 Id, UInt8 Type)
{
	UInt16 loop;
	cleanup_entity_struct *Curr;

	CVehicle* pVehicle;
	CPed *pPed;
	CObject *pObject;


	Curr = cleanup_entities;

	for (loop = 0; loop < MAX_CLEANUP; loop++)
	{
		if ((Curr->Type == Type) && (Curr->Id == Id))
		{
			if (Curr->Type != CLEANUP_UNUSED)
			{
				switch (Curr->Type)
				{
					case CLEANUP_CAR :
						pVehicle = CPools::GetVehiclePool().GetAt(Curr->Id);

						if (pVehicle)
						{
//							ASSERTMSG(pVehicle->VehicleCreatedBy == MISSION_VEHICLE, "CMissionCleanup::RemoveEntityFromList - Car is not a MISSION_VEHICLE");
							
							if (pVehicle->m_nFlags.bIsStaticWaitingForCollision)
							{
								pVehicle->m_nFlags.bIsStaticWaitingForCollision = false;
								
								if (!pVehicle->GetIsStatic())
								{
//									if (!pVehicle->InfiniteMass)
									{
										pVehicle->AddToMovingList();
									}
								}
							}
						}
						break;

					case CLEANUP_CHAR :
						pPed = CPools::GetPedPool().GetAt(Curr->Id);
						
						if (pPed)
						{
//							ASSERTMSG(pPed->CharCreatedBy == MISSION_CHAR, "CMissionCleanup::RemoveEntityFromList - Character is not a MISSION_CHAR");

							if (pPed->m_nFlags.bIsStaticWaitingForCollision)
							{
								pPed->m_nFlags.bIsStaticWaitingForCollision = false;
								
								if (!pPed->GetIsStatic())
								{
//									if (!pPed->InfiniteMass)
									{
										pPed->AddToMovingList();
									}
								}
							}
						}

						break;

					case CLEANUP_OBJECT :
						pObject = CPools::GetObjectPool().GetAt(Curr->Id);

						if (pObject)
						{
							// Objects don't seem to ever be deleted - just turned into
							// dummy objects
//							ASSERTMSG(pObject->ObjectCreatedBy == MISSION_OBJECT, "CMissionCleanup::RemoveEntityFromList - Object is not a MISSION_OBJECT");

							if (pObject->m_nFlags.bIsStaticWaitingForCollision)
							{
								pObject->m_nFlags.bIsStaticWaitingForCollision = false;
								
								if (!pObject->GetIsStatic())
								{
//									if (!pObject->InfiniteMass)
									{
										pObject->AddToMovingList();
									}
								}
							}
						}
						break;
					
					case CLEANUP_ATTRACTOR :
						break;
					
					case CLEANUP_THREAT_LIST :
						break;
					
					case CLEANUP_SEQUENCE_TASK :			
						break;
						
					case CLEANUP_PEDGROUP :
						break;
						
					case CLEANUP_EFFECT_SYSTEM :
						break;
						
					case CLEANUP_DECISION_MAKER :
						break;

					case CLEANUP_FRIEND_LIST :
						break;
						
					case CLEANUP_SEARCHLIGHT :
						break;
						
					case CLEANUP_CHECKPOINT :
						break;
						
					case CLEANUP_TEXTURE_DICTIONARY :
						break;
						
					default :
						ASSERTMSG(0, "CMissionCleanup::RemoveEntityFromList - Unknown Type");
						break;
				}
			}

			Curr->Id = 0;
			Curr->Type = CLEANUP_UNUSED;
			Count--;
		}

		Curr++;
	}
}




#ifndef FINAL
void CMissionCleanup::RemoveEntityFromNonMissionList(Int32 Id, UInt8 Type, Char *pScriptName)
{
	UInt16 loop;
	non_mission_stuff *pCurr;
	
	Char debug_string[100];
	
	pCurr = non_mission_entities;
	
	for (loop = 0; loop < MAX_NON_MISSION_CLEANUP; loop++)
	{
		if ( (pCurr->Type == Type) && (pCurr->Id == Id) )
		{
			if (pCurr->Type != CLEANUP_UNUSED)
			{
				if (strcmp(pScriptName, pCurr->script_name))
				{
					sprintf(debug_string, "RemoveEntityFromNonMissionList - entity added by %s removed by %s", pCurr->script_name, pScriptName);
					DEBUGLOG(debug_string);
				}

				pCurr->Type = CLEANUP_UNUSED;
				pCurr->Id = 0;
				pCurr->script_name[0] = 0;
				NonMissionCount--;
			}
		}
		
		pCurr++;
	}
}


void CMissionCleanup::DisplayAllNonMissionEntities(Bool8 bDisplayUsingVarConsole, Int16 CycleNumber, UInt8 *pDisplayRow)
{
	UInt16 loop;
	non_mission_stuff *pCurr;
	Int32 ModelIndex;
	
	CVehicle* pVehicle;
	CPed *pPed;
	CObject *pObject;
	
	pCurr = non_mission_entities;

//	if (bDisplayNonMissionEntities)

	
	for (loop = 0; loop < MAX_NON_MISSION_CLEANUP; loop++)
	{
		if (pCurr->Type != CLEANUP_UNUSED)
		{
			switch (pCurr->Type)
			{
				case CLEANUP_CAR :
					pVehicle = CPools::GetVehiclePool().GetAt(pCurr->Id);

					if (pVehicle)
					{
						ModelIndex = pVehicle->GetModelIndex();
						const Char *pModelName = CModelInfo::GetModelInfo(ModelIndex)->GetModelName();
						
						sprintf(gString, "Car %s : %s", pModelName, pCurr->script_name);
					}
					else
					{
						sprintf(gString, "Some kind of vehicle maybe : %s", pCurr->script_name);
					}
					break;

				case CLEANUP_CHAR :
					pPed = CPools::GetPedPool().GetAt(pCurr->Id);
					
					if (pPed)
					{
						ModelIndex = pPed->GetModelIndex();
						const Char *pModelName = CModelInfo::GetModelInfo(ModelIndex)->GetModelName();
						
						sprintf(gString, "Ped %s : %s", pModelName, pCurr->script_name);
					}
					else
					{
						sprintf(gString, "Some kind of ped maybe : %s", pCurr->script_name);
					}
					break;

				case CLEANUP_OBJECT :
					pObject = CPools::GetObjectPool().GetAt(pCurr->Id);

					if (pObject)
					{
						ModelIndex = pObject->GetModelIndex();
						const Char *pModelName = CModelInfo::GetModelInfo(ModelIndex)->GetModelName();
						
						sprintf(gString, "Object %s : %s", pModelName, pCurr->script_name);
					}
					else
					{
						sprintf(gString, "Some kind of object maybe : %s", pCurr->script_name);
					}
					break;

				case CLEANUP_EFFECT_SYSTEM :
					sprintf(gString, "Effect System %d : %s", pCurr->Id, pCurr->script_name);
					break;

				case CLEANUP_PEDGROUP :
					sprintf(gString, "Ped Group %d : %s", pCurr->Id, pCurr->script_name);
					break;

				case CLEANUP_THREAT_LIST :
					sprintf(gString, "Threat List %d : %s", pCurr->Id, pCurr->script_name);
					break;
				
				case CLEANUP_ATTRACTOR :
					sprintf(gString, "Attractor %d : %s", pCurr->Id, pCurr->script_name);
					break;
				
				case CLEANUP_SEQUENCE_TASK :
					sprintf(gString, "Sequence %d : %s", pCurr->Id, pCurr->script_name);
					break;
					
				case CLEANUP_DECISION_MAKER :
					sprintf(gString, "Decision Maker %d : %s", pCurr->Id, pCurr->script_name);
					break;

				case CLEANUP_FRIEND_LIST :
					sprintf(gString, "Friend List %d : %s", pCurr->Id, pCurr->script_name);
					break;
					
				case CLEANUP_SEARCHLIGHT :
					sprintf(gString, "Search Light %d : %s", pCurr->Id, pCurr->script_name);
					break;
					
				case CLEANUP_CHECKPOINT :
					sprintf(gString, "Checkpoint %d : %s", pCurr->Id, pCurr->script_name);
					break;
					
				case CLEANUP_TEXTURE_DICTIONARY :
					sprintf(gString, "Texture Dictionary %d : %s", pCurr->Id, pCurr->script_name);
					break;
						
				default :
					ASSERTMSG(0, "CMissionCleanup::DisplayAllNonMissionEntities - Unknown Type");
					break;
			}
			
			if (bDisplayUsingVarConsole)
			{															
				VarConsole.AddDebugOutput(gString);
			}
			else
			{
				strcat(gString, "\n");
				if(CycleNumber==0)
					DEBUGLOG(gString);
					
					
				if (pDisplayRow)
				{
					ObrsPrintfString(gString, 4, *pDisplayRow);
					(*pDisplayRow)++;
				}
			}
		}
		
		pCurr++;
	}
}	
#endif


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CMissionCleanup::Process
// PURPOSE : This function is called once the mission is finished. It cleans up
//				all cars, chars, objects created by the mission. It also clears
//				the StoreVehicle.
/////////////////////////////////////////////////////////////////////////////////
void CMissionCleanup::Process(void)
{
	UInt16 loop;
	cleanup_entity_struct *Curr;

	CVehicle* pVehicle;
	CPed *pPed;
	CObject *pObject;
//	cAudioScriptObject *pSound;
	CPlayerInfo *pPlayer;
	Int32 PlayersGroup;

	Int32 ActualIndex;
	
	DoFadeScrewUpCheck();

	CTrain::DisableRandomTrains(false);
	CTrain::ReleaseMissionTrains();

	//nEnterCarRangeMultiplier = 1.0f;
	//nThreatReactionRangeMultiplier = 1.0f;
	CPlane::SwitchAmbientPlanes(true);

	CPopulation::PedDensityMultiplier = 1.0f;
	CCarCtrl::CarDensityMultiplier = 1.0f;
	if (CGangWars::bTrainingMission)
	{
		CGangWars::bTrainingMission = false;
		CTheZones::FillZonesWithGangColours(!CGangWars::bGangWarsActive);
	}
	
	CPopulation::m_AllRandomPedsThisType = -1;

	CGangWars::bCanTriggerGangWarWhenOnAMission = false;
	CGangWars::ClearSpecificZonesToTriggerGangWar();
	CPopulation::m_bDontCreateRandomGangMembers = false;
	CPopulation::m_bOnlyCreateRandomGangMembers = false;
	CPopulation::m_bDontCreateRandomCops = false;
	CGarages::NoResprays = false;
	CGarages::AllRespraysCloseOrOpen(true);
	CCullZones::bMilitaryZonesDisabled = false;
	
//	Reset the Wanted Level multiplier
	FindPlayerWanted()->m_fMultiplier = 1.0f;
	CPickups::RemoveMissionPickUps();
	
	CRoadBlocks::ClearScriptRoadBlocks();
//	CFakeHeli::SetScriptPoliceHeliToChase(NULL);
	CStreaming::DisableCopBikes(false);
	
	g_LoadMonitor.EnableAmbientCrime();
	
	CObject::bArea51SamSiteDisabled = false;
	CObject::bAircraftCarrierSamSiteDisabled = true;
	
	//CRouteNode::Initialise();
	ThePaths.ReleaseRequestedNodes();
	ThePaths.UnMarkAllRoadNodesAsDontWander();
	ThePaths.TidyUpNodeSwitchesAfterMission();

	
	CVehicleRecording::RemoveAllRecordingsThatArentUsed();

//	CPad::GetPad(0)->EnableControlsScript();

	TheCamera.SetWideScreenOff();
	TheCamera.m_ModeForTwoPlayersSeparateCars = CCam::MODE_TWOPLAYER_SEPARATE_CARS;
	TheCamera.m_ModeForTwoPlayersSameCarShootingAllowed = CCam::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING;
	TheCamera.m_ModeForTwoPlayersSameCarShootingNotAllowed = CCam::MODE_BEHINDCAR;
	TheCamera.m_ModeForTwoPlayersNotBothInCar = CCam::MODE_TWOPLAYER;
	TheCamera.m_bDisableFirstPersonInCar = false;
	TheCamera.ForceCinemaCam(false);
	TheCamera.InitialiseScriptableComponents();
	TheCamera.ResetDuckingSystem(NULL);
	gCurDistForCam = 1.0f;
	gAllowScriptedFixedCameraCollision = false;		//	A camera flag that needs to be reset on mission cleanup and when a new game starts
	CGameLogic::bScriptCoopGameGoingOn = false;

	CTheScripts::bDrawCrossHair = SCRIPT_CROSSHAIR_OFF;
	CSpecialFX::bVideoCam = false;
	CSpecialFX::bLiftCam = false;
// Reset all Filters that can be applied by the scripts
	CPostEffects::ScriptResetForEffects();

//---------------------------------------------------
	
	CEntryExitManager::DisableAllEntryExits(false);

	if (CGame::currArea == AREA_MAIN_MAP)
	{
		CTimeCycle::StopExtraColour(false);
	}
	
#ifdef USE_AUDIOENGINE
	for(loop = 0; loop < AE_NUM_MISSION_SLOTS; loop++)
		AudioEngine.ClearMissionAudio(loop);
#else //USE_AUDIOENGINE
	for (loop = 0; loop < AM_MAX_MISSION_AUDIO_SLOTS; loop++)
	{
		DMAudio.ClearMissionAudio(loop);
	}
#endif //USE_AUDIOENGINE
	
	CWeather::ReleaseWeather();
	
	gFireManager.m_nMaxGenerationsAllowed = 99999;

//	#ifdef GTA_LIBERTY	
	for (loop = 0; loop < NUM_OF_SPECIALCHARS; loop++)
	{
		CStreaming::SetMissionDoesntRequireSpecialChar(loop);
	}
		

	CTheScripts::ClearAllSuppressedCarModels();
	
	CTheScripts::ForceRandomCarModel = -1;

	CStreaming::EnableStreaming();

	CHud::m_ItemToFlash = -1;	//	Clear flashing hud
//	CHud::SetHelpMessage(NULL);	//	Clear help message	- now only done if the player is not in a shop
	CHud::bScriptDontDisplayRadar = false;
	CHud::bScriptDontDisplayVehicleName = false;
	CHud::bScriptDontDisplayAreaName = false;
	CHud::bScriptForceDisplayWithCounters = false;
	
	FrontEndMenuManager.bPauseWhenInWidescreen = false;
	
	CTheScripts::RadarZoomValue = 0;
	CTheScripts::RadarShowBlipOnAllLevels = FALSE;
	CTheScripts::HideAllFrontEndMapBlips = FALSE;
	
	C3dMarkers::ForceRender(FALSE);
	
	CTheScripts::bDisplayHud = true;
	CTheScripts::fCameraHeadingWhenPlayerIsAttached = 0.0f;
	CTheScripts::fCameraHeadingStepWhenPlayerIsAttached = 0.0f;
	
	CTheScripts::bEnableCraneRaise = true;
	CTheScripts::bEnableCraneLower = true;
	CTheScripts::bEnableCraneRelease = true;
	
	CUserDisplay::OnscnTimer.FreezeTimers = FALSE;
	
	CTheScripts::bUseMessageFormatting = false;
	CTheScripts::MessageCentre = 0;
	CTheScripts::MessageWidth = 0;
	
	CTheScripts::bDrawOddJobTitleBeforeFade = true;
	CTheScripts::bDrawSubtitlesBeforeFade = true;

	
//	CTheScripts::RemoveScriptTextureDictionary();	//	only do this if the texture dictionary has been loaded by the mission script
													//	this is so that if the player is on a mission and also in a shop or mini game that
													//	uses a texture dictionary, the texture dictionary for that shop/mini game is not removed

	pPlayer = &(CWorld::Players[0]);
	ASSERTMSG(pPlayer, "CMissionCleanup::Process - Player doesn't exist");
	ASSERTMSG(pPlayer->pPed, "CMissionCleanup::Process - Player exists but doesn't have a character pointer");
	ASSERTMSG(pPlayer->pPed->GetPlayerWanted(), "CMissionCleanup::Process - Player character doesn't have a wanted class");

/*
	if((pPlayer->PlayerState != CPlayerInfo::PLAYERSTATE_HASBEENARRESTED) && (pPlayer->PlayerState != CPlayerInfo::PLAYERSTATE_HASDIED))
	{
		if(TheCamera.GetFadingDirection()==CCamera::FADE_OUT)
//			TheCamera.SetFadingDirection(FADE_IN);
	}
*/
	// Set the player's group back to its default values
	PlayersGroup = pPlayer->pPed->GetPlayerData()->m_PlayerGroup;
	CPedGroups::ms_groups[PlayersGroup].GetGroupIntelligence()->SetDefaultTaskAllocatorType(CPedGroupDefaultTaskAllocatorTypes::DEFAULT_TASK_ALLOCATOR_FOLLOW_LIMITED);
	CPedGroups::ms_groups[PlayersGroup].GetGroupIntelligence()->SetGroupDecisionMakerType(CDecisionMakerTypes::DEFAULT_DECISION_MAKER);
	CPedGroups::ms_groups[PlayersGroup].GetGroupMembership()->SetMaxSeparation(CPedGroupMembership::ms_fPlayerGroupMaxSeparation);
	pPlayer->pPed->GetPlayerData()->m_GroupStuffDisabled = false;
	pPlayer->pPed->ForceGroupToAlwaysFollow(false);
	pPlayer->pPed->ForceGroupToNeverFollow(false);
	pPlayer->pPed->MakePlayerGroupReappear();

	pPlayer->pPed->GetPlayerData()->m_nScriptLimitToGangSize = 99;

	pPlayer->pPed->FireDamageMultiplier = 1.0f;

//	pPlayer->pPed->m_nDrunkenness = 0;
//	CMBlur::ClearDrunkBlur();
	pPlayer->PlayerPedData.m_bFadeDrunkenness = true;
	pPlayer->PlayerPedData.m_nDrugLevel = 0;

	pPlayer->pPed->EnablePedSpeech();
	pPlayer->pPed->EnablePedSpeechForScriptSpeech();

	CPad::GetPad(0)->SetDrunkInputDelay(0);
	CPad::GetPad(0)->bApplyBrakes = FALSE;
	CPad::GetPad(0)->bDisablePlayerEnterCar = false;
	CPad::GetPad(0)->bDisablePlayerDuck = false;
	CPad::GetPad(0)->bDisablePlayerFireWeapon = false;
	CPad::GetPad(0)->bDisablePlayerFireWeaponWithL1 = false;
	CPad::GetPad(0)->bDisablePlayerCycleWeapon = false;
	CPad::GetPad(0)->bDisablePlayerJump = false;
	CPad::GetPad(0)->bDisablePlayerDisplayVitalStats = false;
	pPlayer->bCanDoDriveBy = true;
	
	
	// If the player is using a mobile phone then end the phonecall now
	CTask *pTaskPrimary = pPlayer->pPed->GetPedIntelligence()->FindTaskPrimaryByType(CTaskTypes::TASK_COMPLEX_USE_MOBILE_PHONE);

	if(pTaskPrimary && pTaskPrimary->GetTaskType()==CTaskTypes::TASK_COMPLEX_USE_MOBILE_PHONE)
	{
		CTaskComplexUseMobilePhone *pTaskPhone=(CTaskComplexUseMobilePhone*)pTaskPrimary;
//		pTaskPhone->MakeAbortable(pPlayer->pPed, CTask::ABORT_PRIORITY_LEISURE,0);
		pTaskPhone->Stop(pPlayer->pPed);
	}


//	CPedGroups::ms_groups[FindPlayerPed()->GetPlayerData()->m_PlayerGroup].GetGroupMembership()->SetMaxSeparation(CPedGroupMembership::ms_fMaxSeparation);

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	DMAudio.ShutUpPlayerTalking(FALSE);
#endif //USE_AUDIOENGINE

	CVehicle::bDisableRemoteDetonation = false;
	CVehicle::bDisableRemoteDetonationOnContact = false;

//	CGameLogic::ClearShortCut();
	CGameLogic::ClearSkip(true);

	if (CGameLogic::GameState != CGameLogic::GAMESTATE_ARREST && CGameLogic::GameState != CGameLogic::GAMESTATE_DEATH && FindPlayerPed()->GetPedState() != PED_DEAD && FindPlayerPed()->GetPedState() != PED_DIE)
	{
		CRestart::ClearRespawnPointForDurationOfMission();
	}

	CTheScripts::RiotIntensity = 0;
	
	gFireManager.ClearAllScriptFireFlags();

//	CWorld::SetAllCarsCanBeDamaged(TRUE);
	
//	pPlayer->pPed->SetPlayerCanBeDamaged(TRUE);

// Clean up the StoreVehicleIndex
	CTheScripts::StoreVehicleIndex = -1;
	CTheScripts::StoreVehicleWasRandom = TRUE;

// Clear the array used for checking for upside-down cars
	CTheScripts::UpsideDownCars.Init();
	
// Clear the array used for checking for stuck cars
	CTheScripts::StuckCars.Init();

// Clean up all entities created by the latest mission
	Curr = cleanup_entities;

// Reset the Update Stats
	CStats::bShowUpdateStats = true;
	
// Reset the GunShot Range
	CEventGunShot:: ms_fGunShotSenseRangeForRiot2=-1.0f;

// Reset Hud HelpBox Size
	CHud::m_HelpMessageBoxSize = 200.0f;

	const char *pShopName = CShopping::GetShopLoaded();
	
	CVehicle::ms_forceVehicleLightsOff = false;
	
//	if ( (pShopName[0] == '\0') && (CTheScripts::bMiniGameInProgress == false) )
	if (CTheScripts::bMiniGameInProgress == false)
	{	// only restore the camera and give the player control back when he is not in a shop

		// don't restore camera if we have a remote vehicle
		if(CWorld::Players[CWorld::PlayerInFocus].pRemoteVehicle == NULL)
			TheCamera.Restore();	//	RestoreWithJumpCut();

		pPlayer->pPed->GetPlayerWanted()->m_PoliceBackOff = FALSE;
		pPlayer->pPed->GetPlayerWanted()->m_EverybodyBackOff = FALSE;

		pPlayer->MakePlayerSafe(FALSE);
		
		CHud::SetHelpMessage(NULL, true);
	}

	for (loop = 0; loop < MAX_CLEANUP; loop++)
	{
		if (Curr->Type != CLEANUP_UNUSED)
		{
			switch (Curr->Type)
			{
				case CLEANUP_CAR :
					pVehicle = CPools::GetVehiclePool().GetAt(Curr->Id);

					if (pVehicle)
					{
						ASSERTMSG(pVehicle->GetVehicleCreatedBy() == MISSION_VEHICLE, "CMissionCleanup::Process - Car is not a MISSION_VEHICLE");
						
						CTheScripts::CleanUpThisVehicle(pVehicle);
					}
					break;

				case CLEANUP_CHAR :
					pPed = CPools::GetPedPool().GetAt(Curr->Id);
					
					if (pPed)
					{
						ASSERTMSG(pPed->GetCharCreatedBy() == MISSION_CHAR, "CMissionCleanup::Process - Character is not a MISSION_CHAR");

						CTheScripts::CleanUpThisPed(pPed);
					}

					break;

				case CLEANUP_OBJECT :
					pObject = CPools::GetObjectPool().GetAt(Curr->Id);

					if (pObject)
					{
						// Objects don't seem to ever be deleted - just turned into
						// dummy objects
						ASSERTMSG(pObject->ObjectCreatedBy == MISSION_OBJECT, "CMissionCleanup::Process - Object is not a MISSION_OBJECT");

						CTheScripts::CleanUpThisObject(pObject);
					}
					break;
					
/*
				case CLEANUP_SOUND :
					pSound = CPools::GetAudioScriptObjectPool().GetAt(Curr->Id);
					
					if (pSound)
					{
						ASSERTMSG(pSound->AudioEntityHandle >= 0, "Error when cleaning up a SOUND in CMissionCleanup::Process()");
			
						DMAudio.DestroyLoopingScriptObject(pSound->AudioEntityHandle);
						delete(pSound);
					}
					break;
*/

				case CLEANUP_EFFECT_SYSTEM :
				{
					ActualIndex = CTheScripts::GetActualScriptThingIndex(Curr->Id, UNIQUE_SCRIPT_EFFECT_SYSTEM);
					
					if (ActualIndex >= 0)
					{
						if (CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem)
						{
							CTheScripts::ScriptEffectSystemArray[ActualIndex].pFXSystem->Kill();
							CTheScripts::RemoveScriptEffectSystem(Curr->Id);
						}
					}
				}
					break;
				
				case CLEANUP_DECISION_MAKER :
				{
					ActualIndex = CTheScripts::GetActualScriptThingIndex(Curr->Id, UNIQUE_SCRIPT_DECISION_MAKER);
					
					if (ActualIndex >= 0)
					{
						CDecisionMakerTypesFileLoader::UnloadDecisionMaker(ActualIndex);
					}
				}
				break;
				
				case CLEANUP_PEDGROUP :
				{
					ActualIndex = CTheScripts::GetActualScriptThingIndex(Curr->Id, UNIQUE_SCRIPT_PEDGROUP);
					if (ActualIndex >= 0)
					{
						CPedGroups::RemoveGroup(ActualIndex);
					}
				}
				break;

				//case CLEANUP_THREAT_LIST :
				//{
				//	CPedThreatLists::ms_threatList.RemoveList(Curr->Id);
				//}
				//break;
				
				case CLEANUP_SEQUENCE_TASK :
				{
					ActualIndex = CTheScripts::GetActualScriptThingIndex(Curr->Id, UNIQUE_SCRIPT_SEQUENCE_TASK);
					
					if (ActualIndex >= 0)
					{
						CTaskSequences::ms_taskSequence[ActualIndex].SetCanBeEmptied(true);	//	Flush();
						CTaskSequences::ms_bIsOpened[ActualIndex] = false;
						
						CTheScripts::ScriptSequenceTaskArray[ActualIndex].bUsed = false;
					}
				}
				break;
	
					
				case CLEANUP_ATTRACTOR :
				{
					ActualIndex = CTheScripts::GetActualScriptThingIndex(Curr->Id, UNIQUE_SCRIPT_ATTRACTOR);
					
					if (ActualIndex >= 0)
					{
						CScripted2dEffects::ms_activated[ActualIndex]=false;
					}
				}
				break;

				//case CLEANUP_FRIEND_LIST :
				//{
				//	CPedFriendLists::ms_friendList.RemoveList(Curr->Id);
				//}
				//break;
				
				case CLEANUP_SEARCHLIGHT :
				{
					ActualIndex = CTheScripts::GetActualScriptThingIndex(Curr->Id, UNIQUE_SCRIPT_SEARCHLIGHT);
					
					if (ActualIndex >= 0)
					{
						CTheScripts::RemoveScriptSearchLight(Curr->Id);
					}
				}
				break;
				
				case CLEANUP_CHECKPOINT :
				{
					ActualIndex = CTheScripts::GetActualScriptThingIndex(Curr->Id, UNIQUE_SCRIPT_CHECKPOINT);
					
					if (ActualIndex >= 0)
					{
						CTheScripts::RemoveScriptCheckpoint(Curr->Id);
					}
				}
				break;
				
				case CLEANUP_TEXTURE_DICTIONARY :
					CTheScripts::RemoveScriptTextureDictionary();
				break;

				default :
					ASSERTMSG(0, "CMissionCleanup::Process - Unknown Type");
					break;
			}

//			Curr->Id = 0;
//			Curr->Type = CLEANUP_UNUSED;
//			Count--;
			RemoveEntityFromList(Curr->Id, Curr->Type);
		}

		Curr++;
	}
	
	
	//Clear all relationships of all pedtypes to pedtype_mission1-8
	int pedtype;
	for(pedtype=PEDTYPE_PLAYER1;pedtype<PEDTYPE_MISSION1;pedtype++)
	{
		int otherpedtype;
		for(otherpedtype=PEDTYPE_MISSION1;otherpedtype<=PEDTYPE_MISSION8;otherpedtype++)
		{
			int acquaintancetype;
			for(acquaintancetype=ACQUAINTANCE_TYPE_PED_RESPECT;acquaintancetype<MAX_NUM_ACQUAINTANCE_TYPES;acquaintancetype++)
			{
				CPedType::ClearPedTypeAsAcquaintance(acquaintancetype,pedtype,CPedType::GetPedFlag(otherpedtype));
			}
		}
	}
	//Clear all relationships of all pedtype_mission1-8 to everything.
	for(pedtype=PEDTYPE_MISSION1;pedtype<=PEDTYPE_MISSION8;pedtype++)
	{
		int acquaintancetype;
		for(acquaintancetype=ACQUAINTANCE_TYPE_PED_RESPECT;acquaintancetype<MAX_NUM_ACQUAINTANCE_TYPES;acquaintancetype++)
		{
			CPedType::ClearPedTypeAcquaintances(acquaintancetype,pedtype);
		}
	}

	
#ifndef FINAL	
	// Print out all the models that scripts still require
	DEBUGLOG("SCRIPTS STILL REQUIRE:\n");
	for(int32 i=1; i<NUM_MODEL_INFOS; i++)
	{
		if(CStreaming::GetModelFlags(i) & STRFLAG_MISSION_REQUIRED)
			DEBUGLOG1("%s\n", CModelInfo::GetModelInfo(i)->GetModelName());
	}
	for(int32 i=1; i<ANIMMANAGER_MAXNOOFANIMBLOCKS; i++)
	{
		if(CStreaming::GetModelFlags(ANIM_INDEX_TO_INDEX(i)) & STRFLAG_MISSION_REQUIRED){
			DEBUGLOG1("anims %s\n", CAnimManager::GetAnimationBlock(i)->m_name);
		}	
	}
	DEBUGLOG("\n");
#endif

// CD protection thing.
#ifdef GTA_PC
#ifdef CDPROTECTION
	if ((CGeneral::GetRandomNumber() & 3) == 2)	// once every 8 times we get here.
	{
		_asm {
		push 850
		push 274
		push 337
		}
		if (GetLastError() != 850)
		{		// Security has been comprimised. Make the weather rainy all the time.
			CWeather::MessUpWeatherForCDProtection();
		}

		_asm {
//		pop
//		pop
//		pop
		add ESP, +12
		}
	}
#endif
#endif

}

// Class to handle checking for mission cars being stuck on their roofs

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CUpsideDownCarCheck::Init
// PURPOSE : Clears the array used to check for upside-down cars
/////////////////////////////////////////////////////////////////////////////////
void CUpsideDownCarCheck::Init(void)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_UPSIDEDOWN_CAR_CHECKS; loop++)
	{
		Cars[loop].CarID = -1;
		Cars[loop].UpsideDownTimer = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CUpsideDownCarCheck::IsCarUpsideDown
// PURPOSE : Returns TRUE if the specified car is upside-down and moving slowly
// INPUTS :	ID of car to check
// RETURNS : TRUE if the car is stuck on its roof, FALSE if the car is not on its 
//				roof or is moving at a reasonable speed
/////////////////////////////////////////////////////////////////////////////////
Bool8 CUpsideDownCarCheck::IsCarUpsideDown(Int32 CarToCheckID)
{
	CVehicle* pVehicleToCheck;
	pVehicleToCheck = CPools::GetVehiclePool().GetAt(CarToCheckID);
	ASSERTMSG(pVehicleToCheck, "CUpsideDownCarCheck::IsCarUpsideDown - Car doesn't exist");

	return IsCarUpsideDown(pVehicleToCheck);
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : As above, but takes pointer to vehicle
/////////////////////////////////////////////////////////////////////////////////

Bool8 CUpsideDownCarCheck::IsCarUpsideDown(const CVehicle* pVehicleToCheck)
{
	ASSERTMSG(pVehicleToCheck, "CUpsideDownCarCheck::IsCarUpsideDown - car doesn't exist");
//	CVector vCarUp = pVehicleToCheck->GetMatrix().GetUp();

	uint32 nNumWheelsOnGround = 0;
	if(pVehicleToCheck->GetBaseVehicleType()==VEHICLE_TYPE_CAR)
		nNumWheelsOnGround = ((CAutomobile *)pVehicleToCheck)->nNoOfContactWheels;
	else if(pVehicleToCheck->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
		nNumWheelsOnGround = ((CBike *)pVehicleToCheck)->nNoOfContactWheels;

	if((pVehicleToCheck->GetMatrix().GetUp().z < UPSIDEDOWN_CAR_UPZ_THRESHOLD && nNumWheelsOnGround < 4)
	|| pVehicleToCheck->GetMatrix().GetUp().z < 0.0f)
	{
		if(pVehicleToCheck->CanPedStepOutCar(false))
			return true;

//		if(pVehicleToCheck->GetMoveSpeed().Magnitude2D() < UPSIDEDOWN_CAR_MOVE_THRESHOLD_2D
//		&& CMaths::Abs(pVehicleToCheck->GetMoveSpeed().z) < UPSIDEDOWN_CAR_MOVE_THRESHOLD_Z
//		&& pVehicleToCheck->GetTurnSpeed().MagnitudeSqr() < UPSIDEDOWN_CAR_TURN_THRESHOLD*UPSIDEDOWN_CAR_TURN_THRESHOLD)
//		{
//			return true;
//		}
	}
	
	return false;

/*
	if(vCarUp.z <= UPSIDEDOWN_CAR_UPZ_THRESHOLD)	//	-0.97f)
	{			
		if (pVehicleToCheck->GetMoveSpeed().Magnitude() < UPSIDEDOWN_CAR_MOVE_THRESHOLD)	//	0.01f)
		{
			if (pVehicleToCheck->GetTurnSpeed().Magnitude() < UPSIDEDOWN_CAR_TURN_THRESHOLD)	//	0.005f)
			{
				return TRUE;
			}
		}
	}
*/
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CUpsideDownCarCheck::UpdateTimers
// PURPOSE : Increases the timer of all cars in the array which are currently upside down
/////////////////////////////////////////////////////////////////////////////////
void CUpsideDownCarCheck::UpdateTimers(void)
{
	UInt16 loop;
	UInt32 TimeStep;
	CVehicle *pVehicle;
	
	TimeStep = CTimer::GetTimeElapsedInMilliseconds();

	for (loop = 0; loop < MAX_UPSIDEDOWN_CAR_CHECKS; loop++)
	{
		pVehicle = CPools::GetVehiclePool().GetAt(Cars[loop].CarID);
		
		if (pVehicle)
		{
			if (IsCarUpsideDown(Cars[loop].CarID))
			{
				Cars[loop].UpsideDownTimer += TimeStep;
			}
			else
			{
				Cars[loop].UpsideDownTimer = 0;
			}	
		}
		else
		{	// Car is no longer on the map - remove it from the array of cars to check
			Cars[loop].CarID = -1;
			Cars[loop].UpsideDownTimer = 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CUpsideDownCarCheck::AreAnyCarsUpsideDown
// PURPOSE : Returns TRUE if any car in the array has been upside-down 
//				for a reasonable length of time
// RETURNS : TRUE if any car has been stuck on its roof for a while
//				FALSE otherwise
/////////////////////////////////////////////////////////////////////////////////
Bool8 CUpsideDownCarCheck::AreAnyCarsUpsideDown(void)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_UPSIDEDOWN_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID >= 0)
		{
			if (Cars[loop].UpsideDownTimer > UPSIDEDOWN_CAR_TIMER_THRESHOLD)
			{
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CUpsideDownCarCheck::AddCarToCheck
// PURPOSE : Adds a car to the array of cars to be checked for being upside-down
// INPUTS : ID of car to add
/////////////////////////////////////////////////////////////////////////////////
void CUpsideDownCarCheck::AddCarToCheck(Int32 NewCarID)
{
	UInt16 loop;
	
	loop = 0;

// Check car is not already in the array
	while (loop < MAX_UPSIDEDOWN_CAR_CHECKS)
	{
		ASSERTMSG((Cars[loop].CarID != NewCarID), "CUpsideDownCarCheck::AddCarToCheck - Trying to add car to upsidedown check again");
		loop++;
	}

	loop = 0;

// Find a space in the array for the car		
	while ((loop < MAX_UPSIDEDOWN_CAR_CHECKS) && (Cars[loop].CarID >= 0))
	{
		loop++;
	}
	
	ASSERTMSG((loop < MAX_UPSIDEDOWN_CAR_CHECKS), "CUpsideDownCarCheck::AddCarToCheck - Too many upsidedown car checks");

// Add the car to the array	
	if (loop < MAX_UPSIDEDOWN_CAR_CHECKS)
	{
		Cars[loop].CarID = NewCarID;
		Cars[loop].UpsideDownTimer = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CUpsideDownCarCheck::RemoveCarFromCheck
// PURPOSE : Removes a car from the array of cars to be checked (if it is there)
// INPUTS : ID of car to remove
/////////////////////////////////////////////////////////////////////////////////
void CUpsideDownCarCheck::RemoveCarFromCheck(Int32 RemoveCarID)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_UPSIDEDOWN_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID == RemoveCarID)
		{
			Cars[loop].CarID = -1;
			Cars[loop].UpsideDownTimer = 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CUpsideDownCarCheck::HasCarBeenUpsideDownForAWhile
// PURPOSE : Returns TRUE if the specified car (which must be in the array) 
//				has been upside-down for the required length of time
// INPUTS : ID of car to check (must have already been added to the array)
// RETURNS : TRUE if the car has been upside-down for a reasonable time
/////////////////////////////////////////////////////////////////////////////////
Bool8 CUpsideDownCarCheck::HasCarBeenUpsideDownForAWhile(Int32 CheckCarID)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_UPSIDEDOWN_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID == CheckCarID)
		{
			if (Cars[loop].UpsideDownTimer > UPSIDEDOWN_CAR_TIMER_THRESHOLD)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	
	ASSERTMSG(0, "CUpsideDownCarCheck::HasCarBeenUpsideDownForAWhile - Vehicle not in upsidedown check array");
	return FALSE;
}


// End of code for checking for upsidedown cars

// Class to handle checking for mission cars being stuck
#if defined (GTA_PS2)
#pragma optimize_for_size on
#pragma auto_inline off
#endif //GTA_PS2

void CStuckCarCheck::ResetArrayElement(UInt16 Index)
{
	Cars[Index].CarID = -1;
	Cars[Index].LastPos = CVector(-5000.0f, -5000.0f, -5000.0f);
	Cars[Index].LastChecked = -1;
	Cars[Index].StuckRadius = 0.0f;
	Cars[Index].CheckTime = 0;
	Cars[Index].bStuck = FALSE;
	
	Cars[Index].bWarpCar = false;
	Cars[Index].bWarpIfStuckFlag = false;
	Cars[Index].bWarpIfUpsideDownFlag = false;
	Cars[Index].bWarpIfInWaterFlag = false;
	Cars[Index].NumberOfNodesToCheck = 0;
}

void CStuckCarCheck::Init(void)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_STUCK_CAR_CHECKS; loop++)
	{
		ResetArrayElement(loop);
	}
}

bool CStuckCarCheck::AttemptToWarpVehicle(CVehicle *pVehicle, CVector &NewCoords, float NewHeading)
{
	float fRadiusForOffScreenChecks = 4.0f;	//	should this be available to the level designers too?
	float MinX, MinY, MinZ;
	float MaxX, MaxY, MaxZ;
	
	Int16 NumberOfEnts;

	if (TheCamera.IsSphereVisible(NewCoords, fRadiusForOffScreenChecks) == false)
	{
//		NewCoords.z += 1.0f;
		MinX = NewCoords.x - fRadiusForOffScreenChecks;
		MaxX = NewCoords.x + fRadiusForOffScreenChecks;
		
		MinY = NewCoords.y - fRadiusForOffScreenChecks;
		MaxY = NewCoords.y + fRadiusForOffScreenChecks;
		
		MinZ = NewCoords.z - fRadiusForOffScreenChecks;
		MaxZ = NewCoords.z + fRadiusForOffScreenChecks;
		
		CWorld::FindMissionEntitiesIntersectingCube(CVector(MinX, MinY, MinZ), 
													CVector(MaxX, MaxY, MaxZ), 
													&NumberOfEnts, 2, NULL, 
													TRUE, TRUE, TRUE);

		if (NumberOfEnts == 0)
		{
			CCarCtrl::SetCoordsOfScriptCar(pVehicle, NewCoords.x, NewCoords.y, NewCoords.z, true);	//	clear orientation to ensure the vehicle isn't upside-down
			pVehicle->SetHeading(DEGTORAD(NewHeading));
			return true;
		}
	}
	
	return false;
}

void CStuckCarCheck::Process(void)
{
	UInt32 TimeStep;
	UInt32 loop;
	
	CVehicle *pVehicle;
	CVector DiffVec;
	float DistanceMoved;
	
	Bool8 bAttemptToWarpThisCarNow;
	CVector NewCoords;
	float NewHeading;
	
	Bool8 bFound;
	UInt8 ClosestNodeIndex;
	CNodeAddress ResultNode;
	
	TimeStep = CTimer::GetTimeInMilliseconds();
	
	for (loop = 0; loop < MAX_STUCK_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID >= 0)
		{
			pVehicle = CPools::GetVehiclePool().GetAt(Cars[loop].CarID);
			
			if (pVehicle)
			{
				if (pVehicle->pDriver)
				{
					if (TimeStep > (Cars[loop].LastChecked + Cars[loop].CheckTime))
					{
						const CVector &NewPos = pVehicle->GetPosition();
						
						DiffVec = NewPos - Cars[loop].LastPos;
						DistanceMoved = DiffVec.Magnitude();
						
						if (DistanceMoved < Cars[loop].StuckRadius)
						{
							Cars[loop].bStuck = TRUE;
						}
						else
						{
							Cars[loop].bStuck = FALSE;
						}
						
						Cars[loop].LastPos = NewPos;
						Cars[loop].LastChecked = TimeStep;
					}
				
					if (Cars[loop].bWarpCar)
					{
						bAttemptToWarpThisCarNow = false;
						
						if (Cars[loop].bWarpIfStuckFlag)
						{
							if (Cars[loop].bStuck)
							{
								bAttemptToWarpThisCarNow = true;
							}
						}
						
						if (Cars[loop].bWarpIfUpsideDownFlag)
						{

	//						CVector vCarUp = pVehicle->GetMatrix().GetUp();
	//						if (vCarUp.z <= UPSIDEDOWN_CAR_UPZ_THRESHOLD)
	//						{
	//							bAttemptToWarpThisCarNow = true;
	//						}
							if(CTheScripts::UpsideDownCars.IsCarUpsideDown(pVehicle))
								bAttemptToWarpThisCarNow = true;
						}
						
						if (Cars[loop].bWarpIfInWaterFlag)
						{
							if (pVehicle->m_nPhysicalFlags.bIsInWater)
							{
								bAttemptToWarpThisCarNow = true;
							}
						}


						if (bAttemptToWarpThisCarNow)
						{
							if (TheCamera.IsSphereVisible(pVehicle->GetBoundCentre(), pVehicle->GetBoundRadius()) == false)
							{	//	car is offscreen

								if (Cars[loop].NumberOfNodesToCheck < 0)
								{	//	find the vehicle's last route coords
									CNodeAddress OldNode, NewNode;
									
									OldNode = pVehicle->AutoPilot.OldNode;
									NewNode = pVehicle->AutoPilot.NewNode;
						
									bool bSuccess;
									NewCoords = ThePaths.FindNodeCoorsForScript(OldNode, NewNode, &NewHeading, &bSuccess);
	//								ASSERT(bSuccess);
									if (bSuccess)
									{
										AttemptToWarpVehicle(pVehicle, NewCoords, NewHeading);
									}
								}
								else
								{	//	closest car nodes - check up to NumberOfNodesToCheck closest car nodes from its current position
									ClosestNodeIndex = 0;
									bFound = false;
									while ( (ClosestNodeIndex < Cars[loop].NumberOfNodesToCheck) && !bFound)
									{
										ResultNode = ThePaths.FindNthNodeClosestToCoors(pVehicle->GetPosition(), PFGRAPH_CARS, 999999.9f, false, true, ClosestNodeIndex);

										bool	bSuccess;
										NewCoords = ThePaths.FindNodeCoorsForScript(ResultNode, &bSuccess);
	//									ASSERT(bSuccess);	// Code needed to deal with this.

										if (bSuccess)
										{
											NewHeading = ThePaths.FindNodeOrientationForCarPlacement(ResultNode);	//	in degrees

											if (AttemptToWarpVehicle(pVehicle, NewCoords, NewHeading))
											{
												bFound = true;
											}
											else
											{
												ClosestNodeIndex++;
											}
										}
										else
										{
											ClosestNodeIndex++;
										}
									}
								}
							}
						}
					}	//	end of if (Cars[loop].bWarpCar)
				}	//	end of if (pVehicle->pDriver)
				else
				{	//	vehicle doesn't have a driver
					Cars[loop].LastPos = pVehicle->GetPosition();
					Cars[loop].LastChecked = TimeStep;
				}
			}
			else
			{	//	Car is no longer on the map - remove it from the array of cars to check
				ResetArrayElement(loop);
			}
		}	//	end of if (Cars[loop].CarID >= 0)
	}	//	end of for
}

void CStuckCarCheck::AddCarToCheck(Int32 NewCarID, float StuckRad, UInt32 CheckEvery, Bool8 bWarpCar, Bool8 bStuckFlag, Bool8 bUpsideDownFlag, Bool8 bInWaterFlag, Int8 NumberOfNodesToCheck)
{
	UInt16 loop;
	
	CVehicle *pVehicle;

	pVehicle = CPools::GetVehiclePool().GetAt(NewCarID);
			
	if (pVehicle)
	{	//	Only add the car if it is valid

		loop = 0;
	
	// Check car is not already in the array
		while (loop < MAX_STUCK_CAR_CHECKS)
		{
			ASSERTMSG( (Cars[loop].CarID != NewCarID), "CStuckCarCheck::AddCarToCheck - Trying to add car to stuck car check again");
			loop++;
		}
	
		loop = 0;
		
	// Find a space in the array for the car
		while ((loop < MAX_STUCK_CAR_CHECKS) && (Cars[loop].CarID >= 0))
		{
			loop++;
		}
		
		ASSERTMSG( (loop < MAX_STUCK_CAR_CHECKS), "CStuckCarCheck::AddCarToCheck - Too many stuck car checks");
	
	// Add the car to the array
		if (loop < MAX_STUCK_CAR_CHECKS)
		{
			Cars[loop].CarID = NewCarID;
			Cars[loop].LastPos = pVehicle->GetPosition();
			Cars[loop].LastChecked = CTimer::GetTimeInMilliseconds();
			Cars[loop].StuckRadius = StuckRad;
			Cars[loop].CheckTime = CheckEvery;
			Cars[loop].bStuck = FALSE;
			
			Cars[loop].bWarpCar = bWarpCar;
			Cars[loop].bWarpIfStuckFlag = bStuckFlag;
			Cars[loop].bWarpIfUpsideDownFlag = bUpsideDownFlag;
			Cars[loop].bWarpIfInWaterFlag = bInWaterFlag;
			Cars[loop].NumberOfNodesToCheck = NumberOfNodesToCheck;
		}
	}
}

void CStuckCarCheck::RemoveCarFromCheck(Int32 RemoveCarID)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_STUCK_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID == RemoveCarID)
		{
			ResetArrayElement(loop);
		}
	}
}

Bool8 CStuckCarCheck::HasCarBeenStuckForAWhile(Int32 CheckCarID)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_STUCK_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID == CheckCarID)
		{
			if (Cars[loop].bStuck)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	
	ASSERTMSG(0, "CStuckCarCheck::HasCarBeenStuckForAWhile - Vehicle not in stuck car check array");
	return FALSE;
}

//	Clears the stuck flag if the car is present in the stuck car check array
void CStuckCarCheck::ClearStuckFlagForCar(Int32 CheckCarID)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_STUCK_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID == CheckCarID)
		{
			Cars[loop].bStuck = false;
		}
	}
}


Bool8 CStuckCarCheck::IsCarInStuckCarArray(Int32 CheckCarID)
{
	UInt16 loop;
	
	for (loop = 0; loop < MAX_STUCK_CAR_CHECKS; loop++)
	{
		if (Cars[loop].CarID == CheckCarID)
		{
			return true;
		}
	}
	
	return false;
}

#if defined (GTA_PS2)
#pragma optimize_for_size reset
#pragma auto_inline reset
#endif //GTA_PS2
// End of code for checking for mission cars being stuck


Int32 *CRunningScript::GetPointerToLocalVariable(UInt32 VarIndex)
{
	if (ThisMustBeTheOnlyMissionRunning)
	{
		ASSERTOBJ(VarIndex < MAX_LOCAL_VARIABLES_FOR_A_MISSION_SCRIPT, ScriptName, "GetPointerToLocalVariable - Local Variable Index is out of range (mission script)");
		return (&CTheScripts::LocalVariablesForCurrentMission[VarIndex]);
	}
	else
	{
		ASSERTOBJ(VarIndex < (NUM_LOCAL_VARS+NUM_SPECIAL_LOCALS), ScriptName, "GetPointerToLocalVariable - Local Variable Index is out of range (non-mission script)");
		return (&Locals[VarIndex]);
	}
}

Int32 *CRunningScript::GetPointerToLocalArrayElement(UInt32 BaseAddress, UInt16 ElementNumber, UInt8 SizeOfEachElement)
{
	UInt32 VarIndex;
	
	VarIndex = BaseAddress + (SizeOfEachElement * ElementNumber);

	if (ThisMustBeTheOnlyMissionRunning)
	{
		ASSERTOBJ(VarIndex < MAX_LOCAL_VARIABLES_FOR_A_MISSION_SCRIPT, ScriptName, "GetPointerToLocalArrayElement - Local Variable Index is out of range (mission script)");
		return (&CTheScripts::LocalVariablesForCurrentMission[VarIndex]);
	}
	else
	{
//		SizeOfEachElement /= 4;
		ASSERTOBJ(VarIndex < (NUM_LOCAL_VARS+NUM_SPECIAL_LOCALS), ScriptName, "GetPointerToLocalArrayElement - Local Variable Index is out of range (non-mission script)");
		return (&Locals[VarIndex]);
	}
}

void CRunningScript::ReadArrayInformation(Int32 bUpdatePC, UInt16 *pReturnArrayBaseAddress, Int32 *pReturnArrayIndex)
{
	UInt8 *TempPCPointer = PCPointer;
//	Int8 ArgType;
	UInt16 VarIndex;
	
	UInt16 ArrayInfo;
	UInt16 TypeOfArrayElements, SizeOfThisArray;
	
	Int32 *pLocalVariable;

	
	*pReturnArrayBaseAddress = CTheScripts::ReadUInt16FromScript(TempPCPointer);	//	could be global or local
	VarIndex = CTheScripts::ReadUInt16FromScript(TempPCPointer);	//	could be global or local
	ArrayInfo = CTheScripts::ReadUInt16FromScript(TempPCPointer);
	if (ArrayInfo & 0x08000)
	{	//	Array Index is a global variable
		ArrayInfo &= 0x07fff;
		*pReturnArrayIndex = *((Int32 *)&CTheScripts::ScriptSpace[VarIndex]);
	}
	else
	{
		pLocalVariable = GetPointerToLocalVariable(VarIndex);
		*pReturnArrayIndex = *pLocalVariable;
	}
	TypeOfArrayElements = (ArrayInfo >> 8);	//	should be VAR_TEXT_LABEL or VAR_TEXT_LABEL16
	SizeOfThisArray = ArrayInfo & 0x0ff;
	//	Check ArrayIndex is within the range of this array
	if (*pReturnArrayIndex < 0)
	{
#ifndef FINAL
		sprintf(CTheScripts::ScriptErrorMessage, "ReadArrayInformation - Array index is less than 0, ArrayBase %d Variable %d", *pReturnArrayBaseAddress, VarIndex);
		ASSERTOBJ(0, ScriptName, CTheScripts::ScriptErrorMessage);
#endif
	}
	
	if (*pReturnArrayIndex >= SizeOfThisArray)
	{
#ifndef FINAL
		sprintf(CTheScripts::ScriptErrorMessage, "ReadArrayInformation - Array index is greater than or equal to the size of the array, ArrayBase %d Variable %d", *pReturnArrayBaseAddress, VarIndex);
		ASSERTOBJ(0, ScriptName, CTheScripts::ScriptErrorMessage);
#endif
	}
	
	if (bUpdatePC)
	{
		PCPointer = TempPCPointer;
	}
}


void CRunningScript::ReadTextLabelFromScript(Char *pStringToFill, UInt8 MaxStringLength)
{
	Int8 ArgType;
	UInt16 VarIndex;
	
	UInt16 ArrayBaseAddress;	//	, ArrayInfo;
	Int32 ArrayIndex;
//	UInt16 TypeOfArrayElements, SizeOfThisArray;
	Int16 LengthOfString;
	UInt8 CharLoop;
	
	Int32 *pLocalVariable;
	
//	strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), 8);
//	*pPC += 8;

	ASSERTOBJ(MaxStringLength >= 8, ScriptName, "CRunningScript::ReadTextLabelFromScript - need space for at least 8 characters when reading a string");

	ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
	
	switch (ArgType)
	{
		case ARGLIST_TEXT_LABEL :
//			strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), 8);
//			*pPC += 8;
			for (CharLoop = 0; CharLoop < 8; CharLoop++)
			{
				pStringToFill[CharLoop] = CTheScripts::ReadInt8FromScript(PCPointer);
			}
			break;
			
		case ARGLIST_GLOBAL_TEXT_LABEL_VARIABLE :
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[VarIndex]), 8);
			break;
			
		case ARGLIST_LOCAL_TEXT_LABEL_VARIABLE :
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			pLocalVariable = GetPointerToLocalVariable(VarIndex);
			strncpy(pStringToFill, ( (char *) pLocalVariable), 8);
			break;
		
		case ARGLIST_GLOBAL_TEXT_LABEL_ARRAY :
		case ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY :
			ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);

			//	Size of each element is 8 or 16
			if (ArgType == ARGLIST_GLOBAL_TEXT_LABEL_ARRAY)
			{
				strncpy(pStringToFill, ((char *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (8 * ArrayIndex)]), 8);
			}
			else
			{
//				ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a global array element containing a 16 character string");
				ASSERTOBJ(ArgType == ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY, ScriptName, "CRunningScript::ReadTextLabelFromScript - expected argument type to be ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY");
				
				if (MaxStringLength < 16)
				{
					strncpy(pStringToFill, ((char *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (16 * ArrayIndex)]), MaxStringLength);
				}
				else
				{
					strncpy(pStringToFill, ((char *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (16 * ArrayIndex)]), 16);
				}
			}
			break;
		
		case ARGLIST_LOCAL_TEXT_LABEL_ARRAY :
		case ARGLIST_LOCAL_TEXT_LABEL16_ARRAY :
			ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);

			//	Size of each element is 8 or 16
			if (ArgType == ARGLIST_LOCAL_TEXT_LABEL_ARRAY)
			{
				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 2);
				strncpy(pStringToFill, ((char *) pLocalVariable), 8);
			}
			else
			{
//				ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a local array element containing a 16 character string");
				ASSERTOBJ(ArgType == ARGLIST_LOCAL_TEXT_LABEL16_ARRAY, ScriptName, "CRunningScript::ReadTextLabelFromScript - expected argument type to be ARGLIST_LOCAL_TEXT_LABEL16_ARRAY");

				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 4);
				
				if (MaxStringLength < 16)
				{
					strncpy(pStringToFill, ((char *) pLocalVariable), MaxStringLength);
				}
				else
				{
					strncpy(pStringToFill, ((char *) pLocalVariable), 16);
				}
			}
			break;
			
		case ARGLIST_LONG_TEXT_LABEL :
			LengthOfString = CTheScripts::ReadInt8FromScript(PCPointer);
			if (LengthOfString < 1)
			{
				ASSERTOBJ(0, ScriptName, "CRunningScript::ReadTextLabelFromScript - Length of long string is less than 1");
			}
			
			if (LengthOfString >= MaxStringLength)
			{
				ASSERTOBJ(0, ScriptName, "CRunningScript::ReadTextLabelFromScript - Long string is too long to be stored");
			}
			//	strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), LengthOfString);
			//	*pPC += LengthOfString;
			
			for (CharLoop = 0; CharLoop < LengthOfString; CharLoop++)
			{
				pStringToFill[CharLoop] = CTheScripts::ReadInt8FromScript(PCPointer);
			}
			//	Fill the rest of the string with 0s
			for (CharLoop = LengthOfString; CharLoop < MaxStringLength; CharLoop++)
			{
				pStringToFill[CharLoop] = 0;
			}

			break;
			
		case ARGLIST_TEXT_LABEL16 :
//			ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a 16 character string");
			if (MaxStringLength < 16)
			{
//				strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), MaxStringLength);
				for (CharLoop = 0; CharLoop < MaxStringLength; CharLoop++)
				{
					pStringToFill[CharLoop] = CTheScripts::ReadInt8FromScript(PCPointer);
				}
				while (CharLoop < 16)
				{	//	Is this right? This is how the old code worked - Program Counter was always increased by 16
					PCPointer++;
					CharLoop++;
				}
			}
			else
			{
//				strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), 16);
				for (CharLoop = 0; CharLoop < 16; CharLoop++)
				{
					pStringToFill[CharLoop] = CTheScripts::ReadInt8FromScript(PCPointer);
				}
			}
//			*pPC += 16;
			break;

		case ARGLIST_GLOBAL_TEXT_LABEL16_VARIABLE :
//			ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a global variable containing a 16 character string");
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			if (MaxStringLength < 16)
			{
				strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[VarIndex]), MaxStringLength);
			}
			else
			{
				strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[VarIndex]), 16);
			}
			break;
			
		case ARGLIST_LOCAL_TEXT_LABEL16_VARIABLE :
//			ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a local variable containing a 16 character string");
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			
			pLocalVariable = GetPointerToLocalVariable(VarIndex);
			if (MaxStringLength < 16)
			{
				strncpy(pStringToFill, ( (char *) pLocalVariable), MaxStringLength);
			}
			else
			{
				strncpy(pStringToFill, ( (char *) pLocalVariable), 16);
			}
			break;

		default:
			ASSERTOBJ(0, ScriptName, "CRunningScript::ReadTextLabelFromScript - Unknown parameter type");
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CollectParameters
// PURPOSE :  Collects the next so-many parameters from the script
/////////////////////////////////////////////////////////////////////////////////

void CRunningScript::CollectParameters(Int16 Number)
{
	Int8 ArgType;
	UInt16 VarIndex;
//	float TempFloat;
	Int32* pScriptParams = &ScriptParams[0];

	UInt16 ArrayBaseAddress;	//	, ArrayInfo;
	Int32 ArrayIndex;
//	UInt16 TypeOfArrayElements, SizeOfThisArray;
	
	Int32 *pLocalVariable;

	while(Number--)
	{
		ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
		
		switch (ArgType)
		{
			case ARGLIST_VALUE:
				*pScriptParams = CTheScripts::ReadInt32FromScript(PCPointer);
				//	Maybe should check for int/float
				break;
			case ARGLIST_GLOBAL:
				VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
				*pScriptParams = *((Int32 *)&CTheScripts::ScriptSpace[VarIndex]);
				break;
			case ARGLIST_LOCAL:
				VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
				pLocalVariable = GetPointerToLocalVariable(VarIndex);
				*pScriptParams = *pLocalVariable;
				break;
			case ARGLIST_VALUE8 :
				*pScriptParams = CTheScripts::ReadInt8FromScript(PCPointer);
				break;
			case ARGLIST_VALUE16 :
				*pScriptParams = CTheScripts::ReadInt16FromScript(PCPointer);
				break;
			case ARGLIST_FLOAT :
//				don't need to read as a float because we are instantly converting to an int32			
//				TempFloat = CTheScripts::ReadFloatFromScript(pPC);
//				*pScriptParams = *((Int32 *) &TempFloat);
				*pScriptParams = CTheScripts::ReadInt32FromScript(PCPointer);
				break;
			case ARGLIST_GLOBAL_ARRAY :

				ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);
				
				*pScriptParams = *((Int32 *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (4 * ArrayIndex)]);
				break;
			case ARGLIST_LOCAL_ARRAY :

				ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);
				
				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 1);
				*pScriptParams = *pLocalVariable;
				break;

			default:
				ASSERTOBJ(0, ScriptName, "CRunningScript::CollectParameters - Unknown parameter type");
				break;
		}
		pScriptParams++;
	}
}

Int32 CRunningScript::CollectNextParameterWithoutIncreasingPC(void)
{
	Int8 ArgType;
	UInt16 VarIndex;
	Int32 ArgValue = -1;
	Int16 ArgValue16;
	Int8 ArgValue8;
//	float ArgFloat;
	
	UInt16 ArrayBaseAddress;	//	, ArrayInfo;
	Int32 ArrayIndex;
//	UInt16 TypeOfArrayElements, SizeOfThisArray;
	
	Int32 *pLocalVariable;
	
	UInt8 *TempPCPointer = PCPointer;
	
	ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
	
	switch(ArgType)
	{
		case ARGLIST_VALUE:
			ArgValue = CTheScripts::ReadInt32FromScript(PCPointer);
			break;
			
		case ARGLIST_GLOBAL:
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			ArgValue = *((Int32 *)&CTheScripts::ScriptSpace[VarIndex]);
			break;
			
		case ARGLIST_LOCAL:
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			pLocalVariable = GetPointerToLocalVariable(VarIndex);
			ArgValue = *pLocalVariable;
			break;
		case ARGLIST_VALUE8 :
			ArgValue8 = CTheScripts::ReadInt8FromScript(PCPointer);
			ArgValue = (Int32) ArgValue8;
			break;
		case ARGLIST_VALUE16 :
			ArgValue16 = CTheScripts::ReadInt16FromScript(PCPointer);
			ArgValue = (Int32) ArgValue16;
			break;
		case ARGLIST_FLOAT :
//			don't need to read as a float because we are instantly converting to an int32
//			ArgFloat = CTheScripts::ReadFloatFromScript(&PC);
//			ArgValue = *((Int32 *)&ArgFloat);
			ArgValue = CTheScripts::ReadInt32FromScript(PCPointer);
			break;
		case ARGLIST_GLOBAL_ARRAY :

			ReadArrayInformation(false, &ArrayBaseAddress, &ArrayIndex);

			ArgValue = *((Int32 *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (4 * ArrayIndex)]);
			break;
		case ARGLIST_LOCAL_ARRAY :

			ReadArrayInformation(false, &ArrayBaseAddress, &ArrayIndex);
			
			pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 1);
			ArgValue = *pLocalVariable;
			break;
			
		default:
			ASSERTOBJ(0, ScriptName, "CRunningScript::CollectNextParameterWithoutIncreasingPC - Unknown parameter type");
			break;
	}
	
	PCPointer = TempPCPointer;
	
	return ArgValue;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : StoreParameters
// PURPOSE :  Stores the parameters that have been collected in their proper place
/////////////////////////////////////////////////////////////////////////////////

void CRunningScript::StoreParameters(Int16 Number)
{
	Int16	C;
	Int8 ArgType;
	UInt16 VarIndex;

	UInt16 ArrayBaseAddress;	//	, ArrayInfo;
	Int32 ArrayIndex;
//	UInt16 TypeOfArrayElements, SizeOfThisArray;
	
	Int32 *pLocalVariable;

	for (C = 0; C < Number; C++)
	{
		ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
		
		switch(ArgType)
		{
			case ARGLIST_VALUE:
			case ARGLIST_VALUE8:
			case ARGLIST_VALUE16:
			case ARGLIST_FLOAT:
				ASSERTOBJ(0, ScriptName, "CRunningScript::StoreParameters - Can't store a value in a number");
				break;
			case ARGLIST_GLOBAL:
				VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
				*((Int32 *)&CTheScripts::ScriptSpace[VarIndex]) = ScriptParams[C];
				break;
			case ARGLIST_LOCAL:
				VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
				pLocalVariable = GetPointerToLocalVariable(VarIndex);
				*pLocalVariable = ScriptParams[C];
				break;
			case ARGLIST_GLOBAL_ARRAY :

				ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);

				*((Int32 *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (4 * ArrayIndex)]) = ScriptParams[C];
				break;
			case ARGLIST_LOCAL_ARRAY :

				ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);
				
				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 1);
				*pLocalVariable = ScriptParams[C];
				break;
				
			default:
				ASSERTOBJ(0, ScriptName, "CRunningScript::StoreParameters - Unknown parameter type");
				break;
		}
	}
}


void CRunningScript::ReadParametersForNewlyStartedScript(CRunningScript *pNewScript)
{
	Int32 *pLocals;
//	float ReturnFloat;
	Int8 ArgType;
	UInt16 VarIndex;
	UInt16 ArrayBaseAddress;
	Int32 ArrayIndex;
	Int32 *pLocalVariable;


	// Read the argument list and punt the values into the local
	// variables of the script that has just been created.
	pLocals = &pNewScript->Locals[0];
	
	ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
	
	while (ArgType != ARGLIST_END)
	{
		switch (ArgType)
		{
			case ARGLIST_VALUE:
				*pLocals = CTheScripts::ReadInt32FromScript(PCPointer);
				break;
			case ARGLIST_GLOBAL:
				VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
				*pLocals = *((Int32 *)&CTheScripts::ScriptSpace[VarIndex]);
				break;
			case ARGLIST_LOCAL:
				VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
				pLocalVariable = GetPointerToLocalVariable(VarIndex);
				*pLocals = *pLocalVariable;
				break;
			case ARGLIST_VALUE8:
				*pLocals = CTheScripts::ReadInt8FromScript(PCPointer);
				break;
			case ARGLIST_VALUE16:
				*pLocals = CTheScripts::ReadInt16FromScript(PCPointer);
				break;
			case ARGLIST_FLOAT:
				// don't need to read as a float because we are instantly converting to an int32
//							ReturnFloat = CTheScripts::ReadFloatFromScript(&PC);
//							*pLocals = *((Int32 *)&ReturnFloat);
				*pLocals = CTheScripts::ReadInt32FromScript(PCPointer);
				break;
				
			case ARGLIST_GLOBAL_ARRAY :
				ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);
				*pLocals = *((Int32 *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (4 * ArrayIndex)]);
				break;

			case ARGLIST_LOCAL_ARRAY :
				ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);
				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 1);
				*pLocals = *pLocalVariable;
				break;
							
// START_NEW_SCRIPT can't handle strings being passed in
/*
			case ARGLIST_TEXT_LABEL :
				strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), 8);
				*pPC += 8;
				break;
			case ARGLIST_TEXT_LABEL16 :
	//			ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a 16 character string");
				if (MaxStringLength < 16)
				{
					strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), MaxStringLength);
				}
				else
				{
					strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), 16);
				}
				*pPC += 16;
				break;
			case ARGLIST_LONG_TEXT_LABEL :
				LengthOfString = CTheScripts::ReadInt8FromScript(pPC);
				if (LengthOfString < 1)
				{
					ASSERTOBJ(0, ScriptName, "CRunningScript::ReadTextLabelFromScript - Length of long string is less than 1");
				}
				
				if (LengthOfString >= MaxStringLength)
				{
					ASSERTOBJ(0, ScriptName, "CRunningScript::ReadTextLabelFromScript - Long string is too long to be stored");
				}
				strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[PC]), LengthOfString);
				//	Fill the rest of the string with 0s
				for (CharLoop = LengthOfString; CharLoop < MaxStringLength; CharLoop++)
				{
					pStringToFill[CharLoop] = 0;
				}
				*pPC += LengthOfString;
				break;
			case ARGLIST_GLOBAL_TEXT_LABEL_VARIABLE :
				VarIndex = CTheScripts::ReadUInt16FromScript(pPC);
				strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[VarIndex]), 8);
				break;
			case ARGLIST_GLOBAL_TEXT_LABEL16_VARIABLE :
	//			ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a global variable containing a 16 character string");
				VarIndex = CTheScripts::ReadUInt16FromScript(pPC);
				if (MaxStringLength < 16)
				{
					strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[VarIndex]), MaxStringLength);
				}
				else
				{
					strncpy(pStringToFill, ( (char *) &CTheScripts::ScriptSpace[VarIndex]), 16);
				}
				break;
			case ARGLIST_GLOBAL_TEXT_LABEL_ARRAY :
			case ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY :
				ReadArrayInformation(pPC, true, &ArrayBaseAddress, &ArrayIndex);

				//	Size of each element is 8 or 16
				if (ArgType == ARGLIST_GLOBAL_TEXT_LABEL_ARRAY)
				{
					strncpy(pStringToFill, ((char *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (8 * ArrayIndex)]), 8);
				}
				else
				{
	//				ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a global array element containing a 16 character string");
					ASSERTOBJ(ArgType == ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY, ScriptName, "CRunningScript::ReadTextLabelFromScript - expected argument type to be ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY");
					
					if (MaxStringLength < 16)
					{
						strncpy(pStringToFill, ((char *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (16 * ArrayIndex)]), MaxStringLength);
					}
					else
					{
						strncpy(pStringToFill, ((char *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (16 * ArrayIndex)]), 16);
					}
				}
				break;

			case ARGLIST_LOCAL_TEXT_LABEL_VARIABLE :
				VarIndex = CTheScripts::ReadUInt16FromScript(pPC);
				pLocalVariable = GetPointerToLocalVariable(VarIndex);
				strncpy(pStringToFill, ( (char *) pLocalVariable), 8);
				break;
			case ARGLIST_LOCAL_TEXT_LABEL16_VARIABLE :
	//			ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a local variable containing a 16 character string");
				VarIndex = CTheScripts::ReadUInt16FromScript(pPC);
				pLocalVariable = GetPointerToLocalVariable(VarIndex);
				if (MaxStringLength < 16)
				{
					strncpy(pStringToFill, ( (char *) pLocalVariable), MaxStringLength);
				}
				else
				{
					strncpy(pStringToFill, ( (char *) pLocalVariable), 16);
				}
				break;
			case ARGLIST_LOCAL_TEXT_LABEL_ARRAY :
			case ARGLIST_LOCAL_TEXT_LABEL16_ARRAY :
				ReadArrayInformation(pPC, true, &ArrayBaseAddress, &ArrayIndex);

				//	Size of each element is 8 or 16
				if (ArgType == ARGLIST_LOCAL_TEXT_LABEL_ARRAY)
				{
					pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 2);
					strncpy(pStringToFill, ((char *) pLocalVariable), 8);
				}
				else
				{
	//				ASSERTOBJ(MaxStringLength > 16, ScriptName, "CRunningScript::ReadTextLabelFromScript - not enough space to read a local array element containing a 16 character string");
					ASSERTOBJ(ArgType == ARGLIST_LOCAL_TEXT_LABEL16_ARRAY, ScriptName, "CRunningScript::ReadTextLabelFromScript - expected argument type to be ARGLIST_LOCAL_TEXT_LABEL16_ARRAY");

					pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 4);
					
					if (MaxStringLength < 16)
					{
						strncpy(pStringToFill, ((char *) pLocalVariable), MaxStringLength);
					}
					else
					{
						strncpy(pStringToFill, ((char *) pLocalVariable), 16);
					}
				}
				break;
*/

			default:
				ASSERTOBJ(0, ScriptName, "START_NEW_SCRIPT - Unknown argument type");
				break;
		}
		pLocals++;
		ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
	}
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : PrintListSizes
// PURPOSE :  Prints some debug info (the sizes of the lists)
/////////////////////////////////////////////////////////////////////////////////

void CTheScripts::PrintListSizes(void)
{
	Int16			Actives = 0;
	Int16			Idles = 0;
	CRunningScript	*pNode;

	pNode = CTheScripts::pActiveScripts;
	while (pNode)
	{
		Actives++;
		pNode = pNode->pNext;
	}
	pNode = CTheScripts::pIdleScripts;
	while (pNode)
	{
		Idles++;
		pNode = pNode->pNext;
	}

//	sprintf(Str, "********* Actives:%d Idles:%d", Actives, Idles );
//	DEBUGLOG(Str);

}


UInt16 CRunningScript::GetIndexOfGlobalVariable(void)
{
	Int8 ArgType;
	UInt16 VarIndex;
	
	UInt16 ArrayBaseAddress;
	Int32 ArrayIndex;
	
	ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
	
//	ASSERTOBJ(ArgType == ARGLIST_GLOBAL, ScriptName, "GetIndexOfGlobalVariable - Expected a global variable");
	switch (ArgType)
	{
		case ARGLIST_GLOBAL :
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			break;
			
		case ARGLIST_GLOBAL_ARRAY :
			ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);

			VarIndex = ArrayBaseAddress + (4 * ArrayIndex);
			break;
			
		default :
			ASSERTOBJ(0, ScriptName, "GetIndexOfGlobalVariable - Expected a global number variable or array (not text, not local)");
			break;
	}
	
//	return ((Int32 *) &CTheScripts::ScriptSpace[VarIndex]);
	return VarIndex;
}


Int32 *CRunningScript::GetPointerToScriptVariable(UInt8 VariableScope)
{
	Int8 ArgType;
	UInt16 VarIndex;
	
	UInt16 ArrayBaseAddress;	//	, ArrayInfo;
	Int32 ArrayIndex;
//	UInt16 TypeOfArrayElements, SizeOfThisArray;
	
	Int32 *pLocalVariable;
	
	ArgType = CTheScripts::ReadInt8FromScript(PCPointer);
	
#ifndef FINALBUILD	
	switch (VariableScope)
	{
		case SCOPE_LOCAL :
			ASSERTOBJ( (ArgType == ARGLIST_LOCAL) || (ArgType == ARGLIST_LOCAL_ARRAY) 
					|| (ArgType == ARGLIST_LOCAL_TEXT_LABEL_VARIABLE) || (ArgType == ARGLIST_LOCAL_TEXT_LABEL_ARRAY) 
					|| (ArgType == ARGLIST_LOCAL_TEXT_LABEL16_VARIABLE) || (ArgType == ARGLIST_LOCAL_TEXT_LABEL16_ARRAY), 
					ScriptName, "GetPointerToScriptVariable - Expected a local variable");
			break;
		case SCOPE_GLOBAL :
			ASSERTOBJ( (ArgType == ARGLIST_GLOBAL) || (ArgType == ARGLIST_GLOBAL_ARRAY) 
					|| (ArgType == ARGLIST_GLOBAL_TEXT_LABEL_VARIABLE) || (ArgType == ARGLIST_GLOBAL_TEXT_LABEL_ARRAY) 
					|| (ArgType == ARGLIST_GLOBAL_TEXT_LABEL16_VARIABLE) || (ArgType == ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY), 
					ScriptName, "GetPointerToScriptVariable - Expected a global variable");
			break;
		case SCOPE_DONTMATTER :
			break;
	}
#endif

	switch (ArgType)
	{
		case ARGLIST_GLOBAL:
		case ARGLIST_GLOBAL_TEXT_LABEL_VARIABLE :
		case ARGLIST_GLOBAL_TEXT_LABEL16_VARIABLE :
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			return ((Int32 *) &CTheScripts::ScriptSpace[VarIndex]);
			break;

		case ARGLIST_LOCAL:
		case ARGLIST_LOCAL_TEXT_LABEL_VARIABLE :
		case ARGLIST_LOCAL_TEXT_LABEL16_VARIABLE :
			VarIndex = CTheScripts::ReadUInt16FromScript(PCPointer);
			pLocalVariable = GetPointerToLocalVariable(VarIndex);
			return pLocalVariable;
			break;

		case ARGLIST_GLOBAL_ARRAY :
		case ARGLIST_GLOBAL_TEXT_LABEL_ARRAY :
		case ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY :

			ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);

			if (ArgType == ARGLIST_GLOBAL_TEXT_LABEL16_ARRAY)
			{
				return ((Int32 *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (16 * ArrayIndex)]);
			}
			else if (ArgType == ARGLIST_GLOBAL_TEXT_LABEL_ARRAY)
			{
				return ((Int32 *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (8 * ArrayIndex)]);
			}
			else
			{
				return ((Int32 *)&CTheScripts::ScriptSpace[ArrayBaseAddress + (4 * ArrayIndex)]);
			}
			break;
		case ARGLIST_LOCAL_ARRAY :
		case ARGLIST_LOCAL_TEXT_LABEL_ARRAY :
		case ARGLIST_LOCAL_TEXT_LABEL16_ARRAY :

			ReadArrayInformation(true, &ArrayBaseAddress, &ArrayIndex);

			if (ArgType == ARGLIST_LOCAL_TEXT_LABEL16_ARRAY)
			{
				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 4);
			}
			else if (ArgType == ARGLIST_LOCAL_TEXT_LABEL_ARRAY)
			{
				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 2);
			}
			else
			{
				pLocalVariable = GetPointerToLocalArrayElement(ArrayBaseAddress, ArrayIndex, 1);
			}

			return pLocalVariable;
			
			break;
			
		default:
			ASSERTOBJ(0, ScriptName, "CRunningScript::GetPointerToScriptVariable - Unknown parameter type");
			break;
	}
	
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : Init
// PURPOSE :  Init this one script.
/////////////////////////////////////////////////////////////////////////////////

void CRunningScript::Init(void)
{
	UInt16 loop;
	
	strcpy(ScriptName, "noname");
	
	BaseAddressOfThisScript = NULL;
	
	pNext = pPrevious = NULL;
	PCPointer = NULL;
	
	for (loop = 0; loop < MAX_STACK_DEPTH; loop++)
	{
		PCStack[loop] = NULL;
	}
	StackDepth = 0;
	ActivateTime = 0;
//	CmpFlag = DbgFlag = FALSE;			// debugging off at start
	bActive = false;
	CmpFlag = FALSE;
	IsThisAMissionScript = FALSE;
	bIsThisAStreamedScript = false;
	bIsThisAMiniGameScript = false;
	ScriptBrainType = -1;
//	IsMessageWait = FALSE;
	for (loop = 0; loop < (NUM_LOCAL_VARS + NUM_SPECIAL_LOCALS); loop++)
	{
//		Locals[NUM_LOCAL_VARS] = 0;			// reset timerA
//		Locals[NUM_LOCAL_VARS+1] = 0;		// reset timerB
		Locals[loop] = 0;
	}
	AndOrState = 0;
	NotForLatestExpression = FALSE;
	
	DeatharrestCheckEnabled = TRUE;
	DoneDeatharrest = FALSE;
	
	EndOfScriptedCutscenePC = 0;
/*
#ifdef DEBUG
	TotalCommandsProcessed = 0;
	NumberOfFramesOfProcessing = 0;
#endif
*/
	ThisMustBeTheOnlyMissionRunning = FALSE;
}

void CTheScripts::RemoveScriptTextureDictionary(void)
{
	UInt16 loop;
	Int32 txdIndex;
	
	// remove handles to sprites
	for(loop = 0; loop < NUM_SCRIPT_SPRITES; loop++)
		CTheScripts::ScriptSprites[loop].Delete();
	
	txdIndex = CTxdStore::FindTxdSlot("script");

	if (txdIndex != -1)
	{
		if (CTxdStore::GetTxd(txdIndex))
		{
			CTxdStore::RemoveTxd(txdIndex);
		}
	}
}

void CTheScripts::DrawScriptSpritesAndRectangles(Bool8 bBeforeFade)
{
	UInt16 loop;
	float CentreX, CentreY, Width, Height;
	float x1, y1, x2, y2, x3, y3, x4, y4;
	float cos_float, sin_float;
	
	for (loop = 0; loop < NUM_SCRIPT_RECTANGLES; loop++)
	{

		if (CTheScripts::IntroRectangles[loop].ScriptRectBeforeFade == bBeforeFade)
		{
			switch(CTheScripts::IntroRectangles[loop].eWindowType)
			{
				case WINDOW_NONE :
					// Do Nothing
				break;
				case WINDOW_HEADER_AND_TEXT :
				{	 // window with text inside (fancy)
					ASSERT(CTheScripts::IntroRectangles[loop].pMessage);
					FrontEndMenuManager.DrawWindowedText(	CTheScripts::IntroRectangles[loop].ScriptRectMinX,
															CTheScripts::IntroRectangles[loop].ScriptRectMinY,
															CTheScripts::IntroRectangles[loop].ScriptRectMaxX, 
															CTheScripts::IntroRectangles[loop].pTitle,
															CTheScripts::IntroRectangles[loop].pMessage,
				     										CTheScripts::IntroRectangles[loop].nAlignment);
				}
				break;
				case WINDOW_HEADER_NO_TEXT :
				{	//	Draw a nice box with a header (no text) (fancy)
//					ASSERT(CTheScripts::IntroRectangles[loop].pTitle);
					FrontEndMenuManager.DrawWindow( CRect(CTheScripts::IntroRectangles[loop].ScriptRectMinX, 
													CTheScripts::IntroRectangles[loop].ScriptRectMinY, 
													CTheScripts::IntroRectangles[loop].ScriptRectMaxX, 
													CTheScripts::IntroRectangles[loop].ScriptRectMaxY),
													CTheScripts::IntroRectangles[loop].pTitle,
													0,
													CRGBA(0, 0, 0, 190),
													CTheScripts::IntroRectangles[loop].nSwirl);
				}
				break;
				case WINDOW_SOLID_COLOUR :
				{	//	Draw a rectangle of solid colour (not fancy)
					ASSERT(CTheScripts::IntroRectangles[loop].ScriptSpriteIndex < 0);
					CSprite2d::DrawRect ( CRect(CTheScripts::IntroRectangles[loop].ScriptRectMinX, 
												CTheScripts::IntroRectangles[loop].ScriptRectMinY, 
												CTheScripts::IntroRectangles[loop].ScriptRectMaxX, 
												CTheScripts::IntroRectangles[loop].ScriptRectMaxY), 
										CTheScripts::IntroRectangles[loop].ScriptRectColour );
				}
				break;
				case WINDOW_SPRITE_NO_ROTATION :
				{	//	Draw a sprite no rotation
					ASSERT (CTheScripts::IntroRectangles[loop].ScriptRectRotation == 0.0f);
					CTheScripts::ScriptSprites[CTheScripts::IntroRectangles[loop].ScriptSpriteIndex].Draw(
						CRect(CTheScripts::IntroRectangles[loop].ScriptRectMinX, 
								CTheScripts::IntroRectangles[loop].ScriptRectMinY, 
								CTheScripts::IntroRectangles[loop].ScriptRectMaxX, 
								CTheScripts::IntroRectangles[loop].ScriptRectMaxY), 
						CTheScripts::IntroRectangles[loop].ScriptRectColour );
					}
				break;
				case WINDOW_SPRITE_WITH_ROTATION :
				{
					// script sprites with rotation
					CentreX = (CTheScripts::IntroRectangles[loop].ScriptRectMinX + CTheScripts::IntroRectangles[loop].ScriptRectMaxX) / 2.0f;
					CentreY = (CTheScripts::IntroRectangles[loop].ScriptRectMinY + CTheScripts::IntroRectangles[loop].ScriptRectMaxY) / 2.0f;
					Width = CentreX - CTheScripts::IntroRectangles[loop].ScriptRectMinX;
					Height = CentreY - CTheScripts::IntroRectangles[loop].ScriptRectMinY;

					cos_float = CMaths::Cos(CTheScripts::IntroRectangles[loop].ScriptRectRotation);
					sin_float = CMaths::Sin(CTheScripts::IntroRectangles[loop].ScriptRectRotation);

					x1 = (-Width * cos_float) + (Height * sin_float);
					y1 = (-Width * sin_float) - (Height * cos_float);
					x1 += CentreX;
					y1 += CentreY;

					x2 = (Width * cos_float) + (Height * sin_float);
					y2 = (Width * sin_float) - (Height * cos_float);
					x2 += CentreX;
					y2 += CentreY;

					x3 = (-Width * cos_float) - (Height * sin_float);
					y3 = (-Width * sin_float) + (Height * cos_float);
					x3 += CentreX;
					y3 += CentreY;

					x4 = (Width * cos_float) - (Height * sin_float);
					y4 = (Width * sin_float) + (Height * cos_float);
					x4 += CentreX;
					y4 += CentreY;

					CTheScripts::ScriptSprites[CTheScripts::IntroRectangles[loop].ScriptSpriteIndex].Draw(
							x1, y1, x2, y2, x3, y3, x4, y4,
							CTheScripts::IntroRectangles[loop].ScriptRectColour);
				}
				break;
			}	// end switch
		}	// end if bBeforeFade
	}	// End for
	
	if (bBeforeFade) CVuTurboSprite::FlushSpriteBuffer();
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : Init
// PURPOSE :  Initialises all the scripts in the world (and puts them into the empty list)
/////////////////////////////////////////////////////////////////////////////////
#if defined GTA_XBOX
#include "Xbcontentpack.h"
#endif	// defined GTA_XBOX

void CTheScripts::Init()
{
	Int32	Counter, Counter2;


	// Clear the actual script space
	for (Counter = 0; Counter < SIZE_SCRIPT_SPACE; Counter++)
	{
		ScriptSpace[Counter] = 0;
	}
	
	WipeLocalVariableMemoryForMissionScript();

	// Clear the linked lists of active and Idle script
	pActiveScripts = pIdleScripts = NULL;

	// Clear the scripts
	for (Counter = 0; Counter < MAX_NUM_SCRIPTS; Counter++)
	{
		ScriptsArray[Counter].Init();
		// Add this script to the 'idle' list
		ScriptsArray[Counter].AddScriptToList(&pIdleScripts);
	}

	MissionCleanUp.Init();

	UpsideDownCars.Init();
	
	StuckCars.Init();
	
	ScriptsForBrains.Init();
	
	ScriptResourceManager.Initialise();
	
	for (Counter = 0; Counter < MAX_ENTITIES_WAITING_FOR_SCRIPT_BRAIN; Counter++)
	{
		EntitiesWaitingForScriptBrain[Counter].pEntity = NULL;
		EntitiesWaitingForScriptBrain[Counter].ScriptBrainIndex = -1;
	}

#ifndef FINAL
	CScriptDebugger::Initialise();
	
	bTimeScripts = false;
	bDisplayStreamedScriptInfo = false;
	
	bDisplayNonMissionEntities = false;
	
	bDisplayUsedScriptResources = false;
#endif

#ifndef MASTER
	bClosestDoorInfo = false;
#endif

	if (CGame::bMissionPackGame)
	{
#ifdef GTA_PS2
		LOAD_CODE_OVERLAY(mc);
#endif
		Int32 SizeRead = 0;
		
		while (SizeRead < SIZE_MPACK_SCRIPT)  // keep showing menu if didnt read script correctly
		{
			if (FrontEndMenuManager.CheckMissionPackValidMenu())
			{
#ifdef GTA_PS2
				// PS2 stuff:
				CMemoryCardFileMgr::Initialise(0);
				
				sprintf(gString, "SCR%d.SCM", CGame::bMissionPackGame);

				int32 gFileId = CMemoryCardFileMgr::OpenFile(0, gString, SCE_RDONLY);
				
				if(gFileId >= 0)
				{
					SizeRead = CMemoryCardFileMgr::Read(gFileId, (char *) &CTheScripts::ScriptSpace[0], SIZE_MAIN_SCRIPT);
					CMemoryCardFileMgr::CloseFile(gFileId);
				}
#elif defined (GTA_PC)			// PC stuff:
				
				CFileMgr::SetDirMyDocuments();

				sprintf(gString, "MPACK//MPACK%d//SCR.SCM",CGame::bMissionPackGame);

				int32 gFileId = CFileMgr::OpenFile(gString, "rb");
				
				if(gFileId > 0)
				{
					SizeRead = CFileMgr::Read(gFileId, (char *) &CTheScripts::ScriptSpace[0], SIZE_MAIN_SCRIPT);
					CFileMgr::CloseFile(gFileId);
				}
#elif defined (GTA_XBOX)			// XBox stuff:
				// MPACKXBOX // need to put an xbox version based on PC code above HERE
				SizeRead = g_MissionPackHelper.LoadMissionPack(CGame::bMissionPackGame, &CTheScripts::ScriptSpace[0], SIZE_MAIN_SCRIPT);
#else
				ASSERT(FALSE);
#endif
				if (SizeRead >= SIZE_MPACK_SCRIPT)
				{				
					// reload the text so we get the mission text added:
					TheText.Load();
				}
			}
			else
			{
				break;
			}
		}

#ifdef GTA_PS2
		REMOVE_CODE_OVERLAY();
#endif //GTA_PS2

		CFileMgr::SetDir("");
	}
	
	if (!CGame::bMissionPackGame)
	// The next bit loads the script test.scc into the code memory
	{
		UInt32	fpos;
//		GDFS	fid;

		fpos = 0;
		CFileMgr::SetDir("data\\script");

/*
		if (CGame::currLevel == LEVEL_INDUSTRIAL)
		{
			NumBytes = CFileMgr::LoadFile("i_test.scc", ScriptSpace, sizeof(ScriptSpace));
		}
		else if (CGame::currLevel == LEVEL_SUBURBAN)
		{
			NumBytes = CFileMgr::LoadFile("s_test.scc", ScriptSpace, sizeof(ScriptSpace));
		}
		else if (CGame::currLevel == LEVEL_COMMERCIAL)
		{
			NumBytes = CFileMgr::LoadFile("c_test.scc", ScriptSpace, sizeof(ScriptSpace));
		}
		else
		{
			CDebug::DebugMessage("Undefined level name, CTheScripts::Init, script.cpp");
		}
*/
//		NumBytes = CFileMgr::LoadFile("main.scm", ScriptSpace, SIZE_MAIN_SCRIPT);

#ifdef GTA_PS2
		int32 id = CFileMgr::OpenFile("main.scm");	//	data
#else
		int32 id = 0;
		if (FrontEndMenuManager.m_UseDebugScripts)
			id = CFileMgr::OpenFile("main_dbg.scm", "rb");
		else id = CFileMgr::OpenFile("main.scm", "rb");	//	data
#endif

		ASSERTMSG(id != 0, "CTheScripts::Init - couldn't open main.scm for reading");

		// Check the script has a valid version
		CheckScriptVersion(id, "main.sc");
		
		CFileMgr::Read(id, (char *) &ScriptSpace[0], SIZE_MAIN_SCRIPT);

		CFileMgr::CloseFile(id);
/*
		fid = gdFsOpen("test.scc", NULL);
		if (fid == NULL)
		{
			CDebug::DebugLog("Cannot read script file");
			gdFsClose(fid);
			ASSERT(0);
		}

		gdFsGetFileSize(fid, &sn);
		ASSERT(sn < SIZE_SCRIPT_SPACE);
*/

//		{		// Print out a debug version of the script
//			char	Str[128];
//			sprintf(Str, "sn:%d", sn);
//			CDebug::DebugLog(Str);	
//		}


//		gdFsRead(fid, 9999999, ScriptSpace);
//		gdFsClose(fid);


/*
		{		// Print out a debug version of the script
			Int16	C;
			char	Str[128];
			for (C = 0; C < 400; C+=4)
			{
				sprintf(Str, "(%d): %d (sn:%d)", C,
						*((Int32 *)&CTheScripts::ScriptSpace[C]), sn);
				CDebug::DebugLog(Str);
			}
		}
*/

//{
//	char	Str[128];
//	sprintf(Str, "ScriptSpace:%p\0", &(ScriptSpace[0]) );
//	CDebug::DebugAddText(Str);
//}
	}


	CFileMgr::SetDir("");

//	This is used to allow the level designers to alter one non-mission-created vehicle
//		at a time
	StoreVehicleIndex = -1;
	StoreVehicleWasRandom = TRUE;

	OnAMissionFlag = 0;
//	OnAGangOneMissionFlag = NULL;
//	OnAGangTwoMissionFlag = NULL;
//	OnAGangThreeMissionFlag = NULL;
	LastMissionPassedTime = MAX_UINT32;

//	GangOneBaseBriefId = 0;
//	GangTwoBaseBriefId = 0;
//	GangThreeBaseBriefId = 0;

/*
	for (Counter = 0; Counter < NUMBER_OF_CONTACTS; Counter++)
	{
		OnAMissionForContactFlag[Counter] = 0;
		BaseBriefIdForContact[Counter] = 0;
	}
*/

//	DeatharrestCheckEnabled = FALSE;
//	DoneDeatharrest = FALSE;

#ifdef GTA_LIBERTY
	for (Counter = 0; Counter < MAX_NUMBER_OF_COLLECTIVE_MEMBERS; Counter++)
	{
		CollectiveArray[Counter].CollectiveIndex = -1;
		CollectiveArray[Counter].MemberPedIndex = 0;
	}
	
	NextFreeCollectiveIndex = 0;
#endif

	LastRandomPedId = -1;
	
	for (Counter = 0; Counter < MAX_USED_OBJECTS; Counter++)
	{
		for (Counter2 = 0; Counter2 < USED_OBJECT_NAME_LENGTH; Counter2++)
		{
			UsedObjectArray[Counter].Name[Counter2] = 0;
		}
		UsedObjectArray[Counter].Index = 0;
	}

	NumberOfUsedObjects = 0;

	ReadObjectNamesFromScript();
	UpdateObjectIndices();		//	must do this after CFileLoader::LoadLevel("GTA3.DAT");
	
	bAlreadyRunningAMissionScript = FALSE;
	bUsingAMultiScriptFile = TRUE;
	
	for (Counter = 0; Counter < MAX_NUMBER_OF_MISSION_SCRIPT_FILES; Counter++)
	{
		MultiScriptArray[Counter].StartOfScriptIndex = 0;
	}
	
	MainScriptSize = 0;
	LargestMissionScriptSize = 0;
	NumberOfMissionScripts = 0;
	NumberOfExclusiveMissionScripts = 0;
	LargestNumberOfMissionScriptLocalVariables = 0;

	ReadMultiScriptFileOffsetsFromScript();

	if (!CGame::bMissionPackGame)
	{
		//	Initialise and read Streamed Script data here
		// This is called in CStreaming::Initialise()
		//StreamedScripts.Initialise();
		
		StreamedScripts.ReadStreamedScriptData();
	}
	
	FailCurrentMission = 0;
	ScriptPickupCycleIndex = 0;
	
	bMiniGameInProgress = false;
	bDisplayNonMiniGameHelpMessages = true;
	
	bPlayerHasMetDebbieHarry = false;
	RiotIntensity = 0;
	bPlayerIsOffTheMap = false;
	RadarZoomValue = 0;
	RadarShowBlipOnAllLevels = FALSE;
	HideAllFrontEndMapBlips = FALSE;
	bDisplayHud = true;
	fCameraHeadingWhenPlayerIsAttached = 0.0f;
	fCameraHeadingStepWhenPlayerIsAttached = 0.0f;

	bEnableCraneRaise = true;
	bEnableCraneLower = true;
	bEnableCraneRelease = true;
	
	bDrawCrossHair = SCRIPT_CROSSHAIR_OFF;
	
	gAllowScriptedFixedCameraCollision = false;		//	A camera flag that needs to be reset on mission cleanup and when a new game starts
	
	ForceRandomCarModel = -1;
	
	bAddNextMessageToPreviousBriefs = true;
	bScriptHasFadedOut = false;
	
	bDrawOddJobTitleBeforeFade = true;
	bDrawSubtitlesBeforeFade = true;
	
#ifdef GTA_PC
#ifdef GTA_LIBERTY
	CountdownToMakePlayerUnsafe = 0;
	DelayMakingPlayerUnsafeThisTime = 1;	// This is another fudge for the intro of the PC version of GTA3.
											//	The main script contains a SET_PLAYER_CONTROL ON before CGame::playingIntro is set.
											//	This variable is set to 1 so that the effect of the first SET_PLAYER_CONTROL ON
											//	is delayed - in fact it should never happen as a SET_PLAYER_CONTROL OFF
											//	in the intro script happens first.
											//	Hopefully the player will never have control until 8ball enters the car on the bridge.
#endif
#endif

#ifndef FINAL
	NumScriptDebugLines = 0;
#endif	
	// Also clear the timers
//	CCDTimers::Init();
#ifndef FINAL
	DbgFlag = FALSE;			// debugging off at start
#endif

	for (Counter = 0; Counter < MAX_SCRIPT_SPHERES; Counter++)
	{
		ScriptSphereArray[Counter].bUsed = FALSE;
		ScriptSphereArray[Counter].ReferenceIndex = 1;
		ScriptSphereArray[Counter].UniqueId = 0;
		ScriptSphereArray[Counter].Centre = CVector(0.0f, 0.0f, 0.0f);
		ScriptSphereArray[Counter].Radius = 0.0f;
	}

//	Stuff for displaying intro text
	for (Counter = 0; Counter < SCRIPT_TEXT_NUM_LINES; Counter++)
	{
		IntroTextLines[Counter].ScriptTextXScale = 0.48f;
		IntroTextLines[Counter].ScriptTextYScale = 1.12f;
		IntroTextLines[Counter].ScriptTextColor = CRGBA(225, 225, 225, 255);
		IntroTextLines[Counter].ScriptTextJustify = FALSE;
		IntroTextLines[Counter].ScriptTextRightJustify = FALSE;
		IntroTextLines[Counter].ScriptTextCentre = FALSE;
		IntroTextLines[Counter].ScriptTextBackgrnd = FALSE;
		IntroTextLines[Counter].ScriptTextBackgrndOnlyText = FALSE;
		IntroTextLines[Counter].ScriptTextWrapX = SCREEN_WIDTH;//182.0f;
		IntroTextLines[Counter].ScriptTextCentreSize = SCREEN_WIDTH;
		IntroTextLines[Counter].ScriptTextBackgrndColor = CRGBA(128, 128, 128, 128);
		IntroTextLines[Counter].ScriptTextProportional = TRUE;
		IntroTextLines[Counter].ScriptTextDropShadowColour = CRGBA(0, 0, 0, 255);
		IntroTextLines[Counter].ScriptTextDropShadow = 2;
		IntroTextLines[Counter].ScriptTextEdge = 0;
		IntroTextLines[Counter].ScriptTextBeforeFade = FALSE;
		IntroTextLines[Counter].ScriptTextFontStyle = FO_FONT_STYLE_STANDARD; //_HEADING;
		IntroTextLines[Counter].ScriptTextAtX = 0.0f;
		IntroTextLines[Counter].ScriptTextAtY = 0.0f;
		
//		for (Counter2 = 0; Counter2 < SCRIPT_TEXT_MAX_LINE_LENGTH; Counter2++)
		for (Counter2 = 0; Counter2 < 8; Counter2++)
		{
//			IntroTextLines[Counter].ScriptTextToDisplay[Counter2] = 0;
			IntroTextLines[Counter].ScriptTextLabel[Counter2] = 0;
		}
		IntroTextLines[Counter].NumberToInsert1 = -1;
		IntroTextLines[Counter].NumberToInsert2 = -1;
	}
	
	NumberOfIntroTextLinesThisFrame = 0;
	
	UseTextCommands = 0;
//	End of stuff for displaying intro text

	bUseMessageFormatting = false;
	MessageCentre = 0;
	MessageWidth = 0;

// Stuff for displaying intro rectangles
	for (Counter = 0; Counter < NUM_SCRIPT_RECTANGLES; Counter++)
	{
		IntroRectangles[Counter].eWindowType = WINDOW_NONE;
		IntroRectangles[Counter].ScriptRectBeforeFade = FALSE;
		IntroRectangles[Counter].ScriptSpriteIndex = -1;	//	-1 = solid colour, no texture
		IntroRectangles[Counter].ScriptRectMinX = 0.0f;
		IntroRectangles[Counter].ScriptRectMinY = 0.0f;
		IntroRectangles[Counter].ScriptRectMaxX = 0.0f;
		IntroRectangles[Counter].ScriptRectMaxY = 0.0f;
		IntroRectangles[Counter].ScriptRectRotation = 0.0f;
		IntroRectangles[Counter].ScriptRectColour = CRGBA(255, 255, 255, 255);
		IntroRectangles[Counter].pTitle[0] = NULL;
		IntroRectangles[Counter].nSwirl = SWIRLS_BOTH;
	}
	
	NumberOfIntroRectanglesThisFrame = 0;
	
//	static CSprite2d ScriptSprites[NUM_SCRIPT_SPRITES];
	RemoveScriptTextureDictionary();

//	End of stuff for displaying intro rectangles

	for (Counter = 0; Counter < MAX_NUM_BUILDING_SWAPS; Counter++)
	{
		BuildingSwapArray[Counter].pBuilding = NULL;
		BuildingSwapArray[Counter].NewModelIndex = -1;
		BuildingSwapArray[Counter].OldModelIndex = -1;
	}
	
	for (Counter = 0; Counter < MAX_NUM_INVISIBILITY_SETTINGS; Counter++)
	{
		InvisibilitySettingArray[Counter].pEntity = NULL;
	}
	
	ClearAllSuppressedCarModels();
	ClearAllVehicleModelsBlockedByScript();
	
	InitialiseAllConnectLodObjects();
	InitialiseSpecialAnimGroupsAttachedToCharModels();

/*
	for (Counter = 0; Counter <= COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED; Counter++)
	{
		CommandUsage[Counter] = 0;
		OrderedCommands[Counter] = 0;
		OrderedUsage[Counter] = 0;
		CommandTime[Counter] = 0;
		OrderedTime[Counter] = 0;
	}
*/

	for (Counter = 0; Counter < MAX_SCRIPT_EFFECT_SYSTEMS; Counter++)
	{
		ScriptEffectSystemArray[Counter].bUsed = FALSE;
		ScriptEffectSystemArray[Counter].ReferenceIndex = 1;
		ScriptEffectSystemArray[Counter].pFXSystem = NULL;
	}
	
	for (Counter = 0; Counter < MAX_SCRIPT_SEARCHLIGHTS; Counter++)
	{
		ScriptSearchLightArray[Counter].bUsed = FALSE;
		ScriptSearchLightArray[Counter].bClipIfColliding = false;
		ScriptSearchLightArray[Counter].bRenderGroundLight = true;
		ScriptSearchLightArray[Counter].SearchLightState = SEARCHLIGHT_STATIONARY;
		ScriptSearchLightArray[Counter].ReferenceIndex = 1;
		ScriptSearchLightArray[Counter].SearchLightSource.Set(0.0f, 0.0f, 0.0f);
		ScriptSearchLightArray[Counter].SearchLightPointAt.Set(0.0f, 0.0f, 0.0f);
		ScriptSearchLightArray[Counter].Radius = 0.0f;
		ScriptSearchLightArray[Counter].SourceRadius = 0.0f;
		ScriptSearchLightArray[Counter].SearchLightPoint1.Set(0.0f, 0.0f, 0.0f);
		ScriptSearchLightArray[Counter].SearchLightPoint2.Set(0.0f, 0.0f, 0.0f);
		ScriptSearchLightArray[Counter].MoveSpeed = 0.0f;

		ScriptSearchLightArray[Counter].pSearchLightSourceEntity = NULL;
		ScriptSearchLightArray[Counter].pSearchLightTargetEntity = NULL;
		
		ScriptSearchLightArray[Counter].pSearchLightBase = NULL;
		ScriptSearchLightArray[Counter].pSearchLightHousing = NULL;
		ScriptSearchLightArray[Counter].pSearchLightBulb = NULL;
	}
	NumberOfScriptSearchLights = 0;

	for (Counter = 0; Counter < MAX_SCRIPT_CHECKPOINTS; Counter++)
	{
		ScriptCheckpointArray[Counter].bUsed = FALSE;
		ScriptCheckpointArray[Counter].ReferenceIndex = 1;
		
		ScriptCheckpointArray[Counter].pCheckpoint = NULL;
	
	}
	NumberOfScriptCheckpoints = 0;

	for (Counter = 0; Counter < CTaskSequences::MAX_NUM_SEQUENCE_TASKS; Counter++)
	{
		ScriptSequenceTaskArray[Counter].bUsed = FALSE;
		ScriptSequenceTaskArray[Counter].ReferenceIndex = 1;
	}

//	ClearScriptDialogueQueue();
	
//	ClearNextDieAnim();

#ifndef FINAL
	CScriptDebugger::ReadSizeOfGlobalVariableBlock();
//	CScriptDebugger::LoadScriptDebugFile();
	
	VarConsole.Add("Time scripts", &bTimeScripts, true, VC_MISC);
	VarConsole.Add("Streamed script info", &bDisplayStreamedScriptInfo, true, VC_MISC);
	VarConsole.Add("Non mission stuff", &bDisplayNonMissionEntities, true, VC_MISC);
	VarConsole.Add("Script resources", &bDisplayUsedScriptResources, true, VC_MISC);
	VarConsole.Add("Output processed commands", &gbRecordScriptCommands, true, VC_MISC);
	VarConsole.Add("Threat search view angle", &CPedAcquaintanceScanner::ms_fThresholdDotProduct, 0.0f, 0.870f, true, VC_PEDS);
#endif

#ifndef MASTER
	VarConsole.Add("Display Closest Door Info", &bClosestDoorInfo, true, VC_MISC);
#endif

	CScripted2dEffects::Init();
	CTaskSequences::Init();
	CPedGroups::Init();
#ifdef USE_INFORM_FRIENDS_QUEUE
	CInformFriendsEventQueue::Init();	
#endif

#ifdef USE_INFORM_GROUP_QUEUE
	CInformGroupEventQueue::Init();
#endif

	CDecisionMakerTypes::GetDecisionMakerTypes()->Init();
}

void CTheScripts::WipeLocalVariableMemoryForMissionScript(void)
{
	UInt32 loop;
	
	for (loop = 0; loop < MAX_LOCAL_VARIABLES_FOR_A_MISSION_SCRIPT; loop++)
	{
		LocalVariablesForCurrentMission[loop] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ShutdownThisScript
// PURPOSE :  Handles any tidyup that needs done when a RunningScript is moved
//				from the pActiveScripts list to the pIdleScripts list
/////////////////////////////////////////////////////////////////////////////////
void CRunningScript::ShutdownThisScript(void)
{
	Int32 StreamedScriptIndex;
	UInt8 CurrentNumberOfUsers;
	int32* pLocalVariable = NULL;
	
	bActive = false;
	if (bIsThisAStreamedScript)
	{
		StreamedScriptIndex = CTheScripts::StreamedScripts.GetStreamedScriptWithThisStartAddress(BaseAddressOfThisScript);
		CurrentNumberOfUsers = CTheScripts::StreamedScripts.GetNumUsersOfStreamedScript(StreamedScriptIndex);
		ASSERTOBJ(CurrentNumberOfUsers > 0, CTheScripts::StreamedScripts.GetStreamedScriptFilename(StreamedScriptIndex), "ShutdownThisScript - Current number of users for streamed script should be at least 1");
		CTheScripts::StreamedScripts.SetNumUsersOfStreamedScript(StreamedScriptIndex, CurrentNumberOfUsers - 1);
	}
	
	switch(ScriptBrainType)
	{
		case CScriptsForBrains::CODE_ATTRACTOR_PED:
		case CScriptsForBrains::CODE_PED:
		case CScriptsForBrains::PED_STREAMED:
		case CScriptsForBrains::PED_GENERATOR_STREAMED:
			{
				pLocalVariable = GetPointerToLocalVariable(0);
				CPed* pPed = CPools::GetPedPool().GetAt(*pLocalVariable);
				
				if (pPed)
				{
					pPed->m_nPedFlags.bHasAScriptBrain=false;
					if(CScriptsForBrains::CODE_ATTRACTOR_PED==ScriptBrainType)
					{
						CScriptedBrainTaskStore::SetTask(pPed,new CTaskSimpleFinishBrain());
					}
					
	//				ASSERTMSG(pPed->m_nPedFlags.bWaitingForScriptBrainToLoad, "CRunningScript::ShutdownThisScript - ");
				}
			}
			break;
			
		case CScriptsForBrains::OBJECT_STREAMED :
		case CScriptsForBrains::CODE_OBJECT :
			{
				pLocalVariable = GetPointerToLocalVariable(0);
				CObject* pObject = CPools::GetObjectPool().GetAt(*pLocalVariable);
				
				if (pObject)
				{
	//				pObject->m_nObjectFlags.bHasAScriptBrain=false;
					pObject->m_nObjectFlags.ScriptBrainStatus = CObject::OBJECT_SCRIPT_BRAIN_NOT_LOADED;
	//				ASSERTMSG(pObject->m_nPedFlags.bWaitingForScriptBrainToLoad, "CRunningScript::ShutdownThisScript - ");
				}
			}
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : RemoveScriptFromList
// PURPOSE :  Removes this node from its list
/////////////////////////////////////////////////////////////////////////////////

void CRunningScript::RemoveScriptFromList(CRunningScript **ppList)
{
	if (pPrevious)	// First in list ?
	{				// no
		pPrevious->pNext = pNext;
	}
	else
	{
		*ppList = pNext;
	}

	if (pNext)		// Last in list ?
	{				// no
		pNext->pPrevious = pPrevious;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : AddScriptToList
// PURPOSE :  Adds this script to the list (the start of the list in fact)
/////////////////////////////////////////////////////////////////////////////////

void CRunningScript::AddScriptToList(CRunningScript **ppList)
{
	// Tie up all the pointers and things
	pNext = *ppList;
	pPrevious = NULL;

	if (*ppList)
	{
		(*ppList)->pPrevious = this;
	}
	*ppList = this;
}



/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : StartNewScript
// PURPOSE :  Allocates a new CRunningScript and starts it. (adds it to the
//			  'active' list)
/////////////////////////////////////////////////////////////////////////////////

CRunningScript *CTheScripts::StartNewScript(UInt8 *pStartPC)
{
	CRunningScript	*pNewOne;

	ASSERTMSG(pIdleScripts, "CTheScripts::StartNewScript - no free scripts");	// Make sure there is one available

	pNewOne = pIdleScripts;							// This one will do thank you very much
	pNewOne->RemoveScriptFromList(&pIdleScripts);	// Take it out of the 'idle' list
	pNewOne->Init();

	pNewOne->PCPointer = pStartPC;				// Start at address 0 for now (will change)

				// Add this guy to the start of the 'active' list. It will
				// be executed next frame.
	pNewOne->AddScriptToList(&pActiveScripts);
	pNewOne->bActive = true;

	return(pNewOne);
}


CRunningScript *CTheScripts::StartNewScript(UInt8 *pStartPC, UInt16 ScriptIndex)
{
	CRunningScript	*pNewOne, *pScriptToUse;
	
	ASSERTMSG( ScriptIndex < MAX_NUM_SCRIPTS, "CTheScripts::StartNewScript - script index is out of range");
	pScriptToUse = &ScriptsArray[ScriptIndex];

	ASSERTMSG(pIdleScripts, "CTheScripts::StartNewScript - no free scripts");	// Make sure there is one available

	pNewOne = pIdleScripts;							// Start with first free script and cycle through until we find the one we want
	
	while (pNewOne && pNewOne != pScriptToUse)
	{
		pNewOne = pNewOne->pNext;
	}
	
	ASSERTMSG(pNewOne, "CTheScripts::StartNewScript - couldn't find the correct script in the list of free scripts");
	if (pNewOne)
	{
		pNewOne->RemoveScriptFromList(&pIdleScripts);	// Take it out of the 'idle' list
		pNewOne->Init();

		pNewOne->PCPointer = pStartPC;				// Start at address 0 for now (will change)

					// Add this guy to the start of the 'active' list. It will
					// be executed next frame.
		pNewOne->AddScriptToList(&pActiveScripts);
		pNewOne->bActive = true;
	}

	return(pNewOne);
}


UInt16 CTheScripts::GetScriptIndexFromPointer(CRunningScript *pScript)
{
	UInt16 ScriptIndex = pScript - ScriptsArray;

	return ScriptIndex;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : Process
// PURPOSE :  Processes the scripts.
/////////////////////////////////////////////////////////////////////////////////

void CTheScripts::Process(void)
{
	CRunningScript	*pCurrent, *pNextOne;
	UInt32			Timer;
	UInt32			TimeStep;
	UInt16 loop, loop2;

		// Don't process the scripts with a replay going on.
	if (CReplay::ReplayGoingOn()) return;

#ifndef MASTER
	if (bClosestDoorInfo)
	{
		DisplayKatieDoorDebugInfo();
	}
#endif


#ifndef FINAL
	if (CTimer::bSkipProcessThisFrame && (CScriptDebugger::bProcessTheNextCommand == false) ) return;
	
	
	if (bDisplayStreamedScriptInfo)
	{
		UInt16 streamed_script_loop;
		for (streamed_script_loop = 0; streamed_script_loop < MAX_STREAMED_SCRIPTS; streamed_script_loop++)
		{
			if (CTheScripts::StreamedScripts.GetPointerToStreamedScriptMemory(streamed_script_loop))
			{
				sprintf(gString, "Streamed script %s is loaded: %d Users", CTheScripts::StreamedScripts.GetStreamedScriptFilename(streamed_script_loop), 
																			CTheScripts::StreamedScripts.GetNumUsersOfStreamedScript(streamed_script_loop));
				VarConsole.AddDebugOutput(gString);
			}
/*	Streamed attractor script brains will have one user for each attractor before the file has streamed
			else
			{
				ASSERTOBJ(CTheScripts::StreamedScripts.GetNumUsersOfStreamedScript(streamed_script_loop) == 0, CTheScripts::StreamedScripts.GetStreamedScriptFilename(streamed_script_loop), "Streamed script isn't loaded but has > 0 users");
			}
*/
		}
	}
	
	if (bDisplayNonMissionEntities)
	{
		MissionCleanUp.DisplayAllNonMissionEntities();
	}
	
	if (bDisplayUsedScriptResources)
	{
		ScriptResourceManager.DisplayScriptResources();
	}
#endif
	
	// Processes the timers
//	CCDTimers::Process();

	Timer = CTimer::GetDebugTimer();

	CommandsExecuted = 0;
	ScriptsUpdated = 0;
	TimeStep = CTimer::GetTimeElapsedInMillisecondsNonClipped();

	UpsideDownCars.UpdateTimers();
	
	StuckCars.Process();
	
	MissionCleanUp.CheckIfCollisionHasLoadedForMissionObjects();

	DrawScriptSpheres();
	
//	ProcessScriptDialogueQueue();
	
	ProcessAllSearchLights();
	
	ProcessWaitingForScriptBrainArray();

	if (FailCurrentMission > 0)
	{
		FailCurrentMission--;
	}

#ifdef GTA_PC
#ifdef GTA_LIBERTY
	if (CountdownToMakePlayerUnsafe)
	{
		CountdownToMakePlayerUnsafe--;
		
		if (CountdownToMakePlayerUnsafe == 0)
		{
			CWorld::Players[0].MakePlayerSafe(FALSE);
		}
	}
#endif
#endif

//	Only reset the intro text and sprite variables if the intro is running
	if (UseTextCommands)
	{
		for (loop = 0; loop < SCRIPT_TEXT_NUM_LINES; loop++)
		{
			IntroTextLines[loop].ScriptTextXScale = 0.48f;
			IntroTextLines[loop].ScriptTextYScale = 1.12f;
			IntroTextLines[loop].ScriptTextColor = CRGBA(225, 225, 225, 255);
			IntroTextLines[loop].ScriptTextJustify = FALSE;
			IntroTextLines[loop].ScriptTextRightJustify = FALSE;
			IntroTextLines[loop].ScriptTextCentre = FALSE;
			IntroTextLines[loop].ScriptTextBackgrnd = FALSE;
			IntroTextLines[loop].ScriptTextBackgrndOnlyText = FALSE;
			IntroTextLines[loop].ScriptTextWrapX = 182.0f;
			IntroTextLines[loop].ScriptTextCentreSize = 640.0f;
			IntroTextLines[loop].ScriptTextBackgrndColor = CRGBA(128, 128, 128, 128);
			IntroTextLines[loop].ScriptTextProportional = TRUE;
			IntroTextLines[loop].ScriptTextDropShadowColour = CRGBA(0, 0, 0, 255);
			IntroTextLines[loop].ScriptTextDropShadow = 2;
			IntroTextLines[loop].ScriptTextEdge = 0;
			IntroTextLines[loop].ScriptTextBeforeFade = FALSE;
			IntroTextLines[loop].ScriptTextFontStyle = FO_FONT_STYLE_STANDARD; //_HEADING;
			IntroTextLines[loop].ScriptTextAtX = 0.0f;
			IntroTextLines[loop].ScriptTextAtY = 0.0f;
			
//			for (loop2 = 0; loop2 < SCRIPT_TEXT_MAX_LINE_LENGTH; loop2++)
			for (loop2 = 0; loop2 < 8; loop2++)
			{
//				IntroTextLines[loop].ScriptTextToDisplay[loop2] = 0;
				IntroTextLines[loop].ScriptTextLabel[loop2] = 0;
			}
			IntroTextLines[loop].NumberToInsert1 = -1;
			IntroTextLines[loop].NumberToInsert2 = -1;
		}
		
		NumberOfIntroTextLinesThisFrame = 0;

	// Stuff for displaying intro rectangles
		for (loop = 0; loop < NUM_SCRIPT_RECTANGLES; loop++)
		{
			IntroRectangles[loop].eWindowType = WINDOW_NONE;
			IntroRectangles[loop].ScriptRectBeforeFade = FALSE;
		}

		NumberOfIntroRectanglesThisFrame = 0;
		
		if (UseTextCommands == 1)
		{
			UseTextCommands = 0;
		}
		
	//	static CSprite2d ScriptSprites[NUM_SCRIPT_SPRITES];
	}
//	End of stuff for displaying intro rectangles

/*
#ifndef FINALBUILD
	ClearAllEntityCheckedForDeadFlags();
#endif
*/

//	sprintf(Str, "CoorsBefore:%f %f %f", CWorld::Players[CWorld::PlayerInFocus].pPed->GetPosition().x, CWorld::Players[CWorld::PlayerInFocus].pPed->GetPosition().y, CWorld::Players[CWorld::PlayerInFocus].pPed->GetPosition().z);
//	CDebug::DebugString(Str, 0, 17);

	Bool8 bBreakOutDueToSingleStep;
	bBreakOutDueToSingleStep = false;

//	Increase TIMERA and TIMERB for the one mission script that can be running
	LocalVariablesForCurrentMission[NUM_LOCAL_VARS] += TimeStep;		// Increase timerA with time elapsed
	LocalVariablesForCurrentMission[NUM_LOCAL_VARS+1] += TimeStep;	// Increase timerB with time elapsed

	// Go through the active lists.
	pCurrent = pActiveScripts;

	CLoadingScreen::NewChunkLoaded();
	
#ifndef FINAL
//	Fiddly code to allow updating of the screen during Script Debugger Single Step mode
	if (CScriptDebugger::pCurrentSingleStepScript)
		pCurrent = CScriptDebugger::pCurrentSingleStepScript;
#endif
	while (pCurrent && !bBreakOutDueToSingleStep)
	{
#ifndef FINALBUILD
		int32 time;
		if(bTimeScripts)
			time = CTimer::GetCurrentTimeInCycles();
			
		if ( (CScriptDebugger::pCurrentSingleStepScript == NULL) && (CScriptDebugger::bClearCheckedForDeadFlags) )
			ClearAllEntityCheckedForDeadFlags();
#endif

		ScriptsUpdated++;
		pNextOne = pCurrent->pNext;		// Store this in case our list gets deleted
		pCurrent->Locals[NUM_LOCAL_VARS] += TimeStep;	// Increase timerA with time elapsed
		pCurrent->Locals[NUM_LOCAL_VARS+1] += TimeStep;	// Increase timerB with time elapsed
		
#ifdef FINAL
		pCurrent->Process();			// pCurrent can get deleted at this point but pNext is stil valid
#else
		if (pCurrent->Process())
		{
			bBreakOutDueToSingleStep = true;
		}
		
		if(bTimeScripts)
		{
			time = CTimer::GetCurrentTimeInCycles() - time;
			{
#define BUSCLOCK_MILLISECOND_RATIO	(147456.0f)
				sprintf(gString, "%s: %f", pCurrent->ScriptName, time/BUSCLOCK_MILLISECOND_RATIO);
				VarConsole.AddDebugOutput(gString);
			}	
		}	
#endif
		pCurrent = pNextOne;
		if (pCurrent && pCurrent->bActive == false)
		{
			pCurrent = NULL;
		}
	}
	
#ifndef FINAL
	gbRecordScriptCommands = false;
	CScriptDebugger::bClearCheckedForDeadFlags = true;
#endif

	CLoadingScreen::NewChunkLoaded();

	//Clear all the recorded events for every mission ped.
	CPedPool& pool = CPools::GetPedPool();
	CPed* pPed;
	int32 i=pool.GetSize();	
	while(i--)
	{
		pPed = pool.GetSlot(i);
		if(pPed && pPed->GetCharCreatedBy()==MISSION_CHAR)
		{
			pPed->GetPedIntelligence()->RecordEventForScript(CEventTypes::EVENT_NONE,0);
		}
	}
	
/*
#ifndef FINAL
//	Fiddly code to allow updating of the screen during Script Debugger Single Step mode
	if (pCurrent == NULL)
	{
		CScriptDebugger::pCurrentSingleStepScript = NULL;
	}
#endif
*/

/*
#ifdef DEBUG
	pCurrent = pActiveScripts;
	while (pCurrent)
	{
		if (pCurrent->NumberOfFramesOfProcessing > 0)
		{
			CDebug::DebugLog("%s - %d\n", pCurrent->ScriptName, (pCurrent->TotalCommandsProcessed / pCurrent->NumberOfFramesOfProcessing) );
		}
		pCurrent = pCurrent->pNext;
		
		if (pCurrent == NULL)
		{
			CDebug::DebugLog("\n\n\n");
		}
	}
#endif
*/
/*
	// Debug: print out the script as read in
	{
		char	Str[128];
		Int16	C;

		for (C = 20; C < 40; C++)
		{
			sprintf(Str, "%d", *((Int32 *)&CTheScripts::ScriptSpace[C * 4]) );
			CDebug::DebugString(Str, 10, 10+C);
		}
	}
*/
		// Print some debug info to screen
//	Timer = CTimer::GetDebugTimer() - Timer;
//	sprintf(Str, "Timer:%d", Timer);
//	CDebug::DebugString(Str, 0, 26);

//	sprintf(Str, "Scripts:%d Cmds:%d", ScriptsUpdated, CommandsExecuted);
//	CDebug::DebugString(Str, 0, 25);

/*
	UInt16 InsertPoint, ShiftIndex;
	Bool8 Found;
	UInt16 stop_here;
	
	stop_here = 11;
	
	
	for (loop = 0; loop <= COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED; loop++)
	{
		OrderedCommands[loop] = 0;
		OrderedUsage[loop] = 0;
		OrderedTime[loop] = 0;
	}

	for (loop = 0; loop <= COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED; loop++)
	{
//		if (CommandUsage[loop] > 0)
		if (CommandTime[loop] > 0)
		{
			InsertPoint = 0;
			Found = FALSE;
			
			while ((InsertPoint <= COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED) && (!Found))
			{
//				if (CommandUsage[loop] > OrderedUsage[InsertPoint])
				if (CommandTime[loop] > OrderedTime[InsertPoint])
				{	//	Shift all OrderedCommands entries up by 1 and write loop at OrderedCommands[InsertPoint]
				
					for (ShiftIndex = COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED; ShiftIndex > InsertPoint; ShiftIndex--)
					{
						OrderedCommands[ShiftIndex] = OrderedCommands[ShiftIndex - 1];
						OrderedUsage[ShiftIndex] = OrderedUsage[ShiftIndex - 1];
						OrderedTime[ShiftIndex] = OrderedTime[ShiftIndex - 1];
					}
					
					OrderedCommands[InsertPoint] = loop;
					OrderedUsage[InsertPoint] = CommandUsage[loop];
					OrderedTime[InsertPoint] = CommandTime[loop];
					
					Found = TRUE;
				}
				
				InsertPoint++;
			}
		}
	}


	stop_here = 12;
*/
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : StartTestScript
// PURPOSE :  Initialises a little test script
/////////////////////////////////////////////////////////////////////////////////

void CTheScripts::StartTestScript(void)
{
	StartNewScript(&ScriptSpace[0]);
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : CTheScripts::IsPlayerOnAMission
// PURPOSE :  Returns TRUE if the player is currently on a mission.
//				This is called to decide whether contact points should be 
//				displayed by the radar code
/////////////////////////////////////////////////////////////////////////////////
Bool8 CTheScripts::IsPlayerOnAMission(void)
{
	if (OnAMissionFlag && (*((UInt32*)&ScriptSpace[OnAMissionFlag]) == 1))
	{
		return TRUE;
	}

	return FALSE;
}

Bool8 CRunningScript::IsPedDead(CPed *pPed)
{
	ASSERTOBJ(pPed, ScriptName, "CRunningScript::IsPedDead - ped doesn't exist");
	
	if ( (pPed->GetPedState() == PED_DEAD) || (pPed->GetPedState() == PED_DIE) || (pPed->GetPedState() == PED_DIE_BY_STEALTH))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
// name:		GivePedScriptedTask
// description:	Give ped a new task. Instead of just giving a task the ped gets given
//				a scripted task event. This ensures the ped will get the task and that
//				it doesn't break the current task
void CRunningScript::GivePedScriptedTask(int32 iPedID, CTask* pTask, int32 CurrCommand)
{
#ifndef FINAL
   	sprintf(gString, "%s, %s", ScriptName, pTask->GetName());
#endif

    if(-1!=iPedID)
    {
		// Give task to ped
        CPed* pPed = CPools::GetPedPool().GetAt(iPedID);

    	ASSERTOBJ(-1==CTaskSequences::ms_iActiveSequence,gString,"Sequence opened unexpectedly");
		ASSERTOBJ(pPed, gString, "Character doesn't exist");
		ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, gString, "Check character is alive this frame");
		
		CPedGroup* pPedGroup=CPedGroups::GetPedsGroup(pPed);
		
		CPed* pPedForBrain=0;
		if(CScriptsForBrains::CODE_ATTRACTOR_PED==ScriptBrainType|| 
		   CScriptsForBrains::PED_GENERATOR_STREAMED==ScriptBrainType || 
		   CScriptsForBrains::PED_STREAMED==ScriptBrainType || 
		   CScriptsForBrains::CODE_PED==ScriptBrainType)
		{
			int32* pLocalVariable = GetPointerToLocalVariable(0);
			pPedForBrain = CPools::GetPedPool().GetAt(*pLocalVariable);
		}
		
		if(pPed->m_nPedFlags.bHasAScriptBrain && pPedForBrain!=pPed)
		{
			delete pTask;
			return;	
		}
		
		if(pPedForBrain && CScriptsForBrains::CODE_ATTRACTOR_PED==ScriptBrainType)
		{
			if(CScriptedBrainTaskStore::SetTask(pPed,pTask))
			{
				const int iVacantSlot=CPedScriptedTaskRecord::GetVacantSlot();
				ASSERT(iVacantSlot>=0);
				CPedScriptedTaskRecord::ms_scriptedTasks[iVacantSlot].SetAsAttractorScriptTask(pPed,CurrCommand,pTask);	
			}
		}						
		else if(pPedGroup && !pPed->IsPlayer())
		{
			ASSERT(0==pPedForBrain);
			pPedGroup->GetGroupIntelligence()->SetScriptCommandTask(pPed,*pTask);
			const CTask* pTaskInGroup=pPedGroup->GetGroupIntelligence()->GetTaskScriptCommand(pPed);
			const int iVacantSlot=CPedScriptedTaskRecord::GetVacantSlot();
			ASSERT(iVacantSlot>=0);
			CPedScriptedTaskRecord::ms_scriptedTasks[iVacantSlot].SetAsGroupTask(pPed,CurrCommand,pTaskInGroup);			
			delete pTask;
		}
		else
		{
			// create event and give to ped
			CEventScriptCommand event(CTaskManager::TASK_PRIORITY_PRIMARY,pTask);
			CEvent* pEvent=pPed->GetPedIntelligence()->AddEvent(event);
			if(pEvent)
			{
				const int iVacantSlot=CPedScriptedTaskRecord::GetVacantSlot();
				ASSERT(iVacantSlot>=0);
#ifndef FINAL			
				CPedScriptedTaskRecord::ms_scriptedTasks[iVacantSlot].Set(pPed,CurrCommand,(const CEventScriptCommand*)pEvent,gString);
#else
				CPedScriptedTaskRecord::ms_scriptedTasks[iVacantSlot].Set(pPed,CurrCommand,(const CEventScriptCommand*)pEvent);
#endif
			}
		}
    }
    else
    {
    	// Add task to sequence
    	ASSERTOBJ(CTaskSequences::ms_iActiveSequence >=0 && CTaskSequences::ms_iActiveSequence<CTaskSequences::MAX_NUM_SEQUENCE_TASKS, gString, "Sequence task closed");
		ASSERTOBJ(CTaskSequences::ms_bIsOpened[CTaskSequences::ms_iActiveSequence], gString, "Sequence task closed");
		CTaskSequences::ms_taskSequence[CTaskSequences::ms_iActiveSequence].AddTask(pTask);
    }
}

void CRunningScript::UpdatePC(Int32 NewPCValue)
{
//	UInt8 *ReturnPCPointer;
	
	//	Deal with Streamed Scripts
	
	if (NewPCValue < 0)	//	What if NewPCValue == 0? Can this happen or is MISSION_START always there?
	{
//		ReturnPC = SIZE_MAIN_SCRIPT - NewPCValue;	//	Add -NewPCValue to the base offset of the mission script
		ASSERTMSG(BaseAddressOfThisScript != NULL, "CRunningScript::UpdatePC - This script must have a base address");
//		ReturnPCPointer = &BaseAddressOfThisScript[-NewPCValue];
		PCPointer = &BaseAddressOfThisScript[-NewPCValue];
	}
	else
	{
//		ReturnPCPointer = &CTheScripts::ScriptSpace[NewPCValue];
		PCPointer = &CTheScripts::ScriptSpace[NewPCValue];
	}

//	return ReturnPCPointer;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : Process
// PURPOSE :  Process this one script. (has to be an active one)
/////////////////////////////////////////////////////////////////////////////////

Bool8 CRunningScript::Process(void)
{
#ifndef FINALBUILD
	Int8	ReturnVal;
	Int32	DebugCount;
#endif	
//	Int16 BigMessageRow;

	if (EndOfScriptedCutscenePC)
	{
		if (CCutsceneMgr::IsCutsceneSkipButtonBeingPressed())
		{
			CHud::m_BigMessage[BIG_MESSAGE_TITLE][0] = 0;  // remove mission name
			UpdatePC(EndOfScriptedCutscenePC);

			EndOfScriptedCutscenePC = 0;
			ActivateTime = 0;	//	Quit any WAITs which are currently taking place in this script
		}
	}

	if (IsThisAMissionScript)
	{
		DoDeatharrestCheck();	// Should this be inside the if?
								// or even in ProcessOneCommand()
								// This assumes that a script is a mission
								// and that only the main script and one mission script
								// are running at the same time.
	}
/*	Cleanup for script brains is going to be handled by the level designers
	if (ScriptBrainType >= 0)
	{
		Bool8 bRemoveThisScriptBrain = false;
		if ( (ScriptBrainType == CScriptsForBrains::CODE_OBJECT) || (ScriptBrainType == CScriptsForBrains::OBJECT) )
		{	//	First Local Variable is the index of an object
			CObject *pObjForThisBrain = CPools::GetObjectPool().GetAt(Locals[0]);
			if (pObjForThisBrain == NULL)
			{	//	if the object doesn't exist, terminate the script
				bRemoveThisScriptBrain = true;
			}
		}
		else
		{	//	First Local Variable is the index of a ped
			CPed *pPedForThisBrain = CPools::GetPedPool().GetAt(Locals[0]);
			if (pPedForThisBrain == NULL)
			{	//	if the ped is dead, terminate the script
				bRemoveThisScriptBrain = true;
			}
		}
		
		if (bRemoveThisScriptBrain)
		{
			RemoveScriptFromList(&CTheScripts::pActiveScripts);
			AddScriptToList(&CTheScripts::pIdleScripts);
			ShutdownThisScript();
			return false;		// Time to go to next one. (Since this one doesn't exist anymore)
		}
	}
*/
	
if (ThisMustBeTheOnlyMissionRunning)
{

#ifdef FAIL_CURRENT_MISSION_VIA_PAD
//    if (CPad::GetPad(1)->GetButtonCross() && CPad::GetPad(1)->GetButtonCircle())
   	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_AT, KEYBOARD_MODE_STANDARD, "fail mission"))
#else
	if (CTheScripts::FailCurrentMission == 1)
#endif//FAIL_CURRENT_MISSION_VIA_PAD...
	{
		// Make sure we are not at top level already
		ASSERTOBJ(StackDepth > 0, ScriptName, "CRunningScript::Process - fail current mission - Stack is empty");
		// first off, unwind stack ready to do cleanup
		while (StackDepth > 1)
		{
			StackDepth--;
		}

		if (StackDepth == 1)
		{
			PCPointer = PCStack[--StackDepth];
		}
	}
}

	CTheScripts::ReinitialiseSwitchStatementData();
	// Is it time to process this list this frame ?
	if (CTimer::GetTimeInMilliseconds() >= ActivateTime)
	{		// Yes it is.
#ifdef FINALBUILD
		while(!ProcessOneCommand()) {}
#else		
		int32 fid = -1;
		if(gbRecordScriptCommands)
		{
			sprintf(gString, "%s.txt", ScriptName);
			fid = CFileMgr::OpenFileForWriting(gString);
		}	

		DebugCount = 0;
			// Keep processing until there is reason to stop.
		while ( !(ReturnVal = ProcessOneCommand()) )
		{
			DebugCount++;
			ASSERTOBJ(DebugCount<6000, ScriptName, "Need to add a WAIT");
//	Fiddly code to allow updating of the screen during Script Debugger Single Step mode
			if (CScriptDebugger::bSingleStep)
			{
				return true;
			}
			if(fid > 0)
			{
				char* pCmd = CScriptDebugger::CommandNames[CScriptDebugger::CurrentCommand];
				CFileMgr::Write(fid, pCmd, strlen(pCmd));
				CFileMgr::Write(fid, "\r\n", 2);
			}	
		}
/*
#ifdef DEBUG
		TotalCommandsProcessed += DebugCount;
		NumberOfFramesOfProcessing++;
#endif
*/
		if (ReturnVal < 0)
		{
			sprintf(CTheScripts::ScriptErrorMessage, "CRunningScript::Process - probably unknown command, %d %s", CScriptDebugger::CurrentCommand, CScriptDebugger::CommandNames[CScriptDebugger::CurrentCommand]);
			ASSERTOBJ(0, ScriptName, CTheScripts::ScriptErrorMessage);
		}
		else
		{	//	Reached the end of the script (WAIT or TERMINATE_THIS_SCRIPT) so turn off single step and process all the other scripts this frame
			CScriptDebugger::bSingleStep = false;
			CScriptDebugger::pCurrentSingleStepScript = NULL;
		}
		
		if(fid >= 0)
		{
			CFileMgr::CloseFile(fid);
		}
#endif
	}
/*
	else
	{
		if (IsMessageWait)
		{
			if (Pads[0].ButtonCrossJustDown())
			{	// if is button pressed
				ActivateTime = 0;
	
				// Clear current big messages
				for (BigMessageRow = 0; BigMessageRow < NUM_BIG_MESSAGE_ROWS; BigMessageRow++)
				{
					if (CMessages::BIGMessages[BigMessageRow][0].pMessage)
					{
						CMessages::BIGMessages[BigMessageRow][0].WhenStarted = 0;
					}
				}
				
				// Clear current message
				if (CMessages::BriefMessages[0].pMessage)
				{
					CMessages::BriefMessages[0].WhenStarted = 0;
				}
	
			}
		}
	}
*/	
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ProcessOneCommand
// PURPOSE :  Does the next command of this list
// RETURNS :  <0 for an error
//            0 for 'next one please'
//			  1 for 'done for this frame'
/////////////////////////////////////////////////////////////////////////////////
Int8 CRunningScript::ProcessOneCommand(void)
{
	typedef Int8 (CRunningScript::*ScriptFunction)(Int32 command);
	static ScriptFunction processCommandFuncs[] = {
		&CRunningScript::ProcessCommands0To99,
		&CRunningScript::ProcessCommands100To199,
		&CRunningScript::ProcessCommands200To299,
		&CRunningScript::ProcessCommands300To399,
		&CRunningScript::ProcessCommands400To499,
		&CRunningScript::ProcessCommands500To599,
		&CRunningScript::ProcessCommands600To699,
		&CRunningScript::ProcessCommands700To799,
		&CRunningScript::ProcessCommands800To899,
		&CRunningScript::ProcessCommands900To999,
		&CRunningScript::ProcessCommands1000To1099,
		&CRunningScript::ProcessCommands1100To1199,
		&CRunningScript::ProcessCommands1200To1299,
		&CRunningScript::ProcessCommands1300To1399,
		&CRunningScript::ProcessCommands1400To1499,
		&CRunningScript::ProcessCommands1500To1599,
		&CRunningScript::ProcessCommands1600To1699,
		&CRunningScript::ProcessCommands1700To1799,
		&CRunningScript::ProcessCommands1800To1899,
		&CRunningScript::ProcessCommands1900To1999,
		&CRunningScript::ProcessCommands2000To2099,
		&CRunningScript::ProcessCommands2100To2199,
		&CRunningScript::ProcessCommands2200To2299,
		&CRunningScript::ProcessCommands2300To2399,
		&CRunningScript::ProcessCommands2400To2499,
		&CRunningScript::ProcessCommands2500To2599,
		&CRunningScript::ProcessCommands2600To2699
	};
	Int16 CurrentCommand;

	// Make sure the PC is a reasonable number
//	ASSERT (PC < 100000);
//	ASSERT ( (PC & 3) == 0);

//if (DbgFlag)
/*
{

	sprintf(Str, "PC:%d Com:%d Scr:%p %d", PC,
		*((Int32 *)&CTheScripts::ScriptSpace[PC]),
		this, COMMAND_CAR_CREATE);
//	debug_printf(XStr);
//	CDebug::DebugLog(Str);
//	strcpy (Str, "Test fucker2\0");

	CDebug::DebugAddText(Str);

//	sprintf(Str, "Player Coors: %f %f %f",
//			CWorld::Players[CWorld::PlayerInFocus].pPed->GetPosition().x,
//			CWorld::Players[CWorld::PlayerInFocus].pPed->GetPosition().y,
//			CWorld::Players[CWorld::PlayerInFocus].pPed->GetPosition().z );
//	CDebug::DebugLog(Str);
}
*/
	Int8 ReturnValue;

#ifndef FINAL
	CScriptDebugger::SetCurrentCommand(PCPointer, ScriptName);

	if (CScriptDebugger::bSingleStep)
	{
		CScriptDebugger::DisplayCurrentCommand(true);
	}
/*	This has been moved after the processing of the commands so that breakpoints and watchpoints stop in the correct place - not one command late
	if ( (CScriptDebugger::IsThisABreakpoint(PC))
		|| (CScriptDebugger::CheckAllWatchpoints())
		|| (CScriptDebugger::bSingleStep) )
	{
//		CScriptDebugger::WaitInSingleStepMode();
		CScriptDebugger::DisplayScriptBreakpoints();
		CScriptDebugger::DisplayScriptWatchpoints();
		CScriptDebugger::DisplayScriptViewVariables();

		CScriptDebugger::DisplayCurrentCommand();


		CTimer::bSlowMotionActive = TRUE;
		CTimer::ms_fSlowMotionScale = -1.0f;
		CScriptDebugger::pCurrentSingleStepScript = this;
		CScriptDebugger::bSingleStep = true;
		CScriptDebugger::bProcessTheNextCommand = false;
	}
*/
#endif

	CTheScripts::CommandsExecuted++;

	CurrentCommand = CTheScripts::ReadInt16FromScript(PCPointer);

	if (CurrentCommand & 0x8000)
	{
		NotForLatestExpression = TRUE;
	}
	else
	{
		NotForLatestExpression = FALSE;
	}
	CurrentCommand &= 0x7fff;

/*
	UInt32 StartTime, EndTime, TimeDiff;

	if (CurrentCommand <= COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED)
	{
		CommandUsage[CurrentCommand]++;
		
		StartTime = CTimer::GetCurrentTimeInCycles();
	}
*/
	ReturnValue = -1;

	ASSERT(CurrentCommand >= COMMAND_NOP);
	
	ScriptFunction fn = processCommandFuncs[CurrentCommand/100];
	ReturnValue = (this->*fn)(CurrentCommand);
	
	/*if ((CurrentCommand < COMMAND_SUB_INT_VAR_FROM_INT_LVAR))
	{
		ReturnValue = ProcessCommands0To99(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_VAR_FLOAT)
	{
		ReturnValue = ProcessCommands100To199(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_SET_CHAR_INVINCIBLE)
	{
		ReturnValue = ProcessCommands200To299(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_ADD_UPSIDEDOWN_CAR_CHECK)
	{
		ReturnValue = ProcessCommands300To399(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_IS_CAR_UPSIDEDOWN)
	{
		ReturnValue = ProcessCommands400To499(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_SET_COLL_OBJ_WAIT_ON_FOOT)
	{
		ReturnValue = ProcessCommands500To599(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_SET_SWAT_REQUIRED)
	{
		ReturnValue = ProcessCommands600To699(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_IS_CHAR_IN_PLAYERS_GROUP)
	{
		ReturnValue = ProcessCommands700To799(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_PRINT_STRING_IN_STRING_NOW)
	{
		ReturnValue = ProcessCommands800To899(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_FLASH_RADAR_BLIP)
	{
		ReturnValue = ProcessCommands900To999(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_LOAD_COLLISION_WITH_SCREEN)
	{
		ReturnValue = ProcessCommands1000To1099(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_IS_INT_VAR_GREATER_THAN_CONSTANT)
	{
		ReturnValue = ProcessCommands1100To1199(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_SET_CHAR_CAN_BE_DAMAGED_BY_MEMBERS_OF_GANG)
	{
		ReturnValue = ProcessCommands1200To1299(CurrentCommand);
	}
	else if (CurrentCommand < COMMAND_REGISTER_VIGILANTE_LEVEL)
	{
		ReturnValue = ProcessCommands1300To1399(CurrentCommand);
	}
	else if (CurrentCommand < 1500)
	{
		ReturnValue = ProcessCommands1400To1499(CurrentCommand);
	}
	else if (CurrentCommand < 1600)
	{
		ReturnValue = ProcessCommands1500To1599(CurrentCommand);
	}
	else if (CurrentCommand < 1700)
	{
		ReturnValue = ProcessCommands1600To1699(CurrentCommand);
	}
	else if (CurrentCommand < 1800)
	{
		ReturnValue = ProcessCommands1700To1799(CurrentCommand);
	}
	else if (CurrentCommand < 1900)
	{
		ReturnValue = ProcessCommands1800To1899(CurrentCommand);
	}
	else if (CurrentCommand < 2000)
	{
		ReturnValue = ProcessCommands1900To1999(CurrentCommand);
	}
	else if (CurrentCommand < 2100)
	{
		ReturnValue = ProcessCommands2000To2099(CurrentCommand);
	}
	else if (CurrentCommand < 2200)
	{
		ReturnValue = ProcessCommands2100To2199(CurrentCommand);
	}
	else if (CurrentCommand < 2300)
	{
		ReturnValue = ProcessCommands2200To2299(CurrentCommand);
	}
	else if (CurrentCommand < 2400)
	{
		ReturnValue = ProcessCommands2300To2399(CurrentCommand);
	}
	else if (CurrentCommand < 2500)
	{
		ReturnValue = ProcessCommands2400To2499(CurrentCommand);
	}*/

#ifndef FINAL
	CScriptDebugger::SetCurrentCommand(PCPointer, ScriptName);

	if ( (CScriptDebugger::IsThisABreakpoint(PCPointer))
		|| (CScriptDebugger::CheckAllWatchpoints())
		|| (CScriptDebugger::bSingleStep) )
	{
//		CScriptDebugger::WaitInSingleStepMode();
		CScriptDebugger::DisplayScriptBreakpoints();
		CScriptDebugger::DisplayScriptWatchpoints();
		CScriptDebugger::DisplayScriptViewVariables();

		if (ReturnValue == 0)
		{
			CScriptDebugger::DisplayCurrentCommand(false);
		}
		else
		{	//	WAIT or TERMINATE_THIS_SCRIPT
			CDebug::DebugLog("Finished processing %s for this frame\n", CScriptDebugger::CurrentScriptName);
		}

		CTimer::bSlowMotionActive = TRUE;
		CTimer::ms_fSlowMotionScale = -1.0f;
		CScriptDebugger::pCurrentSingleStepScript = this;
		CScriptDebugger::bSingleStep = true;
		CScriptDebugger::bProcessTheNextCommand = false;
	}
#endif

/*
	if (CurrentCommand <= COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED)
	{
		EndTime = CTimer::GetCurrentTimeInCycles();
		
		TimeDiff = EndTime - StartTime;
		
		CommandTime[CurrentCommand] += TimeDiff;
	}
*/

/*
#ifndef MASTER
	char DebugText[100];
	
	if (ThisMustBeTheOnlyMissionRunning)
	{
		sprintf(DebugText, "Comm %d Cmp %d", CurrentCommand, CmpFlag);
		CDebug::DebugAddText(DebugText);
	}
	
#endif
*/

/*
#ifndef FINAL
//	Fiddly code to allow updating of the screen during Script Debugger Single Step mode
	if (CScriptDebugger::bSingleStep)
	{
		CTimer::m_CodePause = true;
		CScriptDebugger::pCurrentSingleStepScript = this;
	}
#endif
*/
	return ReturnValue;
	
//	return(-1);		// Shouldn't be here. Unknown command probably.
}


Int8 CRunningScript::ProcessCommands0To99(Int32 CurrCommand)
{
	Int32 PlayerIndex;

	Bool8 LatestCmpFlagResult;

//	CPlayerInfo *pPlayer;
	CVector TempCoors;

	float NewX, NewY, NewZ;
//	float CentreZ;	//	LowestZ;

	Int32 *pGlobalVar, *pGlobalVar2;
	Int32 *pLocalVar, *pLocalVar2;

	switch(CurrCommand)
	{
//	0
		case COMMAND_NOP:
			return (0);		// keep going
			break;

		case COMMAND_WAIT:

			CollectParameters(1);
			
			ActivateTime = ScriptParams[0] + CTimer::GetTimeInMilliseconds();
			
//			IsMessageWait = FALSE;
			
#ifndef FINAL
			ASSERTMSG (strcmp(ScriptName, "noname"), "This Script has not been given a name before the first WAIT");
#endif
			
			return (1);		// done for now
			break;

		case COMMAND_GOTO:
		
			CollectParameters(1);
			
			UpdatePC(ScriptParams[0]);
			
			return (0);		// keep going
			break;

		case COMMAND_SHAKE_CAM:

			CollectParameters(1);

			// First parameter is power of shake

			CamShakeNoPos(&TheCamera, 0.001f * ScriptParams[0]);

			return (0);		// keep going
			break;

		case COMMAND_SET_VAR_INT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);

			*pGlobalVar = ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_SET_VAR_FLOAT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);

			*((float *)pGlobalVar) = *((float *)&ScriptParams[0]);
			
			return (0);		// keep going
			break;
		case COMMAND_SET_LVAR_INT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*pLocalVar = ScriptParams[0];
			
			return (0);		// keep going
			break;
		case COMMAND_SET_LVAR_FLOAT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*((float *)pLocalVar) = *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;

		case COMMAND_ADD_VAL_TO_INT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*pGlobalVar += ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_ADD_VAL_TO_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*((float *)pGlobalVar) += *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;
//	10
		case COMMAND_ADD_VAL_TO_INT_LVAR:
		
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*pLocalVar += ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_ADD_VAL_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*((float *)pLocalVar) += *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;
		case COMMAND_SUB_VAL_FROM_INT_VAR:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*pGlobalVar -= ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_SUB_VAL_FROM_FLOAT_VAR:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*((float *)pGlobalVar) -= *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;
		case COMMAND_SUB_VAL_FROM_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*pLocalVar -= ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_SUB_VAL_FROM_FLOAT_LVAR:
		
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*((float *)pLocalVar) -= *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;
		case COMMAND_MULT_INT_VAR_BY_VAL:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*pGlobalVar *= ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_MULT_FLOAT_VAR_BY_VAL:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*((float *)pGlobalVar) *= *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;
		case COMMAND_MULT_INT_LVAR_BY_VAL:
		
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*pLocalVar *= ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_MULT_FLOAT_LVAR_BY_VAL:
		
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*((float *)pLocalVar) *= *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;
//	20
		case COMMAND_DIV_INT_VAR_BY_VAL:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*pGlobalVar /= ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_DIV_FLOAT_VAR_BY_VAL:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*((float *)pGlobalVar) /= *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;
		case COMMAND_DIV_INT_LVAR_BY_VAL:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*pLocalVar /= ScriptParams[0];

			return (0);		// keep going
			break;
		case COMMAND_DIV_FLOAT_LVAR_BY_VAL:
		
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*((float *)pLocalVar) /= *((float *)&ScriptParams[0]);

			return (0);		// keep going
			break;

		////////// TESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTS
		////////// TESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTS
		////////// TESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTSTESTS

		// Ints >

		case COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *pGlobalVar > ScriptParams[0] );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_INT_LVAR_GREATER_THAN_NUMBER:
		
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *pLocalVar > ScriptParams[0] );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_NUMBER_GREATER_THAN_INT_VAR:

			CollectParameters(1);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( ScriptParams[0] > *pGlobalVar);
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_NUMBER_GREATER_THAN_INT_LVAR:
		
			CollectParameters(1);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( ScriptParams[0] > *pLocalVar);
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_VAR_GREATER_THAN_INT_VAR:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *pGlobalVar > *pGlobalVar2 );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_LVAR:
		
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			LatestCmpFlagResult = ( *pLocalVar > *pLocalVar2 );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
//	30
		case COMMAND_IS_INT_VAR_GREATER_THAN_INT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			LatestCmpFlagResult = ( *pGlobalVar > *pLocalVar );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *pLocalVar > *pGlobalVar );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		// Floats >

		case COMMAND_IS_FLOAT_VAR_GREATER_THAN_NUMBER:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			LatestCmpFlagResult = ( *((float *)pGlobalVar) > *((float *)&ScriptParams[0]) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_NUMBER:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *((float *)pLocalVar) > *((float *)&ScriptParams[0]) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_VAR:

			CollectParameters(1);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *((float *)&ScriptParams[0]) > *((float *)pGlobalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_LVAR:

			CollectParameters(1);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)&ScriptParams[0]) > *((float *)pLocalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *((float *)pGlobalVar) > *((float *)pGlobalVar2) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)pLocalVar) > *((float *)pLocalVar2) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)pGlobalVar) > *((float *)pLocalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *((float *)pLocalVar) > *((float *)pGlobalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
//	40
		// Ints >=
		case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_NUMBER:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *pGlobalVar >= ScriptParams[0] );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *pLocalVar >= ScriptParams[0] );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_VAR:

			CollectParameters(1);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( ScriptParams[0] >= *pGlobalVar );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_LVAR:

			CollectParameters(1);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( ScriptParams[0] >= *pLocalVar );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			LatestCmpFlagResult = ( *pGlobalVar >= *pGlobalVar2 );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *pLocalVar >= *pLocalVar2 );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *pGlobalVar >= *pLocalVar );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			LatestCmpFlagResult = ( *pLocalVar >= *pGlobalVar );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		// Floats >=

		case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_NUMBER:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			LatestCmpFlagResult = ( *((float *)pGlobalVar) >= *((float *)&ScriptParams[0]) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *((float *)pLocalVar) >= *((float *)&ScriptParams[0]) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
//	50
		case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_VAR:

			CollectParameters(1);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *((float *)&ScriptParams[0]) >= *((float *)pGlobalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_LVAR:

			CollectParameters(1);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)&ScriptParams[0]) >= *((float *)pLocalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *((float *)pGlobalVar) >= *((float *)pGlobalVar2) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)pLocalVar) >= *((float *)pLocalVar2) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

		case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)pGlobalVar) >= *((float *)pLocalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);

			LatestCmpFlagResult = ( *((float *)pLocalVar) >= *((float *)pGlobalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;



		// Ints ==

		case COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER:
			
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *pGlobalVar == ScriptParams[0] );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_LVAR_EQUAL_TO_NUMBER:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);

			LatestCmpFlagResult = ( *pLocalVar == ScriptParams[0] );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_VAR_EQUAL_TO_INT_VAR:
			
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			LatestCmpFlagResult = ( *pGlobalVar == *pGlobalVar2 );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_INT_LVAR_EQUAL_TO_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *pLocalVar == *pLocalVar2 );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
//	60
		case COMMAND_IS_INT_VAR_EQUAL_TO_INT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *pGlobalVar == *pLocalVar );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

//	COMMAND_IS_INT_VAR_NOT_EQUAL_TO_NUMBER
//	COMMAND_IS_INT_LVAR_NOT_EQUAL_TO_NUMBER
//	COMMAND_IS_INT_VAR_NOT_EQUAL_TO_INT_VAR
//	COMMAND_IS_INT_LVAR_NOT_EQUAL_TO_INT_LVAR
//	COMMAND_IS_INT_VAR_NOT_EQUAL_TO_INT_LVAR

		// Floats ==

		case COMMAND_IS_FLOAT_VAR_EQUAL_TO_NUMBER:
			
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			LatestCmpFlagResult = ( *((float *)pGlobalVar) == *((float *)&ScriptParams[0]) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_NUMBER:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			LatestCmpFlagResult = ( *((float *)pLocalVar) == *((float *)&ScriptParams[0]) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
						
			LatestCmpFlagResult = ( *((float *)pGlobalVar) == *((float *)pGlobalVar2) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
		case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)pLocalVar) == *((float *)pLocalVar2) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;
//	70
		case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);

			LatestCmpFlagResult = ( *((float *)pGlobalVar) == *((float *)pLocalVar) );
			UpdateCompareFlag(LatestCmpFlagResult);

			return (0);		// keep going
			break;

//	COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_NUMBER
//	COMMAND_IS_FLOAT_LVAR_NOT_EQUAL_TO_NUMBER
//	COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_FLOAT_VAR
//	COMMAND_IS_FLOAT_LVAR_NOT_EQUAL_TO_FLOAT_LVAR
//	COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_FLOAT_LVAR

		///// CONDITIONAL STATEMENTS.CONDITIONAL STATEMENTS.CONDITIONAL STATEMENTS
		///// CONDITIONAL STATEMENTS.CONDITIONAL STATEMENTS.CONDITIONAL STATEMENTS
		///// CONDITIONAL STATEMENTS.CONDITIONAL STATEMENTS.CONDITIONAL STATEMENTS
/*
		case COMMAND_GOTO_IF_TRUE:

			CollectParameters(1);

			if (CmpFlag)
			{
				PC = UpdatePC(ScriptParams[0]);
			}

			return (0);		// keep going
			break;
*/
		case COMMAND_GOTO_IF_FALSE:

			CollectParameters(1);

			if (!CmpFlag)
			{
				UpdatePC(ScriptParams[0]);
			}

			return (0);		// keep going
			break;

		case COMMAND_TERMINATE_THIS_SCRIPT:

			if (ThisMustBeTheOnlyMissionRunning)
			{
				CTheScripts::bAlreadyRunningAMissionScript = FALSE;
			}

			RemoveScriptFromList(&CTheScripts::pActiveScripts);
			AddScriptToList(&CTheScripts::pIdleScripts);
			ShutdownThisScript();
			return (1);		// Time to go to next one. (Since this one doesn't exist anymore)
			break;

		case COMMAND_START_NEW_SCRIPT:
			{
				CRunningScript	*pNewOne;
				Int32 NewScriptStartIndex;

				CollectParameters(1);
				
				if (ScriptParams[0] < 0)	//	What if ScriptParams[0] == 0? Can this happen or is MISSION_START always there?
				{
					ASSERTOBJ(0, ScriptName, "START_NEW_SCRIPT - can only start a script which will always be in memory");
//					NewScriptStartIndex = SIZE_MAIN_SCRIPT - ScriptParams[0];	//	Add -ScriptParams[0] to the base offset of the mission script
				}
				else
				{
					NewScriptStartIndex = ScriptParams[0];
				}

				pNewOne = CTheScripts::StartNewScript(&CTheScripts::ScriptSpace[NewScriptStartIndex]);
				
				ReadParametersForNewlyStartedScript(pNewOne);
			}
			return (0);
			break;
//	80
		case COMMAND_GOSUB:
			ASSERTOBJ(StackDepth < MAX_STACK_DEPTH, ScriptName, "GOSUB - Stack is already full");	// Make sure stack depth is sufficient

			CollectParameters(1);
			PCStack[StackDepth++] = PCPointer;

			UpdatePC(ScriptParams[0]);
			
			return (0);		// keep going
			break;

		case COMMAND_RETURN:
			ASSERTOBJ(StackDepth > 0, ScriptName, "RETURN - can't return without a GOSUB");	// Make sure we are not at top level allready
			PCPointer = PCStack[--StackDepth];
			return(0);		// keep going
			break;

		case COMMAND_LINE:

			CollectParameters(6);

			CDebug::DebugLine3D(*((float *)&ScriptParams[0]),
								*((float *)&ScriptParams[1]),
								*((float *)&ScriptParams[2]),
								*((float *)&ScriptParams[3]),
								*((float *)&ScriptParams[4]),
								*((float *)&ScriptParams[5]),
								0x3fff00ff, 0x3f00ffff);

			return(0);		// keep going
			break;

		case COMMAND_CREATE_PLAYER:

			CollectParameters(4);

			PlayerIndex = *((Int32 *)&ScriptParams[0]);
			DEBUGLOG1("&&&&&&&&&&&&&Creating player: %d\n", PlayerIndex);
			ASSERT(PlayerIndex < MAX_NUM_PLAYERS);

			// Load player model
			if (!CStreaming::HasModelLoaded(0))
			{
				CStreaming::RequestSpecialModel(0, "player", STRFLAG_DONTDELETE|STRFLAG_FORCE_LOAD|STRFLAG_PRIORITY_LOAD);
				CStreaming::LoadAllRequestedModels(true);
			}

			CPlayerPed::SetupPlayerPed(PlayerIndex);

			CWorld::Players[PlayerIndex].pPed->SetCharCreatedBy(MISSION_CHAR);

			CPlayerPed::DeactivatePlayerPed(PlayerIndex);

			NewX = *((float *)&ScriptParams[1]);
			NewY = *((float *)&ScriptParams[2]);
			NewZ = *((float *)&ScriptParams[3]);
			
			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
//				NewZ += 2.0f;	// Safety margin
			}

			NewZ += CWorld::Players[PlayerIndex].pPed->GetDistanceFromCentreOfMassToBaseOfModel();

			CWorld::Players[PlayerIndex].pPed->SetPosition(NewX, NewY, NewZ);

			CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), CWorld::Players[PlayerIndex].pPed);

			CPlayerPed::ReactivatePlayerPed(PlayerIndex);

//!PC - this won't work for PC yet
#if defined (GTA_PS2)
#ifndef CDROM
			CClothes::RebuildPlayer(CWorld::Players[PlayerIndex].pPed);
#endif
#endif //GTA_PS2

//			gStartX = NewX;
//			gStartY = NewY;
//			gStartZ = NewZ;

//	return index of player so that script can refer to him - maybe a bit of a waste
//		as this is the same as the first parameter passed in
			ScriptParams[0] = PlayerIndex;

			StoreParameters(1);

#ifndef FINAL
	//	Assume the player is alive when first created
			CWorld::Players[PlayerIndex].pPed->m_nFlags.bCheckedForDead = TRUE;
#endif

			CWorld::Players[PlayerIndex].pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimplePlayerOnFoot());
			
			return(0);		// keep going
			break;
/*
		case COMMAND_GET_PLAYER_COORDINATES:

			CollectParameters(1);

			PlayerIndex = *((Int32 *)&ScriptParams[0]);
	
			ASSERTOBJ(CWorld::Players[PlayerIndex].pPed, ScriptName, "GET_PLAYER_COORDINATES - Player exists but doesn't have a character pointer");
			ASSERTOBJ(CWorld::Players[PlayerIndex].pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "GET_PLAYER_COORDINATES - Check player character is alive this frame");

			if ( (CWorld::Players[PlayerIndex].pPed->m_nPedFlags.bInVehicle) && (CWorld::Players[PlayerIndex].pPed->m_pMyVehicle) )
			{
				ASSERTOBJ(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle, ScriptName, "GET_PLAYER_COORDINATES - Player is not in a vehicle");
//				ASSERTOBJ(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "GET_PLAYER_COORDINATES - Check player's car is alive this frame");

				TempCoors = CWorld::Players[PlayerIndex].pPed->m_pMyVehicle->GetPosition();
			}
			else
			{
				TempCoors = CWorld::Players[PlayerIndex].pPed->GetPosition();
			}				


//			TempCoors = FindPlayerCoors();

			ScriptParams[0] = *((Int32 *)&(TempCoors.x) );
			ScriptParams[1] = *((Int32 *)&(TempCoors.y) );
			ScriptParams[2] = *((Int32 *)&(TempCoors.z) );

 			StoreParameters(3);

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_SET_PLAYER_COORDINATES:

			CollectParameters(4);

			PlayerIndex = *((Int32 *)&ScriptParams[0]);

			TempCoors.x = *((float *)&ScriptParams[1]);
			TempCoors.y = *((float *)&ScriptParams[2]);
			TempCoors.z = *((float *)&ScriptParams[3]);

			if (TempCoors.z <= -100.0f)
			{
				TempCoors.z = CWorld::FindGroundZForCoord(TempCoors.x, TempCoors.y);
			}

			ASSERTOBJ(CWorld::Players[PlayerIndex].pPed, ScriptName, "SET_PLAYER_COORDINATES - Player exists but doesn't have a character pointer");
			ASSERTOBJ(CWorld::Players[PlayerIndex].pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_PLAYER_COORDINATES - Check player character is alive this frame");

//			if (CWorld::Players[PlayerIndex].pVehicle == NULL)
			if ( (CWorld::Players[PlayerIndex].pPed->m_nPedFlags.bInVehicle) && (CWorld::Players[PlayerIndex].pPed->m_pMyVehicle) )
			{
// When a car is teleported, its suspension must be reset so that the force of the springs does not
//	create a large Z speed
//				TempCoors.z += 1.0f;	// Safety margin
				ASSERTOBJ(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle, ScriptName, "SET_PLAYER_COORDINATES - Player is not in a vehicle");
//				ASSERTOBJ(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_PLAYER_COORDINATES - Check player's car is alive this frame");
				
				TempCoors.z += CWorld::Players[PlayerIndex].pPed->m_pMyVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
				
				if (CWorld::Players[PlayerIndex].pPed->m_pMyVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BOAT)
				{
					((CBoat*)(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle))->Teleport(TempCoors);
					
					CTheScripts::ClearSpaceForMissionEntity(TempCoors, CWorld::Players[PlayerIndex].pPed->m_pMyVehicle);
				}
				else if (CWorld::Players[PlayerIndex].pPed->m_pMyVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
				{
					((CBike*)(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle))->Teleport(TempCoors);
					
					CTheScripts::ClearSpaceForMissionEntity(TempCoors, CWorld::Players[PlayerIndex].pPed->m_pMyVehicle);
				}
				else
				{
					((CAutomobile*)(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle))->Teleport(TempCoors);
					
					CTheScripts::ClearSpaceForMissionEntity(TempCoors, CWorld::Players[PlayerIndex].pPed->m_pMyVehicle);
					
//					((CAutomobile*)(CWorld::Players[PlayerIndex].pPed->m_pMyVehicle))->PlaceOnRoadProperly();
				}
			}
			else
			{
//				TempCoors.z += 2.0f;	// Safety margin
				TempCoors.z += CWorld::Players[PlayerIndex].pPed->GetDistanceFromCentreOfMassToBaseOfModel();
				
				const CVector vOldPlayerPos=CWorld::Players[PlayerIndex].pPed->GetPosition();
				CWorld::Players[PlayerIndex].pPed->Teleport(TempCoors);
				
				CTheScripts::ClearSpaceForMissionEntity(TempCoors, CWorld::Players[PlayerIndex].pPed);
				
				
				CPed* pPlayerPed=CWorld::Players[PlayerIndex].pPed;
				CPed* pFollowerPed;
				int k;
				
				if (pPlayerPed)
				{
					for(k=0;k<pPlayerPed->m_NumClosePeds;k++)
					{
					    pFollowerPed=pPlayerPed->m_apClosePeds[k];
					    if ( (pFollowerPed) && (IsPedPointerValid(pFollowerPed) ) )
					    {
					        if((pFollowerPed->m_pObjectivePed==pPlayerPed)&&(FOLLOW_CHAR_IN_FORMATION==pFollowerPed->m_Objective))
					        {
	                            CVector vFollowerPedPos=pFollowerPed->GetFormationPosition();
	                            CTheScripts::ClearSpaceForMissionEntity(vFollowerPedPos,pFollowerPed);
	                            bool bFoundGround=false;  
	                            const float fGroundZ=1.0f+CWorld::FindGroundZFor3DCoord(vFollowerPedPos.x,vFollowerPedPos.y,vFollowerPedPos.z+1.0f,&bFoundGround);
			                    if(bFoundGround)
			                    {
			                        vFollowerPedPos.z=fGroundZ;		                        
			                        if(CWorld::GetIsLineOfSightClear(vFollowerPedPos,pPlayerPed->GetPosition(),true,false,false,true,false,false,false))
			                        {
	       		                        pFollowerPed->Teleport(vFollowerPedPos);
	                                }
			                    }
			                }
					        else if(pFollowerPed->pLeader==pPlayerPed)
					        {
					            CVector vFollowerPedPos;
					            if(PED_FORMATION_NONE==pFollowerPed->m_FormationType)
					            {
					                vFollowerPedPos=pPlayerPed->GetPosition()+pFollowerPed->GetPosition()-vOldPlayerPos;
					            }
					            else
					            {
    					            vFollowerPedPos=pFollowerPed->GetFormationPosition();
					            }
	                            CTheScripts::ClearSpaceForMissionEntity(vFollowerPedPos,pFollowerPed);
	                            bool bFoundGround=false;
			                    const float fGroundZ=1.0f+CWorld::FindGroundZFor3DCoord(vFollowerPedPos.x,vFollowerPedPos.y,vFollowerPedPos.z+1.0f,&bFoundGround);
			                    if(bFoundGround)
			                    {
			                        vFollowerPedPos.z=fGroundZ;		                        
			                        if(CWorld::GetIsLineOfSightClear(vFollowerPedPos,pPlayerPed->GetPosition(),true,false,false,true,false,false,false))
			                        {
	       		                        pFollowerPed->Teleport(vFollowerPedPos);
	                                }	                       
			                    }
					        }
					    }
					}
                }				  
           	}

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_IS_PLAYER_IN_AREA_2D:

			CollectParameters(6);

			// First parameter is Player ID
			// Second, third parameters are MinX, MinY
			// Fourth, fifth parameters are MaxX, MaxY
			// Sixth parameter is flag determining whether area should be highlighted

			pPlayer = &(CWorld::Players[*((Int32 *)&ScriptParams[0])]);

			ASSERTOBJ(pPlayer, ScriptName, "IS_PLAYER_IN_AREA_2D - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "IS_PLAYER_IN_AREA_2D - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_AREA_2D - Check player character is alive this frame");

			if ( (pPlayer->pPed->m_nPedFlags.bInVehicle) && (pPlayer->pPed->m_pMyVehicle) )
			{
				ASSERTOBJ(pPlayer->pPed->m_pMyVehicle, ScriptName, "IS_PLAYER_IN_AREA_2D - Player is not in a vehicle");
//				ASSERTOBJ(pPlayer->pPed->m_pMyVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_AREA_2D - Check player's car is alive this frame");

				if (pPlayer->pPed->m_pMyVehicle->IsWithinArea(*((float *)&ScriptParams[1]), 
										*((float *)&ScriptParams[2]),
										*((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4])))
				{
					LatestCmpFlagResult = TRUE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
				else
				{
					LatestCmpFlagResult = FALSE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
			}
			else
			{
				if (pPlayer->pPed->IsWithinArea(*((float *)&ScriptParams[1]), 
										*((float *)&ScriptParams[2]),
										*((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4])))
				{
					LatestCmpFlagResult = TRUE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
				else
				{
					LatestCmpFlagResult = FALSE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
			}

			if (ScriptParams[5])
			{
				CTheScripts::HighlightImportantArea( ((UInt32)(this) + PC), 
					*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
					*((float *)&ScriptParams[3]), *((float *)&ScriptParams[4]), 
					-100.0f);
			}

			if (CTheScripts::DbgFlag)
			{
				CTheScripts::DrawDebugSquare(*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
								*((float *)&ScriptParams[3]), *((float *)&ScriptParams[4]));
			}

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_IS_PLAYER_IN_AREA_3D:

			CollectParameters(8);
			
			// First parameter is Player ID
			// Second. third, fourth parameters are MinX, MinY, MinZ
			// Fifth, sixth, seventh parameters are MaxX, MaxY, MaxZ
			// Eighth parameter is flag determining whether area should be highlighted

			pPlayer = &(CWorld::Players[*((Int32 *)&ScriptParams[0])]);

			ASSERTOBJ(pPlayer, ScriptName, "IS_PLAYER_IN_AREA_3D - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "IS_PLAYER_IN_AREA_3D - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_AREA_3D - Check player character is alive this frame");

			if ( (pPlayer->pPed->m_nPedFlags.bInVehicle) && (pPlayer->pPed->m_pMyVehicle) )
			{
				ASSERTOBJ(pPlayer->pPed->m_pMyVehicle, ScriptName, "IS_PLAYER_IN_AREA_3D - Player is not in a vehicle");
//				ASSERTOBJ(pPlayer->pPed->m_pMyVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_AREA_3D - Check player's car is alive this frame");
				
				if (pPlayer->pPed->m_pMyVehicle->IsWithinArea(*((float *)&ScriptParams[1]), 
										*((float *)&ScriptParams[2]),
										*((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4]),
										*((float *)&ScriptParams[5]),
										*((float *)&ScriptParams[6])))
				{
					LatestCmpFlagResult = TRUE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
				else
				{
					LatestCmpFlagResult = FALSE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
			}
			else
			{
				if (pPlayer->pPed->IsWithinArea(*((float *)&ScriptParams[1]), 
										*((float *)&ScriptParams[2]),
										*((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4]),
										*((float *)&ScriptParams[5]),
										*((float *)&ScriptParams[6])))
				{
					LatestCmpFlagResult = TRUE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
				else
				{
					LatestCmpFlagResult = FALSE;
					UpdateCompareFlag(LatestCmpFlagResult);
				}
			}

			if (ScriptParams[7])
			{
*/
/*
				if (*((float *)&ScriptParams[3]) > *((float *)&ScriptParams[6]))
				{
					LowestZ = *((float *)&ScriptParams[6]);
				}
				else
				{
					LowestZ = *((float *)&ScriptParams[3]);
				}
*/
/*
				CentreZ = ( *((float *)&ScriptParams[3]) + *((float *)&ScriptParams[6]) ) / 2.0f;
				
				CTheScripts::HighlightImportantArea( ((UInt32)(this) + PC), 
					*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
					*((float *)&ScriptParams[4]), *((float *)&ScriptParams[5]), 
					CentreZ);	//	LowestZ);
			}

			if (CTheScripts::DbgFlag)
			{
				CTheScripts::DrawDebugCube(*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), *((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4]), *((float *)&ScriptParams[5]), *((float *)&ScriptParams[6]));
			}

			return(0);		// keep going
			break;
*/
		case COMMAND_ADD_INT_VAR_TO_INT_VAR:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar += *pGlobalVar2;

			return(0);		// keep going
			break;
		case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_VAR:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) += *((float *)pGlobalVar2);

			return(0);		// keep going
			break;
//	90
		case COMMAND_ADD_INT_LVAR_TO_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pLocalVar += *pLocalVar2;

			return(0);		// keep going
			break;
		case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) += *((float *)pLocalVar2);

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_ADD_INT_VAR_TO_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pLocalVar += *pGlobalVar;

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pLocalVar) += *((float *)pGlobalVar);

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_ADD_INT_LVAR_TO_INT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pGlobalVar += *pLocalVar;

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pGlobalVar) += *((float *)pLocalVar);

			return(0);		// keep going
			break;

		case COMMAND_SUB_INT_VAR_FROM_INT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar -= *pGlobalVar2;

			return(0);		// keep going
			break;
		case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) -= *((float *)pGlobalVar2);

			return(0);		// keep going
			break;
		case COMMAND_SUB_INT_LVAR_FROM_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);

			*pLocalVar -= *pLocalVar2;

			return(0);		// keep going
			break;
		case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) -= *((float *)pLocalVar2);

			return(0);		// keep going
			break;
	}
	return(-1);		// Shouldn't be here. Unknown command probably.
}

Int8 CRunningScript::ProcessCommands100To199(Int32 CurrCommand)
{
	Bool8 LatestCmpFlagResult;

//	char ErrorMsg[256];

	CPed *pPed;
	CVehicle *pVehicle;
	CVector TempCoors;

	GxtChar *pString;
	Char TextLabel[10];	//	should only really need 8 characters

//	CAutomobile *pNewCar;
//	CBoat *pNewBoat;
//	CBike *pNewBike;
	CVehicle *pNewVehicle;

	float NewX, NewY, NewZ;
	float CentreZ;	//	LowestZ;
	
	Int32 *pGlobalVar, *pGlobalVar2;
	Int32 *pLocalVar, *pLocalVar2;

//	Int32 WanderDir;

	switch(CurrCommand)
	{
//	100
// GSW changed
		case COMMAND_SUB_INT_VAR_FROM_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pLocalVar -= *pGlobalVar;

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pLocalVar) -= *((float *)pGlobalVar);

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_SUB_INT_LVAR_FROM_INT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pGlobalVar -= *pLocalVar;

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pGlobalVar) -= *((float *)pLocalVar);

			return(0);		// keep going
			break;

		case COMMAND_MULT_INT_VAR_BY_INT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar *= *pGlobalVar2;

			return(0);		// keep going
			break;
		case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) *= *((float *)pGlobalVar2);

			return(0);		// keep going
			break;
		case COMMAND_MULT_INT_LVAR_BY_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pLocalVar *= *pLocalVar2;

			return(0);		// keep going
			break;
		case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) *= *((float *)pLocalVar2);

			return(0);		// keep going
			break;
		case COMMAND_MULT_INT_VAR_BY_INT_LVAR:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pGlobalVar *= *pLocalVar;

			return(0);		// keep going
			break;
		case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pGlobalVar) *= *((float *)pLocalVar);

			return(0);		// keep going
			break;
//	110
		case COMMAND_MULT_INT_LVAR_BY_INT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pLocalVar *= *pGlobalVar;

			return(0);		// keep going
			break;
		case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pLocalVar) *= *((float *)pGlobalVar);

			return(0);		// keep going
			break;

		case COMMAND_DIV_INT_VAR_BY_INT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar /= *pGlobalVar2;

			return(0);		// keep going
			break;
		case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) /= *((float *)pGlobalVar2);

			return(0);		// keep going
			break;
		case COMMAND_DIV_INT_LVAR_BY_INT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pLocalVar /= *pLocalVar2;

			return(0);		// keep going
			break;
		case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) /= *((float *)pLocalVar2);

			return(0);		// keep going
			break;
		case COMMAND_DIV_INT_VAR_BY_INT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pGlobalVar /= *pLocalVar;

			return(0);		// keep going
			break;
		case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_LVAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pGlobalVar) /= *((float *)pLocalVar);

			return(0);		// keep going
			break;
		case COMMAND_DIV_INT_LVAR_BY_INT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pLocalVar /= *pGlobalVar;

			return(0);		// keep going
			break;
		case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_VAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pLocalVar) /= *((float *)pGlobalVar);

			return(0);		// keep going
			break;
//	120
////////// Time Compensated adds/subs
		case COMMAND_ADD_TIMED_VAL_TO_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*((float *)pGlobalVar) += *((float *)&ScriptParams[0]) * CTimer::GetTimeStep();

			return (0);		// keep going
			break;
		case COMMAND_ADD_TIMED_VAL_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*((float *)pLocalVar) += *((float *)&ScriptParams[0]) * CTimer::GetTimeStep();

			return (0);		// keep going
			break;
		case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) += *((float *)pGlobalVar2) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;
		case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) += *((float *)pLocalVar2) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pLocalVar) += *((float *)pGlobalVar) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pGlobalVar) += *((float *)pLocalVar) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;

		case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			CollectParameters(1);
			
			*((float *)pGlobalVar) -= *((float *)&ScriptParams[0]) * CTimer::GetTimeStep();

			return (0);		// keep going
			break;
		case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			CollectParameters(1);
			
			*((float *)pLocalVar) -= *((float *)&ScriptParams[0]) * CTimer::GetTimeStep();

			return (0);		// keep going
			break;
		case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) -= *((float *)pGlobalVar2) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;
		case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) -= *((float *)pLocalVar2) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;
//	130
// GSW changed
		case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_LVAR:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pLocalVar) -= *((float *)pGlobalVar) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;
// GSW changed
		case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_VAR:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pGlobalVar) -= *((float *)pLocalVar) * CTimer::GetTimeStep();

			return(0);		// keep going
			break;

		case COMMAND_SET_VAR_INT_TO_VAR_INT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar = *pGlobalVar2;

			return(0);		// keep going
			break;
		case COMMAND_SET_LVAR_INT_TO_LVAR_INT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pLocalVar = *pLocalVar2;

			return(0);		// keep going
			break;

		case COMMAND_SET_VAR_FLOAT_TO_VAR_FLOAT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar = *pGlobalVar2;

			return(0);		// keep going
			break;

		case COMMAND_SET_LVAR_FLOAT_TO_LVAR_FLOAT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pLocalVar = *pLocalVar2;

			return(0);		// keep going
			break;

		case COMMAND_SET_VAR_FLOAT_TO_LVAR_FLOAT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pGlobalVar = *pLocalVar;

			return(0);		// keep going
			break;

		case COMMAND_SET_LVAR_FLOAT_TO_VAR_FLOAT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pLocalVar = *pGlobalVar;

			return(0);		// keep going
			break;

		case COMMAND_SET_VAR_INT_TO_LVAR_INT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pGlobalVar = *pLocalVar;

			return(0);		// keep going
			break;
		case COMMAND_SET_LVAR_INT_TO_VAR_INT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pLocalVar = *pGlobalVar;

			return(0);		// keep going
			break;
//	140	
				// Conversion sets
		case COMMAND_CSET_VAR_INT_TO_VAR_FLOAT:
		
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar = (Int32) (*((float *)pGlobalVar2));

			return(0);		// keep going
			break;
		case COMMAND_CSET_VAR_FLOAT_TO_VAR_INT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pGlobalVar2 = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) = (float) (*pGlobalVar2);

			return(0);		// keep going
			break;
		case COMMAND_CSET_LVAR_INT_TO_VAR_FLOAT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pLocalVar = (Int32) (*((float *)pGlobalVar));

			return(0);		// keep going
			break;
		case COMMAND_CSET_LVAR_FLOAT_TO_VAR_INT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pLocalVar) = (float) (*pGlobalVar);

			return(0);		// keep going
			break;
		case COMMAND_CSET_VAR_INT_TO_LVAR_FLOAT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pGlobalVar = (Int32) *((float *)pLocalVar);

			return(0);		// keep going
			break;
		case COMMAND_CSET_VAR_FLOAT_TO_LVAR_INT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pGlobalVar) = (float) *pLocalVar;

			return(0);		// keep going
			break;
		case COMMAND_CSET_LVAR_INT_TO_LVAR_FLOAT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pLocalVar = (Int32) *((float *)pLocalVar2);

			return(0);		// keep going
			break;
		case COMMAND_CSET_LVAR_FLOAT_TO_LVAR_INT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			pLocalVar2 = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) = (float) *pLocalVar2;

			return(0);		// keep going
			break;

		case COMMAND_ABS_VAR_INT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar = ABS(*pGlobalVar);

			return(0);		// keep going
			break;
		case COMMAND_ABS_LVAR_INT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*pLocalVar = ABS(*pLocalVar);

			return(0);		// keep going
			break;
//	150
		case COMMAND_ABS_VAR_FLOAT:

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*((float *)pGlobalVar) = ABS(*((float *)pGlobalVar));

			return(0);		// keep going
			break;
		case COMMAND_ABS_LVAR_FLOAT:

			pLocalVar = GetPointerToScriptVariable(SCOPE_LOCAL);
			
			*((float *)pLocalVar) = ABS(*((float *)pLocalVar));

			return(0);		// keep going
			break;

		case COMMAND_GENERATE_RANDOM_FLOAT:		// between 0.0 and 1.0

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			CGeneral::GetRandomNumber();
			CGeneral::GetRandomNumber();
			CGeneral::GetRandomNumber();
			*((float *)pGlobalVar) = CGeneral::GetRandomNumber() / 65536.0f;

			return(0);		// keep going
			break;
		case COMMAND_GENERATE_RANDOM_INT:		// between 0 and 65535

			pGlobalVar = GetPointerToScriptVariable(SCOPE_GLOBAL);
			
			*pGlobalVar = CGeneral::GetRandomNumber();

			return(0);		// keep going
			break;

				// Some character test stuff.
		case COMMAND_CREATE_CHAR:		// This allocates a ped and returns a pointer to it		// UNFINISHED

//	Maybe the create command should take in the type of ped civilian, cop, emergency etc.
//	and the subtype within this type Male, Female, Fireman etc.
			CollectParameters(5);

			#ifndef FINAL
				CTheScripts::CheckThisModelHasBeenRequestedAndLoaded(ScriptParams[1]);
			#endif

			GetCorrectPedModelIndexForEmergencyServiceType(ScriptParams[0], &ScriptParams[1]);

			if (ScriptParams[0] == PEDTYPE_COP)
			{
				pPed = new CCopPed((eCopType) ScriptParams[1]);
			}
			else if ((ScriptParams[0] == PEDTYPE_MEDIC) || (ScriptParams[0] == PEDTYPE_FIRE))
			{
				pPed = new CEmergencyPed(ScriptParams[0], ScriptParams[1]);
			}
			else
			{
				pPed = new CCivilianPed((ePedType)ScriptParams[0], ScriptParams[1]);
			}
			
			ASSERTOBJ(pPed, ScriptName, "CREATE_CHAR - Couldn't create a new ped");

			pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimpleStandStill(999999,true));

			pPed->SetCharCreatedBy(MISSION_CHAR);
			//pPed->SetThreatReactionOverwritesObjectiveFlag(FALSE);
			pPed->m_nPedFlags.bAllowMedicsToReviveMe = FALSE;
			//pPed->m_nPedFlags.bPlayerFriend = false;
			
			NewX = *((float *)&ScriptParams[2]);
			NewY = *((float *)&ScriptParams[3]);
			NewZ = *((float *)&ScriptParams[4]);

			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
//				NewZ += 2.0f;	// Safety margin
			}

			// Ped are 1 metre above the ground
			NewZ += 1.0f;//pPed->GetDistanceFromCentreOfMassToBaseOfModel();

			TempCoors = CVector(NewX, NewY, NewZ);

			pPed->SetPosition(TempCoors);

			pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));

			CTheScripts::ClearSpaceForMissionEntity(TempCoors, pPed);

			if (IsThisAMissionScript)
			{	//	Can only set this flag in a mission script because it is only cleared when the entity is in 
				//	the mission cleanup array
				pPed->m_nFlags.bIsStaticWaitingForCollision = true;	//	Set this before adding to the World
			}

			CWorld::Add(pPed);
			
//			pPed->LivesInThisLevel = CTheZones::GetLevelFromPosition(&TempCoors);

			CPopulation::ms_nTotalMissionPeds++;

			ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);

			StoreParameters(1);

// Should only be called for mission scripts (not the main script)
			if (IsThisAMissionScript)
			{
				CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
			}
			else
			{
#ifndef FINAL
				CTheScripts::MissionCleanUp.AddEntityToNonMissionList(ScriptParams[0], CLEANUP_CHAR, ScriptName);
#endif
			}


#ifndef FINAL
			// Assume the ped is alive in the frame in which it is created
			pPed->m_nFlags.bCheckedForDead = TRUE;
#endif

			return(0);		// keep going
			break;
			
		case COMMAND_DELETE_CHAR:		// Deletes this ped

			CollectParameters(1);
			
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);

/*
			if (pPed)
			{
				if (pPed->m_nPedFlags.bInVehicle)
				{
					if(pPed->m_pMyVehicle)
					{
						if(pPed->m_pMyVehicle->pDriver == pPed) // need this check because there may be a new driver already
						{
							pPed->m_pMyVehicle->RemoveDriver();
							pPed->m_pMyVehicle->SetStatus(STATUS_ABANDONED);
							if (pPed->m_pMyVehicle->m_eDoorLockState == CARLOCK_LOCKED_INITIALLY) pPed->m_pMyVehicle->m_eDoorLockState = CARLOCK_UNLOCKED;
							
							if ((pPed->GetPedType()==PEDTYPE_COP) && (pPed->m_pMyVehicle->IsLawEnforcementVehicle()))
							{
								pPed->m_pMyVehicle->ChangeLawEnforcerState(FALSE);
							}
						}
						else
						{
								pPed->m_pMyVehicle->RemovePassenger(pPed);
						}
					
					}
				}

				CWorld::RemoveReferencesToDeletedObject(pPed);
			
				delete(pPed);

				CPopulation::ms_nTotalMissionPeds--;
			}
*/
			CTheScripts::RemoveThisPed(pPed);
			
			if (IsThisAMissionScript)
			{
				CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
			}
			else
			{
#ifndef FINAL
				CTheScripts::MissionCleanUp.RemoveEntityFromNonMissionList(ScriptParams[0], CLEANUP_CHAR, ScriptName);
#endif
			}

			return(0);		// keep going
			break;
/*
		case COMMAND_CHAR_WANDER_DIR:

			CollectParameters(2);
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "CHAR_WANDER_DIR - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CHAR_WANDER_DIR - Check character is alive this frame");

			pPed->ClearAll();
			
			if ((ScriptParams[1] < 0) || (ScriptParams[1] > 7))
			{
				WanderDir = CGeneral::GetRandomNumberInRange(0, 7);
			}
			else
			{
				WanderDir = ScriptParams[1];
			}
			
			pPed->SetWanderPath(WanderDir);
			return(0);		// keep going
			break;
*/
/*
		case COMMAND_CHAR_FOLLOW_PATH:

			CollectParameters(6);
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "CHAR_FOLLOW_PATH - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CHAR_FOLLOW_PATH - Check character is alive this frame");

			if((pPed->m_nPedState == PED_ATTACK)||(pPed->m_nPedState == PED_FIGHT)||!pPed->IsPedInControl())
				return(0);		// keep going

			NewX = *((float *)&ScriptParams[1]);
			NewY = *((float *)&ScriptParams[2]);
			NewZ = *((float *)&ScriptParams[3]);
					
			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
			}
			
			const float fGotoCoordOnFootFromPathDistance = *((float *)&ScriptParams[4]);
			
		    const int iPedMoveState = *((int *)&ScriptParams[5]);
			eMoveState eGotoCoordOnFootMoveState=PEDMOVE_NONE;
			switch(iPedMoveState)
			{   
			    case 0:
    			    eGotoCoordOnFootMoveState=PEDMOVE_WALK;
    			    break;
    		    case 1:
    			    eGotoCoordOnFootMoveState=PEDMOVE_RUN;
    			    break;
    			default:
    			    eGotoCoordOnFootMoveState=PEDMOVE_WALK;
    			    break;
    	    }
    		
			pPed->ClearAll();
			pPed->m_iPathFollowTimer=0;
			pPed->SetFollowPath(CVector(NewX, NewY, NewZ),fGotoCoordOnFootFromPathDistance,eGotoCoordOnFootMoveState);
			return(0);		// keep going
			break;
*/
/*
		case COMMAND_CHAR_SET_IDLE:

			CollectParameters(1);

			// First parameter is Char ID

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "CHAR_SET_IDLE - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CHAR_SET_IDLE - Check character is alive this frame");

//			pPed->ClearAll();
//			pPed->SetIdle();
			pPed->SetScriptObjectivePassedFlag(FALSE);
			pPed->SetObjective(WAIT_ON_FOOT);

			return(0);		// keep going
			break;
*/
//	160
		case COMMAND_GET_CHAR_COORDINATES:

			CollectParameters(1);

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "GET_CHAR_COORDINATES - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "GET_CHAR_COORDINATES - Check character is alive this frame");

			if (pPed->m_nPedFlags.bInVehicle)
			{
				pVehicle = pPed->m_pMyVehicle;
//				ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "GET_CHAR_COORDINATES - Check character's car is alive this frame");
			}
			else
			{
				pVehicle = NULL;
			}

			if (pVehicle)
			{
				TempCoors = pVehicle->GetPosition();
			}
			else
			{
				TempCoors = pPed->GetPosition();
			}

			ScriptParams[0] = *((Int32 *)&(TempCoors.x));
			ScriptParams[1] = *((Int32 *)&(TempCoors.y));
			ScriptParams[2] = *((Int32 *)&(TempCoors.z));
			StoreParameters(3);

			return(0);		// keep going
			break;
		case COMMAND_SET_CHAR_COORDINATES:

			CollectParameters(4);

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "SET_CHAR_COORDINATES - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CHAR_COORDINATES - Check character is alive this frame");

			NewX = *((float *)&ScriptParams[1]);
			NewY = *((float *)&ScriptParams[2]);
			NewZ = *((float *)&ScriptParams[3]);

			SetCharCoordinates(pPed, NewX, NewY, NewZ, true);

			return(0);		// keep going
			break;
/*
		case COMMAND_IS_CHAR_STILL_ALIVE:
#ifdef GTA_LIBERTY
			CollectParameters(1);
			
			// Parameter is Character ID
			
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			
//			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_STILL_ALIVE - Character doesn't exist");
			
			if (pPed)
			{
				if ( (pPed->GetPedState() == PED_DEAD) || (pPed->GetPedState() == PED_DIE) )
				{
					LatestCmpFlagResult = FALSE;
				}
				else
				{
					LatestCmpFlagResult = TRUE;
#ifndef FINAL
					pPed->m_nFlags.bCheckedForDead = TRUE;
#endif
				}
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

			UpdateCompareFlag(LatestCmpFlagResult);
#else
			ASSERTOBJ(0, ScriptName, "IS_CHAR_STILL_ALIVE - command only valid in GTA_LIBERTY");
#endif
			return(0);		// keep going
			break;
*/
		case COMMAND_IS_CHAR_IN_AREA_2D :

			CollectParameters(6);

			// First parameter is CharID
			// Second, third parameters are MinX, MinY
			// Fourth, fifth parameters are MaxX, MaxY
			// Sixth parameter is flag determining whether area should be highlighted
			
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_IN_AREA_2D - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_AREA_2D - Check character is alive this frame");

			if (pPed->m_nPedFlags.bInVehicle)
			{
				pVehicle = pPed->m_pMyVehicle;
//				ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_AREA_2D - Check character's car is alive this frame");
			}
			else
			{
				pVehicle = NULL;
			}

			if (pVehicle)
			{
				if (pVehicle->IsWithinArea(*((float *)&ScriptParams[1]), 
									*((float *)&ScriptParams[2]),
									*((float *)&ScriptParams[3]),
									*((float *)&ScriptParams[4])))
				{
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
			}
			else
			{	// Character is on foot

				if (pPed->IsWithinArea(*((float *)&ScriptParams[1]), 
										*((float *)&ScriptParams[2]),
										*((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4])))
				{
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			if (ScriptParams[5])
			{
				CTheScripts::HighlightImportantArea( ((UInt32)(this) + (UInt32)PCPointer), 
					*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
					*((float *)&ScriptParams[3]), *((float *)&ScriptParams[4]), 
					-100.0f);
			}

			if (CTheScripts::DbgFlag)
			{
				CTheScripts::DrawDebugSquare(*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
											*((float *)&ScriptParams[3]), *((float *)&ScriptParams[4]));
			}

			return(0);		// keep going
			break;

		case COMMAND_IS_CHAR_IN_AREA_3D :

			CollectParameters(8);
			
			// First parameter is Char ID
			// Second, third, fourth parameters are MinX, MinY, MinZ
			// Fifth, sixth, seventh parameters are MaxX, MaxY, MaxZ
			// Eighth parameter is flag determining whether area should be highlighted

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_IN_AREA_3D - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_AREA_3D - Check character is alive this frame");

			if (pPed->m_nPedFlags.bInVehicle)
			{
				pVehicle = pPed->m_pMyVehicle;
//				ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_AREA_3D - Check character's car is alive this frame");
			}
			else
			{
				pVehicle = NULL;
			}

			if (pVehicle)
			{
				if (pVehicle->IsWithinArea(*((float *)&ScriptParams[1]), 
										*((float *)&ScriptParams[2]),
										*((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4]),
										*((float *)&ScriptParams[5]),
										*((float *)&ScriptParams[6])))
				{
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
			}
			else
			{
				if (pPed->IsWithinArea(*((float *)&ScriptParams[1]), 
										*((float *)&ScriptParams[2]),
										*((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4]),
										*((float *)&ScriptParams[5]),
										*((float *)&ScriptParams[6])))
				{
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			if (ScriptParams[7])
			{
/*
				if (*((float *)&ScriptParams[3]) > *((float *)&ScriptParams[6]))
				{
					LowestZ = *((float *)&ScriptParams[6]);
				}
				else
				{
					LowestZ = *((float *)&ScriptParams[3]);
				}
*/
				CentreZ = ( *((float *)&ScriptParams[3]) + *((float *)&ScriptParams[6]) ) / 2.0f;
				
				CTheScripts::HighlightImportantArea( ((UInt32)(this) + (UInt32)PCPointer), 
					*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
					*((float *)&ScriptParams[4]), *((float *)&ScriptParams[5]), 
					CentreZ);	//	LowestZ);
			}				

			if (CTheScripts::DbgFlag)
			{
				CTheScripts::DrawDebugCube(*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), *((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4]), *((float *)&ScriptParams[5]), *((float *)&ScriptParams[6]));
			}
			
			return(0);		// keep going
			break;

				// Some vehicle test stuff.
		case COMMAND_CREATE_CAR:		// This allocates a vehicle and returns its pointer

			ASSERTOBJ(CCarCtrl::NumMissionCars < MAX_NUM_MISSION_CARS, ScriptName, "CREATE_CAR - Too many mission cars");

			CollectParameters(4);
			
			// First parameter is Model to create
			// Second, third, fourth parameters are X Y Z
			
			#ifndef FINAL
				CTheScripts::CheckThisModelHasBeenRequestedAndLoaded(ScriptParams[0]);
			#endif

			NewX = *((float *)&ScriptParams[1]);
			NewY = *((float *)&ScriptParams[2]);
			NewZ = *((float *)&ScriptParams[3]);
			
			ASSERTOBJ( (ScriptParams[0] >= MODELID_CAR_FIRST) && (ScriptParams[0] < 612), ScriptName, "CREATE_CAR - this is not a vehicle model");

			pNewVehicle = CCarCtrl::CreateCarForScript(ScriptParams[0], CVector(NewX, NewY, NewZ), IsThisAMissionScript);

			ScriptParams[0] = CPools::GetVehiclePool().GetIndex(pNewVehicle);
			StoreParameters(1);
		
#ifndef FINAL
//	Assume the new car is alive in the frame it is created
			pNewVehicle->m_nFlags.bCheckedForDead = TRUE;
			
			if (!IsThisAMissionScript)
			{
				CTheScripts::MissionCleanUp.AddEntityToNonMissionList(ScriptParams[0], CLEANUP_CAR, ScriptName);
			}
#endif

/*
// Should only be called for mission scripts (not the main script)
			if (IsThisAMissionScript)
			{
				CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CAR);
			}
*/
			return(0);		// keep going
			break;

		case COMMAND_DELETE_CAR:		// Deletes this car

			CollectParameters(1);
			
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
//			ASSERTOBJ(pVehicle, ScriptName, "DELETE_CAR - Car doesn't exist");

			if (pVehicle)
			{			
				CWorld::Remove(pVehicle);
				CWorld::RemoveReferencesToDeletedObject(pVehicle);
				delete(pVehicle);
			}

			if (IsThisAMissionScript)
			{
				CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
			}
			else
			{
#ifndef FINAL
				CTheScripts::MissionCleanUp.RemoveEntityFromNonMissionList(ScriptParams[0], CLEANUP_CAR, ScriptName);
#endif
			}
			
			return(0);		// keep going
			break;

			// This one is not implemented yet
		case COMMAND_CAR_GOTO_COORDINATES:			// UNFINISHED

			CollectParameters(4);
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "CAR_GOTO_COORDINATES - Car doesn't exist");

			ASSERTOBJ(pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT, ScriptName, "CAR_GOTO_COORDINATES - can't call this for a boat");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CAR_GOTO_COORDINATES - Check car is alive this frame");
			
			NewX = *((float*)&ScriptParams[1]);
			NewY = *((float*)&ScriptParams[2]);
			NewZ = *((float*)&ScriptParams[3]);
			
			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
//				NewZ += 1.0f;	// Safety margin
			}

			NewZ += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();

//			if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, CVector(*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), *((float *)&ScriptParams[3])) ) )
			if (CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, CVector(NewX, NewY, NewZ) ) )
			{
				pVehicle->AutoPilot.SetMission(CAutoPilot::MISSION_GOTOCOORDINATES_STRAIGHTLINE);
			}
			else
			{
				pVehicle->AutoPilot.SetMission(CAutoPilot::MISSION_GOTOCOORDINATES);
			}

			pVehicle->SetStatus(STATUS_PHYSICS);		// or maybe STATUS_SIMPLE
			pVehicle->SetEngineOn(true);					// For sound

				// Make sure we've got a certain minimum speed
			pVehicle->AutoPilot.CruiseSpeed = MAX(pVehicle->AutoPilot.CruiseSpeed, MIN_CRUISE_SPEED);
			pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
			

			return(0);		// keep going
			break;

		case COMMAND_CAR_WANDER_RANDOMLY:

			CollectParameters(1);
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "CAR_WANDER_RANDOMLY - Car doesn't exist");
			
			ASSERTOBJ(pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT, ScriptName, "CAR_WANDER_RANDOMLY - Can't call this for a boat");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CAR_WANDER_RANDOMLY - Check car is alive this frame");

			CCarCtrl::JoinCarWithRoadSystem(pVehicle);
			pVehicle->AutoPilot.SetMission(CAutoPilot::MISSION_CRUISE);
			pVehicle->SetEngineOn(true);					// For sound

				// Make sure we've got a certain minimum speed
			pVehicle->AutoPilot.CruiseSpeed = MAX(pVehicle->AutoPilot.CruiseSpeed, MIN_CRUISE_SPEED);
			pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();

			return(0);		// keep going
			break;

		case COMMAND_CAR_SET_IDLE:

			CollectParameters(1);
			
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "CAR_SET_IDLE - Car doesn't exist");
			
			ASSERTOBJ(pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT, ScriptName, "CAR_SET_IDLE - Can't call this for a boat");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CAR_SET_IDLE - Check car is alive this frame");

			pVehicle->AutoPilot.SetMission(CAutoPilot::MISSION_NONE);
			return(0);		// keep going
			break;
//	170
		case COMMAND_GET_CAR_COORDINATES:

			CollectParameters(1);
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "GET_CAR_COORDINATES - Car doesn't exist");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "GET_CAR_COORDINATES - Check car is alive this frame");

			TempCoors = pVehicle->GetPosition();
			ScriptParams[0] = *((Int32 *)&(TempCoors.x));
			ScriptParams[1] = *((Int32 *)&(TempCoors.y));
			ScriptParams[2] = *((Int32 *)&(TempCoors.z));
			StoreParameters(3);
			return(0);		// keep going
			break;

		case COMMAND_SET_CAR_COORDINATES:

			CollectParameters(4);
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "SET_CAR_COORDINATES - Car doesn't exist");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CAR_COORDINATES - Check car is alive this frame");

			NewX = *((float*)&ScriptParams[1]);
			NewY = *((float*)&ScriptParams[2]);
			NewZ = *((float*)&ScriptParams[3]);
			
			CCarCtrl::SetCoordsOfScriptCar(pVehicle, NewX, NewY, NewZ, false);

			return(0);		// keep going
			break;
/*
		case COMMAND_IS_CAR_STILL_ALIVE:
#ifdef GTA_LIBERTY
			CollectParameters(1);
			
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
//			ASSERTOBJ(pVehicle, ScriptName, "IS_CAR_STILL_ALIVE - Car doesn't exist");

			if (pVehicle)
			{
				if (pVehicle->GetStatus() == STATUS_WRECKED)
				{
					LatestCmpFlagResult = FALSE;
				}
				else if ((pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT) && (pVehicle->IsInWater))
				{
					LatestCmpFlagResult = FALSE;
#ifndef FINAL
					pVehicle->m_nFlags.bCheckedForDead = TRUE;
#endif
				}
				else
				{
					LatestCmpFlagResult = TRUE;
#ifndef FINAL
					pVehicle->m_nFlags.bCheckedForDead = TRUE;
#endif
				}
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			
			UpdateCompareFlag(LatestCmpFlagResult);
#else
			ASSERTOBJ(0, ScriptName, "IS_CAR_STILL_ALIVE - command only valid in GTA_LIBERTY");
#endif
			return(0);		// keep going
			break;
*/
		case COMMAND_SET_CAR_CRUISE_SPEED:

			CollectParameters(2);
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "SET_CAR_CRUISE_SPEED - Car doesn't exist");
			
			ASSERTOBJ(pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT, ScriptName, "SET_CAR_CRUISE_SPEED - Can't call this command for a boat");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CAR_CRUISE_SPEED - Check car is alive this frame");
			
			pVehicle->AutoPilot.CruiseSpeed = *((float *)&ScriptParams[1]);
			pVehicle->AutoPilot.CruiseSpeed = MIN (pVehicle->AutoPilot.CruiseSpeed, pVehicle->pHandling->Transmission.m_fMaxFlatVelocity * 60.0f);	// limit maximum speed so that guy still slows down at corners
//			pVehicle->AutoPilot.NumPathNodes = 0;	// Obbe: Hopefully fixes the funeral bug. (added 28/7/2004) CANNOT do this since the scripters set the cruisespeed every frame.
			return(0);		// keep going
			break;
		case COMMAND_SET_CAR_DRIVING_STYLE:

			CollectParameters(2);
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "SET_CAR_DRIVING_STYLE - Car doesn't exist");
			ASSERTOBJ(pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT, ScriptName, "SET_CAR_DRIVING_STYLE - Can't call this command for a boat");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CAR_DRIVING_STYLE - Check car is alive this frame");

			pVehicle->AutoPilot.DrivingMode = ScriptParams[1];
			return(0);		// keep going
			break;
		case COMMAND_SET_CAR_MISSION:

			CollectParameters(2);
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "SET_CAR_MISSION - Car doesn't exist");

			ASSERTOBJ( ( (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_CAR) 
						|| (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BOAT)
						|| (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE) ), ScriptName, "SET_CAR_MISSION - Can only call this command for cars, boats and bikes");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CAR_MISSION - Check car is alive this frame");

			pVehicle->AutoPilot.SetMission(ScriptParams[1]);
			pVehicle->AutoPilot.LastTimeNotStuck = CTimer::GetTimeInMilliseconds();
			pVehicle->SetEngineOn(true);					// For sound
			return(0);		// keep going
			break;

		case COMMAND_IS_CAR_IN_AREA_2D:

			CollectParameters(6);
			
			// First parameter is Car ID
			// Second, third parameters are MinX, MinY
			// Fourth, fifth parameters are MaxX, MaxY
			// Sixth parameter is flag determining whether area should be highlighted

			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "IS_CAR_IN_AREA_2D - Car doesn't exist");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CAR_IN_AREA_2D - Check car is alive this frame");

			if (pVehicle->IsWithinArea(*((float *)&ScriptParams[1]), 
									*((float *)&ScriptParams[2]),
									*((float *)&ScriptParams[3]),
									*((float *)&ScriptParams[4])))
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			if (ScriptParams[5])
			{
				CTheScripts::HighlightImportantArea( ((UInt32)(this) + (UInt32)PCPointer), 
					*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
					*((float *)&ScriptParams[3]), *((float *)&ScriptParams[4]), 
					-100.0f);
			}
			
			if (CTheScripts::DbgFlag)
			{
				CTheScripts::DrawDebugSquare(*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
											*((float *)&ScriptParams[3]), *((float *)&ScriptParams[4]));
			}
			
			return(0);		// keep going
			break;

		case COMMAND_IS_CAR_IN_AREA_3D:

			CollectParameters(8);
			
			// First parameter is Car ID
			// Second, third, fourth parameters are MinX, MinY, MinZ
			// Fifth, sixth, seventh parameters are MaxX, MaxY, MaxZ
			// Eighth parameter is flag determining whether the area should be highlighted

			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "IS_CAR_IN_AREA_3D - Car doesn't exist");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CAR_IN_AREA_3D - Check car is alive this frame");

			if (pVehicle->IsWithinArea(*((float *)&ScriptParams[1]), 
									*((float *)&ScriptParams[2]),
									*((float *)&ScriptParams[3]),
									*((float *)&ScriptParams[4]),
									*((float *)&ScriptParams[5]),
									*((float *)&ScriptParams[6])))
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			if (ScriptParams[7])
			{
/*
				if (*((float *)&ScriptParams[3]) > *((float *)&ScriptParams[6]))
				{
					LowestZ = *((float *)&ScriptParams[6]);
				}
				else
				{
					LowestZ = *((float *)&ScriptParams[3]);
				}
*/
				CentreZ = ( *((float *)&ScriptParams[3]) + *((float *)&ScriptParams[6]) ) / 2.0f;
				
				CTheScripts::HighlightImportantArea( ((UInt32)(this) + (UInt32)PCPointer), 
					*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), 
					*((float *)&ScriptParams[4]), *((float *)&ScriptParams[5]), 
					CentreZ);	//	LowestZ);
			}
			
			if (CTheScripts::DbgFlag)
			{
				CTheScripts::DrawDebugCube(*((float *)&ScriptParams[1]), *((float *)&ScriptParams[2]), *((float *)&ScriptParams[3]),
										*((float *)&ScriptParams[4]), *((float *)&ScriptParams[5]), *((float *)&ScriptParams[6]));
			}
			
			return(0);		// keep going
			break;
/*
		case COMMAND_SPECIAL_0:
			return(0);		// keep going
			break;
		case COMMAND_SPECIAL_1:
			return(0);		// keep going
			break;
//	180
		case COMMAND_SPECIAL_2:
			return(0);		// keep going
			break;
		case COMMAND_SPECIAL_3:
			return(0);		// keep going
			break;
		case COMMAND_SPECIAL_4:
			return(0);		// keep going
			break;
		case COMMAND_SPECIAL_5:
			return(0);		// keep going
			break;
		case COMMAND_SPECIAL_6:
			return(0);		// keep going
			break;
		case COMMAND_SPECIAL_7:
			return(0);		// keep going
			break;
*/
		case COMMAND_PRINT_BIG:

			ReadTextLabelFromScript(&TextLabel[0]);
			pString = TheText.Get(TextLabel);
			ASSERTOBJ(pString, ScriptName, "PRINT_BIG - Can't find Key in the text file");
			
			CollectParameters(2);
			
			// Second parameter is length of time to display the message
			// Third parameter is row
			
			CMessages::AddBigMessage( pString, ScriptParams[0], (ScriptParams[1] - 1));

			return(0);		// keep going
			break;

		case COMMAND_PRINT:

			ReadTextLabelFromScript(&TextLabel[0]);
			pString = TheText.Get(TextLabel);
			ASSERTOBJ(pString, ScriptName, "PRINT - Can't find Key in the text file");
			
			CollectParameters(2);
			
			if (pString && (pString[0] == FO_CHAR_TOKEN_DELIMITER) && (pString[1] == FO_CHAR_TOKEN_DIALOGUE) && 
				(pString[2] == FO_CHAR_TOKEN_DELIMITER) && (!FrontEndMenuManager.m_PrefsShowSubtitles) )
			{	//	If this is dialogue and subtitles are turned off then don't display the text
			
			}
			else
			{
				CMessages::AddMessage( pString, ScriptParams[0], ScriptParams[1], CTheScripts::bAddNextMessageToPreviousBriefs);
			}
			CTheScripts::bAddNextMessageToPreviousBriefs = true;

			return(0);		// keep going
			break;

		case COMMAND_PRINT_NOW:

			ReadTextLabelFromScript(&TextLabel[0]);
			pString = TheText.Get(TextLabel);
			ASSERTOBJ(pString, ScriptName, "PRINT_NOW - Can't find Key in the text file");
			
			CollectParameters(2);

			if (pString && (pString[0] == FO_CHAR_TOKEN_DELIMITER) && (pString[1] == FO_CHAR_TOKEN_DIALOGUE) && 
				(pString[2] == FO_CHAR_TOKEN_DELIMITER) && (!FrontEndMenuManager.m_PrefsShowSubtitles) )
			{	//	If this is dialogue and subtitles are turned off then don't display the text
			
			}
			else
			{
				CMessages::AddMessageJumpQ( pString, ScriptParams[0], ScriptParams[1], CTheScripts::bAddNextMessageToPreviousBriefs);
			}
			CTheScripts::bAddNextMessageToPreviousBriefs = true;
			
			return(0);		// keep going
			break;
/*
		case COMMAND_PRINT_SOON:
			
			ReadTextLabelFromScript(&TextLabel);			
//			pString = TheText.Get( (char *) &CTheScripts::ScriptSpace[PC] );
			pString = TheText.Get(TextLabel);
			ASSERTOBJ(pString, ScriptName, "PRINT_SOON - Can't find Key in the text file");
//			PC += 8;
			
			CollectParameters(2);
			
			CMessages::AddMessageSoon( pString, ScriptParams[0], ScriptParams[1]);
			
			return(0);		// keep going
			break;
*/
//	190
		case COMMAND_CLEAR_PRINTS:

			CMessages::ClearMessages();
			return(0);		// keep going
			break;

		case COMMAND_GET_TIME_OF_DAY:

			ScriptParams[0] = CClock::GetGameClockHours();
			ScriptParams[1] = CClock::GetGameClockMinutes();
			StoreParameters(2);
			return(0);		// keep going
			break;

		case COMMAND_SET_TIME_OF_DAY:

			CollectParameters(2);
			CClock::SetGameClock(ScriptParams[0], ScriptParams[1]);
			return(0);		// keep going
			break;

		case COMMAND_GET_MINUTES_TO_TIME_OF_DAY:

			CollectParameters(2);
			ScriptParams[0] = CClock::GetGameClockMinutesUntil(ScriptParams[0], ScriptParams[1]);
			StoreParameters(1);
			return(0);		// keep going
			break;
/*
//	See COMMAND_DISPLAY_ONSCREEN_TIMER
		case COMMAND_START_TIMER:

			VarInt32 = CTheScripts::ReadInt32FromScript(&PC);
			CCDTimers::RegisterTimer(VarInt32);
			return(0);		// keep going
			break;

// See COMMAND_CLEAR_ONSCREEN_TIMER
		case COMMAND_STOP_TIMER:

			VarInt32 = CTheScripts::ReadInt32FromScript(&PC);
			CCDTimers::UnRegisterTimer(VarInt32);

			return(0);		// keep going
			break;
*/
		case COMMAND_IS_POINT_ON_SCREEN:
			
			CollectParameters(4);
			
			// First, second, third parameters are X Y Z
			// Fourth parameter is Radius
			
			NewX = *((float *)&ScriptParams[0]);
			NewY = *((float *)&ScriptParams[1]);
			NewZ = *((float *)&ScriptParams[2]);
			
			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
			}
			
			LatestCmpFlagResult = FALSE;
			
			if (TheCamera.IsSphereVisible(CVector(NewX, NewY, NewZ), *((float *)&ScriptParams[3])))
			{
				LatestCmpFlagResult = TRUE;
			}
			
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;

		case COMMAND_DEBUG_ON:				// Switches debugging (printing code) on
#ifndef FINAL
			CTheScripts::DbgFlag = 1;
#endif			
			return(0);		// keep going
			break;

		case COMMAND_DEBUG_OFF:				// Switches debugging (printing code) off

#ifndef FINAL
			CTheScripts::DbgFlag = 0;
#endif
			return(0);		// keep going
			break;
/*
		case COMMAND_RETURN_TRUE:				// Test function that returns 'true'

			LatestCmpFlagResult = TRUE;
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
		case COMMAND_RETURN_FALSE:				// Test function that returns 'false'

			LatestCmpFlagResult = FALSE;
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
//	COMMAND_VAR_INT
	}
	return(-1);		// Shouldn't be here. Unknown command probably.
}

Int8 CRunningScript::ProcessCommands200To299(Int32 CurrCommand)
{
//	Int32 PlayerIndex;

	Bool8 LatestCmpFlagResult;

	CVehicle *pVehicle;
	CVehicle *pVehicle2;
	CPlayerInfo *pPlayer;
	CVector TempCoors;
	CPed *pPed;
	CObject *pObj;

//	Char ZoneLabel[10];
//	Int16 ZoneIndex;
//	CZone *pZone;
//	CZone *pCurrNZone, *pCurrLNZone;

	Int16 ButtonState;

	float NewX, NewY, NewZ;
	Int32 ArrayIndex;
	
	Int32 NewStoreVehicleIndex;
	CVehicle *pOldStoreVehicle;

	switch(CurrCommand)
	{
//	200
//	COMMAND_VAR_FLOAT
//	COMMAND_LVAR_INT
//	COMMAND_LVAR_FLOAT
//	COMMAND_START_SCOPE
//	COMMAND_END_SCOPE

//	COMMAND_REPEAT
//	COMMAND_ENDREPEAT
//	COMMAND_IF
//	COMMAND_IFNOT
//	COMMAND_ELSE
//	210
//	COMMAND_ENDIF
//	COMMAND_WHILE
//	COMMAND_WHILENOT
//	COMMAND_ENDWHILE

		case COMMAND_AND_OR :

			CollectParameters(1);
			
			AndOrState = ScriptParams[0];

			if (AndOrState == NO_ANDS_OR_ORS)
			{
				CmpFlag = FALSE;		//	Doesn't really matter about resetting compare flag
										//	as it will just be overwritten with result anyway
			}
			else if ((AndOrState >= NUMBER_OF_ANDS1) && (AndOrState <= NUMBER_OF_ANDS8))
			{
				CmpFlag = TRUE;			//	If this was set to FALSE initially then ANDs would
										//	always give a result of FALSE

				AndOrState += 1;		//	Current value of AndOrState refers to the number of
										//	ANDs. Increase by one to reflect the number of expressions.
			}
			else if ((AndOrState >= NUMBER_OF_ORS1) && (AndOrState <= NUMBER_OF_ORS8))
			{
				CmpFlag = FALSE;		//	If this was set to TRUE initially then ORs would
										//	always give a result of TRUE

				AndOrState += 1;		//	Current value of AndOrState refers to the number of
										//	ORs. Increase by one to reflect the number of expressions.
			}
			else
			{
				ASSERTOBJ(0, ScriptName, "AND_OR error");
			}

			return(0);		// keep going
			break;


		case COMMAND_LAUNCH_MISSION :
/*
			//	For now LAUNCH_MISSION is treated as a GOSUB to a label within a subscript file
			ASSERT(StackDepth < MAX_STACK_DEPTH);	// Make sure stack depth is sufficient
			PC += 4;
			PCStack[StackDepth++] = PC + 4;
			PC = *((Int32 *)&CTheScripts::ScriptSpace[PC]);
*/

// Or should this StartNewScript and set its PC
//		also set IsThisAMissionScript to TRUE
			{
				CRunningScript *pNewOne;

				CollectParameters(1);
				
				pNewOne = CTheScripts::StartNewScript(&CTheScripts::ScriptSpace[ScriptParams[0]]);

				// Assume there are no arguments

			//	pNewOne->IsThisAMissionScript = TRUE;
			}
			return(0);		// keep going
			break;

		case COMMAND_MISSION_HAS_FINISHED :

			if (IsThisAMissionScript)
			{
			
#ifdef GTA_LIBERTY
				if (strcmp(ScriptName, "love3") == 0)
				{
					CPickups::RemoveAllFloatingPickUps();
				}
#endif
				CTheScripts::MissionCleanUp.Process();
			}
			return(0);		// keep going
			break;

		case COMMAND_STORE_CAR_CHAR_IS_IN :

			CollectParameters(1);

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "STORE_CAR_CHAR_IS_IN - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "STORE_CAR_CHAR_IS_IN - Check character is alive this frame");

			if (pPed->m_nPedFlags.bInVehicle)
			{
				pVehicle = pPed->m_pMyVehicle;
			}
			else
			{
				pVehicle = NULL;
			}
			
			ASSERTOBJ(pVehicle, ScriptName, "STORE_CAR_CHAR_IS_IN - Character is not in a car");

#ifndef FINAL
	//	Assume the character's car is alive in this frame
			pVehicle->m_nFlags.bCheckedForDead = TRUE;
#endif

// Character is in a vehicle
			NewStoreVehicleIndex = CPools::GetVehiclePool().GetIndex((CAutomobile *)pVehicle);
			
			if (NewStoreVehicleIndex != CTheScripts::StoreVehicleIndex)
			{
				if (IsThisAMissionScript)
				{
//	Deal with the existing Store Vehicle first
					pOldStoreVehicle = CPools::GetVehiclePool().GetAt(CTheScripts::StoreVehicleIndex);
					if (pOldStoreVehicle)
					{
						CCarCtrl::RemoveFromInterestingVehicleList(pOldStoreVehicle);
//						ASSERTOBJ(pOldStoreVehicle->VehicleCreatedBy == MISSION_VEHICLE, ScriptName, "STORE_CAR_CHAR_IS_IN - Old car is not a MISSION_VEHICLE");
//						ASSERTOBJ(pOldStoreVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "STORE_CAR_CHAR_IS_IN - Check old car is alive this frame");

						if (pOldStoreVehicle->GetVehicleCreatedBy() == MISSION_VEHICLE)
						{
							if (CTheScripts::StoreVehicleWasRandom)
							{
								pOldStoreVehicle->SetVehicleCreatedBy(RANDOM_VEHICLE);
								pOldStoreVehicle->m_nVehicleFlags.bIsLocked = FALSE;
																
								CTheScripts::MissionCleanUp.RemoveEntityFromList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
							}
						}
					}

//	Now deal with the new vehicle
					CTheScripts::StoreVehicleIndex = NewStoreVehicleIndex;

					switch (pVehicle->GetVehicleCreatedBy())
					{
						case RANDOM_VEHICLE :
						{
							pVehicle->SetVehicleCreatedBy(MISSION_VEHICLE);

							CTheScripts::StoreVehicleWasRandom = TRUE;
							
							CTheScripts::MissionCleanUp.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
						}
						break;
						
						case PARKED_VEHICLE :
						{
							pVehicle->SetVehicleCreatedBy(MISSION_VEHICLE);
														
							CTheScripts::StoreVehicleWasRandom = TRUE;
							
							CTheScripts::MissionCleanUp.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
						}
						break;
						
						case MISSION_VEHICLE :
						case PERMANENT_VEHICLE :
						{
							CTheScripts::StoreVehicleWasRandom = FALSE;
						}
						break;
						
						default :
						{
							ASSERTMSG(0, "STORE_CAR_CHAR_IS_IN - Unexpected vehicle creation type");
						}
						break;
					}
				}
				else
				{
					ASSERTOBJ(0, ScriptName, "STORE_CAR_CHAR_IS_IN : Can only be used within a mission script");
				}

			}

// Can a non-player Ped get in/out cars?
// Does ped still exist once it is inside a car?
			ScriptParams[0] = CTheScripts::StoreVehicleIndex;
			StoreParameters(1);

			return(0);		// keep going
			break;
/*
		case COMMAND_STORE_CAR_PLAYER_IS_IN :

			CollectParameters(1);

//			ASSERTOBJ(CWorld::Players[ScriptParams[0]], ScriptName, "STORE_CAR_PLAYER_IS_IN - Player doesn't exist");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed, ScriptName, "STORE_CAR_PLAYER_IS_IN - Player exists but doesn't have a character pointer");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "STORE_CAR_PLAYER_IS_IN - Check player character is alive this frame");

			if (CWorld::Players[ScriptParams[0]].pPed->m_nPedFlags.bInVehicle)
			{	//	If the player is in a vehicle then store it

				// In GTA2, had to check if there was already a different
				//	vehicle stored. If so, its creator type had to be changed
				//	back from cr_mission. The creator type of the new StoreVehicle
				//	then had to be set to cr_mission. In the mission clean up code, 
				//	StoreVehicle had to be set back to a normal car and the pointer
				//	set to NULL.

				//	In fact, the only reason for storing the car here is so that 
				//	the creator type etc. can be manipulated.
				ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle, ScriptName, "STORE_CAR_PLAYER_IS_IN - Player is not in a car 1");

				pVehicle = CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle;
				
#ifndef FINAL
	//	Assume the player's car is alive in this frame
				pVehicle->m_nFlags.bCheckedForDead = TRUE;
#endif
				
				NewStoreVehicleIndex = CPools::GetVehiclePool().GetIndex((CAutomobile *)pVehicle);
				
				if (NewStoreVehicleIndex != CTheScripts::StoreVehicleIndex)
				{
					if (IsThisAMissionScript)
					{
	//	Deal with the existing Store Vehicle first
						pOldStoreVehicle = CPools::GetVehiclePool().GetAt(CTheScripts::StoreVehicleIndex);
						if (pOldStoreVehicle)
						{
							CCarCtrl::RemoveFromInterestingVehicleList(pOldStoreVehicle);
	//						ASSERTOBJ(pOldStoreVehicle->VehicleCreatedBy == MISSION_VEHICLE, ScriptName, "STORE_CAR_PLAYER_IS_IN - Old car is not a MISSION_VEHICLE");
	//						ASSERTOBJ(pOldStoreVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "STORE_CAR_PLAYER_IS_IN - Check old car is alive this frame");

							if (pOldStoreVehicle->VehicleCreatedBy == MISSION_VEHICLE)
							{
								if (CTheScripts::StoreVehicleWasRandom)
								{
									pOldStoreVehicle->VehicleCreatedBy = RANDOM_VEHICLE;
									pOldStoreVehicle->IsLocked = FALSE;
									
									CCarCtrl::NumRandomCars++;
									CCarCtrl::NumMissionCars--;
									
									CTheScripts::MissionCleanUp.RemoveEntityFromList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
								}
							}
						}

//	Now deal with the new vehicle
						CTheScripts::StoreVehicleIndex = NewStoreVehicleIndex;

						switch (pVehicle->VehicleCreatedBy)
						{
							case RANDOM_VEHICLE :
							{
								pVehicle->SetVehicleCreatedBy(MISSION_VEHICLE);
							
								
								CTheScripts::StoreVehicleWasRandom = TRUE;
								
								CTheScripts::MissionCleanUp.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
							}
							break;
							
							case PARKED_VEHICLE :
							{
								pVehicle->VehicleCreatedBy = MISSION_VEHICLE;
								
								CCarCtrl::NumMissionCars++;
								CCarCtrl::NumParkedCars--;
								
								CTheScripts::StoreVehicleWasRandom = TRUE;
								
								CTheScripts::MissionCleanUp.AddEntityToList(CTheScripts::StoreVehicleIndex, CLEANUP_CAR);
							}
							break;
							
							case MISSION_VEHICLE :
							case PERMANENT_VEHICLE :
							{
								CTheScripts::StoreVehicleWasRandom = FALSE;
							}
							break;
							
							default :
							{
								ASSERTMSG(0, "STORE_CAR_PLAYER_IS_IN - Unexpected vehicle creation type");
							}
							break;
						}
					}
					else
					{
						ASSERTOBJ(0, ScriptName, "STORE_CAR_PLAYER_IS_IN : Can only be used within a mission script");
					}

				}

				ScriptParams[0] = CTheScripts::StoreVehicleIndex;
				StoreParameters(1);
			}
			else
			{	//	Player isn't in a vehicle
				ASSERTOBJ(0, ScriptName, "STORE_CAR_PLAYER_IS_IN - Player is not in a car 2");
			}

			return(0);		// keep going
			break;
*/
		case COMMAND_IS_CHAR_IN_CAR :

			CollectParameters(2);

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
//	How can we tell if this is a boat?
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);

// Can a non-player Ped get in/out cars?
// Does ped still exist once it is inside a car?

			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_IN_CAR - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_CAR - Check character is alive this frame");

			ASSERTOBJ(pVehicle, ScriptName, "IS_CHAR_IN_CAR - Car doesn't exist");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_CAR - Check car is alive this frame");

			if (pPed->m_nPedFlags.bInVehicle)
			{
				pVehicle2 = pPed->m_pMyVehicle;
			}
			else
			{
				pVehicle2 = NULL;
			}

			if (pVehicle2 && (pVehicle2 == pVehicle))
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
//	220
/*
		case COMMAND_IS_PLAYER_IN_CAR :

			CollectParameters(2);

//			ASSERTOBJ(CWorld::Players[ScriptParams[0]], ScriptName, "IS_PLAYER_IN_CAR - Player doesn't exist");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed, ScriptName, "IS_PLAYER_IN_CAR - Player exists but doesn't have a character pointer");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_CAR - Check player character is alive this frame");

//	How can we tell if this is a boat?
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[1]);

			ASSERTOBJ(pVehicle, ScriptName, "IS_PLAYER_IN_CAR - Car doesn't exist");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_CAR - Check car is alive this frame");

			if (CWorld::Players[ScriptParams[0]].pPed->m_nPedFlags.bInVehicle)
			{	//	If the player is in a vehicle
				ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle, ScriptName, "IS_PLAYER_IN_CAR - Player is not in a car");
				
				if (CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle == pVehicle)
				{	//	If this vehicle is the same as the one to check for
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
		case COMMAND_IS_CHAR_IN_MODEL :

			CollectParameters(2);

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);

			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_IN_MODEL - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_MODEL - Check character is alive this frame");
			
			if (pPed->m_nPedFlags.bInVehicle)
			{
				ASSERTOBJ(pPed->m_pMyVehicle, ScriptName, "IS_CHAR_IN_MODEL - Character is not in a car");
				pVehicle = pPed->m_pMyVehicle;
			}
			else
			{
				pVehicle = NULL;
			}

			// Is it ok to check the visible model rather than the physical model?
			if (pVehicle && (pVehicle->m_nModelIndex == ScriptParams[1]))
			{	// Character's vehicle is of the expected model
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

			UpdateCompareFlag(LatestCmpFlagResult);
// Can a non-player Ped get in/out cars?
// Does ped still exist once it is inside a car?

			return(0);		// keep going
			break;
/*
		case COMMAND_IS_PLAYER_IN_MODEL :

			CollectParameters(2);

//			ASSERTOBJ(CWorld::Players[ScriptParams[0]], ScriptName, "IS_PLAYER_IN_MODEL - Player doesn't exist");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed, ScriptName, "IS_PLAYER_IN_MODEL - Player exists but doesn't have a character pointer");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_MODEL - Check player character is alive this frame");

		// Is it ok to check the visible model rather than the physical model?
			if ( (CWorld::Players[ScriptParams[0]].pPed->m_nPedFlags.bInVehicle) && (CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle) )
			{	//	If the player is in a vehicle
				ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle, ScriptName, "IS_PLAYER_IN_MODEL - Player is not in a car");
			
				if (CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle->m_nModelIndex == ScriptParams[1])
				{	//	Player's vehicle is of the expected model
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
		case COMMAND_IS_CHAR_IN_ANY_CAR :

			CollectParameters(1);

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);

// Can a non-player Ped get in/out cars?
// Does ped still exist once it is inside a car?

			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_IN_ANY_CAR - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_IN_ANY_CAR - Check character is alive this frame");

			if ( (pPed->m_nPedFlags.bInVehicle) && (pPed->m_pMyVehicle) )
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
/*
		case COMMAND_IS_PLAYER_IN_ANY_CAR :

			CollectParameters(1);
			
//			ASSERTOBJ(CWorld::Players[ScriptParams[0]], ScriptName, "IS_PLAYER_IN_ANY_CAR - Player doesn't exist");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed, ScriptName, "IS_PLAYER_IN_ANY_CAR - Player exists but doesn't have a character pointer");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_ANY_CAR - Check player character is alive this frame");

			if ( (CWorld::Players[ScriptParams[0]].pPed->m_nPedFlags.bInVehicle) && (CWorld::Players[ScriptParams[0]].pPed->m_pMyVehicle) )
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
		case COMMAND_IS_BUTTON_PRESSED :
			
			CollectParameters(2);
			
			if (GetPadState(ScriptParams[0], ScriptParams[1]) && !CPad::GetPad(0)->JustOutOfFrontEnd)
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}

#ifdef GTA_PC
#ifdef GTA_LIBERTY
			//	This is a botch for the intro
			//	Pressing Space, Return and Left Mouse Button
			//	have the same effect as pressing Start on Pad1
			if (CGame::playingIntro)
			{
				if ( (ScriptParams[0] == 0) && (ScriptParams[1] == 12) )	//	0 = Pad1, 12 = Start Button
				{
	 				if ( (CPad::GetPad(0)->GetLeftMouseButtonJustDown()) ||
						 (CPad::GetPad(0)->GetReturnJustDown()) ||
						 (CPad::GetPad(0)->GetKeyJustDown(' ')) )
					{
						LatestCmpFlagResult = TRUE;
					}
				}
			}
#endif
#endif
			
			UpdateCompareFlag(LatestCmpFlagResult);
			
			return(0);		// keep going
			break;

		case COMMAND_GET_PAD_STATE :
				
			CollectParameters(2);
				
			ButtonState = GetPadState(ScriptParams[0], ScriptParams[1]);
				
			ScriptParams[0] = (Int32) ButtonState;
				
			StoreParameters(1);
				
			return(0);		// keep going
			break;

/*
		case COMMAND_IS_PLAYER_PRESSING_SQUARE :

			CollectParameters(1);

//			if (CPad::GetPad(ScriptParams[0])->GetButtonX())
			if (CPad::GetPad(ScriptParams[0])->GetButtonSquare())
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;

		case COMMAND_IS_PLAYER_PRESSING_TRIANGLE :

			CollectParameters(1);

			if (CPad::GetPad(ScriptParams[0])->GetButtonTriangle())
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;

		case COMMAND_IS_PLAYER_PRESSING_CROSS :

			CollectParameters(1);

			if (CPad::GetPad(ScriptParams[0])->GetButtonCross())
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;

		case COMMAND_IS_PLAYER_PRESSING_CIRCLE :

			CollectParameters(1);

			if (CPad::GetPad(ScriptParams[0])->GetButtonCircle())
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
			
*/
/*
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_2D :
		case COMMAND_LOCATE_PLAYER_ON_FOOT_2D :
		case COMMAND_LOCATE_PLAYER_IN_CAR_2D :
//	230
		case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_2D :
		case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_2D :
		case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_2D :
			LocatePlayerCommand(CurrCommand, &PC);
			return(0);		// keep going
			break;
*/
/*
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_2D :
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_2D :
		case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_2D :
			LocatePlayerCharCommand(CurrCommand, &PC);
			return(0);		// keep going
			break;
*/
		case COMMAND_LOCATE_CHAR_ANY_MEANS_2D :
		case COMMAND_LOCATE_CHAR_ON_FOOT_2D :
		case COMMAND_LOCATE_CHAR_IN_CAR_2D :
		case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D :
//	240
		case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D :
		case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D :
			LocateCharCommand(CurrCommand);
			return(0);		// keep going
			break;

		case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_2D :
		case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_2D :
		case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_2D :
			LocateCharCharCommand(CurrCommand);
			return(0);		// keep going
			break;
/*
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_3D :
		case COMMAND_LOCATE_PLAYER_ON_FOOT_3D :
		case COMMAND_LOCATE_PLAYER_IN_CAR_3D :
		case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D :
		case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D :
//	250
		case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D :
			LocatePlayerCommand(CurrCommand, &PC);
			return(0);		// keep going
			break;
*/
/*
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_3D :
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_3D :
		case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_3D :
			LocatePlayerCharCommand(CurrCommand, &PC);
			return(0);		// keep going
			break;
*/
		case COMMAND_LOCATE_CHAR_ANY_MEANS_3D :
		case COMMAND_LOCATE_CHAR_ON_FOOT_3D :
		case COMMAND_LOCATE_CHAR_IN_CAR_3D :
		case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D :
		case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D :
		case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D :
			LocateCharCommand(CurrCommand);
			return(0);		// keep going
			break;
//	260
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D :
		case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D :
		case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D :
			LocateCharCharCommand(CurrCommand);
			return(0);		// keep going
			break;

		case COMMAND_CREATE_OBJECT :	// This allocates an object and returns its pointer
			{
				Int32 ModelIndex;

				// Collect parameters for model type and co-ordinates
				CollectParameters(4);

				ModelIndex = *((Int32 *)&ScriptParams[0]);
				
				if (ModelIndex < 0)
				{
					ArrayIndex = -ModelIndex;
					ASSERTOBJ(ArrayIndex < CTheScripts::NumberOfUsedObjects, ScriptName, "CREATE_OBJECT - Model Index is larger than NumberOfUsedObjects");
					ModelIndex = CTheScripts::UsedObjectArray[ArrayIndex].Index;
				}

/*
				if (CModelInfo::ms_nDynamicModelDir != -1)
				{	//	Object Model Index should be between 0 and OBJECTDATA_MAXNOOFOBJECTINFOS.
					//	This is added to the base index for dynamic objects to get the
					//	real index for the dynamic object
//					&& modelIndex < ms_anDirMaxModelIndex[ms_nDynamicModelDir]
//					&& modelIndex >= ms_anDirMaxModelIndex[ms_nDynamicModelDir - 1])
					ModelIndex += CModelInfo::ms_anDirMaxModelIndex[CModelInfo::ms_nDynamicModelDir - 1];
				}
*/

				CBaseModelInfo* pModel = CModelInfo::GetModelInfo(ModelIndex);

				ASSERTOBJ(pModel, ScriptName, "CREATE_OBJECT - Couldn't find Model Info");
				ASSERTOBJ(pModel->IsDynamic(), ScriptName, "CREATE_OBJECT - Model isn't dynamic");

				// Don't fade this model in
				pModel->SetAlpha(255);

				CObject* pObject = CObject::Create(ModelIndex, false);
				ASSERTOBJ(pObject, ScriptName, "CREATE_OBJECT - Couldn't create a new object");

				if (bIsThisAStreamedScript || ScriptBrainType != -1)
				{	//	Streamed scripts and script brains shouldn't create MISSION_OBJECTs because these scripts are not stored in saved games
					//	but MISSION_OBJECTS are. This means that when the game is re-loaded the object will exist but the script won't exist
					//	so no script will have a handle to the object with which to delete it at a later date.
					//	The object would remain in the object pool forever.
					pObject->ObjectCreatedBy = MISSION_BRAIN_OBJECT;
				}
				else
				{
					pObject->ObjectCreatedBy = MISSION_OBJECT;
				}

				NewX = *((float *)&ScriptParams[1]);
				NewY = *((float *)&ScriptParams[2]);
				NewZ = *((float *)&ScriptParams[3]);
				
				if (NewZ <= -100.0f)
				{
					NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
				}

				NewZ += pObject->GetDistanceFromCentreOfMassToBaseOfModel();

				pObject->SetPosition(NewX, NewY, NewZ);
				pObject->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
				pObject->UpdateRwMatrix();
				pObject->UpdateRwFrame();
				
				// Setup big building
				if(pModel->AsLodAtomicModelInfoPtr())
					pObject->SetupBigBuilding();

				CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pObject);
				
				CWorld::Add(pObject);

				ScriptParams[0] = CPools::GetObjectPool().GetIndex(pObject);

				StoreParameters(1);

#ifndef FINAL
	//	Assume object is alive in the frame it is created in
				pObject->m_nFlags.bCheckedForDead = TRUE;
#endif

				if (IsThisAMissionScript)
				{
					CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_OBJECT);
				}
				else
				{
#ifndef FINAL
					CTheScripts::MissionCleanUp.AddEntityToNonMissionList(ScriptParams[0], CLEANUP_OBJECT, ScriptName);
#endif
				}
			}
			return(0);		// keep going
			break;

		case COMMAND_DELETE_OBJECT :

			CollectParameters(1);

			pObj = CPools::GetObjectPool().GetAt(ScriptParams[0]);
//			ASSERTOBJ(pObj, ScriptName, "DELETE_OBJECT - Object doesn't exist");

			if (pObj)
			{
				CWorld::Remove(pObj);
				CWorld::RemoveReferencesToDeletedObject(pObj);
				delete(pObj);
			}

			if (IsThisAMissionScript)
			{
				CTheScripts::MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
			}
			else
			{
#ifndef FINAL
				CTheScripts::MissionCleanUp.RemoveEntityFromNonMissionList(ScriptParams[0], CLEANUP_OBJECT, ScriptName);
#endif
			}
			return(0);		// keep going
			break;

		case COMMAND_ADD_SCORE :

			CollectParameters(2);

//			PlayerIndex = *((Int32 *)&ScriptParams[0]);
			
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "ADD_SCORE - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "ADD_SCORE - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ADD_SCORE - Check player character is alive this frame");

			pPlayer->Score += *((Int32 *)&ScriptParams[1]);

			// can now have negative score:			
/*			if (CWorld::Players[PlayerIndex].Score < 0)
			{
				CWorld::Players[PlayerIndex].Score = 0;
			}
*/
			return(0);		// keep going
			break;

		case COMMAND_IS_SCORE_GREATER :	//	COMMAND_CHECK_SCORE_GREATER :

			CollectParameters(2);
			{
				Int32 PlayerScore;

//				PlayerIndex = *((Int32 *)&ScriptParams[0]);

				pPlayer = &(CWorld::Players[ScriptParams[0]]);
				ASSERTOBJ(pPlayer, ScriptName, "IS_SCORE_GREATER - Player doesn't exist");
				ASSERTOBJ(pPlayer->pPed, ScriptName, "IS_SCORE_GREATER - Player exists but doesn't have a character pointer");
				ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_SCORE_GREATER - Check player character is alive this frame");

				PlayerScore = pPlayer->Score;

				if (PlayerScore > *((Int32 *)&ScriptParams[1]))
				{
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
				UpdateCompareFlag(LatestCmpFlagResult);
			}
			return(0);		// keep going
			break;

		case COMMAND_STORE_SCORE :

			CollectParameters(1);

//			PlayerIndex = *((Int32 *)&ScriptParams[0]);
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "STORE_SCORE - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "STORE_SCORE - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "STORE_SCORE - Check player character is alive this frame");

			ScriptParams[0] = pPlayer->Score;

			StoreParameters(1);

			return(0);		// keep going
			break;

		case COMMAND_GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER :
		
			ASSERTOBJ(0, ScriptName, "GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER - command has been removed");
/*
			CollectParameters(5);

			//	First parameter is Player ID (ignored for now)
			//	Second, third, fourth parameters are X, Y, Z for new car
			//	Fifth parameter is orientation of new car
			//	(maybe have an extra parameter for model of car?)
			
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER - Check player character is alive this frame");
			
			NewX = *((float *)&ScriptParams[1]);
			NewY = *((float *)&ScriptParams[2]);
			NewZ = *((float *)&ScriptParams[3]);
			
			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
			}
			
			CRemote::GivePlayerRemoteControlledCar(NewX, NewY, NewZ, DEGTORAD(*((float *)&ScriptParams[4])));
*/
			return(0);		// keep going
			break;

		case COMMAND_ALTER_WANTED_LEVEL :

			CollectParameters(2);

//			PlayerIndex = *((Int32 *)&ScriptParams[0]);
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "ALTER_WANTED_LEVEL - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "ALTER_WANTED_LEVEL - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ALTER_WANTED_LEVEL - Check player character is alive this frame");

			pPlayer->pPed->SetWantedLevel(*((Int32 *)&ScriptParams[1]));

			return(0);		// keep going
			break;
//	270
		case COMMAND_ALTER_WANTED_LEVEL_NO_DROP :

			CollectParameters(2);

//			PlayerIndex = *((Int32 *)&ScriptParams[0]);
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "ALTER_WANTED_LEVEL_NO_DROP - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "ALTER_WANTED_LEVEL_NO_DROP - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ALTER_WANTED_LEVEL_NO_DROP - Check player character is alive this frame");

			pPlayer->pPed->SetWantedLevelNoDrop(*((Int32 *)&ScriptParams[1]));

			return(0);		// keep going
			break;

		case COMMAND_IS_WANTED_LEVEL_GREATER : 	//	COMMAND_CHECK_HEADS_GREATER :

			CollectParameters(2);

//			PlayerIndex = *((Int32 *)&ScriptParams[0]);

			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "IS_WANTED_LEVEL_GREATER - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "IS_WANTED_LEVEL_GREATER - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_WANTED_LEVEL_GREATER - Check player character is alive this frame");

			if (FindPlayerWanted(ScriptParams[0])->m_WantedLevel > *((Int32 *)&ScriptParams[1]))
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;

		case COMMAND_CLEAR_WANTED_LEVEL :

			CollectParameters(1);

//			PlayerIndex = *((Int32 *)&ScriptParams[0]);
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "CLEAR_WANTED_LEVEL - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "CLEAR_WANTED_LEVEL - Player exists but doesn't have a character pointer");
//			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CLEAR_WANTED_LEVEL - Check player character is alive this frame");

			pPlayer->pPed->SetWantedLevel(WANTED_CLEAN);

			return(0);		// keep going
			break;

		case COMMAND_SET_DEATHARREST_STATE :

			CollectParameters(1);

			//	First parameter is on/off

			if (ScriptParams[0] == TRUE)
			{
//				CTheScripts::DeatharrestCheckEnabled = TRUE;
				DeatharrestCheckEnabled = TRUE;
			}
			else
			{
//				CTheScripts::DeatharrestCheckEnabled = FALSE;
				DeatharrestCheckEnabled = FALSE;
			}

			return(0);		// keep going
			break;

		case COMMAND_HAS_DEATHARREST_BEEN_EXECUTED : 	//	COMMAND_CHECK_DEATHARREST_EXECUTED :

			// set latest cmp flag result to TRUE if death arrest code
			//	has been executed for the current mission
//			if (CTheScripts::DoneDeatharrest)
			if (DoneDeatharrest)
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;

//		case COMMAND_GIVE_WEAPON_TO_PLAYER :
/*
		case COMMAND_ADD_AMMO_TO_PLAYER :

			CollectParameters(3);

			// First parameter is Player ID
			// Second parameter is weapon type
			// Third parameter is amount of ammo to add
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			
			ASSERTOBJ(pPlayer, ScriptName, "ADD_AMMO_TO_PLAYER - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "ADD_AMMO_TO_PLAYER - Player exists but doesn't have a character");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ADD_AMMO_TO_PLAYER - Check player character is alive this frame");

			pPlayer->pPed->GrantAmmo((eWeaponType) ScriptParams[1], ScriptParams[2]);

			return(0);		// keep going
			break;
*/
//		case COMMAND_GIVE_WEAPON_TO_CHAR :
		case COMMAND_ADD_AMMO_TO_CHAR :

			CollectParameters(3);

			// First parameter is Character ID
			// Second parameter is weapon type
			// Third parameter is amount of ammo to add
			
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			
			ASSERTOBJ(pPed, ScriptName, "ADD_AMMO_TO_CHAR - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ADD_AMMO_TO_CHAR - Check character is alive this frame");

			pPed->GrantAmmo((eWeaponType) ScriptParams[1], ScriptParams[2]);

			return(0);		// keep going
			break;

/*
//		case COMMAND_GIVE_WEAPON_TO_CAR :
		case COMMAND_ADD_AMMO_TO_CAR :		// UNFINISHED

			CollectParameters(3);

			// First parameter is Car ID
			// Second parameter is weapon type
			// Third parameter is amount of ammo to add
			
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			
			ASSERTOBJ(pVehicle, ScriptName, "ADD_AMMO_TO_CAR - Car doesn't exist");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ADD_AMMO_TO_CAR - Check vehicle is alive this frame");

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_IS_PLAYER_STILL_ALIVE :

			CollectParameters(1);

			// Parameter is Player ID
			// set latest cmp flag to TRUE if player is still alive
//			ASSERTOBJ(CWorld::Players[ScriptParams[0]], ScriptName, "IS_PLAYER_STILL_ALIVE - Player doesn't exist");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed, ScriptName, "IS_PLAYER_STILL_ALIVE - Player exists but doesn't have a character pointer");

			if (CWorld::Players[ScriptParams[0]].PlayerState == CPlayerInfo::PLAYERSTATE_HASDIED)
			{
				LatestCmpFlagResult = FALSE;
			}
			else
			{
				LatestCmpFlagResult = TRUE;
				
#ifndef FINAL
	//	Only set this in IS_PLAYER_PLAYING?
//				CWorld::Players[ScriptParams[0]].pPed->m_nFlags.bCheckedForDead = TRUE;
#endif
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
		case COMMAND_IS_PLAYER_DEAD :

			CollectParameters(1);

			// Parameter is Player ID
			// set latest cmp flag to TRUE if player is dead
//			ASSERTOBJ(CWorld::Players[ScriptParams[0]], ScriptName, "IS_PLAYER_DEAD - Player doesn't exist");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed, ScriptName, "IS_PLAYER_DEAD - Player exists but doesn't have a character pointer");

			if (CWorld::Players[ScriptParams[0]].PlayerState == CPlayerInfo::PLAYERSTATE_HASDIED)
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
				
#ifndef FINAL
	//	Only set this in IS_PLAYER_PLAYING?
//				CWorld::Players[ScriptParams[0]].pPed->m_nFlags.bCheckedForDead = TRUE;
#endif
			}
			UpdateCompareFlag(LatestCmpFlagResult);
			
			return(0);		// keep going
			break;
//	280
		case COMMAND_IS_CHAR_DEAD :

			CollectParameters(1);

			// Parameter is Character ID
			// set latest cmp flag to TRUE if character is dead

			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			
//			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_DEAD - Character doesn't exist");

			if (pPed)
			{
				if (IsPedDead(pPed))
				{
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
					
#ifndef FINAL
					pPed->m_nFlags.bCheckedForDead = TRUE;
#endif
				}
			}
			else
			{
				LatestCmpFlagResult = TRUE;
			}
			
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;

		case COMMAND_IS_CAR_DEAD :

			CollectParameters(1);

			// Parameter is Car ID
			// set latest cmp flag to TRUE if car is dead

			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
//			ASSERTOBJ(pVehicle, ScriptName, "IS_CAR_DEAD - Car doesn't exist");
			
			if (pVehicle)
			{
				if (pVehicle->GetStatus() == STATUS_WRECKED)
				{
					LatestCmpFlagResult = TRUE;
				}
				else if (pVehicle->m_nVehicleFlags.bIsDrowning)
				{
					LatestCmpFlagResult = TRUE;
#ifndef FINAL
					pVehicle->m_nFlags.bCheckedForDead = TRUE;
#endif
				}
				else
				{
					LatestCmpFlagResult = FALSE;
#ifndef FINAL
					pVehicle->m_nFlags.bCheckedForDead = TRUE;
#endif
				}
			}
			else
			{
				LatestCmpFlagResult = TRUE;
			}
			
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
/*
		case COMMAND_SET_CHAR_THREAT_SEARCH :

			CollectParameters(2);

			//	First parameter is Character ID
			//	Second parameter is threat search
			
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "SET_CHAR_THREAT_SEARCH - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CHAR_THREAT_SEARCH - Check character is alive this frame");

			pPed->m_nThreats |= ScriptParams[1];

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_SET_CHAR_THREAT_REACTION :		// UNFINISHED

			CollectParameters(2);

			// First parameter is Character ID
			// Second parameter is threat reaction


//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CHAR_THREAT_REACTION - Check character is alive this frame");

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_SET_CHAR_OBJ_NO_OBJ :
			
			CollectParameters(1);
			
			// First parameter is Character ID
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "SET_CHAR_OBJ_NO_OBJ - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CHAR_OBJ_NO_OBJ - Check character is alive this frame");

			pPed->SetScriptObjectivePassedFlag(FALSE);
//			pPed->SetObjective(NO_OBJ);
			pPed->ClearObjective();

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_ORDER_DRIVER_OUT_OF_CAR :		// UNFINISHED

			CollectParameters(1);

			// Parameter is character ID

			// Set the character's objective to leave_car

//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ORDER_DRIVER_OUT_OF_CAR - Check character is alive this frame");

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_ORDER_CHAR_TO_DRIVE_CAR :		// UNFINISHED

			CollectParameters(2);

			// First parameter is Character ID
			// Second parameter is Car ID

			// Set the character's objective to enter_car
//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ORDER_CHAR_TO_DRIVE_CAR - Check character is alive this frame");

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_ADD_PATROL_POINT :		// UNFINISHED

			CollectParameters(4);

			// First parameter is Character ID
			// next three params are X, Y, Z

			NewX = *((float*)&ScriptParams[1]);
			NewY = *((float*)&ScriptParams[2]);
			NewZ = *((float*)&ScriptParams[3]);
			
			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
			}

			// Gets a free path of MAX_PATH_LENGTH nodes if necessary
			// and adds the new node to the end of the path

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_IS_PLAYER_IN_GANG_ZONE :		// UNFINISHED

			CollectParameters(2);

			// First parameter is Player ID
			// Second parameter is Gang ID

			pPlayer = &(CWorld::Players[*((Int32 *)&ScriptParams[0])]);

			ASSERTOBJ(pPlayer, ScriptName, "IS_PLAYER_IN_GANG_ZONE - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "IS_PLAYER_IN_GANG_ZONE - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_GANG_ZONE - Check player character is alive this frame");

			TempCoors = pPlayer->GetPos();

// CGang *CTheZones::FindGang(CVector location)
//	If gang returned by the above is not NULL check if it
//	matches the gang whose ID is the second param

			// Set last cmp flag result depending on whether the 
			//	player is within the zone

			LatestCmpFlagResult = FALSE;
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_IS_PLAYER_IN_ZONE :

			CollectParameters(1);

			// First parameter is Player ID
			// second parameter is Zone Label

			pPlayer = &(CWorld::Players[*((Int32 *)&ScriptParams[0])]);

			ASSERTOBJ(pPlayer, ScriptName, "IS_PLAYER_IN_ZONE - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "IS_PLAYER_IN_ZONE - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_IN_ZONE - Check player character is alive this frame");

			ReadTextLabelFromScript(&ZoneLabel[0]);

			ASSERTOBJ(strcmp(ZoneLabel, "IND_ZON") != 0, ScriptName, "IS_PLAYER_IN_ZONE IND_ZON - Better to use IS_COLLISION_IN_MEMORY LEVEL_INDUSTRIAL");
			ASSERTOBJ(strcmp(ZoneLabel, "COM_ZON") != 0, ScriptName, "IS_PLAYER_IN_ZONE COM_ZON - Better to use IS_COLLISION_IN_MEMORY LEVEL_COMMERCIAL");
			ASSERTOBJ(strcmp(ZoneLabel, "SUB_ZON") != 0, ScriptName, "IS_PLAYER_IN_ZONE SUB_ZON - Better to use IS_COLLISION_IN_MEMORY LEVEL_SUBURBAN");

			ZoneIndex = CTheZones::FindZoneByLabelAndReturnIndex(ZoneLabel, ZONE_NAVIGATION);

			if (ZoneIndex == -1)
			{
				ASSERTOBJ(0, ScriptName, "IS_PLAYER_IN_ZONE - Couldn't find the Zone");
			}

			TempCoors = pPlayer->GetPos();

			pZone = CTheZones::GetNavigationZone(ZoneIndex);

			ASSERTOBJ(pZone, ScriptName, "IS_PLAYER_IN_ZONE - Couldn't find the Zone (2)");

			if (CTheZones::PointLiesWithinZone(&TempCoors, pZone))
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
//	290
		case COMMAND_IS_PLAYER_PRESSING_HORN :

			CollectParameters(1);
			// Parameter is Player ID

//	How can we tell if this is a boat?
//			pVehicle = CWorld::Players[ScriptParams[0]].pVehicle;
//			ASSERTOBJ(CWorld::Players[ScriptParams[0]], ScriptName, "IS_PLAYER_PRESSING_HORN - Player doesn't exist");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed, ScriptName, "IS_PLAYER_PRESSING_HORN - Player exists but doesn't have a character pointer");
			ASSERTOBJ(CWorld::Players[ScriptParams[0]].pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_PLAYER_PRESSING_HORN - Check player character is alive this frame");

//			if (CWorld::Players[ScriptParams[0]].pPed->m_nPedFlags.bInVehicle)
			if (CWorld::Players[ScriptParams[0]].pPed->GetPedState() == PED_DRIVING)
			{	//	If the player is in a vehicle
				if (CPad::GetPad(ScriptParams[0])->GetHorn())
				{	
					LatestCmpFlagResult = TRUE;
				}
				else
				{
					LatestCmpFlagResult = FALSE;
				}
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
/*
		case COMMAND_HAS_CHAR_SPOTTED_PLAYER :

			CollectParameters(2);

//	CVector vPlayerHead, vPedHead, vPedToPlayer;
//	CColPoint CollisionPoint;
//	CEntity *pCollisionEntity;

			// First parameter is Character ID
			// Second parameter is Player ID
			
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "HAS_CHAR_SPOTTED_PLAYER - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "HAS_CHAR_SPOTTED_PLAYER - Check character is alive this frame");
			
			pPlayer = &(CWorld::Players[ScriptParams[1]]);
			ASSERTOBJ(pPlayer, ScriptName, "HAS_CHAR_SPOTTED_PLAYER - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "HAS_CHAR_SPOTTED_PLAYER - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "HAS_CHAR_SPOTTED_PLAYER - Check player character is alive this frame");

			LatestCmpFlagResult = FALSE;
			
			// Check ped has a clear line of sight to the player
			// Check that the player is not too far from the ped
			// Check that the angle between the vector to the player and the vector for the ped's heading 
			//		is less than X degrees
*/
/*			
			if (pPlayer->pPed->m_nPedFlags.bInVehicle)
			{
				ASSERTOBJ(pPlayer->pPed->m_pMyVehicle, ScriptName, "HAS_CHAR_SPOTTED_PLAYER - Player is not in a car");
				pVehicle = pPlayer->pPed->m_pMyVehicle;
				vPlayerHead = pPlayer->GetPos();	//	Don't think we need to add the offset from 
													//	the car's centre point to the player's seat
			}
			else
			{
				pVehicle = NULL;
				vPlayerHead = pPlayer->GetPos();
				vPlayerHead += CVector(0.0f, 0.0f, 1.0f);	//	Add on 1 metre for distance from waist to head
			}
			
			if (pPed->m_nPedFlags.bInVehicle)
			{
				ASSERTOBJ(pPed->m_pMyVehicle, ScriptName, "HAS_CHAR_SPOTTED_PLAYER - Character is not in a car");
				vPedHead = pPed->m_pMyVehicle->GetPosition();	//	Don't think we need to add the offset from 
																//	the car's centre point to the character's seat
			}
			else
			{
				vPedHead = pPed->GetPosition();
				vPedHead += CVector(0.0f, 0.0f, 1.0f);	//	Add on 1 metre for distance from waist to head
			}
			
//			if (CWorld::ProcessLineOfSight(const CVector& vecStart, const CVector& vecEnd, CColPoint& colPoint, CEntity*& refEntityPtr, bool bCheckBuildings, bool bCheckVehicles, bool bCheckPeds, bool bCheckObjects, bool bCheckDummies, bool bSeeThroughStuff))
			if (!CWorld::ProcessLineOfSight(vPedHead, vPlayerHead, CollisionPoint, pCollisionEntity, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE))
			{
				vPedToPlayer = vPlayerHead - vPedHead;
			
				if (vPedToPlayer.Magnitude() <= 100.0f)
				{
					if (pVehicle)
					{
						if (pPed->CanSeeEntity(pVehicle))
						{
							LatestCmpFlagResult = TRUE;
						}
					}
					else
					{
						if (pPed->CanSeeEntity(pPlayer->pPed))
						{
							LatestCmpFlagResult = TRUE;
						}
					}
				}
			}
*/
/*
			if (pPed->OurPedCanSeeThisOne(pPlayer->pPed))
			{
				LatestCmpFlagResult = TRUE;
			}
				
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_ORDER_CHAR_TO_BACKDOOR :		// UNFINISHED

			CollectParameters(2);

			// First parameter is Character ID
			// Second parameter is Car ID
			
//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ORDER_CHAR_TO_BACKDOOR - Check character is alive this frame");

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_ADD_CHAR_TO_GANG :		// UNFINISHED

			CollectParameters(2);

			// First parameter is Character ID
			// Second parameter is Gang ID

//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "ADD_CHAR_TO_GANG - Check character is alive this frame");

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_IS_CHAR_OBJECTIVE_PASSED :

			CollectParameters(1);
			
			// First parameter is Char ID
			
			pPed = CPools::GetPedPool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pPed, ScriptName, "IS_CHAR_OBJECTIVE_PASSED - Character doesn't exist");
			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "IS_CHAR_OBJECTIVE_PASSED - Check character is alive this frame");

			// Set latest cmp flag result depending on data held in the 
			//	ped class
//			if (pPed->m_nPedFlags.bObjectivePassed)
			if (pPed->m_nPedFlags.bScriptObjectivePassed)
			{
				LatestCmpFlagResult = TRUE;
			}
			else
			{
				LatestCmpFlagResult = FALSE;
			}
			
			UpdateCompareFlag(LatestCmpFlagResult);

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_SET_CHAR_DRIVE_AGGRESSION :		// UNFINISHED

			// Is this the same as COMMAND_SET_CAR_DRIVING_STYLE?

			CollectParameters(2);

			// First parameter is Character ID
			// Second parameter is on/off

//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CHAR_DRIVE_AGGRESSION - Check character is alive this frame");

			return(0);		// keep going
			break;
*/
/*
		case COMMAND_SET_CHAR_MAX_DRIVESPEED :		// UNFINISHED

			// Is this the same as COMMAND_SET_CAR_CRUISE_SPEED?

			CollectParameters(2);

			// First parameter is Character ID
			// Second parameter is max drive speed (float)

//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "SET_CHAR_MAX_DRIVESPEED - Check character is alive this frame");

			return(0);		// keep going
			break;
*/
		case COMMAND_CREATE_CHAR_INSIDE_CAR :

			CollectParameters(3);

			// Parameter is Car ID
			pVehicle = CPools::GetVehiclePool().GetAt(ScriptParams[0]);
			ASSERTOBJ(pVehicle, ScriptName, "CREATE_CHAR_INSIDE_CAR - Car doesn't exist");
//	Check that the car doesn't already have a driver
			ASSERTOBJ(pVehicle->pDriver == NULL, ScriptName, "CREATE_CHAR_INSIDE_CAR - Car already has a driver");
			ASSERTOBJ(pVehicle->m_nFlags.bCheckedForDead == TRUE, ScriptName, "CREATE_CHAR_INSIDE_CAR - Check car is alive this frame");

			#ifndef FINAL
				CTheScripts::CheckThisModelHasBeenRequestedAndLoaded(ScriptParams[2]);
			#endif

			GetCorrectPedModelIndexForEmergencyServiceType(ScriptParams[1], &ScriptParams[2]);
			
			if (ScriptParams[1] == PEDTYPE_COP)
			{
				pPed = new CCopPed((eCopType) ScriptParams[2]);
			}
			else if ((ScriptParams[1] == PEDTYPE_MEDIC) || (ScriptParams[1] == PEDTYPE_FIRE))
			{
				pPed = new CEmergencyPed(ScriptParams[1], ScriptParams[2]);
			}
			else
			{
				pPed = new CCivilianPed((ePedType)ScriptParams[1], ScriptParams[2]);
			}
			
			ASSERTOBJ(pPed, ScriptName, "CREATE_CHAR_INSIDE_CAR - Couldn't create a new character");

			pPed->SetCharCreatedBy(MISSION_CHAR);
			//pPed->SetThreatReactionOverwritesObjectiveFlag(FALSE);
			pPed->m_nPedFlags.bAllowMedicsToReviveMe = FALSE;
			//pPed->m_nPedFlags.bPlayerFriend = false;

//			if(pVehicle->m_bIsBus)
//				pPed->m_nPedFlags.bRenderPedInCar = false;

			{
				ASSERTOBJ(0==pVehicle->pDriver, ScriptName, "CREATE_CHAR_INSIDE_CAR - vehicle already has a driver");
	            CTaskSimpleCarSetPedInAsDriver task(pVehicle,0);
	            task.SetIsWarpingPedIntoCar();
	          	task.ProcessPed(pPed);
          	}

			TempCoors = pVehicle->GetPosition();
/*

			pPed->SetPosition(TempCoors);	// as it will be done when the char leaves the car

			pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));

			pPed->SetPedState(PED_DRIVING);

			// Is the new character to be the driver?
			// Create character inside car
			pVehicle->pDriver = pPed;
			REGREF_NOTINWORLD(pVehicle->pDriver, (CEntity **) &(pVehicle->pDriver));
			
			pPed->m_pMyVehicle = pVehicle;
			REGREF(pPed->m_pMyVehicle, (CEntity **) &(pPed->m_pMyVehicle));
			pPed->m_nPedFlags.bInVehicle = TRUE;

			pVehicle->SetStatus(STATUS_PHYSICS);
			
			if (pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT)
			{
				pVehicle->AutoPilot.Mission = CAutoPilot::MISSION_CRUISE;	//	MISSION_NONE;
			}
			
			pVehicle->SetEngineOn(true);					// For sound
			pPed->SetUsesCollision(false);
			
			pPed->AddInCarAnims(pVehicle, TRUE);
*/
//			pPed->LivesInThisLevel = CTheZones::GetLevelFromPosition(&TempCoors);
			
			CWorld::Add(pPed);

			CPopulation::ms_nTotalMissionPeds++;

			//	and return this character's ID
			ScriptParams[0] = CPools::GetPedPool().GetIndex(pPed);

			StoreParameters(1);

#ifndef FINAL
	//	Assume the ped is alive in the frame it is created
			pPed->m_nFlags.bCheckedForDead = TRUE;
#endif


// Should only be called for mission scripts (not main script)
			if (IsThisAMissionScript)
			{
				CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
			}
			else
			{
#ifndef FINAL
				CTheScripts::MissionCleanUp.AddEntityToNonMissionList(ScriptParams[0], CLEANUP_CHAR, ScriptName);
#endif
			}
			return(0);		// keep going
			break;
/*
		case COMMAND_WARP_PLAYER_FROM_CAR_TO_COORD :

			CollectParameters(4);

			// First parameter is Player ID
			// next three parameters are X, Y, Z
			
			pPlayer = &(CWorld::Players[ScriptParams[0]]);
			ASSERTOBJ(pPlayer, ScriptName, "WARP_PLAYER_FROM_CAR_TO_COORD - Player doesn't exist");
			ASSERTOBJ(pPlayer->pPed, ScriptName, "WARP_PLAYER_FROM_CAR_TO_COORD - Player exists but doesn't have a character pointer");
			ASSERTOBJ(pPlayer->pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "WARP_PLAYER_FROM_CAR_TO_COORD - Check player character is alive this frame");

			NewX = *((float*)&ScriptParams[1]);
			NewY = *((float*)&ScriptParams[2]);
			NewZ = *((float*)&ScriptParams[3]);
			
			if (NewZ <= -100.0f)
			{
				NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
			}
			
			ASSERTOBJ(pPlayer->pPed->m_nPedFlags.bInVehicle, ScriptName, "WARP_PLAYER_FROM_CAR_TO_COORD - Player is not in a vehicle");
			
			if ( (pPlayer->pPed->m_nPedFlags.bInVehicle) && (pPlayer->pPed->m_pMyVehicle) )
			{
				ASSERTOBJ(pPlayer->pPed->m_pMyVehicle, ScriptName, "WARP_PLAYER_FROM_CAR_TO_COORD - Player is not in a vehicle (2)");

				if(pPlayer->pPed->m_pMyVehicle->m_bIsBus)
				{
					pPlayer->pPed->m_nPedFlags.bRenderPedInCar = TRUE;
				}	
			
				if (pPlayer->pPed->m_pMyVehicle->pDriver == pPlayer->pPed)
				{
					pPlayer->pPed->m_pMyVehicle->RemoveDriver();
					pPlayer->pPed->m_pMyVehicle->SetStatus(STATUS_ABANDONED);
					pPlayer->pPed->m_pMyVehicle->EngineOn = FALSE;
					pPlayer->pPed->m_pMyVehicle->AutoPilot.CruiseSpeed = 0; // slow this car down as we get out
				}
				else
				{
					pPlayer->pPed->m_pMyVehicle->RemovePassenger(pPlayer->pPed);
				}
			}

			pPlayer->pPed->RemoveInCarAnims(true);
			pPlayer->pPed->m_nPedFlags.bInVehicle = FALSE;
			pPlayer->pPed->m_pMyVehicle = NULL;		// Maybe shouldn't clear m_pMyVehicle
			pPlayer->pPed->SetPedState(PED_IDLE);
			pPlayer->pPed->SetUsesCollision(TRUE);
			pPlayer->pPed->SetMoveSpeed(CVector(0.0f, 0.0f, 0.0f));
//			pPlayer->pPed->AddWeaponModel(CWeaponInfo::GetWeaponInfo(pPlayer->pPed->GetWeapon()->GetWeaponType())->GetModelId());
			pPlayer->pPed->ReplaceWeaponWhenExitingVehicle();
			
			if (pPlayer->pPed->m_pAnim)
			{
				pPlayer->pPed->m_pAnim->SetBlendDelta(-1000.0f);
			}

			pPlayer->pPed->m_pAnim = NULL;
//			pPlayer->pPed->ApplyMoveSpeed();	
//			m_nPedStoredState = PED_IDLE;
//			m_eStoredMoveState = PEDMOVE_STILL; // don't have a stored state, so this will do
//			SetMoveState(PEDMOVE_STILL);
			pPlayer->pPed->SetMoveState(PEDMOVE_NONE);
			CAnimManager::BlendAnimation((RpClump*)pPlayer->pPed->m_pRwObject, pPlayer->pPed->m_motionAnimGroup, ANIM_STD_IDLE, 1000.0f);

			pPlayer->pPed->RestartNonPartialAnims();
			
			AudioManager.PlayerJustLeftCar();

			NewZ += pPlayer->pPed->GetDistanceFromCentreOfMassToBaseOfModel();

			pPlayer->pPed->Teleport(CVector(NewX, NewY, NewZ));
			
			CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pPlayer->pPed);
*/
/*
This code has been copied from
void CPed::SetExitCar(CVehicle *pVehicle, uint32 nDoor)
void CPed::PedSetOutCarCB(CAnimBlendAssociation* pAnim, void* pData)
*/
/*
			return(0);		// keep going
			break;
*/
/*
		case COMMAND_MAKE_CHAR_DO_NOTHING :	//	Same as COMMAND_CHAR_SET_IDLE?		// UNFINISHED

			CollectParameters(1);

			// Parameter is Character ID

			// Set character objective to do_nothing

//			ASSERTOBJ(pPed->m_nFlags.bCheckedForDead == TRUE, ScriptName, "MAKE_CHAR_DO_NOTHING - Check character is alive this frame");

			return(0);
			break;
*/
	}
	return(-1);		// Shouldn't be here. Unknown command probably.
}


void CRunningScript::SetCharCoordinates(CPed* pPed, float NewX,float NewY, float NewZ, bool bWarpGang, bool bOffset/*=true*/)
{
	CVehicle* pVehicle;
	if (pPed->m_nPedFlags.bInVehicle)
	{
		pVehicle = pPed->m_pMyVehicle;
	}
	else
	{
		pVehicle = NULL;
	}

	if (NewZ <= -100.0f)
	{
		NewZ = CWorld::FindGroundZForCoord(NewX, NewY);
	}

	if (pVehicle)
	{
		// When a car is teleported, its suspension must be reset so that the force of the springs does not
		//	create a large Z speed
		//	NewZ += 1.0f;	// Safety margin
		NewZ += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();

		if (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BOAT)
		{
			((CBoat*)pVehicle)->Teleport(CVector(NewX, NewY, NewZ), false);
		}
		else if (pVehicle->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
		{
			((CBike*)pVehicle)->Teleport(CVector(NewX, NewY, NewZ), false);
		}
		else
		{
			((CAutomobile*)pVehicle)->Teleport(CVector(NewX, NewY, NewZ), false);
		}
		
		CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pVehicle);
	}
	else
	{
		if(bOffset)
		{
			NewZ += pPed->GetDistanceFromCentreOfMassToBaseOfModel();
		}		
		//Clear some space for pPed.
		CTheScripts::ClearSpaceForMissionEntity(CVector(NewX, NewY, NewZ), pPed);
		
		CPedGroup* pPedGroup=CPedGroups::GetPedsGroup(pPed);
		if(pPedGroup && pPedGroup->GetGroupMembership()->IsLeader(pPed) && bWarpGang)
		{
			//Ped is leader of the group so teleport the whole group.
			pPedGroup->Teleport(CVector(NewX, NewY, NewZ));
		}
		else
		{
			//Ped isn't leader of any group so just teleport the ped.
			pPed->Teleport(CVector(NewX, NewY, NewZ));
		}
	}
}

void CRunningScript::GetCorrectPedModelIndexForEmergencyServiceType(Int32 PedType, Int32 *pPedModelIndex)
{
	//	For Cop, Fire, Medic Types - the ped model index is converted into a subtype
	
	switch (*pPedModelIndex)
	{
		case MODELID_COP_LA_PED :
		case MODELID_COP_SF_PED :
		case MODELID_COP_LV_PED :
		case MODELID_COP_MC_PED :
			if (PedType == PEDTYPE_COP)
			{
				*pPedModelIndex = COPTYPE_NORMAL;
			}
			break;
			
		case MODELID_SWAT_PED :
			if (PedType == PEDTYPE_COP)
			{
				*pPedModelIndex = COPTYPE_SWAT;
			}
			break;
			
		case MODELID_FBI_PED :
			if (PedType == PEDTYPE_COP)
			{
				*pPedModelIndex = COPTYPE_FBI;
			}
			break;
			
		case MODELID_ARMY_PED :
			if (PedType == PEDTYPE_COP)
			{
				*pPedModelIndex = COPTYPE_ARMY;
			}
			break;
			
		case MODELID_COP_RURAL_PED:
			if (PedType == PEDTYPE_COP)
			{
				*pPedModelIndex = COPTYPE_RURAL;
			}
			break;
	}
}

bool CTheScripts::CheckScriptVersion(int32 nFileID, char* strFileName)
{
#ifdef USE_SCRIPT_VERSION_INFO
	ASSERTMSG(nFileID > 0, "CTheScripts::CheckScriptVersion() : Invalid File ID");
	
	CVersionInfo verInfo;
	CFileMgr::Read(nFileID, (char *) verInfo.GetpVersion(), verInfo.GetSize());
	int32 nDiff = verInfo.IsCurrent();
	if(nDiff != 0)
	{
		char strErr[1024];
		if(nDiff < 0)
		{
			if( ((int16)verInfo.GetpVersion()->m_nID) == 0x02)
			{
				sprintf(strErr, "Script may have no Version Info  %s : Get Latest GTA.EXE", verInfo.GetVersionNumber(), strFileName);
			}
			else
			{
				sprintf(strErr, "Incorrect Script Version (%d) in %s : Get Latest GTA.EXE", verInfo.GetVersionNumber(), strFileName);
			}
		}
		else
		{
			sprintf(strErr, "Incorrect Script Version (%d) in %s : Get Latest SC.EXE and recompile scripts", verInfo.GetVersionNumber(), strFileName);
		}
		ASSERTMSG(false, strErr);
		return false;
	}
#endif
	return true;
}

bool CTheScripts::CheckStreamedScriptVersion(RwStream* pStream , char* strFileName)
{
#ifdef USE_SCRIPT_VERSION_INFO_IN_STREAMED_SCRIPTS
	ASSERTMSG(pStream, "CTheScripts::CheckStreamedScriptVersion() : Stream is NULL");
	
	CVersionInfo verInfo;
	RwStreamRead(pStream, (char *) verInfo.GetpVersion(), verInfo.GetSize());

	int32 nDiff = verInfo.IsCurrent();
	if(nDiff != 0)
	{
		char strErr[1024];
		if(nDiff < 0)
		{
			if( ((int16)verInfo.GetpVersion()->m_nID) == 0x02)
			{
				sprintf(strErr, "Streamed Script may have no Version Info  %s : Get Latest GTA.EXE", verInfo.GetVersionNumber(), strFileName);
			}
			else
			{
				sprintf(strErr, "Incorrect Streamed Script Version (%d) in %s : Get Latest GTA.EXE", verInfo.GetVersionNumber(), strFileName);
			}
		}
		else
		{
			sprintf(strErr, "Incorrect Streamed Script Version (%d) in %s : Get Latest SC.EXE and recompile scripts", verInfo.GetVersionNumber(), strFileName);
		}
		ASSERTMSG(false, strErr);
		return false;
	}
#endif
	return true;
}

void CMissionCleanup::DoFadeScrewUpCheck()
{
	CPlayerInfo* pPlayer = &(CWorld::Players[0]);
	
	// Check if fading
//	if(TheCamera.GetFadingDirection()==CCamera::FADE_OUT)
	if (CTheScripts::bScriptHasFadedOut)
	{
		CTheScripts::bScriptHasFadedOut = false;
		// Check Player hasnt been arrested OR Hasn't died
		if((pPlayer->PlayerState != CPlayerInfo::PLAYERSTATE_HASBEENARRESTED) && (pPlayer->PlayerState != CPlayerInfo::PLAYERSTATE_HASDIED))
		{
			TheCamera.Fade(0.5f, CCamera::FADE_IN);
			CPad::GetPad(0)->EnableControlsScript();
		}
	}
}


#ifndef MASTER
void CTheScripts::DisplayKatieDoorDebugInfo(void)
{
	CVector vecDifference;
	CObject *pObj = NULL;
	CObjectPool& pObjPool = CPools::GetObjectPool();
	int32 nPoolSize = pObjPool.GetSize();
	
	float	Distance;
	float	ClosestDistance = 99999.9f;
	CObject *pClosestObj = NULL;
	int32 i = 0;
	
	if (FindPlayerPed(0))
	{
		const CVector &PlayerPos = FindPlayerCentreOfWorld(0);
		
		for(i=0; i<nPoolSize; i++)
		{
			pObj = pObjPool.GetSlot(i);

			if(pObj)
			{			
				if(pObj->m_nPhysicalFlags.bDoorPhysics)
				{
					vecDifference = PlayerPos - pObj->GetPosition();
					Distance = vecDifference.Magnitude();
					
					if (Distance < ClosestDistance)
					{
						ClosestDistance = Distance;
						pClosestObj = pObj;
					}
				}
			}
		}

		if (pClosestObj)
		{
			sprintf(gString, "Door Position %f %f %f", pClosestObj->GetPosition().x, pClosestObj->GetPosition().y, pClosestObj->GetPosition().z);
			VarConsole.AddDebugOutput(gString);
			
			sprintf(gString, "Door Turn Speed %f %f %f", pClosestObj->GetTurnSpeed().x, pClosestObj->GetTurnSpeed().y, pClosestObj->GetTurnSpeed().z);
			VarConsole.AddDebugOutput(gString);
			
			if (pClosestObj->GetIsStatic())
			{
				sprintf(gString, "Door Is Static");
			}
			else
			{
				sprintf(gString, "Door Isn't Static");
			}
			VarConsole.AddDebugOutput(gString);
			
			if (pClosestObj->GetIsStuck())
			{
				sprintf(gString, "Door Is Stuck");
			}
			else
			{
				sprintf(gString, "Door Isn't Stuck");
			}
			VarConsole.AddDebugOutput(gString);
			
			if (pClosestObj->m_nPhysicalFlags.bInfiniteMassFixed)
			{
				sprintf(gString, "Door InfiniteMassFixed is TRUE");
			}
			else
			{
				sprintf(gString, "Door InfiniteMassFixed is FALSE");
			}
			VarConsole.AddDebugOutput(gString);

			if (pClosestObj->m_nPhysicalFlags.bInfiniteMass)
			{
				sprintf(gString, "Door InfiniteMass is TRUE");
			}
			else
			{
				sprintf(gString, "Door InfiniteMass is FALSE");
			}
			VarConsole.AddDebugOutput(gString);
			
			sprintf(gString, "Door Original Angle %f", pClosestObj->m_fOriginalAngle);
			VarConsole.AddDebugOutput(gString);
			
			sprintf(gString, "Door Move Speed %f %f %f", pClosestObj->m_vecMoveSpeed.x, pClosestObj->m_vecMoveSpeed.y, pClosestObj->m_vecMoveSpeed.z);
			VarConsole.AddDebugOutput(gString);

			sprintf(gString, "Door Turn Friction %f %f %f", pClosestObj->m_vecTurnFriction.x, pClosestObj->m_vecTurnFriction.y, pClosestObj->m_vecTurnFriction.z);
			VarConsole.AddDebugOutput(gString);

	
			if (pClosestObj->m_nPhysicalFlags.bDoorHitEndStop)
			{
				sprintf(gString, "Door HitEndStop is TRUE");
			}
			else
			{
				sprintf(gString, "Door HitEndStop is FALSE");
			}
			VarConsole.AddDebugOutput(gString);
			
			if (pClosestObj->m_nObjectFlags.bWasDoorLocked)
			{
				sprintf(gString, "Door bWasDoorLocked is TRUE");
			}
			else
			{
				sprintf(gString, "Door bWasDoorLocked is FALSE");
			}
			VarConsole.AddDebugOutput(gString);
		}
	}
}
#endif
