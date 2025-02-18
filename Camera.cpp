
/////////////////////////////////////////////////////////////////////////////////
//
// FILE :    Camera.cpp
// PURPOSE : Everything having to do with the camera.
// AUTHOR :  Obbe
// CREATED : 26-10-98
//
//
//
//
//
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef MASTER 
	#define DebugMenuOn
	#ifdef DebugMenuOn
		#include "VarConsole.h"
	#endif
#endif 
#include "Camera.h"
#include "Timer.h"
#include "Debug.h"
#include "automobile.h"
#include "pad.h"
#include "player.h"
#include "world.h"
#include "timer.h"
#include "general.h"
#include "Frontend.h"
#include "Game.h"
#include "PlayerPed.h"
#include "Maths.h"
#include "handy.h"
#include "quaternion.h"
#include "vehicle.h"
#include "FileMgr.h"
#include "string.h"
#include "main.h" 
#include "waterlevel.h" 
#include "Physical.h"
#include "Vector.h"
#include "Boat.h"
#include "Bike.h"
#include "Planes.h"
#include "Floater.h"
#include "shadows.h"
#include "setup.h"
#include "RwCam.h"
#include "sprite2d.h"
#include "Pools.h"
#include "ModelIndices.h"
#include "Train.h"
#include "Population.h"
#include "script.h"
#include "CivilianPed.h"
#include "Streaming.h"
#include "zonecull.h"
#include "fluff.h"
#include "mblur.h"
#include "globals.h"
#include "garages.h"
#include "HandlingMgr.h"
#include "Renderer.h"
#include "replay.h"
#include "particle.h"
#include "VehicleModelInfo.h"
#include "TaskPolice.h"
#include "IplStore.h"
#include "clouds.h"
#include "posteffects.h"
#include "FileLoader.h"
#include "WeaponEffects.h"
#include "taskcar.h"
#include "dummyobject.h"
#include "dummy.h"
#include "object.h"

#ifdef GTA_PC

#ifndef OSW
	#include "windows.h"
#endif

    #include "controllerconfig.h"
    #include "event.h"
	#include "win.h"
#endif

#include "GenericGameStorage.h"
#include "font.h"
#include "CutsceneMgr.h"
//#define ALIGNED_CAMERA_OFF_FOR_ANDRE
#include "vector.h"
#include "edit.h"
#include "timecycle.h"
#include "pickups.h"

#ifndef MASTER
	#include "gridref.h"
	#include "ps2keyboard.h"  // for keyboard FOV debug stuff
#endif

#include "PedIntelligence.h"
#include "Task.h"
#include "TaskTypes.h"
#include "TaskTypes.h"
#include "gamelogic.h"
#include "TaskJumpFall.h"
#include "TaskPlayer.h"
#include "TaskAttack.h"
#include "TaskCarAccessories.h"
#include "timebars.h"
#include "TaskGangs.h"

#include "objectdata.h"

#include "TouchInterface.h"

//!PC - no post fx for PC at the moment
#if defined (GTA_PS2)
#include "dma_post_fx.h"
extern CPostFx	postFx;
#endif // GTA_PS2

#ifdef USE_AUDIOENGINE
#include "AudioEngine.h"
#include "GameAudioEvents.h"
#endif

#include "streamingdebug.h"
#include "IKChain.h"
#include "SurfaceTable.h"

#ifndef FINAL // just incase I forget to comment this out!
//	#pragma optimization_level 0
//	#pragma dont_inline on
//	#pragma auto_inline off
#endif

#pragma optimize_for_size on
//#pragma auto_inline off

bool gbFirstPersonRunThisFrame = false; // D W- flags to renderer that we are in first person - so dont render our vehicle.

CHandShaker gHandShaker[MAX_HANDSHAKERS];

bool gPlayerPedVisible = true;

int8 gbCineyCamMessageDisplayed = 2; // number of times cinematic camera message is displayed

static int32 DirectionIsLooking = LOOKING_FORWARD; // Dw - to clean up later

static int32 gLastCamMode = -1; // DW - special vehicle alpha angle height considerations.

static uint32 gLastTime2PlayerCameraWasOK = 0;
static uint32 gLastTime2PlayerCameraCollided = 0;


CVector gTargetCoordsForLookingBehind; // DW - last minute fix

bool 	gAllowScriptedFixedCameraCollision = false; // for Andy Duthie... allows the scripted camera to process collision... last minute hacks.

bool	bDidWeProcessAnyCinemaCam = false;
// some tweak variables that are needed in more than one function
float fRangePlayerRadius = 0.50f;
float fCloseNearClipLimit = 0.15f;
//
float PLAYERPED_LEVEL_SMOOTHING_CONST_INV = 0.60f;
float PLAYERPED_TREND_SMOOTHING_CONST_INV = 0.80f;
//float PLAYERPED_LEVEL_SMOOTHING_CONST = 0.80f;
//float PLAYERPED_TREND_SMOOTHING_CONST = 0.60f;
float PLAYERFIGHT_LEVEL_SMOOTHING_CONST = 0.90f;//0.95f;
float PLAYERFIGHT_TREND_SMOOTHING_CONST = 0.50f;
//

//#define E3
const int MAX_NUM_VEHICLE_TYPE=5;


bool	CCamera::m_bUseMouse3rdPerson = false;  // set default to "TRUE"

#define MOTION_BLUR_ALPHA_DECREASE  (20)
#define CUT_SCENE_MOTION_BLUR		(40)

#define WIDESCREEN_EXTRA_FOV (16.0f)  // amount to change the FOV for in widescreen to get the right perspective

#define DISTCAM1 (20.0f)
#define DISTCAM2 (16.0f)
#define DISTCAM3 (30.0f)
#define DISTCAM5 (30.0f)
#define DISTCAM9 (15.0f)
#define DISTCAM10 (5.0f)
#define DISTCAM11 (20.0f)
#define DISTCAM12 (5.0f)

#define DISTCAM1_RELEASE (40.0f)
#define DISTCAM1_RELEASE_TOO_CLOSE (4.5f)
#define DISTCAM2_RELEASE (29.0f)
#define DISTCAM2_RELEASE_TOO_CLOSE (2.0f)
#define DISTCAM3_RELEASE (48.0f)
#define DISTCAM5_RELEASE (38.0f)
#define DISTCAM9_RELEASE (20.0f)
#define DISTCAM10_RELEASE (8.0f)
#define DISTCAM11_RELEASE (25.0f)
#define DISTCAM12_RELEASE (8.0f)

const int HELI_CAM_DIST_AWAY_ONE=  34.0f;
const int HELI_CAM_DIST_AWAY_TWO=  30.0f;
const int HELI_CAM_DIST_AWAY_THREE= 25.5f;
const int HELI_CAM_DIST_AWAY_FOUR=  23.0f;
const int HELI_CAM_DIST_AWAY_FIVE=  16.0f;


const int HELI_CAM_MAX_DIST_AWAY_ONE=  HELI_CAM_DIST_AWAY_ONE + 10.0f;
const int HELI_CAM_MAX_DIST_AWAY_TWO=  HELI_CAM_DIST_AWAY_TWO + 20.0f;
const int HELI_CAM_MAX_DIST_AWAY_THREE= HELI_CAM_DIST_AWAY_THREE + 25.0f;
const int HELI_CAM_MAX_DIST_AWAY_FOUR=  HELI_CAM_DIST_AWAY_FOUR + 34.0f;
const int HELI_CAM_MAX_DIST_AWAY_FIVE=  HELI_CAM_DIST_AWAY_FIVE + 20.0f;

const int HELI_CAM_DIST_TOO_CLOSE_ONE=  3.6f;
const int HELI_CAM_DIST_TOO_CLOSE_TWO=  3.0f;
const int HELI_CAM_DIST_TOO_CLOSE_THREE= 2.0f;
const int HELI_CAM_DIST_TOO_CLOSE_FOUR=  1.6f;
const int HELI_CAM_DIST_TOO_CLOSE_FIVE=  2.0f;

const UInt32 MAX_TIME_NO_INPUT_GOIDLE=120000; //in milliseconds  
const UInt32 MAX_TIME_IN_IDLE=18000; //in milliseconds
const UInt32 MAX_TIME_BEFORE_BACK_IN_IDLE=20000;//for when the user still hasn't touched the joystick and we are
										//are still in idle mode

//{Car, Bike, Heli, Plane, Boat}
float TiltTopSpeed[MAX_NUM_VEHICLE_TYPE]={0.035f, 0.035f, 0.001f, 0.005f, 0.035f};
float TiltSpeedStep[MAX_NUM_VEHICLE_TYPE]={0.016f, 0.016f, 0.0002f, 0.0014f, 0.016f};
float TiltOverShoot[MAX_NUM_VEHICLE_TYPE]={1.05f, 1.05f, 0.0f, 0.0f, 1.0f};
float ZOOM_ONE_DISTANCE[MAX_NUM_VEHICLE_TYPE]={-1.0f, -0.2f, -3.2f, 0.05f, -2.41f}; // was.25
float ZOOM_TWO_DISTANCE[MAX_NUM_VEHICLE_TYPE]={1.0f, 1.4f, 0.65f, 1.9f, 6.49f};//(1.5f)
float ZOOM_THREE_DISTANCE[MAX_NUM_VEHICLE_TYPE]={6.0f, 6.0f, 15.9f, 15.9f, 15.0f}; // was 2.9 default
float ZmOneAlphaOffset[MAX_NUM_VEHICLE_TYPE]=	{0.08f,  0.08f, 0.15f, 0.08f, 0.08f};//-0.117f};
float ZmTwoAlphaOffset[MAX_NUM_VEHICLE_TYPE]=	{0.07f,  0.08f, 0.30f, 0.08f, 0.08f};//0.17f;
float ZmThreeAlphaOffset[MAX_NUM_VEHICLE_TYPE]=	{0.055f, 0.05f, 0.15f, 0.06f, 0.08f};//0.500311f;


//extra offsets for the plane and rc heli
//float INIT_RC_HELI_HORI_EXTRA=6.0f;
//float INIT_RC_PLANE_HORI_EXTRA=9.5f;
//float INIT_RC_HELI_ALPHA_EXTRA=0.20f;
//float INIT_RC_PLANE_ALPHA_EXTRA=0.294991f;

/*			
float INIT_SYPHON_GROUND_DIST=0.768;//1.518;//2.419f;//1.37f;
float INIT_SYPHON_ALPHA_OFFSET=0.601;//0.104;//-0.052399;
float INIT_SYPHON_DEGREE_OFFSET=-0.70;//-0.4974;//-0.523658f;//0.3141;//DEGTORAD(18.0f);
float FrontOffsetSyphon=-0.445;//-0.445058;//DEGTORAD(0);
*/ // OLD VERSIONS (please leave this for me to comment back in, sandy)
float INIT_SYPHON_GROUND_DIST=2.419f;//1.37f;
float INIT_SYPHON_ALPHA_OFFSET=-0.052399;
float INIT_SYPHON_DEGREE_OFFSET=-0.523658f;//0.3141;//DEGTORAD(18.0f);
float FrontOffsetSyphon=-0.445058;//DEGTORAD(0);


float INIT_SYPHON_Z_OFFSET=-0.5f;

float MAX_ANGLE_BEFORE_AIMWEAPON_JUMPCUT = 30.0f;//90.0f;
bool TEST_FOLLOW_CAR_USING_FOLLOW_PED_CODE = true;

static float DrunkRotation = 0.0f;
static bool JustGoneIntoObbeCamera=false;

#ifndef MASTER
Int16	DebugCamMode = 0;
#endif

#define NEARLY_ONE 1.05f

extern CAutomobile* gpCar;
extern CColPoint gaTempSphereColPoints[PHYSICAL_MAXNOOFCOLLISIONPOINTS];




////////////////////////////////////////////////////////////
// The actual mem for the camera
CCamera TheCamera;


int8 gCinematicModeSwitchDir = 1;

float CCamera::m_fMouseAccelHorzntl;	// acceleration multiplier for 1st person controls
float CCamera::m_fMouseAccelVertical;	// acceleration multiplier for 1st person controls
float CCamera::m_f3rdPersonCHairMultX;
float CCamera::m_f3rdPersonCHairMultY;

Int32 CCam::CAM_BUMPED_SWING_PERIOD = 800;
Int32 CCam::CAM_BUMPED_END_TIME = 600;
float CCam::CAM_BUMPED_DAMP_RATE = 0.95f;
float CCam::CAM_BUMPED_MOVE_MULT = 0.1f;

// numbers for the top down camera
#define CAMTD_SPEEDMAXZOOMOUT (0.9f)
#define CAMTD_MAXLOOKAHEAD (18.0f)
#define CAMTD_MINDIST (30.0f)
#define CAMTD_MAXDIST (70.0f)
#define SNIPER_NEAR_CLIP (0.30f)
#define FIRSTPERSON_PED_NEAR_CLIP (0.40f)
#define FIRSTPERSON_CAR_NEAR_CLIP (0.25f)
#define CULLZONE_CAM_LATERALS_FAILED (0.7f)
#define NORMAL_NEAR_CLIP  (0.3f)//(0.90f)
#define NEAR_CLIP_IN_TUNNEL (0.9f)//(0.50f)
#define NEAR_CLIP_CAR_LOOK_OBSCURED (0.9f)
#define NEAR_CLIP_PED_VIEW_OBSCURED (0.05f)
#define MAX_WIDE_SCREEN_REDUCTION (30.0f)

#define BETA_DIFF_FOR_CUT_OFF_DOPPLER (0.30f)//angle in radians.
#define HEIGHT_OF_FIRE_ENGINE (3.026f)
//if it goes above this for one frame then will set just_switched to true



// defines for camera on foot distance
#define ZOOM_PED_ONE_DISTANCE 	(0.25f) // was.25
#define ZOOM_PED_TWO_DISTANCE 	(1.5f)
#define ZOOM_PED_THREE_DISTANCE (2.9f) // was 2.9 default

#define ZOOM_PED_ONE_Z_HEIGHT 	(-0.1483f) // was.25
#define ZOOM_PED_TWO_Z_HEIGHT 	(0.1595f)
#define ZOOM_PED_THREE_Z_HEIGHT (0.11075f) // was 2.9 default
#define ZOFFSET1RSTPERSONBOAT (0.5f)

#define	FADE_IN_TIME_INSTEAD_OF_IMMEDIATE (1.0f) //one second
#define FADE_OUT_TIME_INSTEAD_OF_IMMEDIATE (1.0f) //one second

#define PED_IN_WAY_HEIGHT_OFFSET_FOR_FOLLOW_PED (1.3f) 
#define PED_IN_WAY_HEIGHT_OFFSET_FOR_CAM_STRING_PED (1.0f) 
#define MAX_TIME_IN_FIRST_PERSON_NO_INPUT (2850.0f) //2.85 seconds
//#ifdef DEBUG
bool PrintDebugCode=TRUE;
//#endif
//#ifndef DEBUG
//bool PrintDebugCode=FALSE;
//#endif

//--------------------NEW CINEY CAM STUFF - DW --------------------------------
int32 gDWLastModeForCineyCam = -1;
//--------------------NEW CINEY CAM STUFF - DW --------------------------------


enum CarAlphaRange
	{
	RANGE_ONE, RANGE_TWO, RANGE_THREE, RANGE_FOUR 
	};


enum{IDLE_PAPS_BUM_CAM=0, NORMAL_CAM, IDLE_SEXY_CARS_CAM, NORMAL_CAM_AGAIN};


extern CAutomobile* gpCar;

static bool gbWhatAmILookingAt = false;

static float LesCam=30.0f;
static float TrainSpeedToGoIntoAaronCam=0.001f;
const float g_MaximumVerticalDisplacement=0.10f;
const float g_MaximumLateralDisplacement=0.18f;

static float AvoidTheWallsFraction=0.0f;	
static float AvoidTheWallsFractionSpeed=0.0f;






int32 gbCineyCamProcessedOnFrame = 0;

// DW - Supports all the Idle Cam Features.
class CIdleCam
{
	public :

		// Target Tracking
		CEntity *pTarget;
	
		// Slerp control
		CVector positionToSlerpFrom; // OK its not slerping because I'm not using quats, but terminology rightly or wrongly remains the same.
		float	timeSlerpStarted;
		float   slerpDuration;
		CVector lastIdlePos;		// A cached position that can be used anytime as the last position we where looking at to kick off a slerp from.
		float 	slerpTime;
			
		// Kick in control
		float 	timeControlsIdleForIdleToKickIn;
		float   timeIdleCamStarted;
		int32   lastFrameProcessed;

		// Target Selection
		float	timeLastTargetSelected;
		float   timeMinimumToLookAtSomething;

		// Collision
		float   timeTargetEntityWasLastVisible;
		float   timeToConsiderNonVisibleEntityAsOccluded;
		float 	distTooClose;
		float   distStartFOVZoom;
		float   distTooFar;
		int32 	targetLOSFramestoReject;
		int32 	targetLOSCounter;
		
		// FOV Control
		enum {ZOOMING_IN, ZOOMING_OUT, ZOOMED_IN, ZOOMED_OUT};
		int32 zoomState; // 
		float zoomFrom;
		float zoomTo;
		float timeZoomStarted;	
		float zoomNearest;	
		float zoomFarthest;	
		float curFOV;
		float durationFOVZoom;
		bool  bForceAZoomOut; // make the camera zoom out before reaquiring another target,
		bool  bHasZoomedIn;
		float timeBeforeNewZoomIn;
		float timeLastZoomIn;
		float increaseMinimumTimeFactorforZoomedIn;
		
		// Shake 
		float degreeShakeIdleCam;
		float shakeBuildUpTime;
		
		// new timer stuff
		int32 lastTimePadTouched;
		int32 idleTickerFrames;
		
		
		// Handy camera reference 
		CCam *pCam;
		
		CIdleCam() { Init(); }
		~CIdleCam() {}
		
		void Init();
		void Reset(bool bResetControls);
		void IdleCamGeneralProcess();
		bool Process();
		void Run();
		void ProcessTargetSelection();
		float ProcessSlerp(float *pAngX, float *pAngZ);
		void ProcessFOVZoom(float t);		
		bool IsItTimeForIdleCam();
		void GetLookAtPositionOnTarget(CEntity *pEnt, CVector *pVec);
		void SetTargetPlayer();
		void SetTarget(CEntity *pEntity);	
		void FinaliseIdleCamera(float curAngleX,float curAngleZ, float shakeDegree);	
		void VectorToAnglesRotXRotZ(CVector *pV,float *pA, float *pB);
		bool IsTargetValid(CEntity *pTest);
		void ProcessIdleCamTicker();

};


CIdleCam gIdleCam;




//-----------------------------------------------------------------------------------
// DW - Handy ASSERTS for debug
void CheckValidCam()
{
#ifndef FINAL
	RwFrame *cameraFrame;
	RwMatrix *cameraMatrix;

	cameraFrame = RwCameraGetFrame(TheCamera.m_pRwCamera);
	cameraMatrix = RwFrameGetMatrix(cameraFrame);

//	ASSERT(CMaths::Abs(cameraMatrix->pos.x) < 99999.0f);
//	ASSERT(CMaths::Abs(cameraMatrix->pos.y) < 99999.0f);
//	ASSERT(CMaths::Abs(cameraMatrix->pos.z) < 99999.0f);
	
	ASSERT(CMaths::Abs(cameraMatrix->at.x) <= NEARLY_ONE);
	ASSERT(CMaths::Abs(cameraMatrix->at.y) <= NEARLY_ONE);
	ASSERT(CMaths::Abs(cameraMatrix->at.z) <= NEARLY_ONE);

	ASSERT(CMaths::Abs(cameraMatrix->up.x) <= NEARLY_ONE);
	ASSERT(CMaths::Abs(cameraMatrix->up.y) <= NEARLY_ONE);
	ASSERT(CMaths::Abs(cameraMatrix->up.z) <= NEARLY_ONE);

	ASSERT(CMaths::Abs(cameraMatrix->right.x) <= NEARLY_ONE);
	ASSERT(CMaths::Abs(cameraMatrix->right.y) <= NEARLY_ONE);
	ASSERT(CMaths::Abs(cameraMatrix->right.z) <= NEARLY_ONE);
#endif	
}

static CVector vecPedPosEst(0.0f, 0.0f, 10000.0f);
static CVector vecPedPosTrend(0.0f, 0.0f, 0.0f);

/////////////////////////////////
// DW - Main processing of camera
void CCam::Process()
{
	gIdleCam.IdleCamGeneralProcess(); // must be run to prevent interesting events from continually running.

	CVector		ThisCamsTarget;
	float		SpeedVarDesired=0.0f;
	float		Multiplier=0.0f, Length=0.0f;
	float		FrontX=0.0f, FrontY=0.0f, SpeedX=0.0f, SpeedY=0.0f;
	float		TargetOrientation=0.0f;

	if (CamTargetEntity==NULL)
	{
		TIDYREF(CamTargetEntity, &CamTargetEntity);
		CamTargetEntity=TheCamera.pTargetEntity;
		REGREF(CamTargetEntity, &CamTargetEntity);
	}

	ASSERTMSG(CamTargetEntity!=NULL, "CamTargetEntity is NULL");

	// remove target recticle immediately for heat-seeking missiles, but not
	// for normal weapn targetting which fades out gradually
	if(gCrossHair[0].m_type == TARGET_FLIGHT)
	{
		// If player is in the Harrier (or any other vehicle which may produce the TARGET_FLIGHT
		// crosshair) then don't clear the cross-hair...
		CPlayerPed * player = FindPlayerPed();
		if(player && player->m_pMyVehicle && player->m_pMyVehicle->GetModelIndex() == MODELID_PLANE_HARRIER)
		{
		
		}
		else {
		
			CWeaponEffects::ClearCrossHairImmediately(0);
		}
	}

	//optoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptopt
	//this is an optimiser for the camera for all the line of sight stuuf - DW - it seems to only modify near clip
	m_iFrameNumWereAt++;
	if (m_iFrameNumWereAt>m_iDoCollisionCheckEveryNumOfFrames)
		m_iFrameNumWereAt=1;
	m_bCollisionChecksOn = (m_iFrameNumWereAt==m_iDoCollisionChecksOnFrameNum);
	//optoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptoptopt

	if (m_bCamLookingAtVector==0)
	{
		// We're not on foot
		if (CamTargetEntity->GetIsTypeVehicle())
		{	
			ThisCamsTarget = CamTargetEntity->GetPosition();
			// Calculate the beta we would get right behind the car.
			if (CamTargetEntity->GetMatrix().xy == 0.0f && CamTargetEntity->GetMatrix().yy == 0.0f)
			{
				TargetOrientation = 0.0f;
			}
			else
			{
				TargetOrientation = CGeneral::GetATanOfXY(CamTargetEntity->GetMatrix().xy, CamTargetEntity->GetMatrix().yy);
			}
		
			// We calculate the speed thing. This is used by most camera modes
			// to make changes depending on the speed the player is going at.
			// Make sure we only take the speed component in the direction of the car
			CVector TempFront(0,0,0);
			TempFront.x = CamTargetEntity->GetMatrix().xy;
			TempFront.y = CamTargetEntity->GetMatrix().yy;
			TempFront.Normalise();
			Length = CMaths::Sqrt(TempFront.x*TempFront.x + TempFront.y*TempFront.y);
			if (Length == 0.0f)
			{
				ASSERTMSG(0, "Oops get Mark something bad has happened");
			}
			else
			{
				TempFront.x /= Length;
				TempFront.y /= Length;
			}
			
			SpeedX = TempFront.x * ((CVehicle*) CamTargetEntity)->GetMoveSpeed().x;
			SpeedY = TempFront.y * ((CVehicle*) CamTargetEntity)->GetMoveSpeed().y;
		
			if (SpeedX + SpeedY > 0.0f)
			{
				SpeedVarDesired = MIN(1.0f, CMaths::Sqrt( SpeedX * SpeedX + SpeedY * SpeedY ) / CAMTD_SPEEDMAXZOOMOUT);
			}
			else
			{
				SpeedVarDesired = -MIN(0.5f, CMaths::Sqrt( SpeedX * SpeedX + SpeedY * SpeedY ) / (CAMTD_SPEEDMAXZOOMOUT*2.0f));
			}
		
			Multiplier = 0.895f;//( 0.96f, CTimer::GetTimeStep());
			//Multiplier = CMaths::Pow( 0.96f, CTimer::GetTimeStep());
			SpeedVar = (1.0f - Multiplier)*SpeedVarDesired + Multiplier*SpeedVar;
			
			if(DirectionWasLooking!=LOOKING_FORWARD && !(CPad::GetPad(0)->GetLookBehindForCar()
			&& !CPad::GetPad(0)->GetLookLeft() && !CPad::GetPad(0)->GetLookRight()))
			{
				TheCamera.m_bCamDirectlyBehind=true;
			}
		}
		else
		{
			// ok want the smooth the camera's target position when it's aiming at the playerped
			if(CamTargetEntity == (CEntity *)FindPlayerPed())
			{
				CVector vecTargetPedPos = FindPlayerPed()->GetPosition();
				if(FindPlayerPed()->GetPedIntelligence()->GetTaskClimb())
				{
					FindPlayerPed()->GetPedIntelligence()->GetTaskClimb()->GetCameraTargetPos(FindPlayerPed(), vecTargetPedPos);
				}

				// using a Linear Exponential Smoothing method (Holt's Method)
				CVector vecNewPosEstimate, vecNewTrendEstimate;
				float fTimeScaledLevelSmooth, fTimeScaledTrendSmooth;
				
//				if(FindPlayerPed()->GetPedIntelligence()->GetTaskFighting())
//				{
//					fTimeScaledLevelSmooth = 1.0f - CMaths::Pow(PLAYERFIGHT_LEVEL_SMOOTHING_CONST_INV, CTimer::GetTimeStep());
//					fTimeScaledTrendSmooth = 1.0f - CMaths::Pow(PLAYERFIGHT_TREND_SMOOTHING_CONST_INV, CTimer::GetTimeStep());
//				}
//				else
//				{
//					fTimeScaledLevelSmooth = 1.0f - CMaths::Pow(PLAYERPED_LEVEL_SMOOTHING_CONST_INV, CTimer::GetTimeStep());
//					fTimeScaledTrendSmooth = 1.0f - CMaths::Pow(PLAYERPED_TREND_SMOOTHING_CONST_INV, CTimer::GetTimeStep());
//				}	
				// if playerped has moved too far from the previous estimated position
				// or timestep was too small (ie coming from a pause, or something else has fucked time)
				// in the last frame - want to reset smoothing variables and use current position
				//
				// also don't want to do buffering for 3rdPerson Mouse Control method cause looks shit when strafing
				//
				if( (vecPedPosEst - vecTargetPedPos).MagnitudeSqr() > 9.0f
				|| CTimer::GetTimeStep() < 0.2f || Using3rdPersonMouseCam() || TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront)
				{
					vecNewPosEstimate = vecTargetPedPos;
					vecNewTrendEstimate = CVector(0.0f, 0.0f, 0.0f);
				}
				else if(FindPlayerPed()->GetPedIntelligence()->GetTaskFighting() && Mode==CCam::MODE_AIMWEAPON)
				{
					fTimeScaledLevelSmooth = CMaths::Pow(PLAYERFIGHT_LEVEL_SMOOTHING_CONST, CTimer::GetTimeStep());
					vecNewPosEstimate = fTimeScaledLevelSmooth*vecPedPosEst + (1.0f - fTimeScaledLevelSmooth)*vecTargetPedPos;
					vecNewTrendEstimate = CVector(0.0f, 0.0f, 0.0f);
				}
				else
				{
					fTimeScaledLevelSmooth = 1.0f - CMaths::Pow(PLAYERPED_LEVEL_SMOOTHING_CONST_INV, CTimer::GetTimeStep());
					fTimeScaledTrendSmooth = 1.0f - CMaths::Pow(PLAYERPED_TREND_SMOOTHING_CONST_INV, CTimer::GetTimeStep());
					// estimate of new position is a weighted average of the new actual position
					// and the sum of the old estimated position and the estimated trend (or velocity)
					vecNewPosEstimate = fTimeScaledLevelSmooth*vecTargetPedPos + (1.0f - fTimeScaledLevelSmooth)*(vecPedPosEst + vecPedPosTrend*CTimer::GetTimeStep());
					vecNewPosEstimate.z = vecTargetPedPos.z;
					// a new estimate of the trend is then calculated as a weighted average
					// of the old trend, and the difference between the new and old position estimates
					vecNewTrendEstimate = fTimeScaledTrendSmooth*(vecNewPosEstimate - vecPedPosEst)/CMaths::Max(1.0f, CTimer::GetTimeStep()) + (1.0f - fTimeScaledTrendSmooth)*vecPedPosTrend;
					vecNewTrendEstimate.z = 0.0f;
				}
				
				// the camera's target can then be given as the smoothed estimate of the player ped's position
				ThisCamsTarget = vecNewPosEstimate;
				// and the current estimates of position and trend saved for the next frame
				vecPedPosEst = vecNewPosEstimate;
				vecPedPosTrend = vecNewTrendEstimate;
			}
			else
			{
				ThisCamsTarget = CamTargetEntity->GetPosition();
			}
				
				// Calculate the beta we would get right behind the ped.
			if (CamTargetEntity->GetMatrix().xy == 0.0f && 
				CamTargetEntity->GetMatrix().yy == 0.0f)
			{
				TargetOrientation = 0.0f;
			}
			else
			{
				TargetOrientation = CGeneral::GetATanOfXY(CamTargetEntity->GetMatrix().xy, CamTargetEntity->GetMatrix().yy);
			}
		
			SpeedVar = SpeedVarDesired = 0.0f;	
		}
		/////end of specific processing
	}
	else
		ThisCamsTarget=m_cvecCamFixedModeVector;

#ifndef FINAL
	if(CPlayerPed::bDebugPlayerInfo)
	{
		sprintf(gString, "CamMode = %d\n", Mode);
		CDebug::PrintToScreenCoors(gString, 35,2);
	}
#endif

	DirectionWasLooking = DirectionIsLooking;
	DirectionIsLooking=LOOKING_FORWARD; // bug fix - test - DW - seems to fix look behind shite	
	
#ifndef MASTER
	CheckValidCam();
#endif	
	
	// =====================================================================================================
	// DW - Determine look behind state before processing normal camera ( which must always be processed )
	if  (&TheCamera.Cams[TheCamera.ActiveCam]==this)
	{	
		bool bNotTransitioning = TheCamera.m_uiTransitionState == TheCamera.TRANS_NONE;
		
		if (((Mode==MODE_CAM_ON_A_STRING)||(Mode==MODE_1STPERSON)||(Mode==MODE_BEHINDBOAT)||(Mode==MODE_BEHINDCAR))&&(CamTargetEntity->GetIsTypeVehicle()))
		{
			//want to disable look left and right for helicopters / planes (cause L2/R2 buttons are flight controls)
			bool bIgnoreLeftRight = (	CamTargetEntity && CamTargetEntity->GetIsTypeVehicle() &&
										( ((CVehicle*)CamTargetEntity)->GetVehicleAppearance()==APR_HELI || ((CVehicle*)CamTargetEntity)->GetVehicleAppearance()==APR_PLANE)
									);
			if(CPad::GetPad(0)->GetLookBehindForCar())// && bNotTransitioning)
			{
				TheCamera.m_uiTransitionState = TheCamera.TRANS_NONE;	
			    TheCamera.m_vecDoingSpecialInterPolation=false;
				TheCamera.m_bWaitForInterpolToFinish=false;				
				
				if (DirectionWasLooking!=LOOKING_BEHIND)
				 	TheCamera.m_bJust_Switched=TRUE;
				DirectionIsLooking=LOOKING_BEHIND;
			}
			else if(CPad::GetPad(0)->GetLookLeft() && !bIgnoreLeftRight)// && bNotTransitioning)
			{
				TheCamera.m_uiTransitionState = TheCamera.TRANS_NONE;		
			    TheCamera.m_vecDoingSpecialInterPolation=false;
				TheCamera.m_bWaitForInterpolToFinish=false;						
					
				if (DirectionWasLooking!=LOOKING_LEFT)
				 	TheCamera.m_bJust_Switched=TRUE;
				DirectionIsLooking=LOOKING_LEFT;
			}
			else if(CPad::GetPad(0)->GetLookRight() && !bIgnoreLeftRight)// && bNotTransitioning)
			{
				TheCamera.m_uiTransitionState = TheCamera.TRANS_NONE;	
			    TheCamera.m_vecDoingSpecialInterPolation=false;
				TheCamera.m_bWaitForInterpolToFinish=false;						
						
				if (DirectionWasLooking!=LOOKING_RIGHT)
				 	TheCamera.m_bJust_Switched=TRUE;
				DirectionIsLooking=LOOKING_RIGHT;
			
			}
			else
			{
				if (DirectionWasLooking!=LOOKING_FORWARD)
				 	TheCamera.m_bJust_Switched=TRUE;
				DirectionIsLooking=LOOKING_FORWARD;
			}
		}				
		else if ((Mode==MODE_FOLLOWPED)&&(CamTargetEntity->GetIsTypePed()))///only enable look behind in follow the ped mode   //now do it for a ped
		{
			if (CPad::GetPad(0)->GetLookBehindForPed())
			{
				if (DirectionWasLooking!=LOOKING_BEHIND && bNotTransitioning)
				 	TheCamera.m_bJust_Switched=TRUE;
				DirectionIsLooking=LOOKING_BEHIND;
			}
			else
			{
				if (DirectionWasLooking!=LOOKING_FORWARD) // messily preserved cos I dont know how the code works as a whole - DW
				 	gCurDistForCam = 1.0f;				// A SPECIAL CASE - DW
				DirectionIsLooking=LOOKING_FORWARD;
			}
		}
		else if(Mode == MODE_AIMWEAPON)
		{
			if (DirectionWasLooking!=LOOKING_FORWARD) // messily preserved cos I dont know how the code works as a whole - DW
			 	gCurDistForCam = 1.0f;				// A SPECIAL CASE - DW
			DirectionIsLooking=LOOKING_FORWARD;
		}
	}	
	
	// DW - If we just switched its safe to push the camera back out to the furthest out position
	if (TheCamera.m_bJust_Switched)
	{
		gCurDistForCam = 1.0f;
		TheCamera.m_bResetOldMatrix = true;
	}


	if (Mode != MODE_BEHINDCAR && Mode != MODE_CAM_ON_A_STRING && Mode != MODE_BEHINDBOAT
	&& Mode != MODE_1STPERSON && Mode != MODE_TWOPLAYER_IN_CAR_AND_SHOOTING)
	{
		CPostEffects::SpeedFXDisableForThisFrame();	// switch this off goddamit!
	}	

	gbFirstPersonRunThisFrame = false;

	// =====================================================================================================
	switch(Mode)
	{
		case MODE_BEHINDCAR:	
		case MODE_CAM_ON_A_STRING:
		case MODE_BEHINDBOAT:
			Process_FollowCar_SA(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;			
		case MODE_FOLLOWPED:
			{
#ifdef GTA_PC
			static bool USE_FOLLOWPED_SA_WITH_MOUSE = true;
			if(TheCamera.m_bUseMouse3rdPerson && !USE_FOLLOWPED_SA_WITH_MOUSE)
				Process_FollowPedWithMouse(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);	
			else
#endif		
			//Process_FollowPed(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			Process_FollowPed_SA(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
			}
#ifndef MASTER			
		case MODE_DEBUG:
			Process_Debug(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
#endif			
		case MODE_SNIPER:
		case MODE_CAMERA:
/*			if(TheCamera.m_bUseMouse3rdPerson)
		        Process_1rstPersonPedOnPC(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			else
*/
//				Process_Sniper(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
				Process_M16_1stPerson(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
		case MODE_ROCKETLAUNCHER:
/*			if(TheCamera.m_bUseMouse3rdPerson)
		        Process_1rstPersonPedOnPC(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			else
*/
				Process_Rocket(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired, false);
			break;
		case MODE_ROCKETLAUNCHER_HS:
				Process_Rocket(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired, true);

			break;
		case MODE_M16_1STPERSON:
		case MODE_HELICANNON_1STPERSON:
/*				if(TheCamera.m_bUseMouse3rdPerson)
		        Process_1rstPersonPedOnPC(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			else
*/
				Process_M16_1stPerson(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
#ifndef MASTER			
		case MODE_MODELVIEW:
			Process_ModelView(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
#endif			
		case MODE_AIMWEAPON:
		case MODE_AIMWEAPON_FROMCAR:
		case MODE_AIMWEAPON_ATTACHED:
			Process_AimWeapon(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;			
		case MODE_WHEELCAM:
			Process_WheelCam(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
		case MODE_FIXED:
			Process_Fixed(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
#ifdef CAMERA_USE_DEAD_TARGET_FIXED_CAM
		case MODE_SPECIAL_FIXED_FOR_SYPHON:
			Process_SpecialFixedForSyphon(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
#endif
		case MODE_1STPERSON:
			Process_1stPerson(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
		case MODE_FLYBY:
			//if (TheCamera.GetPositionAlongSpline()>0.6)
			//{TheCamera.LoadPathSplines("tsplin2");}
			Process_FlyBy(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
		case MODE_PED_DEAD_BABY:
			ProcessPedsDeadBaby();
			TheCamera.m_bPlayerIsInGarage = false; // DW - stops mad shit happening when killed in garage.
			TheCamera.m_bJustCameOutOfGarage = false;
			break;
		case MODE_TWOPLAYER:
			Process_Cam_TwoPlayer();
			break;
		case MODE_TWOPLAYER_IN_CAR_AND_SHOOTING:
			Process_Cam_TwoPlayer_InCarAndShooting();
			break;
		case MODE_TWOPLAYER_SEPARATE_CARS:
			Process_Cam_TwoPlayer_Separate_Cars();
			break;
		case MODE_TWOPLAYER_SEPARATE_CARS_TOPDOWN:
			Process_Cam_TwoPlayer_Separate_Cars_TopDown();
			break;
	 //   case MODE_1RSTPERSON_PED_PC:
#ifdef GTA_PC	 
		case MODE_EDITOR:
			Process_Editor(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
	  	case MODE_SNIPER_RUNABOUT:
	  	case MODE_ROCKETLAUNCHER_RUNABOUT:
	  	case MODE_ROCKETLAUNCHER_RUNABOUT_HS:
	  	case MODE_1STPERSON_RUNABOUT:  
		case MODE_M16_1STPERSON_RUNABOUT:
		case MODE_FIGHT_CAM_RUNABOUT: 
	        Process_1rstPersonPedOnPC(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
	        break;
#endif			

#ifdef GTA_IDLECAM		
		case MODE_PILLOWS_PAPS:
			Process_CushyPillows_Arse();	
			break;
#endif			
		case MODE_ARRESTCAM_ONE:
			ProcessArrestCamOne();
			break;
		case MODE_ARRESTCAM_TWO:
			ASSERT(0);
//			ProcessArrestCamTwo();
			break;
		case MODE_ATTACHCAM:
			Process_AttachedCam();
			break;
		case MODE_DW_HELI_CHASE:
			Process_DW_HeliChaseCam(false);
			break;			
		case MODE_DW_CAM_MAN:
			Process_DW_CamManCam(false);
			break;			
		case MODE_DW_BIRDY:
			Process_DW_BirdyCam(false);
			break;			
		case MODE_DW_PLANE_SPOTTER:
			Process_DW_PlaneSpotterCam(false);
			break;			
		case MODE_DW_DOG_FIGHT:
			Process_DW_DogFightCam(false);
			break;			
		case MODE_DW_FISH:
			Process_DW_FishCam(false);
			break;		
		case MODE_DW_PLANECAM1:
			Process_DW_PlaneCam1(false);
			break;		
		case MODE_DW_PLANECAM2:
			Process_DW_PlaneCam2(false);
			break;		
		case MODE_DW_PLANECAM3:
			Process_DW_PlaneCam3(false);
			break;		
										
		default:
			ASSERTMSG(0, "Invalid camera type");
			// Just pick any coordinates just now.
			Source = CVector(0.0f, 0.0f, 0.0f);
			Front = CVector(0.0f, 1.0f, 0.0f);
			Up = CVector(0.0f, 0.0f, 1.0f);
			break;

		// Please find old cases at bottom of file - DW
	}

#ifndef MASTER
	CheckValidCam();
#endif	

#ifndef FINAL
	static bool gbDisplayAlphaBeta = false;
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_J) && PS2Keyboard.GetKeyDown(PS2_KEY_CTRL))
		gbDisplayAlphaBeta = !gbDisplayAlphaBeta;

	if (gbDisplayAlphaBeta)
	{
		char str[255];
		sprintf(str,"Alpha (%3.3f) Beta (%3.3f)",Alpha,Beta);
		VarConsole.AddDebugOutput(str);
	}
#endif

	if (Mode<MODE_DW_HELI_CHASE || Mode > MODE_DW_PLANECAM3)
	{
		gDWLastModeForCineyCam = -1;
	}

	gLastCamMode = Mode;

	CVector VecForBetaAlpha=Source - m_cvecTargetCoorsForFudgeInter; 
	float DistOnGround=CMaths::Sqrt(VecForBetaAlpha.x * VecForBetaAlpha.x + VecForBetaAlpha.y * VecForBetaAlpha.y);
	m_fTrueBeta=CGeneral::GetATanOfXY(VecForBetaAlpha.x, VecForBetaAlpha.y);				
	m_fTrueAlpha=CGeneral::GetATanOfXY(DistOnGround,  VecForBetaAlpha.z);	

	if (TheCamera.m_uiTransitionState == TheCamera.TRANS_NONE)
	{
		KeepTrackOfTheSpeed(Source, m_cvecTargetCoorsForFudgeInter, Up,  m_fTrueAlpha, m_fTrueBeta, FOV);
	}

/*
	char Str[200];
	sprintf(Str, "Cam Mode %u", Mode);
	OutputDebugString(Str);
	sprintf(Str, "Transition State %u", TheCamera.m_uiTransitionState);
	OutputDebugString(Str);
*/

	// DW - Process looking back etc.
	LookingBehind = LookingLeft = LookingRight = false;
	SourceBeforeLookBehind=Source;	
	
	if  (&TheCamera.Cams[TheCamera.ActiveCam]==this)
	{	
		switch (DirectionIsLooking)
		{
			case LOOKING_BEHIND :
				LookBehind();
				break;
			case LOOKING_LEFT :
				LookRight(false);
				break;
			case LOOKING_RIGHT :
				LookRight(true);
				break;
			default:
				break;
		}

		DirectionWasLooking = DirectionIsLooking;
	}

#ifndef MASTER
	CheckValidCam();
#endif	

	if (TheCamera.m_bFOVScript)
	{
		FOV = TheCamera.m_MyFOV;
		TheCamera.m_bFOVScript = false;
	}
	
	if (TheCamera.m_bVectorMoveScript)
	{
		Source = TheCamera.m_VectorMoveScript;
		TheCamera.m_bVectorMoveScript = false;
	}

	if (TheCamera.m_bVectorTrackScript)
	{
		Front = TheCamera.m_VectorTrackScript - Source;
		Front.Normalise();
		GetVectorsReadyForRW();
		
//		ThisCamsTarget = TheCamera.m_VectorTrackScript;
		TheCamera.m_bVectorTrackScript = false;
	}				
	// Redundant clipping ped in front - see end of file	
}



























bool bIsLampPost(int32 idx)
{
	return (idx == MI_SINGLESTREETLIGHTS1
		||	idx == MI_SINGLESTREETLIGHTS2
		||	idx == MI_SINGLESTREETLIGHTS3
		||	idx == MI_BOLLARDLIGHT
		||	idx == MI_MLAMPPOST
		||	idx == MI_STREETLAMP1
		||	idx == MI_STREETLAMP2
		||	idx == MI_TELPOLE02
		||	idx == MI_TRAFFICLIGHTS_MIAMI
		||	idx == MI_TRAFFICLIGHTS_TWOVERTICAL
		||	idx == MI_TRAFFICLIGHTS_3
		||	idx == MI_TRAFFICLIGHTS_4
		||	idx == MI_TRAFFICLIGHTS_GAY
		||	idx == MI_TRAFFICLIGHTS_5);
}
// disabling of entities
#ifdef ALLOW_OBJECT_DISABLING
	bool gObjectDisableFileLoaded = false;
	uint8 gDisableObjectBuffer[LOAD_BUF_SIZE];
	bool gbDisableFileValid = false;
#endif

#ifndef MASTER

	char *gpMaxFileForLookAtOrStandOn = NULL;
	char *gpArtistForLookAtOrStandOn = NULL;
	bool gbLoadedArtistDffAssociationFile = false;
	#define MAX_SIZE_ART_DFF_ASS 	 100
	#define MAX_STR_SIZE_ART_DFF_ASS 50 
	char gArtDffAss_Artist[MAX_SIZE_ART_DFF_ASS][MAX_STR_SIZE_ART_DFF_ASS];
	char gArtDffAss_Dff[MAX_SIZE_ART_DFF_ASS][MAX_STR_SIZE_ART_DFF_ASS];
	int32 gNumArtDffAss = 0;
	#define ARTDFFASS  "ARTDFFASS.TXT" 
	#define BUF_SIZE_FOR_ART_DFF_ASS (1024*10) // 10K - must be ok for stack!
	#define TERMINATOR_TOKEN "TERMINATE_FILE_READ"
//-----------------------------------------------------
// Searches a file for the artist associated with a DFF
char* FindArtistForDff(Char *pDffName)
{
	if (!gbLoadedArtistDffAssociationFile)
	{
		// load this file now we know we want it.		
//		char buf[BUF_SIZE_FOR_ART_DFF_ASS]; // on stack quickie. - rough and ready
		char *pBuf = (char*)GtaMalloc(BUF_SIZE_FOR_ART_DFF_ASS);
		char *pBufOriginal = pBuf;
		CFileMgr::SetDir("");
		int32 fd = CFileMgr::LoadFile(ARTDFFASS, (unsigned char *)pBuf);						
		if (fd>=0)
		{
			gbLoadedArtistDffAssociationFile = true;
			int bytesRead;
			char str[255];
			while(sscanf(pBuf,"%s %n",&str,&bytesRead) == 1 && strcmp(str,TERMINATOR_TOKEN) )
			{
				strcpy(gArtDffAss_Dff[gNumArtDffAss],str);
				pBuf += bytesRead;
				int ret = sscanf(pBuf,"%s %n",&gArtDffAss_Artist[gNumArtDffAss],&bytesRead);
				ASSERT(ret>=0);
				pBuf += bytesRead;
				gNumArtDffAss++;
				ASSERT(gNumArtDffAss<MAX_SIZE_ART_DFF_ASS);
			}
		}
		else
		{
			// handy thing... create a file with all the dff files listed
			CFileMgr::SetDir("");
			int32 fdWrite = CFileMgr::OpenFileForWriting(ARTDFFASS);
			if (fdWrite>=0)
			{
				CIDEFileDataDebug *pD = gIDEFilesDataDebug.m_IdeFileData;	
				for (int32 j=0;j<gIDEFilesDataDebug.m_numFiles;j++)
				{
					char str[255];
					sprintf(str,"%s NO_ARTIST_ASSIGNED_SEE_AARON\n",pD[j].m_fn);
					int32 res = CFileMgr::Write(fdWrite, str, strlen(str)+2);
					ASSERT(res);
				}				
				CFileMgr::CloseFile(fdWrite);
			}									
		}
		GtaFree(pBufOriginal);
	}
	
	int32 i = 0;
	while (i <= gNumArtDffAss)
	{
		if (!strcmp(pDffName,gArtDffAss_Dff[i]))
			return gArtDffAss_Artist[i];
		i++;			
	}		
	return NULL;
}
#endif
	
///////////////////////////////////////////////////////////////////////////
// NAME       : WellBufferMe(()
// PURPOSE    : generic buffering function 
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
void WellBufferMe(float TheTarget, float* TheValueToChange, float* ValueSpeedSoFar, float TopSpeed, float SpeedStep, bool IsAnAngle)
{
	float DeltaChange=0.0f;		
	float ReqSpeed;

	DeltaChange=TheTarget-(*TheValueToChange); 		

	if (IsAnAngle==true)
	{
		DeltaChange=TheTarget-(*TheValueToChange); 
		while (DeltaChange>=(PI)) {DeltaChange-=2.0f * PI;}
		while (DeltaChange<(-PI)) {DeltaChange+=2.0f * PI;}
	}
	
	ReqSpeed = DeltaChange * TopSpeed;
		
	
	if (ReqSpeed - (*ValueSpeedSoFar) > 0.0f)///This can be in a positive or negatve direction
									/// we are going slower than we want to.INCREASE THE SPEED!!!!!
	{
		(*ValueSpeedSoFar) += SpeedStep * CMaths::Abs(ReqSpeed - (*ValueSpeedSoFar)) * CTimer::GetTimeStep();//// increase the speed
	}
	else ///we are going faster than we want. Slow down or we're all going to die. Aiiiieeee. 
	{
		(*ValueSpeedSoFar) -= SpeedStep * CMaths::Abs(ReqSpeed - (*ValueSpeedSoFar)) * CTimer::GetTimeStep();//decrease the speed 
	}


	if ((ReqSpeed < 0.0f) && ((*ValueSpeedSoFar) < ReqSpeed)) //checks incase Beta speed in now too low
	{
		(*ValueSpeedSoFar) = ReqSpeed; ///sets it to the limit
	}
	else if ((ReqSpeed > 0.0f) && ((*ValueSpeedSoFar) > ReqSpeed)) //checks incase Beta speed in now too high
	{
		(*ValueSpeedSoFar) = ReqSpeed;
	}

	(*TheValueToChange) += (*ValueSpeedSoFar) * MIN(10.0f, CTimer::GetTimeStep() );

}

//-----------------------------------------------------------------------------------
void MakeAngleLessThan180(float &AngleToChange)
{
	while (AngleToChange>=(PI)) {AngleToChange-=2.0f * PI;}
	while (AngleToChange<(-PI)) {AngleToChange+=2.0f * PI;}
}

///////////////////////////////////////////////////////////////////////////
// PURPOSE    : Makes sure Alpha ranges from 0 to <360.
inline void CCam::ClipAlpha(void)
{

	while (Alpha>=(2.0f * PI)) {Alpha-=2.0f * PI;}
 	while (Alpha<(0.0f)) {Alpha+=2.0f * PI;}
}

///////////////////////////////////////////////////////////////////////////
// PURPOSE    : Makes sure Beta ranges from 0 to <360.
inline void CCam::ClipBeta(void)
{
//	while (Beta>=(2.0f * PI)) {Beta-=2.0f * PI;}
// 	while (Beta<(0.0f)) {Beta+=2.0f * PI;}
	if(Beta > PI)	Beta -= TWO_PI;
	else if(Beta < -PI)	Beta += TWO_PI;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : GetBoatLook_L_R_HeightOffset
// PURPOSE    : Goes into the handling data and fills in the variable for what the
//			  :	height should be  			
// RETURNS    : true or false for sucess or fail
bool CCam::GetBoatLook_L_R_HeightOffset(float &HeightOffset)
{
	int nModelIndex=0;
	if (CamTargetEntity)
	{	
		nModelIndex=CamTargetEntity->GetModelIndex();
	}
	else
	{
		return false;
	}
	CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(nModelIndex);
	ASSERT(pModelInfo);
	ASSERT(pModelInfo->GetModelType() == MI_TYPE_VEHICLE);
	int32 nHandlingIndex = pModelInfo->GetHandlingId();

	tBoatHandlingData* pBoatHandlingData=mod_HandlingManager.GetBoatPointer(nHandlingIndex);
	if 	(pBoatHandlingData!=NULL)
	{
		HeightOffset= pBoatHandlingData->Look_L_R_CamHeight;
		return true;
	}
	else
	{
		return false;
	}
}

///////////////////////////////////////////
void CCam::GetVectorsReadyForRW()
{
	CVector TempRight;
	Up = CVector(0.0f, 0.0f, 1.0f);
	Front.Normalise();
	if ((Front.x==0.0f) && (Front.y==0.0f))
	{
		Front.x=0.0001f;
		Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( Front, Up );
	TempRight.Normalise();
	Up = CrossProduct( TempRight, Front );
}

///////////////////////////////////////////
bool CCam::GetWeaponFirstPersonOn()
{
	if (CamTargetEntity!=NULL) 
		if (CamTargetEntity->GetIsTypePed())  
	   		return ((CPed*)CamTargetEntity)->GetWeapon()->GetFirstPersonWeaponMode();

	return false;
}

/////////////////////////////////////////////////////
// NAME       : Init()
// PURPOSE    : Chooses top down as default camera
void CCam::Init()
{
	gIdleCam.Reset(true); // must be rest every load!
//	ResetFirstPersonUpCastFixer(); // not required for PC version - DW
	
	Mode = MODE_FOLLOWPED;
	//Source = CVector (0.0f, 0.0f, 20.0f);
	Front = CVector (0.0f, 0.0f, -1.0f);
	Up = CVector (0.0f, 0.0f, 1.0f);
		//uses clipping plane of 0.9f
	Rotating=false;
	m_iDoCollisionChecksOnFrameNum=1; 
	m_iDoCollisionCheckEveryNumOfFrames=9;
	m_iFrameNumWereAt=0;	
	m_bCollisionChecksOn=true;
	m_fRealGroundDist=0.0f;
	BetaSpeed=0.0f;
	AlphaSpeed=0.0f;
	DistanceSpeed=0.0f;
	m_fCameraHeightMultiplier=0.75f;
	f_max_role_angle=DEGTORAD(2.0f);//(10.0f);//DEGTORAD(5.0f);
	Distance = 30.0f;
	DistanceSpeed = 0.0f;
	m_pLastCarEntered=NULL;
	m_pLastPedLookedAt=NULL;
    ResetStatics=TRUE;
	Beta = 0.0f;
	m_fTilt=0.0f;
	m_fTiltSpeed=0.0f;

	//Used in the Follow the ped camera mode
	m_bFixingBeta=FALSE;
	CA_MIN_DISTANCE=0.0f;
	CA_MAX_DISTANCE=0.0f;
	LookingBehind=false;
	LookingLeft=false;
	LookingRight=false;
 	m_fPlayerInFrontSyphonAngleOffSet=DEGTORAD(20.0f);
	m_fSyphonModeTargetZOffSet=0.50f;
	m_fRadiusForDead=1.5f;
	DirectionWasLooking=LOOKING_FORWARD;
	
	LookBehindCamWasInFront=false;
	f_Roll=0.0f; //These two floating fuckers are used for adding a slight roll to the camera in the
				///camera on a string mode
	f_rollSpeed=0.0f;
	m_fCloseInPedHeightOffset=0.0f;
	m_fCloseInPedHeightOffsetSpeed=0.0f;
	
	m_fCloseInCarHeightOffset=0.0f;
	m_fCloseInCarHeightOffsetSpeed=0.0f;
	
	m_fPedBetweenCameraHeightOffset=0.0f;

	m_fTargetBeta=0.0f;
	m_fBufferedTargetBeta=0.0f;
	m_fBufferedTargetOrientation=0.0f;
	m_fBufferedTargetOrientationSpeed=0.0f;

	
	m_fDimensionOfHighestNearCar=0.0f;
	
	Beta_Targeting = 0.0f;
	X_Targetting = Y_Targetting = 0.0f;
	CarWeAreFocussingOn = 0;
	CarWeAreFocussingOnI = 0.0f;
	
	m_fCamBumpedHorz = 1.0f;
	m_fCamBumpedVert = 0.0f;
	m_nCamBumpedTime = 0;

	CVector vecInit(0.0f,0.0f,0.0f);
	for(int i=0; i<CAM_NUM_TARGET_HISTORY; i++)
	{
		m_aTargetHistoryPos[i] = vecInit;
		m_nTargetHistoryTime[i] = 0;
	}

	m_nCurrentHistoryPoints = 0;
	
	// I'm assuming this gets called when the player starts new game / loads game
	gPlayerPedVisible = true;
	gbCineyCamMessageDisplayed = 2;
	DirectionIsLooking = LOOKING_FORWARD;
	gLastCamMode = -1;

	gLastTime2PlayerCameraWasOK = 0;
	gLastTime2PlayerCameraCollided = 0;

	TheCamera.m_bForceCinemaCam = false;
	
	//gAllowScriptedFixedCameraCollision = false; // musnt be reset!
}

////////////////////////////////////////////////////////////////////////////
//NAME        : Look Behind
//PURPOSE     : Reverses the cameras Front Vector
bool CCam::LookBehind(void)
{
	CColPoint colPoint;
	CVector TestVector;
	CVector TargetCoors;
	
	
	bool bIsCar 	= (((Mode==MODE_CAM_ON_A_STRING)||(Mode==MODE_BEHINDBOAT)||(Mode==MODE_BEHINDCAR))&&(CamTargetEntity->GetIsTypeVehicle()));
	bool bIsPed 	= CamTargetEntity->GetIsTypePed();
	bool bIsFPCar 	= (Mode==MODE_1STPERSON && CamTargetEntity->GetIsTypeVehicle());

	if (!bIsCar && !bIsFPCar && !bIsPed)
		return false;


	TargetCoors=CamTargetEntity->GetPosition();
	Front = CamTargetEntity->GetPosition() - Source;

	if (bIsCar)
	{
		TargetCoors=gTargetCoordsForLookingBehind;
	
		LookingBehind=true;
		
		bool ViewNotClear=FALSE;
		float GroundDistance=0.0f;

		if (Mode==MODE_CAM_ON_A_STRING)
		{
			GroundDistance=CA_MAX_DISTANCE;
		}
		else ///in a boat
		{
			GroundDistance=15.5f;
		}

/*
		CVector CarForward=CamTargetEntity->GetMatrix().GetForward();
		CarForward.Normalise();
		float CarAngle=CGeneral::GetATanOfXY(CarForward.x, CarForward.y);
		float Diff=CarAngle-Beta;
		while (Diff>=(PI)) {Diff-=2.0f * PI;}
		while (Diff<(-PI)) {Diff+=2.0f * PI;}

		if (DirectionWasLooking!=LOOKING_BEHIND)//rightho we've just gone into looking behind
		{
			if ((Diff>DEGTORAD(-90))&&(Diff<DEGTORAD(90)))
			{
				LookBehindCamWasInFront=false;
			}
			else
			{
				LookBehindCamWasInFront=true;
			}
		}

		if (LookBehindCamWasInFront==true)
		{
			CarAngle=(CarAngle+PI);
		}


		Source.x=TargetCoors.x + (GroundDistance*CMaths::Cos(CarAngle));
		Source.y=TargetCoors.y + (GroundDistance*CMaths::Sin(CarAngle));
*/
		
//		if(DirectionWasLooking!=LOOKING_BEHIND)
//		{
//			if(DotProduct(Front, CamTargetEntity->GetMatrix().GetForward()) > 0.0f)
//				LookBehindCamWasInFront=false;
//			else
//				LookBehindCamWasInFront=true;
//		}
		
		Source = CamTargetEntity->GetMatrix().GetForward();
//		if(LookBehindCamWasInFront==true)
//			Source *= -1.0f;
		
		Source.z += 0.2f;
		Source = TargetCoors + GroundDistance*Source;

	
		CVector TempSource=Source;
//		float VehicleLength = CamTargetEntity->GetColModel().GetBoundBoxMax().Magnitude2D();
		CVector TempTarget=TargetCoors;
//		CVector TempFront=Source-TargetCoors;
//		TempFront.Normalise();
//		TempTarget+=VehicleLength*TempFront;



#ifdef DW_CAMERA_COLLISION
		CWorld::pIgnoreEntity = CamTargetEntity;
		TheCamera.CameraGenericModeSpecialCases(NULL);
		TheCamera.CameraVehicleModeSpecialCases((CVehicle*)CamTargetEntity);		
		TheCamera.CameraColDetAndReact(&Source,&TempTarget);	

		Front =  CamTargetEntity->GetPosition() - Source;
		GetVectorsReadyForRW();

		TheCamera.ImproveNearClip((CVehicle*)CamTargetEntity,NULL,&Source,&TempTarget);						
		CWorld::pIgnoreEntity = NULL;
#else
		TheCamera.AvoidTheGeometry(TempSource,TempTarget, Source, FOV);
#endif		
	}

	if(bIsFPCar)
	{
		LookingBehind=true;
		float DistanceInFront=0.25f;
	//	FOV=35.0f;
		RwCameraSetNearClipPlane(Scene.camera, 0.05f); //for occasions when the spaz user is right up against a wall etc
		
		Front=CamTargetEntity->GetMatrix().GetForward();
		Front.Normalise();
		if (((CVehicle*)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
		{
			Source.z-=ZOFFSET1RSTPERSONBOAT;
		}
		
		
 		if(((CVehicle*)CamTargetEntity)->GetVehicleAppearance()==APR_BIKE)
 		{
 			static float gDistanceInFront = 1.1f;
 			DistanceInFront=gDistanceInFront;

 		/*	if(((CVehicle*)CamTargetEntity)->pDriver)
 			{
 				CVector vecBikeFwdTemp(0.0f,0.0f,0.0f);
				vecBikeFwdTemp = CamTargetEntity->GetPosition();
 				((CVehicle*)CamTargetEntity)->pDriver->GetBonePosition(vecBikeFwdTemp, BONETAG_HEAD, true);
 				vecBikeFwdTemp += ((CVehicle*)CamTargetEntity)->m_vecMoveSpeed*CTimer::GetTimeStep() - CamTargetEntity->GetPosition();
 				DistanceInFront += 0.2f + MAX(0.0f, DotProduct(vecBikeFwdTemp, CamTargetEntity->GetMatrix().GetForward()));
 			}*/
	
			DistanceInFront = 2.3f; // hack - DW - for bug fix

			// hack - DW
			Source+=DistanceInFront*Front;
			//extern float FTrunc(float n, int digits = 4);
			//Source = CamTargetEntity->GetPosition() + DistanceInFront*Front;

			Front=-Front;

	/*		static int digs = 2;
			Source.x = FTrunc(Source.x,digs);
			Source.y = FTrunc(Source.y,digs);
			Source.z = FTrunc(Source.z,digs);


			Front.x = FTrunc(Front.x,digs);
			Front.y = FTrunc(Front.y,digs);
			Front.z = FTrunc(Front.z,digs);
*/
			GetVectorsReadyForRW();

/*
			char str[255];
			sprintf(str,"Src %f %f %f",Source.x,Source.y,Source.z);
			VarConsole.AddDebugOutput(str, TRUE);

			sprintf(str,"Front %f %f %f",Front.x,Front.y,Front.z);
			VarConsole.AddDebugOutput(str, TRUE);

			sprintf(str,"TargetPos %f %f %f",CamTargetEntity->GetPosition().x,CamTargetEntity->GetPosition().y,CamTargetEntity->GetPosition().z);
			VarConsole.AddDebugOutput(str, TRUE);

			sprintf(str,"TargetFrnt %f %f %f",CamTargetEntity->GetMatrix().GetForward().x,CamTargetEntity->GetMatrix().GetForward().y,CamTargetEntity->GetMatrix().GetForward().z);
			VarConsole.AddDebugOutput(str, TRUE);
*/
		}
 		else if(((CVehicle*)CamTargetEntity)->GetVehicleAppearance()==APR_HELI)
 		{
 			Front = -1.0f*CamTargetEntity->GetMatrix().GetUp();
 			Up = CamTargetEntity->GetMatrix().GetForward();
			Source+=DistanceInFront*Front;
 		}
 		else
 		{
			Source+=DistanceInFront*Front;
			Front=-Front;
 		}
	}
	
	if (bIsPed)
	{
//		float LocalBeta;
//		const float DistAway=4.5f;
//		CVector SourceToTarget;
//		SourceToTarget=Source-TargetCoors;
//		LocalBeta=CGeneral::GetATanOfXY(SourceToTarget.x , SourceToTarget.y);
//		Source=TargetCoors + DistAway*CVector(CMaths::Cos(LocalBeta + PI), CMaths::Sin(LocalBeta + PI), 0);
//		Source.z+=1.15f;

//		Source.x = -1.0f*(Source.x - TargetCoors.x) + TargetCoors.x;
//		Source.y = -1.0f*(Source.y - TargetCoors.y) + TargetCoors.y;
//		Source.z = TargetCoors.z + 1.15f;

		static float extraPedLookBackDist = 2.0f;	// 0.5f	

		Source = CVector(-CMaths::Cos(Beta), -CMaths::Sin(Beta), 0.0f);
		Source.z -= DotProduct(((CPed *)CamTargetEntity)->m_vecGroundNormal, Source);
		Source.z += 0.3f;
		Source.Normalise();
		Source = Source*MAX(0.6f, (TheCamera.m_fPedZoomSmoothed + extraPedLookBackDist)) + TargetCoors;
		CVector TestTarget=TargetCoors;
		TestTarget.z=Source.z;//make level
		CVector TempSource=Source;
		
		// DW - make look behind a bit higher - rather than looking at the players crotch when collision occurs.				
		// DW - for near look behind its too low down...
/*		static float gLookBackNearZModSource[3]		= {0.7f,0.3f,0.0f};
		static float gLookBackNearZModTarget[3]		= {0.7f,1.0f,1.0f};		

		static float gLookBackZModColModSource[3] 	= {0.5f,0.5f,0.5f}; // mulitplier which scale the extra heights for camera undergoing collision
		static float gLookBackZModColModTarget[3] 	= {0.5f,0.5f,0.5f}; // 1 is no modification 0.0 is no delta

		static float gScaleWhenLookingBackSwimming[3] 			= {-4.0f,-3.0f,-2.0f};
		static float gExtraHeightWhenLookingBackSwimming[3] 	= {-0.5f,0.0f,0.0f};
*/

		static float gLookBackNearZModSource[3]		= {0.6f,0.0f,0.0f};
		static float gLookBackNearZModTarget[3]		= {0.6f,0.6f,0.6f};		

		static float gLookBackZModColModSource[3] 	= {0.5f,0.5f,0.5f}; // mulitplier which scale the extra heights for camera undergoing collision
		static float gLookBackZModColModTarget[3] 	= {0.5f,0.5f,0.5f}; // 1 is no modification 0.0 is no delta

		static float gScaleWhenLookingBackSwimming[3] 			= {-1.0f,-1.0f,-1.0f};
		static float gExtraHeightWhenLookingBackSwimming[3] 	= {0.0f,1.0f,1.0f};


		float scaleOfCollision	= gCurDistForCam;
		int idx 				= TheCamera.m_nPedZoom-1;
		float zDeltaSrc 		= gLookBackNearZModSource[idx];
		float zDeltaDst 		= gLookBackNearZModTarget[idx];
		float zColModSrc 		= gLookBackZModColModSource[idx]; 		
		float zColModDst 		= gLookBackZModColModTarget[idx];			

		if (((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskSwim())
		{
			float scale 		= gScaleWhenLookingBackSwimming[idx];
			float extraHeight 	= gExtraHeightWhenLookingBackSwimming[idx];
		
			CVector ToTarget = TargetCoors-Source;
			Source = TargetCoors + ToTarget*scale;
			Source.z += extraHeight;
		}

		Source.z 		+= zDeltaSrc;// * DW_LERP(scaleOfCollision,zColModSrc,1.0f);
		TargetCoors.z 	+= zDeltaDst;// * DW_LERP(scaleOfCollision,zColModDst,1.0f);	

		//==============================================================
		// DW - HANDLE DUCKING - and transtitions
		TheCamera.HandleCameraMotionForDucking((CPed*)CamTargetEntity,&Source,&TargetCoors);
		// DW - END HANDLE DUCKING
		//==============================================================	

		
#ifdef DW_CAMERA_COLLISION
		// Ped follow cam	
		TheCamera.CameraGenericModeSpecialCases((CPed*)CamTargetEntity);
		TheCamera.CameraPedModeSpecialCases((CPed*)CamTargetEntity);	
		TheCamera.CameraColDetAndReact(&Source,&TargetCoors);	

		Front=TargetCoors-Source;
		GetVectorsReadyForRW();
		
		TheCamera.ImproveNearClip(NULL,(CPed*)CamTargetEntity,&Source,&TargetCoors);						
		
		float nc = RwCameraGetNearClipPlane(Scene.camera);
		if (TheCamera.m_nPedZoom==ZOOM_ONE && nc > 0.05f)
		{			
			RwCameraSetNearClipPlane(Scene.camera, 0.05f); // to prevent player running into camera when looking behind ( a common problem apparently )			
		}				
#else		
		TheCamera.AvoidTheGeometry(TempSource,TargetCoors, Source, FOV);
#endif
	}

	GetVectorsReadyForRW(); // bug fix - DW

	return true;
}


////////////////////////////////////////////////////////////////////////////
float BOAT_1STPERSON_LR_OFFSETZ = 0.2f;
float BOAT_1STPERSON_L_OFFSETX = 0.7f;
float BOAT_1STPERSON_R_OFFSETX = 0.3f;


bool CCam::LookLeft(void)
{
	return LookRight(false);
}

////////////////////////////////////////////////////////////////////////////
bool CCam::LookRight(bool bIsRight)
{
	bool bIsCar 	= (((Mode==MODE_CAM_ON_A_STRING)||(Mode==MODE_BEHINDBOAT)||(Mode==MODE_BEHINDCAR))&&(CamTargetEntity->GetIsTypeVehicle()));
	bool bIsPed 	= CamTargetEntity->GetIsTypePed();
	bool bIsFPCar 	= (Mode==MODE_1STPERSON && CamTargetEntity->GetIsTypeVehicle());

	float signForLeftRightSwitch = 1.0f;
	
	if (!bIsRight)
	{
		LookingLeft=true;	
		signForLeftRightSwitch *= -1.0f;
	}
	else
	{
		LookingRight=true;
	}

	if (!bIsCar && !bIsFPCar && !bIsPed)
		return false;

	if (bIsCar)
	{
		CVector TargetCoors;
		CColPoint colPoint;

		float Offset=DEGTORAD(90);
		
		bool ViewNotClear=FALSE;
		TargetCoors = CamTargetEntity->GetPosition();

		//float GroundDistance=CMaths::Sqrt(Front.x * Front.x + Front.y * Front.y);
		float GroundDistance=0.0f;
		if (Mode==MODE_CAM_ON_A_STRING)
		{
			GroundDistance=CA_MAX_DISTANCE;
		}
		else if (Mode==MODE_BEHINDBOAT) ///in a boat
		{
			GroundDistance=9.0f;
			float BoatLookLeftAndRightOffset=0.0f;
			if (GetBoatLook_L_R_HeightOffset(BoatLookLeftAndRightOffset) && !CCullZones::Cam1stPersonForPlayer())
			{
				Source.z=TargetCoors.z + BoatLookLeftAndRightOffset;
			}
		}
		else
		{
			GroundDistance=9.0f;
		}
		

		CVector CarForward=CamTargetEntity->GetMatrix().GetForward();
		CarForward.Normalise();
		float CarAngle=CGeneral::GetATanOfXY(CarForward.x, CarForward.y);

		Offset *= signForLeftRightSwitch;

		Source.x=TargetCoors.x + (GroundDistance*CMaths::Cos(CarAngle+Offset));
		Source.y=TargetCoors.y + (GroundDistance*CMaths::Sin(CarAngle+Offset));
	
		CColModel &TargetColModel = CamTargetEntity->GetColModel();
		CVector TempSource = Source;
		CVector TempTarget = TargetCoors;
		float fOriginalHeight = Source.z;
		float VehicleWidth = TargetColModel.GetBoundBoxMax().x;
		VehicleWidth=ABS(VehicleWidth);
//		CVector TempFront=Source-TargetCoors;
//		TempFront.Normalise();
//		TempTarget+=VehicleWidth*TempFront;

#ifdef DW_CAMERA_COLLISION
		CWorld::pIgnoreEntity = CamTargetEntity;
		TheCamera.CameraGenericModeSpecialCases(NULL);
		TheCamera.CameraVehicleModeSpecialCases((CVehicle*)CamTargetEntity);
		TheCamera.CameraColDetAndReact(&Source,&TempTarget);			
		CWorld::pIgnoreEntity = NULL;
#else
		TheCamera.AvoidTheGeometry(TempSource,TempTarget, Source, FOV);
#endif

		// try and keep the camera at a min height above the vehicle (but not higher than it was alreay)
		CVector vecRightSideHeight = CamTargetEntity->GetPosition();

		if (!bIsRight)
			// try and keep the camera at a min height above the vehicle (but not higher than it was alreay)
			vecRightSideHeight += CamTargetEntity->GetMatrix().GetRight()*TargetColModel.GetBoundBoxMax().x;
		else
			vecRightSideHeight += CamTargetEntity->GetMatrix().GetRight()*TargetColModel.GetBoundBoxMin().x;

		vecRightSideHeight += CamTargetEntity->GetMatrix().GetUp()*TargetColModel.GetBoundBoxMax().z;
		float fLimitHeight = MIN(fOriginalHeight, MAX(m_cvecTargetCoorsForFudgeInter.z, vecRightSideHeight.z) + 0.1f);
		Source.z = MAX(fLimitHeight, Source.z);

// aghh - this bit does NOTHING cause the Front vector is recalculated somewhere later in the code, damnit
		Front =  CamTargetEntity->GetPosition() - Source;
		Front.z += 1.1f;
		if (Mode==MODE_BEHINDBOAT)
		{
			Front.z += 1.2f;
		}
//////////////////////////////////////////

		GetVectorsReadyForRW();
	}
	else if (bIsFPCar)
	{
		if (!bIsRight)
			LookingLeft=true;
		else 
			LookingRight=true;
			
			
		static float fpClipInCar = 0.05f;
		RwCameraSetNearClipPlane(Scene.camera, fpClipInCar); //for occasions when the spaz user is right up against a wall etc
		if (((CVehicle*)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
		{
			if(((CVehicle *)CamTargetEntity)->pDriver)
			{
				CVector vecShoulderHeight(0.0f,0.0f,0.0f);
				CPed *pPed = ((CVehicle *)CamTargetEntity)->pDriver;
				/////////////////////////////////////
				pPed->SetPedPositionInCar();
				pPed->UpdateRwMatrix();
				pPed->UpdateRwFrame();
				pPed->UpdateRpHAnim();
				/////////////////////////////////////
				pPed->GetBonePosition(vecShoulderHeight, BONETAG_NECK, true);
				
				if (bIsRight)	
					vecShoulderHeight += BOAT_1STPERSON_L_OFFSETX*CamTargetEntity->GetMatrix().GetRight();						
				else
					vecShoulderHeight += BOAT_1STPERSON_R_OFFSETX*CamTargetEntity->GetMatrix().GetRight();
				vecShoulderHeight += BOAT_1STPERSON_LR_OFFSETZ*CamTargetEntity->GetMatrix().GetUp();
				Source = vecShoulderHeight;
			}
			else
				Source.z-=ZOFFSET1RSTPERSONBOAT;
		}

		if (((CVehicle*)CamTargetEntity)->GetBaseVehicleType()!=VEHICLE_TYPE_BIKE)
		{
			Source -= 0.35f*CamTargetEntity->GetMatrix().GetRight();
		}

		Up=CamTargetEntity->GetMatrix().GetUp();
		Up.Normalise();
		Front=CamTargetEntity->GetMatrix().GetForward();
		Front.Normalise();
		
		if (bIsRight)
			Front=CrossProduct(Front, Up);
		else
			Front=CrossProduct(Up,Front);
					
		Front.Normalise();
 	
 		if (((CVehicle*)CamTargetEntity)->GetVehicleAppearance()==APR_BIKE)
 		{
 			Source-=1.45f*Front;
 		}
 	}
 	
 	return true;
}

///////////////////////////////////////////
void CCam::KeepTrackOfTheSpeed(const CVector &TheSource, const CVector &TheTargetToLookAt, const CVector &TheUpVector, const float &TrueAlpha, const float &TrueBeta, const float &TheFOV)
{
	static CVector PreviousSource = TheSource;
	static CVector PreviousTarget = TheTargetToLookAt;	
	static CVector PreviousUp = TheUpVector;

	static float PreviousBeta = TrueBeta;				
	static float PreviousAlpha = TrueAlpha;	
	static float PreviousFov = TheFOV;

	if (TheCamera.m_bJust_Switched)
	{
		PreviousSource = TheSource;
		PreviousTarget = TheTargetToLookAt;	
		PreviousUp = TheUpVector;
	}
	
	m_cvecSourceSpeedOverOneFrame = TheSource-PreviousSource;
	m_cvecTargetSpeedOverOneFrame = TheTargetToLookAt-PreviousTarget;
	m_cvecUpOverOneFrame=TheUpVector-PreviousUp;
	m_fFovSpeedOverOneFrame=TheFOV-PreviousFov; 
	m_fBetaSpeedOverOneFrame=TrueBeta-PreviousBeta;
	MakeAngleLessThan180(m_fBetaSpeedOverOneFrame);
	m_fAlphaSpeedOverOneFrame=TrueAlpha-PreviousAlpha;
	MakeAngleLessThan180(m_fAlphaSpeedOverOneFrame);

	PreviousSource=TheSource;
	PreviousTarget=TheTargetToLookAt;	
	PreviousUp=TheUpVector;
	PreviousBeta=TrueBeta;	
	PreviousAlpha=TrueAlpha;
	PreviousFov=TheFOV;
}


// truncate the precision of a floating point number, by a number of decimal digits.
float FTrunc(float n, int digits)
{

// Rounds a number to a specified number of digits.
// Number is the number you want to round.
// Num_digits specifies the number of digits to which you want 
// to round number.
// If num_digits is greater than 0, then number is rounded 
// to the specified number of decimal places.
// If num_digits is 0, then number is rounded to the nearest integer.
// Examples
//        ROUND(2.15, 1)        equals 2.2
//        ROUND(2.149, 1)        equals 2.1
//        ROUND(-1.475, 2)    equals -1.48

    float doComplete5i, doComplete5(n * powf(10.0f, (float) (digits + 1)));
    
    if(n < 0.0f)
        doComplete5 -= 5.0f;
    else
        doComplete5 += 5.0f;
    
    doComplete5 /= 10.0f;
    modff(doComplete5, &doComplete5i);

	float result = doComplete5i / powf(10.0f, (float) digits);  

    return result;
}

void VecTrunc(CVector *pV, int digits = 4)
{
	pV->x = FTrunc(pV->x,digits);
	pV->y = FTrunc(pV->y,digits);
	pV->z = FTrunc(pV->z,digits);
}

//
//float AIMWEAPON_DEFAULT_ALPHA = -0.22f;
//float AIMWEAPON_INCAR_DEFAULT_ALPHA = -0.16f;
float AIMWEAPON_STICK_SENS = 0.007f;
float AIMWEAPON_TARGET_SENS = 0.1f;
float AIMWEAPON_FREETARGET_SENS = 0.1f;
float AIMWEAPON_DRIVE_SENS_MULT = 0.25f;
float AIMWEAPON_DRIVE_CLOSE_ENOUGH = DEGTORAD(10.0f);

float AIMWEAPON_RIFLE1_ZOOM = 50.0f;
float AIMWEAPON_RIFLE2_ZOOM = 35.0f;

enum
{
	AIMWEAPON_ONFOOT=0,
	AIMWEAPON_ONBIKE,
	AIMWEAPON_INCAR,
	AIMWEAPON_MELEE
};

#define AIMWEAPON_BASE_DIST 0
#define AIMWEAPON_ANGLE_DIST 1
#define AIMWEAPON_ANGLE_FALLOFF 2
#define AIMWEAPON_DEFAULT_ALPHA 3
#define AIMWEAPON_DEFAULT_Z 4
#define AIMWEAPON_ROTMAX_UP 5
#define AIMWEAPON_ROTMAX_DOWN 6


float AIMWEAPON_SETTINGS[AIMWEAPON_MELEE+1][AIMWEAPON_ROTMAX_DOWN+1] =
{
	//  dist	angle_d	angle_f	alpha		z		max_up				max_down
	{	1.0f,	1.6f,	1.0f,	-0.12f,		0.0f,	DEGTORAD(45.0f),	DEGTORAD(89.0f)},
	{	3.5f,	0.7f,	1.0f,	-0.16f,		0.2f,	DEGTORAD(35.0f),	DEGTORAD(70.0f)},
	{	6.0f,	0.7f,	1.0f,	-0.16f,		0.4f,	DEGTORAD(35.0f),	DEGTORAD(70.0f)},
	{	2.5f,	0.7f,	1.0f,	-0.12f,		0.15f,	DEGTORAD(45.0f),	DEGTORAD(45.5f)}
};

static bool ACQUIRED_FREEAIM_DIRECTION = false;
static int32 ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = 0;
static float ACQUIRED_FREEAIM_PEDHEADING = -1001.0f;
// this is used for double-tap reverse direction in vehicles
static uint32 FREEAIM_INCAR_TARGET_TAP_TIME = 0;
int32 ACQUIRED_FREEAIM_STATIC_LIM_A = 5000;
int32 ACQUIRED_FREEAIM_STATIC_LIM_B = 2000;

TweakFloat fTweakFOV = 1.1;//1.05
TweakFloat fMouseAvoidGeomReturnRate = 0.92f;
TweakFloat fTweakPedAimDirn = -0.05f;
TweakFloat AIMWEAPON_FOV_ZOOM_RATE = 1.0f;

TweakFloat AIMWEAPON_EXTRA_MELEE_TARGETING_ALPHA = 3.0f;
TweakFloat AIMWEAPON_EXTRA_MELEE_TARGETING_BETA = 20.0f;
TweakFloat AIMWEAPON_EXTRA_MELEE_FIGHTING_ALPHA = 0.0f;
TweakFloat AIMWEAPON_EXTRA_MELEE_FIGHTING_BETA = 70.0f;
TweakFloat AIMWEAPON_EXTRA_MELEE_ANGLE_RATE = 0.96f;
//
#pragma mark ----aim-weapon----------------
//
void CCam::Process_AimWeapon(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired)
{
	// make this static so we can smooth it for melee weapons
	static CVector vecWeaponTargetPos(0.0f,0.0f,0.0f);
	CVector vecIdealSource, vecTargetCoords;
	float fStickX, fStickY;
	float StickAlphaOffset, StickBetaOffset;
     
	if(!CamTargetEntity->GetIsTypePed())
		return;
	
	if(!((CPed *)CamTargetEntity)->IsPlayer())
		return;
	
	CPed *pPed = (CPed *)CamTargetEntity;
	CWeaponInfo *pWeaponInfo = NULL;
	if(pPed->GetPedIntelligence()->GetTaskUseGun())
		pWeaponInfo = pPed->GetPedIntelligence()->GetTaskUseGun()->GetWeaponInfo();
	else
		pWeaponInfo = CWeaponInfo::GetWeaponInfo(pPed->GetWeapon()->GetWeaponType());

	int nAimType = AIMWEAPON_ONFOOT;
	if(pPed->m_nPedFlags.bInVehicle)
	{
		if(pPed->m_pMyVehicle && (pPed->m_pMyVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE
		|| pPed->m_pMyVehicle->GetVehicleType()==VEHICLE_TYPE_QUADBIKE))
			nAimType = AIMWEAPON_ONBIKE;
		else
			nAimType = AIMWEAPON_INCAR;
	}
	else if(pPed->GetPedIntelligence()->GetTaskJetPack())
		nAimType = AIMWEAPON_ONBIKE;
	else if(pPed->GetWeapon()->IsTypeMelee())
		nAimType = AIMWEAPON_MELEE;

	
	float fDesiredFOV = 70.0f;
	if(pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_AK47
	|| pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_M4)
		fDesiredFOV = AIMWEAPON_RIFLE1_ZOOM;
	else if(pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_COUNTRYRIFLE)
		fDesiredFOV = AIMWEAPON_RIFLE2_ZOOM;

	if(TheCamera.m_uiTransitionState==CCamera::TRANS_NONE)
	{
		if(ResetStatics && pPed->GetWeapon()->GetWeaponType()!=WEAPONTYPE_COUNTRYRIFLE)
		{
			//if(pPed->GetWeapon()->GetWeaponType()!=WEAPONTYPE_COUNTRYRIFLE) // see bug fix below - urgh nasty - DW
				FOV = fDesiredFOV;
		}
		else 
		{
//			float fRate = CMaths::Pow(AIMWEAPON_FOV_ZOOM_RATE, CTimer::GetTimeStep());
//			FOV = fRate*FOV + (1.0f - fRate)*fDesiredFOV;
			float fRate = AIMWEAPON_FOV_ZOOM_RATE*CTimer::GetTimeStep();
			if(fDesiredFOV > FOV + fRate)
				FOV += fRate;
			else if(fDesiredFOV < FOV - fRate)
				FOV -= fRate;
			else
				FOV = fDesiredFOV;
		}
	}
/*
	if(pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_COUNTRYRIFLE)
	{ // DW - Bug fix - The FOV for some unknown reason doesnt like zooming really quick with the countryside rifle... so I interpolate it for a few frames so it appears instantaneous when zooming in ( which is what seems to be desired here )
		static float gFOVRate = 0.60f;
		float dFOV = FOV-fDesiredFOV;
		if (dFOV>0.1f)
			FOV -= dFOV * gFOVRate;
		else
			FOV = fDesiredFOV;
	}
*/	
	// target recticle is off to the side 
	float fAspectR = CDraw::GetAspectRatio();
	// I know this is off by a bit, but just to get it working ok!
	float fAimAngleBeta, fAimAngleAlpha;

	static float FIGHT_AIM_TOWARD_TARGET = 0.0f;
	if(pWeaponInfo->GetWeaponFireType()==FIRETYPE_MELEE)
	{
		static float FIGHT_AIM_ANGLE_ALPHA = AIMWEAPON_EXTRA_MELEE_TARGETING_ALPHA;
		static float FIGHT_AIM_ANGLE_BETA = AIMWEAPON_EXTRA_MELEE_TARGETING_BETA;
		float fDesFightAimAngleAlpha = AIMWEAPON_EXTRA_MELEE_TARGETING_ALPHA;
		float fDesFightAimAngleBeta = AIMWEAPON_EXTRA_MELEE_TARGETING_BETA;
		float fDesFightAimTowardTarget = 0.0f;
		if(pPed->GetPedIntelligence()->GetTaskFighting() && pPed->GetMoveState() < PEDMOVE_WALK
		&& pPed->GetWeaponLockOnTarget())
		{
			static float sAimWeaponMeleeLOSCounter = 0.0f;
			if(sAimWeaponMeleeLOSCounter > CTimer::GetTimeStep())
				sAimWeaponMeleeLOSCounter -= CTimer::GetTimeStep();
			else if(sAimWeaponMeleeLOSCounter < -CTimer::GetTimeStep())
				sAimWeaponMeleeLOSCounter += CTimer::GetTimeStep();
			else
			{
				CVector vecTestPos = pPed->GetPosition() + CVector(0.0f,0.0f,0.75f);
				CVector vecOffset = CrossProduct(pPed->GetWeaponLockOnTarget()->GetPosition() - pPed->GetPosition(), CVector(0.0f,0.0f,1.0f));
				vecOffset *= 2.0f / CMaths::Max(0.7f, vecOffset.Magnitude());
				
				if(CWorld::GetIsLineOfSightClear(vecTestPos, vecTestPos + vecOffset, true, true, false, true, false, true, true))
					sAimWeaponMeleeLOSCounter = 100.0f;
				else
					sAimWeaponMeleeLOSCounter = -100.0f;
			}
		
			if(sAimWeaponMeleeLOSCounter >= 0.0f)
			{
				fDesFightAimAngleAlpha = AIMWEAPON_EXTRA_MELEE_FIGHTING_ALPHA;
				fDesFightAimAngleBeta = AIMWEAPON_EXTRA_MELEE_FIGHTING_BETA;
				fDesFightAimTowardTarget = 1.0f;
			}
		}
		
		if(ResetStatics)
		{
			FIGHT_AIM_ANGLE_ALPHA = fDesFightAimAngleAlpha;
			FIGHT_AIM_ANGLE_BETA = fDesFightAimAngleBeta;
			FIGHT_AIM_TOWARD_TARGET = 0.0f;
		}
		// don't swing the camera around when it's transitioning
		else if(TheCamera.m_uiTransitionState==CCamera::TRANS_NONE)
		{
			float fRate = CMaths::Pow(AIMWEAPON_EXTRA_MELEE_ANGLE_RATE, CTimer::GetTimeStep());
			FIGHT_AIM_ANGLE_ALPHA = fRate*FIGHT_AIM_ANGLE_ALPHA + (1.0f - fRate)*fDesFightAimAngleAlpha;
			FIGHT_AIM_ANGLE_BETA = fRate*FIGHT_AIM_ANGLE_BETA + (1.0f - fRate)*fDesFightAimAngleBeta;
			FIGHT_AIM_TOWARD_TARGET = fRate*FIGHT_AIM_TOWARD_TARGET + (1.0f - fRate)*fDesFightAimTowardTarget;
		}
		
		fAimAngleAlpha = FIGHT_AIM_ANGLE_ALPHA;
		fAimAngleBeta = FIGHT_AIM_ANGLE_BETA;


		fAimAngleBeta = DEGTORAD(fAimAngleBeta);
		fAimAngleAlpha = DEGTORAD(fAimAngleAlpha);
	}
	else
	{
		float fScreenAngle, fScreenPosMult;
		
		fScreenAngle = DEGTORAD(0.5f*FOV);
		fScreenPosMult = 2.0f*(CCamera::m_f3rdPersonCHairMultX - 0.5f);
		fAimAngleBeta = CMaths::ATan(fScreenPosMult*CMaths::Tan(fScreenAngle));

		fScreenPosMult = 2.0f*(0.5f - CCamera::m_f3rdPersonCHairMultY);
		fAimAngleAlpha = CMaths::ATan(fScreenPosMult*(1.0f/CDraw::GetAspectRatio())*CMaths::Tan(fScreenAngle));

		FIGHT_AIM_TOWARD_TARGET = 0.0f;
	}
//	fAimAngleBeta = DEGTORAD(fAimAngleBeta);
//	fAimAngleAlpha = DEGTORAD(fAimAngleAlpha);


	if(ResetStatics)
	{
		TheCamera.ResetDuckingSystem(pPed);
		Rotating=FALSE;
		m_bCollisionChecksOn=true;
		ACQUIRED_FREEAIM_DIRECTION = true;//false;
		ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = 60000;
		ACQUIRED_FREEAIM_PEDHEADING = -1001.0f;
		FREEAIM_INCAR_TARGET_TAP_TIME = 0;
		AlphaSpeed = 0.0f;
		BetaSpeed = 0.0f;

#ifdef GTA_PC
		if(TheCamera.m_bUseMouse3rdPerson && !pPed->GetWeaponLockOnTarget())
		{
			// don't do anything to alpha or beta
			// want to use current camera direction
		}
		else
#endif
		{
			// reset alpha and beta to be behind ped at default angle
			Alpha = AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_DEFAULT_ALPHA];
			// for drivebys always start off looking ahead
			if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle)
			{
				Beta = pPed->m_fCurrentHeading - HALF_PI - fAimAngleBeta;
				Alpha += CMaths::ASin(MAX(-1.0f, MIN(1.0f, pPed->m_pMyVehicle->GetMatrix().GetForward().z)));
			}
			else if(!pPed->GetWeaponLockOnTarget())
			{
				Beta = pPed->m_fCurrentHeading - HALF_PI + fAimAngleBeta;
				
				if(pPed->GetIsStanding())
				{
					float fGroundNormalFwd = DotProduct(pPed->m_vecGroundNormal, pPed->GetMatrix().GetForward());
					Alpha -= CMaths::ASin(MIN(1.0f, MAX(-1.0f, fGroundNormalFwd)));
					
					if(pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_EXTINGUISHER)
						Alpha += CWeapon::ms_fExtinguisherAimAngle;
				}
			}
//////////////////// new 14.04.2004		
//			else if(pPed->GetWeaponLockOnTarget())
//			{
//				ACQUIRED_FREEAIM_DIRECTION = true;
//			}
////////////////////
//			else
//			{
//				float fGroundNormalFwd = DotProduct(pPed->m_vecGroundNormal, pPed->GetMatrix().GetForward());
//				Alpha -= CMaths::ASin(MIN(1.0f, MAX(-1.0f, fGroundNormalFwd)));
//			}
		}
	}
	
	if(CTheScripts::fCameraHeadingStepWhenPlayerIsAttached > 0.0)
	{
		float fDiff = Beta - CTheScripts::fCameraHeadingWhenPlayerIsAttached;
		if (fDiff < 0.0f)
			fDiff += TWO_PI;
		
		float fDiff2 = TWO_PI - fDiff;
		
		if ( (fDiff < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached) || (fDiff2 < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached) )
		{	//	Shortest distance to destination rotation is less than the step value so just jump to the destination rotation
			Beta = CTheScripts::fCameraHeadingWhenPlayerIsAttached;
			CTheScripts::fCameraHeadingStepWhenPlayerIsAttached = 0.0f;
		}
		else
		{
			if (fDiff > fDiff2)
				Beta += CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
			else
				Beta -= CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
		}
	}
	
	vecTargetCoords=ThisCamsTarget;

	// look at position roughly above player's head
	float MOVED_Z = vecTargetCoords.z;
	CVector vecCamTargetPosZ(0.0f, 0.0f ,0.0f);
	
	pPed->UpdateRpHAnim();
	
	// ok this wasn't working because m_nPedFlags.bCalledPreRender is false at this point
	// so it was just returning the ped's position + 0.5m
	// we could get it to use the bone properly, and remove the duck stuff?
	// but for now I'll just set the CamTarget to the results we were getting anyway	
	//pPed->GetBonePosition(vecCamTargetPosZ, BONETAG_R_CLAVICLE, true);
	vecCamTargetPosZ = pPed->GetPosition() + CVector(0.0f,0.0f,0.5f);
	vecCamTargetPosZ.z += AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_DEFAULT_Z];
	if(FOV < 70.0f)
		vecCamTargetPosZ.z += 0.1f*CMaths::Min(1.0f, (70.0f - FOV) / (70.0f - AIMWEAPON_RIFLE1_ZOOM));

	vecTargetCoords.z = vecCamTargetPosZ.z;
	MOVED_Z = vecTargetCoords.z - MOVED_Z;


	// slide to the side to look over shoulder
	float fSide = 0.20f;
	if(pWeaponInfo && !pWeaponInfo->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM)
	&& pPed->GetPlayerData()->m_pClothes->HasVisibleNewHairCut(1))
		fSide += 0.1f;
	else if(FOV < 70.0f)
		fSide += 0.1f*CMaths::Min(1.0f, (70.0f - FOV) / (70.0f - AIMWEAPON_RIFLE2_ZOOM));
	
	static bool TEST_POSITION_AIM_CAM_USING_CAM = true;
	if(TEST_POSITION_AIM_CAM_USING_CAM)
	{
		CVector vecTempRight = CrossProduct(Front, Up);
		float fSideMult2 = CMaths::Max(0.0f, CMaths::Min(1.0f, DotProduct(vecTempRight, pPed->GetMatrix().GetRight())));
		fSideMult2 = 1.0f - CMaths::ACos(fSideMult2) / HALF_PI;
		vecTargetCoords += fSide*fSideMult2*vecTempRight;
	}
	else
		vecTargetCoords += fSide*pPed->GetMatrix().GetRight();

//	vecTargetCoords.z += AIMWEAPON_DEFAULT_Z;	// move up to look at ped's head rather than torso
//	float MOVED_Z = AIMWEAPON_DEFAULT_Z;
	
	if(pPed->GetWeaponLockOnTarget())
	{
		CVector vecTempTarget(0.0f,0.0f,0.0f);
		if(pPed->GetWeaponLockOnTarget()->GetIsTypePed() && pWeaponInfo->GetWeaponFireType()!=FIRETYPE_MELEE)
			((CPed *)pPed->GetWeaponLockOnTarget())->GetBonePosition(vecTempTarget, BONETAG_SPINE1, true);
		else
			vecTempTarget = pPed->GetWeaponLockOnTarget()->GetPosition();
		
		if(pWeaponInfo->GetWeaponFireType()==FIRETYPE_MELEE)
		{
			static float TWEAK_MELEE_TARGETZ = 0.75f;
			vecTempTarget.z += TWEAK_MELEE_TARGETZ*MOVED_Z;
		}
		
		if(ResetStatics || pPed->GetPedIntelligence()->GetTaskFighting()==NULL)
			vecWeaponTargetPos = vecTempTarget;
		else
		{
//			static float MELEE_TARGET_SMOOTH_RATE = 0.9f;
//			float fSmooth = CMaths::Pow(MELEE_TARGET_SMOOTH_RATE, CTimer::GetTimeStep());
			float fSmooth = CMaths::Pow(PLAYERFIGHT_LEVEL_SMOOTHING_CONST, CTimer::GetTimeStep());
			vecWeaponTargetPos = fSmooth*vecWeaponTargetPos + (1.0f - fSmooth)*vecTempTarget;
		}
	}

	if(pPed->GetWeaponLockOnTarget())
	{
		CVector vecDelta = vecWeaponTargetPos - vecTargetCoords;
		float fTargetBeta = CMaths::ATan2(-vecDelta.x, vecDelta.y) - HALF_PI;
		float fTargetAlpha = 0.0f;
		fTargetAlpha = CMaths::ATan2(vecDelta.z, vecDelta.Magnitude2D());

		if(pWeaponInfo->GetWeaponFireType()==FIRETYPE_MELEE)
		{
			fTargetAlpha *= CMaths::Cos(fAimAngleBeta);
		}
		else
		{
			float fDistToCam = MIN((vecTargetCoords - Source).Magnitude(), AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_BASE_DIST]);
			float fDistToPed = vecDelta.Magnitude();
			float fDistCamToPed = fDistToCam + fDistToPed;

			fAimAngleAlpha *= fDistCamToPed/fDistToPed;
			fAimAngleBeta *= fDistCamToPed/fDistToPed;
		}
		
		fTargetBeta += fAimAngleBeta;
		fTargetAlpha -= fAimAngleAlpha;
	
		if(fTargetAlpha < -PI)	fTargetAlpha += TWO_PI;
		else if(fTargetAlpha > PI)	fTargetAlpha -= TWO_PI;
		
		float fTargetRate = AIMWEAPON_TARGET_SENS*CTimer::GetTimeStep();
		if(ResetStatics)
			fTargetRate = 1000.0f;
		
		float fTargetDiff = fTargetAlpha - Alpha;
		if(CMaths::Abs(fTargetDiff) < fTargetRate)
			Alpha = fTargetAlpha;
		else if(fTargetDiff < 0.0f)
			Alpha -= fTargetRate;
		else
			Alpha += fTargetRate;
		
		if(fTargetBeta - Beta > PI)	fTargetBeta -= TWO_PI;
		else if(fTargetBeta -Beta < -PI)	fTargetBeta += TWO_PI;
		
		fTargetDiff = fTargetBeta - Beta;
		if(CMaths::Abs(fTargetDiff) < fTargetRate)
			Beta = fTargetBeta;
		else if(fTargetDiff < 0.0f)
			Beta -= fTargetRate;
		else
			Beta += fTargetRate;

		AlphaSpeed = 0.0f;
		BetaSpeed = 0.0f;
	}
#ifdef GTA_PC
	else if(TheCamera.m_bUseMouse3rdPerson && CPad::GetPad(0)->DisablePlayerControls==0
	&& (CPad::GetPad(0)->GetAmountMouseMoved().x != 0.0f || CPad::GetPad(0)->GetAmountMouseMoved().y!=0.0f))
	{
		RwV2d m_MouseMovement = CPad::GetPad(0)->GetAmountMouseMoved();

		fStickX=-m_MouseMovement.x * 2.5f;
		fStickY=m_MouseMovement.y * 4.0f;

		StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
		StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (FOV/80.0f);
		
		Beta += StickBetaOffset;
		Alpha += StickAlphaOffset;						

		// don't want any movement left over when we stop moving the mouse
		AlphaSpeed = BetaSpeed = 0.0f;
	}
#endif
	else
	{
		fStickX= -(CPad::GetPad(0)->AimWeaponLeftRight(pPed));
		fStickY= CPad::GetPad(0)->AimWeaponUpDown(pPed);	
			
		StickBetaOffset = AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(fStickX)*fStickX * (0.25f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
		StickAlphaOffset = AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(fStickY)*fStickY * (0.15f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();

		static float AIM_WEAPON_STICK_RATE_UP = 0.8f;
		static float AIM_WEAPON_STICK_RATE_DOWN = 0.5f;
		
		float fRate = AIM_WEAPON_STICK_RATE_UP;
		if(CMaths::Abs(fStickX) < 2.0f && CMaths::Abs(fStickY) < 2.0f)
			fRate = AIM_WEAPON_STICK_RATE_DOWN;
		
		fRate = CMaths::Pow(fRate, CTimer::GetTimeStep());
		
		BetaSpeed = fRate*BetaSpeed + (1.0f - fRate)*StickBetaOffset;
		AlphaSpeed = fRate*AlphaSpeed + (1.0f - fRate)*StickAlphaOffset;
		
		StickBetaOffset = BetaSpeed;
		StickAlphaOffset = AlphaSpeed;

		if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle && pPed->m_pMyVehicle->pDriver!=pPed)
		{
			if(CPad::GetPad(0)->TargetJustDown())
			{
				static float AIM_WEAPON_IN_CAR_PASSENGER_DOUBLETAP_TIME = 500;
				if(CTimer::GetTimeInMilliseconds() - FREEAIM_INCAR_TARGET_TAP_TIME < AIM_WEAPON_IN_CAR_PASSENGER_DOUBLETAP_TIME)
				{
					StickBetaOffset = PI;
					StickAlphaOffset = 0.0f;
				}
				else
					FREEAIM_INCAR_TARGET_TAP_TIME = CTimer::GetTimeInMilliseconds();
			}
		}
		else if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle && pPed->m_pMyVehicle->pDriver==pPed)
		{
			if(fStickX!=0.0f || fStickY!=0.0f)
				ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = 0;
			else if(!CPad::GetPad(0)->GetWeapon(pPed))
				ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER += CTimer::GetTimeElapsedInMilliseconds();
			
			if(ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER > ACQUIRED_FREEAIM_STATIC_LIM_A)
			{
				ACQUIRED_FREEAIM_DIRECTION = false;
				ACQUIRED_FREEAIM_PEDHEADING = pPed->m_fCurrentHeading - HALF_PI + fAimAngleBeta;
			}
			else if(ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER > ACQUIRED_FREEAIM_STATIC_LIM_B)
			{
				float fAngleDiff = (pPed->m_fCurrentHeading - HALF_PI) - fAimAngleBeta - Beta;
				if(fAngleDiff > TWO_PI)	fAngleDiff -= TWO_PI;
				else if(fAngleDiff < -TWO_PI)	fAngleDiff += TWO_PI;
				if(fAngleDiff < DEGTORAD(30.0f))
				{
					ACQUIRED_FREEAIM_DIRECTION = false;
					ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = ACQUIRED_FREEAIM_STATIC_LIM_A + 1;
					ACQUIRED_FREEAIM_PEDHEADING = pPed->m_fCurrentHeading - HALF_PI + fAimAngleBeta;
				}
				else
					ACQUIRED_FREEAIM_DIRECTION = true;
			}
			else
			{
				ACQUIRED_FREEAIM_DIRECTION = true;
				if(CPad::GetPad(0)->GetWeapon(pPed))
					ACQUIRED_FREEAIM_INCAR_IDLE_COUNTER = 0;
			}
		}

		if(!ACQUIRED_FREEAIM_DIRECTION)
		{
//			if(pWeaponInfo && !pWeaponInfo->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM)
//			&& pWeaponInfo->GetWeaponFireType()!=FIRETYPE_MELEE)
//			{
//				pPed->m_fCurrentHeading += StickBetaOffset;
//				pPed->m_fDesiredHeading = pPed->m_fCurrentHeading;
//				// need to manually set the entity heading here as well to keep in sync with camera
//				pPed->SetHeading(pPed->m_fCurrentHeading);
//				pPed->UpdateRwMatrix();
//			}
//			float fTargetBeta = pPed->m_fCurrentHeading - HALF_PI;
			float fTargetBeta = pPed->m_fCurrentHeading - HALF_PI;
			if(ACQUIRED_FREEAIM_PEDHEADING < -1000.0f)
				ACQUIRED_FREEAIM_PEDHEADING = fTargetBeta;
			else
				fTargetBeta = ACQUIRED_FREEAIM_PEDHEADING;
				
			if(pWeaponInfo && !pWeaponInfo->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM)
			&& pWeaponInfo->GetWeaponFireType()!=FIRETYPE_MELEE)
			{
				pPed->m_fCurrentHeading = pPed->m_fDesiredHeading = ACQUIRED_FREEAIM_PEDHEADING + HALF_PI;
				// need to manually set the entity heading here as well to keep in sync with camera
				pPed->SetHeading(pPed->m_fCurrentHeading);
				pPed->UpdateRwMatrix();
			}

			fTargetBeta -= fAimAngleBeta;

			float fTargetRate = AIMWEAPON_FREETARGET_SENS*CTimer::GetTimeStep();
			float fCloseEnoughRange = 0.0f;
			if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle && pPed->m_pMyVehicle->pDriver==pPed)
			{
				fTargetRate *= AIMWEAPON_DRIVE_SENS_MULT;
				fCloseEnoughRange = AIMWEAPON_DRIVE_CLOSE_ENOUGH;
			}
			if(fTargetBeta - Beta > PI)	fTargetBeta -= TWO_PI;
			else if(fTargetBeta -Beta < -PI)	fTargetBeta += TWO_PI;
		
			float fTargetDiff = fTargetBeta - Beta;
			if(fCloseEnoughRange > 0.0f)
			{
				if(fTargetDiff > fCloseEnoughRange)
					fTargetDiff -= fCloseEnoughRange;
				else if(fTargetDiff < -fCloseEnoughRange)
					fTargetDiff += fCloseEnoughRange;
				else
					fTargetDiff = 0.0f;
			}
			
			if(CMaths::Abs(fTargetDiff) < fTargetRate)
			{
				Beta += fTargetDiff;
				ACQUIRED_FREEAIM_DIRECTION = true;
			}
			else if(fTargetDiff < 0.0f)
				Beta -= fTargetRate;
			else
				Beta += fTargetRate;
			
			if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle && pPed->m_pMyVehicle->pDriver==pPed)
			{
				float fTargetAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, pPed->m_pMyVehicle->GetMatrix().GetForward().z)));
				fTargetAlpha += AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_DEFAULT_ALPHA];
				if(fTargetAlpha - Alpha > PI)	fTargetAlpha -= TWO_PI;
				else if(fTargetAlpha -Alpha < -PI)	fTargetAlpha += TWO_PI;
				fTargetDiff = fTargetAlpha - Alpha;
	
				if(fTargetDiff > fCloseEnoughRange)
					fTargetDiff -= fCloseEnoughRange;
				else if(fTargetDiff < -fCloseEnoughRange)
					fTargetDiff += fCloseEnoughRange;
				else
					fTargetDiff = 0.0f;

				if(CMaths::Abs(fTargetDiff) < fTargetRate)
					Alpha += fTargetDiff;
				else if(fTargetDiff < 0.0f)
					Alpha -= fTargetRate;
				else
					Alpha += fTargetRate;
			}
			else
				Alpha+=StickAlphaOffset;
		}		
		else
		{
			ACQUIRED_FREEAIM_PEDHEADING = -1001.0f;
			Beta+= StickBetaOffset;
			Alpha+=StickAlphaOffset;
		}
	}
	
	ClipBeta();
	
	if(Alpha > AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_ROTMAX_UP])
		Alpha = AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_ROTMAX_UP];
	else if (Alpha< -AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_ROTMAX_DOWN])
		Alpha = -AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_ROTMAX_DOWN];
	
	float fDefaultDistFromPed = AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_BASE_DIST];
	if(Alpha > 0.0f)
		fDefaultDistFromPed += AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_ANGLE_DIST] * CMaths::Cos( MIN(HALF_PI, AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_ANGLE_FALLOFF]*Alpha) );
	else
		fDefaultDistFromPed += AIMWEAPON_SETTINGS[nAimType][AIMWEAPON_ANGLE_DIST] * CMaths::Cos( Alpha );
		
// temp - we're gonna do all jump cuts, so don't bother smoothing anything
//	if(nAimType==AIMWEAPON_ONFOOT)
//	{
//		TheCamera.m_fPedZoomTotal = fDefaultDistFromPed;
//		fDefaultDistFromPed = TheCamera.m_fPedZoomSmoothed;
//	}
	
	Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
	Source = vecTargetCoords - fDefaultDistFromPed * Front;

	//==============================================================
	// DW - HANDLE DUCKING - and transtitions
	TheCamera.HandleCameraMotionForDuckingDuringAim(pPed, &Source, &vecTargetCoords);
	// DW - END HANDLE DUCKING
	//==============================================================
	m_cvecTargetCoorsForFudgeInter = vecTargetCoords;

//////////////////////////////////////
// this chunk of code is pretty tempory, kinda copied from original follow ped
// and used in Process_AimWeapon(), Process_Follow_History(), and new Process_FollowPed()
// as a quick fix to stop the camera clipping through shit
// would like to smooth it all out and do some kind of predictive moving the camera forward like
// in MH, but don't want to start fucking around with the camera angles because that gets messy
//////////////////////////////////////
#ifdef DW_CAMERA_COLLISION

	TheCamera.SetColVarsAimWeapon(nAimType);
	if (DirectionIsLooking==LOOKING_FORWARD)
	{
		// Aim weapon follow cams		
		TheCamera.CameraGenericModeSpecialCases(pPed);
		TheCamera.CameraPedAimModeSpecialCases(pPed);
		TheCamera.CameraColDetAndReact(&Source,&vecTargetCoords);
		TheCamera.ImproveNearClip(NULL,pPed,&Source,&vecTargetCoords);	// doesnt do anything here I think - DW		
	}
#else
	// See at bottom of file .. comment @BLOCK 1@
#endif // DW_CAMERA_COLLISION

	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

	// swing the camera around a bit to look toward target ped
	if(FIGHT_AIM_TOWARD_TARGET > 0.0f && pPed->GetWeaponLockOnTarget())
	{
		CVector vecTempTarget = (1.0f - 0.5f*FIGHT_AIM_TOWARD_TARGET)*vecTargetCoords + 0.5f*FIGHT_AIM_TOWARD_TARGET*vecWeaponTargetPos;
		Front = vecTempTarget - Source;
		Front.Normalise();
	}

	GetVectorsReadyForRW();

	if(pWeaponInfo && (!pWeaponInfo->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM) || pPed->m_nPedFlags.bIsDucking)
	&& pWeaponInfo->GetWeaponFireType()!=FIRETYPE_MELEE && !pPed->m_nPedFlags.bInVehicle)
	{
		float CamDirection = -101.0f;
	
		if (pPed->GetWeapon()->GetWeaponType() == WEAPONTYPE_SPRAYCAN) // DW - removed due to 'tag' problems.
		{
			CamDirection = CMaths::ATan2(-Front.x, Front.y) - fAimAngleBeta;
		}
		else
		{
			if(pPed->GetWeaponLockOnTarget())
			{
				CVector vecDelta = pPed->GetWeaponLockOnTarget()->GetPosition() - pPed->GetPosition();
				CamDirection = CMaths::ATan2(-vecDelta.x, vecDelta.y);
			}
			else if(ACQUIRED_FREEAIM_DIRECTION)
			{
				CamDirection = CMaths::ATan2(-Front.x, Front.y) - fAimAngleBeta;
			}
		}
				
		if(CamDirection > -100.0f)
		{
			pPed->m_fCurrentHeading=CamDirection + fTweakPedAimDirn;
			pPed->m_fDesiredHeading=CamDirection + fTweakPedAimDirn;
			// need to manually set the entity heading here as well to keep in sync with camera
			pPed->SetHeading(CamDirection);
			pPed->UpdateRwMatrix();
		}		
		((CPlayerPed *)TheCamera.pTargetEntity)->GetPlayerData()->m_fLookPitch = TheCamera.Find3rdPersonQuickAimPitch();
	}
////////////////////////////
// end of nasty chunk
////////////////////////////
	ResetStatics = false;
	

#ifndef FINAL	
	char str[255];
	sprintf(str,"Alpha %f Beta %f\n",Alpha,Beta);
	VarConsole.AddDebugOutput(str);		
	sprintf(str,"Src %f %f %f\n",Source.x,Source.y,Source.z);
	VarConsole.AddDebugOutput(str);	
	sprintf(str,"targ %f %f %f\n",vecTargetCoords.x,vecTargetCoords.y,vecTargetCoords.z);
	VarConsole.AddDebugOutput(str);	
	sprintf(str,"ThisCamsTarget %f %f %f\n",ThisCamsTarget.x,ThisCamsTarget.y,ThisCamsTarget.z);
	VarConsole.AddDebugOutput(str);			
#endif	
}


float fTestShiftHeliCamTarget = 0.6f;
//
struct CamFollowHistData
{
	// place and constrain points by creation point and time
//	int32 fDecayTime;
	// second attempt create at set time intervals, as set countdown timer
	float fResolution;	// how often we place points
	int32 nMaxNumPts;	// max length of trail in pts (may extend past this)
	int32 nCountDown;	// counter on each point that counts down to extinction
	float fMaxDist;		// max length of trail in m
	// where to place the camera
	float fTargetOffsetZ;		// where on entity to aim at
	float fTargetSlowFwdMult;	// where to place pts behind entity when moving slow
	float fBaseCamDist;			// base distance of camera behind target
	float fBaseCamZ;			// base height of camera above target
	// currently using quat-slerps
//	float fRotationRate;
	// new interpolation using.. other stuff
	float fDiffRate;
	float fDiffCap;
	// speed rate stuff
	float fDiffAlphaRate;
	float fDiffAlphaCap;
	float fSpeedAlphaRate;
	float fDiffBetaRate;
	float fDiffBetaCap;
	float fSpeedBetaRate;
	// add swing for cresting rises
	float fCrestAlphaRate;
	float fCrestAlphaCap;
	// angle limits
	float fUpLimit;
	float fDownLimit;
};
//
//static bool USE_NEW_DATA = true;
//static bool TEST_CAM_SOURCE_AT_POINT = false;
//#define USE_SPEED_FOR_SWING_RATE;


// these are used in Follow_History AND FollowCar_SA functions
float CAR_FOV_START_SPEED = 0.4f;
float CAR_FOV_FADE_MULT = 0.98f;
//
enum
{
	FHISTORY_ONBIKE=0,
	FHISTORY_INCAR,
	FHISTORY_INHELI,
	FHISTORY_INPLANE,
	FHISTORY_RCCAR,
	FHISTORY_RCHELI
};


struct CamFollowPedData
{
	// where to place the camera
	float fTargetOffsetZ;		// where on entity to aim at
	float fBaseCamDist;			// base distance of camera behind target
	float fBaseCamZ;			// base height of camera above target
	float fMinDist;				// min dist camera drawn from ped
	float fMinFollowDist;		// min dist used to follow ped
	// new interpolation using.. other stuff
	float fDiffAlphaRate;
	float fDiffAlphaCap;
	float fDiffAlphaSwing;
	// how to swing the camera around behind the player
	float fDiffBetaRate;
	float fDiffBetaCap;
	float fDiffBetaSwing;
	float fDiffBetaSwingCap;
	// sometimes need to tweak how fast the right stick moves the camera around
	float fStickMult;
	// alpha angle limits
	float fUpLimit;
	float fDownLimit;
};
//
enum{
	FOLLOW_PED_OUTSIDE = 0,
	FOLLOW_PED_INSIDE,
	FOLLOW_PED_2PLAYER,
	FOLLOW_PED_NUMSETTTINGS
};

CamFollowPedData PEDCAM_SET[FOLLOW_PED_NUMSETTTINGS] =
{
//	targZ	baseDst baseZ	minDist	minFol	aRate	aCap 	aSwing	bRate	bCap	bSwing	bSwingCap	stickM	upLimit			downLimit
	{0.6f,	2.0f,	0.15f,	2.0f,	4.0f,	0.8f,	0.1f, 	0.5f,	0.8f,	0.1f,	0.1f,	0.02f,		1.0f, 	DEGTORAD(45.0f), DEGTORAD(85.0f)},	// onfoot
	{0.6f,	2.0f,	0.15f,	2.0f,	3.0f,	0.9f, 	0.1f, 	1.0f,	0.8f,	0.1f,	0.3f,	0.05f,		1.0f,	DEGTORAD(45.0f), DEGTORAD(45.0f)},	// interior
	{0.6f,	2.0f,	0.15f,	2.0f,	4.0f,	0.8f,	0.1f, 	0.5f,	0.8f,	0.1f,	0.1f,	0.02f,		1.0f,	DEGTORAD(45.0f), DEGTORAD(85.0f)}	// 2player
};
//

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
float gLastCamDist = 0.0f;
bool gForceCamBehindPlayer = false;
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


#ifndef MASTER 
	#define DW_PRINTF// printf
#else
	#define DW_PRINTF
#endif
	
//----------------------------------------------------------------
// when the idle cam is effectively switched off - even indirectly
void CIdleCam::IdleCamGeneralProcess() // to still be pur into code in right place
{
	if (!IsItTimeForIdleCam()) // no longer want idle cam
	{		
		// safer to comment this out - DW	
//		if (lastFrameProcessed == CTimer::m_FrameCounter-1) // but we where processing it last frame
//		{
//			DW_PRINTF("Idle Cam deactivated - no interesting events being looked for\n");			
//		}
		g_InterestingEvents.SetActive(false); // must switch this stuff off!
		//gIdleCam.idleTickerFrames = 0;
	}
	
	if (TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_FOLLOWPED) 
	{
		g_InterestingEvents.SetActive(false);
		gIdleCam.idleTickerFrames = 0;
	}	
}

//------------------------------
void CIdleCam::Init()
{	
	slerpDuration = 2000;								// time(ms) to interpolate camera angles from one target to another			
		
	timeControlsIdleForIdleToKickIn = 90000;			// time(ms) to expire with no user input before idle will kick in

	timeToConsiderNonVisibleEntityAsOccluded = 3000;	// After this time(ms) if the entity is still occluded we eventually consider it so ( we give up )

	distTooClose 		= 4.0f;							// objects this close are not looked at
	distStartFOVZoom 	= 15.0f;						// objects at this distance and further can be FOV zoomed into
	distTooFar 			= 80.0f;						// Objects this far away are too far away to look at 
	degreeShakeIdleCam	= 1.0f; 						// A multiplier to tweak the shake effect of the camera
	
	shakeBuildUpTime    = 3000;							// The time that it takes to build up shake to full effect from being completely stable
	
	zoomNearest 		= 15.0f;						// The nearest FOV zoom
	zoomFarthest		= 70.0f;						// The furthest FOV zoom
	durationFOVZoom		= 1000.0f;						// The time(ms) that zooms take
	targetLOSFramestoReject = 14;						// If LOS is blocked for this number of FRAMES we reject the object
	timeBeforeNewZoomIn = 12000;						// Time(ms) that must have expired before we permit zooming in again after having been zoomed in before
	timeMinimumToLookAtSomething = 5000;				// The minimum time(ms) that we look at something ( we dont want a shifty camera, lets persist looking at stuff for at least some time if we have gone to all the effort to rotate towards it and zoom in!
	increaseMinimumTimeFactorforZoomedIn = 2.0f;		// A multiplier that makes the minimum time to look at an object increase if we have zoomed in ( since zooming in shows more interest in an object ) 
	lastTimePadTouched = 0;								// The last time the pad was touched ( cached so on a per frame basis we can see if it has changed ) 
	idleTickerFrames = 0;								// A frame ticker that indicates number of pad inactive frames passed
	
	Reset(false);
}

//------------------------------
void CIdleCam::Reset(bool bResetControls)
{
	pTarget		= NULL;

	positionToSlerpFrom = CVector(0,0,0);
	lastIdlePos			= CVector(0,0,0);
	timeSlerpStarted = -1;
	timeIdleCamStarted = -1;
	timeLastTargetSelected = -1;

	zoomState = ZOOMED_OUT;
	zoomFrom = -1;
	zoomTo   = -1;
	timeZoomStarted = -1;

	timeTargetEntityWasLastVisible = -1;
	bForceAZoomOut = false;
	
	curFOV = 70.0f;
	
	targetLOSCounter=0;
	bHasZoomedIn = false;
	timeLastZoomIn = -1;
	slerpTime = 1.0f;
	
	if (bResetControls)
	{
		CPad::GetPad(0)->LastTimeTouched = CTimer::GetTimeInMilliseconds();
	}
}


//------------------------------
bool CIdleCam::Process()
{		
	ProcessIdleCamTicker();
	
	if (IsItTimeForIdleCam())
	{		
		float time 		= CTimer::GetTimeInMilliseconds();
		pCam = &TheCamera.Cams[TheCamera.ActiveCam];
	
		// Is this this first frame of processing since it has been no longer an idle cam
		bool bIsFirstFrame = lastFrameProcessed < CTimer::m_FrameCounter-1; // check this!
		
		if (bIsFirstFrame)
		{
			g_InterestingEvents.SetActive(true);
			Reset(false);				// first time - so reset 
			timeIdleCamStarted = time;					
			SetTargetPlayer();			// make sure we start off looking at the player first
		}		
		
		lastFrameProcessed = CTimer::m_FrameCounter;	
	
		Run();
	
		gbCineyCamProcessedOnFrame = lastFrameProcessed; // a global that indicates to other parts of code the processing state of the camera		
	
		return true;
	}
	return false;
}

//------------------------------------
// Validates selected target
bool CIdleCam::IsTargetValid(CEntity *pTest)
{
	ASSERT(pTest);
	
	if (pTest)
	{
		CEntity *pPlayer = (CEntity*)FindPlayerPed();	
		ASSERT(pPlayer);	
		if (pTest == pPlayer)	// player is always valid
			return true;

		CVector target;
		GetLookAtPositionOnTarget(pTest,&target);
		CVector delta = pCam->Source-target;
		float dist = delta.Magnitude();

		// Distance reject
		if (dist<distTooClose)
		{
			DW_PRINTF("Idle Cam too close - invalid!\n");
			return false;
		}
		else if (dist>distTooFar)
		{
			DW_PRINTF("Idle Cam too FAR - invalid!\n");		
			return false;
		}
										
		if (slerpTime>=1.0f) // we dont invalidate objects if we arent yet properly looking at them yet ( for collision at least ) 
		{
			// Collision reject
			CEntity *pOldIgnore = CWorld::pIgnoreEntity;
			CWorld::pIgnoreEntity = pTest;		
			bool bLOSClear = CWorld::GetIsLineOfSightClear(pCam->Source,target, true, false, false, true, false, false, true);
			CWorld::pIgnoreEntity = pOldIgnore;							
			
			if (!bLOSClear)
			{
				DW_PRINTF("Idle Cam LOS bad\n");
				if (targetLOSCounter++>targetLOSFramestoReject)
				{
					DW_PRINTF("Idle Cam LOS bad... now invalid\n");		
					return false;
				}
			}
		}
		
		return true;		
	}	

	return false;
}

//-----------------------------------------
// Chooses targets
void CIdleCam::ProcessTargetSelection()
{	
	CEntity *pPlayer = (CEntity*)FindPlayerPed();	
	ASSERT(pPlayer);	

	float time 		= CTimer::GetTimeInMilliseconds();
	float tDelta = time-timeLastTargetSelected;
	if (zoomState!=ZOOMED_OUT && targetLOSCounter<=0)	
		tDelta /= increaseMinimumTimeFactorforZoomedIn; // we want to keep zoomed in for longer if we have bothered to do it, but usually not when collision is registered.

	if (tDelta>timeMinimumToLookAtSomething) // enough time expired looking at this target - lets maybe choose a new target
	{			
		g_InterestingEvents.InvalidateNonVisibleEvents();
		const TInterestingEvent *pLatestEvent = g_InterestingEvents.GetInterestingEvent();
		if (!pLatestEvent) // no events
		{
			DW_PRINTF("No Events\n");
			if (!pTarget || !IsTargetValid(pTarget))
			{	
				DW_PRINTF("Might set the traget to player\n");					
			
				if (pTarget != pPlayer) // ensure we look at the player
				{
					if (zoomState==ZOOMED_OUT)
					{
						DW_PRINTF("now the target is the player\n");										
						SetTargetPlayer();
					}
					else
					{
						DW_PRINTF("have to zoom out first\n");					
						bForceAZoomOut = true;
					}
				}
			}
		}
		else
		{
			CEntity *pEntity = pLatestEvent->m_pEntity;
			ASSERT(pEntity);
			if (pTarget == pEntity) // no change in entity
			{
				if (!IsTargetValid(pEntity))
				{							
					DW_PRINTF("We invalidate this event\n");														
					g_InterestingEvents.InvalidateEvent(pLatestEvent);
				}
			}
			else // an event might be worthwhile changing to - but we have to check this is OK first.
			{
				// first strip out bad collision events... becuas ethey might have been hanging around for a while.
				if (IsTargetValid(pEntity))
				{
					// hold this is OK to set the target but we want to kick off a FOV zoom out first, and we want to persist looking where we where in
					// in some circumstances... so we delay this slightly				
					if (zoomState==ZOOMED_OUT)
					{
						DW_PRINTF("New Event target selected!\n");														
						SetTarget(pEntity);
					}
					else
					{	
						DW_PRINTF("New Event target pending zoom out\n");																							
						bForceAZoomOut = true; // lets zoom out and hope we will eventually get the same target once we have finished zooming out.
					}
				}
			}
		}
	}
	
	// no target at all?
	if (!pTarget)
	{
		DW_PRINTF("Emergency we - have no target\n");																								
		bForceAZoomOut = true;
		pTarget = pPlayer;// fixes a weird bug
		SetTargetPlayer();
	}
				
	// We must have a target now.... but is it valid?...if not the player is chosen
	ASSERT(pTarget);
	if (!IsTargetValid(pTarget) && tDelta>timeMinimumToLookAtSomething)
	{		
		DW_PRINTF("Target is invalid afterall!\n");																								
	
		if (zoomState==ZOOMED_OUT)
		{
			DW_PRINTF("Target set to player!\n");																								
			SetTargetPlayer();
		}				
		else
		{
			DW_PRINTF("TForcing a zoom out first\n");																								
			bForceAZoomOut = true;
		}

		// actually we better do something here		
		if (targetLOSCounter>0)
		{
			SetTargetPlayer();
		}		
	}	
	
	// really hard case to fix!
	if (targetLOSCounter>targetLOSFramestoReject) // we want the player cos bad evil shit is going on!
	{
		SetTargetPlayer();
		bForceAZoomOut = true;
	}
}


//---------------------------------------------------------------------
// most control logic aside we process the camera as if it will be idle
void CIdleCam::Run()
{
	float time 		= CTimer::GetTimeInMilliseconds();

	ProcessTargetSelection();
	
	// Set shake degree ( can build up smoothly )
	float shakeDegree = 1.0f;
	float timeRunning = time-timeIdleCamStarted;
	if (timeRunning<shakeBuildUpTime)
		shakeDegree = timeRunning/shakeBuildUpTime;

	float curAngleX,curAngleZ;
	slerpTime = ProcessSlerp(&curAngleX,&curAngleZ);
	ProcessFOVZoom(slerpTime);
	
	FinaliseIdleCamera(curAngleX,curAngleZ,shakeDegree);
}

//------------------------------
void CIdleCam::FinaliseIdleCamera(float curAngleX,float curAngleZ, float shakeDegree)
{

//	printf("curAngleX %f SlerpFromRotX %f SlerpToRotX %f\n",curAngleX,SlerpFromRotX,SlerpToRotX);
//	printf("curAngleZ %f SlerpFromRotZ %f SlerpToRotZ %f\n",curAngleZ,SlerpFromRotZ,SlerpToRotZ);

#ifndef MASTER
/*	char str[255];
	sprintf(str,"X ANG %f FROM %f TO %f",curAngleX,SlerpFromRotX,SlerpToRotX);
	VarConsole.AddDebugOutput(str);
	sprintf(str,"Z ANG %f FROM %f TO %f",curAngleZ,SlerpFromRotZ,SlerpToRotZ);
	VarConsole.AddDebugOutput(str);
*/	
#endif

	pCam->Front = CVector(-CMaths::Cos(curAngleZ) *CMaths::Cos(curAngleX) , -CMaths::Sin(curAngleZ)*CMaths::Cos(curAngleX), CMaths::Sin(curAngleX));			
	pCam->Front.Normalise();
	
	lastIdlePos = pCam->Source + pCam->Front; // cache where we are looking ( without the shake! ) 
	
	CHandShaker *pS = &gHandShaker[0];		
	pS->Process(degreeShakeIdleCam*shakeDegree);		
	float roll = pS->ang.z * degreeShakeIdleCam*shakeDegree;
	CVector temp = Multiply3x3(pCam->Front,pS->resultMat);
	pCam->Front = temp;	
	
	pCam->Up=CVector(CMaths::Sin(roll),0.0f,CMaths::Cos(roll));
	CVector TempRight= CrossProduct(pCam->Front, pCam->Up);
	TempRight.Normalise();
	pCam->Up=CrossProduct(TempRight, pCam->Front);

	if ((pCam->Front.x==0.0f) && (pCam->Front.y==0.0f))
	{
		pCam->Front.x=0.0001f;
		pCam->Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( pCam->Front, pCam->Up );
	TempRight.Normalise();
	pCam->Up = CrossProduct( TempRight, pCam->Front );			
	
	// pCam->GetVectorsReadyForRW();	// was protected.
	
	pCam->Up = CVector(0.0f, 0.0f, 1.0f);
	pCam->Front.Normalise();
	if ((pCam->Front.x==0.0f) && (pCam->Front.y==0.0f))
	{
		pCam->Front.x=0.0001f;
		pCam->Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( pCam->Front, pCam->Up );
	TempRight.Normalise();
	pCam->Up = CrossProduct( TempRight, pCam->Front );		
}

//------------------------------
float CIdleCam::ProcessSlerp(float *pAngX, float *pAngZ)
{
	float time 		= CTimer::GetTimeInMilliseconds();	
	
	// work out angles for slerp.
	
	CVector targetToLookAtPos;
	GetLookAtPositionOnTarget(pTarget,&targetToLookAtPos);
	
	if (targetLOSCounter>=targetLOSFramestoReject)
	{
		DW_PRINTF("Persisting looking at old pos!\n");																									
		targetToLookAtPos = lastIdlePos;// persist looking where we last saw our target
	}
	
	CVector rvFrom 	= positionToSlerpFrom-pCam->Source;
	CVector rvTo 	= targetToLookAtPos-pCam->Source;
	float SlerpToRotAngX,SlerpToRotAngZ;	
	float SlerpFromRotAngX,SlerpFromRotAngZ;	
		
	VectorToAnglesRotXRotZ(&rvFrom,&SlerpFromRotAngX,&SlerpFromRotAngZ);
	VectorToAnglesRotXRotZ(&rvTo,&SlerpToRotAngX,&SlerpToRotAngZ);

	if(SlerpToRotAngX - SlerpFromRotAngX > PI)		SlerpToRotAngX -= TWO_PI;
	else if(SlerpToRotAngX -SlerpFromRotAngX < -PI)	SlerpToRotAngX += TWO_PI;

	if(SlerpToRotAngZ - SlerpFromRotAngZ > PI)		SlerpToRotAngZ -= TWO_PI;
	else if(SlerpToRotAngZ -SlerpFromRotAngZ < -PI)	SlerpToRotAngZ += TWO_PI;
		
	float timeDeltaSlerp = time-timeLastTargetSelected;
	float t = timeDeltaSlerp/slerpDuration;	

	if (t>1.0f)
		t = 1.0f;
	*pAngX = DW_SINE_ACCEL_DECEL_LERP(t,SlerpFromRotAngX,SlerpToRotAngX);
	*pAngZ = DW_SINE_ACCEL_DECEL_LERP(t,SlerpFromRotAngZ,SlerpToRotAngZ);			
	
	return t;
}

//------------------------------
void CIdleCam::ProcessFOVZoom(float t)
{
	float time 		= CTimer::GetTimeInMilliseconds();

	bool bInFOVZoomRange = false;
	// zoom is kicked off to go in when the camera has slerped to this object

	bool bWeHaveAFemale = false;
	float localZoomNearest = zoomNearest;
			
	if (pTarget)
	{
		CVector target;
		GetLookAtPositionOnTarget(pTarget,&target);
		CVector delta = pCam->Source-target;
		float dist = delta.Magnitude();	
					

		if (pTarget->GetIsTypePed())
		{
			CPed *pPed = (CPed*)pTarget;
			if (CPedType::IsFemaleType(pPed->GetPedType()))
			{
				bWeHaveAFemale = true;
				bInFOVZoomRange = true; // always zoom in on females
				localZoomNearest *=0.5f; // zoom in closer on females
			}
		}

		if (bWeHaveAFemale && dist<8.0f)
		{
			bForceAZoomOut = true;
		}

		if (dist>distStartFOVZoom)
		{
			bInFOVZoomRange = true;
		}		
	}	
			
	// trigger lerps
	if (t>=1.0f)
	{
		int32 origZoomState = zoomState;
		if (bInFOVZoomRange)
		{
			float tDelta = time-timeLastZoomIn;
			if (tDelta>timeBeforeNewZoomIn)
			{
				bool bLOSClear = true;
				
				if (pTarget)
				{
					CEntity *pOldIgnore = CWorld::pIgnoreEntity;
					CWorld::pIgnoreEntity = pTarget;		
					CVector target;
					GetLookAtPositionOnTarget(pTarget,&target);								
					bLOSClear = CWorld::GetIsLineOfSightClear(pCam->Source,target, true, false, false, true, false, false, true);	
					CWorld::pIgnoreEntity = pOldIgnore;							
				}			
					
				if (targetLOSCounter>10) // lets not make things worse looking! by zooming in on obscured stuff
				{
					if (zoomState==ZOOMED_IN)
					{
						zoomState = ZOOMING_OUT;
					}
				}
					
						
				if (zoomState==ZOOMED_OUT && !bHasZoomedIn && bLOSClear) // dont zoom in twice!.. dont zoom in if it is occluded!
				{
					zoomState = ZOOMING_IN;
					zoomTo    = localZoomNearest;	
					
					if (origZoomState != zoomState)
					{
						timeZoomStarted = time;	
						zoomFrom  		= curFOV;		
					}
				}
			}
		}
		else
		{
			if (zoomState==ZOOMED_IN)
			{
				zoomState = ZOOMING_OUT;
				zoomTo    = zoomFarthest;
			
				if (origZoomState != zoomState)
				{
					timeZoomStarted = time;	
					zoomFrom  		= curFOV;		
				}
			}		
		}
	}
	
	
	if (zoomState==ZOOMED_IN)
	{
		timeLastZoomIn = time; // dont want to zoom too much
	}
	
	if (bForceAZoomOut && zoomState==ZOOMED_IN)
	{
		timeZoomStarted = time;
		zoomFrom  		= curFOV;
		zoomState 		= ZOOMING_OUT;
		zoomTo    		= zoomFarthest;		
	}

	bForceAZoomOut 	= false; // the zoom has to be forced out each frame to work!
	
	// Set the current FOV
	switch(zoomState)
	{
		case ZOOMING_IN :										
			if (CMaths::Abs(curFOV-localZoomNearest)<1.0f)
			{
				zoomState = ZOOMED_IN;				
				curFOV = localZoomNearest;
				bHasZoomedIn = true;	
			}	
			else
			{
				float t = (time-timeZoomStarted)/durationFOVZoom;
				curFOV = DW_SINE_ACCEL_DECEL_LERP(t,zoomFrom,zoomTo);
			}	
			break;
		case ZOOMING_OUT:
			if (CMaths::Abs(curFOV-zoomFarthest)<1.0f)
			{
				zoomState = ZOOMED_OUT;
				curFOV = zoomFarthest;
			}
			else
			{
				float t = (time-timeZoomStarted)/durationFOVZoom;
				curFOV = DW_SINE_ACCEL_DECEL_LERP(t,zoomFrom,zoomTo);
			}												
			break;
		case ZOOMED_IN : 
			curFOV = localZoomNearest;
			break;
		case ZOOMED_OUT : 
			curFOV = zoomFarthest;
			break;
		default :
			ASSERT(false);
			break;
	}	
	
	
	pCam->FOV = curFOV;
}

//------------------------------
void CIdleCam::ProcessIdleCamTicker()
{
	if (lastTimePadTouched == CPad::GetPad(0)->LastTimeTouched)
	{
		idleTickerFrames+=CTimer::GetTimeElapsedInMilliseconds(); // DW - chnaged for PC version // should this be the clipped version
	}
	else
	{
		lastTimePadTouched = CPad::GetPad(0)->LastTimeTouched;
		idleTickerFrames=0;
	}
	
}

//------------------------------
bool CIdleCam::IsItTimeForIdleCam()
{
	if (idleTickerFrames>timeControlsIdleForIdleToKickIn) // DW - chnaged for PC version
	{
		return true;
	}
	else
	{
		return false;
	}
	
//	float time 		= CTimer::GetTimeInMilliseconds();
//	float deltaTime = time - CPad::GetPad(0)->LastTimeTouched;				
//	return (deltaTime>timeControlsIdleForIdleToKickIn);
}

//------------------------------
void CIdleCam::GetLookAtPositionOnTarget(CEntity *pTest,CVector *pVec)
{
	ASSERT(pTest);
	*pVec = pTest->GetPosition();
			
	if (pTest->GetIsTypePed())
	{
		CPed *pPed = (CPed*)pTest;
		if (CPedType::IsFemaleType(pPed->GetPedType()))
		{
			static float femalePartsOffset = 0.1f;
			pVec->z += femalePartsOffset;	
		}
		else
		{			
			pVec->z += 0.5f;	
		}
	}
}

//------------------------------
void CIdleCam::SetTargetPlayer()
{
	float time = CTimer::GetTimeInMilliseconds();
	
	CEntity *pPlayer = (CEntity*)FindPlayerPed();	
	ASSERT(pPlayer);	

	SetTarget(pPlayer);
	DW_PRINTF("Player set to target\n");																									
	
	bForceAZoomOut = true;
}

//-------------------------
// Safely change the target
void CIdleCam::SetTarget(CEntity *pEntity)
{
	float time = CTimer::GetTimeInMilliseconds();
		
	if (pTarget)
	{
		positionToSlerpFrom = lastIdlePos;
		DW_PRINTF("Slerping form last look at pos\n");																									
		
		//GetLookAtPositionOnTarget(pTarget,&positionToSlerpFrom);
	}
	else
	{
		DW_PRINTF("Slerping from camera matrix\n");																									
	
		positionToSlerpFrom = pCam->Source + pCam->Front;
	}


	if (pEntity)
	{
		if (pTarget)
			TIDYREF(pTarget,(CEntity **)&pTarget);						
		
		pTarget = pEntity;				
		REGREF(pTarget, (CEntity **)&pTarget);		
	}
	else
	{
		if (pTarget)
		{
			TIDYREF(pTarget,(CEntity **)&pTarget);
			pTarget = NULL;		
		}
	}

	timeSlerpStarted	   = time;
	timeLastTargetSelected = time;	
	
	targetLOSCounter 	   	= 0; // reset collision counter.
	bHasZoomedIn 			= false;
}

//-------------------------------------------------------------
// From a vector get the angles in world Rotation about Z and X
void CIdleCam::VectorToAnglesRotXRotZ(CVector *pV,float *pA, float *pB)
{
	*pB = CGeneral::GetATanOfXY(pV->x,pV->y) + PI;
	float gd = CMaths::Sqrt(pV->x*pV->x + pV->y*pV->y);	
	*pA = CGeneral::GetATanOfXY(gd,pV->z);
}

CVector LookatOffset(0,0,0.326);
TweakFloat SWIM_CAM_ALPHA_FORCE = 0.5f;
TweakFloat SWIM_CAM_ALPHA_EXTRA = DEGTORAD(-15.0f);
TweakFloat JETPACK_CAM_ALPHA_FORCE = 3.0f;
TweakFloat JETPACK_CAM_ALPHA_EXTRA = DEGTORAD(-20.0f);
//
#pragma mark ----follow-ped----------------
//
void CCam::Process_FollowPed_SA(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired, bool bScriptSetAngles)
{
	if(!CamTargetEntity->GetIsTypePed())
		return;

	if(!((CPed *)CamTargetEntity)->IsPlayer())
		return;
	
	CPed *pPed = (CPed *)CamTargetEntity;
	CPad *pPad = CPad::GetPad(0);
	if(pPed->GetPedType()==PEDTYPE_PLAYER2)
		pPad = CPad::GetPad(1);
	
	CVector vecTargetCoords = ThisCamsTarget;
	bool bIsGettingIntoCar = false;	
	
	int32 nType = FOLLOW_PED_OUTSIDE;
	// for now consider everything except the main map as an interior
	if(CGame::currArea!=AREA_MAIN_MAP)
		nType = FOLLOW_PED_INSIDE;
		
	float distToPlayer = PEDCAM_SET[nType].fBaseCamDist;
	if (!pPed->GetIsStanding() && TheCamera.m_nPedZoom==ZOOM_THREE)
	{
		bool bIsParachuting = pPed->GetPedIntelligence()->GetUsingParachute();	
		if (bIsParachuting)
		{
			distToPlayer *=2.0f;
		}
	}
	
	float fCamDistance = TheCamera.m_fPedZoomSmoothed + distToPlayer;
	float fMinDistance = PEDCAM_SET[nType].fMinDist;
	float fMinFollowDist = PEDCAM_SET[nType].fMinFollowDist;

	float fUpLimit 		= PEDCAM_SET[nType].fUpLimit;
	float fDownLimit 	= PEDCAM_SET[nType].fDownLimit;
		
	//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
	// make mode changes  (via select key) apparent which mode you are in - DW
	if(fCamDistance > gLastCamDist)
		fMinDistance = fCamDistance;		
	gLastCamDist = fCamDistance;
	//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

	float fCamAlpha = PEDCAM_SET[nType].fBaseCamZ;
	if(TheCamera.m_nPedZoom==ZOOM_ONE)
		fCamAlpha += m_fTargetZoomOneZExtra;
	else if(TheCamera.m_nPedZoom==ZOOM_TWO)
	{
		if(nType==FOLLOW_PED_INSIDE)
			fCamAlpha += m_fTargetZoomTwoInteriorZExtra;
		else
			fCamAlpha += m_fTargetZoomTwoZExtra;
	}
	else if (TheCamera.m_nPedZoom==ZOOM_THREE)
		fCamAlpha += m_fTargetZoomThreeZExtra;
	
	float fForceCamAlphaMult = 0.0f;
	float fForceCamBetaMult = 0.0f;
	if(pPed->GetPedIntelligence()->GetTaskSwim())
	{
		fForceCamBetaMult = 1.0f;
		if(pPed->GetPedIntelligence()->GetTaskSwim()->m_nSwimState != SWIM_UNDERWATER)
		{
			fForceCamAlphaMult = SWIM_CAM_ALPHA_FORCE;
		}
	}
	else if(pPed->GetPedIntelligence()->GetTaskJetPack())
	{
		fForceCamBetaMult = 0.5f;
		if(!pPed->GetIsStanding())
		{
			fForceCamAlphaMult = JETPACK_CAM_ALPHA_FORCE;
		}
	}

	if(TheCamera.m_uiTransitionState==CCamera::TRANS_NONE)
	{
		float fDesiredFOV = 70.0f;
		if(ResetStatics)
			FOV = fDesiredFOV;
		else 
		{
			float fRate = AIMWEAPON_FOV_ZOOM_RATE*CTimer::GetTimeStep();
			if(fDesiredFOV > FOV + fRate)
				FOV += fRate;
			else if(fDesiredFOV < FOV - fRate)
				FOV -= fRate;
			else
				FOV = fDesiredFOV;
		}
	}

	vecTargetCoords.z += PEDCAM_SET[nType].fTargetOffsetZ;
	float fFollowDist = MAX(fCamDistance, fMinFollowDist);
	
	// because camera has been re-initialised
	if(ResetStatics || TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront || bScriptSetAngles)
	{
		// DW - not required for PC version
//		ResetFirstPersonUpCastFixer(); // make sure when you go back into a car this upcast hack fixer has been kicked in, look behind interferes with its operation is bizarre ways....hence this is here...urgh
	
		if(bScriptSetAngles)
		{
			vecPedPosEst = pPed->GetPosition();
			vecPedPosTrend = CVector(0.0f, 0.0f, 0.0f);
			
			vecTargetCoords = pPed->GetPosition();
			vecTargetCoords.z += PEDCAM_SET[nType].fTargetOffsetZ;
		}
	
		TheCamera.ResetDuckingSystem(pPed);
		Rotating=FALSE;
		gForceCamBehindPlayer = false;
		m_bCollisionChecksOn=true;
		if(!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
		{
			Beta = pPed->GetHeading() - HALF_PI;
			if(TheCamera.m_bCamDirectlyInFront)
				Beta += PI;
		}
		
		BetaSpeed = 0.0f;
		AlphaSpeed = 0.0f;
		Distance = 1000.0f;
		
		if(fCamDistance == TheCamera.m_fPedZoomBase)
			fCamDistance = TheCamera.m_fPedZoomSmoothed = TheCamera.m_fPedZoomTotal;

		if(!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
		{
			Alpha = 0.0f;
			if(pPed->GetIsStanding())
			{
				float fGroundNormalFwd = DotProduct(pPed->m_vecGroundNormal, pPed->GetMatrix().GetForward());
				Alpha -= CMaths::ASin(MIN(1.0f, MAX(-1.0f, fGroundNormalFwd)));
			}
		}

		Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));

		// we want to store the last camera position always without including its alpha offset
		m_aTargetHistoryPos[0] = vecTargetCoords - fFollowDist*Front;
		m_nTargetHistoryTime[0] = CTimer::GetTimeInMilliseconds();
		m_aTargetHistoryPos[1] = vecTargetCoords - fCamDistance*Front;
		m_nCurrentHistoryPoints = 0;

		if(!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
			Alpha = -fCamAlpha;
		
		if(pPed->GetPedIntelligence()->GetTaskSwim() && pPed->GetPedIntelligence()->GetTaskSwim()->m_nSwimState != SWIM_UNDERWATER)
			Alpha += SWIM_CAM_ALPHA_EXTRA;
		else if(pPed->GetPedIntelligence()->GetTaskJetPack())
			Alpha += JETPACK_CAM_ALPHA_EXTRA;

		#ifdef GTA_PC
			CPad::GetPad(0)->ClearMouseHistory();
		#endif
	}
	// test moving the camera along when standing on trains
	else if(pPed->m_pGroundPhysical)
	{
		if((pPed->m_pGroundPhysical->GetIsTypeVehicle() && ((CVehicle *)pPed->m_pGroundPhysical)->GetVehicleType()==VEHICLE_TYPE_TRAIN)
		|| (pPed->m_pGroundPhysical->m_pAttachToEntity && pPed->m_pGroundPhysical->m_pAttachToEntity->GetIsTypeVehicle() && ((CVehicle *)pPed->m_pGroundPhysical->m_pAttachToEntity)->GetVehicleType()==VEHICLE_TYPE_TRAIN))
		{
			static float AMOUNT_OF_SPEED_TO_ADD = 0.01f;
			float fMagnitude = pPed->m_pGroundPhysical->GetMoveSpeed().Magnitude();
			fMagnitude = CMaths::Max(0.0f, fMagnitude - AMOUNT_OF_SPEED_TO_ADD) / CMaths::Max(AMOUNT_OF_SPEED_TO_ADD, fMagnitude);
			m_aTargetHistoryPos[0] += fMagnitude*pPed->m_pGroundPhysical->GetMoveSpeed()*CTimer::GetTimeStep();
		 	m_aTargetHistoryPos[1] += fMagnitude*pPed->m_pGroundPhysical->GetMoveSpeed()*CTimer::GetTimeStep();
		}		
	}

	Front = vecTargetCoords - m_aTargetHistoryPos[0];
	Front.Normalise();
	float fTempLength = (vecTargetCoords - m_aTargetHistoryPos[1]).Magnitude();	
	if(fTempLength < fCamDistance && fCamDistance > PEDCAM_SET[nType].fMinDist)
		fCamDistance = MAX(fMinDistance, fTempLength);
	

	float fTargetDiff = 0.0f;
	float fDiffMult = 0.0f;
	float fDiffCap = 0.0f;
	float fTargetBeta = CMaths::ATan2(-Front.x, Front.y) - HALF_PI;
	if(fTargetBeta < -PI)	fTargetBeta += TWO_PI;
	
	// want to swing the camera around by a variable percentage to match the players heading
	float fHeadingBeta = pPed->GetHeading() - HALF_PI;
	if(fHeadingBeta - fTargetBeta > PI)	fHeadingBeta -= TWO_PI;
	else if(fHeadingBeta -fTargetBeta < -PI)	fHeadingBeta += TWO_PI;
	
	if(pPad->ForceCameraBehindPlayer())
		gForceCamBehindPlayer = true;
	else if(gForceCamBehindPlayer)
	{
		if(pPed->GetMoveSpeed().MagnitudeSqr() > 0.001f || CMaths::Abs(fHeadingBeta - fTargetBeta) < 0.01f || pPad->AimWeaponLeftRight(pPed) != 0 || pPad->AimWeaponUpDown(pPed) != 0)
		{
			gForceCamBehindPlayer = false;
		}
	}
	
	static float HEADING_TOWARD_PLAYER_BETA_LIMIT = DEGTORAD(170.0f);
	if(CMaths::Abs(fHeadingBeta - fTargetBeta) < HEADING_TOWARD_PLAYER_BETA_LIMIT || gForceCamBehindPlayer || fForceCamBetaMult)
	{
		fDiffMult = PEDCAM_SET[nType].fDiffBetaSwing*CTimer::GetTimeStep();
		fDiffCap = PEDCAM_SET[nType].fDiffBetaSwingCap*CTimer::GetTimeStep();
		if(gForceCamBehindPlayer || fForceCamBetaMult)
		{
			static float FORCE_CAM_SPEED_MULT = 0.5f;
			fDiffMult = MIN(1.0f, fDiffMult*FORCE_CAM_SPEED_MULT);
			static float FORCE_CAM_CAP_MULT = 2.0f;
			fDiffCap *= FORCE_CAM_CAP_MULT;

			if(fForceCamBetaMult)
			{
				fDiffMult *= fForceCamBetaMult;
				fDiffCap *= fForceCamBetaMult;
			}
		}
		else
		{
			if(pPed->m_pGroundPhysical)
				fDiffMult = MIN(1.0f, (pPed->GetMoveSpeed() - pPed->m_pGroundPhysical->GetMoveSpeed()).Magnitude()*fDiffMult);
			else
				fDiffMult = MIN(1.0f, pPed->GetMoveSpeed().Magnitude()*fDiffMult);
		}
		
		fTargetDiff = max ( min ( fDiffMult*(fHeadingBeta - fTargetBeta), fDiffCap ), -fDiffCap );
	}
	
	fTargetBeta += fTargetDiff;
	if(fTargetBeta > Beta + PI)	fTargetBeta -= TWO_PI;
	else if(fTargetBeta < Beta - PI)	fTargetBeta += TWO_PI;
	float fCamControlBetaSpeed = (fTargetBeta - Beta)/CMaths::Max(1.0f, CTimer::GetTimeStep());
	

	float fTargetAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, Front.z)));

// WARNING! this code is supposed to help the camera swing up as the target goes over the brow of hills
// but it seems to be causing some jerks in the camera motion so I need to figure out a better way to do this
//	if(fTargetAlpha < Alpha && fTempLength > fCamDistance)
//	{
//		fTargetAlpha = CMaths::ASin((fCamDistance / fTempLength)*CMaths::Sin(fTargetAlpha));
//	}

	// limit alpha when the player is walking towards the camera
	static float HEADING_TOWARD_PLAYER_FOR_ALPHA = HALF_PI;
	static float HEADING_TOWARD_PLAYER_ALPHA_MAX = DEGTORAD(20.0f);
	if(CMaths::Abs(fHeadingBeta - fTargetBeta) > HEADING_TOWARD_PLAYER_FOR_ALPHA && pPed->GetMoveSpeed().MagnitudeSqr() > 0.002f)
	{
		float fAlphaLimit = CMaths::Abs(fHeadingBeta - fTargetBeta) - HEADING_TOWARD_PLAYER_FOR_ALPHA;
		fAlphaLimit = CMaths::Min(1.0f, 1.2f*fAlphaLimit / (PI - HEADING_TOWARD_PLAYER_FOR_ALPHA));
		fAlphaLimit = HALF_PI - (HALF_PI - HEADING_TOWARD_PLAYER_ALPHA_MAX)*fAlphaLimit;
		
		static float HEADING_TOWARD_PLAYER_ALPHA_RATE = 0.90f;
		float fRate = CMaths::Pow(HEADING_TOWARD_PLAYER_ALPHA_RATE, CTimer::GetTimeStep());
		
		if(fTargetAlpha > fAlphaLimit)
			fTargetAlpha = fRate*fTargetAlpha + (1.0f - fRate)*fAlphaLimit;
		else if(fTargetAlpha < -fAlphaLimit)
			fTargetAlpha = fRate*fTargetAlpha - (1.0f - fRate)*fAlphaLimit;
	}

	fTargetDiff = 0.0f;
	if(fForceCamAlphaMult || (gForceCamBehindPlayer && pPed->GetIsStanding()))
	{
		float fGroundAlpha = 0.0f;
		if(pPed->GetPedIntelligence()->GetTaskJetPack())
		{
			fGroundAlpha += JETPACK_CAM_ALPHA_EXTRA;
		}
		else if(pPed->GetPedIntelligence()->GetTaskSwim())
		{
			fGroundAlpha += SWIM_CAM_ALPHA_EXTRA;
		}
		else if(pPed->GetIsStanding())
		{
			float fGroundNormalFwd = DotProduct(pPed->m_vecGroundNormal, pPed->GetMatrix().GetForward());
			fGroundAlpha = -CMaths::ASin(MIN(1.0f, MAX(-1.0f, fGroundNormalFwd)));
		}

		static float FORCE_CAM_ALPHA_SPEED_MULT = 1.0f;
		fDiffMult = MIN(1.0f, fDiffMult*FORCE_CAM_ALPHA_SPEED_MULT);
		static float FORCE_CAM_ALPHA_CAP_MULT = 4.0f;
		fDiffCap *= FORCE_CAM_ALPHA_CAP_MULT;

		if(fForceCamAlphaMult)
		{
			fDiffMult *= fForceCamAlphaMult;
			fDiffCap *= fForceCamAlphaMult;
		}
		
		fTargetDiff = max ( min ( fDiffMult*(fGroundAlpha - fTargetAlpha), fDiffCap ), -fDiffCap );
	}
	
	fTargetAlpha += fTargetDiff;
	fTargetAlpha -= fCamAlpha;

	if(fTargetAlpha > fUpLimit)
		fTargetAlpha = fUpLimit;
	else if(fTargetAlpha < -fDownLimit)
		fTargetAlpha = -fDownLimit;
	
	fDiffMult = CMaths::Pow(PEDCAM_SET[nType].fDiffAlphaRate, CTimer::GetTimeStep());
	fDiffCap = PEDCAM_SET[nType].fDiffAlphaCap*CTimer::GetTimeStep();

	fTargetDiff = max ( min ( (1.0f - fDiffMult)*(fTargetAlpha - Alpha), fDiffCap ), -fDiffCap );
	
	
////////////////
	float StickBetaOffset = -(pPad->AimWeaponLeftRight(pPed));
	float StickAlphaOffset = pPad->AimWeaponUpDown(pPed);	
	
	if (bIsGettingIntoCar)
	{
	 	StickBetaOffset = 0;
	 	StickAlphaOffset = 0;
	}	
	// if player is firing gun without aiming (without R1) and it's a 2 handed gun
	// then let them rotate the camera with the left stick to aim (otherwise the left stick does nothing)
	else if(pPed->GetPedIntelligence()->GetTaskUseGun() && pPed->GetPedIntelligence()->GetTaskUseGun()->GetIsFiring()
	&& pPed->GetPedIntelligence()->GetTaskUseGun()->GetWeaponInfo() && !pPed->GetPedIntelligence()->GetTaskUseGun()->GetWeaponInfo()->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM))
	{
		if(CMaths::Abs(pPad->GetPedWalkLeftRight()) > CMaths::Abs(StickBetaOffset))
			StickBetaOffset = -(pPad->GetPedWalkLeftRight());
	}
		
	if ((StickAlphaOffset || StickBetaOffset) && !(pPad->GetPedWalkLeftRight() || pPad->GetPedWalkUpDown()))
	{
		CPed* pPlayerPed = FindPlayerPed(0);
		CVector playerForward = pPlayerPed->GetMatrix().GetForward();
		CVector cameraForward = TheCamera.GetForward();
	
		if (DotProduct(playerForward, cameraForward)>0.3f)
		{		
			CVector lookAtPos = pPlayerPed->GetPosition() + (5.0f*cameraForward);
			g_ikChainMan.LookAt("FollowPedSA", pPlayerPed, NULL, 1500, -1, (RwV3d*)&lookAtPos, false);//, 0.25, 300);
		}
	}
			
	StickBetaOffset = AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(StickBetaOffset)*StickBetaOffset * (0.25f/3.5f * (FOV/80.0f));// * CTimer::GetTimeStep();
	StickAlphaOffset = AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(StickAlphaOffset)*StickAlphaOffset * (0.15f/3.5f * (FOV/80.0f));// * CTimer::GetTimeStep();
	
	// let the the climb task provide stick inputs to the camera to make it swing up while we're vaulting over stuff
	if(pPed->GetPedIntelligence()->GetTaskClimb())
	{
		pPed->GetPedIntelligence()->GetTaskClimb()->GetCameraStickModifier(pPed, Alpha, Beta, StickAlphaOffset, StickBetaOffset);
	}
	// let enter car task swing camera around for entering car
	else if(pPed->GetPedIntelligence()->GetTaskActive()->GetTaskType()==CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER)
	{
		((CTaskComplexEnterCar *)pPed->GetPedIntelligence()->GetTaskActive())->GetCameraStickModifier(pPed, fCamDistance, Alpha, Beta, StickAlphaOffset, StickBetaOffset);
	}

	fDiffMult = CMaths::Pow(PEDCAM_SET[nType].fDiffBetaRate, CTimer::GetTimeStep());
	fDiffCap = PEDCAM_SET[nType].fDiffBetaCap;

	fCamControlBetaSpeed += StickBetaOffset;

	if(fCamControlBetaSpeed > fDiffCap)
		fCamControlBetaSpeed = fDiffCap;
	else if(fCamControlBetaSpeed < -fDiffCap)
		fCamControlBetaSpeed = -fDiffCap;

	BetaSpeed = fDiffMult*BetaSpeed + (1.0f - fDiffMult)*fCamControlBetaSpeed;

	if (bIsGettingIntoCar)
	{
		BetaSpeed = 0;
	}

	static float gBetaSpeedTol  =0.0001f;		
	if (CMaths::Abs(BetaSpeed)<gBetaSpeedTol)
		BetaSpeed = 0.0f;

#ifdef GTA_PC
	if(TheCamera.m_bUseMouse3rdPerson && pPad->DisablePlayerControls==0)
	{
		float fStickX = -pPad->GetAmountMouseMoved().x * 2.5f;
		StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
		Beta+= StickBetaOffset;
		BetaSpeed = 0.0f;
	}
	else
#endif
	{
		Beta += BetaSpeed*CTimer::GetTimeStep();
	}
	
//	if (TheCamera.m_bJustCameOutOfGarage) // dont want to change alpha and beta!... hack hack hack
//		Beta = CGeneral::GetATanOfXY(Front.x,Front.y);
	
	ClipBeta();

	AlphaSpeed = fDiffMult*StickAlphaOffset + (1.0f - fDiffMult)*AlphaSpeed;
	if(AlphaSpeed > fDiffCap)
		AlphaSpeed = fDiffCap;
	else if(AlphaSpeed < -fDiffCap)
		AlphaSpeed = -fDiffCap;

	static float gAlphaSpeedTol  =0.0001f;		
	if (CMaths::Abs(AlphaSpeed)<gAlphaSpeedTol)
		AlphaSpeed = 0.0f;

	if (bIsGettingIntoCar)
	{
		AlphaSpeed = 0;
	}

	fTargetAlpha += AlphaSpeed*CTimer::GetTimeStep();
	
#ifdef GTA_PC
    
	if(TheCamera.m_bUseMouse3rdPerson && pPad->DisablePlayerControls==0)
	{
		float fStickY = pPad->GetAmountMouseMoved().y * 2.5f;
		StickAlphaOffset = TheCamera.m_fMouseAccelHorzntl * fStickY * (FOV/80.0f);

		// if the Camera's faded out and it's now fading in - want pitch to spring to -0.22 Rad
		if( (TheCamera.GetFading() && TheCamera.GetFadingDirection() == FADING_IN && CDraw::FadeValue > 45) || CDraw::FadeValue > 200)
	    {
	    	float fDefaultAlphaOrient = -fCamAlpha;
	    
	    	if(Alpha < fDefaultAlphaOrient - 0.05f)
	    		StickAlphaOffset = 0.05f;
	    	else if(Alpha < fDefaultAlphaOrient)
	    		StickAlphaOffset = fDefaultAlphaOrient - Alpha;
	    	else if(Alpha > fDefaultAlphaOrient + 0.05f)
	    		StickAlphaOffset = -0.05f;
	    	else if(Alpha > fDefaultAlphaOrient)
	    		StickAlphaOffset = fDefaultAlphaOrient - Alpha;
	    	else
	    		StickAlphaOffset = 0.0f;
		}

		Alpha += StickAlphaOffset;
		AlphaSpeed = 0.0f;
	}
	else
#endif
	{
		Alpha += fTargetDiff;
	}

	if(Alpha > fUpLimit)
	{
		Alpha = fUpLimit;
		AlphaSpeed = 0.0f;
	}
	else if(Alpha < -fDownLimit)
	{
		Alpha = -fDownLimit;
		AlphaSpeed = 0.0f;
	}
////////////////
	
	static float gOldAlpha = -9999.0f;
	static float gOldBeta = -9999.0f;
	static float gAlphaTol = 0.0001f;
	static float gBetaTol = 0.0001f;
	
	float dA = CMaths::Abs(gOldAlpha - Alpha);
	if (dA<gAlphaTol)
		Alpha = gOldAlpha;
	gOldAlpha = Alpha;		
	
	float dB = CMaths::Abs(gOldBeta - Beta);
	if (dB<gBetaTol)
		Beta = gOldBeta;
	gOldBeta = Beta;		


#if 0		
	// Right ... plan is to move the camera to a nice position to view the player gettting into the car.
	// once finished the camera is reset BEHIND the car.
	// My plan is to quaternion slerp the camera from one orientation to another.
	// First I need to find good positions for the camera... fucking hard!
	// then I need to choose the best of the good positions... also hard!
	// then I slerp to it, but the car can be moving and this means the collision must be calculated each frame, so basically it might be best to go straight to the follow 
	// car camera with fixed alpha and beta, then to rotate this alpha and beta to zero upon hitting 0 or the default control is passed back to player.
		
	// SUPER IDEA... rotate alpha and beta to fixed value that will work for most vehicles.
	static float DestAlphaOffset = DEGTORAD(45.0f);
	static float DestBeta 	= 1.0f;
	static float multiplierCarJack = 0.1f;


	static float lastValidAlpha = 0.0f;
	static float lastValidBeta = 0.0f;
	static int32 timeAlphaWasOK = 0.0f;
	static int32 timeAlphaLerp = 1;

	CTask* pTask = pPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_COMPLEX_ENTER_CAR_AS_DRIVER);
	//Ped is getting in as a driver.
	if(pTask)
	{
		CTaskComplexEnterCarAsDriver *pDriverTask = (CTaskComplexEnterCarAsDriver*)pTask;
				
		int32 doorVal = 1;//pDriverTask->GetTargetDoor(); // gordon to write...
		if (doorVal) // non zero is ( target attained) 
		{
			bool bDoorIsRight = doorVal == CAR_DOOR_RF;
							
	     	bIsGettingIntoCar = true; 
			CTaskComplexEnterCarAsDriver* pTaskEnter=(CTaskComplexEnterCarAsDriver*)pTask;
			CVehicle* pTargetVehicle=pTaskEnter->GetTargetVehicle();
	     	CEntity *pEntGettingInto = pTargetVehicle;     	  

			static CVector carRelVec[3] = { CVector(-1.0f,0.0f,0.5f),
											CVector(-1.0f,-0.5f,0.5f),
											CVector(-1.0f,0.5f,0.5f) };
			
			static int32 idxCarRelVec = 0;
			
	//		idxCarRelVec++;
			if (idxCarRelVec >= 3)
				idxCarRelVec = 0;				
						
			CVector crv(-carRelVec[idxCarRelVec].y,carRelVec[idxCarRelVec].x,carRelVec[idxCarRelVec].z); // rotate 90 degrees fark knows why
					
			CMatrix mat = pEntGettingInto->GetMatrix();		
			CVector rotCarRelVec = Multiply3x3(mat,crv);
			
			// now take this world vector and calculate by doing first principle equations the Alpha and beta		

			float len = CMaths::Sqrt((rotCarRelVec.x*rotCarRelVec.x) + (rotCarRelVec.y*rotCarRelVec.y));
			float vecBeta  = CMaths::ATan2(-rotCarRelVec.x, rotCarRelVec.y);  // yaw   - rotation about world up vector
			float vecAlpha = CMaths::ATan2(-rotCarRelVec.z, len);
			MakeAngleLessThan180(vecBeta);
			MakeAngleLessThan180(vecAlpha);			
		
			float time = CTimer::GetTimeInMilliseconds();
			float t = (time-timeAlphaWasOK)/timeAlphaLerp;
			if 		(t<0.0f) 	t =	0.0f;
			else if (t>1.0f) 	t = 1.0f;

			static bool gChangeAlpha = true;
			static bool gChangeBeta  = true;

			float d = vecAlpha - lastValidAlpha;
			if (CMaths::Abs(d)>PI)
			{
				if (vecAlpha<0)
					vecAlpha += TWO_PI;
				
				if (lastValidAlpha<0)
					lastValidAlpha += TWO_PI;
			}

			d = vecBeta - lastValidBeta;
			if (CMaths::Abs(d)>PI)
			{
				if (vecBeta<0)
					vecBeta += TWO_PI;
				
				if (lastValidBeta<0)
					lastValidBeta += TWO_PI;
			}


			if (timeAlphaLerp<=1)
			{
				Alpha = vecAlpha;
				Beta = vecBeta;
			}
			else
			{
				if (gChangeAlpha)
					Alpha 	= DW_LERP(t,lastValidAlpha,vecAlpha);   
				if (gChangeBeta)
					Beta 	= DW_LERP(t,lastValidBeta,vecBeta);   		  	 	 
			}


			char str[255];
			sprintf(str,"A %3.3f TA %3.3f",RADTODEG(Alpha),RADTODEG(vecAlpha));
			VarConsole.AddDebugOutput(str);	
			sprintf(str,"B %3.3f TB %3.3f",RADTODEG(Beta),RADTODEG(vecBeta));
			VarConsole.AddDebugOutput(str);	
			
			TheCamera.m_bCamDirectlyBehind = true; // this should ensure that the camera will go directly behind once in follow car mode.
		}
	}

	if (bIsGettingIntoCar)
	{	
	}
	else
	{
		timeAlphaWasOK = CTimer::GetTimeInMilliseconds();
		lastValidAlpha = Alpha;
		lastValidBeta = Beta;		
	}
#endif

	Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
	GetVectorsReadyForRW();

	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

	Source  = vecTargetCoords - Front * fCamDistance;
	
	VecTrunc(&Source);
	
	fTargetAlpha += fCamAlpha;
	m_aTargetHistoryPos[0] = vecTargetCoords - fFollowDist * CVector(-CMaths::Cos(Beta) *CMaths::Cos(fTargetAlpha) , -CMaths::Sin(Beta)*CMaths::Cos(fTargetAlpha), CMaths::Sin(fTargetAlpha));
	m_aTargetHistoryPos[1] = vecTargetCoords - fCamDistance * CVector(-CMaths::Cos(Beta) *CMaths::Cos(fTargetAlpha) , -CMaths::Sin(Beta)*CMaths::Cos(fTargetAlpha), CMaths::Sin(fTargetAlpha));
	
	if(pPad->ForceCameraBehindPlayer() && pPad->AimWeaponLeftRight(NULL))
	{
		float fHeadingDiff = Beta - (pPed->GetCurrentHeading() - HALF_PI);
		if(fHeadingDiff > PI)	fHeadingDiff -= TWO_PI;
		else if(fHeadingDiff < -PI)	fHeadingDiff += TWO_PI;
		
		if(CMaths::Abs(fHeadingDiff) < 0.1f*CTimer::GetTimeStep())
			pPed->SetDesiredHeading(Beta + HALF_PI);
	}


	//==============================================================
	// DW - HANDLE DUCKING - and transtitions
	TheCamera.HandleCameraMotionForDucking(pPed, &Source, &vecTargetCoords);
	// DW - END HANDLE DUCKING
	//==============================================================	
	// need to do this after HandleCameraMotionForDucking or the interpolation messes up when ducked
	m_cvecTargetCoorsForFudgeInter = vecTargetCoords;
	

	VecTrunc(&Source);
	
//////////////////////////////////////
// this chunk of code is pretty tempory, kinda copied from original follow ped
// and used in Process_AimWeapon(), Process_Follow_History(), and new Process_FollowPed()
// as a quick fix to stop the camera clipping through shit
// would like to smooth it all out and do some kind of predictive moving the camera forward like
// in MH, but don't want to start fucking around with the camera angles because that gets messy
//////////////////////////////////////
#ifdef DW_CAMERA_COLLISION
	TheCamera.SetColVarsPed(nType,TheCamera.m_nPedZoom);
	if (DirectionIsLooking==LOOKING_FORWARD)
	{
		// Ped follow cam			
		TheCamera.CameraGenericModeSpecialCases(pPed);
		TheCamera.CameraPedModeSpecialCases(pPed);	
		TheCamera.CameraColDetAndReact(&Source,&vecTargetCoords);		
		TheCamera.ImproveNearClip(NULL,pPed,&Source,&vecTargetCoords);								
		
		VecTrunc(&Source);		
	}
#else
	// See at bottom of file comment @BLOCK 2@
#endif

	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

	VecTrunc(&Source);
	
	GetVectorsReadyForRW();
		
#ifndef MASTER
/*	char str[255];
	sprintf(str,"FRONT  %3.9f %3.9f %3.9f",Front.x,Front.y,Front.z);
	VarConsole.AddDebugOutput(str);	
	sprintf(str,"UP     %3.9f %3.9f %3.9f",Up.x,Up.y,Up.z);
	VarConsole.AddDebugOutput(str);	
	sprintf(str,"SOURCE %3.9f %3.9f %3.9f",Source.x,Source.y,Source.z);
	VarConsole.AddDebugOutput(str);	
	sprintf(str,"A %3.9f B %3.9f",Alpha,Beta);
	VarConsole.AddDebugOutput(str);	
	sprintf(str,"COL %3.9f",gCurDistForCam);
	VarConsole.AddDebugOutput(str);	
*/	
#endif

	bool bProcessedIdleCam = false;
	if (nType == FOLLOW_PED_OUTSIDE && TheCamera.WhoIsInControlOfTheCamera!=SCRIPT_CAM_CONTROL && pPed->GetIsStanding() && !CGameLogic::IsCoopGameGoingOn())
	{
		if (!TheCamera.m_bFOVScript && !TheCamera.m_bVectorMoveScript && !TheCamera.m_bVectorTrackScript)
		{
			float v = pPed->m_vecMoveSpeed.MagnitudeSqr();
			if (v<=0.01f*0.01f) // no idle cam when moving!
			{
				gIdleCam.Process();	
				bProcessedIdleCam = true;		
			}
		}
	}
	
	if (!bProcessedIdleCam)
	{
		gIdleCam.idleTickerFrames = 0;
	}
	
	
//		PossiblyProcessIdleCam();
	

	ResetStatics = false;
////////////////////////////
// end of nasty chunk
////////////////////////////
}

#if 0
// Determines a period of user inactivity that would signify time to 
// change camera to idle camera
static 	bool bForce = false;
static 	float timeToKickOffIdleCam = 60000; 		// one minute - the idle time to wait to kick off the idle cam
float 	gTimeLastIdleTargetSelected = 0.0f;		// time a target was selected so we dont keep changing tragets too frequently
float 	gMaxDurationForAnIdleCam = 12000.0f;	// The duration minimum to wait before choosing another target		
CVehicle* pIdleTargetVehicle;					// A persistent pointer to a target vehicle we have chosen to watch
CPed* 	pIdleTargetPed;							// A persistent pointer to a target ped we have chosen to watch
CVector SlerpFromPos;							// A position that we will interpolation from 
float 	gTimeSlerpStarted = 0.0f;				// When did the last slerp start
float 	timeToSlerpFromOneToAnother = 2000.0f;	// Time for a slerp to last
float 	outOfSightTime = 0.0f;					// 
bool 	bWasOutOfSight = false;					// Was the traget out of sight last frame?
float 	TargetHasGoneTime = 3000.0f;			// Time before giving up on object becoming visible again.
float 	SlerpFromRotX;							
float 	SlerpFromRotZ;
float 	framesInThisMode = 0.0f;				// A way of determining the time we have been consistenly processing Idle can, so we can fade up the shake smoothly.
float 	framesToInterpolateUpTheShake = 30.0f;	// We interpolate the degree of shake up over this number of frames
bool 	bWasFOVZooming = false;					// Was the FOV in a zomming transition last frame
float 	FOVZoomIn = 20.0f;						// As far as we zoom in.
float 	FOVZoomOut = 70.0f;						// normal zoom setting
float 	cachedFOV = 70.0f;						// The cached FOV, FOV is snapshotted to start a lerp from
float	doubleCachedFOV = 70.0f;				// dont ask!
float 	timeFOVZoomStarted = 0.0f;				// When did FOV zoom start?
float 	timeFOVZoomDuration = 800.0f;			// How long to complete an FOV zoom?
float 	RandDegreeOfVehicleSelection = 0.5f;	// how often will we choose to watch a vehicle over a ped... 0.5 is 50%
float 	distanceToRejectObject0 = 5.0f;			// how near can a target be to camera?
float 	distanceToRejectObject1 = 15.0f;		// the medium distance (FOV zoom trigger)
float 	distanceToRejectObject2 = 40.0f;		// the far distance... stop looking at this object
float gInitialIgnoreCollisionandDistanceFactorsTime = 2000.0f; // first n seconds dont worry about objects getting too close or far away.
bool bWeHaveAlreadyZoomedForThisTarget = false; // sort this out
bool bWasInvalidated = false; // does an object not want to be chosen again due to collision etc.	
int32 gbCineyCamProcessedOnFrame = 0;
float	gLastDurationSelected = gMaxDurationForAnIdleCam;
float randFactorForExpiryTime = 0.5f;
float nextTargetAcquireRot = HALF_PI*0.5f;
float pieAngleFromCameraToPlayer = HALF_PI * 0.7f;

//------------------------------------------------------------
// Is is or is it not time to allow the idle camera to kick in
bool CCam::IsItTimeForIdleCam()
{
	float time 		= CTimer::GetTimeInMilliseconds();
	float deltaTime = time - CPad::GetPad(0)->LastTimeTouched;
	
	if (deltaTime >timeToKickOffIdleCam)
	{			
		return true;
	}
	return false;
}

//-------------------------------------------------------------
// From a vector get the angles in world Rotation about Z and X
void CCam::VectorToAnglesRotXRotZ(CVector *pV,float *pA, float *pB)
{
	*pB = CGeneral::GetATanOfXY(pV->x,pV->y) + PI;
	float gd = CMaths::Sqrt(pV->x*pV->x + pV->y*pV->y);	
	*pA = CGeneral::GetATanOfXY(gd,pV->z);
}

//----------------------------------------------
// Remove references to entities we are tracking
void CCam::RemoveEntRefs()
{
	// Ref tidy
	if (pIdleTargetPed)		
	{		
		TIDYREF(pIdleTargetPed,(CEntity **)&pIdleTargetPed);
		pIdleTargetPed = NULL;
	}
	if (pIdleTargetVehicle)	
	{
		TIDYREF(pIdleTargetVehicle,(CEntity **)&pIdleTargetVehicle);
		pIdleTargetVehicle = NULL;
	}	
}

//-----------------------------------
// Is it time to select a new target?
bool CCam::IsItTimeForNewIdleTarget()
{
	float time 	= CTimer::GetTimeInMilliseconds();	
	float deltaTime = time - gTimeLastIdleTargetSelected;

	bWasInvalidated = false;

	// first time - we have no target so yes select one
	if (!pIdleTargetPed && !pIdleTargetVehicle)
	{
//		printf("first time so give me new idle target\n");	
		return true;
	}		

	// time expired we have been looking at our target for long enough
	if (deltaTime>gLastDurationSelected)
	{
//		printf("time expired so give me new idle target\n");
		return true;
	}
	
	CPlayerPed* pPlayerPed = FindPlayerPed();	
	
	if (pIdleTargetPed != pPlayerPed) // player is a valid target unless time expires so collision and distance are not a factor
	{		
		// Distance based?!
		CVector targetToLookAtPos;
		GetLookAtPos(&targetToLookAtPos);		
		CVector delta = Source-targetToLookAtPos;
		float dist = delta.Magnitude();
			
		if (deltaTime>gInitialIgnoreCollisionandDistanceFactorsTime) // initially after having a target selected we dont want to suddenly change our mind, we stick with this decision and persist looking at it for a minimal amount of time.
		{
			// Collision blocked ?
			CEntity *pOldIgnore = CWorld::pIgnoreEntity;
			if 		(pIdleTargetVehicle)		CWorld::pIgnoreEntity = pIdleTargetVehicle;	
			else if (pIdleTargetPed)			CWorld::pIgnoreEntity = pIdleTargetPed;				
			bool bLOSClear = CWorld::GetIsLineOfSightClear(Source,targetToLookAtPos, true, true, false, true, false, false, true);
			CWorld::pIgnoreEntity = pOldIgnore;			
			if (!bLOSClear)
			{
//				printf("object cant be seen\n");		
				bWasInvalidated = true;
				return true;		
			}

			// too close
			if (dist<distanceToRejectObject0*0.25f) // we allow the distance to be much closer because we know we have transitioned to the traget so we dont have angle direction of rotation problems... ideally I should have used quaternions to prevent this!
			{
//				printf("object became too close\n");
				bWasInvalidated = true;			
				return true;
			}

			if (pIdleTargetVehicle)
			{
				dist *=0.5f; // effectively make vehicles interesting up to twice the distance set for peds.
			}
			
			// too far away
			if (dist>distanceToRejectObject2)
			{
//				printf("object became too far away\n");		
				bWasInvalidated = true;
				return true;
			}		
			
			
			// dont look behind player! - adams concerns over streaming.
			if (pIdleTargetPed != pPlayerPed)
			{
				CVector targetToLookAtPos;
				GetLookAtPos(&targetToLookAtPos);
				
				CVector rvFrom 	= pPlayerPed->GetPosition() - Source;
				CVector rvTo 	= targetToLookAtPos-Source;
				float SlerpToRotX,SlerpToRotZ;	
					
				VectorToAnglesRotXRotZ(&rvFrom,&SlerpFromRotX,&SlerpFromRotZ);
				VectorToAnglesRotXRotZ(&rvTo,&SlerpToRotX,&SlerpToRotZ);

				if(SlerpToRotX - SlerpFromRotX > PI)		SlerpToRotX -= TWO_PI;
				else if(SlerpToRotX -SlerpFromRotX < -PI)	SlerpToRotX += TWO_PI;

				if(SlerpToRotZ - SlerpFromRotZ > PI)		SlerpToRotZ -= TWO_PI;
				else if(SlerpToRotZ -SlerpFromRotZ < -PI)	SlerpToRotZ += TWO_PI;
					
				float deltaRotZ = CMaths::Abs(SlerpFromRotZ-SlerpToRotZ);
				
				while (deltaRotZ > PI) deltaRotZ-=PI;						
				
				if (deltaRotZ > pieAngleFromCameraToPlayer)
					return true;			
			}														
		}
	}	
	else
	{
		if (deltaTime>gInitialIgnoreCollisionandDistanceFactorsTime)
			return true;
	}			
		
	return false;
}

//---------------------------------------------------------------
// now that we have decided we want a new target lets choose one.
bool CCam::SelectNewIdleTarget()
{
	bool bLookAtPed = CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > RandDegreeOfVehicleSelection;
	float time 	= CTimer::GetTimeInMilliseconds();
	
	gTimeLastIdleTargetSelected = time;

	CPed *pLastPed = pIdleTargetPed;
	CVehicle *pLastVehicle = pIdleTargetVehicle;
	
	CVector oldLookAtPos;
	CPlayerPed* pPlayerPed = FindPlayerPed();
	if (pLastPed || pLastVehicle)
		GetLookAtPos(&oldLookAtPos);
	else
	{
		oldLookAtPos = pPlayerPed->GetPosition();
		oldLookAtPos.z += 0.5f;
	}
	
	RemoveEntRefs();	
	
	if (bLookAtPed)
	{
		pIdleTargetPed = pPlayerPed->GetPedIntelligence()->GetClosestPedInRange();												
		if (!pIdleTargetPed) // no peds ... get a car
		{
			pIdleTargetVehicle = pPlayerPed->GetPedIntelligence()->GetClosestVehicleInRange();			
			if (!pIdleTargetVehicle)
			{				
				pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
				REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);				
				
				if (pLastPed == pIdleTargetPed)
					return false;
				return true;
			}
			else
			{
				REGREF(pIdleTargetVehicle, (CEntity **)&pIdleTargetVehicle);
			}
		}
		else
		{
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);
		}
	}
	else // look at a vehicle
	{
		pIdleTargetVehicle = pPlayerPed->GetPedIntelligence()->GetClosestVehicleInRange();		
		if (!pIdleTargetVehicle)
		{
			pIdleTargetPed = pPlayerPed->GetPedIntelligence()->GetClosestPedInRange();				
			if (!pIdleTargetPed)
			{
				pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
				REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);						
				if (pLastPed == pIdleTargetPed)
					return false;
				return true;
			}
			else
			{
				REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);			
			}
		}
		else
		{
			REGREF(pIdleTargetVehicle, (CEntity **)&pIdleTargetVehicle);
		}
	}	

	CVector targetToLookAtPos;
	
	// WHAT ARE WE LOOKING AT?
	if (pIdleTargetPed)
	{			
		targetToLookAtPos = pIdleTargetPed->GetPosition();
		targetToLookAtPos.z += 0.5f;
	}	
	else if (pIdleTargetVehicle)
	{
		targetToLookAtPos = pIdleTargetVehicle->GetPosition();
	}	

	CEntity *pOldIgnore = CWorld::pIgnoreEntity;
	if 		(pIdleTargetVehicle)	CWorld::pIgnoreEntity = pIdleTargetVehicle;	
	else if (pIdleTargetPed)		CWorld::pIgnoreEntity = pIdleTargetPed;	
	
	if (!CWorld::GetIsLineOfSightClear(Source,targetToLookAtPos, true, true, false, true, false, false, true))
	{
		RemoveEntRefs();
		pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
		REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);				
	}
	CWorld::pIgnoreEntity = pOldIgnore;

	if (pIdleTargetPed != pPlayerPed)
	{
		GetLookAtPos(&targetToLookAtPos);		
		CVector delta = Source-targetToLookAtPos;
		float dist = delta.Magnitude();
		

		// too close
		if (dist<distanceToRejectObject0)		
		{
//			printf("object IS too close so we choose the player\n");	
			RemoveEntRefs();
			pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);						
		}	

		// too far away
		if (pIdleTargetVehicle)
		{
			dist *=0.5f; // effectively make vehicles interesting up to twice the distance set for peds.
		}
		
		if (dist>distanceToRejectObject2)
		{
//			printf("object IS too far away so we choose the player\n");		
			RemoveEntRefs();
			pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);				
		}	
		
		if (pIdleTargetPed != pPlayerPed) // only even swing 90 degrees to next target.
		{
			CVector rvFrom 	= oldLookAtPos-Source;
			CVector rvTo 	= targetToLookAtPos-Source;
			float SlerpToRotX,SlerpToRotZ;	
				
			VectorToAnglesRotXRotZ(&rvFrom,&SlerpFromRotX,&SlerpFromRotZ);
			VectorToAnglesRotXRotZ(&rvTo,&SlerpToRotX,&SlerpToRotZ);


			if(SlerpToRotX - SlerpFromRotX > PI)		SlerpToRotX -= TWO_PI;
			else if(SlerpToRotX -SlerpFromRotX < -PI)	SlerpToRotX += TWO_PI;

			if(SlerpToRotZ - SlerpFromRotZ > PI)		SlerpToRotZ -= TWO_PI;
			else if(SlerpToRotZ -SlerpFromRotZ < -PI)	SlerpToRotZ += TWO_PI;
							
			float deltaRotZ = CMaths::Abs(SlerpFromRotZ-SlerpToRotZ);
			
			while (deltaRotZ > PI) deltaRotZ-=PI;
			
			if (deltaRotZ > nextTargetAcquireRot)
			{
				RemoveEntRefs();
				pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
				REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);				
			}
		}									
	}

	// dont look behind player! - adams concerns over streaming.
	if (pIdleTargetPed != pPlayerPed)
	{
		CVector targetToLookAtPos;
		GetLookAtPos(&targetToLookAtPos);
		
		CVector rvFrom 	= pPlayerPed->GetPosition() - Source;
		CVector rvTo 	= targetToLookAtPos-Source;
		float SlerpToRotX,SlerpToRotZ;	
			
		VectorToAnglesRotXRotZ(&rvFrom,&SlerpFromRotX,&SlerpFromRotZ);
		VectorToAnglesRotXRotZ(&rvTo,&SlerpToRotX,&SlerpToRotZ);

		if(SlerpToRotX - SlerpFromRotX > PI)		SlerpToRotX -= TWO_PI;
		else if(SlerpToRotX -SlerpFromRotX < -PI)	SlerpToRotX += TWO_PI;

		if(SlerpToRotZ - SlerpFromRotZ > PI)		SlerpToRotZ -= TWO_PI;
		else if(SlerpToRotZ -SlerpFromRotZ < -PI)	SlerpToRotZ += TWO_PI;
		
		float deltaRotZ = CMaths::Abs(SlerpFromRotZ-SlerpToRotZ);
		
		while (deltaRotZ > PI) deltaRotZ-=PI;
		
		if (deltaRotZ > pieAngleFromCameraToPlayer)
		{
			RemoveEntRefs();
			pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);				
		}
	}									
				
	if (pIdleTargetPed && pLastPed == pIdleTargetPed)
	{
//		printf("we choose the same ped again\n");
		if (bWasInvalidated)
		{
			RemoveEntRefs();
			pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);			
			return true;
		}
		return false;
	}
	if (pIdleTargetVehicle && pLastVehicle == pIdleTargetVehicle)
	{
//		printf("we choose the same vehicle again\n");
		if (bWasInvalidated)
		{
			RemoveEntRefs();
			pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);			
			return true;
		}		
		return false;
	}
					
	return true;	
}

void CCam::StartSlerp(CPed *pLastPed,CVehicle *pLastVehicle)
{
	float time 	= CTimer::GetTimeInMilliseconds();
	gTimeSlerpStarted = time;

//	printf("slerp started at %f\n",gTimeSlerpStarted);
	
	if (pLastVehicle)			
		SlerpFromPos = pLastVehicle->GetPosition();
	else if (pLastPed)
	{							
		SlerpFromPos = pLastPed->GetPosition();
		SlerpFromPos.z += 0.5f;			
	}
	else
		SlerpFromPos = Source + Front;
}

void CCam::ProcessIdleFOVZooms()
{
	// Dont kick off a zoom if we are about to change camera maybe...
	float time 	= CTimer::GetTimeInMilliseconds();	
	float deltaTime = time - gTimeLastIdleTargetSelected;	
	
	bool bCanKickOffZoom = true;
	
	if (deltaTime/gMaxDurationForAnIdleCam>0.75f)
		bCanKickOffZoom = false;

	CVector lookAt;
	GetLookAtPos(&lookAt);

	if (bWeHaveAlreadyZoomedForThisTarget)
		bCanKickOffZoom = false;
	
	float timeDeltaSlerp = time-gTimeSlerpStarted;
	float t = timeDeltaSlerp/timeToSlerpFromOneToAnother;	
	CVector delta = Source-lookAt;
	float dist = delta.Magnitude();
			
	if (t<=1.0f)
	{
		if (dist<distanceToRejectObject1)
		{			
//			printf("zooming out\n");
			FOV = DW_SINE_ACCEL_DECEL_LERP(t,doubleCachedFOV,FOVZoomOut); // zoom out when slerping always.
		}
		else
		{
			FOV = doubleCachedFOV;
		}
	}	
	else if (bCanKickOffZoom)	// otherwise zoom in if permitrted and target at trigger range
	{		
		if (dist>distanceToRejectObject1)	
		{
//			printf("kick off zoom in\n");
			bWeHaveAlreadyZoomedForThisTarget = true;
			cachedFOV			= doubleCachedFOV;
			timeFOVZoomStarted 	= time;	
			FOV = cachedFOV;	
		}		
	}
	else if (bWeHaveAlreadyZoomedForThisTarget)
	{					
//		printf("zooming in or zoomed in\n");	
		float dt = time-timeFOVZoomStarted;
		float t2  = dt/timeFOVZoomDuration;		
		if (t2>1.0f) t2 = 1.0f;			
		FOV = DW_SINE_ACCEL_DECEL_LERP(t2,cachedFOV,FOVZoomIn);							
	}	
	
	doubleCachedFOV = FOV; // cached FOV needs to be preserved as when set, so this is a stroe of the last FOV as it was set here.
}


void CCam::GetLookAtPos(CVector *pVec)
{
	// WHAT ARE WE LOOKING AT?
	if (pIdleTargetPed)
	{		
		*pVec = pIdleTargetPed->GetPosition();
		pVec->z += 0.5f;
	}	
	else if (pIdleTargetVehicle)
	{
		*pVec = pIdleTargetVehicle->GetPosition();
	}		
	else
	{
		ASSERT(false);
	}
}	

//-----------------------------------------------------------------------------------------------
// After a period of user inactivity process an idle camera that nosies about looking at things.
// Not implemented as a camera mode because of all the confusion about mode changing shite... I basically
// cannot fully understand what is going on so I'm implementing this in a safe way that I know will work.
bool CCam::PossiblyProcessIdleCam()
{	
	float time 		= CTimer::GetTimeInMilliseconds();	
	CPlayerPed* pPlayerPed = FindPlayerPed();
		
	if (!IsItTimeForIdleCam())
	{
		RemoveEntRefs();
		framesInThisMode = 0; 	
		cachedFOV = 70.0f;						
		doubleCachedFOV = 70.0f;			
		return false;
	}
	
	// First time stuff
	if (framesInThisMode<=0)
	{
		cachedFOV = 70.0f;						
		doubleCachedFOV = 70.0f;			
		gTimeSlerpStarted = 0.0f;
		gTimeLastIdleTargetSelected = 0.0f;
	}
		
	float shakeDegree = framesInThisMode/framesToInterpolateUpTheShake;
	if (shakeDegree>1.0f) shakeDegree = 1.0f;
	framesInThisMode++;


	// dont move the source we make it like the virtual cameraman is bored... ( I tell you what he is not as bored as I am writing this shite )		
	CVector sourcePos = Source;
	CVector targetToLookAtPos;

	// If we have an old reference to another object we slerp from its position	
	CPed *pLastPed 			= pIdleTargetPed;
	CVehicle *pLastVehicle 	= pIdleTargetVehicle;
		
	if (IsItTimeForNewIdleTarget())
	{
		gLastDurationSelected = gMaxDurationForAnIdleCam *  CGeneral::GetRandomNumberInRange(randFactorForExpiryTime, 1.0f);
		
		if (SelectNewIdleTarget())
		{
			bWeHaveAlreadyZoomedForThisTarget = false; // one time only reset this so we are allowed to zoom

			StartSlerp(pLastPed,pLastVehicle);									
		}				
	}

	GetLookAtPos(&targetToLookAtPos);

	ProcessIdleFOVZooms();				

	// work out angles for slerp.
	CVector rvFrom 	= SlerpFromPos-Source;
	CVector rvTo 	= targetToLookAtPos-Source;
	float SlerpToRotX,SlerpToRotZ;	
		
	VectorToAnglesRotXRotZ(&rvFrom,&SlerpFromRotX,&SlerpFromRotZ);
	VectorToAnglesRotXRotZ(&rvTo,&SlerpToRotX,&SlerpToRotZ);

	if(SlerpToRotX - SlerpFromRotX > PI)		SlerpToRotX -= TWO_PI;
	else if(SlerpToRotX -SlerpFromRotX < -PI)	SlerpToRotX += TWO_PI;

	if(SlerpToRotZ - SlerpFromRotZ > PI)		SlerpToRotZ -= TWO_PI;
	else if(SlerpToRotZ -SlerpFromRotZ < -PI)	SlerpToRotZ += TWO_PI;
	
	float timeDeltaSlerp = time-gTimeSlerpStarted;
	float t = timeDeltaSlerp/timeToSlerpFromOneToAnother;	

	if (t>1.0f)
		t = 1.0f;
	float curAngleX = DW_SINE_ACCEL_DECEL_LERP(t,SlerpFromRotX,SlerpToRotX);
	float curAngleZ = DW_SINE_ACCEL_DECEL_LERP(t,SlerpFromRotZ,SlerpToRotZ);	

//	printf("curAngleX %f SlerpFromRotX %f SlerpToRotX %f\n",curAngleX,SlerpFromRotX,SlerpToRotX);
//	printf("curAngleZ %f SlerpFromRotZ %f SlerpToRotZ %f\n",curAngleZ,SlerpFromRotZ,SlerpToRotZ);

#ifndef MASTER
/*	char str[255];
	sprintf(str,"X ANG %f FROM %f TO %f",curAngleX,SlerpFromRotX,SlerpToRotX);
	VarConsole.AddDebugOutput(str);
	sprintf(str,"Z ANG %f FROM %f TO %f",curAngleZ,SlerpFromRotZ,SlerpToRotZ);
	VarConsole.AddDebugOutput(str);
*/	
#endif

	Front = CVector(-CMaths::Cos(curAngleZ) *CMaths::Cos(curAngleX) , -CMaths::Sin(curAngleZ)*CMaths::Cos(curAngleX), CMaths::Sin(curAngleX));
		
	// If target obscured look at player, as we can guarantee the player is not obscured, since we havent moved the camera						
	Front.Normalise();
	
	
	static float degreeShakeIdleCam = 1.0f;
	CHandShaker *pS = &gHandShaker[0];		
	pS->Process(degreeShakeIdleCam*shakeDegree);		
	float roll = pS->ang.z * degreeShakeIdleCam*shakeDegree; // might be z ???
	CVector temp = Multiply3x3(Front,pS->resultMat);
	Front = temp;	
	
	Up=CVector(CMaths::Sin(roll),0.0f,CMaths::Cos(roll));
	CVector TempRight= CrossProduct(Front, Up);
	TempRight.Normalise();
	Up=CrossProduct(TempRight, Front);

	if ((Front.x==0.0f) && (Front.y==0.0f))
	{
		Front.x=0.0001f;
		Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( Front, Up );
	TempRight.Normalise();
	Up = CrossProduct( TempRight, Front );			
	
	GetVectorsReadyForRW();	
	
	return true;
}
#endif

//----------------------------------------------------------------------
// here you specify the expectptions to the rule for camera behaviour...
void CCamera::InitCameraVehicleTweaks()
{
	m_VehicleTweakLenMod	 	= 1.0f; // mulitpliers of calculated vechile length and height for camera positioning.
	m_VehicleTweakTargetZMod	= 1.0f;
	m_VehicleTweakPitchMod= 0.0f;
	m_VehicleTweakLastModelId   = -1;	

	if (!m_bInitedVehicleCamTweaks) // lazy force initialise this data
	{
		for (int32 i=0;i<MAX_VEHICLE_CAM_TWEAKS;i++)
		{
			m_VehicleTweaks[i].Set(-1,1.0f,1.0f,0.0f);
		}
	
		m_VehicleTweaks[0].Set(MODELID_CAR_RCGOBLIN,1.000000,1.000000,0.178997); // Les decided on this value 
		
		
		// m_VehicleTweaks[MAX_VEHICLE_CAM_TWEAKS-1] << do not set
		// Dont set the last used vehicle tweak as it is a 
		
		m_bInitedVehicleCamTweaks = true;
	}		
}

//---------------------------------------------------------------------------------
// DW - a useful function to allow per vehicle camera modifictions on its behaviour
void CCamera::ApplyVehicleCameraTweaks(CVehicle *pVehicle)
{
#ifdef MASTER
	if (pVehicle->GetModelIndex() != m_VehicleTweakLastModelId) // optimisation for when all editing is complete, no need to recompute these values once setup.
#else
	static bool gbAllowTweaking = false; // DW - I set this to true to allow keybnoard modifications.
	char str[255];		
	
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_T) && PS2Keyboard.GetKeyDown(PS2_KEY_CTRL))
		gbAllowTweaking = !gbAllowTweaking;


#ifndef FINAL
	if (gbAllowTweaking)
		VarConsole.AddDebugOutput("(CTRL)KEYS: (T)WEAK (S)AVE (R)ESET (V) = Add");
#endif

	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_S) && PS2Keyboard.GetKeyDown(PS2_KEY_CTRL))
	{
		int32 fd = CFileMgr::OpenFileForWriting("X:\\CAMTWEAK.TXT");
		if (fd>0)
		{
			sprintf(str,"Paste this stuff into CCamera::InitCameraVehicleTweaks() to permanently set these fields for these vehicles- DW");
			CFileMgr::Write(fd,str,strlen(str)+1);
			for (int32 i=0;i<MAX_VEHICLE_CAM_TWEAKS;i++)
			{
				sprintf(str,"m_VehicleTweaks[i].Set(MODELID_CAR_<%d>,%f,%f,%f);\n",m_VehicleTweaks[i].m_ModelId,m_VehicleTweaks[i].m_LenMod,m_VehicleTweaks[i].m_TargetZMod,m_VehicleTweaks[i].m_PitchMod);
				CFileMgr::Write(fd,str,strlen(str)+1);
			}
			CFileMgr::CloseFile(fd);
		}		
	}		
#endif	
	{				
		InitCameraVehicleTweaks();
		
		
#ifndef FINAL
		// Want to set this modifer in code... here you go...
		if (PS2Keyboard.GetKeyDown(PS2_KEY_CTRL) && PS2Keyboard.GetKeyJustDown(PS2_KEY_V))
		{
			CVehicleCamTweak *pTweak = m_VehicleTweaks;
			for (int32 i=0;i<MAX_VEHICLE_CAM_TWEAKS;i++)
			{
				if (pTweak->m_ModelId<0)		
				{
					pTweak->m_ModelId 		= pVehicle->GetModelIndex();
					pTweak->m_LenMod  		= 1.0f;				
					pTweak->m_TargetZMod  	= 1.0f;
					pTweak->m_PitchMod 		= 0.0f;
					break;
				}
				pTweak++;
			}
		}
#endif
						
		CVehicleCamTweak *pTweak = m_VehicleTweaks;
		for (int32 i=0;i<MAX_VEHICLE_CAM_TWEAKS;i++)
		{
			if (pTweak->m_ModelId<0)
			{
				pTweak++;
			 	continue; // end of used array.				 // used to break out but script uses highest slot of tweak array.
			}	
			
			if (pTweak->m_ModelId==pVehicle->GetModelIndex()) // found it ... lets grab the data
			{
				m_VehicleTweakLenMod 			= pTweak->m_LenMod;     // or break point here if already defined
				m_VehicleTweakLastModelId    	= pTweak->m_ModelId;
				m_VehicleTweakTargetZMod		= pTweak->m_TargetZMod;	
				m_VehicleTweakPitchMod			= pTweak->m_PitchMod;
#ifndef MASTER		
				if (gbAllowTweaking)
				{
					// Want to set this modifer in code... here you go...
					if (PS2Keyboard.GetKeyDown(PS2_KEY_CTRL) && PS2Keyboard.GetKeyJustDown(PS2_KEY_R))
					{
						pTweak->m_ModelId 		= pVehicle->GetModelIndex();
						pTweak->m_LenMod  		= 1.0f;				
						pTweak->m_TargetZMod  	= 1.0f;
						pTweak->m_PitchMod 		= 0.0f;
					}				
				
					static float 	gModScale 			= 0.001f;
					static int 		gFieldMode 			= 0;
					static float    gPitchModifierDelta = 0.001f;
					static float 	gModMul 			= 10.0f;
					#define MAX_FIELDS 3
					
					float scale = 0.0f;	
					if (PS2Keyboard.GetKeyDown(PS2_KEY_UP)) 		scale = 1.0f;
					if (PS2Keyboard.GetKeyDown(PS2_KEY_DOWN))		scale = -1.0f;
					if (PS2Keyboard.GetKeyJustDown(PS2_KEY_RIGHT))	if (++gFieldMode >= MAX_FIELDS) gFieldMode=0;
					if (PS2Keyboard.GetKeyJustDown(PS2_KEY_LEFT))	if (--gFieldMode < 0) 			gFieldMode=MAX_FIELDS-1;
					if (PS2Keyboard.GetKeyDown(PS2_KEY_CTRL)) 		scale *= gModMul;
													
					switch (gFieldMode)
					{
						case 0: 
							pTweak->m_LenMod += gModScale * scale; 							
							sprintf(str, "Cam-Vehicle tweak : Len = %f",pTweak->m_LenMod);
							break;
						case 1:
							pTweak->m_TargetZMod += gModScale * scale; 
							sprintf(str, "Cam-Vehicle tweak : TargetZMod = %f",pTweak->m_TargetZMod);																									
							break;											
						case 2:
							pTweak->m_PitchMod += gPitchModifierDelta * scale; 
							sprintf(str, "Cam-Vehicle tweak : PitchMod = %f",pTweak->m_PitchMod);																									
							break;							
					}															
					VarConsole.AddDebugOutput(str);
				}
#endif				
				break;
			}			
			pTweak++;
		}				
	}
}





enum{
	FOLLOW_CAR_INCAR = 0,
	FOLLOW_CAR_ONBIKE,
	FOLLOW_CAR_INHELI,
	FOLLOW_CAR_INPLANE,
	FOLLOW_CAR_INBOAT,
	FOLLOW_CAR_RCCAR,
	FOLLOW_CAR_RCHELI
};

CamFollowPedData CARCAM_SET[FOLLOW_CAR_RCHELI+1] =
{
//	targZ	baseDst baseZ	minDist	minFol	aRate	aCap 	aSwing	bRate	bCap	bSwing	bSwingCap	stickM	upLimit			downLimit
	{1.3f,	1.0f,	0.40f,	10.0f,	15.0f,	0.5f, 	1.0f, 	1.0f,	0.85f,	0.2f,	0.075,	0.05f,		0.80f,	DEGTORAD(45.0f), DEGTORAD(89.0f)},	// car
	{1.1f,	1.0f,	0.10f,	10.0f,	11.0f,	0.5f, 	1.0f, 	1.0f,	0.85f,	0.2f,	0.075f,	0.05f,		0.75f,	DEGTORAD(45.0f), DEGTORAD(89.0f)},	// bike
	{1.1f,	1.0f,	0.20f,	10.0f,	15.0f,	0.05f, 	0.05f, 	0.0f,	0.9f,	0.05f,	0.01f,	0.05f,		1.0f,	DEGTORAD(10.0f), DEGTORAD(70.0f)},	// heli
	{1.1f,	3.5f,	0.20f,	10.0f,	25.0f,	0.5f, 	1.0f, 	1.0f,	0.75f,	0.1f,	0.005f,	0.20f,		1.0f,	DEGTORAD(89.0f), DEGTORAD(89.0f)},	// plane
	{1.3f,	1.0f,	0.40f,	10.0f,	15.0f,	0.5f, 	1.0f, 	0.0f,	0.9f,	0.05f,	0.005f,	0.05f,		1.0f,	DEGTORAD(20.0f), DEGTORAD(70.0f)},	// boat
	{1.1f,	1.0f,	0.20f,	10.0f,	5.0f,	0.5f, 	1.0f, 	1.0f,	0.75f,	0.1f,	0.005f,	0.20f,		1.0f,	DEGTORAD(45.0f), DEGTORAD(89.0f)},	// rc_car
	{1.1f,	1.0f,	0.20f,	10.0f,	5.0f,	0.5f, 	1.0f, 	1.0f,	0.75f,	0.1f,	0.005f,	0.20f,		1.0f,	DEGTORAD(20.0f), DEGTORAD(70.0f)}	// rc_heli
};
//
#pragma mark ----follow-car----------------
//
void CCam::Process_FollowCar_SA(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired, bool bScriptSetAngles)
{
	if(!CamTargetEntity->GetIsTypeVehicle())
		return;

	CVehicle *pVehicle = (CVehicle *)CamTargetEntity;
	CVector vecTargetCoords = ThisCamsTarget;
	CPad *pPad = CPad::GetPad(0);
	if(pVehicle->pDriver && pVehicle->pDriver->GetPedType()==PEDTYPE_PLAYER2)
		pPad = CPad::GetPad(1);

	int32 nType = FOLLOW_CAR_INCAR;
	
	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ'Code Tweaker'ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
	// The idea here is to allow a final tweaking of values, for a small subset of vehicles    ZZZ
	// which is 100% memory efficient, no slower to load, and easier modified by only me. - DW ZZZ
	TheCamera.ApplyVehicleCameraTweaks(pVehicle);
	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ	
	
	if(pVehicle->GetModelIndex()==MODELID_CAR_RCBANDIT || pVehicle->GetModelIndex()==MODELID_CAR_RCBARON
	|| pVehicle->GetModelIndex()==MODELID_CAR_RCTIGER || pVehicle->GetModelIndex()==MODELID_CAR_RCCAM)
		nType = FOLLOW_CAR_RCCAR;
	else if(pVehicle->GetModelIndex()==MODELID_CAR_RCCOPTER || pVehicle->GetModelIndex()==MODELID_CAR_RCGOBLIN)
		nType = FOLLOW_CAR_RCHELI;
	else if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE
	|| pVehicle->GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
		nType = FOLLOW_CAR_ONBIKE;
	else if(pVehicle->GetVehicleType()==VEHICLE_TYPE_HELI)
		nType = FOLLOW_CAR_INHELI;
	else if(pVehicle->GetVehicleType()==VEHICLE_TYPE_PLANE)
	{
		if(pVehicle->GetModelIndex()==MODELID_PLANE_HARRIER
		&& ((CAutomobile *)pVehicle)->m_nSuspensionHydraulics >= CPlane::HARRIER_NOZZLE_SWITCH_LIMIT)
			nType = FOLLOW_CAR_INHELI;
		else if(pVehicle->GetModelIndex()==MODELID_CAR_VORTEX)
			nType = FOLLOW_CAR_INCAR;
		else
			nType = FOLLOW_CAR_INPLANE;
	}
	else if(pVehicle->GetVehicleType()==VEHICLE_TYPE_BOAT)
		nType = FOLLOW_CAR_INBOAT;

	float fCamDistance = TheCamera.m_fCarZoomSmoothed + CARCAM_SET[nType].fBaseCamDist;
	//float fCamAlpha = CARCAM_SET[nType].fBaseCamZ;
		
	int PositionInArray = 0;
	TheCamera.GetArrPosForVehicleType(pVehicle->GetVehicleAppearance(), PositionInArray);

	float fCamAlpha = 0.0f;
	if(pVehicle->GetStatus()==STATUS_PLAYER_REMOTE)  // this is a hack but there's no other way right now
		fCamAlpha += ZmTwoAlphaOffset[PositionInArray];
	else if(TheCamera.m_nCarZoom==ZOOM_ONE)
		fCamAlpha += ZmOneAlphaOffset[PositionInArray];
	else if(TheCamera.m_nCarZoom==ZOOM_TWO)
		fCamAlpha += ZmTwoAlphaOffset[PositionInArray];
	else if(TheCamera.m_nCarZoom==ZOOM_THREE)
		fCamAlpha += ZmThreeAlphaOffset[PositionInArray];
	
	float CarHeight = pVehicle->GetColModel().GetBoundBoxMax().z;
	float CarLength = 2.0f*CMaths::Abs(pVehicle->GetColModel().GetBoundBoxMin().y);

	// need to add on length for trailer if we're towing
	static float sBlendExtraPos = 0.0f;
	static float BLEND_EXTRA_POS_RATE = 0.02f;
	if(pVehicle->m_pVehicleBeingTowed)
	{
		if(sBlendExtraPos < 1.0f)
			sBlendExtraPos = CMaths::Min(1.0f, sBlendExtraPos + BLEND_EXTRA_POS_RATE*CTimer::GetTimeStep());
	
		static float TRAILER_CAMDIST_MULT = 0.5f;
		CarLength += sBlendExtraPos*TRAILER_CAMDIST_MULT*(pVehicle->m_pVehicleBeingTowed->GetColModel().GetBoundBoxMax() - pVehicle->m_pVehicleBeingTowed->GetColModel().GetBoundBoxMin()).Magnitude();
		CarHeight += sBlendExtraPos*(CMaths::Max(CarHeight, pVehicle->m_pVehicleBeingTowed->GetColModel().GetBoundBoxMax().z) - CarHeight);

		vecTargetCoords = (1.0f - 0.5f*sBlendExtraPos)*vecTargetCoords + 0.5f*sBlendExtraPos*pVehicle->m_pVehicleBeingTowed->GetPosition();
	}
	else if(pVehicle->GetVehicleType()==VEHICLE_TYPE_BIKE || pVehicle->GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
	{
		if(pVehicle->pPassengers[0]!=NULL)
		{
			if(sBlendExtraPos < 1.0f)
				sBlendExtraPos = CMaths::Min(1.0f, sBlendExtraPos + BLEND_EXTRA_POS_RATE*CTimer::GetTimeStep());
		}
		else if(sBlendExtraPos > 0.0f)
			sBlendExtraPos = CMaths::Max(0.0f, sBlendExtraPos - BLEND_EXTRA_POS_RATE*CTimer::GetTimeStep());
		
		static float BIKE_WITH_PASSENGER_HEIGHT_ADD = 0.4f;
		CarHeight += sBlendExtraPos*BIKE_WITH_PASSENGER_HEIGHT_ADD;
	}
	else
		sBlendExtraPos = 0.0f;

	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
	//ZZZZZZZ per vehicle camera tweaking - see top of func. ZZZZZZ
	CarLength *=  TheCamera.m_VehicleTweakLenMod;
	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ

	fCamDistance += CarLength;
	float fMinDistance = CARCAM_SET[nType].fMinDist*CarLength;
	
/* - DW - since the cycling of the car modes doesnt push out the camera - due to ciney cam, this isnt required right now.
	//MMMMMMMMMMMMMMMMMMMMMMMM'Slow camera zoom out'MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
	// make mode changes  (via select key) apparent which mode you are in - DW
	if(fCamDistance > gLastCamDist)
		fMinDistance = fCamDistance;		
	gLastCamDist = fCamDistance;
	//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
*/
	// use a different offset for full-size heli's (not r/c ones)
	if(pVehicle->GetVehicleAppearance()==APR_HELI && pVehicle->GetStatus()!=STATUS_PLAYER_REMOTE)
		vecTargetCoords += pVehicle->GetMatrix().GetUp()*fTestShiftHeliCamTarget*CarHeight;
	else
	{
		float fTargetZMod = CarHeight*CARCAM_SET[nType].fTargetOffsetZ -  CARCAM_SET[nType].fBaseCamZ;
		if(fTargetZMod > 0.0f)
		{
			vecTargetCoords.z += fTargetZMod;
			fCamDistance += fTargetZMod;
			
			static float TEST_CAM_ALPHA_RAISE_MULT = 0.3f;
			fCamAlpha += TEST_CAM_ALPHA_RAISE_MULT*fTargetZMod/fCamDistance;
		}
	}

	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
	//ZZZZZZZ per vehicle camera tweaking - see top of func. ZZZ
	vecTargetCoords.z *= TheCamera.m_VehicleTweakTargetZMod;
	fCamAlpha += TheCamera.m_VehicleTweakPitchMod;
	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ	

	float fTempMinFollowDist = CARCAM_SET[nType].fMinFollowDist;
	if(TheCamera.m_nCarZoom==ZOOM_ONE && (nType==FOLLOW_CAR_INCAR || nType==FOLLOW_CAR_ONBIKE))
			fTempMinFollowDist *= 0.65f;
	float fFollowDist = MAX(fCamDistance, fTempMinFollowDist);

	// damn globals passed between functions!
	CA_MAX_DISTANCE=fCamDistance;
	CA_MIN_DISTANCE=3.5f;
	
	if(ResetStatics)
		FOV=70.f;
	else
	{
		if((pVehicle->GetVehicleType()==VEHICLE_TYPE_CAR || pVehicle->GetVehicleType()==VEHICLE_TYPE_BIKE)
		&& DotProduct(pVehicle->GetMoveSpeed(), pVehicle->GetMatrix().GetForward()) > CAR_FOV_START_SPEED)
			FOV += (DotProduct(pVehicle->GetMoveSpeed(), pVehicle->GetMatrix().GetForward()) - CAR_FOV_START_SPEED)*CTimer::GetTimeStep();
		
		if(FOV > 70.0f)
			FOV = 70.0f + (FOV - 70.0f)*CMaths::Pow(CAR_FOV_FADE_MULT, CTimer::GetTimeStep());
			
		if(FOV > 100.0f)
			FOV = 100.0f;
		else if(FOV < 70.0f)
			FOV = 70.0f;
	}

	// because camera has been re-initialised
	if(ResetStatics || TheCamera.m_bCamDirectlyBehind || TheCamera.m_bCamDirectlyInFront)
	{
//		ResetFirstPersonUpCastFixer();
	
		ResetStatics = false;
		Rotating=FALSE;
		m_bCollisionChecksOn=true;
		TheCamera.m_bResetOldMatrix = true;
		
		if(!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
		{
			Alpha = 0.0f;
			Beta = pVehicle->GetHeading() - HALF_PI;
			if(TheCamera.m_bCamDirectlyInFront)
				Beta += PI;
		}
		BetaSpeed = 0.0f;
		AlphaSpeed = 0.0f;
		Distance = 1000.0f;
		
		Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));

		// we want to store the last camera position always without including its alpha offset
		m_aTargetHistoryPos[0] = vecTargetCoords - fFollowDist*Front;
		m_nTargetHistoryTime[0] = CTimer::GetTimeInMilliseconds();
		m_aTargetHistoryPos[1] = vecTargetCoords - fCamDistance*Front;
		m_nCurrentHistoryPoints = 0;

		if(!TheCamera.m_bJustCameOutOfGarage && !bScriptSetAngles) // dont want to change alpha and beta!
		{
			Alpha = -fCamAlpha;
		}			
	}

	Front = vecTargetCoords - m_aTargetHistoryPos[0];
	Front.Normalise();
	float fTempLength = (vecTargetCoords - m_aTargetHistoryPos[1]).Magnitude();	

	float fTargetDiff, fDiffMult, fDiffCap;
	float fTargetBeta = CMaths::ATan2(-Front.x, Front.y) - HALF_PI;
	if(fTargetBeta < -PI)	fTargetBeta += TWO_PI;
	
	// want to swing the camera around by a variable percentage to match the players heading
	float fHeadingBeta = 0.0f;
	if(pVehicle->GetMoveSpeed().Magnitude2D() > 0.02f)
		fHeadingBeta = CMaths::ATan2(-pVehicle->GetMoveSpeed().x, pVehicle->GetMoveSpeed().y) - HALF_PI;
	else
		fHeadingBeta = fTargetBeta;
//		fHeadingBeta = pVeh->GetHeading() - HALF_PI;
	
	if(fHeadingBeta > fTargetBeta + PI)
		fHeadingBeta -= TWO_PI;
	else if(fHeadingBeta < fTargetBeta - PI)
		fHeadingBeta += TWO_PI;

	// swing camera based on movement speed of vehicle
	{	
		fDiffMult = CARCAM_SET[nType].fDiffBetaSwing*CTimer::GetTimeStep();
		fDiffCap = CARCAM_SET[nType].fDiffBetaSwingCap*CTimer::GetTimeStep();

		float fAltSpeed = DotProduct(Front, pVehicle->GetMoveSpeed());
		fAltSpeed = (pVehicle->GetMoveSpeed() - fAltSpeed*Front).Magnitude();
		fDiffMult = MIN(1.0f, fAltSpeed*fDiffMult);
		
		fTargetDiff = fDiffMult*(fHeadingBeta - fTargetBeta);
		if(fTargetDiff > fDiffCap)
			fTargetDiff = fDiffCap;
		else if(fTargetDiff < -fDiffCap)
			fTargetDiff = -fDiffCap;
	}
	//////////////////
	
	fTargetBeta += fTargetDiff;
	if(fTargetBeta > Beta + PI)	fTargetBeta -= TWO_PI;
	else if(fTargetBeta < Beta - PI)	fTargetBeta += TWO_PI;
	
	float fCamControlBetaSpeed = (fTargetBeta - Beta)/CMaths::Max(1.0f, CTimer::GetTimeStep());
	
	float fTargetAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, Front.z)));

// WARNING! this code is supposed to help the camera swing up as the target goes over the brow of hills
// but it seems to be causing some jerks in the camera motion so I need to figure out a better way to do this
//	if(fTargetAlpha < Alpha && fTempLength > fCamDistance)
//	{
//		fTargetAlpha = CMaths::ASin((fCamDistance / fTempLength)*CMaths::Sin(fTargetAlpha));
//	}

	if(fTempLength < fCamDistance && fCamDistance > fMinDistance)
		fCamDistance = CMaths::Max(fMinDistance, fTempLength);
	
	float fAlphaUpLimit =  CARCAM_SET[nType].fUpLimit;
	// we want to stop the player pushing the camera down and into the vehicle they're driving
	if(pVehicle->GetMoveSpeed().MagnitudeSqr() < 0.2f*0.2f
	&& !(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE && ((CBike *)pVehicle)->nNoOfContactWheels<4)
	&& pVehicle->GetVehicleType()!=VEHICLE_TYPE_HELI
	&& !(pVehicle->GetVehicleType()==VEHICLE_TYPE_PLANE && ((CAutomobile *)pVehicle)->nNoOfContactWheels==0))
	{
		CVector vecTempRight = CrossProduct(pVehicle->GetMatrix().GetForward(), CVector(0.0f,0.0f,1.0f));
		vecTempRight.Normalise();
		CVector vecTempUp = CrossProduct(vecTempRight, pVehicle->GetMatrix().GetForward());
		vecTempUp.Normalise();
		if(DotProduct(Front, vecTempUp) > 0.0f)
		{
//			StickAlphaOffset *= 0.5f;
			CColModel &vehCol = pVehicle->GetColModel();
			float fVehicleHeight = pVehicle->GetHeightAboveRoad();
			fVehicleHeight += vecTargetCoords.z - pVehicle->GetPosition().z;
//			float fAngleLength = 0.75f*CMaths::ATan2(fVehicleHeight, -vehCol.GetBoundBoxMin().y);
//			float fAngleWidth = 0.75f*CMaths::ATan2(fVehicleHeight, vehCol.GetBoundBoxMax().x);
			float fCornerAngle = CMaths::ATan2(vehCol.GetBoundBoxMax().x, -vehCol.GetBoundBoxMin().y);

			// difference between camera and vehicle's heading			
			float fBetaDiff = CMaths::Abs(CMaths::Sin(Beta - (pVehicle->GetHeading() - HALF_PI)));
			fBetaDiff = CMaths::ASin(fBetaDiff);

			static float STICK_DOWN_LENGTH_ADD = 1.5f;
			static float STICK_DOWN_WIDTH_ADD = 1.2f;
			static float STICK_DOWN_DIST_LIMIT_MULT = 1.2f;
			float fVehicleDist;
			if(fBetaDiff > fCornerAngle)
				fVehicleDist = (vehCol.GetBoundBoxMax().x + STICK_DOWN_WIDTH_ADD) / CMaths::Cos(CMaths::Max(0.0f, HALF_PI - fBetaDiff));
			else
				fVehicleDist = (-vehCol.GetBoundBoxMin().y + STICK_DOWN_LENGTH_ADD) / CMaths::Cos(fBetaDiff);
			
			fVehicleDist *= STICK_DOWN_DIST_LIMIT_MULT;
			float fAngleLimit = CMaths::ATan2(fVehicleHeight, fVehicleDist);
			
			fAlphaUpLimit = fAngleLimit;

			// add pitch of car
			fAlphaUpLimit += CMaths::Cos(Beta - (pVehicle->GetHeading() - HALF_PI))*CMaths::ATan2(pVehicle->GetMatrix().GetForward().z, pVehicle->GetMatrix().GetForward().Magnitude2D());			
			
			// if wheels on ground add roll of car
			if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR && ((CAutomobile *)pVehicle)->nNoOfContactWheels > 1
			&& CMaths::Abs(DotProduct(pVehicle->GetTurnSpeed(), pVehicle->GetMatrix().GetForward())) < 0.05f)
			{
				fAlphaUpLimit += CMaths::Cos(Beta - (pVehicle->GetHeading() - HALF_PI) + HALF_PI)*CMaths::ATan2(pVehicle->GetMatrix().GetRight().z, pVehicle->GetMatrix().GetRight().Magnitude2D());
			}
		}
	}

	fTargetAlpha -= fCamAlpha;
	if(fTargetAlpha > fAlphaUpLimit)//CARCAM_SET[nType].fUpLimit)
		fTargetAlpha = fAlphaUpLimit;//CARCAM_SET[nType].fUpLimit;
	else if(fTargetAlpha < -CARCAM_SET[nType].fDownLimit)
		fTargetAlpha = -CARCAM_SET[nType].fDownLimit;
	
	fDiffMult = CMaths::Pow(CARCAM_SET[nType].fDiffAlphaRate, CTimer::GetTimeStep());
	fDiffCap = CARCAM_SET[nType].fDiffAlphaCap*CTimer::GetTimeStep();

	fTargetDiff = (1.0f - fDiffMult)*(fTargetAlpha - Alpha);
	if(fTargetDiff > fDiffCap)
		fTargetDiff = fDiffCap;
	else if(fTargetDiff < -fDiffCap)
		fTargetDiff = -fDiffCap;

////////////////

	float StickBetaOffset = -(pPad->AimWeaponLeftRight(NULL));
	float StickAlphaOffset = pPad->AimWeaponUpDown(NULL);

	if (FrontEndMenuManager.m_ControlMethod == TOUCH_CONTROLLER_SCREEN && CTouchInterface::GetSteeringMode() == CTouchInterface::SM_EXPERIMENTAL) {
		CVector2D vecTouch(0, 0);
		CTouchInterface::IsTouched ( CTouchInterface::WIDGET_LOOK, &vecTouch );

		StickBetaOffset = vecTouch.x * -25.0f;
		StickAlphaOffset = vecTouch.y * -15.0f;

		static float betaTotal = 0.0f;
		betaTotal += StickBetaOffset * 0.0005f;
		float delta = CMaths::Min(CTimer::GetTimeStep() * 10.0f, 1.0f);
		StickBetaOffset = betaTotal * delta;
		betaTotal *= (1.0f - delta);
		BetaSpeed = 0.0f;

		StickAlphaOffset = 0; 
	} else {
		if(TheCamera.m_bUseMouse3rdPerson)
			StickAlphaOffset = 0.0f;
			
		StickBetaOffset = AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(StickBetaOffset)*StickBetaOffset * (0.25f/3.5f * (FOV/80.0f));// * CTimer::GetTimeStep();
		StickAlphaOffset = AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(StickAlphaOffset)*StickAlphaOffset * (0.15f/3.5f * (FOV/80.0f));// * CTimer::GetTimeStep();
	}

	bool bFixAlphaAngle = true;	
	// this is a tempory measure for vehicles that use the right stick for something else	
	if(pVehicle->GetModelIndex()==MODELID_CAR_PACKER || pVehicle->GetModelIndex()==MODELID_CAR_DOZER
	|| pVehicle->GetModelIndex()==MODELID_CAR_DUMPER || pVehicle->GetModelIndex()==MODELID_CAR_CEMENT
	|| pVehicle->GetModelIndex()==MODELID_PLANE_ANDROM || pVehicle->GetModelIndex()==MODELID_PLANE_HARRIER
	|| pVehicle->GetModelIndex()==MODELID_CAR_TOWTRUCK || pVehicle->GetModelIndex()==MODELID_CAR_FORKLIFT
	|| pVehicle->GetModelIndex()==MODELID_CAR_TRACTOR)
	{		
		StickAlphaOffset = 0.0f;
	}
	else if(pVehicle->GetModelIndex() == MODELID_CAR_RCTIGER
// 	|| pVehicle->GetModelIndex()==MODELID_CAR_RHINO		//gonna use this to control the tank turret instead of disabling it
//	|| pVehicle->GetModelIndex()==MODELID_CAR_FIRETRUCK //gonna use this to control the firetruck hose instead of disabling it
	|| (pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR && ((CAutomobile *)pVehicle)->hFlagsLocal &HF_HYDRAULICS_INSTALLED))
	{
		StickAlphaOffset = 0.0f;
		StickBetaOffset = 0.0f;
	}
	else
		bFixAlphaAngle = false;

	// Dw - to prevent wierd shit on radar - as per bug issued... 
	if (DirectionIsLooking!=LOOKING_FORWARD)
	{
		StickAlphaOffset = 0.0f;
		StickBetaOffset = 0.0f;	
	}
	
	if(nType==FOLLOW_CAR_INCAR && CMaths::Abs(pPad->GetSteeringUpDown()) > 120.0f
	&& pVehicle->pDriver && pVehicle->pDriver->GetPedIntelligence()->GetTaskActive() && pVehicle->pDriver->GetPedIntelligence()->GetTaskActive()->GetTaskType()!=CTaskTypes::TASK_COMPLEX_LEAVE_CAR)
	{
		float fExtraLeftStickMod = pPad->GetSteeringUpDown();
		fExtraLeftStickMod = 0.5f*AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(fExtraLeftStickMod)*fExtraLeftStickMod * (0.15f/3.5f * (FOV/80.0f));
		StickAlphaOffset += fExtraLeftStickMod;
	}
	
	// slow down how fast the camera swings downwards
//	static float CAM_STICK_ALPHA_SWING_DOWN_MULT = 6.0f;
//	if(StickAlphaOffset > 0.0f && Alpha > -fCamAlpha)
//		StickAlphaOffset *= 1.0f - CMaths::Min(0.8f, CAM_STICK_ALPHA_SWING_DOWN_MULT*(Alpha + fCamAlpha));
	if(StickAlphaOffset > 0.0f)
		StickAlphaOffset *= 0.5f;
	
#ifdef GTA_PC
	bool bUsingMouse = false;
	if(TheCamera.m_bUseMouse3rdPerson && pPad->DisablePlayerControls==0)
	{
		float fStickY = pPad->GetAmountMouseMoved().y * 2.0f;
		float fStickX = -pPad->GetAmountMouseMoved().x * 2.0f;

		static float MOUSE_INPUT_COUNTER = 0.0f;

		bool8 mouseSteeringOn = true;
		if (pVehicle->GetVehicleType()==VEHICLE_TYPE_PLANE || pVehicle->GetVehicleType()==VEHICLE_TYPE_HELI)
		{
			mouseSteeringOn = CVehicle::m_bEnableMouseFlying;
		}
		else
		{
			mouseSteeringOn = CVehicle::m_bEnableMouseSteering;
		}

		if((fStickX != 0.0f || fStickY != 0.0f) && (pPad->GetVehicleMouseLook() || !mouseSteeringOn))
		{
			StickAlphaOffset = TheCamera.m_fMouseAccelHorzntl * fStickY * (FOV/80.0f);
			StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
			AlphaSpeed = BetaSpeed = 0.0f;
			fTargetAlpha = Alpha;
		
			// how long after mouse input before camera returns to normal motion
			static float MOUSE_INPUT_BUFFER_TIME = 1.0f;

			MOUSE_INPUT_COUNTER = FRAMES_PER_SECOND * MOUSE_INPUT_BUFFER_TIME;
			bUsingMouse = true;
		}
		else if(MOUSE_INPUT_COUNTER > 0.0f)
		{
			StickAlphaOffset = 0.0f;
			StickBetaOffset = 0.0f;
			AlphaSpeed = BetaSpeed = 0.0f;
			fTargetAlpha = Alpha;

			MOUSE_INPUT_COUNTER = CMaths::Max(0.0f, MOUSE_INPUT_COUNTER - CTimer::GetTimeStep());
			bUsingMouse = true;
		}
	}
#endif		

	if(pVehicle->pPassengers[0] && pVehicle->pPassengers[0]->GetPedIntelligence()->GetTaskActive()
	&& pVehicle->pPassengers[0]->GetPedIntelligence()->GetTaskActive()->GetTaskType()==CTaskTypes::TASK_COMPLEX_PROSTITUTE_SOLICIT)
	{
		if(((CTaskComplexProstituteSolicit *)pVehicle->pPassengers[0]->GetPedIntelligence()->GetTaskActive())->IsInSexCamMode())
		{
			static float PROSTITUTE_CAM_ALPHA_ANGLE = 0.1f;
			static float PROSTITUTE_CAM_ALPHA_RATE = 0.0035f;
			if(Alpha < fAlphaUpLimit - PROSTITUTE_CAM_ALPHA_ANGLE)
				StickAlphaOffset = PROSTITUTE_CAM_ALPHA_RATE*CTimer::GetTimeStep();
			else
				StickAlphaOffset = 0.0f;
		}
	}
	
	if (bFixAlphaAngle)		
	{
		// has this just happened 
		// then simulate to -fCamAlpha
		static float gStickAlphaFix = 0.05f;
		static bool gAcquiredAlpha = false;
		
		if (gLastCamMode != MODE_CAM_ON_A_STRING)
			gAcquiredAlpha = false; // If just got into this vehicle

		if(!gAcquiredAlpha && CMaths::Abs(Alpha + fCamAlpha) > 0.05f)
			StickAlphaOffset = (-fCamAlpha - Alpha)*gStickAlphaFix;
		else			
			gAcquiredAlpha = true; // clear static
	}
	

	StickAlphaOffset *= CARCAM_SET[nType].fStickMult;
	StickBetaOffset *= CARCAM_SET[nType].fStickMult;
	
//	fDiffMult = CARCAM_SET[nType].fDiffBetaRate*CTimer::GetTimeStep();
	fDiffMult = CMaths::Pow(CARCAM_SET[nType].fDiffBetaRate, CTimer::GetTimeStep());
	fDiffCap = CARCAM_SET[nType].fDiffBetaCap;

	fCamControlBetaSpeed += StickBetaOffset;

	if(fCamControlBetaSpeed > fDiffCap)
		fCamControlBetaSpeed = fDiffCap;
	else if(fCamControlBetaSpeed < -fDiffCap)
		fCamControlBetaSpeed = -fDiffCap;

//	BetaSpeed = fDiffMult*fCamControlBetaSpeed + (1.0f - fDiffMult)*BetaSpeed;
	BetaSpeed = fDiffMult*BetaSpeed + (1.0f - fDiffMult)*fCamControlBetaSpeed;

	static float gBetaSpeedTol  =0.0001f;		
	if (CMaths::Abs(BetaSpeed)<gBetaSpeedTol)
		BetaSpeed = 0.0f;

#ifdef GTA_PC
	if(bUsingMouse)
		Beta += StickBetaOffset;
	else
#endif
	Beta += BetaSpeed*CTimer::GetTimeStep();
	
	if (TheCamera.m_bJustCameOutOfGarage) // dont want to change alpha and beta!... hack hack hack
		Beta = CGeneral::GetATanOfXY(Front.x,Front.y) + PI;	
	
	ClipBeta();

// WARNING! this code is supposed to help the camera swing up as the target goes over the brow of hills
// but it seems to be causing some jerks in the camera motion so I need to figure out a better way to do this
	if(nType <= FOLLOW_CAR_ONBIKE && fTargetAlpha < Alpha && fTempLength >= fCamDistance)
	{
		int nWheelsOnGround = 0;
		if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_CAR)
			nWheelsOnGround = ((CAutomobile *)pVehicle)->nNoOfContactWheels;
		else if(pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
			nWheelsOnGround = ((CBike *)pVehicle)->nNoOfContactWheels;
		
		if(nWheelsOnGround > 1)
		{
			static float CREST_HILL_STICK_MULT = 0.075f;
			StickAlphaOffset += (fTargetAlpha - Alpha) * CREST_HILL_STICK_MULT;
		}
	}

//	AlphaSpeed = fDiffMult*StickAlphaOffset + (1.0f - fDiffMult)*AlphaSpeed;
	AlphaSpeed = fDiffMult*AlphaSpeed + (1.0f - fDiffMult)*StickAlphaOffset;
	// slow how fast the camera swings down off the stick
	if(StickAlphaOffset > 0.0f)
		fDiffCap *= 0.5f;
	
	if(AlphaSpeed > fDiffCap)
		AlphaSpeed = fDiffCap;
	else if(AlphaSpeed < -fDiffCap)
		AlphaSpeed = -fDiffCap;


	static float gAlphaSpeedTol  =0.0001f;		
	if (CMaths::Abs(AlphaSpeed)<gAlphaSpeedTol)
		AlphaSpeed = 0.0f;
		
#ifdef GTA_PC
	if(bUsingMouse)
	{
		fTargetAlpha += StickAlphaOffset;
		Alpha += StickAlphaOffset;
	}
	else
#endif
	{
		fTargetAlpha += AlphaSpeed*CTimer::GetTimeStep();
		Alpha += fTargetDiff;
	}

	if(Alpha > fAlphaUpLimit)//CARCAM_SET[nType].fUpLimit)
	{
		Alpha = fAlphaUpLimit;//CARCAM_SET[nType].fUpLimit;
		AlphaSpeed = 0.0f;
	}
	else if(Alpha < -CARCAM_SET[nType].fDownLimit)
	{
		Alpha = -CARCAM_SET[nType].fDownLimit;
		AlphaSpeed = 0.0f;
	}
////////////////

	static float gOldAlpha = -9999.0f;
	static float gOldBeta = -9999.0f;
	static float gAlphaTol = 0.0001f;
	static float gBetaTol = 0.0001f;
	
	float dA = CMaths::Abs(gOldAlpha - Alpha);
	if (dA<gAlphaTol)
		Alpha = gOldAlpha;
	gOldAlpha = Alpha;		
	
	float dB = CMaths::Abs(gOldBeta - Beta);
	if (dB<gBetaTol)
		Beta = gOldBeta;
	gOldBeta = Beta;		
		
	Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));

	VecTrunc(&Source);

	GetVectorsReadyForRW();

	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

	Source  = vecTargetCoords - Front * fCamDistance;
	m_cvecTargetCoorsForFudgeInter = vecTargetCoords;//ThisCamsTarget;
	
	fTargetAlpha += fCamAlpha;
	m_aTargetHistoryPos[2] = m_aTargetHistoryPos[0];
	m_aTargetHistoryPos[0] = vecTargetCoords - fFollowDist * CVector(-CMaths::Cos(Beta) *CMaths::Cos(fTargetAlpha) , -CMaths::Sin(Beta)*CMaths::Cos(fTargetAlpha), CMaths::Sin(fTargetAlpha));
	m_aTargetHistoryPos[1] = vecTargetCoords - fCamDistance * CVector(-CMaths::Cos(Beta) *CMaths::Cos(fTargetAlpha) , -CMaths::Sin(Beta)*CMaths::Cos(fTargetAlpha), CMaths::Sin(fTargetAlpha));
	
//////////////////////////////////////
// this chunk of code is pretty tempory, kinda copied from original follow ped
// and used in Process_AimWeapon(), Process_Follow_History(), and new Process_FollowPed()
// as a quick fix to stop the camera clipping through shit
// would like to smooth it all out and do some kind of predictive moving the camera forward like
// in MH, but don't want to start fucking around with the camera angles because that gets messy
//////////////////////////////////////
#ifdef DW_CAMERA_COLLISION

	TheCamera.SetColVarsVehicle(nType,TheCamera.m_nCarZoom);
	
	if (DirectionIsLooking==LOOKING_FORWARD)
	{
		CWorld::pIgnoreEntity = pVehicle;
		// Vehicle follow cam					
		// DW - TODO : If car moving slowly dont do any checks but still must zoom out...	
		TheCamera.CameraGenericModeSpecialCases(NULL);
		TheCamera.CameraVehicleModeSpecialCases(pVehicle);

		extern bool gTopSphereCastTest;
		bool bTopCast = gTopSphereCastTest;
		if (pVehicle->m_nVehicleFlags.bIsBig)
		{				
			gTopSphereCastTest = true;
		}

		TheCamera.CameraColDetAndReact(&Source,&vecTargetCoords);	
								
		// now col det and react may have set the clip distance we now overide the clip distance to improve Z fighting in some circumstances
		TheCamera.ImproveNearClip(pVehicle,NULL,&Source,&vecTargetCoords);	

#if 0 // not required for PC build now since I fixed the collision prediction algorithm
		if (pVehicle->m_nVehicleFlags.bIsBig)
		{
			gTopSphereCastTest = bTopCast;
			if (CheckCollisionAboveVehicle(pVehicle, &vecTargetCoords))
			{
				Process_1stPerson(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			}
		}
#endif					
		CWorld::pIgnoreEntity = NULL;
		
		VecTrunc(&Source);
	}
#else
	// See at bottom of file comment @BLOCK 3@
#endif

	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

	VecTrunc(&Source);

	GetVectorsReadyForRW();
		
	gTargetCoordsForLookingBehind = vecTargetCoords;
		
////////////////////////////
// end of nasty chunk
////////////////////////////
}

#if 0 // not required for PC version - DW
static int32 framesBadCollisionAbove = 0;
static int32 framesToStickCameraIntoFP = 10; // stick it into first person, this time after consistent collision
static int32 frameCounterTestAbove = 0;
static int32 frameCounterTestAboveRate = 10; // test every n frames, optimisation
static int32 numFramesToStayInFPAtLeastFor = 60*1;
static int32 framesInFP = 0;

void CCam::ResetFirstPersonUpCastFixer()
{
	framesBadCollisionAbove = 0;
	framesToStickCameraIntoFP = 10; // stick it into first person, this time after consistent collision
	frameCounterTestAbove = 0;
	frameCounterTestAboveRate = 10; // test every n frames, optimisation
	numFramesToStayInFPAtLeastFor = 60*1;
	framesInFP = 0;
}
#endif

#if 0 // not required for PC version - DW
//----------------------------------------------------------------------
// for camera tunnel fixes - DW
bool CCam::CheckCollisionAboveVehicle(CVehicle *pVehicle, CVector *pVec)
{	
	if (frameCounterTestAbove++>frameCounterTestAboveRate || framesBadCollisionAbove>0)
	{
		frameCounterTestAbove = 0;
		
		static float heightToScanForCollisionAbove = 0.5f;	
		CVector start = *pVec;
		CVector end = *pVec;		
		end.z += heightToScanForCollisionAbove;	
				
		CColModel &vehCol = pVehicle->GetColModel();		
		start.z = CMaths::Min(vehCol.GetBoundBoxMax().z+pVehicle->GetPosition().z,pVec->z);
		
		// paranoia	
		if (start.z<pVehicle->GetPosition().z)
			start.z = pVehicle->GetPosition().z;			

		// some vehicles need a ray cast up to determine if they are in a tunnel
		if (TheCamera.RayCastForNastyCollisionRightAboveCar(&start,&end))
			framesBadCollisionAbove++;
		else
			framesBadCollisionAbove=0;
	}			

	if (framesBadCollisionAbove>=framesToStickCameraIntoFP)
		framesInFP = numFramesToStayInFPAtLeastFor;
	else
		framesInFP--;

	if (framesInFP>0)
		return true;
	else
		framesInFP = 0;
		
	return false;
}

//---------------------------
// fixes for tunnels etc - DW
bool CCamera::RayCastForNastyCollisionRightAboveCar(CVector *pSrc, CVector *pTarget)
{

	CVector a = *pSrc;
	CVector b = *pTarget; 	
	bool bCollided = !CWorld::GetIsLineOfSightClear(a,b,1,0,0,0,0,0,0);		
	return bCollided;
}
#endif

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_1stPerson()
// PURPOSE    : 1st person mode. Different for car and ped
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
int BOAT_UNDERWATER_CAM_BLUR = 20;
float BOAT_UNDERWATER_CAM_COLORMAG_LIMIT = 10.0f;
int BOAT_UNDERWATER_CAM_ALPHA_LIMIT = 200;
float BOAT_UNDERWATER_CAM_DEPTH_SCALE = 1.0f;
//
float fBike1stPersonOffsetZ = 0.15f;
float fDuckingRightOffset = 0.18f;
float fDuckingBackOffset = 0.5f;
//
void CCam::Process_1stPerson(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	gbFirstPersonRunThisFrame = true;

	CVector TargetCoors;
	float MaxHorizontalAngle=DEGTORAD(70.0f);//how far can he turn his neck
	float MaxVerticalRotation=DEGTORAD(89.5f);
	float MaxRotationUp=DEGTORAD(60.0f);
	float MaxRotationDown=DEGTORAD(89.5f);
 	float fStickX=0.0f; 
	float fStickY=0.0f; 
	float StickBetaOffset=0.0f;
	float StickAlphaOffset=0.0f;
	float DistBack=0.3f;

	float HeightToNose=0.10f;
	float DistToNose=0.19f;
	float DeltaBeta=0.0f, DeltaAlpha=0.0f, ReqSpeed=0.0f, BetaMultiplier=0.0f, BetaStep=0.0f, AlphaMultiplier=0.0f, AlphaStep=0.0f ;
	static float DontLookThroughWorldFixer=0.0f;// 	
	TargetCoors=ThisCamsTarget;
	BetaMultiplier= 0.1f;
	BetaStep=0.015f;
	AlphaMultiplier= 0.1f;
	AlphaStep=0.015f;
	FOV=70.0f;		
	
	if (CamTargetEntity->GetRwObject()!=NULL)//stop crashing in into in game
	{	
		if (ResetStatics)
		{
			Beta=TargetOrientation;//makes player initially look in the directioin that the player is facing
			Alpha=0.0f;
			m_fInitialPlayerOrientation=TargetOrientation;
			if (CamTargetEntity->GetIsTypePed())
			{
				Beta=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;//makes player initially look in the directioin that the player is facing
				Alpha=0.0f;
				m_fInitialPlayerOrientation=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;
			}	
			DontLookThroughWorldFixer=0.0f;//for when we are upside down
			TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
		}	
		
		
		//if (CWorld::Players[CWorld::PlayerInFocus].pVehicle==NULL)
		if (CamTargetEntity->GetIsTypePed())
		{
			ASSERT(FALSE); // eek It thought this was redundant DW.
/*
			if (((CPed*)CamTargetEntity)->m_nPedFlags.bIsDucking) 
			{
			 	DistBack=0.8f;
			}			
			
			float StartingDPadHoriz=70.0f;
			float StartingDPadVert=70.0f;
			static bool FailedTestTwelveFramesAgo=false;
			static float DPadHorizontal;
			static float DPadVertical;
			
				
			TargetCoors=ThisCamsTarget;
			
			ASSERTMSG(CamTargetEntity->GetIsTypePed(), "flubba dubba");
				
			if (ResetStatics)
			{
				Beta=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;//makes player initially look in the directioin that the player is facing
				Alpha=0.0f;
				m_fInitialPlayerOrientation=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;	
				ResetStatics=FALSE;
				FailedTestTwelveFramesAgo=false;
				DPadHorizontal=0;
				DPadVertical=0;
				m_bCollisionChecksOn=true;
			}	
			
			/////////////////////////////////////
			((CPed*)CamTargetEntity)->UpdateRwMatrix();
			((CPed*)CamTargetEntity)->UpdateRwFrame();
			((CPed*)CamTargetEntity)->UpdateRpHAnim();
			/////////////////////////////////////
			CVector posn;
			((CPed*)CamTargetEntity)->GetBonePosition(posn, BONETAG_HEAD, true);
			
			Source=posn;
			Source.z+= HeightToNose;
			
			// when ducking the ped's head tends to move further forward and a bit right - need to compensate
			if(((CPed*)CamTargetEntity)->m_nPedFlags.bIsDucking)
			{
				Source.x -= fDuckingBackOffset*CamTargetEntity->GetMatrix().GetForward().x;//CMaths::Cos(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
				Source.y -= fDuckingBackOffset*CamTargetEntity->GetMatrix().GetForward().y;//CMaths::Sin(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
				Source.x -= fDuckingRightOffset*CamTargetEntity->GetMatrix().GetRight().x;
				Source.y -= fDuckingRightOffset*CamTargetEntity->GetMatrix().GetRight().y;
			}
			else
			{
				Source.x -= DistBack*CamTargetEntity->GetMatrix().GetForward().x;//CMaths::Cos(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
				Source.y -= DistBack*CamTargetEntity->GetMatrix().GetForward().y;//CMaths::Sin(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
			}
			 
			fStickX= 	-(CPad::GetPad(0)->LookAroundLeftRight());
			fStickY=	CPad::GetPad(0)->LookAroundUpDown();	
				
			float X_Sign=1.0f;
			float Y_Sign=1.0f;
			
			if (fStickX<0){X_Sign=-1.0f;}
			if (fStickY<0){Y_Sign=-1.0f;}

			StickBetaOffset=X_Sign * (fStickX/100.0f * fStickX/100.0f) * (0.20f/3.5f * (FOV/80.0f));
			StickAlphaOffset=Y_Sign *(fStickY/150.0f * fStickY/150.0f) * (0.25f/3.5f * (FOV/80.0f));
			
			Beta+= StickBetaOffset;
			Alpha+=StickAlphaOffset;						
			ClipBeta();
			
			if (Alpha>(0.0f + MaxRotationUp))
			{Alpha=0.0f + MaxRotationUp;}
			else if (Alpha<(0.0f-MaxRotationDown))
			{Alpha=0.0f-MaxRotationDown;}
			////////////////////////
			
			TargetCoors=Source + 3 * CVector(CMaths::Cos(Beta) *CMaths::Cos(Alpha) , CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
			
			
			Front = TargetCoors- Source;
			Front.Normalise();
			
			Source+=0.4f*Front;//so it doesn't count the player
							  //this is fixed back later on	
			
			//right lets check 2 meters in front of the player if we find anything set the clipping distance to short
			if (m_bCollisionChecksOn==true)
			{
				if (!(CWorld::GetIsLineOfSightClear(TargetCoors, Source ,1, 1 , 0, true ,0, true, true)))
			 	{	
				 	RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
				 	FailedTestTwelveFramesAgo=true;
			 	}
				else  
				{
					CVector TestTarget;
					TestTarget=Source + 3 * CVector(CMaths::Cos(Beta + DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta+ DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
					if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
			 		{		
						RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
						FailedTestTwelveFramesAgo=true;
					}
					else
					{
						TestTarget=Source + 3 * CVector(CMaths::Cos(Beta - DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta - DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
						if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
				 		{
					 		RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
							FailedTestTwelveFramesAgo=true;
						}
						else
						{
							FailedTestTwelveFramesAgo=false;
						}
					}
				}
			}
			
			if (FailedTestTwelveFramesAgo)
			{
				RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP);
			} //for occasions when the spaz user is right up against a wall etc
			
			
			Source-=0.4f*Front;
			GetVectorsReadyForRW();
			float CamDirection;
			CamDirection=CGeneral::GetATanOfXY(Front.x, Front.y)-(PI/2);
			((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
			((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;
*/
		}
		else
		{
			static float timeLastWheelieWheelCam = -1;
			static float timeMinimumForWheelieWheelCam = 3000;
						
			float time = CTimer::GetTimeInMilliseconds();

			if (timeLastWheelieWheelCam>time)	// reset this static global if its ever bad.
				timeLastWheelieWheelCam = 0;			
			
			float dTime = time - timeLastWheelieWheelCam;
			if( (dTime<timeMinimumForWheelieWheelCam) || (
				((CVehicle *)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE
			&&  (((CBike *)CamTargetEntity)->m_nBikeFlags.bWheelieForCamera || TheCamera.m_fAvoidTheGeometryProbsTimer > 0.0f)) )
			{			
				if (dTime>=timeMinimumForWheelieWheelCam)
					timeLastWheelieWheelCam	= time;
					
				if(!CPad::GetPad(0)->GetLeftShoulder2() && !CPad::GetPad(0)->GetRightShoulder2())
				{
					if(CCam::Process_WheelCam(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired))
					{
						if(((CBike *)CamTargetEntity)->m_nBikeFlags.bWheelieForCamera)
							TheCamera.m_fAvoidTheGeometryProbsTimer = 50.0f;
						else{
							TheCamera.m_fAvoidTheGeometryProbsTimer -= CTimer::GetTimeStep();
							((CBike *)CamTargetEntity)->m_nBikeFlags.bWheelieForCamera = true;
						}							
						return;
					}
					// else tried but line check failed
					else
					{
						timeLastWheelieWheelCam = 0; // dont persist with wheelie wheel cam, as position is bad!
						TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
						((CBike *)CamTargetEntity)->m_nBikeFlags.bWheelieForCamera = false;
					}
				}
/*				else
				{
					TheCamera.m_fAvoidTheGeometryProbsTimer = 0.0f;
					((CBike *)CamTargetEntity)->m_nBikeFlags.bWheelieForCamera = false;
				}*/
			}
			
		//	RwCameraSetNearClipPlane(Scene.camera, (RwReal) FIRSTPERSON_CAR_NEAR_CLIP);
			CVector TempRight;
			float HeightOffSet=0.0f;
			float DoorDepthOffSet=0.0f;
		    CVehicle *pVehicle;
			CVector vecDoorPosition;
			HeightOffSet=0.62f;
			DoorDepthOffSet=0.08f;

			const CMatrix *pTargetMatrix = &(CamTargetEntity->GetMatrix());
			if(((CVehicle *)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
			{
				((CBike *)CamTargetEntity)->CalculateLeanMatrix();
				pTargetMatrix = &(((CBike *)CamTargetEntity)->m_LeanMatrix);
			}
			
			pVehicle = (CVehicle *)CamTargetEntity;
			CVehicleModelInfo* pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(pVehicle->GetModelIndex());
			vecDoorPosition = pModelInfo->GetFrontSeatPosn(); 
	//		vecDoorPosition.x-=vecDoorPosition.x/0.59f;
			vecDoorPosition.x=0.0f;
			vecDoorPosition.y+=DoorDepthOffSet;
			vecDoorPosition.z+=HeightOffSet;
			FOV=60.0f;
			
			Source=Multiply3x3( *pTargetMatrix, vecDoorPosition);
			Source+=CamTargetEntity->GetPosition();
			
			if (((CVehicle*)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
			{
				Source.z+=ZOFFSET1RSTPERSONBOAT;
			}
			else if(((CVehicle *)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
			{
				if(((CVehicle *)CamTargetEntity)->pDriver)
				{
					CVector vecShoulderHeight(0.0f,0.0f,0.0f);
					((CVehicle *)CamTargetEntity)->pDriver->GetBonePosition(vecShoulderHeight, BONETAG_NECK, true);
					vecShoulderHeight += ((CVehicle *)CamTargetEntity)->m_vecMoveSpeed*CTimer::GetTimeStep();
					Source.z = vecShoulderHeight.z + fBike1stPersonOffsetZ;
					
					//--------------------------------------------------------------------------------
					// DW - This stops bike leaning through collision in first person
					CVector right = CrossProduct(Front,Up);
					static float scaleSideVec = 1.0f;
					right *= scaleSideVec;
					CVector a = Source + right;
					CVector b = Source - right;
					if (!(CWorld::GetIsLineOfSightClear(a,b,1,0,0,0,0,0,0)))
					{
						Source = CamTargetEntity->GetPosition();
						Source.z = HeightOffSet + vecShoulderHeight.z + fBike1stPersonOffsetZ;
					}
					//--------------------------------------------------------------------------------
				}
			}

			

			if (((CVehicle*)CamTargetEntity)->IsUpsideDown())
			{
				float TargetDontLookThroughWorldFixer=0.5f;
				if (DontLookThroughWorldFixer<TargetDontLookThroughWorldFixer)
				{
					DontLookThroughWorldFixer+=0.03f;
				}
				else
				{
					DontLookThroughWorldFixer=TargetDontLookThroughWorldFixer;
				}
			}
			else
			{
				if (DontLookThroughWorldFixer<0.0f)
				{
					DontLookThroughWorldFixer-=0.03f;
				}
				else
				{
					DontLookThroughWorldFixer=0.0f;
				}
			}
			
			//Front=Source-TargetCoors;
			Source.z+=DontLookThroughWorldFixer;

			Front=pTargetMatrix->GetForward();
			Front.Normalise();
			
			Up=pTargetMatrix->GetUp();
			
			Up.Normalise();
			TempRight=CrossProduct(Front, Up);
			TempRight.Normalise();
			Up=CrossProduct(TempRight, Front);
			Up.Normalise();


#ifdef GTA_SA
			float fCamWaterLevel = 0.0f;
			if( CWaterLevel::GetWaterLevel(Source.x, Source.y, Source.z, &fCamWaterLevel, true) )
			{
				if(Source.z < fCamWaterLevel - 0.3f)
				{
					float fWaterColourMag = CMaths::Sqrt(CTimeCycle::GetWaterRed()*CTimeCycle::GetWaterRed() + CTimeCycle::GetWaterGreen()*CTimeCycle::GetWaterGreen() + CTimeCycle::GetWaterBlue()*CTimeCycle::GetWaterBlue());
					if(fWaterColourMag > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT)
					{
						fWaterColourMag = BOAT_UNDERWATER_CAM_COLORMAG_LIMIT/fWaterColourMag;
						TheCamera.SetMotionBlur(fWaterColourMag*CTimeCycle::GetWaterRed(), fWaterColourMag*CTimeCycle::GetWaterGreen(), fWaterColourMag*CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
					}
					else
						TheCamera.SetMotionBlur(CTimeCycle::GetWaterRed(), CTimeCycle::GetWaterGreen(), CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
				}
			}
#endif		
		}
	
		ResetStatics=FALSE;
	}
}

// offset to where we want the camera to be in relation to playerped's head
static CVector vecHeadCamOffset(0.06f, 0.05f, 0.0f);

void CCam::Process_1rstPersonPedOnPC(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	#ifdef GTA_PC
	{
		CVector TargetCoors;
		float MaxHorizontalAngle=DEGTORAD(70.0f);//how far can he turn his neck
		float MaxVerticalRotation=DEGTORAD(89.5f);
		float MaxRotationUp=DEGTORAD(60.0f);
		float MaxRotationDown=DEGTORAD(89.5f);
	 	float fStickX=0.0f; 
		float fStickY=0.0f; 
		float StickBetaOffset=0.0f;
		float StickAlphaOffset=0.0f;
		float DistBack=0.13f;
		const float CloseDistCamToWall=0.75f ;
//		float HeightToNose=-0.10f;//0.10
		float DeltaBeta=0.0f, DeltaAlpha=0.0f, ReqSpeed=0.0f, BetaMultiplier=0.0f, BetaStep=0.0f, AlphaMultiplier=0.0f, AlphaStep=0.0f ;
		static float DontLookThroughWorldFixer=0.0f;// 	
		float HeadToWallDist=0.0f;
		CColPoint colPoint;
		static CVector InitialHeadPos;
		TargetCoors=ThisCamsTarget;
		BetaMultiplier= 0.1f;
		BetaStep=0.015f;
		AlphaMultiplier= 0.1f;
		AlphaStep=0.015f;
		// need to allow zooming in sniper mode
		if(Mode != MODE_SNIPER_RUNABOUT)
			FOV=70.f;
		TheCamera.m_1rstPersonRunCloseToAWall=false;
		if (CamTargetEntity->GetRwObject()!=NULL)//stop crashing in into in game
		{	
			if (CamTargetEntity->GetIsTypePed())
			{
				float StartingDPadHoriz=70.0f;
				float StartingDPadVert=70.0f;
				static bool FailedTestTwelveFramesAgo=false;
				static float DPadHorizontal;
				static float DPadVertical;

				// want to add an offset to the head posn to move it closer to eye height
				// do it this way so it gets transformed relative to the head's orientation (eg looking up/down)
				CVector posn = vecHeadCamOffset;				
				RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)(CamTargetEntity->m_pRwObject));
				int32 nBone = ConvertPedNode2BoneTag(PED_HEAD);
				
				// convert to index
				nBone = RpHAnimIDGetIndex(pHierarchy, nBone);
				
				// hmm try this 1st
				RwMatrix *pMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + nBone);
				RwV3dTransformPoints(&posn, &posn, 1, pMatrix);
				
				// try and scale the matrix size to zero (effectively turn head off)
		        RwV3d vecScale;
		        vecScale.x = 0.0f; vecScale.y = 0.0f;	vecScale.z = 0.0f;
        		RwMatrixScale(pMatrix, &vecScale, rwCOMBINEPRECONCAT);
				if (ResetStatics)
				{
					//CPad::GetPad(0)->ClearMouseHistory();
					Beta=TargetOrientation;//makes camera initially look in the directioin that the player is facing
					Alpha=0.0f;
					m_fInitialPlayerOrientation=TargetOrientation;
					if (CamTargetEntity->GetIsTypePed())
					{
						Beta=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;//makes camera initially look in the directioin that the player is facing
						Alpha=0.0f;
						m_fInitialPlayerOrientation=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;
					    FailedTestTwelveFramesAgo=false;
						DPadHorizontal=0;
						DPadVertical=0;
						m_bCollisionChecksOn=true;
					}	
					DontLookThroughWorldFixer=0.0f;//for when we are upside down
					m_vecBufferedPlayerBodyOffset = posn;	
					InitialHeadPos = posn;

				}
				
				m_vecBufferedPlayerBodyOffset.y = posn.y;
				// update the buffered offset to the ped's body
				if ((TheCamera.m_bHeadBob))
				{
					m_vecBufferedPlayerBodyOffset.x = TheCamera.GetGaitSway()*m_vecBufferedPlayerBodyOffset.x + (1.0f-TheCamera.GetGaitSway())*posn.x;
					m_vecBufferedPlayerBodyOffset.z = TheCamera.GetGaitSway()*m_vecBufferedPlayerBodyOffset.z + (1.0f-TheCamera.GetGaitSway())*posn.z;
					posn = CamTargetEntity->GetMatrix() * m_vecBufferedPlayerBodyOffset;
				}
				else
				{
					CVector HeadDiff=posn-InitialHeadPos;
					CVector PlayHead=CamTargetEntity->GetMatrix().GetForward();
					HeadDiff.z=0.0f;
					PlayHead.z=0.0f;
					PlayHead.Normalise();
					posn = CamTargetEntity->GetPosition() + (PlayHead * HeadDiff.Magnitude() * 1.23f);
					posn.z += 0.59f;
					
				}
				// transform through the ped matrix to get global posn of camera
				
				
				
					
				TargetCoors=ThisCamsTarget;
				
				ASSERTMSG(CamTargetEntity->GetIsTypePed(), "flubba dubba");
				
//				CVector posn;
//				((CPed*)CamTargetEntity)->m_ik.GetComponentPosition(posn, PED_HEAD);

				
	//			CVector ComponentHeadToBodDif(-0.166f, 0.03f, 0.637f);
	//			posn=CamTargetEntity->GetPosition() + ComponentHeadToBodDif;
				Source=posn;
				RwV3d TorsoPos;
				((CPed*)CamTargetEntity)->GetBonePosition(TorsoPos, BONETAG_SPINE1, true);
				CVector TempTorsoVec=CVector(Source-TorsoPos);
				float TorsoLength=TempTorsoVec.Magnitude();
//				Source.z+= HeightToNose;
				 
				bool bUsingMouse = false;
				RwV2d m_MouseMovement=CPad::GetPad(0)->GetAmountMouseMoved();
				if ((m_MouseMovement.x==0.0f) && (m_MouseMovement.y==0.0f))	
				{
					fStickX= 	-(CPad::GetPad(0)->LookAroundLeftRight());
					fStickY=	CPad::GetPad(0)->LookAroundUpDown();	
				}
				else
				{
					fStickX=-m_MouseMovement.x * 3.0f;//CTimer::GetTimeStep(); //	-(CPad::GetPad(0)->LookAroundLeftRight());
					fStickY=m_MouseMovement.y * 4.0f;//CTimer::GetTimeStep(); //	CPad::GetPad(0)->LookAroundUpDown();	
					bUsingMouse = true;
				}
	
				if(bUsingMouse)
				{
					StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
					StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (FOV/80.0f);
				}
				else
				{
					float X_Sign=1.0f;
					float Y_Sign=1.0f;
				
					if (fStickX<0){X_Sign=-1.0f;}
					if (fStickY<0){Y_Sign=-1.0f;}

					StickBetaOffset=X_Sign * (fStickX/100.0f * fStickX/100.0f) * (0.20f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
					StickAlphaOffset=Y_Sign *(fStickY/150.0f * fStickY/150.0f) * (0.25f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
				}
				
				Beta+= StickBetaOffset;
				Alpha+=StickAlphaOffset;						
				ClipBeta();
				
				if (Alpha>(0.0f + MaxRotationUp))
				{Alpha=0.0f + MaxRotationUp;}
				else if (Alpha<(0.0f-MaxRotationDown))
				{Alpha=0.0f-MaxRotationDown;}
				
				if( ((CPed*)CamTargetEntity)->IsPlayer() && ((CPlayerPed *)CamTargetEntity)->m_pAttachToEntity)
				{
					CPlayerPed *pPlayerPed = (CPlayerPed *)CamTargetEntity;
					float fDefaultHeading;
					switch(pPlayerPed->m_nAttachLookDirn)
					{
						case 0:	// forward (remember heading taken from x vector?)
							fDefaultHeading = pPlayerPed->m_pAttachToEntity->GetHeading() + HALF_PI;
							break;
						case 1:	// left
							fDefaultHeading = pPlayerPed->m_pAttachToEntity->GetHeading() + PI;
							break;
						case 2:	// back
							fDefaultHeading = pPlayerPed->m_pAttachToEntity->GetHeading() - HALF_PI;
							break;
						case 3: // right
							fDefaultHeading = pPlayerPed->m_pAttachToEntity->GetHeading();
							break;
					}
	
					float fCamDelta = Beta - fDefaultHeading;

					if(fCamDelta > PI)
						fCamDelta -= TWO_PI;
					else if(fCamDelta < -PI)
						fCamDelta += TWO_PI;
		
					if(fCamDelta > pPlayerPed->m_fAttachHeadingLimit)
					{
						fCamDelta = pPlayerPed->m_fAttachHeadingLimit;
					}
					else if(fCamDelta < -pPlayerPed->m_fAttachHeadingLimit)
					{
						fCamDelta = -pPlayerPed->m_fAttachHeadingLimit;
					}
				
					Beta = fDefaultHeading + fCamDelta;
				}
				
				////////////////////////
				
//				Source.x+=CMaths::Cos(Beta)*DistBack; //move a bit forward for the head 
//				Source.y+=CMaths::Sin(Beta)*DistBack;
				TargetCoors=Source + 3 * CVector(CMaths::Cos(Beta) *CMaths::Cos(Alpha) , CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
				
		/*		CVector HeadDisp;
				HeadDisp=WorkOutHeadDisplacement1rstPerson(Beta);
				Source+=HeadDisp;
				TargetCoors+=HeadDisp; 
		*/		
				Front = TargetCoors- Source;
				Front.Normalise();
				
				Source+=0.4f*Front;//so it doesn't count the player
								  //this is fixed back later on	
				
				//right lets check 2 meters in front of the player if we find anything set the clipping distance to short
//MARK COMMENT THIS BACK IN AND FIX	
/*	
				CVector TempCollsionVec; //used to work out the magnitude between the collision point and the camera

				if ((m_bCollisionChecksOn==true)||(FailedTestTwelveFramesAgo))
				{
					CEntity* refEntityPtr=NULL;
					float LengthFixerForBending=0.0f;
					
					//LengthFixerForBending=ABS(CloseDistCamToWall* CMaths::Cos(Alpha));
					
					
					
					if (CWorld::ProcessLineOfSight(Source, TargetCoors, colPoint, refEntityPtr,true, true , 0, true ,0, true, true))
					{
						TempCollsionVec=Source-colPoint.GetPosition();
						HeadToWallDist=TempCollsionVec.Magnitude();
						if (HeadToWallDist<=CloseDistCamToWall)
						{
							TheCamera.m_1rstPersonRunCloseToAWall=true;
						}
						RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
					 	FailedTestTwelveFramesAgo=true;
	
					}
					else  
					{
						CVector TestTarget;
						TestTarget=Source + 3 * CVector(CMaths::Cos(Beta + DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta+ DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
						
						
						if (CWorld::ProcessLineOfSight( Source,TestTarget, colPoint, refEntityPtr,true, true , 0, true ,0, true, true))
						{
							TempCollsionVec=Source-colPoint.GetPosition();
							HeadToWallDist=TempCollsionVec.Magnitude();
							if (HeadToWallDist<=CloseDistCamToWall)
							{
								TheCamera.m_1rstPersonRunCloseToAWall=true;
							}
							RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
						 	FailedTestTwelveFramesAgo=true;
		
						}
						else
						{
							TestTarget=Source + 3 * CVector(CMaths::Cos(Beta - DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta - DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
							if (CWorld::ProcessLineOfSight(Source, TestTarget, colPoint, refEntityPtr,true, true , 0, true ,0, true, true))
							{
								TempCollsionVec=Source-colPoint.GetPosition();
								HeadToWallDist=TempCollsionVec.Magnitude();
								if (HeadToWallDist<=CloseDistCamToWall)								
								{
									TheCamera.m_1rstPersonRunCloseToAWall=true;
								}
								RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
							 	FailedTestTwelveFramesAgo=true;
			
							}
							else
							{
								FailedTestTwelveFramesAgo=false;
							}
						}
					}
				}
				
//				if (FailedTestTwelveFramesAgo)
//				{
//					RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP);
//				} //for occasions when the spaz user is right up against a wall etc
				
				
				Source-=0.4f*Front;
				TheCamera.m_AlphaForPlayerAnim1rstPerson=Alpha;
				if (TheCamera.m_1rstPersonRunCloseToAWall)
				{
					float DistanceBack=HeadToWallDist-CloseDistCamToWall;//TempCollsionVec.Magnitude())/CMaths::Cos(Alpha);
					DistanceBack=DistanceBack *0.98f;
					CVector NewHeadPos= Source + (Front * (DistanceBack));//now been moved back the correct amount for X and Y 
					CVector TempHeadTorsoVec=NewHeadPos- TorsoPos;
					TempHeadTorsoVec.Normalise();
					TempHeadTorsoVec=TempHeadTorsoVec * TorsoLength;
					
					float HeightUp=TempHeadTorsoVec.z;
					ASSERT(HeightUp/TorsoLength<=1 && HeightUp/TorsoLength>=-1);
					//head now in valid position lets work out alpha
					TheCamera.m_AlphaForPlayerAnim1rstPerson=CMaths::ACos(HeightUp/TorsoLength);
				}
*/				
				TheCamera.m_AlphaForPlayerAnim1rstPerson=Alpha;

				GetVectorsReadyForRW();
				float CamDirection;
//				CamDirection=CGeneral::GetATanOfXY(Front.x, Front.y)-(PI/2);
				CamDirection = CMaths::ATan2(-Front.x, Front.y);
				((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
				((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;
				// nned to manually set the entity heading here as well to keep in sync with camera
				TheCamera.pTargetEntity->SetHeading(CamDirection);
				TheCamera.pTargetEntity->UpdateRwMatrix();
				
			
				if( Mode == MODE_SNIPER_RUNABOUT)
				{
					if ((CPad::GetPad(0)->SniperZoomOut()) || (CPad::GetPad(0)->SniperZoomIn())) 
					{
						if (CPad::GetPad(0)->SniperZoomOut()) 
						{
							FOV *= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
				//			TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE, MB_BLUR_SNIPER_ZOOM); //cause it gets subtracted later
						}
						else 
						{
							if (CPad::GetPad(0)->SniperZoomIn()) 
							{
								FOV /= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
				//				TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE,MB_BLUR_SNIPER_ZOOM); //cause it gets subtarcted later
							}
						}
					}

					TheCamera.SetMotionBlur(180,255,180,120/*145+MOTION_BLUR_ALPHA_DECREASE*/,MB_BLUR_SNIPER); //cause it gets subtarcted later

					// Clip the FOV to sensible values
					if (FOV > 70.0f) FOV = 70.0f; //for streaming
					if (FOV < 15.0f) FOV = 15.0f;
				}
				
			}
			ResetStatics=FALSE;
		
				RwCameraSetNearClipPlane(Scene.camera, 0.05f);

		
		}
		
//		CDebug::DebugLine3D(Source.x, Source.y, Source.z, Front.x, Front.y, Front.z, 0xffffffff, 0xffffffff);
	}
	#endif
}
///////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////

#ifdef GTA_PC
void CCam::Process_Editor(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
// This mode moves independant of any vehicles. It is
	// controlled by the joystick and the crosshair buttons.
	//RwCameraSetNearClipPlane(Scene.camera, (RwReal)(0.09));
	float Distance=0.0f;
	Distance=7.0f;
	static float Speed=0.0f;
	int16 XValues;
	int16 YValues;
	bool ChangeHeight=TRUE;
	CVector TargetCoors(0.0f,0.0f,0.0f);
//	CVector vecHeading = CVector(-5.0f,0.0f,-5.0f);
	
	if (ResetStatics)
	{
	 	Source=CVector(796.0f,-937.0f,40.0f);
		TIDYREF(CamTargetEntity, &CamTargetEntity);
	 	CamTargetEntity = NULL;
	}
	ResetStatics=false;
	
	RwCameraSetNearClipPlane(Scene.camera, (RwReal)(NORMAL_NEAR_CLIP));
	FOV=70.0f;
	
	XValues=CPad::GetPad(1)->GetLeftStickX() ;
	YValues=CPad::GetPad(1)->GetLeftStickY() ;
	
	Alpha+=(DEGTORAD(1) * YValues)/50.0f;///50;
	Beta+=(DEGTORAD(1.5f)  * XValues)/19.0f ;
	
//!PC - CSceneEdit disabled
	if(CamTargetEntity) // && CSceneEdit::m_bCameraFollowActor)
	{
		TargetCoors = CamTargetEntity->GetPosition(); 		
	}
	else
	{
		//!PC - CSceneEdit disabled
		//if(CSceneEdit::m_bRecording)
		if (FALSE)
		{
			TargetCoors = Distance * CVector(CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Cos(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha)); 		
			TargetCoors += Source;
		}
		else
		{
			//!PC - CSceneEdit disabled
			//TargetCoors =CSceneEdit::m_vecCamHeading; 		
			TargetCoors += Source;
		}
	}
	//!PC - CSceneEdit disabled
	//CSceneEdit::m_vecCurrentPosition = TargetCoors;
	//CSceneEdit::m_vecCamHeading = TargetCoors-Source;

	if (Alpha>DEGTORAD(89.5f))
	{Alpha=DEGTORAD(89.5f);}
	
	if (Alpha<DEGTORAD(-89.5f))
	{Alpha=DEGTORAD(-89.5f);}

#ifndef FINALBUILD		
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_HASH))
		TheCamera.bStaticFrustum = !TheCamera.bStaticFrustum;
#endif	
	else if  (CPad::GetPad(1)->GetButtonSquare()){Speed+=0.1f;}//0.5f;}
	else if  (CPad::GetPad(1)->GetButtonCross()){Speed-=0.1f;}//0.5f;}
	else {Speed=0.0f;}
	
	if (Speed>70){Speed=70;}
	if (Speed<-70){Speed=-70;}
	
	Front = TargetCoors - Source;
	Front.Normalise();
	
	if (ChangeHeight==TRUE)
	{
		Source+=(Speed * Front);
	}
	
	if (ChangeHeight==FALSE)
	{
		Source.x+=(Speed * Front.x);
		Source.y+=(Speed * Front.y);
	}	
	
	
	if (Source.z<-450.0f){Source.z=-450.0f;}
	
	if  (CPad::GetPad(1)->RightShoulder2JustDown())
	{
		if (FindPlayerVehicle() != NULL)
		{
			FindPlayerVehicle()->Teleport(Source, FALSE); //!PC added FALSE here
		}
		else ////will change this to teleport once the ped stuff is okay
		{
			CWorld::Players[CWorld::PlayerInFocus].pPed->SetPosition(Source.x, Source.y, Source.z);
		}

	}


	while (WORLD_WORLDTOSECTORX_FLOAT(Source.x)>(WORLD_WIDTHINSECTORS-5))
	{Source.x-=1.0f;}
	while (WORLD_WORLDTOSECTORX_FLOAT(Source.x)<(0+5))
	{Source.x+=1.0f;}
	while (WORLD_WORLDTOSECTORY_FLOAT(Source.y)>(WORLD_DEPTHINSECTORS-5))
	{Source.y-=1.0f;}
	while (WORLD_WORLDTOSECTORY_FLOAT(Source.y)<(0+5))
	{Source.y+=1.0f;}


	GetVectorsReadyForRW();


	// CVector Temp = coordinates of camera
	if( (!CPad::GetPad(1)->ShockButtonLJustDown()) && gbBigWhiteDebugLightSwitchedOn) 
	{
		CShadows::StoreShadowToBeRendered(SHAD_TYPE_ADDITIVE, gpShadowExplosionTex, &Source, 12.0f, 0.0f,
												0.0f, -12.0f, 128, 128, 128, 128, 1000.0f);
	}

	char Str[250];

	if (CHud::m_Wants_To_Draw_Hud)
	{
		sprintf(Str, "CamX: %0.3f CamY: %0.3f  CamZ:  %0.3f", Source.x, Source.y, Source.z) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
		CDebug::PrintToScreenCoors(Str, 8, 6);
		sprintf(Str, "Frontx: %0.3f, Fronty: %0.3f, Frontz: %0.3f ", Front.x, Front.y, Front.z ) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
		CDebug::PrintToScreenCoors(Str, 8, 7);
		sprintf(Str, "LookAT: %0.3f, LookAT: %0.3f, LookAT: %0.3f ", (Front.x + Source.x), (Front.y +Source.y), (Front.z + Source.z) ) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
		CDebug::PrintToScreenCoors(Str, 8, 8);
	}


}
#endif

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Debug()
// PURPOSE    : Processes the zelda-esque 'follow the ped' camera mode.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
//
#ifndef MASTER

static bool bCutSceneLastFrame = false;

void CCam::Process_Debug(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	// This mode moves independant of any vehicles. It is
	// controlled by the joystick and the crosshair buttons.
	//RwCameraSetNearClipPlane(Scene.camera, (RwReal)(0.09));
	float Distance=0.0f;
	Distance=3.0f;
	static float Speed=0.0f;
	int16 XValues;
	int16 YValues;
	bool ChangeHeight=TRUE;
	CVector TargetCoors;
	
	if (TheCamera.m_bUseNearClipScript)
	{
		RwCameraSetNearClipPlane(Scene.camera, TheCamera.m_fNearClipScript);
	}
	else
	{
		RwCameraSetNearClipPlane(Scene.camera, (RwReal)(NORMAL_NEAR_CLIP));
	}
	FOV=70.0f;
	
	XValues=CPad::GetPad(1)->GetLeftStickX() ;
	YValues=CPad::GetPad(1)->GetLeftStickY() ;
	
	Alpha+=(DEGTORAD(1) * YValues)/50.0f;///50;
	Beta+=(DEGTORAD(1.5f)  * XValues)/19.0f ;
	
	TargetCoors=Distance * CVector(CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Cos(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha)); 
	TargetCoors += Source;

	if (Alpha>DEGTORAD(89.5f))
	{Alpha=DEGTORAD(89.5f);}
	
	if (Alpha<DEGTORAD(-89.5f))
	{Alpha=DEGTORAD(-89.5f);}


	bool LeftMouseOn=false;
	bool RightMouseOn=false;
	
	#ifdef GTA_PC
	{
		if  (CPad::GetPad(1)->GetLeftMouseButtonOn()){LeftMouseOn=true;}//0.5f;}
		if  (CPad::GetPad(1)->GetRightMouseButtonOn()){RightMouseOn=true;}//0.5f;}
	}
	#endif

#ifndef FINALBUILD		
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_HASH))
		TheCamera.bStaticFrustum = !TheCamera.bStaticFrustum;
#endif	
	if  (CPad::GetPad(1)->GetButtonSquare()||(LeftMouseOn)){Speed+=0.1f;}//0.5f;}
	else if  (CPad::GetPad(1)->GetButtonCross()||(RightMouseOn)){Speed-=0.1f;}//0.5f;}
	else {Speed=0.0f;}

	


	
	if (Speed>70){Speed=70;}
	if (Speed<-70){Speed=-70;}
	
	Front = TargetCoors - Source;
	Front.Normalise();
	
	if (ChangeHeight==TRUE)
	{
		Source+=(Speed * Front);
	}
	
	if (ChangeHeight==FALSE)
	{
		Source.x+=(Speed * Front.x);
		Source.y+=(Speed * Front.y);
	}
	
	if(CPad::GetPad(1)->GetRightStickX())
	{
		Source.x += 0.0000006f*CMaths::Pow(CPad::GetPad(1)->GetRightStickX(),3.0f)*CTimer::GetTimeStep();
	}
	if(CPad::GetPad(1)->GetRightStickY())
	{
		Source.y += 0.0000006f*CMaths::Pow(CPad::GetPad(1)->GetRightStickY(),3.0f)*CTimer::GetTimeStep();
	}
	
	
	YValues=CPad::GetPad(1)->GetLeftStickY() ;
	
	
	if (Source.z<-450.0f){Source.z=-450.0f;}
	
	if (CPad::GetPad(1)->RightShoulder2JustDown())
	{
		if (FindPlayerVehicle(-1, true) != NULL)
		{
			FindPlayerVehicle(-1, true)->Teleport(Source, true);
		}
		else ////will change this to teleport once the ped stuff is okay
		{
			CWorld::Players[CWorld::PlayerInFocus].pPed->SetPosition(Source.x, Source.y, Source.z);
		}

	}

/*		Player is now allowed to leave the map area
	while (WORLD_WORLDTOSECTORX_FLOAT(Source.x)>(WORLD_WIDTHINSECTORS-0))
	{Source.x-=1.0f;}
	while (WORLD_WORLDTOSECTORX_FLOAT(Source.x)<(0+0))
	{Source.x+=1.0f;}
	while (WORLD_WORLDTOSECTORY_FLOAT(Source.y)>(WORLD_DEPTHINSECTORS-0))
	{Source.y-=1.0f;}
	while (WORLD_WORLDTOSECTORY_FLOAT(Source.y)<(0+0))
	{Source.y+=1.0f;}
*/

	GetVectorsReadyForRW();


	// CVector Temp = coordinates of camera
//	if( (!CPad::GetPad(1)->ShockButtonLJustDown()) && gbBigWhiteDebugLightSwitchedOn) 
	if (!PS2Keyboard.GetKeyJustDown(PS2_KEY_NUMPAD_7) && gbBigWhiteDebugLightSwitchedOn)
	{
		CShadows::StoreShadowToBeRendered(SHAD_TYPE_ADDITIVE, gpShadowExplosionTex, &Source, 12.0f, 0.0f,
												0.0f, -12.0f, 128, 128, 128, 128, 1000.0f);
	}

	//
	// display cam coords:
	//
	CVector posn = FindPlayerCoors();

	char TempString[200];
//	Char16 UniTempString[200];
	
	uint8 grid_pos_x = 0;
	uint8 grid_pos_y = 0;
	
	CGridRef::GetGridRefPositions(&grid_pos_x, &grid_pos_y);  // get the current grid position
	
	if (CHud::m_Wants_To_Draw_Hud)
	{
		if (CGridRef::displayCamCords)
		{
			sprintf(TempString, "FrontX: %0.3f, FrontY: %0.3f, FrontZ: %0.3f ", Front.x, Front.y, Front.z ) ;
			VarConsole.AddDebugOutput(TempString, TRUE);

			sprintf(TempString, "LookAT: %0.3f, LookAT: %0.3f, LookAT: %0.3f ", (Front.x + Source.x), (Front.y +Source.y), (Front.z + Source.z) ) ;
			VarConsole.AddDebugOutput(TempString, TRUE);
		}
		
		sprintf(TempString,"CAM: X:%0.2f Y:%0.2f Z:%0.2f",Source.x, Source.y, Source.z);
		VarConsole.AddDebugOutput(TempString, TRUE);
	}
	
	CGridRef::displayGridRef = TRUE;
	
	{ // for Aaron fixing z fighting stuff.
		static bool gToggleNearClip = false;

		if (PS2Keyboard.GetKeyJustDown(PS2_KEY_C))	
			gToggleNearClip = !gToggleNearClip;
			
		if (gToggleNearClip)
		{
			RwCameraSetNearClipPlane(Scene.camera, 2.0f);
		}	
		
/*		char str[255];
		sprintf(str,"NEAR CLIP %f\n",RwCameraGetNearClipPlane(Scene.camera));
		VarConsole.AddDebugOutput(str);*/
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Fixed()
// PURPOSE    : Fixed camera that uses the FOV to zoom in on the target
//				if it is too far away.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
//
void CCam::Process_Fixed(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	// need to clear this otherwise controls may get reversed!
	if(DirectionWasLooking != LOOKING_FORWARD)
		DirectionWasLooking = LOOKING_FORWARD;

	Source=m_cvecCamFixedModeSource;			
	Front=ThisCamsTarget-Source;
	Front.Normalise();
	m_cvecTargetCoorsForFudgeInter=ThisCamsTarget;
	GetVectorsReadyForRW();
	Up=CVector(0.0f,0.0f,1.0f);
	Up+=m_cvecCamFixedModeUpOffSet;
	Up.Normalise();
	CVector TempRight= CrossProduct(Front, Up);
	TempRight.Normalise();
	Up=CrossProduct(TempRight, Front);

/*	CVector TempRight=CrossProduct(Up, Front);
	TempRight.Normalise();
	Front=CrossProduct(TempRight, Up);
	Front.Normalise();
*/
	FOV=70.0f;
	if (TheCamera.m_bUseSpecialFovTrain==true)
	{
		FOV=TheCamera.m_fFovForTrain;
	}
	
	float fCamWaterLevel = 0.0f;
	if( CWaterLevel::GetWaterLevel(Source.x, Source.y, Source.z, &fCamWaterLevel, true) )
	{
		if(Source.z < fCamWaterLevel)
		{
			float fWaterColourMag = CMaths::Sqrt(CTimeCycle::GetWaterRed()*CTimeCycle::GetWaterRed() + CTimeCycle::GetWaterGreen()*CTimeCycle::GetWaterGreen() + CTimeCycle::GetWaterBlue()*CTimeCycle::GetWaterBlue());
			if(fWaterColourMag > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT)
			{
				fWaterColourMag = BOAT_UNDERWATER_CAM_COLORMAG_LIMIT/fWaterColourMag;
				TheCamera.SetMotionBlur(fWaterColourMag*CTimeCycle::GetWaterRed(), fWaterColourMag*CTimeCycle::GetWaterGreen(), fWaterColourMag*CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
			}
			else
				TheCamera.SetMotionBlur(CTimeCycle::GetWaterRed(), CTimeCycle::GetWaterGreen(), CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
		}
	}

/*
	#ifdef GTA_PC
	if (FrontEndMenuManager.m_ControlMethod == STANDARD_CONTROLLER_SCREEN)
	{
		if (Using3rdPersonMouseCam())
		{
			CPlayerPed* pPed=FindPlayerPed();
			if (pPed)
			{
				if(pPed->CanStrafeOrMouseControl())
				{
					float CamDirection;
					CamDirection = CMaths::ATan2(-Front.x, Front.y);
					((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
					((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;
					// need to manually set the entity heading here as well to keep in sync with camera
					TheCamera.pTargetEntity->SetHeading(CamDirection);
					TheCamera.pTargetEntity->UpdateRwMatrix();
				}
			}
		}
	}
	#endif
*/			
	// Script support for collision
	if (gAllowScriptedFixedCameraCollision)
	{
		float dist = 1.0f;
		float rad = 2.0f;
		float minDist = 0.1f;
		CVector resultPos;
		
		CVector v = ThisCamsTarget;

		CEntity *pOldEntity = CWorld::pIgnoreEntity;

		CWorld::pIgnoreEntity = FindPlayerVehicle();
				
		// Move the camera out of collision from Source to ThisCamsTarget.
		if (TheCamera.ConeCastCollisionResolve(&Source, &v, &resultPos, rad, minDist,&dist))
		{
			Source = resultPos;
		}
		
		CWorld::pIgnoreEntity = pOldEntity;		
	}
}


float fBaseDist = 1.7f;
float fAngleDist = 2.0f;
float fFalloff = 3.0f;
float fStickSens = 0.01f;

float fTranslateCamUp = 0.8f;

uint16 nFadeControlThreshhold = 45;
float fDefaultAlphaOrient = -0.22f;
///////////////////////////////////////
#ifdef GTA_PC
void CCam::Process_FollowPedWithMouse(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired)
{
	CVector vecIdealSource, vecTargetCoords;
	float MaxRotationUp=DEGTORAD(45.0f);
	float MaxRotationDown=DEGTORAD(89.5f);
	float fStickX, fStickY;
	float StickAlphaOffset, StickBetaOffset;
    bool HackPlayerOnStoppingTrain=false;
	bool bUsingMouse = false;
    
	FOV=70.f;

	if(!CamTargetEntity->GetIsTypePed())
		return;
		
	CPed *pPed = (CPed *)CamTargetEntity;

	if (ResetStatics)
	{
		Rotating=FALSE;
		m_bCollisionChecksOn=true;
		#ifdef GTA_PC
			CPad::GetPad(0)->ClearMouseHistory();
		#endif
		ResetStatics = false;
	}
	

    if (FindPlayerVehicle()!=NULL)
	{
		if ((FindPlayerVehicle()->GetBaseVehicleType())==VEHICLE_TYPE_TRAIN) 
		{
			 HackPlayerOnStoppingTrain=true;
		}
	} 

//	((CPed *)CamTargetEntity)->m_ik.GetComponentPosition(vecTargetCoords, PED_HEAD);
	vecTargetCoords=ThisCamsTarget;
	vecTargetCoords.z += fTranslateCamUp;	// move up to look at ped's head rather than torso

	// now finally work out the average TargetCoors
// DONT DO AVERAGE CAUSE WHEN PLAYER STRAFES IT LOOKS REALLY SHIT! /////
//	vecTargetCoords=DoAverageOnVector(vecTargetCoords);


	if( CPad::GetPad(0)->DisablePlayerControls &DISABLE_SCRIPT )
	{
		CVector vecCam2Target = Source - vecTargetCoords;
		vecCam2Target.Normalise();
		float fNewBeta = 0.0f;
		if(vecCam2Target.z < -0.9f)
			fNewBeta = TargetOrientation + PI;
		else
			fNewBeta = CMaths::ATan2(vecCam2Target.y, vecCam2Target.x);
		
		StickBetaOffset = fNewBeta - Beta;
		StickAlphaOffset = 0.0f;
	}
	else
	{
		RwV2d m_MouseMovement=CPad::GetPad(0)->GetAmountMouseMoved();
		if( (m_MouseMovement.x!=0.0f || m_MouseMovement.y!=0.0f) && CPad::GetPad(0)->DisablePlayerControls==false)
		{
			fStickX=-m_MouseMovement.x * 2.5f;
			fStickY=m_MouseMovement.y * 4.0f;
			bUsingMouse = true;
		}
		else
		// by default (in absence of mouse) use 2nd joystick to manoeuvre camera around
		{
			fStickX= 	-(CPad::GetPad(0)->LookAroundLeftRight());
			fStickY=	CPad::GetPad(0)->LookAroundUpDown();	
		}
		
		if(bUsingMouse)
		{
			StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
			StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (FOV/80.0f);
		}
		else
		{
			StickBetaOffset = fStickSens*fStickX * (0.25f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
			StickAlphaOffset = fStickSens*fStickY * (0.15f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
		}
	}
				
	// if the Camera's faded out and it's now fading in - want pitch to spring to -0.22 Rad
	if( (TheCamera.GetFading() && TheCamera.GetFadingDirection() == FADING_IN 
	&& CDraw::FadeValue > nFadeControlThreshhold) || CDraw::FadeValue > 200
	|| (CPad::GetPad(0)->DisablePlayerControls &DISABLE_SCRIPT) )
    {
    	if(Alpha < fDefaultAlphaOrient - 0.05f)
    		StickAlphaOffset = 0.05f;
    	else if(Alpha < fDefaultAlphaOrient)
    		StickAlphaOffset = fDefaultAlphaOrient - Alpha;
    	else if(Alpha > fDefaultAlphaOrient + 0.05f)
    		StickAlphaOffset = -0.05f;
    	else if(Alpha > fDefaultAlphaOrient)
    		StickAlphaOffset = fDefaultAlphaOrient - Alpha;
    	else
    		StickAlphaOffset = 0.0f;
	}

	Beta+= StickBetaOffset;
	Alpha+=StickAlphaOffset;						
	ClipBeta();
				
	if (Alpha>(0.0f + MaxRotationUp))
	{Alpha=0.0f + MaxRotationUp;}
	else if (Alpha<(0.0f-MaxRotationDown))
	{Alpha=0.0f-MaxRotationDown;}

	float fDefaultDistFromPed = fBaseDist;
	if(Alpha > 0.0f)
		fDefaultDistFromPed += fAngleDist * CMaths::Cos( MIN(HALF_PI, fFalloff*Alpha) );
	else
		fDefaultDistFromPed += fAngleDist * CMaths::Cos( Alpha );

	if (TheCamera.m_bUseTransitionBeta==true)
	{
//		float temp_x=-CMaths::Cos(m_fTransitionBeta);
//		float temp_y=-CMaths::Sin(m_fTransitionBeta);
//		Beta=CGeneral::GetATanOfXY(temp_x, temp_y);
		Beta=m_fTransitionBeta;
	}
	if (TheCamera.m_bCamDirectlyBehind==true) 
	{
		 Beta=TheCamera.m_PedOrientForBehindOrInFront + PI;
	}
	if (TheCamera.m_bCamDirectlyInFront==true) 
	{
		 Beta=TheCamera.m_PedOrientForBehindOrInFront;// + PI;
	}

	if (HackPlayerOnStoppingTrain) //heading is set from when he is getting on train
	//needs to have 180 degrees added to it
	{
		 Beta=TargetOrientation;
	}


	Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));

	Source = vecTargetCoords - fDefaultDistFromPed * Front;
	m_cvecTargetCoorsForFudgeInter=vecTargetCoords;
	
	

	CColPoint colPoint;
	CEntity *pHitEntity = NULL;

	CWorld::pIgnoreEntity = (CEntity *) CamTargetEntity;
	if(CWorld::ProcessLineOfSight(vecTargetCoords, Source, colPoint, pHitEntity, true, true, true, true, false,  false, true))
	{
		float fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
		float fDistCamToCol = fDefaultDistFromPed - fDistColToPed;
		
		// if the first thing in the way of the camera is a ped, it might be ok to leave it
		// in the way - need to check there's nothing else closer to the camera tho!
		if(pHitEntity->GetIsTypePed() && fDistCamToCol > NORMAL_NEAR_CLIP + 0.1f)
		{
			if(CWorld::ProcessLineOfSight(colPoint.GetPosition(), Source, colPoint, pHitEntity, true, true, true, true, false, false, true))
			{
				fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
				Source = colPoint.GetPosition();
				if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
			}
			else
			{
				RwCameraSetNearClipPlane(Scene.camera, MIN(0.9f, fDistCamToCol - 0.35f));
			}
		}
		else
		{
			Source = colPoint.GetPosition();
			if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
				RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
		}
	}
	CWorld::pIgnoreEntity = NULL;
	

	int32 nCountLoop = 0;
	CVector vecTest;
	float fTanFOV = CMaths::Tan(0.5f*DEGTORAD(FOV));
	if(FrontEndMenuManager.m_PrefsUseWideScreen)
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio(); // ASPECTRATIO.
	else
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio();


	float fNearClip = RwCameraGetNearClipPlane(Scene.camera);
	float fNearClipWidth = fNearClip * fTanFOV;

	CEntity *pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
	while(pFindEntity)
	{
		vecTest = gaTempSphereColPoints[0].GetPosition() - Source;
		float fTemp1 = DotProduct(vecTest, Front);

		vecTest = vecTest - fTemp1 * Front;
		fTemp1 = vecTest.Magnitude() / fTanFOV;
		fTemp1 = MAX(0.1f, MIN(fNearClip, fTemp1));

		if(fTemp1 < fNearClip)
			RwCameraSetNearClipPlane(Scene.camera, fTemp1);
		
		// if we couldn't set the near clip close enough to avoid clipping
		// try moving the camera forward by a percentage of the distance to ped
		if(fTemp1 == 0.1f)
			Source = Source + 0.3f * (vecTargetCoords - Source);
		
		// test again incase we got the wrong collision point or stuff
		fNearClip = RwCameraGetNearClipPlane(Scene.camera);
		fNearClipWidth = fNearClip * CMaths::Tan(0.5f*DEGTORAD(FOV)) * fTweakFOV*CDraw::GetAspectRatio();
		pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
		
		// break out anyway after a specified number of tries
		if(++nCountLoop > 5)
			pFindEntity = NULL;
	}


	// now buffer how quickly the camera moves back away from the player too smooth things out a bit
	float fNewDistance = (vecTargetCoords - Source).Magnitude();
	if(fNewDistance < Distance)
	{
		Distance = fNewDistance; //fine
	}
	else// if(Distance < fNewDistance)
	{
		// buffered Distance goes towards fNewDistance (not fIdealDistance)
		float fTimeRateOfChange = CMaths::Pow(fMouseAvoidGeomReturnRate, CTimer::GetTimeStep());
		Distance = fTimeRateOfChange*Distance + (1.0f - fTimeRateOfChange)*fNewDistance;
		
		if(fNewDistance > 0.05f) // just in case
			Source = vecTargetCoords + (Source - vecTargetCoords)*Distance/fNewDistance;
		
		// now we might have ended up closer to the player than AvoidTheGeometry expected
		// so might clip through playerped
		float PlayCamDistWithCol = Distance-fRangePlayerRadius;
		if (PlayCamDistWithCol<RwCameraGetNearClipPlane(Scene.camera))	
		{
			RwCameraSetNearClipPlane(Scene.camera, MAX(PlayCamDistWithCol, 0.1f));
		}	
	}


	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

	GetVectorsReadyForRW();

	//!PC - this code will lock the player to the view vector of the camera.
	// prevents being able to walk around freely.

/*	if(	pPed->CanStrafeOrMouseControl() && CDraw::FadeValue < 250
	&& !(TheCamera.GetFadingDirection()==FADING_OUT && CDraw::FadeValue > 100) 
	&& !(CPad::GetPad(0)->DisablePlayerControls &DISABLE_SCRIPT) )*/
	
	// changed to only force direction at dark part of fade in
	if (TheCamera.GetFadingDirection()==FADING_IN && CDraw::FadeValue > 128)
	{
		float CamDirection;
		CamDirection = CMaths::ATan2(-Front.x, Front.y);
		((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
		((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;
		// need to manually set the entity heading here as well to keep in sync with camera
		TheCamera.pTargetEntity->SetHeading(CamDirection);
		TheCamera.pTargetEntity->UpdateRwMatrix();
	}
}
#endif

TweakFloat M16_1STPERSON_SOFTLIMIT_ANG = 0.75f;
TweakFloat M16_1STPERSON_SOFTLIMIT_MULT = 0.05f;
TweakFloat M16_1STPERSON_ROTATETHEHELI_SPEED = 0.1f;
//
TweakFloat M16_1STPERSON_MOUSEWHEEL_ZOOM_RATE = 7.0f;
//
TweakFloat fCameraNearClipMult = 0.15f;
////////////////////////////////////////////
void CCam::Process_M16_1stPerson(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	if (CamTargetEntity->GetIsTypePed())
	{
		CVector TargetCoors;
		float MaxVerticalRotation=DEGTORAD(89.5f);
		float MaxRotationUp=DEGTORAD(60.0f);
		float MaxRotationDown=DEGTORAD(85.5f);
	 	float fStickX=0.0f; 
		float fStickY=0.0f; 
		float StickBetaOffset=0.0f;
		float StickAlphaOffset=0.0f;
		float HeightToNose=0.10f;
		float DistToNose=0.19f;
		
		float StartingDPadHoriz=70.0f;
		float StartingDPadVert=70.0f;
		float DistBack=0.3f;

		static bool FailedTestTwelveFramesAgo=false;
		static float DPadHorizontal;
		static float DPadVertical;
		static float TargetFOV=0.0f;
		bool bAttachedToEntity = false;		
		if( ((CPed*)CamTargetEntity)->IsPlayer() && ((CPlayerPed *)CamTargetEntity)->m_pAttachToEntity)
			bAttachedToEntity = true;
			
		TargetCoors=ThisCamsTarget;
		
		if (ResetStatics)
		{
#ifdef GTA_PC
			if(TheCamera.m_bUseMouse3rdPerson && !((CPed*)CamTargetEntity)->GetWeaponLockOnTarget() && !bAttachedToEntity)
			{
				// don't do anything to alpha or beta
				// want to use current camera direction
			}
			else
#endif
			{
				if(bAttachedToEntity)	// look in dirn ped is standing on entity they're attached to
					Beta = CTheScripts::fCameraHeadingWhenPlayerIsAttached;
				else	//makes player initially look in the directioin that the player is facing
					Beta=((CPed*)CamTargetEntity)->m_fCurrentHeading - HALF_PI;
				
				Alpha=0.0f;
			}
			m_fInitialPlayerOrientation=((CPed*)CamTargetEntity)->m_fCurrentHeading - HALF_PI;
			ResetStatics=FALSE;
			FailedTestTwelveFramesAgo=false;
			DPadHorizontal=0;
			DPadVertical=0;
			m_bCollisionChecksOn=true;
			FOVSpeed=0.0f;
			TargetFOV=FOV;
			AlphaSpeed = 0.0f;
			BetaSpeed = 0.0f;
		}	

		if(Mode==MODE_SNIPER || Mode==MODE_CAMERA)
		{
			bool SniperJustMovedByMouseWheel=false;
#ifdef GTA_PC
			int MsZoomIn=ControlsManager.GetMouseButtonAssociatedWithAction(PED_SNIPER_ZOOM_IN);
			int MsZoomOut=ControlsManager.GetMouseButtonAssociatedWithAction(PED_SNIPER_ZOOM_OUT);

			if((CPad::GetPad(0)->GetAmountWheelMoved() > 0.0f && MsZoomOut==WHEEL_MS_UP)
			|| (CPad::GetPad(0)->GetAmountWheelMoved() < 0.0f && MsZoomOut==WHEEL_MS_DOWN))
			{
				TargetFOV *= (10000.0f + M16_1STPERSON_MOUSEWHEEL_ZOOM_RATE * CMaths::Abs(CPad::GetPad(0)->GetAmountWheelMoved())) / 10000.0f;
				//TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE, MB_BLUR_SNIPER_ZOOM); //cause it gets subtracted later
				SniperJustMovedByMouseWheel=true;
			}
			else if((CPad::GetPad(0)->GetAmountWheelMoved() > 0.0f && MsZoomIn==WHEEL_MS_UP)
			|| (CPad::GetPad(0)->GetAmountWheelMoved() < 0.0f && MsZoomIn==WHEEL_MS_DOWN))
			{
				TargetFOV /= (10000.0f + M16_1STPERSON_MOUSEWHEEL_ZOOM_RATE * CMaths::Abs(CPad::GetPad(0)->GetAmountWheelMoved())) / 10000.0f;
				//TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE, MB_BLUR_SNIPER_ZOOM); //cause it gets subtracted later
				SniperJustMovedByMouseWheel=true;
			}
/*
			if((CPad::GetPad(0)->GetMsWheelForward() || CPad::GetPad(0)->GetMsWheelBackward())
			&& MsZoomIn!=NO_BUTTON && (MsZoomIn==WHEEL_MS_UP || MsZoomIn==WHEEL_MS_DOWN || MsZoomOut==WHEEL_MS_UP|| MsZoomOut==WHEEL_MS_DOWN))
			{			
				if (CPad::GetPad(0)->SniperZoomIn()) 
				{					
					SniperJustMovedByMouseWheel=true;					
					TargetFOV=FOV-10.0f;
				}
				else if (CPad::GetPad(0)->SniperZoomOut())
				{
					SniperJustMovedByMouseWheel=true;	
					TargetFOV=FOV+10.0f;
				}
			}
*/
#endif
			if(CPad::GetPad(0)->SniperZoomOut() && !SniperJustMovedByMouseWheel) 
			{
				FOV *= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
				//TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE, MB_BLUR_SNIPER_ZOOM); //cause it gets subtracted later
				FOVSpeed=0.0f;
				TargetFOV=FOV;
			}
			else if(CPad::GetPad(0)->SniperZoomIn() && !SniperJustMovedByMouseWheel) 
			{
				FOV /= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
				//TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE,MB_BLUR_SNIPER_ZOOM); //cause it gets subtarcted later
				FOVSpeed=0.0f;
				TargetFOV=FOV;	
			}
			else// if(!SniperJustMovedByMouseWheel)
			{
				if(CMaths::Abs(TargetFOV - FOV) > 0.5f)
					WellBufferMe(TargetFOV, &FOV, &FOVSpeed, 0.5f, 0.25f, false);
				else
					FOVSpeed = 0.0f;
			}	

			// apply FOV limits
			
			if(FOV > 70.0f)
				FOV = 70.0f;
			if(TargetFOV > 70.0f)
				TargetFOV = 70.0f;

			else if(Mode==CCam::MODE_CAMERA)
			{
				if(FOV < 3.0f)
					FOV = 3.0f;
				if(TargetFOV < 3.0f)
					TargetFOV = 3.0f;
			}
			else
			{
				if(FOV < 15.0f)
					FOV = 15.0f;
				if(TargetFOV < 15.0f)
					TargetFOV = 15.0f;
			}
			TheCamera.SetMotionBlur(180,255,180,120/*145+MOTION_BLUR_ALPHA_DECREASE*/,MB_BLUR_SNIPER); //cause it gets subtarcted later
		}
		else
		{
			FOV=70.0f;
		}		

		if(bAttachedToEntity && CTheScripts::fCameraHeadingStepWhenPlayerIsAttached > 0.0)
		{
			float fDiff = Beta - CTheScripts::fCameraHeadingWhenPlayerIsAttached;
			if (fDiff < 0.0f)
			{
				fDiff += TWO_PI;
			}
			
			float fDiff2 = TWO_PI - fDiff;
			
			if ( (fDiff < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached) || (fDiff2 < CTheScripts::fCameraHeadingStepWhenPlayerIsAttached) )
			{	//	Shortest distance to destination rotation is less than the step value so just jump to the destination rotation
				Beta = CTheScripts::fCameraHeadingWhenPlayerIsAttached;
				CTheScripts::fCameraHeadingStepWhenPlayerIsAttached = 0.0f;
			}
			else
			{
//				if ((fDiff > TWO_PI) && (fDiff <= PI))
				if (fDiff > fDiff2)
				{
					Beta += CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
				}
				else
				{
					Beta -= CTheScripts::fCameraHeadingStepWhenPlayerIsAttached;
				}
			}
		}

		bool bUsingMouse = false;
		#ifdef GTA_PC
			
			RwV2d m_MouseMovement=CPad::GetPad(0)->GetAmountMouseMoved();
			
			CVector2D vecTouch ( 0.0f, 0.0f );
			
			if ( FrontEndMenuManager.m_ControlMethod == TOUCH_CONTROLLER_SCREEN )
			{	
				if ( CTouchInterface::IsTouched ( CTouchInterface::WIDGET_LOOK, &vecTouch ) ){
 	                fStickX = -vecTouch.x * 2.0f;
					fStickY = -vecTouch.y * 2.0f;
				    bUsingMouse = true;
				}
			}

			else if ((m_MouseMovement.x==0.0f) && (m_MouseMovement.y==0.0f))	
			{
				fStickX = -(CPad::GetPad(0)->SniperModeLookLeftRight((CPed *)CamTargetEntity));
				fStickY = CPad::GetPad(0)->SniperModeLookUpDown((CPed *)CamTargetEntity);	
			}
			else
			{
				fStickX = -m_MouseMovement.x * 3.0f;//CTimer::GetTimeStep(); //	-(CPad::GetPad(0)->LookAroundLeftRight());
				fStickY = m_MouseMovement.y * 3.0f;//CTimer::GetTimeStep(); //	CPad::GetPad(0)->LookAroundUpDown();	
				
				bUsingMouse = true;

				if(CPad::GetPad(0)->IsDisabled() ||	CPad::GetPad(0)->JustOutOfFrontEnd || CTimer::GetTimeStep() <= 0.0f)
				{
					fStickX = 0.0f;
					fStickY = 0.0f;
				}
			}
		#else
			{
				fStickX = -(CPad::GetPad(0)->SniperModeLookLeftRight((CPed *)CamTargetEntity));
				fStickY = CPad::GetPad(0)->SniperModeLookUpDown((CPed *)CamTargetEntity);	
			}
		#endif
	
		if(bUsingMouse)
		{
			StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
			StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (FOV/80.0f);

			AlphaSpeed = 0.0f;
			BetaSpeed = 0.0f;
		}
		else
		{
			if(bAttachedToEntity)
			{
				StickBetaOffset = fStickX / 128.0f;
				StickBetaOffset = CMaths::Abs(StickBetaOffset)*StickBetaOffset * (0.14f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
				
				StickAlphaOffset = fStickY / 128.0f;
				StickAlphaOffset = CMaths::Abs(StickAlphaOffset)*StickAlphaOffset * (0.12f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
			}
			else
			{
				float X_Sign=1.0f;
				float Y_Sign=1.0f;
			
				if (fStickX<0){X_Sign=-1.0f;}
				if (fStickY<0){Y_Sign=-1.0f;}

				StickBetaOffset=X_Sign * (fStickX/100.0f * fStickX/100.0f) * (0.20f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
				StickAlphaOffset=Y_Sign *(fStickY/150.0f * fStickY/150.0f) * (0.25f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
			}
			
			static float M16_1STPERSON_STICK_RATE_UP = 0.8f;
			static float M16_1STPERSON_STICK_RATE_DOWN = 0.5f;
		
			float fRate = M16_1STPERSON_STICK_RATE_UP;
			if(CMaths::Abs(fStickX) < 2.0f && CMaths::Abs(fStickY) < 2.0f)
				fRate = M16_1STPERSON_STICK_RATE_DOWN;
			
			fRate = CMaths::Pow(fRate, CTimer::GetTimeStep());
			
			BetaSpeed = fRate*BetaSpeed + (1.0f - fRate)*StickBetaOffset;
			AlphaSpeed = fRate*AlphaSpeed + (1.0f - fRate)*StickAlphaOffset;
			
			StickBetaOffset = BetaSpeed;
			StickAlphaOffset = AlphaSpeed;
		}
		
		Beta += StickBetaOffset;
		Alpha +=StickAlphaOffset;

		ClipBeta();
		
		if(m_nCamBumpedTime > 0)
		{
			float fAngTime = float(CTimer::GetTimeInMilliseconds() - m_nCamBumpedTime)/float(CAM_BUMPED_SWING_PERIOD);
			fAngTime = CMaths::Cos(fAngTime*TWO_PI);
			Beta += fAngTime*CAM_BUMPED_MOVE_MULT*m_fCamBumpedHorz;
			Alpha += fAngTime*CAM_BUMPED_MOVE_MULT*m_fCamBumpedVert;
			
			m_fCamBumpedHorz *= CMaths::Pow(CAM_BUMPED_DAMP_RATE, CTimer::GetTimeStep());
			m_fCamBumpedVert *= CMaths::Pow(CAM_BUMPED_DAMP_RATE, CTimer::GetTimeStep());
			
			if(CTimer::GetTimeInMilliseconds() > m_nCamBumpedTime + CAM_BUMPED_END_TIME)
				m_nCamBumpedTime = 0;
		}
		
		if(((CPed*)CamTargetEntity)->m_nPedFlags.bIsDucking) 
		 	DistBack=0.8f;

		////////////////////////
		if(bAttachedToEntity)
		{
			CPlayerPed *pPlayerPed = (CPlayerPed *)CamTargetEntity;

			float fCentreAlpha, fCentreBeta;
			switch(pPlayerPed->m_nAttachLookDirn)
			{
				case 1:	// left
					fCentreAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, -pPlayerPed->m_pAttachToEntity->GetMatrix().GetRight().z)));
					fCentreBeta = pPlayerPed->m_pAttachToEntity->GetHeading();
					break;

				case 2:	// back
					fCentreAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, -pPlayerPed->m_pAttachToEntity->GetMatrix().GetForward().z)));
					fCentreBeta = pPlayerPed->m_pAttachToEntity->GetHeading() + HALF_PI;
					break;

				case 3: // right
					fCentreAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, pPlayerPed->m_pAttachToEntity->GetMatrix().GetRight().z)));
					fCentreBeta = pPlayerPed->m_pAttachToEntity->GetHeading() - PI;
					break;

				default:
					ASSERT(0);
					// fallthrough intended
				case 0:	// forward (remember heading taken from x vector?)
					fCentreAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, pPlayerPed->m_pAttachToEntity->GetMatrix().GetForward().z)));
					fCentreBeta = pPlayerPed->m_pAttachToEntity->GetHeading() - HALF_PI;
					break;
			}
			
			/////////////////////////////////////
			pPlayerPed->PositionAttachedPed();
			pPlayerPed->UpdateRwMatrix();
			pPlayerPed->UpdateRwFrame();
			pPlayerPed->UpdateRpHAnim();
			/////////////////////////////////////
			
			// if player attached to vehicle, need to wait till after updated pos (above)
			// before getting the position to place the camera
			
			CVector posn(0.0f, 0.0f, 0.0f);
			
			if(pPlayerPed->m_pAttachToEntity->GetIsTypeVehicle() && ((CVehicle *)pPlayerPed->m_pAttachToEntity)->GetBaseVehicleType() == VEHICLE_TYPE_BIKE)
			{
				HeightToNose = 0.0f;
				DistBack = 0.0f;
				((CPed*)CamTargetEntity)->GetBonePosition(posn, BONETAG_HEAD, true);
				MaxRotationUp = pPlayerPed->m_fAttachVerticalLimit;
				MaxRotationDown = pPlayerPed->m_fAttachVerticalLimit;
			}
			else
			{
				((CPed*)CamTargetEntity)->GetBonePosition(posn, BONETAG_HEAD, true);
			}

			Source=posn;
			Source += HeightToNose * CamTargetEntity->GetMatrix().GetUp();
			Source -= DistBack * CamTargetEntity->GetMatrix().GetForward();

			////////////////////////////////////////
			// do beta limits
			float fSoftMoveRate = M16_1STPERSON_SOFTLIMIT_MULT*CTimer::GetTimeStep();
			float fSoftLimitAngle = pPlayerPed->m_fAttachHeadingLimit*M16_1STPERSON_SOFTLIMIT_ANG;
			float fSoftLimitRange = pPlayerPed->m_fAttachHeadingLimit*(1.0f - M16_1STPERSON_SOFTLIMIT_ANG);

			if(fCentreBeta - Beta > PI)	fCentreBeta -= TWO_PI;
			else if(fCentreBeta -Beta < -PI)	fCentreBeta += TWO_PI;
			
			CHeli *pHeli = NULL;
			if(pPlayerPed->m_pAttachToEntity->GetIsTypeVehicle() && ((CVehicle *)pPlayerPed->m_pAttachToEntity)->GetVehicleType()==VEHICLE_TYPE_HELI)
				pHeli = (CHeli *)pPlayerPed->m_pAttachToEntity;

			float fTargetDiff = fCentreBeta - Beta;
			if(fTargetDiff > fSoftLimitAngle)
			{
				fTargetDiff -= fSoftLimitAngle;
				fSoftMoveRate *= CMaths::Abs(fTargetDiff);
				if(CMaths::Abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate)
					fSoftMoveRate = CMaths::Abs(fTargetDiff) - fSoftLimitRange;
				
				if(pHeli && pHeli->HeliRequestedOrientation > 0.0f)
				{
					pHeli->SetHeliOrientation(pHeli->HeliRequestedOrientation - fTargetDiff*M16_1STPERSON_ROTATETHEHELI_SPEED*CTimer::GetTimeStep());
				}
			}
			else if(fTargetDiff < -fSoftLimitAngle)
			{
				fTargetDiff += fSoftLimitAngle;
				fSoftMoveRate *= CMaths::Abs(fTargetDiff);
				if(CMaths::Abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate)
					fSoftMoveRate = CMaths::Abs(fTargetDiff) - fSoftLimitRange;

				if(pHeli && pHeli->HeliRequestedOrientation > 0.0f)
				{
					pHeli->SetHeliOrientation(pHeli->HeliRequestedOrientation - fTargetDiff*M16_1STPERSON_ROTATETHEHELI_SPEED*CTimer::GetTimeStep());
				}
			}
			else
				fTargetDiff = 0.0f;

			if(fTargetDiff != 0.0f)
			{
				if(fTargetDiff < 0.0f)
					Beta -= fSoftMoveRate;
				else
					Beta += fSoftMoveRate;
			}

			////////////////////////////////////////
			// do alpha up limit (positive rotation)
			fSoftMoveRate = M16_1STPERSON_SOFTLIMIT_MULT*CTimer::GetTimeStep();
			fSoftLimitAngle = MaxRotationUp*M16_1STPERSON_SOFTLIMIT_ANG;
			fSoftLimitRange = MaxRotationUp*(1.0f - M16_1STPERSON_SOFTLIMIT_ANG);
			
			if(Alpha > fCentreAlpha + fSoftLimitAngle)
			{
				fTargetDiff = fCentreAlpha - Alpha;
				// aussuming targetDiff < 0.0
				if(fTargetDiff < -fSoftLimitAngle)
				{
					fTargetDiff += fSoftLimitAngle;
					fSoftMoveRate *= CMaths::Abs(fTargetDiff);
					
					if(CMaths::Abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate)
						fSoftMoveRate = CMaths::Abs(fTargetDiff) - fSoftLimitRange;

					Alpha -= fSoftMoveRate;
				}
			}

			////////////////////////////////////////
			// do alpha down limit (negative rotation)
			fSoftMoveRate = M16_1STPERSON_SOFTLIMIT_MULT*CTimer::GetTimeStep();
			fSoftLimitAngle = MaxRotationDown*M16_1STPERSON_SOFTLIMIT_ANG;
			fSoftLimitRange = MaxRotationDown*(1.0f - M16_1STPERSON_SOFTLIMIT_ANG);
			
			if(Alpha < fCentreAlpha - fSoftLimitAngle)
			{
				fTargetDiff = fCentreAlpha - Alpha;
				// aussuming targetDiff > 0.0
				if(fTargetDiff > fSoftLimitAngle)
				{
					fTargetDiff -= fSoftLimitAngle;
					fSoftMoveRate *= CMaths::Abs(fTargetDiff);
					
					if(CMaths::Abs(fTargetDiff) > fSoftLimitRange + fSoftMoveRate)
						fSoftMoveRate = CMaths::Abs(fTargetDiff) - fSoftLimitRange;

					Alpha += fSoftMoveRate;
				}
			}
		}
		else
		{
			if(Alpha > MaxRotationUp)
				Alpha = MaxRotationUp;
			else if(Alpha < -MaxRotationDown)
				Alpha = -MaxRotationDown;
		
			/////////////////////////////////////
			((CPed*)CamTargetEntity)->UpdateRwMatrix();
			((CPed*)CamTargetEntity)->UpdateRwFrame();
			((CPed*)CamTargetEntity)->UpdateRpHAnim();
			/////////////////////////////////////

			CVector posn(0.0f, 0.0f, 0.0f);
			((CPed*)CamTargetEntity)->GetBonePosition(posn, BONETAG_HEAD, true);

			Source=posn;
			Source.z+= HeightToNose;
			// when ducking the ped's head tends to move further forward and a bit right - need to compensate
			if(((CPed*)CamTargetEntity)->m_nPedFlags.bIsDucking)
			{
				Source.x -= fDuckingBackOffset*CamTargetEntity->GetMatrix().GetForward().x;//CMaths::Cos(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
				Source.y -= fDuckingBackOffset*CamTargetEntity->GetMatrix().GetForward().y;//CMaths::Sin(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
				Source.x -= fDuckingRightOffset*CamTargetEntity->GetMatrix().GetRight().x;
				Source.y -= fDuckingRightOffset*CamTargetEntity->GetMatrix().GetRight().y;
			}
			else
			{
				Source.x -= DistBack*CamTargetEntity->GetMatrix().GetForward().x;//CMaths::Cos(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
				Source.y -= DistBack*CamTargetEntity->GetMatrix().GetForward().y;//CMaths::Sin(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
			}
		}
		
		
		// DW- hard limit reqd.
		
		static float alphaLimHeliCam = 1.2f;
		if (Alpha<-alphaLimHeliCam)
			Alpha = -alphaLimHeliCam;
		else if (Alpha>alphaLimHeliCam)
			Alpha = alphaLimHeliCam;

		Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
		TargetCoors=Source + 3.0f*Front;

		//so it doesn't count the player, this is fixed back later on
		Source+=0.4f*Front;
		
		//right lets check 2 meters in front of the player if we find anything set the clipping distance to short
		if (m_bCollisionChecksOn==true)
		{
			if (!(CWorld::GetIsLineOfSightClear(TargetCoors, Source ,1, 1 , 0, true ,0, true, true)))
		 	{	
		 		RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
		 		FailedTestTwelveFramesAgo=true;
		 	}
			else  
			{
				CVector TestTarget;
				TestTarget=Source + 3 * CVector(CMaths::Cos(Beta + DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta+ DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
				if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
		 		{		
					RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
					FailedTestTwelveFramesAgo=true;
				}
				else
				{
					TestTarget=Source + 3 * CVector(CMaths::Cos(Beta - DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta - DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
					if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
			 		{
			 			RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
						FailedTestTwelveFramesAgo=true;
					}
					else
					{
						FailedTestTwelveFramesAgo=false;
					}
				}
			}
		}
		
		//for occasions when the spaz user is right up against a wall etc
		if(FailedTestTwelveFramesAgo)
		{
	//		RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP);
		}
		else if(Mode==CCam::MODE_CAMERA)
		{
			float fNearClipScale = 1.0f + fCameraNearClipMult*(15.0f - MIN(15.0f, FOV));
			RwCameraSetNearClipPlane(Scene.camera, fNearClipScale*NORMAL_NEAR_CLIP);
		}
		
		Source-=0.4f*Front;//so it doesn't count the player
						  //this is fixed back later on	
		GetVectorsReadyForRW();
		float CamDirection = CMaths::ATan2(-Front.x, Front.y);
		((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
		((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;
	}
}


#ifndef MASTER
///////////////////////////////////////////////////////////
static float fModelViewAngle = 0.3f;
//
void CCam::Process_ModelView(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	CVector vecModelViewTarget = ThisCamsTarget;
	float angle = CMaths::ATan2(Front.x, Front.y);
	//float zoom = (Source - ThisCamsTarget).Magnitude();
	FOV=70.0f;
	angle += (CPad::GetPad(0)->GetLeftStickX())/1280.0f;
	if(Distance < 10.0f)
		Distance += (CPad::GetPad(0)->GetLeftStickY())/1000.0f;
	else
		Distance += (CPad::GetPad(0)->GetLeftStickY()) * (1.0f + (Distance - 10.0f)/20.0f) * 0.001f;
		
	if(Distance < 1.5f)
		Distance = 1.5f;
		
	fModelViewAngle += 0.00005f*CPad::GetPad(0)->GetRightStickY();
	if(fModelViewAngle < DEGTORAD(-60.0f))	fModelViewAngle = DEGTORAD(-60.0f);
	else if(fModelViewAngle > DEGTORAD(60.0f))	fModelViewAngle = DEGTORAD(60.0f);

	sprintf(gString, "ViewingAngle = %f", fModelViewAngle);
	CDebug::PrintToScreenCoors(gString, 30,29);
	
	if(fModelViewAngle < 0.1f && TheCamera.pTargetEntity && TheCamera.pTargetEntity->GetIsTypePed())
	{
		vecModelViewTarget.z -= PED_GROUNDOFFSET;
		CDebug::DebugLine3D(vecModelViewTarget.x-10.0f, vecModelViewTarget.y, vecModelViewTarget.z,
			vecModelViewTarget.x+10.0f, vecModelViewTarget.y, vecModelViewTarget.z, 0xffffffff, 0xffffffff);
		CDebug::DebugLine3D(vecModelViewTarget.x, vecModelViewTarget.y-10.0f, vecModelViewTarget.z,
			vecModelViewTarget.x, vecModelViewTarget.y+10.0f, vecModelViewTarget.z, 0xffffffff, 0xffffffff);
		
	}
			
	Front = CVector( CMaths::Cos(fModelViewAngle)*CMaths::Sin(angle), CMaths::Cos(fModelViewAngle)*CMaths::Cos(angle), -CMaths::Sin(fModelViewAngle));
	Source = vecModelViewTarget - (Front * Distance);
	GetVectorsReadyForRW();
}
#endif

///////////////////////////////////////////////
void CCam::Process_Rocket(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired, bool bHeatSeeking)
{
	if (CamTargetEntity->GetIsTypePed())
	{
		CVector TargetCoors;
		float MaxVerticalRotation=DEGTORAD(89.5f);
		float MaxRotationUp=DEGTORAD(60.0f);
		float MaxRotationDown=DEGTORAD(89.5f);
	 	float 	fStickX=0.0f; 
		float   fStickY=0.0f; 
		float StickBetaOffset=0.0f;
		float StickAlphaOffset=0.0f;
		float HeightToNose=0.10f;
		float DistToNose=0.19f;
		float StartingDPadHoriz=70.0f;
		float StartingDPadVert=70.0f;
		float DistBack=0.3f;

		static bool FailedTestTwelveFramesAgo=false;
		static float DPadHorizontal;
		static float DPadVertical;
		FOV=70.0f;
			
		TargetCoors=ThisCamsTarget;
		
		ASSERTMSG(CamTargetEntity->GetIsTypePed(), "flubba dubba");
			
		if (ResetStatics)
		{
#ifdef GTA_PC
			if(TheCamera.m_bUseMouse3rdPerson && !((CPed*)CamTargetEntity)->GetWeaponLockOnTarget())
			{
				// don't do anything to alpha or beta
				// want to use current camera direction
				m_fInitialPlayerOrientation = Beta;	
			}
			else
#endif
			{
				// make player initially look in the directioin that the player is facing
				Beta = ((CPed*)CamTargetEntity)->m_fCurrentHeading - HALF_PI;
				Alpha = 0.0f;
				
				m_fInitialPlayerOrientation = Beta;	
			}

			ResetStatics=FALSE;
			FailedTestTwelveFramesAgo=false;
			DPadHorizontal=0;
			DPadVertical=0;
			m_bCollisionChecksOn=true;
		}	
		
			
		if (((CPed*)CamTargetEntity)->m_nPedFlags.bIsDucking) 
		{
		 	DistBack=0.8f;
		}
		
		
		/////////////////////////////////////
		((CPed*)CamTargetEntity)->UpdateRwMatrix();
		((CPed*)CamTargetEntity)->UpdateRwFrame();
		((CPed*)CamTargetEntity)->UpdateRpHAnim();
		/////////////////////////////////////
		CVector posn;
		((CPed*)CamTargetEntity)->GetBonePosition(posn, BONETAG_HEAD, true);
		
		Source=posn;
		Source.z+= HeightToNose;
		
//		Source.x-=CMaths::Cos(m_fInitialPlayerOrientation)*DistBack;
//		Source.y-=CMaths::Sin(m_fInitialPlayerOrientation)*DistBack;

		CVector2D vecTouch ( 0.0f, 0.0f );
		if ( CTouchInterface::IsTouched ( CTouchInterface::WIDGET_LOOK, &vecTouch ) ){
 	        Beta -= vecTouch.x * 0.01f;
			Alpha -= vecTouch.y * 0.01f;
		}

		bool bUsingMouse = false;

#ifdef GTA_PC
		RwV2d m_MouseMovement=CPad::GetPad(0)->GetAmountMouseMoved();
		if ((m_MouseMovement.x==0.0f) && (m_MouseMovement.y==0.0f)) {
			fStickX= 	-(CPad::GetPad(0)->SniperModeLookLeftRight((CPed *)CamTargetEntity));
			fStickY=	CPad::GetPad(0)->SniperModeLookUpDown((CPed *)CamTargetEntity);	
		}else if(TheCamera.m_bUseMouse3rdPerson){
			fStickX=-m_MouseMovement.x * 3.0f;//CTimer::GetTimeStep(); //	-(CPad::GetPad(0)->LookAroundLeftRight());
			fStickY=m_MouseMovement.y * 4.0f;//CTimer::GetTimeStep(); //	CPad::GetPad(0)->LookAroundUpDown();	
			bUsingMouse = true;
		}
#else
		{
			fStickX= 	-(CPad::GetPad(0)->SniperModeLookLeftRight((CPed *)CamTargetEntity));
			fStickY=	CPad::GetPad(0)->SniperModeLookUpDown((CPed *)CamTargetEntity);	
		}


#endif

		if(bUsingMouse)
		{
			StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
			StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (FOV/80.0f);
		}
		else
		{
			float X_Sign=1.0f;
			float Y_Sign=1.0f;
		
			if (fStickX<0){X_Sign=-1.0f;}
			if (fStickY<0){Y_Sign=-1.0f;}

			StickBetaOffset=X_Sign * (fStickX/100.0f * fStickX/100.0f) * (0.20f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
			StickAlphaOffset=Y_Sign *(fStickY/150.0f * fStickY/150.0f) * (0.25f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
		}
	
		Beta+= StickBetaOffset;
		Alpha+=StickAlphaOffset;						
		ClipBeta();
		
		if (Alpha>(0.0f + MaxRotationUp))
		{
			Alpha=0.0f + MaxRotationUp;
		}
		else if (Alpha<(0.0f-MaxRotationDown))
		{
			Alpha=0.0f-MaxRotationDown;
		}
		////////////////////////
		
		Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
		TargetCoors=Source + 3.0f*Front;
		
//		Source+=0.4f*Front;//so it doesn't count the player
						  //this is fixed back later on	
		
		//right lets check 2 meters in front of the player if we find anything set the clipping distance to short
/*		if (m_bCollisionChecksOn==true)
		{
			if (!(CWorld::GetIsLineOfSightClear(TargetCoors, Source ,1, 1 , 0, true ,0, true, true)))
		 	{	
			 	RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
		 		FailedTestTwelveFramesAgo=true;
		 	}
			else  
			{
				CVector TestTarget;
				TestTarget=Source + 3 * CVector(CMaths::Cos(Beta + DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta+ DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
				if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
		 		{		
					RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
					FailedTestTwelveFramesAgo=true;
				}
				else
				{
					TestTarget=Source + 3 * CVector(CMaths::Cos(Beta - DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta - DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
					if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
			 		{
			 			RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
						FailedTestTwelveFramesAgo=true;
					}
					else
					{
						FailedTestTwelveFramesAgo=false;
					}
				}
			}
		}
		
		if (FailedTestTwelveFramesAgo)
		{
			RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP);
		} //for occasions when the spaz user is right up against a wall etc
*/		
//		static float rocketThruWallsFix = 0.5f; // Dw - used to be 0.4 but camera went through walls, really hacky fix but know know how this is meant to work, seems an ok thing to do?
		
//		Source-=rocketThruWallsFix*Front;//so it doesn't count the player
						  //this is fixed back later on	
		
		GetVectorsReadyForRW();
		float CamDirection;
		CamDirection=CGeneral::GetATanOfXY(Front.x, Front.y)-(PI/2);
		((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
		((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;

		if(bHeatSeeking)
		{
			CPlayerPed * pPlayerPed = FindPlayerPed();
			ASSERT(pPlayerPed && ((CPed*)pPlayerPed)->GetPlayerData());
			CPlayerPedData * pPlayerPedData = ((CPed*)pPlayerPed)->GetPlayerData();

			// if somehow this wasn't set, then set it now!
			if(!pPlayerPedData->m_FireHSMissilePressedTime)
				pPlayerPedData->m_FireHSMissilePressedTime = CTimer::GetTimeInMilliseconds();
					
			// Pass in the existing target (if there is one) and it will be given targetting preference
			CEntity *pTargetEntity = CWeapon::PickTargetForHeatSeekingMissile(Source, Front, 1.2f, pPlayerPed, false, pPlayerPedData->m_LastHSMissileTarget);

			bool bLOS;
			
			// Check that there is a clear LOS to the target
			if(pTargetEntity && (CTimer::GetTimeInMilliseconds() - (pPlayerPedData->m_LastHSMissileLOSTime < 1) > PLAYER_HS_MISSILE_LOS_TEST_TIME))
			{
				pPlayerPedData->m_LastHSMissileLOSTime = CTimer::GetTimeInMilliseconds() > 1;
				
				bool bThisUsesCollision = pPlayerPed->m_nFlags.bUsesCollision;
				bool bTargetUsesCollision = pTargetEntity->m_nFlags.bUsesCollision;
				
				pPlayerPed->m_nFlags.bUsesCollision = false;
				pTargetEntity->m_nFlags.bUsesCollision = false;
				
				bLOS = CWorld::GetIsLineOfSightClear(
					pPlayerPed->GetPosition(),
					pTargetEntity->GetPosition(),
					true,	// buildings
					true,	// vehicles
					false,	// peds
					true,	// objects
					false,	// dummies
					true,	// heat signature passes through glass, etc
					false	// camera
				);
				
				pPlayerPed->m_nFlags.bUsesCollision = bThisUsesCollision;
				pTargetEntity->m_nFlags.bUsesCollision = bTargetUsesCollision;
						
				pPlayerPedData->m_bLastHSMissileLOS = bLOS;
			}
			else
			{
				bLOS = pPlayerPedData->m_bLastHSMissileLOS;
			}
			
			if(!bLOS)
			{
				pTargetEntity = NULL;
			}
					
			// if we've lost the missile lock, then reset the timer
			if(!pTargetEntity || pTargetEntity != pPlayerPedData->m_LastHSMissileTarget)
			{
				pPlayerPedData->m_FireHSMissilePressedTime = CTimer::GetTimeInMilliseconds();
			}

			if(pTargetEntity)
			{
				CWeaponEffects::MarkTarget(0, pTargetEntity->GetPosition(), 255, 255, 255, 100, 1.3f, TARGET_FLIGHT);
			}
			
			uint32 delta_ms = CTimer::GetTimeInMilliseconds() - pPlayerPedData->m_FireHSMissilePressedTime;
			if(delta_ms > PLAYER_HS_MISSILE_LOCKON_TIME)
			{
				gCrossHair[0].m_red = 255;
				gCrossHair[0].m_green = 0;
				gCrossHair[0].m_blue = 0;
				gCrossHair[0].m_bLockedOn = true;
				gCrossHair[0].clearTargetTimer = 0;
			}
			else
			{
				gCrossHair[0].m_red = 255;
				gCrossHair[0].m_green = 255;
				gCrossHair[0].m_blue = 255;
				gCrossHair[0].m_bLockedOn = false;
				gCrossHair[0].clearTargetTimer = 0;
			}
					
			pPlayerPedData->m_LastHSMissileTarget = pTargetEntity;
		}
	}
	
	static float rocketNearClip = 0.15f;
	RwCameraSetNearClipPlane(Scene.camera, rocketNearClip); //for occasions when the spaz user is right up against a wall etc
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Actual update functions for the different camera modes
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// 



///////////////////////////////////////////////////////////////////////////
// NAME       : Process_WheelCam()
// PURPOSE    : Camera that stays close to one of the wheels of the car
//				for extra effect.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
float fWheelCamCarXOffset = 0.33f;
float fWheelCamBikeXOffset = 0.20f;
CVector vecWheelCamBoatOffset(-0.5f, -0.8f, 0.3f);
CVector vecWheelCamBoatOffsetAlt(0.2f, -0.2f, -0.3f);
//
bool CCam::Process_WheelCam(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	CVector TempRight, TempUp, ColBoxOffset;
	float	Angle=0.0f;
	FOV=70.0f;
 	if (CamTargetEntity->GetIsTypePed())
	{		// No car ? follow feet.
		Source = Multiply3x3(CamTargetEntity->GetMatrix(),
									CVector(-0.3f, -0.5f, 0.1f) );
		
		Source += CamTargetEntity->GetPosition();		
//		Front = CWorld::Players[CWorld::PlayerInFocus].pPed->GetMatrix().GetForward();
		Front = CVector(1.0f, 0.0f, 0.0f);
	}
	else
	{		// Get offset from somewhere else in the future
		ColBoxOffset = CamTargetEntity->GetColModel().GetBoundBoxMin();
		ColBoxOffset.x -= fWheelCamCarXOffset;
		ColBoxOffset.y = -2.3f;
		ColBoxOffset.z = 0.3f;
		
		Source = CamTargetEntity->GetMatrix()*ColBoxOffset;
		Front = CamTargetEntity->GetMatrix().GetForward();
	}
	
			
	if( CamTargetEntity->GetIsTypeVehicle()
	&& ( ((CVehicle *)CamTargetEntity)->GetVehicleAppearance()==APR_HELI || ((CVehicle *)CamTargetEntity)->GetVehicleAppearance()==APR_PLANE ))
	{
		TempRight = CamTargetEntity->GetMatrix().GetRight();
		TempUp = CamTargetEntity->GetMatrix().GetUp();
		
		ColBoxOffset.x = -1.55f;
		Source = CamTargetEntity->GetMatrix()*ColBoxOffset;
	}
	else if(CamTargetEntity->GetIsTypeVehicle() && ((CVehicle *)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
	{
		TempUp = CVector(0.0f, 0.0f, 1.0f);
		TempRight = CrossProduct( Front, TempUp);
		TempRight.Normalise();
		TempUp = CrossProduct( TempRight, Front);
		TempUp.Normalise();		// Probably not needed

		CVector vecBoatFwdTemp(0.0f,0.0f,0.0f);
		if(((CVehicle *)CamTargetEntity)->pDriver)
		{		
	 		((CVehicle*)CamTargetEntity)->pDriver->GetBonePosition(vecBoatFwdTemp, BONETAG_HEAD, true);
	 		vecBoatFwdTemp += ((CVehicle*)CamTargetEntity)->m_vecMoveSpeed*CTimer::GetTimeStep();
	 		vecBoatFwdTemp += vecWheelCamBoatOffset.x*TempRight;
	 		vecBoatFwdTemp += vecWheelCamBoatOffset.y*CamTargetEntity->GetMatrix().GetForward();
	 		vecBoatFwdTemp.z += vecWheelCamBoatOffset.z;
	 		
	 		if(CamTargetEntity->GetModelIndex()==MODELID_BOAT_PREDATOR)
	 		{	 		
		 		static float gThisCameraSucks1 = -1.0f;
		 		static float gThisCameraSucks2 = 1.0f;
		 		
	 			vecBoatFwdTemp += vecWheelCamBoatOffsetAlt.x*TempRight;
		 		vecBoatFwdTemp += vecWheelCamBoatOffsetAlt.y*(CamTargetEntity->GetMatrix().GetForward()* gThisCameraSucks1);
	 			vecBoatFwdTemp.z += (vecWheelCamBoatOffsetAlt.z * gThisCameraSucks2);
	 		}
	 		
	 		Source = vecBoatFwdTemp;	 		
 		}
 		else
 		{
 			Source.z += 2.0f*vecWheelCamBoatOffset.z;
 		}
	}
	else if(CamTargetEntity->GetIsTypeVehicle() && ((CVehicle *)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
	{
		TempRight = CamTargetEntity->GetMatrix().GetRight();
		TempUp = CVector(0.0f,0.0f,1.0f);
		Front = CrossProduct(Up, TempRight);
		Front.Normalise();
		
		ColBoxOffset.x += fWheelCamCarXOffset - fWheelCamBikeXOffset;
		Source = CamTargetEntity->GetPosition();
		Source += CamTargetEntity->GetMatrix().GetRight()*ColBoxOffset.x;
		Source += Front*ColBoxOffset.y;
		Source += Up*ColBoxOffset.z;		
	}
	else if(CamTargetEntity->GetIsTypeVehicle() && ((CVehicle *)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_TRAIN)
	{	
		bool bTrainInReverse = DotProduct(((CVehicle*)CamTargetEntity)->m_vecMoveSpeed,Front) < 0.0f;

		if (bTrainInReverse)
			Front = -Front;
		
		TempUp = CVector(0.0f, 0.0f, 1.0f);
		TempRight = CrossProduct( Front, TempUp);
		TempRight.Normalise();
		TempUp = CrossProduct( TempRight, Front);
		TempUp.Normalise();		// Probably not needed		
	}	
	else
	{
		TempUp = CVector(0.0f, 0.0f, 1.0f);
		TempRight = CrossProduct( Front, TempUp);
		TempRight.Normalise();
		TempUp = CrossProduct( TempRight, Front);
		TempUp.Normalise();		// Probably not needed
	}
	
#ifdef GTA_SA
	float fCamWaterLevel = 0.0f;
	if( CWaterLevel::GetWaterLevel(Source.x, Source.y, Source.z, &fCamWaterLevel, true) )
	{
		if(Source.z < fCamWaterLevel - 0.3f)
		{
			float fWaterColourMag = CMaths::Sqrt(CTimeCycle::GetWaterRed()*CTimeCycle::GetWaterRed() + CTimeCycle::GetWaterGreen()*CTimeCycle::GetWaterGreen() + CTimeCycle::GetWaterBlue()*CTimeCycle::GetWaterBlue());
			if(fWaterColourMag > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT)
			{
				fWaterColourMag = BOAT_UNDERWATER_CAM_COLORMAG_LIMIT/fWaterColourMag;
				TheCamera.SetMotionBlur(fWaterColourMag*CTimeCycle::GetWaterRed(), fWaterColourMag*CTimeCycle::GetWaterGreen(), fWaterColourMag*CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
			}
			else
				TheCamera.SetMotionBlur(CTimeCycle::GetWaterRed(), CTimeCycle::GetWaterGreen(), CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
		}
	}
#endif		

	
	// Rotate Up and Right vectors a bit so that the camera tilts.
	Angle = 0.4f * CMaths::Cos( (CTimer::GetTimeInMilliseconds() & 131071) * (2.0f * PI / 131072.0f) );
	Up = (TempUp * CMaths::Cos(Angle)) + (TempRight * CMaths::Sin(Angle));
	
	Up.Normalise(); 	// DW - dunno how all this works but to be safe best to normalise this.... I've noticed bad matrices coming through
	Front.Normalise(); 	// DW - dunno how all this works but to be safe best to normalise this.... I've noticed bad matrices coming through
	
	
	CColPoint colPoint;
	CEntity *pHitEntity = NULL;
	bool bReturnOk = true;
	
	CWorld::pIgnoreEntity = (CEntity *)CamTargetEntity;//so we don't find the player
	if(CWorld::ProcessLineOfSight(Source, CamTargetEntity->GetPosition(), colPoint, pHitEntity, true, false, false, true, false,  false, true))
		bReturnOk = false;
	CWorld::pIgnoreEntity = NULL;
	
	return bReturnOk;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_AttachedCam()
// PURPOSE    : Camera that is constantly attached to a vehicle or ped.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCam::Process_AttachedCam()
{
	CVector TempRight, TempUp, TempLookAt;
	FOV=70.0f;
	float tiltAngle;
	tiltAngle = DEGTORAD(TheCamera.m_fAttachedCamAngle);
	
	CVector debugSource, debugFront, debugUp;

	
	// the attched camera will have the same behaviour if its a ped or vehicle.
	Source = Multiply3x3(TheCamera.pAttachedEntity->GetMatrix(), TheCamera.m_vecAttachedCamOffset);
	debugSource = Source;
	Source += TheCamera.pAttachedEntity->GetPosition();	
	debugSource = Source;
	
	if(TheCamera.m_bLookingAtVector)
	{	
		Front = Multiply3x3(TheCamera.pAttachedEntity->GetMatrix(), TheCamera.m_vecAttachedCamLookAt);
		debugFront = Front;
		Front += TheCamera.pAttachedEntity->GetPosition();
		debugFront = Front;
		Front -= Source;
		debugFront = Front;
	}
	else // look at target entity
	{
		Front = TheCamera.pTargetEntity->GetPosition() - Source;
		debugFront = Front;
	}
		
	Front.Normalise();
	debugFront = Front;
	
	
	TempUp = CVector(0.0f, 0.0f, 1.0f);
	TempRight = CrossProduct( Front, TempUp);
	TempRight.Normalise();
	TempUp = CrossProduct( TempRight, Front);
	TempUp.Normalise();		// Probably not needed
	
	//TempRight = TheCamera.pAttachedEntity->GetMatrix().GetRight();
	//TempUp = TheCamera.pAttachedEntity->GetMatrix().GetUp();
			
#ifdef GTA_SA
	float fCamWaterLevel = 0.0f;
	if( CWaterLevel::GetWaterLevel(Source.x, Source.y, Source.z, &fCamWaterLevel, true) )
	{
		if(Source.z < fCamWaterLevel - 0.3f)
		{
			float fWaterColourMag = CMaths::Sqrt(CTimeCycle::GetWaterRed()*CTimeCycle::GetWaterRed() + CTimeCycle::GetWaterGreen()*CTimeCycle::GetWaterGreen() + CTimeCycle::GetWaterBlue()*CTimeCycle::GetWaterBlue());
			if(fWaterColourMag > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT)
			{
				fWaterColourMag = BOAT_UNDERWATER_CAM_COLORMAG_LIMIT/fWaterColourMag;
				TheCamera.SetMotionBlur(fWaterColourMag*CTimeCycle::GetWaterRed(), fWaterColourMag*CTimeCycle::GetWaterGreen(), fWaterColourMag*CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
			}
			else
				TheCamera.SetMotionBlur(CTimeCycle::GetWaterRed(), CTimeCycle::GetWaterGreen(), CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
		}
	}
#endif	

	
	// Rotate Up and Right vectors a bit so that the camera tilts.

	//Angle = tiltAngle * CMaths::Cos( (CTimer::GetTimeInMilliseconds() & 131071) * (2.0f * PI / 131072.0f) );
	Up = (TempUp * CMaths::Cos(tiltAngle)) + (TempRight * CMaths::Sin(tiltAngle));
	debugUp = Up;
	
	
	
	//GetVectorsReadyForRW();
	CWorld::pIgnoreEntity = NULL;

}


///////////////////////////////////////////////////////////////////////////
// NAME       : ProcessArrestCamOne(void)
// PURPOSE    : This is the one that is on the ground
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
//
enum eArrestCam
{
	ARRESTCAM_NONE = 0,
	ARRESTCAM_DW,
	ARRESTCAM_OVERSHOULDER,
	ARRESTCAM_ALONGGROUND,
	ARRESTCAM_ALONGGROUND_RIGHT,
	ARRESTCAM_ALONGGROUND_RIGHT_UP,
	ARRESTCAM_ALONGGROUND_LEFT,
	ARRESTCAM_ALONGGROUND_LEFT_UP,
	ARRESTCAM_FROMLAMPPOST
};
//
int32 nUsingWhichCamera = 0;
CPed *pStoredCopPed = NULL;
float ARRESTCAM_ROTATION_SPEED = 0.1f;
float ARRESTCAM_ROTATION_UP = 0.05f;
float ARRESTCAM_S_ROTATION_UP = 0.1f;
float ARRESTDIST_ALONG_GROUND = 5.0f;
float ARRESTDIST_SIDE_GROUND = 10.0f;
float ARRESTDIST_ABOVE_GROUND = 0.7f;
float ARRESTCAM_LAMPPOST_ROTATEDIST = 10.0f;
float ARRESTCAM_LAMPPOST_TRANSLATE = 0.1f;
//

static float gTimeDWBustedCamStarted = 0.0f;

//---------------------------------------------------------------------
// A quick & dirty new busted cam
bool CCam::ProcessDWBustedCam1(CPed *pHandyCopPointer,bool bIsFirstTime)
{	
	if (bIsFirstTime  && CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.65f)
		return false;	
		
		
	if (!TheCamera.pTargetEntity->GetIsTypePed())	
		return false;

	if (pHandyCopPointer && TheCamera.pTargetEntity)
	{	
		static float gTimeToStartDWBustedMotion	= 0.0f;
		static float gTimeToApplyMotion	= 1000.0f;
		float time = CTimer::GetTimeInMilliseconds();	
		float timeExpired = time - gTimeDWBustedCamStarted;
		float timeInMotion = timeExpired-gTimeToStartDWBustedMotion;
		
		{
			float t = (timeExpired-gTimeToStartDWBustedMotion) / gTimeToApplyMotion;
			if 	(t>1.0f) 		t = 1.0f;								
			else if (t<0.0f) 	t = 0.0f;					
		
			static bool bKickOffBustedText = false;
		
			if (t>=1.0f && bKickOffBustedText) // migrated!
			{									
				bKickOffBustedText = false;
			}
			else
			{
				bKickOffBustedText = true;
			}			
		
		
			static CVector offsetLookAt2(0.0f,0.0f,0.0f);
			static CVector offsetPosAt2(0.0f,0.0f,-0.5f);
			static float bustedDWFOV2 = 100.0f;
			static float bustedDWFOV2target = 70.0f;
			static float headZOff = -0.06f;
			
			
			
//			offsetLookAt2.z = DW_SINE_ACCEL_DECEL_LERP(t,0.0f,0.5f);
			CVector lastSource = Source;
			Source = TheCamera.pTargetEntity->GetPosition();
			CVector lookAt;// = pHandyCopPointer->GetPosition();				
//Les wanted it removed.			
			FOV = 100.0f;//DW_SINE_ACCEL_DECEL_LERP(t,bustedDWFOV2target,bustedDWFOV2);
//			lookAt += offsetLookAt2;
			
			RpHAnimHierarchy *pHierarchy = GetAnimHierarchyFromSkinClump((RpClump*)pHandyCopPointer->m_pRwObject);
			RwMatrix* boneMat = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, BONETAG_HEAD));
			lookAt = *RwMatrixGetPos(boneMat);
			lookAt.z += headZOff;
			static float tScale = 0.5f;
			lookAt.z -= t * tScale;
						
			Source += offsetPosAt2;			
			
			// new calc of busted target
/*			CVector d = Source - lookAt;
			static float xLen = 0.4f;
			float dLen = d.Magnitude();
			float hLen = CMaths::Sqrt(xLen*xLen + dLen*dLen);
			float r = CMaths::ASin(xLen/hLen);
			float newFOV = RADTODEG(r*2);
			bustedDWFOV2target = newFOV;
*/			
			
			
			Front  = lookAt - Source;
			Front.Normalise();
			Up=CVector(0.0f,0.0f,1.0f);
			CVector TempRight= CrossProduct(Front, Up);
			TempRight.Normalise();
			Up=CrossProduct(TempRight, Front);								
		
			// once we know the proposed position we check for line of sight
			if (bIsFirstTime)
				if (!CWorld::GetIsLineOfSightClear(Source,lookAt, true, true, false, true, false, false, true))
					return false;	
			else
				if (!CWorld::GetIsLineOfSightClear(Source,lookAt, true, false, false, true, false, false, false))						
					Source=lastSource; // dont go through walls.
		
			TheCamera.pTargetEntity->SetIsVisible(false);	// somehow the player is set to be visible again when restarts the level.															
			return true;
		}
	}
	
	return false;
}

#define NUM_ARREST_CAMS 6

bool CCam::ProcessArrestCamOne(void)
{
	bool DoNotAssert=true;
	bool bGotCameraPos = false;
	CVector ArrestCamPos;
	CVector PlayerPosition;
	CVector CopPosition; 
	CVector CopToPlayer;
	
	CPed *pHandyPedPointer = NULL;
	CVehicle *pHandyCarPointer = NULL;
	CPed *pHandyCopPointer = NULL;
	
	CEntity *pHitEntity = NULL;
	CColPoint colPoint;
	FOV=45.0f;
	int32 aTryArrestCamList[NUM_ARREST_CAMS];// = {-1, -1, -1, -1, -1};

	for (int32 i=0;i<NUM_ARREST_CAMS;i++) 
		aTryArrestCamList[i] = -1;

	if(ResetStatics)
	{
		nUsingWhichCamera = 0;
		
		if(TheCamera.pTargetEntity->GetIsTypePed())
		{
			pHandyPedPointer=(CPed*)TheCamera.pTargetEntity;
			pHandyPedPointer->GetBonePosition(PlayerPosition, BONETAG_SPINE1, true);
			if(FindPlayerPed() && FindPlayerPed()->GetPlayerData()->m_ArrestingOfficer)
				pHandyCopPointer = FindPlayerPed()->GetPlayerData()->m_ArrestingOfficer;
			
			if(pHandyCopPointer && CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.5f)
			{
				aTryArrestCamList[0] = ARRESTCAM_DW;
				aTryArrestCamList[1] = ARRESTCAM_OVERSHOULDER;
				aTryArrestCamList[2] = ARRESTCAM_ALONGGROUND;
				aTryArrestCamList[3] = ARRESTCAM_OVERSHOULDER;
				aTryArrestCamList[4] = ARRESTCAM_FROMLAMPPOST;
			}
			else
			{
				aTryArrestCamList[0] = ARRESTCAM_DW;			
				aTryArrestCamList[1] = ARRESTCAM_ALONGGROUND;
				aTryArrestCamList[2] = ARRESTCAM_OVERSHOULDER;
				aTryArrestCamList[3] = ARRESTCAM_FROMLAMPPOST;
			}	
		}
		else if(TheCamera.pTargetEntity->GetIsTypeVehicle())
		{
			pHandyCarPointer=(CVehicle*)TheCamera.pTargetEntity;
			if(pHandyCarPointer->pDriver && pHandyCarPointer->pDriver->IsPlayer())
			{
				pHandyPedPointer = pHandyCarPointer->pDriver;
				pHandyPedPointer->GetBonePosition(PlayerPosition, BONETAG_SPINE1, true);
			}
			else
			{
				pHandyPedPointer = NULL;
				PlayerPosition = pHandyCarPointer->GetPosition();
			}

			if(FindPlayerPed() && FindPlayerPed()->GetPlayerData()->m_ArrestingOfficer)
				pHandyCopPointer = FindPlayerPed()->GetPlayerData()->m_ArrestingOfficer;
			
			if(pHandyCopPointer && CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > 0.65f)
			{
				aTryArrestCamList[0] = ARRESTCAM_OVERSHOULDER;
				aTryArrestCamList[1] = ARRESTCAM_FROMLAMPPOST;
				aTryArrestCamList[2] = ARRESTCAM_ALONGGROUND;
				aTryArrestCamList[3] = ARRESTCAM_OVERSHOULDER;
			}
			else
			{
				aTryArrestCamList[0] = ARRESTCAM_FROMLAMPPOST;
				aTryArrestCamList[1] = ARRESTCAM_ALONGGROUND;
				aTryArrestCamList[2] = ARRESTCAM_OVERSHOULDER;
			}	
		}
		else // uh oh
		{
			ASSERTMSG(false, "Camera target isn't ped or vehicle");
			return false;
		}

		if(!CHud::m_BigMessage[BIG_MESSAGE_WASTED][0])
		{
			GxtChar *pText = TheText.Get("BUSTED");
			CMessages::AddBigMessage(pText, 1000*5, 2);										
		}
		
		int32 nTryCam = 0;
		while(nUsingWhichCamera==0 && nTryCam<NUM_ARREST_CAMS && aTryArrestCamList[nTryCam] > 0)
		{
			pStoredCopPed = NULL;
		
			switch(aTryArrestCamList[nTryCam])
			{
				case ARRESTCAM_DW:
					{
						gTimeDWBustedCamStarted = CTimer::GetTimeInMilliseconds();
						if (ProcessDWBustedCam1(pHandyCopPointer, true))
						{
							TheCamera.pTargetEntity->SetIsVisible(false);	// somehow the player is set to be visible again when restarts the level... slightly dangerous, but tested and looks OK, no idea how game logic works to put this in sensibly...urgh
							nUsingWhichCamera=ARRESTCAM_DW;						
							ResetStatics = false;
							return true;
						}
					}						
					break;			
				case ARRESTCAM_OVERSHOULDER:
					if(pHandyCopPointer)
					{
						bGotCameraPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, pHandyCopPointer, PlayerPosition, ArrestCamPos);
						pStoredCopPed = pHandyCopPointer;
						pHandyCopPointer = NULL;
					}
					else if(pHandyPedPointer)
					{
						int i;
						const int N=pHandyPedPointer->GetPedIntelligence()->GetMaxNumPedsInRange();
						CEntity** ppNearbyEntities=pHandyPedPointer->GetPedIntelligence()->GetNearbyPeds();
						for(i=0;i<N;i++)
						{
							CEntity* pNearbyEntity=ppNearbyEntities[i];
							if(pNearbyEntity)
							{
								ASSERT(pNearbyEntity->GetType()==ENTITY_TYPE_PED);
								CPed* pNearbyPed=(CPed*)pNearbyEntity;
								CTaskSimpleArrestPed* pTaskArrest=(CTaskSimpleArrestPed*)pNearbyPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_ARREST_PED);
								if(pTaskArrest)
								{
									if(FindPlayerPed()==pTaskArrest->GetTargetPed())
									{
										bGotCameraPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, pNearbyPed, PlayerPosition, ArrestCamPos);								
										if(bGotCameraPos)
										{
											pStoredCopPed = pNearbyPed;
											break;
										}			
									}
								}											
							}
						}
						/*
						for(int32 nClosePed=0; nClosePed<pHandyPedPointer->m_NumClosePeds; nClosePed++)
						{
							if(pHandyPedPointer->m_apClosePeds[nClosePed]->GetPedState()==PED_ARREST_PLAYER)
								bGotCameraPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, pHandyPedPointer->m_apClosePeds[nClosePed], PlayerPosition, ArrestCamPos);
							
							if(bGotCameraPos)
							{
								pStoredCopPed = pHandyPedPointer->m_apClosePeds[nClosePed];
								break;
							}
						}
						*/
					}
					break;
				
				case ARRESTCAM_ALONGGROUND:
					if(pHandyCopPointer)
					{
						bGotCameraPos = GetLookAlongGroundPos(TheCamera.pTargetEntity, pHandyCopPointer, PlayerPosition, ArrestCamPos);
						pStoredCopPed = pHandyCopPointer;
						pHandyCopPointer = NULL;
					}
					else if(pHandyPedPointer)
					{
						int i;
						const int N=pHandyPedPointer->GetPedIntelligence()->GetMaxNumPedsInRange();
						CEntity** ppNearbyEntities=pHandyPedPointer->GetPedIntelligence()->GetNearbyPeds();
						for(i=0;i<N;i++)
						{
							CEntity* pNearbyEntity=ppNearbyEntities[i];
							if(pNearbyEntity)
							{
								ASSERT(pNearbyEntity->GetType()==ENTITY_TYPE_PED);
								CPed* pNearbyPed=(CPed*)pNearbyEntity;
								CTaskSimpleArrestPed* pTaskArrest=(CTaskSimpleArrestPed*)pNearbyPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_ARREST_PED);
								if(pTaskArrest)
								{
									if(FindPlayerPed()==pTaskArrest->GetTargetPed())
									{
										bGotCameraPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, pNearbyPed, PlayerPosition, ArrestCamPos);								
										if(bGotCameraPos)
										{
											pStoredCopPed = pNearbyPed;
											break;
										}			
									}
								}											
							}
						}
						/*
						for(int32 nClosePed=0; nClosePed<pHandyPedPointer->m_NumClosePeds; nClosePed++)
						{
							if(pHandyPedPointer->m_apClosePeds[nClosePed]->GetPedState()==PED_ARREST_PLAYER)
								bGotCameraPos = GetLookAlongGroundPos(TheCamera.pTargetEntity, pHandyPedPointer->m_apClosePeds[nClosePed], PlayerPosition, ArrestCamPos);
							
							if(bGotCameraPos)
							{
								pStoredCopPed = pHandyPedPointer->m_apClosePeds[nClosePed];
								break;
							}
						}
						*/
					}
					break;
					
				case ARRESTCAM_FROMLAMPPOST:					
					bGotCameraPos = GetLookFromLampPostPos(TheCamera.pTargetEntity, pHandyCopPointer, PlayerPosition, ArrestCamPos);
					break;
			}
			
			if(bGotCameraPos)
			{
				// if we've saved the pointer to the arresting cop, need to register the reference
				if(pStoredCopPed)
					REGREF(pStoredCopPed, (CEntity **)&pStoredCopPed);

				nUsingWhichCamera = aTryArrestCamList[nTryCam];
				
				// if using camera looking along the ground, need to decide if the cam can rotate
				if(nUsingWhichCamera==ARRESTCAM_ALONGGROUND)
				{
					float fChooseCamRotation = CGeneral::GetRandomNumberInRange(0.0f, 5.0f);
					if(fChooseCamRotation < 1.0f)
						nUsingWhichCamera = ARRESTCAM_ALONGGROUND;
					else if(fChooseCamRotation < 2.0f)
						nUsingWhichCamera = ARRESTCAM_ALONGGROUND_RIGHT;
					else if(fChooseCamRotation < 3.0f)
						nUsingWhichCamera = ARRESTCAM_ALONGGROUND_RIGHT_UP;
					else if(fChooseCamRotation < 4.0f)
						nUsingWhichCamera = ARRESTCAM_ALONGGROUND_LEFT;
					else
						nUsingWhichCamera = ARRESTCAM_ALONGGROUND_LEFT_UP;
				}
			}
			
			nTryCam++;
		}

//		GxtChar *pText = TheText.Get("BUSTED"); // DW - migrated
//		CMessages::AddBigMessage(pText, 1000*5, 2);		

		Source = ArrestCamPos;
		CVector SourceBeforeChange=Source;
		TheCamera.AvoidTheGeometry(SourceBeforeChange, PlayerPosition, Source, FOV);
	
		Front = PlayerPosition - Source;
		Front.Normalise();
		
		Up=CVector(0.0f,0.0f,1.0f);
		CVector TempRight= CrossProduct(Front, Up);
		TempRight.Normalise();
		Up=CrossProduct(TempRight, Front);
		
		if(nUsingWhichCamera!=0)
			ResetStatics = false;
		
		return true;
	}
	else
	{
		if (nUsingWhichCamera==ARRESTCAM_DW)
		{
			TheCamera.pTargetEntity->SetIsVisible(false);

//			pHandyPedPointer=(CPed*)TheCamera.pTargetEntity;
//			pHandyPedPointer->GetBonePosition(PlayerPosition, BONETAG_SPINE1, true);
			if(FindPlayerPed() && FindPlayerPed()->GetPlayerData()->m_ArrestingOfficer)
				pHandyCopPointer = FindPlayerPed()->GetPlayerData()->m_ArrestingOfficer;
			
			if (ProcessDWBustedCam1(pHandyCopPointer,false))		
			{	
				return true;
			}
			else
			{
				return false;
			}
		}
	
		if(TheCamera.pTargetEntity->GetIsTypePed())
		{
			((CPed*)TheCamera.pTargetEntity)->GetBonePosition(PlayerPosition, BONETAG_SPINE1, true);
		}
		else if(TheCamera.pTargetEntity->GetIsTypeVehicle())
		{
			if(((CVehicle *)TheCamera.pTargetEntity)->pDriver && ((CVehicle *)TheCamera.pTargetEntity)->pDriver->IsPlayer())
			{
				((CVehicle *)TheCamera.pTargetEntity)->pDriver->GetBonePosition(PlayerPosition, BONETAG_SPINE1, true);
			}
			else
				PlayerPosition = TheCamera.pTargetEntity->GetPosition();
		}
		else
			return false;
			
		if(nUsingWhichCamera==ARRESTCAM_OVERSHOULDER && pStoredCopPed)
		{
			bGotCameraPos = GetLookOverShoulderPos(TheCamera.pTargetEntity, pStoredCopPed, PlayerPosition, ArrestCamPos);
			// limit how fast this camera can rotate up after it's been initialised
			if(ArrestCamPos.z > Source.z + ARRESTCAM_S_ROTATION_UP*CTimer::GetTimeStep())
				ArrestCamPos.z = Source.z + ARRESTCAM_S_ROTATION_UP*CTimer::GetTimeStep();
		}
		else if(nUsingWhichCamera>ARRESTCAM_ALONGGROUND && nUsingWhichCamera<=ARRESTCAM_ALONGGROUND_LEFT_UP)
		{		
			ArrestCamPos = Source;
			Front = PlayerPosition - Source;
			Front.Normalise();
			
			Up=CVector(0.0f,0.0f,1.0f);
			CVector TempRight= CrossProduct(Front, Up);
			TempRight.Normalise();
			
			// if we want to rotate to the left (clockwise) just flip the right vector
			if(nUsingWhichCamera==ARRESTCAM_ALONGGROUND_LEFT || nUsingWhichCamera==ARRESTCAM_ALONGGROUND_LEFT_UP)
				TempRight *= -1.0f;
			
			if(!CWorld::TestSphereAgainstWorld(Source + 0.5f*TempRight, 0.4f, TheCamera.pTargetEntity, true, true, false, true, false, true))
			{
				bGotCameraPos = true;
				ArrestCamPos += TempRight*ARRESTCAM_ROTATION_SPEED*CTimer::GetTimeStep();
				// if we want to rotate up into the air as well
				if(nUsingWhichCamera==ARRESTCAM_ALONGGROUND_RIGHT_UP || nUsingWhichCamera==ARRESTCAM_ALONGGROUND_LEFT_UP)
				{
					ArrestCamPos.z += ARRESTCAM_ROTATION_UP*CTimer::GetTimeStep();
				}
				// else stay const distance above the ground
				else
				{
					bool bGroundFound = false;
					float fGroundZ=CWorld::FindGroundZFor3DCoord(ArrestCamPos.x, ArrestCamPos.y, ArrestCamPos.z, &bGroundFound); 
					if(bGroundFound)
						ArrestCamPos.z = fGroundZ + ARRESTDIST_ABOVE_GROUND;
				}
			}
			else
			{
			}
		}
		else if(nUsingWhichCamera==ARRESTCAM_FROMLAMPPOST)
		{
			ArrestCamPos = Source;
			Front = PlayerPosition - ArrestCamPos;
			Front.z = 0.0f;
			Front.Normalise();

			Up=CVector(0.0f,0.0f,1.0f);
			CVector TempRight= CrossProduct(Front, Up);
			TempRight.Normalise();
			
			Front = (PlayerPosition - ArrestCamPos) + TempRight*ARRESTCAM_LAMPPOST_ROTATEDIST;
			Front.z = 0.0f;
			Front.Normalise();


			
			if(!CWorld::TestSphereAgainstWorld(ArrestCamPos + 0.5f*Front, 0.4f, TheCamera.pTargetEntity, true, true, false, true, false, true))
			{
				bGotCameraPos = true;
				ArrestCamPos += Front*ARRESTCAM_LAMPPOST_TRANSLATE*CTimer::GetTimeStep();								
			}						
		}
		
		if(bGotCameraPos)
		{
			Source = ArrestCamPos;
			CVector SourceBeforeChange=Source;
			TheCamera.AvoidTheGeometry(SourceBeforeChange, PlayerPosition, Source, FOV);
		
			Front = PlayerPosition - Source;
			Front.Normalise();
			
			Up=CVector(0.0f,0.0f,1.0f);
			CVector TempRight= CrossProduct(Front, Up);
			TempRight.Normalise();
			Up=CrossProduct(TempRight, Front);									
		}
		else
		{
			CVector SourceBeforeChange=Source;
			TheCamera.AvoidTheGeometry(SourceBeforeChange, PlayerPosition, Source, FOV);
		}							
	}
	
	return true;
	
///////////////////////////////////////////////////
/*
		Beta=CGeneral::GetATanOfXY(CopToPlayer.x, CopToPlayer.y);
		Source=PlayerPosition+(DistanceAwayFromPlayer*CVector(CMaths::Cos(Beta), CMaths::Sin(Beta), 0));  
		Source.z+=6.0f;
		
		GroundZ=CWorld::FindGroundZFor3DCoord(Source.x, Source.y, Source.z, &GroundFound); 
		if (!GroundFound)
		{
			GroundZ=CWorld::FindRoofZFor3DCoord(Source.x, Source.y, Source.z, &GroundFound);
			if (!GroundFound)
			{
				return false;
			}
		}
		Source.z=GroundZ+0.25f;
		//CopPosition.z+=0.35;

		if (!(CWorld::GetIsLineOfSightClear(Source, CopPosition, 1, 1, 0, true, 0, true, true)))
		{
			Beta+=DEGTORAD(115);
			Source=PlayerPosition+(DistanceAwayFromPlayer*CVector(CMaths::Cos(Beta), CMaths::Sin(Beta), 0));  
			Source.z+=6.0f;
			GroundZ=CWorld::FindGroundZFor3DCoord(Source.x, Source.y, Source.z, &GroundFound); 
			if (!GroundFound)
			{
				GroundZ=CWorld::FindRoofZFor3DCoord(Source.x, Source.y, Source.z, &GroundFound);
				if (!GroundFound)
				{
					return false;
				}
			}
			Source.z=GroundZ+0.25f;
			CopPosition.z+=0.35f;
			Front=CopPosition-Source;
			if (!(CWorld::GetIsLineOfSightClear(Source, CopPosition, 1, 1, 0, true, 0, true, true)))
			{
				return false;
			}
		}
		CopPosition.z+=0.35f;
		m_cvecTargetCoorsForFudgeInter=CopPosition;
		Front=CopPosition-Source;
		ResetStatics=false;
		GetVectorsReadyForRW();
		return true;
	}
	return true;
*/
}








///////////////////////////////////////////////////////////
float DEADCAM_HEIGHT_START = 2.0f;
float DEADCAM_HEIGHT_RATE = 0.04f;
float DEADCAM_WAFT_AMPLITUDE = 2.0f;
float DEADCAM_WAFT_RATE = 600.0f;
float DEADCAM_WAFT_TILT_AMP = -0.35f;
bool bForceOldWaft = false;
//
void CCam::ProcessPedsDeadBaby(void)
{
	CVector PlayerPosition;
	CVector DeadCamPos;
	CVector vecTempRight;

	// first find the target position
	if(TheCamera.pTargetEntity->GetIsTypePed())
	{
		((CPed*)TheCamera.pTargetEntity)->GetBonePosition(PlayerPosition, BONETAG_SPINE1, true);
	}
	else if(TheCamera.pTargetEntity->GetIsTypeVehicle())
	{
		PlayerPosition = TheCamera.pTargetEntity->GetPosition();
		PlayerPosition.z += TheCamera.pTargetEntity->GetColModel().GetBoundBoxMax().z;
	}
	else
		return;
#if 1

	static float startTimeDWDeadCam = 0.0f;
	static float cameraRot = 0.0f;
	
	if(ResetStatics)
	{
		startTimeDWDeadCam = CTimer::GetTimeInMilliseconds();
		cameraRot = 0.0f;
		
		TheCamera.m_uiTimeLastChange = CTimer::GetTimeInMilliseconds();
		DeadCamPos = PlayerPosition;
		DeadCamPos.z += DEADCAM_HEIGHT_START;
		
		float fCamWaterLevel = 0.0f;
		if( CWaterLevel::GetWaterLevelNoWaves(DeadCamPos.x, DeadCamPos.y, DeadCamPos.z, &fCamWaterLevel) )
		{
			if(fCamWaterLevel + 1.5f > DeadCamPos.z)
				DeadCamPos.z = fCamWaterLevel + 1.5f;
		}
		
		vecTempRight = CrossProduct(TheCamera.pTargetEntity->GetMatrix().GetForward(), CVector(0.0f, 0.0f, 1.0f));
		vecTempRight.z = 0.0f;
		vecTempRight.Normalise();
		
		Front = PlayerPosition - DeadCamPos;
		Front.Normalise();
		
		Up = CrossProduct(vecTempRight, Front);
		Up.Normalise();
			
		ResetStatics=false;
	}
	else
	{
		DeadCamPos = Source;
		
		if(!CWorld::TestSphereAgainstWorld(DeadCamPos + 0.2f*CVector(0.0f,0.0f,1.0f), 0.3f, TheCamera.pTargetEntity, true, true, false, true, false, true))
		{
			DeadCamPos.z += DEADCAM_HEIGHT_RATE*CTimer::GetTimeStep();
		}
		
		vecTempRight = CrossProduct(TheCamera.pTargetEntity->GetMatrix().GetForward(), CVector(0.0f, 0.0f, 1.0f));
		vecTempRight.z = 0.0f;
		vecTempRight.Normalise();
		
		float fTimeOffset = (float)(CTimer::GetTimeInMilliseconds() - TheCamera.m_uiTimeLastChange);
		float fWaftOffset = (MIN(1000.0f, fTimeOffset)/1000.0f)*CMaths::Sin(fTimeOffset/DEADCAM_WAFT_RATE);
		CVector vecWaftPos = vecTempRight*DEADCAM_WAFT_AMPLITUDE*fWaftOffset;
		vecWaftPos.x += PlayerPosition.x;
		vecWaftPos.y += PlayerPosition.y;
		vecWaftPos.z = DeadCamPos.z;
		
		CVector vecTempTest = vecWaftPos - DeadCamPos;
		vecTempTest.Normalise();
		
		if(!CWorld::TestSphereAgainstWorld(DeadCamPos + 0.2f*vecTempTest, 0.3f, TheCamera.pTargetEntity, true, true, false, true, false, true))
		{
			DeadCamPos = vecWaftPos;
		}
		
		Front = CVector(0.0f, 0.0f, -1.0f);//PlayerPosition - DeadCamPos;
//		Front.Normalise();
		Front += vecTempRight*DEADCAM_WAFT_TILT_AMP*(MIN(2000.0f, fTimeOffset)/2000.0f)*CMaths::Cos(fTimeOffset/DEADCAM_WAFT_RATE);
//		Front += vecTempRight*DEADCAM_WAFT_TILT_AMP*fWaftOffset;
		Front.Normalise();
		
		Up = CrossProduct(vecTempRight, Front);
		Up.Normalise();
	}
#endif

//#define SPIRALCAM
#ifdef SPIRALCAM
	float curTime 	= 	CTimer::GetTimeInMilliseconds();
	float deltaTime = 	curTime - startTimeDWDeadCam;
	static float rotSpeed = 0.25f;
	static float spiralOutSpeed = 0.25f;
	float rot 		= 	TWO_PI * (deltaTime/1000.0f) * rotSpeed; 

	static CVector rotatedOffset = CVector (0.1f,0,0);
	
	CVector rotOff = rotatedOffset + (rotatedOffset * (deltaTime/1000.0f) * spiralOutSpeed);
	
	CMatrix rotMat;
	rotMat.SetRotateZ(rot);
	CVector r = Multiply3x3(rotOff,rotMat);
	
	static float extraZ = 3.0f;
	static float gainHeightSpeed = 0.25f;
	
	float eZ = extraZ + ((deltaTime/1000.0f) * gainHeightSpeed); 
	
	Source = PlayerPosition;
	Source 		+= r;
	Source.z 	+= eZ;
	
	Front = PlayerPosition - Source;
	Front.Normalise();

	vecTempRight = CrossProduct(Front, CVector(0.0f, 0.0f, 1.0f));
	vecTempRight.z = 0.0f;
	vecTempRight.Normalise();
	
	Up = CrossProduct(vecTempRight, Front);
	Up.Normalise();	
#endif

	Source = DeadCamPos;
	CVector SourceBeforeChange=Source;
	TheCamera.AvoidTheGeometry(SourceBeforeChange, PlayerPosition, Source, FOV);
	// don't want TheCamera to recalculate Front and Up vectors
	TheCamera.m_bMoveCamToAvoidGeom = false;


// Mark's old wasted cam //////////////////////
/*	float CamToPlayerDist=0.0f;
	float TargetBeta=0.0f;
	float TargetAlpha=DEGTORAD(89.5f);
	static bool SafeToRotate=false;
	FOV=70.0f;

	CVector CamToPlayer=CVector(Source-CamTargetEntity->GetPosition());
	CamToPlayerDist=CMaths::Sqrt(CamToPlayer.x * CamToPlayer.x + CamToPlayer.y * CamToPlayer.y + CamToPlayer.z * CamToPlayer.z);
	Beta= CGeneral::GetATanOfXY(CamToPlayer.x, CamToPlayer.y);
	//Alpha=CGeneral::GetATanOfXY(3 , CamToPlayer.z);

	while (Beta>=(PI)) {Beta-=2.0f * PI;}
	while (Beta<(-PI)) {Beta+=2.0f * PI;}

	//while (Alpha>=(PI)) {Alpha-=2.0f * PI;}
	//while (Alpha<(-PI)) {Alpha+=2.0f * PI;}
	if (ResetStatics==TRUE)
	{
		//check like an eqilateral triangle
		bool ThisSideCamSafe;
		bool LateralLeftCornerCamSafe;
		bool LateralRightCornerCamSafe;
		CVector TestSource;
		//lets test if it is safe to rotate
		TestSource=CamTargetEntity->GetPosition() + 4*CVector(CMaths::Cos(Beta) * CMaths::Cos(Alpha) , CMaths::Cos(Alpha)*CMaths::Sin(Beta), CMaths::Sin(Alpha));
		ThisSideCamSafe=(CWorld::GetIsLineOfSightClear(TestSource, CamTargetEntity->GetPosition() ,1, 0 , 0, true ,0, true, true));


		TestSource=CamTargetEntity->GetPosition() + 4*CVector(CMaths::Cos(Beta+DEGTORAD(120)) * CMaths::Cos(Alpha) , CMaths::Cos(Alpha)*CMaths::Sin(Beta+DEGTORAD(120)), CMaths::Sin(Alpha));
		LateralLeftCornerCamSafe=(CWorld::GetIsLineOfSightClear(TestSource, CamTargetEntity->GetPosition() ,1, 0 , 0, true ,0, true, true));

		TestSource=CamTargetEntity->GetPosition() + 4*CVector(CMaths::Cos(Beta-DEGTORAD(120)) * CMaths::Cos(Alpha) , CMaths::Cos(Alpha)*CMaths::Sin(Beta-DEGTORAD(120)), CMaths::Sin(Alpha));
		LateralRightCornerCamSafe=(CWorld::GetIsLineOfSightClear(TestSource, CamTargetEntity->GetPosition() ,1, 0 , 0, true ,0, true, true));

			if ((ThisSideCamSafe)&&(LateralLeftCornerCamSafe)&&(LateralRightCornerCamSafe))
			{
				SafeToRotate=true;
			}
			else
			{
				SafeToRotate=false;
			}
			
		ResetStatics=false;
	}

	if (SafeToRotate)
	{
		TargetBeta=Beta+DEGTORAD(175.0f);
		WellBufferMe(TargetBeta, &Beta, &BetaSpeed, 0.015f, 0.007f, true);
	}

	WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.015f, 0.07f, true);
	WellBufferMe(35.0f, &CamToPlayerDist, &DistanceSpeed, 0.006f, 0.007f); 

	Source=CamTargetEntity->GetPosition() + CamToPlayerDist*CVector(CMaths::Cos(Beta) * CMaths::Cos(Alpha) , CMaths::Cos(Alpha)*CMaths::Sin(Beta), CMaths::Sin(Alpha));
	m_cvecTargetCoorsForFudgeInter=CamTargetEntity->GetPosition();
	Front=CamTargetEntity->GetPosition()-Source;
	Front.Normalise();
	GetVectorsReadyForRW(); 
*/
}



///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Cam_TwoPlayer()
// PURPOSE    : 
// RETURNS    : nowt
// PARAMETERS :
//			  : A camera that keeps two players in view.
///////////////////////////////////////////////////////////////////////////


TweakUInt32 gTimeBeforeEmergency2PlayerModeKicksIn = 500;
TweakUInt32 CAM_2PLAYER_STOP_MOVEMENT_INPUT_TIME = 1000;
TweakUInt32 CAM_2PLAYER_STOP_STICK_INPUT_TIME = 1000;

TweakFloat CAM_2PLAYER_COLSWING_MULT = 0.2f;
TweakFloat CAM_2PLAYER_COLSWING_CAP = 0.1f;

#define TESTSTEP 		(0.15f)
#define	ANGLETRIES 		(21)

void CCam::Process_Cam_TwoPlayer()
{
	CVector	TempSource, TempFront;
//	float	BetaDiff;
	Int32	TestCount;

	float origAlpha = Alpha;
	float origBeta = Beta;
	ASSERT(CWorld::Players[0].pPed && CWorld::Players[1].pPed);
	
	if(CPad::GetPad(0)->CycleCameraModeUpJustDown())
	{
		if(CGameLogic::n2PlayerPedInFocus==0)
			CGameLogic::n2PlayerPedInFocus = 2;
		else
			CGameLogic::n2PlayerPedInFocus = 0;
	}
	else if(CPad::GetPad(1)->CycleCameraModeUpJustDown())
	{
		if(CGameLogic::n2PlayerPedInFocus==1)
			CGameLogic::n2PlayerPedInFocus = 2;
		else
			CGameLogic::n2PlayerPedInFocus = 1;
	}
	
	switch(CGameLogic::n2PlayerPedInFocus)
	{
		case 0:
			if(CWorld::Players[0].pPed->m_nPedFlags.bInVehicle && CWorld::Players[0].pPed->m_pMyVehicle)
			{
				CamTargetEntity = CWorld::Players[0].pPed->m_pMyVehicle;
				Process_FollowCar_SA(CamTargetEntity->GetPosition(), 0.0f, 0.0f, 0.0f);
				CamTargetEntity = CWorld::Players[0].pPed;
			}		
			else
			{
				Process_FollowPed_SA(CWorld::Players[0].pPed->GetPosition(), 0.0f, 0.0f, 0.0f);
			}
			ResetStatics = false;
			return;
			break;
		case 1:
			if(CWorld::Players[1].pPed->m_nPedFlags.bInVehicle && CWorld::Players[1].pPed->m_pMyVehicle)
			{
				CamTargetEntity = CWorld::Players[1].pPed->m_pMyVehicle;
				Process_FollowCar_SA(CamTargetEntity->GetPosition(), 0.0f, 0.0f, 0.0f);
				CamTargetEntity = CWorld::Players[0].pPed;
			}
			else
			{
				CamTargetEntity = CWorld::Players[1].pPed;
				Process_FollowPed_SA(CWorld::Players[1].pPed->GetPosition(), 0.0f, 0.0f, 0.0f);
				CamTargetEntity = CWorld::Players[0].pPed;
			}
			ResetStatics = false;
			return;
			break;
		case 2:
		default:
			// default option is to just continue on with 2player camera
			break;
	}


	Alpha = (-30.0f / 180.0f) * PI;
	AlphaSpeed = 0.0f;
	float	TestBeta = Beta;

	// Find a good beta to go for.
	TestCount = 0;
	while (TestCount < ANGLETRIES)
	{
		if (TestCount & 1)
		{
			TestBeta = Beta + ((TestCount + 1) / 2) * TESTSTEP;
		}
		else
		{
			TestBeta = Beta - ((TestCount + 1) / 2) * TESTSTEP;
		}
		
		Process_Cam_TwoPlayer_CalcSource(TestBeta, &TempSource, &TempFront, &m_cvecTargetCoorsForFudgeInter);
		if (Process_Cam_TwoPlayer_TestLOSs(TempSource))
		{
			// This angle will do.
			gLastTime2PlayerCameraWasOK = CTimer::GetTimeInMilliseconds();
			break;
		}
		TestCount++;
	}
	
	// Didn't find an angle. Use old one. (don't move)
	if(TestCount == ANGLETRIES)
		TestBeta = Beta;
	
	if(TestCount > 0)
		gLastTime2PlayerCameraCollided = CTimer::GetTimeInMilliseconds();
		
	if(ResetStatics)
	{
		Beta = TestBeta;
	}

	float fTargetBeta = TestBeta;
	if(fTargetBeta > Beta + PI)
		fTargetBeta -= TWO_PI;
	else if(fTargetBeta < Beta - PI)
		fTargetBeta += TWO_PI;
	
	fTargetBeta = CAM_2PLAYER_COLSWING_MULT*(fTargetBeta - Beta);
	if(fTargetBeta > CAM_2PLAYER_COLSWING_CAP)
		fTargetBeta = CAM_2PLAYER_COLSWING_CAP;
	else if(fTargetBeta < -CAM_2PLAYER_COLSWING_CAP)
		fTargetBeta = -CAM_2PLAYER_COLSWING_CAP;

	fTargetBeta += Beta;
	
	uint32 nType = FOLLOW_PED_2PLAYER;
	float fDiffMult, fDiffCap;
	float fTargetDiff = 0.0f;
	if(TestCount==0 && CTimer::GetTimeInMilliseconds() >= gLastTime2PlayerCameraCollided + CAM_2PLAYER_STOP_MOVEMENT_INPUT_TIME)
	{
		CVector vecAveSpeed = CWorld::Players[0].pPed->GetMoveSpeed() + CWorld::Players[1].pPed->GetMoveSpeed();
		if(vecAveSpeed.MagnitudeSqr() > 0.01f)
		{
			fDiffMult = PEDCAM_SET[nType].fDiffBetaSwing*CTimer::GetTimeStep();
			fDiffCap = PEDCAM_SET[nType].fDiffBetaSwingCap*CTimer::GetTimeStep();
		
			float fHeadingBeta = CMaths::ATan2(-vecAveSpeed.x, vecAveSpeed.y) - HALF_PI;
			if(fHeadingBeta - fTargetBeta > PI)	fHeadingBeta -= TWO_PI;
			else if(fHeadingBeta -fTargetBeta < -PI)	fHeadingBeta += TWO_PI;


			fDiffMult = MIN(1.0f, vecAveSpeed.Magnitude()*fDiffMult);
			
			fTargetDiff = fDiffMult*(fHeadingBeta - fTargetBeta);
			if(fTargetDiff > fDiffCap)
				fTargetDiff = fDiffCap;
			else if(fTargetDiff < -fDiffCap)
				fTargetDiff = -fDiffCap;
		}
		
		if(fTargetDiff > 0.01f)
			TestBeta += TESTSTEP;
		else if(fTargetDiff < 0.01f)
			TestBeta -= TESTSTEP;
		
		if(CMaths::Abs(fTargetDiff) > 0.01f)
		{
			Process_Cam_TwoPlayer_CalcSource(TestBeta, &TempSource, &TempFront, &m_cvecTargetCoorsForFudgeInter);
			if(!Process_Cam_TwoPlayer_TestLOSs(TempSource))
			{
				if(fTargetDiff > 0.0f && BetaSpeed > 0.0f)
					BetaSpeed = 0.0f;
				else if(fTargetDiff < 0.0f && BetaSpeed < 0.0f)
					BetaSpeed = 0.0f;

				gLastTime2PlayerCameraCollided = CTimer::GetTimeInMilliseconds();
				fTargetDiff = 0.0f;
			}
		}
	}
	
	fTargetBeta += fTargetDiff;
	if(fTargetBeta > Beta + PI)	fTargetBeta -= TWO_PI;
	else if(fTargetBeta < Beta - PI)	fTargetBeta += TWO_PI;

	float fCamControlBetaSpeed = (fTargetBeta - Beta)/CMaths::Max(1.0f, CTimer::GetTimeStep());

	fDiffMult = CMaths::Pow(PEDCAM_SET[nType].fDiffBetaRate, CTimer::GetTimeStep());
	fDiffCap = PEDCAM_SET[nType].fDiffBetaCap;
	if(TestCount==0 && CTimer::GetTimeInMilliseconds() >= gLastTime2PlayerCameraCollided + CAM_2PLAYER_STOP_STICK_INPUT_TIME)
	{
		float StickBetaOffset = -(CPad::GetPad(0)->AimWeaponLeftRight(CWorld::Players[0].pPed));
		StickBetaOffset += -(CPad::GetPad(1)->AimWeaponLeftRight(CWorld::Players[1].pPed));
		if(StickBetaOffset > 128.0f)
			StickBetaOffset = 128.0f;
		else if(StickBetaOffset < -128.0f)
			StickBetaOffset = -128.0f;
		
		StickBetaOffset = AIMWEAPON_STICK_SENS*AIMWEAPON_STICK_SENS*CMaths::Abs(StickBetaOffset)*StickBetaOffset * (0.25f/3.5f * (FOV/80.0f));// * CTimer::GetTimeStep();

		if(StickBetaOffset > 0.01f)
			TestBeta += TESTSTEP;
		else if(StickBetaOffset < 0.01f)
			TestBeta -= TESTSTEP;
		
		if(CMaths::Abs(StickBetaOffset) > 0.01f)
		{
			Process_Cam_TwoPlayer_CalcSource(TestBeta, &TempSource, &TempFront, &m_cvecTargetCoorsForFudgeInter);
			if(!Process_Cam_TwoPlayer_TestLOSs(TempSource))
			{
				if(StickBetaOffset > 0.0f && BetaSpeed > 0.0f)
					BetaSpeed = 0.0f;
				else if(StickBetaOffset < 0.0f && BetaSpeed < 0.0f)
					BetaSpeed = 0.0f;

				gLastTime2PlayerCameraCollided = CTimer::GetTimeInMilliseconds();
				StickBetaOffset = 0.0f;
			}
		}

		fCamControlBetaSpeed += StickBetaOffset;
	}
	
	if(fCamControlBetaSpeed > fDiffCap)
		fCamControlBetaSpeed = fDiffCap;
	else if(fCamControlBetaSpeed < -fDiffCap)
		fCamControlBetaSpeed = -fDiffCap;

	BetaSpeed = fDiffMult*BetaSpeed + (1.0f - fDiffMult)*fCamControlBetaSpeed;
	Beta += BetaSpeed*CTimer::GetTimeStep();

/*	
	BetaDiff = TestBeta - Beta;
	while (BetaDiff > PI) BetaDiff -= 2.0f * PI;
	while (BetaDiff < -PI) BetaDiff += 2.0f * PI;
	// Calculate the new speed for beta
	if (BetaDiff < 0.0f)
	{
		BetaSpeed -= CTimer::GetTimeStep() * 0.002f;
		if (BetaSpeed < BetaDiff * 0.05f)
		{
			BetaSpeed = BetaDiff * 0.05f;
		}
	}
	else
	{
		BetaSpeed += CTimer::GetTimeStep() * 0.002f;
		if (BetaSpeed > BetaDiff * 0.05f)
		{
			BetaSpeed = BetaDiff * 0.05f;
		}
	}
	Beta += BetaSpeed * CTimer::GetTimeStep();	
*/
	Process_Cam_TwoPlayer_CalcSource(Beta, &Source, &Front, &m_cvecTargetCoorsForFudgeInter);
	
	if(TestCount == ANGLETRIES)
	{
		// One or both players are blocked in LOS
		// Choose a player to move in and look at - DW - this is a fix for a bug
		if (CTimer::GetTimeInMilliseconds() - gLastTime2PlayerCameraWasOK > gTimeBeforeEmergency2PlayerModeKicksIn)
		{
			CColPoint colPoint;
			CEntity* pHitEntity=NULL;
			gCurCamColVars = CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE; // this should be set back// Dw - I dont understand why this has been done?... Sandy?

			// Test whether the players are obscured. If one of them is, then this beta is not suitable.
			if(CWorld::ProcessLineOfSight(m_cvecTargetCoorsForFudgeInter, Source, colPoint, pHitEntity, true, false, false, false, false, true, true))
			{
				Source = colPoint.GetPosition();
			}
			
			if(CTimer::GetTimeInMilliseconds() > CGameLogic::nPrintFocusHelpTimer && CGameLogic::nPrintFocusHelpCounter < 6)
			{
				CHud::SetHelpMessage(TheText.Get("WRN2_2P"), false);
				CGameLogic::nPrintFocusHelpTimer = CTimer::GetTimeInMilliseconds() + 60000;
				CGameLogic::nPrintFocusHelpCounter++;
			}
		}
	}

	
	Up=CVector(0.0f, 0.0f, 1.0f);
	Up.Normalise();
//	Front.Normalise();
	CVector TempRight = CrossProduct( Up, Front );
	TempRight.Normalise();
	Up=CrossProduct( Front, TempRight );
	Up.Normalise();
	FOV=70.0f;
	
	ResetStatics = false;
}


///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Cam_TwoPlayer_TestBeta()
// PURPOSE    : 
// RETURNS    : nowt
// PARAMETERS :
//			  : This function will return false if the LOS to either player is blocked.
///////////////////////////////////////////////////////////////////////////

bool CCam::Process_Cam_TwoPlayer_TestLOSs(CVector TempSource)
{
	CColPoint colPoint;
	CEntity* pHitEntity=NULL;
	gCurCamColVars = CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE; // this should be set back// Dw - I dont understand why this has been done?... Sandy?

	// Test whether the players are obscured. If one of them is, then this beta is not suitable.
	if(CWorld::ProcessLineOfSight(TempSource, CWorld::Players[0].pPed->GetPosition(), colPoint, pHitEntity, true, false, false, false, false, true, true))
	{
		return false;
	}
	if(CWorld::ProcessLineOfSight(TempSource, CWorld::Players[1].pPed->GetPosition(), colPoint, pHitEntity, true, false, false, false, false, true, true))
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Cam_TwoPlayer_TestBeta()
// PURPOSE    : 
// RETURNS    : nowt
// PARAMETERS :
//			  : This function will return false if the LOS to either player is blocked.
///////////////////////////////////////////////////////////////////////////

void CCam::Process_Cam_TwoPlayer_CalcSource(float Beta, CVector *pSource, CVector *pLookAt, CVector *pTarget)
{
	float	CamDist, PlayersApart, Player0Weight;

	*pLookAt =CVector(-CMaths::Cos(Beta)*CMaths::Cos(Alpha), -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
	CVector CamLookAtHor = CVector(pLookAt->x, pLookAt->y, 0.0f);
	CamLookAtHor.Normalise();

	PlayersApart = (CWorld::Players[0].pPed->GetPosition() - CWorld::Players[1].pPed->GetPosition()).Magnitude();
	CamDist = 7.0f + PlayersApart * 0.67f;

	CVector Vec1To2 = CWorld::Players[0].pPed->GetPosition() - CWorld::Players[1].pPed->GetPosition();
	Vec1To2.z = 0;
	Vec1To2.Normalise();
	
	Player0Weight = 0.5f - 0.25f * DotProduct(Vec1To2, CamLookAtHor);

	*pTarget = (CWorld::Players[0].pPed->GetPosition() * Player0Weight) + (CWorld::Players[1].pPed->GetPosition() * (1.0f - Player0Weight));

	*pSource = *pTarget - (CamDist * *pLookAt);

	// Move source a little bit higher so that we see a bit more at the top of the screen.
	pSource->z += CamDist * 0.1f;

}

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Cam_TwoPlayer_InCarAndShooting()
// PURPOSE    : 
// RETURNS    : nowt
// PARAMETERS :
//			  : One person is driving and the other is aiming a crosshair to shoot.
///////////////////////////////////////////////////////////////////////////
TweakFloat TWOPLAYER_INCARANDSHOOTING_DIST = 0.5f;
TweakFloat TWOPLAYER_INCARANDSHOOTING_DIST_MAX_ADD = 2.0f;
TweakFloat TWOPLAYER_INCARANDSHOOTING_ALPHA_DEFAULT = -DEGTORAD(20.0f);
TweakFloat TWOPLAYER_INCARANDSHOOTING_ALPHA_STICK_RATE = 0.5f;

TweakFloat TWOPLAYER_INCARANDSHOOTING_ALPHA_MIN = DEGTORAD(-60.0f);
TweakFloat TWOPLAYER_INCARANDSHOOTING_ALPHA_MAX = DEGTORAD(30.0f);
TweakFloat TWOPLAYER_INCARANDSHOOTING_ALPHA_RATE = 0.96f;
TweakFloat TWOPLAYER_INCARANDSHOOTING_ALPHA_SPEED_MULT = 5.0f;

TweakFloat TWOPLAYER_INHELIANDSHOOTING_ALPHA_MIN = DEGTORAD(-60.0f);
TweakFloat TWOPLAYER_INHELIANDSHOOTING_ALPHA_MAX = DEGTORAD(10.0f);
TweakFloat TWOPLAYER_INHELIANDSHOOTING_ALPHA_SPEED_MULT = 3.0f;


TweakFloat TWOPLAYER_AIM_STICK_SENS = 0.004f;
TweakFloat TWOPLAYER_LOCKON_TRACKRATE = 0.85f;
TweakFloat TWOPLAYER_LOCKON_TRACKCAP = 0.01f;
//
void CCam::Process_Cam_TwoPlayer_InCarAndShooting()
{
	float	RotateCam;
	CPad 	*p2ndPad;
	CPed	*p2ndPed;

	ASSERT(CWorld::Players[0].pPed && CWorld::Players[1].pPed);
	ASSERT(CWorld::Players[0].pPed->m_pMyVehicle == CWorld::Players[1].pPed->m_pMyVehicle);
	ASSERT(CWorld::Players[0].pPed->m_pMyVehicle);
	ASSERT(CamTargetEntity->GetIsTypeVehicle());
	
	if(!CamTargetEntity->GetIsTypeVehicle())
		return;

	CVehicle *pVehicle = CWorld::Players[0].pPed->m_pMyVehicle;

	CVector VecDistance;
	float DistMagnitude=0.0f;
	//FOV=30.0f;
	CVector TargetCoors = CWorld::Players[0].pPed->m_pMyVehicle->GetPosition();
	float TargetOrientation = pVehicle->GetHeading() - HALF_PI;

	// The player that isn't the driver gets the opportunity to look around and move the crosshair.
	if (CWorld::Players[0].pPed->m_pMyVehicle->pDriver == CWorld::Players[0].pPed)
	{
		p2ndPad = CPad::GetPad(1);
		p2ndPed = CWorld::Players[1].pPed;
	}
	else
	{
		p2ndPad = CPad::GetPad(0);
		p2ndPed = CWorld::Players[0].pPed;
	}

//	if(ResetStatics)
//	{
//		FOV=70.f;
//	}
//	else
	{
		if((pVehicle->GetVehicleType()==VEHICLE_TYPE_CAR || pVehicle->GetVehicleType()==VEHICLE_TYPE_BIKE)
		&& DotProduct(pVehicle->GetMoveSpeed(), pVehicle->GetMatrix().GetForward()) > CAR_FOV_START_SPEED)
			FOV += (DotProduct(pVehicle->GetMoveSpeed(), pVehicle->GetMatrix().GetForward()) - CAR_FOV_START_SPEED)*CTimer::GetTimeStep();
		
		if(FOV > 70.0f)
			FOV = 70.0f + (FOV - 70.0f)*CMaths::Pow(CAR_FOV_FADE_MULT, CTimer::GetTimeStep());
			
		if(FOV > 100.0f)
			FOV = 100.0f;
		else if(FOV < 70.0f)
			FOV = 70.0f;
	}

	float fStickX = p2ndPad->AimWeaponLeftRight(p2ndPed);
	float fStickY = -(p2ndPad->AimWeaponUpDown(p2ndPed));	
			
	X_Targetting += TWOPLAYER_AIM_STICK_SENS*TWOPLAYER_AIM_STICK_SENS*CMaths::Abs(fStickX)*fStickX * (0.25f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
	Y_Targetting += TWOPLAYER_AIM_STICK_SENS*TWOPLAYER_AIM_STICK_SENS*CMaths::Abs(fStickY)*fStickY * (0.15f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
/*
	X_Targetting += CTimer::GetTimeStep() * p2ndPad->GetSteeringLeftRight() / 3000.0f;
	if (CPad::bInvertLook4Pad)
	{
		Y_Targetting += CTimer::GetTimeStep() * p2ndPad->GetSteeringUpDown() / 3000.0f;
	}
	else
	{
		Y_Targetting -= CTimer::GetTimeStep() * p2ndPad->GetSteeringUpDown() / 3000.0f;
	}
*/

	float X_TargettingLockOn, Y_TargettingLockOn;
	CWeaponInfo *pWeaponInfo = CWeaponInfo::GetWeaponInfo(p2ndPed->GetWeapon()->GetWeaponType(), p2ndPed->GetWeaponSkill());
	CEntity *pLockOnTarget = CWeapon::FindNearestTargetEntityWithScreenCoors(X_Targetting, Y_Targetting, INCAR_2PLAYER_WEAPON_RANGE_MULTIPLIER*pWeaponInfo->GetWeaponRange(), p2ndPed->GetPosition(), &X_TargettingLockOn, &Y_TargettingLockOn);

	if(pLockOnTarget && CMaths::Abs(fStickX) < 120.0f && CMaths::Abs(fStickY) < 120.0f)
	{
		float fDiff = X_TargettingLockOn - X_Targetting;
		fDiff *= 1.0f - CMaths::Pow(TWOPLAYER_LOCKON_TRACKRATE, CTimer::GetTimeStep());
		if(fDiff > TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep())
			fDiff = TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep();
		else if(fDiff < -TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep())
			fDiff = -TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep();
		X_Targetting += fDiff;

		fDiff = Y_TargettingLockOn - Y_Targetting;
		fDiff *= 1.0f - CMaths::Pow(TWOPLAYER_LOCKON_TRACKRATE, CTimer::GetTimeStep());
		if(fDiff > TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep())
			fDiff = TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep();
		else if(fDiff < -TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep())
			fDiff = -TWOPLAYER_LOCKON_TRACKCAP*CTimer::GetTimeStep();
		Y_Targetting += fDiff;
	}

	RotateCam = 0.0f;
	if (X_Targetting > 0.9f) { RotateCam = X_Targetting - 0.9f; X_Targetting = 0.9f; }
	if (X_Targetting < -0.9f) { RotateCam = X_Targetting + 0.9f; X_Targetting = -0.9f; }
	if (Y_Targetting > 0.9f) { Alpha -= TWOPLAYER_INCARANDSHOOTING_ALPHA_STICK_RATE*(Y_Targetting - 0.9f)*CTimer::GetTimeStep();	Y_Targetting = 0.9f; }
	if (Y_Targetting < -0.9f) { Alpha -= TWOPLAYER_INCARANDSHOOTING_ALPHA_STICK_RATE*(Y_Targetting + 0.9f)*CTimer::GetTimeStep();	Y_Targetting = -0.9f; }

	if(CMaths::Abs(fStickX) < 1.0f && CMaths::Abs(fStickY) < 1.0f && pLockOnTarget==NULL)
	{
		#define YPOS (-0.4f)
		float	deltaX = X_Targetting;
		float	deltaY = Y_Targetting - YPOS;
		float	Diff = CMaths::Sqrt(deltaX * deltaX + deltaY * deltaY);
		float	Step = CTimer::GetTimeStep() * 0.002f;	// was 0.005f

		if (Diff < Step)
		{
			X_Targetting = 0.0f;
			Y_Targetting = YPOS;
		}
		else
		{
			X_Targetting -= deltaX * Step / Diff;
			Y_Targetting -= deltaY * Step / Diff;
		}
	}

//	if (X_Targetting < 0.0f) X_Targetting = MIN(0.0f, X_Targetting + CTimer::GetTimeStep() * 0.004f);
//	else if (X_Targetting > 0.0f) X_Targetting = MAX(0.0f, X_Targetting - CTimer::GetTimeStep() * 0.004f);
//	if (Y_Targetting < 0.0f) Y_Targetting = MIN(0.0f, Y_Targetting + CTimer::GetTimeStep() * 0.004f);
//	else if (Y_Targetting > 0.0f) Y_Targetting = MAX(0.0f, Y_Targetting - CTimer::GetTimeStep() * 0.004f);

//	Beta_Targeting += CTimer::GetTimeStep() * p2ndPad->GetSteeringLeftRight() / 100000.0f;
//	Beta_Targeting *= CMaths::Pow( 0.99f, CTimer::GetTimeStep());

	float CarHeight = pVehicle->GetColModel().GetBoundBoxMax().z;
	float CarLength = 2.0f*CMaths::Abs(pVehicle->GetColModel().GetBoundBoxMin().y);

	float fTargetAlpha = TWOPLAYER_INCARANDSHOOTING_ALPHA_DEFAULT;
	float fCamDistance = TWOPLAYER_INCARANDSHOOTING_DIST;
	fCamDistance += CarLength;
	
	// use a different offset for full-size heli's (not r/c ones)
	if(pVehicle->GetVehicleAppearance()==APR_HELI && pVehicle->GetStatus()!=STATUS_PLAYER_REMOTE)
		TargetCoors += pVehicle->GetMatrix().GetUp()*fTestShiftHeliCamTarget*CarHeight;
	else
	{
		float fTargetZMod = CarHeight*CARCAM_SET[FOLLOW_CAR_INCAR].fTargetOffsetZ -  CARCAM_SET[FOLLOW_CAR_INCAR].fBaseCamZ;
		if(fTargetZMod > 0.0f)
		{
			TargetCoors.z += fTargetZMod;
			fCamDistance += fTargetZMod;
			
			static float TEST_CAM_ALPHA_RAISE_MULT = 0.3f;
			fTargetAlpha += TEST_CAM_ALPHA_RAISE_MULT*fTargetZMod/fCamDistance;
		}
	}

	CA_MIN_DISTANCE = fCamDistance*0.9f; // (0.9f because it's 2d distance and alpha is 25deg)
	CA_MAX_DISTANCE = fCamDistance + TWOPLAYER_INCARANDSHOOTING_DIST_MAX_ADD;

	VecDistance = Source - TargetCoors ;
	DistMagnitude = CMaths::Sqrt(VecDistance.x*VecDistance.x + VecDistance.y * VecDistance.y);		
	m_fDistanceBeforeChanges = DistMagnitude; //used for fixing the acmnera if it is behind a wall
	if (DistMagnitude<0.002)
	{
		ASSERTMSG(0, "BANGO");
		DistMagnitude=0.002f;
	} 
		 
	Beta = CMaths::ATan2(-(TargetCoors.x - Source.x), TargetCoors.y - Source.y) - HALF_PI;
	//////ORIGINAL CAM ON A STRING
	if (DistMagnitude> CA_MAX_DISTANCE)
	{
		// Suss vector between object and camera 
		// Move Camera 
		Source.x = TargetCoors.x+(VecDistance.x*(CA_MAX_DISTANCE/DistMagnitude));
		Source.y = TargetCoors.y+(VecDistance.y*(CA_MAX_DISTANCE/DistMagnitude));
	}      
	else
	{
	  	// is player too close to the camera 
	  	if (DistMagnitude < CA_MIN_DISTANCE)
	  	{
	     	// Suss vector between object and camera   
	     	// Move Camera 
	 		Source.x = TargetCoors.x+(VecDistance.x*(CA_MIN_DISTANCE/DistMagnitude));
	 		Source.y = TargetCoors.y+(VecDistance.y*(CA_MIN_DISTANCE/DistMagnitude));
	  	}
	}
//	TargetCoors.z+=0.80f;

	if(pVehicle->GetMoveSpeed().MagnitudeSqr() > 0.01f*0.01f)
	{
		float fSpeedAlpha = CMaths::ATan2(pVehicle->GetMoveSpeed().z, pVehicle->GetMoveSpeed().Magnitude2D());
		float fSpeedAlphaMult = TWOPLAYER_INCARANDSHOOTING_ALPHA_SPEED_MULT;
		if(pVehicle->GetVehicleType()==VEHICLE_TYPE_HELI)
			fSpeedAlphaMult = TWOPLAYER_INHELIANDSHOOTING_ALPHA_SPEED_MULT;
			
		fSpeedAlpha *= CMaths::Min(1.0f, fSpeedAlphaMult*(pVehicle->GetMoveSpeed().Magnitude() - 0.01f));

		fTargetAlpha += fSpeedAlpha;
		float fAlphaRate = CMaths::Pow(TWOPLAYER_INCARANDSHOOTING_ALPHA_RATE, CTimer::GetTimeStep());
		Alpha = fAlphaRate*Alpha + (1.0f - fAlphaRate)*fTargetAlpha;
	}

	float fAlphaMin = TWOPLAYER_INCARANDSHOOTING_ALPHA_MIN, fAlphaMax = TWOPLAYER_INCARANDSHOOTING_ALPHA_MAX;
	if(pVehicle->GetVehicleType()==VEHICLE_TYPE_HELI){	fAlphaMin = TWOPLAYER_INHELIANDSHOOTING_ALPHA_MIN; fAlphaMax = TWOPLAYER_INHELIANDSHOOTING_ALPHA_MAX;	}
	
	if(Alpha < fAlphaMin)
		Alpha = fAlphaMin;
	else if(Alpha > fAlphaMax)
		Alpha = fAlphaMax;
	
	Source.z= TargetCoors.z - (CA_MAX_DISTANCE*CMaths::Sin(Alpha));
	RotCamIfInFrontCar(TargetCoors, TargetOrientation);
	m_cvecTargetCoorsForFudgeInter=TargetCoors;
	CVector TempSource=Source;
	TheCamera.AvoidTheGeometry(TempSource,m_cvecTargetCoorsForFudgeInter, Source, FOV);		

	// The 2nd player can rotate the camera. This is handled here
	float	DiffX, DiffY, NewDiffX, NewDiffY;
	DiffX = Source.x - TargetCoors.x;
	DiffY = Source.y - TargetCoors.y;
	NewDiffX = DiffX * CMaths::Cos(RotateCam) + DiffY * CMaths::Sin(RotateCam);
	NewDiffY = DiffY * CMaths::Cos(RotateCam) - DiffX * CMaths::Sin(RotateCam);
	Source.x = TargetCoors.x + NewDiffX;
	Source.y = TargetCoors.y + NewDiffY;

	Front = TargetCoors - Source;
	ResetStatics=false;
	GetVectorsReadyForRW();

	if(!pVehicle->CanPedLeanOut(p2ndPed))
	{
#ifdef USE_AUDIOENGINE
		p2ndPed->GetWeapon()->Update(p2ndPed);
#else //USE_AUDIOENGINE
		p2ndPed->GetWeapon()->Update(pDriver->AudioHandle, p2ndPed);
#endif //USE_AUDIOENGINE

//		p2ndPed->GetWeapon()->Reload();
	}

	// Fire the pistol.
	if (p2ndPad->GetCarGunFired())
	{
		// Shake the camera a bit.
		static float gCam2PlayerGunFireShake = 0.03f;

		if(!pVehicle->CanPedLeanOut(p2ndPed) && !p2ndPed->GetWeapon()->IsTypeMelee()
		&& p2ndPed->GetWeapon()->GetWeaponState()==WEAPONSTATE_READY)
		{
			// Are we targetting an entity at the moment?
			CVector	TargetCoors;
			//#define SHOTDIST (200.0f)
			if (pLockOnTarget)
			{
				TargetCoors = pLockOnTarget->GetPosition();
			}
			else
			{
				CVector TempRight = CrossProduct( Front, Up );

				TargetCoors = Front + TempRight * X_Targetting * CMaths::Tan(FOV*(PI/180.0f)*0.5f) - Up * (Y_Targetting * CMaths::Tan(FOV*(PI/180.0f)*0.5f) / CDraw::GetAspectRatio());

				TargetCoors *= MAX_TARGET_WEAPON_RANGE_MULTIPLIER*pWeaponInfo->GetWeaponRange();
				TargetCoors += Source;
			}
			
			bool bRhsSeat = true;
			if(p2ndPed == pVehicle->pPassengers[1])
				bRhsSeat = false;
			
			float fCameraAngle = CMaths::ATan2(-(TargetCoors - Source).x, (TargetCoors - Source).y);
			fCameraAngle -= pVehicle->GetHeading();
			if(fCameraAngle > PI)	fCameraAngle -= TWO_PI;
			else if(fCameraAngle < -PI)	fCameraAngle += TWO_PI;
		
			float fAngle = fCameraAngle + QUARTER_PI;
			if(fAngle < 0.0f)	fAngle += TWO_PI;
			int nChosenFireDirection = int(fAngle / HALF_PI);
			
			CTaskSimpleGangDriveBy tempTask(NULL, NULL, 100.0f, 100, DRIVEBY_AI_ALL_DIRN, bRhsSeat);
			tempTask.SetUpForCameraFire(CWeaponInfo::GetWeaponInfo(p2ndPed->GetWeapon()->GetWeaponType(), p2ndPed->GetWeaponSkill()), nChosenFireDirection);
			tempTask.FireGun(p2ndPed);

			CamShakeNoPos(&TheCamera, gCam2PlayerGunFireShake);
			//p2ndPed->GetWeapon()->FireInstantHitFromCar2(TheCamera.GetPosition(), TargetCoors, pVehicle, p2ndPed);
			//CWeapon weapon(WEAPONTYPE_PISTOL, 1000);
			//weapon.FireInstantHitFromCar2(TheCamera.GetPosition(), TargetCoors, pVehicle, p2ndPed);
		}
	}

}

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Cam_TwoPlayer_Separate_Cars()
// PURPOSE    : 
// RETURNS    : nowt
// PARAMETERS :
//			  : Keep both cars in view.
///////////////////////////////////////////////////////////////////////////

void CCam::Process_Cam_TwoPlayer_Separate_Cars()
{
	CVector	Source0, Source1, Front0, Front1, InterSide;

	ASSERT(CWorld::Players[0].pPed);
	ASSERT(CWorld::Players[1].pPed);
	ASSERT(CWorld::Players[0].pPed->m_pMyVehicle);
	ASSERT(CWorld::Players[1].pPed->m_pMyVehicle);
	FOV = 80;
	// This camera will look from the main car to the second car. The main car will be the one that
	// is always closest to the camera though.
	CVehicle	*pCar0 = CWorld::Players[0].pPed->m_pMyVehicle;
	CVehicle	*pCar1 = CWorld::Players[1].pPed->m_pMyVehicle;
	
	CVector		VecTo2ndCar = pCar1->GetPosition() - pCar0->GetPosition();
	VecTo2ndCar.Normalise();
	
	Source0 = pCar0->GetPosition();
	Source0.z += CModelInfo::GetBoundingBox(pCar0->GetModelIndex()).GetBoundBoxMax().z + 1.0f;
	Source0 -= VecTo2ndCar * 6.0f;

	Source1 = pCar1->GetPosition();
	Source1.z += CModelInfo::GetBoundingBox(pCar1->GetModelIndex()).GetBoundBoxMax().z + 1.0f;
	Source1 += VecTo2ndCar * 6.0f;

	Front0 = pCar1->GetPosition() - Source0;
	Front0.Normalise();

	Front1 = pCar0->GetPosition() - Source1;
	Front1.Normalise();

	// Identify side vector for interpolation.
	InterSide.x = Source0.y - Source1.y;
	InterSide.y = Source1.x - Source0.x;
	InterSide.z = 0.0f;
	InterSide.Normalise();
	InterSide.z = -0.1f;		// Look down a wee bit.
	InterSide.Normalise();

	float	InterVal = CMaths::Sin(CarWeAreFocussingOnI * PI);
	float	InterVal2 = 0.5f + 0.5f * CMaths::Cos(CarWeAreFocussingOnI * PI);

	float	DistApart = (pCar0->GetPosition() - pCar1->GetPosition()).Magnitude();
	Source = Source0 * InterVal2 + Source1 * (1.0f - InterVal2) - InterSide * InterVal * DistApart * 0.75f;
	Front = (Front0 * InterVal2 + Front1 * (1.0f - InterVal2)) * (1.0f - InterVal) + InterSide * InterVal;
	Front.Normalise();

	if (CarWeAreFocussingOn)
	{
		m_cvecTargetCoorsForFudgeInter = pCar0->GetPosition();
	}
	else
	{
		m_cvecTargetCoorsForFudgeInter = pCar1->GetPosition();
	}

	Up=CVector(0.0f, 0.0f, 1.0f);
	Up.Normalise();
	CVector TempRight = CrossProduct( Up, Front );
	TempRight.Normalise();
	Up=CrossProduct( Front, TempRight );
	Up.Normalise();

	// The camera can jump from one car to the other if we're looking backwards at too high a speed.
	CVehicle	*pMainCar = CWorld::Players[CarWeAreFocussingOn].pPed->m_pMyVehicle;
	CVehicle	*p2ndCar = CWorld::Players[(CarWeAreFocussingOn+1)&1].pPed->m_pMyVehicle;
	CVector	MainSpeed = pMainCar->GetMoveSpeed(); //(pMainCar->GetMoveSpeed() + p2ndCar->GetMoveSpeed()) * 0.5f;
	CVector	FrontHor = Front;
	FrontHor.z = 0.0f;
	FrontHor.Normalise();
	float	DotPr = DotProduct(MainSpeed, FrontHor);
	if (DotPr < -0.13f)
	{		// Swap the camera round
		// Only if the other guy wouldn't have the same problem though.
		CVector	OtherSpeed = p2ndCar->GetMoveSpeed();
		FrontHor = -FrontHor;
		float	DotPr2 = DotProduct(OtherSpeed, FrontHor);
		if (DotPr < DotPr2)
		{
			CarWeAreFocussingOn = (CarWeAreFocussingOn+1)&1;
		}
	}
	
static float	TimeS = 0.04f;
	if (CarWeAreFocussingOn)
	{
		CarWeAreFocussingOnI = MIN(CarWeAreFocussingOnI + CTimer::GetTimeStep() * TimeS, 1.0f);
	}
	else
	{
		CarWeAreFocussingOnI = MAX(CarWeAreFocussingOnI - CTimer::GetTimeStep() * TimeS, 0.0f);
	}
}



///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Cam_TwoPlayer_Separate_Cars_TopDown()
// PURPOSE    : 
// RETURNS    : nowt
// PARAMETERS :
//			  : Keep both cars in view.
///////////////////////////////////////////////////////////////////////////

void CCam::Process_Cam_TwoPlayer_Separate_Cars_TopDown()
{
	CVector	TopDownVec;

	ASSERT(CWorld::Players[0].pPed);
	ASSERT(CWorld::Players[1].pPed);
//	ASSERT(CWorld::Players[0].pPed->m_pMyVehicle);
//	ASSERT(CWorld::Players[1].pPed->m_pMyVehicle);
	FOV = 80;
	// This camera will look top down from a point in the middle of the two cars.
	CPhysical	*pCar0, *pCar1;
	
	if (CWorld::Players[0].pPed->m_pMyVehicle)
	{
		pCar0 = CWorld::Players[0].pPed->m_pMyVehicle;
	}
	else
	{
		pCar0 = CWorld::Players[0].pPed;
	}

	if (CWorld::Players[1].pPed->m_pMyVehicle)
	{
		pCar1 = CWorld::Players[1].pPed->m_pMyVehicle;
	}
	else
	{
		pCar1 = CWorld::Players[1].pPed;
	}
	
	
	CVector		LookAtCoors = (pCar0->GetPosition() + pCar1->GetPosition()) * 0.5f;
	float		DistApart = (pCar0->GetPosition() - pCar1->GetPosition()).Magnitude();	
const float MINHEIGHT = 30.0f;
	float		CameraHeight, WantedCameraHeightMultiplier;
	
	if (CWorld::Players[0].pPed->m_pMyVehicle && CWorld::Players[1].pPed->m_pMyVehicle)
	{		// Both in their cars
		WantedCameraHeightMultiplier = 1.0f;
	}
	else if ((!CWorld::Players[0].pPed->m_pMyVehicle) && (!CWorld::Players[1].pPed->m_pMyVehicle))
	{		// Both on foot
		WantedCameraHeightMultiplier = 0.45f;
	}
	else
	{		// One on foot one in car
		WantedCameraHeightMultiplier = 0.75f;
	}
	
	float HeightMultDiff = WantedCameraHeightMultiplier - m_fCameraHeightMultiplier;
	float MaxChange = CTimer::GetTimeStep() * 0.005f;
	if (ABS(HeightMultDiff) < MaxChange)
	{
		m_fCameraHeightMultiplier = WantedCameraHeightMultiplier;
	}
	else if (HeightMultDiff < 0.0f)
	{
		m_fCameraHeightMultiplier -= MaxChange;
	}
	else
	{
		m_fCameraHeightMultiplier += MaxChange;
	}
	
	
	CameraHeight = MAX(MINHEIGHT, DistApart + 10.0f);	// This could perhaps be smoothed out a bit.
	CameraHeight *= m_fCameraHeightMultiplier;

	// Camera being straight down looks a bit disorientating. Give it a slight angle
//	#define TOPDOWNANGLE (0.4f)
static float TOPDOWNANGLE = 0.4f;
	TopDownVec = CVector (0.0f, CMaths::Sin(TOPDOWNANGLE), -CMaths::Cos(TOPDOWNANGLE)); 
	Source = LookAtCoors - (CameraHeight * TopDownVec);
	Front = TopDownVec;
	Up = CVector(0.0f, -TopDownVec.z, TopDownVec.y);
	
	m_cvecTargetCoorsForFudgeInter = Source;

}

CEntity *CCam::Get_TwoPlayer_AimVector(CVector &vecFront)
{
	// Are we targetting an entity at the moment?
	CPed *p2ndPed = CWorld::Players[0].pPed;
	if(CWorld::Players[0].pPed->m_pMyVehicle && CWorld::Players[0].pPed->m_pMyVehicle->pDriver!=p2ndPed)
		p2ndPed = CWorld::Players[1].pPed;

	CWeaponInfo *pWeaponInfo = CWeaponInfo::GetWeaponInfo(p2ndPed->GetWeapon()->GetWeaponType(), p2ndPed->GetWeaponSkill());
	CEntity *pLockOnTarget = CWeapon::FindNearestTargetEntityWithScreenCoors(X_Targetting, Y_Targetting, INCAR_2PLAYER_WEAPON_RANGE_MULTIPLIER*pWeaponInfo->GetWeaponRange(), p2ndPed->GetPosition());

//	CEntity *pLockOnTarget = CWeapon::FindNearestTargetEntityWithScreenCoors(X_Targetting, Y_Targetting);

	if(pLockOnTarget)
	{
		vecFront = pLockOnTarget->GetPosition() - Source;
	}
	else
	{
		CVector TempRight = CrossProduct( Front, Up );
		vecFront = Front + TempRight * X_Targetting * CMaths::Tan(FOV*(PI/180.0f)*0.5f) - Up * (Y_Targetting * CMaths::Tan(FOV*(PI/180.0f)*0.5f) / CDraw::GetAspectRatio());
	}
	vecFront.Normalise();

	return pLockOnTarget;
}


///////////////////////////////////////////////////////////////////////////
// NAME       : RotCamIfInFrontCar(CVector TargetCoors )
// PURPOSE    : If the Camera is within a certain range and the car is accelerating forward
//			  : then we have to rotate
// RETURNS    : True if had to fix false is it didin't
// PARAMETERS : The TargetCoors of the car 
///////////////////////////////////////////////////////////////////////////


bool CCam::RotCamIfInFrontCar(const CVector &TargetCoors, float TargetOrientation)
{
	Bool GoingForward=FALSE;
	float AcceptableRange=0.0f; 
	float DistMagnitude=0.0f, DeltaBeta=0.0f, ReqSpeed=0.0f;
	float ForwardResult=0.0f;
	float BetaTopSpeed= 0.15f;
	float BetaSpeedStep= 0.007f;
	CVector VecDistance, FrontVect;
	AcceptableRange=DEGTORAD(160); 

	ASSERT(CamTargetEntity->GetIsTypeVehicle())
	if(!CamTargetEntity->GetIsTypeVehicle())
		return false;

	CVehicle *pVehicle = (CVehicle *)CamTargetEntity;
	ForwardResult=DotProduct(CamTargetEntity->GetMatrix().GetForward(), pVehicle->GetMoveSpeed());
	if (ForwardResult>0.1f)//0.02f)//0.03f)//we are going forward 
	{
		GoingForward=TRUE;
	}
	
//	if(pVehicle->GetVehicleType()==VEHICLE_TYPE_HELI || pVehicle->GetVehicleType()==VEHICLE_TYPE_PLANE)
	{
		AcceptableRange=DEGTORAD(160.0f);
		
		BetaTopSpeed= 0.10f;
		BetaSpeedStep= 0.003f;
		
		CVector HeliDir = pVehicle->GetMoveSpeed();
		HeliDir.z = 0;
		if(HeliDir.MagnitudeSqr() > 0.06f*0.06f)
		{
			TargetOrientation = CMaths::ATan2(-HeliDir.x, HeliDir.y) - HALF_PI;
		}
	}

	//need to work out the distance
	VecDistance = Source - TargetCoors ;
	DistMagnitude=CMaths::Sqrt(VecDistance.x*VecDistance.x + VecDistance.y * VecDistance.y);		

	DeltaBeta = TargetOrientation - Beta;

	while (DeltaBeta > PI) DeltaBeta -= TWO_PI;
	while (DeltaBeta < -PI) DeltaBeta += TWO_PI;
		
	//Need to work out if we need to switch 	

	if(CMaths::Abs(DeltaBeta) > (PI - AcceptableRange) && GoingForward)
	{
		//CDebug::PrintToScreenCoors("Fixing", 3,14);
		if (TheCamera.m_uiTransitionState==TheCamera.TRANS_NONE)
		{
			m_bFixingBeta=TRUE;
		}
	}        	

	//if we have just swtiched from looking behind, right or left we want to wap the camera 
	//right behind the car.	
	if (!((CPad::GetPad(0)->GetLookBehindForCar())||(CPad::GetPad(0)->GetLookBehindForPed())||(CPad::GetPad(0)->GetLookLeft())||(CPad::GetPad(0)->GetLookRight())))
	{
		if (DirectionWasLooking!=LOOKING_FORWARD)
		//if (LastCalled[ArrayMarker]!=LOOKING_FORWARD)
		{
			TheCamera.m_bCamDirectlyBehind=true;
		}
	}


	if (((m_bFixingBeta==FALSE)&&(TheCamera.m_bUseTransitionBeta==false))&&((TheCamera.m_bCamDirectlyBehind==false)&&(TheCamera.m_bCamDirectlyInFront==false)))
	{
		return false;	 
	}

	bool GotToRotateAndIsSafe=false;
	
	if ((TheCamera.m_bCamDirectlyBehind==true)||(TheCamera.m_bCamDirectlyInFront==true)||(TheCamera.m_bUseTransitionBeta==true))
	{
		if ((&(TheCamera.Cams[TheCamera.ActiveCam]))==this)
		{
			GotToRotateAndIsSafe=true;
		}
	}
	if ((m_bFixingBeta)||(GotToRotateAndIsSafe))//need to get to the target orientation quickly
	{		
		WellBufferMe(TargetOrientation, &Beta, &BetaSpeed, BetaTopSpeed,BetaSpeedStep, true);	
		
		if ((TheCamera.m_bCamDirectlyBehind==true)&&(&TheCamera.Cams[TheCamera.ActiveCam]==this))
		{
		 	Beta = TargetOrientation;
		}
		
		if ((TheCamera.m_bCamDirectlyInFront==true)&&(&TheCamera.Cams[TheCamera.ActiveCam]==this))
		{
		 	Beta = TargetOrientation + PI;
		}
		
		///Mark delete this when they decide that they don't like the get into car view
		
		if ((TheCamera.m_bUseTransitionBeta)&&(&TheCamera.Cams[TheCamera.ActiveCam]==this))
		{
			Beta = m_fTransitionBeta;
		}	
	
		FrontVect.x = DistMagnitude * -CMaths::Cos(Beta);  //* (15.0f - SpeedVar * 3.0f) * pow(0.8f, m_fCarZoomSmoothed);
		FrontVect.y = DistMagnitude * -CMaths::Sin(Beta); //*  (15.0f - SpeedVar * 3.0f) * pow(0.8f, m_fCarZoomSmoothed);  

		Source.x = TargetCoors.x - FrontVect.x;
		Source.y = TargetCoors.y - FrontVect.y;
		//need to see if we are in range 

		DeltaBeta= TargetOrientation - Beta;

		while (DeltaBeta > PI) DeltaBeta -= (2.0f*PI);
		while (DeltaBeta < -PI) DeltaBeta += (2.0f*PI);

		//happy when within 2 degrees of beta
		if (CMaths::Abs(DeltaBeta)<DEGTORAD(2))
		{
			m_bFixingBeta=FALSE;
		} 
	}
	
	
	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;
	return true;
}

/*
void CCam::SetFirstPersonRunAboutModesActive(bool ActiveOrNot)
{
	m_bFirstPersonRunAboutActive=ActiveOrNot;
}
*/
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////


/*
if ((Mode==MODE_CAM_ON_A_STRING)&&(CamTargetEntity->GetIsTypeVehicle()))
{
CVector BehindVect;
BehindVect = CVector( -CMaths::Sin(Beta + DEGTORAD(145)) * CMaths::Cos(m_fIdealAlpha + Alpha), -CMaths::Cos(Beta + DEGTORAD(145)) * CMaths::Cos(m_fIdealAlpha + Alpha ), CMaths::Sin(m_fIdealAlpha + Alpha));
Source = TargetCoors + (0.5f * BehindVect * (15.0f - SpeedVar * 3.0f) * pow(0.8f, m_fCarZoomSmoothed));
	
Front = CWorld::Players[CWorld::PlayerInFocus].pVehicle->GetPosition() - Source;
}

GetVectorsReadyForRW();
*/


/////////////////////////////////////////////////////////////////
// NAME       : Using3rdPersonMouseCam
// PURPOSE    : Is 3rd person mouse mode enabled, and is that 
//				camera mode currently being used
/////////////////////////////////////////////////////////////////

bool CCam::Using3rdPersonMouseCam(void)
{
	if( TheCamera.m_bUseMouse3rdPerson && ( Mode==MODE_FOLLOWPED
//	|| (TheCamera.m_bPlayerIsInGarage && FindPlayerPed() && FindPlayerPed()->GetPedState()!=PED_DRIVING
//	&& Mode!=MODE_TOPDOWN && CamTargetEntity==FindPlayerPed())
	))
		return true;
	
	return false;
}

//
// name:		CCamera
// description:	Constructor
CCamera::CCamera()
{
	m_CurShakeCam = 1;
	m_bMusicFadedOut = FALSE;
	m_pMat = &m_cameraMatrix;
	ResetDuckingSystem(NULL);
	InitialiseScriptableComponents(); // - DW - scriptable doo dah.
}

//
// name:		~CCamera
// description:	Destructor
CCamera::~CCamera() 
{
	// Set this to NULL otherwise CPlaceable's destructor will try and delete the matrix
	m_pMat = NULL;
}

//
//
//
float FovTweakValue=1.0f;
float fAvoidProbTimerDamp = 0.90f;
float fAvoidTweakFOV = 1.15f;
//
bool bAvoidTest1 = false;
bool bAvoidTest2 = false;
bool bAvoidTest3 = false;
//
void CCamera::AvoidTheGeometry(const CVector &TheCamPos,const CVector &TheTargetPos, CVector &ResultantCameraPos,  float fFOV)
{
//#define DW_REPLACE_OLD_CLUNKY_COLLISION_DETECTION_WITH_MINE
#ifdef DW_REPLACE_OLD_CLUNKY_COLLISION_DETECTION_WITH_MINE

	CVector cam = TheCamPos;
	CVector targ = TheTargetPos;

	if (TheCamera.CameraColDetAndReact(&cam, &targ))
	{
		ResultantCameraPos = TheCamera.Cams[TheCamera.ActiveCam].Source;
	}
//	ASSERT(false);
#else

	float TempBeta=0.0f;
	float TempAlpha=0.0f; 
	float fDefaultDistFromPlayer=0.0f;
	float fGroundDistance=0.0f;
	
	
	CVector PlayerToCam=TheTargetPos-TheCamPos;
	m_vecClearGeometryVec=CVector(0.0f, 0.0f, 0.0f);

	fDefaultDistFromPlayer=PlayerToCam.Magnitude();
	fGroundDistance=PlayerToCam.Magnitude2D();
	
	if (!((PlayerToCam.x==0.0f) && (PlayerToCam.y==0.0f)))	
	{
		TempBeta=CGeneral::GetATanOfXY(PlayerToCam.x, PlayerToCam.y);
	}
	else
	{
		//this should never happen
		TempBeta=CGeneral::GetATanOfXY(GetMatrix().xy, GetMatrix().yy);
	}
	
	if (!((fGroundDistance==0.0f)&&(PlayerToCam.z==0.0f)))
	{
		TempAlpha=CGeneral::GetATanOfXY(fGroundDistance, PlayerToCam.z);
	}
	
	
	CVector Front = CVector(CMaths::Cos(TempBeta) *CMaths::Cos(TempAlpha) , CMaths::Sin(TempBeta)*CMaths::Cos(TempAlpha), CMaths::Sin(TempAlpha));
	ResultantCameraPos = TheTargetPos - fDefaultDistFromPlayer * Front;
	Front.Normalise();	
	
	
	
	CColPoint colPoint;
	CEntity *pHitEntity = NULL;
	
	// MAKE SURE CWorld::pIgnoreEntity IS CLEARED AGAIN ASAP
	CWorld::pIgnoreEntity = (CEntity *) pTargetEntity;//so we don't find the player
	// if we're not going to use the results from peds/cars why bother checking them?
	if(CWorld::ProcessLineOfSight(TheTargetPos, ResultantCameraPos, colPoint, pHitEntity, true, false, false, true, false,  false, true))
	{
		CVector ColPointTagToCam=colPoint.GetPosition();	
		{
			ResultantCameraPos=colPoint.GetPosition();
		}
		
if(bAvoidTest1)
{
		//check if the other direction to see if they would come through our clipping plane
		//now enable checking for the player
		if(CWorld::ProcessLineOfSight(ResultantCameraPos, TheTargetPos, colPoint, pHitEntity, false, true, true, true, false,  false, true))
		{
			if (((ResultantCameraPos-colPoint.GetPosition()).Magnitude())<RwCameraGetNearClipPlane(Scene.camera))
			{
				//go to the original position
				ResultantCameraPos=colPoint.GetPosition();
			}
			else
			{
				//see if we need to go to the origanl position only if we would clip through it
				if (((ResultantCameraPos-ColPointTagToCam).Magnitude())<RwCameraGetNearClipPlane(Scene.camera))
				{
					ResultantCameraPos=ColPointTagToCam;
				}
			}
		}
}
	}
	CWorld::pIgnoreEntity = NULL;

	PlayerToCam=TheTargetPos-ResultantCameraPos;
	float PlayCamDist=PlayerToCam.Magnitude();
	if (FindPlayerPed())
	{
		float PlayCamDistWithCol=PlayCamDist-fRangePlayerRadius;//(FindPlayerPed()->GetBoundRadius());			
		if (PlayCamDistWithCol<RwCameraGetNearClipPlane(Scene.camera))	
		{
			RwCameraSetNearClipPlane(Scene.camera, MAX(PlayCamDistWithCol, fCloseNearClipLimit));
		}	
	}
	
	int32 nCountLoop = 0;
	CVector vecTest;
	float fTanFOV = CMaths::Tan(0.5f*DEGTORAD(fFOV));
	if(FrontEndMenuManager.m_PrefsUseWideScreen)
	    fTanFOV *= fAvoidTweakFOV*CDraw::GetAspectRatio(); // ASPECTRATIO.
	else
	    fTanFOV *= fAvoidTweakFOV*CDraw::GetAspectRatio();

	float fNearClip = RwCameraGetNearClipPlane(Scene.camera);
	float fNearClipWidth = fNearClip * fTanFOV;

	CVector CenterOfCircle = ResultantCameraPos + fNearClip*Front;
	CEntity *pFindEntity = CWorld::TestSphereAgainstWorld(CenterOfCircle, fNearClipWidth, NULL, true, false, false, true, false, true);
	float AvoidTheWallTargetFraction=0.0f;
	if (pFindEntity)
	{
		CVector CentreToCol=gaTempSphereColPoints[0].GetPosition()-CenterOfCircle;
		CVector CameraSphereEdgeToCol = gaTempSphereColPoints[0].GetPosition() - ResultantCameraPos; 
		
		float NearClipDepthForNoClip=DotProduct(CameraSphereEdgeToCol, Front);
	
		if ((NearClipDepthForNoClip>fCloseNearClipLimit)&&(NearClipDepthForNoClip<0.90f))
		{
			//we can set the near clip
			if (NearClipDepthForNoClip<RwCameraGetNearClipPlane(Scene.camera))	
			{
				RwCameraSetNearClipPlane(Scene.camera, NearClipDepthForNoClip); 
			}
		}
		else if (NearClipDepthForNoClip<fCloseNearClipLimit)
		{
			RwCameraSetNearClipPlane(Scene.camera, fCloseNearClipLimit); 
		}
		
		//now move the camera for no clipping
		//o.k know there is probably an easier way to do this but I don't know it
		float DistOuterSphereToColPoint=fNearClipWidth-CentreToCol.Magnitude();
		CentreToCol.Normalise();
		CVector ColNormal=gaTempSphereColPoints[0].GetNormal();
		ColNormal.Normalise();
		CVector ColPointSphereOuter= DistOuterSphereToColPoint*CentreToCol;
		
		//Center To col no facing toward the camera.
		if(DotProduct(ColNormal, -CentreToCol) < 0.0f)
		{
			ColNormal=-ColNormal;
//			ColPointSphereOuter = -(2.0f*fNearClipWidth - DistOuterSphereToColPoint)*CentreToCol;
		}

		m_vecClearGeometryVec=ColNormal * DotProduct(ColNormal, -ColPointSphereOuter);	
		AvoidTheWallTargetFraction=1.0f;
		
		if(pTargetEntity && pTargetEntity->GetIsTypePed() && RwCameraGetNearClipPlane(Scene.camera) < 2.0f*fCloseNearClipLimit)
		{
			// is target orientation directly behind or ahead of player (positive==behind)
			if(DotProduct(ColNormal, pTargetEntity->GetMatrix().GetForward()) < 0.0f)
			{
				if(m_fAvoidTheGeometryProbsTimer < 0.0f)	m_fAvoidTheGeometryProbsTimer=0.0f;
				m_fAvoidTheGeometryProbsTimer += CTimer::GetTimeStep();
			}
			else if(DotProduct(ColNormal, pTargetEntity->GetMatrix().GetForward()) > 0.5f)
			{
				if(m_fAvoidTheGeometryProbsTimer > 0.0f)	m_fAvoidTheGeometryProbsTimer=0.0f;
				m_fAvoidTheGeometryProbsTimer -= CTimer::GetTimeStep();
			}
			// what is the prefered direction of rotation
			if( m_nAvoidTheGeometryProbsDirn != 0)
				;// don't change chosen direction until it's been completed
			else if(CrossProduct( pTargetEntity->GetPosition() - ResultantCameraPos, ColNormal).z > 0.0f)
				m_nAvoidTheGeometryProbsDirn = -1;
			else
				m_nAvoidTheGeometryProbsDirn = 1;
		}
	}
	
	m_fAvoidTheGeometryProbsTimer *= CMaths::Pow(fAvoidProbTimerDamp, CTimer::GetTimeStep());
	
	WellBufferMe(AvoidTheWallTargetFraction, &AvoidTheWallsFraction, &AvoidTheWallsFractionSpeed, 0.2f, 0.05f);	
	m_vecClearGeometryVec=AvoidTheWallsFraction* m_vecClearGeometryVec;
	m_bMoveCamToAvoidGeom=true;
#endif	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : CamControl()
// PURPOSE    : Finds out what mode to use at this point (taking into account
//				ped/car issue and modes requested by script.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
static int CamModeToRestore = -1;	

void CCamera::CamControl()
{
	static		Int16	GameMode;
	static 		Int16	ReqMode, ReqMinZoom, ReqMaxZoom;
	static 		int 	LastPedState; //using for working out when the ped has just been arrested  
	static 		int 	ThePickedArrestMode;
	static bool PlaceForFixedWhenSniperFound=false;
	//Some camera Modes require a jump cut
	bool NeedToDoAJumpCutForGameCam=false;
	bool TargetIsBoat=FALSE;
	bool IsInArrestCam=false;
	bool PretendPlayerInAGarage=false; //special hack for levl designers
	CVector PlayerFrontVector;
 	CVector CenterOfGarage;//find center of garage
	CVector CenterOfDoorOne;
	CVector CenterOfDoorTwo;
	CVector CenterToActiveDoor;
	CVector PlayerPosition;
	CVector DoorFront;
	CVector TheCamPosition;
	CEntity *pDoorWeAreUnder=NULL;
	int DoorWeAreUnder=0;
	float DistanceCarToDoorOne=0.0f;
	float DistanceCarToDoorTwo=0.0f;
	float CamDistanceAwayForGarage=13.0f;
	int ModeAtStartOfCamControl=Cams[ActiveCam].Mode;
	m_bObbeCinematicPedCamOn=false;
	m_bObbeCinematicCarCamOn=false;
	m_bUseTransitionBeta=FALSE;		
	m_bUseSpecialFovTrain=false;
	m_bJustCameOutOfGarage=false;
	m_bTargetJustCameOffTrain=false;
	m_bInATunnelAndABigVehicle=false;
	m_bJustJumpedOutOf1stPersonBecauseOfTarget = false;
	JustGoneIntoObbeCamera=false;

	if ((Cams[ActiveCam].CamTargetEntity==NULL)&&(pTargetEntity==NULL))
	{
		TIDYREF(pTargetEntity, &pTargetEntity);
		pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed;
		REGREF(pTargetEntity, &pTargetEntity);
	}
	
	ASSERTMSG(pTargetEntity!=NULL, "No Target set yet for camera")
	



	m_iZoneCullFrameNumWereAt++;

	if (m_iZoneCullFrameNumWereAt>m_iCheckCullZoneThisNumFrames)
	{
		m_iZoneCullFrameNumWereAt=1;
	}
	if (m_iZoneCullFrameNumWereAt==m_iCheckCullZoneThisNumFrames)
	{
		m_bCullZoneChecksOn=true;
	}
	else
	{
		m_bCullZoneChecksOn=false;
	}

	if (m_bCullZoneChecksOn==true)
	{
		if (CCullZones::CamCloseInForPlayer())
		{
			m_bFailedCullZoneTestPreviously=true;
		}
		else
		{
			m_bFailedCullZoneTestPreviously=false;
		}
	}


	if (m_bLookingAtPlayer==true)
	{
		CPad::GetPad(0)->EnableControlsCamera();
		FindPlayerPed()->m_nFlags.bIsVisible=true; 
	}

#ifdef GTA_IDLECAM
	if (CanWeBeInIdleMode()==true)
	{
		if(m_bIdleOn==false)
		{
			m_uiTimeWeEnteredIdle=CTimer::GetTimeInMilliseconds();
			NeedToDoAJumpCutForGameCam=true;
		}
		m_bIdleOn=true;
		ReqMode=CCam::MODE_PILLOWS_PAPS;
	}
	else
	{
		m_bIdleOn=false;
	}
#endif

	if (GAMEISNOTPAUSED)  //Want to change the zoom stuff if the game is  not paused 
	{
		if (m_bIdleOn==false)
		{
			float CarTargetCloseHeightOffset=0.0f;
			float PedTargetCloseHeightOffset=0.0f;

			if (m_bTargetJustBeenOnTrain==true)
			{
				bool NeedToRestoreCamera=false;		
				float TrainSpeed=0.0f;
				bool ComingToAStopAStation=true;
			
				if ((pTargetEntity->GetIsTypeVehicle())==false)
				{
					 NeedToRestoreCamera=true;
					
				}
				else //it's a vehicle
				{
					if (((CVehicle*)pTargetEntity)->GetBaseVehicleType()!=VEHICLE_TYPE_TRAIN) 
					{
						 NeedToRestoreCamera=true;
					}
					
				}
			
				if (NeedToRestoreCamera)
				{
					Restore();
					m_bTargetJustCameOffTrain=true;
					m_bTargetJustBeenOnTrain=false;
					SetWideScreenOff();
				}
				
			}

			if (pTargetEntity->GetIsTypeVehicle()) //for vechicle modes
			{	
			
			
				// we might be pending some mode revert - tunnel handling - nasty - DW
				if (CamModeToRestore>0)
				{		
					bool bCollisionCheckIsStillBad = false;
					
					
					
					if (bCollisionCheckIsStillBad)
					{
					}
					else
					{
						ReqMode = CamModeToRestore;
						CamModeToRestore = -1;
					}					
				}			
			
			
				// We're in a car or boat or train
				if (((CVehicle*)pTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_TRAIN)
				{
					ReqMode=CCam::MODE_BEHINDCAR;
//					Process_DWTrain();
#ifdef GTA_TRAIN		
					// Marks old stuff. Not sure how it works.
					if (m_bTargetJustBeenOnTrain==false)
					{
						m_bInitialNodeFound=false;
						m_bInitialNoNodeStaticsSet=false;
					}
					Process_Train_Camera_Control();
#endif // GTA_TRAIN			
				}
				else //o.k we're not in a train
				{
					if (((CVehicle*)pTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
					{
						if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
						{
							TargetIsBoat=TRUE;
						}
					}
						
					if((CPad::GetPad(0)->CycleCameraModeUpJustDown() || CPad::GetPad(0)->CycleCameraModeDownJustDown())
					&& !CReplay::ReplayGoingOn() && !m_WideScreenOn && !m_bFailedCullZoneTestPreviously
					&& (m_bLookingAtPlayer==true || WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL) && !CGameLogic::IsCoopGameGoingOn())
					{
						if(CPad::GetPad(0)->CycleCameraModeUpJustDown())
							m_nCarZoom-=1;
						else
							m_nCarZoom+=1;
				
						if(m_nCarZoom>ZOOM_FIVE)
							m_nCarZoom = ZOOM_ZERO;
						else if(m_nCarZoom<ZOOM_ZERO)
							m_nCarZoom = ZOOM_FIVE;
							
						if(m_nCarZoom==ZOOM_FOUR)
						{
							if(CPad::GetPad(0)->CycleCameraModeUpJustDown())
								m_nCarZoom = ZOOM_THREE;
							else
								m_nCarZoom = ZOOM_FIVE;
						}
						else if(m_nCarZoom==ZOOM_ZERO && m_bDisableFirstPersonInCar)
						{
							if(CPad::GetPad(0)->CycleCameraModeUpJustDown())
								m_nCarZoom = ZOOM_FIVE;
							else
								m_nCarZoom = ZOOM_ONE;
						}
					}
/*				
					if (m_bFailedCullZoneTestPreviously==false)//need to not worry about zoom shit
					{
						if ((CPad::GetPad(0)->CycleCameraModeUpJustDown())&&(!CReplay::ReplayGoingOn())&&((m_bLookingAtPlayer==true)||(WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL))&&(m_WideScreenOn==false))  
						{
							m_nCarZoom-=1;
							//skip over TOP_DOWN
							if (m_nCarZoom==ZOOM_FOUR || 
								(m_nCarZoom==ZOOM_ZERO && m_bDisableFirstPersonInCar))
							{
								m_nCarZoom-=1;
							}
						}
						if ((CPad::GetPad(0)->CycleCameraModeDownJustDown())&&(!CReplay::ReplayGoingOn())&&((m_bLookingAtPlayer==true)||(WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL))&&(m_WideScreenOn==false))  
						{
							m_nCarZoom+=1;
							//skip over TOP_DOWN
							if (m_nCarZoom==ZOOM_FOUR || 
								(m_nCarZoom==ZOOM_ZERO && m_bDisableFirstPersonInCar))
							{
								m_nCarZoom+=1;
							}					
						}
					
						//Hey lets clip
						if  (m_nCarZoom>ZOOM_FIVE) m_nCarZoom-=ZOOM_FIVE+1;
						else if (m_nCarZoom<ZOOM_ZERO)	m_nCarZoom+=ZOOM_FIVE+1;
							//finshed clipping
					}
*/
					//need to not worry about zoom shit
					if(m_bFailedCullZoneTestPreviously && m_nCarZoom!=ZOOM_FOUR && m_nCarZoom!=ZOOM_ZERO)
					{
						ReqMode=CCam::MODE_CAM_ON_A_STRING;
					}	
					int32 iVehicleType=((CVehicle*)pTargetEntity)->GetBaseVehicleType();

					if (iVehicleType==VEHICLE_TYPE_BOAT)
					{
						if ((pTargetEntity->GetModelIndex())==MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
						{
							iVehicleType=VEHICLE_TYPE_CAR;
						}
					}
									
					if(iVehicleType==VEHICLE_TYPE_CAR || iVehicleType==VEHICLE_TYPE_BIKE)
					{
						CAttributeZone* pZoneLikeAGarage = NULL;
						if(iVehicleType==VEHICLE_TYPE_BIKE && CCullZones::CamStairsForPlayer())
						{
							if((pZoneLikeAGarage=CCullZones::FindZoneWithStairsAttributeForPlayer()) !=NULL)
							{
								PretendPlayerInAGarage=true;
							}
						}
					
						if(CGarages::IsPointInAGarageCameraZone(pTargetEntity->GetPosition()) || PretendPlayerInAGarage)
					    {
							CObject *pDoor1, *pDoor2;
							if((m_bGarageFixedCamPositionSet==false && m_bLookingAtPlayer==true) || WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL)
							{
								if(pToGarageWeAreIn!=NULL || pZoneLikeAGarage!=NULL)
								{
									PlayerPosition=pTargetEntity->GetPosition();
								
									if(pToGarageWeAreIn!=NULL)
									{

										pToGarageWeAreIn->FindDoorsWithGarage(&pDoor1, &pDoor2);
									
										//first work out what door we are under
										if 	(pDoor1!=NULL)
										{
											DoorWeAreUnder=1;
											CenterOfDoorOne.x= pDoor1->GetPosition().x;
											CenterOfDoorOne.y= pDoor1->GetPosition().y;
											CenterOfDoorOne.z=0;
											PlayerPosition.z=0;
											DistanceCarToDoorOne=(PlayerPosition - CenterOfDoorOne).Magnitude();
										}
										else if (pDoor2!=NULL)
										{
											if (DoorWeAreUnder==1) //need to see if we are closer to door two
											{
												CenterOfDoorTwo.x=pDoor2->GetPosition().x;
												CenterOfDoorTwo.y=pDoor2->GetPosition().y;
												CenterOfDoorTwo.z=0;
												PlayerPosition.z=0;
												DistanceCarToDoorTwo=(PlayerPosition - CenterOfDoorTwo).Magnitude();
												if (DistanceCarToDoorTwo<DistanceCarToDoorOne)
												{
													DoorWeAreUnder=2;
												}
											}						
											else //door one didn't exist, mustbe door two we are under 
											{
												DoorWeAreUnder=2;
											}
										}
										else //its a garge with no doors
										{
											// DW - this is bad ...maybe... 
											
											#ifndef MASTER
												// debug opportunity to find out why the garage has no doors.
												pToGarageWeAreIn->FindDoorsWithGarage(&pDoor1, &pDoor2);
											#endif
										
										
											//just pretend we are under door one
											DoorWeAreUnder=1;
											CenterOfDoorOne.x=pTargetEntity->GetPosition().x;
											CenterOfDoorOne.y=pTargetEntity->GetPosition().y;
											CenterOfDoorTwo.z=0;							
										}
									}
									else
									{
										ASSERTMSG(PretendPlayerInAGarage, "Cullzone garage stuff gone wrong");
										DoorWeAreUnder=1;
										CenterOfDoorOne=Cams[ActiveCam].Source;
										CenterOfGarage=PlayerPosition;//pZoneLikeAGarage->ZoneDef.FindCenter();
										if((CenterOfGarage - CenterOfDoorOne).Magnitude2D() > 15.0f)
										{
											bool bFoundExitDirn = true;
											CVector vecTestExitDirn = pTargetEntity->GetPosition() - CenterOfGarage;
											vecTestExitDirn.z = 0.0f;
											vecTestExitDirn.Normalise();
											
											float fMaxZoneDim = MAX( ABS(pZoneLikeAGarage->ZoneDef.Vec1X) + ABS(pZoneLikeAGarage->ZoneDef.Vec2X), ABS(pZoneLikeAGarage->ZoneDef.Vec1Y) + ABS(pZoneLikeAGarage->ZoneDef.Vec2Y));
											
											CVector vecTestExitPos = pTargetEntity->GetPosition() + 2.0f*fMaxZoneDim*vecTestExitDirn;
											if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true))
											{
												vecTestExitPos = pTargetEntity->GetPosition() - 2.0f*fMaxZoneDim*vecTestExitDirn;
												if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true))
												{
													bFoundExitDirn = false;	
												}
											}
											
											if(bFoundExitDirn)
												CenterOfDoorOne = vecTestExitPos;
										}
									}
									
									//now find the normal out the way
									if(pToGarageWeAreIn!=NULL)
									{
										CenterOfGarage=CVector((pToGarageWeAreIn->MinX+pToGarageWeAreIn->MaxX)/2, (pToGarageWeAreIn->MinY+pToGarageWeAreIn->MaxY)/2, 0);
									}
									else
									{
										CenterOfDoorOne.z=0.0f;
										if(pZoneLikeAGarage==NULL)
										{
											CenterOfGarage=CVector(pTargetEntity->GetPosition().x, pTargetEntity->GetPosition().y, 0);
										}
									}
									
									if (DoorWeAreUnder==1)
									{
										DoorFront=CenterOfDoorOne-CenterOfGarage;
									}
									else
									{
										ASSERTMSG(DoorWeAreUnder==2, "Get Mark Something worng with garages");
										DoorFront=CenterOfDoorTwo-CenterOfGarage;
									}							
										
									bool GroundFound;
									float GroundZ=0.0f;
									float HeightUp=3.1f;
									PlayerPosition=pTargetEntity->GetPosition();
									GroundZ=CWorld::FindGroundZFor3DCoord(PlayerPosition.x, PlayerPosition.y, PlayerPosition.z, &GroundFound);
									//ASSERTMSG(GroundFound, "Could find the ground are we in the sky?");
									if (GroundFound==false) //this shouldn't be happening but been having
									//lots of complaints about the game hanging so this is the best alternative
									{
										GroundZ=PlayerPosition.z-0.20f;
									}
									DoorFront.z=0;
									DoorFront.Normalise();
//CDebug::DebugLine3D( CenterOfGarage.x, CenterOfGarage.y, PlayerPosition.z ,
//CenterOfDoorOne.x, CenterOfDoorOne.y, PlayerPosition.z ,0xff00ffff, 0xff00ffff);

									if (DoorWeAreUnder==1)
									{
										if(pToGarageWeAreIn==NULL && pZoneLikeAGarage!=NULL)
										{
											CamDistanceAwayForGarage = 3.75f + 0.7f*MAX( ABS(pZoneLikeAGarage->ZoneDef.Vec1X) + ABS(pZoneLikeAGarage->ZoneDef.Vec2X), ABS(pZoneLikeAGarage->ZoneDef.Vec1Y) + ABS(pZoneLikeAGarage->ZoneDef.Vec2Y));
											TheCamPosition = CenterOfGarage + CamDistanceAwayForGarage*DoorFront;
										}
										else
										{
											TheCamPosition=CenterOfDoorOne+(CamDistanceAwayForGarage*DoorFront);
										}
									}
									else
									{
										ASSERT(DoorWeAreUnder==2)
										TheCamPosition=CenterOfDoorTwo+(CamDistanceAwayForGarage*DoorFront);
									}
									TheCamPosition.z=GroundZ+HeightUp;
									
									
									// DW - Right this is a pile of crap, I'm going to set the camera position myself.
									CVector player;
									CVector door1(0,0,0);
									CVector door2(0,0,0);
									CVector directionOfDoor1;
									CVector directionOfDoor2;
									CVector doorDir;
																		
									if (pDoor1)
									{								
										door1 = pDoor1->GetRelatedDummy()->GetPosition();
										directionOfDoor1 = pDoor1->GetRelatedDummy()->GetMatrix().GetRight();
										doorDir = directionOfDoor1;
									}
									
									if (pDoor2)
									{
										door2 = pDoor2->GetRelatedDummy()->GetPosition();
										directionOfDoor2 = pDoor2->GetRelatedDummy()->GetMatrix().GetRight();
										doorDir = directionOfDoor2;									
									}

									CVector posForCamera;
									if (pDoor1)																		
									{
										if (pDoor2)
											posForCamera = (door2-door1)*0.5f + door1;
										else
											posForCamera = door1;
									}
									else if (pDoor2)																		
									{
										posForCamera = door2;
									}
									else
									{
										posForCamera = PlayerPosition;
										doorDir = posForCamera-CenterOfGarage;
										doorDir.z = 0.0f;
										doorDir.Normalise();
//										ASSERT(false); // See me  - DW
									}

									static float gLenFromDoor = -10.0f;
									static float gHeightFromDoor = 2.0f;
									TheCamPosition = posForCamera + doorDir * gLenFromDoor;
									TheCamPosition.z += gHeightFromDoor;
									
									// now we have a dcent position for the camera ray cast to find a safe place to put it.
									
									
									SetCamPositionForFixedMode((TheCamPosition),CVector(0,0,0));
//CDebug::DebugLine3D( CenterOfDoorOne.x, CenterOfDoorOne.y, PlayerPosition.z ,
//TheCamPosition.x, TheCamPosition.y, TheCamPosition.z, 0xff00ffff, 0xff00ffff);

									m_bGarageFixedCamPositionSet=true;							
								}
							}
							
							if((CGarages::CameraShouldBeOutside() || PretendPlayerInAGarage) && m_bGarageFixedCamPositionSet==true && (m_bLookingAtPlayer==true || WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL))
							{
								if(pToGarageWeAreIn!=NULL || pZoneLikeAGarage!=NULL)
								{
									ReqMode=CCam::MODE_FIXED;
									m_bPlayerIsInGarage=true;
								}
							}
							else
							{
								if(m_bPlayerIsInGarage)
								{
									m_bJustCameOutOfGarage=true;
									m_bPlayerIsInGarage=false;
								}
								ReqMode=CCam::MODE_CAM_ON_A_STRING;
							}
						}
						else
						{
							if (m_bPlayerIsInGarage)
							{
								m_bJustCameOutOfGarage=true;
								m_bPlayerIsInGarage=false;
							}
							pDoorWeAreUnder=NULL;
							m_bGarageFixedCamPositionSet=false;
							ReqMode=CCam::MODE_CAM_ON_A_STRING;
						}
					}
					else if(iVehicleType==VEHICLE_TYPE_BOAT)
					{
						ReqMode=CCam::MODE_BEHINDBOAT;
					}

					eVehicleAppearance VehicleApperance=((CVehicle*)(pTargetEntity))->GetVehicleAppearance(); 
	 				int PositionInArray=0;
					GetArrPosForVehicleType(VehicleApperance, PositionInArray);

					
					if ((m_nCarZoom==ZOOM_ZERO)&&(!m_bPlayerIsInGarage))
					{
						m_fCarZoomBase=0.0f;
						ReqMode=CCam::MODE_1STPERSON;
					}
					else if (m_nCarZoom==ZOOM_ONE)
						m_fCarZoomBase = ZOOM_ONE_DISTANCE[PositionInArray];
					else if (m_nCarZoom==ZOOM_TWO)
						m_fCarZoomBase = ZOOM_TWO_DISTANCE[PositionInArray];
					else if (m_nCarZoom==ZOOM_THREE)
						m_fCarZoomBase = ZOOM_THREE_DISTANCE[PositionInArray];
					
					if ((m_nCarZoom==ZOOM_FOUR)&&(!m_bPlayerIsInGarage))
					{
						m_fCarZoomBase=1.0f;
						//ReqMode=CCam::MODE_TOPDOWN;
					}
					// might not need this but I'm worried about starting with a value of zero
					if(m_fCarZoomTotal==0.0f)
						m_fCarZoomTotal = m_fCarZoomBase;
					

/*					if ((iVehicleType==VEHICLE_TYPE_CAR)&&(!m_bPlayerIsInGarage))
					{
						float VehicleHeight = pTargetEntity->GetColModel().GetBoundBoxMax().z
										  	- pTargetEntity->GetColModel().GetBoundBoxMin().z;
						if ((CCullZones::Cam1stPersonForPlayer())&&(VehicleHeight>=HEIGHT_OF_FIRE_ENGINE))
						{
							if (pToGarageWeAreInForHackAvoidFirstPerson==NULL)
							{
								ReqMode=CCam::MODE_1STPERSON;				
								m_bInATunnelAndABigVehicle=true;
							}
						}
					}
*/					
/*					if ((ReqMode==CCam::MODE_TOPDOWN)&&((CCullZones::Cam1stPersonForPlayer())||(CCullZones::CamNoRain())||(CCullZones::PlayerNoRain())))
					{
						ReqMode=CCam::MODE_1STPERSON;
					}
*/					
					//the script can set the zooming value, we only want to use this if the script has
					//taken control of the camera
		
					
					if(m_bUseScriptZoomValueCar==true)
					{
						if(m_fCarZoomSmoothed < m_fCarZoomValueScript)
						{
							m_fCarZoomSmoothed = MIN(m_fCarZoomValueScript, m_fCarZoomSmoothed + CTimer::GetTimeStep()*0.12f);
						}
						else
						{
							m_fCarZoomSmoothed = MAX(m_fCarZoomValueScript, m_fCarZoomSmoothed - CTimer::GetTimeStep()*0.12f);
						}
					}
					else if(m_bFailedCullZoneTestPreviously)
					{
						float ZoomedInVal=-0.65f;
						CarTargetCloseHeightOffset=0.65f;
						if (m_fCarZoomSmoothed < ZoomedInVal)
						{
							m_fCarZoomSmoothed = MIN(ZoomedInVal, m_fCarZoomSmoothed + CTimer::GetTimeStep()*0.12f);
						}
						else
						{
							m_fCarZoomSmoothed = MAX(ZoomedInVal, m_fCarZoomSmoothed - CTimer::GetTimeStep()*0.12f);
						}
					}
					else
					{
						if (m_fCarZoomSmoothed < m_fCarZoomBase)
						{
							m_fCarZoomSmoothed = MIN(m_fCarZoomBase, m_fCarZoomSmoothed + CTimer::GetTimeStep()*0.12f);
						}
						else
						{
							m_fCarZoomSmoothed = MAX(m_fCarZoomBase, m_fCarZoomSmoothed - CTimer::GetTimeStep()*0.12f);
						}
						//this is to indicate that we have just come out of first person mode
						if ((m_nCarZoom==ZOOM_THREE)&&(m_fCarZoomBase==0.0f))	
						{
							m_fCarZoomSmoothed = m_fCarZoomBase;
						}
					}				
					
					// reset total value so it doesn't mess up if next camera process doesn't use it
//					m_fCarZoomTotal = m_fCarZoomBase;

					// right lets do any we off sett type shite
					WellBufferMe(CarTargetCloseHeightOffset, &(Cams[ActiveCam].m_fCloseInCarHeightOffset), &(Cams[ActiveCam].m_fCloseInCarHeightOffsetSpeed), 0.1f, 0.25f);

					//Right if the player is in the water then we have to change the mode to
					//player in water mode

// DW - removed		
/*					if( Cams[ActiveCam].IsTargetInWater(Cams[ActiveCam].Source) && TargetIsBoat==FALSE 
					&& Cams[ActiveCam].CamTargetEntity->GetIsTypePed()==false
					&& pTargetEntity->GetModelIndex()!=MODELID_BOAT_SEAPLANE 
					&& pTargetEntity->GetModelIndex()!=MODELID_HELI_SEASPARROW )
					{
						//N.B the seaplane is classes as a boat but the camera treats it like a car
						ReqMode = CCam::MODE_PLAYER_FALLEN_WATER;
					}
*/					
// DW - end removed		

				}//end of not being in a train	
			}
			else if(pTargetEntity->GetIsTypePed()) // We're on foot
			{		
				//this is the part where we cycle the zoom modes
				ASSERTMSG(pTargetEntity->GetIsTypePed(), "What in gods green earth is going on?"); 
				if((CPad::GetPad(0)->CycleCameraModeUpJustDown() || CPad::GetPad(0)->CycleCameraModeDownJustDown())
				&& !CReplay::ReplayGoingOn() && !m_WideScreenOn && !m_bFailedCullZoneTestPreviously && !m_bFirstPersonBeingUsed
				&& (m_bLookingAtPlayer==true || WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL) && !CGameLogic::IsCoopGameGoingOn())
				{
					if(CPad::GetPad(0)->CycleCameraModeUpJustDown())
						m_nPedZoom-=1;
					else
						m_nPedZoom+=1;
			
					if(m_nPedZoom > ZOOM_THREE)
						m_nPedZoom=ZOOM_ONE;
					else if(m_nPedZoom < ZOOM_ONE)
						m_nPedZoom=ZOOM_THREE;
				}
/*				
				if( CPad::GetPad(0)->CycleCameraModeUpJustDown() && !CReplay::ReplayGoingOn()
				&& (m_bLookingAtPlayer==true || WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL)
				&& m_WideScreenOn==false && m_bFailedCullZoneTestPreviously==false && m_bFirstPersonBeingUsed==false)
				{
					#ifdef GTA_PC
					if (FrontEndMenuManager.m_ControlMethod==STANDARD_CONTROLLER_SCREEN)
					{
						if (m_nPedZoom==ZOOM_THREE)
						{
							m_nPedZoom=ZOOM_ONE;
						}
						else
						{
							m_nPedZoom=ZOOM_THREE;
						}
					}
					else
					#endif
					{
						m_nPedZoom-=1;
					}				
				}
				
				if( CPad::GetPad(0)->CycleCameraModeDownJustDown() && !CReplay::ReplayGoingOn()
				&& (m_bLookingAtPlayer==true || WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL)
				&& m_WideScreenOn==false && m_bFailedCullZoneTestPreviously==false && m_bFirstPersonBeingUsed==false)  
				{
					#ifdef GTA_PC
					if (FrontEndMenuManager.m_ControlMethod==STANDARD_CONTROLLER_SCREEN)
					{
						if (m_nPedZoom==ZOOM_THREE)
						{
							m_nPedZoom=ZOOM_ONE;
						}
						else
						{
							m_nPedZoom=ZOOM_THREE;
						}
					}
					else
					#endif
					{
						m_nPedZoom+=1;
					}	
				}
				
				//hack for standard camera mode
				
				
				//Hey lets clip
				if  (m_nPedZoom>ZOOM_THREE) m_nPedZoom=ZOOM_ONE;
				else if (m_nPedZoom<ZOOM_ONE) m_nPedZoom=ZOOM_THREE;
				//finshed clipping
*/				
				ReqMode = CCam::MODE_FOLLOWPED;
				
				//this is the part where we work out if we want to be in first person mode
				if(((m_bLookingAtPlayer==true)||(m_bEnable1rstPersonCamCntrlsScript))&&(pTargetEntity->GetIsTypePed())&&((m_WideScreenOn==false)||(m_bEnable1rstPersonCamCntrlsScript))
				&& Cams[0].Using3rdPersonMouseCam()==false)
				{
					if(FindPlayerPed()->GetPedIntelligence()->GetTaskSecondaryAttack()==NULL
					&& (CPad::GetPad(0)->LookAroundLeftRight()!=0 || CPad::GetPad(0)->LookAroundUpDown()!=0))
					{
						//the first person viewer is being used
//						m_uiFirstPersonCamLastInputTime=CTimer::GetTimeInMilliseconds();
//						m_bFirstPersonBeingUsed=true;
					}

					//right the viewiing camera is not active so lets see if the time they last did some input
					//is above the timer 
					if(m_bFirstPersonBeingUsed)
					{
						//if the user has indicated that they wanted to move then lets finish first person stuff
						if(CPad::GetPad(0)->GetPedWalkLeftRight() || CPad::GetPad(0)->GetPedWalkUpDown()
						|| CPad::GetPad(0)->GetButtonSquare() || CPad::GetPad(0)->GetButtonTriangle()
						|| CPad::GetPad(0)->GetButtonCross() || CPad::GetPad(0)->GetButtonCircle()
						|| CPad::GetPad(0)->GetSelect())
						{
							m_bFirstPersonBeingUsed=false;
						}
						//they've not indicated they want the ped to move 
						//lets see if they've gone past timer
						else 
						{
							if ((CTimer::GetTimeInMilliseconds()-m_uiFirstPersonCamLastInputTime)>MAX_TIME_IN_FIRST_PERSON_NO_INPUT)
							{
							   	m_bFirstPersonBeingUsed=false;
							}
							else if (CPad::GetPad(0)->TargetJustDown())
							{		// If the player uses the aiming button he wants to get out of first person
							   	m_bFirstPersonBeingUsed=false;
							   	m_bJustJumpedOutOf1stPersonBecauseOfTarget=true;
							}
						}
					}
				}
				else
					m_bFirstPersonBeingUsed=false;

				if (FindPlayerPed()->IsPedInControl()==false || FindPlayerPed()->GetPlayerData()->m_moveBlendRatio > 0.0f)
					m_bFirstPersonBeingUsed=false;				
				
				if (m_bFirstPersonBeingUsed)
				{
					ReqMode=CCam::MODE_1STPERSON;
					CPad::GetPad(0)->DisableControlsCamera();
				}

				switch(m_nPedZoom)
				{
					case ZOOM_ONE:
						m_fPedZoomBase = Cams[ActiveCam].m_fTargetZoomGroundOne;//ZOOM_PED_ONE_DISTANCE;
						break;
					case ZOOM_THREE:
						m_fPedZoomBase = Cams[ActiveCam].m_fTargetZoomGroundThree;//ZOOM_PED_THREE_DISTANCE;
						break;
					case ZOOM_TWO:
					default:
						m_fPedZoomBase = Cams[ActiveCam].m_fTargetZoomGroundTwo;//ZOOM_PED_TWO_DISTANCE;
						break;
				}
				// might not need this but I'm worried about starting with a value of zero
//				if(m_fPedZoomTotal==0.0f)
//					m_fPedZoomTotal = m_fPedZoomBase;
			
				//the script can set the zooming value, we only want to use this if the script has
				//taken control of the camera
				if(m_bUseScriptZoomValuePed==true)
				{
					if(m_fPedZoomSmoothed < m_fPedZoomValueScript)
						m_fPedZoomSmoothed = MIN(m_fPedZoomValueScript,  m_fPedZoomSmoothed + CTimer::GetTimeStep()*0.12f);
					else
						m_fPedZoomSmoothed = MAX(m_fPedZoomValueScript,  m_fPedZoomSmoothed - CTimer::GetTimeStep()*0.12f);
				}
				else if(m_bFailedCullZoneTestPreviously)
				{
					static float PedZoomedInVal=0.5f;
					PedTargetCloseHeightOffset=0.7f;
					if(m_fPedZoomSmoothed < PedZoomedInVal)
						m_fPedZoomSmoothed = MIN(PedZoomedInVal, m_fPedZoomSmoothed + CTimer::GetTimeStep()*0.12f);
					else
						m_fPedZoomSmoothed = MAX(PedZoomedInVal, m_fPedZoomSmoothed - CTimer::GetTimeStep()*0.12f);
				}
				else
				{
					if(m_fPedZoomSmoothed < m_fPedZoomBase)
						m_fPedZoomSmoothed = MIN(m_fPedZoomBase, m_fPedZoomSmoothed + CTimer::GetTimeStep()*0.12f);
					else
						m_fPedZoomSmoothed = MAX(m_fPedZoomBase, m_fPedZoomSmoothed - CTimer::GetTimeStep()*0.12f);
						
					// for when the player has come out of sniperesq mode
					if(m_nPedZoom==ZOOM_THREE && m_fPedZoomBase==0.0f)
					{
						m_fPedZoomSmoothed = m_fPedZoomBase;
					}
				}	

//				// reset total value so it doesn't mess up if next camera process doesn't use it
//				m_fPedZoomTotal = m_fPedZoomBase;

				// right lets do any we off sett type shite
				WellBufferMe(PedTargetCloseHeightOffset, &(Cams[ActiveCam].m_fCloseInPedHeightOffset), &(Cams[ActiveCam].m_fCloseInPedHeightOffsetSpeed), 0.1f, 0.025);

#if 0 // DW - Fight Cam is shite.
				if(!m_bFirstPersonBeingUsed)
				{
					if ((FindPlayerPed()->GetPedState())==PED_FIGHT && !m_bUseMouse3rdPerson)
					{
						ReqMode=CCam::MODE_FIGHT_CAM;
					}

					if ((((CPed*)pTargetEntity)->GetWeapon()->m_eWeaponType)==WEAPONTYPE_BASEBALLBAT)
					{
						if ((FindPlayerPed()->GetPedState())==PED_ATTACK && !m_bUseMouse3rdPerson)
						{
							ReqMode=CCam::MODE_FIGHT_CAM;
						}
					}
				}
#endif				
				
				CAttributeZone* pZoneLikeAGarage = NULL;
				if(CCullZones::CamStairsForPlayer())
				{
					if((pZoneLikeAGarage=CCullZones::FindZoneWithStairsAttributeForPlayer()) !=NULL)
					{
						PretendPlayerInAGarage=true;
					}
				}
// apparently redundant now - PC can run garages as normal.
//#ifdef GTA_PC					
//				if((CGarages::IsPointInAGarageCameraZone(pTargetEntity->GetPosition()) && !m_bUseMouse3rdPerson) || PretendPlayerInAGarage)
//#else
				if(CGarages::IsPointInAGarageCameraZone(pTargetEntity->GetPosition()) || PretendPlayerInAGarage)
//#endif
				{
					if ((m_bGarageFixedCamPositionSet==false)&&(m_bLookingAtPlayer==true))
					{
						if ((pToGarageWeAreIn!=NULL)||(PretendPlayerInAGarage))
						{
							if (pToGarageWeAreIn!=NULL)//so we don't accces duff memory
							{
								PlayerPosition=pTargetEntity->GetPosition();
							
								//first work out what door we are under
								CObject *pDoor1, *pDoor2;
								pToGarageWeAreIn->FindDoorsWithGarage(&pDoor1, &pDoor2);
			
								if 	(pDoor1!=NULL)
								{
									DoorWeAreUnder=1;
									CenterOfDoorOne.x=pDoor1->GetPosition().x;
									CenterOfDoorOne.y=pDoor1->GetPosition().y;
									CenterOfDoorOne.z=0;
									PlayerPosition.z=0;
									DistanceCarToDoorOne=(PlayerPosition - CenterOfDoorOne).Magnitude();
								}
								else if (pDoor2!=NULL)
								{
									if (DoorWeAreUnder==1) //need to see if we are closer to door two
									{
										CenterOfDoorTwo.x=pDoor2->GetPosition().x;
										CenterOfDoorTwo.y=pDoor2->GetPosition().y;
										CenterOfDoorTwo.z=0;
										PlayerPosition.z=0;
										DistanceCarToDoorTwo=(PlayerPosition - CenterOfDoorTwo).Magnitude();
										if (DistanceCarToDoorTwo<DistanceCarToDoorOne)
										{
											DoorWeAreUnder=2;
										}
									}						
									else //door one didn't exist, mustbe door two we are under 
									{
										DoorWeAreUnder=2;
									}
								}
								else //its a garge with no doors
								{
									//just pretend we are under door one
									DoorWeAreUnder=1;
									CenterOfDoorOne.x=pTargetEntity->GetPosition().x;
									CenterOfDoorOne.y=pTargetEntity->GetPosition().y;
									CenterOfDoorTwo.z=0;							
								}	
							}
							else//PretendPlayerInAGarage==true
							{
								ASSERTMSG(PretendPlayerInAGarage, "Cullzone garage stuff gone wrong");
								DoorWeAreUnder=1;
								CenterOfDoorOne=Cams[ActiveCam].Source;

								if(pZoneLikeAGarage)
								{
									CenterOfGarage=pZoneLikeAGarage->ZoneDef.FindCenter();
// hack fix for GTA_MIAMI only
//#ifdef GTA_MIAMI
//									if(pTargetEntity->GetPosition().x > 376.0f && pTargetEntity->GetPosition().x < 383.0f
//									&& pTargetEntity->GetPosition().y > -496.0f && pTargetEntity->GetPosition().y < -489.0f
//									&& pTargetEntity->GetPosition().z > 11.6f && pTargetEntity->GetPosition().z < 13.6f)
//									{
//										CenterOfDoorOne = CVector(382.6f, -489.6f, 13.1f);
//									}
//									else
//#endif
//									if((CenterOfGarage - CenterOfDoorOne).Magnitude2D() > 15.0f)
									{
										bool bFoundExitDirn = true;
										CVector vecTestExitDirn = pTargetEntity->GetPosition() - CenterOfGarage;
										vecTestExitDirn.z = 0.0f;
										vecTestExitDirn.Normalise();
										
										float fMaxZoneDim = MAX( ABS(pZoneLikeAGarage->ZoneDef.Vec1X) + ABS(pZoneLikeAGarage->ZoneDef.Vec2X), ABS(pZoneLikeAGarage->ZoneDef.Vec1Y) + ABS(pZoneLikeAGarage->ZoneDef.Vec2Y));
										
										CVector vecTestExitPos = pTargetEntity->GetPosition() + 2.0f*fMaxZoneDim*vecTestExitDirn;
										if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true))
										{
											vecTestExitPos = pTargetEntity->GetPosition() - 2.0f*fMaxZoneDim*vecTestExitDirn;
											if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), vecTestExitPos, true, false, false, false, false, false, true))
											{
												bFoundExitDirn = false;	
											}
										}
										
										if(bFoundExitDirn)
											CenterOfDoorOne = vecTestExitPos;
									}
								}
							}
							
							//now find the normal out the way
							if (pToGarageWeAreIn!=NULL)
							{
								CenterOfGarage=CVector((pToGarageWeAreIn->MinX+pToGarageWeAreIn->MaxX)/2, (pToGarageWeAreIn->MinY+pToGarageWeAreIn->MaxY)/2, 0);
							}
							else
							{
								CenterOfDoorOne.z=0.0f;
								if(pZoneLikeAGarage==NULL)
								{
									CenterOfGarage=CVector(pTargetEntity->GetPosition().x, pTargetEntity->GetPosition().y, 0);
								}
							}
							
							if (DoorWeAreUnder==1)
							{
								DoorFront=CenterOfDoorOne-CenterOfGarage;
							}
							else
							{
								ASSERTMSG(DoorWeAreUnder==2, "Get Mark Something worng with garages");
								DoorFront=CenterOfDoorTwo-CenterOfGarage;
							}							
										
							bool GroundFound;
							float GroundZ=0.0f;
							float HeightUp=3.1f;
							PlayerPosition=pTargetEntity->GetPosition();
							GroundZ=CWorld::FindGroundZFor3DCoord(PlayerPosition.x, PlayerPosition.y, PlayerPosition.z, &GroundFound);
							//ASSERTMSG(GroundFound, "Could find the ground are we in the sky?");
							if (GroundFound==false) //this shouldn't be happening but been having
							//lots of complaints about the game hanging so this is the best alternative
							{
								GroundZ=PlayerPosition.z-0.20f;
							}
							DoorFront.z=0;
							
							
							if (DoorWeAreUnder==1)
							{
								if ((pToGarageWeAreIn==NULL)&&(PretendPlayerInAGarage))
								{
									DoorFront.Normalise();
									if(pZoneLikeAGarage)
									{
										CamDistanceAwayForGarage = 3.75f + 0.7f*MAX( ABS(pZoneLikeAGarage->ZoneDef.Vec1X) + ABS(pZoneLikeAGarage->ZoneDef.Vec2X), ABS(pZoneLikeAGarage->ZoneDef.Vec1Y) + ABS(pZoneLikeAGarage->ZoneDef.Vec2Y));
										TheCamPosition = CenterOfGarage + CamDistanceAwayForGarage*DoorFront;
									}
									else
									{
										CamDistanceAwayForGarage=3.75f;
										TheCamPosition = CenterOfDoorOne + CamDistanceAwayForGarage*DoorFront;
									}
								}
								else
								{
									DoorFront.Normalise();
									TheCamPosition=CenterOfDoorOne+(CamDistanceAwayForGarage*DoorFront);
								}
							}
							else
							{
								ASSERT(DoorWeAreUnder==2)
								DoorFront.Normalise();
								TheCamPosition=CenterOfDoorTwo+(CamDistanceAwayForGarage*DoorFront);
							}							
							
							if ((m_nPedZoom==ZOOM_FOUR)&&(!PretendPlayerInAGarage))
							{
								TheCamPosition=CenterOfGarage;
								TheCamPosition.z+=FindPlayerPed()->GetPosition().z + 2.1f;
								if (pToGarageWeAreIn!=NULL)
								{
									if (TheCamPosition.z>pToGarageWeAreIn->MaxX)
									{
										TheCamPosition.z=pToGarageWeAreIn->MaxX;
									}
								}
							}
							else
							{
								TheCamPosition.z=GroundZ+HeightUp;
							}
							
							SetCamPositionForFixedMode((TheCamPosition),CVector(0,0,0));
							m_bGarageFixedCamPositionSet=true;
						}
								
				
						// FOR A PED....
						// DW - Right this is a pile of crap, I'm going to set the camera position myself.
						CVector player;
						CVector door1(0,0,0);
						CVector door2(0,0,0);
						CVector directionOfDoor1;
						CVector directionOfDoor2;
						CVector doorDir;

						if (pToGarageWeAreIn!=NULL)
						{
							//first work out what door we are under
							CObject *pDoor1, *pDoor2;
							pToGarageWeAreIn->FindDoorsWithGarage(&pDoor1, &pDoor2);
																
							if (pDoor1)
							{								
								door1 = pDoor1->GetRelatedDummy()->GetPosition();
								directionOfDoor1 = pDoor1->GetRelatedDummy()->GetMatrix().GetRight();
								doorDir = directionOfDoor1;
							}
							
							if (pDoor2)
							{
								door2 = pDoor2->GetRelatedDummy()->GetPosition();
								directionOfDoor2 = pDoor2->GetRelatedDummy()->GetMatrix().GetRight();
								doorDir = directionOfDoor2;									
							}

							CVector posForCamera;
							if (pDoor1)																		
							{
								if (pDoor2)
									posForCamera = (door2-door1)*0.5f + door1;
								else
									posForCamera = door1;
							}
							else if (pDoor2)																		
							{
								posForCamera = door2;
							}
							else
							{
								posForCamera = PlayerPosition;
								doorDir = posForCamera-CenterOfGarage;
								doorDir.z = 0.0f;
								doorDir.Normalise();
	//										ASSERT(false); // See me  - DW
							}

							static float gLenFromDoor = -10.0f;
							static float gHeightFromDoor = 2.0f;
							TheCamPosition = posForCamera + doorDir * gLenFromDoor;
							TheCamPosition.z += gHeightFromDoor;
							
							// now we have a dcent position for the camera ray cast to find a safe place to put it.
							
							
							SetCamPositionForFixedMode((TheCamPosition),CVector(0,0,0));				
							m_bGarageFixedCamPositionSet=true;
						}				
					}
						
					if (((CGarages::CameraShouldBeOutside()||(PretendPlayerInAGarage))&&(m_bLookingAtPlayer==true)&&(m_bGarageFixedCamPositionSet==true)))
					{
						if ((pToGarageWeAreIn!=NULL)||(PretendPlayerInAGarage))
						{
							ReqMode=CCam::MODE_FIXED;
							m_bPlayerIsInGarage=true;
						}
					}
					else
					{
						if (m_bPlayerIsInGarage)
						{
							m_bJustCameOutOfGarage=true;
							m_bPlayerIsInGarage=false;
						}
						ReqMode=CCam::MODE_FOLLOWPED;
					}
				}
				else
				{
					if (m_bPlayerIsInGarage)
					{
						m_bJustCameOutOfGarage=true;
						m_bPlayerIsInGarage=false;
					}
					m_bGarageFixedCamPositionSet=false;
				}

/* This is Vice Code
				if( !m_bFirstPersonBeingUsed && (pTargetEntity->GetPosition() - CVector(474.3f, -1717.6f, 0.0f)).Magnitude2D() < 6.0f
				&& ((pTargetEntity->GetPosition() - CVector(474.3f, -1717.6f, 0.0f)).Magnitude2D() < 3.8f || pTargetEntity->GetPosition().z > 50.0f))
				{
#ifdef GTA_PC
					if(!Cams[ActiveCam].Using3rdPersonMouseCam())
#endif
						ReqMode = CCam::MODE_LIGHTHOUSE;
				}*/
				
// DW - removed		
/*				if (((Cams[ActiveCam].IsTargetInWater(Cams[ActiveCam].Source)) && (TargetIsBoat==FALSE)) && (Cams[ActiveCam].CamTargetEntity->GetIsTypePed()==true)) 
				{
					ReqMode = CCam::MODE_PLAYER_FALLEN_WATER;
				}*/
// DW - end removed		

/* DW - removed				if (m_nPedZoom==ZOOM_FOUR)
				{
					if ((!(CCullZones::Cam1stPersonForPlayer()))&&(!(CCullZones::CamNoRain()||CCullZones::PlayerNoRain())))
					{
						if ((!m_bFirstPersonBeingUsed)&&(!m_bPlayerIsInGarage))
						{
							ReqMode=CCam::MODE_TOP_DOWN_PED;
						}
					}
				}*/
								
				 //need to do this here, it's a last minute hack, should be getting cleared elsewhere but it's not
				if(!CPad::GetPad(0)->GetTarget() && PlayerWeaponMode.Mode!=CCam::MODE_NONE
				&& PlayerWeaponMode.Mode!=CCam::MODE_HELICANNON_1STPERSON 
				&& PlayerWeaponMode.Mode!=CCam::MODE_AIMWEAPON_FROMCAR
				&& PlayerWeaponMode.Mode!=CCam::MODE_AIMWEAPON_ATTACHED
				&& !(PlayerWeaponMode.Mode==CCam::MODE_CAMERA && FindPlayerPed()->m_pAttachToEntity))
				{
					ClearPlayerWeaponMode();
				}
				
	
				// See if any low priority camera modes should be used.
				if (m_PlayerMode.Mode != CCam::MODE_NONE)
				{
					ReqMode = m_PlayerMode.Mode;
				}

				// See if any high priority camera modes should be used.
				if ((PlayerWeaponMode.Mode != CCam::MODE_NONE)&&(PretendPlayerInAGarage==false)) 
				{
					//only go out of sniper modes when the ped is trying to enter a car
					if(PlayerWeaponMode.Mode==CCam::MODE_SNIPER	|| PlayerWeaponMode.Mode==CCam::MODE_ROCKETLAUNCHER
					|| PlayerWeaponMode.Mode==CCam::MODE_ROCKETLAUNCHER_HS || PlayerWeaponMode.Mode==CCam::MODE_M16_1STPERSON
					|| PlayerWeaponMode.Mode==CCam::MODE_HELICANNON_1STPERSON || PlayerWeaponMode.Mode==CCam::MODE_SNIPER
					|| PlayerWeaponMode.Mode==CCam::MODE_CAMERA	|| Cams[ActiveCam].GetWeaponFirstPersonOn())
					{	
						if ((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState())==PED_SEEK_CAR)
						{
							if ((ReqMode!=CCam::MODE_TOP_DOWN_PED)&&(!(Cams[ActiveCam].GetWeaponFirstPersonOn())))//if we are in top down or 1rst person mode want to stay in it
							{
								ReqMode=CCam::MODE_FOLLOWPED;
							}
							else
							{
								
								ReqMode = PlayerWeaponMode.Mode;		
							}
						}
						else
						{
							
							ReqMode = PlayerWeaponMode.Mode;		
						}
					}
					else if(ReqMode!=CCam::MODE_TOP_DOWN_PED)//&&(m_nPedZoom!=ZOOM_THREE))//Right the fucker is Wantin to go into Syphon Filter Mode (only go into it if we are not in top down mode
					{
						float Crim_Player_Cam_Angle=0.0f;
						float CurrentDistOnGround=0.0f;
						float DistanceOnGround=0.0f;
						float Length=0.0f;
						float CrimToPlayerDist=0.0f;
						float PlayerToCamDistance=0.0f;
						static float MinDistBetweenPlayerDeadCrim=3.0f;//used for when the target is deadied
						static float MinDistBetweenPlayerAliveCrim=1.5f;//3.0f;
						static float MaxDistBetweenPlayerCrim = 4.0f;
						static float SpecialFixedCamDist = 4.0f;
						static float MaxPlayerToCameraDist = 10.0f;

						float PlayerToCrimAngle=0.0f;
						float PlayerToCamAngle=0.0f;	
						bool  TargetDyingOrDead=false;
						CVector PlayerPos=pTargetEntity->GetPosition();
						CVector PlayerToCrimVector=m_cvecAimingTargetCoors-PlayerPos;
						CVector PlayerToCamVector=Cams[ActiveCam].Source-PlayerPos;
					
						if ((m_bPlayerIsInGarage==false)||(PretendPlayerInAGarage==false))  //we only want to go into syhpon filter esque modes
													      //if not in a garage 
						{
//							if (m_nPedZoom==ZOOM_ONE)
//							{
//							    MinDistBetweenPlayerDeadCrim=2.25f;
//							}
							float DistToUseRePedStatus=0.0f;
							if(FindPlayerPed()->GetWeaponLockOn() || FindPlayerPed()->GetPlayerData()->m_bFreeAiming) 
							{
								if(FindPlayerPed()->GetWeaponLockOnTarget() && FindPlayerPed()->GetWeaponLockOnTarget()->GetIsTypePed()
								&& (((CPed*)FindPlayerPed()->GetWeaponLockOnTarget())->GetPedState()==PED_DEAD || ((CPed*)FindPlayerPed()->GetWeaponLockOnTarget())->GetPedState()==PED_DIE) )
								{
									TargetDyingOrDead=true;
									DistToUseRePedStatus=MinDistBetweenPlayerDeadCrim;
								}
//								else
//								{
//									DistToUseRePedStatus=MinDistBetweenPlayerAliveCrim;
//								}
								
								CrimToPlayerDist=CMaths::Sqrt(PlayerToCrimVector.x * PlayerToCrimVector.x + PlayerToCrimVector.y * PlayerToCrimVector.y);
								PlayerToCamDistance=CMaths::Sqrt(PlayerToCamVector.x * PlayerToCamVector.x + PlayerToCamVector.y * PlayerToCamVector.y);
								
								PlayerToCamAngle=CGeneral::GetATanOfXY(PlayerToCamVector.x , PlayerToCamVector.y);
								PlayerToCrimAngle=CGeneral::GetATanOfXY(PlayerToCrimVector.x , PlayerToCrimVector.y);
								
								ReqMode = PlayerWeaponMode.Mode;
/*
								//if ((Cams[ActiveCam].Mode!=CCam::MODE_SYPHON)&&(TargetDyingOrDead==false))//if we are Not in the Syphon filter mode yet
								if (Cams[ActiveCam].Mode!=CCam::MODE_SYPHON)
								//don't want to come out if we are behind the player
								{
									float CrimToPlayerMaxDist=3.5f;
									Crim_Player_Cam_Angle=PlayerToCamAngle - PlayerToCrimAngle;
									while (Crim_Player_Cam_Angle>=(PI)) {Crim_Player_Cam_Angle-=2.0f * PI;}
							 		while (Crim_Player_Cam_Angle<(-PI)) {Crim_Player_Cam_Angle+=2.0f * PI;}
									
									if ((((CMaths::Abs(Crim_Player_Cam_Angle))<DEGTORAD(90.0f)) && (CrimToPlayerDist<CrimToPlayerMaxDist))&&((PlayerToCrimVector.z)>-1.0f))
									{
										ReqMode = CCam::MODE_SYPHON_CRIM_IN_FRONT;
									}
								}
*/								

#ifdef CAMERA_USE_DEAD_TARGET_FIXED_CAM
								float SpecialFixedSyphonDist=0.0f;
//								if((ReqMode==CCam::MODE_SYPHON_CRIM_IN_FRONT)||(ReqMode==CCam::MODE_SYPHON))
								if(ReqMode==CCam::MODE_AIMWEAPON && TargetDyingOrDead
								&& FindPlayerPed()->GetWeaponLockOn())// && !FindPlayerPed()->GetWeapon()->IsTypeMelee())
								{
									 //finished any transitions
									if(m_uiTransitionState==TRANS_NONE || Cams[ActiveCam].Mode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)
									{
										if(Cams[ActiveCam].Mode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON && FindPlayerPed()->GetWeaponLockOnTarget()->GetIsTypePed())
											DistToUseRePedStatus = MaxDistBetweenPlayerCrim;
										
										if (CrimToPlayerDist<DistToUseRePedStatus)
										{
											ReqMode = CCam::MODE_SPECIAL_FIXED_FOR_SYPHON;
											SpecialFixedSyphonDist = SpecialFixedCamDist;
											
											if (TargetDyingOrDead)
											{
												if (ReqMode==CCam::MODE_SYPHON_CRIM_IN_FRONT)
												{
													SpecialFixedSyphonDist=5.0f;
												}
												else
												{
													SpecialFixedSyphonDist=5.6f;
												}
												ReqMode = CCam::MODE_SPECIAL_FIXED_FOR_SYPHON;
												
											}
										//	else
										//	{
										//		OutputDebugString("DistFromSyphonInFront Ped Alive");
										//		SpecialFixedSyphonDist=3.0f;
										//	}
										}
									}
								}
#endif

	/*							
								if (ReqMode==CCam::MODE_SYPHON)
								{
									if ((m_uiTransitionState==TRANS_NONE)||(Cams[ActiveCam].Mode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)) 
									//will only go into MODE_SPECIAL_FIXED_FOR_SYPHON when transitions have finished
									{	
									//	if (Cams[ActiveCam].Mode==CCam::MODE_SYPHON) //alreaady in this mode so will be close to
										//player	
										{
											if (CrimToPlayerDist<DistToUseRePedStatus)
											{
												if (TargetDyingOrDead)
												{
													SpecialFixedSyphonDist=3.0f;
													ReqMode = CCam::MODE_SPECIAL_FIXED_FOR_SYPHON;
												}
											//	else
											//	{
											//		OutputDebugString("DistFromNormalSyphon Ped Alive");
											//		SpecialFixedSyphonDist=1.0f;
											///	}
											}
										}
									}						
								}
	*/		

#ifdef CAMERA_USE_DEAD_TARGET_FIXED_CAM
								if (ReqMode == CCam::MODE_SPECIAL_FIXED_FOR_SYPHON) 
								{
									if (PlaceForFixedWhenSniperFound==false)
									{	
										CVector TheFixedCamPositionToGoTo;
										CColPoint TempCol;
										CEntity* TempHit=NULL;
											
										TheFixedCamPositionToGoTo=pTargetEntity->GetPosition(); 
										TheFixedCamPositionToGoTo.x+=CMaths::Cos(PlayerToCamAngle)*SpecialFixedSyphonDist;
										TheFixedCamPositionToGoTo.y+=CMaths::Sin(PlayerToCamAngle)*SpecialFixedSyphonDist;
										TheFixedCamPositionToGoTo.z+=1.15f;
											//Lets Do A Quick Collision check
										if (CWorld::ProcessLineOfSight(pTargetEntity->GetPosition(), TheFixedCamPositionToGoTo, TempCol, TempHit, true, false, false, true, false, true, true))
					   				    {
										    SetCamPositionForFixedMode(TempCol.GetPosition(), CVector(0,0,0));
										}
										else
										{
											SetCamPositionForFixedMode(TheFixedCamPositionToGoTo, CVector(0,0,0));
										}
											
										PlaceForFixedWhenSniperFound=true;
									}
								}
								else
#endif
								{
									PlaceForFixedWhenSniperFound=false;
								}
							}
						}
					}
				}
			}//END OF PED STUFF
		}//END OF IDLE STUFF
	} //END OF GAME IS PAUSED STUFF


	// Pick the right camera for the circumstance in cooperative camera mode.
	if (m_bCooperativeCamMode && CWorld::Players[0].pPed && CWorld::Players[1].pPed)
	{
		if (CWorld::Players[0].pPed->m_nPedFlags.bInVehicle && CWorld::Players[1].pPed->m_nPedFlags.bInVehicle &&
			CWorld::Players[0].pPed->m_pMyVehicle && CWorld::Players[1].pPed->m_pMyVehicle)
		{		// Both players are in a car.
			if (CWorld::Players[0].pPed->m_pMyVehicle == CWorld::Players[1].pPed->m_pMyVehicle)
			{			// Both players are in the same car
				if (m_bAllowShootingWith2PlayersInCar)
				{
					ReqMode = m_ModeForTwoPlayersSameCarShootingAllowed;
					pTargetEntity = CWorld::Players[0].pPed->m_pMyVehicle;
				}
				else
				{
					ReqMode = m_ModeForTwoPlayersSameCarShootingNotAllowed;
					pTargetEntity = CWorld::Players[0].pPed->m_pMyVehicle;
				}
			}
			else
			{			// Players are each in their own car.
				ReqMode = m_ModeForTwoPlayersSeparateCars;
				pTargetEntity = CWorld::Players[0].pPed->m_pMyVehicle;
			}
		}
		else
		{
			ReqMode = m_ModeForTwoPlayersNotBothInCar;
		}
	}

#ifndef MASTER
	// See if the debug camera mode overwrites the standard mode
	// (this will not happen in the final game)	
	if (DebugCamMode != CCam::MODE_NONE)
	{
		ReqMode = DebugCamMode;
	//	ReqMinZoom = CCam::MAX_ZOOM_IN;
	//	ReqMaxZoom = CCam::MAX_ZOOM_OUT;
	}
#endif
	
	//now do stuff if the player has been arrested
	CPed* PlayerPed; 
	bool bJustArrested=false;
	static bool bPreviouslyArrested=false;
	bool bJustNotArrested=false;
	PlayerPed=CWorld::Players[CWorld::PlayerInFocus].pPed;

	if(PlayerPed->GetPedState()==PED_ARRESTED)
	{
		bPreviouslyArrested=true;
	}
	else if(bPreviouslyArrested)
	{
		bJustNotArrested=true;//this is required so that the camea will do a jump cut
		bPreviouslyArrested=false;
	}	
	
	if(LastPedState!=PED_ARRESTED && PlayerPed->GetPedState()==PED_ARRESTED)
	{
		//special hack for when car is in first person view 
		if( m_nCarZoom!=ZOOM_ZERO || !pTargetEntity->GetIsTypeVehicle())
			bJustArrested=true;
	}
	else
	{
		bJustArrested=false;
	}
	
	LastPedState=PlayerPed->GetPedState();
	
	if(bJustArrested==true) //haven't picked the arrest cam yet
	{
		ReqMode=CCam::MODE_ARRESTCAM_ONE;
		ThePickedArrestMode=ReqMode;
		Cams[ActiveCam].ResetStatics=true;
//Marks old stuff///////////////////////////
/*		if(m_uiTransitionState==TRANS_NONE)
		{
			bool GoodToGoToMode=false;
			
			if (pTargetEntity->GetIsTypePed())
			{
				Cams[(ActiveCam + 1) % 2].ResetStatics=true;
				GoodToGoToMode=Cams[(ActiveCam + 1) % 2].ProcessArrestCamOne();
				ReqMode=CCam::MODE_ARRESTCAM_ONE;
			}
			else 
			{
				Cams[(ActiveCam + 1) % 2].ResetStatics=true;
				GoodToGoToMode=Cams[(ActiveCam + 1) % 2].ProcessArrestCamTwo();
				ReqMode=CCam::MODE_ARRESTCAM_TWO;
			}
			
			if (GoodToGoToMode==false)
			{
				ReqMode=Cams[ActiveCam].Mode;
			}
		}
		else
		{
			ReqMode=Cams[ActiveCam].Mode;
		}
		
		ThePickedArrestMode=ReqMode;
*/
	}
	else if ((PlayerPed->GetPedState())==PED_ARRESTED)
	{
		ReqMode=ThePickedArrestMode;
	}
		
	if (CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState() ==PED_DEAD)
	{
		m_bObbeCinematicCarCamOn = false; // DW - cant understand the flow of logic but we certainly dont want ciney cam when you are dead.

		if(Cams[ActiveCam].Mode==CCam::MODE_PED_DEAD_BABY)//stay in the mode while dead
		{
			ReqMode=CCam::MODE_PED_DEAD_BABY;
		}
		else if(Cams[ActiveCam].Mode==CCam::MODE_ARRESTCAM_ONE)//stay in the mode while dead
		{
			ReqMode=CCam::MODE_ARRESTCAM_ONE;
		}
		else //we are not in the mode yet
		{
			bool bUseArrestCam = false;
			if(pTargetEntity->GetIsTypePed())
			{
				CPed *pTempTargetPed = (CPed *)pTargetEntity;
				
				int i;
				const int N=pTempTargetPed->GetPedIntelligence()->GetMaxNumPedsInRange();
				CEntity** ppNearbyEntities=pTempTargetPed->GetPedIntelligence()->GetNearbyPeds();
				for(i=0;i<N;i++)
				{
					CEntity* pNearbyEntity=ppNearbyEntities[i];
					if(pNearbyEntity)
					{
						ASSERT(pNearbyEntity->GetType()==ENTITY_TYPE_PED);
						CPed* pNearbyPed=(CPed*)pNearbyEntity;
						CTaskSimpleArrestPed* pTaskArrest=(CTaskSimpleArrestPed*)pNearbyPed->GetPedIntelligence()->FindTaskActiveByType(CTaskTypes::TASK_SIMPLE_ARREST_PED);
						if(pTaskArrest && pTaskArrest->GetTargetPed()==FindPlayerPed())
						{
							if((pNearbyPed->GetPosition() - pTempTargetPed->GetPosition()).Magnitude() < 4.0f)
							{
								bUseArrestCam = true;
								ReqMode = CCam::MODE_ARRESTCAM_ONE;
								Cams[ActiveCam].ResetStatics=true;
								break;
							}
						}
					}
				}
				/*
				for(int32 nClosePed = 0; nClosePed < pTempTargetPed->m_NumClosePeds; nClosePed++)
				{
					if(pTempTargetPed->m_apClosePeds[nClosePed] && pTempTargetPed->m_apClosePeds[nClosePed]->GetPedState()==PED_ARREST_PLAYER
					&& (pTempTargetPed->m_apClosePeds[nClosePed]->GetPosition() - pTempTargetPed->GetPosition()).Magnitude() < 4.0f)
					{
						bUseArrestCam = true;
						ReqMode = CCam::MODE_ARRESTCAM_ONE;
						Cams[ActiveCam].ResetStatics=true;
						break;
					}
				}
				*/
			}
		
			if(bUseArrestCam==false)
			{
				ReqMode=CCam::MODE_PED_DEAD_BABY;
				Cams[ActiveCam].ResetStatics=true;
//////////////////////////////////			
/*				bool RoofFound=false;
				CVector TempPlayPos=FindPlayerPed()->GetPosition();
				CWorld::FindRoofZFor3DCoord(TempPlayPos.x, TempPlayPos.y, TempPlayPos.z, &RoofFound);
				if (RoofFound==false)
				{
					ReqMode=CCam::MODE_PED_DEAD_BABY;
				}
*/
			}
		}
	}
	
	
	if (m_bRestoreByJumpCut==true)
	{
		
		 if (!((ReqMode==CCam::MODE_FOLLOWPED)||(ReqMode==CCam::MODE_M16_1STPERSON)||
		 		(ReqMode==CCam::MODE_SNIPER) || (ReqMode==CCam::MODE_ROCKETLAUNCHER)||(ReqMode==CCam::MODE_ROCKETLAUNCHER_HS)||
		 		(ReqMode==CCam::MODE_CAMERA)||(ReqMode==CCam::MODE_SYPHON)||(ReqMode==CCam::MODE_SYPHON_CRIM_IN_FRONT)
		 		||(ReqMode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)||(ReqMode==CCam::MODE_CAM_ON_A_STRING)
		 		||(ReqMode==CCam::MODE_BEHINDCAR) || 
		 		(m_bUseMouse3rdPerson)))
		 {
	 		SetCameraDirectlyBehindForFollowPed_CamOnAString();				
		 }
	 	 
	 	 	
		 ReqMode = m_iModeToGoTo;
		 Cams[ActiveCam].Mode=ReqMode;	
		 m_bJust_Switched=TRUE;
		 Cams[ActiveCam].ResetStatics=true;
		 Cams[ActiveCam].m_cvecCamFixedModeVector=m_vecFixedModeVector;
		 TIDYREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
		 Cams[ActiveCam].CamTargetEntity=pTargetEntity;
		 REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
		 Cams[ActiveCam].m_cvecCamFixedModeSource=m_vecFixedModeSource;
		 Cams[ActiveCam].m_cvecCamFixedModeUpOffSet=m_vecFixedModeUpOffSet; 
		 Cams[ActiveCam].m_bCamLookingAtVector=false;
		 Cams[ActiveCam].m_vecLastAboveWaterCamPosition=Cams[(ActiveCam + 1) % 2].m_vecLastAboveWaterCamPosition;
		 m_bRestoreByJumpCut=false;
		 Cams[ActiveCam].ResetStatics=true;
//		 REGREF(pTargetEntity, &(pTargetEntity));
//		 REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));		
		 m_fCarZoomSmoothed=m_fCarZoomBase;
		 m_fPedZoomSmoothed=m_fPedZoomBase;
		 m_uiTransitionState=TRANS_NONE;
		 m_vecDoingSpecialInterPolation=false;

	}

	
	
	//


	if (gbModelViewer)
	{
		ReqMode = CCam::MODE_MODELVIEW;
	}

	// Now we do the higher level stuff. (Scripting requesting certain
	// modes for cut scenes etc)

	//righty ho if we are going to or from top down then we just want to jump cut switch

	//if not on train
	//if not if garage
	//if not in dead
	//if not using weapon 
	//if not fallen in water
	//if not arrested then 
	//if not script got control
	//its o.k. to go to obbecam
	bool CanTryObbeCam=true;
	
	//need to do this outside the normal update target entity loop 
	//as obbe has take control of camera	
	if (pTargetEntity!=NULL)
	{
		if (pTargetEntity->GetIsTypeVehicle()==false)
		{
			if (m_nPedZoom==ZOOM_FIVE)
			{
			//	pTargetEntity=FindPlayerPed();
				m_bObbeCinematicPedCamOn=true;
			}
		}
		else	//
		{
			if (m_nCarZoom==ZOOM_FIVE)
			{
			//	pTargetEntity=FindPlayerVehicle();
				m_bObbeCinematicCarCamOn=true; //don't go into it yet though, we need to check that we are not in a garage
			}
		}
	}
	
	if (FindPlayerVehicle()!=NULL)
	{
		if ((FindPlayerVehicle()->GetBaseVehicleType())==VEHICLE_TYPE_TRAIN) 
		{
			m_bObbeCinematicCarCamOn=true;
		}
	}
			
	// DW - stop the fucking cinema camera when player is dead or arrested... geez how hard can a simple thing be!
	if (pTargetEntity!=NULL)
	{
		if (pTargetEntity->GetIsTypeVehicle())
		{
			CPlayerPed *PlayerPed;	
			PlayerPed=FindPlayerPed(); 
			//check if we are being arrested or are dead we want to go to those camera modes
			if (PlayerPed)
			{
				if ((PlayerPed->GetPedState()==PED_ARRESTED)||(PlayerPed->GetPedState()==PED_DEAD))
				{
					CanTryObbeCam=false;
					m_bObbeCinematicPedCamOn = false;
					if (PlayerPed->GetPedState()==PED_ARRESTED)
					{
						ReqMode = CCam::MODE_ARRESTCAM_ONE;// force the requested mode!... cant seem stop it doing mad shit
					}
					else if (PlayerPed->GetPedState()==PED_DEAD)
					{
						ReqMode = CCam::MODE_PED_DEAD_BABY;
					}
				}	
			}
		}
	}	
			
	if ((m_bTargetJustBeenOnTrain==true)||(ReqMode==CCam::MODE_PED_DEAD_BABY)
		||(ReqMode==CCam::MODE_PED_DEAD_BABY)||(ReqMode==CCam::MODE_PLAYER_FALLEN_WATER)
		||(ReqMode==CCam::MODE_SYPHON_CRIM_IN_FRONT)||(ReqMode==CCam::MODE_SYPHON) 
		||(ReqMode==CCam::MODE_SNIPER)||(ReqMode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)
		||(ReqMode==CCam::MODE_ROCKETLAUNCHER)||(ReqMode==CCam::MODE_ROCKETLAUNCHER_HS)||(ReqMode==CCam::MODE_PLAYER_FALLEN_WATER)
		||(ReqMode==CCam::MODE_ARRESTCAM_ONE)||(ReqMode==CCam::MODE_ARRESTCAM_TWO)
		||(ReqMode==CCam::MODE_M16_1STPERSON)||(ReqMode==CCam::MODE_FIGHT_CAM)
		||(ReqMode==CCam::MODE_SNIPER_RUNABOUT)||(ReqMode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT)||(ReqMode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS)
		||(ReqMode==CCam::MODE_M16_1STPERSON_RUNABOUT)||(ReqMode==CCam::MODE_FIGHT_CAM_RUNABOUT)
		||(ReqMode==CCam::MODE_1STPERSON_RUNABOUT) || (ReqMode==CCam::MODE_HELICANNON_1STPERSON)
		||(ReqMode==CCam::MODE_CAMERA)
		||(WhoIsInControlOfTheCamera==SCRIPT_CAM_CONTROL)||(m_bJustCameOutOfGarage)||(m_bPlayerIsInGarage)
		||(Cams[ActiveCam].Mode==CCam::MODE_PED_DEAD_BABY)) // DW - stop the fucking cinematic camera you twats!
	{
		CanTryObbeCam=false;
	}


	// script chance to switch on cinematic cam
	// DW - migrated due to (WhoIsInControlOfTheCamera==SCRIPT_CAM_CONTROL) bit above... dunno why!?
	if (m_bForceCinemaCam)
	{
		m_bObbeCinematicCarCamOn=true;
		CanTryObbeCam=true;
		ASSERT(pTargetEntity);
		ASSERT(pTargetEntity->GetIsTypeVehicle());
	}
	
		
	if ((m_bObbeCinematicPedCamOn)&&(CanTryObbeCam==true))
	{
//		ProcessObbeCinemaCameraPed();
	}
	else if ((m_bObbeCinematicCarCamOn)&&(CanTryObbeCam==true))
	{	
		CPostEffects::SpeedFXDisableForThisFrame(); // no speed blur please....
		
		//if heli or plane go into the special heli cam cinematic mode		
		if (pTargetEntity->GetIsTypeVehicle())
		{
			int32 vehicleType = ((CVehicle *)pTargetEntity)->GetVehicleType();
		
			if (vehicleType==VEHICLE_TYPE_PLANE)
				ProcessObbeCinemaCameraPlane();
			else if((((CVehicle *)pTargetEntity)->GetVehicleAppearance()==APR_HELI))		
				ProcessObbeCinemaCameraHeli();
			else if (vehicleType==VEHICLE_TYPE_BOAT)
				ProcessObbeCinemaCameraBoat();
			else if (vehicleType==VEHICLE_TYPE_TRAIN) 
			{						
//				RwCameraSetNearClip(0.3f); // make sure the trains near clip is not bad!
				ProcessObbeCinemaCameraTrain();
			}
			else
				ProcessObbeCinemaCameraCar();
		}
	}
	else
	{
		if (m_bPlayerIsInGarage)
		{
			//was have entered a garage in obbe cam
			//there will be occasions that the camera is hitched onto he side of the car, 
			//we do not want to intepolate from this position
			if (m_bObbeCinematicCarCamOn)
			{
				NeedToDoAJumpCutForGameCam=true;
			}
		}
		CanTryObbeCam=false;
		DontProcessObbeCinemaCamera();	
	}
	
	
	if (m_bLookingAtPlayer==true)
	{
	
		if (Cams[ActiveCam].Mode!=ReqMode)
		{
/*
			// DW - I dont know what I'm doing anymore...but ho hum...not my fault its this bitching bit of stuff that is supposedly called 'code' 
			float stickX = CPad::GetPad(0)->GetRightStickX();
			float stickY = CPad::GetPad(0)->GetRightStickY();
			static bool gAllowQuickSnapBehindGettingIntoCar = true;
			static int8 gStickUsingTol = 10; //DW - guess at dead zone since its not clear in code what is used for dead zone at all?
			bool bUsingRightStick = CMaths::Abs(stickX) > gStickUsingTol || CMaths::Abs(stickY)	> gStickUsingTol;
			if (!bUsingRightStick && !m_bJustCameOutOfGarage && gAllowQuickSnapBehindGettingIntoCar)
			{
				if (ReqMode == CCam::MODE_BEHINDCAR ||
					ReqMode == CCam::MODE_CAM_ON_A_STRING ||
					ReqMode == CCam::MODE_BEHINDBOAT)
				{
					static bool gbJumpcutToBehindCar = false; // change this to have immediate or over a number of frames cut when getting into a car
					if (gbJumpcutToBehindCar)
					{
						NeedToDoAJumpCutForGameCam=true;	 // make a jumpcut when you get into a car
					}
					else
					{
						TheCamera.m_bCamDirectlyBehind = true;
					}
				}		
			}
*/
		}
				
		if ((ReqMode==CCam::MODE_TOPDOWN)||(ReqMode==CCam::MODE_1STPERSON)||(ReqMode==CCam::MODE_TOP_DOWN_PED))//always jump cut for this
		{
			NeedToDoAJumpCutForGameCam=true;
		}
		else if ((ReqMode==CCam::MODE_CAM_ON_A_STRING)||(ReqMode==CCam::MODE_BEHINDBOAT))
		{
			//this bit is for coming out of these modes
			switch (Cams[ActiveCam].Mode)
			{
				case CCam::MODE_TOPDOWN:
				case CCam::MODE_1STPERSON:
				case CCam::MODE_TOP_DOWN_PED:
					NeedToDoAJumpCutForGameCam=true;
					break;
			}			
		}
		else if (ReqMode==CCam::MODE_FIXED)
		{
			if (Cams[ActiveCam].Mode==CCam::MODE_TOPDOWN)
			{
				NeedToDoAJumpCutForGameCam=true;
			}
		}
		
		if((ReqMode==CCam::MODE_AIMWEAPON || ReqMode==CCam::MODE_AIMWEAPON_FROMCAR || ReqMode==CCam::MODE_AIMWEAPON_ATTACHED)
		&& pTargetEntity && pTargetEntity->GetIsTypePed())
		{
			bool bIsUsingJetPack = false;
			
			if (pTargetEntity->GetIsTypePed() && ((CPed *)pTargetEntity)->GetPedIntelligence()->GetTaskJetPack())
				bIsUsingJetPack = true;
			
			
			if(ReqMode==CCam::MODE_AIMWEAPON && Cams[ActiveCam].Mode==CCam::MODE_FOLLOWPED && !bIsUsingJetPack)
			{
				float fTargetBeta = 0.0f;
				if(((CPed *)pTargetEntity)->GetWeaponLockOnTarget())
				{
					CVector vecAimDelta = ((CPed *)pTargetEntity)->GetWeaponLockOnTarget()->GetPosition() - pTargetEntity->GetPosition();
					fTargetBeta = CMaths::ATan2(-vecAimDelta.x, vecAimDelta.y) - HALF_PI;
				}
				else
				{
					fTargetBeta = pTargetEntity->GetHeading() - HALF_PI;
				}
				
				if(fTargetBeta > Cams[ActiveCam].Beta + PI)
					fTargetBeta -= TWO_PI;
				else if(fTargetBeta < Cams[ActiveCam].Beta - PI)
					fTargetBeta += TWO_PI;
				
				if(CMaths::Abs(fTargetBeta - Cams[ActiveCam].Beta) > DEGTORAD(MAX_ANGLE_BEFORE_AIMWEAPON_JUMPCUT))
					NeedToDoAJumpCutForGameCam=true;
				else if((pTargetEntity->GetPosition() - GetMatrix().GetTranslate()).Magnitude() > 1.5f*(TheCamera.m_fPedZoomSmoothed + 2.0f))
					NeedToDoAJumpCutForGameCam=true;

#ifdef GTA_PC
				if(TheCamera.m_bUseMouse3rdPerson)
					NeedToDoAJumpCutForGameCam = false;
#endif
			}
			else
				NeedToDoAJumpCutForGameCam=true;
		}
		
		if((ReqMode==CCam::MODE_TWOPLAYER && Cams[ActiveCam].Mode!=CCam::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING)
		|| (ReqMode==CCam::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING && Cams[ActiveCam].Mode!=CCam::MODE_TWOPLAYER)
		|| (Cams[ActiveCam].Mode==CCam::MODE_TWOPLAYER && ReqMode!=CCam::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING)
		|| (Cams[ActiveCam].Mode==CCam::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING && ReqMode!=CCam::MODE_TWOPLAYER))
		{
			NeedToDoAJumpCutForGameCam=true;
		}
		
//		if(ReqMode==CCam::MODE_FOLLOWPED && Cams[ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING
//		&& pTargetEntity && pTargetEntity->GetIsTypePed())
//		{
//			if(((CPed *)pTargetEntity)->GetMoveSpeed().MagnitudeSqr() > 0.4f*0.4f)
//			{
//				NeedToDoAJumpCutForGameCam = true;
//			}
//		}
		
		///special cases for top down guys it is safe to interpolate when going form one top soen mode to the other
		//also safe for interpolating to the dead camera
		if (ReqMode==CCam::MODE_TOPDOWN)
		{
			if ((Cams[ActiveCam].Mode==CCam::MODE_TOP_DOWN_PED)||(Cams[ActiveCam].Mode==CCam::MODE_PED_DEAD_BABY))
			{
				NeedToDoAJumpCutForGameCam=false;		
			}
		
		}
		else if (ReqMode==CCam::MODE_TOP_DOWN_PED)
		{
			if ((Cams[ActiveCam].Mode==CCam::MODE_TOPDOWN)||(Cams[ActiveCam].Mode==CCam::MODE_PED_DEAD_BABY)) 
			{
				NeedToDoAJumpCutForGameCam=false;		
			}
		
		}
		
		//this bit is for going into these modes //when you are a ped
		if ((((ReqMode==CCam::MODE_1STPERSON)||(ReqMode==CCam::MODE_SNIPER)
		||(ReqMode==CCam::MODE_M16_1STPERSON)||(ReqMode==CCam::MODE_ROCKETLAUNCHER)||(ReqMode==CCam::MODE_ROCKETLAUNCHER_HS)
		||(ReqMode==CCam::MODE_SNIPER_RUNABOUT)||(ReqMode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT)||(ReqMode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS)
		||(ReqMode==CCam::MODE_M16_1STPERSON_RUNABOUT)||(ReqMode==CCam::MODE_FIGHT_CAM_RUNABOUT)
		||(ReqMode==CCam::MODE_1STPERSON_RUNABOUT) || (ReqMode==CCam::MODE_HELICANNON_1STPERSON)
		||(ReqMode==CCam::MODE_ARRESTCAM_ONE)||(ReqMode==CCam::MODE_ARRESTCAM_TWO)||(ReqMode==CCam::MODE_CAMERA)))
		&&(pTargetEntity->GetIsTypePed()))
		{
			NeedToDoAJumpCutForGameCam=true;
		}
		else if ((ReqMode==CCam::MODE_FIXED)&&(m_bPlayerIsInGarage==true))//this is for going into this mode
		{
			if (((Cams[ActiveCam].Mode==CCam::MODE_SNIPER)||(Cams[ActiveCam].Mode==CCam::MODE_HELICANNON_1STPERSON)
				||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_HS)||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON)
				||(Cams[ActiveCam].Mode==CCam::MODE_TOP_DOWN_PED)|| (PretendPlayerInAGarage)
				||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)||(Cams[ActiveCam].Mode==CCam::MODE_SNIPER_RUNABOUT)
				||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS)||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON_RUNABOUT)
				||(Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON_RUNABOUT)
				||(Cams[ActiveCam].Mode==CCam::MODE_CAMERA))
				&&(pTargetEntity!=NULL)&&(pTargetEntity->GetIsTypeVehicle()))//just came out of these modes
			{
				NeedToDoAJumpCutForGameCam=true;
			}
		}
		//This bit is for going out of these modes
		else if (ReqMode==CCam::MODE_FOLLOWPED)
		{
			bool JumpCutForSyphons=false;
			if(Cams[ActiveCam].Mode==CCam::MODE_AIMWEAPON)
			{
				if(((CPed*)pTargetEntity)->CanWeRunAndFireWithWeapon() && !((CPed*)pTargetEntity)->m_nPedFlags.bIsDucking)
				{
					float fPedHeading = pTargetEntity->GetHeading() - HALF_PI;
					if(fPedHeading > Cams[ActiveCam].Beta + PI)
						fPedHeading -= TWO_PI;
					else if(fPedHeading < Cams[ActiveCam].Beta - PI)
						fPedHeading += TWO_PI;
					
					// use same angle to decide whether to jumpcut out of aimweapon as used to decide whether to jumpcut in
					if(CMaths::Abs(fPedHeading - Cams[ActiveCam].Beta) > DEGTORAD(MAX_ANGLE_BEFORE_AIMWEAPON_JUMPCUT)
					|| !((CPed*)pTargetEntity)->GetIsStanding())
						JumpCutForSyphons=true;
#ifdef GTA_PC
					if(TheCamera.m_bUseMouse3rdPerson)
					{
						JumpCutForSyphons = false;
						// set this so we don't reset alpha and beta angles to behind the ped
						m_bJustCameOutOfGarage = true;
					}
#endif
				}
			}
			
			if ((Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)||(Cams[ActiveCam].Mode==CCam::MODE_SNIPER)
			||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_HS)
			||(Cams[ActiveCam].Mode==CCam::MODE_PED_DEAD_BABY)||(Cams[ActiveCam].Mode==CCam::MODE_ARRESTCAM_ONE)
			||(Cams[ActiveCam].Mode==CCam::MODE_ARRESTCAM_TWO)||(Cams[ActiveCam].Mode==CCam::MODE_PILLOWS_PAPS)
			||(Cams[ActiveCam].Mode==CCam::MODE_SNIPER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS)
			||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM_RUNABOUT)
			||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_HELICANNON_1STPERSON)
			||(Cams[ActiveCam].Mode==CCam::MODE_TOPDOWN)||(Cams[ActiveCam].Mode==CCam::MODE_TOP_DOWN_PED)
			||(Cams[ActiveCam].Mode==CCam::MODE_CAMERA)||(JumpCutForSyphons)||(bJustNotArrested))
			//||(Cams[ActiveCam].Mode==CCam::MODE_FIXED))
			{
				if (!m_bJustCameOutOfGarage) ///can leave this bit in no harm
				{
					if ((Cams[ActiveCam].Mode==CCam::MODE_SNIPER)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_HS)
					||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON)||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)
					||(Cams[ActiveCam].Mode==CCam::MODE_SNIPER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS)
					||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM_RUNABOUT)
					||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_HELICANNON_1STPERSON)
					||(Cams[ActiveCam].Mode==CCam::MODE_CAMERA))
					{
						float CamDirection;
						CamDirection=CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y)-(PI/2);
						((CPed*)pTargetEntity)->m_fCurrentHeading=CamDirection;
						((CPed*)pTargetEntity)->m_fDesiredHeading=CamDirection;	
					}
					
					NeedToDoAJumpCutForGameCam=true;
					m_bUseTransitionBeta=true;
					if (Cams[ActiveCam].Mode==CCam::MODE_TOP_DOWN_PED)
					{
						CVector CamToPlayer=Cams[ActiveCam].Source-(FindPlayerPed()->GetPosition());
						CamToPlayer.z=0;
						CamToPlayer.Normalise();
						if ((CamToPlayer.x=0.001f)&&(CamToPlayer.y=0.001f)) 
						{
							CamToPlayer.y=1.0f;
						}
						Cams[ActiveCam].m_fTransitionBeta=CGeneral::GetATanOfXY(CamToPlayer.x, CamToPlayer.y);
					}
					else
					{
						Cams[ActiveCam].m_fTransitionBeta=(CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y)+PI);
           			}
               }
           }
			
		
		}
/*	DWREMOVED	else if (ReqMode==CCam::MODE_FIGHT_CAM)
		{
			if (Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)
			{
					NeedToDoAJumpCutForGameCam=true;
			}
		}*/
		else if(ReqMode==CCam::MODE_LIGHTHOUSE)
		{
			NeedToDoAJumpCutForGameCam=true;
		}
		else if(ReqMode==CCam::MODE_ARRESTCAM_ONE || ReqMode==CCam::MODE_ARRESTCAM_TWO
		|| ReqMode==CCam::MODE_PED_DEAD_BABY)
		{
			NeedToDoAJumpCutForGameCam=true;
		}
		else if(Cams[ActiveCam].Mode==CCam::MODE_PED_DEAD_BABY && ReqMode!=CCam::MODE_PED_DEAD_BABY)
		{
			NeedToDoAJumpCutForGameCam=true;
		}

		//final last check in case any of the enities have been deleted 
		//we want to do a jump cut
		if (ReqMode!=Cams[ActiveCam].Mode)
		{
// Not sure why this #ifdef was here
// (poss cause of networking, but then surely ifdef should be round whole if statement?)
//#ifdef GTA_PS2
		  	if (Cams[ActiveCam].CamTargetEntity==NULL)
//#endif	  	
		  	{
		  		ASSERTMSG(pTargetEntity, "Mark- King of sex but not programming");
		  		NeedToDoAJumpCutForGameCam=true;
		  	}
		}	
	
		//special special case for the fucking well bastarding ice cream van
		if (m_bPlayerIsInGarage)
		{
			if (pToGarageWeAreIn!=NULL)
			{
				if ((pToGarageWeAreIn->Type==CGarage::GARAGE_BOMBSHOP1)||
				(pToGarageWeAreIn->Type==CGarage::GARAGE_BOMBSHOP2)||(pToGarageWeAreIn->Type==CGarage::GARAGE_BOMBSHOP3))
				{
					//check for special case when "MR WHOOPEE" is reversing into a garage
					if (pTargetEntity->GetIsTypeVehicle())
					{
						if (((CVehicle*)pTargetEntity)->GetModelIndex() == MODELID_CAR_MRWHOOPEE)//check that van is entirely inside
						{
							if (ReqMode!=Cams[ActiveCam].Mode)
							{
								NeedToDoAJumpCutForGameCam=true;
							}
						}
					}
				}
			}
			
			
			// need to jumpcut to outside garage instead of a crappy interpolation.
			if (Cams[ActiveCam].CamTargetEntity)
			{
				// dotproduct test with TheCamPosition
				CVector v1 = Cams[ActiveCam].CamTargetEntity->GetPosition() - m_vecFixedModeSource;
				CVector v2 = Cams[ActiveCam].CamTargetEntity->GetPosition() - Cams[ActiveCam].Source;
				float dot = DotProduct(v1,v2);
				if (dot < 0.0f)
				{
					NeedToDoAJumpCutForGameCam = true;
				}								
			}
		}				
		
#ifdef GTA_PC	
		//!PC - CSceneEdit disabled	
/*		if (CSceneEdit::m_bEditOn)//Alans editor is on lets go into that mode
		{
			ReqMode=CCam::MODE_EDITOR;
		}*/
#endif		
		
		if(ReqMode!=Cams[ActiveCam].Mode && (m_uiTransitionState==TRANS_NONE || NeedToDoAJumpCutForGameCam==true))
		{
			if (NeedToDoAJumpCutForGameCam==true)
			{
			 	if (!(m_bPlayerIsInGarage)||(m_bJustCameOutOfGarage))//also can keep this in
			 	//just in case they cahnge there mind about interpolating
			 	{
					 if (!((ReqMode==CCam::MODE_FOLLOWPED)||(ReqMode==CCam::MODE_M16_1STPERSON)||
			 		(ReqMode==CCam::MODE_SNIPER) || (ReqMode==CCam::MODE_ROCKETLAUNCHER)|| (ReqMode==CCam::MODE_ROCKETLAUNCHER_HS)||
			 		(ReqMode==CCam::MODE_CAMERA)||(ReqMode==CCam::MODE_SYPHON)||
			 		(ReqMode==CCam::MODE_1STPERSON)||(ReqMode==CCam::MODE_SYPHON_CRIM_IN_FRONT)
			 		||(ReqMode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)||(m_bUseMouse3rdPerson)))
			 		{
			 			SetCameraDirectlyBehindForFollowPed_CamOnAString();				
					}
				}
				 Cams[ActiveCam].Mode=ReqMode;	
				 m_bJust_Switched=TRUE;
				 Cams[ActiveCam].m_cvecCamFixedModeVector=m_vecFixedModeVector;
				 TIDYREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
				 Cams[ActiveCam].CamTargetEntity=pTargetEntity;
				 REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
				 Cams[ActiveCam].m_cvecCamFixedModeSource=m_vecFixedModeSource;
		  		 Cams[ActiveCam].m_cvecCamFixedModeUpOffSet=m_vecFixedModeUpOffSet; 
				 Cams[ActiveCam].m_bCamLookingAtVector=m_bLookingAtVector;
			     Cams[ActiveCam].m_vecLastAboveWaterCamPosition=Cams[(ActiveCam + 1) % 2].m_vecLastAboveWaterCamPosition;
				 m_fCarZoomSmoothed=m_fCarZoomBase;
				 m_fPedZoomSmoothed=m_fPedZoomBase;
				 m_uiTransitionState=TRANS_NONE;
				 m_vecDoingSpecialInterPolation=false;

				 m_bStartInterScript=FALSE;
				 Cams[ActiveCam].ResetStatics=true;
//				REGREF(pTargetEntity, &(pTargetEntity));
//			 	REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));		
						
			}
			else if (m_bWaitForInterpolToFinish==false)
			{
				StartTransition(ReqMode);
//				REGREF(pTargetEntity, &(pTargetEntity));
//			 	REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));		
			
			}
		}
	    else if ((m_uiTransitionState != TRANS_NONE) && (ReqMode != Cams[ActiveCam].Mode))//&&((ReqMode==CCam::MODE_SYPHON)||(ReqMode==CCam::MODE_SYPHON_CRIM_IN_FRONT)))
	    {
       		bool SafeToInterpolate=true;
/*DWREMOVED       		if ((ReqMode==CCam::MODE_FIGHT_CAM)||(Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM))	     
			{
				SafeToInterpolate=false;
			}	       */
         	
/*DWREMOVED        		if ((ReqMode==CCam::MODE_FOLLOWPED)&&(Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM))	     
			{
				SafeToInterpolate=false;
			}	       */
         	
         	// Test whether a possible transition has finished
			// also if the user is being a wee prick and trying to break the syphon filter cam
	
			CVector PlayerFarAway;
			
			if (m_bWaitForInterpolToFinish==false)
			{
				if ((m_bLookingAtPlayer) && (m_uiTransitionState != TRANS_NONE))	
				{
					PlayerFarAway.x=FindPlayerPed()->GetPosition().x-GetMatrix().tx;
					PlayerFarAway.y=FindPlayerPed()->GetPosition().y-GetMatrix().ty;
					PlayerFarAway.z=FindPlayerPed()->GetPosition().z-GetMatrix().tz;

					if (pTargetEntity!=NULL)
					{ 
						if (pTargetEntity->GetIsTypePed())
						{
							if (PlayerFarAway.Magnitude()>17.5f)
							{
								if ((ReqMode==CCam::MODE_SYPHON)||(ReqMode==CCam::MODE_SYPHON_CRIM_IN_FRONT))
								{
									 //fuck it lets abort the transition
									 //they can keep interpolating
									m_bWaitForInterpolToFinish=true;
								}
							}
						}
					}
				}
	       	}
         	
         	
         	
         	if (m_bWaitForInterpolToFinish==true)
			{
				SafeToInterpolate=false;
			}
         	
         	if (SafeToInterpolate)
         	{ 
         		StartTransitionWhenNotFinishedInter(ReqMode);
//	    		REGREF(pTargetEntity, &(pTargetEntity));
//			 	REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));		
			}
	    }
	    else if ((ReqMode==CCam::MODE_FIXED)&&(pTargetEntity!=Cams[ActiveCam].CamTargetEntity)) 
	    {
	          if (m_bPlayerIsInGarage)
	          {
		            if( m_uiTransitionState == TRANS_NONE)
		            {
		            	StartTransition(ReqMode);
		            }
		            else
		            {
		            	StartTransitionWhenNotFinishedInter(ReqMode);
		    		}
//		    		REGREF(pTargetEntity, &(pTargetEntity));
//				 	REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));	
	          }
	    }
	}
	else //we are in scripting mode
	{
		
		 bool bJumpToFirstPerson=false; 
		 bool bFirstPersonWeaponActive=false;		
		 if ((m_bEnable1rstPersonCamCntrlsScript==true)||(m_bAllow1rstPersonWeaponsCamera==true))
		 {
		 	if (ReqMode==CCam::MODE_1STPERSON)
		 	{
				if (Cams[ActiveCam].Mode!=ReqMode)
				{
					bJumpToFirstPerson=true; 	
		 		}
		 	}
			else if (((PlayerWeaponMode.Mode == CCam::MODE_SNIPER)||(PlayerWeaponMode.Mode == CCam::MODE_1STPERSON)
			 		||(PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER) ||(PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER_HS)) &&(CPad::GetPad(0)->GetTarget())
			 		&&(m_bAllow1rstPersonWeaponsCamera))
			{
				bJumpToFirstPerson=true;
				bFirstPersonWeaponActive=true;
			}
			else
			{
				if (Cams[ActiveCam].Mode!=m_iModeToGoTo) //have just come out of first person mode
				//want to switch back to the mode that were in
				{
					m_bStartInterScript=TRUE;
					m_iTypeOfSwitch=TRANS_JUMP_CUT;
					CPad::GetPad(0)->EnableControlsCamera();
				}
			}
		 }
		
				
		if (((m_uiTransitionState==TRANS_NONE) && (m_bStartInterScript==TRUE)) && (m_iTypeOfSwitch==TRANS_INTERPOLATION))
		{
			ReqMode = m_iModeToGoTo;
		 	StartTransition(ReqMode);
//			REGREF(pTargetEntity, &(pTargetEntity));
//			REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));
		}	
		else if (((m_uiTransitionState!=TRANS_NONE) && (m_bStartInterScript==TRUE)) && (m_iTypeOfSwitch==TRANS_INTERPOLATION))
		{
			ReqMode = m_iModeToGoTo;
			StartTransitionWhenNotFinishedInter(ReqMode);
//			REGREF(pTargetEntity, &(pTargetEntity));
//			REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));
		}
		else if (((m_bStartInterScript==TRUE) && (m_iTypeOfSwitch==TRANS_JUMP_CUT))||(bJumpToFirstPerson))
		{
			 m_uiTransitionState=TRANS_NONE;
			 m_vecDoingSpecialInterPolation=false;
			 if ((m_bEnable1rstPersonCamCntrlsScript==true)&&(ReqMode==CCam::MODE_1STPERSON))
			 {
				Cams[ActiveCam].Mode=ReqMode;	
			 }
			 else if (bFirstPersonWeaponActive)
			 {
			 	Cams[ActiveCam].Mode=PlayerWeaponMode.Mode;
			 }
			 else
			 {	
			 	Cams[ActiveCam].Mode=m_iModeToGoTo;	
			 }
			 m_bJust_Switched=TRUE;
		
			 Cams[ActiveCam].ResetStatics=true;
			 Cams[ActiveCam].m_cvecCamFixedModeVector=m_vecFixedModeVector;
			 TIDYREF_NOTINWORLD(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
			 Cams[ActiveCam].CamTargetEntity=pTargetEntity;
			 REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
			 Cams[ActiveCam].m_cvecCamFixedModeSource=m_vecFixedModeSource;
	  		 Cams[ActiveCam].m_cvecCamFixedModeUpOffSet=m_vecFixedModeUpOffSet; 
			 Cams[ActiveCam].m_bCamLookingAtVector=m_bLookingAtVector;
			 Cams[ActiveCam].m_vecLastAboveWaterCamPosition=Cams[(ActiveCam + 1) % 2].m_vecLastAboveWaterCamPosition;
		 	 m_bJust_Switched=TRUE;
//			 REGREF(pTargetEntity, &(pTargetEntity));
//			 REGREF(Cams[ActiveCam].CamTargetEntity, &(Cams[ActiveCam].CamTargetEntity));	
			 m_fCarZoomSmoothed=m_fCarZoomBase;
			 m_fPedZoomSmoothed=m_fPedZoomBase;
	
		}	
	}	
	m_bStartInterScript=FALSE;
		 
	
	if (Cams[ActiveCam].CamTargetEntity==NULL)
	{
		TIDYREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
    	Cams[ActiveCam].CamTargetEntity=pTargetEntity;
    	REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
    }
	//do visibility shit 
	if( Cams[ActiveCam].Mode==CCam::MODE_FLYBY || ( pTargetEntity->GetIsTypePed() &&
	(  Cams[ActiveCam].Mode==CCam::MODE_1STPERSON || Cams[ActiveCam].Mode==CCam::MODE_SNIPER
	|| Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON || Cams[ActiveCam].Mode==CCam::MODE_CAMERA
	|| Cams[ActiveCam].Mode==CCam::MODE_HELICANNON_1STPERSON || Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER || Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_HS
//	|| Cams[ActiveCam].Mode==CCam::MODE_SNIPER_RUNABOUT || Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT|| Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS
//	|| Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON_RUNABOUT || Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM_RUNABOUT
//	|| Cams[ActiveCam].Mode==CCam::MODE_1STPERSON_RUNABOUT
	)))
	{
		if (FindPlayerPed()->m_nFlags.bIsVisible)
		{
		//pTargetEntity->m_nFlags.bIsVisible=false; //so the ped cannot see their feet
		FindPlayerPed()->m_nFlags.bIsVisible=false;
			
			// hack! I couldn't find a nicer way to do this - if the player is wearing goggles they need to disappear
			// instantly as soon as the cam goes first person (setting the visible flag in the hold task
			// does not do the job fast enough and the goggles appear onscreen for a frame) (JG)
			CTaskSimpleHoldEntity* pTask = FindPlayerPed()->GetPedIntelligence()->GetTaskHold();
		
			if (pTask && pTask->GetEntityBeingHeld())
			{
				pTask->GetEntityBeingHeld()->m_nFlags.bIsVisible = false;
			}
		}
	}
	else
	{
		//pTargetEntity->m_nFlags.bIsVisible=true; 
		FindPlayerPed()->m_nFlags.bIsVisible=true;
	}

	// DW - fix for player visibility with dodgy script stuff	
	if ( Cams[ActiveCam].Mode==CCam::MODE_FIXED)
	{
		FindPlayerPed()->m_nFlags.bIsVisible=gPlayerPedVisible;
	}
	
	bool JustGoneOutOfObbeCam=false;
	if (CanTryObbeCam==false)
	{
		if (WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL)
		{
			RestoreWithJumpCut();
			JustGoneOutOfObbeCam=true;//needed cause processing one frame behind;
			SetCameraDirectlyBehindForFollowPed_CamOnAString();
		}
	}
	if ((ModeAtStartOfCamControl!=Cams[ActiveCam].Mode)||(JustGoneOutOfObbeCam)||(Cams[ActiveCam].Mode==CCam::MODE_FOLLOWPED)||(Cams[ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING))
	{
		//want to trigger sound if the user has pressed select
		//to change the camera mode
		if ((CPad::GetPad(0)->CycleCameraModeJustDown())&&(!CReplay::ReplayGoingOn())&&((m_bLookingAtPlayer==true)||(WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL))&&(m_WideScreenOn==false))  
		{
			if (WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL)
			{
				if(JustGoneIntoObbeCamera==true)
				{
					//play the sound	
#ifdef USE_AUDIOENGINE
					if (!CPad::GetPad(0)->IsDisabled())
					{			
						AudioEngine.ReportFrontendAudioEvent(AUDIO_EVENT_FRONTEND_DISPLAY_INFO);
					}
#else //USE_AUDIOENGINE
					DMAudio.PlayFrontEndSound(AE_DISPLAY_INFO, 0);
#endif //USE_AUDIOENGINE
				}
			}			
			else //always play the sound
			{
#ifdef USE_AUDIOENGINE
				AudioEngine.ReportFrontendAudioEvent(AUDIO_EVENT_FRONTEND_DISPLAY_INFO);
#else //USE_AUDIOENGINE
				if (!CPad::GetPad(0)->IsDisabled())
				{
					DMAudio.PlayFrontEndSound(AE_DISPLAY_INFO, 0);
				}
#endif //USE_AUDIOENGINE
			}	
		}
	}

}
//////////////////////////
///////////////////////////////////////////////////////////////////////////
// NAME       : CamShake()
// PURPOSE    : Shakes the camera. The larger the number the more violent
//				the shake is. Also takes longer to die out for longer shakes.
//            : 
// RETURNS    : nowt
// PARAMETERS : Power. use 1 for huge big fuck off explosion
///////////////////////////////////////////////////////////////////////////

void CamShakeNoPos(CCamera *pCam, float Power)
{
	
	float	CurrentShakeForce=0.0f;
	// Find out what the current ShakeForce is.
	CurrentShakeForce = pCam->m_fCamShakeForce - (CTimer::GetTimeInMilliseconds() - pCam->m_uiCamShakeStart) * 0.001f ;
	CurrentShakeForce = MIN ( CurrentShakeForce, 2.0f );
	CurrentShakeForce = MAX ( CurrentShakeForce, 0.0f );

	// Find out whether the new one is stronger
	if (Power > CurrentShakeForce)	
	{
		pCam->m_fCamShakeForce = Power;
		pCam->m_uiCamShakeStart = CTimer::GetTimeInMilliseconds();
	}
	
	return;
}






#ifdef GTA_IDLECAM
bool CCamera::CanWeBeInIdleMode(void)
{
	// TEMP REMOVE IDLE CAM FOR NOW
	return false;

	bool FirstGoThroughIdle=false;
	if (m_bLookingAtPlayer==false)
	{
		return false;
	}
	
	CPlayerPed *PlayerPed;	
	PlayerPed=FindPlayerPed(); 
	//check if we are being arrested or are dead we want to go to those camera modes
	if (PlayerPed)
	{
		if ((PlayerPed->GetPedState()==PED_ARRESTED)||(PlayerPed->GetPedState()==PED_DEAD))
		{
			return false;
		}	
	}


	if (CPad::GetPad(0)->InputHowLongAgo()==0)
	{
		FirstGoThroughIdle=true;
	}

	if (CPad::GetPad(0)->InputHowLongAgo()<MAX_TIME_NO_INPUT_GOIDLE)	
	{
		return false;
	}

	if (m_bIdleOn==false)//we are not in idle mode yet
	{
	
		if (!((Cams[ActiveCam].Mode==CCam::MODE_FOLLOWPED)||(Cams[ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING)))
		//only allow the idle camera to go into the idle mode if we were in Followped or Camera On String Modes
		{
			return false;	
		}
		//now check that we can find a bird and that the line of sight is clear
		//use the camera that is not being used a process it once just to see
		Cams[(ActiveCam + 1) % 2].ResetStatics=true;
		Cams[(ActiveCam + 1) % 2].Source=Cams[ActiveCam].Source;
		if (Cams[(ActiveCam + 1) % 2].Process_CushyPillows_Arse()==false)
		{
			return false;
	
		}
	}
	else //ooh we are already in idle
	{
		
		if ((CTimer::GetTimeInMilliseconds()-m_uiTimeWeEnteredIdle)>MAX_TIME_IN_IDLE)//been in it for too long
		{
			m_uiTimeWeLeftIdle_StillNoInput=CTimer::GetTimeInMilliseconds();
			return false;
		}
		else
		{
			//now check that we can find a bird and that the line of sight is clear
			//use the camera that is not being used a process it once just to see
			if (Cams[(ActiveCam + 1) % 2].Process_CushyPillows_Arse()==false)
			{
				m_uiTimeWeLeftIdle_StillNoInput=CTimer::GetTimeInMilliseconds();
				return false;
			}
		}
		
	}
	
	if (FirstGoThroughIdle==false)
	{
		if ((CTimer::GetTimeInMilliseconds()-m_uiTimeWeLeftIdle_StillNoInput)<MAX_TIME_BEFORE_BACK_IN_IDLE)
		{
			return false;
		}
	}
	
	return true;
}
#endif	


///////////////////////////////////////////////////////////////////////////
// NAME       : CamShake()
// PURPOSE    : Shakes the camera. The larger the number the more violent
//				the shake is. Also takes longer to die out for longer shakes.
//            : Uses Distance as well i.e. The closer to the event the bigger
//            : the shake will be
// RETURNS    : nowt
// PARAMETERS : Power and Distance (Use 0 to 100 for distance)
///////////////////////////////////////////////////////////////////////////

void CCamera::CamShake(float Power, float xCoor, float yCoor, float zCoor)
{
	
	CVector WorkingDist;
	float DistOnGround=0.0f;
	float DistFromCam=0.0f;
	WorkingDist.x=Cams[ActiveCam].Source.x-xCoor;
	WorkingDist.y=Cams[ActiveCam].Source.y-yCoor;
	WorkingDist.z=Cams[ActiveCam].Source.z-zCoor;
	
	DistOnGround=CMaths::Sqrt(WorkingDist.x*WorkingDist.x + WorkingDist.y*WorkingDist.y);
	DistFromCam=CMaths::Sqrt(DistOnGround*DistOnGround + WorkingDist.z*WorkingDist.z);
	
//	char Str[250];
	//sprintf (Str, " Dist %f", DistFromCam) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
    //CDebug::PrintToScreenCoors(Str, 3, 8);
	
	
	if (DistFromCam>100){DistFromCam=100;}//clips distance
	if (DistFromCam<0){DistFromCam=0;}//clips distance
	
	float	CurrentShakeForce=0.0f;
	float DistanceFactor=-0.01*DistFromCam+1;//this is the equivalent to y=0.1x+1
	// Find out what the current ShakeForce is.
	CurrentShakeForce = m_fCamShakeForce - (CTimer::GetTimeInMilliseconds() - m_uiCamShakeStart) * 0.001f ;
	CurrentShakeForce=	CurrentShakeForce*DistanceFactor;
	CurrentShakeForce = MIN ( CurrentShakeForce, 2.0f );
	CurrentShakeForce = MAX ( CurrentShakeForce, 0.0f );

	// Find out whether the new one is stronger
	Power*=DistanceFactor*0.35f;		// (*0.35 - tone down effect for SA)
	if (Power > CurrentShakeForce)	
	{
		m_fCamShakeForce = Power;
		m_uiCamShakeStart = CTimer::GetTimeInMilliseconds();
	}
}

///////////////////////////////////////////////////////////////////////////
// NAME       : ClearPlayerMode()
// PURPOSE    : Goes back to the standard player mode. Clears the mode set
//				by SetNewPlayerMode
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
void CCamera::ClearPlayerWeaponMode(void)
{
//	this->SetMotionBlur(CTimeCycle::m_fCurrentBlurRed,CTimeCycle::m_fCurrentBlurGreen,CTimeCycle::m_fCurrentBlurBlue, this->GetMotionBlur(), MB_BLUR_LIGHT_SCENE); 
	PlayerWeaponMode.Mode = CCam::MODE_NONE;
	PlayerWeaponMode.MaxZoom = CCam::PED_MAX_ZOOM_OUT;
	PlayerWeaponMode.MinZoom = CCam::PED_MAX_ZOOM_IN;
	PlayerWeaponMode.Duration = 0.0f;		// Not needed really
}

//////////////////////////////////////
void CCamera::DontProcessObbeCinemaCamera()
{
	bDidWeProcessAnyCinemaCam = false;
}


//
// name:		GetScreenRect
// description:	Get the screen rectangle
void CCamera::GetScreenRect(CRect& rect)
{
	rect.left = 0;
	rect.right = SCREEN_WIDTH;
	if(m_WideScreenOn)
	{
		rect.top = (m_ScreenReductionPercentage/100 * (SCREEN_HEIGHT/2))-(22*HUD_MULT_Y);
		rect.bottom = (SCREEN_HEIGHT-(m_ScreenReductionPercentage/100 * (SCREEN_HEIGHT/2)))-(14*HUD_MULT_Y);
	}
	else
	{
		rect.top = 0;
		rect.bottom = SCREEN_HEIGHT;
	}	
}

///////////////////////////////////////////////////////////
void CCamera::DrawBordersForWideScreen()
{
	CRect screenRect;
	
	GetScreenRect(screenRect);
	
	//(float left, float bottom, float right, float top)
	if ((GetBlurType()==MB_BLUR_NONE) || (GetBlurType()==MB_BLUR_LIGHT_SCENE)) //INTRO) && (GetBlurType()!=MB_BLUR_INTRO2) &&  (GetBlurType()!=MB_BLUR_INTRO3) && (GetBlurType()!=MB_BLUR_INTRO4))
	{
		if (m_ScreenReductionPercentage>0)
		{
			SetMotionBlurAlpha(80);//255,255,255,CUT_SCENE_MOTION_BLUR+MOTION_BLUR_ALPHA_DECREASE),MB_BLUR_CUT_SCENE);    	//SetMotionBlurAlpha(110);//255,255,255,(CUT_SCENE_MOTION_BLUR+MOTION_BLUR_ALPHA_DECREASE),MB_BLUR_CUT_SCENE);    //it will really be 150, this is just we don't have
		}																//abrupt jump
		else
		{
		 	SetMotionBlurAlpha(80);//255,255,255,CUT_SCENE_MOTION_BLUR+MOTION_BLUR_ALPHA_DECREASE),MB_BLUR_CUT_SCENE);    	//SetMotionBlurAlpha(110);//255,255,255,(0+MOTION_BLUR_ALPHA_DECREASE),MB_BLUR_CUT_SCENE);    //it will really be 0, this is just we don't have
		}
	}

//	CSprite2d::DrawRect( CRect(0, (m_ScreenReductionPercentage/100 * (SCREEN_HEIGHT/2))-8, SCREEN_WIDTH, 0), CRGBA(0,0,0,255));
//	CSprite2d::DrawRect( CRect(0, SCREEN_HEIGHT, SCREEN_WIDTH, (SCREEN_HEIGHT-(m_ScreenReductionPercentage/100 * (SCREEN_HEIGHT/2)))-8), CRGBA(0,0,0,255));

	// the -22 and -14 stuff here is to shove the widescreen up as we now need space for 3 lines of text at the bottom
	// during cutscenes.   i doubt if the 15 pixels will make a difference to the widescreen look?!, but i am doing it anyway!!!
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)NULL);
	
	CSprite2d::DrawRect( CRect(-5, screenRect.top, SCREEN_WIDTH+5, -5), CRGBA(0,0,0,255));
	CSprite2d::DrawRect( CRect(-5, SCREEN_HEIGHT+5, SCREEN_WIDTH+5, screenRect.bottom), CRGBA(0,0,0,255));

}


void CCamera::Enable1rstPersonCamCntrlsScript(void)
{
	m_bEnable1rstPersonCamCntrlsScript=true; 

}	
void CCamera::Enable1rstPersonWeaponsCamera(void)
{	
	m_bAllow1rstPersonWeaponsCamera=true;
}

#define AUDIO_FADE_TIME_FRACTION	(0.3f)
#define MIN_AUDIO_FADE_TIME_SEC		(0.3f)
#define MIN_TIME_AFTER_AUDIO_FADE_OUT_SEC	(0.1f)

////////////////////////////
void CCamera::Fade(float FadeOutTime,Int16 OutOrIn)
{
	m_bFading = TRUE;
	m_iFadingDirection=OutOrIn;
	m_fTimeToFadeOut=FadeOutTime;
	m_uiFadeTimeStarted=CTimer::GetTimeInMilliseconds();   

	//Colin - Force all audio fade ins, whatever the script may request.
	if((m_bIgnoreFadingStuffForMusic == false) || (OutOrIn == FADE_IN))
	{
		m_bMusicFading=TRUE;
		m_iMusicFadingDirection=OutOrIn;
		m_fTimeToFadeMusic=MIN(MAX(FadeOutTime * AUDIO_FADE_TIME_FRACTION, MIN_AUDIO_FADE_TIME_SEC),
			FadeOutTime);
		if(OutOrIn == FADE_OUT)
		{
			m_fTimeToWaitToFadeMusic = FadeOutTime - m_fTimeToFadeMusic;
			//Leave a short safety zone at the end of the audio fade to ensure we fully fade out.
			m_fTimeToFadeMusic = MAX(m_fTimeToFadeMusic - MIN_TIME_AFTER_AUDIO_FADE_OUT_SEC, 0.0f);
		}
		else
		{
			m_fTimeToWaitToFadeMusic = 0.0f;
		}
		m_uiFadeTimeStartedMusic=CTimer::GetTimeInMilliseconds(); 
	}
}

// name:		FinishCutscene
// description:	Setup variables for cutscene to be finished
void CCamera::FinishCutscene()
{
	SetPercentAlongCutScene(100);
	m_fPositionAlongSpline = 1.0f;
	m_bcutsceneFinished = true;
}


///////////////////////////////////////////////////////////////////////////
// NAME       : FindCamFOV()
// PURPOSE    : Returns the cameras FOV.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

float CCamera::FindCamFOV(void)
{
#ifndef MASTER
	if (WorldViewerBeingUsed)
	{
		return (Cams[2].FOV);
	}
	else
#endif	
	{
		return (Cams[ActiveCam].FOV);
	}
}

bool CCamera::Find3rdPersonCamTargetVector(float fRange, CVector vecGunMuzzle, CVector &vecSource, CVector &vecTarget)
{
	// shouldn't really have to check this cause well it shouldn't have been called otherwise
//	if(Cams[ActiveCam].Mode != CCam::MODE_FOLLOWPED || !m_bUseMouse3rdPerson)
//		return false;
// AF: Cannot look behind while aiming
/*	if(CPad::GetPad(0)->GetLookBehindForPed())
	{
		// just use fire forward
		vecSource = vecGunMuzzle;
		vecTarget = vecSource + fRange * Cams[ActiveCam].CamTargetEntity->GetMatrix().GetForward();
	}
	else*/
	{
		// need to work out what angles from the camera's front vector, the target is positioned at
		float fScreenAngle, fScreenPosMult;
		
		fScreenAngle = DEGTORAD(0.5f*Cams[ActiveCam].FOV);
		fScreenPosMult = 2.0f*(m_f3rdPersonCHairMultX - 0.5f);
		float fRightMult = fScreenPosMult*CMaths::Tan(fScreenAngle);
		
		fScreenPosMult = 2.0f*(0.5f - m_f3rdPersonCHairMultY);
		float fUpMult = fScreenPosMult*(1.0f/CDraw::GetAspectRatio())*CMaths::Tan(fScreenAngle);

		vecSource = Cams[ActiveCam].Source;
		vecTarget = Cams[ActiveCam].Front;

		if(Cams[ActiveCam].Mode==CCam::MODE_TWOPLAYER_IN_CAR_AND_SHOOTING)
		{
			Cams[ActiveCam].Get_TwoPlayer_AimVector(vecTarget);
		}
		else
		{
			// rotate the vector up to point through the target
			vecTarget += Cams[ActiveCam].Up * fUpMult;//CMaths::Tan(fAimAngleY);
			// rotate the vector right to point through the target
			vecTarget += CrossProduct(Cams[ActiveCam].Front, Cams[ActiveCam].Up) * fRightMult;//CMaths::Tan(fAimAngleX);

			vecTarget.Normalise();
		}

		// want to move source of gunshot vector along to the gun muzzle
		// so can't hit anything behind the gun!
		float fGunVec = DotProduct(vecGunMuzzle - vecSource, vecTarget);
		vecSource += fGunVec * vecTarget;
		
		// finally use the unit shot vector to calculate the end target using the supplied range
		vecTarget = vecSource + fRange * vecTarget;
		
		return true;
	}
	return false;
}


float CCamera::Find3rdPersonQuickAimPitch()
{
	float fScreenAngle = DEGTORAD(0.5f*Cams[ActiveCam].FOV);
	float fScreenPosMult = 2.0f*(0.5f - m_f3rdPersonCHairMultY);
	float fUpMult = fScreenPosMult*(1.0f/CDraw::GetAspectRatio())*CMaths::Tan(fScreenAngle);

	float fPitch = Cams[ActiveCam].Alpha + CMaths::ATan(fUpMult);
	return -fPitch;

//	// first get pitch offset of targeting crosshair from camera centre
//	float fPitch = 1.8f*(0.5f - m_f3rdPersonCHairMultY) * 0.5f*Cams[ActiveCam].FOV;
//	fPitch = DEGTORAD(fPitch);

	// then add the pitch of the camera itself
//	fPitch += CMaths::ASin(MAX(-1.0f, MIN(1.0f, Cams[ActiveCam].Front.z)));
	
//	return -fPitch;
}


///////////////////////////////////////////////////////////////////////////
// NAME       : GetCutSceneFinishTime
// PURPOSE    : 
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
UInt32 CCamera::GetCutSceneFinishTime(void)
{
	if (Cams[ActiveCam].Mode==CCam::MODE_FLYBY)
	{ 
		return Cams[ActiveCam].m_uiFinishTime;
	}
	else if (Cams[(ActiveCam + 1) % 2].Mode==CCam::MODE_FLYBY)
	{
		return Cams[(ActiveCam + 1) % 2].m_uiFinishTime;
	}
	else
	return NULL;//not in fly by mode
}

Bool8 CCamera::GetFading(void)
{
	return m_bFading;
}

int CCamera::GetFadingDirection(void)
{
	if (m_bFading==FALSE)
	{
		return NOT_FADING;
	}
	else if (m_iFadingDirection==FADE_IN)
	{
		return FADING_IN;
	}
	else
	{
		ASSERTMSG(m_iFadingDirection==FADE_OUT, "Mark is a fat fool");
		return FADING_OUT;
	}
}



///////////////////////////////////////////////////////////////////////////
// NAME       : Get_Just_Switched_Status()
// PURPOSE    : Returns the status of the Justswitched flag Raymond need this to prevent 
//			  : A massive doppler shift when the camera jumps somewhere
// RETURNS    : TRUE OR FALSE depending if we've just switched
// PARAMETERS :None
///////////////////////////////////////////////////////////////////////////

bool8 CCamera::Get_Just_Switched_Status(void)
{
	return m_bJust_Switched;
}


//////////////////////////////////////////
int CCamera::GetScreenFadeStatus(void)
{
	if (m_fFloatingFade==0.0f)
	{
		return CLEAR_SCREEN;
	}
	else if (m_fFloatingFade==255.0f)
	{
		return DARK_SCREEN; ///means completely faded
	}
	else 
	{
		return IN_MIDDLE_OF_FADING; 
	}
	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : GetGameCamPosition()
// PURPOSE    : Get the game cameras position (Not the debug cam)			
// RETURNS    : The game cameras position (Not the debug cam)
// PARAMETERS : Nothing
///////////////////////////////////////////////////////////////////////////
CVector* CCamera::GetGameCamPosition(void)
{
	return &m_vecGameCamPos;
}	




///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CCamera::GetLookingLRBFirstPerson(void)
{
	if ((Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)&&(Cams[ActiveCam].DirectionWasLooking!=LOOKING_FORWARD))
	{
		return true;
	}
	else
	{
		return false;
	}

}
int CCamera::GetLookDirection(void)/////////
{
	if ((Cams[ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING)||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)
	||(Cams[ActiveCam].Mode==CCam::MODE_BEHINDBOAT)||(Cams[ActiveCam].Mode==CCam::MODE_FOLLOWPED))
	{
		if (Cams[ActiveCam].DirectionWasLooking!=LOOKING_FORWARD)
		{
			return Cams[ActiveCam].DirectionWasLooking;
		}
	}
	return LOOKING_FORWARD;
}
bool CCamera::GetLookingForwardFirstPerson(void)
{
	if ((Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)&&(Cams[ActiveCam].DirectionWasLooking==LOOKING_FORWARD))
	{
		return true;
	}
	else
	{
		return false;
	}

}

//Function Name: GetArrPosForVehicleType
//Comments: This function relates to the following variables

//float TiltTopSpeed[MAX_NUM_VEHICLE_TYPE]={0.035f, 0.035f, 0.035f, 0.035f, 0.035f};
//float TiltSpeedStep[MAX_NUM_VEHICLE_TYPE]={0.0314f, 0.0314f, 0.0314f, 0.0314f, 0.0314f};
//float TiltOverShoot[MAX_NUM_VEHICLE_TYPE]={1.05f, 1.05f, 1.05f, 1.05f, 1.05f};
//float ZOOM_ONE_DISTANCE[MAX_NUM_VEHICLE_TYPE]={0.05f, 0.05f, 0.05f, 0.05f, 0.05f}; // was.25
//float ZOOM_TWO_DISTANCE[MAX_NUM_VEHICLE_TYPE]={1.9f, 1.9f, 1.9f, 1.9f, 1.9f};//(1.5f)
//float ZOOM_THREE_DISTANCE[MAX_NUM_VEHICLE_TYPE]={15.9f, 15.9f, 15.9f, 15.9f, 15.9f}; // was 2.9 default
//float ZmOneAlphaOffset[MAX_NUM_VEHICLE_TYPE]={-0.1f, -0.1f, -0.1f, -0.1f, -0.1f};
//float ZmTwoAlphaOffset[MAX_NUM_VEHICLE_TYPE]={0.045f, 0.045f, 0.045f, 0.045f, 0.045f};//0.17f;
//float ZmThreeAlphaOffset[MAX_NUM_VEHICLE_TYPE]={0.005f, 0.005f, 0.005f, 0.005f, 0.005f};//0.500311f;
//Each bit of the array is related to this
//{Car==0, Bike==1, Heli==2, Plane==3, Boat==4}
bool CCamera::GetArrPosForVehicleType(int eTypeOfVehicle, int& ArrPosResult)
{
	switch (eTypeOfVehicle)
	{
		case APR_CAR: 	ArrPosResult=0; return true;
		case APR_BIKE:	ArrPosResult=1;	return true;	
		case APR_HELI:	ArrPosResult=2;	return true;	
		case APR_PLANE:	ArrPosResult=3;	return true;
		case APR_BOAT: 	ArrPosResult=4;	return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : GetPositionAlongSpline()
// PURPOSE    : 
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
float CCamera::GetPositionAlongSpline(void)
{
	return m_fPositionAlongSpline;
}


///////////////////////////////////////////////////////////////////////////
void CCamera::InitialiseCameraForDebugMode(void)
{
#ifndef MASTER

	if (!PS2Keyboard.GetKeyDown(PS2_KEY_H))
	{
		if (FindPlayerVehicle(-1, true) != NULL)
		{
			Cams[2].Source = FindPlayerVehicle(-1, true)->GetPosition();
		}
		else if(FindPlayerPed())
	 	{
	 		Cams[2].Source = FindPlayerPed()->GetPosition();
		}
	}
	else
	{
		Cams[2].Source = Cams[0].Source; // holds the camera where it is.
	}
	
	Cams[2].Alpha=0.0f;
	Cams[2].Beta=0.0f;
	Cams[2].Mode=CCam::MODE_DEBUG;
#endif	
}

///////////////////////////////////////////////////////////////////////////
bool	CCamera::IsItTimeForNewcam(Int32 MovieCamMode, Int32 TimeWhenStarted)
{
	CVector	Diff;
	float NearClipWheelCam=0.6f;
	float NearClipCarOnFloor=0.0f;
	if (MovieCamMode < 0) return true;
	static float maxTimeForAnyCam = 20000; // 30 secs is max time to sit in any movie cam!
	static float aNewReasonableMaxTimeForSomeCams = 15000; // 30 secs is max time to sit in any movie cam!

	float t = CTimer::GetTimeInMilliseconds();
	float tExpired = t-TimeWhenStarted;

	if (tExpired > maxTimeForAnyCam) return true;
	
	// Les requests cinematic camera to be selectable
	float stickX = CPad::GetPad(0)->GetRightStickX();
	static bool gRightStickCinematicCameraSwitchReturnedToZero = true;
	float stickXAbs = CMaths::Abs(stickX);
	float stickRange = 128.0f;
	if (stickXAbs<=stickRange*0.25f)
		gRightStickCinematicCameraSwitchReturnedToZero = true;
	else if (stickXAbs >stickRange*0.75f && gRightStickCinematicCameraSwitchReturnedToZero)
	{
		if (stickX>0.0f)
			gCinematicModeSwitchDir = 1;
		else
			gCinematicModeSwitchDir = -1;
		
		gRightStickCinematicCameraSwitchReturnedToZero = false;
		return true;
	}
	// End Les requests cinematic camera to be selectable
		
	switch (MovieCamMode)
	{
		case MOVIECAM0:		// WheelCam on player
			{
				CVehicle *pVehicle=FindPlayerVehicle();
				if (pVehicle!=NULL)
				{
					if (pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
					{
						if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
						{
							return true;
						}						
					}
					
					if (pVehicle->GetModelIndex()==MODELID_CAR_RHINO)
					{
						return true;
					}
					
					if (!(CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), Cams[ActiveCam].Source, true, false, false, false, false, false, false)))
					{
						return true;
					}
				}
				if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 5000) return true;
		//		SetNearClipScript(NearClipWheelCam);
		
				RwCameraSetNearClipPlane(Scene.camera, (RwReal) NORMAL_NEAR_CLIP*0.5f);			
			}
			break;

		case MOVIECAM1:		// Fixed cam just above the road. Quite far away.
//			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 3000) return true;
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;
			if (FindPlayerVehicle()!=NULL)
			{
				if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
				{
					if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
					{
						return true;
					}	
				}
			}
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
			Diff = FindPlayerCoors() - m_vecFixedModeSource;
			Diff.z = 0.0f;
			if (Diff.Magnitude() > DISTCAM1_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			if (Diff.Magnitude() < DISTCAM1_RELEASE_TOO_CLOSE) return true;
			break;
	
		case MOVIECAM2:		// Fixed right on front just above road
//			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 1500) return true;
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;
			if (FindPlayerVehicle()!=NULL)
			{
				if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
				{
					if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
					{
						return true;
					}	
				}
			}
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
			Diff = FindPlayerCoors() - m_vecFixedModeSource;
			Diff.z = 0.0f;
			if (Diff.Magnitude()<2.0f)
			{
				float TempNearClip=Diff.Magnitude()/2.0f;
				if (TempNearClip<0.05f){TempNearClip=0.05f;}//must be right above us
				SetNearClipScript(TempNearClip);
			}
			if (Diff.Magnitude() > DISTCAM2_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			if (Diff.Magnitude() <DISTCAM2_RELEASE_TOO_CLOSE) return true;

			RwCameraSetNearClipPlane(Scene.camera, (RwReal) NORMAL_NEAR_CLIP*0.5f);	

			break;

		case MOVIECAM3:		// Fixed cam quite high up
//			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 3000) return true;
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
			Diff = FindPlayerCoors() - m_vecFixedModeSource;
			Diff.z = 0.0f;
			if (Diff.Magnitude() > DISTCAM3_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			
			RwCameraSetNearClipPlane(Scene.camera, (RwReal) NORMAL_NEAR_CLIP*0.5f);				
			break;

		case MOVIECAM4:		// 1st person camera		
			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 3000) return true;
			break;

		case MOVIECAM5:		// Fixed cam. Just above roofs of cars
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;		
//			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 2000) return true;
			if (FindPlayerVehicle()!=NULL)
			{
				if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
				{
					if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
					{
						return true;
					}	
				}
			}
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
			Diff = FindPlayerCoors() - m_vecFixedModeSource;
			Diff.z = 0.0f;
			if (Diff.Magnitude() > DISTCAM5_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			break;

		case MOVIECAM6:		// Standard camera
			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 3000) return true;
			break;
	
		case MOVIECAM7:		// Chase cam (try to find copper chasing us)
			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 2000 &&
				!FindPlayerVehicle()->GetIsOnScreen()) return true;

			break;
		case MOVIECAM8:		// Chase cam (try to find copper chasing us)
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;	
			if (FindPlayerVehicle()!=NULL)
			{
				if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
				{
					if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
					{
						return true;
					}	
				}
			}
			if (!(CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), Cams[ActiveCam].Source, true, false, false, false, false, false, false)))
			{
				return true;
			}
			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 1000) return true;
			SetNearClipScript(NearClipWheelCam);
			break;

		case MOVIECAM9:		// Fixed cam. Just above roofs of cars
//			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
//			Diff = FindPlayerCoors() - m_vecFixedModeSource;
//			Diff.z = 0.0f;
//			if (Diff.Magnitude() > DISTCAM9_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			break;
		case MOVIECAM10:		// Fixed cam. Just above roofs of cars
//			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
//			Diff = FindPlayerCoors() - m_vecFixedModeSource;
//			Diff.z = 0.0f;
//			if (Diff.Magnitude() > DISTCAM10_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			break;
		case MOVIECAM11:		// Fixed cam. Just above roofs of cars
//			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
//			Diff = FindPlayerCoors() - m_vecFixedModeSource;
//			Diff.z = 0.0f;
//			if (Diff.Magnitude() > DISTCAM11_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			break;
		case MOVIECAM12:		// Fixed cam. Just above roofs of cars
//			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
//			Diff = FindPlayerCoors() - m_vecFixedModeSource;
//			Diff.z = 0.0f;
//			if (Diff.Magnitude() > DISTCAM12_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
			break;
//		case MOVIECAM13:		// Top down
//			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 5000) return true;
//			break;
		//all helicopter cams
		case MOVIECAM14:  
			// Clipped to Side of Heli
/*			if (FindPlayerVehicle()!=NULL)
			{
				if (!(CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), Cams[ActiveCam].Source, true, false, false, false, false, false, false)))
				{
					return true;
				}
			}
			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 8000) return true;
			SetNearClipScript(NearClipWheelCam);*/
			break;
		case MOVIECAM15: //straight in front of the player
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;		
			if (FindPlayerVehicle()!=NULL)
			{
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
				Diff = FindPlayerCoors() - m_vecFixedModeSource;
				Diff.z = 0.0f;
				if (Diff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_ONE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
				if (Diff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_ONE) return true;
			}
			break;
		case MOVIECAM16: //underneath infront of and to the side of the player
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;		
			if (FindPlayerVehicle()!=NULL)
			{
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
				Diff = FindPlayerCoors() - m_vecFixedModeSource;
				Diff.z = 0.0f;
				if (Diff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_TWO /*&& DotProduct(FindPlayerSpeed(), Diff) > 0.0f*/) return true;
				if (Diff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_TWO) return true;
			}
			break;
		case MOVIECAM17:
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;		
			if (FindPlayerVehicle()!=NULL)
			{
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
				Diff = FindPlayerCoors() - m_vecFixedModeSource;
				Diff.z = 0.0f;
				if (Diff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_THREE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return true;
				if (Diff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_THREE) return true;
			}
			break;		
		case MOVIECAM18:
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;		
			//Directly above the player 
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
				Diff = FindPlayerCoors() - m_vecFixedModeSource;
				if (Diff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_FOUR /*&& DotProduct(FindPlayerSpeed(), Diff) > 0.0f*/) return true;
				if (Diff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_FOUR) return true;
			break;
		case MOVIECAM19:
			if (tExpired > aNewReasonableMaxTimeForSomeCams) return true;		
			//Directly above the player To the side
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false)) return true;
				Diff = FindPlayerCoors() - m_vecFixedModeSource;
				Diff.z = 0.0f;
				if (Diff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_FIVE /*&& DotProduct(FindPlayerSpeed(), Diff) > 0.0f*/) return true;
				if (Diff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_FIVE) return true;
			break;
		case MOVIECAM20:
			if (!Cams[ActiveCam].Process_DW_HeliChaseCam(true))
				return true;
			break;
		case MOVIECAM21:
			if (!Cams[ActiveCam].Process_DW_CamManCam(true))
				return true;
			break;
		case MOVIECAM22:
			if (!Cams[ActiveCam].Process_DW_BirdyCam(true))
				return true;
			break;
		case MOVIECAM23:
			if (!Cams[ActiveCam].Process_DW_PlaneSpotterCam(true))
				return true;
			break;
		case MOVIECAM24:
			if (!Cams[ActiveCam].Process_DW_DogFightCam(true))
				return true;
			break;
		case MOVIECAM25:
			if (!Cams[ActiveCam].Process_DW_FishCam(true))
				return true;
			break;
		case MOVIECAMPLANE1:
			if (!Cams[ActiveCam].Process_DW_PlaneCam1(true))
				return true;
			break;
		case MOVIECAMPLANE2:
			if (!Cams[ActiveCam].Process_DW_PlaneCam2(true))
				return true;
			break;
		case MOVIECAMPLANE3:
			if (!Cams[ActiveCam].Process_DW_PlaneCam3(true))
				return true;
			break;
		case CAM_ON_A_STRING_LAST_RESORT:
			if (CTimer::GetTimeInMilliseconds() > TimeWhenStarted + 5000) return true;
			break;
	}
	

	return false;
}

#ifdef GTA_TRAIN
/////////////////////////////////////////////////////////////
void CCamera::NoTrainNodeFoundDefaults(void)
{
	static int PositionInControlLoop=0;//set to ridicuously high value so switch will alwys happen the first time
	CVector FixedCamPos;
	CVector FixedCamPosOffSet(0,0,0);
	CVector DistFromTrackOffset;
		
	CVector2D DistFromCurrNode;
	float MinDistanceAwayFromTrain=125; //acceptable distance
	float MaxDistanceAwayFromTrain=225; //acceptable distance away from train
	float NodeDistance=0.0f;
	int MaximumNumberOfModes=2; //is the same as the  number of cases so its really 3
	int DesiredPositionInControlLoop=PositionInControlLoop;

	extern Int16 NumTrackNodes;
	extern CTrainNode* pTrackNodes;
	extern float EngineTrackSpeed[];
	bool OkToRemainInPreferredLoop=false;
	bool AcceptableDistandReached=false;
	bool FirstRun=false;
	float AngleBetweenPoints=0.0f;
	float TrainSpeed=0.0f;
	CVector Target;

	if (m_bInitialNoNodeStaticsSet==false)//nonode meaning train nodes 
	{
		m_uiTimeLastChange=CTimer::GetTimeInMilliseconds();
		PositionInControlLoop=0;
		m_bInitialNoNodeStaticsSet=true;
		FirstRun=true;
	}

	CTrain* LocalPointerToTheTrain=((CTrain*)pTargetEntity);
	//first work work if we have to change
	if ((CTimer::GetTimeInMilliseconds()-m_uiTimeLastChange)>m_uiLongestTimeInMill)
	{
		DesiredPositionInControlLoop=(PositionInControlLoop+1);
	}
	//now clip 
	if (DesiredPositionInControlLoop>MaximumNumberOfModes)
	{
		DesiredPositionInControlLoop=0;
	}



	TrainSpeed=EngineTrackSpeed[0];

	if (TrainSpeed<0.01f)//we are slowing down to stop, don't bother changing, it just looks gash 
	{
		DesiredPositionInControlLoop=PositionInControlLoop;
	}
	//if we are changing to a fixed camera then this is dependant on the nodes, therefore
	//we need to check that things are acceptable
	if ((DesiredPositionInControlLoop!=PositionInControlLoop)||(FirstRun==true))//always set this if it's the first run
	{
		m_bLookingAtVector=false;//required for a sort of restore for the fixed camera stuff
		switch (DesiredPositionInControlLoop)
		{
		case 0:
		TakeControl(LocalPointerToTheTrain, CCam::MODE_CAM_RUNNING_SIDE_TRAIN, TRANS_INTERPOLATION);
		break;
		case 1:
		TakeControl(LocalPointerToTheTrain, CCam::MODE_IM_THE_PASSENGER_WOOWOO, TRANS_INTERPOLATION);
		break;
		case 2:
		TakeControl(LocalPointerToTheTrain, CCam::MODE_CAM_ON_TRAIN_ROOF, TRANS_INTERPOLATION);
		break;
		//case 4:
		//TakeControl(LocalPointerToTheTrain, CCam::MODE_BLOOD_ON_THE_TRACKS, TRANS_INTERPOLATION);
		//break;
		/*
		case 4:
		SetCamPositionForFixedMode(FixedCamPos, FixedCamPosOffSet);
		TakeControlNoEntity(Target,   TRANS_INTERPOLATION);
		break;
		*/
		}
		m_uiTimeLastChange=CTimer::GetTimeInMilliseconds();
		PositionInControlLoop=DesiredPositionInControlLoop;
	}	
}
#endif //end #ifdef GTA_TRAIN

//
// name:		CalculateHeightAboveGround
// description:	Calculate the height above the ground
#define RECALCULATEHEIGHT_DIST 20.0f
float CCamera::CalculateGroundHeight(uint32 nBoundBoxPos)
{
	static float fGroundHeight = 0.0f;
	static float fGroundHeightBase = 0.0f;
	static float fGroundHeightTop = 0.0f;
	static CVector fLastPosnCalcuation(0.0f, 0.0f, 0.0f);
	CVector camPosn = GetPosition();
	
	// Should I recalculate ground level
	if(CMaths::Abs(fLastPosnCalcuation.x - camPosn.x) > RECALCULATEHEIGHT_DIST ||
		CMaths::Abs(fLastPosnCalcuation.y - camPosn.y) > RECALCULATEHEIGHT_DIST ||
		CMaths::Abs(fLastPosnCalcuation.z - camPosn.z) > RECALCULATEHEIGHT_DIST)
	{
		CColPoint TestColPoint;
		CEntity* pTestEntity;
		
		// If we get a collision, the height isnt the collision point but the base of the
		// object we hit
		camPosn.z = 1000.0f;
		if(CWorld::ProcessVerticalLine(camPosn, -1000.0f, TestColPoint, pTestEntity, true, false, false, false))
		{
			fGroundHeight = TestColPoint.GetPosition().z;
			fGroundHeightTop = pTestEntity->GetPosition().z + CModelInfo::GetColModel(pTestEntity->GetModelIndex()).GetBoundBoxMax().z;
		
			if (CModelInfo::GetColModel(pTestEntity->GetModelIndex()).GetBoundBoxMax().x - CModelInfo::GetColModel(pTestEntity->GetModelIndex()).GetBoundBoxMin().x > 120.0f ||
				CModelInfo::GetColModel(pTestEntity->GetModelIndex()).GetBoundBoxMax().y - CModelInfo::GetColModel(pTestEntity->GetModelIndex()).GetBoundBoxMin().y > 120.0f)
			{		// We've hit a really big thing. Probably a mountain or something. Use the z coordinate as is.
				fGroundHeightBase = TestColPoint.GetPosition().z;
			}
			else
			{		// We've hit a relatively narrow thing (building or smaller). Use the base of the object.
				fGroundHeightBase = pTestEntity->GetPosition().z;
				fGroundHeightBase += CModelInfo::GetColModel(pTestEntity->GetModelIndex()).GetBoundBoxMin().z;
			}
			
			if(fGroundHeightBase < 0.0f)
				fGroundHeightBase = 0.0f;
		}

		fLastPosnCalcuation = GetPosition();
	}
	
	if(nBoundBoxPos==2)
		return fGroundHeightTop;
	else if(nBoundBoxPos==1)
		return fGroundHeight;

	return fGroundHeightBase;
}

#ifndef FINAL
//
// name:		WhatAmILookingAt
// description:	Describe what camera is looking at
void WhatAmILookingAt(CCamera* pCamera)
{
	// debug code to find out what the player is looking at
	static CEntity* pLookAtEntity = NULL;
	CEntity* pLod;
	
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_F9, KEYBOARD_MODE_STANDARD, "What am I looking at"))
		gbWhatAmILookingAt = !gbWhatAmILookingAt;
	if(gbWhatAmILookingAt)
	{
		CColPoint colPoint;
		// make sure previous entity is visible
		if(pLookAtEntity)
		{
			pLookAtEntity->m_nFlags.bIsVisible = true;
			pLod = pLookAtEntity->GetLod();
			while(pLod)
			{
				pLod->m_nFlags.bIsVisible = true;
				pLod = pLod->GetLod();
			}
			pLookAtEntity = NULL;
		}	
		if(CWorld::ProcessLineOfSight(pCamera->GetPosition(), pCamera->GetPosition()+pCamera->GetMatrix().GetForward()*600.0f, colPoint, pLookAtEntity))
		{
			int32 polys = 0;
			int32 collpolys = 0;
			
			if(pLookAtEntity->GetRwObject())
			{
				if(RwObjectGetType(pLookAtEntity->GetRwObject()) == rpATOMIC)
					polys = RpGeometryGetNumTriangles(RpAtomicGetGeometry((RpAtomic*)pLookAtEntity->GetRwObject()));
			}
			
			CCollisionData* pColData = CModelInfo::GetColModel(pLookAtEntity->GetModelIndex()).GetCollisionData();
			if(pColData)
				collpolys = pColData->m_nNoOfTriangles;
							
			if(CTimer::m_FrameCounter & 1)
				pLookAtEntity->m_nFlags.bIsVisible = false;
				
			int32 idx = pLookAtEntity->GetModelIndex();
			CBaseModelInfo *pBMI = CModelInfo::GetModelInfo(idx);
			int32 tdIdx = pBMI->GetTexDictionary();
			char *pModelName = (char*)pBMI->GetModelName();
			
			// Print MODEL & TXD with sizes			
			sprintf(gString, "%d: %s (%dK) : TXD %s (%dK)",idx, pModelName,CStreaming::GetModelSize(idx) * 2,CTxdStore::GetTxdName(tdIdx),CStreaming::GetModelSize(RWTXD_INDEX_TO_INDEX(tdIdx)) * 2);					
			VarConsole.AddDebugOutput(gString);			
			
			// Print model details.
			sprintf(gString, "LOD %d POLYS %d COLTRI %d", (int32)pBMI->GetLodDistance(),polys, collpolys);
			VarConsole.AddDebugOutput(gString);

			sprintf(gString, "POSN %.2f,%.2f,%.2f", pLookAtEntity->GetPosition().x, pLookAtEntity->GetPosition().y, pLookAtEntity->GetPosition().z);
			VarConsole.AddDebugOutput(gString);

			// DW - get TEX% ( ratio of textures used with dictionary size )			
			float percentage = pBMI->GetTexPercentage();					
			sprintf(gString, "TEX%% %3.1f", percentage);
			VarConsole.AddDebugOutput(gString);

			// DW - display the collision type
//			char *pColTypeName = aSurfaceNames[colPoint.GetSurfaceTypeB()];
#ifndef FINAL
			char *pColTypeName = g_surfaceInfos.GetSurfaceNameFromId(colPoint.GetSurfaceTypeB());
			sprintf(gString, "COLTYPE %s", pColTypeName);
			VarConsole.AddDebugOutput(gString);	
#else
			VarConsole.AddDebugOutput("COLTYPE - not stored in final build");	
#endif	
			

#ifdef ALLOW_OBJECT_DISABLING
			// DW - allow object 'disabling'
			bool bDisableObject = PS2Keyboard.GetKeyJustDown(PS2_KEY_DEL, KEYBOARD_MODE_STANDARD, "Remove selected object");
			if (bDisableObject)
			{
				// Remove instance
				pLookAtEntity->DisableAndSave(true);				
				pLookAtEntity = NULL;
				return;			
			}		
			
			// DW - allow lod modification.
			float inc = 0;
			static bool gbKeyIncrementWasDownLastFrame = false;
			bool bKeyUp = false;
			if (PS2Keyboard.GetKeyDown(PS2_KEY_RIGHT))
			{			
				inc = 1.0f;
				gbKeyIncrementWasDownLastFrame = true;
			}
			else if (PS2Keyboard.GetKeyDown(PS2_KEY_LEFT))
			{
				inc = -1.0f;
				gbKeyIncrementWasDownLastFrame = true;
			}
			else
			{
				bKeyUp = gbKeyIncrementWasDownLastFrame;
				gbKeyIncrementWasDownLastFrame = false;
			}
			
			if (PS2Keyboard.GetKeyDown(PS2_KEY_CTRL))
				inc *= 10.0f;
						
			//if (inc)
			{				
				if (pBMI)
				{
					// mod the LOD
					float lod = pBMI->GetLodDistance() + inc;
					pBMI->SetLodDistance(lod);
					
					// keep file continually up to date in case of a crash.
					// load the obj modification file in 
				
					if (bKeyUp)
					{
						uint8 *pBuffer = new uint8[LOAD_BUF_SIZE];
						ASSERT(pBuffer);
						memset(pBuffer,QUIT_TOKEN,LOAD_BUF_SIZE-1);					
						pBuffer[LOAD_BUF_SIZE-1] = '\0';
						int32 result = CFileMgr::LoadFile(OBJMODFILE,pBuffer);
						
						bool bQuit = (result<0);

						// start writing back out the file
						int32 fd = CFileMgr::OpenFileForWriting(OBJMODFILE);
						ASSERT(fd>=0);
						
						float loadLod;
						bool bFound = false;					
						char str[255],str2[255];
						uint8 *pBuf = pBuffer;					
						while(!bQuit && (sscanf((const char*)pBuf,"%s %f %s",&str,&loadLod,&str2) == 3) && strcmp(str,LOD_STRING))
						{					
							if (!strcmp(pModelName,str2))
							{
								loadLod = lod; // replace this entry with our modification
								bFound = true;
							}		
													
							sprintf(str,"%4s %10.3f %30s%c%c",LOD_STRING,loadLod,str2,0x0D,0x0A); // new line difficulties unknown why it doesnt work. - DW
							CFileMgr::Write(fd, str, strlen(str)+1);	
							while(!bQuit)
							{
								pBuf++;
								if (!strncmp((const char*)pBuf,(const char*)LOD_STRING,LOD_STRING_LEN))
									break;
								if (*pBuf == QUIT_TOKEN)
									bQuit = true;
							}
						}
						
						if (!bFound) // add new entry to file.
						{						
							sprintf(str,"%4s %10.3f %30s%c%c\n",LOD_STRING,lod,pModelName,0x0D,0x0A); 
							CFileMgr::Write(fd, str, strlen(str)+1);
						}
						
						CFileMgr::CloseFile(fd);	
						
						delete pBuffer;
					}
				}
			}
#endif	// object_disabling				
					
			CEntity* pLod = pLookAtEntity->GetLod();
			while(pLod)
			{
				if(CTimer::m_FrameCounter & 1)
					pLod->m_nFlags.bIsVisible = false;
				polys = 0;
				VarConsole.AddDebugOutput("Has Lod:");
				if(pLod->GetRwObject())
				{
					if(RwObjectGetType(pLod->GetRwObject()) == rpATOMIC)
						polys = RpGeometryGetNumTriangles(RpAtomicGetGeometry((RpAtomic*)pLod->GetRwObject()));
				}
				sprintf(gString, "%d: %s TXD %s", 
						pLod->GetModelIndex(), 
						CModelInfo::GetModelInfo(pLod->GetModelIndex())->GetModelName(),
						CTxdStore::GetTxdName(CModelInfo::GetModelInfo(pLod->GetModelIndex())->GetTexDictionary()),
						(int32)CModelInfo::GetModelInfo(pLod->GetModelIndex())->GetLodDistance());
				VarConsole.AddDebugOutput(gString);
				sprintf(gString, "LOD %d POLYS %d", 
						(int32)CModelInfo::GetModelInfo(pLod->GetModelIndex())->GetLodDistance(),
						polys);
				VarConsole.AddDebugOutput(gString);
				sprintf(gString, "POSN %.2f,%.2f,%.2f", pLod->GetPosition().x, pLod->GetPosition().y, pLod->GetPosition().z);
				VarConsole.AddDebugOutput(gString);

				
				pLod = pLod->GetLod();
			}
			
		}
	}
	else if(pLookAtEntity)
	{
		pLookAtEntity->m_nFlags.bIsVisible = true;
		pLod = pLookAtEntity->GetLod();
		while(pLod)
		{
			pLod->m_nFlags.bIsVisible = true;
			pLod = pLod->GetLod();
		}
		pLookAtEntity = NULL;
	}	

	// Make sure we know at all times the dff name and the artist for that DFF.
	if (pLookAtEntity)
	{
		char *pDffName = CStreamingDebug::ModelFindDffFileName(pLookAtEntity->GetModelIndex());
		gpArtistForLookAtOrStandOn = FindArtistForDff(pDffName);
		
		if (gpArtistForLookAtOrStandOn)
		{
			char str[255];
			sprintf(str,"ARTIST = %s",gpArtistForLookAtOrStandOn);
			VarConsole.AddDebugOutput(str);
		}
	}
		
}
#endif	

///////////////////////////////////////////////////////////////////////////
// NAME       : Process()
// PURPOSE    : Works out the new camera matrix taking everything into account.
//				Updates up to two cameras that might have to be interpolated.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
//
void CCamera::Process()
{
	extern void ResetMadeInvisibleObjects(); // DW - migrated because there are instances where it doesnt work.
	ResetMadeInvisibleObjects();

#ifndef FINAL

	{
		// Make sure we know at all times the dff name and the artist for that DFF.
		CEntity* pEntityStandingOn = FindPlayerPed(0)->m_pEntityStandingOn;	
		if (pEntityStandingOn)
		{
			char *pDffName = CStreamingDebug::ModelFindDffFileName(pEntityStandingOn->GetModelIndex());
//			gpArtistForLookAtOrStandOn = FindArtistForDff(pDffName);
			
/*			if (gpArtistForLookAtOrStandOn)
			{
				char str[255];
				sprintf(str,"MAPSTOODON : ARTIST = %s",gpArtistForLookAtOrStandOn);
				VarConsole.AddDebugOutput(str);
			}*/
		}
	}
	
	static bool DoOnce=true;
	#ifdef DebugMenuOn	
	{
		if (DoOnce)
		{
			DoOnce=false;
	//		VarConsole.Add(text, pvalue, inc, min, max, loop);
			float GentleIncrementDegrees=DEGTORAD(1.5f);
			float CarIncrementDistHor=0.1f;
			float CarIncrementDistVert=0.005f;
			float PedIncrementDistHor=0.05f;
			float PedIncrementDistVert=0.02f;
				
			float SpeedStepStuff=0.01f;
			int heading=0.0f;

			VarConsole.Add("What am I looking at",&gbWhatAmILookingAt, true, VC_CAMERA);

			VarConsole.Add("Camera PedPos Buffer A",&PLAYERPED_LEVEL_SMOOTHING_CONST_INV , 0.02f, 0.0f, 1.0f, false, VC_CAMERA);
			VarConsole.Add("Camera PedVel Buffer B",&PLAYERPED_TREND_SMOOTHING_CONST_INV , 0.02f, 0.0f, 1.0f, false, VC_CAMERA);

			VarConsole.Add("Aim Weapon Jumpcut Angle(deg)", &MAX_ANGLE_BEFORE_AIMWEAPON_JUMPCUT, 1.0f, 0.0f, 200.0f, false, VC_TARGETTING);
			VarConsole.Add("FollowCar Swing Multiplier", &CARCAM_SET[0].fDiffBetaSwing, 0.025f, 0.0f, 0.3f, false, VC_CAMERA);


			// DW - added these unknown thingmyjigs for Les... dunno what they do though
			VarConsole.Add("AimWeapon Base Dist", 		&AIMWEAPON_SETTINGS[0][0], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeapon Flat Dist", 		&AIMWEAPON_SETTINGS[0][1], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeapon Flat Ang Falloff",&AIMWEAPON_SETTINGS[0][2], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeapon Default Alpha",	&AIMWEAPON_SETTINGS[0][3], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeapon Z Offset", 		&AIMWEAPON_SETTINGS[0][4], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeapon Up Limit", 		&AIMWEAPON_SETTINGS[0][5], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeapon Down Limit",  	&AIMWEAPON_SETTINGS[0][6], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);

			VarConsole.Add("AimWeaponMELEE Dist", 		&AIMWEAPON_SETTINGS[3][0], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeaponMELEE ang d", 		&AIMWEAPON_SETTINGS[3][1], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeaponMELEE ang f", 		&AIMWEAPON_SETTINGS[3][2], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeaponMELEE pitch", 		&AIMWEAPON_SETTINGS[3][3], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeaponMELEE Z", 			&AIMWEAPON_SETTINGS[3][4], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeaponMELEE maxup ", 	&AIMWEAPON_SETTINGS[3][5], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("AimWeaponMELEE maxdown",  	&AIMWEAPON_SETTINGS[3][6], 0.05f, -10000.0f, 200000.0f, false, VC_TARGETTING);


	// Had to remove these cos of the way varconsole works
			VarConsole.Add("Aim Weapon ON MELEE Dist", 						&AIMWEAPON_SETTINGS[3][0], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("Aim Weapon ON MELEE angle d", 					&AIMWEAPON_SETTINGS[3][1], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("Aim Weapon ON MELEE angle f", 					&AIMWEAPON_SETTINGS[3][2], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("Aim Weapon ON MELEE alpha (pitch up down)", 	&AIMWEAPON_SETTINGS[3][3], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("Aim Weapon ON MELEE Z? ", 						&AIMWEAPON_SETTINGS[3][4], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("Aim Weapon ON MELEE max up angle(radians)? ", 	&AIMWEAPON_SETTINGS[3][5], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);
			VarConsole.Add("Aim Weapon ON MELEE max down angle(radians)? ", &AIMWEAPON_SETTINGS[3][6], 0.005f, -10000.0f, 200000.0f, false, VC_TARGETTING);

			// End DW - added these unknown thingmyjigs for Les... dunno what they do though


			VarConsole.Add("Aim Weapon(DW added) ZOffset", &AIMWEAPON_SETTINGS[0][4], 0.05f, -1.0f, 2.0f, false, VC_TARGETTING);

float AIMWEAPON_SETTINGS[AIMWEAPON_MELEE+1][AIMWEAPON_ROTMAX_DOWN+1] =
{
	//  dist	angle_d	angle_f	alpha		z		max_up				max_down
	{	0.4f,	0.7f,	1.0f,	-0.12f,		0.15f,	DEGTORAD(45.0f),	DEGTORAD(89.5f)},
	{	3.5f,	0.7f,	1.0f,	-0.16f,		0.2f,	DEGTORAD(35.0f),	DEGTORAD(70.0f)},
	{	6.0f,	0.7f,	1.0f,	-0.16f,		0.4f,	DEGTORAD(35.0f),	DEGTORAD(70.0f)},
	{	1.8f,	0.7f,	1.0f,	-0.12f,		0.15f,	DEGTORAD(45.0f),	DEGTORAD(45.5f)}
};




			
			VarConsole.Add("Follow_Car using Follow_Ped code", &TEST_FOLLOW_CAR_USING_FOLLOW_PED_CODE, true, VC_CAMERA);

			VarConsole.Add("PED HORIZ ZM1 DIST",&(Cams[0].m_fTargetZoomGroundOne) , PedIncrementDistHor, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED HORIZ ZM2 DIST",&(Cams[0].m_fTargetZoomGroundTwo) , PedIncrementDistHor, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED HORIZ ZM3 DIST",&(Cams[0].m_fTargetZoomGroundThree) , PedIncrementDistHor, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED HEIGHT OFFSET ZM1",&(Cams[0].m_fTargetZoomOneZExtra) , PedIncrementDistVert, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED HEIGHT OFFSET ZM2",&(Cams[0].m_fTargetZoomTwoZExtra) , PedIncrementDistVert, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED HEIGHT OFFSET INTERIOR ZM2",&(Cams[0].m_fTargetZoomTwoInteriorZExtra) , PedIncrementDistVert, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED HEIGHT OFFSET ZM3",&(Cams[0].m_fTargetZoomThreeZExtra) , PedIncrementDistVert, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED OVERSHDER TARGET HORZ DIST",&INIT_SYPHON_GROUND_DIST , PedIncrementDistHor, -1000.0f, 1000.0f, false, VC_PEDS);	
			VarConsole.Add("PED OVERSHDER HEIGHT OFFSET",&INIT_SYPHON_ALPHA_OFFSET , GentleIncrementDegrees, -1000.0f, 1000.0f, false, VC_PEDS);
			VarConsole.Add("PED OVERSHDER DEGREE OFFSET",&INIT_SYPHON_DEGREE_OFFSET , GentleIncrementDegrees, -1000.0, 1000.0f, false, VC_PEDS);		
			VarConsole.Add("PED OVERSHDER CAMLOOK OFFSET",&FrontOffsetSyphon,  GentleIncrementDegrees, -1000.0, 1000.0f, false, VC_PEDS);		

			//{Car, Bike, Heli, Plane, Boat}

			//CAR
			VarConsole.Add("CAR TILT TOP SPEED",&TiltTopSpeed[0] , SpeedStepStuff, -1000.0f, 1000.0f, false, VC_VEHICLES);
			VarConsole.Add("CAR TILT STARTING SPEED",&TiltSpeedStep[0] , SpeedStepStuff, -1000.0f, 1000.0f, false, VC_VEHICLES);
			VarConsole.Add("CAR TILT OVERSHOOT",&TiltOverShoot[0] , PedIncrementDistHor, -1000.0f, 1000.0f, false, VC_VEHICLES);		
			VarConsole.Add("CAR HORIZ ZM1 DIST",&ZOOM_ONE_DISTANCE[0] , CarIncrementDistHor, -1000.0f, 1000.0f, false, VC_VEHICLES);		
			VarConsole.Add("CAR HORIZ ZM2 DIST",&ZOOM_TWO_DISTANCE[0] , CarIncrementDistHor, -1000.0f, 1000.0f, false, VC_VEHICLES);
			VarConsole.Add("CAR HORIZ ZM3 DIST",&ZOOM_THREE_DISTANCE[0] , CarIncrementDistHor, -1000.0f, 1000.0f, false, VC_VEHICLES);
			VarConsole.Add("CAR HEIGHT OFFSET ZM 1",&ZmOneAlphaOffset[0] , CarIncrementDistVert, -1000.0f, 1000.0f, false, VC_VEHICLES);
			VarConsole.Add("CAR HEIGHT OFFSET ZM 2",&ZmTwoAlphaOffset[0] , CarIncrementDistVert, -1000.0f, 1000.0f, false, VC_VEHICLES);
			VarConsole.Add("CAR HEIGHT OFFSET ZM 3",&ZmThreeAlphaOffset[0] , CarIncrementDistVert, -1000.0f, 1000.0f, false, VC_VEHICLES);
/*			//BIKE		
			VarConsole.Add("BIKE TILT TOP SPEED",&TiltTopSpeed[1] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("BIKE TILT STARTING SPEED",&TiltSpeedStep[1] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("BIKE TILT OVERSHOOT",&TiltOverShoot[1] , PedIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("BIKE HORIZ ZM1 DIST",&ZOOM_ONE_DISTANCE[1] , CarIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("BIKE HORIZ ZM2 DIST",&ZOOM_TWO_DISTANCE[1] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("BIKE HORIZ ZM3 DIST",&ZOOM_THREE_DISTANCE[1] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("BIKE HEIGHT OFFSET ZM 1",&ZmOneAlphaOffset[1] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("BIKE HEIGHT OFFSET ZM 2",&ZmTwoAlphaOffset[1] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("BIKE HEIGHT OFFSET ZM 3",&ZmThreeAlphaOffset[1] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			//HELI			
			VarConsole.Add("HELI TILT TOP SPEED",&TiltTopSpeed[2] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("HELI TILT STARTING SPEED",&TiltSpeedStep[2] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("HELI TILT OVERSHOOT",&TiltOverShoot[2] , PedIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("HELI HORIZ ZM1 DIST",&ZOOM_ONE_DISTANCE[2] , CarIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("HELI HORIZ ZM2 DIST",&ZOOM_TWO_DISTANCE[2] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("HELI HORIZ ZM3 DIST",&ZOOM_THREE_DISTANCE[2] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("HELI HEIGHT OFFSET ZM 1",&ZmOneAlphaOffset[2] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("HELI HEIGHT OFFSET ZM 2",&ZmTwoAlphaOffset[2] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("HELI HEIGHT OFFSET ZM 3",&ZmThreeAlphaOffset[2] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			//PLANE
			VarConsole.Add("PLANE TILT TOP SPEED",&TiltTopSpeed[3] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("PLANE TILT STARTING SPEED",&TiltSpeedStep[3] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("PLANE TILT OVERSHOOT",&TiltOverShoot[3] , PedIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("PLANE HORIZ ZM1 DIST",&ZOOM_ONE_DISTANCE[3] , CarIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("PLANE HORIZ ZM2 DIST",&ZOOM_TWO_DISTANCE[3] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("PLANE HORIZ ZM3 DIST",&ZOOM_THREE_DISTANCE[3] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("PLANE HEIGHT OFFSET ZM 1",&ZmOneAlphaOffset[3] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("PLANE HEIGHT OFFSET ZM 2",&ZmTwoAlphaOffset[3] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("PLANE HEIGHT OFFSET ZM 3",&ZmThreeAlphaOffset[3] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			//BOAT
			VarConsole.Add("BOAT TILT TOP SPEED",&TiltTopSpeed[4] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("BOAT TILT STARTING SPEED",&TiltSpeedStep[4] , SpeedStepStuff, -1000.0f, 1000.0f, false);
			VarConsole.Add("BOAT TILT OVERSHOOT",&TiltOverShoot[4] , PedIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("BOAT HORIZ ZM1 DIST",&ZOOM_ONE_DISTANCE[4] , CarIncrementDistHor, -1000.0f, 1000.0f, false);		
			VarConsole.Add("BOAT HORIZ ZM2 DIST",&ZOOM_TWO_DISTANCE[4] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("BOAT HORIZ ZM3 DIST",&ZOOM_THREE_DISTANCE[4] , CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("BOAT HEIGHT OFFSET ZM 1",&ZmOneAlphaOffset[4] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("BOAT HEIGHT OFFSET ZM 2",&ZmTwoAlphaOffset[4] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("BOAT HEIGHT OFFSET ZM 3",&ZmThreeAlphaOffset[4] , CarIncrementDistVert, -1000.0f, 1000.0f, false);
			//REMOTE CONTROL
			VarConsole.Add("RCHELI HORIZONTALOFFSET",&INIT_RC_HELI_HORI_EXTRA, CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("RCHELI ALPHAOFFSET",&INIT_RC_HELI_ALPHA_EXTRA, CarIncrementDistVert, -1000.0f, 1000.0f, false);
			VarConsole.Add("RCPLANE HORIZONTALOFFSET",&INIT_RC_PLANE_HORI_EXTRA, CarIncrementDistHor, -1000.0f, 1000.0f, false);
			VarConsole.Add("RCPLANE ALPHAOFFSET",&INIT_RC_PLANE_ALPHA_EXTRA, CarIncrementDistVert, -1000.0f, 1000.0f, false);
*/			
		}
	}
	#endif
#endif // MASTER


	static bool InterpolatorNotInitialised=TRUE;
	CVector		Right;
	CVector		FinalSource, FinalFront, FinalUp, DebugFinalSource, DebugFinalFront, DebugFinalUp, FudgedTargetCoors;
	CVector 	TestTempRight;
	CVector 	TempTargetWhenInterPol;
	UInt16		Random;
	UInt32		TimeInInterpolation;
	float		CurrentShakeForce=0.0f, InterValue=0.0f;
	float		FinalFOV=0.0f, DebugFinalFOV=0.0f;
	float 		BetaBefore=0.0f;
	float 		BetaAfter=0.0f;
	float		BetaDiff=0.0f;
 	static 		float 	MinDistCamAwayFromPlayWhenInter=1.3f;//1.6f; //this is so the player cannot run into the camera
																  ///camera has to stay this distance away
	float		Length=0.0f;
	bool  		WasDoingACarLookThingy=false;
	
	static bool WasPreviouslyInterSyhonFollowPed=false;
	//char Str[250];
	// Select the mode and take care of transitions and things
	
	m_bJust_Switched=FALSE;
  
  	m_RealPreviousCameraPosition = GetPosition();
	
	if ((m_bLookingAtPlayer==true)||(m_bTargetJustBeenOnTrain==true)||(WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL))//this is because the train cam takes control of all the different modes
	//like it was a script camera but the target entity is the player
	{
		UpdateTargetEntity();//for normal modes i.e. not scripting 
	}
	//////
	if (pTargetEntity==NULL)
	{
		pTargetEntity=FindPlayerPed();
		REGREF(pTargetEntity, &pTargetEntity);
	}
	if (Cams[ActiveCam].CamTargetEntity==NULL)
	{
		Cams[ActiveCam].CamTargetEntity=pTargetEntity;	
		REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
	}
	if (Cams[((ActiveCam + 1) % 2)].CamTargetEntity==NULL)
	{
		Cams[((ActiveCam + 1) % 2)].CamTargetEntity=pTargetEntity;	
		REGREF(Cams[((ActiveCam + 1) % 2)].CamTargetEntity, &Cams[((ActiveCam + 1) % 2)].CamTargetEntity);
	}
	
	CamControl();
	
	// hopefully good place for this???...I'm lost! - DW
	TheCamera.ProcessScriptedCommands();

#ifndef MASTER
	TheCamera.ProcessTestBed();
#endif	
		
	if (m_bFading){ProcessFade();}
	if (m_bMusicFading){ProcessMusicFade();}
	if (m_WideScreenOn){ProcessWideScreenOn();}

#ifndef MASTER	
	if ((CPad::GetPad(1)->ButtonCircleJustDown() && !CGameLogic::Disable2ndControllerForDebug()) || PS2Keyboard.GetKeyJustDown(PS2_KEY_ENTER))//for debug stuff
	{
					
		WorldViewerBeingUsed=!WorldViewerBeingUsed;
		if (WorldViewerBeingUsed)
		{
			InitialiseCameraForDebugMode();
			CGridRef::UsePlayerCoords = FALSE;
			
			#ifdef GTA_PC
				CPad::SetMapPadOneToPadTwo(true);
			#endif
	    }			
		else
		{
			CGridRef::UsePlayerCoords = TRUE;

			#ifdef GTA_PC
				CPad::SetMapPadOneToPadTwo(false);
			#endif
		}
		
		
#ifndef FINALBUILD		
		// reset static frustum
		bStaticFrustum = false;
#endif		
	}
#endif
	
	RwCameraSetNearClipPlane(Scene.camera, (RwReal) NORMAL_NEAR_CLIP);//for interpolation to work as 1rst person mode 
	//uses a clipping plane that is small so we need to reset it
	if ((Cams[ActiveCam].Front.x==0.0f)&&(Cams[ActiveCam].Front.y==0.0f))
	{
		BetaBefore=0.0f;
	}
	else
	{
		BetaBefore=CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y);
	}
	// Process the two camera modes that could be active
	Cams[ActiveCam].Process();				// The one that is active
//	Cams[ActiveCam].ProcessSpecialHeightRoutines(); // dwremoved

	if ((Cams[ActiveCam].Front.x==0.0f)&&(Cams[ActiveCam].Front.y==0.0f))
	{
		BetaAfter=0.0f;
	}
	else
	{
		BetaAfter=CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y);;
	}


		
	/*
	// Test whether a possible transition has finished
	// also if the user is being a wee prick and trying to break the syphon filter cam
	
	bool UserTryingToBreakCameraBeNasty=false;
	CVector PlayerFarAway;

	if ((m_bLookingAtPlayer) && (m_uiTransitionState != TRANS_NONE))	
	{
		PlayerFarAway.x=FindPlayerPed()->GetPosition().x-GetMatrix().tx;
		PlayerFarAway.y=FindPlayerPed()->GetPosition().y-GetMatrix().ty;
		PlayerFarAway.z=FindPlayerPed()->GetPosition().z-GetMatrix().tz;

		if (pTargetEntity!=NULL)
		{ 
			if (pTargetEntity->GetIsTypePed())
			{
				if (PlayerFarAway.Magnitude()>17.5f)
				{
						UserTryingToBreakCameraBeNasty=true; //fuck it lets abort the transition
						//and jump there
				}
			}
		}
	}	
 	*/
 	
 	if (m_uiTransitionState != TRANS_NONE)
	{
	//	if ((CTimer::GetTimeInMilliseconds() > (m_uiTimeTransitionStart + m_uiTransitionDuration))||(UserTryingToBreakCameraBeNasty))
		if ((CTimer::GetTimeInMilliseconds() > (m_uiTimeTransitionStart + m_uiTransitionDuration)))
		{
			m_uiTransitionState = TRANS_NONE;
		    m_vecDoingSpecialInterPolation=false;
			m_bWaitForInterpolToFinish=false;
		}
	}




	
	if (m_bUseNearClipScript)
	{
		RwCameraSetNearClipPlane(Scene.camera, m_fNearClipScript);//for interpolation to work as 1rst person mode 
	}

	//Sound thingy for raymond
	BetaDiff=BetaAfter-BetaBefore;
	
	while (BetaDiff>=(PI)) {BetaDiff-=2.0f * PI;}
	while (BetaDiff<(-PI)) {BetaDiff+=2.0f * PI;}
	
//	char Str[200];
//	sprintf(Str, "BetaSpeed %f ", CMaths::Abs(BetaDiff));
//	CDebug::PrintToScreenCoors(Str, 3, 11);
	
	if (CMaths::Abs(BetaDiff)>BETA_DIFF_FOR_CUT_OFF_DOPPLER)
	{
		m_bJust_Switched=true;
	}

	/*
	if (!gbModelViewer)
	{
		Cams[ActiveCam].PrintMode();	// Print out the mode that we use at the moment
	}
	*/
#ifndef MASTER	
	if (WorldViewerBeingUsed)
	{
		Cams[2].Process();
	}
#endif
	// Interpolate the two camera modes if there is a transition going on.
 	//don't do it if we are looking at a car and have just gon into look behind etc
 	
 	if (((Cams[ActiveCam].DirectionWasLooking!=LOOKING_FORWARD))&&(pTargetEntity->GetIsTypeVehicle()))
	{
		WasDoingACarLookThingy=true;
	}
			
	// DW - insert my stuff now
	ProcessShake();		
	
	if ((!m_uiTransitionState)||(WasDoingACarLookThingy))
	{
#ifndef MASTER
		if (WorldViewerBeingUsed)
		{
			FinalSource = Cams[2].Source;
			FinalFront = Cams[2].Front;
			FinalUp = Cams[2].Up;
			FinalFOV = Cams[2].FOV;
		}
		else
#endif		
		{	
			FinalSource = Cams[ActiveCam].Source;
			FinalUp = Cams[ActiveCam].Up;
			if (m_bMoveCamToAvoidGeom)
			{
		
				FinalSource+=m_vecClearGeometryVec;
				FinalFront = Cams[ActiveCam].m_cvecTargetCoorsForFudgeInter-FinalSource;
				FinalFront.Normalise();
				//now work out the right Vector
				CVector TempRight = CrossProduct( FinalFront, FinalUp);
				TempRight.Normalise();
				FinalUp=CrossProduct(TempRight, FinalFront);
				FinalUp.Normalise();
			}
			else
			{
				FinalFront = Cams[ActiveCam].Front;
				FinalUp = Cams[ActiveCam].Up;
			}
			FinalFOV = Cams[ActiveCam].FOV;
		}			
		WasPreviouslyInterSyhonFollowPed=false;
	}
	else //We're doing a transition
	{	
	
		TimeInInterpolation = CTimer::GetTimeInMilliseconds() - m_uiTimeTransitionStart;
		TimeInInterpolation = MIN(TimeInInterpolation, m_uiTransitionDuration);
		InterValue = ((float)TimeInInterpolation) / m_uiTransitionDuration;
		ASSERT(InterValue>=0.0f && InterValue<=1.000000f);
		float InterFraction=0.0f;
		float TargetInterFraction=0.0f;
		float TempBeta=0.0f;
		float TemporaryDistanceGround=0.0f;	
		CVector TempDistanceVec;
		
		//work out the target coors first
		//only if the player is the main focus of the camera.
		//we want the target coors to be worked out quickly so the player cant run off the screen

		ASSERTMSG(m_uiTransitionDurationTargetCoors!=0, "Can't be zero, it just can't");
		
		float TempInterValue = ((float)TimeInInterpolation) / m_uiTransitionDurationTargetCoors;
		
		if (TempInterValue<0.0f)
		{
			TempInterValue=0.0f;
		}
		else if (TempInterValue>1.0f)
		{
			TempInterValue=1.0f;
		}
		
		if (TempInterValue<=m_fFractionInterToStopMovingTarget)
		{
			if (m_fFractionInterToStopMovingTarget==0.0f) //avoid divisiion by zero (was m_fFractionInterToStopMoving (Obbe))
			{
				TargetInterFraction=0.0f;
			}
			else
			{
				TargetInterFraction=((m_fFractionInterToStopMovingTarget-TempInterValue)/m_fFractionInterToStopMovingTarget);
			}
			// Rather than a linear value for the interpolation value we
			// use a cosine so that it accellerates and decellerates smoothly.
			TargetInterFraction = 0.5f - (0.5f * CMaths::Cos(TargetInterFraction * PI));
			m_vecTargetWhenInterPol=m_cvecStartingTargetForInterPol + (m_cvecTargetSpeedAtStartInter * (TargetInterFraction));
			TempTargetWhenInterPol=m_vecTargetWhenInterPol;
		}
		else if (TempInterValue>m_fFractionInterToStopMovingTarget)	
		{
			ASSERT(TempInterValue<=1.0f);
			//camera now stopped, want to catch up to where it should be 
			if (m_fFractionInterToStopCatchUpTarget==0.0f) //avoid divisiion by zero (was m_fFractionInterToStopCatchUp (Obbe))
			{
				TargetInterFraction=1.0f;
			}
			else
			{
				TargetInterFraction=((TempInterValue-m_fFractionInterToStopMovingTarget)/m_fFractionInterToStopCatchUpTarget);
			}
			
			// Rather than a linear value for the interpolation value we
			// use a cosine so that it accellerates and decellerates smoothly.
			TargetInterFraction = 0.5f - (0.5f * CMaths::Cos(TargetInterFraction * PI));
			if (m_fFractionInterToStopMovingTarget == 0.0f) m_vecTargetWhenInterPol=m_cvecStartingTargetForInterPol;	// This makes sure that if m_fFractionInterToStopMovingTarget == 0.0f we still have a valid value for m_vecTargetWhenInterPol
			TempTargetWhenInterPol= m_vecTargetWhenInterPol + ((Cams[ActiveCam].m_cvecTargetCoorsForFudgeInter - m_vecTargetWhenInterPol) * TargetInterFraction);
		}
		////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////
		//now do the stuff for source, fov and up
		if (InterValue<=m_fFractionInterToStopMoving)  
		{
			//if point is moving want to slow it to a stop
			if (m_fFractionInterToStopMoving==0.0f) //avoid divisiion by zero
			{
				InterFraction=0.0f;
			}
			else
			{
				InterFraction=((m_fFractionInterToStopMoving-InterValue)/m_fFractionInterToStopMoving);
			}
			// Rather than a linear value for the interpolation value we
			// use a cosine so that it accellerates and decellerates smoothly.
			InterFraction = 0.5f - (0.5f * CMaths::Cos(InterFraction * PI));

			m_vecSourceWhenInterPol=m_cvecStartingSourceForInterPol + (m_cvecSourceSpeedAtStartInter *  (InterFraction));
			//work out if we are too close to the camera, if so we want to move the camera back
			if (m_bLookingAtPlayer)
			{
				CVector TempForGroundDist=m_vecSourceWhenInterPol-TempTargetWhenInterPol;
				if (TempForGroundDist.Magnitude2D()<MinDistCamAwayFromPlayWhenInter)
				{
					float VecAngle=CGeneral::GetATanOfXY(TempForGroundDist.x, TempForGroundDist.y);
					m_vecSourceWhenInterPol.x=TempTargetWhenInterPol.x + MinDistCamAwayFromPlayWhenInter*CMaths::Cos(VecAngle);
					m_vecSourceWhenInterPol.y=TempTargetWhenInterPol.y + MinDistCamAwayFromPlayWhenInter*CMaths::Sin(VecAngle);
				}
			}	
	
	//		m_vecTargetWhenInterPol=m_cvecStartingTargetForInterPol + (m_cvecTargetSpeedAtStartInter * (InterFraction));
			m_vecUpWhenInterPol=m_cvecStartingUpForInterPol + (m_cvecUpSpeedAtStartInter * (InterFraction));
			m_fFOVWhenInterPol=m_fStartingFOVForInterPol +  (m_fFOVSpeedAtStartInter * (InterFraction));
			FinalSource = m_vecSourceWhenInterPol;
			
			
			FinalFront = TempTargetWhenInterPol-FinalSource;
			StoreValuesDuringInterPol(FinalSource, m_vecTargetWhenInterPol, m_vecUpWhenInterPol, m_fFOVWhenInterPol);
			FinalFront.Normalise();
			
			
			if (m_bLookingAtPlayer==true) 
			{
				FinalUp=CVector(0.0f,0.0f,1.0f);//hack to get rid of roll as side effect of interpolation
			}
			else
			{
				FinalUp=m_vecUpWhenInterPol;
			}
			
			FinalUp.Normalise();
			if ((Cams[ActiveCam].Mode!=CCam::MODE_TOPDOWN)&&(Cams[ActiveCam].Mode!=CCam::MODE_TOP_DOWN_PED))
			{
				//effect of inteepoltation
			
				FinalFront.Normalise();
				FinalUp.Normalise();
			
				TestTempRight = CrossProduct(FinalFront, FinalUp);
				TestTempRight.Normalise();
				FinalUp=CrossProduct(TestTempRight, FinalFront);
				FinalUp.Normalise();
			}
			else 
			{
				
				FinalFront.Normalise();
				TestTempRight=CVector(-1.0f, 0.0f, 0.0f);
				FinalUp = CrossProduct(FinalFront, TestTempRight);
				FinalUp.Normalise();

			}
			
			FinalFOV=m_fFOVWhenInterPol;
		}
		else if ((InterValue>m_fFractionInterToStopMoving)&&(InterValue<=1.0f))	
		{
			//camera now stopped, want to catch up to where it should be 
			
			if (m_fFractionInterToStopCatchUp==0.0f) //avoid divisiion by zero
			{
				InterFraction=1.0f;
			}
			else
			{
				InterFraction=((InterValue-m_fFractionInterToStopMoving)/m_fFractionInterToStopCatchUp);
			}
			
			// Rather than a linear value for the interpolation value we
			// use a cosine so that it accellerates and decellerates smoothly.
			InterFraction = 0.5f - (0.5f * CMaths::Cos(InterFraction * PI));

			FinalSource = m_vecSourceWhenInterPol + ((Cams[ActiveCam].Source - m_vecSourceWhenInterPol) * InterFraction);
			
			if (m_bLookingAtPlayer)
			{
				CVector TempForGroundDist=FinalSource-TempTargetWhenInterPol;
				if (TempForGroundDist.Magnitude2D()<MinDistCamAwayFromPlayWhenInter)
				{
					float VecAngle=CGeneral::GetATanOfXY(TempForGroundDist.x , TempForGroundDist.y);
					FinalSource.x=TempTargetWhenInterPol.x + MinDistCamAwayFromPlayWhenInter*CMaths::Cos(VecAngle);
					FinalSource.y=TempTargetWhenInterPol.y + MinDistCamAwayFromPlayWhenInter*CMaths::Sin(VecAngle);
				}
			}			
			
			
			FinalFOV=m_fFOVWhenInterPol + ((Cams[ActiveCam].FOV - m_fFOVWhenInterPol) * InterFraction);
			FinalUp = m_vecUpWhenInterPol + ((Cams[ActiveCam].Up-m_vecUpWhenInterPol) * InterFraction);
			BetaDiff=Cams[ActiveCam].m_fTrueBeta - m_fBetaWhenInterPol;
			MakeAngleLessThan180(BetaDiff);
			TempBeta = m_fBetaWhenInterPol + (BetaDiff * InterFraction);
			
			
			FinalFront = TempTargetWhenInterPol - FinalSource;
			StoreValuesDuringInterPol(FinalSource, TempTargetWhenInterPol, FinalUp, FinalFOV);
			FinalFront.Normalise();
			if (m_bLookingAtPlayer==true)
			{
				FinalUp=CVector(0.0f,0.0f,1.0f);//hack to get rid of roll as side of interpolation
			}
			

			if ((Cams[ActiveCam].Mode!=CCam::MODE_TOPDOWN)&&(Cams[ActiveCam].Mode!=CCam::MODE_TOP_DOWN_PED))
			{
				FinalFront.Normalise();
				FinalUp.Normalise();
				TestTempRight = CrossProduct(FinalFront, FinalUp);
				TestTempRight.Normalise();
				FinalUp=CrossProduct(TestTempRight, FinalFront);
				FinalUp.Normalise();
			}
			else 
			{
			
				FinalFront.Normalise();
				TestTempRight=CVector(-1.0f, 0.0f, 0.0f);
				FinalUp = CrossProduct(FinalFront, TestTempRight);
				FinalUp.Normalise();
			}
			
			FinalFOV=m_fFOVWhenInterPol;
		}

		CVector VecForBetaAlpha=FinalSource - TempTargetWhenInterPol; 
		float DistOnGround=CMaths::Sqrt(VecForBetaAlpha.x * VecForBetaAlpha.x + VecForBetaAlpha.y * VecForBetaAlpha.y);
		float TempTrueAlpha=CGeneral::GetATanOfXY(DistOnGround,  VecForBetaAlpha.z);	
		float TempTrueBeta=CGeneral::GetATanOfXY(VecForBetaAlpha.x, VecForBetaAlpha.y);				
	
		Cams[ActiveCam].KeepTrackOfTheSpeed(FinalSource, TempTargetWhenInterPol, FinalUp, TempTrueAlpha, TempTrueBeta, FinalFOV);

	}
	
	//lets check that we are not in obscured position
	if (((m_uiTransitionState!=TRANS_NONE)&&(m_bLookingAtVector==false))&&(m_bLookingAtPlayer==true)&&(!(CCullZones::CamStairsForPlayer()))&&(!m_bPlayerIsInGarage))
	{
		CColPoint CollisionPoint;
		CEntity* pHitEntity=NULL;
		if (CWorld::ProcessLineOfSight(pTargetEntity->GetPosition() , FinalSource, CollisionPoint, pHitEntity,  true, 0, 0, true, false, true, true))
		{
			FinalSource=CollisionPoint.GetPosition();
			RwCameraSetNearClipPlane(Scene.camera, (RwReal)NEAR_CLIP_PED_VIEW_OBSCURED ); //for occasions when the spaz user is right up against a wall

		}
	}
	
	if (CMBlur::Drunkness > 0.0f)
	{
		// MN - Removed Maths Tables
/*		int32 index = DEGTORAD(DrunkRotation) * (PARTICLE_SINCOSTAB_SIZE) / (2.0f * PI);
		index &= (PARTICLE_SINCOSTAB_SIZE-1);
*/

		float drunk;

//		Cams[ActiveCam].FOV *= (1+CMBlur::Drunkness);
		drunk = CMBlur::Drunkness * -0.020f;
/*
		// MN - Removed Maths Tables
		FinalSource.x += CParticle::m_CosTable[index] * drunk;
		FinalSource.z += CParticle::m_SinTable[index] * drunk;
*/
		// MN - Replacement Math Table Code
		float angle = DEGTORAD(DrunkRotation);
		FinalSource.x += CMaths::Cos(angle) * drunk;
		FinalSource.z += CMaths::Sin(angle) * drunk;
		

		FinalUp.Normalise();
//#ifdef GTA_PS2
		drunk = CMBlur::Drunkness * 0.05f;
//#else
//		drunk = CMBlur::Drunkness * 0.015f;
//#endif

/*
		// MN - Removed Maths Tables
		FinalUp.x += CParticle::m_CosTable[index] * drunk;
		FinalUp.y += CParticle::m_SinTable[index] * drunk;
*/

		// MN - Replacement Math Table Code
		FinalUp.x += CMaths::Cos(angle) * drunk;
		FinalUp.y += CMaths::Sin(angle) * drunk;		

		FinalUp.Normalise();
		
		FinalFront.Normalise();
		drunk = CMBlur::Drunkness * -0.1f;
/*	
		// MN - Removed Maths Tables
		FinalFront.x += CParticle::m_CosTable[index] * drunk;
		FinalFront.y += CParticle::m_SinTable[index] * drunk;
*/

		// MN - Replacement Math Table Code
		FinalFront.x += CMaths::Cos(angle) * drunk;
		FinalFront.y += CMaths::Sin(angle) * drunk;	

		FinalFront.Normalise();

		// We have to reorhogonilise the camera matrix to avoid glitches
		TestTempRight = CrossProduct(FinalFront, FinalUp);
		TestTempRight.Normalise();
		FinalUp=CrossProduct(TestTempRight, FinalFront);
		FinalUp.Normalise();
		
		DrunkRotation += 5.0f;

	}
	
	Right = CrossProduct( FinalUp, FinalFront );

	GetMatrix().xx = Right.x; GetMatrix().xy = FinalFront.x; GetMatrix().xz = FinalUp.x;
	GetMatrix().yx = Right.y; GetMatrix().yy = FinalFront.y; GetMatrix().yz = FinalUp.y;
	GetMatrix().zx = Right.z; GetMatrix().zy = FinalFront.z; GetMatrix().zz = FinalUp.z;
	GetMatrix().tx = FinalSource.x;
	GetMatrix().ty = FinalSource.y;
	GetMatrix().tz = FinalSource.z;

	// Do camera shaking that might go on.
	//CurrentShakeForce = m_fCamShakeForce - (CTimer::GetTimeInMilliseconds() - m_uiCamShakeStart) * 0.001f;
	CurrentShakeForce = m_fCamShakeForce - (CTimer::GetTimeInMilliseconds() - m_uiCamShakeStart) * 0.00028f;
	CurrentShakeForce = MIN ( CurrentShakeForce, 2.0f );
	CurrentShakeForce = MAX ( CurrentShakeForce, 0.0f );
	const float blur_delta = CurrentShakeForce;
	Random = CGeneral::GetRandomNumber();
	CurrentShakeForce *= 0.1f;

	GetMatrix().tx += ((Random & 0x000f) - 7) * CurrentShakeForce;
	GetMatrix().ty += (((Random & 0x00f0) >> 4) - 7) * CurrentShakeForce;
	GetMatrix().tz += (((Random & 0x0f00) >> 8) - 7) * CurrentShakeForce;

	// produce motionblurred camera shaking:
	//
	if((CurrentShakeForce>0.0f) && (GetBlurType()!=MB_BLUR_SNIPER ))
	{
		int32 alpha = 25+int32(blur_delta*255.0f);
		if(alpha>150)
			alpha=150;
		//printf(" *** alpha=%d\n", alpha);
		this->SetMotionBlurAlpha(alpha);//MAX(alpha, GetMotionBlur()));
	}
	else
	{
//		this->SetMotionBlur(0,0,0,				0,		MB_BLUR_NONE);
	}
		

	// Big fudge: If the player is in a car in 1st person and the car is
	// upside down we throw in some motion blur to disguise the fact that
	// the bitmap in the background is fucked up.
    static    bool bBlurSet = false;

    if (Cams[ActiveCam].Mode == CCam::MODE_1STPERSON &&
    FindPlayerVehicle() && FindPlayerVehicle()->GetMatrix().zz < 0.2f)
    {
	    this->SetMotionBlur(255, 255, 255, 240, MB_BLUR_SNIPER);
	    bBlurSet = true;
    }
    else if (bBlurSet)
    {
//	    this->SetMotionBlur(CTimeCycle::m_fCurrentBlurRed,CTimeCycle::m_fCurrentBlurGreen,CTimeCycle::m_fCurrentBlurBlue, this->GetMotionBlur(), MB_BLUR_LIGHT_SCENE);
	    bBlurSet = false;
    }
/*
	if (CMBlur::Drunkness > 0.0f)
	{
		int32 index = DEGTORAD(DrunkRotation) * (PARTICLE_SINCOSTAB_SIZE) / (2.0f * PI);
		index &= (PARTICLE_SINCOSTAB_SIZE-1);
		DrunkRotation += 5.0f;
		float drunk;

		Cams[ActiveCam].FOV *= (1+CMBlur::Drunkness);
		drunk = CMBlur::Drunkness * -0.020f;
		GetMatrix().tx += CParticle::m_CosTable[index] * drunk;
//		GetMatrix().ty += CParticle::m_SinTable[index] * drunk;
		GetMatrix().tz += CParticle::m_SinTable[index] * drunk;

		drunk = CMBlur::Drunkness * 0.05f;
		GetMatrix().xz += CParticle::m_CosTable[index] * drunk;
		GetMatrix().yz += CParticle::m_SinTable[index] * drunk;
//		GetMatrix().zz += CParticle::m_SinTable[index] * drunk;

		drunk = CMBlur::Drunkness * -0.075f;
		GetMatrix().xx += CParticle::m_CosTable[index] * drunk;
		GetMatrix().yx += CParticle::m_SinTable[index] * drunk;
//		GetMatrix().zx += CParticle::m_SinTable[index] * drunk;
	
		drunk = CMBlur::Drunkness;
		GetMatrix().xy += CParticle::m_CosTable[index] * drunk;
//		GetMatrix().yy += CParticle::m_SinTable[index] * drunk;
		GetMatrix().zy += CParticle::m_SinTable[index] * drunk;
	}
*/

	// Commented out widescreen stuff because it puts too much pressure on the streaming
	// make the widescreen stuff get extra view of the screen and not cut off the top and bottom:
//	if (FrontEndMenuManager.m_PrefsUseWideScreen && !CCutsceneMgr::IsCutsceneRunning()) CDraw::SetFOV(FinalFOV+WIDESCREEN_EXTRA_FOV);
//	else
	CDraw::SetFOV(FinalFOV); // DW - Migrated this call as calculate derived values requires it for calculating the frustum planes

	CalculateDerivedValues();



#if 0 // not likely to work, in fact it doesnt.... too much effort  - DW - 
	// Player sprint shaking - hopefully...
	static bool bShakeMe = false;
	bool bFirstTime = false;
	#define COOL_DOWN_FRAMES 50
	static int32 coolDownCounter = 0;
	
	CPed *pPed = (CPed *)Cams[ActiveCam].CamTargetEntity;	
	if (pPed &&
		Cams[ActiveCam].Mode == CCam::MODE_FOLLOWPED &&
		Cams[ActiveCam].CamTargetEntity->GetIsTypePed() &&
		pPed->GetMoveState() == PEDMOVE_SPRINT)
	{			

		bFirstTime = (!bShakeMe);		
		bShakeMe = true;
		coolDownCounter = COOL_DOWN_FRAMES;	
	}
	else
	{	
		if (coolDownCounter--<0) coolDownCounter = 0;	
		bShakeMe = false;			
	}					
	
	if (bShakeMe || coolDownCounter>0)
	{
		CVector f,u,r;	
	
		r.x = GetMatrix().xx; f.x = GetMatrix().xy; u.x = GetMatrix().xz;
		r.y = GetMatrix().yx; f.y = GetMatrix().yy; u.y = GetMatrix().yz;
		r.z = GetMatrix().zx; f.z = GetMatrix().zy; u.z = GetMatrix().zz;

		CHandShaker *pS = &gHandShaker[1];	
		
		if (bFirstTime) pS->Reset();
		
		float degreeShake = (float)coolDownCounter/(float)COOL_DOWN_FRAMES;
		
		static float OverallShakeScaleForSprint = 10.0f;
		degreeShake *= OverallShakeScaleForSprint;
		
		pS->Process(degreeShake);		
		float roll = pS->ang.z * degreeShake; // might be z ???
		CVector temp = Multiply3x3(f,pS->resultMat);
		f = temp;	
		f.Normalise();	

		u=CVector(CMaths::Sin(roll),0.0f,CMaths::Cos(roll));
		r=CrossProduct(f, u);
		r.Normalise();
		u=CrossProduct(r, f);

		if ((f.x==0.0f) && (f.y==0.0f))
		{
			f.x=0.0001f;
			f.y=0.0001f;
		}
		
		r = CrossProduct( f, u );
		r.Normalise();
		u = CrossProduct(r, f );		
				
		GetMatrix().xx = r.x; GetMatrix().xy = f.x; GetMatrix().xz = u.x;
		GetMatrix().yx = r.y; GetMatrix().yy = f.y; GetMatrix().yz = u.y;
		GetMatrix().zx = r.z; GetMatrix().zy = f.z; GetMatrix().zz = u.z;	
		
		CopyCameraMatrixToRWCam();
	}
#endif

	// Now we actually put the matrix that we calculated into the
	// RenderWare matrix.
	
#ifndef MASTER
	//
	// debug FOV control on keyboard...
	//
	if (PS2Keyboard.GetKeyJustDown(PS2_KEY_F, KEYBOARD_MODE_STANDARD, "FOV control")) display_kbd_debug = !display_kbd_debug;
	if (display_kbd_debug)
	{
		static float temp_value = 0.0f;
		if (PS2Keyboard.GetKeyDown(PS2_KEY_LEFT)) temp_value+=0.5f;
		if (PS2Keyboard.GetKeyDown(PS2_KEY_RIGHT)) temp_value-=0.5f;
		
		kbd_fov_value = FinalFOV + temp_value;
		CDraw::SetFOV(kbd_fov_value);
	}
#endif // MASTER
#ifndef MASTER		
	if (!WorldViewerBeingUsed)
#endif	
	{

		CopyCameraMatrixToRWCam(false);

		m_vecGameCamPos.x = GetMatrix().tx;
		m_vecGameCamPos.y = GetMatrix().ty;
		m_vecGameCamPos.z = GetMatrix().tz;

	}
#ifndef MASTER		
	else //ooh we want to look at the world 
	{

		DebugFinalSource = Cams[2].Source;
		DebugFinalFront = Cams[2].Front;
		DebugFinalUp = Cams[2].Up;
		DebugFinalFOV = Cams[2].FOV;

		Right = CrossProduct( DebugFinalUp, DebugFinalFront );
		GetMatrix().xx = Right.x; GetMatrix().xy = DebugFinalFront.x; GetMatrix().xz = DebugFinalUp.x;
		GetMatrix().yx = Right.y; GetMatrix().yy = DebugFinalFront.y; GetMatrix().yz = DebugFinalUp.y;
		GetMatrix().zx = Right.z; GetMatrix().zy = DebugFinalFront.z; GetMatrix().zz = DebugFinalUp.z;
		GetMatrix().tx = DebugFinalSource.x;
		GetMatrix().ty = DebugFinalSource.y;
		GetMatrix().tz = DebugFinalSource.z;

		// make the widescreen stuff get extra view of the screen and not cut off the top and bottom:
		
		// DW - fix for willys camera clothers shop problem
//		if (FrontEndMenuManager.m_PrefsUseWideScreen && !CCutsceneMgr::IsCutsceneRunning()) CDraw::SetFOV(DebugFinalFOV+WIDESCREEN_EXTRA_FOV);
//		else 
		CDraw::SetFOV(DebugFinalFOV);


		m_vecGameCamPos.x=Cams[ActiveCam].Source.x;
		m_vecGameCamPos.y=Cams[ActiveCam].Source.y;
		m_vecGameCamPos.z=Cams[ActiveCam].Source.z;

		CopyCameraMatrixToRWCam(false);

		CIplStore::AddIplsNeededAtPosn(GetPosition());
#ifndef FINAL
		WhatAmILookingAt(this);
#endif		
	}
#endif
	
	UpdateSoundDistances();		// Works out some sound values that the sound boys are interested in


			// This takes into account the FOV. Smaller aperture->bigger LOD multipliers
			// 70 is taken as default angle
	if(CCutsceneMgr::IsCutsceneRunning() && !CCutsceneMgr::UsesLodMultiplier())
	{
		LODDistMultiplier =1.0f;
	}
	else
	{
		LODDistMultiplier = 70.0f / CDraw::GetFOV();
	}
	
	GenerationDistMultiplier = LODDistMultiplier;
	LODDistMultiplier *= CRenderer::GetLodDistanceScale();
	
    float farclip = RwCameraGetFarClipPlane(Scene.camera); 
    farclip *= 100.0f;
    int32 nd = farclip;
    farclip = (float)nd / 100.0f;
    RwCameraSetFarClipPlane(Scene.camera, farclip);     		
	
#if !(defined(MASTER) || defined(FINAL) || defined(FINALBUILD))
	{	
		char str[255];
		sprintf(str,"NEARCLIP %f FARCLIP %f\n",RwCameraGetNearClipPlane(Scene.camera),RwCameraGetFarClipPlane(Scene.camera));
		//VarConsole.AddDebugOutput(str);		
	}
#endif	
	
	// Update values in CDraw
	CDraw::SetNearClipZ(RwCameraGetNearClipPlane(m_pRwCamera));
	CDraw::SetFarClipZ(RwCameraGetFarClipPlane(m_pRwCamera));

	
	//do speed thing for raymond
	if ((m_bJustInitalised==true)||(m_bJust_Switched==true))
	{
		m_PreviousCameraPosition.x=	GetMatrix().tx;
		m_PreviousCameraPosition.y= GetMatrix().ty;
		m_PreviousCameraPosition.z= GetMatrix().tz;
		m_bJustInitalised=false;//Just so the speed thingy doesn't go mad right at the start
	}
	
	
	CVector DistDiff=CVector(GetMatrix().tx - m_PreviousCameraPosition.x, GetMatrix().ty - m_PreviousCameraPosition.y, GetMatrix().tz - m_PreviousCameraPosition.z);

	m_CameraSpeedSoFar+=CMaths::Sqrt(DistDiff.x *DistDiff.x + DistDiff.y * DistDiff.y + DistDiff.z * DistDiff.z); //this is a running total
	
	m_iNumFramesSoFar++; //counter
	
	if (m_iNumFramesSoFar==m_iWorkOutSpeedThisNumFrames)//duh
	{
		m_CameraAverageSpeed=m_CameraSpeedSoFar/m_iWorkOutSpeedThisNumFrames;
		m_CameraSpeedSoFar=0.0f;
		m_iNumFramesSoFar=0;
	}
	
	m_PreviousCameraPosition.x=	GetMatrix().tx;
	m_PreviousCameraPosition.y= GetMatrix().ty;
	m_PreviousCameraPosition.z= GetMatrix().tz;

	
	//if we have done any look lef, behind left/right shit lets swap stuff back
	float OrientPlusPi=Orientation + PI;
	
	if (Cams[ActiveCam].DirectionWasLooking!=LOOKING_FORWARD)
	{
		if (Cams[ActiveCam].Mode!=CCam::MODE_TOP_DOWN_PED)
		{
			Cams[ActiveCam].Source=Cams[ActiveCam].SourceBeforeLookBehind;
			Orientation = OrientPlusPi; //this is a right pain in the fanny 
			//done so that payer control zelda will work
		}
	}

	if (m_uiTransitionState!=TRANS_NONE)
	{	
		if ((Cams[((ActiveCam + 1) % 2)].CamTargetEntity!=NULL)&&(pTargetEntity!=NULL))
		{
			if ((pTargetEntity->GetIsTypePed())&&(!(Cams[(ActiveCam+1) % 2].CamTargetEntity->GetIsTypeVehicle())))
			{
				if (Cams[ActiveCam].Mode!=CCam::MODE_TOP_DOWN_PED)
				{
					if (Cams[(ActiveCam+1) % 2].DirectionWasLooking!=LOOKING_FORWARD)
					{
						Cams[(ActiveCam+1) % 2].Source=Cams[ActiveCam%2].SourceBeforeLookBehind;
						Orientation = OrientPlusPi; //this is a right pain in the fanny 
						//done so that payer control zelda will work 
					}
				}
			}
		}
	}

	m_bCameraJustRestored=false;
	m_bMoveCamToAvoidGeom=false;//gets reset every frame if we need to move the cam


	// Deal with camera being above and below the water level.
	#define ABOVEBELOWWATER (0.6f)
	float	LocalWaterHeight;
	CVector	TestPos = GetPosition() + (0.4f * GetForward());
	bool bWaterFound = CWaterLevel::GetWaterLevel(TestPos.x, TestPos.y, TestPos.z, &LocalWaterHeight, true);

	if (!bWaterFound || LocalWaterHeight < TestPos.z-ABOVEBELOWWATER)
	{
		CWeather::UnderWaterness = 0.0f;
	}
	else
	{
		CWeather::WaterDepth = MAX(0.0f, LocalWaterHeight - TestPos.z);
	
		if (LocalWaterHeight > TestPos.z+ABOVEBELOWWATER)
		{		// Fully submerged
			CWeather::UnderWaterness = 1.0f;
		}
		else
		{		// In the transition area
			CWeather::UnderWaterness = 1.0f - (TestPos.z - (LocalWaterHeight - ABOVEBELOWWATER)) / (ABOVEBELOWWATER + ABOVEBELOWWATER);

				// Render some froth sprites to mask the transition
//			CClouds::m_fFrothIntensity = 1.0f - ABS((TestPos.z - LocalWaterHeight) / ABOVEBELOWWATER);
//			ASSERT(CClouds::m_fFrothIntensity >= 0.0f);
//			CClouds::m_FrothCoors = CVector(TestPos.x, TestPos.y, LocalWaterHeight);
		}
	}

#if 0//ndef MASTER	
	static bool bShowCamMatrix = false;
	if (bShowCamMatrix)
	{
		char str[255];
		sprintf(str,"P %03.5f %03.5f %03.5f",GetMatrix().tx,GetMatrix().ty,GetMatrix().tz);
		VarConsole.AddDebugOutput(str);
		sprintf(str,"R %03.5f %03.5f %03.5f",GetMatrix().xx,GetMatrix().yx,GetMatrix().zx);
		VarConsole.AddDebugOutput(str);
		sprintf(str,"A %03.5f %03.5f %03.5f",GetMatrix().xy,GetMatrix().yy,GetMatrix().zy);
		VarConsole.AddDebugOutput(str);
		sprintf(str,"U %03.5f %03.5f %03.5f",GetMatrix().xz,GetMatrix().yz,GetMatrix().zz);
		VarConsole.AddDebugOutput(str);
	}	
#endif	       
}


//
// FUNCTION: CopyCameraMatrixToRWCam
// Takes the game values of the camera matrix and sticks it in the RW Cam.
//

void CCamera::CopyCameraMatrixToRWCam(bool bMirrorCam)
{
	RwFrame *cameraFrame;
	RwMatrix *cameraMatrix;
	
	cameraFrame = RwCameraGetFrame(m_pRwCamera);
	cameraMatrix = RwFrameGetMatrix(cameraFrame);

#ifndef MASTER
	CheckValidCam();
#endif

	// copy contents of current RwCamera frame to old matrix
	// (make sure we not doing a temp update for a mirror!)
	if(!bMirrorCam)
		m_cameraMatrixOld.UpdateMatrix(cameraMatrix);

	// Coordinates first. The camera rotates about the origin.
	cameraMatrix->pos.x = GetMatrix().tx;
	cameraMatrix->pos.y = GetMatrix().ty;
	cameraMatrix->pos.z = GetMatrix().tz;

	// LookAt vector is the vector to the target
	cameraMatrix->at.x = GetMatrix().xy;
	cameraMatrix->at.y = GetMatrix().yy;
	cameraMatrix->at.z = GetMatrix().zy;
	// Normalise LookAt
	// Set up vector
	cameraMatrix->up.x = GetMatrix().xz;
	cameraMatrix->up.y = GetMatrix().yz;
	cameraMatrix->up.z = GetMatrix().zz;
	// Set right vector
	cameraMatrix->right.x = GetMatrix().xx;
	cameraMatrix->right.y = GetMatrix().yx;
	cameraMatrix->right.z = GetMatrix().zx;
	
#if 1
	// DW - clamp very small changes
	{
		static CVector gLastRWCameraTrans(-99999,-99999,-99999);
		static CVector gLastRWCameraAt(-99999,-99999,-99999);
		static CVector gLastRWCameraUp(-99999,-99999,-99999);
		static CVector gLastRWCameraRight(-99999,-99999,-99999);
	
		static float clampTrans = 0.00001f;
		static float clampRot 	= 0.00001f;
		
		CVector d = gLastRWCameraTrans - cameraMatrix->pos;
		if (d.MagnitudeSqr()<clampTrans*clampTrans)
			cameraMatrix->pos = gLastRWCameraTrans;
	
		d = gLastRWCameraAt - cameraMatrix->at;
		if (d.MagnitudeSqr()<clampRot*clampRot)
			cameraMatrix->at = gLastRWCameraAt;
	
		d = gLastRWCameraUp - cameraMatrix->up;
		if (d.MagnitudeSqr()<clampRot*clampRot)
			cameraMatrix->up = gLastRWCameraUp;

		d = gLastRWCameraRight - cameraMatrix->right;
		if (d.MagnitudeSqr()<clampRot*clampRot)
			cameraMatrix->right = gLastRWCameraRight;

		gLastRWCameraTrans 	= cameraMatrix->pos;
		gLastRWCameraAt 	= cameraMatrix->at;
		gLastRWCameraUp 	= cameraMatrix->up;
		gLastRWCameraRight 	= cameraMatrix->right;			
	}
#endif
	
#ifndef MASTER
	CheckValidCam();
#endif

	RwMatrixUpdate(cameraMatrix);
	
#ifndef MASTER
	CheckValidCam();
#endif
	
	RwFrameUpdateObjects(cameraFrame);

#ifndef MASTER
	CheckValidCam();
#endif
	
	RwFrameOrthoNormalize(cameraFrame);
	
#ifndef MASTER
	CheckValidCam();
#endif

	if(m_bResetOldMatrix && !bMirrorCam)
	{
		m_cameraMatrixOld.UpdateMatrix(cameraMatrix);
		m_bResetOldMatrix = false;
	}
}

//
// FUNCTION: CalculateMirroredMatrix
//
//

void CCamera::CalculateMirroredMatrix(CVector MirrorNormal, float MirrorV, CMatrix *pMat, CMatrix *pResult)
{
	CVector	CamPos = pMat->GetTranslate();
	float	DistanceOut = DotProduct(CamPos, MirrorNormal) - MirrorV;
	CVector NewCameraPos = CamPos - (DistanceOut * 2.0f) * MirrorNormal;

	pResult->GetTranslate().x = NewCameraPos.x;
	pResult->GetTranslate().y = NewCameraPos.y;
	pResult->GetTranslate().z = NewCameraPos.z;

	CVector OldAt = CVector(pMat->GetForward().x, pMat->GetForward().y, pMat->GetForward().z);		
	float	InMirrorDir = DotProduct(OldAt, MirrorNormal);
	CVector NewAt = OldAt - (2.0f * InMirrorDir) * MirrorNormal;

	pResult->GetForward().x = NewAt.x;
	pResult->GetForward().y = NewAt.y;
	pResult->GetForward().z = NewAt.z;

	CVector OldUp = CVector(pMat->GetUp().x, pMat->GetUp().y, pMat->GetUp().z);
	float	InMirrorDirUp = DotProduct(OldUp, MirrorNormal);
	CVector NewUp = OldUp - (2.0f * InMirrorDirUp) * MirrorNormal;

	pResult->GetUp().x = NewUp.x;
	pResult->GetUp().y = NewUp.y;
	pResult->GetUp().z = NewUp.z;

	pResult->GetRight().x = pResult->GetForward().z * pResult->GetUp().y - pResult->GetForward().y * pResult->GetUp().z;
	pResult->GetRight().y = pResult->GetForward().x * pResult->GetUp().z - pResult->GetForward().z * pResult->GetUp().x;
	pResult->GetRight().z = pResult->GetForward().y * pResult->GetUp().x - pResult->GetForward().x * pResult->GetUp().y;

}

//
// FUNCTION: SetCameraUpForMirror
// Mirrors the camera and does everything needed to render the scene for the mirror image.
//
CMatrix	StoreMatrixForMirror;

void CCamera::SetCameraUpForMirror()
{
	CMatrix	*pMat = &GetMatrix();
	StoreMatrixForMirror = *pMat;

//	CalculateMirroredMatrix(MirrorNormal, MirrorV, pMat, pMat);
	*pMat = m_matMirror;	// Has already been calculated in DealWithMirrorBeforeConstructRenderList

	CopyCameraMatrixToRWCam(true);
	CalculateDerivedValues(true, false);	// Calc inverse
}

//
// FUNCTION: RestoreCameraAfterMirror
// Restores the camera so that we can now render the scene normally.
//

void CCamera::RestoreCameraAfterMirror()
{
	SetMatrix(StoreMatrixForMirror);

	CopyCameraMatrixToRWCam(true);
	CalculateDerivedValues(false, false);	// Calc inverse
}

//
// FUNCTION: DealWithMirrorBeforeConstructRenderList
// Set the 2nd inverse matrix so that we can test on-screen accurately.
//

void CCamera::DealWithMirrorBeforeConstructRenderList(bool bMirrorActive, CVector MirrorNormal, float MirrorV, CMatrix *pMatrix)
{
	m_bMirrorActive = bMirrorActive;
	
	if (m_bMirrorActive)
	{		
		if (pMatrix)
		{
			m_matMirror = *pMatrix;
		}
		else
		{
			CalculateMirroredMatrix(MirrorNormal, MirrorV, &GetMatrix(), &m_matMirror);
		}

		m_matMirrorInverse = Invert(m_matMirror);
	}

}



//
// name:		CalculateFrustumPlanes
// description:	Calculate frustum planes in camera space and world space
void CCamera::CalculateFrustumPlanes(bool bMirrored)
{
	float fov = CDraw::GetFOV() * (3.1415f / 180.0f) / 2.0f;
	float fCos = CMaths::Cos(fov);
	float fSin = CMaths::Sin(fov);

	m_vecFrustumNormals[FRUSTUM_LEFT] = CVector(fCos, -fSin, 0.0f);
	m_vecFrustumNormals[FRUSTUM_RIGHT] = CVector(-fCos, -fSin, 0.0f);

	fCos *= SCREEN_HEIGHT/(float)SCREEN_WIDTH;
	fSin *= SCREEN_HEIGHT/(float)SCREEN_WIDTH;
	
	m_vecFrustumNormals[FRUSTUM_BOTTOM] = CVector(0.0f, -fSin, -fCos);
	m_vecFrustumNormals[FRUSTUM_TOP] = CVector(0.0f, -fSin, fCos);

	if (!bMirrored)
	{
		// Calculate normals for world space planes
		TransformVectors(m_vecFrustumWorldNormals, 4, GetMatrix(), m_vecFrustumNormals);

		// Calculate plane offsets for world space planes
		for(int32 i=0; i<4; i++)
			m_fFrustumPlaneOffsets[i] = DotProduct(*(RwV3d*)&m_vecFrustumWorldNormals[i], GetPosition());
	}
	else
	{
		// Calculate normals for world space planes
		TransformVectors(m_vecFrustumWorldNormals_Mirror, 4, GetMatrix(), m_vecFrustumNormals);

		// Calculate plane offsets for world space planes
		for(int32 i=0; i<4; i++)
			m_fFrustumPlaneOffsets_Mirror[i] = DotProduct(*(RwV3d*)&m_vecFrustumWorldNormals_Mirror[i], GetPosition());
	}
}

//////////////////////////////////////////////////
void CCamera::CalculateDerivedValues(bool bMirror, bool bUpdateOrientation)
{
	// Calculate some derived values that other bits of code use.
	m_matInverse = Invert(GetMatrix());

	CalculateFrustumPlanes(bMirror);

	if ( (GetMatrix().xy == 0.0f) && (GetMatrix().yy == 0.0f) )
	{
		GetMatrix().xy=0.0001f;//just enough to stop it crashing
	}
	else if(bUpdateOrientation)
	{
		Orientation = CMaths::ATan2(GetMatrix().xy, GetMatrix().yy);
	}

	// Calculate the front vector normalised to 1 if the z was 0. Used by other bits of code.
	CamFrontXNorm = GetMatrix().GetForward().x;
	CamFrontYNorm = GetMatrix().GetForward().y;
	float Length = CMaths::Sqrt(CamFrontXNorm*CamFrontXNorm + CamFrontYNorm*CamFrontYNorm);
	if (Length == 0.0f)
	{
		CamFrontXNorm = 1.0f;
	}
	else
	{
		CamFrontXNorm /= Length;
		CamFrontYNorm /= Length;
	}

}

//////////////////////////////////////////////////
void CCamera::ProcessFade(void)
{
/*	float TimeElapsed=(CTimer::GetTimeInMilliseconds()-m_uiFadeTimeStarted)/1000.0f;
	if (m_bFading==TRUE)
	{
		if (m_iFadingDirection==FADE_IN)
		{
			if (m_fTimeToFadeOut==0.0f) 	
			{
				m_bFading=FALSE;
				CDraw::FadeValue=0;
				m_fFloatingFade=0.0f;
			}
			else
			{
				m_fFloatingFade=255 - (255 * TimeElapsed/m_fTimeToFadeOut);
				if (m_fFloatingFade<=0.0f)
				{
					m_bFading=FALSE;
					CDraw::FadeValue=0;
					m_fFloatingFade=0.0f;
				}
			}
		}
		else if (m_iFadingDirection==FADE_OUT)
		{
			if (m_fTimeToFadeOut==0.0f) 	
			{
				m_bFading=FALSE;
				CDraw::FadeValue=255;
				m_fFloatingFade=255.0f;
			}
			else
			{
				m_fFloatingFade=(255 * TimeElapsed/m_fTimeToFadeOut);
				if (m_fFloatingFade>=255.0f)
				{
					m_bFading=FALSE;
					CDraw::FadeValue=255;
					m_fFloatingFade=255.0f;
				}
			}
		}
		CDraw::FadeValue=m_fFloatingFade;
	}*/
	
	if (m_bFading==TRUE)
	{
		if (m_iFadingDirection==FADE_IN)
		{
			if(m_fTimeToFadeOut == 0.0f)
				m_fFloatingFade = 0.0f;
			else
				m_fFloatingFade -= 255.0f * CTimer::GetTimeStepInSeconds()/m_fTimeToFadeOut;
			if (m_fFloatingFade <= 0.0f)
			{

				m_bFading=FALSE;
				m_fFloatingFade=0.0f;
			}
		}
		else if (m_iFadingDirection==FADE_OUT)
		{
			if(m_fFloatingFade >= 255.0f)
				m_bFading=FALSE;
			if(m_fTimeToFadeOut == 0.0f)
				m_fFloatingFade = 255.0f;
			else
				m_fFloatingFade += 255.0f * CTimer::GetTimeStepInSeconds()/m_fTimeToFadeOut;
			if (m_fFloatingFade >= 255.0f)
			{
				m_fFloatingFade=255.0f;
			}	
		}
		CDraw::FadeValue=m_fFloatingFade;
	}
}

///////////////////////////////////////////////
void CCamera::ProcessMusicFade(void)
{
/*	float TimeElapsed=(CTimer::GetTimeInMilliseconds()-m_uiFadeTimeStartedMusic)/1000.0f;
	if (m_bMusicFading==TRUE)
	{
		if (m_iMusicFadingDirection==FADE_IN)
		{
			if (m_fTimeToFadeMusic==0.0f) 	
				m_fFloatingFadeMusic=256.0f;
			else
				m_fFloatingFadeMusic=(255 * TimeElapsed/m_fTimeToFadeMusic);
				
			if (m_fFloatingFadeMusic>255.0f)
			{
				m_bMusicFading=FALSE;
				m_fFloatingFadeMusic=0.0f;
				DMAudio.SetEffectsFadeVol(127);	// Faded in completely
				DMAudio.SetMusicFadeVol(127);	// Faded in completely
			}
			else
			{
				UInt8 Vol = (127 * (m_fFloatingFadeMusic/255));
				DMAudio.SetEffectsFadeVol(Vol);
				DMAudio.SetMusicFadeVol(Vol);	
			}
		}
		else if (m_iMusicFadingDirection==FADE_OUT)
		{
			if(m_fTimeToFadeMusic==0.0f) 
				m_fFloatingFadeMusic=256.0f;
			else
				m_fFloatingFadeMusic=(255 * TimeElapsed/m_fTimeToFadeMusic);
			
			if (m_fFloatingFadeMusic>255.0f)

			{
				m_bMusicFading=FALSE;
				m_fFloatingFadeMusic=255.0f;
				DMAudio.SetEffectsFadeVol(0);	// Faded out completely
				DMAudio.SetMusicFadeVol(0);	// Faded out completely
			}
			else
			{
				UInt8 Vol = 127 - (127 * (m_fFloatingFadeMusic/255));
				DMAudio.SetEffectsFadeVol(Vol);
				DMAudio.SetMusicFadeVol(Vol);	
			}

		}
	}*/

#ifdef USE_AUDIOENGINE
	if(m_bMusicFading)
	{
		if(m_fTimeToWaitToFadeMusic > 0.0f)
		{
			m_fTimeToWaitToFadeMusic -= CTimer::GetTimeStepInSeconds();
		}
		else if(m_iMusicFadingDirection == FADE_IN)
		{
			if(m_fTimeToFadeMusic <= 0.0f)
				m_fFloatingFadeMusic = 1.0f;
			else
				m_fFloatingFadeMusic += CTimer::GetTimeStepInSeconds() / m_fTimeToFadeMusic;
			if(m_fFloatingFadeMusic >= 1.0f)
			{
				m_bMusicFadedOut = FALSE;
				m_bMusicFading = FALSE;
				m_fFloatingFadeMusic = 1.0f;
			}
		}
		else if(m_iFadingDirection == FADE_OUT)
		{
			if(m_fFloatingFadeMusic <= 0.0f)
			{
				m_bMusicFadedOut = TRUE;
				m_bMusicFading = FALSE;
				m_fFloatingFadeMusic = 0.0f;
			}
			if(m_fTimeToFadeMusic <= 0.0f)
				m_fFloatingFadeMusic = 0.0f;
			else
				m_fFloatingFadeMusic -= CTimer::GetTimeStepInSeconds() / m_fTimeToFadeMusic;
			if(m_fFloatingFadeMusic <= 0.0f)
			{
				m_fFloatingFadeMusic = 0.0f;
			}
		}

		if(!AudioEngine.IsLoadingTuneActive())
		{
			AudioEngine.SetMusicFaderScalingFactor(m_fFloatingFadeMusic);
			AudioEngine.SetEffectsFaderScalingFactor(m_fFloatingFadeMusic);	
		}
	}
#else //USE_AUDIOENGINE
	if (m_bMusicFading==TRUE)
	{
		if (m_iMusicFadingDirection==FADE_IN)
		{
			if(m_fTimeToFadeMusic == 0.0f)
				m_fFloatingFadeMusic = 0.0f;
			else
				m_fFloatingFadeMusic -= 255.0f * CTimer::GetTimeStepInSeconds()/m_fTimeToFadeMusic;
			if (m_fFloatingFadeMusic <= 0.0f)
			{
				m_bMusicFading=FALSE;
				m_fFloatingFadeMusic=0.0f;
			}
		}
		else if (m_iFadingDirection==FADE_OUT)
		{
			if(m_fTimeToFadeMusic == 0.0f)
				m_fFloatingFadeMusic = 255.0f;
			else
				m_fFloatingFadeMusic += 255.0f * CTimer::GetTimeStepInSeconds()/m_fTimeToFadeMusic;
			if (m_fFloatingFadeMusic >= 255.0f)
			{
				m_bMusicFading=FALSE;
				m_fFloatingFadeMusic=255.0f;
			}
		}
		int32 Vol = 127.0f - (127.0f * (m_fFloatingFadeMusic/255.0f));
		DMAudio.SetEffectsFadeVol(Vol);
		DMAudio.SetMusicFadeVol(Vol);	
	}
#endif //USE_AUDIOENGINE
}

#define DW_CAMS
#ifdef DW_CAMS
	#define NUM_CAMS_CAR (11) // DW- note how camera 0 is used a bit here to make the heli cams and fov zoom cams recover the streaming less noticably.
	Int32	SequenceOfCarCams[NUM_CAMS_CAR+1] = {	MOVIECAM20, MOVIECAM22,MOVIECAM7, MOVIECAM3, MOVIECAM22, MOVIECAM1, MOVIECAM21, MOVIECAM8, MOVIECAM2, MOVIECAM22, MOVIECAM5, MOVIECAM6 }; // One extra mode that is 'safe' in case all others fail

	#define NUM_CAMS_TRAIN (6) 
	Int32	SequenceOfTrainCams[NUM_CAMS_TRAIN+1] = {	MOVIECAM20, MOVIECAM22, MOVIECAM2, MOVIECAM21, MOVIECAM3, MOVIECAM0 }; // One extra mode that is 'safe' in case all others fail

	const int NUM_CAMS_HELI=7; // new ones added to be like the planes - looks better - DW
	int32 SequenceOfHeliCams[NUM_CAMS_HELI+1]={MOVIECAMPLANE1,MOVIECAMPLANE2,  MOVIECAM23,MOVIECAM20, MOVIECAM22,MOVIECAMPLANE3,MOVIECAM23, MOVIECAM0};
	
	// DW - NOTE TO SELF : If you want to get something write it yourself. DO NOT FORGET!!!
	const int NUM_CAMS_PLANE=6; 
	int32 SequenceOfPlaneCams[NUM_CAMS_PLANE+1]={MOVIECAMPLANE1,MOVIECAMPLANE2, MOVIECAM20, MOVIECAM22,MOVIECAMPLANE3, MOVIECAM23, MOVIECAM0};

	const int NUM_CAMS_BOAT=3; 
	int32 SequenceOfBoatCams[NUM_CAMS_BOAT+1]={MOVIECAM20,MOVIECAM3,MOVIECAM18, MOVIECAM0};

#else
	#define NUM_CAMS_CAR (14)
	Int32	SequenceOfCarCams[NUM_CAMS_CAR+1] = {
				MOVIECAM0, MOVIECAM7, MOVIECAM3, MOVIECAM1, MOVIECAM3, MOVIECAM8, MOVIECAM2,
				MOVIECAM3, MOVIECAM8, MOVIECAM7, MOVIECAM2, MOVIECAM3, MOVIECAM5, MOVIECAM3,
				MOVIECAM6		// One extra mode that is 'safe' in case all others fail
	};

	const int NUM_CAMS_HELI=1; 
	int32 SequenceOfHeliCams[NUM_CAMS_HELI]={MOVIECAM14, MOVIECAM15, MOVIECAM16, MOVIECAM17, MOVIECAM18, MOVIECAM19};
#endif 



/* REDUNDANT CODE - DW
#define NUMPEDCAMS (5)
Int32	SequenceOfPedCams[NUMPEDCAMS] = {
			MOVIECAM9, MOVIECAM10, MOVIECAM11, MOVIECAM12, MOVIECAM13,
};
*/


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ProcessObbeCinemaCameraCar
// PURPOSE :  Picks a new camera mode depending on what looks best.
/////////////////////////////////////////////////////////////////////////////////
void CCamera::ProcessObbeCinemaCameraCar()
{
	static Int32	OldMode = -1;
	static Int32	TimeForNext = 0;
	Int32 InfiniteLoopSafetyCounterTwo=0;//probably will never happen but good to avoid it just incase
	
	if (!bDidWeProcessAnyCinemaCam)
	{
		OldMode = -1;
		///we've just switched 
		///print your switched message here		
		if (gbCineyCamMessageDisplayed>0 && !m_bForceCinemaCam)
		{
			gbCineyCamMessageDisplayed--;
			CHud::SetHelpMessage(TheText.Get("CINCAM"), true);			
		}

		JustGoneIntoObbeCamera=true;

	}

	if ((!bDidWeProcessAnyCinemaCam) || CCamera::IsItTimeForNewcam(SequenceOfCarCams[OldMode], TimeForNext))
	{
		
		OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_CAR;
		if (OldMode<0) OldMode = NUM_CAMS_CAR-1;
		else if (OldMode>NUM_CAMS_CAR-1) OldMode = 0;
		
		while ((!CCamera::TryToStartNewCamMode(SequenceOfCarCams[OldMode]))&&(InfiniteLoopSafetyCounterTwo<=NUM_CAMS_CAR))
		{
			OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_CAR;
			if (OldMode<0) OldMode = NUM_CAMS_CAR-1;
			else if (OldMode>NUM_CAMS_CAR-1) OldMode = 0;		
			InfiniteLoopSafetyCounterTwo++;
		}
		TimeForNext = CTimer::GetTimeInMilliseconds();
		if (InfiniteLoopSafetyCounterTwo>=NUM_CAMS_CAR)
		{	// pick a safe mode
			OldMode = NUM_CAMS_CAR;
			CCamera::TryToStartNewCamMode(SequenceOfCarCams[OldMode]);
		}
	}
	m_iModeObbeCamIsInForCar=OldMode;
	
	bDidWeProcessAnyCinemaCam = true;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ProcessObbeCinemaCameraTrain
// PURPOSE :  Picks a new camera mode depending on what looks best.
/////////////////////////////////////////////////////////////////////////////////
void CCamera::ProcessObbeCinemaCameraTrain()
{
	static Int32	OldMode = -1;
	static Int32	TimeForNext = 0;
	Int32 InfiniteLoopSafetyCounterTwo=0;//probably will never happen but good to avoid it just incase
	
	if (!bDidWeProcessAnyCinemaCam)
	{
		OldMode = -1;
		///we've just switched 
		///print your switched message here
		if (gbCineyCamMessageDisplayed>0 && !m_bForceCinemaCam)
		{
			gbCineyCamMessageDisplayed--;
			CHud::SetHelpMessage(TheText.Get("CINCAM"), true);			
		}

		JustGoneIntoObbeCamera=true;

	}

	if ((!bDidWeProcessAnyCinemaCam) || CCamera::IsItTimeForNewcam(SequenceOfTrainCams[OldMode], TimeForNext))
	{
		OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_TRAIN;
		if (OldMode<0) OldMode = NUM_CAMS_TRAIN-1;
		else if (OldMode>NUM_CAMS_TRAIN-1) OldMode = 0;

		
		while ((!CCamera::TryToStartNewCamMode(SequenceOfTrainCams[OldMode]))&&(InfiniteLoopSafetyCounterTwo<=NUM_CAMS_TRAIN))
		{
			OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_TRAIN;
			if (OldMode<0) OldMode = NUM_CAMS_TRAIN-1;
			else if (OldMode>NUM_CAMS_TRAIN-1) OldMode = 0;			

			InfiniteLoopSafetyCounterTwo++;
		}
		TimeForNext = CTimer::GetTimeInMilliseconds();
		if (InfiniteLoopSafetyCounterTwo>=NUM_CAMS_TRAIN)
		{	// pick a safe mode
			OldMode = NUM_CAMS_TRAIN;
			CCamera::TryToStartNewCamMode(SequenceOfTrainCams[OldMode]);
		}
	}
	m_iModeObbeCamIsInForCar=OldMode;
		
	
	bDidWeProcessAnyCinemaCam = true;
}


//---------------------------------------------------------------------------
bool CameraObscuredByWaterLevel()
{
	CVector *pCamPos = &TheCamera.Cams[TheCamera.ActiveCam].Source;
	float LocalWaterHeight;
	bool bWaterFound = CWaterLevel::GetWaterLevel(pCamPos->x, pCamPos->y,pCamPos->z, &LocalWaterHeight, true);
	if (!bWaterFound)
		return false;
	if (LocalWaterHeight<pCamPos->z)
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ProcessObbeCinemaCameraHeli
// PURPOSE :  Picks a new camera mode depending on what looks best.
/////////////////////////////////////////////////////////////////////////////////
void CCamera::ProcessObbeCinemaCameraHeli()
{
	static Int32	OldMode = MOVIECAM14;
	static Int32	TimeForNext = 0;
	Int32 InfiniteLoopSafetyCounterTwo=0;//probably will never happen but good to avoid it just incase
	
	if (!bDidWeProcessAnyCinemaCam)
	{
		OldMode = -1;
		///we've just switched 
		///print your switched message here
		if (gbCineyCamMessageDisplayed>0 && !m_bForceCinemaCam)
		{
			gbCineyCamMessageDisplayed--;
			CHud::SetHelpMessage(TheText.Get("CINCAM"), true);			
		}

		JustGoneIntoObbeCamera=true;

	}

	if ((!bDidWeProcessAnyCinemaCam) || CameraObscuredByWaterLevel() ||  CCamera::IsItTimeForNewcam(SequenceOfHeliCams[OldMode], TimeForNext))
	{
		OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_HELI;
		if (OldMode<0) OldMode = NUM_CAMS_HELI-1;
		else if (OldMode>NUM_CAMS_HELI-1) OldMode = 0;	
		
		while ((!CCamera::TryToStartNewCamMode(SequenceOfHeliCams[OldMode]))&&(InfiniteLoopSafetyCounterTwo<=NUM_CAMS_HELI))
		{
			OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_HELI;
			if (OldMode<0) OldMode = NUM_CAMS_HELI-1;
			else if (OldMode>NUM_CAMS_HELI-1) OldMode = 0;	
					
			InfiniteLoopSafetyCounterTwo++;
		}
		
		if (InfiniteLoopSafetyCounterTwo>=NUM_CAMS_HELI)
		{	// pick a safe mode
			// go into the normal camera on a string mode
			OldMode=NUM_CAMS_HELI;//so will always be reset
			if (Cams[ActiveCam].Mode!=CCam::MODE_CAM_ON_A_STRING)//could theorectically be an occasion when we can't fin a 
			//suitable mode for longer that 5000 so we don't want to tell the camera to do a jump cut we we are already in the mode 
			{
				CCamera::TryToStartNewCamMode(CAM_ON_A_STRING_LAST_RESORT);
				TimeForNext = CTimer::GetTimeInMilliseconds();
			}
		}
		else
		{
			TimeForNext = CTimer::GetTimeInMilliseconds();
		}
	}
	m_iModeObbeCamIsInForCar=OldMode;
	bDidWeProcessAnyCinemaCam = true;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ProcessObbeCinemaCameraHeli
// PURPOSE :  Picks a new camera mode depending on what looks best.
/////////////////////////////////////////////////////////////////////////////////

void CCamera::ProcessObbeCinemaCameraPlane()
{
	static Int32	OldMode = -1;
	static Int32	TimeForNext = 0;
	Int32 InfiniteLoopSafetyCounterTwo=0;//probably will never happen but good to avoid it just incase
	
	if (!bDidWeProcessAnyCinemaCam)
	{
		OldMode = -1;
		///we've just switched 
		///print your switched message here
	
		JustGoneIntoObbeCamera=true;
		if (gbCineyCamMessageDisplayed>0 && !m_bForceCinemaCam)
		{
			gbCineyCamMessageDisplayed--;
			CHud::SetHelpMessage(TheText.Get("CINCAM"), true);			
		}
	}

	if ((!bDidWeProcessAnyCinemaCam) || CameraObscuredByWaterLevel() || CCamera::IsItTimeForNewcam(SequenceOfPlaneCams[OldMode], TimeForNext))
	{
		OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_PLANE;
		if (OldMode<0) OldMode = NUM_CAMS_PLANE-1;
		else if (OldMode>NUM_CAMS_PLANE-1) OldMode = 0;	
		
		while ((!CCamera::TryToStartNewCamMode(SequenceOfPlaneCams[OldMode]))&&(InfiniteLoopSafetyCounterTwo<=NUM_CAMS_PLANE))
		{
			OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_PLANE;
			if (OldMode<0) OldMode = NUM_CAMS_PLANE-1;
			else if (OldMode>NUM_CAMS_PLANE-1) OldMode = 0;	
	
			InfiniteLoopSafetyCounterTwo++;
		}
		
		if (InfiniteLoopSafetyCounterTwo>=NUM_CAMS_PLANE)
		{	// pick a safe mode
			// go into the normal camera on a string mode
			OldMode=NUM_CAMS_PLANE;//so will always be reset
			if (Cams[ActiveCam].Mode!=CCam::MODE_CAM_ON_A_STRING)//could theorectically be an occasion when we can't fin a 
			//suitable mode for longer that 5000 so we don't want to tell the camera to do a jump cut we we are already in the mode 
			{
				CCamera::TryToStartNewCamMode(CAM_ON_A_STRING_LAST_RESORT);
				TimeForNext = CTimer::GetTimeInMilliseconds();
			}
		}
		else
		{
			TimeForNext = CTimer::GetTimeInMilliseconds();
		}
	}
	m_iModeObbeCamIsInForCar=OldMode;
	bDidWeProcessAnyCinemaCam = true;
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ProcessObbeCinemaCameraBoat
// PURPOSE :  Picks a new camera mode depending on what looks best.
/////////////////////////////////////////////////////////////////////////////////
void CCamera::ProcessObbeCinemaCameraBoat()
{
	static Int32	OldMode = MOVIECAM14;
	static Int32	TimeForNext = 0;
	Int32 InfiniteLoopSafetyCounterTwo=0;//probably will never happen but good to avoid it just incase
	
	if (!bDidWeProcessAnyCinemaCam)
	{
		OldMode = -1;
		///we've just switched 
		///print your switched message here
		if (gbCineyCamMessageDisplayed>0 && !m_bForceCinemaCam)
		{
			gbCineyCamMessageDisplayed--;
			CHud::SetHelpMessage(TheText.Get("CINCAM"), true);			
		}

		JustGoneIntoObbeCamera=true;

	}

	if ((!bDidWeProcessAnyCinemaCam) || CCamera::IsItTimeForNewcam(SequenceOfBoatCams[OldMode], TimeForNext))
	{
		OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_BOAT;
		if (OldMode<0) OldMode = NUM_CAMS_BOAT-1;
		else if (OldMode>NUM_CAMS_BOAT-1) OldMode = 0;		
			
		while ((!CCamera::TryToStartNewCamMode(SequenceOfBoatCams[OldMode]))&&(InfiniteLoopSafetyCounterTwo<=NUM_CAMS_BOAT))
		{
			OldMode = (OldMode + gCinematicModeSwitchDir) % NUM_CAMS_BOAT;
			if (OldMode<0) OldMode = NUM_CAMS_BOAT-1;
			else if (OldMode>NUM_CAMS_BOAT-1) OldMode = 0;		
			InfiniteLoopSafetyCounterTwo++;
		}
		
		if (InfiniteLoopSafetyCounterTwo>=NUM_CAMS_BOAT)
		{	// pick a safe mode
			// go into the normal camera on a string mode
			OldMode=NUM_CAMS_BOAT;//so will always be reset
			if (Cams[ActiveCam].Mode!=CCam::MODE_CAM_ON_A_STRING)//could theorectically be an occasion when we can't fin a 
			//suitable mode for longer that 5000 so we don't want to tell the camera to do a jump cut we we are already in the mode 
			{
				CCamera::TryToStartNewCamMode(CAM_ON_A_STRING_LAST_RESORT);
				TimeForNext = CTimer::GetTimeInMilliseconds();
			}
		}
		else
		{
			TimeForNext = CTimer::GetTimeInMilliseconds();
		}
	}
	m_iModeObbeCamIsInForCar=OldMode;
	bDidWeProcessAnyCinemaCam = true;
}




////////////////////////////////////////////

void CCamera::ProcessObbeCinemaCameraPed()
{
}

#ifdef GTA_TRAIN

///////////////////////////////////////////
void CCamera::Process_Train_Camera_Control()
{
	CTrain* LocalPointerToTheTrain=((CTrain*)pTargetEntity);
	//CTrain* LocalPointerToTheTrain=(CTrain*)CPools::GetVehicle(1);
	bool FoundASuitableNode=false;
	bool JustLoaded=false;
	UInt32 NodeIterator=0;
	CVector HandyDistVector;
	m_bUseSpecialFovTrain=true;
	static bool OKtoGoBackToNodeCam=true;//if we are in non train node cam mode don't just jump around,
	//lets make sure that we've been in it for at least 5 seconds.

	
	///if the train splines have not been loaded then lets load them
	if ((LocalPointerToTheTrain->m_bUndergroundTrain==false) && (m_bAboveGroundTrainNodesLoaded==false))
	{
		m_bAboveGroundTrainNodesLoaded=true;
		m_bBelowGroundTrainNodesLoaded=false;
		LoadTrainCamNodes("Train");
		m_uiTimeLastChange=CTimer::GetTimeInMilliseconds();
		OKtoGoBackToNodeCam=true;
		m_iCurrentTrainCamNode=0;
		JustLoaded=true;
	}

	///if the train splines have not been loaded then lets load them
	if ((LocalPointerToTheTrain->m_bUndergroundTrain==true) && (m_bBelowGroundTrainNodesLoaded==false))
	{
		m_bBelowGroundTrainNodesLoaded=true;
		m_bAboveGroundTrainNodesLoaded=false;
		LoadTrainCamNodes("Train2");
		m_uiTimeLastChange=CTimer::GetTimeInMilliseconds();
		OKtoGoBackToNodeCam=true;
		m_iCurrentTrainCamNode=0;
		JustLoaded=true;
	}

	
	m_bTargetJustBeenOnTrain=true;
	
	NodeIterator=m_iCurrentTrainCamNode;
	int LoopCounter=0;
	//lets find the best node to go to	
	//i.e. the one that it is closest to the camera
	//do debug shit

	while ((LoopCounter<m_uiNumberOfTrainCamNodes)&&(FoundASuitableNode==false))
	{
		if (LocalPointerToTheTrain->IsWithinArea(m_arrTrainCamNode[NodeIterator].m_cvecMinPointInRange.x, m_arrTrainCamNode[NodeIterator].m_cvecMinPointInRange.y, m_arrTrainCamNode[NodeIterator].m_cvecMinPointInRange.z, m_arrTrainCamNode[NodeIterator].m_cvecMaxPointInRange.x, m_arrTrainCamNode[NodeIterator].m_cvecMaxPointInRange.y, m_arrTrainCamNode[NodeIterator].m_cvecMaxPointInRange.z))
		{
			FoundASuitableNode=true;
			m_iCurrentTrainCamNode=NodeIterator;
		}
		NodeIterator++;
		//better clip
		if (NodeIterator>=m_uiNumberOfTrainCamNodes)
		{
			NodeIterator=0.0f;
		}
		LoopCounter++;
	}

	if (FoundASuitableNode==true)///right baw bags we've found a good node lets go there
	{
		SetWideScreenOn();		
		float TrainSpeed=DotProduct(((CTrain*)pTargetEntity)->GetMoveSpeed(), pTargetEntity->GetMatrix().GetForward());
		//if	((TrainSpeed<TrainSpeedToGoIntoAaronCam)&&(FindPlayerPed()->GetPedState()==PED_DRIVING))//we are slowing down to stop, don't bother changing, it just looks gash 
		if	(TrainSpeed<TrainSpeedToGoIntoAaronCam)
		{
			TakeControl(FindPlayerPed(), CCam::MODE_FOLLOWPED, TRANS_JUMP_CUT);
			if(((CTrain*)LocalPointerToTheTrain)->Door[0].IsFullyOpen())
			{
					//we are still in control so turn widescreen off
					SetWideScreenOff();
			}
		}
		else
		{
			SetCamPositionForFixedMode(m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecCamPosition, CVector(0,0,0));
			if (((m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt.x == 999) && (m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt.y == 999)) 
			&& (m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt.z==999))
			{
				TakeControl(LocalPointerToTheTrain, CCam::MODE_FIXED, TRANS_JUMP_CUT);
				
			}
			else
			{
				TakeControlNoEntity((m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt), TRANS_JUMP_CUT);
			}
			ASSERT (m_arrTrainCamNode[m_iCurrentTrainCamNode].m_fDesiredFOV<=70.0f);
			RwCameraSetNearClipPlane(Scene.camera, m_arrTrainCamNode[m_iCurrentTrainCamNode].m_fNearClip); //for occasions when the spaz user is right up against a wall
		}
	}
	else 
	{
		float TrainSpeed=DotProduct(((CTrain*)pTargetEntity)->GetMoveSpeed(), pTargetEntity->GetMatrix().GetForward());
		//if	((TrainSpeed<TrainSpeedToGoIntoAaronCam)&&(FindPlayerPed()->GetPedState()==PED_DRIVING))//we are slowing down to stop, don't bother changing, it just looks gash 
		if	(TrainSpeed<TrainSpeedToGoIntoAaronCam)
		{
			TakeControl(FindPlayerPed(), CCam::MODE_FOLLOWPED, TRANS_JUMP_CUT);
			if(((CTrain*)LocalPointerToTheTrain)->Door[0].IsFullyOpen())
			{
					//we are still in control so turn widescreen off
					SetWideScreenOff();
			}
		}
	}
}

#endif



void CCamera::ProcessWideScreenOn()
{
	if (m_bWantsToSwitchWidescreenOff==false)
	{
		m_fFOV_Wide_Screen=(Cams[ActiveCam].FOV * (MAX_WIDE_SCREEN_REDUCTION/100));		
		m_fWideScreenReductionAmount= 1.0f;
		m_ScreenReductionPercentage=MAX_WIDE_SCREEN_REDUCTION;
	}
	else
	{
		m_bWantsToSwitchWidescreenOff=false;
		m_WideScreenOn=false;
		m_ScreenReductionPercentage=0.0f;
		m_fFOV_Wide_Screen=0.0f;
		m_fWideScreenReductionAmount= 0.0f;
	}
}



/////////////////////////////
void CCamera::RenderMotionBlur()
{
//	if (!postFx.TuneColor && postFx.On)
//		postFx.SetColour((RwUInt8) m_BlurRed, (RwUInt8) m_BlurGreen, (RwUInt8) m_BlurBlue);
	{
	 	//#ifndef DEBUG //motion blur doesnae work in debug for some weirdo reason
		if (m_BlurType != MB_BLUR_NONE)
		{
			CMBlur::MotionBlurRender(m_pRwCamera, m_BlurRed, m_BlurGreen, m_BlurBlue, m_motionBlur, m_BlurType, m_imotionBlurAddAlpha);

		}
	}
}


///////////////////////////////////////////////////////////////////////////
// NAME       : Restore()
// PURPOSE    : Returns the camera to its original stuff. Used for scripting
// RETURNS    : Nowt
// PARAMETERS : Nada
///////////////////////////////////////////////////////////////////////////
void CCamera::Restore(void)
{

	m_bLookingAtPlayer=true;
	m_bLookingAtVector=false;
	m_iTypeOfSwitch=TRANS_INTERPOLATION;
	//rightyho time to clear all the new created floats for the camera spline
	m_bUseNearClipScript=false;
	m_iModeObbeCamIsInForCar=NOT_IN_OBBE_MODE_YET;
	m_fPositionAlongSpline=0;
	m_bStartingSpline=FALSE;
	m_bScriptParametersSetForInterPol=false;

	WhoIsInControlOfTheCamera=NO_ONE;
	if (FindPlayerVehicle() == NULL)//player is in sometime of vehicle	
	{
		m_iModeToGoTo=CCam::MODE_FOLLOWPED;
		TIDYREF_NOTINWORLD(pTargetEntity, &pTargetEntity);
		pTargetEntity=(CEntity*)CWorld::Players[CWorld::PlayerInFocus].pPed;
		REGREF(pTargetEntity, &pTargetEntity);
	}
	else	//but if they are entering a car or in a car set the target entity to the car
	{
		m_iModeToGoTo=CCam::MODE_CAM_ON_A_STRING;
		TIDYREF(pTargetEntity, &pTargetEntity);
		pTargetEntity=(CEntity*) FindPlayerVehicle();
		REGREF(pTargetEntity, &pTargetEntity);
	}
	//this is required to stop camera jumping when getting into the car
	if 	((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_ENTER_CAR)
	{
		m_iModeToGoTo=CCam::MODE_CAM_ON_A_STRING;
		//TIDYREF(pTargetEntity, &pTargetEntity);
		//pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed->m_pTargetEntity;
		//REGREF(pTargetEntity, &pTargetEntity);
	}
	
	if 	(((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_CARJACK) || ((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_OPEN_DOOR))
	{
		m_iModeToGoTo=CCam::MODE_CAM_ON_A_STRING;
		//TIDYREF(pTargetEntity, &pTargetEntity);
		//pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed->m_pTargetEntity;
		//REGREF(pTargetEntity, &pTargetEntity);
	}


    if 	((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_EXIT_CAR)
	{
		m_iModeToGoTo=CCam::MODE_FOLLOWPED;
		TIDYREF(pTargetEntity, &pTargetEntity);
		pTargetEntity=(CEntity*)CWorld::Players[CWorld::PlayerInFocus].pPed;
		REGREF(pTargetEntity, &pTargetEntity);
	}
	
	
	if (pAttachedEntity != NULL)
	{
		TIDYREF_NOTINWORLD(pAttachedEntity, &pAttachedEntity);
		pAttachedEntity = NULL;
	}
	
	m_bEnable1rstPersonCamCntrlsScript=false; 
	m_bAllow1rstPersonWeaponsCamera=false;

	m_bUseScriptZoomValuePed=false;
	m_bUseScriptZoomValueCar=false;
	m_bStartInterScript=TRUE;
	m_bCameraJustRestored=true;
	m_fAvoidTheGeometryProbsTimer=0.0;
}


void CCamera::RestoreWithJumpCut(void)
{
	m_bRestoreByJumpCut=true;
	m_bLookingAtPlayer=true;
	m_bLookingAtVector=false;
	m_iTypeOfSwitch=TRANS_JUMP_CUT;
	WhoIsInControlOfTheCamera=NO_ONE;
	m_fPositionAlongSpline=0;
	m_bStartingSpline=FALSE;
	m_bUseNearClipScript=false;
	m_iModeObbeCamIsInForCar=NOT_IN_OBBE_MODE_YET;
	m_bScriptParametersSetForInterPol=false;
	m_bCameraJustRestored=true;

	m_bEnable1rstPersonCamCntrlsScript=false; 
	m_bAllow1rstPersonWeaponsCamera=false;

	
	
	if (FindPlayerVehicle() == NULL)//player is in sometime of vehicle	
	{
		m_iModeToGoTo=CCam::MODE_FOLLOWPED;
		TIDYREF(pTargetEntity, &pTargetEntity);
		pTargetEntity=(CEntity*)CWorld::Players[CWorld::PlayerInFocus].pPed;
		REGREF(pTargetEntity, &pTargetEntity);
	}
	else	//but if they are entering a car or in a car set the target entity to the car
	{
		m_iModeToGoTo=CCam::MODE_CAM_ON_A_STRING;
		TIDYREF(pTargetEntity, &pTargetEntity);
		pTargetEntity=(CEntity*) FindPlayerVehicle();
		REGREF(pTargetEntity, &pTargetEntity);
	}
	//this is required to stop camera jumping when getting into the car
	if 	((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_ENTER_CAR)
	{
		m_iModeToGoTo=CCam::MODE_CAM_ON_A_STRING;
		//TIDYREF(pTargetEntity, &pTargetEntity);
		//pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed->m_pTargetEntity;
		//REGREF(pTargetEntity, &pTargetEntity);
	}

	if 	(((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_CARJACK) || ((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_OPEN_DOOR))
	{
		m_iModeToGoTo=CCam::MODE_CAM_ON_A_STRING;
		//TIDYREF(pTargetEntity, &pTargetEntity);
		//pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed->m_pTargetEntity;
		//REGREF(pTargetEntity, &pTargetEntity);
	}


	if 	((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()) == PED_EXIT_CAR)
	{
		m_iModeToGoTo=CCam::MODE_FOLLOWPED;
		TIDYREF(pTargetEntity, &pTargetEntity);
		pTargetEntity=(CEntity*)CWorld::Players[CWorld::PlayerInFocus].pPed;
		REGREF(pTargetEntity, &pTargetEntity);
	}

	if (m_bCooperativeCamMode && CWorld::Players[0].pPed && CWorld::Players[1].pPed)
	{
		TIDYREF(pTargetEntity, &pTargetEntity);

		if (CWorld::Players[0].pPed->m_nPedFlags.bInVehicle && CWorld::Players[1].pPed->m_nPedFlags.bInVehicle &&
			CWorld::Players[0].pPed->m_pMyVehicle && CWorld::Players[1].pPed->m_pMyVehicle)
		{		// Both players are in a car.
			if (CWorld::Players[0].pPed->m_pMyVehicle == CWorld::Players[1].pPed->m_pMyVehicle)
			{			// Both players are in the same car
				if (m_bAllowShootingWith2PlayersInCar)
				{
					m_iModeToGoTo = m_ModeForTwoPlayersSameCarShootingAllowed;
					pTargetEntity = CWorld::Players[0].pPed->m_pMyVehicle;
				}
				else
				{
					m_iModeToGoTo = m_ModeForTwoPlayersSameCarShootingNotAllowed;
					pTargetEntity = CWorld::Players[0].pPed->m_pMyVehicle;
				}
			}
			else
			{			// Players are each in their own car.
				m_iModeToGoTo = m_ModeForTwoPlayersSeparateCars;
				pTargetEntity = CWorld::Players[0].pPed->m_pMyVehicle;
			}
		}
		else
		{
			m_iModeToGoTo = m_ModeForTwoPlayersNotBothInCar;
			pTargetEntity = CWorld::Players[0].pPed;
		}

		REGREF(pTargetEntity, &pTargetEntity);
	}
	
	m_bUseScriptZoomValuePed=false;
	m_bUseScriptZoomValueCar=false;
}

/////////////////////////////////////////
void CCamera::SetCamCutSceneOffSet(const CVector &CamCutSceneOffset)
{
	m_vecCutSceneOffset=CamCutSceneOffset;
}	

///////////////////////////////////////////////////////////
void CCamera::SetCameraDirectlyBehindForFollowPed_CamOnAString(void)
{
	m_bCamDirectlyBehind = true;
//	m_bRestoreByJumpCut = true;
	
	CPlayerPed* ThePlayer=NULL; 
	ThePlayer=FindPlayerPed();
	if (ThePlayer)
	{
		m_PedOrientForBehindOrInFront=CGeneral::GetATanOfXY(ThePlayer->GetMatrix().xy, ThePlayer->GetMatrix().yy);
	}
}
///////////////////////////////////////////////////////
void CCamera::SetCameraDirectlyInFrontForFollowPed_CamOnAString(void)
{
	m_bCamDirectlyInFront=true;	
	CPlayerPed* ThePlayer=NULL; 
	ThePlayer=FindPlayerPed();
	if (ThePlayer)
	{
		m_PedOrientForBehindOrInFront=CGeneral::GetATanOfXY(ThePlayer->GetMatrix().xy, ThePlayer->GetMatrix().yy);
	}

}

///////////////////////////////////////////////////////////
void CCamera::SetCameraDirectlyBehindForFollowPed_ForAPed_CamOnAString(CPed *pPed)
{
	if (!pPed)
		return;
	m_bCamDirectlyBehind = true;
	m_bLookingAtPlayer = false; // fuck knows what this does, but it might just work...
	TheCamera.pTargetEntity = pPed;
	TIDYREF(TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity,&TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity);	
	TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity = pPed;
	REGREF(TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity,&TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity);
//	m_bRestoreByJumpCut = true;
		m_PedOrientForBehindOrInFront=CGeneral::GetATanOfXY(pPed->GetMatrix().xy, pPed->GetMatrix().yy);
}

///////////////////////////////////////////////////////
void CCamera::SetCameraDirectlyInFrontForFollowPed_ForAPed_CamOnAString(CPed *pPed)
{
	if (!pPed)
		return;

	m_bLookingAtPlayer = false; // fuck knows what this does, but it might just work...
	TheCamera.pTargetEntity = pPed;
	TIDYREF(TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity,&TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity);	
	TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity = pPed;
	REGREF(TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity,&TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity);
		
	m_bCamDirectlyInFront=true;	
		m_PedOrientForBehindOrInFront=CGeneral::GetATanOfXY(pPed->GetMatrix().xy, pPed->GetMatrix().yy);
}

////////////////////////////////////////
void CCamera::SetCamPositionForFixedMode(const CVector &CamPosToGoTo, const  CVector&UpOffsets)
{
  	m_vecFixedModeSource=CamPosToGoTo;
  	m_vecFixedModeUpOffSet=UpOffsets;
  	// need to clear garage cam set flag, otherwise if we were in a garage and go back there, cam won't get set
  	m_bGarageFixedCamPositionSet=false;
}	

//////////////////////////////////////////////////////
void CCamera::SetFadeColour(UInt8 RedFade,UInt8 GreenFade, UInt8 BlueFade)
{
	m_FadeTargetIsSplashScreen=false;
	if ((RedFade==2) && (GreenFade==2) && (BlueFade==2))
	{
		m_FadeTargetIsSplashScreen=true;
	}

	CDraw::FadeRed=RedFade;
	CDraw::FadeGreen=GreenFade;
	CDraw::FadeBlue=BlueFade;
}

void CCamera::SetMotionBlur(int Red, int Green, int Blue,int blur, int Type)
{ 
	//ASSERTMSG(((blur>=0)&&(blur<=255)), "Only set mblur within these ranges");
	m_BlurRed = Red;	
	m_BlurGreen = Green;	
	m_BlurBlue = Blue;	
	m_BlurType = Type;	
	m_motionBlur = blur;
//	if (!postFx.TuneColor)
//		postFx.SetColour((RwUInt8) m_BlurRed, (RwUInt8) m_BlurGreen, (RwUInt8) m_BlurBlue, 1);
}

void CCamera::SetMotionBlurAlpha(int blur)
{ 
	m_imotionBlurAddAlpha = blur;
}



void CCamera::SetNearClipScript(float NearClipValue)
{
	m_fNearClipScript=NearClipValue;
	m_bUseNearClipScript=true;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : SetNewPlayerWeaponMode()
// PURPOSE    : Selects a new camera for the player to override the standard
//				one. This is to be used to sort out tricky bits like tunnels.
//				Also the way to trigger sniping mode etc
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCamera::SetNewPlayerWeaponMode(Int16 Mode, Int16 MinZoom, Int16 MaxZoom)
{
//	this->SetMotionBlur(CTimeCycle::m_fCurrentBlurRed,CTimeCycle::m_fCurrentBlurGreen,CTimeCycle::m_fCurrentBlurBlue, this->GetMotionBlur(), MB_BLUR_LIGHT_SCENE); 
	PlayerWeaponMode.Mode = Mode;
	PlayerWeaponMode.MaxZoom = MaxZoom;
	PlayerWeaponMode.MinZoom = MinZoom;
	PlayerWeaponMode.Duration = 0.0f;		// Not needed really
}







/////////////////////////////////////////////////////////////////
// NAME       : Using1stPersonWeaponCam
// PURPOSE    : Is camera currently in a 1st person weapon targeting
//				mode (eg sniper or M16 targeting)
/////////////////////////////////////////////////////////////////

bool CCamera::Using1stPersonWeaponMode(void)
{
	if( PlayerWeaponMode.Mode==CCam::MODE_SNIPER 
	|| PlayerWeaponMode.Mode==CCam::MODE_M16_1STPERSON
	|| PlayerWeaponMode.Mode==CCam::MODE_ROCKETLAUNCHER 
	|| PlayerWeaponMode.Mode==CCam::MODE_ROCKETLAUNCHER_HS
	|| PlayerWeaponMode.Mode==CCam::MODE_HELICANNON_1STPERSON 
	|| PlayerWeaponMode.Mode==CCam::MODE_CAMERA
	|| PlayerWeaponMode.Mode==CCam::MODE_AIMWEAPON_ATTACHED)
		return true;
		
	return false;
}



///////////////////////////////////////////////
void CCamera::SetParametersForScriptInterpolation (float PercentageInterToStopMoving, float PercentageInterToCatchUp, UInt32 TimeForInterPolation)
{
	ASSERTMSG(m_bLookingAtPlayer==false, "Take Control of camera first");
	ASSERTMSG(PercentageInterToStopMoving + PercentageInterToCatchUp==100.0f, "Interpolation Fraction not eq 100 percent");
	m_fScriptPercentageInterToStopMoving=PercentageInterToStopMoving/100.0f;
	m_fScriptPercentageInterToCatchUp=PercentageInterToCatchUp/100.0f;
	m_fScriptTimeForInterPolation=TimeForInterPolation;
	m_bScriptParametersSetForInterPol=true;
}



///////////////////////////////////////////////
void CCamera::SetPercentAlongCutScene(float Percentage)
{
	if (Cams[ActiveCam].Mode==CCam::MODE_FLYBY)
	{ 
		Cams[ActiveCam].m_fTimeElapsedFloat=((Percentage/100)*Cams[ActiveCam].m_uiFinishTime);
	}
	else if (Cams[(ActiveCam + 1) % 2].Mode==CCam::MODE_FLYBY)
	{
		Cams[(ActiveCam + 1) % 2].m_fTimeElapsedFloat=((Percentage/100)*Cams[(ActiveCam + 1) % 2].m_uiFinishTime);
	}
}


///////////////////////////////////////////////////////////////////////////
// NAME       : SetRwCamera()
// PURPOSE    : Setup RW camera. 
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
void CCamera::SetRwCamera(RwCamera* pRwCamera)
{
	ASSERTMSG(pRwCamera!=NULL, "you haven't created the rwcamera");
	m_pRwCamera = pRwCamera;
	m_viewMatrix.Attach(RwCameraGetViewMatrix(m_pRwCamera));
// Even the BlurOn = false, it still need to open it for making a color on top of it without any motion blur
	CMBlur::MotionBlurOpen(m_pRwCamera);  
/*	
	if (CMBlur::BlurOn)
	{
		CMBlur::MotionBlurOpen(m_pRwCamera);
	}else
	{
		CMBlur::MotionBlurClose();
	}
*/	
}



///////////////////////////////////////////////////////////////////////////
// NAME       : SetWideScreenOn(void)
// PURPOSE    : Duh
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCamera::SetWideScreenOn(void)
{
	m_WideScreenOn=true;
	m_bWantsToSwitchWidescreenOff=false;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : SetWideScreenOff(void)
// PURPOSE    : Double Duh
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////


void CCamera::SetWideScreenOff(void)
{
	if (m_WideScreenOn==true) m_bWantsToSwitchWidescreenOff=true;
	else m_bWantsToSwitchWidescreenOff=false;
}


void CCamera::SetZoomValueFollowPedScript(Int16 ZoomVal)
{
	ASSERTMSG(m_bLookingAtPlayer==false, "Set the zoom val AFTER Taking Control of Cam");

	if (ZoomVal==SCRIPT_ZOOM_ONE)
	{
		m_fPedZoomValueScript=ZOOM_PED_ONE_DISTANCE;
	}
	else if (ZoomVal==SCRIPT_ZOOM_TWO) 
	{
		m_fPedZoomValueScript=ZOOM_PED_TWO_DISTANCE;
	}
	else if (ZoomVal==SCRIPT_ZOOM_THREE)
	{
		m_fPedZoomValueScript=ZOOM_PED_THREE_DISTANCE;	
		//don't bother changing anything if supplied wrong value
	}
	else
	{
		m_fPedZoomValueScript=m_fPedZoomValueScript;
	}
	m_bUseScriptZoomValuePed=true;
}

void CCamera::SetZoomValueCamStringScript(Int16 ZoomVal)
{
	ASSERTMSG(m_bLookingAtPlayer==false, "Set the zoom val AFTER Taking Control of Cam");
	// better check this is a car, if not call ped version
	if(!Cams[0].CamTargetEntity->GetIsTypeVehicle())
	{
		SetZoomValueFollowPedScript(ZoomVal);
		return;
	}

	eVehicleAppearance VehicleApperance=((CVehicle*)(Cams[0].CamTargetEntity))->GetVehicleAppearance(); 
	int PositionInArray=0;
	GetArrPosForVehicleType(VehicleApperance, PositionInArray);



	if (ZoomVal==SCRIPT_ZOOM_ONE) m_fCarZoomValueScript=ZOOM_ONE_DISTANCE[PositionInArray];
	else if (ZoomVal==SCRIPT_ZOOM_TWO) m_fCarZoomValueScript=ZOOM_TWO_DISTANCE[PositionInArray];
	else if (ZoomVal==SCRIPT_ZOOM_THREE) m_fCarZoomValueScript=ZOOM_THREE_DISTANCE[PositionInArray];
//don't bother changing anything if supplied wrong value
	else
	m_fCarZoomValueScript=m_fCarZoomValueScript;
	
	m_bUseScriptZoomValueCar=true;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : StartTransition()
// PURPOSE    : Starts a transition to a new camera mode.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCamera::StartTransition(Int16 Mode) 
{
	// save original mode so we can do switch rates later
	Int16 nOldMode = Cams[ActiveCam].Mode;

	m_bItsOkToLookJustAtThePlayer=false;
	m_bUseTransitionBeta=FALSE;
	
	// default numbers
	m_fFractionInterToStopMoving=0.25f; //these dudes must add up to one 
	m_fFractionInterToStopCatchUp=0.75f;
	
	// first person modes set playerped's heading when they're done
	if (Cams[ActiveCam].Mode== CCam::MODE_SNIPER || Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER || Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER_HS
	|| Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON ||(Cams[ActiveCam].Mode==CCam::MODE_SNIPER_RUNABOUT)
	||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS)||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON_RUNABOUT)
	||(Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_HELICANNON_1STPERSON)
	||(Cams[ActiveCam].Mode==CCam::MODE_CAMERA)||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON_RUNABOUT))
	{
		if (pTargetEntity->GetIsTypePed())
		{
			float CamDirection;
			CamDirection=CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y)-(PI/2);
			((CPed*)pTargetEntity)->m_fCurrentHeading=CamDirection;
			((CPed*)pTargetEntity)->m_fDesiredHeading=CamDirection;
		}
	}

	Cams[ActiveCam].m_cvecCamFixedModeVector=m_vecFixedModeVector;
	TIDYREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
	Cams[ActiveCam].CamTargetEntity=pTargetEntity;
	REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
	Cams[ActiveCam].m_cvecCamFixedModeSource=m_vecFixedModeSource;
  	Cams[ActiveCam].m_cvecCamFixedModeUpOffSet=m_vecFixedModeUpOffSet;
	Cams[ActiveCam].m_bCamLookingAtVector=m_bLookingAtVector;

	// stuff we want to do to the old weapon mode
	switch (Mode)
	{
		case CCam::MODE_SNIPER:
		case CCam::MODE_ROCKETLAUNCHER:
		case CCam::MODE_ROCKETLAUNCHER_HS:
		case CCam::MODE_M16_1STPERSON:
		case CCam::MODE_HELICANNON_1STPERSON:
		case CCam::MODE_SNIPER_RUNABOUT:
		case CCam::MODE_ROCKETLAUNCHER_RUNABOUT:
		case CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS:
		case CCam::MODE_1STPERSON_RUNABOUT:
		case CCam::MODE_M16_1STPERSON_RUNABOUT:
		case CCam::MODE_FIGHT_CAM_RUNABOUT: 
 		case CCam::MODE_CAMERA:
			// Take orientation from player
			if (FindPlayerVehicle() != NULL)
				Cams[ActiveCam].Beta = CMaths::ATan2(FindPlayerVehicle()->GetMatrix().xy, FindPlayerVehicle()->GetMatrix().yy);
			else
				Cams[ActiveCam].Beta = CMaths::ATan2(CWorld::Players[CWorld::PlayerInFocus].pPed->GetMatrix().xy, CWorld::Players[CWorld::PlayerInFocus].pPed->GetMatrix().yy);

			Cams[ActiveCam].Alpha = 0.0f;
			break;
			
/* DWREOMVED		case CCam::MODE_SYPHON:
			Cams[ActiveCam].Alpha=0.0f;	
			Cams[ActiveCam].AlphaSpeed=0.0f;
		break;	*/
/*	DWREMOVED	case CCam::MODE_FIGHT_CAM:
			Cams[ActiveCam].Beta=0.0f;	
			Cams[ActiveCam].BetaSpeed=0.0f;	
			Cams[ActiveCam].Alpha=0.0f;	
			Cams[ActiveCam].AlphaSpeed=0.0f;
		break;	*/
		case CCam::MODE_FOLLOWPED:
			{
	// SR_TEST_SWITCH
				if(Cams[ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING)
				{
	//				m_fPedZoomSmoothed = m_fCarZoomSmoothed;
	//				Cams[ActiveCam].Mode = Mode;
	//				return;
				}
	// SR_TEST_SWITCH
				float CamOffsetForHack=DEGTORAD(55.0f);
				float CamOffsetHackNormalDoorBlocked=DEGTORAD(95.0f);
				ASSERT(pTargetEntity->GetIsTypePed());

				if (m_bJustCameOutOfGarage)
				{
	//			     m_bUseTransitionBeta=true;
					Cams[ActiveCam].Beta = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x,Cams[ActiveCam].Front.y) + PI;
				     
				     
	//				 #ifdef GTA_PC
	//				 if (FrontEndMenuManager.m_ControlMethod == CLASSIC_CONTROLLER_SCREEN)
	//			     {
	//			     	Cams[ActiveCam].m_fTransitionBeta=PI+CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x,Cams[ActiveCam].Front.y);
	//				 }
	//				 else
	//				  #endif
					{
					
	/*				 	if (Cams[ActiveCam].Front.x!=0.0f && Cams[ActiveCam].Front.y!=0.0f)
			     		{
			     			Cams[ActiveCam].m_fTransitionBeta=PI+CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x,Cams[ActiveCam].Front.y);
				 		}
				 		else
				 		{*/
							Cams[ActiveCam].m_fTransitionBeta=0.0f;
	//				 	}
					}	
				}
				if (m_bTargetJustCameOffTrain)
				{
					m_bCamDirectlyInFront=true;
				}
			
				if (Cams[ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING)
				{
					m_bUseTransitionBeta=true;
					bool MatrixWouldCauseCrash=false;
					/*
					if (((CPed*)pTargetEntity)->m_pObjectiveVehicle!=NULL)
					{
						if (((CPed*)pTargetEntity)->m_pObjectiveVehicle->GetMatrix().xy == 0.0f && 
							((CPed*)pTargetEntity)->m_pObjectiveVehicle->GetMatrix().yy == 0.0f)
						{
							MatrixWouldCauseCrash=true;
						}
					}
					*/
					// Calculate the beta we would get right behind the ped.
					if (MatrixWouldCauseCrash)
					{
						Cams[ActiveCam].m_fTransitionBeta = 0.0f;
					}
					else
					{
						//this is a hack, I am very very ashamed of myself, and my code. 
						bool infront=false;
						float TargetOrientation=0.0f;
						float TempBeta=0.0f;
						TempBeta=CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x,Cams[ActiveCam].Front.y);

						//if (((CPed*)pTargetEntity)->m_pObjectiveVehicle==NULL)
						{
							TargetOrientation=TempBeta;
						}
						/*
						else
						{
							TargetOrientation=CGeneral::GetATanOfXY(((CPed*)pTargetEntity)->m_pObjectiveVehicle->GetMatrix().xy, ((CPed*)pTargetEntity)->m_pObjectiveVehicle->GetMatrix().yy);//this is the angle the car is facing
						}
						*/
						
						
						float CamDiff= TargetOrientation - TempBeta;
						while (CamDiff > PI) CamDiff -= (2.0f*PI);
						while (CamDiff < -PI) CamDiff += (2.0f*PI);
							
						//Need to work out if we need to switch 	
						if (CMaths::Abs(CamDiff)>(DEGTORAD(90)))///AcceptableRange)&&(GoingForward))
						{
							infront=true;
						}        	
						else
						{
							infront=false;
						}
						
						
						//int exitdoor=((CPed*)pTargetEntity)->m_nDoor;
										
						if (infront)
						{
							/*
							if (((CPed*)pTargetEntity)->m_pObjectiveVehicle!=NULL)
							{
								if (((CPed*)pTargetEntity)->m_pObjectiveVehicle->IsUpsideDown())//left is right, right is left
								{
									if ((exitdoor==CAR_DOOR_LF)||(exitdoor==CAR_DOOR_LR))//things are reversed
									{
										CamOffsetForHack=-CamOffsetHackNormalDoorBlocked;
									}
								}
								else
								{
									if ((exitdoor==CAR_DOOR_RF)||(exitdoor==CAR_DOOR_RR))//things are reversed
									{
										CamOffsetForHack=-CamOffsetHackNormalDoorBlocked;
									}
								}
							}
							*/
							Cams[ActiveCam].m_fTransitionBeta = TargetOrientation+CamOffsetForHack;
						}
						else
						{
							/*
							if (((CPed*)pTargetEntity)->m_pObjectiveVehicle!=NULL)
							{
								if (((CPed*)pTargetEntity)->m_pObjectiveVehicle->IsUpsideDown())//left is right, right is left
								{
									if ((exitdoor==CAR_DOOR_RF)||(exitdoor==CAR_DOOR_RR))
									{
										CamOffsetForHack=-CamOffsetForHack;
									}
									else if ((exitdoor==CAR_DOOR_LF)||(exitdoor==CAR_DOOR_LR))
									{
										CamOffsetForHack=CamOffsetHackNormalDoorBlocked;
									}
								}
								else
								{
									if ((exitdoor==CAR_DOOR_LF)||(exitdoor==CAR_DOOR_LR))
									{
										CamOffsetForHack=-CamOffsetForHack;
									}
									else if ((exitdoor==CAR_DOOR_RF)||(exitdoor==CAR_DOOR_RR))
									{
										CamOffsetForHack=CamOffsetHackNormalDoorBlocked;
									}
								}					
							}
							*/

							Cams[ActiveCam].m_fTransitionBeta = TargetOrientation+PI+CamOffsetForHack;
						}
					}
				}
			}
			break;
	case CCam::MODE_BEHINDCAR:
		Cams[ActiveCam].BetaSpeed=0.0f;
		break;	
	case CCam::MODE_BEHINDBOAT:
		Cams[ActiveCam].BetaSpeed=0.0f;
		break;	
	case CCam::MODE_CAM_ON_A_STRING:
		{
	// SR_TEST_SWITCH
	//		if(Cams[ActiveCam].Mode==CCam::MODE_FOLLOWPED)
	//		{
	//			m_fCarZoomSmoothed = m_fPedZoomSmoothed;
	//			Cams[ActiveCam].Mode = Mode;
	//			return;
	//		}
	// SR_TEST_SWITCH
			float CamOffsetForCarHack=DEGTORAD(57.0f);
			float CamOffsetHackCarNormalDoorBlocked=DEGTORAD(57.0f);
			ASSERTMSG(pTargetEntity->GetIsTypeVehicle(), "oops something gone wrong");

			///Mark delete this when they decide that they don't like the get into car view
			//this is a hack, I am very very ashamed of myself, and my code. 
			if ((m_bLookingAtPlayer==true)&&(m_bJustCameOutOfGarage==false))
			{
				m_bUseTransitionBeta=true;
						
				//Need to work out if we need to switch 	
				bool FixedModeSpecialCaseActive=false;
				//need to do special case when we are going from a fixed cam
				//the player has not entered a door so lets just 
				//go to the angle we are currently at 
				if (Cams[ActiveCam].Mode==CCam::MODE_FIXED)
				{
					FixedModeSpecialCaseActive=true;
				}
				
				//int EnterDoor=(FindPlayerPed())->m_nDoor;
				float CurrentCamBeta=CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y);
				if (FixedModeSpecialCaseActive)				
				{
					Cams[ActiveCam].m_fTransitionBeta = CurrentCamBeta;
				}
				else
				{
					Cams[ActiveCam].m_fTransitionBeta = CurrentCamBeta;
					//we want to go to the closest angle to the car plus offset.
					//4 possible options depending on whether the camera is in front of the car or
					//behind the car
		/*			float TempClosestValue=0.0f;
					float TempDiff=0.0f;
					float CarTargetOrientation=CGeneral::GetATanOfXY(pTargetEntity->GetMatrix().xy, pTargetEntity->GetMatrix().yy);//this is the angle the car is facing
					//do first possiblity
					TempClosestValue=CarTargetOrientation + CamOffsetForCarHack;
					//set the transitionbeta equal to this as it is the first possiblity
					Cams[ActiveCam].m_fTransitionBeta=TempClosestValue;
					
					//now try the second possiblitly
					TempClosestValue=CarTargetOrientation - CamOffsetForCarHack;
					TempDiff=TempClosestValue-CurrentCamBeta;
					MakeAngleLessThan180(TempDiff);
					//now compare what we have for the m_fTransitionBeta with what we could have
					//if it is smaller then chose the smaller of the two.
					float CurrentCamDiff=Cams[ActiveCam].m_fTransitionBeta - CurrentCamBeta;
					MakeAngleLessThan180(CurrentCamDiff);
					if ((ABS(TempDiff))<(ABS(CurrentCamDiff)))
					{
						Cams[ActiveCam].m_fTransitionBeta=TempClosestValue;
					}
					
					//now try the third possibility
					TempClosestValue=CarTargetOrientation + CamOffsetForCarHack + PI;
					TempDiff=TempClosestValue-CurrentCamBeta;
					MakeAngleLessThan180(TempDiff);
					CurrentCamDiff=Cams[ActiveCam].m_fTransitionBeta - CurrentCamBeta;
					MakeAngleLessThan180(CurrentCamDiff);

					//now compare what we have for the m_fTransitionBeta with what we could have
					//if it is smaller then chose the smaller of the two.
					if ((ABS(TempDiff))<(ABS(CurrentCamDiff)))
					{
						Cams[ActiveCam].m_fTransitionBeta=TempClosestValue;
					}
					
					//and finaly the forth 
					TempClosestValue=CarTargetOrientation - CamOffsetForCarHack + PI;
					TempDiff=TempClosestValue-CurrentCamBeta;
					MakeAngleLessThan180(TempDiff);
					CurrentCamDiff=Cams[ActiveCam].m_fTransitionBeta - CurrentCamBeta;
					MakeAngleLessThan180(CurrentCamDiff);
					//now compare what we have for the m_fTransitionBeta with what we could have
					//if it is smaller then chose the smaller of the two.
					if ((ABS(TempDiff))<(ABS(CurrentCamDiff)))
					{
						Cams[ActiveCam].m_fTransitionBeta=TempClosestValue;
					}
	*/
				}
			}
		}
	break;	
	case CCam::MODE_PED_DEAD_BABY:
		Cams[ActiveCam].Alpha=DEGTORAD(15.0f);//this is the starting alphaphapha
	break;
	}
	
	// Initialise the new camera
	// There are some value we might want to carry across to the new camera
	float fTempBeta = Cams[ActiveCam].Beta;
	if((Cams[ActiveCam].Mode==CCam::MODE_FOLLOWPED && Mode==CCam::MODE_CAM_ON_A_STRING)
	|| (Cams[ActiveCam].Mode==CCam::MODE_CAM_ON_A_STRING && Mode==CCam::MODE_FOLLOWPED))
	{
		Cams[ActiveCam].Mode = Mode;
	}
	else
	{
		Cams[ActiveCam].Init();
		Cams[ActiveCam].Mode = Mode;
		Cams[ActiveCam].Beta = fTempBeta;
	}
	
	m_uiTransitionDuration = 1350;
	
	bool bGoingFromSyphonToPed = false;	
	if(Mode==CCam::MODE_SYPHON && nOldMode==CCam::MODE_SYPHON_CRIM_IN_FRONT)
	{
		m_uiTransitionDuration = 1800;//2000;	// in milliseconds
	}
/*DWREMOVED	else if(Mode==CCam::MODE_FOLLOWPED && nOldMode==CCam::MODE_FIGHT_CAM)
	{
		//these dudes must add up to one
		//m_fFractionInterToStopMoving=0.0f;
		//m_fFractionInterToStopCatchUp=1.0f;
		//static uint32 TranDur=300;
		m_uiTransitionDuration = 750;//2000;	// in milliseconds
	} */
	else if(Mode==CCam::MODE_CAM_ON_A_STRING
	&& (nOldMode==CCam::MODE_SYPHON_CRIM_IN_FRONT || nOldMode==CCam::MODE_FOLLOWPED
	|| nOldMode==CCam::MODE_SYPHON || nOldMode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON
	|| nOldMode==CCam::MODE_AIMWEAPON))
	{
		//these dudes must add up to one
		m_fFractionInterToStopMoving=0.10f;
		m_fFractionInterToStopCatchUp=0.90f;
		m_uiTransitionDuration = 750;
	}
	else if(nOldMode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)
	{
		static float fTestFixedSyphonPedMoving = 0.2;
		static float fTestFixedSyphonPedCatchUp = 0.8f;
		static uint32 fTestFixedSyphonPedDuration = 1000;
		//these dudes must add up to one 
		m_fFractionInterToStopMoving = fTestFixedSyphonPedMoving;//0.00f;
		m_fFractionInterToStopCatchUp = fTestFixedSyphonPedCatchUp;//1.00f;
		m_uiTransitionDuration = fTestFixedSyphonPedDuration;//600;
	}
	else if(nOldMode==CCam::MODE_FIXED)
	{
		//these dudes must add up to one
		m_fFractionInterToStopMoving=0.05f;
		m_fFractionInterToStopCatchUp=0.95f;
		//m_uiTransitionDuration = 1000;
	}
	else if(m_bPlayerWasOnBike && Mode==CCam::MODE_FOLLOWPED && nOldMode==CCam::MODE_CAM_ON_A_STRING)
	{
		m_uiTransitionDuration = 800;
		m_fFractionInterToStopMoving = 0.02f;//0.05f; //these dudes must add up to one 
		m_fFractionInterToStopCatchUp = 0.98f;//0.95f;
	}
	else if((Mode==CCam::MODE_CAM_ON_A_STRING || Mode==CCam::MODE_BEHINDBOAT)
	&& (nOldMode==CCam::MODE_SNIPER_RUNABOUT || nOldMode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT
	|| nOldMode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS || nOldMode==CCam::MODE_1STPERSON_RUNABOUT
	|| nOldMode==CCam::MODE_M16_1STPERSON_RUNABOUT || nOldMode==CCam::MODE_FIGHT_CAM_RUNABOUT
	|| nOldMode==CCam::MODE_CAMERA))
	{
		//these dudes must add up to one 
		m_fFractionInterToStopMoving=0.00f;
		m_fFractionInterToStopCatchUp=1.0f;
		m_uiTransitionDuration = 1;
	}
	else if(Mode==CCam::MODE_AIMWEAPON)
	{
		static float fTestSyphonPedMoving = 0.0;
		static float fTestSyphonPedCatchUp = 1.0f;
		static uint32 fTestSyphonPedDuration = 400;
		//these dudes must add up to one 
		m_fFractionInterToStopMoving = fTestSyphonPedMoving;//0.5f;
		m_fFractionInterToStopCatchUp = fTestSyphonPedCatchUp;//0.5f;
		m_uiTransitionDuration = fTestSyphonPedDuration;//350;
		
		bGoingFromSyphonToPed = true;
	}
	else if((Mode==CCam::MODE_FOLLOWPED || Mode==CCam::MODE_AIMWEAPON
	|| Mode==CCam::MODE_SYPHON_CRIM_IN_FRONT || Mode==CCam::MODE_SYPHON
	|| Mode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)
	&& (nOldMode==CCam::MODE_SYPHON_CRIM_IN_FRONT || nOldMode==CCam::MODE_FOLLOWPED
	|| nOldMode==CCam::MODE_SYPHON || nOldMode==CCam::MODE_SPECIAL_FIXED_FOR_SYPHON
	|| nOldMode==CCam::MODE_AIMWEAPON))
	{
		//these dudes must add up to one 
		m_fFractionInterToStopMoving = 0.1f;
		m_fFractionInterToStopCatchUp = 0.9f;
		m_uiTransitionDuration = 350;

		bGoingFromSyphonToPed = true;
	}
	else 	// Transition will take a while
	{
		m_uiTransitionDuration = 1350;//1500;	// in milliseconds
	}	
	
	m_uiTransitionState = TRANS_INTERPOLATION;
	m_uiTimeTransitionStart = CTimer::GetTimeInMilliseconds();
	m_uiTransitionJUSTStarted = 1;	// Tell code we've just started with this transition
	
	if (m_vecDoingSpecialInterPolation)
	{
		m_cvecStartingSourceForInterPol=SourceDuringInter;
		m_cvecStartingTargetForInterPol=TargetDuringInter;
		m_cvecStartingUpForInterPol=UpDuringInter;
		m_fStartingAlphaForInterPol=m_fAlphaDuringInterPol;
		m_fStartingBetaForInterPol=m_fBetaDuringInterPol;
	}	
	else
	{
		
		m_cvecStartingSourceForInterPol=Cams[ActiveCam].Source;
		m_cvecStartingTargetForInterPol=Cams[ActiveCam].m_cvecTargetCoorsForFudgeInter;
		m_cvecStartingUpForInterPol=Cams[ActiveCam].Up;
		m_fStartingAlphaForInterPol=Cams[ActiveCam].m_fTrueAlpha;
		m_fStartingBetaForInterPol=Cams[ActiveCam].m_fTrueBeta;
	}
	
    /*
    char Str[200];
    sprintf(Str,"Start transition mode %i", Mode) ;
	OutputDebugString(Str);
	*/
	///copy over any variables needed
	Cams[ActiveCam].m_bCamLookingAtVector=m_bLookingAtVector;
	Cams[ActiveCam].m_cvecCamFixedModeVector=m_vecFixedModeVector;
	Cams[ActiveCam].m_cvecCamFixedModeSource=m_vecFixedModeSource;
	Cams[ActiveCam].m_cvecCamFixedModeUpOffSet=m_vecFixedModeUpOffSet;
	Cams[ActiveCam].Mode=Mode;
	TIDYREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
	Cams[ActiveCam].CamTargetEntity=pTargetEntity;
	REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
	m_uiTransitionState = TRANS_INTERPOLATION;
	m_uiTimeTransitionStart = CTimer::GetTimeInMilliseconds();
	m_uiTransitionJUSTStarted = 1;	// Tell code we've just started with this transition
	m_fStartingFOVForInterPol=Cams[ActiveCam].FOV;
 	m_cvecSourceSpeedAtStartInter=Cams[ActiveCam].m_cvecSourceSpeedOverOneFrame;
	m_cvecTargetSpeedAtStartInter=Cams[ActiveCam].m_cvecTargetSpeedOverOneFrame;
	m_cvecUpSpeedAtStartInter=Cams[ActiveCam].m_cvecUpOverOneFrame;
	m_fAlphaSpeedAtStartInter=Cams[ActiveCam].m_fAlphaSpeedOverOneFrame;
	m_fBetaSpeedAtStartInter=Cams[ActiveCam].m_fBetaSpeedOverOneFrame;
	m_fFOVSpeedAtStartInter=Cams[ActiveCam].m_fFovSpeedOverOneFrame;
//	Cams[ActiveCam].ResetStatics=true;	// this is done in cam init function above

	//defaults overwridden below if appropriate 
	if (m_bLookingAtPlayer==false)
	{
		if (m_bScriptParametersSetForInterPol==true)
		{
			m_fFractionInterToStopMoving=m_fScriptPercentageInterToStopMoving; 
			m_fFractionInterToStopCatchUp=m_fScriptPercentageInterToCatchUp;
			m_uiTransitionDuration=m_fScriptTimeForInterPolation;
		}
		m_uiTransitionDurationTargetCoors=m_uiTransitionDuration;
		m_fFractionInterToStopMovingTarget=m_fFractionInterToStopMoving; //these dudes must add up to one 
		m_fFractionInterToStopCatchUpTarget=m_fFractionInterToStopCatchUp;

	}	
	else //want to look at the target quickly
	{
		if (bGoingFromSyphonToPed)
		{
			m_uiTransitionDurationTargetCoors=350;
		}
		else
		{
			m_uiTransitionDurationTargetCoors=600;
		}
		m_fFractionInterToStopMovingTarget=0.0f;//0.1f; //these dudes must add up to one 
		m_fFractionInterToStopCatchUpTarget=1.0f;//0.9f;

	}
	ASSERT( m_fFractionInterToStopMoving+m_fFractionInterToStopCatchUp > 0.99f 
		&&  m_fFractionInterToStopMoving+m_fFractionInterToStopCatchUp < 1.01f );
	
	ASSERT( m_fFractionInterToStopMovingTarget+m_fFractionInterToStopCatchUpTarget > 0.99f 
		&&  m_fFractionInterToStopMovingTarget+m_fFractionInterToStopCatchUpTarget < 1.01f );
}

void CCamera::StartTransitionWhenNotFinishedInter(Int16 Mode)
{
/*
	m_vecOldSourceForInter.x = GetMatrix().tx;
	m_vecOldSourceForInter.y = GetMatrix().ty;
	m_vecOldSourceForInter.z = GetMatrix().tz;


			// LookAt vector is the vector to the target
	m_vecOldFrontForInter.x = GetMatrix().xy;
	m_vecOldFrontForInter.y = GetMatrix().yy;
	m_vecOldFrontForInter.z = GetMatrix().zy;
			// Normalise LookAt
			// Set up vector
	m_vecOldUpForInter.x = GetMatrix().xz;
	m_vecOldUpForInter.y = GetMatrix().yz;
	m_vecOldUpForInter.z = GetMatrix().zz;

	m_vecOldFOVForInter=CDraw::GetFOV();
*/	
	m_vecDoingSpecialInterPolation=true;
	
	
	StartTransition(Mode);
}

void CCamera::StartCooperativeCamMode(void)
{	
	m_bCooperativeCamMode=true;
	CGameLogic::n2PlayerPedInFocus = 2;
}

void CCamera::StopCooperativeCamMode(void)
{	
	m_bCooperativeCamMode=false;
	CGameLogic::n2PlayerPedInFocus = 2;
}

void CCamera::AllowShootingWith2PlayersInCar(bool bArg)
{	
	m_bAllowShootingWith2PlayersInCar = bArg;
}


void CCamera::StoreValuesDuringInterPol(CVector &TheSource, CVector &TheTarget, CVector &TheUp, float &TheFOV)
{
	SourceDuringInter=TheSource;
	TargetDuringInter=TheTarget;
	UpDuringInter=TheUp;
	FOVDuringInter=TheFOV;
	CVector VecForBetaAlpha=TheSource - TargetDuringInter; 
	float DistOnGround=CMaths::Sqrt(VecForBetaAlpha.x * VecForBetaAlpha.x + VecForBetaAlpha.y * VecForBetaAlpha.y);
	m_fBetaDuringInterPol=CGeneral::GetATanOfXY(VecForBetaAlpha.x, VecForBetaAlpha.y);				
	m_fAlphaDuringInterPol=CGeneral::GetATanOfXY(DistOnGround,  VecForBetaAlpha.z);	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : UpdateSoundDistances()
// PURPOSE    : Scans a couple of lines to find the distance to the nearest
//				buildings..
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCamera::UpdateSoundDistances(void)
{
	CVector			ScanStart, TempVec;
	CColPoint		TestColPoint;
	CEntity*		TestEntity;
	Int16			Count;
	float			Inter=0.0f;

	// Find the point to to the scanning from.
	if ((((Cams[ActiveCam].Mode==CCam::MODE_1STPERSON)||(Cams[ActiveCam].Mode==CCam::MODE_SNIPER)
		||(Cams[ActiveCam].Mode==CCam::MODE_SNIPER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_RUNABOUT_HS)
		||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_FIGHT_CAM_RUNABOUT)
		||(Cams[ActiveCam].Mode==CCam::MODE_1STPERSON_RUNABOUT)||(Cams[ActiveCam].Mode==CCam::MODE_HELICANNON_1STPERSON)
		||(Cams[ActiveCam].Mode==CCam::MODE_CAMERA)
		||(Cams[ActiveCam].Mode==CCam::MODE_M16_1STPERSON)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER)||(Cams[ActiveCam].Mode==CCam::MODE_ROCKETLAUNCHER_HS)))&&(pTargetEntity->GetIsTypePed()))
	{
		ScanStart = this->GetPosition() + this->GetMatrix().GetForward() * 0.5f;//need to do this so that it is 
		//shorter than normal and doesn't go under the ground
	}
	else 
	{
		ScanStart = this->GetPosition() + this->GetMatrix().GetForward() * 5.0f;
	}
	// Do the 'up' scan.
	Count = (CTimer::m_FrameCounter % 12);	// 0 1 2 3 4 5 6 7 8 9 10 11 12 0 1 ...
	if (Count == 0)
	{
		SoundDistUpAsReadOld = SoundDistUpAsRead;
		if (CWorld::ProcessVerticalLine(ScanStart, ScanStart.z + CAM_DIST_SOUND_SCAN, TestColPoint, TestEntity, true, false, false, false))
		{
			SoundDistUpAsRead = TestColPoint.GetPosition().z - ScanStart.z;
		}
		else
		{
			SoundDistUpAsRead = CAM_DIST_SOUND_SCAN;
		}
	}
	Inter = (Count + 1) / 6.0f;
	SoundDistUp = SoundDistUpAsRead * Inter + SoundDistUpAsReadOld * (1.0f - Inter);

/*
	
	// Do the 'left' scan.
	Count = ((CTimer::m_FrameCounter+2) % 12);	// 0 1 2 3 4 5 0 1 ...
	if (Count == 0)
	{
		SoundDistLeftAsReadOld = SoundDistLeftAsRead;
		TempVec = ScanStart + (this->GetMatrix().GetRight() * CAM_DIST_SOUND_SCAN);
//	CDebug::DebugLine3D(ScanStart.x, ScanStart.y, ScanStart.z,
//								TempVec.x, TempVec.y, TempVec.z,
//								0xffff00ff, 0xffffffff );
		if (CWorld::ProcessLineOfSight(ScanStart, TempVec, TestColPoint, TestEntity, true, false, false, true, false, true, true))
		{
			SoundDistLeftAsRead = (TestColPoint.GetPosition() - ScanStart).Magnitude();
		}
		else
		{
			SoundDistLeftAsRead = CAM_DIST_SOUND_SCAN;
		}
	}	
	Inter = (Count + 1) / 6.0f;
	SoundDistLeft = SoundDistLeftAsRead * Inter + SoundDistLeftAsReadOld * (1.0f - Inter);
		
		
	// Do the 'right' scan.
	TempVec = ScanStart + (this->GetMatrix().GetRight() * -CAM_DIST_SOUND_SCAN);
	Count = ((CTimer::m_FrameCounter+4) % 12);	// 0 1 2 3 4 5 0 1 ...
	if (Count == 0)
	{
		SoundDistRightAsReadOld = SoundDistRightAsRead;
		TempVec = ScanStart - (this->GetMatrix().GetRight() * CAM_DIST_SOUND_SCAN);
//	CDebug::DebugLine3D(ScanStart.x, ScanStart.y, ScanStart.z,
//								TempVec.x, TempVec.y, TempVec.z,
//								0xff0000ff, 0xffffff00 );
		if (CWorld::ProcessLineOfSight(ScanStart, TempVec, TestColPoint, TestEntity, true, false, false, true, false, true, true))
		{
			SoundDistRightAsRead = (TestColPoint.GetPosition() - ScanStart).Magnitude();
		}
		else
		{
			SoundDistRightAsRead = CAM_DIST_SOUND_SCAN;
		}
	}
	Inter = (Count + 1) / 6.0f;
	SoundDistRight = SoundDistRightAsRead * Inter + SoundDistRightAsReadOld * (1.0f - Inter);

//	printf("Up:%f Left:%f Right:%f (%f %f %f)\n", SoundDistUp, SoundDistLeft, SoundDistRight,
//			SoundDistUpAsRead, SoundDistLeftAsRead, SoundDistRightAsRead);
	
*/
}




void CCamera::UpdateTargetEntity(void)
{
 	
  	bool InObbeCamCanGoIntoUpdateLoop=false;
 	bool PedInMidWayGettingIntoCarState=false;
 	CEntity* pTargetB4Update=pTargetEntity;
  	
  	m_bPlayerWasOnBike=false;
  	if (pTargetEntity!=NULL)
	{
	  	if (pTargetEntity->GetIsTypeVehicle())
		{
//			if (((CVehicle*)pTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
//			{
//				m_bPlayerWasOnBike=true;
//			}
			static float USE_BIKE_TRANSITION_SPEED = 0.3f;
			if(((CVehicle *)pTargetEntity)->GetMoveSpeed().MagnitudeSqr() > USE_BIKE_TRANSITION_SPEED)
				m_bPlayerWasOnBike = true;
		}
  	}
  	
  	
  	if (WhoIsInControlOfTheCamera==OBBE_CAM_CONTROL)
  	{ 
	  	InObbeCamCanGoIntoUpdateLoop=true;
	  	if ((m_iModeObbeCamIsInForCar==MOVIECAM8)||(m_iModeObbeCamIsInForCar==MOVIECAM7))
	  	{
	  		if (FindPlayerPed()->GetPedState()!=PED_ARRESTED)
	  		{
	  			InObbeCamCanGoIntoUpdateLoop=false;
	  		}
	  		
	  		if (FindPlayerVehicle() == NULL)
	  		{
		  		TIDYREF(pTargetEntity, &pTargetEntity);
	  			pTargetEntity=(CEntity*)FindPlayerPed();
	  			REGREF(pTargetEntity, &pTargetEntity);
	  		}
	  	}
  	}
  
 
	// try and work out what this set of bracketing does, damnit
    if ((((m_bLookingAtPlayer==true)||(InObbeCamCanGoIntoUpdateLoop)) && (m_uiTransitionState == TRANS_NONE))||((pTargetEntity==NULL)
    ||(m_bTargetJustBeenOnTrain==true)))//		
	{	
		if(FindPlayerVehicle() == NULL //player is in sometime of vehicle	
		|| (!CGameLogic::IsCoopGameGoingOn() && FindPlayerPed()->GetPedIntelligence()->GetTaskActiveSimplest()
		&& FindPlayerPed()->GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_GANG_DRIVEBY))
		{
			TIDYREF(pTargetEntity, &pTargetEntity);
			pTargetEntity=(CEntity*)FindPlayerPed();
			REGREF(pTargetEntity, &pTargetEntity);
			//if ((Cams[ActiveCam].Mode!=CCam::MODE_CAM_ON_A_STRING)&&(m_uiTransitionState==TRANS_NONE))
			//want to make sure they are not enterning a car big hack for below as there is a few frames
			//where the player vehicle is null when the dude is getting in the car
			{
			    if(FindPlayerPed()->GetPedState()==PED_ENTER_CAR
			    || FindPlayerPed()->GetPedState()==PED_CARJACK
			    || FindPlayerPed()->GetPedState()==PED_OPEN_DOOR)
			    {
//TEST			    	if(FindPlayerPed()->m_pObjectiveVehicle && m_nCarZoom!=ZOOM_ZERO)
//TEST			    		pTargetEntity = FindPlayerPed()->m_pObjectiveVehicle;
//TEST			    	else
				    	PedInMidWayGettingIntoCarState=true;
			    }
			    
			     
			    if (PedInMidWayGettingIntoCarState==false)
			    {
				    if (pTargetEntity!=Cams[ActiveCam].CamTargetEntity)
				    {
				      //  int whatisit;
				        //whatisit=FindPlayerPed()->GetPedState();
				        TIDYREF_NOTINWORLD(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
				        Cams[ActiveCam].CamTargetEntity=pTargetEntity; //fuck it lets just switch quickly
				        REGREF(Cams[ActiveCam].CamTargetEntity, &Cams[ActiveCam].CamTargetEntity);
				        //this should only happen when network game is getting set up quickly
				        //PlayerPed not 0 in array been switched somewhere
				    }
				}
			}
		}
		else	//but if they are entering a car or in a car set the target entity to the car
		{
			TIDYREF(pTargetEntity, &pTargetEntity);
			pTargetEntity=(CEntity*) FindPlayerVehicle();
			REGREF(pTargetEntity, &pTargetEntity);
		}
			//this is required to stop camera jumping when getting into the car
		bool bTargetCarIsLocked=true;
		if(CWorld::Players[CWorld::PlayerInFocus].pPed!=NULL)
		{
			if(CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle!=NULL)
			{
				if(FindPlayerPed()->m_pMyVehicle->CanPedOpenLocks(FindPlayerPed()))
				{
					bTargetCarIsLocked=false;
				}
			}
			/*
			else if(FindPlayerPed()->m_pObjectiveVehicle && (FindPlayerPed()->GetPedState()==PED_ENTER_CAR
		    || FindPlayerPed()->GetPedState()==PED_CARJACK || FindPlayerPed()->GetPedState()==PED_OPEN_DOOR))
		    {
				if(FindPlayerPed()->m_pObjectiveVehicle->CanPedOpenLocks(FindPlayerPed()))
				{
					bTargetCarIsLocked=false;
				}
		    }
		    */
		}
			
			
		if(CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()==PED_ENTER_CAR && bTargetCarIsLocked==false)
		{
			//pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed->m_pTargetEntity;
			if ((PedInMidWayGettingIntoCarState==false)&&(m_nCarZoom!=ZOOM_ZERO))//don't want to change immediately
			//when the player has entered a car and is in car first person mode i.s. want to show some of the anim of them
			//getting into the car.
			{
				TIDYREF(pTargetEntity, &pTargetEntity);
				pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle;
				if (CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle==NULL)
				{
					pTargetEntity=FindPlayerPed();
				}
				REGREF(pTargetEntity, &pTargetEntity);
			}
		}
			
		if((CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()==PED_CARJACK || CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()==PED_OPEN_DOOR) && bTargetCarIsLocked==false)
		{
			if ((PedInMidWayGettingIntoCarState==false)&&(m_nCarZoom!=ZOOM_ZERO))//don't want to change immediately
			//when the player has entered a car and is in car first person mode i.s. want to show some of the anim of them
			//getting into the car.
			{
				TIDYREF(pTargetEntity, &pTargetEntity);
				pTargetEntity=CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle;
				REGREF(pTargetEntity, &pTargetEntity);
			}
			if (CWorld::Players[CWorld::PlayerInFocus].pPed->m_pMyVehicle==NULL)
			{
				TIDYREF(pTargetEntity, &pTargetEntity);
				pTargetEntity=FindPlayerPed();
				REGREF(pTargetEntity, &pTargetEntity);
			}
		}
			
		if(CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()==PED_EXIT_CAR)
		{
			TIDYREF(pTargetEntity, &pTargetEntity);
			pTargetEntity=FindPlayerPed();
			REGREF(pTargetEntity, &pTargetEntity);
		}
		
		if(CWorld::Players[CWorld::PlayerInFocus].pPed->GetPedState()==PED_DRAGGED_FROM_CAR)
		{
			TIDYREF(pTargetEntity, &pTargetEntity);
			pTargetEntity=FindPlayerPed();
			REGREF(pTargetEntity, &pTargetEntity);
		}	
	  		//welcome to hackville
	  	if(pTargetEntity->GetIsTypeVehicle())
	  	{
			if(m_nCarZoom==ZOOM_ZERO && FindPlayerPed()->GetPedState()==PED_ARRESTED)
			{
				TIDYREF(pTargetEntity, &pTargetEntity);
				pTargetEntity=FindPlayerPed();
				REGREF(pTargetEntity, &pTargetEntity);
			}
		}
		
		
		
		
	}
	

	ASSERTMSG (pTargetEntity!=NULL, "Nothing for the camera to look at");
}



///////////////////////////////////////////////////////////////////////////
// NAME       : TakeControl()
// PURPOSE    : Lets the camera point to something else, Also allows you to set the mode
// RETURNS    : Nowt
// PARAMETERS : A pointer to the thing you want to look at, the Mode that you want to be in
///////////////////////////////////////////////////////////////////////////

void CCamera::TakeControl(CEntity* NewTarget, Int16 CamMode, Int16 CamSwitchStyle, int WhoIsTakingControl)
{
	
	bool SafeToTakeControl=true; 
	//only let obbe take control of the camera if script does have it already, also let 
	//the script tell obbe "NO NO NO "
	
	if (WhoIsTakingControl==OBBE_CAM_CONTROL)
	{
		if (WhoIsInControlOfTheCamera==SCRIPT_CAM_CONTROL)
		{
			SafeToTakeControl=false;
		}
	}
	
	if (m_bForceCinemaCam) // to allow Chris to very speicfially take control of the girlfriend driving camera.
	{
		SafeToTakeControl = true;
	}
	
	
	if (SafeToTakeControl)
	{
				
		
		WhoIsInControlOfTheCamera=WhoIsTakingControl;
		if (NewTarget==0) //they want to look at the player
		{
			if (FindPlayerVehicle() != NULL)
			{
			NewTarget= FindPlayerVehicle();
			}
			else
			{
			NewTarget=CWorld::Players[CWorld::PlayerInFocus].pPed;
			}
		}
			
		else if (CamMode==0) //They want to go to the defaults
		{
			if (pTargetEntity->GetIsTypePed())
			{
			CamMode=CCam::MODE_FOLLOWPED;
			}
			else if (pTargetEntity->GetIsTypeVehicle())
			{
			CamMode=CCam::MODE_CAM_ON_A_STRING;
			}
		
		}	
		m_bLookingAtVector=false;

		TIDYREF(pTargetEntity, &pTargetEntity);
		pTargetEntity=NewTarget;
		REGREF(pTargetEntity, &pTargetEntity);
		m_iModeToGoTo=CamMode;
		m_iTypeOfSwitch=CamSwitchStyle;
		m_bLookingAtPlayer=false;
		m_bStartInterScript=TRUE;
		CPlayerPed *pPlayer=FindPlayerPed();
	}
}

void CCamera::TakeControlNoEntity(const CVector& TargetCoors, Int16 CamSwitchStyle, int WhoIsTakingControl)
{
	bool SafeToTakeControl=true; 
	//only let obbe take control of the camera if script does have it already, also let 
	//the script tell obbe "NO NO NO "

	if (WhoIsTakingControl==OBBE_CAM_CONTROL)
	{
		if (WhoIsInControlOfTheCamera==SCRIPT_CAM_CONTROL)
		{
			SafeToTakeControl=false;
		}
	}
	
	if (SafeToTakeControl)
	{
		WhoIsInControlOfTheCamera=WhoIsTakingControl;
		m_bLookingAtVector=true;
		m_iModeToGoTo=CCam::MODE_FIXED;
		m_bLookingAtPlayer=false;
		m_vecFixedModeVector=TargetCoors;
		m_iTypeOfSwitch=CamSwitchStyle;
		m_bStartInterScript=TRUE;
	}
}

void CCamera::TakeControlAttachToEntity(CEntity* NewTarget, CEntity* AttachEntity, CVector& vecOffset, CVector& vecLookAt, float fTilt, Int16 CamSwitchStyle, int WhoIsTakingControl)
{
	bool SafeToTakeControl=true; 
	//only let obbe take control of the camera if script does have it already, also let 
	//the script tell obbe "NO NO NO "
	
	if (WhoIsTakingControl==OBBE_CAM_CONTROL)
	{
		if (WhoIsInControlOfTheCamera==SCRIPT_CAM_CONTROL)
		{
			SafeToTakeControl=false;
		}
	}
	
	if (SafeToTakeControl)
	{
				
		
		WhoIsInControlOfTheCamera=WhoIsTakingControl;
		
		// attach to player if nothing is passed in
		if (AttachEntity==0) 
		{
			if (FindPlayerVehicle() != NULL)
			{
			AttachEntity= FindPlayerVehicle();
			}
			else
			{
			AttachEntity=CWorld::Players[CWorld::PlayerInFocus].pPed;
			}
		}
		
		if(NewTarget)
		{
			TIDYREF(pTargetEntity, &pTargetEntity);
			pTargetEntity = NewTarget;
			REGREF(pTargetEntity, &pTargetEntity);
			m_bLookingAtVector=false;
		}
		else
		{
			m_bLookingAtVector=true;
			if(vecLookAt != vecOffset) 
			{
				m_vecAttachedCamLookAt=vecLookAt;
			}
			else
			{
				m_vecAttachedCamLookAt=CVector(0.0f, 0.0f, 0.0f);	
			}
		}
		
		if(vecOffset != CVector(0.0f, 0.0f, 0.0f))
		{
			m_vecAttachedCamOffset = vecOffset;
		}
		else
		{
			// use default
			m_vecAttachedCamOffset = CVector(0.0f, 0.0f, 2.0f); 
		}

		m_fAttachedCamAngle = fTilt;
		TIDYREF(pAttachedEntity, &pAttachedEntity);
		pAttachedEntity=AttachEntity;
		REGREF(pAttachedEntity, &pAttachedEntity);
		m_iModeToGoTo=CCam::MODE_ATTACHCAM;
		m_iTypeOfSwitch=CamSwitchStyle;
		m_bLookingAtPlayer=false;
		m_bStartInterScript=TRUE;
		CPlayerPed *pPlayer=FindPlayerPed();
	}

}

void CCamera::TakeControlWithSpline(Int16 CamSwitchStyle)
{
	
	m_iModeToGoTo=CCam::MODE_FLYBY;
	m_bLookingAtPlayer=false;
	m_bLookingAtVector=false;
	m_bcutsceneFinished = false;
	m_iTypeOfSwitch=CamSwitchStyle;
	m_bStartInterScript=TRUE;

}



//
//
float fHeliMinHeightAboveWater = 1.0f;
float fSeaplaneMinHeightAboveWater = -2.0f;
//
bool	CCamera::TryToStartNewCamMode(Int32 MovieCamMode)
{
	CVector			FixedCoors, Speed;
	float			NewZ;
	bool			bIs;
	CVector 		Diff;

	//!PC - these have been moved up in scope!
	Int32			i;
	CVehicle* pVeh;
	CAutomobile* pCar;
	float HorizontalAng;
	bool GroundFound;
	float GroundPosZ;
	CVector CamDiff;

	switch(MovieCamMode)
	{		
		case MOVIECAM0:		// WheelCam on player
			{
				CVehicle *pVehicle=FindPlayerVehicle();
				if (pVehicle!=NULL)
				{
					if (pVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
					{
						if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
						{
							return false;
						}	
					}
					
					if (pVehicle->GetModelIndex()==MODELID_CAR_RHINO)
					{
						return false;
					}
					
					//just to give a bit of background to whomever is reading this 		
					//really fucked off at this minute getting flicky shit happening 
					//where camera goes into mode for one second, to avoid this
					//basically will do the processing that the camera would do if in wheel cam 
					//mode and see if it is clear 
					//Very hacky, but we have about 12 hours till final submission
					//Cheers
					CVector  TempSource, TempFront;
					float	Angle=0.0f;
					TempSource = Multiply3x3(FindPlayerVehicle()->GetMatrix(),
												CVector(-1.4f, -2.3f, 0.3f) );
					TempSource += FindPlayerVehicle()->GetPosition();
					TempFront = FindPlayerVehicle()->GetMatrix().GetForward();
					
					if (!(CWorld::GetIsLineOfSightClear(pVehicle->GetPosition(), TempSource, true, false, false, false, false, false, false)))
					{
						return false;
					}
					
					TakeControl(pVehicle, CCam::MODE_WHEELCAM, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
					return true;
				}
				else
				{
					return false;
				}
			}
			break;

		case MOVIECAM1:		// Fixed cam just above the road. Quite far away.
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM1;
			FixedCoors += CVector (Speed.y * 3.0f, -Speed.x * 3.0f, 0.0f);
			if (FindPlayerVehicle()!=NULL)
			{
				if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
				{
					if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
					{
						return false;
					}	
				}
			}	
								
			NewZ = CWorld::FindGroundZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z + 5.0f, &bIs);
			if (bIs)
			{
				FixedCoors.z = NewZ + 1.5f;
			}
			else
			{
				NewZ = CWorld::FindRoofZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z - 5.0f, &bIs);
				if (bIs)
				{
					FixedCoors.z = NewZ + 1.5f;
				}
				else
				{
				}
			}
			
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) return false;
			
			Diff = FindPlayerCoors() - FixedCoors;
			Diff.z = 0.0f;
			if (Diff.Magnitude() > DISTCAM1_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return false;
			if (Diff.Magnitude() < DISTCAM1_RELEASE_TOO_CLOSE) return false;
			
			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
			
			if (CameraObscuredByWaterLevel()) 
				return false;			
				
			return true;
			break;
		case MOVIECAM2:		// Fixed right in front just above road
			if (FindPlayerVehicle()!=NULL)
			{
				if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
				{
					if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
					{
						return false;
					}	
				}
			}
			
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM2;
			FixedCoors += CVector (Speed.y * 2.5f, -Speed.x * 2.5f, 0.0f);
			
			NewZ = CWorld::FindGroundZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z + 5.0f, &bIs);
			if (bIs)
			{
				FixedCoors.z = NewZ + 0.5f;     //+ 0.25f; 
			}
			else
			{
				NewZ = CWorld::FindRoofZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z - 5.0f, &bIs);
				if (bIs)
				{
					FixedCoors.z = NewZ + 0.5f; //+ 0.25f;
				}
				else
				{
				}
			}	
			
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) return false;
			Diff = FindPlayerCoors() - FixedCoors;
			Diff.z = 0.0f;
			
			if (Diff.Magnitude() > DISTCAM2_RELEASE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return false;
			if (Diff.Magnitude() <DISTCAM2_RELEASE_TOO_CLOSE) return false;

			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

			RwCameraSetNearClipPlane(Scene.camera, (RwReal) NORMAL_NEAR_CLIP*0.5f);	
					
			if (CameraObscuredByWaterLevel()) 
				return false;			
			
			return true;
			break;

		case MOVIECAM3:		// Fixed can quite high up
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM3;
			FixedCoors += CVector (8.0f * Speed.y, -8.0f * Speed.x, 0.0f);
			FixedCoors.z += 16.0f;
			
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) return false;

			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
			
			if (CameraObscuredByWaterLevel()) 
				return false;			

			RwCameraSetNearClipPlane(Scene.camera, (RwReal) NORMAL_NEAR_CLIP*0.5f);	
						
			return true;
			break;
/*	DW- REMOVED	case MOVIECAM4:		// 1st person
			if(m_bDisableFirstPersonInCar)
				return false;
				
			TakeControl(FindPlayerEntity(), CCam::MODE_1STPERSON, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
			return true;
			break;
*/
		case MOVIECAM5:		// Fixed cam. Just above roofs of cars
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM5;
			FixedCoors += CVector(-6.0f * Speed.y, 6.0f * Speed.x, 0.0f);
				
			NewZ = CWorld::FindGroundZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z + 5.0f, &bIs);
			if (bIs)
			{
				FixedCoors.z = NewZ + 3.5f; 
			}
			else
			{
				NewZ = CWorld::FindRoofZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z - 5.0f, &bIs);
				if (bIs)
				{
					FixedCoors.z = NewZ + 3.5f;
				}
				else
				{
				}
			}
			
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) return false;
	
			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
			
			if (CameraObscuredByWaterLevel()) 
				return false;			
			
			return true;
			break;
		case MOVIECAM6:		// Standard camera
			TakeControl(FindPlayerEntity(), CCam::MODE_1STPERSON, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
			return true;
			break;
		case MOVIECAM7:		// Chase cam (try to find copper chasing us)
			{
				if (FindPlayerPed()->GetWantedLevel() < WANTED_LEVEL1) return false;	// No chase going on
				if (!FindPlayerVehicle()) return false;

				if (FindPlayerVehicle()!=NULL)
				{
					if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
					{
						if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
						{
							return false;
						}	
					}
				}
				

				CVehiclePool& VehiclePool = CPools::GetVehiclePool();
				i=VehiclePool.GetSize();	

				while(i--)
				{
					pVeh = VehiclePool.GetSlot(i);

					if ((pVeh && pVeh->GetBaseVehicleType() == VEHICLE_TYPE_CAR &&
						pVeh != FindPlayerVehicle() && pVeh->GetIsLawEnforcer()) &&  (pVeh->GetStatus() == STATUS_PHYSICS))
					{
						pCar = (CAutomobile *)pVeh;

						// Car has to be behind us AND the front vectors should be matched
						// up.
						float	DiffX, DiffY;
						DiffX = pVeh->GetPosition().x - FindPlayerCoors().x;
						DiffY = pVeh->GetPosition().y - FindPlayerCoors().y;

						if ( (FindPlayerCoors() - pVeh->GetPosition()).Magnitude() < 30.0f)
						{					
							if ( DiffX * FindPlayerVehicle()->GetMatrix().GetForward().x +
								DiffY * FindPlayerVehicle()->GetMatrix().GetForward().y < 0.0f)
							{
								if (pVeh->GetMatrix().GetForward().x * FindPlayerVehicle()->GetMatrix().GetForward().x +
									pVeh->GetMatrix().GetForward().y * FindPlayerVehicle()->GetMatrix().GetForward().y > 0.8f)
								{
									TakeControl(pVeh, CCam::MODE_CAM_ON_A_STRING, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
									
									if (!CameraObscuredByWaterLevel()) 
										return true;
								}
							}
						}
					}
				}
				return false;
			}
			break;
			
		case MOVIECAM8:		// Chase cam (try to find copper chasing us)
			{
				if (FindPlayerPed()->GetWantedLevel() < WANTED_LEVEL1) return false;	// No chase going on
				if (!FindPlayerVehicle()) return false;

				CVehiclePool &VehiclePool2 = CPools::GetVehiclePool();
				i=VehiclePool2.GetSize();	
				
				if (FindPlayerVehicle()!=NULL)
				{
					if (FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
					{
						if ((pTargetEntity->GetModelIndex())!=MODELID_BOAT_SEAPLANE)//the seaplane is classes as a boat but the camera treats it like a car
						{
							return false;
						}	
					}
				}
				

				
				while(i--)
				{
					pVeh = VehiclePool2.GetSlot(i);

					if (pVeh && pVeh->GetBaseVehicleType() == VEHICLE_TYPE_CAR &&
						pVeh != FindPlayerVehicle() && pVeh->GetIsLawEnforcer())
					{
						pCar = (CAutomobile *)pVeh;

						// Car has to be behind us AND the front vectors should be matched
						// up.
						float	DiffX, DiffY;
						DiffX = pVeh->GetPosition().x - FindPlayerCoors().x;
						DiffY = pVeh->GetPosition().y - FindPlayerCoors().y;
						
						if ( (FindPlayerCoors() - pVeh->GetPosition()).Magnitude() < 30.0f)
						{
							if ( DiffX * FindPlayerVehicle()->GetMatrix().GetForward().x +
								DiffY * FindPlayerVehicle()->GetMatrix().GetForward().y < 0.0f)
							{
								if (pVeh->GetMatrix().GetForward().x * FindPlayerVehicle()->GetMatrix().GetForward().x +
									pVeh->GetMatrix().GetForward().y * FindPlayerVehicle()->GetMatrix().GetForward().y > 0.8f)
								{
									
									//just to give a bit of background to whomever is reading this 		
									//really fucked off at this minute getting flicky shit happening 
									//where camera goes into mode for one second, to avoid this
									//basically will do the processing that the camera would do if in wheel cam 
									//mode and see if it is clear 
									//Very hacky, but we have about 12 hours till final submission
									//Cheers
									CVector  TempSource, TempFront;
									float	Angle=0.0f;
									TempSource = Multiply3x3(pVeh->GetMatrix(),
																CVector(-1.4f, -2.3f, 0.3f) );
									TempSource += pVeh->GetPosition();
									TempFront =pVeh->GetMatrix().GetForward();
									
									if (!(CWorld::GetIsLineOfSightClear(pVeh->GetPosition(), TempSource, true, false, false, false, false, false, false)))
									{
										return false;
									}
									
									
									TakeControl(pVeh, CCam::MODE_WHEELCAM, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
									
									if (!CameraObscuredByWaterLevel()) 
										return true;
								}
							}
						}
					}
				}
				return false;
			}
			break;

#if 0 // DW - not used
			// The following are ped cameras
		case MOVIECAM9:		// Fixed cam just above the road. Quite far away.
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM9;
			FixedCoors += CVector (2.0f, 1.0f, 0.0f);
				
			NewZ = CWorld::FindGroundZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z + 5.0f, &bIs);
			if (bIs)
			{
				FixedCoors.z = NewZ + 0.5f;
			}
			else
			{
				NewZ = CWorld::FindRoofZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z - 5.0f, &bIs);
				if (bIs)
				{
					FixedCoors.z = NewZ + 0.5f;
				}
				else
				{
				}
			}
			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

			if (CameraObscuredByWaterLevel()) 
				return false;			

			return true;
			break;
		case MOVIECAM10:		// Fixed cam quite low
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM10;
			FixedCoors += CVector (2.0f, 1.0f, 0.5f);
			
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) return false;

			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

			if (CameraObscuredByWaterLevel()) 
				return false;			

			return true;
			break;

		case MOVIECAM11:		// Fixed cam quite low
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM11;
			FixedCoors += CVector (2.0f, 1.0f, 20.0f);
			
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) return false;

			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

			if (CameraObscuredByWaterLevel()) 
				return false;			

			return true;
			break;

		case MOVIECAM12:		// Fixed cam quite low
			FixedCoors = FindPlayerCoors();
			Speed = FindPlayerSpeed();
			Speed.z = 0.0f;
			Speed.Normalise();
			FixedCoors += Speed * DISTCAM12;
			FixedCoors += CVector (2.0f, 1.0f, 10.5f);
			
			if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) return false;

			SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
			TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
			return true;
			break;
//		case MOVIECAM13:		// Topdown
//			TakeControl(FindPlayerEntity(), CCam::MODE_TOPDOWN, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
//			return true;
//			break;
#endif						
		//all heli cams
		case MOVIECAM14:  
			// Cam clipped to side of heli
			//CVehicle *pVehicle=FindPlayerVehicle();			
/*			pVehicle=FindPlayerVehicle();			
			if (pVehicle!=NULL && pVehicle->GetBaseVehicleType() != VEHICLE_TYPE_BOAT) // DW - cant allow boats for this mode cos its all a pile of unmaintainable code
			{			
				//Do the processing that the camera would do if in wheel cam 
				//mode and see if it is clear 
				CVector  TempSource, TempFront;
				float	Angle=0.0f;
				TempSource = Multiply3x3(FindPlayerVehicle()->GetMatrix(),
											CVector(-1.4f, -2.3f, 0.3f) );
				TempSource += FindPlayerVehicle()->GetPosition();
				TempFront = FindPlayerVehicle()->GetMatrix().GetForward();
				
				if(pVehicle->GetBaseVehicleType()!=VEHICLE_TYPE_BOAT &&
				!(CWorld::GetIsLineOfSightClear(pVehicle->GetPosition(), TempSource, true, false, false, false, false, false, false)))
				{
					return false;
				}

				TakeControl(pVehicle, CCam::MODE_WHEELCAM, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
				return true;
			}
			else*/
			{
				return false;
			}
			break;
		case MOVIECAM15: //straight in front of the player
			if (FindPlayerVehicle()!=NULL)
			{
				FixedCoors = FindPlayerCoors();
				Speed = FindPlayerSpeed();
				Speed.z = 0.0f;
				Speed.Normalise();
				FixedCoors += Speed * HELI_CAM_DIST_AWAY_ONE;
				FixedCoors.z=FindPlayerCoors().z + 0.5f;
				
				if(FindPlayerVehicle()->GetBaseVehicleType() == VEHICLE_TYPE_BOAT)
					FixedCoors.z += 1.0f;
					
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) 
				{
					return false;
				}
				CVector CamDiff = FindPlayerCoors() - FixedCoors;
				
				Diff = FindPlayerCoors() - FixedCoors;
				Diff.z = 0.0f;
				if (CamDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_ONE && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return false;
				if (CamDiff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_ONE) return false;

				SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
				TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
	
			}
			break;
		case MOVIECAM16: //underneath infront of and to the side of the player
			if (FindPlayerVehicle()!=NULL)
			{
				FixedCoors = FindPlayerCoors();
				Speed = FindPlayerSpeed();
				Speed.z = 0.0f;
				Speed.Normalise();
				float HorizontalAng=CGeneral::GetATanOfXY(Speed.x, Speed.y) + DEGTORAD(60);
				Speed+=CVector(CMaths::Cos(HorizontalAng), CMaths::Sin(HorizontalAng), 0);
				Speed.Normalise();
				FixedCoors += Speed * HELI_CAM_DIST_AWAY_TWO;
				FixedCoors.z=FindPlayerCoors().z - 5.5f;				
				bool RoofFound=false;
				float RoofPosZ=CWorld::FindRoofZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z, &RoofFound);
				if (RoofFound==TRUE)
				{
					FixedCoors.z=RoofPosZ + 0.5f;
				}								
				else if(CWaterLevel::GetWaterLevelNoWaves(FixedCoors.x, FixedCoors.y, FixedCoors.z, &RoofPosZ))
				{
					float fMinHeightAboveWater = fHeliMinHeightAboveWater;
					if(FindPlayerVehicle() && FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
						fMinHeightAboveWater = fSeaplaneMinHeightAboveWater;
						
					if(FixedCoors.z < RoofPosZ + fMinHeightAboveWater)
						FixedCoors.z = RoofPosZ + fMinHeightAboveWater;
				}
				
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) 
				{
					return false;
				}
				CVector CamDiff = FindPlayerCoors() - FixedCoors;
				
				if (CamDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_TWO  /*&& DotProduct(FindPlayerSpeed(), Diff) > 0.0f*/) return false;
				if (CamDiff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_TWO) return false;

				SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
				TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
	
			}
			break;
		case MOVIECAM17://Behind player slightly to the side 
			if (FindPlayerVehicle()!=NULL)
			{
				FixedCoors = FindPlayerCoors();
				Speed = FindPlayerSpeed();
				Speed.z = 0.0f;
				Speed.Normalise();
				float HorizontalAng=CGeneral::GetATanOfXY(Speed.x, Speed.y) + DEGTORAD(190.0f);
				Speed+=CVector(CMaths::Cos(HorizontalAng), CMaths::Sin(HorizontalAng), 0);
				Speed.Normalise();
				FixedCoors += Speed * HELI_CAM_DIST_AWAY_THREE;
				FixedCoors.z=FindPlayerCoors().z - 1.0f;				
				bool RoofFound=false;
				float RoofPosZ=CWorld::FindRoofZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z, &RoofFound);
				if (RoofFound==TRUE)
				{
					FixedCoors.z=RoofPosZ + 0.5f;
				}
				else if(CWaterLevel::GetWaterLevelNoWaves(FixedCoors.x, FixedCoors.y, FixedCoors.z, &RoofPosZ))
				{
					float fMinHeightAboveWater = fHeliMinHeightAboveWater;
					if(FindPlayerVehicle() && FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
						fMinHeightAboveWater = fSeaplaneMinHeightAboveWater;
						
					if(FixedCoors.z < RoofPosZ + fMinHeightAboveWater)
						FixedCoors.z = RoofPosZ + fMinHeightAboveWater;
				}

				
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) 
				{
					return false;
				}

				CamDiff = FindPlayerCoors() - FixedCoors;
				Diff = FindPlayerCoors() - FixedCoors;
				Diff.z = 0.0f;
				if (CamDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_THREE  && DotProduct(FindPlayerSpeed(), Diff) > 0.0f) return false;
				if (CamDiff.Magnitude() <HELI_CAM_DIST_TOO_CLOSE_THREE) return false;

				SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
				TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			


				return true;
	
			}
			break;		
		case MOVIECAM18:
			{
			//  Directly above the player 
				FixedCoors = FindPlayerCoors();
				if(FindPlayerVehicle() && FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
					FixedCoors.z+=HELI_CAM_DIST_AWAY_FOUR;
				else
					FixedCoors.z-=HELI_CAM_DIST_AWAY_FOUR;
				Speed = FindPlayerSpeed();
				HorizontalAng=CGeneral::GetATanOfXY(Speed.x, Speed.y) + DEGTORAD(145.0f);
				Speed+=CVector(CMaths::Cos(HorizontalAng), CMaths::Sin(HorizontalAng), 0);
				Speed.z=0.0f;
				Speed.Normalise();
				FixedCoors += Speed * 15.0f;
				
				GroundFound=false;
				GroundPosZ=CWorld::FindGroundZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z, &GroundFound);
				if(GroundPosZ==TRUE)
				{
					if (FixedCoors.z < GroundPosZ)
					FixedCoors.z=GroundPosZ + 0.5f;
				}
				else if(CWaterLevel::GetWaterLevelNoWaves(FixedCoors.x, FixedCoors.y, FixedCoors.z, &GroundPosZ))
				{
					float fMinHeightAboveWater = fHeliMinHeightAboveWater;
					if(FindPlayerVehicle() && FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
						fMinHeightAboveWater = fSeaplaneMinHeightAboveWater;
						
					if(FixedCoors.z < GroundPosZ + fMinHeightAboveWater)
						FixedCoors.z = GroundPosZ + fMinHeightAboveWater;
				}
				
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) 
				{
					return false;
				}
				CVector CamDiff = FindPlayerCoors() - FixedCoors;
				
				if (CamDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_FOUR /*&& DotProduct(FindPlayerSpeed(), Diff) > 0.0f*/) return false;
				if (CamDiff.Magnitude() < HELI_CAM_DIST_TOO_CLOSE_FOUR) return false;

				SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
				TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			break;
		case MOVIECAM19:
			//Directly above the player To the side
				FixedCoors = FindPlayerCoors();
				if(FindPlayerVehicle() && FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
					FixedCoors.z += 4.0f;
				else
					FixedCoors.z -= 1.0f;				
//				CVector RightVector=FindPlayerVehicle()->GetMatrix().GetRight();
//				FixedCoors+=RightVector * 9.0f;
				Speed = FindPlayerSpeed();
				HorizontalAng=CGeneral::GetATanOfXY(Speed.x, Speed.y) + DEGTORAD(28.0f);
				Speed+=CVector(CMaths::Cos(HorizontalAng), CMaths::Sin(HorizontalAng), 0);
				Speed.z=0.0f;
				Speed.Normalise();
				FixedCoors+=Speed  * 12.5f;

				GroundFound=false;
				GroundPosZ=CWorld::FindGroundZFor3DCoord(FixedCoors.x, FixedCoors.y, FixedCoors.z, &GroundFound);
				if (GroundPosZ==TRUE)
				{
					if (FixedCoors.z<GroundPosZ)
					FixedCoors.z=GroundPosZ + 0.5f;
				}								
				else if(CWaterLevel::GetWaterLevelNoWaves(FixedCoors.x, FixedCoors.y, FixedCoors.z, &GroundPosZ))
				{
					float fMinHeightAboveWater = fHeliMinHeightAboveWater;
					if(FindPlayerVehicle() && FindPlayerVehicle()->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
						fMinHeightAboveWater = fSeaplaneMinHeightAboveWater;
						
					if(FixedCoors.z < GroundPosZ + fMinHeightAboveWater)
						FixedCoors.z = GroundPosZ + fMinHeightAboveWater;
				}
				
				if (!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), FixedCoors, true, false, false, false, false, false, false)) 
				{
					return false;
				}
				CamDiff = FindPlayerCoors() - FixedCoors;
				
				if (CamDiff.Magnitude() > HELI_CAM_MAX_DIST_AWAY_FIVE /*&& DotProduct(FindPlayerSpeed(), Diff) > 0.0f*/) return false;
				if (CamDiff.Magnitude() < HELI_CAM_DIST_TOO_CLOSE_FIVE) return false;

				SetCamPositionForFixedMode(FixedCoors, CVector(0.0f, 0.0f, 0.0f)); 
				TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			break;
		case MOVIECAM20: // Heli chase
			if (Cams[ActiveCam].Process_DW_HeliChaseCam(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_HELI_CHASE, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;
			break;
		case MOVIECAM21: // Cam man
			if (Cams[ActiveCam].Process_DW_CamManCam(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_CAM_MAN, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;
			break;
		case MOVIECAM22: // Bird
			if (Cams[ActiveCam].Process_DW_BirdyCam(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_BIRDY, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;
			break;		
		case MOVIECAM23: // Plane Spotter
			if (Cams[ActiveCam].Process_DW_PlaneSpotterCam(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_PLANE_SPOTTER, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;
			break;				
		case MOVIECAM24: // Dog Fight

			if (Cams[ActiveCam].Process_DW_DogFightCam(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_DOG_FIGHT, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;
			break;			
		case MOVIECAM25: // Fish cam
			if (Cams[ActiveCam].Process_DW_FishCam(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_FISH, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
				return true;
			}
			return false;
			break;		
		case MOVIECAMPLANE1: // Plane 1
			if (Cams[ActiveCam].Process_DW_PlaneCam1(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_PLANECAM1, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;
		case MOVIECAMPLANE2: // Plane 2
			if (Cams[ActiveCam].Process_DW_PlaneCam2(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_PLANECAM2, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;			
			break;
		case MOVIECAMPLANE3: // Plane 1
			if (Cams[ActiveCam].Process_DW_PlaneCam3(true))
			{
				TakeControl(FindPlayerEntity(), CCam::MODE_DW_PLANECAM3, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);

				if (CameraObscuredByWaterLevel()) 
					return false;			

				return true;
			}
			return false;															
		case CAM_ON_A_STRING_LAST_RESORT:
			TakeControl(FindPlayerEntity(), CCam::MODE_CAM_ON_A_STRING, CCamera::TRANS_JUMP_CUT, OBBE_CAM_CONTROL);
			return true;
			break;
	
	}
	return false;
}









///////////////////////////////////////////////////////////////////////////
// NAME       : UpdateAimingCoors()
// PURPOSE    : Updates the coordinates that the camera is aiming for. Has
//				to be called each frame.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCamera::UpdateAimingCoors(const CVector &NewAimingCoors)
{
	m_cvecAimingTargetCoors = NewAimingCoors;
}



//
//
//
float ARRESTDIST_BEHIND_COP = 5.0f;
float ARRESTDIST_RIGHTOF_COP = 3.0f;
float ARRESTDIST_ABOVE_COP	= 1.4f;
float ARRESTDIST_MINFROM_PLAYER = 8.0f;
//
bool CCam::GetLookOverShoulderPos(CEntity *pTargetEntity, CPed *pCopPed, CVector &vecTarget, CVector &vecSource)
{
	if(pTargetEntity==NULL || pCopPed==NULL)
		return false;

	float fTempMagnitude = 0.0f;
	CVector CopPosition = pCopPed->GetPosition();// + CVector(0.0f, 0.0f, ARRESTDIST_ABOVE_COP);
	CVector CopToPlayer = vecTarget - CopPosition;
	
	CVector vecTempDirn = CrossProduct(CopToPlayer, CVector(0.0f, 0.0f, 1.0f));
	vecTempDirn.Normalise();
	
	CopPosition += vecTempDirn*ARRESTDIST_RIGHTOF_COP;
	
	CopToPlayer.Normalise();
	if(CopToPlayer.z < -0.7071f)
	{
		CopToPlayer.z = -0.7071f;
		float fTempScale2d = CopToPlayer.Magnitude2D() / 0.7071f;
		if(fTempScale2d > 0.0f){
			CopToPlayer.x /= fTempScale2d;
			CopToPlayer.y /= fTempScale2d;
		}
		CopToPlayer.Normalise();
	}
	else if(CopToPlayer.z > 0.0f)
	{
		CopToPlayer.z = 0.0f;
		CopToPlayer.Normalise();
	}
	
	
	CopPosition -= ARRESTDIST_BEHIND_COP*CopToPlayer;
		
	// recalculate cop to player ped vector
	CopToPlayer = vecTarget - CopPosition;
	// make sure we're at least a min distance away from the player
	if((fTempMagnitude = CopToPlayer.Magnitude()) < ARRESTDIST_MINFROM_PLAYER && fTempMagnitude > 0.0f)
	{
		CopToPlayer *= ARRESTDIST_MINFROM_PLAYER/fTempMagnitude;
	}

	vecSource = vecTarget - CopToPlayer;
	return true;
}


//
//
//
bool CCam::GetLookAlongGroundPos(CEntity *pTargetEntity, CPed *pCopPed, CVector &vecTarget, CVector &vecSource)
{
	if(pTargetEntity==NULL || pCopPed==NULL)
		return false;

	float fTempMagnitude = 0.0f;
	CVector CopPosition = pCopPed->GetPosition();
	CVector CopToPlayer = vecTarget - CopPosition;
	CopToPlayer.z = 0.0f;
	CopToPlayer.Normalise();

	vecSource = vecTarget + ARRESTDIST_ALONG_GROUND*CopToPlayer;
	vecSource += ARRESTDIST_SIDE_GROUND*CrossProduct(CopToPlayer, CVector(0.0f, 0.0f, 1.0f));
	vecSource.z = vecTarget.z + 5.0f;
	
	bool bGroundFound = false;
	float fGroundZ=CWorld::FindGroundZFor3DCoord(vecSource.x, vecSource.y, vecSource.z, &bGroundFound); 
	if(bGroundFound)
		vecSource.z = fGroundZ + ARRESTDIST_ABOVE_GROUND;
		
	return true;
}

//
//
float ARRESTCAM_LAMP_BEST_DIST = 17.0f;
//
bool CCam::GetLookFromLampPostPos(CEntity *pTargetEntity, CPed *pCopPed, CVector &vecTarget, CVector &vecSource)
{
	CEntity *ppResults[16];
	CEntity *pFoundEntity = NULL;
	int16	nNum, i;
	float	BestDist, Dist;
	CVector vecTestLosTarget, vecTestLosStart;
	
	// search for objects and dummies
	CWorld::FindObjectsInRange(vecTarget, 30.0f, true, &nNum, 15, ppResults, false, false, false, true, true);
	BestDist = 10000.0f;
	for(i=0; i<nNum; i++) // find best one
	{
		// we're only really looking for lampposts or traffic lights at the mo'
		if( ppResults[i]->GetIsStatic() && ppResults[i]->GetMatrix().zz > 0.9f
		&&	bIsLampPost(ppResults[i]->GetModelIndex()))
		{
			Dist = (ppResults[i]->GetPosition() - vecTarget).Magnitude2D();
			
			if( Dist > 5.0f && CMaths::Abs(ARRESTCAM_LAMP_BEST_DIST - Dist) < BestDist)
			{
				vecTestLosStart = ppResults[i]->GetColModel().GetBoundBoxMax();
				vecTestLosStart = ppResults[i]->GetMatrix() * vecTestLosStart;
				
				vecTestLosTarget = vecTestLosStart - vecTarget;
				vecTestLosTarget.Normalise();
				vecTestLosTarget += vecTarget;
				
				if(CWorld::GetIsLineOfSightClear(vecTestLosStart, vecTestLosTarget, true, false, false, false, false, true, true))
				{
					BestDist = CMaths::Abs(ARRESTCAM_LAMP_BEST_DIST - Dist);
					pFoundEntity = ppResults[i];
					vecSource = vecTestLosStart;
				}
			}
		}
	}
	
	if(pFoundEntity)
		return true;
	
	else
		return false;
}


void CCam::DoCamBump(float fBumpHorz, float fBumpVert)
{
	m_fCamBumpedHorz = fBumpHorz;
	m_fCamBumpedVert = fBumpVert;
	
	m_nCamBumpedTime = CTimer::GetTimeInMilliseconds();
}


/*
bool CCamera::GetIsSurfaceTypeSteepHill(int e_SurfaceType)
{
	switch (e_SurfaceType)
	{
		case COLPOINT_SURFACETYPE_GRASS:
		case COLPOINT_SURFACETYPE_GRASS_ROUGH:
		case COLPOINT_SURFACETYPE_GRASS_SMOOTH:
		case COLPOINT_SURFACETYPE_GRAVEL:
		case COLPOINT_SURFACETYPE_MUD:
		case COLPOINT_SURFACETYPE_STEEPHILL:
		case COLPOINT_SURFACETYPE_RUBBER:
		case COLPOINT_SURFACETYPE_THICK_METAL_PLATE:
			return true;
			break;
		default:
			return false;
			break;
	}	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : GetBetaAngleAmendmentForClearView
// PURPOSE    : Works out the Beta angle where you get a clear view of the player 
// RETURNS    : An float representing the addition to a beta angle(in radians) where you get a clear view
//			  : and true or false depending whether you have found a clear view(true)
//			  :	or false because you haven't
// PARAMETERS : Takes in the Players position. The Amount you want to check behind the camera, 
//            : And the amount in degrees you wish to go to either side of the camera 
///////////////////////////////////////////////////////////////////////////

bool CCamera::GetBetaAngleAmendmentForClearView(const CVector &ThePlayerPos,  float DistBehind, float SideOffset,const float Beta, float* result, float CameraZOffSet, bool bCheckBuildings, bool bCheckVehicles, bool bCheckPeds, bool bCheckObjects, bool bCheckDummies)
{
	float PosTestAngle=0.0f;
	float NegTestAngle=0.0f;
	float AngleCounter=0.0f;
	CVector TestVector;
	CVector BehindVect;
//	CVector PlayerPosition;
	CColPoint colPoint;
	CEntity* pHitEntity=NULL;

	while (AngleCounter<=PI)///Do loop while positive angle is 
 							  // less than 180 degrees, if find successful value, exit.
 	{		
		//work out a vector that would be behind the camera taking into account the sideoffset and the postestangle
		if (SideOffset<=0)
		{	
			BehindVect = DistBehind * CVector( CMaths::Cos(Beta + PosTestAngle + SideOffset), CMaths::Sin(Beta + PosTestAngle + SideOffset), 0.0f);
			BehindVect.z+= CameraZOffSet; 
			TestVector=ThePlayerPos + BehindVect;
	 		//test vector is now in the position having 5 degrees added to it   
					

	 		if (!(CWorld::ProcessLineOfSight(ThePlayerPos, TestVector, colPoint, pHitEntity, bCheckBuildings, bCheckVehicles, bCheckPeds, bCheckObjects, bCheckDummies, false, true)))	
			{
				*result=PosTestAngle;
				return true;
			}
	
			PosTestAngle+=0.087266f;//equal to +5 degrees
		}
		
		
		if (SideOffset>=0)
		{
			//work out a vector that would be behind the camera taking into account the sideoffset and the negtestangle
			BehindVect = DistBehind * CVector( CMaths::Cos(Beta + NegTestAngle + SideOffset), CMaths::Sin(Beta + NegTestAngle + SideOffset), 0.0f);
			BehindVect.z+= CameraZOffSet;
			TestVector=ThePlayerPos + BehindVect;
			//test vector is now in the position having 5 degrees subtracted from it   
	 		if (!(CWorld::ProcessLineOfSight(ThePlayerPos, TestVector, colPoint, pHitEntity, bCheckBuildings, bCheckVehicles, bCheckPeds, bCheckObjects, bCheckDummies,  false, true)))		
			{
				*result=NegTestAngle;
				return true;
			}	

			NegTestAngle-=0.087266f;//equal to -5 degrees	 			
		}
		AngleCounter+=0.087266f;//equal to +5 degrees
	}	
	return false;
}

*/

#ifdef DW_CAMERA_COLLISION

//--------------------------------------------------------------------------------
// Variables controlling the collision detection and reaction of a camera
// finely tuned variables - be careful!
// dont reorder.
SCamColVars gCamColVars[CAM_COL_VARS_MAX] =
{
//	radius  	maxRadius 	minDist 	clipDist	clipNearest	 	zoomOutSpeed
	{1.00f,		3.0f, 		0.1f,		0.75f,		0.04f,			0.2f}, // CAM_COL_VARS_PLAYER_AIM_WEAP_ONFOOT,
	{0.65f,		3.0f,		0.1f,		0.3f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_AIM_WEAP_ONBIKE,
	{0.65f,		3.0f,		0.1f,		0.3f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_AIM_WEAP_INCAR,
	{0.65f,		3.0f,		0.1f,		0.3f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_AIM_WEAP_MELEE,
	{0.65f,		3.0f,		0.1f,		0.5f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_OUTSIDE_NEAR_RANGE,
	{0.65f,		3.0f,		0.1f,		0.3f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE,
	{0.65f,		3.0f,		0.05f,		0.3f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_OUTSIDE_FAR_RANGE,
	{0.65f,		3.0f,		0.1f,		0.5f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_INSIDE_NEAR_RANGE,
	{0.65f,		3.0f,		0.1f,		0.3f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_INSIDE_MED_RANGE,
	{0.65f,		3.0f,		0.05f,		0.3f,		0.05f,			0.2f}, // CAM_COL_VARS_PLAYER_INSIDE_FAR_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_CAR_NEAR_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_CAR_MED_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_CAR_FAR_RANGE,
	{0.65f,		0.65f,		0.07f,		0.3f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_BIKE_NEAR_RANGE,
	{0.65f,		0.65f,		0.05f,		0.3f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_BIKE_MED_RANGE,
	{0.65f,		0.65f,		0.02f,		0.3f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_BIKE_FAR_RANGE,
	{0.65f,		3.0f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_HELI_NEAR_RANGE,
	{0.65f,		3.0f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_HELI_MED_RANGE,
	{0.65f,		3.0f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_HELI_FAR_RANGE,
	{0.65f,		3.0f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_PLANE_NEAR_RANGE,
	{0.65f,		3.0f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_PLANE_MED_RANGE,
	{0.65f,		3.0f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_PLANE_FAR_RANGE,
	{0.65f,		0.25f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_RCCAR_NEAR_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_RCCAR_MED_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_RCCAR_FAR_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_RCHELI_NEAR_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}, // CAM_COL_VARS_FOLLOW_RCHELI_MED_RANGE,
	{0.65f,		0.65f,		0.1f,		0.1f,		0.05f,			0.1f}  // CAM_COL_VARS_FOLLOW_RCHELI_FAR_RANGE,
};

uint8 gCurCamColVars = CAM_COL_VARS_PLAYER_OUTSIDE_MED_RANGE; // the default camera mode
float gCurDistForCam = 1.0f; // the current distance of the camera
struct SCamColVars *gpCamColVars = &gCamColVars[gCurCamColVars]; // a pointer to the current camera collision variable dataset

float gPedMaxHeight 	= 2.0f;	// height of player for defining cylinder about the ped
float gPedCylinderWidth = 1.0f;//p//0.75f;//4f;	// radius of cylinder about ped at which it should not be rendered
float gPedClipDist 		= 0.0f; // DW -not used//0.75f;// radius ( cylinder ) about ped at which clipping should be brought in.

//--------------------------------------------------------------------------------
// for setting the variables controlling camera collision detection and reaction
void CCamera::SetCamCollisionVarDataSet(int32 id)
{
	ASSERT(id<CAM_COL_VARS_MAX);
	
	if (id==gCurCamColVars) // already set - dont reinitialise!
		return;
		
	gCurDistForCam = 1.0f;
	gCurCamColVars = id;
	gpCamColVars   = &gCamColVars[id];
}

//--------------------------------------------------------------------------
// Attempts to scale the clip distance ( near ) by the distance to a ped
void CCamera::SetNearClipBasedOnPedCollision(float nearestDist)
{	
	ASSERT(false);
	float len = CMaths::Sqrt(nearestDist);
			
	// This distance will scale the clipping Distance from Current to gpCamColVars->clippingDistance
	// by this distance proportion of 0 to gPedCylinderWidth		
	len /= gPedClipDist;		
	float clipMax = NORMAL_NEAR_CLIP;//RwCameraGetNearClipPlane(Scene.camera); // might need to be smarter...?
	float clipMin = gpCamColVars->clippingDistance;
	float clipRange = clipMax - clipMin;		
		
	static float gScalePedClipping = 0.25f; // had to bring it in closer to be better, quite severe on z fighting etc. but still worth it.
	float newClip = clipMin + (clipRange * (len*gScalePedClipping));					
	
	if (newClip < clipMin) 
		newClip = clipMin;	
		
	RwCameraSetNearClipPlane(Scene.camera, newClip);
}


float gRadiusScalarForLengthToVehicle = 0.2939f; // scales the radius of cameras to achieve a normalised behaviour from original settings....blah blah blah

float gLastRadiusUsedInCollisionPreventionOfCamera = 0.0f;

#define MIN3(a,b,c)	(((a) < (b)) ? ((a)<(c)) ? (a) : (c) : ((b)<(c)) ? (b) : (c) )
//--------------------------------------------------------------------------------
// highest level camera collison detection and reaction function for various modes
bool CCamera::CameraColDetAndReact(CVector *pSource, CVector *pVecTargetCoords)
{	
	START_TIMER("DWCAM");
	
	bool bCollided = false;
	CVector dest;
	float dist;
	CVector delta = *pSource - *pVecTargetCoords;
	ASSERT(gpCamColVars);
	bool bClippingIsSetNear = false;
	
	// radius is bigger the longer the distance we need to cast
	float len = delta.Magnitude();
	float radius = len * gRadiusScalarForLengthToVehicle*gpCamColVars->camRad;
		
	// Constrain radius size
	if (gCurCamColVars>=CAM_COL_VARS_FOLLOW_CAR_NEAR_RANGE)
	{
		// Constrain radius to no bigger than vehicle driven.... tricky to do this (this is done every frame)... rewrite how radius is fed in
		CEntity *pEntity = CWorld::pIgnoreEntity;
		if(pEntity && pEntity->GetIsTypeVehicle() && ((CVehicle *)pEntity)->GetVehicleType()==VEHICLE_TYPE_CAR)
		{
			static uint32 DIST_LOWEST_Z_STORED = 0;
			static float DIST_LOWEST_Z = 0.0f;
			if(pEntity->GetModelIndex()!=DIST_LOWEST_Z_STORED)
			{
				CColModel& colModel = pEntity->GetColModel();
				CCollisionData* pColData = colModel.GetCollisionData();
				DIST_LOWEST_Z = 100.0f;
				if(pColData)
				{
					for(int i=0; i<pColData->m_nNoOfSpheres; i++)
				{
						if(pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius < DIST_LOWEST_Z)
							DIST_LOWEST_Z = pColData->m_pSphereArray[i].m_vecCentre.z - pColData->m_pSphereArray[i].m_fRadius;
					}
				}
			
				DIST_LOWEST_Z_STORED = pEntity->GetModelIndex();
			}
			
			float fCamPosZ = DotProduct(*pVecTargetCoords - pEntity->GetPosition(), pEntity->GetMatrix().GetUp());
			fCamPosZ =  CMaths::Max(0.2f, fCamPosZ - DIST_LOWEST_Z);
			radius = MIN3(gpCamColVars->maxCamRad, fCamPosZ, radius);
		}
		else if(pEntity)
		{
			CVector d = CModelInfo::GetBoundingBox(pEntity->m_nModelIndex).GetBoundBoxMax() - CModelInfo::GetBoundingBox(pEntity->m_nModelIndex).GetBoundBoxMin();
			d*=0.5f;
			float smallestDim = MIN3(d.x,d.y,d.z);		
			radius = MIN3(gpCamColVars->maxCamRad,smallestDim,radius);
		}			
	}		

	// Camera mode constraints.
	if (radius>gpCamColVars->maxCamRad)
		radius = gpCamColVars->maxCamRad;
	
	// Camera mode constraints.
	static float gThinnestCollisionRadiusAllowed = 0.65f;

	float thinRad = gThinnestCollisionRadiusAllowed;

// Removed - not required	
//	if (gCurCamColVars>=CAM_COL_VARS_FOLLOW_BIKE_NEAR_RANGE && gCurCamColVars<=CAM_COL_VARS_FOLLOW_BIKE_FAR_RANGE)
//	{
//		thinRad = 1.5f;
//	}	
	
	if (radius<thinRad) // a fix for bikes... too thin!
		radius = thinRad;


	float minDist = gpCamColVars->camMinDist;

	// Make the distance to bring the camera in under collision a fixed distance	
	if (gCurCamColVars <= CAM_COL_VARS_PLAYER_INSIDE_FAR_RANGE)
	{		
		static float minDistAllowedToPlayer = 0.30f; // pushed out as far as possible... every bit improves clipping. // reduced now to allow really near clip.
		static float minDistAllowedToPlayer2 = 0.18f; // fark this is annoying... the distance along this vector needs to be lower
		
		float minDistAllowed = minDistAllowedToPlayer;
		
		if (gCurCamColVars <= CAM_COL_VARS_PLAYER_AIM_WEAP_MELEE)
		{
			minDistAllowed = minDistAllowedToPlayer2;
		} 		
		
		CVector delta = *pSource - *pVecTargetCoords;
		float len = delta.Magnitude();
		
		minDist = minDistAllowed/len;
	}
	
	// Bike must have different settings.
	bool bIsABike = false;
	static float gNearestDistToBikeAllowed = 0.05f; // must be allowed really close to bike!
	if (gCurCamColVars>=CAM_COL_VARS_FOLLOW_CAR_NEAR_RANGE)
	{
		CEntity *pEntity = CWorld::pIgnoreEntity;
		if(pEntity && pEntity->GetIsTypeVehicle() && ((CVehicle *)pEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
		{	
			bIsABike = true;
			minDist  = gNearestDistToBikeAllowed;
		}
	}
		
	
	gLastRadiusUsedInCollisionPreventionOfCamera = radius;
	
	if (TheCamera.ConeCastCollisionResolve(pSource, pVecTargetCoords, &dest, radius, minDist, &dist))
	{			
		bCollided = true;
		if (dist<=gpCamColVars->distToModClipping)
		{		
			RwCameraSetNearClipPlane(Scene.camera, gpCamColVars->clippingDistance);  //MAX(0.05f, fDistColToPed - 0.3f));
			bClippingIsSetNear = true;
		}
	}
		
	if (dist<gCurDistForCam)
		gCurDistForCam = dist;
	else
	{
		// now this is a little possible bit of polish.. but we shouldnt really zoom out unless we are moving the pads
		static CVector gCamPosCached(0,0,0);
		static float gSpeedToConsiderZoomOut = 0.01f;
		static float gMaxAmountToZoomOutCurDistTForCamCollision = 0.05f;
		CVector d = *pSource - gCamPosCached;
		float len = d.MagnitudeSqr();
						
		if (len>(gSpeedToConsiderZoomOut*gSpeedToConsiderZoomOut))
		{
			// at full frame rate we wanted to zoom out at moveDelta rate
			float moveDelta = gpCamColVars->speedZoomOut * CTimer::GetTimeStep(); // scale by timestep. ( sync to framerate ) 
			float amountToMove = (dist-gCurDistForCam) * moveDelta;
			if (amountToMove>gMaxAmountToZoomOutCurDistTForCamCollision)
				amountToMove = gMaxAmountToZoomOutCurDistTForCamCollision;

			gCurDistForCam += amountToMove;		
		
		}
		
		gCamPosCached = *pSource;
	}
		
	if (gCurDistForCam>1.0f) gCurDistForCam = 1.0f;
	
	*pSource = *pVecTargetCoords + (delta * gCurDistForCam);			
		
	if (bIsABike) // Bikes need to have a really near clip for player on it clips with camera often.
	{
		static float gCollisionTriggerForNearerClipOnBikes = 0.5f;
		if (gCurDistForCam<gCollisionTriggerForNearerClipOnBikes)
		{
			static float gBikeNearClip = 0.05f;
			RwCameraSetNearClipPlane(Scene.camera, gBikeNearClip);
		}
	}
	
							
	END_TIMER("DWCAM");
	
	return bCollided;
}


//-----------------------------------------------------------------------
// The predictive cone cast method of getting the camera out of collision
// pass in the position of the camera, the position you are looking at, and it will return a destination position and the distance to this
// it will limit the movement by minimum distance which is expressed in a normalised scale from 1.0f (immediate collision) to 0.0f (no miminum)
bool CCamera::ConeCastCollisionResolve(CVector *pPos, CVector *pLookAt, CVector *pDest, float rad, float minDist, float *pDist)
{
	if (*pPos == *pLookAt) // paranoia
		return false; // illegal use of this function

	ASSERT(minDist>=0.0f);
	ASSERT(minDist<=1.0f);

	// setup spheres
	CColSphere lookAt;
	lookAt.Set(rad,*pLookAt);
	
	CColSphere cam;
	cam.Set(rad,*pPos);

	// Now check the distance to collision	
	bool bCollided = CCollision::CameraConeCastVsWorldCollision(&lookAt,&cam,pDist,minDist); // Set last parameter to make the test self truncating

	if (bCollided)
	{
		ASSERT(*pDist<=1.0f);		
		CVector d = *pPos - *pLookAt;
		*pDest = *pLookAt + ((*pDist) * d);		
	}
	else // no collision - nothing to do but lets set pDest anyway.
	{
		*pDest = *pPos;
		*pDist = 1.0f; // represents no collision.
	}
	
	return bCollided;
}

//---------------------------------------------------------------------------
// Setup collision detection and reaction variables for these camera submodes
// Aim weapon cam variable mode switching
void CCamera::SetColVarsAimWeapon(int32 nAimType)
{
	// Aim weapon follow cam
	switch (nAimType)
	{
		case AIMWEAPON_ONFOOT :	
			SetCamCollisionVarDataSet(CAM_COL_VARS_PLAYER_AIM_WEAP_ONFOOT);	
			break;
		case AIMWEAPON_ONBIKE :	
			SetCamCollisionVarDataSet(CAM_COL_VARS_PLAYER_AIM_WEAP_ONBIKE);	
			break;
		case AIMWEAPON_INCAR  :	
			SetCamCollisionVarDataSet(CAM_COL_VARS_PLAYER_AIM_WEAP_INCAR);	
			break;
		case AIMWEAPON_MELEE  :	
			SetCamCollisionVarDataSet(CAM_COL_VARS_PLAYER_AIM_WEAP_MELEE);	
			break;
		default :
			ASSERT(false);
			break;
	}
}

//---------------------------------------------------------------------------
// Setup collision detection and reaction variables for these camera submodes
// Ped normal follow cam
void CCamera::SetColVarsPed(int32 nType, int32 nPedZoom)
{
	ASSERT(ZOOM_ONE==1);
	ASSERT(ZOOM_TWO==2);
	ASSERT(ZOOM_THREE==3);
	
	// Ped follow cam
	switch (nType)
	{
		case FOLLOW_PED_OUTSIDE:
			SetCamCollisionVarDataSet(CAM_COL_VARS_PLAYER_OUTSIDE_NEAR_RANGE+nPedZoom-1);
			break;
		case FOLLOW_PED_INSIDE:
			SetCamCollisionVarDataSet(CAM_COL_VARS_PLAYER_INSIDE_NEAR_RANGE+nPedZoom-1);
			break;
		default :
			ASSERT(false);
			break;
	}
}


//---------------------------------------------------------------------------
// Setup collision detection and reaction variables for these camera submodes
// Vehicle follow cam variable mode switching
void CCamera::SetColVarsVehicle(int32 nType, int32 nCarZoom)
{
	ASSERT(ZOOM_ONE==1);
	ASSERT(ZOOM_TWO==2);
	ASSERT(ZOOM_THREE==3);
	
	switch (nType)
	{
		case FOLLOW_CAR_INCAR :
		case FOLLOW_CAR_INBOAT :
			SetCamCollisionVarDataSet(CAM_COL_VARS_FOLLOW_CAR_NEAR_RANGE+nCarZoom-1);
			break;
		case FOLLOW_CAR_ONBIKE :
			SetCamCollisionVarDataSet(CAM_COL_VARS_FOLLOW_BIKE_NEAR_RANGE+nCarZoom-1);
			break;
		case FOLLOW_CAR_INHELI :
			SetCamCollisionVarDataSet(CAM_COL_VARS_FOLLOW_HELI_NEAR_RANGE+nCarZoom-1);
			break;
		case FOLLOW_CAR_INPLANE :
			SetCamCollisionVarDataSet(CAM_COL_VARS_FOLLOW_PLANE_NEAR_RANGE+nCarZoom-1);
			break;
		case FOLLOW_CAR_RCCAR :
			SetCamCollisionVarDataSet(CAM_COL_VARS_FOLLOW_RCCAR_NEAR_RANGE+nCarZoom-1);
			break;		
		case FOLLOW_CAR_RCHELI :
			SetCamCollisionVarDataSet(CAM_COL_VARS_FOLLOW_RCHELI_NEAR_RANGE+nCarZoom-1);
			break;				
		default :
			ASSERT(false);
			break;
	}
}

//--------------------------------------------------------------------------------
// Generic special cases for all modes - applied first
void CCamera::CameraGenericModeSpecialCases(CPed *pPed)
{
	m_numExtrasEntitysToIgnore = 0; // reset

	if (!pPed)
		return;
		
	// Ignore objects being held... for all camera modes.
	CTaskSimpleHoldEntity *pTaskHold = pPed->GetPedIntelligence()->GetTaskHold();
	if (pTaskHold)
	{
		CEntity *pEnt = pTaskHold->GetEntityBeingHeld();		
		if (pEnt)
		{
			m_pExtrasEntitysToIgnore[m_numExtrasEntitysToIgnore++] = pEnt;
			ASSERT(m_numExtrasEntitysToIgnore<=MAX_CAMERA_EXTRA_ENTITIES_TO_IGNORE);	
		}
	}
}

//--------------------------------------------------------------------------------
// for ped mode we can case mode specific special cases
void CCamera::CameraPedModeSpecialCases(CPed *pPed)
{
	CCollision::bCamCollideWithVehicles = true; // Reset each frame.
	CCollision::bCamCollideWithObjects 	= true;	
	CCollision::bCamCollideWithPeds     = true;
}

//--------------------------------------------------------------------------------
// for ped aim mode we can case mode specific special cases
void CCamera::CameraPedAimModeSpecialCases(CPed *pPed)
{
	CCollision::bCamCollideWithVehicles = true; // Reset each frame.
	CCollision::bCamCollideWithObjects 	= true;	
	CCollision::bCamCollideWithPeds     = true;

	// If we in a vehicle (i.e. doing driveby, need to ignore our vehicle)
	if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle)
	{
		m_pExtrasEntitysToIgnore[m_numExtrasEntitysToIgnore++] = pPed->m_pMyVehicle;
		ASSERT(m_numExtrasEntitysToIgnore<=MAX_CAMERA_EXTRA_ENTITIES_TO_IGNORE);	
	}			
}

//--------------------------------------------------------------------------------
// for vehicle mode we can case mode specific special cases
void CCamera::CameraVehicleModeSpecialCases(CVehicle *pVehicle)
{
	static float SPEED_FOR_CAMERA_TO_IGNORE_VECHICLES	= 0.2f;	
	CCollision::bCamCollideWithObjects 	= false;

	// If travelling really fast ignore other vehicle collisions	
	float speedSqr = pVehicle->GetMoveSpeed().MagnitudeSqr(); // metres per frame
	if (speedSqr > SPEED_FOR_CAMERA_TO_IGNORE_VECHICLES*SPEED_FOR_CAMERA_TO_IGNORE_VECHICLES)
	{
		CCollision::relVelCamCollisionVehiclesSqr = FAST_CAM_REL_VEL_VEHICLE_COLLISION_SQR;
		CCollision::bCamCollideWithVehicles = true; // trying to implement a relative speed system
		CCollision::bCamCollideWithPeds     = false;
		CCollision::bCamCollideWithObjects 	= false; // vehicles dont collide with objects for now... but when we get flags into object.dat we will.
	}
	else
	{
		CCollision::relVelCamCollisionVehiclesSqr = DEFAULT_CAM_REL_VEL_VEHICLE_COLLISION_SQR;// if travelling at low speeds all cars should be collided with... might need tweaked? - DW
		CCollision::bCamCollideWithVehicles = true; // Reset each frame.		
		CCollision::bCamCollideWithPeds     = true;
		CCollision::bCamCollideWithObjects 	= true; // vehicles dont collide with objects for now... but when we get flags into object.dat we will.
	}	
	
	// If we are trailing a vehicle set to ignore this too!
	CEntity *pTrailer = pVehicle->m_pVehicleBeingTowed;
	if (pTrailer)
	{
		m_pExtrasEntitysToIgnore[m_numExtrasEntitysToIgnore++] = pTrailer;
		ASSERT(m_numExtrasEntitysToIgnore<=MAX_CAMERA_EXTRA_ENTITIES_TO_IGNORE);	
	}			
}

//--------------------------------------------------------------------------------
// Is this entity one we also wish to ignore from our per frame list?
bool CCamera::IsExtraEntityToIgnore(CEntity *pEntity)
{
	for (int32 i=0;i<m_numExtrasEntitysToIgnore;i++)
		if (m_pExtrasEntitysToIgnore[i] == pEntity)
			return true;
	
	return false;
}


#endif // DW_CAMERA_COLLISION








//============================
// Ducking code

//--------------------------------------------------------------------------------
bool CCamera::ConsiderPedAsDucking(CPed *pPed)
{
	CTaskSimpleDuck *pTask = pPed->GetPedIntelligence()->GetTaskDuck();	
	return (pTask && pPed->m_nPedFlags.bIsDucking && !pTask->GetIsAborting());		
}

static float gDuckBugFixLo 	= 0.3f; // was ducking the camera too low when you back up against collision, all you would see is the players back.
static float gDuckBugFixMed = 0.3f;
			

//-------------------------------------------------------------------------------
void CCamera::ResetDuckingSystem(CPed *pPed)
{	
	m_duckZMod 		= CAM_DUCK_Z_MOD_HI;
	m_duckZMod_Aim 	= CAM_DUCK_Z_MOD_HI_AIM;

	if(pPed && ConsiderPedAsDucking(pPed))		
	{
		float speedSqr = pPed->GetMoveSpeed().MagnitudeSqr();

		{
			bool bPedIsMoving = (speedSqr > CAM_DUCK_Z_MOVE_TOL_SQR);
		
			if (bPedIsMoving)	m_duckZMod = CAM_DUCK_Z_MOD_MED + gDuckBugFixMed;
			else				m_duckZMod = CAM_DUCK_Z_MOD_LOW + gDuckBugFixLo;
		}
		{
			bool bPedIsMoving = (speedSqr > CAM_DUCK_Z_MOVE_TOL_SQR_AIM);
		
			if (bPedIsMoving)	m_duckZMod_Aim = CAM_DUCK_Z_MOD_MED_AIM;//+ gDuckBugFixMed;
			else				m_duckZMod_Aim = CAM_DUCK_Z_MOD_LOW_AIM;//+ gDuckBugFixLo;
		}
	}
}

//------------------------------------------------------------------------
// Move camera up/down based on ducking state
void CCamera::HandleCameraMotionForDucking(CPed *pPed, CVector *pSource, CVector *pTarget, bool bIsLookingBehind)
{			
	CTaskSimpleDuck *pTask = pPed->GetPedIntelligence()->GetTaskDuck();		
	float duckZModTo = CAM_DUCK_Z_MOD_HI;
	
	// Work out where we are going to
	if (ConsiderPedAsDucking(pPed))
	{		
		bool bPedIsMoving = (pPed->GetMoveSpeed().MagnitudeSqr() > CAM_DUCK_Z_MOVE_TOL_SQR);
		
		if (bPedIsMoving)	duckZModTo = CAM_DUCK_Z_MOD_MED + gDuckBugFixMed;
		else				duckZModTo = CAM_DUCK_Z_MOD_LOW + gDuckBugFixLo;
	}

	
	float speed = CTimer::GetTimeStep() * CAM_DUCK_Z_SPEED;
	float delta = duckZModTo - m_duckZMod;

	if (!bIsLookingBehind) // when looking behind this (effective GLOBAL) is already being interpolated ( as look behind and normal camera are both processed in the one frame) 
		m_duckZMod += delta * speed;
	
	if (pSource)	
		pSource->z += m_duckZMod;
		
	if (pTarget)
		pTarget->z += m_duckZMod;
}


//------------------------------------------------------------------------
// Move camera up/down based on ducking state
// For aiming weapon
void CCamera::HandleCameraMotionForDuckingDuringAim(CPed *pPed, CVector *pSource, CVector *pTarget, bool bIsLookingBehind)
{			
	CTaskSimpleDuck *pTask = pPed->GetPedIntelligence()->GetTaskDuck();		
	float duckZModTo = CAM_DUCK_Z_MOD_HI_AIM;
	
	// Work out where we are going to
	if (ConsiderPedAsDucking(pPed))
	{		
		bool bPedIsMoving = (pPed->GetMoveSpeed().MagnitudeSqr() > CAM_DUCK_Z_MOVE_TOL_SQR_AIM);
		
		if (bPedIsMoving)	duckZModTo = CAM_DUCK_Z_MOD_MED_AIM;// + gDuckBugFixMed; // its a good idea to hve these values the same else the aiming will go off when you shoot.
		else				duckZModTo = CAM_DUCK_Z_MOD_LOW_AIM;// + gDuckBugFixLo; // its a good idea to hve these values the same else the aiming will go off when you shoot.
	}
	
	float speed = CTimer::GetTimeStep() * CAM_DUCK_Z_SPEED_AIM;
	float delta = duckZModTo - m_duckZMod_Aim;
	if (!bIsLookingBehind) // when looking behind this (effective GLOBAL) is already being interpolated ( as look behind and normal camera are both processed in the one frame) 
		m_duckZMod_Aim += delta * speed;

	if (pSource)	
		pSource->z += m_duckZMod_Aim;
		
	if (pTarget)
		pTarget->z += m_duckZMod_Aim;
}

// End ducking code
//============================



//============================
// Start New Camera Script commands

// The camera will never revert back to other scripted commands
// it will persist and look at where it was looking last frame unless
// calling a post process scripted effect.

//------------------------------------------------------------------------
// CAMERA_SET_VECTOR_TRACK
void CCamera::VectorMoveLinear( CVector *pTo, CVector *pFrom, float time, bool bSetSmoothEnds)      	
{
	ASSERT(time>0.0f);
	
	m_vectorMoveStartTime 		= CTimer::GetTimeInMilliseconds();
	m_vectorMoveEndTime 		= m_vectorMoveStartTime + time;
	m_vectorMoveFrom 			= *pFrom;
	m_vectorMoveTo 				= *pTo;
	m_bVectorMoveSmoothEnds    = bSetSmoothEnds;
}

//------------------------------------------------------------------------
// CAMERA_SET_VECTOR_TRACK
void CCamera::VectorTrackLinear( CVector *pTo, CVector *pFrom, float time, bool bSetSmoothEnds)      	
{
	ASSERT(time>0.0f);
	
	m_vectorTrackStartTime 		= CTimer::GetTimeInMilliseconds();
	m_vectorTrackEndTime 		= m_vectorTrackStartTime + time;
	m_vectorTrackFrom 			= *pFrom;
	m_vectorTrackTo 			= *pTo;
	m_bVectorTrackSmoothEnds    = bSetSmoothEnds;
}

//------------------------------------------------------------------------
// CAMERA_SET_SHAKE_SIMULATION
void CCamera::AddShake(float durationMs, float scaleShake,float twitchFreq, float twitchScale, float dummy)//float timeSway, float angleSway, float time, float fadeTimer, bool bSetSmoothEnds)					
{
	ASSERT(durationMs>0.0f);
	
	AddShakeSimple(durationMs,1,1.0f);
/*	
	m_DegreeHandShake	= scaleShake;		
	CHandShaker *pS 	= &gHandShaker[1];		
	pS->twitchFreq		= twitchFreq;			
	pS->twitchVel		= twitchScale;			
	m_shakeStartTime 	= CTimer::GetTimeInMilliseconds();
	m_shakeEndTime 		= m_shakeStartTime + durationMs;*/
}

void CCamera::AddShakeSimple(float durationMs,int32 shakeCamId, float degreeOfShake)
{
	ASSERT(shakeCamId<MAX_HANDSHAKERS);

	m_DegreeHandShake = degreeOfShake;
	m_CurShakeCam     = shakeCamId;
	m_shakeStartTime 	= CTimer::GetTimeInMilliseconds();
	m_shakeEndTime 		= m_shakeStartTime + durationMs;	
}

//------------------------------------------------------------------------
//CAMERA_SET_LERP_FOV
void CCamera::LerpFOV(float from, float to, float time, bool bSetSmoothEnds)			
{
	ASSERT(time>0.0f);
	
	m_FOVLerpStartTime 	= CTimer::GetTimeInMilliseconds();
	m_FOVLerpEndTime   	= m_FOVLerpStartTime + time;
	m_FOVLerpStart 		= from;
	m_FOVLerpEnd   		= to;
	m_bFOVLerpSmoothEnds= bSetSmoothEnds;	
}

//-------------------------------------------------
// Initialise Script controling members for safety
// also switches off scriptable commands!
void CCamera::InitialiseScriptableComponents()
{
	m_vectorTrackStartTime 	= -1;
	m_vectorTrackEndTime    = -1;
	
	m_vectorMoveStartTime 	= -1;
	m_vectorMoveEndTime    = -1;	
	
	m_shakeStartTime		= -1;
	m_shakeEndTime			= -1;	
	
	m_FOVLerpEndTime 		= -1;
	m_FOVLerpStartTime		= -1;
	
	m_FOVLerpStart			= 0;
	m_FOVLerpEnd			= 0;
	
	m_bVectorTrackSmoothEnds= true;
	m_bFOVLerpSmoothEnds    = true;	
	m_bVectorMoveSmoothEnds	= true;
	
	m_vectorMoveStartTime 	= -1;
	m_vectorMoveEndTime 	= -1;
		
	m_bPersistFOV 			= false;
	m_bPersistCamPos 		= false;
	m_bPersistCamLookAt 	= false;	
	
	m_bVectorTrackScript    = false;
	m_bVectorMoveScript 	= false;
	m_bFOVScript 			= false;
}

//-----------------------------------------------------
// Process Camera script command
void CCamera::ProcessVectorTrackLinear()
{
	float curTime = CTimer::GetTimeInMilliseconds();
	if (curTime <= m_vectorTrackEndTime)
	{
		float t = (curTime - m_vectorTrackStartTime) / (m_vectorTrackEndTime - m_vectorTrackStartTime);
		ASSERT(ISNORMALISEDVAL(t));
		ProcessVectorTrackLinear(t);
	}
	else if (m_bPersistCamLookAt)
		m_bVectorTrackScript = true;
}

//-----------------------------------------------------
// Process Camera script command
void CCamera::ProcessVectorMoveLinear()
{
	float curTime = CTimer::GetTimeInMilliseconds();
	if (curTime <= m_vectorMoveEndTime)
	{
		float t = (curTime - m_vectorMoveStartTime) / (m_vectorMoveEndTime - m_vectorMoveStartTime);
		ASSERT(ISNORMALISEDVAL(t));
		ProcessVectorMoveLinear(t);
	}
	else if (m_bPersistCamPos)
		m_bVectorMoveScript = true;
}

//-----------------------------------------------------
// Process Camera script command
void CCamera::ProcessShake()
{
	float curTime = CTimer::GetTimeInMilliseconds();	
	if (curTime <= m_shakeEndTime)
	{
		float t = (curTime - m_shakeStartTime) / (m_shakeEndTime - m_shakeStartTime);
		ASSERT(ISNORMALISEDVAL(t));
		ProcessShake(t);
	}
}

//-----------------------------------------------------
// Process Camera script command
void CCamera::ProcessFOVLerp()
{
	float curTime = CTimer::GetTimeInMilliseconds();
	if (curTime <= m_FOVLerpEndTime)
	{
		float t = (curTime - m_FOVLerpStartTime) / (m_FOVLerpEndTime - m_FOVLerpStartTime);
		ASSERT(ISNORMALISEDVAL(t));
		ProcessFOVLerp(t);
	}
	else if (m_bPersistFOV)
		m_bFOVScript = true;
}

//------------------------------
// Process Camera script command
void CCamera::ProcessVectorTrackLinear(float t)
{	
	m_bVectorTrackScript = true;
	
	if (m_bVectorTrackSmoothEnds)
	{
		m_VectorTrackScript.x = DW_SINE_ACCEL_DECEL_LERP(t,m_vectorTrackFrom.x,m_vectorTrackTo.x);
		m_VectorTrackScript.y = DW_SINE_ACCEL_DECEL_LERP(t,m_vectorTrackFrom.y,m_vectorTrackTo.y);
		m_VectorTrackScript.z = DW_SINE_ACCEL_DECEL_LERP(t,m_vectorTrackFrom.z,m_vectorTrackTo.z);
	}
	else
	{
		m_VectorTrackScript.x = DW_LERP(t,m_vectorTrackFrom.x,m_vectorTrackTo.x);
		m_VectorTrackScript.y = DW_LERP(t,m_vectorTrackFrom.y,m_vectorTrackTo.y);
		m_VectorTrackScript.z = DW_LERP(t,m_vectorTrackFrom.z,m_vectorTrackTo.z);
	}
}


//------------------------------
// Process Camera script command
void CCamera::ProcessVectorMoveLinear(float t)
{	
	m_bVectorMoveScript = true;
	
	if (m_bVectorMoveSmoothEnds)
	{
		m_VectorMoveScript.x = DW_SINE_ACCEL_DECEL_LERP(t,m_vectorMoveFrom.x,m_vectorMoveTo.x);
		m_VectorMoveScript.y = DW_SINE_ACCEL_DECEL_LERP(t,m_vectorMoveFrom.y,m_vectorMoveTo.y);
		m_VectorMoveScript.z = DW_SINE_ACCEL_DECEL_LERP(t,m_vectorMoveFrom.z,m_vectorMoveTo.z);
	}
	else
	{
		m_VectorMoveScript.x = DW_LERP(t,m_vectorMoveFrom.x,m_vectorMoveTo.x);
		m_VectorMoveScript.y = DW_LERP(t,m_vectorMoveFrom.y,m_vectorMoveTo.y);
		m_VectorMoveScript.z = DW_LERP(t,m_vectorMoveFrom.z,m_vectorMoveTo.z);
	}
}

//------------------------------
// Process Camera script command
void CCamera::ProcessFOVLerp(float t)
{
	m_bFOVScript = true;
	if (m_bFOVLerpSmoothEnds)
		m_MyFOV = DW_SINE_ACCEL_DECEL_LERP(t,m_FOVLerpStart,m_FOVLerpEnd);
	else
		m_MyFOV = DW_LERP(t,m_FOVLerpStart,m_FOVLerpEnd);
}

//------------------------------
// Process Camera script command
void CCamera::ProcessShake(float time)
{	
	CCam *pCam = &Cams[ActiveCam];
		
	// dirty initialisation for an already dirty camera system - 3 hours sleep so cant function or navigate through this legacy of mess today.
	static bool gInitShakeCams = false;
	if (!gInitShakeCams) 
	{
		gInitShakeCams = true;
		gHandShaker[1].lim 					= CVector(0.02f,0.02f,0.01f);	
		gHandShaker[1].motion				= CVector(0.0002f,0.0002f,0.0001f);
		gHandShaker[1].slow 				= CVector(1.3f,1.3f,1.4f);

		gHandShaker[1].scaleReactionMin 	= 0.3f;			// a scale to modify the degree of reaction based on how angularily far we are away from it.
		gHandShaker[1].scaleReactionMax 	= 1.0f;			
		gHandShaker[1].twitchFreq 			= 15;			// frequency of twitching
		gHandShaker[1].twitchVel 			= 0.001f;		// amount of twitch						


		gHandShaker[2].lim 					= CVector(0.02f,0.02f,0.04f);	
		gHandShaker[2].motion				= CVector(0.0002f,0.0002f,0.0001f);
		gHandShaker[2].slow 				= CVector(1.3f,1.3f,1.4f);

		gHandShaker[2].scaleReactionMin 	= 0.3f;			// a scale to modify the degree of reaction based on how angularily far we are away from it.
		gHandShaker[2].scaleReactionMax 	= 1.0f;			
		gHandShaker[2].twitchFreq 			= 20;			// frequency of twitching
		gHandShaker[2].twitchVel 			= 0.001f;		// amount of twitch						


		gHandShaker[3].lim 					= CVector(0.02f,0.02f,0.01f);	
		gHandShaker[3].motion				= CVector(0.0002f,0.0002f,0.0001f);
		gHandShaker[3].slow 				= CVector(1.3f,1.3f,1.4f);

		gHandShaker[3].scaleReactionMin 	= 0.3f;			// a scale to modify the degree of reaction based on how angularily far we are away from it.
		gHandShaker[3].scaleReactionMax 	= 1.0f;			
		gHandShaker[3].twitchFreq 			= 10;			// frequency of twitching
		gHandShaker[3].twitchVel 			= 0.0005f;		// amount of twitch						


		gHandShaker[4].lim 					= CVector(0.02f,0.02f,0.01f);	
		gHandShaker[4].motion				= CVector(0.0002f,0.0002f,0.0001f);
		gHandShaker[4].slow 				= CVector(1.3f,1.3f,1.4f);

		gHandShaker[4].scaleReactionMin 	= 0.3f;			// a scale to modify the degree of reaction based on how angularily far we are away from it.
		gHandShaker[4].scaleReactionMax 	= 1.0f;			
		gHandShaker[4].twitchFreq 			= 20;			// frequency of twitching
		gHandShaker[4].twitchVel 			= 0.002f;		// amount of twitch						


		gHandShaker[5].lim 					= CVector(0.02f,0.02f,0.01f);	
		gHandShaker[5].motion				= CVector(0.0002f,0.0002f,0.0001f);
		gHandShaker[5].slow 				= CVector(1.3f,1.3f,1.4f);

		gHandShaker[5].scaleReactionMin 	= 0.3f;			// a scale to modify the degree of reaction based on how angularily far we are away from it.
		gHandShaker[5].scaleReactionMax 	= 1.0f;			
		gHandShaker[5].twitchFreq 			= 2;			// frequency of twitching
		gHandShaker[5].twitchVel 			= 0.003f;		// amount of twitch						
	}
		
	CHandShaker *pS = &gHandShaker[m_CurShakeCam];		
	pS->Process(m_DegreeHandShake);		
	float roll = pS->ang.z * m_DegreeHandShake; // might be z ???
	CVector temp = Multiply3x3(pCam->Front,pS->resultMat);
	pCam->Front = temp;	
	pCam->Front.Normalise();	

	pCam->Up=CVector(CMaths::Sin(roll),0.0f,CMaths::Cos(roll));
	CVector TempRight= CrossProduct(pCam->Front, pCam->Up);
	TempRight.Normalise();
	pCam->Up=CrossProduct(TempRight, pCam->Front);

	if ((pCam->Front.x==0.0f) && (pCam->Front.y==0.0f))
	{
		pCam->Front.x=0.0001f;
		pCam->Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( pCam->Front, pCam->Up );
	TempRight.Normalise();
	pCam->Up = CrossProduct(TempRight, pCam->Front );		
}	

//-------------------------------------------------
// Go through DW scripted commands and process them
void CCamera::ProcessScriptedCommands()
{
	ProcessVectorMoveLinear();
	ProcessVectorTrackLinear();
	ProcessFOVLerp();
}


#ifndef MASTER
//--------------------------------------------------
// Test harness for scriptable commands - DW
static CVector gTestVectorTrackTo(2489,-1667,13);
static CVector gTestVectorTrackFrom(2489,-1667,14);
static float   gTestVectorTrackLerpTime = 10000.0f; // ms
static bool    gbFireScriptCommand1 = false;
static bool    gbFireScriptCommand2 = false;
static bool    gbFireScriptCommand3 = false;
static float   gTestShakeScale 		= 100.0f;
static float   gTestShakeTwitchVel 	= 0.00005f;
static float   gTestShakeTwitchFreq = 15;
static float   gTestShakeTime   = 300000.00f;
static float   gTestFOVTime   	= 10.0f;
static float   gTestFOVFrom   	= 45.0f;
static float   gTestFOVTo   	= 60.0f;

void CCamera::ProcessTestBed()
{
	if (gbFireScriptCommand1)
		TheCamera.VectorTrackLinear(&gTestVectorTrackTo, &gTestVectorTrackFrom, gTestVectorTrackLerpTime, true);
	if (gbFireScriptCommand2)
//	TheCamera.AddShake(gTestShakeTime, float scaleShake,float twitchFreq, float twitchScale, float dummy)//float timeSway, float angleSway, float time, float fadeTimer, bool bSetSmoothEnds)					

		TheCamera.AddShake(gTestShakeTime, gTestShakeScale, gTestShakeTwitchFreq, gTestShakeTwitchVel, 0);					
	if (gbFireScriptCommand3)
		TheCamera.LerpFOV(gTestFOVFrom, gTestFOVTo, gTestFOVTime,true);		
		
	gbFireScriptCommand1 = gbFireScriptCommand2 = gbFireScriptCommand3 = false;


	static float degree = 100.0f;
	static bool g1 = false;
	if (g1)	
		TheCamera.AddShakeSimple(999999,1,degree);
	static bool g2 = false;
	if (g2)	
		TheCamera.AddShakeSimple(999999,2,degree);
	static bool g3 = false;
	if (g3)	
		TheCamera.AddShakeSimple(999999,3,degree);
	static bool g4 = false;
	if (g4)	
		TheCamera.AddShakeSimple(999999,4,degree);
	static bool g5 = false;
	if (g5)	
		TheCamera.AddShakeSimple(999999,5,degree);
	static bool g6 = false;
	if (g6)	
		TheCamera.AddShakeSimple(999999,6,degree);

	g1=g2=g3=g4=g5=g6=false;
}
#endif

/*
TheCamera.InitialiseScriptableComponents();	// initialise whole systems
*/

// End New Camera Script commands
//============================

float CCamera::GetRoughDistanceToGround()
{
	float heightOfGround = CalculateGroundHeight();
	float heightOfCam    = Cams[ActiveCam].Source.z;
	return heightOfCam - heightOfGround;
}


float GetNearestDistanceOfPedSphereToCameraNearClip(CPed *pPed)
{
	// This will work as long as there is a separating planbe pbetween the camera and the spheres
	/// not good when a sphere lies behind the plane... -ve result.
	float nearestDist = 999999.0f;
	CPedModelInfo *pModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(pPed->GetModelIndex());
	pModelInfo->AnimatePedColModelSkinnedWorld((RpClump *)pPed->m_pRwObject);
	CColModel *pColModel = pModelInfo->GetHitColModel();
	CCollisionData* pColData = pColModel->GetCollisionData();

	CCam *pCam = &TheCamera.Cams[TheCamera.ActiveCam];

	float planeD = DotProduct(pCam->Front,pCam->Source);	
	float nearClip = RwCameraGetNearClipPlane(Scene.camera);	
	for (int32 i=0;i<NUM_PED_COLMODEL_NODES;i++)
	{
		CColSphere *pSphere = &pColData->m_pSphereArray[i];
		float pdtp = DotProduct(pSphere->m_vecCentre, pCam->Front) - planeD;
		pdtp -= pSphere->m_fRadius;
		pdtp -= nearClip;
		if (pdtp<nearestDist)
			nearestDist = pdtp;
	}
	
	return nearestDist;	
}


//---------------------------------------------------------------------------------------------------------
// DW - a multipurpose attempt to improve near clip distance ( overrides whatever the game thinks it doing...usually )
void CCamera::ImproveNearClip(CVehicle *pVehicle,CPed *pPed,CVector *pSource,CVector *pTarget)
{
	static float gDistToImproveNonVerticalClip = 10.0f;
	static float gNonVerticalNearClipTarget    = 1.0f;

	static float gFlyingVehicleNearClip 	= 5.0f;		// a target near clip to achieve if possible
	static float gFlyingPedNearClip 		= 2.0f;		// a target near clip to achieve if possible
	static float gScaleOfDistToVehicleClip 	= 0.1f;		// the percentage (0-1) of distance to vehicle which will scale the near clip distance ( thus is resides on a sliding scale for smooth clip changes etc. ) 
	static float gScaleOfDistToPedClip 		= 0.3f;		// the percentage (0-1) of distance to vehicle which will scale the near clip distance ( thus is resides on a sliding scale for smooth clip changes etc. ) 
	static float gIgnoreThisCollisionDist 	= 0.3f;		// when collision occurs below this pecentile(0-1) distance then we cease trying to smart with clipping, because we think we might end up clipping the vehicle out.
	static float gDist2GroundHeightTrigger  = 10.0f;	// when this height above the ground we switch on aggresive clipping

	// general purpose - far away from our target and not too bad collision... push out clip

	float dist = (*pSource - *pTarget).Magnitude();
	if (dist>gDistToImproveNonVerticalClip)
	{
		float curClip = RwCameraGetNearClipPlane(Scene.camera);	
		float proposedClip = gCurDistForCam * gNonVerticalNearClipTarget;
		if (proposedClip > curClip) 
		{
			RwCameraSetNearClipPlane(Scene.camera, proposedClip);
		}	
	}	

	if (pVehicle)
	{		
		if (pVehicle->GetVehicleType()==VEHICLE_TYPE_PLANE ||
			pVehicle->GetVehicleType()==VEHICLE_TYPE_HELI)
		{
			if (gCurDistForCam>gIgnoreThisCollisionDist)
			{
				float h = GetRoughDistanceToGround();
				if (h>gDist2GroundHeightTrigger)
				{								
					float curClip = RwCameraGetNearClipPlane(Scene.camera);			
					float dist = (*pSource - *pTarget).Magnitude();
					float proposedClip = gFlyingVehicleNearClip*gCurDistForCam;
					float distToVehicleClip = dist*gScaleOfDistToVehicleClip;
					proposedClip = CMaths::Min(distToVehicleClip,proposedClip);
											
					if (proposedClip > curClip) 
					{
						RwCameraSetNearClipPlane(Scene.camera, proposedClip);
					}
				}
			}	
			else if (pVehicle->GetVehicleType()==VEHICLE_TYPE_HELI)
			{
				static float gCloseHeliClip = 0.1f;
				RwCameraSetNearClipPlane(Scene.camera, gCloseHeliClip);
			}													
		}
	}
	else if (pPed)
	{
		if (!pPed->GetIsStanding())
		{
			bool bIsParachuting = pPed->GetPedIntelligence()->GetUsingParachute();//(pPed->m_WeaponSlots[WEAPONSLOT_TYPE_PARACHUTE].m_eWeaponType == WEAPONTYPE_PARACHUTE);
//			bool bClimbing = false;
			
/*			
			if(pPed->GetPedIntelligence()->GetTaskActiveSimplest())
			{ 
//				bClimbing = (pPed->GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_CLIMB);
				bIsSwimming = (//GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_SWIM);
			}
*/			
			CTaskSimpleSwim *pSwimTask = pPed->GetPedIntelligence()->GetTaskSwim();
			if (pSwimTask)
			{
				bool bUnderWater = pSwimTask->m_nSwimState == SWIM_UNDERWATER;
				float waterZ = 0.0f;
				bool bWater = CWaterLevel::GetWaterLevel(pSource->x,pSource->y,pSource->z, &waterZ, false);
				float waterSurfaceDelta = waterZ - pSource->z;
				static float gWaterSurfaceDeltaTolerance = 0.3f;
				static float gSwimClipCloseToWater 	= 0.10f;
				static float gSwimClipUnderWater 	= 0.10f;
				float absWaterDelta  = CMaths::Abs(waterSurfaceDelta);
				bool bReallyCloseToWater = absWaterDelta<gWaterSurfaceDeltaTolerance;
				
				
				
			//	RwCameraSetNearClipPlane(Scene.camera, 0.05f); // clipping with boats can be bad, keep it fixed
				
				if (bWater && bReallyCloseToWater) // for making the clipping with water as subtle as possible
				{
					RwCameraSetNearClipPlane(Scene.camera, gSwimClipCloseToWater);
				}
				else if (bUnderWater && TheCamera.m_nPedZoom==ZOOM_ONE) // for prevent clipping with feet
					RwCameraSetNearClipPlane(Scene.camera, gSwimClipUnderWater);
			}
			else if (bIsParachuting || pPed->GetPedIntelligence()->GetTaskJetPack())			
			{
				float h = GetRoughDistanceToGround();
				if (h>gDist2GroundHeightTrigger)
				{
					float curClip = RwCameraGetNearClipPlane(Scene.camera);			
					float dist = (*pSource - *pTarget).Magnitude();			
					float proposedClip = gFlyingPedNearClip*gCurDistForCam;				
					float distToPedClip = dist*gScaleOfDistToPedClip;
					proposedClip = CMaths::Min(distToPedClip,proposedClip);
											
					if (proposedClip > curClip) 
					{
						RwCameraSetNearClipPlane(Scene.camera, proposedClip);
					}							
				}
			}
		}
		else
		{
			// Based on the distance of collision we define the nearclip to be inside the current sphere size at this location			
			float radiusOfCameraUnderNoCollision = gLastRadiusUsedInCollisionPreventionOfCamera;
			float fov = TheCamera.Cams[TheCamera.ActiveCam].FOV*0.5f;
			float ang = 90.0f - fov;
			float furthestClip = radiusOfCameraUnderNoCollision * CMaths::Sin(DEGTORAD(ang));

			// This will work as long as there is a separating planbe pbetween the camera and the spheres
			/// not good when a sphere lies behind the plane... -ve result.
			float nearestDist = 999999.0f;
			CPedModelInfo *pModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(pPed->GetModelIndex());
			pModelInfo->AnimatePedColModelSkinnedWorld((RpClump *)pPed->m_pRwObject);
			CColModel *pColModel = pModelInfo->GetHitColModel();
			CCollisionData* pColData = pColModel->GetCollisionData();

			CCam *pCam = &TheCamera.Cams[ActiveCam];

			float planeD = DotProduct(pCam->Front,pCam->Source);	
			for (int32 i=0;i<NUM_PED_COLMODEL_NODES;i++)
			{
				CColSphere *pSphere = &pColData->m_pSphereArray[i];
				float pdtp = DotProduct(pSphere->m_vecCentre, pCam->Front) - planeD;
				pdtp -= pSphere->m_fRadius;
				
				if (pSphere->m_data.m_nPieceType == PED_SPHERE_HEAD) // make the head bugger to compensate for afros
				{
					static float gAfroScaler = 1.0f;
					pdtp -= pSphere->m_fRadius*gAfroScaler; // give it more radius
				}
				
				if (pdtp<nearestDist)
					nearestDist = pdtp;
			}
			
			static float nearestMinClipDist 		= 0.02f;

			if (nearestDist>furthestClip)
				nearestDist = furthestClip;	
			else if (nearestDist<nearestMinClipDist)
				nearestDist = nearestMinClipDist;

			static float neverMoreThanThisClip 		= 0.3f; // used to be a very nice 0.54, but reduce to 0.3 for nicer ped clipping
			if (nearestDist>neverMoreThanThisClip)
				nearestDist = neverMoreThanThisClip;	


			nearestDist *= 100.0f;
			int32 nd = nearestDist;
			nearestDist = (float)nd / 100.0f;

			RwCameraSetNearClipPlane(Scene.camera, nearestDist);													
		}
	}	
	
	// PED COLLISION
	// moved since I need to know what the near clip is presently set to also
	// Once we know where exactly the camera is going to we can do checks at this position to see excatly what it finally intersects....
	{
		#define RESET_NEAREST_DIST 999999.0f // big number
		float nearestDist = RESET_NEAREST_DIST;
		CCollision::CheckPeds(pSource,&Cams[ActiveCam].Front,&nearestDist);			
//	if (nearestDist != RESET_NEAREST_DIST && !bClippingIsSetNear)
//			SetNearClipBasedOnPedCollision(nearestDist);
	}	
}

// --------  NEW CINEY CAM STUFF -------------
// --------  NEW CINEY CAM STUFF -------------
// --------  NEW CINEY CAM STUFF -------------
// --------  NEW CINEY CAM STUFF -------------
// --------  NEW CINEY CAM STUFF -------------
// --------  NEW CINEY CAM STUFF -------------


//-----------------------------------------------------------------------------------------------------------
// Duplicated code for the millionth time, but want to be sure the camera is being built properly... too much
// fluff in the code.
// This function is the last bit of work the camera does to perform and finalise its cinematic use.
// Whatever I say goes... dont interfere ...the rest of the whole game can bog off... I am in sole control of the camera.
// .. no sub modes, no transitions, no overrides, no history dependance etc etc.

// Dont trust the camera to preserve these values so I cache them myself.

CVector DWCineyCamLastPos; 
CVector DWCineyCamLastUp; 
CVector DWCineyCamLastRight; 
CVector DWCineyCamLastFwd; 
float   DWCineyCamLastFov;
float   DWCineyCamLastNearClip;

#define NUM_DW_CINEY_CAMS 9

int32   gLastFrameProcessedDWCineyCam = -1; // make the DW ciney cams know when to reset themselves.
int32   gDWCineyCamStartTime = -1;
int32   gDWCineyCamEndTime = -1;
int32 	gDWCamMaxDurationMs[NUM_DW_CINEY_CAMS] 	= {20000,	10000,	5000,	10000,	1,		1,		12000,	7000,	5000};
float 	gMovieCamMinDist[NUM_DW_CINEY_CAMS] 	= {3.0f,	3.0f,	1.0f,	3.0f,	5.0f,	3.0f,	3.0f,	3.0f,	3.0f};
float 	gMovieCamMaxDist[NUM_DW_CINEY_CAMS] 	= {185.0f,	100.0f,	100.0f,	100.0f,	30.0f,	30.0f,	100.0f,	100.0f,	100.0f};
bool    gbExitCam[NUM_DW_CINEY_CAMS] 			= {false,	false,	false,	false,	false,	false,	false,	false,	false};

//---------------------------------------------------------------------------------------------------
void CCam::GetCoreDataForDWCineyCamMode(CEntity **pEntity, CVehicle **pVehicle, 
										CVector *dst, CVector *src, 
										CVector *targetUp, CVector *targetRight,CVector *targetForward,
										CVector *targetMotion, float *targetVel, 
										CVector *targetAngMotion, float *targetAngVel,
										CColSphere *sph)
{	
	*pEntity 		= CamTargetEntity;
	*pVehicle 		= (CVehicle*)*pEntity;
	*dst 			= (*pVehicle)->GetPosition();
	*src 			= DWCineyCamLastPos; // where we where last frame by default.

	*targetUp 		= (*pVehicle)->GetUp();
	*targetRight 	= (*pVehicle)->GetRight();
	*targetForward 	= (*pVehicle)->GetForward();

	*targetMotion	= (*pVehicle)->GetMoveSpeed();		
	*targetVel		= targetMotion->Magnitude();// * CTimer::GetTimeStep();

	*targetAngMotion= (*pVehicle)->GetTurnSpeed();		
	*targetAngVel	= targetAngMotion->Magnitude();

	sph->Set((*pVehicle)->GetBoundRadius(),(*pVehicle)->GetBoundCentre());	
}

//---------------------------------------------------------------------------------------------------
void CCam::CacheLastSettingsDWCineyCam()
{
	DWCineyCamLastUp 		= Up;
	CVector TempRight		= CrossProduct(Front, Up);
	DWCineyCamLastRight 	= TempRight;
	DWCineyCamLastFwd  		= Front;
	DWCineyCamLastFov		= FOV;
	DWCineyCamLastNearClip	= RwCameraGetNearClipPlane(Scene.camera);
	DWCineyCamLastPos		= Source;
}

//-----------------------------------------------------------------------------------
void CHandShaker::SetDefaults()
{
	lim 	= CVector(0.02f,0.02f,0.01f);	
	motion	= CVector(0.0002f,0.0002f,0.0001f);
	slow 	= CVector(1.3f,1.3f,1.4f);

	scaleReactionMin 	= 0.3f;			// a scale to modify the degree of reaction based on how angularily far we are away from it.
	scaleReactionMax 	= 1.0f;			
	twitchFreq 			= 20;			// frequency of twitching
	twitchVel 			= 0.001f;		// amount of twitch	
	
	Reset();
}
//-----------------------------------------------------------------------------------
void CHandShaker::Reset()
{
	ang 	= CVector(0,0,0);
	vel 	= CVector( 	CGeneral::GetRandomNumberInRange(0.0f,motion.x),
						CGeneral::GetRandomNumberInRange(0.0f,motion.y),
						CGeneral::GetRandomNumberInRange(0.0f,motion.z));
}

//-----------------------------------------------------------------------------------
#define SAME_SIGN(a,b) ((a)>0 && (b)>0 ? true : (a)<0 && (b)<0 ? true : false )

void CHandShaker::Process(float degree)
{
	CVector scaleReaction(	DW_LERP(CMaths::Abs(ang.x/lim.x),scaleReactionMin,scaleReactionMax),
							DW_LERP(CMaths::Abs(ang.y/lim.y),scaleReactionMin,scaleReactionMax),
							DW_LERP(CMaths::Abs(ang.z/lim.z),scaleReactionMin,scaleReactionMax));

	if (SAME_SIGN(ang.x,vel.x))	scaleReaction.x *= slow.x;
	if (SAME_SIGN(ang.y,vel.y))	scaleReaction.y *= slow.y;
	if (SAME_SIGN(ang.z,vel.z))	scaleReaction.z *= slow.z;
	
	CVector rDelta(	CGeneral::GetRandomNumberInRange(0.0f,motion.x) * scaleReaction.x,
					CGeneral::GetRandomNumberInRange(0.0f,motion.y) * scaleReaction.y,
					CGeneral::GetRandomNumberInRange(0.0f,motion.z) * scaleReaction.z);

	if (ang.x>0.0f)  rDelta.x *= -1.0f;
	if (ang.y>0.0f)  rDelta.y *= -1.0f;
	if (ang.z>0.0f)  rDelta.z *= -1.0f;
				
	vel += rDelta;

	float timeExpiredScale = (float)CTimer::GetTimeElapsedInMillisecondsNonClipped()/(1000.0f/30.0f);
	int32 twitch = twitchFreq*timeExpiredScale;

	if (CGeneral::GetRandomNumberInRange(1,twitch) == 2)
	{
		vel += CVector(	CGeneral::GetRandomNumberInRange(-twitchVel,twitchVel),
						CGeneral::GetRandomNumberInRange(-twitchVel,twitchVel),
						CGeneral::GetRandomNumberInRange(-twitchVel,twitchVel));
	}
		
	ang += vel*timeExpiredScale; // scaled for PC	

	if (ang.x>lim.x) ang.x = lim.x; else if (ang.x<-lim.x) ang.x = -lim.x;
	if (ang.y>lim.y) ang.y = lim.y; else if (ang.y<-lim.y) ang.y = -lim.y;
	if (ang.z>lim.z) ang.z = lim.z; else if (ang.z<-lim.z) ang.z = -lim.z;				

	resultMat.SetRotate(ang.x*degree,ang.y*degree,ang.z*degree); // the roll is redundant really here.
}

//---------------------------------------------------------------------------------------------------
void CCam::Finalise_DW_CineyCams(CVector *pSrc, CVector *pDst,float roll, float fov, float nearClip, float degreeShake)
{
	Front=*pDst-*pSrc;
	Front.Normalise();
	
	Source = *pSrc;

	Up=CVector(CMaths::Sin(roll),0.0f,CMaths::Cos(roll));
	CVector TempRight= CrossProduct(Front, Up);
	TempRight.Normalise();
	Up=CrossProduct(TempRight, Front);

	if ((Front.x==0.0f) && (Front.y==0.0f))
	{
		Front.x=0.0001f;
		Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( Front, Up );
	TempRight.Normalise();
	Up = CrossProduct( TempRight, Front );


	//GetVectorsReadyForRW(); // Bad function!!!!.. me no like - DW

	FOV=fov;

// unfortunately it looks shit - DW.
/*	static float fovThresholdForMotionBlur = 0.001f;
	static float FOVScale = 0.3f;
	float dFOV = CMaths::Abs(FOV-DWCineyCamLastFov);
	if (dFOV > fovThresholdForMotionBlur)
	{
		CPostEffects::m_bSpeedFXUserFlagCurrentFrame = true; // re anable - no function provided
		CPostEffects::SpeedFXManualSpeedForThisFrame(CMaths::Max(1.0f,dFOV*FOVScale));		
	}*/
	
	
	RwCameraSetNearClipPlane(Scene.camera,0.4f);//Clip);

	CacheLastSettingsDWCineyCam();
	
	gLastFrameProcessedDWCineyCam = CTimer::m_FrameCounter;
	

	//----------------------------------------------------------------------------------	
	//----------------------------------------------------------------------------------	
	//--------------Now apply handshake simulation--------------------------------------	
	//----------------------------------------------------------------------------------	
	CHandShaker *pS = &gHandShaker[0];		
	pS->Process(degreeShake);		
	roll = pS->ang.z * degreeShake; // might be z ???
	CVector temp = Multiply3x3(Front,pS->resultMat);
	Front = temp;
	//----------------------------------------------------------------------------------	
	//----------------------------------------------------------------------------------	
	//------And rebuild the matrix again------------------------------------------------	
	//----------------------------------------------------------------------------------	

	Front.Normalise();
	
	Source = *pSrc;

	Up=CVector(CMaths::Sin(roll),0.0f,CMaths::Cos(roll));
	TempRight= CrossProduct(Front, Up);
	TempRight.Normalise();
	Up=CrossProduct(TempRight, Front);

	if ((Front.x==0.0f) && (Front.y==0.0f))
	{
		Front.x=0.0001f;
		Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( Front, Up );
	TempRight.Normalise();
	Up = CrossProduct( TempRight, Front );					
}

//----------------------------------------------------------------------------------	
class CHeliCamSettings
{
	public :
		CVector heliMoveTo;
		CVector heliMoveFrom;
		float heliSpeed 					;// Speed of helicopter roughtly over its journey
		float behindStart 					;// distance behind the player to start from
		float heliHeight 					;// helicopter height above player
		float heliOutSideOfVehicle 			;// the side vector of the heli from the car
		float zoomInTime 					;// the time taken to do first zoom in...
		float zoomFOVStart 					;// FOV lerp start for first FOV zoom
		float zoomFOVEnd 					;// FOV lerp end for first FOV zoom
		float heliSpeedLookInFrontMul		;// Heli can look in front of the vehicle
		float heliRollScale  				;// degree or roll in the heli over time
		float heliClip 						;// near clip distance for heli
		bool  gbLockDest 					;// are we locking where we are looking at
		int32 gLockDestTimeOut 				;// timeout for locked look at 
		int32 gLockDestHeliTimeoutDefault 	;// timeout default for locked look at
		CVector gLockedDest					;// the cached destination for locked look at
		int32 numAttemptsToFindAValidOne 	;// iterations for search for a good camera.
		bool  gbHeliCollided				;// did the simulated heli actually hit a building?
		int32 gDefaultFramesOutOfSightBeforeWeLoseTheCar; // where the car is out of sight time in frames until we give in trying to track it
		int32 gFramesBeenOutOfSight 		; // a frame counter
		float gHeliDistFOVZoomMore			;	// distance of car to heli for starting second FOV zoom
		float gHeliDistFOVZoomMoreMax		;	// distance of car to heli to end second FOV zoom lerp
		float gLessFOV						;	// amount of FOV zoom in second FOV lerp
		float gLenToCarToPushCameraOut		;	// min XY 2d distance to push the heli away from the car to stop 180 degree flip in camera roll
		float gRadiusOfSphereAroundHeli		;	// for stopping the camera from moving into collision - takes acoount fo generous clip distance
		float gZoomOutFOV					; 	// when we lose the car we zoom out the FOV
		bool  gbWeLostTheCar				;
		bool  gbNoZoom						;	// disabliing of zoom at start
	
		float gFOVZoomBackOutFrom;
		bool  gZoomBackOut;
		int32 gZoomBackOutTimeStart;
		int32 gZoomBackOutTimeEnd;	
		float timeToZoomOut;
		int32 gZoomBackOutSpeed;
	
		
		CHeliCamSettings() { SetDefault(); }
		~CHeliCamSettings() {}	
		void SetDefault();
		void Reset();
		void RandomiseABit();
};

//----------------------------------------------------------------------------------	

void CHeliCamSettings::RandomiseABit()
{
	heliSpeed   *=  CGeneral::GetRandomNumberInRange(0.1f,1.5f);
	behindStart *=  CGeneral::GetRandomNumberInRange(0.5f,1.0f);
	heliHeight  *=  CGeneral::GetRandomNumberInRange(0.5f,1.0f);
	heliOutSideOfVehicle *= CGeneral::GetRandomNumberInRange(0.5f,1.0f);
	zoomInTime *=   CGeneral::GetRandomNumberInRange(0.5f,2.0f);
}

//----------------------------------------------------------------------------------	

void CHeliCamSettings::Reset()
{
	gbLockDest 						= false;	// are we locking where we are looking at
	gLockDestTimeOut 				= 0;		// timeout for locked look at 
	gbHeliCollided					= false;	// did the simulated heli actually hit a building?
	gbWeLostTheCar					= false;
	gbNoZoom						= false;
	gZoomOutFOV						= zoomFOVStart - zoomFOVEnd; // when we lose the car we zoom out the FOV
	gFramesBeenOutOfSight 			= gDefaultFramesOutOfSightBeforeWeLoseTheCar; // a frame counter
	gLockDestTimeOut 				= gLockDestHeliTimeoutDefault;
	gZoomBackOut					= false;
}

//----------------------------------------------------------------------------------	

void CHeliCamSettings::SetDefault()
{
	heliSpeed 						= 50.0f;	// Speed of helicopter roughtly over its journey
	behindStart 					= 30.0f;	// distance behind the player to start from
	heliHeight 						= 55.0f;	// helicopter height above player
	heliOutSideOfVehicle 			= 50.0f;	// the side vector of the heli from the car
	zoomInTime 						= 0.05f;	// the time taken to do first zoom in...
	zoomFOVStart 					= 70.0f;	// FOV lerp start for first FOV zoom
	zoomFOVEnd 						= 22.0f;	// FOV lerp end for first FOV zoom
	heliSpeedLookInFrontMul			= 1.0f;		// Heli can look in front of the vehicle
	heliRollScale  					= 0.0f;		// degree or roll in the heli over time
	heliClip 						= 10.0f;	// near clip distance for heli
	gLockDestHeliTimeoutDefault 	= 30;		// timeout default for locked look at
	numAttemptsToFindAValidOne 		= 8;		// iterations for search for a good camera.
	gDefaultFramesOutOfSightBeforeWeLoseTheCar = 60; // where the car is out of sight time in frames until we give in trying to track it
	gHeliDistFOVZoomMore			= 100.0f;	// distance of car to heli for starting second FOV zoom
	gHeliDistFOVZoomMoreMax			= 110.0f;	// distance of car to heli to end second FOV zoom lerp
	gLessFOV						= 10.0f;	// amount of FOV zoom in second FOV lerp
	gLenToCarToPushCameraOut		= 5.0f;		// min XY 2d distance to push the heli away from the car to stop 180 degree flip in camera roll
	gRadiusOfSphereAroundHeli		= 12.0f;	// for stopping the camera from moving into collision - takes acoount fo generous clip distance
	timeToZoomOut 					= 0.75f;	// At what degree of time should we start resuming a non zoomed view.
	gZoomBackOutSpeed				= 4000;		// how many milliseconds to zoom back out in
	
	Reset();
}

//----------------------------------------------------------------------------------	

#define NUM_HELI_CAM_SETTINGS 1
CHeliCamSettings gHCM[NUM_HELI_CAM_SETTINGS];

//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_HeliChaseCam(bool bCheckValid)
{
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack

	int32 selectSettings = CGeneral::GetRandomNumberInRange(0,NUM_HELI_CAM_SETTINGS-1);
	ASSERT(selectSettings<NUM_HELI_CAM_SETTINGS);
	CHeliCamSettings *pS = &gHCM[selectSettings];

	if(!CamTargetEntity || !CamTargetEntity->GetIsTypeVehicle())
		return false;			

#ifndef MASTER
// VarConsole.AddDebugOutput("DW-HELI-CAM");
#endif

	int32 camId = MOVIECAM20-MOVIECAM20;

	CEntity *pEntity;
	CVehicle *pVehicle;
	CVector dst,src,targetUp,targetRight,targetForward,targetMotion,targetAngMotion;
	float	targetVel,targetAngVel;
	CColSphere sph;
	GetCoreDataForDWCineyCamMode(&pEntity,&pVehicle,&dst,&src,&targetUp,&targetRight,&targetForward,&targetMotion,&targetVel,&targetAngMotion,&targetAngVel,&sph);
	int32 curTime = CTimer::GetTimeInMilliseconds();


	
	//////////////////////////////
	// FIRST FRAME
	//////////////////////////////			
	
	if (gDWLastModeForCineyCam != MODE_DW_HELI_CHASE || gLastFrameProcessedDWCineyCam < CTimer::m_FrameCounter - 1)
	{	// initialise in here	
		bool bIsValid = false;

		gbExitCam[camId] 		= false;
		gDWLastModeForCineyCam 	= MODE_DW_HELI_CHASE;		
		gDWCineyCamStartTime 	= curTime;
		gDWCineyCamEndTime	 	= curTime + gDWCamMaxDurationMs[camId];
		pS->SetDefault();
		pS->Reset();
		pS->RandomiseABit();
		
		gHandShaker[0].Reset();
		
		for (int32 i=0;i<pS->numAttemptsToFindAValidOne;i++)
		{
			// Set a couple to positions in the sky for the heli to move to.
			// The heli should be so far away from the player that really its quite happy hovering and keeping the player in shot.
			// First phase is to zoom in quickly.
			// then just track

			//////////////////////////////
			// SETTING UP INITIAL CRITERIA
			//////////////////////////////
			
			CVector fwd = targetForward;
			
			pS->heliMoveFrom 	= dst - (fwd * pS->behindStart);
			pS->heliMoveTo 		= dst + (fwd * pS->heliSpeed);
			pS->heliMoveFrom.z  += pS->heliHeight;
			pS->heliMoveTo.z  	+= pS->heliHeight;										
			
			// Make the heli cross the vehicles path (randomly)
			bool bSign1 = CGeneral::GetRandomTrueFalse();
			bool bSign2 = CGeneral::GetRandomTrueFalse();
			float mul1 = 1.0f;
			float mul2 = 1.0f;
			
			if (bSign1) mul1 = -1.0f;
			if (bSign2) mul2 = -1.0f;			
			
			CVector sideVec = targetRight;
			sideVec.z = 0.0f;
			
			pS->heliMoveFrom	+= sideVec * pS->heliOutSideOfVehicle * mul1;
			pS->heliMoveTo		+= sideVec * pS->heliOutSideOfVehicle * mul2;						

			//////////////////////////////////////
			// VALIDITY CHECKING FOR INITIALISING
			//////////////////////////////////////						
			
			// Now we know where we are going... is it safe to go there?
			if (CWorld::TestSphereAgainstWorld(pS->heliMoveFrom, pS->gRadiusOfSphereAroundHeli, NULL, true, true, false, false, false))
				continue;				
			
			CColPoint colPoint;
			CEntity *pHitEntity;
			
			CWorld::pIgnoreEntity = pEntity;
			bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, pS->heliMoveFrom, colPoint, pHitEntity, true, true, false, false, false, false, false);
			CWorld::pIgnoreEntity = NULL;							
			
			if (!bLineOfSightToVehicleIsClear)
				continue;
			
			bIsValid = true;
			break;
		}
		
		if (!bIsValid)
		{
			pS->Reset();
			gbExitCam[camId] = true;
			return false;
		}
		
		pS->gbNoZoom 				= CGeneral::GetRandomTrueFalse();
		pS->gZoomBackOut 			= CGeneral::GetRandomTrueFalse();
	}		

	///////////////////////////////////
	// CAMERA PROCESSING OF MOTION
	///////////////////////////////////
	// We found this camera to be invalid last time we processed it.
	if (gbExitCam[camId])
		return false;

	float t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);
	
	src = CVector(	DW_LERP(t,pS->heliMoveFrom.x,pS->heliMoveTo.x),
					DW_LERP(t,pS->heliMoveFrom.y,pS->heliMoveTo.y),
					DW_LERP(t,pS->heliMoveFrom.z,pS->heliMoveTo.z));	
		
	dst += targetForward + targetForward * targetVel * pS->heliSpeedLookInFrontMul;
	
	//------------------------------------------------------
	// prevent 180 degree flip when going right over the car.
	CVector toCar = dst-src;
	float lenToCar = toCar.Magnitude2D();	
	toCar /= lenToCar;
		
	if (lenToCar<pS->gLenToCarToPushCameraOut)
	{
		src.x = dst.x - (toCar.x * pS->gLenToCarToPushCameraOut); 
		src.y = dst.y - (toCar.y * pS->gLenToCarToPushCameraOut); 
	}	
	
	//-----------------
	// Zoom in at start 
	float newFOV = pS->zoomFOVEnd;
	if (t<pS->zoomInTime && !pS->gbNoZoom)
	{
		newFOV = DW_SINE_ACCEL_DECEL_LERP(t*(1.0f/pS->zoomInTime),pS->zoomFOVStart,pS->zoomFOVEnd);
	}

	//----------------------------------------------------------
	// We are really far away now - lets zoom in again a wee bit
	float lessFOV = 0.0f;
	
	float dist = (src-dst).Magnitude();
	if (dist>pS->gHeliDistFOVZoomMore)
	{
		float t = DW_CONSTRAIN((dist-pS->gHeliDistFOVZoomMore) / (pS->gHeliDistFOVZoomMoreMax-pS->gHeliDistFOVZoomMore),0.0f,1.0f);
		lessFOV = DW_SINE_ACCEL_DECEL_LERP(t,0,pS->gLessFOV);
	}
	newFOV -= lessFOV;

	float roll = t * pS->heliRollScale; // not used - shouldnt be used really.

	//---------------------------------------------
	// We hit a building or something - stop moving
	static float buildingCheckRadius  = 15.0f;
	if (pS->gbHeliCollided || CWorld::TestSphereAgainstWorld(src, buildingCheckRadius, NULL, true, true, false, false, false))	
	{
		// slow down motion greatly instead of just stopping... if iy where easy???? - todo 
		static CVector SlowPos;
		static float slowDownDegree = 0.5f;
		static int32 gTimeSlowTime = 100;
			
		if (!pS->gbHeliCollided)
		{
			pS->gbHeliCollided = true;
			gTimeSlowTime = 100;
			SlowPos = src;			
		}
		
		if (gTimeSlowTime-- < 0)
		{
			src = DWCineyCamLastPos; // the camera hit something... it cant move any more, but this isnt a reason to quit.				
			gbExitCam[camId] = true;
			return false;			
		}
		else
		{
			CVector d = src - SlowPos;
			src = SlowPos + (d * slowDownDegree);
		}
	}

	///////////////////////////////////
	// CAMERA PROCESSING EARLY OUTS
	///////////////////////////////////
	if (!pS->gbLockDest)	
	{
		CColPoint colPoint;
		CEntity *pHitEntity;
		
		CWorld::pIgnoreEntity = pEntity;
		bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
		CWorld::pIgnoreEntity = NULL;
		
		if (!bLineOfSightToVehicleIsClear)
		{		
			pS->gbWeLostTheCar = true;
			
			if (!pS->gZoomBackOut && pS->gFramesBeenOutOfSight < pS->gDefaultFramesOutOfSightBeforeWeLoseTheCar/4)
			{
				// show we lost the car... 
				pS->gFOVZoomBackOutFrom = newFOV;
				pS->gZoomBackOut = true;
				pS->gZoomBackOutTimeStart = curTime;
				pS->gZoomBackOutTimeEnd   = curTime + pS->gZoomBackOutSpeed;
			}						
			
			if (pS->gFramesBeenOutOfSight-- == 0) // Lock the position.. ie. Pretend we lost the car!.. Damn we lost him...
			{
				pS->gLockedDest = dst; // the position to keep looking at								
				pS->gbLockDest = true;			
			}			
		}
		else
		{
			pS->gFramesBeenOutOfSight++;			
			
			if (pS->gFramesBeenOutOfSight > pS->gDefaultFramesOutOfSightBeforeWeLoseTheCar)
				pS->gFramesBeenOutOfSight = pS->gDefaultFramesOutOfSightBeforeWeLoseTheCar;
		}
	}
	else
	{
		dst = pS->gLockedDest;
		if (pS->gLockDestTimeOut-- == 0)
		{
			gbExitCam[camId] = true;
			return false;
		}
	}

//	float amountOFFOVZoomOut = 1.0f - ((float)gFramesBeenOutOfSight/(float)gDefaultFramesOutOfSightBeforeWeLoseTheCar);			 
//	newFOV += gZoomOutFOV * amountOFFOVZoomOut;

	if (!pS->gZoomBackOut && t>=pS->timeToZoomOut)
	{
		pS->gFOVZoomBackOutFrom = newFOV;
		pS->gZoomBackOut = true;
		pS->gZoomBackOutTimeStart = curTime;
		pS->gZoomBackOutTimeEnd   = curTime + pS->gZoomBackOutSpeed;
	}	
	
	if (pS->gZoomBackOut)
	{
		float zt = DW_CONSTRAIN(((float)curTime - (float)pS->gZoomBackOutTimeStart) / ((float)pS->gZoomBackOutTimeEnd - (float)pS->gZoomBackOutTimeStart),0.0f,1.0f);		
		newFOV = DW_SINE_ACCEL_DECEL_LERP(zt,pS->gFOVZoomBackOutFrom,pS->zoomFOVStart); // overrides
	}

	///////////////////////////////////
	// CAMERA STANDARD REASONS TO QUIT
	///////////////////////////////////

	if (IsTimeToExitThisDWCineyCamMode(MOVIECAM20,&src,&dst,t, false))
	{
		gbExitCam[camId] = true;
		return false;
	}

	///////////////////////////////////
	// MAKE THE CHANGES TO THE CAMERA 
	///////////////////////////////////
	
	// Dynamic clip to prevent lot sof bad zfighting on tops of cars.
	float nearClip = pS->heliClip;// * (70.0f/newFOV);
//	static float nearClipMax = 40.0f;
//	if (nearClip > nearClipMax) nearClip  = nearClipMax;
	
	Finalise_DW_CineyCams(&src, &dst, roll, newFOV, nearClip, 1.0f);	
	
	
	
	
	return true;	
}

//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_CamManCam(bool bCheckValid)
{		
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack

	if(!CamTargetEntity || !CamTargetEntity->GetIsTypeVehicle())
		return false;		

#ifndef MASTER
//	VarConsole.AddDebugOutput("DW-CAM_MAN-CAM");
#endif

	CEntity *pEntity;
	CVehicle *pVehicle;
	CVector dst,src,targetUp,targetRight,targetForward,targetMotion,targetAngMotion;
	float	targetVel,targetAngVel;
	CColSphere sph;
	GetCoreDataForDWCineyCamMode(&pEntity,&pVehicle,&dst,&src,&targetUp,&targetRight,&targetForward,&targetMotion,&targetVel,&targetAngMotion,&targetAngVel,&sph);
	int32 curTime = CTimer::GetTimeInMilliseconds();

	int32 camId = MOVIECAM21-MOVIECAM20;
	static CVector persistantLampPostPos;
	static int32 gTimeOutDefault = 100;
	static int32 timeout = gTimeOutDefault;	
	
	
	if (gDWLastModeForCineyCam != MODE_DW_CAM_MAN || gLastFrameProcessedDWCineyCam < CTimer::m_FrameCounter - 1)
	{	// initialise in here		
		gbExitCam[camId] = false;
		gDWLastModeForCineyCam = MODE_DW_CAM_MAN;		
		gDWCineyCamStartTime = curTime;
		gDWCineyCamEndTime	 = gDWCineyCamStartTime + gDWCamMaxDurationMs[camId];
				
		timeout = gTimeOutDefault;// non visible timeout
		
		gHandShaker[0].Reset();				

		//----------------------------------------------------------------
		// choose a position by a mapost where we might drive towards...
		static float distToScanForLampPosts = 50.0f;
		static float distToAcceptLampPosts = 5.0f;

		{		
			CEntity *ppResults[16];
			CEntity *pFoundEntity = NULL;
			int16	nNum, i;
			float	BestDist, Dist;
			CVector vecTestLosTarget, vecTestLosStart;
			
			// search for objects and dummies			
			CVector scanPos = dst + targetForward * distToScanForLampPosts; // Scan ahead so we might get a shot of the car passing by us.
			
			CWorld::FindObjectsInRange(scanPos, distToScanForLampPosts, true, &nNum, 15, ppResults, false, false, false, true, true);
			BestDist = 10000.0f;
			for(i=0; i<nNum; i++) // find best one
			{
				// we're only really looking for lampposts or traffic lights at the mo'
				if( ppResults[i]->GetIsStatic() && ppResults[i]->GetMatrix().zz > 0.9f
				&& bIsLampPost(ppResults[i]->GetModelIndex()))
				{
					Dist = (ppResults[i]->GetPosition() - dst).Magnitude2D();
					
					if(Dist < BestDist && Dist > distToAcceptLampPosts)
					{						
						vecTestLosStart = ppResults[i]->GetColModel().GetBoundBoxMax();
						vecTestLosStart = ppResults[i]->GetMatrix() * vecTestLosStart;
						
						// Lower down the lamppost  - to approximately human head height
						vecTestLosStart.z -= ppResults[i]->GetColModel().GetBoundBoxMax().z;
						vecTestLosStart.z += ppResults[i]->GetColModel().GetBoundBoxMin().z * 0.5f;
						
						vecTestLosTarget = vecTestLosStart - dst;
						vecTestLosTarget.Normalise();
						vecTestLosTarget += dst;
						
						static float heightToleranceToAcceptLampPosts = 6.0f;
						
						if (CMaths::Abs(vecTestLosTarget.z-vecTestLosTarget.z) < heightToleranceToAcceptLampPosts)
						{
							if(CWorld::GetIsLineOfSightClear(vecTestLosStart, vecTestLosTarget, true, false, false, false, false, true, true))
							{
								BestDist = Dist;
								pFoundEntity = ppResults[i];
								src = vecTestLosStart;
								persistantLampPostPos = src;
							}
						}
					}
				}
			}

			if (!pFoundEntity)
			{		
				gbExitCam[camId] = true;
				return false;
			}	
		}				
	}	


	float t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

	//------------------------------------------------------------------------
	// Set the camera position and make sure the lampost we are beside it is not obsuring the camera....		
	if (!gbExitCam[camId])
	{
		// 	
		static float gRadiusAroundLampost = 1.0f;
		src = persistantLampPostPos;
		
		CVector toCar = dst-src;
		toCar.Normalise();
		src += toCar * gRadiusAroundLampost;
	}

	//--------------------------------------------------------
	// Zoom in to the deisired FOV instead of snapping
	CVector distVec = dst-src;
	float len		= distVec.Magnitude();
	static float lenDiv 	= 30.0f;		// distance over which lerp takes place
	static float nearFOV 	= 70.0f;		// the FOV when near to camera
	static float farFOV 	= 15.0f;		// the FOV when at or beyond its furthest distance ( See above) 
	
	float nt = DW_CONSTRAIN(len/lenDiv,0.0f,1.0f);	
	float newFOV = DW_SINE_ACCEL_DECEL_LERP(nt,nearFOV,farFOV);	
	
	static float timeToZoomInAtStart = 0.1f;
	if (t<timeToZoomInAtStart)
	{
		static float startFOV = 70.0f;
		float tToZoom = DW_CONSTRAIN(t / timeToZoomInAtStart,0.0f,1.0f);		
		newFOV = DW_SINE_ACCEL_DECEL_LERP(tToZoom,startFOV,newFOV); // override with initial zoom In
	}	

	//-------------------------------------------------------
	// standard reasons to quit this camera.	
	if (IsTimeToExitThisDWCineyCamMode(MOVIECAM21,&src,&dst,t,false))
	{
		gbExitCam[camId] = true;	
		return false;
	}

	//-------------------------------------------------------
	// If not visible timeout eventually
	CColPoint colPoint;
	CEntity *pHitEntity;
	
	CWorld::pIgnoreEntity = pEntity;
	bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
	CWorld::pIgnoreEntity = NULL;							
	
	if (!bLineOfSightToVehicleIsClear)
	{
		if (timeout-- == 0)
		{		
			gbExitCam[camId] = true;	
			return false;	
		}
	}
	else
	{
		if (timeout++>gTimeOutDefault) timeout = gTimeOutDefault;
	}			
		
	//-------------------------------------------------------
	// make degree of shake proportional to motion of vehicle
	float tmLen = targetMotion.Magnitude();
	static float tmMul = 8.0f;
	static float minTmLen = 0.2f;
	float degree = DW_CONSTRAIN(CMaths::Max(tmLen * tmMul,minTmLen),0.0f,1.0f);
	
	float nearClip = DW_LERP(newFOV/70.0f,10.0f,0.3f);
	
	Finalise_DW_CineyCams(&src, &dst, 0.0f, newFOV, nearClip, degree);	
	
	return true;	
}

//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_BirdyCam(bool bCheckValid)
{
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack

	if(!CamTargetEntity || !CamTargetEntity->GetIsTypeVehicle())
		return false;		

#ifndef MASTER
//	VarConsole.AddDebugOutput("DW-BIRDY-CAM");
#endif

	CEntity *pEntity;
	CVehicle *pVehicle;
	CVector dst,src,targetUp,targetRight,targetForward,targetMotion,targetAngMotion;
	float	targetVel,targetAngVel;
	CColSphere sph;
	GetCoreDataForDWCineyCamMode(&pEntity,&pVehicle,&dst,&src,&targetUp,&targetRight,&targetForward,&targetMotion,&targetVel,&targetAngMotion,&targetAngVel,&sph);
	int32 curTime = CTimer::GetTimeInMilliseconds();

	int32 camId = MOVIECAM22-MOVIECAM20;
	static CVector persistantLampPostPosBirdy[2];
	static int32 gTimeOutDefault = 30;
	static int32 timeout = gTimeOutDefault;	
	
	
	if (gDWLastModeForCineyCam != MODE_DW_BIRDY || gLastFrameProcessedDWCineyCam < CTimer::m_FrameCounter - 1)
	{	// initialise in here		
		gbExitCam[camId] = false;
		gDWLastModeForCineyCam = MODE_DW_BIRDY;		
		gDWCineyCamStartTime = curTime;
		gDWCineyCamEndTime	 = gDWCineyCamStartTime + gDWCamMaxDurationMs[camId];
				
		timeout = gTimeOutDefault;// non visible timeout
		
		gHandShaker[0].Reset();				

		//----------------------------------------------------------------
		// choose a position by a mapost where we might drive towards...
		static float distToScanForLampPostsBirdy1 = 50.0f;
		static float distToScanForLampPostsBirdy2 = 50.0f;		
		static float distToAcceptLampPostsBirdy = 2.0f;

		{		
			#define N_OBJS 128
			CEntity *ppResults1[N_OBJS];
			CEntity *ppResults2[N_OBJS];
			CEntity *ppResultsLamp1[N_OBJS];						
			CEntity *ppResultsLamp2[N_OBJS];						
			CEntity *pFoundEntity[2] = {NULL,NULL};
			int16	nNum1,nNum2;
			int16	nNumLamp1,nNumLamp2;
//			float	BestDist, Dist;
			CVector vecTestLosTarget;
			CVector vecTestLosStart1, vecTestLosStart2;			
			static float range1 = 2.0f;			
			static float range2 = 1.0f;			
			
			// now separate objects into behind car and in front. roughly
			// search for objects and dummies			
			CVector scanPos1 = dst + targetForward * distToScanForLampPostsBirdy1*range1; // Scan ahead so we might get a shot of the car passing by us.			
			CWorld::FindObjectsInRange(scanPos1, distToScanForLampPostsBirdy1, true, &nNum1, N_OBJS-1, ppResults1, false, false, false, true, true);
			
			CVector scanPos2 = dst - targetForward * distToScanForLampPostsBirdy2*range2; // Scan ahead so we might get a shot of the car passing by us.
			CWorld::FindObjectsInRange(scanPos2, distToScanForLampPostsBirdy2, true, &nNum2, N_OBJS-1, ppResults2, false, false, false, true, true);						
			
			nNumLamp1 = 0;
			for (int32 i=0;i<nNum1;i++)			
				if (ppResults1[i]->GetIsStatic() && ppResults1[i]->GetMatrix().zz > 0.9f && bIsLampPost(ppResults1[i]->GetModelIndex()))
					ppResultsLamp1[nNumLamp1++] = ppResults1[i];

			nNumLamp2 = 0;
			for (int32 i=0;i<nNum2;i++)			
				if (ppResults2[i]->GetIsStatic() && ppResults2[i]->GetMatrix().zz > 0.9f && bIsLampPost(ppResults2[i]->GetModelIndex()))
					ppResultsLamp2[nNumLamp2++] = ppResults2[i];
			
			bool bOK = false;
			
			// Choose any two lamposts than can see each other... just by nature of world split up this will be random and we quickout asap because we dont want to do too many ray casts
			for (int32 i=0;i<nNumLamp1;i++)
			{
				if (bOK)
					break;
			
				if (ppResultsLamp1[i]->GetIsStatic() && ppResultsLamp1[i]->GetMatrix().zz > 0.9f && bIsLampPost(ppResultsLamp1[i]->GetModelIndex()))
				{
					vecTestLosStart1 = ppResultsLamp1[i]->GetColModel().GetBoundBoxMax();
					vecTestLosStart1 = ppResultsLamp1[i]->GetMatrix() * vecTestLosStart1;
					
					// Lower down the lamppost  - to approximately human head height
					float heightScope = ppResultsLamp1[i]->GetColModel().GetBoundBoxMax().z -  ppResultsLamp1[i]->GetColModel().GetBoundBoxMin().z * 0.5f;
					vecTestLosStart1.z -= CGeneral::GetRandomNumberInRange(1.0f,heightScope);
					
					vecTestLosTarget = vecTestLosStart1 - dst;
					vecTestLosTarget.Normalise();
					vecTestLosTarget += dst;
					
					static float heightToleranceToAcceptLampPosts = 6.0f;
					
					if (CMaths::Abs(vecTestLosTarget.z-vecTestLosTarget.z) < heightToleranceToAcceptLampPosts)
					{									
						for (int32 j=i;j<nNumLamp2;j++)
						{
							if (bOK)
								break;						
						
							if (ppResultsLamp2[j]->GetIsStatic() && ppResultsLamp2[j]->GetMatrix().zz > 0.9f && bIsLampPost(ppResultsLamp2[j]->GetModelIndex()))
							{
								vecTestLosStart2 = ppResultsLamp2[j]->GetColModel().GetBoundBoxMax();
								vecTestLosStart2 = ppResultsLamp2[j]->GetMatrix() * vecTestLosStart2;
								
								// Lower down the lamppost  - to approximately human head height
								float heightScope = ppResultsLamp2[j]->GetColModel().GetBoundBoxMax().z -  ppResultsLamp2[j]->GetColModel().GetBoundBoxMin().z * 0.5f;
								vecTestLosStart2.z -= CGeneral::GetRandomNumberInRange(1.0f,heightScope);
								
								if(CWorld::GetIsLineOfSightClear(vecTestLosStart1, vecTestLosStart2, true, false, false, false, false, true, true))
								{
									if(CWorld::GetIsLineOfSightClear(vecTestLosStart2, vecTestLosTarget, true, false, false, false, false, true, true))
									{
										src = vecTestLosStart1;
										persistantLampPostPosBirdy[0] = vecTestLosStart1;
										persistantLampPostPosBirdy[1] = vecTestLosStart2;	
										bOK = true;									
									}
								}																			
							}	
						}
					}
				}
			}
			
			if (!bOK)
			{
				gbExitCam[camId] = true;
				return false;			
			}	
		}		
	}
			
#if 0			
			for (int32 j=1;j>=0;j--)
			{
				BestDist = 10000.0f;	
				
				if (j==0)
				{
					BestDist *= -1;
				}
									
				for(i=0; i<nNum; i++) // find best one
				{	
					// we're only really looking for lampposts or traffic lights at the mo'
					if( ((pFoundEntity[0] != ppResults[i]))  && 
						ppResults[i]->GetIsStatic() && ppResults[i]->GetMatrix().zz > 0.9f
					&&	bIsLampPost(ppResults[i]->GetModelIndex()))
					{						
						Dist = (ppResults[i]->GetPosition() - dst).Magnitude2D();
						// go from far one to near one.
						bool bAccept;						
						if (j==0)
							bAccept = (Dist > BestDist && Dist > distToAcceptLampPostsBirdy);
						else
							bAccept = (Dist < BestDist && Dist > distToAcceptLampPostsBirdy);
						
						if(bAccept)
						{						
							vecTestLosStart = ppResults[i]->GetColModel().GetBoundBoxMax();
							vecTestLosStart = ppResults[i]->GetMatrix() * vecTestLosStart;
							
							// Lower down the lamppost  - to approximately human head height
							float heightScope = ppResults[i]->GetColModel().GetBoundBoxMax().z -  ppResults[i]->GetColModel().GetBoundBoxMin().z * 0.5f;
							
							vecTestLosStart.z -= CGeneral::GetRandomNumberInRange(1.0f,heightScope);
							
							vecTestLosTarget = vecTestLosStart - dst;
							vecTestLosTarget.Normalise();
							vecTestLosTarget += dst;
							
							static float heightToleranceToAcceptLampPosts = 6.0f;
							
							if (CMaths::Abs(vecTestLosTarget.z-vecTestLosTarget.z) < heightToleranceToAcceptLampPosts)
							{
								bool bOK = true;
								
								// line of sight to each lamppost
								if (j==1)
								{
									if(CWorld::GetIsLineOfSightClear(persistantLampPostPosBirdy[0], vecTestLosTarget, true, false, false, false, false, true, true))
									{
										bOK = false;
									}
								}							
														
								if(CWorld::GetIsLineOfSightClear(vecTestLosStart, vecTestLosTarget, true, false, false, false, false, true, true))
								{
									BestDist = Dist;
									pFoundEntity[j] = ppResults[i];
									src = vecTestLosStart;
									persistantLampPostPosBirdy[j] = src;
								}
							}
						}
					}
				}

				if (!pFoundEntity[j])
				{		
					gbExitCam[camId] = true;
					return false;
				}	
			}
		}				
	}	
#endif

	float t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);


	static float toOtherScale = 1.0f;
	CVector goingFrom 	= persistantLampPostPosBirdy[1]; // notice I swapped them
	CVector goingTo 	= persistantLampPostPosBirdy[0];
	CVector toOther 	= goingFrom-goingTo;
	toOther.Normalise();
	goingFrom 	-= toOther * toOtherScale;
	goingTo 	+= toOther * toOtherScale * 2.0f;
			
	//------------------------------------------------------------------------	
	// 
	if (!gbExitCam[camId])
	{	
		src = CVector(	DW_SINE_ACCEL_DECEL_LERP(t,goingFrom.x,goingTo.x),
						DW_SINE_ACCEL_DECEL_LERP(t,goingFrom.y,goingTo.y),
						DW_SINE_ACCEL_DECEL_LERP(t,goingFrom.z,goingTo.z));									
	}

	//--------------------------------------------------------
	// Zoom in to the deisired FOV instead of snapping
	float newFOV = 70.0f;
/*	CVector distVec = dst-src;
	float len		= distVec.Magnitude();
	static float lenDiv 	= 30.0f;		// distance over which lerp takes place
	static float nearFOV 	= 70.0f;		// the FOV when near to camera
	static float farFOV 	= 10.0f;		// the FOV when at or beyond its furthest distance ( See above) 
	
	float nt = DW_CONSTRAIN(len/lenDiv,0.0f,1.0f);	
	float newFOV = DW_SINE_ACCEL_DECEL_LERP(nt,nearFOV,farFOV);	
*/	
/*	static float timeToZoomInAtStart = 0.1f;
	if (t<timeToZoomInAtStart)
	{
		static float startFOV = 70.0f;
		float tToZoom = DW_CONSTRAIN(t / timeToZoomInAtStart,0.0f,1.0f);		
		newFOV = DW_SINE_ACCEL_DECEL_LERP(tToZoom,startFOV,newFOV); // override with initial zoom In
	}	
*/
	//-------------------------------------------------------
	// standard reasons to quit this camera.	
	if (IsTimeToExitThisDWCineyCamMode(MOVIECAM22,&src,&dst,t,false))
	{
		gbExitCam[camId] = true;	
		return false;
	}

	//-------------------------------------------------------
	// If not visible timeout eventually
	CColPoint colPoint;
	CEntity *pHitEntity;
	
	CWorld::pIgnoreEntity = pEntity;
	bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
	CWorld::pIgnoreEntity = NULL;							
	
	if (!bLineOfSightToVehicleIsClear)
	{
		if (timeout-- == 0)
		{		
			gbExitCam[camId] = true;	
			return false;	
		}
	}
	else
	{
		if (timeout++>gTimeOutDefault) timeout = gTimeOutDefault;
	}			
	
	//-------------------------------------------------------
	// make degree of shake proportional to motion of vehicle
///	float tmLen = targetMotion.Magnitude();
//	static float tmMul = 8.0f;
//	static float minTmLen = 0.2f;
//	float degree = DW_CONSTRAIN(CMaths::Max(tmLen * tmMul,minTmLen),0.0f,1.0f);
	Finalise_DW_CineyCams(&src, &dst, 0.0f, newFOV, 0.3f, 0.0f);//degree);	
	
	return true;	
}

//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_PlaneSpotterCam(bool bCheckValid)
{		
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack

	if(!CamTargetEntity || !CamTargetEntity->GetIsTypeVehicle())
		return false;	

#ifndef MASTER	
//	VarConsole.AddDebugOutput("DW-PLANESPOTTER-CAM");
#endif

	CEntity *pEntity;
	CVehicle *pVehicle;
	CVector dst,src,targetUp,targetRight,targetForward,targetMotion,targetAngMotion;
	float	targetVel,targetAngVel;
	CColSphere sph;
	GetCoreDataForDWCineyCamMode(&pEntity,&pVehicle,&dst,&src,&targetUp,&targetRight,&targetForward,&targetMotion,&targetVel,&targetAngMotion,&targetAngVel,&sph);
	int32 curTime = CTimer::GetTimeInMilliseconds();
	int32 camId = MOVIECAM23-MOVIECAM20;

	static int32 numAttemptsForPlaneSpotter = 8; // how many iterations do we have of selecting a random location to check for land from the flying vehicle.
	static CVector planeSpotterLocation;
	static float gMaxHeightDiffForPlaneSpotter 	= 100.0f;
	static float planeSpotterTrans 				= 100.0f; // fickle thing...	keep proportional to the height for implicit angle limits.
	static bool gbFOVZoomPlaneSpotter = false;
	
	if (gDWLastModeForCineyCam != MODE_DW_PLANE_SPOTTER || gLastFrameProcessedDWCineyCam < CTimer::m_FrameCounter - 1)
	{	// initialise in here
		gbExitCam[camId] = false;
		gDWLastModeForCineyCam = MODE_DW_PLANE_SPOTTER;	
		gDWCineyCamStartTime = curTime;
		gDWCineyCamEndTime	 = gDWCineyCamStartTime + gDWCamMaxDurationMs[camId];
		
		bool bLineOfSightToVehicleIsClear = false;
		for (int32 i=0;i<numAttemptsForPlaneSpotter;i++)
		{
			src = dst;
			src.z -= gMaxHeightDiffForPlaneSpotter;
			src.x += CGeneral::GetRandomNumberInRange(planeSpotterTrans/2.0f,planeSpotterTrans);
			src.y += CGeneral::GetRandomNumberInRange(planeSpotterTrans/2.0f,planeSpotterTrans);
			
			// Find a position not too far from the ground where we can look up at the plane
			//-------------------------------------------------------
			// If not visible timeout eventually
			CColPoint colPoint;
			CEntity *pHitEntity;
			
			CWorld::pIgnoreEntity = pEntity;
			bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
			CWorld::pIgnoreEntity = NULL;							
			
			if (!bLineOfSightToVehicleIsClear)
			{ // we found the ground ... create a plane spotter.				
				planeSpotterLocation = colPoint.GetPosition();										
				planeSpotterLocation.z += 2.0f; // push off the ground
				break;
			}				
		}
		
		if (bLineOfSightToVehicleIsClear) // THIS MEANS WE COULDNT FIND ANY LAND!!!!
		{
			gbExitCam[camId] = true;	
			return false;
		}
		
		gbFOVZoomPlaneSpotter = CGeneral::GetRandomTrueFalse();				
	}

	float t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

	src = planeSpotterLocation;

	float newFOV = 70.0f;

	// the flying vehcile is right above us - quit out, its going to flip us 180 degrees.
	CVector d = src-dst;
	if (d.Magnitude2D() < 5.0f)
	{
		gbExitCam[camId] = true;	
		return false;		
	}

	//--------------------------------------------------------
	// Zoom in to the deisired FOV instead of snapping
	if (gbFOVZoomPlaneSpotter)
	{
		CVector distVec = dst-src;
		float len		= distVec.Magnitude();
		static float lenDivPlane 	= 100.0f;		// distance over which lerp takes place
		static float nearFOVPlane 	= 70.0f;		// the FOV when near to camera
		static float farFOVPlane 	= 12.0f;		// the FOV when at or beyond its furthest distance ( See above) 
		
		float nt = DW_CONSTRAIN(len/lenDivPlane,0.0f,1.0f);	
		newFOV = DW_SINE_ACCEL_DECEL_LERP(nt,nearFOVPlane,farFOVPlane);	
		
		static float timeToZoomInAtStart = 0.1f;
		if (t<timeToZoomInAtStart)
		{
			static float startFOV = nearFOVPlane;
			float tToZoom = DW_CONSTRAIN(t / timeToZoomInAtStart,0.0f,1.0f);		
			newFOV = DW_SINE_ACCEL_DECEL_LERP(tToZoom,startFOV,newFOV); // override with initial zoom In
		}	
	}
		
	//-------------------------------------------------------
	// If not visible timeout eventually
	CColPoint colPoint;
	CEntity *pHitEntity;
	
	CWorld::pIgnoreEntity = pEntity;
	bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
	CWorld::pIgnoreEntity = NULL;							
	
	static int32 gTimeOutDefaultPlaneSpotter = 100;
	static int32 timeoutPlaneSpotter = gTimeOutDefaultPlaneSpotter;	

	if (!bLineOfSightToVehicleIsClear)
	{
		if (timeoutPlaneSpotter-- == 0)
		{		
			gbExitCam[camId] = true;	
			return false;	
		}
	}
	else
	{
		if (timeoutPlaneSpotter++>gTimeOutDefaultPlaneSpotter) timeoutPlaneSpotter = gTimeOutDefaultPlaneSpotter;
	}			
	
	if (IsTimeToExitThisDWCineyCamMode(MOVIECAM23,&src,&dst,t,false))
	{
		gbExitCam[camId] = true;	
		return false;
	}
	
	float nearClip = DW_LERP(newFOV/70.0f,10.0f,0.3f);
	
	Finalise_DW_CineyCams(&src, &dst, 0.0f, newFOV, nearClip,1.0f);

	return true;	
}

//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_DogFightCam(bool bCheckValid)
{
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack
	return false;	
}

//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_FishCam(bool bCheckValid)
{
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack
	return false;	
}

//---------------------------
// DW - Ask me about this.
float WaveFunc(float val, float min, float max, float freq)
{
	float dx = max-min;	// the range.
	float x  = val-min;	// the cur offset.
	float t  = x/dx;	// the 0-1 t val.
	// now as t goes from 0 to 1 we pass through the wave function
	
	float tBig = t * freq * 360.0f;	// make the range 0 to freq
	float y = CMaths::Sin(DEGTORAD(tBig));	
	return y;
}

//------------------------------------------------------------------------------------------------------
// follow the plane as if we are another plane.
bool CCam::Process_DW_PlaneCam1(bool bCheckValid)
{
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack

	if(!CamTargetEntity || !CamTargetEntity->GetIsTypeVehicle())
		return false;	

#ifndef MASTER	
	VarConsole.AddDebugOutput("DW-PLANECAM1-CAM");
#endif

	CEntity *pEntity;
	CVehicle *pVehicle;
	CVector dst,src,targetUp,targetRight,targetForward,targetMotion,targetAngMotion;
	float	targetVel,targetAngVel;
	CColSphere sph;
	GetCoreDataForDWCineyCamMode(&pEntity,&pVehicle,&dst,&src,&targetUp,&targetRight,&targetForward,&targetMotion,&targetVel,&targetAngMotion,&targetAngVel,&sph);
	int32 curTime = CTimer::GetTimeInMilliseconds();
	int32 camId = MOVIECAMPLANE1-MOVIECAM20;
	
	static float dirMove;
	static float randSign = 1.0f;
	float distanceToGround = dst.z; // an absolute guess... not strictly correct, but ray cast is clunky ( I'd rather have a distance to sector bounding box max z, but this doesnt exist!)
	static float gDistanceToGroundToDoThisCam = 80.0f;

	if (distanceToGround < gDistanceToGroundToDoThisCam)
	{
		gbExitCam[camId] = true;	
		return false;	
	}
		
	if (gDWLastModeForCineyCam != MODE_DW_PLANECAM1 || gLastFrameProcessedDWCineyCam < CTimer::m_FrameCounter - 1)
	{	// initialise in here
		gbExitCam[camId] = false;
		gDWLastModeForCineyCam = MODE_DW_PLANECAM1;	
		gDWCineyCamStartTime = curTime;
		gDWCineyCamEndTime	 = gDWCineyCamStartTime + gDWCamMaxDurationMs[camId];
		
		bool bLineOfSightToVehicleIsNotClear = false;

		CColPoint colPoint;
		CEntity *pHitEntity;
			
		CWorld::pIgnoreEntity = pEntity;
		bLineOfSightToVehicleIsNotClear = CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
		CWorld::pIgnoreEntity = NULL;							
			
		if (bLineOfSightToVehicleIsNotClear)
		{
			gbExitCam[camId] = true;	
			return false;
		}	
		
		// go above to below or vica versa
		dirMove = 1.0f;
		if (CGeneral::GetRandomTrueFalse()) 	
			dirMove *= -1.0f;
			
		if (CGeneral::GetRandomTrueFalse()) 				
			randSign = -1.0f;
	}
	else
	{
		static float theCameraRoughDistToGround = 30.0f;
		if (TheCamera.GetRoughDistanceToGround() < theCameraRoughDistToGround)
		{
			gbExitCam[camId] = true;	
			return false;
		}	
	}

	float t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

	static float 	planeCamDistForward 			= 10.0f;
	static float 	planeCamDistSide    			= 30.0f;
	static float 	planeCamDistUp    	 			= 15.0f;
	static float 	planeAirWaveFloating 			= 0.5f; 
	static int 		planeAirWaveFloatingNumWaves	= 4;	
	static float 	planePitchYFactor				= -150.0f;
	CVector fwd 	= targetForward;
	CVector right 	= targetRight;
	
	fwd.Normalise();
	right.Normalise();
	
	right *= randSign;
	
	src = dst + fwd * planeCamDistForward;
	src += right * planeCamDistSide;
	src += targetUp * planeCamDistUp;
	src += targetUp * planePitchYFactor * (t-0.5f) * dirMove;


	float waveHeight = WaveFunc(curTime, gDWCineyCamStartTime, gDWCineyCamEndTime, planeAirWaveFloatingNumWaves);
	src += targetUp * planeAirWaveFloating * waveHeight;

	float newFOV = 70.0f;
		
	//-------------------------------------------------------
	// If not visible timeout eventually
	CColPoint colPoint;
	CEntity *pHitEntity;
	
	CWorld::pIgnoreEntity = pEntity;
	bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
	CWorld::pIgnoreEntity = NULL;							
	
	static int32 gTimeOutDefaultPlaneCam1 = 100;
	static int32 timeoutPlaneCam1 = gTimeOutDefaultPlaneCam1;	


	if (!bLineOfSightToVehicleIsClear)
	{
		if (timeoutPlaneCam1-- == 0)
		{		
			gbExitCam[camId] = true;	
			return false;	
		}
	}
	else
	{
		if (timeoutPlaneCam1++>gTimeOutDefaultPlaneCam1) timeoutPlaneCam1 = gTimeOutDefaultPlaneCam1;
	}			
	
	if (IsTimeToExitThisDWCineyCamMode(MOVIECAMPLANE1,&src,&dst,t,false))
	{
		gbExitCam[camId] = true;	
		return false;
	}
	
//	float nearClip = DW_LERP(newFOV/70.0f,10.0f,0.3f);
	
	Finalise_DW_CineyCams(&src, &dst, 0.0f, newFOV, 5.0f ,1.0f);

	return true;	
}

//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_PlaneCam2(bool bCheckValid)
{
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack

	if(!CamTargetEntity || !CamTargetEntity->GetIsTypeVehicle())
		return false;	

#ifndef MASTER	
	VarConsole.AddDebugOutput("DW-PLANECAM2-CAM");
#endif

	CEntity *pEntity;
	CVehicle *pVehicle;
	CVector dst,src,targetUp,targetRight,targetForward,targetMotion,targetAngMotion;
	float	targetVel,targetAngVel;
	CColSphere sph;
	GetCoreDataForDWCineyCamMode(&pEntity,&pVehicle,&dst,&src,&targetUp,&targetRight,&targetForward,&targetMotion,&targetVel,&targetAngMotion,&targetAngVel,&sph);
	int32 curTime = CTimer::GetTimeInMilliseconds();
	int32 camId = MOVIECAMPLANE2-MOVIECAM20;

	static float dirMove2,dirMove3;
	static float randSign2 = 1.0f;
	
	float distanceToGround = dst.z; // an absolute guess... not strictly correct, but ray cast is clunky ( I'd rather have a distance to sector bounding box max z, but this doesnt exist!)
	static float gDistanceToGroundToDoThisCam = 80.0f;
	if (distanceToGround < gDistanceToGroundToDoThisCam)
	{
		gbExitCam[camId] = true;	
		return false;	
	}		

	if (gDWLastModeForCineyCam != MODE_DW_PLANECAM2 || gLastFrameProcessedDWCineyCam < CTimer::m_FrameCounter - 1)
	{	// initialise in here
		gbExitCam[camId] = false;
		gDWLastModeForCineyCam = MODE_DW_PLANECAM2;	
		gDWCineyCamStartTime = curTime;
		gDWCineyCamEndTime	 = gDWCineyCamStartTime + gDWCamMaxDurationMs[camId];
		
		bool bLineOfSightToVehicleIsNotClear = false;

		CColPoint colPoint;
		CEntity *pHitEntity;
			
		CWorld::pIgnoreEntity = pEntity;
		bLineOfSightToVehicleIsNotClear = CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
		CWorld::pIgnoreEntity = NULL;							
			
		if (bLineOfSightToVehicleIsNotClear)
		{
			gbExitCam[camId] = true;	
			return false;
		}		
		
		// go above to below or vica versa
		dirMove2 = 1.0f;
		if (CGeneral::GetRandomTrueFalse()) 	
			dirMove2 *= -1.0f;		
			
		// go above to below or vica versa
		dirMove3 = 1.0f;
		if (CGeneral::GetRandomTrueFalse()) 	
			dirMove3 *= -1.0f;	
			
		if (CGeneral::GetRandomTrueFalse()) 				
			randSign2 = -1.0f;						
	}

	float t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

	static float 	planeCamDistForward2 			= 30.0f;
	static float 	planeCamDistSide2    			= 30.0f;
	static float 	planeCamDistUp2    	 			= 5.0f;
	static float 	planeAirWaveFloating2 			= 0.5f; 
	static int 		planeAirWaveFloatingNumWaves2	= 4;	
	CVector fwd 	= targetForward;
	CVector right 	= targetRight;
	
//	fwd.z 	= 0.0f;
//	right.z = 0.0f;
	fwd.Normalise();
	right.Normalise();

	right 	*= 1.0f - t;
	fwd 	*= DW_LERP(t,1.0f,-1.0f);
	fwd 	*= dirMove3;

	right *= randSign2;
			
	src = dst + fwd * planeCamDistForward2;
	src += right * planeCamDistSide2;
	src += targetUp * planeCamDistUp2;


	float waveHeight = WaveFunc(curTime, gDWCineyCamStartTime, gDWCineyCamEndTime, planeAirWaveFloatingNumWaves2);
	src += targetUp * planeAirWaveFloating2 * waveHeight;

	float newFOV = 70.0f;
		
	//-------------------------------------------------------
	// If not visible timeout eventually
	CColPoint colPoint;
	CEntity *pHitEntity;
	
	CWorld::pIgnoreEntity = pEntity;
	bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
	CWorld::pIgnoreEntity = NULL;							
	
	static int32 gTimeOutDefaultPlaneCam2 = 100;
	static int32 timeoutPlaneCam2= gTimeOutDefaultPlaneCam2;	

	if (!bLineOfSightToVehicleIsClear)
	{
		if (timeoutPlaneCam2-- == 0)
		{		
			gbExitCam[camId] = true;	
			return false;	
		}
	}
	else
	{
		if (timeoutPlaneCam2++>gTimeOutDefaultPlaneCam2) timeoutPlaneCam2 = gTimeOutDefaultPlaneCam2;
	}			
	
	if (IsTimeToExitThisDWCineyCamMode(MOVIECAMPLANE2,&src,&dst,t,false))
	{
		gbExitCam[camId] = true;	
		return false;
	}
	
//	float nearClip = DW_LERP(newFOV/70.0f,10.0f,0.3f);
	
	Finalise_DW_CineyCams(&src, &dst, 0.0f, newFOV, 5.0f ,1.0f);

	return true;}


//------------------------------------------------------------------------------------------------------
bool CCam::Process_DW_PlaneCam3(bool bCheckValid)
{
	TheCamera.m_bUseNearClipScript=false; // Get the fuck out of my code you piece of shit hack

	if(!CamTargetEntity || !CamTargetEntity->GetIsTypeVehicle())
		return false;	

#ifndef MASTER	
	VarConsole.AddDebugOutput("DW-PLANECAM3-CAM");
#endif

	CEntity *pEntity;
	CVehicle *pVehicle;
	CVector dst,src,targetUp,targetRight,targetForward,targetMotion,targetAngMotion;
	float	targetVel,targetAngVel;
	CColSphere sph;
	GetCoreDataForDWCineyCamMode(&pEntity,&pVehicle,&dst,&src,&targetUp,&targetRight,&targetForward,&targetMotion,&targetVel,&targetAngMotion,&targetAngVel,&sph);
	int32 curTime = CTimer::GetTimeInMilliseconds();
	int32 camId = MOVIECAMPLANE3-MOVIECAM20;


	float distanceToGround = dst.z; // an absolute guess... not strictly correct, but ray cast is clunky ( I'd rather have a distance to sector bounding box max z, but this doesnt exist!)
	static float gDistanceToGroundToDoThisCam = 80.0f;
	if (distanceToGround < gDistanceToGroundToDoThisCam)
	{
		gbExitCam[camId] = true;	
		return false;	
	}		

	if (gDWLastModeForCineyCam != MODE_DW_PLANECAM3 || gLastFrameProcessedDWCineyCam < CTimer::m_FrameCounter - 1)
	{	// initialise in here
		gbExitCam[camId] = false;
		gDWLastModeForCineyCam = MODE_DW_PLANECAM3;	
		gDWCineyCamStartTime = curTime;
		gDWCineyCamEndTime	 = gDWCineyCamStartTime + gDWCamMaxDurationMs[camId];
		
		bool bLineOfSightToVehicleIsNotClear = false;

		CColPoint colPoint;
		CEntity *pHitEntity;
			
		CWorld::pIgnoreEntity = pEntity;
		bLineOfSightToVehicleIsNotClear = CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
		CWorld::pIgnoreEntity = NULL;							
			
		if (bLineOfSightToVehicleIsNotClear)
		{
			gbExitCam[camId] = true;	
			return false;
		}		
	}

	float t = (float)(curTime - gDWCineyCamStartTime) / (float)(gDWCineyCamEndTime - gDWCineyCamStartTime);

	static float fwd1 = 15.0f;
	static float up1  = 5.0f;

	CVector d = CModelInfo::GetBoundingBox(pEntity->m_nModelIndex).GetBoundBoxMax() - CModelInfo::GetBoundingBox(pEntity->m_nModelIndex).GetBoundBoxMin();
	d*=0.5f;
	fwd1 = d.y * 2.0f;

	src = dst + targetForward * fwd1;
	src += targetUp * up1;
	
	float newFOV = 70.0f;

	//-------------------------------------------------------
	// If not visible timeout eventually
	CColPoint colPoint;
	CEntity *pHitEntity;
	
	CWorld::pIgnoreEntity = pEntity;
	bool bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(dst, src, colPoint, pHitEntity, true, true, false, false, false, false, false);
	CWorld::pIgnoreEntity = NULL;							
	
	static int32 gTimeOutDefaultPlaneCam3 = 100;
	static int32 timeoutPlaneCam3 = gTimeOutDefaultPlaneCam3;	

	if (!bLineOfSightToVehicleIsClear)
	{
		if (timeoutPlaneCam3-- == 0)
		{		
			gbExitCam[camId] = true;	
			return false;	
		}
	}
	else
	{
		if (timeoutPlaneCam3++>gTimeOutDefaultPlaneCam3) timeoutPlaneCam3 = gTimeOutDefaultPlaneCam3;
	}			
	
	if (IsTimeToExitThisDWCineyCamMode(MOVIECAMPLANE3,&src,&dst,t,false))
	{
		gbExitCam[camId] = true;	
		return false;
	}
	
	Finalise_DW_CineyCams(&src, &dst, 0.0f, newFOV, 5.0f ,1.0f);

	return true;
}



//----------------------------------------------------
bool CCam::IsTimeToExitThisDWCineyCamMode(int32 camId, CVector *pSrc, CVector *pDst,float t, bool bLineOfSightCheck)
{	
	camId -= MOVIECAM20;
	
	if (gbExitCam[camId])
		return true;	
	
	float *pMin = &gMovieCamMinDist[camId];
	float *pMax = &gMovieCamMaxDist[camId];
	
	CVector Diff = *pDst - *pSrc;
	float len = Diff.Magnitude();
	bool bLenInDistLimits =  (len >= *pMin && len <= *pMax);
		
	CEntity 	*pEntity 		= CamTargetEntity;
	CVehicle 	*pVehicle 		= (CVehicle*)pEntity;
	
	bool bLineOfSightToVehicleIsClear = true;
	
	if (bLineOfSightCheck)
	{	
		CColPoint colPoint;
		CEntity *pHitEntity;
		
		CWorld::pIgnoreEntity = pEntity;
		bLineOfSightToVehicleIsClear = !CWorld::ProcessLineOfSight(*pDst, *pSrc, colPoint, pHitEntity, true, true, false, false, false, false, false);
		CWorld::pIgnoreEntity = NULL;
	}
	
	bool bTimeExpired = CTimer::GetTimeInMilliseconds()>gDWCineyCamEndTime;
	
	switch (camId+MOVIECAM20)
	{
		case MOVIECAM20:
		case MOVIECAM21:		
		case MOVIECAM22:
		case MOVIECAM23:
		case MOVIECAM24:		
		case MOVIECAM25:
		case MOVIECAMPLANE1:
		case MOVIECAMPLANE2:
		case MOVIECAMPLANE3:						
			if (!bLenInDistLimits || !bLineOfSightToVehicleIsClear || bTimeExpired)
				return true;
			break;
		default : 
			ASSERT(false);
			break;
	}
			
	return false;		
}

				
				
































































































































////////////////////////////////////////////////////
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
void CCam::Process_Syphon_Crim_In_Front(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
#if 0  //XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
	FOV=70.0f;
	if (CamTargetEntity->GetIsTypePed())
	{
		bool CameraObscured=false;
		float MinimumDistCrimToPlayer=2.5;
		float Crim_Player_Cam_Angle=0.0f;
		float MimimumDistPlayerToCam=4.0f;
		float CurrentDistOnGround=0.0f;
		float DistanceOnGround=0.0f;
		float Length=0.0f;
		float PedToPlayerAngle=0.0f;
		float IdealBeta=0.0f;
		float DeltaBeta=0.0f;
		float ReqSpeed=0.0f;
		float SourceZOffSet=0.0f;
		CVector PlayerToCrimVector;
		CVector PlayerToCamVector;
		CVector Dist;
		CVector TargetCoors;
		CColPoint colPoint;
		
		//first move the camera like the normal player mode
		TargetCoors = ThisCamsTarget;
		DistanceOnGround=MimimumDistPlayerToCam + (TheCamera.m_fPedZoomSmoothed/2.0f); 
		//this variable is needed if we go behind a wall
		Dist = Source - TargetCoors;
		CurrentDistOnGround=CMaths::Sqrt(Dist.x * Dist.x + Dist.y * Dist.y);
		//////


		Length = CMaths::Sqrt(Dist.x * Dist.x + Dist.y * Dist.y);
		SourceZOffSet=(DistanceOnGround-2.65f);
		if (SourceZOffSet<0.0f)
		{
			SourceZOffSet=0.0f;
		}

		if (Length != 0.0f)
		{
			Source = TargetCoors + CVector( Dist.x * DistanceOnGround / Length,
										Dist.y * DistanceOnGround / Length,
										SourceZOffSet);//This is like finding a unit vector
										//from the player to the camera after the player has moved.
		}
		else
		{
			Source = TargetCoors + CVector(1.0f, 1.0f, SourceZOffSet);
		}

		PedToPlayerAngle=CGeneral::GetATanOfXY((TheCamera.m_cvecAimingTargetCoors.x - TargetCoors.x), (TheCamera.m_cvecAimingTargetCoors.y - TargetCoors.y)); 

		while (PedToPlayerAngle>=2.0f * PI) {PedToPlayerAngle-=2.0f * PI;}
		while (PedToPlayerAngle<0.0f) {PedToPlayerAngle+=2.0f * PI;}

		if (ResetStatics==true)
		{
			if (PedToPlayerAngle>0.0f)
			{
				m_fPlayerInFrontSyphonAngleOffSet=-m_fPlayerInFrontSyphonAngleOffSet;
			}
			ResetStatics=false;
		}

		IdealBeta=PedToPlayerAngle + m_fPlayerInFrontSyphonAngleOffSet;

		if (TheCamera.PlayerWeaponMode.Mode!=MODE_SYPHON)	//know that this seems weird, it's to stop the interpolation fucking up
		{
			IdealBeta=Beta;
		}	
		else
		{
			Beta=IdealBeta;
		}
				
		Source.x=TargetCoors.x;
		Source.y=TargetCoors.y;

		Source.x+=  (CMaths::Cos(Beta) * DistanceOnGround) ;
		Source.y+=  (CMaths::Sin(Beta) * DistanceOnGround) ;


		//now check If Camera is behind a wall
		CVector CurrentDist, DistancePedToWall;
		float TargetDistance=0.0f;
		float DeltaDistance=0.0f;
		if (CameraObscured==TRUE)

		TargetCoors=ThisCamsTarget;
		TargetCoors.z+=m_fSyphonModeTargetZOffSet;
		m_cvecTargetCoorsForFudgeInter=TargetCoors;
		CVector TempSource=Source;
		TheCamera.AvoidTheGeometry(TempSource,TargetCoors, Source, FOV);
	
		Front=TargetCoors-Source;
		GetVectorsReadyForRW();
	}
#endif	
}

#if 0
// onfoot, inbike, incar
CamFollowHistData FHISTORY_SETTINGS[FHISTORY_RCHELI+1] =
{
//	res		maxpts	cDown	maxDist	targZ	sFwd	baseDst baseZ	tRate	tCap   	aRate	aCap	aSRate	bRate	bCap	bSRate	cRate	cCap	upLimit			downLimit
	{1.0f,	32,		500, 	15.0f,	1.1f,	-1.0f,	1.0f,	0.10f,	0.1f, 	0.05f,	0.2f,	0.05f,	0.2f,	0.2f,	0.03f,	0.2f,	0.05f,	0.002f,	DEGTORAD(45.0f), DEGTORAD(89.0f)},	// onbike
	{1.0f,	32,		500, 	15.0f,	1.1f,	-1.0f,	1.0f,	0.20f,	0.1f, 	0.05f,	0.2f,	0.05f,	0.2f,	0.2f,	0.03f,	0.2f,	0.05f,	0.002f,	DEGTORAD(45.0f), DEGTORAD(89.0f)},	// incar
	{2.0f,	16,		5000, 	15.0f,	1.1f,	-1.0f,	1.0f,	0.20f,	0.1f, 	0.05f,	0.2f,	0.05f,	0.2f,	0.2f,	0.03f,	0.2f,	0.05f,	0.002f,	DEGTORAD(20.0f), DEGTORAD(70.0f)},	// inheli
	{2.0f,	32,		5000, 	25.0f,	1.1f,	-1.0f,	1.0f,	0.20f,	0.1f, 	0.05f,	0.2f,	0.05f,	0.2f,	0.2f,	0.03f,	0.2f,	0.05f,	0.002f,	DEGTORAD(89.0f), DEGTORAD(89.0f)},	// inplane
	{0.5f,	32,		500, 	7.0f,	1.1f,	-1.0f,	1.0f,	0.20f,	0.1f, 	0.05f,	0.2f,	0.05f,	0.2f,	0.2f,	0.03f,	0.2f,	0.05f,	0.002f,	DEGTORAD(45.0f), DEGTORAD(89.0f)},	// rccar
	{0.5f,	16,		5000, 	7.0f,	1.1f,	0.05f,	1.0f,	0.20f,	0.1f, 	0.05f,	0.2f,	0.05f,	0.2f,	0.2f,	0.03f,	0.2f,	0.05f,	0.002f,	DEGTORAD(20.0f), DEGTORAD(70.0f)}	// rcheli
};
#endif
//
void CCam::Process_Follow_History(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired)
{
#if 0 //XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX

	int i=0;
	int nType = 0;
	if(!CamTargetEntity->GetIsTypeVehicle())
		return;
		
#ifndef MASTER
	if(CTimer::bSkipProcessThisFrame)	
		return;
#endif

	CVehicle *pTargetVehicle = (CVehicle *)CamTargetEntity;
	CVector vecTargetCoords = ThisCamsTarget;
	
	float fCamDistance = 0.0f;
	float fCamAlpha = 0.0f;
	float fLimitHistDist = 5.0f;
	int32 nMinHistPts = 1;
	
	if(CamTargetEntity->GetModelIndex()==MODELID_CAR_RCBANDIT || CamTargetEntity->GetModelIndex()==MODELID_CAR_RCBARON
	|| CamTargetEntity->GetModelIndex()==MODELID_CAR_RCTIGER || CamTargetEntity->GetModelIndex()==MODELID_CAR_RCCAM)
		nType = FHISTORY_RCCAR;
	else if(CamTargetEntity->GetModelIndex()==MODELID_CAR_RCCOPTER || CamTargetEntity->GetModelIndex()==MODELID_CAR_RCGOBLIN)
		nType = FHISTORY_RCHELI;
	else if(pTargetVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE
	|| pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_QUADBIKE)
		nType = FHISTORY_ONBIKE;
	else if(pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_HELI)
		nType = FHISTORY_INHELI;
	else if(pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_PLANE)
	{
		if(pTargetVehicle->GetModelIndex()==MODELID_PLANE_HARRIER
		&& ((CAutomobile *)pTargetVehicle)->m_nSuspensionHydraulics >= CPlane::HARRIER_NOZZLE_SWITCH_LIMIT)
			nType = FHISTORY_INHELI;
		else
			nType = FHISTORY_INPLANE;
	}
	else
		nType = FHISTORY_INCAR;
		
//	TheCamera.m_fCarZoomTotal = FOLLOW_SETTINGS[nType].fBaseCamDist;
//	TheCamera.m_fCarZoomTotal += TheCamera.m_fCarZoomBase;
	fCamDistance = TheCamera.m_fCarZoomSmoothed + FHISTORY_SETTINGS[nType].fBaseCamDist;

	int PositionInArray = 0;
	TheCamera.GetArrPosForVehicleType(pTargetVehicle->GetVehicleAppearance(), PositionInArray);

	float CarHeight = CamTargetEntity->GetColModel().GetBoundBoxMax().z;
	float CarLength = (CamTargetEntity->GetColModel().GetBoundBoxMax() - CamTargetEntity->GetColModel().GetBoundBoxMin()).Magnitude();
	// bikes are pretty small so camera ends up too close
	if(pTargetVehicle->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
		CarLength *= 1.45f;
	else if(pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK)
	{
		static float MONSTER_TRUCK_CAMDIST_MULT = 1.2f;
		static float MONSTER_TRUCK_CAMHEIGHT_MULT = 0.65f;
		CarLength *= MONSTER_TRUCK_CAMDIST_MULT;
		CarHeight *= MONSTER_TRUCK_CAMHEIGHT_MULT;
	}
	else if(pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_PLANE)
	{
		static float PLANE_DIST_MULT = 1.3f;
		CarLength = PLANE_DIST_MULT*CamTargetEntity->GetColModel().GetBoundBoxMin().Magnitude();
	}
	
	// need to add on length for trailer if we're towing
	if(pTargetVehicle->m_pVehicleBeingTowed)
	{
		static float TRAILER_CAMDIST_MULT = 0.5f;
		CarLength += TRAILER_CAMDIST_MULT*(pTargetVehicle->m_pVehicleBeingTowed->GetColModel().GetBoundBoxMax() - pTargetVehicle->m_pVehicleBeingTowed->GetColModel().GetBoundBoxMin()).Magnitude();
		CarHeight = MAX(CarHeight, pTargetVehicle->m_pVehicleBeingTowed->GetColModel().GetBoundBoxMax().z);

		vecTargetCoords = 0.5f*(vecTargetCoords + pTargetVehicle->m_pVehicleBeingTowed->GetPosition());
	}

//	TheCamera.m_fCarZoomTotal += CarLength;
	fCamDistance += CarLength;

	// use a different offset for full-size heli's (not r/c ones)
	if(pTargetVehicle->GetVehicleAppearance()==APR_HELI && CamTargetEntity->GetStatus()!=STATUS_PLAYER_REMOTE)
		vecTargetCoords += CamTargetEntity->GetMatrix().GetUp()*fTestShiftHeliCamTarget*CarHeight;
	else
	{
		vecTargetCoords.z -= FHISTORY_SETTINGS[nType].fBaseCamZ;
		vecTargetCoords.z += CarHeight*FHISTORY_SETTINGS[nType].fTargetOffsetZ;
	}

	fCamAlpha = 0.0f;//FOLLOW_SETTINGS[nType].fBaseCamZ;
	if (TheCamera.m_nCarZoom==ZOOM_ONE)
		fCamAlpha += ZmOneAlphaOffset[PositionInArray];
	else if (TheCamera.m_nCarZoom==ZOOM_TWO)
		fCamAlpha += ZmTwoAlphaOffset[PositionInArray];
	else if (TheCamera.m_nCarZoom==ZOOM_THREE)
		fCamAlpha += ZmThreeAlphaOffset[PositionInArray];

	if(ResetStatics)
		FOV=70.f;
	else
	{
		if((pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_CAR || pTargetVehicle->GetVehicleType()==VEHICLE_TYPE_BIKE)
		&& DotProduct(pTargetVehicle->GetMoveSpeed(), pTargetVehicle->GetMatrix().GetForward()) > CAR_FOV_START_SPEED)
			FOV += (DotProduct(pTargetVehicle->GetMoveSpeed(), pTargetVehicle->GetMatrix().GetForward()) - CAR_FOV_START_SPEED)*CTimer::GetTimeStep();
		
		if(FOV > 70.0f)
			FOV = 70.0f + (FOV - 70.0f)*CMaths::Pow(CAR_FOV_FADE_MULT, CTimer::GetTimeStep());
			
		if(FOV > 100.0f)
			FOV = 100.0f;
		else if(FOV < 70.0f)
			FOV = 70.0f;
	}

//	fCamDistance = TheCamera.m_fCarZoomSmoothed;

	// damn globals passed between functions!
	CA_MAX_DISTANCE=fCamDistance;//TheCamera.m_fCarZoomTotal;
	CA_MIN_DISTANCE=3.5f;

	fLimitHistDist = FHISTORY_SETTINGS[nType].fMaxDist;
	nMinHistPts = 5;

	CVector vecNewTrailCoords = vecTargetCoords;
	if(ResetStatics || TheCamera.m_bCamDirectlyBehind) //because camera has been re-initialised
	{
		m_aTargetHistoryPos[0] = vecTargetCoords - 2.5f*FHISTORY_SETTINGS[nType].fResolution*pTargetVehicle->GetMatrix().GetForward();
		m_nTargetHistoryTime[0] = FHISTORY_SETTINGS[nType].nCountDown;
		m_aTargetHistoryPos[1] = vecTargetCoords - 1.5f*FHISTORY_SETTINGS[nType].fResolution*pTargetVehicle->GetMatrix().GetForward();
		m_nTargetHistoryTime[1] = FHISTORY_SETTINGS[nType].nCountDown;
		m_nCurrentHistoryPoints = 2;
		
		vecNewTrailCoords -= 1.0f*FHISTORY_SETTINGS[nType].fResolution*pTargetVehicle->GetMatrix().GetForward();

		Beta = pTargetVehicle->GetHeading() - HALF_PI;
		BetaSpeed = 0.0f;
		Alpha = -fCamAlpha;
		AlphaSpeed = 0.0f;
		Rotating=FALSE;
		m_bCollisionChecksOn=true;
		Distance = 1000.0f;
		
		if(fCamDistance==TheCamera.m_fCarZoomBase)
			fCamDistance = TheCamera.m_fCarZoomSmoothed = TheCamera.m_fCarZoomTotal;
		
		ResetStatics  = false;
	}
	else
	{
		float fFwdOffsetMult = 0.0f;
		if(pTargetVehicle->GetMoveSpeed().MagnitudeSqr() < FHISTORY_SETTINGS[nType].fTargetSlowFwdMult*FHISTORY_SETTINGS[nType].fTargetSlowFwdMult)
		{
			fFwdOffsetMult = 2.0f - pTargetVehicle->GetMoveSpeed().Magnitude()  / FHISTORY_SETTINGS[nType].fTargetSlowFwdMult;
			if(fFwdOffsetMult > 1.0f)	fFwdOffsetMult = 1.0f;
			fFwdOffsetMult *= -FHISTORY_SETTINGS[nType].fResolution;
		}
		
		if(fFwdOffsetMult < 0.0f)
			vecNewTrailCoords += fFwdOffsetMult*pTargetVehicle->GetMatrix().GetForward();
	}
	
	// count how many points at the tail of the list are too far from the target now
//	CVector vecNewPos = ThisCamsTarget + CAM_HISTORY_PED_OFFSET*CamTargetEntity->GetMatrix().GetForward();
	if(m_nCurrentHistoryPoints)
	{
		int32 nInvalidCount = 0;
		bool bContinueDistCheck = true;
		if(m_nCurrentHistoryPoints > nMinHistPts)
		{
			if(m_nTargetHistoryTime[0] > 0)
			{
				uint32 nDecrementTime = CTimer::GetTimeElapsedInMilliseconds();
				if(m_nCurrentHistoryPoints > FHISTORY_SETTINGS[nType].nMaxNumPts-1)
					nDecrementTime *= 1 + m_nCurrentHistoryPoints - (FHISTORY_SETTINGS[nType].nMaxNumPts - 1);
				if((m_aTargetHistoryPos[0] - vecTargetCoords).MagnitudeSqr() > fLimitHistDist*fLimitHistDist)
					nDecrementTime *= MAX(2, (uint32)CMaths::Ceil(10.0f*(m_aTargetHistoryPos[0] - vecTargetCoords).Magnitude()/FHISTORY_SETTINGS[nType].fMaxDist));
				if(Rotating)
					nDecrementTime *= 10;
				
				if(m_nTargetHistoryTime[0] > nDecrementTime)
					m_nTargetHistoryTime[0] -= nDecrementTime;
				else
				{
					nDecrementTime -= m_nTargetHistoryTime[0];
					m_nTargetHistoryTime[0] = 0;
					if(m_nTargetHistoryTime[1] > nDecrementTime	&& m_nCurrentHistoryPoints > 2)
						m_nTargetHistoryTime[1] -= nDecrementTime;
				}
			}

			for(i=0; i < m_nCurrentHistoryPoints-1; i++)
			{
				if(m_nTargetHistoryTime[i]==0)
					nInvalidCount++;
				else
					break;
			}
		}
		
		// if list is full then force 1 at tail to be removed, otherwise we won't be able
		// to add a new point
//if(USE_NEW_DATA){
//		if(nInvalidCount==0 && m_nCurrentHistoryPoints > FOLLOW_SETTINGS[nType].nMaxNumPts-1)
//			nInvalidCount = 1;
//}
//else{
		if(nInvalidCount==0 && m_nCurrentHistoryPoints > CAM_NUM_TARGET_HISTORY-1)
			nInvalidCount = 1;
//}	
		// pop the invalid entries off the list
		if(nInvalidCount > 0)
		{
			for(i=0; i<m_nCurrentHistoryPoints - nInvalidCount; i++)
			{
				m_aTargetHistoryPos[i] = m_aTargetHistoryPos[i+nInvalidCount];
				m_nTargetHistoryTime[i] = m_nTargetHistoryTime[i+nInvalidCount];
			}
			
			m_nCurrentHistoryPoints -= nInvalidCount;
		}
		
		if((vecNewTrailCoords - m_aTargetHistoryPos[m_nCurrentHistoryPoints - 1]).MagnitudeSqr() > FHISTORY_SETTINGS[nType].fResolution*FHISTORY_SETTINGS[nType].fResolution
		&& (m_nCurrentHistoryPoints <= 1 || (vecNewTrailCoords - m_aTargetHistoryPos[m_nCurrentHistoryPoints - 2]).MagnitudeSqr() > FHISTORY_SETTINGS[nType].fResolution*FHISTORY_SETTINGS[nType].fResolution)
		&& (m_nCurrentHistoryPoints <= 2 || (vecNewTrailCoords - m_aTargetHistoryPos[m_nCurrentHistoryPoints - 3]).MagnitudeSqr() > FHISTORY_SETTINGS[nType].fResolution*FHISTORY_SETTINGS[nType].fResolution))
		{
//			float fDistRatio = (vecNewTrailCoords - m_aTargetHistoryPos[m_nCurrentHistoryPoints - 1]).Magnitude() / FOLLOW_SETTINGS[nType].fResolution;
//			m_nTargetHistoryTime[m_nCurrentHistoryPoints] = FHISTORY_SETTINGS[nType].nCountDown * fDistRatio;
			m_nTargetHistoryTime[m_nCurrentHistoryPoints] = FHISTORY_SETTINGS[nType].nCountDown;
			m_nTargetHistoryTime[m_nCurrentHistoryPoints-1] = FHISTORY_SETTINGS[nType].nCountDown;
			m_aTargetHistoryPos[m_nCurrentHistoryPoints] = vecNewTrailCoords;
			m_nCurrentHistoryPoints++;
			
			// check for intersections (in x-y plane) and remove inner loops
			CVector vecNewSeg = m_aTargetHistoryPos[m_nCurrentHistoryPoints-1] - m_aTargetHistoryPos[m_nCurrentHistoryPoints-2];
			if(m_nCurrentHistoryPoints > 4)
			for(i = m_nCurrentHistoryPoints-1; i>1; i--)
			{
				CVector vecOldSeg = m_aTargetHistoryPos[i] - m_aTargetHistoryPos[i-1];
				
				CVector vecTempRight(vecOldSeg.y, -vecOldSeg.x, 0.0f);

				CVector vecTempOffset = m_aTargetHistoryPos[m_nCurrentHistoryPoints-1] - m_aTargetHistoryPos[i];
				float fDot1 = DotProduct(vecTempRight, vecTempOffset);
				
				vecTempOffset = m_aTargetHistoryPos[m_nCurrentHistoryPoints-2] - m_aTargetHistoryPos[i];
				float fDot2 = DotProduct(vecTempRight, vecTempOffset);

				if( (fDot1 > 0.0f && fDot2 < 0.0f) || (fDot1 < 0.0f && fDot2 > 0.0f) )
				{
					vecTempRight = CVector(vecNewSeg.y, -vecNewSeg.x, 0.0f);
					
					vecTempOffset = m_aTargetHistoryPos[i] - m_aTargetHistoryPos[m_nCurrentHistoryPoints-1];
					fDot1 = DotProduct(vecTempRight, vecTempOffset);
					
					vecTempOffset = m_aTargetHistoryPos[i-1] - m_aTargetHistoryPos[m_nCurrentHistoryPoints-1];
					fDot2 = DotProduct(vecTempRight, vecTempOffset);
					
					if( (fDot1 > 0.0f && fDot2 < 0.0f) || (fDot1 < 0.0f && fDot2 > 0.0f) )
					{
						// ok have found a cross over, need to remove intermediate points
						m_aTargetHistoryPos[i] = m_aTargetHistoryPos[m_nCurrentHistoryPoints-1];
						m_nTargetHistoryTime[i] = m_nTargetHistoryTime[m_nCurrentHistoryPoints-1];
						m_nCurrentHistoryPoints = i+1;
					}
				}
			}
		}
		
	}
	else
	{
		m_aTargetHistoryPos[0] = vecTargetCoords;
		m_nTargetHistoryTime[0] = FHISTORY_SETTINGS[nType].nCountDown;
		m_nCurrentHistoryPoints = 1;
	}

	CVector vecHistCamPos;
	if(m_nCurrentHistoryPoints <= 1 || m_nTargetHistoryTime[0]==FHISTORY_SETTINGS[nType].nCountDown)
		vecHistCamPos = m_aTargetHistoryPos[0];
	else
	{
		float fTimeFrac = float(m_nTargetHistoryTime[0]) / float(FHISTORY_SETTINGS[nType].nCountDown);

		if(fTimeFrac > 1.0f)	fTimeFrac = 1.0f;
		else if(fTimeFrac < 0.0f)	fTimeFrac = 0.0f;

		vecHistCamPos = fTimeFrac*m_aTargetHistoryPos[0] + (1.0f - fTimeFrac)*m_aTargetHistoryPos[1];
	}	

//if(TEST_CAM_SOURCE_AT_POINT)
//	vecHistCamPos.z += fCamZ;
//else
//	vecHistCamPos.z += fCamZ * (vecTargetCoords - vecHistCamPos).Magnitude();

	Front = vecTargetCoords - vecHistCamPos;
	Front.Normalise();

	float fTargetBeta = CMaths::ATan2(-Front.x, Front.y) - HALF_PI;
	if(fTargetBeta < -PI)	fTargetBeta += TWO_PI;
	if(fTargetBeta - Beta > PI)	fTargetBeta -= TWO_PI;
	else if(fTargetBeta -Beta < -PI)	fTargetBeta += TWO_PI;

	float fTargetAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, Front.z))) - fCamAlpha;

#ifdef USE_SPEED_FOR_SWING_RATE
///////////////////////////////
	float fDiffCap = FHISTORY_SETTINGS[nType].fDiffBetaCap;
	float fDiffMult = FHISTORY_SETTINGS[nType].fDiffBetaRate;
	float fSpeedBlend = FHISTORY_SETTINGS[nType].fSpeedBetaRate*CTimer::GetTimeStep();

	float fCamBetaSpeed = fDiffMult*(fTargetBeta - Beta)/CMaths::Max(1.0f, CTimer::GetTimeStep());

	if(fCamBetaSpeed > fDiffCap)
		fCamBetaSpeed = fDiffCap;
	else if(fCamBetaSpeed < -fDiffCap)
		fCamBetaSpeed = -fDiffCap;

	BetaSpeed = fSpeedBlend*fCamBetaSpeed + (1.0f - fSpeedBlend)*BetaSpeed;
	Beta += BetaSpeed*CTimer::GetTimeStep();
	ClipBeta();
	
	
	fDiffCap = FHISTORY_SETTINGS[nType].fDiffAlphaCap;
	fDiffMult = FHISTORY_SETTINGS[nType].fDiffAlphaRate*CTimer::GetTimeStep();
	fSpeedBlend = FHISTORY_SETTINGS[nType].fSpeedAlphaRate*CTimer::GetTimeStep();

	if(fTargetAlpha > FHISTORY_SETTINGS[nType].fUpLimit)
		fTargetAlpha = FHISTORY_SETTINGS[nType].fUpLimit;
	else if(fTargetAlpha < -FHISTORY_SETTINGS[nType].fDownLimit)
		fTargetAlpha = -FHISTORY_SETTINGS[nType].fDownLimit;
	
	float fCamAlphaSpeed = fDiffMult*(fTargetAlpha - Alpha)/CMaths::Max(CTimer::GetTimeStep());

	// test to make camera pan up when going over crests of hills
	if(pTargetVehicle->GetIsTypeVehicle() && pTargetVehicle->GetMoveSpeed().MagnitudeSqr() > 0.01f)
	// and at least 1 wheel on ground?
	{
		float fMoveSpeed = pTargetVehicle->GetMoveSpeed().Magnitude();
		float fVelocityAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, pTargetVehicle->GetMoveSpeed().z/fMoveSpeed))) - fCamAlpha;
		if(fVelocityAlpha < fTargetAlpha)
		{
			fVelocityAlpha = FHISTORY_SETTINGS[nType].fCrestAlphaRate*(fVelocityAlpha - fTargetAlpha)*fMoveSpeed;
			if(fVelocityAlpha < -FHISTORY_SETTINGS[nType].fCrestAlphaCap)
				fVelocityAlpha = -FHISTORY_SETTINGS[nType].fCrestAlphaCap;

			fCamAlphaSpeed += fVelocityAlpha;
		}
	}
	
	if(fCamAlphaSpeed > fDiffCap)
		fCamAlphaSpeed = fDiffCap;
	else if(fCamAlphaSpeed < -fDiffCap)
		fCamAlphaSpeed = -fDiffCap;

	AlphaSpeed = fSpeedBlend*fCamAlphaSpeed + (1.0f - fSpeedBlend)*AlphaSpeed;
	Alpha += AlphaSpeed*CTimer::GetTimeStep();
	
	if(Alpha > FHISTORY_SETTINGS[nType].fUpLimit)
	{
		Alpha = FHISTORY_SETTINGS[nType].fUpLimit;
		AlphaSpeed = 0.0f;
	}
	else if(Alpha < -FHISTORY_SETTINGS[nType].fDownLimit)
	{
		Alpha = -FHISTORY_SETTINGS[nType].fDownLimit;
		AlphaSpeed = 0.0f;
	}
///////////////////////////////
#else
///////////////////////////////
	float fTargetDiff;
	fTargetDiff = FHISTORY_SETTINGS[nType].fDiffRate*(fTargetBeta - Beta);
	if(fTargetDiff > FHISTORY_SETTINGS[nType].fDiffCap)
		fTargetDiff = FHISTORY_SETTINGS[nType].fDiffCap;
	else if(fTargetDiff < -FHISTORY_SETTINGS[nType].fDiffCap)
		fTargetDiff = -FHISTORY_SETTINGS[nType].fDiffCap;
	fTargetDiff *= CTimer::GetTimeStep();

	Beta += fTargetDiff;
	ClipBeta();
	
	if(fTargetAlpha > FHISTORY_SETTINGS[nType].fUpLimit)
		fTargetAlpha = FHISTORY_SETTINGS[nType].fUpLimit;
	else if(fTargetAlpha < -FHISTORY_SETTINGS[nType].fDownLimit)
		fTargetAlpha = -FHISTORY_SETTINGS[nType].fDownLimit;

	fTargetDiff = FHISTORY_SETTINGS[nType].fDiffRate*(fTargetAlpha - Alpha);
	if(fTargetDiff > FHISTORY_SETTINGS[nType].fDiffCap)
		fTargetDiff = FHISTORY_SETTINGS[nType].fDiffCap;
	else if(fTargetDiff < -FHISTORY_SETTINGS[nType].fDiffCap)
		fTargetDiff = -FHISTORY_SETTINGS[nType].fDiffCap;
	fTargetDiff *= CTimer::GetTimeStep();
	
	Alpha += fTargetDiff;
///////////////////////////////
#endif
	
	Front = CVector(-CMaths::Cos(Beta) *CMaths::Cos(Alpha) , -CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
	GetVectorsReadyForRW();

	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

//	if(TEST_CAM_SOURCE_AT_POINT)
//		Source = vecHistCamPos;
//	else
		Source  = vecTargetCoords - Front * fCamDistance;

	m_cvecTargetCoorsForFudgeInter = vecTargetCoords;//ThisCamsTarget;

//////////////////////////////////////
// this chunk of code is pretty tempory, kinda copied from original follow ped
// and used in Process_AimWeapon(), Process_Follow_History(), and new Process_FollowPed()
// as a quick fix to stop the camera clipping through shit
// would like to smooth it all out and do some kind of predictive moving the camera forward like
// in MH, but don't want to start fucking around with the camera angles because that gets messy
//////////////////////////////////////
static float MAX_SPEED_CAM_HISTORY_REQUIRES_COL_TEST = 0.2f;
if(pTargetVehicle->GetMoveSpeed().Magnitude() < MAX_SPEED_CAM_HISTORY_REQUIRES_COL_TEST)
{
	CColPoint colPoint;
	CEntity *pHitEntity = NULL;
	CPed *pPed = NULL;
	if(pTargetVehicle->GetIsTypePed())
		pPed = (CPed *)pTargetVehicle;

	CWorld::pIgnoreEntity = (CEntity *) CamTargetEntity;
	// if ped is in a car then their collision will be off anyway
	// so set ignore peds vehicle instead
	if(pPed && pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle)
		CWorld::pIgnoreEntity = pPed->m_pMyVehicle;
		
	if(CWorld::ProcessLineOfSight(vecTargetCoords, Source, colPoint, pHitEntity, true, false, false, false, false,  false, true))
	{
		float fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
		float fDistCamToCol = fCamDistance - fDistColToPed;
		
		// if the first thing in the way of the camera is a ped, it might be ok to leave it
		// in the way - need to check there's nothing else closer to the camera tho!
		if(pHitEntity->GetIsTypePed() && fDistCamToCol > NORMAL_NEAR_CLIP + 0.1f)
		{
			if(CWorld::ProcessLineOfSight(colPoint.GetPosition(), Source, colPoint, pHitEntity, true, false, false, false, false, false, true))
			{
				fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
				Source = colPoint.GetPosition();
				if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
			}
			else
			{
				RwCameraSetNearClipPlane(Scene.camera, MIN(0.9f, fDistCamToCol - 0.35f));
			}
		}
		else
		{
			Source = colPoint.GetPosition();
			if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
				RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
		}
	}

	CWorld::pIgnoreEntity = NULL;
}
	int32 nCountLoop = 0;
	CVector vecTest;
	float fTanFOV = CMaths::Tan(0.5f*DEGTORAD(FOV));
	if(FrontEndMenuManager.m_PrefsUseWideScreen)
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio(); // ASPECTRATIO.
	else
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio();


	float fNearClip = RwCameraGetNearClipPlane(Scene.camera);
	float fNearClipWidth = fNearClip * fTanFOV;

	CEntity *pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
	while(pFindEntity)
	{
		vecTest = gaTempSphereColPoints[0].GetPosition() - Source;
		float fTemp1 = DotProduct(vecTest, Front);

		vecTest = vecTest - fTemp1 * Front;
		fTemp1 = vecTest.Magnitude() / fTanFOV;
		fTemp1 = MAX(0.1f, MIN(fNearClip, fTemp1));

		if(fTemp1 < fNearClip)
			RwCameraSetNearClipPlane(Scene.camera, fTemp1);
		
		// if we couldn't set the near clip close enough to avoid clipping
		// try moving the camera forward by a percentage of the distance to ped
//		if(fTemp1 == 0.1f && !(pPed && pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle==pFindEntity))
//			Source = Source + 0.3f * (vecTargetCoords - Source);
		
		// test again incase we got the wrong collision point or stuff
		fNearClip = RwCameraGetNearClipPlane(Scene.camera);
		fNearClipWidth = fNearClip * CMaths::Tan(0.5f*DEGTORAD(FOV)) * fTweakFOV*CDraw::GetAspectRatio();
		pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
		
		// break out anyway after a specified number of tries
		if(++nCountLoop > 5)
			pFindEntity = NULL;
	}


	// now buffer how quickly the camera moves back away from the player too smooth things out a bit
	float fNewDistance = (vecTargetCoords - Source).Magnitude();
	if(fNewDistance < Distance)
	{
		Distance = fNewDistance; //fine
	}
	else// if(Distance < fNewDistance)
	{
		// buffered Distance goes towards fNewDistance (not fIdealDistance)
		float fTimeRateOfChange = CMaths::Pow(fMouseAvoidGeomReturnRate, CTimer::GetTimeStep());
		Distance = fTimeRateOfChange*Distance + (1.0f - fTimeRateOfChange)*fNewDistance;
		
		if(fNewDistance > 0.05f) // just in case
			Source = vecTargetCoords + (Source - vecTargetCoords)*Distance/fNewDistance;
		
		// now we might have ended up closer to the player than AvoidTheGeometry expected
		// so might clip through playerped
		float PlayCamDistWithCol = Distance-fRangePlayerRadius;
		if (PlayCamDistWithCol<RwCameraGetNearClipPlane(Scene.camera))	
		{
			RwCameraSetNearClipPlane(Scene.camera, MAX(PlayCamDistWithCol, 0.1f));
		}	
	}

	TheCamera.m_bCamDirectlyBehind=false;
	TheCamera.m_bCamDirectlyInFront=false;

	GetVectorsReadyForRW();
////////////////////////////
// end of nasty chunk
////////////////////////////

	if(CPlayerPed::bDebugPlayerInfo)
	{
		CDebug::RenderDebugSphere3D(Source.x, Source.y, Source.z, 0.2f);
		CDebug::DebugLine3D(Source, Source + 2.0f*Front, 0x00ff00ff, 0x00ff00ff);
		CDebug::DebugLine3D(Source, Source + 2.0f*Up, 0x0000ffff, 0x0000ffff);

		for(i=0; i<m_nCurrentHistoryPoints; i++)
		{
			CDebug::DebugLine3D(m_aTargetHistoryPos[i] + CVector(0.0f,0.0f,0.5f), m_aTargetHistoryPos[i] - CVector(0.0f,0.0f,0.5f), 0xffffffff, 0xffffffff);
			CDebug::DebugLine3D(m_aTargetHistoryPos[i] + CVector(0.0f,0.5f,0.0f), m_aTargetHistoryPos[i] - CVector(0.0f,0.5f,0.0f), 0xffffffff, 0xffffffff);
			CDebug::DebugLine3D(m_aTargetHistoryPos[i] + CVector(0.5f,0.0f,0.0f), m_aTargetHistoryPos[i] - CVector(0.5f,0.0f,0.0f), 0xffffffff, 0xffffffff);
		}
	}
#endif	
}




void CCam::ClipIfPedInFrontOfPlayer(void)
{
#if 0 //XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
	//need to do a a check if there is a ped that is standing in the near clip
	//plane in front of the camera.
	//N.B. If there is a cut scene playing then we don't need to do these checks 
	//As the cutscene animator has either thought of it or someone has moaned about it. 
	float CameraFrontDir=0.0f;
	int ClosestPedAliveNumber=0;
	bool FoundCloseAlivePed=false;		
	static bool PreviouslyFailedRoadHeightCheck=false;
	CVector PlayerToCamVect=(TheCamera.pTargetEntity->GetPosition()-(*TheCamera.GetGameCamPosition()));
	float PlayerToCamDist=CMaths::Sqrt(PlayerToCamVect.x * PlayerToCamVect.x + PlayerToCamVect.y * PlayerToCamVect.y);
	float CameraDirection=CGeneral::GetATanOfXY(PlayerToCamVect.x, PlayerToCamVect.y);


	CameraFrontDir=CGeneral::GetATanOfXY(TheCamera.GetMatrix().xy, TheCamera.GetMatrix().yy);
	CPed* PlayerPed=CWorld::Players[CWorld::PlayerInFocus].pPed;

	ASSERTMSG(PlayerPed!=NULL, "NICE ONE MARK YA DICK");
	///do the thing for the closest ped
	ASSERT((Mode==CCam::MODE_SNIPER)||(Mode==CCam::MODE_ROCKETLAUNCHER)||(Mode==CCam::MODE_M16_1STPERSON||(Mode==CCam::MODE_ROCKETLAUNCHER_HS))
	||(Mode==CCam::MODE_HELICANNON_1STPERSON)||(Mode==CCam::MODE_1STPERSON)||(Mode==CCam::MODE_CAMERA)||(PlayerPed->GetWeapon()->GetFirstPersonWeaponMode()));


	//Compute the closest living ped to the player
	CPed* pClosestPedAlive=0;
	if(PlayerPed->GetPedIntelligence()->GetClosestPedInRange())
	{	
		pClosestPedAlive=PlayerPed->GetPedIntelligence()->GetClosestPedInRange();
	}
	else
	{
		float fMin2=1000.0;
		int i;
		CEntity** ppNearbyPeds=PlayerPed->GetPedIntelligence()->GetNearbyPeds();
		const int N=PlayerPed->GetPedIntelligence()->GetMaxNumPedsInRange();
		for(i=0;i<N;i++)
		{
			CEntity* pNearbyEntity=ppNearbyPeds[i];
			if(pNearbyEntity)
			{
				ASSERT(pNearbyEntity->GetType()==ENTITY_TYPE_PED);
				CPed* pNearbyPed=(CPed*)pNearbyEntity;
				if(pNearbyPed->IsAlive())
				{
					CVector vDiff;
					vDiff.FromSubtract(PlayerPed->GetPosition(),pNearbyPed->GetPosition());
					const float f2=vDiff.MagnitudeSqr();
					if(f2<fMin2)
					{
						fMin2=f2;
						pClosestPedAlive=pNearbyPed;
					}
				}				
			}
		}
	}
	
	//Test if the closest living ped to the player is in the near clip plane.
	if (pClosestPedAlive)
	{
		CVector ClosestPedToPlayer=pClosestPedAlive->GetPosition()-(*TheCamera.GetGameCamPosition()); 
		float ClosestPedToPlayerAngle=CGeneral::GetATanOfXY(ClosestPedToPlayer.x, ClosestPedToPlayer.y);
		
		float AngleBetweenPlayer_ClosePed_Camera=0.0f; 	
		AngleBetweenPlayer_ClosePed_Camera=CameraFrontDir-ClosestPedToPlayerAngle;

		while (AngleBetweenPlayer_ClosePed_Camera>=(PI))
		{
			AngleBetweenPlayer_ClosePed_Camera-=2.0f * PI;
		}
		while (AngleBetweenPlayer_ClosePed_Camera<(-PI))
		{
			AngleBetweenPlayer_ClosePed_Camera+=2.0f * PI;
		}

		if  ((CMaths::Abs(AngleBetweenPlayer_ClosePed_Camera))<DEGTORAD(90))
		{
			//o.k. the player is at least in front of the camera
			float DistanceNearPedToCam=CMaths::Sqrt(ClosestPedToPlayer.x * ClosestPedToPlayer.x + ClosestPedToPlayer.y * ClosestPedToPlayer.y); 
			if (DistanceNearPedToCam<1.25f)
			{
				float NearClipVal=0.90f-(1.25f-DistanceNearPedToCam);
				if (NearClipVal<0.05f)
				{
					NearClipVal=0.05f;
				}
				RwCameraSetNearClipPlane(Scene.camera, NearClipVal);
			}
		}
	}	
	
	/*
	while ((ClosestPedAliveNumber<(PlayerPed->m_NumClosePeds)) && (FoundCloseAlivePed==false))
	{
		if ((PlayerPed->m_apClosePeds[ClosestPedAliveNumber]!=NULL)&&(PlayerPed->m_apClosePeds[ClosestPedAliveNumber]->GetPedState()!=PED_DEAD))
		{
			FoundCloseAlivePed=true;
		}
		else
		{
			ClosestPedAliveNumber++;
		}
	}
		
	if (FoundCloseAlivePed)
	{
		CVector ClosestPedToPlayer=(PlayerPed->m_apClosePeds[ClosestPedAliveNumber]->GetPosition())-(*TheCamera.GetGameCamPosition()); 
		float ClosestPedToPlayerAngle=CGeneral::GetATanOfXY(ClosestPedToPlayer.x, ClosestPedToPlayer.y);
		
		float AngleBetweenPlayer_ClosePed_Camera=0.0f; 	
		AngleBetweenPlayer_ClosePed_Camera=CameraFrontDir-ClosestPedToPlayerAngle;

		while (AngleBetweenPlayer_ClosePed_Camera>=(PI))
		{
			AngleBetweenPlayer_ClosePed_Camera-=2.0f * PI;
		}
		while (AngleBetweenPlayer_ClosePed_Camera<(-PI))
		{
			AngleBetweenPlayer_ClosePed_Camera+=2.0f * PI;
		}

		if  ((CMaths::Abs(AngleBetweenPlayer_ClosePed_Camera))<DEGTORAD(90))
		{
			//o.k. the player is at least in front of the camera
			float DistanceNearPedToCam=CMaths::Sqrt(ClosestPedToPlayer.x * ClosestPedToPlayer.x + ClosestPedToPlayer.y * ClosestPedToPlayer.y); 
			if (DistanceNearPedToCam<1.25f)
			{
				////
				//char Str[200];
				//sprintf(Str, "Distance near ped to cam %f ", DistanceNearPedToCam);
				//CDebug::PrintToScreenCoors(Str, 3, 11);
				///
				float NearClipVal=0.90f-(1.25f-DistanceNearPedToCam);
				if (NearClipVal<0.05f)
				{
					NearClipVal=0.05f;
				}
				RwCameraSetNearClipPlane(Scene.camera, NearClipVal);
			}
		}
	}	
	*/
#endif
}


///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Cam_On_A_String()
// PURPOSE    : 
// RETURNS    : nowt
// PARAMETERS :
//			  : Cheers to Les whose code I nicked this from
///////////////////////////////////////////////////////////////////////////
//
float FIRETRUCK_TRACKING_MULT = 0.1f;
//
void CCam::Process_Cam_On_A_String(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
#if 0 
	if (CamTargetEntity->GetIsTypeVehicle())//cheap nasty bug fix
	{
		float CarLength=0.0f;
		CVector TargetCoors;
		CVector ZoomVect;
		CVector CamToCarDist;
		CVector VehDimension;
		CVector SourceRoadCoors;
		CVector IdealSource;
		CVector BehindVect;
		////////////////////////////////////////////////////
		/////////Statics that need to be reset
		//  static float AlphaSpeed=0.0f;
		static float DeltaSpeed=0.0f;
		static float BufferedZTargetForEsparanto=0.0f;
		static float DistanceSpeed=0.0f;
		static bool  DistWasSmallCauseOfCar=false;
		
		int PosInVehCamParamArr=0;
		
		
		if (!(TheCamera.GetArrPosForVehicleType(((CVehicle*)CamTargetEntity)->GetVehicleAppearance(), PosInVehCamParamArr)))
		{
			ASSERTMSG(0, "unknown vehicle type");
		}
		
		FOV=70.0f;	

		if (ResetStatics)
		{
			
			AlphaSpeed=0.0f;
			DeltaSpeed=0.0f;
			DistanceSpeed=0.0f;
			DistWasSmallCauseOfCar=false;
			m_fTilt=0.0f;
			m_fTiltSpeed=0.0f;

			 ////////// ResetStatics=FALSE; //done in the camera height stuff	
		}

#ifdef GTA_SA
		float fCamWaterLevel = 0.0f;
		if( CWaterLevel::GetWaterLevel(Source.x, Source.y, Source.z, &fCamWaterLevel, true) )
		{
			if(Source.z < fCamWaterLevel - 0.3f)
			{
				float fWaterColourMag = CMaths::Sqrt(CTimeCycle::GetWaterRed()*CTimeCycle::GetWaterRed() + CTimeCycle::GetWaterGreen()*CTimeCycle::GetWaterGreen() + CTimeCycle::GetWaterBlue()*CTimeCycle::GetWaterBlue());
				if(fWaterColourMag > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT)
				{
					fWaterColourMag = BOAT_UNDERWATER_CAM_COLORMAG_LIMIT/fWaterColourMag;
					TheCamera.SetMotionBlur(fWaterColourMag*CTimeCycle::GetWaterRed(), fWaterColourMag*CTimeCycle::GetWaterGreen(), fWaterColourMag*CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
				}
				else
					TheCamera.SetMotionBlur(CTimeCycle::GetWaterRed(), CTimeCycle::GetWaterGreen(), CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
			}
		}
#endif		
		
		//Lets store stuff before we make any changes	
		ASSERTMSG(CamTargetEntity->GetIsTypeVehicle(), "DUBBA");

		VehDimension = CamTargetEntity->GetColModel().GetBoundBoxMax();
		VehDimension -= CamTargetEntity->GetColModel().GetBoundBoxMin();
		TargetCoors = ThisCamsTarget;
		
		CarLength = VehDimension.Magnitude();
		//CarLength = CMaths::Sqrt(VehDimension.x*VehDimension.x + VehDimension.y * VehDimension.y);

static float MONSTER_TRUCK_CAMDIST_MULT = 1.2f;
static float MONSTER_TRUCK_CAMHEIGHT_MULT = 0.65f;
		// bikes are pretty small so camera ends up too close
		if(((CVehicle *)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BIKE)
			CarLength *= 1.45f;
		else if(((CVehicle *)CamTargetEntity)->GetVehicleType()==VEHICLE_TYPE_MONSTERTRUCK)
		{
			CarLength *= MONSTER_TRUCK_CAMDIST_MULT;
			VehDimension.z *= MONSTER_TRUCK_CAMHEIGHT_MULT;
		}
		// use a different offset for full-size heli's (not r/c ones)
		if(((CVehicle *)CamTargetEntity)->GetVehicleAppearance()==APR_HELI && CamTargetEntity->GetStatus()!=STATUS_PLAYER_REMOTE)
			TargetCoors += CamTargetEntity->GetMatrix().GetUp()*fTestShiftHeliCamTarget*VehDimension.z;
		else
			TargetCoors.z+= VehDimension.z*0.80f;
		//TargetCoors.z+= VehDimension.z - 0.10f;
		
		Beta=CGeneral::GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y );//maintain beta anyway
		
		Alpha = CGeneral::LimitRadianAngle(Alpha);
		Beta = CGeneral::LimitRadianAngle(Beta);
		
		if(CamTargetEntity->GetModelIndex()==MODELID_CAR_FIRETRUCK && CPad::GetPad(0)->GetCarGunFired()
		&& ((CAutomobile *)CamTargetEntity)->m_vecMoveSpeed.Magnitude2D() < 0.01f)
		{
			// want to rotate the camera to look where the firetruck is squirting, not so easy though since Beta isn't really used!
			float fWaterJetHeading = CamTargetEntity->GetHeading();
			fWaterJetHeading -= ((CAutomobile *)CamTargetEntity)->GunOrientation;  // for some reason, GunOrientation rotation is flipped
			fWaterJetHeading += HALF_PI;
			fWaterJetHeading = CGeneral::LimitRadianAngle(fWaterJetHeading);
			
			fWaterJetHeading = fWaterJetHeading - Beta;
			if(fWaterJetHeading > PI)	fWaterJetHeading = -TWO_PI + fWaterJetHeading;
			else if(fWaterJetHeading < -PI)	fWaterJetHeading = TWO_PI + fWaterJetHeading;
			
			fWaterJetHeading = MAX(-0.8f, MIN(0.8f, fWaterJetHeading));
			fWaterJetHeading *=FIRETRUCK_TRACKING_MULT*(TargetCoors - Source).Magnitude();
			
			CVector vecTempRight = CrossProduct(Front, CVector(0.0f, 0.0f, 1.0f));
			Source += fWaterJetHeading * vecTempRight;
		}
		

		CVector VecDistance=Source- TargetCoors ;
		float DistMagnitude=CMaths::Sqrt(VecDistance.x*VecDistance.x + VecDistance.y * VecDistance.y);		 
		m_fDistanceBeforeChanges=DistMagnitude;
		Cam_On_A_String_Unobscured(TargetCoors, CarLength); //do the goo dold cam on a string algorythm
		//want to change the length of the distance if the camera is behind a wall
		///Lets fix if obscured want to reduce length
		WorkOutCamHeight (TargetCoors, TargetOrientation, VehDimension.z);
		RotCamIfInFrontCar(TargetCoors, TargetOrientation);

	//	FixCamIfObscured(TargetCoors, VehDimension.z, TargetOrientation); 
		//the building check was clear
		FixCamWhenObscuredByVehicle(TargetCoors);
		m_cvecTargetCoorsForFudgeInter=TargetCoors;
		CVector TempSource=Source;

		// if the space between the TargetCoords and the centre of the TargetEntity is not clear
		// do avoid geometry from the centre of the entity, otherwise might get on wrong side of collision
		if(!CWorld::GetIsLineOfSightClear(CamTargetEntity->GetPosition(), m_cvecTargetCoorsForFudgeInter, true, false, false, true, false, false, true))
			TheCamera.AvoidTheGeometry(TempSource, CamTargetEntity->GetPosition(), Source, FOV);
		else
			TheCamera.AvoidTheGeometry(TempSource, m_cvecTargetCoorsForFudgeInter, Source, FOV);
	
		Front=TargetCoors-Source;
		Front.Normalise();

		

		bool RollForCarOn=true;
		eVehicleAppearance VehicleApperance=((CVehicle*)(CamTargetEntity))->GetVehicleAppearance(); 
 		int PositionInArray=0;
		TheCamera.GetArrPosForVehicleType(VehicleApperance, PositionInArray);

		if (VehicleApperance==APR_HELI)
		{
			//we want to affect the up vector depending on how the helicopter is tilting
			//with respect to the camera, i.e. if the camera is directly behind it we want
			//it to tilt maximally  
			float TargetTilt=DotProduct(Front, ((CPhysical*)CamTargetEntity)->GetSpeed(0,0,0));
			CVector HeliUpVec=CamTargetEntity->GetMatrix().GetUp();
			HeliUpVec.Normalise();
			//buffer a bit so it feels like a real camera follwing in a heli
			int Sign=1;
	
			if (TargetTilt<0.0f)
			{
				Sign=-1;
			}
			if (m_fTilt!=0.0f)
			{
				TargetTilt+=TiltOverShoot[PositionInArray] * (TargetTilt/m_fTilt) * Sign;
			}
			WellBufferMe(TargetTilt, &(m_fTilt), &(m_fTiltSpeed), TiltTopSpeed[PositionInArray], TiltSpeedStep[PositionInArray], false);			
			
			CVector TiltVec=(CVector(0.0f, 0.0f, 1.0f)- HeliUpVec) * (m_fTilt);
			ASSERTMSG(TiltVec!=(CVector(0.0f, 0.0f, 1.0f)), "Tilt would make game crash");
			Up=CVector(0.0f, 0.0f, 1.0f) - TiltVec;
	
			Up.Normalise();
			Front.Normalise();
			CVector TempRight = CrossProduct( Up, Front );
			Up=CrossProduct( Front, TempRight );
			Up.Normalise();
		}
		else if (RollForCarOn)
		{
			//right lets do the roll
			float TargetRoll=0.0f;
			float CarToCamAngle=0.0f;
			float CloseTo90=0.0f;
			CVector CarFront;
			float TheMaxSpeed=210.0f;
			if (!(CPad::GetPad(0)->GetDPadLeft()||CPad::GetPad(0)->GetDPadRight()))
			//they are using the joypad
			{
				float CarSpeed = (DotProduct(((CAutomobile*)CamTargetEntity)->m_vecMoveSpeed, ((CAutomobile*)CamTargetEntity)->GetMatrix().GetForward()))/GAME_VELOCITY_CONST;

				if (CarSpeed>TheMaxSpeed)
				{
					CarSpeed=TheMaxSpeed;
				}
				
				TargetRoll=(CarSpeed*(CPad::GetPad(0)->GetLeftStickX()/128.0f))/TheMaxSpeed;
				//work out how much of an angle camera is at
				CarFront=((CAutomobile*)CamTargetEntity)->GetMatrix().GetForward();
				CarFront.Normalise();
				CarToCamAngle=DotProduct(CarFront, Front); //both vectors length equal to one 
				//therefore car to car angle should cos of the angle
				CarToCamAngle = MIN(1.0f, CMaths::Abs(CarToCamAngle));

//				1.0 is full behind or infront of the car
//				0 is half way to the car
				ASSERT(CarToCamAngle>=0.0f && CarToCamAngle<=1.0f);
				float CarToCamRoll=CMaths::Sin(CMaths::ACos(CarToCamAngle));
				ASSERT(CarToCamRoll>=0.0f && CarToCamRoll<=1.0f);
			
				TargetRoll*=CarToCamRoll;

///////////////////				
				ASSERTMSG(TargetRoll<=1, "Get Mark TargetRoll>1");
				TargetRoll*=(f_max_role_angle + TiltOverShoot[PositionInArray]*DEGTORAD(10.0f));
			}
			
			else
			{
				float CarSpeed = (DotProduct(((CAutomobile*)CamTargetEntity)->m_vecMoveSpeed, ((CAutomobile*)CamTargetEntity)->GetMatrix().GetForward()))/GAME_VELOCITY_CONST;

				
				if (CarSpeed>TheMaxSpeed)
				{
					CarSpeed=TheMaxSpeed;
				}
				
				
				if (CPad::GetPad(0)->GetDPadLeft())
				{
					TargetRoll=(f_max_role_angle + TiltOverShoot[PositionInArray]*DEGTORAD(10.0f));
				}
				else
				{
					TargetRoll=-(f_max_role_angle + TiltOverShoot[PositionInArray]*DEGTORAD(10.0f));
				}
				
				TargetRoll=(CarSpeed*TargetRoll)/TheMaxSpeed;
				
				CarFront=((CAutomobile*)CamTargetEntity)->GetMatrix().GetForward();
				CarFront.Normalise();
				CarToCamAngle=DotProduct(CarFront, Front); //both vectors length equal to one 
				//therefore car to car angle should cos of the angle
				CarToCamAngle = MIN(1.0f, CMaths::Abs(CarToCamAngle));


//				1.0 is full behind or infront of the car
//				0 is half way to the car
				ASSERT(CarToCamAngle>=0.0f && CarToCamAngle<=1.0f);
				float CarToCamRoll=CMaths::Sin(CMaths::ACos(CarToCamAngle));
				ASSERT(CarToCamRoll>=0.0f && CarToCamRoll<=1.0f);				
				
				TargetRoll*=CarToCamRoll;
			}
			
//			sprintf (Str, "Target Roll %f", TargetRoll) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
//    		CDebug::PrintToScreenCoors(Str, 3, 11);
			
			WellBufferMe(TargetRoll, &f_Roll, &f_rollSpeed, 0.15f, 0.07f);
			float TempRollValue=f_Roll+DEGTORAD(90); //this is because the roll value is 0 for directly up
//			sprintf (Str, "Temp Roll Roll %f", TempRollValue) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
//    		CDebug::PrintToScreenCoors(Str, 3, 12);


			Up=CVector(CMaths::Cos(TempRollValue), 0.0f, CMaths::Sin(TempRollValue));
			Up.Normalise();
			Front.Normalise();
			CVector TempRight = CrossProduct( Up, Front );
			TempRight.Normalise();
			Up=CrossProduct( Front, TempRight );
			Up.Normalise();
		}	
		else
		{		
			GetVectorsReadyForRW();
		}
		ResetStatics=FALSE;	
	}	
#endif	
}



///////////////////////
void CCam::FixCamWhenObscuredByVehicle(const CVector &TargetCoors)
{
#if 0 // DW - removed
	CColPoint colPoint;
	CEntity* pHitEntity=NULL;
	static float HeightFixerCarsObscuring=0.0f;
	static float HeightFixerCarsObscuringSpeed=0.0f;
	float HeightToGoUpTo=0.0f;
	float FinalTargetZ=0.0f;
			
	if (CWorld::ProcessLineOfSight(TargetCoors, Source, colPoint, pHitEntity, false, true, false, false, false, false, false))
	{
		HeightToGoUpTo = pHitEntity->GetColModel().GetBoundBoxMax().z;
		HeightToGoUpTo+=1.0f;
		FinalTargetZ=(HeightToGoUpTo+TargetCoors.z)-Source.z;
		if (FinalTargetZ<0.0f)
		{
			FinalTargetZ=0.0f;
		}
	}
	/*
#ifdef GTA_SA
	else if(CamTargetEntity->GetModelIndex()==MODELID_CAR_SENTINEL && ((CAutomobile *)CamTargetEntity)->m_fTransformPosition > 0.0f)
	{
		float fCamWaterLevel = 0.0f;
		float fCarWaterLevel = 0.0f;

		if(CWaterLevel::GetWaterLevelNoWaves(Source.x, Source.y, Source.z, &fCamWaterLevel)
		&& CWaterLevel::GetWaterLevel(TargetCoors.x, TargetCoors.y, TargetCoors.z, &fCarWaterLevel, true))
		{
			// gonna try line check from water level down to -2.0m to see if there's room for the camera
			CColPoint tempCol;
			CEntity *pTempHitEnt = NULL;
			CVector vecTestDefaultPos = Source;
			vecTestDefaultPos.z = fCamWaterLevel + 5.0f;

			if(Source.z > fCamWaterLevel - 2.5f && TargetCoors.z < fCarWaterLevel + 1.2f
			&& !CWorld::ProcessVerticalLine(vecTestDefaultPos, fCamWaterLevel - 2.0f, tempCol, pTempHitEnt, true, false, false, false, false, false, NULL))
			{
				FinalTargetZ = fCamWaterLevel - 2.5f - Source.z;
			}
		}
	}
#endif
	*/
	
	WellBufferMe(FinalTargetZ, &(HeightFixerCarsObscuring), &(HeightFixerCarsObscuringSpeed),  0.20f, 0.025f);
	Source.z+=HeightFixerCarsObscuring;
#endif	
}



///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Syphon()
// PURPOSE    : Processes camera mode inspired by syphon filters.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
//float fReAimSyphonDistBase = 0.5f;
//float fReAimSyphonHeightMult = 0.5f;
//float fTweakNewBetaOffset = 0.9f;
//float fTweakAvoidGeometryPos = 1.5f;
//
void CCam::Process_Syphon(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired)
{
#if 0 // DW - removed
	FOV=70.0f;
	if(!CamTargetEntity->GetIsTypePed())
		return;	// shouldn't be here?
	
	float  Length, DistanceOnGround,  CurrentDistOnGround;
	CVector	Dist, TargetCoors, ZoomVect;
	CVector IdealSource;
	CPlayerPed* ThePlayer = CWorld::Players[CWorld::PlayerInFocus].pPed;
	CColPoint colPoint;
	static bool CameraObscured=false;
	static bool FailedClippingTestPrevously=false;
	static float BetaOffset=INIT_SYPHON_DEGREE_OFFSET;//was 32
	static float AngleToGoTo=0.0f;   //extra offset to deal with walls etc
	static float AngleToGoToSpeed=0.0f; //as above
	float TargetAngleToGoTo=0.0f;
	float DistBetweenPedAndPlayer=0.0f;
	const float DistBetweenPedAndPlayerBeforeBufferOn=2.0f;
	bool DistBetweenPedAndPlayerOn=false;
	static bool DistBetweenPedAndPlayerPreviouslyOn=false;
	static float HeightDown=INIT_SYPHON_Z_OFFSET;
	static float AlphaOffset=INIT_SYPHON_ALPHA_OFFSET;
	static bool MakeBetaOffsetNegative=true;//false;
	bool OnMovingVehicle=false;
	
	//char	Str[128];
	TargetCoors = ThisCamsTarget;
	AlphaOffset=INIT_SYPHON_ALPHA_OFFSET;

	DistanceOnGround=INIT_SYPHON_GROUND_DIST;
	CurrentDistOnGround=DistanceOnGround;
	while (Beta>=(PI)) {Beta-=2.0f * PI;}
	while (Beta<(-PI)) {Beta+=2.0f * PI;}
	float ReqSpeed;
	float IdealBeta;	
	
	Dist=TheCamera.m_cvecAimingTargetCoors - TargetCoors;
	
	DistBetweenPedAndPlayer=CMaths::Sqrt(Dist.x * Dist.x + Dist.y * Dist.y);
	IdealBeta = CGeneral::GetATanOfXY(Dist.x, Dist.y);	
	IdealBeta+=PI;
	
			
//	while (IdealBeta>=2.0f * PI) {IdealBeta-=2.0f * PI;}
//	while (IdealBeta<0.0f) {IdealBeta+=2.0f * PI;}
	if (ResetStatics)
	{
		BetaOffset=INIT_SYPHON_DEGREE_OFFSET;
		Beta=CGeneral::GetATanOfXY((Source.x - TargetCoors.x),( Source.y - TargetCoors.y));	
		CameraObscured=false; 	
//#define GO_TO_NEAREST_SIDE
#ifdef GO_TO_NEAREST_SIDE
		MakeBetaOffsetNegative=false;
		float LeftAngle=IdealBeta-BetaOffset;
		float RightAngle=IdealBeta+BetaOffset;
		float LeftDiff=LeftAngle-Beta;
		float RightDiff=RightAngle-Beta;
		MakeAngleLessThan180(LeftDiff);
		MakeAngleLessThan180(RightDiff);
		if (ABS(LeftDiff)<ABS(RightDiff))
		{
			MakeBetaOffsetNegative=true;
		}
#endif
		AngleToGoTo=0.0f;
		AngleToGoToSpeed=0.0f;
		FailedClippingTestPrevously=false;
		DistBetweenPedAndPlayerPreviouslyOn=false;
		ResetStatics=FALSE;
	
	}
	
	if (MakeBetaOffsetNegative)
	{
		BetaOffset=-INIT_SYPHON_DEGREE_OFFSET;
	}
	
	Beta =IdealBeta + BetaOffset;	
	
		
	Source=TargetCoors;
	Source.x+=(CMaths::Cos(Beta) * DistanceOnGround);
	Source.y+=(CMaths::Sin(Beta) * DistanceOnGround);
	
	CVector SpeedOfEntityStoodOn(0.0f,0.0f,0.0f);
	//if we are on a boat we need to  remove the speed of the boat from the Target Coors so that only the ped movement
	//is taken into account 		

	if (((CPed*)CamTargetEntity)->m_pEntityStandingOn)
	{
		if ((((CPed*)CamTargetEntity)->m_pEntityStandingOn->GetIsTypeVehicle())||(((CPed*)CamTargetEntity)->m_pEntityStandingOn->GetIsTypeObject()))
		{
//					SpeedOfEntityStoodOn=((CPhysical*)(((CPed*)CamTargetEntity)->m_pEntityStandingOn))->GetSpeed(SpeedOfEntityStoodOn) ;
//					SpeedOfEntityStoodOn*=CTimer::GetTimeStep();
			OnMovingVehicle=true;
		}
	}

//			Source.x+= SpeedOfEntityStoodOn.x;
//			Source.y+= SpeedOfEntityStoodOn.y;



	TargetCoors.z+=m_fSyphonModeTargetZOffSet;//0.5f
    

	Dist = Source - TargetCoors;
	Length = CMaths::Sqrt(Dist.x * Dist.x + Dist.y * Dist.y);

////////////////////////////////
// this is the bit the pulls the camera out when player gets too close to target
	bool bPullCameraOut = false;
	CVector PedToPlayer;
	PedToPlayer=TheCamera.m_cvecAimingTargetCoors-TargetCoors;
	float GroundDistance=CMaths::Sqrt(PedToPlayer.x * PedToPlayer.x + PedToPlayer.y * PedToPlayer.y); 
	float SmallestGroundDistForHeight=6.500f;
	if (GroundDistance<SmallestGroundDistForHeight)
	{
		GroundDistance=SmallestGroundDistForHeight;
		bPullCameraOut = true;
	}

	float PedToPlayerAngle = CGeneral::GetATanOfXY((GroundDistance),( TheCamera.m_cvecAimingTargetCoors.z - TargetCoors.z));	
	if (ResetStatics)
	{
		Alpha=-PedToPlayerAngle;
	}
	float DeltaAlpha=0.0f;

	while  (PedToPlayerAngle>PI){PedToPlayerAngle-=  2.0f * PI;}
	while  (PedToPlayerAngle<-PI){PedToPlayerAngle+= 2.0f * PI;}
	while  (Alpha>PI){Alpha-=  2.0f * PI;}
	while  (Alpha<-PI){Alpha+= 2.0f * PI;}
			
	DeltaAlpha= -PedToPlayerAngle - Alpha;				
		
	while  (DeltaAlpha>PI){DeltaAlpha=DeltaAlpha - 2.0f * PI;}
	while  (DeltaAlpha<-PI){DeltaAlpha=DeltaAlpha + 2.0f * PI;}
	
	float TopSpeed=0.07f;
	float SpeedStep=0.015f;
	
	if (OnMovingVehicle)
	{
		TopSpeed/=2;
		SpeedStep/=2;
	}
			
	ReqSpeed = DeltaAlpha  * TopSpeed;	// want to get there in 0.25s
	if (ReqSpeed - AlphaSpeed > 0.0f)///This can be in a positive or negative direction
							/// we are going slower than we want to.INCREASE THE SPEED!!!!!
	{
		AlphaSpeed += SpeedStep * CMaths::Abs(ReqSpeed - AlphaSpeed) * CTimer::GetTimeStep();//// increase the speed
	}
	else ///we are going faster than we want. Slow down. 
	{
		AlphaSpeed -= SpeedStep * CMaths::Abs(ReqSpeed - AlphaSpeed) * CTimer::GetTimeStep();//decrease the speed 
	}	

	if ((ReqSpeed < 0.0f) && (AlphaSpeed < ReqSpeed)) //checks incase Beta speed in now too low
	{
		AlphaSpeed = ReqSpeed; ///sets it to the limit
	}
	else if ((ReqSpeed > 0.0f) && (AlphaSpeed > ReqSpeed)) //checks incase Beta speed in now too high
	{
		AlphaSpeed = ReqSpeed;
	}

	Alpha += AlphaSpeed * MIN(10.0f, CTimer::GetTimeStep() ); // beta is now where we want to be

	Source.z+=(CMaths::Sin(Alpha + AlphaOffset) * CurrentDistOnGround) + (CurrentDistOnGround/5);

	if (Source.z<(TargetCoors.z+ HeightDown))
	{
		Source.z=TargetCoors.z + HeightDown;
	}


/////////////////////////////////////////////////////
// ok having a bit of trouble following what's going on here
// so gonna use the alpha values from above (tilting up/down)
// and test the LOS to the weapontarget using the calculated source position
// then use any obstacles to modify the BetaOffset and DistanceOnGround to move the source around
/////////////////////////////////////////////////////
// (hopefully)
	
	if(!bPullCameraOut)
	{
		CColPoint colPoint;
		CEntity *pHitEntity = NULL;
	
		// MAKE SURE CWorld::pIgnoreEntity IS CLEARED AGAIN ASAP
		CWorld::pIgnoreEntity = (CEntity *) CamTargetEntity;//so we don't find the player
		// if we're not going to use the results from peds/cars why bother checking them?
		if(CWorld::ProcessLineOfSight(TheCamera.m_cvecAimingTargetCoors, Source, colPoint, pHitEntity, true, false, false, true, false,  false, true))
		{
			CVector vecTempFront = TheCamera.m_cvecAimingTargetCoors - Source;
			vecTempFront.Normalise();
			
			// if this check only found a collision ahead of the player peds position
			if( DotProduct(vecTempFront, colPoint.GetPosition() - Source) > DotProduct(vecTempFront, ThisCamsTarget - Source))
			{
				// want to a sphere test from on the line between the player and their target
				// next to the colpoint we just found, to look for collision closer to the player/target vector
				float fTargetToColDist = (TheCamera.m_cvecAimingTargetCoors - colPoint.GetPosition()).Magnitude();
				CVector vecTempPedToTarget = TheCamera.m_cvecAimingTargetCoors - ThisCamsTarget;
				float fTempPedToTargetMag = vecTempPedToTarget.Magnitude();
				if(fTempPedToTargetMag > 0.01f)
					vecTempPedToTarget /= fTempPedToTargetMag;
				else
					vecTempPedToTarget.x = 1.0f;
				
				CVector vecTestPos = TheCamera.m_cvecAimingTargetCoors - fTargetToColDist*vecTempPedToTarget;
				float fDistToCol = (colPoint.GetPosition() - vecTestPos).Magnitude();
				
				CEntity *pFindEntity = CWorld::TestSphereAgainstWorld(vecTestPos, fDistToCol, NULL, true, false, false, true, false, true);
				if (pFindEntity)
				{
					bool bUseNewCol = true;
					bool bCarefulColOnLHS = false;
					// we are interested in the perpendicular distance between the new collision point
					// and the player target vector, so first get rid of component along this vector
					CVector vecToNewCol = gaTempSphereColPoints[0].GetPosition() - vecTestPos;
					float fGetRidForwardComponent = DotProduct(vecToNewCol, vecTempPedToTarget);
					vecToNewCol -= fGetRidForwardComponent*vecTempPedToTarget;
					// do the same for the old colpoint incase the sphere check messed up
					//and found a point further away from the player/target vector
					CVector vecToOldCol = colPoint.GetPosition() - vecTestPos;
					fGetRidForwardComponent = DotProduct(vecToOldCol, vecTempPedToTarget);
					vecToOldCol -= fGetRidForwardComponent*vecTempPedToTarget;
				
					if( DotProduct(vecToOldCol, vecToNewCol) < 0.0f )
					{
						bUseNewCol = false;	// damn the new col point is on the wrong side!
						bCarefulColOnLHS = true;
					}
					else if( vecToOldCol.MagnitudeSqr() < vecToNewCol.MagnitudeSqr() )
					{
						bUseNewCol = false;	// damn the new col point further away from target vector
					}

					// scale up offset to length beside playerped
					float fNewSideMag;
					if( bUseNewCol )
						fNewSideMag = vecToNewCol.Magnitude();
					else
						fNewSideMag = vecToNewCol.Magnitude();

					float fNewBetaOffset = 0.0f;
					if(fNewSideMag > 0.0f && fTargetToColDist > 0.1f)
					{
						fNewSideMag = fNewSideMag * fTempPedToTargetMag / fTargetToColDist;
						fNewBetaOffset = fTweakNewBetaOffset*CMaths::ASin(MIN(1.0f, fNewSideMag/DistanceOnGround));
					}
					
					if(fNewBetaOffset < BetaOffset)
					{
						float fBetaRatio = fNewBetaOffset/BetaOffset;
					
#ifndef FINAL
if(CPlayerPed::bDebugPlayerInfo)
{
	sprintf(gString, "OldNewBeta: = %g / %g\n", BetaOffset, fNewBetaOffset);
	CDebug::PrintToScreenCoors(gString, 35,3);
}
#endif
						BetaOffset = fNewBetaOffset;
						Beta = IdealBeta + BetaOffset;
						// move camera in a bit as well as rotation?
						DistanceOnGround *= MAX(fReAimSyphonDistBase, fBetaRatio);
						Source.x = TargetCoors.x + (CMaths::Cos(Beta) * DistanceOnGround);
						Source.y = TargetCoors.y + (CMaths::Sin(Beta) * DistanceOnGround);
						// move the camera up a bit more?
//						Source.z = TargetCoors.z + (1.0f + fReAimSyphonHeightMult*(1.0f - fBetaRatio))*(Source.z - TargetCoors.z);
						Source.z += fReAimSyphonHeightMult*(1.0f - fBetaRatio);
					}
				}
				else
				{
#ifndef FINAL
					printf("bugger\t");
#endif
				}				
			}
			// otherwise just rely on AvoidTheGeometry to move the camera forward out of collision
		}
		CWorld::pIgnoreEntity=NULL;
	}

////////////////////////////////////////////////////
	Front = TheCamera.m_cvecAimingTargetCoors - Source;
	float FrontLength = Front.Magnitude2D();
	CurrentDistOnGround = DistanceOnGround;
	
	Front.Normalise();
////////////////////////////////////////////////////
/*
	CVector TestTarget=TargetCoors;
	Front = TargetCoors - Source;
	float FrontLength=Front.Magnitude2D();
	TestTarget.z=Source.z;
	CurrentDistOnGround=DistanceOnGround;
	
	Front.Normalise();
	float FrontAngle=CGeneral::GetATanOfXY(Front.x, Front.y);
	if (MakeBetaOffsetNegative)
	{			
		FrontAngle+=FrontOffsetSyphon;
	}
	else
	{
		FrontAngle-=FrontOffsetSyphon;
	}
	
	Front.x=CMaths::Cos(FrontAngle);
	Front.y=CMaths::Sin(FrontAngle);			
	Front.Normalise();
	TestTarget=TargetCoors;		
*/	
	
	m_cvecTargetCoorsForFudgeInter=Source + FrontLength*Front;
	m_cvecTargetCoorsForFudgeInter.z=TargetCoors.z;
	
	float fPreserveZPos = Source.z;
	CVector TempSource=Source;
//	CVector TargetAlongFrontVec = Source + Front*fTweakAvoidGeometryPos*DistanceOnGround*CMaths::Cos(BetaOffset);
	TheCamera.AvoidTheGeometry(TempSource, ThisCamsTarget + CVector(0.0f,0.0f,0.75f), Source, FOV);

	Source.z = fPreserveZPos;
	GetVectorsReadyForRW();

/*	// old version
	CVector TempSource=Source;
	TheCamera.AvoidTheGeometry(TempSource,ThisCamsTarget, Source, FOV);
	GetVectorsReadyForRW();
*/
#endif
}


/*
//
//
float MAX_HEIGHT_UP=15.0f; 
float WATER_Z_ADDITION=2.75f;
float WATER_Z_ADDITION_MIN = 1.5f;
float SMALLBOAT_CLOSE_ALPHA_MINUS = 0.2f;
//
float afBoatBetaDiffMult[3] = {0.15f, 0.07f, 0.01f};
float afBoatBetaSpeedDiffMult[3] = {0.020f, 0.015f, 0.005f};
//
*/
void CCam::Process_BehindBoat(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	#if 0
	if (CamTargetEntity->GetIsTypeVehicle())//cheap nasty bug fix
	{
		CVector VehDimension, BehindVect, SourceBefore;
		CVector TestVector, TestSource; 
		CVector TargetCoors=ThisCamsTarget;
		float  DeltaBeta=0.0f, ReqSpeed=0.0f;	
		float SafeCentre=0.0f, SafeLeft=0.0f, SafeRight=0.0f;
		static float TargetWhenChecksWereOn=0.0f;	
		static float CenterObscuredWhenChecksWereOn=0.0f;
		
		float fMaxHeightUp=MAX_HEIGHT_UP;
		float fWaterZAdditionMin = WATER_Z_ADDITION_MIN;
		float WaterLevel=0.0f;
		static float BufferedWaterLevel=0.0f;
		static float WaterLevelSpeed=0.0f;
		
		float fBoatBetaDiffMult = 0.0f;
		float fBoatBetaSpeedDiffMult = 0.0f;
		
		Beta=CGeneral::GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y );
		FOV=70.0f;
		float TargetAlpha=0.0f;

		if(ResetStatics)
		{
			TargetWhenChecksWereOn=0.0f;	
		    CenterObscuredWhenChecksWereOn=0.0f;
			Beta=TargetOrientation;
						
		}
		else if(DirectionWasLooking!=LOOKING_FORWARD)
			Beta=TargetOrientation;



	    if (!(CWaterLevel::GetWaterLevelNoWaves(TargetCoors.x, TargetCoors.y, TargetCoors.z, &WaterLevel)))//is not in water
	    {
			WaterLevel=TargetCoors.z-0.5f;
		}
		if (ResetStatics)
		{
			BufferedWaterLevel=WaterLevel;
			WaterLevelSpeed=0.0f;	
		}
		WellBufferMe(WaterLevel, &BufferedWaterLevel,  &WaterLevelSpeed, 0.20f, 0.07f, false);
		static float FixerForGoingBelowGround=0.4f;
		if ((TargetCoors.z-BufferedWaterLevel + WATER_Z_ADDITION)>-FixerForGoingBelowGround)
		{
			BufferedWaterLevel+=(TargetCoors.z-BufferedWaterLevel + WATER_Z_ADDITION)-FixerForGoingBelowGround;
		}

		//work out the length of the vehicle and add it onto vehicle zoom value smooth
		VehDimension = CamTargetEntity->GetColModel().GetBoundBoxMax();
		VehDimension -= CamTargetEntity->GetColModel().GetBoundBoxMin();
		
		float BoatLength=CMaths::Sqrt(VehDimension.x*VehDimension.x + VehDimension.y * VehDimension.y);
		
		eVehicleAppearance VehicleApperance=((CVehicle*)(CamTargetEntity))->GetVehicleAppearance(); 
		int PositionInArray=0;
		TheCamera.GetArrPosForVehicleType(VehicleApperance, PositionInArray);
	
		if (TheCamera.m_nCarZoom==ZOOM_ONE)
		{
			TargetAlpha=ZmOneAlphaOffset[PositionInArray];
			fBoatBetaDiffMult = afBoatBetaDiffMult[0];
			fBoatBetaSpeedDiffMult = afBoatBetaSpeedDiffMult[0];
		}
		else if (TheCamera.m_nCarZoom==ZOOM_TWO)
		{
			TargetAlpha=ZmTwoAlphaOffset[PositionInArray];
			fBoatBetaDiffMult = afBoatBetaDiffMult[1];
			fBoatBetaSpeedDiffMult = afBoatBetaSpeedDiffMult[1];
		}
		else if (TheCamera.m_nCarZoom==ZOOM_THREE) 
		{
			TargetAlpha=ZmThreeAlphaOffset[PositionInArray];
			fBoatBetaDiffMult = afBoatBetaDiffMult[2];
			fBoatBetaSpeedDiffMult = afBoatBetaSpeedDiffMult[2];
		}
		
		// force the camera down to just above the water under bridges.
		if(TheCamera.m_nCarZoom==ZOOM_ONE && BoatLength < 10.0f)
		{
			TargetAlpha -= SMALLBOAT_CLOSE_ALPHA_MINUS;
			BoatLength = 10.0f;
		}
		else if(CCullZones::Cam1stPersonForPlayer())
		{
			float fTempWaterLevel = 0.0f;
			CWaterLevel::GetWaterLevelNoWaves(TargetCoors.x, TargetCoors.y, TargetCoors.z, &fTempWaterLevel);
			fTempWaterLevel = (WaterLevel+WATER_Z_ADDITION_MIN-BufferedWaterLevel-WATER_Z_ADDITION)/(fMaxHeightUp + (VehDimension.z/2.0f));
			TargetAlpha = CMaths::ASin(MAX(-1.0f, MIN(1.0f, fTempWaterLevel)));
		}
		
		if (ResetStatics)
		{
			Alpha=TargetAlpha;	
			AlphaSpeed=0.0f;
		}

		WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.15f, 0.07f, true);
 
		if(ResetStatics)
		{
			Beta=TargetOrientation;
			DeltaBeta = 0.0f;
		}
		else
		{
			DeltaBeta = TargetOrientation - Beta;
			while (DeltaBeta > PI) DeltaBeta -= (2.0f*PI);
			while (DeltaBeta < -PI) DeltaBeta += (2.0f*PI);
		}

		ReqSpeed = DeltaBeta * fBoatBetaDiffMult;	// want to get there in 0.25s
		ReqSpeed *= ((CPhysical *)CamTargetEntity)->m_vecMoveSpeed.Magnitude();

		if (ReqSpeed - BetaSpeed > 0.0f)///This can be in a positive or negatve direction
										/// we are going slower than we want to.INCREASE THE SPEED!!!!!
		{
			BetaSpeed += fBoatBetaSpeedDiffMult * CMaths::Abs(ReqSpeed - BetaSpeed) * CTimer::GetTimeStep();//// increase the speed
		}
		else ///we are going faster than we want. Slow down or we're all going to die. Aiiiieeee. 
		{
			BetaSpeed -= fBoatBetaSpeedDiffMult * CMaths::Abs(ReqSpeed - BetaSpeed) * CTimer::GetTimeStep();//decrease the speed 
		}


		if ((ReqSpeed < 0.0f) && (BetaSpeed < ReqSpeed)) //checks incase Beta speed in now too low
		{
			BetaSpeed = ReqSpeed; ///sets it to the limit
		}
		else if ((ReqSpeed > 0.0f) && (BetaSpeed > ReqSpeed)) //checks incase Beta speed in now too high
		{
			BetaSpeed = ReqSpeed;
		}
		
		
		
		Beta += BetaSpeed * MIN(10.0f, CTimer::GetTimeStep() ); // beta is now where we want to be

		BehindVect = CVector( -CMaths::Cos(Beta), -CMaths::Sin(Beta), 0);
		Source = TargetCoors + (TheCamera.m_fCarZoomSmoothed + BoatLength) * BehindVect;
		Source.z = BufferedWaterLevel + WATER_Z_ADDITION + (fMaxHeightUp + (VehDimension.z/2.0f))*CMaths::Sin(Alpha);
		
		CVector Test=Source-TargetCoors;
		float VertDist=Test.z;
		float HorzDist=Test.Magnitude2D();
		
//		TargetCoors.z+=VehDimension.z +  1.5f;
		
		m_cvecTargetCoorsForFudgeInter=TargetCoors;
		CVector TempSource=Source;
		TheCamera.AvoidTheGeometry(TempSource,m_cvecTargetCoorsForFudgeInter, Source, FOV);
		
		Front = TargetCoors - Source;
		Front.Normalise();
		bool RollForBoatOn=true;
		if (RollForBoatOn)
		{
			//right lets do the roll
			float TargetRoll=0.0f;
			float BoatToCamAngle=0.0f;
			float CloseTo90=0.0f;
			CVector BoatFront;
			if (!(CPad::GetPad(0)->GetDPadLeft()||CPad::GetPad(0)->GetDPadRight()))
			//they are using the joypad
			{
				float BoatSpeed = (DotProduct(((CAutomobile*)CamTargetEntity)->m_vecMoveSpeed, ((CAutomobile*)CamTargetEntity)->GetMatrix().GetForward()))/GAME_VELOCITY_CONST;

				const float TheMaxSpeed=210.0f;
				if (BoatSpeed>TheMaxSpeed)
				{
					BoatSpeed=TheMaxSpeed;
				}
				TargetRoll=(BoatSpeed*(CPad::GetPad(0)->GetLeftStickX()/128.0f))/TheMaxSpeed;
				//work out how much of an angle camera is at
				BoatFront=((CPlaceable*)CamTargetEntity)->GetMatrix().GetForward();
				BoatFront.Normalise();
				BoatToCamAngle=DotProduct(BoatFront, Front); //both vectors length equal to one 
				//therefore car to car angle should cos of the angle
				BoatToCamAngle = MIN(1.0f, CMaths::Abs(BoatToCamAngle));

//				1.0 is full behind or infront of the car
//				0 is half way to the car
				ASSERT(BoatToCamAngle>=0.0f && BoatToCamAngle<=1.0f);
				float BoatToCamRoll=CMaths::Sin(CMaths::ACos(BoatToCamAngle));
				ASSERT(BoatToCamRoll>=0.0f && BoatToCamRoll<=1.0f);
			
				TargetRoll*=BoatToCamRoll;

///////////////////				
				ASSERTMSG(TargetRoll<=1, "Get Mark TargetRoll>1");
				TargetRoll*=(f_max_role_angle + TiltOverShoot[PositionInArray]*DEGTORAD(10.0f));
			}
			
			else
			{
				if (CPad::GetPad(0)->GetDPadLeft())
				{
					TargetRoll=(f_max_role_angle + TiltOverShoot[PositionInArray]*DEGTORAD(10.0f));
				}
				else
				{
					TargetRoll=-(f_max_role_angle + TiltOverShoot[PositionInArray]*DEGTORAD(10.0f));
				}

				BoatFront=((CPlaceable*)CamTargetEntity)->GetMatrix().GetForward();
				BoatFront.Normalise();
				BoatToCamAngle=DotProduct(BoatFront, Front); //both vectors length equal to one 
				//therefore car to car angle should cos of the angle
				BoatToCamAngle = MIN(1.0f, CMaths::Abs(BoatToCamAngle));


//				1.0 is full behind or infront of the car
//				0 is half way to the car
				ASSERT(BoatToCamAngle>=0.0f && BoatToCamAngle<=1.0f);
				float BoatToCamRoll=CMaths::Sin(CMaths::ACos(BoatToCamAngle));
				ASSERT(BoatToCamRoll>=0.0f && BoatToCamRoll<=1.0f);				
				
				TargetRoll*=BoatToCamRoll;
			}
			
			
			WellBufferMe(TargetRoll, &f_Roll, &f_rollSpeed, 0.15f, 0.07f);
			float TempRollValue=f_Roll+DEGTORAD(90); //this is because the roll value is 0 for directly up


			Up=CVector(CMaths::Cos(TempRollValue), 0.0f, CMaths::Sin(TempRollValue));
			Up.Normalise();
			Front.Normalise();
			CVector TempRight = CrossProduct( Up, Front );
			Up=CrossProduct( Front, TempRight );
			Up.Normalise();
		}	
		else
		{		
			GetVectorsReadyForRW();
		}
	}
  	ResetStatics=FALSE;	
	#endif  	
}

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Sniper()
// PURPOSE    : Sniper mode. Player controls the camera with the stick.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
//
void CCam::Process_Sniper(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
#if 0
	if (CamTargetEntity->GetIsTypePed())
	{
		CVector TargetCoors;
		float MaxVerticalRotation=DEGTORAD(89.5f);
		float MaxRotationUp=DEGTORAD(60.0f);
		float MaxRotationDown=DEGTORAD(89.5f);
	 	float 	fStickX=0.0f; 
		float   fStickY=0.0f; 
		float StickBetaOffset=0.0f;
		float StickAlphaOffset=0.0f;
		float HeightToNose=0.10f;
		//float DistToNose=0.19f;
		float DistBack=0.19f;
		float StartingDPadHoriz=70.0f;
		float StartingDPadVert=70.0f;
		static bool FailedTestTwelveFramesAgo=false;
		TargetCoors=ThisCamsTarget;
		static float DPadHorizontal;
		static float DPadVertical;
		static float TargetFOV=0.0f;

		if (ResetStatics)
		{
			Beta=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;//makes player initially look in the directioin that the player is facing
			Alpha=0.0f;
			m_fInitialPlayerOrientation=((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2;
			FailedTestTwelveFramesAgo=false;
			ResetStatics=FALSE;
			DPadHorizontal=0;
			DPadVertical=0;
			m_bCollisionChecksOn=true;
			FOVSpeed=0.0f;
			TargetFOV=FOV;
		}	
		
		
			
		if (((CPed*)CamTargetEntity)->m_nPedFlags.bIsDucking) 
		{
		 	DistBack=0.8f;
		}
		
		ASSERTMSG(CamTargetEntity->GetIsTypePed(), "flubba dubba");
		
		/////////////////////////////////////
		((CPed*)CamTargetEntity)->UpdateRwMatrix();
		((CPed*)CamTargetEntity)->UpdateRwFrame();
		((CPed*)CamTargetEntity)->UpdateRpHAnim();
		/////////////////////////////////////
		CVector posn;
		((CPed*)CamTargetEntity)->GetBonePosition(posn, BONETAG_HEAD, true);
		
		Source=posn;
		Source.z+= HeightToNose;
		
		// when ducking the ped's head tends to move further forward and a bit right - need to compensate
		if(((CPed*)CamTargetEntity)->m_nPedFlags.bIsDucking)
		{
			Source.x -= fDuckingBackOffset*CamTargetEntity->GetMatrix().GetForward().x;//CMaths::Cos(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
			Source.y -= fDuckingBackOffset*CamTargetEntity->GetMatrix().GetForward().y;//CMaths::Sin(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
			Source.x -= fDuckingRightOffset*CamTargetEntity->GetMatrix().GetRight().x;
			Source.y -= fDuckingRightOffset*CamTargetEntity->GetMatrix().GetRight().y;
		}
		else
		{
			Source.x -= DistBack*CamTargetEntity->GetMatrix().GetForward().x;//CMaths::Cos(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
			Source.y -= DistBack*CamTargetEntity->GetMatrix().GetForward().y;//CMaths::Sin(((CPed*)CamTargetEntity)->m_fCurrentHeading+PI/2)*DistBack;
		}

				 
		bool bUsingMouse = false;
		
		#ifdef GTA_PC
			RwV2d m_MouseMovement=CPad::GetPad(0)->GetAmountMouseMoved();
			if ((m_MouseMovement.x==0.0f) && (m_MouseMovement.y==0.0f))	
			{
				fStickX= 	-(CPad::GetPad(0)->SniperModeLookLeftRight());
				fStickY=	CPad::GetPad(0)->SniperModeLookUpDown();	
			}
			else
			{
				fStickX=-m_MouseMovement.x * 3.0f;//CTimer::GetTimeStep(); //	-(CPad::GetPad(0)->LookAroundLeftRight());
				fStickY=m_MouseMovement.y * 4.0f;//CTimer::GetTimeStep(); //	CPad::GetPad(0)->LookAroundUpDown();	
				bUsingMouse = true;
			}
		#else
			{
				fStickX= 	-(CPad::GetPad(0)->SniperModeLookLeftRight());
				fStickY=	CPad::GetPad(0)->SniperModeLookUpDown();	
			}
		#endif

		if(bUsingMouse)
		{
			StickBetaOffset = TheCamera.m_fMouseAccelHorzntl * fStickX * (FOV/80.0f);
			StickAlphaOffset = TheCamera.m_fMouseAccelVertical * fStickY * (FOV/80.0f);
		}
		else
		{
			float X_Sign=1.0f;
			float Y_Sign=1.0f;
		
			if (fStickX<0){X_Sign=-1.0f;}
			if (fStickY<0){Y_Sign=-1.0f;}

			StickBetaOffset=X_Sign * (fStickX/100.0f * fStickX/100.0f) * (0.20f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
			StickAlphaOffset=Y_Sign *(fStickY/150.0f * fStickY/150.0f) * (0.25f/3.5f * (FOV/80.0f)) * CTimer::GetTimeStep();
		}

		Beta+= StickBetaOffset;
		Alpha+=StickAlphaOffset;						
		ClipBeta();
		
		
		if (Alpha>(0.0f + MaxRotationUp))
		{Alpha=0.0f + MaxRotationUp;}
		else if (Alpha<(0.0f-MaxRotationDown))
		{Alpha=0.0f-MaxRotationDown;}
		////////////////////////
		
		TargetCoors=Source + 3 * CVector(CMaths::Cos(Beta) *CMaths::Cos(Alpha) , CMaths::Sin(Beta)*CMaths::Cos(Alpha), CMaths::Sin(Alpha));
		
		bool SniperJustMovedByMouseWheel=false;
		
		#ifdef GTA_PC
			int MsZoomIn=ControlsManager.GetMouseButtonAssociatedWithAction(PED_SNIPER_ZOOM_IN);
			int MsZoomOut=ControlsManager.GetMouseButtonAssociatedWithAction(PED_SNIPER_ZOOM_OUT);
			
			if (MsZoomIn!=NO_BUTTON) 
			{
				if ((MsZoomIn==WHEEL_MS_UP)||(MsZoomIn==WHEEL_MS_DOWN)||(MsZoomOut==WHEEL_MS_UP)||(MsZoomOut==WHEEL_MS_DOWN))
				{			
					if ((CPad::GetPad(0)->GetMsWheelForward())||(CPad::GetPad(0)->GetMsWheelBackward()))
					{	

						if (CPad::GetPad(0)->SniperZoomIn()) 
						{					
							SniperJustMovedByMouseWheel=true;					
							TargetFOV=FOV-10.0f;
						}
						else if (CPad::GetPad(0)->SniperZoomOut())
						{
							SniperJustMovedByMouseWheel=true;	
							TargetFOV=FOV+10.0f;
						}
					}
				}
			}
		
		#endif

		if (((CPad::GetPad(0)->SniperZoomOut()) || (CPad::GetPad(0)->SniperZoomIn()))&&(SniperJustMovedByMouseWheel==false)) 
		{
			if (CPad::GetPad(0)->SniperZoomOut()) 
			{
				FOV *= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
	//			TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE, MB_BLUR_SNIPER_ZOOM); //cause it gets subtracted later
				FOVSpeed=0.0f;
				TargetFOV=FOV;
			}
			else 
			{
				if (CPad::GetPad(0)->SniperZoomIn()) 
				{
					FOV /= (10000.0f + 255.0f * CTimer::GetTimeStep()) / 10000.0f;
	//				TheCamera.SetMotionBlur(180,255,180,145+MOTION_BLUR_ALPHA_DECREASE,MB_BLUR_SNIPER_ZOOM); //cause it gets subtarcted later
					FOVSpeed=0.0f;
					TargetFOV=FOV;	
				}
			}
		}
		else
		{
			if ((ABS(TargetFOV - FOV))>0.5f)
			{
				WellBufferMe(TargetFOV, &FOV, &FOVSpeed, 0.5f, 0.25f, false);
			}
			else
			{
				FOVSpeed=0.0f;
			}
		}	
	
	
	/*	else
		{
			TheCamera.SetMotionBlur(180,255,180,BlurAmount,MB_BLUR_SNIPER); //cause it gets subtarcted later
			
		}
	*/
		TheCamera.SetMotionBlur(180,255,180,120/*145+MOTION_BLUR_ALPHA_DECREASE*/,MB_BLUR_SNIPER); //cause it gets subtarcted later

			
			
			
		// Clip the FOV to sensible values
		if (FOV > 70.0f) FOV = 70.0f; //for streaming

		if(Mode==CCam::MODE_CAMERA)
		{			
			if (FOV < 3.0f) FOV = 3.0f;
		}
		else
			if (FOV < 15.0f) FOV = 15.0f;
		
		//lets check 2 meters in front of the sniper if it is obscured by something, set the clipping distance to short
		Front = TargetCoors- Source;
		float fSourceTargetDist = Front.Magnitude();
		if(fSourceTargetDist >0.0001f)
			Front /= fSourceTargetDist;
		else
			Front.x = 1.0f;
		
		Source+=0.4f*Front;//so it doesn't count the player
						  //this is fixed back later on	
		//right lets check 2 meters in front of the player if we find anything set the clipping distance to short
		if (m_bCollisionChecksOn==true)
		{
			if (!(CWorld::GetIsLineOfSightClear(TargetCoors, Source ,1, 1 , 0, true ,0, true, true)))
		 	{	
		 		RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
		 		FailedTestTwelveFramesAgo=true;
		 	}
			else  
			{
				CVector TestTarget;
				TestTarget=Source + 3 * CVector(CMaths::Cos(Beta + DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta+ DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
				if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
		 		{		
					RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
					FailedTestTwelveFramesAgo=true;
				}
				else
				{
					TestTarget=Source + 3 * CVector(CMaths::Cos(Beta - DEGTORAD(35)) *CMaths::Cos(Alpha - DEGTORAD(20)) , CMaths::Sin(Beta - DEGTORAD(35))*CMaths::Cos(Alpha - DEGTORAD(20)), CMaths::Sin(Alpha - DEGTORAD(20)));
					if (!(CWorld::GetIsLineOfSightClear(TestTarget, Source ,1, 1 , 0, true ,0, true, true)))
			 		{
				 		RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
						FailedTestTwelveFramesAgo=true;
					}
					else
					{
						FailedTestTwelveFramesAgo=false;
					}
				}
			}
		}
		
		if (FailedTestTwelveFramesAgo)
		{
			RwCameraSetNearClipPlane(Scene.camera, SNIPER_NEAR_CLIP);
		} //for occasions when the spaz user is right up against a wall etc
		else if(Mode==CCam::MODE_CAMERA)
		{
			float fNearClipScale = 1.0f + fCameraNearClipMult*(15.0f - MIN(15.0f, FOV));
			RwCameraSetNearClipPlane(Scene.camera, fNearClipScale*NORMAL_NEAR_CLIP);
		}
		
		Source-=0.4f*Front;//so it doesn't count the player
						  //this is fixed back later on	

		GetVectorsReadyForRW();
		float CamDirection;
		CamDirection=CGeneral::GetATanOfXY(Front.x, Front.y)-(PI/2);
		((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
		((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;
	}
#endif	
}


///////////////////////////////////////////////////////
void CCam::Process_Player_Fallen_Water(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired)
{
#if 0 // DW - removed
	float DistanceBehind=4.5f;
	CColPoint colPoint;
	CEntity* pHitEntity=NULL;
	FOV=70.0f;
	Source=	m_vecLastAboveWaterCamPosition;


	Source.z=m_vecLastAboveWaterCamPosition.z + 4.0f;
	m_cvecTargetCoorsForFudgeInter=ThisCamsTarget;
	Front=ThisCamsTarget-Source;
	Front.Normalise();

	if (CWorld::ProcessLineOfSight(ThisCamsTarget , Source,  colPoint, pHitEntity, true, false, false, true, false, true, true)) 
	{
		Source=colPoint.GetPosition();
	}
	GetVectorsReadyForRW();
	Front=ThisCamsTarget-Source;
	Front.Normalise();
#endif
}

/*
void CCam::AvoidWallsTopDownPed(const CVector &TargetCoors,  const CVector& OffsetToAdd, float *TargetAdjusterForDirection, float *TargetAdjusterForDirectionSpeed, float MinDistBeforeGoFast)
{
	CVector TestSource;
	CVector  ;
	float TargetExtra=0.0f;
	float AdjusterTopSpeed=0.13f ;
	float AdjusterSpeedStep=0.015f;
	float TestDistSize=0.0f;
	CVector  TestDistVect;
	CColPoint TestColPoint;
	CEntity* pHitEntity=NULL; 
	//check behind
	TargetExtra=0.0f;
	AdjusterTopSpeed=0.13f ;
	AdjusterSpeedStep=0.015f;
	TestDistSize=0.0f;
	TestSource= TargetCoors + OffsetToAdd;
	TestTarget = TargetCoors;
	TestTarget.z = TestSource.z;
	
	if (CWorld::ProcessLineOfSight(TestTarget, TestSource, TestColPoint, pHitEntity, true, false, false, false, false, false, false))
	{
		TestDistVect=TestTarget-TestColPoint.GetPosition();
		TestDistSize=TestDistVect.y;
		if (TestDistSize>MinDistBeforeGoFast)
		{
			TestDistSize=MinDistBeforeGoFast;
		}
		float AdjusterSpeedMultiplier=MinDistBeforeGoFast-ABS(TestDistSize/MinDistBeforeGoFast); 
		TargetExtra=2.5f;
		AdjusterTopSpeed+=0.3f*AdjusterSpeedMultiplier;
		AdjusterSpeedStep+= 0.03f*AdjusterSpeedMultiplier;
	}

	WellBufferMe(TargetExtra, TargetAdjusterForDirection , TargetAdjusterForDirectionSpeed,  AdjusterTopSpeed, AdjusterSpeedStep,  false);
}



/////////////////////////////////////////////////////////////////
// NAME       : Using3rdPersonMouseCam
// PURPOSE    : Is 3rd person mouse mode enabled, and is that 
//				camera mode currently being used
/////////////////////////////////////////////////////////////////

bool CCam::Using3rdPersonMouseCam(void)
{
	if(TheCamera.m_bUseMouse3rdPerson && ( Mode==MODE_FOLLOWPED || 
	(TheCamera.m_bPlayerIsInGarage && FindPlayerPed() && FindPlayerPed()->GetPedState()!=PED_DRIVING)
	&&(Mode!=MODE_TOPDOWN && CamTargetEntity==FindPlayerPed())))
		return true;
	
	return false;
}



bool CCam::CollisionPointsVerticalLineCheck(const CVector &LineStart, const float& zEnd, const  CVector &TargetPosition, const float MinimumDistBelowRoof, const float MinimumDistAboveGround, float& HighestClearZ, bool CurrentlyObscured,const CVector &CameraPos)
{
	CEntity* pEntity=NULL;
	float ZDiff;
	CVector TestSource=CameraPos;
	CColPoint ColPoint;
	bool FoundFirstClear=false;
	float HighestSoFar=0.0f;
	
	
	if (CWorld::ProcessVerticalLine_FillGlobeColPoints(LineStart, zEnd, pEntity, true, false, false, true, false, false, false))
	{
		//sort the collision points into order
		//lowest to highest
		qsort(gaTempSphereColPoints, FilledColPointIndex, sizeof(gaTempSphereColPoints[0]), &cmp);
		
		int PointIndex=0;
		while ((PointIndex<FilledColPointIndex))
		{
			ZDiff=TestSource.z-gaTempSphereColPoints[PointIndex].GetPosition().z;
			if (((ABS(ZDiff))<MinimumDistAboveGround)||(CurrentlyObscured==true))
			{
				TestSource.z=gaTempSphereColPoints[PointIndex].GetPosition().z + MinimumDistAboveGround;									
				if (!(CWorld::ProcessLineOfSight(TargetPosition, TestSource, ColPoint, pEntity, true, false, false, false, false, false, false)))
				{
					if (FoundFirstClear==false)
					{
						FoundFirstClear=true;
						HighestSoFar=gaTempSphereColPoints[PointIndex].GetPosition().z +  MinimumDistAboveGround;
					}
					else
					{
						HighestSoFar=MAX(HighestSoFar, gaTempSphereColPoints[PointIndex].GetPosition().z + MinimumDistAboveGround);
					}
				}
			
			
				TestSource.z=gaTempSphereColPoints[PointIndex].GetPosition().z - MinimumDistBelowRoof;									
				if (!(CWorld::ProcessLineOfSight(TargetPosition, TestSource, ColPoint, pEntity, true, false, false, false, false, false, false)))
				{

					if (FoundFirstClear==false)
					{
						FoundFirstClear=true;
						HighestSoFar=gaTempSphereColPoints[PointIndex].GetPosition().z - MinimumDistBelowRoof ;
					}
					//need to check that it's not an overhang type thing 
					//if the collision planes are less than one meter apart you can end up being
					//20 cm from the top
					int PointCheck=0;
					float TestHighestSoFar=0.0f;
					bool ToCloseToOtherPlane=false;
					while ((PointCheck<FilledColPointIndex)&&(ToCloseToOtherPlane==false))
					{
						TestHighestSoFar=MAX(HighestSoFar, gaTempSphereColPoints[PointIndex].GetPosition().z - MinimumDistBelowRoof);
						//check if to close to other plane
						if (PointCheck!=PointIndex)
						{
							if (gaTempSphereColPoints[PointCheck].GetPosition().z>TestHighestSoFar)
							{
								if ((gaTempSphereColPoints[PointCheck].GetPosition().z-TestHighestSoFar)<MinimumDistBelowRoof)
								{
									ToCloseToOtherPlane=true;
								}
							}
						}
						PointCheck++;
					}
					if	(ToCloseToOtherPlane==false)
					{
						HighestSoFar=MAX(HighestSoFar, gaTempSphereColPoints[PointIndex].GetPosition().z - MinimumDistBelowRoof);
					}
				}
				
			}
			
			PointIndex++;
		}
	}
	if (FoundFirstClear)
	{
		HighestClearZ=HighestSoFar;
	}
	return FoundFirstClear;
}

///////////////////////////////////////////////////////////////////////////
// NAME       : PrintMode()
// PURPOSE    : Print out the mode that this cam is in at the moment
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCam::PrintMode()
{
	char Str[250];
	if (PrintDebugCode==TRUE)
	{
	sprintf (Str, "                                                   ") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
    CDebug::PrintToScreenCoors(Str, 3, 9);
	sprintf (Str, "                                                   ") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
    CDebug::PrintToScreenCoors(Str, 3, 10);
	sprintf (Str, "                                                   ") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
    CDebug::PrintToScreenCoors(Str, 3, 11);
	}

	/*
		case MODE_NONE:
			sprintf (Str, " Game Default") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
	       break;
		case MODE_TOPDOWN:
			sprintf (Str, " CamMode TopDown") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		 	break;
		case MODE_GTACLASSIC:
			sprintf (Str, " CamMode GTAClassic") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
			break;
		case MODE_BEHINDCAR:
			sprintf (Str, " CamMode BehindCar") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
            break;
		case MODE_FOLLOWPED:
			sprintf (Str, " CamMode Followped") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
            break;
		case MODE_AIMING:
			sprintf (Str, " CamMode Aiming") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_DEBUG:
			sprintf (Str, " CamMode Debug") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
            break;
		case MODE_CAM_ON_A_STRING:
			sprintf (Str, " CamMode Camera_on_a_string") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
            break;
		case MODE_SNIPER:
			sprintf (Str, " CamMode Sniper") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_ROCKETLAUNCHER:
		sprintf (Str, " CamMode RocketLauncher") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_MODELVIEW:
		sprintf (Str, " CamMode Modelview") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_BILL:
		sprintf (Str, " CamMode bill") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_SYPHON:
		sprintf (Str, " CamMode Syphon") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_CIRCLE:
		sprintf (Str, " CamMode Circle") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_CHEESYZOOM:
		sprintf (Str, " CamMode CheesyZoom") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_WHEELCAM:
		sprintf (Str, " CamMode Wheelcam") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_FIXED:
		sprintf (Str, " CamMode Fixed") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		case MODE_1STPERSON:
		sprintf (Str, " CamMode 1rsPerson") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
         CDebug::PrintToScreenCoors(Str, 3, 13);
            	break;
		case MODE_FLYBY:
		sprintf (Str, " CamMode None") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
		  break;
		default:
		sprintf (Str, " Unknown cam mode") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
            CDebug::PrintToScreenCoors(Str, 3, 13);
	

	}
	*/
/*	if (DebugCamMode!=CCam::MODE_NONE)
	{	
		switch(Mode)
		{
		    if (PrintDebugCode==TRUE)
			{
			case MODE_FOLLOWPED:
				sprintf (Str, "Debug:- Cam Choice1. No Locking, used as game default") ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
	            CDebug::PrintToScreenCoors(Str, 3, 9);
	            break;
			
			case MODE_REACTION:
				sprintf (Str, "Debug:- Cam Choice2. Reaction Cam On A String ") ;
				CDebug::PrintToScreenCoors(Str, 3, 9);
	           sprintf (Str,  "        Uses Locking Button LeftShoulder 1. ") ;
	            CDebug::PrintToScreenCoors(Str, 3, 10);
	           
	            break;
			case MODE_FOLLOW_PED_WITH_BIND:
				sprintf (Str, "Debug:- Cam Choice3. Game ReactionCam with Locking ") ;
				CDebug::PrintToScreenCoors(Str, 3, 9);
	           	sprintf (Str, "        Uses Locking Button LeftShoulder 1. ") ;
	            CDebug::PrintToScreenCoors(Str, 3, 10);
			break;
			case MODE_CHRIS:
				sprintf (Str, "Debug:- Cam Choice4. Chris's idea.  ") ;
				CDebug::PrintToScreenCoors(Str, 3, 9);
	           	sprintf (Str, "        Uses Locking Button LeftShoulder 1. ") ;
	            CDebug::PrintToScreenCoors(Str, 3, 10);
				sprintf (Str, "        Also control the camera using the right analogue stick.") ;
	            CDebug::PrintToScreenCoors(Str, 3, 11);
			break;	
		  }
		}
	}

}

/////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// NAME       : Process_Circle()
// PURPOSE    : Camera mode with camera circling around player.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCam::Process_Circle(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
	CVector		TempRight;
	FOV=70.0f;
	Front = CVector( CMaths::Cos(0.7f)*CMaths::Sin((CTimer::GetTimeInMilliseconds() & 4095) * (2.0f*PI/4096.0f)), CMaths::Cos(0.7f)*CMaths::Cos((CTimer::GetTimeInMilliseconds() & 4095) * (2.0f*PI/4096.0f)), -CMaths::Sin(0.7f));

	Source = ThisCamsTarget - (Front * 4.0f);
	Source.z += 1.0f;
	GetVectorsReadyForRW();
}



///////////////////////////////////////////////////////////////////////////
// NAME       : Process_TopDown()
// PURPOSE    : Processes the topdown camera modes.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCam::Process_TopDown(const CVector &ThisCamsTarget, float TargetOrientation,
						   float SpeedVar, float SpeedVarDesired)
{
	FOV=70.0f;
	if (CamTargetEntity->GetIsTypeVehicle())//cheap nasty bug fix
	{
		CVector		ToAimForTarget;
		CVector     TargetCoors;
		float		Multiplier=0.0f;
		float 		TargetHeightOffSetForSpeed=0.0f;
		const 		float FarClipDistance=200.0f;
		static 		float AdjustHeightTargetMoveBuffer=0.0f;
		static  	float AdjustHeightTargetMoveSpeed=0.0f;
		static 		float NearClipDistance=1.5f;	

		// Look ahead a wee bit
		TargetCoors=ThisCamsTarget;
		ToAimForTarget = ThisCamsTarget;
		
		ToAimForTarget.x += CamTargetEntity->GetMatrix().xy * CAMTD_MAXLOOKAHEAD * SpeedVar;
		ToAimForTarget.y += CamTargetEntity->GetMatrix().yy * CAMTD_MAXLOOKAHEAD * SpeedVar;

		if (ResetStatics)
		{
			AdjustHeightTargetMoveBuffer=0.0f;
			AdjustHeightTargetMoveSpeed=0.0f;
		}
		
		Multiplier = CMaths::Pow( 0.8f, 4.0f);
		TargetCoors = (ToAimForTarget*(1.0f - Multiplier)) + (TargetCoors*Multiplier);

		if (Mode == MODE_GTACLASSIC)	// No buffering for the GTA cam
		{
			SpeedVar = SpeedVarDesired;
		}
				
		// Just pick any coordinates just now.
	//	Source = TargetCoors + CVector(0.0f, 0.0f, (CAMTD_MINDIST + (CAMTD_MAXDIST - CAMTD_MINDIST) * SpeedVar) * pow(0.8f, m_fCarZoomSmoothed) );
		Source = TargetCoors + CVector(0.0f, 0.0f, (CAMTD_MINDIST + (CAMTD_MAXDIST - CAMTD_MINDIST) * SpeedVar) * 0.8f);

		if (Mode == MODE_GTACLASSIC)
		{
			Source.x += (((Int16)(ThisCamsTarget.x * 100.0f)) & 0xff) * 0.002f;
		}

		//don't go through roofs
		CVector TestSourceForHeight;
		CVector TestTargetForHeight;
		CColPoint TestColPoint;
		CEntity* pHitEntity=NULL;
		TestSourceForHeight=Source; //make target parellel to camera
		TestTargetForHeight=TestSourceForHeight;
		TestTargetForHeight.z=TargetCoors.z;
		if (CWorld::ProcessLineOfSight(TestTargetForHeight, TestSourceForHeight, TestColPoint, pHitEntity, true, false, false, false, false, false, false)) //found a roof below
		{
			//we've found roof, dont go any lower than it
			if ((TestColPoint.GetPosition().z + 3.0)>(Source.z))
			{
				TargetHeightOffSetForSpeed=TestColPoint.GetPosition().z + 3.0 - Source.z;
			}
		}
		else //hmmm no roof below check five meters above
		{
			TestSourceForHeight=Source; //make target parellel to camera
			TestTargetForHeight=TestSourceForHeight;
			TestTargetForHeight.z=TestSourceForHeight.z+10.0f;
			if (CWorld::ProcessLineOfSight(TestTargetForHeight, TestSourceForHeight, TestColPoint, pHitEntity, true, false, false, false, false, false, false)) //found a roof below
			{
				//we've found roof, dont go any lower than it
				if ((TestColPoint.GetPosition().z + 3.0)>(Source.z))
				{
					TargetHeightOffSetForSpeed=TestColPoint.GetPosition().z + 3.0 - Source.z;
				}
			}
		}

		WellBufferMe(TargetHeightOffSetForSpeed, &(AdjustHeightTargetMoveBuffer), &(AdjustHeightTargetMoveSpeed), 0.20f, 0.02f);
		Source.z+=AdjustHeightTargetMoveBuffer;
		if (RwCameraGetFarClipPlane(Scene.camera)>FarClipDistance) //don't want to set it if it is small 
		//would spoli fogginess type effects
		{
				RwCameraSetFarClipPlane(Scene.camera, (RwReal)(FarClipDistance));
		}

		RwCameraSetNearClipPlane(Scene.camera, (RwReal)(NearClipDistance));

		
		Front = CVector(-0.01f, -0.01f, -1.0f);

		Front.Normalise();
		
		float CamCarDis=CVector(Source-ThisCamsTarget).Magnitude();
		m_cvecTargetCoorsForFudgeInter=Source+(CamCarDis*Front);

		Up = CVector(0.0f, 1.0f, 0.0f);
		ResetStatics=false;
	}
}

/////

void CCam::Process_TopDownPed(const CVector &ThisCamsTarget, float TargetOrientation,
						   float SpeedVar, float SpeedVarDesired)
{
	if (CamTargetEntity->GetIsTypePed())
	{
		CVector     TargetCoors;
		CVector 	Dist;
		int 		MoveState=0;
		float 		DistBetweenPedAndPlayer=0.0f;
		float		Multiplier=0.0f;
		float 		TargetHeightOffSetForSpeed=0.0f;
		static 		int	NumPedPosCountsSoFar=0;
		static 		float PedAverageSpeed=0.0f;
		static 		float AdjustHeightTargetMoveBuffer=0.0f;
		static  	float AdjustHeightTargetMoveSpeed=0.0f;
		static 		float PedSpeedSoFar=0.0f;
		static 		CVector PreviousPlayerMoveSpeedVec;
		const   	float MaxDistanceTargetAway=4.0f;	
		static 		float FarClipDistance=200.0f;
		static 		float NearClipDistance=1.5f;
		static 		float TargetAdjusterForSouth=0.0f;
		static 		float TargetAdjusterSpeedForSouth=0.0f;
		static 		float TargetAdjusterForNorth=0.0f;
		static 		float TargetAdjusterSpeedForNorth=0.0f;
		static 		float TargetAdjusterForEast=0.0f;
		static 		float TargetAdjusterSpeedForEast=0.0f;
		static 		float TargetAdjusterForWest=0.0f;
		static 		float TargetAdjusterSpeedForWest=0.0f;

		const int 	WorkOutPedSpeedThisNumFrames=5;
		const int 	yDistBehind=1;
		const float DefaultHeightUp=9.0f;
		const float HeightToAdd=3.0f;
		CVector		TempPlayerMoveSpeedVec;
		
		FOV=70.0f;
		TargetCoors=ThisCamsTarget;
		
		ASSERTMSG(CamTargetEntity->GetIsTypePed(), "In Top Down Ped Mode but not Ped");

		TempPlayerMoveSpeedVec=((CPed*)CamTargetEntity)->GetMoveSpeed();

		if (ResetStatics)
		{
			AdjustHeightTargetMoveBuffer=0.0f;
			AdjustHeightTargetMoveSpeed=0.0f;
			NumPedPosCountsSoFar=0;
			PreviousPlayerMoveSpeedVec=TempPlayerMoveSpeedVec;
			PedSpeedSoFar=0.0f;
			PedAverageSpeed=0.0f;
			TargetAdjusterForSouth=0.0f;
			TargetAdjusterSpeedForSouth=0.0f;
			TargetAdjusterForNorth=0.0f;
			TargetAdjusterSpeedForNorth=0.0f;
			TargetAdjusterForEast=0.0f;
			TargetAdjusterSpeedForEast=0.0f;
			TargetAdjusterForWest=0.0f;
			TargetAdjusterSpeedForWest=0.0f;
		}
		
		if (RwCameraGetFarClipPlane(Scene.camera)>FarClipDistance) //don't want to set it if it is small 
		//would spoli fogginess type effects
		{
				RwCameraSetFarClipPlane(Scene.camera, (RwReal)(FarClipDistance));
		}
		
		RwCameraSetNearClipPlane(Scene.camera, (RwReal)(NearClipDistance));
		//MoveState=((CPed*)CamTargetEntity)->GetMoveState();
		
		CVector DistDiff=CVector(ThisCamsTarget - PreviousPlayerMoveSpeedVec);

		PedSpeedSoFar+=CMaths::Sqrt(TempPlayerMoveSpeedVec.x *TempPlayerMoveSpeedVec.x + TempPlayerMoveSpeedVec.y * TempPlayerMoveSpeedVec.y + TempPlayerMoveSpeedVec.z * TempPlayerMoveSpeedVec.z); //this is a running total
		
		NumPedPosCountsSoFar++; //counter
		
		if (NumPedPosCountsSoFar==WorkOutPedSpeedThisNumFrames)//duh
		{
			PedAverageSpeed=(0.4f*PedAverageSpeed) + (0.6f*PedSpeedSoFar/WorkOutPedSpeedThisNumFrames);
			PedSpeedSoFar=0.0f;
			NumPedPosCountsSoFar=0;
		}
		
		PreviousPlayerMoveSpeedVec=TempPlayerMoveSpeedVec;
		
	//	char Str[200];
	//	sprintf (Str, "Average Speed %f", PedAverageSpeed) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
	//    CDebug::PrintToScreenCoors(Str, 3, 13);
		
		if ((PedAverageSpeed>0.01f)&&(PedAverageSpeed<=0.04f))//PEDMOVE_WALK 0.027 seems to be the threshold do wee bit over to be sure 
		{
			TargetHeightOffSetForSpeed=2.5f;			
		}
		else if ((PedAverageSpeed>0.04f) && (PedAverageSpeed<=0.145f))// PEDMOVE_RUN: 0.12 seems to be the threshold do wee bit over to be sure 
		{
			TargetHeightOffSetForSpeed=4.5f;
		}
		else if (PedAverageSpeed>0.145f)// PEDMOVE_SPRINT: 0.2 seems to be the fastest he can run
		{
			TargetHeightOffSetForSpeed=7.0f;
		}
		else //ped is still 
		{
			TargetHeightOffSetForSpeed=0.0f;
		}
		
		if (FindPlayerPed()->GetWeaponLockOn())
		{
			if ((CPed*)FindPlayerPed()->m_pEntLockOnTarget!=NULL)
			{
				Dist=FindPlayerPed()->m_pEntLockOnTarget->GetPosition() - ThisCamsTarget;
				DistBetweenPedAndPlayer=CMaths::Sqrt(Dist.x * Dist.x + Dist.y * Dist.y);
			//	sprintf (Str, "PedPlayer Dist%f", DistBetweenPedAndPlayer) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
		   // 	CDebug::PrintToScreenCoors(Str, 3, 14);
				if (DistBetweenPedAndPlayer>6.0f)
				{
					TargetHeightOffSetForSpeed=MAX(TargetHeightOffSetForSpeed, ((DistBetweenPedAndPlayer/22.0f) * 37.0f)); //22.0 is the max dist person can be away. 35.0 max height up  
				}
			}
		}
		
		Source=TargetCoors + CVector(0.0f, -yDistBehind, DefaultHeightUp);
		//don't go through roofs
		CVector TestSourceForHeight;
		CVector TestTargetForHeight;
		CColPoint TestColPoint;
		CEntity* pHitEntity=NULL;
		TestSourceForHeight=TargetCoors + CVector(0.0f, -yDistBehind, DefaultHeightUp); //make target parellel to camera
		TestSourceForHeight.z+=TargetHeightOffSetForSpeed;
		TestTargetForHeight=TestSourceForHeight;
		TestTargetForHeight.z=TargetCoors.z;
		if (CWorld::ProcessLineOfSight(TestTargetForHeight, TestSourceForHeight, TestColPoint, pHitEntity, true, false, false, false, false, false, false)) //found a roof below
		{
			//we've found roof, dont go any lower than it
			if ((TestColPoint.GetPosition().z + 3.0)>(TargetCoors.z+DefaultHeightUp+TargetHeightOffSetForSpeed))
			{
				TargetHeightOffSetForSpeed=TestColPoint.GetPosition().z + 3.0 -DefaultHeightUp-TargetCoors.z;
			}
		}
		else //hmmm no roof below check five meters above
		{
			TestSourceForHeight=TargetCoors + CVector(0.0f, -yDistBehind, DefaultHeightUp); //make target parellel to camera
			TestSourceForHeight.z+=TargetHeightOffSetForSpeed;
			TestTargetForHeight=TestSourceForHeight;
			TestTargetForHeight.z=TestSourceForHeight.z+10.0f;
			if (CWorld::ProcessLineOfSight(TestTargetForHeight, TestSourceForHeight, TestColPoint, pHitEntity, true, false, false, false, false, false, false)) //found a roof below
			{
				//we've found roof, dont go any lower than it
				if ((TestColPoint.GetPosition().z + 3.0)>(TargetCoors.z+DefaultHeightUp+TargetHeightOffSetForSpeed))
				{
					TargetHeightOffSetForSpeed=TestColPoint.GetPosition().z + 3.0 -DefaultHeightUp-TargetCoors.z;
				}
			}
		}
		
		
		WellBufferMe(TargetHeightOffSetForSpeed, &(AdjustHeightTargetMoveBuffer), &(AdjustHeightTargetMoveSpeed), 0.3f, 0.03f);
		Source.z+=AdjustHeightTargetMoveBuffer;
		
		CVector OffsetToAdd;
		//check south
		OffsetToAdd=CVector(0.0f, -3.0f, HeightToAdd);		
	    AvoidWallsTopDownPed(TargetCoors, OffsetToAdd, &TargetAdjusterForSouth, &TargetAdjusterSpeedForSouth, yDistBehind);
		Source.y+=TargetAdjusterForSouth;
		//check north
		OffsetToAdd=CVector(0.0f, 3.0f, HeightToAdd);		
	    AvoidWallsTopDownPed(TargetCoors, OffsetToAdd, &TargetAdjusterForNorth, &TargetAdjusterSpeedForNorth, yDistBehind);
		Source.y-=TargetAdjusterForNorth;
		//check west
		OffsetToAdd=CVector(3.0f, 0.0f, HeightToAdd);		
	    AvoidWallsTopDownPed(TargetCoors, OffsetToAdd, &TargetAdjusterForWest, &TargetAdjusterSpeedForWest, yDistBehind);
		Source.x-=TargetAdjusterForWest;
		//check east
		OffsetToAdd=CVector(-3.0f, 0.0f, HeightToAdd);		
	    AvoidWallsTopDownPed(TargetCoors, OffsetToAdd, &TargetAdjusterForEast, &TargetAdjusterSpeedForEast, yDistBehind);
		Source.x+=TargetAdjusterForEast;
		
		
		TargetCoors.y= yDistBehind + Source.y;
		TargetCoors.y+= TargetAdjusterForSouth;
		//TargetCoors.y-= TargetAdjusterForNorth;
		TargetCoors.x+= TargetAdjusterForEast;
		TargetCoors.x-= TargetAdjusterForWest;
		
		
		
		Front=TargetCoors-Source;
		Front.Normalise();
		if ((Front.x=0.00f) && (Front.y=0.00f))
		{
			Front.y=0.0001f;
		}
		m_cvecTargetCoorsForFudgeInter=TargetCoors;
		CVector TempRight=CVector(-1.0f, 0.0f, 0.0f);
		Up = CrossProduct(Front, TempRight);
		Up.Normalise();
		ResetStatics=false;

	}
}
*/

///////////////////////////////////////////////////////////////////////////
// NAME       : Process_LightHouse()
// PURPOSE    :
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
//static float fLightHouseSwitchBuffer = 0.0f;
//static float LIGHTHOUSE_SWITCH_TIME = 24.0f;
//
void CCam::Process_LightHouse(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
#if 0
	Source=ThisCamsTarget;
	Source.x = 474.3f;
	Source.y = -1717.6f;
	int32 nCameraChoice = 0;

	// need to buffer the switch between modes / angles
	if(ThisCamsTarget.z > 57.0f && (ThisCamsTarget - Source).Magnitude2D() > 3.2f)
	{
		if(fLightHouseSwitchBuffer > 0.0f){
			fLightHouseSwitchBuffer -= CTimer::GetTimeStep();
			nCameraChoice = 1;
		}
		else{
			fLightHouseSwitchBuffer = -LIGHTHOUSE_SWITCH_TIME;
			nCameraChoice = 2;
		}
	}
	else if(ThisCamsTarget.z > 57.0f)
	{
		if(fLightHouseSwitchBuffer < 0.0f){
			fLightHouseSwitchBuffer += CTimer::GetTimeStep();
			nCameraChoice = 2;
		}
		else{
			fLightHouseSwitchBuffer = LIGHTHOUSE_SWITCH_TIME;
			nCameraChoice = 1;
		}
	}
	else
	{
		fLightHouseSwitchBuffer = 0.0f;
		nCameraChoice = 0;
	}
	
	if(nCameraChoice==2)
	{
		Source.z = 57.5f;
		Front = Source - ThisCamsTarget;
		Front.Normalise();
		
		Source.x = ThisCamsTarget.x - 5.0f*Front.x;
		Source.y = ThisCamsTarget.y - 5.0f*Front.y;
	}
	else if(nCameraChoice==1)
	{
		Front=ThisCamsTarget-Source;
		Front.Normalise();
		
		Source.x -= 2.0f * Front.x;
		Source.y -= 2.0f * Front.y;
	}
	else
	{
		Source.z += 4.0f;
		Front=ThisCamsTarget-Source;
		Front.Normalise();
		Source -= 4.0f * Front;		// Pull cam back a bit to give more space
		Source.z = MIN(Source.z, 55.0f);
		Front=ThisCamsTarget-Source;
	}	
	
	m_cvecTargetCoorsForFudgeInter=ThisCamsTarget;
	GetVectorsReadyForRW();
	Up=CVector(0.0f,0.0f,1.0f);
	Up+=m_cvecCamFixedModeUpOffSet;
	Up.Normalise();
	CVector TempRight= CrossProduct(Front, Up);
	TempRight.Normalise();
	Up=CrossProduct(TempRight, Front);


	FOV=70.0f;
	if (TheCamera.m_bUseSpecialFovTrain==true)
	{
		FOV=TheCamera.m_fFovForTrain;
	}
	
/* this doesn't do anything cause Using3rdPersonMouseCam() always returns false since camera Mode isn't follow ped
	#ifdef GTA_PC
	if (FrontEndMenuManager.m_ControlMethod == STANDARD_CONTROLLER_SCREEN)
	{
		if (Using3rdPersonMouseCam())
		{
			CPlayerPed* pPed=FindPlayerPed();
			if (pPed)
			{
				if(pPed->CanStrafeOrMouseControl())
				{
					float CamDirection;
					CamDirection = CMaths::ATan2(-Front.x, Front.y);
					((CPed*)TheCamera.pTargetEntity)->m_fCurrentHeading=CamDirection;
					((CPed*)TheCamera.pTargetEntity)->m_fDesiredHeading=CamDirection;
					// need to manually set the entity heading here as well to keep in sync with camera
					TheCamera.pTargetEntity->SetHeading(CamDirection);
					TheCamera.pTargetEntity->GetMatrix().UpdateRW();
				}
			}
		}
	}
	#endif
*/
#endif
}


///////////////////////////////////////////////////////////////////////////
// NAME       : Cam_On_A_String_Unobscured(CVector TargetCoors, float CarLength)
// PURPOSE    : Do the plain old simple camera on a string algorythm
// RETURNS    : Nothing
// PARAMETERS : The TargetCoors of the car and the carLength as this is used to work out the max distance
///////////////////////////////////////////////////////////////////////////
void CCam::Cam_On_A_String_Unobscured(const CVector &TargetCoors, float CarLength)
{
#if 0
	CVector VecDistance;
	float DistMagnitude=0.0f;
	float RemoteControlledDistAway=0.0f;
	UInt32 VehicleModIndex=0;
 	VehicleModIndex=CamTargetEntity->GetModelIndex();
 	if(VehicleModIndex==MODELID_CAR_RCCOPTER || VehicleModIndex==MODELID_CAR_RCGOBLIN)
	{
		RemoteControlledDistAway=INIT_RC_HELI_HORI_EXTRA;
	}
	else if(VehicleModIndex==MODELID_CAR_RCBARON)
	{
		RemoteControlledDistAway=INIT_RC_PLANE_HORI_EXTRA;	
	}
	
	
	CA_MAX_DISTANCE=CarLength + 0.1f + TheCamera.m_fCarZoomSmoothed + RemoteControlledDistAway;
	CA_MIN_DISTANCE=3.5f;//3.5f 

	if (CA_MIN_DISTANCE<(CarLength*0.6f)){CA_MIN_DISTANCE=(CarLength*0.6f);}//previously didn't have this // was .6

	//last check
	if (CA_MIN_DISTANCE>CA_MAX_DISTANCE)
	{
		CA_MIN_DISTANCE=CA_MAX_DISTANCE-0.05f;
	}

	ASSERTMSG(CA_MIN_DISTANCE>=0.0f,  "Camera too close to car");
	VecDistance=Source- TargetCoors ;
	//mark delete this when they decided they don't want the look 
	//get into car cam

	if (ResetStatics)
	{
		Source=TargetCoors+((CA_MAX_DISTANCE+1)*VecDistance);
	}


	VecDistance=Source- TargetCoors ;
	DistMagnitude=CMaths::Sqrt(VecDistance.x*VecDistance.x + VecDistance.y * VecDistance.y);		

	if (DistMagnitude<0.001f)
	{
		CVector CarForward=CamTargetEntity->GetMatrix().GetForward();
		CarForward.z=0.0f;
		CarForward.Normalise();
		Source=TargetCoors-(CA_MAX_DISTANCE*CarForward);
		VecDistance=Source-TargetCoors ;
		DistMagnitude=CMaths::Sqrt(VecDistance.x*VecDistance.x + VecDistance.y * VecDistance.y);
	} 
	 
	 
	//////ORIGINAL CAM ON A STRING
	if (DistMagnitude> CA_MAX_DISTANCE)
	{
		// Suss vector between object and camera 
		// Move Camera 
		Source.x=  TargetCoors.x+(VecDistance.x*(CA_MAX_DISTANCE/DistMagnitude));
		Source.y=  TargetCoors.y+(VecDistance.y*(CA_MAX_DISTANCE/DistMagnitude));
	}      
	else
	{
	  // is player too close to the camera 
	  	if (DistMagnitude < CA_MIN_DISTANCE)
	  	{
	     	// Suss vector between object and camera   
	     	// Move Camera 
		 	Source.x=  TargetCoors.x+(VecDistance.x*(CA_MIN_DISTANCE/DistMagnitude));
		 	Source.y=  TargetCoors.y+(VecDistance.y*(CA_MIN_DISTANCE/DistMagnitude));
	  	}
	}
#endif	
}


#ifdef GTA_TRAIN

void CCamera::LoadTrainCamNodes(const char *FileName)
{
	CFileMgr::SetDir("data\\paths");
	int32 filePosn = 0;
	int32 fileSize = 0;
	int32 ArrayNumInData=0;
	char FloatHolder[15]={0};	
	int FloatPosition=0;
	uint8* NodeBuffer;
	int32 NodeBufferSize=20000;
	char FileHolder[16]={'/0'};
	int  DatInsertPoint;


	strcpy(FileHolder, FileName );
	DatInsertPoint=strlen(FileHolder);
	ASSERTMSG(DatInsertPoint<8, "HISSSSSS");
	FileHolder[DatInsertPoint]='.';
	FileHolder[DatInsertPoint+1]='d';
	FileHolder[DatInsertPoint+2]='a';
	FileHolder[DatInsertPoint+3]='t';

	m_uiNumberOfTrainCamNodes = 0;
	NodeBuffer=new uint8[NodeBufferSize];
			
	fileSize=CFileMgr::LoadFile(FileHolder, NodeBuffer, NodeBufferSize);
	ASSERTOBJ(fileSize!=-1, FileName, "Cannot load train file");

///Lets clear any shit that may not have beeen cleared.

	for (int ArrayNo=0; ArrayNo<MAX_NUM_OF_NODES; ArrayNo++)
	{
		m_arrTrainCamNode[ArrayNo].m_cvecCamPosition==CVector(0,0,0);
		m_arrTrainCamNode[ArrayNo].m_cvecPointToLookAt=CVector(0,0,0);
		m_arrTrainCamNode[ArrayNo].m_cvecMinPointInRange=CVector(0,0,0);
		m_arrTrainCamNode[ArrayNo].m_cvecMaxPointInRange=CVector(0,0,0);
		m_arrTrainCamNode[ArrayNo].m_fDesiredFOV=0.0f;
		m_arrTrainCamNode[ArrayNo].m_fNearClip=0.0f;
	}

//we only need a PathArray when we load a path.		
	while (filePosn <= fileSize)
	{
//rightyho then cycle through the numbers and only add a number to the array
//if it is valid
		switch (NodeBuffer[filePosn])
		{	
			case '0' :
			case '1' :	
			case '2' :	
			case '3' :	
			case '4' :	
			case '5' :	
			case '6' :	
			case '7' :	
			case '8' :	
			case '9' :
			case '10':
			case '11':
			case '12':
			case '13':
			case '.' :
			case '-' :
			FloatHolder[FloatPosition]=NodeBuffer[filePosn];
			FloatPosition++;
			filePosn++;
			break;

			case',': //this is the trigger fire it in
			switch ((ArrayNumInData+14)%14)
			{
				case 0:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.x =atof(FloatHolder);
				break;
				case 1:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.y =atof(FloatHolder);
				break;
				case 2:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.z =atof(FloatHolder);
				break;
				case 3:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.x =atof(FloatHolder);
				break;
				case 4:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.y =atof(FloatHolder);
				break;
				case 5:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.z =atof(FloatHolder);
				break;
				case 6:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.x =atof(FloatHolder);
				break;
				case 7:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.y =atof(FloatHolder);
				break;
				case 8:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.z =atof(FloatHolder);
				break;
				case 9:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.x =atof(FloatHolder);
				break;
				case 10:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.y =atof(FloatHolder);
				break;
				case 11:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.z =atof(FloatHolder);
				break;
				case 12:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_fDesiredFOV =atof(FloatHolder);
				break;
				case 13:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_fNearClip =atof(FloatHolder);
				m_uiNumberOfTrainCamNodes++;
				break;
			}
		
			filePosn ++;
			ArrayNumInData++;
			FloatPosition=0;
			while (FloatPosition<14)
			{FloatHolder[FloatPosition]=0;
			FloatPosition++;
			}
			FloatPosition=0; //need to clear it again
			ASSERT (m_uiNumberOfTrainCamNodes<(MAX_NUM_OF_NODES-1));
			break;
		
			case';': //this is the trigger fire it in
			switch ((ArrayNumInData+14)%14)
			{
				case 0:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.x =atof(FloatHolder);
				break;
				case 1:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.y =atof(FloatHolder);
				break;
				case 2:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.z =atof(FloatHolder);
				break;
				case 3:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.x =atof(FloatHolder);
				break;
				case 4:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.y =atof(FloatHolder);
				break;
				case 5:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.z =atof(FloatHolder);
				break;
				case 6:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.x =atof(FloatHolder);
				break;
				case 7:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.y =atof(FloatHolder);
				break;
				case 8:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.z =atof(FloatHolder);
				break;
				case 9:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.x =atof(FloatHolder);
				break;
				case 10:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.y =atof(FloatHolder);
				break;
				case 11:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.z =atof(FloatHolder);
				break;
				case 12:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_fDesiredFOV =atof(FloatHolder);
				case 13:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_fNearClip =atof(FloatHolder);
				m_uiNumberOfTrainCamNodes++;
				break;
			}
		
			filePosn ++;
			ArrayNumInData++;
			FloatPosition=0;
			while (FloatPosition<14)
			{FloatHolder[FloatPosition]=0;
			FloatPosition++;
			}
			FloatPosition=0; //need to clear it again
			ASSERT (m_uiNumberOfTrainCamNodes<(MAX_NUM_OF_NODES-1));
			break;	
		
			default: //allows the program to skip over spaces etc.
			filePosn ++;
		}
	}
	// read all the lines of the file
	delete [] NodeBuffer;
	CFileMgr::SetDir("");
	NodeBuffer=0;
}

#endif // GTA_TRAIN




/*
int cmp(const void *p1, const void *p2)
{
 	CColPoint i = *(CColPoint*)p1; //gets floats pointed to
 	CColPoint j = *(CColPoint*)p2; //gets floats pointed to
 	if ((i.GetPosition().z) < (j.GetPosition().z)) return -1; //first number is less than second
	if ((i.GetPosition().z)== (j.GetPosition().z)) return 0; //equal numbers
	else return  1; //first number is more than second
} 	
*/


///////////////////////////////////////////

bool CCam::IsTargetInWater(const CVector &CamPos)
{
	ASSERT(0);
#if 0 // DW - removed
	if (CamTargetEntity!=NULL)
	{
		float WaterLevel=-6000.0f;
		CVector EntityPos=((CPhysical*)CamTargetEntity)->GetPosition();
		if (!(CWaterLevel::GetWaterLevel(EntityPos.x, EntityPos.y, EntityPos.z, &WaterLevel, true)))
		{
			float WaterLevel=-6000.0f;
		}
		

		if (CamTargetEntity->GetIsTypePed())
		{
			if (((CPed*)CamTargetEntity)->m_nPedFlags.bIsDrowning) 
			{
				if(((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskActiveSimplest() &&
				(  ((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_SWIM
				|| ((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_CLIMB ))
					return false;

				return true;
			}
			else
			{
				if ((((CPhysical*)CamTargetEntity)->m_nPhysicalFlags.bIsInWater)&&(EntityPos.z<WaterLevel))		
				{
					if(((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskActiveSimplest() &&
					(  ((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_SWIM
					|| ((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskActiveSimplest()->GetTaskType()==CTaskTypes::TASK_SIMPLE_CLIMB ))
						return false;
						
					return true;
				}
				else
				{
					m_vecLastAboveWaterCamPosition=Source;
					return false;
				}
			}
		}
		else if(CamTargetEntity->GetIsTypeVehicle())
		{
			if (((CVehicle*)CamTargetEntity)->m_nVehicleFlags.bIsDrowning) 
			{
				return true;
			}
			else
			{
				if ((((CPhysical*)CamTargetEntity)->m_nPhysicalFlags.bIsInWater)&&(EntityPos.z<WaterLevel))	
				{
					return true;
				}
				else
				{
					m_vecLastAboveWaterCamPosition=Source;
					return false;
				}
			}
			
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
#endif	

	return false;
}


////////////////////////////////////////////
CVector CCam::DoAverageOnVector(const CVector &ThingToAverage)
{
#if 1
	CVector AveragedVector(0.0f, 0.0f, 0.0f);
	ASSERT(FALSE);
#else

	if (ResetStatics)
	{
		m_iRunningVectorArrayPos=0;
		m_iRunningVectorCounter=1;
	}

	if (m_iRunningVectorCounter==NUMBER_OF_VECTORS_FOR_AVERAGE+1)//that it all finshed swap over
	{
		CVector TempVectorHolderForSwap[NUMBER_OF_VECTORS_FOR_AVERAGE];
		//copy vectors over
		TempVectorHolderForSwap[0]=m_arrPreviousVectors[1];
	//	TempVectorHolderForSwap[1]=m_arrPreviousVectors[2];
	//	TempVectorHolderForSwap[2]=m_arrPreviousVectors[3];
	//	TempVectorHolderForSwap[3]=ThingToAverage;
		TempVectorHolderForSwap[1]=ThingToAverage;

		m_arrPreviousVectors[0]=TempVectorHolderForSwap[0];
		m_arrPreviousVectors[1]=TempVectorHolderForSwap[1];
	//	m_arrPreviousVectors[2]=TempVectorHolderForSwap[2];
	//	m_arrPreviousVectors[3]=TempVectorHolderForSwap[3];
	}
	else
	{
		m_arrPreviousVectors[m_iRunningVectorArrayPos]=ThingToAverage;
	}
	
	int counter=0;
	while (counter<=m_iRunningVectorArrayPos)
	{
		ASSERTMSG(counter<NUMBER_OF_VECTORS_FOR_AVERAGE, "Going over end of array");
		AveragedVector+=m_arrPreviousVectors[counter];
		counter++;
	}
	
	ASSERT((counter>0) && (counter<=NUMBER_OF_VECTORS_FOR_AVERAGE));
	AveragedVector=(AveragedVector/counter);
	
	m_iRunningVectorArrayPos++;
	m_iRunningVectorCounter++;
	
	//clip
	if (m_iRunningVectorArrayPos>=NUMBER_OF_VECTORS_FOR_AVERAGE)
	{
		m_iRunningVectorArrayPos=NUMBER_OF_VECTORS_FOR_AVERAGE-1;
	}
	
	if (m_iRunningVectorCounter>(NUMBER_OF_VECTORS_FOR_AVERAGE+1))
	{
		m_iRunningVectorCounter=NUMBER_OF_VECTORS_FOR_AVERAGE+1;
	}
	
#endif	
	return AveragedVector;		
}



#if 0  //DWREMOVED

		case MODE_FIGHT_CAM:
			ASSERT(FALSE);
//			Process_Fight_Cam(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;

//		case MODE_PLAYER_FALLEN_WATER:
//			Process_Player_Fallen_Water(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
//			break;		case MODE_TOPDOWN:
		case MODE_GTACLASSIC:
		case MODE_TOP_DOWN_PED:
			ASSERTMSG(false, "You'll find the code for this commented out at the bottom of the file");
			break;
		case MODE_SYPHON_CRIM_IN_FRONT:
			ASSERTMSG(false, "You'll find the code for this commented out at the bottom of the file");
//DWREMOVED			Process_Syphon_Crim_In_Front(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;


		case MODE_SYPHON:
			ASSERTMSG(false, "You'll find the code for this commented out at the bottom of the file");
//DW REMOVED Process_Syphon(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
		
		case MODE_CIRCLE:
			ASSERTMSG(false, "You'll find the code for this commented out at the bottom of the file");
		//	Process_Circle(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
			
		//case MODE_LIGHTHOUSE:
			//Process_LightHouse(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			//break;
	/*	case MODE_LOOK_AT_CARS:
			Process_Look_At_Cars();
			break;
	*/
//		case MODE_PLAYER_FALLEN_WATER:		// mode to be removed		

		case MODE_BEHINDBOAT:
//		if(TEST_FOLLOW_CAR_USING_FOLLOW_PED_CODE)
//			Process_FollowCar_SA(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
//DWREMOVED		else
//DWREMOVED			Process_Follow_History(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
//			Process_Cam_On_A_String(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
//			Process_BehindBoat(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;			

/*		case MODE_SPECIAL_FIXED_FOR_SYPHON:
			Process_SpecialFixedForSyphon(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;*/

		case MODE_BEHINDCAR:	
			//Process_Cam_On_A_String(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			Process_BehindCar(ThisCamsTarget, TargetOrientation, SpeedVar, SpeedVarDesired);
			break;
#endif


#if 0 // DW - should be redundant code now. - from camera:: process
   	if ((Mode==CCam::MODE_SNIPER)||(Mode==CCam::MODE_ROCKETLAUNCHER)||(Mode==CCam::MODE_ROCKETLAUNCHER_HS)
	||(Mode==CCam::MODE_M16_1STPERSON)||(Mode==CCam::MODE_1STPERSON)
	||(Mode==CCam::MODE_HELICANNON_1STPERSON)||(Mode==CCam::MODE_CAMERA)||(GetWeaponFirstPersonOn()))          
	{
		ClipIfPedInFrontOfPlayer();
	}
#endif

#if 0
bool CCam::LookLeft(void)
{
	bool bIsCar 	= (((Mode==MODE_CAM_ON_A_STRING)||(Mode==MODE_BEHINDBOAT)||(Mode==MODE_BEHINDCAR))&&(CamTargetEntity->GetIsTypeVehicle()));
	bool bIsPed 	= CamTargetEntity->GetIsTypePed();
	bool bIsFPCar 	= (Mode==MODE_1STPERSON && CamTargetEntity->GetIsTypeVehicle());

	if (!bIsCar && !bIsFPCar && !bIsPed)
		return false;
		
	CColPoint colPoint;
	bool ViewNotClear=FALSE;
	CVector TempDistance, TargetCoors;
	float Offset=(DEGTORAD(90.0f));
	if (bIsCar)
	{
		LookingLeft=true;
		CVector TargetCoors;
		bool ViewNotClear=FALSE;
		TargetCoors = CamTargetEntity->GetPosition();

		float GroundDistance=0.0f;
		if (Mode==MODE_CAM_ON_A_STRING)
		{
			GroundDistance=CA_MAX_DISTANCE;
		}
		else if (Mode==MODE_BEHINDBOAT) ///in a boat
		{
			GroundDistance=9.0f;
			float BoatLookLeftAndRightOffset=0.0f;
			if (GetBoatLook_L_R_HeightOffset(BoatLookLeftAndRightOffset) && !CCullZones::Cam1stPersonForPlayer())
			{
				Source.z=TargetCoors.z + BoatLookLeftAndRightOffset;
			}
		}
		else
		{
			GroundDistance=9.0f;
		}
		

	
		CVector CarForward=CamTargetEntity->GetMatrix().GetForward();
		CarForward.Normalise();
		float CarAngle=CGeneral::GetATanOfXY(CarForward.x, CarForward.y);

		Source.x=TargetCoors.x + (GroundDistance*CMaths::Cos(CarAngle-Offset));
		Source.y=TargetCoors.y + (GroundDistance*CMaths::Sin(CarAngle-Offset));
	
		CColModel &TargetColModel = CamTargetEntity->GetColModel();
		CVector TempSource = Source;
		CVector TempTarget = TargetCoors;
		float fOriginalHeight = Source.z;
		float VehicleWidth = TargetColModel.GetBoundBoxMax().x;
		VehicleWidth = ABS(VehicleWidth);
//		CVector TempFront = Source - TargetCoors;
//		TempFront.Normalise();
//		TempTarget += VehicleWidth*TempFront;

#ifdef DW_CAMERA_COLLISION
		CWorld::pIgnoreEntity = CamTargetEntity;
		TheCamera.CameraGenericModeSpecialCases(NULL);
		TheCamera.CameraVehicleModeSpecialCases((CVehicle*)CamTargetEntity);
		TheCamera.CameraColDetAndReact(&Source,&TempTarget);			
		CWorld::pIgnoreEntity = NULL;
#else
		TheCamera.AvoidTheGeometry(TempSource,TempTarget, Source, FOV);
#endif		

		// try and keep the camera at a min height above the vehicle (but not higher than it was alreay)
		CVector vecRightSideHeight = CamTargetEntity->GetPosition();
		vecRightSideHeight += CamTargetEntity->GetMatrix().GetRight()*TargetColModel.GetBoundBoxMax().x;
		vecRightSideHeight += CamTargetEntity->GetMatrix().GetUp()*TargetColModel.GetBoundBoxMax().z;
		float fLimitHeight = MIN(fOriginalHeight, MAX(m_cvecTargetCoorsForFudgeInter.z, vecRightSideHeight.z) + 0.1f);
		Source.z = MAX(fLimitHeight, Source.z);


// aghh - this bit does NOTHING cause the Front vector is recalculated somewhere later in the code, damnit
		Front =  CamTargetEntity->GetPosition() - Source;
		Front.z += 1.1f;
		if (Mode==MODE_BEHINDBOAT)
		{
			Front.z += 1.2f;
		}
////////////////////////////////
		
		GetVectorsReadyForRW();
	}
 	
	if (bIsFPCar)
	{
		LookingLeft=true;
		RwCameraSetNearClipPlane(Scene.camera, FIRSTPERSON_CAR_NEAR_CLIP); //for occasions when the spaz user is right up against a wall etc
		if (((CVehicle*)CamTargetEntity)->GetBaseVehicleType()==VEHICLE_TYPE_BOAT)
		{
			if(((CVehicle *)CamTargetEntity)->pDriver)
			{
				CVector vecShoulderHeight(0.0f,0.0f,0.0f);
				CPed *pPed = ((CVehicle *)CamTargetEntity)->pDriver;
				/////////////////////////////////////
				pPed->SetPedPositionInCar();
				pPed->UpdateRwMatrix();
				pPed->UpdateRwFrame();
				pPed->UpdateRpHAnim();
				/////////////////////////////////////
				pPed->GetBonePosition(vecShoulderHeight, BONETAG_NECK, true);
				vecShoulderHeight += BOAT_1STPERSON_L_OFFSETX*CamTargetEntity->GetMatrix().GetRight();
				vecShoulderHeight += BOAT_1STPERSON_LR_OFFSETZ*CamTargetEntity->GetMatrix().GetUp();
				Source = vecShoulderHeight;
			}
			else
				Source.z-=ZOFFSET1RSTPERSONBOAT;
		}
 		Up=CamTargetEntity->GetMatrix().GetUp();
		Up.Normalise();
		Front=CamTargetEntity->GetMatrix().GetForward();
		Front.Normalise();
		Front=-CrossProduct(Front, Up);
		Front.Normalise();
 		if (((CVehicle*)CamTargetEntity)->GetVehicleAppearance()==APR_BIKE)
 		{
 			Source-=1.45f*Front;
 		}
 	//	GetVectorsReadyForRW();
 	}
 	
 	return true; 	
}

#endif


/////////////////////////////////////////////////////////
//float FIGHT_HORIZ_DIST = 3.0f;
//float FIGHT_VERT_DIST = 1.0f;
//float FIGHT_BETA_ANGLE = 125.0f;
// 
void CCam::Process_Fight_Cam(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
#if 0
	if (CamTargetEntity->GetIsTypePed())
	{
		FOV=70.0f;
		const float CamDistanceAwayFromPed=FIGHT_HORIZ_DIST;	//4.0f
		const float CamZOffset=FIGHT_VERT_DIST;				//-0.5f;
		const float TargetDistanceAwayPlayer=0.1f;//0.5f;
		float BetaTopStep=0.015f;
		float BetaSpeedStep=0.007f;
		float LateralLeftOffset=0.0f;
		float LateralRightOffset=0.0f;
		CVector TargetCoors;
		static bool PreviouslyFailedBuildingChecks=false;
		m_fMinDistAwayFromCamWhenInterPolating=CamDistanceAwayFromPed;
		float DeltaBeta=0.0f;
		//lets work our beta first of all
		Front=Source-ThisCamsTarget;
		if (ResetStatics)
		{
			Beta=CGeneral::GetATanOfXY(Front.x, Front.y);
		}
		while (TargetOrientation>=(PI)) {TargetOrientation-=2.0f * PI;}
		while (TargetOrientation<(-PI)) {TargetOrientation+=2.0f * PI;}

		while (Beta>=(PI)) {Beta-=2.0f * PI;}
		while (Beta<(-PI)) {Beta+=2.0f * PI;}

		float RotateLeft=TargetOrientation-DEGTORAD(FIGHT_BETA_ANGLE);//90.0f
		float RotateRight=TargetOrientation+DEGTORAD(FIGHT_BETA_ANGLE);//90.0f
		
		float DeltaBetaLeft=Beta-RotateLeft;
		float DeltaBetaRight=Beta-RotateRight;
		while (DeltaBetaLeft>=(PI)) {DeltaBetaLeft-=2.0f * PI;}
		while (DeltaBetaLeft<(-PI)) {DeltaBetaLeft+=2.0f * PI;}
		while (DeltaBetaRight>=(PI)) {DeltaBetaRight-=2.0f * PI;}
		while (DeltaBetaRight<(-PI)) {DeltaBetaRight+=2.0f * PI;}
		
		
		if (ResetStatics)
		{
			if (ABS(DeltaBetaLeft)<ABS(DeltaBetaRight))
			{
				m_fTargetBeta=RotateLeft;
			}
			else
			{
				m_fTargetBeta=RotateRight;
			}
			m_fBufferedTargetOrientation=TargetOrientation;
			m_fBufferedTargetOrientationSpeed=0.0f;
		    m_bCollisionChecksOn=true;
			BetaSpeed=0.0f;
		}
		else
		{
			if (CPad::GetPad(0)->GetMeleeAttack())
			{
				if (ABS(DeltaBetaLeft)<ABS(DeltaBetaRight))
				{
					m_fTargetBeta=RotateLeft;
				}
				else
				{
					m_fTargetBeta=RotateRight;
				}
			}
		}
		
		
	    
		
		WellBufferMe(m_fTargetBeta, &Beta, &BetaSpeed, BetaTopStep, BetaSpeedStep, true);
		Source=ThisCamsTarget + CamDistanceAwayFromPed*CVector(CMaths::Cos(Beta), CMaths::Sin(Beta), 0);
		Source.z+=CamZOffset;

		WellBufferMe(TargetOrientation, &m_fBufferedTargetOrientation, &m_fBufferedTargetOrientationSpeed, 0.07f, 0.004f, true);
		TargetCoors=ThisCamsTarget+(TargetDistanceAwayPlayer*CVector(CMaths::Cos(m_fBufferedTargetOrientation), CMaths::Sin(m_fBufferedTargetOrientation), 0));	
		
		float TargetZOffSet=0.0f;
		TargetZOffSet=MAX((m_fDimensionOfHighestNearCar), m_fPedBetweenCameraHeightOffset);
		float FinalTarget=ThisCamsTarget.z+TargetZOffSet-Source.z+CamZOffset;
		

		
		
		if (FinalTarget>m_fCamBufferedHeight)
		{
			WellBufferMe(FinalTarget, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed), 0.15f, 0.04f);
		}
		else
		{
			WellBufferMe(0.0f, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed), 0.08f, 0.0175f);
		}
		
		Source.z+=m_fCamBufferedHeight;
		m_cvecTargetCoorsForFudgeInter=TargetCoors;
		
		CVector TempSource=Source;
		TheCamera.AvoidTheGeometry(TempSource,TargetCoors, Source, FOV);
		Front=TargetCoors-Source;

		Front.Normalise();
		GetVectorsReadyForRW();	
		ResetStatics=false;
	}
#endif	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : Process_CushyPillows_Arse
// PURPOSE    : Looks at bums and paps
// RETURNS    : 
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
#ifdef GTA_IDLECAM
bool CCam::Process_CushyPillows_Arse(void)
{
	float MaxDistanceAwayFromLady=45.0f;
	CVector TargetCoors;
	static bool BirdFound=false;
	static CPed* pBird=NULL;
	static float TargetHeight=0.0f;
	static float TargetHeightSpeed=0.0f;
	static CVector InitialCamPos;
	float MaxFOV=60.0f;
	float TargetFOV=60.0f;
	CVector posn;
	int i=0;


	if (ResetStatics==true)
	{
		BirdFound=false;
		CPed* pBird=NULL;
		TargetHeight=0.0f;
		TargetHeightSpeed=0.0f;
		m_fCamBufferedHeight=0.0f;
		m_fCamBufferedHeightSpeed=0.0f;
		ResetStatics=false;
		InitialCamPos=Source;
	}

	CPed* PlayerPed=CWorld::Players[CWorld::PlayerInFocus].pPed;
	ASSERTMSG(PlayerPed!=NULL, "ANOTHER QUALITY BIT OF CODE");

	while ((i < (PlayerPed->m_NumClosePeds))&&(BirdFound==false))
	{
		if(	(CVector(InitialCamPos - (PlayerPed->m_apClosePeds[i]->GetPosition())).Magnitude() < MaxDistanceAwayFromLady)&&(PlayerPed->m_apClosePeds[i]->GetIsOnScreen())&&
		((PlayerPed->m_apClosePeds[i]->m_nPedType ==PEDTYPE_CIVFEMALE)||(PlayerPed->m_apClosePeds[i]->m_nPedType ==PEDTYPE_PROSTITUTE)))
		{
		//now check that the ped is not driving
			if (PlayerPed->m_apClosePeds[i]->m_nPedState!=PED_DRIVING)	
			{
				BirdFound=true;
				pBird=(PlayerPed->m_apClosePeds[i]);
				REGREF(((CEntity*)pBird), ((CEntity**)&pBird));
				pBird->GetBonePosition(posn, BONETAG_SPINE1, false);
				TargetCoors=posn;
				TargetHeight=posn.z;
			}
		}
		i++;
	}
   
	i--; //just to get it back to the value it was
	//right found a bird lets use the moral, decent arse and paps cam
	if ((BirdFound==true)&&(pBird!=NULL))
	{
				
		CVector CamToBird=pBird->GetPosition()-InitialCamPos; 
		CVector BirdFront=pBird->GetMatrix().GetForward();
		CamToBird.Normalise();
		BirdFront.Normalise();
		

		//sprintf (Str, "CamToBirdAngle %f", RADTODEG(CMaths::Abs(ResultAngle))) ;// TX : %f, TY: %f, TZ: %f, ThePPosX %f, ThepPosy %f, ThePPosZ %f ", ,ThePlayerPos.x,ThePlayerPos.y,ThePlayerPos.z);
		//CDebug::PrintToScreenCoors(Str, 3, 18); 	
			
		//Right if they are walking straight to us then ResultAngle>DEGTORAD(90.0)
		//And if they are in front then we want to look at their paps

		//If they are have passed us then we want to stare a their arse
		pBird->GetBonePosition(posn, BONETAG_SPINE1, false);
		TargetCoors=posn;

		if (DotProduct(CamToBird, BirdFront)>0)//she's passed us
		{
			TargetCoors.z-=0.30f;
		}
		else
		{
			TargetCoors.z-=0.02f;//0.27
		}

		WellBufferMe(TargetCoors.z, &TargetHeight, &TargetHeightSpeed, 0.015f, 0.007f);
		TargetCoors.z=TargetHeight;
		
		
		
		bool LineOfSightClear=true;

		LineOfSightClear=CWorld::GetIsLineOfSightClear(InitialCamPos, TargetCoors , 1, 0, 0, true,0, true, true);
		

		if (LineOfSightClear)		
		{
			CVector SourceBeforeChange=InitialCamPos;
		}
		
 	
		float FinalTarget=TargetCoors.z+ m_fPedBetweenCameraHeightOffset-InitialCamPos.z;

		if (&(TheCamera.Cams[TheCamera.ActiveCam])==this)
		{
			if (m_fPedBetweenCameraHeightOffset!=0.0f)
			{
				WellBufferMe(FinalTarget, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed), 0.20f, 0.04f);
			}
			else
			{
				WellBufferMe(0.0f, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed),  0.20f, 0.025f);
			}

			
			Source.z=InitialCamPos.z + m_fCamBufferedHeight;
		}		
		
		Front=TargetCoors-Source;
		
			
		if(Front.Magnitude()!=0.0f)
		{
			if ((Front.Magnitude()>MaxDistanceAwayFromLady)||(LineOfSightClear==false))
			{
				TargetFOV=(1/Front.Magnitude()) * 50.0f;//180
				Front.Normalise();
				//clip
				if (TargetFOV>MaxFOV)
				{
					TargetFOV=MaxFOV;
				}
				WellBufferMe(TargetFOV, &FOV, &FOVSpeed,0.020f, 0.009f);
				GetVectorsReadyForRW();
				return false;
			}
			else
			{
					TargetFOV=(1/Front.Magnitude()) * 50.0f;
					Front.Normalise();
					if (TargetFOV>MaxFOV)
					{
						TargetFOV=MaxFOV;
					}
					
					WellBufferMe(TargetFOV, &FOV, &FOVSpeed,0.020f, 0.009f);
					GetVectorsReadyForRW();
					return true;
			}
		}
	}

	return false;

}
#endif

///////////////////////////////////////////////////////////////////////////
void CCam::Process_SpecialFixedForSyphon(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
#ifdef CAMERA_USE_DEAD_TARGET_FIXED_CAM
	Source=m_cvecCamFixedModeSource;
	
	m_cvecTargetCoorsForFudgeInter=ThisCamsTarget ;
	m_cvecTargetCoorsForFudgeInter.z+=m_fSyphonModeTargetZOffSet;
	Front=ThisCamsTarget-Source;
	CVector SourceBeforeChange=Source;
	TheCamera.AvoidTheGeometry(SourceBeforeChange, m_cvecTargetCoorsForFudgeInter, Source, FOV);
	Front.z+=m_fSyphonModeTargetZOffSet;
	GetVectorsReadyForRW();
	Up+=m_cvecCamFixedModeUpOffSet;
	Up.Normalise();
	CVector TempRight=CrossProduct(Up, Front);
	TempRight.Normalise();
	Front=CrossProduct(TempRight, Up);
	Front.Normalise();
	FOV=70.0f;
	
	if(CamTargetEntity && CamTargetEntity->GetIsTypePed() && ((CPed *)CamTargetEntity)->GetWeaponLockOnTarget())
	{
		CPed *pPed = (CPed *)CamTargetEntity;
		CWeaponInfo* pWeaponInfo = CWeaponInfo::GetWeaponInfo(pPed->GetWeapon()->GetWeaponType(), pPed->GetWeaponSkill());
		
		if(pWeaponInfo && (!pWeaponInfo->IsWeaponFlagSet(WEAPONTYPE_CANAIMWITHARM) || pPed->m_nPedFlags.bIsDucking)
		&& pWeaponInfo->GetWeaponFireType()!=FIRETYPE_MELEE)
		{
			CVector vecDelta = pPed->GetWeaponLockOnTarget()->GetPosition() - pPed->GetPosition();
			pPed->m_fCurrentHeading = pPed->m_fDesiredHeading = CMaths::ATan2(-vecDelta.x, vecDelta.y);
			// need to manually set the entity heading here as well to keep in sync with camera
			pPed->SetHeading(pPed->m_fCurrentHeading);
			pPed->UpdateRwMatrix();
		}
	}
#endif	
}


///////////////////////////////////////////////////////////////////////////
// NAME       : Process_BehindCar()
// PURPOSE    : Processes the 'behind the car' camera mode.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

void CCam::Process_BehindCar(const CVector &ThisCamsTarget, float TargetOrientation,
							   float SpeedVar, float SpeedVarDesired)
{
#if 0
	FOV=70.0f;
	if (CamTargetEntity->GetIsTypeVehicle()) //cheap and nasty bug fix for assert  
	//happening 
	{
		CVector VecDistance;
		float DistMagnitude=0.0f;
		//FOV=30.0f;
		CVector TargetCoors=ThisCamsTarget;
		//TargetCoors.z+=0.10;




		TargetCoors.z-=0.2f;

		CA_MAX_DISTANCE=9.95f;
		CA_MIN_DISTANCE=8.5f;//3.5f 

		VecDistance=Source- TargetCoors ;
		DistMagnitude=CMaths::Sqrt(VecDistance.x*VecDistance.x + VecDistance.y * VecDistance.y);		
		m_fDistanceBeforeChanges=DistMagnitude; //used for fixing the acmnera if it is behind a wall
		if (DistMagnitude<0.002)
		{
			ASSERTMSG(0, "BANGO");
			DistMagnitude=0.002f;
		} 
		 

		Beta=CGeneral::GetATanOfXY(TargetCoors.x - Source.x, TargetCoors.y - Source.y );
		//////ORIGINAL CAM ON A STRING
		if (DistMagnitude> CA_MAX_DISTANCE)
		{
			// Suss vector between object and camera 
			// Move Camera 
			Source.x=  TargetCoors.x+(VecDistance.x*(CA_MAX_DISTANCE/DistMagnitude));
			Source.y=  TargetCoors.y+(VecDistance.y*(CA_MAX_DISTANCE/DistMagnitude));
		}      
		else
		{
		  	// is player too close to the camera 
		  	if (DistMagnitude < CA_MIN_DISTANCE)
		  	{
		     	// Suss vector between object and camera   
		     	// Move Camera 
		 		Source.x=  TargetCoors.x+(VecDistance.x*(CA_MIN_DISTANCE/DistMagnitude));
		 		Source.y=  TargetCoors.y+(VecDistance.y*(CA_MIN_DISTANCE/DistMagnitude));
		  	}
		}
		TargetCoors.z+=0.80f;
		Alpha=DEGTORAD(25.0f);;
		Source.z= TargetCoors.z + (CA_MAX_DISTANCE*CMaths::Sin(Alpha));
		RotCamIfInFrontCar(TargetCoors, TargetOrientation);
		m_cvecTargetCoorsForFudgeInter=TargetCoors;
		CVector TempSource=Source;
		TheCamera.AvoidTheGeometry(TempSource,m_cvecTargetCoorsForFudgeInter, Source, FOV);		

		Front = TargetCoors - Source;
		ResetStatics=false;
		GetVectorsReadyForRW();
	}
#endif	
}


#if 0 
	// See at bottom of file .. comment @BLOCK 1@
	CColPoint colPoint;
	CEntity *pHitEntity = NULL;

	CWorld::pIgnoreEntity = (CEntity *) CamTargetEntity;
	// if ped is in a car then their collision will be off anyway
	// so set ignore peds vehicle instead
	if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle)
		CWorld::pIgnoreEntity = pPed->m_pMyVehicle;
		
	if(CWorld::ProcessLineOfSight(vecTargetCoords, Source, colPoint, pHitEntity, true, true, false, false, false,  false, true))
	{
		float fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
		float fDistCamToCol = fDefaultDistFromPed - fDistColToPed;
		
		// if the first thing in the way of the camera is a ped, it might be ok to leave it
		// in the way - need to check there's nothing else closer to the camera tho!
		if(pHitEntity->GetIsTypePed() && fDistCamToCol > NORMAL_NEAR_CLIP + 0.1f)
		{
			if(CWorld::ProcessLineOfSight(colPoint.GetPosition(), Source, colPoint, pHitEntity, true, true, false, false, false, false, true))
			{
				fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
				Source = colPoint.GetPosition();
				if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
			}
			else
			{
				RwCameraSetNearClipPlane(Scene.camera, MIN(0.9f, fDistCamToCol - 0.35f));
			}
		}
		else
		{
			Source = colPoint.GetPosition();
			if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
				RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
		}
	}

	CWorld::pIgnoreEntity = NULL;

	int32 nCountLoop = 0;
	CVector vecTest;
	float fTanFOV = CMaths::Tan(0.5f*DEGTORAD(FOV));
	if(FrontEndMenuManager.m_PrefsUseWideScreen)
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio(); // ASPECTRATIO.
	else
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio();


	float fNearClip = RwCameraGetNearClipPlane(Scene.camera);
	float fNearClipWidth = fNearClip * fTanFOV;

	CEntity *pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
	while(pFindEntity)
	{
		vecTest = gaTempSphereColPoints[0].GetPosition() - Source;
		float fTemp1 = DotProduct(vecTest, Front);

		vecTest = vecTest - fTemp1 * Front;
		fTemp1 = vecTest.Magnitude() / fTanFOV;
		fTemp1 = MAX(0.1f, MIN(fNearClip, fTemp1));

		if(fTemp1 < fNearClip)
			RwCameraSetNearClipPlane(Scene.camera, fTemp1);
		
		// if we couldn't set the near clip close enough to avoid clipping
		// try moving the camera forward by a percentage of the distance to ped
		if(fTemp1 == 0.1f && !(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle==pFindEntity))
			Source = Source + 0.3f * (vecTargetCoords - Source);
		
		// test again incase we got the wrong collision point or stuff
		fNearClip = RwCameraGetNearClipPlane(Scene.camera);
		fNearClipWidth = fNearClip * CMaths::Tan(0.5f*DEGTORAD(FOV)) * fTweakFOV*CDraw::GetAspectRatio();
		pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
		
		// break out anyway after a specified number of tries
		if(++nCountLoop > 5)
			pFindEntity = NULL;
	}


	// now buffer how quickly the camera moves back away from the player too smooth things out a bit
	float fNewDistance = (vecTargetCoords - Source).Magnitude();
	if(fNewDistance < Distance)
	{
		Distance = fNewDistance; //fine
	}
	else// if(Distance < fNewDistance)
	{
		// buffered Distance goes towards fNewDistance (not fIdealDistance)
		float fTimeRateOfChange = CMaths::Pow(fMouseAvoidGeomReturnRate, CTimer::GetTimeStep());
		Distance = fTimeRateOfChange*Distance + (1.0f - fTimeRateOfChange)*fNewDistance;
		
		if(fNewDistance > 0.05f) // just in case
			Source = vecTargetCoords + (Source - vecTargetCoords)*Distance/fNewDistance;
		
		// now we might have ended up closer to the player than AvoidTheGeometry expected
		// so might clip through playerped
		float PlayCamDistWithCol = Distance-fRangePlayerRadius;
		if (PlayCamDistWithCol<RwCameraGetNearClipPlane(Scene.camera))	
		{
			RwCameraSetNearClipPlane(Scene.camera, MAX(PlayCamDistWithCol, 0.1f));
		}	
	}
#endif

#if 0
	// See at bottom of file comment @BLOCK 2@
	CColPoint colPoint;
	CEntity *pHitEntity = NULL;

	CWorld::pIgnoreEntity = (CEntity *) CamTargetEntity;
	// if ped is in a car then their collision will be off anyway
	// so set ignore peds vehicle instead
	if(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle)
		CWorld::pIgnoreEntity = pPed->m_pMyVehicle;
		
	if(CWorld::ProcessLineOfSight(vecTargetCoords, Source, colPoint, pHitEntity, true, true, false, true, false,  true, true))
	{
		float fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
		float fDistCamToCol = fCamDistance - fDistColToPed;
		
		// if the first thing in the way of the camera is a ped, it might be ok to leave it
		// in the way - need to check there's nothing else closer to the camera tho!
		if(pHitEntity->GetIsTypePed() && fDistCamToCol > NORMAL_NEAR_CLIP + 0.1f)
		{
			if(CWorld::ProcessLineOfSight(colPoint.GetPosition(), Source, colPoint, pHitEntity, true, true, false, true, false, true, true))
			{
				fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
				Source = colPoint.GetPosition();
				if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
			}
			else
			{
				RwCameraSetNearClipPlane(Scene.camera, MIN(0.9f, fDistCamToCol - 0.35f));
			}
		}
		else
		{
			Source = colPoint.GetPosition();
			if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
				RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
		}
	}

	CWorld::pIgnoreEntity = NULL;

	int32 nCountLoop = 0;
	CVector vecTest;
	float fTanFOV = CMaths::Tan(0.5f*DEGTORAD(FOV));
	if(FrontEndMenuManager.m_PrefsUseWideScreen)
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio(); // ASPECTRATIO.
	else
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio();


	float fNearClip = RwCameraGetNearClipPlane(Scene.camera);
	float fNearClipWidth = fNearClip * fTanFOV;

	CEntity *pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
	while(pFindEntity)
	{
		vecTest = gaTempSphereColPoints[0].GetPosition() - Source;
		float fTemp1 = DotProduct(vecTest, Front);

		vecTest = vecTest - fTemp1 * Front;
		fTemp1 = vecTest.Magnitude() / fTanFOV;
		fTemp1 = MAX(0.1f, MIN(fNearClip, fTemp1));

		if(fTemp1 < fNearClip)
			RwCameraSetNearClipPlane(Scene.camera, fTemp1);
		
		// if we couldn't set the near clip close enough to avoid clipping
		// try moving the camera forward by a percentage of the distance to ped
//		if(fTemp1 == 0.1f && !(pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle==pFindEntity))
//			Source = Source + 0.3f * (vecTargetCoords - Source);
		
		// test again incase we got the wrong collision point or stuff
		fNearClip = RwCameraGetNearClipPlane(Scene.camera);
		fNearClipWidth = fNearClip * CMaths::Tan(0.5f*DEGTORAD(FOV)) * fTweakFOV*CDraw::GetAspectRatio();
		pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
		
		// break out anyway after a specified number of tries
		if(++nCountLoop > 5)
			pFindEntity = NULL;
	}


	// now buffer how quickly the camera moves back away from the player too smooth things out a bit
	float fNewDistance = (vecTargetCoords - Source).Magnitude();
	if(fNewDistance < Distance)
	{
		Distance = fNewDistance; //fine
	}
	else// if(Distance < fNewDistance)
	{
		// buffered Distance goes towards fNewDistance (not fIdealDistance)
		float fTimeRateOfChange = CMaths::Pow(fMouseAvoidGeomReturnRate, CTimer::GetTimeStep());
		Distance = fTimeRateOfChange*Distance + (1.0f - fTimeRateOfChange)*fNewDistance;
		
		if(fNewDistance > 0.05f) // just in case
			Source = vecTargetCoords + (Source - vecTargetCoords)*Distance/fNewDistance;
		
		// now we might have ended up closer to the player than AvoidTheGeometry expected
		// so might clip through playerped
		float PlayCamDistWithCol = Distance-fRangePlayerRadius;
		if (PlayCamDistWithCol<RwCameraGetNearClipPlane(Scene.camera))	
		{
			RwCameraSetNearClipPlane(Scene.camera, MAX(PlayCamDistWithCol, 0.1f));
		}					
	}
#endif

#if 0
// @block 3@
static float MAX_SPEED_CAM_HISTORY_REQUIRES_COL_TEST = 0.2f;
if(pVehicle->GetMoveSpeed().Magnitude() < MAX_SPEED_CAM_HISTORY_REQUIRES_COL_TEST)
{
	CColPoint colPoint;
	CEntity *pHitEntity = NULL;

	CWorld::pIgnoreEntity = (CEntity *) CamTargetEntity;
	if(CWorld::ProcessLineOfSight(vecTargetCoords, Source, colPoint, pHitEntity, true, false, false, false, false,  false, true))
	{
		float fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
		float fDistCamToCol = fCamDistance - fDistColToPed;
		
		// if the first thing in the way of the camera is a ped, it might be ok to leave it
		// in the way - need to check there's nothing else closer to the camera tho!
		if(pHitEntity->GetIsTypePed() && fDistCamToCol > NORMAL_NEAR_CLIP + 0.1f)
		{
			if(CWorld::ProcessLineOfSight(colPoint.GetPosition(), Source, colPoint, pHitEntity, true, false, false, false, false, false, true))
			{
				fDistColToPed = (vecTargetCoords - colPoint.GetPosition()).Magnitude();
				Source = colPoint.GetPosition();
				if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
					RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
			}
			else
			{
				RwCameraSetNearClipPlane(Scene.camera, MIN(0.9f, fDistCamToCol - 0.35f));
			}
		}
		else
		{
			Source = colPoint.GetPosition();
			if( fDistColToPed < NORMAL_NEAR_CLIP + 0.3f)
				RwCameraSetNearClipPlane(Scene.camera, MAX(0.05f, fDistColToPed - 0.3f));
		}
	}

	CWorld::pIgnoreEntity = NULL;
}
	int32 nCountLoop = 0;
	CVector vecTest;
	float fTanFOV = CMaths::Tan(0.5f*DEGTORAD(FOV));
	if(FrontEndMenuManager.m_PrefsUseWideScreen)
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio(); // ASPECTRATIO.
	else
	    fTanFOV *= fTweakFOV*CDraw::GetAspectRatio();


	float fNearClip = RwCameraGetNearClipPlane(Scene.camera);
	float fNearClipWidth = fNearClip * fTanFOV;

	CEntity *pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
	while(pFindEntity)
	{
		vecTest = gaTempSphereColPoints[0].GetPosition() - Source;
		float fTemp1 = DotProduct(vecTest, Front);

		vecTest = vecTest - fTemp1 * Front;
		fTemp1 = vecTest.Magnitude() / fTanFOV;
		fTemp1 = MAX(0.1f, MIN(fNearClip, fTemp1));

		if(fTemp1 < fNearClip)
			RwCameraSetNearClipPlane(Scene.camera, fTemp1);
		
		// if we couldn't set the near clip close enough to avoid clipping
		// try moving the camera forward by a percentage of the distance to ped
//		if(fTemp1 == 0.1f && !(pPed && pPed->m_nPedFlags.bInVehicle && pPed->m_pMyVehicle==pFindEntity))
//			Source = Source + 0.3f * (vecTargetCoords - Source);
		
		// test again incase we got the wrong collision point or stuff
		fNearClip = RwCameraGetNearClipPlane(Scene.camera);
		fNearClipWidth = fNearClip * CMaths::Tan(0.5f*DEGTORAD(FOV)) * fTweakFOV*CDraw::GetAspectRatio();
		pFindEntity = CWorld::TestSphereAgainstWorld(Source + fNearClip*Front, fNearClipWidth, NULL, true, true, false, true, false);
		
		// break out anyway after a specified number of tries
		if(++nCountLoop > 5)
			pFindEntity = NULL;
	}


	// now buffer how quickly the camera moves back away from the player too smooth things out a bit
	float fNewDistance = (vecTargetCoords - Source).Magnitude();
	if(fNewDistance < Distance)
	{
		Distance = fNewDistance; //fine
	}
	else// if(Distance < fNewDistance)
	{
		// buffered Distance goes towards fNewDistance (not fIdealDistance)
		float fTimeRateOfChange = CMaths::Pow(fMouseAvoidGeomReturnRate, CTimer::GetTimeStep());
		Distance = fTimeRateOfChange*Distance + (1.0f - fTimeRateOfChange)*fNewDistance;
		
		if(fNewDistance > 0.05f) // just in case
			Source = vecTargetCoords + (Source - vecTargetCoords)*Distance/fNewDistance;
		
		// now we might have ended up closer to the player than AvoidTheGeometry expected
		// so might clip through playerped
		float PlayCamDistWithCol = Distance-fRangePlayerRadius;
		if (PlayCamDistWithCol<RwCameraGetNearClipPlane(Scene.camera))	
		{
			RwCameraSetNearClipPlane(Scene.camera, MAX(PlayCamDistWithCol, 0.1f));
		}	
	}
#endif


///////////////////////////////////////////////////////////////////////////
// NAME       : Process_FollowPed()
// PURPOSE    : Processes the zelda-esque 'follow the ped' camera mode.
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
float fAvoidGeomThreshhold = 1.5f;
float fDefaultSpeedStep = 0.025f; //defaults
float fDefaultSpeedMultiplier = 0.09f; //defaults
float fDefaultSpeedLimit = 0.15f;
float fDefaultSpeedStep4Avoid = 0.02f; //defaults
float fDefaultSpeedMultiplier4Avoid = 0.05f; //defaults
float fDefaultSpeedLimit4Avoid = 0.25f;
float fAvoidGeomReturnRate = 0.97f;
float fMiniGunBetaOffset = 0.3f;
//

void CCam::Process_FollowPed(const CVector &ThisCamsTarget, float TargetOrientation, float SpeedVar, float SpeedVarDesired)
{
#if 0 
	if (CamTargetEntity->GetIsTypePed())
	{
		CVector TargetCoors, VecDistance, Dist, ZoomVect, IdealSource, RealGroundVec;
		float Length=0.0f, DistanceOnGround=0.0f, DeltaBeta=0.0f, RotationDist=0.0f;
		float SpeedStep=0.0f, SpeedMultiplier=0.0f, SpeedLimit=0.0f, ReqSpeed=0.0f;
		float  LateralLeft=0.0f, LateralRight=0.0f, Centre=0.0f, LateralFraction=0.0f, CentreMultiplier=0.0f ;
		float TargetAboveGroundFix=0.0f;
		static bool PreviouslyObscured;
		static bool PickedASide;
		static float LastAngleWithNoPickedASide=0.0f;
	    static float FixedTargetOrientation=0.0f;
		
		bool ShootingOrSuch=false;
		
		float AngleToGoTo=0.0f;
		bool Obscured=false;
		bool BuildingCheckObscured=false;
		bool BelowRealGroundMinDist=false;
		bool HackPlayerOnStoppingTrain=false;
		const float TargetCoorsZReductForBuildings=0.10f;//this is low for normal buildings
		float LateralAngleCheckOffsetBuildings=0.45f;
		float LateralAngleOffsetBuildingsGoinBehind=0.45f;

		const float ZReductionForBuildingsRotating=0.10f;
		float TargetGroundDistExtra=0.0f;
		float TargetZDistExtra=0.0f;

		const Int32 TimeBeforeGoDown=0; //five seconds
		float ExtraSideOffsetWhenLookBehindEtc=0.0f;
		float GroundDistForRotatingBuildTest=0.0f;
		static UInt32 TimeIndicatedWantedToGoDown=0;
		static bool StartedCountingForGoDown=false;
		
		static float ExtraHorizontalDistToAdd=0.0f;
		static float ExtraHorizontalDistToAddSpeed=0.0f;

		static float ExtraVerticalDistToAdd=0.0f;	
		static float ExtraVerticalDistToAddSpeed=0.0f;	
		
		m_bFixingBeta=false;
		bBelowMinDist=false;
		bBehindPlayerDesired=false;

		FOV=70.0f;
			
		if (ResetStatics) //because camera has been re-initialised
		{
			FixedTargetOrientation=0.0f;
			Rotating=FALSE;
			PickedASide=FALSE;
			LastAngleWithNoPickedASide=0.0f;
			PreviouslyObscured=false;
			m_bCollisionChecksOn=true;
			AngleToGoTo=0.0f;
			StartedCountingForGoDown=false;
		
			ExtraHorizontalDistToAdd=0.0f;
			ExtraHorizontalDistToAddSpeed=0.0f;

			ExtraVerticalDistToAdd=0.0f;	
			ExtraVerticalDistToAddSpeed=0.0f;	
			Distance=500.0f;
//			DistanceSpeed=0.0f;
		}
		
		TargetCoors=ThisCamsTarget;

		
		ASSERTMSG(CamTargetEntity->GetIsTypePed()==TRUE, "YUBBA");
		
		CVector SpeedOfEntityStoodOn(0.0f,0.0f,0.0f);
		//if we are on a boat we need to  remove the speed of the boat from the Target Coors so that only the ped movement
		//is taken into account 		
		if (((CPed*)CamTargetEntity)->m_pEntityStandingOn)
		{
			if ((((CPed*)CamTargetEntity)->m_pEntityStandingOn->GetIsTypeVehicle())||(((CPed*)CamTargetEntity)->m_pEntityStandingOn->GetIsTypeObject()))
			{
				// first get offset of targetentity relative to the entity it's standing on
				SpeedOfEntityStoodOn = CamTargetEntity->GetPosition() - ((CPed*)CamTargetEntity)->m_pEntityStandingOn->GetPosition();
				// then get the speed of that point on the (stood on) entity
				SpeedOfEntityStoodOn=((CPhysical*)(((CPed*)CamTargetEntity)->m_pEntityStandingOn))->GetSpeed(SpeedOfEntityStoodOn) ;
				SpeedOfEntityStoodOn*=CTimer::GetTimeStep();
			}
		}
	
		Source += SpeedOfEntityStoodOn;
		IdealSource=Source;
		DistanceOnGround=m_fMinRealGroundDist; 
		
		TargetCoors.z += m_fSyphonModeTargetZOffSet; 

		//no w finally work out the average TargetCoors
//		CVector TempTargetCoors=DoAverageOnVector(TargetCoors);
//		TargetCoors.z=TempTargetCoors.z;
		
			
		Dist.x = IdealSource.x - TargetCoors.x;
		Dist.y = IdealSource.y - TargetCoors.y;
		Length = CMaths::Sqrt(Dist.x * Dist.x + Dist.y * Dist.y);

		if (Length != 0.0f)
		{
			IdealSource = TargetCoors + CVector( Dist.x * DistanceOnGround / Length,
										Dist.y * DistanceOnGround / Length,
										SpeedOfEntityStoodOn.z);//This is like finding a unit vector
										//from the player to the camera after the player has moved.
		}
		else
		{
			IdealSource = TargetCoors + CVector(1.0f, 1.0f, 0.0f);
		}
		
		if ((TheCamera.m_bUseTransitionBeta)&&(ResetStatics))
		{
			CVector VecDistance;
			IdealSource.x=TargetCoors.x + DistanceOnGround*CMaths::Cos(m_fTransitionBeta);
			IdealSource.y=TargetCoors.y + DistanceOnGround*CMaths::Sin(m_fTransitionBeta);
			Beta= CGeneral::GetATanOfXY((IdealSource.x - TargetCoors.x),(IdealSource.y - TargetCoors.y));	
		}
		else
		{
			Beta= CGeneral::GetATanOfXY((Source.x - TargetCoors.x),(Source.y - TargetCoors.y));
		}

		
		
		if (TheCamera.m_bCamDirectlyBehind==true)
		{
			 m_bCollisionChecksOn=true;
			 Beta=TargetOrientation+PI;
		}
		
		if (FindPlayerVehicle()!=NULL)
		{
			if ((FindPlayerVehicle()->GetBaseVehicleType())==VEHICLE_TYPE_TRAIN) 
			{
				 HackPlayerOnStoppingTrain=true;
			}
		}
		
		if (TheCamera.m_bCamDirectlyInFront==true)
		{
			 m_bCollisionChecksOn=true;
			 Beta=TargetOrientation;
		}	

		
		
		while (Beta>=(PI)) {Beta-=2.0f * PI;}
		while (Beta<(-PI)) {Beta+=2.0f * PI;}


		
		//oops game is about to eb released no time to be elegant
		if (TheCamera.m_nPedZoom==ZOOM_ONE
		&& ((CPed *)CamTargetEntity)->GetPedState()!=PED_ENTER_CAR && ((CPed *)CamTargetEntity)->GetPedState()!=PED_CARJACK)
		{
			TargetGroundDistExtra=m_fTargetZoomGroundOne;
			TargetZDistExtra=m_fTargetZoomOneZExtra;
		}
		else if (TheCamera.m_nPedZoom==ZOOM_TWO || TheCamera.m_nPedZoom==ZOOM_ONE)
		{
			TargetGroundDistExtra=m_fTargetZoomGroundTwo;
			TargetZDistExtra=m_fTargetZoomTwoZExtra;
		}
		else if (TheCamera.m_nPedZoom==ZOOM_THREE)
		{
			TargetGroundDistExtra=m_fTargetZoomGroundThree;
			TargetZDistExtra=m_fTargetZoomThreeZExtra;
		}
		
		static float SWIMMING_CAMERA_EXTRA_Z = 0.8f;
		if(((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskSwim())
		{
			CTaskSimpleSwim *pSwim = ((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskSwim();
			if(pSwim->m_nSwimState==SWIM_UNDERWATER)
				TargetZDistExtra = -CMaths::Sin(pSwim->m_fDiveAngle)*TargetGroundDistExtra;
			else
				TargetZDistExtra += SWIMMING_CAMERA_EXTRA_Z;
		}

		float fCamWaterLevel = 0.0f;
		if( CWaterLevel::GetWaterLevel(Source.x, Source.y, Source.z, &fCamWaterLevel, true) )
		{
			if(Source.z < fCamWaterLevel - 0.3f)
			{
				float fWaterColourMag = CMaths::Sqrt(CTimeCycle::GetWaterRed()*CTimeCycle::GetWaterRed() + CTimeCycle::GetWaterGreen()*CTimeCycle::GetWaterGreen() + CTimeCycle::GetWaterBlue()*CTimeCycle::GetWaterBlue());
				if(fWaterColourMag > BOAT_UNDERWATER_CAM_COLORMAG_LIMIT)
				{
					fWaterColourMag = BOAT_UNDERWATER_CAM_COLORMAG_LIMIT/fWaterColourMag;
					TheCamera.SetMotionBlur(fWaterColourMag*CTimeCycle::GetWaterRed(), fWaterColourMag*CTimeCycle::GetWaterGreen(), fWaterColourMag*CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);
				}
				else
					TheCamera.SetMotionBlur(CTimeCycle::GetWaterRed(), CTimeCycle::GetWaterGreen(), CTimeCycle::GetWaterBlue(), BOAT_UNDERWATER_CAM_BLUR, MB_BLUR_LIGHT_SCENE);

/*				// trying to make the screen darker with depth but it doesn't seem to be working
				int nDepthAlpha = (int)((fCamWaterLevel - 0.3f - Source.z) / BOAT_UNDERWATER_CAM_DEPTH_SCALE);
				nDepthAlpha = MIN(BOAT_UNDERWATER_CAM_ALPHA_LIMIT, 80 + nDepthAlpha);
				TheCamera.SetMotionBlurAlpha(nDepthAlpha);
*/
			}
		}


		if (m_fCloseInPedHeightOffset>0.00001f)
		{
			TargetGroundDistExtra=m_fTargetCloseInDist;
			TargetZDistExtra=m_fTargetZoomZCloseIn;
		}
		if (ResetStatics)///REALLY NASTY HACK REMOVE AFTER E3
		{
			ExtraHorizontalDistToAdd=TargetGroundDistExtra;
			ExtraVerticalDistToAdd=TargetZDistExtra;
		}
		
		bool ShootingWeapon = false;
		bool SwimmingOrJetPack = false;
		SpeedStep = fDefaultSpeedStep;
		SpeedMultiplier = fDefaultSpeedMultiplier;
		SpeedLimit = fDefaultSpeedLimit;
		
		#ifndef ALIGNED_CAMERA_OFF_FOR_ANDRE
		{
			if(CPad::GetPad(0)->GetWeapon((CPed*)CamTargetEntity)
			&& ((CPed*)CamTargetEntity)->GetWeapon()->m_eWeaponType!=WEAPONTYPE_UNARMED 
			&& ((CPed*)CamTargetEntity)->GetWeapon()->m_eWeaponType!=WEAPONTYPE_DETONATOR
			&& ((CPed*)CamTargetEntity)->GetWeapon()->m_eWeaponType!=WEAPONTYPE_BASEBALLBAT)
			{
				ShootingWeapon = true;
			}
			else if(/*((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskSwim()*/ (((CPed *)CamTargetEntity)->GetPedIntelligence()->GetTaskJetPack() && !((CPed *)CamTargetEntity)->GetIsStanding()))
			{
				SwimmingOrJetPack = true;
			}
		}
		#endif

		if((CPad::GetPad(0)->ForceCameraBehindPlayer() && !CPickups::PlayerOnWeaponPickup)
		|| ShootingWeapon || SwimmingOrJetPack)
		{
			if (PickedASide==FALSE)
			{
				FixedTargetOrientation=TargetOrientation + PI;// + AngleToGoTo;
				Rotating=TRUE;
				PickedASide=TRUE;
				if(ShootingWeapon && ((CPed*)CamTargetEntity)->GetWeapon()->GetWeaponType()==WEAPONTYPE_MINIGUN)
					FixedTargetOrientation -= fMiniGunBetaOffset;
			}
			else //we've picked a side lets stick to it
			{
				if (AngleToGoTo==0.0f)
				{
					FixedTargetOrientation=TargetOrientation + PI;
					if(ShootingWeapon && ((CPed*)CamTargetEntity)->GetWeapon()->GetWeaponType()==WEAPONTYPE_MINIGUN)
						FixedTargetOrientation -= fMiniGunBetaOffset;
				}
				Rotating=TRUE;
			}
		}
		else if(CMaths::Abs(TheCamera.m_fAvoidTheGeometryProbsTimer) > fAvoidGeomThreshhold	&& !Rotating)
		{
			// might be wanting to go ahead of the ped rather than behind
			if(TheCamera.m_fAvoidTheGeometryProbsTimer < 0.0f)
				FixedTargetOrientation=TargetOrientation;
			else
				FixedTargetOrientation=TargetOrientation + PI;// + AngleToGoTo;

			float fTempRangeMag = (Source - TargetCoors).Magnitude();
			if(fTempRangeMag > 0.1f)
				fTempRangeMag = 1.0f / fTempRangeMag;
			else
				fTempRangeMag = 10.0f;

			SpeedStep = fTempRangeMag*fDefaultSpeedStep4Avoid;
			SpeedMultiplier = fTempRangeMag*fDefaultSpeedMultiplier4Avoid;
			SpeedLimit = fTempRangeMag*fDefaultSpeedLimit4Avoid;
		}

		float PedSpeed=0.0f;
		PedSpeed=DotProduct(CamTargetEntity->GetMatrix().GetForward(), ((CPhysical*)CamTargetEntity)->GetSpeed(0,0,0)) ;
		
		int MoveResult=(((CPed*)CamTargetEntity)->GetMoveState());
		
		if( MoveResult!=PEDMOVE_NONE && MoveResult!=PEDMOVE_STILL && !(CPad::GetPad(0)->ForceCameraBehindPlayer() && !CPickups::PlayerOnWeaponPickup)
		&& !ShootingWeapon && !SwimmingOrJetPack)
		{
			Rotating=FALSE;
			// might be rotating to avoid collision
			if(TheCamera.m_fAvoidTheGeometryProbsTimer <= fAvoidGeomThreshhold)
				BetaSpeed=0.0f; 
		}

		RotationDist=m_fMinRealGroundDist;
			
		if(Rotating || CMaths::Abs(TheCamera.m_fAvoidTheGeometryProbsTimer) > fAvoidGeomThreshhold)
		{
			m_bFixingBeta=true;
		
			while (FixedTargetOrientation>=(PI)) {FixedTargetOrientation-=2.0f * PI;}
		 	while (FixedTargetOrientation<(-PI)) {FixedTargetOrientation+=2.0f * PI;}
			while (Beta>=(PI)) {Beta-=2.0f * PI;}
		 	while (Beta<(-PI)) {Beta+=2.0f * PI;}
	
			DeltaBeta = FixedTargetOrientation-Beta; 
			while (DeltaBeta>=(PI)) {DeltaBeta-=2.0f * PI;}
		 	while (DeltaBeta<(-PI)) {DeltaBeta+=2.0f * PI;}
		
			// using m_fAvoidTheGeometryProbsTimer, need to be able to specify the direction of rotation
			if(!Rotating && ((TheCamera.m_nAvoidTheGeometryProbsDirn==-1 && DeltaBeta > 0.0f) || (TheCamera.m_nAvoidTheGeometryProbsDirn==1 && DeltaBeta < 0.0f)) )
				DeltaBeta *= -1.0f;

		   	ReqSpeed = DeltaBeta * SpeedMultiplier;
		   	// limit this so we don't spin too too fast
		   	if(ReqSpeed > SpeedLimit)	ReqSpeed = SpeedLimit;
		   	else if(ReqSpeed < -SpeedLimit)	ReqSpeed = -SpeedLimit;
		   	
			if (ReqSpeed - BetaSpeed > 0.0f)// This can be in a positive or negatve direction, we are going slower than we want to.INCREASE THE SPEED!!!!!
			{
				BetaSpeed += SpeedStep * CMaths::Abs(ReqSpeed - BetaSpeed) * CTimer::GetTimeStep();//// increase the speed
			}
			else // we are going faster than we want. Slow down or we're all going to die. Aiiiieeee. 
			{
				BetaSpeed -= SpeedStep * CMaths::Abs(ReqSpeed - BetaSpeed) * CTimer::GetTimeStep();//decrease the speed 
			}
		
		
			if ((ReqSpeed < 0.0f) && (BetaSpeed < ReqSpeed)) //checks incase Beta speed in now too low
			{
				BetaSpeed = ReqSpeed; ///sets it to the limit
			}
			else if ((ReqSpeed > 0.0f) && (BetaSpeed > ReqSpeed)) //checks incase Beta speed in now too high
			{
				BetaSpeed = ReqSpeed;
			}
		
				Beta += BetaSpeed * MIN(10.0f, CTimer::GetTimeStep() ); // beta is now where we want to be
		
			if (ResetStatics==TRUE)//if we have just gone into this mode then we want to instantly fix 
			{
		    	 Beta=FixedTargetOrientation;
	    		 BetaSpeed=0.0f;
				 Source.x=TargetCoors.x + RotationDist*CMaths::Cos(Beta);
				 Source.y=TargetCoors.y + RotationDist*CMaths::Sin(Beta);
			}
			
			else
			{
				 Source.x=TargetCoors.x + RotationDist*CMaths::Cos(Beta);
				 Source.y=TargetCoors.y + RotationDist*CMaths::Sin(Beta);
			}	
			//work out if we need to stop rotating 
	   		
	   		DeltaBeta = FixedTargetOrientation-Beta; 

			while (DeltaBeta>=(PI)) {DeltaBeta-=2.0f * PI;}
		 	while (DeltaBeta<(-PI)) {DeltaBeta+=2.0f * PI;}
			
	   		
	   		if (CMaths::Abs(DeltaBeta)<DEGTORAD(1.0f))
	   		{
		   		if (bBehindPlayerDesired==false)
		   		{
			   		PickedASide=FALSE;
			   		Rotating=FALSE;
	   			 	BetaSpeed=0.0f;
	   			}
	   		}
	    } 
	
		if(TheCamera.m_bCamDirectlyBehind==true || TheCamera.m_bCamDirectlyInFront==true || HackPlayerOnStoppingTrain
		|| HackPlayerOnStoppingTrain || Rotating || (TheCamera.m_bUseTransitionBeta && ResetStatics)
		|| CMaths::Abs(TheCamera.m_fAvoidTheGeometryProbsTimer) > fAvoidGeomThreshhold)
		{

			if (TheCamera.m_bUseTransitionBeta)
			{
			 	 Beta=m_fTransitionBeta;
				 Source.x=TargetCoors.x + RotationDist*CMaths::Cos(m_fTransitionBeta);
				 Source.y=TargetCoors.y + RotationDist*CMaths::Sin(m_fTransitionBeta);
			}

			if (TheCamera.m_bCamDirectlyBehind==true) //even if not rotating lets wap it behind
			{
				 Beta=TargetOrientation+PI;
				 Source.x=TargetCoors.x + RotationDist*CMaths::Cos(Beta);
				 Source.y=TargetCoors.y + RotationDist*CMaths::Sin(Beta);
			}
			if (TheCamera.m_bCamDirectlyInFront==true) //even if not rotating lets wap it behind
			{
				 Beta=TargetOrientation;
				 Source.x=TargetCoors.x + RotationDist*CMaths::Cos(Beta);
				 Source.y=TargetCoors.y + RotationDist*CMaths::Sin(Beta);
			}
			
			
			if (HackPlayerOnStoppingTrain) //heading is set from when he is getting on train
			//needs to have 180 degrees added to it
			{
				 Beta=TargetOrientation+PI;
				 Source.x=TargetCoors.x + RotationDist*CMaths::Cos(Beta);
				 Source.y=TargetCoors.y + RotationDist*CMaths::Sin(Beta);
				 m_fDimensionOfHighestNearCar=0.0f;///nearest car will be the train, keep the height low
				 m_fCamBufferedHeight=0.0f;
				 m_fCamBufferedHeightSpeed=0.0f;
			}
		}
		else
		{
	 		Source=IdealSource;
	 		BetaSpeed=0.0f;
		}
		
				

		Source.z=IdealSource.z;

		Front=TargetCoors-Source;
		Front.Normalise();
		
		WellBufferMe(TargetGroundDistExtra, &ExtraHorizontalDistToAdd, &ExtraHorizontalDistToAddSpeed, 0.20f, 0.07f, false);
		WellBufferMe(TargetZDistExtra, &ExtraVerticalDistToAdd, &ExtraVerticalDistToAddSpeed, 0.20f, 0.07f, false);

		
		Source.x-= Front.x * ExtraHorizontalDistToAdd;
		Source.y-= Front.y * ExtraHorizontalDistToAdd;
		Source.z+= ExtraVerticalDistToAdd;
				
	
		float TargetZOffSet=0.0f;
		TargetZOffSet=MAX((m_fDimensionOfHighestNearCar), m_fPedBetweenCameraHeightOffset);
		//TargetZOffSet=MAX(TargetZOffSet, m_fDimensionOfHighestNearCar); 
		float FinalTarget=ThisCamsTarget.z+TargetZOffSet-Source.z;

		if (FinalTarget>m_fCamBufferedHeight)
		{
			if ((TargetZOffSet==m_fPedBetweenCameraHeightOffset)&&(TargetZOffSet>m_fCamBufferedHeight))
			{
				WellBufferMe(FinalTarget, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed), 0.20f, 0.04f);
			}
			else
			{
				WellBufferMe(FinalTarget, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed),  0.20f, 0.025f);
			}
			StartedCountingForGoDown=false;
		}
		else
		{
			if (StartedCountingForGoDown==false)	
			{
					TimeIndicatedWantedToGoDown=CTimer::GetTimeInMilliseconds();
					StartedCountingForGoDown=true;
			}
			else
			{
				if ((CTimer::GetTimeInMilliseconds()-TimeIndicatedWantedToGoDown)>TimeBeforeGoDown)
				{
					//get the fuck down 
					//WellBufferMe(0.0f, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed), 0.40f, 0.05f);
					if (FinalTarget>0.0f)
					{
						WellBufferMe(FinalTarget, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed), 0.20f, 0.010f);
					}
					else
					{
						WellBufferMe(0.0f, &(m_fCamBufferedHeight), &(m_fCamBufferedHeightSpeed), 0.20f, 0.010f);
					}
				}
			}
		}
		
		Source.z+=m_fCamBufferedHeight;
	
		//for now we will do this every frame
	
		
		TargetCoors.z += 1.0f * MIN(1.0f, m_fCamBufferedHeight/2.0f);    //1.0f is maximum it can go up

		m_cvecTargetCoorsForFudgeInter=TargetCoors;
		
		CVector SourceBeforeChange=Source;
		TheCamera.AvoidTheGeometry(SourceBeforeChange,TargetCoors, Source, FOV);
		
		// but this one really needs to be calculated now, so thats ok
		float fNewDistance = (TargetCoors - Source).Magnitude();
		if(fNewDistance < Distance)
		{
			Distance = fNewDistance; //fine
		}
		else// if(Distance < fNewDistance)
		{
			// buffered Distance goes towards fNewDistance (not fIdealDistance)
			float fTimeRateOfChange = CMaths::Pow(fAvoidGeomReturnRate, CTimer::GetTimeStep());
			Distance = fTimeRateOfChange*Distance + (1.0f - fTimeRateOfChange)*fNewDistance;
			
			if(fNewDistance > 0.05f) // just in case
				Source = TargetCoors + (Source - TargetCoors)*Distance/fNewDistance;
			
			// now we might have ended up closer to the player than AvoidTheGeometry expected
			// so might clip through playerped
			float PlayCamDistWithCol = Distance-fRangePlayerRadius;
			if (PlayCamDistWithCol<RwCameraGetNearClipPlane(Scene.camera))	
			{
				RwCameraSetNearClipPlane(Scene.camera, MAX(PlayCamDistWithCol, fCloseNearClipLimit));
			}	
		}
		
	/*	if (SourceBeforeChange!=Source)
		{
			if (OnlySphereTestFailed)	
			{
				float TargetDistance=0.0f;
				TargetDistance=(Source-TargetCoors).Magnitude()-(SourceBeforeChange-TargetCoors).Magnitude();
				WellBufferMe(TargetDistance, &Distance, &DistanceSpeed,  0.45f, 0.15f) ; 
				CVector TempFront=SourceBeforeChange-TargetCoors;
				TempFront.Normalise();
				Source=SourceBeforeChange +  Distance*TempFront;			
			}
			else
			{
				Distance=(Source-TargetCoors).Magnitude()-(SourceBeforeChange-TargetCoors).Magnitude();
				DistanceSpeed=0.0f;
			}
		}		
		else
		{
			if (Distance!=0.0f)	
			{
				WellBufferMe(0.0f, &Distance, &DistanceSpeed,  0.45f, 0.15f) ; 
				CVector TempFront=SourceBeforeChange-TargetCoors;
				TempFront.Normalise();
				Source=SourceBeforeChange +  Distance*TempFront;	
			}
		}
	*/

		
		Front=TargetCoors-Source;
		m_fRealGroundDist=Front.Magnitude2D();
		m_fMinDistAwayFromCamWhenInterPolating=m_fRealGroundDist;	
		Front.Normalise();
		GetVectorsReadyForRW();
		TheCamera.m_bCamDirectlyBehind=false;
		TheCamera.m_bCamDirectlyInFront=false;

		ResetStatics=FALSE;
	
	}
#endif	
}	


///////////////////////////////////////////////////////////////////////////
// NAME       : WorkOutCamHeight
// PURPOSE    : If the Camera is within a certain range and the car is accelerating forward
//			  : then we have to rotate
// RETURNS    : True if had to fix false is it didin't
// PARAMETERS : The TargetCoors of the car 
///////////////////////////////////////////////////////////////////////////
//float fSpeedJumpTiltUp = 2.0f;
//float fSpeedJumpTiltDown = 0.7f;
//
void CCam::WorkOutCamHeight(const CVector &TargetCoors, float TargetOrientation, float VehicleHeight)
{
	ASSERT(false);
#if 0
	// as Mark would say - this is a cheap fix
	if(!CamTargetEntity->GetIsTypeVehicle())
		return;

	//now lets work out how high the camera should be 
	//work out the final distance 
	CVector CarAlphaVec, FinalDistance;
	float CarAlphaAngle=0.0f, FowardLength=0.0f, DeltaAlpha=0.0f, InFrontOfCar=0.0f;
	float FinalDistMag=0.0f, TopAlphaSpeed=0.0f, AlphaSpeedStep=0.0f;	
	float AcceptTurnAroundXY=0.0f;f
	float TargetAlpha=Alpha;
	bool DegreesAboveNotObscured=TRUE;
	bool AboveNotObscured=TRUE;	  	
	bool PreviouslyFailedLOSTest;
	float LocalCarZoomValueSmooth=0.0f;
	float ReqSpeed=0.0f;
	static float LastTargetAlphaWithCollisionOn=0.0f;
	static float LastTopAlphaSpeed=0.0f;
	static float LastAlphaSpeedStep=0.0f;
	static bool PreviousNearCheckNearClipSmall=false;
	static bool JustCameOutTunnel=false;
	static bool PreviouslyInTunnel=false;
	static float AlphaHeliSway=0.0f;
	static float AlphaHeliSwaySpeed=0.0f, AlphaOffSet=0.0f, AlphaOffSetSpeed=0.0f;
	static float RoadHeightFix=0.0f;
	static float RoadHeightFixSpeed=0.0f;

	const float MinimumDistAboveGround=1.25f;
	const float MinimumDistBelowRoof=1.25f;
	const float DistanceToBelow=5.0f;
	const float DistanceToCheckAbove=5.0f;
	float TestFinalDistanceMag=0.0f;
	float OrientationLooseness=0.0f;//DEGTORAD(5.0f);
	float ExtraOffsetDueToVehicleType=0.0f;
	bool VehicleIsABike=false;
	bool VehicleIsAHeli=false;
	bool VehicleIsACar=false;
	
	eVehicleAppearance VehicleApperance=((CVehicle*)(CamTargetEntity))->GetVehicleAppearance(); 
	if(VehicleApperance==APR_BIKE)
		VehicleIsABike=true;
	if(VehicleApperance==APR_HELI)
		VehicleIsAHeli=true;
	if(VehicleApperance==APR_CAR)
		VehicleIsACar=true;
	
	int PositionInArray=0;
	TheCamera.GetArrPosForVehicleType(VehicleApperance, PositionInArray);
	
	UInt32 VehicleModIndex=0;
 	float RcAlphaExtra=0.0f;
 	VehicleModIndex=CamTargetEntity->GetModelIndex();
 	if(VehicleModIndex==MODELID_CAR_RCCOPTER || VehicleModIndex==MODELID_CAR_RCGOBLIN)
	{
		RcAlphaExtra=INIT_RC_HELI_ALPHA_EXTRA;
	}
	else if (VehicleModIndex==MODELID_CAR_RCBARON)
	{
		RcAlphaExtra=INIT_RC_PLANE_ALPHA_EXTRA;	
	}
	
	
    if (ResetStatics)
	{
		LastTargetAlphaWithCollisionOn=0.0f;
		LastTopAlphaSpeed=0.0f;
		LastAlphaSpeedStep=0.0f;
		PreviousNearCheckNearClipSmall=false;
	  	PreviouslyFailedLOSTest=false;
		AlphaHeliSway=0.0f;
		AlphaHeliSwaySpeed=0.0f;
		AlphaOffSet=0.0f;
		AlphaOffSetSpeed=0.0f;
		JustCameOutTunnel=false;
		PreviouslyInTunnel=false;
		RoadHeightFix=0.0f;
		RoadHeightFixSpeed=0.0f;

		
		if (TheCamera.m_nCarZoom==ZOOM_ONE)
		{
			AlphaOffSet=ZmOneAlphaOffset[PositionInArray] + RcAlphaExtra;	
		}
		else 
		{
			if (TheCamera.m_nCarZoom==ZOOM_TWO)
			{
				AlphaOffSet=ZmTwoAlphaOffset[PositionInArray] + RcAlphaExtra;
			}
			else 
			{
				if (TheCamera.m_nCarZoom==ZOOM_THREE) // furthest out
				{
					AlphaOffSet=ZmThreeAlphaOffset[PositionInArray] + RcAlphaExtra;
				}
			}
		}
	}

	AcceptTurnAroundXY=0.02f;

	TopAlphaSpeed=0.15f;
	AlphaSpeedStep=0.015f;

	LocalCarZoomValueSmooth=TheCamera.m_fCarZoomSmoothed;
	if (LocalCarZoomValueSmooth<0.1f)
	{LocalCarZoomValueSmooth=0.1f;}


	if (TheCamera.m_nCarZoom==ZOOM_ONE)
	{
			WellBufferMe( ZmOneAlphaOffset[PositionInArray] + RcAlphaExtra, &AlphaOffSet, &AlphaOffSetSpeed, 0.17f, 0.08f, false);	
	}
	else 
	{
		if (TheCamera.m_nCarZoom==ZOOM_TWO)
		{
			WellBufferMe(ZmTwoAlphaOffset[PositionInArray] + RcAlphaExtra, &AlphaOffSet, &AlphaOffSetSpeed, 0.17f, 0.08f, false);	
		}
		else 
		{
			if (TheCamera.m_nCarZoom==ZOOM_THREE) // furthest out
			{
				WellBufferMe(ZmThreeAlphaOffset[PositionInArray] + RcAlphaExtra, &AlphaOffSet,  &AlphaOffSetSpeed, 0.17f, 0.08f, false);	
			}
		}
	}



	//if there is a helicopter then at some extra on if the player is using the shoulder buttons 
	//to tilt the helicopters roll
/*	if ((CamTargetEntity->GetModelIndex()==MODELID_CAR_TESTCHOPPER)||(CamTargetEntity->GetModelIndex()==MODELID_CAR_COPTER_HUNTER))
	{
			// if the camera is directly behind it we want
			//it to tilt maximally  
			float GoUpOrDown=DotProduct(Front, ((CPhysical*)CamTargetEntity)->GetMatrix().GetRight());

			CVector HeliUp=((CPhysical*)CamTargetEntity)->GetMatrix().GetRight();
			float GroundLength=CMaths::Sqrt(HeliUp.x * HeliUp.x + HeliUp.y* HeliUp.y);
			float TargetAlphaHeliSway=CGeneral::GetATanOfXY(GroundLength, HeliUp.z);
			TargetAlphaHeliSway=CGeneral::LimitRadianAngle(TargetAlphaHeliSway);
			TargetAlphaHeliSway=TargetAlphaHeliSway*(-GoUpOrDown);
			//buffer a bit so it feels like a real camera follwing in a heli
			WellBufferMe(TargetAlphaHeliSway, &(AlphaHeliSway), &(AlphaHeliSwaySpeed), 0.1f, 0.013f, true);			
			//add this in to Alpha Offset (it gets taken off again at the end of the function 'cause Alpha Offset now
			//a static
			AlphaOffSet+=AlphaHeliSway;
	}
*/

	FinalDistance=Source- TargetCoors ;
	FinalDistMag=FinalDistance.Magnitude2D();		


	
	ASSERTMSG(CamTargetEntity->GetIsTypeVehicle(), "DUM");

	CarAlphaVec=CamTargetEntity->GetMatrix().GetForward();

	//char Str[250];
	FowardLength=CMaths::Sqrt(CarAlphaVec.x*CarAlphaVec.x + CarAlphaVec.y*CarAlphaVec.y);
	//was previously forward length of 1
	CarAlphaAngle=CGeneral::GetATanOfXY(FowardLength,  CarAlphaVec.z);

	while (  CarAlphaAngle > PI)   CarAlphaAngle -= (2.0f*PI);
	while (  CarAlphaAngle < -PI)   CarAlphaAngle += (2.0f*PI);

	while (  Beta > PI)   Beta -= (2.0f*PI);
	while (  Beta < -PI)   Beta += (2.0f*PI);


	float CamToCarAngle=Beta-TargetOrientation;

	while (  CamToCarAngle > PI)   CamToCarAngle -= (2.0f*PI);
	while (  CamToCarAngle < -PI)   CamToCarAngle += (2.0f*PI);


	float PercentageCamBehindCar=(CMaths::Cos(CamToCarAngle)); 

	if (PercentageCamBehindCar>0) //CameraBehindCar=true;
	{

		CarAlphaAngle=(-CarAlphaAngle * CMaths::Abs(PercentageCamBehindCar));
	}
	else //camera in frotn of car
	{
		CarAlphaAngle=(CarAlphaAngle * CMaths::Abs(PercentageCamBehindCar));
	}
	
	
	//if the bike is in the air then we want the camera to go up high so we can see where to land
	float VehicleSpeed=DotProduct(((CVehicle*)CamTargetEntity)->m_vecMoveSpeed, ((CVehicle*)CamTargetEntity)->GetMatrix().GetForward())/GAME_VELOCITY_CONST;

	if(CamTargetEntity->GetModelIndex()==MODELID_CAR_FIRETRUCK && CPad::GetPad(0)->GetCarGunFired())
	{
		CarAlphaAngle=DEGTORAD(10.0f);
	}
	else if(false)//VehicleIsABike || VehicleIsACar)
	{
/*
		if( (VehicleIsABike && ((CBike *)CamTargetEntity)->nNoOfContactWheels==0)
		||  (VehicleIsACar && ((CAutomobile *)CamTargetEntity)->nNoOfContactWheels==0) )
		{
			// first find the ground below the default camera position
			bool GroundFound=false;
			CVector vecTestDefaultPos = Source;
			float GroundZ=CWorld::FindGroundZFor3DCoord(vecTestDefaultPos.x, vecTestDefaultPos.y, CamTargetEntity->GetPosition().z + 10.0f, &GroundFound);
			
			// the calc desired jumping camera angle based on velocity
			float fTempVertVel = ((CVehicle *)CamTargetEntity)->m_vecMoveSpeed.z;
			if(fTempVertVel < 0.0f)	fTempVertVel *= fSpeedJumpTiltDown;
			else	fTempVertVel *= fSpeedJumpTiltUp;
			
			CarAlphaAngle = CMaths::ATan2(-fTempVertVel, ((CVehicle *)CamTargetEntity)->m_vecMoveSpeed.Magnitude2D());
			
			// work out the height of the camera with this angle
			float fTestJumpCameraZ = TargetCoors.z + (FinalDistMag*CMaths::Sin(CarAlphaAngle + AlphaOffSet)) + m_fCloseInCarHeightOffset;
			if(GroundZ + 0.9f > fTestJumpCameraZ)
			{			
				float fTempForASin = MIN(1.0f, MAX(-1.0f, (GroundZ + 0.9f - TargetCoors.z - m_fCloseInCarHeightOffset)/FinalDistMag));
				CarAlphaAngle = CMaths::ASin(fTempForASin) - AlphaOffSet;
			}
			
			// CarAlphaAngle is going to get clipped to 0.0 so need to modify AlphaOffset as well
			if(CarAlphaAngle < 0.0f) AlphaOffSet += CarAlphaAngle;
		}
*/
		CVector VehPos;
		VehPos=CamTargetEntity->GetPosition();
		bool GroundFound=false;
		bool DoingAWheelie=false;
		float GroundZ=CWorld::FindGroundZFor3DCoord(VehPos.x, VehPos.y, VehPos.z, &GroundFound);
		
		if ((VehicleIsABike)&&(CWorld::Players[CWorld::PlayerInFocus].nBikeRearWheelCounter>0))
		{
			DoingAWheelie=true;
		}
		
		if ((GroundFound)||(DoingAWheelie))
		{
			if((VehPos.z-GroundZ)>1.45f)
			{
				float VehicleSpeedFactor=ABS(VehicleSpeed/180.0f);
				if (VehicleSpeedFactor>1.0f){VehicleSpeedFactor=1.0f;}
				float TargetCarAlphaAngle=DEGTORAD(30.0f) * VehicleSpeedFactor;
				if (TargetCarAlphaAngle>CarAlphaAngle)
				{
					CarAlphaAngle=TargetCarAlphaAngle;
				}
			}
			else
			{
				if (DoingAWheelie)
				{
					if (CarAlphaAngle<DEGTORAD(10.0f))
					{
						CarAlphaAngle=DEGTORAD(10.0f);
					}
				}
			}
		}

	}
	else if(VehicleIsAHeli)
	{
		CarAlphaAngle=0.0f;
		CVector HeliFront = CamTargetEntity->GetMatrix().GetForward();
		float HeliGroundDist=0.0f;
		float AmountToGoUpOrDown=0.0f;

		AmountToGoUpOrDown=ABS(VehicleSpeed/90.0f);
		if (AmountToGoUpOrDown>1.0f){AmountToGoUpOrDown=1.0f;}
		
		HeliGroundDist=HeliFront.Magnitude2D();	 
		if (!((HeliGroundDist==0.0f)&&(HeliFront.z==0)))
		{
			CarAlphaAngle=CGeneral::GetATanOfXY(HeliGroundDist, ABS(HeliFront.z)) * AmountToGoUpOrDown;
		}
		
		// gonna try line check from height, down to heli's coords and try and move CarAlphaAngle down to avoid roof
		CColPoint tempCol;
		CEntity *pTempHitEnt = NULL;
		CVector vecTestDefaultPos = Source;
		vecTestDefaultPos.z = 0.2f + TargetCoors.z + (FinalDistMag*CMaths::Sin(CarAlphaAngle + AlphaOffSet)) + m_fCloseInCarHeightOffset;
		if(CWorld::ProcessVerticalLine(vecTestDefaultPos, CamTargetEntity->GetPosition().z, tempCol, pTempHitEnt, true, false, false, false, false, false, NULL))
		{
			float fTempForASin = MIN(1.0f, MAX(-1.0f, (tempCol.GetPosition().z - 0.2f - TargetCoors.z - m_fCloseInCarHeightOffset)/FinalDistMag));
			CarAlphaAngle = CMaths::ASin(fTempForASin) - AlphaOffSet;
			// CarAlphaAngle is going to get clipped to 0.0 so need to modify AlphaOffset as well
			if(CarAlphaAngle < 0.0f) AlphaOffSet += CarAlphaAngle;
		}
	}
	
	CarAlphaAngle=CGeneral::LimitRadianAngle(CarAlphaAngle);
		
	if (CarAlphaAngle<0.0f)
	{
		//limit it
		CarAlphaAngle=0.0f;
	}
	
	if (CarAlphaAngle>DEGTORAD(89.0f))
	{
		CarAlphaAngle=DEGTORAD(89.0f);
	}
	//

	///want to move the height of the camera according to how the car moves
	//This has a slackness of +- 1.8DEG  
	if (ResetStatics)
	{
		Alpha=CarAlphaAngle;
	}
		

	if (TargetAlpha<-0.01){TargetAlpha=-0.01f;}//clipping of alpha
	DeltaAlpha=CarAlphaAngle - Alpha;						
	while (  DeltaAlpha > PI)    DeltaAlpha -= (2.0f*PI);
	while (  DeltaAlpha < -PI)   DeltaAlpha += (2.0f*PI);

	TargetAlpha=Alpha;
	
	if ((ABS(DeltaAlpha))>OrientationLooseness)
	{
		if (TheCamera.m_bVehicleSuspenHigh==false)
		{
			TargetAlpha=CarAlphaAngle;
		}
	}


	if (VehicleIsABike)
	{
		WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.09f, 0.04f, true);
	}
	else if (VehicleIsAHeli)
	{
		WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.09f, 0.04f, true);	
	}
	else
	{
		WellBufferMe(TargetAlpha, &Alpha, &AlphaSpeed, 0.15f, 0.07f, true);
	}
	Source.z= TargetCoors.z + (FinalDistMag*CMaths::Sin(Alpha + AlphaOffSet)) + m_fCloseInCarHeightOffset;


	//Old Collision Stuff
/*
	CVector CarPosition=CamTargetEntity->GetPosition();
	CVector LineStart=Source;
	LineStart.z-=10.0f;
	float zEnd=Source.z+7.0f;
	CEntity *pEntity=NULL;
	float ZDiff=0.0f;
	float TargetZRoadHeightFix=0.0f;
	
	
	CColPoint ColPoint;

	//let's check above the car	
	bool RoofFound=false;
	float RoofHeight=0.0f;
	RoofHeight=CWorld::FindRoofZFor3DCoord(CarPosition.x, CarPosition.y, CarPosition.z, &RoofFound);
	float TargetZRoadHeightFixForCar=0.0f;	
	CVector TestSource=Source;

	if (RoofFound)
	{
		if ((RoofHeight-MinimumDistBelowRoof)<Source.z)
		{
			TargetZRoadHeightFixForCar=(RoofHeight - MinimumDistBelowRoof)-Source.z;
		}
		TestSource.z+=TargetZRoadHeightFixForCar;

	}

	bool CameraIsObscured=false;
	if (CWorld::ProcessLineOfSight(CarPosition, TestSource, ColPoint, pEntity, true, false, false, false, false, false, false))
	{
		CameraIsObscured=true;				
	}

	
	float HighestClearZ=0.0f;
	
	if (CollisionPointsVerticalLineCheck(LineStart, zEnd, CarPosition, MinimumDistBelowRoof, MinimumDistAboveGround, HighestClearZ, CameraIsObscured, TestSource))
	{
		TargetZRoadHeightFix=HighestClearZ-Source.z;	
	}
	else
	{
		if (RoofFound)
		{
			TargetZRoadHeightFix=TargetZRoadHeightFixForCar;
		}
	}	
	
	WellBufferMe(TargetZRoadHeightFix, &RoadHeightFix, &RoadHeightFixSpeed, 0.20f, 0.07f, false);
	Source.z+=RoadHeightFix;
*/
	///finished checking the height stuff
	//need to take this off added abouve
	AlphaOffSet-=AlphaHeliSway;
	//FixIfBelowGround(&Source); //final check if cam is below the floor lets raise it
#endif	
}


//#define OPTIMISED_CAMPED_AVOID
//static float fPedGonnaBeInRangeTime = 1.5f*FRAMES_PER_SECOND;
//
void CCam::ProcessSpecialHeightRoutines(void)
{
#if 0
	//need to do a a check if there is a ped that is standing in the near clip
	//plane in front of the camera.
	//N.B. If there is a cut scene playing then we don't need to do these checks 
	//As the cutscene animator has either thought of it or someone has moaned about it. 
	float CameraFrontDir=0.0f;
	int ClosestPedAliveNumber=0;
	int ClosePedNumber=0;
	bool FoundCloseAlivePed=false;		
	bool ClipOnBoat=false;
	static bool PreviouslyFailedRoadHeightCheck=false;
	CVector PlayerToCamVect=(TheCamera.pTargetEntity->GetPosition()-(*TheCamera.GetGameCamPosition()));
	float PlayerToCamDist=CMaths::Sqrt(PlayerToCamVect.x * PlayerToCamVect.x + PlayerToCamVect.y * PlayerToCamVect.y);
	float CameraDirection=CGeneral::GetATanOfXY(PlayerToCamVect.x, PlayerToCamVect.y);
	m_bTheHeightFixerVehicleIsATrain=false;


	float SmalledDistanceSoFar=0.0f;
	float RangeToCheckForPeds=2.1f;
	CameraFrontDir=CGeneral::GetATanOfXY(TheCamera.GetMatrix().xy, TheCamera.GetMatrix().yy);
	CPed* PlayerPed=CWorld::Players[CWorld::PlayerInFocus].pPed;
	m_bTheHeightFixerVehicleIsATrain=false;

	if (PlayerToCamDist>10.0f)//in case player has been warped and camera has not been restored with it
	{
		PlayerToCamDist=10.0f;
	}

	ASSERTMSG(PlayerPed!=NULL, "NICE ONE MARK YA DICK");
		///do the thing for the closest ped
	
	if ((CamTargetEntity)!=NULL)
	{
		if (((CPed*)CamTargetEntity)->GetIsTypePed())	
		{
			if (FindPlayerPed()->m_pEntityStandingOn!=NULL)
			{
				if (FindPlayerPed()->m_pEntityStandingOn->GetIsTypeVehicle())
				{	
					if ((((CVehicle*)(FindPlayerPed()->m_pEntityStandingOn))->GetBaseVehicleType())==VEHICLE_TYPE_BOAT)
					{
						ClipOnBoat=true;
					}
				}	
			}
			
#ifdef OPTIMISED_CAMPED_AVOID
			float fFoundPedPosZ = -100.0f;
			float fPedRangeSqr = -1.0f;
			CVector vecUnAlteredCamPos = *TheCamera.GetGameCamPosition();
			vecUnAlteredCamPos.z -= m_fCamBufferedHeight;//MAX(m_fPedBetweenCameraHeightOffset, m_fDimensionOfHighestNearCar);
			
			if ((Mode==CCam::MODE_FOLLOWPED)||(Mode==CCam::MODE_FIGHT_CAM)||(Mode==MODE_PILLOWS_PAPS))
			{			
				//Want to go through the close ped, store the one that is the closest to the camera
				//and in front of the cam 
				int i;
				const int N=PlayerPed->GetPedIntelligence()->GetMaxNumPedsInRange();
				CEntity** ppNearbyEntities=PlayerPed->GetPedIntelligence()->GetNearbyPeds();
				for(i=0;i<N;i++)
				{
					CEntity* pNearbyEntity=ppNearbyEntities[i];
					if(pNearbyEntity)
					{
						ASSERT(pNearbyEntity->GetType()==ENTITY_TYPE_PED);
						CPed* pNearbyPed=(CPed*)pNearbyEntity;
						if(pNearbyPed->IsAlive())
						{
							CVector CamToPedVect = pNearbyPed->GetPosition() - vecUnAlteredCamPos;
							
							if (ABS(CamToPedVect.z)<1.0f)
							{
								if((fPedRangeSqr=CamToPedVect.MagnitudeSqr()) < RangeToCheckForPeds*RangeToCheckForPeds)
								{
									if(pNearbyPed->GetPosition().z > fFoundPedPosZ)
										fFoundPedPosZ = pNearbyPed->GetPosition().z;
								}
								else
								{
									fPedRangeSqr = CMaths::Sqrt(fPedRangeSqr);
									CamToPedVect /= fPedRangeSqr;
									
									float fCamVel = DotProduct(Front, PlayerPed->m_vecMoveSpeed);
									float fClosingVel = DotProduct(fCamVel*Front - pNearbyPed->m_vecMoveSpeed, CamToPedVect);
									
									if( fClosingVel > 0.01f && ((m_fPedBetweenCameraHeightOffset>0.0f && (fPedRangeSqr - RangeToCheckForPeds)/fClosingVel < fPedGonnaBeInRangeTime)
									|| (m_fPedBetweenCameraHeightOffset<=0.0f && (fPedRangeSqr - RangeToCheckForPeds)/fClosingVel < 0.1f*fPedGonnaBeInRangeTime)) )
									{
										if(pNearbyPed->GetPosition().z > fFoundPedPosZ)
											fFoundPedPosZ = pNearbyPed->GetPosition().z;
									}
								}
							}
						}
						
					}
				}
			
				/*
				//Want to go through the close ped, store the one that is the closest to thecamera
				//and in front of the cam 
				for(ClosePedNumber=0; ClosePedNumber < PlayerPed->m_NumClosePeds; ClosePedNumber++)
				{
					if ((PlayerPed->m_apClosePeds[ClosePedNumber]!=NULL)&&(PlayerPed->m_apClosePeds[ClosePedNumber]->GetPedState()!=PED_DEAD))
					{
						CVector CamToPedVect = PlayerPed->m_apClosePeds[ClosePedNumber]->GetPosition() - vecUnAlteredCamPos;
						
						if (ABS(CamToPedVect.z)<1.0f)
						{
							if((fPedRangeSqr=CamToPedVect.MagnitudeSqr()) < RangeToCheckForPeds*RangeToCheckForPeds)
							{
								if(PlayerPed->m_apClosePeds[ClosePedNumber]->GetPosition().z > fFoundPedPosZ)
									fFoundPedPosZ = PlayerPed->m_apClosePeds[ClosePedNumber]->GetPosition().z;
							}
							else
							{
								fPedRangeSqr = CMaths::Sqrt(fPedRangeSqr);
								CamToPedVect /= fPedRangeSqr;
								
								float fCamVel = DotProduct(Front, PlayerPed->m_vecMoveSpeed);
								float fClosingVel = DotProduct(fCamVel*Front - PlayerPed->m_apClosePeds[ClosePedNumber]->m_vecMoveSpeed, CamToPedVect);
								
								if( fClosingVel > 0.01f && ((m_fPedBetweenCameraHeightOffset>0.0f && (fPedRangeSqr - RangeToCheckForPeds)/fClosingVel < fPedGonnaBeInRangeTime)
								|| (m_fPedBetweenCameraHeightOffset<=0.0f && (fPedRangeSqr - RangeToCheckForPeds)/fClosingVel < 0.1f*fPedGonnaBeInRangeTime)) )
								{
									if(PlayerPed->m_apClosePeds[ClosePedNumber]->GetPosition().z > fFoundPedPosZ)
										fFoundPedPosZ = PlayerPed->m_apClosePeds[ClosePedNumber]->GetPosition().z;
								}
							}
						}
					}
				}
				*/
				
				if(fFoundPedPosZ > -99.0f)
				{
					float ZoomHeightExtra=0.0f;
					float PlayerClosePedHeightDiff=0.0f;
					if(fFoundPedPosZ > PlayerPed->GetPosition().z)
						PlayerClosePedHeightDiff = fFoundPedPosZ - PlayerPed->GetPosition().z;

					if (Mode==CCam::MODE_FOLLOWPED)
					{
						if (TheCamera.m_nPedZoom==ZOOM_ONE
						&& PlayerPed->GetPedState()!=PED_ENTER_CAR && PlayerPed->GetPedState()!=PED_CARJACK)
						{
							ZoomHeightExtra=0.45f + PlayerClosePedHeightDiff;
						}
						if (TheCamera.m_nPedZoom==ZOOM_TWO || TheCamera.m_nPedZoom==ZOOM_ONE)
						{
							ZoomHeightExtra=0.35f + PlayerClosePedHeightDiff;
						}
						if (TheCamera.m_nPedZoom==ZOOM_THREE)
						{
							ZoomHeightExtra=0.25f + PlayerClosePedHeightDiff;
						}
							
						m_fPedBetweenCameraHeightOffset=PED_IN_WAY_HEIGHT_OFFSET_FOR_FOLLOW_PED + ZoomHeightExtra;
					}
/*	DWREMOVED		else if (Mode==CCam::MODE_FIGHT_CAM)
					{
						ZoomHeightExtra=0.5f+PlayerClosePedHeightDiff;
						m_fPedBetweenCameraHeightOffset=PED_IN_WAY_HEIGHT_OFFSET_FOR_FOLLOW_PED + ZoomHeightExtra;
					}*/
					else if (Mode==CCam::MODE_PILLOWS_PAPS)
					{
						ZoomHeightExtra=0.45f + PlayerClosePedHeightDiff;
						m_fPedBetweenCameraHeightOffset=PED_IN_WAY_HEIGHT_OFFSET_FOR_FOLLOW_PED + ZoomHeightExtra;
					}
				
				}
				else  //bring it down 
				{
					m_fPedBetweenCameraHeightOffset=0.0f;
				}
			}
#else			
			if ((Mode==CCam::MODE_FOLLOWPED)||(Mode==CCam::MODE_FIGHT_CAM)||(Mode==MODE_PILLOWS_PAPS))
			{
				//Want to go through the close ped, store the one that is the closest to thecamera
				//and in front of the cam 
				while ((ClosePedNumber<(PlayerPed->m_NumClosePeds)))
				{
					if ((PlayerPed->m_apClosePeds[ClosePedNumber]!=NULL)&&(PlayerPed->m_apClosePeds[ClosePedNumber]->GetPedState()!=PED_DEAD))
					{
						CVector PedToCamVect=(PlayerPed->m_apClosePeds[ClosePedNumber]->GetPosition())-(*TheCamera.GetGameCamPosition()); 
						if (ABS(PedToCamVect.z)<1.0f)
						{
							PedToCamVect.z=0.0f;
							if (FoundCloseAlivePed==false) //this is the first one we have found
							{
								FoundCloseAlivePed=true;
								ClosestPedAliveNumber=ClosePedNumber;
								SmalledDistanceSoFar=PedToCamVect.Magnitude();
							}
							else
							{
								if (PedToCamVect.Magnitude()<SmalledDistanceSoFar)
								{
									ClosestPedAliveNumber=ClosePedNumber;
									SmalledDistanceSoFar=PedToCamVect.Magnitude();
								}
							}
						}
						else
						{
							float fCamVel = DotProduct(Front, PlayerPed->m_vecMoveSpeed);
							vecClosingVel = PedToCamVect*( DotProduct(fCamVel*Front, PedToCamVect) + DotProduct(PlayerPed->m_apClosePeds[ClosePedNumber]->m_vecMoveSpeed, PedToCamVect));
							
							
						}
					}
					ClosePedNumber++;

				}
				
				if (FoundCloseAlivePed)
				{
					CVector ClosestPedToPlayerCamVect=(PlayerPed->m_apClosePeds[ClosestPedAliveNumber]->GetPosition())-(*TheCamera.GetGameCamPosition()); 
					float DistanceNearPedToCam=CMaths::Sqrt(ClosestPedToPlayerCamVect.x * ClosestPedToPlayerCamVect.x + ClosestPedToPlayerCamVect.y * ClosestPedToPlayerCamVect.y); 
					if (DistanceNearPedToCam<(RangeToCheckForPeds))
					{
						float ZoomHeightExtra=0.0f;
						float PlayerClosePedHeightDiff=0.0f;
						
						if (((PlayerPed->m_apClosePeds[ClosestPedAliveNumber]->GetPedState()!=PED_FALL)||(PlayerPed->m_apClosePeds[ClosestPedAliveNumber]->GetPedState()!=PED_DIE))&&(PlayerPed->m_apClosePeds[ClosestPedAliveNumber]->GetIsStanding()))
						PlayerClosePedHeightDiff=PlayerPed->m_apClosePeds[ClosestPedAliveNumber]->GetPosition().z-PlayerPed->GetPosition().z;
						//clip
						if (PlayerClosePedHeightDiff>1.2f)
						{
							PlayerClosePedHeightDiff=0.0f;
						}
						if (PlayerClosePedHeightDiff<-1.2f)
						{
							PlayerClosePedHeightDiff=0.0f;
						}
						
						//ASSERTMSG(PlayerToCamDist!=0.0f, "Dividing by zero special height routines");
						
						float AmountToGoUpFraction=(RangeToCheckForPeds-DistanceNearPedToCam)/RangeToCheckForPeds;
						
						ASSERTMSG(AmountToGoUpFraction>=0.0f && AmountToGoUpFraction<=1.0f, "Muck up in special  height routines");
						if (Mode==CCam::MODE_FOLLOWPED)
						{
							if (TheCamera.m_nPedZoom==ZOOM_ONE
							&& PlayerPed->GetPedState()!=PED_ENTER_CAR && PlayerPed->GetPedState()!=PED_CARJACK)
							{
								ZoomHeightExtra=0.45f*AmountToGoUpFraction + PlayerClosePedHeightDiff;
							}
							if (TheCamera.m_nPedZoom==ZOOM_TWO || TheCamera.m_nPedZoom==ZOOM_ONE)
							{
								ZoomHeightExtra=0.35f*AmountToGoUpFraction + PlayerClosePedHeightDiff;
							}
							if (TheCamera.m_nPedZoom==ZOOM_THREE)
							{
								ZoomHeightExtra=0.25f*AmountToGoUpFraction + PlayerClosePedHeightDiff;
							}
							//if the guy is behind us we need to go up quickly 
							//have already check not inf fornt of player now do behind
							if (ABS(CGeneral::GetRadianAngleBetweenPoints(ClosestPedToPlayerCamVect.x, ClosestPedToPlayerCamVect.y, PlayerToCamVect.x, PlayerToCamVect.y))>DEGTORAD(90.0f))
							{
								ZoomHeightExtra+=0.3f; 
							}
								
							m_fPedBetweenCameraHeightOffset=PED_IN_WAY_HEIGHT_OFFSET_FOR_FOLLOW_PED + ZoomHeightExtra;
						}
/*	DWREMOVED			else if (Mode==CCam::MODE_FIGHT_CAM)
						{
							ZoomHeightExtra=0.5f+PlayerClosePedHeightDiff;
							m_fPedBetweenCameraHeightOffset=PED_IN_WAY_HEIGHT_OFFSET_FOR_FOLLOW_PED + ZoomHeightExtra;
						}*/
						else if (Mode==CCam::MODE_PILLOWS_PAPS)
						{
							ZoomHeightExtra=0.45f*AmountToGoUpFraction + PlayerClosePedHeightDiff;
							m_fPedBetweenCameraHeightOffset=PED_IN_WAY_HEIGHT_OFFSET_FOR_FOLLOW_PED + ZoomHeightExtra;
						}
					}
					else  //bring it down 
					{
						m_fPedBetweenCameraHeightOffset=0.0f;
					}
				}
				else  //bring it down 
				{
					m_fPedBetweenCameraHeightOffset=0.0f;
				}
			}	
#endif		
			
	
			/////Now do the thing for avoiding cars
			if (m_bCollisionChecksOn)
			{
				if ((Mode==CCam::MODE_FOLLOWPED))//||(Mode==CCam::MODE_FIGHT_CAM))
				{
					float DistanceToCheckDirectlyBehind=PlayerToCamDist+1.25f;
					float DistanceToCheckLaterally=PlayerToCamDist+0.30f;
					float HighestZSoFar=0.0f;
					float CarHeight=0.0f;
					float OffSetToCheckLaterally=DEGTORAD(28.0f);
					bool ACarHasBeenFound=false;
					CColPoint colPoint;
					CEntity *pHitEntity=NULL;
					CVector TemporarySourceForCars; 
					CVector VehDimension;
					CVector PlayerTargetCoors;
					
					// if we're alreay avoiding a vehice, increase range slightly to aviod bobing effect
					if(m_fDimensionOfHighestNearCar > 0.0f)
						DistanceToCheckDirectlyBehind += 0.3f;
					
					PlayerTargetCoors=((CPed*)CamTargetEntity)->GetPosition();
					PlayerTargetCoors.z-=0.15f;
					TemporarySourceForCars=PlayerTargetCoors -DistanceToCheckDirectlyBehind*(CVector(CMaths::Cos(CameraDirection), CMaths::Sin(CameraDirection), 0));
		//			CDebug::DebugLine3D( PlayerPed->GetPosition().x, PlayerPed->GetPosition().y, PlayerPed->GetPosition().z ,
		//			TemporarySourceForCars.x, TemporarySourceForCars.y, TemporarySourceForCars.z ,
		//			0xffffffff, 0xffffffff);
					if (CWorld::ProcessLineOfSight(((CPed*)CamTargetEntity)->GetPosition()  , TemporarySourceForCars, colPoint, pHitEntity, false, true, false, false, false, false, false))
					{
						if (((pHitEntity)->GetIsTypeVehicle()))
						{
							VehDimension = pHitEntity->GetColModel().GetBoundBoxMax();
							VehDimension -= pHitEntity->GetColModel().GetBoundBoxMin();

							if (ACarHasBeenFound)
							{
								HighestZSoFar=MAX(HighestZSoFar, VehDimension.z);
							}
							else
							{
								ACarHasBeenFound=true;
								HighestZSoFar=VehDimension.z;
							}
							if (((CVehicle*)pHitEntity)->GetBaseVehicleType()==VEHICLE_TYPE_TRAIN)
							{
								m_bTheHeightFixerVehicleIsATrain=true;
							}
						}
					}
						
					TemporarySourceForCars= PlayerTargetCoors - DistanceToCheckDirectlyBehind*(CVector(CMaths::Cos(CameraDirection + OffSetToCheckLaterally), CMaths::Sin(CameraDirection + OffSetToCheckLaterally), 0));
		//			CDebug::DebugLine3D( PlayerPed->GetPosition().x, PlayerPed->GetPosition().y, PlayerPed->GetPosition().z ,
		//			TemporarySourceForCars.x, TemporarySourceForCars.y, TemporarySourceForCars.z ,
		//			0xffffffff, 0xffffffff);
					if (CWorld::ProcessLineOfSight(((CPed*)CamTargetEntity)->GetPosition() , TemporarySourceForCars, colPoint, pHitEntity,  false, true, false, false, false, false, false))
					{
						if (((pHitEntity)->GetIsTypeVehicle()))
						{
							VehDimension = pHitEntity->GetColModel().GetBoundBoxMax();
							VehDimension -= pHitEntity->GetColModel().GetBoundBoxMin();
							
							if (ACarHasBeenFound)
							{
								HighestZSoFar=MAX(HighestZSoFar, VehDimension.z);
							}
							else
							{
								ACarHasBeenFound=true;
								HighestZSoFar=VehDimension.z;
							}
							if (((CVehicle*)pHitEntity)->GetBaseVehicleType()==VEHICLE_TYPE_TRAIN)
							{
								m_bTheHeightFixerVehicleIsATrain=true;
							}
						}
					}
					
					TemporarySourceForCars=PlayerTargetCoors - DistanceToCheckDirectlyBehind*(CVector(CMaths::Cos(CameraDirection - OffSetToCheckLaterally), CMaths::Sin(CameraDirection - OffSetToCheckLaterally), 0));			
		//			CDebug::DebugLine3D( PlayerPed->GetPosition().x, PlayerPed->GetPosition().y, PlayerPed->GetPosition().z ,
		//			TemporarySourceForCars.x, TemporarySourceForCars.y, TemporarySourceForCars.z ,
		//			0xffffffff, 0xffffffff);
					if (CWorld::ProcessLineOfSight(((CPed*)CamTargetEntity)->GetPosition()  , TemporarySourceForCars, colPoint, pHitEntity,false, true, false, false, false, false, false))
					{
						if (((pHitEntity)->GetIsTypeVehicle()))
						{
							VehDimension = pHitEntity->GetColModel().GetBoundBoxMax();
							VehDimension -= pHitEntity->GetColModel().GetBoundBoxMin();
							
							if (ACarHasBeenFound)
							{
								
								HighestZSoFar=MAX(HighestZSoFar, VehDimension.z);
							}
							else
							{
								ACarHasBeenFound=true;
								HighestZSoFar=VehDimension.z;
							}
							if (((CVehicle*)pHitEntity)->GetBaseVehicleType()==VEHICLE_TYPE_TRAIN)
							{
								m_bTheHeightFixerVehicleIsATrain=true;
							}
						}
					}
					
					if (ACarHasBeenFound)
					{
						//m_fDimensionOfHighestNearCar=HighestZSoFar/2.0f+0.4f;
						m_fDimensionOfHighestNearCar=HighestZSoFar+0.1f;	
						
/* DWREMOVED			if (Mode==CCam::MODE_FIGHT_CAM)
						{
							m_fDimensionOfHighestNearCar+=0.75f;
						}*/
						
					}
					else
					{
						m_fDimensionOfHighestNearCar=0.0f;
					}
				}
			}
		}
	}
	if (ClipOnBoat)
	{
		m_fDimensionOfHighestNearCar=1.0f;
		m_fPedBetweenCameraHeightOffset=0.0f;
	}
#endif
}
///////////////////////////////////////////////////////////////////////////
// NAME       : ShouldPedControlsBeRelative()
// PURPOSE    : For some modes the ped controls should be relative.
//				(left/right rotates and forward moves)
//				This function returns true if that is the case.
//				(Car controls are always relative)
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////

Int8 CCamera::ShouldPedControlsBeRelative(void)
{
	return 1;
#if 0	
	switch(Cams[ActiveCam].Mode)
	{
		case CCam::MODE_1STPERSON:
			return(1);
			break;
		default:
			return(0);
			break;
	}
#endif	
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// NAME       : ProcessArrestCamTwo(void)
// PURPOSE    : this is the one that is up in the air
// RETURNS    :
// PARAMETERS :
///////////////////////////////////////////////////////////////////////////
bool  CCam::ProcessArrestCamTwo(void)
{
	ASSERT(false);
	#if 0
	bool DoNotAssert=true;

	CVector PlayerPosition;
	CVector CamToPlayer;
	//CPed* HandyPedPointer; 
	float DistanceAwayFromPlayer=11.5f;
	CVector OtherCamsSource;
	CPlayerPed* PlayerPed=((CPlayerPed*)CWorld::Players[CWorld::PlayerInFocus].pPed);
	ASSERTMSG(PlayerPed!=NULL, "ITS NOT A BUG ITS A FEATURE");
	if (ResetStatics)
	{
		ResetStatics=false;
		int Sign=1;
		if ((&(TheCamera.Cams[TheCamera.ActiveCam]))==this)
		{
			OtherCamsSource=TheCamera.Cams[(TheCamera.ActiveCam + 1) % 2].Source;
		}
		else
		{
			OtherCamsSource=TheCamera.Cams[TheCamera.ActiveCam].Source;
		}
		for (int counter=0; counter<=1; counter++)
		{
			if (counter==0)
			{
				Sign=1;		
			}
			else
			{
				Sign=-1;
			}

			FOV= 60.0F;
			PlayerPosition=PlayerPed->GetPosition();
			CamToPlayer=PlayerPosition- OtherCamsSource;
			Beta=CGeneral::GetATanOfXY(CamToPlayer.x, CamToPlayer.y);
			Source=PlayerPosition+(DistanceAwayFromPlayer*CVector(CMaths::Cos(Beta+DEGTORAD(80*Sign)), CMaths::Sin(Beta+DEGTORAD(80*Sign)), 0));  
			CVector AvoidPlayerCar=Source-PlayerPosition;
			AvoidPlayerCar.Normalise();
			PlayerPosition.x+=AvoidPlayerCar.x*0.4f;
			PlayerPosition.y+=AvoidPlayerCar.y*0.4f;
			
	//		CDebug::DebugLine3D(Source.x, Source.y, Source.z, PlayerPosition.x, PlayerPosition.y, PlayerPosition.z,  0xffffffff, 0xffffffff);
			if (CWorld::GetIsLineOfSightClear(Source, PlayerPosition, 1, 1, 0, true, 0, true, true))
			{
				Source.z+=5.5f;
				PlayerPosition.x-=AvoidPlayerCar.x*0.8f;
				PlayerPosition.y-=AvoidPlayerCar.y*0.8f;
				PlayerPosition.z+=2.2f;
				m_cvecTargetCoorsForFudgeInter=PlayerPosition;
				Front=PlayerPosition-Source;
				ResetStatics=false;
				GetVectorsReadyForRW();
				return true;
			}
		}
		return false;
	}
	#endif
	return true;
}


#if 0
//-----------------------------------------------------------------------------------------------
// After a period of user inactivity process an idle camera that nosies about looking at things.
// Not implemented as a camera mode because of all the confusion about mode changing shite... I basically
// cannot fully understand what is going on so I'm implementing this in a safe way that I know will work.
bool CCam::PossiblyProcessIdleCam()
{	
	float time 		= CTimer::GetTimeInMilliseconds();	
	CPlayerPed* pPlayerPed = FindPlayerPed();
		
	if (!IsItTimeForIdleCam())
	{
		RemoveEntRefs();
		framesInThisMode = 0; 		
		return false;
	}
	
	// First time stuff
	if (framesInThisMode<=0)
	{
//		printf("first time in idle cam\n");
		bWasOutOfSight = false;
		outOfSightTime = 0.0f;
		gTimeSlerpStarted = 0.0f;
		gTimeLastIdleTargetSelected = 0.0f;
				
	}
		
	float shakeDegree = framesInThisMode/framesToInterpolateUpTheShake;
	if (shakeDegree>1.0f) shakeDegree = 1.0f;
	framesInThisMode++;

	// If we have an old reference to another object we slerp from its position	
	CPed *pLastPed 			= pIdleTargetPed;
	CVehicle *pLastVehicle 	= pIdleTargetVehicle;
		
	if (IsItTimeForNewIdleTarget())
	{
		if (SelectNewIdleTarget())
		{
			StartSlerp();									
		}				
	}

	ProcessIdleFOVZooms();
	ProcessSlerps();

	if (dist>distanceToRejectObject1)
	{
		// FOV Start zoom in
	}
	
	// Continuous processing
	if (dist<distanceToRejectObject0)
	{	
		if (pIdleTargetPed && pIdleTargetPed == pPlayerPed)
		{
//			printf("player too near but this is ok\n");						
		}
		else
		{		
//			printf("target too near , setting to lost target\n");						
			bTargetLost = true;
		}
	}			
	else if (dist>distanceToRejectObject1)
	{	
//		printf("target at FOV start range \n");							
	
		// are we slerping to another target
		float timeDeltaSlerp = time-gTimeSlerpStarted;
		float t2 = timeDeltaSlerp/timeToSlerpFromOneToAnother;
		bool bAllowInitiateFOVZoom = bWasFOVZooming || (deltaTime/gMaxDurationForAnIdleCam)<0.75f; // dont want to initiate an FOV zoom that might get interupted or right at the end of changing targets.
		if (t2>=1.0f && bAllowInitiateFOVZoom)
		{
			if (!bWasFOVZooming)
			{			
				bWasFOVZooming 	= true;
				cachedFOV		= FOV;
				timeFOVZoomStarted = time;
			}

			float dt = time-timeFOVZoomStarted;
			float t  = dt/timeFOVZoomDuration;		
			if (t>1.0f) t = 1.0f;		
			FOV = DW_SINE_ACCEL_DECEL_LERP(t,cachedFOV,FOVZoomIn);			
		}
//		printf("FOV %f\n");	
		
		if (dist>distanceToRejectObject2)
		{
			bTargetLost = true;
		}		
	}
	else 
	{	
//		printf("target in nice range \n");							
				
		if (bWasFOVZooming)
		{		
			bWasFOVZooming = false;								
			
			float dt = time-timeFOVZoomStarted; // as above however it calculated where we where at.
			float t  = dt/timeFOVZoomDuration;			
			if (t>1.0f) t = 1.0f;
			cachedFOV = DW_SINE_ACCEL_DECEL_LERP(t,cachedFOV,FOVZoomIn);
//			printf("setting a new FOV to zoom too %f \n",cachedFOV);	

			timeFOVZoomStarted = time;			
		}		

		float dt = time-timeFOVZoomStarted;
		float t  = dt/timeFOVZoomDuration;
		if (t>1.0f) t = 1.0f;		
		FOV = DW_SINE_ACCEL_DECEL_LERP(t,cachedFOV,FOVZoomOut);
	}	
	
	

	
	// dont move the source we make it like the virtual cameraman is bored... ( I tell you what he is not as bored as I am writing this shite )		
	CVector sourcePos = Source;
	CVector targetToLookAtPos;
	
	float deltaTime = time - gTimeLastIdleTargetSelected;

	bool bSlerp = true;	
	bool bNewTarget = false;
				
	if (deltaTime>gMaxDurationForAnIdleCam)
	{
//		printf("time expired looking at this target\n");
		gTimeLastIdleTargetSelected = time;				
		
		// If we have an old reference to another object we slerp from its position	
		CPed *pLastPed = pIdleTargetPed;
		CVehicle *pLastVehicle = pIdleTargetVehicle;

		RemoveEntRefs();
		
		bool bLookAtPed = CGeneral::GetRandomNumberInRange(0.0f, 1.0f) > RandDegreeOfVehicleSelection;
		if (bLookAtPed)
		{
			pIdleTargetPed = pPlayerPed->GetPedIntelligence()->GetClosestPedInRange();												
			if (!pIdleTargetPed) // no peds ... get a car
				pIdleTargetVehicle = pPlayerPed->GetPedIntelligence()->GetClosestVehicleInRange();			
		}
		else // look at a vehicle
		{
			pIdleTargetVehicle = pPlayerPed->GetPedIntelligence()->GetClosestVehicleInRange();		
			if (!pIdleTargetVehicle)
				pIdleTargetPed = pPlayerPed->GetPedIntelligence()->GetClosestPedInRange();				
		}

		// WHAT ARE WE LOOKING AT?
		if (pIdleTargetPed)
		{		
			targetToLookAtPos = pIdleTargetPed->GetPosition();
			targetToLookAtPos.z += 0.5f;
//			printf("we choose a ped to look at\n");
			
		}	
		else if (pIdleTargetVehicle)
		{
			targetToLookAtPos = pIdleTargetVehicle->GetPosition();
//			printf("we choose a vehicle to look at\n");			
		}	

		// Dont choose an object with collision problems straight off!
		{
			CEntity *pOldIgnore = CWorld::pIgnoreEntity;
			if (pIdleTargetVehicle)		CWorld::pIgnoreEntity = pIdleTargetVehicle;	
			else if (pIdleTargetPed)	CWorld::pIgnoreEntity = pIdleTargetPed;	
			
			if (!CWorld::GetIsLineOfSightClear(Source,targetToLookAtPos, true, true, false, true, false, false, true))
			{
//				printf("collision blocked\n");				
				RemoveEntRefs();
			}
			CWorld::pIgnoreEntity = pOldIgnore;
		}
					
		if (pIdleTargetPed)
		{
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);
		}
		else if (pIdleTargetVehicle)
		{
			REGREF(pIdleTargetVehicle, (CEntity **)&pIdleTargetVehicle);
		}
		else
		{
			pIdleTargetPed = pPlayerPed; // look at the player - nothing else to look at.
			REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);
		}
					
		// If we have an old reference to another object we slerp from its position		
		if (pIdleTargetPed && pIdleTargetPed == pLastPed)
			bSlerp = false;	// we happen to have selected the same object again, so we maintain the look at this same object	
		if (pIdleTargetVehicle && pIdleTargetVehicle == pLastVehicle)
			bSlerp = false; // we happen to have selected the same object again, so we maintain the look at this same object	

		if (pLastPed)
		{
			SlerpFromPos = pLastPed->GetPosition();
			SlerpFromPos.z += 0.5f;	
		}
				
		if (bSlerp)
		{	
			gTimeSlerpStarted = time;
			
			if (pLastVehicle)			
			{
//				printf("slerp from pos chosen as vehicle\n");					
				SlerpFromPos = pLastVehicle->GetPosition();
			}
			else if (pLastPed)
			{
//				printf("slerp from pos chosen as ped\n");								
				SlerpFromPos = pLastPed->GetPosition();
				SlerpFromPos.z += 0.5f;			
			}
			else
			{
//				printf("slerp from pos chosen as player\n");								
				SlerpFromPos = Source + Front;
			}
		}	
				
		bNewTarget = true;
	}		
	
	
	// WHAT ARE WE LOOKING AT?
	if (pIdleTargetPed)
	{		
		targetToLookAtPos = pIdleTargetPed->GetPosition();
		targetToLookAtPos.z += 0.5f;
	}	
	else if (pIdleTargetVehicle)
	{
		targetToLookAtPos = pIdleTargetVehicle->GetPosition();
	}		
	

	// COLLISION check
	bool bTargetLost = false;	
	
	CEntity *pOldIgnore = CWorld::pIgnoreEntity;
		
	if (pIdleTargetVehicle)		CWorld::pIgnoreEntity = pIdleTargetVehicle;	
	else if (pIdleTargetPed)	CWorld::pIgnoreEntity = pIdleTargetPed;	
	
	if (!CWorld::GetIsLineOfSightClear(Source,targetToLookAtPos, true, true, false, true, false, false, true))
	{		
		printf("collision blocked again!\n");						
	
		if (!bWasOutOfSight)
		{
			bWasOutOfSight = true;
			outOfSightTime  = time; // cache the time it was first out of sight
		}
		else if (time-outOfSightTime > TargetHasGoneTime) // eventually we have to give up on our object, its gone!
			bTargetLost = true;
	}	
	else
	{
		bWasOutOfSight = false;
		outOfSightTime = 0.0f;
	}	
	CWorld::pIgnoreEntity = pOldIgnore;



	// object has gone too far away
	CVector delta = Source-targetToLookAtPos;
	float dist = delta.Magnitude();

	// We dont want any dodgy slerps...
	if (bNewTarget && (dist<distanceToRejectObject0 || dist>distanceToRejectObject2))
	{
		targetToLookAtPos = SlerpFromPos;
	}

	if (dist<distanceToRejectObject0)
	{	
		if (pIdleTargetPed && pIdleTargetPed == pPlayerPed)
		{
//			printf("player too near but this is ok\n");						
		}
		else
		{		
//			printf("target too near , setting to lost target\n");						
			bTargetLost = true;
		}
	}			
	else if (dist>distanceToRejectObject1)
	{	
//		printf("target at FOV start range \n");							
	
		// are we slerping to another target
		float timeDeltaSlerp = time-gTimeSlerpStarted;
		float t2 = timeDeltaSlerp/timeToSlerpFromOneToAnother;
		bool bAllowInitiateFOVZoom = bWasFOVZooming || (deltaTime/gMaxDurationForAnIdleCam)<0.75f; // dont want to initiate an FOV zoom that might get interupted or right at the end of changing targets.
		if (t2>=1.0f && bAllowInitiateFOVZoom)
		{
			if (!bWasFOVZooming)
			{			
				bWasFOVZooming 	= true;
				cachedFOV		= FOV;
				timeFOVZoomStarted = time;
			}

			float dt = time-timeFOVZoomStarted;
			float t  = dt/timeFOVZoomDuration;		
			if (t>1.0f) t = 1.0f;		
			FOV = DW_SINE_ACCEL_DECEL_LERP(t,cachedFOV,FOVZoomIn);			
		}
//		printf("FOV %f\n");	
		
		if (dist>distanceToRejectObject2)
		{
			bTargetLost = true;
		}		
	}
	else 
	{	
//		printf("target in nice range \n");							
				
		if (bWasFOVZooming)
		{		
			bWasFOVZooming = false;								
			
			float dt = time-timeFOVZoomStarted; // as above however it calculated where we where at.
			float t  = dt/timeFOVZoomDuration;			
			if (t>1.0f) t = 1.0f;
			cachedFOV = DW_SINE_ACCEL_DECEL_LERP(t,cachedFOV,FOVZoomIn);
//			printf("setting a new FOV to zoom too %f \n",cachedFOV);	

			timeFOVZoomStarted = time;			
		}		

		float dt = time-timeFOVZoomStarted;
		float t  = dt/timeFOVZoomDuration;
		if (t>1.0f) t = 1.0f;		
		FOV = DW_SINE_ACCEL_DECEL_LERP(t,cachedFOV,FOVZoomOut);
	}
	
	
	if (bTargetLost)
	{	
//		printf("target was lost\n");	
		
		if (pIdleTargetPed && pIdleTargetPed == pPlayerPed)
			return false; //the player cannot even be seen
		
		// We now want to slerp back to the player
		gTimeSlerpStarted = time;
		SlerpFromPos = targetToLookAtPos; // this isnt right - we are slerping back froma position that has been found to be invalid.
				
		gTimeLastIdleTargetSelected = time;
		pIdleTargetPed = pPlayerPed;
		REGREF(pIdleTargetPed, (CEntity **)&pIdleTargetPed);			

		targetToLookAtPos = pIdleTargetPed->GetPosition();
		targetToLookAtPos.z += 0.5f;				
	}

	// work out angles for slerp.
	CVector rvFrom 	= SlerpFromPos-Source;
	CVector rvTo 	= targetToLookAtPos-Source;
	float SlerpToRotX,SlerpToRotZ;	
		
	VectorToAnglesRotXRotZ(&rvFrom,&SlerpFromRotX,&SlerpFromRotZ);
	VectorToAnglesRotXRotZ(&rvTo,&SlerpToRotX,&SlerpToRotZ);

	if(SlerpToRotX - SlerpFromRotX > PI)		SlerpToRotX -= TWO_PI;
	else if(SlerpToRotX -SlerpFromRotX < -PI)	SlerpToRotX += TWO_PI;

	if(SlerpToRotZ - SlerpFromRotZ > PI)		SlerpToRotZ -= TWO_PI;
	else if(SlerpToRotZ -SlerpFromRotZ < -PI)	SlerpToRotZ += TWO_PI;
	
	float timeDeltaSlerp = time-gTimeSlerpStarted;
	float t = timeDeltaSlerp/timeToSlerpFromOneToAnother;	

	if (t>1.0f)
		t = 1.0f;
	float curAngleX = DW_SINE_ACCEL_DECEL_LERP(t,SlerpFromRotX,SlerpToRotX);
	float curAngleZ = DW_SINE_ACCEL_DECEL_LERP(t,SlerpFromRotZ,SlerpToRotZ);	

#ifndef MASTER
	char str[255];
	sprintf(str,"X ANG %f FROM %f TO %f",curAngleX,SlerpFromRotX,SlerpToRotX);
	VarConsole.AddDebugOutput(str);
	sprintf(str,"Z ANG %f FROM %f TO %f",curAngleZ,SlerpFromRotZ,SlerpToRotZ);
	VarConsole.AddDebugOutput(str);
#endif

	Front = CVector(-CMaths::Cos(curAngleZ) *CMaths::Cos(curAngleX) , -CMaths::Sin(curAngleZ)*CMaths::Cos(curAngleX), CMaths::Sin(curAngleX));
		
	// If target obscured look at player, as we can guarantee the player is not obscured, since we havent moved the camera						
	Front.Normalise();
	
	
	static float degreeShakeIdleCam = 1.0f;
	CHandShaker *pS = &gHandShaker[0];		
	pS->Process(degreeShakeIdleCam*shakeDegree);		
	float roll = pS->ang.z * degreeShakeIdleCam*shakeDegree; // might be z ???
	CVector temp = Multiply3x3(Front,pS->resultMat);
	Front = temp;	
	
	Up=CVector(CMaths::Sin(roll),0.0f,CMaths::Cos(roll));
	CVector TempRight= CrossProduct(Front, Up);
	TempRight.Normalise();
	Up=CrossProduct(TempRight, Front);

	if ((Front.x==0.0f) && (Front.y==0.0f))
	{
		Front.x=0.0001f;
		Front.y=0.0001f;
	}
	
	TempRight = CrossProduct( Front, Up );
	TempRight.Normalise();
	Up = CrossProduct( TempRight, Front );			
	
	
	
	GetVectorsReadyForRW();	
	
	//playerPos = FindPlayer();
	return true;
}
#endif

////////////////////////////////////////////////////
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX
//XXXXXXXXXXXXXXXXXXXXXX DELETED MODES - DW XXXXXXXX

