//
// CustomBuildingRenderer - custom building renderer for d3d9;
//
//	15/10/2004	-	JohnW:		- early port to PC;
//	03/02/2005	-	Andrzej:	- initial;
//
//
//
//
//
#ifndef	__PCCUSTOMBUILDINGRENDERER_H__
#define __PCCUSTOMBUILDINGRENDERER_H__

#ifdef GTA_PC
	#include "CustomBuildingDNPipeline.h"
#endif
#ifdef GTA_XBOX
	#include "CustomBuildingDNPipeline_Xbox.h"
#endif


//
//
//
//
//
class CCustomBuildingRenderer
{
public:
	CCustomBuildingRenderer();
	~CCustomBuildingRenderer();

public:
	static RwBool 			Initialise();
	static RwBool 			PluginAttach();
	static void				Shutdown();
	
	static void				Update();

public:
	static RpAtomic*		AtomicSetup(RpAtomic *pAtomic);
	static RpClump*			ClumpSetup(RpClump *pClump);

	static RpMaterial*		MaterialSetup(RpMaterial *pMaterial);
	static RpMaterial*		MaterialSetupDN(RpMaterial *pMaterial) { return NULL; }
	
	static RwRGBA*			SetupDNVertColourTab(RpGeometry *pGeometry) { return NULL; }

public:
	static RwBool			IsCBPCPipelineAttached(RpAtomic *pAtomic);

public:	
	inline static void		SetDayNightBalanceParam(float b);
	inline static float		GetDayNightBalanceParam();
	
#ifndef OSW
	inline static void		UpdateAtomicDN(RpAtomic* pAtomic, RwBool bForceUpdate);
	inline static void		UpdateClumpDN(RpClump* pClump, RwBool bForceUpdate);
#endif

private:
	static void				UpdateDayNightBalanceParam();
	static void				UpdateAmbientColor();
};


//
//
//
//
void CCustomBuildingRenderer::SetDayNightBalanceParam(float b)
{
	CCustomBuildingDNPipeline::SetDNBalanceParam(b);
}


//
//
//
//
float CCustomBuildingRenderer::GetDayNightBalanceParam()
{
	return(CCustomBuildingDNPipeline::GetDNBalanceParam());
}




//
//
// useful shortcuts for Get/SetDayBalanceParam() on different platforms:
//
inline float cbrGetDayNightBalanceParam()
{
#ifdef GTA_PS2
	return(CVuCustomBuildingRenderer::GetDayNightBalanceParam());
#endif
#if defined(GTA_PC) || defined(GTA_XBOX)
	return(CCustomBuildingRenderer::GetDayNightBalanceParam());
#endif
}

inline void cbrSetDayNightBalanceParam(float b)
{
#ifdef GTA_PS2
	CVuCustomBuildingRenderer::SetDayNightBalanceParam(b);
#endif
#if defined(GTA_PC) || defined(GTA_XBOX)
	CCustomBuildingRenderer::SetDayNightBalanceParam(b);
#endif
}


#ifndef OSW
//
//
//
//
inline
void CCustomBuildingRenderer::UpdateAtomicDN(RpAtomic* pAtomic, RwBool bForceUpdate)
{
	CCustomBuildingDNPipeline::UpdateAtomicDN(pAtomic, bForceUpdate);
}

//
//
//
//
inline
void CCustomBuildingRenderer::UpdateClumpDN(RpClump* pClump, RwBool bForceUpdate)
{
	CCustomBuildingDNPipeline::UpdateClumpDN(pClump, bForceUpdate);
}
#endif


#endif //__PCCUSTOMBUILDINGRENDERER_H__...

