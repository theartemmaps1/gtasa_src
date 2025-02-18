//
// CustomBuildingDNPipeline - custom pipeline for buildings with "Day&Night" support for MBR
//

#ifndef __CUSTOMBUILDINGDNPIPELINE_H__
#define __CUSTOMBUILDINGDNPIPELINE_H__

#define PDSPC_CBuildingDNEnvMap_AtmID		(rpPDSPC_MAKEPIPEID(rwVENDORID_ROCKSTAR,	rwCUSTPDSPC_CBDNENVMAP_ATM_PIPE_ID))

// add this data structure to the RpGeometry structure using the plugin
// stuff. Contains ptrs to the two colour tables and current DNvalue
// On the XBox these tables do not exist and the day and night colours for
// each vertex are held in the vertex colour and normal. A vertex shader interpolates
// between these colours using m_fCurrentDNValue
struct CDNGeomExtension {
	RwRGBA*		m_pNightColTable;
	RwRGBA*		m_pDayColTable;
	float		m_fCurrentDNValue;
};

class CCustomBuildingDNPipeline {
	static RpMaterial*		CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__=NULL);

	static void*			pluginExtraVertColourConstructorCB(void* obj, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__);
	static void*			pluginExtraVertColourDestructorCB(void* obj, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__);

	static RwStream* 		pluginExtraVertColourStreamReadCB(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size);
	static RwStream*		pluginExtraVertColourStreamWriteCB(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size);
	static RwInt32 			pluginExtraVertColourStreamGetSizeCB(const void* pData, RwInt32 offset, RwInt32 size);
	
	static RwBool			CustomPipeInstanceCB(void *object, RxOpenGLMeshInstanceData *instanceData, const RwBool instanceDLandVA, const RwBool reinstance);
	static void				CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);

	static RxPipeline*		CreateCustomObjPipe();
	static RxPipeline* 		ObjPipeline;
	static float			m_fDNBalanceParam;
	static uint32			m_AtmDNWorkingIndex;
	static RwBool			m_bCameraChange;
	static RwBool			m_bDeviceSupportsVS11;
	static int32			ms_extraVertColourPluginOffset;

public:
	CCustomBuildingDNPipeline();
	~CCustomBuildingDNPipeline();

	static RwBool			CreatePipe();
	static void				DestroyPipe();	

	static RpAtomic*		CustomPipeAtomicSetup(RpAtomic *pAtomic);
	static RwBool			UsesThisPipeline(RpAtomic *pAtomic);

	static RwBool			ExtraVertColourPluginAttach();

	static uint8* 			GetExtraVertColourPtr(RpGeometry *pGeom);

	inline static void		SetDNBalanceParam(float b);
	inline static float		GetDNBalanceParam();

	// EnvMap plugin data access: 
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
};

void CCustomBuildingDNPipeline::SetDNBalanceParam(float b) {
	m_fDNBalanceParam = b;
}

float CCustomBuildingDNPipeline::GetDNBalanceParam() {
	return(m_fDNBalanceParam);
}

#endif