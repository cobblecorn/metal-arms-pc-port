#include "fang.h"
#include "flinklist.h"
#include "fScriptSystem.h"

#include "ApeNew.h"

#include "AiApi.h"
#include "AIBotMover.h"
#include "AIBrain.h"
#include "AIBrainman.h"
#include "AIBrainMems.h"
#include "AiBrainUtils.h"
#include "AIBTAtable.h"
#include "AIEnviro.h"
#include "AIEdgeLock.h"
#include "AIFSM.h"
#include "AIGameUtils.h"
#include "AIGraph.h"
#include "AIGroup.h"
#include "AIMain.h"
#include "AIMover.h"
#include "AICarMover.h"
#include "AINodePools.h"
#include "AIPatrolPath.h"
#include "AIRooms.h"
#include "AITactics.h"
#include "AIThoughtsGeneric.h"
#include "AIThoughtsGround.h"

#include "../alarmsys.h"
#include "../Bot.h"
#include "../BotProbe.h"
#include "../BotScientist.h"
#include "../BotTalkInst.h"
#include "../BotTalkData.h"
#include "../Bottitan.h"
#include "../BotZom.h"
#include "../Entity.h"
#include "../espline.h"
#include "../GString.h"
#include "../Player.h"
#include "../Site_BotWeapon.h"
#include "../Vehicle.h"
#include "../Weapon.h"
#include "../VehicleLoader.h"

extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );

static const f32 kfOOHun = 1.0f/100.0f;
static f32 _kfPanicOddsMinWorldAdd = 0.9f;
static f32 _kfKamikazeeOdsMinWOrldAdd = 0.2f;
u16 _kuInvestigateMarkFailureTime = 5;
f32 _kfInvestigateMarkMinDist = 5.0f;
static u16 _uGlobalKamikazeTimeOut = 0;	  //not const 
static f32 _kfMaxInvestigateMarkTimeWithLOS = 1.0f;
f32 kfTitanRocketSafetyRange = 15.0f;
f32 _kfTitanEraptSmartDelayTimeBase = 5.0f;

enum
{
	ARS_0_FLAG_NONE								= 0x00,
	ARS_0_FLAG_HIDE_UNTIL_BOTLAYER_PANICSECS	= 0x01,
	ARS_0_TEAM_GRENADE_RANGE_TACTIC				= 0x02
};


BOOL InRules_ARS_0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_STANDGROUND);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GAStandGroundSetParams(pT,
							2,//u8 uDodgeTimeMin,
							4,//u8 uDodgeTimeMax,
							2,//u8 uDodgeOutTimeMin,
							4,//u8 uDodgeOutTimeMax,
							2,//u8 uDamageDodgeTimeMin
							3,//u8 uEnemyRelAttackPtSearchDelayTimeMin
							7,//u8 uEnemyRelAttackPtSearchDelayTimeMax	//max < min means never!
							CGroundCombat::STANDGROUNDSTATE_VEHICLESEARCH);

	return AIFSM_INIT_CB_DONE;
}



BOOL DoRules_ARS_0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//while in base state
	//cur tactic can be RANGEATTACK, or CIRCLESTRAFE
	u16 uInvestigateMarkAfterTime = 5 + (u16) 2*(pBrain->GetWorldAddRandom() < 0.5f) - (u16) ((f32)5*pBrain->GetAttrib(CAIBrain::ATTRIB_AGGRESSION)*kfOOHun);
	f32 fNormHealth = _pWD->pMover->GetEntity()->NormHealth();
	f32 fHideHealth = 1.0f - _pWD->fCourage;
	u32 uAllies = aigroup_CountEnemiesOf(pT->m_pEnemy);

	// if
	//	  low damage threshold reached &&
	//    very infrequent global kamikazee timer is up &&
	//	  very low odds diceroll
	// then
	//	  kamikazee
	//
	if (fNormHealth < fHideHealth &&
		_uGlobalKamikazeTimeOut < _pWD->uNowTime &&
		pBrain->GetWorldAddRandom() < _kfKamikazeeOdsMinWOrldAdd)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_CHALLENGE);

		pT->m_uAttackFlags |= CGroundCombat::FLAG_KAMIKAZEE;
		_uGlobalKamikazeTimeOut = _pWD->uNowTime+(u16)fmath_RandomChoice(20);
	}
	
	// if
	//	  low damage threshold reached
	// then
	//	  hide, panic
	//
	else if (fNormHealth < fHideHealth &&
			 uAllies < 2 &&
			 pBrain->GetWorldAddRandom() < _kfPanicOddsMinWorldAdd)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}
	
	// if
	//	  bot has panic flag set
	// then
	//	  hide, panic
	//
	else if (_pWD->pBot && (_pWD->pBot->GetPanicSecs() > 0.0f))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}

	// else if
	//	  Getting damaged
	// then
	//	  Take Cover
	//
	else if (!pBrain->GetAIMover()->IsTalkingWithAnim() &&
			 pT->m_fDamageTakenPerSec > 0.1f)   //base->Hidefromdamage
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDEFROMDAMAGE);

		//BTAPLAY
		pT->CombatMode_PlayBTA("P*E_RUNS", FALSE, FALSE);
	}
	//else if
	//	 Haven't seen the enemy for some time (aggression is a factor) &&
	//   currently am more than _kfInvestigateMarkMinDist feet from enemymark  (prevents rule from firing repeatedly)
	//then
	//   Investigate the mark
	//
	else if (_pWD->fDistToEnemyMark >= _kfInvestigateMarkMinDist &&
			 pT->m_pEnemy &&//have I ever seen him?
			 pT->m_fTimeWithoutLOS > (f32)uInvestigateMarkAfterTime)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_INVESTIGATEMARK);
	}
	//if
	//	 curTactic==STANDGROUND &&
	//   enemy standing near center of a large open area
	//then
	//   switch to circle strafe techniq
	//
	else if (_pWD->pEnemyPoi && pT->m_TacticFSM.GetActiveStateHandle() == CGroundCombat::TACTIC_STANDGROUND)
	{
		pT->ChangeTactic(CGroundCombat::TACTIC_CIRCLESTRAFE);
	}
	//if
	//	 curTactic == CGroundCombat::TACTIC_CIRCLESTRAFE &&
	//   enemy Not standing near center of a large open area
	//then
	//   switch to stand ground tactic
	//
	else if (!_pWD->pEnemyPoi && pT->m_TacticFSM.GetActiveStateHandle() == CGroundCombat::TACTIC_CIRCLESTRAFE)
	{
		pT->ChangeTactic(CGroundCombat::TACTIC_STANDGROUND);
		_GAStandGroundSetParams(pT,
								2,//u8 uDodgeTimeMin,
								4,//u8 uDodgeTimeMax,
								2,//u8 uDodgeOutTimeMin,
								4,//u8 uDodgeOutTimeMax,
								2,//u8 uDamageDodgeTimeMin
								3,//u8 uEnemyRelAttackPtSearchDelayTimeMin
								7,//u8 uEnemyRelAttackPtSearchDelayTimeMax	//max < min means never!
								CGroundCombat::STANDGROUNDSTATE_VEHICLESEARCH);
	}
	//if
	//	 curTactic==STANDGROUND &&
	//   there is a team grenade attack under way, switch to range tactic for a few seconds
	//then
	//   switch to circle strafe techniq
	//
	else if (pT->m_TacticFSM.GetActiveStateHandle() == CGroundCombat::TACTIC_STANDGROUND &&
			 pBrain->GetKnowledge().CanRememberAny(MEMORY_MEDIUM_TEAMGRENADE))
	{
		pT->m_uARS_0_flags |= ARS_0_TEAM_GRENADE_RANGE_TACTIC;
		pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
		_GARangeSetParams(	pT,
					4,		   //u8 uDodgeTimeMin,   
					0,		   //u8 uDodgeTimeMax,   
					4,		   //u8 uDodgeOutTimeMin,
					0,		   //u8 uDodgeOutTimeMax,
					1,		   //u8 uDamageDodgeTimeMin
					20.f,	   //f32 fRangeMin,      
					1000.f,	   //f32 fRangeMax
					CGroundCombat::ERAPTUSEAGECTRL_NEVER,
					0.0f);	 //erapt los wait time
	}
	//if
	//	 curTactic == CGroundCombat::TACTIC_CIRCLESTRAFE &&
	//   enemy Not standing near center of a large open area
	//then
	//   switch to stand ground tactic
	//
	else if (pT->m_uARS_0_flags & ARS_0_TEAM_GRENADE_RANGE_TACTIC &&
			 (pT->m_TacticFSM.GetActiveStateHandle() == CGroundCombat::TACTIC_RANGEATTACK) &&
			!(pBrain->GetKnowledge().CanRememberAny(MEMORY_MEDIUM_TEAMGRENADE)))
	{
		pT->ChangeTactic(CGroundCombat::TACTIC_STANDGROUND);
		_GAStandGroundSetParams(pT,
								2,//u8 uDodgeTimeMin,
								4,//u8 uDodgeTimeMax,
								2,//u8 uDodgeOutTimeMin,
								4,//u8 uDodgeOutTimeMax,
								2,//u8 uDamageDodgeTimeMin
								3,//u8 uEnemyRelAttackPtSearchDelayTimeMin
								7,//u8 uEnemyRelAttackPtSearchDelayTimeMax	//max < min means never!
								CGroundCombat::STANDGROUNDSTATE_VEHICLESEARCH);
	}
	return AIFSM_WORK_CB_RETURN;
}

BOOL InRules_ARS_0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GARangeSetParams(	pT,
						4,		   //u8 uDodgeTimeMin,   
						0,		   //u8 uDodgeTimeMax,   
						4,		   //u8 uDodgeOutTimeMin,
						0,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						0.0f,	   //f32 fRangeMin,      
						2.0f,	    //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);	

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//While in InvestigateMark Tactic is RANGE_ATTACK

	f32 fNormHealth = _pWD->pMover->GetEntity()->NormHealth();
	f32 fHideHealth = 1.0f - _pWD->fCourage;
	u32 uAllies = aigroup_CountEnemiesOf(pT->m_pEnemy);

	pT->m_uAttackFlags |= CGroundCombat::FLAG_HEAD_SCAN;

	// if
	//	  low damage threshold reached &&
	//    very infrequent global kamikazee timer is up &&
	//	  very low odds diceroll
	// then
	//	  kamikazee
	//
	if (fNormHealth < fHideHealth &&
		_uGlobalKamikazeTimeOut < _pWD->uNowTime &&
		pBrain->GetWorldAddRandom() < _kfKamikazeeOdsMinWOrldAdd)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_CHALLENGE);

		pT->m_uAttackFlags |= CGroundCombat::FLAG_KAMIKAZEE;
		_uGlobalKamikazeTimeOut = _pWD->uNowTime+(u16)fmath_RandomChoice(20);
	}
	// if
	//	  low damage threshold reached
	// then
	//	  hide, panic
	//
	else if (	fNormHealth < fHideHealth &&
				uAllies < 2 &&
				pBrain->GetWorldAddRandom() < _kfPanicOddsMinWorldAdd)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}
	
	// if
	//	  bot has panic flag set
	// then
	//	  hide, panic
	//
	else if (_pWD->pBot && (_pWD->pBot->GetPanicSecs() > 0.0f))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}

	// if
	//	  Getting damaged
	// then
	//	  Take Cover
	//
	else if (!pBrain->GetAIMover()->IsTalkingWithAnim() &&
			 pT->m_fDamageTakenPerSec > 0.1f)	//investigatemark->hidefromdamage
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDEFROMDAMAGE);

		//BTAPLAY
		pT->CombatMode_PlayBTA("P*E_RUNS", FALSE, FALSE);
	}
	//if
	//   I have had LOS with enemy for a short, but random amount of time
	//then
	//   go to base state
	else if (pT->m_fTimeWithLOS > 0.0f + pBrain->GetInfreqCycleRandom()*_kfMaxInvestigateMarkTimeWithLOS)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_BASE);
	}
	//else if
	//	 close to the mark but don't see enemy or
	//	 can't figure out how to get to the mark (is stopped un-intentionally)
	//then
	//   go to search state
	else if ((_pWD->fDistToEnemyMark < _kfInvestigateMarkMinDist) ||
			 (0))//	!pBrain->HasMovedOneFootSince(_kuInvestigateMarkFailureTime) && pT->m_fTimeWithLOS == 0.0f))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_SEARCH);
	}
	
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2 )
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_PEEKABOO);
	pT->m_uLastPeekABooBailOut = _pWD->uNowTime;  
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GAPeekABooSetParams(pT,						 
						2, 4,						 //timer for when to come out of cover!          (min > max means never)                                                                                  
						2, 4);						 //timer for how long to stay out of cover under normal conditions

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2 )
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//tactic is peekaboo for HideFromDamage
	f32 fNormHealth = _pWD->pMover->GetEntity()->NormHealth();
	f32 fHideHealth = 1.0f - _pWD->fCourage;
	u32 uAllies = aigroup_CountEnemiesOf(pT->m_pEnemy);
	
	// if 
	//
	//    not taking much damage recently &&
	//	      There are no good local attack pts and I have been in pT state for at least some SHORTish amount of time,  ||
	//		  There are good local attack pts &&
	//				I have been doing pT for awhile, ||
	//				I can't remember having done any damage to the baddie in a while
	// then
	//    switch back to base state
	//
	if ((pT->m_fDamageTakenPerSec < 0.25f) &&  //quit hiding from damage
		((((pT->m_AttackPtSearch.m_uRepeatTimeMax < pT->m_AttackPtSearch.m_uRepeatTimeMin) || pT->m_AttackPtSearch.m_uFailureCount > 4) && (_pWD->uNowTime - pT->m_uLastPeekABooBailOut) > 3 ) ||
		 (pT->m_AttackPtSearch.m_uFailureCount < 4 && (_pWD->uNowTime - pT->m_uLastPeekABooBailOut) > 3  && (((_pWD->uNowTime - pT->m_uLastPeekABooBailOut) > 30 ) || (pT->m_AttackPtSearch.m_uSucceedCount > 0 &&(!_pWD->pDamageDealtMem || (_pWD->uNowTime - _pWD->pDamageDealtMem->m_uWhenTimeSecs > 10)))))))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_INVESTIGATEMARK);
	}
	// if
	//	  low damage threshold reached
	// then
	//	  hide, panic
	//
	else if (	fNormHealth < fHideHealth &&
				uAllies < 2 &&
				pBrain->GetWorldAddRandom() < _kfPanicOddsMinWorldAdd)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}
	
	// if
	//	  bot has panic flag set
	// then
	//	  hide, panic
	//
	else if (_pWD->pBot && (_pWD->pBot->GetPanicSecs() > 0.0f))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}

	if (pT->m_AttackPtSearch.m_uFailureCount < 4 && 
		(pT->m_AttackPtSearch.m_uRepeatTimeMax >= pT->m_AttackPtSearch.m_uRepeatTimeMin))
	{
		if (_pWD->pDamageDealtMem)
		{
			_pWD->pDamageDealtMem->m_uHowLongSecs++;		//extend the "normal" life ofthe damage dealt memory because I'm using it in pT state
		}
	}

	return AIFSM_WORK_CB_RETURN;
}

BOOL InRules_ARS_0_Hide(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_PEEKABOO);
	pT->m_uLastPeekABooBailOut = _pWD->uNowTime;
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GAPeekABooSetParams(pT,						 
						1, 0,						 //timer for when to come out of cover!          (min > max means never)                                                                                  
						1, 0);						 //timer for how long to stay out of cover under normal conditions


	pT->m_uARS_0_flags &= ~ARS_0_FLAG_HIDE_UNTIL_BOTLAYER_PANICSECS;
	if (_pWD->pBot &&
		_pWD->pBot->GetPanicSecs() >=0.01f)
	{
		pT->m_uARS_0_flags |= ARS_0_FLAG_HIDE_UNTIL_BOTLAYER_PANICSECS;
	}
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_0_Hide(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bIsPanicOn = FALSE;

	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_DIR_FORWARD;  //normal torso cntrl would have torso face the enemy.

	//a few rules to do panic if in hide state
	bIsPanicOn = pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON;
	if (!bIsPanicOn &&
		pT->m_uLastPanicOnDelayTimeOut < _pWD->uNowTime)
	{
		pT->m_uAttackFlags |= CGroundCombat::FLAG_PANIC_ON;
		pT->m_uLastPanicOnDelayTimeOut = _pWD->uNowTime+5+(u16)fmath_RandomChoice(10);
		pBrain->PushBaseSpeedSetting(100); 	 //push a new speed setting on the stack
	}

	if (bIsPanicOn &&
		(pT->m_TacticFSM.GetActiveStateHandle() != CGroundCombat::TACTIC_PEEKABOO || pT->m_uPeekABooState == CGroundCombat::TCSTATE_COVER) &&
		pT->m_uLastPanicOnDelayTimeOut < _pWD->uNowTime)
	{
		pT->m_uLastPanicOnDelayTimeOut = _pWD->uNowTime+4+(u16)fmath_RandomChoice(4);
		pT->m_uAttackFlags &= ~CGroundCombat::FLAG_PANIC_ON;
		pBrain->PopBaseSpeedSetting();
	}
	
	if (pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON)
	{
		_pWD->pMover->m_Controls.Panic();
		_pWD->pMover->m_Controls.SetFlag_Slam_RotateCW_Average();
		pBrain->PopBaseSpeedSetting();
		pBrain->PushBaseSpeedSetting(100); 	 //push a new speed setting on the stack

		if (_pWD->fDistToEnemyMark < 50.0f &&
			pT->m_bPathJustCompleted &&
			pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_TAKINGCOVER &&
			pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON)
		{	//when panic'd no cover is good enough
			_GAPeekABooReset(pT);	 
		}
	}


	if (!(_pWD->pBot && _pWD->pBot->IsWalkingDead()) &&
		pT->m_uARS_0_flags & ARS_0_FLAG_HIDE_UNTIL_BOTLAYER_PANICSECS &&
		_pWD->pBot && 
		_pWD->pBot->GetPanicSecs() == 0.0f)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_BASE);
	}


#if 0
	// SER: This seems like a bad thing to do!
	if (pT->m_fDamageTakenPerSec == 0.0f &&	  //quit hide
		pT->m_fTimeWithoutLOS > 7.0f)
	{
		//charge back up and resume attack.
		if (_pWD->pMover->GetEntity()->NormHealth() < 1.0f)
		{
			_pWD->pMover->GetEntity()->SetNormHealth(2.0f);
		}
		pT->ChangeTactic(ARS_0_BASE);
	}
#endif
	return AIFSM_WORK_CB_RETURN;
}

BOOL InRules_ARS_0_Challenge(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	pT->m_uLastPeekABooBailOut = _pWD->uNowTime;
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GARangeSetParams(	pT,
						4,		   //u8 uDodgeTimeMin,   
						0,		   //u8 uDodgeTimeMax,   
						4,		   //u8 uDodgeOutTimeMin,
						0,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						1.0f,	   //f32 fRangeMin,      
						3.0f,	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_0_Challenge(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	if (_pWD->fDistToEnemyMark < 5.0f && _pWD->bHasLOSThisFrame)
	{
		if (pT->m_uAttackFlags & CGroundCombat::FLAG_KAMIKAZEE)
		{
			pT->m_uAttackFlags &= ~CGroundCombat::FLAG_KAMIKAZEE;
			if (_pWD->pMover->GetEntity() &&
				_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT &&
				((CBot*) _pWD->pMover->GetEntity())->m_apWeapon[0] &&
				((CBot*) _pWD->pMover->GetEntity())->m_apWeapon[0]->TypeBits() & ENTITY_BIT_WEAPONGREN)
			{
				_pWD->pMover->m_Controls.Fire2();		 //Self Detonate
			}
		}

		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDEFROMDAMAGE);
	}
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//kick into search mode, then eventually back to our job if no enemy can be found
	pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GASearchSetParams(pT);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	f32 fNormHealth = _pWD->pMover->GetEntity()->NormHealth();
	f32 fHideHealth = 1.0f - _pWD->fCourage;
	u32 uAllies = aigroup_CountEnemiesOf(pT->m_pEnemy);

	// if
	//	  low damage threshold reached &&
	//    very infrequent gloabl kamikazee timer is up &&
	//	  very low odds diceroll
	// then
	//	  kamikazee
	//
	if (fNormHealth < fHideHealth &&
		_uGlobalKamikazeTimeOut < _pWD->uNowTime &&
		pBrain->GetWorldAddRandom() < _kfKamikazeeOdsMinWOrldAdd)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_CHALLENGE);
		pT->m_uAttackFlags |= CGroundCombat::FLAG_KAMIKAZEE;
		_uGlobalKamikazeTimeOut = _pWD->uNowTime+(u16)fmath_RandomChoice(20);
	}
	// if
	//	  low damage threshold reached
	// then
	//	  hide, panic
	//
	else if (	fNormHealth < fHideHealth &&
				uAllies < 2 &&
				pBrain->GetWorldAddRandom() < _kfPanicOddsMinWorldAdd)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}
	
	// if
	//	  bot has panic flag set
	// then
	//	  hide, panic
	//
	else if (_pWD->pBot && (_pWD->pBot->GetPanicSecs() > 0.0f))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}

	// if
	//	  Getting damaged
	// then
	//	  Take Cover
	//
	else if (!pBrain->GetAIMover()->IsTalkingWithAnim() &&
			 pT->m_fDamageTakenPerSec > 0.1f)	//search->hidefromdamage
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDEFROMDAMAGE);

		//BTAPLAY
		pT->CombatMode_PlayBTA("P*E_RUNS", FALSE, FALSE);

	}
	//
	//If 
	//		sees the enemy
	//then
	//		go back to base state
	//
	else if (pT->m_fTimeWithoutLOS < 0.7f)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_BASE);
	}
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_0_SiteGunner(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	pT->ResetCommonTacticVbls();
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_0_SiteGunner(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bExitPillState = FALSE;
	if (!_pWD->pMover->GetCurMech())
	{
		//we are not in our site weapon

		//why?

		//first, assume it is dead, or broken, or that we are incapable of operating it anymore
		bExitPillState = TRUE;

		//always look alert if standing on a pill
		_pWD->pMover->m_Controls.Alert();


		if (pBrain->GetMechLock()  &&
			!pBrain->GetMechLock()->IsDeadOrDying() && 
			pBrain->GetMechLock()->IsInWorld())	//assume that the pill I have locked is the vehicle that I'm currently "in"
		{
			FASSERT(pBrain->GetMechLock()->TypeBits() & ENTITY_BIT_SITEWEAPON);
			if (pBrain->GetMechLock() &&
				pBrain->GetMechLock()->TypeBits() & ENTITY_BIT_SITEWEAPON &&
				pT->m_fTimeWithoutLOS < 2.0f)
			{
				CBotSiteWeapon* pSite = (CBotSiteWeapon*) pBrain->GetMechLock();
				CFVec3A DeltaToEnemy;
				DeltaToEnemy.Sub(_pWD->EnemyMarkTagPos, _pWD->pMover->GetEyePos());
				
				if ((FVid_nFrameCounter & 1) &&   //just so it waits a frame before trying again
					((!pBrain->GetMechLock()->IsPillBox()) ||  //only pill has limited yaw
					DeltaToEnemy.SafeUnitAndMag(DeltaToEnemy) > 0.0f && 
					DeltaToEnemy.Dot(pSite->BaseMtxToWorld()->m_vFront) > FMATH_DEG2RAD(5)))
				{
					_pWD->pMover->RequestMechEntry(pSite);  //start controlling the pill
				}
				bExitPillState = FALSE;
			}
		}
	}
	else if (_pWD->pMover->GetCurMech()->TypeBits() & ENTITY_BIT_SITEWEAPON)
	{
		FASSERT(pBrain->GetMechLock() && pBrain->GetMechLock() == ((CBot*) _pWD->pMover->GetEntity())->GetCurMech());
		CBotSiteWeapon* pSite = (CBotSiteWeapon*) ((CBot*) _pWD->pMover->GetEntity())->GetCurMech();
		
		//
		// when to leave the pillbox, but stay on the platform
		//

		CFVec3A DeltaToEnemy;
		DeltaToEnemy.Sub(_pWD->EnemyMarkTagPos, _pWD->pMover->GetEyePos());

/*		code to test site weapon front vec
		CFVec3A tmp;
		tmp = pSite->BaseMtxToWorld()->m_vFront;
		tmp.Mul(10.0f);
		tmp.Add(_pWD->pMover->GetEyePos());
		aiutils_DebugTrackRay(_pWD->pMover->GetEyePos(), tmp, 2);
*/
		
/*		if ((FVid_nFrameCounter & 1) &&
			(!pBrain->GetMechLock()->IsPillBox() ||  //only pill has limited yaw
				DeltaToEnemy.SafeUnitAndMag(DeltaToEnemy) > 0.0f))
		{
			f32 fDot = DeltaToEnemy.Dot(pSite->BaseMtxToWorld()->m_vFront);
			if (fDot < -FMATH_DEG2RAD(45))
			{
				//just so it waits one frame before trying again
				_pWD->pMover->RequestMechExit(pSite);  //quit controlling the pill
			}
		}
  */
	}
	else if (0)
	{
		//shot from outof pill range, then exit pill
	}

	if (1)//aiutils_FTotalLoopSecs() - pT->m_fTimeWithoutLOS > 2.0f)
	{
		//init pill head look behavior	 (will only be used when normal torso and head controls determin target has been lost
		if (pT->m_pLookAtMgr)
		{
			pT->m_pLookAtMgr->m_uWaitFlags |= (CGenericWait::WAIT_AUTOSCAN_TORSO | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_STANDING | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_WALKING);
			pT->m_pLookAtMgr->m_uWaitFlags &= ~CGenericWait::WAIT_VISRAYS_ENABLED;
		}
	}

	if (bExitPillState)
	{
		if (pT->m_pLockedPoi)
		{
			FASSERT(pT->m_pLockedPoi->m_uFlags & CGraphPoi::POIFLAG_SITEWEAPON);
			pT->ReleaseLockedPoi();
		}

		//go back to base state
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_BASE);

		//cleanup from pill head look behavior
		if (pT->m_pLookAtMgr)
		{
			pT->m_pLookAtMgr->m_uWaitFlags &= ~(CGenericWait::WAIT_AUTOSCAN_TORSO | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_WALKING);
			pT->m_pLookAtMgr->m_uWaitFlags |= CGenericWait::WAIT_VISRAYS_ENABLED;
		}
	}
	return AIFSM_WORK_CB_RETURN;
}


BOOL DoRules_ARS_0_Passenger(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_0_Driver(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
	pT->ResetCommonTacticVbls();
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_0_Driver(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//just sit and ride.
	BOOL bExitState = FALSE;
	if (!((CBot*) _pWD->pMover->GetEntity())->GetCurMech())
	{
		//why aren't we in a vehicle?
		//maybe it is dead, or broken
		bExitState = TRUE;

		if (pBrain->GetMechLock())
		{
			if ((_pWD->uNowTime - pT->m_uTacticInitTime) < 2 &&
				pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED)
			{
				//need to have another state for mount, but now
				//we will just request vehicle entry
				if (FVid_nFrameCounter & 1)   //just so it waits a frame before trying again
				{
    				_pWD->pMover->RequestMechEntry(pBrain->GetMechLock());
				}
				bExitState = FALSE;
			}
		}
	}
	
	
	//
	// some special code for drivers of vehicles using ATTACKRULSET_RAT0 because they don't leave attack ever!
	//	  //findfix: probably should make a custom driver ruleset for them
	if (_pWD->pMover->GetCurMech() &&
		_pWD->pMover->GetCurMech()->AIBrain() && 
		_pWD->pMover->GetCurMech()->AIBrain()->GetCurThoughtPtr() &&
		_pWD->pMover->GetCurMech()->AIBrain()->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT) &&
		_pWD->pMover->GetCurMech()->AIBrain()->GetAttackRuleSet() == CAIBrain::ATTACKRULESET_RAT0 )
	{
		// way to stop CGroundCombat from timing out like happens normally to bots with no enemies
		pT->m_uAttackFlags |= CGroundCombat::FLAG_DISABLE_ENEMY_RELATED_EXIT_CONDITIONS;


		if (_pWD->pMover->GetCurMech() &&
			_pWD->pMover->GetCurMech()->TypeBits() & ENTITY_BIT_VEHICLE &&
			((CVehicle*)_pWD->pMover->GetCurMech())->GetSplinePath())
		{
			pT->m_uThoughtFlags &= ~CAIThought::THOUGHTFLAG_LOD_TREAT_GOAL_WITH_JOB_LOD_RULES;
		}
		else
		{
			pT->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_LOD_TREAT_GOAL_WITH_JOB_LOD_RULES;
		}
	}

	if (pT->m_pLookAtMgr)//aiutils_FTotalLoopSecs() - pT->m_fTimeWithoutLOS > 2.0f)
	{
		pT->m_pLookAtMgr->m_uWaitFlags |= (CGenericWait::WAIT_AUTOSCAN_TORSO | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_STANDING | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_WALKING);
		pT->m_pLookAtMgr->m_uWaitFlags &= ~CGenericWait::WAIT_VISRAYS_ENABLED;
	}

	if (bExitState)
	{
		//go back to base state
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_BASE);

		//cleanup from pill head look behavior
		if (pT->m_pLookAtMgr)
		{
			pT->m_pLookAtMgr->m_uWaitFlags &= ~(CGenericWait::WAIT_AUTOSCAN_TORSO | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_WALKING);
			pT->m_pLookAtMgr->m_uWaitFlags |= CGenericWait::WAIT_VISRAYS_ENABLED;
		}
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	pT->m_uARS_0_flags = 0;
	if (pBrain->GetMechLock() && pBrain->GetAIMover()->GetCurMech())
	{
		if (pBrain->GetMechLock()->TypeBits() & ENTITY_BIT_SITEWEAPON)
		{
			pT->ChangeRuleSetState(CGroundCombat::ARS_0_SITEGUNNER);
		}
		else
		{
			pT->ChangeRuleSetState(CGroundCombat::ARS_0_DRIVER);
		}
	}
	else
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_BASE);
	}

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//
	//	One type of robot in one situation might use the following
	//	  logic to decide which tactics to use
	//

	//
	// CurTactic is the "state"
	//	  check rules for each state
#ifdef _DEBUG
	FSMStateHandle uRuleState = pT->m_RuleSetStateFSM.GetActiveStateHandle();
	cchar* pszRuleStateName = pT->m_RuleSetStateFSM.GetActiveStateName();
	switch (uRuleState)
	{
		case FSMSTATEHANDLE_INVALID:
			break;
		case CGroundCombat::ARS_0_BASE:
			break;
		case CGroundCombat::ARS_0_INVESTIGATEMARK:
			break;
		case CGroundCombat::ARS_0_HIDEFROMDAMAGE:
			break;
		case CGroundCombat::ARS_0_HIDE:
			break;
		case CGroundCombat::ARS_0_CHALLENGE:
			break;
		case CGroundCombat::ARS_0_SEARCH:
			break;
		case CGroundCombat::ARS_0_SITEGUNNER:
			break;
		case CGroundCombat::ARS_0_PASSENGER:
			break;
		case CGroundCombat::ARS_0_DRIVER:
			break;
		default:
			FASSERT(pT->m_RuleSetStateFSM.IsStateChangePending());
			pT->ChangeRuleSetState(CGroundCombat::ARS_0_BASE);
			break;
	}
#endif

	//
	//	RuleSetState work
	//
	pT->m_RuleSetStateFSM.Work();

	//no matter which state I'm in, if I find myself having locked vehicle (pill or otherwise)
	//then switch to the appropriate ruleset
	if (pBrain->GetMechLock() &&
		((pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED) ||		  //I 
		 pBrain->GetAIMover()->GetCurMech()))
	{
		if ((pBrain->GetMechLock()->TypeBits() & ENTITY_BIT_SITEWEAPON))
		{
			if (pT->m_RuleSetStateFSM.GetActiveStateHandle() != CGroundCombat::ARS_0_SITEGUNNER)
			{
				pT->ChangeRuleSetState(CGroundCombat::ARS_0_SITEGUNNER);
			}
		}
		else
		{
			if (pT->m_RuleSetStateFSM.GetActiveStateHandle() != CGroundCombat::ARS_0_DRIVER)
			{
				pT->ChangeRuleSetState(CGroundCombat::ARS_0_DRIVER);
			}
		}
	}

	pT->DoGenericTauntWork();
	
	//
	//  Special RULE that happens no matter what the sub mode of rs0 is
	//
	if (_pWD->pBot && _pWD->pBot->IsWalkingDead() && pT->m_RuleSetStateFSM.GetActiveStateHandle() != CGroundCombat::ARS_0_HIDE)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_0_HIDE);
	}

	// if
	// 		Not following an attackpos path ||
	//		folowing an attack path for circle strafe purposes
	//	then
	//		enable Alert Mode.
	//
	u8 uCurPathReason = CGroundCombat::SEARCHREASON_INVALID;
	if (pT->IsFollowingPath())
	{
		uCurPathReason = pT->m_anSearchReason[pT->m_nCurPathRequestSlot];	//if following a path, this will be the slot of the currentpath
	}

	if (_pWD->pBot && _pWD->pBot->IsCapableofAlert())
	{
		if (pT->m_TacticFSM.GetActiveStateHandle() == CGroundCombat::TACTIC_CIRCLESTRAFE ||	//circlestrafe tru
			!(pT->m_RuleSetStateFSM.GetActiveStateHandle() == CGroundCombat::ARS_0_SEARCH ||
				_pWD->pBot->IsThrowing() ||	 //can't be alert and throwing at same time
				((pT->m_uAttackFlags & CGroundCombat::FLAG_COMBAT_CANT_MOVE_BOT) && _pWD->pMover->IsFollowingPath()) ||
				uCurPathReason == CGroundCombat::SEARCHREASON_NEWATTACKPOS ||
				uCurPathReason == CGroundCombat::SEARCHREASON_TAKINGCOVER ||
				uCurPathReason == CGroundCombat::SEARCHREASON_RETREAT ||
				pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED ||
				pT->m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON	||
				(pT->m_RuleSetStateFSM.GetActiveStateHandle()== CGroundCombat::ARS_0_INVESTIGATEMARK  && uCurPathReason == CGroundCombat::SEARCHREASON_RANGE && _pWD->fDistToEnemyMarkXZ > (15.0f+pT->GetBrain()->GetWorldAddRandom()*10.0f+_pWD->pMover->GetRadiusXZ()))
				))
		{
			if (!_pWD->pMover->IsTalkingWithAnim() && pBrain->GetWorldAddRandom() < 0.925f)
			{
				pT->m_uLastAlertOnDelayTimeOut = _pWD->uNowTime + 4;
				_pWD->pMover->m_Controls.Alert();		//Alert doesn't stick, so dude will only be in alert while in state_base
			}
		}
	}

	// if
	//		no sight of enemy for a really long time,
	//
	// then 
	//		attack mode fails
	//
	u16 uAttackTimeWithoutLOS = 70 + (u16)(30.0f*_pWD->fAggression)+ (u16) (pBrain->GetWorldAddRandom()*10.0f);
	if ((pT->m_uAttackFlags & CGroundCombat::FLAG_CANTFINDENEMY) &&
		(_pWD->uNowTime - pT->m_uLostEnemyTime > uAttackTimeWithoutLOS) ||
		(_pWD->uNowTime - pT->m_uLostEnemyTime > 10 && !(pBrain->GetLODFlags() & (CAIBrain::LOD_VISIBLE | CAIBrain::LOD_ACTIVE))))
	{
		pT->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;	 //give up.
	}
	

	//
	//  Grenades, etc
	//
	pT->DoGenericEnviroReactRules(TRUE, TRUE, TRUE);



	//
	//if
	//		combat mode is recently activated
	//then 
	//		Don't require stop and shoot in weaponaim and fire logic
	//
	//	  (prevents sluggish appearence at origin of attack)
	if (aiutils_FTotalLoopSecs() - pT->m_fAttackThoughtInitTime	< 2.0f)
	{
		pT->m_uAttackFlags |= CGroundCombat::FLAG_TRIGGEROVERRIDE_STOPSHOOT;  	// FLAG_TRIGGEROVERRIDE_STOPSHOOT  (Cleared every frame )
	}



	if (_pWD->pBot && _pWD->pBot->TypeBits() & ENTITY_BIT_BOTJUMPER &&
		!_pWD->pBot->IsWalkingDead())
	{
		CAIHoverBotMover* pJMover = (CAIHoverBotMover*) _pWD->pMover;
		if (pJMover->GetLastGraphVertId()) 
		{
			GraphVert* pV = aimain_pAIGraph->GetVert(pJMover->GetLastGraphVertId());
			pJMover->HoverTowardY(pV->m_Location.y+pV->m_fHeightClearance-pJMover->GetHeight());
		}
		
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_1(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	_GARangeSetParams(	pT,
						2,		   //u8 uDodgeTimeMin,   
						4,		   //u8 uDodgeTimeMax,   
						2,		   //u8 uDodgeOutTimeMin,
						4,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						5.0f,	   //f32 fRangeMin,      
						10.0f,	  //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_1(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

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

	pT->DoGenericTauntWork();

	//
	//  Special RULE that happens no matter what the sub mode of rs0 is
	//
	if (_pWD->pBot && _pWD->pBot->IsWalkingDead())
	{
		pT->ChangeRuleSet(CAIBrain::ATTACKRULESET_0);	   //will automatically kick into hide/panic mode whenthis happens
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_NPC_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	f32 fRangeMin = _pWD->pMover->GetRadiusXZ()*4.0f;
	f32 fRangeMax = pBrain->GetEyeScanDist()*0.75f;
	if (_pWD->pEnemyBot)
	{
		fRangeMin += _pWD->pEnemyBot->m_fCollCylinderRadius_WS;
		fRangeMax += _pWD->pEnemyBot->m_fCollCylinderRadius_WS;
	}

	if (pBrain->GetLeader() && pT->m_pEnemy)
	{
		f32 fLeaderToEnemyDist = pBrain->GetLeader()->GetLoc().Dist(pT->m_pEnemy->MtxToWorld()->m_vPos);
		fRangeMin += fLeaderToEnemyDist;
		fRangeMax += fLeaderToEnemyDist;
	}

	_GARangeSetParams(	pT,
						8,				//u8 uDodgeTimeMin,   
						15,				//u8 uDodgeTimeMax,   
						4,				//u8 uDodgeOutTimeMin,
						8,				//u8 uDodgeOutTimeMax,
						0,			//u8 uDamageDodgeTimeMin
						fRangeMin,		//f32 fRangeMin,      
						fRangeMax,		//f32 fRangeMax	  
						CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED,	 //time set in SetAttackPtSearchParams
						0.0f);

	//customize the erapt search timer
	pT->m_AttackPtSearch.SetAttackPtSearchParams(	5,// u8 uRepeatTimeMin,
												30,// u8 uRepeatTimeMax,
												1,// u8 uDirInc,
												0,// u8 uDirMin,
												3,// u8 uDirMax,
												_pWD->pMover->GetRadiusXZ()*2.0f>255.0f ? 255:(u8)(_pWD->pMover->GetRadiusXZ()*2.0f),// s8 nRangeXZInc
												fRangeMin > 255.0f ? 255: (u8) fRangeMin,
												fRangeMax > 255.0f ? 255: (u8) fRangeMax, // u8 uRangeXZMin
												0,
												0,
												0);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_NPC_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	u16 uInvestigateMarkAfterTime = 1;

	if (_pWD->fDistToEnemyMark >= _kfInvestigateMarkMinDist &&		//I'm not already there
		pT->m_pEnemy &&												//have I ever seen him?
		pT->m_fTimeWithoutLOS > (f32)uInvestigateMarkAfterTime)		//little time delay, incase he comes back
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_NPC_INVESTIGATEMARK);
	}
	return AIFSM_WORK_CB_RETURN;
}



BOOL InRules_ARS_NPC_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GARangeSetParams(	pT,
						0,		   //u8 uDodgeTimeMin,   
						0,		   //u8 uDodgeTimeMax,   
						0,		   //u8 uDodgeOutTimeMin,
						0,		   //u8 uDodgeOutTimeMax,
						0,			//u8 uDamageDodgeTimeMin
						0.0f,	   //f32 fRangeMin,      
						4.0f,	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER, 0.0f);   
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_NPC_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//if
	//   I get LOS with enemy since this investigate mark state has started
	//
	//then
	//   go to base state
	if (pT->m_fTimeWithoutLOS == 0.0f &&
		1
	//	((u16) pT->m_fTimeWithoutLOS < (_pWD->uNowTime - pT->m_uTacticInitTime)) &&
		)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_NPC_BASE);
	}
	//else if
	//	 close to the mark but don't see enemy or
	//	 can't figure out how to get to the mark (is stopped un-intentionally)
	//then
	//   go to search state
	else if ((_pWD->fDistToEnemyMark < _kfInvestigateMarkMinDist) ||
			 (	!pBrain->HasMovedOneFootSince(_kuInvestigateMarkFailureTime) && pT->m_fTimeWithLOS == 0.0f))
	{
		pT->m_uAttackFlags |= CGroundCombat::FLAG_HEAD_SCAN;
	}
	return AIFSM_WORK_CB_RETURN;
}



BOOL InitRuleSet_NPC(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	
	pT->ChangeRuleSetState(CGroundCombat::ARS_NPC_BASE);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_NPC(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	//
	//	One type of robot in one situation might use the following
	//	  logic to decide which tactics to use
	//

	//
	// CurTactic is the "state"
	//	  check rules for each state
#ifdef _DEBUG
	FSMStateHandle uRuleState = pT->m_RuleSetStateFSM.GetActiveStateHandle();
	cchar* pszRuleStateName = pT->m_RuleSetStateFSM.GetActiveStateName();
	switch (uRuleState)
	{
		case FSMSTATEHANDLE_INVALID:
			break;
		case CGroundCombat::ARS_NPC_BASE:
			break;
		case CGroundCombat::ARS_NPC_INVESTIGATEMARK:
			break;
		default:
			if ( !pT->m_RuleSetStateFSM.IsStateChangePending() )
			{
//				FASSERT_NOW;
				pT->ChangeRuleSetState( CGroundCombat::ARS_NPC_BASE );
			}
			break;
	}
#endif

	/*
	CAIBrain* pLeaderBrain = pT->GetBrain()->GetLeader();
	if (pLeaderBrain)
	{
		bOkToAttack = FALSE;
		//O.k to attack?
		if (GetKnowledge().CanRememberEntity(MEMORY_LARGE_DAMAGED_BY, pSeenMem->m_pEntity) ||
			pLeaderBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_DAMAGE_DEALT, pSeenMem->m_pEntity))
		{
			bOkToAttack = TRUE;
		}
	}
	 */

	//
	//	RuleSetState work
	//
	pT->m_RuleSetStateFSM.Work();


	//
	//  Special RULE that happens no matter what the sub mode of rs0 is
	//
	if (_pWD->pBot && _pWD->pBot->IsWalkingDead())
	{
		pT->ChangeRuleSet(CAIBrain::ATTACKRULESET_0);	   //will automatically kick into hide/panic mode whenthis happens
	}

	pT->DoGenericEnviroReactRules(TRUE, TRUE, TRUE);

	cchar* pszBTAName = NULL;
	if (pBrain->GetLeader())
	{
		f32 fTimeSinceNewAttack = aiutils_GetCurTimeSecs() - pT->m_fAttackThoughtInitTime;
		if (	!pBrain->IsTalking() &&
				(fTimeSinceNewAttack > 0.25f && fTimeSinceNewAttack < 0.5f) &&
				_pWD->pEnemyBot && 
				(pT->m_uAttackFlags & CGroundCombat::FLAG_HASTARGETLOS) &&
				!_pWD->pDamagedByMem &&
				(pszBTAName = AIBTA_EventToBTAName("N*E_SEES", pBrain)))
		{
			if (ai_TalkModeBegin(pBrain,
							AIBrain_TalkModeCB,
							(void*) pszBTAName,
							pBrain->GetAIMover()->GetEntity(),
							1,
							TRUE, //STOP
							FALSE, //DON"T LOOK
							0))
			{
				//did it
				int i =2;
			}
		}
		else if (pT->m_pEnemy &&
			(!pT->m_pEnemy->IsInWorld() || 
			( _pWD->pEnemyBot && ( _pWD->pEnemyBot->m_nPossessionPlayerIndex == -1 ) && _pWD->pEnemyBot->IsDeadOrDying()) ||
			( _pWD->pEnemyBot && ( _pWD->pEnemyBot->m_nPossessionPlayerIndex >= 0 ) && _pWD->pEnemyBot->IsDead() ) ) )
		{
			//must have killed enemy

			pszBTAName = AIBTA_EventToBTAName("N*E_TAUN", pBrain);
			if (!pszBTAName)
			{
				pszBTAName = AIBTA_EventToBTAName("N*E_LAFF", pBrain);
			}
			if (pszBTAName)
			{
				ai_TalkModeBegin(pBrain,
								AIBrain_TalkModeCB,
								(void*) pszBTAName,
								pBrain->GetAIMover()->GetEntity(),
								1,
								TRUE, //STOP
								TRUE, //LOOK/.DON"T LOOK
								pBrain->GetLeader()->GetAIMover()->GetEntity()->Guid());
			}
		}
		else if (	pT->m_fDamageTakenPerSec >= 0.1f &&  //stops taunting when getting damaged
					pBrain->GetAIMover()->IsTalkingWithAnim() &&
					_pWD->pBot &&
					_pWD->pBot->m_pActiveTalkInst &&
					AIBTA_IsBTATaunt(_pWD->pBot->m_pActiveTalkInst->GetTalkData()->m_pszSourceFileName))
		{
			ai_TalkModeEnd(pBrain);
		}

	}
	
	// if
	//		no sight of enemy for a really long time,
	//
	// then 
	//		attack mode fails
	//
	u16 uAttackTimeWithoutLOS = 30;
	if ((pT->m_uAttackFlags & CGroundCombat::FLAG_CANTFINDENEMY) &&
		_pWD->uNowTime - pT->m_uLostEnemyTime > uAttackTimeWithoutLOS)
	{
		pT->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;	 //couldn't find that guy!
	}

	return AIFSM_WORK_CB_RETURN;
}


//shield off,  on
f32 _kfTitanMinRange[2] = {40.0f, 20.0f};
f32 _kfTitanMaxRange[2] = {100.0f, 50.0f};


BOOL InRules_ARS_Titan0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());
	_GARangeSetParams(	pT,
						5,		//u8 uDodgeTimeMin,   
						10,		//u8 uDodgeTimeMax,   
						2,		//u8 uDodgeOutTimeMin,
						3,		//u8 uDodgeOutTimeMax,
						4,		//u8 uDamageDodgeTimeMin
						pBrain->GetAIMover()->GetRadiusXZ()+_kfTitanMinRange[bShieldOn],	   //f32 fRangeMin
						pBrain->GetAIMover()->GetRadiusXZ()+_kfTitanMaxRange[bShieldOn],	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_SMART,
						0.5f+_pWD->fAggression*_kfTitanEraptSmartDelayTimeBase);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Titan0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

	//when titan0 is in base state, tactic should always be RANGE

	//adjust range times depending on whether the titan sheild is on.
	_GARangeSetParams(	pT,
						5,		//u8 uDodgeTimeMin,   
						10,		//u8 uDodgeTimeMax,   
						2,		//u8 uDodgeOutTimeMin,
						3,		//u8 uDodgeOutTimeMax,
						4,		//u8 uDamageDodgeTimeMin
						pBrain->GetAIMover()->GetRadiusXZ()+_kfTitanMinRange[bShieldOn],	   //f32 fRangeMin,      
						pBrain->GetAIMover()->GetRadiusXZ()+_kfTitanMaxRange[bShieldOn],	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_SMART,
						0.5f+_pWD->fAggression*_kfTitanEraptSmartDelayTimeBase);

	//give range attack time
	f32 fTitanInvestigateMarkAfterTime = pT->m_fEraptLosIsLostWaitTime + 3 + (u16) 2*(pBrain->GetWorldAddRandom() < 0.5f) - (u16) (2.0f*_pWD->fAggression);

	//if
	//	 Haven't seen the enemy for some time (aggression is a factor) &&
	//   currently am more than _kfInvestigateMarkMinDist feet from enemymark  (prevents rule from firing repeatedly)
	//then
	//   Investigate the mark
	//
	if (pT->m_fTimeWithoutLOS > fTitanInvestigateMarkAfterTime &&
		!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS) &&  //
		(pT->m_uRangeTacticSmartEraptDecisionTimeOut > 0 && (pT->m_AttackPtSearch.m_uFailureCount > 5 || (pT->m_uRangeTacticSmartEraptDecisionTimeOut < _pWD->uNowTime))) &&
		_pWD->fDistToEnemyMark >= _kfInvestigateMarkMinDist+_pWD->pMover->GetRadiusXZ())
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_TITAN0_INVESTIGATEMARK);
	}
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_Titan0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GARangeSetParams(	pT,
						4,		   //u8 uDodgeTimeMin,	   //Never Dodge
						0,		   //u8 uDodgeTimeMax,   
						4,		   //u8 uDodgeOutTimeMin,   //Never Dodge
						0,		   //u8 uDodgeOutTimeMax,
						3,			//u8 uDamageDodgeTimeMin
						2.0f,	   //f32 fRangeMin,      
						4.0f,	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Titan0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

	//if
	//   seen the enemy since pT tactic began.
	//then
	//   go to base state
	if ((u16) pT->m_fTimeWithoutLOS < (_pWD->uNowTime - pT->m_uTacticInitTime))
	{
 		pT->ChangeRuleSetState(CGroundCombat::ARS_TITAN0_BASE);
	}
	//if
	//	 close to the mark but don't see enemy or
	//	 been stuck for some time
	//then
	//   go to search state
	else if ((_pWD->fDistToEnemyMark < _kfInvestigateMarkMinDist) ||
		(!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE) && (_pWD->uNowTime - pT->m_uTacticInitTime) > 2.0f+pBrain->GetInfreqCycleRandom()*5.0f))
			 //!pBrain->HasMovedOneFootSince(_kuInvestigateMarkFailureTime))
	{
		//kick into search mode, then eventually back to our job if no enemy can be found
		pT->ChangeRuleSetState(CGroundCombat::ARS_TITAN0_SEARCH);
	}
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_Titan0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GASearchSetParams(pT);

	return AIFSM_INIT_CB_DONE;
}

BOOL DoRules_ARS_Titan0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	//
	//If 
	//		sees the enemy
	//then
	//		go back to base state
	//
	if (pT->m_fTimeWithoutLOS < 0.7f)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_TITAN0_BASE);
	}
	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_Titan0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeRuleSetState(CGroundCombat::ARS_TITAN0_BASE);
	return AIFSM_INIT_CB_DONE;
}


extern f32 _CalcJumpVelFor45DegreeTakeoffTrajectory(f32 dy, f32 dxz, f32 a);

BOOL bHasTakenToTheAirForAStomp = FALSE;
u16 uNextSuperStompTimeOut = 0;
u32 uJumpType = 0;

BOOL _Titan_ShouldStomp(CAIBrain* pTitanBrain, CEntity* pBotInQuestion)
{
	BOOL bYes = TRUE;
	if (pBotInQuestion->AIBrain() &&
		(pBotInQuestion->AIBrain()->GetAIMover()->GetHeight()) >= (pTitanBrain->GetAIMover()->GetHeight()-1.0f))
	{
		bYes = FALSE;
	}

	if (bYes &&
		pBotInQuestion->AIBrain() &&
		pBotInQuestion->AIBrain()->GetAIMover()->GetCurMech())
	{
		bYes = FALSE;
	}

	return bYes;
}


BOOL DoRuleSet_Titan0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//	FASSERT(pBrain->GetAIMover()->GetEntity() && pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTTITAN);
	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

#ifdef _DEBUG
	FSMStateHandle uState = pT->m_RuleSetStateFSM.GetActiveStateHandle();
	cchar* pszStateName = pT->m_RuleSetStateFSM.GetActiveStateName();
	switch (uState)
	{
		case FSMSTATEHANDLE_INVALID:
			break;
		case CGroundCombat::ARS_TITAN0_BASE:
			break;
		case CGroundCombat::ARS_TITAN0_INVESTIGATEMARK:
			break;
		case CGroundCombat::ARS_TITAN0_SEARCH:
			break;
		default:
			FASSERT(pT->m_RuleSetStateFSM.IsStateChangePending());
			break;
	}
#endif

	pT->m_RuleSetStateFSM.Work();


	//
	// Titan Jump Logic
	//
	if (pT->m_uAttackFlags & (CGroundCombat::FLAG_JUST_DID_FAILED_MELEE | CGroundCombat::FLAG_JUST_DID_SUCCESSFUL_MELEE))
	{
		//just finished a mellee attack, set the melee timeOut
		pT->m_uMeleeTimeOut = aiutils_GetTimeOutRandMinMax(pT->m_uMeleeTimeMin, pT->m_uMeleeTimeMax);
	}


	BOOL bHasRequestedAStomp = FALSE;

	if (_pWD->pMover->CanJump() && pT->m_pEnemy && _Titan_ShouldStomp(pT->GetBrain(), pT->m_pEnemy))
	{
		//stomp quick just to get guys away from me.
		if ((_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTTITAN) &&
			(pT->m_uAttackFlags & CGroundCombat::FLAG_HASTARGETLOS) &&
			pT->m_uMeleeTimeOut < aiutils_GetCurTimeSecsU16() && 
			!_pWD->pBot->IsJumping() &&
			!bHasTakenToTheAirForAStomp &&
			!(pT->m_uAttackFlags & CGroundCombat::FLAG_BOT_WAS_DOING_SPECIAL_MOVE) &&
			(_pWD->fDistToEnemyMark < (f32)pT->m_uMeleeRad+_pWD->pMover->GetRadiusXZ()))
		{
			

			switch (uJumpType)
			{
				case 0:
					_pWD->pMover->m_Controls.JumpHigh(); //stomp to get guys away from me.
					bHasRequestedAStomp = TRUE;
					break;
				case 1:
					//stomp with an initial velocity of straight up
					_pWD->pMover->m_Controls.Jump();
					bHasTakenToTheAirForAStomp = TRUE;
					bHasRequestedAStomp = TRUE;
					break;
				case 2:
				//Jump at the player and try to squish him
					if (fmath_RandomChoice(100) < _pWD->uAlternateRuleSetOdds)	//uAlternateRuleSetOdds is odds of doing a crazy stomp
					{

						CFVec3A Clearance = _pWD->pMover->GetEyePos();
						Clearance.y+=_pWD->pMover->GetHeight();
						if (aiutils_IsLOSObstructed_IgnoreBots(_pWD->pMover->GetEyePos(), Clearance, _pWD->pBot))
						{
							_pWD->pMover->m_Controls.JumpHigh(); 
							bHasRequestedAStomp = TRUE;
						}
						else
						{

							CFVec3A GoalLoc;
							GoalLoc = _pWD->EnemyMark;
							GoalLoc.y+=15.0f;

							//f32	 aiutils_ComputeTrajectoryFromXZVel(CFVec3A& rVelocityVec, const CFVec3A& rStartPos,const CFVec3A& rEndPos,f32 fXZVelocity, f32 fGravity=32.0f);
							f32 fIdealJumpDist = _pWD->pMover->GetLoc().DistXZ(GoalLoc);
							f32 fIdealJumpSpeed = _CalcJumpVelFor45DegreeTakeoffTrajectory(GoalLoc.y - _pWD->pMover->GetLoc().y, fIdealJumpDist, _pWD->pBot->m_fGravity);


							CFVec3A AdjustedGoalLoc = CFVec3A::m_Null;
							if (fIdealJumpSpeed > 0.0f)
							{
								FMATH_CLAMP(fIdealJumpSpeed, 0.0f,  50.0f); //safety
								//cheap leading code
								f32 fTimeOfTravel = _pWD->fDistToEnemyMark/fIdealJumpSpeed;
								AdjustedGoalLoc = _pWD->pEnemyBot->m_VelocityXZ_WS;//m_LastKnownVel;
								AdjustedGoalLoc.Mul(0.6f*fTimeOfTravel);		//I'm feel'n a little 0.6-ish right now
								f32 fTotalLeadingVel = AdjustedGoalLoc.SafeUnitAndMag(AdjustedGoalLoc);
								FMATH_CLAMP(fTotalLeadingVel, 0.0f,  50.0f); //safety
								GoalLoc.Add(AdjustedGoalLoc);
							}

							CFVec3A IdealVec;
							IdealVec.Sub(GoalLoc, _pWD->pMover->GetLoc());
							IdealVec.y = 0.0f;
							if (IdealVec.SafeUnitAndMag(IdealVec) > 0.0f)
							{
								IdealVec.y = 1.0f;
								IdealVec.Mul(fIdealJumpSpeed);  
								_pWD->pMover->m_Controls.JumpVec(IdealVec);
							}
							else
							{
								IdealVec = CFVec3A::m_UnitAxisY;
								IdealVec.Mul(10.0f);
								_pWD->pMover->m_Controls.JumpVec(IdealVec);
							}
						}
						bHasTakenToTheAirForAStomp = TRUE;
						bHasRequestedAStomp = TRUE;
					}
					else
					{
						uJumpType = 0;
					}
					break;
			}
		}

		if (bHasRequestedAStomp)
		{
			uJumpType++;
			if (uJumpType > 1)
			{
				uJumpType = 0;
			}
		}

		if (!bHasRequestedAStomp)
		{
			if (bHasTakenToTheAirForAStomp && _pWD->pBot->IsJumping())
			{
				if (!((CBotTitan*)_pWD->pBot)->IsStomping() && _pWD->pBot->m_Velocity_WS.y > 0.0f)
				{
					if (_pWD->pBot->IsInAir() && _pWD->pBot->m_Velocity_WS.y < 3.0f)
					{
						_pWD->pMover->m_Controls.Jump(); //stomp
					}
					else if (_pWD->fDistToEnemyMarkXZ < _pWD->pMover->GetRadiusXZ())
					{
						_pWD->pMover->m_Controls.Jump(); //stomp

						f32 fYVel = _pWD->pBot->m_Velocity_WS.y;
						_pWD->pBot->ZeroVelocity();
						CFVec3A VertVel;
						VertVel.Zero();
						VertVel.y = fYVel;
						_pWD->pBot->ApplyVelocityImpulse_WS(VertVel);

					}
				}
			}
			else if (!_pWD->pBot->IsJumping())
			{
				bHasTakenToTheAirForAStomp = FALSE;
			}
		}
	}

	pT->DoGenericTauntWork();

	//
	//  Special RULE that happens no matter what the sub mode of rs0 is
	//
	if (_pWD->pBot && _pWD->pBot->IsWalkingDead())
	{
		pT->ChangeRuleSet(CAIBrain::ATTACKRULESET_0);	   //will automatically kick into hide/panic mode whenthis happens
	}
	

	//
	//	React to Environmental stuff
	//
	pT->DoGenericEnviroReactRules(	TRUE,	 //Dodge Grenades
						FALSE,	 //Dodge Vehicles
						TRUE);	 //Play DamageBTA

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
	return AIFSM_WORK_CB_RETURN;
}


void CGroundCombat::DoWeaponAimAndFireWork_Titan(void)
{
	CAIWeaponCtrl* pChainWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
	CAIWeaponCtrl* pRocketWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(1);
	if (!(pChainWeaponCtrl && pRocketWeaponCtrl) )
	{
		return;
	}
	pChainWeaponCtrl->Work();	 //call this once a frame, before asking any questions of the weaponnctrl obj
	pRocketWeaponCtrl->Work();

	if (m_pBrain->GetAIMover()->IsTalkingWithAnim())
	{
		return;
	}

	f32 _fIdealChainGunRangeMin = 25.0f;
	f32 _fIdealChainGunRangeMax = 70.0f+10.0f*GetBrain()->GetInfreqCycleRandom();
	BOOL bOverrideTargetLock = FALSE;
	if (!s_pWD->pMover->CanFirePrimary() && !s_pWD->pMover->CanFireSecondary())
	{
		return;  //this titan has neither primary or secondary
	}

	if (m_pEnemy &&
		s_pWD->pDamagedByMem && (s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs) < 2)
	{
		_fIdealChainGunRangeMin = 0.0f;
		m_uAttackFlags |= FLAG_TRIGGERHAPPY;
	}
	else if (!s_pWD->pMover->CanJump() ||
		(m_pEnemy && !_Titan_ShouldStomp(GetBrain(), m_pEnemy)))
	{
		_fIdealChainGunRangeMin = 0.0f;	   //this might chafe a bit with titan since he can't aim that close up..
	}

	if (!s_pWD->pMover->CanFirePrimary())
	{
		if (!s_pWD->pMover->CanJump() || !s_pWD->pMover->CanMove())
		{
			_fIdealChainGunRangeMax = 0.0f;
			if (m_pBrain->GetInfreqCycleRandom() < 0.4f)
			{
				bOverrideTargetLock = TRUE;
			}
		}
		_fIdealChainGunRangeMax = _fIdealChainGunRangeMax*s_pWD->pMover->GetEntity()->NormHealth();
	}


	BOOL bFireChain = FALSE;
	BOOL bFireRocket = FALSE;
	BOOL bCanFireRocketInNearFuture = s_pWD->pMover->CanFireSecondary();
	u32 bAimRocket = FALSE;
	if (pChainWeaponCtrl->IsBursting())
	{
		bFireChain = TRUE;
	}
	else if (s_pWD->fDistToEnemyMark >= _fIdealChainGunRangeMax)
	{
		bFireRocket = TRUE;
		bAimRocket = TRUE;
	}
	else if ((s_pWD->pDamagedByMem && (s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs) < 2 && !pChainWeaponCtrl->IsBursting()) ||
			 (m_uFireOddsTimeOut < s_pWD->uNowTime && (pChainWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&			 //IsReady this is so that fireoddstimeout doesn't get set if there is no we we could fire
			 ((s_pWD->fDistToEnemyMark > _fIdealChainGunRangeMin && s_pWD->fDistToEnemyMark < _fIdealChainGunRangeMax) ||  !bCanFireRocketInNearFuture)))
	{
		m_uFireOddsTimeOut = s_pWD->uNowTime +1;

		if (m_fTimeWithoutLOS < 1.0f && 
			s_pWD->pMover->HasTargetLock()) // HasTargetLock means that the weapon will fire angle is within some internal tolerance to be considered a "lock" circa 20degrees
		{
			u32 uFireOdds = 100;
			if (m_uAttackFlags & FLAG_HASTARGETLOS || s_pWD->pDamagedByMem)
			{
				uFireOdds = 97;
			}
			else
			{
				uFireOdds = 5;
				if (m_fTimeWithoutLOS < 3.0f)
				{
					uFireOdds += (u16) ((f32)80*m_fTimeWithoutLOS/3.0f);
				}
			}
/*
			//being farther away from target decreases the desire to take a shot
			f32 fMaxDistRelatedFireOddsDampening = 0.25f;	  //findfix: adjust these for different units possibly
			f32 fFireOddsDampeningInnerRad = _fIdealChainGunRangeMin;
			f32 fFireOddsDampeningOutterRad = _fIdealChainGunRangeMax;
			fDistRelatedDampeningFeetThreshold += fDistRelatedDampeningFeetThreshold*s_pWD->fAggression; //how far away
			f32 fDistRelatedDampening = fMaxDistRelatedDampening;
			if (uFireOdds && s_pWD->fDistToEnemyMark < fDistRelatedDampeningFeetThreshold)
			{
				fDistRelatedDampening = fMaxDistRelatedDampening*s_pWD->fDistToEnemyMark/fDistRelatedDampeningFeetThreshold;
				fDistRelatedDampening += (1.0f - fDistRelatedDampening)*s_pWD->fAggression;
			}
			uFireOdds = (u8) (uFireOdds*fDistRelatedDampening);
  */
			if (fmath_RandomChoice(100) < uFireOdds)
			{
				bFireChain = TRUE;
			}
		}
		else if (!m_pEnemy)
		{
			if (m_uAttackFlags & FLAG_CANTFINDENEMY &&
				m_pBrain->GetWorldAddRandom() < 0.77f)
			{
				if (s_pWD->uNowTime - (u16)m_fAttackThoughtInitTime < 2)
				{
					bFireChain = TRUE;
				}
			}

			if (!bFireChain &&
				s_pWD->fAggression > 0.75f &&
				s_pWD->pDamagedByMem &&
				s_pWD->uNowTime < m_uBlindResponseToDamageTimeOut && 
				(s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs< 2) )
			{
				bFireChain = TRUE;
			}
		}
		else if (m_pEnemy &&
				!(m_pEnemy->TypeBits() & ENTITY_BIT_BOT))
		{
			bFireChain = TRUE;
		}
	}

	if (this->m_fDamageTakenPerSec > 0.075f)
	{
		bFireRocket = TRUE;
	}

	if (!s_pWD->pMover->CanAimPrimary() && this->m_fDamageTakenPerSec > 0.025f)
	{
		bFireRocket = TRUE;
	}

//	m_uAttackFlags |= FLAG_TRIGGERHAPPY; test code
	if (s_pWD->pMover->CanFirePrimary() &&
		pChainWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY) &&
		(bFireChain || (m_uAttackFlags & FLAG_TRIGGERHAPPY)))
	{  
		pChainWeaponCtrl->NotifyFire();
		s_pWD->pMover->m_Controls.Fire();
	}
	else
	{
		int yy = 44;
	}

	if (s_pWD->pMover->CanFireSecondary() &&
		m_pEnemy &&
		pRocketWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY) &&
		(s_pWD->pMover->HasTargetLock() || bOverrideTargetLock) &&
		(bFireRocket || (m_uAttackFlags & FLAG_TRIGGERHAPPY)))
	{  
		CFVec3A RocketSafetyEndPt;
		RocketSafetyEndPt.Sub(this->m_LastAim, s_pWD->pMover->GetEyePos());
		if (RocketSafetyEndPt.SafeUnitAndMag(RocketSafetyEndPt) > 0.0f)
		{
			RocketSafetyEndPt.Mul(kfTitanRocketSafetyRange);
			RocketSafetyEndPt.Add(s_pWD->pMover->GetEyePos());
			if (!aiutils_IsLOSObstructed_IgnoreBots(s_pWD->pMover->GetEyePos(), RocketSafetyEndPt, s_pWD->pBot))
			{
				pRocketWeaponCtrl->NotifyFire();
				s_pWD->pMover->m_Controls.Fire2();
				aiutils_DebugTrackRay(s_pWD->pMover->GetEyePos(), RocketSafetyEndPt, TRUE);
			}
			else
			{
				aiutils_DebugTrackRay(s_pWD->pMover->GetEyePos(), RocketSafetyEndPt, FALSE);
			}
		}
	}


	//AIM!
	if (bAimRocket)
	{
		Titan_RocketAiming();
	}
	else
	{
		DoLagWeaponAiming(0); //0 is the chaingun ID
	}
}


f32 _kfZombieMinRange = 1.0f;
f32 _kfZombieMaxRange = 10.0f;
f32 _kfZombieHideFromDamagePerSecThresh = 0.08f;
f32 _kfZombieHideDamageBase = 2.0f;
f32 _kfZombieHideDamageCourageScale = 4.0f;
u8 uZombieSearchSpeedMin = 60;
u8 uZombieSearchSpeedMax = 90;

BOOL InRules_ARS_Zombie0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	_GARangeSetParams(	pT,
						2,		   //u8 uDodgeTimeMin,   
						4,		   //u8 uDodgeTimeMax,   
						2,		   //u8 uDodgeOutTimeMin,
						4,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						_kfZombieMinRange,	   //f32 fRangeMin,      
						_kfZombieMaxRange,	  //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);

	return AIFSM_INIT_CB_DONE;
}

f32 _fZombieMyRight = 5.0f;
BOOL DoRules_ARS_Zombie0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	f32 fEnemyInMechMeleeRangeBoost = 0.0f;
	if (_pWD->pEnemyBot && _pWD->pEnemyBot->GetCurMech())
	{
		fEnemyInMechMeleeRangeBoost = _pWD->pEnemyBot->GetCurMech()->m_fCollCylinderRadius_WS-2.0f;
	}

	u32 uZombieRank = aigroup_GetRankAmongAllies(pT->m_pEnemy, _pWD->pBot, ENTITY_BIT_BOTZOM);
	if (uZombieRank <= 2)
	{
		_GARangeSetParams(	pT,
						2,		   //u8 uDodgeTimeMin,   
						4,		   //u8 uDodgeTimeMax,   
						2,		   //u8 uDodgeOutTimeMin,
						4,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						fEnemyInMechMeleeRangeBoost+_kfZombieMinRange,	  //f32 fRangeMin,      
						fEnemyInMechMeleeRangeBoost+_kfZombieMaxRange,	  //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);
	}
	else
	{
		f32 fZombieExtraMinRange = 20.0f+fEnemyInMechMeleeRangeBoost;
		fZombieExtraMinRange+= _pWD->pBot->m_fCollCylinderRadius_WS*(uZombieRank-1);
		_GARangeSetParams(	pT,
							2,		   //u8 uDodgeTimeMin,   
							4,		   //u8 uDodgeTimeMax,   
							2,		   //u8 uDodgeOutTimeMin,
							4,		   //u8 uDodgeOutTimeMax,
							1,			//u8 uDamageDodgeTimeMin
							fEnemyInMechMeleeRangeBoost+_kfZombieMinRange+fZombieExtraMinRange,	  //f32 fRangeMin,      
							fEnemyInMechMeleeRangeBoost+_kfZombieMaxRange+fZombieExtraMinRange,	  //f32 fRangeMax
							CGroundCombat::ERAPTUSEAGECTRL_NEVER,
							0.0f);

	}

	_pWD->pMover->m_uMoverFlags &= ~CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
	if (pT->m_pEnemy &&
		_pWD->pBot &&
		_pWD->pBot->TypeBits() & ENTITY_BIT_BOTZOM &&
		(!_pWD->pEnemyBot || !_pWD->pEnemyBot->GetCurMech() || !(_pWD->pEnemyBot->GetCurMech()->TypeBits() & ENTITY_BIT_SITEWEAPON)) &&
		(((CBotZom*) _pWD->pBot)->IsSlashing() ||
		(_pWD->pEnemyBot && _pWD->pEnemyBot->IsPlayerBot() && uZombieRank == 0 && pT->m_fTimeWithoutLOS < 0.25f && _pWD->fDistToEnemyMarkXZ < 30.0f)))
	{
		_pWD->pMover->m_uMoverFlags |= CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
		pT->m_uAttackFlags |= CGroundCombat::FLAG_MANUAL_MOVE_CONTROL;	//cleared every frame


		CFVec3A MyRight = CFVec3A::m_Null;

		if (uZombieRank == 1)
		{
			MyRight = _pWD->pMover->GetTorsoLookAtXZ();
			f32 fTmp = MyRight.x;
			MyRight.x = MyRight.z;
			MyRight.z = -fTmp;
			MyRight.Mul(_pWD->pEnemyBot->m_fCollCylinderRadius_WS+_fZombieMyRight);  //zombies are lefties, so aim for the right when swinging
		}
		if (uZombieRank == 2)
		{
			MyRight = _pWD->pMover->GetTorsoLookAtXZ();
			f32 fTmp = MyRight.x;
			MyRight.x = MyRight.z;
			MyRight.z = -fTmp;
			MyRight.Mul(_pWD->pEnemyBot->m_fCollCylinderRadius_WS+_fZombieMyRight);  //zombies are lefties, so aim for the right when swinging
		}

		CFVec3A Goal = pT->m_pEnemy->MtxToWorld()->m_vPos;
		Goal.Add(MyRight);
		CFVec3A OverShoot;
		OverShoot.Sub(Goal, _pWD->pMover->GetLoc());
		OverShoot.y = 0.0f;
		OverShoot.Mul(5.0f);
		OverShoot.Add(Goal);
		_pWD->pMover->MoveToward(OverShoot,  NULL);	 //try to drive righ through, while swinging
	}


	//
	// Speed Control
	//

	//	 pgm... commented the setting of a stun speed out for now, made zombies too vulnerable.

	if (pT->m_pEnemy &&	pT->m_fTimeWithoutLOS < 0.5f &&
		_pWD->fDistToEnemyMark > 40.0f+_pWD->pEnemyBot->m_fCollCylinderRadius_WS)
	{
		pBrain->PopBaseSpeedSetting();
		pBrain->PushBaseSpeedSetting(100);
	}
	else
	{

		//
		//
		//
		if (_pWD->pEnemyBot && 
			_pWD->pEnemyBot->IsPlayerBot() &&
			_pWD->fDistToEnemyMark < _pWD->pEnemyBot->m_fCollCylinderRadius_WS+15.0f &&
			_pWD->pDamagedByMem &&
			_pWD->uNowTime - _pWD->pDamagedByMem->m_uWhenTimeSecs < 2 &&
			pBrain->GetInfreqCycleRandom() < 0.6f*(1.0f-_pWD->fAggression))
		{
			pBrain->PopBaseSpeedSetting();
			pBrain->PushBaseSpeedSetting((u8)((f32)pT->m_uAttackMoveSpeed*(0.15f+ ((_pWD->uNowTime - _pWD->pDamagedByMem->m_uWhenTimeSecs))*0.5f)));
		}
		else
		{
			pBrain->PopBaseSpeedSetting();
			pBrain->PushBaseSpeedSetting(pT->m_uAttackMoveSpeed);
		}

	}


	//
	// Bunch of extra complicated 
	//  rules and sub-states that zombie does
	//  when it is fighting players
	//
	if ((_pWD->uNowTime - pT->m_uRuleSetStateInitTime) > 1 &&
		aiutils_IsPlayer(_pWD->pEnemyBot) &&
		aiutils_GetCurTimeSecs() - pT->m_fAttackThoughtInitTime > 5.0f)
	{

		//give range attack time
		f32 fZombieSearchAfterTimeWithoutLOS = 0.25f+pT->GetBrain()->GetInfreqCycleRandom()*0.75f;
		f32 fQuitAttackWithoutLOSTime = 10.0f + 10.0f*_pWD->fAggression + pBrain->GetWorldAddRandom()*10.0f;

		//if
		//	 Haven't seen or heard the enemy for short amount of time awhile
		//   and am pretty close to where I thought he was,
		//then
		//
		//   start searching
		//
		if (pT->m_pEnemy &&
			pT->m_fTimeWithoutLOS > fZombieSearchAfterTimeWithoutLOS &&
			_pWD->fDistToEnemyMark < _kfZombieMinRange+_pWD->pMover->GetRadiusXZ()+3.0f &&
			(!_pWD->pHeardMem || ((_pWD->uNowTime - _pWD->pHeardMem->m_uWhenTimeSecs) > 3)))
		{
			pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIE0_SEARCH);
		}
		else  if (pT->m_fDamageTakenPerSec > _kfZombieHideFromDamagePerSecThresh &&
				(pT->m_fTimeWithoutLOS > fZombieSearchAfterTimeWithoutLOS ||
				(_pWD->fDistToEnemyMark > _kfZombieMaxRange+0.5f &&
				(!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_RANGE) &&
					pT->m_uRangePathFailureCount > 3))))   //base->Hidefromdamage
		{
			pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIE0_HIDEFROMDAMAGE);

			if (!pBrain->GetAIMover()->IsTalkingWithAnim())
			{
				//BTAPLAY
				pT->CombatMode_PlayBTA("P*E_RUNS", FALSE, FALSE);
			}
		}
		// if
		//		have an enemy	   &&
		//		 Haven't seen or heard in a long time ||
		//		 just can't get close enough to do damage
		// then 
		//		go to quitterstate, which means go back to where I spawned from and scatter my pieces, waiting for player to come again
		//
		else if (	!(_pWD->fCourage > 0.99f && _pWD->fAggression > 0.99f && _pWD->fIntelligence > 0.99f) &&
					((pT->m_fTimeWithoutLOS > fQuitAttackWithoutLOSTime && (!_pWD->pHeardMem || ((_pWD->uNowTime - _pWD->pHeardMem->m_uWhenTimeSecs) > 3))) ||
					((pT->m_uRangePathFailureCount > 10 || pBrain->GetAIMover()->IsStuck(5.0f)) && (!_pWD->pEnemyBot || (_pWD->uNowTime - (u16) pT->m_fEnemyMarkLastSmallGridChangeTime) > 5) &&  !_pWD->pDamageDealtMem && ((CBotZom*) _pWD->pBot)->GetZomMode() == CBotZom::ZOMMODE_FORMING)))
		{
			pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIE0_QUITTER);
		}
	}
 
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_Zombie0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_SEARCH);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GASearchSetParams(pT);

	pT->GetBrain()->PopBaseSpeedSetting();
	pT->GetBrain()->PushBaseSpeedSetting(uZombieSearchSpeedMin+ (u8) fmath_RandomChoice(uZombieSearchSpeedMax-uZombieSearchSpeedMin));

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Zombie0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//
	//if 
	//		finds the enemy
	//then
	//		go back to base state
	//
	
	if (pT->m_fTimeWithoutLOS < (_pWD->uNowTime - pT->m_uTacticInitTime) || 
		(_pWD->pHeardMem && ((_pWD->uNowTime - _pWD->pHeardMem->m_uWhenTimeSecs) < (_pWD->uNowTime - pT->m_uTacticInitTime))))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIE0_BASE);
	}

	return AIFSM_WORK_CB_RETURN;
}

BOOL InRules_ARS_Zombie0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_PEEKABOO);
	pT->m_uLastPeekABooBailOut = _pWD->uNowTime;  
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GAPeekABooSetParams(pT,						 
						2, 4,						 //timer for when to come out of cover!          (min > max means never)                                                                                  
						2, 4);						 //timer for how long to stay out of cover under normal conditions

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Zombie0_HideFromDamage(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	if ((pT->m_fTimeWithLOS > 0.15f && pT->m_fEnemyMarkLastSmallGridChangeTime > pT->m_uRuleSetStateInitTime) ||
		(pT->m_fDamageTakenPerSec < _kfZombieHideFromDamagePerSecThresh &&
			(_pWD->uNowTime - pT->m_uRuleSetStateInitTime) > _kfZombieHideDamageBase+_pWD->fCourage*_kfZombieHideDamageCourageScale) &&
			(pT->m_fTimeWithoutLOS >_kfZombieHideDamageBase+_pWD->fCourage*_kfZombieHideDamageCourageScale))
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIE0_BASE);
	}
			 
	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_Zombie0_Quitter(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_NONE);

					
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Zombie0_Quitter(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	if (!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS))
	{
		pT->RequestSearch(pT->m_AttackOrigin, CGroundCombat::SEARCHREASON_NEWATTACKPOS, pT->GetBrain()->GetWakeupRadius());
	}
	
	BOOL bTmp;
	if (!_LocalPoiSearchWork(pT, &bTmp) || pT->m_uAttackFlags & CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED)   //this function sits and waits for thepath to the NEWATTACKPOS to be completed
	{   //failure means path could not be acquired, or bot didn't make it all the way.
		//when NEWATTACKPOS is reached,	 CGroundCombat::FLAG_STRATEGIC_ATTACK_PT_REACHED is set
		if (_pWD->pBot && _pWD->pBot->TypeBits() & ENTITY_BIT_BOTZOM && !(_pWD->pBot->IsSleeping() || _pWD->pBot->IsWaking()) )
		{
			_pWD->pBot->Sleep();
		}

		pT->GetBrain()->GetKnowledge().ForgetAllId(MEMORY_LARGE_SEEN);
		pT->GetBrain()->GetKnowledge().ForgetAllId(MEMORY_LARGE_HEARD);
		pT->GetBrain()->GetKnowledge().ForgetAllId(MEMORY_LARGE_DAMAGE_DEALT);
		pT->GetBrain()->GetKnowledge().ForgetAllId(MEMORY_LARGE_DAMAGED_BY);
		pT->GetBrain()->GetKnowledge().ForgetAllId(MEMORY_LARGE_COLLIDED);
		pT->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;	 //give up attack
	}
	else if (_pWD->pDamageDealtMem && 
			 _pWD->pDamageDealtMem->m_uWhenTimeSecs > pT->m_uRuleSetStateInitTime &&
			 _pWD->uNowTime - pT->m_uRuleSetStateInitTime > 2)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIE0_BASE);
	}



	return AIFSM_WORK_CB_RETURN;
}


BOOL OutRules_ARS_Zombie0_Quitter(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	u8 uAttackPtSlot;
	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
	{
		pT->FreeRequestSlot(uAttackPtSlot);
	}
	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_Zombie0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeRuleSetState(CGroundCombat::ARS_ZOMBIE0_BASE);
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_Zombie0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//	FASSERT(pBrain->GetAIMover()->GetEntity() && pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTZOMBIE);
#ifdef _DEBUG
	switch (pT->m_RuleSetStateFSM.GetActiveStateHandle())
	{
		case FSMSTATEHANDLE_INVALID:
			break;
		case CGroundCombat::ARS_ZOMBIE0_BASE:
			break;
		case CGroundCombat::ARS_ZOMBIE0_HIDEFROMDAMAGE:
			break;
		case CGroundCombat::ARS_ZOMBIE0_QUITTER:
			break;
		case CGroundCombat::ARS_ZOMBIE0_SEARCH:
			break;
		default:
			FASSERT(pT->m_RuleSetStateFSM.IsStateChangePending());
			break;
	}
#endif

	if (_pWD->fDistToEnemyMarkXZ < 13.0f &&
		!_pWD->pBot->IsJumping() &&
		((_pWD->uNowTime & 3) == (pT->GetBrain()->GetGUID() & 3)) &&
		_pWD->EnemyMark.y > _pWD->pMover->GetEntity()->MtxToWorld()->m_vPos.y+_pWD->pMover->GetHeight()*0.75f &&
		_pWD->EnemyMark.y < _pWD->pMover->GetEntity()->MtxToWorld()->m_vPos.y+_pWD->pMover->GetHeight()*5.0f &&
		_pWD->pBot->m_fSpeedXZ_WS  < 1.0f)
	{
		_pWD->pMover->m_Controls.Jump();
	}


	pT->m_RuleSetStateFSM.Work();


	//
	//  zombie hack logic
	//
	if (pT->m_uAttackFlags & (CGroundCombat::FLAG_JUST_DID_FAILED_MELEE | CGroundCombat::FLAG_JUST_DID_SUCCESSFUL_MELEE))
	{
		//just finished a mellee attack, set the melee timeOut
		pT->m_uMeleeTimeOut = aiutils_GetTimeOutRandMinMax(pT->m_uMeleeTimeMin, pT->m_uMeleeTimeMax);
	}

	f32 fEnemyInMechMeleeRangeBoost = 0.0f;
	if (_pWD->pEnemyBot && _pWD->pEnemyBot->GetCurMech())
	{
		fEnemyInMechMeleeRangeBoost = _pWD->pEnemyBot->GetCurMech()->m_fCollCylinderRadius_WS;
	}
	if (pT->m_pEnemy && 
		pT->m_fTimeWithoutLOS < 2.0f &&
		_pWD->fDistToEnemyMark < fEnemyInMechMeleeRangeBoost+(f32) pT->m_uMeleeRad)
	{
		_pWD->pMover->m_Controls.Fire();
	}



	//
	//	React to Environmental stuff
	//
	pT->DoGenericTauntWork();
	pT->DoGenericEnviroReactRules(	TRUE,	 //Dodge Grenades
							FALSE,	 //Dodge Vehicles
							TRUE);	 //Play DamageBTA


	//
	// Controls what to look at
	//
	if (!_pWD->bHasLOSThisFrame && 
		pT->m_fTimeWithoutLOS < 20.0f && 
		pT->m_RuleSetStateFSM.GetActiveStateHandle() == CGroundCombat::ARS_ZOMBIE0_QUITTER)
	{
		pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_DIR_ENEMY;
	}
	else
	{
		pT->m_uAttackFlags &= ~CGroundCombat::FLAG_TORSO_DIR_ENEMY;
	}
	return AIFSM_WORK_CB_RETURN;
}


//
//
//  The Rat
//
//
//

typedef enum
{
	RAT0_RuleSet_STATE_IDLE,
	RAT0_RuleSet_STATE_INIT_SPLINE,
	RAT0_RuleSet_STATE_REQUEST_PATH,
	RAT0_RuleSet_STATE_REQUEST_PATH_WAITING,
	RAT0_RuleSet_STATE_REQUEST_PATH_FAILED,
	RAT0_RuleSet_STATE_FOLLOWING_PATH,
	RAT0_RuleSet_STATE_COMPLETED_PATH,
	RAT0_RuleSet_STATE_COMPLETED_SPLINE,
	RAT0_RuleSet_STATE_NUM_STATES,
} Rat0_RuleSetState_e;

char *_pszRat0State[ RAT0_RuleSet_STATE_NUM_STATES ] =
{
		"IDLE",
		"INIT_SPLINE",
		"REQUEST_PATH",
		"REQUEST_PATH_WAITING",
		"REQUEST_PATH_FAILED",
		"FOLLOWING_PATH",
		"COMPLETED_PATH",
		"COMPLETED_SPLINE",
};

static void _StuffRuleSet_Rat0Data(	CAIMemoryLarge* pMem,
									CESpline* pVehicleSpline,		// pointer to spline entity on which vehicle is travelling
									u16 uVehicleSplinePoint,		// current vehicle move-to point on spline
									u16 eRat0RuleSetState,
									s16 nRequestPathRetryCount)
{
	FASSERT(pMem);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PokeU32(pBuff, (u32) pVehicleSpline);				//2
	pBuff = pMem->PokeU16(pBuff, uVehicleSplinePoint);			//2
	pBuff = pMem->PokeU16(pBuff, eRat0RuleSetState);			//2
	pBuff = pMem->PokeS16(pBuff, nRequestPathRetryCount);		//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryLarge));
}


static void _ExtractRuleSet_Rat0Data(	CAIMemoryLarge* pMem,
									CESpline** ppVehicleSpline,		// pointer to spline entity on which vehicle is travelling
									u16 *puVehicleSplinePoint,		// current vehicle move-to point on spline
									u16 *peRat0RuleSetState,
									s16 *pnRequestPathRetryCount)
{
	FASSERT(pMem && puVehicleSplinePoint && peRat0RuleSetState && pnRequestPathRetryCount && ppVehicleSpline);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PeekU32(pBuff, (u32*) ppVehicleSpline);				//2
	pBuff = pMem->PeekU16(pBuff, puVehicleSplinePoint);			//2
	pBuff = pMem->PeekU16(pBuff, peRat0RuleSetState);			//2
	pBuff = pMem->PeekS16(pBuff, pnRequestPathRetryCount);		//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryLarge));
}


BOOL InitRuleSet_Rat0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//set default values for the data specific to RuleSet_Rat0
	CESpline* pVehicleSpline = NULL;
	u16 uVehicleSplinePoint = 0;
	u16 eRat0RuleSetState = RAT0_RuleSet_STATE_IDLE;
	s16 nRequestPathRetryCount = 0;

	//store them in a mini-heap called CAIBraim::m_Knowledge
	CAIMemoryLarge* pDataStore = NULL;
	if (pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_RAT0_RULESET_DATA, (CAIMemorySmall**) &pDataStore) ||
		(pDataStore = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_RAT0_RULESET_DATA, 2)))
	{
		_StuffRuleSet_Rat0Data(pDataStore, pVehicleSpline, uVehicleSplinePoint, eRat0RuleSetState, nRequestPathRetryCount);
	}

	// way to stop CGroundCombat from timing out like happens normally to bots with no enemies
	pT->m_uAttackFlags |= CGroundCombat::FLAG_DISABLE_ENEMY_RELATED_EXIT_CONDITIONS;

	return AIFSM_INIT_CB_DONE;
}


#define _REQUEST_PATH_MAX_RETRIES	( 2 )
BOOL DoRuleSet_Rat0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	CFVec3A vTargetPoint, vDelta;
	const CFVec3A* paPts;
	f32 fHeight;
	BOOL bResult;
	CAIMover* pMover;
	CVehicle *pVehicle;
	u32 uIndex, uClosest;
	f32 fDistSq, fClosestDistSq;
	u8 uAttackPtSlot;

	//
	// Unpack Rat0 RulSet data for this function
	//
	CESpline* pVehicleSpline = NULL;
	u16 uVehicleSplinePoint = 0;
	u16 eRat0RuleSetState = RAT0_RuleSet_STATE_IDLE;
	s16 nRequestPathRetryCount = 0;
	CAIMemoryLarge* pDataStore = NULL;
	if ( pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_RAT0_RULESET_DATA, (CAIMemorySmall**) &pDataStore) ||
		(pDataStore = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_RAT0_RULESET_DATA, 2)))
	{
		_ExtractRuleSet_Rat0Data(pDataStore, &pVehicleSpline, &uVehicleSplinePoint, &eRat0RuleSetState, &nRequestPathRetryCount);
		pDataStore->ResetTimer(2);
	}

	// way to stop CGroundCombat from timing out like happens normally to bots with no enemies
	pT->m_uAttackFlags |= CGroundCombat::FLAG_DISABLE_ENEMY_RELATED_EXIT_CONDITIONS;

	// way to tell CGroundCombat not to do any torso or head logic for me.
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;

	if (_pWD->pMover->GetCurMech() &&
		_pWD->pMover->GetCurMech()->TypeBits() & ENTITY_BIT_VEHICLE &&
		((CVehicle*)_pWD->pMover->GetCurMech())->GetSplinePath())
	{
		pT->m_uThoughtFlags &= ~CAIThought::THOUGHTFLAG_LOD_TREAT_GOAL_WITH_JOB_LOD_RULES;
	}
	else
	{
		pT->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_LOD_TREAT_GOAL_WITH_JOB_LOD_RULES;
	}



	pMover = pBrain->GetAIMover();
	if( pMover )
	{
		pVehicle = (CVehicle*) pMover->GetEntity();
	}

//	ftext_Printf( 0.15f, 0.50f, "~f1~C92929299~w1~al~s0.69State: %s", _pszRat0State[eRat0RuleSetState] );

	if( pVehicle )
	{
		if( pVehicle->GetSplinePath() == NULL && pVehicleSpline != NULL )
		{
			// if vehicle spline becomes NULL while following path, 
			// stop following spline, reset variables and go to idle.
			eRat0RuleSetState = RAT0_RuleSet_STATE_COMPLETED_SPLINE;
		}
	}

	switch( eRat0RuleSetState )
	{
		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_IDLE:

			if( pMover )
			{
				if( pVehicle )
				{
					if( pVehicle->GetSplinePath() != NULL )
					{
						pVehicleSpline = pVehicle->GetSplinePath();
						eRat0RuleSetState = RAT0_RuleSet_STATE_INIT_SPLINE;
					}
				}
			}
			

#if 0
			if( pVehicleSpline != NULL )
			{
				// check entire spline over graph
				CAIMover* pMover = pBrain->GetAIMover();
				if( pMover )
				{
					CVehicle *pVehicle = (CVehicle*) pMover->GetEntity();
					if( pVehicle )
					{
						if( !CPatrolPath::Register( pVehicleSpline,(u8) pVehicle->m_fCollCylinderRadius_WS, (u8) pVehicle->m_fCollCylinderHeight_WS, FALSE ) )
						{
							FASSERT_NOW;
						}
					}
				}
			}
#endif
			break;

		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_INIT_SPLINE:
			FASSERT( pVehicleSpline );
			FASSERT( pMover );
			FASSERT( pVehicle );

			// Calculate the spline point towards which the vehicle should begin moving.
			paPts = pVehicleSpline->PointArray();
			FASSERT( paPts );
			fClosestDistSq = FMATH_MAX_FLOAT;
			uClosest = -1;
			for( uIndex = 0; uIndex < pVehicleSpline->PointCount(); uIndex++ )
			{
				// Test to see if pT spline point is closer to vehicle than any previous points
				fDistSq = pVehicle->MtxToWorld()->m_vPos.DistSq( paPts[uIndex] );
				if( fDistSq < fClosestDistSq )
				{
					// See if pT spline point is in front of vehicle
					vDelta.Set( paPts[uIndex] );
					vDelta.Sub( pVehicle->MtxToWorld()->m_vPos );
					if( pVehicle->MtxToWorld()->m_vFront.Dot( vDelta ) > 0.0f )
					{
						fClosestDistSq = fDistSq;
						uClosest = uIndex;
					}
				}
			}
// debug test
//if( uClosest == 0 ) uClosest = 1;
			if( uClosest >= 0 )
			{
				uVehicleSplinePoint = uClosest + 1;
				SCRIPT_MESSAGE( "Vehicle %s found spline point %d on path %s", pVehicle->Name(), uClosest, pVehicleSpline->Name() );
			}
			else
			{
				SCRIPT_ERROR( "Vehicle %s couldn't find a spline point on path %s", pVehicle->Name(), pVehicleSpline->Name() );
			}

			nRequestPathRetryCount = 0;
			eRat0RuleSetState = RAT0_RuleSet_STATE_REQUEST_PATH;
			if( pT->FindRequestSlotInUse( CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot ) )
			{
				pT->FreeRequestSlot( uAttackPtSlot );
			}
			FASSERT( !pT->FindRequestSlotInUse( CGroundCombat::SEARCHREASON_NEWATTACKPOS) );

			{
				CAICarMover* pMover = (CAICarMover*) pBrain->GetAIMover();
				if( pMover )
				{
					// reset stuck counter and clear stuck flag
					pMover->m_fStuckForSecs = 0.0f;
					pMover->m_uAICarMoverFlags = 0;
				}
			}
			break;

		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_REQUEST_PATH:
			FASSERT( pVehicleSpline );
			if( !pVehicleSpline )
			{
				eRat0RuleSetState = RAT0_RuleSet_STATE_IDLE; 
			}

			FASSERT( uVehicleSplinePoint <= pVehicleSpline->PointCount() );
			if( uVehicleSplinePoint >= pVehicleSpline->PointCount() )
			{
				eRat0RuleSetState = RAT0_RuleSet_STATE_COMPLETED_SPLINE; 
				break;
			}

			paPts = pVehicleSpline->PointArray();
			FASSERT( paPts );
			if( !paPts )
			{
				eRat0RuleSetState = RAT0_RuleSet_STATE_IDLE; 
			}

			vTargetPoint.Set( paPts[ uVehicleSplinePoint ] );
			aiutils_FloorHeightAt( vTargetPoint, &fHeight, 50.0f );
			if( fHeight > vTargetPoint.y )
			{
				// only adjust vertical height if point is below the terrain
				vTargetPoint.y = fHeight;
			}
			bResult = pT->RequestSearch( vTargetPoint, CGroundCombat::SEARCHREASON_NEWATTACKPOS, 20.0f /* fudge distance */ );
//			FASSERT( bResult );
			if( bResult )
			{
				eRat0RuleSetState = RAT0_RuleSet_STATE_REQUEST_PATH_WAITING; 
			}
			else
			{
				// do I have to do pT if RequestSearch returns false?
				if( pT->FindRequestSlotInUse( CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot ) )
				{
					pT->FreeRequestSlot( uAttackPtSlot );
				}
				eRat0RuleSetState = RAT0_RuleSet_STATE_REQUEST_PATH_FAILED; 
			}

			break;

		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_REQUEST_PATH_WAITING:
			if( pT->IsSearchDone( CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot ) )
			{
				// pathfinder found a path for us, start following it
				pT->Attack_AssignPath( uAttackPtSlot );
				nRequestPathRetryCount = 0;
				eRat0RuleSetState = RAT0_RuleSet_STATE_FOLLOWING_PATH; 
			}

			if( pT->IsSearchFailed(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot ) )
			{
				if( pT->m_aSearchQuery[ uAttackPtSlot ].GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_END )
				{
					DEVPRINTF( "AI: DoRuleSet_Rat0 Search failed due to bad end.\n" );
				}
				if( pT->m_aSearchQuery[ uAttackPtSlot ].GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_START )
				{
					DEVPRINTF( "AI: DoRuleSet_Rat0 Search failed due to bad start.\n" );
				}

				if( pT->FindRequestSlotInUse( CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot ) )
				{
					pT->FreeRequestSlot( uAttackPtSlot );
				}

				eRat0RuleSetState = RAT0_RuleSet_STATE_REQUEST_PATH_FAILED; 
			}
			break;

		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_REQUEST_PATH_FAILED:
			++nRequestPathRetryCount;
			if( nRequestPathRetryCount < _REQUEST_PATH_MAX_RETRIES )
			{
				eRat0RuleSetState = RAT0_RuleSet_STATE_REQUEST_PATH; 
			}
			else
			{
				// move to next point and try again
				nRequestPathRetryCount = 0;
				eRat0RuleSetState = RAT0_RuleSet_STATE_COMPLETED_PATH; 
			}
			break;

		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_FOLLOWING_PATH:
			if ( pT->m_bPathJustCompleted && pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_NEWATTACKPOS )
			{
				// vehicle just arrived at end of path
				if( pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
				{
					// free path
					pT->FreeRequestSlot(uAttackPtSlot);
				}

				eRat0RuleSetState = RAT0_RuleSet_STATE_COMPLETED_PATH; 
			}
#if 0
			if( pMover && aiutils_FTotalLoopSecs() - pMover->m_fLinkStartTime > pMover->m_fLinkEstimateTime * 3.0f )
			{
				// vehicle has not completed a link in a long time, try requesting a new path
				eRat0RuleSetState = RAT0_RuleSet_STATE_REQUEST_PATH_FAILED;
			}
#endif
			break;

		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_COMPLETED_PATH:
			FASSERT( !pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS ) );

			// try to get a path to a new point
			if( pT->FindRequestSlotInUse( CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot ) )
			{
				pT->FreeRequestSlot( uAttackPtSlot );
			}

			if( pVehicleSpline )
			{
				++uVehicleSplinePoint;

				if( uVehicleSplinePoint < pVehicleSpline->PointCount() )
				{
					eRat0RuleSetState = RAT0_RuleSet_STATE_REQUEST_PATH; 
				}
				else
				{
					eRat0RuleSetState = RAT0_RuleSet_STATE_COMPLETED_SPLINE; 
				}
			}
			else
			{
				eRat0RuleSetState = RAT0_RuleSet_STATE_IDLE; 
			}
			break;

		//---------------------------------------------------------------------
		case RAT0_RuleSet_STATE_COMPLETED_SPLINE:
			pVehicleSpline = NULL;
			if( pVehicle )
			{
				pVehicle->SetSplinePath( NULL );
			}

			uVehicleSplinePoint = 0;
			if( pT->FindRequestSlotInUse( CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot ) )
			{
				pT->FreeRequestSlot( uAttackPtSlot );
			}
			eRat0RuleSetState = RAT0_RuleSet_STATE_IDLE; 
			break;

		//---------------------------------------------------------------------
		default:
			FASSERT_NOW;
			break;
	}

//	CAIMover* pMover = pBrain->GetAIMover();
//	if( pMover )
//	{
//		CVehicle *pVehicle = (CVehicle*) pMover->GetEntity();
//		if( pVehicle )
//		{
//			if( pVehicleSpline )
//			{
//				u32 nIndex;
//				CFVec3A vColor;
//				const CFVec3A* paPts = pVehicleSpline->PointArray();
//
//				vColor.Set( 0.0f, 0.2f, 0.8f );
//				nIndex = uVehicleSplinePoint - 1;
//				FMATH_CLAMPMIN( nIndex, 1 );
//				for( ; nIndex < pVehicleSpline->PointCount(); nIndex++ )
//				{
//					pVehicle->m_PhysObj.DrawLine( (CFVec3A*)&paPts[nIndex - 1], (CFVec3A*)&paPts[nIndex] );
//				}
//			}										  _
//		}
//	}

	if (_pWD->pBot->IsDying())
	{
		pT->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;	 //couldn't find that guy!
	}

	//store them in a mini-heap called CAIBraim::m_Knowledge
	if (pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_RAT0_RULESET_DATA, (CAIMemorySmall**) &pDataStore) ||
		(pDataStore = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_RAT0_RULESET_DATA, 2)))
	{
		_StuffRuleSet_Rat0Data(pDataStore, pVehicleSpline, uVehicleSplinePoint, eRat0RuleSetState, nRequestPathRetryCount);
	}

	return AIFSM_WORK_CB_RETURN;
}


CAIKnowledge _ScientistClassKnowledge;
f32 _fScientistClassDemeanor = 0.0f;


void CGroundCombat::DoWeaponAimAndFireWork_Scientist(void)
{
}


BOOL InRules_ARS_Scientist0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Scientist0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_Scientist0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	_GARangeSetParams(	pT,
				4,		   //u8 uDodgeTimeMin,   
				0,		   //u8 uDodgeTimeMax,   
				4,		   //u8 uDodgeOutTimeMin,
				0,		   //u8 uDodgeOutTimeMax,
				3,		   //u8 uDamageDodgeTimeMin
				8.f,	   //f32 fRangeMin,      
				1000.f,	   //f32 fRangeMax
				CGroundCombat::ERAPTUSEAGECTRL_NEVER,
				0.0f);	 //erapt los wait time

	return AIFSM_INIT_CB_DONE;
}

extern f32 _GetDistSqOffOfLOF(CAIBrain *pShooter, CAIBrain* pFriend, const CFVec3A& LineOfFire, f32 fDistToCheck);

BOOL DoRuleSet_Scientist0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	if (_pWD->pDamagedByMem && !(_pWD->pDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED))
	{
		_pWD->pDamagedByMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
		_fScientistClassDemeanor += _pWD->pDamagedByMem->m_fLargeData;
	}

	if (_pWD->pBot->TypeBits() & ENTITY_BIT_BOTSCIENTIST)
	{
		CBotScientist* pScientist = (CBotScientist*) _pWD->pBot;

		if (pT->m_pEnemy &&
			_pWD->fDistToEnemyMark < (f32)pT->m_uMeleeRad)
		{
			_pWD->pMover->m_Controls.AimAt(_pWD->EnemyMarkTagPos);
			_pWD->pMover->m_Controls.Fire();
		}
		else
		{
 			if (_pWD->uLastFrameNowTime != _pWD->uNowTime)
			{
				CAIBrain*pShooterBrain = pBrain;
				CBot* pSelfBot = _pWD->pBot;
				CFVec3A LOF = pSelfBot->MtxToWorld()->m_vFront;
				f32 fDistToCheck = pSelfBot->m_fCollCylinderRadius_WS + pScientist->GetMaxAttackRange();

				CAIMemoryLarge* pSeen = NULL;
				if (pShooterBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_SEEN, (CAIMemorySmall**)&pSeen))
				{   
					while (pSeen)
					{
						if (pSeen->m_pEntity &&
							pSeen->m_pEntity->ComputeUnitHealth() < 0.9f &&
							pSeen->m_pEntity != pT->m_pEnemy &&		 //works even when m_pEnemy is NULL
							pSeen->m_pEntity->AIBrain() &&
							(pT->s_pWD->uNowTime - pSeen->m_uWhenTimeSecs) < 2 &&
							!(pSelfBot && pSeen->m_pEntity->TypeBits() & ENTITY_BIT_BOT &&	pSelfBot->GetCurMech() == pSeen->m_pEntity || ((CBot*)pSeen->m_pEntity)->GetCurMech() == pSelfBot) &&
							aiutils_IsFriendly(pShooterBrain->GetAIMover()->GetEntity(), pSeen->m_pEntity) &&
							_GetDistSqOffOfLOF(pShooterBrain, pSeen->m_pEntity->AIBrain(), LOF, fDistToCheck) < (pSeen->m_pEntity->AIBrain()->GetAIMover()->GetRadiusXZ()*pSeen->m_pEntity->AIBrain()->GetAIMover()->GetRadiusXZ()))
						{
 							//zap thy friend
							_pWD->pMover->m_Controls.AimAt(*(pSeen->m_pEntity->GetTagPoint(0)));
							_pWD->pMover->m_Controls.Fire2();
							break;
						}

						pSeen = (CAIMemoryLarge*) pShooterBrain->GetKnowledge().GetNextOldestOfId(pSeen);
					}
				}
			}


			if (pScientist && pScientist->GetTargetBot())
			{
				pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;
				_pWD->pMover->FaceToward(*(pScientist->GetTargetBot()->GetTagPoint(0)));
			}
		}
	}


	return AIFSM_WORK_CB_RETURN;
}