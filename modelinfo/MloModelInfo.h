//
//
//    Filename: MloModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a MLO model. A MLO(Multi level object) model consists of an array of simple
//				objects at predefined positions
//
//
#ifndef INC_MLO_MODELINFO_H_
#define INC_MLO_MODELINFO_H_

#ifdef GTA_PS2
// SCE headers
//$DW$#include <eekernel.h>
#endif
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "ClumpModelInfo.h"

/*

These aren't used anymore. In actual fact they were never used.

class CMloModelInfo : public CClumpModelInfo
{
public:
	CMloModelInfo() : CClumpModelInfo(MI_TYPE_MLO) {}

	inline void Init() {CClumpModelInfo::Init(); m_instStart = 0; m_instEnd = 0;}
	
	void ConstructClump();
	float GetLodDistance() {return m_lodDistance;}// * ms_lodDistScale;}
	void SetLodDistance(float dist) {m_lodDistance = dist;}
	void SetInstanceStart(int32 start) {m_instStart = start; m_instEnd = start;}
	void IncrementInstanceEnd() {m_instEnd++;}
protected:

	float m_lodDistance;
	int32 m_instStart, m_instEnd;
};

*/
#endif // INC_MLO_MODELINFO_H_