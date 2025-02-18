//
// CustomBuildingDNPipeline - custom pipeline for buildings with "Day&Night" support for d3d9;
//
//	15/10/2004	- JonhW:	- initial port to PC;
//	04/02/2005	- Andrzej:	- initial;
//
//
//
//
#ifndef __PCCUSTOMBUILDINGDNPIPELINE_H__
#define __PCCUSTOMBUILDINGDNPIPELINE_H__

#ifdef GTA_XBOX
	#error "This sourcecode is NOT a part of GTA_XBOX build."
#endif


//
//
//
//
#define PDSPC_CBuildingDNEnvMap_AtmID		(rpPDSPC_MAKEPIPEID(rwVENDORID_ROCKSTAR,	rwCUSTPDSPC_CBDNENVMAP_ATM_PIPE_ID))



//
// add this data structure to the RpGeometry structure using the plugin
// stuff. Contains ptrs to the two colour tables and current DNvalue
// On the XBox these tables do not exist and the day and night colours for
// each vertex are held in the vertex colour and normal. A vertex shader interpolates
// between these colours using m_fCurrentDNValue
//
struct CDNGeomExtension
{
public:
	RwRGBA*		m_pNightColTable;
	RwRGBA*		m_pDayColTable;
	float		m_fCurrentDNValue;
};

//
//
//
//
class CCustomBuildingDNPipeline
{
public:
	CCustomBuildingDNPipeline();
	~CCustomBuildingDNPipeline();

public:
	static RwBool			CreatePipe();
	static void				DestroyPipe();		

public:
	static RpAtomic*		CustomPipeAtomicSetup(RpAtomic *pAtomic);
	static RwBool			UsesThisPipeline(RpAtomic *pAtomic);
private:
	static RpMaterial*		CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__=NULL);


public:
	static RwBool			ExtraVertColourPluginAttach();

private:
	static void*			pluginExtraVertColourConstructorCB(void* obj, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__);
	static void*			pluginExtraVertColourDestructorCB(void* obj, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__);

	static RwStream* 		pluginExtraVertColourStreamReadCB(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size);
	static RwStream*		pluginExtraVertColourStreamWriteCB(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size);
	static RwInt32 			pluginExtraVertColourStreamGetSizeCB(const void* pData, RwInt32 offset, RwInt32 size);

public:
	static uint8* 			GetExtraVertColourPtr(RpGeometry *pGeom);
	static float* 			GetExtraVertDNValue(RpGeometry *pGeom);

public:	
	inline static void		SetDNBalanceParam(float b);
	inline static float		GetDNBalanceParam();


public:
	static void				Update(RwBool bCameraChange);
	static void				UpdateAtomicDN(RpAtomic* pAtomic, RwBool bForceUpdate);
	static void				UpdateClumpDN(RpClump* pClump, RwBool bForceUpdate);

private:
	static void				UpdateAtomicPrelitLum(RpAtomic* pAtomic, float interpVal);




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


private:
#if defined(GTA_PC)
	static RwBool			CustomPipeInstanceCB(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance);
	static RwBool			CustomPipeReinstanceCB(void *object, RwResEntry *resEntry, RxD3D9AllInOneInstanceCallBack instanceCallback);
	static void				CustomPipeLightingCB(void *object);
	static void				CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
#endif
	static RxPipeline*		CreateCustomObjPipe();

private:
	static RxPipeline* 		ObjPipeline;



private:
	static float			m_fDNBalanceParam;
	static uint32			m_AtmDNWorkingIndex;
	static RwBool			m_bCameraChange;
	static RwBool			m_bDeviceSupportsVS11;

private:
	static int32			ms_extraVertColourPluginOffset;
};



//
//
//
//
void CCustomBuildingDNPipeline::SetDNBalanceParam(float b)
{
	m_fDNBalanceParam = b;
}

//
//
//
//
float CCustomBuildingDNPipeline::GetDNBalanceParam()
{
	return(m_fDNBalanceParam);
}



#endif //__PCCUSTOMBUILDINGDNPIPELINE_H__...

