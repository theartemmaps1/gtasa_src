#ifndef SHADERDESC_H
#define SHADERDESC_H

#define maxDirLights (16)
#define maxPointLights (16)
#define maxShaderConstantSlots (192)

enum
{
    _rpSkinXboxShaderDesc_matfxnull = 0,
    _rpSkinXboxShaderDesc_matfxbump = 1,
    _rpSkinXboxShaderDesc_matfxenv = 2,
    _rpSkinXboxShaderDesc_matfxbumpenv = 3,
    _rpSkinXboxShaderDesc_matfxdual = 4,
    _rpSkinXboxShaderDesc_matfxuvanim = 5,
    _rpSkinXboxShaderDesc_matfxdualuvanim = 6,
    _rpSkinXboxShaderDesc_matfxmtxbox = 7
};

typedef struct _rpSkinXboxShaderDesc
/* list the properties of this shader */
{
    RwUInt8 numWeights;
    RwUInt8 numDirect;
    RwUInt8 numPoint;
    RwUInt8 matfx : 4;
    RwUInt8 bnt : 3;
    RwUInt8 prelit : 1;

} _rpSkinXboxShaderDesc;

#define IS_SHADERDESC_EQUAL(desc1, desc2) (*((const RwUInt32 *)(&(desc1))) == (*((const RwUInt32 *)(&(desc2)))))
#define IS_SHADERDESC_LESS(desc1, desc2) (*((const RwUInt32 *)(&(desc1))) < (*((const RwUInt32 *)(&(desc2)))))

extern const DWORD *
_rpSkinXboxGetShader(const _rpSkinXboxShaderDesc *desc);


#endif
