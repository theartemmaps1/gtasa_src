//
// name:		WeaponModelInfo.cpp
// description:	File containing class describing weapon model info objects
// Written by:	Adam Fowler

#include "WeaponModelInfo.h"
#include "WeaponInfo.h"
#include "AnimManager.h"
#include "VisibilityPlugins.h"
#include "Vehicle.h"

RwObject *GetCurrentAtomicObjectCB(RwObject *pObject, void *data);

void CWeaponModelInfo::Init()
{
	CClumpModelInfo::Init();
	m_weaponInfo = WEAPONTYPE_UNARMED;
}

//
// name:		SetAtomic
// description:	Set atomic for weapon modelinfo
void CWeaponModelInfo::SetClump(RpClump* pClump) 
{
	CClumpModelInfo::SetClump(pClump);
	RpClumpForAllAtomics(pClump, &SetAtomicRendererCB, (void*) &CVisibilityPlugins::RenderWeaponCB);
//	CVisibilityPlugins::SetAtomicRenderCallback(pAtomic, &CVisibilityPlugins::RenderWeaponCB);

	RwFrame *pFlashFrame = CClumpModelInfo::GetFrameFromName(pClump, "gunflash");
	if(pFlashFrame)
	{
		RpAtomic *pFlashAtomic = (RpAtomic*)GetFirstObject(pFlashFrame);

		if(pFlashAtomic != NULL)
		{
			CVehicle::SetComponentAtomicAlpha(pFlashAtomic, 0);
			RpAtomicSetFlags(pFlashAtomic, 0);
			
			RpGeometry* pGeom = RpAtomicGetGeometry(pFlashAtomic);
			RpMaterial* pMat = RpGeometryGetMaterial(pGeom, 0);
			pMat->surfaceProps.ambient = 16.0f;
		}
	}
}
/*
//
// name:		SetAnimFile
// description:	Set anim file required when loading this model
void CWeaponModelInfo::SetAnimFile(const char* pName)
{
	if(!stricmp(pName, "null"))
		return;
	// temporarily let animFileIndex be a pointer to the anim block
	m_animFileIndex = (int32)new char[strlen(pName)+1];
	strcpy((char*)m_animFileIndex, pName);
}

//
// name:		ConvertAnimFileIndex
// description:	Convert animfile name to an animfile index
void CWeaponModelInfo::ConvertAnimFileIndex()
{
	if(m_animFileIndex == -1)
		return;
	int32 index = CAnimManager::GetAnimationBlockIndex((char*)m_animFileIndex);
	
	delete[] (char*)m_animFileIndex;
	m_animFileIndex = index;
}

*/