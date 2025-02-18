//
// CustomPipelinesCommonXbox - all the common stuff for Xbox pipes, that can be shared between custom pipes;
//
// 01/03/2005	- Andrzej:	- initital;
//
//
//
#ifndef __CUSTOMPIPELINESCOMMONXBOX_H__
#define __CUSTOMPIPELINESCOMMONXBOX_H__


#ifdef GTA_PC
	#error "This sourcecode is NOT a part of GTA_PC build."
#endif


#if 0
//
// from rwsdk/driver/d3d9/d3d9device.c
//
extern "C"
{
extern RwRGBAReal	AmbientSaturated;
extern RpLight		*AmbientLight;
extern RpLight		*MainLight;
extern RpLight		*PointLight;
};

//
//
//
//
extern "C"
{
extern RwBool		_rwD3D9RenderStateVertexAlphaEnable(RwBool enable);
extern RwBool		_rwD3D9RenderStateIsVertexAlphaEnable(void);
extern RwRGBAReal   AmbientSaturated;
};
#endif //#if 0...

#define FLOATASINT(f)	(*((const RwInt32 *)&(f)))
#define COLORSCALAR		(0.003921568627450980392156862745098f)		 /* 1.0f/ 255.0f */


/////////////////////////////////
//
// Vehicle pipeline:
//
/////////////////////////////////
//
//
// handy inliners for accesing FxPassBits in RpMaterial->surfaceProps.specular:
//
inline 
uint32 CARFX_GET_PASS_CFG(RpMaterial *pMAT)
{
	const uint32 passMask = (*((uint32*)(&pMAT->surfaceProps.specular)));
	return(passMask);

}

inline
void CARFX_SET_PASS_CFG(RpMaterial *pMAT, uint32 passMask)
{
	*((uint32*)(&pMAT->surfaceProps.specular)) = passMask;
}



/////////////////////////////////
//
// Building pipeline:
//
/////////////////////////////////
//
// handy inliners for accesing FxPassBits in RpMaterial->surfaceProps.specular:
//
inline 
uint32 CBMATFX_GET_PASS_CFG(RpMaterial *pMAT)
{
	const uint32 passMask = (*((uint32*)(&pMAT->surfaceProps.specular)));
	return(passMask);

}

inline
void CBMATFX_SET_PASS_CFG(RpMaterial *pMAT, uint32 passMask)
{
	*((uint32*)(&pMAT->surfaceProps.specular)) = passMask;
}




#if 0
#if defined(GTA_PC)
	void	CustomD3D9Pipeline_AtomicRenderBlack(RxD3D9ResEntryHeader *resEntryHeader, RxD3D9InstanceData *instancedData);
	void	CustomD3D9Pipeline_AtomicDefaultRender(RxD3D9ResEntryHeader *resEntryHeader, RxD3D9InstanceData *instancedData, RwUInt32 flags, RwTexture *baseTexture);

	RwBool	CustomD3D9Pipeline_RwSetSurfaceProperties(const RwSurfaceProperties *surfaceProps,
                           const RwRGBA *color, RwUInt32 flags, float matSpecular=0.0f, float matSpecPower=0.0f);
	void	CustomD3D9Pipeline_RwSetSurfacePropertiesCacheResync();
#endif //GTA_PC
#endif //#if 0...

#endif//__CUSTOMPIPELINESCOMMONXBOX_H__....
