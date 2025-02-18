//
// CustomBuildingPipeline - custom rendering pipeline for buildings on d3d9;
//
//	15/10/2004	- JonhW:	- initial port to PC;
//	04/02/2005	- Andrzej:	- initial;	
//	17/02/2005	- Andrzej:	- new version of InstanceCB() added;
//
//
//
#if defined (GTA_PC)
	#include "windows.h"
	#include "win.h"
	#include <d3d9.h>
#endif //GTA_PC

#include "rwcore.h"
#include "rpworld.h"
#include "rwPlugin.h"
#include "MemoryMgr.h"
#include "CompressedFloat.h"
#include "PipelinePlugin.h"

#if defined(GTA_PC)
	#include "CustomEnvMapPipelineCSL.h"
	#include "CustomPipelinesCommon.h"
	#include "CustomCarEnvMapPipeline.h"
#endif

#include "CustomBuildingPipeline.h"



//
// all vertex declarations + terminator declaration.
//
#define VERTELEM_NUMDECLARATIONS	(4+1)


//
//
//
//
RxPipeline*	CCustomBuildingPipeline::ObjPipeline = NULL;




//
//
//
//
CCustomBuildingPipeline::CCustomBuildingPipeline()
{
	// do nothing
}

//
//
//
//
CCustomBuildingPipeline::~CCustomBuildingPipeline()
{
	// do nothing
}

//
//
//
//
RwBool CCustomBuildingPipeline::CreatePipe()
{
    ObjPipeline = CreateCustomObjPipe();
    if(ObjPipeline == NULL)
    {
        DEBUGLOG("Cannot create static pipeline.");
        return(FALSE);
    }

    return(TRUE);
}

//
//
//
//
void CCustomBuildingPipeline::DestroyPipe()
{
    if(ObjPipeline)
    {
        RxPipelineDestroy(ObjPipeline);
        ObjPipeline = NULL;
    }
}	

//
//
//
//
RpMaterial*	CCustomBuildingPipeline::CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__)
{
#if defined(GTA_PC)
	ASSERT(pMaterial);
	ASSERT(ObjPipeline);

	CBMATFX_SET_PASS_CFG(pMaterial, 0);	// clear pass bits


const RpMatFXMaterialFlags effects = RpMatFXMaterialGetEffects(pMaterial);
	if(effects == rpMATFXEFFECTENVMAP)
	{
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

	//
	// set pass configuration to material:
	// use 3 least signifant bits in pMaterial->surfaceProps.specular;
	//
	uint32 matPassMask = CBMATFX_GET_PASS_CFG(pMaterial);
	{
		matPassMask &= ~(CARFX_ALLMASK_PASS_BIT);
		matPassMask |= passMask;
	}
	CBMATFX_SET_PASS_CFG(pMaterial, matPassMask);


#endif //GTA_PC...
	return(pMaterial);
}


//
//
//
//
RpAtomic*	CCustomBuildingPipeline::CustomPipeAtomicSetup(RpAtomic *pAtomic)
{
RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);

	RpGeometryForAllMaterials(pGeometry, CCustomBuildingPipeline::CustomPipeMaterialSetup, NULL);

	RpAtomicSetPipeline(pAtomic, (RxPipeline*)ObjPipeline);
	SetPipelineID(pAtomic, PDSPC_CBuildingEnvMap_AtmID);

	return(pAtomic);
}


//
//
// old (simpler) version of InstanceCB, which doesn't support re-instancing:
//
#if 0	//#if defined(GTA_PC)
RwBool CCustomBuildingPipeline::CustomPipeInstanceCB(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
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
const bool bInstanceNormal	= bool(geometryFlags&rpGEOMETRYNORMALS);	
const bool bInstanceRGBA	= bool(geometryFlags&rpGEOMETRYPRELIT);

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

	// Vertex Colors:
	//ASSERT(geometryFlags&rpGEOMETRYPRELIT);
	if(bInstanceRGBA)
	{
		declaration[declarationIndex].Stream	= 0;
		declaration[declarationIndex].Offset	= offset;
		declaration[declarationIndex].Type		= D3DDECLTYPE_D3DCOLOR;
		declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
		declaration[declarationIndex].Usage		= D3DDECLUSAGE_COLOR;
		declaration[declarationIndex].UsageIndex= 0;
		declarationIndex++;
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
		offset += sizeof(RwRGBA);
		vertexStream->stride += sizeof(RwRGBA);
		vertexStream->geometryFlags |= rpGEOMETRYLOCKPRELIGHT;
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

	// Vertex Colors:
	if(bInstanceRGBA)
	{
		const RwRGBA* colors = RpGeometryGetPreLightColors(pGeometry);
		ASSERT(colors);

		offset += _rpD3D9VertexDeclarationInstColor(lockedVertexBuffer + offset,
												colors,
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
RwBool CCustomBuildingPipeline::CustomPipeInstanceCB(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
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
const bool bInstanceNormal	= bool(geometryFlags&rpGEOMETRYNORMALS);	
const bool bInstanceRGBA	= bool(geometryFlags&rpGEOMETRYPRELIT);



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

		// Vertex Colors:
		//ASSERT(geometryFlags&rpGEOMETRYPRELIT);
		if(bInstanceRGBA)
		{
			declaration[declarationIndex].Stream	= 0;
			declaration[declarationIndex].Offset	= offset;
			declaration[declarationIndex].Type		= D3DDECLTYPE_D3DCOLOR;
			declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
			declaration[declarationIndex].Usage		= D3DDECLUSAGE_COLOR;
			declaration[declarationIndex].UsageIndex= 0;
			declarationIndex++;
			ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
			offset += sizeof(RwRGBA);
			vertexStream->stride += sizeof(RwV3d);
			vertexStream->geometryFlags |= rpGEOMETRYLOCKPRELIGHT;
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

	// Vertex Colors:
	if(bInstanceRGBA && (lockedSinceLastInst&rpGEOMETRYLOCKPRELIGHT))
	{
		const RwRGBA* colors = RpGeometryGetPreLightColors(pGeometry);
		ASSERT(colors);

        // Find prelit:
        declarationIndex = 0;
        while(	(declaration[declarationIndex].Usage != D3DDECLUSAGE_COLOR)	||
				(declaration[declarationIndex].UsageIndex != 0)				)
        {
            declarationIndex++;
        }
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);

        const RwRGBA    *color = (const RwRGBA *)(pGeometry->preLitLum);
        uint32 stride = resEntryHeader->vertexStream->stride;

        int32 numMeshes = resEntryHeader->numMeshes;
        instancedData = (RxD3D9InstanceData*)(resEntryHeader + 1);

        do
        {
            instancedData->vertexAlpha = _rpD3D9VertexDeclarationInstColor(lockedVertexBuffer +
															declaration[declarationIndex].Offset +
															((instancedData->minVert) * stride),
															color + instancedData->minVert,
															instancedData->numVertices,
															stride);
			instancedData++;
        }
        while(--numMeshes);
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
RwBool CCustomBuildingPipeline::CustomPipeReinstanceCB(void *object, RwResEntry *resEntry, RxD3D9AllInOneInstanceCallBack instanceCallback)
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
//
//
//
#if defined(GTA_PC)
void CCustomBuildingPipeline::CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags)
{

RxD3D9ResEntryHeader    *resEntryHeader;
RxD3D9InstanceData      *instancedData;
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

		const uint32 matPassMask = CBMATFX_GET_PASS_CFG(pMaterial);

const bool bUseEnvMap1		= (matPassMask&CARFX_ENVMAP1_PASS_BIT);

////////////////////////////////////////////////////////////////////////////////////////////////////////


	RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);

	// EnvMap1:
	if(bUseEnvMap1)
	{
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
		RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,	D3DTTFF_DISABLE/*D3DTTFF_COUNT2*//*(D3DTTFF_COUNT3|D3DTTFF_PROJECTED)*/); // projected avoids translation "jumps"
		
		RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);	// no next pass
	}// if(bUseEnvMap1)...

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
                RwD3D9SetSurfaceProperties(&(instancedData->material->surfaceProps),
                                           &(instancedData->material->color),
                                           flags);
			}
		    
			CustomD3D9Pipeline_AtomicDefaultRender(resEntryHeader, instancedData, flags, instancedData->material->texture);
		}
        else
        {
            CustomD3D9Pipeline_AtomicRenderBlack(resEntryHeader, instancedData);
        }

		instancedData++;
    }// for(int32 i=0; i<numMeshes; i++)...


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
#endif //GTA_PC...



//
//
//
//
RxPipeline* CCustomBuildingPipeline::CreateCustomObjPipe(void)
{

	RxPipeline *pipe = RxPipelineCreate();
#if defined (GTA_XBOX)
	RxNodeDefinition *nodedef = RxNodeDefinitionGetXboxAtomicAllInOne();
#elif defined (GTA_PC)
	RxNodeDefinition *nodedef = RxNodeDefinitionGetD3D9AtomicAllInOne();
#endif

	if(pipe != NULL)
	{
		RxLockedPipe *lpipe = RxPipelineLock(pipe);
		if ( lpipe != NULL )
		{
			if(RxLockedPipeAddFragment(lpipe, NULL, nodedef, NULL))
			{
				if(RxLockedPipeUnlock(lpipe))
				{
					RxPipelineNode *allinone = RxPipelineFindNodeByName(pipe, nodedef->name, NULL, 0);
					ASSERT(allinone);
					// use default callbacks for rendering and instancing:
#if defined(GTA_PC)
					// use standard instancing to avoid problems with onchip accelerators (Intel, S3, etc.)
					// this may be connected with the fact, they're sharing system memory
					RxD3D9AllInOneSetInstanceCallBack(	allinone,	/*CustomPipeInstanceCB*/RxD3D9AllInOneGetInstanceCallBack(allinone));
					RxD3D9AllInOneSetReinstanceCallBack(allinone,	CustomPipeReinstanceCB);	//RxD3D9AllInOneGetReinstanceCallBack(allinone));
					RxD3D9AllInOneSetRenderCallBack(	allinone,	CustomPipeRenderCB);		//RxD3D9AllInOneGetRenderCallBack(allinone));
#endif
					pipe->pluginId	= PDSPC_CBuildingEnvMap_AtmID;
					pipe->pluginData= PDSPC_CBuildingEnvMap_AtmID;
					return(pipe); // success
				}
			}
		}

		RxPipelineDestroy(pipe);
	}

    return(NULL);
}// end of CreateCustomD3D9ObjPipe()...



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PLUGINS PROPERTIES:
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(GTA_PC)
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
void CCustomBuildingPipeline::SetFxEnvTexture(RpMaterial *pMaterial, RwTexture *fxTexture)
{
	ASSERT(pMaterial);
	
	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;
	 
	if(!fxTexture)
	{
		pData->pEnvTexture = GetFXMatEnvTexture(pMaterial);
		/*
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
		*/
		
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
RwTexture* CCustomBuildingPipeline::GetFxEnvTexture(RpMaterial *pMaterial)
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
void CCustomBuildingPipeline::SetFxEnvScale(RpMaterial *pMaterial, float envScaleX, float envScaleY)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvScaleX(envScaleX);
	pData->SetfEnvScaleY(envScaleY);

}


//
//
//
//
float CCustomBuildingPipeline::GetFxEnvScaleX(RpMaterial *pMaterial)
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
float CCustomBuildingPipeline::GetFxEnvScaleY(RpMaterial *pMaterial)
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
void CCustomBuildingPipeline::SetFxEnvTransScl(RpMaterial *pMaterial, float envTransX, float envTransY)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvTransSclX(envTransX);
	pData->SetfEnvTransSclY(envTransY);

}

float CCustomBuildingPipeline::GetFxEnvTransSclX(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclX());
}

float CCustomBuildingPipeline::GetFxEnvTransSclY(RpMaterial *pMaterial)
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
void CCustomBuildingPipeline::SetFxEnvShininess(RpMaterial *pMaterial, float envShininess)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;
	
	pData->SetfShininess(envShininess);
}

//
//
//
//
float CCustomBuildingPipeline::GetFxEnvShininess(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfShininess());
}
#endif //GTA_PC



