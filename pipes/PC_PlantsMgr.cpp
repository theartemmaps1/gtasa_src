//
// CPlantManager - takes care about plants generated around camera;
//
//
//	30/07/2004	-	JohnW:		PC version of this stuff
//	15/02/2005	-	Andrzej:	Shutdown(): fixed mem leak (entity cache cleaning added);
//	10/04/2005	-	Andrzej:	- full port to GTA_PC/GTA_XBOX;
//
//
//
//
//
//
//
#include "MemoryMgr.h"
#include "general.h"
#include "camera.h"
#include "ColPoint.h"		//CColPoint...
#include "ColTriangle.h"	//CColTriangle...
#include "ColModel.h"		//CColModel...
#include "World.h"			//ProcessVerticalLine()...
#include "TxdStore.h"		// CTxdStore{}...
#include "main.h"			// SCREEN_WIDTH, SCREEN_HEIGHT
#include "AtomicModelInfo.h"	// CAtomicModelInfo{}

#include "TimeCycle.h"		//CTimeCycle{}
#include "mblur.h"
#include "weather.h"		//CWeather{}
#include "handy.h"			// GETATOMICFROMCLUMP()...


#include "PC_PlantsMgr.h"
#include "PC_GrassRenderer.h"
#include "PlantSurfPropMgr.h"

#include "ProcObjects.h"
#include "SurfaceTable.h"

#include "Entity.h"			// REGREF(), TIDYREF()

#include "fx.h"


//
//
//
//
#define USE_COLLISION_SECTIONS		(1)		// define this to use colmodel sections to improve processing

//#define USE_COLLISION_2D_DIST		(1)		// LocTris: use 2D distance rather than 3D 




//
// stuff for dbg VarConsole:
//
#ifndef MASTER
	#include		"VarConsole.h"
	#include		"Shadows.h"				// RenderBuffer()...
	bool8			gbPlantMgrActive		= TRUE;
	bool8			gbShowCPlantMgrPolys 	= FALSE;
	bool8			gbDisplayCPlantMgrInfo	= FALSE;
#endif





//
//
//
//
uint16				CPlantMgr::m_scanCode = 0;
CRGBA				CPlantMgr::m_AmbientColor(255, 255, 255, 255);


//
//
//
//
CPlantLocTri*		CPlantMgr::m_UnusedLocTriListHead								= NULL;
CPlantLocTri*		CPlantMgr::m_CloseLocTriListHead[CPLANT_NUM_PLANT_SLOTS]		= {NULL};
CPlantLocTri		CPlantMgr::m_LocTrisTab			[CPLANT_MAX_CACHE_LOC_TRIS_NUM];


CPlantColEntEntry*	CPlantMgr::m_UnusedColEntListHead 	= NULL;
CPlantColEntEntry*	CPlantMgr::m_CloseColEntListHead	= NULL;
CPlantColEntEntry	CPlantMgr::m_ColEntCacheTab[CPLANT_COL_ENTITY_CACHE_SIZE];


//
//
//
//
RwTexture*		CPlantMgr::PC_PlantTextureTab0[CPLANT_SLOT_NUM_TEXTURES]	= {NULL};
RwTexture*		CPlantMgr::PC_PlantTextureTab1[CPLANT_SLOT_NUM_TEXTURES]	= {NULL};
RwTexture*		CPlantMgr::PC_PlantTextureTab2[CPLANT_SLOT_NUM_TEXTURES]	= {NULL};
RwTexture*		CPlantMgr::PC_PlantTextureTab3[CPLANT_SLOT_NUM_TEXTURES]	= {NULL};
RwTexture**		CPlantMgr::PC_PlantSlotTextureTab[CPLANT_NUM_PLANT_SLOTS]	= {NULL};

RpAtomic*		CPlantMgr::PC_PlantModelsTab0[CPLANT_SLOT_NUM_MODELS]		= {NULL};
RpAtomic*		CPlantMgr::PC_PlantModelsTab1[CPLANT_SLOT_NUM_MODELS]		= {NULL};
RpAtomic*		CPlantMgr::PC_PlantModelsTab2[CPLANT_SLOT_NUM_MODELS]		= {NULL};
RpAtomic*		CPlantMgr::PC_PlantModelsTab3[CPLANT_SLOT_NUM_MODELS]		= {NULL};
RpAtomic**		CPlantMgr::PC_PlantModelSlotTab[CPLANT_NUM_PLANT_SLOTS]		= {NULL};





#pragma mark -----------------------
#pragma mark --- CPlantMgr stuff ---
#pragma mark -----------------------


//
//
//
//
CPlantMgr::CPlantMgr()
{
}

//
//
//
//
CPlantMgr::~CPlantMgr()
{
}


//
//
//
//
void CPlantMgr::Shutdown()
{
	// clear whole entity cache:
	{
		CPlantColEntEntry *pEntry = m_CloseColEntListHead;
		while(pEntry)
		{
			CPlantColEntEntry *pNextEntry = pEntry->m_pNextEntry;
			pEntry->ReleaseEntry();
			pEntry = pNextEntry;
		}
	}


	for(int32 i=0; i<CPLANT_SLOT_NUM_MODELS; i++)
	{
#define ATOMIC_DESTROY(A)			{if(A) {RwFrame *f=RpAtomicGetFrame(A); if(f) {RpAtomicSetFrame(A, NULL); RwFrameDestroy(f);} RpAtomicDestroy(A); A=NULL;}}
		ATOMIC_DESTROY(PC_PlantModelsTab0[i]);
		ATOMIC_DESTROY(PC_PlantModelsTab1[i]);
		ATOMIC_DESTROY(PC_PlantModelsTab2[i]);
		ATOMIC_DESTROY(PC_PlantModelsTab3[i]);
#undef ATOMIC_DESTROY
	}
	

	for(int32 i=0; i<CPLANT_NUM_PLANT_SLOTS; i++)
	{
#define TEXTURE_DESTROY(R)			{if(R){RwTextureDestroy(R);R=NULL;}}
		TEXTURE_DESTROY(PC_PlantTextureTab0[i]);
		TEXTURE_DESTROY(PC_PlantTextureTab1[i]);
		TEXTURE_DESTROY(PC_PlantTextureTab2[i]);
		TEXTURE_DESTROY(PC_PlantTextureTab3[i]);
#undef TEXTURE_DESTROY
	}


	UInt32 grassTxdId = CTxdStore::FindTxdSlot("grass_pc");
	if (grassTxdId != -1)
	{
		CTxdStore::RemoveTxdSlot(grassTxdId);
	}

}// end of Shutdown()...





//
// this function helps to avoid ugly visual "rebuilding" of plants while
// camera is teleported into new remote location;
//
// to be called ONCE before teleporting camera / player to new far position:
//
bool8 CPlantMgr::PreUpdateOnceForNewCameraPos(const CVector& newCameraPos)
{

	CPlantMgr::AdvanceCurrentScanCode();
	CGrassRenderer::SetCurrentScanCode(CPlantMgr::GetCurrentScanCode());
	
	CGrassRenderer::SetGlobalCameraPos(newCameraPos);

	CPlantMgr::UpdateAmbientColor();
	
	float bending = CPlantMgr::CalculateWindBending();
	CGrassRenderer::SetGlobalWindBending(bending);


	//
	// empty + update ColEntityCache:
	//
	// force updating all EntityCache:
	_ColEntityCache_Update(newCameraPos);
	
	//
	// update ALL LocTris in CEntities in _ColEntityCache:
	//
	_UpdateLocTris(newCameraPos, CPLANT_ENTRY_TRILOC_PROCESS_ALWAYS);

	return(TRUE);
}


//
//
//
//
bool8 CPlantMgr::Update(const CVector& cameraPos)
{
#ifndef MASTER
	if(!gbPlantMgrActive)	// skip updating, if varconsole switch is off
		return(TRUE);
#endif


	CPlantMgr::AdvanceCurrentScanCode();
	CGrassRenderer::SetCurrentScanCode(CPlantMgr::GetCurrentScanCode());
	
	CGrassRenderer::SetGlobalCameraPos(cameraPos);

	CPlantMgr::UpdateAmbientColor();
	
	
	float bending = CPlantMgr::CalculateWindBending();
	CGrassRenderer::SetGlobalWindBending(bending);


	//
	// update ColEntityCache:
	//
static
uint8 nUpdateEntCache = 0;

	//
	// do not update this stuff too often:
	//
	if( !((++nUpdateEntCache)&(CPLANT_COL_ENTITY_UPDATE_CACHE-1)) )
	{
		// full update of entry cache (FULL update):
		_ColEntityCache_Update(cameraPos);
	}
	else
	{
		// partial update of entry cache (QUICK update):
		_ColEntityCache_Update(cameraPos, TRUE);
	}

	
	//
	// update LocTris in CEntities in _ColEntityCache:
	//
static
uint8 nLocTriSkipCounter=0;

	const int32 iTriProcessSkipMask = (nLocTriSkipCounter++)&(CPLANT_ENTRY_TRILOC_PROCESS_UPDATE-1);
	_UpdateLocTris(cameraPos, iTriProcessSkipMask);


	return(TRUE);
}// end of CPlantMgr::Update()...


//
//
//
//
void CPlantMgr::UpdateAmbientColor()
{
#define AMBIENT_SCALE	(2.5f)		// color scale
#define AMBIENT_ADD		(64)


	CVector vecAmbientColor;
//	if(CMBlur::BlurOn)
//	{
//		vecAmbientColor.x	= CTimeCycle::m_fCurrentAmbientRed_Bl;
//		vecAmbientColor.y	= CTimeCycle::m_fCurrentAmbientGreen_Bl;
//		vecAmbientColor.z	= CTimeCycle::m_fCurrentAmbientBlue_Bl;
//	}
//	else
//	{
		vecAmbientColor.x	= CTimeCycle::GetAmbientRed();
		vecAmbientColor.y	= CTimeCycle::GetAmbientGreen();
		vecAmbientColor.z	= CTimeCycle::GetAmbientBlue();
//	}
	vecAmbientColor *= AMBIENT_SCALE;

	uint16 ambientColorRed	= AMBIENT_ADD + uint16(vecAmbientColor.x * 255.0f);
	uint16 ambientColorGreen= AMBIENT_ADD + uint16(vecAmbientColor.y * 255.0f);
	uint16 ambientColorBlue	= AMBIENT_ADD + uint16(vecAmbientColor.z * 255.0f);
	
	#define CHECK_IN_RANGE_255(VAR)	{if(VAR>255) VAR=255;}
		CHECK_IN_RANGE_255(ambientColorRed);
		CHECK_IN_RANGE_255(ambientColorGreen);
		CHECK_IN_RANGE_255(ambientColorBlue);
	#undef CHECK_IN_RANGE_255

	m_AmbientColor.red	= uint8(ambientColorRed);
	m_AmbientColor.green= uint8(ambientColorGreen);
	m_AmbientColor.blue	= uint8(ambientColorBlue);

#undef AMBIENT_ADD
#undef AMBIENT_SCALE
}// end of CPlantMgr::UpdateAmbientColor()...







extern float WindTabel[];	// defined in Entity.cpp

//
//
// stolen from CEntity::ModifyMatrixForTreeInWind():
//
float CPlantMgr::CalculateWindBending()
{

static
uint32 RandomSeed = CGeneral::GetRandomNumber();


	float bending = 0.0f;

	if(CWeather::Wind < 0.5f)	// Calm weather
	{
		if(CWeather::Wind < 0.2f)
			bending = 0.005f * CMaths::Sin( (CTimer::GetTimeInMilliseconds()&4095) * (6.28f/4096.0f) );
		else 
			//m_mat.xz = 0.02f * CMaths::Sin( ((CTimer::GetTimeInMilliseconds()+((uint32)this))&4095) * (6.28f/4096.0f) );
			bending = 0.008f * CMaths::Sin( (CTimer::GetTimeInMilliseconds()&4095) * (6.28f/4096.0f) );
	}
	else
	{
		uint32	TabelEntry65536	= ((CTimer::GetTimeInMilliseconds()<<3) + RandomSeed)&0x0ffff;	// LC was <<4
		uint32	TabelEntry16	= (TabelEntry65536>>12)&0x0f;
		
		float	Inter		= (TabelEntry65536 & 4095) / 4096.0f;
		float	WindValue	= 1.0f + (1.0f-Inter) * WindTabel[TabelEntry16] + Inter * WindTabel[(TabelEntry16+1)&15];

		bending = 0.015f * WindValue * CWeather::Wind;	// LC was 0.008
	}


	// palms are bend pernamently:
	//atX += -0.07f * CWeather::Wind;

	return(bending);
}



//
//
//
// IsInsideCurrentViewFrustum?
//
static
inline bool IsLocTriVisibleByCamera(CPlantLocTri *pLocTri)
{
	// is sphere containing triangle visible:?
	if(TheCamera.IsSphereVisible(pLocTri->m_Center, pLocTri->m_SphereRadius))
	{
		return(TRUE);
	}

	return(FALSE);
}





//
//
//
//
void CPlantMgr::Render()
{
	//disable plant drawing completely for low fx quality
	if (g_fx.GetFxQuality() == FX_QUALITY_LOW)
	{
		return;
	}

#ifndef MASTER
	if(!gbPlantMgrActive)	// skip rendering, if varconsole switch is off
		return;
#endif //MASTER...


#ifndef MASTER
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DBG_COUNT_DRAWN_PLANTS		(1)
static uint32 nLocTriPlantsDrawn[CPLANT_NUM_PLANT_SLOTS] = {0};
static uint32 nLocTriDrawn[CPLANT_NUM_PLANT_SLOTS] = {0};

	if(gbDisplayCPlantMgrInfo)
	{
		uint32 nPlantsCountAll = 0;
		uint32 nLocTrisCountAll = 0, nLocTrisCountPlanted = 0;
		uint32 nEntitiesCountAll = 0;

		CPlantMgr::DbgCountCachedEntities(&nEntitiesCountAll);


		char txt[128];
		const int16 START_X = 30, START_Y = 24;
		::sprintf(txt, "CPlantMgr::Info:");
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+0);
		
		::sprintf(txt, "Entity: All=%d", nEntitiesCountAll);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+1);

		CPlantMgr::DbgCountLocTrisAndPlants(0, &nLocTrisCountAll, &nPlantsCountAll);
		::sprintf(txt, "LocTris[0]: All=%d (dr=%d) Plants=%d (dr=%d)", nLocTrisCountAll, nLocTriDrawn[0], nPlantsCountAll, nLocTriPlantsDrawn[0]);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+2);

		CPlantMgr::DbgCountLocTrisAndPlants(1, &nLocTrisCountAll, &nPlantsCountAll);
		::sprintf(txt, "LocTris[1]: All=%d (dr=%d) Plants=%d (dr=%d)", nLocTrisCountAll, nLocTriDrawn[1], nPlantsCountAll, nLocTriPlantsDrawn[1]);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+3);

		CPlantMgr::DbgCountLocTrisAndPlants(2, &nLocTrisCountAll, &nPlantsCountAll);
		::sprintf(txt, "LocTris[2]: All=%d (dr=%d) Plants=%d (dr=%d)", nLocTrisCountAll, nLocTriDrawn[2], nPlantsCountAll, nLocTriPlantsDrawn[2]);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+4);

		CPlantMgr::DbgCountLocTrisAndPlants(3, &nLocTrisCountAll, &nPlantsCountAll);
		::sprintf(txt, "LocTris[3]: All=%d (dr=%d) Plants=%d (dr=%d)", nLocTrisCountAll, nLocTriDrawn[3], nPlantsCountAll, nLocTriPlantsDrawn[3]);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+5);

//		CPlantMgr::DbgCountLocTrisAndPlants(4, &nLocTrisCountAll, &nPlantsCountAll);
//		::sprintf(txt, "LocTris[4]: All=%d (Plants=%d)", nLocTrisCountAll, nPlantsCountAll);
//		CDebug::PrintToScreenCoors(txt, START_X, START_Y+6);
	}

	if(gbShowCPlantMgrPolys)
	{
		// hide other collision stuff:
		//gbShowCollisionPolys = TRUE;

		CPlantMgr::DbgRenderCachedEntities();
		CPlantMgr::DbgRenderLocTris();
		return;
	}

	#ifdef DBG_COUNT_DRAWN_PLANTS
		for(int32 i=0; i<CPLANT_NUM_PLANT_SLOTS; i++)
		{
			nLocTriPlantsDrawn[i]	= 0;	// reset statistics
			nLocTriDrawn[i]			= 0;	// reset statistics
		}
	#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //MASTER...





//const float CONST_FOV_SCALE = 70.0f / CDraw::GetFOV();
//const float ScaleX = SCREEN_WIDTH	* CONST_FOV_SCALE;
//const float ScaleY = SCREEN_HEIGHT	* CONST_FOV_SCALE;


	    // Set the appropriate rendermode:
	    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,			(void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,			(void*)TRUE);
	    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,	(void*)TRUE);
	    RwRenderStateSet(rwRENDERSTATESRCBLEND,				(void*)rwBLENDSRCALPHA);
	    RwRenderStateSet(rwRENDERSTATEDESTBLEND,			(void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE,			(void*)TRUE);
#if defined(GTA_PC) || defined(GTA_XBOX)
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF,	(void*)0x00);
		RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION,	(void*)rwALPHATESTFUNCTIONALWAYS);
#endif



	//
	// render all close lists:
	//
	for(int32 slotID=0; slotID<CPLANT_NUM_PLANT_SLOTS; slotID++)
	{
		CPlantLocTri *pLocTri = m_CloseLocTriListHead[slotID];
		
		RwTexture **ppGrassTextureTab = PC_PlantSlotTextureTab[slotID];

		uint32 PlantModelSetID = PPPLANTBUF_MODEL_SET0;
		switch(slotID)
		{
			case(0):	PlantModelSetID = PPPLANTBUF_MODEL_SET0;	break;
			case(1):	PlantModelSetID = PPPLANTBUF_MODEL_SET1;	break;
			case(2):	PlantModelSetID = PPPLANTBUF_MODEL_SET2;	break;
			case(3):	PlantModelSetID	= PPPLANTBUF_MODEL_SET3;	break;
		}

		while(pLocTri)
		{
			if(pLocTri->m_createsPlants)	// draw only if it creates plants
			{
				CPlantSurfProp	*pSurfProp = CPlantSurfPropMgr::GetSurfProperties(pLocTri->m_nSurfaceType);
				ASSERTMSG(pSurfProp, "Invalid SurfaceProps found!");

				for(int32 pdi=0; pdi<CPLANT_SURF_PROP_PLANTDATA_NUM; pdi++)
				{
					// check if pLocTri inside current View Frustum:
					if(IsLocTriVisibleByCamera(pLocTri))
					// is modelID valid for this plant cover definition?
					if(pSurfProp->m_PlantData[pdi].m_nModelID != CPLANT_SURF_PROP_INVALID_MODELID)
					{
						const int32 PLANT_DATA_INDEX = pdi;
						CPlantSurfPropPlantData *surfProp = &pSurfProp->m_PlantData[PLANT_DATA_INDEX];

						PPTriPlant triPlant;
						triPlant.V1				=	pLocTri->m_V1;
						triPlant.V2				=	pLocTri->m_V2;
						triPlant.V3				=	pLocTri->m_V3;
						triPlant.center			=	pLocTri->m_Center;

						triPlant.model_id		=	surfProp->m_nModelID;

						// num of plants to generate (must be multiple of 8):
						triPlant.num_plants		=	8 + (pLocTri->m_nMaxNumPlants[PLANT_DATA_INDEX])&(~0x07);

						triPlant.scale.x		=	surfProp->m_fScaleXY;	// scale in XY
						triPlant.scale.y		=	surfProp->m_fScaleZ;	// scale in Z
						
						//triPlant.uv_offset.x	=	0.0f;	//pSurfProp->m_PlantData[PLANT_DATA_INDEX].m_vecOffsetUV.x;
						//triPlant.uv_offset.y	=	0.0f;	//pSurfProp->m_PlantData[PLANT_DATA_INDEX].m_vecOffsetUV.y;
						triPlant.texture_ptr	=	ppGrassTextureTab[surfProp->m_nTextureID];		// texture ptr to use

			//#define CALC_RGB_INTENSITY(COLOR, INTENSITY) 	((uint16(COLOR)*uint16(INTENSITY)) >> 8)
			//			const uint16 Intensity	= pSurfProp->m_nIntensity;
			//			color.red				= CALC_RGB_INTENSITY(pSurfProp->m_rgbaColor.red, 	Intensity);
			//			color.green				= CALC_RGB_INTENSITY(pSurfProp->m_rgbaColor.green,	Intensity);
			//			color.blue				= CALC_RGB_INTENSITY(pSurfProp->m_rgbaColor.blue,	Intensity);
			//			triPlant.color			= color;
			//#undef CALC_RGB_INTENSITY

						triPlant.color			= surfProp->m_rgbaColor;

						//#define AMBIENT_COLOR_MULTIPLY(COL, AMBIENTCOLOR)		uint8((uint16(COL)*uint16(AMBIENTCOLOR))>>8) 

						const uint8 lighting = uint8(CColTriangle::CalculateLighting(pLocTri->m_nLighting) * 255.0f);
						
						//#define AMBIENT_COLOR_MULTIPLY(COL, AMBIENTCOLOR, LIGHTING)		uint8( (uint32(COL)*uint32(AMBIENTCOLOR)*uint32(LIGHTING)) >> 16 )
						#define AMBIENT_COLOR_MULTIPLY(COL, AMBIENTCOLOR, LIGHTING)			uint8( (uint32(COL)*uint32(LIGHTING)) >> 8 )
							triPlant.color.red		= AMBIENT_COLOR_MULTIPLY(triPlant.color.red,	m_AmbientColor.red,		lighting);
							triPlant.color.green	= AMBIENT_COLOR_MULTIPLY(triPlant.color.green,	m_AmbientColor.green,	lighting);
							triPlant.color.blue		= AMBIENT_COLOR_MULTIPLY(triPlant.color.blue,	m_AmbientColor.blue,	lighting);
						#undef AMBIENT_COLOR_MULTIPLY


						triPlant.intensity		=	surfProp->m_nIntensity;
						triPlant.intensity_var	=	surfProp->m_nIntensityVar;

						triPlant.seed			=	pLocTri->m_Seed[PLANT_DATA_INDEX];
						triPlant.scale_var_xy	=	surfProp->m_fScaleVarXY;			// scale xyz variation	//0.0f;
						triPlant.scale_var_z	=	surfProp->m_fScaleVarZ;

						triPlant.wind_bend_scale=	surfProp->m_fWindBendScale;
						triPlant.wind_bend_var	=	surfProp->m_fWindBendVar;
						
						CGrassRenderer::AddTriPlant(&triPlant, PlantModelSetID);

					#ifdef DBG_COUNT_DRAWN_PLANTS
						nLocTriPlantsDrawn[slotID] += pLocTri->m_nMaxNumPlants[pdi];
						nLocTriDrawn[slotID] += 1;
					#endif
					}
					
				}//for(int32 pdi=0; pdi<CPLANT_SURF_PROP_PLANTDATA_NUM; pdi++)...

			}// if(!pLocTri->m_createsPlants)...
			
			pLocTri = pLocTri->m_pNextTri;
		}//while(pLocTri)...
		
		CGrassRenderer::FlushTriPlantBuffer();

	}//for(int32 slotID=0; slotID<CPLANT_NUM_PLANT_SLOTS; slotID++)...




    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,			(void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE,			(void*)TRUE);
#if defined(GTA_PC) || defined(GTA_XBOX)
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION,	(void*)rwALPHATESTFUNCTIONGREATEREQUAL);
	RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF,	(void*)0x00);
#endif


}// end of CPlantMgr::Render()...




#pragma mark -



static
float _CalcDistanceSqrToEntity(CEntity *pEntity, const CVector& pos)
{
	CColModel *pColModel	= &pEntity->GetColModel();
	CVector vecSphereCenter;
	
	pEntity->TransformIntoWorldSpace(vecSphereCenter, pColModel->GetBoundOffset());

	CVector distVec			= pos - vecSphereCenter;

#if 0
	// faster method:
	float	colDistSqr		= distVec.MagnitudeSqr();
#else
	// slower method:
	const float	colRadius	= pColModel->GetBoundRadius();
	float	colDist			= distVec.Magnitude();
	if(colDist > colRadius)
	{
		colDist -= colRadius;		// are we outside sphere?
	}
	const float colDistSqr = colDist*colDist;
#endif
	
	return(colDistSqr);
}


//
// to do:
// should be replaced with _CalcDistanceSqrToEntity():
//
// (a-b)^2 = a^2 - 2*a*b - b^2
//
static
float _CalcDistanceToEntity(CEntity *pEntity, const CVector& pos)
{
	return CMaths::Sqrt(_CalcDistanceSqrToEntity(pEntity, pos));
}


//
//
// updates cache: looks for close entities to be added to cache,
//					removes ones which are located too far:
//
// this function should be called every Nth frame (N=8, 16, 32, etc);
//
//
// distance checks should be done on distSqrs and not dists!
//
//
bool8 CPlantMgr::_ColEntityCache_Update(const CVector& camPos, bool8 bQuickUpdate)
{

	if(bQuickUpdate)
	{
		//
		// 1. Update entries of the cache first:
		//
		{
			CPlantColEntEntry *pEntry = m_CloseColEntListHead;
			while(pEntry)
			{
				CPlantColEntEntry *pNextEntry = pEntry->m_pNextEntry;
				
				// special case when m_pEntity was already cleared by CEntity reference watchdog:
				if(!pEntry->m_pEntity)
				{
					pEntry->ReleaseEntry();
				}

				pEntry = pNextEntry;
			}
		
		}

	}
	else
	{

		//
		// 1. Update entries of the cache first:
		//
		{
			CPlantColEntEntry *pEntry = m_CloseColEntListHead;
			while(pEntry)
			{
				CPlantColEntEntry *pNextEntry = pEntry->m_pNextEntry;
				
				// special case when m_pEntity was already cleared by CEntity reference watchdog:
				if(!pEntry->m_pEntity)
				{
					pEntry->ReleaseEntry();
				}
				else
				{
					const float distSqr = _CalcDistanceSqrToEntity(pEntry->m_pEntity, camPos);
					if(distSqr > CPLANT_COL_ENTITY_DIST_SQR || (!pEntry->m_pEntity->IsInCurrentArea()))
					{
						pEntry->ReleaseEntry();
					}
				}			
				pEntry = pNextEntry;
			}
		
		}



		//
		//
		// 2. eventually add new entries :
		//
		if(m_UnusedColEntListHead)
		{
		//	const float farDistance		= CPLANT_COL_ENTITY_DIST;
		//	const float farDistanceSqr	= CPLANT_COL_ENTITY_DIST_SQR;

			const CVector vecCentre(camPos);
			const float Left	= vecCentre.x - CPLANT_COL_ENTITY_DIST;
			const float Bottom	= vecCentre.y - CPLANT_COL_ENTITY_DIST;
			const float Right	= vecCentre.x + CPLANT_COL_ENTITY_DIST;
			const float Top		= vecCentre.y + CPLANT_COL_ENTITY_DIST;

			const int32 nLeft	= MAX(	WORLD_WORLDTOSECTORX(Left),		0					);
			const int32 nBottom	= MAX(	WORLD_WORLDTOSECTORY(Bottom),	0					);
			const int32 nRight	= MIN(	WORLD_WORLDTOSECTORX(Right),	WORLD_WIDTHINSECTORS-1);
			const int32 nTop	= MIN(	WORLD_WORLDTOSECTORY(Top),		WORLD_DEPTHINSECTORS-1);


			CWorld::AdvanceCurrentScanCode();
			for(int32 Y=nBottom; Y <= nTop; Y++)
			{
				for(int32 X=nLeft; X <= nRight; X++)
				{
					CSector&	Sector	= CWorld::GetSector(X, Y);
					CPtrList&	List	= Sector.GetOverlapBuildingPtrList();
					CPtrNode	*pNode	= List.GetHeadPtr();
					
					while(pNode)
					{
						CEntity *pEntity = (CEntity*)pNode->GetPtr();
						
						if(pEntity->m_nFlags.bIsProcObject)
						{
							pNode = pNode->GetNextPtr();
							continue;	// don't allow proc objects to be added to PlantMgr entity cache (to avoid recursion)
						}
						
						pNode = pNode->GetNextPtr();
						if(pEntity->GetScanCode() != CWorld::GetCurrentScanCode())
						{
							pEntity->SetScanCode(CWorld::GetCurrentScanCode());

							if(!pEntity->IsInCurrentArea())
								continue;
								
							CBaseModelInfo 		*pBaseModelInfo		= CModelInfo::GetModelInfo(pEntity->GetModelIndex());
							CAtomicModelInfo	*pAtomicModelInfo	= NULL;
							if(pBaseModelInfo->GetModelType()==MI_TYPE_ATOMIC)
							{
								pAtomicModelInfo = (CAtomicModelInfo*)pBaseModelInfo;
							}
		

							// select TERRAIN ONLY entities (buildings have this flag set):
							//if(!pEntity->m_nFlags.bDontCastShadowsOn)
							if(pAtomicModelInfo && pAtomicModelInfo->GetIsPlantFriendly())
							{
								CPlantColEntEntry *pEntry = _ColEntityCache_FindInCache(pEntity);
								if(!pEntry)
								{
									const float distSqr= _CalcDistanceSqrToEntity(pEntity, camPos);
									if(distSqr <= CPLANT_COL_ENTITY_DIST_SQR)
									{
										// add pEntity to our cache:
										_ColEntityCache_Add(pEntity, FALSE);
									
										// no more free slots in cache?
										if(!m_UnusedColEntListHead)
										{
											//ASSERTMSG(FALSE, "No more free slots in ColEntCache!");
											#ifdef DEBUG
												printf("[_ColEntityCache_Update()]: No more free slots in ColEntCache!\n\n");
											#endif
											return(FALSE);
										}
									}
								}					
								
							}
							
						}
						
					}//while(pNode)...

				}//for(int32 X=nLeft; X <= nRight; X++)...

			}//for(int32 Y=nBottom; Y <= nTop; Y++)...

		}// if(m_UnusedColEntListHead)...

	}// bQuickUpdate...

	return(TRUE);
}// end of CPlantMgr::_ColEntityCache_Update()...



//
//
//
//
CPlantColEntEntry* CPlantMgr::_ColEntityCache_FindInCache(CEntity *pEntity)
{
	if(m_CloseColEntListHead)
	{
		CPlantColEntEntry *pEntry = m_CloseColEntListHead;
		while(pEntry)
		{
			if(pEntry->m_pEntity == pEntity)
			{
				return(pEntry);
			}
			
			pEntry = pEntry->m_pNextEntry;
		}
	}
		
	return(NULL);
}

//
//
//
//
CPlantColEntEntry* CPlantMgr::_ColEntityCache_Add(CEntity *pEntity, bool8 bCheckCacheFirst)
{

	CPlantColEntEntry *pEntry;
	
	if(bCheckCacheFirst)
	{
		pEntry = _ColEntityCache_FindInCache(pEntity);
		if(pEntry)
		{
			// seems that this CEntity if already in the cache:
			return(pEntry);
		}
	}
	

	if(m_UnusedColEntListHead)
	{
		pEntry = m_UnusedColEntListHead;
		if(pEntry->AddEntry(pEntity))
		{
			return(pEntry);
		}
	}
	
	
	return(NULL);
}

//
//
//
//
void CPlantMgr::_ColEntityCache_Remove(CEntity *pEntity)
{
	CPlantColEntEntry *pEntry = _ColEntityCache_FindInCache(pEntity);
	if(pEntry)
	{
		pEntry->ReleaseEntry();
	}
}






#pragma mark -
#ifdef USE_COLLISION_SECTIONS
//
//
//
//
bool8 CPlantMgr::_ProcessEntryCollisionDataSections(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask)
{
	CEntity *pEntity		= pEntry->m_pEntity;
	CColModel *pColModel	= &pEntity->GetColModel();
	ASSERTMSG(pEntry->m_nNumTris > 0, "Not valid number of triangles in entry!");
	//ASSERT(pColModel->m_useColModelSections==TRUE);	// colmodel support for col sections obligatory from now

	CCollisionData* pColData = pColModel->GetCollisionData();
	
	if(pColData == NULL)
		return false;

	//
	// sometimes collision data is not streamed in (???):
	//
	//ASSERTMSG(pColModel->m_nNoOfTriangles == pEntry->m_nNumTris, "Different number of ColTris in CEntity & cached entry!");
	if(pColData->m_nNoOfTriangles != pEntry->m_nNumTris)
	{
		//DEBUGLOG("\n [*]_ProcessEntryCollisionData(): entry was skipped because no collision data was found!\n"); 
		return(FALSE);
	}


	const int32 numEntryColTris = pEntry->m_nNumTris;

	//
	// first pass over col tris: 
	// removing those falling outside
	//
	_ProcessEntryCollisionDataSections_RemoveLocTris(pEntry, camPos, iTriProcessSkipMask, 0, numEntryColTris-1);

	if(pColData->m_useColModelSections)
	{
		const int32 numSections = pColData->GetNoOfColModelSections();
		for(int32 i=0; i<numSections; i++)
		{
			const CColModelSection& section = pColData->GetColModelSection(i);
			
			CVector 	bb[2] = {section.bb.m_vecMin, section.bb.m_vecMax};	
			CVector	tBB[2];
//			TransformPoints((RwV3d*)tBB, 2, pEntity->GetMatrix(), (RwV3d*)bb, sizeof(CVector));
			TransformPoints(tBB, 2, pEntity->GetMatrix(), bb);

			CSphere rangeCamSphere;		// spehere with camera inside and radius=TRILOC_FAR_DIST
			CBox	sectionBBox;		// bbox of ColSection
			sectionBBox.Set(tBB[0], tBB[1]);
			sectionBBox.Recalc();
			rangeCamSphere.Set(CPLANT_TRILOC_FAR_DIST, camPos);
			
			if(CCollision::TestSphereBox(rangeCamSphere, sectionBBox))
			{
				_ProcessEntryCollisionDataSections_AddLocTris(pEntry, camPos, iTriProcessSkipMask, 
																section.startIndex, section.endIndex);
			}
		}//for(int32 i=0; i<numSections; i++)...
	}
	else
	{
		_ProcessEntryCollisionDataSections_AddLocTris(pEntry, camPos, iTriProcessSkipMask, 0, numEntryColTris-1);
	}

	
	return(TRUE);
}// end of CPlantMgr::_ProcessEntryCollsionData()...




//
//
//
//
void CPlantMgr::_ProcessEntryCollisionDataSections_RemoveLocTris(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask,
														int32 colStartIndex, int32 colEndIndex)
{

CEntity *pEntity		= pEntry->m_pEntity;
CColModel *pColModel	= &pEntity->GetColModel();


	for(int32 i=colStartIndex; i<=colEndIndex; i++)
	{
		if(pEntry->m_LocTriArray[i])
		{
			CPlantLocTri *pLocTri = pEntry->m_LocTriArray[i];
		
			// check if this triangle creates objects but hasn't created any yet and re process
			if (pLocTri->m_createsObjects && !pLocTri->m_createdObjects)
			{
				ASSERT(pLocTri->m_createsPlants);
			
				if (g_procObjMan.ProcessTriangleAdded(pLocTri))					
				{
					// objects have been created on this triangle ok
					pLocTri->m_createdObjects = true;
				}
			}		
		}

		if(iTriProcessSkipMask != CPLANT_ENTRY_TRILOC_PROCESS_ALWAYS)
		{
			if(iTriProcessSkipMask != (i&(CPLANT_ENTRY_TRILOC_PROCESS_UPDATE-1)))
				continue;
		}	
	
		if(pEntry->m_LocTriArray[i])
		{
			//
			// check if LocTri is in range:
			// if not, remove it:
			//
			CPlantLocTri *pLocTri = pEntry->m_LocTriArray[i];

#if USE_COLLISION_2D_DIST
			const CVector2D colDistV1(camPos - pLocTri->m_V1);
			const CVector2D colDistV2(camPos - pLocTri->m_V2);
			const CVector2D colDistV3(camPos - pLocTri->m_V3);
			const CVector2D colDistV4(camPos - pLocTri->m_Center);
#else			
			const CVector colDistV1(camPos - pLocTri->m_V1);
			const CVector colDistV2(camPos - pLocTri->m_V2);
			const CVector colDistV3(camPos - pLocTri->m_V3);
			const CVector colDistV4(camPos - pLocTri->m_Center);
#endif
			const float colDistSqr1 = colDistV1.MagnitudeSqr();
			const float colDistSqr2 = colDistV2.MagnitudeSqr();
			const float colDistSqr3 = colDistV3.MagnitudeSqr();
			const float colDistSqr4 = colDistV4.MagnitudeSqr();


			// calc middle points distances (distance between camera and tri edges' middle points):
#if USE_COLLISION_2D_DIST
			const CVector2D colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
			const CVector2D colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
			const CVector2D colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#else
			const CVector colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
			const CVector colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
			const CVector colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#endif
			const float colDistSqr5 = colDistMV12.MagnitudeSqr();
			const float colDistSqr6 = colDistMV23.MagnitudeSqr();
			const float colDistSqr7 = colDistMV31.MagnitudeSqr();
				

			if( (colDistSqr1 >= CPLANT_TRILOC_FAR_DIST_SQR)	&&
				(colDistSqr2 >= CPLANT_TRILOC_FAR_DIST_SQR)	&&
				(colDistSqr3 >= CPLANT_TRILOC_FAR_DIST_SQR)	&&
				(colDistSqr4 >= CPLANT_TRILOC_FAR_DIST_SQR) &&
				
				(colDistSqr5 >= CPLANT_TRILOC_FAR_DIST_SQR) &&
				(colDistSqr6 >= CPLANT_TRILOC_FAR_DIST_SQR) &&
				(colDistSqr7 >= CPLANT_TRILOC_FAR_DIST_SQR)		)
			{
				pLocTri->Release();
				pEntry->m_LocTriArray[i] = NULL;
			}
		}

	}// for(int32 i=colStartIndex; i<=colEndIndex; i++)...


}// end of CPlantMgr::_ProcessEntryCollisionDataSections_RemoveLocTris()...



//
//
//
//
void CPlantMgr::_ProcessEntryCollisionDataSections_AddLocTris(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask,
														int32 colStartIndex, int32 colEndIndex)
{

	CEntity *pEntity		= pEntry->m_pEntity;
	CColModel *pColModel	= &pEntity->GetColModel();
	
	CCollisionData* pColData = pColModel->GetCollisionData();
	
	if(pColData == NULL)
		return;
				

		for(int32 i=colStartIndex; i<=colEndIndex; i++)
		{
			if(iTriProcessSkipMask != CPLANT_ENTRY_TRILOC_PROCESS_ALWAYS)
			{
				if(iTriProcessSkipMask != (i&(CPLANT_ENTRY_TRILOC_PROCESS_UPDATE-1)))
					continue;
			}	

			if(!pEntry->m_LocTriArray[i])
			{
				if(m_UnusedLocTriListHead)
				{				
					// generate new LocTri for this ColTriangle (if necessary):
					CVector v[3];
					CVector tv[3];
				
					CColTriangle *pColTri = &pColData->m_pTriangleArray[i];
					pColData->GetTrianglePoint(v[0], pColTri->m_nIndex1);
					pColData->GetTrianglePoint(v[1], pColTri->m_nIndex2);
					pColData->GetTrianglePoint(v[2], pColTri->m_nIndex3);
//					TransformPoints(tv, 3, pEntity->GetMatrix(), v, sizeof(CVector));
					TransformPoints(tv, 3, pEntity->GetMatrix(), v);
					//v1 = pEntity->TransformIntoWorldSpace(v1);
					//v2 = pEntity->TransformIntoWorldSpace(v2);
					//v3 = pEntity->TransformIntoWorldSpace(v3);

					CVector colCenter((tv[0]+tv[1]+tv[2]) / 3.0f);

#ifdef USE_COLLISION_2D_DIST
					CVector2D colDistV1 = camPos - tv[0];
					CVector2D colDistV2 = camPos - tv[1];
					CVector2D colDistV3 = camPos - tv[2];
					CVector2D colDistV4 = camPos - colCenter;
#else
					CVector colDistV1 = camPos - tv[0];
					CVector colDistV2 = camPos - tv[1];
					CVector colDistV3 = camPos - tv[2];
					CVector colDistV4 = camPos - colCenter;
#endif
					const float colDistSqr1 = colDistV1.MagnitudeSqr();
					const float colDistSqr2 = colDistV2.MagnitudeSqr();
					const float colDistSqr3 = colDistV3.MagnitudeSqr();
					const float colDistSqr4 = colDistV4.MagnitudeSqr();
					

					// calc middle points distances (distance between camera and tri edges' middle points):
#ifdef USE_COLLISION_2D_DIST
					const CVector2D colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
					const CVector2D colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
					const CVector2D colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#else
					const CVector colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
					const CVector colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
					const CVector colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#endif
					const float colDistSqr5 = colDistMV12.MagnitudeSqr();
					const float colDistSqr6 = colDistMV23.MagnitudeSqr();
					const float colDistSqr7 = colDistMV31.MagnitudeSqr();

					
					// we're counting how many tri vertices are inside bounds:
					//int32 in = 0;
					//in += (colDistSqr1 < CPLANT_TRILOC_FAR_DIST_SQR);
					//in += (colDistSqr2 < CPLANT_TRILOC_FAR_DIST_SQR);
					//in += (colDistSqr3 < CPLANT_TRILOC_FAR_DIST_SQR);
							
					if( (colDistSqr1 < CPLANT_TRILOC_FAR_DIST_SQR)	||
						(colDistSqr2 < CPLANT_TRILOC_FAR_DIST_SQR)	||
						(colDistSqr3 < CPLANT_TRILOC_FAR_DIST_SQR)	||
						(colDistSqr4 < CPLANT_TRILOC_FAR_DIST_SQR)	||
						
						(colDistSqr5 < CPLANT_TRILOC_FAR_DIST_SQR)	||
						(colDistSqr6 < CPLANT_TRILOC_FAR_DIST_SQR)	||
						(colDistSqr7 < CPLANT_TRILOC_FAR_DIST_SQR)	)
	//				if(in >= 2)
					{
						// check what this surface creates
						bool8 createsPlants  = g_surfaceInfos.CreatesPlants(pColTri->m_nSurfaceType);	//CPlantSurfPropMgr::IsColSurfaceTypePlantFriendly(pColTri->m_nSurfaceType);
						bool8 createsObjects = g_surfaceInfos.CreatesObjects(pColTri->m_nSurfaceType);	//g_procObjMan.DoesSurfaceCreateObjects(pColTri->m_nSurfaceType);
						
						if (createsPlants || createsObjects)
						{
							// this triangle creates plants or objects - add it to the list
							CPlantLocTri *pLocTri = m_UnusedLocTriListHead;

							if(pLocTri->Add(tv[0], tv[1], tv[2], pColTri->m_nSurfaceType, pColTri->m_nLighting, createsPlants, createsObjects))
							{
								// it's added to the list
								ASSERT(pEntry->m_LocTriArray[i]==NULL);
								pEntry->m_LocTriArray[i] = pLocTri;
							
								if (pLocTri->m_createsObjects)
								{
									// this triangle creates objects - process it to see if any get created 
									if (g_procObjMan.ProcessTriangleAdded(pLocTri))					
									{
										// objects have been created on this triangle ok
										pLocTri->m_createdObjects = true;
									}
									else if (!pLocTri->m_createsPlants)
									{
										// no objects have been created and no plants need created - release the triangle
										pLocTri->Release();
										pEntry->m_LocTriArray[i] = NULL;
									}
								}
								else
								{
									ASSERT(pLocTri->m_createsPlants);
								}		
							}
						}
					
					}
				}//if(m_UnusedLocTriListHead)...
			}

		}//for(int32 i=0; i<colModel.m_nNoOfTriangles; i++)...


}//end of CPlantMgr::_ProcessEntryCollisionDataSections_AddLocTris()...



#pragma mark -
#else //USE_COLLISION_SECTIONS
//
//
//
//
bool8 CPlantMgr::_ProcessEntryCollisionData(CPlantColEntEntry *pEntry, const CVector& camPos, int32 iTriProcessSkipMask)
{

	CEntity *pEntity		= pEntry->m_pEntity;
	CColModel *pColModel	= &pEntity->GetColModel();
	ASSERTMSG(pEntry->m_nNumTris > 0, "Not valid number of triangles in entry!");

	//
	// sometimes collision data is not streamed in (???):
	//
	//ASSERTMSG(pColModel->m_nNoOfTriangles == pEntry->m_nNumTris, "Different number of ColTris in CEntity & cached entry!");
	if(pColModel->m_nNoOfTriangles != pEntry->m_nNumTris)
	{
		//DEBUGLOG("\n [*]_ProcessEntryCollisionData(): entry was skipped because no collision data was found!\n"); 
		return(FALSE);
	}


	const int32 count = pEntry->m_nNumTris;//pColModel->m_nNoOfTriangles;
	for(int32 i=0; i<count; i++)
	{
		if(pEntry->m_LocTriArray[i])
		{
			CPlantLocTri *pLocTri = pEntry->m_LocTriArray[i];
		
			// check if this triangle creates objects but hasn't created any yet and re process
			if (pLocTri->m_createsObjects && !pLocTri->m_createdObjects)
			{
				ASSERT(pLocTri->m_createsPlants);
			
				if (g_procObjMan.ProcessTriangleAdded(pLocTri))					
				{
					// objects have been created on this triangle ok
					pLocTri->m_createdObjects = true;
				}
			}		
		}

		if(iTriProcessSkipMask != CPLANT_ENTRY_TRILOC_PROCESS_ALWAYS)
		{
			if(iTriProcessSkipMask != (i&(CPLANT_ENTRY_TRILOC_PROCESS_UPDATE-1)))
				continue;
		}	
	
		if(pEntry->m_LocTriArray[i])
		{
			//
			// check if LocTri is in range:
			// if not, remove it:
			//
			CPlantLocTri *pLocTri = pEntry->m_LocTriArray[i];
			
#ifdef USE_COLLISION_2D_DIST
			const CVector2D colDistV1(camPos - pLocTri->m_V1);
			const CVector2D colDistV2(camPos - pLocTri->m_V2);
			const CVector2D colDistV3(camPos - pLocTri->m_V3);
			const CVector2D colDistV4(camPos - pLocTri->m_Center);
#else
			const CVector colDistV1(camPos - pLocTri->m_V1);
			const CVector colDistV2(camPos - pLocTri->m_V2);
			const CVector colDistV3(camPos - pLocTri->m_V3);
			const CVector colDistV4(camPos - pLocTri->m_Center);
#endif

			const float colDistSqr1 = colDistV1.MagnitudeSqr();
			const float colDistSqr2 = colDistV2.MagnitudeSqr();
			const float colDistSqr3 = colDistV3.MagnitudeSqr();
			const float colDistSqr4 = colDistV4.MagnitudeSqr();


			// calc middle points distances (distance between camera and tri edges' middle points):
#ifdef USE_COLLISION_2D_DIST
			const CVector2D colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
			const CVector2D colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
			const CVector2D colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#else
			const CVector colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
			const CVector colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
			const CVector colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#endif

			const float colDistSqr5 = colDistMV12.MagnitudeSqr();
			const float colDistSqr6 = colDistMV23.MagnitudeSqr();
			const float colDistSqr7 = colDistMV31.MagnitudeSqr();
				

			if( (colDistSqr1 >= CPLANT_TRILOC_FAR_DIST_SQR)	&&
				(colDistSqr2 >= CPLANT_TRILOC_FAR_DIST_SQR)	&&
				(colDistSqr3 >= CPLANT_TRILOC_FAR_DIST_SQR)	&&
				(colDistSqr4 >= CPLANT_TRILOC_FAR_DIST_SQR) &&
				
				(colDistSqr5 >= CPLANT_TRILOC_FAR_DIST_SQR) &&
				(colDistSqr6 >= CPLANT_TRILOC_FAR_DIST_SQR) &&
				(colDistSqr7 >= CPLANT_TRILOC_FAR_DIST_SQR)		)
			{
				pLocTri->Release();
				pEntry->m_LocTriArray[i] = NULL;
			}
		}
		else
		{
			if(m_UnusedLocTriListHead)
			{				
				// generate new LocTri for this ColTriangle (if necessary):
				CVector v[3];
				CVuVector tv[3];
			
				CColTriangle *pColTri = &pColModel->m_pTriangleArray[i];
				pColModel->GetTrianglePoint(v[0], pColTri->m_nIndex1);
				pColModel->GetTrianglePoint(v[1], pColTri->m_nIndex2);
				pColModel->GetTrianglePoint(v[2], pColTri->m_nIndex3);
				TransformPoints(tv, 3, pEntity->GetMatrix(), v, sizeof(CVector));
				//v1 = pEntity->TransformIntoWorldSpace(v1);
				//v2 = pEntity->TransformIntoWorldSpace(v2);
				//v3 = pEntity->TransformIntoWorldSpace(v3);

				CVector colCenter((tv[0]+tv[1]+tv[2]) / 3.0f);

#ifdef USE_COLLISION_2D_DIST
				CVector2D colDistV1 = camPos - tv[0];
				CVector2D colDistV2 = camPos - tv[1];
				CVector2D colDistV3 = camPos - tv[2];
				CVector2D colDistV4 = camPos - colCenter;
#else
				CVector colDistV1 = camPos - tv[0];
				CVector colDistV2 = camPos - tv[1];
				CVector colDistV3 = camPos - tv[2];
				CVector colDistV4 = camPos - colCenter;
#endif
				const float colDistSqr1 = colDistV1.MagnitudeSqr();
				const float colDistSqr2 = colDistV2.MagnitudeSqr();
				const float colDistSqr3 = colDistV3.MagnitudeSqr();
				const float colDistSqr4 = colDistV4.MagnitudeSqr();
				

				// calc middle points distances (distance between camera and tri edges' middle points):
#ifdef USE_COLLISION_2D_DIST
				const CVector2D colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
				const CVector2D colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
				const CVector2D colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#else
				const CVector colDistMV12( (colDistV1+colDistV2) * 0.5f );	// V1-2
				const CVector colDistMV23( (colDistV2+colDistV3) * 0.5f );	// V2-3
				const CVector colDistMV31( (colDistV3+colDistV1) * 0.5f );	// V3-1
#endif
				const float colDistSqr5 = colDistMV12.MagnitudeSqr();
				const float colDistSqr6 = colDistMV23.MagnitudeSqr();
				const float colDistSqr7 = colDistMV31.MagnitudeSqr();

				
				// we're counting how many tri vertices are inside bounds:
				//int32 in = 0;
				//in += (colDistSqr1 < CPLANT_TRILOC_FAR_DIST_SQR);
				//in += (colDistSqr2 < CPLANT_TRILOC_FAR_DIST_SQR);
				//in += (colDistSqr3 < CPLANT_TRILOC_FAR_DIST_SQR);
						
				if( (colDistSqr1 < CPLANT_TRILOC_FAR_DIST_SQR)	||
					(colDistSqr2 < CPLANT_TRILOC_FAR_DIST_SQR)	||
					(colDistSqr3 < CPLANT_TRILOC_FAR_DIST_SQR)	||
					(colDistSqr4 < CPLANT_TRILOC_FAR_DIST_SQR)	||
					
					(colDistSqr5 < CPLANT_TRILOC_FAR_DIST_SQR)	||
					(colDistSqr6 < CPLANT_TRILOC_FAR_DIST_SQR)	||
					(colDistSqr7 < CPLANT_TRILOC_FAR_DIST_SQR)	)
//				if(in >= 2)
				{
					// check what this surface creates
					bool8 createsPlants  = g_surfaceInfos.CreatesPlants(pColTri->m_nSurfaceType);	//CPlantSurfPropMgr::IsColSurfaceTypePlantFriendly(pColTri->m_nSurfaceType);
					bool8 createsObjects = g_surfaceInfos.CreatesObjects(pColTri->m_nSurfaceType);	//g_procObjMan.DoesSurfaceCreateObjects(pColTri->m_nSurfaceType);
					
					if (createsPlants || createsObjects)
					{
						// this triangle creates plants or objects - add it to the list
						CPlantLocTri *pLocTri = m_UnusedLocTriListHead;

						if(pLocTri->Add(tv[0], tv[1], tv[2], pColTri->m_nSurfaceType, pColTri->m_nLighting, createsPlants, createsObjects))
						{
							// it's added to the list
							ASSERT(pEntry->m_LocTriArray[i]==NULL);
							pEntry->m_LocTriArray[i] = pLocTri;
						
							if (pLocTri->m_createsObjects)
							{
								// this triangle creates objects - process it to see if any get created 
								if (g_procObjMan.ProcessTriangleAdded(pLocTri))					
								{
									// objects have been created on this triangle ok
									pLocTri->m_createdObjects = true;
								}
								else if (!pLocTri->m_createsPlants)
								{
									// no objects have been created and no plants need created - release the triangle
									pLocTri->Release();
									pEntry->m_LocTriArray[i] = NULL;
								}
							}
							else
							{
								ASSERT(pLocTri->m_createsPlants);
							}		
						}
					}
				
/*				
					if (createsObjects)
					{

						// process col triangle:
						CPlantLocTri *pLocTri = m_UnusedLocTriListHead;

						if(pLocTri->Add(v1, v2, v3, pColTri->m_nSurfaceType, pColTri->m_nLighting, TRUE))
						{
							if (g_procObjMan.ProcessTriangleAdded(pLocTri, v1, v2, v3, pColTri->m_nSurfaceType, pColTri->m_nLighting))					
							{
								ASSERT(pEntry->m_LocTriArray[i]==NULL);	
								pEntry->m_LocTriArray[i] = pLocTri;
							}
							else
							{
								pLocTri->Release();
							}
						}
					}
					else if(createsObjects)
					{
						// process col triangle:
						CPlantLocTri *pLocTri = m_UnusedLocTriListHead;

						if(pLocTri->Add(v1, v2, v3, pColTri->m_nSurfaceType, pColTri->m_nLighting, FALSE))
						{
							ASSERT(pEntry->m_LocTriArray[i]==NULL);
							pEntry->m_LocTriArray[i] = pLocTri;
						}					
					}
*/					
				}
			}//if(m_UnusedLocTriListHead)...
		}

	}//for(int32 i=0; i<colModel.m_nNoOfTriangles; i++)...


	
	return(TRUE);
}// end of CPlantMgr::_ProcessEntryCollsionData()...
#endif//USE_COLLISION_SECTIONS...


#pragma mark -
//
//
// goes through active LocTris list, tries to reject
// some located far away and generate new LocTris from ColModel's triangles:
//
//
bool8 CPlantMgr::_UpdateLocTris(const CVector& camPos, int32 iTriProcessSkipMask)
{

	CPlantColEntEntry *pEntry = m_CloseColEntListHead;
	while(pEntry)
	{
#ifdef USE_COLLISION_SECTIONS
		_ProcessEntryCollisionDataSections(pEntry, camPos, iTriProcessSkipMask);
#else
		_ProcessEntryCollisionData(pEntry, camPos, iTriProcessSkipMask);
#endif		
		pEntry = pEntry->m_pNextEntry;
	}

	return(TRUE);
}// end of CPlantMgr::_UpdateLocTris()...




#pragma mark -
//
//
// Based on Turk's algorithm:
//
// s = rand[0; 1]
// t = rand[0; 1]
//
static
void _GeneratePointInsideTriangle(CVector *pOut, CPlantLocTri *pLocTri, float s, float t)
{
	const CVector& A = pLocTri->m_V1;
	const CVector& B = pLocTri->m_V2;
	const CVector& C = pLocTri->m_V3;	

	if(s+t > 1.0f)
	{
		s = 1.0f - s;
		t = 1.0f - t;
	}
	
	const float a = 1.0f - s - t;
	const float b = s;
	const float c = t;

	pOut->x = a*A.x + b*B.x + c*C.x;
	pOut->y = a*A.y + b*B.y + c*C.y;
	pOut->z = a*A.z + b*B.z + c*C.z;
}


//
//
//
//
bool8 CPlantMgr::DbgRenderLocTris()
{
#ifndef MASTER

// small value to shift dbg polys in Z, so they are better visible:
#define DBG_LOCTRI_ADDZ			(0.25f)

int32	nCountAll		= 0,
		nCountPlanted	= 0;

	RenderBuffer::ClearRenderBuffer();
	
    // Set the appropriate rendermode  
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,		(void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND,			(void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,		(void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,	NULL);



	for(int32 slotID=0; slotID<CPLANT_NUM_PLANT_SLOTS; slotID++)
	{
		CPlantLocTri *pLocTri = m_CloseLocTriListHead[slotID];
		while(pLocTri)
		{
			nCountAll++;

			const CVector *v1 = &pLocTri->m_V1;
			const CVector *v2 = &pLocTri->m_V2;
			const CVector *v3 = &pLocTri->m_V3;
			
			RwIm3DVertex 	*pVertices;
			RwImVertexIndex *pIndices;

			uint8 Red	= 0;
			uint8 Green = 200;
			uint8 Blue  = 0;

	#define PROCESS_COLOR(COLOR) 	((float(COLOR) * float(count_all&0x7f)) / 128.0f)

			//Red		= 	PROCESS_COLOR(Red);
			//Green	=	PROCESS_COLOR(Green);
			//Blue	=	PROCESS_COLOR(Blue);

	#undef PROCESS_COLOR
			
			RenderBuffer::StartStoring(6, 3, &pIndices, &pVertices);
			{
				RwIm3DVertexSetRGBA(&pVertices[0], Red, Green, Blue, 255);
				RwIm3DVertexSetRGBA(&pVertices[1], Red, Green, Blue, 255);
				RwIm3DVertexSetRGBA(&pVertices[2], Red, Green, Blue, 255);
				RwIm3DVertexSetU(&pVertices[0], 0.0f);
				RwIm3DVertexSetV(&pVertices[0], 0.0f);
				RwIm3DVertexSetU(&pVertices[1], 0.0f);
				RwIm3DVertexSetV(&pVertices[1], 1.0f);
				RwIm3DVertexSetU(&pVertices[2], 1.0f);
				RwIm3DVertexSetV(&pVertices[2], 1.0f);
				RwIm3DVertexSetPos(&pVertices[0], v1->x, v1->y, (v1->z + DBG_LOCTRI_ADDZ));
				RwIm3DVertexSetPos(&pVertices[1], v2->x, v2->y, (v2->z + DBG_LOCTRI_ADDZ));
				RwIm3DVertexSetPos(&pVertices[2], v3->x, v3->y, (v3->z + DBG_LOCTRI_ADDZ));

				pIndices[0] = 0;
				pIndices[1] = 1;
				pIndices[2] = 2;

				pIndices[3] = 0;
				pIndices[4] = 2;
				pIndices[5] = 1;
			}
			RenderBuffer::StopStoring();


			pLocTri = pLocTri->m_pNextTri;
		}//while(pLocTri)...

	}//for(int32 groupID=0; groupID<CPLANT_NUM_GROUP_MODELS; groupID++)...
	

	RenderBuffer::RenderStuffInBuffer();
	RwRenderStateSet(rwRENDERSTATESRCBLEND,			(void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,		(void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,		(void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,		(void*)TRUE);



#undef DBG_LOCTRI_ADDZ

#endif//MASTER...

	return(TRUE);
}// end of CPlantMgr::DbgRenderLocTris()...

//
//
//
//
bool8 CPlantMgr::DbgRenderCachedEntities(uint32 *pCountAll)
{

#ifndef MASTER
uint32 nCountEnt = 0;

	if(m_CloseColEntListHead)
	{
		CPlantColEntEntry *pEntry = m_CloseColEntListHead;
		while(pEntry)
		{
			CEntity *pEntity = pEntry->m_pEntity;

			CCollision::DrawColModel_Coloured(pEntity->GetMatrix(), CModelInfo::GetColModel(pEntity->GetModelIndex()), pEntity->GetModelIndex(), pEntity);
			nCountEnt++;

			pEntry = pEntry->m_pNextEntry;
		}
	}
	
	if(pCountAll)
	{
		*pCountAll = nCountEnt;
	}
#endif//MASTER...

	return(TRUE);
}// end of CPlantMgr::DbgRenderCachedEntities()...




//
//
//
//
bool8 CPlantMgr::DbgCountLocTrisAndPlants(uint32 groupID, uint32 *pCountLocTris, uint32 *pCountPlants)
{
#ifndef MASTER

int32	nCountTris		= 0,
		nCountPlants	= 0;


	CPlantLocTri *pLocTri = m_CloseLocTriListHead[groupID];
	while(pLocTri)
	{
		nCountTris++;

		for(int32 n=0; n<CPLANT_SURF_PROP_PLANTDATA_NUM; n++)
		{
			nCountPlants += pLocTri->m_nMaxNumPlants[n];
		}
		
		pLocTri = pLocTri->m_pNextTri;
	}//while(pLocTri)...


	if(pCountLocTris)
	{
		*pCountLocTris = nCountTris;
	}
	
	if(pCountPlants)
	{
		*pCountPlants = nCountPlants;
	}
#endif//MASTER

	return(TRUE);
}// end of CPlantMgr::DbgCountLocTrix()...

//
//
//
//
bool8 CPlantMgr::DbgCountCachedEntities(uint32 *pCountAll)
{
#ifndef MASTER
uint32 nCountEnt = 0;

	CPlantColEntEntry *pEntry = m_CloseColEntListHead;
	while(pEntry)
	{
		CEntity *pEntity = pEntry->m_pEntity;
		nCountEnt++;

		pEntry = pEntry->m_pNextEntry;
	}
	
	if(pCountAll)
	{
		*pCountAll = nCountEnt;
	}

#endif//MASTER
	return(TRUE);
}// end of CPlantMgr::DbgCountCachedEntities()...







#pragma mark -
#pragma mark --- MoveToList stuff ---



//
//
//
//
CPlantLocTri* CPlantMgr::MoveLocTriToList(CPlantLocTri **ppCurrentList, CPlantLocTri **ppNewList, CPlantLocTri *pTri)
{

	ASSERTMSG(*ppCurrentList, "CPlant::MoveLocTriToList(): m_CurrentList==NULL!");


	// First - Cut out of old list
	if(pTri->m_pPrevTri==NULL)
	{
		// if at head of old list
		*ppCurrentList = pTri->m_pNextTri;
		if(*ppCurrentList)	// might have been only 1 entry in list
			(*ppCurrentList)->m_pPrevTri = NULL;
	}
	else if(pTri->m_pNextTri==NULL)
	{
		// if at tail of old list
		pTri->m_pPrevTri->m_pNextTri = NULL;
	}
	else
	{	// else if in middle of old list
		pTri->m_pNextTri->m_pPrevTri = pTri->m_pPrevTri;
		pTri->m_pPrevTri->m_pNextTri = pTri->m_pNextTri;
	}

	// Second - Insert at start of new list
	pTri->m_pNextTri	= *ppNewList;
	pTri->m_pPrevTri	= NULL;
	*ppNewList = pTri;
	
	if(pTri->m_pNextTri)
		pTri->m_pNextTri->m_pPrevTri = pTri;

	return(pTri);

}// end of CPlantMgr::MoveLocTriToList()...



//
//
//
//
CPlantColEntEntry* CPlantMgr::MoveColEntToList(CPlantColEntEntry **ppCurrentList, CPlantColEntEntry **ppNewList, CPlantColEntEntry *pEntry)
{
	ASSERTMSG(*ppCurrentList, "CPlant::MoveColEndToList(): m_CurrentList==NULL!");


	// First - Cut out of old list
	if(pEntry->m_pPrevEntry==NULL)
	{
		// if at head of old list
		*ppCurrentList = pEntry->m_pNextEntry;
		if(*ppCurrentList)	// might have been only 1 entry in list
			(*ppCurrentList)->m_pPrevEntry = NULL;
	}
	else if(pEntry->m_pNextEntry==NULL)
	{
		// if at tail of old list
		pEntry->m_pPrevEntry->m_pNextEntry = NULL;
	}
	else
	{	// else if in middle of old list
		pEntry->m_pNextEntry->m_pPrevEntry = pEntry->m_pPrevEntry;
		pEntry->m_pPrevEntry->m_pNextEntry = pEntry->m_pNextEntry;
	}

	// Second - Insert at start of new list
	pEntry->m_pNextEntry	= *ppNewList;
	pEntry->m_pPrevEntry	= NULL;
	*ppNewList = pEntry;
	
	if(pEntry->m_pNextEntry)
		pEntry->m_pNextEntry->m_pPrevEntry = pEntry;

	return(pEntry);
}// end of CPlantMgr::MoveColEntToList()...






//
// checks CollissionModel of AtomicModelInfo and
// sets SIMPLE_IS_PLANT_FRIENDLY flag accordingly;
//
//
bool8 CPlantMgr::SetPlantFriendlyFlagInAtomicMI(CAtomicModelInfo *pModelInfo)
{
	CColModel *pColModel = &pModelInfo->GetColModel();

	pModelInfo->SetIsPlantFriendly(FALSE);

	CCollisionData* pColData = pColModel->GetCollisionData();
	
	if(pColData == NULL)
		return false;

	if(pColData->m_nNoOfTriangles==0)
		return(FALSE);

	const int32 count = pColData->m_nNoOfTriangles;
	for(int32 i=0; i<count; i++)
	{
		CColTriangle *pColTri = &pColData->m_pTriangleArray[i];

//		if(CPlantSurfPropMgr::IsColSurfaceTypePlantFriendly(pColTri->m_nSurfaceType))
		if (g_surfaceInfos.CreatesPlants(pColTri->m_nSurfaceType))
		{
			// set this flag if at least one triangle in collsion model has plant-friendly surface type:
			pModelInfo->SetIsPlantFriendly(TRUE);
			return(TRUE);
		}
//		else if (g_procObjMan.DoesSurfaceCreateObjects(pColTri->m_nSurfaceType))
		else if (g_surfaceInfos.CreatesObjects(pColTri->m_nSurfaceType))
		{			
			// set this flag if at least one triangle in collsion model has plant-friendly surface type:
			pModelInfo->SetIsPlantFriendly(TRUE);
			return(TRUE);
		}
	}

	return(FALSE);
}




#pragma mark -
#pragma mark --- CPlantLocTri stuff ---

//
// 1) way no. 1:
// Area = |P1xP2 + P2xP3 + P3xP1| / 2;
//
//
// 2) way no. 2:
// V1 = P2-P1, V2 = P3-P1
// Area = |V1xV2| / 2;
//
//
//
static
float _CalcLocTriArea(CPlantLocTri *pLocTri)
{
//	const CVector p1p2 = CrossProduct(pLocTri->m_V1, pLocTri->m_V2);
//	const CVector p2p3 = CrossProduct(pLocTri->m_V2, pLocTri->m_V3);
//	const CVector p3p1 = CrossProduct(pLocTri->m_V3, pLocTri->m_V1);
//	const CVector p(p1p2 + p2p3 + p3p1);
//	const float area1 = p.Magnitude() * 0.5f;


	const CVector V1(pLocTri->m_V2 - pLocTri->m_V1);
	const CVector V2(pLocTri->m_V3 - pLocTri->m_V1);
	const CVector c = CrossProduct(V1, V2);
	const float area2 = c.Magnitude() * 0.5f;

	return(area2);
}




//
//
//
//
CPlantLocTri* CPlantLocTri::Add(const RwV3d& v1, const RwV3d& v2, const RwV3d& v3, uint8 nSurfaceType, uint8 nLighting, bool8 createsPlants, bool8 createsObjects)
{

	ASSERT(CPlantMgr::m_UnusedLocTriListHead);
//	if(!CPlantMgr::m_UnusedLocTriListHead)
//	{
//		// no more free slots
//		return(NULL);
//	}


	this->m_V1 = v1;
	this->m_V2 = v2;
	this->m_V3 = v3;
	this->m_nSurfaceType	= nSurfaceType;
	this->m_nLighting		= nLighting;
	
//	this->m_IsUsedByProcObj = bIsUsedByProcObj;
	this->m_createsPlants	= createsPlants;
	this->m_createsObjects	= createsObjects;
	this->m_createdObjects	= false;

	const float _1_3 = 1.0f / 3.0f;
	this->m_Center = (CVector)v1 + (CVector)v2 + (CVector)v3;
	this->m_Center *= _1_3;

	CVector vecSphRadius(this->m_Center - this->m_V1);
	this->m_SphereRadius = vecSphRadius.Magnitude();
	this->m_SphereRadius *= 1.75f;	// make sphere radius 75% bigger (for better sphere visibility detection)


	if(m_createsObjects && !m_createsPlants)
	{
		// deal with ONLY procedurally generated objects
		CPlantMgr::MoveLocTriToList(&CPlantMgr::m_UnusedLocTriListHead, &CPlantMgr::m_CloseLocTriListHead[3], this);
		return(this);
	}
	else
	{		
		// deal with procedurally generated plants and objects
		CPlantSurfProp	*pSurfProp	= CPlantSurfPropMgr::GetSurfProperties(nSurfaceType);
		ASSERTMSG(pSurfProp, "Error getting SurfProp for this SurfaceType!");

		// invalid pointer to SurfProps?
		//if(!pSurfProp)
		//	return(NULL);

		const float triArea			= _CalcLocTriArea(this);

		//
		// max number of plants to generate for this TriLoc
		// this is connected with triangle area + surface type:
		//
		const uint16 maxNumPlants0 = uint16(triArea * pSurfProp->m_PlantData[0].m_fDensity);
		const uint16 maxNumPlants1 = uint16(triArea * pSurfProp->m_PlantData[1].m_fDensity);
		const uint16 maxNumPlants2 = uint16(triArea * pSurfProp->m_PlantData[2].m_fDensity);


		//ASSERTMSG(maxNumPlants>0, "maxNumPlants is 0 for this SurfaceType!");

		// only update the list if Density > 0.0f:
		if(((maxNumPlants0 + maxNumPlants1 + maxNumPlants2) > 0))
		{
			this->m_Seed[0]	= CGeneral::GetRandomNumberInRange(0.0f, 1.0f);
			this->m_Seed[1]	= CGeneral::GetRandomNumberInRange(0.0f, 1.0f);
			this->m_Seed[2]	= CGeneral::GetRandomNumberInRange(0.0f, 1.0f);


			this->m_nMaxNumPlants[0] = maxNumPlants0;
			this->m_nMaxNumPlants[1] = maxNumPlants1;
			this->m_nMaxNumPlants[2] = maxNumPlants2;

			const uint32 slotID = pSurfProp->m_nPlantSlotID;
			CPlantMgr::MoveLocTriToList(&CPlantMgr::m_UnusedLocTriListHead, &CPlantMgr::m_CloseLocTriListHead[slotID], this);
			return(this);
		}
		else if (m_createsObjects)
		{
			this->m_createsPlants = false;
			CPlantMgr::MoveLocTriToList(&CPlantMgr::m_UnusedLocTriListHead, &CPlantMgr::m_CloseLocTriListHead[3], this);
			return(this);
		}

		return(NULL);
	}
	
}// end of CPlantLocTri::Add()...


//
//
//
//
void CPlantLocTri::Release()
{
	for(int32 n=0; n<CPLANT_SURF_PROP_PLANTDATA_NUM; n++)
	{
		this->m_nMaxNumPlants[n]	= 0;
	}

	if (m_createdObjects)
	{
		g_procObjMan.ProcessTriangleRemoved(this);
	}

	if (m_createsObjects && !m_createsPlants)
	{
		CPlantMgr::MoveLocTriToList(&CPlantMgr::m_CloseLocTriListHead[3], &CPlantMgr::m_UnusedLocTriListHead, this);

		this->m_nSurfaceType			= 0xFE;		// simple tag to show who released this
	}
	else
	{
		CPlantSurfProp	*pSurfProp	= CPlantSurfPropMgr::GetSurfProperties(this->m_nSurfaceType);
		ASSERTMSG(pSurfProp, "Error getting SurfProp for this SurfaceType!");
		
		const uint32 slotID = pSurfProp->m_nPlantSlotID;
		CPlantMgr::MoveLocTriToList(&CPlantMgr::m_CloseLocTriListHead[slotID], &CPlantMgr::m_UnusedLocTriListHead, this);

		this->m_nSurfaceType			= 0xFF;		// simple tag to show who released this
	}
	
	this->m_createsPlants = false;
	this->m_createsObjects = false;
	this->m_createdObjects = false;
	
//	this->m_nSurfaceType			= 0;
}




#pragma mark -
#pragma mark --- CPlantColEntEntry stuff ---



//
//
//
//
CPlantColEntEntry* CPlantColEntEntry::AddEntry(CEntity *pEntity)
{
	ASSERT(CPlantMgr::m_UnusedColEntListHead);


	this->m_pEntity 		= pEntity;
	REGREF((this->m_pEntity), ((CEntity**)&this->m_pEntity));

	
	CColModel *pColModel	= &pEntity->GetColModel();
	ASSERT(pColModel);
	//ASSERTMSG(pColModel->m_nNoOfTriangles > 0, "Bad number of ColModel triangles or bad type of CEntity!");
	
	CCollisionData* pColData = pColModel->GetCollisionData();
	
	if(pColData == NULL)
		return false;
				
	if(pColData->m_nNoOfTriangles > 0)
	{
		const int32 numTris = pColData->m_nNoOfTriangles;
		this->m_nNumTris 	= numTris;
		//this->m_LocTriArray = new CPlantLocTri* [numTris];
		this->m_LocTriArray	= (CPlantLocTri**)GtaMalloc(numTris * sizeof(CPlantLocTri*));



		for(int32 i=0; i<numTris; i++)
		{
			this->m_LocTriArray[i] = NULL;
		}

		CPlantMgr::MoveColEntToList(&CPlantMgr::m_UnusedColEntListHead, &CPlantMgr::m_CloseColEntListHead, this);
		return(this);
	}
	
	return(NULL);
}


//
//
//
//
void CPlantColEntEntry::ReleaseEntry()
{

	if(this->m_LocTriArray)
	{
		// release LocTris first:
		for(int32 i=0; i<this->m_nNumTris; i++)
		{
			CPlantLocTri *pTri = this->m_LocTriArray[i];
			if(pTri)
			{
				pTri->Release();	
			}
		}
	
		//delete [] this->m_LocTriArray;
		GtaFree(this->m_LocTriArray);
		this->m_LocTriArray = NULL;
		this->m_nNumTris	= 0;
	}


	if(this->m_pEntity)
	{
		TIDYREF((this->m_pEntity), ((CEntity**)&this->m_pEntity));
		this->m_pEntity = NULL;
	}

	CPlantMgr::MoveColEntToList(&CPlantMgr::m_CloseColEntListHead, &CPlantMgr::m_UnusedColEntListHead, this);
}




