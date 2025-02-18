///////////////////////////////////////////////////////////////////////////////
//  
//	FILE: 	"Ragdoll.cpp"
//	BY	: 	Mark Nicholson
//	FOR	:	Rockstar North
//	ON	:	07 Oct 2003 
//	WHAT:	Routines to handle ragdoll physics
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//  INCLUDES
///////////////////////////////////////////////////////////////////////////////
/*
// system
#include	"rpAnimBlend.h"

// user
#include	"Ragdoll.h"
#include	"PlayerPed.h"


///////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
///////////////////////////////////////////////////////////////////////////////

RagdollManager_c	g_ragdollMan;


///////////////////////////////////////////////////////////////////////////////
//  CLASS Ragdoll_c
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//  Constructor
///////////////////////////////////////////////////////////////////////////////
	
				Ragdoll_c::Ragdoll_c				()
{}


///////////////////////////////////////////////////////////////////////////////
//  Destructor
///////////////////////////////////////////////////////////////////////////////
	
				Ragdoll_c::~Ragdoll_c				()
{}


///////////////////////////////////////////////////////////////////////////////
//  Init
///////////////////////////////////////////////////////////////////////////////
	
bool8			Ragdoll_c::Init						(CPed* pPed)
{
	// store the ped pointer
	m_pPed = pPed;
	
	// stop this ped from being updated via the animation system	
	CAnimBlendClumpData *pAnimBlendClump = ANIMBLENDCLUMPFROMCLUMP(m_pPed->GetRwObject());
	AnimBlendFrameData*	frameData = pAnimBlendClump->GetFrameDataArray();
	float* pVel = pAnimBlendClump->GetVelocityPtr();

	for (int32 i=0; i<pAnimBlendClump->GetNumberOfFrames(); i++)
	{
		frameData[i].Flags |= rpFRAMEROTCAPTURED;
		frameData[i].Flags |= rpFRAMETRANSCAPTURED;
	}
	
	pVel[0] = pVel[1] = pVel[2] = 0;

	// set up the bone node data
	RpHAnimHierarchy* 	pHierarchy = GetAnimHierarchyFromSkinClump((RpClump *)m_pPed->GetRwObject());

	for (int32 i=0; i<BONETAG_MAX_NUM; i++)
	{		
		int32 boneTag = aBONETAG_ENUM_TAB[i];
		int32 boneId  = RpHAnimIDGetIndex(pHierarchy, boneTag);
		RpHAnimBlendInterpFrame* pAnimKeyFrame = frameData[boneId].pStdKeyFrame;	

		m_boneNodes[i].Init(boneTag, pAnimKeyFrame);
	}
	
	// set up the bone hierarchy
	SetupBoneHierarchy();	

	m_blend = 1.0f;
	
	// apply a test force to the head
//	RwV3d dir = {0.0f, 0.0f, 1.0f};
//	for (int32 i=0; i<BONETAG_MAX_NUM; i++)
//	{
//		ApplyForce(aBONETAG_ENUM_TAB[i], CGeneral::GetRandomNumberInRange(5.0f, 10.0f), dir);
//	}
	
	return true;
}


///////////////////////////////////////////////////////////////////////////////
//  Exit
///////////////////////////////////////////////////////////////////////////////
	
void			Ragdoll_c::Exit						()
{
	// give this ped control back 
	CAnimBlendClumpData *pAnimBlendClump = ANIMBLENDCLUMPFROMCLUMP(m_pPed->GetRwObject());
	AnimBlendFrameData*	frameData = pAnimBlendClump->GetFrameDataArray();	

	for (int32 i=0; i<pAnimBlendClump->GetNumberOfFrames(); i++)
	{
		frameData[i].Flags ^= rpFRAMEROTCAPTURED;
		frameData[i].Flags ^= rpFRAMETRANSCAPTURED;
	}

	// let the ped be rendered again
	m_pPed->m_nPedFlags.bDontRender = false;
}


///////////////////////////////////////////////////////////////////////////////
//  Update
///////////////////////////////////////////////////////////////////////////////
	
void			Ragdoll_c::Update					(float deltaTime)
{
	// update the bone nodes	
	m_pRootBoneNode->CalcWldMat(m_pPed->GetRwMatrix());

	ResolveSystem(deltaTime);
	
	// limit the bone rotations
//	for (int32 i=1; i<BONETAG_MAX_NUM; i++)
//	{
//		m_boneNodes[i].Limit();
//	}

	// blend into the animation
	for (int32 i=0; i<BONETAG_MAX_NUM; i++)
	{	
		m_boneNodes[i].BlendKeyframe(m_blend);
	}
}


///////////////////////////////////////////////////////////////////////////////
//  SetBlend
///////////////////////////////////////////////////////////////////////////////
	
void			Ragdoll_c::SetBlend					(float blend)
{
	m_blend = blend;
}


///////////////////////////////////////////////////////////////////////////////
//  ApplyForce
///////////////////////////////////////////////////////////////////////////////
	
void			Ragdoll_c::ApplyForce				(int32 boneTag, float force, RwV3d dir)
{
	// get curr bone node
	BoneNode_c* pBoneNode = &m_boneNodes[BoneNode_c::GetIdFromBoneTag(boneTag)];
	
	// update it's velocity
	pBoneNode->m_vel.x += dir.x*force;
	pBoneNode->m_vel.y += dir.y*force;
	pBoneNode->m_vel.z += dir.z*force;
}


///////////////////////////////////////////////////////////////////////////////
//  ResolveSystem
///////////////////////////////////////////////////////////////////////////////
	
void			Ragdoll_c::ResolveSystem			(float deltaTime)
{
	printf("INFO: RAGDOLL - ResolveSystem()\n");

	// apply gravity to bone nodes	
	for (int32 i=0; i<BONETAG_MAX_NUM; i++)
	{
		m_boneNodes[i].m_vel.z -= 9.81f*deltaTime;
	}
	
	// update the bone node rotations 
	for (int32 i=0; i<BONETAG_MAX_NUM; i++)
	{
		if (m_boneNodes[i].m_boneTag != BONETAG_HEAD)
		{
			continue;
		}	
	
		// calculate vector from parent node to this node
		RwV3d parentToNodeVec;
		RwV3dSub(&parentToNodeVec, RwMatrixGetPos(&m_boneNodes[i].m_wldMat), RwMatrixGetPos(&m_boneNodes[i].m_parent->m_wldMat));

		// get the cross product of this and the velocity vector
		RwV3d axis;
		RwV3dCrossProduct(&axis, &parentToNodeVec, &m_boneNodes[i].m_vel);

		// calculate the rotational speed
		float speed = RwV3dNormalize(&axis, &axis);
		
		if (speed > 0.01f)
		{
			// put this axis into parent space
			RtQuat parentQuat, invParentQuat;
			if (m_boneNodes[i].m_parent)
			{
				RtQuatConvertFromMatrix(&parentQuat, &m_boneNodes[i].m_parent->m_wldMat);
			}
			else
			{
				RtQuatConvertFromMatrix(&parentQuat, m_pPed->GetRwMatrix());	
			}
			
			RtQuatReciprocal(&invParentQuat, &parentQuat);
			RwV3d relativeAxis;
			RtQuatTransformVectors(&relativeAxis, &axis, 1, &invParentQuat);
			
			printf("%.2f %.2f %.2f %.2f - parent quat\n", parentQuat.imag.x, parentQuat.imag.y, parentQuat.imag.z, parentQuat.real);
			printf("%.2f %.2f %.2f %.2f - inv parent quat\n", invParentQuat.imag.x, invParentQuat.imag.y, invParentQuat.imag.z, invParentQuat.real);

			// apply the rotation to the current bone's parent
			RtQuatRotate(&m_boneNodes[i].m_parent->m_keyFrameQuat, &relativeAxis, speed, rwCOMBINEPOSTCONCAT);		

			// limit the rotation
//			m_boneNodes[i].Limit();

			// update the world matrices down the tree from the current bone 
			if (m_boneNodes[i].m_parent)
			{
				m_boneNodes[i].CalcWldMat(&m_boneNodes[i].m_parent->m_wldMat);	
			}
			else
			{
				m_boneNodes[i].CalcWldMat(m_pPed->GetRwMatrix());	
			}
		}
	}
	
	// apply friction to the bone node velocities
	for (int32 i=0; i<BONETAG_MAX_NUM; i++)
	{
		m_boneNodes[i].m_vel.x *= 0.98f;
	}
}


///////////////////////////////////////////////////////////////////////////////
//  SetupBoneHierarchy
///////////////////////////////////////////////////////////////////////////////
	
void			Ragdoll_c::SetupBoneHierarchy		()
{
	// get a pointer to the root bone node
	m_pRootBoneNode = &m_boneNodes[aBONETAG_ENUM_TAB[BONETAG_ROOT]];
	
	// create the bone hierarchy
	for (int32 i=0; i<BONETAG_MAX_NUM; i++)
	{
		if (BoneNodeManager_c::ms_boneInfos[i].parentBoneTag > -1)
		{
			m_boneNodes[BoneNode_c::GetIdFromBoneTag(BoneNodeManager_c::ms_boneInfos[i].parentBoneTag)].AddChild(&m_boneNodes[i]);
		}
	}

#ifndef FINAL
	// output the bone tree
	m_pRootBoneNode->OutputNodeInfo(0);
#endif
}


#ifndef FINAL

///////////////////////////////////////////////////////////////////////////////
//  RenderDebug
///////////////////////////////////////////////////////////////////////////////
	
void			Ragdoll_c::RenderDebug				()
{
	if (BoneNodeManager_c::ms_renderDebugInfo)
	{
		// render the blended animation
		m_pRootBoneNode->RenderDebug();
	}
}

#endif






///////////////////////////////////////////////////////////////////////////////
//  CLASS RagdollManager_c
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//  Constructor
///////////////////////////////////////////////////////////////////////////////
	
				RagdollManager_c::RagdollManager_c	()
{}


///////////////////////////////////////////////////////////////////////////////
//  Destructor
///////////////////////////////////////////////////////////////////////////////
	
				RagdollManager_c::~RagdollManager_c	()
{}


///////////////////////////////////////////////////////////////////////////////
//  Init
///////////////////////////////////////////////////////////////////////////////
	
bool8			RagdollManager_c::Init				()
{
	// add the ragdolls to the pool
	for (int32 i=0; i<NUM_RAGDOLLS; i++)
	{
		m_ragdollPool.AddItem(&m_ragdolls[i]);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//  Exit
///////////////////////////////////////////////////////////////////////////////
	
void			RagdollManager_c::Exit				()
{	
	// delete any active ragdolls	
	Ragdoll_c* pRagdoll = (Ragdoll_c*)m_ragdollList.GetHead();

	while (pRagdoll)
	{
		pRagdoll->Exit();
		pRagdoll = (Ragdoll_c*)m_ragdollList.GetNext(pRagdoll);
	}
	
	m_ragdollList.RemoveAll();
	m_ragdollPool.RemoveAll();
}


///////////////////////////////////////////////////////////////////////////////
//  Reset
///////////////////////////////////////////////////////////////////////////////
	
void			RagdollManager_c::Reset				()
{	
	Exit();
	Init();
}


///////////////////////////////////////////////////////////////////////////////
//  Update
///////////////////////////////////////////////////////////////////////////////
	
void			RagdollManager_c::Update			(float deltaTime)
{	
	Ragdoll_c* pRagdoll = (Ragdoll_c*)m_ragdollList.GetHead();

	while (pRagdoll)
	{
		pRagdoll->Update(deltaTime);
		
		pRagdoll = (Ragdoll_c*)m_ragdollList.GetNext(pRagdoll);
	}
}


///////////////////////////////////////////////////////////////////////////////
//  AddRagdoll
///////////////////////////////////////////////////////////////////////////////
	
Ragdoll_c*			RagdollManager_c::AddRagdoll	(CPed* pPed)
{
	Ragdoll_c* pRagdoll = (Ragdoll_c*)m_ragdollPool.RemoveHead();
	
	if (pRagdoll)
	{
		pRagdoll->Init(pPed);	
		m_ragdollList.AddItem(pRagdoll);
	}
	
	return pRagdoll;
}


///////////////////////////////////////////////////////////////////////////////
//  RemoveRagdoll
///////////////////////////////////////////////////////////////////////////////

void			RagdollManager_c::RemoveRagdoll		(Ragdoll_c* pRagdoll)
{
	pRagdoll->Exit();
	m_ragdollList.RemoveItem(pRagdoll);
	m_ragdollPool.AddItem(pRagdoll);
}	



#ifndef FINAL

///////////////////////////////////////////////////////////////////////////////
//  RenderDebug
///////////////////////////////////////////////////////////////////////////////
	
void			RagdollManager_c::RenderDebug		()
{
	if (BoneNodeManager_c::ms_renderDebugInfo)
	{
		Ragdoll_c* pRagdoll = (Ragdoll_c*)m_ragdollList.GetHead();

		while (pRagdoll)
		{
			pRagdoll->RenderDebug();
			pRagdoll = (Ragdoll_c*)m_ragdollList.GetNext(pRagdoll);
		}
	}
}

#endif
*/