//
// CustomBuildingPipeline - custom rendering pipeline for buildings on d3d9;
//
//	15/10/2004	- JonhW:	- initial port to PC;
//	04/02/2005	- Andrzej:	- initial;	
//	28/02/2005	- Andrzej:	- GTA_XBOX has its own sourcecode for pipeline;
//
//
//
//
#ifndef __XBOXCUSTOMBUILDINGPIPELINE_H__
#define __XBOXCUSTOMBUILDINGPIPELINE_H__

#ifdef GTA_PC
	#error "This sourcecode is NOT a part of GTA_PC build."
#endif


//
//
//
//
#define PDSPC_CBuildingEnvMap_AtmID			(rpPDSPC_MAKEPIPEID(rwVENDORID_ROCKSTAR,	rwCUSTPDSPC_CBENVMAP_ATM_PIPE_ID))


//
//
//
//
class CCustomBuildingPipeline
{
public:
	CCustomBuildingPipeline();
	~CCustomBuildingPipeline();

public:
	static RwBool			CreatePipe();
	static void				DestroyPipe();		

public:
	static RpAtomic*		CustomPipeAtomicSetup(RpAtomic *pAtomic);
private:
	static RpMaterial*		CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__=NULL);


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
	static void				CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
	static RwBool			CustomPipeInstanceCB(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance);
	static RwBool			CustomPipeReinstanceCB(void *object, RwResEntry *resEntry, RxD3D9AllInOneInstanceCallBack instanceCallback);
#endif
	static RxPipeline*		CreateCustomObjPipe();

private:
	static RxPipeline* 		ObjPipeline;
};




#endif //__XBOXCUSTOMBUILDINGPIPELINE_H__....


