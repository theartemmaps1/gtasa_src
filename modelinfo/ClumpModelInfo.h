//
//
//    Filename: ClumpModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a model with a RenderWare Clump attached
//
//
#ifndef INC_CLUMP_MODELINFO_H_
#define INC_CLUMP_MODELINFO_H_

//$DW$#ifdef GTA_PS2
// SCE headers
//$DW$	#include <eekernel.h>
//$DW$#endif	
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "BaseModelInfo.h"

typedef struct {
	char* pName;
	int32 hierId;
	uint32 flags;
} RwObjectNameIdAssocation;

// Structures used in find atomic callback function and find frame callback functions
typedef struct {
	const char* pName;
	RwObject* pObject;
} RwObjectNameAssoc;
typedef struct {
	int32 id;
	RwObject* pObject;
} RwObjectIdAssoc;


#define CLUMP_NO_FRAMEID	(0x1)		// don't store frame id

// flags
#define CLUMP_IS_ANIMATED					(1<<8)
#define CLUMP_IS_COMPLEX					(1<<9)
#define CLUMP_IS_DOOR						(1<<10)
#define CLUMP_COLLISION_STREAMED_WITH_MODEL	(1<<11)


class CClumpModelInfo : public CBaseModelInfo
{
public:
	CClumpModelInfo() {}

	virtual void Init();
	virtual void Shutdown();

	// Solid Models get their BB from their collision model
	virtual CBoundingBox& GetBoundingBox() {return GetColModel();}
		
	virtual uint8 GetModelType() {return MI_TYPE_CLUMP;}
	virtual int32 GetRwModelType() {return rpCLUMP;}
	virtual RwObject* CreateInstance();
	virtual RwObject* CreateInstance(RwMatrix* pMatrix);
	//virtual RwObject* GetRwObject() {return (RwObject*)m_pClump;}
	virtual void DeleteRwObject();
	
	virtual void SetClump(RpClump* pClump);
	RpClump* GetClump() {return (RpClump*)m_pRwObject;}
	
	void SetAnimFile(const char* pName);
	void ConvertAnimFileIndex();
	int32 GetAnimFileIndex() {return m_animFileIndex;}

	// flags
	uint32 GetIsAnimated() const {return m_flags & CLUMP_IS_ANIMATED;}
	void SetIsAnimated(uint32 bIsAnimated) {if(bIsAnimated) m_flags |= CLUMP_IS_ANIMATED; else m_flags &= ~CLUMP_IS_ANIMATED;}
	uint32 GetHasComplexHierarchy() const {return m_flags & CLUMP_IS_COMPLEX;}
	void SetHasComplexHierarchy(uint32 bIsComplex) {if(bIsComplex) m_flags |= CLUMP_IS_COMPLEX; else m_flags &= ~CLUMP_IS_COMPLEX;}
	uint32 GetIsDoor() const {return m_flags & CLUMP_IS_DOOR;}
	void SetIsDoor(uint32 isDoor) {if(isDoor) m_flags |= CLUMP_IS_DOOR; else m_flags &= ~CLUMP_IS_DOOR;}
	uint32 GetWasCollisionStreamedWithModel() {return m_flags & CLUMP_COLLISION_STREAMED_WITH_MODEL;}
	void SetCollisionWasStreamedWithModel(uint32 coll) {if(coll) m_flags |= CLUMP_COLLISION_STREAMED_WITH_MODEL; else m_flags &= ~CLUMP_COLLISION_STREAMED_WITH_MODEL;}

	// returns a pointer to the atomic with given ID. This is not very fast
	static RwFrame* GetFrameFromId(RpClump* pClump, int32 id);
	static RwFrame* GetFrameFromName(RpClump* pClump, const char* pName);
	// slightly faster version
	static void FillFrameArray(RpClump* pClump, RwFrame** ppFrame); 

	static RpAtomic* SetAtomicRendererCB(RpAtomic* pAtomic, void* pData);
	
protected:

	void SetFrameIds(RwObjectNameIdAssocation* pAssoc);
	// Frame callbacks
	static RwFrame* FindFrameFromNameCB(RwFrame* pFrame, void* pData);
	static RwFrame* FindFrameFromNameWithoutIdCB(RwFrame* pFrame, void* pData);
	static RwFrame* FindFrameFromIdCB(RwFrame* pFrame, void* pData);
	static RwFrame* FillFrameArrayCB(RwFrame* pFrame, void* pData);

	// Model pointer has been moved to CBaseModelInfo
	//RpClump* m_pClump;
	int32 m_animFileIndex;

};
 
#endif // INC_CLUMP_MODELINFO_H_