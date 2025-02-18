//
// CustomPipelinesCommonPC - all the common stuff for PC pipes, that can be shared between custom pipes;
//
// 14/02/2005	- Andrzej:	- initital;
//
//
//
#if defined (GTA_PC) && !defined(OSW)
	#include "windows.h"
	#include "win.h"
	#include <d3d9.h>
#endif //GTA_PC

#include "rwcore.h"
#include "rpworld.h"
#include "PipelinePlugin.h"

#if defined(GTA_PC)
	#include "CustomEnvMapPipelineCSL.h"
	#include "CustomPipelinesCommon.h"
#endif



//
//
//
//
#if defined(GTA_PC) && !defined(OSW)
void CustomD3D9Pipeline_AtomicRenderBlack(RxD3D9ResEntryHeader *resEntryHeader, RxD3D9InstanceData *instancedData)
{
    RwD3D9SetPixelShader(NULL);
    RwD3D9SetVertexShader(instancedData->vertexShader);

    if (resEntryHeader->indexBuffer != NULL)
    {
        RwD3D9DrawIndexedPrimitive((D3DPRIMITIVETYPE)resEntryHeader->primType,
                                   instancedData->baseIndex,
                                   0, instancedData->numVertices,
                                   instancedData->startIndex, instancedData->numPrimitives);
    }
    else
    {
        RwD3D9DrawPrimitive((D3DPRIMITIVETYPE)resEntryHeader->primType,
                            instancedData->baseIndex,
                            instancedData->numPrimitives);
    }
}
#endif // GTA_PC...



//
//
//
//
#if defined(GTA_PC) && !defined(OSW)
void CustomD3D9Pipeline_AtomicDefaultRender(RxD3D9ResEntryHeader *resEntryHeader,
                                     RxD3D9InstanceData *instancedData,
                                     RwUInt32 flags,
                                     RwTexture *baseTexture)
{
//    const MatFXUVAnimData *uvAnim = NULL;

    if (flags & (rxGEOMETRY_TEXTURED|rpGEOMETRYTEXTURED2))
    {
		RwD3D9SetTexture(baseTexture, 0);

//        const rpMatFXMaterialData   *matFXData;
//      matFXData = *MATFXMATERIALGETCONSTDATA(instancedData->material);

//        if (matFXData != NULL &&
//            matFXData->flags == rpMATFXEFFECTUVTRANSFORM)
//        {
//            uvAnim = MATFXD3D9UVANIMGETDATA(instancedData->material);
//            ApplyAnimTextureMatrix(0, uvAnim->baseTransform);
//        }

        RwD3D9SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        RwD3D9SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    else
    {
        RwD3D9SetTexture(NULL, 0);

        RwD3D9SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
        RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }

    // Set the default Pixel shader
    RwD3D9SetPixelShader(NULL);

	// Vertex shader
    RwD3D9SetVertexShader(instancedData->vertexShader);

	// Draw the geometry
    if (resEntryHeader->indexBuffer != NULL)
    {
        // Draw the indexed primitive:
        RwD3D9DrawIndexedPrimitive((D3DPRIMITIVETYPE)resEntryHeader->primType,
                                   instancedData->baseIndex,
                                   0, instancedData->numVertices,
                                   instancedData->startIndex, instancedData->numPrimitives);
    }
    else
    {
        RwD3D9DrawPrimitive((D3DPRIMITIVETYPE)resEntryHeader->primType,
                            instancedData->baseIndex,
                            instancedData->numPrimitives);
    }

//    if (uvAnim != NULL)
//    {
//        RwD3D9SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
//    }
    
}
#endif //GTA_PC...



/**
 * \ingroup rwengined3d9
 * \ref RwD3D9SetSurfaceProperties sets the surface properties to be used in
 * subsequent rendering operations.
 *
 * This function internally converts the RenderWare \ref RwSurfaceProperties
 * to the equivalent D3D9 D3DMATERIAL9 structure, and sets this as the current
 * material for subsequent rendering operations.
 *
 * \param surfaceProps Pointer to a \ref RwSurfaceProperties structure
 * with the surface properties to be set.
 * \param color Pointer to a \ref RwRGBA structure with the color to be set.
 * \param flags Geometry flags.
 *
 * \return TRUE if the surface properties were set, FALSE otherwise.
 *
 */
//
//
// based on RwD3D9SetSurfaceProperties() to achieve similar results
// with more specular-oriented flexibility:
//
#if defined(GTA_PC) && !defined(OSW)
RwBool CustomD3D9Pipeline_RwSetSurfaceProperties(const RwSurfaceProperties *surfaceProps,
                           const RwRGBA *color, RwUInt32 flags, float matSpecular, float matSpecPower)
{

static
D3DMATERIAL9 material = {
    {0.0f, 0.0f, 0.0f, 1.0f},	// Diffuse
    {0.0f, 0.0f, 0.0f, 1.0f},	// Ambient
    {0.0f, 0.0f, 0.0f, 1.0f},	// Specular 
    {0.0f, 0.0f, 0.0f, 1.0f},	// Emissive 
    0.0f						// Power 
};

    ASSERT(surfaceProps != NULL);
    ASSERT(color != NULL);
    ASSERT((flags&rxGEOMETRY_LIGHT) != 0);


	material.Specular.r = material.Specular.g = material.Specular.b = matSpecular;
	material.Power = matSpecPower;

    // Keep only useful flags:
    flags &= (rxGEOMETRY_MODULATE | rxGEOMETRY_PRELIT);


    if( (flags&rxGEOMETRY_MODULATE)					&&
		(*((const RwUInt32 *)color) != 0xffffffff)	)
    {
        RwReal coef = surfaceProps->diffuse * COLORSCALAR;
		material.Diffuse.r = (color->red)	* coef;
        material.Diffuse.g = (color->green) * coef;
        material.Diffuse.b = (color->blue)	* coef;
        material.Diffuse.a = (color->alpha) * COLORSCALAR;

        coef = surfaceProps->ambient * COLORSCALAR;

        if (flags & rxGEOMETRY_PRELIT)
        {
            RwD3D9SetRenderState(D3DRS_AMBIENT, ((D3DCOLOR)((((RwUInt32)color->alpha)<<24)|(((RwUInt32)color->red)<<16)|(((RwUInt32)color->green)<<8)|((RwUInt32)color->blue))));
            RwD3D9SetRenderState(D3DRS_COLORVERTEX, TRUE);
            RwD3D9SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,	D3DMCS_COLOR1);
            RwD3D9SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,	D3DMCS_MATERIAL);

            material.Ambient.r = 0.0f;
            material.Ambient.g = 0.0f;
            material.Ambient.b = 0.0f;

            material.Emissive.r = (color->red)	* AmbientSaturated.red	* coef;
            material.Emissive.g = (color->green)* AmbientSaturated.green* coef;
            material.Emissive.b = (color->blue) * AmbientSaturated.blue	* coef;
        }
        else
        {
            RwD3D9SetRenderState(D3DRS_AMBIENT,		0xffffffff);
            RwD3D9SetRenderState(D3DRS_COLORVERTEX, FALSE);
            RwD3D9SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,	D3DMCS_MATERIAL);
            RwD3D9SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,	D3DMCS_MATERIAL);

            material.Ambient.r = (color->red)	* AmbientSaturated.red	* coef;
            material.Ambient.g = (color->green) * AmbientSaturated.green* coef;
            material.Ambient.b = (color->blue)	* AmbientSaturated.blue * coef;

            material.Emissive.r = 0.0f;
            material.Emissive.g = 0.0f;
            material.Emissive.b = 0.0f;
        }
    }
    else
    {
        material.Diffuse.b = material.Diffuse.g = material.Diffuse.r = surfaceProps->diffuse;
        material.Diffuse.a = 1.0f;

        RwD3D9SetRenderState(D3DRS_AMBIENT,					0xffffffff);
        RwD3D9SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,	D3DMCS_MATERIAL);

        if (flags & rxGEOMETRY_PRELIT)
        {
            // Emmisive color from the vertex colors:
            RwD3D9SetRenderState(D3DRS_COLORVERTEX,				TRUE);
            RwD3D9SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,	D3DMCS_COLOR1);
        }
        else
        {
            // Emmisive color from material, set to black in the submit node:
			RwD3D9SetRenderState(D3DRS_COLORVERTEX,				FALSE);
            RwD3D9SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,	D3DMCS_MATERIAL);

            material.Emissive.r = 0.0f;
            material.Emissive.g = 0.0f;
            material.Emissive.b = 0.0f;
        }

        if (surfaceProps->ambient != 1.0f)
        {
            material.Ambient.r = surfaceProps->ambient * AmbientSaturated.red;
            material.Ambient.g = surfaceProps->ambient * AmbientSaturated.green;
            material.Ambient.b = surfaceProps->ambient * AmbientSaturated.blue;
        }
        else
        {
            material.Ambient.r = AmbientSaturated.red;
            material.Ambient.g = AmbientSaturated.green;
            material.Ambient.b = AmbientSaturated.blue;
        }
    }

	RwD3D9SetMaterial(&material);

    return(TRUE);
}// end of CustomRwD3D9SetSurfaceProperties()...
#endif //GTA_PC...


//
//
// synchronize RW's internal material cache;
// it should be used every time after calling CustomD3D9Pipeline_RwSetSurfaceProperties():
//
#if defined(GTA_PC) && !defined(OSW)
void CustomD3D9Pipeline_RwSetSurfacePropertiesCacheResync()
{
	static RwRGBA defColor = {255,255,255,255};
	static RwSurfaceProperties defSurfaceProps = {1.0f, 0.0f, 0.0f};
	RwD3D9SetSurfaceProperties(&defSurfaceProps, &defColor, rxGEOMETRY_LIGHT);
}
#endif //GTA_PC...




