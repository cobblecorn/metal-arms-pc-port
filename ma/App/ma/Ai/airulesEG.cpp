#include "fang.h"
#include "flinklist.h"
#include "fScriptSystem.h"
#include "ApeNew.h"
#include "AIBrain.h"
#include "AIBrainMems.h"
#include "AiApi.h"
#include "AIBotMover.h"
#include "AIBrain.h"
#include "AITactics.h"
#include "AIThoughtsGround.h"
#include "../BotEliteGuard.h"
#include "../Weapon_staff.h"


enum
{
	EG_STRATEGY_NONE,
	EG_STRATEGY_STUN,	   //hang back, try to stun the enemy before moving in for the stick
	EG_STRATEGY_CHARGE,		   //close any gap fast and then stick him with staff end.
	EG_STRATEGY_HARRASS,	   //stay close to enemy, stick him repeatedly with staff end.
	NUM_EG_STRATEGIES,
};
f32 _kafEliteGuardMinRange[] = {10.0f, 0.0f,  0.0f,  0.0f};
f32 _kafEliteGuardMaxRange[] = {70.0f, 9.0f, 9.0f, 9.0f};

u16 uNextStrategyChangeTimeOut = 0;
u16 uNumStunShots = 0;
u16 uChargeTimeOut = 0;
u8 uCurStrategy = EG_STRATEGY_NONE;
CAIMemoryMedium* pEgAttackDataMem = NULL;

static void _StuffEGAttackParams(	CAIMemoryMedium* pMem,
										u16 uNextStrategyChangeTimeOut,	
										u16 uNumStunShots,	
										u16 uChargeTimeOut,	
										u8 uCurStrategy)				
{
	FASSERT(pMem);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PokeU16(pBuff, uNextStrategyChangeTimeOut);			//2
	pBuff = pMem->PokeU16(pBuff, uNumStunShots);			//2
	pBuff = pMem->PokeU16(pBuff, uChargeTimeOut);			//2
	pBuff = pMem->PokeU8U8(pBuff, uCurStrategy, 0);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryMedium));
}


static void _ExtractEGAttackParams(	CAIMemoryMedium* pMem,
								u16 *puNextStrategyChangeTimeOut,	
								u16 *puNumStunShots,	
								u16 *puChargeTimeOut,	
								u8 *puCurStrategy)				
{
	FASSERT(pMem && puNextStrategyChangeTimeOut && puNumStunShots && puNumStunShots);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PeekU16(pBuff, puNextStrategyChangeTimeOut);				//2
	pBuff = pMem->PeekU16(pBuff, puNumStunShots);					//2
	pBuff = pMem->PeekU16(pBuff, puChargeTimeOut);					//2
	pBuff = pMem->PeekU8U8(pBuff, puCurStrategy, NULL);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryMedium));
}

static void _EGBeginFrame(CAIBrain* pBrain)
{
	pEgAttackDataMem = NULL;
	if ( pBrain->GetKnowledge().CanRememberAny(MEMORY_MEDIUM_ELITEGUARDATTACKDATA, (CAIMemorySmall**) &pEgAttackDataMem))
	{
		_ExtractEGAttackParams(pEgAttackDataMem, &uNextStrategyChangeTimeOut, &uNumStunShots, &uChargeTimeOut, &uCurStrategy);
		pEgAttackDataMem->ResetTimer(2);
	}
	else
	{
		//problem, just put defaults

	}
}

static void _EGEndFrame(CAIBrain* pBrain)
{
	if (pEgAttackDataMem ||
		(pEgAttackDataMem = pBrain->GetKnowledge().RememberMedium(MEMORY_MEDIUM_ELITEGUARDATTACKDATA, 2)))
	{
		_StuffEGAttackParams(pEgAttackDataMem, uNextStrategyChangeTimeOut, uNumStunShots, uChargeTimeOut, uCurStrategy);
	}

}

BOOL InRules_ARS_EliteGuard0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	FASSERT(sizeof(_kafEliteGuardMinRange)/sizeof(f32) == NUM_EG_STRATEGIES);
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	//first state is Base
	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);

	_GARangeSetParams(	pT,
						4,		   //u8 uDodgeTimeMin,   
						20,		   //u8 uDodgeTimeMax,   
						3,		   //u8 uDodgeOutTimeMin,
						5,		   //u8 uDodgeOutTimeMax,
						0,			//u8 uDamageDodgeTimeMin
						_kafEliteGuardMinRange[uCurStrategy],	  //f32 fRangeMin
						_kafEliteGuardMaxRange[uCurStrategy],	  //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);

	_EGEndFrame(pBrain);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_EliteGuard0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	_EGBeginFrame(pBrain);

	//
	// When to change strategy
	//
/*	BOOL uNewStrategy = uCurStrategy;
	u8 uExtraStrategyChangeDelay = 0;

	if (_pWD->pEnemyBot)
	{
		if (_pWD->pEnemyBot->IsSlowed() &&
			pT->m_fTimeWithLOS > 1.0f && 
			uCurStrategy != EG_STRATEGY_CHARGE)
		{
			uNewStrategy = EG_STRATEGY_CHARGE;
			uNextStrategyChangeTimeOut = 0;
		}
		else if (uCurStrategy == EG_STRATEGY_STUN)
		{
			uNewStrategy = EG_STRATEGY_HARRASS;
		}
		else if (uCurStrategy == EG_STRATEGY_HARRASS)
		{
//			uNewStrategy = EG_STRATEGY_STUN;
		}
		else
		{
//			uNewStrategy = EG_STRATEGY_STUN;
			if (fmath_RandomChoice(100) < 50)
			{
				uNewStrategy = EG_STRATEGY_HARRASS;
			}
		}
	}


	if (uNewStrategy != uCurStrategy && uNextStrategyChangeTimeOut < _pWD->uNowTime)
	{
		uCurStrategy = uNewStrategy;
		uNextStrategyChangeTimeOut = uExtraStrategyChangeDelay + aiutils_GetTimeOutRandMinMax((u8)30, 60);

		//reset strategy fires.
		uNumStunShots = 0;
	}
  */


	//
	//
	//	Adjust range based on strategy
	//
	//
	FASSERT(uCurStrategy < NUM_EG_STRATEGIES);
	if(pT->m_TacticFSM.GetActiveStateHandle() ==  CGroundCombat::TACTIC_RANGEATTACK)
	{

		_GARangeSetParams(	pT,
							4,		   //u8 uDodgeTimeMin,   
							20,		   //u8 uDodgeTimeMax,   
							3,		   //u8 uDodgeOutTimeMin,
							5,		   //u8 uDodgeOutTimeMax,
							0,			//u8 uDamageDodgeTimeMin
							_kafEliteGuardMinRange[uCurStrategy],	  //f32 fRangeMin
							_kafEliteGuardMaxRange[uCurStrategy],	  //f32 fRangeMax
							CGroundCombat::ERAPTUSEAGECTRL_NEVER,
							0.0f);
	}


	//if 
	//	 STRATEGY_CHARGE    && 
	//	 got enough space in front of me, 
	//    
	//then
	//   roll because it is faster than running
	//
	CFVec3A NextWayLoc;
	f32 fMaxSafeChargeRoleDist = 10.0f;
	if (uCurStrategy == EG_STRATEGY_CHARGE &&
		uChargeTimeOut < _pWD->uNowTime &&
		pT->m_fTimeWithLOS > 0.5f &&
		_pWD->fDistToEnemyMark > 30.0f)
//		_pWD->pMover->IsFollowingPath() &&
//		_pWD->pMover->m_PathWalker.GetCurLoc(&NextWayLoc))
	{
//		if (NextWayLoc.DistSq(_pWD->pMover->GetLoc()) < fMaxSafeChargeRoleDist*fMaxSafeChargeRoleDist)
		{
			//lunge at player
			_pWD->pMover->m_Controls.Special1();
			uChargeTimeOut = _pWD->uNowTime+ 5+fmath_RandomChoice(5);
		}

	}


	if (_pWD->pBot && _pWD->pBot->IsCapableofAlert())
	{
/*		if (pT->m_TacticFSM.GetActiveStateHandle() == CGroundCombat::TACTIC_CIRCLESTRAFE ||	//circlestrafe tru
			!(pT->m_RuleSetStateFSM.GetActiveStateHandle() == CGroundCombat::ARS_0_SEARCH ||
				_pWD->pBot->IsThrowing() ||	 //can't be alert and throwing at same time
				uCurPathReason == CGroundCombat::SEARCHREASON_NEWATTACKPOS ||
				uCurPathReason == CGroundCombat::SEARCHREASON_TAKINGCOVER ||
				uCurPathReason == CGroundCombat::SEARCHREASON_RETREAT ||
				pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED ||
				pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON	||
				(pT->m_RuleSetStateFSM.GetActiveStateHandle()== CGroundCombat::ARS_0_INVESTIGATEMARK  && uCurPathReason == CGroundCombat::SEARCHREASON_RANGE && _pWD->fDistToEnemyMarkXZ > (15.0f+pT->GetBrain()->GetWorldAddRandom()*10.0f+_pWD->pMover->GetRadiusXZ()))
				))
*/		{
			if (!_pWD->pMover->IsTalkingWithAnim() && !((CBotEliteGuard*) _pWD->pBot)->IsSwinging())
			{
				pT->m_uLastAlertOnDelayTimeOut = _pWD->uNowTime + 4;
				_pWD->pMover->m_Controls.Alert();		//Alert doesn't stick, so dude will only be in alert while in state_base
			}
		}
	}
	

	//give range attack time
	u16 uEliteGuardInvestigateMarkAfterTime = 3 + (u16) 2*(pBrain->GetWorldAddRandom() < 0.5f) - (u16) ((f32)2*_pWD->fAggression);

	//if
	//	 Haven't seen the enemy for some time (aggression is a factor) &&
	//   currently am more than _kfInvestigateMarkMinDist feet from enemymark  (prevents rule from firing repeatedly)
	//then
	//   Investigate the mark
	//
	if (pT->m_fTimeWithoutLOS > (f32)uEliteGuardInvestigateMarkAfterTime &&
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS) &&  //
		_pWD->fDistToEnemyMark >= _kfInvestigateMarkMinDist)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_ELITEGUARD0_INVESTIGATEMARK);
	}

	_EGEndFrame(pBrain);

	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_EliteGuard0_UseCover(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	_EGEndFrame(pBrain);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_EliteGuard0_UseCover(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	_EGEndFrame(pBrain);
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_EliteGuard0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	pT->ChangeTactic(CGroundCombat::TACTIC_PEEKABOO);
	pT->m_uLastPeekABooBailOut = _pWD->uNowTime;  
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GAPeekABooSetParams(pT,						 
						2, 8,						 //timer for when to come out of cover!          (min > max means never)                                                                                  
						2, 4);						 //timer for how long to stay out of cover under normal conditions

	_EGEndFrame(pBrain);
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_EliteGuard0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	_EGEndFrame(pBrain);

	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_EliteGuard0_WatchDog(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	_EGEndFrame(pBrain);
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_EliteGuard0_WatchDog(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	_EGEndFrame(pBrain);
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_EliteGuard0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GARangeSetParams(	pT,
						4,		   //u8 uDodgeTimeMin,   
						0,		   //u8 uDodgeTimeMax,   
						4,		   //u8 uDodgeOutTimeMin,
						0,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						2.0f,	   //f32 fRangeMin,      
						4.0f,	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);

	_EGEndFrame(pBrain);
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_EliteGuard0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	//if
	//   seen the enemy since Range Tactic began.
	//then
	//   go to base state
	if ((u16) pT->m_fTimeWithoutLOS < (_pWD->uNowTime - pT->m_uTacticInitTime) )
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_ELITEGUARD0_BASE);
	}

	//if
	//	 close to the mark but don't see enemy or
	//	 been stuck for some time
	//then
	//   go to search state
	else if ((_pWD->fDistToEnemyMark < _kfInvestigateMarkMinDist) ||
			 !pBrain->HasMovedOneFootSince(_kuInvestigateMarkFailureTime))
	{
		//kick into search mode, then eventually back to our job if no enemy can be found
		pT->ChangeRuleSetState(CGroundCombat::ARS_ELITEGUARD0_SEARCH);
	}

	_EGEndFrame(pBrain);
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_EliteGuard0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);
	pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GASearchSetParams(pT);

	_EGEndFrame(pBrain);
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_EliteGuard0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);
	//
	//If 
	//		sees the enemy
	//then
	//		go back to base state
	//
	if (pT->m_fTimeWithoutLOS < 0.7f)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_ELITEGUARD0_BASE);
	}


	_EGEndFrame(pBrain);
	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_EliteGuard0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	pT->ChangeRuleSetState(CGroundCombat::ARS_ELITEGUARD0_BASE);


	_EGEndFrame(pBrain);
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_EliteGuard0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	//
	//   Give RuleSetState Rules a chance
	//
	pT->m_RuleSetStateFSM.Work();


	//
	//   ignore path finding in certain situations
	//
	_pWD->pMover->m_uMoverFlags &= ~CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
	if (pT->m_pEnemy &&
		_pWD->pBot &&
		_pWD->pBot->TypeBits() & ENTITY_BIT_BOTELITEGUARD &&
		((CBotEliteGuard*) _pWD->pBot)->IsSwinging())
	{
		_pWD->pMover->m_uMoverFlags |= CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
		pT->m_uAttackFlags |= CGroundCombat::FLAG_MANUAL_MOVE_CONTROL;	//cleared every frame

		CFVec3A OverShoot;
		OverShoot.Sub(pT->m_pEnemy->MtxToWorld()->m_vPos, _pWD->pMover->GetLoc());
		OverShoot.y = 0.0f;
		OverShoot.Mul(5.0f);
		OverShoot.Add(pT->m_pEnemy->MtxToWorld()->m_vPos);
		_pWD->pMover->MoveToward(OverShoot,  NULL);	 //try to drive righ through, while swinging
	}

	if (pT->m_uAttackFlags & (CGroundCombat::FLAG_JUST_DID_FAILED_MELEE | CGroundCombat::FLAG_JUST_DID_SUCCESSFUL_MELEE))
	{
		//just finished a mellee attack, set the melee timeOut
		pT->m_uMeleeTimeOut = aiutils_GetTimeOutRandMinMax(pT->m_uMeleeTimeMin, pT->m_uMeleeTimeMax);
	}

	pT->DoGenericTauntWork();

	//
	//  Grenades, etc
	//
	pT->DoGenericEnviroReactRules(	TRUE,  //Grenades
									TRUE,  //Vehicles
									TRUE); //BTA on Damage

	// if
	//		no sight of enemy for a really long time,
	//
	// then 
	//		attack mode fails
	//
	u16 uAttackTimeWithoutLOS = 70 + (u16)(30.0f*_pWD->fAggression)+ (u16) (pBrain->GetWorldAddRandom()*10.0f);
	if ((pT->m_uAttackFlags & CGroundCombat::FLAG_CANTFINDENEMY) &&
		_pWD->uNowTime - pT->m_uLostEnemyTime > uAttackTimeWithoutLOS)
	{
		pT->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;	 //couldn't find that guy!
	}

	
	// if
	//	   can't see enemy this frame &&
	//	   I have seen him in the last 30 seconds
	// then
	//     look at the enemy mark.
	//
	if (!_pWD->bHasLOSThisFrame && 
		pT->m_fTimeWithoutLOS < 30.0f)
	{
		pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_DIR_ENEMY;
	}
	else
	{
		pT->m_uAttackFlags &= ~CGroundCombat::FLAG_TORSO_DIR_ENEMY;
	}

	_EGEndFrame(pBrain);

	return AIFSM_WORK_CB_RETURN;
}


void CGroundCombat::DoWeaponAimAndFireWork_EliteGuard(void)
{
	CGroundCombat* pT = this;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_EGBeginFrame(pBrain);

	//early outs
	if( (m_uAttackFlags & FLAG_PANIC_ON) ||
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsHopping() ||
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsRolling() ||
		m_pBrain->GetAIMover()->IsTalkingWithAnim() ||
		!s_pWD->pMover->CanFirePrimary()) {
		return;
	}
	CAIWeaponCtrl* pWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
	FASSERT(pWeaponCtrl);
	pWeaponCtrl->Work();

	//if
	//	Within MeleeRad and has seen recently ||
	//  we're heading towards each other and withing a slightly puffier melee rad
	//then
	//	swing battle axe
	f32 fPuffyMelee = 20.0f;
	if (m_pEnemy && 
		this->m_fTimeWithoutLOS < 2.0f &&
		(s_pWD->fDistToEnemyMark < (f32) m_uMeleeRad ||
		 (s_pWD->fDistToEnemyMark < fPuffyMelee && ((_pWD->pBot->m_fSpeed_WS > 0.0f && _pWD->pBot->m_VelocityXZ_WS.Dot(_pWD->DeltaToEnemyMarkUnit) > 0.25f) ||
												(_pWD->pEnemyBot->m_fSpeed_WS > 0.0f && _pWD->pEnemyBot->m_VelocityXZ_WS.Dot(_pWD->DeltaToEnemyMarkUnit) < -0.25f)))))
	{
		s_pWD->pMover->m_Controls.Fire2();

		if (m_pEnemy && s_pWD->pEnemyBot)
		{
			CFVec3A EnemyMid;
			EnemyMid = m_pEnemy->MtxToWorld()->m_vPos;
			EnemyMid.y +=s_pWD->pEnemyBot->m_fCollCylinderHeight_WS*0.5f; 
			if (s_pWD->pBot &&
				EnemyMid.y < s_pWD->pBot->MtxToWorld()->m_vPos.y+s_pWD->pBot->m_fCollCylinderHeight_WS)
			{
				CFVec3A FrontAndDown;
				FrontAndDown.Sub(EnemyMid, _pWD->pMover->GetLoc());
				if (FrontAndDown.SafeUnitAndMag(FrontAndDown) > 0.0f)
				{
					m_uAttackFlags |= (FLAG_TORSO_NO_WORK & FLAG_HEAD_NO_WORK);	  //cleared ever frame.
					FrontAndDown.Mul(10.0f);
					FrontAndDown.Add(s_pWD->pMover->GetEntity()->MtxToWorld()->m_vPos);
					FrontAndDown.y = s_pWD->pMover->GetLoc().y-10.0f;
					s_pWD->pMover->FaceToward(FrontAndDown, NULL);
				}
			}
		}

	}
/*	else if ((uCurStrategy == EG_STRATEGY_STUN && s_pWD->fDistToEnemyMark > 15.0f) ||
			 pWeaponCtrl->IsBursting() ||
			 s_pWD->fDistToEnemyMark > 25.0f && m_uRangePathFailureCount > 5)
	{
		
		//
		//  force this guy to aim straight up in the air when firing.
		//
		//	 Note! this is special Torso control during fire work
		//
		m_uAttackFlags |= (FLAG_TORSO_NO_WORK & FLAG_HEAD_NO_WORK);
		CFVec3A FakeUp;
		FakeUp.Sub(_pWD->EnemyMarkTagPos, _pWD->pMover->GetLoc());
		if (FakeUp.SafeUnitAndMag(FakeUp) > 0.0f)
		{
			FakeUp.Mul(10.0f);
			FakeUp.Add(s_pWD->pMover->GetEntity()->MtxToWorld()->m_vPos);
			FakeUp.y = s_pWD->pMover->GetLoc().y+40.0f;
			s_pWD->pMover->FaceToward(FakeUp, NULL);
		}

		if (_pWD->pBot &&
			_pWD->pBot->m_apWeapon[0] &&
//				_pWD->pBot->m_apWeapon[0]->TypeBits() & ENTIT_WEAPON_STAFF &&
			((CWeaponStaff*) _pWD->pBot->m_apWeapon[0])->GetUnitChargePtr() &&
			*(((CWeaponStaff*) _pWD->pBot->m_apWeapon[0])->GetUnitChargePtr()) >= 1.0f)
		{
			pWeaponCtrl->m_fFireTimeOut = 0.0f; //kill the current burst
		}

		if (pWeaponCtrl->IsReady(FALSE))
		{  
			pWeaponCtrl->NotifyFire();
			s_pWD->pMover->m_Controls.Fire();
		}


		//Heat seeking stun gun
		if (m_pEnemy && ((CBot*) s_pWD->pMover->GetEntity())->m_apWeapon[0])
		{
			((CBot*) s_pWD->pMover->GetEntity())->m_apWeapon[0]->SetTargetedEntity(m_pEnemy);
		}
	}
*/	else if (pT->m_fTimeWithoutLOS < 4.0f)
	{
		DoWeaponAimAndFireWork_Generic(-1.0f, TRUE,  TRUE);
	}

	_EGEndFrame(pBrain);
}
