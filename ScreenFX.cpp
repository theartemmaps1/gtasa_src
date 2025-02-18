
/****************************************************************************
 *
 *
 * ScreenFX.cpp
 *
 * Original author: David Serafim
 * Reviewed by:
 * Purpose: most amazing screen filters ever seen on the PS2 (not)
 *
 ****************************************************************************/
//
// 12/08/2004	- Andrzej:	- code fixes for calls to _rwDMAOpenVIFPkt();
//
//
//
//
//
//
#include "dmamacros.h"
#include "PostEffects.h"
#include "TimeCycle.h"
#include "skeleton.h"
#include "main.h"
#include "VarConsole.h"
#include "playerped.h"
#include "overlay.h"
#include "lights.h"
#include "zonecull.h"
#include "VehicleModelInfo.h"
#include "cutscenemgr.h"
#include "specialfx.h"
#include "fx.h"
#include "clouds.h"
#include "GameLogic.h"
#include "WaterLevel.h"
#include "FileMgr.h"
#include "main.h"

#if defined (GTA_XBOX)
	#include "FileMgr.h"
	#include "frontend.h"
	#include "sprite.h"
	#include "CustomBuildingRenderer.h"
	#include "time.h"
	#include "xboxglobals.h"
#endif //GTA_XBOX

#ifdef GTA_PC
	#include "win.h"
	#include "FileMgr.h"
	#include "frontend.h"
	#include "sprite.h"
	#include "CustomBuildingRenderer.h"
	#include "VisibilityPlugins.h"
	#include "time.h"
#endif

#ifdef GTA_PS2
	#include "VuCustomBuildingRenderer.h"
	#include "MemoryCard.h"
#endif	
#ifndef MASTER  // only needs to be used if not in a MASTER build
	#include "ps2keyboard.h"  // for keyboard FOV debug stuff
#endif

#ifdef GTA_PC
#define MAX_JPEG_SIZE	(200*1024)
#else
#define MAX_JPEG_SIZE	(80*1024)		// changed from 80 to 200 * 1024 since the screen rez could be much higher on PC than PS2 so produce buger JPEGs - alex
#endif

#ifdef GTA_PC

#define GE_FORCE_5_FIX																\
{																					\
RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP );		\
}

#endif

//#define bCUTSCENE CCutsceneMgr::IsCutsceneRunning()

//#define SHOW_POST_EFFECTS_DEBUG_INFO

#define BLIT_TO_DISPLAYBUFFER

#define DRAW_W SCREEN_DRAW_WIDTH
#define DRAW_H SCREEN_DRAW_HEIGHT
#define DISP_W SCREEN_DISPLAY_WIDTH	
#define DISP_H SCREEN_DISPLAY_HEIGHT
#define DRAW_D 32
#define DISP_D 16
#define MAX_DMADATA 				(21)

#define UNDERWATER_RIPPLE_VALUE 	(0.535f)

extern GlobalScene          Scene;		// In setup.cpp

#if defined (GTA_PS2) || (defined GTA_XBOX)
	void JPegCompressScreenToBuffer(RwCamera* pCamera,char** buffer,unsigned int *size);
#else
	void JPegCompressScreenToFile(RwCamera* pCamera, const char* pFilename);
#endif  // GTA_PC

bool  CPostEffects::m_bSeamRemover               = true;
bool  CPostEffects::m_bSeamRemoverSeamSearchMode = false;
bool  CPostEffects::m_bSeamRemoverDebugMode      = false;
Int32 CPostEffects::m_SeamRemoverMode = 1; // ### Not used anymore! ###
Int32 CPostEffects::m_SeamRemoverShiftTopLeft     = 0;//-1; // Must be 0
Int32 CPostEffects::m_SeamRemoverShiftBottomRight = 1;      // Must be 1
//float CPostEffects::m_fSeamRemoverShiftX = -2.0f; // NEW! (Must be -2.0f)
//float CPostEffects::m_fSeamRemoverShiftY = -1.0f; // NEW! (Must be -1.0f)
//Int32 CPostEffects::m_SeamRemoverType    = 1;     // NEW! (Must be 1)

CPostEffects::imf CPostEffects::ms_imf;

//RwRaster			*CPostEffects::m_pGrayScaleCLUTRaster256 = NULL;
RwRaster			*CPostEffects::m_pGrainRaster = NULL;

bool CPostEffects::m_bGrainEnable = false;
int32 CPostEffects::m_grainStrength = 64;

bool CPostEffects::m_bHilightEnable = false;
//int32 CPostEffects::m_hilightMinIntensity = 255;	Now coming from timecycle
int32 CPostEffects::m_hilightStrength = 128;
int32 CPostEffects::m_hilightScale = 3;
int32 CPostEffects::m_hilightOffset = 2;
bool CPostEffects::m_hilightMblur = false;

//bool CPostEffects::m_NightVision = false;
//RwRGBA CPostEffects::m_NightVisionCol = {32, 32, 32, 32};

float  CPostEffects::m_VisionFXDayNightBalance = 1.0f; // Default = 1.0f

bool   CPostEffects::m_bInCutscene = false;

bool   CPostEffects::m_bNightVision = false; //@@@@

#ifdef GTA_PS2
Int32  CPostEffects::m_NightVisionGrainStrength = 16;
#endif

#ifdef GTA_PC
Int32  CPostEffects::m_NightVisionGrainStrength = 48;
#endif

#ifdef GTA_XBOX
Int32  CPostEffects::m_NightVisionGrainStrength = 8;
#endif

//float  CPostEffects::m_fNightVisionFilterRadius = 0.002f; // Not used!
//RwRGBA CPostEffects::m_NightVisionCol = {32, 32, 32, 32}; // Not used!
RwRGBA CPostEffects::m_NightVisionMainCol = {0, 130, 0, 255}; // Default = (0, 130, 0, 255)
float  CPostEffects::m_fNightVisionSwitchOnFXTime = 50.0f;
float  CPostEffects::m_fNightVisionSwitchOnFXCount = m_fNightVisionSwitchOnFXTime;

/*
bool   CPostEffects::m_bWaveFilter = false;
Int32  CPostEffects::m_WaveFilterSpeed = 4;
Int32  CPostEffects::m_WaveFilterFrequency = 40;
float  CPostEffects::m_fWaveFilterShift = 8.0f;
RwRGBA CPostEffects::m_WaveFilterCol = {255, 255, 255, 128}; // Default = (255, 255, 255, 128)
*/

bool       CPostEffects::m_bInfraredVision = false; //@@@@

#ifdef GTA_PS2
Int32      CPostEffects::m_InfraredVisionGrainStrength = 24;
#endif

#ifdef GTA_PC
Int32      CPostEffects::m_InfraredVisionGrainStrength = 64;
#endif

#ifdef GTA_XBOX
Int32      CPostEffects::m_InfraredVisionGrainStrength = 10;
#endif

float      CPostEffects::m_fInfraredVisionFilterRadius = 0.003f; // Default = 0.003f
RwRGBAReal CPostEffects::m_fInfraredVisionHeatObjectCol = {1.0f, 0.0f, 0.0f, 1.0f}; // Default = (1.0f, 0.0f, 0.0f, 1.0f)
RwRGBA     CPostEffects::m_InfraredVisionCol = {110, 40, 60, 255}; // Default = (110, 40, 60, 255)
RwRGBA     CPostEffects::m_InfraredVisionMainCol = {100, 0, 200, 255}; // Default = (100, 0, 200, 255)

bool       CPostEffects::m_bHeatHazeFX = false;
bool       CPostEffects::m_bHeatHazeMaskModeTest = false;
Int32      CPostEffects::m_HeatHazeFXHourOfDayStart = 10; // Start at 10am
Int32      CPostEffects::m_HeatHazeFXHourOfDayEnd   = 19; // End at 7pm
float      CPostEffects::m_fHeatHazeFXFadeSpeed = 0.05f;  // default = 0.05f
float      CPostEffects::m_fHeatHazeFXInsideBuildingFadeSpeed = 0.5f; // default = 0.5f
Int32      CPostEffects::m_HeatHazeFXType = 0;
Int32      CPostEffects::m_HeatHazeFXTypeLast = -1; // MUST be -1 in order to force a "first time" initialization!
// The following values are "dummies" only! The FX is initialized via "m_HeatHazeFXType"!
Int32      CPostEffects::m_HeatHazeFXIntensity = 150;
Int32      CPostEffects::m_HeatHazeFXRandomShift = 0;
Int32      CPostEffects::m_HeatHazeFXSpeedMin = 6;
Int32      CPostEffects::m_HeatHazeFXSpeedMax = 10;
#ifdef GTA_PS2
	Int32      CPostEffects::m_HeatHazeFXScanSizeX = 24;
	Int32      CPostEffects::m_HeatHazeFXScanSizeY = 24;
	Int32      CPostEffects::m_HeatHazeFXRenderSizeX = 24;
	Int32      CPostEffects::m_HeatHazeFXRenderSizeY = 24;
#else
	Int32      CPostEffects::m_HeatHazeFXScanSizeX = 24 * HUD_MULT_X;
	Int32      CPostEffects::m_HeatHazeFXScanSizeY = 24 * HUD_MULT_Y;
	Int32      CPostEffects::m_HeatHazeFXRenderSizeX = 24 * HUD_MULT_X;
	Int32      CPostEffects::m_HeatHazeFXRenderSizeY = 24 * HUD_MULT_Y;
#endif

bool       CPostEffects::m_bDarknessFilter = false;
Int32      CPostEffects::m_DarknessFilterAlphaDefault = 170;
Int32      CPostEffects::m_DarknessFilterAlpha = m_DarknessFilterAlphaDefault;
Int32      CPostEffects::m_DarknessFilterRadiosityIntensityLimit = 45;

bool       CPostEffects::m_bCCTV = false;
RwRGBA     CPostEffects::m_CCTVcol = { 0, 0, 0, 64 };

bool       CPostEffects::m_bFog = false;

bool       CPostEffects::m_bSpeedFX = true; // This is the main FX flag (could be used in frontend)
bool       CPostEffects::m_bSpeedFXTestMode = false;
//Int32    CPostEffects::m_SpeedFXShift = 2;
#ifdef GTA_PS2
Int32      CPostEffects::m_SpeedFXAlpha = 48; // Must be 48
#else
Int32      CPostEffects::m_SpeedFXAlpha = 36; // Must be 36
#endif

bool       CPostEffects::m_bSpeedFXUserFlag = true; // (default = true) This is the "user" FX flag which can toggle the FX for special camera modes
bool       CPostEffects::m_bSpeedFXUserFlagCurrentFrame = true; // Must be "true"
float      CPostEffects::m_fSpeedFXManualSpeedCurrentFrame = 0.0f; // Must be "0.0f"! This allows to control the speed FX manually without any player movement
#ifdef GTA_PS2
bool       CPostEffects::m_bSpeedFX_DMAPacketErrorMessage_PS2 = false;
#endif

bool       CPostEffects::m_bRadiosity = false;

bool       CPostEffects::m_bRadiosityLinearFilter = true; // Must be "true"
bool       CPostEffects::m_bRadiosityStripCopyMode = true;
//RwRGBA   CPostEffects::m_RadiosityCol           = {180, 180, 180, 128};
#ifdef GTA_PS2
Int32      CPostEffects::m_RadiosityPixelsX            = SCREEN_DRAW_WIDTH;
Int32      CPostEffects::m_RadiosityPixelsY            = SCREEN_DRAW_HEIGHT;
#else
Int32      CPostEffects::m_RadiosityPixelsX            = SCREEN_WIDTH;
Int32      CPostEffects::m_RadiosityPixelsY            = SCREEN_HEIGHT;
#endif
//
Int32      CPostEffects::m_RadiosityFilterPasses       = 2;   // Can be 0-5
Int32      CPostEffects::m_RadiosityRenderPasses       = 1;   // Can be 1-...
Int32      CPostEffects::m_RadiosityIntensityLimit     = 220;
Int32      CPostEffects::m_RadiosityIntensity          = 35;//200;
Int32      CPostEffects::m_RadiosityFilterUCorrection  = 2;   // Must be 2
Int32      CPostEffects::m_RadiosityFilterVCorrection  = 2;   // Must be 2
bool       CPostEffects::m_bRadiosityDebug             = false;
bool       CPostEffects::m_bRadiosityBypassTimeCycleIntensityLimit = false;
bool       CPostEffects::m_bDisableAllPostEffect = false;
bool       CPostEffects::m_bSavePhotoFromScript = false;

#ifndef MASTER
/*
bool	   CPostEffects::m_bShowDebugText = false;
Int32	   CPostEffects::m_ShowDebugTextTime = 0;
*/
#endif

bool CPostEffects::m_bRainEnable =false;

bool CPostEffects::m_smokeyEnable =false;
int32 CPostEffects::m_smokeyStrength = 128;
int32 CPostEffects::m_smokeyDistance = 75;

bool   CPostEffects::m_waterEnable = FALSE;
int32  CPostEffects::m_waterStrength = 64;
float  CPostEffects::m_xoffset = 4;//6;
float  CPostEffects::m_yoffset = 24;
float  CPostEffects::m_waterSpeed = 0.0015f;
float  CPostEffects::m_waterFreq = 0.04f;
RwRGBA CPostEffects::m_waterCol = {64, 64, 64, 64};
//RwRGBA CPostEffects::m_waterCol = {108, 108, 108, 255};
bool   CPostEffects::m_bWaterDepthDarkness = true;
float  CPostEffects::m_fWaterFullDarknessDepth = 90.0f;
float  CPostEffects::m_fWaterFXStartUnderWaterness = UNDERWATER_RIPPLE_VALUE; // This var. is used in the function "GetWaterFXStartUnderWaterness"

int32 CPostEffects::m_colourLeftUOffset=8;
int32 CPostEffects::m_colourRightUOffset=8;
int32 CPostEffects::m_colourTopVOffset=8;
int32 CPostEffects::m_colourBottomVOffset=8;

float CPostEffects::m_colour1Multiplier = 1.0f;
float CPostEffects::m_colour2Multiplier = 1.0f;

#ifndef FINAL
	bool8 CPostEffects::display_kbd_debug = false;
#endif

RwUInt128	 *CPostEffects::m_pDmaPkt = NULL;
int32		CPostEffects::m_defScreenXPosn;
int32		CPostEffects::m_defScreenYPosn;

float CPostEffects::SCREEN_EXTRA_MULT_CHANGE_RATE = 0.0005f;//0.01f;
float CPostEffects::SCREEN_EXTRA_MULT_BASE_CAP	= 0.35f;//0.5f;
float CPostEffects::SCREEN_EXTRA_MULT_BASE_MULT = 1.0f;//3.0f;

#if !defined (GTA_PS2)
	RwRaster *CPostEffects::pRasterFrontBuffer = NULL;
	static	RwIm2DVertex pe_vertex[4];
	static	RwImVertexIndex pe_index[6] = {0, 1, 2, 0, 2, 3};
	static	RwIm2DVertex rastercopy_vertex[4];
	static	RwImVertexIndex rastercopy_index[6] = {0, 1, 2, 0, 2, 3};
	bool CPostEffects::m_bColorEnable = TRUE;
#endif //!GTA_PS2

//!PC - need above static to be inited for PC
#if defined (GTA_PS2)
extern rwDMA_flipData _rwDMAFlipData;
#endif

#ifdef GTA_XBOX
RwRaster* CPostEffects::pRasterFrontBuffer2 = NULL;						// The offscreen buffer 
float CPostEffects::gRasterDestCoords[4] = {0.0f,0.0f,640.0f,480.0f};	// coords to write into the offscreen buffer
RwRGBA CPostEffects::gColRasterCopy = {255,255,255,255};				// the colour of the offscreen raster
RwCamera* CPostEffects::pCam2 = NULL;									// The camera used for copying the raster
RwFrame* CPostEffects::frameCameraPE = NULL;							// This new camera frame
#endif

static	RwIm2DVertex bugfix_vertex[4];

//---------------------------------------------------------------------------------------------------------
// Dw - this is created for the different ways in which the raster must be created for different platforms.
RwRaster* CPostEffects::RasterCreatePostEffects(RwRect rect)
{
	#ifdef GTA_XBOX // DW - because of the platform specific memoery architecture of the XBOX, the tiled format of the front buffer means that we have to create this raster in a different way		
		/* We create a fake texture raster and take the backbuffer adress to perform motion blur
		* On Xbox this raster is linear so we can only use clamp mode and tex coords should
		* go from 0,0 to width,height of the raster
		*/
		//#define USE_FRONTBUFFER
		RwRaster *raster = RwRasterCreate(0, 0, 0, rwRASTERTYPETEXTURE | rwRASTERDONTALLOCATE);
		RwXboxRasterExtension *rasExt = RwXboxRasterGetExtension(raster);
		D3DSURFACE_DESC desc;
		#ifdef USE_FRONTBUFFER
			D3DDevice_GetBackBuffer(-1, D3DBACKBUFFER_TYPE_MONO, (IDirect3DSurface8 **)&rasExt->texture);
		#else
			D3DDevice_GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, (IDirect3DSurface8 **)&rasExt->texture);
		#endif
		D3DSurface_Release((IDirect3DSurface8 *)rasExt->texture);

		D3DSurface_GetDesc((IDirect3DSurface8 *)rasExt->texture, &desc);
		raster->width = desc.Width;
		raster->height = desc.Height;

#ifdef GTA_XBOX
		if (RsXboxGlobalInfo.deviceConfig.multiSampleType != D3DMULTISAMPLE_NONE)
		{
			// anti-aliasing uses a larger back buffer 
			raster->width /= RsXboxGlobalInfo.backbuffer_x_scale; 
			raster->height /= RsXboxGlobalInfo.backbuffer_y_scale; 
		}
#endif
		pRasterFrontBuffer2 = raster;

		return RwRasterCreate(rect.w, rect.h, 0, rwRASTERTYPECAMERATEXTURE);
	#else
		return RwRasterCreate(rect.w, rect.h, RwRasterGetDepth(RwCameraGetRaster(Scene.camera)), rwRASTERTYPECAMERATEXTURE);
	#endif
}

#ifdef GTA_XBOX
//---------------------------------------------------------------------------------------------------------
void CPostEffects::RasterPostEffectsCreateCameraAndImmediateModeVerts(RwRaster *pDst)
{
	RwRaster *pSrc = pRasterFrontBuffer2;
	pCam2 = RwCameraCreate();
	if( pCam2 )
	{
		frameCameraPE = RwFrameCreate();
		RwFrameSetIdentity(frameCameraPE);
		RwCameraSetFrame(pCam2, frameCameraPE);
		RwCameraSetRaster(pCam2, pDst);

		{ // setup immediate mode structs
			/* Grab the raster size (camera textures get rounded up to
			* a power of two in size) so we can calc UVs correctly: */
			RwReal w = RwRasterGetWidth(pDst);
			RwReal h = RwRasterGetHeight(pDst);

			RwReal w1 = RwRasterGetWidth(pSrc);
			RwReal h1 = RwRasterGetHeight(pSrc);

			RwReal U = (w1 + 0.5f);
			RwReal V = (h1 + 0.5f);
			RwReal u = (0.5f);
			RwReal v = (0.5f);

#ifdef GTA_XBOX
			if (RsXboxGlobalInfo.deviceConfig.multiSampleType != D3DMULTISAMPLE_NONE)
			{
				// anti-aliasing uses a larger back buffer 
				U = (w1*RsXboxGlobalInfo.backbuffer_x_scale + 0.5f);
				V = (h1*RsXboxGlobalInfo.backbuffer_y_scale  + 0.5f);
				w1 /= RsXboxGlobalInfo.backbuffer_x_scale;
				h1 /= RsXboxGlobalInfo.backbuffer_y_scale;
			}
#endif
			RwIm2DVertexSetScreenX(	&rastercopy_vertex[0], 0.0f);
			RwIm2DVertexSetScreenY(	&rastercopy_vertex[0], 0.0f);
			RwIm2DVertexSetU(		&rastercopy_vertex[0], u, 1.0f);
			RwIm2DVertexSetV(		&rastercopy_vertex[0], v, 1.0f);

			RwIm2DVertexSetScreenX(	&rastercopy_vertex[1], 0.0f);
			RwIm2DVertexSetScreenY(	&rastercopy_vertex[1], h1);
			RwIm2DVertexSetU(		&rastercopy_vertex[1], u, 1.0f);
			RwIm2DVertexSetV(		&rastercopy_vertex[1], V, 1.0f);

			RwIm2DVertexSetScreenX(	&rastercopy_vertex[2], w1);
			RwIm2DVertexSetScreenY(	&rastercopy_vertex[2], h1);
			RwIm2DVertexSetU(		&rastercopy_vertex[2], U, 1.0f);
			RwIm2DVertexSetV(		&rastercopy_vertex[2], V, 1.0f);

			RwIm2DVertexSetScreenX(	&rastercopy_vertex[3], w1);
			RwIm2DVertexSetScreenY(	&rastercopy_vertex[3], 0.0f);
			RwIm2DVertexSetU(		&rastercopy_vertex[3], U, 1.0f);
			RwIm2DVertexSetV(		&rastercopy_vertex[3], v, 1.0f);

			for (int32 i=0;i<4;i++)
			{
				RwIm2DVertexSetScreenZ(		&rastercopy_vertex[i], RwIm2DGetNearScreenZ());
				RwIm2DVertexSetCameraZ(		&rastercopy_vertex[i], RwCameraGetNearClipPlane(pCam2));
				RwIm2DVertexSetRecipCameraZ(&rastercopy_vertex[i], 1.0f/RwCameraGetNearClipPlane(pCam2));
				RwIm2DVertexSetIntRGBA(		&rastercopy_vertex[i], gColRasterCopy.red, gColRasterCopy.green, gColRasterCopy.blue, gColRasterCopy.alpha);
			}
		}
	}
	ASSERT(pCam2);
}

//---------------------------------------------------------------------------------------------------------
void CPostEffects::RasterPostEffectsDestroyCamera()
{
	if (frameCameraPE)
	{
		RwFrameDestroy(frameCameraPE);
		frameCameraPE = NULL;
	}
	
	if (pCam2)
	{
		RwCameraDestroy(pCam2);
		pCam2 = NULL;
	}
}

//----------------------------------------------------------------
// Dw - Copy from the screen raster to the normal offscreen raster
void CPostEffects::RasterCopyPostEffects()
{
	RwCameraEndUpdate(Scene.camera);
	RwCameraBeginUpdate(pCam2);

	DefinedState(); // just for paranoia
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,	(void *)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,		(void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,		(void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,		(void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,	(void *)pRasterFrontBuffer2); // the texture src
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)FALSE); // probably quicker!!!!... everybody should do this!
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS,   (void *)rwTEXTUREADDRESSCLAMP ); // MUST BE CLAMPED FOR A CAMERA RASTER!
	RwRenderStateSet(rwRENDERSTATECULLMODE,			(void *)rwCULLMODECULLNONE);

	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, rastercopy_vertex, 4, rastercopy_index, 6);

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,		(void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,		(void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,	(void *)NULL);    

	RwCameraEndUpdate(pCam2);
	RwCameraBeginUpdate(Scene.camera);

//	This writes the contents of the offscreen raster to a bitmap image, so you can see what's in it.
//  NB: you must also add the library rtbmp.lib (JG)
/*
	#include "rtbmp.h"

	RwRaster *raster = RwCameraGetRaster(pCam2);
	RwImage *image;
	RwInt32 x, y;
  
    x = RwRasterGetWidth(raster);
    y = RwRasterGetHeight(raster);
    image = RwImageCreate(x, y, 32);

    if( image )
    {
		RwImageRegisterImageFormat("bmp", RtBMPImageRead,RtBMPImageWrite);
        RwImageAllocatePixels(image);
        if (!RwImageSetFromRaster(image, raster))
			ASSERT(0);
		if (!RwImageWrite(image, "D:\\raster.bmp"))
			ASSERT(0);
        RwImageDestroy(image);
    }
*/
}
#endif


void CPostEffects::SetupBackBufferVertex()
{
	RwRect rect = {0, 0, 0, 0};
    RwInt32 logWidth, logHeight;
	rect.w = RwRasterGetWidth(RwCameraGetRaster(Scene.camera));
	rect.h = RwRasterGetHeight(RwCameraGetRaster(Scene.camera));
    logWidth  = (RwUInt32)(log((float)rect.w) /log(2.0f));
    logHeight = (RwUInt32)(log((float)rect.h)/log(2.0f));
    rect.w  = (RwUInt32)powf(2, logWidth+1);
    rect.h = (RwUInt32)powf(2, logHeight+1);

	if (pRasterFrontBuffer && (rect.w != RwRasterGetWidth(pRasterFrontBuffer) || rect.h != RwRasterGetHeight(pRasterFrontBuffer)))
	{
       RwRasterDestroy(pRasterFrontBuffer);
       pRasterFrontBuffer = NULL;
	}
	
	if (!pRasterFrontBuffer)
	{
		pRasterFrontBuffer = RasterCreatePostEffects(rect);
#ifdef GTA_XBOX
		RasterPostEffectsCreateCameraAndImmediateModeVerts(pRasterFrontBuffer);
#endif

		if (!pRasterFrontBuffer)
		{
         	printf(RWSTRING("Error subrastering\n"));
        	RwRasterDestroy(pRasterFrontBuffer);
        	pRasterFrontBuffer = NULL;
		}
	}

    RwReal w = rect.w;
    RwReal h = rect.h;

    RwReal U, V, u, v;
    RwReal zero = 0.0f;
    V = (h+0.5f)/rect.h;
    v = (0.5f)/rect.h;

    U = (w+0.5f)/rect.w;
    u = (0.5f)/rect.w;

	RwIm2DVertexSetScreenX(&pe_vertex[0], zero);
    RwIm2DVertexSetScreenY(&pe_vertex[0], zero);
    RwIm2DVertexSetScreenZ(&pe_vertex[0], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex[0], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex[0], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetU(&pe_vertex[0], u, 1.0f);
    RwIm2DVertexSetV(&pe_vertex[0], v, 1.0f);

    RwIm2DVertexSetScreenX(&pe_vertex[1], zero);
    RwIm2DVertexSetScreenY(&pe_vertex[1], h);
    RwIm2DVertexSetScreenZ(&pe_vertex[1], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex[1], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex[1], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetU(&pe_vertex[1], u, 1.0f);
    RwIm2DVertexSetV(&pe_vertex[1], V, 1.0f);

    RwIm2DVertexSetScreenX(&pe_vertex[2], w);
    RwIm2DVertexSetScreenY(&pe_vertex[2], h);
    RwIm2DVertexSetScreenZ(&pe_vertex[2], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex[2], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex[2], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetU(&pe_vertex[2], U, 1.0f);
    RwIm2DVertexSetV(&pe_vertex[2], V, 1.0f);

    RwIm2DVertexSetScreenX(&pe_vertex[3], w);
    RwIm2DVertexSetScreenY(&pe_vertex[3], zero);
    RwIm2DVertexSetScreenZ(&pe_vertex[3], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex[3], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex[3], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetU(&pe_vertex[3], U, 1.0f);
    RwIm2DVertexSetV(&pe_vertex[3], v, 1.0f);

#ifdef GTA_XBOX
	// if antialiasing is enabled, alter the vertex positions to counter a bug in RW 2d im-mode rendering
	if (RsXboxGlobalInfo.deviceConfig.multiSampleType != D3DMULTISAMPLE_NONE)
		Modify2DPrimitiveVertices(pe_vertex, 4);
#endif

	if (pRasterFrontBuffer)
    {
		ImmediateModeFilterStuffInitialize();
		HeatHazeFXInit();
    }
}

//----------------------------------------------------------------------
// This function is to be called after the level has loaded
//----------------------------------------------------------------------

void CPostEffects::Initialise()
{
#ifdef GTA_PS2
	HeatHazeFXInit();
	ImmediateModeFilterStuffInitialize();

/*	RwRect rect;
	
	rect.w = RwRasterGetWidth(RwCameraGetRaster(Scene.camera));
	rect.h = RwRasterGetHeight(RwCameraGetRaster(Scene.camera));
	m_pFrontBuffer = RwRasterCreate(0, 0, 0, 
    	rwRASTERTYPECAMERATEXTURE | rwRASTERDONTALLOCATE);

	if( m_pFrontBuffer )
	{
    	if( !RwRasterSubRaster(m_pFrontBuffer, RwCameraGetRaster(Scene.camera), &rect) )
    	{
        	printf(RWSTRING("Error subrastering\n"));
        	RwRasterDestroy(m_pFrontBuffer);
        
        	m_pFrontBuffer = NULL;
        
        	return;
    	}
	}*/
/*	// screen filters mask (circle)
//	CTxdStore::PushCurrentTxd();
//	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));
//  	m_pScreenFilterMaskTex = RwTextureRead(RWSTRING("zoommask"), NULL );
  	ASSERT(m_pScreenFilterMaskTex);
//	CTxdStore::PopCurrentTxd();
    
	// 256 colour grayscale palette
	RwRaster *pRast = RwTextureGetRaster(m_pScreenFilterMaskTex);
	RwRGBA *pPalette = (RwRGBA *) RwRasterLockPalette(pRast, rwRASTERLOCKREAD);
	m_pGrayScaleCLUTRaster256 = RwRasterCreate(16, 16, 32, rwRASTERTYPETEXTURE | rwRASTERFORMAT8888);
//	m_pGrayScaleCLUTRaster16 = RwRasterCreate(8, 2, 32, rwRASTERTYPETEXTURE | rwRASTERFORMAT8888);
	if ( !m_pGrayScaleCLUTRaster256 )
		return FALSE;
	RwRGBA *pixels = (RwRGBA *) RwRasterLock(m_pGrayScaleCLUTRaster256, 0, rwRASTERLOCKWRITE);
	for (RwInt32 i=0;i<256;i++)
		pixels[i].red = pixels[i].green = pixels[i].blue = pixels[i].alpha = pPalette[i].red / 4;
	RwRasterUnlock(m_pGrayScaleCLUTRaster256);
	RwRasterUnlockPalette(pRast);*/

  #ifndef FINAL
	// debug stuff
	VarConsole.Add("Disable Post FX", &m_bDisableAllPostEffect, true, VC_RENDER);

	VarConsole.Add("Colour Left X Offset", &m_colourLeftUOffset, 1, -64, 64, true, VC_RENDER);
	VarConsole.Add("Colour Top Y Offset", &m_colourTopVOffset, 1, -64, 64, true, VC_RENDER);
	VarConsole.Add("Colour Right X Offset", &m_colourRightUOffset, 1, -64, 64, true, VC_RENDER);
	VarConsole.Add("Colour Bottom Y Offset", &m_colourBottomVOffset, 1, -64, 64, true, VC_RENDER);

	VarConsole.Add("PostEffects Lighting ChangeRate: ", &SCREEN_EXTRA_MULT_CHANGE_RATE, (float) 0.005f, (float) 0.001f, (float) 0.1f, TRUE, VC_RENDER);
	VarConsole.Add("PostEffects Lighting BaseCap: ", &SCREEN_EXTRA_MULT_BASE_CAP, (float) 0.05f, (float) 0.1f, (float) 1.0f, TRUE, VC_RENDER);
	VarConsole.Add("PostEffects Lighting BaseMult: ", &SCREEN_EXTRA_MULT_BASE_MULT, (float) 0.1f, (float) 0.1f, (float) 10.0f, TRUE, VC_RENDER);
	VarConsole.Add("RGB1 Multiplier: ", &m_colour1Multiplier, (float) 0.1f, (float) 0.1f, (float) 10.0f, TRUE, VC_RENDER);
	VarConsole.Add("RGB2 Multiplier: ", &m_colour2Multiplier, (float) 0.1f, (float) 0.1f, (float) 10.0f, TRUE, VC_RENDER);

	VarConsole.Add("Seam Remover", &m_bSeamRemover, true, VC_RENDER);
	VarConsole.Add("Seam Remover Seam Search Mode (color)", &m_bSeamRemoverSeamSearchMode, true, VC_RENDER);
	VarConsole.Add("Seam Remover Debug Mode", &m_bSeamRemoverDebugMode, true, VC_RENDER);
//	VarConsole.Add("Seam Remover Mode", &m_SeamRemoverMode, 1, 0, 1, true, VC_RENDER);
//	VarConsole.Add("Seam Shift TL", &m_SeamRemoverShiftTopLeft, 1, -10, 10, true, VC_RENDER);
//	VarConsole.Add("Seam Shift BR", &m_SeamRemoverShiftBottomRight, 1, -10, 10, true, VC_RENDER);
//	VarConsole.Add("Seam Shift X", &m_fSeamRemoverShiftX, 1.0f, -10.0f, 10.0f, true, VC_RENDER);
//	VarConsole.Add("Seam Shift Y", &m_fSeamRemoverShiftY, 1.0f, -10.0f, 10.0f, true, VC_RENDER);
//	VarConsole.Add("Seam Remover Type", &m_SeamRemoverType, 1, 0, 1, true, VC_RENDER);

	VarConsole.Add("Grain on/off", &m_bGrainEnable, true, VC_RENDER);
	VarConsole.Add("Grain strength", &m_grainStrength, 1, 0, 255, true, VC_RENDER);

	VarConsole.Add("Hilight on/off", &m_bHilightEnable, true, VC_RENDER);
//	VarConsole.Add("Hilight min intensity", &m_hilightMinIntensity, 1, 0, 255, true, VC_RENDER);	Now coming from timecycle
	VarConsole.Add("Hilight strength", &m_hilightStrength, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Hilight scale", &m_hilightScale, 1, 0, 32, true, VC_RENDER);
	VarConsole.Add("Hilight offset", &m_hilightOffset, 1, 0, 32, true, VC_RENDER);
	VarConsole.Add("Hilight motion blur", &m_hilightMblur, true, VC_RENDER);

	VarConsole.Add("Heat Haze FX", &m_bHeatHazeFX, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Mask Mode (test)", &m_bHeatHazeMaskModeTest, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Type", &m_HeatHazeFXType, 1, 0, 4, true, VC_RENDER);
	/*
	VarConsole.Add("Heat Haze FX Intensity", &m_HeatHazeFXIntensity, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Random Shift", &m_HeatHazeFXRandomShift, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Speed Min.", &m_HeatHazeFXSpeedMin, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Speed Max.", &m_HeatHazeFXSpeedMax, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Scan Size X", &m_HeatHazeFXScanSizeX, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Scan Size Y", &m_HeatHazeFXScanSizeY, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Render Size X", &m_HeatHazeFXRenderSizeX, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Heat Haze FX Render Size Y", &m_HeatHazeFXRenderSizeY, 1, 0, 255, true, VC_RENDER);
//	*/

	VarConsole.Add("Darkness Filter", &m_bDarknessFilter, true, VC_RENDER);
	VarConsole.Add("Darkness Filter Alpha", &m_DarknessFilterAlpha, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Darkness Filter Radiosity Intensity Limit", &m_DarknessFilterRadiosityIntensityLimit, 1, 0, 255, true, VC_RENDER);

//	VarConsole.Add("CCTV FX", &m_bCCTV, true, VC_RENDER);

	VarConsole.Add("Speed FX", &m_bSpeedFX, true, VC_RENDER);
	VarConsole.Add("Speed FX Test Mode", &m_bSpeedFXTestMode, true, VC_RENDER);
//	VarConsole.Add("Speed FX shift", &m_SpeedFXShift, 1, 1, 48, true, VC_RENDER);
	VarConsole.Add("Speed FX alpha", &m_SpeedFXAlpha, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Speed FX Manual Flag", &m_bSpeedFXUserFlag, true, VC_RENDER);
//	VarConsole.Add("Speed FX Manual Control", &m_fSpeedFXManualSpeedCurrentFrame, 0.05f, 0.0f, 1.0f, true, VC_RENDER);

	VarConsole.Add("Radiosity", &m_bRadiosity, true, VC_RENDER);
	VarConsole.Add("Radiosity Strip Copy Mode", &m_bRadiosityStripCopyMode, true, VC_RENDER);
	VarConsole.Add("Radiosity Debug mode", &m_bRadiosityDebug, true, VC_RENDER);

//	/*
	VarConsole.Add("Radiosity Filter Passes", &m_RadiosityFilterPasses, 1, 0, 5, true, VC_RENDER);
	VarConsole.Add("Radiosity Render Passes", &m_RadiosityRenderPasses, 1, 1, 20, true, VC_RENDER);
	VarConsole.Add("Radiosity Intensity", &m_RadiosityIntensity, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Radiosity Intensity Limit", &m_RadiosityIntensityLimit, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Radiosity Bypass Time Cycle Intensity Limit", &m_bRadiosityBypassTimeCycleIntensityLimit, true, VC_RENDER);
//	*/

/*
//	VarConsole.Add("Radiosity U corr.", &m_RadiosityFilterUCorrection, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Radiosity V corr.", &m_RadiosityFilterVCorrection, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Radiosity Linear Filter", &m_bRadiosityLinearFilter, true, VC_RENDER);
//	VarConsole.Add("Radiosity Red", &m_RadiosityCol.red, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Radiosity Green", &m_RadiosityCol.green, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Radiosity Blue", &m_RadiosityCol.blue, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Radiosity Alpha", &m_RadiosityCol.alpha, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Radiosity X pixels", &m_RadiosityPixelsX, 8, 0, 1024,true, VC_RENDER);
//	VarConsole.Add("Radiosity Y pixels", &m_RadiosityPixelsY, 8, 0, 1024,true, VC_RENDER);
*/

/*
	VarConsole.Add("Wave Filter",       &m_bWaveFilter, true, VC_RENDER);
	VarConsole.Add("Wave Filter Speed", &m_WaveFilterSpeed, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Wave Filter Freq.", &m_WaveFilterFrequency, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Wave Filter Shift", &m_fWaveFilterShift, 0.1f, 0.0f, 32.0f, true, VC_RENDER);
*/

//	VarConsole.Add("Vision FX Day/Night balance", &m_VisionFXDayNightBalance, 0.01f, 0.0f, 1.0f,true, VC_RENDER);

	VarConsole.Add("Night Vision", &m_bNightVision, true, VC_RENDER);
///	VarConsole.Add("Night Vision Grain Strength", &m_NightVisionGrainStrength, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Night Vision Radius", &m_fNightVisionFilterRadius, 0.001f, 0.0f, 255.0f,true, VC_RENDER);
//	VarConsole.Add("Night Vision Red", &m_NightVisionCol.red, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Night Vision Green", &m_NightVisionCol.green, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Night Vision Blue", &m_NightVisionCol.blue, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Night Vision Alpha", &m_NightVisionCol.alpha, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Main Red", &m_NightVisionMainCol.red, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Main Green", &m_NightVisionMainCol.green, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Main Blue", &m_NightVisionMainCol.blue, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Night Vision Main Alpha", &m_NightVisionMainCol.alpha, 1, 0, 255, true, VC_RENDER);

/*
	VarConsole.Add("Night Vision", &m_NightVision, true, VC_RENDER);
	VarConsole.Add("Night Vision Red", &m_NightVisionCol.red, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Green", &m_NightVisionCol.green, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Blue", &m_NightVisionCol.blue, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Alpha", &m_NightVisionCol.alpha, 1, 0, 255, true, VC_RENDER);
*/

	VarConsole.Add("Infrared Vision", &m_bInfraredVision, true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Grain Strength", &m_InfraredVisionGrainStrength, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Radius", &m_fInfraredVisionFilterRadius, 0.001f, 0.0f, 255.0f,true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Heat Obj. Red", &m_fInfraredVisionHeatObjectCol.red, 0.01f, 0.0f, 1.0f,true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Heat Obj. Green", &m_fInfraredVisionHeatObjectCol.green, 0.01f, 0.0f, 1.0f,true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Heat Obj. Blue", &m_fInfraredVisionHeatObjectCol.blue, 0.01f, 0.0f, 1.0f,true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Red", &m_InfraredVisionCol.red, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Green", &m_InfraredVisionCol.green, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Blue", &m_InfraredVisionCol.blue, 1, 0, 255, true, VC_RENDER);
///	VarConsole.Add("Infrared Vision Alpha", &m_InfraredVisionCol.alpha, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Main Red", &m_InfraredVisionMainCol.red, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Main Green", &m_InfraredVisionMainCol.green, 1, 0, 255, true, VC_RENDER);
//	VarConsole.Add("Infrared Vision Main Blue", &m_InfraredVisionMainCol.blue, 1, 0, 255, true, VC_RENDER);
///	VarConsole.Add("Infrared Vision Main Alpha", &m_InfraredVisionMainCol.alpha, 1, 0, 255, true, VC_RENDER);

//	VarConsole.Add("Save photos to memory card", &m_SavePhotoInGallery, true, VC_RENDER);
//	VarConsole.Add("Smokey Filter Enable", &m_smokeyEnable, true, VC_RENDER);
//	VarConsole.Add("Smokey strength", &m_smokeyStrength, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Smokey Filter Enable", &m_smokeyEnable, true, VC_RENDER);
	VarConsole.Add("Smokey Distance", &m_smokeyDistance, 1, 0, 1000, true, VC_RENDER);
	VarConsole.Add("Smokey strength", &m_smokeyStrength, 1, 0, 255, true, VC_RENDER);

	VarConsole.Add("Water strength", &m_waterStrength, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Water X Offset", &m_xoffset, 0.1f, 0.0f, SCREEN_DRAW_WIDTH, true, VC_RENDER);
	VarConsole.Add("Water Y Offset", &m_yoffset, 0.1f, 0.0f, SCREEN_DRAW_HEIGHT, true, VC_RENDER);
	VarConsole.Add("Water Speed", &m_waterSpeed, 0.0001, 0, 1.0f, true, VC_RENDER);
	VarConsole.Add("Water Frequency", &m_waterFreq, 0.001, 0, 1.0f, true, VC_RENDER);
	VarConsole.Add("Water Depth Darkness", &m_bWaterDepthDarkness, true, VC_RENDER);
	VarConsole.Add("Water Full Darkness Depth", &m_fWaterFullDarknessDepth, 1, 1, 500, true, VC_RENDER);

	display_kbd_debug = false;
  #else
  
#ifndef MASTER

	VarConsole.Add("Speed FX", &m_bSpeedFX, true, VC_RENDER);
	VarConsole.Add("Speed FX Test Mode", &m_bSpeedFXTestMode, true, VC_RENDER);
    #ifdef GTA_PS2
	  VarConsole.Add("Speed FX DMA error message", &m_bSpeedFX_DMAPacketErrorMessage_PS2, true, VC_RENDER);
	#endif

#endif
	VarConsole.Add("Colour filter on/off", &m_bColorEnable, true, VC_RENDER);
	VarConsole.Add("Hilight on/off", &m_bHilightEnable, true, VC_RENDER);
  #endif // FINAL
#else

	CPostEffects::SetupBackBufferVertex();

#endif
	// grain texture
	RwRGBA *pixels;

	#if defined (GTA_PC) || defined (GTA_XBOX)
		// ### PC & XBOX ###
		Int32 grain_w = GRAIN_RASTER_SIZE;
		Int32 grain_h = GRAIN_RASTER_SIZE;
	#else
		// ### PS2 ###
		Int32 grain_w = GRAIN_RASTER_SIZE * HUD_MULT_X;
		Int32 grain_h = GRAIN_RASTER_SIZE * HUD_MULT_Y;
	#endif

	m_pGrainRaster = RwRasterCreate(grain_w, grain_h, 32, rwRASTERTYPETEXTURE | rwRASTERFORMAT8888);
	if ( !m_pGrainRaster )
		return;
	pixels = (RwRGBA *)RwRasterLock(m_pGrainRaster, 0, rwRASTERLOCKWRITE);
	for (RwInt32 i=0;i<grain_w*grain_h;i++)
	{
		pixels[i].red = pixels[i].green = pixels[i].blue = pixels[i].alpha = rand();
	}
	RwRasterUnlock(m_pGrainRaster);

	/* This is used for PC/XBOX debug

	VarConsole.Add("Infrared Vision", &m_bInfraredVision, true, VC_RENDER);
	VarConsole.Add("Infrared Vision Grain Strength", &m_InfraredVisionGrainStrength, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Infrared Vision Radius", &m_fInfraredVisionFilterRadius, 0.001f, -255.0f, 255.0f,true, VC_RENDER);

	VarConsole.Add("Night Vision", &m_bNightVision, true, VC_RENDER);
	VarConsole.Add("Night Vision Main Red", &m_NightVisionMainCol.red, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Main Green", &m_NightVisionMainCol.green, 1, 0, 255, true, VC_RENDER);
	VarConsole.Add("Night Vision Main Blue", &m_NightVisionMainCol.blue, 1, 0, 255, true, VC_RENDER);

//	*/
}

#ifdef GTA_PC
#endif

#ifdef GTA_PS2
//
// name:		SetUpVideoModeForEngine
// description:	Setup RW video mode structure 
void CPostEffects::SetUpVideoModeForEngine()
{
#ifdef BLIT_TO_DISPLAYBUFFER
	RwSkyVideoMode *cm;
	
	cm = RpSkyGetModeStructurePtr();
	
	cm->draw.width = SCREEN_DRAW_WIDTH;
	cm->draw.height = SCREEN_DRAW_HEIGHT;
	cm->draw.depth = 32;
	cm->draw.flags = (RwVideoModeFlag) 0; // Only rwVIDEOMODEFFINTERLACE to indicate half offset might be used here 
	
	cm->draw.format = rwRASTERFORMATDEFAULT;
	cm->display.width = DISP_W;
	cm->display.height = DISP_H;//512;
	cm->display.depth = 16;
	
	cm->display.flags = (RwVideoModeFlag) (rwVIDEOMODEINTERLACE);
	cm->display.format = rwRASTERFORMATDEFAULT;

	// The display setup routines won't change the Sony libs choice of DISPLAY1/DISPLAY2 MAGH fields, so we'll have to change them after RwEngineStart() 
	cm->numDrawBuffers = 1;
	cm->numDisplayBuffers = 1;
	cm->tvSystem = 0; // Leave the system to guess based on height 
	cm->inplace = 0;

/*   SCE_GS_SET_DIMX(-4.0,  0.0, -3.0,  1.0,
				    2.0, -2.0, -3.0, -1.0,
				   -2.5,  1.5, -3.5,  0.5,
				    3.5, -0.5,  2.5, -1.5);


   SCE_GS_SET_DIMX(2,-4, 3,-2,
 				   -4, 1,-1,-3,
 				   3,-1, 1,-4,
				   -2,-3,-4, 2); 
	SCE_GS_SET_DIMX(-4.0, 2.0, -3.0, 3.0,
					0, -2.0, 1.0, -1.0,
					-3.0, 3.0, -4.0, 2.0,
					1.0, -1.0, 0.0, -2.0);*/

   	SCE_GS_SET_DTHE(1);
	// !In an RWDEBUG build this will assert! Warns that you have selected a non-standard mode 
	RwEngineSetVideoMode(-1);
#else
	int32 i;
	RwVideoMode	videoMode;
	int32 num = RwEngineGetNumVideoModes();
	
	for(i=0; i<num; i++)
	{
        RwEngineGetVideoModeInfo(&videoMode, i);
        if(videoMode.width == DISP_W && 
        	videoMode.height == DISP_H &&
        	videoMode.depth == 32 &&
			(videoMode.flags == (rwVIDEOMODEINTERLACE|rwVIDEOMODEEXCLUSIVE| rwVIDEOMODEFSAA1)))
        {
   	    	RwEngineSetVideoMode(i);
        	break;
        }
	}
	
	ASSERTMSG(i != num, "Can't find requested video mode");
	
#endif	
}



//
//
//
//
void CPostEffects::InitDMAPkt()
{
#ifdef BLIT_TO_DISPLAYBUFFER
 	RwSkyVideoMode *cm;
	
    RWALIGN(static RwUInt128 dmaData[MAX_DMADATA], 64);

	//SetGsDrawEnv(&_rwDMAFlipData.disp1[_rwDMAFlipId & 1],&_rwDMAFlipData.db.draw11, 
	//				   &_rwDMAFlipData.db.draw12);
	SetDMAPkt(&dmaData[0], MAX_DMADATA);
	
	m_pDmaPkt = dmaData;

 	_rwDMAFlipData.dmaPkt[0] = _rwDMAFlipData.dmaPkt[1]
            = _rwDMAFlipData.dmaPkt[2] = _rwDMAFlipData.dmaPkt[3]
                = m_pDmaPkt;
    /* DISPLAY1 */
    _rwDMAFlipData.tcaaDisp.display10 = *(tGS_DISPLAY1 *)&_rwDMAFlipData.db.disp[0].display;
    /* DISPFB1 */
    _rwDMAFlipData.tcaaDisp.dispfb10 = *(tGS_DISPFB1 *)&_rwDMAFlipData.db.disp[0].dispfb;

    /* DISPLAY1 */
    _rwDMAFlipData.tcaaDisp.display11 = *(tGS_DISPLAY1 *)&_rwDMAFlipData.db.disp[1].display;
    /* DISPFB1 */
    _rwDMAFlipData.tcaaDisp.dispfb11 = *(tGS_DISPFB1 *)&_rwDMAFlipData.db.disp[1].dispfb;

    _rwDMAFlipData.db.disp[0].dispfb.DBY = 1; /* dispfb2 has DBY field of 1 */
    _rwDMAFlipData.db.disp[0].display.DH = _rwDMAFlipData.db.disp[0].display.DH - 1;
/*     _rwDMAFlipData.db.disp[0].display.DX
                   = _rwDMAFlipData.db.disp[0].display.DX + 2; */
                                           /* display2 has magh/2 */
    _rwDMAFlipData.db.disp[0].pmode.ALP = 0x80;
    _rwDMAFlipData.db.disp[0].pmode.EN1 = 1;
    _rwDMAFlipData.db.disp[0].pmode.EN2 = 1;

    _rwDMAFlipData.db.disp[1].dispfb.DBY = 1; /* dispfb2 has DBY
                                                 field of 1 */
    _rwDMAFlipData.db.disp[1].display.DH = _rwDMAFlipData.db.disp[1].display.DH - 1;
/*     _rwDMAFlipData.db.disp[1].display.DX
                   = _rwDMAFlipData.db.disp[1].display.DX + 2; */
                                           /* display2 has magh/2 */
    _rwDMAFlipData.db.disp[1].pmode.ALP = 0x80;
    _rwDMAFlipData.db.disp[1].pmode.EN1 = 1;
    _rwDMAFlipData.db.disp[1].pmode.EN2 = 1;
 
	cm = RpSkyGetModeStructurePtr();
	
	if (cm->display.width == 832)
	{
		RwUInt32	magh;
		RwUInt32	dw;
		RwUInt32	dx;
		
		magh = 3-1;
		dw = cm->display.width*(magh+1)-1;
		dx = 636 + ((2560 - (dw+1))); // Roughly. Wrong for HDTV 
		
		
	  	_rwDMAFlipData.db.disp[0].display.MAGH = magh;
		_rwDMAFlipData.db.disp[0].display.DW = dw;
	 	_rwDMAFlipData.db.disp[0].display.DX = dx;
		_rwDMAFlipData.db.disp[1].display.MAGH = magh;
	  	_rwDMAFlipData.db.disp[1].display.DW = dw;
	  	_rwDMAFlipData.db.disp[1].display.DX = dx;
		
		_rwDMAFlipData.tcaaDisp.display10.MAGH = magh;
	  	_rwDMAFlipData.tcaaDisp.display10.DW = dw;
	  	_rwDMAFlipData.tcaaDisp.display10.DX = dx;
	  	_rwDMAFlipData.tcaaDisp.display11.MAGH = magh;
	  	_rwDMAFlipData.tcaaDisp.display11.DW = dw;
	  	_rwDMAFlipData.tcaaDisp.display11.DX = dx;
	}
	if (cm->display.width == 640)
	{
		RwUInt32	magh;
		RwUInt32	dw;
		RwUInt32	dx;
		
		magh = 4-1;
		dw = cm->display.width*(magh+1)-1;
		dx = 636 + ((2560 - (dw+1))); // Roughly. Wrong for HDTV 
		
	  	_rwDMAFlipData.db.disp[0].display.MAGH = magh;
		_rwDMAFlipData.db.disp[0].display.DW = dw;
	 	_rwDMAFlipData.db.disp[0].display.DX = dx;
		_rwDMAFlipData.db.disp[1].display.MAGH = magh;
	  	_rwDMAFlipData.db.disp[1].display.DW = dw;
	  	_rwDMAFlipData.db.disp[1].display.DX = dx;
		
		_rwDMAFlipData.tcaaDisp.display10.MAGH = magh;
	  	_rwDMAFlipData.tcaaDisp.display10.DW = dw;
	  	_rwDMAFlipData.tcaaDisp.display10.DX = dx;
	  	_rwDMAFlipData.tcaaDisp.display11.MAGH = magh;
	  	_rwDMAFlipData.tcaaDisp.display11.DW = dw;
	  	_rwDMAFlipData.tcaaDisp.display11.DX = dx;
	  	
	}

    _rwDMAFlipData.disp1[0] = _rwDMAFlipData.db.disp[0];
    _rwDMAFlipData.disp1[1] = _rwDMAFlipData.db.disp[1];
    _rwDMAFlipData.tcaaDisp1 = _rwDMAFlipData.tcaaDisp;
	  	
    SyncDCache(&_rwDMAFlipData,
               SCESYNCDCACHEROUNDUP((RwUInt8 *) &_rwDMAFlipData
               + sizeof(rwDMA_flipData)));
#endif
  	m_defScreenXPosn = _rwDMAFlipData.db.disp[0].display.DX;
  	m_defScreenYPosn = _rwDMAFlipData.db.disp[0].display.DY;
}

//
// name:		SetScreenOffset
// description:	Set the screen x and y offset
void CPostEffects::SetScreenOffset(int32 x, int32 y)
{
	if(x + CPostEffects::m_defScreenXPosn < 0 || y + CPostEffects::m_defScreenYPosn < 0)
		return;
		
	_rwDMAFlipData.db.disp[1].display.DX = x + m_defScreenXPosn;
	_rwDMAFlipData.db.disp[0].display.DX = x + m_defScreenXPosn;
	_rwDMAFlipData.disp1[1].display.DX = x + m_defScreenXPosn;
	_rwDMAFlipData.disp1[0].display.DX = x + m_defScreenXPosn;
	_rwDMAFlipData.tcaaDisp.display10.DX = x + m_defScreenXPosn;
	_rwDMAFlipData.tcaaDisp.display11.DX = x + m_defScreenXPosn;
	_rwDMAFlipData.tcaaDisp1.display10.DX = x + m_defScreenXPosn;
	_rwDMAFlipData.tcaaDisp1.display11.DX = x + m_defScreenXPosn;

	_rwDMAFlipData.db.disp[1].display.DY = y + m_defScreenYPosn;
	_rwDMAFlipData.db.disp[0].display.DY = y + m_defScreenYPosn;
	_rwDMAFlipData.disp1[1].display.DY = y + m_defScreenYPosn;
	_rwDMAFlipData.disp1[0].display.DY = y + m_defScreenYPosn;
	_rwDMAFlipData.tcaaDisp.display10.DY = y + m_defScreenYPosn;
	_rwDMAFlipData.tcaaDisp.display11.DY = y + m_defScreenYPosn;
	_rwDMAFlipData.tcaaDisp1.display10.DY = y + m_defScreenYPosn;
	_rwDMAFlipData.tcaaDisp1.display11.DY = y + m_defScreenYPosn;
}

//
//
//
//
void CPostEffects::SetDMAPkt(RwUInt128 *dmaData, uint32 dmaDataPktSize)
{
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwRaster* pDisplayBuffer = RpSkyGetDisplayBufferRaster();
    unsigned long tmp, tmp1;
    RwUInt128 ltmp;
//    RwUInt32 offset=0, filter;
//    RwUInt32 hUvOff;
//    RwUInt32 vUvOff;
//    RwUInt32 vUv2Off;
    uint32 count = 0;
	int32 drawWidth, drawPsm;
	uint32 drawRasterAddr = GetRasterInfo(pDrawBuffer, drawWidth, drawPsm);
	int32 displayWidth, displayPsm;
	uint32 displayRasterAddr = GetRasterInfo(pDisplayBuffer, displayWidth, displayPsm);
 	
    /* More generic, centred values for texture co-ordinates */
/*    hUvOff = (RwUInt32)((8.0f*(RwReal)DRAW_W
                         /(RwReal)DISP_W) + 0.5f);
    vUvOff = (RwUInt32)((8.0f*(RwReal)DRAW_H
                         /(RwReal)DISP_H) + 0.5f);
    vUv2Off = (RwUInt32)((16.0f*(RwReal)DRAW_H
                          /(RwReal)DISP_H) + 0.5f);

    filter = 1;*/

    /* DMA header */
    tmp = (7l << 28) | (MAX_DMADATA-1); /* END DMAtag, 19 quadword after the dmatag */
    tmp1 = 0l;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* GIF path3 A+D mode */
    tmp = SCE_GIF_SET_TAG(MAX_DMADATA-2, 1, 0, 0, SCE_GIF_PACKED, 1);
    tmp1 = /* A+D */ (0xel);
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* write front frame register */
    tmp = SCE_GS_SET_FRAME(displayRasterAddr, displayWidth>>6, displayPsm, 0x00000000);
    tmp1 = GS_FRAME_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* write front offset register - no offsets to make coords easy */
    tmp = SCE_GS_SET_XYOFFSET(0,0);
    tmp1 = GS_XYOFFSET_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* write front scissor register - we want the whole screen*/
    tmp = SCE_GS_SET_SCISSOR(0,0x7fff,0,0x7fff);
    tmp1 = GS_SCISSOR_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* write front test register - no Z no alpha */
    tmp = SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS);
    tmp1 = GS_TEST_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* zbuffer not updated */
    tmp = (1l << 32) | skyZbuf_1;
    tmp1 = GS_ZBUF_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

//	tmp = SCE_GS_SET_DIMX(	-2, -4, 3,  -2,
//							-4,  1,  -1, -3,
//							3,  -1, 1,  -4,
//							-2, -3, -4, 2);
// This is a really horrible dither matrix. I'll have to find another							
//	tmp = SCE_GS_SET_DIMX(	1^7, 3^7, 3,  1^7,
//							3^7,  1,  7, 2^7,
//							3,  7, 1,  3^7,
//							1^7, 2^7, 3^7, 2);
//   tmp1 = GS_DIMX;
//    MAKE128(ltmp, tmp1, tmp);
//    dmaData[count++] = ltmp;

    tmp1 = GS_DTHE;
    MAKE128(ltmp, tmp1, 1);
    dmaData[count++] = ltmp;

    /* because it's only a pretense that the texture map is 640x448
     * (and we've told the gs that it's 1024x1024) we need to turn on
     * region clamp to get the right result at the edges.
     */
    tmp = SCE_GS_SET_CLAMP(2,2, 0, DRAW_W-1, 0, DRAW_H-1);
    tmp1 = GS_CLAMP_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* let's use a texture map... */
    tmp = SCE_GS_SET_TEX0(drawRasterAddr, drawWidth >> 6, drawPsm, 10, 10, 0, SCE_GS_DECAL, 0,0,0,0,0);      
    tmp1 = GS_TEX0_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* point filtering */
    tmp = SCE_GS_SET_TEX1(0,0,SCE_GS_NEAREST,SCE_GS_NEAREST,0,0,0);
    tmp1 = GS_TEX1_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* now we want to draw a sprite */
    tmp = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0);
    tmp1 = GS_PRIM;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    tmp = SCE_GS_SET_RGBAQ(128,128,128,128, 0);
    tmp1 = GS_RGBAQ;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

	// Draw screen buffer in 32 pixel strips
/*	for (RwInt32 i=0;i<(SCREEN_DRAW_WIDTH>>5);i++)
	{
	    // First vertex, top left
	    tmp = SCE_GS_SET_UV(GETUVCOORD_NEAR(i<<5), GETUVCOORD_NEAR(0));
	    tmp1 = GS_UV;
	    MAKE128(ltmp, tmp1, tmp);
	    dmaData[count++] = ltmp;

	    // top left (0,0)
	    tmp = SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0);
	    tmp1 = GS_XYZ2;
	    MAKE128(ltmp, tmp1, tmp);
	    dmaData[count++] = ltmp;

	    // bottom right 
	    tmp = SCE_GS_SET_UV(GETUVCOORD_NEAR((i+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT));
	    tmp1 = GS_UV;
	    MAKE128(ltmp, tmp1, tmp);
	    dmaData[count++] = ltmp;

	    tmp = SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5), GETPRIMCOORD(SCREEN_DISPLAY_HEIGHT), 0);
	    tmp1 = GS_XYZ2;
	    MAKE128(ltmp, tmp1, tmp);
	    dmaData[count++] = ltmp;

//		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(i<<5), GETUVCOORD_NEAR(0)));
//		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));
//		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((i+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
//		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
	}*/

    // First vertex, top left 
    tmp = SCE_GS_SET_UV(GETUVCOORD_NEAR(0), GETUVCOORD_NEAR(0));
    tmp1 = GS_UV;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    // top left (0,0) 
    tmp = SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0);
    tmp1 = GS_XYZ2;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    // bottom right  
    tmp = SCE_GS_SET_UV(GETUVCOORD_NEAR(DRAW_W), GETUVCOORD_NEAR(DRAW_H-1));
    tmp1 = GS_UV;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    tmp = SCE_GS_SET_XYZ(GETPRIMCOORD(DISP_W), GETPRIMCOORD(DISP_H), 0);
    tmp1 = GS_XYZ2;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    tmp = skyXyoffset_1;
    tmp1 = GS_XYOFFSET_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    /* finally put back the frame and zbuf registers */
    /* the front and back are the same, so we pick either one */
    tmp = (drawPsm << 24)
          | ((drawWidth >> 6) << 16)
          | (drawRasterAddr);
//    tmp = *(long*)&m_pDraw1->frame1;
    tmp1 = GS_FRAME_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    tmp = skyZbuf_1 & (~(1L << 32) );
    tmp1 = GS_ZBUF_1;
    MAKE128(ltmp, tmp1, tmp);
    dmaData[count++] = ltmp;

    tmp1 = GS_DTHE;
    MAKE128(ltmp, tmp1, 0);
    dmaData[count++] = ltmp;

    SyncDCache(dmaData, SCESYNCDCACHEROUNDUP(dmaData+count));

	ASSERT(count == dmaDataPktSize);
	return;
}// end of CPostEffects::SetDMAPkt()...
#endif



//
//
//
//
void CPostEffects::Update(void)
{
#ifndef FINAL
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_M, KEYBOARD_MODE_STANDARD, "colour multiplier")) 
		display_kbd_debug = !display_kbd_debug;

	if (display_kbd_debug)
	{
		if (PS2Keyboard.GetKeyDown(PS2_KEY_RIGHT)) m_colour1Multiplier+=0.1f;
		if (PS2Keyboard.GetKeyDown(PS2_KEY_LEFT)) m_colour1Multiplier-=0.1f;
		
		if (PS2Keyboard.GetKeyDown(PS2_KEY_UP)) m_colour2Multiplier+=0.1f;
		if (PS2Keyboard.GetKeyDown(PS2_KEY_DOWN)) m_colour2Multiplier-=0.1f;
		
		m_colour1Multiplier = MAX(0.1f, MIN(m_colour1Multiplier, 10.0f));
		m_colour2Multiplier = MAX(0.1f, MIN(m_colour2Multiplier, 10.0f));
	}
#endif
	if(CWeather::Rain > 0.0f)
		CPostEffects:m_bRainEnable = TRUE;
	else
		CPostEffects::m_bRainEnable = FALSE;
	if (!pRasterFrontBuffer)
		CPostEffects::SetupBackBufferVertex();
}

//----------------------------------------------------
//----------------------------------------------------

// ############################################
// ### DoScreenModeDependentInitializations ###
// ############################################

#if defined (GTA_PC)

void CPostEffects::DoScreenModeDependentInitializations()
{
	ImmediateModeFilterStuffInitialize();
	HeatHazeFXInit();

#ifndef MASTER
/*
	m_bShowDebugText = true;
	m_ShowDebugTextTime = 100;
*/
#endif
} // DoScreenModeDependentInitializations //

#endif

// ##########################################
// ### ImmediateModeFilterStuffInitialize ###
// ##########################################

void CPostEffects::ImmediateModeFilterStuffInitialize()
{
	RwUInt8 r, g, b, a;

/*
	// Create the sinus LUT for the wave FX
	for( Int32 i = 0; i < WAVE_FILTER_SINUS_LUT_SIZE; i++ )
	{
		float angleStep = 360.0f / (float)(WAVE_FILTER_SINUS_LUT_SIZE);
		ms_imf.fSin[ i ] = CMaths::Sin( DEGTORAD( (float)(i) * angleStep ) );
	} // for i //
*/

	// The following 2 values are taken from RW example code

/* RW example...
recipZ = 1.0f / RwCameraGetNearClipPlane(camera);
nearScreenZ = RwIm2DGetNearScreenZ();

RwIm2DVertexSetScreenZ(&v[i], nearScreenZ);
RwIm2DVertexSetRecipCameraZ(&v[0], recipZ);
*/

#ifdef GTA_XBOX
	ms_imf.screenZ     = 0.0f; // Special case for XBOX!
#else
	#ifdef GTA_PS2
		ms_imf.screenZ = (RwReal)((3276)); // This was used for PS2!
	#else
		ms_imf.screenZ = RwIm2DGetNearScreenZ(); // ### TODO! Test if this works for XBOX too! ###
	#endif
#endif

#ifdef GTA_PS2
	ms_imf.recipCameraZ = (RwReal)((1.0/6.0)); // This was used for PS2!
#else
	ms_imf.recipCameraZ = 1.0f / RwCameraGetNearClipPlane(Scene.camera);
#endif

	//

	ms_imf.uMinTri = 0.0f;
	ms_imf.uMaxTri = 2.0f;
	ms_imf.vMinTri = 0.0f;
	ms_imf.vMaxTri = 2.0f;

	// Raster stuff
#ifdef GTA_PS2
	ms_imf.pRasterDrawBuffer    = RpSkyGetDrawBufferRaster();
#else
	ms_imf.pRasterDrawBuffer    = pRasterFrontBuffer;
#endif
	ms_imf.sizeDrawBufferX      = RwRasterGetWidth ( ms_imf.pRasterDrawBuffer );
	ms_imf.sizeDrawBufferY      = RwRasterGetHeight( ms_imf.pRasterDrawBuffer );

//	ms_imf.pRasterDisplayBuffer = RpSkyGetDisplayBufferRaster();
//	ms_imf.sizeDisplayBufferX   = RwRasterGetWidth ( ms_imf.pRasterDisplayBuffer );
//	ms_imf.sizeDisplayBufferY   = RwRasterGetHeight( ms_imf.pRasterDisplayBuffer );

	// Misc. stuff
//	RwReal fScreenSizeX = (RwReal)ms_imf.sizeDisplayBufferX;
//	RwReal fScreenSizeY = (RwReal)ms_imf.sizeDisplayBufferY;
//	Int32  iScreenSizeX =         ms_imf.sizeDisplayBufferX;
//	Int32  iScreenSizeY =         ms_imf.sizeDisplayBufferY;

	RwReal fScreenSizeX = (RwReal)ms_imf.sizeDrawBufferX;
	RwReal fScreenSizeY = (RwReal)ms_imf.sizeDrawBufferY;
	Int32  iScreenSizeX =         ms_imf.sizeDrawBufferX;
	Int32  iScreenSizeY =         ms_imf.sizeDrawBufferY;

	// Set up the vertices for the filter triangle
	// -------------------------------------------

	// The following setup makes sure that the "texture" is drawn to the "top left" square
	// part of the triangle (the part which covers the screen).
	//
	//
	// 0       1         2
	// ------------------- 0                             //
	// |xxxxxxx|        /                                //
	// |xxxxxxx|  O   /                                  //
	// |xxxxxxx|    /                                    //
	// |xxxxxxx|  /                                      //
	// |-------|/          1                             //
	// |      /                                          //
	// | O  /                                            //
	// |  /                                              //
	// |/                  2                             //
	//
	// x = The screen
	// O = These parts of the triangle are out of screen

	RwIm2DVertexSetScreenX     ( &ms_imf.triangle[0],                 0.0f );
	RwIm2DVertexSetScreenY     ( &ms_imf.triangle[0],                 0.0f );
	RwIm2DVertexSetScreenZ     ( &ms_imf.triangle[0],                 ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.triangle[0], ms_imf.uMinTri, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.triangle[0], ms_imf.vMinTri, ms_imf.recipCameraZ );
	RwIm2DVertexSetRecipCameraZ( &ms_imf.triangle[0],                 ms_imf.recipCameraZ );

	RwIm2DVertexSetScreenX     ( &ms_imf.triangle[1],                 2.0f * fScreenSizeX );
	RwIm2DVertexSetScreenY     ( &ms_imf.triangle[1],                 0.0f );
	RwIm2DVertexSetScreenZ     ( &ms_imf.triangle[1],                 ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.triangle[1], ms_imf.uMaxTri, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.triangle[1], ms_imf.vMinTri, ms_imf.recipCameraZ );
	RwIm2DVertexSetRecipCameraZ( &ms_imf.triangle[1],                 ms_imf.recipCameraZ );

	RwIm2DVertexSetScreenX     ( &ms_imf.triangle[2],                 0.0f );
	RwIm2DVertexSetScreenY     ( &ms_imf.triangle[2],                 2.0f * fScreenSizeY );
	RwIm2DVertexSetScreenZ     ( &ms_imf.triangle[2],                 ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.triangle[2], ms_imf.uMinTri, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.triangle[2], ms_imf.vMaxTri, ms_imf.recipCameraZ );
	RwIm2DVertexSetRecipCameraZ( &ms_imf.triangle[2],                 ms_imf.recipCameraZ );

	// Set up the vertices for the quad
	// --------------------------------

	r = 0;
	g = 200;
	b = 0;
	a = 255;

	//------------------//
	//                  //
	// Quad triangle #1 //
	//                  //
	// 1 ----------- 2  //
	// x |        /  x  //
	//   | #1   /       //
	//   |    /         //
	//   |  /           //
	//   |/             //
	//                  //
	//  3               //
	//  x               //
	//------------------//

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[0],  0.0f );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[0],  0.0f );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[0],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[0],  0.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[0],  0.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[0],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[0],  r, g, b, a );

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[1],  0.0f );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[1],  0.0f );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[1],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[1],  1.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[1],  0.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[1],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[1],  r, g, b, a );

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[2],  0.0f );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[2],  0.0f );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[2],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[2],  0.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[2],  1.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[2],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[2],  r, g, b, a );

	//------------------//
	//                  //
	// Quad triangle #2 //
	//                  //
	//   ----------- 1  //
	//   |        /|    //
	//   |      /  |    //
	//   |    /    |    //
	//   |  /  #2  |    //
	//   |/________|    //
	//                  //
	//  3            2  //
	//               x  //
	//------------------//

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[3],  0.0f );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[3],  0.0f );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[3],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[3],  1.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[3],  1.0f, ms_imf.recipCameraZ );
	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[3],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[3],  r, g, b, a );

	// Set the default UV's
	DrawQuadSetDefaultUVs();

	// Set up the vertices for the filter triangle strip
	// -------------------------------------------------

/*

//	Int32   colorXOR = 1;

	// x & y
	RwReal  xLeft   = 0.0f;
	RwReal  xRight  = fScreenSizeX;

	RwReal  yTop;
	RwReal  yBottom;
	RwReal  yCurrent = 0.0f;
	RwReal  yStep    = fScreenSizeY / (RwReal)( NUM_V_TRI_STRIP_FILTER_QUADS );

	// u & v
	RwReal  uLeft    =  0.0f;
	RwReal  uRight   =  1.0f;

	RwReal  vTop;
	RwReal  vBottom;
	RwReal  vCurrent = 0.0f;
	RwReal  vStep    = 1.0f / (RwReal)( NUM_V_TRI_STRIP_FILTER_QUADS );

	Int32 n = 0;

	for( Int32 i = 0; i < NUM_V_TRI_STRIP_FILTER_QUADS; i++ )
	{
		yTop    = yCurrent;
		yBottom = yTop + yStep;

		vTop    = vCurrent;
		vBottom = vTop + vStep;

		//

//		colorXOR ^= 1;
//		if( colorXOR == 0 )
//		{
//			r = 255;
//			g = 0;
//			b = 0;
//			a = 164;
//		} // if //
//		else
//		{
//			r = 0;
//			g = 255;
//			b = 0;
//			a = 164;
//		} // if //

		//

		r = m_WaveFilterCol.red;   //255;
		g = m_WaveFilterCol.green; //255;
		b = m_WaveFilterCol.blue;  //255;
		a = m_WaveFilterCol.alpha; //128;

//		r = 255;
//		g = 255;
//		b = 255;
//		a = 128;

		if( i == 0 )
		{
			// Initialize the first 2 triangles...

			//------------------//
			//                  //
			// Quad triangle #1 //
			//                  //
			// 1 ----------- 2  //
			// x |        /  x  //
			//   | #1   /       //
			//   |    /         //
			//   |  /           //
			//   |/             //
			//                  //
			//  3               //
			//  x               //
			//------------------//

			// Vertex 1 of triangle #1
			RwIm2DVertexSetScreenX     ( &ms_imf.triStrip[n],                 xLeft );
			RwIm2DVertexSetScreenY     ( &ms_imf.triStrip[n],                 yTop );
			RwIm2DVertexSetScreenZ     ( &ms_imf.triStrip[n],                 ms_imf.screenZ );
			RwIm2DVertexSetU           ( &ms_imf.triStrip[n], uLeft,          ms_imf.recipCameraZ );
			RwIm2DVertexSetV           ( &ms_imf.triStrip[n], vTop,           ms_imf.recipCameraZ );
			RwIm2DVertexSetRecipCameraZ( &ms_imf.triStrip[n],                 ms_imf.recipCameraZ );
			RwIm2DVertexSetIntRGBA     ( &ms_imf.triStrip[n], r, g, b, a );
			n++;

			// Vertex 2 of triangle #1
			RwIm2DVertexSetScreenX     ( &ms_imf.triStrip[n],                 xRight );
			RwIm2DVertexSetScreenY     ( &ms_imf.triStrip[n],                 yTop );
			RwIm2DVertexSetScreenZ     ( &ms_imf.triStrip[n],                 ms_imf.screenZ );
			RwIm2DVertexSetU           ( &ms_imf.triStrip[n], uRight,         ms_imf.recipCameraZ );
			RwIm2DVertexSetV           ( &ms_imf.triStrip[n], vTop,           ms_imf.recipCameraZ );
			RwIm2DVertexSetRecipCameraZ( &ms_imf.triStrip[n],                 ms_imf.recipCameraZ );
			RwIm2DVertexSetIntRGBA     ( &ms_imf.triStrip[n], r, g, b, a );
			n++;

			// Vertex 3 of triangle #1
			RwIm2DVertexSetScreenX     ( &ms_imf.triStrip[n],                 xLeft );
			RwIm2DVertexSetScreenY     ( &ms_imf.triStrip[n],                 yBottom );
			RwIm2DVertexSetScreenZ     ( &ms_imf.triStrip[n],                 ms_imf.screenZ );
			RwIm2DVertexSetU           ( &ms_imf.triStrip[n], uLeft,          ms_imf.recipCameraZ );
			RwIm2DVertexSetV           ( &ms_imf.triStrip[n], vBottom,        ms_imf.recipCameraZ );
			RwIm2DVertexSetRecipCameraZ( &ms_imf.triStrip[n],                 ms_imf.recipCameraZ );
			RwIm2DVertexSetIntRGBA     ( &ms_imf.triStrip[n], r, g, b, a );
			n++;

			//------------------//
			//                  //
			// Quad triangle #2 //
			//                  //
			//   ----------- 1  //
			//   |        /|    //
			//   |      /  |    //
			//   |    /    |    //
			//   |  /  #2  |    //
			//   |/________|    //
			//                  //
			//  3            2  //
			//               x  //
			//------------------//

			// Vertex 2 of triangle #2
			RwIm2DVertexSetScreenX     ( &ms_imf.triStrip[n],                 xRight );
			RwIm2DVertexSetScreenY     ( &ms_imf.triStrip[n],                 yBottom );
			RwIm2DVertexSetScreenZ     ( &ms_imf.triStrip[n],                 ms_imf.screenZ );
			RwIm2DVertexSetU           ( &ms_imf.triStrip[n], uRight,         ms_imf.recipCameraZ );
			RwIm2DVertexSetV           ( &ms_imf.triStrip[n], vBottom,        ms_imf.recipCameraZ );
			RwIm2DVertexSetRecipCameraZ( &ms_imf.triStrip[n],                 ms_imf.recipCameraZ );
			RwIm2DVertexSetIntRGBA     ( &ms_imf.triStrip[n], r, g, b, a );
			n++;
		} // if //
		else
		{
			// Initialize all the other triangles...
			// (Only the bottom vertices of triangle #1 & #2 need to be initialized from now on
			// because the upper ones use the datas from the vertices of the triangle above!)

			//------------------//
			//                  //
			// Quad triangle #1 //
			//                  //
			// 1 ----------- 2  //
			//   |        /     //
			//   | #1   /       //
			//   |    /         //
			//   |  /           //
			//   |/             //
			//                  //
			//  3               //
			//  x               //
			//------------------//

			// Vertex 3 of triangle #1
			RwIm2DVertexSetScreenX     ( &ms_imf.triStrip[n],                 xLeft );
			RwIm2DVertexSetScreenY     ( &ms_imf.triStrip[n],                 yBottom );
			RwIm2DVertexSetScreenZ     ( &ms_imf.triStrip[n],                 ms_imf.screenZ );
			RwIm2DVertexSetU           ( &ms_imf.triStrip[n], uLeft,          ms_imf.recipCameraZ );
			RwIm2DVertexSetV           ( &ms_imf.triStrip[n], vBottom,        ms_imf.recipCameraZ );
			RwIm2DVertexSetRecipCameraZ( &ms_imf.triStrip[n],                 ms_imf.recipCameraZ );
			RwIm2DVertexSetIntRGBA     ( &ms_imf.triStrip[n], r, g, b, a );
			n++;

			//------------------//
			//                  //
			// Quad triangle #2 //
			//                  //
			//   ----------- 1  //
			//   |        /|    //
			//   |      /  |    //
			//   |    /    |    //
			//   |  /  #2  |    //
			//   |/________|    //
			//                  //
			//  3            2  //
			//               x  //
			//------------------//

			// Vertex 2 of triangle #2
			RwIm2DVertexSetScreenX     ( &ms_imf.triStrip[n],                 xRight );
			RwIm2DVertexSetScreenY     ( &ms_imf.triStrip[n],                 yBottom );
			RwIm2DVertexSetScreenZ     ( &ms_imf.triStrip[n],                 ms_imf.screenZ );
			RwIm2DVertexSetU           ( &ms_imf.triStrip[n], uRight,         ms_imf.recipCameraZ );
			RwIm2DVertexSetV           ( &ms_imf.triStrip[n], vBottom,        ms_imf.recipCameraZ );
			RwIm2DVertexSetRecipCameraZ( &ms_imf.triStrip[n],                 ms_imf.recipCameraZ );
			RwIm2DVertexSetIntRGBA     ( &ms_imf.triStrip[n], r, g, b, a );
			n++;
		} // else //

		//

		yCurrent += yStep;
		vCurrent += vStep;
	} // for i //

*/

	// Here we create the texture coordinates for the "front buffer".
	// The reason why this is needed is the fact the for a screen of 640x480 a RW camera
	// texture raster of the size 1024x512 is used and the texture coordinates to copy the
	// screen via immediate mode will be not (u = 1.0f, v = 1.0f)!!

	RwRaster* pRaster = RwCameraGetRaster(Scene.camera);
	ASSERT( pRaster );

	RwInt32 logWidth, logHeight;
	RwInt32 w = RwRasterGetWidth( pRaster );
	RwInt32 h = RwRasterGetHeight( pRaster );
    logWidth  = (RwUInt32)(log((float)w) / log(2.0f));
    logHeight = (RwUInt32)(log((float)h) / log(2.0f));
    w = (RwUInt32)powf(2, logWidth+1);
    h = (RwUInt32)powf(2, logHeight+1);

	ms_imf.fFrontBufferU1 = 0.0f;
	ms_imf.fFrontBufferU2 = (float)( SCREEN_WIDTH ) / (float)( w );
	ms_imf.fFrontBufferV1 = 0.0f;
	ms_imf.fFrontBufferV2 = (float)( SCREEN_HEIGHT ) / (float)( h );
} // ImmediateModeFilterStuffInitialize //

// ######################################
// ### ImmediateModeRenderStatesStore ###
// ######################################

void CPostEffects::ImmediateModeRenderStatesStore()
{
	RwRenderStateGet( rwRENDERSTATESRCBLEND,          (void *)&ms_imf.blendSrc );		//  1
	RwRenderStateGet( rwRENDERSTATEDESTBLEND,         (void *)&ms_imf.blendDst );		//  2
	RwRenderStateGet( rwRENDERSTATEFOGENABLE,         (void *)&ms_imf.bFog );			//  3
	RwRenderStateGet( rwRENDERSTATECULLMODE,          (void *)&ms_imf.cullMode );		//  4
	RwRenderStateGet( rwRENDERSTATEZTESTENABLE,       (void *)&ms_imf.bZTest );			//  5
	RwRenderStateGet( rwRENDERSTATEZWRITEENABLE,      (void *)&ms_imf.bZWrite );		//  6
	RwRenderStateGet( rwRENDERSTATESHADEMODE,         (void *)&ms_imf.shadeMode );		//  7
	RwRenderStateGet( rwRENDERSTATEVERTEXALPHAENABLE, (void *)&ms_imf.bVertexAlpha );	//  8
	RwRenderStateGet( rwRENDERSTATETEXTUREADDRESS,    (void *)&ms_imf.textureAddress );	//  9
	RwRenderStateGet( rwRENDERSTATETEXTUREFILTER,     (void *)&ms_imf.textureFilter );	// 10
} // ImmediateModeRenderStatesStore //

// ####################################
// ### ImmediateModeRenderStatesSet ###
// ####################################

void CPostEffects::ImmediateModeRenderStatesSet()
{
	// Please DO NOT change any of the following parameters!!!
	RwRenderStateSet( rwRENDERSTATESRCBLEND,          (void *)rwBLENDSRCALPHA );		//  1
	RwRenderStateSet( rwRENDERSTATEDESTBLEND,         (void *)rwBLENDINVSRCALPHA );		//  2
	RwRenderStateSet( rwRENDERSTATEFOGENABLE,         (void *)FALSE );					//  3
	RwRenderStateSet( rwRENDERSTATECULLMODE,          (void *)rwCULLMODECULLNONE );		//  4
	RwRenderStateSet( rwRENDERSTATEZTESTENABLE,       (void *)FALSE );					//  5
	RwRenderStateSet( rwRENDERSTATEZWRITEENABLE,      (void *)FALSE );					//  6
	RwRenderStateSet( rwRENDERSTATESHADEMODE,         (void *)rwSHADEMODEGOURAUD );		//  7
	RwRenderStateSet( rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE );					//  8
	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS,    (void *)rwTEXTUREADDRESSCLAMP ); 	//  9 : rwTEXTUREADDRESSWRAP, rwTEXTUREADDRESSCLAMP
	RwRenderStateSet( rwRENDERSTATETEXTUREFILTER,     (void *)rwFILTERNEAREST );		// 10 : rwFILTERNEAREST, rwFILTERLINEAR
} // ImmediateModeRenderStatesSet //

// ########################################
// ### ImmediateModeRenderStatesReStore ###
// ########################################

void CPostEffects::ImmediateModeRenderStatesReStore()
{
	RwRenderStateSet( rwRENDERSTATESRCBLEND,          (void *)ms_imf.blendSrc );		//  1
	RwRenderStateSet( rwRENDERSTATEDESTBLEND,         (void *)ms_imf.blendDst );		//  2
	RwRenderStateSet( rwRENDERSTATEFOGENABLE,         (void *)ms_imf.bFog );			//  3
	RwRenderStateSet( rwRENDERSTATECULLMODE,          (void *)ms_imf.cullMode );		//  4
	RwRenderStateSet( rwRENDERSTATEZTESTENABLE,       (void *)ms_imf.bZTest );			//  5
	RwRenderStateSet( rwRENDERSTATEZWRITEENABLE,      (void *)ms_imf.bZWrite );			//  6
	RwRenderStateSet( rwRENDERSTATESHADEMODE,         (void *)ms_imf.shadeMode );		//  7
	RwRenderStateSet( rwRENDERSTATEVERTEXALPHAENABLE, (void *)ms_imf.bVertexAlpha );	//  8
	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS,    (void *)ms_imf.textureAddress );	//  9
	RwRenderStateSet( rwRENDERSTATETEXTUREFILTER,     (void *)ms_imf.textureFilter );	// 10
} // ImmediateModeRenderStatesReStore //

// ################
// ### DrawQuad ###
// ################

void CPostEffects::DrawQuad( RwReal xPos, RwReal yPos, RwReal xSize, RwReal ySize,
	                         RwUInt8 r, RwUInt8 g, RwUInt8 b, RwUInt8 a,
	                         RwRaster* pRaster )
{
	// Set the raster
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER, pRaster );
/*
#ifdef GTA_XBOX	// just a test - to remove!
	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP );
#endif*/

	//------------------//
	//                  //
	// Quad triangle #1 //
	//                  //
	// 1 ----------- 2  //
	// x |        /  x  //
	//   | #1   /       //
	//   |    /         //
	//   |  /           //
	//   |/             //
	//                  //
	//  3               //
	//  x               //
	//------------------//

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[0],  xPos );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[0],  yPos );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[0],        ms_imf.screenZ );
//	RwIm2DVertexSetU           ( &ms_imf.quad[0],  0.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetV           ( &ms_imf.quad[0],  0.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[0],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[0],  r, g, b, a );

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[1],  xPos + xSize );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[1],  yPos );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[1],        ms_imf.screenZ );
//	RwIm2DVertexSetU           ( &ms_imf.quad[1],  1.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetV           ( &ms_imf.quad[1],  0.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[1],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[1],  r, g, b, a );

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[2],  xPos );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[2],  yPos + ySize );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[2],        ms_imf.screenZ );
//	RwIm2DVertexSetU           ( &ms_imf.quad[2],  0.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetV           ( &ms_imf.quad[2],  1.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[2],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[2],  r, g, b, a );

	//------------------//
	//                  //
	// Quad triangle #2 //
	//                  //
	//   ----------- 1  //
	//   |        /|    //
	//   |      /  |    //
	//   |    /    |    //
	//   |  /  #2  |    //
	//   |/________|    //
	//                  //
	//  3            2  //
	//               x  //
	//------------------//

	RwIm2DVertexSetScreenX     ( &ms_imf.quad[3],  xPos + xSize );
	RwIm2DVertexSetScreenY     ( &ms_imf.quad[3],  yPos + ySize );
	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[3],        ms_imf.screenZ );
//	RwIm2DVertexSetU           ( &ms_imf.quad[3],  1.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetV           ( &ms_imf.quad[3],  1.0f, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[3],        ms_imf.recipCameraZ );
	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[3],  r, g, b, a );

	// Render the quad
	RwIm2DRenderPrimitive_BUGFIX( rwPRIMTYPETRISTRIP, ms_imf.quad, 4 );
} // DrawQuad //

// ######################
// ### DrawQuadSetUVs ###
// ######################

void CPostEffects::DrawQuadSetUVs( RwReal u1, RwReal v1,
	                               RwReal u2, RwReal v2,
	                               RwReal u3, RwReal v3,
	                               RwReal u4, RwReal v4 )
{
	//------------------//
	//                  //
	// Quad triangle #1 //
	//                  //
	// 1 ----------- 2  //
	// x |        /  x  //
	//   | #1   /       //
	//   |    /         //
	//   |  /           //
	//   |/             //
	//                  //
	//  3               //
	//  x               //
	//------------------//

//	RwIm2DVertexSetScreenX     ( &ms_imf.quad[0],  xPos );
//	RwIm2DVertexSetScreenY     ( &ms_imf.quad[0],  yPos );
//	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[0],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[0],    u1, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[0],    v1, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[0],        ms_imf.recipCameraZ );
//	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[0],  r, g, b, a );

//	RwIm2DVertexSetScreenX     ( &ms_imf.quad[1],  xPos + xSize );
//	RwIm2DVertexSetScreenY     ( &ms_imf.quad[1],  yPos );
//	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[1],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[1],    u2, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[1],    v2, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[1],        ms_imf.recipCameraZ );
//	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[1],  r, g, b, a );

//	RwIm2DVertexSetScreenX     ( &ms_imf.quad[2],  xPos );
//	RwIm2DVertexSetScreenY     ( &ms_imf.quad[2],  yPos + ySize );
//	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[2],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[2],    u4, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[2],    v4, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[2],        ms_imf.recipCameraZ );
//	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[2],  r, g, b, a );

	//------------------//
	//                  //
	// Quad triangle #2 //
	//                  //
	//   ----------- 1  //
	//   |        /|    //
	//   |      /  |    //
	//   |    /    |    //
	//   |  /  #2  |    //
	//   |/________|    //
	//                  //
	//  3            2  //
	//               x  //
	//------------------//

//	RwIm2DVertexSetScreenX     ( &ms_imf.quad[3],  xPos + xSize );
//	RwIm2DVertexSetScreenY     ( &ms_imf.quad[3],  yPos + ySize );
//	RwIm2DVertexSetScreenZ     ( &ms_imf.quad[3],        ms_imf.screenZ );
	RwIm2DVertexSetU           ( &ms_imf.quad[3],    u3, ms_imf.recipCameraZ );
	RwIm2DVertexSetV           ( &ms_imf.quad[3],    v3, ms_imf.recipCameraZ );
//	RwIm2DVertexSetRecipCameraZ( &ms_imf.quad[3],        ms_imf.recipCameraZ );
//	RwIm2DVertexSetIntRGBA     ( &ms_imf.quad[3],  r, g, b, a );
} // DrawQuadSetUVs //

// ###########################
// ### DrawQuadSetPixelUVs ###
// ###########################

void CPostEffects::DrawQuadSetPixelUVs( RwReal u1, RwReal v1,
	                                    RwReal u2, RwReal v2,
	                                    RwReal u3, RwReal v3,
	                                    RwReal u4, RwReal v4 )
{
	RwReal oosx = 1.0f / (RwReal)(ms_imf.sizeDrawBufferX);
	RwReal oosy = 1.0f / (RwReal)(ms_imf.sizeDrawBufferY);
/*
	u1 += 0.0f;
	v1 += 0.0f;

	u2 += 1.0f;
	v2 += 0.0f;

	u3 += 1.0f;
	v3 += 1.0f;

	u4 += 0.0f;
	v4 += 1.0f;
*/
	DrawQuadSetUVs( u1*oosx, v1*oosy,
					u2*oosx, v2*oosy,
					u3*oosx, v3*oosy,
					u4*oosx, v4*oosy );
} // DrawQuadSetPixelUVs //

// #############################
// ### DrawQuadSetDefaultUVs ###
// #############################

void CPostEffects::DrawQuadSetDefaultUVs()
{
	DrawQuadSetUVs( 0.0f, 0.0f,
					1.0f, 0.0f,
					1.0f, 1.0f,
					0.0f, 1.0f );
} // DrawQuadSetDefaultUVs //

//----------------------------------------------------
//----------------------------------------------------

void CPostEffects::Close(void)
{
	RwRasterDestroy(m_pGrainRaster);
//	RwRasterDestroy(m_pGrayScaleCLUTRaster256);
//	RwTextureDestroy(m_pScreenFilterMaskTex);
#ifdef GTA_PC
    if(pRasterFrontBuffer) 
    {
       RwRasterDestroy(pRasterFrontBuffer);
       pRasterFrontBuffer = NULL;

    }
#endif    

#ifdef GTA_XBOX
    if(pRasterFrontBuffer) 
    {
       RwRasterDestroy(pRasterFrontBuffer);
       pRasterFrontBuffer = NULL;
    }
	RasterPostEffectsDestroyCamera();
#endif
}

#ifdef GTA_PS2

void CPostEffects::FinishRendering()
{
	// Wait until all DMA stuff is done...
	if (_rwDMAOpenVIFPkt(RWDMA_FIXUP, 2))
	{
		RwUInt64 tmp, tmp1;
		u_long128 ltmp;

		tmp = /* NLOOP */ 1l
		| /* EOP */ (1l<<15)
		| /* PRE */ (0l<<46)
		| /* FLG */ (0l<<58)
		| /* NREG */ (1l<<60);
		tmp1 = /* A+D */ (0xel<<(64-64));
		MAKE128(ltmp, tmp1, tmp);
		RWDMA_ADD_TO_PKT(ltmp);

		MAKE128(ltmp, GS_TEXFLUSH, 0x0L);
		RWDMA_ADD_TO_PKT(ltmp);
	} // if //
} // FinishRendering //
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CPostEffects::ScriptResetForEffects()
{
	// Make sure to reset/turn off stuff after a mission (script code will handle that)...
	m_bNightVision    = false;
	m_bInfraredVision = false;
	m_bHeatHazeFX     = false;
	m_bDarknessFilter = false;
	m_bCCTV           = false;

	// Make sure that water fog is on as default!
	CWaterLevel::ScriptWaterFogToggle( true );

	// Clear all user 3d markers!
///	C3dMarkers::User3dMarkerDeleteAll();

	// Here's a special case! We need to make sure that the volumetric clouds
	// will be initialized if this function gets called...
///	CClouds::VolumetricCloudsInit();
} // ScriptResetForEffects //

void CPostEffects::ScriptNightVisionSwitch( bool n )
{
	if( n )
	{
		m_bNightVision    = true;
		m_bInfraredVision = false; // Make sure "infrared vision" is OFF!
	} // if //
	else
	{
		m_bNightVision    = false;
	} // else //
} // ScriptNightVisionSwitch //

void CPostEffects::ScriptInfraredVisionSwitch( bool n )
{
	if( n )
	{
		m_bInfraredVision = true;
		m_bNightVision    = false; // Make sure "night vision" is OFF!
	} // if //
	else
	{
		m_bInfraredVision = false;
	} // else //
} // ScriptInfraredVisionSwitch //

void CPostEffects::ScriptHeatHazeFXSwitch( bool n )
{
	m_bHeatHazeFX = n;
} // ScriptHeatHazeFXSwitch //

void CPostEffects::ScriptDarknessFilterSwitch( bool n, Int32 alpha )
{
	m_bDarknessFilter = n;

	if( alpha == -1 )
	{
		m_DarknessFilterAlpha = m_DarknessFilterAlphaDefault;
	} // if //
	else
	{
		if( alpha <   0 ) alpha = 0;
		if( alpha > 255 ) alpha = 255;
		m_DarknessFilterAlpha = alpha;
	} // else //
} // ScriptDarknessFilterSwitch //

void CPostEffects::ScriptCCTVSwitch( bool n )
{
	m_bCCTV = n;
} // ScriptCCTVSwitch //

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef GTA_PS2

// --- Seam remover ---

// This couldn't be done in GS because the Z buffer will be destroyed and the "alpha
// stencil" trick did not work (too many problems)! 16.9.2004 - AI

bool CPostEffects::ShouldSeamRemoverBeUsed()
{
	if( !m_bSeamRemover ) return false;

	if( CGame::currArea != AREA_MAIN_MAP                ||
		FindPlayerPed()->GetAreaCode() != AREA_MAIN_MAP ) return false;

	return true;
} // ShouldSeamRemoverBeUsed //

void CPostEffects::SeamRemoverBegin()
{
	// This piece here simply clears the background with a bright color in order to
	// see seams better!
	if( m_bSeamRemoverSeamSearchMode )
	{
		// Store and set render states
		ImmediateModeRenderStatesStore();
		ImmediateModeRenderStatesSet();

		// Test color background clear
		CPostEffects::DrawQuad( 0.0f, 0.0f,
							//	ms_imf.sizeDrawBufferX,
							//	ms_imf.sizeDrawBufferY,
								SCREEN_DRAW_WIDTH,
								SCREEN_DRAW_HEIGHT,
								64, 255, 0, 255,
								NULL );

		// Restore the render states
		ImmediateModeRenderStatesReStore();
	} // if //

	// The "seam remover" starts after the following line.
	if( !ShouldSeamRemoverBeUsed() ) return;

	//

	// Store and set render states
	ImmediateModeRenderStatesStore();
	ImmediateModeRenderStatesSet();

	// Clear screen (alpha/Z-buffer only! (however, Z is of interest only here!))
	Int32 clearAlpha = 255; //0;

	RwRenderStateSet( rwRENDERSTATESRCBLEND,     (void *)rwBLENDSRCALPHA );
	RwRenderStateSet( rwRENDERSTATEDESTBLEND,    (void *)rwBLENDONE );
//	RwRenderStateSet( rwRENDERSTATEZTESTENABLE,  (void *)FALSE ); // No Z test!
    RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *)TRUE );  // Z write!

	RwReal originalZ = ms_imf.screenZ; // Get current "screenZ"
	ms_imf.screenZ = RwIm2DGetFarScreenZ(); // This will be used by "DrawQuad" and makes sure
											// the quad will be drawn as far as possible for
											// the Z reset!

// 	sprintf( gString, "Far Z = %.2f", ms_imf.screenZ );
//	VarConsole.AddDebugOutput( gString );

	CPostEffects::DrawQuad( 0.0f, 0.0f,
							/*ms_imf.sizeDrawBufferX*/SCREEN_DRAW_WIDTH,
							/*ms_imf.sizeDrawBufferY*/SCREEN_DRAW_HEIGHT,
							0, 0, 0, clearAlpha,
							NULL );

	ms_imf.screenZ = originalZ; // Restore original value!

/*
	// Test...
	RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDINVDESTALPHA );
	RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO );

	CPostEffects::DrawQuad( 0.0f, 0.0f,
							ms_imf.sizeDrawBufferX,
							ms_imf.sizeDrawBufferY,
							0, 0, 0, 255,
							NULL );
//	*/

	// Restore the render states
	ImmediateModeRenderStatesReStore();
} // SeamRemoverBegin //

void CPostEffects::SeamRemoverEnd()
{
	if( !ShouldSeamRemoverBeUsed() ) return;

///	CClouds::RenderSkyPolys();
///	return;

		// Draw the "draw buffer" content 1 pixel XY stretched to the screen!
		// It will only overdraw parts which contain the previously reseted Z coordinate
		// (sky & seams). Z test must be turned on!

		// Store and set render states
		ImmediateModeRenderStatesStore();
		ImmediateModeRenderStatesSet();

/*
		if( m_SeamRemoverMode == 0 )
		{
			RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDDESTALPHA );
			RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO );
		} // if //
		else
		{
			RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDINVDESTALPHA );
			RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE );
		} // else //
*/

		Int32 r = 255;
		Int32 g = 255;
		Int32 b = 255;
		Int32 a = 255;

		if( !m_bSeamRemoverDebugMode )
		{
		    RwRenderStateSet( rwRENDERSTATEZTESTENABLE,  (void *)TRUE );  // Z test!
//		    RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *)FALSE ); // No Z write!
		} // if //
		else
		{
			r = 0;
			g = 255;
			b = 0;
			a = 255;
		} // else //

///		RwRenderStateSet( rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR );

//	if( m_SeamRemoverType == 0 )
//	{
		CPostEffects::DrawQuad( 0.0f+m_SeamRemoverShiftTopLeft, 0.0f+m_SeamRemoverShiftTopLeft,
								ms_imf.sizeDrawBufferX+m_SeamRemoverShiftBottomRight,
								ms_imf.sizeDrawBufferY+m_SeamRemoverShiftBottomRight,
								r, g, b, a,
								ms_imf.pRasterDrawBuffer );
//	} // if //
//	else

//===========================================================================
/*
	{
		RwRenderStateSet( rwRENDERSTATETEXTURERASTER, ms_imf.pRasterDrawBuffer );

		RwReal shiftX = m_fSeamRemoverShiftX;
		RwReal shiftY = m_fSeamRemoverShiftY;

		RwReal fScreenSizeX = (RwReal)ms_imf.sizeDrawBufferX;
		RwReal fScreenSizeY = (RwReal)ms_imf.sizeDrawBufferY;

		RwIm2DVertexSetIntRGBA( &ms_imf.triangle[0], r, g, b, a );
		RwIm2DVertexSetIntRGBA( &ms_imf.triangle[1], r, g, b, a );
		RwIm2DVertexSetIntRGBA( &ms_imf.triangle[2], r, g, b, a );

		RwIm2DVertexSetScreenX( &ms_imf.triangle[0], 0.0f + shiftX );
		RwIm2DVertexSetScreenY( &ms_imf.triangle[0], 0.0f + shiftY );

		RwIm2DVertexSetScreenX( &ms_imf.triangle[1], 2.0f * fScreenSizeX + shiftX );
		RwIm2DVertexSetScreenY( &ms_imf.triangle[1], 0.0f                + shiftY );

		RwIm2DVertexSetScreenX( &ms_imf.triangle[2], 0.0f                + shiftX );
		RwIm2DVertexSetScreenY( &ms_imf.triangle[2], 2.0f * fScreenSizeY + shiftY );

		RwIm2DRenderTriangle( ms_imf.triangle, 3, 0, 1, 2 );

		// Set triangle coords. back to original
		RwIm2DVertexSetScreenX( &ms_imf.triangle[0], 0.0f );
		RwIm2DVertexSetScreenY( &ms_imf.triangle[0], 0.0f );

		RwIm2DVertexSetScreenX( &ms_imf.triangle[1], 2.0f * fScreenSizeX );
		RwIm2DVertexSetScreenY( &ms_imf.triangle[1], 0.0f );

		RwIm2DVertexSetScreenX( &ms_imf.triangle[2], 0.0f );
		RwIm2DVertexSetScreenY( &ms_imf.triangle[2], 2.0f * fScreenSizeY );
	} // else //
*/
//===========================================================================

		// Restore the render states
		ImmediateModeRenderStatesReStore();

return; // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ### GS code test (not used anymore!) ###

//#define GS_SEAM_REMOVER

#ifdef GS_SEAM_REMOVER

	// Store and set render states
	ImmediateModeRenderStatesStore();
	ImmediateModeRenderStatesSet();

	//----------------------------------------------------------------------------
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm, zpsm;
	int32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
	int32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	RwRaster *pSrcRaster;

//	if ( mblur )
//		pSrcRaster = RpSkyGetDisplayBufferRaster();
//	else
		pSrcRaster = pDrawBuffer;

	int32 dwSrcRasterAddr = GetRasterInfo(pSrcRaster, width, psm);

	const int32 numGeneralPackets = 26 - 1;

	const uint32 numGifQWords = numGeneralPackets;
	const uint32 numDMAQWords = 1 + numGifQWords;

	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords) )
		return;

	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );
		//----------------------------------------------------------------------------

		if( 1 )
		{
			ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );

//	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
//	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,0,0,0,0) );

		}
		else
		{
//			ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0, /* Alpha test */
//													SCE_GS_ALPHA_EQUAL/*NOTEQUAL*/,
//													0, /* Reference value */
//													SCE_GS_AFAIL_KEEP,
//													1, /* Destination alpha test */
//													0, /* Destination alpha test mode */
//													1, /* Depth test */
//													SCE_GS_ZALWAYS) );
		} // else //

		ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
		ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
		ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
		ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1) );

		// 1) Copy framebuffer to workbuffer
		// ---------------------------------

		Int32 sizeX = SCREEN_DRAW_WIDTH;
		Int32 sizeY = SCREEN_DRAW_HEIGHT;

		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwSrcRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	//	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR,  SCE_GS_LINEAR,  0, 0, 0 ) );
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );

		if( 1 )
		{
			// Copy screen buffer at once
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );

			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(sizeX), GETPRIMCOORD(sizeY), 0) );
		} // if //
		else
		{
			// New! Copy screen buffer in 32 pixel strips
			Int32 numStrips = sizeX/32;

			for( Int32 s = 0; s < numStrips; s++ )
			{
				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(s<<5), GETUVCOORD(0)));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((s+1)<<5), GETUVCOORD(SCREEN_DRAW_HEIGHT)));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));

			//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(s<<5), GETUVCOORD_NEAR(0)));
			//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
			//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((s+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
			//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
			} // for s //
		} // else //

		// 2) Copy game screen back to framebuffer
		// ---------------------------------------

//goto aa;

ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0, /* Alpha test */
										SCE_GS_ALPHA_EQUAL/*NOTEQUAL*/,
										0, /* Reference value */
										SCE_GS_AFAIL_KEEP,

										1, /* Destination alpha test */
										m_SeamRemoverMode, /* Destination alpha test mode */

										0, /* Depth test */
										SCE_GS_ZALWAYS) );

aa:;

//		Int32 intensity = 255;

		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));

	//	if( m_bRadiosityLinearFilter )
	//	{
			ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
	//	} // if //
	//	else
	//	{
	//		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
	//	} // else //

	//	if( m_bRadiosityDebug )
	//	{
	//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
	//		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 255, 0) ); /* Dummy packet! */
	//	} // if //
	//	else
	//	{
	//		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,intensity>>1) );
	//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );

//			ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
//			ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,1,intensity>>1) );

//                                       (0,1,0,1,0)
ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );

	//	} // else //

			ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );

			Int32 scanSizeX = sizeX;
			Int32 scanSizeY = sizeY;

			Int32 renderSizeX = 150;
			Int32 renderSizeY = 150;
//			Int32 renderSizeX = sizeX+2;
//			Int32 renderSizeY = sizeY+2;

/*
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0),
											  GETUVCOORD(0)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0),
												 GETPRIMCOORD(0), 0) );

			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0+scanSizeX),
											  GETUVCOORD(0+scanSizeY)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0+renderSizeX),
											     GETPRIMCOORD(0+renderSizeY), 0) );
*/

//	/*
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0),
											  GETUVCOORD(0)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(450),
												 GETPRIMCOORD(300), 0) );

			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0+scanSizeX),
											  GETUVCOORD(0+scanSizeY)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(450+renderSizeX),
											     GETPRIMCOORD(300+renderSizeY), 0) );
//	*/

//ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );

		//----------------------------------------------------------------------------
		//LOAD_GIFTAG_QWC();
		ASSERT(_numOfQuadWords == numGifQWords);
	}
	RWDMA_LOCAL_BLOCK_END();
	//----------------------------------------------------------------------------

	// Restore the render states
	ImmediateModeRenderStatesReStore();

#endif // GS_SEAM_REMOVER

} // SeamRemoverEnd //

#endif // GTA_PS2

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CPostEffects::Render()
{
/*#ifdef GTA_XBOX
	return;
#endif*/

	if (CPostEffects::m_bDisableAllPostEffect)
	{
		return;
	}

#ifdef GTA_PC
	//!PC - had to add the begin / end updates as RWDebug was complaining about it otherwise.
	// possible performance issue?

	GE_FORCE_5_FIX; // This must be put before the "RwCameraEndUpdate" before the raster copy!
	RwCameraEndUpdate(Scene.camera);

	RwRasterPushContext(pRasterFrontBuffer);
//	RsCameraBeginUpdate(Scene.camera); // ### Do NOT use this for "RwRasterPushContext" and "RwRasterPopContext"!! ###
	RwRasterRenderFast(RwCameraGetRaster(Scene.camera), 0, 0);
//	RwCameraEndUpdate(Scene.camera);
	RwRasterPopContext();

	RsCameraBeginUpdate(Scene.camera);
#elif defined(GTA_XBOX)
	// im mode pRasterFrontBuffer >>> pRasterFrontBuffer
	RasterCopyPostEffects();
#endif

#ifndef MASTER
/*
	if( m_bShowDebugText )
	{
		sprintf( gString, "--- Screen mode change! ---" );
		VarConsole.AddDebugOutput( gString );
		sprintf( gString, "--- %dx%d ---", SCREEN_WIDTH, SCREEN_HEIGHT );
		VarConsole.AddDebugOutput( gString );

		m_ShowDebugTextTime--;
		if( m_ShowDebugTextTime == 0 )
		{
			m_bShowDebugText = false;
		} // if //
	} // if //
*/
#endif

#ifdef SHOW_POST_EFFECTS_DEBUG_INFO

	//=====================================================================
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );

 	sprintf( gString, "Underwaterness = %.2f", CWeather::UnderWaterness );
	VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "Player water depth = %.2f", CWeather::WaterDepth );
	VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "Water fog intensity percent = %d", CTimeCycle::GetWaterFogIntensityPercent() );
	VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "Water fog FX control value = %.2f", CWeather::WaterFogFXControl );
	VarConsole.AddDebugOutput( gString );
 	sprintf( gString, "Speed = %.2f", FindPlayerSpeed().Magnitude() );
	VarConsole.AddDebugOutput( gString );
	//=====================================================================

#endif

	static float extraMultiplierLocal = SCREEN_EXTRA_MULT_BASE_CAP;
	static Int32 currentStrength = 0;
	float extraMultiplier = 1.0f;
	float prefsMultiplier = 1.0f;
	float multiplier;


	// The fog FX
	if( m_bFog )
	{
		Fog();
	} // if //


#ifdef GTA_PS2
	RwCameraEndUpdate(Scene.camera);
#endif
	RwRGBA col = {CTimeCycle::GetPostFx1Red(),
					CTimeCycle::GetPostFx1Green(),
					CTimeCycle::GetPostFx1Blue(),
					CTimeCycle::GetPostFx1Alpha()};
	RwRGBA col2 = {CTimeCycle::GetPostFx2Red(),
					CTimeCycle::GetPostFx2Green(),
					CTimeCycle::GetPostFx2Blue(),
					CTimeCycle::GetPostFx2Alpha()};
	RwRGBA fogcol = {127,127,127,0};
	
	if(m_bNightVision)
	{
		col.red    = 64;
		col.green  = 64;
		col.blue   = 64;

		col2.red   = 64;
		col2.green = 64;
		col2.blue  = 64;
	}
	if(m_bInfraredVision)
	{
		col.red    = 64;
		col.green  = 64;
		col.blue   = 64;

		col2.red   = 64;
		col2.green = 64;
		col2.blue  = 64;
	}
	
	// MN - affect the screen filter colours if the riots are on
	col.red   *= gfLaRiotsLightMult;
	col.green *= gfLaRiotsLightMult;
	col.blue  *= gfLaRiotsLightMult;
	
	if (FindPlayerPed())
	{
    	float extraMultiplierChangeRate = SCREEN_EXTRA_MULT_CHANGE_RATE*CTimer::GetTimeStep();
    	
    	if(CMaths::Abs(FindPlayerPed()->GetLightingFromCol() - extraMultiplierLocal) < extraMultiplierChangeRate)
    		extraMultiplierLocal = FindPlayerPed()->GetLightingFromCol();
    	else if(FindPlayerPed()->GetLightingFromCol() > extraMultiplierLocal)
    		extraMultiplierLocal += extraMultiplierChangeRate;
		else
    		extraMultiplierLocal -= extraMultiplierChangeRate;
    	
    	if(extraMultiplierLocal > SCREEN_EXTRA_MULT_BASE_CAP)
    		extraMultiplierLocal = SCREEN_EXTRA_MULT_BASE_CAP;
    	
    	extraMultiplier = (1.0f + SCREEN_EXTRA_MULT_BASE_MULT*(1.0f - extraMultiplierLocal/SCREEN_EXTRA_MULT_BASE_CAP));
	}
	
	// use preferences multiplier to scale brightness
/*
	if(FrontEndMenuManager.m_PrefsBrightness < DEFAULT_PREFS_BRIGHTNESS)
	{
//		prefsMultiplier = 1.0f / (2.0f - (FrontEndMenuManager.m_PrefsBrightness/DEFAULT_PREFS_BRIGHTNESS));
		prefsMultiplier = 1.0f;	// Aaron wants the filters not to change when the brightness is below 256.
	}
	else
	{
		prefsMultiplier = 1.0f + (FrontEndMenuManager.m_PrefsBrightness - DEFAULT_PREFS_BRIGHTNESS) / (MAX_PREFS_BRIGHTNESS-DEFAULT_PREFS_BRIGHTNESS);
		prefsMultiplier = 1.0f + (prefsMultiplier - 1.0f)*(0.5f + (0.5f * CVuCustomBuildingRenderer::GetDayNightBalanceParam()));
	}
*/
	prefsMultiplier = 1.0f;		// Now Aaron doesn't want the brightness to affect the filters

	
	multiplier 	= m_colour1Multiplier * extraMultiplier;
	col.red		= MAX(MIN(CMaths::NormalizeMultErr(col.red 	* multiplier), 255), 0);
	col.green 	= MAX(MIN(CMaths::NormalizeMultErr(col.green* multiplier), 255), 0);
	col.blue 	= MAX(MIN(CMaths::NormalizeMultErr(col.blue * multiplier), 255), 0);
	
	multiplier 	= m_colour2Multiplier * extraMultiplier;
	col2.red 	= MAX(MIN(CMaths::NormalizeMultErr(col2.red	 * multiplier), 255), 0);
	col2.green 	= MAX(MIN(CMaths::NormalizeMultErr(col2.green* multiplier), 255), 0);
	col2.blue 	= MAX(MIN(CMaths::NormalizeMultErr(col2.blue * multiplier), 255), 0);


/*	
#define MULTIPLIER_LIMIT	(300.0f)
	// limit the affect of the preferences multiplier
	float averageColour = col.red + (col2.red * (col2.alpha/128.0f));
	averageColour += col.green + (col2.green * (col2.alpha/128.0f));
	averageColour += col.blue + (col2.blue * (col2.alpha/128.0f));
	averageColour /= 3.0f;
		
	if(averageColour * prefsMultiplier > MULTIPLIER_LIMIT)
		prefsMultiplier = MULTIPLIER_LIMIT / averageColour;
*/		

	col.red 	= MAX(MIN(CMaths::NormalizeMultErr(col.red 	* prefsMultiplier), 255), 0);
	col.green 	= MAX(MIN(CMaths::NormalizeMultErr(col.green* prefsMultiplier), 255), 0);
	col.blue 	= MAX(MIN(CMaths::NormalizeMultErr(col.blue * prefsMultiplier), 255), 0);
	col2.red 	= MAX(MIN(CMaths::NormalizeMultErr(col2.red * prefsMultiplier), 255), 0);
	col2.green 	= MAX(MIN(CMaths::NormalizeMultErr(col2.green* prefsMultiplier), 255), 0);
	col2.blue 	= MAX(MIN(CMaths::NormalizeMultErr(col2.blue * prefsMultiplier), 255), 0);

	
	
//-------------------------------------------------------------------------------------



#ifdef GTA_PC
	if (m_bColorEnable)
#endif
	{
/*
		sprintf( gString, "R1 = %d, G1 = %d, B1 = %d, A1 = %d", col.red, col.green, col.blue, col.alpha );
		VarConsole.AddDebugOutput( gString );

		sprintf( gString, "R2 = %d, G2 = %d, B2 = %d, A1 = %d", col2.red, col2.green, col2.blue, col2.alpha );
		VarConsole.AddDebugOutput( gString );
*/
		CPostEffects::ColourFilter(col, col2);
	}

//	if( m_bWaveFilter )
//	{
//		WaveFilter( m_WaveFilterSpeed, m_WaveFilterFrequency, m_fWaveFilterShift );
//	} // if //

	if( m_bDarknessFilter  &&
	    !m_bNightVision    && /* NO darkness FX if night vision is ON!    */
	    !m_bInfraredVision )  /* NO darkness FX if infrared vision is ON! */
	{
		DarknessFilter( m_DarknessFilterAlpha );

        // Light up the brighter parts (flames, lights etc.)...
//		Radiosity(  40/*m_RadiosityIntensityLimit*/,
//				     0/*m_RadiosityFilterPasses*/,
//				     3/*m_RadiosityRenderPasses*/,
//				   200/*m_RadiosityIntensity*/ );

		Radiosity( m_DarknessFilterRadiosityIntensityLimit/*m_RadiosityIntensityLimit*/,
				     0/*m_RadiosityFilterPasses*/,
				     2/*m_RadiosityRenderPasses*/,
				   255/*m_RadiosityIntensity*/ );
	} // if //

	// Speed FX
	if( m_bSpeedFXTestMode )
	{
		SpeedFXManualSpeedForThisFrame( 1.0f );
	} // if //

	if( m_bSpeedFX && m_bSpeedFXUserFlag && m_bSpeedFXUserFlagCurrentFrame )
	{
		CVehicle *pVehicle = FindPlayerVehicle();
		bool bUseSpeedFX = true;

		if( m_fSpeedFXManualSpeedCurrentFrame == 0.0f )
		{
			if( pVehicle )
			{
				Int32 vehicleType = pVehicle->GetVehicleType();

				// Don't use the FX for the following vehicle types
				if( vehicleType == VEHICLE_TYPE_PLANE ||
					vehicleType == VEHICLE_TYPE_HELI  ||
					vehicleType == VEHICLE_TYPE_BOAT  ||
					vehicleType == VEHICLE_TYPE_TRAIN )
				{
					bUseSpeedFX = false;
				} // if //
				else
				{
					if(pVehicle->GetVehicleType()==VEHICLE_TYPE_CAR)
					{
						CAutomobile *pCar = (CAutomobile *)pVehicle;
						if(pCar->hFlagsLocal &HF_NOS_INSTALLED && pCar->m_fTyreTemp < 0.0)
						{

//sprintf( gString, "Gas : %.2f", pVehicle->GetGasPedal() );
//VarConsole.AddDebugOutput( gString );
							float fFwdSpeed = DotProduct(pCar->GetMoveSpeed(), pCar->GetMatrix().GetForward());
							if(fFwdSpeed > 0.2f)// && pVehicle->GetGasPedal() > 0.0f )
							{
								fFwdSpeed = 2.0f*fFwdSpeed*(1.0f + pCar->GetGasPedal());
								if(fFwdSpeed > 1.0f)
									fFwdSpeed = 1.0f;
							
								// Using nitros
								SpeedFXManualSpeedForThisFrame(fFwdSpeed); // Use full speed FX!
//sprintf( gString, "---NITRO---" );
//VarConsole.AddDebugOutput( gString );

								goto Use_manual_speed_FX;
							} // if //
						} // if //
					} // if //
				} // else //
			} // if //
			else
			{
				bUseSpeedFX = false; // No speed FX if player is not in a vehicle!
			} // else //

			if( CCutsceneMgr::IsCutsceneRunning() ) bUseSpeedFX = false; // No speed FX if cutscenes active

			if( bUseSpeedFX )
			{
				SpeedFX( FindPlayerSpeed().Magnitude() );
			} // if //
		} // if //
		else
		{
			Use_manual_speed_FX:;

			SpeedFX( m_fSpeedFXManualSpeedCurrentFrame );
		} // else //
	} // if //

	m_fSpeedFXManualSpeedCurrentFrame = 0.0f; // Must be reseted here!
	m_bSpeedFXUserFlagCurrentFrame    = true; // Must be reseted here!

	//

	m_bInCutscene = false;
	if( CCutsceneMgr::IsCutsceneRunning() ||
		CCutsceneMgr::IsCutsceneProcessing() ) m_bInCutscene = true;

	if( m_bNightVision )
	{
		if( !m_bInCutscene ) // Don't use this FX if cutscene is active!
		{
			NightVision( /*m_NightVisionCol,*/ m_NightVisionMainCol );
			Grain( m_NightVisionGrainStrength, true );
		} // if //
	} // if //
	else
	{
		m_fNightVisionSwitchOnFXCount = m_fNightVisionSwitchOnFXTime;
	} // else //

	if( m_bInfraredVision )
	{
		if( !m_bInCutscene ) // Don't use this FX if cutscene is active!
		{
			InfraredVision( m_InfraredVisionCol, m_InfraredVisionMainCol );
			Grain( m_InfraredVisionGrainStrength, true );
		} // if //
	} // if //

//-------------------------------------------------------------------------------------

	//CPostEffects::Grain(64,true);
	//CPostEffects::DepthOfFieldWithZFog(fogcol);
	//CPostEffects::HeatHaze(bHighStrength);
/* commented out as it is not used
	if(m_bHilightEnable)
		CPostEffects::HighlightGlow(CTimeCycle::GetHighLightMinIntensity(), //m_hilightMinIntensity, 
									m_hilightStrength, 
									m_hilightScale, 
									m_hilightOffset, 
									m_hilightMblur );*/

	if( m_bRadiosity && !m_bDarknessFilter /* Darkness filter uses its own radiosity! */ )
	{
		if( !m_bRadiosityBypassTimeCycleIntensityLimit )
		{
			Radiosity( CTimeCycle::GetHighLightMinIntensity(),
					   m_RadiosityFilterPasses,
					   m_RadiosityRenderPasses,
					   m_RadiosityIntensity );
		} // if //
		else
		{
			Radiosity( m_RadiosityIntensityLimit,
					   m_RadiosityFilterPasses,
					   m_RadiosityRenderPasses,
					   m_RadiosityIntensity );
		} // else //
	} // if //

	if (m_bRainEnable || currentStrength != 0)
	{
		if (currentStrength < (CWeather::Rain * 128))
			currentStrength++;
		if (currentStrength > (CWeather::Rain * 128))
			currentStrength--;
		currentStrength = MAX(0, currentStrength);
		if (!CCullZones::CamNoRain() &&
	 		!CCullZones::PlayerNoRain() &&
 			CWeather::UnderWaterness <= 0.0f &&
 			CGame::currArea == AREA_MAIN_MAP &&
 			TheCamera.GetPosition().z <= 900.0f)
 			{
 				CPostEffects::Grain(currentStrength/4, TRUE);
 			} // if //
		//CPostEffects::RainDropFilter();
	}
	if(m_bGrainEnable)
		CPostEffects::Grain(m_grainStrength, true);

	// Heat haze FX ( also used if we're underwater )
	if( m_bHeatHazeFX               || /* Designer's control flag for heat haze   */
	    CWeather::HeatHaze > 0.0f   || /* Weather control for heat haze           */
	    g_fxMan.m_foundHeatHazeInfo || /* Particle system's control for heat haze */
	    CWeather::UnderWaterness >= GetWaterFXStartUnderWaterness() ) /* Under waterness' control for heat haze FX */
	{
		// Priority #1 : the "m_bHeatHazeFX" flag
		// Priority #2 : "under waterness"
		// Priority #3 : weather control
		// Priority #4 : fx manager (particle system using the alpha mask mode)

		if( m_bHeatHazeFX ) /* Priority #1 */
		{
			HeatHazeFX( 1.0f, false );
		} // if //
		else
		if( CWeather::UnderWaterness >= GetWaterFXStartUnderWaterness() ) /* Priority #2 */
		{
			HeatHazeFX( 1.0f, false );
		} // if //
		else
		if( CWeather::HeatHaze > 0.0f ) /* Priority #3 */
		{
			HeatHazeFX( CWeather::HeatHazeFXControl, false );
		} // if //
		else
		if( g_fxMan.m_foundHeatHazeInfo ) /* Priority #4 */
		{
			HeatHazeFX( 1.0f, true );
		} // if //

		/*
		float fIntensity = CWeather::HeatHazeFXControl;                                      // Get as default the heat haze for the weather
		if( m_bHeatHazeFX )                                               fIntensity = 1.0f; // Set heat haze to maximum if flag is turned on
		if( CWeather::UnderWaterness >= GetWaterFXStartUnderWaterness() ) fIntensity = 1.0f; // Use this to make underwaterness a bit blurry and dynamic. //CMaths::Max(CWeather::UnderWaterness, fIntensity);

		HeatHazeFX( fIntensity, false );
		*/

//		WaveFilter( 16, 110, 2.5f );
	} // if //

	// Enable Water filter when under the water
	static float gfWaterGreen = 0.0f;
	float fWaterGreenMax = 24;

	if (m_waterEnable || CWeather::UnderWaterness >= GetWaterFXStartUnderWaterness())
	{
		// Set the water FX color depending on the depth...
		float fDarknessMult = 1.0f; // Must be 1.0!

		if( m_bWaterDepthDarkness )
		{
			float fCurrentDepth = CWeather::WaterDepth;
			if( fCurrentDepth > m_fWaterFullDarknessDepth ) fCurrentDepth = m_fWaterFullDarknessDepth;

			fDarknessMult = 1.0f - ( fCurrentDepth / m_fWaterFullDarknessDepth );
		} // if //

		RwRGBA col;
		float fXoffsetFactor = 1.0f;

#ifdef GTA_PS2
		// Water color for PS2
		col.red   = RwUInt8( (float)m_waterCol.red   * fDarknessMult );
		col.green = RwUInt8( (float)m_waterCol.green * fDarknessMult );
		col.blue  = RwUInt8( (float)m_waterCol.blue  * fDarknessMult );
#else
		// Water color for PC & XBOX
		Int32 r = m_waterCol.red;
		Int32 g = m_waterCol.green;
		Int32 b = m_waterCol.blue;

		Int32 plusR = 184;
		Int32 plusG = 184 + gfWaterGreen;
		Int32 plusB = 184;

		r += plusR; if( r > 255 ) r = 255;
		g += plusG; if( g > 255 ) g = 255;
		b += plusB; if( b > 255 ) b = 255;

		col.red   = RwUInt8( (float)r * fDarknessMult );
		col.green = RwUInt8( (float)g * fDarknessMult );
		col.blue  = RwUInt8( (float)b * fDarknessMult );

		// Increase the water green level...
		gfWaterGreen += CTimer::GetTimeStep();
		if( gfWaterGreen > fWaterGreenMax ) gfWaterGreen = fWaterGreenMax;

		// Use the green level also to fade the wave effect in (fXoffsetFactor)!
		fXoffsetFactor = gfWaterGreen / fWaterGreenMax;

/* Debug info
//CWeather::UnderWaterness >= GetWaterFXStartUnderWaterness()
sprintf( gString, "r = %d, g = %d, b = %d, uw = %.2f, t = %.2f", col.red, col.green, col.blue,
																 CWeather::UnderWaterness,
																 CTimer::GetTimeStep() );
VarConsole.AddDebugOutput( gString );
*/

#endif

		// Draw the FX
		CPostEffects::UnderWaterRipple( col, fXoffsetFactor * m_xoffset * HUD_MULT_X, m_yoffset * HUD_MULT_Y,
			                            m_waterStrength, m_waterSpeed, m_waterFreq );
	} // if //
#ifndef GTA_PS2
	else
	{
		// This is for PC & XBOX only!
		gfWaterGreen = 0.0f;
	//	CPostEffects::UnderWaterRippleFadeToFX();
	} // else //
#endif

	// The CCTV FX
	if( m_bCCTV )
	{
		CCTV();
	} // if //

#ifdef GTA_PS2

//	commented out as it is not used
	if (m_smokeyEnable)
		CPostEffects::SmokeyFilter(m_smokeyStrength);

	ResetRwRenderState();
#endif
	
	static bool8 done_wait_one_frame = FALSE;
	static bool8 m_SavePhotoInGallery = FALSE;	

	// quit taking photo if we have run into a save icon:
	if (CWeapon::ms_bTakePhoto && FrontEndMenuManager.m_JustEnteredSaveZone)
	{
		CWeapon::ms_bTakePhoto = false;
		m_bSavePhotoFromScript = false;
	}

	if (!done_wait_one_frame && CWeapon::ms_bTakePhoto)
	{
#if defined (GTA_PS2) || defined (GTA_XBOX)  // photos are always added to gallery on pc
		if(	(CPad::GetPad(0)->GetLeftShoulder1()) || (m_bSavePhotoFromScript))
#elif defined (GTA_PC)
		if (FrontEndMenuManager.m_PrefsSavePhotosToGallery)
#endif
		{
			m_SavePhotoInGallery = TRUE;
		}
		done_wait_one_frame = TRUE;
	}
	else
	{
		if (done_wait_one_frame && CWeapon::ms_bTakePhoto)
		{
			done_wait_one_frame = FALSE;
	
			bool8 createdDir = FALSE;

			CTimer::Suspend();

			if (m_SavePhotoInGallery)
			{
				//RwCameraShowRaster(Scene.camera, NULL, 0);  // this fucked it all up

				LOAD_CODE_OVERLAY(jpeg);
#if defined (GTA_PS2) || defined (GTA_XBOX)
				// ps2/xbox: save to buffer
				unsigned int jpeg_buff_size = MAX_JPEG_SIZE;
				FrontEndMenuManager.gallery_image_buffer = (char*)GtaMalloc(MAX_JPEG_SIZE);  // gets freed in frontend after we have done with saving photos
				JPegCompressScreenToBuffer((RwCamera*)TheCamera, &FrontEndMenuManager.gallery_image_buffer,&jpeg_buff_size);

				FrontEndMenuManager.m_bEnteredViaGallery = TRUE;
				FrontEndMenuManager.m_bStartUpFrontEndRequested = TRUE;

#elif defined (GTA_PC)			

				// on pc weapons get rendered after the RenderEffects()... CPostEffects::Render() call
				// so we need to render the weapons now if we want them to be in the shot!
				CVisibilityPlugins::RenderWeaponPedsForPC();
				CVisibilityPlugins::ResetWeaponPedsForPC();

				// pc: saves to "my documents" in gallery dir:
				CFileMgr::SetDirMyDocuments();

				uint32 file_count = 1;  // starts at 1

				// increment file number until we have a new filename:
				sprintf(gString, "Gallery\\gallery%d.jpg", file_count);
				FILE* fp=NULL;
				while ((fp = fopen(gString, "r")) != NULL)
				{
					file_count++;
					sprintf(gString, "Gallery\\gallery%d.jpg", file_count);
					fclose(fp);
				}

				// save this image under the filename:
				JPegCompressScreenToFile((RwCamera*)TheCamera, gString);

				// set current directory back to game dir
				CFileMgr::SetDir("");
	
#endif
				REMOVE_CODE_OVERLAY();
			}
			CTimer::Resume();

			// Trigger a visual effect. The screen will flash and the scene will freeze for a moment
			if (!FrontEndMenuManager.m_bStartUpFrontEndRequested) CSpecialFX::TriggerCameraFlash();

			m_SavePhotoInGallery = FALSE;
			CWeapon::ms_bTakePhoto = false;
			m_bSavePhotoFromScript = false;
		}
	}
 #ifdef GTA_PS2
    if (!RsCameraBeginUpdate(Scene.camera))
    {
    	ASSERT(0);
    }
#endif
}



#ifdef GTA_PS2
//
// name:		ResetRwRenderState
// description:	Reset render state back to what RW intended
//
void CPostEffects::ResetRwRenderState()
{

	const uint32 numGifQWords = 4;
	const uint32 numDMAQWords = 1 + numGifQWords;
	
	if (!_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords))
		return;

	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );

		ADDTOPKT(SCE_GS_TEST_1, 	skyTest_1);
		ADDTOPKT(GS_ALPHA_1, 		skyAlpha_1);
		ADDTOPKT(GS_FOGCOL, 		skyFogcol);
		ADDTOPKT(SCE_GS_ZBUF_1, 	skyZbuf_1);
		
		//LOAD_GIFTAG_QWC();
		ASSERT(_numOfQuadWords == numGifQWords);
	}
	RWDMA_LOCAL_BLOCK_END();
}

//
// name:		DrawScreenIn32PixelStrips
// description:	Draw the screen in 32 pixel wide strips. This is the most optimal way to 
//				draw to the whole screen. Have to do this as a macro as ADDTOPKT doesnt
//				work outside of the function that the packet was generated in
const int32 DRAW_SCREEN_IN_32_PIXEL_STRIPS_NUM_QWORDS	 =	(2 + ((SCREEN_DRAW_WIDTH>>5)*4));

#define DrawScreenIn32PixelStrips(r, g, b, a)																\
MACRO_START																									\
{																											\
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );							\
	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(r, g, b, a, 0) );												\
																											\
	/* Draw screen buffer in 32 pixel strips	*/															\
	const int32 count = (SCREEN_DRAW_WIDTH>>5);																\
	for (RwInt32 i=0;i<count;i++)																			\
	{																										\
/*		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(i<<5), GETUVCOORD_NEAR(0)));						\
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));						\
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((i+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT))); \
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));	*/\
																											\
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(i<<5), GETUVCOORD(0)));						        \
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));						\
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((i+1)<<5), GETUVCOORD(SCREEN_DRAW_HEIGHT)));           \
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));	\
	}																										\
}																											\
MACRO_STOP

//----------------------------------------------------------------------
// 1-Pass Full Screen Motion Blur
//----------------------------------------------------------------------

/*void	CChannelFilter::MotionBlur( RwInt32 strength, RwReal vortexStrength, RwTexture *pMaskTexture, RwReal maskScale )
{
	RwInt32 width, psm;
	unsigned long tex0;
	RwRaster *pSrcRaster;
	pSrcRaster = m_pDisplayBuffer;
	unsigned long dwRasterAddr = GetRasterInfo(m_pFrontBuffer, width, psm, tex0);
	unsigned long dwTexRasterAddr = RpSkyTexCacheRasterGetAddr(pSrcRaster);
	RwInt32 pMaskAddr, pCLUTAddr;
	RwRaster *pMaskRaster;
	if ( pMaskTexture != NULL )
	{
		pMaskRaster = RwTextureGetRaster(pMaskTexture);
		RpSkyTexCacheRasterLock(pMaskRaster,TRUE);
		pMaskAddr = RpSkyTexCacheRasterGetAddr(pMaskRaster);
		
		SetGrayScaleCLUTRaster256(pMaskTexture,strength);
		RpSkyTexCacheRasterLock(m_pGrayScaleCLUTRaster256,TRUE);
		pCLUTAddr = RpSkyTexCacheRasterGetAddr(m_pGrayScaleCLUTRaster256);
	}
	
	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 32) )
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );
	
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
	
	if ( pMaskTexture != NULL )
	{
		// copy mask to framebuffer alpha
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0x00ffffff) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(pMaskAddr, 64>>6, SCE_GS_PSMT8, 10, 10, 1, SCE_GS_DECAL,
																						pCLUTAddr, SCE_GS_PSMCT32, 0, 0, 1) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(10), GETUVCOORD(6)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(RwRasterGetWidth(pMaskRaster)-20*maskScale),
																			GETUVCOORD(RwRasterGetHeight(pMaskRaster)-12*maskScale)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	}
	
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwTexRasterAddr, width>>6, psm, 10, 10,
							1,							//	TCC:texture color component,1:RGBA
							SCE_GS_DECAL,
							0,							//	CBP:CLUT buffer base pointer, addr/64
							0,							//	CPSM:CLUT pixel storage format, PSMCT32
							0,							//	CSM:CLUT storage mode, CSM1
							0,							//	CLUT entry offset
							0));						//	CLUT buffer load control:temp buffer contents are not changed
	if ( vortexStrength > 0 )
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
	else
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	RwInt32 y = 0;
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, y));
	if ( pMaskRaster != NULL )
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,1,1,0) );
	else
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,1,strength) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
	if ( vortexStrength > 1 )
		vortexStrength = 1;
	vortexStrength *= 0.1f;
	RwInt32 x = width * vortexStrength;
	y = SCREEN_DRAW_HEIGHT * vortexStrength;
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(x), GETUVCOORD(y)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width - x), GETUVCOORD(SCREEN_DRAW_HEIGHT - y)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	
	LOAD_GIFTAG_QWC();
    RWDMA_LOCAL_BLOCK_END();
    
	if ( pMaskTexture != NULL )
	{
		RpSkyTexCacheRasterLock(m_pGrayScaleCLUTRaster256,FALSE);
		RpSkyTexCacheRasterLock(pMaskRaster,FALSE);
	}
}*/
#endif
//----------------------------------------------------------------------
// NightVision Filter
//----------------------------------------------------------------------

void	CPostEffects::NightVision( RwRGBA colour )
{
//	RwRaster* pDisplayBuffer = RpSkyGetDisplayBufferRaster();
//	RwRaster* pDrawBuffer    = RpSkyGetDrawBufferRaster();
	RwRaster* pDrawBuffer    = ms_imf.pRasterDrawBuffer;

	// - Switch on FX - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Here we just "add" the scene itself to the current framebuffer "fxLoop" times
	// where "fxLoop" becomes smaller and the scene intensity goes towards normal
	// every frame.
	if( m_fNightVisionSwitchOnFXCount > 0.0f )
	{
		m_fNightVisionSwitchOnFXCount -= CTimer::GetTimeStep() / 1.0f;
		if( m_fNightVisionSwitchOnFXCount <= 0.0f )
		{
			m_fNightVisionSwitchOnFXCount = 0.0f;
		} // if //
	#ifdef GTA_PS2
		if(	RwCameraBeginUpdate(Scene.camera) )
	#endif
		{
		// Store and set render states
		ImmediateModeRenderStatesStore();
		ImmediateModeRenderStatesSet();

		RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDONE );
		RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE );

		Int32 fxLoop = (Int32)m_fNightVisionSwitchOnFXCount;
		for( Int32 i = 0; i < fxLoop; i++ )
		{
			CPostEffects::DrawQuad( 0.0f, 0.0f,
									ms_imf.sizeDrawBufferX/*SCREEN_DRAW_WIDTH*/,
									ms_imf.sizeDrawBufferY/*SCREEN_DRAW_HEIGHT*/,
									8, 8, 8, 255,
									pDrawBuffer );
		} // for i //

		// Restore the render states
		ImmediateModeRenderStatesReStore();

	#ifdef GTA_PS2
			// End camera update
			RwCameraEndUpdate(Scene.camera);
	#endif		
		} // if //
	} // if //
	// - Switch on FX - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// Set main color
#ifdef GTA_PS2

	SetFilterMainColour( pDrawBuffer, colour );

#else

	// Remove the non FX specific colors
	// ---------------------------------

	// Store and set render states
	ImmediateModeRenderStatesStore();
	ImmediateModeRenderStatesSet();

	/* Test only

	Int32 w = SCREEN_WIDTH/2;
	Int32 h = SCREEN_HEIGHT;

	DrawQuad( 0.0f, 0.0f,
			  w, h,
			  0, 0, 0, 255, NULL ); // Clear background

//	RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDZERO );
//	RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDSRCCOLOR );

	RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDONE );
	RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE );

	DrawQuadSetUVs( ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV1,
					ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV1,
					ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV2,
					ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV2 );

	DrawQuad( 0.0f, 0.0f,
			  w, h,
			  255,   0,   0, 255, pDrawBuffer );

	DrawQuad( 0.0f, 0.0f,
			  w, h,
			    0, 255,   0, 255, pDrawBuffer );

	DrawQuad( 0.0f, 0.0f,
			  w, h,
			    0,   0, 255, 255, pDrawBuffer );

	DrawQuadSetDefaultUVs(); // Reset UV's!

//	*/

	RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDZERO ); // rwBLENDINVSRCALPHA
	RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDSRCCOLOR );

#ifdef GTA_PC
	DrawQuad( 0.0f, 0.0f,
			  SCREEN_WIDTH, SCREEN_HEIGHT,
			  32,
			  255,
			  32,
			  255,
			  NULL );
#endif

//@@@@
#ifdef GTA_XBOX
	DrawQuad( 0.0f, 0.0f,
			  SCREEN_WIDTH, SCREEN_HEIGHT,
			  4,
			  255,
			  4,
			  255,
			  NULL );
#endif

	// Restore the render states
	ImmediateModeRenderStatesReStore();

#endif // #ifdef GTA_PS2

	return;

	/* Original version

	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm;
	uint32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);

	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 32) )
		return;

	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );
	
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
	
	// 1st pass: brighten framebuffer
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0) );
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 10, 0) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	// 2nd pass: modulate framebuffer
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,0,1,0) );
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 1));
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(colour.red, colour.green, colour.blue, 128, 0) );
//	if ( colour.alpha > 0 )
//		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(colour.alpha>>2)) );
//	else
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
//	if ( colour.alpha > 0 )
//		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)-(colour.alpha>>4)) );
//	else
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	if ( colour.alpha > 0 )
	{
		// 3rd pass: whiten screen (night vision switching on)
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,colour.alpha>>1) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 0, 0) );
//		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(255-colour.alpha), 0) );
//		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT) - ((255-colour.alpha) << 4), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	}

	LOAD_GIFTAG_QWC();
	RWDMA_LOCAL_BLOCK_END();

//	*/

} // NightVision //

void	CPostEffects::NightVisionSetLights()
{
	if( !m_bNightVision || m_bInCutscene ) return;

	SetLightsForNightVision();
} // NightVisionSetLights //

//----------------------------------------------------------------------
// Wave filter
//----------------------------------------------------------------------

/*

void	CPostEffects::WaveFilter( Int32 speed, Int32 frequency, float fShift )
{
//	RwRaster* pDisplayBuffer = RpSkyGetDisplayBufferRaster();
//	RwRaster* pDrawBuffer    = RpSkyGetDrawBufferRaster();
	RwRaster* pDrawBuffer    = ms_imf.pRasterDrawBuffer;

	// Store and set render states
	ImmediateModeRenderStatesStore();
	ImmediateModeRenderStatesSet();

	// Render the filter
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, ms_imf.pRasterDrawBuffer );
//	RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDSRCALPHA );
//	RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDINVSRCALPHA );

	WaveFilterUpdate( speed, frequency, fShift );
//	RwIm2DRenderPrimitive( rwPRIMTYPETRILIST,  ms_imf.triStrip, NUM_V_TRI_FILTER_VERTICES );
	RwIm2DRenderPrimitive( rwPRIMTYPETRISTRIP, ms_imf.triStrip, NUM_V_TRI_STRIP_FILTER_VERTICES );

	// Restore the render states
	ImmediateModeRenderStatesReStore();
} // WaveFilter //

static float fWaveFilterAngle = 0.0f;

*/

/*

void CPostEffects::WaveFilterUpdate( Int32 speed, Int32 frequency, float fShift )
{
// 	sprintf( gString, "t step = %.2f", timeStep );
//	VarConsole.AddDebugOutput( gString );

	//

	RwReal fScreenSizeX = (RwReal)(ms_imf.sizeDrawBufferX);
	RwReal xLeft;
	RwReal xRight;
	float  shift;
	float  fAmplitude = fShift;

	//

	fWaveFilterAngle       += (float)(speed) * (CTimer::GetTimeStep() / 2.0f);
	UInt32 waveFilterAngle  = (UInt32)(fWaveFilterAngle);

	//

//	RwUInt8 r, g, b, a; // Colors
//	r = colour.red;
//	g = colour.green;
//	b = colour.blue;
//	a = colour.alpha;

	//

	Int32   n = 0;      // Vertex index

	for( Int32 i = 0; i < NUM_V_TRI_STRIP_FILTER_QUADS; i++ )
	{
		if( i == 0 )
		{
			// Update the first 2 triangles...

			//------------------//
			//                  //
			// Quad triangle #1 //
			//                  //
			// 1 ----------- 2  //
			// x |        /  x  //
			//   | #1   /       //
			//   |    /         //
			//   |  /           //
			//   |/             //
			//                  //
			//  3               //
			//                  //
			//------------------//

		//	shift  = fAmplitude * CMaths::Sin( DEGTORAD( angle + i*freq ) );
			shift  = fAmplitude * ms_imf.fSin[ ( waveFilterAngle + i*frequency ) & (WAVE_FILTER_SINUS_LUT_SIZE-1) ];
			xLeft  = 0.0f         + shift;
			xRight = fScreenSizeX + shift;

			RwIm2DVertexSetScreenX( &ms_imf.triStrip[n], xLeft  ); // Vertex 1 of triangle #1
		//	RwIm2DVertexSetIntRGBA( &ms_imf.triStrip[n], r, g, b, a );
			n++;
			RwIm2DVertexSetScreenX( &ms_imf.triStrip[n], xRight ); // Vertex 2 of triangle #1
		//	RwIm2DVertexSetIntRGBA( &ms_imf.triStrip[n], r, g, b, a );
			n++;

			//------------------//
			//                  //
			// Quad triangle #2 //
			//                  //
			//   ----------- 1  //
			//   |        /|    //
			//   |      /  |    //
			//   |    /    |    //
			//   |  /  #2  |    //
			//   |/________|    //
			//                  //
			//  3            2  //
			//  x            x  //
			//------------------//

		//	shift  = fAmplitude * CMaths::Sin( DEGTORAD( angle + (i+1)*freq ) );
			shift  = fAmplitude * ms_imf.fSin[ ( waveFilterAngle + (i+1)*frequency ) & (WAVE_FILTER_SINUS_LUT_SIZE-1) ];
			xLeft  = 0.0f         + shift;
			xRight = fScreenSizeX + shift;

			RwIm2DVertexSetScreenX( &ms_imf.triStrip[n], xLeft  ); // Vertex 3 of triangle #2
		//	RwIm2DVertexSetIntRGBA( &ms_imf.triStrip[n], r, g, b, a );
			n++;
			RwIm2DVertexSetScreenX( &ms_imf.triStrip[n], xRight ); // Vertex 2 of triangle #2
		//	RwIm2DVertexSetIntRGBA( &ms_imf.triStrip[n], r, g, b, a );
			n++;
		} // if //
		else
		{
			// Update all the other triangles...
			// (Only the bottom vertices of triangle #1 & #2 need to be updated from now on
			// because the upper ones use the datas from the vertices of the triangle above!)

		//	shift  = fAmplitude * CMaths::Sin( DEGTORAD( angle + (i+1)*freq ) );
			shift  = fAmplitude * ms_imf.fSin[ ( waveFilterAngle + (i+1)*frequency ) & (WAVE_FILTER_SINUS_LUT_SIZE-1) ];
			xLeft  = 0.0f         + shift;
			xRight = fScreenSizeX + shift;

			//------------------//
			//                  //
			// Quad triangle #1 //
			//                  //
			// 1 ----------- 2  //
			//   |        /     //
			//   | #1   /       //
			//   |    /         //
			//   |  /           //
			//   |/             //
			//                  //
			//  3               //
			//  x               //
			//------------------//

			RwIm2DVertexSetScreenX( &ms_imf.triStrip[n], xLeft  ); // Vertex 3 of triangle #1
		//	RwIm2DVertexSetIntRGBA( &ms_imf.triStrip[n], r, g, b, a );
			n++;

			//------------------//
			//                  //
			// Quad triangle #2 //
			//                  //
			//   ----------- 1  //
			//   |        /|    //
			//   |      /  |    //
			//   |    /    |    //
			//   |  /  #2  |    //
			//   |/________|    //
			//                  //
			//  3            2  //
			//               x  //
			//------------------//

			RwIm2DVertexSetScreenX( &ms_imf.triStrip[n], xRight ); // Vertex 2 of triangle #2
		//	RwIm2DVertexSetIntRGBA( &ms_imf.triStrip[n], r, g, b, a );
			n++;
		} // else //
	} // for i //
} // WaveFilterUpdate //

*/

//----------------------------------------------------------------------
// Infrared vision
//----------------------------------------------------------------------

void	CPostEffects::InfraredVision( RwRGBA colour, RwRGBA colourMain )
{
#ifdef GTA_PS2
	if(	RwCameraBeginUpdate(Scene.camera) )
#endif
	{
	//	RwRaster* pDisplayBuffer = RpSkyGetDisplayBufferRaster();
	//	RwRaster* pDrawBuffer    = RpSkyGetDrawBufferRaster();
		RwRaster* pDrawBuffer    = ms_imf.pRasterDrawBuffer;

		// Store and set render states
		ImmediateModeRenderStatesStore();
		ImmediateModeRenderStatesSet();

		// Render the FX
		RwUInt8 r, g, b, a;
		RwReal shift = m_fInfraredVisionFilterRadius;

		//                     |TopL|  |TopR|  |BotR|  |BotL|
		RwReal uShift[ 4 ] = { -shift,  shift,  shift, -shift }; // x (u)
		RwReal vShift[ 4 ] = { -shift, -shift,  shift,  shift }; // y (v)

		r = colour.red;   // 0;
		g = colour.green; // 80;
		b = colour.blue;  // 0;
		a = 255;          // Must be 255

//		RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSWRAP/*CLAMP*/ );
//		RwRenderStateSet( rwRENDERSTATETEXTURERASTER, ms_imf.pRasterDrawBuffer/*pDrawBuffer/*pDisplayBuffer/*NULL*/ );
		RwRenderStateSet( rwRENDERSTATESRCBLEND,       (void *)rwBLENDONE );
		RwRenderStateSet( rwRENDERSTATEDESTBLEND,      (void *)rwBLENDONE );

#ifdef GTA_PS2
		// The PS2 code
		RwRenderStateSet( rwRENDERSTATETEXTURERASTER, ms_imf.pRasterDrawBuffer/*pDrawBuffer/*pDisplayBuffer/*NULL*/ );

		RwIm2DVertexSetIntRGBA( &ms_imf.triangle[0], r, g, b, a );
		RwIm2DVertexSetIntRGBA( &ms_imf.triangle[1], r, g, b, a );
		RwIm2DVertexSetIntRGBA( &ms_imf.triangle[2], r, g, b, a );

		for( Int32 i = 0; i < 4; i++ )
		{
			RwIm2DVertexSetU( &ms_imf.triangle[0], ms_imf.uMinTri + uShift[ i ], ms_imf.recipCameraZ );
			RwIm2DVertexSetV( &ms_imf.triangle[0], ms_imf.vMinTri + vShift[ i ], ms_imf.recipCameraZ );

			RwIm2DVertexSetU( &ms_imf.triangle[1], ms_imf.uMaxTri + uShift[ i ], ms_imf.recipCameraZ );
			RwIm2DVertexSetV( &ms_imf.triangle[1], ms_imf.vMinTri + vShift[ i ], ms_imf.recipCameraZ );

			RwIm2DVertexSetU( &ms_imf.triangle[2], ms_imf.uMinTri + uShift[ i ], ms_imf.recipCameraZ );
			RwIm2DVertexSetV( &ms_imf.triangle[2], ms_imf.vMaxTri + vShift[ i ], ms_imf.recipCameraZ );

			RwIm2DRenderTriangle( ms_imf.triangle, 3, 0, 1, 2 );
		} // for i //
#else
		// The PC & XBOX code

		// Draw filter FX
		// --------------

		for( Int32 i = 0; i < 4; i++ )
		{
			DrawQuadSetUVs( ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV1, /* Top Left     */
							ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV1, /* Top Right    */
							ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV2, /* Bottom Right */
							ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV2  /* Bottom Left  */ );
/*
			DrawQuad( uShift[ i ]*100.0f - shift,
					  vShift[ i ]*100.0f - shift,
					  SCREEN_WIDTH       + shift*2.0f,
					  SCREEN_HEIGHT      + shift*2.0f,
					  r, g, b, a,
					  ms_imf.pRasterDrawBuffer );
*/
			float n = (float)(i+1);
			float m = shift * 100.0f;

			DrawQuad( -n * m,
					  -n * m,
					  SCREEN_WIDTH  + n * m * 2.0f,
					  SCREEN_HEIGHT + n * m * 2.0f,
					  r, g, b, a,
					  ms_imf.pRasterDrawBuffer );
		} // for i //

		//

		DrawQuadSetDefaultUVs(); // Reset UV's!

		// Remove the non FX specific colors
		// ---------------------------------

#ifdef GTA_PC
		r = 255;
		g = 64;
		b = 255;
		a = 255;
#endif

//@@@@
#ifdef GTA_XBOX
		r = 8;
		g = 8;
		b = 255;
		a = 255;
#endif

		RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDZERO /*rwBLENDINVSRCALPHA*/);
		RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDSRCCOLOR );

		DrawQuad( 0.0f,
				  0.0f,
				  SCREEN_WIDTH,
				  SCREEN_HEIGHT,
				  r, g, b, a,
				  NULL );
#endif // GTA_PS2

		// Restore the render states
		ImmediateModeRenderStatesReStore();

#ifdef GTA_PS2
		// End camera update
		RwCameraEndUpdate(Scene.camera);
#endif
		// Set main color
		SetFilterMainColour( pDrawBuffer, colourMain );
	} // if //
} // InfraredVision //

void	CPostEffects::InfraredVisionSetLightsForHeatObjects()
{
	if( !m_bInfraredVision || m_bInCutscene ) return;

	SetLightsForInfraredVisionHeatObjects();
} // InfraredVisionSetLightsForHeatObjects //

// ### This function currently supports peds only! ###
void	CPostEffects::InfraredVisionStoreAndSetLightsForHeatObjects( CPed* pPed )
{
	if( !m_bInfraredVision || m_bInCutscene ) return;

	// Change ped's FX color after dead
	RwRGBAReal fOriginalColor = m_fInfraredVisionHeatObjectCol; // Store original color

	if( pPed->GetPedState() == PED_DEAD )
	{
		Int32 deathTime = CTimer::GetTimeInMilliseconds() - pPed->m_nTimeOfDeath;
		if( deathTime < 0 ) deathTime = -deathTime;

		float fColorChanger = (float)deathTime / 10000.0f;

	//	m_fInfraredVisionHeatObjectCol.red   = 0.0f;
	//	m_fInfraredVisionHeatObjectCol.green = 0.0f;
	//	m_fInfraredVisionHeatObjectCol.blue  = 0.0f;

		m_fInfraredVisionHeatObjectCol.red   -= fColorChanger; // Take red away
		if( m_fInfraredVisionHeatObjectCol.red < 0.0f )
			m_fInfraredVisionHeatObjectCol.red = 0.0f;

		m_fInfraredVisionHeatObjectCol.green =  0.0f;

		m_fInfraredVisionHeatObjectCol.blue  += fColorChanger; // Add blue
		if( m_fInfraredVisionHeatObjectCol.blue > 1.0f )
			m_fInfraredVisionHeatObjectCol.blue = 1.0f;

//sprintf( gString, "Death time = %d", deathTime );
//VarConsole.AddDebugOutput( gString );
	} // if //

	StoreAndSetLightsForInfraredVisionHeatObjects();

	// Restore original color
	m_fInfraredVisionHeatObjectCol = fOriginalColor;
} // InfraredVisionStoreAndSetLightsForHeatObjects //

void	CPostEffects::InfraredVisionRestoreLightsForHeatObjects()
{
	if( !m_bInfraredVision || m_bInCutscene ) return;

	RestoreLightsForInfraredVisionHeatObjects();
} // InfraredVisionRestoreLightsForHeatObjects //

void	CPostEffects::InfraredVisionSetLightsForDefaultObjects()
{
	if( !m_bInfraredVision || m_bInCutscene ) return;

	SetLightsForInfraredVisionDefaultObjects();
} // InfraredVisionSetLightsForDefaultObjects //

//----------------------------------------------------------------------
// Heat haze (desert)
//----------------------------------------------------------------------
#if defined (GTA_PC) || (defined GTA_XBOX)
	#define MAX_NUM_HEAT_PARTICLES 180 /* Must be 180 */
#else
	#define MAX_NUM_HEAT_PARTICLES 120 /* Must be 120 */
#endif
static Int32 hpX[ MAX_NUM_HEAT_PARTICLES ];
static Int32 hpY[ MAX_NUM_HEAT_PARTICLES ];
static Int32 hpS[ MAX_NUM_HEAT_PARTICLES ];

void	CPostEffects::HeatHazeFXInit()
{
	// Set up current FX
	if( m_HeatHazeFXType != m_HeatHazeFXTypeLast )
	{
		switch( m_HeatHazeFXType )
		{
			case( 0 ):
				m_HeatHazeFXIntensity   = 80;
				m_HeatHazeFXRandomShift = 0;
				m_HeatHazeFXSpeedMin    = 12;
				m_HeatHazeFXSpeedMax    = 18;
				m_HeatHazeFXScanSizeX   = 47 * HUD_MULT_X;
				m_HeatHazeFXScanSizeY   = 47 * HUD_MULT_Y;
				m_HeatHazeFXRenderSizeX = 50 * HUD_MULT_X;
				m_HeatHazeFXRenderSizeY = 50 * HUD_MULT_Y;
				break;

			case( 1 ):
				m_HeatHazeFXIntensity   = 32;
				m_HeatHazeFXRandomShift = 0;
				m_HeatHazeFXSpeedMin    = 6;
				m_HeatHazeFXSpeedMax    = 10;
				m_HeatHazeFXScanSizeX   = 100 * HUD_MULT_X;
				m_HeatHazeFXScanSizeY   = 52 * HUD_MULT_Y;
				m_HeatHazeFXRenderSizeX = 100 * HUD_MULT_X;
				m_HeatHazeFXRenderSizeY = 60 * HUD_MULT_Y;
				break;

			case( 2 ):
				m_HeatHazeFXIntensity   = 32;
				m_HeatHazeFXRandomShift = 0;
				m_HeatHazeFXSpeedMin    = 4;
				m_HeatHazeFXSpeedMax    = 8;
				m_HeatHazeFXScanSizeX   = 70 * HUD_MULT_X;
				m_HeatHazeFXScanSizeY   = 70 * HUD_MULT_Y;
				m_HeatHazeFXRenderSizeX = 80 * HUD_MULT_X;
				m_HeatHazeFXRenderSizeY = 80 * HUD_MULT_Y;
				break;

			case( 3 ):
				m_HeatHazeFXIntensity   = 150;
				m_HeatHazeFXRandomShift = 0;
				m_HeatHazeFXSpeedMin    = 5;
				m_HeatHazeFXSpeedMax    = 8;
				m_HeatHazeFXScanSizeX   = 60 * HUD_MULT_X;
				m_HeatHazeFXScanSizeY   = 24 * HUD_MULT_Y;
				m_HeatHazeFXRenderSizeX = 62 * HUD_MULT_X;
				m_HeatHazeFXRenderSizeY = 24 * HUD_MULT_Y;
				break;

			case( 4 ):
				m_HeatHazeFXIntensity   = 150;
				m_HeatHazeFXRandomShift = 1;
				m_HeatHazeFXSpeedMin    = 5;
				m_HeatHazeFXSpeedMax    = 8;
				m_HeatHazeFXScanSizeX   = 60 * HUD_MULT_X;
				m_HeatHazeFXScanSizeY   = 24 * HUD_MULT_Y;
				m_HeatHazeFXRenderSizeX = 62 * HUD_MULT_X;
				m_HeatHazeFXRenderSizeY = 24 * HUD_MULT_Y;
				break;
		} // switch //

		m_HeatHazeFXTypeLast = m_HeatHazeFXType;

		for( Int32 i = 0; i < MAX_NUM_HEAT_PARTICLES; i++ )
		{
		#ifdef GTA_PS2
			hpX[ i ] = CGeneral::GetRandomNumberInRange(0 /*m_HeatHazeFXScanSizeX*/, SCREEN_WIDTH -0-m_HeatHazeFXScanSizeX);
			hpY[ i ] = CGeneral::GetRandomNumberInRange(0 /*m_HeatHazeFXScanSizeY*/, SCREEN_HEIGHT-0-m_HeatHazeFXScanSizeY);
		#else
			hpX[ i ] = CGeneral::GetRandomNumberInRange(0 /*m_HeatHazeFXScanSizeX*/, RwRasterGetWidth(pRasterFrontBuffer) -0-m_HeatHazeFXScanSizeX);
			hpY[ i ] = CGeneral::GetRandomNumberInRange(0 /*m_HeatHazeFXScanSizeY*/, RwRasterGetHeight(pRasterFrontBuffer)-0-m_HeatHazeFXScanSizeY);
		#endif
			hpS[ i ] = CGeneral::GetRandomNumberInRange(m_HeatHazeFXSpeedMin, m_HeatHazeFXSpeedMax);
		} // for i //
	} // if //
} // HeatHazeFXInit //

void	CPostEffects::HeatHazeFX( float fIntensity, bool bAlphaMaskMode )
{
#if defined (GTA_PC) || (defined GTA_XBOX)
	bool bStencil = true;

	// No stencil buffer if 16bit screen!
	if( RwRasterGetDepth( RwCameraGetRaster(Scene.camera) ) == 16 )
	{
		bStencil = false;

//		sprintf( gString, "16bit... No stencil!" );
//	 	VarConsole.AddDebugOutput( gString );
	} // if //
#endif

	// Reset alpha and set the masks (For PC & XBOX the stencil buffer is used!)
	if( bAlphaMaskMode )
	{
		// We need to reset the Z buffer for this!!!!!!!
		RwRGBA colDummy = { 0, 0, 0, 0 };
		RwCameraClear(Scene.camera, &colDummy, rwCAMERACLEARZ ); // Clear Z-buffer only!

	#ifdef GTA_PS2
		if(	RwCameraBeginUpdate(Scene.camera) )
	#endif
		{
		// Store and set render states
		ImmediateModeRenderStatesStore();
		ImmediateModeRenderStatesSet();

		RwRenderStateSet( rwRENDERSTATESRCBLEND,     (void *)rwBLENDSRCALPHA );
		RwRenderStateSet( rwRENDERSTATEDESTBLEND,    (void *)rwBLENDONE );
//	    RwRenderStateSet( rwRENDERSTATEZTESTENABLE,  (void *)FALSE); // No Z test!
//	    RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *)FALSE); // No Z write!

		// Clear screen (reset alpha to 255)
	#ifdef GTA_PS2
		// ### PS2 ###
		CPostEffects::DrawQuad( 0.0f, 0.0f,
								/*ms_imf.sizeDrawBufferX*/SCREEN_DRAW_WIDTH,
								/*ms_imf.sizeDrawBufferY*/SCREEN_DRAW_HEIGHT,
								0, 0, 0, 255,
								NULL );
	#else
		// ### PC & XBOX ###
		if( bStencil )
		{
			RwRenderStateSet( rwRENDERSTATESTENCILENABLE,	   (void *)TRUE );
			RwRenderStateSet( rwRENDERSTATESTENCILFAIL,		   (void *)rwSTENCILOPERATIONKEEP );
			RwRenderStateSet( rwRENDERSTATESTENCILZFAIL,	   (void *)rwSTENCILOPERATIONKEEP );
			RwRenderStateSet( rwRENDERSTATESTENCILPASS,		   (void *)rwSTENCILOPERATIONREPLACE );
			RwRenderStateSet( rwRENDERSTATESTENCILFUNCTIONREF, (void *)0 ); // Reset to "0"!
			RwRenderStateSet( rwRENDERSTATESTENCILFUNCTION,	   (void *)rwSTENCILFUNCTIONALWAYS );
		} // if //

		CPostEffects::DrawQuad( 0.0f, 0.0f,
								/*ms_imf.sizeDrawBufferX*//*RwRasterGetWidth(pRasterFrontBuffer)*/ SCREEN_WIDTH,
								/*ms_imf.sizeDrawBufferY*//*RwRasterGetHeight(pRasterFrontBuffer)*/SCREEN_HEIGHT,
								0, 0, 0, 255,
								NULL );
	#endif
		//==========================================================================

		// Here comes the particle system which sets the alpha to 0 to make the
		// heat haze FX visible...
		// --------------------------------------------------------------------

		if( bStencil )
		{
			// Set the stencil for particles to "1"
			RwRenderStateSet( rwRENDERSTATESTENCILFUNCTIONREF, (void *)1 );
		} // if //

		g_fx.Render(TheCamera.m_pRwCamera, true);

//bool se;
//RwRenderStateSet( rwRENDERSTATESTENCILENABLE, (void *)TRUE );
//RwRenderStateGet( rwRENDERSTATESTENCILENABLE, (void *)&se );
//if( !se ) ASSERT( 0 );

/*
		// Colorize the FX range (test only)
		CPostEffects::DrawQuad( SCREEN_DRAW_WIDTH/2-70, SCREEN_DRAW_HEIGHT/2+20,
								140,
								250,
								15, 15, 15, 255,
								NULL );

		// Set mask = alpha to 0 (test only)
		CPostEffects::DrawQuad( SCREEN_DRAW_WIDTH/2-70, SCREEN_DRAW_HEIGHT/2+20,
								140,
								250,
								0, 0, 0, 0,
								NULL );

		// Set mask = alpha to 0 (test only)
		CPostEffects::DrawQuad( 0, 0, 100, 100,
								0, 0, 0, 0,
								NULL );
*/
		//==========================================================================

		// Restore the render states
		ImmediateModeRenderStatesReStore();

		#ifdef GTA_PS2
			// End camera update
			RwCameraEndUpdate(Scene.camera);
		#endif	
		} // if //
#ifdef GTA_PS2
		else
		{
			return; // Bye if "RwCameraBeginUpdate" fails!
		} // else //
#endif
	} // if //
	else
	{
		// Don't allow test mode if the mask mode is inactive!
		m_bHeatHazeMaskModeTest = false;
	} // else //

	// Make sure intensity is in range!
	if( fIntensity < 0.0f ) fIntensity = 0.0f;
	if( fIntensity > 1.0f ) fIntensity = 1.0f;

	HeatHazeFXInit();

	//----------------------------------------------------------------------------
#ifdef GTA_PS2
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm, zpsm;
	int32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
	int32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	RwRaster *pSrcRaster;

//	if ( mblur )
//		pSrcRaster = RpSkyGetDisplayBufferRaster();
//	else
		pSrcRaster = pDrawBuffer;

	int32 dwSrcRasterAddr = GetRasterInfo(pSrcRaster, width, psm);




	const int32 numGeneralPackets  = 19;
	int32 numParticleRenderPackets = 1 + 4*MAX_NUM_HEAT_PARTICLES;
	if(m_bHeatHazeMaskModeTest) numParticleRenderPackets = 3;


	const uint32 numGifQWords	= numGeneralPackets + numParticleRenderPackets;
	const uint32 numDMAQWords	= 1 + numGifQWords;
	
	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords) )
		return;

	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );
		//----------------------------------------------------------------------------

		if( !bAlphaMaskMode )
		{
			ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
		}
		else
		{
			ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0, /* Alpha test */
													SCE_GS_ALPHA_EQUAL/*NOTEQUAL*/,
													0, /* Reference value */
													SCE_GS_AFAIL_KEEP,
													1, /* Destination alpha test */
													0, /* Destination alpha test mode */
													1, /* Depth test */
													SCE_GS_ZALWAYS) );
		} // else //

		ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
		ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
		ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
		ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1) );

		// 1) Copy framebuffer to workbuffer
		// ---------------------------------

		Int32 downsampleSizeX = SCREEN_DRAW_WIDTH;
		Int32 downsampleSizeY = SCREEN_DRAW_HEIGHT;

		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwSrcRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	//	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR,  SCE_GS_LINEAR,  0, 0, 0 ) );
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );

	if( !m_bHeatHazeMaskModeTest )
	{
		// ### Default mode ###
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		//
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );

		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(downsampleSizeX), GETPRIMCOORD(downsampleSizeY), 0) );
	} // if //
	else
	{
		// ### Mask debug mode ###
		Int32 debugR = 255;
		Int32 debugG = 0;
		Int32 debugB = 0;

		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,0,0,1,0,0) );
		//
	///	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(debugR, debugG, debugB, 255, 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );

	///	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(debugR, debugG, debugB, 255, 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(downsampleSizeX), GETPRIMCOORD(downsampleSizeY), 0) );
	} // else //

		// 2) Render the particles
		// -----------------------

		Int32 intensity = (Int32)( (float)m_HeatHazeFXIntensity * fIntensity );

		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));

	//	if( m_bRadiosityLinearFilter )
	//	{
			ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
	//	} // if //
	//	else
	//	{
	//		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
	//	} // else //

	//	if( m_bRadiosityDebug )
	//	{
	//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
	//		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 255, 0) ); /* Dummy packet! */
	//	} // if //
	//	else
	//	{
	//		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,intensity>>1) );
	//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );

	//		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
			ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,1,intensity>>1) );
	//	} // else //

		if( !m_bHeatHazeMaskModeTest )
		{
			ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );

			const int32 numHeatParticles = MAX_NUM_HEAT_PARTICLES;

			Int32 scanSizeX = m_HeatHazeFXScanSizeX;
			Int32 scanSizeY = m_HeatHazeFXScanSizeY;

			Int32 renderSizeX = m_HeatHazeFXRenderSizeX;
			Int32 renderSizeY = m_HeatHazeFXRenderSizeY;

			Int32 renderCenterShiftX = ( renderSizeX - scanSizeX ) / 2;
			Int32 renderCenterShiftY = ( renderSizeY - scanSizeY ) / 2;

			for( Int32 i = 0; i < numHeatParticles; i++ )
			{
				Int32 scanX   = hpX[ i ];
				Int32 scanY   = hpY[ i ];
				Int32 renderX = scanX - renderCenterShiftX;
				Int32 renderY = scanY - renderCenterShiftY;

				if( m_HeatHazeFXRandomShift > 0 )
				{
					renderX += CGeneral::GetRandomNumberInRange(-m_HeatHazeFXRandomShift, m_HeatHazeFXRandomShift);
					renderY += CGeneral::GetRandomNumberInRange(-m_HeatHazeFXRandomShift, m_HeatHazeFXRandomShift);
				} // if //

				if( renderX < 0 )
				{
					renderX = 0;
					scanX += renderCenterShiftX;
				} // if //

				if( renderX > ( SCREEN_DRAW_WIDTH - renderSizeX ) )
				{
					renderX = SCREEN_DRAW_WIDTH - renderSizeX;
					scanX -= renderCenterShiftX;
				} // if //

				if( renderY < 0 )
				{
					renderY = 0;
					scanY += renderCenterShiftY;
				} // if //

				if( renderY > ( SCREEN_DRAW_HEIGHT - renderSizeY ) )
				{
					renderY = SCREEN_DRAW_HEIGHT - renderSizeY;
					scanY -= renderCenterShiftY;
				} // if //

				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX),
												  GETUVCOORD(scanY)) );
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX),
													 GETPRIMCOORD(renderY), 0) );

				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+scanSizeX),
												  GETUVCOORD(scanY+scanSizeY)) );
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX+renderSizeX),
												     GETPRIMCOORD(renderY+renderSizeY), 0) );

				// Update
				hpY[ i ] -= (Int32)( (float)hpS[ i ] * (CTimer::GetTimeStep() / 2.0f) );

				if( hpY[ i ] < 0/*scanSizeY*/ )
				{
					hpX[ i ] = CGeneral::GetRandomNumberInRange(0/*scanSizeX*/, SCREEN_DRAW_WIDTH-0-scanSizeX);
					hpY[ i ] = SCREEN_DRAW_HEIGHT-0-scanSizeY;
					hpS[ i ] = CGeneral::GetRandomNumberInRange(m_HeatHazeFXSpeedMin, m_HeatHazeFXSpeedMax);
				} // if //
			} // for i //
		} // if //
		else
		{
			ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,0,0,1,0,0) );

	///		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0),
	///										  GETUVCOORD(0)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0),
												 GETPRIMCOORD(0), 0) );

	///		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(SCREEN_DRAW_WIDTH),
	///										  GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(SCREEN_DRAW_WIDTH),
											     GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
		} // else //

		//----------------------------------------------------------------------------
		//LOAD_GIFTAG_QWC();
		ASSERT(_numOfQuadWords == numGifQWords);
	}
	RWDMA_LOCAL_BLOCK_END();
	//----------------------------------------------------------------------------
#else

	#ifdef GTA_PC
		GE_FORCE_5_FIX; // This must be put before the "RwCameraEndUpdate" before the raster copy!
		RwCameraEndUpdate(Scene.camera);
		RwRasterPushContext(pRasterFrontBuffer);
//		RsCameraBeginUpdate(Scene.camera); // ### Do NOT use this for "RwRasterPushContext" and "RwRasterPopContext"!! ###
		RwRasterRenderFast(RwCameraGetRaster(Scene.camera), 0, 0);
//		RwCameraEndUpdate(Scene.camera);
		RwRasterPopContext();
		RsCameraBeginUpdate(Scene.camera);
	#elif defined(GTA_XBOX)
		// im mode pRasterFrontBuffer >>> pRasterFrontBuffer
		RasterCopyPostEffects();
	#endif

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
//	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void *) rwALPHATESTFUNCTIONEQUAL);
//	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void *) 0);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
/// RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
/// RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
///	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
///	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);

///	Int32 downsampleSizeX = RwRasterGetWidth(pRasterFrontBuffer);
///	Int32 downsampleSizeY = RwRasterGetHeight(pRasterFrontBuffer);
	float u, v;

/*
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
	if( !m_bHeatHazeMaskModeTest )
	{
	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

		RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0.0f, 1.0f);
		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeX+1);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 0.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 1.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), downsampleSizeY+1);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

		RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), 1.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), 1.0f, 1.0f);
		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), downsampleSizeX+1);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), downsampleSizeY+1);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0, 0, 0, 255);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0, 0, 0, 255);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 0, 0, 0, 255);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), 0, 0, 0, 255);
		TempBufferVerticesStored += 4;
	}else
	{
		// ### Mask debug mode ###
		Int32 debugR = 255;
		Int32 debugG = 0;
		Int32 debugB = 0;
	//    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	//    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		
	//	RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	//  RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeX+1);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		
	//    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	//    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), downsampleSizeY+1);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		
	//	RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	//  RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), downsampleSizeX+1);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), downsampleSizeY+1);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), debugR, debugG, debugB, 255);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), debugR, debugG, debugB, 255);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), debugR, debugG, debugB, 255);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), debugR, debugG, debugB, 255);
		TempBufferVerticesStored += 4;
	}

	// 2) Render the particles
	// -----------------------
//	Int32 intensity = (Int32)( (float)m_HeatHazeFXIntensity * fIntensity );
	if (TempBufferVerticesStored > 2)
	{
		RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored);
	}
*/

	Int32 intensity = (Int32)( (float)m_HeatHazeFXIntensity * fIntensity );

	TempBufferVerticesStored = 0;

	if( bStencil )
	{
		// Draw the effects only where the stencil buffer equals "1"!
//		RwRenderStateSet( rwRENDERSTATESTENCILFAIL,			(void*)rwSTENCILOPERATIONKEEP );
//		RwRenderStateSet( rwRENDERSTATESTENCILZFAIL,		(void*)rwSTENCILOPERATIONKEEP );
		RwRenderStateSet( rwRENDERSTATESTENCILPASS,			(void*)rwSTENCILOPERATIONKEEP );
		RwRenderStateSet( rwRENDERSTATESTENCILFUNCTIONREF,	(void *)1 );
		RwRenderStateSet( rwRENDERSTATESTENCILFUNCTION,		(void *)rwSTENCILFUNCTIONEQUAL );
	} // if //

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	if( !m_bHeatHazeMaskModeTest )
	{
		Int32 numHeatParticles = MAX_NUM_HEAT_PARTICLES;

		Int32 scanSizeX = m_HeatHazeFXScanSizeX;
		Int32 scanSizeY = m_HeatHazeFXScanSizeY;

		Int32 renderSizeX = m_HeatHazeFXRenderSizeX;
		Int32 renderSizeY = m_HeatHazeFXRenderSizeY;

		Int32 renderCenterShiftX = ( renderSizeX - scanSizeX ) / 2;
		Int32 renderCenterShiftY = ( renderSizeY - scanSizeY ) / 2;

		for( Int32 i = 0; i < numHeatParticles; i++ )
		{
			Int32 scanX   = hpX[ i ];
			Int32 scanY   = hpY[ i ];
			Int32 renderX = scanX - renderCenterShiftX;
			Int32 renderY = scanY - renderCenterShiftY;

			if( m_HeatHazeFXRandomShift > 0 )
			{
				renderX += CGeneral::GetRandomNumberInRange(-m_HeatHazeFXRandomShift, m_HeatHazeFXRandomShift);
				renderY += CGeneral::GetRandomNumberInRange(-m_HeatHazeFXRandomShift, m_HeatHazeFXRandomShift);
			} // if //

			if( renderX < 0 )
			{
				renderX = 0;
				scanX += renderCenterShiftX;
			} // if //

			if( renderX > ( RwRasterGetWidth(pRasterFrontBuffer) - renderSizeX ) )
			{
				renderX = RwRasterGetWidth(pRasterFrontBuffer) - renderSizeX;
				scanX -= renderCenterShiftX;
			} // if //

			if( renderY < 0 )
			{
				renderY = 0;
				scanY += renderCenterShiftY;
			} // if //

			if( renderY > (RwRasterGetHeight(pRasterFrontBuffer) - renderSizeY ) )
			{
				renderY = RwRasterGetHeight(pRasterFrontBuffer) - renderSizeY;
				scanY -= renderCenterShiftY;
			} // if //
		
#if 0//def GTA_XBOX
			u =  (float) (scanX);
		 	v =  (float) (scanY);
#else
			u =  (float) (scanX) / (float) RwRasterGetWidth(pRasterFrontBuffer);
		 	v =  (float) (scanY) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif

			RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), u, 1.0f);
		    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), v, 1.0f);
			RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderX);
			RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderY);
			RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwIm2DGetNearScreenZ());
			RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

#if 0//def GTA_XBOX
			u =  (float) (scanX+scanSizeX);
		 	v =  (float) (scanY);
#else
			u =  (float) (scanX+scanSizeX) / (float) RwRasterGetWidth(pRasterFrontBuffer);
		 	v =  (float) (scanY) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
			RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
		    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
			RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderX+renderSizeX);
			RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderY);
			RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwIm2DGetNearScreenZ());
			RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

#if 0//def GTA_XBOX
			u =  (float) (scanX);
		 	v =  (float) (scanY+scanSizeY);
#else
			u =  (float) (scanX) / (float) RwRasterGetWidth(pRasterFrontBuffer);
		 	v =  (float) (scanY+scanSizeY) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif

			RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), u, 1.0f);
		    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), v, 1.0f);
			RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), renderX);
			RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), renderY+renderSizeY);
			RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwIm2DGetNearScreenZ());
			RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

#if 0//def GTA_XBOX
			u =  (float) (scanX+scanSizeX);
		 	v =  (float) (scanY+scanSizeY);
#else
			u =  (float) (scanX+scanSizeX) / (float) RwRasterGetWidth(pRasterFrontBuffer);
		 	v =  (float) (scanY+scanSizeY) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
			RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), u, 1.0f);
		    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), v, 1.0f);
			RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), renderX+renderSizeX);
			RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), renderY+renderSizeY);
			RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwIm2DGetNearScreenZ());
			RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 255, 255, 255, intensity);
			RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 255, 255, 255, intensity);
			RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 255, 255, 255, intensity);
			RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), 255, 255, 255, intensity);
			
			TempBufferRenderIndexList[TempBufferIndicesStored] = TempBufferVerticesStored;
			TempBufferRenderIndexList[TempBufferIndicesStored+1] = TempBufferVerticesStored + 2;
			TempBufferRenderIndexList[TempBufferIndicesStored+2] = TempBufferVerticesStored + 1;
			TempBufferRenderIndexList[TempBufferIndicesStored+3] = TempBufferVerticesStored + 1;
			TempBufferRenderIndexList[TempBufferIndicesStored+4] = TempBufferVerticesStored + 2;
			TempBufferRenderIndexList[TempBufferIndicesStored+5] = TempBufferVerticesStored + 3;
			TempBufferVerticesStored += 4;
			TempBufferIndicesStored += 6;

			// Update
			ASSERT(TempBufferIndicesStored < MAXTEMPBUFFERINDICES);
			ASSERT(TempBufferVerticesStored < MAX_TEMP_3DVERTICES);


			hpY[ i ] -= (Int32)( (float)hpS[ i ] * (CTimer::GetTimeStep() / 2.0f) );

			if( hpY[ i ] < 0/*scanSizeY*/ )
			{
				hpX[ i ] = CGeneral::GetRandomNumberInRange(0/*scanSizeX*/, RwRasterGetWidth(pRasterFrontBuffer)-0-scanSizeX);
				hpY[ i ] = RwRasterGetHeight(pRasterFrontBuffer)-0-scanSizeY;
				hpS[ i ] = CGeneral::GetRandomNumberInRange(m_HeatHazeFXSpeedMin, m_HeatHazeFXSpeedMax);
			} // if //

		}
	}else
	{
		Int32 debugR = 255;
		Int32 debugG = 0;
		Int32 debugB = 0;
	//    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	//    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		
	//	RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	//  RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwRasterGetWidth(pRasterFrontBuffer));
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

	//    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	//    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwRasterGetHeight(pRasterFrontBuffer));
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		
	//	RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	//  RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwRasterGetWidth(pRasterFrontBuffer));
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwRasterGetHeight(pRasterFrontBuffer));
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), debugR, debugG, debugB, intensity);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), debugR, debugG, debugB, intensity);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+2]), debugR, debugG, debugB, intensity);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+3]), debugR, debugG, debugB, intensity);
		TempBufferRenderIndexList[TempBufferIndicesStored] = TempBufferVerticesStored;
		TempBufferRenderIndexList[TempBufferIndicesStored+1] = TempBufferVerticesStored + 2;
		TempBufferRenderIndexList[TempBufferIndicesStored+2] = TempBufferVerticesStored + 1;
		TempBufferRenderIndexList[TempBufferIndicesStored+3] = TempBufferVerticesStored + 1;
		TempBufferRenderIndexList[TempBufferIndicesStored+4] = TempBufferVerticesStored + 2;
		TempBufferRenderIndexList[TempBufferIndicesStored+5] = TempBufferVerticesStored + 3;
		TempBufferVerticesStored += 4;
		TempBufferIndicesStored += 6;

		ASSERT(TempBufferIndicesStored < MAXTEMPBUFFERINDICES);
		ASSERT(TempBufferVerticesStored < MAX_TEMP_3DVERTICES);
	}
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
	if (TempBufferVerticesStored)
	{
		RwIm2DRenderIndexedPrimitive_BUGFIX(rwPRIMTYPETRILIST, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored, TempBufferRenderIndexList, TempBufferIndicesStored);
	}
	TempBufferVerticesStored = 0;
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

	if( bStencil )
	{
		// Stencil OFF!
		RwRenderStateSet( rwRENDERSTATESTENCILENABLE, (void *)FALSE );
	} // if //

#endif
//	FinishRendering();
} // HeatHazeFX //

//----------------------------------------------------------------------
// Radiosity
//----------------------------------------------------------------------

void	CPostEffects::Radiosity( Int32 intensityLimit,
								 Int32 filterPasses,
								 Int32 renderPasses,
								 Int32 intensity )
{
	Int32 i;
#ifdef GTA_PS2
	Int32 downsampleSizeX = m_RadiosityPixelsX;
	Int32 downsampleSizeY = m_RadiosityPixelsY;
	Int32 downsampleSizeLastX;
	Int32 downsampleSizeLastY;

	Int32 finalSizeX = downsampleSizeX;
	Int32 finalSizeY = downsampleSizeY;

	Int32 quadSizeX;// = 200;
	Int32 quadSizeY;// = 200;

	Int32 quadPosX;// = 30;
	Int32 quadPosY;// = 120;
	//----------------------------------------------------------------------------
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm, zpsm;
	int32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
	int32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	RwRaster *pSrcRaster;

//	if ( mblur )
//		pSrcRaster = RpSkyGetDisplayBufferRaster();
//	else
		pSrcRaster = pDrawBuffer;

	int32 dwSrcRasterAddr = GetRasterInfo(pSrcRaster, width, psm);
    Int32 numStrips = SCREEN_DRAW_WIDTH/32;

	Int32 numGeneralPackets;// = 20;
	Int32 numFilterPassPackets;// = 4 + 4*filterPasses;
	Int32 numRenderPassPackets;// = 6 + 4*renderPasses;
	Int32 numFinalStripRenderPassPackets=0;

	if( !m_bRadiosityStripCopyMode )
	{
		numGeneralPackets    = 19;
		numFilterPassPackets = 4 + 4*filterPasses;
		numRenderPassPackets = 6 + 4*renderPasses;
	} // if //
	else
	{
		numGeneralPackets              = 15	+ numStrips*4;
		numFilterPassPackets           = 4	+ 4*filterPasses;
		numRenderPassPackets           = 6;	// + 4*renderPasses*numStrips;
		numFinalStripRenderPassPackets = 4*renderPasses*numStrips;
	} // else //

	if(filterPasses==0)
	{
		numFilterPassPackets = 0;
	}
	
	if(renderPasses==0)
	{
		numRenderPassPackets = 0;
	}




	const uint32 numGifQWords	= numGeneralPackets + numFilterPassPackets + numRenderPassPackets;
	const uint32 numDMAQWords	= 1 + numGifQWords;

	if(!_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords))
		return;

	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );
		//----------------------------------------------------------------------------

		ADDTOPKT(SCE_GS_TEST_1, 		SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
		ADDTOPKT(SCE_GS_ZBUF_1, 		skyZbuf_1 | (1L << 32) );
		ADDTOPKT(SCE_GS_XYOFFSET_1, 	SCE_GS_SET_XYOFFSET_1(0, 0));
		ADDTOPKT(SCE_GS_PABE, 			SCE_GS_SET_PABE(0) );
		ADDTOPKT(SCE_GS_COLCLAMP, 		SCE_GS_SET_COLCLAMP(1) );

		/* 
		Original code

		// 1st pass: downsample framebuffer to workbuffer
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwSrcRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width/scale), GETPRIMCOORD(SCREEN_DRAW_HEIGHT/scale), 0) );
		
		// 2nd: remove lower intensity values
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,2,1,128) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(minIntensity, minIntensity, minIntensity, 128, 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width/scale), GETPRIMCOORD(SCREEN_DRAW_HEIGHT/scale), 0) );
		
		// 3rd pass x2: draw on top of framebuffer
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-(scale+1), 0, SCREEN_DRAW_HEIGHT-1));
	//	ADDTOPKT(SCE_GS_ALPHA_1, SET_ALPHA_NOBLEND() );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,strength>>1) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width/scale-1), GETUVCOORD(SCREEN_DRAW_HEIGHT/scale-1)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width-offset*2), GETPRIMCOORD(SCREEN_DRAW_HEIGHT-offset), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(offset),  GETPRIMCOORD(offset), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width/scale-1), GETUVCOORD(SCREEN_DRAW_HEIGHT/scale-1)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width+offset*2), GETPRIMCOORD(SCREEN_DRAW_HEIGHT+offset), 0) );
		
		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
		*/


		// 1) Copy framebuffer to workbuffer
		// ---------------------------------

		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwSrcRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	//	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR,  SCE_GS_LINEAR,  0, 0, 0 ) );
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );

		if( !m_bRadiosityStripCopyMode )
		{
			// Copy screen buffer at once
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );

			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(downsampleSizeX), GETPRIMCOORD(downsampleSizeY), 0) );
		} // if //
		else
		{
			// New! Copy screen buffer in 32 pixel strips
			for( Int32 s = 0; s < numStrips; s++ )
			{
				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(s<<5), GETUVCOORD(0)));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((s+1)<<5), GETUVCOORD(SCREEN_DRAW_HEIGHT)));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));

			//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(s<<5), GETUVCOORD_NEAR(0)));
			//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
			//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((s+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
			//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
			} // for s //
		} // else //

		// 2) Use the bilinear filter to downsample the image as many times as requested
		//    in order to get "color averaged", "big blurry pixeled" version(s) of the
		//    main image...
		// -----------------------------------------------------------------------------

		if( filterPasses > 0 )
		{
	//		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
			ADDTOPKT(SCE_GS_TEXFLUSH, 0);
			ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
			ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
			ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );

			for( i = 0; i < filterPasses; i++ )
			{
				downsampleSizeLastX = downsampleSizeX;
				downsampleSizeLastY = downsampleSizeY;

				downsampleSizeX /= 2; // New X size
				downsampleSizeY /= 2; // New Y size

				// Use a "filter correction value" in order to avoid the shifting of the "image"
				// to the bottom right while using the bilinear filter to downsample it!
	//			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(m_RadiosityFilterUCorrection), GETUVCOORD(m_RadiosityFilterVCorrection)) );
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );

				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(downsampleSizeLastX), GETUVCOORD(downsampleSizeLastY)) );
				// "+1" for x & y is required in order to avoid to get "1/2" pixel garbage
				// at the right and bottom border!
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(downsampleSizeX+1), GETPRIMCOORD(downsampleSizeY+1), 0) );
			} // for i //
		} // if //

		// 3) Remove lower intensity values
		// --------------------------------

		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,2,1,128) );
	//	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,0,1,0) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
	//	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(m_RadiosityCol.red, m_RadiosityCol.green, m_RadiosityCol.blue, m_RadiosityCol.alpha, 0) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(intensityLimit, intensityLimit, intensityLimit, 128, 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(downsampleSizeX+1), GETPRIMCOORD(downsampleSizeY+1), 0) );

		// 4) Render the final image as many times to the framebuffer as requested
		// -----------------------------------------------------------------------

		if( renderPasses > 0 )
		{
			ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
			ADDTOPKT(SCE_GS_TEXFLUSH, 0);
			ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));

			if( m_bRadiosityLinearFilter )
			{
				ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
			} // if //
			else
			{
				ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
			} // else //

			if( m_bRadiosityDebug )
			{
				ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
				ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 255, 0) ); /* Dummy packet! */
			} // if //
			else
			{
				ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,intensity>>1) );
				ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
			} // else //

	//	/*

			quadPosX = 0;
			quadPosY = 0;

			quadSizeX = width;
			quadSizeY = SCREEN_DRAW_HEIGHT;

			finalSizeX = downsampleSizeX;
			finalSizeY = downsampleSizeY;

	//	*/

	/*
			for( i = 0; i < renderPasses; i++ )
			{
				if( !m_bRadiosityStripCopyMode )
				{
					// Copy buffer at once
					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(quadPosX), GETPRIMCOORD(quadPosY), 0) );
					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(finalSizeX), GETUVCOORD(finalSizeY)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(quadPosX+quadSizeX), GETPRIMCOORD(quadPosY+quadSizeY), 0) );
				} // if //
				else
				{
					// New! Copy buffer in 32 pixel strips
					for( Int32 s = 0; s < numStrips; s++ )
					{
						ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(s*(finalSizeX/numStrips)), GETUVCOORD(0)));
						ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
						ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((s+1)*(finalSizeX/numStrips)), GETUVCOORD(finalSizeY)));
						ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));

				 	//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(s<<5), GETUVCOORD_NEAR(0)));
					//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
					//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((s+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
					//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
					} // for s //
				} // else //
			} // for i //
	*/

			if( !m_bRadiosityStripCopyMode )
			{
				for( i = 0; i < renderPasses; i++ )
				{
					// Copy buffer at once
					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(quadPosX), GETPRIMCOORD(quadPosY), 0) );
					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(finalSizeX), GETUVCOORD(finalSizeY)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(quadPosX+quadSizeX), GETPRIMCOORD(quadPosY+quadSizeY), 0) );
				} // for i //
			} // if //

	//		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
		} // if //

		//----------------------------------------------------------------------------
		//LOAD_GIFTAG_QWC();
		ASSERT(_numOfQuadWords == numGifQWords);
	}
	RWDMA_LOCAL_BLOCK_END();
	//----------------------------------------------------------------------------




	// The last copy process (z-buffer to framebuffer) is done here with a new "DMA block"
	if( renderPasses > 0 && m_bRadiosityStripCopyMode )
	{
		ASSERT(numFinalStripRenderPassPackets>0);

		const uint32 numGifQWords	= numFinalStripRenderPassPackets;
		const uint32 numDMAQWords	= 1 + numGifQWords;
	
		if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords) )
			return;

		RWDMA_LOCAL_BLOCK_BEGIN();
		{
			ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );
			//----------------------------------------------------------------------------

			// New! Copy buffer in 32 pixel strips
			for( i = 0; i < renderPasses; i++ )
			{
				for( Int32 s = 0; s < numStrips; s++ )
				{
					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(s*(finalSizeX/numStrips)), GETUVCOORD(0)));
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((s+1)*(finalSizeX/numStrips)), GETUVCOORD(finalSizeY)));
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));

			 	//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(s<<5), GETUVCOORD_NEAR(0)));
				//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(s<<5),GETPRIMCOORD(0), 0));
				//	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((s+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
				//	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((s+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
				} // for s //
			} // for i //

			//----------------------------------------------------------------------------
			//LOAD_GIFTAG_QWC();
			ASSERT(_numOfQuadWords == numGifQWords);
		}
		RWDMA_LOCAL_BLOCK_END();
		//----------------------------------------------------------------------------
	} // if //

#else
	float u, v;
	Int32 downsampleSizeX = m_RadiosityPixelsX;
	Int32 downsampleSizeY = m_RadiosityPixelsY;
	Int32 downsampleSizeLastX;
	Int32 downsampleSizeLastY;

	Int32 finalSizeX = downsampleSizeX;
	Int32 finalSizeY = downsampleSizeY;

	Int32 quadSizeX;// = 200;
	Int32 quadSizeY;// = 200;

	Int32 quadPosX;// = 30;
	Int32 quadPosY;// = 120;

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

	TempBufferVerticesStored = 0;
	if (!m_bRadiosityStripCopyMode)
	{
		RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeX);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeY);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
 	
 		TempBufferVerticesStored+=2;
	}else
	{
		RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), SCREEN_WIDTH);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), SCREEN_HEIGHT);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
 	
 		TempBufferVerticesStored+=2;
	}
	if (TempBufferVerticesStored > 2)
	{
		RwIm2DRenderPrimitive_BUGFIX(rwPRIMTYPETRISTRIP, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored);
	}
	TempBufferVerticesStored = 0;
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	if( filterPasses > 0 )
	{
	    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
	 	for (i = 0; i < filterPasses; i++)
	 	{
			downsampleSizeLastX = downsampleSizeX;
			downsampleSizeLastY = downsampleSizeY;

			downsampleSizeX /= 2; // New X size
			downsampleSizeY /= 2; // New Y size
			
#if 0//def GTA_XBOX
			u = (float) m_RadiosityFilterUCorrection;
			v = (float) m_RadiosityFilterVCorrection;
#else
			u = (float) m_RadiosityFilterUCorrection / (float) RwRasterGetWidth(pRasterFrontBuffer);
			v = (float) m_RadiosityFilterVCorrection / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
		    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), u, 1.0f);
		    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), v, 1.0f);
	  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
			RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
	    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
	    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
	    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));

#if 0//def GTA_XBOX
			u = (float) downsampleSizeLastX;
			v = (float) downsampleSizeLastY;
#else
			u = (float) downsampleSizeLastX / (float) RwRasterGetWidth(pRasterFrontBuffer);
			v = (float) downsampleSizeLastY / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
	 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
		    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
	  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeX+1);
			RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeY+1);
	    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
	    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
	    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
			RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 255, 255, 255, 255);
			RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 255, 255, 255, 255);
	 		TempBufferVerticesStored+=2;
	 	}
	}
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

	RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
	RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
	RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeX+1);
	RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeY+1);
	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
	RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), intensityLimit, intensityLimit, intensityLimit, 128);
	RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), intensityLimit, intensityLimit, intensityLimit, 128);
	TempBufferVerticesStored+=2;

	if (renderPasses > 0)
	{
		if (TempBufferVerticesStored > 2)
		{
			RwIm2DRenderPrimitive_BUGFIX(rwPRIMTYPETRISTRIP, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored);
		}
		TempBufferVerticesStored = 0;
		if( m_bRadiosityLinearFilter )
		{
 		   RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
		}else
		{
 		   RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERMIPNEAREST);
		}

		quadPosX = 0;
		quadPosY = 0;

		quadSizeX = SCREEN_WIDTH;
		quadSizeY = SCREEN_HEIGHT;

		finalSizeX = downsampleSizeX;
		finalSizeY = downsampleSizeY;
		
		if ( !m_bRadiosityStripCopyMode )
		{
			for (i = 0; i < renderPasses; i++)
			{
			
				RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
			    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
		  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]),quadPosX);
				RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), quadPosY);
		    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
		    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
#if 0//def GTA_XBOX
				u = (float) finalSizeX;
				v = (float) finalSizeY;
#else
				u = (float) finalSizeX / (float) RwRasterGetWidth(pRasterFrontBuffer);
				v = (float) finalSizeY / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif

		 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
			    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
		  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), quadPosX+quadSizeX);
				RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), quadPosY+quadSizeY);
		    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
		    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
		    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
				if (m_bRadiosityDebug)
				{
					RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 255, 255, 255, 255);
					RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 255, 255, 255, 255);
		 		}else
		 		{
		 			RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0, 0, 0, intensity);
					RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0, 0, 0, intensity);
		 		}
		 		TempBufferVerticesStored+=2;
			}
		}
	}
	
	if( renderPasses > 0 && m_bRadiosityStripCopyMode )
	{
		{
			for( i = 0; i < renderPasses; i++ )
			{
				RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
			    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
		  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]),0);
				RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
		    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
#if 0//def GTA_XBOX
				u = (float) finalSizeX;
				v = (float) finalSizeY;
#else
				u = (float) finalSizeX / (float) RwRasterGetWidth(pRasterFrontBuffer);
				v = (float) finalSizeY / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
		 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
			    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
		  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), SCREEN_WIDTH);
				RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), SCREEN_HEIGHT);
		    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
		    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
		    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		 		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0, 0, 0, intensity);
				RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0, 0, 0, intensity);
		 		TempBufferVerticesStored+=2;
			}
		}
	}

	if (TempBufferVerticesStored > 2)
	{
		RwIm2DRenderPrimitive_BUGFIX(rwPRIMTYPETRISTRIP, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored);
	}
	TempBufferVerticesStored = 0;
 
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);    
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
#endif
//	FinishRendering();
} // Radiosity //

//----------------------------------------------------------------------
// Darkness filter
//----------------------------------------------------------------------

void	CPostEffects::DarknessFilter( Int32 alpha )
{
#ifdef GTA_PS2
	if(	RwCameraBeginUpdate(Scene.camera) )
#endif	
	{
		ImmediateModeRenderStatesStore();
		ImmediateModeRenderStatesSet();

#ifdef GTA_PS2
		CPostEffects::DrawQuad( 0.0f, 0.0f, SCREEN_DRAW_WIDTH, SCREEN_DRAW_HEIGHT,
								0, 0, 0, alpha,
								NULL );
#else
		CPostEffects::DrawQuad( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT,
								0, 0, 0, alpha,
								NULL );
#endif
		ImmediateModeRenderStatesReStore();

#ifdef GTA_PS2
		// End camera update
		RwCameraEndUpdate(Scene.camera);
#endif		
	} // if //
} // DarknessFilter //




//----------------------------------------------------------------------
// CCTV FX
//----------------------------------------------------------------------
void	CPostEffects::CCTV()
{
//!PC - contains lots of Ps2 only code - argh!
#if defined (GTA_PS2)
		//----------------------------------------------------------------------------

RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
RwInt32 width, psm, zpsm;
int32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
int32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
RwRaster *pSrcRaster;


	//	if ( mblur )
	//		pSrcRaster = RpSkyGetDisplayBufferRaster();
	//	else
			pSrcRaster = pDrawBuffer;

		int32 dwSrcRasterAddr = GetRasterInfo(pSrcRaster, width, psm);


		Int32 lineY      = 0;
		Int32 lineHeight = 2;
		Int32 numLines   = SCREEN_DRAW_HEIGHT / (lineHeight*2);


		const int32 numGeneralPackets  = 8;
		const int32 numFXRenderPackets = 4*numLines;

		const uint32 numGifQWords = numGeneralPackets + numFXRenderPackets;
		const uint32 numDMAQWords = 1 + numGifQWords;

		if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords) )
			return;

		RWDMA_LOCAL_BLOCK_BEGIN();
		{
			ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );
			//----------------------------------------------------------------------------

			ADDTOPKT(SCE_GS_TEST_1, 		SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
			ADDTOPKT(SCE_GS_ZBUF_1, 		skyZbuf_1 | (1L << 32) );
			ADDTOPKT(SCE_GS_XYOFFSET_1, 	SCE_GS_SET_XYOFFSET_1(0, 0));
			ADDTOPKT(SCE_GS_PABE, 			SCE_GS_SET_PABE(0) );
			ADDTOPKT(SCE_GS_COLCLAMP, 		SCE_GS_SET_COLCLAMP(1) );

			// Render the FX
			// -------------

			Int32 intensity = m_CCTVcol.alpha;

	//		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
			ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	//		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));

		//	if( m_bRadiosityLinearFilter )
		//	{
	//			ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		//	} // if //
		//	else
		//	{
		//		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		//	} // else //

		//	if( m_bRadiosityDebug )
		//	{
		//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		//		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 255, 0) ); /* Dummy packet! */
		//	} // if //
		//	else
		//	{
		//		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,intensity>>1) );
		//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );

		//		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
				ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,1,intensity>>1) );
				ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0) );
		//	} // else //

			// Render loop
			for( Int32 i = 0; i < numLines; i++ )
			{
				// Default version
				ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(m_CCTVcol.red, m_CCTVcol.green, m_CCTVcol.blue, 255, 0));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0),
													 GETPRIMCOORD(lineY), 0) );

				ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(m_CCTVcol.red, m_CCTVcol.green, m_CCTVcol.blue, 255, 0));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(SCREEN_DRAW_WIDTH),
												     GETPRIMCOORD(lineY+lineHeight), 0) );

				lineY += lineHeight*2;
			} // if //

			//----------------------------------------------------------------------------
			//LOAD_GIFTAG_QWC();
			ASSERT(_numOfQuadWords == numGifQWords);
		}
		RWDMA_LOCAL_BLOCK_END();
		//----------------------------------------------------------------------------
#else
	#ifdef GTA_PC
		GE_FORCE_5_FIX; // This must be put before the "RwCameraEndUpdate" before the raster copy!
		RwCameraEndUpdate(Scene.camera);
		RwRasterPushContext(pRasterFrontBuffer);
//		RsCameraBeginUpdate(Scene.camera); // ### Do NOT use this for "RwRasterPushContext" and "RwRasterPopContext"!! ###
		RwRasterRenderFast(RwCameraGetRaster(Scene.camera), 0, 0);
//		RwCameraEndUpdate(Scene.camera);
		RwRasterPopContext();
		RsCameraBeginUpdate(Scene.camera);
	#elif defined(GTA_XBOX)
		// im mode pRasterFrontBuffer >>> pRasterFrontBuffer
		RasterCopyPostEffects();
	#endif

	Int32 lineY      = 0;
	Int32 lineHeight = 2 * HUD_MULT_Y;
	Int32 numLines   = SCREEN_HEIGHT / (lineHeight*2);

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

	Int32 intensity = m_CCTVcol.alpha;

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

	CPostEffects::ImmediateModeRenderStatesStore();
	CPostEffects::ImmediateModeRenderStatesSet();

	// Render loop
	for( Int32 i = 0; i < numLines; i++ )
	{
		CPostEffects::DrawQuad( 0.0f, lineY,
								SCREEN_WIDTH,
								lineHeight,
								m_CCTVcol.red, m_CCTVcol.green, m_CCTVcol.blue, 255,
								pRasterFrontBuffer);
		lineY += lineHeight*2;
	} 
	CPostEffects::ImmediateModeRenderStatesReStore();
#endif //GTA_PS2
} // CCTV //

//----------------------------------------------------------------------
// Fog
//----------------------------------------------------------------------

// #########################
// ### THIS IS NOT USED! ###
// #########################

static float gfFogAngle  = 0.0f;
static float gfFogFader  = 0.0f;
static float gfFogRadius = 0.0f;

void	CPostEffects::Fog()
{
	#ifdef GTA_PS2
//		if(	RwCameraBeginUpdate(Scene.camera) )
	#endif
		{
			RwRaster *pRaster = RwTextureGetRaster( CClouds::ms_vc.pTexture[ CClouds::V_CLOUD_TEXTURE_1 ] );

			// Store and set render states
			ImmediateModeRenderStatesStore();
			ImmediateModeRenderStatesSet();

		//	RwRenderStateSet( rwRENDERSTATESRCBLEND,  (void *)rwBLENDONE );
		//	RwRenderStateSet( rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE );

			RwRenderStateSet( rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR );

#if defined (GTA_PS2)
			float cx = (float)(SCREEN_DRAW_WIDTH)/2.0f;  // X center
			float cy = (float)(SCREEN_DRAW_HEIGHT)/2.0f; // Y center
			float sx = (float)(SCREEN_DRAW_WIDTH)/1.5f;  // X size
			float sy = (float)(SCREEN_DRAW_HEIGHT)/1.5f; // Y size
#else //GTA_PS2
			float cx = (float)(SCREEN_WIDTH)/2.0f;  // X center
			float cy = (float)(SCREEN_HEIGHT)/2.0f; // Y center
			float sx = (float)(SCREEN_WIDTH)/1.5f;  // X size
			float sy = (float)(SCREEN_HEIGHT)/1.5f; // Y size
#endif //GTA_PS2

			float fAlpha = 11.0f;
			float fSpeed = FindPlayerSpeed().Magnitude();
			float fFadeSpeed = CTimer::GetTimeStep() / 4.0f;
			float fMaxMoveRadius = 160.0f;

//			if( fSpeed > 0.0f )
//			{
//				gfFogRadius += CTimer::GetTimeStep();
//				if( gfFogRadius > 100.0f ) gfFogRadius = 0.0f;
//			} // if //

			if( fSpeed > 0.06f )
			{
				gfFogRadius += CTimer::GetTimeStep() / 4.0f;
				if( gfFogRadius > fMaxMoveRadius ) gfFogRadius = fMaxMoveRadius;
			} // if //
			else
			{
				gfFogRadius -= CTimer::GetTimeStep() / 4.0f;
				if( gfFogRadius < 0.0f ) gfFogRadius = 0.0f;
			} // else //

/*
			if( fSpeed > 0.06f )
			{
				gfFogFader += fFadeSpeed;
				if( gfFogFader > fAlpha ) gfFogFader = fAlpha;
			} // if //
			else
			{
				gfFogFader -= fFadeSpeed;
				if( gfFogFader < 0.0f ) gfFogFader = 0.0f;
			} // else //

			float fAlphaNew = fAlpha - gfFogFader;
*/
/*
			float fSpeedAlphaFactor = 3.0f;
			float fAlphaNew = ( 1.0f - fSpeed*fSpeedAlphaFactor );
			if( fAlphaNew < 0.0f ) fAlphaNew = 0.0f;
			if( fAlphaNew > 1.0f ) fAlphaNew = 1.0f;
			fAlphaNew *= fAlpha;
*/

#ifndef MASTER

//		 	sprintf( gString, "Speed = %.2f", fSpeed );
//		 	VarConsole.AddDebugOutput( gString );

//		 	sprintf( gString, "Alpha = %.2f", fAlphaNew );
//		 	VarConsole.AddDebugOutput( gString );

#endif

			Int32 alpha = (Int32)fAlpha;//New;

			Int32 r, g, b;

			r = CTimeCycle::GetFogRed();
			g = CTimeCycle::GetFogGreen();
			b = CTimeCycle::GetFogBlue();

			Int32 fxLoop = 10;

			for( Int32 i = 0; i < fxLoop; i++ )
			{
				float radX = cx*0.5f + gfFogRadius;
				float radY = cy*0.5f + gfFogRadius;
				float n = (float)i * 36.0f;
				float x = CMaths::Cos( DEGTORAD( gfFogAngle + n ) )*radX + cx;
				float y = CMaths::Sin( DEGTORAD( gfFogAngle + n ) )*radY + cy;

				CPostEffects::DrawQuad( x-sx*0.5f, y-sy*0.5f,
										/*ms_imf.sizeDrawBufferX*/sx,
										/*ms_imf.sizeDrawBufferY*/sy,
										r, g, b, alpha,
										pRaster );
			} // for i //

			for( Int32 i = 0; i < fxLoop; i++ )
			{
				float radX = cx*0.7f + gfFogRadius;
				float radY = cy*0.7f + gfFogRadius;
				float n = (float)i * 36.0f;
				float x = CMaths::Cos( DEGTORAD( -(gfFogAngle + n) ) )*radX + cx;
				float y = CMaths::Sin( DEGTORAD( -(gfFogAngle + n) ) )*radY + cy;

				CPostEffects::DrawQuad( x-sx*0.5f, y-sy*0.5f,
										/*ms_imf.sizeDrawBufferX*/sx,
										/*ms_imf.sizeDrawBufferY*/sy,
										r, g, b, alpha,
										pRaster );
			} // for i //

			gfFogAngle += CTimer::GetTimeStep() / 6.0f;

			// Restore the render states
			ImmediateModeRenderStatesReStore();

			#ifdef GTA_PS2
					// End camera update
//					RwCameraEndUpdate(Scene.camera);
			#endif
		}
} // Fog //

//----------------------------------------------------------------------
// Speed FX
//----------------------------------------------------------------------

/* Old speed list
	float fxSpeed01 = 0.60f;
	float fxSpeed02 = 0.70f;
	float fxSpeed03 = 0.80f;
	float fxSpeed04 = 0.90f;
	float fxSpeed05 = 0.93f;
	float fxSpeed06 = 0.96f;
	float fxSpeed07 = 1.00f;
*/
/*
	float fxSpeed01 = 0.81f;
	float fxSpeed02 = 0.84f;
	float fxSpeed03 = 0.87f;
	float fxSpeed04 = 0.90f;
	float fxSpeed05 = 0.93f;
	float fxSpeed06 = 0.96f;
	float fxSpeed07 = 1.00f;
*/
#define FX_SPEED_NUM_LEVELS (7)

struct fxSpeedSettings
{
	float fSpeedThreshHold;
	int nLoops;
	int nShift;
	int nShake;
};

fxSpeedSettings FX_SPEED_VARS[FX_SPEED_NUM_LEVELS] = 
{
	{0.60f,	1,	4,	0},
	{0.70f,	2,	4,	0},
	{0.80f,	3,	4,	0},
	{0.90f,	3,	4,	0},
	{0.93f,	4,	4,	1},
	{0.96f,	4,	4,	2},
	{1.00f,	5,	4,	3}
};

bool TEST_SPEEDFX_OLD_VALUES = true;
//
void	CPostEffects::SpeedFX( float speed )
{
	bool  bDrawFXAsStrips = true; // Optimizing option

	// LOOKING_BEHIND, LOOKING_LEFT, LOOKING_RIGHT, LOOKING_FORWARD
	bool bLeftView  = false;
	bool bRightView = false;

	if( TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_LEFT )
	{
		bLeftView = true;
		bDrawFXAsStrips = false; // Optimizing off because filtering looks ugly from side view!
	} // if //

	if( TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_RIGHT )
	{
		bRightView = true;
		bDrawFXAsStrips = false; // Optimizing off because filtering looks ugly from side view!
	} // if //

	//------------------------------------------------------
	// Optimizing stuff
#ifdef GTA_PS2
	Int32 stripWidth = SCREEN_DRAW_WIDTH;
#else
	Int32 stripWidth = SCREEN_WIDTH;
#endif
	Int32 numStrips  = 1;

	if( bDrawFXAsStrips )
	{
		stripWidth = 32; // 1 strip width = 32 pixels
#ifdef GTA_PS2
		numStrips  = SCREEN_DRAW_WIDTH / stripWidth;
#else
		numStrips  = SCREEN_WIDTH / stripWidth;
#endif
	} // if //
	//------------------------------------------------------

	RwRaster* pDrawBuffer = ms_imf.pRasterDrawBuffer;

	Int32 fxLoop  = 0; // FX off as default
	Int32 shift = 0;//m_SpeedFXShift;
	Int32 speedFXShake = 0;

	// The speeds which determine when a new FX intensity kicks in

	// Set the FX parameters depending on the current speed
	
/*	// Old version
	if( speed >= fxSpeed01 ) { fxLoop =  2; shift = 2; }
	if( speed >= fxSpeed02 ) { fxLoop =  4; shift = 2; }
	if( speed >= fxSpeed03 ) { fxLoop =  6; shift = 2; }
	if( speed >= fxSpeed04 ) { fxLoop =  7; shift = 2; }
	if( speed >= fxSpeed05 ) { fxLoop =  8; shift = 2; speedFXShake = shift; }
	if( speed >= fxSpeed06 ) { fxLoop =  9; shift = 2; speedFXShake = shift; }
	if( speed >= fxSpeed07 ) { fxLoop = 10; shift = 2; speedFXShake = shift; }
*/
/*
	if( speed >= fxSpeed01 ) { fxLoop =  1; shift = 4; }
	if( speed >= fxSpeed02 ) { fxLoop =  2; shift = 4; }
	if( speed >= fxSpeed03 ) { fxLoop =  3; shift = 4; }
	if( speed >= fxSpeed04 ) { fxLoop =  3; shift = 4; }
	if( speed >= fxSpeed05 ) { fxLoop =  4; shift = 4; speedFXShake = 1;}//shift-2; }
	if( speed >= fxSpeed06 ) { fxLoop =  4; shift = 4; speedFXShake = 2;}//shift-1; }
	if( speed >= fxSpeed07 ) { fxLoop =  5; shift = 4; speedFXShake = 3;}//shift; }
*/

	//

	// ### This is a debug option. Enable it and go into a vehicle and the                     ###
	// ### speed FX turns on at full intensity without the need of driving the vehicle itself! ###

//	speed = 0.8f;

	//

	for(int i=FX_SPEED_NUM_LEVELS-1; i >= 0; i--)
	{
		if(speed >= FX_SPEED_VARS[i].fSpeedThreshHold)
		{
			fxLoop = FX_SPEED_VARS[i].nLoops;
			shift = FX_SPEED_VARS[i].nShift;
			speedFXShake = FX_SPEED_VARS[i].nShake;
			break;
		}
	}

	if(	bLeftView || bRightView )
	{
		shift /= 2; // Half the "shift" only for side view!
		speedFXShake = 0; // No shake for side view!
	} // if //

	// Render the FX if used
	if( fxLoop > 0 )
	{
	#ifdef GTA_PS2
		//----------------------------------------------------------------------------
		RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
		RwInt32 width, psm, zpsm;
		int32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
		int32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
		RwRaster *pSrcRaster;

	//	if ( mblur )
	//		pSrcRaster = RpSkyGetDisplayBufferRaster();
	//	else
			pSrcRaster = pDrawBuffer;

		int32 dwSrcRasterAddr = GetRasterInfo(pSrcRaster, width, psm);

		const int32 numGeneralPackets  = 14;
		const int32 numFXRenderPackets = 6 + 4*fxLoop*numStrips;


		const uint32 numGifQWords = numGeneralPackets + numFXRenderPackets;
		const uint32 numDMAQWords = 1 + numGifQWords;

		if(!_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords))
		{
			// ##### TEMPORARY DEBUG CODE! #####
#ifndef MASTER
			if( m_bSpeedFX_DMAPacketErrorMessage_PS2 )
			{
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );
			 	sprintf( gString, "-" ); VarConsole.AddDebugOutput( gString );

			 	sprintf( gString, "Warning! \"_rwDMAOpenVIFPkt\" failed for speed FX!" );
				VarConsole.AddDebugOutput( gString );
			} // if //
#endif
			return;
		} // if //


		RWDMA_LOCAL_BLOCK_BEGIN();
		{
			ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );
			//----------------------------------------------------------------------------

			ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
			ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
			ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
			ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
			ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1) );

			// 1) Copy framebuffer to workbuffer
			// ---------------------------------

			Int32 downsampleSizeX = SCREEN_DRAW_WIDTH;
			Int32 downsampleSizeY = SCREEN_DRAW_HEIGHT;

			ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
			ADDTOPKT(SCE_GS_TEXFLUSH, 0);
			ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwSrcRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		//	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR,  SCE_GS_LINEAR,  0, 0, 0 ) );
			ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
			ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
			//
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );

			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
			// "+1" for x & y is required in order to avoid the "half pixel" garbage
			// at the right and bottom border!
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(downsampleSizeX+1), GETPRIMCOORD(downsampleSizeY+1), 0) );

			// 2) Render the FX
			// ----------------

			Int32 intensity = m_SpeedFXAlpha;

			ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
			ADDTOPKT(SCE_GS_TEXFLUSH, 0);
			ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));

		//	if( m_bRadiosityLinearFilter )
		//	{
				ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		//	} // if //
		//	else
		//	{
		//		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		//	} // else //

		//	if( m_bRadiosityDebug )
		//	{
		//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		//		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 255, 0) ); /* Dummy packet! */
		//	} // if //
		//	else
		//	{
		//		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,intensity>>1) );
		//		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );

		//		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
				ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,1,intensity>>1) );
				ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		//	} // else //

			Int32 scanSizeX = SCREEN_DRAW_WIDTH;
			Int32 scanSizeY = SCREEN_DRAW_HEIGHT;

			Int32 renderSizeX = SCREEN_DRAW_WIDTH;
			Int32 renderSizeY = SCREEN_DRAW_HEIGHT;

			Int32 u1Shift = shift;
			Int32 v1Shift = shift;
			Int32 u2Shift = shift;
			Int32 v2Shift = shift;

			// Render loop
			for( Int32 i = 0; i < fxLoop; i++ )
			{
				Int32 scanX   = 0;
				Int32 scanY   = 0;
				Int32 renderX = 0;
				Int32 renderY = 0;

				Int32 u1Rand = 0;
				Int32 v1Rand = 0;
				Int32 u2Rand = 0;
				Int32 v2Rand = 0;

				if( speedFXShake > 0 )
				{
					// Do a bit some "shaking"
					Int32 shakeRange = speedFXShake;

					u1Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
					v1Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
					u2Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
					v2Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
				} // if //

				// Change stuff a bit if we view the vehicle from the side
				if( bLeftView )
				{
					// No FX for...

					// Left
					u1Shift = 0;
					u1Rand  = 0;

					// Right  (use shift FX but no shake!)
				//	u2Shift = 0;
					u2Rand  = 0;

					// Top
					v1Shift = 0;
					v1Rand  = 0;

					// Bottom
					v2Shift = 0;
					v2Rand  = 0;

					// What remains is a bit of stretched blur on the RIGHT
					// side of the screen (which is behind the vehicle/player)...
				} // if //

				if( bRightView )
				{
					// No FX for...

					// Left (use shift FX but no shake!)
				//	u1Shift = 0;
					u1Rand  = 0;

					// Right side
					u2Shift = 0;
					u2Rand  = 0;

					// Top
					v1Shift = 0;
					v1Rand  = 0;

					// Bottom
					v2Shift = 0;
					v2Rand  = 0;

					// What remains is a bit of stretched blur on the LEFT
					// side of the screen (which is behind the vehicle/player)...
				} // if //

				// ### This code draws the FX as fullscreen quads ###
				if( !bDrawFXAsStrips )
				{
					ASSERT(numStrips==1);
					// Default version
					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+u1Shift+u1Rand),
													  GETUVCOORD(scanY+v1Shift+v1Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX),
														 GETPRIMCOORD(renderY), 0) );

					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+scanSizeX-u2Shift+u2Rand),
													  GETUVCOORD(scanY+scanSizeY-v2Shift+v2Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX+renderSizeX),
													     GETPRIMCOORD(renderY+renderSizeY), 0) );
				} // if //
				else
				{
					// Optimized version with strips
					Int32 x = 0;

					scanSizeX   = stripWidth;
					renderSizeX = stripWidth;

					for( Int32 s = 0; s < numStrips; s++ )
					{
						scanX   = x;
						renderX = x;

	if( s == 0 ) // The LEFT border strip
	{
					Int32 u1r = u1Rand; if( u1r < 0 ) u1r = 0; // Make sure not to go out of L border!
					Int32 u2r = u2Rand;

					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+u1r/*+u1Shift+u1Rand*/),
													  GETUVCOORD(scanY+v1Shift+v1Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX),
														 GETPRIMCOORD(renderY), 0) );

					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+scanSizeX+u2r/*-u2Shift+u2Rand*/),
													  GETUVCOORD(scanY+scanSizeY-v2Shift+v2Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX+renderSizeX),
													     GETPRIMCOORD(renderY+renderSizeY), 0) );
	} // if //
	else
	if( s == ( numStrips - 1 ) ) // The RIGHT border strip
	{
					Int32 u1r = u1Rand;
					Int32 u2r = u2Rand; if( u2r > 0 ) u2r = 0; // Make sure not to go out of R border!

					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+u1r/*+u1Shift+u1Rand*/),
													  GETUVCOORD(scanY+v1Shift+v1Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX),
														 GETPRIMCOORD(renderY), 0) );

					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+scanSizeX+u2r/*-u2Shift+u2Rand*/),
													  GETUVCOORD(scanY+scanSizeY-v2Shift+v2Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX+renderSizeX),
													     GETPRIMCOORD(renderY+renderSizeY), 0) );
	} // if //
	else
	{
					Int32 u1s = 0;
					Int32 u2s = 0;

				//	if( bLeftView || bRightView )
				//	{
				//		u1s = u1Shift;
				//		u2s = u2Shift;
				//	}

					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+u1s/*u1Shift*/+u1Rand),
													  GETUVCOORD(scanY+v1Shift+v1Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX),
														 GETPRIMCOORD(renderY), 0) );

					ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(scanX+scanSizeX-u2s/*u2Shift*/+u2Rand),
													  GETUVCOORD(scanY+scanSizeY-v2Shift+v2Rand)) );
					ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(renderX+renderSizeX),
													     GETPRIMCOORD(renderY+renderSizeY), 0) );
	} // else //
						x += stripWidth;
					} // for s //
				} // else //

				u1Shift += shift;
				v1Shift += shift;
				u2Shift += shift;
				v2Shift += shift;
			} // for i //

			//----------------------------------------------------------------------------
			//LOAD_GIFTAG_QWC();
			ASSERT(_numOfQuadWords == numGifQWords);
		}
		RWDMA_LOCAL_BLOCK_END();
		//----------------------------------------------------------------------------

	#else // For "#ifdef GTA_PS2"

		// "PC" & "XBOX" speed FX
		// ----------------------

//--------------
//goto Skip_rest;
//--------------

/*
#if defined (GTA_PC) || defined (GTA_XBOX)

#endif
*/

	/*

		sprintf( gString, "Speed FX..." );
		VarConsole.AddDebugOutput( gString );

		sprintf( gString, "Raster sx = %d, sy = %d", RwRasterGetWidth(pRasterFrontBuffer),
													 RwRasterGetHeight(pRasterFrontBuffer) );
		VarConsole.AddDebugOutput( gString );

		sprintf( gString, "Texture u1 = %.2f, v1 = %.2f", ms_imf.fFrontBufferU1,
													      ms_imf.fFrontBufferV1 );
		VarConsole.AddDebugOutput( gString );

		sprintf( gString, "Texture u2 = %.2f, v2 = %.2f", ms_imf.fFrontBufferU2,
													      ms_imf.fFrontBufferV2 );
		VarConsole.AddDebugOutput( gString );

//	*/

		// Store and set render states
		ImmediateModeRenderStatesStore();
		ImmediateModeRenderStatesSet();

		// ##################################################################################################
		// ### This is a temporary fix for GeForce 5 graphic cards which do not "behave" how they should! ###
		// ##################################################################################################

		// This is NOT used anymore!!!
//		RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSWRAP );

		//

//		RwRenderStateSet( rwRENDERSTATETEXTUREFILTER,  (void *)rwFILTERLINEAR );
//		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
// 		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
//		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
// 		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);

		float fU1Rand = 0.0f;
		float fV1Rand = 0.0f;
		float fU2Rand = 0.0f;
		float fV2Rand = 0.0f;

		if( speedFXShake > 0 )
		{
			// Do a bit some "shaking"
		//	Int32 shakeRange = (float)( speedFXShake );

			float fShakeTweak  = 250.0f; //250.0f;
			float fShakeRange  = (float)( speedFXShake ) / fShakeTweak;
			float fShakeRangeU = fShakeRange * ms_imf.fFrontBufferU2;
			float fShakeRangeV = fShakeRange * ms_imf.fFrontBufferV2;

		//	fU1Rand = (float)( CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange ) ) / (float)( SCREEN_WIDTH );
		//	fV1Rand = (float)( CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange ) ) / (float)( SCREEN_HEIGHT );
			fU1Rand = CGeneral::GetRandomNumberInRange( 0.0f, fShakeRangeU );
			fV1Rand = CGeneral::GetRandomNumberInRange( 0.0f, fShakeRangeV );
			fU2Rand = fU1Rand;
			fV2Rand = fV1Rand;

//sprintf( gString, "Shake FX... %d", speedFXShake );
//VarConsole.AddDebugOutput( gString );
		} // if //

//		float fShiftU = (float)( shift ) / (float)( SCREEN_WIDTH );
//		float fShiftV = (float)( shift ) / (float)( SCREEN_HEIGHT );
		float fShiftTweak = 400.0f; //400.0f;
		float fShiftU = (float)( shift ) * ms_imf.fFrontBufferU2 / fShiftTweak;
		float fShiftV = (float)( shift ) * ms_imf.fFrontBufferV2 / fShiftTweak;

//		float fU1Shift = fShiftU;
//		float fV1Shift = fShiftV;
//		float fU2Shift = fShiftU;
//		float fV2Shift = fShiftV;

		float fv1uShift =  fShiftU;
		float fv1vShift =  fShiftV;
		float fv2uShift = -fShiftU;
		float fv2vShift =  fShiftV;
		float fv3uShift = -fShiftU;
		float fv3vShift = -fShiftV;
		float fv4uShift =  fShiftU;
		float fv4vShift = -fShiftV;

		for( Int32 i = 0; i < fxLoop; i++ )
		{
				// Change stuff a bit if we view the vehicle from the side
				if( bLeftView )
				{
					// No shake FX!
					fU1Rand = 0.0f;
					fU2Rand = 0.0f;
					fV1Rand = 0.0f;
					fV2Rand = 0.0f;

					// No FX for the following texture coordinates...
				//	fv1uShift = 0.0f;
				  	fv1vShift = 0.0f;
					fv2uShift = 0.0f;
				  	fv2vShift = 0.0f;
					fv3uShift = 0.0f;
				  	fv3vShift = 0.0f;
				//	fv4uShift = 0.0f;
				  	fv4vShift = 0.0f;

					// What remains is a blur on the RIGHT
					// side of the screen (which is behind the vehicle/player)...
				} // if //

				if( bRightView )
				{
					// No shake FX!
					fU1Rand = 0.0f;
					fU2Rand = 0.0f;
					fV1Rand = 0.0f;
					fV2Rand = 0.0f;

					// No FX for the following texture coordinates...
					fv1uShift = 0.0f;
				  	fv1vShift = 0.0f;
				//	fv2uShift = 0.0f;
				  	fv2vShift = 0.0f;
				//	fv3uShift = 0.0f;
				  	fv3vShift = 0.0f;
					fv4uShift = 0.0f;
				  	fv4vShift = 0.0f;

					// What remains is a blur on the LEFT
					// side of the screen (which is behind the vehicle/player)...
				} // if //

//			DrawQuadSetUVs( ms_imf.fFrontBufferU1 + fU1Shift + fU1Rand, ms_imf.fFrontBufferV1 + fV1Shift + fV1Rand, /* Top Left     */
//							ms_imf.fFrontBufferU2 - fU2Shift - fU2Rand, ms_imf.fFrontBufferV1 + fV1Shift + fV1Rand, /* Top Right    */
//							ms_imf.fFrontBufferU2 - fU2Shift - fU2Rand, ms_imf.fFrontBufferV2 - fV2Shift - fV2Rand, /* Bottom Right */
//							ms_imf.fFrontBufferU1 + fU1Shift + fU1Rand, ms_imf.fFrontBufferV2 - fV2Shift - fU2Rand  /* Bottom Left  */ );

			DrawQuadSetUVs( ms_imf.fFrontBufferU1 + fv1uShift + fU1Rand, ms_imf.fFrontBufferV1 + fv1vShift + fV1Rand, /* Top Left     */
							ms_imf.fFrontBufferU2 + fv2uShift - fU2Rand, ms_imf.fFrontBufferV1 + fv2vShift + fV1Rand, /* Top Right    */
							ms_imf.fFrontBufferU2 + fv3uShift - fU2Rand, ms_imf.fFrontBufferV2 + fv3vShift - fV2Rand, /* Bottom Right */
							ms_imf.fFrontBufferU1 + fv4uShift + fU1Rand, ms_imf.fFrontBufferV2 + fv4vShift - fU2Rand  /* Bottom Left  */ );
/*
			DrawQuadSetUVs( ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV1,
							ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV1,
							ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV2,
							ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV2 );
*/
/*
		sprintf( gString, "UV1-%d: %.2f, %.2f (%d)", i, ms_imf.fFrontBufferU1 + fv1uShift + fU1Rand, ms_imf.fFrontBufferV1 + fv1vShift + fV1Rand, pRasterFrontBuffer );
		VarConsole.AddDebugOutput( gString );
		sprintf( gString, "UV2-%d: %.2f, %.2f (%d)", i, ms_imf.fFrontBufferU2 + fv2uShift - fU2Rand, ms_imf.fFrontBufferV1 + fv2vShift + fV1Rand, pRasterFrontBuffer );
		VarConsole.AddDebugOutput( gString );
		sprintf( gString, "UV3-%d: %.2f, %.2f (%d)", i, ms_imf.fFrontBufferU2 + fv3uShift - fU2Rand, ms_imf.fFrontBufferV2 + fv3vShift - fV2Rand, pRasterFrontBuffer );
		VarConsole.AddDebugOutput( gString );
		sprintf( gString, "UV4-%d: %.2f, %.2f (%d)", i, ms_imf.fFrontBufferU1 + fv4uShift + fU1Rand, ms_imf.fFrontBufferV2 + fv4vShift - fU2Rand, pRasterFrontBuffer );
		VarConsole.AddDebugOutput( gString );
*/

			DrawQuad( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT,
					  255, 255, 255, m_SpeedFXAlpha,
					  /*RwTextureGetRaster( CClouds::ms_vc.pTexture[ CClouds::V_CLOUD_TEXTURE_1 ] ) );*/
					  pRasterFrontBuffer );

//			fU1Shift += fShiftU;
//			fV1Shift += fShiftV;
//			fU2Shift += fShiftU;
//			fV2Shift += fShiftV;

			fv1uShift +=  fShiftU;
			fv1vShift +=  fShiftV;
			fv2uShift += -fShiftU;
			fv2vShift +=  fShiftV;
			fv3uShift += -fShiftU;
			fv3vShift += -fShiftV;
			fv4uShift +=  fShiftU;
			fv4vShift += -fShiftV;
		} // for i //

		DrawQuadSetDefaultUVs(); // Reset UV's!

		// Brighten a bit up to compensate the "alpha darkening"
	/*

		Int32 col = 40;//20 * fxLoop;

		RwRenderStateSet(rwRENDERSTATESRCBLEND,  (void *)rwBLENDONE);
 		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);

		DrawQuadSetUVs( ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV1,
						ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV1,
						ms_imf.fFrontBufferU2, ms_imf.fFrontBufferV2,
						ms_imf.fFrontBufferU1, ms_imf.fFrontBufferV2 );

		DrawQuad( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT,
				  col, col, col, 255,
				  pRasterFrontBuffer );

		DrawQuadSetDefaultUVs(); // Reset UV's!

//	*/

		// Restore the render states
		ImmediateModeRenderStatesReStore();

//-------------------------------------
// ### The code below is not used! ###
#if 0
//goto Skip_rest;
//-------------------------------------

		//

		bDrawFXAsStrips = false;

		Int32 downsampleSizeX = SCREEN_WIDTH;
		Int32 downsampleSizeY = SCREEN_HEIGHT;
		float u, v; 

		RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSWRAP );
		
	    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERMIPNEAREST);
	    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
	    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
	    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
	
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

		TempBufferVerticesStored = 0;
		
		RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeX+1);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), downsampleSizeY+1);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
 	
 		TempBufferVerticesStored+=2;
		Int32 intensity = m_SpeedFXAlpha;

		if (TempBufferVerticesStored > 2)
		{
			RwIm2DRenderPrimitive_BUGFIX(rwPRIMTYPETRISTRIP, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored);
		}
		TempBufferVerticesStored = 0;

		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

		Int32 scanSizeX = SCREEN_WIDTH;
		Int32 scanSizeY = SCREEN_HEIGHT;

		Int32 renderSizeX = SCREEN_WIDTH;
		Int32 renderSizeY = SCREEN_HEIGHT;

		Int32 u1Shift = shift;
		Int32 v1Shift = shift;
		Int32 u2Shift = shift;
		Int32 v2Shift = shift;

		for( Int32 i = 0; i < fxLoop; i++ )
		{
			Int32 scanX   = 0;
			Int32 scanY   = 0;
			Int32 renderX = 0;
			Int32 renderY = 0;

			Int32 u1Rand = 0;
			Int32 v1Rand = 0;
			Int32 u2Rand = 0;
			Int32 v2Rand = 0;

			if( speedFXShake > 0 )
			{
				// Do a bit some "shaking"
				Int32 shakeRange = speedFXShake;

				u1Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
				v1Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
				u2Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
				v2Rand = CGeneral::GetRandomNumberInRange( -shakeRange, shakeRange );
			} // if //

			// Change stuff a bit if we view the vehicle from the side
			if( bLeftView )
			{
				// No FX for...

				// Left
				u1Shift = 0;
				u1Rand  = 0;

				// Right  (use shift FX but no shake!)
			//	u2Shift = 0;
				u2Rand  = 0;

				// Top
				v1Shift = 0;
				v1Rand  = 0;

				// Bottom
				v2Shift = 0;
				v2Rand  = 0;

				// What remains is a bit of stretched blur on the RIGHT
				// side of the screen (which is behind the vehicle/player)...
			} // if //

			if( bRightView )
			{
				// No FX for...

				// Left (use shift FX but no shake!)
			//	u1Shift = 0;
				u1Rand  = 0;

				// Right side
				u2Shift = 0;
				u2Rand  = 0;

				// Top
				v1Shift = 0;
				v1Rand  = 0;

				// Bottom
				v2Shift = 0;
				v2Rand  = 0;

				// What remains is a bit of stretched blur on the LEFT
				// side of the screen (which is behind the vehicle/player)...
			} // if //

			// ### This code draws the FX as fullscreen quads ###
			if( !bDrawFXAsStrips )
			{
#if 0//def GTA_XBOX
				u = (float) (scanX+u1Shift+u1Rand);
				v = (float) (scanY+v1Shift+v1Rand);
#else
				u = (float) (scanX+u1Shift+u1Rand) / (float) RwRasterGetWidth(pRasterFrontBuffer);
				v = (float) (scanY+v1Shift+v1Rand) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
			    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), u, 1.0f);
			    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), v, 1.0f);
		  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderX);
				RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderY);
		    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
		    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
#if 0//def GTA_XBOX
				u = (float) (scanX+scanSizeX-u2Shift+u2Rand);
				v = (float) (scanY+scanSizeY-v2Shift+v2Rand);
#else
				u = (float) (scanX+scanSizeX-u2Shift+u2Rand) / (float) RwRasterGetWidth(pRasterFrontBuffer);
				v = (float) (scanY+scanSizeY-v2Shift+v2Rand) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
		 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
			    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
		  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderX+renderSizeX);
				RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderY+renderSizeY);
		    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
		    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
		    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
				RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 255, 255, 255, intensity);
				RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 255, 255, 255, intensity);
		 		TempBufferVerticesStored+=2;
			}else
			{
				Int32 x = 0;

				scanSizeX   = stripWidth;
				renderSizeX = stripWidth;

				for( Int32 s = 0; s < numStrips; s++ )
				{
					scanX   = x;
					renderX = x;
					if( s == 0 || s == ( numStrips - 1 ) ) /* This only for the 1st and last (left and right) strip! */
					{
#if 0//def GTA_XBOX
						u = (float) (scanX+u1Shift+u1Rand);
						v = (float) (scanY+v1Shift+v1Rand);
#else
						u = (float) (scanX+u1Shift+u1Rand) / (float) RwRasterGetWidth(pRasterFrontBuffer);
						v = (float) (scanY+v1Shift+v1Rand) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
					    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), u, 1.0f);
					    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), v, 1.0f);
				  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderX);
						RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderY);
				    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
				    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
				    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
#if 0//def GTA_XBOX
						u = (float) (scanX+scanSizeX-u2Shift+u2Rand);
						v = (float) (scanY+scanSizeY-v2Shift+v2Rand);
#else
						u = (float) (scanX+scanSizeX-u2Shift+u2Rand) / (float) RwRasterGetWidth(pRasterFrontBuffer);
						v = (float) (scanY+scanSizeY-v2Shift+v2Rand) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
				 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
					    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
				  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderX+renderSizeX);
						RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderY+renderSizeY);
				    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
				    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
				    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
						RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 255, 255, 255, intensity);
						RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 255, 255, 255, intensity);
				 		TempBufferVerticesStored+=2;
					}else
					{
						float u1s = 0.0f;
						float u2s = 0.0f;
#if 0//def GTA_XBOX
						u = (float) (scanX+u1s+u1Rand);
						v = (float) (scanY+v1Shift+v1Rand);
#else
						u = (float) (scanX+u1s+u1Rand) / (float) RwRasterGetWidth(pRasterFrontBuffer);
						v = (float) (scanY+v1Shift+v1Rand) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif

					    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), u, 1.0f);
					    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), v, 1.0f);
				  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderX);
						RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), renderY);
				    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
				    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
				    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
						
#if 0//def GTA_XBOX
						u = (float) (scanX+scanSizeX-u2s+u2Rand);
						v = (float) (scanY+scanSizeY-v2Shift+v2Rand);
#else
						u = (float) (scanX+scanSizeX-u2s+u2Rand) / (float) RwRasterGetWidth(pRasterFrontBuffer);
						v = (float) (scanY+scanSizeY-v2Shift+v2Rand) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
				 	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
					    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
				  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderX+renderSizeX);
						RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), renderY+renderSizeY);
				    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 0);
				    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
				    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
						RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 255, 255, 255, intensity);
						RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 255, 255, 255, intensity);
				 		TempBufferVerticesStored+=2;
					}
					x += stripWidth;
				}
			}
			u1Shift += shift;
			v1Shift += shift;
			u2Shift += shift;
			v2Shift += shift;
		}
/*
		if (TempBufferVerticesStored > 2)
		{
			RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored);
		}
*/
	    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
	    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
	    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);    
	    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
	    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
	#endif
	//	FinishRendering();

//--------------
#endif
//Skip_rest:;
//--------------

	} // if //
} // SpeedFX //

static float fDayNightBalanceParamOld;

void	CPostEffects::FilterFX_StoreAndSetDayNightBalance()
{
	if( m_bInCutscene ) return;

	fDayNightBalanceParamOld = cbrGetDayNightBalanceParam();
	cbrSetDayNightBalanceParam( m_VisionFXDayNightBalance );
} // FilterFX_StoreAndSetDayNightBalance //

void	CPostEffects::FilterFX_RestoreDayNightBalance()
{
	if( m_bInCutscene ) return;

	cbrSetDayNightBalanceParam( fDayNightBalanceParamOld );
} // FilterFX_RestoreDayNightBalance //


bool	CPostEffects::IsVisionFXActive()
{
	if( m_bInCutscene ) return false;

	if( m_bNightVision || m_bInfraredVision ) return true;

	return false;
} // IsVisionFXActive //

//

//
//
//
//
void	CPostEffects::SetFilterMainColour( RwRaster* pRaster, RwRGBA colour )
{
#ifdef GTA_PS2
	RwInt32 width, psm;
	uint32 dwRasterAddr = GetRasterInfo(pRaster, width, psm);


	const uint32 numGifQWords = 12;
	const uint32 numDMAQWords = 1 + numGifQWords;

	if(!_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords))
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );
		
		
		ADDTOPKT(SCE_GS_TEST_1, 	SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
		ADDTOPKT(SCE_GS_ZBUF_1, 	skyZbuf_1 | (1L << 32) );
		ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
		ADDTOPKT(SCE_GS_PABE, 		SCE_GS_SET_PABE(0) );
		
		// 1st pass: brighten framebuffer
		/*
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 10, 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
		*/
		// 2nd pass: modulate framebuffer
		ADDTOPKT(SCE_GS_ALPHA_1, 	SCE_GS_SET_ALPHA(0,2,0,1,0) );
		ADDTOPKT(SCE_GS_TEX0_1, 	SCE_GS_SET_TEX0(dwRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 1));
		ADDTOPKT(SCE_GS_PRIM, 		SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );

		ADDTOPKT(SCE_GS_RGBAQ, 		SCE_GS_SET_RGBAQ(colour.red, colour.green, colour.blue, colour.alpha, 0) );
	//	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(         0,          255,           0,          255, 0) );

	//	if ( colour.alpha > 0 )
	//		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(colour.alpha>>2)) );
	//	else
			ADDTOPKT(SCE_GS_UV, 	SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_XYZ2, 		SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	//	if ( colour.alpha > 0 )
	//		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)-(colour.alpha>>4)) );
	//	else
			ADDTOPKT(SCE_GS_UV, 	SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
		ADDTOPKT(SCE_GS_XYZ2, 		SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );

	/*
		if ( colour.alpha > 0 )
		{
			// 3rd pass: whiten screen (night vision switching on)
			ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
			ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,colour.alpha>>1) );
			ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 0, 0) );
	//		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(255-colour.alpha), 0) );
	//		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT) - ((255-colour.alpha) << 4), 0) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
		}
	*/
		//LOAD_GIFTAG_QWC();
		ASSERT(_numOfQuadWords == numGifQWords);
	}
	RWDMA_LOCAL_BLOCK_END();

#else

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

    RwIm2DVertexSetIntRGBA(&pe_vertex[0], colour.red, colour.green, colour.blue, colour.alpha);
    RwIm2DVertexSetIntRGBA(&pe_vertex[1], colour.red, colour.green, colour.blue, colour.alpha);
    RwIm2DVertexSetIntRGBA(&pe_vertex[2], colour.red, colour.green, colour.blue, colour.alpha);
    RwIm2DVertexSetIntRGBA(&pe_vertex[3], colour.red, colour.green, colour.blue, colour.alpha);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDDESTALPHA);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, pe_vertex, 4, pe_index, 6);

    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);    
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

#endif
}

#ifdef GTA_PS2
void	CPostEffects::FilterFX_SetConstantParametersForTimeCycle()
{
	
}

//

#define SET_ALPHA_NOBLEND()		SCE_GS_SET_ALPHA(0,2,2,2,128)
#define SET_ALPHA_MODULATE()	SCE_GS_SET_ALPHA(0,1,0,1,0)
#define SET_ALPHA_ADDITIVE()	SCE_GS_SET_ALPHA(0,2,0,1,0)
#define SET_ALPHA_MODULATE_FIXALPHA(a)	SCE_GS_SET_ALPHA(0,1,2,1,a)
#define SET_ALPHA_ADDITIVE_FIXALPHA(a)	SCE_GS_SET_ALPHA(0,2,2,1,a)
#endif

//
//
//
//
void	CPostEffects::ColourFilter( RwRGBA col1, RwRGBA col2 )
{
#ifdef GTA_PS2
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	
	int32 width, psm, zpsm;
	uint32 dwRasterAddr 		= GetRasterInfo(pDrawBuffer, width, psm);
	uint32 dwZBufRasterAddr 	= GetZBufferRasterInfo(zpsm);

	

	const uint32 numGifQWords = 22 + ((width>>5)*4) + DRAW_SCREEN_IN_32_PIXEL_STRIPS_NUM_QWORDS*2;
	const uint32 numDMAQWords = 1 + numGifQWords;
	
	if (!_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords))
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1));
		
		
		ADDTOPKT(SCE_GS_TEST_1, 	SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
		ADDTOPKT(SCE_GS_ZBUF_1, 	skyZbuf_1 | (1L << 32) );
		ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
		ADDTOPKT(SCE_GS_PABE, 		SCE_GS_SET_PABE(0) );
		ADDTOPKT(SCE_GS_SCISSOR_1, 	SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
		ADDTOPKT(SCE_GS_COLCLAMP, 	SCE_GS_SET_COLCLAMP(1) );
		ADDTOPKT(SCE_GS_CLAMP, 		SCE_GS_SET_CLAMP(2,2, 0, width-1, 0, SCREEN_DRAW_HEIGHT-1));

		// 1st pass copy to zbuffer 
		ADDTOPKT(SCE_GS_FRAME_1, 	SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,zpsm,0xff000000) );
		ADDTOPKT(SCE_GS_TEX0_1, 	SCE_GS_SET_TEX0( dwRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, 	SCE_GS_SET_TEX1( 0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_ALPHA_1, 	SCE_GS_SET_ALPHA(0,2,2,2,128) );

		DrawScreenIn32PixelStrips(127,127,127,0);
		
		/*	
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(127, 127, 127, 0, 0) );
		// Draw screen buffer in 32 pixel strips
		for (RwInt32 i=0;i<(width>>5);i++)
		{
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(i<<5), GETUVCOORD_NEAR(0)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((i+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
		}
		*/
		
		// 2nd pass copy back to framebuffer with colour applied to zbuffer
		ADDTOPKT(SCE_GS_FRAME_1, 	SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0x00000000) );
		ADDTOPKT(SCE_GS_TEX0_1, 	SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, zpsm, 10, 10, 0, SCE_GS_MODULATE, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, 	SCE_GS_SET_TEX1( 0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_ALPHA_1, 	SCE_GS_SET_ALPHA(0,2,2,2,128) );

		DrawScreenIn32PixelStrips(col1.red, col1.green, col1.blue, 0);
		
		/*	
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(col1.red, col1.green, col1.blue, 0, 0) );

		// Draw screen buffer in 32 pixel strips
		for (RwInt32 i=0;i<(width>>5);i++)
		{
			ADDTOPKT(SCE_GS_UV, 	SCE_GS_SET_UV(GETUVCOORD_NEAR(i<<5), GETUVCOORD_NEAR(0)));
			ADDTOPKT(SCE_GS_XYZ2, 	SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));
			ADDTOPKT(SCE_GS_UV, 	SCE_GS_SET_UV(GETUVCOORD_NEAR((i+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
			ADDTOPKT(SCE_GS_XYZ2, 	SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
		}
		*/

		// 3rd pass copy back to framebuffer with colour applied to zbuffer
		ADDTOPKT(SCE_GS_FRAME_1, 	SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0x00000000) );
		ADDTOPKT(SCE_GS_TEX0_1, 	SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, zpsm, 10, 10, 0, SCE_GS_MODULATE, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, 	SCE_GS_SET_TEX1( 0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_ALPHA_1, 	SCE_GS_SET_ALPHA(0,2,2,1,col2.alpha) );
		ADDTOPKT(SCE_GS_PRIM, 		SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_RGBAQ, 		SCE_GS_SET_RGBAQ(col2.red, col2.green, col2.blue, 0, 0) );

		// Draw screen buffer in 32 pixel strips
		const int32 count = (width>>5);
		for(int32 i=0;i<count;i++)
		{
			float ratio1 = i/(float)(width>>5);
			float offset1 = m_colourLeftUOffset * (1.0f - ratio1) + m_colourRightUOffset * ratio1;
			float ratio2 = (i+1)/(float)(width>>5);
			float offset2 = m_colourLeftUOffset * (1.0f - ratio2) + m_colourRightUOffset * ratio2;
			ADDTOPKT(SCE_GS_UV, 	SCE_GS_SET_UV(GETUVCOORD(i<<5)+offset1, GETUVCOORD(0)+m_colourTopVOffset));
			ADDTOPKT(SCE_GS_XYZ2, 	SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));
			ADDTOPKT(SCE_GS_UV, 	SCE_GS_SET_UV(GETUVCOORD((i+1)<<5)+offset2, GETUVCOORD(SCREEN_DRAW_HEIGHT)+m_colourBottomVOffset));
			ADDTOPKT(SCE_GS_XYZ2, 	SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
		}

		ADDTOPKT(SCE_GS_CLAMP, 		SCE_GS_SET_CLAMP(0,0, 0,0,0,0));

		//LOAD_GIFTAG_QWC();
		ASSERT(_numOfQuadWords == numGifQWords);
	}
	RWDMA_LOCAL_BLOCK_END();

#else

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
#if 0//def GTA_XBOX
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSCLAMP);
#endif

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);

//	RwIm2DVertexSetIntRGBA(&pe_vertex[0], col1.red, col1.green, col1.blue, /*col1.alpha*/255);
//	RwIm2DVertexSetIntRGBA(&pe_vertex[1], col1.red, col1.green, col1.blue, /*col1.alpha*/255);
//	RwIm2DVertexSetIntRGBA(&pe_vertex[2], col1.red, col1.green, col1.blue, /*col1.alpha*/255);
//	RwIm2DVertexSetIntRGBA(&pe_vertex[3], col1.red, col1.green, col1.blue, /*col1.alpha*/255);
//	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDZERO);
// 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDSRCALPHA);
//	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, pe_vertex, 4, pe_index, 6);

	RwIm2DVertexSetIntRGBA(&pe_vertex[0], col1.red, col1.green, col1.blue, col1.alpha);
	RwIm2DVertexSetIntRGBA(&pe_vertex[1], col1.red, col1.green, col1.blue, col1.alpha);
	RwIm2DVertexSetIntRGBA(&pe_vertex[2], col1.red, col1.green, col1.blue, col1.alpha);
	RwIm2DVertexSetIntRGBA(&pe_vertex[3], col1.red, col1.green, col1.blue, col1.alpha);
//	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
//	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, pe_vertex, 4, pe_index, 6);

//	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
//	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwIm2DVertexSetIntRGBA(&pe_vertex[0], col2.red, col2.green, col2.blue, col2.alpha);
    RwIm2DVertexSetIntRGBA(&pe_vertex[1], col2.red, col2.green, col2.blue, col2.alpha);
    RwIm2DVertexSetIntRGBA(&pe_vertex[2], col2.red, col2.green, col2.blue, col2.alpha);
    RwIm2DVertexSetIntRGBA(&pe_vertex[3], col2.red, col2.green, col2.blue, col2.alpha);
//	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
// 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, pe_vertex, 4, pe_index, 6);

	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
//	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
//	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

#endif
}

//----------------------------------------------------------------------
// Grainy Screen
//----------------------------------------------------------------------

void	CPostEffects::Grain( int32 strengthMask, bool update )
{
//goto Skip_this;

	UInt32 randomSeed;

	if ( update )
	{
		RwRGBA *pixels = (RwRGBA *)RwRasterLock(m_pGrainRaster, 0, rwRASTERLOCKWRITE);
		// initialize R register

		UInt32         randomSeedCount  = 0;
		static UInt32  randomSeedCount2 = 0;

		randomSeed = CTimer::GetTimeInMilliseconds() + (unsigned)time( NULL );
		srand( randomSeed );

		srand(CTimer::GetCurrentTimeInMilleseconds());
		Int32 grain_w = GRAIN_RASTER_SIZE;// * HUD_MULT_X;
		Int32 grain_h = GRAIN_RASTER_SIZE;// * HUD_MULT_Y;
		for (RwInt32 cnt = 0; cnt < grain_w * grain_h; cnt++)
		{
			// Make sure a new random seed is set every "rndN"th pixel.
			// This makes sure we have a "real" whito noise style grain effect!
			UInt32 rndN = 100;
			randomSeedCount++;
			if( randomSeedCount >= rndN )
			{
				randomSeedCount = 0;
				randomSeedCount2++;
				randomSeed = CTimer::GetTimeInMilliseconds() + (unsigned)time( NULL ) + randomSeedCount2;
				srand( randomSeed );
			} // if //

			pixels[cnt].red = pixels[cnt].green = pixels[cnt].blue = pixels[cnt].alpha =
		/*	CGeneral::GetRandomNumberInRange( 32, 255 );*/
			rand();
		} // for cnt //

		RwRasterUnlock(m_pGrainRaster);
	} // if //

	// Store and set render states
	ImmediateModeRenderStatesStore();
	ImmediateModeRenderStatesSet();

	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS, (void *)rwTEXTUREADDRESSWRAP ); // Wrap must be used for grain FX!
	RwRenderStateSet( rwRENDERSTATETEXTUREFILTER,  (void *)rwFILTERLINEAR );

//	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
//	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
//	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
//	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);

//	strengthMask = 255; Debug! Forces maximum intensity!

	float fGrainMaxU = 1.5f; // Default texture X size = 1.5

	if( SCREEN_WIDTH > 640 )
	{
		fGrainMaxU *= ( (float)( SCREEN_WIDTH ) / 640.0f );
	} // if //

	float fGrainMaxV = fGrainMaxU * ( (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH );

//sprintf( gString, "UV = (%.2f, %.2f)", fGrainMaxU, fGrainMaxV );
//VarConsole.AddDebugOutput( gString );

	DrawQuadSetUVs(       0.0f,       0.0f, /* Top Left     */
					fGrainMaxU,       0.0f, /* Top Right    */
					fGrainMaxU, fGrainMaxV, /* Bottom Right */
					      0.0f, fGrainMaxV  /* Bottom Left  */ );

//	RwRenderStateSet( rwRENDERSTATETEXTUREFILTER,  (void *)rwFILTERLINEAR );

//	RwTextureSetAddressing( RwRasterGetTexture( m_pGrainRaster ), (void *)rwTEXTUREADDRESSWRAP );

//	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS,  (void *)rwTEXTUREADDRESSWRAP ); // This must be set for PC!
//	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESSU, (void *)rwTEXTUREADDRESSWRAP );
//	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESSV, (void *)rwTEXTUREADDRESSWRAP );

/*
	RwRenderStateSet( rwRENDERSTATETEXTURERASTER,  m_pGrainRaster );

	RwIm2DVertexSetIntRGBA( &ms_imf.triangle[0], 255, 255, 255, 255 );
	RwIm2DVertexSetIntRGBA( &ms_imf.triangle[1], 255, 255, 255, 255 );
	RwIm2DVertexSetIntRGBA( &ms_imf.triangle[2], 255, 255, 255, 255 );

//	for( Int32 i = 0; i < 1; i++ )
//	{
		RwIm2DVertexSetU( &ms_imf.triangle[0], ms_imf.uMinTri,              ms_imf.recipCameraZ );
		RwIm2DVertexSetV( &ms_imf.triangle[0], ms_imf.vMinTri,              ms_imf.recipCameraZ );

		RwIm2DVertexSetU( &ms_imf.triangle[1], ms_imf.uMaxTri * fGrainMaxU, ms_imf.recipCameraZ );
		RwIm2DVertexSetV( &ms_imf.triangle[1], ms_imf.vMinTri,              ms_imf.recipCameraZ );

		RwIm2DVertexSetU( &ms_imf.triangle[2], ms_imf.uMinTri,              ms_imf.recipCameraZ );
		RwIm2DVertexSetV( &ms_imf.triangle[2], ms_imf.vMaxTri * fGrainMaxV, ms_imf.recipCameraZ );

//	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESS,  (void *)rwTEXTUREADDRESSWRAP ); // This must be set for PC!
//	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESSU, (void *)rwTEXTUREADDRESSWRAP );
//	RwRenderStateSet( rwRENDERSTATETEXTUREADDRESSV, (void *)rwTEXTUREADDRESSWRAP );

		RwIm2DRenderTriangle( ms_imf.triangle, 3, 0, 1, 2 );
//	} // for i //

*/

// /*
	DrawQuad( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT,
			  255, 255, 255, strengthMask,
			  m_pGrainRaster );

	DrawQuadSetDefaultUVs(); // Reset UV's!
// */

	// Restore the render states
	ImmediateModeRenderStatesReStore();

/*

	if ( update )
	{
		RwRGBA *pixels = (RwRGBA *)RwRasterLock(m_pGrainRaster, 0, rwRASTERLOCKWRITE);
		// initialize R register

		srand(CTimer::GetCurrentTimeInMilleseconds());
		Int32 grain_w = GRAIN_RASTER_SIZE * HUD_MULT_X;
		Int32 grain_h = GRAIN_RASTER_SIZE * HUD_MULT_Y;
		for (RwInt32 cnt = 0; cnt < grain_w * grain_h; cnt++)
		{
			pixels[cnt].red = pixels[cnt].green = pixels[cnt].blue = pixels[cnt].alpha = rand();
		}

		RwRasterUnlock(m_pGrainRaster);
	}

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)m_pGrainRaster);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

    RwIm2DVertexSetIntRGBA(&pe_vertex[0], 255, 255, 255, strengthMask);
    RwIm2DVertexSetIntRGBA(&pe_vertex[1], 255, 255, 255, strengthMask);
    RwIm2DVertexSetIntRGBA(&pe_vertex[2], 255, 255, 255, strengthMask);
    RwIm2DVertexSetIntRGBA(&pe_vertex[3], 255, 255, 255, strengthMask);
 	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, pe_vertex, 4, pe_index, 6);

 
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);    
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

*/

Skip_this:;

}
/*
#ifdef GTA_PS2

//----------------------------------------------------------------------
// Depth of Field With Z-Buffer Fog
//----------------------------------------------------------------------

void	CPostEffects::DepthOfFieldWithZFog( RwRGBA fog )
{
	static uint32 CLIP_ZVALUE = 0xff8000;
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
//	const int iQWC = 9;		// quad word count
	
	int32 width, psm, zpsm;
	uint32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
	uint32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);

	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 100) )
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );
	
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
	ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
	ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1) );

	// 1st pass: invert z-buffer values
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,zpsm,0xff000000) );
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,2,128) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	// 2nd pass: remove closest (highest) z-buffer values from z-buffer, keeping only lower values
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(1,0,0,2,0,0,1,SCE_GS_ZGREATER) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 & (~(1L << 32) ));
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,0,0,0,0,0) );
	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(0, 0, 0, 0, 0) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), CLIP_ZVALUE) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), CLIP_ZVALUE) );
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	

	// 3rd pass: copy z-buffer G channel into framebuffer alpha
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,SCE_GS_PSMCT16S,0x00003fff) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, SCE_GS_PSMCT16S, 10, 10, 1, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_TEXA, SCE_GS_SET_TEXA( 0,0,0 ) );
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,2,64) );
	ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT*2-1));
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
	
	for (RwInt32 i=0;i<(width>>4);i++)
	{
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+(i<<8),8) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ((8<<4)+(i<<8),(0<<4),0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+(8<<4)+(i<<8),8+((SCREEN_DRAW_HEIGHT*2)<<4)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ((8<<4)+(8<<4)+(i<<8),((SCREEN_DRAW_HEIGHT*2)<<4),0) );
	}
	ADDTOPKT(SCE_GS_TEXA, SCE_GS_SET_TEXA( 0,0,128 ) );
	ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
	
	// 4th pass: downsample framebuffer to workbuffer
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
	const RwInt32 w = width/2;
	const RwInt32 h = SCREEN_DRAW_HEIGHT/2;
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(w), GETPRIMCOORD(h), 0) );
	
	// 5th pass x2: draw workbuffer onto framebuffer with dest alpha (z-buf channel)
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,1,0,0) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(1), GETUVCOORD(1)) );
	RwInt32 offset = 16;
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(1)+offset, GETPRIMCOORD(1)+offset, 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(w), GETUVCOORD(h)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width+1)+offset, GETPRIMCOORD(SCREEN_DRAW_HEIGHT+1)+offset, 0) );
	
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0)+offset, GETUVCOORD(0)+offset) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(w), GETUVCOORD(h)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width)-offset, GETPRIMCOORD(SCREEN_DRAW_HEIGHT)-offset, 0) );
	
	if ( fog.alpha  )
	{
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(fog.red, fog.green, fog.blue, 0, 0) );
		// 6th pass: add saturation
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,fog.alpha) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
		// 6th pass: add fog
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,1,1,0) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	}
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
	
	LOAD_GIFTAG_QWC();
	RWDMA_LOCAL_BLOCK_END();
}
*/
//----------------------------------------------------------------------
// Out of Focus
//----------------------------------------------------------------------

/*void	CPostEffects::OutofFocus( RwInt32 strength, RwTexture *pMaskTexture, RwReal maskScale, RwInt32 maskBrightness )
{
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm, zpsm;
	unsigned long tex0;
	unsigned long dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm, tex0);
	unsigned long dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	RwInt32 pMaskAddr, pCLUTAddr;
	RwRaster *pMaskRaster;
	if ( pMaskTexture != NULL )
	{
		pMaskRaster = RwTextureGetRaster(pMaskTexture);
		RpSkyTexCacheRasterLock(pMaskRaster,TRUE);
		pMaskAddr = RpSkyTexCacheRasterGetAddr(pMaskRaster);
		
		SetGrayScaleCLUTRaster256(pMaskTexture,strength);
		RpSkyTexCacheRasterLock(m_pGrayScaleCLUTRaster256,TRUE);
		pCLUTAddr = RpSkyTexCacheRasterGetAddr(m_pGrayScaleCLUTRaster256);
	}
	
	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 32) )
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );
	
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
	ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
	
	// 1st pass: downsample framebuffer to workbuffer
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
	const RwInt32 w = width/4;
	const RwInt32 h = SCREEN_DRAW_HEIGHT/3;
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(w), GETPRIMCOORD(h), 0) );
	
	if ( pMaskTexture != NULL )
	{
		// copy mask to framebuffer alpha
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0x00ffffff) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(pMaskAddr, 64>>6, SCE_GS_PSMT8, 10, 10, 1,
																						SCE_GS_DECAL, pCLUTAddr, SCE_GS_PSMCT32, 0, 0, 1) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(10), GETUVCOORD(6)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(RwRasterGetWidth(pMaskRaster)-20*maskScale),
																			GETUVCOORD(RwRasterGetHeight(pMaskRaster)-12*maskScale)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	}
	
	// 2nd pass x2: draw workbuffer onto framebuffer
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 0));
	if ( pMaskTexture != NULL )
	{
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,1,1,0) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(maskBrightness, maskBrightness, maskBrightness, 128, 0) );
	}
	else
	{
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,1,strength) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0) );
	}
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
	const RwInt32 offset = 16;
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(offset, offset, 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(w), GETUVCOORD(h)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(offset+GETPRIMCOORD(width), offset+GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+offset, 8+offset) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(w), GETUVCOORD(h)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width)-offset, GETPRIMCOORD(SCREEN_DRAW_HEIGHT)-offset, 0) );
	
	LOAD_GIFTAG_QWC();
	RWDMA_LOCAL_BLOCK_END();
	
	if ( pMaskTexture != NULL )
	{
		RpSkyTexCacheRasterLock(m_pGrayScaleCLUTRaster256,FALSE);
		RpSkyTexCacheRasterLock(pMaskRaster,FALSE);
	}
}*/

//----------------------------------------------------------------------
// Glow of bright areas
//----------------------------------------------------------------------
/*
void CPostEffects::HighlightGlow( int32 minIntensity, int32 strength, int32 scale, int32 offset, bool mblur )
{
#ifdef GTA_PS2
	RwRaster* pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm, zpsm;
	int32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
	int32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	RwRaster *pSrcRaster;
	if ( mblur )
		pSrcRaster = RpSkyGetDisplayBufferRaster();
	else
		pSrcRaster = pDrawBuffer;
	int32 dwSrcRasterAddr = GetRasterInfo(pSrcRaster, width, psm);
	
	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 32) )
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );
	
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
	ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1) );
	
	// 1st pass: downsample framebuffer to workbuffer
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwSrcRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width/scale), GETPRIMCOORD(SCREEN_DRAW_HEIGHT/scale), 0) );
	
	// 2nd: remove lower intensity values
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,2,1,128) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(minIntensity, minIntensity, minIntensity, 128, 0) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width/scale), GETPRIMCOORD(SCREEN_DRAW_HEIGHT/scale), 0) );
	
	// 3rd pass x2: draw on top of framebuffer
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEXFLUSH, 0);
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-(scale+1), 0, SCREEN_DRAW_HEIGHT-1));
//	ADDTOPKT(SCE_GS_ALPHA_1, SET_ALPHA_NOBLEND() );
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,strength>>1) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width/scale-1), GETUVCOORD(SCREEN_DRAW_HEIGHT/scale-1)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width-offset*2), GETPRIMCOORD(SCREEN_DRAW_HEIGHT-offset), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(offset),  GETPRIMCOORD(offset), 0) );
	ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width/scale-1), GETUVCOORD(SCREEN_DRAW_HEIGHT/scale-1)) );
	ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width+offset*2), GETPRIMCOORD(SCREEN_DRAW_HEIGHT+offset), 0) );
	
	ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
	
	LOAD_GIFTAG_QWC();
	RWDMA_LOCAL_BLOCK_END();
#else
	RwRaster *pSrcRaster;
	if ( mblur )
		pSrcRaster = pRasterFrontBuffer;
	else
		pSrcRaster = NULL;

    RwReal w = RwRasterGetWidth(pRasterFrontBuffer);
    RwReal h = RwRasterGetHeight(pRasterFrontBuffer);
    RwReal U, V, u, v;
    RwReal zero = 0.0f;
    V = (h/scale+0.5f)/h;
    v = (0.5f)/h;
    U = (w/scale+0.5f)/w; 
    u = (0.5f)/w; 

	static	RwIm2DVertex pe_vertex2[4];

//    RwIm2DVertexSetScreenX(&pe_vertex2[0], zero);
//    RwIm2DVertexSetScreenY(&pe_vertex2[0], zero);
    RwIm2DVertexSetScreenZ(&pe_vertex2[0], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex2[0], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex2[0], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
//    RwIm2DVertexSetU(&pe_vertex2[0], u, 1.0f);
//    RwIm2DVertexSetV(&pe_vertex2[0], v, 1.0f);
    
//    RwIm2DVertexSetScreenX(&pe_vertex2[1], zero);
//    RwIm2DVertexSetScreenY(&pe_vertex2[1], h/scale);
    RwIm2DVertexSetScreenZ(&pe_vertex2[1], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex2[1], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex2[1], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
//    RwIm2DVertexSetU(&pe_vertex2[1], u, 1.0f);
//    RwIm2DVertexSetV(&pe_vertex2[1], V, 1.0f);
    
//    RwIm2DVertexSetScreenX(&pe_vertex2[2], w/scale);
//    RwIm2DVertexSetScreenY(&pe_vertex2[2], h/scale);
    RwIm2DVertexSetScreenZ(&pe_vertex2[2], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex2[2], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex2[2], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
//    RwIm2DVertexSetU(&pe_vertex2[2], U, 1.0f);
//    RwIm2DVertexSetV(&pe_vertex2[2], V, 1.0f);
    
//    RwIm2DVertexSetScreenX(&pe_vertex2[3], w/scale);
//    RwIm2DVertexSetScreenY(&pe_vertex2[3], zero);
    RwIm2DVertexSetScreenZ(&pe_vertex2[3], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetCameraZ(&pe_vertex2[3], RwCameraGetNearClipPlane(Scene.camera));
    RwIm2DVertexSetRecipCameraZ(&pe_vertex2[3], 1.0f/RwCameraGetNearClipPlane(Scene.camera));
//    RwIm2DVertexSetU(&pe_vertex2[3], U, 1.0f);
//    RwIm2DVertexSetV(&pe_vertex2[3], v, 1.0f);

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
//	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pSrcRaster);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

    V = (h+0.5f)/h;
    v = (0.5f)/h;
    U = (w+0.5f)/w; 
    u = (0.5f)/w; 

    RwIm2DVertexSetScreenX(&pe_vertex2[0], zero);
    RwIm2DVertexSetScreenY(&pe_vertex2[0], zero);
    RwIm2DVertexSetU(&pe_vertex2[0], u, 1.0f);
    RwIm2DVertexSetV(&pe_vertex2[0], v, 1.0f);
    
    RwIm2DVertexSetScreenX(&pe_vertex2[1], zero);
    RwIm2DVertexSetScreenY(&pe_vertex2[1], h+offset);
    RwIm2DVertexSetU(&pe_vertex2[1], u, 1.0f);
    RwIm2DVertexSetV(&pe_vertex2[1], V, 1.0f);
    
    RwIm2DVertexSetScreenX(&pe_vertex2[2], w+offset*2);
    RwIm2DVertexSetScreenY(&pe_vertex2[2], h+offset);
    RwIm2DVertexSetU(&pe_vertex2[2], U, 1.0f);
    RwIm2DVertexSetV(&pe_vertex2[2], V, 1.0f);
    
    RwIm2DVertexSetScreenX(&pe_vertex2[3], w+offset*2);
    RwIm2DVertexSetScreenY(&pe_vertex2[3], zero);
    RwIm2DVertexSetU(&pe_vertex2[3], U, 1.0f);
    RwIm2DVertexSetV(&pe_vertex2[3], v, 1.0f);

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
    RwIm2DVertexSetIntRGBA(&pe_vertex2[0], 255, 255, 255, strength);
    RwIm2DVertexSetIntRGBA(&pe_vertex2[1], 255, 255, 255, strength);
    RwIm2DVertexSetIntRGBA(&pe_vertex2[2], 255, 255, 255, strength);
    RwIm2DVertexSetIntRGBA(&pe_vertex2[3], 255, 255, 255, strength);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
 	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
 	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, pe_vertex2, 4, pe_index, 6);

    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);    
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

#endif	
}*/

//
//
//
//
void CPostEffects::UnderWaterRipple(RwRGBA col, float xoffset, float yoffset, int32 strength, float speed, float freq)
{
#ifdef GTA_PS2
	
	RwRaster *pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm, zpsm;
	int32 	dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
	uint32 	dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	
	

	const uint32 numGifQWords = 20 + ((width>>5)*4) + ((SCREEN_DRAW_HEIGHT/int32(yoffset)+1)*4);
	const uint32 numDMAQWords = 1 + numGifQWords;
	
	if (!_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords))
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, 	SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1));
		
		ADDTOPKT(SCE_GS_TEST_1, 		SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, SCE_GS_ZALWAYS));
		ADDTOPKT(SCE_GS_ZBUF_1, 		skyZbuf_1 | (1L << 32));
		ADDTOPKT(SCE_GS_XYOFFSET_1, 	SCE_GS_SET_XYOFFSET(0,0));
		ADDTOPKT(SCE_GS_PABE, 			SCE_GS_SET_PABE(0));

		// 1st pass copy to zbuffer 
		ADDTOPKT(SCE_GS_FRAME_1, 		SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,zpsm,0xff000000) );
		ADDTOPKT(SCE_GS_TEX0_1, 		SCE_GS_SET_TEX0( dwRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, 		SCE_GS_SET_TEX1( 0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_ALPHA_1, 		SCE_GS_SET_ALPHA(0,2,2,2,128) );
		ADDTOPKT(SCE_GS_PRIM, 			SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_RGBAQ, 			SCE_GS_SET_RGBAQ(127, 127, 127, 0, 0) );

		// Draw screen buffer in 32 pixel strips
		const int32 count = (width>>5);
		for (RwInt32 i=0;i<count;i++)
		{
			ADDTOPKT(SCE_GS_UV, 		SCE_GS_SET_UV(GETUVCOORD_NEAR(i<<5), GETUVCOORD/*_NEAR*/(0)));
			ADDTOPKT(SCE_GS_XYZ2, 		SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));
			ADDTOPKT(SCE_GS_UV, 		SCE_GS_SET_UV(GETUVCOORD_NEAR((i+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
			ADDTOPKT(SCE_GS_XYZ2, 		SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
		}


		// Draw screen in strips with sin wave affecting the x values, to produce ripple effect
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5, width>>6, psm, 0xff000000));
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwZBufRasterAddr, width>>6, zpsm, 10, 10, 0, SCE_GS_MODULATE, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,0,strength>>1) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(col.red, col.green, col.blue, strength, 0));

		int32 y=0;
		for(y=0; y<SCREEN_DRAW_HEIGHT; y+=yoffset)
		{
			const float xofs = xoffset * CMaths::Sin(CTimer::GetTimeInMilliseconds() * speed + y*freq);
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(xoffset + xofs), GETUVCOORD(y)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(y), 0));
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((width+xofs-xoffset)), GETUVCOORD(y)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((width)), GETPRIMCOORD(y), 0));
		}
		const float xofs = CMaths::Sin(CTimer::GetTimeInMilliseconds() * speed + y*freq);
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(xoffset+xofs), GETUVCOORD(y)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(y), 0));
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((width+xofs-xoffset)), GETUVCOORD(y)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((width)), GETPRIMCOORD(y), 0));

		/*	
		RwInt32 y;
		float xofs;
		float factor = 0.0f;
		float angle = CTimer::GetTimeInMilliseconds() * speed;
		RwInt32 yofs = yoffset + 6.0f * CMaths::Cos(angle);
		factor = 6.0f * CMaths::Sin(angle);

		for (y = 0; y < SCREEN_DRAW_HEIGHT; y+=yofs)
		{
			if (factor < 0.0f)
				xofs = ((y+yofs)/yofs) & 1;
			else
				xofs = ((y)/yofs) & 1;
			if (!xofs)
			{
				xofs = 0;
			}else
			{
				if (factor > 0.0f)
					xofs *= (xoffset + (int32) factor);
				else
					xofs *= (xoffset + (int32) (-1.0f * factor));
			}
			ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(col.red, col.green, col.blue, strength, 0));
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(xofs), GETUVCOORD(y)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(y), 0));
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((width+xofs)), GETUVCOORD(y)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((width)), GETPRIMCOORD(y), 0));
		}
		y = SCREEN_DRAW_HEIGHT;
		if (factor < 0.0f)
			xofs = ((y+yofs)/yofs) & 1;
		else
			xofs = ((y)/yofs) & 1;
		if (!xofs)
			xofs = 0;
		else
			if (factor >= 0.0f)
				xofs *= (xoffset + (int32) factor);
			else
				xofs *= (xoffset + (int32) (-1.0f*factor));
		*/
		/*	
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(col.red, col.green, col.blue, strength, 0));
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(xofs), GETUVCOORD(y)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(y), 0));
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((width+xofs)), GETUVCOORD(y)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((width)), GETPRIMCOORD(y), 0));
		*/

		ASSERT(_numOfQuadWords == numGifQWords);
		//LOAD_GIFTAG_QWC();
	}
	RWDMA_LOCAL_BLOCK_END();

#else

	// PC & XBOX code...

	//#########################################################################
	//#########################################################################
	//#########################################################################
	// Temporary code until GeForce 5 code is fixed
	// Water wobble FX is OFF if FX quality equals FX_QUALITY_LOW

	/*
		enum FxQuality_e
		{
			FX_QUALITY_LOW = 0,
			FX_QUALITY_MEDIUM,
			FX_QUALITY_HIGH,
			FX_QUALITY_VERY_HIGH,
		};
	*/

/* GeForce 5 problem is now fixed. This is not required anymore!

	FxQuality_e fxq;
	fxq = g_fx.GetFxQuality();

	if( fxq == FX_QUALITY_LOW )
	{
		return;
	} // if //

*/
	//#########################################################################
	//#########################################################################
	//#########################################################################

#ifdef GTA_PC
	strength = 255; // Alpha MUST be maximum for PC!
#endif
	Int32 r  = col.red;
	Int32 g  = col.green;
	Int32 b  = col.blue;

	RwInt32 y;
	float xofs, u, v;
	#ifdef GTA_PC
		RwCameraEndUpdate(Scene.camera);
/* This is not required here! It's already done!
		RwRasterPushContext(pRasterFrontBuffer);
//		RsCameraBeginUpdate(Scene.camera); // ### Do NOT use this for "RwRasterPushContext" and "RwRasterPopContext"!! ###
		RwRasterRenderFast(RwCameraGetRaster(Scene.camera), 0, 0);
//		RwCameraEndUpdate(Scene.camera);
		RwRasterPopContext();
*/
		RsCameraBeginUpdate(Scene.camera);
	#elif defined(GTA_XBOX)
		// im mode pRasterFrontBuffer >>> pRasterFrontBuffer
		RasterCopyPostEffects();
	#endif

	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;

//	RwRGBA clearColor = {0,0,0,0};
//	RwCameraClear(Scene.camera, &clearColor, rwCAMERACLEARIMAGE);

	// Store and set render states
	ImmediateModeRenderStatesStore();
	ImmediateModeRenderStatesSet();

//	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERNEAREST);
//	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
//	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
//	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)pRasterFrontBuffer);
//	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
//	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
//	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

//	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
//	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDSRCALPHA);

	Int32 xCorrect = xoffset * 2; // This makes sure the "waves" are not visible at the border!

//sprintf( gString, "n = %d", xCorrect );
//VarConsole.AddDebugOutput( gString );

	for (y = 0; y < RwRasterGetHeight(pRasterFrontBuffer); y+=yoffset)
	{
		xofs = xoffset * CMaths::Sin(CTimer::GetTimeInMilliseconds() * speed + y * freq);
#if 0//def GTA_XBOX
		u =  (float) (xoffset + xofs);
 		v =  (float) (y);
#else
 		u =  (float) (xoffset + xofs) / (float) RwRasterGetWidth(pRasterFrontBuffer);
 		v =  (float) (y) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
	    RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), u, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), v, 1.0f);
  		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), y);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
#if 0//def GTA_XBOX
		u =  (float) ((RwRasterGetWidth(pRasterFrontBuffer)+xofs-xoffset));
#else
		u =  (float) ((RwRasterGetWidth(pRasterFrontBuffer)+xofs-xoffset)) / (float) RwRasterGetWidth(pRasterFrontBuffer);
#endif
		RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
	    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
		RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwRasterGetWidth(pRasterFrontBuffer) + xCorrect);
		RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), y);
    	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwIm2DGetNearScreenZ());
    	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
    	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), r, g, b, strength);
		RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), r, g, b, strength);
		TempBufferVerticesStored += 2;
	}
	xofs = CMaths::Sin(CTimer::GetTimeInMilliseconds() * speed + y*freq);
#if 0//def GTA_XBOX
	u =  (float) (xoffset + xofs);
 	v =  (float) (y);
#else
	u =  (float) (xoffset + xofs) / (float) RwRasterGetWidth(pRasterFrontBuffer);
 	v =  (float) (y) / (float) RwRasterGetHeight(pRasterFrontBuffer);
#endif
	RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), u, 1.0f);
    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), v, 1.0f);
	RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 0);
	RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), y);
	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), RwCameraGetNearClipPlane(Scene.camera));
	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
#if 0//def GTA_XBOX
	u = (float) ((RwRasterGetWidth(pRasterFrontBuffer)+xofs-xoffset));
#else
	u = (float) ((RwRasterGetWidth(pRasterFrontBuffer)+xofs-xoffset)) / (float) RwRasterGetWidth(pRasterFrontBuffer);
#endif
	RwIm2DVertexSetU(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), u, 1.0f);
    RwIm2DVertexSetV(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), v, 1.0f);
	RwIm2DVertexSetScreenX(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwRasterGetWidth(pRasterFrontBuffer) + xCorrect);
	RwIm2DVertexSetScreenY(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), y);
	RwIm2DVertexSetScreenZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), RwCameraGetNearClipPlane(Scene.camera));
	RwIm2DVertexSetRecipCameraZ(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), 1.0f/RwCameraGetNearClipPlane(Scene.camera));
	RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored]), r, g, b, strength);
	RwIm2DVertexSetIntRGBA(&(TempVertexBuffer.m_2d[TempBufferVerticesStored+1]), r, g, b, strength);
	TempBufferVerticesStored+=2;
	if (TempBufferVerticesStored > 2)
	{
		RwIm2DRenderPrimitive_BUGFIX(rwPRIMTYPETRISTRIP, &TempVertexBuffer.m_2d[0], TempBufferVerticesStored);
	}

	// Restore the render states
	ImmediateModeRenderStatesReStore();

/*
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
*/
#endif
}

// This is for PC & XBOX only!
void CPostEffects::UnderWaterRippleFadeToFX( void )
{
	// Fade to underwater color!
//	sprintf( gString, "uw = %.2f, uwStart = %.2f", CWeather::UnderWaterness, GetWaterFXStartUnderWaterness() );
//	VarConsole.AddDebugOutput( gString );
/*
	if( CWeather::UnderWaterness > 0.0f &&
		CWeather::UnderWaterness < GetWaterFXStartUnderWaterness() )
	{
		float fColDiv = 2.5f + CWeather::UnderWaterness;
		float fR = (float)col.red;
		float fG = (float)col.green;
		float fB = (float)col.blue;

		fR /=   fColDiv;
		fG /= ( fColDiv - 0.0f );
		fB /=   fColDiv;
	} // if //
*/
} // UnderWaterRippleFadeToFX //

#ifdef GTA_PS2

//
//
//
//
void CPostEffects::SmokeyFilter(int32 strength)
{

int32 fogStrength = strength;

RwRaster* pDrawBuffer	= RpSkyGetDrawBufferRaster();
RwInt32 width, psm, zpsm;
int32 dwRasterAddr = GetRasterInfo(pDrawBuffer, width, psm);
uint32 dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);

	fogStrength--;


const int32 theOffset	= 1;
const int32 theWidth 	= 32;
const int32 theHeight 	= 72;
	

	const uint32 numGifQWords	= 62 + ((width>>4)*4) + ((((width/2)-theWidth)/(theWidth*4)+1) * (2+(5*(SCREEN_DRAW_HEIGHT/theHeight+1))));
	const uint32 numDMAQWords	= 1 + numGifQWords;

	if(!_rwDMAOpenVIFPkt(RWDMA_FIXUP, numDMAQWords))
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	{
		ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(numGifQWords, 1, 0, 0, SCE_GIF_PACKED, 1) );

		ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
		ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
		ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
		ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );

		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
		ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1) );

		// 1st pass rg to ba
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,SCE_GS_PSMCT16S,0x00003fff) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT*2-1));
		ADDTOPKT(SCE_GS_TEXA, SCE_GS_SET_TEXA( 0,0,0x80 ) );
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, SCE_GS_PSMZ16S, 10, 10, 1, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,2,64) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );

		const int32 count = (width>>4);
		for(int32 i=0; i<count; i++)
		{
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+(i<<8),8) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ((8<<4)+(i<<8),(0<<4),0) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+(8<<4)+(i<<8),8+((SCREEN_DRAW_HEIGHT*2)<<4)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ((8<<4)+(8<<4)+(i<<8),((SCREEN_DRAW_HEIGHT*2)<<4),0) );
		}
		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));

		// 4th pass: downsample framebuffer to workbuffer
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)) );
		const int32 _w = width/2;
		const int32 _h = SCREEN_DRAW_HEIGHT/2;
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(_w), GETPRIMCOORD(_h), 0) );
		
		// 5th pass x2: draw workbuffer onto framebuffer with dest alpha (z-buf channel)
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,1,0,0) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(1), GETUVCOORD(1)) );
		const int32 _offset = 16;
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(1)+_offset, GETPRIMCOORD(1)+_offset, 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(_w), GETUVCOORD(_h)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width+1)+_offset, GETPRIMCOORD(SCREEN_DRAW_HEIGHT+1)+_offset, 0) );
		
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0)+_offset, GETUVCOORD(0)+_offset) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(_w), GETUVCOORD(_h)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width)-_offset, GETPRIMCOORD(SCREEN_DRAW_HEIGHT)-_offset, 0) );

		// 1st pass: darken framebuffer
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,2,2,32) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(16, 16, 16, 32, 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );

		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 1));
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(255, 255, 255, 128, 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );



		
		static RwReal rHaze = 3;
		static RwReal rHazeWobble = 0.1f;
		const RwInt32 d = 1L << 12;
		const RwInt32 HStrips = ((width/theWidth)/2) + 1;
		static RwInt32 *yofs=NULL;
		static RwInt32 *wadd=NULL;
		static RwInt32 xodd = 0;
		RwInt32 n;

		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 1));
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0));
	//	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,strength>>1) );

		rHaze += rHazeWobble;
		RwInt32 iMaxWobble;
		iMaxWobble = 4;
		if ( rHaze > iMaxWobble || rHaze < -(iMaxWobble-1) )
			rHazeWobble = -rHazeWobble;
		if ( !yofs )
		{
			yofs = new RwInt32 [HStrips];
			wadd = new RwInt32 [HStrips];
			for (n=0;n<HStrips;n++)
			{
				yofs[n] = 0;
				wadd[n] = 0;
			}
		}
		for (n=0;n<HStrips;n++)
		{
			RwInt32 iMask;
			iMask = 11;
			yofs[n] -= rand() & iMask;
			if ( yofs[n] < 0 )
			{
				yofs[n] = theHeight;
				wadd[n] = rand() & 10;
				if ( !n )
					xodd = (xodd & 1) + (rand()&1);
			}
		}

		n = 0;
		const int32 count2 = width/2;
		for (RwInt32 x=theWidth; x<count2; x += theWidth*4)
		{
			strength = MIN(MAX(64,strength-48), 255);
			
			ADDTOPKT(SCE_GS_PRIM, 		SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP,0,1,0,1,0,1,0,0) );
			ADDTOPKT(SCE_GS_ALPHA_1, 	SCE_GS_SET_ALPHA(0,2,2,0,strength>>1) );
			
			for (RwInt32 y = /*(n+1)*h*/0; y<SCREEN_DRAW_HEIGHT/*-(n+1)*h*/; y += theHeight)
			{
				RwInt32 xofs = (y/theHeight) & 1;
				if (!xofs)
					xofs = -rHaze;
				else
					xofs *= rHaze;
				RwInt32 alpha = (yofs[n] * 255) / theHeight;
				ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(100, 100, 100, /*(alpha > 128)?255-alpha:alpha*/strength, 0));
				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(x), GETUVCOORD(y+yofs[n])));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((x-xofs)+theOffset), GETPRIMCOORD(y+yofs[n]), d));
				ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD((width-x)-wadd[n]), GETUVCOORD(y+yofs[n])));
				ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((width-x)-xofs+theOffset-wadd[n]), GETPRIMCOORD(y+yofs[n]), d));
			}
			n++;
		}

		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,fogStrength) );
		fogStrength *= 0.1;
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(fogStrength, fogStrength, fogStrength, 0, 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(0), GETUVCOORD(0)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width), GETUVCOORD(SCREEN_DRAW_HEIGHT)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );


		//LOAD_GIFTAG_QWC();
		ASSERT(_numOfQuadWords == numGifQWords);
	}
	RWDMA_LOCAL_BLOCK_END();

}

/*
#define DROPSEG 	10
#define MAXDROPS 	5
#define DROPSIZE	12
void CPostEffects::RainDropFilter()
{
	RwRaster *m_pDrawBuffer = RpSkyGetDrawBufferRaster();
	RwInt32  width, psm, zpsm;
	int32 	 dwRasterAddr = GetRasterInfo(m_pDrawBuffer, width, psm);
	uint32   dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	int32 	 i, j;
	int32	 numDrops = MAXDROPS;
	static RwInt32 *xCoor = NULL;
	static RwInt32 *yCoor = NULL;
	static UInt8   *dropAlpha = NULL;
	
	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 512) )
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );

	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET(0,0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0));

	// 1st pass copy to zbuffer 
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,zpsm,0xff000000) );
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwRasterAddr, width>>6, psm, 10, 10, 0, SCE_GS_DECAL, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,2,128) );
	ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
	ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(127, 127, 127, 0, 0) );

	// Draw screen buffer in 32 pixel strips
	for (RwInt32 i=0;i<(width>>5);i++)
	{
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR(i<<5), GETUVCOORD_NEAR(0)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(i<<5),GETPRIMCOORD(0), 0));
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD_NEAR((i+1)<<5), GETUVCOORD_NEAR(SCREEN_DRAW_HEIGHT)));
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((i+1)<<5),GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0));
	}

	if (!xCoor)
	{
		xCoor = new RwInt32[MAXDROPS];
		yCoor = new RwInt32[MAXDROPS];
		dropAlpha = new UInt8[MAXDROPS];
		
		for (i = 0; i < MAXDROPS; i++)
		{
			xCoor[i] = 0;
			yCoor[i] = 0;
			dropAlpha[i] = 0;
		}
	}

	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, zpsm, 10, 10, 0, SCE_GS_MODULATE, 0, 0, 0, 0, 0));
	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );

	float x, y, angle, u, v;
	#define OFFSET 5
	#define RED 	127
	#define GREEN 	127
	#define BLUE	140
	float red, green, blue;
	for (i = 0; i < numDrops; i++)
	{
		if (dropAlpha[i] <= 0)
		{
			yCoor[i] = CGeneral::GetRandomNumberInRange(DROPSIZE, SCREEN_HEIGHT);
			xCoor[i] = CGeneral::GetRandomNumberInRange(DROPSIZE, SCREEN_WIDTH);
			dropAlpha[i] = CGeneral::GetRandomNumberInRange(0, 64);
		}
		
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_TRI,0,1,0,1,0,1,0,0));
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(RED, GREEN, BLUE, dropAlpha[i], 0) );
		for (j = 0; j < DROPSEG; j++)
		{
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(xCoor[i]), GETUVCOORD(yCoor[i])));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(xCoor[i]), GETPRIMCOORD(yCoor[i]), 0) );
			
			angle = (j+1) * TWO_PI / DROPSEG;
			x = MAX(0, xCoor[i] + DROPSIZE * CMaths::Cos(angle));
			y = MAX(0, yCoor[i] + DROPSIZE * CMaths::Sin(angle));
			red = MAX(0, RED - 30 * CMaths::Cos(angle+PI));
			green = MAX(0, GREEN - 30 * CMaths::Cos(angle+PI));
			blue = MAX(0, BLUE - 30 * CMaths::Cos(angle+PI));
			u = OFFSET * CMaths::Cos(angle);
			v = OFFSET * CMaths::Sin(angle);
			ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(red, green, blue, dropAlpha[i]-10, 0) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(x-u), GETUVCOORD(y-v)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(x), GETPRIMCOORD(y), 0) );
			
			angle = (j) * TWO_PI / DROPSEG;
			x = MAX(0, xCoor[i] + DROPSIZE * CMaths::Cos(angle));
			y = MAX(0, yCoor[i] + DROPSIZE * CMaths::Sin(angle));
			u = OFFSET * CMaths::Cos(angle);
			v = OFFSET * CMaths::Sin(angle);
			ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(red, green, blue, dropAlpha[i]-30, 0) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(x-u), GETUVCOORD(y-v)));
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(x), GETPRIMCOORD(y), 0) );
			
			
		}
		dropAlpha[i] -= CGeneral::GetRandomNumberInRange(5, 10);;
	}
	
	LOAD_GIFTAG_QWC();
	RWDMA_LOCAL_BLOCK_END();
}
*/
/*
//----------------------------------------------------------------------
// Heat Haze
//----------------------------------------------------------------------
void	CChannelFilter::HeatHaze( RwBool highStrength )
{
	RwRaster* m_pFrontBuffer = RpSkyGetDrawBufferRaster();
	RwInt32 width, psm;
	unsigned long tex0;
	unsigned long dwRasterAddr = GetRasterInfo(m_pFrontBuffer, width, psm, tex0);
	
	static RwReal rHaze = 3;
	static RwReal rHazeWobble = 0.5f;
//	static RwInt32 x = 100;
//	static RwInt32 w = 64;
//	static RwInt32 d = 1L << 18;
	const RwInt32 offset = 1;
	const RwInt32 w = 48;
	const RwInt32 h = 64;
	const RwInt32 d = 1L << 12;
	const RwInt32 HStrips = ((width/w)/2) + 1;
	static RwInt32 *yofs=NULL;
	static RwInt32 *wadd=NULL;
	static RwInt32 xodd = 0;
	RwInt32 n;
	
	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 32) )
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );
	
//	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZGREATER) );
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
	
	// 1st pass: draw wobbling sprite on framebuffer
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 1));
	ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
	ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,0,1,0) );
	//TO DO: have to replace this statics has member vars
	rHaze += rHazeWobble;
	RwInt32 iMaxWobble;
	if ( highStrength )
		iMaxWobble = 4;
	else
		iMaxWobble = 3;
	if ( rHaze > iMaxWobble || rHaze < -(iMaxWobble-1) )
		rHazeWobble = -rHazeWobble;
	if ( !yofs )
	{
		yofs = new RwInt32 [HStrips];
		wadd = new RwInt32 [HStrips];
		for (n=0;n<HStrips;n++)
		{
			yofs[n] = 0;
			wadd[n] = 0;
		}
	}
	for (n=0;n<HStrips;n++)
	{
		RwInt32 iMask;
		if ( highStrength )
			iMask = 31;
		else
			iMask = 15;
		yofs[n] -= rand() & iMask;
		if ( yofs[n] <= 0 )
		{
			yofs[n] = h;
			wadd[n] = rand() & 30;
			if ( !n )
				xodd = (xodd & 1) + (rand()&1);
		}
	}
	
//	LOAD_GIFTAG_QWC();
//	RWDMA_LOCAL_BLOCK_END();
	
	// TO DO: do this using vu1
	n = 0;
	for (RwInt32 x=xodd+(w>>1); x<width; x+=w<<1)
	{
		
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP,0,1,0,1,0,1,0,0) );
		for (RwInt32 y=0; y<SCREEN_DRAW_HEIGHT; y+=h)
		{
			RwInt32 xofs = (y/h) & 1;
			if ( !xofs)
				xofs = -rHaze;
			else
				xofs *= rHaze;
			RwInt32 alpha = (yofs[n] * 255) / h;
			ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, (alpha > 128) ? 255-alpha : alpha, 0) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(x), GETUVCOORD(y+yofs[n])) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD((x-xofs)+offset), GETPRIMCOORD(y+yofs[n]), d) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(x+w+wadd[n]), GETUVCOORD(y+yofs[n])) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(x+xofs+offset+w+wadd[n]), GETPRIMCOORD(y+yofs[n]), d) );
		}
//	LOAD_GIFTAG_QWC();
//	RWDMA_LOCAL_BLOCK_END();
		n++;
	}
	
	LOAD_GIFTAG_QWC();
	RWDMA_LOCAL_BLOCK_END();
}

//----------------------------------------------------------------------
// Flash Bang
//----------------------------------------------------------------------

void	CChannelFilter::FlashBang( RwInt32 strength )
{
	RwInt32 width, psm, zpsm;
	unsigned long tex0;
	unsigned long dwRasterAddr = GetRasterInfo(m_pFrontBuffer, width, psm, tex0);
	unsigned long dwTexRasterAddr = RpSkyTexCacheRasterGetAddr(m_pDisplayBuffer);
	RwRGBA *pixels = (RwRGBA *) RwRasterLock(m_pGrayScaleCLUTRaster256, 0, rwRASTERLOCKWRITE);
	for (RwInt32 i=0;i<256;i++)
	{
		pixels[i].red = pixels[i].green = pixels[i].blue = i;
		pixels[i].alpha = i;
	}
	RwRasterUnlock(m_pGrayScaleCLUTRaster256);
	RpSkyTexCacheRasterLock(m_pGrayScaleCLUTRaster256,TRUE);
	RwInt32 pCLUTAddr = RpSkyTexCacheRasterGetAddr(m_pGrayScaleCLUTRaster256);
	unsigned long dwZBufRasterAddr = GetZBufferRasterInfo(zpsm);
	
	if ( !_rwDMAOpenVIFPkt(RWDMA_FIXUP, 32) )
		return;
	
	RWDMA_LOCAL_BLOCK_BEGIN();
	ADDTOPKT(SCE_GIF_PACKED_AD, SCE_GIF_SET_TAG(0, 1, 0, 0, SCE_GIF_PACKED, 1) );
	
	ADDTOPKT(SCE_GS_TEST_1, SCE_GS_SET_TEST(0,0,0,0,0,0,1,SCE_GS_ZALWAYS) );
	ADDTOPKT(SCE_GS_ZBUF_1, skyZbuf_1 | (1L << 32) );
	ADDTOPKT(SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET_1(0, 0));
	ADDTOPKT(SCE_GS_PABE, SCE_GS_SET_PABE(0) );
	
	ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
	if ( strength == 255 )
	{
		// 1st pass: copy inverted framebuffer G channel into z-buffer alpha
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwZBufRasterAddr>>5,width>>6,SCE_GS_PSMCT16S,0x00003fff) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0(dwRasterAddr, width>>6, SCE_GS_PSMCT16S, 10, 10, 1, SCE_GS_DECAL, 0, 0, 0, 0, 0));
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_NEAREST, SCE_GS_NEAREST, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_TEXA, SCE_GS_SET_TEXA( 0,0,0 ) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(2,0,2,2,128) );
		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT*2-1));
		ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(0) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		for (RwInt32 i=0;i<(width>>4);i++)
		{
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+(i<<8),8) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ((8<<4)+(i<<8),(0<<4),0) );
			ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+(8<<4)+(i<<8),8+((SCREEN_DRAW_HEIGHT*2)<<4)) );
			ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ((8<<4)+(8<<4)+(i<<8),((SCREEN_DRAW_HEIGHT*2)<<4),0) );
		}
		ADDTOPKT(SCE_GS_TEXA, SCE_GS_SET_TEXA( 0,0,128 ) );
		ADDTOPKT(SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR(0, width-1, 0, SCREEN_DRAW_HEIGHT-1));
		ADDTOPKT(SCE_GS_COLCLAMP, SCE_GS_SET_COLCLAMP(1) );
	}
	if ( strength > 200 )
	{
		// replace fb with still negative image
		ADDTOPKT(SCE_GS_FRAME_1, SCE_GS_SET_FRAME(dwRasterAddr>>5,width>>6,psm,0xff000000) );
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwZBufRasterAddr, width>>6, SCE_GS_PSMT8H, 10, 10,
																						1, SCE_GS_DECAL, pCLUTAddr, SCE_GS_PSMCT32, 0, 0, 1));
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+((255-strength)<<5), 8) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV((8+(width << 4))-((255-strength)<<5), 8+(SCREEN_DRAW_HEIGHT << 4)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
		// add another negative layer
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,1,2,1,64) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8, 8+((255-strength)<<4)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV((8+(width << 4)), (8+(SCREEN_DRAW_HEIGHT << 4))-((255-strength)<<4)) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
		// add white on top of it
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,(255-strength)*3+80) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 0, 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	}
	else
	{
		// 1st pass: additive motion blur
		ADDTOPKT(SCE_GS_TEXFLUSH, 0);
		ADDTOPKT(SCE_GS_TEX0_1, SCE_GS_SET_TEX0( dwTexRasterAddr, width>>6, psm, 10, 10, 1, SCE_GS_MODULATE, 0, 0, 0, 0, 1));
		ADDTOPKT(SCE_GS_TEX1_1, SCE_GS_SET_TEX1(0, 0, SCE_GS_LINEAR, SCE_GS_LINEAR, 0, 0, 0 ) );
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(0,2,2,1,strength) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 0, 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(8+strength, 8+strength) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(0), GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_UV, SCE_GS_SET_UV(GETUVCOORD(width)-strength, GETUVCOORD(SCREEN_DRAW_HEIGHT)-strength) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
		// 2nd pass: adjust colour contrast
		ADDTOPKT(SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,0,0,0) );
		ADDTOPKT(SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA(1,0,2,1,strength) );
		ADDTOPKT(SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(128, 128, 128, 0, 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(2),  GETPRIMCOORD(0), 0) );
		ADDTOPKT(SCE_GS_XYZ2, SCE_GS_SET_XYZ(GETPRIMCOORD(width-2), GETPRIMCOORD(SCREEN_DRAW_HEIGHT), 0) );
	}
	
	LOAD_GIFTAG_QWC();
	RWDMA_LOCAL_BLOCK_END();
	
	RpSkyTexCacheRasterLock(m_pGrayScaleCLUTRaster,FALSE);
}

//----------------------------------------------------------------------

void	CChannelFilter::SetGrayScaleCLUTRaster256( RwTexture *pMaskTexture, RwInt32 alpha, RwBool invert )
{
	_rwDMAForceBufferSwap();
	
	RwRaster *pRast = RwTextureGetRaster(pMaskTexture);
	RwRGBA *pPalette = (RwRGBA *) RwRasterLockPalette(pRast, rwRASTERLOCKREAD);
	RwRGBA *pixels = (RwRGBA *) RwRasterLock(m_pGrayScaleCLUTRaster256, 0, rwRASTERLOCKWRITE);
	RwReal ratio = (1.0f / 128) * alpha;
	// TO DO: optimize this loop using vu0
	for (RwInt32 i=0;i<256;i++)
	{
		if ( invert )
			pixels[i].alpha = ((255 - pPalette[i].red) >> 1) * ratio;
		else
			pixels[i].alpha = (pPalette[i].red >> 1) * ratio;
	}
	RwRasterUnlock(m_pGrayScaleCLUTRaster256);
	RwRasterUnlockPalette(pRast);
}
*/
//----------------------------------------------------------------------
//	return:	base pointer of raster: (addr >> 11) << 5 = (addr >> 6) ,   width = (W >> 6) << 6
//----------------------------------------------------------------------
uint32 CPostEffects::GetRasterInfo(RwRaster *pRaster, int32 &width, int32 &format)
{
	uint32	msb, lsb, rasterAddress;
	// Find the base pointer of texture level raster...
	skyTexGetTex0(pRaster, &msb, &lsb);
	//	back buffer raster : << 5 done, off course, it is times of 64
	rasterAddress = lsb & 0x3FFF;	
	//	width is realSize / 64
	width = ((lsb >> 14) & 0x3F) << 6;
	format= ((lsb >> 20) & 0x3F);

//	lMsbLsb = ((((unsigned long)msb) << 32) | (unsigned long)lsb);
//	ASSERT(width == pRaster->originalWidth);
	return rasterAddress;
}


//----------------------------------------------------------------------
//----------------------------------------------------------------------
uint32 CPostEffects::GetZBufferRasterInfo( int32 &format )
{
	int32	rasterAddress;
	rasterAddress = (skyZbuf_1 & 0x1FF) << 5;
	format = (skyZbuf_1 >> 24) & 0x0F;
	format |= 0x30;
	
	return rasterAddress;
}

#endif //GTA_PS2....

