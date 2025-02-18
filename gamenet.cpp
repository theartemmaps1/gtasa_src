/////////////////////////////////////////////////////////////////////////////////
//
// FILE :    gamenet.cpp
// PURPOSE : Contains the game side of the network code
// AUTHOR :  Obbe & Dan.
// CREATED : 23/10/01
//
/////////////////////////////////////////////////////////////////////////////////

// The state of player with the network code when it was decided there wasn't going to be
// any networking in SA:
// It is possible for clients to connect to a server using a hard coded IP address.
// 2 players can run about using weapons on eachother. Death & resurrection works.
// I was working on getting in and out of cars. This isn't working yet. The only case
// that does work is the client getting into the driver seat of an empty car.
// Obbe. 1/10/2003


#ifdef GTA_NETWORK

// For log file
//#include <time.h>

// Local stuff
//#include "Netguid.h"
#include "Network.h"
#include "GameNet.h"
#include "Netmessages.h"
#include "frontend.h"
#include "globals.h"
#include "timer.h"
#include "world.h"
#include "console.h"
#include "general.h"
//#include "Stuff.h"
#ifdef GTA_PC
#include "win.h"	// for psGlobal
#endif
#include "font.h"
#include "playerped.h"
#include "pools.h"
#include "camera.h"
#include "gamelogic.h"
#include "cranes.h"
#include "pickups.h"
#include "projectileinfo.h"
#include "radar.h"
#include "explosion.h"
#include "bulletinfo.h"
#include "weapon.h"
#include "console.h"
#include "zonecull.h"
#include "animblendassociation.h"
#include "VehicleModelInfo.h"
#include "pedintelligence.h"
#include "taskmanager.h"
#include "taskbasic.h"
#include "taskplayer.h"
#include "taskcar.h"
//#include "gamespy.h"

// For calling game functions
//#include "Game.h"
//#include "GameTime.h"
//#include "cam.h"
//#include "wind.h"
//#include "beacons.h"
//#include "players.h"

#include "main.h"


Int32	CGameNet::NetWorkStatus = NETSTAT_SINGLEPLAYER;
Int32	CGameNet::SyncsDone = 0;				// How many successful sync messages have been processed
Int64	CGameNet::TimeDiffWithServer = 0;		// What is out time difference (in msecs) with the server.
bool	CGameNet::bInGameSpyGame = false;		// true if this is a game through GameSpy. false for LAN.
bool	CGameNet::bGameSpyStartMsgSent = false;	// true if we have send a message to GameSpy to tell it our game has started
bool	CGameNet::mNetworkOpen = false;
bool	CGameNet::mTimeIsValid = false;

		// For connecting to the network
#ifdef GTA_PC
CNetwork	CGameNet::mNet;
#endif
#ifdef GTA_PS2
sceEENetSockaddr	CGameNet::ServerSockAddr;			// The IP address for this client.
#endif

#define TIME_OUT_TIME (6000000)		// In milliseconds.
static	UInt8	ReceiveBuffer[2048];

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

//#define DBGP(x)		dbgprintf ( "GameNet:" x "\n" )


//bool MessageHandler_Client ( void *pMessage, NETID ID );
//void MessageHandler_Server ( void *pMessage, NETID ID );



//CGameNet	gGameNet;


///////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
/*
CGameNet::CGameNet ( void ) //: mServer ( NW_MAXNUMPLAYERS ), mClients ( MAX_LOCAL_CLIENTS )
{

	mNetworkOpen = false;
	mTimeIsValid = false;
	NetWorkStatus = NETSTAT_SINGLEPLAYER;
	bInGameSpyGame = false;
	bGameSpyStartMsgSent = false;
};


///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

CGameNet::~CGameNet ( void )
{
	OutputDebugString ( "Destroying network." );

	CloseNetwork ();

//	CloseLog ();

};
*/



///////////////////////////////////////////////////////////////////////////
// Does the needed network processing before the game update
///////////////////////////////////////////////////////////////////////////

void CGameNet::PreProcess ( void )
{

#ifdef GTA_PS2
	Network_PS2::Process();	// This deals with the network startup & shutdown.
#endif

#ifdef GTA_PS2
	if (Network_PS2::NetworkStatus_PS2 != Network_PS2::NETSTAT_RUNNING) return;
#endif

	// Process local Server/Clients
	if (NetWorkStatus == NETSTAT_SERVER)
	{
		PreProcessServer ();
	}
	else if (NetWorkStatus == NETSTAT_CLIENTSTARTINGUP || NetWorkStatus == NETSTAT_CLIENTRUNNING)
	{
		PreProcessClient ();
	}
	else
	{
		ASSERT(0);
	}

//	CGame::UpdateGame( CGameTime::TimeStep );

//	if ( !IsServerLocal() ) PrintQ ( &mOutMsgQ );	// Debug


}


///////////////////////////////////////////////////////////////////////////
// Does the needed network processing after the game update
///////////////////////////////////////////////////////////////////////////

void CGameNet::PostProcess ( void )
{

#ifdef GTA_PS2
	if (Network_PS2::NetworkStatus_PS2 != Network_PS2::NETSTAT_RUNNING) return;
#endif


	if (NetWorkStatus == NETSTAT_SERVER)
	{
		PostProcessServer ();
	}
	else if (NetWorkStatus == NETSTAT_CLIENTSTARTINGUP || NetWorkStatus == NETSTAT_CLIENTRUNNING)
	{
		PostProcessClient ();
	}
	else
	{
		ASSERT(0);
	}
	
}

///////////////////////////////////////////////////////////////////////////
// This function gets rid of all the tasks that this ped is performing.
///////////////////////////////////////////////////////////////////////////

void SetBasicTaskForPed(CPed *pPed)
{
	if (pPed)
	{
		pPed->GetPedIntelligence()->GetTaskManager()->Flush();	// Don't leave any tasks dangling on our machine.
	
		if (pPed->GetPedType() == PEDTYPE_PLAYER1 || pPed->GetPedType() == PEDTYPE_PLAYER_NETWORK)
		{
			if (pPed->m_bInVehicle && pPed->m_pMyVehicle)
			{
				pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimpleCarDrive(pPed->m_pMyVehicle));
			}
			else
			{
				pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimplePlayerOnFoot());
			}
		}
		else
		{
			if (pPed->m_bInVehicle && pPed->m_pMyVehicle)
			{
				pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimpleCarDrive(pPed->m_pMyVehicle));
			}
			else
			{
				pPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimpleStandStill(999999,true));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::PreProcessServer ( void )
{
	CMsgGeneric	*pMsg;
	UInt8		*pMsgReadIn;
//	DPID		idFrom, idTo;

	// If we are in a gamespy type game we will send updates to GameSpy regularly
/*
	if (bInGameSpyGame)
	{
		if (!bGameSpyStartMsgSent)
		{
			GameSpy.StateChangeUpdate(GAMESPY_STATE_PLAYING);
			bGameSpyStartMsgSent = true;
		}
		
		static	UInt32	LastTimeGSSend = 0;
		
		if (CTimer::GetTimeInMilliseconds() - LastTimeGSSend > 50)
		{
    		GameSpy.Process();
    		LastTimeGSSend = CTimer::GetTimeInMilliseconds();
    	}
    }
*/

	// Make sure we have a buffer for the messages to be stored in.
/*
	for (Int32 C = 1; C < MAX_NUM_PLAYERS; C++)
	{
		if (CWorld::Players[C].bInUse)
		{
			if (!mNet.apBuffers[C])
			{
				mNet.apBuffers[C] = new CMessageBuffer;
			}
			ASSERT(mNet.apBuffers[C]);
		}
	}
*/

	// Are there any messages for us ?
/*
#ifdef GTA_PC	PC specific stuff
	while (mNet.ReceiveMessage ( &idFrom, &idTo, &pMsgReadIn ))
	{
		if ( idFrom == DPID_SYSMSG )
		{		// system message
			LPDPMSG_GENERIC lpMsg = (LPDPMSG_GENERIC) pMsgReadIn;
		
			switch (lpMsg->dwType)
			{
				case DPSYS_CREATEPLAYERORGROUP:
					{
						LPDPMSG_CREATEPLAYERORGROUP		pData = (LPDPMSG_CREATEPLAYERORGROUP) lpMsg;

						strncpy ( gString, pData->dpnName.lpszShortNameA, NET_MAX_NAME_SIZE );

						// We have to find a player slot for this player (if it doesn't already have one)
						Int32	SlotNumber = CWorld::FindFreeSlotForNewNetworkPlayer(pData->dpId, pData->dpnName.lpszShortNameA);

					
						if (SlotNumber < 0)
						{
							ASSERT(0);	// Tell player to go away (Too many players)
						}
						
						
						// In the case there already is a message buffer (previous player in same slot)
						// we set the pointer back to 0
						if (mNet.apBuffers[SlotNumber])
						{
							mNet.apBuffers[SlotNumber]->DataIndex = 0;
						}

					}
					break;
					
					// If a client quits we remove it nicely.
				case DPSYS_DESTROYPLAYERORGROUP:	
					{
						LPDPMSG_DESTROYPLAYERORGROUP		pData = (LPDPMSG_DESTROYPLAYERORGROUP) lpMsg;
						// We have to find the player slot for this player (if it has one anyway)
						Int32	SlotNumber = CWorld::FindPlayerSlotForNetworkPlayer(pData->dpId);

						if (SlotNumber >= 0)
						{	// Remove the player from this slot

							sprintf(gString, "%s has left", CWorld::Players[SlotNumber].Name);
							TheConsole.AddLine(gString);						


							strcpy(CWorld::Players[SlotNumber].Name, "Player left");
							CWorld::Players[SlotNumber].bInUse = false;
							CWorld::Players[SlotNumber].DpID = 0;
							if (CWorld::Players[SlotNumber].RadarBlip)
							{
								CRadar::ClearBlip(CWorld::Players[SlotNumber].RadarBlip);
								CWorld::Players[SlotNumber].RadarBlip = 0;
							}
							if (CWorld::Players[SlotNumber].pPed && CWorld::Players[SlotNumber].pPed->m_bInVehicle && CWorld::Players[SlotNumber].pPed->m_pMyVehicle)
							{
								CWorld::Remove(CWorld::Players[SlotNumber].pPed->m_pMyVehicle);
								delete(CWorld::Players[SlotNumber].pPed->m_pMyVehicle);
							}
							if (CWorld::Players[SlotNumber].pPed)
							{
								CWorld::Remove(CWorld::Players[SlotNumber].pPed);
								delete(CWorld::Players[SlotNumber].pPed);
								CWorld::Players[SlotNumber].pPed = NULL;
							}
							
							// Also remove the buffer we have for sending messages
							if (mNet.apBuffers[SlotNumber])
							{
								delete mNet.apBuffers[SlotNumber];
								mNet.apBuffers[SlotNumber] = NULL;
							}
						}
					}
					break;
				default:
					sprintf(gString, "System mesg: %d\n", lpMsg->dwType);
					TheConsole.AddLine(gString);
					break;
			}
		}
		else
		{		// game message

			pMsg = (CMsgGeneric *)pMsgReadIn; 

//			sprintf(gString, "Received: %s\n", cNetMessages::GetMessageText ( pMsg ) );
//			TheConsole.AddLine(gString);

			switch (pMsg->Message)
			{
				case MSG_TEXT:
					((CMsgText *)pMsg)->Get();
					
					// A text message has to be send to all the clients as well.
					mNet.SendMessageToAllClients ( pMsg, sizeof(CMsgText) );
					
					break;
					
				case MSG_SYNC:

					if (!((CMsgSync *)pMsg)->IsVersionUpToDate())
					{
						CMsgKickedOut	Msg;
					
						strcpy(Msg.String, "Wrong version of game");

						mNet.SendMessageToClient_Directly ( idFrom, &Msg, sizeof(CMsgKickedOut) );
						
					
					}
					else
					{


							// Fill in time and bounce message back.
						((CMsgSync *)pMsg)->ServerTimeOnBounce = CTimer::GetTimeInMilliseconds();
						Int32	PlayerSlot = CWorld::FindPlayerSlotForNetworkPlayer(idFrom);
						((CMsgSync *)pMsg)->PlayerNumberOfClient = PlayerSlot;
						ASSERT(PlayerSlot >= 0);
						
						// Store the colours requested by the client
						CWorld::Players[PlayerSlot].ColourR = ((CMsgSync *)pMsg)->R;
						CWorld::Players[PlayerSlot].ColourG = ((CMsgSync *)pMsg)->G;
						CWorld::Players[PlayerSlot].ColourB = ((CMsgSync *)pMsg)->B;
						CWorld::Players[PlayerSlot].Points = 0;
						CWorld::Players[PlayerSlot].Points2 = 0;
						
						CWorld::Players[PlayerSlot].Team = CNetGames::FindTeamForNewPlayer(PlayerSlot);
						
						// We have to make sure the client has a ped to play with. (otherwise he couldn't get started)
						if (!CWorld::Players[PlayerSlot].pPed)
						{		// Give us a ped
							CVector	StartPos;
							CNetGames::SelectSafeStartPoint(&StartPos, CWorld::Players[PlayerSlot].Team);
							CWorld::Players[PlayerSlot].pPed = new CPlayerPed;
							CWorld::Players[PlayerSlot].pPed->SetPosition(StartPos);
							CWorld::Players[PlayerSlot].pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
							CWorld::Add(CWorld::Players[PlayerSlot].pPed);
							CWorld::Players[PlayerSlot].pPed->SetShootingAccuracy(100);
							CWorld::Players[PlayerSlot].pPed->CharCreatedBy = MISSION_CHAR;
							
								// Switch controls off on this player ped.
							((CPlayerPed *)CWorld::Players[PlayerSlot].pPed)->m_nPedType = PEDTYPE_PLAYER_NETWORK;
						}

						ASSERT(CWorld::Players[PlayerSlot].pPed);
						
						((CMsgSync *)pMsg)->PedIndexValue = FindIndexValueForPointer(CWorld::Players[PlayerSlot].pPed);	//    (CPools::GetPedPool().GetJustIndex(CWorld::Players[PlayerSlot].pPed));
						((CMsgSync *)pMsg)->PedCoorsX = CWorld::Players[PlayerSlot].pPed->GetPosition().x;
						((CMsgSync *)pMsg)->PedCoorsY = CWorld::Players[PlayerSlot].pPed->GetPosition().y;
						((CMsgSync *)pMsg)->PedCoorsZ = CWorld::Players[PlayerSlot].pPed->GetPosition().z;

						mNet.SendMessageToClient ( ((CMsgSync *)pMsg)->PlayerNumberOfClient, pMsg, sizeof(CMsgSync) );
					}
					break;




				case MSG_PERIODICSYNC:
						// Fill in time and bounce message back.
					((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce = CTimer::GetTimeInMilliseconds();
					mNet.SendMessageToClient ( ((CMsgPeriodicSync *)pMsg)->PlayerNumberOfClient, pMsg, sizeof(CMsgPeriodicSync) );
					break;


				case MSG_UPDATEPED:
				
					// Make sure the client is actually in control of this ped
					CPed	*pPedToBeUpdated;
					
					pPedToBeUpdated = (CPed *)gGameNet.FindPointerForIndexValue ( ((CMsgUpdatePed *)pMsg)->PoolIndexValue);
					
					if (pPedToBeUpdated && (!pPedToBeUpdated->bControlledByServer) && pPedToBeUpdated->UpdateControlCounter == ((CMsgUpdatePed *)pMsg)->UpdateControlCounter)
					{
//						((CMsgUpdatePed *)pMsg)->Get();
						CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
					}
					break;
					
				case MSG_UPDATECAR:
					// Make sure the client is actually in control of this car
					CAutomobile	*pCarToBeUpdated;
					
					pCarToBeUpdated = (CAutomobile *)gGameNet.FindPointerForIndexValue ( ((CMsgUpdateCar *)pMsg)->PoolIndexValue);
					
					if (pCarToBeUpdated && (!pCarToBeUpdated->bControlledByServer) && pCarToBeUpdated->UpdateControlCounter == ((CMsgUpdateCar *)pMsg)->UpdateControlCounter)
					{
//						((CMsgUpdateCar *)pMsg)->Get();
						CMsgUpdateCar::GetCarMessage((UInt8 *)pMsg);
					}
					break;
					
				case MSG_REQUESTTOENTERCAR:
					eDoors DoorId;
					uint32 nGettingIn = 0;
					CMsgRequestToEnterCar *pMsgReqEnter = (CMsgRequestToEnterCar *)pMsg;

					ASSERT(pMsgReqEnter->PlayerRequesting);
					CPed *pEntPed = CWorld::Players[pMsgReqEnter->PlayerRequesting].pPed;

					CVehicle *pCar = (CVehicle *)FindPointerForIndexValue(pMsgReqEnter->CarToBeEntered);

					if (pCar)
					{
						if (!CCranes::IsThisCarBeingCarriedByAnyCrane(pCar))
						{
							switch(pMsgReqEnter->nDoor)
							{
								case CAR_DOOR_LF:	DoorId = FRONT_LEFT_DOOR;  nGettingIn = 1; break;
								case CAR_DOOR_LR:	DoorId = REAR_LEFT_DOOR; nGettingIn = 2;break;
								case CAR_DOOR_RF:	DoorId = FRONT_RIGHT_DOOR; nGettingIn = 4;break;
								case CAR_DOOR_RR:	DoorId = REAR_RIGHT_DOOR; nGettingIn = 8;break;
								default: nGettingIn = 0; break;
							}
								
							if (!pMsgReqEnter->DraggedPed)
							{		// Just entering car (don't need to do carjack stuff since the seat we're going for is empty						
				
								if(pEntPed->IsPedInControl() &&
								(pEntPed->m_nHealth>0) &&
								(!(pCar->m_nGettingInFlags & nGettingIn)) &&
								(!(pCar->m_nGettingOutFlags & nGettingIn))&&
								!pCar->bIsBeingCarJacked &&
								(!nGettingIn || pCar->IsDoorReady(DoorId)||pCar->IsDoorFullyOpen(DoorId)) &&
								(!pEntPed->m_pAnim) )
								{
									pEntPed->SetObjective(ENTER_CAR_AS_DRIVER, pCar);
									pEntPed->SetEnterCar_AllClear(pCar, pMsgReqEnter->nDoor, nGettingIn);
									pEntPed->m_nDoor = pMsgReqEnter->nDoor;

									// The car as well as the ped are now under control of the server until the whole
									// getting into the car operation is finished.
									pEntPed->bControlledByServer = true;
									pCar->bControlledByServer = true;
									if (pCar->pDriver) pCar->pDriver->bControlledByServer = true;	// In case this is a player.
																		
//		TheConsole.AddLine("Server ALLOWED entering car");
								}
							}	
							else
							{		// Carjack. The seat we're going for has somebody in it.
								CPed *pDraggedPed = (CPed *)FindPointerForIndexValue(pMsgReqEnter->DraggedPed);
							
								if((pEntPed->m_nPedState != PED_CARJACK)&&
								(!(pCar->m_nGettingInFlags & nGettingIn))&&
								(!(pCar->m_nGettingOutFlags & nGettingIn))&&
								!pCar->bIsBeingCarJacked &&
								((pDraggedPed) && (pDraggedPed->m_nPedState == PED_DRIVING))&&
								(pCar->IsDoorReady(DoorId)||pCar->IsDoorFullyOpen(DoorId)) &&
								(!pEntPed->m_pAnim) && ( (!pDraggedPed->m_pAnim) || pDraggedPed->m_pAnim->HasNoFinishCallback() ))
								{
									pEntPed->SetObjective(ENTER_CAR_AS_DRIVER, pCar);
									pEntPed->SetCarJack_AllClear(pCar, pMsgReqEnter->nDoor, nGettingIn);
									pEntPed->m_nDoor = pMsgReqEnter->nDoor;

									// The car as well as the ped are now under control of the server until the whole
									// getting into the car operation is finished.
									pEntPed->bControlledByServer = true;
									pCar->bControlledByServer = true;
									if (pDraggedPed) pDraggedPed->bControlledByServer = true;	// In case this is a player.
									
//		TheConsole.AddLine("Server ALLOWED entering car (jacking)");
								}
							}
						}
					}

//					sprintf(gString, "MSG_REQUESTTOENTERCAR on server\n" );
//					TheConsole.AddLine(gString);
					break;
					
				case MSG_PROJECTILE:
					CProjectileInfo::GetProjectileMessage_Server((CMsgProjectile *)pMsg);
					break;

				case MSG_SNIPERBULLET:
					{
						CVector	Origin, Speed;
					
						Origin.x = ((CMsgSniperBullet *)pMsg)->CoorsX;
						Origin.y = ((CMsgSniperBullet *)pMsg)->CoorsY;
						Origin.z = ((CMsgSniperBullet *)pMsg)->CoorsZ;
						Speed.x = ((CMsgSniperBullet *)pMsg)->SpeedX;
						Speed.y = ((CMsgSniperBullet *)pMsg)->SpeedY;
						Speed.z = ((CMsgSniperBullet *)pMsg)->SpeedZ;
						CBulletInfo::AddBullet(CGameNet::FindPointerForIndexValue(((CMsgSniperBullet *)pMsg)->Owner),
												(eWeaponType)((CMsgSniperBullet *)pMsg)->WeaponType, Origin, Speed);
					}
					break;
					
				case MSG_GUNSHOTFIRED:
					CWeapon::ProcessGunShotMessageOnServer((CMsgGunShotFired *)pMsg);
					break;

				case MSG_PLAYERWANTSTOLEAVECAR:
					{
						Int32	Player = ((CMsgPlayerWantsToLeaveCar *)pMsg)->Player;
						CPed*	pPed = CWorld::Players[Player].pPed;
						
						if (pPed && pPed->m_pMyVehicle)
						{
							pPed->SetObjective(LEAVE_CAR, pPed->m_pMyVehicle);
							pPed->bControlledByServer = true;
//							pPed->m_pMyVehicle->bControlledByServer = true;
						}
					}
					break;
					
				case MSG_CAMERAMATRIX:
					((CMsgCameraMatrix *)pMsg)->Get(&CWorld::Players[((CMsgCameraMatrix *)pMsg)->Player].CamInverseMat );
					break;

				case MSG_STRIKE:
					((CMsgStrike *)pMsg)->Get();
					break;

				case MSG_FLAMETHROWERSHOT:
					CWeapon::ProcessFlameThrowerMessage_ForServer((CMsgFlameThrowerShot *)pMsg);
					break;
					
				default:
					sprintf(gString, "Unknown msg (s): %d %d\n", pMsg->Message, idFrom);
					TheConsole.AddLine(gString);
					break;
			
			}
		}
	}
#endif
*/


	static sceEENetSockaddr	FromAddress;

	pMsgReadIn = ReceiveBuffer;
	while (Network_PS2::ReceiveMessage ( &FromAddress, &pMsgReadIn ))
	{
		Int32	Player;
		pMsg = (CMsgGeneric *)pMsgReadIn; 

		
		Player = CWorld::FindPlayerSlotForNetworkPlayer(&FromAddress);
		if (Player < 0)
		{		// This player doesn't exist yet. If we have space, add it.

			if (pMsg->Message == MSG_SYNC)
			{
				// We have to find a player slot for this player (if it doesn't already have one)
				Player = CWorld::FindFreeSlotForNewNetworkPlayer(&FromAddress, "New Player");
	
				if (Player < 0)
				{
					ASSERT(0);	// Tell player to go away (Too many players)
				}
				DEBUGLOG1("New player has been added in slot: %d\n", Player);
				CWorld::Players[Player].bInUse = true;
//			CWorld::Players[Player].SockAddr = FromAddress;
			}
			else
			{		// This player doesn't exist and the message is not a MSG_SYNC ignore message
				continue;
			}
		}

		CWorld::Players[Player].TimeOfLastMessageReceived = CTimer::GetTimeInMilliseconds();	// Needed so that the server can detect time-outs


//			sprintf(gString, "Received: %s\n", cNetMessages::GetMessageText ( pMsg ) );
//			TheConsole.AddLine(gString);

		switch (pMsg->Message)
		{
			case MSG_TEXT:
				((CMsgText *)pMsg)->Get();
					
				// A text message has to be send to all the clients as well.
				SendMessageToAllClients ( pMsg, sizeof(CMsgText), 200 );
					
				break;
					
			case MSG_SYNC:

//				if (!((CMsgSync *)pMsg)->IsVersionUpToDate())
//				{
//					CMsgKickedOut	Msg;
					
//					strcpy(Msg.String, "Wrong version of game");

//					SendMessageToClient_Directly ( idFrom, &Msg, sizeof(CMsgKickedOut) );
//						
//					ASSERT(0);	
//				}
//				else
				{

						// Fill in time and bounce message back.
					((CMsgSync *)pMsg)->ServerTimeOnBounce = CTimer::GetTimeInMilliseconds();
					Int32	PlayerSlot = CWorld::FindPlayerSlotForNetworkPlayer(&FromAddress);
					((CMsgSync *)pMsg)->PlayerNumberOfClient = PlayerSlot;
					DEBUGLOG1("Server received a sync message from client. Player:%d\n", PlayerSlot);

					ASSERT(PlayerSlot >= 0);
						
					// Store the colours requested by the client
					CWorld::Players[PlayerSlot].ColourR = ((CMsgSync *)pMsg)->R;
					CWorld::Players[PlayerSlot].ColourG = ((CMsgSync *)pMsg)->G;
					CWorld::Players[PlayerSlot].ColourB = ((CMsgSync *)pMsg)->B;
					CWorld::Players[PlayerSlot].Points = 0;
					CWorld::Players[PlayerSlot].Points2 = 0;
					strncpy(CWorld::Players[PlayerSlot].Name, ((CMsgSync *)pMsg)->PlayerName, NET_MAX_NAME_SIZE);
						
					CWorld::Players[PlayerSlot].Team = CNetGames::FindTeamForNewPlayer(PlayerSlot);
						
					// We have to make sure the client has a ped to play with. (otherwise he couldn't get started)
					if (!CWorld::Players[PlayerSlot].pPed)
					{		// Give us a ped
						CVector	StartPos;
						CNetGames::SelectSafeStartPoint(&StartPos, CWorld::Players[PlayerSlot].Team);
						CWorld::Players[PlayerSlot].pPed = new CPlayerPed;
						CWorld::Players[PlayerSlot].pPed->SetPosition(StartPos);
						CWorld::Players[PlayerSlot].pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
						CWorld::Add(CWorld::Players[PlayerSlot].pPed);
						CWorld::Players[PlayerSlot].pPed->SetShootingAccuracy(100);
						CWorld::Players[PlayerSlot].pPed->SetCharCreatedBy(MISSION_CHAR);
							
							// Switch controls off on this player ped.
						((CPlayerPed *)CWorld::Players[PlayerSlot].pPed)->m_nPedType = PEDTYPE_PLAYER_NETWORK;
					}

					ASSERT(CWorld::Players[PlayerSlot].pPed);
						
					((CMsgSync *)pMsg)->PedIndexValue = FindIndexValueForPointer(CWorld::Players[PlayerSlot].pPed);	//    (CPools::GetPedPool().GetJustIndex(CWorld::Players[PlayerSlot].pPed));
					((CMsgSync *)pMsg)->PedCoorsX = CWorld::Players[PlayerSlot].pPed->GetPosition().x;
					((CMsgSync *)pMsg)->PedCoorsY = CWorld::Players[PlayerSlot].pPed->GetPosition().y;
					((CMsgSync *)pMsg)->PedCoorsZ = CWorld::Players[PlayerSlot].pPed->GetPosition().z;

					SendMessageToClient ( ((CMsgSync *)pMsg)->PlayerNumberOfClient, pMsg, sizeof(CMsgSync), 0 );

					cprintf("Player joining:%s\n", CWorld::Players[PlayerSlot].Name);
				}
				break;


			case MSG_PERIODICSYNC:
					// Fill in time and bounce message back.
				((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce = CTimer::GetTimeInMilliseconds();
				SendMessageToClient ( ((CMsgPeriodicSync *)pMsg)->PlayerNumberOfClient, pMsg, sizeof(CMsgPeriodicSync), 0 );
				break;


			case MSG_PLAYERQUIT:
					// This player is removed from the game.

				break;

			case MSG_UPDATEPED:
				
				// Make sure the client is actually in control of this ped
				CPed	*pPedToBeUpdated;
					
				pPedToBeUpdated = (CPed *)FindPointerForIndexValue ( ((CMsgUpdatePed *)pMsg)->PoolIndexValue);
					
				if (pPedToBeUpdated && (!pPedToBeUpdated->bControlledByServer) && pPedToBeUpdated->UpdateControlCounter == ((CMsgUpdatePed *)pMsg)->UpdateControlCounter)
				{
//					((CMsgUpdatePed *)pMsg)->Get();
					CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
					SetBasicTaskForPed(pPedToBeUpdated);
				}
				break;
					
			case MSG_UPDATECAR:
				// Make sure the client is actually in control of this car
				CAutomobile	*pCarToBeUpdated;
					
				pCarToBeUpdated = (CAutomobile *)FindPointerForIndexValue ( ((CMsgUpdateCar *)pMsg)->PoolIndexValue);
					
				if (pCarToBeUpdated && (!pCarToBeUpdated->bControlledByServer) && pCarToBeUpdated->UpdateControlCounter == ((CMsgUpdateCar *)pMsg)->UpdateControlCounter)
				{
//					((CMsgUpdateCar *)pMsg)->Get();
					CMsgUpdateCar::GetCarMessage((UInt8 *)pMsg);
				}
				break;
					
			case MSG_REQUESTTOENTERCAR:
				eDoors DoorId;
				uint32 nGettingIn = 0;
				CMsgRequestToEnterCar *pMsgReqEnter = (CMsgRequestToEnterCar *)pMsg;

				ASSERT(pMsgReqEnter->PlayerRequesting);
				CPed *pEntPed = CWorld::Players[pMsgReqEnter->PlayerRequesting].pPed;

				CVehicle *pCar = (CVehicle *)FindPointerForIndexValue(pMsgReqEnter->CarToBeEntered);

				if (pCar)
				{
					if (!CCranes::IsThisCarBeingCarriedByAnyCrane(pCar))
					{
						switch(pMsgReqEnter->nDoor)
						{
							case CAR_DOOR_LF:	DoorId = FRONT_LEFT_DOOR;  nGettingIn = 1; break;
							case CAR_DOOR_LR:	DoorId = REAR_LEFT_DOOR; nGettingIn = 2;break;
							case CAR_DOOR_RF:	DoorId = FRONT_RIGHT_DOOR; nGettingIn = 4;break;
							case CAR_DOOR_RR:	DoorId = REAR_RIGHT_DOOR; nGettingIn = 8;break;
							default: nGettingIn = 0; break;
						}
								
						if (!pMsgReqEnter->DraggedPed)
						{		// Just entering car (don't need to do carjack stuff since the seat we're going for is empty						
				
							//Needs to be updated to use new ai code - Gordon.
							if(pEntPed->IsPedInControl() &&
							(pEntPed->m_nHealth>0) &&
							(!(pCar->m_nGettingInFlags & nGettingIn)) &&
							(!(pCar->m_nGettingOutFlags & nGettingIn))&&
							!pCar->bIsBeingCarJacked &&
							(!nGettingIn || pCar->IsDoorReady(DoorId)||pCar->IsDoorFullyOpen(DoorId)) )
							{
//								pEntPed->SetObjective(ENTER_CAR_AS_DRIVER, pCar);
//								pEntPed->SetEnterCar_AllClear(pCar, pMsgReqEnter->nDoor, nGettingIn);
//								pEntPed->m_nDoor = pMsgReqEnter->nDoor;

								ASSERT(pMsgReqEnter->TaskType == CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER);

								// Remove any tasks this ped may have.
								pEntPed->GetPedIntelligence()->GetTaskManager()->Flush();	// Don't leave any tasks dangling on our machine.
								pEntPed->GetPedIntelligence()->AddTaskDefault(new CTaskSimplePlayerOnFoot());

								// Create a new 'GetInCar' task.
								pEntPed->GetPedIntelligence()->AddTaskPrimary(new CTaskComplexEnterCarAsDriver(pCar));
								// We should consider jumping straight to the right sub-task somehow.
								//pEntPed->GetPedIntelligence()->GetTaskManager()->SetNextSubTask(pSimplestTask->GetParent());
								CTask* pNextSubTask = ((CTaskComplexEnterCarAsDriver *)pEntPed->GetPedIntelligence()->GetTaskPrimary())->CreateNextSubTask_AfterSimpleCarAlign(pEntPed);	// Find the appropriate sub-task

	// 


								// We have to give the ped that is entering the car the CTaskComplexEnterCarAsDriver task (Or similar)
								// Then we have to find the appropriate subtask.

								// The car as well as the ped are now under control of the server until the whole
								// getting into the car operation is finished.
								pEntPed->bControlledByServer = true;
								pCar->bControlledByServer = true;
								if (pCar->pDriver) pCar->pDriver->bControlledByServer = true;	// In case this is a player.
																		
								TheConsole.AddLine("Server ALLOWED entering car");
							}
						}	
						else
						{
//		TheConsole.AddLine("Server ALLOWED entering car (jacking)");
ASSERT(0);	// Code not implemented yet.
						}
					}
				}

//				sprintf(gString, "MSG_REQUESTTOENTERCAR on server\n" );
//				TheConsole.AddLine(gString);
				break;
					
			case MSG_PROJECTILE:
				CProjectileInfo::GetProjectileMessage_Server((CMsgProjectile *)pMsg);
				break;

/*
			case MSG_SNIPERBULLET:
				{
					CVector	Origin, Speed;
					
					Origin.x = ((CMsgSniperBullet *)pMsg)->CoorsX;
					Origin.y = ((CMsgSniperBullet *)pMsg)->CoorsY;
					Origin.z = ((CMsgSniperBullet *)pMsg)->CoorsZ;
					Speed.x = ((CMsgSniperBullet *)pMsg)->SpeedX;
					Speed.y = ((CMsgSniperBullet *)pMsg)->SpeedY;
					Speed.z = ((CMsgSniperBullet *)pMsg)->SpeedZ;
					CBulletInfo::AddBullet(CGameNet::FindPointerForIndexValue(((CMsgSniperBullet *)pMsg)->Owner),
											(eWeaponType)((CMsgSniperBullet *)pMsg)->WeaponType, Origin, Speed);
				}
				break;
*/					
			case MSG_GUNSHOTFIRED:
				CWeapon::ProcessGunShotMessage((CMsgGunShotFired *)pMsg);
				break;

			case MSG_PLAYERWANTSTOLEAVECAR:
				{
					Int32	Player = ((CMsgPlayerWantsToLeaveCar *)pMsg)->Player;
					CPed*	pPed = CWorld::Players[Player].pPed;
						
					if (pPed && pPed->m_pMyVehicle)
					{
						pPed->SetObjective(LEAVE_CAR, pPed->m_pMyVehicle);
						pPed->bControlledByServer = true;
//						pPed->m_pMyVehicle->bControlledByServer = true;
					}
				}
				break;
					
			case MSG_CAMERAMATRIX:
				((CMsgCameraMatrix *)pMsg)->Get(&CWorld::Players[((CMsgCameraMatrix *)pMsg)->Player].CamInverseMat );
				break;

			case MSG_STRIKE:
				((CMsgStrike *)pMsg)->Get();
				break;

			case MSG_FLAMETHROWERSHOT:
				CWeapon::ProcessFlameThrowerMessage_ForServer((CMsgFlameThrowerShot *)pMsg);
				break;
					
					
			case MSG_DAMAGEEVENT:
				((CMsgDamageEvent *)pMsg)->Get();
				break;
			
			default:
				DEBUGLOG1(gString, "Unknown msg (s): %d\n", pMsg->Message);
				TheConsole.AddLine(gString);
				break;
			
		}
	}
};

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::PostProcessServer ( void )
{
	Int32 i;

	// Once in a while send a message to tell the clients about the names of the
	// players that have joined.
	if ( (CTimer::GetTimeInMilliseconds()/1700) != (CTimer::GetPreviousTimeInMilliseconds()/1700))
	{
		CMsgPlayerNames::SendPlayerNamesToClients();
	}
/*
	// Tell the clients about the player states (players may have died etc)
	if (CTimer::GetTimeInMilliseconds() / 412 != CTimer::GetPreviousTimeInMilliseconds() / 412)
	{
		CMsgPlayerStates MsgStates;
		MsgStates.Set();
		SendMessageToAllClients ( &MsgStates, sizeof(CMsgPlayerStates), 300 );
	}
*/	
	
		// Go through the peds
		
		// Only do ped of player 0 (server) for now
//		if (CWorld::Players[0].pPed)
//		{
//			Msg.Set(CWorld::Players[0].pPed);
//			mNet.SendMessageToAllClients ( &Msg, sizeof(CMsgUpdatePed) );
//		}

#define TWEAKNETLOAD(X)

	for (Int32 ClientPlayer = 1; ClientPlayer < MAX_NUM_PLAYERS; ClientPlayer++)
	{
		if (CWorld::Players[ClientPlayer].bInUse)
		{
			TWEAKNETLOAD( Int32 PedsTested = 0; Int32 PedsSend = 0; Int32 CarsTested = 0; Int32 CarsSend = 0; )
			// Do all peds
			CPedPool& PedPool = CPools::GetPedPool();
			CPed* pPed;
			i=PedPool.GetSize();
			
			while(i--)
			{
				pPed = PedPool.GetSlot(i);
				if(pPed)
				{
					TWEAKNETLOAD(PedsTested++;)
					if (pPed->NumQuickUpdates ||
							CNetLoad::ShouldPedBeUpdated(pPed, ClientPlayer, CTimer::GetTimeInMilliseconds() - pPed->NetLoad.LastUpdateTime[ClientPlayer]))
					{
						Int32	MsgSize = CMsgUpdatePed::BuildPedMessageInBuffer(pPed);
						SendMessageToClient ( ClientPlayer, gMsgBuffer, MsgSize, 300 );
						pPed->NetLoad.LastUpdateTime[ClientPlayer] = CTimer::GetTimeInMilliseconds();
						TWEAKNETLOAD(PedsSend++;)
					}
				}
			}

			// Do all cars

			CVehiclePool& VehiclePool = CPools::GetVehiclePool();
			CVehicle* pVehicle;
			i=VehiclePool.GetSize();

			while(i--)
			{
				pVehicle = VehiclePool.GetSlot(i);
				if(pVehicle && pVehicle->GetVehicleType() == VEHICLE_TYPE_CAR)
				{
					TWEAKNETLOAD(CarsTested++;)
					if (pVehicle->NumQuickUpdates ||
							CNetLoad::ShouldCarBeUpdated((CAutomobile *)pVehicle, ClientPlayer, CTimer::GetTimeInMilliseconds() - pVehicle->NetLoad.LastUpdateTime[ClientPlayer]))
					{						
						Int32	MsgSize = CMsgUpdateCar::BuildCarMessageInBuffer((CAutomobile *)pVehicle);
						SendMessageToClient ( ClientPlayer, gMsgBuffer, MsgSize, 300 );
						pVehicle->NetLoad.LastUpdateTime[ClientPlayer] = CTimer::GetTimeInMilliseconds();

						TWEAKNETLOAD(CarsSend++;)
					}
				}
			}
			TWEAKNETLOAD( sprintf(gString, "Peds: %d/%d Cars: %d/%d", PedsSend, PedsTested, CarsSend, CarsTested); )
			TWEAKNETLOAD( DEBUGLOG(gString); )
		}
	}


		// Decrease the number of quickupdates to be done.
	{
		CPedPool& PedPool = CPools::GetPedPool();
		CPed* pPed;
		i=PedPool.GetSize();	
		while(i--)
		{
			pPed = PedPool.GetSlot(i);
			if(pPed && pPed->NumQuickUpdates) pPed->NumQuickUpdates--;
		}

		CVehiclePool& VehiclePool = CPools::GetVehiclePool();
		CVehicle* pVehicle;
		i=VehiclePool.GetSize();	
		while(i--)
		{
			pVehicle = VehiclePool.GetSlot(i);
			if(pVehicle && pVehicle->NumQuickUpdates) pVehicle->NumQuickUpdates--;
		}
	}
		
	if (CTimer::GetTimeInMilliseconds() / 1702 != CTimer::GetPreviousTimeInMilliseconds() / 1702)
	{
		CMsgEmptyVehicles MsgEV;
		MsgEV.Set();
		SendMessageToAllClients ( &MsgEV, sizeof(CMsgEmptyVehicles), 200 );
	}

	if (CTimer::GetTimeInMilliseconds() / 1705 != CTimer::GetPreviousTimeInMilliseconds() / 1705)
	{
		CMsgEmptyPeds MsgEP;
		MsgEP.Set();
		SendMessageToAllClients ( &MsgEP, sizeof(CMsgEmptyPeds), 200 );
	}


	// Once in a while send a message to update the clock and the weather
	if ( (CTimer::GetTimeInMilliseconds()/960) != (CTimer::GetPreviousTimeInMilliseconds()/960))
	{
		CMsgSyncGameClocks	Msg;
		Msg.Set();
		SendMessageToAllClients ( &Msg, sizeof(CMsgSyncGameClocks), 200 );
	}
	if ( (CTimer::GetTimeInMilliseconds()/1960) != (CTimer::GetPreviousTimeInMilliseconds()/1960))
	{
		CMsgWeather	Msg;
		Msg.Set();
		SendMessageToAllClients ( &Msg, sizeof(CMsgWeather), 500 );
	}



	// Test for time-outs. Players that haven't send us a message in a long time will be removed from the game.
	for (Int32 Player = 1; Player < MAX_NUM_PLAYERS; Player++)
	{
		if (CWorld::Players[Player].bInUse)
		{
			if (CTimer::GetTimeInMilliseconds() > CWorld::Players[Player].TimeOfLastMessageReceived + TIME_OUT_TIME)
			{		// It's been too long since we heard from this player. Throw him out.
				ThrowPlayerOutOfGame(Player);
			}
		}
	}

//	SendAllBufferedMessages_ForServer();	// Must be done last

};


///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::PreProcessClient ( void )
{
	CMsgGeneric	*pMsg;
	UInt8		*pMsgReadIn;
	UInt32	Size;
	Int32	C;
	bool	Result;
#ifdef GTA_PC
	DPID	idFrom, idTo;
#endif

	// Make sure we have a buffer for the messages to be stored in.
/*
	if (!mNet.apBuffers[0])
	{
		mNet.apBuffers[0] = new CMessageBuffer;
	}
	ASSERT(mNet.apBuffers[0]);
*/

/* PC specific stuff
#ifdef GTA_PC
	while (mNet.ReceiveMessage ( &idFrom, &idTo, &pMsgReadIn ))
	{

		if ( idFrom == DPID_SYSMSG )
		{		// system message
			LPDPMSG_GENERIC lpMsg = (LPDPMSG_GENERIC) pMsgReadIn;
		
			switch (lpMsg->dwType)
			{
				case DPSYS_SESSIONLOST:

					TheConsole.AddLine("Lost connection to host");	// This should really appear in the frontend somehow

					// Jump back to frontend
					CCullZones::bCullZonesDisabled = true;
					gGameNet.CloseNetwork();
					gGameNet.NetWorkStatus = NETSTAT_SINGLEPLAYER;
					FrontEndMenuManager.m_WantsToRestartGame = true;	
					FrontEndMenuManager.m_QuitToFrontEnd = true;

					gGameNet.SwitchToSinglePlayerGame();
					
					return;
					break;
				case DPSYS_DESTROYPLAYERORGROUP:
					// One of the players has left the game.
						LPDPMSG_DESTROYPLAYERORGROUP		pData = (LPDPMSG_DESTROYPLAYERORGROUP) lpMsg;
						Int32	SlotNumber = CWorld::FindPlayerSlotForNetworkPlayer(pData->dpId);

						if (SlotNumber >= 0)
						{	// Remove the player from this slot
							sprintf(gString, "%s has left", CWorld::Players[SlotNumber].Name);
							TheConsole.AddLine(gString);						

							CWorld::Players[SlotNumber].bInUse = false;
							CWorld::Players[SlotNumber].DpID = 0;
							if (CWorld::Players[SlotNumber].RadarBlip)
							{
								CRadar::ClearBlip(CWorld::Players[SlotNumber].RadarBlip);
								CWorld::Players[SlotNumber].RadarBlip = 0;
							}
							if (CWorld::Players[SlotNumber].pPed && CWorld::Players[SlotNumber].pPed->m_bInVehicle && CWorld::Players[SlotNumber].pPed->m_pMyVehicle)
							{
								CWorld::Remove(CWorld::Players[SlotNumber].pPed->m_pMyVehicle);
								delete(CWorld::Players[SlotNumber].pPed->m_pMyVehicle);
							}
							if (CWorld::Players[SlotNumber].pPed)
							{
								CWorld::Remove(CWorld::Players[SlotNumber].pPed);
								delete(CWorld::Players[SlotNumber].pPed);
								CWorld::Players[SlotNumber].pPed = NULL;
							}
						}
					break;
				default:
					sprintf(gString, "System mesg: %d\n", lpMsg->dwType);
					TheConsole.AddLine(gString);
					break;
			}
		}
		else
		{		// game message

			pMsg = (CMsgGeneric *)pMsgReadIn; 

			switch (pMsg->Message)
			{
				case MSG_TEXT:
					((CMsgText *)pMsg)->Get();
					break;
					
				case MSG_SYNC:

					// Make sure we have a ped to play with. If not; create one.
					Int32	PlayerSlot = ((CMsgSync *)pMsg)->PlayerNumberOfClient;
					


					CPed *pPed = (CPed *) gGameNet.FindPointerForIndexValue(((CMsgSync *)pMsg)->PedIndexValue);
					
					if ( (!pPed) || CWorld::Players[PlayerSlot].pPed != pPed)
					{		// Have to create a new ped here.

						// Is this slot occupied at the moment.
						pPed = (CPed *) gGameNet.FindPointerForIndexValue_DontTestFlags(((CMsgSync *)pMsg)->PedIndexValue);
						if ( pPed )
						{	// Get rid of the old ped in this slot
							CWorld::Remove(pPed);
							delete pPed;
							pPed = NULL;
						}
		
										// The << 8 is to deal with the funny way the pools work
						CWorld::Players[PlayerSlot].pPed = new( (((CMsgSync *)pMsg)->PedIndexValue) >> 8 ) CPlayerPed;
						gGameNet.SetPoolFlags( ((CMsgSync *)pMsg)->PedIndexValue );	// We have to set the right 'flags' value for this pool element. This way we can work out whether a slot has been emptied and filled on the server

						ASSERT(CWorld::Players[PlayerSlot].pPed);
						CWorld::Players[PlayerSlot].pPed->SetPosition(((CMsgSync *)pMsg)->PedCoorsX, ((CMsgSync *)pMsg)->PedCoorsY, ((CMsgSync *)pMsg)->PedCoorsZ);
						CWorld::Players[PlayerSlot].pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
						CWorld::Add(CWorld::Players[PlayerSlot].pPed);
						CWorld::Players[PlayerSlot].pPed->SetShootingAccuracy(100);		
					}
					

					ASSERT(CWorld::Players[PlayerSlot].pPed);

					// This might cause some havoc
					CWorld::PlayerInFocus = PlayerSlot;
					TheCamera.pTargetEntity=FindPlayerPed();		// This will be replaced with something nicer
					TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity=FindPlayerPed();

					// We have to receive a number of these so that we can synch to clocks up (client uses
					// same clock as server)
					if (CTimer::GetTimeInMilliseconds() - ((CMsgSync *)pMsg)->LocalTimeOnSend < 2000)	// Make sure message has not been going for ages
					{
						Int64	TimeDiff = ((UInt64)((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce) -
										(   ((UInt64)CTimer::GetTimeInMilliseconds()) + ((UInt64)((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend)    ) / 2;
											
						if (SyncsDone == 0)
						{
							TimeDiffWithServer = TimeDiff;
						}
						else
						{
							TimeDiffWithServer += TimeDiff;
						}

						SyncsDone++;

						if (SyncsDone >= NW_NUMSYNCSATSTART)
						{
							NetWorkStatus = NETSTAT_CLIENTRUNNING;
							CTimer::m_snTimeInMilliseconds = ((Int64)CTimer::m_snTimeInMilliseconds) + (TimeDiffWithServer / NW_NUMSYNCSATSTART);
						}
					}
					
					// Switch controls off on player 0. This one is controlled by the server
					if (CWorld::Players[0].pPed)
					{
						((CPlayerPed *)CWorld::Players[0].pPed)->m_nPedType = PEDTYPE_PLAYER_NETWORK;
					}
					
//					sprintf(gString, "We are player: %d\n", (short)((CMsgSync *)pMsg)->PlayerNumberOfClient );
//					TheConsole.AddLine(gString);


					break;

				case MSG_KICKEDOUT:
					TheConsole.AddLine(((CMsgKickedOut *)pMsg)->String);	// This should really appear in the frontend somehow
	
					// Jump back to frontend
					CCullZones::bCullZonesDisabled = true;
					gGameNet.CloseNetwork();
					gGameNet.NetWorkStatus = NETSTAT_SINGLEPLAYER;
					FrontEndMenuManager.m_WantsToRestartGame = true;	
					FrontEndMenuManager.m_QuitToFrontEnd = true;
					gGameNet.SwitchToSinglePlayerGame();


					break;


				case MSG_PERIODICSYNC:
				
UInt32 TestTime = CTimer::GetTimeInMilliseconds();

					if (CTimer::GetTimeInMilliseconds() - ((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend < 2000)	// Make sure message has not been going for ages
					{
						// The time on this client should be at least as big as the time on the server when it bounced
						CTimer::m_snTimeInMilliseconds = MAX(CTimer::m_snTimeInMilliseconds, ((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce);
						// If the server got the message at an earlier time than it was sent we should modify the time directly
						if (((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend > ((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce)
						{
							CTimer::m_snTimeInMilliseconds -= ((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend - ((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce;
						}

						Int64	TimeDiff = ((UInt64)((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce) -
										( ((UInt64)CTimer::GetTimeInMilliseconds()) + ((UInt64)((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend)) / 2;

							// Only allow for a certain maximum change
						TimeDiff /= 3;	// Only do 33% of difference (avoid jumping too much)
						TimeDiff = MAX(-SNC_MAXCHANGE, TimeDiff);
						TimeDiff = MIN(SNC_MAXCHANGE, TimeDiff);
						CTimer::m_snTimeInMilliseconds = ((Int64)CTimer::m_snTimeInMilliseconds) + TimeDiff;
						
//						sprintf(gString, "PerSync: %d\n", TimeDiff );
//						TheConsole.AddLine(gString);

					}
//sprintf(gString, "TimeDiff: %d\n", TestTime - CTimer::GetTimeInMilliseconds() );
//OutputDebugString(gString);
					break;
					
				case MSG_PLAYERNAMES:
//					((CMsgPlayerNames *)pMsg)->Get();
					CMsgPlayerNames::ReceivePlayerNames((UInt8 *)pMsg);
					break;

				case MSG_UPDATEPED:

					if (CWorld::Players[CWorld::PlayerInFocus].pPed != (CPed *)gGameNet.FindPointerForIndexValue ( ((CMsgUpdatePed *)pMsg)->PoolIndexValue))
					{		// Peds that are not the local players'
						CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
						CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					}
					else if (((CMsgUpdatePed *)pMsg)->bControlledByServer)
					{		// Server is in control of the local player
						CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
						CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					}
					else if (((CMsgUpdatePed *)pMsg)->UpdateControlCounter != CWorld::Players[CWorld::PlayerInFocus].pPed->UpdateControlCounter)
					{		// Or the server overwrites the data. (once this happened we're in control again)
						CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
						CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					}
					else if (CWorld::Players[CWorld::PlayerInFocus].pPed)
					{
						CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					}
					break;

				case MSG_UPDATECAR:
				
					// Don't update a car if our own ped is driving it
					if ( (!CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle) ||
						   CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle != (CVehicle *)gGameNet.FindPointerForIndexValue ( ((CMsgUpdateCar *)pMsg)->PoolIndexValue) ||
						   (CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle->pDriver != CWorld::Players[CWorld::PlayerInFocus].pPed)
						||		// Unless the server is in control
						(((CMsgUpdateCar *)pMsg)->bControlledByServer)
						||		// Or the server overwrites the data. (once this happened we're in control again)
						(((CMsgUpdateCar *)pMsg)->UpdateControlCounter != CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle->UpdateControlCounter)
						)
					{					
						CMsgUpdateCar::GetCarMessage((UInt8 *)pMsg);
						CMsgUpdateCar::GetCarMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					}
					else
					{
						CMsgUpdateCar::GetCarMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					}

					break;

				case MSG_EMPTYVEHICLES:
					((CMsgEmptyVehicles *)pMsg)->Get();
					break;

				case MSG_ONEEMPTYVEHICLE:
					((CMsgOneEmptyVehicle *)pMsg)->Get();
					break;

				case MSG_EMPTYPEDS:
					((CMsgEmptyPeds *)pMsg)->Get();
					break;

				case MSG_ONEEMPTYPED:
					((CMsgOneEmptyPed *)pMsg)->Get();
					break;

				case MSG_REQUESTTOENTERCAR:
					CMsgRequestToEnterCar *pMsgReqEnter = (CMsgRequestToEnterCar *)pMsg;

					ASSERT(CWorld::PlayerInFocus == pMsgReqEnter->PlayerRequesting);


//					if (pMsgReqEnter->bGranted)		// Make sure we got permission
//					{
//						CVehicle *pVeh = (CVehicle *) FindPointerForIndexValue(pMsgReqEnter->CarToBeEntered);
//					
//						if (pVeh)	// Make sure the car is (still) here
//						{
//							sprintf(gString, "MSG_REQUESTTOENTERCAR on client\n" );
//							TheConsole.AddLine(gString);
//
//							CWorld::Players[pMsgReqEnter->PlayerRequesting].pPed->SetObjective(ENTER_CAR_AS_DRIVER, pVeh );
//						}
//					}

					break;


				case MSG_PICKUPCOORS:
					((CMsgPickupCoors *)pMsg)->Get();
					break;

				case MSG_PLAYERSTATES:
					((CMsgPlayerStates *)pMsg)->Get();
					break;

				case MSG_PICKUP:
					CPickups::aPickUps[((CMsgPickup *)pMsg)->Index].ProcessUpdateFromServer((CMsgPickup *)pMsg);
					break;

				case MSG_ALLPICKUPS:
					CPickups::ProcessUpdateFromServer((UInt8 *)pMsg);
					break;

				case MSG_GIVEWEAPON:
					CWorld::Players[CWorld::PlayerInFocus].pPed->GiveWeapon( (eWeaponType)((CMsgGiveWeapon *)pMsg)->WeaponType,((CMsgGiveWeapon *)pMsg)->Ammo);
						// Only if player is unarmed will we make player use this specific weapon
					if (CWorld::Players[CWorld::PlayerInFocus].pPed->m_nChosenWeapon == CWorld::Players[CWorld::PlayerInFocus].pPed->GetWeaponSlot(WEAPONTYPE_UNARMED)) CWorld::Players[CWorld::PlayerInFocus].pPed->m_nChosenWeapon = CWorld::Players[CWorld::PlayerInFocus].pPed->GetWeaponSlot( (eWeaponType)((CMsgGiveWeapon *)pMsg)->WeaponType );
					break;
					
				case MSG_PROJECTILE:
					CProjectileInfo::GetProjectileMessage_Client((CMsgProjectile *)pMsg);
					break;

				case MSG_PROJECTILESOFF:
					CProjectileInfo::ProcessProjectilesOffMessage((CMsgProjectilesOff *)pMsg);
					break;

				case MSG_EXPLOSION:
					CVector	Coors;
					Coors.x = ((CMsgExplosion *)pMsg)->CoorsX;
					Coors.y = ((CMsgExplosion *)pMsg)->CoorsY;
					Coors.z = ((CMsgExplosion *)pMsg)->CoorsZ;
					CExplosion::AddExplosion(NULL, NULL, (eExplosionType) ((CMsgExplosion *)pMsg)->ExplosionType,
											Coors, 0);
					break;

				case MSG_PARTICLES:
					((CMsgParticles *)pMsg)->Get();
					break;

				case MSG_BULLETTRACE:
					((CMsgBulletTrace *)pMsg)->Get();
					break;


				case MSG_STASHTHECASH:
					((CMsgStashTheCashUpdate *)pMsg)->Get();
					break;

				case MSG_CAPTURETHEFLAG:
					((CMsgCaptureTheFlagUpdate *)pMsg)->Get();
					break;

				case MSG_RATRACE:
					((CMsgRatRaceUpdate *)pMsg)->Get();
					break;

				case MSG_TEAMPOINTS:
					((CMsgTeamPoints *)pMsg)->Get();
					break;

				case MSG_SYNCGAMECLOCKS:
					((CMsgSyncGameClocks *)pMsg)->Get();
					break;

				case MSG_SYNCWEATHER:
					((CMsgWeather *)pMsg)->Get();
					break;

				case MSG_FLAMETHROWERSHOT:
					CWeapon::ProcessFlameThrowerMessage_ForClient((CMsgFlameThrowerShot *)pMsg);
					break;

				case MSG_DOMINATIONUPDATE:
					((CMsgDominationUpdate *)pMsg)->Get();
					break;

				default:
					sprintf(gString, "Unknown msg: %d %d %d\n", pMsg->Message, idFrom, Size);
					TheConsole.AddLine(gString);
					break;
			
			}
		}
	}
#endif
*/

	static sceEENetSockaddr	FromAddress;

	pMsgReadIn = ReceiveBuffer;
	while (Network_PS2::ReceiveMessage ( &FromAddress, &pMsgReadIn ))
	{
		CWorld::Players[0].TimeOfLastMessageReceived = CTimer::GetTimeInMilliseconds();

		pMsg = (CMsgGeneric *)pMsgReadIn; 

		switch (pMsg->Message)
		{
			case MSG_TEXT:
				((CMsgText *)pMsg)->Get();
				break;
					
			case MSG_SYNC:

				// Make sure we have a ped to play with. If not; create one.
				Int32	PlayerSlot = ((CMsgSync *)pMsg)->PlayerNumberOfClient;

				CPed *pPed = (CPed *) CGameNet::FindPointerForIndexValue(((CMsgSync *)pMsg)->PedIndexValue);

DEBUGLOG1("Client received MSG_SYNC back. Player:%d\n", PlayerSlot);
					
				if ( (!pPed) || CWorld::Players[PlayerSlot].pPed != pPed)
				{		// Have to create a new ped here.

					// Is this slot occupied at the moment.
					pPed = (CPed *) CGameNet::FindPointerForIndexValue_DontTestFlags(((CMsgSync *)pMsg)->PedIndexValue);
					if ( pPed )
					{	// Get rid of the old ped in this slot
						CWorld::Remove(pPed);
						delete pPed;
						pPed = NULL;
					}

					// Also get rid of the old ped that was our player (shouldn't really be here after startup is done properly)
					if (CWorld::Players[PlayerSlot].pPed)
					{
						CWorld::Remove(CWorld::Players[PlayerSlot].pPed);
						delete CWorld::Players[PlayerSlot].pPed;
						CWorld::Players[PlayerSlot].pPed = NULL;
					}
		
									// The << 8 is to deal with the funny way the pools work
					CWorld::Players[PlayerSlot].pPed = new( (((CMsgSync *)pMsg)->PedIndexValue) >> 8 ) CPlayerPed;
					CGameNet::SetPoolFlags( ((CMsgSync *)pMsg)->PedIndexValue );	// We have to set the right 'flags' value for this pool element. This way we can work out whether a slot has been emptied and filled on the server
					ASSERT(CWorld::Players[PlayerSlot].pPed);
					CWorld::Players[PlayerSlot].pPed->SetPosition(((CMsgSync *)pMsg)->PedCoorsX, ((CMsgSync *)pMsg)->PedCoorsY, ((CMsgSync *)pMsg)->PedCoorsZ);
					CWorld::Players[PlayerSlot].pPed->SetOrientation(DEGTORAD(0.0f), DEGTORAD(0.0f), DEGTORAD(0.0f));
					CWorld::Players[PlayerSlot].pPed->SetPedStats(PEDSTAT_PLAYER);
					CWorld::Players[PlayerSlot].pPed->SetCharCreatedBy(MISSION_CHAR);
					CWorld::Add(CWorld::Players[PlayerSlot].pPed);
					CWorld::Players[PlayerSlot].pPed->SetShootingAccuracy(100);
					CWorld::Players[PlayerSlot].bInUse = true;
					
				}
					
				ASSERT(CWorld::Players[PlayerSlot].pPed);

				CWorld::PlayerInFocus = PlayerSlot;
				TheCamera.pTargetEntity=FindPlayerPed();		// This will be replaced with something nicer
				TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity=FindPlayerPed();

				// We have to receive a number of these so that we can synch to clocks up (client uses
				// same clock as server)
				if (CTimer::GetTimeInMilliseconds() - ((CMsgSync *)pMsg)->LocalTimeOnSend < 2000)	// Make sure message has not been going for ages
				{
					Int64	TimeDiff = ((UInt64)((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce) -
									(   ((UInt64)CTimer::GetTimeInMilliseconds()) + ((UInt64)((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend)    ) / 2;
											
					if (SyncsDone == 0)
					{
						TimeDiffWithServer = TimeDiff;
					}
					else
					{
						TimeDiffWithServer += TimeDiff;
					}

					SyncsDone++;

					if (SyncsDone >= NW_NUMSYNCSATSTART)
					{
						NetWorkStatus = NETSTAT_CLIENTRUNNING;
						CTimer::m_snTimeInMilliseconds = ((Int64)CTimer::m_snTimeInMilliseconds) + (TimeDiffWithServer / NW_NUMSYNCSATSTART);
						CWorld::Players[0].TimeOfLastMessageReceived = CTimer::GetTimeInMilliseconds();	// Since we have changed our local time we have to set this variable afresh
						DEBUGLOG("Client status is now: NETSTAT_CLIENTRUNNING\n");
					}
				}
					
				// Switch controls off on player 0. This one is controlled by the server now
				if (CWorld::Players[0].pPed)
				{
					((CPlayerPed *)CWorld::Players[0].pPed)->m_nPedType = PEDTYPE_PLAYER_NETWORK;
				}
				CWorld::Players[0].bInUse = true;
					
//				sprintf(gString, "We are player: %d\n", (short)((CMsgSync *)pMsg)->PlayerNumberOfClient );
//				TheConsole.AddLine(gString);

				break;

			case MSG_KICKEDOUT:
//				TheConsole.AddLine(((CMsgKickedOut *)pMsg)->String);	// This should really appear in the frontend somehow
				DEBUGLOG(((CMsgKickedOut *)pMsg)->String);
	
				// Jump back to frontend
//				CCullZones::bCullZonesDisabled = true;
				CGameNet::CloseNetwork();
				CGameNet::NetWorkStatus = NETSTAT_SINGLEPLAYER;
				FrontEndMenuManager.m_WantsToRestartGame = false;	
				FrontEndMenuManager.m_QuitToFrontEnd = true;
				CGameNet::SwitchToSinglePlayerGame();

				break;

			case MSG_PERIODICSYNC:
				
				if (CTimer::GetTimeInMilliseconds() - ((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend < 2000)	// Make sure message has not been going for ages
				{
					// The time on this client should be at least as big as the time on the server when it bounced
					CTimer::m_snTimeInMilliseconds = MAX(CTimer::m_snTimeInMilliseconds, ((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce);
					// If the server got the message at an earlier time than it was sent we should modify the time directly
					if (((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend > ((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce)
					{
						CTimer::m_snTimeInMilliseconds -= ((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend - ((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce;
					}

					Int64	TimeDiff = ((UInt64)((CMsgPeriodicSync *)pMsg)->ServerTimeOnBounce) -
									( ((UInt64)CTimer::GetTimeInMilliseconds()) + ((UInt64)((CMsgPeriodicSync *)pMsg)->LocalTimeOnSend)) / 2;

						// Only allow for a certain maximum change
					TimeDiff /= 3;	// Only do 33% of difference (avoid jumping too much)
					TimeDiff = MAX(-SNC_MAXCHANGE, TimeDiff);
					TimeDiff = MIN(SNC_MAXCHANGE, TimeDiff);
					CTimer::m_snTimeInMilliseconds = ((Int64)CTimer::m_snTimeInMilliseconds) + TimeDiff;

					CWorld::Players[0].TimeOfLastMessageReceived = CTimer::GetTimeInMilliseconds();	// Since we have changed our local time we have to set this variable afresh
						
//					sprintf(gString, "PerSync: %d\n", TimeDiff );
//					TheConsole.AddLine(gString);

				}
//sprintf(gString, "TimeDiff: %d\n", TestTime - CTimer::GetTimeInMilliseconds() );
//OutputDebugString(gString);
				break;

			case MSG_PLAYERNAMES:
				CMsgPlayerNames::ReceivePlayerNames((UInt8 *)pMsg);
				break;

			case MSG_UPDATEPED:
				CPed *pPedToBeUpdated = (CPed *)CGameNet::FindPointerForIndexValue ( ((CMsgUpdatePed *)pMsg)->PoolIndexValue);
				if (CWorld::Players[CWorld::PlayerInFocus].pPed != pPedToBeUpdated)
				{		// Peds that are not the local players'
					CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
					CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					SetBasicTaskForPed(pPedToBeUpdated);	// Don't leave any tasks dangling on our machine.
				}
				else if (((CMsgUpdatePed *)pMsg)->bControlledByServer)
				{		// Server is in control of the local player
					CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
					CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					SetBasicTaskForPed(pPedToBeUpdated);	// Don't leave any tasks dangling on our machine.
				}
				else if (((CMsgUpdatePed *)pMsg)->UpdateControlCounter != CWorld::Players[CWorld::PlayerInFocus].pPed->UpdateControlCounter)
				{		// Or the server overwrites the data. (once this happened we're in control again)
					CMsgUpdatePed::GetPedMessage((UInt8 *)pMsg);
					CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
					SetBasicTaskForPed(pPedToBeUpdated);	// Don't leave any tasks dangling on our machine.
				}
				else if (CWorld::Players[CWorld::PlayerInFocus].pPed)
				{
					CMsgUpdatePed::GetPedMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
				}
				break;
				
			case MSG_UPDATECAR:
				
				// Don't update a car if our own ped is driving it
				if ( (!CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle) ||
					   CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle != (CVehicle *)CGameNet::FindPointerForIndexValue ( ((CMsgUpdateCar *)pMsg)->PoolIndexValue) ||
					   (CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle->pDriver != CWorld::Players[CWorld::PlayerInFocus].pPed)
					||		// Unless the server is in control
					(((CMsgUpdateCar *)pMsg)->bControlledByServer)
					||		// Or the server overwrites the data. (once this happened we're in control again)
					(((CMsgUpdateCar *)pMsg)->UpdateControlCounter != CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle->UpdateControlCounter)
					)
				{					
					CMsgUpdateCar::GetCarMessage((UInt8 *)pMsg);
					CMsgUpdateCar::GetCarMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
				}
				else
				{
					CMsgUpdateCar::GetCarMessage_VariablesServerHasControlOver((UInt8 *)pMsg);
				}

				break;

			case MSG_EMPTYVEHICLES:
				((CMsgEmptyVehicles *)pMsg)->Get();
				break;

			case MSG_ONEEMPTYVEHICLE:
				((CMsgOneEmptyVehicle *)pMsg)->Get();
				break;

			case MSG_EMPTYPEDS:
				((CMsgEmptyPeds *)pMsg)->Get();
				break;

			case MSG_ONEEMPTYPED:
				((CMsgOneEmptyPed *)pMsg)->Get();
				break;

			case MSG_REQUESTTOENTERCAR:
				CMsgRequestToEnterCar *pMsgReqEnter = (CMsgRequestToEnterCar *)pMsg;

				ASSERT(CWorld::PlayerInFocus == pMsgReqEnter->PlayerRequesting);
/*

				if (pMsgReqEnter->bGranted)		// Make sure we got permission
				{
					CVehicle *pVeh = (CVehicle *) FindPointerForIndexValue(pMsgReqEnter->CarToBeEntered);
				
					if (pVeh)	// Make sure the car is (still) here
					{
						sprintf(gString, "MSG_REQUESTTOENTERCAR on client\n" );
						TheConsole.AddLine(gString);
						CWorld::Players[pMsgReqEnter->PlayerRequesting].pPed->SetObjective(ENTER_CAR_AS_DRIVER, pVeh );
					}
				}
*/
				break;


			case MSG_PICKUPCOORS:
				((CMsgPickupCoors *)pMsg)->Get();
				break;

			case MSG_PLAYERSTATES:
				((CMsgPlayerStates *)pMsg)->Get();
				break;

			case MSG_PICKUP:
				CPickups::aPickUps[((CMsgPickup *)pMsg)->Index].ProcessUpdateFromServer((CMsgPickup *)pMsg);
				break;

			case MSG_ALLPICKUPS:
				CPickups::ProcessUpdateFromServer((UInt8 *)pMsg);
				break;

			case MSG_GIVEWEAPON:
				CWorld::Players[CWorld::PlayerInFocus].pPed->GiveWeapon( (eWeaponType)((CMsgGiveWeapon *)pMsg)->WeaponType,((CMsgGiveWeapon *)pMsg)->Ammo);
					// Only if player is unarmed will we make player use this specific weapon
				if (CWorld::Players[CWorld::PlayerInFocus].pPed->m_nChosenWeapon == CWorld::Players[CWorld::PlayerInFocus].pPed->GetWeaponSlot(WEAPONTYPE_UNARMED)) CWorld::Players[CWorld::PlayerInFocus].pPed->m_nChosenWeapon = CWorld::Players[CWorld::PlayerInFocus].pPed->GetWeaponSlot( (eWeaponType)((CMsgGiveWeapon *)pMsg)->WeaponType );
				break;
					
			case MSG_PROJECTILE:
				CProjectileInfo::GetProjectileMessage_Client((CMsgProjectile *)pMsg);
				break;

			case MSG_PROJECTILESOFF:
				CProjectileInfo::ProcessProjectilesOffMessage((CMsgProjectilesOff *)pMsg);
				break;

			case MSG_EXPLOSION:
				CVector	Coors;
				Coors.x = ((CMsgExplosion *)pMsg)->CoorsX;
				Coors.y = ((CMsgExplosion *)pMsg)->CoorsY;
				Coors.z = ((CMsgExplosion *)pMsg)->CoorsZ;
				CExplosion::AddExplosion(NULL, NULL, (eExplosionType) ((CMsgExplosion *)pMsg)->ExplosionType,
										Coors, 0);
				break;

			case MSG_GUNSHOTFIRED:
				CWeapon::ProcessGunShotMessage((CMsgGunShotFired *)pMsg);
				break;

			case MSG_PARTICLES:
				((CMsgParticles *)pMsg)->Get();
				break;

			case MSG_BULLETTRACE:
				((CMsgBulletTrace *)pMsg)->Get();
				break;


			case MSG_STASHTHECASH:
				((CMsgStashTheCashUpdate *)pMsg)->Get();
				break;

			case MSG_CAPTURETHEFLAG:
				((CMsgCaptureTheFlagUpdate *)pMsg)->Get();
				break;

			case MSG_RATRACE:
				((CMsgRatRaceUpdate *)pMsg)->Get();
				break;

			case MSG_TEAMPOINTS:
				((CMsgTeamPoints *)pMsg)->Get();
				break;

			case MSG_SYNCGAMECLOCKS:
				((CMsgSyncGameClocks *)pMsg)->Get();
				break;

			case MSG_SYNCWEATHER:
				((CMsgWeather *)pMsg)->Get();
				break;

			case MSG_FLAMETHROWERSHOT:
				CWeapon::ProcessFlameThrowerMessage_ForClient((CMsgFlameThrowerShot *)pMsg);
				break;

			case MSG_DOMINATIONUPDATE:
				((CMsgDominationUpdate *)pMsg)->Get();
				break;

			default:
				DEBUGLOG2(gString, "Unknown msg: %d %d\n", pMsg->Message, Size);
//				TheConsole.AddLine(gString);
				break;
		}
	}





	// If we're in the process of starting up we need to sync with the server
	if (NetWorkStatus == NETSTAT_CLIENTSTARTINGUP)
	{
		CMsgSync	Msg;
		
		Msg.LocalTimeOnSend = CTimer::GetTimeInMilliseconds();
		Msg.PlayerNumberOfClient = -1;
		Msg.R = FrontEndMenuManager.m_PrefsPlayerRed;
		Msg.G = FrontEndMenuManager.m_PrefsPlayerGreen;
		Msg.B = FrontEndMenuManager.m_PrefsPlayerBlue;
		strncpy(Msg.PlayerName, FrontEndMenuManager.m_PlayerName, NET_MAX_NAME_SIZE);
		
		SendMessageToServer ( &Msg, sizeof(CMsgSync), 0 );
		DEBUGLOG("Client sent MSG_SYNC to server\n");

	}
	
};




///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::PostProcessClient ( void )
{
	// Send an update message for the ped we're controlling

	if (NetWorkStatus == NETSTAT_CLIENTRUNNING)
	{
		if (CTimer::GetTimeInMilliseconds() / 100 != CTimer::GetPreviousTimeInMilliseconds() / 100)
		{
			CMsgUpdatePed	Msg;

			ASSERT(CWorld::Players[CWorld::PlayerInFocus].pPed);

			Int32	MsgSize = CMsgUpdatePed::BuildPedMessageInBuffer(CWorld::Players[CWorld::PlayerInFocus].pPed);
			SendMessageToServer ( gMsgBuffer, MsgSize, 100 );

			// If we control a car we send an update message for that also
			if (CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle &&
				CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle->pDriver == CWorld::Players[CWorld::PlayerInFocus].pPed &&
				CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle->GetVehicleType() == VEHICLE_TYPE_CAR)
			{
				Int32	MsgSize = CMsgUpdateCar::BuildCarMessageInBuffer((CAutomobile *)CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle);
				SendMessageToServer ( gMsgBuffer, MsgSize, 100 );

			
//				CMsgUpdateCar MsgCar;
//				MsgCar.Set( (CAutomobile *) CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle);
//				mNet.SendMessageToServer ( &MsgCar, sizeof(CMsgUpdateCar) );
			}
		}


		// Send the (inverse) matrix of our camera so that the server can do sensible car/ped generation
		if (CTimer::GetTimeInMilliseconds() / 300 != CTimer::GetPreviousTimeInMilliseconds() / 300)
		{
			CMsgCameraMatrix	Msg;
			Msg.Player = CWorld::PlayerInFocus;
			Msg.Set(&TheCamera.m_matInverse);
			SendMessageToServer ( &Msg, sizeof(CMsgCameraMatrix), 300 );

//			CMsgText	MsgT;
//			strcpy( MsgT.String, "Text message send!!!!");
//			SendMessageToServer ( &MsgT, sizeof(CMsgText), 300 );
		}
		DoSyncThing();		// Keep the clock in sync with the clock on the server. (They get out of sync usually)
	}


	// Test for time-out. If we haven't heard from the server for a while we jump out of the game.
	if (CTimer::GetTimeInMilliseconds() > CWorld::Players[0].TimeOfLastMessageReceived + TIME_OUT_TIME)
	{		// It's been too long since we heard from this player. Throw him out.
			// First send a message to the server to tell it we're quitting
DEBUGLOG2("TIME-OUT on client. Time:%d TimeOfLastMessageReceived:%d\n",
			CTimer::GetTimeInMilliseconds(), CWorld::Players[0].TimeOfLastMessageReceived);


		CPlayerQuit	Msg;
		SendMessageToServer (&Msg, sizeof(CPlayerQuit), 0);
		CGameNet::NetWorkStatus = NETSTAT_SINGLEPLAYER;
		FrontEndMenuManager.m_WantsToRestartGame = false;	
		FrontEndMenuManager.m_QuitToFrontEnd = true;
		SwitchToSinglePlayerGame();
	}
//	mNet.SendAllBufferedMessages_ForClient();	// Must be done last

};




///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

bool CGameNet::OpenNetwork ( void )
{
#ifndef FINAL
	DEBUGLOG2("*** Msg size UpdatePed:%d UpdateCar:%d", sizeof(CMsgUpdatePed), sizeof(CMsgUpdateCar));
#endif

	if (mNetworkOpen) return true;	// was already open. No need opening again.

	CNetGames::AddNetworkObject(1503.0f,-1662.0f,14.0f, NOBJ_SPAWNPOINT);
	CNetGames::AddNetworkObject(1479.0f,-1706.5f,14.0f, NOBJ_SPAWNPOINT);
	CNetGames::AddNetworkObject(1448.0f,-1662.5f,14.0f, NOBJ_SPAWNPOINT);
	CNetGames::AddNetworkObject(1478.0f,-1619.0f,14.0f, NOBJ_SPAWNPOINT);



#ifdef GTA_PC
	if ( mNet.Setup ( PsGlobal.instance ) ) //gpMyD3DApp->m_hInst ) )
	{
		mNetworkOpen = true;
		return true;
	}
#endif



#ifdef GTA_PS2
	Network_PS2::StartUpNetwork();
//	mNetworkOpen = true;	This is done by the startup code. (It may take a few seconds for the network to start up)
	return true;
#endif

	return false;
};


///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::CloseNetwork ( void )
{
	DEBUGLOG( "Disconnecting from network." );

//	DestroyClients ();
//	DestroyServer ();

/*
	if (NetWorkStatus = NETSTAT_SERVER)
	{
		if (bInGameSpyGame)
		{
			GameSpy.StateChangeUpdate(GAMESPY_STATE_EXITING);
		}
	}
	bInGameSpyGame = false;
	bGameSpyStartMsgSent = false;
*/

	mTimeIsValid = false;
	
	NetWorkStatus = NETSTAT_SINGLEPLAYER;

	if ( mNetworkOpen )
	{
#ifdef GTA_PC
		mNet.Destroy ();
#endif
		mNetworkOpen = false;
//		LOGOUT ( << "[Disconnected from network]" << endl );
	}

#ifdef GTA_PC
	memset ( &mNet, 0, sizeof(CNetwork) );
//	this->mNet.SetMaxPlayers ( MAX_NUM_PLAYERS );

	CMenuManager::pConnection = NULL;		// Get rid of a prepicked connection type we might have
#endif

};


///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::StartGameAsServer ( void )
{
#ifdef GTA_PC
	assert(FrontEndMenuManager.pConnection);
#endif

	OpenNetwork();		// Make sure network is open
//	DestroyClients ();
//	DestroyServer ();
//	CreateServer ( "Test server" );
//	CreateClient ( "Test client on server machine" );
#ifdef GTA_PC
	mNet.Connect ( FrontEndMenuManager.pConnection );
	mNet.EnumSessions();	// Do we have to do this ?

	// STUFF: Host a new session on this service provider
	if ( !CGameNet::mNet.HostSession ( CMenuManager::m_GameName /*szSessionName*/, CNetGames::GameType, CNetGames::GameMap, "obr"/*szPlayerName*/ ) )
	{
		OutputDebugString("HostSession is nae working\n");
		assert(0);
	}
#endif

	NetWorkStatus = NETSTAT_SERVER;

	// Mark player 0 as being controlled by the server.
#ifdef GTA_PC
	CWorld::Players[0].DpID = DPID_SERVERPLAYER;
#endif
	strcpy(CWorld::Players[0].Name, FrontEndMenuManager.m_PlayerName);
	CWorld::Players[0].ColourR = FrontEndMenuManager.m_PrefsPlayerRed;
	CWorld::Players[0].ColourG = FrontEndMenuManager.m_PrefsPlayerGreen;
	CWorld::Players[0].ColourB = FrontEndMenuManager.m_PrefsPlayerBlue;
	CWorld::Players[0].bInUse = true;

	CNetGames::Init_ServerAtBeginOfGame();

}


///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::StartGameAsClient ( void )
{
//	OpenNetwork();		// Make sure network is open
	ASSERT(mNetworkOpen);	// Make sure network is open

#ifdef GTA_PC
	mNet.Connect ( FrontEndMenuManager.pConnection );

	if ( !CGameNet::mNet.JoinSession ( &FrontEndMenuManager.m_GUIDWeWantToJoin, &CNetGames::GameType, &CNetGames::GameMap,CMenuManager::m_PlayerName ) )
	{
		assert(0);
	}
	NetWorkStatus = NETSTAT_CLIENTSTARTINGUP;
	SyncsDone = 0;

	CNetGames::Init_ClientAtBeginOfGame();
#endif

	CWorld::Players[0].TimeOfLastMessageReceived = CTimer::GetTimeInMilliseconds();
	
	// For now just pick a random name and colour.
	FrontEndMenuManager.m_PrefsPlayerRed = CGeneral::GetRandomNumber() & 255;
	FrontEndMenuManager.m_PrefsPlayerGreen = CGeneral::GetRandomNumber() & 255;
	FrontEndMenuManager.m_PrefsPlayerBlue = CGeneral::GetRandomNumber() & 255;
	switch (CGeneral::GetRandomNumber() & 7)
	{
		case 0: strncpy(FrontEndMenuManager.m_PlayerName, "unknown soldier", NET_MAX_NAME_SIZE); break;
		case 1: strncpy(FrontEndMenuManager.m_PlayerName, "tulip", NET_MAX_NAME_SIZE); break;
		case 2: strncpy(FrontEndMenuManager.m_PlayerName, "cheese head", NET_MAX_NAME_SIZE); break;
		case 3: strncpy(FrontEndMenuManager.m_PlayerName, "clogboy", NET_MAX_NAME_SIZE); break;
		case 4: strncpy(FrontEndMenuManager.m_PlayerName, "wanka", NET_MAX_NAME_SIZE); break;
		case 5: strncpy(FrontEndMenuManager.m_PlayerName, "joker", NET_MAX_NAME_SIZE); break;
		case 6: strncpy(FrontEndMenuManager.m_PlayerName, "moron", NET_MAX_NAME_SIZE); break;
		default: strncpy(FrontEndMenuManager.m_PlayerName, "loser", NET_MAX_NAME_SIZE); break;
	}

}






///////////////////////////////////////////////////////////////////////////
// This bit makes the client send Sync messages once in a while.
// The actual syncing takes place in the message handler. (After the
// message has been bounced back by the server.
///////////////////////////////////////////////////////////////////////////

void CGameNet::DoSyncThing(void)
{
	static UInt32				LastSyncMessage = 0;
	CMsgPeriodicSync			Msg_Sync;

	if (CTimer::GetTimeInMilliseconds() - LastSyncMessage > SNC_NEWSAMPLEEVERY)
	{
		// Now send a sync message to the server
		Msg_Sync.LocalTimeOnSend = CTimer::GetTimeInMilliseconds();
		Msg_Sync.PlayerNumberOfClient = CWorld::PlayerInFocus;
		SendMessageToServer ( &Msg_Sync, sizeof(Msg_Sync), 0 );
		LastSyncMessage = CTimer::GetTimeInMilliseconds();
	}

}







//////////////////////////////////////////////////////////////////////
// FUNCTION : PickPreferedConnectionType
// FUNCTION : If the connection type has not been selected we pick TCP/IP
//////////////////////////////////////////////////////////////////////

#ifdef GTA_PC
void CGameNet::PickPreferedConnectionType(CConnection **ppConnection)
{
	Int32	Count;

	// If we already have a connection selected we're happy. If not we pick the TCP/IP. (if it's there)
	if (!(*ppConnection))
	{
		CGameNet::OpenNetwork();		// This should already be the case. Just to make sure though.

		CGameNet::mNet.EnumConnections ();

		for (Count = 0; Count < CGameNet::mNet.mAvailableConnections.mCount; Count++)
		{
			if (CGameNet::mNet.mAvailableConnections.maConnections[Count].IsTCPIP())
			{		// GREAT. select this one.
				(*ppConnection) = &(CGameNet::mNet.mAvailableConnections.maConnections[Count]);
			}
		}
	}
}
#endif







// These 2 functions translate pointers into indices so that they can be send across the network to other compus
// Also take the 7bit code from the pool into account so that we can make sure the entity hasn't died/regenerated

// packed as follows

//  0  0  0  0  0  0  0  0 - 0  0  0  0  0  0  0  0 - 0  0  0  0  0  0  0  0 - 0  0  0  0  0  0  0  0
//     I     N     D     E      X   (In pool array)   Flags (pool)                   IVTYPE_NONE

enum { IVTYPE_NONE = 0, IVTYPE_PED, IVTYPE_VEHICLE, IVTYPE_BUILDING, IVTYPE_TREADABLE, IVTYPE_OBJECT, IVTYPE_DUMMY };

UInt32 CGameNet::FindIndexValueForPointer ( class CEntity *pEntity )
{
	if (!pEntity) return (IVTYPE_NONE);

	if (pEntity->GetIsTypePed())
	{
		return(IVTYPE_PED | (CPools::GetPedPool().GetIndex((CPed *)pEntity) << 8));
	}
	else if (pEntity->GetIsTypeVehicle())
	{
		return(IVTYPE_VEHICLE | (CPools::GetVehiclePool().GetIndex((CVehicle *)pEntity) << 8));
	}
	else if (pEntity->GetIsTypeObject())
	{
		return(IVTYPE_OBJECT | (CPools::GetObjectPool().GetIndex((CObject *)pEntity) << 8));
	}
	else if (pEntity->GetIsTypeDummy())
	{
		return(IVTYPE_DUMMY | (CPools::GetDummyPool().GetIndex((CDummy *)pEntity) << 8));
	}
	else if (pEntity->GetIsTypeBuilding())
	{
//		if (((CBuilding *)pEntity)->GetIsATreadable())
//		{
//			return(IVTYPE_TREADABLE | (CPools::GetTreadablePool().GetIndex((CTreadable *)pEntity) << 8));
//		}
//		else
		{
			return(IVTYPE_BUILDING | (CPools::GetBuildingPool().GetIndex((CBuilding *)pEntity) << 8));
		}
	}

	ASSERT(0);	// BAAAAAD pointer
	return (0);
}


CEntity *CGameNet::FindPointerForIndexValue ( UInt32 IndexVal )
{
	switch(IndexVal & 7)
	{
		case IVTYPE_NONE:
			return NULL;
			break;

		case IVTYPE_PED:
			return (CPools::GetPedPool().GetAt(IndexVal>>8));
			break;

		case IVTYPE_VEHICLE:
			return (CPools::GetVehiclePool().GetAt(IndexVal>>8));
			break;

		case IVTYPE_OBJECT:
			return (CPools::GetObjectPool().GetAt(IndexVal>>8));
			break;

		case IVTYPE_DUMMY:
			return (CPools::GetDummyPool().GetAt(IndexVal>>8));
			break;

//		case IVTYPE_TREADABLE:
//			return (CPools::GetTreadablePool().GetAt(IndexVal>>8));
//			break;

		case IVTYPE_BUILDING:
			return (CPools::GetBuildingPool().GetAt(IndexVal>>8));
			break;

	}
	ASSERT(0);	// Nae right
	return NULL;
}



CEntity *CGameNet::FindPointerForIndexValue_DontTestFlags ( UInt32 IndexVal )
{
	switch(IndexVal & 7)
	{
		case IVTYPE_NONE:
			return NULL;
			break;

		case IVTYPE_PED:
			return (CPools::GetPedPool().GetSlot(IndexVal>>16));
			break;

		case IVTYPE_VEHICLE:
			return (CPools::GetVehiclePool().GetSlot(IndexVal>>16));
			break;

		case IVTYPE_OBJECT:
			return (CPools::GetObjectPool().GetSlot(IndexVal>>16));
			break;

		case IVTYPE_DUMMY:
			return (CPools::GetDummyPool().GetSlot(IndexVal>>16));
			break;

//		case IVTYPE_TREADABLE:
//			return (CPools::GetTreadablePool().GetSlot(IndexVal>>16));
//			break;

		case IVTYPE_BUILDING:
			return (CPools::GetBuildingPool().GetSlot(IndexVal>>16));
			break;
	}
	ASSERT(0);	// Nae right
	return NULL;
}


Int32 CGameNet::FindIndexForIndexValue(UInt32 IndexVal)
{
	return(IndexVal>>16);

}


// Will set the 8 bits flags in the pool so that we can work out whether an object has been
// removed/added.

void CGameNet::SetPoolFlags ( UInt32 IndexVal )
{
	switch(IndexVal & 7)
	{
		case IVTYPE_NONE:
			return;
			break;

		case IVTYPE_PED:
			return (CPools::GetPedPool().SetFlagsValue(IndexVal>>16, IndexVal>>8));
			break;

		case IVTYPE_VEHICLE:
			return (CPools::GetVehiclePool().SetFlagsValue(IndexVal>>16, IndexVal>>8));
			break;

		case IVTYPE_OBJECT:
			return (CPools::GetObjectPool().SetFlagsValue(IndexVal>>16, IndexVal>>8));
			break;

		case IVTYPE_DUMMY:
			return (CPools::GetDummyPool().SetFlagsValue(IndexVal>>16, IndexVal>>8));
			break;

//		case IVTYPE_TREADABLE:
//			return (CPools::GetTreadablePool().SetFlagsValue(IndexVal>>16, IndexVal>>8));
//			break;

		case IVTYPE_BUILDING:
			return (CPools::GetBuildingPool().SetFlagsValue(IndexVal>>16, IndexVal>>8));
			break;

	}
	ASSERT(0);	// Nae right
}






//////////////////////////////////////////////////////////////////////
// FUNCTION : SendRequestToEnterCar
// FUNCTION : Sends a request to figure out whether we can enter this car
//////////////////////////////////////////////////////////////////////

/*
void CGameNet::SendRequestToEnterCar ( CVehicle *pTargetVehicle )
{
	CMsgRequestToEnterCar	Msg;
	
	Msg.PlayerRequesting = CWorld::PlayerInFocus;
	Msg.CarToBeEntered = FindIndexValueForPointer(pTargetVehicle);
	
	mNet.SendMessageToServer ( &Msg, sizeof(CMsgRequestToEnterCar) );
}
*/

//////////////////////////////////////////////////////////////////////
// FUNCTION : BuildChatMessage
// FUNCTION : Do the stuff needed to talk to construct a message (chat)
//////////////////////////////////////////////////////////////////////

char	ChatString[BR_CHARSINTEXTSTRING];
bool	bBuildingChatString = false;

void CGameNet::BuildChatMessage (  )
{
#ifdef GTA_NETWORK
	if (CGameNet::NetWorkStatus == NETSTAT_SINGLEPLAYER)
	{
		bBuildingChatString = false;
		return;
	}


	if (!bBuildingChatString)
	{
		if ( (CPad::NewKeyState.kAscii['t'] && !CPad::OldKeyState.kAscii['t']) ||
			 (CPad::NewKeyState.kAscii['T'] && !CPad::OldKeyState.kAscii['T']) )
		{
			bBuildingChatString = true;
			ChatString[0] = 0;
		}
	}
	else
	{		// We're already editting a string
		if (CPad::EditString(ChatString, BR_CHARSINTEXTSTRING - NET_MAX_NAME_SIZE - 3))
		{
		}
		else
		{	// Done building string. Send it away.
			bBuildingChatString = false;

			if (ChatString[0] == 0) return;	// empty string. Don't bother.

			CMsgText	Msg;
			
			sprintf(Msg.String, "%s: %s", CWorld::Players[CWorld::PlayerInFocus].Name, ChatString);
			Msg.ColourR = (256 + CWorld::Players[CWorld::PlayerInFocus].ColourR) / 2;
			Msg.ColourG = (256 + CWorld::Players[CWorld::PlayerInFocus].ColourG) / 2;
			Msg.ColourB = (256 + CWorld::Players[CWorld::PlayerInFocus].ColourB) / 2;
			
			if ( CGameNet::NetWorkStatus == NETSTAT_SERVER )
			{
				CGameNet::mNet.SendMessageToAllClients ( &Msg, sizeof(CMsgText), 300 );
				Msg.Get();	// print on our own screen as well
			}
			else if ( CGameNet::NetWorkStatus == NETSTAT_CLIENTSTARTINGUP || CGameNet::NetWorkStatus == NETSTAT_CLIENTRUNNING )
			{
				SendMessageToServer ( &Msg, sizeof(CMsgText), 200 );
			}
			else
			{	// Print it for debug purposes
				Msg.Get();
			}			
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// FUNCTION : PrintNetworkMessagesToScreen
// FUNCTION : Prints out things like the players, playernames, number of points etc to screen
//////////////////////////////////////////////////////////////////////

UInt32	PrintScoresTriggered;

void CGameNet::PrintNetworkMessagesToScreen (  )
{
//#ifdef GTA_NETWORK

	Int32	C, NumPrinted = 0;

	if (!CHud::m_Wants_To_Draw_Hud) return;

	BuildChatMessage();

	CFont::SetProportional(TRUE);
	CFont::SetBackground(FALSE);
	CFont::SetScale( 0.8f , 0.8f); // was .8
//	CFont::SetCentreOff();
//	CFont::SetRightJustifyOff();
//	CFont::SetJustifyOn();
	CFont::SetRightJustifyWrap ( 0 );
//	CFont::SetBackGroundOnlyTextOff( );
	CFont::SetFontStyle( FO_FONT_STYLE_STANDARD );	// or bank/heading perhaps
//	CFont::SetPropOff();
	CFont::SetColor(CRGBA(50,255,50,230));
	CFont::SetWrapx(SCREEN_WIDTH);//SCREEN_WIDTH);

/*
	if (NetWorkStatus != NETSTAT_SINGLEPLAYER)
	{
		for (C = 0; C < NW_MAXNUMPLAYERS; C++)
		{
			if (CWorld::Players[C].bInUse)
			{
				sprintf(gString, "(%d) %s PdID:%d Pts:%d\n",
						C, CWorld::Players[C].Name, CWorld::Players[C].DpID, CWorld::Players[C].Points );
		
				AsciiToUnicode(gString, gUString);

				CFont::SetColor(CRGBA(CWorld::Players[C].ColourR, CWorld::Players[C].ColourG, CWorld::Players[C].ColourB, 255));
				CFont::PrintString( 10.0f, 10.0f + NumPrinted * 12.0f, gUString); 
	
				NumPrinted++;
			}
		}		
	}
*/

	// Print the players in a ranking type fashion. (ordered)
//	if (CPad::NewKeyState.kTAB || (CTimer::GetTimeInMilliseconds() - PrintScoresTriggered < 6000))
	{
		Int32	Highest = -1, HighestPlayer = 0;
		bool	bAlreadyPrinted[MAX_NUM_PLAYERS];
		
		for (C = 0; C < MAX_NUM_PLAYERS; C++)
		{
			bAlreadyPrinted[C] = false;
		}
		
		if (NetWorkStatus != NETSTAT_SINGLEPLAYER)
		{
			while (HighestPlayer >= 0)
			{
				HighestPlayer = -1;
				Highest = -1;
			
				for (C = 0; C < MAX_NUM_PLAYERS; C++)
				{
					if (CWorld::Players[C].bInUse && !bAlreadyPrinted[C])
					{
						if (CWorld::Players[C].Points > Highest)
						{
							HighestPlayer = C;
							Highest = CWorld::Players[C].Points;
						}
					}
				}
			
				if (HighestPlayer >= 0)
				{

					if (CNetGames::GameType == GAMETYPE_RATRACE)
					{
						sprintf(gString, "%s :%d (%d)", CWorld::Players[HighestPlayer].Name, CWorld::Players[HighestPlayer].Points, CWorld::Players[HighestPlayer].Points2 );
					}
					else
					{
						sprintf(gString, "%s :%d", CWorld::Players[HighestPlayer].Name, CWorld::Players[HighestPlayer].Points );
					}
				
					AsciiToGxtChar(gString, gGxtString);

					Int32	R, G, B;	
					CNetGames::FindPlayerMarkerColour(HighestPlayer, &R, &G, &B);

					if (HighestPlayer == CWorld::PlayerInFocus)
					{		// Make this guy flash a bit (it's us)
						float	Mult = 1.0f + 0.2f * CMaths::Sin( (CTimer::GetTimeInMilliseconds() & 1023) * (6.28/1024.0f));
						R = MIN(R*Mult, 255);
						G = MIN(G*Mult, 255);
						B = MIN(B*Mult, 255);
					}


					CFont::SetColor(CRGBA( R, G, B, 255));

					CFont::PrintString( SCREEN_WIDTH * 0.8f, 10.0f + (SCREEN_HEIGHT * 0.4f) + NumPrinted * 12.0f, gGxtString);
		//CFont::PrintString( SCREEN_WIDTH - 110.0f, 10.0f + NumPrinted * 12.0f, gUString); 
					NumPrinted++;
					bAlreadyPrinted[HighestPlayer] = true;
				}
			}
		}
	}

/*
	Int32	Lowest = 999999, LowestPlayer = 0;
	bool	bAlreadyPrinted[NW_MAXNUMPLAYERS];
	
	for (C = 0; C < NW_MAXNUMPLAYERS; C++)
	{
		bAlreadyPrinted[C] = false;
	}
	
	if (NetWorkStatus != NETSTAT_SINGLEPLAYER)
	{
		while (LowestPlayer >= 0)
		{
			LowestPlayer = -1;
			Lowest = 999999;
		
			for (C = 0; C < NW_MAXNUMPLAYERS; C++)
			{
				if (CWorld::Players[C].bInUse && !bAlreadyPrinted[C])
				{
					if (CWorld::Players[C].Points < Lowest)
					{
						LowestPlayer = C;
						Lowest = CWorld::Players[C].Points;
					}
				}
			}
		
			if (LowestPlayer >= 0)
			{
				sprintf(gString, "%s :%d\n",
						CWorld::Players[LowestPlayer].Name, CWorld::Players[LowestPlayer].Points );
			
				AsciiToUnicode(gString, gUString);

				Int32	R, G, B;	
				CNetGames::FindPlayerMarkerColour(LowestPlayer, &R, &G, &B);

				CFont::SetColor(CRGBA( R, G, B, 255));
				CFont::PrintString( 10.0f, 10.0f + NumPrinted * 12.0f, gUString); 
				NumPrinted++;
				bAlreadyPrinted[LowestPlayer] = true;
			}
		}
	}
*/



	// Print out the coordinates of the player for now

	// To test the clock we change the colour of this message
/*
	{
		Int32	C;
		
		C = (CTimer::GetTimeInMilliseconds()>>11) & 7;
		
		switch (C)
		{
			case 0:
				CFont::SetColor(CRGBA(255,0,0,255));
				break;
			case 1:
				CFont::SetColor(CRGBA(0,255,0,255));
				break;
			case 2:
				CFont::SetColor(CRGBA(0,0,255, 255));
				break;
			case 3:
				CFont::SetColor(CRGBA(255,0,255,255));
				break;
			case 4:
				CFont::SetColor(CRGBA(255,255,0,255));
				break;
			case 5:
				CFont::SetColor(CRGBA(0,255,255,255));
				break;
			case 6:
				CFont::SetColor(CRGBA(255,255,255,255));
				break;
			case 7:
				CFont::SetColor(CRGBA(0,0,0,255));
				break;
		}
	}

	sprintf(gString, "%f %f %f %d", FindPlayerCoors().x, FindPlayerCoors().y, FindPlayerCoors().z,
					(CTimer::GetTimeInMilliseconds()>>10) & 63);
	AsciiToUnicode(gString, gUString);
#ifdef _DEBUG
	CFont::PrintString( 10.0f, SCREEN_HEIGHT - 20.0f, gUString); 
#endif	
*/

	// Do the stuff needed to talk to construct a message (chat)
	if (bBuildingChatString)
	{
		CFont::SetColor(CRGBA(255,255,255,200));
		strcpy(gString, ChatString);
		if (CTimer::GetTimeInMilliseconds() & 512)
		{
			strcat(gString, "-");
		}
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString( 10.0f, SCREEN_HEIGHT - 40.0f, gGxtString);
	}


		// Print out the team points for each team (if we're in a team game that is)
	if (CGameNet::NetWorkStatus != NETSTAT_SINGLEPLAYER)
	{
		if (CNetGames::TeamGameGoingOn())
		{
			Int32	Team;
			
			CFont::SetScale(0.8f, 0.8f);
			for (Team = 0; Team < NUM_TEAMS;Team++)
			{
				CFont::SetColor(CRGBA(TeamColours[Team * 3],TeamColours[Team * 3 + 1],TeamColours[Team * 3 + 2],255));
				if (CNetGames::GameType == GAMETYPE_DOMINATION)
				{
					sprintf(gString, "Team%d:%d", Team, CNetGames::TeamPoints[Team]/1000 );
				}
				else
				{
					sprintf(gString, "Team%d:%d", Team, CNetGames::TeamPoints[Team] );
				}
				
				AsciiToGxtChar(gString, gGxtString);
				CFont::PrintString( SCREEN_WIDTH - 160.0f, 110.0f + Team * 20.0f, gGxtString);
			}
		}
		else if (CNetGames::GameType == GAMETYPE_RATRACE)
		{
			Int32	TimeToGo = 59 - (CTimer::GetTimeInMilliseconds() % 60000) / 1000;

			CFont::SetScale(2.0f, 2.0f);
			CFont::SetColor(CRGBA(255, 255, 128,255));
			sprintf(gString, "%d", TimeToGo );
			AsciiToGxtChar(gString, gGxtString);
			CFont::PrintString( SCREEN_WIDTH - 150.0f, 110.0f, gGxtString);
		}
	}


	
		// Just for debug purposes print a message to show whether we are the server or a client
	if (CGameNet::NetWorkStatus == NETSTAT_SERVER)
	{
		CFont::SetColor(CRGBA(255,255,255,255));
		CFont::SetScale( 0.8f , 0.8f);
		sprintf(gString, "Server %.1f", CTimer::ms_fTimeStepNonClipped );
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString( SCREEN_WIDTH - 110.0f, 10.0f, gGxtString);
	}
	else if (CGameNet::NetWorkStatus == NETSTAT_CLIENTSTARTINGUP || CGameNet::NetWorkStatus == NETSTAT_CLIENTRUNNING)
	{
		CFont::SetColor(CRGBA(255,255,255,255));
		CFont::SetScale( 0.8f , 0.8f);
		sprintf(gString, "Client %.1f", CTimer::ms_fTimeStepNonClipped );
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString( SCREEN_WIDTH - 110.0f, 10.0f, gGxtString);
	}
//#endif
}

void CGameNet::PrintScoresForAWhile (  )
{
	PrintScoresTriggered = CTimer::GetTimeInMilliseconds();
}


///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::PlayerPedHasBeenKilledInNetworkGame (const CPed *pPed)
{
	Int32	Player = 0;

	// Find the player that this ped belongs to
	while (Player < MAX_NUM_PLAYERS && CWorld::Players[Player].pPed != pPed)
	{
		Player++;
	}

	if (Player < MAX_NUM_PLAYERS)
	{		// We found the player. Change his status.
		if (CWorld::Players[Player].PlayerState == CPlayerInfo::PLAYERSTATE_PLAYING)
		{

			// Activate logic to revive player.
			CWorld::Players[Player].PlayerState = CPlayerInfo::PLAYERSTATE_HASDIED;
			CWorld::Players[Player].TimeOfLastEvent = CTimer::GetTimeInMilliseconds();
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// This function gets called by the taskmanager when a task gets deleted on the server.
// This function is responsible for handing control back to the clients after a
// enter/exit car procedure.
///////////////////////////////////////////////////////////////////////////

void CTaskManager::DeleteTaskOnServerMachine(CTask* pTask)  // Called when task gets deleted (lives in gamenet.cpp)
{
	switch (pTask->GetTaskType())
	{
		case CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER:
			{
				CTaskComplexEnterCarAsDriver *pTaskEnterCar;
				pTaskEnterCar = (CTaskComplexEnterCarAsDriver *) pTask;
				
				if (m_pPed->bControlledByServer)
				{
					m_pPed->bControlledByServer = false;
					m_pPed->ForceUpdateToClient();
				}
				if(pTaskEnterCar->GetTargetVehicle())
				{
					pTaskEnterCar->GetTargetVehicle()->bControlledByServer = false;
					pTaskEnterCar->GetTargetVehicle()->ForceUpdateToClient();
				}
			}
			break;
			
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////
// This function will get rid of a player in a network game. Should be called
// after a time out or when the player tells the server he quits.
///////////////////////////////////////////////////////////////////////////

void CGameNet::ThrowPlayerOutOfGame (Int32 Player)
{
	ASSERT(CGameNet::NetWorkStatus == NETSTAT_SERVER);	// Only the server should call this.
	ASSERT(CWorld::Players[Player].bInUse == true);		// Make sure there is a player there in the first place.
	
	
	// If this player is in a vehicle we have to warp him outside.
	if (CWorld::Players[Player].pPed->m_bInVehicle && CWorld::Players[Player].pPed->m_pMyVehicle)
	{
		if (CWorld::Players[Player].pPed->m_bInVehicle)
		{
			CWorld::Remove(CWorld::Players[Player].pPed->m_pMyVehicle);
			delete(CWorld::Players[Player].pPed->m_pMyVehicle);
		}
	}

	// Remove the player ped.
	if (CWorld::Players[Player].pPed)
	{
		CWorld::Remove(CWorld::Players[Player].pPed);
		delete CWorld::Players[Player].pPed;
		CWorld::Players[Player].pPed = NULL;
	}

	// Remove radar blip
	if (CWorld::Players[Player].RadarBlip)
	{
		CRadar::ClearBlip(CWorld::Players[Player].RadarBlip);
		CWorld::Players[Player].RadarBlip = 0;
	}

	// Print info on the console and tell other players about it.
	sprintf(gString, "%s has left the game", CWorld::Players[Player].Name);
	SendTextMessageToEveryone(gString);
	
	strcpy(CWorld::Players[Player].Name, "Player left");
	CWorld::Players[Player].bInUse = false;
}


///////////////////////////////////////////////////////////////////////////
// If we are the server a message is send to all clients. If we are a client
// a message is sent to the server (who will bounce it back to all clients)
///////////////////////////////////////////////////////////////////////////

void CGameNet::SendTextMessageToEveryone ( char *pString, UInt8 R, UInt8 G, UInt8 B )
{
	CMsgText	Msg;
	
	strncpy(Msg.String, pString, BR_CHARSINTEXTSTRING);

	Msg.ColourR = R;
	Msg.ColourG = G;
	Msg.ColourB = B;
			
	if ( CGameNet::NetWorkStatus == NETSTAT_SERVER )
	{
		SendMessageToAllClients ( &Msg, sizeof(CMsgText), 300 );
		Msg.Get();	// print on our own screen as well
	}
	else if ( CGameNet::NetWorkStatus == NETSTAT_CLIENTSTARTINGUP || CGameNet::NetWorkStatus == NETSTAT_CLIENTRUNNING )
	{
		SendMessageToServer ( &Msg, sizeof(CMsgText), 200 );
	}
	else
	{
		ASSERT(0);	// What the fuck is going on?
	}
}


///////////////////////////////////////////////////////////////////////////
// This function does everything necessary to go to single player game.
///////////////////////////////////////////////////////////////////////////

void CGameNet::SwitchToSinglePlayerGame ( void )
{
	Int32	C;

DEBUGLOG("SwitchToSinglePlayerGame\n");

		// Get rid of the other players
	for (C = 0; C < MAX_NUM_PLAYERS; C++)
	{
		if (C != CWorld::PlayerInFocus && CWorld::Players[C].bInUse)
		{
			strcpy(CWorld::Players[C].Name, "Player gone");
			CWorld::Players[C].bInUse = false;
#ifdef GTA_PC
			CWorld::Players[C].DpID = 0;
#endif
			if (CWorld::Players[C].RadarBlip)
			{
				CRadar::ClearBlip(CWorld::Players[C].RadarBlip);
				CWorld::Players[C].RadarBlip = 0;
			}
			if (CWorld::Players[C].pPed)
			{
				if (CWorld::Players[C].pPed->m_bInVehicle)
				{
					CWorld::Remove(CWorld::Players[C].pPed->m_pMyVehicle);
					delete(CWorld::Players[C].pPed->m_pMyVehicle);
				}
				if (CWorld::Players[C].pPed)
				{
					CWorld::Remove(CWorld::Players[C].pPed);
					delete(CWorld::Players[C].pPed);
					CWorld::Players[C].pPed = NULL;
				}
			}		
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////

void CGameNet::SendMessageToClient ( Int32 ClientPlayer, void *pMsg, UInt32 Size, UInt32 MaxDelay )
{
ASSERT(CGameNet::NetWorkStatus == NETSTAT_SERVER);

#ifdef GTA_PC
	mNet.SendMessageToClient ( ClientPlayer, pMsg, Size );
#endif
#ifdef GTA_PS2
	Network_PS2::SendMessageToClient(ClientPlayer, pMsg, Size, MaxDelay);
#endif

}



///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void CGameNet::SendMessageToAllClients ( void *pMsg, UInt32 Size, UInt32 MaxDelay )
{
ASSERT(CGameNet::NetWorkStatus == NETSTAT_SERVER);

#ifdef GTA_PC
	mNet.SendMessageToAllClients ( ClientPlayer, pMsg, Size );
#endif
#ifdef GTA_PS2
	Network_PS2::SendMessageToAllClients(pMsg, Size, -1, MaxDelay);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void CGameNet::SendMessageToAllClientsBarOne ( void *pMsg, UInt32 Size, Int32 BarThisOne, UInt32 MaxDelay )
{
ASSERT(CGameNet::NetWorkStatus == NETSTAT_SERVER);

#ifdef GTA_PC
	mNet.SendMessageToAllClients ( ClientPlayer, pMsg, Size );
#endif
#ifdef GTA_PS2
	Network_PS2::SendMessageToAllClients(pMsg, Size, BarThisOne, MaxDelay);
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void CGameNet::SendMessageToServer ( void *pMsg, UInt32 Size, UInt32 MaxDelay )
{
ASSERT(CGameNet::NetWorkStatus == NETSTAT_CLIENTRUNNING || CGameNet::NetWorkStatus == NETSTAT_CLIENTSTARTINGUP);

#ifdef GTA_PC
	mNet.SendMessageToServer ( pMsg, Size );
#endif
#ifdef GTA_PS2
	Network_PS2::SendMessageToServer(pMsg, Size, MaxDelay);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void CGameNet::SendMessageToEveryone ( void *pMsg, UInt32 Size, UInt32 MaxDelay )
{
	if (CGameNet::NetWorkStatus == NETSTAT_SERVER)
	{
		SendMessageToAllClients(pMsg, Size, MaxDelay);
	}
	else
	{
		SendMessageToServer(pMsg, Size, MaxDelay);
	}
}





#endif

