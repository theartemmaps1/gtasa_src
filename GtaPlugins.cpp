//
//
//    Filename: GtaPlugins.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Plugins for displaying objects
//
//
// Game headers
#include "VisibilityPlugins.h"
#include "rwplugin.h"
#include "NodeNamePlugin.h"
#include "ModelInfo.h"
#include "AtomicModelInfo.h"
#include "TimeModelInfo.h"
#include "MloModelInfo.h"
#include "Clock.h"
#ifdef GTA_PS2
#include "eeregs.h"
#endif
#include "renderer.h"
#include "Vehicle.h"
#include "lights.h"
#include "camera.h"
#include "timecycle.h"
#include "player.h"
#include "world.h"
#include "meminfo.h"
#include "TexturePool.h"
#include "VehicleModelInfo.h"
#include "ModelIndices.h"
#include "ps2all.h"

#if (defined(GTA_PC) || defined(GTA_XBOX))
#include "ped.h"
#endif //GTA_PC

#ifndef OSW
extern "C" {
RpAtomic* AtomicDefaultRenderCallBack(RpAtomic* pAtomic);
}
#endif

#define DEFAULT_RENDER_CALLBACK		AtomicDefaultRenderCallBack

// typedef's
typedef RwBool (*ClumpVisibilityCallback)(RpClump* pClump);

//
// LOD distances
//
// Standard Vehicle lod distances
#define HIER_LOD_DIST0_RMP					(45.0f  * TheCamera.GenerationDistMultiplier)		// RenderMultiPass LOD distance for vehicles
#define HIER_LOD_DIST0						(70.0f  * TheCamera.GenerationDistMultiplier)
#define HIER_LOD_DIST1						(150.0f * TheCamera.GenerationDistMultiplier)
#define HIER_LOD_DIST0_RMP_SQUARED			(HIER_LOD_DIST0_RMP*HIER_LOD_DIST0_RMP)
#define HIER_LOD_DIST0_SQUARED				(HIER_LOD_DIST0*HIER_LOD_DIST0)
#define HIER_LOD_DIST1_SQUARED				(HIER_LOD_DIST1*HIER_LOD_DIST1)


#define HIER_DIST_CULL_COMPS				(20.0f * TheCamera.LODDistMultiplier)	
#define HIER_DIST_CULL_COMPS_SQUARED 		(HIER_DIST_CULL_COMPS*HIER_DIST_CULL_COMPS)

// Special vehicle atomics lod distances
#define HIER_LOD_DIST0_BIGVEHICLE			(150.0f * TheCamera.GenerationDistMultiplier)
//#define HIER_LOD_DIST1_BIGVEHICLE	(150.0f * TheCamera.GenerationDistMultiplier)
#define HIER_LOD_DIST0_SQUARED_BIGVEHICLE	(HIER_LOD_DIST0_BIGVEHICLE*HIER_LOD_DIST0_BIGVEHICLE)
//#define HIER_LOD_DIST1_SQUARED_BIGVEHICLE	(HIER_LOD_DIST1_BIGVEHICLE*HIER_LOD_DIST1_BIGVEHICLE)
#define HIER_DIST_CULL_COMPS_BIG			(50.0f * TheCamera.LODDistMultiplier)	
#define HIER_DIST_CULL_COMPS_SQUARED_BIG 	(HIER_DIST_CULL_COMPS_BIG*HIER_DIST_CULL_COMPS_BIG)

#define HIER_LOD_DIST_PED					(60.0f * TheCamera.LODDistMultiplier)	
#define HIER_LOD_FADE_DIST_PED				(70.0f * TheCamera.LODDistMultiplier)	
#define HIER_LOD_DIST_SQUARED_PED			(HIER_LOD_DIST_PED*HIER_LOD_DIST_PED)
#define HIER_LOD_FADE_DIST_SQUARED_PED		(HIER_LOD_FADE_DIST_PED*HIER_LOD_FADE_DIST_PED)




//
//
//
#define ATOMIC_DISABLE_MULTIPASS_LOD(pAtomic)	CVisibilityPlugins::SetAtomicFlag(pAtomic,		VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES_LOD)
#define ATOMIC_ENABLE_MULTIPASS_LOD(pAtomic)	CVisibilityPlugins::ClearAtomicFlag(pAtomic,	VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES_LOD)


//
//
//
//
inline
void CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(RpAtomic *pAtomic, float vehicleDistanceFromCamera)
{
	if(vehicleDistanceFromCamera < ms_vehicleLod0RenderMultiPassDist)
	{
		ATOMIC_ENABLE_MULTIPASS_LOD(pAtomic);
	}
	else
	{
		ATOMIC_DISABLE_MULTIPASS_LOD(pAtomic);
	}
}



#define MODELINFOINDEXFROMATOMIC(a)		(*(int16 *)(((RwUInt8*)a) + ms_atomicPluginOffset))
#define USERVALUEFROMATOMIC(a)		(*(uint16 *)(((RwUInt8*)a) + ms_atomicPluginOffset+2))
#define USERVALUEFROMFRAME(a)		(*(int32 *)(((RwUInt8*)a) + ms_framePluginOffset))
#define VISIBILITYCBFROMCLUMP(c)	(*(ClumpVisibilityCallback *)(((RwUInt8*)c) + ms_clumpPluginOffset))
#define ALPHAVALUEFROMCLUMP(c)		(*(int32 *)(((RwUInt8*)c) + ms_clumpPluginOffset+4))

// static member variables
RwInt32 CVisibilityPlugins::ms_atomicPluginOffset = -1;
RwInt32 CVisibilityPlugins::ms_framePluginOffset = -1;
RwInt32 CVisibilityPlugins::ms_clumpPluginOffset = -1;
RpAtomicCallBackRender CVisibilityPlugins::ms_defaultRenderer = NULL;
RwCamera* CVisibilityPlugins::ms_pCamera = NULL;
RwV3d* CVisibilityPlugins::ms_pCameraPosn = NULL;
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaList;
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaBoatAtomicList;
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaEntityList;
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaUnderwaterEntityList;
CLinkList<CVisibilityPlugins::AlphaObjectInfo>CVisibilityPlugins::m_alphaReallyDrawLastList;// LOD distances. Initialised at the beginning of each frame

float CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist;
float CVisibilityPlugins::ms_vehicleLod0Dist;
float CVisibilityPlugins::ms_vehicleLod1Dist;
//float CVisibilityPlugins::ms_vehicleFadeDist;
float CVisibilityPlugins::ms_bigVehicleLod0Dist;
//float CVisibilityPlugins::ms_bigVehicleLod1Dist;
float CVisibilityPlugins::ms_pedLodDist;
float CVisibilityPlugins::ms_pedFadeDist;
float CVisibilityPlugins::ms_cullCompsDist;
float CVisibilityPlugins::ms_cullBigCompsDist;

#if (defined(GTA_PC) || defined(GTA_XBOX))
CLinkList<CPed*> CVisibilityPlugins::ms_weaponPedsForPC;
#endif //GTA_PC

static float gVehicleDistanceFromCamera;
static float gAngleWithHorizontal;

#ifndef MASTER
//#define COUNT_RWOBJECTS
#ifdef DEBUG
#define STORE_ALLOCATED_TEXTURES
#endif
#endif
bool preAlloc = false; 
#ifdef COUNT_RWOBJECTS
int32 numAtomics=0;
int32 maxAtomics=0;		// current max is 3500
int32 numClumps=0;		
int32 maxClumps=0;		// current max is 89
int32 numFrames=0;
int32 maxFrames=0;		// current max is 3300
int32 numGeometry=0;
int32 maxGeometry=0;	// current max is 3300
int32 numRasters=0;
int32 maxRasters=0;
int32 numTextures=0;
int32 maxTextures=0;
int32 numTexDictionary=0;
int32 maxTexDictionary=0;
int32 numMaterials=0;
int32 maxMaterials=0;	// current max is 6600
#endif

#ifdef STORE_ALLOCATED_TEXTURES
RwTexture* gAllocatedArray[10000] = {NULL};

void AddTextureToArray(RwTexture* pTexture)
{
	for(int32 i=0; i<10000; i++)
	{
		if(gAllocatedArray[i] == NULL)
		{
			gAllocatedArray[i] = pTexture;
			break;
		}	
	}
}
void RemoveTextureFromArray(RwTexture* pTexture)
{
	for(int32 i=0; i<10000; i++)
	{
		if(gAllocatedArray[i] == pTexture)
		{
			gAllocatedArray[i] = NULL;
			break;
		}	
	}
}

void PrintAllocatedTextures()
{
	for(int32 i=0; i<10000; i++)
	{
		if(gAllocatedArray[i])
		{
			DEBUGLOG3("%s/%s ref %d\n", gAllocatedArray[i]->name, gAllocatedArray[i]->mask, gAllocatedArray[i]->refCount);
		}	
	}
}
#endif

// --- Atomic plugin functions ----------------------------------------------------------------
// Stores a pointer to the CModelInfo class describing this objects
//
void* CVisibilityPlugins::AtomicConstructor(void* atom, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	ASSERT(ms_atomicPluginOffset > 0);

#ifdef COUNT_RWOBJECTS
	numAtomics++;
	if(numAtomics>maxAtomics)
	{
		maxAtomics = numAtomics;
		if(!preAlloc)
			printf("Max Atomics %d\n", maxAtomics);
	}	
#endif
		
	MODELINFOINDEXFROMATOMIC(atom) = -1;
	USERVALUEFROMATOMIC(atom) = 0;
	return atom;
}

void* CVisibilityPlugins::AtomicCopyConstructor(void* pDestAtom, const void* pSrcAtom, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	ASSERT(ms_atomicPluginOffset > 0);

	MODELINFOINDEXFROMATOMIC(pDestAtom) = MODELINFOINDEXFROMATOMIC(pSrcAtom);
	USERVALUEFROMATOMIC(pDestAtom) = USERVALUEFROMATOMIC(pSrcAtom);

	return pDestAtom;
}

void* CVisibilityPlugins::AtomicDestructor(void* atom, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
#ifdef COUNT_RWOBJECTS
	numAtomics--;
#endif	
	return atom;
}

// --- Frame plugin functions ----------------------------------------------------------------
// stores a hierarchy Id or Clump CModelInfo class pointer
void* CVisibilityPlugins::FrameConstructor(void* pFrame, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	ASSERT(ms_framePluginOffset > 0);

#ifdef COUNT_RWOBJECTS
	numFrames++;
	if(numFrames>maxFrames)
	{
		maxFrames = numFrames;
		if(!preAlloc)
			printf("Max Frames %d\n", maxFrames);
	}
#endif	

	USERVALUEFROMFRAME(pFrame) = NULL;
	return pFrame;
}

void* CVisibilityPlugins::FrameCopyConstructor(void* pDestFrame, const void* pSrcFrame, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	ASSERT(ms_framePluginOffset > 0);

	USERVALUEFROMFRAME(pDestFrame) = USERVALUEFROMFRAME(pSrcFrame);

	return pDestFrame;
}

void* CVisibilityPlugins::FrameDestructor(void* pFrame, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
#ifdef COUNT_RWOBJECTS
	numFrames--;
#endif	
	return pFrame;
}

// --- Clump plugin functions ----------------------------------------------------------------
// stores a visibility callback for the clump
void* CVisibilityPlugins::ClumpConstructor(void* pClump, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	ASSERT(ms_clumpPluginOffset > 0);

#ifdef COUNT_RWOBJECTS
	numClumps++;
	if(numClumps>maxClumps)
	{
		maxClumps = numClumps;
		if(!preAlloc)
			printf("Max clumps %d\n", maxClumps);
	}	
#endif	

//	MODELINFOFROMCLUMP(pClump) = NULL;
	VISIBILITYCBFROMCLUMP(pClump) = &DefaultVisibilityCB;
	ALPHAVALUEFROMCLUMP(pClump) = 255;

	return pClump;
}

void* CVisibilityPlugins::ClumpCopyConstructor(void* pDestClump, const void* pSrcClump, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	ASSERT(ms_clumpPluginOffset > 0);

//	MODELINFOFROMCLUMP(pDestClump) = MODELINFOFROMCLUMP(pSrcClump);
	VISIBILITYCBFROMCLUMP(pDestClump) = VISIBILITYCBFROMCLUMP(pSrcClump);

	return pDestClump;
}

void* CVisibilityPlugins::ClumpDestructor(void* pClump, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
#ifdef COUNT_RWOBJECTS
	numClumps--;
#endif	
	return pClump;
}

#ifdef COUNT_RWOBJECTS
//Geometry
void* GeometryConstructor(void* pGeometry, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numGeometry++;
	if(numGeometry>maxGeometry)
	{
		maxGeometry = numGeometry;
		if(!preAlloc)
			printf("Max geometry %d\n", maxGeometry);
	}	
	return pGeometry;
}
void* GeometryDestructor(void* pGeometry, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numGeometry--;
	return pGeometry;
}
void* GeometryCopy(void* pGeometry, const void* pOtherGeometry, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	return pGeometry;
}
//Rasters
void* RasterConstructor(void* pRaster, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numRasters++;
	if(numRasters>maxRasters)
	{
		maxRasters = numRasters;
		if(!preAlloc)
			printf("Max rasters %d\n", maxRasters);
	}	
	return pRaster;
}
void* RasterDestructor(void* pRaster, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numRasters--;
	return pRaster;
}
void* RasterCopy(void* pRaster, const void* pOtherRaster, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	return pRaster;
}
//Textures
void* TextureConstructor(void* pTexture, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
#ifdef STORE_ALLOCATED_TEXTURES
	AddTextureToArray((RwTexture*)pTexture);
#endif
	numTextures++;
	if(numTextures>maxTextures)
	{
		maxTextures = numTextures;
		if(!preAlloc)
			printf("Max textures %d\n", maxTextures);
	}	
	return pTexture;
}
void* TextureDestructor(void* pTexture, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
#ifdef STORE_ALLOCATED_TEXTURES
	RemoveTextureFromArray((RwTexture*)pTexture);
#endif
	numTextures--;
	return pTexture;
}
void* TextureCopy(void* pTexture, const void* pOtherTexture, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	return pTexture;
}
//TexDictionaries
void* TexDictionaryConstructor(void* pTexDictionary, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numTexDictionary++;
	if(numTexDictionary>maxTexDictionary)
	{
		maxTexDictionary = numTexDictionary;
		if(!preAlloc)
			printf("Max tex dictionaries %d\n", maxTexDictionary);
	}	
	return pTexDictionary;
}
void* TexDictionaryDestructor(void* pTexDictionary, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numTexDictionary--;
	return pTexDictionary;
}
void* TexDictionaryCopy(void* pTexDictionary, const void* pOtherTexDictionary, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	return pTexDictionary;
}
//Materials
void* MaterialConstructor(void* pMaterial, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numMaterials++;
	if(numMaterials>maxMaterials)
	{
		maxMaterials = numMaterials;
		if(!preAlloc)
			printf("Max materials %d\n", maxMaterials);
	}	
	return pMaterial;
}
void* MaterialDestructor(void* pMaterial, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	numMaterials--;
	return pMaterial;
}
void* MaterialCopy(void* pMaterial, const void* pOtherMaterial, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	return pMaterial;
}
#endif
//
//        name: GtaAtomicPluginAttach
// description: Register Gta Atomic plugin
//         out: successful
//
RwBool CVisibilityPlugins::PluginAttach()
{
	ms_atomicPluginOffset = RpAtomicRegisterPlugin(4, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_ATOMIC_ID),
											AtomicConstructor,
											AtomicDestructor,
											AtomicCopyConstructor);

	ms_framePluginOffset = RwFrameRegisterPlugin(4, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_FRAME_ID),
											FrameConstructor,
											FrameDestructor,
											FrameCopyConstructor);

	ms_clumpPluginOffset = RpClumpRegisterPlugin(8, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_CLUMP_ID),
											ClumpConstructor,
											ClumpDestructor,
											ClumpCopyConstructor);
											
#ifdef COUNT_RWOBJECTS
	RpGeometryRegisterPlugin(0, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_CLUMP_ID),
							GeometryConstructor, GeometryDestructor, GeometryCopy);
	RwRasterRegisterPlugin(0, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_CLUMP_ID),
							RasterConstructor, RasterDestructor, RasterCopy);
	RpMaterialRegisterPlugin(0, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_CLUMP_ID),
							MaterialConstructor, MaterialDestructor, MaterialCopy);
	RwTextureRegisterPlugin(0, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_CLUMP_ID),
							TextureConstructor, TextureDestructor, TextureCopy);
	RwTexDictionaryRegisterPlugin(0, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwGTA_CLUMP_ID),
							TexDictionaryConstructor, TexDictionaryDestructor, TexDictionaryCopy);
#endif

	return (ms_atomicPluginOffset != -1 && ms_clumpPluginOffset != -1);
}


//
// name:		Initialise
// description:	Initialise visibility plugins. Initialise linklists
void CVisibilityPlugins::Initialise()
{
	m_alphaList.Init(NUM_ALPHA_ATOMICS);
	m_alphaList.GetFirst()->item.dist = 0.0f;
	m_alphaList.GetLast()->item.dist = 100000000.0f;
	m_alphaBoatAtomicList.Init(NUM_ALPHA_BOAT_ATOMICS);
	m_alphaBoatAtomicList.GetFirst()->item.dist = 0.0f;
	m_alphaBoatAtomicList.GetLast()->item.dist = 100000000.0f;
	m_alphaEntityList.Init(NUM_ALPHA_ENTITY_ATOMICS);
	m_alphaEntityList.GetFirst()->item.dist = 0.0f;
	m_alphaEntityList.GetLast()->item.dist = 100000000.0f;
	m_alphaUnderwaterEntityList.Init(NUM_ALPHA_UNDERWATER_ENTITY_ATOMICS);
	m_alphaUnderwaterEntityList.GetFirst()->item.dist = 0.0f;
	m_alphaUnderwaterEntityList.GetLast()->item.dist = 100000000.0f;
	m_alphaReallyDrawLastList.Init(NUM_ALPHA_REALLYDRAWLIST_OBJECTS);
	m_alphaReallyDrawLastList.GetFirst()->item.dist = 0.0f;
	m_alphaReallyDrawLastList.GetLast()->item.dist = 100000000.0f;

#if (defined(GTA_PC) || defined(GTA_XBOX))
	ms_weaponPedsForPC.Init(NUM_WEAPON_PEDS_FOR_PC);
#endif //GTA_PC
}
 
//
// name:		Shutdown
// description:	Shutdown visibility plugins. Shutdown linklists
void CVisibilityPlugins::Shutdown()
{
	m_alphaList.Shutdown();
	m_alphaBoatAtomicList.Shutdown();
	m_alphaEntityList.Shutdown();
	m_alphaUnderwaterEntityList.Shutdown();
	m_alphaReallyDrawLastList.Shutdown();

#if (defined(GTA_PC) || defined(GTA_XBOX))
	ms_weaponPedsForPC.Shutdown();
#endif //GTA_PC
}

//!PC - for PC the muzzle flashes will cause alpha punch through of stuff rendered after them,
// so need to be able to render weapons & flashes after other stuff
#if (defined(GTA_PC) || defined(GTA_XBOX))
void CVisibilityPlugins::RenderWeaponPedsForPC()
{
	CLink<CPed*>* pLink = ms_weaponPedsForPC.GetLast()->GetPrevious();
	int32 boneTag;
	CPed* pPed;
	RwMatrix* pNodeMatrix;
	RwFrame *pWeaponFrame;
	RwMatrix *pWeaponMatrix;
	RpHAnimHierarchy *pHierarchy;

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) TRUE);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *) TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *) rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *) rwBLENDINVSRCALPHA);
#if (defined GTA_PC || defined GTA_XBOX)
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)20);
#endif

	// For the muzzle flash to render properly this should be set. But then the weapons
	// won't draw properly! In order to fix for PC would have to render the gun with z write enable
	// on, then render the muzzle flash with it off. Argh!
//	RwRenderStateSet( rwRENDERSTATEZTESTENABLE,  (void *)TRUE );
//	RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *)FALSE );

	while(pLink != ms_weaponPedsForPC.GetFirst())
	{
		ASSERT(pLink != NULL);
		pPed = pLink->item;

		// PC crash fix - DW
		if (!pPed || !pPed->m_pWeaponClump)
		{
			pLink = pLink->GetPrevious();
			continue;
		}

		pPed->SetupLighting();
		if(pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_PARACHUTE)
			boneTag = BONETAG_SPINE1;
		else
			boneTag = BONETAG_R_HAND;

		pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)(pPed->m_pRwObject));
		pNodeMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, boneTag));
		if(boneTag==BONETAG_ROOT)
			pNodeMatrix = pPed->GetRwMatrix();

		RwFrame *pWeaponFrame = RpClumpGetFrame(pPed->m_pWeaponClump);
		RwMatrix *pWeaponMatrix = RwFrameGetMatrix(pWeaponFrame);

		*pWeaponMatrix = *pNodeMatrix;
		
		if(pPed->GetWeapon()->GetWeaponType()==WEAPONTYPE_PARACHUTE)
		{
			static RwV3d parachuteOffset = {0.1f, -0.15f, 0.0f};
			RwMatrixTranslate(pWeaponMatrix,  &parachuteOffset, rwCOMBINEPRECONCAT);
			RwMatrixRotate(pWeaponMatrix, &CPedIK::YaxisIK, 90.0f, rwCOMBINEPRECONCAT);
		}

		// need to update the alpha on the gunflash just before we render the weapon
		pPed->SetGunFlashAlpha(false);
		
		RwFrameUpdateObjects(pWeaponFrame);
		// now render the weapon
		RpClumpRender(pPed->m_pWeaponClump);

		// try and render the weapon again in the other hand!
		if(CWeaponInfo::GetWeaponInfo(pPed->GetWeapon()->GetWeaponType(), pPed->GetWeaponSkill())->IsWeaponFlagSet(WEAPONTYPE_TWIN_PISTOLS))
		{
			boneTag = BONETAG_L_HAND;
			pNodeMatrix = (RpHAnimHierarchyGetMatrixArray(pHierarchy) + RpHAnimIDGetIndex(pHierarchy, boneTag));
			*pWeaponMatrix = *pNodeMatrix;

			static RwV3d axis = {1.0f, 0.0f, 0.0f};
			RwMatrixRotate(pWeaponMatrix,  &axis, 180.0f, rwCOMBINEPRECONCAT);
			static RwV3d trans = {0.04f, -0.05f, 0.0f};
			RwMatrixTranslate(pWeaponMatrix,  &trans, rwCOMBINEPRECONCAT);
			
			// need to update the alpha on the gunflash just before we render the weapon
			pPed->SetGunFlashAlpha(true);
			
			RwFrameUpdateObjects(pWeaponFrame);
			// now render the weapon			
			RpClumpRender(pPed->m_pWeaponClump);
		}

		pPed->ResetGunFlashAlpha();
		pLink = pLink->GetPrevious();
	}	
}
#endif //GTA_PC

//
//        name: SetAtomicModelInfo
// description: set the model info associated with this atomic
//          in: pAtomic = pointer to atomic
//
void CVisibilityPlugins::SetModelInfoIndex(RpAtomic *pAtomic, int32 index)
{
	ASSERT(ms_atomicPluginOffset > 0);

	MODELINFOINDEXFROMATOMIC(pAtomic) = index;
}

int32 CVisibilityPlugins::GetModelInfoIndex(RpAtomic *pAtomic)
{
	ASSERT(ms_atomicPluginOffset > 0);

	return MODELINFOINDEXFROMATOMIC(pAtomic);
}

class CBaseModelInfo* CVisibilityPlugins::GetModelInfo(RpAtomic *pAtomic)
{
	ASSERT(ms_atomicPluginOffset > 0);

	int32 index = MODELINFOINDEXFROMATOMIC(pAtomic);
	if(index == -1)
		return NULL;
	return CModelInfo::GetModelInfo(MODELINFOINDEXFROMATOMIC(pAtomic));
}

//
//        name: SetAtomicId
// description: set the id associated with this atomic
//          in: pAtomic = pointer to atomic
//
void CVisibilityPlugins::SetAtomicId(RpAtomic *pAtomic, int32 id)
{
	ASSERT(ms_atomicPluginOffset > 0);

	USERVALUEFROMATOMIC(pAtomic) = id;
}

void CVisibilityPlugins::SetAtomicFlag(RpAtomic *pAtomic, uint16 flag)
{
	ASSERT(ms_atomicPluginOffset > 0);

	USERVALUEFROMATOMIC(pAtomic) |= flag;
}


static
RpAtomic* setClumpForAllAtomicsFlagCB(RpAtomic *pAtomic, void *data)
{
	CVisibilityPlugins::SetAtomicFlag(pAtomic, int32(data));
	return(pAtomic);
}

void CVisibilityPlugins::SetClumpForAllAtomicsFlag(RpClump *pClump, int32 id)
{
	RpClumpForAllAtomics(pClump, &setClumpForAllAtomicsFlagCB, (void*)id);
}


//
//
//
//
void CVisibilityPlugins::ClearAtomicFlag(RpAtomic *pAtomic, uint16 flag)
{
	ASSERT(ms_atomicPluginOffset > 0);

	USERVALUEFROMATOMIC(pAtomic) &= ~flag;
}

static
RpAtomic* clearClumpForAllAtomicsFlagCB(RpAtomic *pAtomic, void *data)
{
	CVisibilityPlugins::ClearAtomicFlag(pAtomic, int32(data));
	return(pAtomic);	
}

void CVisibilityPlugins::ClearClumpForAllAtomicsFlag(RpClump *pClump, int32 id)
{
	RpClumpForAllAtomics(pClump, &clearClumpForAllAtomicsFlagCB, (void*)id);
}


int32 CVisibilityPlugins::GetAtomicId(RpAtomic *pAtomic)
{
	ASSERT(ms_atomicPluginOffset > 0);

	return USERVALUEFROMATOMIC(pAtomic);
}

//
// name:		SetTagAlpha/GetTagAlpha
// description:	Access functions for tag alpha. Alpha is stored in 
void CVisibilityPlugins::SetUserValue(RpAtomic* pAtomic, uint16 value)
{
	USERVALUEFROMATOMIC(pAtomic) = value;
}
uint16 CVisibilityPlugins::GetUserValue(RpAtomic* pAtomic)
{
	return USERVALUEFROMATOMIC(pAtomic);
}

// --- Render callbacks for atomics ---------------------------------------------------------------

//
// name:		SetupVehicleVariables
// description:	Setup the standard vehicle variables
void CVisibilityPlugins::SetupVehicleVariables(RpClump* pClump)
{
	if(RwObjectGetType((RwObject*)pClump) != rpCLUMP)
		return;
		
	RwFrame* pCarFrame = RpClumpGetFrame(pClump);
	RwV3d result;
	
	gVehicleDistanceFromCamera = GetDistanceSquaredFromCamera(pCarFrame);

	// get vector from object to camera
	RwV3dSub(&result, ms_pCameraPosn, RwMatrixGetPos(RwFrameGetMatrix(pCarFrame)));

	gAngleWithHorizontal = CMaths::ATan2(result.z, CMaths::Sqrt(result.x*result.x + result.y*result.y));
}

RpMaterial* SetAlphaCB(RpMaterial* pMaterial, void* pData)
{
	pMaterial->color.alpha = (uint8) (uint32) pData;
//	RwRGBA colour;
//	colour = *RpMaterialGetColor(pMaterial);
//	colour.alpha = (uint8)pData;
//	RpMaterialSetColor(pMaterial, &colour);
	return pMaterial;
}

RpAtomic* CVisibilityPlugins::RenderWheelAtomicCB(RpAtomic* pAtomic)
{
	CAtomicModelInfo* pModelInfo = (CAtomicModelInfo*)GetModelInfo(pAtomic);

	//RwMatrix *pAtomMatrix = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
	//RwV3d result;
	//float len = CMaths::Sqrt(gVehicleDistanceFromCamera);
	//RpAtomic *pLodAtomic = NULL;

	// get vector from camera to object
	//RwV3dSub(&result, RwMatrixGetPos(pAtomMatrix), ms_pCameraPosn);

	// get distance between camera and object and use this to calculate which level of detail to use
	//len = RwV3dLength(&result);

	// use distance between camera and object to calculate which level of detail to use
	//if(pModelInfo)
	//	pLodAtomic = pModelInfo->GetAtomicFromDistance(len *  TheCamera.LODDistMultiplier / TheCamera.GenerationDistMultiplier);
	// if wheel model was part of vehicle model, it will have no separate ModelInfo
	//else if(len / TheCamera.GenerationDistMultiplier < 70.0f)
	//	pLodAtomic = pAtomic;

	// if there exists a Level of detail
	//if(pLodAtomic)
	//{
	//	if(RpAtomicGetGeometry(pLodAtomic) != RpAtomicGetGeometry(pAtomic))
	//	{
	//		RpAtomicSetGeometry(pAtomic, RpAtomicGetGeometry(pLodAtomic), rpATOMICRENDER | rpATOMICSAMEBOUNDINGSPHERE);
	//	}
		
		//RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)RWRGBALONG( CTimeCycle::m_nCurrentFogColourRed, CTimeCycle::m_nCurrentFogColourGreen, CTimeCycle::m_nCurrentFogColourBlue, 255));
		DEFAULT_RENDER_CALLBACK(pAtomic);
	//}
	
	return pAtomic;
}


//
// RenderGtaAtomic: Render callback for atomics in GTA3
//
RpAtomic* CVisibilityPlugins::RenderObjNormalAtomic(RpAtomic* pAtomic)
{
	RwMatrix *pAtomMatrix = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
	RwV3d result;
	float len;

	// get vector from camera to object
	RwV3dSub(&result, RwMatrixGetPos(pAtomMatrix), ms_pCameraPosn);

	// get distance between camera and object and use this to calculate which level of detail to use
	len = RwV3dLength(&result);

	// Cull objects that use their object normal to decide if they are visible
	// If the dot product is positive then the angle between the two vectors is less than 90 degrees
	float dotProduct = RwV3dDotProduct(&result, RwMatrixGetUp(pAtomMatrix));
	if(dotProduct < -0.3f * len && len > 8.0f)
		return pAtomic;

	DEFAULT_RENDER_CALLBACK(pAtomic);
	
	return pAtomic;
}

#define ORIGALPHA_STORESIZE	150

//
// name:		RenderAtomicWithAlpha
// description;	Render an atomic with the given alpha value
void CVisibilityPlugins::RenderAtomicWithAlpha(RpAtomic* pAtomic, int32 alpha)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);

	//atomics with no geometry? Is this right?
//	if (!pGeom)
//		return;

	uint32 flags = RpGeometryGetFlags(pGeom);
	uint8 origAlpha[ORIGALPHA_STORESIZE];
	int32 i;

	RpGeometrySetFlags(pGeom, flags | rpGEOMETRYMODULATEMATERIALCOLOR);

	ASSERTOBJ(pGeom->matList.numMaterials < ORIGALPHA_STORESIZE, 
				GetModelInfo(pAtomic)->GetModelName(),
				"Object has too many materials");
	// set material alpha
	for(i=0; i<pGeom->matList.numMaterials; i++)
	{
		RwRGBA& colour = pGeom->matList.materials[i]->color;
		origAlpha[i] = colour.alpha;

		if (colour.alpha > alpha) 
			colour.alpha = alpha;
	}
//	RpGeometryForAllMaterials(pGeom, &SetAlphaCB, (void*)alpha);

	DEFAULT_RENDER_CALLBACK(pAtomic);

	// reset materials in geometry	
	while(i--)
	{
		RwRGBA& colour = pGeom->matList.materials[i]->color;
		colour.alpha = origAlpha[i];
	}
//	RpGeometryForAllMaterials(pGeom, &SetAlphaCB, (void*)255);
	RpGeometrySetFlags(pGeom, flags);
}

//
// name:		CalculateFadingAtomic1Alpha
// description:	Calculate the alpha for a fading atomic
int32 CVisibilityPlugins::CalculateFadingAtomicAlpha(CBaseModelInfo* pModelInfo, CEntity* pEntity, float dist)
{
	float fadeDist = RENDER_FADEDIST;
	float lodDist = CMaths::Min(pModelInfo->GetLodDistance(), CRenderer::ms_fFarClipPlane + pModelInfo->GetColModel().GetBoundRadius());
	float ratio;
	if(pEntity->GetLod() == NULL)
	{
		float lodDistUnscaled = CMaths::Min(pModelInfo->GetLodDistanceUnscaled(), lodDist);
		if(lodDistUnscaled > 150.0f)
			fadeDist = (lodDistUnscaled / 15.0f) + 10.0f;

		if(pEntity->m_nFlags.bIsBIGBuilding)
			lodDist *= CRenderer::GetLowLodDistanceScale();
	}

	ratio = (lodDist + RENDER_FADEDIST - dist) / fadeDist;
	
	if(ratio > 1.0f)
		ratio = 1.0f;
		
	return (ratio * pModelInfo->GetAlpha());
}

//
// name:		SetupRenderFadingAtomic
// description:	Setup rendering of fading atomics
static uint32 alphaTest;
void CVisibilityPlugins::SetupRenderFadingAtomic(CBaseModelInfo* pModelInfo, int32 alpha)
{
	uint32 newAlphaTest;
	int32 alphaValue;
	
	if(pModelInfo->GetDrawAdditive())
	    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
	
#ifdef GTA_PS2
	RpSkyRenderStateGet(rpSKYRENDERSTATEATEST_1, &alphaTest);
#endif	
	// Calculate distance over 
/*	float fadeDist = RENDER_FADEDIST;
	if(pEntity->GetLod() == NULL)
	{
		float lodDist = pModelInfo->GetLodDistance();
		if(lodDist > 100.0f)
			fadeDist = (lodDist / 10.0f) + 10.0f;
	}

	ratio = (pModelInfo->GetLodDistance() - dist) / fadeDist;
	
	if(ratio > 1.0f)
		ratio = 1.0f;
		
	alpha = ratio * pModelInfo->GetAlpha();*/
	if(alpha == 255)
		return;
		
#ifdef GTA_PS2
	// setup alpha test	
	alphaValue = alpha;
	if(alphaValue < 32)
		alphaValue = 32;
	newAlphaTest = 0x100c;	// FB_ONLY|GREATER
	newAlphaTest |= (alphaValue>>2)<<4;
	RpSkyRenderStateSet(rpSKYRENDERSTATEATEST_1, (void*)newAlphaTest);
#endif

	return;
}

//
// name:		ResetRenderFadingAtomic
// description:	Reset after rendering of fading atomics
void CVisibilityPlugins::ResetRenderFadingAtomic(CBaseModelInfo* pModelInfo)
{
#ifdef GTA_PS2
	RpSkyRenderStateSet(rpSKYRENDERSTATEATEST_1, (void*)alphaTest);
#endif
	
	if(pModelInfo->GetDrawAdditive())
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);	    
}

//
// name:		RenderFadingAtomic
// description: Render an atomic that is fading in 
void CVisibilityPlugins::RenderFadingAtomic(CBaseModelInfo* pModelInfo, RpAtomic* pAtomic, int32 alpha)
{
	SetupRenderFadingAtomic(pModelInfo, alpha);
	RenderAtomicWithAlpha(pAtomic, alpha);
	ResetRenderFadingAtomic(pModelInfo);
}

//
// name:		RenderAtomicWithAlphaCB
// description;	Render an atomic with the given alpha value
RpAtomic* CVisibilityPlugins::RenderAtomicWithAlphaCB(RpAtomic* pAtomic, void* pAlpha)
{
	if(!(RpAtomicGetFlags(pAtomic) & rpATOMICRENDER))
		return pAtomic;
		
	RenderAtomicWithAlpha(pAtomic, *(int32*)pAlpha);
	return pAtomic;
}

//
// name:		RenderFadingClump
// description: Render a clump that is fading in 
void CVisibilityPlugins::RenderFadingClump(CBaseModelInfo* pModelInfo, RpClump* pClump, int32 alpha)
{
	SetupRenderFadingAtomic(pModelInfo, alpha);
	RpClumpForAllAtomics(pClump, &RenderAtomicWithAlphaCB, &alpha);
	ResetRenderFadingAtomic(pModelInfo);
}


// --- Render callbacks for atomics that are contained in vehicle clumps --------------------------

//
// RenderHierReallyLowDetail: Render callback for the lowest detail parts of vehicle
//
RpAtomic* CVisibilityPlugins::RenderVehicleReallyLowDetailCB(RpAtomic* pAtomic)
{
	//float len = GetDistanceSquaredFromCamera(RpClumpGetFrame(pClump));
	if(gVehicleDistanceFromCamera >= ms_vehicleLod0Dist)
	{
#if defined(GTA_PC)
		ATOMIC_DISABLE_MULTIPASS_LOD(pAtomic);	// no multipass stuff on low detail guys
#endif

		RpClump* pClump = RpAtomicGetClump(pAtomic);
		uint32 alpha = GetClumpAlpha(pClump);
		if(alpha == 255)
			DEFAULT_RENDER_CALLBACK(pAtomic);
		else
			RenderAtomicWithAlpha(pAtomic, alpha);	
	}
	return pAtomic;
}


float CVisibilityPlugins::GetDotProductWithCameraVector(RwMatrix* pMatrix, RwMatrix* pCarMatrix, uint32 atomicId)
{
	RwV3d result;
	float dotProduct;
	float dotProduct2;

	// get vector from camera to object
	RwV3dSub(&result, RwMatrixGetPos(pMatrix), ms_pCameraPosn);

	// Get component orientation from car matrix
	
	// If the dot product is positive then the angle between the two vectors is less than 90 degrees
	if(atomicId & (VEHICLE_ATOMIC_REAR|VEHICLE_ATOMIC_FRONT))
		dotProduct = RwV3dDotProduct(&result, RwMatrixGetUp(pCarMatrix));
	else if(atomicId & (VEHICLE_ATOMIC_LEFT|VEHICLE_ATOMIC_RIGHT))
		dotProduct = RwV3dDotProduct(&result, RwMatrixGetRight(pCarMatrix));
	else
		dotProduct = 0;
	if(atomicId & (VEHICLE_ATOMIC_LEFT|VEHICLE_ATOMIC_REAR))
		dotProduct = -dotProduct;

	const float tailgatMul = 2.5f;
	const float doorsMul = 0.25f;
	if(atomicId & VEHICLE_ATOMIC_TOP)
	{
		if(atomicId & (VEHICLE_ATOMIC_REARDOOR|VEHICLE_ATOMIC_FRONTDOOR))
			dotProduct += doorsMul*RwV3dDotProduct(&result, RwMatrixGetAt(pCarMatrix));
		else 
			dotProduct += tailgatMul*RwV3dDotProduct(&result, RwMatrixGetAt(pCarMatrix));
	}	

	// extra ordering for front and rear doors
	if(atomicId & (VEHICLE_ATOMIC_REARDOOR|VEHICLE_ATOMIC_FRONTDOOR))
	{
		if(atomicId & VEHICLE_ATOMIC_REARDOOR)
			dotProduct2 = -RwV3dDotProduct(&result, RwMatrixGetUp(pCarMatrix));
		else if(atomicId & VEHICLE_ATOMIC_FRONTDOOR)
			dotProduct2 = RwV3dDotProduct(&result, RwMatrixGetUp(pCarMatrix));
				
		if(dotProduct < 0 && dotProduct2 < 0)
			dotProduct += dotProduct2;
		if(dotProduct > 0 && dotProduct2 > 0)
			dotProduct += dotProduct2;
	}
	
	return dotProduct;			
}

//
// RenderHierHiDetail: Render callback for high detail parts of vehicle
// 
RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailCB(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera < ms_vehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);
	
		RwFrame* pCarFrame = RpClumpGetFrame(RpAtomicGetClump(pAtomic));
		int32 atomicId = GetAtomicId(pAtomic);
		if(gVehicleDistanceFromCamera > ms_cullCompsDist && !(atomicId & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
		{
			RwMatrix* pMatrix = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
			float dotProduct = GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), atomicId);
			// If on the wrong side of the vehicle cull			
			if(dotProduct > 0 && ((atomicId & VEHICLE_ATOMIC_FLAT) || dotProduct * dotProduct > gVehicleDistanceFromCamera * 0.1f))
				return pAtomic;
		}			
			
		DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}

//
// RenderHierHiDetail: Render callback for high detail parts of vehicle
//
RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailAlphaCB(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera < ms_vehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);

		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);
		RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
#ifdef DEBUG	
		const char* pName = GetFrameNodeName(pFrame);
#endif	
		RwMatrix* pMatrix = RwFrameGetLTM(pFrame);
		int32 atomicId = GetAtomicId(pAtomic);
		float dotProduct = GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), atomicId);
		
		if(gVehicleDistanceFromCamera > ms_cullCompsDist && !(atomicId & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
		{
			// If on the wrong side of the vehicle and vehicle is far enough away then cull	
			if(dotProduct > 0 && ((atomicId & VEHICLE_ATOMIC_FLAT) || dotProduct * dotProduct > gVehicleDistanceFromCamera * 0.1f))
				return pAtomic;
		}		

		if(atomicId & VEHICLE_ATOMIC_ALPHA)
		{
			// ensure objects with alpha are drawn after chassis
			if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera-0.0001f))
				DEFAULT_RENDER_CALLBACK(pAtomic);
			return pAtomic;	
		}

		if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera + dotProduct))
			DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}



RpAtomic* CVisibilityPlugins::RenderHeliRotorAlphaCB(RpAtomic* pAtomic)
{ 
	//float len = GetDistanceSquaredFromCamera(pCarFrame);

	if(gVehicleDistanceFromCamera < ms_bigVehicleLod0Dist)
	{
		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);
		RwV3d result;
		float dotProduct;
		RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
		RwMatrix* pMatrix = RwFrameGetLTM(pFrame);
		RwMatrix* pCarMatrix = RwFrameGetLTM(pCarFrame);

		// get vector from camera to object
		RwV3dSub(&result, RwMatrixGetPos(pMatrix), ms_pCameraPosn);
		dotProduct = RwV3dDotProduct(&result, RwMatrixGetAt(pCarMatrix));
		
		// AF: *20.0f to exagerate the fact that the rotor should be drawn last.
		if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera + dotProduct*20.0f))
//		if(!InsertAtomicIntoReallyDrawLastList(pAtomic, gVehicleDistanceFromCamera))
			DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderHeliTailRotorAlphaCB(RpAtomic* pAtomic)
{
	//float len = GetDistanceSquaredFromCamera(pCarFrame);

	if(gVehicleDistanceFromCamera < ms_vehicleLod0Dist)
	{
		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);
		RwV3d result;
		float dotProduct;
		RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
		RwMatrix* pMatrix = RwFrameGetLTM(pFrame);
		RwMatrix* pCarMatrix = RwFrameGetLTM(pCarFrame);

		// get vector from camera to object
		RwV3dSub(&result, RwMatrixGetPos(pMatrix), ms_pCameraPosn);
		dotProduct = -RwV3dDotProduct(&result, RwMatrixGetRight(pCarMatrix)) - RwV3dDotProduct(&result, RwMatrixGetUp(pCarMatrix));
		
		if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera + dotProduct))
//		if(!InsertAtomicIntoReallyDrawLastList(pAtomic, gVehicleDistanceFromCamera + dotProduct))
			DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}



//
// RenderHierHiDetail: Render callback for high detail parts of vehicle
// 
RpAtomic* CVisibilityPlugins::RenderTrainHiDetailCB(RpAtomic* pAtomic)
{

	if(gVehicleDistanceFromCamera < ms_bigVehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);

		RwFrame* pCarFrame = RpClumpGetFrame(RpAtomicGetClump(pAtomic));
		int32 atomicId = GetAtomicId(pAtomic);
		if(gVehicleDistanceFromCamera > ms_cullCompsDist && !(atomicId & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
		{
			RwMatrix* pMatrix = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
			float dotProduct = GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), atomicId);
			// If on the wrong side of the vehicle cull			
			if(dotProduct > 0 && ((atomicId & VEHICLE_ATOMIC_FLAT) || dotProduct * dotProduct > gVehicleDistanceFromCamera * 0.1f))
				return pAtomic;
		}			
			
		DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}

//
// RenderHierHiDetail: Render callback for high detail parts of vehicle
//
RpAtomic* CVisibilityPlugins::RenderTrainHiDetailAlphaCB(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera < ms_bigVehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);

		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);

		RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
		RwMatrix* pMatrix = RwFrameGetLTM(pFrame);
		int32 atomicId = GetAtomicId(pAtomic);
		float dotProduct = GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), atomicId);
		
		if(gVehicleDistanceFromCamera > ms_cullCompsDist && !(atomicId & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
		{
			// If on the wrong side of the vehicle and vehicle is far enough away then cull	
			if(dotProduct > 0 && ((atomicId & VEHICLE_ATOMIC_FLAT) || dotProduct * dotProduct > gVehicleDistanceFromCamera * 0.1f))
				return pAtomic;
		}
		if(atomicId & VEHICLE_ATOMIC_ALPHA)
		{
			if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera))
				DEFAULT_RENDER_CALLBACK(pAtomic);
			return pAtomic;	
		}
			
		if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera + dotProduct))
			DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}





//////////// The same as above but now for big vehicles (trains, planes etc)

//
// RenderHierReallyLowDetail: Render callback for the lowest detail parts of vehicle
//
RpAtomic* CVisibilityPlugins::RenderVehicleReallyLowDetailCB_BigVehicle(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera >= ms_bigVehicleLod0Dist)
		DEFAULT_RENDER_CALLBACK(pAtomic);
	return pAtomic;
}

//
// RenderHierLowDetail: Render callback for low detail parts of vehicle
//
/*RpAtomic* CVisibilityPlugins::RenderVehicleLowDetailCB_BigVehicle(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera >= ms_bigVehicleLod0Dist && gVehicleDistanceFromCamera < ms_bigVehicleLod1Dist)
	{
		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);

		if(gVehicleDistanceFromCamera > ms_cullCompsDist && !(GetAtomicId(pAtomic) & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
		{
			RwMatrix* pMatrix = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
			// If on the wrong side of the vehicle cull			
			if(GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), GetAtomicId(pAtomic)) > 0)
				return pAtomic;
		}			
		DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}*/

//
// RenderHierHiDetail: Render callback for high detail parts of vehicle
// 
RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailCB_BigVehicle(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera < ms_bigVehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);

		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);

		if(gVehicleDistanceFromCamera > ms_cullBigCompsDist && !(GetAtomicId(pAtomic) & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
		{
			RwMatrix* pMatrix = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
			// If on the wrong side of the vehicle cull			
			if(GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), GetAtomicId(pAtomic)) > 0)
				return pAtomic;
		}			
		DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}

//
// RenderHierLowDetail: Render callback for low detail parts of vehicle
//
/*RpAtomic* CVisibilityPlugins::RenderVehicleLowDetailAlphaCB_BigVehicle(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera >= ms_bigVehicleLod0Dist && gVehicleDistanceFromCamera < ms_bigVehicleLod1Dist)
	{
		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);
	
		RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
		RwMatrix* pMatrix = RwFrameGetLTM(pFrame);
		int32 atomicId = GetAtomicId(pAtomic);
		float dotProduct = GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), atomicId);
		
		// If on the wrong side of the vehicle and vehicle is far enough away then cull	
		if(dotProduct > 0 && gVehicleDistanceFromCamera > ms_cullCompsDist && !(atomicId & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
			return pAtomic;
		
		if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera + dotProduct))
			DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}*/

//
// RenderHierHiDetail: Render callback for high detail parts of vehicle
//
RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailAlphaCB_BigVehicle(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera < ms_bigVehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);

		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);
		RwFrame* pFrame = RpAtomicGetFrame(pAtomic);
		RwMatrix* pMatrix = RwFrameGetLTM(pFrame);
		int32 atomicId = GetAtomicId(pAtomic);
		float dotProduct = GetDotProductWithCameraVector(pMatrix, RwFrameGetLTM(pCarFrame), atomicId);

		if(gVehicleDistanceFromCamera > ms_cullBigCompsDist && !(atomicId & VEHICLE_ATOMIC_DONTCULL) && gAngleWithHorizontal < 0.2f)
		{
			// If on the wrong side of the vehicle and vehicle is far enough away then cull	
			if(dotProduct > 0 && ((atomicId & VEHICLE_ATOMIC_FLAT) || dotProduct * dotProduct > gVehicleDistanceFromCamera * 0.1f))
				return pAtomic;
		}		
		
		if(atomicId & VEHICLE_ATOMIC_ALPHA)
		{
			// ensure objects with alpha are drawn after chassis
			if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera-0.0001f))
				DEFAULT_RENDER_CALLBACK(pAtomic);
			return pAtomic;	
		}

		if(!InsertAtomicIntoSortedList(pAtomic, gVehicleDistanceFromCamera + dotProduct))
			DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}

//
// RenderVehicleLoDetailCB_Boat: Render callback for low detail parts of boat
// 
RpAtomic* CVisibilityPlugins::RenderVehicleLoDetailCB_Boat(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera >= ms_vehicleLod0Dist)
	{
#if defined(GTA_PC)
		ATOMIC_DISABLE_MULTIPASS_LOD(pAtomic);	// no multipass stuff on low detail guys
#endif

		RpClump* pClump = RpAtomicGetClump(pAtomic);
		RwFrame* pCarFrame = RpClumpGetFrame(pClump);

		uint32 alpha = GetClumpAlpha(pClump);
		if(alpha == 255)
			DEFAULT_RENDER_CALLBACK(pAtomic);
		else
			RenderAtomicWithAlpha(pAtomic, alpha);	
	}
	return pAtomic;
}
//
// RenderVehicleHiDetailCB_Boat: Render callback for high detail parts of boat
// 
RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailCB_Boat(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera < ms_vehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);

		DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}

//
// RenderVehicleHiDetailAlphaCB_Boat: Render callback for high detail parts of boat with alpha
// 
RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailAlphaCB_Boat(RpAtomic* pAtomic)
{
	if(gVehicleDistanceFromCamera < ms_vehicleLod0Dist)
	{
		CVisibilityPlugins::RenderVehicleCB_ControlRenderMultiPassLOD(pAtomic, gVehicleDistanceFromCamera);

		int32 atomicId = GetAtomicId(pAtomic);
		if(atomicId & VEHICLE_ATOMIC_ALPHA)
		{
			if(!InsertAtomicIntoBoatSortedList(pAtomic, gVehicleDistanceFromCamera))
				DEFAULT_RENDER_CALLBACK(pAtomic);
		}	
		else
			DEFAULT_RENDER_CALLBACK(pAtomic);
	}	
	return pAtomic;
}


/*
RpAtomic* CVisibilityPlugins::RenderWaterCreaturesAlphaCB(RpAtomic* pAtomic)
{
	int32 alphaVal = GetAtomicId(pAtomic);
	RenderAtomicWithAlpha(pAtomic, alphaVal);
	return pAtomic;
}
*/


//
// name:		SetTextureCB
// description:	Material callback that sets its texture
RpMaterial* SetTextureCB(RpMaterial* pMaterial, void* pData)
{
	RpMaterialSetTexture(pMaterial, (RwTexture*)pData);
	return pMaterial;
}

//
// name:		RenderPlayerCB
// description:	Render the player and chose his skin texture
RpAtomic* CVisibilityPlugins::RenderPlayerCB(RpAtomic* pAtomic)
{
	RwTexture* pTexture = CWorld::Players[0].m_pSkinTexture;
	if(pTexture)
		RpGeometryForAllMaterials(RpAtomicGetGeometry(pAtomic), &SetTextureCB, (void*)pTexture);
	
	DEFAULT_RENDER_CALLBACK(pAtomic);
	return pAtomic;
}

//
// name:		RenderPedCB
// description:	ped render callback
//
RpAtomic* CVisibilityPlugins::RenderPedCB(RpAtomic* pAtomic)
{
	RpClump* pClump = RpAtomicGetClump(pAtomic);
	float len = GetDistanceSquaredFromCamera(RpClumpGetFrame(pClump));
	if(len < ms_pedLodDist)
	{
		uint32 alpha = GetClumpAlpha(pClump);
		if(alpha == 255)
			DEFAULT_RENDER_CALLBACK(pAtomic);
		else
			RenderAtomicWithAlpha(pAtomic, alpha);	
	}
	return pAtomic;
}


//
// name:		RenderFadingClumpCB
// description:	render callback for a fading object
RpAtomic* CVisibilityPlugins::RenderFadingClumpCB(RpAtomic* pAtomic)
{
	RpClump* pClump = RpAtomicGetClump(pAtomic);
	uint32 alpha = GetClumpAlpha(pClump);
	
	if(alpha == 255)
		DEFAULT_RENDER_CALLBACK(pAtomic);
	else
		RenderAtomicWithAlpha(pAtomic, alpha);	
	
	return pAtomic;
}


//
// name:		RenderWeaponCB
// description:	weapon render callback. Switches off after distance
//
RpAtomic* CVisibilityPlugins::RenderWeaponCB(RpAtomic* pAtomic)
{
	RpClump* pClump = RpAtomicGetClump(pAtomic);
	CClumpModelInfo* pModelInfo = GetClumpModelInfo(pClump);
	float len = GetDistanceSquaredFromCamera(RpClumpGetFrame(pClump));
	float lodDist = pModelInfo->GetLodDistance();
	
	if(len < lodDist*lodDist)
	{
		DEFAULT_RENDER_CALLBACK(pAtomic);
	}
	
	return pAtomic;
}

//
//        name: SetAtomicRenderCallback
// description: Set the renderer for an atomic and store the old renderer
//

void CVisibilityPlugins::SetAtomicRenderCallback(RpAtomic* pAtomic, RpAtomicCallBackRender renderCB)
{
	if(renderCB == NULL)
		renderCB = DEFAULT_RENDER_CALLBACK;
	RpAtomicSetRenderCallBack(pAtomic, renderCB);
}

//
//        name: SetRenderWareCamera
// description: Set the camera that is being used to render objects
//
void CVisibilityPlugins::SetRenderWareCamera(RwCamera* pCamera)
{
	ms_pCamera = pCamera;
	ms_pCameraPosn = RwMatrixGetPos(RwFrameGetMatrix(RwCameraGetFrame(ms_pCamera)));
	
	ms_cullCompsDist = HIER_DIST_CULL_COMPS_SQUARED;
	ms_cullBigCompsDist = HIER_DIST_CULL_COMPS_SQUARED_BIG;
	
	ms_vehicleLod0RenderMultiPassDist	= HIER_LOD_DIST0_RMP_SQUARED*2;
	ms_vehicleLod0Dist 					= HIER_LOD_DIST0_SQUARED*2;
	ms_vehicleLod1Dist					= HIER_LOD_DIST1_SQUARED*2;
	ms_bigVehicleLod0Dist				= HIER_LOD_DIST0_SQUARED_BIGVEHICLE*2;
//	ms_bigVehicleLod1Dist				= HIER_LOD_DIST1_SQUARED_BIGVEHICLE;
	ms_pedLodDist						= HIER_LOD_DIST_SQUARED_PED*2;
	ms_pedFadeDist						= HIER_LOD_FADE_DIST_SQUARED_PED*2;
}

//
//        name: IsAtomicVisible
// description: Return if clump is visible
//
RwBool CVisibilityPlugins::IsAtomicVisible(RpAtomic *pAtomic)
{
	RwSphere sphere;
	sphere = *RpAtomicGetBoundingSphere(pAtomic);
	RwV3dTransformPoints(&sphere.center, &sphere.center, 1, RwFrameGetMatrix(RpAtomicGetFrame(pAtomic)));
	return (RwCameraFrustumTestSphere(ms_pCamera, &sphere) != rwSPHEREOUTSIDE);
}

// --- Frame plugin ---------------------------------------------------------------------------------------------------

//
//        name: SetFrameHierarchyId
// description: set the hierarchy Id associated with this frame. 
//				Id and a model info
//          in: pFrame = pointer to atomic
//
void CVisibilityPlugins::SetFrameHierarchyId(RwFrame *pFrame, int32 value)
{
	ASSERT(ms_framePluginOffset > 0);

	USERVALUEFROMFRAME(pFrame) = value;
}

int32 CVisibilityPlugins::GetFrameHierarchyId(RwFrame *pFrame)
{
	ASSERT(ms_framePluginOffset > 0);

	return USERVALUEFROMFRAME(pFrame);
}

// --- Clump plugin ---------------------------------------------------------------------------------------------------

//
// DefaultVisibilityCB: default visibility callback for a clump. 
//
RwBool CVisibilityPlugins::DefaultVisibilityCB(RpClump* pClump)
{
	return TRUE;
}

//
// FrustumSphereCB: returns if the clumps bounding sphere is in the frustum
//
RwBool CVisibilityPlugins::FrustumSphereCB(RpClump* pClump)
{
	RwSphere sphere;
	RwFrame* pFrame = RpClumpGetFrame(pClump);
	CClumpModelInfo* pModel = (CClumpModelInfo*)GetFrameHierarchyId(pFrame);
	ASSERT(pModel);
		
	sphere.radius = (pModel->GetColModel()).GetBoundRadius();
	sphere.center.x = (pModel->GetColModel()).GetBoundOffset().x;
	sphere.center.y = (pModel->GetColModel()).GetBoundOffset().y;
	sphere.center.z = (pModel->GetColModel()).GetBoundOffset().z;

	RwV3dTransformPoints(&sphere.center, &sphere.center, 1, RwFrameGetLTM(pFrame));
	
	return (RwCameraFrustumTestSphere(ms_pCamera, &sphere) != rwSPHEREOUTSIDE);
}

//
// MloVisibilityCB: MLO visibility callback for a clump. 
//
/*RwBool CVisibilityPlugins::MloVisibilityCB(RpClump* pClump)
{
	RwFrame* pFrame = RpClumpGetFrame(pClump);
	CMloModelInfo* pModel = (CMloModelInfo*)GetFrameHierarchyId(pFrame);
	ASSERT(pModel->GetModelType() == MI_TYPE_MLO);
	
	float len = GetDistanceSquaredFromCamera(pFrame);
	float maxDist = pModel->GetLodDistance();

	if(len > maxDist*maxDist)
		return FALSE;

	return FrustumSphereCB(pClump);
}*/


//
// VehicleVisibilityCB: Vehicle object visibility callback for a clump. 
//
RwBool CVisibilityPlugins::VehicleVisibilityCB(RpClump* pClump)
{
	RwFrame* pFrame = RpClumpGetFrame(pClump);
	ASSERT(((CClumpModelInfo*)GetFrameHierarchyId(pFrame))->GetModelType() == MI_TYPE_VEHICLE);
	
	float len = GetDistanceSquaredFromCamera(pFrame);
	
	if(len > ms_vehicleLod1Dist)
		return FALSE;

	return FrustumSphereCB(pClump);
}

//
// VehicleVisibilityCB: Vehicle object visibility callback for a clump. 
//
RwBool CVisibilityPlugins::VehicleVisibilityCB_BigVehicle(RpClump* pClump)
{
	RwFrame* pFrame = RpClumpGetFrame(pClump);
	ASSERT(((CClumpModelInfo*)GetFrameHierarchyId(pFrame))->GetModelType() == MI_TYPE_VEHICLE);
	
	return FrustumSphereCB(pClump);
}


//
//        name: SetClumpModelInfo
// description: set the model info associated with this Clump
//          in: pNode = pointer to node
//				pClump = pointer to Clump
//
void CVisibilityPlugins::SetClumpModelInfo(RpClump *pClump, class CClumpModelInfo* pModelInfo)
{
	RwFrame* pFrame = RpClumpGetFrame(pClump);
	
	ASSERT(pFrame);
	ASSERT(ms_clumpPluginOffset > 0);

	SetFrameHierarchyId(pFrame, (int32)pModelInfo);
	switch(pModelInfo->GetModelType())
	{
	//case MI_TYPE_MLO:
	//	VISIBILITYCBFROMCLUMP(pClump) = &MloVisibilityCB;
	//	break;
	case MI_TYPE_VEHICLE:
		if (((CVehicleModelInfo *)pModelInfo)->GetVehicleClass() == VEHICLE_TYPE_TRAIN /*|| ((CVehicleModelInfo *)pModelInfo)->GetVehicleClass() == VEHICLE_TYPE_FAKE_HELI*/ || ((CVehicleModelInfo *)pModelInfo)->GetVehicleClass() == VEHICLE_TYPE_FAKE_PLANE)
		{
			VISIBILITYCBFROMCLUMP(pClump) = &VehicleVisibilityCB_BigVehicle;
		}
		else
		{
			VISIBILITYCBFROMCLUMP(pClump) = &VehicleVisibilityCB;
		}
		break;
	default:
		break;
	}
}

class CClumpModelInfo* CVisibilityPlugins::GetClumpModelInfo(RpClump *pClump)
{
	RwFrame* pFrame = RpClumpGetFrame(pClump);
	ASSERT(pFrame);

	return (CClumpModelInfo*)GetFrameHierarchyId(pFrame);
}

//
//        name: IsClumpVisible
// description: Return if clump is visible
//
RwBool CVisibilityPlugins::IsClumpVisible(RpClump *pClump)
{
	return VISIBILITYCBFROMCLUMP(pClump)(pClump);
}

//
// name:		SetClumpAlpha/GetClumpAlpha
// description:	Access functions for clump alpha value
void CVisibilityPlugins::SetClumpAlpha(RpClump* pClump, int32 alpha)
{
	ALPHAVALUEFROMCLUMP(pClump) = alpha;
}
int32 CVisibilityPlugins::GetClumpAlpha(RpClump* pClump)
{
	return 	ALPHAVALUEFROMCLUMP(pClump);
}

// -- Alpha-ed polys ----------------------------------------------------------------------

void CVisibilityPlugins::InitAlphaAtomicList()
{
	m_alphaList.Clear();
}

void CVisibilityPlugins::InitAlphaEntityList()
{
	m_alphaEntityList.Clear();
	m_alphaBoatAtomicList.Clear();
	m_alphaUnderwaterEntityList.Clear();
	m_alphaReallyDrawLastList.Clear();
}

//
// name:		InsertAtomicIntoSortedList
// description:	Insert atomic into sorted list of entities for displaying later
bool CVisibilityPlugins::InsertAtomicIntoSortedList(RpAtomic* pAtomic, float dist)
{
	AlphaObjectInfo info;
	
	//info.obj.pAtomic = pAtomic;
	info.pObj = pAtomic;
	info.RenderFn = &RenderAtomic;
	info.dist = dist;
	
	return m_alphaList.InsertSorted(info);
}

//
// name:		InsertAtomicIntoBoatSortedList
// description:	Insert atomic into sorted list of entities for displaying later
bool CVisibilityPlugins::InsertAtomicIntoBoatSortedList(RpAtomic* pAtomic, float dist)
{
	AlphaObjectInfo info;
	
//	info.obj.pAtomic = pAtomic;
	info.pObj = pAtomic;
	info.RenderFn = &RenderAtomic;
	info.dist = dist;
	
	return m_alphaBoatAtomicList.InsertSorted(info);
}

//
// name:		InsertEntityIntoSortedList
// description:	Insert entity into sorted list of entities for displaying later
bool CVisibilityPlugins::InsertEntityIntoSortedList(CEntity* pEntity, float dist)
{
	AlphaObjectInfo info;
	
//	info.obj.pEntity = pEntity;
	info.pObj = pEntity;
	info.RenderFn = &RenderEntity;
	info.dist = dist;
	
	if(pEntity->GetModelIndex() == MI_GRASSHOUSE)
	{
		if(InsertEntityIntoReallyDrawLastList(pEntity, dist))
			return true;
	}
	// If entity is underwater then add into a list that is rendered before the water
	if(pEntity->m_nFlags.bUnderwater)
	{
		return m_alphaUnderwaterEntityList.InsertSorted(info);
	}
	return m_alphaEntityList.InsertSorted(info);
}

//
// name:		InsertEntityIntoReallyDrawLastList
// description:	Insert entity into sorted list of entities for drawing right at the end
bool CVisibilityPlugins::InsertEntityIntoUnderwaterList(class CEntity* pEntity, float dist)
{
	AlphaObjectInfo info;
	
	info.pObj = pEntity;
	info.RenderFn = &RenderEntity;;
	info.dist = dist;
	
	return m_alphaUnderwaterEntityList.InsertSorted(info);
}

//
// name:		InsertObjectIntoSortedList
// description:	Insert object into sorted list of entities with a render function for displaying later 
bool CVisibilityPlugins::InsertObjectIntoSortedList(void* pObj, float dist, RenderFunction fn)
{
	AlphaObjectInfo info;
	
//	info.obj.pEntity = pEntity;
	info.pObj = pObj;
	info.RenderFn = fn;
	info.dist = dist;
	
	return m_alphaEntityList.InsertSorted(info);
}


//
// name:		InsertAtomicIntoReallyDrawLastList
// description:	Insert atomic into sorted list of entities for drawing right at the end
bool CVisibilityPlugins::InsertAtomicIntoReallyDrawLastList(RpAtomic* pAtomic, float dist)
{
	AlphaObjectInfo info;
	
	info.pObj = pAtomic;
	info.RenderFn = &RenderAtomic;
	info.dist = dist;
	
	return m_alphaReallyDrawLastList.InsertSorted(info);
}
//
// name:		InsertEntityIntoReallyDrawLastList
// description:	Insert entity into sorted list of entities for drawing right at the end
bool CVisibilityPlugins::InsertEntityIntoReallyDrawLastList(class CEntity* pEntity, float dist)
{
	AlphaObjectInfo info;
	
	info.pObj = pEntity;
	info.RenderFn = &RenderEntity;;
	info.dist = dist;
	
	return m_alphaReallyDrawLastList.InsertSorted(info);
}

//
// name:		RenderAtomicList
// description:	Render a list of atomics from back to front 
/*void CVisibilityPlugins::RenderAtomicList(CLinkList<AlphaObjectInfo>& atomicList)
{
	CLink<AlphaObjectInfo>* pLink = atomicList.GetLast()->GetPrevious();
	
	while(pLink != atomicList.GetFirst())
	{
		ASSERT(pLink != NULL)
		DEFAULT_RENDER_CALLBACK(pLink->item.obj.pAtomic);
		pLink = pLink->GetPrevious();
	}	
}*/

//
// name:		RenderAlphaAtomics
// description:	Render the list of alpha atomics
void CVisibilityPlugins::RenderAlphaAtomics()
{
//	RenderAtomicList(m_alphaList);
	RenderOrderedList(m_alphaList);
}

//
// name:		RenderBoatAlphaAtomics
// description:	Render the list of alpha atomics belonging to boats
void CVisibilityPlugins::RenderBoatAlphaAtomics()
{
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);
//	RenderAtomicList(m_alphaBoatAtomicList);
	RenderOrderedList(m_alphaBoatAtomicList);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);
}

//
// name:		RenderAtomic
// description:	Render an atomic
void CVisibilityPlugins::RenderAtomic(void* pObj, float dist)
{
	DEFAULT_RENDER_CALLBACK((RpAtomic*)pObj);
	//RpAtomicRender((RpAtomic*)pObj);
}

//
// name:		RenderEntity
// description:	Render an entity
void CVisibilityPlugins::RenderEntity(void* pObj, float dist)
{
	CBaseModelInfo* pModelInfo;
	CEntity* pEntity = (CEntity*)pObj;

	if(pEntity->GetRwObject()) 
	{
		pModelInfo = CModelInfo::GetModelInfo(pEntity->m_nModelIndex);
		if(pModelInfo->GetDontWriteZBuffer())
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);
				
#ifdef ES2EMU
		extern void HitTestSetModelName(const char* name);
		const char* modelName = pModelInfo->GetModelName();
		HitTestSetModelName(modelName);
#endif

		if(pEntity->m_nFlags.bDistanceFade == true)
		{
#if (defined(GTA_PC) || defined(GTA_XBOX))
			//!PC - disable alpha cutoff when fading out
			RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)0);
#endif //GTA_PC

			bool bResetColours;
			int32 alpha = CalculateFadingAtomicAlpha(pModelInfo, pEntity, dist);
			pEntity->m_nFlags.bImBeingRendered = true;

			if(!pEntity->m_nFlags.bBackfaceCulled)
				RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
			
			bResetColours = pEntity->SetupLighting();
			
			if(RwObjectGetType(pEntity->GetRwObject()) == rpATOMIC)
				RenderFadingAtomic(pModelInfo, (RpAtomic*)pEntity->GetRwObject(), alpha);
			else
				RenderFadingClump(pModelInfo, (RpClump*)pEntity->GetRwObject(), alpha);
			
			pEntity->RemoveLighting(bResetColours);
				
			pEntity->m_nFlags.bImBeingRendered = false;

			if(!pEntity->m_nFlags.bBackfaceCulled)
				RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLBACK);

#if defined(GTA_PC) && !defined(FINALBUILD) && !defined(OSW)
			ASSERTOBJ(gbIndexBufferError == false, pModelInfo->GetModelName(),
						"Object contains a mesh with zero vertices");
#endif
	
		}	
		else
		{
#if (defined(GTA_PC) || defined(GTA_XBOX))
			//!PC - required to minimize alpha halo problems outside (bushes & trees mainly)
			if ((CGame::currArea==AREA_MAIN_MAP) && (!pModelInfo->GetDontWriteZBuffer()))
			{
				RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)100);
			}
			else
			{
				RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)0);
			}
#endif //GTA_PC
			CRenderer::RenderOneNonRoad(pEntity);
		}		
			
#ifdef ES2EMU
		HitTestSetModelName(NULL);
#endif

		if(pModelInfo->GetDontWriteZBuffer())
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) TRUE);
	}
}	

#if defined (GTA_PS2)
//
// name:		RenderOrderedList
// description:	Render a list of objects from back to front 
void CVisibilityPlugins::RenderOrderedList(CLinkList<AlphaObjectInfo>& list)
{
	CLink<AlphaObjectInfo>* pLink = list.GetLast()->GetPrevious();
	
	while(pLink != list.GetFirst())
	{
		ASSERT(pLink != NULL);
		(pLink->item.RenderFn)(pLink->item.pObj, pLink->item.dist);
		pLink = pLink->GetPrevious();
	}	
}
#else //GTA_PS2
//!PC - need some jiggery pokery to improve rendering of alpha cutouts - linear filtering can
// cause a halo effect unless an alpha cutoff is used. Can't use it with fading objects though!
// name:		RenderOrderedList
// description:	Render a list of objects from back to front 
void CVisibilityPlugins::RenderOrderedList(CLinkList<AlphaObjectInfo>& list)
{
	CLink<AlphaObjectInfo>* pLink = list.GetLast()->GetPrevious();

	while(pLink != list.GetFirst())
	{
		ASSERT(pLink != NULL);
		(pLink->item.RenderFn)(pLink->item.pObj, pLink->item.dist);
		pLink = pLink->GetPrevious();
	}	
}
#endif //GTA_PS2

//
// name:		RenderFadingEntities
// description:	Render the link list of entities
/*void CVisibilityPlugins::RenderFadingEntities(CLinkList<AlphaObjectInfo>& list)
{
	CLink<AlphaObjectInfo>* pLink = list.GetLast()->GetPrevious();
	CBaseModelInfo* pModelInfo;
	CEntity* pEntity;

#ifndef FINALBUILD
	if(gbDisplayUnmatchedBigBuildings)
		return;
#endif
		
	// Render draw last	
	while(pLink != list.GetFirst())
	{
		pEntity = pLink->item.obj.pEntity;
		if(pEntity->GetRwObject()) 
		{
			pModelInfo = CModelInfo::GetModelInfo(pEntity->m_nModelIndex);
			if(pModelInfo->GetDontWriteZBuffer())
				RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);
					
			if(pEntity->m_nFlags.bDistanceFade == true)
			{
				int32 alpha = CalculateFadingAtomicAlpha(pModelInfo, pEntity, pLink->item.dist);
				pEntity->m_nFlags.bImBeingRendered = true;

				if(!pEntity->m_nFlags.bBackfaceCulled)
					RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
				
				if(RwObjectGetType(pEntity->GetRwObject()) == rpATOMIC)
					RenderFadingAtomic(pModelInfo, (RpAtomic*)pEntity->GetRwObject(), alpha);
				else
					RenderFadingClump(pModelInfo, (RpClump*)pEntity->GetRwObject(), alpha);
					
				pEntity->m_nFlags.bImBeingRendered = false;

				if(!pEntity->m_nFlags.bBackfaceCulled)
					RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLBACK);

#if defined(GTA_PC) && !defined(FINALBUILD)
				ASSERTOBJ(gbIndexBufferError == false, pModelInfo->GetModelName(),
							"Object contains a mesh with zero vertices");
#endif
	
			}	
			else
			{
				CRenderer::RenderOneNonRoad(pEntity);
			}		

			if(pModelInfo->GetDontWriteZBuffer())
				RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) TRUE);
		}	
		pLink = pLink->GetPrevious();
	}	
}*/

//
// name:		RenderFadingEntities
// description:	Render the link list of fading entities
void CVisibilityPlugins::RenderFadingEntities()
{
	RenderOrderedList(m_alphaEntityList);
	RenderBoatAlphaAtomics();
}

//
// name:		RenderFadingEntities
// description:	Render the link list of fading entities that are under the water
void CVisibilityPlugins::RenderFadingUnderwaterEntities()
{
	RenderOrderedList(m_alphaUnderwaterEntityList);
}

//
// name:		RenderFadingEntities
// description:	Render the link list of fading entities that are under the water
void CVisibilityPlugins::RenderReallyDrawLastObjects()
{
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);		
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);	
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLNONE);

#ifdef GTA_PS2
	PS2AllRecalculateLighting();
#endif
	SetAmbientColours();
	DeActivateDirectional();
	
	RenderOrderedList(m_alphaReallyDrawLastList);
	
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
}

// -- Miscellaneous functions ---------------------------------------------------------------

//
// GetDistanceFromCamera: return the distance frame is from camera
//
float CVisibilityPlugins::GetDistanceSquaredFromCamera(RwFrame* pFrame)
{
	RwV3d *pAtomPosn = RwMatrixGetPos(RwFrameGetLTM(pFrame));
	RwV3d result;

	// get vector from camera to object
	RwV3dSub(&result, pAtomPosn, ms_pCameraPosn);
	return (result.x*result.x + result.y*result.y + result.z*result.z);
}

//
// GetDistanceFromCamera: return the distance frame is from camera
//
float CVisibilityPlugins::GetDistanceSquaredFromCamera(RwV3d* pVec)
{
	RwV3d result;

	// get vector from camera to object
	RwV3dSub(&result, pVec, ms_pCameraPosn);
	return (result.x*result.x + result.y*result.y + result.z*result.z);
}

