//
//
//    Filename: AtomicModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a simple model. Consists of four levels of detail
//
//
#ifndef INC_LOD_MODELINFO_H_
#define INC_LOD_MODELINFO_H_

#ifdef GTA_PS2
// SCE headers
//$DW$#include <eekernel.h>
#endif
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "AtomicModelInfo.h"
#include "Camera.h"


class CAtomicModelInfo;

//
//   Class Name: CLodAtomicModelInfo
//
class CLodAtomicModelInfo : public CAtomicModelInfo
{
public:
	CLodAtomicModelInfo() : numChildren(0), numChildrenRendered(0) {}

	virtual uint8 GetModelType() {return MI_TYPE_LOD;}
	virtual CLodAtomicModelInfo* AsLodAtomicModelInfoPtr() {return this;}
	void Init();

	// access functions
	//void Setup(int32 minIndex, int32 maxIndex);
	//void FindRelatedModel(int32 minIndex, int32 maxIndex);
	//CAtomicModelInfo* GetRelatedModel() {return m_pRelated;}
	
	//void SetNearDistance(float dist) {m_nearDistance = dist;}
	//float GetNearDistance() { return m_nearDistance * TheCamera.LODDistMultiplier;}
	
	void AddLodChild() {numChildren++;}
	int32 GetNumLodChildren() {return numChildren;}
	void AddLodChildRendered() {numChildrenRendered++;}
	void ResetLodRenderedCounter() {numChildrenRendered = 0;}
	bool HasLodChildBeenRendered() {return (numChildrenRendered > 0);}
protected:

	// num child higher level LODs
	int16 numChildren;
	// num child higher level LODs that have been rendered
	int16 numChildrenRendered;

	//CAtomicModelInfo* m_pRelated;
	//float m_nearDistance;
};

#endif // INC_LOD_MODELINFO_H_