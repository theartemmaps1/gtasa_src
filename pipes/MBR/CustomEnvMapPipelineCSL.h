//
//	Handy RW MatFX defines taken directly from rwsrc;
//

#ifndef __CUSTOMENVMAPPIPELINECSL_H__
#define __CUSTOMENVMAPPIPELINECSL_H__

#include "../plugin/matfx/effectPipes.h"
#include "../plugin/matfx/mbr/effectPipesMBR.h"

void SetEnvMapTexture(RwTexture* tex, float shininess, float scaleX, float scaleY, RwV2d offset);
void ResetEnvMap();

#endif//__CUSTOMENVMAPPIPELINECSL_H__

