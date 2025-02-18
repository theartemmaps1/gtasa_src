//
//
//    Filename: PedModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a Pedestrian model
//
//
#ifndef INC_PED_MODELINFO_H_
#define INC_PED_MODELINFO_H_

#ifdef GTA_PS2
// SCE headers
//$DW$#include <eekernel.h>
#endif
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "ClumpModelInfo.h"
#include "AnimManagerData.h"
#include "PedType.h"
#include "Vector.h"

#define MAX_PER_PED_MATERIALS	15
#define NUM_PED_COLMODEL_NODES 12

#define STD_PED_BOUND_RADIUS 	(1.0f)
#define STD_PED_BOUND_BOX_MIN	(-1.0f)
#define STD_PED_BOUND_BOX_MAX	(0.95f)
#define OLD_PED_BOUND_BOX_MAX	(0.9f)	// want buoyancy to stay the same!


enum ePedPieceTypes{
	PED_COL_SPHERE_LEG = 0,
	PED_COL_SPHERE_MID,
	PED_COL_SPHERE_HEAD,
	
	PED_SPHERE_CHEST,
	PED_SPHERE_MIDSECTION,
	PED_SPHERE_UPPERARM_L,
	PED_SPHERE_UPPERARM_R,
	PED_SPHERE_LEG_L,
	PED_SPHERE_LEG_R,
	PED_SPHERE_HEAD
};

enum ePedModelInfoFlags {
	PED_MINFO_FLAG_BUYSDRUGS = 0,
};

typedef struct {
	char *pName;
	uint32 boneId;
	int32 nPieceType;
//	float fHorzOffset;
//	float fVertOffset;
	float xOffset;
	float yOffset;
	float zOffset;
	float fRadius;
} colModelNodeInfo;


class CPedModelInfo : public CClumpModelInfo
{
public:
	CPedModelInfo() : m_pHitColModel(NULL) {}
	~CPedModelInfo() { if(m_pHitColModel!=NULL) delete m_pHitColModel;}
	
	virtual uint8 GetModelType() {return MI_TYPE_PED;}
	virtual void SetClump(RpClump* pClump);
	void SetLowDetailClump(RpClump* pClump);
	virtual void DeleteRwObject();
	
	void SetMotionAnimGroup(AssocGroupId grp) {m_motionAnimGroup = grp;}
	AssocGroupId GetMotionAnimGroup() {return m_motionAnimGroup;}
	void SetDefaultPedType(ePedType type) {m_defaultPedType = type;}
	ePedType GetDefaultPedType() {return m_defaultPedType;}
	void SetDefaultPedStats(ePedStats nType) {m_defaultPedStats = nType;}
	ePedStats GetDefaultPedStats() {return m_defaultPedStats;}

	void AddXtraAtomics(RpClump* pClump);
	
	void SetCanDriveCars(uint32 drives) { m_drivesWhichCars = drives;}
	bool CanPedDriveCars(uint32 type) { return m_drivesWhichCars&(1<<type);}

	void SetRace(UInt8 Race) { m_Race = Race; }
	UInt8 GetRace() { return(m_Race); }

	void SetFlags(uint32 flags) { m_flags = flags;}
	bool GetFlag(uint32 type) { return m_flags&(1<<type);}
	
	static RwTexture* GetLastFaceTexture() {return NULL;}
	void SetFaceTexture(RwTexture* pTexture);
	CColModel* GetHitColModel() { return m_pHitColModel;}
	// radio stations
	void SetRadioStations(int32 radio1, int32 radio2) {m_radio1 = radio1; m_radio2 = radio2;}
	int32 GetFirstRadioStation() {return m_radio1;}
	int32 GetSecondRadioStation() {return m_radio2;}
	
	void CreateHitColModelSkinned(RpClump *pClump);
	CColModel *AnimatePedColModelSkinned(RpClump *pClump);
	CColModel *AnimatePedColModelSkinnedWorld(RpClump *pClump);
	
	void SetAudioPedType(Int16 apt) {m_AudioPedType=apt;}
	Int16 GetAudioPedType(void) {return(m_AudioPedType);}
	void SetFirstVoice(Int16 fv) {m_FirstVoice=fv;}
	Int16 GetFirstVoice(void) {return(m_FirstVoice);}
	void SetLastVoice(Int16 lv) {m_LastVoice=lv;}
	Int16 GetLastVoice(void) {return(m_LastVoice);}
	void SetNextVoice(Int16 nv) {m_NextVoice=nv;}
	Int16 GetNextVoice(void) {return(m_NextVoice);}
	void IncrementVoice(void);

protected:

	static RwObjectNameIdAssocation m_pPedIds[];
	static colModelNodeInfo m_pColNodeInfos[];

	AssocGroupId m_motionAnimGroup;
	ePedType m_defaultPedType;
	ePedStats m_defaultPedStats;
	uint16 m_drivesWhichCars;
	uint16 m_flags;
	CColModel*	m_pHitColModel;
	int8 m_radio1, m_radio2;
	UInt8	m_Race;
	Int16 m_AudioPedType;
	Int16 m_FirstVoice, m_LastVoice;
	Int16 m_NextVoice;
};


#endif // INC_PED_MODELINFO_H_