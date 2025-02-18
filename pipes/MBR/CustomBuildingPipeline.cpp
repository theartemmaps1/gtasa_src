//
// CustomBuildingPipeline - custom rendering pipeline for buildings on MBR renderer
//

#include "rwcore.h"
#include "rpworld.h"
#include "rwPlugin.h"
#include "MemoryMgr.h"
#include "CompressedFloat.h"
#include "PipelinePlugin.h"
#include "CustomEnvMapPipelineCSL.h"
#include "CustomPipelinesCommon.h"
#include "CustomCarEnvMapPipeline.h"
#include "CustomBuildingPipeline.h"

#include "stubbedgl.h"

RxPipeline*	CCustomBuildingPipeline::ObjPipeline = NULL;

CCustomBuildingPipeline::CCustomBuildingPipeline()
{}

CCustomBuildingPipeline::~CCustomBuildingPipeline()
{}

RwBool CCustomBuildingPipeline::CreatePipe() {
    ObjPipeline = CreateCustomObjPipe();
    if(ObjPipeline == NULL) {
        DEBUGLOG("Cannot create static pipeline.");
        return(FALSE);
    }

    return(TRUE);
}

void CCustomBuildingPipeline::DestroyPipe() {
    if(ObjPipeline) {
        RxPipelineDestroy(ObjPipeline);
        ObjPipeline = NULL;
    }
}	

RpMaterial*	CCustomBuildingPipeline::CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__) {
	ASSERT(pMaterial);
	ASSERT(ObjPipeline);

	CBMATFX_SET_PASS_CFG(pMaterial, 0);	// clear pass bits

	const RpMatFXMaterialFlags effects = RpMatFXMaterialGetEffects(pMaterial);
	if(effects == rpMATFXEFFECTENVMAP) {
		// set EnvMapFX texture from MatFX:
		CCustomBuildingPipeline::SetFxEnvTexture(pMaterial, NULL);
	}


	uint32 passMask = 0;

	if( (CCustomBuildingPipeline::GetFxEnvShininess(pMaterial) != 0.0f)	&&
		(CCustomBuildingPipeline::GetFxEnvTexture(pMaterial)				)																	)
	{
		RwTexture *pEnvMapTex = CCustomBuildingPipeline::GetFxEnvTexture(pMaterial);
		passMask |= CARFX_ENVMAP1_PASS_BIT;
	}

	// set pass configuration to material:
	// use 3 least signifant bits in pMaterial->surfaceProps.specular;
	uint32 matPassMask = CBMATFX_GET_PASS_CFG(pMaterial);
	{
		matPassMask &= ~(CARFX_ALLMASK_PASS_BIT);
		matPassMask |= passMask;
	}
	CBMATFX_SET_PASS_CFG(pMaterial, matPassMask);

	return(pMaterial);
}

RpAtomic*	CCustomBuildingPipeline::CustomPipeAtomicSetup(RpAtomic *pAtomic) {
	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);

	RpGeometryForAllMaterials(pGeometry, CCustomBuildingPipeline::CustomPipeMaterialSetup, NULL);

	RpAtomicSetPipeline(pAtomic, (RxPipeline*)ObjPipeline);
	SetPipelineID(pAtomic, PDSPC_CBuildingEnvMap_AtmID);

	return(pAtomic);
}

void CCustomBuildingPipeline::CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags) {
#if 1
    RxOpenGLResEntryHeader      *resEntryHeader;
    RxOpenGLMeshInstanceData    *instanceData;
    RwUInt16                    numMeshes;

    /* get the instanced data */
    resEntryHeader = (RxOpenGLResEntryHeader *)(repEntry + 1);
    ASSERT( NULL != resEntryHeader );

    instanceData = (RxOpenGLMeshInstanceData *)(resEntryHeader + 1);
    ASSERT( NULL != instanceData );

    numMeshes = resEntryHeader->numMeshes;
    while ( numMeshes-- )
    {
        const RwRGBA    *matColor;

        RwBool          vertexAlphaEnabled;


        ASSERT( NULL != instanceData->material );

        matColor = RpMaterialGetColor( instanceData->material );
        ASSERT( NULL != matColor );

        if ( (FALSE != instanceData->vertexAlpha) ||
             (0xFF != matColor->alpha) )
        {
			// don't draw zero alpha objects
			if (matColor->alpha == 0) {
				instanceData++;
				continue;
			}

            vertexAlphaEnabled = TRUE;
			emu_EnableAlphaModulate(matColor->alpha / 255.0f);
        }
        else
        {
            vertexAlphaEnabled = FALSE;
        }
        _rwOpenGLSetRenderState( rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlphaEnabled );

        if ( FALSE != _rwOpenGLIsLightingEnabled() )
        {
            /* set up the material properties */
            _rwOpenGLLightsSetMaterialProperties( instanceData->material,
                                                  flags );
        }
        else
        {
            _rwOpenGLDisableColorMaterial();
            if ( 0 == (flags & rxGEOMETRY_PRELIT) )
            {
                glColor4fv( (const GLfloat *)&_rwOpenGLOpaqueBlack );
            }
        }    

        /* bind or unbind textures */
        if ( 0 != (flags & (rxGEOMETRY_TEXTURED | rpGEOMETRYTEXTURED2)) )
        {
            RwTexture   *matTexture;


            matTexture = RpMaterialGetTexture( instanceData->material );
            if ( NULL != matTexture )
            {
                RwRaster    *matRaster;
				static int i = 0;

                matRaster = RwTextureGetRaster( matTexture );
                ASSERT( NULL != matRaster );

				if (matRaster->privateFlags & rwRASTERSKIPRENDER) {
					instanceData++;
					continue;
				}

				// skip setting the texture's filter and addressing modes with NoExtras
                _rwOpenGLSetRenderStateNoExtras( rwRENDERSTATETEXTURERASTER, (void *)matRaster );

                _rwOpenGLSetRenderState( rwRENDERSTATETEXTUREADDRESSU, (void *)RwTextureGetAddressingU( matTexture ) );
                _rwOpenGLSetRenderState( rwRENDERSTATETEXTUREADDRESSV, (void *)RwTextureGetAddressingV( matTexture ) );
                _rwOpenGLSetRenderState( rwRENDERSTATETEXTUREFILTER, (void *)RwTextureGetFilterMode( matTexture ) );
            }
            else
            {
                _rwOpenGLSetRenderState( rwRENDERSTATETEXTURERASTER, (void *)NULL );
            }
        }
        else
        {
            _rwOpenGLSetRenderState( rwRENDERSTATETEXTURERASTER, (void *)NULL );
        }

		instanceData->DrawStored();

        /* next RxOpenGLMeshInstanceData pointer */
		instanceData += 1;

		emu_DisableAlphaModulate();
    }
#else
	RxOpenGLResEntryHeader    *resEntryHeader;
	RxOpenGLInstanceData      *instancedData;
	RpMatFXMaterialFlags    effectType;
	RwBool                  bLightingEnabled=FALSE;
	D3DLIGHT9				oldLight1;
	RwBool                  forceBlack=FALSE;
	float					materialSpecular	= 0.0f;
	float					materialSpecPower	= 0.0f;
	const float				envMapLightMult		= 1.0f;	//CCustomCarEnvMapPipeline::m_EnvMapLightingMult * 1.85f;

	RpAtomic *pAtomic = (RpAtomic*)object;
	ASSERT(pAtomic);

	// Set clipping:
    _rwD3D9EnableClippingIfNeeded(object, type);

    // Get lighting state:
    RwD3D9GetRenderState(D3DRS_LIGHTING, &bLightingEnabled);

    if(bLightingEnabled || ((flags & rxGEOMETRY_PRELIT) != 0)) {
        forceBlack = FALSE;
    } else {
        forceBlack = TRUE;
        RwD3D9SetTexture(NULL, 0);
        RwD3D9SetRenderState(D3DRS_TEXTUREFACTOR, 0xff000000);
        RwD3D9SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
        RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    }

    // Get the instanced data:
    resEntryHeader = (RxD3D9ResEntryHeader *)(repEntry + 1);
    instancedData = (RxD3D9InstanceData *)(resEntryHeader + 1);

	// Set Indices:
    if (resEntryHeader->indexBuffer != NULL) {
        RwD3D9SetIndices(resEntryHeader->indexBuffer);
    }

	// Set the stream source
    _rwD3D9SetStreams(resEntryHeader->vertexStream, resEntryHeader->useOffsets);

	// Vertex Declaration
    RwD3D9SetVertexDeclaration(resEntryHeader->vertexDeclaration);

    const int32 numMeshes = resEntryHeader->numMeshes;
    for(int32 i=0; i<numMeshes; i++) {
		RpMaterial *pMaterial = instancedData->material;
		ASSERT(pMaterial);
		CustomEnvMapPipeMaterialData *pEnvMatData = GETENVMAPPIPEDATAPTR(pMaterial);
		ASSERT(pEnvMatData);

		const uint32 matPassMask = CBMATFX_GET_PASS_CFG(pMaterial);
		const bool bUseEnvMap1 = (matPassMask&CARFX_ENVMAP1_PASS_BIT);

		RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
		RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);

		// EnvMap1:
		if(bUseEnvMap1) {
			RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSWRAP);

			static D3DMATRIX envTexMatrix={0};

			envTexMatrix.m[0][0] = pEnvMatData->GetfEnvScaleX();	//*0.25f;
			envTexMatrix.m[1][1] = pEnvMatData->GetfEnvScaleY();	//*0.25f;
			envTexMatrix.m[2][2] = envTexMatrix.m[3][3] = 1.0f;

			RwTexture *pEnvTexture = pEnvMatData->pEnvTexture;
			ASSERT(pEnvTexture);
			RwD3D9SetTexture(pEnvTexture, 1);

			int32 shin = pEnvMatData->GetfShininess() * 254.0f * envMapLightMult;		//32;
			if(shin > 255) shin=255;
			const uint8 nShininess = uint8(shin); //128;

			RwD3D9SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(0xFF,nShininess,nShininess,nShininess));
			RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MULTIPLYADD);// out = arg1 + arg2*arg3;
			RwD3D9SetTextureStageState(1, D3DTSS_COLORARG0, D3DTA_CURRENT );	// result from previous blending stage
			RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);		// texture for this blending stage
			RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG2);
			RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE );
			RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG2,	D3DTA_CURRENT );
			RwD3D9SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,			D3DTSS_TCI_CAMERASPACENORMAL | 1); // env mapping + wrapping mode of TEXTURE1
			RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,	D3DTTFF_DISABLE/*D3DTTFF_COUNT2*//*(D3DTTFF_COUNT3|D3DTTFF_PROJECTED)*/); // projected avoids translation "jumps"
		
			RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);	// no next pass
		}

		if ((instancedData->vertexAlpha) || (0xFF != instancedData->material->color.alpha)) {
            _rwD3D9RenderStateVertexAlphaEnable(TRUE);
        } else {
            _rwD3D9RenderStateVertexAlphaEnable(FALSE);
        }

        if (!forceBlack) {
            if(bLightingEnabled) {
                RwD3D9SetSurfaceProperties(&(instancedData->material->surfaceProps),
                                           &(instancedData->material->color),
                                           flags);
			}
		    
			CustomD3D9Pipeline_AtomicDefaultRender(resEntryHeader, instancedData, flags, instancedData->material->texture);
		} else {
            CustomD3D9Pipeline_AtomicRenderBlack(resEntryHeader, instancedData);
        }

		instancedData++;
    }

	// best to turn off the second pass effect:
	RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,				D3DTOP_DISABLE);
	RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,				D3DTOP_DISABLE);
	RwD3D9SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,			1);
	RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
#endif
}

RxPipeline* CCustomBuildingPipeline::CreateCustomObjPipe() {
	RxPipeline *pipe = RxPipelineCreate();
	RxNodeDefinition *nodedef = RxNodeDefinitionGetOpenGLAtomicAllInOne();

	if(pipe != NULL) {
		RxLockedPipe *lpipe = RxPipelineLock(pipe);
		if ( lpipe != NULL ) {
			if(RxLockedPipeAddFragment(lpipe, NULL, nodedef, NULL)) {
				if(RxLockedPipeUnlock(lpipe)) {
					RxPipelineNode *allinone = RxPipelineFindNodeByName(pipe, nodedef->name, NULL, 0);
					ASSERT(allinone);

					RxOpenGLAllInOneSetRenderCallBack(	allinone,	CustomPipeRenderCB);

					pipe->pluginId	= PDSPC_CBuildingEnvMap_AtmID;
					pipe->pluginData= PDSPC_CBuildingEnvMap_AtmID;
					return(pipe); // success
				}
			}
		}

		RxPipelineDestroy(pipe);
	}

    return(NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// PLUGINS PROPERTIES:
/////////////////////////////////////////////////////////////////////////////////////////////////////

static RwTexture* GetFXMatEnvTexture(RpMaterial *pMaterial) {
	if(!pMaterial)
		return(NULL);
	
	rpMatFXMaterialData *pMaterialData = (rpMatFXMaterialData*)*MATFXMATERIALGETDATA(pMaterial);
	ASSERTMSG(pMaterialData, "pMaterialData invalid!");

	RwTexture *envTexture = pMaterialData->data[rpSECONDPASS].data.envMap.texture;
	return(envTexture);
}

void CCustomBuildingPipeline::SetFxEnvTexture(RpMaterial *pMaterial, RwTexture *fxTexture) {
	ASSERT(pMaterial);
	
	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;
	 
	if(!fxTexture) {
		pData->pEnvTexture = GetFXMatEnvTexture(pMaterial);	
	} else {
		pData->pEnvTexture = fxTexture;
	}

	// force proper wrap modes + filtering for envmap texture:
	RwTexture *pTexture = pData->pEnvTexture;
	if(pTexture) {
		RwTextureSetAddressing(pTexture, rwTEXTUREADDRESSWRAP);
		RwTextureSetFilterMode(pTexture, rwFILTERLINEAR);
 	}
}

RwTexture* CCustomBuildingPipeline::GetFxEnvTexture(RpMaterial *pMaterial) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(NULL);

	return(pData->pEnvTexture);
}

void CCustomBuildingPipeline::SetFxEnvScale(RpMaterial *pMaterial, float envScaleX, float envScaleY) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvScaleX(envScaleX);
	pData->SetfEnvScaleY(envScaleY);
}

float CCustomBuildingPipeline::GetFxEnvScaleX(RpMaterial *pMaterial) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvScaleX());
}

float CCustomBuildingPipeline::GetFxEnvScaleY(RpMaterial *pMaterial) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvScaleY());
}

void CCustomBuildingPipeline::SetFxEnvTransScl(RpMaterial *pMaterial, float envTransX, float envTransY) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvTransSclX(envTransX);
	pData->SetfEnvTransSclY(envTransY);
}

float CCustomBuildingPipeline::GetFxEnvTransSclX(RpMaterial *pMaterial) {
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclX());
}

float CCustomBuildingPipeline::GetFxEnvTransSclY(RpMaterial *pMaterial) {
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclY());
}

void CCustomBuildingPipeline::SetFxEnvShininess(RpMaterial *pMaterial, float envShininess) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;
	
	pData->SetfShininess(envShininess);
}

float CCustomBuildingPipeline::GetFxEnvShininess(RpMaterial *pMaterial) {
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfShininess());
}