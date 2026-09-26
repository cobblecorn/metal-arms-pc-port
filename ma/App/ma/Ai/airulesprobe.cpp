#include "fang.h"
#include "AiBrainMems.h"
#include "AIBrain.h"
#include "AIFSM.h"
#include "AIGameUtils.h"
#include "AIMover.h"
#include "AINodePools.h"
#include "AITactics.h"
#include "AIThoughts3DBot.h"
#include "AIThoughtsGeneric.h"
#include "AIThoughtsGround.h"
#include "AIGroup.h"
#include "../BotProbe.h"
#include "../BotGlitch.h"

//
//
//  Probe Logic
//
//
//
//
enum
{
	PROBE_STRATEGY_NONE = 0,
	PROBE_STRATEGY_GRAB,
	PROBE_STRATEGY_SPIN 
};
static u16 _uNextStrategyChangeTimeOut = 0;
static u16 _uMagnetStateChangeTimeOut = 0;
static u8 _uCurProbeStrategy = PROBE_STRATEGY_NONE;
static CAIMemoryMedium* _pProbeAttackMemory = NULL;
static CBotProbe* _pThisProbe = NULL;
static u16 _uThisProbeRank = 0;

static void _ExtractProbeAttackParams(	CAIMemoryMedium* pMem)				
{
	FASSERT(pMem );
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PeekU16(pBuff, &_uNextStrategyChangeTimeOut);				//2
	pBuff = pMem->PeekU16(pBuff, &_uMagnetStateChangeTimeOut);					//2
	pBuff = pMem->PeekU8U8(pBuff, &_uCurProbeStrategy, NULL);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryLarge));
}


static void _StuffProbeAttackParams(	CAIMemoryMedium* pMem)				
{
	FASSERT(pMem);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PokeU16(pBuff, _uNextStrategyChangeTimeOut);			//2
	pBuff = pMem->PokeU16(pBuff, _uMagnetStateChangeTimeOut);			//2
	pBuff = pMem->PokeU8U8(pBuff, _uCurProbeStrategy, 0);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryMedium));
}



static void _ProbeAIBeginFrame(CAIBrain* pBrain, CGroundCombat* pT)
{
	if ( pBrain->GetKnowledge().CanRememberAny(MEMORY_MEDIUM_PROBEATTACKDATA, (CAIMemorySmall**) &_pProbeAttackMemory) ||
		(_pProbeAttackMemory = pBrain->GetKnowledge().RememberMedium(MEMORY_MEDIUM_PROBEATTACKDATA, 2)))
	{
		_ExtractProbeAttackParams(_pProbeAttackMemory);
		_pProbeAttackMemory->ResetTimer(2);
	}

	_uThisProbeRank = 0;
	if (pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTPROBE)
	{
		_pThisProbe = (CBotProbe*) pBrain->GetAIMover()->GetEntity();
		if (pT->m_pEnemy)
		{
			_uThisProbeRank = aigroup_GetRankAmongAllies(pT->m_pEnemy, _pThisProbe, ENTITY_BIT_BOTPROBE);
		}
	}
	else
	{
		_pThisProbe = NULL;
	}

}


static void _ProbeAIEndFrame(void)
{
	if (_pProbeAttackMemory)
	{
		_StuffProbeAttackParams(_pProbeAttackMemory);
	}
}


f32 _fGlobalProbeDelayMinRange = 0.0f;
f32 _fGlobalProbeDelayMaxRange = 0.0f;

f32 _fGlobalProbeSpinDelay = 0.0f;
f32 _fGlobalProbeGrabDelay = 0.0f;

u32 _uGlobalProbeFVid_nFrameCounter = 0;
u16 _uGlobalProbeSecs = 0;

void _SetGrabDelay(f32 fDelayTimeMin, f32 fDelayTimeMax,  BOOL bForce = FALSE)
{
	if (bForce || _fGlobalProbeGrabDelay == 0.0f)
	{
		f32 fNewDelay = fDelayTimeMin+ fmath_RandomFloat()*(fDelayTimeMax- fDelayTimeMin);
		if (fNewDelay > _fGlobalProbeGrabDelay)
		{
			_fGlobalProbeGrabDelay = fNewDelay;
		}
	}
}
void _UpdateProbeGlobals(void)
{
	if (CGroundCombat::s_pWD->uNowTime < 2 || CGroundCombat::s_pWD->uNowTime < _uGlobalProbeSecs)
	{
		_fGlobalProbeGrabDelay = 0.0f;


		_uGlobalProbeFVid_nFrameCounter = 0;
		_uGlobalProbeSecs = CGroundCombat::s_pWD->uNowTime;
	}
	else if (_uGlobalProbeFVid_nFrameCounter != FVid_nFrameCounter)
	{
		_uGlobalProbeFVid_nFrameCounter = FVid_nFrameCounter;
		_uGlobalProbeSecs = CGroundCombat::s_pWD->uNowTime;

		_fGlobalProbeGrabDelay-=FLoop_fPreviousLoopSecs;
		if (_fGlobalProbeGrabDelay < 0.0f)
		{
			_fGlobalProbeGrabDelay  = 0.0f;
		}
	}
}


BOOL InitRuleSet_Probe0(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_ProbeAIBeginFrame(pBrain, pT);
	
	pT->ChangeRuleSetState(CGroundCombat::ARS_PROBE0_BASE);
	pT->ChangeTactic(CGroundCombat::TACTIC_RANGE3D);
	_GARangeSetParams(	pT,
						1,		   //u8 uDodgeTimeMin,   
						0,		   //u8 uDodgeTimeMax,   
						1,		   //u8 uDodgeOutTimeMin,
						0,		   //u8 uDodgeOutTimeMax,
						1,		//u8 uDamageDodgeTimeMin
						0.0f,	   //f32 fRangeMin,      
						-_pWD->pMover->GetRadiusXZ(),	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);

	//flip this on, because the probe doesn't avoid players, he hunts them, and is actually allowed to collide with him.
	_pWD->pMover->m_uMoverFlags |= CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS;

	_ProbeAIEndFrame();

	return AIFSM_INIT_CB_DONE;
}


void _DoProbeStrategyRules(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	if (_pThisProbe && _pThisProbe->IsLeeching(NULL))
	{
		_SetGrabDelay(6.0f, 15.0f, FALSE);
	}

	BOOL uNewStrategy = PROBE_STRATEGY_SPIN;
	u8 uExtraStrategyChangeDelay = 0;

	if (pT->m_uAttackFlags & CGroundCombat::FLAG_JUST_DID_SUCCESSFUL_MELEE && 
		_uCurProbeStrategy == PROBE_STRATEGY_GRAB)
	{	//force stragy change to spin
		_uNextStrategyChangeTimeOut = 0;
		uExtraStrategyChangeDelay = 7;
		if (_pWD->fAggression>0.75f && pBrain->GetInfreqCycleRandom() > 0.5f)
		{
			pT->m_uMeleeTimeOut = aiutils_GetTimeOutRandMinMax((u8) 1, 2);	//min max are set at time special move began.  values depend on situation and typeof melee.
		}
		else
		{

		}

		uNewStrategy = PROBE_STRATEGY_SPIN;
	}
	else if ((pT->m_pEnemy &&
		((1.0f - pBrain->GetInfreqCycleRandom()) < 0.5f*_pWD->fCourage) &&
		 pT->m_pEnemy->TypeBits() & ENTITY_BIT_BOT && _pThisProbe->CanGrabBot((CBot*) pT->m_pEnemy)) &&
		(!_pWD->pDamagedByMem || 
		_pWD->uNowTime - _pWD->pDamagedByMem->m_uWhenTimeSecs > 5))
	{
		uNewStrategy = PROBE_STRATEGY_GRAB;
	}
	else 
	{
		uNewStrategy = PROBE_STRATEGY_SPIN;
	}

	if (uNewStrategy != _uCurProbeStrategy && _uNextStrategyChangeTimeOut < _pWD->uNowTime)
	{
		_uCurProbeStrategy = uNewStrategy;
		_uNextStrategyChangeTimeOut = uExtraStrategyChangeDelay + aiutils_GetTimeOutRandMinMax((u8)10,15);
	}
}


void _DoPositionWork(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//
	//  Melee Mode positoning, doesn't use pathfinding
	//
	CAIMemoryLarge* pStrikeMem = NULL;
	pT->GetBrain()->GetKnowledge().CanRememberAny(MEMORY_LARGE_PROBESTRIKE, (CAIMemorySmall**) &pStrikeMem);
	if (_uThisProbeRank == 0 && 
		pT->m_uAttackFlags & CGroundCombat::FLAG_BOT_WAS_DOING_SPECIAL_MOVE || _pThisProbe->IsUsingMagnet())
	{
		if (pT->m_fTimeWithLOS > 1.0f && _pWD->fDistToEnemyMark < 40.0f)
		{
			_pWD->pMover->m_uMoverFlags |= CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
			pT->m_uAttackFlags |= CGroundCombat::FLAG_MANUAL_MOVE_CONTROL;	//cleared every frame

			CFVec3A OverShoot;
			OverShoot.Sub(pT->m_pEnemy->MtxToWorld()->m_vPos, _pWD->pMover->GetLoc());
			OverShoot.y = 0.0f;
			OverShoot.Mul(5.0f);
			OverShoot.Add(pT->m_pEnemy->MtxToWorld()->m_vPos);
			_pWD->pMover->MoveToward(OverShoot,  NULL);	 //try to drive right through, while meleeing
			if (!pStrikeMem)
			{
				pStrikeMem = (CAIMemoryLarge*) pT->GetBrain()->GetKnowledge().RememberThis(MEMORY_LARGE_PROBESTRIKE, 3, 0);
				if (pStrikeMem)
				{
					pStrikeMem->m_Loc = _pWD->pMover->GetLoc().v3;
					
				}
			}
			else
			{
				pStrikeMem->ResetTimer(1);
			}
		}
	}
	else
	{
		_pWD->pMover->m_uMoverFlags &= ~CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
		if (pT->m_uRangePathFailureCount > 3)
		{
			if (pStrikeMem)
			{
				_pWD->pMover->m_uMoverFlags |= CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
				pT->m_uAttackFlags |= CGroundCombat::FLAG_MANUAL_MOVE_CONTROL;	//cleared every frame
				if (_pWD->pMover->MoveToward(pStrikeMem->m_Loc, NULL))		//try to drive right through, while meleeing
				{
					pT->GetBrain()->GetKnowledge().ForgetThis(pStrikeMem);
				}
			}
		}

		if (_uThisProbeRank == 0)
		{
			if (_uCurProbeStrategy == PROBE_STRATEGY_GRAB)
			{
				pT->m_uAttackFlags |= CGroundCombat::FLAG_FOLLOW_PATHS_WITH_PRECISION;

				//position myself to grab
				_GARangeSetParams(	pT,
								1,		   //u8 uDodgeTimeMin,   
								0,		   //u8 uDodgeTimeMax,   
								1,		   //u8 uDodgeOutTimeMin,
								0,		   //u8 uDodgeOutTimeMax,
								1,		//u8 uDamageDodgeTimeMin
								0.0f,	   //f32 fRangeMin,      
								-_pWD->pMover->GetRadiusXZ(),	   //f32 fRangeMax
								CGroundCombat::ERAPTUSEAGECTRL_NEVER,
								0.0f);
			}
			else if (_uCurProbeStrategy == PROBE_STRATEGY_SPIN)
			{
				pT->m_uAttackFlags &= ~CGroundCombat::FLAG_FOLLOW_PATHS_WITH_PRECISION;

				//position myself to spin
				_GARangeSetParams(	pT,
							1,		   //u8 uDodgeTimeMin,   
							0,		   //u8 uDodgeTimeMax,   
							1,		   //u8 uDodgeOutTimeMin,
							0,		   //u8 uDodgeOutTimeMax,
							1,			//u8 uDamageDodgeTimeMin
							10.0f,	   //f32 fRangeMin,      
							14,			//f32 fRangeMax
							CGroundCombat::ERAPTUSEAGECTRL_NEVER,
							0.0f);

			}
		}
		else
		{
			//position myself at range
			_GARangeSetParams(	pT,
							2,		   //u8 uDodgeTimeMin,   
							8,		   //u8 uDodgeTimeMax,   
							4,		   //u8 uDodgeOutTimeMin,
							8,		   //u8 uDodgeOutTimeMax,
							0,		//u8 uDamageDodgeTimeMin
							30.0f+_uThisProbeRank*_pWD->pMover->GetRadiusXZ(),	   //f32 fRangeMin,      
							40.0f+_uThisProbeRank*_pWD->pMover->GetRadiusXZ(),	   //f32 fRangeMax
							CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED,
							0.0f);
			pT->m_AttackPtSearch.m_uRepeatTimeMin = 4;
			pT->m_AttackPtSearch.m_uRepeatTimeMax = 16;
		}
	}

}

BOOL DoRuleSet_Probe0(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	FASSERT(_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTPROBE );

	_ProbeAIBeginFrame(pBrain, pT);

	_UpdateProbeGlobals();


	//
	// Any sub states can work
	//
	pT->m_RuleSetStateFSM.Work();


	//book keeping
	if (pT->m_uAttackFlags & CGroundCombat::FLAG_JUST_DID_SUCCESSFUL_MELEE)
	{
		//just finished a mellee successfull attack, set the melee timeOut
		pT->m_uMeleeTimeOut = aiutils_GetTimeOutRandMinMax(pT->m_uMeleeTimeMin, pT->m_uMeleeTimeMax);	//min max are set at time special move began.  values depend on situation and typeof melee.
	}
	else if (pT->m_uAttackFlags & CGroundCombat::FLAG_JUST_DID_FAILED_MELEE)
	{
		//don't set meleetimeout
	}

	//
	//	When to Change Strategy
	//
	_DoProbeStrategyRules(uControlParam, pParam1, pvParam2);


	//
	// Where to position self based on strategy and other states
	//
	_DoPositionWork(uControlParam, pParam1, pvParam2);

	//
	// start a grab	whenever opportunity presents itself as long as there is not a global grab delay
	//	   - regardless of wether strategy is grab or not
	BOOL bStartGrab = FALSE;
	if (_fGlobalProbeGrabDelay == 0.0f  &&
		_pWD->pEnemyBot &&
		_pThisProbe->CanGrabBot(_pWD->pEnemyBot) &&
		!(pT->m_uAttackFlags & CGroundCombat::FLAG_BOT_WAS_DOING_SPECIAL_MOVE))//pT->m_uMeleeTimeOut < aiutils_GetCurTimeSecsU16())
	{
		f32 fMinDistToGrab = 3.0;
		if ((pT->m_fTimeWithoutLOS  < 1.0f) &&
			pT->m_uMeleeTimeOut < aiutils_GetCurTimeSecsU16() && 
			(pT->m_pEnemy && pT->m_pEnemy->GetTagPoint(0)->DistSqXZ(_pThisProbe->MtxToWorld()->m_vPos) < fMinDistToGrab*fMinDistToGrab) && 
			(pT->m_pEnemy->GetTagPoint(0)->y - _pThisProbe->MtxToWorld()->m_vPos.y) < 7.0f && 
			(pT->m_pEnemy->GetTagPoint(0)->y - _pThisProbe->MtxToWorld()->m_vPos.y) > 0.0f)	
		{
			bStartGrab = TRUE;
		}
	}

	if (bStartGrab)
	{
		pT->m_uMeleeTimeMin = 7;	   //just incase we actually pull it off.
		pT->m_uMeleeTimeMax = 12;	   //just incase we actually pull it off.
		_pWD->pMover->m_Controls.Fire2(); //grab
	}

	
	//
	// do a spin
	//	 -when within range and 
	//   -strategy is spin, or opportunity presents itself if there is a global grab delab
	BOOL bStartSpin = FALSE;
	if (!bStartGrab &&
		(_uCurProbeStrategy == PROBE_STRATEGY_SPIN || _fGlobalProbeGrabDelay > 0.0f) &&
		!(pT->m_uAttackFlags & CGroundCombat::FLAG_BOT_WAS_DOING_SPECIAL_MOVE) &&
		pT->m_uMeleeTimeOut < aiutils_GetCurTimeSecsU16() &&
		(!_pWD->pEnemyBot || !(_pWD->pEnemyBot->TypeBits() & ENTITY_BIT_BOTGLITCH) || !((CBotGlitch *) _pWD->pEnemyBot)->IsLeeched()) || (_pWD->fAggression > 0.8f && pBrain->GetInfreqCycleRandom() < 0.5f))  //special rule that don't ever spin a guy if he is being leeched, unless your really aggressive and randomness permits
	{
		f32 fMinDistToSpinSq = 75.0f;//pT->m_uMeleeRad+_pThisProbe->GetSpinDeathZoneRadius())*(pT->m_uMeleeRad+_pThisProbe->GetSpinDeathZoneRadius();
		fMinDistToSpinSq*=fMinDistToSpinSq;
		if (((aiutils_FTotalLoopSecs() - pT->m_fAttackThoughtInitTime) < 0.25f) ||   //recently alerted
			(pT->m_fTimeWithoutLOS  < 1.0f &&
			(pT->m_pEnemy && pT->m_pEnemy->GetTagPoint(0)->DistSq(_pThisProbe->MtxToWorld()->m_vPos) < fMinDistToSpinSq)))
		{
			pT->m_uMeleeTimeMin = 2;
			pT->m_uMeleeTimeMax = 4;
			_pWD->pMover->m_Controls.Fire(); //spin
		}
	}

	if (bStartSpin)
	{
		pT->m_uMeleeTimeMin = 2; //reset spin timers for normal states 
		pT->m_uMeleeTimeMax = 4;
		_pWD->pMover->m_Controls.Fire(); //spin
	}


	//
	//  Magnet control
	//
	BOOL bUseMagnet = FALSE;
	f32 fMinMagTryRad = _pThisProbe->GetMagnetMaxRadius()*2.0f;
	if (_uCurProbeStrategy == PROBE_STRATEGY_GRAB &&
		pT->m_pEnemy && 
	//	(_pWD->uNowTime & 7)<4 &&
		(pT->m_pEnemy->GetTagPoint(0)->DistSqXZ(_pThisProbe->MtxToWorld()->m_vPos) < fMinMagTryRad*fMinMagTryRad) && 
		(pT->m_pEnemy->GetTagPoint(0)->y - _pThisProbe->MtxToWorld()->m_vPos.y) < _pThisProbe->GetMagnetMaxHeight() && 
		(pT->m_pEnemy->GetTagPoint(0)->y - _pThisProbe->MtxToWorld()->m_vPos.y) > -_pThisProbe->GetMagnetMaxHeight())	
	{
		bUseMagnet = TRUE;
	}

	if (!(bUseMagnet != _pThisProbe->IsUsingMagnet() && _uMagnetStateChangeTimeOut < _pWD->uNowTime))
	{
		bUseMagnet = _pThisProbe->IsUsingMagnet();
	}
	if (pT->m_pEnemy &&
		pT->m_pEnemy->TypeBits() & ENTITY_BIT_BOT &&
		bUseMagnet && !_pThisProbe->IsUsingMagnet())
	{
		_pThisProbe->BeginMagnet((CBot*) pT->m_pEnemy);
	}
	else if (!bUseMagnet && _pThisProbe->IsUsingMagnet())
	{
		_pThisProbe->EndMagnet();
	}

	
	pT->DoGenericTauntWork();

	_ProbeAIEndFrame();

	return AIFSM_WORK_CB_RETURN;
}

void CGroundCombat::DoWeaponAimAndFireWork_Probe(void)
{
	return;
}
