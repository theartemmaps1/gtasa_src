//
//	Handy RW MatFX defines taken directly from rwsrc;
//
//	25/02/2003	-	Andrzej:	- initial;
//	18/06/2004	-	Andrzej:	- UVANIM stuff added;
//	31/01/2005	-	Andrzej:	- D3D9 specific stuff added;
//
//
//
#ifndef __CUSTOMENVMAPPIPELINECSL_H__
#define __CUSTOMENVMAPPIPELINECSL_H__

#if defined(__cplusplus)
	extern "C" {
#endif // __cplusplus

#if defined(GTA_XBOX)
	//
	// taken from rwsdk\plugin\matfx\xbox\effectPipesXbox.h:
	//
	/*===========================================================================*
	*--- Types -----------------------------------------------------------------*
	*===========================================================================*/
	struct MatFXDualData
	{
		RwTexture          *texture;
		RwBlendFunction     srcBlendMode;
		RwBlendFunction     dstBlendMode;

		/*--- device specific ---*/
	};

	struct MatFXBumpMapData
	{
		RwFrame   *frame;
		RwTexture *texture;
		RwTexture *bumpTexture;
		RwReal    coef;
		RwReal    invBumpWidth;
		RwInt32	  vbStride;	
		void      *tex2;
		RwUInt32  bumpVertexShader;
	};

	/*===========================================================================*
	*--- Defines ---------------------------------------------------------------*
	*===========================================================================*/
	#define MATFXXBOXENVMAPGETDATA(material)                                \
	&((*((rpMatFXMaterialData **)                                         \
		(((RwUInt8 *)material) +                                         \
			MatFXMaterialDataOffset)))->data[rpSECONDPASS].data.envMap)

	#define MATFXXBOXBUMPMAPGETDATA(material)                                \
	&((*((rpMatFXMaterialData **)                                         \
		(((RwUInt8 *)material) +                                         \
			MatFXMaterialDataOffset)))->data[rpSECONDPASS].data.bumpMap)

	#define MATFXXBOXDUALPASSGETDATA(material)                                \
	&((*((rpMatFXMaterialData **)                                         \
		(((RwUInt8 *)material) +                                         \
			MatFXMaterialDataOffset)))->data[rpSECONDPASS].data.dual)

	#define MATFXXBOXBUMPENVMAPGETDATA(material)                                \
	&((*((rpMatFXMaterialData **)                                         \
		(((RwUInt8 *)material) +                                         \
			MatFXMaterialDataOffset)))->data[rpTHIRDPASS].data.envMap)

	#define MATFXXBOXUVANIMGETDATA(material)  \
	&((*((rpMatFXMaterialData **)             \
		(((RwUInt8 *)material) +             \
			MatFXMaterialDataOffset)))->data[rpSECONDPASS].data.uvAnim)

	#define MATFXXBOXDUALUVANIMGETDUALDATA(material)  \
	&((*((rpMatFXMaterialData **)                     \
		(((RwUInt8 *)material) +                     \
			MatFXMaterialDataOffset)))->data[rpTHIRDPASS].data.dual)


#endif //GTA_XBOX...


#if defined(GTA_PC)
	//
	// taken from rwsdk\plugin\matfx\d3d9\effectPipesD3D9.h:
	//
	/*===========================================================================*
	*--- Types -----------------------------------------------------------------*
	*===========================================================================*/
	struct MatFXDualData
	{
		RwTexture          *texture;
		RwBlendFunction     srcBlendMode;
		RwBlendFunction     dstBlendMode;
		/*--- device specific ---*/
	};

	struct MatFXBumpMapData
	{
		RwFrame   *frame;
		RwTexture *texture;
		RwTexture *bumpTexture;
		RwReal    coef;
		RwReal    invBumpWidth;
	};

	/*===========================================================================*
	*--- Defines ---------------------------------------------------------------*
	*===========================================================================*/

	#define MATFXD3D9ENVMAPGETDATA(material, pass)                          \
	&((*((rpMatFXMaterialData **)                                         \
		(((RwUInt8 *)material) +                                         \
			MatFXMaterialDataOffset)))->data[pass].data.envMap)

	#define MATFXD3D9BUMPMAPGETDATA(material)                               \
	&((*((rpMatFXMaterialData **)                                         \
		(((RwUInt8 *)material) +                                         \
			MatFXMaterialDataOffset)))->data[rpSECONDPASS].data.bumpMap)

	#define MATFXD3D9DUALGETDATA(material)                                  \
	&((*((rpMatFXMaterialData **)                                         \
		(((RwUInt8 *)material) +                                         \
			MatFXMaterialDataOffset)))->data[rpSECONDPASS].data.dual)

	#define MATFXD3D9UVANIMGETDATA(material)  \
	&((*((rpMatFXMaterialData **)             \
		(((RwUInt8 *)material) +             \
			MatFXMaterialDataOffset)))->data[rpSECONDPASS].data.uvAnim)

	#define MATFXD3D9DUALUVANIMGETDUALDATA(material)  \
	&((*((rpMatFXMaterialData **)                     \
		(((RwUInt8 *)material) +                     \
			MatFXMaterialDataOffset)))->data[rpTHIRDPASS].data.dual)
#endif //GTA_PC...


#ifdef GTA_PS2
	//
	//
	// stolen from rwsdk\plugin\matfx\sky2\effectPipesSky.h:
	//
	struct MatFXDualData
	{
		RwTexture       *texture;
		RwBlendFunction  srcBlendMode;
		RwBlendFunction  dstBlendMode;
		void            *padAlign;
	};

	struct MatFXBumpMapData
	{
		RwFrame   *frame;
		RwTexture *texture;
		RwTexture *bumpTexture;
		RwReal    coef;
		RwReal    invBumpWidth;
	};

	struct MatFXSkyMaterial
	{
		RwUInt64 alpha;
		RwUInt64 fogcol;
		RwUInt64 test;
	};
	typedef struct MatFXSkyMaterial MatFXSkyMaterial; 
#endif //GTA_PS2...




//
//
// stolen from rwsdk\plugin\matfx\effectPipes.h:
//
#define rpMATFXALIGNMENT  rwFRAMEALIGNMENT

#define RPMATFXALIGNMENT(_x) \
   (! (((rpMATFXALIGNMENT)-1) & ((RwUInt32)(_x))))

typedef struct rpMatFXMaterialData RWALIGN(rpMatFXMaterialData, rpMATFXALIGNMENT);


/*===========================================================================*
 *--- Types -----------------------------------------------------------------*
 *===========================================================================*/
enum MatFXPass
{
    rpSECONDPASS = 0,
    rpTHIRDPASS  = 1,
    rpMAXPASS    = 2
};
typedef enum MatFXPass MatFXPass;

typedef struct MatFXBumpMapData RWALIGN(MatFXBumpMapData, rpMATFXALIGNMENT);

typedef struct MatFXEnvMapData RWALIGN(MatFXEnvMapData, rpMATFXALIGNMENT);
struct MatFXEnvMapData
{
    RwFrame   *frame;
    RwTexture *texture;
    RwReal    coef;
    RwBool    useFrameBufferAlpha;
};

typedef struct MatFXUVAnimData RWALIGN(MatFXUVAnimData, rpMATFXALIGNMENT);
struct MatFXUVAnimData
{
    RwMatrix *baseTransform;
    RwMatrix *dualTransform;
};

/*
 * MatFXDualData is device specific as it contains extra data
 * on the PS2 for the blend modes.
 */
typedef struct MatFXDualData RWALIGN(MatFXDualData, rpMATFXALIGNMENT);

typedef union MatFXEffectUnion RWALIGN(MatFXEffectUnion, rpMATFXALIGNMENT);
union MatFXEffectUnion
{
    MatFXBumpMapData  bumpMap;
    MatFXEnvMapData   envMap;
    MatFXDualData     dual;
    MatFXUVAnimData   uvAnim;
    
#if (defined (MULTITEXD3D8_H))
    MatFXD3D8Material d3d8Mat;
#endif
};

typedef struct MatFXEffectData RWALIGN(MatFXEffectData, rpMATFXALIGNMENT);
struct MatFXEffectData
{
    MatFXEffectUnion     data;
    RpMatFXMaterialFlags flag;
    
#if (defined (SKY2_DRVMODEL_H))
    MatFXSkyMaterial     skyMat;
#endif
};

struct rpMatFXMaterialData
{
    MatFXEffectData      data[rpMAXPASS];
    RpMatFXMaterialFlags flags;
};

/*===========================================================================*
 *--- Global Variables ------------------------------------------------------*
 *===========================================================================*/
extern RwInt32 MatFXMaterialDataOffset;

/*===========================================================================*
 *--- Defines ---------------------------------------------------------------*
 *===========================================================================*/
#define MATFXMATERIALGETDATA(material)          \
    ((rpMatFXMaterialData **)                   \
     (((RwUInt8 *)material)+                    \
      MatFXMaterialDataOffset))

#define MATFXMATERIALGETCONSTDATA(material)     \
    ((const rpMatFXMaterialData * const *)      \
     (((const RwUInt8 *)material)+              \
      MatFXMaterialDataOffset))



#ifdef GTA_PS2
	//
	// stolen from rwsdk\plugin\matfx\sky2\effectspipesdriver.h
	//
	#define _rpMatFXSkyResetTextureRaster(_textureA, _textureB)             \
	MACRO_START                                                             \
	{                                                                       \
		if( (NULL != _textureA) && (_textureA->raster->depth <=8) &&        \
			(NULL != _textureB) && (_textureB->raster->depth <=8) )         \
		{                                                                   \
			skyTextureRaster = (RwRaster*)-1;                               \
		}                                                                   \
	}                                                                       \
	MACRO_STOP
#endif //GTA_PS2...


typedef RwReal RWALIGN(matFXEnvArray[4], rpMATFXALIGNMENT);

#if defined(__cplusplus)
	};
#endif // __cplusplus

#endif//__CUSTOMENVMAPPIPELINECSL_H__

