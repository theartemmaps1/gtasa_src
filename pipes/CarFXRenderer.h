//
// CarFXRenderer - custom vehicle renderer for d3d9;
//
//	15/10/2004	- JohnW:	- early port to PC; 
//	24/01/2005	- Andrzej:	- initial;
//
//
//
//
//
#ifndef __PCCARFXRENDERER_H__
#define __PCCARFXRENDERER_H__

//
//
//
//
#define CARFX_NUM_DIRT_LEVELS				(16)
#define CARFX_DIRT_TEXTURE_NAME				("vehiclegrunge256")

class CVehicleModelInfo;



//
//
//
//
class CCarFXRenderer
{
public:
	static RwBool		Initialise();
	static RwBool		RegisterPlugins();
	static void			Shutdown();

public:
	static void			PreRenderUpdate();		// to be called once per render frame


////////////////////////////////////////////////////////////////////////////////////////////
//
// CustomCarPipeStuff:
//
////////////////////////////////////////////////////////////////////////////////////////////
public:
	static RpClump*		CustomCarPipeClumpSetup(RpClump *pClump);
	static RpAtomic*	CustomCarPipeAtomicSetup(RpAtomic *pAtomic);

	static RpAtomic*	SetCustomFXAtomicRenderPipelinesVMICB(RpAtomic* pAtomic, void* pData);

public:
	static RwBool		IsCCPCPipelineAttached(RpAtomic *pAtomic); 

public:
	static void			SetFxEnvMapLightMult(float m);
	static float		GetFxEnvMapLightMult();



////////////////////////////////////////////////////////////////////////////////////////////
//
// DirtTexture stuff:
//
////////////////////////////////////////////////////////////////////////////////////////////
public:
	static RwBool		InitialiseDirtTexture();
	static void			RemapDirt(CVehicleModelInfo* pModelInfo, UInt32 dirtLevel);

private:
	static RpMaterial*	MaterialRemapDirtCB(RpMaterial* pMaterial, void* pData);
	static RpAtomic*	AtomicRemapDirtCB(RpAtomic* pAtomic, void* pData);


private:
	static RwTexture*	ms_aDirtTextures[CARFX_NUM_DIRT_LEVELS];
};




#endif //__PCCARFXRENDERER_H__...


