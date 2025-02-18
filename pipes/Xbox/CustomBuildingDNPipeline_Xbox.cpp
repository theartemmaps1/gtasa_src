//
// CustomBuildingDNPipeline - custom pipeline for buildings with "Day&Night" support for d3d9;
//
//	15/10/2004	- JonhW:	- initial port to PC;
//	04/02/2005	- Andrzej:	- initial;
//	14/02/2005	- Andrzej:	- EnvMap support added;
//	17/02/2005	- Andrzej:	- new version of InstanceCB() added;
//	21/02/2005	- Andrzej:	- GTA_PC: DNVertexShader support added (if present on device caps);
//	28/02/2005	- Andrzej:	- GTA_XBOX has its own sourcecode for pipeline;
//
//
//
#if defined (GTA_PC)
	#include "windows.h"
	#include "win.h"
	#include <d3d9.h>
	#include <d3dx9math.h>	//D3DXMATRIX
#endif //GTA_PC

#include "rwcore.h"
#include "rpworld.h"
#include "camera.h"
#include "rwPlugin.h"
#include "MemoryMgr.h"
#include "PipelinePlugin.h"
#ifdef GTA_XBOX
	#include "hdstream.h"
#endif

#if defined(GTA_PC)
	#include "CustomEnvMapPipelineCSL.h"
	#include "CustomPipelinesCommonPC.h"
	#include "CustomCarEnvMapPipeline.h"
#endif

#include "CustomBuildingDNPipeline_Xbox.h"


//
// set hint for default RW's InstanceCB(): this data is going to be changed every frame
// and dynamic vertex buffer can be used (DYNAMICPRELIT flag):
//
#ifdef GTA_PC
	#define USE_DYNAMICPRELIT_HINT			(1)
#endif


#ifdef GTA_PC
// switch off VS_1_1 support, because it fails on ATI video cards (their DX implentation is more strict)
//	#define GTA_PC_USE_DN_VS					(1)
#endif


//
//
//
//
//
int32 CCustomBuildingDNPipeline::ms_extraVertColourPluginOffset = -1;

#define EXTRAVERTCOLOUR_SIZE 			(sizeof(CDNGeomExtension))
#define EXTRAVERTCOLOURFROMGEOM(GEOM) 	((char*)(((RwUInt8*)GEOM) + ms_extraVertColourPluginOffset))
#define GETPLUGINBASE(GEOM) 			((char*)(((RwUInt8*)GEOM) + ms_extraVertColourPluginOffset))
#define EXTRAVERT_CURRENTDN(GEOM)		(((CDNGeomExtension*)GETPLUGINBASE(GEOM))->m_fCurrentDNValue)
#define EXTRAVERT_DAYTABLE(GEOM)		(((CDNGeomExtension*)GETPLUGINBASE(GEOM))->m_pDayColTable)
#define EXTRAVERT_NIGHTTABLE(GEOM)		(((CDNGeomExtension*)GETPLUGINBASE(GEOM))->m_pNightColTable)

#define	DN_NUM_FRAMES_FOR_PROCESS		(16)		// process pre lighting over this number of frames



//
//
//
//
float		CCustomBuildingDNPipeline::m_fDNBalanceParam	= 1.0f;
uint32		CCustomBuildingDNPipeline::m_AtmDNWorkingIndex	= 0;			// used to process atomics across several frames
RwBool		CCustomBuildingDNPipeline::m_bCameraChange		= FALSE;		// set if require DN update to process all atomics this frame
RxPipeline*	CCustomBuildingDNPipeline::ObjPipeline			= NULL;
RwBool		CCustomBuildingDNPipeline::m_bDeviceSupportsVS11= FALSE;

//
// all vertex declarations + terminator declaration.
//
#ifdef GTA_PC_USE_DN_VS
	#define VERTELEM_NUMDECLARATIONS	(5+1)
#else
	#define VERTELEM_NUMDECLARATIONS	(4+1)
#endif


//////////// new XBox vertex shader stuff (JG) /////////////////////////////
#if defined(GTA_XBOX)
	#define DN_USE_VERTEX_SHADER		(1)

	// the compiled vertex shader
	#include "DNshader.h"
	static DWORD		DNVertexShader; // shader id

	// determine what is put in the input registers (v0->v15) use by the shader asm code (in DNshader.vsh)
	// Each register holds 4 floats.
	static
	DWORD	DNVertexShaderDeclaration[] =
	{
		D3DVSD_STREAM( 0 ),
		D3DVSD_REG( 0, D3DVSDT_FLOAT3),			// position  - register v0 
		D3DVSD_REG( 1, D3DVSDT_NORMPACKED3),	// normal  - register v1 
		D3DVSD_REG( 2, D3DVSDT_D3DCOLOR ),		// diffuse colour - register v2
 		D3DVSD_REG( 3, D3DVSDT_FLOAT2),			// tex coords t0  - register v3 
		D3DVSD_END()
	};
#endif // GTA_XBOX

//////////////////////////////////////////////////////////////////////////////

#ifdef GTA_PC_USE_DN_VS
	#include "DNshaderPC.h"
	static DWORD		DNVertexShaderPC=0;		// shader ir
#endif //GTA_PC_USE_DN_VS...

//
//
//
//
CCustomBuildingDNPipeline::CCustomBuildingDNPipeline()
{
	// do nothing
}

//
//
//
//
CCustomBuildingDNPipeline::~CCustomBuildingDNPipeline()
{
	// do nothing
}

//
//
//
//
RwBool	CCustomBuildingDNPipeline::CreatePipe()
{

	ObjPipeline = CreateCustomObjPipe();
    if(ObjPipeline  == NULL)
    {
        DEBUGLOG("Cannot create static pipeline.");
        return FALSE;
    }

    return(TRUE);
}


//
//
//
//
void CCustomBuildingDNPipeline::DestroyPipe()
{
    if(ObjPipeline)
    {
#ifdef DN_USE_VERTEX_SHADER
		D3DDevice_DeleteVertexShader(DNVertexShader);
#endif

#ifdef GTA_PC_USE_DN_VS
		if(DNVertexShaderPC && m_bDeviceSupportsVS11)
		{
			RwD3D9DeleteVertexShader((void*)DNVertexShaderPC);
			DNVertexShaderPC = 0;
		}
#endif

        RxPipelineDestroy(ObjPipeline);
        ObjPipeline = NULL;
    }
}	

//
//
//
//
RpMaterial*	CCustomBuildingDNPipeline::CustomPipeMaterialSetup(RpMaterial *pMaterial, void *__unused__)
{
#if defined(GTA_PC)
	ASSERT(pMaterial);
	ASSERT(ObjPipeline);

	CBMATFX_SET_PASS_CFG(pMaterial, 0);	// clear pass bits


const RpMatFXMaterialFlags effects = RpMatFXMaterialGetEffects(pMaterial);
	if(effects == rpMATFXEFFECTENVMAP)
	{
		// set EnvMapFX texture from MatFX:
		CCustomBuildingDNPipeline::SetFxEnvTexture(pMaterial, NULL);
	}


uint32 passMask = 0;

	if( (CCustomBuildingDNPipeline::GetFxEnvShininess(pMaterial) != 0.0f)	&&
		(CCustomBuildingDNPipeline::GetFxEnvTexture(pMaterial)				)																	)
	{
		RwTexture *pEnvMapTex = CCustomBuildingDNPipeline::GetFxEnvTexture(pMaterial);
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
RpAtomic* CCustomBuildingDNPipeline::CustomPipeAtomicSetup(RpAtomic *pAtomic)
{
	RpGeometry *pGeometry = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeometry);

	RpGeometryForAllMaterials(pGeometry, CCustomBuildingDNPipeline::CustomPipeMaterialSetup, NULL);

#ifdef USE_DYNAMICPRELIT_HINT
	// set hint for default RW's InstanceCB(): this data is going to be changed every frame
	// and dynamic vertex buffer can be used:
	RpD3D9GeometrySetUsageFlags(pGeometry, rpD3D9GEOMETRYUSAGE_DYNAMICPRELIT);
#endif

	RpAtomicSetPipeline(pAtomic, (RxPipeline*)ObjPipeline);
	SetPipelineID(pAtomic, PDSPC_CBuildingDNEnvMap_AtmID);

	return(pAtomic);
}


//
//
//
//
RwBool CCustomBuildingDNPipeline::UsesThisPipeline(RpAtomic *pAtomic)
{
RxPipeline* pPipeline=NULL;
	
	RpAtomicGetPipeline(pAtomic, &pPipeline);
	if(ObjPipeline == pPipeline)
	{
		return(TRUE);
	}
	
	return(FALSE);
}



//static UInt32 count[DN_NUM_FRAMES_FOR_PROCESS];

// if the change in DN value is above this value then force a DN update of the prelitLum in the geom.
#define BIG_UPDATE_THRESHOLD	(0.3f)
// if the change in DN value is above this value then do a scheduled update of the prelitLum stuff
#define SMALL_UPDATE_THRESHOLD	(0.01f)
// otherwise no update for this atomic will be done (since it is expensive...)

//
//
//
//
void CCustomBuildingDNPipeline::UpdateAtomicDN(RpAtomic* pAtomic, RwBool bForceUpdate) 
{
#ifdef GTA_PC_USE_DN_VS
		if(m_bDeviceSupportsVS11)
		{
			return;	// already done by VertexShader
		}
#endif

		if(!pAtomic)
			return;

		if(!CCustomBuildingDNPipeline::UsesThisPipeline(pAtomic))
		{
			return;
		}


		RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
		const float DNValDiff = CMaths::Abs(EXTRAVERT_CURRENTDN(pGeom) - m_fDNBalanceParam);
		
		if((DNValDiff > SMALL_UPDATE_THRESHOLD) || bForceUpdate)
		{	
			// this should give a pretty even distribution of atomics to process each frame
			const uint32 index = ((UInt32)pAtomic / (sizeof(RpAtomic)))%DN_NUM_FRAMES_FOR_PROCESS;
			
			// process this atomic if camera has made a big change or this is it's turn to process this frame
			if ((index == m_AtmDNWorkingIndex) || m_bCameraChange || bForceUpdate || (DNValDiff > BIG_UPDATE_THRESHOLD))
			{
//				count[m_AtmDNWorkingIndex]++;
				UpdateAtomicPrelitLum(pAtomic, m_fDNBalanceParam); 
			}
		}

}

//
//
//
//
static
RpAtomic* UpdateClumpAtomicCB(RpAtomic *pAtomic, void *data)
{
	RwBool bForceUpdate = *((RwBool*)data);
	CCustomBuildingDNPipeline::UpdateAtomicDN(pAtomic, bForceUpdate);
	return(pAtomic);
}

//
//
//
//
void CCustomBuildingDNPipeline::UpdateClumpDN(RpClump* pClump, RwBool bForceUpdate) 
{
#ifdef GTA_PC_USE_DN_VS
	if(m_bDeviceSupportsVS11)
	{
		return;	// already done by VertexShader
	}
#endif

	RpClumpForAllAtomics(pClump, UpdateClumpAtomicCB, &bForceUpdate);
}




//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// DN_USE_VERTEX_SHADER stuff:
//
//
#ifdef DN_USE_VERTEX_SHADER

#if defined (GTA_XBOX)
	RxXboxAllInOneRenderCallBack DefaultRenderCallback = NULL;
#elif defined (GTA_PC)
	RxD3D9AllInOneRenderCallBack DefaultRenderCallback = NULL;
#endif //GTA_XBOX

// computes the projection matrix used by the vertex shader to project the vertex
// from world to screen space
static
void ComputeProjViewWorld(D3DXMATRIX * projViewWorldMatrix, RpAtomic *pAtomic)
{
    D3DXMATRIX projMatrix;
	D3DXMATRIX worldMatrix;
	D3DXMATRIX viewMatrix;
	D3DXMATRIX viewWorldMatrix;
                                 
    RwMatrix invCamMtx;
    RwMatrix* pAtomicLTM;

    RwCamera* pCamera = RwCameraGetCurrentCamera();
    RwMatrix* pCamLTM = RwFrameGetLTM(RwCameraGetFrame(pCamera));

	RwMatrixSetIdentity(&invCamMtx);
    RwMatrixInvert(&invCamMtx, pCamLTM);

    // View matrix - (camera matrix)
    viewMatrix.m[0][0] = -invCamMtx.right.x;
    viewMatrix.m[0][1] = -invCamMtx.up.x;
    viewMatrix.m[0][2] = -invCamMtx.at.x;
    viewMatrix.m[0][3] = -invCamMtx.pos.x;

    viewMatrix.m[1][0] = invCamMtx.right.y;
    viewMatrix.m[1][1] = invCamMtx.up.y;
    viewMatrix.m[1][2] = invCamMtx.at.y;
    viewMatrix.m[1][3] = invCamMtx.pos.y;

    viewMatrix.m[2][0] = invCamMtx.right.z;
    viewMatrix.m[2][1] = invCamMtx.up.z;
    viewMatrix.m[2][2] = invCamMtx.at.z;
    viewMatrix.m[2][3] = invCamMtx.pos.z;

    viewMatrix.m[3][0] = 0.0f;
    viewMatrix.m[3][1] = 0.0f;
    viewMatrix.m[3][2] = 0.0f;
    viewMatrix.m[3][3] = 1.0f;

    // World space transformation matrix
    pAtomicLTM = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));

    worldMatrix.m[0][0] = pAtomicLTM->right.x;
    worldMatrix.m[0][1] = pAtomicLTM->up.x;
    worldMatrix.m[0][2] = pAtomicLTM->at.x;
    worldMatrix.m[0][3] = pAtomicLTM->pos.x;

    worldMatrix.m[1][0] = pAtomicLTM->right.y;
    worldMatrix.m[1][1] = pAtomicLTM->up.y;
    worldMatrix.m[1][2] = pAtomicLTM->at.y;
    worldMatrix.m[1][3] = pAtomicLTM->pos.y;

    worldMatrix.m[2][0] = pAtomicLTM->right.z;
    worldMatrix.m[2][1] = pAtomicLTM->up.z;
    worldMatrix.m[2][2] = pAtomicLTM->at.z;
    worldMatrix.m[2][3] = pAtomicLTM->pos.z;

    worldMatrix.m[3][0] = 0.0f;
    worldMatrix.m[3][1] = 0.0f;
    worldMatrix.m[3][2] = 0.0f;
    worldMatrix.m[3][3] = 1.0f;

	// Projection matrix
    projMatrix.m[0][0] = pCamera->recipViewWindow.x;
    projMatrix.m[0][1] = 0.0f;
    projMatrix.m[0][2] = 0.0f;
    projMatrix.m[0][3] = 0.0f;

    projMatrix.m[1][0] = 0.0f;
    projMatrix.m[1][1] = pCamera->recipViewWindow.y;
    projMatrix.m[1][2] = 0.0f;
    projMatrix.m[1][3] = 0.0f;

    projMatrix.m[2][0] = 0.0f;
    projMatrix.m[2][1] = 0.0f;
    projMatrix.m[2][2] = pCamera->farPlane / (pCamera->farPlane - pCamera->nearPlane);
    projMatrix.m[2][3] = -projMatrix.m[2][2] * pCamera->nearPlane;

    projMatrix.m[3][0] = 0.0f;
    projMatrix.m[3][1] = 0.0f;
    projMatrix.m[3][2] = 1.0f;
    projMatrix.m[3][3] = 0.0f;

    D3DXMatrixMultiply(&viewWorldMatrix, &viewMatrix, &worldMatrix);
    D3DXMatrixMultiply(projViewWorldMatrix, &projMatrix, &viewWorldMatrix);
}

// sets up some data needed by the vertex shader in the constat registers
static
void VertexShaderSetConstantRegisters(RpAtomic *pAtomic)
{
	RwV4d offset;
	D3DXMATRIX projViewWorldMatrix;

	ComputeProjViewWorld(&projViewWorldMatrix, pAtomic);

    // Set the constant registers c0-c3 with the transformation matrix
    D3DDevice_SetVertexShaderConstant(0, (void *)&projViewWorldMatrix, 4);

	// set register c4 with the DN interp value
	float DNBalanceParam = CCustomBuildingDNPipeline::GetDNBalanceParam();

	if (DNBalanceParam < 0.0f)
		DNBalanceParam = 0.0f;

	if (DNBalanceParam > 1.0f)
		DNBalanceParam = 1.0f;
	
	D3DDevice_SetVertexShaderConstant( 4, (float*)&DNBalanceParam, 1 );

	// set register c5 with 1 - the DN interp value
	DNBalanceParam = 1.0f - DNBalanceParam;
	D3DDevice_SetVertexShaderConstant( 5, (float*)&DNBalanceParam, 1 );

#ifdef HDSTREAM_DEBUG
	// this is for colouring the map red or green to show how the hard drive streaming is working
	// ** NEED TO CHANGE THE SHADER ALSO **
	float colour1[4] = {1.0f, 0.0f, 0.0f, 1.0f};
	float colour2[4] = {0.0f, 1.0f, 0.0f, 1.0f};
	float colour3[4] = {0.0f, 0.0f, 1.0f, 1.0f};

	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	CDNGeomExtension*	pGeomExtension = (CDNGeomExtension*)GETPLUGINBASE(pGeom);

	if (pGeomExtension->m_fCurrentDNValue  > 0.6f)
		D3DDevice_SetVertexShaderConstant( 6, (float*)&colour2, 4 );
	else if (pGeomExtension->m_fCurrentDNValue  > 0.3f)
		D3DDevice_SetVertexShaderConstant( 6, (float*)&colour1, 4 );
	else
		D3DDevice_SetVertexShaderConstant( 6, (float*)&colour3, 4 );
#endif

}

static
void VShaderRenderCB(RxXboxResEntryHeader *resEntryHeader, void *object, RwUInt8 type, RwUInt32 flags)
{
RxXboxInstanceData      *mesh;
 	D3DDevice_SetShaderConstantMode(D3DSCM_96CONSTANTS);
	// Set up vertex shader with required constants
    VertexShaderSetConstantRegisters((RpAtomic *)object);
	// Set the stream source - shared by all meshes in geometry
	// (already done in DefaultRenderCallback)
/*
	done in DefaultRenderCallback
	D3DDevice_SetStreamSource(0,
							 (D3DVertexBuffer *)resEntryHeader->vertexBuffer,
	                           resEntryHeader->stride);
							   */

	for (mesh = resEntryHeader->begin; mesh != resEntryHeader->end; ++mesh)
	{
		mesh->vertexShader = DNVertexShader;
	}

	D3DDevice_SetVertexShader(DNVertexShader);
	// render as normal
	DefaultRenderCallback(resEntryHeader, object, type, flags);

}
#endif //DN_USE_VERTEX_SHADER....
//////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
//
//
static 
RwBool Custom_rpD3D9VertexDeclarationInstColor(RwUInt8 *mem,
                       const RwRGBA *srcColor, RwInt32 numVerts, RwUInt32 stride)
{
    RwUInt8     alpha = 0xff;

    ASSERT(NULL != mem);
    ASSERT(NULL != srcColor);

    RwUInt32 *dstColor = (RwUInt32 *)mem;
    for(int32 i=0; i<numVerts; i++)
    {
		RwRGBA srccolor = srcColor[i];

#define SCALE_COLOR(COL)		{ uint32 c=COL; c*=3; if(c>255) c=255; COL=c; }
		SCALE_COLOR(srccolor.red);
		SCALE_COLOR(srccolor.green);
		SCALE_COLOR(srccolor.blue);
#undef SCALE_COLOR

        *(dstColor) = ((srccolor.alpha	<< 24) |
                       (srccolor.red	<< 16) |
                       (srccolor.green	<<  8) |
                       (srccolor.blue	<<  0)	);

//		*(dstColor) = 0xFF00FFFF;

		alpha &= srcColor[i].alpha;	       // Does the pre-light contain alpha?
       
        dstColor = (RwUInt32 *)(((RwUInt8 *)dstColor) + stride);
    }

    return(alpha != 0xff);
}



//
//
//
//
#if defined(GTA_PC)
RwBool CCustomBuildingDNPipeline::CustomPipeInstanceCB(void *object, RxD3D9ResEntryHeader *resEntryHeader, RwBool reinstance)
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
			vertexStream->stride += sizeof(RwRGBA);
			vertexStream->geometryFlags |= rpGEOMETRYLOCKPRELIGHT;
		}

#ifdef GTA_PC_USE_DN_VS
		if(bInstanceRGBA && m_bDeviceSupportsVS11)
		{
			declaration[declarationIndex].Stream	= 0;
			declaration[declarationIndex].Offset	= offset;
			declaration[declarationIndex].Type		= D3DDECLTYPE_D3DCOLOR;
			declaration[declarationIndex].Method	= D3DDECLMETHOD_DEFAULT;
			declaration[declarationIndex].Usage		= D3DDECLUSAGE_COLOR;
			declaration[declarationIndex].UsageIndex= 1;
			declarationIndex++;
			ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);
			offset += sizeof(RwRGBA);
			vertexStream->stride += sizeof(RwRGBA);
			vertexStream->geometryFlags |= rpGEOMETRYLOCKPRELIGHT;
		}
#endif

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

		RwRGBA* pDayTable = /*EXTRAVERT_DAYTABLE*/EXTRAVERT_NIGHTTABLE(pGeometry);
		ASSERT(pDayTable);
		const RwRGBA    *color = pDayTable;

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

#ifdef GTA_PC_USE_DN_VS
	// Vertex Colors 2:
	if(bInstanceRGBA && (lockedSinceLastInst&rpGEOMETRYLOCKPRELIGHT) && m_bDeviceSupportsVS11)
	{
		const RwRGBA* colors = RpGeometryGetPreLightColors(pGeometry);
		ASSERT(colors);

        // Find prelit:
        declarationIndex = 0;
        while(	(declaration[declarationIndex].Usage != D3DDECLUSAGE_COLOR)	||
				(declaration[declarationIndex].UsageIndex != 1)				)
        {
            declarationIndex++;
        }
		ASSERT(declarationIndex < VERTELEM_NUMDECLARATIONS);

		RwRGBA* pNightTable = /*EXTRAVERT_NIGHTTABLE*/EXTRAVERT_DAYTABLE(pGeometry);
		ASSERT(pNightTable);
		const RwRGBA *color = pNightTable;
	
		/*Custom*/_rpD3D9VertexDeclarationInstColor(lockedVertexBuffer + declaration[declarationIndex].Offset,
									color,
									resEntryHeader->totalNumVertex,
									resEntryHeader->vertexStream[declaration[declarationIndex].Stream].stride);
	}
#endif


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
RwBool CCustomBuildingDNPipeline::CustomPipeReinstanceCB(void *object, RwResEntry *resEntry, RxD3D9AllInOneInstanceCallBack instanceCallback)
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




#if defined(GTA_PC)
// from rwsdk/driver/d3d9/d3d9device.c:
extern RwRGBAReal AmbientSaturated;	
// defined in lights.cpp:
extern RpLight *pAmbient;
extern RpLight *pDirect;

//
//
//
//
void CCustomBuildingDNPipeline::CustomPipeLightingCB(void *object)
{
    RpAtomic *pAtomic = (RpAtomic*)object;
	ASSERT(pAtomic);

    const uint32 flags = RpGeometryGetFlags(RpAtomicGetGeometry(pAtomic));
    if(flags & rpGEOMETRYLIGHT)
    {
        RwBool lighting = FALSE;
        const RwRGBAReal *color;
        
        /* if I wanted to find lights in the world for global lights use
           RpWorldForAllLights testing against each light's type for ambient or
           directional.
           For point or spot use RpAtomicForAllWorldSectors and RpWorldSectorForAllLights
           as local lights are tied to world sectors. A local light can be tied to more
           than one sector so for atomic lighting to not use the light more than once do:

                RWSRCGLOBAL(lightFrame)++;

                if (light->lightFrame != RWSRCGLOBAL(lightFrame)))
                {
                    light->lightFrame = RWSRCGLOBAL(lightFrame);
                    if (atomic's bounding sphere instersects light)
                    {
                        lighting |= _rwD3D9LightLocalEnable(light);
                    }
                }                    
         */

        // AmbientSaturated is the accumulated ambient lighting for the current object
        // AmbientSaturated is later used in RwD3D9SetSurfaceProperties called from
        // the pipeline's render callback:
		color = RpLightGetColor(pAmbient);
        AmbientSaturated.red	= color->red;
        AmbientSaturated.green	= color->green;
        AmbientSaturated.blue	= color->blue;
        AmbientSaturated.alpha	= 1.0f;
        
        // these functions calls RwD3D9SetLight:
		lighting |= _rwD3D9LightDirectionalEnable(pDirect);
        //lighting |= _rwD3D9LightLocalEnable(PointLight);

        // this functions calls RwD3D9EnableLight to enable the lights set above and 
        //   disables any other lights. Also calls RwD3D9SetRenderState(D3DRS_LIGHTING */
        // RpWorldSectors use rwSECTORATOMIC instead of rpATOMIC
		_rwD3D9LightsEnable(lighting, (RwUInt32)rpATOMIC);
    }
    else
    {
        _rwD3D9LightsEnable(FALSE, (RwUInt32)rpATOMIC);
        RwD3D9SetRenderState(D3DRS_AMBIENT, 0);
    }

}
#endif //GTA_PC...



#ifdef GTA_PC_USE_DN_VS
//
// computes the projection matrix used by the vertex shader to project the vertex
// from world to screen space
//
static
void ComputeProjViewWorld(D3DXMATRIX * projViewWorldMatrix, RpAtomic *pAtomic)
{
    D3DXMATRIX projMatrix;
	D3DXMATRIX worldMatrix;
	D3DXMATRIX viewMatrix;
	D3DXMATRIX viewWorldMatrix;
                                 
    RwMatrix invCamMtx;
    RwMatrix* pAtomicLTM;

    RwCamera* pCamera = RwCameraGetCurrentCamera();
    RwMatrix* pCamLTM = RwFrameGetLTM(RwCameraGetFrame(pCamera));

	RwMatrixSetIdentity(&invCamMtx);
    RwMatrixInvert(&invCamMtx, pCamLTM);

    // View matrix - (camera matrix)
    viewMatrix.m[0][0] = -invCamMtx.right.x;
    viewMatrix.m[0][1] = -invCamMtx.up.x;
    viewMatrix.m[0][2] = -invCamMtx.at.x;
    viewMatrix.m[0][3] = -invCamMtx.pos.x;

    viewMatrix.m[1][0] = invCamMtx.right.y;
    viewMatrix.m[1][1] = invCamMtx.up.y;
    viewMatrix.m[1][2] = invCamMtx.at.y;
    viewMatrix.m[1][3] = invCamMtx.pos.y;

    viewMatrix.m[2][0] = invCamMtx.right.z;
    viewMatrix.m[2][1] = invCamMtx.up.z;
    viewMatrix.m[2][2] = invCamMtx.at.z;
    viewMatrix.m[2][3] = invCamMtx.pos.z;

    viewMatrix.m[3][0] = 0.0f;
    viewMatrix.m[3][1] = 0.0f;
    viewMatrix.m[3][2] = 0.0f;
    viewMatrix.m[3][3] = 1.0f;

    // World space transformation matrix
    pAtomicLTM = RwFrameGetLTM(RpAtomicGetFrame(pAtomic));

    worldMatrix.m[0][0] = pAtomicLTM->right.x;
    worldMatrix.m[0][1] = pAtomicLTM->up.x;
    worldMatrix.m[0][2] = pAtomicLTM->at.x;
    worldMatrix.m[0][3] = pAtomicLTM->pos.x;

    worldMatrix.m[1][0] = pAtomicLTM->right.y;
    worldMatrix.m[1][1] = pAtomicLTM->up.y;
    worldMatrix.m[1][2] = pAtomicLTM->at.y;
    worldMatrix.m[1][3] = pAtomicLTM->pos.y;

    worldMatrix.m[2][0] = pAtomicLTM->right.z;
    worldMatrix.m[2][1] = pAtomicLTM->up.z;
    worldMatrix.m[2][2] = pAtomicLTM->at.z;
    worldMatrix.m[2][3] = pAtomicLTM->pos.z;

    worldMatrix.m[3][0] = 0.0f;
    worldMatrix.m[3][1] = 0.0f;
    worldMatrix.m[3][2] = 0.0f;
    worldMatrix.m[3][3] = 1.0f;

	// Projection matrix
    projMatrix.m[0][0] = pCamera->recipViewWindow.x;
    projMatrix.m[0][1] = 0.0f;
    projMatrix.m[0][2] = 0.0f;
    projMatrix.m[0][3] = 0.0f;

    projMatrix.m[1][0] = 0.0f;
    projMatrix.m[1][1] = pCamera->recipViewWindow.y;
    projMatrix.m[1][2] = 0.0f;
    projMatrix.m[1][3] = 0.0f;

    projMatrix.m[2][0] = 0.0f;
    projMatrix.m[2][1] = 0.0f;
    projMatrix.m[2][2] = pCamera->farPlane / (pCamera->farPlane - pCamera->nearPlane);
    projMatrix.m[2][3] = -projMatrix.m[2][2] * pCamera->nearPlane;

    projMatrix.m[3][0] = 0.0f;
    projMatrix.m[3][1] = 0.0f;
    projMatrix.m[3][2] = 1.0f;
    projMatrix.m[3][3] = 0.0f;

    D3DXMatrixMultiply(&viewWorldMatrix, &viewMatrix, &worldMatrix);
    D3DXMatrixMultiply(projViewWorldMatrix, &projMatrix, &viewWorldMatrix);
}

// sets up some data needed by the vertex shader in the constat registers
static
void VertexShaderSetConstantRegisters(RpAtomic *pAtomic)
{
D3DXMATRIX projViewWorldMatrix;

	ComputeProjViewWorld(&projViewWorldMatrix, pAtomic);

    // Set the constant registers c0-c3 with the transformation matrix
	RwD3D9SetVertexShaderConstant(0, (void*)&projViewWorldMatrix, 4);

	// set register c4 with the DN interp value
	float DNBalanceParam = CCustomBuildingDNPipeline::GetDNBalanceParam();
	if (DNBalanceParam < 0.0f)
		DNBalanceParam = 0.0f;

	if (DNBalanceParam > 1.0f)
		DNBalanceParam = 1.0f;

	// c4:
	float dnparam[4];
	dnparam[0] = 1.0f - DNBalanceParam;
	dnparam[1] = DNBalanceParam;
	RwD3D9SetVertexShaderConstant(4, (float*)&dnparam[0], 1 );

}
#endif //GTA_PC_USE_DN_VS...


//
//
//
//
#if defined(GTA_PC)
void CCustomBuildingDNPipeline::CustomPipeRenderCB(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags)
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

#ifdef GTA_PC_USE_DN_VS
	if(m_bDeviceSupportsVS11)
	{
		RwD3D9SetVertexShader((void*)DNVertexShaderPC);
		VertexShaderSetConstantRegisters(pAtomic);
		//instancedData->vertexShader = (void*)DNVertexShaderPC;	// VS is set in CustomD3D9Pipeline_AtomicDefaultRender()
	}
#endif


    const int32 numMeshes = resEntryHeader->numMeshes;
    for(int32 i=0; i<numMeshes; i++)
    {
		RpMaterial *pMaterial = instancedData->material;
		ASSERT(pMaterial);
		CustomEnvMapPipeMaterialData *pEnvMatData = GETENVMAPPIPEDATAPTR(pMaterial);
		ASSERT(pEnvMatData);

#ifdef GTA_PC_USE_DN_VS
		if(m_bDeviceSupportsVS11)
		{
			instancedData->vertexShader = (void*)DNVertexShaderPC;	// VS is set in CustomD3D9Pipeline_AtomicDefaultRender()
		}
#endif

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
		RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);	//D3DTA_TFACTOR);
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
RxPipeline* CCustomBuildingDNPipeline::CreateCustomObjPipe(void)
{
#if defined (GTA_PC)
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GTA_PC_USE_DN_VS
		const D3DCAPS9 *d3d9Caps = (const D3DCAPS9*)RwD3D9GetCaps();
		ASSERT(d3d9Caps);
		if(d3d9Caps->VertexShaderVersion >= D3DVS_VERSION(1,1))
		{
			m_bDeviceSupportsVS11 = TRUE;
		}
		else
		{
			m_bDeviceSupportsVS11 = FALSE;
		}
#endif



	RxPipeline *pipe = RxPipelineCreate();
	RxNodeDefinition *nodedef = RxNodeDefinitionGetD3D9AtomicAllInOne();

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
#ifdef GTA_PC

#ifdef USE_DYNAMICPRELIT_HINT
					if(m_bDeviceSupportsVS11)
					{
						RxD3D9AllInOneSetInstanceCallBack(	allinone,	CustomPipeInstanceCB);
						RxD3D9AllInOneSetReinstanceCallBack(allinone,	CustomPipeReinstanceCB);
					}
					else
					{
						// use dynamic vertex buffers (rpD3D9GEOMETRYUSAGE_DYNAMICPRELIT)
						// it's supported only in generic InstanceCB:
						RxD3D9AllInOneSetInstanceCallBack(	allinone,	RxD3D9AllInOneGetInstanceCallBack(allinone));
						RxD3D9AllInOneSetReinstanceCallBack(allinone,	RxD3D9AllInOneGetReinstanceCallBack(allinone));
					}
#else
					RxD3D9AllInOneSetInstanceCallBack(	allinone,	CustomPipeInstanceCB);
					RxD3D9AllInOneSetReinstanceCallBack(allinone,	CustomPipeReinstanceCB);
#endif

					//RxD3D9AllInOneSetLightingCallBack(	allinone,	CustomPipeLightingCB);
					RxD3D9AllInOneSetRenderCallBack(	allinone,	CustomPipeRenderCB);		//RxD3D9AllInOneGetRenderCallBack(allinone));
					pipe->pluginId	= PDSPC_CBuildingDNEnvMap_AtmID;
					pipe->pluginData= PDSPC_CBuildingDNEnvMap_AtmID;

#ifdef GTA_PC_USE_DN_VS
					DNVertexShaderPC = 0;
					if(m_bDeviceSupportsVS11)
					{
						if(!RwD3D9CreateVertexShader((const RwUInt32*)&/*dwDNVertexShaderPC_vs11*/g_vs11_main[0], (void**)&DNVertexShaderPC))
						{
							RxPipelineDestroy(pipe);
							return(NULL);
						}
					}
#endif//GTA_PC_USE_DN_VS...

#endif
					return (pipe); // success
				}
			}
		}

		RxPipelineDestroy(pipe);
	}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //GTA_PC...


#if defined (GTA_XBOX)
////////////////////////////////////////////////////////////////////////////////////////////////////
	RxPipeline *pipe = RxPipelineCreate();
	RxNodeDefinition *nodedef = RxNodeDefinitionGetXboxAtomicAllInOne();

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

					// use default callbacks for rendering and instancing:

#ifdef DN_USE_VERTEX_SHADER
					// use default callbacks for rendering and instancing 
					// new pipe does adds the day / night update to the renderer
						DefaultRenderCallback = RxXboxAllInOneGetRenderCallBack(allinone);
						RxXboxAllInOneSetRenderCallBack(allinone, VShaderRenderCB);
						//new XBox vertex shader stuff (JG)
						// create the vertex shader
						if (D3D_OK != D3DDevice_CreateVertexShader(DNVertexShaderDeclaration,
																	dwDNshaderVertexShader,
																	&DNVertexShader, 0))
						{
							DEBUGLOG("Cannot create vertex shader for building DN pipeline.");
							RxPipelineDestroy(pipe);
							return NULL;
						}
#endif // DN_USE_VERTEX_SHADER

					return (pipe); // success
				}
			}
		}

		RxPipelineDestroy(pipe);
	}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //GTA_XBOX...



    return(NULL);
}



//
//
//
//
void CCustomBuildingDNPipeline::Update(RwBool bCameraChange)
{
	// need to identify which subset of atomics we want to change each frame
	m_AtmDNWorkingIndex = (m_AtmDNWorkingIndex+1) % DN_NUM_FRAMES_FOR_PROCESS;
	
	// forces all rendered atomics to be updated (for when camera flips 180 degress etc.)
	m_bCameraChange = bCameraChange;
}



//
//
// edit the prelitlum values for this atomic
//
void CCustomBuildingDNPipeline::UpdateAtomicPrelitLum(RpAtomic* pAtomic, float interpVal)
{
#ifdef HDSTREAM_DEBUG
	return;
#endif

	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeom);
	CDNGeomExtension*	pGeomExtension = (CDNGeomExtension*)GETPLUGINBASE(pGeom);
	ASSERT(pGeomExtension);

//	if (!pGeom || !(RpGeometryGetPreLightColors(pGeom)))
//	{
//		return;
//	}

	// store the current DN value which has been used to build the prelitLum data for this geometry
	pGeomExtension->m_fCurrentDNValue = interpVal;

#ifdef DN_USE_VERTEX_SHADER
	return;
#endif

	RwRGBA* pColourTable = RpGeometryGetPreLightColors(pGeom);
	ASSERT(pColourTable);
	uint8** ppInfo = (uint8**)EXTRAVERTCOLOURFROMGEOM(pGeom);
	ASSERT(ppInfo);
	RwRGBA* pNightTable = EXTRAVERT_NIGHTTABLE(pGeom);
	ASSERT(pNightTable);
	RwRGBA* pDayTable = EXTRAVERT_DAYTABLE(pGeom);
	ASSERT(pDayTable);

	const int32 tableSize = RpGeometryGetNumVertices(pGeom);

	// might not actually be a DN atomic
//	if (!pNightTable)
//	{
//		return;
//	}
	
	pGeom = RpGeometryLock(pGeom, rpGEOMETRYLOCKPRELIGHT);
	
	if (interpVal < 0.0f)
		interpVal = 0.0f;
		
	if (interpVal > 1.0f)
		interpVal = 1.0f;
	

	const float table1Scale = interpVal;
	const float table2Scale = (1.0f - interpVal);
	
	// fill entries in preLitLum table with interpolated values from day / night tables
	for(int32 i=0; i<tableSize; i++)
	{
		pColourTable[i].red		= (pNightTable[i].red	* table1Scale)+ (pDayTable[i].red	* table2Scale);
		pColourTable[i].blue	= (pNightTable[i].blue  * table1Scale)+	(pDayTable[i].blue	* table2Scale);
		pColourTable[i].green	= (pNightTable[i].green * table1Scale)+ (pDayTable[i].green * table2Scale);
		pColourTable[i].alpha	= (pNightTable[i].alpha * table1Scale)+ (pDayTable[i].alpha * table2Scale);
	}
	
	RpGeometryUnlock(pGeom);

}// end of UpdateAtomicPrelitLum()...









/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//------------ ExtraVertColour plugin stuff----------
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//
// extra vertex color table ("Night Color") attached to geometry:
//
RwBool CCustomBuildingDNPipeline::ExtraVertColourPluginAttach()
{
	ms_extraVertColourPluginOffset = -1;

	ms_extraVertColourPluginOffset =  RpGeometryRegisterPlugin(	EXTRAVERTCOLOUR_SIZE,
																MAKECHUNKID(rwVENDORID_ROCKSTAR, rwEXTRAVERTCOL_ID),
																pluginExtraVertColourConstructorCB,
																pluginExtraVertColourDestructorCB,
																NULL);
	
	if(ms_extraVertColourPluginOffset != -1)
	{
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

//
//
//
//
void* CCustomBuildingDNPipeline::pluginExtraVertColourConstructorCB(void* object, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	RpGeometry *pGeom = (RpGeometry*)object;
	uint8 **ppInfo = NULL;
	
	if(ms_extraVertColourPluginOffset > 0)
	{
		ppInfo = (uint8**)GETPLUGINBASE(pGeom);
		memset(ppInfo, 0, EXTRAVERTCOLOUR_SIZE);
	}
	
	return(object);
}


//
//
//
//
void* CCustomBuildingDNPipeline::pluginExtraVertColourDestructorCB(void* object, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	RpGeometry *pGeom = (RpGeometry*)object;

	RwRGBA* pTable = EXTRAVERT_NIGHTTABLE(pGeom);
	if(pTable)
	{
		GtaFree(pTable);
		EXTRAVERT_NIGHTTABLE(pGeom) = NULL;
	}
	
	pTable = EXTRAVERT_DAYTABLE(pGeom);
	if(pTable)
	{
		GtaFree(pTable);
		EXTRAVERT_DAYTABLE(pGeom) = NULL;
	}

	return(object);
}


//
//
//
//
//
RwStream* CCustomBuildingDNPipeline::pluginExtraVertColourStreamWriteCB(RwStream *pStream, RwInt32 len, const void *pData, RwInt32 offset, RwInt32 size)
{	
#ifndef GTA_XBOX
	RpGeometry* pGeom = (RpGeometry*)pData;
	uint8** ppInfo = (uint8**)EXTRAVERT_NIGHTTABLE(pData);

	RwStreamWrite(pStream, ppInfo, sizeof(uint8*));

	uint8* pInfo = *ppInfo;
	if(!pInfo)
	{
		return(pStream);
	}

	RwStreamWrite(pStream, pInfo, sizeof(uint8) * RpGeometryGetNumVertices(pGeom) * 4);
#endif

	return(pStream);
}



//
//
//
//
RwInt32 CCustomBuildingDNPipeline::pluginExtraVertColourStreamGetSizeCB(const void* pData, RwInt32 offset, RwInt32 size)
{
	RpGeometry* pGeom = (RpGeometry*)pData;
	if(!pGeom)
		return(0);

	// the stream only contains the night prelit data
	const uint32 uiSize = 4 + (sizeof(uint8) * RpGeometryGetNumVertices(pGeom) * 4 * 2);

	return(uiSize);
}

//
//
//
//
RwStream* CCustomBuildingDNPipeline::pluginExtraVertColourStreamReadCB(RwStream *pStream, RwInt32 len, void *pData, RwInt32 offset, RwInt32 size)
{
	RpGeometry* pGeom = (RpGeometry*)pData;
	CDNGeomExtension*	pGeomExtension = (CDNGeomExtension*)GETPLUGINBASE(pData);

	// this ptr to the DN table isn't used by PC - we have a more complex structure for the plugin
	uint8*	trash;
	
	RwStreamRead(pStream, &trash, sizeof(uint8*));
	if(!trash)
	{
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
	
//!PC
//disable all lighting if you want vertex alpha to work
//	UInt32 flags = RpGeometryGetFlags(pGeom);
//	RpGeometrySetFlags(pGeom, (flags & ~(rpGEOMETRYLIGHT)));
//
	// copy colour table (if there is one)
	if (pColourTable)
	{
		RwRGBA* pDayTable = (RwRGBA*)EXTRAVERT_DAYTABLE(pData);
		RwRGBA* pNightTable = (RwRGBA*)EXTRAVERT_NIGHTTABLE(pData);
		
		for(int32 i=0; i<tableSize; i++)
		{
			// on PC some of the atomics behave oddly unless I force the alpha up.
			// might be OK to only force alpha up if at a low value?
//			pNightTable[i].alpha = 10;

			pDayTable[i].red = pColourTable[i].red;
			pDayTable[i].blue = pColourTable[i].blue;
			pDayTable[i].green = pColourTable[i].green;
			pDayTable[i].alpha = pColourTable[i].alpha;
		}
	}

	return(pStream);
}


//
// returns pointer to second set of vertex colours (if one exists):
//
uint8* CCustomBuildingDNPipeline::GetExtraVertColourPtr(RpGeometry *pGeom)
{
	ASSERT(ms_extraVertColourPluginOffset >0);
	return((uint8*)EXTRAVERT_NIGHTTABLE(pGeom));
}

//
// returns pointer to second set of vertex colours (if one exists):
//
float* CCustomBuildingDNPipeline::GetExtraVertDNValue(RpGeometry *pGeom)
{
	ASSERT(ms_extraVertColourPluginOffset >0)
	return(&EXTRAVERT_CURRENTDN(pGeom));
}



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
void CCustomBuildingDNPipeline::SetFxEnvTexture(RpMaterial *pMaterial, RwTexture *fxTexture)
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
RwTexture* CCustomBuildingDNPipeline::GetFxEnvTexture(RpMaterial *pMaterial)
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
void CCustomBuildingDNPipeline::SetFxEnvScale(RpMaterial *pMaterial, float envScaleX, float envScaleY)
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
float CCustomBuildingDNPipeline::GetFxEnvScaleX(RpMaterial *pMaterial)
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
float CCustomBuildingDNPipeline::GetFxEnvScaleY(RpMaterial *pMaterial)
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
void CCustomBuildingDNPipeline::SetFxEnvTransScl(RpMaterial *pMaterial, float envTransX, float envTransY)
{
	ASSERT(pMaterial);

	CustomEnvMapPipeMaterialData *pData = CCustomCarEnvMapPipeline::DuplicateCustomEnvMapPipeMaterialData(GETENVMAPPIPEDATAPTRPTR(pMaterial));
	if(!pData)
		return;

	pData->SetfEnvTransSclX(envTransX);
	pData->SetfEnvTransSclY(envTransY);

}

float CCustomBuildingDNPipeline::GetFxEnvTransSclX(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfEnvTransSclX());
}

float CCustomBuildingDNPipeline::GetFxEnvTransSclY(RpMaterial *pMaterial)
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
void CCustomBuildingDNPipeline::SetFxEnvShininess(RpMaterial *pMaterial, float envShininess)
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
float CCustomBuildingDNPipeline::GetFxEnvShininess(RpMaterial *pMaterial)
{
	ASSERT(pMaterial);
	CustomEnvMapPipeMaterialData *pData = GETENVMAPPIPEDATAPTR(pMaterial);
	if(!pData)
		return(0.0f);

	return(pData->GetfShininess());
}
#endif //GTA_PC




