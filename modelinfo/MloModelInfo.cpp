//
//
//    Filename: MloModelInfo.cpp
//     Creator: Adam Fowler
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Class describing a MLO model. A MLO(Multi level object) model consists of an array of simple
//				objects at predefined positions
//
//
#include "MloModelInfo.h"
#include "Instance.h"
#include "SimpleModelInfo.h"
#include "ModelInfo.h"
#include "VisibilityPlugins.h"


//
//        name: CMloModelInfo::ConstructClump
// description: Construct a RenderWare clump to represent the MLO
//          in: 
//         out: 
//
void CMloModelInfo::ConstructClump()
{
	int32 i;
	RwFrame* pClumpFrame;
	RpAtomic* pAtomic;
	RwFrame* pAtomicFrame;
	RwMatrix* pMatrix;

	ASSERT(m_instStart < m_instEnd);
	
	// construct clump and frame
	m_pClump = RpClumpCreate();
	pClumpFrame = RwFrameCreate();
	RwFrameSetIdentity(pClumpFrame);
	RpClumpSetFrame(m_pClump, pClumpFrame);

	// for all the instance in this model create an atomic and a frame
	for(i = m_instStart; i < m_instEnd; i++)
	{
		int32 index = CModelInfo::GetMloInstanceStore()[i].GetIndex();
		RwMatrix* pInstMatrix = CModelInfo::GetMloInstanceStore()[i].GetMatrix();
		CSimpleModelInfo* pModelInfo = (CSimpleModelInfo*)CModelInfo::GetModelInfo(index);

		ASSERT(pModelInfo->GetModelType() == MI_TYPE_SIMPLE || pModelInfo->GetModelType() == MI_TYPE_TIME);

		pAtomic = pModelInfo->GetAtomic(0);
		if(pAtomic)
		{
			// clone the atomic, create a frame, clone the matrix
			RpAtomic* pAtomicClone = RpAtomicClone(pAtomic);
			pAtomicFrame = RwFrameCreate();

			// if anything failed to create then ignore this atomic
			if(pAtomicClone == NULL || pAtomicFrame == NULL)
			{
				CDebug::DebugLog("Failed to allocate memory while creating template MLO.\n");
				continue;
			}
			pMatrix = RwFrameGetMatrix(pAtomicFrame);
			RwMatrixCopy(pMatrix, pInstMatrix);
			RpAtomicSetFrame(pAtomicClone, pAtomicFrame);
			RwFrameAddChild(pClumpFrame, pAtomicFrame);
			RpClumpAddAtomic(m_pClump, pAtomicClone);
		}
						
	}
	if(RpClumpGetNumAtomics(m_pClump) == 0)
	{
		RpClumpDestroy(m_pClump);
		m_pClump = NULL;
	}
	else
	{
		CVisibilityPlugins::SetClumpModelInfo(m_pClump, this);
	}
}


