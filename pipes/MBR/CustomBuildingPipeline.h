//
// CustomBuildingPipeline - custom rendering pipeline for buildings on MBR
//

#ifndef __PCCUSTOMBUILDINGPIPELINE_H__
#define __PCCUSTOMBUILDINGPIPELINE_H__

#define PDSPC_CBuildingEnvMap_AtmID			(rpPDSPC_MAKEPIPEID(rwVENDORID_ROCKSTAR,	rwCUSTPDSPC_CBENVMAP_ATM_PIPE_ID))

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
	static void				CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags);
	static RxPipeline*		CreateCustomObjPipe();

private:
	static RxPipeline* 		ObjPipeline;
};

#endif 