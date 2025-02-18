//
//
//    Filename: VehicleModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a model with a hierarchy
//
//
// RenderWare headers
//$DW$#include <rwcore.h>
#if (rwLIBRARYCURRENTVERSION >= 0x310)
//$DW$#include <rpmatfx.h>
#endif

// Game headers
#include "MemoryMgr.h"
#include "ModelInfo.h"
#include "VehicleModelInfo.h"
#include "AtomicModelInfo.h"
#include "HierarchyIds.h"
#include "NodeNamePlugin.h"
#include "VisibilityPlugins.h"

//!PC - PC doesn't support custom renderers etc.
#if defined (GTA_PS2)
#include "eemacros.h"
#include "ps2all.h"
#include "CustomCVCarEnvMapPipeline.h"
#endif

#include "FileMgr.h"
#include "FileLoader.h"
#include "General.h"
#include "HandlingMgr.h"
#include "handy.h"
#include "txdstore.h"
#include "weather.h"
#include "clock.h"
#include "camera.h"
#include "lights.h"
#include "modelindices.h"
#include "AnimManager.h"
#include "player.h"
#include "vehicle.h"
#include "cheat.h"

// security stuff
#include "securom_api.h"

bool gbUseCarColTex = FALSE;

#ifdef GTA_PS2
	#ifdef GTA_SA
		#define USE_VU_CARFX_RENDERER		(1)
	#endif
	
	#ifdef USE_VU_CARFX_RENDERER
		#include "VuCarFXRenderer.h"
	#endif 
#endif

#if (defined(GTA_PC) || defined(GTA_XBOX))
		#define USE_PC_CARFX_RENDERER		(1)
#endif

#ifdef USE_PC_CARFX_RENDERER
	#include "CarFXRenderer.h"
#endif


#include "CustomCarPlateMgr.h"

CPool<CVehicleModelInfo::CVehicleStructure>* CVehicleModelInfo::CVehicleStructure::m_pInfoPool;

RwRGBA CVehicleModelInfo::ms_vehicleColourTable[NUM_VEHICLE_COLOURS];
//RwTexture* CVehicleModelInfo::ms_colourTextureTable[NUM_VEHICLE_COLOURS];
//RwTexture* CVehicleModelInfo::ms_pEnvironmentMaps[NUM_ENVIRON_MAPS];
RwTexDictionary* CVehicleModelInfo::ms_pCommonTextureDictionary = NULL;
RwTexture* CVehicleModelInfo::ms_pLightsTexture=NULL;
RwTexture* CVehicleModelInfo::ms_pLightsOnTexture=NULL;
int16 CVehicleModelInfo::ms_numWheelUpgrades[NUM_WHEEL_UPGRADE_CLASSES];
int16 CVehicleModelInfo::ms_upgradeWheels[NUM_WHEEL_UPGRADE_CLASSES][NUM_WHEEL_UPGRADES];
int8 CVehicleModelInfo::ms_compsUsed[2];
int8 CVehicleModelInfo::ms_compsToUse[2] = {-2, -2};
uint8 CVehicleModelInfo::ms_currentCol[4];
RwTexture* CVehicleModelInfo::ms_pRemapTexture;
CVehicleModelInfo::CLinkedUpgradeList CVehicleModelInfo::ms_linkedUpgrades;
RwTexture* gpWhiteTexture;
bool CVehicleModelInfo::ms_lightsOn[4];
#ifndef FINAL
char CVehicleModelInfo::ms_vehicleColourText[NUM_VEHICLE_COLOURS][255];
#endif

RwSurfaceProperties gLightSurfProps = {16.0f, 0.0f, 0.0f};
#if defined (GTA_PC) || defined (GTA_XBOX)
RwSurfaceProperties gLightOffSurfProps = {0.0f, 0.0f, 0.0f};
#endif
//
// Hierarchy descriptions
//
static RwObjectNameIdAssocation carIds[] = {
	{"chassis", 	CAR_CHASSIS, 	0},
	{"wheel_rf_dummy", 	CAR_WHEEL_RF, 	VEHICLE_PARENT_WHEEL|VEHICLE_RIGHT},
	{"wheel_rm_dummy", 	CAR_WHEEL_RM, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb_dummy", 	CAR_WHEEL_RR, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_lf_dummy", 	CAR_WHEEL_LF, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lm_dummy", 	CAR_WHEEL_LM, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb_dummy", 	CAR_WHEEL_LR, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"door_rf_dummy", 	CAR_DOOR_RF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_FLAT|VEHICLE_TOP|VEHICLE_FRONTDOOR},
	{"door_rr_dummy", 	CAR_DOOR_RR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_TOP|VEHICLE_REARDOOR},
	{"door_lf_dummy", 	CAR_DOOR_LF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_FLAT|VEHICLE_TOP|VEHICLE_FRONTDOOR},
	{"door_lr_dummy", 	CAR_DOOR_LR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_TOP|VEHICLE_REARDOOR},
	{"bump_front_dummy",CAR_BUMPFRONT,	VEHICLE_CRUSH|VEHICLE_FRONT},
	{"bump_rear_dummy", CAR_BUMPREAR, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"wing_rf_dummy", 	CAR_WING_RF, 	VEHICLE_CRUSH/*|VEHICLE_RIGHT*/},
	{"wing_lf_dummy", 	CAR_WING_LF, 	VEHICLE_CRUSH/*|VEHICLE_LEFT*/},
	{"bonnet_dummy", 	CAR_BONNET, 	VEHICLE_CRUSH},
	{"boot_dummy", 		CAR_BOOT, 		VEHICLE_CRUSH|VEHICLE_REAR|VEHICLE_TOP},
	{"windscreen_dummy", CAR_WINDSCREEN, 	VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_WINDSCREEN|VEHICLE_FRONT},
	{"exhaust_ok", 		CAR_EXHAUST, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"misc_a",			CAR_MISC_A,		0},
	{"misc_b",			CAR_MISC_B,		0},
	{"misc_c",			CAR_MISC_C,		0},
	{"misc_d",			CAR_MISC_D,		0},
	{"misc_e",			CAR_MISC_E,		0},
	// positions
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"hookup",			VEHICLE_HOOKUP_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_arm",			VEHICLE_PEDARM_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_c",		VEHICLE_MISC_C_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_d",		VEHICLE_MISC_D_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		VEHICLE_MISC_A_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		VEHICLE_MISC_B_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	// extras
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	// upgrade positions
	{"ug_bonnet",		VEH_UPGRADE_BONNET, 	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_bonnet_left",	VEH_UPGRADE_BONNET_LEFT, 	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_bonnet_right",	VEH_UPGRADE_BONNET_RIGHT, 	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_bonnet_dam",	VEH_UPGRADE_BONNET_DAMAGED, 	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_bonnet_left_dam",	VEH_UPGRADE_BONNET_LEFT_DAMAGED, 	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_bonnet_right_dam",	VEH_UPGRADE_BONNET_RIGHT_DAMAGED, 	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_spoiler",		VEH_UPGRADE_SPOILER,	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_spoiler_dam",	VEH_UPGRADE_SPOILER_DAMAGED,	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_wing_left",	VEH_UPGRADE_WING_LEFT, 	VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_wing_right",	VEH_UPGRADE_WING_RIGHT, VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_frontbullbar",	VEH_UPGRADE_FRONTBULLBAR, VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_backbullbar",	VEH_UPGRADE_BACKBULLBAR, VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_lights",		VEH_UPGRADE_FRONTLIGHTS, VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_lights_dam",	VEH_UPGRADE_FRONTLIGHTS_DAMAGED, VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_roof",			VEH_UPGRADE_ROOF, VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{"ug_nitro",		VEH_UPGRADE_NITRO, VEHICLE_UPGRADE_POSN|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};


static RwObjectNameIdAssocation planeIds[] = {
	{"chassis", 		CAR_CHASSIS, 	0},
	{"wheel_rf_dummy", 	CAR_WHEEL_RF, 	VEHICLE_PARENT_WHEEL|VEHICLE_RIGHT},
	{"wheel_rm_dummy", 	CAR_WHEEL_RM, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb_dummy", 	CAR_WHEEL_RR, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_lf_dummy", 	CAR_WHEEL_LF, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lm_dummy", 	CAR_WHEEL_LM, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb_dummy", 	CAR_WHEEL_LR, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"door_rf_dummy", 	CAR_DOOR_RF, 	VEHICLE_CRUSH|VEHICLE_DOOR/*|VEHICLE_RIGHT|VEHICLE_FRONTDOOR*/|VEHICLE_DONTCULL|VEHICLE_ALPHA},
	{"door_rr_dummy", 	CAR_DOOR_RR, 	VEHICLE_CRUSH|VEHICLE_DOOR/*|VEHICLE_RIGHT|VEHICLE_REAR|VEHICLE_REARDOOR*/|VEHICLE_DONTCULL|VEHICLE_ALPHA},
	{"door_lf_dummy", 	CAR_DOOR_LF, 	VEHICLE_CRUSH|VEHICLE_DOOR/*|VEHICLE_LEFT|VEHICLE_FRONTDOOR*/|VEHICLE_DONTCULL|VEHICLE_ALPHA},
	{"door_lr_dummy", 	CAR_DOOR_LR, 	VEHICLE_CRUSH|VEHICLE_DOOR/*|VEHICLE_LEFT|VEHICLE_REAR|VEHICLE_REARDOOR*/|VEHICLE_DONTCULL|VEHICLE_ALPHA},
	{"static_prop",		PLANE_STATIC_PROP,	VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_FRONT|VEHICLE_UNIQUE_MATERIALS},
	{"moving_prop", 	PLANE_MOVING_PROP, 	VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_FRONT|VEHICLE_UNIQUE_MATERIALS},
	{"static_prop2", 	PLANE_STATIC_PROP2, VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_FRONT|VEHICLE_UNIQUE_MATERIALS},
	{"moving_prop2", 	PLANE_MOVING_PROP2, VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_FRONT|VEHICLE_UNIQUE_MATERIALS},
	{"rudder",			PLANE_RUDDER, 		VEHICLE_CRUSH|VEHICLE_REAR},
	{"elevator_l", 		PLANE_ELEVATOR_L, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"elevator_r", 		PLANE_ELEVATOR_R, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"aileron_l", 		PLANE_AILERON_L, 	VEHICLE_CRUSH},
	{"aileron_r",		PLANE_AILERON_R,	VEHICLE_CRUSH},
	{"gear_l",			PLANE_GEAR_L,		0},
	{"gear_r",			PLANE_GEAR_R,		0},
	{"misc_a",			PLANE_MISC_A,		0},
	{"misc_b",			PLANE_MISC_B,		0},
	// positions
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"aileron_pos",		PLANE_AILERON_POSN,			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"elevator_pos",	PLANE_ELEVATOR_POSN,		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"rudder_pos",		PLANE_RUDDER_POSN,			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"wingtip_pos",		PLANE_WINGTIP_POSN,			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		VEHICLE_MISC_A_POSN,		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		VEHICLE_MISC_B_POSN,		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	// extras
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};

static RwObjectNameIdAssocation heliIds[] = {
	{"chassis", 	CAR_CHASSIS, 	0},
	{"wheel_rf_dummy", 	CAR_WHEEL_RF, 	VEHICLE_PARENT_WHEEL|VEHICLE_RIGHT},
	{"wheel_rm_dummy", 	CAR_WHEEL_RM, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb_dummy", 	CAR_WHEEL_RR, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_lf_dummy", 	CAR_WHEEL_LF, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lm_dummy", 	CAR_WHEEL_LM, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb_dummy", 	CAR_WHEEL_LR, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"door_rf_dummy", 	CAR_DOOR_RF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_DONTCULL|VEHICLE_RIGHT|VEHICLE_FRONTDOOR},
	{"door_rr_dummy", 	CAR_DOOR_RR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_DONTCULL|VEHICLE_RIGHT|VEHICLE_REAR|VEHICLE_REARDOOR},
	{"door_lf_dummy", 	CAR_DOOR_LF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_DONTCULL|VEHICLE_LEFT|VEHICLE_FRONTDOOR},
	{"door_lr_dummy", 	CAR_DOOR_LR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_DONTCULL|VEHICLE_LEFT|VEHICLE_REAR|VEHICLE_REARDOOR},
	{"static_rotor",	HELI_STATIC_ROTOR,	VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_UNIQUE_MATERIALS},
	{"moving_rotor", 	HELI_MOVING_ROTOR, 	VEHICLE_CRUSH|VEHICLE_ALPHA|/*VEHICLE_TOP|*/VEHICLE_UNIQUE_MATERIALS},
	{"static_rotor2", 	HELI_STATIC_ROTOR2, VEHICLE_CRUSH|VEHICLE_RIGHT|VEHICLE_UNIQUE_MATERIALS},
	{"moving_rotor2", 	HELI_MOVING_ROTOR2, VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_RIGHT|VEHICLE_UNIQUE_MATERIALS},
	{"rudder",		 	HELI_RUDDER, 		VEHICLE_CRUSH|VEHICLE_REAR},
	{"elevators", 		HELI_ELEVATORS, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"misc_a",			HELI_MISC_A, 		VEHICLE_CRUSH},
	{"misc_b",			HELI_MISC_B,		VEHICLE_CRUSH},
	{"misc_c",			HELI_MISC_C,		VEHICLE_CRUSH},
	{"misc_d",			HELI_MISC_D,		VEHICLE_CRUSH},
	// positions
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"hookup",			VEHICLE_HOOKUP_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_arm",			VEHICLE_PEDARM_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_c",		VEHICLE_MISC_C_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_d",		VEHICLE_MISC_D_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		VEHICLE_MISC_A_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		VEHICLE_MISC_B_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	// extras
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};

static RwObjectNameIdAssocation truckIds[] = {
	{"chassis", 	CAR_CHASSIS, 	0},
	{"wheel_rf_dummy", 	CAR_WHEEL_RF, 	VEHICLE_PARENT_WHEEL|VEHICLE_RIGHT}, // don't add wheels?
	{"wheel_rm_dummy", 	CAR_WHEEL_RM, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb_dummy", 	CAR_WHEEL_RR, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_lf_dummy", 	CAR_WHEEL_LF, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lm_dummy", 	CAR_WHEEL_LM, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb_dummy", 	CAR_WHEEL_LR, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"door_rf_dummy", 	CAR_DOOR_RF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_FLAT|VEHICLE_FRONTDOOR},
	{"door_rr_dummy", 	CAR_DOOR_RR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_REARDOOR},
	{"door_lf_dummy", 	CAR_DOOR_LF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_FLAT|VEHICLE_FRONTDOOR},
	{"door_lr_dummy", 	CAR_DOOR_LR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_REARDOOR},
	{"bump_front_dummy",CAR_BUMPFRONT,	VEHICLE_CRUSH|VEHICLE_FRONT},
	{"bump_rear_dummy", CAR_BUMPREAR, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"wing_rf_dummy", 	CAR_WING_RF, 	VEHICLE_CRUSH},
	{"wing_lf_dummy", 	CAR_WING_LF, 	VEHICLE_CRUSH},
	{"bonnet_dummy", 	CAR_BONNET, 	VEHICLE_CRUSH},
	{"boot_dummy", 		CAR_BOOT, 		VEHICLE_CRUSH|VEHICLE_REAR},
	{"windscreen_dummy", CAR_WINDSCREEN, 	VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_WINDSCREEN|VEHICLE_FRONT},
	{"transmission_f",	TRUCK_TRANSMISSION_F,	0}, // don't crush these guys cause wheels attached to them?
	{"transmission_r",	TRUCK_TRANSMISSION_R,	0},
	{"loadbay",			TRUCK_LOADBAY,			VEHICLE_CRUSH},
	{"misc_a",			TRUCK_MISC_A,			VEHICLE_CRUSH},
	// positions
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"hookup",			VEHICLE_HOOKUP_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_arm",			VEHICLE_PEDARM_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_c",		VEHICLE_MISC_C_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_d",		VEHICLE_MISC_D_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		VEHICLE_MISC_A_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		VEHICLE_MISC_B_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	// extras
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};


static RwObjectNameIdAssocation quadIds[] = {
	{"chassis", 	CAR_CHASSIS, 	0},
	{"wheel_rf_dummy", 	CAR_WHEEL_RF, 	VEHICLE_PARENT_WHEEL|VEHICLE_RIGHT}, // don't add wheels?
	{"wheel_rm_dummy", 	CAR_WHEEL_RM, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb_dummy", 	CAR_WHEEL_RR, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_lf_dummy", 	CAR_WHEEL_LF, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lm_dummy", 	CAR_WHEEL_LM, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb_dummy", 	CAR_WHEEL_LR, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"door_rf_dummy", 	CAR_DOOR_RF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_FLAT|VEHICLE_FRONTDOOR},
	{"door_rr_dummy", 	CAR_DOOR_RR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_REARDOOR},
	{"door_lf_dummy", 	CAR_DOOR_LF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_FLAT|VEHICLE_FRONTDOOR},
	{"door_lr_dummy", 	CAR_DOOR_LR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_REARDOOR},
	{"body_front_dummy",QUAD_BODY_F,	VEHICLE_CRUSH|VEHICLE_FRONT},
	{"body_rear_dummy", QUAD_BODY_R, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"suspension_rf", 	QUAD_SUSPENSION_RF, VEHICLE_CRUSH|VEHICLE_FRONT},	// don't crush cause wheels attached?
	{"suspension_lf", 	QUAD_SUSPENSION_LF, VEHICLE_CRUSH|VEHICLE_FRONT},
	{"rear_axle",	 	QUAD_REAR_AXLE, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"handlebars", 		QUAD_HANDLEBARS, 	VEHICLE_CRUSH|VEHICLE_FRONT},
	{"misc_a",			QUAD_MISC_A, 		VEHICLE_CRUSH},
	{"misc_b",			QUAD_MISC_B,		VEHICLE_CRUSH},
	// positions
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"hookup",			VEHICLE_HOOKUP_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_arm",			VEHICLE_PEDARM_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_c",		VEHICLE_MISC_C_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_d",		VEHICLE_MISC_D_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		VEHICLE_MISC_A_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		VEHICLE_MISC_B_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	// extras
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};


static RwObjectNameIdAssocation boatIds[] = {
	{"boat_moving_hi", 		BOAT_MOVING, 			0},
	{"boat_rudder_hi",		BOAT_RUDDER, 			0},
	{"boat_flap_left",		BOAT_FLAP_LEFT,			0},
	{"boat_flap_right",		BOAT_FLAP_RIGHT,		0},
	{"boat_rearflap_left",	BOAT_REARFLAP_LEFT,		0},
	{"boat_rearflap_right",	BOAT_REARFLAP_RIGHT,	0},
	{"static_prop",			BOAT_STATIC_PROP,		VEHICLE_REAR|VEHICLE_UNIQUE_MATERIALS},
	{"moving_prop",			BOAT_MOVING_PROP,		VEHICLE_REAR|VEHICLE_ALPHA|VEHICLE_UNIQUE_MATERIALS},
	{"static_prop2",		BOAT_STATIC_PROP2,		VEHICLE_REAR|VEHICLE_UNIQUE_MATERIALS},
	{"moving_prop2",		BOAT_MOVING_PROP2,		VEHICLE_REAR|VEHICLE_ALPHA|VEHICLE_UNIQUE_MATERIALS},
	{"windscreen_hi_ok", 	BOAT_WINDSCREEN, 		VEHICLE_WINDSCREEN|VEHICLE_ALPHA},
	{"ped_frontseat",		BOAT_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	// extras
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};

static RwObjectNameIdAssocation trainIds[] = {
	{"door_lf_dummy",	TRAIN_DOORLHS, 		VEHICLE_CRUSH|VEHICLE_LEFT},
	{"door_rf_dummy",	TRAIN_DOORRHS,		VEHICLE_CRUSH|VEHICLE_RIGHT},
	{"wheel_rf1_dummy",	TRAIN_WHEEL_RF1,	VEHICLE_PARENT_WHEEL|VEHICLE_RIGHT},
	{"wheel_rf2_dummy",	TRAIN_WHEEL_RF2,	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rf3_dummy",	TRAIN_WHEEL_RF3,	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb1_dummy",	TRAIN_WHEEL_RR1,	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb2_dummy",	TRAIN_WHEEL_RR2,	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb3_dummy",	TRAIN_WHEEL_RR3,	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_lf1_dummy",	TRAIN_WHEEL_LF1,	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lf2_dummy",	TRAIN_WHEEL_LF2,	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lf3_dummy",	TRAIN_WHEEL_LF3,	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb1_dummy",	TRAIN_WHEEL_LR1,	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb2_dummy",	TRAIN_WHEEL_LR2,	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb3_dummy",	TRAIN_WHEEL_LR3,	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"bogie_front",		TRAIN_BOGIE_FRONT,	VEHICLE_PARENT_CLONE_ATOMIC},
	{"bogie_rear",		TRAIN_BOGIE_REAR,	VEHICLE_ADD_CLONE_ATOMIC},
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_left_entry",	TRAIN_FRONT_POS, 			VEHICLE_STORE_POSN|VEHICLE_DOOR|CLUMP_NO_FRAMEID},
	{"ped_mid_entry",	TRAIN_MID_POS, 				VEHICLE_STORE_POSN|VEHICLE_DOOR|CLUMP_NO_FRAMEID},
	{"ped_right_entry",	TRAIN_REAR_POS, 			VEHICLE_STORE_POSN|VEHICLE_DOOR|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};
	
static RwObjectNameIdAssocation fakeHeliIds[] = {
	{"chassis_dummy", 	FAKE_HELI_CHASSIS, 		VEHICLE_CRUSH},
	{"toprotor",		FAKE_HELI_TOPROTOR,		VEHICLE_ALPHA/*|VEHICLE_TOP*/},
	{"backrotor",		FAKE_HELI_REARROTOR,	VEHICLE_ALPHA},
	{"tail",			FAKE_HELI_TAIL,			0},
	{"topknot",			FAKE_HELI_TOPKNOT,		0},
	{"skid_left",		FAKE_HELI_SKID_LEFT,	0},
	{"skid_right",		FAKE_HELI_SKID_RIGHT,	0},
	{NULL,NULL}
};

static RwObjectNameIdAssocation fakePlaneIds[] = {
	{"wheel_front_dummy",	FAKE_PLANE_FRONTWHEELS,	0},
	{"wheel_rear_dummy",	FAKE_PLANE_REARWHEELS,	0},
	{"propeller",			FAKE_PLANE_PROPELLER,	VEHICLE_ALPHA|VEHICLE_FRONT},
	{"light_tailplane",		FAKE_PLANE_TAILLIGHT_POSN,	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"light_left",			FAKE_PLANE_LEFTLIGHT_POSN,	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"light_right",			FAKE_PLANE_RIGHTLIGHT_POSN,	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};


static RwObjectNameIdAssocation bikeIds[] = {
	{"chassis_dummy",	BIKE_CHASSIS,		0},
	{"forks_front",		BIKE_FORKS,			0},
	{"forks_rear",		BIKE_SWINGARM,		0},
	{"wheel_front",		BIKE_WHEEL_F,		0},
	{"wheel_rear",		BIKE_WHEEL_R,		0},
	{"mudguard",		BIKE_MUDGUARD_F,	0},
	{"handlebars",		BIKE_HANDLEBARS,	0},
	{"misc_a",			BIKE_MISC_A,		0},
	{"misc_b",			BIKE_MISC_B,		0},
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"hookup",			VEHICLE_HOOKUP_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"bargrip",			BIKE_BARGRIP_POSN, 			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		BIKE_MISC_A_POSN, 			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		BIKE_MISC_B_POSN, 			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};

// BMX is derived from Bike so follows the same basic structure (kinda)
static RwObjectNameIdAssocation bmxIds[] = {
	{"chassis_dummy",	BMX_CHASSIS,		0},
	{"forks_front",		BMX_FORKS,			0},
	{"forks_rear",		BMX_SWINGARM,		0},
	{"wheel_front",		BMX_WHEEL_F,		0},
	{"wheel_rear",		BMX_WHEEL_R,		0},
	{"handlebars",		BMX_HANDLEBARS,		0},
	{"chainset",		BMX_CHAINSET,		0},
	{"pedal_r",			BMX_PEDAL_L,		0},
	{"pedal_l",			BMX_PEDAL_R,		0},
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"hookup",			VEHICLE_HOOKUP_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"bargrip",			BIKE_BARGRIP_POSN, 			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		BIKE_MISC_A_POSN, 			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		BIKE_MISC_B_POSN, 			VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};


static RwObjectNameIdAssocation trailerIds[] = {
	{"chassis", 	CAR_CHASSIS, 	0},
	{"wheel_rf_dummy", 	CAR_WHEEL_RF, 	VEHICLE_PARENT_WHEEL|VEHICLE_RIGHT},
	{"wheel_rm_dummy", 	CAR_WHEEL_RM, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_rb_dummy", 	CAR_WHEEL_RR, 	VEHICLE_ADD_WHEELS|VEHICLE_RIGHT},
	{"wheel_lf_dummy", 	CAR_WHEEL_LF, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lm_dummy", 	CAR_WHEEL_LM, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"wheel_lb_dummy", 	CAR_WHEEL_LR, 	VEHICLE_ADD_WHEELS|VEHICLE_LEFT},
	{"door_rf_dummy", 	CAR_DOOR_RF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_FLAT|VEHICLE_FRONTDOOR},
	{"door_rr_dummy", 	CAR_DOOR_RR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_RIGHT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_REARDOOR},
	{"door_lf_dummy", 	CAR_DOOR_LF, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_FLAT|VEHICLE_FRONTDOOR},
	{"door_lr_dummy", 	CAR_DOOR_LR, 	VEHICLE_CRUSH|VEHICLE_DOOR|VEHICLE_LEFT|VEHICLE_REAR|VEHICLE_FLAT|VEHICLE_REARDOOR},
	{"bump_front_dummy",CAR_BUMPFRONT,	VEHICLE_CRUSH|VEHICLE_FRONT},
	{"bump_rear_dummy", CAR_BUMPREAR, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"wing_rf_dummy", 	CAR_WING_RF, 	VEHICLE_CRUSH/*|VEHICLE_RIGHT*/},
	{"wing_lf_dummy", 	CAR_WING_LF, 	VEHICLE_CRUSH/*|VEHICLE_LEFT*/},
	{"bonnet_dummy", 	CAR_BONNET, 	VEHICLE_CRUSH},
	{"boot_dummy", 		CAR_BOOT, 		VEHICLE_CRUSH|VEHICLE_REAR},
	{"windscreen_dummy", CAR_WINDSCREEN, 	VEHICLE_CRUSH|VEHICLE_ALPHA|VEHICLE_WINDSCREEN|VEHICLE_FRONT},
	{"exhaust_ok", 		CAR_EXHAUST, 	VEHICLE_CRUSH|VEHICLE_REAR},
	{"misc_a",			CAR_MISC_A,		VEHICLE_CRUSH},
	{"misc_b",			CAR_MISC_B,		VEHICLE_CRUSH},
	{"misc_c",			CAR_MISC_C,		VEHICLE_CRUSH},
	// positions
	{"ped_frontseat",	VEHICLE_FRONTSEAT_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_backseat",	VEHICLE_BACKSEAT_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights", 		VEHICLE_HEADLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights", 		VEHICLE_TAILLIGHTS_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"headlights2",		VEHICLE_HEADLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"taillights2",		VEHICLE_TAILLIGHTS2_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"exhaust",			VEHICLE_EXHAUST_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"engine",			VEHICLE_ENGINE_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"petrolcap",		VEHICLE_PETROLCAP_POSN, 	VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"hookup",			VEHICLE_HOOKUP_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"ped_arm",			VEHICLE_PEDARM_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_c",		VEHICLE_MISC_C_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_d",		VEHICLE_MISC_D_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_a",		VEHICLE_MISC_A_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	{"miscpos_b",		VEHICLE_MISC_B_POSN, 		VEHICLE_STORE_POSN|CLUMP_NO_FRAMEID},
	// extras
	{"extra1",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra2",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra3",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra4",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra5",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{"extra6",			0, 				VEHICLE_XTRACOMP|VEHICLE_ALPHA|CLUMP_NO_FRAMEID},
	{NULL,NULL}
};



VehicleDesc CVehicleModelInfo::ms_vehicleDescs[] = 
{
	carIds,
	truckIds,
	quadIds,
	heliIds,
	planeIds,
	boatIds,
	trainIds,
	fakeHeliIds,
	fakePlaneIds,
	bikeIds,
	bmxIds,
	trailerIds
};
	
static int32 wheelIds[] = {CAR_WHEEL_LF, CAR_WHEEL_LR, CAR_WHEEL_RF, CAR_WHEEL_RR};

static RwTextureCallBackFind gDefaultFindCB = NULL;

//
// Constructor
// 
CVehicleModelInfo::CVehicleStructure::CVehicleStructure()
{
	int32 i;
	// vehicle part positions
	for(i=0; i<VEHICLE_MAX_POSNS; i++)
		m_positions[i] = CVector(0.0f,0.0f,0.0f);

	// vehicle part positions
	for(i=0; i<VEH_UPGRADE_MAX; i++)
		m_upgrades[i].parentCompId = -1;

	for(i=0; i<MAX_NUM_XTRA_COMPONENTS; i++)
		m_pXtraAtomic[i] = NULL;
	m_numXtraAtomics = 0;
	m_nFlagDamageModelsAvailable = 0;
}


//
// Destructor
// 
CVehicleModelInfo::CVehicleStructure::~CVehicleStructure()
{
	// delete extra comps
	for(int32 i=0; i<m_numXtraAtomics; i++)
	{
		RwFrame* pFrame = RpAtomicGetFrame(m_pXtraAtomic[i]);
		RpAtomicDestroy(m_pXtraAtomic[i]);
		RwFrameDestroy(pFrame);
	}
}

void CVehicleModelInfo::ShutdownLightTexture()
{
	if (ms_pLightsTexture)
	{
		RwTextureDestroy(ms_pLightsTexture);
		ms_pLightsTexture = NULL;
	}

	if (ms_pLightsOnTexture)
	{
		RwTextureDestroy(ms_pLightsOnTexture);
		ms_pLightsOnTexture = NULL;
	}


}


//
// name:		AddUpgradeLink
// description:	Add an two upgrades to the link list
void CVehicleModelInfo::CLinkedUpgradeList::AddUpgradeLink(int16 m1, int16 m2)
{
	ASSERTMSG(m_number < NUM_LINKED_UPGRADES, "Reached maximum number of linked upgrades");
	m_model1[m_number] = m1;
	m_model2[m_number] = m2;
	m_number++;
}

//
// name:		FindOtherUpgrade
// description:	Return if another upgrade is linked to this one
int16 CVehicleModelInfo::CLinkedUpgradeList::FindOtherUpgrade(int16 modelId) const
{
	int32 i = m_number;
	while(i--)
	{
		if(m_model1[i] == modelId)
			return m_model2[i];
		if(m_model2[i] == modelId)
			return m_model1[i];
	}
	return -1;
}

//
// name:		LoadTextureCB
// description:	Texture load callback for loading a vehicle
RwTexture* CVehicleModelInfo::FindTextureCB(const char* pTexname)
{
	RwTexture* pTexture = RwTexDictionaryFindNamedTexture(ms_pCommonTextureDictionary, pTexname);
	if(pTexture == NULL)
	{
		pTexture = RwTexDictionaryFindNamedTexture(RwTexDictionaryGetCurrent(), pTexname);
		if(!strncmp(pTexname, "remap", 5))
		{
			if(pTexture == NULL)
			{
				// we may have changed the name of remap textures so we should search for the
				// altered name as well
				char warpedName[32];
				strcpy(warpedName, pTexname);
				warpedName[0] = REMAP_TEXTURE_IDENTIFIER;
				pTexture = RwTexDictionaryFindNamedTexture(RwTexDictionaryGetCurrent(), &warpedName[0]);
			}
			else
			{
	    		// Use # as a quick way of recognising a remap texture
	    		pTexture->name[0] = REMAP_TEXTURE_IDENTIFIER;
			}
		}
	}	
	return pTexture;
}

//
// name:		UseCommonVehicleTexDicationary
// description:	Setup callback for finding textures so that it also looks in the vehicle
//				common texture dictionary as well as the current texture dictionary
void CVehicleModelInfo::UseCommonVehicleTexDicationary()
{
	ASSERT(gDefaultFindCB == NULL);
	gDefaultFindCB = RwTextureGetFindCallBack();
	RwTextureSetFindCallBack(FindTextureCB);
}

//
// name:		UseCommonVehicleTexDicationary
// description:	Remove callback for finding textures in the vehicle common texture dictionary
void CVehicleModelInfo::StopUsingCommonVehicleTexDicationary()
{
	ASSERT(gDefaultFindCB);
	RwTextureSetFindCallBack(gDefaultFindCB);
	gDefaultFindCB = NULL;
}

//
// Constructor
// 
CVehicleModelInfo::CVehicleModelInfo()
{
	m_pStructure = NULL;
	m_numPossibleColours = 0;	
   	m_animFileIndex=-1;
   	m_flags = 0;
   	
   	for(int32 i=0; i<MAX_POSSIBLE_UPGRADES; i++)
   		m_upgradeModels[i] = -1;
   	for(int32 i=0; i<MAX_NUM_REMAPS; i++)
   		m_remaps[i] = -1;
}

//
// name:		SetAnimFile
// description:	Set anim file required when loading this model
void CVehicleModelInfo::SetAnimFile(const char* pName)
{
	if(!stricmp(pName, "null"))
	{
    	m_animFileIndex=-1;
		return;
	}
	// temporarily let animFileIndex be a pointer to the anim block
	m_animFileIndex = (int32)new char[strlen(pName)+1];
	strcpy((char*)m_animFileIndex, pName);
}

//
// name:		ConvertAnimFileIndex
// description:	Convert animfile name to an animfile index
void CVehicleModelInfo::ConvertAnimFileIndex()
{
	if(m_animFileIndex == -1)
		return;
	int32 index = CAnimManager::GetAnimationBlockIndex((char*)m_animFileIndex);
	
	delete[] (char*)m_animFileIndex;
	m_animFileIndex = index;
}


//
//        name: CVehicleModelInfo::SetClump
// description: Set pointer to clump and update render callbacks etc
//
void CVehicleModelInfo::SetClump(RpClump* pClump) 
{
	ASSERT(m_vehicleType != VEHICLE_TYPE_NONE);
	ASSERT(m_pStructure == NULL);
	
	m_pStructure = new CVehicleStructure;
	
	CClumpModelInfo::SetClump(pClump);
	SetAtomicRenderCallbacks();
	SetFrameIds(ms_vehicleDescs[m_vehicleType].pAssocArray);
	SetRenderPipelines();

	PreprocessHierarchy();
	
	ReduceMaterialsInVehicle();

	FindEditableMaterialList();

	SetCarCustomPlate();
}

//
//        name: CVehicleModelInfo::CreateInstance
// description: Creates a copy of the clump. Same as CClumpModelInfo::CreateInstance except it sets
// 				the colour of the vehicle
//
RwObject* CVehicleModelInfo::CreateInstance()
{
	RpClump* pClump = (RpClump*)CClumpModelInfo::CreateInstance();
	
	// create random extra if there are extra components to add and do it every 2 in 3 times
	if(m_pStructure->m_numXtraAtomics)
	{
		RwFrame* pParentFrame = NULL;
		// these two are effectively the same
		if(m_vehicleType==VEHICLE_TYPE_BIKE || m_vehicleType==VEHICLE_TYPE_BMX)
			pParentFrame = CVehicleModelInfo::GetFrameFromId(pClump, BIKE_CHASSIS);
		// these guys are derived from cars
		else if(m_vehicleType < VEHICLE_TYPE_BOAT)
			pParentFrame = CVehicleModelInfo::GetFrameFromId(pClump, CAR_CHASSIS);
		
		if(pParentFrame==NULL)
			pParentFrame = RpClumpGetFrame(pClump);

		int32 index = ChooseComponent();
		if(index != -1 && m_pStructure->m_pXtraAtomic[index])
		{
			RpAtomic* pAtomic = RpAtomicClone(m_pStructure->m_pXtraAtomic[index]);
			RwFrame* pFrame = RwFrameCreate();
			
			RwFrameTransform(pFrame, RwFrameGetMatrix(RpAtomicGetFrame(m_pStructure->m_pXtraAtomic[index])), rwCOMBINEREPLACE);
			RpAtomicSetFrame(pAtomic, pFrame);
			
			RpClumpAddAtomic(pClump, pAtomic);
			RwFrameAddChild(pParentFrame, pFrame);
		}
		ms_compsUsed[0] = index;	
		index = ChooseSecondComponent();
		if(index != -1 && m_pStructure->m_pXtraAtomic[index])
		{
			RpAtomic* pAtomic = RpAtomicClone(m_pStructure->m_pXtraAtomic[index]);
			RwFrame* pFrame = RwFrameCreate();
			
			RwFrameTransform(pFrame, RwFrameGetMatrix(RpAtomicGetFrame(m_pStructure->m_pXtraAtomic[index])), rwCOMBINEREPLACE);
			RpAtomicSetFrame(pAtomic, pFrame);
			
			RpClumpAddAtomic(pClump, pAtomic);
			RwFrameAddChild(pParentFrame, pFrame);
		}	
		ms_compsUsed[1] = index;
		
		// remove and add chassis first so that it drawn before the extras
		/*RwObjectNameAssoc nameAssoc = {"chassis_dummy", NULL};
		FindFrameFromNameCB(pParentFrame, &nameAssoc);
		if(nameAssoc.pObject)
		{
			RwFrame* pChassis = (RwFrame*)nameAssoc.pObject;
			RwFrame* pParent = RwFrameGetParent(pChassis);
			RwFrameRemoveChild(pChassis);
			RwFrameAddChild(pParent, pChassis);
		}*/
	}	
	else
	{
		ms_compsUsed[0] = -1;
		ms_compsUsed[1] = -1;
	}

// !PC - uncomment this to disable env maps
//	<calls the old env map setup stuff now.>
#if defined (GTA_PC)
	RpClumpForAllAtomics(pClump, &SetEnvironmentMapAtomicCB, NULL);
#endif //GTA_PC
	
	return (RwObject*)pClump;
}

#if defined (GTA_PC) || defined (GTA_XBOX)
#define REMOVE_ENVMAP	0xffff
void CVehicleModelInfo::DisableEnvMap()
{
	if (m_pRwObject)
	{
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &SetEnvironmentMapAtomicCB, (void*)REMOVE_ENVMAP);
	}
}

void CVehicleModelInfo::SetEnvMapCoeff(float coeff)
{
	UInt32	intCoeff = CMaths::Floor(coeff*1000);
	if (m_pRwObject)
	{
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &SetEnvMapCoeffAtomicCB, (void*)(intCoeff));
	}
}
#endif //GTA_PC & GTA_XBOX

//
// name:		DeleteRwObject
// description:	Deletes the associated RW clump and the Xtra components
void CVehicleModelInfo::DeleteRwObject()
{
	if(m_pStructure)
	{
		delete m_pStructure;
		m_pStructure = NULL;
	}	
	CClumpModelInfo::DeleteRwObject();
}

//
// CollapseFramesCB: Attach all objects to another frame and remove frame
//
RwFrame* CVehicleModelInfo::CollapseFramesCB(RwFrame* pFrame, void* pData)
{
	RwFrameForAllChildren(pFrame, &CollapseFramesCB, pData);
	RwFrameForAllObjects(pFrame, &MoveObjectsCB, pData);
	RwFrameDestroy(pFrame);
	
	return pFrame;
}

//
// MoveObjectsCB: move objects so they are attached to another frame. Gets called with 
//				pData = frame to move objects to
//
RwObject* CVehicleModelInfo::MoveObjectsCB(RwObject* pObj, void* pData)
{
	ASSERT(RwObjectGetType(pObj) == rpATOMIC);
	RpAtomicSetFrame((RpAtomic*)pObj, (RwFrame*)pData);
	
	return pObj;
}

//
// Hide damaged atomic callback function. Looks for "_dam" in name
//
RpAtomic* CVehicleModelInfo::HideDamagedAtomicCB(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	
	if(strstr(GetFrameNodeName(pFrame), "_dam")/*  &&
		!strstr(GetFrameNodeName(pFrame), "chassis")*/)
	{
		RpAtomicSetFlags(pAtomic, 0);
		CVisibilityPlugins::SetAtomicFlag(pAtomic, VEHICLE_ATOMIC_DAMAGED);
	}
	else if(strstr(GetFrameNodeName(pFrame), "_ok"))
		CVisibilityPlugins::SetAtomicFlag(pAtomic, VEHICLE_ATOMIC_OK);

	return pAtomic;
}

//
// Hide damaged atomic callback function. Looks for "_dam" in name
//
RpAtomic* CVehicleModelInfo::HideAllComponentsAtomicCB(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	
	if(CVisibilityPlugins::GetAtomicId(pAtomic) & (uint32)pData)
		RpAtomicSetFlags(pAtomic, 0);
	else
		RpAtomicSetFlags(pAtomic, rpATOMICRENDER);

	return pAtomic;
}

//
// HasAlphaMaterialCB: returns if material contains alpha
//
RpMaterial* CVehicleModelInfo::HasAlphaMaterialCB(RpMaterial* pMaterial, void* pData)
{
    const RwRGBA *pColour = RpMaterialGetColor(pMaterial);
    
    if( pColour->alpha != 255 )
    {
    	*(bool*)pData = true;
    	return NULL;
    }
    return pMaterial;
}

//
// Set the renderer used by the atomic depending on the name of the atomic
//
RpAtomic* CVehicleModelInfo::SetAtomicRendererCB(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	const char* pName = GetFrameNodeName(pFrame);
	
	if(strstr(pName, "_vlo"))
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleReallyLowDetailCB);
	else
	{
		bool hasAlpha = false;
		
		RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &HasAlphaMaterialCB, &hasAlpha);

		if(hasAlpha)
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailAlphaCB);
 		else if(!strncmp(pName, "windscreen", 10))
 			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailAlphaCB);
		else	
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailCB);
	}
	
	HideDamagedAtomicCB(pAtomic, NULL);

	return pAtomic;
}


//
// Set the renderer used by the atomic depending on the name of the atomic
//
RpAtomic* CVehicleModelInfo::SetAtomicRendererCB_RealHeli(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	const char* pName = GetFrameNodeName(pFrame);
	
	if(!strcmp(pName, "moving_rotor"))
	{
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderHeliRotorAlphaCB);
	}
	else if(!strcmp(pName, "moving_rotor2"))
	{
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderHeliTailRotorAlphaCB);
	}
	else if(strstr(pName, "_vlo"))
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleReallyLowDetailCB);
	else
	{
		bool hasAlpha = false;
		
		RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &HasAlphaMaterialCB, &hasAlpha);

		if(hasAlpha)
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailAlphaCB);
 		else if(!strncmp(pName, "windscreen", 10))
 			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailAlphaCB);
		else	
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailCB);
	}	
	
	HideDamagedAtomicCB(pAtomic, NULL);

	return pAtomic;
}


//
// Set the renderer used by the atomic depending on the name of the atomic
//
RpAtomic* CVehicleModelInfo::SetAtomicRendererCB_BigVehicle(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	const char* pName = GetFrameNodeName(pFrame);
	
	if(strstr(pName, "_vlo"))
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleReallyLowDetailCB_BigVehicle);
	else
	{
		bool hasAlpha = false;
		
		RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &HasAlphaMaterialCB, &hasAlpha);

		if(hasAlpha)
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailAlphaCB_BigVehicle);
		else	
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailCB_BigVehicle);
	}	

	HideDamagedAtomicCB(pAtomic, NULL);
	return pAtomic;
}


//
// Set the renderer used by the atomic depending on the name of the atomic
//
RpAtomic* CVehicleModelInfo::SetAtomicRendererCB_Boat(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	const char* pName = GetFrameNodeName(pFrame);
	bool hasAlpha = false;
	
	RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &HasAlphaMaterialCB, &hasAlpha);
	
	if(!strcmp(pName, "boat_hi"))
	{
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailCB_Boat);
	}
	else if(strstr(pName, "_vlo"))
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleLoDetailCB_Boat);
	else
	{
		if(hasAlpha)
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailAlphaCB_Boat);
		else
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleHiDetailCB_Boat);
	}	

	HideDamagedAtomicCB(pAtomic, NULL);
	return pAtomic;
}


//
// Set the renderer used by the atomic depending on the name of the atomic
//
RpAtomic* CVehicleModelInfo::SetAtomicRendererCB_Heli(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	const char* pName = GetFrameNodeName(pFrame);

	if(!strncmp(pName, "toprotor", 8))
	{
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderHeliRotorAlphaCB);
	}
	else if(!strncmp(pName, "rearrotor", 9))
	{
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderHeliTailRotorAlphaCB);
	}
	else
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, NULL);
		
	return pAtomic;
}


//
// Set the renderer used by the atomic depending on the name of the atomic
//
RpAtomic* CVehicleModelInfo::SetAtomicRendererCB_Train(RpAtomic* pAtomic, void* pData)
{
	RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
	const char* pName = GetFrameNodeName(pFrame);

	if(strstr(pName, "_vlo"))
		CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderVehicleReallyLowDetailCB_BigVehicle);
	else
	{
		bool hasAlpha = false;
		
		RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &HasAlphaMaterialCB, &hasAlpha);

		if(hasAlpha)
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderTrainHiDetailAlphaCB);
		else
			CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderTrainHiDetailCB);
	}	

	HideDamagedAtomicCB(pAtomic, NULL);
	return pAtomic;
}


//
// CVehicleModelInfo::SetAtomicRenderCB: Setup the render callbacks for the atomics
//			in this hierarchy
//
void CVehicleModelInfo::SetAtomicRenderCallbacks()
{
	if (GetVehicleClass() == VEHICLE_TYPE_TRAIN)
	{
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::SetAtomicRendererCB_Train, NULL);
	}
/*	else if (GetVehicleClass() == VEHICLE_TYPE_FAKE_HELI)
	{
		RpClumpForAllAtomics(m_pClump, &CVehicleModelInfo::SetAtomicRendererCB_Heli, NULL);
	}*/
	else if (GetVehicleClass() == VEHICLE_TYPE_FAKE_PLANE || GetVehicleClass() == VEHICLE_TYPE_PLANE)
	{
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::SetAtomicRendererCB_BigVehicle, NULL);
	}
	else if(GetVehicleClass() == VEHICLE_TYPE_BOAT)
	{
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::SetAtomicRendererCB_Boat, m_pRwObject);
	}
	else if(GetVehicleClass() == VEHICLE_TYPE_HELI)
	{
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::SetAtomicRendererCB_RealHeli, m_pRwObject);
	}
	else
	{
		RpClumpForAllAtomics((RpClump*)m_pRwObject, &CVehicleModelInfo::SetAtomicRendererCB, m_pRwObject);
	}
//	RpClumpForAllAtomics(m_pClump, &CVehicleModelInfo::HideDamagedAtomicCB, NULL);
}

//
// name:		CVehicleModelInfo::SetAtomicFlagCB
// description:	Set an atomic flag. 
//
RwObject* CVehicleModelInfo::SetAtomicFlagCB(RwObject* pObj, void* pData)
{
	CVisibilityPlugins::SetAtomicFlag((RpAtomic*)pObj, (int32)pData);
	return pObj;
}

//
// name:		CVehicleModelInfo::ClearAtomicFlagCB
// description:	Clear an atomic flag. 
//
RwObject* CVehicleModelInfo::ClearAtomicFlagCB(RwObject* pObj, void* pData)
{
	CVisibilityPlugins::ClearAtomicFlag((RpAtomic*)pObj, (int32)pData);
	return pObj;
}

//
// name:		CVehicleModelInfo::GetOkAndDamagedAtomicCB
// description:	Fill out Atomic array with OK and Damaged atomics
RwObject* GetOkAndDamagedAtomicCB(RwObject* pObj, void* pData)
{
	if(CVisibilityPlugins::GetAtomicId((RpAtomic*)pObj) & VEHICLE_ATOMIC_OK)
		((RpAtomic**)pData)[0] = (RpAtomic*)pObj;
	else if(CVisibilityPlugins::GetAtomicId((RpAtomic*)pObj) & VEHICLE_ATOMIC_DAMAGED)
		((RpAtomic**)pData)[1] = (RpAtomic*)pObj;

	return pObj;
}


//
// name:		CVehicleModelInfo::SetVehicleComponentFlags
// description:	Set flags for atomic
void CVehicleModelInfo::SetVehicleComponentFlags(RwFrame* pFrame, uint32 flags)
{
	tHandlingData* pHandling = mod_HandlingManager.GetPointer(m_handlingId);

	/*if(flags & VEHICLE_WINDSCREEN)
	{
		// windscreen on a rhino happens to be its turret
		if(this == CModelInfo::GetModelInfo(MODELID_CAR_RHINO))
			return;
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_WINDSCREEN);
	}*/	
	if(flags & VEHICLE_FLAT)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_FLAT);

	if(flags & VEHICLE_DONTCULL)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_DONTCULL);
		
	if(flags & VEHICLE_UNIQUE_MATERIALS)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_UNIQUE_MATERIALS);
		
	if(flags & VEHICLE_FRONT)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_FRONT);
	// If rear flag is set and vehicle is a van or the left and right flags are not set then
	// set component to be on the rear
	else if(flags & VEHICLE_REAR &&
			(pHandling->mFlags &MF_IS_VAN || 
			 !(flags & (VEHICLE_LEFT|VEHICLE_RIGHT))))
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_REAR);
	else if(flags & VEHICLE_LEFT)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_LEFT);
	else if(flags & VEHICLE_RIGHT)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_RIGHT);

	// hatchback boots and doors are flagged as objects whose render order is affect by how 
	// high the camera is
	if(flags & VEHICLE_TOP && 
		(pHandling->mFlags &MF_IS_HATCHBACK || 
			flags & (VEHICLE_REARDOOR|VEHICLE_FRONTDOOR)))
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_TOP);
		
	if(flags & VEHICLE_REARDOOR)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_REARDOOR);
	else if(flags & VEHICLE_FRONTDOOR)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_FRONTDOOR);
		
	if(flags & VEHICLE_ALPHA)
		RwFrameForAllObjects(pFrame, &SetAtomicFlagCB, (void*)VEHICLE_ATOMIC_ALPHA);
}

//
// PreprocessHierarchy: crush the RwFrame hierarchy. Remove non necessary frames
//
void CVehicleModelInfo::PreprocessHierarchy()
{
	VehicleDesc* pVehicle = &ms_vehicleDescs[m_vehicleType];
	tHandlingData* pHandling = mod_HandlingManager.GetPointer(m_handlingId);
	int32 i=0;
	RwFrame* pFrame;
	RwMatrix* pMatrix;
	RpAtomic* pAtomic;
	RpAtomic* pParentWheelAtomic = NULL;
	RpAtomic* pParentCloneAtomic = NULL;
	
	m_numDoors = 0;
	
	// do frame positions and xtra components first as these frames might be deleted by other 
	// preprocessing or confused with other frames as their ID's will have been set by SetFrameIds().
	while(pVehicle->pAssocArray[i].pName != NULL)
	{
		if(pVehicle->pAssocArray[i].flags & (VEHICLE_STORE_POSN|VEHICLE_UPGRADE_POSN|VEHICLE_XTRACOMP))
		{
			RwObjectNameAssoc nameAssoc;
			nameAssoc.pObject = NULL;
			nameAssoc.pName = pVehicle->pAssocArray[i].pName;
			RwFrameForAllChildren(RpClumpGetFrame((RpClump*)m_pRwObject), &FindFrameFromNameWithoutIdCB, &nameAssoc);
			if(nameAssoc.pObject)
			{
				if(pVehicle->pAssocArray[i].flags & VEHICLE_STORE_POSN)
				{
					pFrame = (RwFrame*)nameAssoc.pObject;
					pMatrix = RwFrameGetMatrix(pFrame);
					// get frame position
					m_pStructure->m_positions[pVehicle->pAssocArray[i].hierId] = *RwMatrixGetPos(pMatrix);
		
					// Multiple position by all the matrices in its hierarchy to get the position 
					// relative	to vehicle root
					pFrame = RwFrameGetParent(pFrame);
					if(pFrame)
					{
						// while the frame has a parent (ie the frame is not the root)
						while(RwFrameGetParent(pFrame))
						{
							pMatrix = RwFrameGetMatrix(pFrame);
							RwV3dTransformPoints(&m_pStructure->m_positions[pVehicle->pAssocArray[i].hierId], 
												&m_pStructure->m_positions[pVehicle->pAssocArray[i].hierId], 
												1, pMatrix);
							pFrame = RwFrameGetParent(pFrame);
						}
					}	
					// destory frame
					RwFrameDestroy((RwFrame*)nameAssoc.pObject);
				}
				else if(pVehicle->pAssocArray[i].flags & VEHICLE_UPGRADE_POSN)
				{
					// extract frame from hierarchy and atomic from clump
					pFrame = (RwFrame*)nameAssoc.pObject;
					
					const int32 id = pVehicle->pAssocArray[i].hierId;
					RwMatrix* pMatrix = RwFrameGetMatrix(pFrame);
					RwFrame* pParentFrame = RwFrameGetParent(pFrame);
					const int32 parentId = CVisibilityPlugins::GetFrameHierarchyId(pParentFrame);
					ASSERTOBJ(parentId>0, pVehicle->pAssocArray[i].pName, "Is not attached to correct hierarchy component");
					
					m_pStructure->m_upgrades[id].posn = *RwMatrixGetPos(pMatrix);
					m_pStructure->m_upgrades[id].quat.Set(*pMatrix);
					m_pStructure->m_upgrades[id].parentCompId = parentId;
				}
				else
				{
					// extract frame from hierarchy and atomic from clump
					pFrame = (RwFrame*)nameAssoc.pObject;
					pAtomic = (RpAtomic*)GetFirstObject(pFrame);
					ASSERT(pAtomic);
					RpClumpRemoveAtomic((RpClump*)m_pRwObject, pAtomic);
					RwFrameRemoveChild(pFrame);

					SetVehicleComponentFlags(pFrame, pVehicle->pAssocArray[i].flags);
					
					m_pStructure->m_pXtraAtomic[m_pStructure->m_numXtraAtomics++] = pAtomic;
				}
			}
		}
		
		// need to find wheel atomic in 1st loop too
		if(pVehicle->pAssocArray[i].flags &VEHICLE_PARENT_WHEEL
		|| pVehicle->pAssocArray[i].flags &VEHICLE_PARENT_CLONE_ATOMIC)
		{
			RwObjectIdAssoc assoc;
			assoc.pObject = NULL;
			assoc.id = pVehicle->pAssocArray[i].hierId;
			RwFrameForAllChildren(RpClumpGetFrame((RpClump*)m_pRwObject), &FindFrameFromIdCB, &assoc);
			if(assoc.pObject)
			{
				pFrame = (RwFrame*)assoc.pObject;
				while(pFrame)
				{
					if(GetFirstObject(pFrame))
					{
						if(pVehicle->pAssocArray[i].flags &VEHICLE_PARENT_WHEEL)
						pParentWheelAtomic = (RpAtomic*)GetFirstObject(pFrame);
						else
							pParentCloneAtomic = (RpAtomic*)GetFirstObject(pFrame);
						
						break;
					}
					
					pFrame = GetFirstChild(pFrame);
				}
			}
		}

		i++;
	}		
	
	i=0;
	while(pVehicle->pAssocArray[i].pName != NULL)
	{
		RwObjectIdAssoc assoc;
		
		// Ignore vehicle position associations as they have been dealt with already
		if(pVehicle->pAssocArray[i].flags & (VEHICLE_STORE_POSN|VEHICLE_UPGRADE_POSN|VEHICLE_XTRACOMP))
		{
			i++;
			continue;
		}	
			
		assoc.pObject = NULL;
		assoc.id = pVehicle->pAssocArray[i].hierId;
		RwFrameForAllChildren(RpClumpGetFrame((RpClump*)m_pRwObject), &FindFrameFromIdCB, &assoc);
		
		if(assoc.pObject)
		{
			// if door increment number of doors
			if(pVehicle->pAssocArray[i].flags & VEHICLE_DOOR)
				m_numDoors++;
			
			// if collapse hierarchy flag set
			if(pVehicle->pAssocArray[i].flags & VEHICLE_CRUSH)
			{
				RpAtomic* pAtomics[2]={NULL,NULL};
				
				RwFrameForAllChildren((RwFrame*)assoc.pObject, &CollapseFramesCB, assoc.pObject);
				RwFrameUpdateObjects((RwFrame*)assoc.pObject);
				
				// ensure the ok and damaged atomic both have the same render callback
				RwFrameForAllObjects((RwFrame*)assoc.pObject, &GetOkAndDamagedAtomicCB, &pAtomics[0]);
				if(pAtomics[0] && pAtomics[1])
				{
					RpAtomicSetRenderCallBack(pAtomics[1], RpAtomicGetRenderCallBack(pAtomics[0]));
					// flag that a damage atomic is actually available for this component
					m_pStructure->m_nFlagDamageModelsAvailable |= (1<<pVehicle->pAssocArray[i].hierId);
				}
			}

			SetVehicleComponentFlags((RwFrame*)assoc.pObject, pVehicle->pAssocArray[i].flags);
			
			if(pVehicle->pAssocArray[i].flags &(VEHICLE_ADD_WHEELS|VEHICLE_PARENT_WHEEL))
			{
				// if wheel is part of model
				if(pParentWheelAtomic)
				{
					if( !(pVehicle->pAssocArray[i].flags &VEHICLE_PARENT_WHEEL) )
					{
						pAtomic = RpAtomicClone(pParentWheelAtomic);
						RpAtomicSetFrame(pAtomic, (RwFrame*)assoc.pObject);
						RpClumpAddAtomic((RpClump*)m_pRwObject, pAtomic);
	
						CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderWheelAtomicCB);

						if(pVehicle->pAssocArray[i].hierId!=CAR_WHEEL_RF && pVehicle->pAssocArray[i].hierId!=CAR_WHEEL_LF
						&& pHandling->mFlags &MF_DOUBLE_REAR_WHEELS)
						{
							pAtomic = RpAtomicClone(pParentWheelAtomic);
							RwFrame *pExtraFrame = RwFrameCreate();
							RpAtomicSetFrame(pAtomic, pExtraFrame);
							RwFrameAddChild((RwFrame*)assoc.pObject,  pExtraFrame); 
							RwMatrix *pExtraMatrix = RwFrameGetMatrix(pExtraFrame);
							RwMatrixSetIdentity(pExtraMatrix);
							
							static float DOUBLE_WHEEL_POS_MULT = 1.15f;
							RwMatrixGetPos(pExtraMatrix)->x -= DOUBLE_WHEEL_POS_MULT*STD_CAR_WHEEL_WIDTH;//*m_wheelScaleRear/STD_CAR_WHEEL_SCALE;

							//RpAtomicSetFrame(pAtomic, (RwFrame*)assoc.pObject);
							RpClumpAddAtomic((RpClump*)m_pRwObject, pAtomic);
	
							CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderWheelAtomicCB);
						}
					}
					else
					{
						RwFrameForAllChildren((RwFrame*)assoc.pObject, &CollapseFramesCB, assoc.pObject);
						RwFrameUpdateObjects((RwFrame*)assoc.pObject);
						CVisibilityPlugins::SetAtomicRenderCallback(pParentWheelAtomic, CVisibilityPlugins::RenderWheelAtomicCB);
					}
				}
				// if using separate wheel model
/*				else if(m_wheelId != -1)
				{
					CAtomicModelInfo* pModel = (CAtomicModelInfo*)CModelInfo::GetModelInfo(m_wheelId);
					ASSERT(pModel->GetModelType() == MI_TYPE_ATOMIC);
					pAtomic = (RpAtomic*)pModel->CreateInstance();
					ASSERTMSG(pAtomic, "Cannot create wheels");
					RwFrameDestroy(RpAtomicGetFrame(pAtomic));
					RpAtomicSetFrame(pAtomic, (RwFrame*)assoc.pObject);
					RpClumpAddAtomic(m_pClump, pAtomic);
					
					CVisibilityPlugins::SetModelInfoIndex(pAtomic, -1);
					CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderWheelAtomicCB);
					
					// scale wheel
					RwV3d scale = {m_wheelScale, m_wheelScale, m_wheelScale};
					RwFrameScale((RwFrame*)assoc.pObject, &scale, rwCOMBINEPRECONCAT);
				}*/
// Might want wheel positions for suspension, without a wheel model				
//				else
//					RwFrameDestroy((RwFrame*)assoc.pObject);
			}
			else if(pVehicle->pAssocArray[i].flags &VEHICLE_ADD_CLONE_ATOMIC && pParentCloneAtomic)
			{
				pAtomic = RpAtomicClone(pParentCloneAtomic);
				RpAtomicSetFrame(pAtomic, (RwFrame*)assoc.pObject);
				RpClumpAddAtomic((RpClump*)m_pRwObject, pAtomic);
				CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, CVisibilityPlugins::RenderWheelAtomicCB);
			}
		}
		i++;
	}
	
/*	RwObjectIdAssoc assoc;
	
	assoc.pObject = NULL;
	assoc.id = CAR_ALLCOMPS;
	RwFrameForAllChildren(RpClumpGetFrame(m_pClump), &FindFrameFromIdCB, &assoc);
	if(assoc.pObject)
		RpClumpForAllAtomics(m_pClump, &HideAllComponentsAtomicCB, (void*)(VEHICLE_ATOMIC_DAMAGED|VEHICLE_ATOMIC_OK));
*/		
}

//
// GetWheelPosn::Return posn of wheel
// in: 	num = number of wheel
//		posn = vector to put posn in
//
void CVehicleModelInfo::GetWheelPosn(int32 num, CVector& posn, bool bSkipTransforms)
{
	RwFrame* pFrame;
	pFrame = GetFrameFromId((RpClump*)m_pRwObject, wheelIds[num]);
	ASSERT(pFrame);
	
	if(m_vehicleType==VEHICLE_TYPE_PLANE && !bSkipTransforms)
	{
		// need to transform matrix through it's parents to get final pos
		RwMatrix* pNodeLTM = RwMatrixCreate();
		RwMatrixCopy(pNodeLTM, RwFrameGetMatrix(pFrame));
		pFrame = RwFrameGetParent(pFrame);
		while(pFrame)
		{
			RwMatrixTransform(pNodeLTM, RwFrameGetMatrix(pFrame), rwCOMBINEPOSTCONCAT);
			pFrame = RwFrameGetParent(pFrame);
		}
				
		// get position vector and add offset for each semi-wheel
		posn = pNodeLTM->pos;
		// musn't forget to destroy the matrix we created
		RwMatrixDestroy(pNodeLTM);
	}
	else
	{
		RwMatrix* pMat = RwFrameGetMatrix(pFrame);
		posn.x = pMat->pos.x;
		posn.y = pMat->pos.y;
		posn.z = pMat->pos.z;
	}
}

bool CVehicleModelInfo::GetOriginalCompPosition(CVector& posn, int32 nId)
{
	RwFrame* pFrame;
	pFrame = GetFrameFromId((RpClump*)m_pRwObject, nId);
	if(pFrame)
	{
		RwMatrix* pMat = RwFrameGetMatrix(pFrame);
		posn.x = pMat->pos.x;
		posn.y = pMat->pos.y;
		posn.z = pMat->pos.z;
		
		return true;
	}
	
	ASSERTMSG(false, "CVehicleModelInfo::GetOriginalCompPosition() invalid id");
	return false;
}

#define FIRST_COMPRULE(m) 		((m&0xf000)>>12)
#define SECOND_COMPRULE(m) 		((m&0xf0000000)>>28)
#define COMPS_IN_FIRSTRULE(m)	(m&0xfff)
#define COMPS_IN_SECONDRULE(m)	((m>>16)&0xfff)
#define COMP_IN_FIRSTRULE(m,c)	((m>>(c*4))&0xf)
#define COMP_IN_SECONDRULE(m,c)	((m>>(c*4+16))&0xf)
#define COMP_IN_RULE(m,c)		((m>>(c*4))&0xf)

//
// name:		IsValidRule
// description:	Is this rule active at the moment. ie Is it raining etc
//
bool IsValidCompRule(int32 rule)
{
	switch(rule)
	{
	case VEHICLE_COMPRULE_NEEDINRAIN:
		if(CWeather::OldWeatherType == WEATHER_RAINY_COUNTRYSIDE || CWeather::NewWeatherType == WEATHER_RAINY_COUNTRYSIDE ||
		   CWeather::OldWeatherType == WEATHER_RAINY_SF || CWeather::NewWeatherType == WEATHER_RAINY_SF)
			return true;
		return false;
			
	default:
		return true;	
	}
}

//
// name:		GetListOfComponentsNotUsedByRules
// description:	Returns a list of components that aren't used by the rules. Function assumes there 
//				is a first rule
//
int32 GetListOfComponentsNotUsedByRules(uint32 mask, int32 numComps, int32* pArray)
{
	int32 value;
	int32 array[6] = {0,1,2,3,4,5};
	int32 i;
	int32 count=0;
	
	// if there is a first rule
	if(FIRST_COMPRULE(mask) && IsValidCompRule(FIRST_COMPRULE(mask)))
	{
		if(FIRST_COMPRULE(mask) == VEHICLE_COMPRULE_NEEDALL)
			return 0;	
			
		for(i=0; i<3; i++)
		{
			value = COMP_IN_FIRSTRULE(mask,i);
			if(value != 0xf)
				array[value] = 0xf;
		}
	}	
	// if there is a second rule
	if(SECOND_COMPRULE(mask) && IsValidCompRule(SECOND_COMPRULE(mask)))
	{
		for(i=0; i<3; i++)
		{
			value = COMP_IN_SECONDRULE(mask,i);
			if(value != 0xf)
				array[value] = 0xf;
		}		
	}
	
	for(i=0; i<numComps; i++)
	{
		if(array[i] != 0xf)
		{
			*pArray++ = i;
			count++;
		}
	}
	return count;
}

//
// name:		CountCompsInRule
// description:	returns the number of components used by this rule
//
int32 CountCompsInRule(int32 comps)
{
	int32 count=0;
	while(comps)
	{
		if((comps&0xf) != 0xf) 
			count++;
		comps >>= 4;
	}
	return count;
}

//
// name:		ChooseComponent
// description:	Choose a component given a rule and a component mask
//
int32 ChooseComponent(int32 rule, int32 comps)
{
	int32 numComps;
	int32 rnd;
	
	switch(rule)
	{
	// return one of comps in rule
	case VEHICLE_COMPRULE_NEED:
		numComps = CountCompsInRule(comps);
		rnd = CGeneral::GetRandomNumberInRange(0, numComps);
		return COMP_IN_RULE(comps, rnd);

	// return one of comps in rule if it is raining
	case VEHICLE_COMPRULE_NEEDINRAIN:
		numComps = CountCompsInRule(comps);
		rnd = CGeneral::GetRandomNumberInRange(0, numComps);
		return COMP_IN_RULE(comps, rnd);

	// maybe return one of comps in rule
	case VEHICLE_COMPRULE_ONEOFTHESE:
		numComps = CountCompsInRule(comps);
		rnd = CGeneral::GetRandomNumberInRange(-1, numComps);
		if(rnd == -1)
			return -1;
		return COMP_IN_RULE(comps, rnd);

	// return one of comps from all the comps , this is a special case for the flatbed which only has 5 extras now
	case VEHICLE_COMPRULE_NEEDALL:
		return CGeneral::GetRandomNumberInRange(0, 5);
		
	default:
		return -1;	
	}
	return -1;
}

//
// name:		ChooseComponent/ChooseSecondComponent
// description:	Use rule mask to select a component. If there aren't any rules just select number randomly
//
int32 CVehicleModelInfo::ChooseComponent()
{
	int32 compArray[6];
	int32 number;
	int32 index=-1;
	
	// if a specific component was requested
	if(ms_compsToUse[0] != -2)
	{
		index = ms_compsToUse[0];
		ms_compsToUse[0] = -2;
		return index;
	}	
		
	if(FIRST_COMPRULE(m_compRules) && IsValidCompRule(FIRST_COMPRULE(m_compRules)))
	{
		index = ::ChooseComponent(FIRST_COMPRULE(m_compRules), COMPS_IN_FIRSTRULE(m_compRules));
	}
	else if(CGeneral::GetRandomNumberInRange(0, 3) < 2)
	{
		number = GetListOfComponentsNotUsedByRules(m_compRules, m_pStructure->m_numXtraAtomics, &compArray[0]);
		if(number)
		{
			index = CGeneral::GetRandomNumberInRange(0, number);
			index = compArray[index];
		}	
	}	
	return index;
}
int32 CVehicleModelInfo::ChooseSecondComponent()
{
	int32 compArray[6];
	int32 number;
	int32 index=-1;
	
	// if a specific component was requested
	if(ms_compsToUse[1] != -2)
	{
		index = ms_compsToUse[1];
		ms_compsToUse[1] = -2;
		return index;
	}	
		
	if(SECOND_COMPRULE(m_compRules) && IsValidCompRule(SECOND_COMPRULE(m_compRules)))
	{
		index = ::ChooseComponent(SECOND_COMPRULE(m_compRules), COMPS_IN_SECONDRULE(m_compRules));
	}
	else if(FIRST_COMPRULE(m_compRules) && 
			IsValidCompRule(FIRST_COMPRULE(m_compRules)) && 
			CGeneral::GetRandomNumberInRange(0, 3) < 2)
	{
		number = GetListOfComponentsNotUsedByRules(m_compRules, m_pStructure->m_numXtraAtomics, &compArray[0]);
		if(number)
		{
			index = CGeneral::GetRandomNumberInRange(0, number);
			index = compArray[index];
		}	
	}	
	return index;
}

//
// name:		IsUpgradeAvailable
// description:	Return if there is position information available for this upgrade
bool CVehicleModelInfo::IsUpgradeAvailable(VehicleUpgradePosn upgrade)
{
	return (m_pStructure->m_upgrades[upgrade].parentCompId >= 0);
}

//!PC & !XBOX - vehicle colours are done differently to PS2 version - <sigh>.
#if defined (GTA_PC) || defined (GTA_XBOX)

static uint32 gStoredMaterials[512];
//
// name:		CVehicleModelInfo::SetEditableMaterialsCB()
// description:	Setup the editable materials for this vehicle
#define STORE_VALUE(p, v)	\
	*(*p)++ = (uint32)&v;	\
	*(*p)++ = *(uint32*)&v;

RpMaterial* CVehicleModelInfo::SetEditableMaterialsCB(RpMaterial* pMat, void* pData)
{
	uint32** ppData = (uint32**)pData;
	int32 index;
	uint32 matCol = (*(uint32*)&pMat->color) & 0xffffff;
	
	// check for remap textures for paint jobs
	if(ms_pRemapTexture)
	{
		if(pMat->texture && pMat->texture->name[0] == REMAP_TEXTURE_IDENTIFIER)
		{
			STORE_VALUE(ppData, pMat->texture);
			pMat->texture = ms_pRemapTexture;
		}
	}
	
	if(pMat->texture == ms_pLightsTexture)
	{
		index = -1;
		if(matCol == LIGHT_FRONT_LEFT_COLOUR)
			 index = FRONT_LEFT_LIGHT;
		else if(matCol == LIGHT_FRONT_RIGHT_COLOUR)
			 index = FRONT_RIGHT_LIGHT;
		else if(matCol == LIGHT_REAR_LEFT_COLOUR)
			 index = REAR_LEFT_LIGHT;
		else if(matCol == LIGHT_REAR_RIGHT_COLOUR)
			 index = REAR_RIGHT_LIGHT;
		// set material colour	 
		STORE_VALUE(ppData, pMat->color);
		*(uint32*)&pMat->color |= 0xffffff;
		if(index != -1 && ms_lightsOn[index])
		{
			STORE_VALUE(ppData, pMat->texture);
			STORE_VALUE(ppData, pMat->surfaceProps);
			pMat->texture = ms_pLightsOnTexture;
			pMat->surfaceProps = gLightSurfProps;
		}	
		return pMat;	
	}
	
	if(matCol == MAT1_COLOUR)
		 index = ms_currentCol[0];
	else if(matCol == MAT2_COLOUR)
		 index = ms_currentCol[1];
	else if(matCol == MAT3_COLOUR)
		 index = ms_currentCol[2];
	else if(matCol == MAT4_COLOUR)
		 index = ms_currentCol[3];
	else
		return pMat;

	{
		STORE_VALUE(ppData, pMat->color);
		pMat->color.red = ms_vehicleColourTable[index].red;
		pMat->color.green = ms_vehicleColourTable[index].green;
		pMat->color.blue = ms_vehicleColourTable[index].blue;
	}
	return pMat;
}

//
// name:		RemoveWindowAlphaCB
// description:	Used for removing windows when drivebys are happening
RpMaterial* RemoveWindowAlphaCB(RpMaterial* pMat, void* pData)
{
	uint32** ppData = (uint32**)pData;
	if(pMat->color.alpha < 255)
	{
		STORE_VALUE(ppData, pMat->color);
		*(uint32*)&pMat->color = 0;
	}
	return pMat;
}

//
// name:		CVehicleModelInfo::SetEditableMaterials()
// description:	Setup the editable materials for this atomic
RpAtomic* CVehicleModelInfo::SetEditableMaterialsCB(RpAtomic* pAtomic, void* pData)
{
	if(RpAtomicGetFlags(pAtomic) & rpATOMICRENDER)
	{
#ifndef FINAL
		const char* pName = GetFrameNodeName(RpAtomicGetFrame(pAtomic));
#endif
		if(CVisibilityPlugins::GetAtomicId(pAtomic) & VEHICLE_ATOMIC_DONTRENDERALPHA)
		{
			RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &RemoveWindowAlphaCB, pData);
		}
		RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &SetEditableMaterialsCB, pData);
	}
	return pAtomic;
}

//
// name:		CVehicleModelInfo::SetEditableMaterials()
// description:	Setup the editable materials for this vehicle
void CVehicleModelInfo::SetEditableMaterials(RpClump* pClump)
{
	uint32* pData = &gStoredMaterials[0];
	RpClumpForAllAtomics(pClump, &CVehicleModelInfo::SetEditableMaterialsCB, (void*)&pData);
	*pData = NULL;
}

//
// name:		CVehicleModelInfo::SetEditableMaterials()
// description:	Setup the editable materials for this vehicle
void CVehicleModelInfo::ResetEditableMaterials(RpClump* pClump)
{
	uint32* pData = &gStoredMaterials[0];
	while(*pData)
	{
		uint32* pValue = (uint32*)*pData++;
		*pValue = *pData++;
	}
}





//
// CVehicleModelInfo::GetEditableMaterialListCB: add materials to lists if they are changable
// 
RpMaterial* CVehicleModelInfo::GetEditableMaterialListCB(RpMaterial* pMaterial, void* pData)
{
	VehicleColourCBData* pColourData = (VehicleColourCBData*)pData;
	
	SECUROM_MARKER_HIGH_SECURITY_ON(2);

	RwTexture* pTex = RpMaterialGetTexture(pMaterial);
	if (pTex && (rwstrcmp(pTex->name,CARFX_DIRT_TEXTURE_NAME)==0))
	{
		//add this material to this vehicles dirt remap list for this vehicle (remap just before rendering)
		pColourData->pModelInfo->m_aDirtMaterials[pColourData->numDirt++] = pMaterial;
		ASSERTOBJ(pColourData->numDirt< MAX_PER_VEHICLE_DIRT_MATERIALS, pColourData->pModelInfo->GetModelName(), "Dirt material array is too small");
	}
	SECUROM_MARKER_HIGH_SECURITY_OFF(2);

/*	if (pTex && ((rwstrcmp(pTex->name,"vehiclelights128")==0)||(rwstrcmp(pTex->name,"vehiclelightson128")==0)))
	{
		if((RpMaterialGetColor(pMaterial)->blue << 16 | RpMaterialGetColor(pMaterial)->green << 8 |RpMaterialGetColor(pMaterial)->red) == LIGHT_FRONT_LEFT_COLOUR ||
		   (pMaterial->color.red == 254 && pMaterial->color.green == 255 && pMaterial->color.blue == 255))
		{
			pMaterial->color.red = 254;	
			pMaterial->color.green = 255;	
			pMaterial->color.blue = 255;	
			pMaterial->color.alpha = 255;	
	//		pColourData->pModelInfo->m_firstColour[pColourData->numMat1++] = pMaterial;
	//		CDebug::DebugLog("FL %d %d %d\n", pMaterial->color.red, pMaterial->color.green, pMaterial->color.blue);
	//		ASSERTOBJ(pColourData->numMat1 < MAX_PER_VEHICLE_MATERIALS, pColourData->pModelInfo->GetModelName(), "First material array is too small");
			if (ms_lightsOn[FRONT_LEFT_LIGHT])
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsOnTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightSurfProps);
			}
			else
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightOffSurfProps);
			}
		}else if((RpMaterialGetColor(pMaterial)->blue << 16 | RpMaterialGetColor(pMaterial)->green << 8 |RpMaterialGetColor(pMaterial)->red) == LIGHT_FRONT_RIGHT_COLOUR ||
				  (pMaterial->color.red == 255 && pMaterial->color.green == 255 && pMaterial->color.blue == 254))
		{
			pMaterial->color.red = 255;	
			pMaterial->color.green = 255;	
			pMaterial->color.blue = 254;	
			pMaterial->color.alpha = 255;	
	//		pColourData->pModelInfo->m_secondColour[pColourData->numMat2++] = pMaterial;
	//		CDebug::DebugLog("FR %d %d %d\n", pMaterial->color.red, pMaterial->color.green, pMaterial->color.blue);
	//		ASSERTOBJ(pColourData->numMat2 < MAX_PER_VEHICLE_MATERIALS2, pColourData->pModelInfo->GetModelName(), "Fourth material array is too small");
			if (ms_lightsOn[FRONT_RIGHT_LIGHT])
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsOnTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightSurfProps);
			}
			else
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightOffSurfProps);
			}
		}else if((RpMaterialGetColor(pMaterial)->blue << 16 | RpMaterialGetColor(pMaterial)->green << 8 |RpMaterialGetColor(pMaterial)->red) == LIGHT_REAR_LEFT_COLOUR ||
		   (pMaterial->color.red == 254 && pMaterial->color.green == 254 && pMaterial->color.blue == 254))
		{
			pMaterial->color.red = 254;	
			pMaterial->color.green = 254;	
			pMaterial->color.blue = 254;	
			pMaterial->color.alpha = 255;	
	//		pColourData->pModelInfo->m_thirdColour[pColourData->numMat3++] = pMaterial;
	//			CDebug::DebugLog("RL %d %d %d\n", pMaterial->color.red, pMaterial->color.green, pMaterial->color.blue);
	//		ASSERTOBJ(pColourData->numMat3 < MAX_PER_VEHICLE_MATERIALS3, pColourData->pModelInfo->GetModelName(), "Fourth material array is too small");
			if (ms_lightsOn[REAR_LEFT_LIGHT])
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsOnTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightSurfProps);
			}
			else
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightOffSurfProps);
			}
		}else if((RpMaterialGetColor(pMaterial)->blue << 16 | RpMaterialGetColor(pMaterial)->green << 8 |RpMaterialGetColor(pMaterial)->red) == LIGHT_REAR_RIGHT_COLOUR ||
		   (pMaterial->color.red == 255 && pMaterial->color.green == 254 && pMaterial->color.blue == 255))
		{
			pMaterial->color.red = 255;	
			pMaterial->color.green = 254;	
			pMaterial->color.blue = 255;	
			pMaterial->color.alpha = 255;	
	//		pColourData->pModelInfo->m_fourthColour[pColourData->numMat4++] = pMaterial;
	//		CDebug::DebugLog("RR %d %d %d\n", pMaterial->color.red, pMaterial->color.green, pMaterial->color.blue);
	//		ASSERTOBJ(pColourData->numMat4 < MAX_PER_VEHICLE_MATERIALS4, pColourData->pModelInfo->GetModelName(), "Fourth material array is too small");
			if (ms_lightsOn[REAR_RIGHT_LIGHT])
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsOnTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightSurfProps);
			}
			else
			{
				RpMaterialSetTexture(pMaterial, CVehicleModelInfo::ms_pLightsTexture);
				RpMaterialSetSurfaceProperties(pMaterial,  &gLightOffSurfProps);
			}
		}
	//	RwTexture* pTex = RpMaterialGetTexture(pMaterial);
	//	CDebug::DebugLog("FL %d FR %d RL: %d RR:%d %s\n", ms_lightsOn[FRONT_LEFT_LIGHT], ms_lightsOn[FRONT_RIGHT_LIGHT], ms_lightsOn[REAR_LEFT_LIGHT], ms_lightsOn[REAR_RIGHT_LIGHT], pTex->name);
		return pMaterial;
	}
		
	if(RpMaterialGetColor(pMaterial)->red == MAT1_COLOUR_RED &&
		RpMaterialGetColor(pMaterial)->green == MAT1_COLOUR_GREEN &&
		RpMaterialGetColor(pMaterial)->blue == MAT1_COLOUR_BLUE)
	{
		pColourData->pModelInfo->m_firstColour[pColourData->numMat1++] = pMaterial;
		ASSERTOBJ(pColourData->numMat1 < MAX_PER_VEHICLE_MATERIALS, pColourData->pModelInfo->GetModelName(), "First material array is too small");
	}
	else if(RpMaterialGetColor(pMaterial)->red == MAT2_COLOUR_RED &&
		RpMaterialGetColor(pMaterial)->green == MAT2_COLOUR_GREEN &&
		RpMaterialGetColor(pMaterial)->blue == MAT2_COLOUR_BLUE)
	{
		pColourData->pModelInfo->m_secondColour[pColourData->numMat2++] = pMaterial;
		ASSERTOBJ(pColourData->numMat2 < MAX_PER_VEHICLE_MATERIALS2, pColourData->pModelInfo->GetModelName(), "Second material array is too small");
	}
	else if(RpMaterialGetColor(pMaterial)->red == MAT3_COLOUR_RED &&
		RpMaterialGetColor(pMaterial)->green == MAT3_COLOUR_GREEN &&
		RpMaterialGetColor(pMaterial)->blue == MAT3_COLOUR_BLUE)
	{
		pColourData->pModelInfo->m_thirdColour[pColourData->numMat3++] = pMaterial;
		ASSERTOBJ(pColourData->numMat3 < MAX_PER_VEHICLE_MATERIALS3, pColourData->pModelInfo->GetModelName(), "Third material array is too small");

	}
	else if(RpMaterialGetColor(pMaterial)->red == MAT4_COLOUR_RED &&
		RpMaterialGetColor(pMaterial)->green == MAT4_COLOUR_GREEN &&
		RpMaterialGetColor(pMaterial)->blue == MAT4_COLOUR_BLUE)
	{
		pColourData->pModelInfo->m_fourthColour[pColourData->numMat4++] = pMaterial;
		ASSERTOBJ(pColourData->numMat4 < MAX_PER_VEHICLE_MATERIALS4, pColourData->pModelInfo->GetModelName(), "Fourth material array is too small");
	}
	*/
	return pMaterial;
}

//
// CVehicleModelInfo::GetEditableMaterialListCB: add materials to lists if they are changable
// 
RpAtomic* CVehicleModelInfo::GetEditableMaterialListCB(RpAtomic* pAtomic, void* pData)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	RpGeometryForAllMaterials(pGeom, &GetEditableMaterialListCB, pData);
	
	return pAtomic;	
}

#endif //GTA_PC & GTA_XBOX
//
// GetMaterialList: Fill out material lists.
//
static int32 maxNumMat1=0;
static int32 maxNumMat2=0;
static int32 maxNumMat3=0;
static int32 maxNumMat4=0;
static int32 maxNumDirt=0;
void CVehicleModelInfo::FindEditableMaterialList()
{
#if defined (GTA_PC) || defined (GTA_XBOX)
	VehicleColourCBData cbData;
	
	cbData.pModelInfo = this;
	cbData.numMat1 = 0;
	cbData.numMat2 = 0;
	cbData.numMat3 = 0;
	cbData.numMat4 = 0;
	cbData.numDirt = 0;
	RpClumpForAllAtomics(GetClump(), GetEditableMaterialListCB, &cbData);
	
	for(int32 i=0; i<m_pStructure->m_numXtraAtomics; i++)
		GetEditableMaterialListCB(m_pStructure->m_pXtraAtomic[i], &cbData);		
		
	m_firstColour[cbData.numMat1] = NULL;
	m_secondColour[cbData.numMat2] = NULL;
	m_thirdColour[cbData.numMat3] = NULL;
	m_fourthColour[cbData.numMat4] = NULL;
	if(cbData.numMat1 > maxNumMat1)
		maxNumMat1 = cbData.numMat1;
	if(cbData.numMat2 > maxNumMat2)
		maxNumMat2 = cbData.numMat2;
	if(cbData.numMat3 > maxNumMat3)
		maxNumMat3 = cbData.numMat3;
	if(cbData.numMat4 > maxNumMat4)
		maxNumMat4 = cbData.numMat4;
	if(cbData.numDirt> maxNumDirt)
		maxNumDirt= cbData.numDirt;

#endif //GTA_PC & GTA_XBOX
	
	m_lastCol[0] = 0xff;
	m_lastCol[1] = 0xff;
	m_lastCol[2] = 0xff;
	m_lastCol[3] = 0xff;
}

//
// CVehicleModelInfo::SetVehicleColour: set the colour of the first two materials for each geometry
// 
void CVehicleModelInfo::SetVehicleColour(uint8 col1, uint8 col2, uint8 col3, uint8 col4)
{
#if 1 //defined (GTA_PS2)
	ms_currentCol[0] = col1;
	ms_currentCol[1] = col2;
	ms_currentCol[2] = col3;
	ms_currentCol[3] = col4;
	m_lastCol[0] = col1;
	m_lastCol[1] = col2;
	m_lastCol[2] = col3;
	m_lastCol[3] = col4;
	return;
#else //GTA_PS2
	
	RpMaterial** pMat = &m_firstColour[0];

	RwRGBA color = ms_vehicleColourTable[col1];
	RwTexture* pTextureSet;

	if(col1 != m_lastCol[0])
	{
		pMat = &m_firstColour[0];
	
		color = ms_vehicleColourTable[col1];
		while(*pMat)
		{
			pTextureSet = RpMaterialGetTexture(*pMat);
			{
				(*pMat)->color.red = color.red;
				(*pMat)->color.green = color.green;
				(*pMat)->color.blue = color.blue;
			}

			pMat++;
		}
		m_lastCol[0] = col1;
	}
	
	if(col2 != m_lastCol[1])
	{
		pMat = &m_secondColour[0];

		color = ms_vehicleColourTable[col2];
		while(*pMat)
		{
			pTextureSet = RpMaterialGetTexture(*pMat);		
			{
				(*pMat)->color.red = color.red;
				(*pMat)->color.green = color.green;
				(*pMat)->color.blue = color.blue;
			}		
			pMat++;
		}
		m_lastCol[1] = col2;
	}
	
	if(col3 != m_lastCol[2])
	{
		pMat = &m_thirdColour[0];

		color = ms_vehicleColourTable[col3];
		while(*pMat)
		{
			pTextureSet = RpMaterialGetTexture(*pMat);		
			{
				(*pMat)->color.red = color.red;
				(*pMat)->color.green = color.green;
				(*pMat)->color.blue = color.blue;
			}		
			pMat++;
		}
		m_lastCol[2] = col3;
	}
	
	if(col4 != m_lastCol[3])
	{
		pMat = &m_fourthColour[0];

		color = ms_vehicleColourTable[col4];
		while(*pMat)
		{
			pTextureSet = RpMaterialGetTexture(*pMat);		
			{
				(*pMat)->color.red = color.red;
				(*pMat)->color.green = color.green;
				(*pMat)->color.blue = color.blue;
			}		
			pMat++;
		}
		m_lastCol[3] = col4;
	}
#endif //GTA_PS2	
}

#ifndef FINAL
void CVehicleModelInfo::SetVehicleColourOnTheFly(uint8 r, uint8 g, uint8 b, int32 index)
{
#if defined (GTA_PS2)
	ms_vehicleColourTable[m_lastCol[index]].red = r;
	ms_vehicleColourTable[m_lastCol[index]].green = g;
	ms_vehicleColourTable[m_lastCol[index]].blue = b;
	return;
#endif //GTA_PS2
//!PC - need to do colour shifting for vehicles here on PC rather than in the pipeline	
#if defined (GTA_PC)

	RpMaterial** pMat = &m_firstColour[0];
	RwTexture* pTextureSet;

	pMat = &m_firstColour[0];
	while(*pMat)
	{
		pTextureSet = RpMaterialGetTexture(*pMat);
		{
			(*pMat)->color.red = r;
			(*pMat)->color.green = g;
			(*pMat)->color.blue = b;
		}
		pMat++;
	}
	
	pMat = &m_secondColour[0];
	while(*pMat)
	{
		pTextureSet = RpMaterialGetTexture(*pMat);
		{
			(*pMat)->color.red = r;
			(*pMat)->color.green = g;
			(*pMat)->color.blue = b;
		}
		pMat++;
	}
	
#endif //GTA_PC
	
}


//
// name:		CreateCarColourTexture
// description:	Create a plain texture of the specified colour
//
void CVehicleModelInfo::UpdateCarColourTexture(uint8 r, uint8 g, uint8 b, int32 index)
{
#ifdef GTA_PS2		
/*
	RwImage* pImage = RwImageCreate(2,2,32);
	int32 w, h, d, f;
	uint8* pPixels = (uint8*)GtaMalloc(4*4);
	int32 i;
	RwTexture* pTexture;

	pTexture = ms_colourTextureTable[m_lastCol[index]];
	ASSERT(pTexture);
	RwRaster* pRaster = RwTextureGetRaster(pTexture);
	
	for(i=0; i<4; i++)
	{
		*(pPixels + i*4) = r;
		*(pPixels + i*4 + 1) = g;
		*(pPixels + i*4 + 2) = b;
		*(pPixels + i*4 + 3) = 255;
	}
	
	RwImageSetPixels(pImage, pPixels);
	RwImageSetStride(pImage, 8);
	RwImageFindRasterFormat(pImage, rwRASTERTYPETEXTURE, &w, &h, &d, &f);
	
	RwRasterSetFromImage(pRaster, pImage);
	
	// destroy image
	RwImageDestroy(pImage);
	GtaFree(pPixels);
	// this indicates this is a colour texture
	pTexture->name[0] = CARCOL_TEXTURE_IDENTIFIER;*/
#endif
	
}
#endif
//
// CVehicleModelInfo::ChooseVehicleColour: Choose two vehicle colours from the list of possible colours
// 
void CVehicleModelInfo::ChooseVehicleColour(uint8& col1, uint8& col2, uint8& col3, uint8& col4, int32 inc)
{
	if(m_numPossibleColours == 0 || CCheat::IsCheatActive(CCheat::BLACKCARS_CHEAT))
	{
		col1 = 0;
		col2 = 0;
		col3 = 0;
		col4 = 0;
		return;
	}
	
	if(CCheat::IsCheatActive(CCheat::PINKCARS_CHEAT))
	{
		col1 = 126;
		col2 = 126;
		col3 = 126;
		col4 = 126;
		return;
	}
	// just cycle colours instead of using random numbers
	m_lastColUsed = (m_lastColUsed + inc) % m_numPossibleColours;
	col1 = m_possibleColours[0][m_lastColUsed];
	col2 = m_possibleColours[1][m_lastColUsed];
	col3 = m_possibleColours[2][m_lastColUsed];
	col4 = m_possibleColours[3][m_lastColUsed];

	// If there are more than one set of possible colours	
	if(m_numPossibleColours > 1)
	{
		// If player vehicle is the same as this vehicle and has the current selected colours 
		// choose another colour
		CVehicle* pPlayerVehicle = FindPlayerVehicle();
		if(pPlayerVehicle && CModelInfo::GetModelInfo(pPlayerVehicle->GetModelIndex()) == this)
		{
			if(pPlayerVehicle->m_colour1 == col1 && 
				pPlayerVehicle->m_colour2 == col2 &&
				pPlayerVehicle->m_colour3 == col3 && 
				pPlayerVehicle->m_colour4 == col4)
			{
				m_lastColUsed = (m_lastColUsed + inc) % m_numPossibleColours;
				col1 = m_possibleColours[0][m_lastColUsed];
				col2 = m_possibleColours[1][m_lastColUsed];
				col3 = m_possibleColours[2][m_lastColUsed];
				col4 = m_possibleColours[3][m_lastColUsed];
			}
		}
	}	
}

//
// CVehicleModelInfo::AvoidSameVehicleColour: Try to give this car different colours from the ones used before
// 
/*void CVehicleModelInfo::AvoidSameVehicleColour(UInt8 *pCarCol1, UInt8 *pCarCol2, UInt8 *pCarCol3, UInt8 *pCarCol4)
{
	if(m_bBlackCarsCheat)
	{
		*pCarCol1 = 0;
		*pCarCol2 = 0;
		*pCarCol3 = 0;
		*pCarCol4 = 0;
		return;
	}
	else if(m_bPinkCarsCheat)
	{
		*pCarCol1 = 68;
		*pCarCol2 = 68;
		*pCarCol3 = 68;
		*pCarCol4 = 68;
		return;
	}
	
	if(m_numPossibleColours > 1)
	{
		Int32	index, Escape = 0;
		
		while (Escape < 8 && (*pCarCol1) == OldCol1 && (*pCarCol2) == OldCol2 && 
				(*pCarCol3) == OldCol3 && (*pCarCol4) == OldCol4)
		{
			index = CGeneral::GetRandomNumberInRange(0, m_numPossibleColours);
			(*pCarCol1) = m_possibleFirstColours[index];
			(*pCarCol2) = m_possibleSecondColours[index];
			(*pCarCol3) = m_possibleFirstColours[index];
			(*pCarCol4) = m_possibleSecondColours[index];
			Escape++;
		}
	}

	OldCol1 = (*pCarCol1);
	OldCol2 = (*pCarCol2);
	OldCol3 = (*pCarCol3);
	OldCol4 = (*pCarCol4);
	return;
}*/



//
// name:		RegisterRemapTexture
// description:	Check if TXD contains a remap texture
void CVehicleModelInfo::RegisterRemapTexture(const char* pName, int32 txdIndex)
{
	char name[24];
	// Vehicle Remap TXDs are the same name as the vehicle but have a number at the
	// end
	
	// has name got a digit at the end
	int32 len = strlen(pName);
	if(isdigit(*(pName+len-1)))
	{
		int32 end = len-2;
		while(isdigit(*(pName+end)))
			end--;
		end++;
		
		// copy name without number
		strncpy(name, pName, end);
		name[end] = '\0';
		
		// find vehicle
		CBaseModelInfo* pModel = CModelInfo::GetModelInfo(name, MODELID_CAR_FIRST, MODELID_CAR_LAST);
		// if model is a vehicle
		if(pModel && pModel->GetModelType() == MI_TYPE_VEHICLE)
			((CVehicleModelInfo*)pModel)->AddRemap(txdIndex);
	}
}

//
// name:		GetNumRemaps
// description:	Return number of remap textures
int32 CVehicleModelInfo::GetNumRemaps()
{
	int32 count=0;
	while(m_remaps[count] != -1 && count < MAX_NUM_REMAPS)
		count++;
	return count;	
}

//
// name:		AddRemap
// description:	Add a remap texture
void CVehicleModelInfo::AddRemap(int16 txdIndex)
{
	int32 index = GetNumRemaps();
	ASSERTOBJ(index < MAX_NUM_REMAPS, GetModelName(), "Max number of remap textures added");
	m_remaps[index] = txdIndex;
}

//
// name:		AddWheelUpgrade
// description:	Add wheel to upgrade class
void CVehicleModelInfo::AddWheelUpgrade(int32 wheelClass, int32 model)
{
	ASSERTMSG(ms_numWheelUpgrades[wheelClass] < NUM_WHEEL_UPGRADES, "Too many wheel upgrades in class");
	ms_upgradeWheels[wheelClass][ms_numWheelUpgrades[wheelClass]] = model;
	ms_numWheelUpgrades[wheelClass]++;
}
//
// name:		GetNumWheelUpgrades
// description:	Get number of wheels in upgrade class
int32 CVehicleModelInfo::GetNumWheelUpgrades(int32 wheelClass)
{
	return ms_numWheelUpgrades[wheelClass];
}
//
// name:		GetWheelUpgrade
// description:	Get wheel from upgrade class
int32 CVehicleModelInfo::GetWheelUpgrade(int32 wheelClass, int32 index)
{
	ASSERTMSG(index >= 0 && index < GetNumWheelUpgrades(wheelClass), "Wheel upgrade index is out of range");
	return ms_upgradeWheels[wheelClass][index];
}

//
// name:		DeleteVehicleColourTextures
// description:	Delete the vehicle colour textures
void CVehicleModelInfo::DeleteVehicleColourTextures()
{
/*	int32 i;
	for(i=0; i<NUM_VEHICLE_COLOURS; i++)
	{
		if(ms_colourTextureTable[i]){
			RwTextureDestroy(ms_colourTextureTable[i]);
			ms_colourTextureTable[i] = NULL;
		}
	}*/
}

// -- Environment map stuff -------------------------------------------------

static RwFrame* pMatFxIdentityFrame=NULL;
static bool initialised = false;

//
// Load Vehicle colour file "carcol.dat"
//
void CVehicleModelInfo::LoadEnvironmentMaps()
{
	char* environmentMapNames[6] = {"reflection01", "reflection02", "reflection03", 
									"reflection04", "reflection05", "reflection06"};
//	int32 i;
	
	// Load texture dictionary
	int32 particleTxdId = CTxdStore::FindTxdSlot("particle");
	ASSERT(particleTxdId != -1);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(particleTxdId);

/*	for(i=0; i<NUM_ENVIRON_MAPS; i++)
	{
		ms_pEnvironmentMaps[i] = RwTextureRead(environmentMapNames[i], NULL);
		ASSERTOBJ(ms_pEnvironmentMaps[i] != NULL, environmentMapNames[i], "Cannot load");
		RwTextureSetFilterMode(ms_pEnvironmentMaps[i], rwFILTERLINEAR);
	}*/
	if(gpWhiteTexture == NULL)
	{
		gpWhiteTexture = (RwTextureRead("white",NULL));
		gpWhiteTexture->name[0] = CARCOL_TEXTURE_IDENTIFIER;
		RwTextureSetFilterMode(gpWhiteTexture, rwFILTERLINEAR);
	}
	CTxdStore::PopCurrentTxd();
}

//
// name:		ShutdownEnvironmentMaps
// description:	Delete environment stuff
void CVehicleModelInfo::ShutdownEnvironmentMaps()
{
	initialised = false;
	
	RwTextureDestroy(gpWhiteTexture);
	gpWhiteTexture = NULL;
	
/*	for(int32 i=0; i<NUM_ENVIRON_MAPS; i++)
	{
		if(ms_pEnvironmentMaps[i]){
			RwTextureDestroy(ms_pEnvironmentMaps[i]);
			ms_pEnvironmentMaps[i] = NULL;
		}
	}*/

	if(pMatFxIdentityFrame)
		RwFrameDestroy(pMatFxIdentityFrame);
	pMatFxIdentityFrame = NULL;
}





// Andrzej: this code below is never used anywhere:
//
// name:		SetEnvironmentMapCB
// description:	callback functions that setup the environment maps
//
//
// HasSpecularMaterialCB: returns if material contains alpha
//
#if defined (GTA_PC) || defined (GTA_XBOX)
RpMaterial* CVehicleModelInfo::GetMatFXEffectMaterialCB(RpMaterial* pMaterial, void* pData)
{
	if(RpMatFXMaterialGetEffects(pMaterial) != rpMATFXEFFECTNULL)
	{
    	*(int32*)pData = RpMatFXMaterialGetEffects(pMaterial);
    	return NULL;
    }
    return pMaterial;
}

RpAtomic* CVehicleModelInfo::SetEnvironmentMapAtomicCB(RpAtomic* pAtomic, void* pData)
{
	RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &SetEnvironmentMapCB, pData);

	return pAtomic;
}

RpMaterial* CVehicleModelInfo::SetEnvironmentMapCB(RpMaterial* pMaterial, void* pData)
{
	extern RpLight *pDirect;
	
	//disable environment maps for now (on PC only)
	if (pData == (void*)REMOVE_ENVMAP)
	{
		RpMatFXMaterialSetEffects(pMaterial, rpMATFXEFFECTNULL);
		return (pMaterial);
	}
	 
	if(RpMatFXMaterialGetEffects(pMaterial) == rpMATFXEFFECTENVMAP)
	{
		RpMatFXMaterialSetEnvMapFrame(pMaterial, pMatFxIdentityFrame);
		if(RpMaterialGetTexture(pMaterial) == NULL)
			RpMaterialSetTexture(pMaterial, gpWhiteTexture);
		
		// hard wire this envmap coefficient for the time being (looks ok...)
		static float	coeff = 0.25f;
		RpMatFXMaterialSetEnvMapCoefficient(pMaterial, coeff);	
	}

	return pMaterial;
}

RpAtomic* CVehicleModelInfo::SetEnvMapCoeffAtomicCB(RpAtomic* pAtomic, void* pData)
{
	RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &SetEnvMapCoeffCB, pData);

	return pAtomic;
}

RpMaterial* CVehicleModelInfo::SetEnvMapCoeffCB(RpMaterial* pMaterial, void* pData)
{ 
	UInt32 coeff = (UInt32)pData;
	
	if(RpMatFXMaterialGetEffects(pMaterial) == rpMATFXEFFECTENVMAP)
	{
		RpMatFXMaterialSetEnvMapCoefficient(pMaterial, ((float)coeff / 1000.0f));	
	}

	return pMaterial;
}
#endif //GTA_PC & GTA_XBOX


//
//
//
//
RpAtomic* CVehicleModelInfo::SetRenderPipelinesCB(RpAtomic* pAtomic, void* pData)
{
//!PC - no custom renderer for PC
#if defined (GTA_PS2)
	return(CVuCarFXRenderer::SetCustomFXAtomicRenderPipelinesVMICB(pAtomic, pData));
#else
	return pAtomic;	
#endif //GTA_PS2
}


//
//
//
// name:		SetEnvironmentMap
// description:	Set the environment map for this vehicle
//
void CVehicleModelInfo::SetRenderPipelines()
{
#ifdef USE_PC_CARFX_RENDERER
	CCarFXRenderer::CustomCarPipeClumpSetup((RpClump*)m_pRwObject);
#endif


#ifdef USE_VU_CARFX_RENDERER
	// do nothing
#else
	#if defined (GTA_PS2)
		return;
	#endif //GTA_PS2 - just in case...
#endif

#ifndef USE_VU_CARFX_RENDERER
	// If identity matrix hasn't been made yet
	if(pMatFxIdentityFrame==NULL)
	{
		RwV3d axis = {1.0f, 0.0f, 0.0f};
		pMatFxIdentityFrame = RwFrameCreate();
		RwMatrixRotate(RwFrameGetMatrix(pMatFxIdentityFrame), &axis, 60.0f, rwCOMBINEREPLACE);
//		RwMatrixSetIdentity(RwFrameGetMatrix(pMatFxIdentityFrame));
		RwFrameUpdateObjects(pMatFxIdentityFrame);
		RwFrameGetLTM(pMatFxIdentityFrame);
	}
#endif



/*	// Andrzej: this clump setup is done already in CClumpModelInfo::SetClump():
#ifndef FINAL
	extern char *dbgLoadedModelName;
	// set name of the model we're going to load (this is for printing debug/assert info purposes only):
	dbgLoadedModelName = this->m_modelName;
#endif
	RpClumpForAllAtomics((RpClump*)m_pRwObject, &SetRenderPipelinesCB, NULL);
*/

/*	// Andrzej: Adam said this code is not necessary anymore:
	// set wheel environment map stuff
	if(m_wheelId != -1)
	{
		CAtomicModelInfo* pWheelModel = (CAtomicModelInfo*)CModelInfo::GetModelInfo(m_wheelId);
		SetRenderPipelinesCB(pWheelModel->GetAtomic(), NULL);
	}	
*/
}




//
//
//
//
void CVehicleModelInfo::SetCarCustomPlate()
{
	this->pCustomPlateMaterial	= NULL;
	this->SetCustomCarPlateText(NULL); 						// invalidate plate text:
	this->SetCustomCarPlateDesign(CARPLATE_DESIGN_DEFAULT);	// design created accordingly to where camera currently is
	
	char plateText[CUSTOM_PLATE_NUM_CHARS+1] = "DEFAULT";
	CCustomCarPlateMgr::GeneratePlateText(plateText, CUSTOM_PLATE_NUM_CHARS);
	
	RpMaterial *pMaterial = CCustomCarPlateMgr::SetupClump((RpClump*)m_pRwObject, plateText, this->GetCustomCarPlateDesign());
	if(pMaterial)
	{
		this->pCustomPlateMaterial = pMaterial;
	}
}





//
//
//
//
char* CVehicleModelInfo::GetCustomCarPlateText()
{
	if(this->tabCustomPlateText[0])
	{
		return(&this->tabCustomPlateText[0]);
	}
	else
	{
		return(NULL);
	}
}


//
//
// to be called sometime just before creation of vehicle:
//
void CVehicleModelInfo::SetCustomCarPlateText(char *pText)
{
	if(pText)
	{
		::strncpy(tabCustomPlateText, pText, CUSTOM_PLATE_NUM_CHARS);
	}
	else
	{
		// invalidate plate text:
		this->tabCustomPlateText[0] = 0;
	}
	
}




//
//
//
//
int32 CVehicleModelInfo::GetMaximumNumberOfPassengersFromNumberOfDoors(int32 model_index)
{
	UInt8 num_of_doors;
	CVehicleModelInfo *pModelInfo = (CVehicleModelInfo*)CModelInfo::GetModelInfo(model_index);
	
	int32 nHandlingIndex = pModelInfo->GetHandlingId();
	tHandlingData* pHandling = mod_HandlingManager.GetPointer(nHandlingIndex);
	
	ASSERTOBJ(pModelInfo->GetRwObject(), pModelInfo->GetModelName(), "Vehicle model is not loaded");

	if(pModelInfo->GetVehicleClass() == VEHICLE_TYPE_BOAT)
		return 0;
	else if(pModelInfo->GetVehicleClass()==VEHICLE_TYPE_TRAILER)
		return 0;
				
	if(model_index == MODELID_TRAIN_TRAIN)// special case for trains
		num_of_doors = 3;
	else if(model_index == MODELID_CAR_FIRETRUCK || model_index == MODELID_CAR_JOURNEY)
		num_of_doors = 2;
	else if(model_index == MODELID_HELI_HUNTER)
		num_of_doors = 1;
	else
		num_of_doors = pModelInfo->GetNumberOfDoors();
		
	if (num_of_doors == 0)
	{
		if(pModelInfo->GetVehicleClass() == VEHICLE_TYPE_BIKE
		|| pModelInfo->GetVehicleClass() == VEHICLE_TYPE_BMX
		|| pModelInfo->GetVehicleClass() == VEHICLE_TYPE_QUADBIKE
	 	|| pHandling->mFlags &MF_TANDEM_SEATING)
		{
			CVector vecSeatPos = pModelInfo->GetBackSeatPosn();
			if(vecSeatPos.x!=0.0f || vecSeatPos.y!=0.0f || vecSeatPos.z!=0.0f)
				return 1;
			else
				return 0;
		}
	 	else if(model_index == MODELID_CAR_RCBANDIT || model_index==MODELID_CAR_RCTIGER)
			return 0;
		else
			return 1;
	}
	else if(pHandling->mFlags &MF_TANDEM_SEATING)
	{
		//After subtracting off the two doors that lead to the driver seat there is 
		//only one seat for every two remaining doors.		
		return MAX(0, (num_of_doors-2)/2);
	}
	else
	{
		if(model_index == MODELID_CAR_COACH || model_index == MODELID_CAR_BUS)
			return MAX_PASSENGERS;
		else
			return (num_of_doors - 1);
	}
}


//
// name:		AreRpMaterialsEqual
// description:	Return if RpMaterials are equal
static
bool8 AreRpMaterialsEqual(RpMaterial* pMat1, RpMaterial* pMat2)
{
	// Is basic material different
	if( (pMat1->texture					!= pMat2->texture)					||
		(*(uint32*)&pMat1->color		!= *(uint32*)&pMat2->color)		 	||
		(pMat1->pipeline				!= pMat2->pipeline)					||
		(pMat1->surfaceProps.specular 	!= pMat2->surfaceProps.specular)	)
	{
		return(FALSE);
	}
	
bool8 bEnvPluginEqual = FALSE;

//!PC - assume that materials are false for PC?
#if defined (GTA_PS2)
	// Is custom environment map material different
	CustomEnvMapPipeMaterialData* pEnvMat1 = CCustomCVCarEnvMapPipeline::GetEnvMapPipeDataPtr(pMat1);
	CustomEnvMapPipeMaterialData* pEnvMat2 = CCustomCVCarEnvMapPipeline::GetEnvMapPipeDataPtr(pMat2);
	if(	(pEnvMat1 == pEnvMat2)	|| 
			(	pEnvMat1 && pEnvMat2 && 
				(pEnvMat1->cfEnvScaleX		== pEnvMat2->cfEnvScaleX)		&&
				(pEnvMat1->cfEnvScaleY		== pEnvMat2->cfEnvScaleY)		&&
				(pEnvMat1->cfEnvTransSclX	== pEnvMat2->cfEnvTransSclX)	&&
				(pEnvMat1->cfEnvTransSclY	== pEnvMat2->cfEnvTransSclY)	&&
				(pEnvMat1->cfShininess 		== pEnvMat2->cfShininess)		&&
				(pEnvMat1->pEnvTexture 		== pEnvMat2->pEnvTexture)		)	)
	{
		bEnvPluginEqual = TRUE;
	}

	if(bEnvPluginEqual)
	{
		// Is custom specular map material different
		CustomSpecMapPipeMaterialData* pSpecMat1 = CCustomCVCarEnvMapPipeline::GetSpecularPipeDataPtr(pMat1);
		CustomSpecMapPipeMaterialData* pSpecMat2 = CCustomCVCarEnvMapPipeline::GetSpecularPipeDataPtr(pMat2);
		if(	(pSpecMat1 == pSpecMat1)	|| 
				(	pSpecMat1 && pSpecMat1 && 
					(pSpecMat1->fSpecularity	== pSpecMat1->fSpecularity)		&&
					(pSpecMat1->pSpecularTexture== pSpecMat1->pSpecularTexture)	)	)
		{
			return(TRUE);
		}
	}
	
#endif //GTA_PS2		
	return(FALSE);
}

//
// name:		AddUniqueMaterialToList
// description:	Add a material to a list if a version of it is not already in the list
static RpMaterial* AddUniqueMaterialToList(RpMaterialList* pMatList, RpMaterial* pMat)
{
	// search list for material that is the same
	for(int32 i=0; i<pMatList->numMaterials; i++)
	{
		if(AreRpMaterialsEqual(pMatList->materials[i], pMat))
		{
			return(pMatList->materials[i]);
		}
	}

	// If we couldn't find one then add it to the list
	_rpMaterialListAppendMaterial(pMatList, pMat);
	
	return(NULL);
}

//
// name:		UseReducedMaterialListCB
// description:	Atomic callback adding materials to unique list or using materials from list
static RpAtomic* UseReducedMaterialListCB(RpAtomic* pAtomic, void* pData)
{
	RpMaterialList* pMatList = (RpMaterialList*)pData;
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	RpMesh* pMesh = (RpMesh*)(pGeom->mesh + 1);

	if(CVisibilityPlugins::GetAtomicId(pAtomic) & VEHICLE_ATOMIC_UNIQUE_MATERIALS)
		return pAtomic;
		
	// Attempt to use materials in unique list
	for(int32 i=0; i<pGeom->mesh->numMeshes; i++)
	{
		RpMaterial* pMat = AddUniqueMaterialToList(pMatList, pMesh->material);
		// if found material already in list
		if(pMat)
		{
			ASSERT(pMesh->material->refCount>0);
			int32 index = _rpMaterialListFindMaterialIndex(&pGeom->matList, pMesh->material);
			ASSERT(index != pGeom->matList.numMaterials);
			ASSERT(pMesh->material == pGeom->matList.materials[index]);

			// destroy material
			RpMaterialDestroy(pMesh->material);
			
			// setup new material pointers
			pGeom->matList.materials[index] = pMat;
			pMesh->material = pMat;
			
			// reference material so it isn't deleted at the wrong time
			RpMaterialAddRef(pMat);
		}
		pMesh++;
	}	
	return pAtomic;
}

//
// name:		ReduceMaterialsInVehicle
// description:	Reduce the number of materials in a vehicle
void CVehicleModelInfo::ReduceMaterialsInVehicle()
{
	RpMaterialList matList;
	RpClump* pClump = GetClump();
	
	CMemoryMgr::LockScratchPad();
	CMemoryMgr::InitScratchPad();
	// setup material list
	//_rpMaterialListInitialize(&matList);
	//_rpMaterialListSetSize(&matList, 20);
	// Initialise matlist with 
	matList.space = 20;

//!PC - can't alloc from scratch pad on pc
#if defined (GTA_PS2)
	matList.materials = (RpMaterial **)CMemoryMgr::MallocFromScratchPad(sizeof(RpMaterial *) * 20);
#else  //GTA_PS2
	matList.materials = (RpMaterial **)GtaMalloc(sizeof(RpMaterial *) * 20);
#endif //GTA_PS2

	matList.numMaterials = 0;
	
	uint32 time = CTimer::GetCurrentTimeInCycles();
	
	RpClumpForAllAtomics(pClump, &UseReducedMaterialListCB, &matList);
	for(int32 i=0; i<m_pStructure->m_numXtraAtomics; i++)
	{
		ASSERT(m_pStructure->m_pXtraAtomic[i]);
		UseReducedMaterialListCB(m_pStructure->m_pXtraAtomic[i], &matList);
	}	
	
	time = CTimer::GetCurrentTimeInCycles() - time;
	
	float timef = time/(float)CTimer::GetCyclesPerMillisecond();
	
	DEBUGLOG1("Reduce materials in %f milliseconds\n", timef);
	// destroy material list
	_rpMaterialListDeinitialize(&matList);
	
#if !defined (GTA_PS2)
//	GtaFree(matList.materials);
#endif //!GTA_PS2

	CMemoryMgr::ReleaseScratchPad();
	
	// this flag is not required anymore after this point, so clear its position
	//  (place is shared with VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES_LOD):
	CVisibilityPlugins::ClearClumpForAllAtomicsFlag(pClump, VEHICLE_ATOMIC_UNIQUE_MATERIALS);	
}

//
// name:		SetupLightFlags
// description: setup the flags for which lights are on
void CVehicleModelInfo::SetupLightFlags(CVehicle* pVehicle)
{
/*	ms_lightsOn[0] = false;
	ms_lightsOn[1] = false;
	ms_lightsOn[2] = false;
	ms_lightsOn[3] = false;
	if(pVehicle->m_nVehicleFlags.bEngineOn)
	{
		if(pVehicle->m_nVehicleFlags.bLightsOn)*/
		{
			// Rhs or Main Headlight
			ms_lightsOn[FRONT_RIGHT_LIGHT] = pVehicle->m_bLightOnFR;
			// Optional Lhs Headlight
			ms_lightsOn[FRONT_LEFT_LIGHT] = pVehicle->m_bLightOnFL;
			// Rhs or Main Taillight
			ms_lightsOn[REAR_RIGHT_LIGHT] = pVehicle->m_bLightOnRR;
			// Optional Lhs Taillight
			ms_lightsOn[REAR_LEFT_LIGHT] = pVehicle->m_bLightOnRL;
		}/*
		else if(pVehicle->m_fBrakePedal > 0.0f && pVehicle->m_fGasPedal >= 0.0f)
		{
			// Rhs or Main Taillight
			ms_lightsOn[REAR_RIGHT_LIGHT] = pVehicle->m_bUseRR;
			// Optional Lhs Taillight
			ms_lightsOn[REAR_LEFT_LIGHT] = pVehicle->m_bUseRL;
		}	
	}*/
}
