#ifndef SKIN_DEFS_H
#define SKIN_DEFS_H

/* For use in the vertex shader descriptor */
#define VSD_REG_POS                     0
#define VSD_REG_WEIGHTS                 1
#define VSD_REG_INDICES                 2
#define VSD_REG_NORMAL                  3
#define VSD_REG_COLOR                   4
#define VSD_REG_TEXCOORD0               5
#define VSD_REG_TEXCOORD1               6
#define VSD_REG_TEXCOORD2               7
#define VSD_REG_TEXCOORD3               8
#define VSD_REG_TANGENT                 9
#define VSD_REG_BINORMAL                10

/* Input register - For use in the vertex shader code */
#define VSIN_REG_POS                    v0
#define VSIN_REG_WEIGHTS                v1
#define VSIN_REG_INDICES                v2
#define VSIN_REG_NORMAL                 v3
#define VSIN_REG_COLOR                  v4
#define VSIN_REG_TEXCOORD0              v5
#define VSIN_REG_TEXCOORD1              v6
#define VSIN_REG_TEXCOORD2              v7
#define VSIN_REG_TEXCOORD3              v8
#define VSIN_REG_TANGENT                v9
#define VSIN_REG_BINORMAL               v10

/* Temporary register - For use in the vertex shader code */

/* (keep r1 free for pairing up ILU instructions!) */
#define VSTMP_REG_ILU_TMP               r1
#define VSTMP_REG_POS_TMP               r2
#define VSTMP_REG_POS_ACCUM             r3
#define VSTMP_REG_NORMAL_TMP            r4
#define VSTMP_REG_NORMAL_ACCUM          r5
#define VSTMP_REG_CLAMP_TMP             r6
#define VSTMP_REG_DIST_TMP              r7
#define VSTMP_REG_ATTEN_TMP             r8
#define VSTMP_REG_BUMPDIR_TMP           r9
#define VSTMP_REG_TEX                   r10
#define VSTMP_REG_COLOR_TMP             r11

/* Reusing some registers for temp skinning transform */
#define VSTMP_REG_TRANS_0 VSTMP_REG_ATTEN_TMP
#define VSTMP_REG_TRANS_1 VSTMP_REG_BUMPDIR_TMP
#define VSTMP_REG_TRANS_2 VSTMP_REG_TEX

/* Vertex shader defines */

/* bone uses 3 constants slots */
#define SHADERCONSTS_PER_BONE   3

/* per atomic constants as one contiguous block starting
from bottom & growing upwards */
#define PER_ATOMIC_OFFSET               -96

/* Always need a screenspace offset */
#define VSCONST_REG_SCREENSPACE_OFFSET  -96
#define VSCONST_REG_SCREENSPACE_SIZE    1

/* Always need a transform */
#define VSCONST_REG_TRANSFORM_OFFSET    -95
#define VSCONST_REG_TRANSFORM_SIZE      4

/* always need an ambient light */
#define VSCONST_REG_AMBIENT_SIZE        1
#define VSCONST_REG_AMBIENT_OFFSET      -91
 
/* optionally need some directional & point lights */
#define VSCONST_REG_DIR_LIGHT_SIZE      2
#define VSCONST_REG_DIR_LIGHT_OFFSET    -90

#define VSCONST_REG_PNT_LIGHT_SIZE      2

/* bones follow on from the per atomic constants
   Perl & C figure out where they go depending on # lights used... */

/* per material constants as one contiguous block,
starting from top and growing down - we can vary the material
without affect the per atomic constants */

/* always need a material color */
#define VSCONST_REG_MAT_COLOR_SIZE      1
#define VSCONST_REG_MAT_COLOR_OFFSET    95

/* sometimes need some env map coefficients */
#define VSCONST_REG_ENV_SIZE            2
#define VSCONST_REG_ENV_OFFSET          93

/* world transform used for bump mapping */
#define VSCONST_REG_BUMPWORLD_SIZE      3
#define VSCONST_REG_BUMPWORLD_OFFSET    90

/* bump position in xyz and fudge factor in w */
#define VSCONST_REG_BUMPLIGHTRIGHT_SIZE   1
#define VSCONST_REG_BUMPLIGHTRIGHT_OFFSET 89

/* bump shifting fudge factor */
#define VSCONST_REG_BUMPLIGHTUP_SIZE      1
#define VSCONST_REG_BUMPLIGHTUP_OFFSET    88

/* texture transform matrices for MT Xbox */
#define VSCONST_REG_TEXTRANS_OFFSET     79
#define VSCONST_REG_TEXTRANS_SIZE       4

/* Constant register - For use in the vertex shader code */
#define VSCONST_REG_TRANSFORM_X         c[0 + VSCONST_REG_TRANSFORM_OFFSET]
#define VSCONST_REG_TRANSFORM_Y         c[1 + VSCONST_REG_TRANSFORM_OFFSET]
#define VSCONST_REG_TRANSFORM_Z         c[2 + VSCONST_REG_TRANSFORM_OFFSET]
#define VSCONST_REG_TRANSFORM_W         c[3 + VSCONST_REG_TRANSFORM_OFFSET]

#define VSCONST_REG_MAT_COLOR           c[VSCONST_REG_MAT_COLOR_OFFSET]

#define VSCONST_REG_AMBIENT             c[VSCONST_REG_AMBIENT_OFFSET]

#define VSCONST_REG_LIGHT_DIR           c[VSCONST_REG_DIR_LIGHT_OFFSET]


#endif /* SKIN_DEFS_H */
