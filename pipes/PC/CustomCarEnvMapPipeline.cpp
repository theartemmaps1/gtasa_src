//
// CustomCarEnvMapPipeline - custom vehicle pipeline for D3D9;
//
//	24/01/2005	- Andrzej:	- initial;
//	28/01/2005	- Andrzej:	- EnvMapPipeAtomic plugin support added;
//	31/01/2005	- Andrzej:	- EnvMapPipeMaterial plugin support added;
//	02/02/2005	- Andrzej:	- PreRenderUpdate() added;
//	14/02/2005	- Andrzej:	- support for CustomPipelinesCommonPC.h added;
//	17/02/2005	- Andrzej:	- new version of InstanceCB() added;
//	09/06/2005	- Andrzej:	- CustomPipeRenderCB: fix for blown-up vehicles added (they're dark now);
//
//
//

#include "d3d9.h"

#include "rwcore.h"
#include "rpworld.h"
#include "dma.h"
#include "world.h"					// WORLD_WIDTH/DEPTHOFSECTOR...
#include "rwplugin.h"
#include "VisibilityPlugins.h"		// GetAtomicId(), etc.
#include "VehicleModelInfo.h"
#include "PipelinePlugin.h"
#include "fx.h"						// g_fx.GetFxQuality

#include "CustomEnvMapPipelineCSL.h"
#include "CustomPipelinesCommon.h"
#include "CustomCarEnvMapPipeline.h"




//
// all vertex declarations + terminator declaration:
//
#define VERTELEM_NUMDECLARATIONS	(4+1)

//
// Light[0] is game's directional light, so we use Light[1] for specular lighting:
//
#define CARFX_D3D9_SPEC_LIGHT_INDEX			(1)

D3DLIGHT9							CCustomCarEnvMapPipeline::m_gSpecularLight;	// global specular light used by the pipe

//
//
//
//
RxPipeline* 						CCustomCarEnvMapPipeline::ObjPipeline				= NULL;

int32								CCustomCarEnvMapPipeline::ms_envMapPluginOffset		= -1;
int32								CCustomCarEnvMapPipeline::ms_envMapAtmPluginOffset	= -1;
int32								CCustomCarEnvMapPipeline::ms_specularMapPluginOffset= -1;

CustomEnvMapPipeMaterialDataPool*	CCustomCarEnvMapPipeline::m_gEnvMapPipeMatDataPool	= NULL;
CustomEnvMapPipeAtomicDataPool*		CCustomCarEnvMapPipeline::m_gEnvMapPipeAtmDataPool	= NULL;
CustomSpecMapPipeMaterialDataPool*	CCustomCarEnvMapPipeline::m_gSpecMapPipeMatDataPool	= NULL;


#ifndef FINALBUILD
	#define USE_POOL_COUNTER	// pool counter available only in non-final compilation
#endif

#ifdef USE_POOL_COUNTER
	// local counter to see how much pool is filled during execution:
	static int32	nEnvMapPipeMatDataPoolCounter=0;
	static int32	nEnvMapPipeAtmDataPoolCounter=0;
	static int32	nSpecMapPipeMatDataPoolCounter=0;
#endif

// fake EnvMapPipeMaterialData to save on default setting:
CustomEnvMapPipeMaterialData		CCustomCarEnvMapPipeline::fakeEnvMapPipeMatData;

//
//
// contains LightMultiplier calculated by CRenderer::SetupVehiclePedLights();
//
float CCustomCarEnvMapPipeline::m_EnvMapLightingMult = 1.0f;




//
//
//
//
CCustomCarEnvMapPipeline::CCustomCarEnvMapPipeline()
{
	// do nothing
}

//
//
//
//
CCustomCarEnvMapPipeline::~CCustomCarEnvMapPipeline()
{
	// do nothing
}




//
//
//
//
RpMaterial* CCustomCarEnvMapPipeline::CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__)
{
	ASSERT(pMaterial);
	ASSERT(ObjPipeline);

	CARFX_SET_PASS_CFG(pMaterial, 0);	// clear pass bits


const RpMatFXMaterialFlags effects = RpMatFXMaterialGetEffects(pMaterial);
	if(effects == rpMATFXEFFECTENVMAP)
	{
		// set EnvMapFX texture from MatFX:
		CCustomCarEnvMapPipeline::SetFxEnvTexture(pMaterial, NULL);
	}


uint32 passMask = 0;

	if( (CCustomCarEnvMapPipeline::GetFxEnvShininess(pMaterial) != 0.0f)	&&
		(CCustomCarEnvMapPipeline::GetFxEnvTexture(pMaterial)				)																	)
	{
		RwTexture *pEnvMapTex = CCustomCarEnvMapPipeline::GetFxEnvTexture(pMaterial);
		if(IsTextureUsingEnvMap2FX(pEnvMapTex))
		{
			// note: additionaly second set of UVs must be set for geometry using this pipe!
			passMask |= CARFX_ENVMAP2_PASS_BIT;
		}
		else
		{
			passMask |= CARFX_ENVMAP1_PASS_BIT;
		}
	}
	

	if(	(CCustomCarEnvMapPipeline::GetFxSpecSpecularity(pMaterial) != 0.0f)	&&
		(CCustomCarEnvMapPipeline::GetFxSpecTexture(pMaterial)				)													)
	{
		passMask |= CARFX_SPECMAP_PASS_BIT;
	}


	//
	// set pass configuration to material:
	// use 3 least signifant bits in pMaterial->surfaceProps.specular;
	//
	uint32 matPassMask = CARFX_GET_PASS_CFG(pMaterial);
	{
		matPassMask &= ~(CARFX_ALLMASK_PASS_BIT);
		matPassMask |= passMask;
	}
	CARFX_SET_PASS_CFG(pMaterial, matPassMask);


	return(pMaterial);
}// end of CustomPipeMaterialSetup()...



//
//
//
//
RpAtomic* CCustomCarEnvMapPipeline::CustomPipeAtomicSetup(RpAtomic *pAtomic)
{
	ASSERT(pAtomic);
	ASSERT(ObjPipeline);


	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);

	RpGeometryForAllMaterials(pGeometry, CCustomCarEnvMapPipeline::CustomPipeMaterialSetup, NULL);

	if(!RpAtomicSetPipeline(pAtomic, ObjPipeline))
		return(NULL);

	SetPipelineID(pAtomic, PDSPC_CarFXPipe_AtmID);

	return(pAtomic);
}// end of CustomPipeAtomicSetup()...




////////////////////////////////////////////////////////////////////////////////////////////////////


//
//
// old version of instanceCB - doesn't support re-instancing:
//
#if 0	//#if defined(GTA_PC)
RwBool CCustomCarEnvMapPipeline::CustomPipeInstanceCB(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
{

D3DVERTEXELEMENT9    declaration[VERTELEM_NUMDECLARATIONS];
RwUInt32             declarationIndex, offset;
RwUInt8             *lockedVertexBuffer;
RxD3D9VertexStream  *vertexStream;
RxD3D9InstanceData  *instancedData;
RwUInt32             vbSize;

	ASSERT(reinstance==FALSE);	// no support for re-instancing

const RpAtomic*		pAtomic = (const RpAtomic*)object;
RpGeometry*			pGeometry = pAtomic->geometry;
	ASSERT(pGeometry);
const RwUInt32 geometryFlags = RpGeometryGetFlags(pGeometry);

RpMorphTarget *pMorphTarget = RpGeometryGetMorphTarget(pGeometry, 0);
	ASSERT(pMorphTarget);
	
	resEntryHeader->totalNumVertex = pGeometry->numVertices;
    vertexStream = &(resEntryHeader->vertexStream[0]);


const bool bInstanceXYZ		= TRUE;	// obligatory
const bool bInstanceUV		= bool(geometryFlags&(rpGEOMETRYTEXTURED|rpGEOMETRYTEXTURED2));
const bool bInstanceUV2		= bool(geometryFlags&rpGEOMETRYTEXTURED2);
const bool bInstanceNormal	= bool(geometryFlags&rpGEOMETRYNORMALS);	


	declarationIndex = 0;
    offset = 0;

	// Positions:
	if(bInstanceXYZ)
	{
		declaration[declarationIndex].Stream	= 0;
		declaration[declarationIndex].Offset	= offset;
		declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT3;
		declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
		declaration[declarationIndex].Usage		= D3DDECLUSAGE_POSITION;
		declaration[declarationIndex].UsageIndex= 0;
		declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
		offset += sizeof(RwV3d);
		vertexStream->stride = sizeof(RwV3d);
		vertexStream->geometryFlags = rpGEOMETRYLOCKVERTICES;
	}

	// first pass UVs:
	//ASSERT(geometryFlags&(rpGEOMETRYTEXTURED|rpGEOMETRYTEXTURED2));
	if(bInstanceUV)
	{
		declaration[declarationIndex].Stream	= 0;
		declaration[declarationIndex].Offset	= offset;
		declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT2;
		declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
		declaration[declarationIndex].Usage		= D3DDECLUSAGE_TEXCOORD;
		declaration[declarationIndex].UsageIndex= 0;
		declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
		offset += sizeof(RwV2d);
		vertexStream->stride += sizeof(RwV2d);
		vertexStream->geometryFlags |= rpGEOMETRYLOCKTEXCOORDS1;
	}

	// second pass UVs:
	//ASSERT(geometryFlags&rpGEOMETRYTEXTURED2);
	if(bInstanceUV2)
	{
		// second set of UVs:
		declaration[declarationIndex].Stream	= 0;
		declaration[declarationIndex].Offset	= offset;
		declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT2;
		declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
		declaration[declarationIndex].Usage		= D3DDECLUSAGE_TEXCOORD;
		declaration[declarationIndex].UsageIndex= 1;
		declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
		offset += sizeof(RwV2d);
		vertexStream->stride += sizeof(RwV2d);
		vertexStream->geometryFlags |= rpGEOMETRYLOCKTEXCOORDS2;
	}

	// Normals:
	//ASSERT(geometryFlags&rpGEOMETRYNORMALS);
	if(bInstanceNormal)
	{
		declaration[declarationIndex].Stream	= 0;
		declaration[declarationIndex].Offset	= offset;
		declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT3;
		declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
		declaration[declarationIndex].Usage		= D3DDECLUSAGE_NORMAL;
		declaration[declarationIndex].UsageIndex= 0;
		declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
		offset += sizeof(RwV3d);
		vertexStream->stride += sizeof(RwV3d);
		vertexStream->geometryFlags |= rpGEOMETRYLOCKNORMALS;
	}

	// no more vertex data:
	declaration[declarationIndex].Stream	= 0xFF;
    declaration[declarationIndex].Offset	= 0;
    declaration[declarationIndex].Type		= D3DDECLTYPE_UNUSED;
    declaration[declarationIndex].Method	= 0;
    declaration[declarationIndex].Usage		= 0;
    declaration[declarationIndex].UsageIndex= 0;


    // Set the vertex shader flags
    if(!RwD3D9CreateVertexDeclaration(declaration, &(resEntryHeader->vertexDeclaration)))
	{
		return(FALSE);
	}

    // Create the vertex buffer
    vbSize = (vertexStream->stride) * (resEntryHeader->totalNumVertex);

	vertexStream->managed = TRUE;

    if(!RwD3D9CreateVertexBuffer(vertexStream->stride, vbSize, &vertexStream->vertexBuffer, &vertexStream->offset))
    {
        return(FALSE);
    }

    // Fix base index:
    resEntryHeader->useOffsets = FALSE;

    const RwInt32 numMeshes = resEntryHeader->numMeshes;
    instancedData = (RxD3D9InstanceData *)(resEntryHeader + 1);

    for(RwInt32 i=0; i<numMeshes; i++)
    {
        instancedData->baseIndex = instancedData->minVert + (vertexStream->offset / vertexStream->stride);
        instancedData++;
    }


	lockedVertexBuffer = NULL;

	//
    // Lock the vertex buffer
    //
    IDirect3DVertexBuffer9_Lock((LPDIRECT3DVERTEXBUFFER9)vertexStream->vertexBuffer,
                                vertexStream->offset, vbSize, (void**)&lockedVertexBuffer, 0);
	ASSERT(lockedVertexBuffer);
	if(!lockedVertexBuffer)
	{
		return(FALSE);
	}

    declarationIndex = 0;
    offset = 0;

	// Positions:
	if(bInstanceXYZ)
	{
		const RwV3d* positions = (const RwV3d*)RpMorphTargetGetVertices(pMorphTarget);
		ASSERT(positions);

		offset += _rpD3D9VertexDeclarationInstV3d(declaration[declarationIndex].Type,
												lockedVertexBuffer + offset,
												positions,
												resEntryHeader->totalNumVertex,
												vertexStream->stride);
		declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
	}

    // Texture coordinates:
	if(bInstanceUV)
	{
        const RwV2d *texCoords = (const RwV2d *)(pGeometry->texCoords[0]);
		ASSERT(texCoords);

        offset += _rpD3D9VertexDeclarationInstV2d(declaration[declarationIndex].Type,
                                                  lockedVertexBuffer + offset,
                                                  texCoords,
                                                  resEntryHeader->totalNumVertex,
                                                  vertexStream->stride);
        declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
    }

	// Tex coordinates 2:
	if(bInstanceUV2)
	{
		const RwV2d *texCoords2 = (const RwV2d*)RpGeometryGetVertexTexCoords(pGeometry, rwTEXTURECOORDINATEINDEX1);
		ASSERT(texCoords2);

        offset += _rpD3D9VertexDeclarationInstV2d(declaration[declarationIndex].Type,
                                                  lockedVertexBuffer + offset,
                                                  texCoords2,
                                                  resEntryHeader->totalNumVertex,
                                                  vertexStream->stride);
        declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
	}

	// Normals:
	if(bInstanceNormal)
	{
		const RwV3d* normals = (const RwV3d*)RpMorphTargetGetVertexNormals(pMorphTarget);
		ASSERT(normals);

		offset += _rpD3D9VertexDeclarationInstV3d(declaration[declarationIndex].Type,
												lockedVertexBuffer + offset,
												normals,
												resEntryHeader->totalNumVertex,
												vertexStream->stride);
		declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
	}

    // Unlock the vertex buffer
    IDirect3DVertexBuffer9_Unlock((LPDIRECT3DVERTEXBUFFER9)vertexStream->vertexBuffer);

	return(TRUE);
}
#endif //GTA_PC...


//
//
//
//
#if defined(GTA_PC)
RwBool CCustomCarEnvMapPipeline::CustomPipeInstanceCB(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
{

D3DVERTEXELEMENT9    declaration[VERTELEM_NUMDECLARATIONS];
RwUInt32             declarationIndex, offset;
RwUInt8             *lockedVertexBuffer;
RxD3D9VertexStream  *vertexStream;
RxD3D9InstanceData  *instancedData;
RwUInt32             vbSize;
RwUInt32			lockedSinceLastInst;


	const RpAtomic*		pAtomic = (const RpAtomic*)object;
	ASSERT(pAtomic);
	RpGeometry*			pGeometry = pAtomic->geometry;
	ASSERT(pGeometry);
	const RwUInt32 geometryFlags = RpGeometryGetFlags(pGeometry);

	RpMorphTarget *pMorphTarget = RpGeometryGetMorphTarget(pGeometry, 0);
	ASSERT(pMorphTarget);
	

const bool bInstanceXYZ		= TRUE;	// obligatory
const bool bInstanceUV		= bool(geometryFlags&(rpGEOMETRYTEXTURED|rpGEOMETRYTEXTURED2));
const bool bInstanceUV2		= bool(geometryFlags&rpGEOMETRYTEXTURED2);
const bool bInstanceNormal	= bool(geometryFlags&rpGEOMETRYNORMALS);	



    if(!reinstance)
    {
		resEntryHeader->totalNumVertex = pGeometry->numVertices;
		vertexStream = &(resEntryHeader->vertexStream[0]);

		// delete old vertexBuffer (if present):
		if(vertexStream->vertexBuffer != NULL)
        {
            ASSERT(resEntryHeader->vertexStream[0].managed==TRUE);	// no dynamic vbuffers
            RwD3D9DestroyVertexBuffer(vertexStream->stride,
				vertexStream->stride * (resEntryHeader->totalNumVertex),
				vertexStream->vertexBuffer, vertexStream->offset);
            vertexStream->vertexBuffer = NULL;
        }

        vertexStream->offset		= 0;
        vertexStream->stride		= 0;
        vertexStream->geometryFlags = 0;
        vertexStream->managed		= FALSE;
        vertexStream->dynamicLock	= FALSE;


		declarationIndex = 0;
		offset = 0;

		// Positions:
		if(bInstanceXYZ)
		{
			declaration[declarationIndex].Stream	= 0;
			declaration[declarationIndex].Offset	= offset;
			declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT3;
			declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
			declaration[declarationIndex].Usage		= D3DDECLUSAGE_POSITION;
			declaration[declarationIndex].UsageIndex= 0;
			declarationIndex++;
			ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
			offset += sizeof(RwV3d);
			vertexStream->stride = sizeof(RwV3d);
			vertexStream->geometryFlags = rpGEOMETRYLOCKVERTICES;
		}

		// first pass UVs:
		//ASSERT(geometryFlags&(rpGEOMETRYTEXTURED|rpGEOMETRYTEXTURED2));
		if(bInstanceUV)
		{
			declaration[declarationIndex].Stream	= 0;
			declaration[declarationIndex].Offset	= offset;
			declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT2;
			declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
			declaration[declarationIndex].Usage		= D3DDECLUSAGE_TEXCOORD;
			declaration[declarationIndex].UsageIndex= 0;
			declarationIndex++;
			ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
			offset += sizeof(RwV2d);
			vertexStream->stride += sizeof(RwV2d);
			vertexStream->geometryFlags |= rpGEOMETRYLOCKTEXCOORDS1;
		}

		// second pass UVs:
		//ASSERT(geometryFlags&rpGEOMETRYTEXTURED2);
		if(bInstanceUV2)
		{
			// second set of UVs:
			declaration[declarationIndex].Stream	= 0;
			declaration[declarationIndex].Offset	= offset;
			declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT2;
			declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
			declaration[declarationIndex].Usage		= D3DDECLUSAGE_TEXCOORD;
			declaration[declarationIndex].UsageIndex= 1;
			declarationIndex++;
			ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
			offset += sizeof(RwV2d);
			vertexStream->stride += sizeof(RwV2d);
			vertexStream->geometryFlags |= rpGEOMETRYLOCKTEXCOORDS2;
		}


		// Normals:
		//ASSERT(geometryFlags&rpGEOMETRYNORMALS);
		if(bInstanceNormal)
		{
			declaration[declarationIndex].Stream	= 0;
			declaration[declarationIndex].Offset	= offset;
			declaration[declarationIndex].Type		= D3DDECLTYPE_FLOAT3;
			declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
			declaration[declarationIndex].Usage		= D3DDECLUSAGE_NORMAL;
			declaration[declarationIndex].UsageIndex= 0;
			declarationIndex++;
			ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
			offset += sizeof(RwV3d);
			vertexStream->stride += sizeof(RwV3d);
			vertexStream->geometryFlags |= rpGEOMETRYLOCKNORMALS;
		}

		// no more vertex data:
		declaration[declarationIndex].Stream	= 0xFF;
		declaration[declarationIndex].Offset	= 0;
		declaration[declarationIndex].Type		= D3DDECLTYPE_UNUSED;
		declaration[declarationIndex].Method	= 0;
		declaration[declarationIndex].Usage		= 0;
		declaration[declarationIndex].UsageIndex= 0;


		// Set the vertex shader flags
		if(!RwD3D9CreateVertexDeclaration(declaration, &(resEntryHeader->vertexDeclaration)))
		{
			return(FALSE);
		}

		// Create the vertex buffer
		vbSize = (vertexStream->stride) * (resEntryHeader->totalNumVertex);
		vertexStream->managed = TRUE;

		if(!RwD3D9CreateVertexBuffer(vertexStream->stride, vbSize, &vertexStream->vertexBuffer, &vertexStream->offset))
		{
			return(FALSE);
		}

		// Fix base index:
		resEntryHeader->useOffsets = FALSE;

		const RwInt32 numMeshes = resEntryHeader->numMeshes;
		instancedData = (RxD3D9InstanceData *)(resEntryHeader + 1);

		for(RwInt32 i=0; i<numMeshes; i++)
		{
			instancedData->baseIndex = instancedData->minVert + (vertexStream->offset / vertexStream->stride);
			instancedData++;
		}

    }	// if(!reinstance)...
    else	
    {
		uint32 n=0;
		IDirect3DVertexDeclaration9_GetDeclaration((LPDIRECT3DVERTEXDECLARATION9)resEntryHeader->vertexDeclaration,
													declaration, &n);
        ASSERT(n <= VERTELEM_NUMDECLARATIONS);
    }


    //
    // Lock the vertex buffer
    //
	lockedVertexBuffer = NULL;

	if (!reinstance)
    {
        lockedSinceLastInst = rpGEOMETRYLOCKALL;

        vertexStream = &(resEntryHeader->vertexStream[0]);
        vbSize = vertexStream->stride * (resEntryHeader->totalNumVertex);

        IDirect3DVertexBuffer9_Lock((LPDIRECT3DVERTEXBUFFER9)vertexStream->vertexBuffer,
            vertexStream->offset, vbSize, (void**)&lockedVertexBuffer,
            D3DLOCK_NOSYSLOCK /*0*/);
    }
    else
    {
        lockedSinceLastInst = pGeometry->lockedSinceLastInst;
		vertexStream = &(resEntryHeader->vertexStream[0]);

        if(	(rpGEOMETRYLOCKALL == lockedSinceLastInst)				||
            ((lockedSinceLastInst & vertexStream->geometryFlags)	!= 0)	)
        {
            vbSize = vertexStream->stride * (resEntryHeader->totalNumVertex);

            ASSERT(vertexStream->managed==TRUE);	// no dynamic vbuffers
            IDirect3DVertexBuffer9_Lock((LPDIRECT3DVERTEXBUFFER9)vertexStream->vertexBuffer,
                vertexStream->offset, vbSize, (void**)&lockedVertexBuffer,
                D3DLOCK_NOSYSLOCK /*0*/);
        }
    }

	if(!lockedVertexBuffer)
	{
		// nothing to (re-)instance
		return(TRUE);
	}


    declarationIndex = 0;
    offset = 0;

	// Positions:
	if(bInstanceXYZ && (lockedSinceLastInst&rpGEOMETRYLOCKVERTICES))
	{
		const RwV3d* positions = (const RwV3d*)RpMorphTargetGetVertices(pMorphTarget);
		ASSERT(positions);

            // Find positions
            declarationIndex = 0;
            while(	(declaration[declarationIndex].Usage != D3DDECLUSAGE_POSITION)	||
					(declaration[declarationIndex].UsageIndex != 0)					)
            {
                declarationIndex++;
            }
			ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);

            _rpD3D9VertexDeclarationInstV3d(declaration[declarationIndex].Type,
								lockedVertexBuffer + declaration[declarationIndex].Offset,
								positions,
								resEntryHeader->totalNumVertex,
								resEntryHeader->vertexStream[declaration[declarationIndex].Stream].stride);
	}

    // Texture coordinates:
	if(bInstanceUV && (lockedSinceLastInst&rpGEOMETRYLOCKTEXCOORDS1))
	{
        const RwV2d *texCoords = (const RwV2d *)(pGeometry->texCoords[0]);
		ASSERT(texCoords);

        // Find tex coords:
        declarationIndex = 0;
        while(	(declaration[declarationIndex].Usage != D3DDECLUSAGE_TEXCOORD)	||
				(declaration[declarationIndex].UsageIndex != 0)					)
        {
            declarationIndex++;
        }
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);

		_rpD3D9VertexDeclarationInstV2d(declaration[declarationIndex].Type,
							lockedVertexBuffer + declaration[declarationIndex].Offset,
							texCoords,
							resEntryHeader->totalNumVertex,
							resEntryHeader->vertexStream[declaration[declarationIndex].Stream].stride);
	}

    // Texture coordinates 2:
	if(bInstanceUV2 && (lockedSinceLastInst&rpGEOMETRYLOCKTEXCOORDS2))
	{
        const RwV2d *texCoords = (const RwV2d *)(pGeometry->texCoords[1]);
		ASSERT(texCoords);

        // Find tex coords:
        declarationIndex = 0;
        while(	(declaration[declarationIndex].Usage != D3DDECLUSAGE_TEXCOORD)	||
				(declaration[declarationIndex].UsageIndex != 1)					)
        {
            declarationIndex++;
        }
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);

		_rpD3D9VertexDeclarationInstV2d(declaration[declarationIndex].Type,
							lockedVertexBuffer + declaration[declarationIndex].Offset,
							texCoords,
							resEntryHeader->totalNumVertex,
							resEntryHeader->vertexStream[declaration[declarationIndex].Stream].stride);
	}

	// Normals:
	if(bInstanceNormal && (lockedSinceLastInst&rpGEOMETRYLOCKNORMALS))
	{
		const RwV3d* normals = (const RwV3d*)RpMorphTargetGetVertexNormals(pMorphTarget);
		ASSERT(normals);

        // Find normals:
        declarationIndex = 0;
        while (declaration[declarationIndex].Usage != D3DDECLUSAGE_NORMAL ||
            declaration[declarationIndex].UsageIndex != 0)
        {
            declarationIndex++;
        }
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);

        _rpD3D9VertexDeclarationInstV3d(declaration[declarationIndex].Type,
							lockedVertexBuffer + declaration[declarationIndex].Offset,
							normals,
							resEntryHeader->totalNumVertex,
							resEntryHeader->vertexStream[declaration[declarationIndex].Stream].stride);
	}

	if(lockedVertexBuffer)
	{
		// Unlock the vertex buffer
		IDirect3DVertexBuffer9_Unlock((LPDIRECT3DVERTEXBUFFER9)vertexStream->vertexBuffer);
	}

	return(TRUE);
}
#endif //GTA_PC...



//
//
//
//
#if defined(GTA_PC)
RwBool CCustomCarEnvMapPipeline::CustomPipeReinstanceCB(void *object, RwResEntry *resEntry, RxD3D9AllInOneInstanceCallBack instanceCallback)
{
    ASSERT(object);
    ASSERT(resEntry);

	const RpAtomic  *atomic		= (const RpAtomic*)object;
    RpGeometry      *geometry	= RpAtomicGetGeometry(atomic);
    ASSERT(geometry);

    //
    // Call the instance callback
    //
    if (instanceCallback)
    {
        RxD3D9ResEntryHeader *resEntryHeader = (RxD3D9ResEntryHeader*)(resEntry + 1);
        if(!instanceCallback(object, resEntryHeader, TRUE))
        {
            return(FALSE);
        }
    }

    return(TRUE);
}
#endif //GTA_PC...



//
// returns fractional part of X:
//
#define CALC_ENVMAP_OFFSETX(X)	((X-float(int32(X/WIDTH_OF_SECTOR)*WIDTH_OF_SECTOR))/WIDTH_OF_SECTOR)
#define CALC_ENVMAP_OFFSETY(Y)	((Y-float(int32(Y/DEPTH_OF_SECTOR)*DEPTH_OF_SECTOR))/DEPTH_OF_SECTOR)

#define CALC_ENVMAP_OFFSETX2(X)	((float(int32(X/WIDTH_OF_SECTOR)*WIDTH_OF_SECTOR)-X)/WIDTH_OF_SECTOR)
#define CALC_ENVMAP_OFFSETY2(Y)	((float(int32(Y/DEPTH_OF_SECTOR)*DEPTH_OF_SECTOR)-Y)/DEPTH_OF_SECTOR)
#define CALC_ENVMAP_SECTORX(X)	(int32(X/WIDTH_OF_SECTOR))
#define CALC_ENVMAP_SECTORY(Y)	(int32(Y/DEPTH_OF_SECTOR))


//
//
//
//
static
RwBool _rpMatFXSyncEnvMap1Data(RpAtomic *pAtomic, CustomEnvMapPipeMaterialData *pEnvMatData, RwV2d *outVec)
{
		ASSERT(pAtomic);
		ASSERT(pEnvMatData);
		ASSERT(outVec);
//
// calculate env offset with use of actual car pos in the world sector:
//
const float WIDTH_OF_SECTOR = WORLD_WIDTHOFSECTOR * pEnvMatData->GetfEnvTransSclX();
const float DEPTH_OF_SECTOR = WORLD_DEPTHOFSECTOR * pEnvMatData->GetfEnvTransSclY();

RpClump *currentClump = RpAtomicGetClump(pAtomic);
RwMatrix *atomicLtm=NULL;

		if(currentClump)
		{
			// get LTM from parent clump (it's usually vehicle main chasis):
			atomicLtm = RwFrameGetLTM(RpClumpGetFrame(currentClump));
		}
		else
		{
			// this atomic is separated part (like separated doors, bonnet, etc.)
            atomicLtm = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
		}
        ASSERT(atomicLtm);

		const RwV3d MapPos = *RwMatrixGetPos(atomicLtm);
		float CoordU = -CALC_ENVMAP_OFFSETX(MapPos.x);
		float CoordV = -CALC_ENVMAP_OFFSETY(MapPos.y);

		ASSERT(CoordU <= 1.0f);
		ASSERT(CoordU >= -1.0f);
		ASSERT(CoordV <= 1.0f);
		ASSERT(CoordV >= -1.0f);

		outVec->x = CoordU;
		outVec->y = CoordV;

		return(TRUE);
}


//
//
//
//
static
bool8 _rpMatFXSyncEnvMap2Data(RpAtomic *pAtomic, CustomEnvMapPipeMaterialData *pEnvData, 
								CustomEnvMapPipeAtomicData *pEnvData2, RwV2d *outVec)
{
	ASSERT(pAtomic);
	ASSERT(outVec);
	ASSERT(pEnvData2);

RpClump *currentClump = RpAtomicGetClump(pAtomic);
RwMatrix *atomicLTM=NULL;

		if(currentClump)
		{
			// get LTM from parent clump (it's usually vehicle main chasis):
			atomicLTM = RwFrameGetLTM(RpClumpGetFrame(currentClump));
		}
		else
		{
			// this atomic is separated part (like separated doors, bonnet, etc.)
            atomicLTM = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));
		}
        ASSERT(atomicLTM);


//
// calculate env offset with use of actual car pos in the world sector:
//
const float WIDTH_OF_SECTOR = WORLD_WIDTHOFSECTOR * pEnvData->GetfEnvTransSclX();
const float DEPTH_OF_SECTOR = WORLD_DEPTHOFSECTOR * pEnvData->GetfEnvTransSclY();


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef FINAL
bool8 gbDisplayEnvMap2DbgInfo=FALSE;

if(gbDisplayEnvMap2DbgInfo)
{
static uint16 lastRenderFrame = 0;

	const uint16 currRenderFrame = uint16(RWSRCGLOBAL(renderFrame));
	if(lastRenderFrame!=currRenderFrame)
	{
		lastRenderFrame = currRenderFrame;

		const float mapPosX = /*pMatFXEnvMatrix*/atomicLTM->pos.x;
		const float mapPosY = /*pMatFXEnvMatrix*/atomicLTM->pos.y;
		const int32 SectorX	= CALC_ENVMAP_SECTORX(mapPosX);
		const int32 SectorY = CALC_ENVMAP_SECTORY(mapPosY);	

		float OffsetU;
		float OffsetV;
	
		if(SectorX & 0x01)
		{
			OffsetU	= CMaths::Abs(CALC_ENVMAP_OFFSETX2(mapPosX));
		}
		else
		{
			OffsetU	= 1.0f - CMaths::Abs(CALC_ENVMAP_OFFSETX2(mapPosX));
		}
		
		if(SectorY & 0x01)
		{
			OffsetV	= CMaths::Abs(CALC_ENVMAP_OFFSETY2(mapPosY));
		}
		else
		{
			OffsetV	= 1.0f - CMaths::Abs(CALC_ENVMAP_OFFSETY2(mapPosY));
		}
		
		char txt[256];
		const int16 START_X = 4, START_Y = 4;
		::sprintf(txt, "EnvMap2::Info:  Sector Width: %.2f, Sector Height: %.2f", WIDTH_OF_SECTOR, DEPTH_OF_SECTOR);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+0);
		
		::sprintf(txt, "pos=[%.4f, %.4f]", mapPosX, mapPosY);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+1);

		::sprintf(txt, "OffsetUV=[%.4f, %.4f]", OffsetU, OffsetV);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+2);

		::sprintf(txt, "sectorXY=[%d, %d]",SectorX, SectorY);
		CDebug::PrintToScreenCoors(txt, START_X, START_Y+3);
	}
}
#endif//MASTER..
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


		const RwV3d MapPos = *RwMatrixGetPos(atomicLTM);

		//
		// it will work fine with assumption that ALL materials on given atomic will share EnvTransSclX/Y params(!)
		//
		float finalU = pEnvData2->fOffsetU;

		const uint16 currRenderFrame = uint16(RWSRCGLOBAL(renderFrame));
		
		if(pEnvData->nCurrentRenderFrame != currRenderFrame)	
		{
			pEnvData->nCurrentRenderFrame = currRenderFrame;

			const float mapPosX = MapPos.x;	//pMatFXEnvMatrix->pos.x;
			const float mapPosY = MapPos.y;	//pMatFXEnvMatrix->pos.y;
			const int32 SectorX	= CALC_ENVMAP_SECTORX(mapPosX);
			const int32 SectorY = CALC_ENVMAP_SECTORY(mapPosY);	
			//const float OffsetU	= CMaths::Abs(CALC_ENVMAP_OFFSETX2(mapPosX));
			//const float OffsetV	= CMaths::Abs(CALC_ENVMAP_OFFSETY2(mapPosY));
//
// calculate OffsetU & Offset V in "mirrored" way
// (so they never "overflow" (e.g.: 0.0->0.9 or 0.9->0.0, etc.) -
//  - this helps to avoid "jumps" in envmap2 scrolling):
// 
// n,m - actual world sector:
//  U V
// (0,0)----(1,0)-----(0,0)
//   |        |         |    
//   | (n,m)  | (n+1,m) |
//   |        |         |
// (0,1)----(1,1)-----(0,1)
//   |        |         |
//   |(n,m+1) |(n+1,m+1)|
//   |        |         |
// (0,0)----(1,0)-----(0,0)
//
			float OffsetU, OffsetV;
			if(SectorX & 0x01)
				OffsetU	= CMaths::Abs(CALC_ENVMAP_OFFSETX2(mapPosX));
			else
				OffsetU	= 1.0f - CMaths::Abs(CALC_ENVMAP_OFFSETX2(mapPosX));
			
			if(SectorY & 0x01)
				OffsetV	= CMaths::Abs(CALC_ENVMAP_OFFSETY2(mapPosY));
			else
				OffsetV	= 1.0f - CMaths::Abs(CALC_ENVMAP_OFFSETY2(mapPosY));

			// this is actual function for calculating U offset:
			const float currUVPos	= (OffsetU + OffsetV);


			const float prevMapPosX	= pEnvData2->fPrevMapPosX;
			const float prevMapPosY = pEnvData2->fPrevMapPosY;
			const int32 prevSectorX	= CALC_ENVMAP_SECTORX(prevMapPosX);
			const int32 prevSectorY = CALC_ENVMAP_SECTORY(prevMapPosY);	
			//const float prevOffsetU = CMaths::Abs(CALC_ENVMAP_OFFSETX2(prevMapPosX));
			//const float prevOffsetV = CMaths::Abs(CALC_ENVMAP_OFFSETY2(prevMapPosY));
			float  prevOffsetU,  prevOffsetV;
			if(prevSectorX & 0x01)
				prevOffsetU	= CMaths::Abs(CALC_ENVMAP_OFFSETX2(prevMapPosX));
			else
				prevOffsetU	= 1.0f - CMaths::Abs(CALC_ENVMAP_OFFSETX2(prevMapPosX));
			
			if(prevSectorY & 0x01)
				prevOffsetV	= CMaths::Abs(CALC_ENVMAP_OFFSETY2(prevMapPosY));
			else
				prevOffsetV	= 1.0f - CMaths::Abs(CALC_ENVMAP_OFFSETY2(prevMapPosY));
			// previous value of above function:
			const float prevUVPos	= (prevOffsetU + prevOffsetV);



			//
			// detect if car is moving forwards or backwards on the map:
			//
			bool8 bNegateOffset = FALSE;

			// current direction vector on the map:
			CVector vecCurrMapDir(mapPosX - prevMapPosX, mapPosY - prevMapPosY, 0);
			if(vecCurrMapDir.MagnitudeSqr() > 0.0f)	// detect movement
			{
				CVector vecForward(/*pMatFXEnvMatrix*/atomicLTM->up);	// this is how Forward vector is mapped in RwMatrix
				vecCurrMapDir.Normalise();
				vecForward.Normalise();

				const float dotP = DotProduct(vecCurrMapDir, vecForward);
				if(dotP < 0.0f)
				{
					bNegateOffset = TRUE;
				}
			}

			//
			// calculate U offset:
			//
			if(bNegateOffset)
			{
				finalU -= CMaths::Abs(prevUVPos - currUVPos);

				// texture wrapping:
				if(finalU < 0.0f)
					finalU += 1.0f;
			}
			else
			{
				finalU += CMaths::Abs(prevUVPos - currUVPos);

				// texture wrapping:
				if(finalU >= 1.0f)
					finalU -= 1.0f;
			}

			pEnvData2->fOffsetU		= finalU;
			pEnvData2->fPrevMapPosX	= mapPosX;
			pEnvData2->fPrevMapPosY	= mapPosY;
		}


		//
		// calculate V offset:
		//
		// get Up vector
		CVector vecUp(/*pMatFXEnvMatrix*/atomicLTM->at);				// this is how Up vector is mapped in RwMatrix
		// no need to normalise as pMatFXEnvMatrix is orthogonal
		//vecUp.Normalise();
#if 0
		const float finalV = (vecUp.x + vecUp.y);		// up-down tilts & scrolling depends on Up vector
#else
		float finalV = (vecUp.x + vecUp.y);				// up-down tilts & scrolling depends on Up vector	

		// limit scrollV  to <lowVal; topVal> (helps avoid topping-up texture, when vehicle moving uphill):
		const float lowVal = 0.0f;
		const float topVal = 0.1f;
		finalV = CMaths::Min(finalV, topVal);
		finalV = CMaths::Max(finalV, lowVal);
		
		// flip V when car is flipped upside down:
		if(vecUp.z < 0.0f)
		{
			finalV = 1.0f-finalV;
		}
#endif


		//
		// save results:
		//
		outVec->x = -finalU;
		outVec->y = finalV;

		return(TRUE);
}// end of _rpMatFXSyncEnvMap2Data()...





//
//
//
//
static
void dbgCheckMaterialRenderPassBits(uint32 matPassBitsMask, RpMaterial *pMaterial, RpAtomic *pAtomic)
{
#ifndef FINALBUILD
		// check if passmask is valid:
		switch(matPassBitsMask)
		{
			case(CARFX_NOFX_PASS_BIT):
			case(CARFX_ENVMAP1_PASS_BIT):							// 001 - envmap1 only 	(1)
			case(CARFX_ENVMAP2_PASS_BIT):							// 010 - envmap2 only 	(2)
			case(CARFX_SPECMAP_PASS_BIT):							// 100 - specular only	(4)
			case(CARFX_ENVMAP1_PASS_BIT+CARFX_SPECMAP_PASS_BIT):	// 101 - spec + envmap1 (5)
			case(CARFX_ENVMAP2_PASS_BIT+CARFX_SPECMAP_PASS_BIT):	// 110 - spec + envmap2 (6)
				break;
			default:
				ASSERTMSG(FALSE, "CVCarFX Pipe: invalid PassBitMask configuration found!");
				break;
		}

		// check presence of second set of UVs:
		switch(matPassBitsMask)
		{
			case(CARFX_ENVMAP2_PASS_BIT):							// 010 - envmap2 only 	(2)
			case(CARFX_ENVMAP2_PASS_BIT+CARFX_SPECMAP_PASS_BIT):	// 110 - spec + envmap2 (6)
			{
				RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
				ASSERT(pGeometry);
				const uint32 geomFlags = RpGeometryGetFlags(pGeometry);
				if((geomFlags & rpGEOMETRYTEXTURED2)==0)
				{
					char	*modelName	= NULL;
					RpClump *pClump 	= RpAtomicGetClump(pAtomic);
					if(pClump)
					{
						CClumpModelInfo *pModelInfo = CVisibilityPlugins::GetClumpModelInfo(pClump);
						ASSERT(pModelInfo);
						modelName = (char*)pModelInfo->GetModelName();
					}
					
					DEBUGLOG("CVCarFX pipe: Material uses EnvMap2, but geometry doesn't have second set of UVs set.");
					DEBUGLOG("Vehicle was not renderered.\n");
					if(modelName)
					{
						DEBUGLOG1("CVCarFX pipe: Asserted model name: '%s'.\n", modelName);
					}
					
					DEBUGLOG2("CVCarFX pipe: Geometry: %d verts, %d tris.\n", pGeometry->numVertices, pGeometry->numTriangles);

					if(pMaterial->texture)
					if(pMaterial->texture->name)
					{
							DEBUGLOG1("CVCarFX pipe: Asserted material texture: '%s'.\n", pMaterial->texture->name);
					}

					DEBUGLOG("\n\n\n\n");
//					ASSERTMSG(FALSE, "CVCarFX pipe: Material uses EnvMap2, but geometry doesn't have second set of UVs set.");
					return;
				}
			}
			break;
			default:
				break;
		}
#endif//FINALBUILD...
}


//
//
//
//
void CCustomCarEnvMapPipeline::PreRenderUpdate()
{
	
#if defined(GTA_PC)
	// update specular light:
	{
		//
		// specular parameters:
		//
	D3DLIGHT9 *pLight = &m_gSpecularLight;
		ZeroMemory(pLight, sizeof(D3DLIGHT9));

		//RwV3d vecDir= {0.5f, 0.0f, -0.5f};
		//RwV3d vecDir= {0.0f, 0.0f, 0.5f};
#if 0
		RwFrame *pCamFrame = RwCameraGetFrame(RwCameraGetCurrentCamera());
		ASSERT(pCamFrame);
		RwMatrix *pCamMatrix = RwFrameGetMatrix(pCamFrame);
		ASSERT(pCamMatrix);
		CVector vecDir = *RwMatrixGetAt(pCamMatrix);
#else
		extern RpLight *pDirect;	// global directional ingame light
		RwFrame *pSpecLightFrame = RpLightGetFrame(pDirect);
		CVector vecDir = *RwMatrixGetAt(&pSpecLightFrame->modelling);
#endif

		vecDir.Normalise();
		pLight->Direction.x = vecDir.x;
		pLight->Direction.y = vecDir.y;
		pLight->Direction.z = vecDir.z;
		
		pLight->Type			= D3DLIGHT_DIRECTIONAL;

#ifdef FINALBUILD
	#define LIGHTMEMDEF		const
#else
	#define LIGHTMEMDEF		static
#endif

LIGHTMEMDEF float AMBIENT_VAL	= 0.75f;
LIGHTMEMDEF float DIFFUSE_VAL	= 0.25f;
LIGHTMEMDEF float SPECULAR_VAL	= 0.65f;
#undef LIGHTMEMDEF

		pLight->Diffuse.r	= DIFFUSE_VAL;
		pLight->Diffuse.g	= DIFFUSE_VAL;
		pLight->Diffuse.b	= DIFFUSE_VAL;
		pLight->Diffuse.a	= 1.0f;
		pLight->Ambient.r	= AMBIENT_VAL;
		pLight->Ambient.g	= AMBIENT_VAL;
		pLight->Ambient.b	= AMBIENT_VAL;
		pLight->Ambient.a	= 1.0f;
		pLight->Specular.r	= SPECULAR_VAL;
		pLight->Specular.g	= SPECULAR_VAL;
		pLight->Specular.b	= SPECULAR_VAL;
		pLight->Specular.a	= 1.0f;

		pLight->Range			= 1000;
		pLight->Falloff			= 0;
		pLight->Attenuation0	= 1;
		pLight->Attenuation1	= 0;
		pLight->Attenuation2	= 0;
	}
#endif

}// end of PreRenderUpdate()...



#if defined(GTA_PC)
//
//
//
//
void CCustomCarEnvMapPipeline::CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags)
{

RxD3D9ResEntryHeader    *resEntryHeader;
RxD3D9InstanceData      *instancedData;
RpMatFXMaterialFlags    effectType;
RwBool                  bLightingEnabled=FALSE;
D3DLIGHT9				oldLight1;
RwBool                  forceBlack=FALSE;
float					materialSpecular	= 0.0f;
float					materialSpecPower	= 0.0f;
const float				envMapLightMult		= CCustomCarEnvMapPipeline::m_EnvMapLightingMult * 1.85f;



	RpAtomic *pAtomic = (RpAtomic*)object;
	ASSERT(pAtomic);
	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);
	const uint32 geomFlags = RpGeometryGetFlags(pGeometry);

	// detect whether this vehicle should not have extra passes drawn:
	const bool  bNoExtraPasses		= bool(CVisibilityPlugins::GetAtomicId(pAtomic) & (VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES_LOD|VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES));
	const bool	bNoExtraPassesDmg	= bool(CVisibilityPlugins::GetAtomicId(pAtomic) & (VEHICLE_ATOMIC_PIPE_NO_EXTRA_PASSES));

	// Set clipping:
    _rwD3D9EnableClippingIfNeeded(object, type);

    // Get lighting state:
    RwD3D9GetRenderState(D3DRS_LIGHTING, &bLightingEnabled);


	bool bRenderBlownUpAtomic = FALSE;

extern RpLight *pDirect;		// defined in lights.cpp
	// note: rpLIGHTLIGHTATOMICS flag is cleared for scorched vehicles
	const uint32 flagsLightDirect = RpLightGetFlags(pDirect);
	if((!(flagsLightDirect&rpLIGHTLIGHTATOMICS)) && bNoExtraPassesDmg)	
	{
		bRenderBlownUpAtomic = TRUE;
	}
	else
	{
		bRenderBlownUpAtomic = FALSE;
	}


	
	if(bLightingEnabled || ((flags & rxGEOMETRY_PRELIT) != 0))
    {
        forceBlack = FALSE;
    }
    else
    {
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
    if (resEntryHeader->indexBuffer != NULL)
    {
        RwD3D9SetIndices(resEntryHeader->indexBuffer);
    }

	// Set the stream source
    _rwD3D9SetStreams(resEntryHeader->vertexStream, resEntryHeader->useOffsets);

	// Vertex Declaration
    RwD3D9SetVertexDeclaration(resEntryHeader->vertexDeclaration);



    const int32 numMeshes = resEntryHeader->numMeshes;
    for(int32 i=0; i<numMeshes; i++)
    {
		RpMaterial *pMaterial = instancedData->material;
		ASSERT(pMaterial);
		CustomEnvMapPipeMaterialData *pEnvMatData = GETENVMAPPIPEDATAPTR(pMaterial);
		ASSERT(pEnvMatData);

		const uint32 matPassMask = CARFX_GET_PASS_CFG(pMaterial);

const bool bUseEnvMap1		= (matPassMask&CARFX_ENVMAP1_PASS_BIT) && (!bNoExtraPasses);
//const bool bUseEnvMap2		= (matPassMask&CARFX_ENVMAP2_PASS_BIT) && (!bNoExtraPasses);
const bool bUseEnvMap2		= (matPassMask&CARFX_ENVMAP2_PASS_BIT) && (!bNoExtraPasses) && bool(geomFlags&rpGEOMETRYTEXTURED2);


bool bUseSpecular = FALSE;

	if(g_fx.GetFxQuality() >= FX_QUALITY_HIGH)
	{
		// in high visual quality we never switch off specular stuff for vehicles with small LOD:
		bUseSpecular = (matPassMask&CARFX_SPECMAP_PASS_BIT);
	}
	else
	{
		bUseSpecular = (matPassMask&CARFX_SPECMAP_PASS_BIT) && (!bNoExtraPasses);
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
	// debug: check matPassMask configuration:
//	dbgCheckMaterialRenderPassBits(matPassMask, pMaterial, pAtomic);
#endif


	if(bUseSpecular && bLightingEnabled && (!bRenderBlownUpAtomic))
	{
		CustomSpecMapPipeMaterialData *pSpecMatData = GETSPECMAPPIPEDATAPTR(pMaterial);
		ASSERT(pSpecMatData);

		// this doesn't work good on nVidia cards:
		//RwD3D9GetLight(CARFX_D3D9_SPEC_LIGHT_INDEX, &oldLight1);
	
		D3DLIGHT9 *pLight = &m_gSpecularLight;
		RwD3D9SetLight(CARFX_D3D9_SPEC_LIGHT_INDEX,			pLight);
		RwD3D9EnableLight(CARFX_D3D9_SPEC_LIGHT_INDEX,		TRUE);

		// specular specific settings:
		materialSpecular	= 2.0f	* pSpecMatData->fSpecularity * envMapLightMult;		//0.7f;
		if(materialSpecular > 1.0f) materialSpecular = 1.0f; 
		materialSpecPower	= 100.0f * pSpecMatData->fSpecularity;	//20.0f//80.0f;
		RwD3D9SetRenderState(D3DRS_SPECULARENABLE,			TRUE);
		RwD3D9SetRenderState(D3DRS_LOCALVIEWER,				FALSE);				// simplified specular version
		RwD3D9SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);	// Use the color from the current material. 
//		RwD3D9SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1);		// Use the diffuse vertex color. 
//		RwD3D9SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);		// Use the specular vertex color. 
	}// if(bUseSpecular && lighting)...
	else
	{
		RwD3D9SetRenderState(D3DRS_SPECULARENABLE,			FALSE);
		materialSpecular	= 0.0f;
		materialSpecPower	= 0.0f;
	}




	RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);

	// EnvMap1:
	if(bUseEnvMap1 && (!bRenderBlownUpAtomic))
	{
		RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSWRAP);

		static D3DMATRIX envTexMatrix={0};

		envTexMatrix.m[0][0] = pEnvMatData->GetfEnvScaleX();
		envTexMatrix.m[1][1] = pEnvMatData->GetfEnvScaleY();
		envTexMatrix.m[2][2] = envTexMatrix.m[3][3] = 1.0f;

		RwV2d envOffset={0};
		_rpMatFXSyncEnvMap1Data(pAtomic, pEnvMatData, &envOffset);

		envTexMatrix.m[2][0] = envOffset.x;
		envTexMatrix.m[2][1] = envOffset.y;
		RwD3D9SetTransform(D3DTS_TEXTURE1, (void*)&envTexMatrix);

		RwTexture *pEnvTexture = pEnvMatData->pEnvTexture;
		ASSERT(pEnvTexture);
		RwD3D9SetTexture(pEnvTexture, 1);

		int32 shin = pEnvMatData->GetfShininess() * 254.0f * envMapLightMult;		//32;
		if(shin > 255) shin=255;
		const uint8 nShininess = uint8(shin);

#if 0
		RwD3D9SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(nShininess,0xFF,0xFF,0xFF));
		RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_BLENDFACTORALPHA);
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );	// texture for this blending stage
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );	// result from previous blending stage
#else
		RwD3D9SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(0xFF,nShininess,nShininess,nShininess));
		RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MULTIPLYADD);// out = arg1 + arg2*arg3;
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG0, D3DTA_CURRENT );	// result from previous blending stage
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);		// texture for this blending stage
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
#endif
		RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG2);
		RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE );
		RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG2,	D3DTA_CURRENT );
		RwD3D9SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,			D3DTSS_TCI_CAMERASPACENORMAL | 1); // env mapping + wrapping mode of TEXTURE1
		RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,	/*D3DTTFF_COUNT2*/(D3DTTFF_COUNT3|D3DTTFF_PROJECTED)); // projected avoids translation "jumps"
		
		RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);	// no next pass
	}// if(bUseEnvMap1)...


	// EnvMap2:
	if(bUseEnvMap2 && (!bRenderBlownUpAtomic))
	{
		RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSWRAP);

		RwV2d envOffset={0};

		CustomEnvMapPipeAtomicData* pEnvAtomicData = CCustomCarEnvMapPipeline::AllocEnvMapPipeAtomicData(pAtomic);
		if(!pEnvAtomicData)
		{
			ASSERT(FALSE);
			// rare case when pool runs out of memory:
			//bUseEnvMapRoutine2 = FALSE;
			//passBitMask &= ~(CVCFX_ENVMAP2_PASS_BIT);	// switch off this pass
		}
		else
		{
			_rpMatFXSyncEnvMap2Data(pAtomic, pEnvMatData, pEnvAtomicData, &envOffset);
		}

		static D3DMATRIX envTexMatrix={0};
		envTexMatrix.m[0][0] = envTexMatrix.m[1][1] = 
		envTexMatrix.m[2][2] = envTexMatrix.m[3][3] = 1.0f;
		envTexMatrix.m[2][0] = envOffset.x;
		envTexMatrix.m[2][1] = envOffset.y;
		RwD3D9SetTransform(D3DTS_TEXTURE1, (void*)&envTexMatrix);

		RwTexture *pEnvTexture = pEnvMatData->pEnvTexture;
		ASSERT(pEnvTexture);
		RwD3D9SetTexture(pEnvTexture, 1);

		// this is constant value for Shininess wanted by Jolyon:
		int32 shin = 24 * envMapLightMult;		//uint8(pEnvMatData->GetfShininess()* 254.0f * envMapLightMult);	//64;
		if(shin > 255) shin = 255;
		const uint8 nShininess = uint8(shin);

#if 1
		RwD3D9SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(nShininess,0xFF,0xFF,0xFF));
		RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_BLENDFACTORALPHA);
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );	// texture for this blending stage
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );	// result from previous blending stage
#else
		RwD3D9SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(0xFF,nShininess,nShininess,nShininess));
		RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MULTIPLYADD);// out = arg1 + arg2*arg3;
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG0, D3DTA_CURRENT );	// result from previous blending stage
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);		// texture for this blending stage
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
#endif

		RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG2);
		RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE );
		RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG2,	D3DTA_CURRENT );
		RwD3D9SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,			1); // env map2 coords
		RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,	D3DTTFF_COUNT2 /*(D3DTTFF_COUNT3|D3DTTFF_PROJECTED)*/); // projected avoids translation "jumps"

		RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);	// no next pass
	}// if(bUseEnvMap2)...


////////////////////////////////////////////////////////////////////////////////////////////////////////



		if( (instancedData->vertexAlpha)					||
            (0xFF != instancedData->material->color.alpha)	)
        {
            _rwD3D9RenderStateVertexAlphaEnable(TRUE);
        }
        else
        {
            _rwD3D9RenderStateVertexAlphaEnable(FALSE);
        }

        if (!forceBlack)
        {
            if(bLightingEnabled)
            {
				RwRGBA color = {instancedData->material->color.red,
								instancedData->material->color.green,
								instancedData->material->color.blue,
								instancedData->material->color.alpha};

#if defined(GTA_PC)
				// GTA_PC's Bug #760 fix: detect and clear all default vehicle re-map colors:
				const uint32 matCol = (*((uint32*)(&color))) & 0x00FFFFFF;
				switch(matCol)
				{
					case(MAT1_COLOUR):
					case(MAT2_COLOUR):
					case(MAT3_COLOUR):
					case(MAT4_COLOUR):
					case(LIGHT_FRONT_LEFT_COLOUR):
					case(LIGHT_FRONT_RIGHT_COLOUR):
					case(LIGHT_REAR_LEFT_COLOUR):
					case(LIGHT_REAR_RIGHT_COLOUR):
					{
						color.red =	color.green = color.blue =	0;
					}
					break;
					
					default:
							break;
				}
#endif


#if 0
				if(bRenderBlownUpAtomic)
				{
					// set dark color on blown-up vehicle:
					//const float darkColorScale = 0.5f;
					//color.red		= uint8(float(color.red)	* darkColorScale);
					//color.green	= uint8(float(color.green)	* darkColorScale);
					//color.blue	= uint8(float(color.blue)	* darkColorScale);
				
					const uint16 darkColorScale = 128;		// =0.25
					color.red		= uint8((uint16(color.red)	* darkColorScale)>>8);
					color.green		= uint8((uint16(color.green)* darkColorScale)>>8);
					color.blue		= uint8((uint16(color.blue)	* darkColorScale)>>8);
				}
#endif //#if 0...


                CustomD3D9Pipeline_RwSetSurfaceProperties(&(instancedData->material->surfaceProps),
														/*&(instancedData->material->color)*/&color,
														flags, materialSpecular, materialSpecPower);
			}
		    
			CustomD3D9Pipeline_AtomicDefaultRender(resEntryHeader, instancedData, flags, instancedData->material->texture);
		}
        else
        {
            CustomD3D9Pipeline_AtomicRenderBlack(resEntryHeader, instancedData);
        }


		if(bUseSpecular && bLightingEnabled)
		{
			RwD3D9SetRenderState(D3DRS_SPECULARENABLE,		FALSE);
			RwD3D9EnableLight(CARFX_D3D9_SPEC_LIGHT_INDEX,	FALSE);
			// this doesn't work nice on nVidia cards:
			//RwD3D9SetLight(CARFX_D3D9_SPEC_LIGHT_INDEX,		&oldLight1);	
		}

		instancedData++;
    }// for(int32 i=0; i<numMeshes; i++)...



	// synchronize RW's internal material cache (after CustomD3D9Pipeline_RwSetSurfaceProperties()):
	CustomD3D9Pipeline_RwSetSurfacePropertiesCacheResync();


	// best to turn off the second pass effect:
	RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,				D3DTOP_DISABLE);
	RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,				D3DTOP_DISABLE);
	RwD3D9SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,			1);
	RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	//RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,				D3DTOP_DISABLE);
	//RwD3D9SetTextureStageState(2, D3DTSS_ALPHAOP,				D3DTOP_DISABLE);
	//RwD3D9SetTextureStageState(2, D3DTSS_TEXCOORDINDEX,		2);
	//RwD3D9SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

}// end of CustomPipeRenderCB()...

#endif


#if defined(GTA_PC)
//
//
//
//
RxPipeline*	CCustomCarEnvMapPipeline::CreateCustomD3D9ObjPipe()
{
	RxPipeline  *pipe = RxPipelineCreate();
    if(pipe)
    {
        RxLockedPipe    *lpipe = RxPipelineLock(pipe);
        if(lpipe)
        {
            // Get the instance node definition
            RxNodeDefinition *instanceNode = RxNodeDefinitionGetD3D9AtomicAllInOne();

			// Add the node to the pipeline
            lpipe = RxLockedPipeAddFragment(lpipe, NULL, instanceNode, NULL);

            // Unlock the pipeline
            lpipe = RxLockedPipeUnlock(lpipe);
			if(lpipe)
			{
				RxPipelineNode		*allinone = RxPipelineFindNodeByName(pipe, instanceNode->name, NULL, 0);
				ASSERT(allinone);

				// use standard instancing to avoid problems with onchip accelerators (Intel, S3, etc.)
				// this may be connected with the fact, they're sharing system memory
				RxD3D9AllInOneSetInstanceCallBack(	allinone,	/*CustomPipeInstanceCB*/RxD3D9AllInOneGetInstanceCallBack(allinone));
				RxD3D9AllInOneSetReinstanceCallBack(allinone,	CustomPipeReinstanceCB);	// RxD3D9AllInOneGetReinstanceCallBack(allinone));
				RxD3D9AllInOneSetRenderCallBack(	allinone,	CustomPipeRenderCB);		// RxD3D9AllInOneGetRenderCallBack(allinone));

				pipe->pluginId		= PDSPC_CarFXPipe_AtmID;
				pipe->pluginData	= PDSPC_CarFXPipe_AtmID;
				return(pipe); // success
			}
        }
        RxPipelineDestroy(pipe);
    }

	return(NULL);	// failure
}



//
//
//
//
RwBool CCustomCarEnvMapPipeline::CreatePipe()
{
	ObjPipeline = CreateCustomD3D9ObjPipe();
	if(!ObjPipeline)
	{
		return(FALSE);
	}

	if(!CCustomCarEnvMapPipeline::CheckDeviceCaps())
	{
		ASSERTMSG(FALSE, "CustomCarFXPipeline: Not all crucial d3d9 device caps supported!");
		return(FALSE);
	}

	ZeroMemory(&m_gSpecularLight, sizeof(D3DLIGHT9));

	//
	// initialise custom EnvMapPipeMatData pool:
	//
	m_gEnvMapPipeMatDataPool = new CustomEnvMapPipeMaterialDataPool(CARFX_ENVMAPDATAPOOL_SIZE, "CustomEnvMapPipeMatDataPool");
	ASSERT(m_gEnvMapPipeMatDataPool);

	m_gEnvMapPipeAtmDataPool = new CustomEnvMapPipeAtomicDataPool(CARFX_ENVMAPATMDATAPOOL_SIZE, "CustomEnvMapPipeAtmDataPool");
	ASSERT(m_gEnvMapPipeAtmDataPool);

	m_gSpecMapPipeMatDataPool = new CustomSpecMapPipeMaterialDataPool(CARFX_SPECMAPDATAPOOL_SIZE, "CustomSpecMapPipeMaterialDataPool");
	ASSERT(m_gSpecMapPipeMatDataPool);

#ifdef USE_POOL_COUNTER
	// local counter to see how much pool is filled during execution:
	nEnvMapPipeMatDataPoolCounter	=0;
	nEnvMapPipeAtmDataPoolCounter	=0;
	nSpecMapPipeMatDataPoolCounter	=0;
#endif

	return(TRUE);
}// end of CCustomCarEnvMapPipeline::CreatePipe()...

//
//
//
//
RwBool CCustomCarEnvMapPipeline::CheckDeviceCaps()
{

const D3DCAPS9 *d3d9Caps = (const D3DCAPS9*)RwD3D9GetCaps();
	ASSERT(d3d9Caps);


int32	VideoCardMaxTextureBlendStages		= d3d9Caps->MaxTextureBlendStages;
bool	VideoCardSupportsMultitexture		= FALSE;
bool	VideoCardSupportsBlendFactorAlpha	= FALSE;
bool	VideoCardSupportsMultiplyAdd		= FALSE;
bool	VideoCardSupportsSpecular			= FALSE;

	if (VideoCardMaxTextureBlendStages > 1)
    {
        VideoCardSupportsMultitexture		= (d3d9Caps->MaxSimultaneousTextures >= 2);
		//VideoCardSupportsModulate2X		= (d3d9Caps->TextureOpCaps & D3DTEXOPCAPS_MODULATE2X);
        VideoCardSupportsMultiplyAdd		= (d3d9Caps->TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD);
		VideoCardSupportsBlendFactorAlpha	= (d3d9Caps->TextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA);
	}
	
	VideoCardSupportsSpecular				= (d3d9Caps->ShadeCaps & D3DPSHADECAPS_SPECULARGOURAUDRGB);


	if(!VideoCardSupportsMultitexture)
		return(FALSE);
	if(!VideoCardSupportsBlendFactorAlpha)
		return(FALSE);
	if(!VideoCardSupportsMultiplyAdd)
		return(FALSE);
	if(!VideoCardSupportsSpecular)
		return(FALSE);

	return(TRUE);
}


//
//
//
//
void CCustomCarEnvMapPipeline::DestroyPipe()
{
	delete m_gEnvMapPipeMatDataPool;
	m_gEnvMapPipeMatDataPool = NULL;

	delete m_gEnvMapPipeAtmDataPool;
	m_gEnvMapPipeAtmDataPool = NULL;

	delete m_gSpecMapPipeMatDataPool;
	m_gSpecMapPipeMatDataPool = NULL;


	if(ObjPipeline)
	{
		RxPipelineDestroy(ObjPipeline);
		ObjPipeline = NULL;
	}
}// end of CCustomCarEnvMapPipeline::DestroyPipe()...

#else

RwBool CCustomCarEnvMapPipeline::CreatePipe() {
	return (FALSE);
}

void CCustomCarEnvMapPipeline::DestroyPipe() {
}

#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PLUGINS PROPERTIES:
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
//
static
RwTexture* GetFXMatEnvTexture(RpMaterial *pMaterial)
{
	if(!pMaterial)
		return(NULL);
	
	rpMatFXMaterialData *pMaterialData = (rpMatFXMaterialData*)*MATFXMATERIALGETDATA(pMaterial);
	ASSERTMSG(pMaterialData, "pMaterialData invalid!");

	RwTexture *envTexture = pMaterialData->data[rpSECONDPASS].data.envMap.texture;
	return(envTexture);
}


//
//
//
//
void CCustomCarEnvMapPipeline::SetFxEnvTexture(RpMaterial *pMaterial, RwTexture *fxTexture)
{
	ASSERT(pMaterial);
	
	CustomEnvMapPipeMaterialData *pData = DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;
	 
	if(!fxTexture)
	{
		pData->pEnvTexture = GetFXMatEnvTexture(pMaterial);
		
		if((pData->pEnvTexture==NULL) || (pData->GetfShininess()==0.0f))
		{
			// *Special Case*:
			// it seems that CustomEnvMapFx will not be visible for this material anyway (envmap texture is NULL),
			// so release pool memory and replace material Data with default one:
			if(pData)
			{
				//GtaFree(pData);
				m_gEnvMapPipeMatDataPool->Delete(pData);
				#ifdef USE_POOL_COUNTER
					nEnvMapPipeMatDataPoolCounter--;
				#endif

				CustomEnvMapPipeMaterialData **ppData = GETENVMAPPIPEDATAPTRPTR(pMaterial);
				*ppData = &fakeEnvMapPipeMatData;
				pData = *ppData;
			}
		}
		
	}
	else
	{
		pData->pEnvTexture = fxTexture;
	}


	//
	// force proper wrap modes + filtering for envmap texture:
	//
	RwTexture *pTexture = pData->pEnvTexture;
	if(pTexture)
	{
		RwTextureSetAddressing(pTexture, rwTEXTUREADDRESSWRAP);
		RwTextureSetFilterMode(pTexture, rwFILTERLINEAR);
 	}

}


//
//
//
//
RwTexture* CCustomCarEnvMapPipeline::GetFxEnvTexture(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(NULL);


	return(pData->pEnvTexture);
}


//
//
//
//
void CCustomCarEnvMapPipeline::SetFxEnvScale(RpMaterial *pMaterial, float envScaleX, float envScaleY)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvScaleX(envScaleX);
	pData->SetfEnvScaleY(envScaleY);

}


//
//
//
//
float CCustomCarEnvMapPipeline::GetFxEnvScaleX(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvScaleX());
}

//
//
//
//
float CCustomCarEnvMapPipeline::GetFxEnvScaleY(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvScaleY());
}


//
//
//
//
void CCustomCarEnvMapPipeline::SetFxEnvTransScl(RpMaterial *pMaterial, float envTransX, float envTransY)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvTransSclX(envTransX);
	pData->SetfEnvTransSclY(envTransY);

}

float CCustomCarEnvMapPipeline::GetFxEnvTransSclX(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclX());
}

float CCustomCarEnvMapPipeline::GetFxEnvTransSclY(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclY());
}




//
//
//
//
void CCustomCarEnvMapPipeline::SetFxEnvShininess(RpMaterial *pMaterial, float envShininess)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;
	
	pData->SetfShininess(envShininess);
}

//
//
//
//
float CCustomCarEnvMapPipeline::GetFxEnvShininess(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfShininess());
}



#pragma mark -
//
//
//
//
void CCustomCarEnvMapPipeline::SetFxSpecTexture(RpMaterial *pMaterial, RwTexture *customTexture)
{
	ASSERT(pMaterial);
	
	CustomSpecMapPipeMaterialData *pData = GETSPECMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return;

	if(customTexture)
	{
		// remove old texture
		if(pData->pSpecularTexture)
		{
			RwTextureDestroy(pData->pSpecularTexture);
			pData->pSpecularTexture = NULL;
		}
		
		if(customTexture)
		{
			pData->pSpecularTexture = customTexture;
			RwTextureAddRef(pData->pSpecularTexture);
		}	
	}
	

 	//
	// force proper wrap modes + filtering for specmap texture:
	//
	RwTexture *pSpecTexture = pData->pSpecularTexture;
	if(pSpecTexture)
	{
		RwTextureSetAddressing(pSpecTexture, rwTEXTUREADDRESSWRAP);
		RwTextureSetFilterMode(pSpecTexture, rwFILTERLINEAR);
	}

}


//
//
//
//
RwTexture* CCustomCarEnvMapPipeline::GetFxSpecTexture(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);

	CustomSpecMapPipeMaterialData *pData = GETSPECMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(NULL);

	return(pData->pSpecularTexture);
}

//
//
//
//
void CCustomCarEnvMapPipeline::SetFxSpecSpecularity(RpMaterial *pMaterial, float specularity)
{
	ASSERT(pMaterial);

	CustomSpecMapPipeMaterialData *pData = GETSPECMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return;

	pData->fSpecularity = specularity;
}

//
//
//
//
float CCustomCarEnvMapPipeline::GetFxSpecSpecularity(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);

	CustomSpecMapPipeMaterialData *pData = GETSPECMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->fSpecularity);
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PLUGINS:
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
//
RwBool CCustomCarEnvMapPipeline::RegisterPlugin()
{
	//
	// EnvMap plugin:
	//
	ms_envMapPluginOffset = RpMaterialRegisterPlugin(sizeof(CustomEnvMapPipeMaterialData*),
												MAKECHUNKID(rwVENDORID_ROCKSTAR, rwCUSTENVMAPMAT_ID),
												pluginEnvMatConstructorCB,
												pluginEnvMatDestructorCB,
												pluginEnvMatCopyConstructorCB);

	if(ms_envMapPluginOffset < 0)
		return(FALSE);

	if(RpMaterialRegisterPluginStream(MAKECHUNKID(rwVENDORID_ROCKSTAR, rwCUSTENVMAPMAT_ID), 
           								pluginEnvMatStreamReadCB,
           								pluginEnvMatStreamWriteCB,
           								pluginEnvMatStreamGetSizeCB) < 0)
	{
    	ms_envMapPluginOffset = -1;
        return(FALSE);
	}

	SetCustomEnvMapPipeMaterialDataDefaults(&fakeEnvMapPipeMatData);



	//
	// EnvMap Atomic plugin:
	//
	ms_envMapAtmPluginOffset = RpAtomicRegisterPlugin(sizeof(CustomEnvMapPipeAtomicData*),
												MAKECHUNKID(rwVENDORID_ROCKSTAR, rwCUSTENVMAPATM_ID),
												pluginEnvAtmConstructorCB,
												pluginEnvAtmDestructorCB,
												pluginEnvAtmCopyConstructorCB);
	if(ms_envMapAtmPluginOffset < 0)
		return(FALSE);



	//
	// Specular plugin:
	//
	ms_specularMapPluginOffset = RpMaterialRegisterPlugin(	sizeof(CustomSpecMapPipeMaterialData*),
												MAKECHUNKID(rwVENDORID_ROCKSTAR, rwCUSTSPECMAPMAT_ID),
												pluginSpecMatConstructorCB,
												pluginSpecMatDestructorCB,
												pluginSpecMatCopyConstructorCB);
	if(ms_specularMapPluginOffset < 0)
		return(FALSE);

	if(RpMaterialRegisterPluginStream(MAKECHUNKID(rwVENDORID_ROCKSTAR, rwCUSTSPECMAPMAT_ID), 
										pluginSpecMatStreamReadCB,
										pluginSpecMatStreamWriteCB,
										pluginSpecMatStreamGetSizeCB) < 0)
	{
		ms_specularMapPluginOffset = -1;
		return(FALSE);
	}


	return(TRUE);
}



//
//
//
//
#pragma mark -
#pragma mark ------ CustomEnvMapMaterialPlugin stuff ------

//
//
//
//
void CCustomCarEnvMapPipeline::SetCustomEnvMapPipeMaterialDataDefaults(CustomEnvMapPipeMaterialData *pData)
{
	ASSERT(pData);

	pData->SetfEnvScaleX	(1.0f);
	pData->SetfEnvScaleY	(1.0f);
	pData->SetfEnvTransSclX	(1.0f);
	pData->SetfEnvTransSclY	(1.0f);
	pData->SetfShininess	(1.0f);
	
	pData->pEnvTexture		= NULL;

	pData->nCurrentRenderFrame = 0;
	//pData->SetRenderPassBits(0);
	
}

//
//
// check if ppData points alredy to fakeData; if yes, then duplicate the fake structure (default values anyway);
//
// this routine will be called always if we are changing anything from defaults in EnvMapMaterialData; 
//
CustomEnvMapPipeMaterialData* CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(CustomEnvMapPipeMaterialData **ppData)
{
	CustomEnvMapPipeMaterialData *pData = *ppData;
	
	if(pData == &fakeEnvMapPipeMatData)
	{
		//*ppData = (CustomEnvMapPipeMaterialData*)GtaMalloc(sizeof(CustomEnvMapPipeMaterialData));
		*ppData = m_gEnvMapPipeMatDataPool->New();
		if(!(*ppData))
		{
			return(NULL);
		}
		#ifdef USE_POOL_COUNTER
			nEnvMapPipeMatDataPoolCounter++;
		#endif
		
		pData = *ppData;
		::memcpy(pData, &fakeEnvMapPipeMatData, sizeof(CustomEnvMapPipeMaterialData));
	}
	
	return(pData);
} 


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginEnvMatConstructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{

	CustomEnvMapPipeMaterialData **ppData = (CustomEnvMapPipeMaterialData**)(((uint8*)object) + offsetInObject);
	*ppData = &fakeEnvMapPipeMatData;	//NULL;

	return(object);
}


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginEnvMatDestructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{

	CustomEnvMapPipeMaterialData **ppData	= (CustomEnvMapPipeMaterialData**)(((uint8*)object) + offsetInObject);
	CustomEnvMapPipeMaterialData *pData		= *ppData;
	
	if(pData == &fakeEnvMapPipeMatData)
	{
		// fake pointer detected - do nothing:
		return(object);
	}

		
	if(pData)
	{
		//GtaFree(pData);
		m_gEnvMapPipeMatDataPool->Delete(pData);
		#ifdef USE_POOL_COUNTER
			nEnvMapPipeMatDataPoolCounter--;
		#endif

		*ppData = NULL;
		pData = NULL;
	}
	
	return(object);
}


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginEnvMatCopyConstructorCB(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject)
{

	CustomEnvMapPipeMaterialData **ppSrcData = (CustomEnvMapPipeMaterialData**)(((uint8*)srcObject) + offsetInObject);
	CustomEnvMapPipeMaterialData **ppDstData = (CustomEnvMapPipeMaterialData**)(((uint8*)dstObject) + offsetInObject);

	CustomEnvMapPipeMaterialData *pSrcData = *ppSrcData;
	CustomEnvMapPipeMaterialData *pDstData = *ppDstData;

	// nothing to copy? - do nothing:
	if(!pSrcData)
		return(dstObject);

	
	if(!pDstData)
	{
		//*ppDstData = (CustomEnvMapPipeMaterialData*)GtaMalloc(sizeof(CustomEnvMapPipeMaterialData));
		*ppDstData = m_gEnvMapPipeMatDataPool->New();
		if(!(*ppDstData))
		{
			// rare case when pool run out of memory:
			*ppDstData = &fakeEnvMapPipeMatData;
			return(dstObject);
		}
		#ifdef USE_POOL_COUNTER
			nEnvMapPipeMatDataPoolCounter++;
		#endif

		pDstData = *ppDstData;
	}


	::memcpy(pDstData, pSrcData, sizeInObject);

	return(dstObject);
}



//
//
//
//
inline
void CopyCustomEnvMapPipeMaterialDataDiskToMem(CustomEnvMapPipeMaterialDiskData *pDiskData, CustomEnvMapPipeMaterialData *pMemData)
{
#define COPY_FIELD(FIELDNAME)			{ pMemData->FIELDNAME = pDiskData->FIELDNAME;	};
#define COPY_FIELD8(METHOD,FIELDNAME)	{ pMemData->METHOD(pDiskData->FIELDNAME); 		};
#define SET_FIELD(FIELDNAME, VAL)		{ pMemData->FIELDNAME = VAL; 					};

	COPY_FIELD8(SetfEnvScaleX,		fEnvScaleX);
	COPY_FIELD8(SetfEnvScaleY,		fEnvScaleY);
	COPY_FIELD8(SetfEnvTransSclX,	fEnvTransSclX);
	COPY_FIELD8(SetfEnvTransSclY,	fEnvTransSclY);
	COPY_FIELD8(SetfShininess,		fShininess);
	COPY_FIELD(pEnvTexture);

	SET_FIELD(nCurrentRenderFrame,	0);
	//SET_FIELD(nRenderPassBits,		0);
	//SET_FIELD(fOffsetU,				0.0f);
	//SET_FIELD(fPrevMapPosX,			0);
	//SET_FIELD(fPrevMapPosY,			0);

#undef COPY_FIELD
#undef COPY_FIELD8
#undef SET_FIELD
}

//
//
//
//
inline
void CopyCustomEnvMapPipeMaterialDataMemToDisk(CustomEnvMapPipeMaterialData *pMemData, CustomEnvMapPipeMaterialDiskData *pDiskData)
{
#define COPY_FIELD(FIELDNAME)			{ pDiskData->FIELDNAME = pMemData->FIELDNAME; 		};
#define COPY_FIELD8(FIELDNAME,METHOD)	{ pDiskData->FIELDNAME = pMemData->METHOD();		};

	COPY_FIELD8(fEnvScaleX,		GetfEnvScaleX);
	COPY_FIELD8(fEnvScaleY,		GetfEnvScaleY);
	COPY_FIELD8(fEnvTransSclX,	GetfEnvTransSclX);
	COPY_FIELD8(fEnvTransSclY,	GetfEnvTransSclY);
	COPY_FIELD8(fShininess,		GetfShininess);
	COPY_FIELD(pEnvTexture);

#undef COPY_FIELD
#undef COPY_FIELD8
}


//
//
//
// 
RwStream* CCustomCarEnvMapPipeline::pluginEnvMatStreamReadCB(RwStream *stream, RwInt32 binaryLength,
									void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	CustomEnvMapPipeMaterialDiskData tempDiskData;
 	RwStreamRead(stream, &tempDiskData, binaryLength);

	CustomEnvMapPipeMaterialData **ppData = (CustomEnvMapPipeMaterialData**)(((uint8*)object) + offsetInObject);
	

	CustomEnvMapPipeMaterialData tempData;
	CopyCustomEnvMapPipeMaterialDataDiskToMem(&tempDiskData, &tempData);


	if(IsCustomEnvMapPipeMaterialDataDefault(&tempData))
	{
		*ppData = &fakeEnvMapPipeMatData;
	}
	else
	{
		//*ppData = (CustomEnvMapPipeMaterialData*)GtaMalloc(sizeof(CustomEnvMapPipeMaterialData));
		*ppData = m_gEnvMapPipeMatDataPool->New();
		if(!(*ppData))
		{
			// rare case when pools run out of free memory:
			*ppData = &fakeEnvMapPipeMatData;
			return(stream);
		}
		#ifdef USE_POOL_COUNTER
			nEnvMapPipeMatDataPoolCounter++;
		#endif
		
		CustomEnvMapPipeMaterialData *pData = *ppData;
		::memcpy(pData, &tempData, sizeof(CustomEnvMapPipeMaterialData));
	}

 	return(stream);
}


//
//
//
//
RwStream* CCustomCarEnvMapPipeline::pluginEnvMatStreamWriteCB(RwStream *stream, RwInt32 binaryLength,
									const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	CustomEnvMapPipeMaterialData **ppData = (CustomEnvMapPipeMaterialData**)(((uint8*)object) + offsetInObject);
	CustomEnvMapPipeMaterialData *pData = *ppData;


CustomEnvMapPipeMaterialDiskData diskData;

	if(pData)
	{
		CopyCustomEnvMapPipeMaterialDataMemToDisk(pData, &diskData);
		RwStreamWrite(stream, &diskData, binaryLength);
 	}
 	else
 	{
		CustomEnvMapPipeMaterialData tempData;
		SetCustomEnvMapPipeMaterialDataDefaults(&tempData);
		CopyCustomEnvMapPipeMaterialDataMemToDisk(pData, &diskData);

		RwStreamWrite(stream, &diskData, binaryLength);
 	}
 	
 	return(stream);
}
	
//
//
//
//
RwInt32 CCustomCarEnvMapPipeline::pluginEnvMatStreamGetSizeCB(const void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)  
{
 	if(object)
 	{
  		return( sizeof(CustomEnvMapPipeMaterialDiskData) );
 	}
 
 	return(0);
}






#pragma mark -
#pragma mark ------ CustomEnvMapAtomicPlugin stuff ------


//
//
//
//
void CCustomCarEnvMapPipeline::SetCustomEnvMapPipeAtomicDataDefaults(CustomEnvMapPipeAtomicData *pData)
{
	pData->fOffsetU			= 0;
	pData->fPrevMapPosX		= 
	pData->fPrevMapPosY		= 0.0f;
}


//
//
//
//
CustomEnvMapPipeAtomicData*	CCustomCarEnvMapPipeline::AllocEnvMapPipeAtomicData(RpAtomic *pAtomic)
{
	CustomEnvMapPipeAtomicData **ppData = GETENVMAPPIPEATMDATAPTRPTR(pAtomic);
	CustomEnvMapPipeAtomicData *pData	= *ppData;

	if(!pData)
	{
		*ppData = m_gEnvMapPipeAtmDataPool->New();
		if(!(*ppData))
		{
			// rare case when pool runs out of memory:
			return(NULL);
		}
		#ifdef USE_POOL_COUNTER
			nEnvMapPipeAtmDataPoolCounter++;
		#endif
		pData = *ppData;
	
		CCustomCarEnvMapPipeline::SetCustomEnvMapPipeAtomicDataDefaults(pData);
	}

	return(pData);
}


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginEnvAtmConstructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{
	CustomEnvMapPipeAtomicData **ppData = (CustomEnvMapPipeAtomicData**)(((uint8*)object) + offsetInObject);
	*ppData = NULL;

	return(object);
}


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginEnvAtmDestructorCB(void *object, RwInt32 offsetInObject, RwInt32 sizeInObject)
{

	CustomEnvMapPipeAtomicData **ppData = (CustomEnvMapPipeAtomicData**)(((uint8*)object) + offsetInObject);
	CustomEnvMapPipeAtomicData *pData	= *ppData;
	
	if(pData)
	{
		m_gEnvMapPipeAtmDataPool->Delete(pData);
		#ifdef USE_POOL_COUNTER
			nEnvMapPipeAtmDataPoolCounter--;
		#endif

		*ppData = NULL;
		pData = NULL;
	}
	
	return(object);
}


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginEnvAtmCopyConstructorCB(void *dstObject, const void *srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject)
{

	CustomEnvMapPipeAtomicData **ppSrcData = (CustomEnvMapPipeAtomicData**)(((uint8*)srcObject) + offsetInObject);
	CustomEnvMapPipeAtomicData **ppDstData = (CustomEnvMapPipeAtomicData**)(((uint8*)dstObject) + offsetInObject);

	CustomEnvMapPipeAtomicData *pSrcData = *ppSrcData;
	CustomEnvMapPipeAtomicData *pDstData = *ppDstData;

	// nothing to copy? - do nothing:
	if(!pSrcData)
		return(dstObject);
	
	if(!pDstData)
	{
		*ppDstData = m_gEnvMapPipeAtmDataPool->New();
		if(!(*ppDstData))
		{
			// rare case when pool runs out of memory:
			return(dstObject);
		}
		#ifdef USE_POOL_COUNTER
			nEnvMapPipeAtmDataPoolCounter++;
		#endif
		pDstData = *ppDstData;
	}

	::memcpy(pDstData, pSrcData, sizeInObject);

	return(dstObject);
}


#pragma mark -
#pragma mark -- CustomSpecularMapMaterial plugin stuff --


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginSpecMatConstructorCB(void *object, int offsetInObject, int sizeInObject)
{
	CustomSpecMapPipeMaterialData **ppExtData = GETSPECMAPPIPEDATAPTRPTR(object);
	*(ppExtData) = NULL;

	return(object);
}


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginSpecMatDestructorCB(void *object, int offsetInObject, int sizeInObject)
{
	CustomSpecMapPipeMaterialData **ppExtData	= GETSPECMAPPIPEDATAPTRPTR(object); 
	CustomSpecMapPipeMaterialData *pExtData		= *ppExtData;

	if(pExtData)
	{
		if(pExtData->pSpecularTexture)
		{
			RwTextureDestroy(pExtData->pSpecularTexture);
			pExtData->pSpecularTexture = NULL;
		}
		//pExtData->fSpecularity = 0.0f;


		//GtaFree(pData);
		m_gSpecMapPipeMatDataPool->Delete(pExtData);
		#ifdef USE_POOL_COUNTER
			nSpecMapPipeMatDataPoolCounter--;
		#endif

		*ppExtData = NULL;
		pExtData = NULL;
	}

	return(object);
}


//
//
//
//
void* CCustomCarEnvMapPipeline::pluginSpecMatCopyConstructorCB(void *dstObject, const void *srcObject, int offsetInObject, int sizeInObject)
{
	CustomSpecMapPipeMaterialData *pSrcData 	= GETSPECMAPPIPEDATAPTR(srcObject);

	CustomSpecMapPipeMaterialData **ppDstData	= GETSPECMAPPIPEDATAPTRPTR(dstObject);
	CustomSpecMapPipeMaterialData *pDstData		= *ppDstData;


		if(!pSrcData)
			return(dstObject);	// nothing to copy!

		if(!pDstData)
		{
			//*ppDstData = (CustomSpecMapPipeMaterialData*)GtaMalloc(sizeof(CustomSpecMapPipeMaterialData));
			*ppDstData = m_gSpecMapPipeMatDataPool->New();
			if(!(*ppDstData))
			{
				// rare case when pool runs out of memory:
				return(dstObject);
			}
			#ifdef USE_POOL_COUNTER
				nSpecMapPipeMatDataPoolCounter++;
			#endif
			pDstData = *ppDstData;
		}

		// copy data:
		pDstData->fSpecularity		= pSrcData->fSpecularity;
		pDstData->pSpecularTexture	= pSrcData->pSpecularTexture;
		if(pDstData->pSpecularTexture)
		{
			RwTextureAddRef(pDstData->pSpecularTexture);	// texture has been instanced, increase its ref count 
		}


	return(dstObject);
}



//
//
//
//
RwStream* CCustomCarEnvMapPipeline::pluginSpecMatStreamReadCB(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size)
{
	CustomSpecMapPipeMaterialData **ppExtData	= GETSPECMAPPIPEDATAPTRPTR(pData);
	CustomSpecMapPipeMaterialData *pExtData		= *ppExtData;

	// prohibit overwriting valid data (they should be freed first):
	ASSERT(pExtData == NULL);


CustomSpecMapPipeMaterialDiskData diskData;
	RwStreamRead(pStream, &diskData, len);

	if(diskData.fSpecularity != 0.0f)
	{
		// read specular texture:
		RwTexture *specTex = RwTextureRead(diskData.cName, NULL);
		#ifndef FINAL
			if(!specTex)
			{
				char buf[256];
				::sprintf(buf, "Specular texture '%s' not found! See PaulK to fix this.", diskData.cName);
				ASSERTMSG(specTex, buf); 
			}
		#else
			ASSERT(specTex);
		#endif

		if(specTex)
		{
			//*ppDstData = (CustomSpecMapPipeMaterialData*)GtaMalloc(sizeof(CustomSpecMapPipeMaterialData));
			*ppExtData = m_gSpecMapPipeMatDataPool->New();
			if(!(*ppExtData))
			{
				// rare case when pool runs out of memory
				return(pStream);
			}
			#ifdef USE_POOL_COUNTER
				nSpecMapPipeMatDataPoolCounter++;
			#endif
			pExtData = *ppExtData;


			pExtData->pSpecularTexture	= specTex;
			pExtData->fSpecularity		= diskData.fSpecularity;
		}
	}

	return(pStream);
}


//
//
//
//
RwStream* CCustomCarEnvMapPipeline::pluginSpecMatStreamWriteCB(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size)
{
	CustomSpecMapPipeMaterialData *pExtData = GETSPECMAPPIPEDATAPTR(pData);

CustomSpecMapPipeMaterialDiskData diskData;

	// default values:
	diskData.fSpecularity = 0.0f;
	::memset(&diskData.cName[0], 0, sizeof(diskData.cName));
	
	if(pExtData)
	{
		diskData.fSpecularity = pExtData->fSpecularity;
		if(pExtData->pSpecularTexture)
		{
			::strncpy(&diskData.cName[0], pExtData->pSpecularTexture->name, sizeof(diskData.cName));
		}
	}

	RwStreamWrite(pStream, &diskData, len);

	return(pStream);
}



//
//
//
//
RwInt32 CCustomCarEnvMapPipeline::pluginSpecMatStreamGetSizeCB(const void* pData, RwInt32 offset, RwInt32 size)
{
	CustomSpecMapPipeMaterialData *pExtData = GETSPECMAPPIPEDATAPTR(pData);

	if(ms_specularMapPluginOffset != -1)
	{
		return(sizeof(CustomSpecMapPipeMaterialDiskData));
	}
	else
	{
		return(-1);
	}
}





