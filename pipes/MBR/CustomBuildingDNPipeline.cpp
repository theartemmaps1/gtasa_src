//
// CustomBuildingDNPipeline - custom pipeline for buildings with "Day&Night" support for MBR
//

#include "rwcore.h"
#include "rpworld.h"
#include "camera.h"
#include "rwPlugin.h"
#include "MemoryMgr.h"
#include "PipelinePlugin.h"

#include "CustomEnvMapPipelineCSL.h"
#include "CustomPipelinesCommon.h"
#include "CustomCarEnvMapPipeline.h"
#include "CustomBuildingDNPipeline.h"

#include "stubbedgl.h"

int32 CCustomBuildingDNPipeline::ms_extraVertColourPluginOffset = -1;

#define EXTRAVERTCOLOUR_SIZE 			(sizeof(CDNGeomExtension))
#define EXTRAVERTCOLOURFROMGEOM(GEOM) 	((char*)(((RwUInt8*)GEOM) + ms_extraVertColourPluginOffset))
#define GETPLUGINBASE(GEOM) 			((char*)(((RwUInt8*)GEOM) + ms_extraVertColourPluginOffset))
#define EXTRAVERT_DAYTABLE(GEOM)		(((CDNGeomExtension*)GETPLUGINBASE(GEOM))->m_pDayColTable)
#define EXTRAVERT_NIGHTTABLE(GEOM)		(((CDNGeomExtension*)GETPLUGINBASE(GEOM))->m_pNightColTable)
#define	DN_NUM_FRAMES_FOR_PROCESS		(16)		// process pre lighting over this number of frames

float		CCustomBuildingDNPipeline::m_fDNBalanceParam	= 1.0f;
uint32		CCustomBuildingDNPipeline::m_AtmDNWorkingIndex	= 0;			// used to process atomics across several frames
RwBool		CCustomBuildingDNPipeline::m_bCameraChange		= FALSE;		// set if require DN update to process all atomics this frame
RxPipeline*	CCustomBuildingDNPipeline::ObjPipeline			= NULL;
RwBool		CCustomBuildingDNPipeline::m_bDeviceSupportsVS11= FALSE;

CCustomBuildingDNPipeline::CCustomBuildingDNPipeline() {}
CCustomBuildingDNPipeline::~CCustomBuildingDNPipeline() {}

RwBool	CCustomBuildingDNPipeline::CreatePipe() {
	ObjPipeline = CreateCustomObjPipe();
    if(ObjPipeline  == NULL) {
        DEBUGLOG("Cannot create static pipeline.");
        return FALSE;
    }

    return(TRUE);
}

void CCustomBuildingDNPipeline::DestroyPipe() {
    if(ObjPipeline) {
        RxPipelineDestroy(ObjPipeline);
        ObjPipeline = NULL;
    }
}	

RpMaterial*	CCustomBuildingDNPipeline::CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__) {
	ASSERT(pMaterial);
	ASSERT(ObjPipeline);

	CBMATFX_SET_PASS_CFG(pMaterial, 0);	// clear pass bits
	const RpMatFXMaterialFlags effects = RpMatFXMaterialGetEffects(pMaterial);
	if(effects == rpMATFXEFFECTENVMAP) {
		// set EnvMapFX texture from MatFX:
		CCustomBuildingDNPipeline::SetFxEnvTexture(pMaterial, NULL);
	}

	uint32 passMask = 0;

	if( (CCustomBuildingDNPipeline::GetFxEnvShininess(pMaterial) != 0.0f)	&&
		(CCustomBuildingDNPipeline::GetFxEnvTexture(pMaterial)				)) {
		RwTexture *pEnvMapTex = CCustomBuildingDNPipeline::GetFxEnvTexture(pMaterial);
		passMask |= CARFX_ENVMAP1_PASS_BIT;
	}

	// set pass configuration to material:
	// use 3 least signifant bits in pMaterial->surfaceProps.specular;
	uint32 matPassMask = CBMATFX_GET_PASS_CFG(pMaterial);
	matPassMask &= ~(CARFX_ALLMASK_PASS_BIT);
	matPassMask |= passMask;
	CBMATFX_SET_PASS_CFG(pMaterial, matPassMask);

	return(pMaterial);
}

RpAtomic* CCustomBuildingDNPipeline::CustomPipeAtomicSetup(RpAtomic *pAtomic) {
	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);

	RpGeometryForAllMaterials(pGeometry, CCustomBuildingDNPipeline::CustomPipeMaterialSetup, NULL);

	RpAtomicSetPipeline(pAtomic, (RxPipeline*)ObjPipeline);
	SetPipelineID(pAtomic, PDSPC_CBuildingDNEnvMap_AtmID);

	return(pAtomic);
}

RwBool CCustomBuildingDNPipeline::UsesThisPipeline(RpAtomic *pAtomic) {
	RxPipeline* pPipeline=NULL;
	
	RpAtomicGetPipeline(pAtomic, &pPipeline);
	if(ObjPipeline == pPipeline) {
		return(TRUE);
	}
	
	return(FALSE);
}

RwBool CCustomBuildingDNPipeline::CustomPipeInstanceCB(void *object, RxOpenGLMeshInstanceData *instanceData, const RwBool instanceDLandVA, const RwBool reinstance) {
    ASSERT( NULL != object );
    ASSERT( NULL != instanceData );

    /* get the atomic */
    const RpAtomic* atomic = (const RpAtomic *)object;
    
    /* get the atomic's geometry */
    const RpGeometry* geometry = RpAtomicGetGeometry(atomic);
    ASSERT( NULL != geometry );

    /* get the number of texture coordinates in this geometry */
    RwInt32 numTexCoords = RpGeometryGetNumTexCoordSets(geometry );

    /* geometry's flags */
    RpGeometryFlag geomFlags = (RpGeometryFlag) RpGeometryGetFlags(geometry);

    /* get the vertex descriptor */
    instanceData->vertexDesc = (RwUInt32)geomFlags;

    /* get the number of morph targets */
    RwInt32 numMorphTargets = RpGeometryGetNumMorphTargets(geometry);

	// Morph targets not supported in MBR
	ASSERT(1 == numMorphTargets);

    /* if anything has changed, we have to instance our geometry */
    if ( (0 != (geometry->lockedSinceLastInst & rpGEOMETRYLOCKALL)) ||
         (FALSE == reinstance) )
    {
		if (!(geometry->flags & rpGEOMETRYNATIVE)) {
			RwRGBA* pNightTable = EXTRAVERT_NIGHTTABLE(geometry);
			RwRGBA* pDayTable = EXTRAVERT_DAYTABLE(geometry);
			ASSERT(pNightTable);
			ASSERT(pDayTable);

			RwUInt8 *baseVertexMem;
			RwUInt32 curOffset;

			if ( reinstance ) {
				if (instanceData->emuArrayRef) {
					emu_ArraysDelete(instanceData->emuArrayRef);
				}
			}

			baseVertexMem = (RwUInt8 *)NULL;

			if ( FALSE == reinstance ) {
				extern RwUInt8 RwHackNoCompressedTexCoords;

				/* position */
				instanceData->vertexStride = sizeof(RwV3d);

				/* normals */
				if ( 0 != (geomFlags & rxGEOMETRY_NORMALS) ) {
					instanceData->vertexStride += sizeof(RwV3d);
				}

				/* prelight */
				if ( 0 != (geomFlags & rxGEOMETRY_PRELIT) ) {
					instanceData->vertexStride += sizeof(RwRGBA) * 2;
				}

				/* texture coordinates */
				if (RwHackNoCompressedTexCoords) {
					instanceData->vertexStride += numTexCoords * sizeof(float) * 2;
				} else {
					instanceData->vertexStride += numTexCoords * sizeof(RwInt16) * 2;
				}

				instanceData->vertexDataSize = instanceData->numVertices * instanceData->vertexStride;
			}

			/* allocate the vertex memory, either in system or video memory with the latter preferred */
			baseVertexMem = (RwUInt8 *) malloc( instanceData->vertexDataSize );
			ASSERT( NULL != baseVertexMem );

			_rxOpenGLAllInOneAtomicInstanceVertexArray( instanceData,
														atomic,
														geometry,
														geomFlags,
														numTexCoords,
														reinstance,
														baseVertexMem,
														pDayTable,
														pNightTable);

			emu_ArraysReset();
			
			if (instanceData->indexData) {
				emu_ArraysIndices(instanceData->indexData, GL_UNSIGNED_SHORT, instanceData->numIndices);
			}

			emu_ArraysVertex(baseVertexMem, instanceData->vertexDataSize, instanceData->numVertices, instanceData->vertexStride);
		

			/* position */
			emu_ArraysVertexAttrib(0, 3, GL_FLOAT, GL_FALSE, 0);
			curOffset = sizeof(RwV3d);

			/* normals */
			if ( 0 != (geomFlags & rxGEOMETRY_NORMALS) ) {
				emu_ArraysVertexAttrib(2, 3, GL_FLOAT, GL_FALSE, curOffset);
				curOffset += sizeof(RwV3d);
			}

			/* prelight */
			if ( 0 != (geomFlags & rxGEOMETRY_PRELIT) ) {
				
				emu_ArraysVertexAttrib(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, curOffset);
				emu_ArraysVertexAttrib(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, curOffset+4);
				curOffset += sizeof(RwRGBA) * 2;
			}

			/* texture coordinates */
			if (numTexCoords > 0) {
				extern RwUInt8 RwHackNoCompressedTexCoords;

				if (RwHackNoCompressedTexCoords) {
					emu_ArraysVertexAttrib(1, 2, GL_FLOAT, GL_FALSE, curOffset);
				} else {
					emu_ArraysVertexAttrib(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, curOffset);
				}
			}

			// WDTODO : Don't set the hint that this should be using cpu buffers or a buffer update.. if there's a way to detect
			// a diff in something other than the modulate material color???
			instanceData->emuArrayRef = emu_ArraysStore(reinstance ? GL_TRUE : GL_FALSE, GL_TRUE);
		} else {
			instanceData->vertexBufferRef = geometry->vertexBuffer;
		}
    }

    return TRUE;
}

void CCustomBuildingDNPipeline::CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags) {
	emu_SetSecondVertexColor(GL_TRUE, m_fDNBalanceParam);

    /* get the instanced data */
    RxOpenGLResEntryHeader* resEntryHeader = (RxOpenGLResEntryHeader *)(repEntry + 1);
    ASSERT( NULL != resEntryHeader );

    RxOpenGLMeshInstanceData* instanceData = (RxOpenGLMeshInstanceData *)(resEntryHeader + 1);
    ASSERT( NULL != instanceData );

    RwUInt16 numMeshes = resEntryHeader->numMeshes;
    while ( numMeshes-- )
    {
        const RwRGBA    *matColor;

        RwBool          vertexAlphaEnabled;

        ASSERT( NULL != instanceData->material );

        matColor = RpMaterialGetColor( instanceData->material );
        ASSERT( NULL != matColor );

        if ((FALSE != instanceData->vertexAlpha) ||
             (0xFF != matColor->alpha)) {
			// don't draw zero alpha objects
			if (matColor->alpha == 0) {
				instanceData++;
				continue;
			}

            vertexAlphaEnabled = TRUE;
			emu_EnableAlphaModulate(matColor->alpha / 255.0f);
        } else {
            vertexAlphaEnabled = FALSE;
        }
        _rwOpenGLSetRenderState( rwRENDERSTATEVERTEXALPHAENABLE, (void *)vertexAlphaEnabled );

        if (FALSE != _rwOpenGLIsLightingEnabled()) {
            /* set up the material properties */
            _rwOpenGLLightsSetMaterialProperties( instanceData->material,
                                                  flags );
        } else {
            _rwOpenGLDisableColorMaterial();
            if (0 == (flags & rxGEOMETRY_PRELIT)) {
                glColor4fv( (const GLfloat *)&_rwOpenGLOpaqueBlack );
            }
        }    
		
		const uint32 matPassMask = CBMATFX_GET_PASS_CFG(instanceData->material);
		const bool bUseEnvMap1		= (matPassMask&CARFX_ENVMAP1_PASS_BIT);

		// EnvMap1:
		if(bUseEnvMap1) {
			CustomEnvMapPipeMaterialData *pEnvMatData = GETENVMAPPIPEDATAPTR(instanceData->material);
			ASSERT(pEnvMatData);
			RwTexture *pEnvTexture = pEnvMatData->pEnvTexture;
			ASSERT(pEnvTexture);

			RwV2d offset = {0};
			SetEnvMapTexture(pEnvTexture, pEnvMatData->GetfShininess(), pEnvMatData->GetfEnvScaleX(), pEnvMatData->GetfEnvScaleY(), offset);
		}

        /* bind or unbind textures */
        if (0 != (flags & (rxGEOMETRY_TEXTURED | rpGEOMETRYTEXTURED2))) {
            RwTexture   *matTexture;
			

            matTexture = RpMaterialGetTexture( instanceData->material );
            if ( NULL != matTexture ) {
                RwRaster    *matRaster;
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
            } else {
                _rwOpenGLSetRenderState( rwRENDERSTATETEXTURERASTER, (void *)NULL );
            }
        } else {
            _rwOpenGLSetRenderState( rwRENDERSTATETEXTURERASTER, (void *)NULL );
        }

		instanceData->DrawStored();

		instanceData += 1;
		emu_DisableAlphaModulate();
		
		if(bUseEnvMap1) {
			ResetEnvMap();
		}
	}

	emu_SetSecondVertexColor(GL_FALSE);
}

RxPipeline* CCustomBuildingDNPipeline::CreateCustomObjPipe() {
	RxPipeline *pipe = RxPipelineCreate();
	RxNodeDefinition *nodedef = RxNodeDefinitionGetOpenGLAtomicAllInOne();

	if(pipe != NULL)
	{
		RxLockedPipe *lpipe = RxPipelineLock(pipe);
		if ( lpipe != NULL )
		{
			if ( RxLockedPipeAddFragment(lpipe, NULL, nodedef, NULL) )
			{
				if ( RxLockedPipeUnlock(lpipe) )
				{
					RxPipelineNode *allinone = RxPipelineFindNodeByName(pipe, nodedef->name, NULL, 0);

					// it's supported only in generic InstanceCB:
					RxOpenGLAllInOneSetInstanceCallBack(allinone, CustomPipeInstanceCB);
					RxOpenGLAllInOneSetRenderCallBack(allinone,	CustomPipeRenderCB);
					pipe->pluginId	= PDSPC_CBuildingDNEnvMap_AtmID;
					pipe->pluginData= PDSPC_CBuildingDNEnvMap_AtmID;

					return (pipe); // success
				}
			}
		}
		RxPipelineDestroy(pipe);
	}
    return(NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------ ExtraVertColour plugin stuff----------
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// extra vertex color table ("Night Color") attached to geometry:
RwBool CCustomBuildingDNPipeline::ExtraVertColourPluginAttach() {
	ms_extraVertColourPluginOffset = -1;

	ms_extraVertColourPluginOffset =  RpGeometryRegisterPlugin(	EXTRAVERTCOLOUR_SIZE,
																MAKECHUNKID(rwVENDORID_ROCKSTAR, rwEXTRAVERTCOL_ID),
																pluginExtraVertColourConstructorCB,
																pluginExtraVertColourDestructorCB,
																NULL);
	
	if(ms_extraVertColourPluginOffset != -1) {
		if(RpGeometryRegisterPluginStream(	MAKECHUNKID(rwVENDORID_ROCKSTAR, rwEXTRAVERTCOL_ID), 
											pluginExtraVertColourStreamReadCB,
											pluginExtraVertColourStreamWriteCB,
											pluginExtraVertColourStreamGetSizeCB) < 0)
		{
			ms_extraVertColourPluginOffset = -1;
		}
	}
		
	return(ms_extraVertColourPluginOffset != -1);
}

void* CCustomBuildingDNPipeline::pluginExtraVertColourConstructorCB(void* object, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__) {
	RpGeometry *pGeom = (RpGeometry*)object;
	uint8 **ppInfo = NULL;
	
	if(ms_extraVertColourPluginOffset > 0) {
		ppInfo = (uint8**)GETPLUGINBASE(pGeom);
		memset(ppInfo, 0, EXTRAVERTCOLOUR_SIZE);
	}
	
	return(object);
}

void* CCustomBuildingDNPipeline::pluginExtraVertColourDestructorCB(void* object, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__) {
	RpGeometry *pGeom = (RpGeometry*)object;

	RwRGBA* pTable = EXTRAVERT_NIGHTTABLE(pGeom);
	if(pTable) {
		GtaFree(pTable);
		EXTRAVERT_NIGHTTABLE(pGeom) = NULL;
	}
	
	pTable = EXTRAVERT_DAYTABLE(pGeom);
	if(pTable) {
		GtaFree(pTable);
		EXTRAVERT_DAYTABLE(pGeom) = NULL;
	}

	return(object);
}

RwStream* CCustomBuildingDNPipeline::pluginExtraVertColourStreamWriteCB(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size) {	
	RpGeometry* pGeom = (RpGeometry*)pData;
	uint8** ppInfo = (uint8**)EXTRAVERT_NIGHTTABLE(pData);

	RwStreamWrite(pStream, ppInfo, sizeof(uint8*));

	uint8* pInfo = *ppInfo;
	if(!pInfo) {
		return(pStream);
	}

	RwStreamWrite(pStream, pInfo, sizeof(uint8) * RpGeometryGetNumVertices(pGeom) * 4);

	return(pStream);
}

RwInt32 CCustomBuildingDNPipeline::pluginExtraVertColourStreamGetSizeCB(const void* pData, RwInt32 offset, RwInt32 size) {
	RpGeometry* pGeom = (RpGeometry*)pData;
	if(!pGeom)
		return(0);

	// the stream only contains the night prelit data
	const uint32 uiSize = 4 + (sizeof(uint8) * RpGeometryGetNumVertices(pGeom) * 4 * 2);

	return(uiSize);
}

RwStream* CCustomBuildingDNPipeline::pluginExtraVertColourStreamReadCB(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size) {
	RpGeometry* pGeom = (RpGeometry*)pData;
	CDNGeomExtension*	pGeomExtension = (CDNGeomExtension*)GETPLUGINBASE(pData);

	// this ptr to the DN table isn't used by PC - we have a more complex structure for the plugin
	uint8*	trash;
	
	RwStreamRead(pStream, &trash, sizeof(uint8*));
	if(!trash) {
		return(pStream);
	}

	//alloc memory for the tables and set up the ptrs in the extension structure
	pGeomExtension->m_pNightColTable	= (RwRGBA*)GtaMalloc(4 * RpGeometryGetNumVertices(pGeom) * sizeof(uint8));
	pGeomExtension->m_pDayColTable		= (RwRGBA*)GtaMalloc(4 * RpGeometryGetNumVertices(pGeom) * sizeof(uint8));
	pGeomExtension->m_fCurrentDNValue	= 1.0f;	//full day

	//stream data only contains the night table values (day is copied from prelitLum in geometry)
	RwStreamRead(pStream, pGeomExtension->m_pNightColTable, sizeof(uint8) * RpGeometryGetNumVertices(pGeom) * 4);	
	
	// PC version also contains a second table which is a copy of the original prelitLum data
	const int32 tableSize = RpGeometryGetNumVertices(pGeom);
	RwRGBA* pColourTable = RpGeometryGetPreLightColors(pGeom);
	
	// copy colour table (if there is one)
	if (pColourTable) {
		RwRGBA* pDayTable = (RwRGBA*)EXTRAVERT_DAYTABLE(pData);
		RwRGBA* pNightTable = (RwRGBA*)EXTRAVERT_NIGHTTABLE(pData);
		
		for(int32 i=0; i<tableSize; i++) {
			pDayTable[i].red = pColourTable[i].red;
			pDayTable[i].blue = pColourTable[i].blue;
			pDayTable[i].green = pColourTable[i].green;
			pDayTable[i].alpha = pColourTable[i].alpha;
		}
	}

	return(pStream);
}

// returns pointer to second set of vertex colours (if one exists):
uint8* CCustomBuildingDNPipeline::GetExtraVertColourPtr(RpGeometry *pGeom) {
	ASSERT(ms_extraVertColourPluginOffset >0);
	return((uint8*)EXTRAVERT_NIGHTTABLE(pGeom));
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

void CCustomBuildingDNPipeline::SetFxEnvTexture(RpMaterial *pMaterial, RwTexture *fxTexture) {
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

RwTexture* CCustomBuildingDNPipeline::GetFxEnvTexture(RpMaterial *pMaterial) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(NULL);

	return(pData->pEnvTexture);
}

void CCustomBuildingDNPipeline::SetFxEnvScale(RpMaterial *pMaterial, float envScaleX, float envScaleY) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvScaleX(envScaleX);
	pData->SetfEnvScaleY(envScaleY);
}

float CCustomBuildingDNPipeline::GetFxEnvScaleX(RpMaterial *pMaterial) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvScaleX());
}

float CCustomBuildingDNPipeline::GetFxEnvScaleY(RpMaterial *pMaterial) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvScaleY());
}

void CCustomBuildingDNPipeline::SetFxEnvTransScl(RpMaterial *pMaterial, float envTransX, float envTransY) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvTransSclX(envTransX);
	pData->SetfEnvTransSclY(envTransY);
}

float CCustomBuildingDNPipeline::GetFxEnvTransSclX(RpMaterial *pMaterial) {
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclX());
}

float CCustomBuildingDNPipeline::GetFxEnvTransSclY(RpMaterial *pMaterial) {
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclY());
}

void CCustomBuildingDNPipeline::SetFxEnvShininess(RpMaterial *pMaterial, float envShininess) {
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;
	
	pData->SetfShininess(envShininess);
}

float CCustomBuildingDNPipeline::GetFxEnvShininess(RpMaterial *pMaterial) {
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfShininess());
}

void SetNormalMatrix(float scaleX, float scaleY, RwV2d offset) {
	const RwMatrix texMat =
    {
        {scaleX, 0.0,    0.0},   0,
        {0.0,    scaleY, 0.0},   0,
        {0.0,    0.0,    1.0},   0,
        {scaleX, scaleY, 0.0},   0
    };

    glMatrixMode( GL_TEXTURE );
    glPushMatrix();
    glLoadIdentity();

    RwCamera        *camera;
    RwFrame         *camFrame;
    const RwMatrix  *camLTM;
    RwMatrix        *tmpMatrix;
    RwMatrix        *result;

    tmpMatrix = RwMatrixCreate();
    result = RwMatrixCreate();

    /* transform the normals back into world space */
    camera = RwCameraGetCurrentCamera();
    ASSERT( NULL != camera );

    camFrame = RwCameraGetFrame( camera );
    ASSERT( NULL != camFrame );

    camLTM = RwFrameGetLTM( camFrame );
    ASSERT( NULL != camLTM );

	RwMatrixCopy(tmpMatrix, camLTM);

    tmpMatrix->flags = 0;

    tmpMatrix->pos.x = offset.x;
    tmpMatrix->pos.y = offset.y;
    tmpMatrix->pos.z = 0.0f;

    RwMatrixMultiply( result, tmpMatrix, &texMat );

    _rwOpenGLApplyRwMatrix( result );

    RwMatrixDestroy( tmpMatrix );
    RwMatrixDestroy( result );
}

void SetEnvMapTexture(RwTexture* tex, float shininess, float scaleX, float scaleY, RwV2d offset) {
    RwRaster *envMapRaster = RwTextureGetRaster( tex );
	ASSERT(envMapRaster);
   
	RwTextureSetAddressing(tex, rwTEXTUREADDRESSWRAP);

	emu_SetEnvMap(((_rwGlRasterExt *)(((RwUInt8 *)(envMapRaster)) + RasterExtOffset))->tex, shininess * 1.5f);

	SetNormalMatrix(scaleX, scaleY, offset);
}

void ResetEnvMap() {
	emu_ResetEnvMap();

    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
}