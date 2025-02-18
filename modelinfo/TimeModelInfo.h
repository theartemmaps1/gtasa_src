//
//
//    Filename: TimeModelInfo.h
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a Time models that are displayed only at certain times of the day.
//
//
#ifndef INC_TIME_MODELINFO_H_
#define INC_TIME_MODELINFO_H_

// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "AtomicModelInfo.h"
#include "LodAtomicModelInfo.h"

class CTimeInfo
{
public:
	CTimeInfo() : m_otherModel(-1) {}

	// access functions
	inline void SetTimes(uint8 timeOn, uint8 timeOff) {m_on = timeOn; m_off = timeOff;}
	inline int32 GetTimeOn() {return m_on;}
	inline int32 GetTimeOff() {return m_off;}
	
	CTimeInfo* FindOtherTimeModel(const char* pName);
	inline void SetOtherTimeModel(int32 index) { m_otherModel = index;}
	inline int32 GetOtherTimeModel() { return m_otherModel;}
	
protected:
	uint8 m_on, m_off;
	int16 m_otherModel;
};

//
// name:		CTimeModelInfo
// description:	Atomic model that switches on and off
class CTimeModelInfo : public CAtomicModelInfo
{
public:
	CTimeModelInfo() {}
	
	virtual uint8 GetModelType() {return MI_TYPE_TIME;}
	virtual CTimeInfo* GetTimeInfo() {return &m_time;}
protected:
	CTimeInfo m_time;
};

//
// name:		CTimeModelInfo
// description:	Atomic model that switches on and off
class CLodTimeModelInfo : public CLodAtomicModelInfo
{
public:
	CLodTimeModelInfo() {}
	
	virtual uint8 GetModelType() {return MI_TYPE_TIME;}
	virtual CTimeInfo* GetTimeInfo() {return &m_time;}
protected:
	CTimeInfo m_time;
};



#endif // INC_MLO_MODELINFO_H_