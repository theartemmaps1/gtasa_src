//
// VuGrassRenderer - interface class for drawing plants with VU1;
//
//
//	30/07/2004	-	John:	- initial;
//	10/04/2005	-	Andrzej:	- full port to GTA_PC/GTA_XBOX;
//
//
//
//
//
//
#ifndef __VUGRASSRENDERER_H__
#define __VUGRASSRENDERER_H__


#include "Dma.h"
#include "vector.h"
#include "camera.h"
#include "color.h"	//CRGBA()
#include "Vector2D.h"
#include "PC_PlantsMgr.h"	//PPPlant{}...


//
//
// 12 = size of input buffer in VU1 microcode renderer:
// 
#define	PPTRIPLANT_BUFFER_SIZE				(32)	//(4*12)


//
//
//
//
#define PPTRIPLANT_MODELS_TAB_SIZE			(4)




//
//
//
//
class CGrassRenderer
{
public:
	CGrassRenderer();
	~CGrassRenderer();

public:
	static bool 		Initialise();
	static void 		Shutdown();

public:
	static void			AddTriPlant(PPTriPlant *pPlant, uint32 ePlantModelSet);
	static void			FlushTriPlantBuffer();

public:
	static RwBool 		DrawTriPlants(PPTriPlant *triPlants, RwInt32 numTris, RpAtomic** plantModelsTab, RwMatrix *pLTM);

private:
	inline void static	GenPointInTriangle(CVector *pRes, const CVector *V1, const CVector *V2, const CVector *V3, float s, float t);


public:
	static bool8		SetPlantModelsTab(uint32 index, RpAtomic **plantModels);
	static RpAtomic**	GetPlantModelsTab(uint32 index);


public:
	static void			SetGlobalCameraPos(const CVector& camPos);
	static void			SetCloseFarAlphaDist(float closeDist, float farDist);
	static void			SetGlobalWindBending(float bending);


public:

	static void			SetCurrentScanCode(uint16 currScanCode);

private:
	static CVector		m_vecCameraPos;
	static float		m_closeDist;
	static float		m_farDist;
	static float		m_windBending;
};




//
//
// 
//
enum
{
	PPPLANTBUF_MODEL_SET0	= 0,
	PPPLANTBUF_MODEL_SET1	= 1,
	PPPLANTBUF_MODEL_SET2	= 2,
	PPPLANTBUF_MODEL_SET3	= 3	
};



//
//
//
//
class CPPTriPlantBuffer
{
public:
	CPPTriPlantBuffer();
	~CPPTriPlantBuffer();

public:
	void			Flush();

	PPTriPlant*		GetPPTriPlantPtr(int32 amountToAdd=1);

	void			ChangeCurrentPlantModelsSet(int32 newSet);
	void			IncreaseBufferIndex(int32 pipeMode, int32 amount=1);


	//
	// current plant models set:
	//
	void			SetPlantModelsSet(int32 set)							{ this->m_plantModelsSet=set;		}
	int32			GetPlantModelsSet()										{ return(this->m_plantModelsSet);	}


public:
	//
	// access to m_pPlantModelsTab[]:
	//
	bool8			SetPlantModelsTab(uint32 index, RpAtomic** pPlantModels);
	RpAtomic**		GetPlantModelsTab(uint32 index);

	
private:
	int32			m_currentIndex;
	PPTriPlant		m_Buffer[PPTRIPLANT_BUFFER_SIZE];
	
	int32			m_plantModelsSet;

	RpAtomic**		m_pPlantModelsTab[PPTRIPLANT_MODELS_TAB_SIZE];

};



//
//
// translated from Andrez's VU code...
//
inline
void CGrassRenderer::GenPointInTriangle(CVector *pRes, const CVector *V1, const CVector *V2, const CVector *V3, float s, float t)
{
	if(s+t > 1.0f)
	{
		s = 1.0f - s;
		t = 1.0f - t;
	}
	
	const float a = 1.0f - s - t;
	const float b = s;
	const float c = t;

	pRes->x = a*V1->x + b*V2->x + c*V3->x;
	pRes->y = a*V1->y + b*V2->y + c*V3->y;
	pRes->z = a*V1->z + b*V2->z + c*V3->z;
}



#endif//__VUGRASSRENDERER_H__


