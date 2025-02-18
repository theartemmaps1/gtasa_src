//
//
//    Filename: LodAtomicModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a simple model( consists of one atomic)
//
//
#include "LodAtomicModelInfo.h"
#include "ModelInfo.h"
#include "TimeModelInfo.h"
#include "TxdStore.h"
#include "camera.h"
#include "general.h"
#include "globals.h"
#include "renderer.h"
#include "AnimManager.h"



//
//        name: CLodAtomicModelInfo::Init
// description: Initialise Simple object values
//
void CLodAtomicModelInfo::Init()
{
	CAtomicModelInfo::Init();
//	m_pRelated = NULL;
}

//
// name:		CLodAtomicModelInfo::SetupBigBuilding()
// description:	Do all the big building setup
//
/*void CLodAtomicModelInfo::Setup(int32 minIndex, int32 maxIndex)
{
	CAtomicModelInfo* pOtherModel;
	
	if(m_lodDistance <= ISBIGBUILDINGDIST)
		return;
		
	if(GetRelatedModel())
		return;
		
	FindRelatedModel(minIndex, maxIndex);
	pOtherModel = GetRelatedModel();
	if(pOtherModel != NULL)
	{
		SetNearDistance(pOtherModel->GetLodDistanceUnscaled());
	}
	// this is different from GTA3. Now big buildings that don't have a related hi detail building
	// don't have a near distance
	else				
		SetNearDistance(0.0f);
}*/

//
// name:		CLodAtomicModelInfo::FindRelatedModel()
// description:	Find the model related to this one. Used specifically for
//				big buildings. So they know which buildings they are replacing
//
/*void CLodAtomicModelInfo::FindRelatedModel(int32 minIndex, int32 maxIndex)
{
	for(int32 i=minIndex; i<=maxIndex; i++)
	{
		CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(i);
		if(pModelInfo == this)
			continue;
		if(pModelInfo &&
			!CGeneral::faststrcmp(GetModelName()+3, pModelInfo->GetModelName()+3))
		{
			if(pModelInfo->AsAtomicModelInfoPtr())
				m_pRelated = pModelInfo->AsAtomicModelInfoPtr();
			// don't load collision for this model as it uses the collision from the hi-detail	
			m_bOwnColModel = false;
			break;
		}
	}	
}*/

