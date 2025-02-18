//
// PlantsMgr - management of plants seeded around camera;
//
//
//	30/07/2004	- JohnW:	- initial;
//	10/04/2005	- Andrzej:	- full port to GTA_PC/GTA_XBOX;
//
//
//
//
#ifndef __CPLANTSMGR_H__
#define __CPLANTSMGR_H__


#include "ColData.h"			// ColData...
#include "PlantSurfPropMgr.h"	// CPLANT_SURF_PROP_PLANTDATA_NUM...


class CPlantColEntEntry;
class CPlantLocTri;
class CPlantMgr;
class CEntity;
class CAtomicModelInfo;

//
// stuff for VarConsole:
//
#ifndef MASTER
	extern bool8	gbShowCPlantMgrPolys;
	extern bool8	gbDisplayCPlantMgrInfo;
	extern bool8	gbPlantMgrActive;
#endif


//
//
// number of grass GroupModels with "big" textures:
// each group consist of 4 models + 1 "big" texture;
//
#define CPLANT_NUM_PLANT_SLOTS				(4)

//
// num of models per slot:
//
#define CPLANT_SLOT_NUM_MODELS				(4)

//
// num of textures per slot:
//
#define CPLANT_SLOT_NUM_TEXTURES			(4)




//
// extra last slot is used exlusively for procedural objects (Mark N):
//
//#define CPLANT_PROC_OBJ_SLOT				(4)



//
// "bigtexture" plant texture scale:
//
// no change for u:
#define CPLANT_BIGTEXTURE_U_SCALE			(1.0f)

// 1/16 for 64x1024 (16 textures, 64x64 each)
//#define CPLANT_BIGTEXTURE_V_SCALE			(1.0f/16.0f)
//
// 1/4  for 64x256  (4 textures, 64x64 each)
#define CPLANT_BIGTEXTURE_V_SCALE			(1.0f/4.0f)




//
//
// distance where TriLocs are considered to be used as base for plant sectors:
//
#define CPLANT_TRILOC_FAR_DIST				(100.0f)
#define CPLANT_TRILOC_FAR_DIST_SQR			(CPLANT_TRILOC_FAR_DIST*CPLANT_TRILOC_FAR_DIST)


#define CPLANT_ALPHA_CLOSE_DIST				(3.0f)			//(5.0f)		//(20.0f)		// fade start range
#define CPLANT_ALPHA_FAR_DIST				(60.0f)			//(100.0f)		// fade stop range



//
//
// min distance, where CEntity will be added to our internal Entity "cache":
//
#define CPLANT_COL_ENTITY_DIST			(220.0f + 120.0f)
//(220.0f + 60.0f)
#define CPLANT_COL_ENTITY_DIST_SQR		(CPLANT_COL_ENTITY_DIST*CPLANT_COL_ENTITY_DIST)



//
//
// number of entities hold in our internal CEntity cache:
//
#define CPLANT_COL_ENTITY_CACHE_SIZE	(32+8)



//
//
// update cache every 32th frame (must be power-of-2):
//
#define CPLANT_COL_ENTITY_UPDATE_CACHE	(32)


//
//
//
//
struct PPTriPlant
{
public:
    CVector		V1, V2, V3;			// 3 vertices
	CVector		center;				// center of the plant triangle		

    uint16		model_id;			// model_id used to calculate offset address for model in VU mem
	uint16		num_plants;			// num plants to generate

    RwV2d		scale;				// x=SxSy, y=Sz
	//uint16		texture_id;		// number of texture to use
	RwTexture	*texture_ptr;		// ptr to texture to use


    RwRGBA		color;				// color of the model
    
    uint8		intensity;			// scale value for the colour
    uint8		intensity_var;		// variation of intensity value
    
    float		seed;				// seed starting value for this triangle;
    float		scale_var_xy;		// scale variation XY
    float		scale_var_z;		// scale variation Z

	float		wind_bend_scale;	// wind bending scale
	float		wind_bend_var;		// wind bending variation
};



//
//
//
//
class CPlantColEntEntry
{
	friend class CPlantMgr;

public:
	CPlantColEntEntry()		{ }
	~CPlantColEntEntry()	{ }


public:
	CPlantColEntEntry*	AddEntry(CEntity *pEntity);
	void				ReleaseEntry();


public:
	CEntity				*m_pEntity;			// pointer to CEntity
	CPlantLocTri		**m_LocTriArray;	// array of size [pEntity->GetColModel().m_nNoOfTriangles],
											// it holds pointers to CPlantLocTri for every CColTriangle in CEntity's ColModel;
	uint16				m_nNumTris;			// size of above array


private:
	CPlantColEntEntry	*m_pNextEntry;
	CPlantColEntEntry	*m_pPrevEntry;
	
};





//
//
// max number of triangles kept in loc triangle cache:
//
#define CPLANT_MAX_CACHE_LOC_TRIS_NUM			(256)

//
// process every 8th TriLoc (must be power-of-2):
//
#define CPLANT_ENTRY_TRILOC_PROCESS_UPDATE		(8)

//
// unique mask value to mark case, when we want all LocTris to be processed:
//
#define CPLANT_ENTRY_TRILOC_PROCESS_ALWAYS		(0xFAFAFAFA)





//
// 
// "Plant Location" triangle:
// it's generated from ColModel data;
//
class CPlantLocTri
{
	friend class CPlantMgr;

public:
	CPlantLocTri()		{ }
	~CPlantLocTri()		{ }


public:
	CPlantLocTri*	Add(const RwV3d& v1, const RwV3d& v2, const RwV3d& v3, uint8 nSurfaceType, uint8 nLighting, bool8 createsPlants, bool8 createsObjects);
	void			Release();



public:
	CVector			m_V1, m_V2, m_V3;									// 3 vertices of triangle
	CVector			m_Center;
	float			m_SphereRadius;										// radius of the sphere containing triangle
	
	float			m_Seed			[CPLANT_SURF_PROP_PLANTDATA_NUM];	// constant seed value for the triangle
	uint16			m_nMaxNumPlants	[CPLANT_SURF_PROP_PLANTDATA_NUM];	// max number of plants to generate for this TriLoc
	
	uint8			m_nSurfaceType;										// copied from CColTriangle;
	uint8			m_nLighting;										// lighting (calculated from CColTriangle);

//	bool8			m_IsUsedByProcObj;

	bool8			m_createsPlants		: 1;
	bool8			m_createsObjects	: 1;
	bool8			m_createdObjects	: 1;
	uint8			m_pad				: 5;

private:
	CPlantLocTri	*m_pNextTri;
	CPlantLocTri	*m_pPrevTri;
};







//
//
//
//
class CPlantMgr
{
	friend class	CPlantLocTri;
	friend class	CPlantColEntEntry;



public:
	CPlantMgr();
	~CPlantMgr();

public:
	static bool8	Initialise();
	static void		Shutdown();

	static bool8	ReloadConfig();

public:
	static bool8	Update(const CVector& camPos);
	static bool8	PreUpdateOnceForNewCameraPos(const CVector& newCamPos);

	static void		Render();

	static void		UpdateAmbientColor();


private:
	static float	CalculateWindBending();


public:
	static bool8 	SetPlantFriendlyFlagInAtomicMI(CAtomicModelInfo *pModelInfo);



private:
	static bool8				_ColEntityCache_Update(const CVector& camPos, bool8 bQuickUpdate=FALSE);
	static CPlantColEntEntry*	_ColEntityCache_FindInCache(CEntity *pEntity);
	static CPlantColEntEntry*	_ColEntityCache_Add(CEntity *pEntity, bool8 bCheckCacheFirst=FALSE);
	static void					_ColEntityCache_Remove(CEntity *pEntity);
	
private:
	static bool8				_ProcessEntryCollisionData(CPlantColEntEntry *pEntry);

	static bool8				_UpdateLocTris(const CVector &camPos, int32 iTriProcessSkipMask);


	static bool8				_ProcessEntryCollisionData(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask);


	static bool8				_ProcessEntryCollisionDataSections(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask);
	static void					_ProcessEntryCollisionDataSections_AddLocTris(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask,
														int32 colStartIndex, int32 colEndIndex);
	static void					_ProcessEntryCollisionDataSections_RemoveLocTris(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask,
															int32 colStartIndex, int32 colEndIndex);


private:
	//
	// particular debug stuff:
	//
	static bool8				DbgRenderLocTris();
	static bool8				DbgRenderCachedEntities(uint32 *pCountAll=NULL);

	static bool8				DbgCountLocTrisAndPlants(uint32 groupID, uint32 *pCountLocTris=NULL, uint32 *pCountPlants=NULL);
	static bool8				DbgCountCachedEntities(uint32 *pCountAll=NULL);
	


	//
	// CPlantColEntityEntry:
	//
private:
	static CPlantColEntEntry*	MoveColEntToList(CPlantColEntEntry **ppCurrentList, CPlantColEntEntry **ppNewList, CPlantColEntEntry *pEntity);

private:
	static CPlantColEntEntry	*m_UnusedColEntListHead;
	static CPlantColEntEntry	*m_CloseColEntListHead;
	static CPlantColEntEntry	m_ColEntCacheTab[CPLANT_COL_ENTITY_CACHE_SIZE];


	//
	// CPlantLocTri:
	//
private:
	static CPlantLocTri* MoveLocTriToList(CPlantLocTri **ppCurrentList, CPlantLocTri **ppNewList, CPlantLocTri *pTri);

private:
	static CPlantLocTri	*m_UnusedLocTriListHead;
	static CPlantLocTri	*m_CloseLocTriListHead[CPLANT_NUM_PLANT_SLOTS];	// each grass GroupModel has its own TriLoc close list;
	static CPlantLocTri	m_LocTrisTab[CPLANT_MAX_CACHE_LOC_TRIS_NUM];



	//
	// stuff used to skip unnecessary PlantModelsSet upload many times during one frame: 
	//
private:
	static void		AdvanceCurrentScanCode()			{ m_scanCode++;			}
	static uint16	GetCurrentScanCode()				{ return(m_scanCode);	}


private:
	static uint16	m_scanCode;
	static CRGBA	m_AmbientColor;
	


public:
	static RwTexture*					PC_PlantTextureTab0[CPLANT_SLOT_NUM_TEXTURES];
	static RwTexture*					PC_PlantTextureTab1[CPLANT_SLOT_NUM_TEXTURES];
	static RwTexture*					PC_PlantTextureTab2[CPLANT_SLOT_NUM_TEXTURES];
	static RwTexture*					PC_PlantTextureTab3[CPLANT_SLOT_NUM_TEXTURES];

	static RwTexture**					PC_PlantSlotTextureTab[CPLANT_NUM_PLANT_SLOTS];

public:
	static RpAtomic*					PC_PlantModelsTab0[CPLANT_SLOT_NUM_MODELS];
	static RpAtomic*					PC_PlantModelsTab1[CPLANT_SLOT_NUM_MODELS];
	static RpAtomic*					PC_PlantModelsTab2[CPLANT_SLOT_NUM_MODELS];
	static RpAtomic*					PC_PlantModelsTab3[CPLANT_SLOT_NUM_MODELS];
	
	static RpAtomic**					PC_PlantModelSlotTab[CPLANT_NUM_PLANT_SLOTS];

};



#endif//__CPLANTSMGR_H___


