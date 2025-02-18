//
//
//
// 
//
//
//
//
// C headers
//$DW$#include <stdio.h>
//$DW$#include <stdlib.h>
//$DW$#include <string.h>
#ifdef GTA_PS2
// SCE headers
//$DW$#include <eeregs.h>
//$DW$#include <sifdev.h>
//$DW$#include <sifrpc.h>
//$DW$#include <libgraph.h>
//$DW$#include <libpc.h>
	#include <libscf.h>
// Metrowerks headers
#include <mwutils.h>
#endif	
#ifdef TEST_STATION
#include <mwutils_PS2AT.h>
#endif
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
#ifdef GTA_PS2
//$DW$#include <rppds.h>
#endif
//$DW$#include <rpskin.h>
//$DW$#include <rtcharse.h>
//$DW$#include <rttilerd.h>
//$DW$#include <rpmatfx.h>
//$DW$#include <rtbmp.h>

#ifndef OSW
#include <rpanisot.h>
#endif

//$DW$#include <rpadc.h>
//$DW$#include <rtdict.h>
#include <rpuvanim.h>
// Game headers
#include "MemoryMgr.h"
#include "Main.h"
#include "overlay.h"
#include "Draw.h"
#include "Sprite.h"
#include "Timer.h"
#include "Clock.h"
#include "Pad.h"
#include "FileMgr.h"
#include "stats.h"
#include "Debug.h"
#include "Profile.h"
#include "Game.h"
#include "Frontend.h"
#include "hud_colour.h"
#include "loadingscreen.h"
#include "World.h"
#include "Renderer.h"
#include "NodeNamePlugin.h"
#include "VisibilityPlugins.h"
#include "MemInfo.h"
#include "RpAnimBlend.h"
#include "PipelinePlugin.h"
#include "WidgetPlayerInfo.h"

#ifdef USE_AUDIOENGINE
#include "AudioEngine.h"
#else //USE_AUDIOENGINE
#include "DMAudio.h"
#include "tracklist.h"
#include "music.h"
#endif //USE_AUDIOENGINE

//!PC - these are stubbed in for PC
#include "ps2all.h"
#include "ps2keyboard.h"

#include "timecycle.h"
#include "general.h"
#include "coronas.h"
#include "Particle.h"
#include "radar.h"
#include "globals.h"
#include "shadows.h"
#include "WeaponEffects.h"
#include "Weather.h"
#include "Skidmarks.h"
#include "pointlights.h"
#include "animviewer.h"
#include "darkel.h"
#include "pools.h"
#include "pickups.h"
#include "rubbish.h"
#include "lights.h"
#include "setup.h"
#include "hud.h"
#include "font.h"
#include "cheat.h"
#include "fileloader.h"
#include "clouds.h"
#include "specialfx.h"
#include "streaming.h"
#include "streamingdebug.h"
#include "heli.h"
#include "camera.h"
#include "watercannon.h"
#include "txdstore.h"
#include "user.h"
#include "pad.h"
#include "collision.h"
#include "localisation.h"
#include "skeleton.h"

#include "RwCam.h"
#include "handy.h"

#include "messages.h" 
#include "script.h"
#include "waterlevel.h"
#include "garages.h"
#include "cdstream.h"
#include "playerped.h"
#include "fluff.h"
#include "glass.h"	
#include "antennas.h"
#include "cutscenemgr.h"
#include "font.h"
#include "zonecull.h"
#include "credits.h"
#include "mblur.h"
#include "console.h"
#include "edit.h"
#include "replay.h"

#ifdef GTA_NETWORK
	#include "gamenet.h"
	#include "netgames.h"
#endif

#include "GenericGameStorage.h"
#include "ropes.h"
#include "stunts.h"

#include "birds.h"
#include "gamelogic.h"
#include "clothes.h"
#include "CollisionPlugin.h"
#include "popcycle.h"
#include "record.h"
#include "cover.h"
#include "procobjects.h"
#include "occlusion.h"
#include "timebars.h"
#include "PedDebugVisualiser.h"
#include "lightning.h"
#include "audiozones.h"

#include "PostEffects.h"
#include "gridref.h"
#include "LoadMonitor.h"
#include "EntryExits.h"
#include "loadingscreen.h"

#ifdef GTA_PS2
#include "VuTurboSprite.h"
#endif

#include "TouchInterface.h"
#include "CPS2Peek.h"

#ifndef MASTER
#include "carcols.h"
	#include "VarConsole.h"
	
	#ifdef GTA_PS2
		#include "SN_Network_PS2.h"
		#include "BugstarPS2.h"
		#define MAX_NETWORK_UP_TRIES 20
		#define LOAD_NETWORK_IRX_ON_BUG
	#elif !defined(OSW)	// GTA_PS2
		#include "NetworkPC.h"
		#include "BugstarPC.h"
	#endif // GTA_PS2
	
	#include "debugbugvisualiser.h"
#endif

#ifndef FINAL
	#include "scriptdebug.h"
	#include "mapstats.h"
#endif

#ifdef GTA3_HASP
	#include "CHasp.h"
#endif//GTA3_HASP

#if defined (GTA_PC)
#include "protCheck.h"
#include "win.h"
#endif //GTA_PC


#ifdef GTA_PS2
	#include "skyfs.h"
	#include "poweroff.h"
	#include "eemacros.h"
	#include "MemoryCard.h"
	#include "MPegBufferVariables.h"
// new network library will hopfuly replace network_ps2.h
	#include "SN_Network_PS2.h"
	#include "network_ps2.h"
	#include "dma_post_fx.h"
	#include "CPS2Peek.h"
	#include "VuCustomBuildingRenderer.h"
	#include "VuCustomSkinPedRenderer.h"
#else //GTA_PS2

	#define MAX_JPEG_SIZE	(200*1024)		// changed from 80 to 200 * 1024 since the screen rez could be much higher on PC than PS2 so produce buger JPEGs - alex


	#include "CustomBuildingRenderer.h"
	#include "controllerconfig.h"	
#endif //GTA_PS2

//#define		USE_TEXTURE_CAMERA			1
//
#ifdef	USE_TEXTURE_CAMERA
	#include "TextureCamera.h"
	extern CTextureCamera	*textureCamera;
#endif//USE_TEXTURE_CAMERA


#define DEBUGTEXT
//#define DEBUG_MEMORY

// Defines for tile rendering
#define TILE_WIDTH	576
#define TILE_HEIGHT 432

#define NO_LOADING_TEXT


#ifdef GTA_PS2
	#ifdef GTA_SA
		#define USE_VU_CARFX_RENDERER		(1)
	#endif
#endif

#if (defined(GTA_PC) || defined(GTA_XBOX))
	#ifdef GTA_SA
		#define USE_PC_CARFX_RENDERER		(1)
	#endif
#endif


//!PC - working on PC versions of these now
#define USE_PLANT_MGR				(1)

#ifdef USE_VU_CARFX_RENDERER
	#include "VuCarFXRenderer.h"
#endif

#ifdef USE_PLANT_MGR
	#include "PlantsMgr.h"
#endif


//
//
//
//
#if (defined GTA_PC || defined GTA_XBOX) && !defined(OSW)
	#define USE_VOLUMETRIC_SHADOW_RENDERER	(1)
#endif

#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
	#include "VolumetricShadowMgr.h"
#endif


#define DW_ZFIGHT_TRICK_FIX	


// -- FX SYSTEM ---------------------------------
#include	"fx.h"
// ----------------------------------------------	

#include	"BreakObject.h"
#include	"InteriorManager.h"
#include	"Ragdoll.h"
#include	"IKChain.h"
#include	"CustomCarPlateMgr.h"

#if (defined(GTA_XBOX))
#include "dongle.h"
#include "XboxSave.h"
#endif //GTA_XBOX

//
// externs for screen positioning:
//

#ifdef GTA_PS2
//	uint8		work_buff[WORKBUFF_SIZE] __attribute__((aligned(64)));
	uint8		idleThreadStack[2048] __attribute__((aligned(16)));
#else
//	uint8		work_buff[WORKBUFF_SIZE];
//	uint8		idleThreadStack[2048];
#endif
//float		FrameTime;
RwBool SkipAllMPEGs = false;

#ifndef FINAL
bool gbDrawPolyDensity = false;
bool8 gbGrabScreenNow = false;
#endif

extern bool bDisplayPathsDebug;	// in pathfind.cpp

#ifdef GTA_PC
void Update(void);
#endif

bool8 g_bUsingAnimViewer;

static volatile int32 frameCount=0;

void PlayMPEG(char *name, bool bCanInterrupt, int size);
void InitMPEGPlayer();
void ShutdownMPEGPlayer();
RwBool Initialise3D(void* param = NULL);	// this file wee bit further along

void RenderPixelDensityForEntity(CEntity* pEntity);


// RenderWare externs
extern "C" {
#ifdef GTA_PS2
	extern uint32 skyGNumTexMemBlocks;
	extern uint32 skyGMaxTexMemBlocks;
	extern int sweMaxFlips;
#endif		
}

#if !defined (MSVC_COMPILER) && !defined(__GNUC__)
#ifdef TIMERS
	extern 	bool 	CHud::m_Wants_To_Draw_Hud;
#endif
#endif //MSVC_COMPILER

extern GlobalScene          Scene;		// In setup.cpp

//!PC - no postFx for PC as yet
#if defined (GTA_PS2)
extern CPostFx	postFx;
#endif //GTA_PS2


#ifndef MASTER
// bugstar stuff isnt in master build?
void JPegCompressScreenToBuffer(RwCamera* pCamera,char** buffer,unsigned int *size);
#endif

float	gHorZ;
CRGBA	gColourTop;
CRGBA	gColourBottom;
uint16 version_number;
char   version_name[64];
bool gameAlreadyInitialised = false;
static bool displayMemoryUsage = false;
static bool displayPoolsUsage = false;
static bool displayDetailedMemoryUsage = false;
static bool displayNumObjsRendered = false;
static bool displayFrameRate = false;
static bool displayLODMultiplier = false;
static bool displayPosition = TRUE;
static bool displayGridDetails = false;
static bool displayNumCarsAndPeds = false;
static bool gDisplayNumberMatrices = false;
static bool gDisplayDebugInfo = TRUE;
// initialise functions
void SystemInit();
void GameInit();
void PlayIntroMPEGs();
bool InitialMemoryCardAndPadChecks();

char* GetIOPModuleFilename(char* pFilename, const char* pModule);
void LoadSystemIOPModules();
void LoadGameIOPModules();
void LoadIOPKernel();
void CreateIdleThread();
RwBool PluginAttach();	// in main.cpp
void TheModelViewer();
void TheGame();
// render calls
void RenderScene();
void RenderDebugShit();
void RenderEffects();
void Render2dStuff();
void RenderMenus();
void Render2dStuffAfterFade();


#ifdef GTA_PS2

//
// name:		VBlankCounter
// description:	VBlank interrupt
int VBlankCounter(int arg)
{
	frameCount++;
	ExitHandler();
	return 0;
}

//
// name:		WaitVBlank
// description:	Wait for VBlank
extern "C" void WaitVBlank()
{
	int32 frame = frameCount;
	
	while(frame == frameCount) {}
}

// MAIN
// this stuff was stolen from sky.c

int main(int argc, char *argv[])
{
	mwInit();

#ifdef TEST_STATION
	mwAtInit(argv[0]);
	printf("%s\n", mwAtGetRFPServer());
#endif
	
	SystemInit();
	
#ifdef GTA_PS2PEEK_PROFILER
	if(!CPS2Peek::Initialise())
	{
		return(FALSE);
	}
#endif

    /* 
     * Initialize the platform independent data.
     * This will in turn initialize the platform specific data...
     */
    if( RsEventHandler(rsINITIALIZE, NULL) == rsEVENTERROR )
    {
        return FALSE;
    }

	IOPMEM_DEBUG("MemoryCardChecks")

	extern short gGenSpeechLookup[][173][2];
	uint32 tc=0;
//	bool found;
	
	/*for(int32 k=0; k<97; k++)
	{
		found = false;
		uint32 count = 0;
		for(int32 i=0; i<2; i++)
		{
			for(int32 j=0; j<173; j++)
			{
				if(gGenSpeechLookup[k][j][i] >= 255)
				{
					count++;
					tc++;
				}	
			}
		}
		if(count > 0)
			DEBUGLOG2("id %d  has %d over 255\n", k, count);
	}
	DEBUGLOG1("total count %d\n", tc);*/
	
	if (InitialMemoryCardAndPadChecks())
	{
		// a waring message box has to be displayed to tell the user there is no memory card or that it
		// is full : stuff has to be initialised beforehand in order to display this box
		LOAD_CODE_OVERLAY(init);
		GameInit();

	#ifdef FINALBUILD
		DelayThread(2000000);
		//	frameCount = 0;
		//	while(frameCount < 100) {} // can we justify the whole 100 frames here - reduce if possible please - DW
	#endif

		CGame::InitialiseEssentialsAfterRW();
		IOPMEM_DEBUG("InitialiseEssentialsAfterRW")
		
		// display warning message box
		FrontEndMenuManager.DrawMemoryCardStartUpMenus();

#ifndef TEST_STATION
		gameAlreadyInitialised = true;
	PlayIntroMPEGs();
		LOAD_CODE_OVERLAY(init);
#endif 
	}
	else
	{	
#ifndef TEST_STATION
		PlayIntroMPEGs();
	LOAD_CODE_OVERLAY(init);
#endif 
	
	GameInit();

#ifdef FINALBUILD
	DelayThread(2000000);
//	frameCount = 0;
//	while(frameCount < 100) {} // can we justify the whole 100 frames here - reduce if possible please - DW
#endif
	
		CGame::InitialiseEssentialsAfterRW();
		IOPMEM_DEBUG("InitialiseEssentialsAfterRW")	
	}

	CLoadingScreen::Init(true);
	
bool bCheck = CGame::InitialiseCoreDataAfterRW();
	ASSERTMSG(bCheck, "CGame::InitialiseCoreDataAfterRW() failed.");
	IOPMEM_DEBUG("InitialiseCoreDataAfterRW")

#ifndef MASTER
#ifdef FINALBUILD
	version_number = GetVersionNumberFromFile();
#endif	

#ifndef FINAL	
	VarConsole.Add("Reload font proportional values",&CFont::LoadFontValues, VC_RENDER);
	VarConsole.Add("Draw poly density",&gbDrawPolyDensity, true, VC_RENDER);
	VarConsole.Add("Debug Targetting",&CPlayerPed::bDebugTargetting, true, VC_TARGETTING);
	VarConsole.Add("Car Colours Editor", &bCarColsEditor, true, VC_VEHICLES);	
	CPedDebugVisualiserMenu::Initialise();
#endif

	CDebugBugVisualiserMenu::Initialise();

	VarConsole.Add("Display debug info list", &gDisplayDebugInfo, true, VC_RENDER);
	VarConsole.Add("Display number of atomics rendered", &displayNumObjsRendered, true, VC_RENDER);
	VarConsole.Add("Display position", &displayPosition, true, VC_RENDER);
	VarConsole.Add("Display grid details", &displayGridDetails, true, VC_RENDER);
	VarConsole.Add("Display framerate", &displayFrameRate, true, VC_RENDER);
	VarConsole.Add("Display lod multiplier", &displayLODMultiplier, true, VC_RENDER);
	VarConsole.Add("Display Boundary Walls", &CGridRef::displayWalls, true, VC_RENDER);
	VarConsole.Add("Display All Walls or just this zone", &CGridRef::displayAllTheWalls, true, VC_RENDER);
	VarConsole.Add("Display Camera Coordinates", &CGridRef::displayCamCords, true, VC_RENDER);	
	VarConsole.Add("Display Number Matrices", &gDisplayNumberMatrices, true, VC_RENDER);	
	VarConsole.Add("Display Number Peds and Cars", &displayNumCarsAndPeds, true, VC_RENDER);	
	VarConsole.Add("Display memory usage", &displayMemoryUsage, true, VC_MEMORY);		
	VarConsole.Add("Display pools usage", &displayPoolsUsage, true, VC_MEMORY);		
	VarConsole.Add("Display detailed memory usage", &displayDetailedMemoryUsage, true, VC_MEMORY);		
	VarConsole.AddCommand("put player", &CPlayerPed::PutPlayerAtCoords);  // add "put player" command
#endif

	//################################################ Front end bit

#ifndef FINALBUILD
////bonjour whomever is looking at this, this is the only way that we can get start
///to be registered at the start of the game 
	CPad::UpdatePadsTillStable(); // DW - A marginally faster version for development.
	
	IOPMEM_DEBUG("UpdatePadsTillStable")


	//if (true)
	if (CPad::GetPad(0)->GetStart())
	{
		TheModelViewer();
	}
	else
#endif
	{
		TheGame();	// The actual game here
	}

	// WE NEVER GET HERE
/*	
	// I suppose we will never get here.
	CMemInfo::Stop();

	CGame::Shutdown();
	
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();

	#ifdef GTA_PS2PEEK_PROFILER
		if(!CPS2Peek::Shutdown())
		{
			return(FALSE);
		}
	#endif
	
	mwExit();    // Clean up, destroy constructed global objects*/
	return(0);
}

//
// name:		SystemInit
// description:	Do necessary system initialisation
void SystemInit()
{
	CLocalisation::Initialise();  // set up the different game versions
	
	CMemoryMgr::Init();
	CFileMgr::InitCdSystem();
 	
	// initialise IOP
#ifndef TEST_STATION	
	LoadIOPKernel();
#endif

	sceSifInitIopHeap();
	IOPMEM_DEBUG("START")
	CFileMgr::InitCd();

	LoadSystemIOPModules();
	LoadGameIOPModules();
	//LoadNetworkIOPModules();
	
	CreateIdleThread();
	PreparePowerOff();
	
	CPad::Initialise();
	CPad::GetPad(0)->SetMode(CPad::PADMODE_CONFIG1);

	FrontEndMenuManager.m_SystemLanguage = LANGUAGE_ENGLISH;
#ifdef PAL_BUILD
#ifndef AUSTRALIAN_GAME 
	// get system language
	Int32 language = sceScfGetLanguage();

	CLocalisation::SetNormalGame();
	
	switch(language)
	{
#ifdef GERMAN_GAME
	case(SCE_GERMAN_LANGUAGE):
		FrontEndMenuManager.m_PrefsLanguage		= LANGUAGE_GERMAN;
		FrontEndMenuManager.m_SystemLanguage		= LANGUAGE_GERMAN;
		CLocalisation::SetGermanGame();
		FrontEndMenuManager.m_PrefsShowSubtitles = TRUE;
		break;
#else
	case(SCE_GERMAN_LANGUAGE):
		FrontEndMenuManager.m_PrefsLanguage		= LANGUAGE_GERMAN;
		FrontEndMenuManager.m_SystemLanguage		= LANGUAGE_GERMAN;
		FrontEndMenuManager.m_PrefsShowSubtitles = TRUE;
		break;
	case(SCE_FRENCH_LANGUAGE):
		FrontEndMenuManager.m_PrefsLanguage		= LANGUAGE_FRENCH;
		FrontEndMenuManager.m_SystemLanguage		= LANGUAGE_FRENCH;
		CLocalisation::SetFrenchGame();
		FrontEndMenuManager.m_PrefsShowSubtitles = TRUE;
		break;
	case(SCE_SPANISH_LANGUAGE):
		FrontEndMenuManager.m_PrefsLanguage		= LANGUAGE_SPANISH;
		FrontEndMenuManager.m_SystemLanguage		= LANGUAGE_SPANISH;
		FrontEndMenuManager.m_PrefsShowSubtitles = TRUE;
		break;
	case(SCE_ITALIAN_LANGUAGE):
		FrontEndMenuManager.m_PrefsLanguage		= LANGUAGE_ITALIAN;
		FrontEndMenuManager.m_SystemLanguage		= LANGUAGE_ITALIAN;
		FrontEndMenuManager.m_PrefsShowSubtitles = TRUE;
		break;
#endif
	default:
		FrontEndMenuManager.m_PrefsLanguage		= LANGUAGE_ENGLISH;
		FrontEndMenuManager.m_SystemLanguage		= LANGUAGE_ENGLISH;
		FrontEndMenuManager.m_PrefsShowSubtitles = FALSE;
		break;
	}

#ifdef GERMAN_GAME
	CLocalisation::SetGermanGame();
	FrontEndMenuManager.m_PrefsShowSubtitles = TRUE;
#endif // GERMAN_GAME

#endif // AUSSIE BUILD
#endif //PAL_BUILD
}

//
// name:		PlayIntroMPEGs
// description:	Play intro mpegs
void PlayIntroMPEGs()
{
#ifdef FINALBUILD
	float sizeForInterrupt = 0;
	if(gameAlreadyInitialised)
		RpSkySuspend();
	
	LOAD_CODE_OVERLAY(mpeg);
		
	InitMPEGPlayer();

	// what is the point of this? (JG)
	//if (TheMemoryCard.MemoryCards[CARD_ONE].IsPresent())
		sizeForInterrupt = 2750000;
	//else
		//sizeForInterrupt = 5500000;
	

	#ifdef PAL_BUILD
		if (!SkipAllMPEGs)
		{
#ifdef MASTER
			PlayMPEG("movies\\vcpal.pss", false, sizeForInterrupt);
#else
			PlayMPEG("movies\\vcpal.pss", true, 0);
#endif
		}
		
		if (!SkipAllMPEGs)
		{
			SkipAllMPEGs = true;
			PlayMPEG("movies\\open1pal.pss", true, 0);
		}
	#else

		if (!SkipAllMPEGs)
		{
#ifdef MASTER
			PlayMPEG("movies\\vcntsc.pss", false, sizeForInterrupt);
#else
			PlayMPEG("movies\\vcntsc.pss", true, 0);
#endif
		}
	
		if (!SkipAllMPEGs)
		{
			SkipAllMPEGs = true;
			PlayMPEG("movies\\open1.pss", true, 0);
		}	
	#endif

	ShutdownMPEGPlayer();

	REMOVE_CODE_OVERLAY();
	
	if(gameAlreadyInitialised)
		RpSkyResume();
#endif	
}
#endif	

#ifdef GTA_PS2

//
//
// name:		InitialMemoryCardAndPadChecks
// description:	Do initial memory card checks like checking memory available etc
//				overlay is loaded before we start this function
//				also checks for pads (pad 1 only)
//
bool InitialMemoryCardAndPadChecks()
{
//#define CHECK_FOR_MEMORYCARDS_AT_STARTUP

	LOAD_CODE_OVERLAY(mc);

	TheMemoryCard.Init();

		
// SONY say we dont have to have these checks in anymore:
#ifdef CHECK_FOR_MEMORYCARDS_AT_STARTUP

	CPad::UpdatePads();
	if (CPad::bNoControllerFound[0] || CPad::bUnsupportedControllerFound[0])
		return true;

	uint32 status;

	// then check memory card:
	status = TheMemoryCard.CheckCardStateAtGameStartUp(CARD_ONE);
	// test stuff:
	switch(status)
	{
		case(EVERYTHING_OK_ON_YOU_GO):
#ifndef CDROM
		case(NO_MEMORY_CARD_AT_START_UP):
#endif
			return false;
			break;
		case(NEED_SPACE_FOR_DIR):
		case(NEED_SPACE_FOR_ONE_SAVE_GAME):
#ifdef CDROM
		case(NO_MEMORY_CARD_AT_START_UP):
#endif
			return true;
			break;
		default:
			ASSERTMSG(FALSE, "Unrecognized MemoryCard status at game startup!");
			break;
	}
	
#endif  // CHECK_FOR_MEMORYCARDS_AT_STARTUP

	return false;
}


#define IOPDIR	"SYSTEM\\"
#define USE_IOPIMAGE
//#define USE_HOST_IOP_MODULES
//
// LoadIOPKernel: can't get this to work yet
void LoadIOPKernel()
{
	char filename[256];

#ifdef USE_HOST_IOP_MODULES
	sprintf(filename, "host0:%s%s", IOPDIR, IOP_IMAGE_FILE);
#elif defined(CDROM)
	sprintf(filename, "cdrom0:\\%s%s;1", IOPDIR, IOP_IMAGE_FILE);
#else	
	sprintf(filename, "host0:%s%s", IOPDIR, IOP_IMAGE_FILE);
#endif	

	// Initialise SIF	
    sceSifInitRpc(0);
    
#if defined(FINALBUILD) || defined(USE_IOPIMAGE)

//	LoadIOPModule("MEM2MB.IRX", true);
	/* Reboot IOP, replace default modules  */
	while ( !sceSifRebootIop(filename) ) {} /* (Important) Unlimited retries */
	while( !sceSifSyncIop() ) {}
	/* Reinitialize */
	sceSifInitRpc(0);

	sceSifLoadFileReset();
 	sceFsReset();

	CFileMgr::InitCdSystem();

#endif	
}

char* GetIOPModuleFilename(char* pFilename, const char* pModule)
{
#if defined(USE_HOST_IOP_MODULES)

#if defined(TEST_STATION)
	strcpy(pFilename, mwAtGetRFPServer());
	strcat(pFilename, ",");
#else	
	strcpy(pFilename, "host0:");
#endif
	
#elif defined(CDROM)
	strcpy(pFilename, "cdrom0:\\");
#else	
	strcpy(pFilename, CFileMgr::GetRootDirectory());
#endif	
	strcat(pFilename, IOPDIR);
	strcat(pFilename, pModule);

	return pFilename;
}

bool LoadIOPModule(char *pModule, bool bAssert, int argc, const char *argv)
{
	static char noargs[] = "";
	char filename[256];

	GetIOPModuleFilename(filename, pModule);
	printf("Loading %s\n", filename);
	while(sceSifLoadModule(filename, argc, argv) < 0)
//	while(sceSifLoadModule(filename, 0, noargs) < 0)
	{
		if(bAssert)
		{
		    printf ("Can't Load Module %s\n", filename);
		    ASSERT(0);
		}
	}
	return true;
}
bool LoadIOPModuleHost(char *pModule, bool bAssert)
{
	static char noargs[] = "";
	char filename[256];

	sprintf(filename, "host0:d:\\miami_ps2\\system\\%s", pModule);
	printf("Loading %s\n", filename);
	while(sceSifLoadModule(filename, 0, noargs) < 0)
	{
		if(bAssert)
		{
		    printf ("Can't Load Module %s\n", filename);
		    ASSERT(0);
		}
	}
	return true;
}
// 
// LoadIOPModules: Load all the IOP modules that are required
//
void LoadSystemIOPModules()
{
	LoadIOPModule("SIO2MAN.IRX", true);
	IOPMEM_DEBUG("SIO2MAN.IRX")
	LoadIOPModule("PADMAN.IRX", true);
	IOPMEM_DEBUG("PADMAN.IRX")
#ifdef USE_SULPHA
	LoadIOPModule("SulphaSound.irx", true);
	IOPMEM_DEBUG("SulphaSound.irx")
	LoadIOPModule("SulphaComm.irx", true);
	IOPMEM_DEBUG("SulphaComm.irx")
#else
	LoadIOPModule("LIBSD.IRX", true);
	IOPMEM_DEBUG("LIBSD.IRX")
#endif
	LoadIOPModule("SDRDRV.IRX", true);
	IOPMEM_DEBUG("SDRDRV.IRX")
	LoadIOPModule("MCMAN.IRX", true);
	IOPMEM_DEBUG("MCMAN.IRX")
	LoadIOPModule("MCSERV.IRX", true);
	IOPMEM_DEBUG("MCSERV.IRX")
#ifndef MASTER
	LoadIOPModule("USBD.IRX", true);  // usb keyboard
	IOPMEM_DEBUG("USBD.IRX")
	LoadIOPModule("USBKB.IRX", true);  // usb keyboard
	IOPMEM_DEBUG("USBKB.IRX")
	LoadIOPModule("DEV9.IRX", true);  // dont think this is usb keyboard?
	IOPMEM_DEBUG("DEV9.IRX")
#endif
}

void LoadGameIOPModules()
{
#if defined(CDROM) || defined(STREAM_FROM_CD)
	LoadIOPModule("CDSTREAM.IRX", true);
	IOPMEM_DEBUG("CDSTREAM.IRX")

#ifdef USE_AUDIOENGINE
	LoadIOPModule("IOPAUDIO.IRX", true);
	IOPMEM_DEBUG("IOPAUDIO.IRX")
#else //USE_AUDIOENGINE
	LoadIOPModule("SAMPMAN2.IRX", true);	// new DTS support (only really works with CD/DVD version)
	IOPMEM_DEBUG("SAMPMAN2.IRX")
#endif //USE_AUDIOENGINE

#else
	// Load CD Streaming module that actually streams off the host harddisk
	LoadIOPModule("CDSTFAKE.IRX", true);
	IOPMEM_DEBUG("CDSTFAKE.IRX")

#ifdef USE_AUDIOENGINE
	LoadIOPModule("IOPAUDIO.IRX", true);
	IOPMEM_DEBUG("IOPAUDIO.IRX")
#else //USE_AUDIOENGINE
	LoadIOPModule("SAMPFAK2.IRX", true);		// Contains DTS support - distort under host but works on DVD/CD
	IOPMEM_DEBUG("SAMPFAK2.IRX")
#endif //USE_AUDIOENGINE

#endif

	//Colin - load network IRX modules here while we can afford the IOP RAM. Loading on the first bug
	//report seems to fail (on Test Stations), even when sufficient IOP RAM is available.
#ifndef MASTER	
#ifndef LOAD_NETWORK_IRX_ON_BUG
	if(SN_NetworkPS2::LoadIOPModules())
		DEBUGLOG("Loaded network IRX files\n");			
	else
		DEBUGLOG("Error Network not started properly.\n Please ensure the Network card is attached and connected to the network");
#endif
#endif
}
#endif
//////// This is pretty much RenderWare specific stuff. (no game stuff here please)
// These functions are called by RenderWare and will themselves
// call the game functions.

//#define DEFAULT_CAMERA_WIDTH    640
//#define DEFAULT_CAMERA_HEIGHT   480
#define DEFAULT_VIEWWINDOW      (RwReal)(0.7f)
#define DEFAULT_ASPECTRATIO     (RwReal)(4.0f/3.0f)


void IdleThread(void* pData)
{
	while(1) {}
}

void CreateIdleThread()
{
#ifdef GTA_PS2
	extern void* _gp;
	struct ThreadParam tParam;
	int tid;
	
	tParam.entry = &IdleThread;
	tParam.stack = &idleThreadStack[0];
	tParam.stackSize = 2048;
	tParam.initPriority = 127;
	tParam.gpReg = &_gp;
	
	tid = CreateThread(&tParam);
	StartThread(tid, NULL);
#endif	
}

#ifndef FINAL
void JPegDecompressToVramFromMemoryCard(RwRaster* pRaster1, RwRaster* pRaster2, const char* pFilename, uint32 photoNumber);
#endif

RwBool RwGrabScreen(RwCamera *camera, RwChar *filename)
{

	char temp[255];
	RwImage* pImage = RsGrabScreen(camera);
	RwBool rt = TRUE;
	
	if(pImage)
	{
#ifdef GTA_PS2
		CTimer::Stop();
#endif
		strcpy(&temp[0], filename);
		
		if (RtBMPImageWrite(pImage, &temp[0]) == NULL)
			rt = FALSE;
		RwImageDestroy(pImage);
	 	return rt;
	}
	return FALSE;

//	postFx.GrabScreen(filename);
//	return TRUE;	
}

//
// name:		TakeJPegPhoto
// description:	If TakePhoto flag is set then store screen on memory card
bool TakeJPegPhoto(RwCamera* pCamera)
{
	// Take photo
	if(CWeapon::ms_bTakePhoto)
	{	
		return true;
	}
	return false;
}


// The idea is that the function:
// DoRWStuffStartOfFrame is called before any display stuff is being done.
// DoRWStuffEndOfFrame is called once all the rendering has been completed
//						this takes care of frame swapping etc.

//////////////////////////////////////////////////////////////////////////
// FUNCTION     DoRWStuffStartOfFrame
// PURPOSE      Deals with the RenderWare at the start of a frame (before
//              any rendering has been done)
//////////////////////////////////////////////////////////////////////////

bool DoRWStuffStartOfFrame(Int16 TopRed, Int16 TopGreen, Int16 TopBlue,
							Int16 BottomRed, Int16 BottomGreen, Int16 BottomBlue, Int16 alpha)
{
  	//setup the aspect ratio
  	CDraw::CalculateAspectRatio();
    CRGBA ColourTop;
    CRGBA ColourBottom;

	CFont::SetAlphaFade( 255 );  // set the initial alpha fade
	
	// this function should not be called if the loading screen render thread is active (separate loading screen thread on ps2!)
#if defined (GTA_PS2)
	ASSERT(!CLoadingScreen::IsActive() || CLoadingScreen::IsPaused());
#endif //GTA_PS2
	
	ColourTop.red = TopRed;
	ColourTop.green = TopGreen;
	ColourTop.blue = TopBlue;
	ColourTop.alpha = alpha;
	ColourBottom.red = BottomRed;
	ColourBottom.green = BottomGreen;
	ColourBottom.blue = BottomBlue;
	ColourBottom.alpha = alpha;

    // Set the FOV and stuff
	// I think a viewwindow of 1.0 corresponds to a FOV of 90. as such:
	float ViewWindow = CMaths::Tan( (CDraw::GetFOV() * (PI / 360.0f) ) );


    CameraSize(Scene.camera, (RwRect *)0, (RwReal)ViewWindow, (RwReal)CDraw::GetAspectRatio()); // last 2 args: VIEWWINDOW, ASPECTRATIO.

	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);

    // Clear the cameras Z buffer
    RwCameraClear(Scene.camera, &ColourTop, rwCAMERACLEARZ/*|rwCAMERACLEARIMAGE*/);

    if (!RsCameraBeginUpdate(Scene.camera))
    {
    	return false;
    }

	CSprite2d::InitPerFrame();

	if(alpha != 0)
	{
		CSprite2d::DrawRect( CRect	(0 ,
									0,
									SCREEN_WIDTH,
									SCREEN_HEIGHT),
									ColourBottom, ColourBottom, ColourTop, ColourTop);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// FUNCTION     DoRWStuffStartOfFrame
// PURPOSE      Deals with the RenderWare at the start of a frame (before
//              any rendering has been done)
//////////////////////////////////////////////////////////////////////////

bool DoRWStuffStartOfFrame_Horizon(Int16 TopRed, Int16 TopGreen, Int16 TopBlue,
									Int16 BottomRed, Int16 BottomGreen, Int16 BottomBlue, Int16 alpha)
{
    // Set the FOV and stuff
	// I think a viewwindow of 1.0 corresponds to a FOV of 90. as such:
	float ViewWindow = CMaths::Tan( (CDraw::GetFOV() * (PI / 360.0f) ) );
		//setup the aspect ratio
	CDraw::CalculateAspectRatio();

    CameraSize(Scene.camera, (RwRect *)0, (RwReal)ViewWindow, CDraw::GetAspectRatio()); // last 2 args: VIEWWINDOW, ASPECTRATIO.

	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);

    // Clear the cameras Z buffer
#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ|rwCAMERACLEARSTENCIL/*|rwCAMERACLEARIMAGE*/);
#else
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ/*|rwCAMERACLEARIMAGE*/);
#endif

    if(RsCameraBeginUpdate(Scene.camera)==0)
    {
    	return false;
    }
    
	DefinedState();

	TheCamera.UpdateViewMatrix();

	CClouds::RenderSkyPolys();
	
	return true;
}


void DoFade ( void )
{
	// Don't do any fading if the game is paused. This might obscure important
	// messages (like no controller connected)
	if (GAMEISPAUSED) return;

	//fudge to overwrite any fade values when the user has selected load game
	if (JustLoadedDontFadeInYet==true)
	{
		TimeStartedCountingForFade=CTimer::GetTimeInMilliseconds();
		JustLoadedDontFadeInYet=false;
	}
	
	if (StillToFadeOut==true) 
	{	
		if (CTimer::GetTimeInMilliseconds()-TimeStartedCountingForFade>TimeToStayFadedBeforeFadeOut)
		{	
				StillToFadeOut=false; 
				TheCamera.Fade(3.0f, TheCamera.FADE_IN);
				TheCamera.ProcessFade();
				TheCamera.ProcessMusicFade();
		}
		else
		{
			TheCamera.SetFadeColour(0.0f,0.0f,0.0f);
			TheCamera.Fade(0.0f, TheCamera.FADE_OUT);
			TheCamera.ProcessFade();
		}
	}
	
	
	// Stuff needed to clear the screen rendering a poly	
	// Draw a 2D poly to do a fade
	if (CDraw::FadeValue)
	{
     	CRGBA	FadeColour;
		Int32	FadeVal32 = CDraw::FadeValue;
		CRect	ScreenRect;
		
        // If there is a fade to a picture going on we don't want to take the CDraw::FadeValue
        // into account.
        if (TheCamera.m_FadeTargetIsSplashScreen)
        {
            FadeVal32 = 0;
        }

		if ((FadeVal32) == 0)
		{
			FadeColour.red = FadeColour.green = FadeColour.blue = FadeColour.alpha = 0;
		}
		else
		{
			FadeColour.red = (CDraw::FadeRed * FadeVal32) / (FadeVal32);
			FadeColour.green = (CDraw::FadeGreen * FadeVal32) / (FadeVal32);
			FadeColour.blue = (CDraw::FadeBlue * FadeVal32) / (FadeVal32);
			FadeColour.alpha = MAX(0, FadeVal32);
		}

#ifdef GTA_PC
		ScreenRect.left = -5;
		ScreenRect.right = SCREEN_WIDTH+5;
		ScreenRect.top = -5;
		ScreenRect.bottom = SCREEN_HEIGHT+5;
#else
		ScreenRect.left = 0;
		ScreenRect.right = SCREEN_WIDTH;
		ScreenRect.top = 0;
		ScreenRect.bottom = SCREEN_HEIGHT;
#endif

		CSprite2d::DrawRect(ScreenRect, FadeColour); //had to do this as was drawing balck over the bleeder
			//was giving the impression of fade happening twice as fast MH.
		
		//
		// now fades to black (not the postcard):
		//
		if ((CDraw::FadeValue)&&(TheCamera.m_FadeTargetIsSplashScreen))
		{
//			CLoadingScreen::RenderSplash(CDraw::FadeValue);
		}
	}
}	


//
// name:		PrintMetrics
// description:	Print RW metrics if available
void PrintMetrics()
{
#ifdef RWMETRICS

	Int32 Screeny = 7;
   RwMetrics          *metrics = RwEngineGetMetrics();

   sprintf(gString, "numTriangles = %08d", metrics->numTriangles);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

   sprintf(gString, "numProcTriangles = %08d", metrics->numProcTriangles);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

   sprintf(gString, "numVertices = %08d", metrics->numVertices);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

   sprintf(gString, "numResourceAllocs = %08d", metrics->numResourceAllocs);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

   sprintf(gString, "numTextureUploads = %08d", metrics->numTextureUploads);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

   sprintf(gString, "sizeTextureUploads = %08d", metrics->sizeTextureUploads);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

#if defined(GTA_PS2)
    RwSkyMetrics *skyMetrics = (RwSkyMetrics *)metrics->devSpecificMetrics;

    RwChar message[200];
    RwInt32 val, integer, fraction;

    val = (10000 * skyMetrics->vu1Running) / skyMetrics->profTotal;
    integer = val / 100;
    fraction = val % 100;
    sprintf(gString, "vu1 utilisation = %02d.%02d%%", integer,  fraction);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

    val = (10000 * skyMetrics->dma1Running) / skyMetrics->profTotal;
    integer = val / 100;
    fraction = val % 100;
    sprintf(gString, "dma1 utilisation = %02d.%02d%%", integer,
            fraction);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

    val = (10000 * skyMetrics->dma2Running) / skyMetrics->profTotal;
    integer = val / 100;
    fraction = val % 100;
    sprintf(gString, "dma2 utilisation = %02d.%02d%%", integer,
            fraction);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

    sprintf(gString, "VSyncs between flips = %d",
            skyMetrics->vSyncsSinceLastFlip);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

    val = (RwInt32) ((float) skyMetrics->flipPktToFlipTime /   14745.60f);
    integer = val / 10;
    fraction = val % 10;
    sprintf(gString, "Flip pkt to flip = %02d.%01d", integer,
            fraction);
	CDebug::PrintToScreenCoors(gString, 6,Screeny++);

#elif defined(GTA_PC)

    RwD3D8Metrics *pD3D8Metrics = (RwD3D8Metrics *)metrics->devSpecificMetrics;
    sprintf(gString, "Render State Changes %d", pD3D8Metrics->numRenderStateChanges);
    CDebug::PrintToScreenCoors(gString, 6, Screeny++);

    sprintf(gString, "numTextureStageStateChanges %d", pD3D8Metrics->numTextureStageStateChanges);
    CDebug::PrintToScreenCoors(gString, 6, Screeny++);

    sprintf(gString, "numMaterialChanges %d", pD3D8Metrics->numMaterialChanges);
    CDebug::PrintToScreenCoors(gString, 6, Screeny++);

    sprintf(gString, "numLightsChanged %d", pD3D8Metrics->numLightsChanged);
    CDebug::PrintToScreenCoors(gString, 6, Screeny++);

    sprintf(gString, "numVBSwitches %d", pD3D8Metrics->numVBSwitches);
    CDebug::PrintToScreenCoors(gString, 6, Screeny++);

#endif

#endif

}

//////////////////////////////////////////////////////////////////////////
// FUNCTION     DoRWStuffEndOfFrame
// PURPOSE      Deals with the RenderWare at the end of a frame (after
//              all the rendering has been done)
//////////////////////////////////////////////////////////////////////////

void DoRWStuffEndOfFrame()
{

	// Print any debug text that may have been stored during the frame
	CDebug::PrintBufferedStringsToScreen();
	CDebug::DebugDisplayTextBuffer();

	FlushObrsPrintfs();

    RwCameraEndUpdate(Scene.camera);

    
#ifndef FINALBUILD
	if(RwResourcesGetArenaUsage() > RwResourcesGetArenaSize())
	{
		CDebug::PrintToScreenCoors("The resource arena is too small", 20, 6);
		DEBUGLOG2("The resource arena is too small %d:%d\n", RwResourcesGetArenaUsage(), RwResourcesGetArenaSize());
	}	
#endif
		
#ifdef DEBUG_MEMORY	
	int32 size = RwResourcesGetArenaUsage();
	sprintf(gString, "Arena %d\n", size);
	CDebug::PrintToScreenCoors(gString, 32, 0);
#endif

	PrintMetrics();
	
	RsCameraShowRaster(Scene.camera);
	
	if(!TakeJPegPhoto(Scene.camera))
	{
		//
		// Take a screen shot if both start and select are pressed on the second pad
		//
#ifndef FINALBUILD
//#if !defined (GTA_PC)
		if(gbGrabScreenNow /*|| CPad::GetPad(1)->ShockButtonLJustDown() */|| PS2Keyboard.GetKeyJustDown(PS2_KEY_BSPACE, KEYBOARD_MODE_STANDARD, "take screen shot"))
//#else
//		if(CPad::GetPad(0)->TakeScreenShotJustDown())
//#endif // GTA_PS2
		{
			gbGrabScreenNow = FALSE;
			static Int32 ScreenShotCounter = 0;

			CFileMgr::SetDir("");
#ifdef GTA_PS2	
			CTimer::Stop();
#endif // GTA_PS2	
			if(CPad::GetPad(0)->GetRightStickY()>0)
			{
				sprintf(gString, "screen%d.ras", ScreenShotCounter);
				RtTileRender(Scene.camera, TILE_WIDTH*2, TILE_HEIGHT*2, TILE_WIDTH, TILE_HEIGHT, &NewTileRendererCB, NULL, gString);
			}
			else
			{
				sprintf(gString, "screen%d.bmp", ScreenShotCounter);
				RwGrabScreen(Scene.camera, gString);
			}
			
			ScreenShotCounter++;  // increment the counter for screenshots
		}
#endif // FINALBUILD
	}
}



//
//
// name:		PluginAttach
// description:	Attach all the RW plugins used
//
RwBool PluginAttach()
{
#ifdef GTA_PS2
    // Attach the world plugin
	if (!RpPDSPluginAttach(RpWorldNumPipes+RpSkinNumPipes+RpMatfxNumPipes +NUM_PDS_CUSTOM_CVB_PIPES+NUM_PDS_CUSTOM_CVC_PIPES+NUM_PDS_CUSTOM_CVSP_PIPES))
    {
        return (FALSE);
    }
#endif

    // Attach the world plugin
	if (!RpWorldPluginAttach())
    {
        return (FALSE);
    }
	
#ifdef GTA_PS2
	RpWorldPipesAttach();
#endif

	// Attach the skin plugin
	if(!RpSkinPluginAttach())
	{
        return (FALSE);
	}
#ifdef GTA_PS2
    RpSkinPipesAttach();
#endif

	if(!RtAnimInitialize())
	{
	  	return(FALSE);
	}
	
	if(!RpHAnimPluginAttach())
	{
	  	return(FALSE);
	}


 	// Attach the nodename plugin
    if (!NodeNamePluginAttach())
    {
        return (FALSE);
    }

    // Attach the atomic/clump visibility plugins
    if (!CVisibilityPlugins::PluginAttach())
    {
        return (FALSE);
    }

    // Attach the animation blending plugin
    if (!RpAnimBlendPluginAttach())
    {
        return (FALSE);
    }

    // Attach the animation blending plugin
    if (!CTxdStore::PluginAttach())
    {
        return (FALSE);
    }

	// Attach the material effects plugin
	if(!RpMatFXPluginAttach())
	{
        return (FALSE);
	}

	// Attach the material uv animation plugin
   	if (!RpUVAnimPluginAttach())
	{
		return FALSE;
	}

	//!PC - there is a dummy plugin for the stream use
#if defined(GTA_PS2)
	if(!CVuCustomBuildingRenderer::PluginAttach())
	{
		return(FALSE);
	}
#endif
#if defined(GTA_PC) || defined(GTA_XBOX)
	if(!CCustomBuildingRenderer::PluginAttach())
	{
		return(FALSE);
	}
#endif

	
#if defined (GTA_PS2)
	// Attach the ADC (tristrip extending) plugin
	if(!RpADCPluginAttach())
	{
        return (FALSE);
	}
#endif //GTA_PS2

#ifdef USE_VU_CARFX_RENDERER
	if(!CVuCarFXRenderer::RegisterPlugins())
	{
		return(FALSE);
	}
#endif
#ifdef USE_PC_CARFX_RENDERER
	if(!CCarFXRenderer::RegisterPlugins())
	{
		return(FALSE);
	}
#endif



// Andrzej switched off default matfx pipes to save 300KB
//#ifdef GTA_PS2
//    RpMatfxPipesAttach();
//#endif	

#if defined(GTA_PC) && !defined(OSW)
	RpAnisotPluginAttach();
#endif
	
#ifdef DEBUG
	if(!CMemInfo::PluginAttach())
	{
        return (FALSE);
	}
#endif//DEBUG

	if (!BreakablePluginAttach())
	{
		return (FALSE);
	}

	if (!CCollisionPlugin::PluginAttach())
	{
		return (FALSE);
	}

	if (!C2dEffect::PluginAttach())
	{
		return (FALSE);
	}

#ifndef GTA_PS2 // This is a PC/Xbox replacement for the PDS pipeline info 
	if(!PipelinePluginAttach())
	{
		return(FALSE);
	}
#endif
    return(TRUE);
}



//
// Set the resolution of the screen
//
/*RwBool SelectVideoDevice(void)
{
	int32 i;
	RwVideoMode	videoMode;
	int32 num = RwEngineGetNumVideoModes();
	
	for(i=0; i<num; i++)
	{
        RwEngineGetVideoModeInfo(&videoMode, i);
        if(videoMode.width == SCREEN_WIDTH && 
        	videoMode.height == SCREEN_HEIGHT &&
        	videoMode.depth == 32 &&
			(videoMode.flags == (rwVIDEOMODEINTERLACE|rwVIDEOMODEEXCLUSIVE| rwVIDEOMODEFSfff1)))
        {
        	RwEngineSetVideoMode(i);
        	break;
        }
	}
	
	ASSERTMSG(i != num, "Can't find requested video mode");
    return (TRUE);
}*/

/*RwBool RsRwInitialise()
{
    RwEngineOpenParams  openParams;

	CMemInfo::EnterBlock(RENDER_MEM_ID);
#ifdef GTA_PS2
	// Number of texture cache blocks
	skyGNumTexMemBlocks = 240;

	// Size of DMA tag pool
	_swePreAlloc(0x180000, 20, TRUE);

	// Limit to storing 2 frames of DMA data
	sweMaxFlips = 0;
	
	// Size of GlobalRxHeap
	_rwRxHeapInitialSize = 16;

    // Start RenderWare
    if (!RwEngineInit(&memFuncs,0, 4*1024*1024))	// 60K arena
    {
        return (FALSE);
    }

	// Install File system
    SkyInstallFileSystem(); 	
#endif // !GTA_PS2

	if (!PluginAttach())	// function in main.cpp
	{		// somethings gone wrong with the plugins
		printf("Couldn't load the plugins for some reason\n");
		while (1){};
	}


    openParams.displayID = NULL;
    
    if (!RwEngineOpen(&openParams))
    {
        return (FALSE);
    }

	SelectVideoDevice();

	//RwResourcesSetArenaSize(4 * 1024 * 1024);
	// Print out possible errors that may have occurred allready.
    if (!RwEngineStart())
    {
        RwEngineClose();
        RwEngineTerm();
        return (FALSE);
    }

	
#ifdef RWDEBUG
	RwDebugSetHandler(&HandleRwErrors);
#endif	

    // Register loaders for an image with a particular file extension
    //RwImageRegisterImageFormat(RWSTRING("bmp"), RwImageReadBMP, NULL);
    //RwImageRegisterImageFormat(RWSTRING("png"), RwImageReadPNG, NULL);
	CMemInfo::ExitBlock();

    return (TRUE);
}*/
/*
#define MAX_ATOMICS			3245//2450
#define MAX_CLUMPS			101//90
#define MAX_FRAMES			2821//3000
#define MAX_GEOMETRY		1404//1200
#define MAX_TEXDICTIONARIES	106//90
#define MAX_TEXTURES		1900//1880
#define MAX_RASTERS			1910//1890
#define MAX_MATERIALS		3300//3100

static void PreAllocateRwObjects()
{
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
}
*/
#if defined(GTA_PC) && !defined(OSW) //from win.cpp
extern D3DGAMMARAMP gOrigRamp;     // DW - some alt tab gamma handling - this is the orignal ramp when the game was started that will be restored when the appliaction alt-tabs out.
extern bool gbRampIsSet;	// - a flag indicating that the gamma of the card has been stored OK.
#endif
//
// name:		Terminate3D
// description:	Terminate the 3d code. Delete all RW objects
static void Terminate3D(void* param)
{
#ifdef NEW_CAMERA // not sure if the best place to shutdown the camera yet. - DW
	CameraManagersTerm();
#endif


#ifndef MASTER
#if defined GTA_PS2 || defined GTA_PC
	CDebugBugVisualiserMenu::Shutdown();
#endif	
#endif

#ifndef FINAL
	CPedDebugVisualiserMenu::Shutdown();
#endif

	CGame::ShutdownRenderWare();
#ifdef RWDEBUG	
	DEBUGLOG("TXDs in memory\n");
	for(int32 i=0; i<CTxdStore::GetSize(); i++)
	{
		if(CTxdStore::IsValidSlot(i) && CTxdStore::GetTxd(i))
		{
			DEBUGLOG1("txd %s in memory\n", CTxdStore::GetTxdName(i));
		}
	}
#endif
#if defined(GTA_PC) && !defined(OSW)
	// Reset back Gamma setting when quitting the game
	if (gbRampIsSet)
	{
		DWORD        flags = D3DSGR_CALIBRATE;	
		IDirect3DDevice9 *pDevice = (IDirect3DDevice9 *)RwD3D9GetCurrentD3DDevice(); // PC
		if (pDevice)
			pDevice->SetGammaRamp(0, flags, &gOrigRamp);				
		gbRampIsSet = FALSE;
	}
#endif

	RsRwTerminate();
}

//
//
//
//
RwCamera* NewTileRendererCB(RwCamera *camera, RwInt32 x, RwInt32 y, void *data)
{
	// backup the Scene.camera pointer so that can be restored
	RwCamera *SaveCamera = Scene.camera;
	Scene.camera = camera;
	CDraw::CalculateAspectRatio();

	RpWorld *CamTileWorld = RpWorldAddCamera(Scene.world, Scene.camera);

	UInt8 alpha = 255;
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	
    // Clear the cameras Z buffer
#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
	 RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ|rwCAMERACLEARSTENCIL/*|rwCAMERACLEARIMAGE*/);
#else
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ/*|rwCAMERACLEARIMAGE*/);
#endif

    if (!RsCameraBeginUpdate(Scene.camera))
    {
    	ASSERT(0);
    }

	gHorZ = CSprite::CalcHorizonCoors();
	
	if (gHorZ < 0.0f)
	{		// All fog colour
		gColourTop.red = CTimeCycle::GetFogRed();
		gColourTop.green = CTimeCycle::GetFogGreen();
		gColourTop.blue = CTimeCycle::GetFogBlue();
		gColourTop.alpha = alpha;
		gColourBottom.red = CTimeCycle::GetFogRed();
		gColourBottom.green = CTimeCycle::GetFogGreen();
		gColourBottom.blue = CTimeCycle::GetFogBlue();
		gColourBottom.alpha = alpha;
		CSprite2d::DrawRect( CRect	(0 , 0,
									SCREEN_WIDTH,
									SCREEN_HEIGHT),
									gColourBottom, gColourBottom, gColourTop, gColourTop);
	
	}
	else if (gHorZ > SCREEN_HEIGHT)
	{		// All sky colour
		gColourTop.red = CTimeCycle::GetSkyTopRed();
		gColourTop.green = CTimeCycle::GetSkyTopGreen();
		gColourTop.blue = CTimeCycle::GetSkyTopBlue();
		gColourTop.alpha = alpha;
		gColourBottom.red = CTimeCycle::GetSkyBottomRed();
		gColourBottom.green = CTimeCycle::GetSkyBottomGreen();
		gColourBottom.blue = CTimeCycle::GetSkyBottomBlue();
		gColourBottom.alpha = alpha;
		CSprite2d::DrawRect( CRect	(0 , 0,
									SCREEN_WIDTH,
									gHorZ),
									gColourBottom, gColourBottom, gColourTop, gColourTop);
	}
	else
	{
		if(y==1)
		{
						// Mix of the two
			gColourTop.red = CTimeCycle::GetSkyTopRed();
			gColourTop.green = CTimeCycle::GetSkyTopGreen();
			gColourTop.blue = CTimeCycle::GetSkyTopBlue();
			gColourTop.alpha = alpha;
			gColourBottom.red = CTimeCycle::GetSkyBottomRed();
			gColourBottom.green = CTimeCycle::GetSkyBottomGreen();
			gColourBottom.blue = CTimeCycle::GetSkyBottomBlue();
			gColourBottom.alpha = alpha;
			CSprite2d::DrawRect( CRect	(0 , -SCREEN_HEIGHT,
										SCREEN_WIDTH,
										MIN (gHorZ + 4.0f, SCREEN_WIDTH)),
										gColourBottom, gColourBottom, gColourTop, gColourTop);
		}
		else
		{
				// Mix of the two
			gColourTop.red = CTimeCycle::GetSkyTopRed();
			gColourTop.green = CTimeCycle::GetSkyTopGreen();
			gColourTop.blue = CTimeCycle::GetSkyTopBlue();
			gColourTop.alpha = alpha;
			gColourBottom.red = CTimeCycle::GetSkyBottomRed();
			gColourBottom.green = CTimeCycle::GetSkyBottomGreen();
			gColourBottom.blue = CTimeCycle::GetSkyBottomBlue();
			gColourBottom.alpha = alpha;
			CSprite2d::DrawRect( CRect	(0 , 0,
										SCREEN_WIDTH,
										MIN (gHorZ + 4.0f, SCREEN_WIDTH)),
										gColourBottom, gColourBottom, gColourTop, gColourTop);
		}
	
		gColourTop.red = CTimeCycle::GetFogRed();
		gColourTop.green = CTimeCycle::GetFogGreen();
		gColourTop.blue = CTimeCycle::GetFogBlue();
		gColourBottom.red = CTimeCycle::GetFogRed();
		gColourBottom.green = CTimeCycle::GetFogGreen();
		gColourBottom.blue = CTimeCycle::GetFogBlue();
/*
		CSprite2d::DrawRect( CRect	(0 , gHorZ,
									SCREEN_WIDTH,
									SCREEN_HEIGHT),
									gColourBottom, gColourBottom, gColourTop, gColourTop);
*/
	}

	
/***************************************************/
// Now hopefully ready to start rendering, bits from TheGame rendering code
//

	// Set the rendering pipeline to standard
		DefinedState();
	// Set some fogging
        RwCameraSetFarClipPlane(Scene.camera, CTimeCycle::GetFarClip());
		RwCameraSetFogDistance(Scene.camera, CTimeCycle::GetFogStart());
	
//		if ( (!CPad::GetPad(1)->GetLeftShoulder1()) || (!CPad::GetPad(1)->GetRightShoulder1()) )
		{
//			CWorld::Render();
			CMovingThings::Render_BeforeClouds();

			CClouds::Render();

#ifdef GTA_PS2
			asm volatile ("sync.l");
#endif

			CRenderer::RenderRoads();		

			CCoronas::RenderReflections();

			DefinedState();
				// Set some fogging
        	RwCameraSetFarClipPlane(Scene.camera, CTimeCycle::GetFarClip());
			RwCameraSetFogDistance(Scene.camera, CTimeCycle::GetFogStart());

			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
			RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPELINEAR);
			RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)RWRGBALONG( CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));

			CRenderer::RenderEverythingBarRoads();
			
			DefinedState();

			CWaterLevel::RenderWater();
	
#ifdef DW_ZFIGHT_TRICK_FIX
			static float gDegreeShadowZBufferFix = 2.0f; // doubled to rule out errors.          //0.000025f;//(1.0f/32768.0f); // zbuffer resolutiuon has increased - theoretically we only need to move the z resolution by 1.0f/8388608.0f but have opted for a slightly safer value.
			static float smallestZUnit = 1.0f / (65536.0f*256.0f);
			float shadowZ = smallestZUnit*gDegreeShadowZBufferFix;
			
			float clipNearOld = RwCameraGetNearClipPlane( Scene.camera );
			float clipFarOld  = RwCameraGetFarClipPlane ( Scene.camera );
			
			// DW - I have to reduce the bias when looking above because of blood pools under peds.
			float z = TheCamera.Cams[TheCamera.ActiveCam].Front.z;
			if (z>0.0f) z = 0.0f;			
			else 		z = -z;			
			shadowZ  = DW_LERP(z,shadowZ,shadowZ*0.25f);
			
			float n = ( clipFarOld - clipNearOld ) * shadowZ;
			RwCameraSetNearClipPlane( Scene.camera, clipNearOld + n );
			RwCameraSetFarClipPlane ( Scene.camera, clipFarOld  + n );
#endif
 
		 	CShadows::UpdateStaticShadows();
			if (!CMirrors::bRenderingReflection) CShadows::RenderStaticShadows();
			if (!CMirrors::bRenderingReflection) CShadows::RenderStoredShadows();

#ifdef DW_ZFIGHT_TRICK_FIX // reinstate clip planes
			RwCameraSetNearClipPlane( Scene.camera, clipNearOld );
			RwCameraSetFarClipPlane ( Scene.camera, clipFarOld  );
#endif
		}

		CBirds::Render();
		CGlass::Render();
		CWaterCannons::Render();
		CSpecialFX::Render();
		CRopes::Render();
		CVehicleRecording::Render();

		if (CHeli::GetNumberOfHeliSearchLights() || CTheScripts::GetNumberOfSearchLights())
		{
			CHeli::Pre_SearchLightCone();
			CHeli::RenderAllHeliSearchLights();
			CTheScripts::RenderAllSearchLights();
			CHeli::Post_SearchLightCone();
		}

		
		CSkidmarks::Render();
//		CStunts::Render();
//		CAntennas::Render();

//		CRubbish::Render();
//		CCoronas::Render();

//		CPacManPickups::Render();
		CWeaponEffects::Render();
		CPointLights::RenderFogEffect();
		CMovingThings::Render();


		// -- FX SYSTEM ---------------------------------

		g_fx.Render(TheCamera.m_pRwCamera, false);
		
		// ----------------------------------------------

	//	CRadar::Update();	// This updates and renders the radar. Should maybe be split up.
	//	CHud::Draw( );//100,1,10,2,1,1,1,1);
	//	CMessages::Display();
	//	CDarkel::DrawMessages();
	//	CGarages::PrintMessages();
	//	CPickups::RenderPickUpText();
	//	CTimer::PrintPauseText();

	RwCameraEndUpdate(Scene.camera);

	RpWorldRemoveCamera(Scene.world, Scene.camera);

	Scene.camera = SaveCamera;	// restore scene camera
	
	return(camera);

}// end of NewTileRendererCB()...


char* GetLevelSplashScreen(int32 level)
{
	static char* splashScreens[] = {
		NULL,
		"splash1",
		"splash2",
		"splash3"};
	return splashScreens[level];
}

//
// name:		Loading screen
// description:	Display loading info
//
void LoadingScreen(const char *pMsg, const char *pMsg2, const char* pSplashName)
{
	if (pSplashName)
	{
//		CLoadingScreen::LoadNextSplash();  	
//		CLoadingScreen::DisplayNextSplash();  	
	}

	if (pMsg) 
		CLoadingScreen::SetLoadingBarMsg(pMsg, pMsg2);
		
	CLoadingScreen::NewChunkLoaded();
}

//
// PrintRwUsageInfo: 
// 
void PrintRwUsageInfo()
{
	int32 line = 3;
#ifdef GTA_PS2	
#ifdef RWDEBUG	
	sprintf(gString, "Max TexCache Blocks %d", skyGMaxTexMemBlocks);
	VarConsole.AddDebugOutput(gString);
//	CDebug::PrintToScreenCoors(gString , 6, line++);
#endif	
#endif
}

#ifndef MASTER
//
// Give information on memory usage
//
void PrintMemoryUsage()
{
	int32 line = 2;
	int32 y = 24;
#ifdef DEBUGMEMHEAP
	CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);
	CFont::SetBackground(FALSE);
	CFont::SetWrapx(640);
	CFont::SetScale( 0.5, 0.9);
	CFont::SetProportional(TRUE);
	CFont::SetOrientation(FO_LEFT);
	CFont::SetColor(CRGBA(200,200,200,200));
	CFont::SetDropShadowPosition(2);		

	if(displayMemoryUsage)
	{
		sprintf(gString, "Total: %d blocks, %d bytes",
				CMemInfo::GetNumAllocatedBlocks(), CMemInfo::GetCurrentMemUsage());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Largest freeblock: %d bytes, Size of holes %d bytes",
				CMemoryMgr::GetLargestFreeBlock(), CMemoryMgr::GetSizeOfHoles());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;	
	}
	if(displayDetailedMemoryUsage)
	{
		sprintf(gString, "Game: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(GAME_MEM_ID),
				CMemInfo::GetCurrentMemUsage(GAME_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "World: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(WORLD_MEM_ID),
				CMemInfo::GetCurrentMemUsage(WORLD_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Render: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(RENDER_MEM_ID),
				CMemInfo::GetCurrentMemUsage(RENDER_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "PreAlloc: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(PREALLOC_MEM_ID),
				CMemInfo::GetCurrentMemUsage(PREALLOC_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Default Models: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(DEFAULTMODELS_MEM_ID),
				CMemInfo::GetCurrentMemUsage(DEFAULTMODELS_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Textures: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(TEXTURES_MEM_ID),
				CMemInfo::GetCurrentMemUsage(TEXTURES_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Streaming: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(STREAM_MEM_ID),
				CMemInfo::GetCurrentMemUsage(STREAM_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Streamed Models: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(STREAM_MODELS_MEM_ID),
				CMemInfo::GetCurrentMemUsage(STREAM_MODELS_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Streamed LODs: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(STREAM_BIGBUILDINGS_MEM_ID),
				CMemInfo::GetCurrentMemUsage(STREAM_BIGBUILDINGS_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Streamed Textures: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(STREAM_TEXTURES_MEM_ID),
				CMemInfo::GetCurrentMemUsage(STREAM_TEXTURES_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Streamed Collision: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(STREAM_COLL_MEM_ID),
				CMemInfo::GetCurrentMemUsage(STREAM_COLL_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Streamed Animation: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(STREAM_ANIM_MEM_ID),
				CMemInfo::GetCurrentMemUsage(STREAM_ANIM_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Ped Attr: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(PEDATTR_MEM_ID),
				CMemInfo::GetCurrentMemUsage(PEDATTR_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Animation: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(ANIM_MEM_ID),
				CMemInfo::GetCurrentMemUsage(ANIM_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Pools: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(POOLS_MEM_ID),
				CMemInfo::GetCurrentMemUsage(POOLS_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Collision: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(COLL_MEM_ID),
				CMemInfo::GetCurrentMemUsage(COLL_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Game Process: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(PROCESS_MEM_ID),
				CMemInfo::GetCurrentMemUsage(PROCESS_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Script: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(SCRIPT_MEM_ID),
				CMemInfo::GetCurrentMemUsage(SCRIPT_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Cars: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(CARS_MEM_ID),
				CMemInfo::GetCurrentMemUsage(CARS_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "FX: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(FX_MEM_ID),
				CMemInfo::GetCurrentMemUsage(FX_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Tasks Events: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(TASKEVENT_MEM_ID),
				CMemInfo::GetCurrentMemUsage(TASKEVENT_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Paths: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(PATHS_MEM_ID),
				CMemInfo::GetCurrentMemUsage(PATHS_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		sprintf(gString, "Pipelines: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(PIPELINES_MEM_ID),
				CMemInfo::GetCurrentMemUsage(PIPELINES_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;
		sprintf(gString, "Audio: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(AUDIO_MEM_ID),
				CMemInfo::GetCurrentMemUsage(AUDIO_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(24, y, gGxtString);
		y += 15;		
		
#ifndef FINAL
		y -= 120;
		sprintf(gString, "PlantsMgr: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(PLANTMGR_MEM_ID),
				CMemInfo::GetCurrentMemUsage(PLANTMGR_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(340, y, gGxtString);
		y += 15;
		sprintf(gString, "Entity FX: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(ENTITYFX_MEM_ID),
				CMemInfo::GetCurrentMemUsage(ENTITYFX_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(340, y, gGxtString);
		y += 15;
		sprintf(gString, "Clothes: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(CLOTHES_MEM_ID),
				CMemInfo::GetCurrentMemUsage(CLOTHES_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(340, y, gGxtString);
		y += 15;
		sprintf(gString, "RoadSigns: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(ROADSIGN_MEM_ID),
				CMemInfo::GetCurrentMemUsage(ROADSIGN_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(340, y, gGxtString);
		y += 15;
		sprintf(gString, "NumPlates: %d blocks, %d bytes", 
				CMemInfo::GetNumAllocatedBlocks(NUMPLATE_MEM_ID),
				CMemInfo::GetCurrentMemUsage(NUMPLATE_MEM_ID));
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(340, y, gGxtString);
		y += 15;
#endif
	}	
#endif

	if(displayPoolsUsage)
	{
		y = 54;
		AsciiToGxtChar("Pools usage:", gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "PtrNode: %d/%d", CPools::GetPtrNodeSingleLinkPool().GetNoOfUsedSpaces(), CPools::GetPtrNodeSingleLinkPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "PtrNode Double: %d/%d", CPools::GetPtrNodeDoubleLinkPool().GetNoOfUsedSpaces(), CPools::GetPtrNodeDoubleLinkPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "EntryInfoNode: %d/%d", CPools::GetEntryInfoNodePool().GetNoOfUsedSpaces(), CPools::GetEntryInfoNodePool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "Ped: %d/%d", CPools::GetPedPool().GetNoOfUsedSpaces(), CPools::GetPedPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "Vehicle: %d/%d", CPools::GetVehiclePool().GetNoOfUsedSpaces(), CPools::GetVehiclePool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "Building: %d/%d", CPools::GetBuildingPool().GetNoOfUsedSpaces(), CPools::GetBuildingPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "Object: %d/%d", CPools::GetObjectPool().GetNoOfUsedSpaces(), CPools::GetObjectPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "Dummy: %d/%d", CPools::GetDummyPool().GetNoOfUsedSpaces(), CPools::GetDummyPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "ColModel: %d/%d", CPools::GetColModelPool().GetNoOfUsedSpaces(), CPools::GetColModelPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;
	#ifdef USE_AUDIOENGINE
	#else //USE_AUDIOENGINE
		sprintf(gString, "AudioScriptObjects: %d/%d", CPools::GetAudioScriptObjectPool().GetNoOfUsedSpaces(), CPools::GetAudioScriptObjectPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
	#endif //USE_AUDIOENGINE
		sprintf(gString, "Tasks: %d/%d", CPools::GetTaskPool().GetNoOfUsedSpaces(), CPools::GetTaskPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;		
		sprintf(gString, "Events: %d/%d", CPools::GetEventPool().GetNoOfUsedSpaces(), CPools::GetEventPool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;
		sprintf(gString, "Point Route: %d/%d", CPools::GetPointRoutePool().GetNoOfUsedSpaces(), CPools::GetPointRoutePool().GetSize());
		AsciiToGxtChar(gString, gGxtString);
		CFont::PrintString(400, y, gGxtString);
		y += 15;
	}	
}
#endif // MASTER

#ifndef MASTER
#pragma dont_inline on
//
// name:		DisplayGameDebugText
// description:	Does what it says
//
void DisplayGameDebugText()
{
#ifdef GTA_PS2
	extern int32 numAtomicsRendered;
	extern int32 numBBfailed;
#endif	

#if defined (GTA_PC) || defined (GTA_XBOX)
if (FrontEndMenuManager.m_MenuActive)
	return;
#endif //GTA_PC & GTA_XBOX


//	static bool bDisplayPosn = false;
//	static bool bDisplayRate = false;
	static float FramesPerSecondCounter=0;
	static float FramesPerSecond=30;
	static Int32 FrameSamples=0;

	#define DEBUGTEXTX	14
	#define DEBUGTEXTY	1

	if((displayMemoryUsage || displayDetailedMemoryUsage || displayPoolsUsage) && 
		!VarConsole.Open())
		PrintMemoryUsage();

#ifndef FINAL		
#ifdef GTA_PS2
	if(displayNumObjsRendered)
	{
		sprintf(gString, "Atomics rendered %d", numAtomicsRendered);
		VarConsole.AddDebugOutput(gString);
//		CDebug::PrintToScreenCoors(gString, 50, 28);
		sprintf(gString, "BB checks failed %d", numBBfailed);
		VarConsole.AddDebugOutput(gString);
//		CDebug::PrintToScreenCoors(gString, 50, 29);
	}
#endif	
#endif
	
	FramesPerSecondCounter+=(1000.0f / CTimer::GetTimeElapsedInMillisecondsNonClipped());
	FrameSamples++;
	FramesPerSecond=FramesPerSecondCounter/FrameSamples;

	if (FrameSamples > 30)
	{
		FrameSamples=0;
		FramesPerSecondCounter=0;
	}		

	//
	// display grid reference & fps stuff:
	//
	uint8 grid_pos_x = 0;
	uint8 grid_pos_y = 0;
		
	CGridRef::GetGridRefPositions(&grid_pos_x, &grid_pos_y);  // get the current grid position

	if(displayPosition || displayLODMultiplier|| displayFrameRate || displayGridDetails || CGridRef::displayCamCords || CGridRef::displayGridRef || CCover::bRenderCoverPoints || displayNumCarsAndPeds)
	{
		CVector posn = FindPlayerCoors();
		float PlyrHeading;
		if (CWorld::Players[0].pPed)
		{
			if (CWorld::Players[0].pPed->m_nPedFlags.bInVehicle)
			{
				ASSERTMSG(CWorld::Players[0].pPed->m_pMyVehicle, "Player is not in a vehicle");
				PlyrHeading = RADTODEG(CWorld::Players[0].pPed->m_pMyVehicle->GetHeading());
			}
			else
			{
				PlyrHeading = RADTODEG(CWorld::Players[0].pPed->GetHeading());
			}
		}else
			PlyrHeading = 0.0f;		
		if (PlyrHeading < 0.0f)
		{
			PlyrHeading += 360.0f;
		}
		
		char TempString[200];
		
		if (CHud::m_Wants_To_Draw_Hud)
		{
			if (TheCamera.WorldViewerBeingUsed)
			{
				sprintf(TempString, "DEBW%d", CWeather::NewWeatherType);
				GxtChar *WeatherType;
				
				WeatherType = TheText.Get(TempString);
				
				sprintf(gString,"WEATHER: %s",GxtCharToAscii(WeatherType));
				
				VarConsole.AddDebugOutput(gString);
			}

			if (displayPosition || displayGridDetails && !TheCamera.WorldViewerBeingUsed)
			{
//#ifdef FINAL
				sprintf(TempString,"PLAYER: X:%4.0f Y:%4.0f Z:%4.0f Dir:%3.0f Ver: %d",posn.x, posn.y, posn.z, PlyrHeading, version_number);
//#else
//				sprintf(TempString,"PLAYER: X:%4.0f Y:%4.0f Z:%4.0f Dir:%3.0f",posn.x, posn.y, posn.z, PlyrHeading);
//#endif

			#if !(defined(MASTER) || defined(FINAL) || defined(FINALBUILD))  // display on anything but the MASTER builds
				//VarConsole.AddDebugOutput(TempString, TRUE);
			#endif
			}
			
			if (displayGridDetails || CGridRef::displayGridRef)
			{				
				if( grid_pos_x < 10 && grid_pos_y < 10)
				{ 
					sprintf(TempString,"GRID: %c:%d  ARTIST: %s",(grid_pos_x+65), grid_pos_y+1, CGridRef::GetArtistName(grid_pos_x, grid_pos_y));
				}
				else
				{
					sprintf(TempString,"GRID: !Outside Grid Area!");
				}
				VarConsole.AddDebugOutput(TempString, TRUE);
			}
			if (displayNumCarsAndPeds)
			{
				sprintf(TempString, "Peds:%d Veh:%d", CPools::GetPedPool().GetNoOfUsedSpaces(), CPools::GetVehiclePool().GetNoOfUsedSpaces());
				VarConsole.AddDebugOutput(TempString, TRUE);
			}
			
			if (displayFrameRate)
			{
				sprintf(TempString, "Lighting(Sum): %f(%f) FrameRate: %d", CWorld::Players[0].pPed->m_lightingFromCollision, CWorld::Players[0].pPed->GetLightingTotal(), (Int32)FramesPerSecond);
				VarConsole.AddDebugOutput(TempString);
				#ifndef GTA_PC		
				sprintf(TempString,"MEM %4.1f,FREE %4.1f,Str %4.1f",  (float)(CMemoryMgr::GetMemoryUsed()/1024.0f), (float)(CMemoryMgr::GetLargestFreeBlock()/1024.0f), (float)(CStreaming::GetMemoryUsed()/1024.0f));
				VarConsole.AddDebugOutput(TempString);
				#endif		
			}
			
			if(displayLODMultiplier)
			{
				sprintf(TempString, "LOD Multiplier: %f", CTimeCycle::GetLODMultiplier());
				VarConsole.AddDebugOutput(TempString);
			}
			
#ifndef FINAL
			if (CCover::bRenderCoverPoints)
			{
				sprintf(TempString,"Coverpoints:%d Buildings:%d TimeTaken:%d (%d)", CCover::m_NumPoints, CCover::m_ListOfProcessedBuildings.CountElements(), CCover::LastTimeRecorded, CCover::SlowestTimeRecorded );
				VarConsole.AddDebugOutput(TempString);
			}
#endif

		}
			
		CGridRef::displayGridRef = FALSE;
	}

	//
	// display boundary walls
	//
	if (CGridRef::displayWalls)
	{
		if (CGridRef::displayAllTheWalls) CGridRef::RenderAllBoundaryWalls(); else CGridRef::RenderBoundaryWall(grid_pos_x,grid_pos_y);
	}

#ifndef FINAL
	if(gDisplayNumberMatrices)
	{
		void PrintNumMatrices(char* pChar);
		PrintNumMatrices(gString);
		VarConsole.AddDebugOutput(gString);
//		CDebug::PrintToScreenCoors(gString, 14, 28);
	}	
#endif
	
#if defined(DEBUGTEXT) && !defined(FINALBUILD)

#ifndef FINAL
	CScriptDebugger::DisplayDebugInfoOnGameScreen();
#endif

	if (!CTheScripts::DbgFlag)
		return;
	
	// Print out: How many objects rendered?
	//extern int32 numObjsRendered;
	//sprintf(gString, "Objects rendered %d", numObjsRendered);
	//CDebug::PrintToScreenCoors(gString, DEBUGTEXTX, 0);
	//numObjsRendered=0;
	
	if(!(displayMemoryUsage || displayDetailedMemoryUsage))
	{
		Int32	Line = 0;



		// Print out: How many msecs did this take ?
		sprintf(gString, "Time:%d fps:%f TS:%f", CTimer::GetTimeElapsedInMillisecondsNonClipped(), 
			FramesPerSecond, CTimer::GetTimeStepNonClipped() );
		VarConsole.AddDebugOutput(gString);
//		CDebug::PrintToScreenCoors(gString, DEBUGTEXTX, DEBUGTEXTY+(Line++));

#ifdef DEBUG_MEMORY
		sprintf(gString, "Memory used %d", CMemInfo::GetCurrentMemUsage());
		VarConsole.AddDebugOutput(gString);
//		CDebug::PrintToScreenCoors(gString, 52, DEBUGTEXTY+(Line++));		
#endif			

		sprintf(gString, "Requested %d, memory size %dK,", CStreaming::GetNumberModelsRequested(), CStreaming::GetMemoryUsed()>>10);
		VarConsole.AddDebugOutput(gString);
//		CDebug::PrintToScreenCoors(gString, DEBUGTEXTX,DEBUGTEXTY+(Line++));

		if (CWorld::Players[0].pPed)
		{
			CVector PlyrPos;
			float PlyrHeading;
			
			PlyrPos = CWorld::Players[0].GetPos();
			
			sprintf(gString, "%f %f %f", PlyrPos.x, PlyrPos.y, PlyrPos.z);
//			CDebug::PrintToScreenCoors(gString, DEBUGTEXTX, DEBUGTEXTY+(Line++));			
			VarConsole.AddDebugOutput(gString);
			if (CWorld::Players[0].pPed->m_nPedFlags.bInVehicle)
			{
				ASSERTMSG(CWorld::Players[0].pPed->m_pMyVehicle, "Player is not in a vehicle");
				PlyrHeading = RADTODEG(CWorld::Players[0].pPed->m_pMyVehicle->GetHeading());
			}
			else
			{
				PlyrHeading = RADTODEG(CWorld::Players[0].pPed->GetHeading());
			}
			
			if (PlyrHeading < 0.0f)
			{
				PlyrHeading += 360.0f;
			}
			
			sprintf(gString, "%f", PlyrHeading);
			VarConsole.AddDebugOutput(gString);
//			CDebug::PrintToScreenCoors(gString, DEBUGTEXTX, DEBUGTEXTY+(Line++));
		}
					// Draw a grey bar behind the text.
/*		CSprite2d::DrawRectXLU( CRect	(0, 0,
									SCREEN_WIDTH,
									MIN(SCREEN_HEIGHT, (Line+1)*16)),
									CRGBA(0,0,0,100), CRGBA(0,0,0,100), CRGBA(0,0,0,100), CRGBA(0,0,0,100));
*/
	}				
	DEBUGCOLSTATS(CCollision::PrintStats();)
#endif
}
#endif

//
// name:		MemoryCardVersionCheck
// description:	Check if memory card with version file is available
void MemoryCardVersionCheck()
{
	#ifdef MAGAZINE_VERSION
	{
		while (!TheMemoryCard.CheckMagazineDirectoryThere(CARD_ONE, DONGLE_STRING, DONGLE_FILENAME) &&
		  	   !TheMemoryCard.CheckMagazineDirectoryThere(CARD_TWO, DONGLE_STRING, DONGLE_FILENAME))
		{
			printf("\n\n*** PLEASE INSERT DONGLE ***\n\n");
// dont try to render until we have sorted the setting up of the camera here.
/*			Char16 UniText[80];

			CFont::SetBackground(false);
			CFont::SetWrapx(640);
			CFont::SetScale( 0.8f, 1.0f);
			CFont::SetOrientation(FO_LEFT);
			CFont::SetCentreSize(640);
			CFont::SetJustify(false);
			CFont::SetColor(CRGBA(255,255,255,255));
			CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);

			AsciiToGxtChar("Version check missing", &UniText[0]);
			CFont::PrintString( SCREEN_WIDTH/2,195, &UniText[0]);
*/
		}
	}
	#endif

}

#ifdef GTA_PS2
//
// name:		CheckForSavedGame
// description:	check for the most recent saved game on the memory card
bool CheckForSavedGame()
{
	if(TheMemoryCard.CheckForSavedGame(CARD_ONE))
	{
		CGame::currLevel = (eLevelName)TheMemoryCard.GetLevelToLoad();
		return true;
	}
	return false;
}
	
#endif
////// Old actual game loop. We have to integrate the new RW stuff
////// with this.

#if !defined(FINAL)
void TheModelViewer()
{
	LoadingScreen("Loading the ModelViewer", NULL);
	
	CAnimViewer::Initialise();
	CTimer::Update();
	
	// Destroy splash screen 
	CLoadingScreen::Shutdown();
	
	// upload custom plates:
//	CCustomCarPlateMgr::Initialise();  not needed as this is done in CAnimViewer::Initialise() (JG)

	while (TRUE)
	{
		CAnimViewer::Update();	// Updates the whole game

		SetLightsWithTimeOfDayColour(Scene.world);
		//CRenderer::ConstructRenderList();

  		//setup the aspect ratio
  		CDraw::CalculateAspectRatio();

		float ViewWindow = CMaths::Tan( (CDraw::GetFOV() * (PI / 360.0f) ) );
	   	CameraSize(Scene.camera, (RwRect *)0, (RwReal)ViewWindow, (RwReal)CDraw::GetAspectRatio()); // last 2 args: VIEWWINDOW, ASPECTRATIO.

		CVisibilityPlugins::SetRenderWareCamera(Scene.camera);

     	CRGBA ScreenColour;

		ScreenColour.red = 100;
		ScreenColour.green = 100;
		ScreenColour.blue = 100;
		
   		// Clear the cameras Z buffer
    	RwCameraClear(Scene.camera, &ScreenColour, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    	RsCameraBeginUpdate(Scene.camera);
 
 		// Initialise list of atomics fading in
		CVisibilityPlugins::InitAlphaEntityList();
		CAnimViewer::Render();
		
	
#ifdef TIMERS
		tbDisplay();
#endif

		DoRWStuffEndOfFrame();
		CTimer::Update();
	}

	CAnimViewer::Shutdown();
	
}
#endif //!FINAL

// Name			:	TheGame
// Purpose		:	Application main function
// Parameters	:	None
// Returns		:	Nothing
void TheGame()
{
#if defined (GTA_PS2)
	bool bFoundRecentSavedGameToLoad;
#endif // GTA_PS2
	
	printf("Into TheGame!!!\n");

	//################################################ Game bit
	
	CMemInfo::EnterBlock(GAME_MEM_ID);
	
	CTimer::Initialise();
	
	CGame::Initialise("DATA\\GTA.DAT");	// Init the things at the start of the game

	LoadingScreen("Starting Game", NULL);

	LOAD_CODE_OVERLAY(mc);
	
#ifdef GTA_PS2
	#ifdef MAGAZINE_VERSION
		MemoryCardVersionCheck();
	#endif
	bFoundRecentSavedGameToLoad = CheckForSavedGame();  // check the memory card for a saved game and set to load it if it finds one

	//
	// GAMELOOP
	//
____RestartGame:

	// If loading then load splash screen
	//if(TheMemoryCard.MemoryCards[CARD_ONE].IsSaveGamePresent())
	//	LoadSplash(GetLevelSplashScreen(CGame::currLevel));

	FrontEndMenuManager.WantsToLoad=FALSE;
	CTimer::Update();
	
	// Destroy splash screen 
	CLoadingScreen::Shutdown();
	FrontEndMenuManager.m_bStartUpFrontEndRequested = FALSE;  // dont want frontend to open when we start a game
	
	while (!(FrontEndMenuManager.m_WantsToRestartGame==true || bFoundRecentSavedGameToLoad))//end of not while
	{
		#ifdef GTA_PS2PEEK_PROFILER
			CPS2Peek::Profile();
		#endif


		#ifdef TIMERS
			tbInit();
		#endif
		CSprite2d::InitPerFrame();
		CFont::InitPerFrame();
		CPointLights::InitForFrame();	// Reset the pointlights. We can start filling them up again now
		
		DEBUGCOLSTATS(CCollision::FlushStats();)

		CMemInfo::EnterBlock(PROCESS_MEM_ID);


		CGame::Process();	// Updates the whole game
		
#ifndef MASTER
		CDebugBugVisualiserMenu::Process();
#endif

#ifndef FINAL
		CPedDebugVisualiserMenu::Process();
#endif	//FINAL
		
#ifndef MASTER // keep bugstar stuff out of the master build
		
		/////////////////////////////////////// call bugstar ////////////////////////////////////
		// added by alex this allow the rigth shoulder button 1 on pad 1
		// to cause a bug to be created and sent to a PC connected to the PS2
		// via firewire
		
		//if (CPad::GetPad(1)->GetShockButtonL())
		if (PS2Keyboard.GetKeyUp(PS2_KEY_CTRL) && PS2Keyboard.GetKeyDown(PS2_KEY_B, KEYBOARD_MODE_STANDARD, "report BugStar bug"))
		{
			ReportBugToBugstar();
		}


		
		///////////////////////////////////////////////////////////////////////////////
#endif // MASTER
		
		
		
#ifndef FINAL
		if(bCarColsEditor)
			CarColsEdit();
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_C, KEYBOARD_MODE_STANDARD, "Car Colour Editor")) bCarColsEditor = !bCarColsEditor;

		CReferences::PrintSomeDebugStuff();
#endif	//FINAL

		CMemInfo::ExitBlock();
	


		#ifdef USE_TEXTURE_CAMERA
			RwRGBA col = {0, 0, 0, 255};
			// Clear the cameras Z buffer & image
			textureCamera->Clear(&col, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);
			
			textureCamera->RenderBegin();
		#endif//USE_TEXTURE_CAMERA

		
		#ifdef TIMERS
			tbStartTimer(0,"Service Audio");
		#endif
		
		CMemInfo::EnterBlock(AUDIO_MEM_ID);
#ifdef USE_AUDIOENGINE
		AudioEngine.Service();
#else //USE_AUDIOENGINE
		DMAudio.Service();
#endif //USE_AUDIOENGINE
		CMemInfo::ExitBlock();

		#ifdef TIMERS
			tbEndTimer("Service Audio");
		#endif

#ifndef MASTER
		VarConsole.Check();
		if (VarConsole.IsOpen)
		{
			static int32 oldDebugStatValue = 9999;
			if (oldDebugStatValue != CStats::DebugStatValue)
			{
				// add some stuff to varconsole for debugging:
				VarConsole.Add("STAT TYPE", &CStats::DebugStatValue, 1, 0, MAX_INT_FLOAT_STATS, TRUE);

				// remove old VarConsole reference to the stat
				VarConsole.Remove("STATS VALUE");

				// add a new reference to the stat debug menu, either float or int stat:
				if (CStats::DebugStatValue < MAX_FLOAT_STATS)
				{
					VarConsole.Add("STATS VALUE", &CStats::StatTypesFloat[CStats::DebugStatValue], 0.1f, 0.0f, 1000.0f, TRUE);
				}
				else
				{
					VarConsole.Add("STATS VALUE", &CStats::StatTypesInt[CStats::DebugStatValue-MAX_FLOAT_STATS], 1, 0, 1000, TRUE);
				}
				oldDebugStatValue = CStats::DebugStatValue;
			}
		}
#endif	//MASTER

		// If running demo mode and 3 minutes have passed and there isn't a cutscene playing then restart
/*		if(CGame::bDemoMode && 
			CTimer::GetTimeInMilliseconds() > (3 * 60 + 30) * 1000 && 
			!CCutsceneMgr::IsCutsceneProcessing())
		{
			FrontEndMenuManager.m_WantsToRestartGame = true;
			FrontEndMenuManager.WantsToLoad=FALSE;
			break;
		}
*/

		// If restart game or load game has been selected
		if(	FrontEndMenuManager.m_WantsToRestartGame==true || bFoundRecentSavedGameToLoad)
			break;

		SetLightsWithTimeOfDayColour(Scene.world);
		
		CMemInfo::EnterBlock(RENDER_MEM_ID);

		// Construct render list even if dark because this requests models in the frustum
		#ifdef TIMERS
			tbStartTimer(0,"ConstrRList");
		#endif

		// moved the pre-calculation step of modifying the water atomics here
		// to try and avoid stalling the graphics pipeline when rendering the water
		// (it doesn't seem to be having the hoped speed improvements tho (on pc at least))
//		CWaterLevel::PreCalcWaterGeometry();
			
		CRenderer::ConstructRenderList();

		
		#ifdef TIMERS
			tbEndTimer("ConstrRList");
		#endif

		// render the game (only if menus are not active):
		if (!FrontEndMenuManager.m_MenuActive && TheCamera.GetScreenFadeStatus() != DARK_SCREEN)
		{

			
			#ifdef TIMERS
				tbStartTimer(0,"Prerender");
			#endif

			CRenderer::PreRender();	// Generate shadows. Set lights etc.

			CWorld::ProcessPedsAfterPreRender();

			#ifdef TIMERS
				tbEndTimer("Prerender");
			#endif

			#ifdef TIMERS
				tbStartTimer(0,"StartOfFrame");
			#endif

			PS2AllRecalculateLighting();			

			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_STARTOFFRAME_START);
			//
			// RENDER THE SCENE
			//
			CMirrors::BeforeMainRender();		// Renders the scene to a bugger if needed. (to render the mirror effect)


			// Do any current camera state changes always BEFORE DoRWStuffStartOfFrame(): 
			// Set some fogging:
	  		if (CPostEffects::m_smokeyEnable)
	  		{
	  	    	RwCameraSetFarClipPlane(Scene.camera, CPostEffects::m_smokeyDistance);
				RwCameraSetFogDistance(Scene.camera, MIN(CTimeCycle::GetFogStart(), CPostEffects::m_smokeyDistance * 0.1f));
				RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)RWRGBALONG(CPostEffects::m_smokeyStrength, CPostEffects::m_smokeyStrength, CPostEffects::m_smokeyStrength, CPostEffects::m_smokeyStrength));
			}
	  	    else
	  	    {
	  	    	RwCameraSetFarClipPlane(Scene.camera, CTimeCycle::GetFarClip());
				RwCameraSetFogDistance(Scene.camera, CTimeCycle::GetFogStart());
			}

			
			// Run the game code outside the RWStart and RWEnd functions.
			if (CWeather::LightningFlash)
			{
				CTimeCycle::SetFogRed(255);
				CTimeCycle::SetFogGreen(255);
				CTimeCycle::SetFogBlue(255);
				DoRWStuffStartOfFrame_Horizon( 255, 255, 255, 255, 255, 255, 255);	// Flash background white
			}
			else
			{
				DoRWStuffStartOfFrame_Horizon( CTimeCycle::GetSkyTopRed(), CTimeCycle::GetSkyTopGreen(), CTimeCycle::GetSkyTopBlue(),
												CTimeCycle::GetSkyBottomRed(), CTimeCycle::GetSkyBottomGreen(), CTimeCycle::GetSkyBottomBlue(), 255);
			}

			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_STARTOFFRAME_END);

			// Set the rendering pipeline to standard
			DefinedState();

			#ifdef TIMERS
				tbEndTimer("StartOfFrame");
			#endif
			
			#ifdef TIMERS
				tbStartTimer(0,"RenderMirrorBuffer");
			#endif
			CMirrors::RenderMirrorBuffer();
			#ifdef TIMERS
				tbEndTimer("RenderMirrorBuffer");
			#endif


			RenderScene();



			
			
				
			#ifdef TIMERS
				tbStartTimer(0,"RenderDebugShit");
			#endif
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_DEBUGSHIT_START);

			RenderDebugShit();	
			
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_DEBUGSHIT_END);
			#ifdef TIMERS
				tbEndTimer("RenderDebugShit");
			#endif


			
			RenderEffects();
			
			
			
			
			#ifdef TIMERS
				tbStartTimer(0,"MotionBlur");
			#endif
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_MBLUR_START);
			
			if ((TheCamera.GetBlurType()==MB_BLUR_NONE) || (TheCamera.GetBlurType()==MB_BLUR_LIGHT_SCENE)) 
			{
				if (TheCamera.m_ScreenReductionPercentage>0)
				{
					TheCamera.SetMotionBlurAlpha(150);
				}
			}

	        TheCamera.RenderMotionBlur();
			
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_MBLUR_END);
			#ifdef TIMERS
				tbEndTimer("MotionBlur");
			#endif




			#ifdef TIMERS
				tbStartTimer(0,"Render2dStuff");
			#endif
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDER2DSTUFF_START);
				Render2dStuff();
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDER2DSTUFF_END);
			#ifdef TIMERS
				tbEndTimer("Render2dStuff");
			#endif
		}
		else
		{
			CDraw::CalculateAspectRatio();
			float ViewWindow = CMaths::Tan( (CDraw::GetFOV() * (PI / 360.0f) ) );
			CameraSize(Scene.camera, (RwRect *)0, (RwReal)ViewWindow, CDraw::GetAspectRatio()); // last 2 args: VIEWWINDOW, ASPECTRATIO.
			CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
			RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ/*|rwCAMERACLEARIMAGE*/);
			if (!RsCameraBeginUpdate(Scene.camera))
			{
				ASSERT(0);
			}		
		}		
	
	
		#ifdef TIMERS
			tbStartTimer(0,"RenderMenus");
		#endif
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERMENUS_START);
		
		#ifndef MASTER
		#ifndef FINAL
		if (!gbDrawPolyDensity)
		#endif	//FINAL
		{
			if (gDisplayDebugInfo) VarConsole.RenderDebugOutput();
		}
		
		VarConsole.debug_count = 0;  // this MUST be called to prevent the list from overflowing
		#endif	//MASTER
	
		RenderMenus();

		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERMENUS_END);
		#ifdef TIMERS
			tbEndTimer("RenderMenus");
		#endif

		// If the memory card wants to load. Break out of loop here so that Derek's frontend
		// screen is still visible.
		if(FrontEndMenuManager.WantsToLoad == true)
		{
			CMemInfo::ExitBlock();
			break;
		}	
			
			
		#ifdef TIMERS
			tbStartTimer(0,"Fade");
		#endif
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_DOFADE_START);
			DoFade ();
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_DOFADE_END);
		#ifdef TIMERS
			tbEndTimer("Fade");
		#endif
		
		#ifdef TIMERS
			tbStartTimer(0,"Render2dStuffAft");
		#endif
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_2DSTUFFAFTERFADE_START);
			Render2dStuffAfterFade();
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_2DSTUFFAFTERFADE_END);
		#ifdef TIMERS
			tbEndTimer("Render2dStuffAft");
		#endif


		#ifdef TIMERS
			if (CHud::m_Wants_To_Draw_Hud==TRUE)
			{
				tbDisplay();
				CFont::DrawFonts();
			}
		#endif
		
		// display load-monitor bars
		g_LoadMonitor.Render();

		
		DoRWStuffEndOfFrame();


#ifdef USE_TEXTURE_CAMERA
		textureCamera->RenderEnd();
#endif//USE_TEXTURE_CAMERA

		CTimer::Update();
			
		CMemInfo::ExitBlock();
	}
	
	CCheat::ResetCheats();
	CPad::StopPadsShaking();
	// Ray test
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	DMAudio.ChangeMusicMode(A_MUSIC_OFF);
#endif //USE_AUDIOENGINE
	// End of Ray test

	CGame::ShutDownForRestart();
	CTimer::Stop();


	if ((FrontEndMenuManager.m_WantsToRestartGame==true) || bFoundRecentSavedGameToLoad)
	{
		if (bFoundRecentSavedGameToLoad==true )
		{
			FrontEndMenuManager.m_WantsToRestartGame=true;
			FrontEndMenuManager.WantsToLoad=TRUE;
			FrontEndMenuManager.MessageScreen("FELD_WR", FALSE);
//			DoRWStuffStartOfFrame( 0,0,0, 0,0,0, 0);
//			DoRWStuffEndOfFrame();
		}
		
/*		
		if (FrontEndMenuManager.WantsToLoad)
			CLoadingScreen::Init("splash1");  // display splash screen for loading game
		else
			CLoadingScreen::Init(); // display splash screen for restarting game
*/	
		
		CGame::InitialiseWhenRestarting();
		bFoundRecentSavedGameToLoad = false;
		FrontEndMenuManager.m_WantsToRestartGame=false;

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
		DMAudio.ChangeMusicMode(A_GAME_MUSIC_MODE);
#endif //USE_AUDIOENGINE

		goto ____RestartGame;
	}


#ifdef DEBUG_MEMORY
	int32 Counter=1000;
	while(Counter--)
	{
		PrintMemoryUsage();
		DoRWStuffStartOfFrame( 50, 50, 50, 0, 0, 0, 255);
		DoRWStuffEndOfFrame();
	}	
#endif //DEBUG_MEMORY
#endif	

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
____ExitApplication:
	DMAudio.Terminate();
#endif //USE_AUDIOENGINE
	
} // end - main





//
//
//
//
void RenderScene()
{
	bool bUnderWater = (CWeather::UnderWaterness > 0.0f);
#ifndef FINALBUILD		
//	if ( (!CPad::GetPad(1)->GetLeftShoulder1()) || (!CPad::GetPad(1)->GetRightShoulder1()) )
if (!PS2Keyboard.GetKeyDown(PS2_KEY_NUMPAD_7, KEYBOARD_MODE_STANDARD, "DisableRenderScene"))
#endif			
	{
#ifdef MOBILE
		static float gBufferFixAmount = 4.0f;
		static float smallestZUnit = 1.0f / 65536.0;
			
		float zFix = smallestZUnit*gBufferFixAmount;
			
		float clipNearOld = RwCameraGetNearClipPlane( Scene.camera );
		float clipFarOld  = RwCameraGetFarClipPlane( Scene.camera );				
		float n = ( clipFarOld - clipNearOld ) * zFix;

		RwCameraEndUpdate(Scene.camera);
		RwCameraSetNearClipPlane( Scene.camera, clipNearOld + n );
		RwCameraBeginUpdate(Scene.camera);
#endif

	    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
	    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
	    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);

		
		//
		// Render background
		//
		START_TIMER("RenderClouds");
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERCLOUDS_START);

		if (CMirrors::TypeOfMirror == CMirrors::MIRTYPE_NONE)
		{
			CMovingThings::Render_BeforeClouds();
			CClouds::Render();

		}

		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERCLOUDS_END);
		END_TIMER("RenderClouds");

	    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
	    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);


		#ifdef USE_VU_CARFX_RENDERER
			CVuCarFXRenderer::PreRenderUpdate();
		#endif
		#ifdef USE_PC_CARFX_RENDERER
			CCarFXRenderer::PreRenderUpdate();
		#endif


		START_TIMER("RenderScene");
		//
		// Render scene
		//

		// "SeamRemoverBegin" needs to be called right BEFORE the scene is about
		// to be rendered! 16.09.2004 - AI
		CPostEffects::SeamRemoverBegin();

		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERROADS_START);
		CRenderer::RenderRoads();			
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERROADS_END);

		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_REFLECTIONS_START);
		CCoronas::RenderReflections();
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_REFLECTIONS_END);

		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_EVERYTHINGBUTROADS_START);
		CRenderer::RenderEverythingBarRoads();
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_EVERYTHINGBUTROADS_END);

		// "SeamRemoverEnd" needs to be called right AFTER the scene has been
		// rendered! 16.09.2004 - AI
		CPostEffects::SeamRemoverEnd();

		#ifdef TIMERS
			tbEndTimer("RenderScene");
		#endif			
		
		// MN - BREAKABLE OBJECTS -----------------------
		START_TIMER("BreakMgr::Render");
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_BREAKMGR_START);
		g_breakMan.Render(false);
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_BREAKMGR_END);
		END_TIMER("BreakMgr::Render");

		// ----------------------------------------------

//		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERTWISTER_START);
//		CWeather::RenderTwister();
//		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERTWISTER_END);


		START_TIMER("RenderUnderwater");
		//CRenderer::RenderBoats();
		CRenderer::RenderFadingInUnderwaterEntities();
		END_TIMER("RenderUnderwater");


		if(!bUnderWater)
		{
			START_TIMER("RenderWater");
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
			CWaterLevel::RenderWater();
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
			END_TIMER("RenderWater");
		}

		START_TIMER("RenderFading");
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERFADING_START);
		CRenderer::RenderFadingInEntities();
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERFADING_END);
		END_TIMER("RenderFading");

		if (!CMirrors::bRenderingReflection)		
		{		

#if defined(DW_ZFIGHT_TRICK_FIX) && !defined(MOBILE)
			static float gDegreeShadowZBufferFix = 2.0f; // doubled to rule out errors.          //0.000025f;//(1.0f/32768.0f); // zbuffer resolutiuon has increased - theoretically we only need to move the z resolution by 1.0f/8388608.0f but have opted for a slightly safer value.
			static float smallestZUnit = 1.0f / (65536.0f*256.0f);

			float shadowZ = smallestZUnit*gDegreeShadowZBufferFix;
			
			float clipNearOld = RwCameraGetNearClipPlane( Scene.camera );

			// DW - I have to reduce the bias when looking above because of blood pools under peds.
			float z = TheCamera.Cams[TheCamera.ActiveCam].Front.z;
			if (z>0.0f) z = 0.0f;			
			else z = -z;			
			shadowZ  = DW_LERP(z,shadowZ,shadowZ*0.25f);
			
			float clipFarOld  = RwCameraGetFarClipPlane( Scene.camera );				
			float n = ( clipFarOld - clipNearOld ) * shadowZ;

			RwCameraEndUpdate(Scene.camera);
			RwCameraSetNearClipPlane( Scene.camera, clipNearOld + n );
			RwCameraBeginUpdate(Scene.camera);
#endif 

			START_TIMER("StaticShd");
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_STATICSHADOWS_START);
				CShadows::UpdateStaticShadows();
				CShadows::RenderStaticShadows();
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_STATICSHADOWS_END);
			END_TIMER("StaticShd");

			START_TIMER("StoredShd");
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_STOREDSHADOWS_START);
				 CShadows::RenderStoredShadows();
			CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_STOREDSHADOWS_END);
			END_TIMER("StoredShd");

#if defined(DW_ZFIGHT_TRICK_FIX) && !defined(MOBILE) // reinstate clip planes
			RwCameraEndUpdate(Scene.camera);
			RwCameraSetNearClipPlane(Scene.camera, clipNearOld);
			RwCameraBeginUpdate(Scene.camera);	
#endif
		}

			
			
		// MN - ALPHAd BREAKABLE OBJECTS ----------------
		START_TIMER("BreakMgr::RenderAs");
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_BREAKMGRALPHA_START);
			
		g_breakMan.Render(true);

		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_BREAKMGRALPHA_END);
		END_TIMER("BreakMgr::RenderAs");

//#ifndef MASTER
//		COcclusion::Render3D();	// Test thingy
//#endif

		// draw plants here after fading-in entities (objects with alpha):
		#ifdef USE_PLANT_MGR
			START_TIMER("CPlantMgr::Render");
			CPlantMgr::Render();
			END_TIMER("CPlantMgr::Render");
		#endif	




		RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);

		START_TIMER("RenderRainAndSun");
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERRAINANDSUN_START);
		if (CMirrors::TypeOfMirror == CMirrors::MIRTYPE_NONE)
		{
			CClouds::RenderBottomFromHeight();
			CWeather::RenderRainStreaks();
			CCoronas::RenderSunReflection();
		}
		
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERRAINANDSUN_END);
		END_TIMER("RenderRainAndSun");

		//

		if(bUnderWater)
		{
			START_TIMER("RenderWater");
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
			CWaterLevel::RenderWater();
			RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
			END_TIMER("RenderWater");
		}

#ifdef GTA_PS2

		// This is for PS2
		// (For the PC and XBOX the following is done in "RenderEffects())"!
		START_TIMER("RenderWaterFog");
		// Render "water fog" (before voulme clouds and moving fog)!
		CWaterLevel::RenderWaterFog();
		END_TIMER("RenderWaterFog");

		START_TIMER("RenderMovingFog");
		// Render (moving) "fog"!
		CClouds::MovingFogRender();
		END_TIMER("RenderMovingFog");

		START_TIMER("RenderVolumeClouds");
		// Render "volume clouds" as last step!
		CClouds::VolumetricCloudsRender();
		END_TIMER("RenderVolumeClouds");

#endif

#ifdef USE_VOLUMETRIC_SHADOW_RENDERER
		START_TIMER("VolShadowMgr");
			CVolumetricShadowMgr::Render();
		END_TIMER("VolShadowMgr");
#endif
	}

}// end of RenderScene()...


//
//
//
//
void RenderDebugShit()
{
#ifndef MASTER
#ifdef GTA_PS2
#ifndef FINAL
	if(!bCarColsEditor)
#endif
#endif
	{
		ThePaths.DisplayPathData();
	}
	
	CStreamingDebug::RenderHeadPosition();
#endif

		// MN - IK & RAGDOLL PHYSICS --------------------
#ifndef FINAL
		// need to do this before RenderTheDebugLines() or the lines we draw
		// won't get rendered until the next frame
		g_ikChainMan.RenderDebug();
//		g_ragdollMan.RenderDebug();
#endif
		// ----------------------------------------------


#ifndef MASTER
		CDebugBugVisualiserMenu::Render();
#endif

#ifndef FINALBUILD
#ifdef GTA_PS2
	if(!bCarColsEditor)
#endif
	{
#ifdef CLOTHES_TEST
		CClothes::RenderTest();
#endif		
		if ((/*CPad::GetPad(1)->GetRightShoulder2() ||*/ PS2Keyboard.GetKeyDown(PS2_KEY_F1, KEYBOARD_MODE_STANDARD, "Render Collision lines")) && !CGameLogic::Disable2ndControllerForDebug())
		{
			CRenderer::RenderCollisionLines();
		}

		if ( (bDisplayPathsDebug || (PS2Keyboard.GetKeyDown(PS2_KEY_NUMPAD_MINUS, KEYBOARD_MODE_STANDARD, "Display path data") || CPad::GetPad(1)->GetLeftShoulder2()) && !CGameLogic::Disable2ndControllerForDebug()))
		{
			CVehicle::DebugRenderCloseVehicleBoundingBoxes();
		}

		
		if (PS2Keyboard.GetKeyDown(PS2_KEY_NUMPAD_ASTERISK, KEYBOARD_MODE_STANDARD, "ped visualiser toggle")) //	if(CPad::GetPad(1)->SelectJustDown())
			CPedDebugVisualiser::SwitchDebugDisplay();
			
		CPedDebugVisualiserMenu::Render();
		
	
	    if(CPedDebugVisualiser::GetDebugDisplay()>1)
		{
	  	    CPedDebugVisualiser debugVisualiser;
	  	    debugVisualiser.VisualiseAll();
		}

		CDebug::RenderTheDebugLines();
		CDebug::RenderTheDebugSpheres();
		CDebug::RenderTheDebugPolys();
		CTheScripts::RenderTheScriptDebugLines();
							
		// MN - INTERIORS -------------------------------
#ifndef FINAL
		#ifdef TIMERS
			tbStartTimer(0,"Interiors::Render");
		#endif
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_INTERIORS_START);
		
		g_interiorMan.Render();
		
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_INTERIORS_END);
		#ifdef TIMERS
			tbEndTimer("Interiors::Render");
		#endif
#endif
		// ----------------------------------------------
	
	
		// MN - PROCEDURAL OBJECTS ----------------------
#ifndef FINAL
		g_procObjMan.Render();
#endif
		// ----------------------------------------------


#ifndef FINAL
		if (CAudioZones::m_bRenderAudioZones)
		{
			CAudioZones::Render();
		}
		if (CCullZones::bRenderCullzones)
		{
			CCullZones::Render();
		}
		if (CEntryExitManager::bRenderEntryExits)
		{
			CEntryExitManager::RenderEntryExits();
		}
#endif


	}
	
#endif

}


//
//
//
//
void RenderEffects()
{
	START_TIMER("Birds");
	CBirds::Render();
	END_TIMER("Birds");

	START_TIMER("CSkidmarks");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SPECIALFX_START);
		CSkidmarks::Render();
		CRopes::Render();
//		CAntennas::Render();
//		CRubbish::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SPECIALFX_END);
	END_TIMER("CSkidmarks");

	START_TIMER("Glass");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_GLASS_START);
		CGlass::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_GLASS_END);
	END_TIMER("Glass");

	START_TIMER("CMovingThings");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_MOVINGTHINGS_START);
		CMovingThings::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_MOVINGTHINGS_END);
	END_TIMER("CMovingThings");

	START_TIMER("ReallyDrawLast");
	CVisibilityPlugins::RenderReallyDrawLastObjects();
	END_TIMER("ReallyDrawLast");
	
	START_TIMER("CCoronas");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_CCORONAS_START);
		CCoronas::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_CCORONAS_END);
	END_TIMER("CCoronas");

	// -- FX SYSTEM ---------------------------------
	
	START_TIMER("FxSystem Render");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_FXSYSTEM_START);
	
	g_fx.Render(TheCamera.m_pRwCamera, false);
	
//	g_fxMan.Render(TheCamera.m_pRwCamera);

	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_FXSYSTEM_END);
	END_TIMER("FxSystem Render");

	

	START_TIMER("CWaterCannons");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_WATERCANNONS_START);
		CWaterCannons::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_WATERCANNONS_END);
	END_TIMER("CWaterCannons");

	//

#if defined (GTA_PC) || defined (GTA_XBOX)

	// This is for PC & XBOX
	// (For the PS2 the following is done in "RenderScene())"!
	START_TIMER("RenderWaterFog");
	// Render "water fog" (before voulme clouds and moving fog)!
	CWaterLevel::RenderWaterFog();
	END_TIMER("RenderWaterFog");

	START_TIMER("RenderMovingFog");
	// Render (moving) "fog"!
	CClouds::MovingFogRender();
	END_TIMER("RenderMovingFog");

	START_TIMER("RenderVolumeClouds");
	// Render "volume clouds" as last step!
	CClouds::VolumetricCloudsRender();
	END_TIMER("RenderVolumeClouds");

#endif

	// ----------------------------------------------
	
	if (CHeli::GetNumberOfHeliSearchLights() || CTheScripts::GetNumberOfSearchLights())
	{
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_HELISEARCHLIGHTS_START);
			CHeli::Pre_SearchLightCone();
			CHeli::RenderAllHeliSearchLights();
			CTheScripts::RenderAllSearchLights();
			CHeli::Post_SearchLightCone();
		CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_HELISEARCHLIGHTS_END);
	}

	
	START_TIMER("CWeaponEffects");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_WEAPONEFFECTS_START);
		CWeaponEffects::Render();
#ifdef GTA_PC
		if (!CReplay::ReplayGoingOn() && !CPad::GetPad(0)->IsDisabled())
		{
			FindPlayerPed()->DrawTriangleForMouseRecruitPed();
		}
#endif//ifdef GTA_PC
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_WEAPONEFFECTS_END);
	END_TIMER("CWeaponEffects");

		
	START_TIMER("CSpecialsFX");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SKIDMARKSSTUNTS_START);
		CSpecialFX::Render();
		CVehicleRecording::Render();
//		CStunts::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SKIDMARKSSTUNTS_END);
	END_TIMER("CSpecialsFX");

	
//	START_TIMER("CPacManPickups");
//	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_PACMANPICKUPS_START);
//		CPacManPickups::Render();
//	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_PACMANPICKUPS_END);
//	END_TIMER("CPacManPickups");


	START_TIMER("RenderFogEffect");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERFOGEFFECT_START);
	CPointLights::RenderFogEffect();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_RENDERFOGEFFECT_END);
	END_TIMER("RenderFogEffect");


	START_TIMER("CLightning");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_CLIGHTING_START);
//		CLightning::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_CLIGHTING_END);
	END_TIMER("CLightning");
	
#ifndef FINAL
	CCover::Render();
#endif


	// ----------------------------------------------


/*		
	#ifdef TIMERS
		tbStartTimer(0,"RenderSmogEffect");
	#endif
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SMOGEFFECT_START);
		CPointLights::RenderSmogEffect();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SMOGEFFECT_END);
	#ifdef TIMERS
		tbEndTimer("RenderSmogEffect");
	#endif
*/
	
	CRenderer::RenderFirstPersonVehicle();
	
	// WDFIXME
#ifndef OSW
	START_TIMER("PostEffects");
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SMOGEFFECT_START);
		CPostEffects::Render();
	CPS2PEEK_INSERT_VIFMARK(CPS2PEEK_VIFMARK_SMOGEFFECT_END);
	END_TIMER("PostEffects");
#endif
	
}

void Render2dStuff()
{
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *) rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *) rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
#if (defined GTA_PC || defined GTA_XBOX)
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
#endif	

	CReplay::Display();
	CPopCycle::Display();

	#ifdef TIMERS
		tbStartTimer(0,"RenderPickUpText");
	#endif
	CPickups::RenderPickUpText();
#ifdef GTA_NETWORK
	CNetGames::RenderPlayerNames();
#endif
	#ifdef TIMERS
		tbEndTimer("RenderPickUpText");
	#endif

	if (TheCamera.m_WideScreenOn && !FrontEndMenuManager.m_PrefsUseWideScreen) // DW - if in widescreen in frontend you dont want the chopping off of stuff that the borders do.
	{
		TheCamera.DrawBordersForWideScreen();
	}
 
	CPlayerPed* pPlayer ;
	uint32 CurrentPlayerWeapon;
	pPlayer=FindPlayerPed();
	if (pPlayer!=NULL)
	{
		CurrentPlayerWeapon=pPlayer->GetWeapon()->GetWeaponType();
	}

 	bool FirstPersonWeaponCamModeActive=false;
 	
 	if(TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_SNIPER
 	|| TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_SNIPER_RUNABOUT
	|| TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER
	|| TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT
	|| TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_CAMERA
	|| TheCamera.Cams[TheCamera.ActiveCam].Mode==CCam::MODE_HELICANNON_1STPERSON)
 	{
 		FirstPersonWeaponCamModeActive=true;
 	}
 	
 	if (((CurrentPlayerWeapon==WEAPONTYPE_SNIPERRIFLE) || (CurrentPlayerWeapon==WEAPONTYPE_ROCKET) 
 			//|| (CurrentPlayerWeapon==WEAPONTYPE_LASERSCOPE)
 		)&&(FirstPersonWeaponCamModeActive))
  	{
    	CRGBA CircleColour=CRGBA(0,0,0,255);
		if (CurrentPlayerWeapon==WEAPONTYPE_ROCKET)
		{
			//top
			CSprite2d::DrawRect( CRect	(0,0,SCREEN_WIDTH, (SCREEN_HEIGHT/2)-(180 * HUD_MULT_Y)),
										CircleColour);
			//bottom
			CSprite2d::DrawRect( CRect	(0,(SCREEN_HEIGHT/2)+(170*HUD_MULT_Y),SCREEN_WIDTH, SCREEN_HEIGHT),
										CircleColour);
		}
		else
		{
			//top
			CSprite2d::DrawRect( CRect	(0,0,SCREEN_WIDTH, (SCREEN_HEIGHT/2)-(210 * HUD_MULT_Y)),
										CircleColour);
			//bottom
			CSprite2d::DrawRect( CRect	(0,(SCREEN_HEIGHT/2)+(210 * HUD_MULT_Y),SCREEN_WIDTH, SCREEN_HEIGHT),
										CircleColour);
		}
		
		//Left
		CSprite2d::DrawRect( CRect	(0,0,(SCREEN_WIDTH/2)-(210*HUD_MULT_X),SCREEN_HEIGHT),
									CircleColour);
		//Right
		CSprite2d::DrawRect( CRect	((SCREEN_WIDTH/2)+(210*HUD_MULT_X),0,SCREEN_WIDTH,SCREEN_HEIGHT),
									CircleColour);


	}

#ifdef USE_AUDIOENGINE
	AudioEngine.DisplayRadioStationName();
#else //USE_AUDIOENGINE
	MusicManager.DisplayRadioStationName();
#endif //USE_AUDIOENGINE	

//	TheConsole.Display();
#ifdef GTA_NETWORK
	CGameNet::PrintNetworkMessagesToScreen( );
	
	TheConsole.Display();
#endif

#ifdef GTA_PC
	//!PC - CSceneEdit disabled
/*	if(CSceneEdit::m_bEditOn)
	{
		CSceneEdit::Draw();
	}
	else*/
#endif	
		CHud::Draw( );
		
#ifndef OSW
		if ( CPad::GetPad(0)->GetRightMouseButtonOn() ){
		    CTouchInterface::VisualizeAll();
		}
		else
#endif
		{
		    CTouchInterface::DrawAll();
		}


#ifndef MASTER	
	CStreamingDebug::Render();
#endif

	CSpecialFX::Render2DFXs();
	CUserDisplay::OnscnTimer.ProcessForDisplay();
	CMessages::Display(true);
	CDarkel::DrawMessages();
	CGarages::PrintMessages();
	CFont::DrawFonts(  );

#ifndef FINAL
	if (gbDisplayPedsAndCarsNumbers) CWorld::DisplayDebugInfoOnPedsAndCars();
#endif
#ifndef MASTER
	COcclusion::Render();
#endif
#ifndef FINAL
	if (CPlayerPed::bDebugTargetting)
	{
		FindPlayerPed()->DisplayTargettingDebug();
	}
#endif


}

void RenderMenus()
{
//	static UInt8	postFx_R = 127, postFx_G = 127, postFx_B = 127; // store the in-game postFx brightness color	
//	static UInt8	postFx_R2 = 0, postFx_G2 = 0, postFx_B2 = 0; // store the in-game postFx brightness color	
#ifdef DEBUG
#ifdef GTA_PS2

	if(CPad::GetPad(1)->StartButtonJustUp())
	{
		FrontEndMenuManager.m_MenuActive = !FrontEndMenuManager.m_MenuActive;
		FrontEndMenuManager.TimeStandsStill = !FrontEndMenuManager.TimeStandsStill;
		CTimer::m_CodePause != CTimer::m_CodePause;
	}

#endif
#endif


	if(FrontEndMenuManager.m_MenuActive)
	{
		//CMemInfo::EnterBlock(FRONTEND_MEM_ID);
		FrontEndMenuManager.DrawFrontEnd();
		//CMemInfo::ExitBlock();
/*		if (!(postFx.Red == 127 && postFx.Green == 127 && postFx.Blue == 127))
		{
			postFx_R = postFx.Red;
			postFx_G = postFx.Green;
			postFx_B = postFx.Blue;
		}
		postFx.SetColour(127, 127, 127, 0);
		if (!(postFx.Red_2 == 0 && postFx.Green_2 == 0 && postFx.Blue_2 == 0))
		{
			postFx_R2 = postFx.Red_2;
			postFx_G2 = postFx.Green_2;
			postFx_B2 = postFx.Blue_2;
		}
		postFx.SetColour(0, 0, 0, 1);*/
	}
	else
	{
/*		if (postFx.Red == 127 && postFx.Green == 127 && postFx.Blue == 127)
			postFx.SetColour(postFx_R, postFx_G, postFx_B, 0);
		if (postFx.Red_2 == 0 && postFx.Green_2 == 0 && postFx.Blue_2 == 0)
			postFx.SetColour(postFx_R2, postFx_G2, postFx_B2, 1);*/
		#ifndef MASTER
		ProcessKeyboardDebug();
		#endif
	}
//	postFx.Update();
}

void Render2dStuffAfterFade()
{
	CHud::DrawAfterFade( );
#ifndef FINAL
#ifdef GTA_PS2
	if(!bCarColsEditor)
#endif
#endif

//!PC - doesn't work on PC yet
#ifndef MASTER
//#if defined (GTA_PS2)
	DisplayGameDebugText();
//#endif //GTA_PS2
#endif // MASTER

	CMessages::Display(false);
	CFont::DrawFonts();
	CCredits::Render();
}

#if defined (GTA_PC) || defined (GTA_XBOX)

//
//
//
//
void Idle(void* param)
{
    // there is a minimum time for a frame which _must_ be adhered to! (to prevent FPS going over limit)
	const	uint32		MIN_FRAME_TIME = 14;	// 1000ms / 70FPS roughly = 14
//	const	uint32		MIN_FRAME_TIME = 100;	// 10FPS
	static Int32 timeLastIdle = 0;

//!PC - only lock max frame rate for PC
#if defined (GTA_PC)
    UInt32 timeNow = 	CTimer::GetCurrentTimeInMilleseconds();

    while((CTimer::GetCurrentTimeInMilleseconds() - timeLastIdle) < MIN_FRAME_TIME) {}
    
    timeLastIdle = CTimer::GetCurrentTimeInMilleseconds();
#endif //GTA_PC

	CTimer::Update();

#if (defined(GTA_XBOX) && defined(ENABLE_DONGLE))
	extern bool DoXBoxDongleCheck();
	if(false == DoXBoxDongleCheck())
		return;
#endif


#ifdef GTA3_HASP



	// check Hasp4 time every 60th frame:
	static int hasp_waiter=60;
	if(!(--hasp_waiter))
	{
		#define	HASP_FAIL_EXIT()				{ RsGlobal.quit = TRUE; return; }

		hasp_waiter=60;

		// check for current plugged-in HASP's ID serial number (maybe dongles were swapped?):
		unsigned int currentID=0;
		if(!gHaspDevice.GetHaspID(&currentID))
		{
			DEBUGLOG("HASP4 error obtaining ID...failed!\n");
			HASP_FAIL_EXIT();
		}
		if(currentID != gHaspIDNumber)
		{
			// not the same number as the the game start:
			HASP_FAIL_EXIT();
		}
		
		
		#ifdef GTA3_HASP_DATELIMIT
			//
			// do not allow to run game after 5th April 2002:
			//
			if(!gHaspDevice.IsHaspDateBeforeThatDate(GTA3_HASP_MAX_DATE_DAY, GTA3_HASP_MAX_DATE_MONTH, GTA3_HASP_MAX_DATE_YEAR))
			{
				DEBUGLOG3("HASP4: You tried to run game after given date (%d.%d.20%d)!\n", GTA3_HASP_MAX_DATE_DAY, GTA3_HASP_MAX_DATE_MONTH, GTA3_HASP_MAX_DATE_YEAR);
				HASP_FAIL_EXIT();
			}
		#endif//GTA3_HASP_DATELIMIT...


		
		// current Hasp4 time in Minutes:		
		float Minutes=0;
		if(!gHaspDevice.GetHaspTimeInMinutes(&Minutes))
		{
			DEBUGLOG("HASP4 getting time...failed!\n");
			HASP_FAIL_EXIT();
		}

		#ifdef GTA3_HASP_GAMEPLAYLENGTH
			float timeDelta = Minutes - gHaspGameStartedInMinutes;
			while(timeDelta<0)
				timeDelta += CHASP_MINUTES_PER_DAY;

			if(timeDelta > float(GTA3_HASP_GAMEPLAYLENGTH))
			{
				// game was running for more than x minutes:
				DEBUGLOG1("HASP4: No more than %d minutes of gameplay allowed!", GTA3_HASP_GAMEPLAYLENGTH);
				HASP_FAIL_EXIT();
			}
			else
			{
				static int waiter=30;
				if(!(--waiter))
				{
					waiter=30;
					float time = float(GTA3_HASP_GAMEPLAYLENGTH)-timeDelta;
					int mins = int(floor(time));
					int secs = int((time-floor(time))*60.0f);
					DEBUGLOG2("\n\n[HASP4]: only %d:%d minute(s) left till the end of gameplay...\n\n", mins, secs);
				}
			}
		#endif//GTA3_HASP_GAMEPLAYLENGTH
		
		#undef HASP_FAIL_EXIT
	}//if(!(--hasp_waiter))...
	
#endif//GTA3_HASP


	{
		#ifdef TIMERS
			tbInit();
		#endif
		
		CTouchInterface::Clear();
		
		CSprite2d::InitPerFrame();
		CFont::InitPerFrame();
		CPointLights::InitForFrame();	// Reset the pointlights. We can start filling them up again now
		
		DEBUGCOLSTATS(CCollision::FlushStats();)

		CMemInfo::EnterBlock(PROCESS_MEM_ID);

		CGame::Process();	// Updates the whole game

		CMemInfo::ExitBlock();
	
#if !defined(MASTER) && !defined(MOBILE)
		//Bugstar interface for PC
		if (PS2Keyboard.GetKeyDown(PS2_KEY_B, KEYBOARD_MODE_STANDARD, "report BugStar bug"))
		{
			ReportBugToBugstar();
		}
#endif //MASTER

#ifndef FINAL
		CPedDebugVisualiserMenu::Process();
#endif	//FINAL


		#ifdef USE_TEXTURE_CAMERA
			RwRGBA col = {0, 0, 0, 255};
			// Clear the cameras Z buffer & image
			textureCamera->Clear(&col, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);
			
			textureCamera->RenderBegin();
		#endif//USE_TEXTURE_CAMERA

		#ifdef TIMERS
			tbStartTimer(0,"Service Audio");
		#endif
		
#ifdef USE_AUDIOENGINE
		AudioEngine.Service();
#else //USE_AUDIOENGINE
		DMAudio.Service();	
#endif //USE_AUDIOENGINE

#ifndef MASTER
		//!PC - do the var console checking stuff
		VarConsole.Check();
		if (VarConsole.IsOpen)
		{
			static int32 oldDebugStatValue = 9999;
			if (oldDebugStatValue != CStats::DebugStatValue)
			{
				// add some stuff to varconsole for debugging:
				VarConsole.Add("STAT TYPE", &CStats::DebugStatValue, 1, 0, MAX_INT_FLOAT_STATS, TRUE);

				// remove old VarConsole reference to the stat
				VarConsole.Remove("STATS VALUE");

				// add a new reference to the stat debug menu, either float or int stat:
				if (CStats::DebugStatValue < MAX_FLOAT_STATS)
				{
					VarConsole.Add("STATS VALUE", &CStats::StatTypesFloat[CStats::DebugStatValue], 0.1f, 0.0f, 1000.0f, TRUE);
				}
				else
				{
					VarConsole.Add("STATS VALUE", &CStats::StatTypesInt[CStats::DebugStatValue-MAX_FLOAT_STATS], 1, 0, 1000, TRUE);
				}
				oldDebugStatValue = CStats::DebugStatValue;
			}
		}
#endif //MASTER
		
		#ifdef TIMERS
			tbEndTimer("Service Audio");
		#endif
		// If running demo mode and 3 minutes have passed and there isn't a cutscene playing then restart
/*		if(CGame::bDemoMode && 
			CTimer::GetTimeInMilliseconds() > (3 * 60 + 30) * 1000 && 
			!CCutsceneMgr::IsCutsceneProcessing())
		{
			FrontEndMenuManager.m_WantsToRestartGame = true;
			FrontEndMenuManager.WantsToLoad=FALSE;
			return;
		}
*/
//!PC - old stuff?
		// If restart game or load game has been selected
//		if(	FrontEndMenuManager.m_WantsToRestartGame==true || bFoundRecentSavedGameToLoad==true)
//			return;
		SetLightsWithTimeOfDayColour(Scene.world);
		
		if((RwBool)param == FALSE)
			return;
		
		CMemInfo::EnterBlock(RENDER_MEM_ID);

    	
		// Enabling the widget needs to be done before the render here.
		
		if ( CWidgetPlayerInfo::PassesDisplayConditions() )
		{
		    CTouchInterface::IsReleased ( CTouchInterface::WIDGET_PLAYER_INFO );  // Keep the widget enabled.
		}

		CTouchInterface::UpdateAll();  // This must happen at the end of the frame, as the widgets are enabled over the course of the frame.


		// render the game (only if the menus are not active):
		if (!FrontEndMenuManager.m_MenuActive && TheCamera.GetScreenFadeStatus() != DARK_SCREEN)
		{
#if defined (GTA_PC)
			//!PC - temp stuff to stop mouse ptr escaping window whilst playing  
			// Only an issue in windowed mode - but it's useful for debugging!
        	
            if ( false )  // WDTOM This breaks window sizing which i need to work for a while.
            {  
			    RwV2d pos;
        	    pos.x = RsGlobal.screenWidth * 0.5f;
    		    pos.y = RsGlobal.screenHeight * 0.5f;
    		    RsMouseSetPos(&pos);
			}
			
#endif //GTA_PC
    		//--------
    		
			#ifdef TIMERS
				tbStartTimer(0,"ConstrRList");
			#endif

			CMemInfo::EnterBlock(PREALLOC_MEM_ID);
			
			// moved the pre-calculation step of modifying the water atomics here
			// to try and avoid stalling the graphics pipeline when rendering the water
			// (it doesn't seem to be having the hoped speed improvements tho (on pc at least))
//!PC - old stuff?
//			CWaterLevel::PreCalcWaterGeometry();
			
			CRenderer::ConstructRenderList();
			
			#ifdef TIMERS
				tbEndTimer("ConstrRList");
			#endif


			#ifdef TIMERS
				tbStartTimer(0,"Prerender");
			#endif

			CRenderer::PreRender();	// Generate shadows. Set lights etc.
			CWorld::ProcessPedsAfterPreRender();

			#ifdef TIMERS
				tbEndTimer("Prerender");
			#endif

			#ifdef TIMERS
				tbStartTimer(0,"StartOfFrame");
			#endif

			// this call needs to be after the pre renders have been called as that is where shadows are requested for this frame
			g_realTimeShadowMan.Update();

			CMemInfo::ExitBlock();

			//
			// RENDER THE SCENE
			//
			CMirrors::BeforeMainRender();		// Renders the scene to a bugger if needed. (to render the mirror effect)
			
//!PC - old stuff?

			// Run the game code outside the RWStart and RWEnd functions.
			if (CWeather::LightningFlash)
			{
				CTimeCycle::SetFogRed(255);
				CTimeCycle::SetFogGreen(255);
				CTimeCycle::SetFogBlue(255);
				if(!DoRWStuffStartOfFrame_Horizon( 255, 255, 255, 255, 255, 255, 255))	// Flash background white
				{
					CMemInfo::ExitBlock();
					return;
				}
			}
			else
			{
				if (!DoRWStuffStartOfFrame_Horizon( CTimeCycle::GetSkyTopRed(), CTimeCycle::GetSkyTopGreen(), CTimeCycle::GetSkyTopBlue(),
												CTimeCycle::GetSkyBottomRed(), CTimeCycle::GetSkyBottomGreen(), CTimeCycle::GetSkyBottomBlue(), 255))
				{
					CMemInfo::ExitBlock();
					return;
				}
			}

			// Set the rendering pipeline to standard
			DefinedState();
			// Set some fogging
	        RwCameraSetFarClipPlane(Scene.camera, CTimeCycle::GetFarClip());
			RwCameraSetFogDistance(Scene.camera, CTimeCycle::GetFogStart());

			#ifdef TIMERS
				tbEndTimer("StartOfFrame");
			#endif

			#ifdef TIMERS
				tbStartTimer(0,"RenderMirrorBuffer");
			#endif
			CMirrors::RenderMirrorBuffer();
			#ifdef TIMERS
				tbEndTimer("RenderMirrorBuffer");
			#endif
				
#if defined(ES2EMU)
			{
				extern bool isDebugHitTesting;
				extern int debugHitX, debugHitY, debugHitNum;

				if (LIB_PointerGetButton(0, 1) == OSPS_ButtonPressed) { // && LIB_KeyboardIsDown(KK_LEFTALT)) {
					debugHitNum = 0;
					LIB_PointerGetCoordinates(0, &debugHitX, &debugHitY);
					isDebugHitTesting = true;
				} 
			}
#endif

			RenderScene();
	
			#ifdef TIMERS
				tbStartTimer(0,"RenderDebugShit");
			#endif
			RenderDebugShit();			
			#ifdef TIMERS
				tbEndTimer("RenderDebugShit");
			#endif
					

//!PC - empty the list of weapon clumps to be renderer for pc muzzle flash fix. Need to do this here because
// the player is drawn in rendereffects() if in car and in first person mode!
#if (defined(GTA_PC) || defined(GTA_XBOX))
			CVisibilityPlugins::RenderWeaponPedsForPC();
			CVisibilityPlugins::ResetWeaponPedsForPC();
#endif //GTA_PC
			
			RenderEffects();

			#ifdef TIMERS
				tbStartTimer(0,"Render2dStuff");
			#endif
			
			if ((TheCamera.GetBlurType()==MB_BLUR_NONE) || (TheCamera.GetBlurType()==MB_BLUR_LIGHT_SCENE)) 
			{
				if (TheCamera.m_ScreenReductionPercentage>0)
				{
					TheCamera.SetMotionBlurAlpha(150);
				}
			}

	        TheCamera.RenderMotionBlur();

			Render2dStuff();
			
			#ifdef TIMERS
				tbEndTimer("Render2dStuff");
			#endif

		}
		else
		{
			CDraw::CalculateAspectRatio();
			float ViewWindow = CMaths::Tan( (CDraw::GetFOV() * (PI / 360.0f) ) );
			CameraSize(Scene.camera, (RwRect *)0, (RwReal)ViewWindow, CDraw::GetAspectRatio()); // last 2 args: VIEWWINDOW, ASPECTRATIO.
			CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
			RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ/*|rwCAMERACLEARIMAGE*/);
			if (!RsCameraBeginUpdate(Scene.camera))
			{
				CMemInfo::ExitBlock();
				return;
			}		
		}		
		
#if defined(ES2EMU)
		{
			extern bool isDebugHitTesting;
			isDebugHitTesting = false;
		}
#endif

		//!PC - copy and paste in from ps2 render loop
#ifndef MASTER
#ifndef FINAL
		if (!gbDrawPolyDensity)
#endif //FINAL
		{
			if (gDisplayDebugInfo) VarConsole.RenderDebugOutput();
		}

		VarConsole.debug_count = 0;  // this MUST be called to prevent the list from overflowing
#endif //MASTER
		
		#ifdef TIMERS
			tbStartTimer(0,"RenderMenus");
		#endif
		
		RenderMenus();

		#ifdef TIMERS
			tbEndTimer("RenderMenus");
		#endif
		
		#ifdef TIMERS
			tbStartTimer(0,"Fade");
		#endif

		//!PC - ensure that fade out polys don't have a texture on them
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
		DoFade();
		
		#ifdef TIMERS
			tbEndTimer("Fade");
		#endif
		
		#ifdef TIMERS
			tbStartTimer(0,"Render2dStuffAft");
		#endif
		Render2dStuffAfterFade();
		CCredits::Render();
		
		#ifdef TIMERS
			tbEndTimer("Render2dStuffAft");
		#endif

		
		#ifdef TIMERS
			if (CHud::m_Wants_To_Draw_Hud==TRUE)
			{
				tbDisplay();
				CFont::DrawFonts();
			}
		#endif//TIMERS
		
			
		DoRWStuffEndOfFrame();
#ifdef USE_TEXTURE_CAMERA
		textureCamera->RenderEnd();
#endif//USE_TEXTURE_CAMERA

		CMemInfo::ExitBlock();

#ifdef GTA_PS2
		if (CCheat::m_bSlowMode)
			ProcessSlowMode();
#endif			
	}



}// end of Idle()...

//
// name:		FrontendIdle
// description:	Idle function for frontend menus
void FrontendIdle()
{
	CDraw::CalculateAspectRatio();
	CTimer::Update();
	CSprite2d::SetRecipNearClip ( );
	CSprite2d::InitPerFrame();
	CFont::InitPerFrame();
	CPad::UpdatePads();

#ifdef GTA_PS2
	CMemInfo::EnterBlock(FRONTEND_MEM_ID);
#endif
	if (FrontEndMenuManager.m_bStartUpFrontEndRequested)
		FrontEndMenuManager.m_MenuActive = TRUE;
	FrontEndMenuManager.Process(); 
#ifdef GTA_PS2
	CMemInfo::ExitBlock();
#endif	
	if(RsGlobal.quit)
		return;
		
	AudioEngine.Service();
	
	float ViewWindow = CMaths::Tan( (CDraw::GetFOV() * (PI / 360.0f) ) );
	CameraSize(Scene.camera, (RwRect *)0, (RwReal)ViewWindow, CDraw::GetAspectRatio()); // last 2 args: VIEWWINDOW, ASPECTRATIO.
	CVisibilityPlugins::SetRenderWareCamera(Scene.camera);
	RwCameraClear(Scene.camera, &gColourTop, rwCAMERACLEARZ/*|rwCAMERACLEARIMAGE*/);
	if (!RsCameraBeginUpdate(Scene.camera))
	{
		return;
	}		
	
	DefinedState();

	RenderMenus();

#if defined (GTA_PC)
	SecuromStateDisplay();
#endif //GTA_PC

	DoFade();
	Render2dStuffAfterFade();

	CFont::DrawFonts();

	DoRWStuffEndOfFrame();

}// end of FrontendIdle


//
//
//
//
void InitialiseGame(void)
{
	// temporary array with the names of the multiplayer maps
	static char* MultiPlayerMapDatFiles[] = {
		"DATA\\MAPS\\MAP0.DAT",
		"DATA\\MAPS\\MAP1.DAT",
		"DATA\\MAPS\\MAP2.DAT",
		"DATA\\MAPS\\MAP3.DAT",
		"DATA\\MAPS\\MAP4.DAT",
		"DATA\\MAPS\\MAP5.DAT",
		"DATA\\MAPS\\MAP6.DAT",
		"DATA\\MAPS\\MAP7.DAT",
	};

#ifdef GTA_PC
#ifndef MASTER
	version_number = GetVersionNumberFromFile();
#else
	version_number = 78;
#endif //MASTER
#endif	//GTA_PC

#ifdef CDROM	
	if(CLocalisation::FrenchGame() || CLocalisation::GermanGame())
		LoadingScreen(NULL, version_name, "loadsc24");
	else	
		LoadingScreen(NULL, version_name, "loadsc0");
#else
//	LoadingScreen(NULL, NULL, "loadsc0");
#endif//CDROM	

#ifdef GTA_NETWORK
	if(FrontEndMenuManager.m_WantsToRestartGameAsServer ||
		FrontEndMenuManager.m_WantsToRestartGameAsClient)
		{
			CGame::Initialise(MultiPlayerMapDatFiles[CNetGames::GameMap]);
		}
		else
#endif
		{
			CGame::Initialise("DATA\\GTA.DAT");
		}

		AudioEngine.Restart();		//flag to audio that game is starting
		//!SHAUN
#if defined(GTA_XBOX)
		{
			g_saveHelper.PopulateSlotInfo();
			int32 nNewestSavedGame = g_saveHelper.GetNewestSavedGame();
			if(-1 != nNewestSavedGame)
			{
				FrontEndMenuManager.m_WantsToRestartGame=true;
				FrontEndMenuManager.WantsToLoad=TRUE;
				CGame::currLevel =  (eLevelName)nNewestSavedGame;
			}
		}
#elif defined (GTA_PC)
		{
			// we also need to check for a savegame on PC too...
			if (FrontEndMenuManager.WantsToLoad)
			{
				FrontEndMenuManager.m_WantsToRestartGame=true;
			}
		}
#endif

}// end of InitialiseGame()...


#endif//GTA_PC & GTA_XBOX



//
//
// AppEventHandler for All platforms:
//
RsEventStatus AppEventHandler(RsEvent event, void *param)
{

    switch(event)
    {
        case(rsINITIALIZE):
        {
        	CGame::InitialiseOnceBeforeRW();
        	if(!RsInitialize())
        		return(rsEVENTERROR);
            return(rsEVENTPROCESSED);
      	}         
      	break;
      	
        case(rsCAMERASIZE):
        {
			float ViewWindow = CMaths::Tan( (CDraw::GetFOV() * (PI / 360.0f) ) );
            CameraSize(Scene.camera, (RwRect *)param, (RwReal)ViewWindow, DEFAULT_ASPECTRATIO);
            return(rsEVENTPROCESSED);
        }
        break;
	
	    case(rsRWINITIALIZE):
        {
        	RwBool rt;
        	rt = Initialise3D(param);
        	if(rt)
        		return(rsEVENTPROCESSED);
        	else
	            return(rsEVENTERROR);
        }
        break;

        case(rsRWTERMINATE):
        {
            Terminate3D(param);
            return(rsEVENTPROCESSED);
        }
        break;

        case(rsTERMINATE):
        {
            CGame::FinalShutdown();
            return(rsEVENTPROCESSED);
        }
        break;

        case(rsPLUGINATTACH):
        {
            return(PluginAttach()?(rsEVENTPROCESSED):(rsEVENTERROR));
        }
        break;

#if defined (GTA_PC)
        case(rsINPUTDEVICEATTACH):
        {
            AttachInputDevices();
            return(rsEVENTPROCESSED);
        }
        break;
		case(rsACTIVATE):
		{
#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
			if(param)
				DMAudio.ReacquireDigitalHandle();
			else
				DMAudio.ReleaseDigitalHandle();
#endif //USE_AUDIOENGINE
			return(rsEVENTPROCESSED);
		}
		break;
#endif//GTA_PC

#if defined (GTA_PC) || defined (GTA_XBOX)
        case(rsIDLE):
        {
            Idle(param);
            return(rsEVENTPROCESSED);
        }
        break;

        case(rsFRONTENDIDLE):
        	FrontendIdle();
        	return(rsEVENTPROCESSED);
        break;
#endif //GTA_PC & GTA_XBOX

        default:
        {
            return(rsEVENTNOTPROCESSED);
        }
        break;
        
    }//switch(event)...

}// end of AppEventHandler()...



/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ProcessKeyboardDebug
// PURPOSE  : some keyboard and debug stuff gets checked here in debug modes
// RETURNS  : NONE
/////////////////////////////////////////////////////////////////////////////////
#ifndef MASTER
void ProcessKeyboardDebug()
{
	VarConsole.Display();

	//
	// toggle between keyboard modes:
	//
	static int32 show_keyboard_mode = 0;
	
// of course, in controller mode the keypresses aren't passed to the debug keyboard...
#if defined (GTA_PC)
	// trigger on tab 'up' - otherwise will skip debug key mode if test on 'down'
	if (( PS2Keyboard.CurrentKeyboardMode == KEYBOARD_MODE_CONTROLLER) &&
		
#ifdef OSW
			(LIB_KeyboardState(KK_TAB) == OSPS_ButtonReleased) &&
#else
			((CPad::NewKeyState.kTAB == 0) && (CPad::OldKeyState.kTAB != 0)) &&
#endif
			(show_keyboard_mode == 0))
	{
		show_keyboard_mode = CTimer::GetTimeInMilliseconds();
		PS2Keyboard.CurrentKeyboardMode++;
		
		if (PS2Keyboard.CurrentKeyboardMode == KEYBOARD_MODE_OVERRIDE) PS2Keyboard.CurrentKeyboardMode = KEYBOARD_MODE_STANDARD;
	} else 
	
#endif //GTA_PC

	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_TAB, KEYBOARD_MODE_OVERRIDE, "Switch between keyboard modes") && PS2Keyboard.CurrentKeyboardMode < KEYBOARD_MODE_OVERRIDE)
	{
		show_keyboard_mode = CTimer::GetTimeInMilliseconds();
		
		PS2Keyboard.CurrentKeyboardMode++;

		if (PS2Keyboard.CurrentKeyboardMode == KEYBOARD_MODE_OVERRIDE) PS2Keyboard.CurrentKeyboardMode = KEYBOARD_MODE_STANDARD;

// nastiness to stop state carrying over
#ifndef MASTER
#if defined (GTA_PC)		
		if (PS2Keyboard.CurrentKeyboardMode == KEYBOARD_MODE_CONTROLLER)
		{
			PS2Keyboard.ClearState();
		}
#endif
#endif // GTA_PC
	}
	
	if (show_keyboard_mode)
	{
		switch (PS2Keyboard.CurrentKeyboardMode)
		{
#if defined (GTA_PC)
			case KEYBOARD_MODE_CONTROLLER:
				sprintf(gString, "KEYBOARD MODE: CONTROL DEVICE");
				break;
#endif //GTA_PC
			case KEYBOARD_MODE_LEVELS:
				sprintf(gString, "KEYBOARD MODE: LEVEL DESIGN");
				break;
			case KEYBOARD_MODE_STANDARD:
			default:
				sprintf(gString, "KEYBOARD MODE: STANDARD");
				break;
		}				
		
		AsciiToGxtChar (gString,gGxtString);

		CFont::SetBackground(TRUE);
		CFont::SetBackgroundColor(CRGBA(0,0,0,200));
		CFont::SetWrapx(246 * HUD_MULT_X);
		CFont::SetScale( 0.7f * HUD_MULT_X, 0.8f * HUD_MULT_Y);
		CFont::SetColor(CRGBA(255,255,255,255));
		CFont::SetOrientation(FO_LEFT);
		CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);

		CFont::PrintString( 20 * HUD_MULT_X, 40 * HUD_MULT_Y, gGxtString);
	
		if (CTimer::GetTimeInMilliseconds() - show_keyboard_mode > 4000)  // 4 secs
		{
			show_keyboard_mode = 0;
		}
	}
	
//!PC - can't create dongle on PC!
#if defined (GTA_PS2)
	// create dongle:
	if ((PS2Keyboard.GetKeyDown(PS2_KEY_CTRL) && PS2Keyboard.GetKeyJustDown(PS2_KEY_BSPACE)) ||
		(CPad::GetPad(0)->GetShockButtonL() && CPad::GetPad(0)->GetShockButtonR() && CPad::GetPad(1)->GetShockButtonL() && CPad::GetPad(1)->GetShockButtonR()))
	{
		CMemoryCard::CreateMemoryCardDongleCallback();
		// flash the screen white:
		CSprite2d::DrawRect( CRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(100, 100, 100, 255));
	}
#endif //GTA_PS2
	
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_F5, KEYBOARD_MODE_STANDARD, "display rendered objects")) displayNumObjsRendered = !displayNumObjsRendered;

	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_NUMPAD_0, KEYBOARD_MODE_STANDARD, "toggle HUD on and off") || (CPad::GetPad(1)->StartJustDown() && !CGameLogic::IsCoopGameGoingOn() && !CGameLogic::Disable2ndControllerForDebug())) CHud::m_Wants_To_Draw_Hud = !CHud::m_Wants_To_Draw_Hud;  // toggles HUD on/off

//  Les want's this cheat removed
//	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_I, KEYBOARD_MODE_STANDARD, "switch on off health loss"))  // switch off health loss
//	{
//		CPlayerPed::bDebugPlayerInvincible = !CPlayerPed::bDebugPlayerInvincible;
//	}

#ifndef FINAL

	//
	// using the fx system with the keyboard:
	//
	// FX SYSTEM
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_0, KEYBOARD_MODE_STANDARD, "switch fx system on off"))  // switch fx system on/off
	{
		g_fx.CreateDebugFxSystem();
	}
	
	if (g_fx.ms_fxSysDebugMenu)  // only if fx system is in use
	{
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_LEFT)) if (g_fx.ms_currDebugFxSystem > 0) g_fx.ms_currDebugFxSystem--;
		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_RIGHT)) if (g_fx.ms_currDebugFxSystem < g_fxMan.GetNumFxSystemBPs()-1) g_fx.ms_currDebugFxSystem++;
	}
		
	// switch on/off polygon density stuff:
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_F12, KEYBOARD_MODE_STANDARD, "switch poly density on off"))
	{
		gbDrawPolyDensity = !gbDrawPolyDensity;

		if(gbDrawPolyDensity) CMapStats::Init();
	}

	if(gbDrawPolyDensity) 
	{
		VarConsole.Exit();
		CPad::GetPad(0)->DisablePlayerControls = true;  // so the polydensity screen control doesnt mess up game controls
		FrontEndMenuManager.DisplayTheTimerBars = FALSE;  // timer bars get switched off when poly denisity viewed
		CMapStats::Process();
		return;
	}
#endif

	// debug info on/off:
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_SEMI_COLON, KEYBOARD_MODE_STANDARD, "debug output on and off")) gDisplayDebugInfo = !gDisplayDebugInfo;
	
	// make time stand still if in the marketing version:
	// Then CROSS/SQUARE moves the map around after pressing CIRCLE - all on PAD2
	if ((PS2Keyboard.GetKeyJustDown(PS2_KEY_ENTER, KEYBOARD_MODE_STANDARD, "debug cam") || CPad::GetPad(1)->ButtonCircleJustDown()) && FrontEndMenuManager.m_MarketingBuild)
	{
		FrontEndMenuManager.TimeStandsStill = !FrontEndMenuManager.TimeStandsStill;
	}
	
	// check for the slow motion stuff:
	Bool8 bPreviousSlowMotionSetting = CTimer::bSlowMotionActive;
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_PAUSE, KEYBOARD_MODE_OVERRIDE, "slow motion")) CTimer::bSlowMotionActive = !CTimer::bSlowMotionActive;
	if (CTimer::bSlowMotionActive)  // display slow motion details here:
	{
		sprintf(gString, "Timer Speed: %0.0f%%",CTimer::ms_fSlowMotionScale*100.0f);
		
		VarConsole.AddDebugOutput(gString);	

/*		AsciiToGxtChar (gString,gGxtString);
		
		CFont::SetBackground(TRUE);
		CFont::SetBackgroundColor(CRGBA(0,0,0,200));
		CFont::SetWrapx(246 * HUD_MULT_X);
		CFont::SetScale( 0.7f * HUD_MULT_X, 0.8f * HUD_MULT_Y);
		CFont::SetColor(CRGBA(255,255,255,255));
		CFont::SetOrientation(FO_LEFT);
		CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);

		CFont::PrintString( 20 * HUD_MULT_X, 100 * HUD_MULT_Y, gGxtString);
*/		
#ifndef FINAL
		if (bPreviousSlowMotionSetting == false)
		{	//	the slow motion effect has just been switched on
			CScriptDebugger::pCurrentSingleStepScript = NULL;
			CScriptDebugger::bSingleStep = true;
			CScriptDebugger::bProcessTheNextCommand = false;
		}
#endif
	}
	else
	{
#ifndef FINAL
		if (bPreviousSlowMotionSetting)
		{	//	the slow motion effect has just been switched off
			CScriptDebugger::pCurrentSingleStepScript = NULL;
			CScriptDebugger::bSingleStep = false;
			CScriptDebugger::bProcessTheNextCommand = false;
			CScriptDebugger::bClearCheckedForDeadFlags = false;	//	need to skip the dead checks for one frame in case 
										//	the script debugger was part of the way through a script when slow motion was switched off
		}
#endif
	}

#ifndef FINAL	
	if (TheCamera.display_kbd_debug)
	{
		sprintf(gString, "FOV: %0.2f",TheCamera.kbd_fov_value);
		AsciiToGxtChar (gString,gGxtString);

		CFont::SetBackground(TRUE);
		CFont::SetBackgroundColor(CRGBA(0,0,0,200));
		CFont::SetWrapx(246 * HUD_MULT_X);
		CFont::SetScale( 0.7f * HUD_MULT_X, 0.8f * HUD_MULT_Y);
		CFont::SetColor(CRGBA(255,255,255,255));
		CFont::SetOrientation(FO_LEFT);
		CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);

		CFont::PrintString( 40 * HUD_MULT_X, 40 * HUD_MULT_Y,gGxtString);
	}
	
	if (CPostEffects::display_kbd_debug)
	{
		float multiplier1, multiplier2;
	
		multiplier1 = CPostEffects::m_colour1Multiplier;// * CPostEffects::m_extraMultiplier;
		multiplier2 = CPostEffects::m_colour2Multiplier;// * CPostEffects::m_extraMultiplier;
	
		sprintf(gString, "RGB1 Multi: %0.2f   RGB1: %.0f %.0f %.0f RGB2 Multi: %0.2f   RGB2: %.0f %.0f %.0f   Player Brightness: %f ExtraMultipier %f", 
				CPostEffects::m_colour1Multiplier, MIN((float)CTimeCycle::GetPostFx1Red() * multiplier1, 255), MIN((float)CTimeCycle::GetPostFx1Green() * multiplier1, 255), MIN((float)CTimeCycle::GetPostFx1Blue() * multiplier1, 255),
				CPostEffects::m_colour2Multiplier, MIN((float)CTimeCycle::GetPostFx2Red() * multiplier2, 255), MIN((float)CTimeCycle::GetPostFx2Green() * multiplier2, 255), MIN((float)CTimeCycle::GetPostFx2Blue() * multiplier2, 255),
				FindPlayerPed()->m_lightingFromCollision/*,
				CPostEffects::m_extraMultiplier*/);
		AsciiToGxtChar (gString,gGxtString);

		CFont::SetBackground(TRUE);
		CFont::SetBackgroundColor(CRGBA(0,0,0,200));
		CFont::SetWrapx(270 * HUD_MULT_X);
		CFont::SetScale( 0.7f * HUD_MULT_X, 0.8f * HUD_MULT_Y);
		CFont::SetColor(CRGBA(255,255,255,255));
		CFont::SetOrientation(FO_LEFT);
		CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);

		CFont::PrintString( 40 * HUD_MULT_X, 40 * HUD_MULT_Y,gGxtString);
	}
#endif
	
	// toggle vibration on/off with keyboard:
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_NUMPAD_SLASH, KEYBOARD_MODE_STANDARD, "pad vibration"))
	{
		FrontEndMenuManager.m_PrefsUseVibration = !FrontEndMenuManager.m_PrefsUseVibration;
		CPad::GetPad(0)->StartShake(300, 150);
	}
	
	// save game
	if (PS2Keyboard.GetKeyDown(PS2_KEY_S, KEYBOARD_MODE_STANDARD, "save game"))
	{
//		FrontEndMenuManager.m_JustEnteredSaveZone = true;
	}
}
#endif  // MASTER



#ifndef MASTER

//!PC - disable bugstar contact for PC just now
#if defined (GTA_PS2)
void ReportBugToBugstar()
{

	char* jpeg_buff = new char[MAX_JPEG_SIZE];
	
	unsigned int jpeg_buff_size = MAX_JPEG_SIZE;
	
	char* loaded_overlay = (char*)GetLoadedOverlayName();
	
	bool was_cutscene = false;
	
#ifndef RELEASE
	if (strcmp(GetLoadedOverlayName(),"cutscene.dbg") != 0)
#else
	if (strcmp(GetLoadedOverlayName(),"cutscene.rel") != 0)
#endif
	{
		LOAD_CODE_OVERLAY(jpeg);
		
		
		JPegCompressScreenToBuffer((RwCamera*)TheCamera, &jpeg_buff,&jpeg_buff_size);
		
		
		REMOVE_CODE_OVERLAY();
		
	}
		
	
	CTimer::Suspend();		
	DEBUGLOG("**********************************************************Start Report Bug...\n");
	
	
	// load irx modules
#ifdef LOAD_NETWORK_IRX_ON_BUG

	AudioEngine.SuspendAudioStreaming();

	if(SN_NetworkPS2::LoadIOPModules())
		DEBUGLOG("Loaded network IRX files\n");			
	else
		DEBUGLOG("Error Network not started properly.\n Please ensure the Network card is attached and connected to the network");
		
#endif

	

	if(!SN_NetworkPS2::IsNetworkUp())
	{
	
		DEBUGLOG("Doing Part2...\n");
		
		if(SN_NetworkPS2::InitNetwork())
		{
			DEBUGLOG("Done Part2.\n");
		}
		else
		{
			DEBUGLOG("Error Network not started properly.\n Please ensure the Network card is attached and connected to the network");
			CTimer::Resume();
			return;			
		}
		
	
		if(!SN_NetworkPS2::IsNetworkUp())
		{
			DEBUGLOG("Network FAILED.\n");
			CTimer::Resume();
			return;		
		}
		else
		{
			DEBUGLOG("Done network.\n");
		}
	
		
	}
	CTimer::Resume();
	
	uint8 grid_pos_x = 0;
	uint8 grid_pos_y = 0;
	char grid[10];

	CGridRef::GetGridRefPositions(&grid_pos_x, &grid_pos_y);  // get the current grid position

	sprintf(grid,"[%c,%d]",(grid_pos_x+65),grid_pos_y+1);

	CVector posn = TheCamera.GetPosition();
	
	
	char* owner = CGridRef::GetArtistName(grid_pos_x, grid_pos_y);
	
	if(owner == NULL)
	{
		owner = "";
	}
			
	CreateBug(posn.x,posn.y,posn.z,grid,owner,jpeg_buff,MAX_JPEG_SIZE);

#ifdef LOAD_NETWORK_IRX_ON_BUG

	CTimer::Suspend();
	
	// stop network
	// unload irx modules	
	SN_NetworkPS2::ShutdownNetwork();	
	
	AudioEngine.ResumeAudioStreaming();

	
	CTimer::Resume();
	
#endif

	delete [] jpeg_buff;
	jpeg_buff = NULL;
			
}

#else //GTA_PS2

// create bug to send to bugstar.
void ReportBugToBugstar()
{
	char* jpeg_buff = new char[MAX_JPEG_SIZE];
	unsigned int jpeg_buff_size = MAX_JPEG_SIZE;

	JPegCompressScreenToBuffer((RwCamera*)TheCamera, &jpeg_buff,&jpeg_buff_size);

	uint8 grid_pos_x = 0;
	uint8 grid_pos_y = 0;
	char grid[10];

	CGridRef::GetGridRefPositions(&grid_pos_x, &grid_pos_y);  // get the current grid position

	sprintf(grid,"[%c,%d]",(grid_pos_x+65),grid_pos_y+1);

	CVector posn = TheCamera.GetPosition();
	

	char* owner = CGridRef::GetArtistName(grid_pos_x, grid_pos_y);
	
	if(owner == NULL)
	{
		owner = "";
	}

#ifndef OSW
	if(!CNetworkPC::IsNetworkUp())
	{
		CNetworkPC::InitNetwork();
	}

	CNetworkPC network;
	CBugstarPC bugstar((CBugstarNetwork*)&network);

	bugstar.CreateBug(posn.x,posn.y,posn.z,grid,owner,jpeg_buff,jpeg_buff_size);
	
#endif
}
#endif //GTA_PS2


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : GetVersionNumberFromFile
// PURPOSE  : gets the version number in "VERSION.TXT"
// RETURNS  : this version number
/////////////////////////////////////////////////////////////////////////////////
uint16 GetVersionNumberFromFile()
{
	char title[32];
	
	uint16 this_version_number = 0;
	
	int32 num[8];
	char* pLine;

	int32 fid;
	CFileMgr::SetDir("");
	fid = CFileMgr::OpenFile("VERSION.TXT", "rb");

	ASSERT(fid);
	while ((pLine = CFileLoader::LoadLine(fid)))
	{
		if(*pLine == '#' || *pLine == '\0') continue;
	
		sscanf(pLine, "%s", title);
		
		if (!strcmp(title, "[VERSION_NUMBER]"))
		{
			pLine = CFileLoader::LoadLine(fid);
			
			sscanf(pLine, "%d", &num[0]);
			
			this_version_number = num[0];
			
			continue;
		}
	}
	CFileMgr::CloseFile(fid);
	
	return (this_version_number);
}

#ifndef FINAL
//!PC - no jpeg stuff for PC yet
#if defined (GTA_PS2)
//
// name:		ViewJPegPhoto
// description:	If certain button configuration is pressed we can view our photo album
bool ViewJPegPhotos(RwCamera* pCamera)
{
	// View photo
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_J))
	{
		RwRaster   *gpJpgTex = RwRasterCreate  ( SCREEN_WIDTH, SCREEN_HEIGHT, 32, rwRASTERTYPETEXTURE);

		char filename[16];
		static int32 currentPhoto = 0;

//#define TEST_FROM_PC
#ifndef TEST_FROM_PC		
		LOAD_CODE_OVERLAY(mc);
		TheMemoryCard.CreateGameDirectory(CARD_ONE);
		if(CMemoryCardFileMgr::ChangeDirectory(CARD_ONE, MC_GAME_ROOT_DIRECTORY) < 0)
			return false;
		TheMemoryCard.PopulateFileTable(CARD_ONE, "*.jpg");

		sprintf(filename, "screen%d.jpg", currentPhoto);
		if(TheMemoryCard.CheckFilePresent(CARD_ONE, filename) == false)
		{
			sprintf(filename, "screen%d.jpg", currentPhoto);
//			if(TheMemoryCard.CheckFilePresent(CARD_ONE, filename) == false)
			{
//				return false;
			}
		}
		REMOVE_CODE_OVERLAY();
#else
		sprintf(filename, "screen%d.jpg", currentPhoto);
#endif		
		CTimer::Suspend();

		LOAD_CODE_OVERLAY(jpeg);
		DEBUGLOG1("Viewing %s\n", filename);
#ifndef TEST_FROM_PC		
		JPegDecompressToVramFromMemoryCard(gpJpgTex, NULL, GALLERY_FILENAME, currentPhoto);
#else
		JPegDecompressToVramFromFile(gpJpgTex, filename);
#endif
		REMOVE_CODE_OVERLAY();

		currentPhoto++;
		
		if (currentPhoto > 15) currentPhoto = 0;
		
		while(!PS2Keyboard.GetKeyJustDown(PS2_KEY_ESC))
		{
			DoRWStuffStartOfFrame( 0,0,0, 0,0,0, 255);
			
		    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
		    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
		    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
		    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
		    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
		    
			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);

			RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS,(void*)rwTEXTUREADDRESSBORDER);

			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpJpgTex);
			CSprite::RenderOneXLUSprite(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 1.0f,
										SCREEN_WIDTH/2, SCREEN_HEIGHT/2,
										255, 255, 255, 255, 1.0f/100.0f);
			
		    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)TRUE);
	    
	    
	 	   DoRWStuffEndOfFrame();
	 	   
	 	   CPad::UpdatePads();
	    }
	    
   		CTimer::Resume();
	    return true;
	}

	return false;
}

#else //GTA_PS2
bool ViewJPegPhotos(RwCamera* pCamera)
{
	return FALSE;
}
#endif //GTA_PS2
#endif

#endif