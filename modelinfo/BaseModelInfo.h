//
//
//    Filename: BaseModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Base class for all model info classes
//
//
#ifndef INC_BASE_MODELINFO_H_
#define INC_BASE_MODELINFO_H_

#define USE_MODELINFO_HASHKEY

// C headers
//$DW$#include <string.h>
//$DW$#ifdef GTA_PS2
// SCE headers
//$DW$	#include <eekernel.h>
//$DW$#endif	
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "dma.h"
#include "Debug.h"
#include "ColModel.h"
#include "Camera.h"
#ifdef USE_MODELINFO_HASHKEY
#include "KeyGen.h"
#endif


#define MODEL_NAME_LEN			(21)

// Possible model types
enum ModelInfoType {
	MI_TYPE_NONE,
	MI_TYPE_ATOMIC,
	MI_TYPE_MLO,
	MI_TYPE_TIME,
	MI_TYPE_WEAPON,
	MI_TYPE_CLUMP,
	MI_TYPE_VEHICLE,
	MI_TYPE_PED,
	MI_TYPE_LOD,
	MI_TYPE_XTRACOMPS
};

class C2dEffect;
class CAtomicModelInfo;
class CDamageAtomicModelInfo;
class CLodAtomicModelInfo;
class CTimeInfo;

// base model flags
#define SIMPLE_PRERENDERED			(1<<0)
#define SIMPLE_DRAW_LAST			(1<<1)
#define SIMPLE_DRAW_ADDITIVE		(1<<2)
#define SIMPLE_DONTWRITE_ZBUFFER	(1<<3)
#define SIMPLE_NO_SHADOWS			(1<<4)
#define SIMPLE_LOD					(1<<5)
#define SIMPLE_BACKFACE_CULLING		(1<<6)
#define SIMPLE_OWN_COLMODEL			(1<<7)

class CBaseModelInfo
{
	friend class CModelInfo;
	
public:
	CBaseModelInfo();
	virtual ~CBaseModelInfo() {}

	// dynamic cast functions
	virtual CAtomicModelInfo* AsAtomicModelInfoPtr() {return NULL;}
	virtual CDamageAtomicModelInfo* AsDamageAtomicModelInfoPtr() {return NULL;}
	virtual CLodAtomicModelInfo* AsLodAtomicModelInfoPtr() {return NULL;}
	
	// access functions
	virtual uint8 GetModelType() = 0;
	inline void SetModelName(const char* pName);
#ifdef USE_MODELINFO_HASHKEY
	inline uint32 GetHashKey() {return m_hashKey;}
#endif	
#ifndef FINALBUILD	
	inline const char* GetModelName() {return &m_modelName[0];}
#endif	

	// lod distance
	float GetLodDistance() {return m_lodDistance * TheCamera.LODDistMultiplier;}
	float GetLodDistanceUnscaled() {return m_lodDistance;}
	void SetLodDistance(float dist) {m_lodDistance = dist;}
	inline bool IsVisibleFromDistance(float dist);
	// object alpha
	void SetAlpha(uint8 alpha) {m_alpha = alpha;}
	uint8 GetAlpha() {return m_alpha;}
	void IncreaseAlpha();
	// flags
	uint16 GetFlags() {return m_flags;}
	uint32 GetHasBeenPreRendered() {return m_flags & SIMPLE_PRERENDERED;}
	void SetHasBeenPreRendered(uint32 preRender) {if(preRender) m_flags |= SIMPLE_PRERENDERED; else m_flags &= ~SIMPLE_PRERENDERED;}
	uint32 GetDrawLast() {return m_flags & SIMPLE_DRAW_LAST;}
	void SetDrawLast(uint32 drawLast) {if(drawLast) m_flags |= SIMPLE_DRAW_LAST; else m_flags &= ~SIMPLE_DRAW_LAST;}
	uint32 GetDrawAdditive() {return m_flags & SIMPLE_DRAW_ADDITIVE;}
	void SetDrawAdditive(uint32 additive) {if(additive) m_flags |= SIMPLE_DRAW_ADDITIVE; else m_flags &= ~SIMPLE_DRAW_ADDITIVE;}
	uint32 GetDontWriteZBuffer() {return m_flags & SIMPLE_DONTWRITE_ZBUFFER;}
	void SetDontWriteZBuffer(uint32 dontWrite) {if(dontWrite) m_flags |= SIMPLE_DONTWRITE_ZBUFFER; else m_flags &= ~SIMPLE_DONTWRITE_ZBUFFER;}
	uint32 GetDontCastShadowsOn() {return m_flags & SIMPLE_NO_SHADOWS;}
	void SetDontCastShadowsOn(uint32 dontWrite) {if(dontWrite) m_flags |= SIMPLE_NO_SHADOWS; else m_flags &= ~SIMPLE_NO_SHADOWS;}
	uint32 GetIsLod() {return m_flags & SIMPLE_LOD;}
	void SetIsLod(uint32 lod) {if(lod) m_flags |= SIMPLE_LOD; else m_flags &= ~SIMPLE_LOD;}
	uint32 GetIsBackFaceCulled() {return m_flags & SIMPLE_BACKFACE_CULLING;}
	void SetIsBackFaceCulled(uint32 coll) {if(coll) m_flags |= SIMPLE_BACKFACE_CULLING; else m_flags &= ~SIMPLE_BACKFACE_CULLING;}
	uint32 GetOwnsColModel() {return m_flags & SIMPLE_OWN_COLMODEL;}
	void SetOwnsColModel(uint32 coll) {if(coll) m_flags |= SIMPLE_OWN_COLMODEL; else m_flags &= ~SIMPLE_OWN_COLMODEL;}

	// collision model
	bool HasColModel() {return (m_pColModel != NULL);}
	bool DoesItOwnTheColModel() {return m_flags & SIMPLE_OWN_COLMODEL;}
	CColModel& GetColModel() { /*ASSERTOBJ(m_pColModel, m_modelName, "Has no collision");*/ return *m_pColModel;}
	CBoundingBox& GetBoundingBox() { /*ASSERTOBJ(m_pColModel, m_modelName, "Has no collision");*/ return *m_pColModel;}
	void SetColModel(CColModel* pColModel, bool bOwnColModel=false);
	void DeleteCollisionModel();
	// dynamic objects (with physics)
	bool IsDynamic() {return (m_dynamicIndex != -1);}
	int32 GetDynamicIndex(void) {return m_dynamicIndex;}
	void SetDynamicIndex(int32 index) {m_dynamicIndex = index;}
	// 2d effect access
	int32 GetNum2dEffects() { return m_num2dEffects; }
	C2dEffect* Get2dEffect(int32 i);
	void Add2dEffect(C2dEffect* pEffect);
	void Init2dEffects();
	// reference counting
	void AddRef();
	void RemoveRef();
	int32 GetNumRefs() {return m_numRefs;}
	// texdictionary stuff
	void SetTexDictionary(const char* pName);
	int32 GetTexDictionary() {return m_txdIndex;}
	void ClearTexDictionary();
	void AddTexDictionaryRef();
	void RemoveTexDictionaryRef();
	
#ifndef FINAL
	float GetTexPercentage();
#endif	
	
	// time stuff
	virtual CTimeInfo* GetTimeInfo() {return NULL;}
	
	virtual void Init();	
	virtual void Shutdown();
	virtual void DeleteRwObject() = 0;
	virtual int32 GetRwModelType() = 0;
	virtual RwObject* CreateInstance() = 0;
	virtual RwObject* CreateInstance(RwMatrix* pMatrix) = 0;
	RwObject* GetRwObject() {return m_pRwObject;}

	virtual void SetAnimFile(const char* pName) {}
	virtual void ConvertAnimFileIndex() {}
	virtual int32 GetAnimFileIndex() {return -1;}

protected:
#ifdef USE_MODELINFO_HASHKEY
	uint32 m_hashKey;
#endif	
#ifndef FINAL
	char m_modelName[MODEL_NAME_LEN];
#endif	
	int16 m_numRefs;
	
	int16 m_txdIndex;
	uint8 m_alpha;
	uint8 m_num2dEffects;
	
	int16 m_n2dEffects;
	int16 m_dynamicIndex;
	
	uint16 m_flags;					// flags are in here to keep size of class down. even though they
									// are only accessed by AtomicModelInfo so far

	CColModel* m_pColModel;
	float m_lodDistance;
	
	RwObject* m_pRwObject;
};

inline void CBaseModelInfo::SetModelName(const char* pName) 
{
#ifdef USE_MODELINFO_HASHKEY
	m_hashKey = CKeyGen::GetUppercaseKey(pName);
#endif	
#ifndef FINAL
	ASSERTOBJ(strlen(pName) < MODEL_NAME_LEN, pName, "ModelName too long"); 
	strcpy(&m_modelName[0], pName);
#endif	
}

//
//        name: CBaseModelInfo::IsVisibleFromDistance
// description: Returns if model is visible from a certain distance
//          in: dist = distance from model
inline bool CBaseModelInfo::IsVisibleFromDistance(float dist)
{
	return (dist < m_lodDistance * TheCamera.LODDistMultiplier);
}

//
// name:		IncreaseAlpha
// description:	Increase the alpha for a modelinfo and return if it is still non-opaque
//
inline void CBaseModelInfo::IncreaseAlpha()
{
#define ALPHA_CHANGE	16
	if(m_alpha < 255 - ALPHA_CHANGE)
	{
		m_alpha += ALPHA_CHANGE;
	}	
	else
	{
		m_alpha = 255;	
	}	
}

#ifndef FINAL
// DW - for clump callback
class CTwoFloats
{
	public :
		float a,b;
};
#endif

#endif // INC_BASE_MODELINFO_H_