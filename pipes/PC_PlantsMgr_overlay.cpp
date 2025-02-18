//
// filename:	PlantsMgr_overlay.cpp
// description:	All the initialisation for the plants manager. So that it
//				can be placed in an overlay
//
//
//	22/01/2004	-	Andrzej:	- CPlantMgr::Initialise() moved to the overlay;
//								- CPlantSurfPropMgr::Initialise() moved to the overlay;
//								- VarConsole stuff moved to CGame::Initialise();
//								- CVuGrassRenderer::Initialise() moved to the overlay;
//								- CCustomGrassPipeline::CreatePipe() moved to the overlay;
//	18/03/2004	-	Andrzej:	- ReloadConfig(): added resetting for procedural obj list slot;
//	10/04/2005	-	Andrzej:	- full port to GTA_PC/GTA_XBOX;
//	15/05/2005	-	Andrzej:	- GTA_PC: grass atomics require prelit colors for correct alphablending
//									(seems like material alpha is ignored when there're no prelit colors);
//
//
//
//
//
//
//
//
//
//
#include "TxdStore.h"
#include "VarConsole.h"
#include "handy.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "CompressedFloat.h"

#include "PC_PlantsMgr.h"
#include "PC_GrassRenderer.h"
#include "PlantSurfPropMgr.h"
#include "SurfaceTable.h"
#include "handy.h"

#include "streaming.h"





//
//
//
// handy assert routines to check against loss of precision on compressed float:
//
inline void ASSERT_POS_FLOAT_RANGE(CVector& V)
{
    ASSERTMSG(V.x <= COMPRESSED_UV_FLOAT_MAX, "Grass pipe: compressed geometry X doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.x >= COMPRESSED_UV_FLOAT_MIN, "Grass pipe: compressed geometry X doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.y <= COMPRESSED_UV_FLOAT_MAX, "Grass pipe: compressed geometry Y doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.y >= COMPRESSED_UV_FLOAT_MIN, "Grass pipe: compressed geometry Y doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.z <= COMPRESSED_UV_FLOAT_MAX, "Grass pipe: compressed geometry Z doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.z >= COMPRESSED_UV_FLOAT_MIN, "Grass pipe: compressed geometry Z doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
}

inline void ASSERT_UV_FLOAT_RANGE(CVector2D& V)
{
    ASSERTMSG(V.x <= COMPRESSED_UV_FLOAT_MAX, "Grass pipe: compressed geometry U doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.x >= COMPRESSED_UV_FLOAT_MIN, "Grass pipe: compressed geometry U doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.y <= COMPRESSED_UV_FLOAT_MAX, "Grass pipe: compressed geometry V doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
    ASSERTMSG(V.y >= COMPRESSED_UV_FLOAT_MIN, "Grass pipe: compressed geometry V doesn't fit in range <"COMPRESSED_UV_FLOAT_MIN_STR", "COMPRESSED_UV_FLOAT_MAX_STR">.");
}


//
//
//
//
#define GETMESHPTRFROMMESHHDR(MESHHDR_PTR)	((RpMesh*)(((uint8*)MESHHDR_PTR)+sizeof(RpMeshHeader)+MESHHDR_PTR->firstMeshOffset))

#if defined (GTA_XBOX)
	#define GRASS_MODELS_DIR	"d:\\models\\grass\\"
#endif //GTA_XBOX...
#if defined (GTA_PC)
	#define GRASS_MODELS_DIR	"models\\grass\\"
#endif //GTA_PC...


//
//
//
//
static
RpClump* LoadPlantClump(char *filename)
{
	RpClump *pClump = NULL;

	RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
	ASSERT(stream);
	
	if(stream)
	{
   		if(RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
   		{
       		pClump = RpClumpStreamRead(stream);
   			ASSERTMSG(pClump, "Error reading plant model clump!");
   		}
	}
   	
   	RwStreamClose(stream, NULL);

	return(pClump);
}	

//
//
//
//
static
RwRaster* LoadPlantRaster(char *name, char *maskname=NULL)
{
	RwTexture *plantTexture = RwTextureRead(RWSTRING(name), RWSTRING(maskname));
	ASSERT(plantTexture);
	
	RwRaster *plantRaster = RwTextureGetRaster(plantTexture);
	ASSERT(plantRaster);
	
	return(plantRaster);
}


//
//
//
//
static
RwTexture* LoadPlantTexture(char *name, char *maskname=NULL)
{
	RwTexture *plantTexture = RwTextureRead(RWSTRING(name), RWSTRING(maskname));
	ASSERT(plantTexture);

	RwTextureSetFilterMode(plantTexture,	rwFILTERLINEAR);
	RwTextureSetAddressing(plantTexture,	rwTEXTUREADDRESSWRAP);

	return(plantTexture);
}





static
RwTexture* _plantTexture = NULL;

static
RpMaterial* setMatColourCB(RpMaterial* pMat, void* pData)
{
	RwRGBA* pCols = (RwRGBA*)pData;
	RpMaterialSetColor(pMat, pCols);
	
	// need to force the texture to set here
	RpMaterialSetTexture(pMat, _plantTexture);
	
	return(pMat);
}

static
void SetMaterialColourAndTexture(RpAtomic* pAtomic, RwRGBA Cols)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	RpGeometryForAllMaterials(pGeom, setMatColourCB, &Cols);
}

#if 0
void SetupUVs(RpAtomic* pAtomic)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	
	UInt32	i,tableSize = RpGeometryGetNumVertices(pGeom);
	RwTexCoords* pTexCoords = RpGeometryGetVertexTexCoords(pGeom,rwTEXTURECOORDINATEINDEX0);
	
	ASSERT(pTexCoords);
	
	pGeom = RpGeometryLock(pGeom, rpGEOMETRYLOCKALL);
	
	// scale the UV coords for the big texture with all the plants in it
	for(i=0;i<tableSize;i++)
	{
		pTexCoords[i].u *= CPLANT_BIGTEXTURE_U_SCALE;
		pTexCoords[i].v *= CPLANT_BIGTEXTURE_V_SCALE;
	}
	
	RpGeometryUnlock(pGeom);
}
#endif //#if 0....



//
//
// checks if atomic's geometry has prelit colors, if not new geometry is created
// and attached to atomic:
//
static
RpAtomic* Atomic_AddPrelitColorToGeometry(RpAtomic *pAtomic)
{
	RpGeometry *pOldGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pOldGeometry);

	if(RpGeometryGetFlags(pOldGeometry) & rpGEOMETRYPRELIT)
	{
		return(pAtomic); 		// not necessary to change anything
	}


	RpMorphTarget *pOldMorphTarget = RpGeometryGetMorphTarget(pOldGeometry, 0);
	ASSERT(pOldMorphTarget);

	const int32 numVerts =	RpGeometryGetNumVertices(pOldGeometry); 
	const int32 numTris =	RpGeometryGetNumTriangles(pOldGeometry);

	// clone oldGeometry and set new materials for every triangle:
	RpGeometry *pNewGeometry = RpGeometryCreate(numVerts, numTris, rpGEOMETRYTRISTRIP|rpGEOMETRYTEXTURED|rpGEOMETRYPRELIT/*|rpGEOMETRYNORMALS*/);
	ASSERT(pNewGeometry);

	RpMorphTarget *pNewMorphTarget = RpGeometryGetMorphTarget(pNewGeometry, 0);
	ASSERT(pNewMorphTarget);

	// copy vertex positions, normals, uv coords:
	{
		RwV3d *pOldVertices = RpMorphTargetGetVertices(pOldMorphTarget);
		ASSERT(pOldVertices);
		//RwV3d *pOldNormals	= RpMorphTargetGetVertexNormals(pOldMorphTarget);
		//ASSERT(pOldNormals);
		RwTexCoords *pOldUVs = RpGeometryGetVertexTexCoords(pOldGeometry, rwTEXTURECOORDINATEINDEX0);
		ASSERT(pOldUVs);

		RwV3d *pNewVertices = RpMorphTargetGetVertices(pNewMorphTarget);
		ASSERT(pNewVertices);
		//RwV3d *pNewNormals	= RpMorphTargetGetVertexNormals(pNewMorphTarget);
		//ASSERT(pNewNormals);
		RwTexCoords *pNewUVs = RpGeometryGetVertexTexCoords(pNewGeometry, rwTEXTURECOORDINATEINDEX0);
		ASSERT(pNewUVs);

		::memcpy(pNewVertices,	pOldVertices,	numVerts*sizeof(RwV3d));
		//::memcpy(pNewNormals,	pOldNormals,	numVerts*sizeof(RwV3d));
		::memcpy(pNewUVs,		pOldUVs,		numVerts*sizeof(RwTexCoords));
	}

	// copy triangles:
	{
		RpTriangle *pOldTriangles = RpGeometryGetTriangles(pOldGeometry);
		ASSERT(pOldTriangles);

		RpTriangle *pNewTriangles = RpGeometryGetTriangles(pNewGeometry);
		ASSERT(pNewTriangles);
		
		::memcpy(pNewTriangles, pOldTriangles, numTris*sizeof(RpTriangle));

		// clone materials:
		RpTriangle *pOldTri		= pOldTriangles;
		RpTriangle *pNewTri		= pNewTriangles;
		for(int32 i=0; i<numTris; i++)
		{
			RpMaterial *pMat = RpGeometryTriangleGetMaterial(pOldGeometry, pOldTri);
			RpGeometryTriangleSetMaterial(pNewGeometry, pNewTri, pMat);
			pOldTri++;
			pNewTri++;
		}
	
	}

	// unlock new geometry:
	RpGeometryUnlock(pNewGeometry);
	
	uint32 flags = RpGeometryGetFlags(pNewGeometry);
	flags |= rpGEOMETRYPOSITIONS;	
	RpGeometrySetFlags(pNewGeometry, flags);

	// destroy old geometry and attach new one:
	RpAtomicSetGeometry(pAtomic, pNewGeometry, rpATOMICSAMEBOUNDINGSPHERE);
	
	return(pAtomic);
}// end of Atomic_AddPrelitColorToGeometry()...


//
//
//
//
static
bool SetGeometryPrelitColors(RpGeometry *pGeom, CRGBA newColor)
{	
	ASSERT(pGeom);
	
	const uint32 flags = RpGeometryGetFlags(pGeom);
	if(!(flags & rpGEOMETRYPRELIT))
		return(FALSE);

	RpGeometryLock(pGeom, rpGEOMETRYLOCKALL);
	{
		// fill in vertex colors:
		RwRGBA *pColors = RpGeometryGetPreLightColors(pGeom);
		if(pColors)
		{
			const RwRGBA rgbaColor = newColor;
			const int32 count = RpGeometryGetNumVertices(pGeom);
			for(int32 i=0; i<count; i++)
			{
				pColors[i] = rgbaColor;
			}
		}
	}
	RpGeometryUnlock(pGeom);
	
	return(TRUE);
}


//
//
//
//
static
bool8 LoadGeometryForPlantSlot(RpAtomic **pDestModelsTab, char **models_fnames)
{

	for(int32 i=0; i<CPLANT_SLOT_NUM_MODELS; i++)
	{
		char _fname[128];
		::sprintf(_fname, GRASS_MODELS_DIR"%s", models_fnames[i]);
		RpClump	*pPlantModelClump = LoadPlantClump(_fname);
		ASSERT(pPlantModelClump);

		RpAtomic *pAtomic = GETATOMICFROMCLUMP(pPlantModelClump);
		ASSERTMSG(pAtomic, "Error getting plant atomic!");
		
		SetFilterModeOnAtomicsTextures(pAtomic, rwFILTERMIPLINEAR);


#if defined(GTA_PC)
		// add prelit colors to geometry (if necessary):
		Atomic_AddPrelitColorToGeometry(pAtomic);
#endif //GTA_PC...

		//grrr - I can't get the colour to come through on the material
		RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
		ASSERT(pGeom);

		RpGeometryLock(pGeom, rpGEOMETRYLOCKALL);
		{
			RwUInt32 flags = RpGeometryGetFlags(pGeom);
			flags = flags & (~rpGEOMETRYLIGHT);
			flags = flags & (~rpGEOMETRYNORMALS);
#if defined(GTA_XBOX)
			// prelit colors are not required for GTA_XBOX
			flags = flags & (~rpGEOMETRYPRELIT);
#endif //GTA_XBOX
#if defined(GTA_PC)
			// prelit colors are required for GTA_PC
			ASSERTMSG(flags & rpGEOMETRYPRELIT, "CPlantMgr: Grass atomics require GEOMETRYPRELIT colors.");		
#endif
			flags = (flags | rpGEOMETRYMODULATEMATERIALCOLOR);
			RpGeometrySetFlags(pGeom, flags);
		}
		RpGeometryUnlock(pGeom);

#if defined(GTA_PC)
		// reset vertex colors for geometry:
		SetGeometryPrelitColors(pGeom, CRGBA(255,255,255,255));
#endif


		RwRGBA newColour = {0,0,0,50};
		SetMaterialColourAndTexture(pAtomic, newColour);		//forces the texture onto the atomic
//		SetupUVs(pAtomic);
		
		// only want the atomics
		RpAtomic *pNewAtomic = RpAtomicClone(pAtomic);
		ASSERT(pNewAtomic);

		// remove the loaded plant clump
		RpClumpDestroy(pPlantModelClump);

		SetFilterModeOnAtomicsTextures(pNewAtomic, rwFILTERLINEAR);
	
		RwFrame *frame = RwFrameCreate();
		ASSERT(frame);
		RpAtomicSetFrame(pNewAtomic, frame);
	
		pDestModelsTab[i] = pNewAtomic;
	}

	return(TRUE);
}// end of LoadGeometryForPlantSlot()...



//
//
//
//
bool8 CPlantMgr::Initialise()
{
	if(!CPlantMgr::ReloadConfig())
		return(FALSE);


	//
	// Load texture dictionary
	//

	CStreaming::MakeSpaceFor(17*2048);
	CStreaming::ImGonnaUseStreamingMemory();

	CTxdStore::PushCurrentTxd();
		
	UInt32 grassTxdId = CTxdStore::FindTxdSlot("grass_pc");
		
	if (grassTxdId == -1)
		grassTxdId = CTxdStore::AddTxdSlot("grass_pc");

	CTxdStore::LoadTxd(grassTxdId, "models\\grass\\plant1.txd");
	CTxdStore::AddRef(grassTxdId);
	CTxdStore::SetCurrentTxd(grassTxdId);
	{
		PC_PlantTextureTab0[0] = LoadPlantTexture("txgrass0_0");
		PC_PlantTextureTab0[1] = LoadPlantTexture("txgrass0_1");
		PC_PlantTextureTab0[2] = LoadPlantTexture("txgrass0_2");
		PC_PlantTextureTab0[3] = LoadPlantTexture("txgrass0_3");

		PC_PlantTextureTab1[0] = LoadPlantTexture("txgrass1_0");
		PC_PlantTextureTab1[1] = LoadPlantTexture("txgrass1_1");
		PC_PlantTextureTab1[2] = LoadPlantTexture("txgrass1_2");
		PC_PlantTextureTab1[3] = LoadPlantTexture("txgrass1_3");

		//need to set up the texture and force it to be used by the atomics
		// material. Can't get the texture to render on the atomic otherwise...
		_plantTexture = 	LoadPlantTexture("gras07Si");	//RwTextureCreate(LoadPlantRaster("gras07Si"));		//RwTextureCreate(PlantSlotRasterTab[1]);
		ASSERT(_plantTexture);
	}
	CTxdStore::PopCurrentTxd();

	// finished loading the textures in...
	CStreaming::IHaveUsedStreamingMemory();

	PC_PlantSlotTextureTab[0] = &PC_PlantTextureTab0[0];
	PC_PlantSlotTextureTab[1] = &PC_PlantTextureTab1[0];
	PC_PlantSlotTextureTab[2] = &PC_PlantTextureTab0[0];	// 2
	PC_PlantSlotTextureTab[3] = &PC_PlantTextureTab1[0];	// 3




char* models_fnames[CPLANT_SLOT_NUM_MODELS];

	models_fnames[0] = "grass0_1.dff";
	models_fnames[1] = "grass0_2.dff";
	models_fnames[2] = "grass0_3.dff";
	models_fnames[3] = "grass0_4.dff";
//	models_fnames[0] = models_fnames[1] = models_fnames[2] = models_fnames[3] = "plant1.dff";
	if(!LoadGeometryForPlantSlot(PC_PlantModelsTab0, models_fnames))
		return(FALSE);

	models_fnames[0] = "grass1_1.dff";
	models_fnames[1] = "grass1_2.dff";
	models_fnames[2] = "grass1_3.dff";
	models_fnames[3] = "grass1_4.dff";
//	models_fnames[0] = models_fnames[1] = models_fnames[2] = models_fnames[3] = "plant1.dff";
	if(!LoadGeometryForPlantSlot(PC_PlantModelsTab1, models_fnames))
		return(FALSE);

/*	models_fnames[0] = "grass2_1.dff";
	models_fnames[1] = "grass2_2.dff";
	models_fnames[2] = "grass2_3.dff";
	models_fnames[3] = "grass2_4.dff";
//	models_fnames[0] = models_fnames[1] = models_fnames[2] = models_fnames[3] = "plant1.dff";
	if(!LoadGeometryForPlantSlot(PC_PlantModelSlotTab[2], models_fnames))
		return(FALSE);*/

/*	models_fnames[0] = "grass3_1.dff";
	models_fnames[1] = "grass3_2.dff";
	models_fnames[2] = "grass3_3.dff";
	models_fnames[3] = "grass3_4.dff";
//	models_fnames[0] = models_fnames[1] = models_fnames[2] = models_fnames[3] = "plant1.dff";
	if(!LoadGeometryForPlantSlot(PC_PlantModelSlotTab[3], models_fnames))
		return(FALSE);*/


	PC_PlantModelSlotTab[0] = &PC_PlantModelsTab0[0];
	PC_PlantModelSlotTab[1] = &PC_PlantModelsTab1[0];
	PC_PlantModelSlotTab[2] = &PC_PlantModelsTab0[0];	//2;
	PC_PlantModelSlotTab[3] = &PC_PlantModelsTab1[0];	//3;


	//PC - need to set the association between tables and sets somewhere?...
	CGrassRenderer::SetPlantModelsTab(PPPLANTBUF_MODEL_SET0,	PC_PlantModelSlotTab[0]);
	CGrassRenderer::SetPlantModelsTab(PPPLANTBUF_MODEL_SET1,	PC_PlantModelSlotTab[0]);	//1]);
	CGrassRenderer::SetPlantModelsTab(PPPLANTBUF_MODEL_SET2,	PC_PlantModelSlotTab[0]);	//2]);
	CGrassRenderer::SetPlantModelsTab(PPPLANTBUF_MODEL_SET3,	PC_PlantModelSlotTab[0]);	//3]);


	CGrassRenderer::SetCloseFarAlphaDist(CPLANT_ALPHA_CLOSE_DIST, CPLANT_ALPHA_FAR_DIST);
	return(TRUE);
}// end of CPlantMgr::Initialise()...


//
//
//
//
bool8 CPlantMgr::ReloadConfig()
{

	//
	// load surface properties:
	//
	if(!CPlantSurfPropMgr::Initialise())
		return(FALSE);


	for(int32 i=0; i<CPLANT_NUM_PLANT_SLOTS; i++)
	{
		m_CloseLocTriListHead[i]	= NULL;
	}
	
//	m_CloseLocTriListHead[CPLANT_PROC_OBJ_SLOT] = NULL;
	
	
	m_UnusedLocTriListHead	= &m_LocTrisTab[0];


	for(int32 i=0; i<CPLANT_MAX_CACHE_LOC_TRIS_NUM; i++)
	{
		CPlantLocTri *pLocTri = &m_LocTrisTab[i];

		pLocTri->m_V1			= CVector(0, 0, 0);
		pLocTri->m_V2			= CVector(0, 0, 0);
		pLocTri->m_V3			= CVector(0, 0, 0);
		pLocTri->m_Center		= CVector(0, 0, 0);

		pLocTri->m_nSurfaceType			= 0;


		for(int32 n=0; n<CPLANT_SURF_PROP_PLANTDATA_NUM; n++)
		{
			pLocTri->m_nMaxNumPlants[n]	= 0;
		}


		if(i==0)
			m_LocTrisTab[i].m_pPrevTri = NULL;
		else
			m_LocTrisTab[i].m_pPrevTri = &m_LocTrisTab[i-1];
		
		if(i==CPLANT_MAX_CACHE_LOC_TRIS_NUM-1)
			m_LocTrisTab[i].m_pNextTri = NULL;
		else
			m_LocTrisTab[i].m_pNextTri = &m_LocTrisTab[i+1];
	}



	m_CloseColEntListHead	= NULL;
	m_UnusedColEntListHead	= &m_ColEntCacheTab[0];

	for(int32 i=0; i<CPLANT_COL_ENTITY_CACHE_SIZE; i++)
	{
		CPlantColEntEntry *pEntry = &m_ColEntCacheTab[i];

		pEntry->m_pEntity		= NULL;
		pEntry->m_LocTriArray	= NULL;
		pEntry->m_nNumTris		= 0;

		if(i==0)
			m_ColEntCacheTab[i].m_pPrevEntry = NULL;
		else
			m_ColEntCacheTab[i].m_pPrevEntry = &m_ColEntCacheTab[i-1];
		
		if(i==CPLANT_COL_ENTITY_CACHE_SIZE-1)
			m_ColEntCacheTab[i].m_pNextEntry = NULL;
		else
			m_ColEntCacheTab[i].m_pNextEntry = &m_ColEntCacheTab[i+1];
	}

	return(TRUE);
}// end of CPlantMgr::ReloadConfig()...











#pragma mark -




//
//
//
//

/*
struct
{
	char	*name;
	uint16	surf_id;
}
tabNameToSurfID[CPLANT_MAX_SURFPROP_TAB_SIZE] = 
{
//	"DEFAULT",				COLPOINT_SURFACETYPE_DEFAULT,
	"TARMAC_FUCKED",		COLPOINT_SURFACETYPE_TARMAC_FUCKED,
	"TARMAC_REALLYFUCKED",	COLPOINT_SURFACETYPE_TARMAC_REALLYFUCKED,
	"PAVEMENT_FUCKED",		COLPOINT_SURFACETYPE_PAVEMENT_FUCKED,
	"GRAVEL",				COLPOINT_SURFACETYPE_GRAVEL,
	"FUCKED_CONCRETE",		COLPOINT_SURFACETYPE_FUCKED_CONCRETE,
	"GRASS_SHORT_LUSH",		COLPOINT_SURFACETYPE_GRASS_SHORT_LUSH,
	"GRASS_MEDIUM_LUSH",	COLPOINT_SURFACETYPE_GRASS_MEDIUM_LUSH,
	"GRASS_LONG_LUSH",		COLPOINT_SURFACETYPE_GRASS_LONG_LUSH,
	"GRASS_SHORT_DRY",		COLPOINT_SURFACETYPE_GRASS_SHORT_DRY,
	"GRASS_MEDIUM_DRY",		COLPOINT_SURFACETYPE_GRASS_MEDIUM_DRY,
	"GRASS_LONG_DRY",		COLPOINT_SURFACETYPE_GRASS_LONG_DRY,
	"GOLFGRASS_ROUGH",		COLPOINT_SURFACETYPE_GOLFGRASS_ROUGH,
	"GOLFGRASS_SMOOTH",		COLPOINT_SURFACETYPE_GOLFGRASS_SMOOTH,
	"STEEP_SLIDYGRASS",		COLPOINT_SURFACETYPE_STEEP_SLIDYGRASS,
	"STEEP_CLIFF",			COLPOINT_SURFACETYPE_STEEP_CLIFF,
	"FLOWERBED",			COLPOINT_SURFACETYPE_FLOWERBED,
	"MEADOW",				COLPOINT_SURFACETYPE_MEADOW,
	"WOODLANDGROUND",		COLPOINT_SURFACETYPE_WOODLANDGROUND,
	"VEGETATION",			COLPOINT_SURFACETYPE_VEGETATION,
	
	"WASTEGROUND",			COLPOINT_SURFACETYPE_WASTEGROUND,
	"MUD_DRY",				COLPOINT_SURFACETYPE_MUD_DRY,
	"DIRT",					COLPOINT_SURFACETYPE_DIRT,
	"DIRTTRACK",				COLPOINT_SURFACETYPE_DIRTTRACK,	
	"SAND_DEEP",			COLPOINT_SURFACETYPE_SAND_DEEP,
	"SAND_MEDIUM",			COLPOINT_SURFACETYPE_SAND_MEDIUM,
	"SAND_COMPACT",			COLPOINT_SURFACETYPE_SAND_COMPACT,
	"SAND_ARID",				COLPOINT_SURFACETYPE_SAND_ARID,
	"SAND_MORE",			COLPOINT_SURFACETYPE_SAND_MORE,
	"ROCK_DRY",				COLPOINT_SURFACETYPE_ROCK_DRY,
	"CORNFIELD",				COLPOINT_SURFACETYPE_CORNFIELD,
	"P_SAND",				COLPOINT_SURFACETYPE_P_SAND,			
	"P_SAND_DENSE",			COLPOINT_SURFACETYPE_P_SAND_DENSE,	
	"P_SAND_ARID",			COLPOINT_SURFACETYPE_P_SAND_ARID,
	"P_SAND_COMPACT",		COLPOINT_SURFACETYPE_P_SAND_COMPACT,		
	"P_GRASS_SHORT",		COLPOINT_SURFACETYPE_P_GRASS_SHORT,
	"P_GRASS_MEADOW",		COLPOINT_SURFACETYPE_P_GRASS_MEADOW,
	"P_GRASS_DRY",			COLPOINT_SURFACETYPE_P_GRASS_DRY,		
	"P_WOODLAND",			COLPOINT_SURFACETYPE_P_WOODLAND,		
	"P_WOODDENSE",			COLPOINT_SURFACETYPE_P_WOODDENSE,		
	"P_ROADSIDE",			COLPOINT_SURFACETYPE_P_ROADSIDE,		
	"P_ROADSIDEDES",		COLPOINT_SURFACETYPE_P_ROADSIDEDES,
	"P_GRASSWEEFLOWERS",	COLPOINT_SURFACETYPE_P_GRASSWEEFLOWERS,
	"P_GRASSDRYTALL",		COLPOINT_SURFACETYPE_P_GRASSDRYTALL,
	"P_GRASSLUSHTALL",		COLPOINT_SURFACETYPE_P_GRASSLUSHTALL,
	"P_GRASSGRNMIX",		COLPOINT_SURFACETYPE_P_GRASSGRNMIX,
	"P_GRASSBRNMIX",		COLPOINT_SURFACETYPE_P_GRASSBRNMIX,
	"P_GRASSLOW",			COLPOINT_SURFACETYPE_P_GRASSLOW,
	"P_GRASSROCKY",			COLPOINT_SURFACETYPE_P_GRASSROCKY,
	"P_GRASSSMALLTREES",	COLPOINT_SURFACETYPE_P_GRASSSMALLTREES,
	"P_GRASSLIGHT",			COLPOINT_SURFACETYPE_P_GRASSLIGHT,
	"P_GRASSLIGHTER",		COLPOINT_SURFACETYPE_P_GRASSLIGHTER,			
	"P_GRASSLIGHTER2",		COLPOINT_SURFACETYPE_P_GRASSLIGHTER2,
	"P_GRASSMID1",			COLPOINT_SURFACETYPE_P_GRASSMID1,		
	"P_GRASSMID2",			COLPOINT_SURFACETYPE_P_GRASSMID2,		
	"P_GRASSDARK",			COLPOINT_SURFACETYPE_P_GRASSDARK,	
	"P_GRASSDARK2",			COLPOINT_SURFACETYPE_P_GRASSDARK2,	
	"P_GRASSDIRTMIX"	,		COLPOINT_SURFACETYPE_P_GRASSDIRTMIX
	
//	NULL,					0
};
*/


//
// number of columns in "Plants.dat":
//
#define NUM_ITEMS_OF_HANDLING_DATA		(18)






//
//
//
//
bool8 CPlantSurfPropMgr::Initialise()
{


	//
	// restart allocation routine:
	//
	CPlantSurfPropMgr::AllocSurfProperties(0, TRUE);

	for(int32 i=0; i<CPLANT_SURF_PROPERTIES_NUM; i++)
	{
		m_SurfPropPtrTab[i] = NULL;
	}


	for(int32 i=0; i<CPLANT_MAX_SURFPROP_TAB_SIZE; i++)
	{
		CPlantSurfProp *pSurfProp = &m_SurfPropTab[i];

		pSurfProp->m_nPlantSlotID						= 0;
		

		for(int32 n=0; n<CPLANT_SURF_PROP_PLANTDATA_NUM; n++)
		{
			pSurfProp->m_PlantData[n].m_nModelID		= CPLANT_SURF_PROP_INVALID_MODELID;

			//pSurfProp->m_PlantData[n].m_vecOffsetUV	= CVector2D(0.0f, 0.0f);
			pSurfProp->m_PlantData[n].m_nTextureID		= 0;
			pSurfProp->m_PlantData[n].m_rgbaColor		= CRGBA(255, 255, 255, 255);	// R,G,B,A

			pSurfProp->m_PlantData[n].m_nIntensity		= 255;
			pSurfProp->m_PlantData[n].m_nIntensityVar	= 0;

			pSurfProp->m_PlantData[n].m_fScaleXY		= 1.0f;
			pSurfProp->m_PlantData[n].m_fScaleZ			= 1.0f;
			pSurfProp->m_PlantData[n].m_fScaleVarXY		= 0.0f;
			pSurfProp->m_PlantData[n].m_fScaleVarZ		= 0.0f;

			pSurfProp->m_PlantData[n].m_fWindBendScale	= 0.0f;
			pSurfProp->m_PlantData[n].m_fWindBendVar	= 0.0f;

			pSurfProp->m_PlantData[n].m_fDensity		= 0.0f;
		}
	}


	if(!CPlantSurfPropMgr::LoadPlantsDat("PLANTS.DAT"))
	{
		ASSERTMSG(FALSE, "Error loading 'Plants.dat'. See Andrzej to fix this.");
		return(FALSE);
	}	

	return(TRUE);
}// end of CPlantSurfPropMgr::Initialise()...


//
// simple table "allocator" for SurfaceProperties:
//
//
CPlantSurfProp* CPlantSurfPropMgr::AllocSurfProperties(uint16 nSurfaceType, bool8 restartAlloc)
{
	// restart allocating:
	if(restartAlloc)
	{
		m_countSurfPropsAllocated = 0;
		return(NULL);
	}

	if(m_countSurfPropsAllocated >= CPLANT_MAX_SURFPROP_TAB_SIZE)
	{
		ASSERTMSG(FALSE, "No more space in m_SurfPropTab[]!");
		return(NULL);
	}
	
	CPlantSurfProp *pSurfProp = &m_SurfPropTab[m_countSurfPropsAllocated];
	m_countSurfPropsAllocated++;
	
	m_SurfPropPtrTab[nSurfaceType] = pSurfProp;
	
	return(pSurfProp);
}




//
// takes UVoffsetID and returns UV vector:
//
//; *********************
//; *  0 *  1 *  2 *  3 *
//; *********************
//; *  4 *  5 *  6 *  7 *
//; *********************
//; *  8 *  9 * 10 * 11 *
//; *********************
//; * 12 * 13 * 14 * 15 *
//; *********************
//
//
//  ******
//  *  0 *
//  ******
//  *  1 *
//  ******
//  *  2 *
//  ******
//  * .  *
//  ******
//  * 14 *
//  ******
//  * 15 *
//  ******
//
static
void _DecodeUVoffID2UVvect(CVector2D *pVecUV, uint16 offsetID)
{
	CVector2D uv;

/*
#define SET_UV(VEC, U, V)	{VEC.x=U; VEC.y=V;}
	switch(offsetID)
	{
		case( 0):	SET_UV(uv,	0.0f,	0.0f);	break;
		case( 1):	SET_UV(uv,	0.25f,	0.0f);	break;
		case( 2):	SET_UV(uv,	0.5,	0.0f);	break;
		case( 3):	SET_UV(uv,	0.75f,	0.0f);	break;

		case( 4):	SET_UV(uv,	0.0f,	0.25f);	break;
		case( 5):	SET_UV(uv,	0.25f,	0.25f);	break;
		case( 6):	SET_UV(uv,	0.5,	0.25f);	break;
		case( 7):	SET_UV(uv,	0.75f,	0.25f);	break;

		case( 8):	SET_UV(uv,	0.0f,	0.5f);	break;
		case( 9):	SET_UV(uv,	0.25f,	0.5f);	break;
		case(10):	SET_UV(uv,	0.5,	0.5f);	break;
		case(11):	SET_UV(uv,	0.75f,	0.5f);	break;

		case(12):	SET_UV(uv,	0.0f,	0.75f);	break;
		case(13):	SET_UV(uv,	0.25f,	0.75f);	break;
		case(14):	SET_UV(uv,	0.5,	0.75f);	break;
		case(15):	SET_UV(uv,	0.75f,	0.75f);	break;
	}
#undef SET_UV
*/



	// strip texture version: 64x256:
	const float _txdy = CPLANT_BIGTEXTURE_V_SCALE;
	uv.x = 0.0f;
	uv.y = _txdy * float(offsetID);


	*(pVecUV) = uv;
}




//
//
// helper function to find connection between Surface name and its id:
//
/*
static
uint16 _GetSurfTypeFromName(const char *pName)
{
	if(!pName)
		return(0);

	for(int32 i=0; i<CPLANT_MAX_SURFPROP_TAB_SIZE; i++)
	{
		if(!::strcmp(tabNameToSurfID[i].name, pName))
		{
			return(tabNameToSurfID[i].surf_id);
		}
	}

	// not found:
	return(0);
}
*/

//
//
//
//
bool8 CPlantSurfPropMgr::LoadPlantsDat(const char *pFilename)
{

	// load data from handling file
	CFileMgr::SetDir("DATA");
	int32 fid = CFileMgr::OpenFile((char*)pFilename, "r");
	ASSERT(fid != 0);
	CFileMgr::SetDir("");

	//
	//
	//
	char*	pLine;
	uint32 	tokenCount = 0;
	int32	lineCount = 0;
	
	// read a line at a time putting vlues into the handling data structure
	while((pLine = CFileLoader::LoadLine(fid)) != NULL)
	{
		lineCount++;

		// check for end of file - don't yet know how to check the file length (TBD)
		if(!strcmp(pLine, ";the end"))
			break;
		// ignore comments
		else if (pLine[0] == ';')
			continue;

		// parse line of file which contains data
		if(TRUE)
		{
			tokenCount = 0;
			char seps[] = " \t";
			char *token = ::strtok(pLine, seps);	// get first token
			
			uint16					nSurfaceType		= 0;
			CPlantSurfProp			*pSurfProp			= NULL;
			uint16					nPlantCoverDefID	= 0;
			CPlantSurfPropPlantData	*pSurfPropPlantData	= NULL; 
			
			do {
			
				switch(tokenCount)
				{
					case( 0):
					{
//						nSurfaceType = _GetSurfTypeFromName(token);
						nSurfaceType = g_surfaceInfos.GetSurfaceIdFromName(token);
						
						// bad name / data?
						if(nSurfaceType == 0)
						{
							char msg[128];
							::sprintf(msg, "Unknown surface name '%s' in 'Plants.dat' (line %d)! See Andrzej to fix this.", token, lineCount);
							ASSERTMSG(FALSE, msg);
							return(FALSE);
						}
						
						// try to get already allocated struct first:
						pSurfProp = CPlantSurfPropMgr::GetSurfProperties(nSurfaceType);
						if(!pSurfProp)
						{
							// if not allocated, then allocate it:
							pSurfProp = CPlantSurfPropMgr::AllocSurfProperties(nSurfaceType);
						}
						

						if(!pSurfProp)
						{
							ASSERTMSG(FALSE, "Error allocating SurfaceProperties for specified name! See Andrzej to fix this.");
							return(FALSE);
						}
					}
					break;

					case( 1):
					{
						nPlantCoverDefID	= ::atoi(token);
						if(nPlantCoverDefID > CPLANT_SURF_PROP_PLANTDATA_NUM-1)
							nPlantCoverDefID = 0;
						pSurfPropPlantData = &pSurfProp->m_PlantData[nPlantCoverDefID];
					}
					break;
					
					case( 2):
					{
						uint16 slot_id = ::atoi(token);
						ASSERT(slot_id < CPLANT_NUM_PLANT_SLOTS);
						pSurfProp->m_nPlantSlotID = slot_id;		
					}	
					break;


					//
					// particular plant data follow from here:
					//
					case( 3):
					{
						uint16 model_id = ::atoi(token);
						ASSERT(model_id < CPLANT_SLOT_NUM_MODELS);
						pSurfPropPlantData->m_nModelID = model_id;
					}	
					break;
					
					case( 4):
					{
						uint16 uvOffsetID = ::atoi(token);
						ASSERT(uvOffsetID < CPLANT_SLOT_NUM_TEXTURES);
						//_DecodeUVoffID2UVvect(&pSurfPropPlantData->m_vecOffsetUV, uvOffsetID);
						pSurfPropPlantData->m_nTextureID = uvOffsetID;
					}
					break;
					
					case( 5):	pSurfPropPlantData->m_rgbaColor.red		= ::atoi(token);		break;
					case( 6):	pSurfPropPlantData->m_rgbaColor.green	= ::atoi(token);		break;
					case( 7):	pSurfPropPlantData->m_rgbaColor.blue	= ::atoi(token);		break;
					
					case( 8):	pSurfPropPlantData->m_nIntensity		= ::atoi(token);		break;
					case( 9):	pSurfPropPlantData->m_nIntensityVar		= ::atoi(token);		break;
					
					case(10):	pSurfPropPlantData->m_rgbaColor.alpha	= ::atoi(token);		break;
					
					case(11):	pSurfPropPlantData->m_fScaleXY			= ::atof(token);		break;
					case(12):	pSurfPropPlantData->m_fScaleZ			= ::atof(token);		break;
					case(13):	pSurfPropPlantData->m_fScaleVarXY		= ::atof(token);		break;
					case(14):	pSurfPropPlantData->m_fScaleVarZ		= ::atof(token);		break;
					
					case(15):	pSurfPropPlantData->m_fWindBendScale	= ::atof(token);		break;
					case(16):	pSurfPropPlantData->m_fWindBendVar		= ::atof(token);		break;
					
					case(17):	pSurfPropPlantData->m_fDensity			= ::atof(token);		break;

				}//switch(count)...			

				token = ::strtok(NULL, seps);	// get next token
				tokenCount++;
				
			} while(token);
		
			// too less data?
			if(tokenCount < NUM_ITEMS_OF_HANDLING_DATA)
				return(FALSE);
		
		}//parse line...
		
	}//while((pLine = CFileLoader::LoadLine(fid)) != NULL)...

	
	CFileMgr::CloseFile(fid);

	return(TRUE);
}// end of CPlantSurfPropMgr::LoadPlantsDat()...


//-------------------------------- other classes
//
//
//
//
bool CGrassRenderer::Initialise()
{
	// do nothing
	return(TRUE);
}



