//
// filename:	Init_overlay.cpp
// description:	Initialisation functions grouped into one file so that
//				we can put them in an overlay
// written by:	Adam Fowler
//

#include "FileMgr.h"
#include "FileLoader.h"
#include "Main.h"
#include "Game.h"
#include "General.h"
#include "MemInfo.h"
#include "VarConsole.h"
#include "TxdStore.h"
#include "VisibilityPlugins.h"
#include "RwCam.h"
#include "lights.h"
#include "font.h"
#include "hud.h"
#include "TempColModels.h"
#include "SurfaceTable.h"
#include "TimeCycle.h"
#include "Scriptdebug.h"
#include "Fx.h"
#include "Ragdoll.h"
#include "IkChain.h"
#include "BreakObject.h"
#ifdef GTA_PS2
#include "skyfs.h"
#include "ps2all.h"
#include "vu0collision.h"
#endif
#include "pools.h"
#include "GameLogic.h"
#include "Conversations.h"
#include "QuadTree.h"
#include "CustomCarPlateMgr.h"
#include "CustomCarPlateMgr.h"
#include "CustomRoadSignMgr.h"
#include "Clothes.h"
#include "ZoneCull.h"
#include "Occlusion.h"
#include "EntryExits.h"
#include "stuntjump.h"
#include "SetPieces.h"
#include "Record.h"
#include "Restart.h"
#include "CutsceneMgr.h"
#include "Pickups.h"
#include "Cargen.h"
#include "Pedgen.h"
#include "Garages.h"
#include "Streaming.h"
#include "network_allprojects.h"
#include "waterlevel.h"
#include "streaming.h"
//#include "pedroutes.h"
#include "Renderer.h"
#include "Radar.h"
#include "TouchInterface.h"
#include "Weapon.h"
#include "TrafficLights.h"
#include "Roadblocks.h"
#include "Property.h"
#include "Coronas.h"
#include "Shadows.h"
#include "WeaponEffects.h"
#include "Glass.h"
#include "Phones.h"
#include "script.h"
#include "gangs.h"
#include "clock.h"
#include "cranes.h"
#include "fluff.h"
#include "darkel.h"
#include "stunts.h"
#include "stats.h"
#include "clouds.h"
#include "specialfx.h"
#include "ropes.h"
#include "watercannon.h"
#include "bridge.h"
#include "pedgroup.h"
#include "credits.h"
#include "replay.h"
#include "frontend.h"
#include "InteriorManager.h"
#include "cdstream.h"
#include "birds.h"
#include "Events.h"
#include "cover.h"
#include "gridref.h"
#include "ProcObjects.h"
#include "PostEffects.h"
#include "gangwars.h"
#include "VehicleAnimGroupData.h"
#include "audiozones.h"
#include "streamingdebug.h"
#include "SurfaceTable.h"
#include "shopping.h"
#include "loadingscreen.h"
#include "WaterCreatures.h"
#include "tag.h"
#include "cheat.h"
#include "TaskAttack.h"
#include "inifile.h"

#if defined GTA_XBOX// || defined GTA_PC
#include "ShaderManager.h"
#endif

#ifdef GTA_PC
#ifndef MASTER

	#include "DebugBugVisualiser.h"
	#include "PedDebugVisualiser.h"
	#include "CarCols.h"

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
#endif
#endif

#ifdef USE_AUDIOENGINE
#include "AudioEngine.h"
#include "GameAudioEvents.h"
#else //USE_AUDIOENGINE
#endif //USE_AUDIOENGINE

#if defined (GTA_PC)
#include "texturePool.h"
#include "playerSkin.h"
#include "cHasp.h"
#endif // GTA_PC

#if defined (GTA_XBOX)
#include "texturePool.h"
#include "playerSkin.h"
#endif //GTA_XBOX

#ifdef GTA_PS2
	#define USE_VU_TURBO_SPRITE		(1)
	#define USE_VU_WATER_RENDERER	(1)

	#ifdef GTA_SA
		#define USE_VU_CARFX_RENDERER	(1)
		#define USE_CVSKINPED_RENDERER	(1)
	#endif
#endif

#if (defined(GTA_PC) || defined(GTA_XBOX))
	#define USE_PC_CARFX_RENDERER	(1)
#endif


//!PC - working on pc version of this now...
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

#ifdef USE_PLANT_MGR
	#if defined (GTA_PS2)
		#include "PlantsMgr.h"
		#include "VuGrassRenderer.h"
	#else	//GTA_PS2
		#include "PC_PlantsMgr.h"
		#include "PC_GrassRenderer.h"
	#endif //GTA_PS2
#endif

#ifdef USE_CVBUILDING_RENDERER
	#ifdef GTA_PS2
		#include "VuCustomBuildingRenderer.h"
	#else //GTA_PS2
		#include "CustomBuildingRenderer.h"
	#endif //GTA_PS2
#endif //USE_CVBUILDING_RENDERER

#ifdef USE_CVSKINPED_RENDERER
	#include "VuCustomSkinPedRenderer.h"
#endif

#if defined (MSVC_COMPILER)
static uint8 __declspec(align(64)) loadbuf[2048]; 
#else //MSVC_COMPILER
static uint8 loadbuf[2048] __attribute__((aligned(64))); 
#endif //MSVC_COMPILER


//
//
// from Main.cpp
//
//

extern bool gameAlreadyInitialised;
int VBlankCounter(int arg);

//
//
//
//
RwBool Initialise3D(void *param=NULL)
{
	CMemInfo::EnterBlock(RENDER_MEM_ID);
	
    if (RsRwInitialize(param))
    {


		CMemInfo::ExitBlock();
		
		return CGame::InitialiseRenderWare();
    }
	CMemInfo::ExitBlock();
	
    return (FALSE);
}

#ifdef GTA_PS2
//
// name:		CdStreamReadSync
// description:	Attempt to read from the DVD. If it fails then try again after a VBlank (don't want
//				to flood the IOP
void CdStreamReadSync(uint8* pBuffer, int32 posn, int32 size)
{
    SyncDCache(pBuffer, pBuffer+size*2048);
#ifdef CDROM
	int32 rt = CdStreamRead(4, pBuffer, posn, size);
	while(CdStreamSync(4) != SCECdErNO || rt == FALSE)
	{
		DelayThread(1000);
		rt = CdStreamRead(4, pBuffer, posn, size);
	}
#else	
	sceCdRMode CDmode;	// Mode settings for CD reading
	int result = 0;

	CDmode.trycount = 20;
	CDmode.spindlctrl = SCECdSpinMax;
	CDmode.datapattern = SCECdSecS2048;

	result = sceCdRead(posn ,size,(void *)pBuffer,&CDmode);
	sceCdSync(0);
#endif	
}

//
// name:		makeInt
// description:	Get integer from not necessarily aligned data pointer
int makeInt(int start)
{
    return( loadbuf[start]+(loadbuf[start+1]<<8)+(loadbuf[start+2]<<16)+(loadbuf[start+3]<<24) );
}
//
// name:		RegisterDirDetails
// description:	Register the contents of a directory
void RegisterDirDetails(char* foldername, int32 startSec, int32 numSectors, int32 offset=0)
{
	typedef struct
	{
		char filename[12];
		int32 lsn;
		int32 size;
	} CdDirInfo;

    int32 i;
    int32 j;
    int32 posn;
    int32 size;
    CdDirInfo dirInfo[64];
    char filename[256];
    int32 foldernameLen;
    
    strcpy(filename, foldername);
    foldernameLen = strlen(filename);
    
    filename[foldernameLen++] = '\\';

    int todoptr = 0;

    for(i=0;i<numSectors;i++)
    {
        j=0;
        // Read Sector, for each entry if file output file details, if dir call this
        // function with start sector and number of sectors
        CdStreamReadSync(&loadbuf[0],(startSec+i) + offset, 1);

        //printf("Sector %d :\n\n",(startSec+i));
        //printf("Len Ext    Start  DataLen    Recording Date GMT Flg USz IGp Vol ID\n");
        //printf("===============================================================================\n");

        while(loadbuf[j])
        {
            // Is it current directory [32] == 1
            if((loadbuf[j+32] == 1) && (loadbuf[j+33] == 0))
            {
                j+=loadbuf[j];
				continue;
			}	
			// Is it the parent directory	
            if( (loadbuf[j+32] == 1) && (loadbuf[j+33] == 1))
            {
                j+=loadbuf[j];
				continue;
			}	

        	posn = makeInt(j+2);
        	size = makeInt(j+10);

			// Is it a directory
            if(loadbuf[j+25] & 2)
            {
                // new directory, put in array to process later
                dirInfo[todoptr].lsn = posn;
                dirInfo[todoptr].size = size;
                strncpy(dirInfo[todoptr].filename, (char*)&loadbuf[j+33], loadbuf[j+32]);
                dirInfo[todoptr].filename[loadbuf[j+32]] = '\0';
                todoptr++;
                ASSERT(todoptr < 256);
            }
            // otherwise it is a file
            else
            {
            	strncpy(&filename[foldernameLen], (char*)&loadbuf[j+33], loadbuf[j+32]);
            	filename[foldernameLen + loadbuf[j+32]] = '\0';
				SkyRegisterFileOnCd(filename, posn+offset, size);
            }
            // add the number of bytes in the record onto j
            j+=loadbuf[j];
        }
    }

	//
	// call this function for all the directories found
	// 
	for(i=0; i<todoptr; i++)
	{
		// ignore folders inside audio folder except the config folder
		if(!stricmp(foldername, "\\AUDIO") && stricmp(dirInfo[i].filename, "config"))
			continue;
		// ignore system folder
		if(!stricmp(dirInfo[i].filename, "system"))
			continue;
		sprintf(filename, "%s\\%s", foldername, dirInfo[i].filename);
		RegisterDirDetails(filename, 
							dirInfo[i].lsn, 
							(dirInfo[i].size + 2047) / 2048,
							offset);
	}
}

//
// name:		GetRootDirDetails
// description:	Get details for the root directory
void GetRootDirDetails(int *secNum, int *numSectors)
{
    // read PVD details from sector 16
    CdStreamReadSync(&loadbuf[0], 0x10, 1);
    // Get value of sector position
    *secNum  = makeInt(158);
    *numSectors  = makeInt(166);
    *numSectors = (*numSectors + 2047) / 2048;

    return;
}

//
// name:		GetRootDirDetails
// description:	Get details for the root directory
void GetLayer1RootDirDetails(int *secNum, int *numSectors)
{
    // read PVD details from sector 16
    CdStreamReadSync(&loadbuf[0], 0x10 + CFileMgr::GetDVDLayer1Start(), 1);
    // Get value of sector position
    *secNum  = makeInt(158);
    *numSectors  = makeInt(166);
    *numSectors = (*numSectors + 2047) / 2048;

    return;
}

//
// name:		RegisterFilesOnCd
// description:	Register all the files on the cd
void RegisterFilesOnCd()
{
	int32 posn, size;
	GetRootDirDetails(&posn, &size);
	RegisterDirDetails("", posn, size);
	
	if(CFileMgr::IsDVDDualLayer())
	{
		GetLayer1RootDirDetails(&posn, &size);
		RegisterDirDetails("", posn, size, CFileMgr::GetDVDLayer1Start());
	}
}

#endif
//
// name:		GameInit
// description:	Do initialisation for the game
void GameInit()
{
#if defined(CDROM) && defined(GTA_PS2)
	RegisterFilesOnCd();
#endif	
	Initialise3D();
	
#ifdef GTA_PS2
	// use clock values as seed for random values
	sceCdCLOCK clock;
	sceCdReadClock(&clock);
	CGeneral::SetRandomSeed(clock.second + (clock.minute*60 + clock.day*60)*60);
#else
	CGeneral::SetRandomSeed(RsTimer());	
#endif

#ifdef GTA_PS2
	// setup VBlank interrupt
	AddIntcHandler(INTC_VBLANK_S, &VBlankCounter, 0);
#endif	
	
}


//
//
// from Game.cpp
//
//
extern int32 gameTxdSlot;

//
// Check this is a valid version
//
void ValidateVersion()
{
#ifdef GTA_PC
  #define CDROM
#endif //#ifdef GTA_PC

#ifdef CDROM
	char buffer[128];
	const char* pGta3 = "grandtheftauto3";
#ifdef GTA_PC
	int32 id = CFileMgr::OpenFile("models\\coll\\peds.col", "rb");
#else	
	int32 id = CFileMgr::OpenFile("models\\coll\\peds.col");
#endif // #ifdef GTA_PC
	int32 i=0;
	
	
	if(id != -1)
	{
		CFileMgr::Seek(id, 100, SEEK_SET);
		while(i<128)
		{
			CFileMgr::Read(id, &buffer[i], 1);
			buffer[i] -= 23;
			if(buffer[i] == '\0')
				break;
			i++;	
			CFileMgr::Seek(id, 99, SEEK_CUR);
		}
		
		if(!strncmp(buffer, "grandtheftauto3", 15))
		{
			strncpy(&version_name[0], buffer+15, 64);
			CFileMgr::CloseFile(id);
			return;
		}
	}
	LoadingScreen("Invalid version", NULL);
	while(1) {}
#endif
#ifdef GTA_PC
  #undef CDROM
#endif //#ifdef GTA_PC

}


// put anything that needs done when D3D is restored in here
#if defined(GTA_PC) && !defined(OSW)
rwD3D9DeviceRestoreCallBack g_RwD3DRestoreCB = NULL;

void MyD3DRestoreCB(void)
{	
	// do RW stuff
	if (g_RwD3DRestoreCB)
	{
		g_RwD3DRestoreCB();
	}

	// tell the real time shadows to recreate their camera texture rasters
	if (g_realTimeShadowMan.m_initialised)
	{
		g_realTimeShadowMan.m_d3dRestored = true;
	}

	// tell the mirrors to recreate the camera rasters.
	CMirrors::d3dRestored = true;
}
#endif

//
// name:		InitialiseRenderWare
// description:	Initialise graphics. Load default texture dictionaries/ make camera and world
bool CGame::InitialiseRenderWare()
{
    RwBBox  bbox;

	ValidateVersion();
	
#if defined(GTA_PC) && !defined(OSW)
	// intialise texture/palette pools
	D3DPoolsInit();
#endif//GTA_PC

	CTxdStore::Initialise();
	CVisibilityPlugins::Initialise();

#ifdef GTA_PS2
	// select tristrip clipper
	RpSkySelectTrueTSClipper(TRUE);
	RpSkySelectTrueTLClipper(TRUE);

	// Setup PS2all
	CMemInfo::EnterBlock(PIPELINES_MEM_ID);
	PS2AllOptimizedPipelinesCreate();
	CMemInfo::ExitBlock();
	
	// preallocate RW objects that use freelists
#ifdef PREALLOC_FREELISTS
//	PreAllocateRwObjects();
#endif
#endif		// GTA_PS2
	
	//
    // Create camera
    //
    Scene.camera = CameraCreate(SCREEN_WIDTH, SCREEN_HEIGHT, TRUE);
    if (!Scene.camera)
        return false;

#ifdef NEW_CAMERA // migrated the initialisation of the pools so the camera can use them
	#include "CameraMain.h"
	CPools::Initialise();
	CameraManagersInit();
#endif

	TheCamera.Init();
	TheCamera.SetRwCamera(Scene.camera);

    RwCameraSetFarClipPlane(Scene.camera, (RwReal)(2000.0f));	// Far plane is also set by the game every frame
    RwCameraSetNearClipPlane(Scene.camera, (RwReal)(0.9f));

	// force a camera size call
    CameraSize(Scene.camera, (RwRect *)0, (RwReal)(0.7f), (RwReal)(4.0f/3.0f)); // last 2 args: VIEWWINDOW, ASPECTRATIO.

    // Create a world
    bbox.sup.x = bbox.sup.y = bbox.sup.z = (RwReal)(10000.0f);
    bbox.inf.x = bbox.inf.y = bbox.inf.z = (RwReal)(-10000.0f);

    Scene.world = RpWorldCreate(&bbox);
	if (!Scene.world)
    {
        CameraDestroy(Scene.camera);
        Scene.camera = (RwCamera *) NULL;
        return (FALSE);
    }


    RpWorldAddCamera(Scene.world, Scene.camera);

    LightsCreate(Scene.world);

	// debug font
	CreateDebugFont();

	// Load standard fonts
	CMemInfo::EnterBlock(TEXTURES_MEM_ID);
	CFont::Initialise();
	CHud::Initialise ();

#if defined (GTA_PC) || defined (GTA_XBOX)
	CPlayerSkin::Initialise();
#endif	//GTA_PC & GTA_XBOX

	CMemInfo::ExitBlock();	
	
	CPostEffects::Initialise();

	m_pWorkingMatrix1 = RwMatrixCreate();
	ASSERT(m_pWorkingMatrix1);

	m_pWorkingMatrix2 = RwMatrixCreate();
	ASSERT(m_pWorkingMatrix2);

#if defined(GTA_PC) && !defined(OSW)
	// get the RW D3D restore call back and set to a user version
	g_RwD3DRestoreCB = _rwD3D9DeviceGetRestoreCallback();
	_rwD3D9DeviceSetRestoreCallback(MyD3DRestoreCB);
#endif

	return true;
}

bool DoHaspChecks()
{
#ifdef GTA3_HASP
	// check for presence of HASP4 dongle:
	if(!gHaspDevice.IsConnected())
	{
		// Hasp is not connected or something went wrong!
		DEBUGLOG("HASP4 connected...failed!\n");
		return(FALSE);		
	}
	DEBUGLOG("HASP4 connected...ok!\n");

	// check for the right type of the dongle:
	if(!gHaspDevice.IsHASP4TimeAttached())
	{
		DEBUGLOG("HASP4 Time is not attached...failed!\n");
		return(FALSE);
	}
	DEBUGLOG("HASP4Time is attached...ok!\n");
	
	// store ID serial number of HASP device currently attached:
	if(!gHaspDevice.GetHaspID(&gHaspIDNumber))
	{
		DEBUGLOG("HASP4 obtaining ID number...failed!\n");
	}
	DEBUGLOG1("HASP4 obtaining ID number (0x%X)...ok!\n", gHaspIDNumber);
	
	
	//
	// decode the password: "GTA3 PC beta 1.0DMA Design Ltd. Scotland, UK    "+00;
	//
	char pass[] =	{	0x07, 0x80, 0xc6, 0x76, 0x3f, 0x16, 0x41, 0xf0, 
						0x1f, 0x18, 0x1e, 0xaa, 0xdd, 0x7b, 0x12, 0x65, 
						0x48, 0xbb, 0x68, 0x08, 0xe8, 0xdf, 0x3a, 0x14, 
						0x98, 0x20, 0xdb, 0x35, 0x84, 0x28, 0x49, 0x9a, 
						0x31, 0x6b, 0x1f, 0x92, 0xa6, 0xa0, 0x13, 0xa5, 
						0x49, 0x67, 0x6b, 0x51, 0x19, 0x5a, 0x81, 0xaa, 
						0xec, 0x9f	};
	if(!gHaspDevice.DecodeData(pass, sizeof(pass)))
	{
		DEBUGLOG("HASP4 error decoding data...failed!\n");
		return(FALSE);
	}

	//
	// check presence of right password:
	//
	const int passLen = ::strlen(pass);
	if(!gHaspDevice.CheckPassword(pass, passLen))
	{
		DEBUGLOG("HASP4 password check...failed!\n");
		return(FALSE);
	}
	DEBUGLOG("HASP4 password check...ok!\n");


	// check for the current date in HASP4:
	int Day=0, Month=0, Year=0;
	if(!gHaspDevice.GetHaspDate(&Day, &Month, &Year))
	{
		DEBUGLOG("HASP4 getting date...failed!");
		return(FALSE);
	}
	// do not permit to run game after December 2002:	
	if(Year > GTA3_HASP_MAX_DATE_YEAR)
	{
		DEBUGLOG("HASP4 date check...failed!\n");
		return(FALSE);
	}
	DEBUGLOG("HASP4 date check...ok!\n");
	

	
	#ifdef GTA3_HASP_DATELIMIT
		//
		// do not allow to run game after 5th April 2002:
		//
		if(!gHaspDevice.IsHaspDateBeforeThatDate(GTA3_HASP_MAX_DATE_DAY, GTA3_HASP_MAX_DATE_MONTH, GTA3_HASP_MAX_DATE_YEAR))
		{
			DEBUGLOG3("HASP4: You tried to run game after given date (%d.%d.20%d)!\n", GTA3_HASP_MAX_DATE_DAY, GTA3_HASP_MAX_DATE_MONTH, GTA3_HASP_MAX_DATE_YEAR);
			return(FALSE);
		}
		DEBUGLOG("HASP4 date limit check...ok!\n");
	#endif//GTA3_HASP_DATELIMIT...

	
	gHaspGameStartedInMinutes=0;
	if(!gHaspDevice.GetHaspTimeInMinutes(&gHaspGameStartedInMinutes))
	{
		DEBUGLOG("HASP4 time check...failed!\n");
		return(FALSE);
	}	
	DEBUGLOG("HASP4 time check...ok!\n");

#endif//GTA3_HASP...

	return TRUE;
}

//
// name:		InitialiseEssentialsAfterRW
// description:	Initialise game related structures needed for warning boxes after RW has been 
bool CGame::InitialiseEssentialsAfterRW()
{
	TheText.Load();
	
#ifdef GTA3_HASP
	if(DoHaspChecks() == FALSE)
		return FALSE;
#endif
	
	CMemInfo::EnterBlock(PIPELINES_MEM_ID);

#ifdef USE_VU_TURBO_SPRITE
	if(!CVuTurboSprite::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CVuTurboSprite!");
		return(FALSE);
	}
#endif

#ifdef USE_VU_WATER_RENDERER
	if(!CVuWaterRenderer::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CVuWaterRenderer!");
		return(FALSE);
	}
#endif


#ifdef USE_VU_CARFX_RENDERER
	if(!CVuCarFXRenderer::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CVuCarFXRenderer!");
		return(FALSE);
	}
#endif

#ifdef USE_PC_CARFX_RENDERER
	if(!CCarFXRenderer::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CCarFXRenderer!");
		return(FALSE);
	}
#endif

#ifdef USE_PLANT_MGR
	if(!CGrassRenderer::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CVuGrassRenderer!");
		return(FALSE);
	}
#endif

#ifdef USE_CVBUILDING_RENDERER
	#if defined(GTA_PS2)
		if(!CVuCustomBuildingRenderer::Initialise())
		{
			ASSERTMSG(FALSE, "Error initialising CVuCustomBuildingRenderer!");
			return(FALSE);
		}
	#endif
	#if (defined(GTA_PC) || defined(GTA_XBOX))
		if(!CCustomBuildingRenderer::Initialise())
		{
			ASSERTMSG(FALSE, "Error initialising CCustomBuildingRenderer!");
			return(FALSE);
		}
	#endif
#endif


#ifdef USE_CVSKINPED_RENDERER
	if(!CVuCustomSkinPedRenderer::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CVuCustomSkinPedRenderer!");
		return(FALSE);
	}
#endif

	CMemInfo::ExitBlock();

	CTimer::Initialise();

#ifdef USE_SHADERS
	g_shaderMan.Init();
#endif

    // The touch controls need to be loaded early.
	CTouchInterface::LoadTouchControls();CTouchInterface::LoadTouchControls();


	return TRUE;
}

//
// name:		InitialiseCoreDataAfterRW
// description:	Initialise game related structures after RW has been 
bool CGame::InitialiseCoreDataAfterRW()
{
	CTempColModels::Initialise();

	// load data files
	mod_HandlingManager.Initialise();
//	CSurfaceTable::Initialise("DATA\\SURFACE.DAT");
	g_surfaceInfos.Init();
	CPedStats::Initialise();
#ifdef PAL_BUILD
	CTimeCycle::Initialise(true);
#else
	CTimeCycle::Initialise(false);
#endif
	CPopCycle::Initialise();
	CVehicleRecording::InitAtStartOfGame();

#ifndef FINAL
	CScriptDebugger::Initialise();
#endif

#ifdef GTA_PS2
	LoadingScreen("Loading the Game", "Initialising audio");
#endif
	IOPMEM_DEBUG("Before Audio")

	CMemInfo::EnterBlock(AUDIO_MEM_ID);

	if(!AudioEngine.Initialise())
		return FALSE;

	CMemInfo::ExitBlock();
	IOPMEM_DEBUG("After Audio")


	// set the loaded skin:
	//CWorld::Players[0].SetPlayerSkin(CMenuManager::m_PrefsSkinFile);

#ifdef GTA_PC
#ifndef MASTER 

	CDebugBugVisualiserMenu::Initialise();

#ifndef FINAL	
	VarConsole.Add("Reload font proportional values",&CFont::LoadFontValues, VC_RENDER);
	VarConsole.Add("Draw poly density",&gbDrawPolyDensity, true, VC_RENDER);
	VarConsole.Add("Debug Targetting",&CPlayerPed::bDebugTargetting, true, VC_TARGETTING);
//	VarConsole.Add("Car Colours Editor", &bCarColsEditor, true, VC_VEHICLES);	
	CPedDebugVisualiserMenu::Initialise();
#endif
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
#endif

#ifdef GTA_XBOX
#ifndef MASTER 
	VarConsole.AddCommand("put player", &CPlayerPed::PutPlayerAtCoords);  // add "put player" command
#endif
#endif
	// MN - BREAKABLE OBJECTS -----------------------
	
//	LoadingScreen("Loading the Game", "Breakable Objects");

	CMemInfo::EnterBlock(BREAKABLE_OBJECTS);	
	if(!g_breakMan.Init())
		return FALSE;
	CMemInfo::ExitBlock();	

	// ----------------------------------------------
	
	// MN - IK & RAGDOLL PHYSICS --------------------
	if(!g_boneNodeMan.Init())
		return FALSE;
	if(g_ikChainMan.Init())
		return FALSE;
//	g_ragdollMan.Init();
	// ----------------------------------------------
	
	return(TRUE);

}// end of CGame::InitialiseOnceAfterRW()...


//
// Name			:	Init1
// Purpose		:	First set of game initialisation code
bool CGame::Init1(const char *pDatFile)
{
	CMaths::InitMathsTables();
	
	// store dat file name
	strcpy(CGame::aDatFile, pDatFile);
			
#ifdef GTA_PS2
#ifdef DO_MEM_CHECKSUM_TEST
	CCodeChecksum::CalcChecksumsAtStart();
#endif
	ColStartLoadMicrocode();
#endif	

#ifndef NEW_CAMERA // migrated the initialisation of the pools so the camera can use them
	CPools::Initialise();
#endif

	CPlaceable::InitMatrixArray();
	
#ifdef GTA_PC
	CIniFile::LoadIniFile();

#ifndef OSW
	D3DPoolsEnable(false);
#endif
#endif
	
	
	// set current level and area
	currLevel = LEVEL_GENERIC;
	currArea = AREA_MAIN_MAP;
	
	// Load all texture dictionaries
	CMemInfo::EnterBlock(TEXTURES_MEM_ID);

//	LoadingScreen("Loading the Game", "Loading generic textures");
	gameTxdSlot = CTxdStore::AddTxdSlot("generic");
	CTxdStore::Create(gameTxdSlot);
	CTxdStore::AddRef(gameTxdSlot);

//#ifndef STOP_OLD_FX // MN - OLD FX SYSTEM (Particle TXD)	
	int32 txdSlot;
//	LoadingScreen("Loading the Game", "Loading particles");
	txdSlot = CTxdStore::AddTxdSlot("particle", true);
	CTxdStore::LoadTxd(txdSlot, "MODELS\\PARTICLE.TXD");
	CTxdStore::AddRef(txdSlot);
//#endif // MN - OLD FX SYSTEM (Particle TXD)	

	CTxdStore::SetCurrentTxd(gameTxdSlot);
//	LoadingScreen("Loading the Game", "Setup game variables");

	CMemInfo::ExitBlock();

#ifdef GTA_PS2
	ColFinishLoadMicrocode();
#endif
	CGameLogic::InitAtStartOfGame();
	CGangWars::InitAtStartOfGame();
	CConversations::Clear();
	CPedToPlayerConversations::Clear();

	CQuadTreeNode::InitPool();

#ifdef USE_PLANT_MGR
	if(!CPlantMgr::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CPlantMgr!");
		return(FALSE);
	}
#endif

//!PC - not custom road signs for PC yet
//#if defined (GTA_PS2)
	if(!CCustomRoadsignMgr::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CCustomRoadsignMgr!");
		return(FALSE);
	}
//#endif //GTA_PS2


	CReferences::Init();

	CDebug::DebugInitTextBuffer();

//	LoadingScreen(NULL, NULL);
	
//	CScriptPaths::Init();
	CTagManager::Init();
	CWeather::Init();
	CCover::Init();
	CCullZones::Init();
	COcclusion::Init();
	CCollision::Init();
	CBirds::Init();
	CEntryExitManager::Init();
	CStuntJumpManager::Init();
	CSetPieces::Init();
	CTheZones::Init();
	CUserDisplay::Init();	
	CMessages::Init();
	// Make sure that any messages being displayed are removed.
	CMessages::ClearAllMessagesDisplayedByGame();
	CVehicleRecording::Init();
	
	CRestart::Initialise();
	
	CMemInfo::EnterBlock(WORLD_MEM_ID);
	CWorld::Initialise();
	CMemInfo::ExitBlock();
	
	CMemInfo::EnterBlock(ANIM_MEM_ID);
	CAnimManager::Initialise();
	CCutsceneMgr::Initialise();
	CMemInfo::ExitBlock();

	CMemInfo::EnterBlock(CARS_MEM_ID);
	CCarCtrl::Init();
	CMemInfo::ExitBlock();

	CMemInfo::EnterBlock(DEFAULTMODELS_MEM_ID);

	InitModelIndices();
	CModelInfo::Initialise();
	// have to be done before level load
	CPickups::Init();
#ifdef GTA_NETWORK	
	CNetGames::Init();	// After pickups
#endif	
	CTheCarGenerators::Init();
	//CThePedGenerators::Init();
	CGarages::Init();
	CAudioZones::Init();

	// Register Gta3.img and Txd.img with cd stream code
	CStreaming::InitImageList();
	CStreaming::ReadIniFile();

	CMemInfo::EnterBlock(PATHS_MEM_ID);
	ThePaths.Init();		// Has to be done after CStreaming::ReadIniFile();
	ThePaths.AllocatePathFindInfoMem();
	CMemInfo::ExitBlock();

	CTaskSimpleFight::LoadMeleeData();
	CCheat::ResetCheats();

	// MN - FX SYSTEM -------------------------------

//	LoadingScreen("Loading the Game", "Fx System");

	CMemInfo::EnterBlock(FX_MEM_ID);	
	g_fx.Init();
	CMemInfo::ExitBlock();	

	// ----------------------------------------------

	return true;
}

//
// name:		ReadPlayerCoordsFile
// description:	Read the player coordinates from a file 	
void ReadPlayerCoordsFile()
{
	char *pLine;
	CVector posn;
	
	int32 fid = CFileMgr::OpenFile("playercoords.txt");
	
	if(fid > 0)
	{
		pLine = CFileLoader::LoadLine(fid);
		if(sscanf(pLine, "%f%f%f", &posn.x, &posn.y, &posn.z) == 3)
		{
			CPed* pPlayer = CWorld::Players[0].pPed;
			if(pPlayer)
			{
				CStreaming::LoadScene(posn);
				pPlayer->SetPosition(posn);
			}	
		}
		CFileMgr::CloseFile(fid);
	}	
}

// Name			:	Init2
// Purpose		:	Second set of game initialisation code
bool CGame::Init2(const char *pDatFile)
{

#ifndef FINALBUILD	
//	CStreaming::DisplayMapMemoryUsage();
	CModelInfo::PrintModelInfoStoreUsage();
#endif	
	LoadingScreen("Loading the Game", "Add Particles");

	CTheZones::PostZoneCreation();
	CEntryExitManager::PostEntryExitsCreation();
	
	CMemInfo::ExitBlock();

	LoadingScreen("Loading the Game", "Setup paths");

#if defined (GTA_PC) || defined (GTA_XBOX)
//	CLoadingScreen::DoPCScreenChange(FALSE);
#endif //GTA_PC & GTA_XBOX

	CMemInfo::EnterBlock(PATHS_MEM_ID);
	ThePaths.PreparePathData();
	CMemInfo::ExitBlock();

	for (Int32 C = 0; C < MAX_NUM_PLAYERS; C++)
		CWorld::Players[C].Clear();

#ifdef GTA_PC
	// load skin texture
	CWorld::Players[0].LoadPlayerSkin();
#endif
			
#ifdef GTA_NETWORK

	// setup network game once level has been loaded and pickups have been
	// initialised
	if (FrontEndMenuManager.m_WantsToRestartGameAsServer)
	{
		LoadingScreen("Loading the Game", "Setup network");
		FrontEndMenuManager.m_WantsToRestartGameAsServer = false;
		CGameNet::StartGameAsServer();
	}
	else if (FrontEndMenuManager.m_WantsToRestartGameAsClient)
	{
		LoadingScreen("Loading the Game", "Setup network");
		FrontEndMenuManager.m_WantsToRestartGameAsClient = false;
		CGameNet::StartGameAsClient();
	}
	else
	{
		CGameNet::CloseNetwork();
	}
#endif		
	
	// Test all models have loaded
	IFWEARENOTINNETWORKGAME(TestModelIndices();)

	LoadingScreen("Loading the Game", "Setup water");
#ifndef FINAL
	CWaterLevel::ProcessWaterFiles();
#endif

	CClothes::Init();			// must be done after streaming initialisation
	CWaterLevel::WaterLevelInitialise();		// This now also loads a texture up (so it has to be after LoadLevelModels)

	#ifdef USE_VU_CARFX_RENDERER
		CVuCarFXRenderer::EnvMapLoadTextures();
	#endif
	
//	TheConsole.Reset();

	CDraw::SetFOV(120.0f);
	CDraw::SetLODDistance(500.0f);

	// initialize CarPlateMgr here, when "vehicle.txd" is already loaded:
	if(!CCustomCarPlateMgr::Initialise())
	{
		ASSERTMSG(FALSE, "Error initialising CCustomCarPlatePipe");
		return(FALSE);
	}

	//
	// Initialise streaming
	//
	LoadingScreen("Loading the Game", "Setup streaming");

	CStreaming::LoadInitialVehicles();
	CStreaming::LoadInitialPeds();
	
	// if we are in a network game load the whole level
	IFWEAREINNETWORKGAME(CStreaming::RequestAllModels();)

	CStreaming::LoadAllRequestedModels();

	DEBUGLOG1("Streaming uses %dK of its memory\n", CStreaming::GetMemoryUsed()/1024);
	
	LoadingScreen("Loading the Game", "Load animations");

	// can only load in animation after player ped has been loaded by streaming	
	CMemInfo::EnterBlock(ANIM_MEM_ID);
	CAnimManager::LoadAnimFiles();
	CMemInfo::ExitBlock();

	//CStreamingDebug::StreamEverything();

	LoadingScreen("Loading the Game", "Setup streaming");
	
	// Can only stream weapons after Anims loaded otherwise it can't stream weapon anims properly
	CStreaming::LoadInitialWeapons();
	CStreaming::LoadAllRequestedModels();

	LoadingScreen("Loading the Game", "Ped Init");
	
	// Can only initialise peds once animations have been loaded
	CPed::Initialise();
	//CRouteNode::Initialise();

	// This needs to be done after streaming is initialised
	CRenderer::Init();		// Will mark the BIG buildings and make sure they're first in the lists.
	
	LoadingScreen("Loading the Game", "Setup game variables");

	// can only initialise radar after streaming has been initialised
	CRadar::Initialise();
	CRadar::LoadTextures();

	CTouchInterface::CreateOrDelete();

	LoadingScreen(NULL, NULL);
	
	// Can only initialise weapons once animations have been loaded
	CWeapon::InitialiseWeapons();

	CRoadBlocks::Init();

	LoadingScreen(NULL, NULL);

	CPopulation::Initialise();

	LoadingScreen(NULL, NULL);
	
	CWorld::PlayerInFocus = 0;
	CCoronas::Init();
	CShadows::Init();

	LoadingScreen(NULL, NULL);

	CWeaponEffects::Init();
	CSkidmarks::Init();
//	CAntennas::Init();
	CGlass::Init();
	CGarages::Init_AfterRestart();

//	gPhoneInfo.Initialise();
#ifdef GTA_PC	
//!PC - doesn't seem to exist anymore?
//	CSceneEdit::Initialise();
#endif	
	
	LoadingScreen("Loading the Game", "Load scripts");

	CMemInfo::EnterBlock(SCRIPT_MEM_ID);
	
	IFWEARENOTINNETWORKGAME(CTheScripts::Init();)	// load in a script and stuff

	// Test thingy for the scripting stuff

	CGangs::Initialise();
	
	CMemInfo::ExitBlock();

	LoadingScreen("Loading the Game", "Setup game variables");

	// initialise game timer
	CClock::Initialise(1000);
	CHeli::InitHelis();
	CCranes::InitCranes();

	LoadingScreen(NULL, NULL);

	CMovingThings::Init();
	CDarkel::Init();
//	CStunts::Init();

	LoadingScreen(NULL, NULL);

	CGridRef::Init();
	CStats::Init();
//	CPacManPickups::Init();
//	CPowerPoints::Init(); Not used anymore
//	CRubbish::Init();
	CClouds::Init();
	CSpecialFX::Init();

	LoadingScreen(NULL, NULL);

//	CLightning::Init();
	CRopes::Init();
	CWaterCannons::Init();

	CBridge::Init();
//	CTheShopDoors::Init();
	CPedGroups::Init();
#ifdef USE_INFORM_FRIENDS_QUEUE
	CInformFriendsEventQueue::Init();	
#endif

#ifdef USE_INFORM_GROUP_QUEUE
	CInformGroupEventQueue::Init();
#endif

//	This is now done in LoadLevel() because collision isn't available here
//	CWorld::RepositionCertainDynamicObjects();

#ifdef OUTPUTTREESHADOWS	
	CShadows::OutputAllTheTreeShadowsInOneBigFile();
#endif
	
	LoadingScreen(NULL, NULL);

	// don't create planes and trains in network games
//	IFWEARENOTINNETWORKGAME(
//	CCullZones::ResolveVisibilities();
//	CFakePlane::InitPlanes();
//	)
		
	CCredits::Init();
	CReplay::Init();

	CShopping::Init();
	LoadingScreen(NULL, NULL);
	
	return true;		
} // end - CGame::Initialise

// Name			:	Init3
// Purpose		:	Third set of game initialisation code
bool CGame::Init3(const char *pDatFile)
{
	LoadingScreen("Loading the Game", "Load scene");
	
	//CCollision::SetCollisionInMemory(currLevel);
	
#ifdef DEBUG
	CWorld::CheckBuildingOrientations();
	CWorld::TestForBuildingsOnTopOfEachOther();
	//CWorld::TestForUnusedModels();
#endif	
	
	CPad::GetPad(0)->Clear(TRUE);
	CPad::GetPad(1)->Clear(TRUE);
	
#if defined(GTA_PC) && !defined(OSW)
	D3DPoolsEnable(true);
#endif

#ifdef USE_AUDIOENGINE
#else //USE_AUDIOENGINE
	#ifdef GTA_PC
	DMAudio.SetStartingTrackPositions(TRUE);  // set default radio station starting positions on PC only due to main menus
	#endif // GTA_PC
	DMAudio.ChangeMusicMode(A_GAME_MUSIC_MODE);
#endif //USE_AUDIOENGINE

	
	// MN - PROCEDURAL INTERIORS --------------------
	
	LoadingScreen("Loading the Game", "Procedural Interiors");

	CMemInfo::EnterBlock(PROCEDURAL_INTERIORS);	
	g_interiorMan.Init();
	CMemInfo::ExitBlock();	

	// ----------------------------------------------
	
	
	// MN - PROCEDURAL OBJECTS ----------------------
	g_procObjMan.Init();
	// ----------------------------------------------
	
	// MN - WATER CREATURES -------------------------
	g_waterCreatureMan.Init();
	// ----------------------------------------------
	
	// MN - REAL TIME SHADOWS -----------------------
	g_realTimeShadowMan.Init();
	// ----------------------------------------------
	
#ifndef FINALBUILD
	ReadPlayerCoordsFile();
#endif

	return true;

}

#ifdef GTA_PS2
//
//
// from ps2all.cpp
//
//
#include "ps2all.h"
#include "rwplugin.h"

// pipelines created
extern RxPipeline* pOptimisedLightPipe;
extern RxPipeline* pOptimisedLightSkinnedPipe;
extern RxPipeline* pVehicleColourMaterialPipe;

RwBool RpMeshPS2AllVehicleBridgeCallBack(RxPS2AllPipeData *ps2AllPipeData);

//
// name:		SetObjectSetupCallback
// description:	Replace default objectsetup callback for pipe with optimised version
void SetObjectSetupCallback(RxPipeline* pPipe, RxPipelineNodePS2AllObjectSetupCallBack cb)
{
	ASSERT(pPipe)

    RxNodeDefinition   *ps2All = RxNodeDefinitionGetPS2All();
    RxPipelineNode     *plNode;

    plNode = RxPipelineFindNodeByName(pPipe,
                                      ps2All->name,
                                      (RxPipelineNode *)NULL,
                                      (RwInt32 *) NULL);

    /* Set up the necessary callbacks */

    /* We are replacing the default RpAtomic objectSetupCB with an
     * optimized version (see above for its definition). */
    RxPipelineNodePS2AllSetCallBack(plNode, rxPS2ALLCALLBACKOBJECTSETUP, cb); /* RpAtomicPS2AllObjectSetupCallBack); */
}

//
// name:		PS2AllOptimizedPipelinesCreate
// description:	Create optimised ps2all pipeline
RwBool PS2AllOptimizedPipelinesCreate()
{
	RpPDSSkyObjTemplate rwPDS_OptmisedLight_AtmPipe =
	{
	    rwVENDORID_ROCKSTAR,
	    rwPDS_OptmisedLight_AtmPipeID,
	    (RxPipelineNodePS2AllObjectSetupCallBack)PS2AllObjectSetupCB,
	    (RxPipelineNodePS2AllObjectFinalizeCallBack)NULL,
	    (RxPipeline *)NULL
	};
	RpPDSSkyObjTemplate rwPDS_OptmisedLightSkinned_AtmPipe =
	{
	    rwVENDORID_ROCKSTAR,
	    rwPDS_OptmisedLightSkinned_AtmPipeID,
	    (RxPipelineNodePS2AllObjectSetupCallBack)PS2AllSkinnedObjectSetupCB,
	    (RxPipelineNodePS2AllObjectFinalizeCallBack)NULL,
	    (RxPipeline *)NULL
	};
	RpPDSSkyMatTemplate rwPDS_G3_VehicleColour_MatPipe =
	{
	    rwVENDORID_ROCKSTAR,                         /* PluginId.            */
	    rwPDS_VehicleColour_MatPipeID,             /* PluginData.          */
	    { { &RxClPS2xyz,    CL_XYZ    },        /* Cluster1.            */
	      { &RxClPS2uv,     CL_UV     },        /* Cluster2.            */
	      { &RxClPS2rgba,   CL_RGBA   },        /* Cluster3.            */
	      { &RxClPS2normal, CL_NORMAL },        /* Cluster4.            */
	      { NULL,           CL_MAXCL  },        /* Cluster5.            */
	      { NULL,           CL_MAXCL  },        /* Cluster6.            */
	      { NULL,           CL_MAXCL  },        /* Cluster7.            */
	      { NULL,           CL_MAXCL  },
	      { NULL,           CL_MAXCL  },
	      { NULL,           CL_MAXCL  } },      /* Cluster8.            */
	    { rwPRIMTYPETRISTRIP,                   /* Primitive type.      */
	      _rwskyStrideOfInputCluster,           /* ClusterStride.       */
	      _rwskyTSVertexCount,                  /* VertexCount.         */
	      _rwskyTLTriCount,                     /* PrimCount.           */
	      _rwskyVIFOffset },                    /* VifOffset.           */
	    &rwPDS_G3_Generic_VU1Code,              /* VU1Code array.       */
	    RpPDS_G3_PS2AllMeshInstanceTestCallBack,/* InstanceTestCallBack */
	    RpPDS_G3_PS2AllResEntryAllocCallBack,   /* ResEntryAllocCallBack*/
	    RpPDS_G3_PS2AllInstanceCallBack,        /* InstanceCallBack     */
	    RpMeshPS2AllVehicleBridgeCallBack,      /* BridgeCallBack       */
	    RpPDS_G3_PS2AllPostMeshCallBack         /* PostMeshCallBack     */
	};

	// Create pipes
	pOptimisedLightPipe = RpPDSSkyObjPipeCreate(&rwPDS_OptmisedLight_AtmPipe);
	pOptimisedLightSkinnedPipe = RpPDSSkyObjPipeCreate(&rwPDS_OptmisedLightSkinned_AtmPipe);
	pVehicleColourMaterialPipe = RpPDSSkyMatPipeCreate(&rwPDS_G3_VehicleColour_MatPipe);
	
	
	// Andrzej switched off default matfx pipes to save 300KB
	// matfx uses the callback as well
//	SetObjectSetupCallback(RpPDSGetPipe(rwPDS_G3_MatfxUV1_GrpAtmPipeID), &PS2AllObjectSetupCB);
//	SetObjectSetupCallback(RpPDSGetPipe(rwPDS_G3_MatfxUV2_GrpAtmPipeID), &PS2AllObjectSetupCB);


	// skin uses the skinned version of the callback
	SetObjectSetupCallback(RpPDSGetPipe(rwPDS_G3_Skin_GrpAtmPipeID), &PS2AllSkinnedObjectSetupCB);
		
    return (TRUE);
}

#endif
//
//
// from RwCam.cpp
//
//

#if defined (GTA_PC) || defined (GTA_PS2)
//
// name:		CameraCreate
// description:	Create the camera
RwCamera *CameraCreate(RwInt32 width, RwInt32 height, RwBool zBuffer)
{
    RwCamera *camera;
    RwRect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = width;
    rect.h = height;

    camera = RwCameraCreate();
    
    if (camera)
    {
        RwCameraSetFrame(camera, RwFrameCreate());
        RwCameraSetRaster(camera, 
                          RwRasterCreate(width,height,0, rwRASTERTYPECAMERA));

        /*RwRasterSubRaster(RwCameraGetRaster(camera), 
                          RwRasterCreate(width, height, 
                                         0, rwRASTERTYPECAMERA),
                          &rect);*/

        if (zBuffer)
        {
            RwCameraSetZRaster(camera, 
                               RwRasterCreate(width,height,0, rwRASTERTYPEZBUFFER));

            /*RwRasterSubRaster(RwCameraGetZRaster(camera), 
                              RwRasterCreate(width, height,
                                         0, rwRASTERTYPEZBUFFER),
                              &rect);*/
        }

        /* now check that everything is valid */
        if (RwCameraGetFrame(camera) &&
            RwCameraGetRaster(camera) &&
            RwRasterGetParent(RwCameraGetRaster(camera)) &&
            (!zBuffer || (RwCameraGetZRaster(camera) &&
                          RwRasterGetParent(RwCameraGetZRaster(camera))))
            )
        {
            /* everything OK */
            return(camera);
        }
    }

    /* if we're here then an error must have occurred so clean up */

    CameraDestroy(camera);
    return(NULL);
}
#endif //GTA_PC & GTA_PS2
//
//
// from Lights.cpp
//
//
#include "lights.h"

/* we have two lights in the demo */
extern RpLight *pAmbient;
extern RpLight *pDirect;

// A array of directional lights to be used for pointlights.
extern RpLight *pExtraDirectionals[NUM_EXTRA_DIRECTIONALS];

//
// name:		LightsCreate
// description:	Create the RW lights used in the game
RpWorld *LightsCreate(RpWorld *world)
{
    RwFrame		*frame;
    Int16		ExtraDirNum;
    
    if (world)
    {
        RwRGBAReal color;
        RwV3d axis;

        pAmbient = RpLightCreate(rpLIGHTAMBIENT);
        ASSERT(pAmbient);
        RpLightSetFlags(pAmbient, rpLIGHTLIGHTATOMICS);
        color.red   = 0.25f;
        color.green = 0.25f;
        color.blue  = 0.20f;
        RpLightSetColor(pAmbient, &color);


        pDirect = RpLightCreate(rpLIGHTDIRECTIONAL);
        ASSERT (pDirect);
        RpLightSetFlags(pDirect, rpLIGHTLIGHTATOMICS);
        color.red   = 1.0f;
        color.green = 0.85f;
        color.blue  = 0.45f;
        RpLightSetColor(pDirect, &color);
        RpLightSetRadius(pDirect, 2.0f);
        frame = RwFrameCreate();
        ASSERT(frame);
        RpLightSetFrame(pDirect, frame);
        axis.x = 1.0f;
        axis.y = 1.0f;
        axis.z = 0.0f;
        RwFrameRotate(frame, &axis, 160.0f, rwCOMBINEPRECONCAT);

        RpWorldAddLight(world, pAmbient);
        RpWorldAddLight(world, pDirect);

		// Create the extra directional lights. Don't add them to
		// the world just now.
		for (ExtraDirNum = 0; ExtraDirNum < NUM_EXTRA_DIRECTIONALS; ExtraDirNum++)
		{
        	pExtraDirectionals[ExtraDirNum] = RpLightCreate(rpLIGHTDIRECTIONAL);
	        ASSERT (pExtraDirectionals[ExtraDirNum]);
    	    RpLightSetFlags(pExtraDirectionals[ExtraDirNum], 0);
    	    
	        color.red   = 1.0f;
    	    color.green = 0.5f;
	        color.blue  = 0.0f;
	        RpLightSetColor(pExtraDirectionals[ExtraDirNum], &color);
	        RpLightSetRadius(pExtraDirectionals[ExtraDirNum], 2.0f);
	        frame = RwFrameCreate();
	        ASSERT(frame);
	        RpLightSetFrame(pExtraDirectionals[ExtraDirNum], frame);

	        RpWorldAddLight(world, (pExtraDirectionals[ExtraDirNum]));
//	        axis.x = 1.0f;
//	        axis.y = 1.0f;
//		    axis.z = 0.0f;
//          RwFrameRotate(frame, &axis, 160.0f, rwCOMBINEPRECONCAT);
		}

        return world;
    }

    // can't add lights - need a world first
    return NULL;
}

//
//
// from Font.cpp
//
//
#include "font.h"

////////////////////////////////////////////////////////////////////////////
// name:	Initialise
//
// purpose: initialises the font stuff and loads in the TXD
////////////////////////////////////////////////////////////////////////////
void CFont::Initialise()
{
	int32 fontTxdId;
	
	fontTxdId = CTxdStore::AddTxdSlot("fonts");
	CTxdStore::LoadTxd(fontTxdId, "MODELS\\FONTS.TXD");
	CTxdStore::AddRef(fontTxdId);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(fontTxdId);
	
	// Load the font bitmaps
	Sprite[FO_FONT_STYLE_BANK].SetTexture("font2","font2m");
	Sprite[FO_FONT_STYLE_STANDARD].SetTexture("font1","font1m");
	
	// load in the proportional values from file:
	CFont::LoadFontValues();
	SetScale(1,1);
	SetSlantRefPoint(SCREEN_WIDTH, 0);
	SetSlant(0.0f);
	SetColor(CRGBA (255,255,255,0));
	SetOrientation(FO_LEFT);
	SetJustify(FALSE);
	SetWrapx( SCREEN_WIDTH );
	SetCentreSize( SCREEN_WIDTH );
	SetBackground(FALSE);
	SetBackgroundColor( CRGBA (128,128,128,128));
	SetProportional(TRUE);
	SetFontStyle(FO_FONT_STYLE_BANK);
	SetRightJustifyWrap ( 0 );
	SetAlphaFade( 255 );
	SetDropShadowPosition(0);

	CTxdStore::PopCurrentTxd();

	// load in PS2 buttons:

	fontTxdId = CTxdStore::AddTxdSlot("ps2btns");
#if defined (GTA_PS2)
	CTxdStore::LoadTxd(fontTxdId, "MODELS\\PS2BTNS.TXD");
#elif defined (GTA_XBOX)
	CTxdStore::LoadTxd(fontTxdId, "MODELS\\XBXBTNS.TXD");
#elif defined (GTA_PC)
	CTxdStore::LoadTxd(fontTxdId, "MODELS\\PCBTNS.TXD");
#endif
	CTxdStore::AddRef(fontTxdId);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(fontTxdId);

	PS2Sprite[FO_PS2_DPAD_UP].SetTexture("up", "upA");
	PS2Sprite[FO_PS2_DPAD_DOWN].SetTexture("down", "downA");
	PS2Sprite[FO_PS2_DPAD_LEFT].SetTexture("left", "leftA");
	PS2Sprite[FO_PS2_DPAD_RIGHT].SetTexture("right", "rightA");

#ifdef GTA_PS2
	PS2Sprite[FO_PS2_CROSS].SetTexture("cross", "crossA");
	PS2Sprite[FO_PS2_CIRCLE].SetTexture("circle", "circleA");
	PS2Sprite[FO_PS2_TRIANGLE].SetTexture("triangle", "triangleA");
	PS2Sprite[FO_PS2_SQUARE].SetTexture("square", "squareA");
	PS2Sprite[FO_PS2_L1].SetTexture("l1","l1A");
	PS2Sprite[FO_PS2_L2].SetTexture("l2","l2A");
	PS2Sprite[FO_PS2_L3].SetTexture("l3","l3A");
	PS2Sprite[FO_PS2_R1].SetTexture("r1","r1A");
	PS2Sprite[FO_PS2_R2].SetTexture("r2","r2A");
	PS2Sprite[FO_PS2_R3].SetTexture("r3","r3A");
#endif

#ifdef GTA_XBOX
	PS2Sprite[FO_PS2_CROSS].SetTexture("A", "Am");
	PS2Sprite[FO_PS2_CIRCLE].SetTexture("B", "Bm");
	PS2Sprite[FO_PS2_TRIANGLE].SetTexture("Y", "Ym");
	PS2Sprite[FO_PS2_SQUARE].SetTexture("X", "Xm");
	PS2Sprite[FO_PS2_L1].SetTexture("L","Lm");
	PS2Sprite[FO_PS2_L2].SetTexture("WHITE","WHITEm");
	PS2Sprite[FO_PS2_L3].SetTexture("LSTICK","LSTICKm");
	PS2Sprite[FO_PS2_R1].SetTexture("R","Rm");
	PS2Sprite[FO_PS2_R2].SetTexture("BLACK","BLACKm");
	PS2Sprite[FO_PS2_R3].SetTexture("RSTICK","RSTICKm");
#endif
	
	CTxdStore::PopCurrentTxd();
}

//
//
// from Hud.cpp
//
//
#include "hud.h"

static sFilename WeaponFilenames[HUD_END]=
{
	{"fist","fistm"},
	{"siteM16","siteM16m"},
	{"siterocket","siterocketm"},
	{"radardisc","radardiscA"},
	{"radarRingPlane","radarRingPlaneA"},
	{"SkipIcon","SkipIconA"},
#if defined GTA_PS2 || defined GTA_XBOX
	{"SkipHigh","SkipHighA"},
#endif
#ifdef DISPLAY_PAGER
	{"pager","pagerA"},
#endif
};



////////////////////////////////////////////////////////////////////////////
// name:	Initialise
//
// purpose: inits stuff and loads in textures
////////////////////////////////////////////////////////////////////////////
void CHud::Initialise ()
{
	int32 hudTxdId;
	
	hudTxdId = CTxdStore::AddTxdSlot("hud");
	CTxdStore::LoadTxd(hudTxdId, "MODELS\\HUD.TXD");
	CTxdStore::AddRef(hudTxdId);
	ASSERT(hudTxdId != -1);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(hudTxdId);
	
	
	// Load two (three miami)three textures for the hud sights
//	if (!gpSniperSightTex)
//	{
//    	gpSniperSightTex = RwTextureRead(RWSTRING("sitesniper"), NULL );
//    	ASSERT(gpSniperSightTex);
//    }
//	if (!gpRocketSightTex)
//	{
//    	gpRocketSightTex = RwTextureRead(RWSTRING("siterocket"), NULL );
//    	ASSERT(gpRocketSightTex);
//    }
//	if (!gpLaserSightTex)
//	{
//    	gpLaserSightTex = RwTextureRead(RWSTRING("sitelaser"), NULL );
//    	ASSERT(gpLaserSightTex);
//    }
//	if (!gpLaserDotTex)
//	{
//    	gpLaserDotTex = RwTextureRead(RWSTRING("laserdot"), RWSTRING("laserdotm") );
//    	ASSERT(gpLaserDotTex);
//   }
//	if (!gpCameraSightTex)
//	{
//    	gpCameraSightTex = RwTextureRead(RWSTRING("viewfinder_128"), RWSTRING("viewfinder_128m"));
//    	ASSERT(gpCameraSightTex);
//    }
    
	// Load them
	for (uint8 Counter = 0 ; Counter < HUD_END ; Counter ++)
	{
		CHud::Sprites[Counter].SetTexture(WeaponFilenames[Counter].pBitmapFileName, WeaponFilenames[Counter].pMaskFileName);
	}
	
	CTxdStore::PopCurrentTxd();

	ReInitialise();  // init all the vars to the default values

}




//
//
// from TempColModels.cpp
//
//
#include "TempColModels.h"
#include "ColPoint.h"
#include "PedModelInfo.h"


extern CColSphere s_aPedSpheres[3];
extern CColLine   s_aPedLines[2];
extern CColSphere s_aPed2Spheres[9];
extern CColLine   s_aPed2Lines[2];
//extern CColSphere s_aPedGSpheres[4];
extern CColSphere s_aDoorSpheres[4];
extern CColSphere s_aBumperSpheres[4];
extern CColSphere s_aPanelSpheres[4];
extern CColSphere s_aBonnetSpheres[4];
extern CColSphere s_aBootSpheres[4];
extern CColSphere s_aWheelSpheres[2];
extern CColSphere s_aBodyPartSpheres1[2];
extern CColSphere s_aBodyPartSpheres2[2];

void CTempColModels::Initialise()
{
	CCollisionData* pColData;

////////////////////////////////////
// basic 4m x 4m x 4m bounding box collsion vol
	ms_colModelBBox.GetBoundSphere().Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelBBox.GetBoundBox().Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
	ms_colModelBBox.m_level = LEVEL_GENERIC;
	
// basic 4m x 4m x 4m bounding box collsion vol for cutscene object
	ms_colModelCutObj[0].GetBoundSphere().Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelCutObj[0].GetBoundBox().Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
	ms_colModelCutObj[0].m_level = LEVEL_GENERIC;
	
// basic 4m x 4m x 4m bounding box collsion vol for cutscene object
	ms_colModelCutObj[1].GetBoundSphere().Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelCutObj[1].GetBoundBox().Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
	ms_colModelCutObj[1].m_level = LEVEL_GENERIC;
	
// basic 4m x 4m x 4m bounding box collsion vol for cutscene object
	ms_colModelCutObj[2].GetBoundSphere().Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelCutObj[2].GetBoundBox().Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
	ms_colModelCutObj[2].m_level = LEVEL_GENERIC;
	
// basic 4m x 4m x 4m bounding box collsion vol for cutscene object
	ms_colModelCutObj[3].GetBoundSphere().Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelCutObj[3].GetBoundBox().Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
	ms_colModelCutObj[3].m_level = LEVEL_GENERIC;
	
// basic 4m x 4m x 4m bounding box collsion vol for cutscene object
	ms_colModelCutObj[4].GetBoundSphere().Set(2.0f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelCutObj[4].GetBoundBox().Set(CVector(-2.0f, -2.0f, -2.0f), CVector(2.0f, 2.0f, 2.0f));
	ms_colModelCutObj[4].m_level = LEVEL_GENERIC;
	
	
////////////////////////////////////	
// Normal ped collision spheres
/*	s_aPedSpheres[0].m_fRadius = 0.35f;
	s_aPedSpheres[1].m_fRadius = 0.35f;
	s_aPedSpheres[2].m_fRadius = 0.35f;

	s_aPedSpheres[0].m_vecCentre = CVector( 0.0f,  0.0f, 0.80f - 1.0f);
	s_aPedSpheres[1].m_vecCentre = CVector( 0.0f,  0.0f, 1.20f - 1.0f);
	s_aPedSpheres[2].m_vecCentre = CVector( 0.0f,  0.0f, 1.60f - 1.0f);
	
	for(int32 i = 0; i < 3; i++)
		s_aPedSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_PED;

	s_aPedSpheres[0].m_data.m_nPieceType = PED_COL_SPHERE_LEG;//COLPOINT_PIECETYPE_PED_SPHERE_LEG;
	s_aPedSpheres[1].m_data.m_nPieceType = PED_COL_SPHERE_MID;//COLPOINT_PIECETYPE_PED_SPHERE_MID;
	s_aPedSpheres[2].m_data.m_nPieceType = PED_COL_SPHERE_HEAD;//COLPOINT_PIECETYPE_PED_SPHERE_HEAD;
	
	s_aPedLines[0].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,-1.0f));
	s_aPedLines[1].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,1.0f));
*/
	ms_colModelPed1.AllocateData(3, 0, 2, 0, 0);
	pColData = ms_colModelPed1.GetCollisionData();
	pColData->m_pSphereArray[0].Set(0.35, CVector( 0.0f,  0.0f, 0.80f - 1.0f), SURFACE_TYPE_PED, PED_COL_SPHERE_LEG);
	pColData->m_pSphereArray[1].Set(0.35, CVector( 0.0f,  0.0f, 1.20f - 1.0f), SURFACE_TYPE_PED, PED_COL_SPHERE_MID);
	pColData->m_pSphereArray[2].Set(0.35, CVector( 0.0f,  0.0f, 1.60f - 1.0f), SURFACE_TYPE_PED, PED_COL_SPHERE_HEAD);

	pColData->m_pLineArray[0].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,-1.0f));
	pColData->m_pLineArray[1].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,1.0f));
	pColData->m_nNoOfLines = 0;
	
	ms_colModelPed1.GetBoundSphere().Set(STD_PED_BOUND_RADIUS, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelPed1.GetBoundBox().Set(CVector(-0.35f, -0.35f, STD_PED_BOUND_BOX_MIN), CVector(0.35f, 0.35f, STD_PED_BOUND_BOX_MAX));
	ms_colModelPed1.m_level = LEVEL_GENERIC;

////////////////////////////////////
// Extra ped col model with extra spheres
// for testing blocked positions on front of ped
/*	s_aPed2Spheres[0].m_fRadius = 0.35f;
	s_aPed2Spheres[1].m_fRadius = 0.35f;
	s_aPed2Spheres[2].m_fRadius = 0.35f;
	s_aPed2Spheres[3].m_fRadius = 0.13f;
	s_aPed2Spheres[4].m_fRadius = 0.13f;
	s_aPed2Spheres[5].m_fRadius = 0.13f;
	s_aPed2Spheres[6].m_fRadius = 0.13f;
	s_aPed2Spheres[7].m_fRadius = 0.20f;
	s_aPed2Spheres[8].m_fRadius = 0.35f;

	s_aPed2Spheres[0].m_vecCentre = CVector( 0.0f,  0.0f, 0.80f - 1.0f);
	s_aPed2Spheres[1].m_vecCentre = CVector( 0.0f,  0.0f, 1.20f - 1.0f);
	s_aPed2Spheres[2].m_vecCentre = CVector( 0.0f,  0.0f, 1.80f - 1.0f);
	s_aPed2Spheres[3].m_vecCentre = CVector( 0.18f, 0.35f, 1.75f - 1.0f);	// right arm standing up
	s_aPed2Spheres[4].m_vecCentre = CVector( 0.18f, 0.50f, 1.75f - 1.0f);	// right arm standing up
	s_aPed2Spheres[5].m_vecCentre = CVector(-0.18f, 0.35f, 1.75f - 1.0f);	// left arm standing up
	s_aPed2Spheres[6].m_vecCentre = CVector(-0.18f, 0.50f, 1.75f - 1.0f);	// left arm standing up
	s_aPed2Spheres[7].m_vecCentre = CVector( 0.0f,  0.50f, 1.45f - 1.0f);	// right arm crouched fire pos
	s_aPed2Spheres[8].m_vecCentre = CVector( 0.0f,  0.55f, 1.20f - 1.0f);	// waist to check for jumping over stuff
	
	for(int32 i = 0; i < 9; i++)
	{
		s_aPed2Spheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_PED;
	}
	s_aPed2Spheres[0].m_data.m_nPieceType = PED_COL_SPHERE_LEG;//COLPOINT_PIECETYPE_PED_SPHERE_LEG;
	s_aPed2Spheres[1].m_data.m_nPieceType = PED_COL_SPHERE_MID;//COLPOINT_PIECETYPE_PED_SPHERE_MID;
	s_aPed2Spheres[2].m_data.m_nPieceType = PED_COL_SPHERE_HEAD;//COLPOINT_PIECETYPE_PED_SPHERE_HEAD;
	s_aPed2Spheres[3].m_data.m_nPieceType = PED_SPHERE_UPPERARM_R;//COLPOINT_PIECETYPE_PED_ZONE_UPPERARM_R;
	s_aPed2Spheres[4].m_data.m_nPieceType = PED_SPHERE_UPPERARM_R;//COLPOINT_PIECETYPE_PED_ZONE_UPPERARM_R;
	s_aPed2Spheres[5].m_data.m_nPieceType = PED_SPHERE_UPPERARM_L;//COLPOINT_PIECETYPE_PED_ZONE_UPPERARM_L;
	s_aPed2Spheres[6].m_data.m_nPieceType = PED_SPHERE_UPPERARM_L;//COLPOINT_PIECETYPE_PED_ZONE_UPPERARM_L;
	s_aPed2Spheres[7].m_data.m_nPieceType = PED_SPHERE_LEG_R;//COLPOINT_PIECETYPE_PED_ZONE_LEG_R;
	s_aPed2Spheres[8].m_data.m_nPieceType = PED_SPHERE_MIDSECTION;//COLPOINT_PIECETYPE_PED_ZONE_MIDSECTION;

	s_aPed2Lines[0].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,-1.0f));
	s_aPed2Lines[1].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,1.0f));*/

	ms_colModelPed2.AllocateData(9, 0, 2, 0, 0);
	pColData = ms_colModelPed2.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.35f, CVector( 0.0f,  0.0f, 0.80f - 1.0f), SURFACE_TYPE_PED, PED_COL_SPHERE_LEG);
	pColData->m_pSphereArray[1].Set(0.35f, CVector( 0.0f,  0.0f, 1.20f - 1.0f), SURFACE_TYPE_PED, PED_COL_SPHERE_MID);
	pColData->m_pSphereArray[2].Set(0.35f, CVector( 0.0f,  0.0f, 1.60 - 1.0f), SURFACE_TYPE_PED, PED_COL_SPHERE_HEAD);
	pColData->m_pSphereArray[3].Set(0.13f, CVector( 0.18f, 0.35f, 1.75f - 1.0f),SURFACE_TYPE_PED, PED_SPHERE_UPPERARM_R);	// right arm standing up
	pColData->m_pSphereArray[4].Set(0.13f, CVector( 0.18f, 0.50f, 1.75f - 1.0f),SURFACE_TYPE_PED, PED_SPHERE_UPPERARM_R);   // right arm standing up
	pColData->m_pSphereArray[5].Set(0.13f, CVector(-0.18f, 0.35f, 1.75f - 1.0f),SURFACE_TYPE_PED, PED_SPHERE_UPPERARM_L);   // left arm standing up
	pColData->m_pSphereArray[6].Set(0.13f, CVector(-0.18f, 0.50f, 1.75f - 1.0f),SURFACE_TYPE_PED, PED_SPHERE_UPPERARM_L);   // left arm standing up
	pColData->m_pSphereArray[7].Set(0.20f, CVector( 0.0f,  0.50f, 1.45f - 1.0f),SURFACE_TYPE_PED, PED_SPHERE_LEG_R);		// right arm crouched fire pos
	pColData->m_pSphereArray[8].Set(0.35f, CVector( 0.0f,  0.55f, 1.20f - 1.0f),SURFACE_TYPE_PED, PED_SPHERE_MIDSECTION);   // waist to check for jumping over stuff
	
	pColData->m_pLineArray[0].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,-1.0f));
	pColData->m_pLineArray[1].Set(CVector(0.0f,0.0f,0.0f), CVector(0.0f,0.0f,1.0f));
	pColData->m_nNoOfLines = 0;	// turn lines off by default

	ms_colModelPed2.GetBoundSphere().Set(STD_PED_BOUND_RADIUS, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelPed2.GetBoundBox().Set(CVector(-0.35f, -0.35f, STD_PED_BOUND_BOX_MIN), CVector(0.35f, 0.9f, STD_PED_BOUND_BOX_MAX));
	ms_colModelPed2.m_level = LEVEL_GENERIC;


///////////////////////////////////////////////
//	Door collision model
/*	s_aDoorSpheres[0].m_fRadius = 0.15f;
	s_aDoorSpheres[1].m_fRadius = 0.15f;
	s_aDoorSpheres[2].m_fRadius = 0.25f;
//	s_aDoorSpheres[3].m_fRadius = 0.1f;


	s_aDoorSpheres[0].m_vecCentre = CVector( 0.0f, -0.25f, -0.35f);
	s_aDoorSpheres[1].m_vecCentre = CVector( 0.0f, -0.95f, -0.35f);
	s_aDoorSpheres[2].m_vecCentre = CVector( 0.0f, -0.60f, 0.25f);
//	s_aDoorSpheres[3].m_vecCentre = CVector( -0.45f, 0.45f, 0.0f);

	for(int32 i = 0; i < 3; i++)
	{
		s_aDoorSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_CAR_PANEL;
		s_aDoorSpheres[i].m_data.m_nPieceType = 0;
	}*/

	ms_colModelDoor1.AllocateData(3, 0, 0, 0, 0);
	pColData = ms_colModelDoor1.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.15f, CVector( 0.0f, -0.25f, -0.35f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[1].Set(0.15f, CVector( 0.0f, -0.95f, -0.35f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[2].Set(0.25f, CVector( 0.0f, -0.60f, 0.25f), SURFACE_TYPE_CAR_PANEL, 0);
	
	ms_colModelDoor1.GetBoundSphere().Set(1.5f, CVector(0.0f, -0.6f, 0.0f));
	ms_colModelDoor1.GetBoundBox().Set(CVector(-0.3f, 0.0f, -0.6f), CVector(0.3f, -1.2f, 0.6f));
	ms_colModelDoor1.m_level = LEVEL_GENERIC;


///////////////////////////////////////////////
//	Bumper collision model
/*	s_aBumperSpheres[0].m_fRadius = 0.15f;
	s_aBumperSpheres[1].m_fRadius = 0.15f;
	s_aBumperSpheres[2].m_fRadius = 0.15f;
	s_aBumperSpheres[3].m_fRadius = 0.15f;


	s_aBumperSpheres[0].m_vecCentre = CVector( 0.85f, -0.05f, 0.0f);
	s_aBumperSpheres[1].m_vecCentre = CVector( 0.40f, 0.05f, 0.0f);
	s_aBumperSpheres[2].m_vecCentre = CVector( -0.40f, 0.05f, 0.0f);
	s_aBumperSpheres[3].m_vecCentre = CVector( -0.85f, -0.05f, 0.0f);

	for(int32 i = 0; i < 4; i++)
	{
		s_aBumperSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_CAR_PANEL;
		s_aBumperSpheres[i].m_data.m_nPieceType = 0;
	}*/

	ms_colModelBumper1.AllocateData(4, 0, 0, 0, 0);
	pColData = ms_colModelBumper1.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.15f, CVector( 0.85f, -0.05f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[1].Set(0.15f, CVector( 0.40f, 0.05f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[2].Set(0.15f, CVector( -0.40f, 0.05f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[3].Set(0.15f, CVector( -0.85f, -0.05f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);

	ms_colModelBumper1.GetBoundSphere().Set(2.2f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelBumper1.GetBoundBox().Set(CVector(-1.2f, -0.3f, -0.2f), CVector(1.2f, 0.3f, 0.2f));
	ms_colModelBumper1.m_level = LEVEL_GENERIC;


///////////////////////////////////////////////
//	Panel collision model
/*	s_aPanelSpheres[0].m_fRadius = 0.15f;
	s_aPanelSpheres[1].m_fRadius = 0.15f;
	s_aPanelSpheres[2].m_fRadius = 0.15f;
	s_aPanelSpheres[3].m_fRadius = 0.15f;


	s_aPanelSpheres[0].m_vecCentre = CVector( 0.15f, 0.45f, 0.0f);
	s_aPanelSpheres[1].m_vecCentre = CVector( 0.15f, -0.45f, 0.0f);
	s_aPanelSpheres[2].m_vecCentre = CVector( -0.15f, -0.45f, 0.0f);
	s_aPanelSpheres[3].m_vecCentre = CVector( -0.15f, 0.45f, 0.0f);

	for(int32 i = 0; i < 4; i++)
	{
		s_aPanelSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_CAR_PANEL;
		s_aPanelSpheres[i].m_data.m_nPieceType = 0;
	}*/

	ms_colModelPanel1.AllocateData(4, 0, 0, 0, 0);
	pColData = ms_colModelPanel1.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.15f, CVector( 0.15f, 0.45f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[1].Set(0.15f, CVector( 0.15f, -0.45f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[2].Set(0.15f, CVector( -0.15f, -0.45f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[3].Set(0.15f, CVector( -0.15f, 0.45f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);

	ms_colModelPanel1.GetBoundSphere().Set(1.4f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelPanel1.GetBoundBox().Set(CVector(-0.3f, -0.6f, -0.15f), CVector(0.3f, 0.6f, 0.15f));
	ms_colModelPanel1.m_level = LEVEL_GENERIC;


///////////////////////////////////////////////
//	Bonnet collision model
/*	s_aBonnetSpheres[0].m_fRadius = 0.2f;
	s_aBonnetSpheres[1].m_fRadius = 0.2f;
	s_aBonnetSpheres[2].m_fRadius = 0.2f;
	s_aBonnetSpheres[3].m_fRadius = 0.2f;


	s_aBonnetSpheres[0].m_vecCentre = CVector( -0.4f, 0.1f, 0.0f);
	s_aBonnetSpheres[1].m_vecCentre = CVector( -0.4f, 0.9f, 0.0f);
	s_aBonnetSpheres[2].m_vecCentre = CVector( 0.4f, 0.1f, 0.0f);
	s_aBonnetSpheres[3].m_vecCentre = CVector( 0.4f, 0.9f, 0.0f);

	for(int32 i = 0; i < 4; i++)
	{
		s_aBonnetSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_CAR_PANEL;
		s_aBonnetSpheres[i].m_data.m_nPieceType = 0;
	}
*/
	ms_colModelBonnet1.AllocateData(4, 0, 0, 0, 0);
	pColData = ms_colModelBonnet1.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.2f, CVector( -0.4f, 0.1f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[1].Set(0.2f, CVector( -0.4f, 0.9f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[2].Set(0.2f, CVector( 0.4f, 0.1f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[3].Set(0.2f, CVector( 0.4f, 0.9f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);

	ms_colModelBonnet1.GetBoundSphere().Set(1.7f, CVector(0.0f, 0.5f, 0.0f));
	ms_colModelBonnet1.GetBoundBox().Set(CVector(-0.7f, -0.2f, -0.3f), CVector(0.7f, 1.2f, 0.3f));
	ms_colModelBonnet1.m_level = LEVEL_GENERIC;


///////////////////////////////////////////////
//	Boot collision model
/*	s_aBootSpheres[0].m_fRadius = 0.2f;
	s_aBootSpheres[1].m_fRadius = 0.2f;
	s_aBootSpheres[2].m_fRadius = 0.2f;
	s_aBootSpheres[3].m_fRadius = 0.2f;


	s_aBootSpheres[0].m_vecCentre = CVector( -0.4f, -0.1f, 0.0f);
	s_aBootSpheres[1].m_vecCentre = CVector( -0.4f, -0.6f, 0.0f);
	s_aBootSpheres[2].m_vecCentre = CVector( 0.4f, -0.1f, 0.0f);
	s_aBootSpheres[3].m_vecCentre = CVector( 0.4f, -0.6f, 0.0f);

	for(int32 i = 0; i < 4; i++)
	{
		s_aBootSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_CAR_PANEL;
		s_aBootSpheres[i].m_data.m_nPieceType = 0;
	}*/

	ms_colModelBoot1.AllocateData(4, 0, 0, 0, 0);
	pColData = ms_colModelBoot1.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.2f, CVector( -0.4f, -0.1f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[1].Set(0.2f, CVector( -0.4f, -0.6f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[2].Set(0.2f, CVector( 0.4f, -0.1f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);
	pColData->m_pSphereArray[3].Set(0.2f, CVector( 0.4f, -0.6f, 0.0f), SURFACE_TYPE_CAR_PANEL, 0);

	ms_colModelBoot1.GetBoundSphere().Set(1.4f, CVector(0.0f, -0.4f, 0.0f));
	ms_colModelBoot1.GetBoundBox().Set(CVector(-0.7f, -0.9f, -0.3f), CVector(0.7f, 0.2f, 0.3f));
	ms_colModelBoot1.m_level = LEVEL_GENERIC;
	
///////////////////////////////////////////////
//	Wheel collision model
/*	s_aWheelSpheres[0].m_fRadius = 0.35f;
	s_aWheelSpheres[1].m_fRadius = 0.35f;

	s_aWheelSpheres[0].m_vecCentre = CVector( -0.3f, 0.0f, 0.0f);
	s_aWheelSpheres[1].m_vecCentre = CVector( 0.3f, 0.0f, 0.0f);

	for(int32 i = 0; i < 4; i++)
	{
		s_aWheelSpheres[i].m_data.m_nSurfaceType = SURFACE_TYPE_WHEELBASE;
		s_aWheelSpheres[i].m_data.m_nPieceType = 0;
	}*/

	ms_colModelWheel1.AllocateData(2, 0, 0, 0, 0);
	pColData = ms_colModelWheel1.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.35f, CVector( -0.3f, 0.0f, 0.0f), SURFACE_TYPE_WHEELBASE, 0);
	pColData->m_pSphereArray[1].Set(0.35f, CVector( 0.3f, 0.0f, 0.0f), SURFACE_TYPE_WHEELBASE, 0);

	ms_colModelWheel1.GetBoundSphere().Set(1.4f, CVector(0.0f, 0.0f, 0.0f));
	ms_colModelWheel1.GetBoundBox().Set(CVector(-0.7f, -0.4f, -0.4f), CVector(0.7f, 0.4f, 0.4f));
	ms_colModelWheel1.m_level = LEVEL_GENERIC;

///////////////////////////////////////////////
//	Leg bodypart collision model
/*	s_aBodyPartSpheres1[0].m_fRadius = 0.2f;
	s_aBodyPartSpheres1[1].m_fRadius = 0.2f;

	s_aBodyPartSpheres1[0].m_vecCentre = CVector( 0.0f, 0.0f, 0.0f);
	s_aBodyPartSpheres1[1].m_vecCentre = CVector( 0.8f, 0.0f, 0.0f);

	for(int32 i = 0; i < 4; i++)
	{
		s_aBodyPartSpheres1[i].m_data.m_nSurfaceType = SURFACE_TYPE_PED; //COLPOINT_SURFACETYPE_WHEELBASE;
		s_aBodyPartSpheres1[i].m_data.m_nPieceType = 0;
	}*/

	ms_colModelBodyPart1.AllocateData(2, 0, 0, 0, 0);
	pColData = ms_colModelBodyPart1.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.2f, CVector( 0.0f, 0.0f, 0.0f), SURFACE_TYPE_PED, 0);
	pColData->m_pSphereArray[1].Set(0.2f, CVector( 0.8f, 0.0f, 0.0f), SURFACE_TYPE_PED, 0);

	ms_colModelBodyPart1.GetBoundSphere().Set(0.7f, CVector(0.4f, 0.0f, 0.0f));
	ms_colModelBodyPart1.GetBoundBox().Set(CVector(-0.3f, -0.3f, -0.3f), CVector(1.1f, 0.3f, 0.3f));
	ms_colModelBodyPart1.m_level = LEVEL_GENERIC;

///////////////////////////////////////////////
//	Arm bodypart collision model
/*	s_aBodyPartSpheres2[0].m_fRadius = 0.15f;
	s_aBodyPartSpheres2[1].m_fRadius = 0.15f;

	s_aBodyPartSpheres2[0].m_vecCentre = CVector( 0.0f, 0.0f, 0.0f);
	s_aBodyPartSpheres2[1].m_vecCentre = CVector( 0.5f, 0.0f, 0.0f);

	for(int32 i = 0; i < 4; i++)
	{
		s_aBodyPartSpheres2[i].m_data.m_nSurfaceType = SURFACE_TYPE_PED; //COLPOINT_SURFACETYPE_WHEELBASE;
		s_aBodyPartSpheres2[i].m_data.m_nPieceType = 0;
	}*/

	ms_colModelBodyPart2.AllocateData(2, 0, 0, 0, 0);
	pColData = ms_colModelBodyPart2.GetCollisionData();

	pColData->m_pSphereArray[0].Set(0.15f, CVector( 0.0f, 0.0f, 0.0f), SURFACE_TYPE_PED, 0);
	pColData->m_pSphereArray[1].Set(0.15f, CVector( 0.5f, 0.0f, 0.0f), SURFACE_TYPE_PED, 0);

	ms_colModelBodyPart2.GetBoundSphere().Set(0.5f, CVector(0.25f, 0.0f, 0.0f));
	ms_colModelBodyPart2.GetBoundBox().Set(CVector(-0.2f, -0.2f, -0.2f), CVector(0.7f, 0.2f, 0.2f));
	ms_colModelBodyPart2.m_level = LEVEL_GENERIC;

//////////////////////////////////////////////
// Weapon collision model
	ms_colModelWeapon.SetBoundRadius(0.25f);
	ms_colModelWeapon.SetBoundBoxMin(CVector(-0.25f, -0.25f, -0.25f));
	ms_colModelWeapon.SetBoundBoxMax(CVector(0.25f, 0.25f, 0.25f));
	
}

//
//
// HandlingDataMgr
//
//
#include "handlingMgr.h"

extern char VehicleNames[VT_MAX][MAX_KEYWORD_LENGTH];

// Setup handling data
//
void cHandlingDataMgr::Initialise()
{
	// load data from handling file
	LoadHandlingData();

	// SETUP VALUES SHARED BY ALL VEHICLES....

	// shared value between all vehicles
	m_fCoefficientOfRestitution = 0.1f;

	// shared value between all vehicles
	m_fWheelFriction = 0.9f;

	// setup terrain frition coefficient values
/*	Using surface table instead
	m_aTerrainFriction[TT_DRY_ROAD] = 0.7f;
	m_aTerrainFriction[TT_GRASS] = 0.5f;
	m_aTerrainFriction[TT_DIRT] = 0.5f;
	m_aTerrainFriction[TT_WET_ROAD] = 0.6f;
	m_aTerrainFriction[TT_WET_GRASS] = 0.4f;
	m_aTerrainFriction[TT_MUD] = 0.3f;
	m_aTerrainFriction[TT_OILY] = 0.3f;
	m_aTerrainFriction[TT_SNOW] = 0.35f;
	m_aTerrainFriction[TT_ICE] = 0.1f;*/

	// resistances, i.e. air, water 
	m_aResistanceTable[RT_AIR] = 1.0f;
	m_aResistanceTable[RT_WATER] = 0.8f;
	m_aResistanceTable[RT_ROLLING] = 0.98f;

}


// loads handling config file into memory
//
void cHandlingDataMgr::LoadHandlingData()
{
	int32 fid;

	CFileMgr::SetDir("DATA");
	fid = CFileMgr::OpenFile((char *)"HANDLING.CFG", "rb");
	ASSERTMSG(fid != 0, "Cannot find HANDLING.CFG");
	
	CFileMgr::SetDir("");

	int veh=0;
	char* pLine;
	tHandlingData* pHand = NULL;
	tBikeHandlingData *pBikeHand = NULL;
	tFlyingHandlingData *pFlyingHand = NULL;
	tBoatHandlingData *pBoatHand = NULL;
	
	// read a line at a time putting vlues into the handling data structure
	while((pLine = CFileLoader::LoadLine(fid)) != NULL)
	{
		// check for end of file - don't yet know how to check the file length (TBD)
		if (!strcmp(pLine, ";the end"))
			break;
		// ignore comments
		if (pLine[0] == ';')
			continue;

		// parse line of file which contains data
		
		// if starts '!' then load as bike handling data
		if(pLine[0] == '!')
		{
			uint8 count = 0;
			char seps[] = " \t";
			char *token = NULL;
			token = strtok(pLine, seps);	// get first token
			do {
			
				switch(count)
				{
					case 0:
						break;
						
					case 1:
						veh = FindExactWord(token, (char *)VehicleNames, MAX_KEYWORD_LENGTH, VT_MAX);
						// error condition, veh should be less than MAX_VEHICLE_TYPES
						ASSERTMSG(veh<VT_MAX,"Bad car handling data");
						pBikeHand =	&m_aBikeHandlingData[veh - VT_BIKE];

						pBikeHand->nVehicleID = (tVehicleType)veh;
						break;
						
					case 2:
						pBikeHand->fLeanFwdCOMMult = atof(token);
						break;
					
					case 3:
						pBikeHand->fLeanFwdForceMult = atof(token);
						break;
					
					case 4:
						pBikeHand->fLeanBakCOMMult = atof(token);
						break;
					
					case 5:
						pBikeHand->fLeanBakForceMult = atof(token);
						break;
					
					case 6:
						pBikeHand->fMaxBankAngle = atof(token);
						break;
					
					case 7:
						pBikeHand->fFullAnimAngle = atof(token);
						break;
					
					case 8:
						pBikeHand->fDesLeanReturnFrac = atof(token);
						break;
					
					case 9:
						pBikeHand->fSpeedSteerFrac = atof(token);
						break;
						
					case 10:
						pBikeHand->fSlippySpeedSteerMult = atof(token);
						break;
					
					case 11:
						pBikeHand->fNoRiderCOMz = atof(token);
						break;
					
					case 12:
						pBikeHand->fWheelieBalancePoint = atof(token);
						break;

					case 13:
						pBikeHand->fStoppieBalancePoint = atof(token);
						break;
						
					case 14:
						pBikeHand->fWheelieSteerMult = atof(token);
						break;

					case 15:
						pBikeHand->fRearBalanceMult = atof(token);
						break;

					case 16:
						pBikeHand->fFrontBalanceMult = atof(token);
						break;
				
				}
				token = strtok( NULL, seps );	// get next token
				count++;

			} while (token != NULL);
			
			ConvertBikeDataToGameUnits(pBikeHand);
		}
		// if if line begins '$' load as flying handling data
		else if(pLine[0] == '$')
		{
			uint8 count = 0;
			char seps[] = " \t";
			char *token = NULL;
			token = strtok(pLine, seps);	// get first token
			do {
			
				switch(count)
				{
					case 0:
						break;
						
					case 1:
						veh = FindExactWord(token, (char *)VehicleNames, MAX_KEYWORD_LENGTH, VT_MAX);
						// error condition, veh should be less than MAX_VEHICLE_TYPES
						ASSERTMSG(veh<VT_MAX,"Bad car handling data");
						pFlyingHand = &m_aFlyingHandlingData[veh - VT_SEAPLANE];

						pFlyingHand->nVehicleID = (tVehicleType)veh;
						break;
					// Thrust
					case 2:
						pFlyingHand->fThrust = atof(token);
						break;
					case 3:
						pFlyingHand->fThrustFallOff = atof(token);
						break;
					// Yaw
					case 4:
						pFlyingHand->fYawMult = atof(token);
						break;
					case 5:
						pFlyingHand->fYawStabilise = atof(token);
						break;
					case 6:
						pFlyingHand->fSideSlipMult = atof(token);
						break;
					// Roll
					case 7:
						pFlyingHand->fRollMult = atof(token);
						break;
					case 8:
						pFlyingHand->fRollStabilise = atof(token);
						break;
					// Pitch
					case 9:
						pFlyingHand->fPitchMult = atof(token);
						break;
					case 10:
						pFlyingHand->fPitchStabilise = atof(token);
						break;
					// Lift
					case 11:
						pFlyingHand->fFormLiftMult = atof(token);
						break;
					case 12:
						pFlyingHand->fAttackLiftMult = atof(token);
						break;
					// special mults
					case 13:
						pFlyingHand->fGearUpResMult = atof(token);
						break;
					case 14:
						pFlyingHand->fGearDownLiftMult = atof(token);
						break;
					case 15:
						pFlyingHand->fWindMult = atof(token);
						break;
					// Move Resistance
					case 16:
						pFlyingHand->fMoveRes = atof(token);
						break;
					// Turn Resistance
					case 17:
						pFlyingHand->vecTurnRes.x = atof(token);
						break;
					case 18:
						pFlyingHand->vecTurnRes.y = atof(token);
						break;
					case 19:
						pFlyingHand->vecTurnRes.z = atof(token);
						break;
					// TurnSpeed Based Damping
					case 20:
						pFlyingHand->vecSpeedRes.x = atof(token);
						break;
					case 21:
						pFlyingHand->vecSpeedRes.y = atof(token);
						break;
					case 22:
						pFlyingHand->vecSpeedRes.z = atof(token);
						break;
				}
				token = strtok( NULL, seps );	// get next token
				count++;

			} while (token != NULL);
		}
		// if if line begins '%' load as boat handling data
		else if(pLine[0] == '%')
		{
			uint8 count = 0;
			char seps[] = " \t";
			char *token = NULL;
			token = strtok(pLine, seps);	// get first token
			do {
			
				switch(count)
				{
					case 0:
						break;
						
					case 1:
						veh = FindExactWord(token, (char *)VehicleNames, MAX_KEYWORD_LENGTH, VT_MAX);
						// error condition, veh should be less than MAX_VEHICLE_TYPES
						ASSERTMSG(veh<VT_MAX,"Bad car handling data");
						pBoatHand = GetBoatPointer(veh);

						pBoatHand->nVehicleID = (tVehicleType)veh;
						break;
					// Thrust
					case 2:
						pBoatHand->fThrustOffsetY = atof(token);
						break;
					case 3:
						pBoatHand->fThrustOffsetZ = atof(token);
						break;
					case 4:
						pBoatHand->fThrustAppOffsetZ = atof(token);
						break;
					// Aquaplane 
					case 5:
						pBoatHand->fAquaplaneForce = atof(token);
						break;
					case 6:
						pBoatHand->fAquaplaneLimit = atof(token);
						break;
					case 7:
						pBoatHand->fAquaplaneOffsetY = atof(token);
						break;
					case 8:
						pBoatHand->fWaveAudioMult = atof(token);
						break;
					// Move Resistance
					case 9:
						pBoatHand->vecMoveResistance.x = atof(token);
						break;
					case 10:
						pBoatHand->vecMoveResistance.y = atof(token);
						break;
					case 11:
						pBoatHand->vecMoveResistance.z = atof(token);
						break;
					// Turn Resistance
					case 12:
						pBoatHand->vecTurnResistance.x = atof(token);
						break;
					case 13:
						pBoatHand->vecTurnResistance.y = atof(token);
						break;
					case 14:
						pBoatHand->vecTurnResistance.z = atof(token);
						break;
					case 15:
						pBoatHand->Look_L_R_CamHeight = atof(token);
						break;
				}
				token = strtok( NULL, seps );	// get next token
				count++;

			} while (token != NULL);
		}
		// if if line begins '^' load as anim group data
		else if(pLine[0] == '^')
		{
			uint8 count = 0;
			char seps[] = " \t";
			char *token = NULL;
			token = strtok(pLine, seps);	// get first token
			
			int flag;
			
			int index;
			int firstGroup;
			int secondGroup;
			int flags=0;
			int specialFlags=0;
			float criticalAnimTimes[CVehicleAnimGroup::MAX_INOUT];
			float processDoorStartTimes[CVehicleAnimGroup::MAX_DOOR_TIMES];
			float processDoorStopTimes[CVehicleAnimGroup::MAX_DOOR_TIMES];			

//!PC - hm, PC compiler generates errors here but PS2 one doesn't			
#if defined (MSVC_COMPILER) || defined(__GNUC__)
#define PC_CAST(x)	(int)(x)
#else //GTA_PC
#define PC_CAST(x)	(x)
#endif //GTA_PC

			do {
			
				switch(count)
				{
					case 0:
						break;
					case 1:
						index=atoi(token);
						break;						
					case 2:
						firstGroup=atoi(token);
						break;
					case 3:
						secondGroup=atoi(token);
						break;
					case 4:	//ALIGN
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_ALIGN));	//left
						}
						break;
					case 5:	//OPENOUTSIDE_FRONT
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_OPEN_OUTSIDE_FRONT));	//left
						}
						break;
					case 6:	//OPENOUTSIDE_REAR
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_OPEN_OUTSIDE_REAR));	//left
						}
						break;
					case 7:	//GETIN_FRONT
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_TYPE_GETIN_FRONT));	//left
						}
						break;
					case 8:	//GETIN_REAR
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_TYPE_GETIN_REAR));	//left
						}
						break;
					case 9:	//JACK
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_PULLOUT));	//left
						}
						break;
					case 10://CLOSEINSIDE_FRONT
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_CLOSE_INSIDE_FRONT));	//left
						}
						break;
					case 11://CLOSEINSIDE_REAR
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_CLOSE_INSIDE_REAR));	//left
						}
						break;
					case 12://SHUFFLE
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_SHUFFLE));	//left
						}
						break;
					case 13://GETOUT_FRONT
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_GETOUT_FRONT));	//left
						}
						break;
					case 14://GETOUT_REAR
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_GETOUT_REAR));	//left
						}
						break;
					case 15://BEJACKED
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_JACKED));	//left
						}
						break;
					case 16://CLOSE OUTSIDE FRONT
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_CLOSE_OUTSIDE_FRONT));	//left		
						}
						break;
					case 17://CLOSE OUTSIDE REAR
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_CLOSE_OUTSIDE_REAR));	//left		
						}
						break;
					case 18://JUMPOUT
						flag=atoi(token);
						if(flag) 
						{
							flags |=  PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_JUMPOUT));	//left
						}
						break;
					case 19://CLOSEROLL
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_CLOSE_DOOR_ROLLING));	//left
						}
						break;
					case 20://FALL OUT
						flag=atoi(token);
						if(flag)
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_FALL_OUT));	//left							
						}
						break;
					case 21://OPEN LOCKED
						flag=atoi(token);
						if(flag) 
						{
							flags |= PC_CAST(CMaths::Pow(2,ANIM_VEH_TYPE_OPEN_OUTSIDE_LOCKED));	//left
						}
						break;					
						
					case 22://GET_IN THRESHOLD TIME
						criticalAnimTimes[CVehicleAnimGroup::GET_IN]=atof(token);
						break;						
					case 23://JUMP_OUT THRESHOLD TIME
						criticalAnimTimes[CVehicleAnimGroup::JUMP_OUT]=atof(token);
						break;
					case 24://GET_OUT THRESHOLD TIME
						criticalAnimTimes[CVehicleAnimGroup::GET_OUT]=atof(token);
						break;
					case 25://JACKED_THRESHOLD_TIME
						criticalAnimTimes[CVehicleAnimGroup::JACKED_OUT]=atof(token);
						break;
					case 26://FALL_OUT_THRESHOLD_TIME
						criticalAnimTimes[CVehicleAnimGroup::FALL_OUT]=atof(token);
						break;						
	
					case 27://OPENDOOROUTSIDE START
						processDoorStartTimes[CVehicleAnimGroup::DOOR_TIME_OPEN_OUTSIDE]=atof(token);
						break;						
					case 28://OPENDOOROUTSIDE STOP
						processDoorStopTimes[CVehicleAnimGroup::DOOR_TIME_OPEN_OUTSIDE]=atof(token);
						break;					
					case 29://CLOSEINSIDE START
						processDoorStartTimes[CVehicleAnimGroup::DOOR_TIME_CLOSE_INSIDE]=atof(token);
						break;					
					case 30://CLOSEINSIDE STOP
						processDoorStopTimes[CVehicleAnimGroup::DOOR_TIME_CLOSE_INSIDE]=atof(token);
						break;												
					case 31://GETIN START
						processDoorStartTimes[CVehicleAnimGroup::DOOR_TIME_OPEN_INSIDE]=atof(token);
						break;						
					case 32://GETIN STOP
						processDoorStopTimes[CVehicleAnimGroup::DOOR_TIME_OPEN_INSIDE]=atof(token);
						break;					
					case 33://CLOSEOUTSIDE START
						processDoorStartTimes[CVehicleAnimGroup::DOOR_TIME_CLOSE_OUTSIDE]=atof(token);
						break;					
					case 34://CLOSEOUTSIDE STOP
						processDoorStopTimes[CVehicleAnimGroup::DOOR_TIME_CLOSE_OUTSIDE]=atof(token);
						break;
	
					case 35://SPECIAL FLAGS
						specialFlags=atoi(token);
						break;	
											
					default:
						ASSERT(false);
						break;
				}
				token = strtok( NULL, seps );	// get next token
				count++;			

			} while (token != NULL);
			
#ifdef USE_ANIM_GROUPS
			CVehicleAnimGroup v;
			v.Set(firstGroup+(int)ANIM_VEH_STD,secondGroup+(int)ANIM_VEH_STD,flags,specialFlags,criticalAnimTimes,processDoorStartTimes,processDoorStopTimes);
			CVehicleAnimGroupData::Set(index,v);	
#endif

		}
		// else load as car handling data
		else
		{

			uint8 count = 0;
			char seps[] = " \t";
			char *token = NULL;
			token = strtok(pLine, seps);	// get first token
			do {
			
				switch(count)
				{
					case 0:
						veh = FindExactWord(token, (char *)VehicleNames, MAX_KEYWORD_LENGTH, VT_MAX);

						// error condition, veh should be less than MAX_VEHICLE_TYPES
						//ASSERT(veh < VT_MAX);
						ASSERTOBJ(veh<VT_MAX, token,"Bad car handling data");
						pHand =	&m_aHandlingData[veh];

						// 'nVehicleID' probably not necessary - good for debug purposes though
						//ASSERT(veh < VT_MAX);
						ASSERTMSG(veh<VT_MAX,"Bad car handling data");
						pHand->nVehicleID = (tVehicleType)veh;
					break;
					
					case 1:
						pHand->fMass = atof(token);
					break;
					
					case 2:
						pHand->fTurnMass = atof(token);
					break;
					
					case 3:
						pHand->fDragCoeff = atof(token);
					break;
					
					case 4:
						pHand->CentreOfMass.x = atof(token);
					break;
					
					case 5:
						pHand->CentreOfMass.y = atof(token);
					break;
					
					case 6:
						pHand->CentreOfMass.z = atof(token);
					break;
					
					case 7:
						pHand->nPercentSubmerged = atoi(token);
					break;
					
					case 8:
						pHand->fTractionMultiplier = atof(token);
					break;

					case 9:
						pHand->fTractionLoss = atof(token);
					break;

					case 10:
						pHand->fTractionBias = atof(token);
					break;
					
					case 11:
						pHand->Transmission.m_nNumberOfGears = atoi(token);
					break;
					
					case 12:
						pHand->Transmission.m_fMaxVelocity = atof(token);
					break;
					
					case 13:
						pHand->Transmission.m_fEngineAcceleration = 0.4*atof(token);
					break;
					
					case 14:
						pHand->Transmission.m_fEngineInertia = atof(token);
					break;
					
					case 15:
						pHand->Transmission.m_nDriveType = token[0];
					break;
					
					case 16:
						pHand->Transmission.m_nEngineType = token[0];
					break;
					
					case 17:
						pHand->fBrakeDeceleration = atof(token);
					break;

					case 18:
						pHand->fBrakeBias = atof(token);
					break;
					
					case 19:
						pHand->bABS = (bool)atoi(token);
					break;
					
					case 20:
						pHand->fSteeringLock = atof(token);
					break;
						
					case 21:
						pHand->fSuspensionForce = atof(token);
					break;

					case 22:
						pHand->fSuspensionDamping = atof(token);
					break;
					
					case 23:
						pHand->fSuspensionHighSpdComDamp = atof(token);
					break;

					case 24:
						pHand->fSuspensionUpperLimit = atof(token);
					break;

					case 25:
						pHand->fSuspensionLowerLimit = atof(token);
					break;

					case 26:
						pHand->fSuspensionBias = atof(token);
					break;
					
					case 27:
						pHand->fSuspensionAntiDive = atof(token);
					break;

					case 28:
						pHand->fSeatOffsetDist = atof(token);
					break;

					case 29:
						pHand->fCollisionDamageMultiplier = atof(token);
					break;

					case 30:
						pHand->nMonetaryValue = atoi(token);
					break;

					case 31:
						sscanf(token, "%x", &pHand->mFlags);
					break;
					
					case 32:
						sscanf(token, "%x", &pHand->hFlags);
						// TEST (need to sort this out)
						pHand->Transmission.m_nFlags = pHand->hFlags;
					break;

					case 33:
						pHand->HeadLightType = atoi(token);
					break;

					case 34:
						pHand->RearLightType = atoi(token);
					break;
					
					case 35:
						pHand->AnimGroup = atoi(token);
						break;

				}			

				token = strtok( NULL, seps );	// get next token
				
				count++;
//				if (count>31)
//					count = 0;

			} while (token != NULL);
			
		
			// calc additional values after loading data from handling config file
			ConvertDataToGameUnits(pHand);

#if defined (GTA_PS2)
#ifdef _DEBUG
			// perform range checking to ensure data integrity
			RangeCheck(pHand);
#endif
#endif //GTA_PS2

		}
		
	}

	CFileMgr::CloseFile(fid);
}


//
//
// CSurfaceTable
//
//
/*
#include "SurfaceTable.h"

extern float CSurfaceTable::ms_aAdhesiveLimitTable[ADHESION_GROUP_LAST * ADHESION_GROUP_LAST];


void CSurfaceTable::Initialise(char* pFilename)
{
	char* pLine;
	int32 fid;

	CFileMgr::SetDir("");
	fid = CFileMgr::OpenFile(pFilename, "rb");
	ASSERT(fid > 0);
	
	int32 j=0;
	while ((pLine = CFileLoader::LoadLine(fid)))
	{
		if(*pLine == ';' || *pLine == '\0')
			continue;

		// read surface name
		char surfacename[256];
		sscanf(pLine, "%s", &surfacename);

		// skip over surface name
		while (*pLine != ' ' && *pLine != '\t')
			pLine++;

		for (int32 i = 0; i <= j; i++)
		{
			// skip commas, tabs and spaces
			while (*pLine == ' ' || *pLine == '\t')
				pLine++;

			// read adhesive limit
			float fAdhesiveLimit = 0.0f;
			if (*pLine != '-')
				sscanf(pLine, "%f", &fAdhesiveLimit);

			// skip entry
			while (*pLine != ' ' && *pLine != '\t' && *pLine != '\0')
				pLine++;

			ms_aAdhesiveLimitTable[i + j * ADHESION_GROUP_LAST] = fAdhesiveLimit;
			ms_aAdhesiveLimitTable[j + i * ADHESION_GROUP_LAST] = fAdhesiveLimit;
		}
		j++;
	}
	ASSERT(j == ADHESION_GROUP_LAST);
	
	CFileMgr::CloseFile(fid);
	
	// Setup see thru, shoot thru arrays
	for(j=0; j<COLPOINT_SURFACETYPE_LAST; j++)
	{
		if(IsSeeThroughInternal(j))
			ms_bIsSeeThru[j] = true;
		else
			ms_bIsSeeThru[j] = false;
		if(IsShootThroughInternal(j))
			ms_bIsShootThru[j] = true;
		else
			ms_bIsShootThru[j] = false;
	}
}
*/


//
//
// from PedType.cpp
//
//
#include "PedType.h"
#include "PedIntelligence.h"
#include "MemInfo.h"

void CPedStats::Initialise()
{
	DEBUGLOG("Initialising CPedStats...\n");

	ms_apPedStats = new CPedStats[PEDSTAT_NUM];
	
	for (uint16 i=0; i<PEDSTAT_NUM; i++)
	{
		ms_apPedStats[i].m_ePedStatType = PEDSTAT_PLAYER;
#if !defined (MSVC_COMPILER) && !defined(__GNUC__)
		ms_apPedStats[i].m_sPedStatName[8] = 'PLAYER';	//copies 'P' into eighth character of string?!?
#else
        strcpy(ms_apPedStats[i].m_sPedStatName, "PLAYER");
#endif
		ms_apPedStats[i].m_fFleeDistance = 20.0f;
		ms_apPedStats[i].m_fMaxHeadingChange = 15.0f;
		ms_apPedStats[i].m_nFear = 50;
		ms_apPedStats[i].m_nTemper = 50;
		ms_apPedStats[i].m_nLawfulness = 50;
		ms_apPedStats[i].m_nSexiness = 50;
		ms_apPedStats[i].m_fAttackMult = 1.0f;
		ms_apPedStats[i].m_fDefendMult = 1.0f;
		ms_apPedStats[i].m_nStatFlags = 0;
		ms_apPedStats[i].m_iDefaultDecisionMaker = 0;
	}

	DEBUGLOG("Loading pedstats data...\n");

	LoadPedStats();

	CMemInfo::EnterBlock(TASKEVENT_MEM_ID);	

	CDecisionMakerTypesFileLoader::LoadDefaultDecisionMaker();

	CMemInfo::ExitBlock();	

	DEBUGLOG("CPedStats ready\n");
} // end - CPedStats::Initialise


void CPedStats::LoadPedStats(void)
{
	char* pLine;
	int32 fid;
	char aString[32];	
	int32 nFear,nTemper,nLawfulness,nSexiness,nFlags, nDefaultDecisionMaker;
	float fFleeDist,fHeadingRate,fAttackMult,fDefendMult;

	ePedStats CurrentPedStat = PEDSTAT_NUM;

	fid = CFileMgr::OpenFile("DATA\\PEDSTATS.DAT");
	ASSERTMSG(fid != 0, "Problem loading pedstats.dat");

	// keep going till we reach the end of the file
	uint32 i=0;
	while ((pLine = CFileLoader::LoadLine(fid)))
	{
		// ignore lines starting with # as they are comments
		if(pLine[0] == '#' || pLine[0] == '\0')
			continue;
			
		sscanf(pLine, "%s %f %f %d %d %d %d %f %f %d %d", 
				aString, &fFleeDist, &fHeadingRate, 
				&nFear, &nTemper, &nLawfulness, &nSexiness, 
				&fAttackMult, &fDefendMult, &nFlags, &nDefaultDecisionMaker);
		
		ms_apPedStats[i].m_ePedStatType = ePedStats(i);
		strcpy(ms_apPedStats[i].m_sPedStatName, aString);
		ms_apPedStats[i].m_fFleeDistance = fFleeDist;
		ms_apPedStats[i].m_fMaxHeadingChange = fHeadingRate;
		ms_apPedStats[i].m_nFear = nFear;
		ms_apPedStats[i].m_nTemper = nTemper;
		ms_apPedStats[i].m_nLawfulness = nLawfulness;
		ms_apPedStats[i].m_nSexiness = nSexiness;
		ms_apPedStats[i].m_fAttackMult = fAttackMult;
		ms_apPedStats[i].m_fDefendMult = fDefendMult;
		ms_apPedStats[i].m_nStatFlags = nFlags;
		ms_apPedStats[i].m_iDefaultDecisionMaker = nDefaultDecisionMaker;

		i++;
	}
	
	CFileMgr::CloseFile(fid);
	
} // end - CPedType::LoadPedData

//
//
// from PedIntelligence.cpp
//
//
void CDecisionMakerTypes::LoadEventIndices(int* pMap, const char* pName)
{	
	//Initialise all the entries.
	int i;
	for(i=0;i<CEventTypes::MAX_NUM_EVENT_TYPES;i++)
	{
		pMap[i]=0;
	}
	
	CFileMgr::SetDir("");	
	CFileMgr::SetDir("data\\decision\\");
	const int32 fileID = CFileMgr::OpenFile(pName);
	CFileMgr::SetDir("");
	ASSERT(fileID>0);
	
	int index=0;
	char line[256];
	while(CFileMgr::ReadLine(fileID, line))
	{
		if(line[0]!='\0'&&line[0]!='\n')
		{
			//Read the events.
			int iEventType;
			char eventName[256];
			sscanf(line, "%s %d", eventName, &iEventType);
			pMap[iEventType]=index;
			index++;	
		}
	}
	
	CFileMgr::CloseFile(fileID);
}

void CDecisionMakerTypesFileLoader::LoadDefaultDecisionMaker()
{	
	CDecisionMakerTypes::GetDecisionMakerTypes()->LoadEventIndices();

 	//unload any decision makers that were left in memory
	for(int8 i = 0; i < CDecisionMakerTypes::MAX_NUM_DECISION_MAKER_TYPES; i++)
	{
		UnloadDecisionMaker(i);
	}	

	CDecisionMaker d;
	{
		d.SetDefault();
		LoadDecisionMaker("RANDOM.ped",d);
		CDecisionMakerTypes::GetDecisionMakerTypes()->SetDefaultRandomPedDecisionMaker(d);
	}
	
	{
		d.SetDefault();
		LoadDecisionMaker("m_norm.ped",d);
		CDecisionMakerTypes::GetDecisionMakerTypes()->SetDefaultMissionPedDecisionMaker(d);
	}
	
	{
		d.SetDefault();
		LoadDecisionMaker("m_plyr.ped",d);
		CDecisionMakerTypes::GetDecisionMakerTypes()->SetDefaultPlayerPedDecisionMaker(d);
	}
	
	
	{
		d.SetDefault();
		LoadDecisionMaker("RANDOM.grp",d);
		CDecisionMakerTypes::GetDecisionMakerTypes()->SetDefaultRandomPedGroupDecisionMaker(d);
	}
	
	{
		d.SetDefault();
		LoadDecisionMaker("MISSION.grp",d);
		CDecisionMakerTypes::GetDecisionMakerTypes()->SetDefaultMissionPedGroupDecisionMaker(d);
	}
	
	//First 8 slots for random peds.
	LoadDecisionMaker("GangMbr.ped",0);	// 0
	LoadDecisionMaker("Cop.ped",0);		// 1
	LoadDecisionMaker("R_Norm.ped",0);	// 2
	LoadDecisionMaker("R_Tough.ped",0);	// 3
	LoadDecisionMaker("R_Weak.ped",0);	// 4
	LoadDecisionMaker("Fireman.ped",0);	// 5
	LoadDecisionMaker("m_empty.ped",0);	// 6
	LoadDecisionMaker("Indoors.ped",0);	// 7
	//Next two slots for random groups.
	LoadDecisionMaker("RANDOM.grp",1); 	// 8
	LoadDecisionMaker("RANDOM2.grp",1);	// 9
	//Remaining slots reserved for special decision makers.
}


//
//
// from TimeCycle.cpp
//
//
#include "timecycle.h"

#define DIRN_LIGHTING_SUN_ORIENTATION (-135.0f)
#define DIRN_LIGHTING_SUN_ELEVEATION (45.0f)

// Name			:	Initialise
// Purpose		:	Inits CTimeCycle object
// Parameters	:	None
// Returns		:	Nothing

void CTimeCycle::Initialise(bool bPalFile)
{
	int32 nAmbR, nAmbG, nAmbB, nAmbR_Obj, nAmbG_Obj, nAmbB_Obj, nSkyTopR, nSkyTopG, nSkyTopB, nSkyBotR, nSkyBotG, nSkyBotB;
	int32 nDirR, nDirG, nDirB, nSunCoreR, nSunCoreG, nSunCoreB, nSunCoronaR, nSunCoronaG, nSunCoronaB;
	float fSunSize, fSpriteSize, fSpriteBrightness, fFarClip, fFogStart, fLightsOnGroundBrightness;
	Int32 nShadowStrength, nLightShadowStrength, nPoleShadowStrength, nCloudR, nCloudG, nCloudB;
	Int32 nFluffBotR, nFluffBotG, nFluffBotB, nHighLightMinIntensity, nWaterFogAlpha;
	float fWaterRed, fWaterGreen, fWaterBlue, fWaterAlpha;
	float fPostFx1Red, fPostFx1Green, fPostFx1Blue, fPostFx1Alpha;
	float fPostFx2Red, fPostFx2Green, fPostFx2Blue, fPostFx2Alpha;
	float fCloudAlpha, fDirLightMult;
	int32 fid;
	int i = 0;
	Int16	Weathers;
	char* pLine;

//	m_NumBoxes = 0;	// Don't comment this in. Boxes don't get re-initialised.

	DEBUGLOG("Intialising CTimeCycle...\n");
	CFileMgr::SetDir("DATA");
#ifdef GTA_PC
	fid = CFileMgr::OpenFile("TIMECYC.DAT", "rb");
#else

#ifdef MASTER
#ifdef PAL_BUILD
	fid = CFileMgr::OpenFile("TIMECYCP.DAT", "rb");
#else
	fid = CFileMgr::OpenFile("TIMECYC.DAT", "rb");
#endif
#else
	if (bPalFile)
	{
		fid = CFileMgr::OpenFile("TIMECYCP.DAT", "rb");
	}
	else
	{
		fid = CFileMgr::OpenFile("TIMECYC.DAT", "rb");
	}
#endif	
	
#ifndef MASTER
	m_bPalFileLoaded = bPalFile;
#endif
#endif	// ifdef GTA_PC
	ASSERT(fid != 0);
	CFileMgr::SetDir("");

	for (Weathers = 0; Weathers < TIMECYCLE_NUMWEATHERTYPES; Weathers++)
	{
		for (i = 0; i < TIMECYCLE_SUBDIVISIONS; i++)
		{
			while ((pLine = CFileLoader::LoadLine(fid)))
			{
				if(*pLine != '/' && *pLine != '\0')
					break;
			}
			ASSERTMSG(pLine, "Timecycle file is missing an entry");

			// now extract the anim data from the string
			sscanf(pLine, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f %f %f %d %d %d %f %f %f %d %d %d %d %d %d %f %f %f %f  %f %f %f %f  %f %f %f %f %f %d %d %f", 
				&nAmbR, &nAmbG, &nAmbB, &nAmbR_Obj, &nAmbG_Obj, &nAmbB_Obj, &nDirR, &nDirG, &nDirB, &nSkyTopR, &nSkyTopG, &nSkyTopB, &nSkyBotR, &nSkyBotG, &nSkyBotB, &nSunCoreR, &nSunCoreG, &nSunCoreB, &nSunCoronaR, &nSunCoronaG, &nSunCoronaB, &fSunSize,
				&fSpriteSize, &fSpriteBrightness, &nShadowStrength, &nLightShadowStrength, &nPoleShadowStrength, &fFarClip, &fFogStart,
				&fLightsOnGroundBrightness, &nCloudR, &nCloudG, &nCloudB,
				&nFluffBotR, &nFluffBotG, &nFluffBotB,
				&fWaterRed, &fWaterGreen, &fWaterBlue, &fWaterAlpha,
				&fPostFx1Alpha, &fPostFx1Red, &fPostFx1Green, &fPostFx1Blue, 
				&fPostFx2Alpha, &fPostFx2Red, &fPostFx2Green, &fPostFx2Blue,
				&fCloudAlpha, &nHighLightMinIntensity, &nWaterFogAlpha, &fDirLightMult);

			m_nAmbientRed[i][Weathers] = nAmbR;
			m_nAmbientGreen[i][Weathers] = nAmbG;
			m_nAmbientBlue[i][Weathers] = nAmbB;
			m_nAmbientRed_Obj[i][Weathers] = nAmbR_Obj;
			m_nAmbientGreen_Obj[i][Weathers] = nAmbG_Obj;
			m_nAmbientBlue_Obj[i][Weathers] = nAmbB_Obj;
//			m_nDirectionalRed[i][Weathers] = nDirR;		always 255
//			m_nDirectionalGreen[i][Weathers] = nDirG;	always 255
//			m_nDirectionalBlue[i][Weathers] = nDirB;	always 255
			m_nSkyTopRed[i][Weathers] = nSkyTopR;
			m_nSkyTopGreen[i][Weathers] = nSkyTopG;
			m_nSkyTopBlue[i][Weathers] = nSkyTopB;
			m_nSkyBottomRed[i][Weathers] = nSkyBotR;
			m_nSkyBottomGreen[i][Weathers] = nSkyBotG;
			m_nSkyBottomBlue[i][Weathers] = nSkyBotB;
			m_nSunCoreRed[i][Weathers] = nSunCoreR;
			m_nSunCoreGreen[i][Weathers] = nSunCoreG;
			m_nSunCoreBlue[i][Weathers] = nSunCoreB;
			m_nSunCoronaRed[i][Weathers] = nSunCoronaR;
			m_nSunCoronaGreen[i][Weathers] = nSunCoronaG;
			m_nSunCoronaBlue[i][Weathers] = nSunCoronaB;
			m_fSunSize[i][Weathers] = (fSunSize * 10.0f + 0.5f);
			m_fSpriteSize[i][Weathers] = (fSpriteSize * 10.0f + 0.5f);
			m_fSpriteBrightness[i][Weathers] = (fSpriteBrightness * 10.0f + 0.5f);
			m_nShadowStrength[i][Weathers] = nShadowStrength;
			m_nLightShadowStrength[i][Weathers] = nLightShadowStrength;
			m_nPoleShadowStrength[i][Weathers] = nPoleShadowStrength;
			m_fFarClip[i][Weathers] = fFarClip;
			m_fFogStart[i][Weathers] = fFogStart;
			m_fLightsOnGroundBrightness[i][Weathers] = (fLightsOnGroundBrightness * 10.0f + 0.5f);
			m_nLowCloudsRed[i][Weathers] = nCloudR;
			m_nLowCloudsGreen[i][Weathers] = nCloudG;
			m_nLowCloudsBlue[i][Weathers] = nCloudB;
			m_nFluffyCloudsBottomRed[i][Weathers] = nFluffBotR;
			m_nFluffyCloudsBottomGreen[i][Weathers] = nFluffBotG;
			m_nFluffyCloudsBottomBlue[i][Weathers] = nFluffBotB;
			m_fWaterRed[i][Weathers] = fWaterRed;
			m_fWaterGreen[i][Weathers] = fWaterGreen;
			m_fWaterBlue[i][Weathers] = fWaterBlue;
			m_fWaterAlpha[i][Weathers] = fWaterAlpha;
			m_fPostFx1Red[i][Weathers] = fPostFx1Red;
			m_fPostFx1Green[i][Weathers] = fPostFx1Green;
			m_fPostFx1Blue[i][Weathers] = fPostFx1Blue;
			m_fPostFx2Red[i][Weathers] = fPostFx2Red;
			m_fPostFx2Green[i][Weathers] = fPostFx2Green;
			m_fPostFx2Blue[i][Weathers] = fPostFx2Blue;

// ps2 alpha values only go up to 128. Need to double them for same results on other platforms
#if defined (GTA_PS2)
			m_fPostFx1Alpha[i][Weathers] = fPostFx1Alpha;
			m_fPostFx2Alpha[i][Weathers] = fPostFx2Alpha;
#else
			m_fPostFx1Alpha[i][Weathers] = fPostFx1Alpha*2;
			m_fPostFx2Alpha[i][Weathers] = fPostFx2Alpha*2;
#endif //GTA_PS2

			m_fCloudAlpha[i][Weathers] = fCloudAlpha;
			m_nHighLightMinIntensity[i][Weathers] = nHighLightMinIntensity;
			m_nWaterFogAlpha[i][Weathers] = nWaterFogAlpha;
			m_nDirectionalMult[i][Weathers] = (UInt8)fDirLightMult * 100.0f;
		}
	}

	CFileMgr::CloseFile(fid);
	// stuff so Aaron can edit directional lighting direction
	m_vecDirnLightToSun.x = CMaths::Cos(DEGTORAD(DIRN_LIGHTING_SUN_ORIENTATION))*CMaths::Cos(DEGTORAD(DIRN_LIGHTING_SUN_ELEVEATION));
	m_vecDirnLightToSun.y = CMaths::Sin(DEGTORAD(DIRN_LIGHTING_SUN_ORIENTATION))*CMaths::Cos(DEGTORAD(DIRN_LIGHTING_SUN_ELEVEATION));
	m_vecDirnLightToSun.z = CMaths::Sin(DEGTORAD(DIRN_LIGHTING_SUN_ELEVEATION));
	// shouldn't really have to normalise, but..
	m_vecDirnLightToSun.Normalise();
	
	m_FogReduction = 0;
	
	m_bExtraColourOn = false;

	DEBUGLOG("CTimeCycle ready\n");
} // end - CTimeCycle::Initialise

//
//
// from PopCycle.cpp
//
//
#include "PopCycle.h"
#include "Zones.h"
#include "VarConsole.h"

// Name			:	Initialise
// Purpose		:	Inits CPopCycle object
// Parameters	:	None
// Returns		:	Nothing

void CPopCycle::Initialise()
{
//	int32 nAmbR, nAmbG, nAmbB, nAmbR_Bl, nAmbG_Bl, nAmbB_Bl, nAmbR_Obj, nAmbG_Obj, nAmbB_Obj, nAmbR_Obj_Bl, nAmbG_Obj_Bl, nAmbB_Obj_Bl, nSkyTopR, nSkyTopG, nSkyTopB, nSkyBotR, nSkyBotG, nSkyBotB;
//	int32 nDirR, nDirG, nDirB, nSunCoreR, nSunCoreG, nSunCoreB, nSunCoronaR, nSunCoronaG, nSunCoronaB;
//	float fSunSize, fSpriteSize, fSpriteBrightness, fFarClip, fFogStart, fLightsOnGroundBrightness;
//	Int32 nShadowStrength, nLightShadowStrength, nPoleShadowStrength, nCloudR, nCloudG, nCloudB;
//	Int32 nFluffBotR, nFluffBotG, nFluffBotB;
//	float fBlurRed, fBlurGreen, fBlurBlue;
//	float fWaterRed, fWaterGreen, fWaterBlue, fWaterAlpha;
	Int32	Zone, Time, TimeOfWeek;
	char* pLine;

	DEBUGLOG("Intialising CPopCycle...\n");

	CFileMgr::SetDir("DATA");
	int32 fid = CFileMgr::OpenFile("POPCYCLE.DAT", "rb");
	ASSERT(fid != 0);
	CFileMgr::SetDir("");

	for (Zone = 0; Zone < POPCYCLE_ZONETYPES; Zone++)
	{
		for (TimeOfWeek = 0; TimeOfWeek < POPCYCLE_TIMESOFWEEK; TimeOfWeek++)
		{
			for (Time = 0; Time < POPCYCLE_SUBDIVISIONS; Time++)
			{
				while ((pLine = CFileLoader::LoadLine(fid)))
				{
					if(*pLine != '/' && *pLine != '\0')
						break;
				}
				ASSERTMSG(pLine, "PopCycle file is missing an entry");

				Int32 nMaxNumPeds, nMaxNumCars, nPercDealers, nPercGang, nPercCops, nPercOther;
				Int32 nPercTypeGroup0, nPercTypeGroup1, nPercTypeGroup2, nPercTypeGroup3, nPercTypeGroup4, nPercTypeGroup5, nPercTypeGroup6, nPercTypeGroup7, nPercTypeGroup8, nPercTypeGroup9;
				Int32 nPercTypeGroup10, nPercTypeGroup11, nPercTypeGroup12, nPercTypeGroup13, nPercTypeGroup14, nPercTypeGroup15, nPercTypeGroup16, nPercTypeGroup17;

				// now extract the data from the string
				sscanf(pLine, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
					&nMaxNumPeds, &nMaxNumCars,
					&nPercDealers, &nPercGang, &nPercCops, &nPercOther,
					&nPercTypeGroup0, &nPercTypeGroup1, &nPercTypeGroup2, &nPercTypeGroup3, &nPercTypeGroup4, &nPercTypeGroup5, &nPercTypeGroup6, &nPercTypeGroup7, &nPercTypeGroup8, &nPercTypeGroup9,
					&nPercTypeGroup10, &nPercTypeGroup11, &nPercTypeGroup12, &nPercTypeGroup13, &nPercTypeGroup14, &nPercTypeGroup15, &nPercTypeGroup16, &nPercTypeGroup17);
				
				m_nMaxNumPeds[Time][TimeOfWeek][Zone] = nMaxNumPeds;
				m_nMaxNumCars[Time][TimeOfWeek][Zone] = nMaxNumCars;

				ASSERTMSG(nPercDealers<=100, "nPercDealers Should be <= 100");
				ASSERTMSG(nPercGang<=100, "nPercGang Should be <= 100");
				ASSERTMSG(nPercCops<=100, "nPercCops Should be <= 100");
				ASSERTMSG(nPercOther<=100, "nPercOther Should be <= 100");

				m_nPercDealers[Time][TimeOfWeek][Zone] = nPercDealers;
				m_nPercGang[Time][TimeOfWeek][Zone] = nPercGang;
				m_nPercCops[Time][TimeOfWeek][Zone] = nPercCops;
				m_nPercOther[Time][TimeOfWeek][Zone] = nPercOther;

				m_nPercTypeGroup[Time][TimeOfWeek][Zone][0] = nPercTypeGroup0;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][1] = nPercTypeGroup1;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][2] = nPercTypeGroup2;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][3] = nPercTypeGroup3;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][4] = nPercTypeGroup4;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][5] = nPercTypeGroup5;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][6] = nPercTypeGroup6;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][7] = nPercTypeGroup7;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][8] = nPercTypeGroup8;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][9] = nPercTypeGroup9;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][10] = nPercTypeGroup10;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][11] = nPercTypeGroup11;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][12] = nPercTypeGroup12;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][13] = nPercTypeGroup13;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][14] = nPercTypeGroup14;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][15] = nPercTypeGroup15;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][16] = nPercTypeGroup16;
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][17] = nPercTypeGroup17;
				
				// Normalise the values to 100%
				Int32	TotalVal = 0;
				for (Int32 C = 0; C < POPCYCLE_TYPEGROUPS; C++)
				{
					TotalVal += m_nPercTypeGroup[Time][TimeOfWeek][Zone][C];
				}
				ASSERTMSG(TotalVal > 0, "The percentages (Workers, business..) add up to 0");
				for (Int32 C = 0; C < POPCYCLE_TYPEGROUPS; C++)
				{
					m_nPercTypeGroup[Time][TimeOfWeek][Zone][C] *= (100.0f / TotalVal);
				}
				// Because of rounding errors we might have less than 100. Give the remainer to the highest one.
				TotalVal = 0;
				Int32 HighestVal = 0, HighestC = 0;
				for (Int32 C = 0; C < POPCYCLE_TYPEGROUPS; C++)
				{
					TotalVal += m_nPercTypeGroup[Time][TimeOfWeek][Zone][C];
					if (m_nPercTypeGroup[Time][TimeOfWeek][Zone][C] >= HighestVal)
					{
						HighestVal = m_nPercTypeGroup[Time][TimeOfWeek][Zone][C];
						HighestC = C;
					}
				}
				ASSERT(TotalVal <= 100);
				m_nPercTypeGroup[Time][TimeOfWeek][Zone][HighestC] += 100 - TotalVal;
			}
		}
	}

	m_nCurrentTimeIndex = 0;
	m_nCurrentTimeOfWeek = 0;
	m_pCurrZoneInfo = NULL;		// &(CTheZones::ZoneInfoArray[CTheZones::NavigationZoneArray[0].ZoneInfoIndex]); we don't want to pick a random zone because the streaming will start loading the wrong peds
//	ASSERT(m_pCurrZoneInfo);
	m_nCurrentZoneType = -1;	// m_pCurrZoneInfo->ZonePopulationType;
	
#ifndef MASTER
	m_nCurrentZoneName = 0;
	m_bDisplayDebugInfo = false;
#endif

	CFileMgr::CloseFile(fid);
	DEBUGLOG("CPopCycle ready\n");
}

//
//
// from Camera.cpp
//
//
#include "Camera.h"
#include "Frontend.h"
#include "mblur.h"

const float MIN_REAL_GROUND_DIST=1.85f;

const float INIT_TARGET_ZM_CLOSE_IN=2.08378f;

const float INIT_TARGET_ZM_GRND_ONE=-0.05;//2.10f - MIN_REAL_GROUND_DIST;//make these consts once les and aaron happy with values
const float INIT_TARGET_ZM_GRND_TWO=3.35f - MIN_REAL_GROUND_DIST;
const float INIT_TARGET_ZM_GRND_THREE=5.85f - MIN_REAL_GROUND_DIST;

extern Int16	DebugCamMode;
extern bool 	gPlayerPedVisible;

///////////////////////////////////////////////////////////////////////////
// NAME       : Init()
// PURPOSE    : Initialises the camera shite. (also inits the two cams)
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCamera::Init()
{
#ifdef GTA_PS2
	if ((FrontEndMenuManager.WantsToLoad==FALSE)&&(FrontEndMenuManager.m_WantsToRestartGame==false))
	{
		// AF: Had to change this as it was overwriting the virtual function
		//pointer table
		memset(((uint8*)this) + sizeof(CPlaceable), 0, sizeof(CCamera)-sizeof(CPlaceable));	// All zeros please
		m_pRwCamera = NULL;
	}
#else
		// AF: Had to change this as it was overwriting the virtual function
		//pointer table
		memset(((uint8*)this) + sizeof(CPlaceable), 0, sizeof(CCamera)-sizeof(CPlaceable));	// All zeros please
		m_pRwCamera = NULL;
#endif	
	
	TheCamera.InitialiseScriptableComponents();

	m_bPlayerWasOnBike=false;
	m_1rstPersonRunCloseToAWall=false;
	m_fPositionAlongSpline=0;
	m_bCameraJustRestored=false;
	Cams[0].Init();
	Cams[1].Init();
	Cams[2].Init();
	Cams[0].Mode=CCam::MODE_FOLLOWPED;
	Cams[1].Mode=CCam::MODE_FOLLOWPED;
	m_bEnable1rstPersonCamCntrlsScript=false; 
	m_bAllow1rstPersonWeaponsCamera=false;
	m_bCooperativeCamMode=false;
	m_bAllowShootingWith2PlayersInCar=true;

	m_bVehicleSuspenHigh=false;
	//make all these consts once les and aaron happy
	Cams[0].m_fMinRealGroundDist=MIN_REAL_GROUND_DIST;
	Cams[0].m_fTargetCloseInDist=INIT_TARGET_ZM_CLOSE_IN-Cams[0].m_fMinRealGroundDist;
	Cams[0].m_fTargetZoomGroundOne=-0.55f;//INIT_TARGET_ZM_GRND_ONE;//-Cams[0].m_fMinRealGroundDist;//make these consts once les and aaron happy with values
	Cams[0].m_fTargetZoomGroundTwo=1.5f;//INIT_TARGET_ZM_GRND_TWO;//-Cams[0].m_fMinRealGroundDist;
	Cams[0].m_fTargetZoomGroundThree=3.6f;//INIT_TARGET_ZM_GRND_THREE;//-Cams[0].m_fMinRealGroundDist;
	Cams[0].m_fTargetZoomOneZExtra=0.06f;//0.34;//-0.14f;//make these consts once les and aaron happy with values
	Cams[0].m_fTargetZoomTwoZExtra=-0.1f;//-0.2;//was -.08;
	Cams[0].m_fTargetZoomTwoInteriorZExtra=0.0f; // extra one for interior
	Cams[0].m_fTargetZoomThreeZExtra=-0.07f;//0.25f;
	Cams[0].m_fTargetZoomZCloseIn=0.900407f;

	m_bMoveCamToAvoidGeom=false;
	ClearPlayerWeaponMode();	
	m_bInATunnelAndABigVehicle=false;
	m_iModeObbeCamIsInForCar=NOT_IN_OBBE_MODE_YET;
	Cams[0].CamTargetEntity=NULL; //kept seperate from init
	Cams[1].CamTargetEntity=NULL; //Init used to reinitalise stuff when game
	Cams[2].CamTargetEntity=NULL; //while game is running
	Cams[0].m_fCamBufferedHeight=0.0f;
	Cams[0].m_fCamBufferedHeightSpeed=0.0f;
	Cams[1].m_fCamBufferedHeight=0.0f;
	Cams[1].m_fCamBufferedHeightSpeed=0.0f;
	Cams[0].m_bCamLookingAtVector=false;
	Cams[1].m_bCamLookingAtVector=false;
	Cams[2].m_bCamLookingAtVector=false;
	Cams[0].SetPlayerVelocityFor1stPerson(0.0f);
	Cams[1].SetPlayerVelocityFor1stPerson(0.0f);
	Cams[2].SetPlayerVelocityFor1stPerson(0.0f);
	m_bHeadBob=false;
	m_fFractionInterToStopMoving=0.25f; //these dudes must add up to one 
	m_fFractionInterToStopCatchUp=0.75f;
	m_fGaitSwayBuffer=0.85f;
	
	m_bScriptParametersSetForInterPol=false;

	m_uiCamShakeStart=0;			// When did the camera shake start.
	m_fCamShakeForce=0;			// How severe is the camera shake.
	m_iModeObbeCamIsInForCar=NOT_IN_OBBE_MODE_YET;
	m_bIgnoreFadingStuffForMusic=false;
	m_bWaitForInterpolToFinish=false;
	pToGarageWeAreIn=NULL;
	pToGarageWeAreInForHackAvoidFirstPerson=NULL;
	m_bPlayerIsInGarage=false;
	m_bJustCameOutOfGarage=false;
	m_fNearClipScript=0.9f;
	m_bUseNearClipScript=false;

	m_vecDoingSpecialInterPolation=false;
	m_bAboveGroundTrainNodesLoaded=false;
	m_bBelowGroundTrainNodesLoaded=false;

	m_ModeForTwoPlayersSeparateCars = CCam::MODE_TWOPLAYER_SEPARATE_CARS;
	m_ModeForTwoPlayersSameCarShootingAllowed = CCam::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING;
	m_ModeForTwoPlayersSameCarShootingNotAllowed = CCam::MODE_BEHINDCAR;
	m_ModeForTwoPlayersNotBothInCar = CCam::MODE_TWOPLAYER;

//	ActiveCam = 0;
	m_WideScreenOn=false;
	m_fFOV_Wide_Screen=0;
	m_bRestoreByJumpCut=false;
	m_nCarZoom = ZOOM_TWO;
	m_nPedZoom = ZOOM_TWO;
	m_fCarZoomBase = m_fCarZoomTotal = m_fCarZoomSmoothed = 0.0f;
	m_fPedZoomBase = m_fPedZoomTotal = m_fPedZoomSmoothed = 0.0f;
	pTargetEntity=NULL;
	if (FindPlayerVehicle() == NULL)
	{
		pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed;
	}
	else
	{
		pTargetEntity= FindPlayerVehicle();
	}
	if (pTargetEntity)
	{
		REGREF(pTargetEntity, &pTargetEntity);
	}
	m_bInitialNodeFound=false;
	m_ScreenReductionPercentage=0.0f;
	m_ScreenReductionSpeed=0.0f;
	m_WideScreenOn=false;
	m_bWantsToSwitchWidescreenOff=false;
	WorldViewerBeingUsed=0;
	PlayerExhaustion = 1.0f;
#ifndef MASTER	
  	DebugCamMode = 0;
#endif  	
	m_PedOrientForBehindOrInFront=0.0f;
#ifdef GTA_PS2	

	if ((FrontEndMenuManager.WantsToLoad==FALSE)&&(FrontEndMenuManager.m_WantsToRestartGame==false))
	{
		m_bFading=FALSE;
		CDraw::FadeValue=0;
		m_fFloatingFade=0.0f;
		m_bMusicFading=FALSE;
		m_fTimeToFadeMusic=0.0f;
		m_fFloatingFadeMusic=0.0f;
	}

#else
	if ((FrontEndMenuManager.m_WantsToRestartGame==false))
	{
		m_bFading=FALSE;
		CDraw::FadeValue=0;
		m_fFloatingFade=0.0f;
		m_bMusicFading=FALSE;
		m_fTimeToFadeMusic=0.0f;
		m_fFloatingFadeMusic=0.0f;
		//m_fMouseAccelHorzntl = 0.0025f;
		m_fMouseAccelVertical = 0.0015f;
	}
#endif

#ifdef GTA_PS2

	if ((FrontEndMenuManager.WantsToLoad==TRUE)||(FrontEndMenuManager.m_WantsToRestartGame==true))
	{
		m_fTimeToFadeMusic = 0.0f;
	}
#else
	if ((FrontEndMenuManager.m_WantsToRestartGame==true))
	{
		m_fTimeToFadeMusic = 0.0f;
	}
#endif	
	m_bStartingSpline=FALSE;
	m_iTypeOfSwitch=TRANS_INTERPOLATION;
	m_bUseScriptZoomValuePed=false;
	m_bUseScriptZoomValueCar=false;
	m_fPedZoomValueScript=0.0f;
	m_fCarZoomValueScript=0.0f;

	m_bUseSpecialFovTrain=false;
    m_fFovForTrain=70.0f;

	
	m_iModeToGoTo = CCam::MODE_FOLLOWPED;
	m_bJust_Switched=FALSE;
	m_bUseTransitionBeta=FALSE;
	// For now set a unity matrix or other code could trip up.
	GetMatrix().SetScale(1.0f);
	m_bTargetJustBeenOnTrain=false;
	m_bInitialNoNodeStaticsSet=false;
	m_uiLongestTimeInMill=5000;
	m_uiTimeLastChange=0;

	m_bIdleOn=false;
  	m_uiTimeWeLeftIdle_StillNoInput=0;
	m_uiTimeWeEnteredIdle=0;



	LODDistMultiplier = 1.0f;
	
#ifndef FINALBUILD	
	bStaticFrustum = false;
#endif
	m_bCamDirectlyBehind=false;	
	m_bCamDirectlyInFront=false;	
	m_motionBlur = 0;
	m_bGarageFixedCamPositionSet=false;
	SetMotionBlur(255,255,255,0,MB_BLUR_NONE);
	
	m_bCullZoneChecksOn=false;
	m_bFailedCullZoneTestPreviously=false;
	m_iCheckCullZoneThisNumFrames=6; 
	m_iZoneCullFrameNumWereAt=0;

	m_CameraAverageSpeed=0.0f; //this is an average depending on how many frames we work it out
	m_CameraSpeedSoFar=0.0f; //this is a running total
	 //this is an average depending on how many frames we work it out
	m_PreviousCameraPosition=CVector(0,0,0); //needed to work out speed
	m_iWorkOutSpeedThisNumFrames=4;//duh	
	m_iNumFramesSoFar=0; //counter
	m_bJustInitalised=true;//Just so the speed thingy doesn't go mad right at the start
	m_uiTransitionState=TRANS_NONE;		// 0:one mode 1:transition
	m_uiTimeTransitionStart=0;	// When was the transition started ?
	m_bLookingAtPlayer=true;
	m_bLookingAtVector=false;
	
	m_f3rdPersonCHairMultX = 0.53f;
	m_f3rdPersonCHairMultY = 0.40f;

	m_fAvoidTheGeometryProbsTimer = 0.0f;
	m_nAvoidTheGeometryProbsDirn = 0;
	
	gPlayerPedVisible = true;
	m_bResetOldMatrix = true;
#ifndef MASTER
	display_kbd_debug = FALSE;
	kbd_fov_value = 0.0f;
#endif // MASTER
}

//
//
// from AnimManager.cpp
//
//
#include "AnimManager.h"

// size of animation code's umcompressed animation cache
#define ANIMCACHE_SIZE		50

//
//        name: CAnimManager::Initialise
// description: Initialise AnimManager data
//
void CAnimManager::Initialise()
{
	ms_numAnimations = 0;
	ms_numAnimBlocks = 0;
	ms_numAnimAssocDefinitions = ANIM_NUM_ASSOCGROUPS;
	
	ms_animCache.Init(ANIMCACHE_SIZE);
	
	ReadAnimAssociationDefinitions();
	
	// ensure ped.ifp is animblock 0
	RegisterAnimBlock("ped");
}

//
// name:		ReadAnimAssociationDefinitions
// description:	Read anim association definition file
void CAnimManager::ReadAnimAssociationDefinitions()
{
	extern AnimDescriptor aStdAnimDescs[];
	bool loadingAnims = false;
	char* pLine;
	int32 fid;
	char name[32];
	char file[32];
	char type[32];
	int32 numAnims;
	AnimAssocDefinition* pDef;

	CFileMgr::SetDir("");
	fid = CFileMgr::OpenFile("DATA\\ANIMGRP.DAT", "rb");
	ASSERT(fid != 0);
	
	int32 j=0;
	while ((pLine = CFileLoader::LoadLine(fid)))
	{
		if(*pLine == '#' || *pLine == '\0')
			continue;
		
		if(loadingAnims == false)
		{
			if(sscanf(pLine, "%s %s %s %d", name, file, type, &numAnims) != 4)
			{
				ASSERTMSG(0, "Corrupt animgrp.dat");
			}
			
			pDef = AddAnimAssocDefinition(name, file, MODELID_DEFAULT_PED, numAnims, aStdAnimDescs);
			loadingAnims = true;
		}
		else
		{
			if(sscanf(pLine, "%s", name) == 1)
			{
				if(!strcmp(name, "end"))
					loadingAnims = false;
				else
					AddAnimToAssocDefinition(pDef, name);
			}		
		}
	}
	
	ASSERTMSG(loadingAnims == false, "Corrupt animgrp.dat");

	CFileMgr::CloseFile(fid);
	
}

//
//
// from ModelIndices.cpp
//
//

//
// name:		InitModelIndices
// description:	Initialise model indices to -1
void InitModelIndices()
{
	MI_TRAFFICLIGHTS = -1;
	MI_TRAFFICLIGHTS_VERTICAL = -1;
	MI_TRAFFICLIGHTS_MIAMI = -1;
	MI_TRAFFICLIGHTS_VEGAS = -1;
	MI_TRAFFICLIGHTS_TWOVERTICAL = -1;

	MI_SINGLESTREETLIGHTS1 = -1;
	MI_SINGLESTREETLIGHTS2 = -1;
	MI_SINGLESTREETLIGHTS3 = -1;
	MI_DOUBLESTREETLIGHTS = -1;
	MI_STREETLAMP1 = -1;
	MI_STREETLAMP2 = -1;
	
								// These trees have a shadow
//	MI_TREE2 = -1;					// veg_tree3 
//	MI_TREE3 = -1;					// veg_treea1 
//	MI_TREE6 = -1;					// veg_treeb1 
//	MI_TREE8 = -1;					// veg_treea3 

	MODELID_CRANE_1 = -1;
	MODELID_CRANE_2 = -1;
	MODELID_CRANE_3 = -1;
	MODELID_CRANE_4 = -1;
	MODELID_CRANE_5 = -1;
	MODELID_CRANE_6 = -1;

	MI_PARKINGMETER = -1;
	MI_PARKINGMETER2 = -1;
	MI_MALLFAN = -1;
	MI_HOTELFAN_NIGHT = -1;
	MI_HOTELFAN_DAY = -1;
	MI_HOTROOMFAN = -1;
	MI_PHONEBOOTH1 = -1;
	MI_WASTEBIN = -1;
	MI_BIN = -1;
	MI_POSTBOX1 = -1;
	MI_NEWSSTAND = -1;
	MI_TRAFFICCONE = -1;
	MI_DUMP1 = -1;
	MI_ROADWORKBARRIER1 = -1;
	MI_ROADBLOCKFUCKEDCAR1 = -1;
	MI_ROADBLOCKFUCKEDCAR2 = -1;
	MI_BUSSIGN1 = -1;
	MI_NOPARKINGSIGN1 = -1;
	MI_PHONESIGN = -1;
	MI_FIRE_HYDRANT = -1;
	MI_COLLECTABLE1 = -1;
	MI_MONEY = -1;
	MI_CARMINE = -1;
	MI_NAUTICALMINE = -1;
	MI_TELLY = -1;
//	MI_CRUSHERBODY = -1;
//	MI_CRUSHERLID = -1;
	MI_BRIEFCASE = -1;
	MI_GLASS1 = -1;
	MI_GLASS8 = -1;
//	MI_BRIDGELIFT = -1;
//	MI_BRIDGEWEIGHT = -1;
//	MI_BRIDGEROADSEGMENT = -1;
	MI_EXPLODINGBARREL = -1;
	MI_PICKUP_ADRENALINE = -1;
	MI_PICKUP_BODYARMOUR = -1;
	MI_PICKUP_INFO = -1;
	MI_PICKUP_HEALTH = -1;
	MI_PICKUP_PROPERTY = -1;
	MI_PICKUP_PROPERTY_FORSALE = -1;
	MI_PICKUP_BONUS = -1;
	MI_PICKUP_BRIBE = -1;
	MI_PICKUP_KILLFRENZY = -1;
	MI_PICKUP_CAMERA = -1;
	MI_PICKUP_PARACHUTE = -1;
	MI_PICKUP_REVENUE = -1;
	MI_PICKUP_SAVEGAME = -1;
	MI_PICKUP_CLOTHES = -1;
	MI_PICKUP_2P_KILLFRENZY = -1;
	MI_PICKUP_2P_COOP = -1;
	MI_BOLLARDLIGHT = -1;
	MI_PARACHUTE_BACKPACK = -1;
	MI_OYSTER = -1;
	MI_HORSESHOE = -1;


	// The ones new to Miami
	MI_LAMPPOST1 = -1;
//	MI_VEG_PALM01 = -1;
//	MI_VEG_PALM02 = -1;
//	MI_VEG_PALM03 = -1;
//	MI_VEG_PALM04 = -1;
//	MI_VEG_PALM05 = -1;
//	MI_VEG_PALM06 = -1;
//	MI_VEG_PALM07 = -1;
//	MI_VEG_PALM08 = -1;
	MI_MLAMPPOST = -1;
	MI_BARRIER1 = -1;

	MI_LITTLEHA_POLICE = -1;
	MI_TELPOLE02 = -1;
	MI_TRAFFICLIGHT01 = -1;
	MI_PARKBENCH = -1;
//	MI_PLC_STINGER = -1;
	MI_LIGHTBEAM = -1;
	MI_AIRPORTRADAR = -1;
	MI_RCBOMB = -1;

	//
	// sea lifeforms start here:
	//	
	MI_BEACHBALL = -1;
	MI_SANDCASTLE1 = -1;
	MI_SANDCASTLE2 = -1;
	MI_JELLYFISH = -1;
	MI_JELLYFISH01 = -1;
	MI_FISH1SINGLE = -1;
	MI_FISH1S = -1;
	MI_FISH2SINGLE = -1;
	MI_FISH2S = -1;
	MI_FISH3SINGLE = -1;
	MI_FISH3S = -1;
	MI_TURTLE = -1;
	MI_DOLPHIN = -1;
	MI_SHARK = -1;
	MI_SUBMARINE = -1;
	MI_ESCALATORSTEP = -1;
	MI_ESCALATORSTEP8 = -1;

	MI_LOUNGE_WOOD_UP = -1;
	MI_LOUNGE_TOWEL_UP = -1;
	MI_LOUNGE_WOOD_DN = -1;
	MI_LOTION = -1;
	MI_BEACHTOWEL01 = -1;
	MI_BEACHTOWEL02 = -1;
	MI_BEACHTOWEL03 = -1;
	MI_BEACHTOWEL04 = -1;

	MI_BLIMP_NIGHT = -1;
	MI_BLIMP_DAY = -1;
	MI_YT_MAIN_BODY = -1;
	MI_YT_MAIN_BODY2 = -1;
	MI_TRAFFICLIGHTS_3 = -1;
	MI_TRAFFICLIGHTS_4 = -1;
	MI_TRAFFICLIGHTS_5 = -1;
	MI_TRAFFICLIGHTS_GAY = -1;

	MI_IMY_SHASH_WALL = -1;	//	"imy_shash_wall"
	
	MI_HYDRAULICS = -1;
	MI_STEREO_UPGRADE = -1;
	
	MI_BASKETBALL = -1;
	MI_POOL_CUE_BALL = -1;
	MI_PUNCHBAG = -1;
	MI_IMY_GRAY_CRATE = -1;
}


//
// from Streaming.h
//
#include "streaming.h"

void CStreaming::ReadIniFile()
{
	char* pLine;
	char* pToken1;
	char* pToken2;
	bool memFound = false;
	// read INI file
	int32 fid = CFileMgr::OpenFile("stream.ini");
	ASSERTMSG(fid != 0, "Cannot find stream.ini");
	
	while((pLine = CFileLoader::LoadLine(fid)) != NULL)
	{
		if(*pLine == '#' || *pLine == '\0')
			continue;

		pToken1 = strtok(pLine, " ,\t");
		pToken2 = strtok(NULL, " ,\t");
		if(!stricmp(pToken1, "memory") && memFound == false)
			ms_memoryAvailable = atoi(pToken2) * 1024;
#ifndef CDROM			
		else if(!stricmp(pToken1, "devkit_memory"))
		{
			ms_memoryAvailable = atoi(pToken2) * 1024;
			memFound = true;
		}
#endif		
		else if(!stricmp(pToken1, "vehicles"))
			desiredNumVehiclesLoaded = atoi(pToken2);
		else if(!stricmp(pToken1, "dontbuildpaths"))
			bDontBuildPaths = true;
		else if(!stricmp(pToken1, "pe_lightchangerate"))
			CPostEffects::SCREEN_EXTRA_MULT_CHANGE_RATE = atof(pToken2);
		else if(!stricmp(pToken1, "pe_lightingbasecap"))
			CPostEffects::SCREEN_EXTRA_MULT_BASE_CAP = atof(pToken2);
		else if(!stricmp(pToken1, "pe_lightingbasemult"))
			CPostEffects::SCREEN_EXTRA_MULT_BASE_MULT = atof(pToken2);
		else if(!stricmp(pToken1, "pe_leftx"))
			CPostEffects::m_colourLeftUOffset = atoi(pToken2);
		else if(!stricmp(pToken1, "pe_rightx"))
			CPostEffects::m_colourRightUOffset = atoi(pToken2);
		else if(!stricmp(pToken1, "pe_topy"))
			CPostEffects::m_colourTopVOffset = atoi(pToken2);
		else if(!stricmp(pToken1, "pe_bottomy"))
			CPostEffects::m_colourBottomVOffset = atoi(pToken2);
		else if(!stricmp(pToken1, "pe_bRadiosity"))
			CPostEffects::m_bRadiosity = atoi(pToken2);
#ifdef NTSC_BUILD
		else if(!stricmp(pToken1, "def_brightness_ntsc"))
			FrontEndMenuManager.m_PrefsBrightness = atoi(pToken2);
#else
		else if(!stricmp(pToken1, "def_brightness_pal"))
			FrontEndMenuManager.m_PrefsBrightness = atoi(pToken2);
#endif
		else
			DEBUGLOG1("Unrecognised token %s", pToken1);	
	}
	
	CFileMgr::CloseFile(fid);
}

//
// from WeaponInfo.cpp
// 
#include "WeaponInfo.h"
#include "TaskAttack.h"
#include "WeaponModelInfo.h"

extern CWeaponInfo aWeaponInfo[WEAPONTYPE_ARRAY_SIZE];

// Name			:	CWeaponInfo::Initialise
// Purpose		:	Init CWeaponInfo object
// Parameters	:	None
// Returns		:	Nothing

void CWeaponInfo::Initialise()
{
	DEBUGLOG("Initialising CWeaponInfo...\n");

	for(int32 i = 0; i < WEAPONTYPE_ARRAY_SIZE; i++)
	{
		aWeaponInfo[i].m_eFireType = FIRETYPE_MELEE;

		aWeaponInfo[i].m_fTargetRange = 0.0f;
		aWeaponInfo[i].m_fWeaponRange = 0.0f;
		aWeaponInfo[i].m_modelId = -1;
		aWeaponInfo[i].m_modelId2 = -1;
	
//		aWeaponInfo[i].m_nReloadSampleTime = 0;
//		aWeaponInfo[i].m_nReloadSampleTime2 = 0;
	
		aWeaponInfo[i].m_nWeaponSlot = -1;//WEAPONSLOT_TYPE_UNARMED;

		// Gun Data
		aWeaponInfo[i].m_animGroup = ANIM_STD_PED;

		aWeaponInfo[i].m_nAmmo = 0;
		aWeaponInfo[i].m_nDamage = 0;
		aWeaponInfo[i].m_vecFireOffset = CVector(0.0f, 0.0f, 0.0f);;
	
		aWeaponInfo[i].m_SkillLevel = WEAPONSKILL_STD;
		aWeaponInfo[i].m_nReqStatLevel = 0;
		aWeaponInfo[i].m_fAccuracy = 1.0f;
		aWeaponInfo[i].m_fMoveSpeed = 1.0f;

		aWeaponInfo[i].m_animLoopStart = 0.0f;
		aWeaponInfo[i].m_animLoopEnd = 0.0f;
		aWeaponInfo[i].m_animFireTime = 0.0f;

		aWeaponInfo[i].m_anim2LoopStart = 0.0f;
		aWeaponInfo[i].m_anim2LoopEnd = 0.0f;
		aWeaponInfo[i].m_anim2FireTime = 0.0f;

		aWeaponInfo[i].m_animBreakoutTime = 0.0f;
	
		aWeaponInfo[i].m_fSpeed = 0.0f;
		aWeaponInfo[i].m_fRadius = 0.0f;
		aWeaponInfo[i].m_fLifeSpan = 0.0f;
		aWeaponInfo[i].m_fSpread = 0.0f;
		
		aWeaponInfo[i].m_nAimOffsetIndex = 0;
		aWeaponInfo[i].m_nFlags = 0;
	
		// Melee Data
		aWeaponInfo[i].m_defaultCombo = MCOMBO_UNARMED_1;
		aWeaponInfo[i].m_nCombosAvailable = 1;
	}
	
	for(int32 i = 0; i < WEAPONTYPE_ANIM_AIM_NUM; i++)
	{
		ms_aWeaponAimOffsets[i].fAimFromX = 0.0f;
		ms_aWeaponAimOffsets[i].fAimFromZ = 0.0f;
		ms_aWeaponAimOffsets[i].fDuckAimX = 0.0f;
		ms_aWeaponAimOffsets[i].fDuckAimZ = 0.0f;

		ms_aWeaponAimOffsets[i].nReloadSampleA = 0;
		ms_aWeaponAimOffsets[i].nReloadSampleB = 0;
		ms_aWeaponAimOffsets[i].nReloadSampleCrouchedA = 0;
		ms_aWeaponAimOffsets[i].nReloadSampleCrouchedB = 0;
	}

	DEBUGLOG("Loading weapon data...\n");
	LoadWeaponData();
	DEBUGLOG("CWeaponInfo ready\n");
} // end - CWeaponInfo::Initialise


// Name			:	LoadWeaponData
// Purpose		:	Loads weapon info from WEAPON.DAT file
// Parameters	:	None
// Returns		:	Nothing
//
#define MAX_LINE_SIZE	256
//
void CWeaponInfo::LoadWeaponData(void)
{
	eWeaponType WeaponType;
	int16 WeaponTypeSlot;
	char* pLine;
	bool bAssocWeaponModel = false;
	// misc
	char aIdent[8];
	char aWeaponName[32], aFireType[32];
	float fRange1, fRange2;
	int32 nModelId, nModelId2;
	int32 nRSampleA, nRSampleB;
	int32 nRSampleCrouchedA, nRSampleCrouchedB;
	int32 nSlot;
	// melee ///////
	char aComboName[32];
	int32 nNumCombos;
	// gun /////////
	char aAnimGroupName[32];
	int32 nAmmo, nDamage;
	CVector vecFireOffset;
	int32 nSkillLevel, nReqStatLevel;
	float fSkillAccuracy, fSkillMoveSpeed;
	int32 nLoopStart, nLoopEnd, nFireTime;
	int32 nLoopStart2, nLoopEnd2, nFireTime2;
	int32 nBreakTime;
	int32 nFlags;
	float fSpeed, fRadius, fLifeSpan, fSpread;

	// open file
	int32 fid = CFileMgr::OpenFile("DATA\\WEAPON.DAT", "rb");
	ASSERT(fid != 0);

	// keep going till we reach the end of the file
	while ((pLine = CFileLoader::LoadLine(fid)))
	{
		// if beginning of line is the end it is empty, or line is a comment
		if(*pLine == '\0' || *pLine == '#')
			continue;
		
		bAssocWeaponModel = false;
		aWeaponName[0] = '\0';
		aFireType[0] = '\0';
		aComboName[0] = '\0';
		aAnimGroupName[0] = '\0';
		
		// Melee format data
		if(*pLine == '�')
		{
			sscanf(pLine, "%s %s %s %f %f %d %d %d %s %d %x %s",
			aIdent, aWeaponName, aFireType,
			&fRange1, &fRange2,
			&nModelId, &nModelId2,
//			&nRSample1, &nRSample2,
			&nSlot,
			aComboName,	&nNumCombos,
			&nFlags,	aAnimGroupName);
			
			// build weapon data structure
			WeaponType = FindWeaponType(aWeaponName);
			
			// generic data
			aWeaponInfo[WeaponType].m_eFireType = FindWeaponFireType(aFireType);
			aWeaponInfo[WeaponType].m_fTargetRange = fRange1;
			aWeaponInfo[WeaponType].m_fWeaponRange = fRange2;
			aWeaponInfo[WeaponType].m_modelId = nModelId;
			aWeaponInfo[WeaponType].m_modelId2 = nModelId2;
//			aWeaponInfo[WeaponType].m_nReloadSampleTime = nRSample1;
//			aWeaponInfo[WeaponType].m_nReloadSampleTime2 = nRSample2;
			aWeaponInfo[WeaponType].m_nWeaponSlot = nSlot;

			// melee specific data
			aWeaponInfo[WeaponType].m_nCombosAvailable = nNumCombos;
			aWeaponInfo[WeaponType].m_defaultCombo = CTaskSimpleFight::GetComboType(aComboName);

			aWeaponInfo[WeaponType].m_nFlags = nFlags;
			
			// Get main animation association group
			if(strncmp(aAnimGroupName, "null", 4))
			{
				for(int32 i=0; i<CAnimManager::GetNumAnimAssocDefinitions(); i++)
				{
					if(!strcmp(aAnimGroupName, CAnimManager::GetAnimGroupName((AssocGroupId)i)))
					{
						aWeaponInfo[WeaponType].m_animGroup = (AssocGroupId)i;
						break;
					}	
				}
				ASSERTOBJ(aWeaponInfo[WeaponType].m_animGroup != ANIM_STD_PED, aAnimGroupName, "Unrecognised animation group name");
			}		
			// link the modelId to a weaponModelInfo
			bAssocWeaponModel = true;
		}
		// Gun format data
		else if(*pLine == '$')
		{
			// these values might not be read in, so need to clear previous values
			fSpeed = fRadius = fLifeSpan = fSpread = 0.0f;
		
			//-------------------------------------------anim-----offset---skill-------loop1----loop2-------flag----------
			sscanf(pLine, "%s %s %s %f %f %d %d %d %s %d %d %f %f %f %d %d %f %f %d %d %d %d %d %d %d %x %f %f %f %f",
			aIdent, aWeaponName, aFireType,
			&fRange1, &fRange2,
			&nModelId, &nModelId2,
//			&nRSample1, &nRSample2,
			&nSlot,
			aAnimGroupName,
			&nAmmo, &nDamage,
			&vecFireOffset.x, &vecFireOffset.y, &vecFireOffset.z,
			&nSkillLevel, &nReqStatLevel, &fSkillAccuracy, &fSkillMoveSpeed,
			&nLoopStart, &nLoopEnd, &nFireTime,
			&nLoopStart2, &nLoopEnd2, &nFireTime2,
			&nBreakTime, &nFlags,
			&fSpeed, &fRadius, &fLifeSpan, &fSpread);
			
			// build weapon data structure
			WeaponType = FindWeaponType(aWeaponName);
			if(WeaponType >= WEAPONTYPE_FIRST_SKILLWEAPON && WeaponType <= WEAPONTYPE_LAST_SKILLWEAPON)
			{
				if(nSkillLevel == WEAPONSKILL_POOR)
					WeaponTypeSlot = (WeaponType - WEAPONTYPE_FIRST_SKILLWEAPON) + WEAPONTYPE_LAST_WEAPONTYPE;
				else if(nSkillLevel == WEAPONSKILL_STD)
					WeaponTypeSlot = WeaponType;
				else if(nSkillLevel == WEAPONSKILL_PRO)
					WeaponTypeSlot = (WeaponType - WEAPONTYPE_FIRST_SKILLWEAPON) + WEAPONTYPE_LAST_WEAPONTYPE + WEAPONTYPE_NUM_SKILLWEAPONS;
				else if(nSkillLevel == WEAPONSKILL_SPECIAL)
					WeaponTypeSlot = (WeaponType - WEAPONTYPE_FIRST_SKILLWEAPON) + WEAPONTYPE_LAST_WEAPONTYPE + 2*WEAPONTYPE_NUM_SKILLWEAPONS;
				else
					ASSERT(false);
			}
			else
			{
				WeaponTypeSlot = WeaponType;
				// force std skill for weapons without different levels
				nSkillLevel = WEAPONSKILL_STD;
			}
			
			// generic data
			aWeaponInfo[WeaponTypeSlot].m_eFireType = FindWeaponFireType(aFireType);
			aWeaponInfo[WeaponTypeSlot].m_fTargetRange = fRange1;
			aWeaponInfo[WeaponTypeSlot].m_fWeaponRange = fRange2;
			aWeaponInfo[WeaponTypeSlot].m_modelId = nModelId;
			aWeaponInfo[WeaponTypeSlot].m_modelId2 = nModelId2;
//			aWeaponInfo[WeaponTypeSlot].m_nReloadSampleTime = nRSample1;
//			aWeaponInfo[WeaponTypeSlot].m_nReloadSampleTime2 = nRSample2;
			aWeaponInfo[WeaponTypeSlot].m_nWeaponSlot = nSlot;
			
			// gun specific data
			aWeaponInfo[WeaponTypeSlot].m_nAmmo = nAmmo;
			aWeaponInfo[WeaponTypeSlot].m_nDamage = nDamage;
			aWeaponInfo[WeaponTypeSlot].m_vecFireOffset = vecFireOffset;
			aWeaponInfo[WeaponTypeSlot].m_SkillLevel = eWeaponSkill(nSkillLevel);
			aWeaponInfo[WeaponTypeSlot].m_nReqStatLevel = nReqStatLevel;
			aWeaponInfo[WeaponTypeSlot].m_fAccuracy = fSkillAccuracy;
			aWeaponInfo[WeaponTypeSlot].m_fMoveSpeed = fSkillMoveSpeed;
			aWeaponInfo[WeaponTypeSlot].m_animLoopStart = nLoopStart*ATTACK_ANIM_RATE;
			aWeaponInfo[WeaponTypeSlot].m_animLoopEnd = nLoopEnd*ATTACK_ANIM_RATE;
			aWeaponInfo[WeaponTypeSlot].m_animFireTime = nFireTime*ATTACK_ANIM_RATE;
			aWeaponInfo[WeaponTypeSlot].m_anim2LoopStart = nLoopStart2*ATTACK_ANIM_RATE;
			aWeaponInfo[WeaponTypeSlot].m_anim2LoopEnd = nLoopEnd2*ATTACK_ANIM_RATE;
			aWeaponInfo[WeaponTypeSlot].m_anim2FireTime = nFireTime2*ATTACK_ANIM_RATE;

			aWeaponInfo[WeaponTypeSlot].m_animBreakoutTime = nBreakTime*ATTACK_ANIM_RATE;
			aWeaponInfo[WeaponTypeSlot].m_nFlags = nFlags;
			
			// projectile/area effect specific info
			aWeaponInfo[WeaponTypeSlot].m_fSpeed = fSpeed;
			aWeaponInfo[WeaponTypeSlot].m_fRadius = fRadius;
			aWeaponInfo[WeaponTypeSlot].m_fLifeSpan = fLifeSpan;
			aWeaponInfo[WeaponTypeSlot].m_fSpread = fSpread;

			// Get main animation association group
			if(strncmp(aAnimGroupName, "null", 4))
			{
				for(int32 i=0; i<CAnimManager::GetNumAnimAssocDefinitions(); i++)
				{
					if(!strcmp(aAnimGroupName, CAnimManager::GetAnimGroupName((AssocGroupId)i)))
					{
						aWeaponInfo[WeaponTypeSlot].m_animGroup = (AssocGroupId)i;
						break;
					}	
				}
				ASSERTOBJ(aWeaponInfo[WeaponTypeSlot].m_animGroup != ANIM_STD_PED, aAnimGroupName, "Unrecognised animation group name");
			}
			
			if(aWeaponInfo[WeaponTypeSlot].m_animGroup >= WEAPONTYPE_ANIM_AIM_FIRST
			&& aWeaponInfo[WeaponTypeSlot].m_animGroup <= WEAPONTYPE_ANIM_AIM_LAST)
			{
				aWeaponInfo[WeaponTypeSlot].m_nAimOffsetIndex = aWeaponInfo[WeaponTypeSlot].m_animGroup - WEAPONTYPE_ANIM_AIM_FIRST;
			}

			// Hack to stop the wrong weapontype getting associated with a model (because of duplicate modelids for weapons)
			if(nSkillLevel != 1 || WeaponType==WEAPONTYPE_DETONATOR)
			{
				bAssocWeaponModel = false;
			}
			else if(WeaponType==WEAPONTYPE_REMOTE_SATCHEL_CHARGE)
			{
				//nModelId = MODELID_WEAPON_BOMB;
				bAssocWeaponModel = true;
			}
			else
				bAssocWeaponModel = true;
			
#ifdef NTSC_BUILD
			float BASE_FRAME = 1.0f/60.0f;
#else
			float BASE_FRAME = 1.0f/50.0f;
#endif
			
			float fLoopLength = aWeaponInfo[WeaponTypeSlot].m_animLoopEnd - aWeaponInfo[WeaponTypeSlot].m_animLoopStart;
			fLoopLength = BASE_FRAME*CMaths::Floor((fLoopLength / BASE_FRAME) + 0.1f);
			fLoopLength -= 0.3f*BASE_FRAME;
			aWeaponInfo[WeaponTypeSlot].m_animLoopEnd = aWeaponInfo[WeaponTypeSlot].m_animLoopStart + fLoopLength;

			fLoopLength = aWeaponInfo[WeaponTypeSlot].m_anim2LoopEnd - aWeaponInfo[WeaponTypeSlot].m_anim2LoopStart;
			fLoopLength = BASE_FRAME*CMaths::Floor((fLoopLength / BASE_FRAME) + 0.1f);
			fLoopLength -= 0.3f*BASE_FRAME;
			aWeaponInfo[WeaponTypeSlot].m_anim2LoopEnd = aWeaponInfo[WeaponTypeSlot].m_anim2LoopStart + fLoopLength;
		}
		// aiming offset data
		else if(*pLine == '%')
		{
			sscanf(pLine, "%s %s %f %f %f %f %d %d %d %d",
			aIdent, aAnimGroupName,
			&vecFireOffset.x, &vecFireOffset.z,
			&fRange1, &fRange2,
			&nRSampleA, &nRSampleB, &nRSampleCrouchedA, &nRSampleCrouchedB);
			
			// Get main animation association group
			int32 nFindAnimGroup = 0;
			for(nFindAnimGroup=0; nFindAnimGroup<CAnimManager::GetNumAnimAssocDefinitions(); nFindAnimGroup++)
			{
				if(!strcmp(aAnimGroupName, CAnimManager::GetAnimGroupName((AssocGroupId)nFindAnimGroup)))
				{
					break;
				}	
			}
			ASSERTOBJ(nFindAnimGroup>=WEAPONTYPE_ANIM_AIM_FIRST && nFindAnimGroup<=WEAPONTYPE_ANIM_AIM_LAST, aAnimGroupName, "Unrecognised animation group name");
			
			nFindAnimGroup -= WEAPONTYPE_ANIM_AIM_FIRST;
			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].fAimFromX = vecFireOffset.x;
			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].fAimFromZ = vecFireOffset.z;
			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].fDuckAimX = fRange1;
			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].fDuckAimZ = fRange2;

			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].nReloadSampleA = nRSampleA;
			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].nReloadSampleB = nRSampleB;
			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].nReloadSampleCrouchedA = nRSampleCrouchedA;
			CWeaponInfo::ms_aWeaponAimOffsets[nFindAnimGroup].nReloadSampleCrouchedB = nRSampleCrouchedB;
		}
		else
		{
			sscanf(pLine, "%s", aWeaponName);
			
			if (strncmp(aWeaponName, "ENDWEAPONDATA", 13) == 0)
				break;
		}
		
		// associate this weapontype with the defined weaponModelInfo
		if(bAssocWeaponModel && nModelId > 0)
		{
			CWeaponModelInfo* pModelInfo = (CWeaponModelInfo*)CModelInfo::GetModelInfo(nModelId);
			ASSERTOBJ(pModelInfo, aWeaponName, "No ModelInfo associated with weapon");
			ASSERTOBJ(pModelInfo->GetModelType() == MI_TYPE_WEAPON, pModelInfo->GetModelName(), "Model is required to be a weapon");
			//ASSERTOBJ(pModelInfo->GetWeaponInfo() == NULL, pModelInfo->GetModelName(), "Model is already associated with a weapon");
			
			pModelInfo->SetWeaponInfo(WeaponType);
		}
	}
	
	CFileMgr::CloseFile(fid);
} // end - CWeaponInfo::LoadWeaponData



// Name			:	FindWeaponFireType
// Purpose		:	Takes a string and returns the corresponding weapon fire type
// Parameters	:	pString - string to check
// Returns		:	Weapon fire type

eFireType CWeaponInfo::FindWeaponFireType(const char *pString)
{
	if (strcmp(pString, "MELEE") == 0)
	{
		return FIRETYPE_MELEE;
	}
	else if (strcmp(pString, "INSTANT_HIT") == 0)
	{
		return FIRETYPE_INSTANT_HIT;
	}
	else if (strcmp(pString, "PROJECTILE") == 0)
	{
		return FIRETYPE_PROJECTILE;
	}
	else if (strcmp(pString, "AREA_EFFECT") == 0)
	{
		return FIRETYPE_AREA_EFFECT;
	}
	else if (strcmp(pString, "CAMERA") == 0)
	{
		return FIRETYPE_CAMERA;
	}
	else if (strcmp(pString, "USE") == 0)
	{
		return FIRETYPE_USE;
	}
	else
	{
#ifndef FINAL	
		CDebug::DebugMessage("Unknown weapon fire type, WeaponInfo.cpp");
#endif		
		return FIRETYPE_INSTANT_HIT;
	}
} // end - CWeaponInfo::FindWeaponFireType


//
// from population.cpp
//

//
// name:		LoadPedGroups
// description:	Load the pedgrp.dat file into the m_pPedGroups array
//
void CPopulation::LoadPedGroups()
{
	int32 fd;
	char line[1024];
	char modelname[256];
	int32 start, end;
	int32 pedgrp, i, pedsingroup;
	CBaseModelInfo* pModelInfo;

	CFileMgr::ChangeDir("\\DATA\\");

	fd = CFileMgr::OpenFile("PEDGRP.DAT");
	ASSERTMSG(fd != 0, "problem loading pedgrp.dat");

	CFileMgr::ChangeDir("\\");

	pedgrp = 0;
	// get each line. Each line is a model file
	while(CFileMgr::ReadLine(fd, &line[0], 1024))
	{
		pedsingroup = 0;
		bool bIsAGroup = false;
		// tidy up line	
		i = 0;
		while(line[i] != '\n')
		{
			if(line[i] == ',' || line[i] == '\r')
				line[i] = ' ';
			i++;	
		}
		// add end of line
		line[i] = '\0';

		start = 0;
		
		for(i=0; i<MAX_PEDGROUP_SIZE; i++)
		{
			// find start of next name
			while(line[start] <= ' ' && line[start] != '\0')
				start++;
					
			if(line[start] == '#')
				break;
						
			// find end of name
			end = start;
			while(line[end] > ' ')
				end++;
					
			if(start == end)
				break;
						
			strncpy(modelname, &line[start], end - start);
			modelname[end-start] = '\0';
				
			Int32	MI;
			pModelInfo = CModelInfo::GetModelInfo(modelname, &MI);

			if (!pModelInfo)
			{
				//ASSERTOBJ(pModelInfo, modelname, "Can't find in ModelInfo array (pedgrp.dat)");
				DEBUGLOG1("Model %s (in pedgrp.dat) doesn't exist\n", modelname);
			}
			else
			{
//				ms_pPedGroups[pedgrp][pedsingroup][WorldZone] = MI;
				SetPedGroupPed(pedgrp, pedsingroup, MI);
//				SetPedGroupPedRace(pedgrp, pedsingroup, FindPedRaceFromName(modelname));
				pedsingroup++;
			}
			bIsAGroup = true;
			start = end;
		}
			
//		if (pedsingroup > 0)
		if (bIsAGroup)
		{
//			m_NumPedsInGroup[pedgrp][WorldZone] = pedsingroup;
			SetNumPedsInGroup(pedgrp, pedsingroup);
			
			// Fill the rest in with -1's.
			for (; pedsingroup<MAX_PEDGROUP_SIZE; pedsingroup++)
			{
//				ms_pPedGroups[pedgrp][pedsingroup][WorldZone] = -1;
				SetPedGroupPedIsEmpty(pedgrp, pedsingroup);

			}	

			pedgrp++;
		}

		ASSERTMSG(pedsingroup==0 || pedsingroup == MAX_PEDGROUP_SIZE, "Error reading pedgrp.dat");
		
	}
	
	CFileMgr::CloseFile(fd);
	
	ASSERTMSG(pedgrp == TOTAL_NUM_PED_GROUPS, "Number of pedgroups in pedgrp.dat is different from the game");
}


//
// name:		LoadCarGroups
// description:	Load the cargrp.dat file into the m_pCarGroups array
//
void CPopulation::LoadCarGroups()
{
	int32 fd;
	char line[1024];
	char modelname[256];
	int32 start, end;
	int32 cargrp, i, carsingroup;
	CBaseModelInfo* pModelInfo;

	CFileMgr::ChangeDir("\\DATA\\");

	fd = CFileMgr::OpenFile("CARGRP.DAT");
	ASSERTMSG(fd >= 0, "problem loading cargrp.dat");

	CFileMgr::ChangeDir("\\");

	cargrp = 0;
	// get each line.
	while(CFileMgr::ReadLine(fd, &line[0], 1024))
	{
		carsingroup = 0;
		// tidy up line	
		i = 0;
		while(line[i] != '\n')
		{
			if(line[i] == ',' || line[i] == '\r')
				line[i] = ' ';
			i++;	
		}
		// add end of line
		line[i] = '\0';

		start = 0;
		
		for(i=0; i<MAX_CARGROUP_SIZE; i++)
		{
			// find start of next name
			while(line[start] <= ' ' && line[start] != '\0')
				start++;
				
			if(line[start] == '#')
				break;
					
			// find end of name
			end = start;
			while(line[end] > ' ')
				end++;
				
			if(start == end)
				break;
					
			strncpy(modelname, &line[start], end - start);
			modelname[end-start] = '\0';
			
			Int32	MI;
			pModelInfo = CModelInfo::GetModelInfo(modelname, &MI);
			if (pModelInfo)
			{
//				ms_pCarGroups[cargrp][carsingroup] = MI;
				SetCarGroupCar(cargrp, carsingroup, MI);
//				CCarCtrl::AddToVehicleArray(MI, cargrp);
				carsingroup++;
			}
			else
			{
//				DEBUGLOG1("This car (%s) from cargrp.dat doesn't exist\n", modelname);
				ASSERTOBJ(pModelInfo, modelname, "Can't find in ModelInfo array (cargrp.dat)");
			}

			
			start = end;
		}
		
		if (carsingroup > 0)
		{
//			m_NumCarsInGroup[cargrp] = carsingroup;
			SetNumCarsInGroup(cargrp, carsingroup);

			// Fill the rest in with -1's.
			for (; carsingroup<MAX_CARGROUP_SIZE; carsingroup++)
			{
//				ms_pCarGroups[cargrp][carsingroup] = -1;
				SetCarGroupCarIsEmpty(cargrp, carsingroup);
			}
		
			cargrp++;
		}
		ASSERTMSG(carsingroup==0 || carsingroup == MAX_CARGROUP_SIZE, "Error reading cargrp.dat");

	}
	
	CFileMgr::CloseFile(fd);
	
	ASSERTMSG(cargrp == NUM_CAR_GROUPS, "Number of cargroups in cargrp.dat is different from the game");
}


void CTaskSimpleFight::LoadMeleeData()
{
	// better initialise this data to something that won't break the game
	for(int32 i=0; i < MCOMBO_NUM - MCOMBO_UNARMED_1; i++)
	{
		m_aComboData[i].nAnimGroup = ANIM_MELEE_UNARMED_1;
		m_aComboData[i].fGroupRange = 1.5f;
		for(int32 j=0; j<5; j++)
		{
			m_aComboData[i].aFireTime[j] = 100.0f;
			m_aComboData[i].aComboTime[j] = 100.0f;
			m_aComboData[i].aRadius[j] = 1.0f;
			m_aComboData[i].aHitLevel[j] = MHIT_LEVEL_NUM;
			m_aComboData[i].aDamage[j] = 0;
#ifdef USE_AUDIOENGINE
			m_aComboData[i].aHitSound[j] = 0;
			m_aComboData[i].aAltHitSound[j] = 0;
#else //USE_AUDIOENGINE
			m_aComboData[i].aHitSound[j] = AE_FIGHT_1;
			m_aComboData[i].aAltHitSound[j] = AE_FIGHT_1;
#endif //USE_AUDIOENGINE
		}
		m_aComboData[i].aGroundLoopStart = 0.0f;
		m_aComboData[i].aBlockLoopStart = 100.0f;
		m_aComboData[i].aBlockLoopEnd = 100.0f;
		m_aComboData[i].nFlags = 0;
	}
	
	for(int32 i=0; i < MHIT_LEVEL_NUM; i++)
	{
		m_aHitOffset[i] = CVector(0.0f, 0.75f, 0.0f);
	}
	
	uint32 filePosn = 0;
	uint8 lineNo = 0;
	
	char aDataType[32], aTempName[32];
	float fRange;
	float fHitTime, fChainTime, fRadius;
	int32 nDamage, nFlags;
	int32 nHitSound, nAltHitSound;
	float fGroundLoop;
	CVector vecHitLevel;

	int32 fid = CFileMgr::OpenFile("DATA\\melee.dat", "rb");
	char* pLine;
	
	int32 nCurrentCombo = 0;
	int32 nCurrentLine = 0;
	int32 nCurrentAttack = 0;
	bool bReadingCombo = false;
	bool bReadingLevels = false;

	// keep going till we reach the end of the file
	while ((pLine = CFileLoader::LoadLine(fid)))
	{
		// if beginning of line is the end it is empty
		if(*pLine == '#' || *pLine == '\0')
			continue;
			
		if(strncmp(pLine, "END_MELEE_DATA", 14)==0)
			break;

		if(bReadingCombo || bReadingLevels)
		{
			if(strncmp(pLine, "END_COMBO", 9)==0)
			{
				if(bReadingCombo)
					nCurrentCombo++;
				nCurrentLine = 0;
				bReadingCombo = false;
				bReadingLevels = false;
			}
			else if(bReadingLevels)
			{
				sscanf(pLine, "%s %f %f %f", aDataType, &vecHitLevel.x, &vecHitLevel.y, &vecHitLevel.z);
				
				#ifndef FINAL
					if(nCurrentLine==MHIT_LEVEL_HIGH)				{ASSERT(strncmp(aDataType, "LEVEL_H", 7)==0);}
					else if(nCurrentLine==MHIT_LEVEL_LOW)			{ASSERT(strncmp(aDataType, "LEVEL_L", 7)==0);}
					else if(nCurrentLine==MHIT_LEVEL_GROUND)		{ASSERT(strncmp(aDataType, "LEVEL_G", 7)==0);}
					else if(nCurrentLine==MHIT_LEVEL_BEHIND)		{ASSERT(strncmp(aDataType, "LEVEL_B", 7)==0);}
					else if(nCurrentLine==MHIT_LEVEL_HIGH_LONG)		{ASSERT(strncmp(aDataType, "LEVEL_HL", 8)==0);}
					else if(nCurrentLine==MHIT_LEVEL_LOW_LONG)		{ASSERT(strncmp(aDataType, "LEVEL_LL", 8)==0);}
					else if(nCurrentLine==MHIT_LEVEL_GROUND_LONG)	{ASSERT(strncmp(aDataType, "LEVEL_GL", 8)==0);}
					else 											{ASSERT(false);}
				#endif
				m_aHitOffset[nCurrentLine] = vecHitLevel;
				nCurrentLine++;
			}
			else
			{
				nCurrentLine++;
				switch(nCurrentLine)
				{
					case 1:
						sscanf(pLine, "%s %s", aDataType, aTempName);
						ASSERT(strncmp(aDataType, "ANIMGROUP", 9)==0);
						// Get main animation association group
						for(int32 i=0; i<CAnimManager::GetNumAnimAssocDefinitions(); i++)
						{
							if(!strcmp(aTempName, CAnimManager::GetAnimGroupName((AssocGroupId)i)))
							{
								m_aComboData[nCurrentCombo].nAnimGroup = (AssocGroupId)i;
								break;
							}	
						}
						break;
					case 2:
						sscanf(pLine, "%s %f", aDataType, &fRange);
						ASSERT(strncmp(aDataType, "RANGES", 6)==0);
						m_aComboData[nCurrentCombo].fGroupRange = fRange;
						break;

					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
						fGroundLoop = 0.0f;
						sscanf(pLine, "%s %f %f %f %s %d %d %d %f", aDataType, &fHitTime, &fChainTime, &fRadius, aTempName, &nDamage, &nHitSound, &nAltHitSound, &fGroundLoop);
						switch(nCurrentLine)
						{
							case 3:	nCurrentAttack = 0;	ASSERT(strncmp(aDataType, "ATTACK1", 7)==0); break;
							case 4:	nCurrentAttack = 1;	ASSERT(strncmp(aDataType, "ATTACK2", 7)==0); break;
							case 5:	nCurrentAttack = 2;	ASSERT(strncmp(aDataType, "ATTACK3", 7)==0); break;
							case 6:	nCurrentAttack = 3;	ASSERT(strncmp(aDataType, "AGROUND", 7)==0); break;
							case 7:	nCurrentAttack = 4;	ASSERT(strncmp(aDataType, "AMOVING", 7)==0); break;
						}
						
						m_aComboData[nCurrentCombo].aFireTime[nCurrentAttack] = fHitTime*ATTACK_ANIM_RATE;
						m_aComboData[nCurrentCombo].aComboTime[nCurrentAttack] = fChainTime*ATTACK_ANIM_RATE;
						m_aComboData[nCurrentCombo].aRadius[nCurrentAttack] = fRadius;
						m_aComboData[nCurrentCombo].aHitLevel[nCurrentAttack] = GetHitLevel(aTempName);
						m_aComboData[nCurrentCombo].aDamage[nCurrentAttack] = nDamage;
						m_aComboData[nCurrentCombo].aHitSound[nCurrentAttack] = GetHitSound(nHitSound);
						m_aComboData[nCurrentCombo].aAltHitSound[nCurrentAttack] = GetHitSound(nAltHitSound);
						
						if(fGroundLoop > 0.0f)
						{
							ASSERT(m_aComboData[nCurrentCombo].aGroundLoopStart==0.0f);
							m_aComboData[nCurrentCombo].aGroundLoopStart = fGroundLoop*ATTACK_ANIM_RATE;
						}	
						break;
					
					case 8:
						sscanf(pLine, "%s %f %f", aDataType, &fHitTime, &fChainTime);
						ASSERT(strncmp(aDataType, "ABLOCK", 5)==0);
						m_aComboData[nCurrentCombo].aBlockLoopStart = fHitTime*ATTACK_ANIM_RATE;
						m_aComboData[nCurrentCombo].aBlockLoopEnd = fChainTime*ATTACK_ANIM_RATE;
						break;

					case 9:
						sscanf(pLine, "%s %x", aDataType, &nFlags);
						ASSERT(strncmp(aDataType, "FLAGS", 5)==0);
						m_aComboData[nCurrentCombo].nFlags = nFlags;
						break;
				}
			}
		}
		else
		{
			if(strncmp(pLine, "START_COMBO", 11)==0)
			{
				#ifndef FINAL
					sscanf(pLine, "%s %s", aDataType, aTempName);
					ASSERT(strcmp(aTempName, saComboNames[nCurrentCombo])==0);
				#endif
			
				bReadingCombo = true;
			}
			else if(strncmp(pLine, "START_LEVELS", 12)==0)
			{
				bReadingLevels = true;
			}
		}

		lineNo++;
	}
	
	CFileMgr::CloseFile(fid);
}

uint8 CTaskSimpleFight::GetHitLevel(const char *pString)
{
    // WDFIXME : check that the strcmp changes worked
	if(*pString == 'H')
		return MHIT_LEVEL_HIGH;
	else if(*pString == 'L')
		return MHIT_LEVEL_LOW;
	else if(*pString == 'G')
		return MHIT_LEVEL_GROUND;
	else if (*pString == 'B')
		return MHIT_LEVEL_BEHIND;
	else if(!strcmp(pString,"HL"))
		return MHIT_LEVEL_HIGH_LONG;
	else if(!strcmp(pString,"LL"))
		return MHIT_LEVEL_LOW_LONG;
	else if(!strcmp(pString,"GL"))
		return MHIT_LEVEL_GROUND_LONG;
	else
		return MHIT_LEVEL_NUM;
}


#ifdef USE_AUDIOENGINE
Int16 CTaskSimpleFight::GetHitSound(const int32 nHitSound)
{
	tAudioEvent AudioEvent;
	switch(nHitSound)
	{
		case 1:
			AudioEvent = AUDIO_EVENT_PED_HIT_HIGH;
			break;
		case 2:
			AudioEvent = AUDIO_EVENT_PED_HIT_LOW;
			break;
		case 3:
			AudioEvent = AUDIO_EVENT_PED_HIT_GROUND;
			break;
		case 4:
			AudioEvent = AUDIO_EVENT_PED_HIT_GROUND_KICK;
			break;
		case 5:
			AudioEvent = AUDIO_EVENT_PED_HIT_HIGH_UNARMED;
			break;
		case 6:
			AudioEvent = AUDIO_EVENT_PED_HIT_LOW_UNARMED;
			break;
		case 7:
			AudioEvent = AUDIO_EVENT_PED_HIT_MARTIAL_PUNCH;
			break;
		case 8:
			AudioEvent = AUDIO_EVENT_PED_HIT_MARTIAL_KICK;
			break;
		default:
			AudioEvent = AUDIO_EVENT_PED_HIT_LOW;
			break;
	}

	return AudioEvent;
}
#else //USE_AUDIOENGINE
tEvent CTaskSimpleFight::GetHitSound(const int32 nHitSound)
{
	/*switch(nHitSound)
	{
		case 1:		return AE_FIGHT_1;
		case 2:		return AE_FIGHT_2;
		case 4:		return AE_FIGHT_4;
		case 5:		return AE_FIGHT_5;
		
		case 21:	return AE_FIGHT_1_PLUS_2;
		case 22:	return AE_FIGHT_2_PLUS_2;
		case 24:	return AE_FIGHT_4_PLUS_2;
		case 25:	return AE_FIGHT_5_PLUS_2;
		
		case -21:	return AE_FIGHT_1_MINUS_2;
		case -22:	return AE_FIGHT_2_MINUS_2;
		case -24:	return AE_FIGHT_4_MINUS_2;
		case -25:	return AE_FIGHT_5_MINUS_2;
		
		default:	ASSERT(0);
	}
	*/
	return AE_FIGHT_1;
}
#endif //USE_AUDIOENGINE



