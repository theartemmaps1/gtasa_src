// This is a fix for a RW 3.6 vertex shader bug. It is not needed for RW 3.7.
// XBoxSkinAtomicAllInOneNode is overloaded and contains the fix. Without this fix any
// atomics rendered with a custom shader will appear to not be properly clipped when
// being rendered in amongst skinned peds. This is because the shaders used by the skinning
// pipeline were not restoring some constants used by all shaders.

// Written by: John Gurney

extern "C" 
{

#include "skindefs.h"
#include "rpdbgerr.h"

extern RwResEntry *_rxXBInstance(void *object,
                                 void *owner,
                                 RwResEntry **resEntryPointer,
                                 RpMeshHeader *meshHeader,
                                 RwInt32 numVertices,
                                 _rxXboxAllInOneInstanceCallBack instanceCallback,
                                 RwBool allocate);

extern void
_rpSkinXboxTo192Constants();

extern void
_rpSkinXboxTo96Constants();

extern void
_rpSkinXboxPurgeShaderCache();

extern void
_rpSkinXboxComputeProjViewWorld
    (
    D3DMATRIX * worldMatrixNoScale,
    D3DMATRIX * viewMatrix,
    D3DMATRIX * projViewWorldMatrix,
    RpAtomic *atomic,
    const RwV4d *screenSpaceScale
    );

extern _MM_ALIGN16 D3DMATRIX _rpSkinXboxCachedWorldMatrix;
extern _MM_ALIGN16 D3DXMATRIX _rpSkinXboxCachedViewMatrix;

extern void
_rpSkinXboxMatrixUpdate
    (
    RwV4d *shaderConstants,
    RpAtomic *atomic,
    RpSkin *skin
    );

#include "rwskin.h"

extern _MM_ALIGN16 RwV4d _rpSkinXboxPerAtomicConstants[1024]; /* We need enough room for any number of bones and lights */

struct __rxXboxSkinInstanceNodeData
{
    _rxXboxAllInOneInstanceCallBack     instanceCallback;   /* Instance callback */
    _rxXboxAllInOneReinstanceCallBack   reinstanceCallback; /* Instance callback */

    RxXboxSkinAllInOneEnumerateLightsCallBack   enumerateLightsCallback;   /* Lighting callback */

    _rxXboxSkinAllInOneRenderCallBack renderCallback;
};

typedef struct __rxXboxSkinInstanceNodeData _rxXboxSkinInstanceNodeData;


static RwBool
XBoxSkinAtomicAllInOneNode(RxPipelineNodeInstance *self,
                        const RxPipelineNodeParam *params)
{
    RpAtomic                *atomic;
    RpGeometry              *geometry;
    RwResEntry              *resEntry;
    RwUInt32                geomFlags;
    RxXboxResEntryHeader    *resEntryHeader;

    _rxXboxSkinInstanceNodeData *privateData;

    RWFUNCTION(RWSTRING("XBoxSkinAtomicAllInOneNode"));
    RWASSERT(NULL != self);
    RWASSERT(NULL != params);

    atomic = (RpAtomic *)RxPipelineNodeParamGetData(params);
    RWASSERT(NULL != atomic);

    geometry = RpAtomicGetGeometry(atomic);
    RWASSERT(NULL != geometry);
    geomFlags = RpGeometryGetFlags(geometry);

    privateData = (_rxXboxSkinInstanceNodeData *)self->privateData;

    /* if we have native data, don't even look at instancing */
    if (!(rpGEOMETRYNATIVE & RpGeometryGetFlags(geometry)))
    {
        RpMeshHeader            *meshHeader;

        /* If there ain't vertices, we cain't make packets... */
        if (geometry->numVertices <= 0)
        {
            /* Don't execute the rest of the pipeline */
            RWRETURN(TRUE);
        }
            
        meshHeader = geometry->mesh;

        /* Early out if no meshes */
        if (meshHeader->numMeshes <= 0)
        {
            /* If the app wants to use plugin data to make packets, it
             * should use its own instancing function. If we have verts
             * here, we need meshes too in order to build a packet. */
            RWRETURN(TRUE);
        }

        /* If the geometry has more than one morph target the resEntry in the
         * atomic is used else the resEntry in the geometry */
        if (RpGeometryGetNumMorphTargets(geometry) != 1)
        {
            resEntry = atomic->repEntry;
        }
        else
        {
            resEntry = geometry->repEntry;
        }

        /* If the meshes have changed we should re-instance */
        if (resEntry)
        {
            resEntryHeader = (RxXboxResEntryHeader *)(resEntry + 1);
            if (resEntryHeader->serialNumber != meshHeader->serialNum)
            {
                /* Destroy resources to force reinstance */
                RwResourcesFreeResEntry(resEntry);
                resEntry = NULL;
            }
        }

        /* Check to see if a resource entry already exists */
        if (resEntry)
        {
            if (geometry->lockedSinceLastInst ||
                (RpGeometryGetNumMorphTargets(geometry) != 1))
            {
                if (!(rpGEOMETRYNATIVE & RpGeometryGetFlags(geometry)))
                {
                    if (privateData->reinstanceCallback)
                    {
                        privateData->reinstanceCallback(atomic, (RxXboxResEntryHeader *)(resEntry + 1));
                    }
                }

                /* Just make sure the interpolator is no longer dirty */
                atomic->interpolator.flags &= ~rpINTERPOLATORDIRTYINSTANCE;

                /* The geometry is up to date */
                geometry->lockedSinceLastInst = 0;
            }

            /* We have a resEntry so use it */
            RwResourcesUseResEntry(resEntry);
        }
        else
        {
			RwBool		nativeInstanceRequest;
            RwResEntry  **resEntryPointer;
            void        *owner;

            meshHeader = geometry->mesh;

            if (RpGeometryGetNumMorphTargets(geometry) != 1)
            {
                owner = (void *)atomic;
                resEntryPointer = &atomic->repEntry;
            }
            else
            {
                owner = (void *)geometry;
                resEntryPointer = &geometry->repEntry;
            }

			nativeInstanceRequest = (rpGEOMETRYNATIVEINSTANCE & RpGeometryGetFlags(geometry)) != 0;

            /*
             * Create vertex buffers and instance
             */
            resEntry = _rxXBInstance((void *)atomic, owner, resEntryPointer,
                                     meshHeader, RpGeometryGetNumVertices(geometry),
                                     privateData->instanceCallback,
                                     nativeInstanceRequest);
            if (!resEntry)
            {
                RWRETURN(FALSE);
            }

            /* The geometry is up to date */
            geometry->lockedSinceLastInst = 0;

            /* If were a morpth target, morph */
            if (RpGeometryGetNumMorphTargets(geometry) != 1)
            {
                if (privateData->reinstanceCallback)
                {
                    privateData->reinstanceCallback(atomic, (RxXboxResEntryHeader *)(resEntry + 1));
                }
            }
        }
    }
    else
    {
        resEntry = geometry->repEntry;
    }
    
    /*
     * Early out of rendering if we're really preinstancing
     * - might be doing this on a loading screen for example...
     */
	if (rpGEOMETRYNATIVEINSTANCE & RpGeometryGetFlags(geometry))
	{
        RWRETURN(TRUE);
    }

    /* This is to fix a bug introduced on the April 2003 XDK */
    _rwXBLightsEnable(0, FALSE);

    /*
     * Render
     */
    if (privateData->renderCallback)
    {
        /*
        Store tranforms, light data etc, which changes only per atomic here,
        Upload to constant registers in one block.
        Must be 16 byte aligned so we can do SSE matrix math in here.
        */
        RwV4d   *shaderConstantPtr = _rpSkinXboxPerAtomicConstants;
        RwInt32 shaderConstantCount, baseSourceConstant;

        /*
        Buffers that arrange lights contiguously by type so we can convert
        them to constants easily after finding them
        */
        RwInt32 dirLightCount, pointLightCount;

        RpSkin  *skin;
        RwV4d   screenSpaceScale;
        DWORD   oldShader;

        const RwUInt8   *meshBatches;
        const RwUInt8   *boneBatches;

        RxXboxInstanceData    *instancedData;

        resEntryHeader = (RxXboxResEntryHeader *)(resEntry + 1);

        /* cache shader, because we don't use the same API calls to set it */
        oldShader = RwXboxGetCurrentVertexShader();
        _rpSkinXboxPurgeShaderCache();
        _rpSkinXboxTo192Constants();

        /* determine per object constants */

        /* ...screen space scale/offset */
        D3DDevice_GetViewportOffsetAndScale(
                (D3DVECTOR4 *)shaderConstantPtr,
                (D3DVECTOR4 *)&screenSpaceScale);
        shaderConstantPtr += VSCONST_REG_SCREENSPACE_SIZE;

        /* ...proj/view/world matrix */
        _rpSkinXboxComputeProjViewWorld(
            &_rpSkinXboxCachedWorldMatrix,
            &_rpSkinXboxCachedViewMatrix,
            (D3DXMATRIX*)shaderConstantPtr,
            atomic,
            &screenSpaceScale );
        shaderConstantPtr += VSCONST_REG_TRANSFORM_SIZE;

        /* ...determine light count */
        if (privateData->enumerateLightsCallback != NULL)
        {
            shaderConstantPtr = privateData->enumerateLightsCallback(
                                                         shaderConstantPtr,
                                                         &dirLightCount,
                                                         &pointLightCount,
                                                         atomic );
        }

        /* ...(someday) add in other per object constants here, e.g. morph blend factors */

        /*
        ...bone constants are added after everything else because
           we can vary the number of them without changing shaders
        */
        skin = RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic));

        _rpSkinXboxMatrixUpdate( shaderConstantPtr, atomic, skin );

        /* upload per object constants */
        baseSourceConstant = ((RwInt32)shaderConstantPtr - (RwInt32)_rpSkinXboxPerAtomicConstants) / sizeof(RwV4d);
        shaderConstantCount = baseSourceConstant;

        meshBatches = skin->skinSplitData.meshRLECount;
        boneBatches = skin->skinSplitData.meshRLE;
        if (meshBatches == NULL)
        {
            shaderConstantCount += skin->platformData.numBonesUsed * SHADERCONSTS_PER_BONE;

            RWASSERT((shaderConstantCount <= maxShaderConstantSlots) && "too many lights/bones!!!");
        }

        /* upload per atomic shader constants */
        D3DDevice_SetVertexShaderConstantFast(PER_ATOMIC_OFFSET,
                                              _rpSkinXboxPerAtomicConstants,
                                              shaderConstantCount);

        /* attach the positions/normals/colors/uvs */
        D3DDevice_SetStreamSource(0,
            (D3DVertexBuffer *)resEntryHeader->vertexBuffer,
            resEntryHeader->stride);

        /* attach the skin weights/indices */
        D3DDevice_SetStreamSource(1,
            (D3DVertexBuffer *)skin->platformData.weightsNindicesVB,
            skin->platformData.weightsNindicesVBStride);

        /* D3DDevice_SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME ); */

        /* for each mesh determine per mesh constants and render meshes */
        for (instancedData = resEntryHeader->begin; instancedData != resEntryHeader->end; instancedData++)
        {
            if (meshBatches != NULL)
            {
                const RwUInt32 firstBatch = meshBatches[0];
                const RwUInt32 numBatches = meshBatches[1];
                const RwUInt32 minBone = boneBatches[firstBatch * 2];
                RwUInt32 currentBatch;

                meshBatches += 2;

                shaderConstantCount = baseSourceConstant;

                for (currentBatch = 0; currentBatch < numBatches; currentBatch++)
                {
                    RwUInt32 firstBone = boneBatches[(firstBatch + currentBatch) * 2];
                    RwUInt32 numBones = boneBatches[(firstBatch + currentBatch) * 2 + 1];

                    do
                    {
                        const RwUInt32 dstIndex = skin->skinSplitData.matrixRemapIndices[firstBone];
                        const RwUInt32 srcIndex = skin->platformData.vertexIndexMap[firstBone];

                        D3DDevice_SetVertexShaderConstantFast(PER_ATOMIC_OFFSET + baseSourceConstant + (dstIndex * SHADERCONSTS_PER_BONE),
                                                              _rpSkinXboxPerAtomicConstants + baseSourceConstant + (srcIndex * SHADERCONSTS_PER_BONE),
                                                              SHADERCONSTS_PER_BONE);

                        firstBone++;
                    }
                    while(--numBones);

                    shaderConstantCount += numBones * SHADERCONSTS_PER_BONE;
                }
            }

            RWASSERT((shaderConstantCount <= (VSCONST_REG_MAT_COLOR_OFFSET - PER_ATOMIC_OFFSET)) && "too many lights and bones for this material!");

            /* upload material constants */
            _rpSkinXboxSetMaterialColor( instancedData->material,
                                         geomFlags & rxGEOMETRY_MODULATE);

            if (resEntryHeader->vertexAlpha ||
                (0xFF != instancedData->material->color.alpha))
            {
                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
            }
            else
            {
                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
            }

            /* Call the call back */
            privateData->renderCallback((RxXboxResEntryHeader *)(resEntry + 1),
                                        instancedData,
                                        atomic,
                                        geomFlags,
                                        skin->platformData.maxWeightsUsed,
                                        dirLightCount,
                                        pointLightCount);

            #ifdef RWMETRICS
            _rwXbMetricsInc( instancedData->numVertices,
                             instancedData->numIndices,
                             resEntryHeader->primType );
            #endif /* RWMETRICS */
        }

// =====================================================================================
// THIS IS THE FIX:
// =====================================================================================

		/*
		* Restoring screen space constants,
		* it seems D3D does it wrong sometimes...
		*/
		RwV4d viewportOffSet;

		D3DDevice_GetViewportOffsetAndScale((D3DVECTOR4 *)&viewportOffSet,
		(D3DVECTOR4 *)&screenSpaceScale);

		D3DDevice_SetVertexShaderConstantFast(-38, &screenSpaceScale, 1);
		D3DDevice_SetVertexShaderConstantFast(-37, &viewportOffSet, 1);

// =====================================================================================
// =====================================================================================

        /* allow fixed function pipeline to work again */
        _rpSkinXboxTo96Constants();

        /* restore old shader */
        D3DDevice_SetVertexShader( oldShader );

        /* Clear stream 1 */
        D3DDevice_SetStreamSource(1, NULL, 0);
	}

    RWRETURN(TRUE);
}

extern RwBool
XBoxSkinAtomicAllInOnePipelineInit(RxPipelineNode *node);

#define NUMCLUSTERSOFINTEREST   0
#define NUMOUTPUTS              0

/**
 * \ingroup rpskinxbox
 * \ref RxNodeDefinitionGetXboxSkinAtomicAllInOne returns a
 * pointer to the \ref RxNodeDefinition associated with 
 * the Xbox \ref RpSkin pipeline.
 *
 * \return The \ref RxNodeDefinition pointer.
 *
 * \see RxXboxSkinAllInOneSetEnumerateLightsCallBack
 * \see RxXboxSkinAllInOneGetEnumerateLightsCallBack
 */
RxNodeDefinition *
RxNodeDefinitionGetXboxSkinAtomicAllInOne(void)
{
    static RxNodeDefinition nodeXboxSkinAtomicAllInOneCSL = { /* */
        "XboxSkinAtomicAllInOne.csl",               /* Name */
        {                                           /* Nodemethods */
            XBoxSkinAtomicAllInOneNode,             /* +-- nodebody */
            NULL,                                   /* +-- nodeinit */
            NULL,                                   /* +-- nodeterm */
            XBoxSkinAtomicAllInOnePipelineInit,     /* +-- pipelinenodeinit */
            NULL,                                   /* +-- pipelineNodeTerm */
            NULL,                                   /* +-- pipelineNodeConfig */
            NULL,                                   /* +-- configMsgHandler */
        },
        {                                           /* Io */
            NUMCLUSTERSOFINTEREST,                  /* +-- NumClustersOfInterest */
            NULL,                                   /* +-- ClustersOfInterest */
            NULL,                                   /* +-- InputRequirements */
            NUMOUTPUTS,                             /* +-- NumOutputs */
            NULL                                    /* +-- Outputs */
        },
        (RwUInt32)sizeof(_rxXboxSkinInstanceNodeData),  /* PipelineNodePrivateDataSize */
        rxNODEDEFCONST,                                      /* editable */
        0                                           /* inPipes */
    };

    RWAPIFUNCTION(RWSTRING("RxNodeDefinitionGetXboxSkinAtomicAllInOne"));

    RWRETURN(&nodeXboxSkinAtomicAllInOneCSL);
}


}

