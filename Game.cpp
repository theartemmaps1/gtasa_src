// Title	:	Game.cpp
// Author	:	G. Speirs
// Started	:	13/07/99


//!PC - need to include windows.h before win.h it seems...
#if defined (GTA_PC)

#ifndef OSW
	#include "windows.h"
#endif

	#include "win.h"
	#include "loadingScreen.h"
#endif //GTA_PC

// RW headers
//$DW$#include <rwcore.h>
//$DW$#include <rpskin.h>
#include "rwinternal.h"
// Game headers
#include "skeleton.h"
#include "rwcam.h"
#include "lights.h"
#include "carctrl.h"
#include "Game.h"
#include "Main.h"
#include "World.h"
#include "Vector.h"
#include "Draw.h"
#include "ModelInfo.h"
#include "Sprite.h"
#include "TempColModels.h"
#include "Collision.h"
#include "automobile.h"
#include "Camera.h"
#include "Clock.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "CivilianPed.h"
#include "CopPed.h"
#include "DummyPed.h"
#include "PedType.h"
#include "Range2D.h"
#include "Vector2D.h"
#include "Pad.h"
#include "PathFind.h"
#include "WeaponInfo.h"
#include "ShotInfo.h"
#include "ProjectileInfo.h"
#include "BulletInfo.h"
#include "Explosion.h"
#include "Profile.h"
#include "General.h"
#include "TimeCycle.h"
#include "FileMgr.h"
#include "script.h"
#include "messages.h"
#include "HandlingMgr.h"
#include "Projectile.h"
#include "ObjectData.h"
//#include "SurfaceTable.h"
#include "population.h"
#include "MemInfo.h"
#include "record.h"
#include "boat.h"
#include "roadblocks.h"
#include "WaterLevel.h"
#include "cargen.h"
#include "pedgen.h"
#include "trafficlights.h"
#include "user.h"
#include "FileLoader.h"
#include "AnimManager.h"
#include "Building.h"
#include "setup.h"
#include "coronas.h"
#include "Particle.h"
#include "ParticleObject.h"
#include "Phones.h"
#include "Accident.h"
#include "Fire.h"
#include "radar.h"
#include "restart.h"
#include "shadows.h" 
#include "gamelogic.h" 
#include "WeaponEffects.h" 
#include "hud.h" 
#include "font.h"
#include "train.h"
#include "weather.h"
#include "streaming.h"
#include "skidmarks.h"
//#include "PedRoutes.h"
#include "cranes.h"
#include "darkel.h"
#include "stats.h"
#include "fluff.h"
#include "pickups.h"
#include "garages.h"
#include "entryexits.h"
#include "stuntjump.h"
#include "rubbish.h"
#include "modelindices.h"
#include "references.h"
#include "clouds.h"
#include "specialfx.h"
#include "heli.h"
#include "remote.h"
#include "watercannon.h"
#include "txdstore.h"
#include "txdload.h"
#include "renderer.h"
#include "CutsceneMgr.h"
#include "gangs.h"
#include "zonecull.h"
#include "Planes.h"
#include "glass.h"
#include "handy.h"
#include "bridge.h"
#include "antennas.h"
#include "timebars.h"
#include "cdstream.h"
#include "VisibilityPlugins.h"
#ifdef GTA_PS2
	#include "vu0collision.h"
	#include "ps2all.h"
#endif	
#include "Frontend.h"
#include "AnimBlendHierarchy.h"
#include "AnimBlendSequence.h"
#include "credits.h"
#include "mblur.h"
#include "pad.h"
#include "network_allprojects.h"
#include "console.h"
#include "edit.h"
#include "GenericGameStorage.h"
#include "replay.h"
#include "TexturePool.h"
#include "PlayerSkin.h"
#include "inifile.h"
#include "windmodifiers.h"
#include "varconsole.h"
#include "ropes.h"
#include "setpieces.h"
#include "occlusion.h"
#include "WaterCreatures.h"
#include "overlay.h"
#include "stunts.h"
#include "cheat.h"
#include "lightning.h"
#include "localisation.h"
#include "PedGroup.h"
#include "pedintelligence.h"
#include "PedDebugVisualiser.h"
#include "record.h"
#include "birds.h"
#include "VehicleModelInfo.h"
#include "clothes.h"
#include "conversations.h"
#include "popcycle.h"
#include "audiozones.h"
#include "tag.h"
#include "loadingscreen.h"
#include "MemoryMgr.h"
#ifdef GTA_XBOX
#include "hdstream.h"
#include "PeripheralsXBox.h"
#endif
//!PC - post fx not support yet
#if defined (GTA_PS2)
#include "dma_post_fx.h"
#endif //GTA_PS2

#include "property.h"
#include "QuadTree.h"
#include "Shopping.h"

#ifndef MASTER
#include "ps2keyboard.h"
#endif // MASTER

// -- FX SYSTEM ---------------------------------
#include "fx.h"
// ----------------------------------------------
#include "BreakObject.h"
#include "InteriorManager.h"
#include "ProcObjects.h"
#include "TaskAttack.h"				// need these to clean up tasks for shutdown
#include "TaskBasic.h"
#include "TaskJumpFall.h"
#include "cover.h"
#include "BoneNode.h"
#include "IKChain.h"
#include "Ragdoll.h"
#include "PostEffects.h"
#include "gangwars.h"
#include "menusystem.h"
#include "colaccel.h"
#include "LoadMonitor.h"
#include "EventCounter.h"
#include "RealTimeShadow.h"

#if defined GTA_XBOX// || defined GTA_PC
#include "ShaderManager.h"
#endif

#ifndef FINAL
	#include "scriptdebug.h"
#endif

#include "TouchInterface.h"
#include "WidgetListText.h"
#include "WidgetTapMeter.h"
#include "WidgetThumbCircle.h"


#ifdef GTA_PS2
	#define USE_VU_TURBO_SPRITE		(1)
	#define USE_VU_WATER_RENDERER	(1)

	#ifdef GTA_SA
		#define USE_VU_CARFX_RENDERER	(1)
		#define USE_CVSKINPED_RENDERER	(1)
	#endif
#endif



#ifdef GTA_SA
	#if (defined(GTA_PC) || defined(GTA_XBOX))
#ifndef OSW
		#define USE_VOLUMETRIC_SHADOW_RENDERER	(1)
#endif
		#define USE_PC_CARFX_RENDERER			(1)
	#endif
#endif


#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
	#include "VolumetricShadowMgr.h"
#endif


//!PC - trying to get this working on PC too now...
#define USE_CVBUILDING_RENDERER	(1)
#define USE_PLANT_MGR			(1)
		
#ifdef USE_VU_TURBO_SPRITE
	#include "VuTurboSprite.h"
#endif

#ifdef USE_VU_WATER_RENDERER
	#include "VuWaterRenderer.h"
#endif

#ifdef USE_VU_CARFX_RENDERER
	#include "VuCarFXRenderer.h"
#endif

#ifdef USE_PC_CARFX_RENDERER
	#include "CarFXRenderer.h"
#endif

#if defined (GTA_PC)
	#include "CarFXRenderer.h"
#endif //GTA_PC

#ifdef USE_PLANT_MGR
	#if defined (GTA_PS2)
		#include "PlantsMgr.h"
		#include "VuGrassRenderer.h"
	#else //GTA_PS2
		#include "PC_PlantsMgr.h"
		#include "PC_GrassRenderer.h"
	#endif //GTA_PS2
#endif

#ifdef USE_CVBUILDING_RENDERER
	#if defined (GTA_PS2)
		#include "VuCustomBuildingRenderer.h"
	#else //GTA_PS2
		#include "CustomBuildingRenderer.h"
	#endif //GTA_PS2
#endif

#ifdef USE_CVSKINPED_RENDERER
	#include "VuCustomSkinPedRenderer.h"
#endif


#include "CustomCarPlateMgr.h"
#include "CustomRoadsignMgr.h"

//#ifdef GTA_NETWORK
	#include "netgames.h"
//#endif

#ifdef GTA3_HASP
	#include "CHasp.h"
#endif//GTA3_HASP

#ifdef USE_AUDIOENGINE
	#include "AudioEngine.h"
#endif //USE_AUDIOENGINE

#ifdef GTA_PS2
	extern sceGsDBuffDc sweDb;
//	extern _rwSkyTwoCircuit tcaaDisp;
#endif


// for moving the player coords in VarConsole
#ifndef MASTER
	CVector CGame::PlayerCoords;
	bool8 CGame::VarUpdatePlayerCoords = FALSE;
#endif // MASTER

uint8 CGame::bMissionPackGame = 0;  // stores which mission pack we are currently in on this memory card

eLevelName CGame::currLevel;
eVisibleArea CGame::currArea = AREA_MAIN_MAP;

char CGame::aDatFile[32];

CAutomobile* gpCar = NULL;
CBoat* gpBoat = NULL;
int32 gameTxdSlot;

extern	bool8	g_reportDups;

RwMatrix* CGame::m_pWorkingMatrix1 = NULL;
RwMatrix* CGame::m_pWorkingMatrix2 = NULL;

extern void ReadPlayerCoordsFile();
void ReloadDataFile(const char* pFileName);

//
// name:		InitialiseOnceBeforeRW
// description:	This is game related structures that only needed initialised once. Assume 
//				Renderware hasn't been initialised.
bool CGame::InitialiseOnceBeforeRW()
{
	CMemoryMgr::Init();		//Very important to do first!!!...

	CLocalisation::Initialise();
	CFileMgr::Initialise();
	// Initialise with 5 channels
	// 2 for streaming models, one for music, one for sfx and one for standard file loading
	CdStreamInit(5);
	IOPMEM_DEBUG("CdStreamInit")
	
#ifdef GTA_XBOX
	// init the xbox peripherals	
	g_peripheralsXBox.Init();
#endif

	CPad::Initialise();
	
#ifndef MASTER	
	printf("size of placeable %d\n", sizeof(CPlaceable));
	printf("size of entity %d\n", sizeof(CEntity));
	printf("size of building %d\n", sizeof(CBuilding));
	printf("size of dummy %d\n", sizeof(CDummy));
	printf("size of physical %d\n", sizeof(CPhysical));

	PS2Keyboard.InitKeyboard();  // init the USB keyboard
#endif	
	
	return true;
}

#define MAX_ATOMICS			1800
#define MAX_CLUMPS			80
#define MAX_FRAMES			2600
#define MAX_GEOMETRY		850
#define MAX_TEXDICTIONARIES	121
#define MAX_TEXTURES		1700
#define MAX_RASTERS			1707
#define MAX_MATERIALS		2600

static void PreAllocateRwObjects()
{
	CMemInfo::EnterBlock(PREALLOC_MEM_ID);
	
	int32 i=0;
	void** pPtrArray = new void*[8192];
	extern bool preAlloc;
	
	preAlloc = true;
	
	for(i=0; i<MAX_ATOMICS; i++)
		pPtrArray[i] = RpAtomicCreate();
	for(i=0; i<MAX_ATOMICS; i++)
		 RpAtomicDestroy((RpAtomic*)pPtrArray[i]);
		 
	for(i=0; i<MAX_CLUMPS; i++)
		pPtrArray[i] = RpClumpCreate();
	for(i=0; i<MAX_CLUMPS; i++)
		RpClumpDestroy((RpClump*)pPtrArray[i]);
		
	for(i=0; i<MAX_FRAMES; i++)
		pPtrArray[i] = RwFrameCreate();
	for(i=0; i<MAX_FRAMES; i++)
		RwFrameDestroy((RwFrame*)pPtrArray[i]);

	for(i=0; i<MAX_GEOMETRY; i++)
		pPtrArray[i] = RpGeometryCreate(0,0,0);
	for(i=0; i<MAX_GEOMETRY; i++)
		RpGeometryDestroy((RpGeometry*)pPtrArray[i]);

	for(i=0; i<MAX_TEXDICTIONARIES; i++)
		pPtrArray[i] = RwTexDictionaryCreate();
	for(i=0; i<MAX_TEXDICTIONARIES; i++)
		RwTexDictionaryDestroy((RwTexDictionary*)pPtrArray[i]);

	for(i=0; i<MAX_TEXTURES; i++)
	{
		RwRaster* pRaster = RwRasterCreate(0,0,0,rwRASTERTYPETEXTURE);
		pPtrArray[i] = RwTextureCreate(pRaster);
	}	
	for(i=0; i<MAX_TEXTURES; i++)
		RwTextureDestroy((RwTexture*)pPtrArray[i]);

	for(i=0; i<MAX_MATERIALS; i++)
		pPtrArray[i] = RpMaterialCreate();
	for(i=0; i<MAX_MATERIALS; i++)
		RpMaterialDestroy((RpMaterial*)pPtrArray[i]);
		
	delete[] pPtrArray;

	preAlloc = false;
	
	CMemInfo::ExitBlock();
}

//
// name:		ShutdownRenderWare
// description:	Remove all RW related data
void CGame::ShutdownRenderWare()
{
	int32 i;
	
	RwMatrixDestroy(m_pWorkingMatrix1);
	RwMatrixDestroy(m_pWorkingMatrix2);

//	CMBlur::MotionBlurClose();
	CLoadingScreen::Shutdown();
	CHud::Shutdown();
	CFont::Shutdown();
	
	for (i = 0; i < MAX_NUM_PLAYERS; i++)
	{
		CWorld::Players[i].DeletePlayerSkin();
	}
	CPlayerSkin::Shutdown();
	DestroyDebugFont();
	LightsDestroy(Scene.world);
	
	CVisibilityPlugins::Shutdown();

	// destroy camera and world
	RpWorldRemoveCamera(Scene.world, Scene.camera);
	
	RpWorldDestroy(Scene.world);
	CameraDestroy(Scene.camera);
	Scene.world = NULL;
	Scene.camera = NULL;
	

	
#ifdef GTA_PC

#ifndef OSW
	// release all the textures in the pools
	D3DPoolsShutdown();
#endif

	CPostEffects::Close();

#ifdef DEBUG
	extern void PrintAllocatedTextures();
	PrintAllocatedTextures();
#endif
	
#endif//GTA_PC
}

//
// name:		FinalShutdown
// description:	Does final clean up
void CGame::FinalShutdown()
{
	// This would be for stuff which is initialised before going into frontend. Needs to be able to clean it
	// up if exitting the frontend without starting / quitting the game.
	ThePaths.Shutdown();
	CTempColModels::Shutdown();
	CTxdStore::Shutdown();
	CPedStats::Shutdown();
	TheText.Unload(FALSE);
	CdStreamShutdown();
}


//
// when the app loses focus (ie alt-tab), some things need to be initialised again...
//
#ifdef GTA_PC
void CGame::InitAfterLostFocus()
{
	// start up the pause menu:
 	if (!FrontEndMenuManager.m_MainMenu && !FrontEndMenuManager.m_MenuActive)
 	{
 		FrontEndMenuManager.m_bStartUpFrontEndRequested = TRUE;
 	}
}
#endif // GTA_PC

void InitVehicleDebug();

//
// name:		Initialise
// description: Initialises game	
bool CGame::Initialise(const char *pDatFile)
{
	Init1(pDatFile);

#if defined (GTA_PC) || defined (GTA_XBOX)
//	CLoadingScreen::DoPCScreenChange(FALSE);
#endif //GTA_PC

	LOAD_CODE_OVERLAY(fileload);
			
	CColAccel::startCache();
			
	CFileLoader::LoadLevel("DATA\\DEFAULT.DAT");
	CFileLoader::LoadLevel(pDatFile);
	
	CColAccel::endCache();

	REMOVE_CODE_OVERLAY();

	LOAD_CODE_OVERLAY(init);
	
	Init2(pDatFile);
	
#if defined (GTA_PC) || defined (GTA_XBOX)
//	CLoadingScreen::DoPCScreenChange(FALSE);
#endif //GTA_PC

	REMOVE_CODE_OVERLAY();
	
	// MN - VarConsole stuff --------------------------------------------------
#ifndef FINAL
    
	VarConsole.Add("Display All PostEffects", &CPostEffects::m_bDisableAllPostEffect, true, VC_RENDER);	
	VarConsole.Add("Display FxSystem Info", &Fx_c::ms_displayFxManagerInfo, true, VC_RENDER);	
	VarConsole.Add("Curr Debug Fx System",&Fx_c::ms_currDebugFxSystem, 1, 0, g_fxMan.GetNumFxSystemBPs()-1, true, VC_RENDER);
	VarConsole.Add("Create Fx System", &Fx_c::CreateDebugFxSystem, VC_RENDER);	
	VarConsole.Add("Ignore FxSystem Brightness", &Fx_c::ms_ignoreBrightness, true, VC_RENDER);	
	VarConsole.Add("Ignore FxSystem Fogging", &Fx_c::ms_ignoreFogging, true, VC_RENDER);	
	VarConsole.Add("Particle System Active", &Fx_c::ms_particlesActive, true, VC_RENDER);
	
	VarConsole.AddCommand("FxSystem", &Fx_c::CreateDebugFxSystemVC);

	VarConsole.Add("Dump Ped Acquaintance Info", &CPedType::DumpPedTypeAcquaintanceInfo, VC_PEDS);	
	
	BoneNodeManager_c::ms_renderDebugInfo = false;
	VarConsole.Add("Render Ragdoll & IK Info", &BoneNodeManager_c::ms_renderDebugInfo, true, VC_RENDER);	
	
#ifdef DEBUG_INTERIORS
	InteriorManager_c::ms_renderDebugInfo = true;
#else
	InteriorManager_c::ms_renderDebugInfo = false;
#endif
	VarConsole.Add("Render Procedural Interior Info", &InteriorManager_c::ms_renderDebugInfo, true, VC_RENDER);	
	
	InteriorManager_c::ms_randomInteriors = false;
	VarConsole.Add("Random Procedural Interiors", &InteriorManager_c::ms_randomInteriors, true, VC_RENDER);
	
	VarConsole.Add("Display Procedural Object Info", &g_procObjMan.m_displayDebugInfo, true, VC_RENDER);

	VarConsole.Add("Dump Game Info", &CDebug::DumpGameInfo);
	
	InitVehicleDebug();
#endif
	// ------------------------------------------------------------------------
#ifdef GTA_PS2
#ifndef CDROM
	VarConsole.AddCommand("reload", &ReloadDataFile);
#endif	
#endif //GTA_PC

		
#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
	if(!CVolumetricShadowMgr::Initialise())
	{
		ASSERTMSG(FALSE, "Can't initialize VolumetricShadowMgr!");
	}
#endif

	//
	// Start script outside of overlay as script loads a code
	// overlay to generate the player
	//	
	LoadingScreen("Loading the Game", "Start script");

#ifdef GTA_PS2
	if (FrontEndMenuManager.WantsToLoad==FALSE)
	{
#ifdef GTA_NETWORK	
        if (CGameNet::NetWorkStatus == NETSTAT_SINGLEPLAYER)
        {
#endif        
		    CTheScripts::StartTestScript();
		    CTheScripts::Process();
#ifdef GTA_NETWORK	
		}
		else
		{    // Give us a player. Script hasn't generated one but we do need one.
			GenerateTempPedAtStartOfNetworkGame();
		}
#endif        
		TheCamera.Process();
	}	
#else

#ifdef GTA_NETWORK
        if (CGameNet::NetWorkStatus == NETSTAT_SINGLEPLAYER)
        {
#endif
		    CTheScripts::StartTestScript();
		    CTheScripts::Process();
#ifdef GTA_NETWORK
		}
		else
		{    // Give us a player. Script hasn't generated one but we do need one.
			GenerateTempPedAtStartOfNetworkGame();

				// Creates a quick plyer so that the game doesn't crash
			// Load player model
			if (!CStreaming::HasModelLoaded(0))
			{
				CStreaming::RequestSpecialModel(0, "player", STRFLAG_DONTDELETE|STRFLAG_FORCE_LOAD);
				CStreaming::LoadAllRequestedModels();
			}
		}
#endif
		TheCamera.Process();
#endif

	LOAD_CODE_OVERLAY(init);
	
	Init3(pDatFile);

#ifdef GTA_XBOX
	HdStreamEnable(); // enable streaming from xbox utility drive
#endif

	REMOVE_CODE_OVERLAY();
	
//!PC - PS2 only options for time being
#if defined (GTA_PS2)
#ifndef MASTER
	VarConsole.Add("Draw HUD", &CHud::m_Wants_To_Draw_Hud, false, VC_RENDER);
	VarConsole.Add("Population debug info",&CPopCycle::m_bDisplayDebugInfo, true, VC_PEDS);
	VarConsole.Add("Population car debug info",&CPopCycle::m_bDisplayCarDebugInfo, true, VC_VEHICLES);
	VarConsole.Add("PlantMgr active",		&gbPlantMgrActive,		TRUE, VC_RENDER);
	VarConsole.Add("Display CPlantMgr Info",&gbDisplayCPlantMgrInfo,TRUE, VC_RENDER);
	VarConsole.Add("Show CPlantMgr Polys", 	&gbShowCPlantMgrPolys, 	TRUE, VC_RENDER);

#ifndef FINALBUILD
	VarConsole.Add("CVB Render Debug Tristrips", &gbVC_CVBRenderDebugTristrips, TRUE, VC_RENDER);
#endif


	// finds the player coords and adds the options to the VarConsole (increments of 10)
	PlayerCoords = FindPlayerCoors();
	VarConsole.Add("Visible Area code", (int32*)&CGame::currArea, 1, AREA_MAIN_MAP, 100, TRUE, VC_RENDER);
	VarConsole.Add("X PLAYER COORD", &PlayerCoords.x, 10.0f, -10000.0f, 10000.0f, TRUE, VC_PEDS);
	VarConsole.Add("Y PLAYER COORD", &PlayerCoords.y, 10.0f, -10000.0f, 10000.0f, TRUE, VC_PEDS);
	VarConsole.Add("Z PLAYER COORD", &PlayerCoords.z, 10.0f, -10000.0f, 10000.0f, TRUE, VC_PEDS);
	VarConsole.Add("UPDATE PLAYER COORD", &VarUpdatePlayerCoords, TRUE, VC_PEDS);
#ifndef FINALBUILD
	// Set some variables to be changeable using the VarConsole stuff.
#ifdef USE_PEDAI_CACHING	
	VarConsole.Add("Use AI entity bounds caching",&CPedGeometryAnalyser::ms_bUseEntityBoundsCaching, true, VC_PEDS);
	VarConsole.Add("Use AI line-of-sight caching",&CPedGeometryAnalyser::ms_bUseLineOfSightCaching, true, VC_PEDS);
	VarConsole.Add("Display AI caching stats",&CPedGeometryAnalyser::ms_bDisplayProfilingInfo, true, VC_PEDS);
#endif // USE_PEDAI_CACHING

#ifdef GTA_PS2
	extern bool bDisplay32bitTextures;
	VarConsole.Add("Display 32bit textures",&bDisplay32bitTextures, true, VC_RENDER);
#endif // GTA_PS2	
	VarConsole.Add("Don't render vehicles",&gbDontRenderVehicles, true, VC_VEHICLES);
	VarConsole.Add("Don't render peds",&gbDontRenderPeds, true, VC_PEDS);
	VarConsole.Add("Don't render buildings",&gbDontRenderBuildings, true, VC_RENDER);
	VarConsole.Add("Don't render bigbuilding",&gbDontRenderBigBuildings, true, VC_RENDER);
	VarConsole.Add("Don't render water",&gbDontRenderWater, true, VC_RENDER);
	VarConsole.Add("Don't render objects",&gbDontRenderObjects, true, VC_RENDER);
	VarConsole.Add("Display Unmatched LODs",&gbDisplayUnmatchedBigBuildings, true, VC_RENDER);

#endif // !FINALBUILD
	VarConsole.Add("Use car col textures", &gbUseCarColTex, true, VC_VEHICLES);
#endif //MASTER
#endif // GTA_PS2

	return true;
}



#ifdef GTA_PC
// Name			:	Shutdown
// Purpose		:	Shuts down CGame object
// Parameters	:	None
// Returns		:	Nothing
bool CGame::Shutdown()
{
	// MN - FX SYSTEM -------------------------------
//	g_fx.Exit();
	// ----------------------------------------------
	
	// MN - BREAKABLE OBJECTS -----------------------
	g_breakMan.Exit();	
	// ----------------------------------------------
	
	// MN - PROCEDURAL INTERIORS --------------------
	g_interiorMan.Exit();	
	// ----------------------------------------------
	
	// MN - PROCEDURAL OBJECTS ----------------------
	g_procObjMan.Exit();
	// ----------------------------------------------
	
	// MN - WATER CREATURES  ------------------------
	g_waterCreatureMan.Exit();
	// ----------------------------------------------
	
	// MN - IK & RAGDOLL PHYSICS --------------------
//	g_boneNodeMan.Exit();
//	g_ikChainMan.Exit();
//	g_ragdollMan.Exit();
	// ----------------------------------------------

	// MN - SHADERS  --------------------------------
#ifdef USE_SHADERS
	g_shaderMan.Exit();
#endif
	// ----------------------------------------------

#ifndef OSW
	D3DPoolsEnable(false);
#endif

#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
	CVolumetricShadowMgr::Shutdown();
#endif

#ifdef USE_PLANT_MGR
	CPlantMgr::Shutdown();
	CGrassRenderer::Shutdown();
#endif



	CRopes::Shutdown();
	CVehicleRecording::ShutDown();
	CReplay::FinishPlayback();
	CReplay::EmptyReplayBuffer();
//	CFakePlane::Shutdown();   //!PC - no longer used?
	CTrain::Shutdown();
//	CScriptPaths::Shutdown();	//!PC - no longer used?
//	CWaterCreatures::RemoveAll();
	CBirds::Shutdown();
	
	CSpecialFX::Shutdown();
	CGarages::Shutdown();
//	CEntryExitManager::Shutdown();
	CStuntJumpManager::Shutdown();	//!PC - typo
	CMovingThings::Shutdown();
//	gPhoneInfo.Shutdown();
	CWeapon::ShutdownWeapons();
	CPedType::Shutdown();
//	CMBlur::MotionBlurClose();
	
	
	for (Int32 C = 0; C < MAX_NUM_PLAYERS; C++)
	{
		if(CWorld::Players[C].pPed)
		{
			CWorld::Remove(CWorld::Players[C].pPed);
			delete CWorld::Players[C].pPed;
			CWorld::Players[C].pPed = NULL;
		}
		CWorld::Players[C].Clear();
	}

	CRenderer::Shutdown();
	CWorld::ShutDown();
	
	//!PC
	// moved here since it is required by some entities shutting down
	CEntryExitManager::Shutdown();
	// MN - FX SYSTEM -------------------------------
	g_fx.Exit();
	// MN - IK & RAGDOLL PHYSICS --------------------
	g_boneNodeMan.Exit();
	g_ikChainMan.Exit();

	// MN - REAL TIME SHADOWS  ----------------------
	g_realTimeShadowMan.Exit();
	// ----------------------------------------------
	

//#ifdef USE_AUDIOENGINE
	//AudioEngine.Shutdown();
//#else //USE_AUDIOENGINE
//	DMAudio.DestroyAllGameCreatedEntities();//deletes the sound entities
//#endif //USE_AUDIOENGINE

	// shutdown modelinfo after world because world requires modelinfo to 
	// delete properly (collision models)
	CModelInfo::ShutDown();	
	CAnimManager::Shutdown();
	CCutsceneMgr::Shutdown();
	CVehicleModelInfo::DeleteVehicleColourTextures();
	CVehicleModelInfo::ShutdownEnvironmentMaps();
	CRadar::Shutdown();		// before CTxdStore::GameShutdown
	CTouchInterface::DeleteAll();
	CStreaming::Shutdown();
	CTxdStore::GameShutdown();

	CCollision::Shutdown();

	CWaterLevel::Shutdown();
//	CRubbish::Shutdown();	// before CParticle::Shutdown
	CClouds::Shutdown();	// before CParticle::Shutdown
	CShadows::Shutdown();	// before CParticle::Shutdown
	CCoronas::Shutdown();	// before CParticle::Shutdown
	CSkidmarks::Shutdown();	// before CParticle::Shutdown
	CWeaponEffects::Shutdown();	// before CParticle::Shutdown
	CTimeCycle::Shutdown();
//	CParticle::Shutdown();	//!PC - no longer used?
//	CLightning::Shutdown();

	CCover::CleanUpForShutdown();		// want to flush dblPtrList of processed buildings 
	CPedGroups::CleanUpForShutDown();	// want to flush CTasks held in static class
	CTaskSequences::CleanUpForShutdown();		// want to flush out the array of tasks held in here
	
	CInformGroupEventQueue::Flush();
	CInformFriendsEventQueue::Flush();
	
	CPools::ShutDown();
#ifndef GTA_PC	 // Not for PC, as the array will be removed when the audio engine shutdown
	CPlaceable::ShutdownMatrixArray();
#endif
#ifdef USE_VU_TURBO_SPRITE
	CVuTurboSprite::Shutdown();
#endif

#ifdef USE_VU_WATER_RENDERER
	CVuWaterRenderer::Shutdown();
#endif

#ifdef USE_VU_CARFX_RENDERER
	CVuCarFXRenderer::Shutdown();
#endif

#ifdef USE_PC_CARFX_RENDERER
	CCarFXRenderer::Shutdown();
#endif

	
#ifdef USE_CVBUILDING_RENDERER
	#if defined(GTA_PS2)
		CVuCustomBuildingRenderer::Shutdown();
	#endif
	#if defined(GTA_PC) || defined(GTA_XBOX)
		CCustomBuildingRenderer::Shutdown();
	#endif
#endif

#ifdef USE_CVSKINPED_RENDERER
	CVuCustomSkinPedRenderer::Shutdown();
#endif
	CVehicleModelInfo::ShutdownLightTexture();

	CVehicle::Shutdown();

#ifdef USE_VU_CARFX_RENDERER
	CVuCarFXRenderer::Shutdown();
#endif
#ifdef USE_PC_CARFX_RENDERER
	CCarFXRenderer::Shutdown();
#endif

	CCustomCarPlateMgr::Shutdown();
	CCustomRoadsignMgr::Shutdown();

	
	// need to call this here so that messages are all removed
	CHud::ReInitialise();

	CTxdStore::RemoveTxdSlot(gameTxdSlot);
	
	CTxdStore::RemoveTxdSlot(CTxdStore::FindTxdSlot("particle"));
	
	// AI stuff cleanup ready for shutdown (cleanup of static structures mainly!)
	CTaskSimpleFight::Shutdown();
#ifndef FINAL
	CTaskCounter::Shutdown();
#endif	
	CTaskSimpleClimb::Shutdown();

#ifndef FINAL
	CEventCounter::Shutdown();
#endif
	CPedAttractor::Shutdown();

	//may need to remove textures which are referenced by script sprites
	CTheScripts::Shutdown();

#ifdef GTA_PC	
	CMBlur::MotionBlurClose();
	CdStreamRemoveImages();

#ifndef OSW
	D3DPoolsShutdownForRestart();
#endif

#endif	

	return true;
} // end - CGame::ShutDown
#endif




////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////

void CGame::ReInitGameObjectVariables()
{
	TheCamera.Init();
	
	CGameLogic::InitAtStartOfGame();
	CGangWars::InitAtStartOfGame();
	
	CWanted::InitialiseStaticVariables();

#ifdef GTA_PS2
	if (!FrontEndMenuManager.WantsToLoad)
#endif	
	{
		TheCamera.Init();
		TheCamera.SetRwCamera(Scene.camera);
	}
	
#ifndef FINAL	
	CDebug::DebugInitTextBuffer();
#endif	

	CSkidmarks::Clear();
	CWeather::Init();
	CCover::Init();
	CUserDisplay::Init();	
	CMessages::Init();
	
	CRestart::Initialise();

	CPostEffects::EffectsOff(); // Make sure all effects are OFF!

////////////////////////////////////////////////////////////////////////////////////////////////////
//we do this 
	CWorld::bDoingCarCollisions = false;
//which is basically Cworld Initialise without the CPools Initialise
//cause CPools Initialise uses "new" and we all know that that is 
//the equivalent of mass murder
////////////////////////////////////////////////////////////////////////////////////////////////////


	CHud::ReInitialise();
	
	CRadar::Initialise();//clears the radar 

	CCarCtrl::ReInit();
	ThePaths.ReInit();

#ifdef PAL_BUILD
	CTimeCycle::Initialise(true);
#else
	CTimeCycle::Initialise(false);
#endif

	CPopCycle::Initialise();
	
	CDraw::SetFOV(120.0f);
	CDraw::SetLODDistance(500.0f);

	CGame::currArea = AREA_MAIN_MAP;

	// Can only initialise peds once animations have been loaded
	CPed::Initialise();

	// This needs to be done after streaming is initialised
	// Can only initialise weapons once animations have been loaded
	CWeapon::InitialiseWeapons();

	CPopulation::Initialise();

	for (Int16 C = 0; C < MAX_NUM_PLAYERS; C++)
	{
		CWorld::Players[C].Clear();
	}
	CWorld::PlayerInFocus = 0;
//	CAntennas::Init();
	CGlass::Init();
	gbLARiots = gbLARiots_NoPoliceCars = false;

//	gPhoneInfo.Initialise();

	CMemInfo::EnterBlock(SCRIPT_MEM_ID);
	
	IFWEARENOTINNETWORKGAME(CTheScripts::Init();)
	LOAD_CODE_OVERLAY(init);
	
	CMenuSystem::Initialise();

	// Test thingy for the scripting stuff
	CGangs::Initialise();
	
	CMemInfo::ExitBlock();

	// initialise global timer
	CTimer::Initialise();

	// initialise game timer
	CClock::Initialise(1000);
	CTheCarGenerators::Init();
	//CThePedGenerators::Init();
	CHeli::InitHelis();
	CMovingThings::Init();
	CDarkel::Init();
//	CStunts::Init();
	CStats::Init();
//	CPacManPickups::Init();
	CGarages::Init_AfterRestart();
	CRoadBlocks::Init();
	CStreaming::DisableCopBikes(false);
//	CEntryExitManager::Init();
	CSpecialFX::Init();
	CRopes::Init();
	CWaterCannons::Init();
//	CScriptPaths::Init();
	CDecisionMakerTypesFileLoader::ReStart();
	CVehicleRecording::Init();
	gFireManager.Init();
	g_interiorMan.Init();
	g_procObjMan.Init();
	g_waterCreatureMan.Init();
	g_realTimeShadowMan.Init();
	CStreaming::RemoveInappropriatePedModels();
#ifdef GTA_NETWORK
	CNetGames::Init();
#endif		

#ifdef USE_AUDIOENGINE
	AudioEngine.ResetStatistics();
#endif //USE_AUDIOENGINE

//	This is now done in LoadLevel() because collision isn't available here
//	CWorld::RepositionCertainDynamicObjects();
//	CCullZones::ResolveVisibilities();
	
	//////WATCH OUT ITS THE LOAD GAME BIT IEEEEEEEEEEEEEEEEEEEEEEEEE!
	//need to be done like this because 
	//we can't Create and allocate stuff to the pools (SetUp through the Scripts)					
	//incase we get duff pointers	
	if (!FrontEndMenuManager.WantsToLoad)
	{
#ifdef GTA_PS2
	
		CCranes::InitCranes(); 

#ifdef GTA_NETWORK
        if ((!FrontEndMenuManager.m_WantsToRestartGameAsServer) && (!FrontEndMenuManager.m_WantsToRestartGameAsClient) )
        {
#endif        
			CTheScripts::StartTestScript();
			CTheScripts::Process();
#ifdef GTA_NETWORK
		}
		else
		{
			GenerateTempPedAtStartOfNetworkGame();
		}
#endif        
		
		TheCamera.Process();
		CTrain::InitTrains();
//		CFakePlane::InitPlanes();
		
#else
		CCranes::InitCranes();

#ifdef GTA_NETWORK
        if ((!FrontEndMenuManager.m_WantsToRestartGameAsServer) && (!FrontEndMenuManager.m_WantsToRestartGameAsClient) )
        {
#endif
			CTheScripts::StartTestScript();
			CTheScripts::Process();
#ifdef GTA_NETWORK
		}
		else
		{
			GenerateTempPedAtStartOfNetworkGame();
		}
#endif
		TheCamera.Process();
		CTrain::InitTrains();
//		CFakePlane::InitPlanes();	//!PC - no longer used?
#endif
	}
	CPad::GetPad(0)->Clear(TRUE);
	CPad::GetPad(1)->Clear(TRUE);
}


///////////////////////////////////
///////////////////////////////////
///////////////////////////////////
void CGame::ReloadIPLs()
{
/*	CTimer::Stop();

	CWorld::RemoveStaticObjects();
	
	ThePaths.Init();

	CCullZones::Init();

	// have to reload IDE
	CFileLoader::ReloadPaths("GTA3.IDE");

	// load object instances
	CMemInfo::EnterBlock(WORLD_MEM_ID);
	CFileLoader::LoadScene("INDUST.IPL");
	CFileLoader::LoadScene("COMMER.IPL");
	CFileLoader::LoadScene("SUBURBAN.IPL");
	CFileLoader::LoadScene("CULL.IPL");
	CMemInfo::ExitBlock();

	ThePaths.PreparePathData();

	CTrafficLights::ScanForLightsOnMap();
	CRoadBlocks::Init();
	CCranes::InitCranes();
	CGarages::Init();
//	This is now done in LoadLevel() because collision isn't available here
//	CWorld::RepositionCertainDynamicObjects();
	CCullZones::ResolveVisibilities();
	CRenderer::SortBIGBuildings();
	
	CTimer::Update();*/
}

// Name			:	ShutDownForRestart
// Purpose		:	The way this works is that the pools get cleared but
//				:	not destroyed and the enities get removed from world
//				:	The texture dictionaries, model infos, anim etc all stay 
//				: 	in memory. This make for a fast restart, cause you don't
//				:	need to load them 
// Parameters	:	None
// Returns		:	Nothing
void CGame::ShutDownForRestart()
{
#if defined(GTA_PC) && !defined(OSW)
	D3DPoolsEnable(false);
#endif
	
	CVehicleRecording::ShutDown();
	CReplay::FinishPlayback();
	CReplay::EmptyReplayBuffer();
	
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	DMAudio.DestroyAllGameCreatedEntities();//deletes the sound entities
#endif //USE_AUDIOENGINE
	
	// remove escalators etc
	CMovingThings::Shutdown();
	
	for (Int16 C = 0; C < MAX_NUM_PLAYERS; C++)
	{
		CWorld::Players[C].Clear();
	}
	
	for(int32 i=0; i<NUM_MAP_AREAS_TO_VISIT; i++)
		for(int32 j=0; j<NUM_MAP_AREAS_TO_VISIT; j++)
		{
			CTheZones::ZonesVisited[i][j] = FALSE;
		}
	
//	CGarages::SetAllDoorsBackToOriginalHeight();

	IFWEARENOTINNETWORKGAME(CTheScripts::UndoBuildingSwaps();)
	IFWEARENOTINNETWORKGAME(CTheScripts::UndoEntityInvisibilitySettings();)

	// MN - PROCEDURAL INTERIORS --------------------
	g_interiorMan.Exit();	
	// ----------------------------------------------
	
	// MN - PROCEDURAL OBJECTS ----------------------
	g_procObjMan.Exit();
	// ----------------------------------------------
	
	// MN - WATER CREATURES -------------------------
	g_waterCreatureMan.Exit();
	// ----------------------------------------------

	CRopes::Shutdown();
	CWorld::ClearForRestart();
//	CGameLogic::ClearShortCut();
	CGameLogic::ClearSkip();
	CTimer::Shutdown();
	
	CStreaming::ReInit();
	
	CRadar::RemoveRadarSections();	
	// ensure frontend textures have been removed
	FrontEndMenuManager.UnloadTextures();

//	CWaterCreatures::RemoveAll();
	CSetPieces::Init();
	
	CConversations::Clear();
	CPedToPlayerConversations::Clear();
	
	CPedType::Shutdown();
	CSpecialFX::Shutdown();
	
	gFireManager.Shutdown();	//	must be done before the fx system is shut down
	
	g_fx.Reset();
	g_breakMan.ResetAll();
	
	// MN - IK & RAGDOLL PHYSICS --------------------
	g_boneNodeMan.Reset();
	g_ikChainMan.Reset();
//	g_ragdollMan.Reset();
	// ----------------------------------------------
	
	// MN - REAL TIME SHADOWS  ----------------------
	g_realTimeShadowMan.Exit();
	// ----------------------------------------------

	CTheZones::ResetZonesRevealed();
	CEntryExitManager::ShutdownForRestart();
	CShopping::ShutdownForRestart();
	CTagManager::ShutdownForRestart();
	CStuntJumpManager::ShutdownForRestart();
	
	CGame::TidyUpMemory(true, false);	
	
	CVehicle::ms_forceVehicleLightsOff = false;
}

// Name			:	InitialiseWhenRestarting
// Purpose		:	Resets everything
// Parameters	:	None
// Returns		:	Nothing
void CGame::InitialiseWhenRestarting()
{
	CRect ScreenRect(0,0, SCREEN_WIDTH, SCREEN_HEIGHT);		
	CRGBA ScreenColor(255,255,255,255);
	bool bWrongVersion;
	
	CTimer::Initialise();

	// need to reset all global timers that are compared against GetTimeInMilliseconds() when timer gets reset
	CEventScanner::m_sDeadPedWalkingTimer = 0;

	if(FrontEndMenuManager.WantsToLoad)
	{
		if (FrontEndMenuManager.WantsToLoad)
		{  // loading game....
			FrontEndMenuManager.MessageScreen("FELD_WR");
		}
		#ifdef GTA_PS2 
		else
		{   // restarting game....
			FrontEndMenuManager.MessageScreen("RESTART");
		}
		#endif // GTA_PS2
	}

#ifndef FINAL

	LOAD_CODE_OVERLAY(fileload);
	CStreaming::ReLoadCdDirectory();

#endif

	if (FrontEndMenuManager.WantsToLoad)
	{
		LOAD_CODE_OVERLAY(mc);
		
		CGenericGameStorage::RestoreForStartLoad();
		
		// remove big buildings from other islands
		CStreaming::RemoveBigBuildings();
	}
	
	LOAD_CODE_OVERLAY(init);

	ReInitGameObjectVariables();//Initialize the things at the start of the game
	
	CTimeCycle::InitForRestart();
	
	CWeaponEffects::Init();
	
	CPlane::InitPlaneGenerationAndRemoval();
	
	if (FrontEndMenuManager.WantsToLoad)
	{
		LOAD_CODE_OVERLAY(mc);

		FrontEndMenuManager.WantsToLoad=FALSE;
		CGenericGameStorage::InitRadioStationPositionList();  // default the radio station lists

		if (CGenericGameStorage::GenericLoad(bWrongVersion))
		{	
#ifdef GTA_PS2	
//			IFWEARENOTINNETWORKGAME(CTheScripts::Init();)
			
//			if (!FrontEndMenuManager.m_PrefsOutput) FrontEndMenuManager.DrawDTSWarningScreenAtStartUp();

			int k=0;
			const int WaitLoopNumIterations=35;
			while (k<WaitLoopNumIterations)
			{
				FrontEndMenuManager.MessageScreen("FESZ_LS");
				k++;
			}	

			// init some new settings after loading successfully:
			CGenericGameStorage::InitNewSettingsAfterLoad();
#endif // GTA_PS2

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			DMAudio.ResetTimers( CTimer::GetTimeInMilliseconds() );
#endif //USE_AUDIOENGINE

			CTrain::InitTrains();
//			CFakePlane::InitPlanes();
		}
		else
		{
			//this is called when the game has failed to load
			#if defined (GTA_PC)
				int i=0;
				const int WaitLoopNumIterations=50;
				while (i<WaitLoopNumIterations)
				{
#ifndef OSW
					ProcessWindowsMessages();
#endif
					if (bWrongVersion)
						FrontEndMenuManager.MessageScreen("FES_LOC"); // need new message here
					else
						FrontEndMenuManager.MessageScreen("FED_LFL");
					i++;
				}
			#elif defined (GTA_PS2)
				FrontEndMenuManager.DrawLoadFailedScreen(bWrongVersion);
			#elif defined (GTA_XBOX)
				FrontEndMenuManager.DrawLoadFailedScreen(bWrongVersion);
			#else 
				#error("Unrecognised target");
			#endif//GTA_PC
			
			TheCamera.SetFadeColour(0.0f,0.0f,0.0f);
			
			CGame::ShutDownForRestart(); ///clears anything previously 
			CTimer::Stop();
			CTimer::Initialise();
			FrontEndMenuManager.WantsToLoad=false;
			
			LOAD_CODE_OVERLAY(init);
			
			ReInitGameObjectVariables();//Initialize the things at the start of the game
			CGame::currLevel = LEVEL_GENERIC;
			CGame::bMissionPackGame = 0;
			CCollision::SortOutCollisionAfterLoad();
			// Obbe has changed everything so let him deal with it
			//call the frontend thingy so that sound levels are restored
			//FrontEndMenuManager.SetSoundLevelsForMusicMenu(); 
			//FrontEndMenuManager.InitialiseMenuContentsAfterLoadingGame();
		}
	}
		
	CTimer::Update();

	LOAD_CODE_OVERLAY(init);


	AudioEngine.ResetSoundEffects(); //One final clean-up.
	AudioEngine.Restart();

	// init some default audio settings:
#ifdef GTA_PS2
	AudioEngine.SetOutputMode(FrontEndMenuManager.m_PrefsAudioOutputMode);
#endif
	AudioEngine.SetMusicMasterVolume(FrontEndMenuManager.m_PrefsMusicVolume);
	AudioEngine.SetEffectsMasterVolume(FrontEndMenuManager.m_PrefsSfxVolume);
	AudioEngine.SetBassEnhanceOnOff(FrontEndMenuManager.m_PrefsUseBass);
	AudioEngine.SetRadioAutoRetuneOnOff(FrontEndMenuManager.m_PrefsAutoRetune);
	AudioEngine.InitialiseRadioStationID(FrontEndMenuManager.m_PrefsRadioStation);


#if defined(GTA_PC) && !defined (OSW)
	D3DPoolsEnable(true);
#endif

#ifndef FINALBUILD
	LOAD_CODE_OVERLAY(init);
	ReadPlayerCoordsFile();
#endif
}

// Name			:	Process
// Purpose		:	Processes CGame object
// Parameters	:	None
// Returns		:	Nothing
//#include "taskanims.h"
void CGame::Process()
{
#ifdef GTA_PS2
#ifndef CDROM
		// if select button is pressed while one of square,cross,circle or triangle are pressed then reload data
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_N, KEYBOARD_MODE_STANDARD, "Reload data files"))
		//if(CPad::GetPad(0)->SelectJustDown() && (CPad::GetPad(0)->GetButtonSquare() || CPad::GetPad(0)->GetButtonCircle() || CPad::GetPad(0)->GetButtonCross() || CPad::GetPad(0)->GetButtonTriangle()))
		{
			ReloadDataFile(NULL);
		}

#endif
#endif //GTA_PS2

#define MILLISECOND_MAXLOAD 	4	// Obbe:Increased this because the value was sometimes 2 for long spells creating no cars and loads of bugs
	// the millisecond timer is used to find out how much time each streaming,population and 
	// carctrl are taking. If streaming takes more than 2 milliseconds then don't do population
	// or carctrl and if population takes more than 2 milliseconds then don't do carctrl

	uint32 millisecondTimer;

#ifdef GTA_XBOX
	// update the xbox peripherals
	g_peripheralsXBox.Update();
#endif

	START_TIMER ("UpdatePad");
	CPad::UpdatePads();
	END_TIMER ("UpdatePad");
	
#ifdef GTA_PS2
	#ifdef TIMERS
		tbStartTimer ( 0,"Tidyup");
	#endif
	
	// do slow tidy up of memory
	ProcessTidyUpMemory();	
	
	#ifdef TIMERS
		tbEndTimer ( "Tidyup");
	#endif
#endif

	// begin frame for monitoring CPU load
	g_LoadMonitor.BeginFrame();

	// time streaming
	millisecondTimer = CTimer::GetCurrentTimeInMilleseconds();
	// streaming needs to update before cutscene manager
	CStreaming::Update();
	millisecondTimer = CTimer::GetCurrentTimeInMilleseconds() - millisecondTimer;
	
	CCutsceneMgr::Update();

	// don't allow the player to pause during cutscenes
	if(!CCutsceneMgr::IsCutsceneProcessing() && !CTimer::m_CodePause)
		FrontEndMenuManager.Process(); 
		
	CTheZones::Update();
	CCover::Update();
	CAudioZones::Update();
	
// Protection thingy. Set gbGameCracked2 if it looks like the game has been cracked.
#ifdef GTA_PC
#ifdef CDPROTECTION
#define TIMETEST (35 * 60 * 1000)	// 35 minutes into a game
	if (CTimer::GetTimeInMilliseconds() >= TIMETEST && !gGameCracked2)
	{
_asm {
	push eax
	push ebx
	push ecx
	mov eax, 6402
	mov ebx, 463
	mov ecx, 9256
}
		if (GetVersion() != 9256)
		{
			gGameCracked2 = 2;	// cracked
		}
		else
		{
			gGameCracked2 = 1;	// ok
		}
_asm {
	pop ecx
	pop ebx
	pop eax
}

	}
#endif
#endif

	CWindModifiers::Update();
	
	if (GAMEISNOTPAUSED)
	{
		//
		// updates the player coords inside the varconsole:
		//
		#ifdef DEBUG
		// checks a few debug keyboard toggles:
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_1, KEYBOARD_MODE_STANDARD, "dont render peds")) gbDontRenderPeds = !gbDontRenderPeds;
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_2, KEYBOARD_MODE_STANDARD, "dont render buildings")) gbDontRenderBuildings = !gbDontRenderBuildings;
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_3, KEYBOARD_MODE_STANDARD, "dont render big buildings")) gbDontRenderBigBuildings = !gbDontRenderBigBuildings;
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_4, KEYBOARD_MODE_STANDARD, "dont render water")) gbDontRenderWater = !gbDontRenderWater;
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_5, KEYBOARD_MODE_STANDARD, "dont render objects")) gbDontRenderObjects = !gbDontRenderObjects;
		
		if (VarUpdatePlayerCoords)
		{
			CPed* pPlayer = FindPlayerPed();
			pPlayer->Teleport(PlayerCoords);
			VarUpdatePlayerCoords = FALSE;
		}
		#endif  // DEBUG
		
		#ifndef FINAL
			ResetDebugPedAICounters();
		#endif	// FINAL
		
		#ifdef TIMERS
			tbStartTimer ( 0,"Init");
		#endif
		
		// Do this at the start of frame
		CSprite2d::SetRecipNearClip ( );
		CSprite2d::InitPerFrame();
		CFont::InitPerFrame();
		
		CCheat::DoCheats();	// Has to be done after SaveOrRetrieveDataForThisFrame or cheats won't work in replay

		CClock::Update();

		#ifdef TIMERS
			tbEndTimer ( "Init");
		#endif
		
		#ifdef TIMERS
			tbStartTimer ( 0,"Weather");
		#endif
		
		IFWEAREINNETWORKGAME( CGameNet::PreProcess(); )
		CWeather::Update();

		#ifdef TIMERS
			tbEndTimer ( "Weather");
		#endif
		
		#ifdef TIMERS
			tbStartTimer ( 0,"Scripts");
		#endif
		
		CMemInfo::EnterBlock(SCRIPT_MEM_ID);
		IFWEARENOTINNETWORKGAME(CTheScripts::Process();)	// Must happen before CWorld::Process so that stuff can be initialised before the game runs.
		CMemInfo::ExitBlock();

		#ifdef TIMERS
			tbEndTimer ( "Scripts");
		#endif
/*
#ifndef FINAL
//	Fiddly code to allow updating of the screen during Script Debugger Single Step mode
		if (CScriptDebugger::bSingleStep)
		{
			return;
		}
#endif
*/

		#ifdef TIMERS
			tbStartTimer ( 0,"Collision");
		#endif
		
		CCollision::Update();
		#ifdef TIMERS
			tbEndTimer ( "Collision");
		#endif	
		
		#ifdef TIMERS
			tbStartTimer ( 0,"Trains/Planes");
		#endif

		CMemInfo::EnterBlock(PATHS_MEM_ID);
		ThePaths.UpdateStreaming();
		CMemInfo::ExitBlock();

//		CScriptPaths::Update();
		CTrain::UpdateTrains();
//		CFakePlane::UpdatePlanes();
		CHeli::UpdateHelis();
		CDarkel::Update();
//		CStunts::Update();
		CSkidmarks::Update();
//		CAntennas::Update();
		CGlass::Update();
		CWanted::UpdateEachFrame();
		CCreepingFire::Update();
#ifdef GTA_PC	
		//!PC - CSceneEdit disabled	
		// CSceneEdit::Update();
#endif
		CSetPieces::Update();
		
//		CTheShopDoors::Update();

		#ifdef TIMERS
			tbEndTimer ( "Trains/Planes");
		#endif


		#ifdef TIMERS
			tbStartTimer ( 0,"Fire");
		#endif
		gFireManager.Update();
		#ifdef TIMERS
			tbEndTimer ( "Fire");
		#endif
		#ifdef TIMERS
			tbStartTimer ( 0,"Pop update");
		#endif
		
		// if streaming took too long don't do population
#ifndef DEBUG
		if(millisecondTimer < MILLISECOND_MAXLOAD)
//		if (1)		// For now always create peds. The streaming is so crap we don't get any peds.
		{
#endif
			// time population
			millisecondTimer = CTimer::GetCurrentTimeInMilleseconds();
			CPopulation::Update(true);
			millisecondTimer = CTimer::GetCurrentTimeInMilleseconds() - millisecondTimer;
#ifndef DEBUG
		}
		else
		{
			CPopulation::Update(false);	// Dont add any peds because streaming took too long
		}
#endif
		
		#ifdef TIMERS
			tbEndTimer ( "Pop update");
		#endif

		#ifdef TIMERS
			tbStartTimer ( 0,"Weapons");
		#endif
		CWeapon::UpdateWeapons();

		if(!CCutsceneMgr::IsCutsceneRunning())
		{
			CTheCarGenerators::Process();
			//CThePedGenerators::Process();
		}

		if (!CReplay::ReplayGoingOn()) CCranes::UpdateCranes();

		CClouds::Update();

		CMovingThings::Update();
		CWaterCannons::Update();
		
//		CWaterLevel::HandleSeaLifeForms();

		CUserDisplay::Process();

		#ifdef TIMERS
			tbEndTimer ( "Weapons");
		#endif

		CReplay::Update();


		CWorld::Process();
		
		ProcessDebugCarCheats();

		ProcessDebugMissionSkip();
	    
		g_LoadMonitor.EndFrame();
			
		#ifdef TIMERS
			tbStartTimer ( 0,"Acc&fx");		
		#endif

		if (!CTimer::bSkipProcessThisFrame)
		{
		//GetAccidentManager()->UpdateAccidentRecord();not used anymore
//		CPacManPickups::Update();
		CPickups::Update();
		CCarCtrl::PruneVehiclesOfInterest();
//		CPowerPoints::Update(); Not used anymore
		IFWEARENOTINNETWORKGAME(CGarages::Update();)
		CEntryExitManager::Update();
		CStuntJumpManager::Update();
		CBirds::Update();
//		CRubbish::Update();
		CSpecialFX::Update();
		CRopes::Update();
		}

		CPostEffects::Update();	
		#ifdef TIMERS
			tbEndTimer ( "Acc&fx");
		#endif
		
		
		#ifdef TIMERS
			tbStartTimer ( 0,"Cam&Zn&Log");
		#endif
		

		// Moved here by Les for blur - 11/08/01
		CTimeCycle::Update();
		CPopCycle::Update();


#ifndef FINAL
		g_DebugFollowCam.Process();
#endif

		// If camera is in idle mode, scan for interesting entities before calling TheCamera.Process()
		if(g_InterestingEvents.IsActive())
		{
			g_InterestingEvents.ScanForNearbyEntities();
		}
	
		if (CReplay::ShouldStandardCameraBeProcessed())
		{
			#ifdef NEW_CAMERA // migrated the initialisation of the pools so the camera can use them
				#include "CameraMain.h"
				CameraManagersProcess();
			#else
				TheCamera.Process();		// Must be called AFTER CWorld::Process to avoid glitch. (don't know why, must be RW thing)
			#endif
		}
		else
		{
			TheCamera.ProcessFade();	// Fades should be processed even during replays.
		}

		
		IFWEARENOTINNETWORKGAME(CCullZones::Update();)
		
		if (!CReplay::ReplayGoingOn())
		{
			CGameLogic::Update();
			CGangWars::Update();
		}
		CConversations::Update();
		CPedToPlayerConversations::Update();
#ifdef GTA_NETWORK
		CNetGames::Update();
#endif		
		CBridge::Update();

		#ifdef TIMERS
			tbEndTimer ( "Cam&Zn&Log");
		#endif
					
		#ifdef TIMERS
			tbStartTimer ( 0,"Cor&Shad");	
		#endif
		CCoronas::DoSunAndMoon();
		CCoronas::Update();
//		CShadows::UpdateStaticShadows();			// MN - moved just before the RenderStaticShadows to solve some ordering problems
		CShadows::UpdatePermanentShadows();
//		gPhoneInfo.Update();
		#ifdef TIMERS
			tbEndTimer ( "Cor&Shad");
		#endif

//		CWeather::AddSmog();

#ifdef USE_PLANT_MGR
		#ifdef TIMERS
			tbStartTimer ( 0,"CPlantMgr::Update");	
		#endif

		CPlantMgr::Update(TheCamera.GetPosition());

		#ifdef TIMERS
			tbEndTimer ( "CPlantMgr::Update");
		#endif
#endif

	
		#ifdef USE_CVBUILDING_RENDERER
			#if defined(GTA_PS2)
				CVuCustomBuildingRenderer::Update();
			#endif
			#if defined(GTA_PC) || defined(GTA_XBOX)
				CCustomBuildingRenderer::Update();
			#endif
		#endif	
	
		#ifdef TIMERS
			tbStartTimer ( 0,"CVolShadow::Update");	
		#endif
		CMemInfo::EnterBlock(GAME_MEM_ID);

		#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
			const float volshadFrontXYMult = 2.0f; //1.0f;		// angle of cast shadows is a bit more higher (shadows cover bigger area around objects now)

			// this is directional light in the game:
			const CVector dirLight(	CTimeCycle::m_fShadowFrontX[CTimeCycle::m_CurrentStoredValue] * volshadFrontXYMult,
									CTimeCycle::m_fShadowFrontY[CTimeCycle::m_CurrentStoredValue] * volshadFrontXYMult,
									-1.5f);
			CVolumetricShadowMgr::SetDirectionalLight(dirLight);

			CVolumetricShadowMgr::Update(TheCamera.GetPosition());
		#endif

		CMemInfo::ExitBlock();
		#ifdef TIMERS
			tbEndTimer ( "CVolShadow::Update");
		#endif


		#ifdef TIMERS
			tbStartTimer ( 0,"Cars");	
		#endif
		CMemInfo::EnterBlock(CARS_MEM_ID);

	
		if (!CReplay::ReplayGoingOn())
		{
			// if streaming or population took too long don't do carctrl
#ifndef DEBUG
			if(millisecondTimer < MILLISECOND_MAXLOAD)
//			if (1)		// Took this out. millisecondTimer can be 2 for long periods resulting in no cars and loads of bugs (Obbe)
			{
#endif
				CCarCtrl::GenerateRandomCars();
#ifndef DEBUG
			}
#endif
			
			CRoadBlocks::GenerateRoadBlocks();
			CCarCtrl::RemoveDistantCars();
			CCarCtrl::RemoveCarsIfThePoolGetsFull();
		}

		IFWEAREINNETWORKGAME( CGameNet::PostProcess(); )

		
		CMemInfo::ExitBlock();
		#ifdef TIMERS
			tbEndTimer ( "Cars");
		#endif
		
		// MN - FX SYSTEM -------------------------------
		
		#ifdef TIMERS
			tbStartTimer(0,"FxSystem Update");
		#endif
	/*
		// attach a system to the player after a few frames
		// (wait to make sure the player is the real player!)
		static int iteration = 0;
		
		if (iteration++ == 500)
		{
			RwV3d pos = {0.0f, 0.0f, 0.0f};
			g_fxSysTest = g_fxMan.CreateFxSystem("bouncers", &pos, FindPlayerPed()->GetRwMatrix());
			g_fxSysTest->Play();
		}
	*/
		
		CMemInfo::EnterBlock(FX_MEM_ID);	
		g_fx.Update(TheCamera.m_pRwCamera, CTimer::GetTimeStepInSeconds());
		CMemInfo::ExitBlock();	

			
		#ifdef TIMERS
			tbEndTimer("FxSystem Update");
		#endif
		
		// ----------------------------------------------


		// MN - BREAKABLE OBJECTS -----------------------

		CMemInfo::EnterBlock(BREAKABLE_OBJECTS);	
		g_breakMan.Update(CTimer::GetTimeStep());
		CMemInfo::ExitBlock();	
		
		// ----------------------------------------------
		
		
		// MN - PROCEDURAL INTERIORS --------------------

		#ifdef TIMERS
			tbStartTimer(0,"Interior Update");
		#endif
		CMemInfo::EnterBlock(PROCEDURAL_INTERIORS);	
		g_interiorMan.Update();
		CMemInfo::ExitBlock();	
		#ifdef TIMERS
			tbEndTimer("Interior Update");
		#endif
		
		// ----------------------------------------------
		
		
		// MN - PROCEDURAL OBJECTS ----------------------
		#ifdef TIMERS
			tbStartTimer(0,"ProcObjects Update");
		#endif
		CMemInfo::EnterBlock(PROCEDURAL_OBJECTS);	
		g_procObjMan.Update();
			CMemInfo::ExitBlock();	
		#ifdef TIMERS
			tbEndTimer("ProcObjects Update");
		#endif
		// ----------------------------------------------
		
		
		// MN - WATER CREATURES -------------------------
		#ifdef TIMERS
			tbStartTimer(0,"WtrCreature Update");
		#endif
		CMemInfo::EnterBlock(WATER_CREATURES);	
		g_waterCreatureMan.Update(CTimer::GetTimeStepInSeconds());
			CMemInfo::ExitBlock();	
		#ifdef TIMERS
			tbEndTimer("WaterCreatures Update");
		#endif
		// ----------------------------------------------
	}
/*
	else
	{
#ifndef FINAL
//	Fiddly code to allow updating of the screen during Script Debugger Single Step mode
		if (CTimer::m_CodePause && CScriptDebugger::bSingleStep)
		{
			#ifdef TIMERS
				tbStartTimer ( 0,"Scripts");
			#endif
			
			CMemInfo::EnterBlock(SCRIPT_MEM_ID);
			IFWEARENOTINNETWORKGAME(CTheScripts::Process();)	// Must happen before CWorld::Process so that stuff can be initialised before the game runs.
			CMemInfo::ExitBlock();

			#ifdef TIMERS
				tbEndTimer ( "Scripts");
			#endif
		}
#endif
	}
*/

/*
#ifdef GTA_PC
	else
	{
		CTimeCycle::Update(); // Make Sure the m_fCurrentFarClip != 0
	}
#endif
*/
#ifdef DO_MEM_CHECKSUM_TEST
	CCodeChecksum::DoTest();
#endif

	#ifdef TIMERS
		tbStartTimer ( 0,"PreRenderWater");
	#endif
	CWaterLevel::PreRenderWater();
	#ifdef TIMERS
		tbEndTimer ( "PreRenderWater");
	#endif
	
} // end - CGame::Process



//
// name:		GenerateTempPedAtStartOfNetworkGame
// description:	Creates a temporary player so that the network game doesn't crash
//              on start-up

void CGame::GenerateTempPedAtStartOfNetworkGame()
{
#ifdef GTA_NETWORK
	// Load player model
	if (!CStreaming::HasModelLoaded(0))
	{
		CStreaming::RequestSpecialModel(0, "player", STRFLAG_DONTDELETE|STRFLAG_FORCE_LOAD);
		CStreaming::LoadAllRequestedModels();
	}
	CPlayerPed::SetupPlayerPed(0);
	CWorld::Players[0].pPed->CharCreatedBy = MISSION_CHAR;
	CWorld::Players[0].pPed->SetPosition(1062.5f, -938.0f, 15.0f);

	CVector	StartCoors;
	CNetGames::SelectSafeStartPoint(&StartCoors, CWorld::Players[0].Team);
	CWorld::Players[0].pPed->SetPosition(StartCoors);
#endif
}








static int32 gNumMemMoved;
//
// name:		MoveMem
// description:	Tries to move memory. Returns if successful
bool MoveMem(void** ppMem)
{
	void* pNewMem;
	
	if(*ppMem == NULL)
		return false;
	
	gNumMemMoved++;	
	pNewMem = CMemoryMgr::MoveMemory(*ppMem);
	if(pNewMem != *ppMem)
	{
		*ppMem = pNewMem;
		return true;
	}

	return false;
}


//
// name:		MoveColModelMemory
// description:	move memory in a colmodel if you can
bool MoveColModelMemory(CColModel& colModel, bool bQuit)
{
	CCollisionData* pColData = colModel.GetCollisionData();
	if(pColData == NULL)
		return false;
		
	if(colModel.m_useSingleAlloc)
	{
		CCollisionData* pNewColData = (CCollisionData*)CMemoryMgr::MoveMemory(pColData);
		if(pNewColData != pColData)
		{
			int32 offset = (uint32)pNewColData - (uint32)pColData;

			colModel.m_pColData = pNewColData;
			// offset pointers
			if(pNewColData->m_pSphereArray)
				pNewColData->m_pSphereArray = (CColSphere*)((uint32)pNewColData->m_pSphereArray + offset);
			if(pNewColData->m_pBoxArray)
				pNewColData->m_pBoxArray = (CColBox*)((uint32)pNewColData->m_pBoxArray + offset);
			if(pNewColData->m_pLineArray)
				pNewColData->m_pLineArray = (CColLine*)((uint32)pNewColData->m_pLineArray + offset);
			if(pNewColData->m_pTriCompressedVectorArray)
				pNewColData->m_pTriCompressedVectorArray = (CompressedVector*)((uint32)pNewColData->m_pTriCompressedVectorArray + offset);
			if(pNewColData->m_pTriangleArray)
				pNewColData->m_pTriangleArray = (CColTriangle*)((uint32)pNewColData->m_pTriangleArray + offset);

			// change pointer in link list
			if(pNewColData->m_pTrianglePlaneArray)
			{
				CLink<CCollisionData*>* pLink = pNewColData->GetLinkPtr();
				pLink->item = pNewColData;
			}
			
			pColData = pNewColData;
			
			if(bQuit)
				return true;
		}
	}	
	else
	{
		if(MoveMem((void**)&pColData->m_pSphereArray) && bQuit)
			return true;
		if(MoveMem((void**)&pColData->m_pLineArray) && bQuit)
			return true;
		if(MoveMem((void**)&pColData->m_pBoxArray) && bQuit)
			return true;
		if(MoveMem((void**)&pColData->m_pTriCompressedVectorArray) && bQuit)
			return true;
		if(MoveMem((void**)&pColData->m_pTriangleArray) && bQuit)
			return true;
	}		
	if(MoveMem((void**)&pColData->m_pTrianglePlaneArray) && bQuit)
		return true;
	return false;	
}

//
// name:		MoveGeometryMemory
// description:	Move geometry memory of objects that are either not instanced or instanced once
RpGeometry* MoveGeometryMemory(RpGeometry* pGeom)
{
	RpGeometry *pNewGeom = (RpGeometry*)CMemoryMgr::MoveMemory(pGeom);
	int32 offset;
	
	if(pNewGeom != pGeom)
	{
		offset = (uint32)pNewGeom - (uint32)pGeom;
		pGeom = pNewGeom;
		if(pGeom->triangles)
			pGeom->triangles = (RpTriangle*)((uint32)pGeom->triangles + offset);
		if(pGeom->preLitLum)
			pGeom->preLitLum = (RwRGBA*)((uint32)pGeom->preLitLum + offset);
		if(pGeom->texCoords[0])
			pGeom->texCoords[0] = (RwTexCoords*)((uint32)pGeom->texCoords[0] + offset);
		if(pGeom->texCoords[1])
			pGeom->texCoords[1] = (RwTexCoords*)((uint32)pGeom->texCoords[1] + offset);
		pGeom->morphTarget[0].parentGeom = pGeom;
		return pGeom;
	}
	return NULL;
}



//
// name:		TidyUpModelInfo2
// description:	Move geometry memory about. Have to have all the atomics that reference this
//				geometry before I can do this so I am only doing it for modelinfo's that are 
//				used once in the level
bool TidyUpModelInfo2(CEntity* pEntity, bool bQuit)
{
	CAtomicModelInfo* pModelInfo = CModelInfo::GetModelInfo(pEntity->GetModelIndex())->AsAtomicModelInfoPtr();
	if(pModelInfo == NULL)
		return false;
		
	RpAtomic* pAtomic = (RpAtomic*)pEntity->GetRwObject();	
	if(pModelInfo->GetNumRefs() > 1)
		return false;
	
	RpAtomic* pLodAtomic = pModelInfo->GetAtomic();
	RpGeometry *pGeom = MoveGeometryMemory(RpAtomicGetGeometry(pLodAtomic));
	if(pGeom)
	{
		if(RpAtomicGetGeometry(pAtomic) == RpAtomicGetGeometry(pLodAtomic))
			pAtomic->geometry = pGeom;
		pLodAtomic->geometry = pGeom;	
		if(bQuit)
			return true;
	}
	return false;
}

static RwBool OffsetPointers(RwUInt32 *pData, uint32 offset)
{
    RwBool      done = FALSE;
    RwBool      ref = TRUE;

    while (!done)
    {
        RwInt32 qwc;
        RwInt32 id;

        /* Bits 0:15 are the QWC */
        qwc = (*pData) & 0xFFFF;

        /* Bits 28:30 are the ID */
        id = ((*pData) >> 28) & 0x7;

        switch (id)
        {
        case 0: /* refe */
            done = TRUE;
            break;
        case 1: /* cnt */
            {
                /* Jump to next tag */
                pData += ((qwc + 1) * 4);
            }
            break;
        case 2: /* next */
            break;
        case 3: /* ref */
            {
                pData++;

                /* Convert the offset to a pointer */
                *((uint8 **)(pData)) += offset;

                /* Jump to next tag */
                pData += 3;

                ref = FALSE;
            }
            break;
        case 4: /* refs */
            break;
        case 5: /* call */
            break;
        case 6: /* ret */
            done = TRUE;
            break;
        case 7: /* end */
            done = TRUE;
            break;
        default:
            break;
        }
    }
	return ref;
}

#ifdef GTA_PS2

//
// name:		MoveMeshMemory
// description:	Move instanced data (scary shit)
bool MoveMeshMemory(RpGeometry* pGeometry, bool bQuit)
{
	int32 i;
	
	if(!(RpGeometryGetFlags(pGeometry) & rpGEOMETRYNATIVE))
		return false;
		
    /* Assume a mesh for every material - This is very scary */
    RwMeshCache* pMeshCache = RWMESHCACHEFROMGEOMETRY(pGeometry);
	if(pMeshCache == NULL)
		return false;
	// for each mesh
    for (i = 0; i < pMeshCache->lengthOfMeshesArray; i++)
    {
        RwResEntry          **ppResEntry;
        RwResEntry          *pNewResEntry;
        rwPS2ResEntryHeader *pPs2ResHeader;

		// is there any instanced data
        ppResEntry = rwMeshCacheGetEntryRef(pMeshCache, i);
        if(*ppResEntry == NULL)
        	continue;
        	
        // is this data being rendered	
        pPs2ResHeader = (rwPS2ResEntryHeader *)(*ppResEntry + 1);
        if(pPs2ResHeader->refCnt > 0)
        	continue;
        void* pData = pPs2ResHeader->data;
        if(pData == NULL)
        {
        	pNewResEntry = (RwResEntry*)CMemoryMgr::MoveMemory(*ppResEntry);
        	if(*ppResEntry != pNewResEntry)
        	{
	        	*ppResEntry = pNewResEntry;
	        	if(bQuit)
	        		return true;
	        }
		}       
        pNewResEntry = (RwResEntry*)CMemoryMgr::MoveMemory(*ppResEntry, &pData, 64);
		if(pNewResEntry != *ppResEntry)
		{
			int32 offset;
			*ppResEntry = pNewResEntry;
	        pPs2ResHeader = (rwPS2ResEntryHeader *)(*ppResEntry + 1);
	        offset = (uint8*)pData - (uint8*)pPs2ResHeader->data;
			pPs2ResHeader->data = (u_long128*)pData;
			
			OffsetPointers((RwUInt32 *)pPs2ResHeader->data, offset);
        	if(bQuit)
        		return true;
		}
    }
    
    return false;
}


//
// name:		MoveAtomicMemory
// description:	Move atomic memory if you can
bool MoveAtomicMemory(RpAtomic* pAtomic, bool bQuit)
{
	uint8* pOrigMesh;
	RpGeometry *pGeom = RpAtomicGetGeometry(pAtomic);
	int32 offset;

	if(MoveMem((void**)&pGeom->matList.materials) && bQuit)
		return true;
	RpMorphTarget *pMorphTarget = (RpMorphTarget*)CMemoryMgr::MoveMemory(pGeom->morphTarget);
	
	if(pMorphTarget != pGeom->morphTarget)
	{
		offset = (uint32)pMorphTarget - (uint32)pGeom->morphTarget;
		pGeom->morphTarget = pMorphTarget;
		if(pGeom->morphTarget[0].verts)
			pGeom->morphTarget[0].verts = (RwV3d*)((uint32)pGeom->morphTarget[0].verts + offset);
		if(pGeom->morphTarget[0].normals)
			pGeom->morphTarget[0].normals = (RwV3d*)((uint32)pGeom->morphTarget[0].normals + offset);
		if(bQuit)
			return true;
	}
	
	pOrigMesh = (uint8*)pGeom->mesh;
	if(MoveMem((void**)&pGeom->mesh))
	{
		RpMesh* pMesh = (RpMesh*)(pGeom->mesh + 1);
		
		offset = ((uint8*)pGeom->mesh) - pOrigMesh;
		// offset all the indices pointers
		for(int32 i=0; i<pGeom->mesh->numMeshes; i++)
		{
			if((pMesh+i)->indices)
				(pMesh+i)->indices = (RxVertexIndex *)(((uint8*)(pMesh+i)->indices) + offset);
		}	
		if(bQuit)
			return true;
	}
	
//	if(RpSkinGeometryGetSkin(pGeom))
//		return false;
//	else	
		return MoveMeshMemory(pGeom, bQuit);
}


RpAtomic* MoveAtomicMemoryCB(RpAtomic* pAtomic, void* data)
{
	if(data)
	{
		if(MoveAtomicMemory(pAtomic, true))
		{
			*((bool*)data) = TRUE;
			return NULL;
		}	
	}
	else
	{
		MoveAtomicMemory(pAtomic, false);
	}
	return pAtomic;
}

//
// name:		MoveClumpMemory
// description:	Move atomic memory for all the atomics in this clump
bool MoveClumpMemory(RpClump* pClump, bool bWait)
{
	bool moveMem = FALSE;
	if(bWait)
		RpClumpForAllAtomics(pClump, &MoveAtomicMemoryCB, &moveMem);
	else	
		RpClumpForAllAtomics(pClump, &MoveAtomicMemoryCB, NULL);
	return moveMem;
}


//
// name:		TidyUpModelInfo
// description:	Tidy up memory related to one modelinfo
bool TidyUpModelInfo(CBaseModelInfo* pModelInfo, bool bWait)
{
	// do collision
	if(pModelInfo->HasColModel() && pModelInfo->DoesItOwnTheColModel())
	{
		CColModel& colModel = pModelInfo->GetColModel();

		if(MoveColModelMemory(colModel, bWait))
			return true;
	}		
	// do geometry
	RwObject* pRwObject = pModelInfo->GetRwObject();
	if(pRwObject)
	{
		if(RwObjectGetType(pRwObject) == rpATOMIC)
		{
			CAtomicModelInfo* pAtomicModelInfo = (CAtomicModelInfo*)pModelInfo;
			if(MoveAtomicMemory(pAtomicModelInfo->GetAtomic(), bWait))
				return true;
		}		
		if(RwObjectGetType(pRwObject) == rpCLUMP)
		{
			if(MoveClumpMemory((RpClump*)pRwObject, bWait))
				return true;
		}
	}
	
	// do ped hit colmodels
	if(pModelInfo->GetModelType() == MI_TYPE_PED)
	{
		CColModel* pColModel = ((CPedModelInfo*)pModelInfo)->GetHitColModel();
		if(pColModel)
		{
			if(MoveColModelMemory(*pColModel, bWait))
				 return true;
		}
	}
	return false;		
}

//
// name:		GetPointerToAddressFromDmaTag
// description:	Get pointer to the address component in a DMA tag
uint8** GetPointerToAddressFromTag(RwInt128* pTag)
{
	return &((uint8**)pTag)[1];
}

//
// name:		MoveTextureMemoryCB
// description:	Moves raster data about. Has to do lots of scary moving of pointers 
//				about. I had to guess a wee bit about what values to change.
RwTexture* MoveTextureMemoryCB(RwTexture* pTexture, void* data)
{
	RwRaster* pRaster = RwTextureGetRaster(pTexture);
	_SkyRasterExt* pRasterExt = &EXTFROMRASTER(pRaster);
	uint8** pPtrs[10];

	int32 pixelD, palD;
	RwUInt8* pNewPixels = NULL;
	
	if(pRaster->originalPixels)
	{
		if(pRaster->originalPixels == pRaster->cpPixels)
			return pTexture;
		// if texture is currently being rendered
		if(pRasterExt->dmaRefCount != 0)
			return pTexture;
		if(pRaster->privateFlags & rwRASTERLOCKED)
			return pTexture;

		/*
		This is taken directly from a support reply
		cpPixels is laid out as follows:

		This section always exists:
		- pktSize (1w)
		- data ptr (1w)
		- pktSize (1W)
		- 1W gap
		Then one of these for each different buffer width transferred:
		- DMATag (4w)
		- GIFTag (4w)
		- BITBLITBUF reg setting
		- DMATag (with mip level address)
		Then one of these if there's a palette: 
		- DMATag (4w)
		- GIFTag (4w)
		- BITBLITBUF reg setting
		- DMATag (with palette address)
		
		Then at the next 128-byte boundary, we start the GIFTags with the actual image data in them.
		*/
		RwUInt128* pTags = (RwUInt128*)pRaster->originalPixels;
		uint8*** ppPtrs = &pPtrs[0];
		int32 numAddresses = 1;

		// dma tag data pointer
		pPtrs[0] = GetPointerToAddressFromTag(pTags);
		pTags += 4;
		
		// then for every four tags until we hit the image data we store a giftag address
		while((uint8*)pTags < *pPtrs[0])
		{
			pPtrs[numAddresses] = GetPointerToAddressFromTag(pTags);
			// if address is out of range then break out of loop
			if(*pPtrs[numAddresses] < pRaster->originalPixels || *pPtrs[numAddresses] > (uint8*)0x8000000)
				break;
			pTags += 4;
			numAddresses++;		
		}
		
		// store pixel and palette offsets
		pixelD = pRaster->cpPixels - pRaster->originalPixels;
		if(pRaster->palette)
			palD = pRaster->palette - pRaster->originalPixels;
		
		gNumMemMoved++;
		pNewPixels = (RwUInt8*)CMemoryMgr::MoveMemory(pRaster->originalPixels);
		if(pNewPixels != pRaster->originalPixels)
		{
	        int32 offset = (uint8*)pNewPixels - pRaster->originalPixels;
		
			// Use offsets from originalPixels pointer to update the DMA and gif tags.
			for(int32 i=0; i<numAddresses; i++)
			{
				pPtrs[i] += offset>>2;
				*pPtrs[i] += offset;
			}	
		
			// update pointers in raster
			pRaster->cpPixels = (uint8*)pRaster->cpPixels + offset;
			if(pRaster->palette)
				pRaster->palette = (uint8*)pRaster->palette + offset;
				
			pRaster->originalPixels = pNewPixels;
			
			// if data is a non NULL pointer to a boolean set it to true and quit
			if((bool*)data)
			{
				*((bool*)data) = TRUE;
				return NULL;
			}	
		}
	}	
	
	return pTexture;
}

#endif // GTA_PS2
//
// name:		DrasticTidyUpMemory
// description:	Move game structures about to defrag memory. If there isn't big enough free blocks
// 				then remove parts of the game and reload them
void CGame::DrasticTidyUpMemory(bool bWait)
{
#if defined(GTA_PC) && !defined(OSW)

#ifndef FINALBUILD	
	DEBUGLOG1("Index buffers use %d memory\n", D3DPoolsGetIndexBufferMemoryUsage());
	int32 t = CTimer::GetCurrentTimeInMilleseconds();
#endif
	
	D3DPoolsReleaseTextures(100);
	D3DPoolsReleaseIndexBuffers(200);

#ifndef FINALBUILD	
	t = CTimer::GetCurrentTimeInMilleseconds() - t; 
	DEBUGLOG1("Took %d milleseconds\n", t);
	DEBUGLOG1("Index buffers use %d memory\n", D3DPoolsGetIndexBufferMemoryUsage());
#endif
#endif	
	
#ifdef USE_STD_MALLOC
	return;
#endif

	static bool bInside = false;
	bool bRemoveCollision = false;
	bool bRemoveBigBuildings = false;

	if(bInside)
		return;
	bInside = true;
		
	TidyUpMemory(true, bWait);
	
	if(CMemoryMgr::GetLargestFreeBlock() < 250000)
	{
		CColStore::RemoveAllCollision();
		bRemoveCollision = true;
		TidyUpMemory(true, bWait);
	}

	if(CMemoryMgr::GetLargestFreeBlock() < 250000)
	{
		CStreaming::RemoveBigBuildings();
		bRemoveBigBuildings = true;
		TidyUpMemory(true, bWait);
	}

	if(bRemoveCollision)
	{
		CColStore::LoadCollision(FindPlayerCoors(), true);
	}
	
	if(bRemoveBigBuildings)
		CStreaming::RequestBigBuildings(TheCamera.GetPosition());

	CStreaming::LoadAllRequestedModels();

	bInside = false;
}

//
// name:		TidyUpMemory
// description:	Move game structures about to defrag memory
void CGame::TidyUpMemory(bool bMoveTextures, bool bWait)
{
#if defined(GTA_PC) && !defined(OSW)

	// need to determine if game is active (not in startup frontend) - can't tidy memory otherwise!
	if (!FindPlayerPed(0))
	{
		return;
	}

	if(bWait)
	{
#ifndef FINALBUILD	
		DEBUGLOG2("Index buffers use %d memory, Textures use %d memory\n", 
			D3DPoolsGetIndexBufferMemoryUsage(), D3DPoolsGetTextureMemoryUsage());
		int32 t = CTimer::GetCurrentTimeInMilleseconds();
#endif
		
		D3DPoolsReleaseTextures(100);
		D3DPoolsReleaseIndexBuffers(200);

#ifndef FINALBUILD	
		t = CTimer::GetCurrentTimeInMilleseconds() - t; 
		DEBUGLOG1("Took %d milleseconds\n", t);
		DEBUGLOG2("Index buffers use %d memory, Textures use %d memory\n", 
			D3DPoolsGetIndexBufferMemoryUsage(), D3DPoolsGetTextureMemoryUsage());
#endif
	}	
#endif	//GTA_PC
	
	int32 i, j;
	
#ifdef USE_STD_MALLOC
	return;
#endif

#ifndef MASTER
	DEBUGLOG1("Largest free block before tidy %d\n", CMemoryMgr::GetLargestFreeBlock());
#endif

	CMemoryMgr::SetRestrictMemoryMove(false);
	// If move textures flag is set. Then world is not being displayed and we can
	// move textures about

// only move textures on ps2
#ifdef GTA_PS2
	if(bMoveTextures)
	{
		if(bWait)
		{
			// wait for DMA queue to empty
			for (i = 0; i < 2; i++)
			{
			  RwCameraBeginUpdate(Scene.camera);
			  RwCameraEndUpdate(Scene.camera);
			  RwCameraShowRaster(Scene.camera, NULL, 0);
			}
		}
		int32 fontTxd = CTxdStore::FindTxdSlot("fonts");
		for(i=0; i<CTxdStore::GetSize(); i++)
		{
			if(CTxdStore::IsValidSlot(i) && i != fontTxd)
			{
				RwTexDictionary* pTxd = CTxdStore::GetTxd(i);
				if(pTxd)
					RwTexDictionaryForAllTextures(pTxd, &MoveTextureMemoryCB, NULL);
			}
		}
	}
#endif //GTA_PS2

	// For all the animation sequences
	for(i=0; i<ANIMMANAGER_MAXNOOFANIMATIONS; i++)
	{
		CAnimBlendHierarchy* pAnim = CAnimManager::GetAnimation(i);
		if(pAnim == NULL)
			continue;
		pAnim->MoveMemory();
	}
	
//only do this on ps2
#if defined (GTA_PS2)
	// do collision and geometry allocated memory
	for(i=0; i<NUM_MODEL_INFOS; i++)
	{
		CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(i);
		if(pModelInfo == NULL)
			continue;
		TidyUpModelInfo(pModelInfo, false);
	}
#endif //GTA_PS2

	// building pool	
	CBuildingPool& pool = CPools::GetBuildingPool();
	CBuilding* pBuilding;
	i = pool.GetSize();
	while(i--)
	{
		pBuilding = pool.GetSlot(i);
		if(pBuilding && pBuilding->GetRwObject())
			TidyUpModelInfo2(pBuilding, false);
	}
	
	// dummy pool
	CDummyPool& pool2 = CPools::GetDummyPool();
	CDummy* pDummy;
	i = pool2.GetSize();
	while(i--)
	{
		pDummy = pool2.GetSlot(i);
		if(pDummy && pDummy->GetRwObject())
			TidyUpModelInfo2(pDummy, false);
	}
	
	CMemoryMgr::SetRestrictMemoryMove(true);
	
#ifndef MASTER
	DEBUGLOG1("Largest free block after tidy %d\n", CMemoryMgr::GetLargestFreeBlock());
#endif	
}





//
// name:		ProcessTidyUpMemory
// description:	do a small bit of memory tidying. Called once every frame
void CGame::ProcessTidyUpMemory()
{
#ifdef USE_STD_MALLOC
	return;
#endif

	static int32 modelIndex = 0;
	static int32 animIndex = 0;
	static int32 txdIndex = 0;
	static int32 buildingIndex = 0;
	static int32 dummyIndex = 0;
	CBaseModelInfo* pModelInfo = NULL;
	CAnimBlendHierarchy* pAnim = NULL;
	RwTexDictionary* pTxd = NULL;
	int32 i, j;
	bool hasMoved = FALSE;
	
	gNumMemMoved = 0;

#ifdef GTA_PS2
	for(i=0; i<10; i++)
	{
		// get next model in memory
		do{
			pModelInfo = CModelInfo::GetModelInfo(modelIndex++);
			if(modelIndex >= NUM_MODEL_INFOS)
				modelIndex = 0;
		}while(pModelInfo == NULL);
		
		TidyUpModelInfo(pModelInfo, false);
	}
#endif //GTA_PS2

	for(i=0; i<10; i++)
	{
		CBuildingPool& pool = CPools::GetBuildingPool();
		CBuilding* pBuilding;
		// get next building that is loaded
		int32 count = 0;
		do{
			pBuilding = pool.GetSlot(buildingIndex++);
			if(buildingIndex >= pool.GetSize())
				buildingIndex = 0;
			count++;
			if(count > 1000)
			{
				pBuilding = NULL;
				break;	
			}	
		}while(pBuilding == NULL || pBuilding->GetRwObject() == NULL);
		
		if(pBuilding == NULL)
			break;
		TidyUpModelInfo2(pBuilding, false);	
	}
	
	for(i=0; i<10; i++)
	{
		CDummyPool& pool = CPools::GetDummyPool();
		CDummy* pDummy;
		// get next building that is loaded
		int32 count = 0;
		do{
			pDummy = pool.GetSlot(dummyIndex++);
			if(dummyIndex >= pool.GetSize())
				dummyIndex = 0;
			count++;
			if(count > 1000)
			{
				pDummy = NULL;
				break;	
			}	
		}while(pDummy == NULL || pDummy->GetRwObject() == NULL);
		
		if(pDummy == NULL)
			break;
		TidyUpModelInfo2(pDummy, false);	
	}

#if defined (GTA_PS2)
	for(i=0; i<3; i++)
	{
		if(gNumMemMoved > 80)
			break;
			
		// get next texture dictionary in memory
		int32 count = 0;
		do{
			if(CTxdStore::IsValidSlot(txdIndex))
				pTxd = CTxdStore::GetTxd(txdIndex);
			txdIndex++;
			if(txdIndex >= CTxdStore::GetSize())
				txdIndex = 0;
			count++;
			if(count > 1000)
				break;	
		}while(pTxd == NULL);
		
		if(pTxd == NULL)
			break;
		RwTexDictionaryForAllTextures(pTxd, &MoveTextureMemoryCB, NULL);
	
		//if(hasMoved)
		//	return;
	}	
#endif //GTA_PS2

	// get next anim in memory
	do{
		pAnim = CAnimManager::GetAnimation(animIndex++);
		if(animIndex >= ANIMMANAGER_MAXNOOFANIMATIONS)
			animIndex = 0;
	}while(pAnim == NULL);
	
	pAnim->MoveMemory();
}

bool CGame::CanSeeOutSideFromCurrArea()
{
	return (CGame::currArea == AREA_MAIN_MAP);
	
//	 ||
//			CGame::currArea == AREA_MALL ||
//			CGame::currArea == AREA_MANSION ||
//			CGame::currArea == AREA_HOTEL
//			);
}

bool CGame::CanSeeWaterFromCurrArea()
{
	return (CGame::currArea == AREA_MAIN_MAP || CGame::currArea == AREA_STRIP_CLUB);
		// Note: Strip club is the area code of madd dogs mansion. It has a pool in it.
	
//	 ||
//			CGame::currArea == AREA_MANSION ||
//			CGame::currArea == AREA_HOTEL
//			);
}

#ifdef GTA_PS2
/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ReloadDataFile
// PURPOSE :  calls the loading file function of the filename you pass.  This is
//			  called by the "reload X" command in Varconsole Userconsole.
//			  Any further loading functions can be added here by comparing the
//			  string passed in as below...
/////////////////////////////////////////////////////////////////////////////////
#ifndef CDROM

void ReloadDataFile(const char* pFilename)
{
	if(pFilename && *pFilename == '\0')
		pFilename = NULL;
	// took of the extension (.dat) for ease of typing in the command.
	if (pFilename == NULL || !strcmp(pFilename, "fonts.dat"))
	{
		CFont::LoadFontValues();
		VarConsole.AddNewTextInputToLast("Loading font.dat");
	}

	if (pFilename == NULL || !strcmp(pFilename, "popcycle.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CPopCycle::Initialise();
		VarConsole.AddNewTextInputToLast("Loading popcycle.dat");
	}

	if (pFilename == NULL || !strcmp(pFilename, "ar_stats.dat"))
	{
		CStats::LoadActionReactionStats();
		VarConsole.AddNewTextInputToLast("Loading ar_stats.dat");
	}

	if (pFilename == NULL || !strcmp(pFilename, "statdisp.dat"))
	{
		CStats::LoadStatUpdateConditions();
		VarConsole.AddNewTextInputToLast("Loading statdisp.dat");
	}
	
	if (pFilename == NULL || !strcmp(pFilename, "pedgrp.dat"))
	{
		CPopulation::LoadPedGroups();
		VarConsole.AddNewTextInputToLast("Loading pedgrp.dat");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "cargrp.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CPopulation::LoadCarGroups();
		VarConsole.AddNewTextInputToLast("Loading cargrp.dat");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "american.gxt"))
	{
		Char MissionTextToReload[MISSION_NAME_LENGTH];
		TheText.GetNameOfLoadedMissionText(MissionTextToReload);
		TheText.Load();
		if (MissionTextToReload[0] != 0)
		{
			TheText.LoadMissionText(MissionTextToReload);
		}
		VarConsole.AddNewTextInputToLast("Loading american.gxt");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "weapon.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CWeaponInfo::LoadWeaponData();
		VarConsole.AddNewTextInputToLast("Loading weapon.dat");
	}	

/*	if (pFilename == NULL || !strcmp(pFilename, "surface.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CSurfaceTable::Initialise("data/surface.dat");
		VarConsole.AddNewTextInputToLast("Loading surface.dat");
	}	
*/
	if (pFilename == NULL || !strcmp(pFilename, "timecyc.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CTimeCycle::Initialise(false);
		VarConsole.AddNewTextInputToLast("Loading timecyc.dat");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "timecycp.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CTimeCycle::Initialise(true);
		VarConsole.AddNewTextInputToLast("Loading timecycp.dat");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "handling.cfg"))
	{
		LOAD_CODE_OVERLAY(init);
		mod_HandlingManager.Initialise();
		if(FindPlayerVehicle())
		{
			FindPlayerVehicle()->SetupSuspensionLines();
			tHandlingData* pHandling = FindPlayerVehicle()->pHandling;
			FindPlayerVehicle()->hFlagsLocal = pHandling->hFlags;
			FindPlayerVehicle()->m_fMass = pHandling->fMass;
			FindPlayerVehicle()->m_fTurnMass = pHandling->fTurnMass;
			if(pHandling->fDragCoeff > 0.01f)
				FindPlayerVehicle()->m_fAirResistance = 0.5f * pHandling->fDragCoeff / GAME_AIR_RESISTANCE_MASS;
			else
				FindPlayerVehicle()->m_fAirResistance = pHandling->fDragCoeff;

			if(FindPlayerVehicle()->m_pVehicleBeingTowed)
			{
				FindPlayerVehicle()->m_pVehicleBeingTowed->SetupSuspensionLines();
				pHandling = FindPlayerVehicle()->m_pVehicleBeingTowed->pHandling;
				FindPlayerVehicle()->m_pVehicleBeingTowed->m_fMass = pHandling->fMass;
				FindPlayerVehicle()->m_pVehicleBeingTowed->m_fTurnMass = pHandling->fTurnMass;
				if(pHandling->fDragCoeff > 0.01f)
					FindPlayerVehicle()->m_pVehicleBeingTowed->m_fAirResistance = 0.5f * pHandling->fDragCoeff / GAME_AIR_RESISTANCE_MASS;
				else
					FindPlayerVehicle()->m_pVehicleBeingTowed->m_fAirResistance = pHandling->fDragCoeff;
			}
		}
		VarConsole.AddNewTextInputToLast("Loading handling.cfg");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "melee.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CTaskSimpleFight::LoadMeleeData();
		VarConsole.AddNewTextInputToLast("Loading melee.dat");
	}	

/*
#ifndef FINAL
	if (pFilename == NULL || !strcmp(pFilename, "scriptdebug.txt"))
	{
		LOAD_CODE_OVERLAY(init);
		CScriptDebugger::LoadScriptDebugFile();
		VarConsole.AddNewTextInputToLast("Loading scriptdebug.txt");
	}
#endif
*/
#ifdef USE_PLANT_MGR
	if (pFilename == NULL || !strcmp(pFilename, "plant.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CPlantMgr::ReloadConfig();
		VarConsole.AddNewTextInputToLast("Loading plant.dat");
	}	
#endif // USE_PLANT_MGR

	if (pFilename == NULL || !strcmp(pFilename, "clothes.dat"))
	{
		LOAD_CODE_OVERLAY(init);
		CClothes::LoadClothesFile();
		VarConsole.AddNewTextInputToLast("Loading clothes.dat");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "carcols.dat"))
	{
		LOAD_CODE_OVERLAY(fileload);
		CVehicleModelInfo::LoadVehicleColours();
		VarConsole.AddNewTextInputToLast("Loading carcols.dat");
	}	

	if (pFilename == NULL || !strcmp(pFilename, "object.dat"))
	{
		LOAD_CODE_OVERLAY(fileload);
		CObjectData::Initialise("DATA\\OBJECT.DAT", true);
		VarConsole.AddNewTextInputToLast("Loading object.dat");
	}
	
	if (pFilename == NULL || !strcmp(pFilename, "procobj.dat"))
	{
		g_procObjMan.Exit();
		g_procObjMan.Init();
		VarConsole.AddNewTextInputToLast("Loading procobj.dat");
	}	
}
#endif
#endif //GTA_PS2


// If the debug car cheats is open, and the user selects an item, we'll do the selected item car cheat and close the widget.

void CGame::ProcessDebugCarCheats(){
   
	if ( CWidget* pCarCheats = CTouchInterface::GetWidget ( CTouchInterface::WIDGET_CAR_CHEATS ) ){
	    
		if ( CTouchInterface::IsReleased ( CTouchInterface::WIDGET_CAR_CHEATS ) ){	    
	
			char szEntry[MAX_PATH];
			int nSelectedEntry = ((CWidgetListText*)pCarCheats)->GetSelectedEntry ( szEntry );
			
			if ( nSelectedEntry != -1 ){

			    int ndx = MODELID_CAR_FIRST;

				for ( ndx = MODELID_CAR_FIRST; ndx < MODELID_CAR_LAST; ndx++ ){
				    if ( CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo ( ndx ) ){
					    if ( ! stricmp ( ((CVehicleModelInfo*)pModelInfo)->GetGameName(), szEntry ) ){
						    break;
						}
					}
			    }

            CCheat::VehicleCheat ( ndx ); 
            CTouchInterface::DeleteWidget ( CTouchInterface::WIDGET_CAR_CHEATS );
            }
		}
	}
}


void CGame::ProcessDebugMissionSkip(){
   
    if ( CWidget* pListText = CTouchInterface::GetWidget ( CTouchInterface::WIDGET_MISSION_SKIP ) ){
	    
		if ( CTouchInterface::IsReleased ( CTouchInterface::WIDGET_MISSION_SKIP ) ){	    
	
			char szEntry[MAX_PATH];
			int nSelectedEntry = ((CWidgetListText*)pListText)->GetSelectedEntry ( szEntry );
			
			if ( nSelectedEntry != -1 ){
				
				#ifdef OSW
				    OS_ReleaseLog("Selected %d",nSelectedEntry);
				#endif

				extern int SkipToMissionNumber;
				SkipToMissionNumber = nSelectedEntry;

				extern void ClearMissionSkip();
			    ClearMissionSkip();
            }
		}
	}
}