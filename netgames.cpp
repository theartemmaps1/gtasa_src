/////////////////////////////////////////////////////////////////////////////////
//
// FILE :    netgames.cpp
// PURPOSE : Game logic. (Ctf, cops&robbers etc) is all done here
// AUTHOR :  Obbe.
// CREATED : 26/11/01
//
/////////////////////////////////////////////////////////////////////////////////

#ifdef GTA_NETWORK


#include "NetGames.h"
#include "Network.h"
#include "GameNet.h"
#include "Netmessages.h"
#include "main.h"
#include "general.h"
#include "coronas.h"
#include "radar.h"
#include "pickups.h"
#include "modelindices.h"
#include "font.h"
#include "camera.h"
#include "sprite.h"

CVector CNetGames::PickUpCoors;
Int32	CNetGames::CurrentPickUp;
Int32	CNetGames::TargetBlipIndex;
Int32	CNetGames::Base0BlipIndex;
Int32	CNetGames::Base1BlipIndex;
Int32	CNetGames::GameType;
Int32	CNetGames::GameMap;
Int32	CNetGames::TeamPoints[NUM_TEAMS];
Int32	CNetGames::NumCashGenerators;
Int32	CNetGames::NumStashLocations;
Int32	CNetGames::CurrentCashGenerator;
Int32	CNetGames::CurrentStashLocations[NUM_TEAMS];
Int32	CNetGames::GameState;
Int32	CNetGames::PlayerCarryingThisFlag[NUM_TEAMS];
CVector	CNetGames::FlagCoordinates[NUM_TEAMS];
Int32	CNetGames::FlagBlipIndex[NUM_TEAMS];
CNodeAddress	CNetGames::CurrentRatRaceNode;
UInt8	CNetGames::RatRaceDirection;
float	CNetGames::CashCoorsX;
float	CNetGames::CashCoorsY;
float	CNetGames::CashCoorsZ;
CVehicle *CNetGames::pCashVehicle;
Int32	CNetGames::NumStartPoints;		// The number of start points registered for this map
Int32	CNetGames::NumCTFBases;
Int32	CNetGames::NumStoredPickUps;
Int32	CNetGames::DominationBases[DOMINATIONBASES];
Int32	CNetGames::DominationBaseBlipIndex[DOMINATIONBASES];
Int32	CNetGames::TeamDominatingBases[DOMINATIONBASES];


UInt32	PreviousTimeRatRace;



// Player creation coordinates.
// In the network games the script will set the values here so that the game knows
// where to spawn/regenerate players.
float NetWorkStartCoors[MAX_START_POINTS][3] =
{
	1034.9f, -940.0f, 15.0f,
	1035.8f, -930.0f, 15.0f,
	1036.1f, -911.4f, 15.0f,
	1038.0f, -899.0f, 15.0f,
	1062.5f, -902.8f, 15.0f,
	1061.6f, -911.5f, 15.0f,
	1062.4f, -923.5f, 15.0f,
	1062.5f, -938.0f, 15.0f,
};


// Team0 red, Team0 green, Team0 blue, Team1 red etc
UInt32	TeamColours[] = {255, 128, 0, 0, 100, 255 };

// Colour of the bases that go with these teams
UInt32	BaseColours[] = {200, 100, 0, 0, 80, 200 };


// Colour of the radar blip for the cash
#define CASHBLIPCOLOURR (255)
#define CASHBLIPCOLOURG (220)
#define CASHBLIPCOLOURB (180)

float CashGenerators[MAX_NUM_CASH_GENERATORS * 3];
float StashLocations[MAX_NUM_STASH_LOCATIONS * 3];
float CTFBases[MAX_NUM_CTF_BASES * 3];
float StoredPickUpCoors[MAX_NUM_STORED_PICKUPS * 3];
Int32 StoredPickUpMI[MAX_NUM_STORED_PICKUPS];


///////////////////////////////////////////////////////////////////////////
//
// Init
// Called at start of game.
//
//////////////////////////////////////////////////////////////////////////

void CNetGames::Init(void)
{
	Int32	C;

	CurrentPickUp = 0;
	TargetBlipIndex = 0;
	Base0BlipIndex = 0;
	Base1BlipIndex = 0;
	NumCashGenerators = 0;
	NumStashLocations = 0;
	GameState = 0;
	NumStartPoints = 0;
	NumCTFBases = 0;
	NumStoredPickUps = 0;
	for (C = 0; C < DOMINATIONBASES; C++) DominationBases[C] = 0;
	for (C = 0; C < DOMINATIONBASES; C++) DominationBaseBlipIndex[C] = 0;
	for (C = 0; C < DOMINATIONBASES; C++) TeamDominatingBases[C] = -1;
	
//	PickUpCoors = CVector( NetWorkPickUpCoors[CurrentPickUp][0], NetWorkPickUpCoors[CurrentPickUp][1], NetWorkPickUpCoors[CurrentPickUp][2] );
	PickUpCoors = CVector( 0.0f, 0.0f, 0.0f );

	for (C = 0; C < NUM_TEAMS; C++)
	{
		PlayerCarryingThisFlag[C] = -1;
	}
};

///////////////////////////////////////////////////////////////////////////
//
// Init_ServerAtBeginOfGame
// Called at start of game.
//
//////////////////////////////////////////////////////////////////////////

void CNetGames::Init_ServerAtBeginOfGame(void)
{

//	if (!GameMap)		// Map 0 has the full map in it and we generate some test objects(pickups/spawn points) for it.
//	{

//		switch (GameType)
//		{
//			case GAMETYPE_DEATHMATCH:
//			case GAMETYPE_DEATHMATCH_NOBLIPS:
//			case GAMETYPE_TEAMDEATHMATCH:
//			case GAMETYPE_TEAMDEATHMATCH_NOBLIPS:
//			case GAMETYPE_CAPTURETHEFLAG:
//			case GAMETYPE_DOMINATION:

	CPickups::GenerateNewOne(CVector (1442.0f, -1603.0f, 14.0f), MODELID_WEAPON_BRASSKNUCKLE, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1520.0f, -1602.0f, 14.0f), MODELID_WEAPON_MOLOTOV, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1512.0f, -1663.0f, 10.2f), MODELID_WEAPON_BAT, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1511.0f, -1675.0f, 14.0f), MODELID_WEAPON_COLT45, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1522.0f, -1720.0f, 14.0f), MODELID_WEAPON_SHOTGUN, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1502.0f, -1674.0f, 14.0f), MODELID_WEAPON_TEC9, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1480.0f, -1750.0f, 15.5f), MODELID_WEAPON_RUGER, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1438.0f, -1718.0f, 14.0f), MODELID_WEAPON_SNIPER, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1451.0f, -1653.0f, 14.0f), MODELID_WEAPON_FLAME, CPickup::PICKUP_ON_STREET, 0);

				// Also some health
	CPickups::GenerateNewOne(CVector (1446.0f, -1660.7f, 10.2f), MI_PICKUP_HEALTH, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1464.0f, -1629.0f, 14.0f), MI_PICKUP_HEALTH, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1505.0f, -1650.0f, 14.0f), MI_PICKUP_HEALTH, CPickup::PICKUP_ON_STREET, 0);
	
				// And some armour
	CPickups::GenerateNewOne(CVector (1498.0f, -1671.0f, 14.0f), MI_PICKUP_BODYARMOUR, CPickup::PICKUP_ON_STREET, 0);
	CPickups::GenerateNewOne(CVector (1470.0f, -1692.0f, 14.0f), MI_PICKUP_BODYARMOUR, CPickup::PICKUP_ON_STREET, 0);
//				break;
//		}

/*		
		// This stuff should come from the map eventually
		AddNetworkObject(1053.0f, -947.0f, 15.0f, NOBJ_CASHGENERATOR);
		AddNetworkObject(903.7f, -817.0f, 14.7f, NOBJ_CASHGENERATOR);
		AddNetworkObject(819.0f, -662.0f, 14.7f, NOBJ_CASHGENERATOR);
		AddNetworkObject(822.0f, -152.6f, -5.0f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1124.0f, -130.0f, 9.7f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1290.0f, -35.0f, 14.0f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1339.0f, -220.0f, 46.5f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1371.0f, -360.0f, 49.8f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1197.0f, -422.7f, 24.8f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1127.0f, -393.0f, 19.8f, NOBJ_CASHGENERATOR);
		AddNetworkObject(986.7f, -470.0f, 14.8f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1129.0f, -1098.4f, 11.5f, NOBJ_CASHGENERATOR);
		AddNetworkObject(1353.0f, -836.0f, 14.7f, NOBJ_CASHGENERATOR);
	}
	else
	{
		// Put these in so that the game doesn't crash. Should be removed ASAP
		
//		AddNetworkObject(1053.0f, -947.0f, 15.0f, NOBJ_CASHGENERATOR);
//		AddNetworkObject(903.7f, -817.0f, 14.7f, NOBJ_CASHGENERATOR);

		for (Int32 C = 0; C < NumStoredPickUps; C++)
		{
			if (StoredPickUpMI[C] == MI_PICKUP_HEALTH)
			{
				CPickups::GenerateNewOne(CVector (StoredPickUpCoors[C * 3], StoredPickUpCoors[C * 3 + 1], StoredPickUpCoors[C * 3 + 2]), MI_PICKUP_HEALTH, CPickup::PICKUP_ON_STREET_SLOW, 0);
			}
			else if (StoredPickUpMI[C] == MI_PICKUP_BODYARMOUR)
			{
				CPickups::GenerateNewOne(CVector (StoredPickUpCoors[C * 3], StoredPickUpCoors[C * 3 + 1], StoredPickUpCoors[C * 3 + 2]), MI_PICKUP_BODYARMOUR, CPickup::PICKUP_ON_STREET_SLOW, 0);
			}
			else if (	StoredPickUpMI[C] == MODELID_WEAPON_GRENADE ||
//						StoredPickUpMI[C] == MODELID_WEAPON_AK47 ||
						StoredPickUpMI[C] == MODELID_WEAPON_BAT ||
						StoredPickUpMI[C] == MODELID_WEAPON_COLT45 ||
						StoredPickUpMI[C] == MODELID_WEAPON_MOLOTOV ||
						StoredPickUpMI[C] == MODELID_WEAPON_ROCKETLAUNCHER ||
						StoredPickUpMI[C] == MODELID_WEAPON_SHOTGUN ||
						StoredPickUpMI[C] == MODELID_WEAPON_SNIPER ||
						StoredPickUpMI[C] == MODELID_WEAPON_UZI ||
//						StoredPickUpMI[C] == MODELID_WEAPON_M16 ||
						StoredPickUpMI[C] == MODELID_WEAPON_FLAME)
			{
				CPickups::GenerateNewOne(CVector (StoredPickUpCoors[C * 3], StoredPickUpCoors[C * 3 + 1], StoredPickUpCoors[C * 3 + 2]), StoredPickUpMI[C], CPickup::PICKUP_ON_STREET, 0);					
			}
			else
			{
				ASSERT(0);		// Unknown pickup type (or AddNetworkObject called with funny parameter)
			}
		}
	}
*/
	CNetGames::Init_ClientAtBeginOfGame();

	CWorld::Players[0].Team = CNetGames::FindTeamForNewPlayer(0);

}


///////////////////////////////////////////////////////////////////////////
//
// Init_ClientAtBeginOfGame
// Called at start of game.
//
//////////////////////////////////////////////////////////////////////////

void CNetGames::Init_ClientAtBeginOfGame(void)
{
	CNetGames::ResetScores();

/*
	if (!GameMap)		// Map 0 has the full map in it and we generate some test objects(pickups/spawn points) for it.
	{
		AddNetworkObject(1488.0f, -680.0f, 11.7f, NOBJ_STASHLOCATION);
		AddNetworkObject(1375.5f, -989.5f, 11.8f, NOBJ_STASHLOCATION);
		AddNetworkObject(1204.0f, -795.5f, 14.6f, NOBJ_STASHLOCATION);
		AddNetworkObject(1022.0f, -1103.0f, 13.0f, NOBJ_STASHLOCATION);
		AddNetworkObject(835.0f, -1089.0f, 6.7f, NOBJ_STASHLOCATION);
		AddNetworkObject(1091.0f, -695.6f, 14.9f, NOBJ_STASHLOCATION);
		AddNetworkObject(884.3f, -523.0f, 16.5f, NOBJ_STASHLOCATION);
		AddNetworkObject(880.0f, -312.0f, 8.6f, NOBJ_STASHLOCATION);
		AddNetworkObject(934.0f, -212.0f, 4.8f, NOBJ_STASHLOCATION);
		AddNetworkObject(1141.0f, 51.0f, 0.0f, NOBJ_STASHLOCATION);
		AddNetworkObject(1177.0f, -96.4f, 7.4f, NOBJ_STASHLOCATION);
		AddNetworkObject(1180.0f, -185.0f, 14.8f, NOBJ_STASHLOCATION);
		AddNetworkObject(1418.0f, -165.7f, 51.1f, NOBJ_STASHLOCATION);
		AddNetworkObject(1160.0f, -310.0f, 22.6f, NOBJ_STASHLOCATION);

		NumStartPoints = 8;	// 8 points have been set up for the full map
	}
	else
	{		// Just to stop the game from crashing.
//		AddNetworkObject(1488.0f, -680.0f, 11.7f, NOBJ_STASHLOCATION);
//		AddNetworkObject(1375.5f, -989.5f, 11.8f, NOBJ_STASHLOCATION);
	}
*/

	// Make sure a subset of the weapons are streamed in (the ones from the basic cheat)
	CStreaming::RequestModel(MODELID_WEAPON_BRASSKNUCKLE, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_BAT, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_MOLOTOV, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_COLT45, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_SHOTGUN, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_TEC9, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_RUGER, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_SNIPER, STRFLAG_DONTDELETE);
	CStreaming::RequestModel(MODELID_WEAPON_FLAME, STRFLAG_DONTDELETE);

//	CStreaming::LoadAllRequestedModels();

}


///////////////////////////////////////////////////////////////////////////
//
// Update
// Called once a frame.
//
//////////////////////////////////////////////////////////////////////////

void CNetGames::Update(void)
{
	Int32	Player, Base;
	CVector	Coors;
	float	Distance;

	if (CGameNet::NetWorkStatus == NETSTAT_SINGLEPLAYER) return;

	switch (GameType)
	{
		case GAMETYPE_DEATHMATCH:
			break;

		case GAMETYPE_DEATHMATCH_NOBLIPS:
			break;
			
		case GAMETYPE_TEAMDEATHMATCH:
			break;

		case GAMETYPE_TEAMDEATHMATCH_NOBLIPS:
			break;

		case GAMETYPE_STASHTHECASH:
			if (CGameNet::NetWorkStatus == NETSTAT_SERVER)
			{
				switch (GameState)
				{
					case 0:
							// Find new coordinates for cash & stash
						ASSERT(NumCashGenerators >= 2);
						ASSERT(NumStashLocations >= 2);
						Int32	Old = CurrentCashGenerator;
						while (Old == CurrentCashGenerator)
						{
							CurrentCashGenerator = CGeneral::GetRandomNumber() % NumCashGenerators;
						}
						CurrentStashLocations[0] = CGeneral::GetRandomNumber() % NumStashLocations;
						CurrentStashLocations[1] = CGeneral::GetRandomNumber() % NumStashLocations;
						while (CurrentStashLocations[0] == CurrentStashLocations[1])
						{
							CurrentStashLocations[1] = CGeneral::GetRandomNumber() % NumStashLocations;
						}
						CashCoorsX = CashGenerators[CurrentCashGenerator * 3];
						CashCoorsY = CashGenerators[CurrentCashGenerator * 3 + 1];
						CashCoorsZ = CashGenerators[CurrentCashGenerator * 3 + 2];
						
						pCashVehicle = NULL;
						GameState = 1;
						break;
						
					case 1:
						// GameOn.
						if (pCashVehicle)	// Somebody is carrying the cash
						{
							// Test whether the cash has been stashed
							// First find the player that own this vehicle
							Int32 Player = CWorld::FindPlayerForCar(pCashVehicle);
							
							if (Player >= 0)
							{
							
								if ((pCashVehicle->GetPosition() - CVector(StashLocations[CNetGames::CurrentStashLocations[CWorld::Players[Player].Team] * 3], StashLocations[CNetGames::CurrentStashLocations[CWorld::Players[Player].Team] * 3 + 1], StashLocations[CNetGames::CurrentStashLocations[CWorld::Players[Player].Team] * 3 + 2]) ).Magnitude() < 7.0f)
								{		// Give the man a point.
									TeamPoints[CWorld::Players[Player].Team]++;
									CWorld::Players[Player].Points++;
									GameState = 0;
									pCashVehicle = NULL;
									CashCoorsZ += 100.0f;	// Move cash out of the way so that it doesn't get picked up before it's moved

									CMsgText	Msg;
									Int32		R, G, B;
									FindPlayerMarkerColour(Player, &R, &G, &B);
									sprintf(Msg.String, "%s stashed the cash", CWorld::Players[Player].Name);
									Msg.ColourR = R;
									Msg.ColourG = G;
									Msg.ColourB = B;
									CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 100 );
									Msg.Get();	// print on our own screen as well
								}
							}
						}
						
							// Test whether the cash is picked up by someone
						if (!pCashVehicle)
						{
							for (Player = 0; Player < MAX_NUM_PLAYERS; Player++)
							{
								if (CWorld::Players[Player].bInUse &&
									CWorld::Players[Player].pPed &&
									CWorld::Players[Player].pPed->m_bInVehicle &&
									CWorld::Players[Player].pPed->m_pMyVehicle &&
									(CWorld::Players[Player].pPed->m_pMyVehicle->GetPosition() - CVector(CashCoorsX, CashCoorsY, CashCoorsZ) ).Magnitude() < 5.0f)
								{		// Thing has been picked up
									pCashVehicle = CWorld::Players[Player].pPed->m_pMyVehicle;

									CMsgText	Msg;
									Int32		R, G, B;
									FindPlayerMarkerColour(Player, &R, &G, &B);
									sprintf(Msg.String, "%s collected the cash", CWorld::Players[Player].Name);
									Msg.ColourR = R;
									Msg.ColourG = G;
									Msg.ColourB = B;
									CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 100 );
									Msg.Get();	// print on our own screen as well

									Player = MAX_NUM_PLAYERS;
								}
							}
						}
						
						if (pCashVehicle)
						{
							CashCoorsX = pCashVehicle->GetPosition().x;
							CashCoorsY = pCashVehicle->GetPosition().y;
							CashCoorsZ = pCashVehicle->GetPosition().z;
						}
				
				}
				
				// Send an update message to the clients
				if (CTimer::GetTimeInMilliseconds() / 330 != CTimer::GetPreviousTimeInMilliseconds() / 330)
				{
					CMsgStashTheCashUpdate	Msg;

					Msg.CurrentStashLocations[0] = CNetGames::CurrentStashLocations[0];
					Msg.CurrentStashLocations[1] = CNetGames::CurrentStashLocations[1];
					Msg.CashCarIsIn = CGameNet::FindIndexValueForPointer(pCashVehicle);
					Msg.CashCoorsX = CNetGames::CashCoorsX;
					Msg.CashCoorsY = CNetGames::CashCoorsY;
					Msg.CashCoorsZ = CNetGames::CashCoorsZ;

					CGameNet::SendMessageToAllClients(&Msg, sizeof(CMsgStashTheCashUpdate), 150);
				}

			}
			
			
			// All the stuff that needs to be displayed for this gametype goes here
			if (CTimer::GetTimeInMilliseconds() & (1<<9))
			{
				CCoronas::RegisterCorona(9, NULL, 255, 255, 255, 255, CVector(CashCoorsX, CashCoorsY, CashCoorsZ), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(10, NULL, 150, 150, 150, 255, CVector(CashCoorsX, CashCoorsY, CashCoorsZ), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
			}

			if (CTimer::GetTimeInMilliseconds() & (1<<9))
			{
				CCoronas::RegisterCorona(11, NULL, TeamColours[0], TeamColours[1], TeamColours[2], 255, CVector(StashLocations[CurrentStashLocations[0] * 3], StashLocations[CurrentStashLocations[0] * 3 + 1], StashLocations[CurrentStashLocations[0] * 3 + 2]), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(12, NULL, TeamColours[0], TeamColours[1], TeamColours[2], 255, CVector(StashLocations[CurrentStashLocations[0] * 3], StashLocations[CurrentStashLocations[0] * 3 + 1], StashLocations[CurrentStashLocations[0] * 3 + 2]), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
			}

			if (CTimer::GetTimeInMilliseconds() & (1<<9))
			{
				CCoronas::RegisterCorona(13, NULL, TeamColours[3], TeamColours[4], TeamColours[5], 255, CVector(StashLocations[CurrentStashLocations[1] * 3], StashLocations[CurrentStashLocations[1] * 3 + 1], StashLocations[CurrentStashLocations[1] * 3 + 2]), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(14, NULL, TeamColours[3], TeamColours[4], TeamColours[5], 255, CVector(StashLocations[CurrentStashLocations[1] * 3], StashLocations[CurrentStashLocations[1] * 3 + 1], StashLocations[CurrentStashLocations[1] * 3 + 2]), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
			}

			if (TargetBlipIndex)
			{		// Remove blip if we already have one
				CRadar::ClearBlip(TargetBlipIndex);
				TargetBlipIndex = 0;
			}

			if ( pCashVehicle || (CTimer::GetTimeInMilliseconds() & 512))
			{
				TargetBlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, CVector(CashCoorsX, CashCoorsY, CashCoorsZ), (CASHBLIPCOLOURR << 24) | (CASHBLIPCOLOURG << 16) | (CASHBLIPCOLOURB << 8) | 255, BLIPDISPLAY_BOTH);
				CRadar::ChangeBlipScale(TargetBlipIndex, 3);
			}

			if (Base0BlipIndex)
			{		// Remove blip if we already have one
				CRadar::ClearBlip(Base0BlipIndex);
				Base0BlipIndex = 0;
			}
			if ( (!pCashVehicle) || (CTimer::GetTimeInMilliseconds() & 512) || (CWorld::FindPlayerForCar(pCashVehicle)<0) || (CWorld::Players[CWorld::FindPlayerForCar(pCashVehicle)].Team != 0))
			{
				Base0BlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, CVector(StashLocations[CurrentStashLocations[0] * 3], StashLocations[CurrentStashLocations[0] * 3 + 1], StashLocations[CurrentStashLocations[0] * 3 + 2]), (BaseColours[0] << 24) | (BaseColours[1] << 16) | (BaseColours[2] << 8) | 255, BLIPDISPLAY_BOTH);
				CRadar::ChangeBlipScale(Base0BlipIndex, 5);
			}
			
			if (Base1BlipIndex)
			{		// Remove blip if we already have one
				CRadar::ClearBlip(Base1BlipIndex);
				Base1BlipIndex = 0;
			}
			if ( (!pCashVehicle) || (CTimer::GetTimeInMilliseconds() & 512) || (CWorld::FindPlayerForCar(pCashVehicle)<0) || (CWorld::Players[CWorld::FindPlayerForCar(pCashVehicle)].Team != 1))
			{
				Base1BlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, CVector(StashLocations[CurrentStashLocations[1] * 3], StashLocations[CurrentStashLocations[1] * 3 + 1], StashLocations[CurrentStashLocations[1] * 3 + 2]), (BaseColours[3] << 24) | (BaseColours[4] << 16) | (BaseColours[5] << 8) | 255, BLIPDISPLAY_BOTH);
				CRadar::ChangeBlipScale(Base1BlipIndex, 5);
			}
			
			
			
			break;


		case GAMETYPE_CAPTURETHEFLAG:
			if (CGameNet::NetWorkStatus == NETSTAT_SERVER)
			{
				switch (GameState)
				{
					case 0:
							// Find new coordinates for bases
						ASSERT(NumCTFBases >= 2);

						CurrentStashLocations[0] = CGeneral::GetRandomNumber() % NumCTFBases;
						CurrentStashLocations[1] = CGeneral::GetRandomNumber() % NumCTFBases;
						Int32	MinDistance = 1000;
							// Make sure we pick the 2 bases reasonably far apart
						while (CurrentStashLocations[0] == CurrentStashLocations[1] ||
								(CVector(CTFBases[CurrentStashLocations[0] * 3], CTFBases[CurrentStashLocations[0] * 3 + 1], 0.0f) - CVector(CTFBases[CurrentStashLocations[1] * 3], CTFBases[CurrentStashLocations[1] * 3 + 1], 0.0f)).Magnitude() < MinDistance )
						{
							CurrentStashLocations[1] = CGeneral::GetRandomNumber() % NumCTFBases;
							MinDistance = MAX(0, MinDistance-1);
						}
						
						for (Int32 C = 0; C < NUM_TEAMS; C++)
						{
							FlagCoordinates[C].x = CTFBases[CurrentStashLocations[C]*3];
							FlagCoordinates[C].y = CTFBases[CurrentStashLocations[C]*3+1];
							FlagCoordinates[C].z = CTFBases[CurrentStashLocations[C]*3+2];
							PlayerCarryingThisFlag[C] = -1;
						}
												
						GameState = 1;
						break;
						
					case 1:
						// GameOn.

						for (Int32 Team = 0; Team < NUM_TEAMS; Team++)
						{	// Update the flags for each team
							if (PlayerCarryingThisFlag[Team] >= 0)
							{	// Somebody is carrying this flag
							
								// Update the flags coordinates:
								if (CWorld::Players[PlayerCarryingThisFlag[Team]].bInUse &&
									CWorld::Players[PlayerCarryingThisFlag[Team]].pPed)
								{
									FlagCoordinates[Team] = CWorld::Players[PlayerCarryingThisFlag[Team]].pPed->GetPosition();								
								}
								
								// He may have died
								if ( (!CWorld::Players[PlayerCarryingThisFlag[Team]].bInUse) ||
									 (!CWorld::Players[PlayerCarryingThisFlag[Team]].pPed) ||
									  CWorld::Players[PlayerCarryingThisFlag[Team]].pPed->m_nPedState == PED_DEAD ||
									  CWorld::Players[PlayerCarryingThisFlag[Team]].pPed->m_nPedState == PED_DIE)
								{
									PlayerCarryingThisFlag[Team] = -1;
									
									CMsgText	Msg;
									sprintf(Msg.String, "A flag has been dropped");
									Msg.ColourR = TeamColours[Team*3];
									Msg.ColourG = TeamColours[Team*3+1];
									Msg.ColourB = TeamColours[Team*3+2];
									CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 150 );
									Msg.Get();	// print on our own screen as well
								}
								
								// He may have reached his hideout
								if (PlayerCarryingThisFlag[Team] >= 0)
								{
									if ((CWorld::Players[PlayerCarryingThisFlag[Team]].pPed->GetPosition() - CVector( CTFBases[CurrentStashLocations[CWorld::Players[PlayerCarryingThisFlag[Team]].Team] * 3], CTFBases[CurrentStashLocations[CWorld::Players[PlayerCarryingThisFlag[Team]].Team] * 3+1], CTFBases[CurrentStashLocations[CWorld::Players[PlayerCarryingThisFlag[Team]].Team] * 3+2])).Magnitude() < 7.0f )
									{
										FlagCoordinates[Team] = CVector( CTFBases[CNetGames::CurrentStashLocations[Team] * 3], CTFBases[CNetGames::CurrentStashLocations[Team] * 3+1], CTFBases[CNetGames::CurrentStashLocations[Team] * 3+2]);
										
										TeamPoints[CWorld::Players[PlayerCarryingThisFlag[Team]].Team]++;
										CWorld::Players[PlayerCarryingThisFlag[Team]].Points++;
									
										CMsgText	Msg;
										sprintf(Msg.String, "%s has brought a flag home", CWorld::Players[PlayerCarryingThisFlag[Team]].Name);
										Int32	R, G, B;
										FindPlayerMarkerColour(PlayerCarryingThisFlag[Team], &R, &G, &B);
										Msg.ColourR = R; Msg.ColourG = G; Msg.ColourB = B;
										CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 100 );
										Msg.Get();	// print on our own screen as well
										
										PlayerCarryingThisFlag[Team] = -1;

									}
								
								}
							}
							else
							{	// Nobody is carrying this flag
								// It could be picked up by somebody
								for (Int32 Player = 0; Player < MAX_NUM_PLAYERS; Player++)
								{
									if (CWorld::Players[Player].bInUse &&
										CWorld::Players[Player].Team != Team &&
										CWorld::Players[Player].pPed &&
										CWorld::Players[Player].pPed->m_nPedState != PED_DEAD && CWorld::Players[Player].pPed->m_nPedState != PED_DIE &&
										(CWorld::Players[Player].pPed->GetPosition() - FlagCoordinates[Team]).Magnitude() < 4.0f)
									{
										PlayerCarryingThisFlag[Team] = Player;
										
										CMsgText	Msg;
										sprintf(Msg.String, "%s picked up the enemy flag", CWorld::Players[Player].Name);
										Int32	R, G, B;
										FindPlayerMarkerColour(PlayerCarryingThisFlag[Team], &R, &G, &B);
										Msg.ColourR = R; Msg.ColourG = G; Msg.ColourB = B;
										CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 100 );
										Msg.Get();	// print on our own screen as well
										
										Player = MAX_NUM_PLAYERS;
									}
								}
								
								// It could be returned to base
								for (Int32 Player = 0; Player < MAX_NUM_PLAYERS; Player++)
								{
									if (CWorld::Players[Player].bInUse &&
										CWorld::Players[Player].Team == Team &&
										CWorld::Players[Player].pPed &&
										(CWorld::Players[Player].pPed->GetPosition() - FlagCoordinates[Team]).Magnitude() < 4.0f)
									{
										if (FlagCoordinates[Team] != CVector( CTFBases[CNetGames::CurrentStashLocations[Team] * 3], CTFBases[CNetGames::CurrentStashLocations[Team] * 3+1], CTFBases[CNetGames::CurrentStashLocations[Team] * 3+2]))
										{
											FlagCoordinates[Team] = CVector( CTFBases[CNetGames::CurrentStashLocations[Team] * 3], CTFBases[CNetGames::CurrentStashLocations[Team] * 3+1], CTFBases[CNetGames::CurrentStashLocations[Team] * 3+2]);

											CMsgText	Msg;
											sprintf(Msg.String, "%s returned his flag to base", CWorld::Players[Player].Name);
											Int32	R, G, B;
											FindPlayerMarkerColour(Player, &R, &G, &B);
											Msg.ColourR = R; Msg.ColourG = G; Msg.ColourB = B;
											CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 100 );
											Msg.Get();	// print on our own screen as well
										
											Player = MAX_NUM_PLAYERS;
										}
									}
								}
							}
						}


				}
				
				// Send an update message to the clients
				if (CTimer::GetTimeInMilliseconds() / 330 != CTimer::GetPreviousTimeInMilliseconds() / 330)
				{
					CMsgCaptureTheFlagUpdate	Msg;

					for (Int32 C = 0; C < NUM_TEAMS; C++)
					{
						Msg.FlagCoordinates[C*3] = FlagCoordinates[C].x;
						Msg.FlagCoordinates[C*3+1] = FlagCoordinates[C].y;
						Msg.FlagCoordinates[C*3+2] = FlagCoordinates[C].z;
					}
					Msg.CurrentStashLocations[0] = CNetGames::CurrentStashLocations[0];
					Msg.CurrentStashLocations[1] = CNetGames::CurrentStashLocations[1];

					CGameNet::SendMessageToAllClients(&Msg, sizeof(CMsgCaptureTheFlagUpdate), 100);
				}
			}
			
		
			for (Int32 C = 0; C < NUM_TEAMS; C++)
			{
				if (FlagBlipIndex[C])
				{		// Remove blip if we already have one
					CRadar::ClearBlip(FlagBlipIndex[C]);
					FlagBlipIndex[C] = 0;
				}
				FlagBlipIndex[C] = CRadar::SetCoordBlip(BLIPTYPE_COORDS, FlagCoordinates[C], (TeamColours[C*3]<<24) | (TeamColours[C*3+1]<<16) | (TeamColours[C*3+2]<<8) | 255, BLIPDISPLAY_BOTH);
				if (CTimer::GetTimeInMilliseconds() & 512)
				{
					CRadar::ChangeBlipScale(FlagBlipIndex[C], 3);
				}
				else
				{
					CRadar::ChangeBlipScale(FlagBlipIndex[C], 4);
				}
			}
			
			// All the stuff that needs to be displayed for this gametype goes here
			if (CTimer::GetTimeInMilliseconds() & (1<<9))
			{
				CCoronas::RegisterCorona(9, NULL, TeamColours[0], TeamColours[1], TeamColours[2], 255, FlagCoordinates[0], 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(10, NULL, TeamColours[0], TeamColours[1], TeamColours[2], 255, FlagCoordinates[0], 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);

				CCoronas::RegisterCorona(11, NULL, TeamColours[3+0], TeamColours[3+1], TeamColours[3+2], 255, FlagCoordinates[1], 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(12, NULL, TeamColours[3+0], TeamColours[3+1], TeamColours[3+2], 255, FlagCoordinates[1], 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
			}

			if (CTimer::GetTimeInMilliseconds() & (1<<9))
			{
				CCoronas::RegisterCorona(13, NULL, TeamColours[0], TeamColours[1], TeamColours[2], 255, CVector(CTFBases[CurrentStashLocations[0] * 3], CTFBases[CurrentStashLocations[0] * 3 + 1], CTFBases[CurrentStashLocations[0] * 3 + 2]), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(14, NULL, TeamColours[0], TeamColours[1], TeamColours[2], 255, CVector(CTFBases[CurrentStashLocations[0] * 3], CTFBases[CurrentStashLocations[0] * 3 + 1], CTFBases[CurrentStashLocations[0] * 3 + 2]), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
			}

			if (CTimer::GetTimeInMilliseconds() & (1<<9))
			{
				CCoronas::RegisterCorona(15, NULL, TeamColours[3], TeamColours[4], TeamColours[5], 255, CVector(CTFBases[CurrentStashLocations[1] * 3], CTFBases[CurrentStashLocations[1] * 3 + 1], CTFBases[CurrentStashLocations[1] * 3 + 2]), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(16, NULL, TeamColours[3], TeamColours[4], TeamColours[5], 255, CVector(CTFBases[CurrentStashLocations[1] * 3], CTFBases[CurrentStashLocations[1] * 3 + 1], CTFBases[CurrentStashLocations[1] * 3 + 2]), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
			}


			if (Base0BlipIndex)
			{		// Remove blip if we already have one
				CRadar::ClearBlip(Base0BlipIndex);
				Base0BlipIndex = 0;
			}
			Base0BlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, CVector(CTFBases[CurrentStashLocations[0] * 3], CTFBases[CurrentStashLocations[0] * 3 + 1], CTFBases[CurrentStashLocations[0] * 3 + 2]), (BaseColours[0] << 24) | (BaseColours[1] << 16) | (BaseColours[2] << 8) | 255, BLIPDISPLAY_BOTH);
			CRadar::ChangeBlipScale(Base0BlipIndex, 5);


			if (Base1BlipIndex)
			{		// Remove blip if we already have one
				CRadar::ClearBlip(Base1BlipIndex);
				Base1BlipIndex = 0;
			}
			Base1BlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, CVector(CTFBases[CurrentStashLocations[1] * 3], CTFBases[CurrentStashLocations[1] * 3 + 1], CTFBases[CurrentStashLocations[1] * 3 + 2]), (BaseColours[3] << 24) | (BaseColours[4] << 16) | (BaseColours[5] << 8) | 255, BLIPDISPLAY_BOTH);
			CRadar::ChangeBlipScale(Base1BlipIndex, 5);			
			break;


		case GAMETYPE_RATRACE:
/*
			if (CGameNet::NetWorkStatus == NETSTAT_SERVER)
			{
				switch (GameState)
				{
					case 0:
						// Find an initial node to create the first checkpoint at.
						CurrentRatRaceNode = ThePaths.FindNodeClosestToCoors(CVector(1000.0f, -500.0f, 0.0f), PFGRAPH_CARS);
						RatRaceDirection = CGeneral::GetRandomNumber() & 7;
						PickUpCoors = ThePaths.FindNodePointer(CurrentRatRaceNode)->GetCoors();
						GameState = 1;
						PreviousTimeRatRace = CTimer::GetTimeInMilliseconds();
						break;
						
					case 1:
						for (Int32 Player = 0; Player < MAX_NUM_PLAYERS; Player++)
						{
							if (CWorld::Players[Player].bInUse && CWorld::Players[Player].pPed)
							{
								if ((CWorld::Players[Player].pPed->GetPosition() - PickUpCoors).Magnitude() < 8.0f)
								{
									CWorld::Players[Player].Points2++;	// A point for this player

									// Need to pick a fresh node
									CPathNode *pResultNode;
									CPathNode *pOldNode = &ThePaths.aNodes[CurrentRatRaceNode];
									
									if ((CGeneral::GetRandomNumber() & 127) == 83)
									{		// Once in a while look for a random node not too close and not too far away
										Int32 	NewNode = ThePaths.FindNodeNearCoors(ThePaths.aNodes[CurrentRatRaceNode].Coors, PFGRAPH_CARS, 75.0f, 150.0f);
										if (NewNode >= 0)
										{
											pResultNode = &ThePaths.aNodes[NewNode];
										}
										else
										{
											pResultNode = &ThePaths.aNodes[CurrentRatRaceNode]; 
										}
									}
									else
									{
										if (!(CGeneral::GetRandomNumber() & (31 << 3)))
										{
											RatRaceDirection = CGeneral::GetRandomNumber() & 7;
										}
										ThePaths.FindNextNodeWandering(PFGRAPH_CARS, PickUpCoors, &pOldNode,
										   				&pResultNode, RatRaceDirection, &RatRaceDirection);
									}
									CurrentRatRaceNode = pResultNode - &ThePaths.aNodes[0];
									PickUpCoors = ThePaths.aNodes[CurrentRatRaceNode].Coors;
									
									
									Player = MAX_NUM_PLAYERS;
								}
							}
						}
						break;
				}

					// At the end of 60 second the player with the highest score gets a real point.
				if (CTimer::GetTimeInMilliseconds() / 60000 != PreviousTimeRatRace / 60000)
				{
					PreviousTimeRatRace = CTimer::GetTimeInMilliseconds();
					Int32	HighestScore = 0;
					Int32	HighestPlayer = -1;
					Int32	NumPlayers = 0;
					bool	bDraw = false;
					for (Int32	Player = 0; Player < MAX_NUM_PLAYERS; Player++)
					{
						if (CWorld::Players[Player].bInUse)
						{
							NumPlayers++;
							if (CWorld::Players[Player].Points2 == HighestScore)
							{
								bDraw = true;
							}
							else if (CWorld::Players[Player].Points2 > HighestScore)
							{
								HighestScore = CWorld::Players[Player].Points2;
								HighestPlayer = Player;
								bDraw = false;
							}
						}
					}
					if (NumPlayers > 1 && HighestScore > 0)
					{
						if (bDraw)
						{
							CMsgText	Msg;
							sprintf(Msg.String, "Last round was a draw");
							Msg.ColourR = 200; Msg.ColourG = 200; Msg.ColourB = 200;
							gGameNet.mNet.SendMessageToAllClients ( &Msg, sizeof(CMsgText) );
							Msg.Get();	// print on our own screen as well
						}
						else
						{
							CWorld::Players[HighestPlayer].Points++;
							
							CMsgText	Msg;
							Int32		R, G, B;
							FindPlayerMarkerColour(HighestPlayer, &R, &G, &B);
							sprintf(Msg.String, "This round was won by %s", CWorld::Players[HighestPlayer].Name);
							Msg.ColourR = R; Msg.ColourG = G; Msg.ColourB = B;
							gGameNet.mNet.SendMessageToAllClients ( &Msg, sizeof(CMsgText) );
							Msg.Get();	// print on our own screen as well
						}
					}
					for (Int32	Player = 0; Player < MAX_NUM_PLAYERS; Player++)
					{
						CWorld::Players[Player].Points2 = 0;
					}
				}


					// Send an update message to the clients
				if (CTimer::GetTimeInMilliseconds() / 330 != CTimer::GetPreviousTimeInMilliseconds() / 330)
				{
					CMsgRatRaceUpdate	Msg;

					Msg.PickupCoorsX = PickUpCoors.x;
					Msg.PickupCoorsY = PickUpCoors.y;
					Msg.PickupCoorsZ = PickUpCoors.z;

					gGameNet.mNet.SendMessageToAllClients(&Msg, sizeof(CMsgRatRaceUpdate));
				}
			}
			
			if (TargetBlipIndex)
			{		// Remove blip if we already have one
				CRadar::ClearBlip(TargetBlipIndex);
				TargetBlipIndex = 0;
			}

			TargetBlipIndex = CRadar::SetCoordBlip(BLIPTYPE_COORDS, PickUpCoors, (CASHBLIPCOLOURR << 24) | (CASHBLIPCOLOURG << 16) | (CASHBLIPCOLOURB << 8) | 255, BLIPDISPLAY_BOTH);
			if ( pCashVehicle || (CTimer::GetTimeInMilliseconds() & 256))
			{
				CRadar::ChangeBlipScale(TargetBlipIndex, 3);
			}
			else
			{
				CRadar::ChangeBlipScale(TargetBlipIndex, 4);				
			}
			
			if (CTimer::GetTimeInMilliseconds() & (1<<9))
			{
				CCoronas::RegisterCorona(9, CASHBLIPCOLOURR, CASHBLIPCOLOURG, CASHBLIPCOLOURB, 255, PickUpCoors + CVector(0.0f, 0.0f, 1.0f), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
				CCoronas::RegisterCorona(10, CASHBLIPCOLOURR, CASHBLIPCOLOURG, CASHBLIPCOLOURB, 255, PickUpCoors + CVector(0.0f, 0.0f, 1.0f), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
			}
			
			break;
*/
		case GAMETYPE_DOMINATION:

			if (CGameNet::NetWorkStatus == NETSTAT_SERVER)
			{
				if (!GameState)
				{
					GameState = 1;
					ASSERT(NumStashLocations >= 3);
					DominationBases[0] = DominationBases[1] = CGeneral::GetRandomNumber() % NumStashLocations;
					while (DominationBases[0] == DominationBases[1])
					{
						DominationBases[2] = DominationBases[1] = CGeneral::GetRandomNumber() % NumStashLocations;
					}
					while (DominationBases[1] == DominationBases[2])
					{
						DominationBases[2] = CGeneral::GetRandomNumber() % NumStashLocations;
					}
					TeamDominatingBases[0] = TeamDominatingBases[1] = TeamDominatingBases[2] = -1;
				}
				// Check whether any of the bases have been taken over by any players.
				static	UInt32	LastBaseTakeOverTime = 0;
				
				if (CTimer::GetTimeInMilliseconds() - LastBaseTakeOverTime > 500)
				{
					for (Player = 0; Player < MAX_NUM_PLAYERS; Player++)
					{
						if (CWorld::Players[Player].bInUse)
						{
							for (Base = 0; Base < DOMINATIONBASES; Base++)
							{
								if (CWorld::Players[Player].bInUse && CWorld::Players[Player].Team != TeamDominatingBases[Base] && CWorld::Players[Player].pPed &&
									(CWorld::Players[Player].pPed->GetPosition() - CVector(StashLocations[DominationBases[Base]*3], StashLocations[DominationBases[Base]*3+1], StashLocations[DominationBases[Base]*3+2])).Magnitude() < 5.0f)
								{
									CMsgText	Msg;
									Int32		R, G, B;
									FindPlayerMarkerColour(Player, &R, &G, &B);
									sprintf(Msg.String, "%s claimed a base", CWorld::Players[Player].Name);
									Msg.ColourR = R; Msg.ColourG = G; Msg.ColourB = B;
									CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 150 );
									Msg.Get();	// print on our own screen as well
									TeamDominatingBases[Base] = CWorld::Players[Player].Team;
									LastBaseTakeOverTime = CTimer::GetTimeInMilliseconds();
									Base = DOMINATIONBASES;		// Jump out
									Player = MAX_NUM_PLAYERS;
								}
							}
						}
					}
				}
				// Add points to the team for each base
				for (Base = 0; Base < DOMINATIONBASES; Base++)
				{
					if (TeamDominatingBases[Base] >= 0)
					{
						TeamPoints[TeamDominatingBases[Base]] += CTimer::GetTimeStep() * 20;
					}
				}
				// Send an update message once in a while
				if (CTimer::GetTimeInMilliseconds() / 330 != CTimer::GetPreviousTimeInMilliseconds() / 330)
				{
					CMsgDominationUpdate	Msg;

					for (Base = 0; Base < DOMINATIONBASES; Base++)
					{
						Msg.DominationBases[Base] = DominationBases[Base];
						Msg.TeamDominatingBases[Base] = TeamDominatingBases[Base];
					}
	
					CGameNet::SendMessageToAllClients(&Msg, sizeof(CMsgDominationUpdate), 150);
				}
			}

			// Display the radar blips (on server & clients)
			for (Base = 0; Base < DOMINATIONBASES; Base ++)
			{
				if (CTimer::GetTimeInMilliseconds() & (1<<9))
				{
					if (TeamDominatingBases[Base] >= 0)
					{
						CCoronas::RegisterCorona(13 + Base * 2, NULL, TeamColours[TeamDominatingBases[Base]*3], TeamColours[TeamDominatingBases[Base]*3+1], TeamColours[TeamDominatingBases[Base]*3+2], 255, CVector(StashLocations[DominationBases[Base] * 3], StashLocations[DominationBases[Base] * 3 + 1], StashLocations[DominationBases[Base] * 3 + 2]), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
						CCoronas::RegisterCorona(14 + Base * 2, NULL, TeamColours[TeamDominatingBases[Base]*3], TeamColours[TeamDominatingBases[Base]*3+1], TeamColours[TeamDominatingBases[Base]*3+2], 255, CVector(StashLocations[DominationBases[Base] * 3], StashLocations[DominationBases[Base] * 3 + 1], StashLocations[DominationBases[Base] * 3 + 2]), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
					}
					else
					{
						CCoronas::RegisterCorona(13 + Base * 2, NULL, 255, 255, 255, 255, CVector(StashLocations[DominationBases[Base] * 3], StashLocations[DominationBases[Base] * 3 + 1], StashLocations[DominationBases[Base] * 3 + 2]), 2.0f, 200.0f, CCoronas::CORONATYPE_HEADLIGHT, CCoronas::FLARETYPE_HEADLIGHTS, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_ON);
						CCoronas::RegisterCorona(14 + Base * 2, NULL, 255, 255, 255, 255, CVector(StashLocations[DominationBases[Base] * 3], StashLocations[DominationBases[Base] * 3 + 1], StashLocations[DominationBases[Base] * 3 + 2]), 7.0f, 200.0f, CCoronas::CORONATYPE_RING, CCoronas::FLARETYPE_NONE, CCoronas::CORREFL_NONE, CCoronas::LOSCHECK_OFF);
					}
				}

				if (DominationBaseBlipIndex[Base])
				{		// Remove blip if we already have one
					CRadar::ClearBlip(DominationBaseBlipIndex[Base]);
					DominationBaseBlipIndex[Base] = 0;
				}
				
				if (TeamDominatingBases[Base] >= 0)
				{
					DominationBaseBlipIndex[Base] = CRadar::SetCoordBlip(BLIPTYPE_COORDS, CVector(StashLocations[DominationBases[Base] * 3], StashLocations[DominationBases[Base] * 3 + 1], StashLocations[DominationBases[Base] * 3 + 2]), (TeamColours[TeamDominatingBases[Base]*3] << 24) | ((TeamColours[TeamDominatingBases[Base]*3+1]) << 16) | ((TeamColours[TeamDominatingBases[Base]*3+2]) << 8) | 255, BLIPDISPLAY_BOTH);
				}
				else
				{
					DominationBaseBlipIndex[Base] = CRadar::SetCoordBlip(BLIPTYPE_COORDS, CVector(StashLocations[DominationBases[Base] * 3], StashLocations[DominationBases[Base] * 3 + 1], StashLocations[DominationBases[Base] * 3 + 2]), ~0, BLIPDISPLAY_BOTH);
				}
				
				if (CTimer::GetTimeInMilliseconds() & 1024)
				{
					CRadar::ChangeBlipScale(DominationBaseBlipIndex[Base], 3);
				}
				else
				{
					CRadar::ChangeBlipScale(DominationBaseBlipIndex[Base], 4);
				}
				
			}

			break;

	
		default:
			ASSERT(0);		// Funny gametype ?
			break;

			
	}

	if (CGameNet::NetWorkStatus == NETSTAT_SERVER)
	{
		if ( (CTimer::GetTimeInMilliseconds() / 1100) != (CTimer::GetPreviousTimeInMilliseconds() / 1100) &&
			TeamGameGoingOn() )
		{
			CMsgTeamPoints	Msg;
	
			for (Int32 C = 0; C < NUM_TEAMS; C++)
			{
				Msg.TeamPoints[C] = TeamPoints[C];
			}
			CGameNet::SendMessageToAllClients(&Msg, sizeof(CMsgTeamPoints), 400);
		}
	}
};


///////////////////////////////////////////////////////////////////////////
//
// RegisterPlayerKill
// Called on the server machine when a player gets killed.
//
//////////////////////////////////////////////////////////////////////////

void CNetGames::RegisterPlayerKill(const CPed *pKilled, const void *pKiller)
{
	Int32	PlayerKilled, PlayerKiller;

	switch (GameType)
	{
		case GAMETYPE_DEATHMATCH:
		case GAMETYPE_TEAMDEATHMATCH:
		case GAMETYPE_DEATHMATCH_NOBLIPS:
		case GAMETYPE_TEAMDEATHMATCH_NOBLIPS:
			PlayerKilled = CWorld::FindPlayerSlotWithPedPointer((void *)pKilled);
			PlayerKiller = CWorld::FindPlayerSlotWithPedPointer((void *)pKiller);

			if (PlayerKiller < 0 && pKiller && ((CEntity *)pKiller)->GetIsTypeVehicle())
			{
				PlayerKiller = CWorld::FindPlayerSlotWithPedPointer(((CVehicle *)pKiller)->pDriver);
			}

			if (PlayerKiller >= 0)
			{
				if (PlayerKiller == PlayerKilled)
				{		// Suicide: take 1 point away
					if (CWorld::Players[PlayerKiller].Points > 0) CWorld::Players[PlayerKiller].Points--;
					if (GameType == GAMETYPE_TEAMDEATHMATCH)
					{
						if (TeamPoints[CWorld::Players[PlayerKiller].Team] > 0) TeamPoints[CWorld::Players[PlayerKiller].Team]--;
					}
				}
				else
				{	// Award a point to the killer
					if (GameType == GAMETYPE_DEATHMATCH || GameType == GAMETYPE_DEATHMATCH_NOBLIPS)
					{
						CWorld::Players[PlayerKiller].Points++;
					}
					else
					{	// GAMETYPE_TEAMDEATHMATCH || GAMETYPE_TEAMDEATHMATCH_NOBLIPS
						if (PlayerKilled >= 0 &&
							CWorld::Players[PlayerKiller].Team != CWorld::Players[PlayerKilled].Team &&
							CWorld::Players[PlayerKiller].Team >= 0)
						{
							CWorld::Players[PlayerKiller].Points++;
							TeamPoints[CWorld::Players[PlayerKiller].Team]++;
						}
					}
				}
			}			
			
					// Send a message to tell the players about this.
			if (PlayerKilled >= 0)
			{
				CMsgText	MsgText;

				if (PlayerKiller >= 0)
				{
					if (PlayerKiller == PlayerKilled)
					{
						// Messages for suicide
						if (CGeneral::GetRandomNumber() & 1)
						{
							sprintf(MsgText.String, "%s killed himself\n", CWorld::Players[PlayerKilled].Name);
						}
						else
						{
							sprintf(MsgText.String, "%s commited suicide\n", CWorld::Players[PlayerKilled].Name);
						}
					}
					else
					{
						// Messages for murder
						if (CGeneral::GetRandomNumber() & 1)
						{
							sprintf(MsgText.String, "%s was killed by %s\n", CWorld::Players[PlayerKilled].Name, CWorld::Players[PlayerKiller].Name);
						}
						else
						{
							sprintf(MsgText.String, "%s murdered %s\n", CWorld::Players[PlayerKiller].Name, CWorld::Players[PlayerKilled].Name);
						}
					}
				}
				else
				{
					// Death but we don't know who did it
					if (CGeneral::GetRandomNumber() & 1)
					{
						sprintf(MsgText.String, "%s died\n", CWorld::Players[PlayerKilled].Name);
					}
					else
					{
						sprintf(MsgText.String, "%s kicked the bucket\n", CWorld::Players[PlayerKilled].Name);
					}
				}
				CGameNet::SendMessageToAllClients ( &MsgText, sizeof(MsgText), 200 );
				MsgText.Get();		// Print message on server as well				
			}		
			break;

		case GAMETYPE_STASHTHECASH:
			break;

		case GAMETYPE_CAPTURETHEFLAG:
			break;



	}
};


//////////////////////////////////////////////////////////////////////
// FUNCTION : SelectSafeStartPoint
// FUNCTION : Picks a start point that is not blocked and doesn't have
//			  other players nearby shooting at it.
//////////////////////////////////////////////////////////////////////

void CNetGames::SelectSafeStartPoint(CVector *pResult, Int32 Team)
{
	Int32	Count, Player;
	Int32	BestStartPoint, Point;
	float	FurthestDistance, ClosestDistanceToPlayer, Distance;
	
	static	Int32	LastVal = 0;
	
	// At the moment we simply cycle through the startpoints. This has to be improved.
	switch (GameType)
	{
		case GAMETYPE_CAPTURETHEFLAG:
			if (Team >= 0)
			{
				pResult->x = CTFBases[CurrentStashLocations[Team] * 3] + (CGeneral::GetRandomNumber() & 7) - 3;
				pResult->y = CTFBases[CurrentStashLocations[Team] * 3 + 1] + (CGeneral::GetRandomNumber() & 7) - 3;
				pResult->z = CTFBases[CurrentStashLocations[Team] * 3 + 2] + 1.0f;
				
				return;
			}
//			no break;	fall through intended
		default:
			ASSERT(NumStartPoints);
				// Find the point furthest away from any of the other players
			FurthestDistance = 0;
			BestStartPoint = 0;
			
			for (Point = 0; Point < NumStartPoints; Point++)
			{
				ClosestDistanceToPlayer = 10000.0f;
				for (Player = 0; Player < MAX_NUM_PLAYERS; Player++)
				{
					if (CWorld::Players[Player].bInUse && CWorld::Players[Player].pPed)
					{
						Distance = (CWorld::Players[Player].pPed->GetPosition() - CVector(NetWorkStartCoors[Point][0], NetWorkStartCoors[Point][1], NetWorkStartCoors[Point][2])).Magnitude();
						if (Distance < ClosestDistanceToPlayer)
						{
							ClosestDistanceToPlayer = Distance;
						}
					}
				}
				if (ClosestDistanceToPlayer > FurthestDistance)
				{		// This is our new 'best point'
					BestStartPoint = Point;
					FurthestDistance = ClosestDistanceToPlayer;
				}
			}

			pResult->x = NetWorkStartCoors[BestStartPoint][0];
			pResult->y = NetWorkStartCoors[BestStartPoint][1];
			pResult->z = NetWorkStartCoors[BestStartPoint][2];

/*		This code used to just 'cycle through'
			LastVal = (LastVal + 1) % NumStartPoints;
	
			pResult->x = NetWorkStartCoors[LastVal][0];
			pResult->y = NetWorkStartCoors[LastVal][1];
			pResult->z = NetWorkStartCoors[LastVal][2];
*/
			break;
	}
}


//////////////////////////////////////////////////////////////////////
// FUNCTION : AddNetworkObject
// FUNCTION : Store a point used for a network game
//////////////////////////////////////////////////////////////////////

// 0 Cash generator
// 1 Stash location
// 2 Spawn point
// 3 CTF Base

void CNetGames::AddNetworkObject(float X, float Y, float Z, Int32 Type)
{
	switch (Type)
	{
		case NOBJ_CASHGENERATOR:	// Cash generator
			ASSERT(NumCashGenerators < MAX_NUM_CASH_GENERATORS);	// Too many cash generators
			if (NumCashGenerators < MAX_NUM_CASH_GENERATORS)
			{
				CashGenerators[NumCashGenerators * 3] = X;
				CashGenerators[NumCashGenerators * 3 + 1] = Y;
				CashGenerators[NumCashGenerators * 3 + 2] = Z;

				NumCashGenerators++;
			}
			break;
		case NOBJ_STASHLOCATION:	// Stash location
			if (NumStashLocations < MAX_NUM_STASH_LOCATIONS)		// Too many stash locations
			{
				StashLocations[NumStashLocations * 3] = X;
				StashLocations[NumStashLocations * 3 + 1] = Y;
				StashLocations[NumStashLocations * 3 + 2] = Z;

				NumStashLocations++;
			}
			break;
		case NOBJ_SPAWNPOINT:		// Spawn point
			ASSERT(NumStartPoints < MAX_START_POINTS);				// Too many spawn points
			if (NumStartPoints < MAX_START_POINTS)
			{
				NetWorkStartCoors[NumStartPoints][0] = X;
				NetWorkStartCoors[NumStartPoints][1] = Y;
				NetWorkStartCoors[NumStartPoints][2] = Z;
				
				NumStartPoints++;
			}
			break;
		case NOBJ_CTFBASE:	// CTF base
			if (NumCTFBases < MAX_NUM_CTF_BASES)					// Too many CTF bases
			{
				CTFBases[NumCTFBases * 3] = X;
				CTFBases[NumCTFBases * 3 + 1] = Y;
				CTFBases[NumCTFBases * 3 + 2] = Z;

				NumCTFBases++;
			}
			break;
		default:	// Pickup
			if (NumStoredPickUps < MAX_NUM_STORED_PICKUPS)			// Too many pickups
			{
				StoredPickUpCoors[NumStoredPickUps * 3] = X;
				StoredPickUpCoors[NumStoredPickUps * 3 + 1] = Y;
				StoredPickUpCoors[NumStoredPickUps * 3 + 2] = Z;
				StoredPickUpMI[NumStoredPickUps] = Type;
			
				NumStoredPickUps++;
			}
		
			break;
	}
}





//////////////////////////////////////////////////////////////////////
// FUNCTION : TeamGameGoingOn
// FUNCTION : Are we in a team game at the mo ?
//////////////////////////////////////////////////////////////////////


bool CNetGames::TeamGameGoingOn()
{
	switch (GameType)
	{
		case GAMETYPE_TEAMDEATHMATCH:
		case GAMETYPE_TEAMDEATHMATCH_NOBLIPS:
		case GAMETYPE_STASHTHECASH:
		case GAMETYPE_CAPTURETHEFLAG:
		case GAMETYPE_DOMINATION:
			return true;
			break;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// FUNCTION : FindTeamForNewPlayer
// FUNCTION : If this is a team game; what team should this player be on ?
//////////////////////////////////////////////////////////////////////

Int32 CNetGames::FindTeamForNewPlayer(Int32 NewPlayer)
{
	Int32	TeamMembers[2], C;
	Int32	Player, SmallestNumber, SmallestTeam;

	if (!TeamGameGoingOn()) return -1;

	if (CWorld::Players[NewPlayer].Team >= 0) return (CWorld::Players[NewPlayer].Team);	// We're already assigned to a team

	// Count the members of the different teams
	for (C = 0; C < 2; C++)
	{
		TeamMembers[C]=0;
	}

	for (Player = 0; Player < MAX_NUM_PLAYERS; Player++)
	{
		if (Player != NewPlayer && CWorld::Players[Player].bInUse)
		{
			if (CWorld::Players[Player].Team >= 0 && CWorld::Players[Player].Team < 2)
			{
				TeamMembers[CWorld::Players[Player].Team]++;
			}
		}
	}

	// Find team with smallest number of players
	SmallestNumber = 99999;
	SmallestTeam = 0;
	for (C = 0; C < 2; C++)
	{
		if (TeamMembers[C] < SmallestNumber)
		{
			SmallestTeam = C;
			SmallestNumber = TeamMembers[C];
		}
	}
	return (SmallestTeam);
}



//////////////////////////////////////////////////////////////////////
// FUNCTION : CarCollided
// FUNCTION : Gets called when a car collides. 
//////////////////////////////////////////////////////////////////////

void CNetGames::CarCollided(CAutomobile *pCar, float Impulse)
{
	switch(GameType)
	{
		case GAMETYPE_STASHTHECASH:
			if (pCar == pCashVehicle && Impulse > 80.0f)
			{
				Int32	Player;
				CNodeAddress TempNode = ThePaths.FindNodeClosestToCoors(pCar->GetPosition(), PFGRAPH_CARS);
				CVector NewCoors = ThePaths.FindNodePointer(TempNode)->GetCoors();
				
				CashCoorsX = NewCoors.x;
				CashCoorsY = NewCoors.y;
				CashCoorsZ = NewCoors.z;
				

				// Tell the world about this

				Player = CWorld::FindPlayerForCar(pCashVehicle);
				pCashVehicle = NULL;
				
				if (Player >= 0)
				{
					CMsgText	Msg;
					sprintf(Msg.String, "%s dropped the cash", CWorld::Players[Player].Name);
			
					Msg.ColourR = 255;
					Msg.ColourG = 255;
					Msg.ColourB = 255;
			
					CGameNet::SendMessageToAllClients ( &Msg, sizeof(CMsgText), 0 );
					Msg.Get();	// print on our own screen as well
				}
			}
			break;
	}
}


//////////////////////////////////////////////////////////////////////
// FUNCTION : Get
// FUNCTION : Processes this CMsgStashTheCashUpdate message
//////////////////////////////////////////////////////////////////////

void CMsgStashTheCashUpdate::Get()
{

	CNetGames::CashCoorsX = this->CashCoorsX;
	CNetGames::CashCoorsY = this->CashCoorsY;
	CNetGames::CashCoorsZ = this->CashCoorsZ;
	CNetGames::pCashVehicle = (CVehicle *)CGameNet::FindPointerForIndexValue(this->CashCarIsIn);
	CNetGames::CurrentStashLocations[0] = this->CurrentStashLocations[0];
	CNetGames::CurrentStashLocations[1] = this->CurrentStashLocations[1];
}


//////////////////////////////////////////////////////////////////////
// FUNCTION : Get
// FUNCTION : Processes this CMsgCaptureTheFlagUpdate message
//////////////////////////////////////////////////////////////////////

void CMsgCaptureTheFlagUpdate::Get()
{
	CNetGames::CurrentStashLocations[0] = this->CurrentStashLocations[0];
	CNetGames::CurrentStashLocations[1] = this->CurrentStashLocations[1];
	for (Int32 C = 0; C < NUM_TEAMS; C++)
	{
		CNetGames::FlagCoordinates[C].x = this->FlagCoordinates[C*3];
		CNetGames::FlagCoordinates[C].y = this->FlagCoordinates[C*3+1];
		CNetGames::FlagCoordinates[C].z = this->FlagCoordinates[C*3+2];
	}
}

//////////////////////////////////////////////////////////////////////
// FUNCTION : Get
// FUNCTION : Processes this CMsgRatRaceUpdate message
//////////////////////////////////////////////////////////////////////

void CMsgRatRaceUpdate::Get()
{
	CNetGames::PickUpCoors.x = this->PickupCoorsX;
	CNetGames::PickUpCoors.y = this->PickupCoorsY;
	CNetGames::PickUpCoors.z = this->PickupCoorsZ;
}

//////////////////////////////////////////////////////////////////////
// FUNCTION : Get
// FUNCTION : Processes this CMsgTeamPoints message
//////////////////////////////////////////////////////////////////////

void CMsgTeamPoints::Get()
{
	for (Int32 C = 0; C < NUM_TEAMS; C++)
	{
		CNetGames::TeamPoints[C] = this->TeamPoints[C];
	}
}

//////////////////////////////////////////////////////////////////////
// FUNCTION : Get
// FUNCTION : Processes this CMsgDominationUpdate message
//////////////////////////////////////////////////////////////////////

void CMsgDominationUpdate::Get()
{
	for (Int32 C = 0; C < DOMINATIONBASES; C++)
	{
		CNetGames::DominationBases[C] = this->DominationBases[C];
		CNetGames::TeamDominatingBases[C] = this->TeamDominatingBases[C];
	}
}

//////////////////////////////////////////////////////////////////////
// FUNCTION : RenderPlayerNames
// FUNCTION : Renders the names of the other players in a network game
//////////////////////////////////////////////////////////////////////

#define MAX_DIST_NAME_RENDERED	(30.0f)
#define MAX_DIST_NAME_STARTFADE	(15.0f)
#define MAX_NAME_ALPHA (150)

void CNetGames::RenderPlayerNames()
{
	float	Distance;

	if (CGameNet::NetWorkStatus != NETSTAT_SINGLEPLAYER)
	{
		Int32	Player;
		
		
		CFont::SetProportional(TRUE);
		CFont::SetBackground(FALSE);
		CFont::SetOrientation(FO_CENTRE);
		CFont::SetCentreSize(SCREEN_WIDTH);
		CFont::SetFontStyle( FO_FONT_STYLE_STANDARD );
		
		for (Player = 0; Player < MAX_NUM_PLAYERS; Player++)
		{
			if (CWorld::Players[Player].bInUse && Player != CWorld::PlayerInFocus && CWorld::Players[Player].pPed)
			{
				Distance = (TheCamera.GetPosition() - CWorld::Players[Player].pPed->GetPosition()).Magnitude();
			
				if (Distance < MAX_DIST_NAME_RENDERED)
				{

					RwV3d ScreenCoors, In;
					float ScaleX, ScaleY;
			
					In.x = CWorld::Players[Player].pPed->GetPosition().x;
					In.y = CWorld::Players[Player].pPed->GetPosition().y;
					In.z = CWorld::Players[Player].pPed->GetPosition().z + 1.2f;

					if (CSprite::CalcScreenCoors(In, &ScreenCoors, &ScaleX, &ScaleY))
					{
						Int32	R, G, B;
						
						FindPlayerMarkerColour(Player, &R, &G, &B);
					
						if (Distance > MAX_DIST_NAME_STARTFADE)
						{
							CFont::SetColor(CRGBA(R, G, B, MAX_NAME_ALPHA * (1.0f - ((Distance - MAX_DIST_NAME_STARTFADE) / (MAX_DIST_NAME_RENDERED - MAX_DIST_NAME_STARTFADE)) )));
						}
						else
						{
							CFont::SetColor(CRGBA(R, G, B, MAX_NAME_ALPHA));
						}
						CFont::SetScale( MIN (2.0f, ScaleX / 100.0f), MIN (2.0f, ScaleY / 100.0f));
						AsciiToGxtChar(CWorld::Players[Player].Name, gGxtString);
						CFont::PrintString( ScreenCoors.x, ScreenCoors.y, gGxtString);
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// FUNCTION : FindPlayerMarkerColour
// FUNCTION : returns the colour of this player (depends on whether there is a teamgame going on)
//////////////////////////////////////////////////////////////////////

void CNetGames::FindPlayerMarkerColour(Int32 PlayerNum, Int32 *pRed, Int32 *pGreen, Int32 *pBlue)
{
	if (CWorld::Players[PlayerNum].Team < 0)
	{
		*pRed = CWorld::Players[PlayerNum].ColourR;
		*pGreen = CWorld::Players[PlayerNum].ColourG;
		*pBlue = CWorld::Players[PlayerNum].ColourB;
	}
	else
	{
		*pRed = (2 * CWorld::Players[PlayerNum].ColourR + 3 * TeamColours[CWorld::Players[PlayerNum].Team * 3]) / 5;
		*pGreen = (2 * CWorld::Players[PlayerNum].ColourG + 3 * TeamColours[CWorld::Players[PlayerNum].Team * 3 + 1]) / 5;
		*pBlue = (2 * CWorld::Players[PlayerNum].ColourB + 3 * TeamColours[CWorld::Players[PlayerNum].Team * 3 + 2]) / 5;
	}
}

//////////////////////////////////////////////////////////////////////
// FUNCTION : ResetScores
// FUNCTION : Set all scores back to zero
//////////////////////////////////////////////////////////////////////

void CNetGames::ResetScores()
{
	Int32	Player, C;
	
	for (Player = 0; Player < MAX_NUM_PLAYERS; Player++)
	{
		CWorld::Players[Player].Points = 0;
		CWorld::Players[Player].Points2 = 0;
	}

	for (C = 0; C < NUM_TEAMS; C++)
	{
		TeamPoints[C] = 0;
	}
}


//////////////////////////////////////////////////////////////////////
// FUNCTION : AwardPoints
// FUNCTION : Award points to a specific player
//////////////////////////////////////////////////////////////////////

void CNetGames::AwardPoints(char Name[], Int32 Points)
{
	Int32	Player;
	
	for (Player = 0; Player < MAX_NUM_PLAYERS; Player++)
	{
		if (CWorld::Players[Player].bInUse &&
						!strncmp( Name, CWorld::Players[Player].Name, strlen(Name)))
		{
			CWorld::Players[Player].Points += Points;
		}
	}
}



#endif

