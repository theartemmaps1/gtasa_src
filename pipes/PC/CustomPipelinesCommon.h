//
// CustomPipelinesCommonPC - all the common stuff for PC pipes, that can be shared between custom pipes;
//
// 14/02/2005	- Andrzej:	- initital;
//
//
//
#ifndef __CUSTOMPIPELINESCOMMONPC_H__
#define __CUSTOMPIPELINESCOMMONPC_H__

#ifdef GTA_XBOX
	#error "This sourcecode is NOT a part of GTA_XBOX build."
#endif

//extern "C"
//{
//extern LPDIRECT3DDEVICE9 _RwD3DDevice;	// global RW object:
//};


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





#if defined(GTA_PC)
	void	CustomD3D9Pipeline_AtomicRenderBlack(RxD3D9ResEntryHeader *resEntryHeader, RxD3D9InstanceData *instancedData);
	void	CustomD3D9Pipeline_AtomicDefaultRender(RxD3D9ResEntryHeader *resEntryHeader, RxD3D9InstanceData *instancedData, RwUInt32 flags, RwTexture *baseTexture);

	RwBool	CustomD3D9Pipeline_RwSetSurfaceProperties(const RwSurfaceProperties *surfaceProps,
                           const RwRGBA *color, RwUInt32 flags, float matSpecular=0.0f, float matSpecPower=0.0f);
	void	CustomD3D9Pipeline_RwSetSurfacePropertiesCacheResync();
#endif //GTA_PC


#endif//__CUSTOMPIPELINESCOMMONPC_H__....
