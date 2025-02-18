//
// name:		WeaponModelInfo.cpp
// description:	File containing class describing weapon model info objects
// Written by:	Adam Fowler
#ifndef INC_WEAPONMODELINFO_H_
#define INC_WEAPONMODELINFO_H_

#include "ClumpModelInfo.h"


class CWeaponModelInfo : public CClumpModelInfo
{
public:
	CWeaponModelInfo() : CClumpModelInfo() {}

	void Init();
	
	virtual uint8 GetModelType() {return MI_TYPE_WEAPON;}
	virtual void SetClump(RpClump* pClump);
	//void SetAnimFile(const char* pName);
	//void ConvertAnimFileIndex();
	//int32 GetAnimFileIndex() {return m_animFileIndex;}

	void SetWeaponInfo(int weapon) {m_weaponInfo = weapon;}
	int GetWeaponInfo() {return m_weaponInfo;}

private:
	int32 m_weaponInfo;
};

#endif