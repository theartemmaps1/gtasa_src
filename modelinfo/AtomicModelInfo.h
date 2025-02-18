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
#ifndef INC_ATOMIC_MODELINFO_H_
#define INC_ATOMIC_MODELINFO_H_

//$DW$#ifdef GTA_PS2
// SCE headers
//$DW$#include <eekernel.h>
//$DW$#endif
// RenderWare headers
//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>
// Game headers
#include "BaseModelInfo.h"


// flags
#define ATOMIC_WET_ROAD_REFLECTION			(1<<8)
#define ATOMIC_IS_PLANTFRIENDLY				(1<<9)
#define ATOMIC_DONT_COLLIDE_WITH_FLYER		(1<<10)

// To reduce the space used up by the flags we have a list of
// atrributes that are exclusive (only one of these can be set)
// These values take the following bits: 11, 12, 13, 14
#define ATOMIC_L_BITSHIFT				(11)
#define ATOMIC_L_MASK					(15 <<ATOMIC_L_BITSHIFT)

#define ATOMIC_L_NOTHING_SPECIAL		(0 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_TREE				(1 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_PALMTREE			(2 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_EXPLODES_WHEN_SHOT		(3 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_CODE_GLASS			(4 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_ARTIST_GLASS		(5 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_TAG					(6 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_GARAGE_DOOR			(7 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_DOOR				(8 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_IS_CRANE				(9 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_DISPLAY_WET_ONLY		(10 <<ATOMIC_L_BITSHIFT)
#define ATOMIC_L_DOES_NOT_PROVIDE_COVER	(11 <<ATOMIC_L_BITSHIFT)

// vehicle upgrade flags
#define ATOMIC_REPLACEMENT_UPGRADE	(1<<8)
#define ATOMIC_IS_UPGRADE			(1<<15)
#define ATOMIC_UPGRADE_ID_SHIFT		(10)
#define ATOMIC_UPGRADE_ID			(0x1f<<ATOMIC_UPGRADE_ID_SHIFT)

//
//   Class Name: CAtomicModelInfo
//
class CAtomicModelInfo : public CBaseModelInfo
{
public:
	CAtomicModelInfo() {}
	
	virtual CAtomicModelInfo* AsAtomicModelInfoPtr() {return this;}

	virtual void Init();

	virtual RwObject* CreateInstance();
	virtual RwObject* CreateInstance(RwMatrix* pMatrix);
	//virtual RwObject* GetRwObject() {return (RwObject*)m_pAtomic;}
	virtual void DeleteRwObject();
	virtual uint8 GetModelType() {return MI_TYPE_ATOMIC;}
	virtual int32 GetRwModelType() {return rpATOMIC;}

	// access functions
	virtual void SetAtomic(RpAtomic* pAtomic);
	RpAtomic* GetAtomic() {return (RpAtomic*)m_pRwObject;}
	RpAtomic* GetAtomicFromDistance(float dist);
	
	// flags
	uint32 GetWetRoadReflection() const {return m_flags & ATOMIC_WET_ROAD_REFLECTION;}
	void SetWetRoadReflection(uint32 useWetRoadReflection) {if(useWetRoadReflection) m_flags |= ATOMIC_WET_ROAD_REFLECTION; else m_flags &= ~ATOMIC_WET_ROAD_REFLECTION;}
//	void SetUseDamageModel(bool useDamage) {if(useDamage) m_flags |= ATOMIC_USE_DAMAGE_MODEL; else m_flags &= ~ATOMIC_USE_DAMAGE_MODEL;}
	uint32 GetIsCodeGlass() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_IS_CODE_GLASS);}
	void SetIsCodeGlass(uint32 isGeneric) {if(isGeneric) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_IS_CODE_GLASS; }
	uint32 GetIsArtistGlass() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_IS_ARTIST_GLASS);}
	void SetIsArtistGlass(uint32 isGeneric) {if(isGeneric) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_IS_ARTIST_GLASS; }
	uint32 GetIsPlantFriendly() const { return(m_flags & ATOMIC_IS_PLANTFRIENDLY); }
	void SetIsPlantFriendly(uint32 isPFriendly) { if(isPFriendly) m_flags |= ATOMIC_IS_PLANTFRIENDLY; else m_flags &= ~ATOMIC_IS_PLANTFRIENDLY; }
	uint32 GetIsTag() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_IS_TAG);}
	void SetIsTag(uint32 isGeneric) {if(isGeneric) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_IS_TAG; }
	uint32 GetIsGarageDoor() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_IS_GARAGE_DOOR);}
	void SetIsGarageDoor(uint32 isGarageDoor) {if(isGarageDoor) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_IS_GARAGE_DOOR; }
	uint32 GetIsTree() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_IS_TREE);}
	void SetIsTree(uint32 isGeneric) {if(isGeneric) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_IS_TREE; }
	uint32 GetIsPalmTree() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_IS_PALMTREE);}
	void SetIsPalmTree(uint32 isGeneric) {if(isGeneric) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_IS_PALMTREE; }
	uint32 GetDontCollideWithFlyer() const {return m_flags & ATOMIC_DONT_COLLIDE_WITH_FLYER;}
	void SetDontCollideWithFlyer(uint32 isGeneric) {if(isGeneric) m_flags |= ATOMIC_DONT_COLLIDE_WITH_FLYER; else m_flags &= ~ATOMIC_DONT_COLLIDE_WITH_FLYER;}
//	uint32 GetExplodesWhenShot() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_EXPLODES_WHEN_SHOT);}
//	void SetExplodesWhenShot(uint32 isGeneric) {if(isGeneric) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_EXPLODES_WHEN_SHOT; }
	uint32 GetIsAnyTree() const {return (GetIsTree() || GetIsPalmTree());}
	uint32 GetIsCrane() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_IS_CRANE);}
	void SetIsCrane(uint32 isCrane) {if(isCrane) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_IS_CRANE; }
	uint32 GetIsDisplayInWetOnly() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_DISPLAY_WET_ONLY);}
	void SetIsDisplayInWetOnly(uint32 isDisplayWet) {if(isDisplayWet) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_DISPLAY_WET_ONLY; }
	uint32 GetDoesNotProvideCover() const {return ((m_flags & ATOMIC_L_MASK) == ATOMIC_L_DOES_NOT_PROVIDE_COVER);}
	void SetDoesNotProvideCover(uint32 Arg) {if(Arg) m_flags = (m_flags & (~ATOMIC_L_MASK)) | ATOMIC_L_DOES_NOT_PROVIDE_COVER; }

	uint32 GetIsUpgrade() const {return m_flags & ATOMIC_IS_UPGRADE;}
	uint32 GetIsReplacementUpgrade() const {return m_flags & ATOMIC_REPLACEMENT_UPGRADE;}
	uint32 GetUpgradeId() const {return (m_flags & ATOMIC_UPGRADE_ID) >> ATOMIC_UPGRADE_ID_SHIFT;}

	void SetupVehicleUpgradeFlags(const char* pName);
	
protected:

	// pointer to model has been moved to CBaseModelInfo
	//RpAtomic* m_pAtomic;
};


//
// class name:	CDamageAtomicModelInfo
// description:	ModelInfo storing information about an atomic that can
//				be damaged
class CDamageAtomicModelInfo : public CAtomicModelInfo
{
public:
	CDamageAtomicModelInfo() {}
	
	virtual CDamageAtomicModelInfo* AsDamageAtomicModelInfoPtr() {return this;}

	virtual void Init();

	void SetDamagedAtomic(RpAtomic* pAtomic);
	RpAtomic* GetDamagedAtomic() {return m_pDamageAtomic;}
	
	virtual RwObject* CreateInstance();
	virtual RwObject* CreateInstance(RwMatrix* pMatrix);
	virtual void DeleteRwObject();

	static void SetCreateDamagedVersion(bool bDamaged) {m_bCreateDamagedVersion = bDamaged;} 
	
protected:
	RpAtomic* m_pDamageAtomic;	
	
	static bool m_bCreateDamagedVersion;
};

#endif // INC_ATOMIC_MODELINFO_H_