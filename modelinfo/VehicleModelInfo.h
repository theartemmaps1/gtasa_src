//
//
//    Filename: VehicleModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a vehicle model
//
//
#ifndef INC_VEHICLE_MODELINFO_H_
#define INC_VEHICLE_MODELINFO_H_

#ifdef GTA_PS2
// SCE headers
//$DW$#include <eekernel.h>
#endif
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "ClumpModelInfo.h"
#include "Game.h"
#include "Pool.h"


#define CARCOL_TEXTURE_IDENTIFIER	'@'
#define REMAP_TEXTURE_IDENTIFIER	'#'
#define LIGHTS_TEXTURE_IDENTIFIER	'~'

#define VEHICLE_LIGHTS_TEXTURE		"vehiclelights128"
#define VEHICLE_LIGHTSON_TEXTURE	"vehiclelightson128"

#if !defined (GTA_PS2)
#define MAX_PER_VEHICLE_MATERIALS	48
#define MAX_PER_VEHICLE_MATERIALS2	32
#define MAX_PER_VEHICLE_MATERIALS3	16
#define MAX_PER_VEHICLE_MATERIALS4	16
#define MAX_PER_VEHICLE_DIRT_MATERIALS	32
#define NUM_DIRT_LEVELS		16
#endif //!GTA_PS2

#define MAX_POSSIBLE_COLOURS		8
#define MAX_POSSIBLE_UPGRADES		18
#define NUM_LINKED_UPGRADES			30
#define NUM_VEHICLE_COLOURS			128
#define MAX_NUM_XTRA_COMPONENTS		6
#define MAX_NUM_REMAPS				4
#define NUM_ENVIRON_MAPS			1
#define NUM_WHEEL_UPGRADE_CLASSES	4
#define NUM_WHEEL_UPGRADES			15

#define INVALID_STEER_ANGLE 999.99f


#define CUSTOM_PLATE_NUM_CHARS		(8)

#define MAT1_COLOUR_RED		0x3c
#define MAT1_COLOUR_GREEN	0xff
#define MAT1_COLOUR_BLUE	0
#define MAT2_COLOUR_RED		0xff
#define MAT2_COLOUR_GREEN	0
#define MAT2_COLOUR_BLUE	0xaf
#define MAT3_COLOUR_RED		0x0
#define MAT3_COLOUR_GREEN	0xff
#define MAT3_COLOUR_BLUE	0xff
#define MAT4_COLOUR_RED		0xff
#define MAT4_COLOUR_GREEN	0
#define MAT4_COLOUR_BLUE	0xff

#define MAT1_COLOUR			0x00ff3c
#define MAT2_COLOUR			0xaf00ff
#define MAT3_COLOUR			0xffff00
#define MAT4_COLOUR			0xff00ff

#define LIGHT_FRONT_LEFT_COLOUR		0x00afff
#define LIGHT_FRONT_RIGHT_COLOUR	0xc8ff00
#define LIGHT_REAR_LEFT_COLOUR		0x00ffb9
#define LIGHT_REAR_RIGHT_COLOUR		0x003cff

enum {
	VEHICLE_MODINFO_FLAGS_STREAM_OUT_QUICKLY = 0,	// This vehicle will be streamed out after appearing only once or twice.
//	VEHICLE_MODINFO_FLAGS_TRY_TO_STREAM_OUT,		// We are now not creating new ones of these and we try to stream it out.
	};

// hierarchy type
enum VehicleType {			// Add new types at the end.
	VEHICLE_TYPE_NONE = -1,
	VEHICLE_TYPE_CAR = 0,
	VEHICLE_TYPE_MONSTERTRUCK,	// derived from autmobile
	VEHICLE_TYPE_QUADBIKE,		// derived from autmobile
	VEHICLE_TYPE_HELI,			// derived from autmobile
	VEHICLE_TYPE_PLANE,			// derived from autmobile
	VEHICLE_TYPE_BOAT,
//	VEHICLE_TYPE_SEAPLANE,		// derived from boat (not sure this's required)
	VEHICLE_TYPE_TRAIN,
	VEHICLE_TYPE_NOT_USED,		// Used to be VEHICLE_TYPE_FAKE_HELI
	VEHICLE_TYPE_FAKE_PLANE,
	VEHICLE_TYPE_BIKE,
	VEHICLE_TYPE_BMX,
	VEHICLE_TYPE_TRAILER
};

// atomic type
enum VehicleAtomicType
{
	VEHICLE_ATOMIC_NONE						= 0,
	VEHICLE_ATOMIC_OK 						= (1<<0),
	VEHICLE_ATOMIC_DAMAGED 					= (1<<1),
	VEHICLE_ATOMIC_MASK 					= 3,
	VEHICLE_ATOMIC_LEFT 					= (1<<2),	// the following four flags define where on the vehicle
	VEHICLE_ATOMIC_RIGHT 					= (1<<3),	// a component is. This is used to order the drawing
	VEHICLE_ATOMIC_FRONT		 			= (1<<4),	// of components with transparency e.g doors, windscreens
	VEHICLE_ATOMIC_REAR 					= (1<<5),
	VEHICLE_ATOMIC_ALPHA 					= (1<<6),	// this components drawing is always posponed
	VEHICLE_ATOMIC_FLAT 					= (1<<7),	// this component is flat and can easily be hidden when not facing the camera
	VEHICLE_ATOMIC_REARDOOR 				= (1<<8),	// this component is a rear door
	VEHICLE_ATOMIC_FRONTDOOR			 	= (1<<9),	// this component is a front door
	VEHICLE_ATOMIC_DONTCULL 				= (1<<10),	// dont cull this component 
	VEHICLE_ATOMIC_UPGRADE 					= (1<<11),	// this is an upgrade
	VEHICLE_ATOMIC_DONTRENDERALPHA			= (1<<12),	// flag to stop alpha being rendered (eg switch off windows)
	VEHICLE_ATOMIC_UNIQUE_MATERIALS 		= (1<<13),	// Don't attempt to merge materials on this atomic
	VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES_LOD	= (1<<13),	// Like VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES, but set accordingly to vehicle's distance to camera
	VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES		= (1<<14),	// render pipe for this atomic should not draw 2nd (envmap) and/or 3rd (spec) pass
														// useful when vehicle blew up and should be rendered in simplified form (1 pass visible only)
	VEHICLE_ATOMIC_TOP	= (1<<15)
};


// hierarchy flags
//#define CLUMP_NO_FRAMEID				// don't store frame id
#define VEHICLE_CRUSH			(0x2)	// remove hierarchy under this component
#define VEHICLE_ADD_WHEELS		(0x4) 	// add a wheel to this component
#define VEHICLE_STORE_POSN		(0x8)	// store position
#define VEHICLE_DOOR			(0x10)	// this component is a door. Used to calculate number of passengers
#define VEHICLE_LEFT			(0x20)	// the following four flags define where on the vehicle
#define VEHICLE_RIGHT			(0x40)	// a component is. This is used to order the drawing
#define VEHICLE_FRONT			(0x80)	// of components with transparency e.g doors, windscreens
#define VEHICLE_REAR			(0x100)
#define VEHICLE_XTRACOMP		(0x200)
#define VEHICLE_ALPHA			(0x400)	// this component is always drawn last
#define VEHICLE_WINDSCREEN		(0x800) // this component isn't drawn when in first person mode
#define VEHICLE_FLAT			(0x1000) // this component is flat and can easily be hidden when not facing the camera
#define VEHICLE_REARDOOR		(0x2000) // this component is flat and can easily be hidden when not facing the camera
#define VEHICLE_FRONTDOOR		(0x4000) // this component is flat and can easily be hidden when not facing the camera
#define VEHICLE_TOP				(0x8000) // this component is above the vehicle (for drawing order)
#define VEHICLE_PARENT_WHEEL	(0x10000)	// this is the wheel to clone to the other wheels
#define VEHICLE_UPGRADE_POSN	(0x20000)	// Vehicle upgrade position
#define VEHICLE_UNIQUE_MATERIALS	(0x40000)	// Component has unique materials
#define VEHICLE_NO_DAMAGE_MODEL		(0x80000)	// component has no damage model (but probably should)
#define VEHICLE_PARENT_CLONE_ATOMIC	(0x100000)	// for trains we want to clone and copy the bogeys
#define VEHICLE_ADD_CLONE_ATOMIC	(0x200000)	// after we've added the wheels
#define VEHICLE_DONTCULL		(0x400000)	// don't attempt to cull this object

// Rules about what components should be generated
#define VEHICLE_COMPRULE_NEED		(1)		// vehicle requires one of these components
#define VEHICLE_COMPRULE_NEEDINRAIN	(2)		// vehicle requires one of these components when it is raining
#define VEHICLE_COMPRULE_ONEOFTHESE	(3)		// vehicle can use one of these components but doesnt have to
#define VEHICLE_COMPRULE_NEEDALL	(4)		// vehicle requires one of all the components

// Positions stored for each vehicle
enum VehiclePartPosn {
	VEHICLE_HEADLIGHTS_POSN = 0,
	VEHICLE_TAILLIGHTS_POSN,
	VEHICLE_HEADLIGHTS2_POSN,
	VEHICLE_TAILLIGHTS2_POSN,
	VEHICLE_FRONTSEAT_POSN,
	VEHICLE_BACKSEAT_POSN,
	VEHICLE_EXHAUST_POSN,
	VEHICLE_ENGINE_POSN,
	VEHICLE_PETROLCAP_POSN,
	VEHICLE_HOOKUP_POSN,
	VEHICLE_PEDARM_POSN,
	VEHICLE_MISC_C_POSN,	// these are out of order because I needed
	VEHICLE_MISC_D_POSN,	// extra spots for the aeroplane positions
	VEHICLE_MISC_A_POSN,
	VEHICLE_MISC_B_POSN,
	VEHICLE_MAX_POSNS,
	
//	TRAIN_HEADLIGHTS_POSN = 0,
//	TRAIN_REARLIGHTS_POSN,
//	TRAIN_FRONTSEAT
	TRAIN_FRONT_POS = VEHICLE_HOOKUP_POSN,
	TRAIN_MID_POS,
	TRAIN_REAR_POS,
	TRAIN_MAX_POSNS,

	PLANE_AILERON_POSN = VEHICLE_HOOKUP_POSN,
	PLANE_ELEVATOR_POSN,
	PLANE_RUDDER_POSN,
	PLANE_WINGTIP_POSN,	// make sure this is no greater than VEHICLE_MISC_A_POSN
	PLANE_MAX_POSNS,
	
	FAKE_PLANE_TAILLIGHT_POSN = 0,
	FAKE_PLANE_LEFTLIGHT_POSN,
	FAKE_PLANE_RIGHTLIGHT_POSN,
	
	BOAT_FRONTSEAT_POSN = 0,
	BOAT_MAX_POSNS,
	
	BIKE_BARGRIP_POSN = VEHICLE_PEDARM_POSN,
	BIKE_MISC_A_POSN,
	BIKE_MISC_B_POSN,
	BIKE_MAX_POSNS
};

// Positions of vehicle upgrades. There is only space for 15
enum VehicleUpgradePosn {
	VEH_UPGRADE_BONNET,
	VEH_UPGRADE_BONNET_LEFT,
	VEH_UPGRADE_BONNET_RIGHT,
	VEH_UPGRADE_BONNET_DAMAGED,
	VEH_UPGRADE_BONNET_LEFT_DAMAGED,
	VEH_UPGRADE_BONNET_RIGHT_DAMAGED,
	VEH_UPGRADE_SPOILER,
	VEH_UPGRADE_SPOILER_DAMAGED,
	VEH_UPGRADE_WING_LEFT,
	VEH_UPGRADE_WING_RIGHT,
	VEH_UPGRADE_FRONTBULLBAR,
	VEH_UPGRADE_BACKBULLBAR,
	VEH_UPGRADE_FRONTLIGHTS,
	VEH_UPGRADE_FRONTLIGHTS_DAMAGED,
	VEH_UPGRADE_ROOF,
	VEH_UPGRADE_NITRO,
	VEH_UPGRADE_HYDRAULICS,
	VEH_UPGRADE_STEREO,
	VEH_UPGRADE_MAX
};


// 
//
//
struct VehicleDesc
{
	RwObjectNameIdAssocation *pAssocArray;
};

//
// name:		UpgradePosnDesc
// description:	Description of where a vehicle upgrade may go
struct UpgradePosnDesc
{
	CVector posn;
	CQuaternion quat;
	int32	parentCompId;
};

//
// 
//
//
class CVehicleModelInfo : public CClumpModelInfo
{
	friend class CVehicle;

	enum {
		FRONT_LEFT_LIGHT=0,
		FRONT_RIGHT_LIGHT,
		REAR_LEFT_LIGHT,
		REAR_RIGHT_LIGHT
	};

	//
	// name:		CVehicleStructure
	// description:	Class containing info ascertained from the vehicle model structure
	class CVehicleStructure
	{
	public:
		CVector m_positions[VEHICLE_MAX_POSNS];
		UpgradePosnDesc m_upgrades[VEH_UPGRADE_MAX];
		RpAtomic* m_pXtraAtomic[MAX_NUM_XTRA_COMPONENTS];
		int8 m_numXtraAtomics;
		uint32 m_nFlagDamageModelsAvailable;
		
		CVehicleStructure();
		~CVehicleStructure();
		
		void* operator new(size_t nSize) {return m_pInfoPool->New();}
		void operator delete(void *pVoid) {m_pInfoPool->Delete((CVehicleStructure*)pVoid);}

		static CPool<class CVehicleStructure>* m_pInfoPool;
	};

	//
	// name:		CLinkedUpgrade
	// description:	class defining a link between two vehicle upgrades
	class CLinkedUpgradeList
	{
	public:
		int16 m_model1[NUM_LINKED_UPGRADES];
		int16 m_model2[NUM_LINKED_UPGRADES];
		int32 m_number;
		
		CLinkedUpgradeList() {m_number = 0;}
		
		void AddUpgradeLink(int16 m1, int16 m2);
		int16 FindOtherUpgrade(int16 modelId) const;
	};
	
public:
	CVehicleModelInfo();

	static void SetupCommonData();
	static void ShutdownLightTexture(void);
	
	inline void Init() {CClumpModelInfo::Init(); m_vehicleType = VEHICLE_TYPE_NONE; m_wheelId = -1; /*m_firstColour[0] = NULL; m_secondColour[0] = NULL;*/ m_steerAngle = INVALID_STEER_ANGLE;}
	virtual uint8 GetModelType() {return MI_TYPE_VEHICLE;}
	virtual void SetClump(RpClump* pClump);
	virtual void DeleteRwObject(); 
	virtual RwObject* CreateInstance();
	
	// set type of object e.g. car, boat etc
	void SetGameName(const char* pName) {strncpy(m_gameName, pName, 8);}
	const char* GetGameName() {return &m_gameName[0];}
	
	void SetAnimFile(const char* pName);
	void ConvertAnimFileIndex();
	int32 GetAnimFileIndex() {return m_animFileIndex;}
	
	void SetVehicleClass(VehicleType type) {m_vehicleType = type;}
	VehicleType GetVehicleClass() {return m_vehicleType;}
	void SetWheels(int32 id, float scale, float scaleRear) {m_wheelId = id; m_wheelScale = scale; m_wheelScaleRear = scaleRear;}
	void GetWheelPosn(int32 num, CVector& posn, bool bSkipTransforms=false);
	float GetWheelScale(bool bFrontWheel=true) {return (bFrontWheel ? m_wheelScale : m_wheelScaleRear);}
//	float GetRearWheelScale() {return m_wheelScaleRear;}
	int32 GetWheelModelId() {return m_wheelId;}
	bool GetOriginalCompPosition(CVector& posn, int32 nId);
	void SetBikeVars(int32 nAngle, float scale, float scaleRear) {m_steerAngle = (float)nAngle; m_wheelScale = scale; m_wheelScaleRear = scaleRear; }
	float GetSteerAngle() {return m_steerAngle;}
	int32 GetNumberOfDoors() {return m_numDoors;}
	void SetNumberOfDoors(const int32 numDoors) {m_numDoors=numDoors;}
	const CVector& GetPosnOnVehicle(VehiclePartPosn posn) { return m_pStructure->m_positions[posn];}
	const CVector& GetFrontSeatPosn() { if(m_vehicleType == VEHICLE_TYPE_BOAT) return m_pStructure->m_positions[BOAT_FRONTSEAT_POSN]; else return m_pStructure->m_positions[VEHICLE_FRONTSEAT_POSN];}
	const CVector& GetBackSeatPosn() { return m_pStructure->m_positions[VEHICLE_BACKSEAT_POSN];}
	void SetHandlingId(int32 id) {m_handlingId = id;}
	int32 GetHandlingId() {return m_handlingId;}
	void SetVehicleList(int32 id) {m_inList = id;}
	int32 GetVehicleList() {return m_inList;}
	void SetVehicleFreq(int32 freq) {m_freq = freq;}
	int32 GetVehicleFreq() {return m_freq;}
	void SetFlags(int32 flags) {m_flags = flags;}
	void SetFlag(int32 flags) {m_flags |= (1<<flags);}
	void ClearFlag(int32 flags) {m_flags = m_flags & (~(1<<flags));}
	bool GetFlag(int32 flag) { return(m_flags & (1<<flag));}
//	int32 IsAvailableInThisLevel(eLevelName level) {return (m_levelMask & (1<<((int32)level-1)));}
	void SetComponentRulesMask(uint32 rules) {m_compRules = rules;}
	int32 ChooseComponent();
	int32 ChooseSecondComponent();
	static int32 GetMaximumNumberOfPassengersFromNumberOfDoors(int32 model_index);
	bool IsCompDamageModelAvailable(uint32 nCompIndex) { return (m_pStructure->m_nFlagDamageModelsAvailable &(1<<nCompIndex)); }

	// Car colours
	static void LoadVehicleColours();
	static void DeleteVehicleColourTextures();
	static RwRGBA& GetVehicleColour(int32 col) {return ms_vehicleColourTable[col];}
	void FindEditableMaterialList();
	void SetVehicleColour(uint8 col1, uint8 col2, uint8 col3, uint8 col4);
	uint8 GetNumPossibleColours() { return m_numPossibleColours;}
#ifndef FINAL	
	void SetVehicleColourOnTheFly(uint8 r, uint8 g, uint8 b, int32 index = 0);
	void UpdateCarColourTexture(uint8 r, uint8 g, uint8 b, int32 index = 0);
	uint8 GetPossibleColours(int32 col, int32 index) { return m_possibleColours[col][index];}
	void SetPossibleColours(int32 col, int32 index, int32 value) { m_possibleColours[col][index] = value;}
	uint8 GetLastColUsed() { return m_lastColUsed; }
	void SetColourTable(uint8 r, uint8 g, uint8 b, uint8 index) { ms_vehicleColourTable[index].red = r; ms_vehicleColourTable[index].green = g; ms_vehicleColourTable[index].blue = b;}
	void SetColourText(char *str, uint8 index) { strcpy(ms_vehicleColourText[index], str);}
	static char	*GetVehicleColourText(int32 col) {return ms_vehicleColourText[col];}
#endif	
	void ChooseVehicleColour(uint8& col1, uint8& col2, uint8& col3, uint8& col4, int32 inc = 1);
	void AvoidSameVehicleColour(UInt8 *pCarCol1, UInt8 *pCarCol2, UInt8 *pCarCol3, UInt8 *pCarCol4);
	
#if defined (GTA_PS2)
	static void UpdateVehicleColour(RxPS2AllPipeData *ps2AllPipeData);
#else //GTA_PS2
	static void UpdateVehicleColour(RpClump *pClump, CVehicleModelInfo *pModelInfo);
	void DisableEnvMap();
	void SetEnvMapCoeff(float coeff);

	RpMaterial* GetDirtMaterial(UInt32 idx) { return m_aDirtMaterials[idx]; }
#endif //GTA_PS2

	// Remap textures
	int32 GetNumRemaps();
	void AddRemap(int16 txdIndex);
	int32 GetRemapTxdIndex(int16 index) {ASSERTMSG(m_remaps[index]!=-1, "Remap doesnt exist"); return m_remaps[index];}
	static void RegisterRemapTexture(const char* pName, int32 txdIndex);
	static void SetRemapTexture(RwTexture* pRemap) {ms_pRemapTexture = pRemap;}
	
	// environment map
	static void LoadEnvironmentMaps();
	void SetRenderPipelines();
	void SetCarCustomPlate();
	static void ShutdownEnvironmentMaps();
	static RpAtomic* SetRenderPipelinesCB(RpAtomic* pAtomic, void* pData);

	// Add random components
	static void SetComponentsToUse(int8 comp1, int8 comp2) {ms_compsToUse[0] = comp1; ms_compsToUse[1] = comp2;}
	static void GetComponentsUsed(int8& comp1, int8& comp2) {comp1 = ms_compsUsed[0]; comp2 = ms_compsUsed[1];}

	// Vehicle upgrade functions
	bool IsUpgradeAvailable(VehicleUpgradePosn upgrade);
	UpgradePosnDesc& GetUpgradeDescriptor(VehicleUpgradePosn upgrade) {return m_pStructure->m_upgrades[upgrade];}
	int32 GetVehicleUpgrade(int32 index) {return m_upgradeModels[index];}
	void SetVehicleUpgrade(int32 index, int32 modelId) {m_upgradeModels[index] = modelId;}
	static void LoadVehicleUpgrades();
	static const CLinkedUpgradeList& GetLinkedUpgradeList() {return ms_linkedUpgrades;}
	static void AddWheelUpgrade(int32 wheelClass, int32 model);
	static int32 GetNumWheelUpgrades(int32 wheelClass);
	static int32 GetWheelUpgrade(int32 wheelClass, int32 index);
	
	void SetWheelUpgradeClass(int8 wheelClass) { m_wheelUpgradeClass = wheelClass;}
	int32 GetWheelUpgradeClass() {return m_wheelUpgradeClass;}
	
	// Object callbacks
	static RwObject* SetAtomicFlagCB(RwObject* pObj, void* pData);
	static RwObject* ClearAtomicFlagCB(RwObject* pObj, void* pData);
	// Atomic callbacks
	static RpAtomic* HideAllComponentsAtomicCB(RpAtomic* pAtomic, void* pData);
	
	// Load texture callback
	static void UseCommonVehicleTexDicationary();
	static void StopUsingCommonVehicleTexDicationary();

	// vehicle lights
	static void SetupLightFlags(CVehicle* pVehicle);

	//
	// custom car plate stuff:
	//
private:
	RpMaterial	*pCustomPlateMaterial;							// pointer to material containing custom plate texture
																// NULL if no customizable plate present
	char 		tabCustomPlateText[CUSTOM_PLATE_NUM_CHARS+1];	// the text itself
	uint8		nCarplateDesign;								// custom design: SF, Vegas, LA or default

public:	
	void				SetCustomCarPlateText(char *pText);
	char*				GetCustomCarPlateText();
	inline void			SetCustomCarPlateDesign(uint8 carplateDesign);
	inline uint8		GetCustomCarPlateDesign();

	inline bool8		CustomCarPlatingAvailable();
	inline RpMaterial*	GetCustomPlateMaterial();

	Int32				GetNumTimesUsed() { return m_NumTimesUsed; }
	void				ResetNumTimesUsed() { m_NumTimesUsed = 0; }
	void				IncreaseNumTimesUsed() { m_NumTimesUsed = MIN(m_NumTimesUsed+1, 120); }

	static RpMaterial* HasAlphaMaterialCB(RpMaterial* pMaterial, void* pData);
	
private:	
	void SetAtomicRenderCallbacks();
	void SetVehicleComponentFlags(RwFrame* pFrame, uint32 flags);
	void SetVehicleComponentFlags();
	void PreprocessHierarchy();
	void ReduceMaterialsInVehicle();

	static void SetEditableMaterials(RpClump* pClump);
	static void ResetEditableMaterials(RpClump* pClump);

	// Frame callbacks
	static RwFrame* CollapseFramesCB(RwFrame* pFrame, void* pData);
	// Object callbacks
	static RwObject* MoveObjectsCB(RwObject* pObj, void* pData);
	// Atomic callbacks
	static RpAtomic* SetAtomicRendererCB(RpAtomic* pAtomic, void* pData);
	static RpAtomic* SetAtomicRendererCB_RealHeli(RpAtomic* pAtomic, void* pData);
	static RpAtomic* SetAtomicRendererCB_Heli(RpAtomic* pAtomic, void* pData);
	static RpAtomic* SetAtomicRendererCB_Boat(RpAtomic* pAtomic, void* pData);
	static RpAtomic* SetAtomicRendererCB_Train(RpAtomic* pAtomic, void* pData);
	static RpAtomic* SetAtomicRendererCB_BigVehicle(RpAtomic* pAtomic, void* pData);
	static RpAtomic* HideDamagedAtomicCB(RpAtomic* pAtomic, void* pData);
	static RpAtomic* GetEditableMaterialListCB(RpAtomic* pAtomic, void* pData);
	static RpAtomic* SetEditableMaterialsCB(RpAtomic* pAtomic, void* pData);
	// Material callbacks
	static RpMaterial* SetEditableMaterialsCB(RpMaterial* pMaterial, void* pData);
	static RpMaterial* GetEditableMaterialListCB(RpMaterial* pMaterial, void* pData);
	static RpMaterial* SetEnvironmentMapCB(RpMaterial* pMaterial, void* pData);
	static RpAtomic* SetEnvironmentMapAtomicCB(RpAtomic* pAtomic, void* pData);
	static RpMaterial* GetMatFXEffectMaterialCB(RpMaterial* pMaterial, void* pData);
	static RpAtomic* SetEnvMapCoeffAtomicCB(RpAtomic* pAtomic, void* pData);
	static RpMaterial* SetEnvMapCoeffCB(RpMaterial* pMaterial, void* pData);
	// texture find callback
	static RwTexture* FindTextureCB(const char* pTexname);
	
	char m_gameName[8];
	VehicleType m_vehicleType;
	float m_wheelScale;		// Front wheels
	float m_wheelScaleRear;	// Rear wheels
	int16 m_wheelId;
	int16 m_handlingId;
	int8 m_numDoors;
	int8 m_inList;
	int8 m_flags;
	int8 m_wheelUpgradeClass;
	int8 m_NumTimesUsed;
	int16 m_freq;
	uint32 m_compRules;
	float m_steerAngle;
	
	CVehicleStructure* m_pStructure;
	//int32 m_xtraComps1, m_xtraComps2;
	
#if !defined (GTA_PS2)
	// Arrays of materials to set the colour of when choosing car colours
	RpMaterial* m_firstColour[MAX_PER_VEHICLE_MATERIALS+1];
	RpMaterial* m_secondColour[MAX_PER_VEHICLE_MATERIALS2+1];
	RpMaterial* m_thirdColour[MAX_PER_VEHICLE_MATERIALS3+1];
	RpMaterial* m_fourthColour[MAX_PER_VEHICLE_MATERIALS4+1];

	// array of dirt textures for which are generated at init and mapped onto cars surface
	RpMaterial*	m_aDirtMaterials[MAX_PER_VEHICLE_DIRT_MATERIALS];
#endif //!GTA_PS2
	
	uint8 m_possibleColours[4][MAX_POSSIBLE_COLOURS];
	uint8 m_numPossibleColours;
	uint8 m_lastColUsed;
	uint8 m_lastCol[4];
//	UInt8 OldCol1, OldCol2;

	int16 m_upgradeModels[MAX_POSSIBLE_UPGRADES];
	int16 m_remaps[MAX_NUM_REMAPS];
	int32 m_animFileIndex;

	// environment map code
	static RwTexture* ms_pEnvironmentMaps[NUM_ENVIRON_MAPS];
	static VehicleDesc ms_vehicleDescs[];
	// vehicle colours
	static RwRGBA ms_vehicleColourTable[NUM_VEHICLE_COLOURS];
#ifndef FINAL	
	static char	  ms_vehicleColourText[NUM_VEHICLE_COLOURS][255];
#endif
	//static RwTexture* ms_colourTextureTable[NUM_VEHICLE_COLOURS];
	// common texture dictionary
	static RwTexDictionary* ms_pCommonTextureDictionary;
	static RwTexture* ms_pLightsTexture;
	static RwTexture* ms_pLightsOnTexture;
	// upgrade wheels
	static int16 ms_numWheelUpgrades[NUM_WHEEL_UPGRADE_CLASSES];
	static int16 ms_upgradeWheels[NUM_WHEEL_UPGRADE_CLASSES][NUM_WHEEL_UPGRADES];
		
	static uint8 ms_currentCol[4];
	static RwTexture* ms_pRemapTexture;
	static int8 ms_compsUsed[2];
	static int8 ms_compsToUse[2];
	static bool ms_lightsOn[4];
	// upgrades
	static CLinkedUpgradeList ms_linkedUpgrades;
};

extern bool gbUseCarColTex;
extern RwSurfaceProperties gLightSurfProps;

#if defined (GTA_PS2)
//
// name:		UpdateVehicleColour
// description:	Update the colour of a vehicle 
//
inline void CVehicleModelInfo::UpdateVehicleColour(RxPS2AllPipeData *ps2AllPipeData)
{
	int32 index;
	uint32 matCol = (*(uint32*)&ps2AllPipeData->matCol) & 0xffffff;
	
	// check for remap textures for paint jobs
	if(ms_pRemapTexture)
	{
		if(ps2AllPipeData->texture && ps2AllPipeData->texture->name[0] == REMAP_TEXTURE_IDENTIFIER)
			ps2AllPipeData->texture = ms_pRemapTexture;
	}
	
	if(ps2AllPipeData->texture == ms_pLightsTexture)
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
		*(uint32*)&ps2AllPipeData->matCol |= 0xffffff;
		if(index != -1 && ms_lightsOn[index])
		{
			ps2AllPipeData->texture = ms_pLightsOnTexture;
			ps2AllPipeData->surfProps = &gLightSurfProps;
		}	
		return;	
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
		return;
			 
	{
		ps2AllPipeData->matCol.red = ms_vehicleColourTable[index].red;
		ps2AllPipeData->matCol.green = ms_vehicleColourTable[index].green;
		ps2AllPipeData->matCol.blue = ms_vehicleColourTable[index].blue;
	}
}

#else //GTA_PS2
#include "CarFXRenderer.h"

typedef struct {
	CVehicleModelInfo* pModelInfo;
	int32 numMat1, numMat2, numMat3, numMat4, numDirt;
} VehicleColourCBData;

// name:		UpdateVehicleColour
// description:	Update the colour of a vehicle 
//
inline void CVehicleModelInfo::UpdateVehicleColour(RpClump *pClump, CVehicleModelInfo *pModelInfo)//RwRGBA* pRGBA, RwTexture** ppTexture)
{
	VehicleColourCBData cbData;
	
	cbData.pModelInfo = pModelInfo;
	cbData.numMat1 = 0;
	cbData.numMat2 = 0;
	cbData.numMat3 = 0;
	cbData.numMat4 = 0;
	cbData.numDirt = 0;
	RpClumpForAllAtomics(pClump, GetEditableMaterialListCB, &cbData);
	int32 index;
/*	RwRGBA* pRGBA = (RwRGBA *) RpMaterialGetColor(aMat); 
	RwTexture* ppTexture = (RpMaterialGetTexture(aMat));

	// check for remap textures for paint jobs
	if(ms_pRemapTexture)
	{
		if(ppTexture && (ppTexture)->name[0] == REMAP_TEXTURE_IDENTIFIER)
			ppTexture = ms_pRemapTexture;
	}

	if(ppTexture && (rwstrcmp((ppTexture)->name,"vehiclelights128")==0) || 
		ppTexture && (rwstrcmp((ppTexture)->name,"vehiclelightson128")==0))
	{
		if(((*(uint32*)pRGBA) & 0xffffff) == LIGHT_FRONT_LEFT_COLOUR)
			index = ms_currentCol[0];
		else if(((*(uint32*)pRGBA) & 0xffffff) == LIGHT_FRONT_RIGHT_COLOUR)
			index = ms_currentCol[1];
		else if(((*(uint32*)pRGBA) & 0xffffff) == LIGHT_REAR_LEFT_COLOUR)
			index = ms_currentCol[2];
		else if(((*(uint32*)pRGBA) & 0xffffff) == LIGHT_REAR_RIGHT_COLOUR)
			index = ms_currentCol[3];
		else return;
		if(index != -1 && ms_lightsOn[index])
		{
			RpMaterialSetTexture(aMat, CVehicleModelInfo::ms_pLightsOnTexture);
		}
		RwTexture* pTex = RpMaterialGetTexture(aMat);
		CDebug::DebugLog("shit %s", pTex->name);

	}else
	{
		if(((*(uint32*)pRGBA) & 0xffffff) == MAT1_COLOUR)
			index = ms_currentCol[0];
		else if(((*(uint32*)pRGBA) & 0xffffff) == MAT2_COLOUR)
			index = ms_currentCol[1];
		else if(((*(uint32*)pRGBA) & 0xffffff) == MAT3_COLOUR)
			index = ms_currentCol[2];
		else if(((*(uint32*)pRGBA) & 0xffffff) == MAT4_COLOUR)
			index = ms_currentCol[3];
		else
			return;
	}		 
*/
/*#ifdef GTA_PS2			
	if((*ppTexture && (*ppTexture)->name[0] != CARCOL_TEXTURE_IDENTIFIER) ||
		gbUseCarColTex == false)
#endif			*/
	{
//		*pRGBA = ms_vehicleColourTable[index];
	}
/*#ifdef GTA_PS2			
	else
	{
		pRGBA->red = 255;
		pRGBA->green = 255;
		pRGBA->blue = 255;
		*ppTexture = ms_colourTextureTable[index];
	}	
#endif*/
}

#endif //GTA_PS2
//
//
//
//
bool8 CVehicleModelInfo::CustomCarPlatingAvailable()
{
	return(this->pCustomPlateMaterial!=NULL);
}

RpMaterial* CVehicleModelInfo::GetCustomPlateMaterial()					
{
	return(this->pCustomPlateMaterial);
}

void CVehicleModelInfo::SetCustomCarPlateDesign(uint8 carplateDesign)
{
	this->nCarplateDesign = carplateDesign;
}

uint8 CVehicleModelInfo::GetCustomCarPlateDesign()
{
	return(this->nCarplateDesign);
}


#endif // INC_VEHICLE_MODELINFO_H_