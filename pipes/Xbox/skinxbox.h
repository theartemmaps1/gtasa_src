#ifndef SKINXBOX_H
#define SKINXBOX_H

/*===========================================================================*
 *--- Include files ---------------------------------------------------------*
 *===========================================================================*/
/* Tell D3D we've 16 byte aligned all our matrices
so 4x4 matrix mul is SSE optimized. */
/*
MS put a booboo in XGMath.inl so it doesn't compile under C anymore
They say it will be fixed on the next XDK release. Brilliant... :-P
Probably not a huge speed hit anyway...
*/
/* #define _USE_XGMATH */

#include <xtl.h>
#include <d3d8.h>

#include <rwcore.h>
#include <rpworld.h>

#include "rpskin.h"
#include "shaderdesc.h"

/*===========================================================================*
 *--- Global Types ----------------------------------------------------------*
 *===========================================================================*/
struct SkinGlobalPlatform
{
    /* space for all the pipelines you could ever want */
    RxPipeline *pipelines[rpSKINXBOXPIPELINEMAX-1];
};

/* accessor macro to pipeline pointers to make the syntax less gruesome */
#define _rpSkinPipeline(pipeline)                                       \
    (_rpSkinGlobals.platform.pipelines[(pipeline) - 1])

struct SkinPlatformData
{
    RwInt32 matrixIndexMap[256];
    RwInt32 vertexIndexMap[256];
    RwInt32 numBonesUsed;
    RwInt32 maxWeightsUsed;
    void    *weightsNindicesVB;
    RwInt32 weightsNindicesVBStride;
};

/*===========================================================================*
 *--- Private Global Variables ----------------------------------------------*
 *===========================================================================*/

/*===========================================================================*
 *--- Private Defines -------------------------------------------------------*
 *===========================================================================*/
#define rpSKINMAXNUMBEROFMATRICES 256

/*===========================================================================*
 *--- Private Functions -----------------------------------------------------*
 *===========================================================================*/
extern RwBool
_rpSkinXboxInitShaders();

extern void
_rpSkinXboxDestroyShaders();

extern void
_rpSkinXboxPurgeShaderCache();

extern void
_rpSkinXboxSetShader( RwInt32 formatIndex, const DWORD *shader );

extern void
_rpSkinXboxComputeProjViewWorld
    (
    D3DMATRIX * worldMatrixNoScale,
    D3DMATRIX * viewMatrix,
    D3DMATRIX * projViewWorldMatrix,
    RpAtomic *atomic,
    const RwV4d *screenSpaceScale
    );

extern RwV4d *
_rpSkinXboxEnumerateLights( RwV4d    *shaderConstantPtr,
                            RwInt32  *directLightCountOut,
                            RwInt32  *pointLightCountOut,
                            RpAtomic *atomic );

extern void
_rpSkinXboxMatrixUpdate
    (
    RwV4d *shaderConstants,
    RpAtomic *atomic,
    RpSkin *skin
    );

extern void
_rpSkinXboxSetMaterialColor(RpMaterial *mat, RwBool modulate);

extern RxPipeline *
_rpSkinXboxCreateAtomicPipeline(void);

extern RxPipeline *
_rpSkinXboxCreatePlainPipe();

extern void
_rpSkinXboxDestroyPlainPipe();

extern RxPipeline *
_rpSkinXboxCreateMatFXPipe();

extern void
_rpSkinXboxDestroyMatFXPipe();

extern RxPipeline *
_rpSkinXboxCreateToonPipe();

extern void
_rpSkinXboxDestroyToonPipe();

extern RwBool
_rpSkinXboxInstanceCallback(void *object,
                               RxXboxResEntryHeader *instancedData,
                               RwBool reinstance);

extern RwBool
_rpSkinXboxReInstanceCallback(void *object, RxXboxResEntryHeader *instancedHeader);

/* recomputed everytime we render a new atomic */
extern _MM_ALIGN16 D3DXMATRIX _rpSkinXboxCachedViewMatrix;

/* recomputed everytime we render an atomic */
extern _MM_ALIGN16 D3DMATRIX _rpSkinXboxCachedWorldMatrix;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SKINXBOX_H */
