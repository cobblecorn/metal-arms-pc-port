//////////////////////////////////////////////////////////////////////////////////////
// AIZombieBoss.cpp -code for Boss AI
//
// Author: Michael Scholz
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/05/03 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "AIZombieBoss.h"
#include "AIMain.h"
#include "AIFSM.h"
#include "AIThoughts3DBot.h"
#include "AIThoughtsGeneric.h"
#include "AIThoughtsGround.h"
#include "AINodePools.h"
#include "AIMover.h"
#include "AIBrain.h"
#include "AIGameUtils.h"
#include "AITactics.h"
#include "../BotZombieBoss.h"
#include "player.h"

static const float _fMaxTimeOnPath = 1.50f; // seconds

static const float _fGrabCloseEnoughDistance = 9.0f; 
static const float _fGrabGiveUpDistance		 = 25.f; 
static const float _fSmashGiveUpDistance	 = 60.f; 
static const float _fFishGiveUpDistance		 = 80.f; 
static const float _fLurchGiveUpDistance	 = 150.f; 

static const float _fSmashCosineHalfAngle = 0.9f; // near cos 26 degrees 

CZombieBossCombat::CZombieBossCombat(void)
 : CGroundCombat()
{
}


CZombieBossCombat::~CZombieBossCombat(void)
{
}


void CZombieBossCombat::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /*= NULL*/)
{
	CGroundCombat::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	m_zb.pZombieBoss = NULL;
	m_zb.pEnemyBot = NULL;		
	m_zb.fTimeSpentOnPath=0.0f;
	m_zb.uCurrPathReason=0;
	m_zb.uNextPathReason=1;
}


void CZombieBossCombat::Cleanup(void)
{
	CGroundCombat::Cleanup();
}


void CZombieBossCombat::Work(void)
{
	CGroundCombat::Work();	   //will do some setup, then call Ruleset_ZombieBoss0 (once per frame)
}


f32 CZombieBossCombat::DistanceToEnemy(void)
{
	FASSERT(m_zb.pEnemyBot);
	FASSERT(m_zb.pZombieBoss);
	return m_zb.pEnemyBot->MtxToWorld()->m_vPos.DistXZ(m_zb.pZombieBoss->MtxToWorld()->m_vPos);
}

void CZombieBossCombat::SetSpeed(u8 uSpeed)
{
	m_pBrain->PopBaseSpeedSetting();
	m_pBrain->PushBaseSpeedSetting(uSpeed);
}

void CZombieBossCombat::HandlePathing(BOOL bStop)
{
	u8 uNextPathSlot;
	u8 uCurrPathSlot;

	if (bStop || m_zb.pZombieBoss->ShouldNotPursue()) // free up our path resources, not needed
	{
		if (IsFollowingPath(TRUE,this->m_zb.uCurrPathReason))
		{
			FindRequestSlotInUse(this->m_zb.uCurrPathReason,&uCurrPathSlot);
			FreeRequestSlot(uCurrPathSlot);
		}
		if (IsFollowingPath(TRUE,this->m_zb.uNextPathReason))
		{
			FASSERT(0); // why'd this state happen?
			FindRequestSlotInUse(this->m_zb.uNextPathReason,&uNextPathSlot);
			FreeRequestSlot(uNextPathSlot);
		}
		return;
	}
	
	BOOL bRequested = FALSE;
	BOOL bPathBusy = FindRequestSlotInUse(m_zb.uCurrPathReason);
	if (bPathBusy) // means we're on it
	{
		m_zb.fTimeSpentOnPath += FLoop_fPreviousLoopSecs;
	}
	else // we're not on the current path
	{
		bRequested = RequestSearch(m_zb.pEnemyBot->MtxToWorld()->m_vPos, m_zb.uCurrPathReason, 1.0f+((f32)m_uRangePathFailureCount));
	}
	
	if (!bRequested && (m_zb.fTimeSpentOnPath > _fMaxTimeOnPath)) // request next path
	{
		if (FindRequestSlotInUse(m_zb.uNextPathReason)==FALSE)
		{
			bRequested = RequestSearch(m_zb.pEnemyBot->MtxToWorld()->m_vPos, m_zb.uNextPathReason, 1.0f+((f32)m_uRangePathFailureCount));
		}
	}

	if (m_zb.fBotDistanceToGlitch > 10.f) 
	{
		CFVec3A AdvancePt;
		BOOL bRequested = FALSE;
		if (m_uRangePathFailureCount < 5)
		{
			
		}
		else if (m_uRangePathFailureCount < 10 )
		{

		}
	}

	if (!IsFollowingPath(TRUE, m_zb.uCurrPathReason))
	{
		// not on a path?
		if (IsSearchDone(m_zb.uCurrPathReason,&uCurrPathSlot))
		{
			Attack_AssignPath(uCurrPathSlot);
			SetSpeed(m_zb.pZombieBoss->GetSpeedForAI());
			m_zb.fTimeSpentOnPath = 0.0f;
		}
		else if (IsSearchFailed(m_zb.uCurrPathReason,&uCurrPathSlot))
		{
			u16 uPathStatus = m_aSearchQuery[uCurrPathSlot].GetStatus();
			m_uRangePathFailureCount++;
			FreeRequestSlot(uCurrPathSlot);
			// if we can't find our next path, try, try again...
			bRequested = RequestSearch(m_zb.pEnemyBot->MtxToWorld()->m_vPos, m_zb.uCurrPathReason, 1.0f+((f32)m_uRangePathFailureCount));
		}
	}
	
	if (IsSearchDone(m_zb.uNextPathReason, &uNextPathSlot)) // implies success
	{
		if (FindRequestSlotInUse(m_zb.uCurrPathReason, &uCurrPathSlot))
		{
			FreeRequestSlot(uCurrPathSlot);
		}
		Attack_AssignPath(uNextPathSlot);
		SetSpeed(m_zb.pZombieBoss->GetSpeedForAI());
		m_zb.fTimeSpentOnPath = 0.0f;
		u8 temp = m_zb.uCurrPathReason;
		m_zb.uCurrPathReason = m_zb.uNextPathReason;
		m_zb.uNextPathReason = temp;
	}
	else if (IsSearchFailed(m_zb.uNextPathReason,&uNextPathSlot))
	{
		u16 uPathStatus = m_aSearchQuery[uNextPathSlot].GetStatus();
		m_uRangePathFailureCount++;
		FreeRequestSlot(uNextPathSlot);
		// if we can't find our next path try again, if not searching on 1,,
		if (!bRequested)
		{
			bRequested = RequestSearch(m_zb.pEnemyBot->MtxToWorld()->m_vPos, m_zb.uNextPathReason, 1.0f+((f32)m_uRangePathFailureCount));
		}
	}
	
	if (m_bPathJustCompleted)
	{
		if (FindRequestSlotInUse(m_zb.uCurrPathReason, &uCurrPathSlot))
		{
			FreeRequestSlot(uCurrPathSlot);
		}
	}
}


BOOL InitRuleSet_ZombieBoss0(u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTZOMBIEBOSS ) ) 
	{
		pT->m_zb.pZombieBoss = (CBotZombieBoss*) _pWD->pBot;
	}
	FASSERT( pT->m_zb.pZombieBoss);

	// My enemy is player zero, always...
	CEntity* pEntity = Player_aPlayer[0].m_pEntityCurrent;
	FASSERT(pEntity->TypeBits() & ENTITY_BIT_BOT);
	CBot* pBot = (CBot*)pEntity;
	pT->m_zb.pEnemyBot = pBot;

	pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIEBOSS_SEARCH);
	return AIFSM_INIT_CB_DONE;
}
BOOL DoRuleSet_ZombieBoss0(u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->m_zb.fBotDistanceToGlitch = pT->DistanceToEnemy();
	pT->m_RuleSetStateFSM.Work();
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward() (cleared every frame)
	return AIFSM_WORK_CB_RETURN;
}

BOOL InitRule_ZombieBoss_Search( u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	return AIFSM_WORK_CB_RETURN;
}
BOOL RuleWork_ZombieBoss_Search( u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	if (_pWD->bHasLOSThisFrame)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SMASH);
		pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	}

	return AIFSM_WORK_CB_RETURN;
}

BOOL InitRule_ZombieBoss_Grab( u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	return AIFSM_WORK_CB_RETURN;
}

BOOL RuleWork_ZombieBoss_Grab( u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	// First test for state exit conditions
	if (_pWD->bHasLOSThisFrame==FALSE)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SEARCH);
		pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	}
	else if (pT->m_zb.fBotDistanceToGlitch > _fGrabGiveUpDistance)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SMASH);
	}

	CFVec3A vGrabPtWS, vGoodSpotToGrab, vSelfMinusGrabPt;
	if (!pT->m_zb.pZombieBoss->ComputeGrabPointWS(vGrabPtWS))
		return AIFSM_WORK_CB_RETURN;

	FASSERT(pT->m_zb.pEnemyBot);

	vSelfMinusGrabPt.Sub(pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos,vGrabPtWS);
	vSelfMinusGrabPt.y =0;

	// CDebugDraw::Sphere(vGrabPtWS, _fGrabCloseEnoughDistance);

	if (vGrabPtWS.DistXZ(pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos) < _fGrabCloseEnoughDistance)
	{
		// do a grab
		_pWD->pMover->m_Controls.Fire2();
	}

	// Next, handle pathing
	vGoodSpotToGrab.Add(vSelfMinusGrabPt,pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos);
	CAIGraph* pGraph = aimain_pGraphSearcher->GetGraph();

	GraphVert *pClosestVert = pGraph->FindClosestLOSVert2D(vGoodSpotToGrab, TRUE);
	if ( (pT->m_zb.fBotDistanceToGlitch < _fGrabGiveUpDistance) && pClosestVert)
	{
		pT->HandlePathing(TRUE); // stop pathing
		// try to line up the grab...
		if ( (pT->m_zb.pZombieBoss->ShouldNotPursue()==FALSE) )
		{
			_pWD->pMover->SimpleMoveToward(vGoodSpotToGrab);

/*
			vGoodSpotToGrab.y += 1.0f;
			CDebugDraw::Line(pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos,vGoodSpotToGrab,&FColor_MotifGreen,&FColor_MotifRed);
			CDebugDraw::Sphere(vGoodSpotToGrab,0.5f,&FColor_MotifRed);
			CFColorRGBA cyan;cyan.OpaqueCyan();
			CDebugDraw::Sphere(pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos,2.5f,&cyan);
*/
		}
		_pWD->pMover->FaceToward(_pWD->pEnemyBot->MtxToWorld()->m_vPos);
		_pWD->pMover->m_Controls.AimAt( _pWD->pEnemyBot->MtxToWorld()->m_vPos );
	}
	else
	{
		pT->HandlePathing();
		_pWD->pMover->FaceToward(_pWD->pEnemyBot->MtxToWorld()->m_vPos);
		_pWD->pMover->m_Controls.AimAt( _pWD->pEnemyBot->MtxToWorld()->m_vPos );
	}
	return AIFSM_WORK_CB_RETURN;
}

BOOL InitRule_ZombieBoss_Smash(u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	return AIFSM_WORK_CB_RETURN;
}
BOOL RuleWork_ZombieBoss_Smash(u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	// First test for state exit conditions
	if (_pWD->bHasLOSThisFrame==FALSE)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SEARCH);
		pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	}
	else if (pT->m_zb.fBotDistanceToGlitch > _fSmashGiveUpDistance)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_FISH);
	}
	else if (pT->m_zb.fBotDistanceToGlitch < _fGrabGiveUpDistance)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_GRAB);
	}
	
	CFVec3A vUnitToTarget,vUnitFront;
	vUnitToTarget.x  = pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos.x - pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos.x;
    vUnitToTarget.y =0.f;
	vUnitToTarget.z  = pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos.z - pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos.z;
	vUnitToTarget.UnitizeXZ();

	vUnitFront = pT->m_zb.pZombieBoss->m_MountUnitFrontXZ_WS;

	f32 fCosAngleToTarget =	(vUnitToTarget.x *
							vUnitFront.x) +
							(vUnitToTarget.z *
							vUnitFront.z); 
	
	if (fCosAngleToTarget > _fSmashCosineHalfAngle)
	{
		// do a swing
		pT->m_zb.pZombieBoss->SetBallTargetWS(pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos);
		_pWD->pMover->m_Controls.Fire();
	}

	// Next, handle pathing
	if (pT->m_zb.fBotDistanceToGlitch < _fGrabGiveUpDistance)
		pT->HandlePathing(TRUE);
	else
		pT->HandlePathing();

	// Don't forget your face toward!
	_pWD->pMover->m_uMoverFlags &= ~CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
	_pWD->pMover->FaceToward(_pWD->pEnemyBot->MtxToWorld()->m_vPos);
	_pWD->pMover->m_Controls.AimAt( _pWD->pEnemyBot->MtxToWorld()->m_vPos );

	return AIFSM_WORK_CB_RETURN;
}

BOOL InitRule_ZombieBoss_Fish( u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	return AIFSM_WORK_CB_RETURN;
}
BOOL RuleWork_ZombieBoss_Fish( u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	// First test for state exit conditions
	if (pT->m_fTimeWithoutLOS>1.0f)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SEARCH);
		pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	}
	else if (pT->m_zb.fBotDistanceToGlitch > _fFishGiveUpDistance)
	{
		pT->m_zb.fLurchTimeCounter = fmath_RandomFloatRange(2.0f,7.0f); // only reset counter from this side of FSM
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_LURCH);
	}
	else if (pT->m_zb.fBotDistanceToGlitch < _fSmashGiveUpDistance)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SMASH);
	}
	
	CFVec3A vUnitToTarget,vUnitFront;
	vUnitToTarget.x  = pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos.x - pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos.x;
    vUnitToTarget.y =0.f;
	vUnitToTarget.z  = pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos.z - pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos.z;
	vUnitToTarget.UnitizeXZ();

	vUnitFront = pT->m_zb.pZombieBoss->m_MountUnitFrontXZ_WS;

	f32 fCosAngleToTarget =	(vUnitToTarget.x *
							vUnitFront.x) +
							(vUnitToTarget.z *
							vUnitFront.z); 

	if (fCosAngleToTarget > _fSmashCosineHalfAngle)
	{
		// do a swing
		pT->m_zb.pZombieBoss->SetBallTargetWS(pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos);
		_pWD->pMover->m_Controls.Fire();
	}

	// Next, handle pathing
	pT->HandlePathing(TRUE);

	// Don't forget your face toward!
	_pWD->pMover->FaceToward(_pWD->pEnemyBot->MtxToWorld()->m_vPos);
	_pWD->pMover->m_Controls.AimAt( _pWD->pEnemyBot->MtxToWorld()->m_vPos );
	return AIFSM_WORK_CB_RETURN;
}

BOOL InitRule_ZombieBoss_Lurch(u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	return AIFSM_WORK_CB_RETURN;
}

BOOL RuleWork_ZombieBoss_Lurch(u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	// First test for state exit conditions
	if (_pWD->bHasLOSThisFrame==FALSE)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SEARCH);
		pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	}
	else if (pT->m_zb.fBotDistanceToGlitch > _fLurchGiveUpDistance)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_GOTO);
	}
	else if (pT->m_zb.fBotDistanceToGlitch < _fFishGiveUpDistance)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_FISH);
	}
	f32 fLTimeOld = pT->m_zb.fLurchTimeCounter;
	pT->m_zb.fLurchTimeCounter -= FLoop_fPreviousLoopSecs;
	
	if ((fLTimeOld > 0.0f) && pT->m_zb.fLurchTimeCounter < 0.0f) // done lurching, pause;
	{
		pT->HandlePathing(TRUE);
	}
	else if (pT->m_zb.fLurchTimeCounter < -1.0f) // done lurching, pause;
	{
		CFVec3A vUnitToTarget,vUnitFront;
		vUnitToTarget.x  = pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos.x - pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos.x;
		vUnitToTarget.y =0.f;
		vUnitToTarget.z  = pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos.z - pT->m_zb.pZombieBoss->MtxToWorld()->m_vPos.z;
		vUnitToTarget.UnitizeXZ();

		vUnitFront = pT->m_zb.pZombieBoss->m_MountUnitFrontXZ_WS;

		f32 fCosAngleToTarget =	(vUnitToTarget.x *
			vUnitFront.x) +
			(vUnitToTarget.z *
			vUnitFront.z); 

		if (fCosAngleToTarget > _fSmashCosineHalfAngle)
		{
			// do a swing
			pT->m_zb.pZombieBoss->SetBallTargetWS(pT->m_zb.pEnemyBot->MtxToWorld()->m_vPos);
			_pWD->pMover->m_Controls.Fire();
			pT->m_zb.fLurchTimeCounter = fmath_RandomFloatRange(2.0f,7.0f);
		}
	}
	else if (pT->m_zb.fLurchTimeCounter > 0.0f)
	{
		pT->HandlePathing();
	}


	// Don't forget your face toward!
	_pWD->pMover->FaceToward(_pWD->pEnemyBot->MtxToWorld()->m_vPos);
	_pWD->pMover->m_Controls.AimAt( _pWD->pEnemyBot->MtxToWorld()->m_vPos );

	return AIFSM_WORK_CB_RETURN;
}

BOOL InitRule_ZombieBoss_Goto(  u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	return AIFSM_WORK_CB_RETURN;
}
BOOL RuleWork_ZombieBoss_Goto(  u32 uControl, void* pParam1, void* pParam2)
{
	CZombieBossCombat* pT = (CZombieBossCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	// First test for state exit conditions
	if (_pWD->bHasLOSThisFrame==FALSE)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_SEARCH);
		pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	}
	else if (pT->m_zb.fBotDistanceToGlitch < _fLurchGiveUpDistance)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_ZOMBIEBOSS_LURCH);
	}
	
	// Next, handle pathing
	pT->HandlePathing();

	// Don't forget your face toward!
	_pWD->pMover->FaceToward(_pWD->pEnemyBot->MtxToWorld()->m_vPos);
	_pWD->pMover->m_Controls.AimAt( _pWD->pEnemyBot->MtxToWorld()->m_vPos );

	return AIFSM_WORK_CB_RETURN;
}

void CGroundCombat::DoWeaponAimAndFireWork_ZombieBoss(void)
{
	//called once per frame by CGroundCombat, after rulesetwork

	// TODO, evaluate and press buttons
}





