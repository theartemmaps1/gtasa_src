//
// CustomCarEnvMapPipeline - custom vehicle pipeline for Xbox (D3D8);
//
//	24/01/2005	- Andrzej:	- initial;
//	28/01/2005	- Andrzej:	- EnvMapPipeAtomic plugin support added;
//	28/02/2005	- Andrzej:	- GTA_XBOX has its own sourcefiles for custom pipelines from now on;
//	01/03/2005	- Andrzej:	- added support for GTA_XBOX;
//
//
//
#ifndef __XBOXCUSTOMCARENVMAPPIPELINE_H__
#define __XBOXCUSTOMCARENVMAPPIPELINE_H__

#ifdef GTA_PC
	#error "This sourcecode is NOT a part of GTA_PC build."
#endif

#include "d3d8.h"	// D3DLIGHT8
#include "Pool.h"
#include "rwplugin.h"


//
//
//
//
#define PDSPC_CarFXPipe_AtmID			(rpPDSPC_MAKEPIPEID(rwVENDORID_ROCKSTAR,	rwCUSTPDSPC_CARFX_ATM_PIPE_ID))



//
// bitflag definitions for FxMaterial (usually stored in 3 LSBs of pMaterial->sufaceProps.specular):
//
// valid combinations:
// 000 - no effects     (0)
// 001 - envmap1 only 	(1)
// 010 - envmap2 only 	(2)
// 100 - specular only	(4)
// 101 - spec + envmap1 (5)
// 110 - spec + envmap2 (6)
// 
//
#define CARFX_NOFX_PASS_BIT			(0)			// use Standard Pipeline (no extra passes)
#define CARFX_ENVMAP1_PASS_BIT		(1)			// use EnvMap1 pass
#define CARFX_ENVMAP2_PASS_BIT		(2)			// use EnvMap2 pass
#define CARFX_SPECMAP_PASS_BIT		(4)			// use SpecularMap pass
#define CARFX_ALLMASK_PASS_BIT		(7)





//
//
// private struct that holds envmap data on disk:
//
struct CustomEnvMapPipeMaterialDiskData
{
	float		fEnvScaleX,		fEnvScaleY;
	float		fEnvTransSclX,	fEnvTransSclY;
	float		fShininess;
	RwTexture	*pEnvTexture;
};

//
//
// private struct that holds envmap data in mem disk:
//
struct CustomEnvMapPipeMaterialData
{
	CompressedFloat8	cfEnvScaleX,	cfEnvScaleY;
	CompressedFloat8	cfEnvTransSclX,	cfEnvTransSclY;

	uint8				cfShininess;
	uint8				__pad0;
	uint16				nCurrentRenderFrame;

	RwTexture			*pEnvTexture;
	
public:
	//
	// handy "float" access interface access:
	//
	float				GetfEnvScaleX()					{	return(DecompressTranslationFloat8(this->cfEnvScaleX));		}
	float				GetfEnvScaleY()					{	return(DecompressTranslationFloat8(this->cfEnvScaleY));		}
	void				SetfEnvScaleX(float sx)			{	this->cfEnvScaleX=CompressTranslationFloat8(sx);			}
	void				SetfEnvScaleY(float sy)			{	this->cfEnvScaleY=CompressTranslationFloat8(sy);			}

	float				GetfEnvTransSclX()				{	return(DecompressTranslationFloat8(this->cfEnvTransSclX));	}
	float				GetfEnvTransSclY()				{	return(DecompressTranslationFloat8(this->cfEnvTransSclY));	}
	void				SetfEnvTransSclX(float tx)		{	this->cfEnvTransSclX=CompressTranslationFloat8(tx);			}
	void				SetfEnvTransSclY(float ty)		{	this->cfEnvTransSclY=CompressTranslationFloat8(ty);			}

	float				GetfShininess()					{	return( float(this->cfShininess) * (1.0f/255.0f) );			}
	void				SetfShininess(float s)			{	this->cfShininess = uint8(s * 255.0f);						}
	
};


//
//
// private struct that holds envmap Atomic data in memory:
//
struct CustomEnvMapPipeAtomicData
{
	// used for storing EnvMap2 calculations:
	float				fOffsetU;
	float				fPrevMapPosX, fPrevMapPosY;
};


//
//
// private struct that holds specular data on disk:
//
struct CustomSpecMapPipeMaterialDiskData
{
	float 		fSpecularity;
	char 		cName[24];
};

//
//
// private struct that holds specular data in mem:
//
struct CustomSpecMapPipeMaterialData
{
	float 		fSpecularity;
	RwTexture	*pSpecularTexture;
};




//
// size of internal EnvMapPipeMatData pool:
//
#define CARFX_ENVMAPDATAPOOL_SIZE						(4096)
typedef CPool<struct CustomEnvMapPipeMaterialData> 		CustomEnvMapPipeMaterialDataPool;

#define CARFX_ENVMAPATMDATAPOOL_SIZE					(1024)
typedef CPool<struct CustomEnvMapPipeAtomicData> 		CustomEnvMapPipeAtomicDataPool;

//
// size of internal SpecMapPipeMatData pool:
//
#define CARFX_SPECMAPDATAPOOL_SIZE						(4096)
typedef CPool<struct CustomSpecMapPipeMaterialData> 	CustomSpecMapPipeMaterialDataPool;



//
// useful macro for getting ptr to above structs:
//
#define GETENVMAPPIPEDATAPTRPTR(pMAT)		((CustomEnvMapPipeMaterialData**)(((uint8*)pMAT) + CCustomCarEnvMapPipeline::ms_envMapPluginOffset))
#define GETENVMAPPIPEDATAPTR(pMAT)			(*(GETENVMAPPIPEDATAPTRPTR(pMAT)))

#define GETENVMAPPIPEATMDATAPTRPTR(pATOMIC)	((CustomEnvMapPipeAtomicData**)(((uint8*)pATOMIC) + CCustomCarEnvMapPipeline::ms_envMapAtmPluginOffset))
#define GETENVMAPPIPEATMDATAPTR(pATOMIC)	(*(GETENVMAPPIPEATMDATAPTRPTR(pATOMIC)))

#define GETSPECMAPPIPEDATAPTRPTR(pMAT)		((CustomSpecMapPipeMaterialData**)(((uint8*)pMAT) + CCustomCarEnvMapPipeline::ms_specularMapPluginOffset))
#define GETSPECMAPPIPEDATAPTR(pMAT)			(*(GETSPECMAPPIPEDATAPTRPTR(pMAT)))






//
//
//
//
class CCustomCarEnvMapPipeline
{
public:
	CCustomCarEnvMapPipeline();
	~CCustomCarEnvMapPipeline();

public:
	static RwBool			CreatePipe();
	static void				DestroyPipe();
	static RwBool			CheckDeviceCaps();

public:
	static void				PreRenderUpdate();


	//
	// Plugins:
	//
public:
	static RwBool			RegisterPlugin();

	inline static CustomEnvMapPipeMaterialData**	GetEnvMapPipeDataPtrPtr(RpMaterial *pMaterial);
	inline static CustomEnvMapPipeMaterialData*		GetEnvMapPipeDataPtr(RpMaterial *pMaterial);

	inline static CustomEnvMapPipeAtomicData**		GetEnvMapPipeAtomicDataPtrPtr(RpAtomic *pAtomic);
	inline static CustomEnvMapPipeAtomicData*		GetEnvMapPipeAtomicDataPtr(RpAtomic *pAtomic);


private:
	//
	// CustomEnvMapPipeMaterialData plugin:
	//
	static void* 		pluginEnvMatDestructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
	static void* 		pluginEnvMatConstructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
	static void* 		pluginEnvMatCopyConstructorCB(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);

	static RwStream*	pluginEnvMatStreamWriteCB(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size);
	static RwStream*	pluginEnvMatStreamReadCB(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size);
	static RwInt32		pluginEnvMatStreamGetSizeCB(const void* pData, RwInt32 offset, RwInt32 size);


private:
	//
	// CustomEnvMapPipeAtomicData plugin:
	//
	static void* 		pluginEnvAtmDestructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
	static void* 		pluginEnvAtmConstructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject);
	static void* 		pluginEnvAtmCopyConstructorCB(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);

public:
	static CustomEnvMapPipeAtomicData*		AllocEnvMapPipeAtomicData(RpAtomic *pAtomic);
	static void								SetCustomEnvMapPipeAtomicDataDefaults(CustomEnvMapPipeAtomicData *pData);

 
private:
	//
	// CustomSpecularMapPipeMaterialData plugin:
	//
	static void* 		pluginSpecMatConstructorCB(void *object, int offsetInObject, int sizeInObject);
	static void* 		pluginSpecMatDestructorCB(void *object, int offsetInObject, int sizeInObject);
	static void* 		pluginSpecMatCopyConstructorCB(void *dstObject, const void *srcObject, int offsetInObject, int sizeInObject);

	static RwStream*	pluginSpecMatStreamWriteCB(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size);
	static RwStream* 	pluginSpecMatStreamReadCB(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size);
	static RwInt32 		pluginSpecMatStreamGetSizeCB(const void* pData, RwInt32 offset, RwInt32 size);


public:
	//
	// EnvMap plugin data access: 
	//
	static void			SetFxEnvTexture(RpMaterial *pMaterial, RwTexture *customTexture=NULL);
	static RwTexture*	GetFxEnvTexture(RpMaterial *pMaterial);

	static void			SetFxEnvShininess(RpMaterial *pMaterial, float envShininess);
	static float		GetFxEnvShininess(RpMaterial *pMaterial);

	static void			SetFxEnvScale(RpMaterial *pMaterial, float envScaleX, float envScaleY);
	static float		GetFxEnvScaleX(RpMaterial *pMaterial);
	static float		GetFxEnvScaleY(RpMaterial *pMaterial);

	static void			SetFxEnvTransScl(RpMaterial *pMaterial, float envTransX, float envTransY);
	static float		GetFxEnvTransSclX(RpMaterial *pMaterial);
	static float		GetFxEnvTransSclY(RpMaterial *pMaterial);

public:
	static inline float	GetEnvMapLightingMult();
	static inline void	SetEnvMapLightingMult(float m);

private:
	static inline RwBool					IsTextureUsingEnvMap2FX(RwTexture *pTexture);
	static inline RwBool					IsCustomEnvMapPipeMaterialDataDefault(CustomEnvMapPipeMaterialData *pData);
	static void								SetCustomEnvMapPipeMaterialDataDefaults(CustomEnvMapPipeMaterialData *pData);
public:
	static CustomEnvMapPipeMaterialData*	DuplicateCustomEnvMapPipeMaterialData(CustomEnvMapPipeMaterialData **ppData);


public:
	//
	// SpecMap plugin data access:
	//
	static void								SetFxSpecTexture(RpMaterial *pMaterial, RwTexture *customTexture=NULL);
	static RwTexture*						GetFxSpecTexture(RpMaterial *pMaterial);

	static void								SetFxSpecSpecularity(RpMaterial *pMaterial, float specularity);
	static float							GetFxSpecSpecularity(RpMaterial *pMaterial);




public:
	static RpAtomic*		CustomPipeAtomicSetup(RpAtomic *pAtomic);
private:
	static RpMaterial*		CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__=NULL);


public:



private:
	static RxPipeline*		CreateCustomD3D8ObjPipe();
	static void				CustomPipeRenderCB(RxXboxResEntryHeader *resEntryHeader, void *object, RwUInt8 type, RwUInt32 flags);
//	static RwBool			CustomPipeReinstanceCB(void *object, RwResEntry *resEntry, RxXboxAllInOneInstanceCallBack instanceCallback);
//	static RwBool			CustomPipeInstanceCB(void *object, RxXboxResEntryHeader *resEntryHeader, RwBool reinstance);

private:
	static RxPipeline* 		ObjPipeline;

public:
	static int32								ms_envMapPluginOffset;
	static CustomEnvMapPipeMaterialDataPool*	m_gEnvMapPipeMatDataPool;
	static CustomEnvMapPipeMaterialData			fakeEnvMapPipeMatData;
	static float								m_EnvMapLightingMult;

	static int32								ms_envMapAtmPluginOffset;
	static CustomEnvMapPipeAtomicDataPool*		m_gEnvMapPipeAtmDataPool;

	static int32								ms_specularMapPluginOffset;
	static CustomSpecMapPipeMaterialDataPool*	m_gSpecMapPipeMatDataPool;

private:
	static D3DLIGHT8							m_gSpecularLight;	// global specular light used by the pipe
};





//
//
//
//
inline CustomEnvMapPipeMaterialData** CCustomCarEnvMapPipeline::GetEnvMapPipeDataPtrPtr(RpMaterial *pMaterial)
{
	ASSERT(ms_envMapPluginOffset>0);
	return GETENVMAPPIPEDATAPTRPTR(pMaterial);
}

//
//
//
//
inline CustomEnvMapPipeMaterialData* CCustomCarEnvMapPipeline::GetEnvMapPipeDataPtr(RpMaterial *pMaterial)
{
	ASSERT(ms_envMapPluginOffset>0);
	return GETENVMAPPIPEDATAPTR(pMaterial);
}


//
//
//
//
inline CustomEnvMapPipeAtomicData** CCustomCarEnvMapPipeline::GetEnvMapPipeAtomicDataPtrPtr(RpAtomic *pAtomic)
{
	ASSERT(ms_envMapAtmPluginOffset>0);
	return GETENVMAPPIPEATMDATAPTRPTR(pAtomic);
}

//
//
//
//
inline CustomEnvMapPipeAtomicData* CCustomCarEnvMapPipeline::GetEnvMapPipeAtomicDataPtr(RpAtomic *pAtomic)
{
	ASSERT(ms_envMapAtmPluginOffset>0);
	return GETENVMAPPIPEATMDATAPTR(pAtomic);
}

//
//
// quick way to find out, whether material is gonna use EnvMap2:
//
inline RwBool CCustomCarEnvMapPipeline::IsTextureUsingEnvMap2FX(RwTexture *pTexture)
{
	// check 1st letter of texture:
	return(pTexture->name[0]=='x');
}

//
//
// detect default case (TRUE for most materials);
//
inline RwBool CCustomCarEnvMapPipeline::IsCustomEnvMapPipeMaterialDataDefault(CustomEnvMapPipeMaterialData *pData)
{
	ASSERT(pData);
	if(::memcmp(pData, &fakeEnvMapPipeMatData, sizeof(CustomEnvMapPipeMaterialData)) == 0)
	{
		return(TRUE);
	}
/*
	if(	(pData->fEnvScaleX			== 1.0f) &&
		(pData->fEnvScaleY			== 1.0f) &&
		(pData->fEnvTransSclX		== 1.0f) &&
		(pData->pData->fEnvTransSclY== 1.0f) &&
		(pData->fShininess			== 1.0f) &&
		(pData->pEnvTexture			== NULL) )
	{
		return(TRUE);
	}
*/
	return(FALSE);
}


//
//
//
//
inline void	CCustomCarEnvMapPipeline::SetEnvMapLightingMult(float m)
{
	CCustomCarEnvMapPipeline::m_EnvMapLightingMult = m;
}

inline float CCustomCarEnvMapPipeline::GetEnvMapLightingMult()
{
	return(CCustomCarEnvMapPipeline::m_EnvMapLightingMult);
}

#endif//__XBOXCUSTOMCARENVMAPPIPELINE_H__...

