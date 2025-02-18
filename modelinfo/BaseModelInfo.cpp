//
//
//    Filename: BaseModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Base class for all model info classes
//
//
#include "BaseModelInfo.h"
#include "TimeModelInfo.h"
#include "ModelInfo.h"
#include "2dEffect.h"
#include "TempColModels.h"
#include "TxdStore.h"


//float CBaseModelInfo::ms_lodDistScale = 1.0f;

CBaseModelInfo::CBaseModelInfo() : 
m_numRefs(0),
m_txdIndex(-1)
{}

void CBaseModelInfo::Init()
{
	m_numRefs = 0;
	m_txdIndex = -1;
	m_pColModel = NULL;
	m_n2dEffects = -1;
	m_num2dEffects = 0;
	m_dynamicIndex = -1;
	m_flags = SIMPLE_BACKFACE_CULLING;
	m_lodDistance = 2000.0f;
	m_pRwObject = NULL;

	SetOwnsColModel(true);
}

void CBaseModelInfo::Shutdown()
{
	ASSERT(m_numRefs == 0);
	DeleteRwObject();
	DeleteCollisionModel();

	SetOwnsColModel(true);
	m_n2dEffects = -1;
	m_num2dEffects = 0;
	m_dynamicIndex = -1;
	m_txdIndex = -1;
}

void CBaseModelInfo::SetTexDictionary(const char* pName)
{
	m_txdIndex = CTxdStore::FindTxdSlot(pName);
	if(m_txdIndex == -1)
		m_txdIndex = CTxdStore::AddTxdSlot(pName);
}

void CBaseModelInfo::ClearTexDictionary()
{
	m_txdIndex = -1;
}

void CBaseModelInfo::AddTexDictionaryRef()
{
	ASSERT(m_txdIndex != -1);	// modelinfo is referencing a txd
	CTxdStore::AddRef(m_txdIndex);	
}

void CBaseModelInfo::RemoveTexDictionaryRef()
{
	ASSERT(m_txdIndex != -1);	// modelinfo is referencing a txd
	CTxdStore::RemoveRef(m_txdIndex);	
}

void CBaseModelInfo::AddRef()
{
	m_numRefs++;
	AddTexDictionaryRef();
}

void CBaseModelInfo::RemoveRef()
{
	m_numRefs--;
	ASSERT(m_numRefs>=0);
	RemoveTexDictionaryRef();
}

void CBaseModelInfo::SetColModel(CColModel* pColModel, bool bOwnColModel)
{	
	m_pColModel = pColModel; 
	SetOwnsColModel(bOwnColModel);
	
	if(bOwnColModel)
	{	
		// If found time model then attach collision to the other time model as well
		CTimeInfo* pTimeInfo = GetTimeInfo();
		if(pTimeInfo)
		{
			int32 otherModel = pTimeInfo->GetOtherTimeModel();
			
			if(otherModel != -1)
			{
				CBaseModelInfo* pOtherModel = CModelInfo::GetModelInfo(otherModel);
				ASSERT(pOtherModel);
				pOtherModel->SetColModel(pColModel, false);
			}	
		}
	}	
}

void CBaseModelInfo::DeleteCollisionModel()
{
	if(m_pColModel && GetOwnsColModel())
	{
		delete m_pColModel;
	}
	m_pColModel = NULL;
}

void CBaseModelInfo::Init2dEffects() 
{
	m_n2dEffects = -1;
	m_num2dEffects = 0;
}
/*
int32 CBaseModelInfo::GetNum2dEffects() 
{
	RpGeometry* pGeom = NULL;
	
	int32 num2dEffects = m_num2dEffects;

	if(GetRwModelType() == rpATOMIC && GetRwObject())
	{
		RpAtomic* pAtomic = (RpAtomic*)GetRwObject();
		pGeom = RpAtomicGetGeometry(pAtomic);
		ASSERT(pGeom);
		
		num2dEffects += RpGeometryGetNum2dEffects(pGeom);	
	}

	return num2dEffects;
}
*/
C2dEffect* CBaseModelInfo::Get2dEffect(int32 i) 
{
	RpGeometry* pGeom = NULL;
	int32 num2dEffects = m_num2dEffects;
		
	ASSERTMSG(i < num2dEffects, "2d effect index out of range"); 
	
	if(GetRwModelType() == rpATOMIC && GetRwObject())
	{
		RpAtomic* pAtomic = (RpAtomic*)GetRwObject();
		pGeom = RpAtomicGetGeometry(pAtomic);
		ASSERT(pGeom);
		
		num2dEffects -= RpGeometryGetNum2dEffects(pGeom);	
	}
	else if (GetRwModelType() == rpCLUMP && GetRwObject())
	{
		RpAtomic* pAtomic = Get2DEffectAtomic((RpClump*)GetRwObject());
		if(pAtomic)
		{
			pGeom = RpAtomicGetGeometry(pAtomic);
			ASSERT(pGeom);
		
			num2dEffects -= RpGeometryGetNum2dEffects(pGeom);
		}
	}	
		
	if (i >= num2dEffects)
	{
		return RpGeometryGet2dEffect(pGeom, i - num2dEffects);
	}
	else
	{
		ASSERT(m_n2dEffects >= 0);
		C2dEffect	*pEff = CModelInfo::Get2dEffectStore().FirstElement();
		return &pEff[m_n2dEffects+i];
	}

	return NULL;
}

void CBaseModelInfo::Add2dEffect(C2dEffect* pEffect)
{

	// if this is the first 2d effect to be added
	if(m_n2dEffects < 0)
	{
		m_n2dEffects = pEffect - CModelInfo::Get2dEffectStore().FirstElement();
		m_num2dEffects = 1;
		return;
	}	

	m_num2dEffects++;
}


#ifndef FINAL

// Callback for atomic texture size ratio with the dictionary (singular) it uses
RpAtomic* AtomicGetTexSizesCB(RpAtomic *pAtomic, void *data)
{
	RpGeometry* pGeom = RpAtomicGetGeometry(pAtomic);
	ASSERT(pGeom);
    RpMaterialList *pMatList = &pGeom->matList;
    ASSERT(pMatList);			    
	RwTexDictionary *pDict;
	int32 totalSize = 0;
	int32 dictionarySize = 0;
	
	for (int32 i=0;i< pMatList->numMaterials ;i++)
	{
		RpMaterial *pMat = pMatList->materials[i];
		RwTexture *pTex = pMat->texture; 
		if (!pTex) // some materials have no texture
			continue;
	    RwRaster *pRas =  pTex->raster;
 	   	pDict = pTex->dict;
 	   	int32 width  = pRas->width;
 	   	int32 height = pRas->height;
 	   	int32 depth  = pRas->depth;
 	   	int32 size   = width * height * depth;
 	   	totalSize    += size;					
	}	
	
	if (pDict) //possible to have atomic without texdicts.
	{	
		// Now find the total size of the dictionary
		RwLinkList *pList = &pDict->texturesInDict;				
	    RwLLLink   *cur = rwLinkListGetFirstLLLink(pList);
	    RwLLLink   *end = rwLinkListGetTerminator(pList);

	    while (cur != end)
	    {
	        RwTexture *pResult = rwLLLinkGetData(cur, RwTexture, lInDictionary);
		    RwRaster *pRas =  pResult->raster;
	 	   	ASSERT(pDict == pResult->dict);
	 	   	int32 width  = pRas->width;
	 	   	int32 height = pRas->height;
	 	   	int32 depth  = pRas->depth;
	 	   	int32 size   = width * height * depth;
	 	   	dictionarySize    += size;									
	        cur = rwLLLinkGetNext(cur);
	    }		
	    
	    ((CTwoFloats*)data)->a += totalSize;
	    ((CTwoFloats*)data)->b += dictionarySize;
	}
	    
	return(pAtomic);
}

//-------------------------------------------------------------------------------------------------
// returns the textures used by a model as a percentage of all the models in the dictionary it uses
// assumes only one dictionary per model
float CBaseModelInfo::GetTexPercentage()
{	
	CTwoFloats sizes;
	sizes.a = 0;
	sizes.b = 0;

	if (GetRwObject())
	{
		int32 modelType = GetRwModelType();
		if(modelType == rpATOMIC)
		{
			AtomicGetTexSizesCB((RpAtomic*)GetRwObject(),(void*)&sizes);
		}
		else if (modelType == rpCLUMP)
		{
			RpClumpForAllAtomics((RpClump*)GetRwObject(), AtomicGetTexSizesCB, (void*)&sizes);
		}
	}	
	
	// The sizes are are actually expressed in bits due to different texture depths. - as a ratio this does not matter.
	float percentage = (sizes.a/sizes.b) * 100.0f;	
	return percentage;
}

#endif
