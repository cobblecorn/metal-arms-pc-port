//////////////////////////////////////////////////////////////////////////////////////
// AIFormations.cpp - 
//
// Author: Pat MacKellar 
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/05/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIFormations.h"
#include "AIBrain.h"
#include "AIMover.h"
#include "AIThoughtsGeneric.h"
#include "AIGameUtils.h"
#include "../Entity.h"
#include "flinklist.h"
#include "fres.h"
#include "fdraw.h"

FLinkRoot_s* _pPtrNodePool = NULL;
static CNiBank<CAIFormation>* _pFormationBank = NULL;
static CNiBank<CAIPost>* _pPostBank = NULL;
const u8 kMAX_NUM_POSTS_PER_FORMATION = 20;
static u8 _auPostSortBuff[kMAX_NUM_POSTS_PER_FORMATION];
CAIPost CNiIterator<CAIPost>::s_ReturnError;
CAIFormation* CNiIterator<CAIFormation*>::s_ReturnError;
CAIPost* CNiIterator<CAIPost*>::s_ReturnError;

BOOL AIFormations_InitSystem(	struct FLinkRoot_s* pGlobalPtrNodePool,
								u32 uFormationBankSize/* = AIFORMATION_DEFAULT_GLOBAL_FORMATION_BANK_SIZE*/,
								u32 uPostBankSize /*= AIFORMATION_DEFAULT_GLOBAL_POST_BANK_SIZE*/)
{
	_pPtrNodePool = pGlobalPtrNodePool;

	FASSERT(_pFormationBank == NULL && _pPostBank == NULL);

	FResFrame_t ResFrame = fres_GetFrame();

	_pFormationBank = APE_NEW CNiBank<CAIFormation>(pGlobalPtrNodePool, uFormationBankSize, APE_NEW CAIFormation[uFormationBankSize]);
	if (!_pFormationBank)
	{
		goto _ExitInitFormationsWithError;
	}

	_pPostBank = APE_NEW CNiBank<CAIPost>(pGlobalPtrNodePool, uPostBankSize, APE_NEW CAIPost[uPostBankSize]);
	if (!_pPostBank)
	{
		goto _ExitInitFormationsWithError;
	}
	
	return TRUE;

_ExitInitFormationsWithError:
	DEVPRINTF( "AIFormations_InitSystem(): Out of memory for pete's sake!.\n" );
	fres_ReleaseFrame( ResFrame );
	_pFormationBank = NULL;
	_pPostBank = NULL;

	return FALSE;


}

void AIFormations_UninitSystem(void)
{
	FASSERT(!_pFormationBank || _pFormationBank->IsFull());  //Has the bank been fully restored?
	APE_DELETE(_pFormationBank); _pFormationBank = NULL;

	FASSERT(_pPostBank->IsFull());  //Has the bank been fully restored?
	APE_DELETE(_pPostBank); _pPostBank = NULL;
}


void AIFormations_Work(void)
{
   //formations work gets called by the owner of the formation see (CAIBrain::Work())
}


CAIFormation* AIFormations_Get(void)
{
	return _pFormationBank->Get();
}

void AIFormations_Recycle(CAIFormation* pFormation)
{
	if (pFormation && _pFormationBank)
	{
		pFormation->Cleanup();
		_pFormationBank->Recycle(pFormation);
	}
}


void CAIPost::Reset(void)
{
	m_uPostFlags = POSTFLAG_NONE;
}



CAIFormation::CAIFormation(void)
:	m_uFormationType(0)
{
	m_FormationMtx.Identity();
}


CAIFormation::~CAIFormation(void)
{
	Cleanup();
}


CAIPost* CAIFormation::Join(CAIBrain* pBrain)
{
	return NULL;
}


BOOL CAIFormation::Quit(CAIBrain* pBrain)
{
	return NULL;
}


void CAIFormation::_ClearData(void)
{
	m_uFormationType = FORMATIONTYPE_INVALID;
	m_pLeaderBrain = NULL;
	for (u32 i = 0; i < NUM_FORMATION_POSTS; i++)
	{
		m_apPostAssignment[i] = NULL;
	}
}


void CAIFormation::Init(u8 uFormationType, CAIBrain* pLeaderBrain)
{
	m_uFormationType = uFormationType;
	m_pLeaderBrain = pLeaderBrain;

	switch (m_uFormationType)
	{
		case FORMATIONTYPE_GLITCHBUDDIES:
			m_FormationMtx.Identity();	
			m_FormationMtx.m_vPos.x = FMATH_MAX_FLOAT; //force slots to be evaluated in first formation::Work

			//default settings
			m_fFormationMarkerResetRad = 25.0f;
			break;
	
	}
}


BOOL CAIFormation::HasAssignment(CAIBrain* pFollower, u32* puPost)
{
	for (u32 i = 0; i < NUM_FORMATION_POSTS; i++)
	{
		if (m_apPostAssignment[i] == pFollower)
		{
			if (puPost)
			{
			   *puPost = i;
			}
			return TRUE;
		}
	}

	return FALSE;
}


void CAIFormation::Work(void)
{
	CAIMover *pLeaderMover = m_pLeaderBrain->GetAIMover();
	CBot* pLeaderBot =NULL;
	if (pLeaderMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
	{
		pLeaderBot = (CBot*) pLeaderMover->GetEntity();
	}
	u32 i;
	CNiIterator<CAIPost> it;

	switch (m_uFormationType)
	{
		case FORMATIONTYPE_GLITCHBUDDIES:


//			if (m_pLeaderBrain->GetFlag_Just_Stopped())
			if (m_FormationMtx.m_vPos.DistSq(m_pLeaderBrain->GetAIMover()->GetEntity()->MtxToWorld()->m_vPos) > m_fFormationMarkerResetRad*m_fFormationMarkerResetRad)
			{
				//
				// Change Orientation of the formation whenever the leader moves a certain threshold
				//

				//Order around the ring (clockwise) in which the formation will be filled up.
				u32 auRingSlotYawRot[NUM_FORMATION_POSTS] = {3,	 //right wingman
															5,	 //left wingman
															2,	 //right wingman (second class)
															6};	 //left wingman (second class)
				CFVec3A aRingSlot[NUM_FORMATION_POSTS];
				f32 fFormDistBase = 15.0f;
				f32 fMaxFollowerRad = 3.0f;//pMover->GetRadiusXZ();

				f32 fRingRad = fMaxFollowerRad + fFormDistBase + pLeaderMover->GetRadiusXZ();

				if (pLeaderBot->GetCurMech() && pLeaderBot->GetCurMech()->AIBrain())
				{
					fRingRad += pLeaderBot->GetCurMech()->AIBrain()->GetAIMover()->GetRadiusXZ();
				}

				for (i = 0; i < NUM_FORMATION_POSTS; i++)
				{
					m_aPostVec_MS[i] = pLeaderMover->GetTorsoLookAtXZ();
					f32 fRotRad = ((f32)auRingSlotYawRot[i])*FMATH_2PI/8.0f;//fmath_Div(FMATH_2PI, (f32)uNumSiblings);
					m_aPostVec_MS[i].RotateY(fRotRad);
					m_aPostVec_MS[i].Mul(fRingRad);
					aRingSlot[i].Add(m_aPostVec_MS[i], pLeaderMover->GetLoc());
				}


				//
				//  Now decide which followers should get assigned the posts
				//
				for (i = 0; i < NUM_FORMATION_POSTS; i++)
				{
					m_apPostAssignment[i] = NULL;
				}

				it = m_pLeaderBrain->m_FollowerList.Begin();
				while (it.IsValid())
				{
					CAIPost* pPost = NULL;
					if (it.Get(&pPost))
					{   //this form of get isn't const
						pPost->Reset();
					}
					it.Next();
				}

				//Assignments are based on Proximity to post
				//with a constrain that straight-shot patj to the post 
				//won't go through the player
				for (i = 0; i < NUM_FORMATION_POSTS; i++)
				{

					f32 fClosestDistSq = -1.0f;
					CAIPost* pClosest = NULL;

					it = m_pLeaderBrain->m_FollowerList.Begin();
					while (it.IsValid())
					{
						CAIPost* pFollowerPost = NULL;
						if (it.Get(&pFollowerPost))
						{
							CAIBrain* pFollower = pFollowerPost->m_pFollower;

							if (!pFollower->GetFlag_Follower_Doesnt_Need_Post() &&
								!HasAssignment(pFollower))
							{
								
								CFVec3A PostVec_WS;
								PostVec_WS.Sub(aRingSlot[i], pLeaderMover->GetLoc());
								CFVec3A LeaderToFollower;
								LeaderToFollower.Sub(pFollower->GetAIMover()->GetLoc(), pLeaderMover->GetLoc());
								f32 fPostVecToFollowerDot = LeaderToFollower.Dot(PostVec_WS);
								f32 fDistSq = aRingSlot[i].DistSq(pFollower->GetAIMover()->GetLoc());
								LeaderToFollower.y = 0.0f;
								if ((LeaderToFollower.Dot(pLeaderMover->GetTorsoLookAtXZ()) < 0.0f || fPostVecToFollowerDot > 0.0f) &&
									(!pClosest || fDistSq < fClosestDistSq))
								{
									pClosest = pFollowerPost;
									fClosestDistSq = fDistSq;
								}
							}
						}
						it.Next();
					}

					if (pClosest)
					{
						m_apPostAssignment[i] = pClosest->m_pFollower;
						if (pClosest)
						{
							pClosest->m_Pos_WS = aRingSlot[i].v3;
							pClosest->m_uPostFlags |= CAIPost::POSTFLAG_FORMATIONSLOT;
						}
					}
				}
				m_FormationMtx = *(m_pLeaderBrain->GetAIMover()->GetEntity()->MtxToWorld());
			}
			else
			{	   //just move the formation posts, don't rotate them
				it = m_pLeaderBrain->m_FollowerList.Begin();
				while (it.IsValid())
				{
					CAIPost* pFollowerPost = NULL;
					if (it.Get(&pFollowerPost))
					{
						u32 uPost = 0;
						if (HasAssignment(pFollowerPost->m_pFollower,  &uPost))
						{
							pFollowerPost->m_Pos_WS = m_aPostVec_MS[uPost].v3;

							pFollowerPost->m_Pos_WS.Add(m_pLeaderBrain->GetAIMover()->GetLoc().v3);
					//		FASSERT(pFollowerPost->m_uPostFlags & CAIPost::POSTFLAG_FORMATIONSLOT);
						}
						else
						{
					//		FASSERT(!(pFollowerPost->m_uPostFlags & CAIPost::POSTFLAG_FORMATIONSLOT));
						}
					}
					it.Next();
				}
			}
			break;
	}
}

void CAIFormation::DebugRender(void)
{
	CFVec3A DrawThisPt;
	CNiIterator<CAIPost> it;
/*	for (u32 i = 0; i < NUM_FORMATION_POSTS; i++)
	{
		DrawThisPt = m_aPostVec_MS[i];
		fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 1, aiutils_paDebugColors+AICOLOR_RED);
	}
	*/

	it = m_pLeaderBrain->m_FollowerList.Begin();
	while (it.IsValid())
	{
		CAIPost* pPost = NULL;
		if (it.Get(&pPost))
		{   //this form of get isn't const
			DrawThisPt.v3 = pPost->m_Pos_WS;
			DrawThisPt.v3.y +=5.0f;
			u8 uColor = AICOLOR_RED;

			if (pPost->m_uPostFlags & CAIPost::POSTFLAG_FORMATIONSLOT)
			{
				uColor = AICOLOR_GREEN;

				fdraw_SolidLine( &(DrawThisPt.v3), &(pPost->m_pFollower->GetAIMover()->GetEntity()->MtxToWorld()->m_vPos.v3), aiutils_paDebugColors+uColor);
			}

			fdraw_FacetedWireSphere( &(DrawThisPt.v3), 1.0f, 1, 1, aiutils_paDebugColors+uColor);
		}
		it.Next();
	}



}


void CAIFormation::Cleanup(void)
{
}

