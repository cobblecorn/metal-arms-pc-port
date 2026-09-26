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
#include "AINodePools.h"
#include "AIPatrolPath.h"
#include "AIRooms.h"
#include "AITactics.h"
#include "AIThoughtsGeneric.h"
#include "AIThoughtsGround.h"
#include "../BotJumper.h"
#include "../Weapon.h"


enum
{
	J_STRATEGY_RANGE = 0,	//hang back, use cannons to do damage
	J_STRATEGY_MELEE,		//stay close to enemy, hit him with arms
	J_STRATEGY_SUPRISE,		//hang back, stay safe, then burst out and do strafe runs in the air
	J_STRATEGY_NONE,
	NUM_EG_STRATEGIES,
};
f32 _kafJumperMinRange[] = {25.0f, 0.0f, 60.0f, 0.0f};
f32 _kafJumperMaxRange[] = {70.0f, 9.0f, 250.0f, 9.0f};

BOOL bHoverDecision = FALSE;
f32 _fGlobalDiveNotify = 0.0f;
f32 _fTimeSinceGlobalDiveNotify = -1.0f;
f32 _fModeTimeout = 0.0f;
f32 _fLastJumperTime = 0;

static void _StuffJCombatData(	CAIMemoryMedium* pMem,
										u16 uNextStrategyChangeTimeOut,
										u8 uCurStrategy)
{
	FASSERT(pMem);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PokeU16(pBuff, uNextStrategyChangeTimeOut);			//2
	pBuff = pMem->PokeU8U8(pBuff, uCurStrategy, 0);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryMedium));
}


static void _ExtractJCombatData(	CAIMemoryMedium* pMem,
								u16 *puNextStrategyChangeTimeOut,	
								u8 *puCurStrategy)				
{
	FASSERT(pMem && puNextStrategyChangeTimeOut && puCurStrategy);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PeekU16(pBuff, puNextStrategyChangeTimeOut);				//2
	pBuff = pMem->PeekU8U8(pBuff, puCurStrategy, NULL);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryMedium));
}


static void _StuffJMeleeCombatData(	CAIMemoryLarge* pMem,
									f32 fLastDivePathFailTime,
									f32 fDiveRunStartedTime,
									u8 uDiveRunState)
{
	FASSERT(pMem);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PokeU8U8(pBuff, uDiveRunState, 0);	//2
	pBuff = pMem->PokeF32(pBuff, fLastDivePathFailTime);					//2
	pBuff = pMem->PokeF32(pBuff, fDiveRunStartedTime);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryLarge));
}


static void _ExtractJMeleeCombatData(	CAIMemoryLarge* pMem,
										f32 *pfLastDivePathFailTime,
										f32 *pfDiveRunStartedTime,
										u8 *puDiveRunState)
{
	FASSERT(pMem && pfLastDivePathFailTime && pfDiveRunStartedTime && puDiveRunState);
	u8 *pBuff = (u8*) &(pMem->m_uSmallData);
	pBuff = pMem->PeekU8U8(pBuff, puDiveRunState, NULL);	//2
	pBuff = pMem->PeekF32(pBuff, pfLastDivePathFailTime);					//2
	pBuff = pMem->PeekF32(pBuff, pfDiveRunStartedTime);						//2
	FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryLarge));
}


enum
{
	DIVERUN_STATE_NONE = 0,
	DIVERUN_STATE_ENROUTE,
	DIVERUN_STATE_WAIT_FOR_PATH,
	DIVERUN_STATE_DIVING
};

u8 _uDefaultStartStrategy = J_STRATEGY_MELEE;//RANGE;
f32 _fLastDivePathFailTime = -30.0f;
f32 _fDiveRunStartedTime = -30.0f;
u8  _uDiveRunState = DIVERUN_STATE_NONE;
u16 _uNextStrategyChangeTimeOut = 0;
u8 _uCurStrategy = _uDefaultStartStrategy;
CAIMemoryMedium* pJData = NULL;
CAIMemoryLarge* pJMeleeData = NULL;

void _JumperAIBeginFrame(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	if (aiutils_GetCurTimeSecs() < _fLastJumperTime-0.2f)
	{
		//reset global timers
		_fGlobalDiveNotify = 0.0f;
		_fTimeSinceGlobalDiveNotify = -1.0f;
		_fModeTimeout = 0.0f;
	}
	_fLastJumperTime = aiutils_GetCurTimeSecs();

	//
	//  Defaults if something is up with memory, or first time called ever.
	//
	if (pT->m_uRuleSet == CAIBrain::ATTACKRULESET_JUMPER0)
	{
		_uDefaultStartStrategy = J_STRATEGY_RANGE;
	}
	else if (pT->m_uRuleSet == CAIBrain::ATTACKRULESET_JUMPER1)
	{
		_uDefaultStartStrategy = J_STRATEGY_MELEE;
	}
	else
	{
		FASSERT(0);
	}
	if (pT->GetBrain()->GetAttrib(CAIBrain::ATTRIB_INTELLIGENCE) == 40)
	{
		_uDefaultStartStrategy = J_STRATEGY_MELEE;
	}
	_fLastDivePathFailTime = -30.0f;
	_fDiveRunStartedTime = -30.0f;
	_uDiveRunState = DIVERUN_STATE_NONE;
	_uNextStrategyChangeTimeOut = 0;
	_uCurStrategy = _uDefaultStartStrategy;


	if (pBrain->GetKnowledge().CanRememberAny(MEMORY_MEDIUM_JUMPERCOMBATDATA, (CAIMemorySmall**) &pJData))
	{
		_ExtractJCombatData(pJData, &_uNextStrategyChangeTimeOut, &_uCurStrategy);
		pJData->ResetTimer(2);
	}
	else
	{	//pick a strategy?

	}


	if (pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_JUMPERMELEECOMBATDATA, (CAIMemorySmall**) &pJMeleeData))
	{
		_ExtractJMeleeCombatData(pJMeleeData, &_fLastDivePathFailTime, &_fDiveRunStartedTime, &_uDiveRunState);
		pJMeleeData->ResetTimer(2);
	}
	else
	{	//pick a strategy?

	}
	
	_fTimeSinceGlobalDiveNotify = _pWD->fNowTime - _fGlobalDiveNotify;

}

void _JumperAIEndFrame(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	if (pJData ||
		(pJData = pBrain->GetKnowledge().RememberMedium(MEMORY_MEDIUM_JUMPERCOMBATDATA, 2)))
	{
		_StuffJCombatData(pJData, _uNextStrategyChangeTimeOut, _uCurStrategy);
	}

	if (pJMeleeData ||
		(pJMeleeData = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_JUMPERMELEECOMBATDATA, 2)))
	{
		_StuffJMeleeCombatData(pJMeleeData, _fLastDivePathFailTime, _fDiveRunStartedTime, _uDiveRunState);
	}
}
	

BOOL InRules_ARS_Jumper0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);

	_GARangeSetParams(	pT,
						1,		   //u8 uDodgeTimeMin,   
						0,		   //u8 uDodgeTimeMax,   
						1,		   //u8 uDodgeOutTimeMin,
						0,		   //u8 uDodgeOutTimeMax,
						1,		   //u8 uDamageDodgeTimeMin
						0.0f,	   //f32 fRangeMin,      
						-_pWD->pMover->GetRadiusXZ(),	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED,
						0.0f);

	pT->m_AttackPtSearch.SetAttackPtSearchParams(	5,  // u8 uRepeatTimeMin,
													15, // u8 uRepeatTimeMax,
													1,  // s8 nDirInc,
													0,  // u8 uDirMin,
													3,  // u8 uDirMax,
													10, // s8 nRangeXZInc
													15, // u8 uRangeXZMin
													60, // u8 uRangeXZMax
													0,
													0,
													0);
	return AIFSM_INIT_CB_DONE;
}




static void _HandleDiveRun(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	BOOL bQuitDive = FALSE;

	if (!_pWD->pBot || !(_pWD->pBot->TypeBits() & ENTITY_BIT_BOTJUMPER))
	{
		return ;
	}

	CAIHoverBotMover* pJMover = (CAIHoverBotMover*) _pWD->pMover;
	BOOL bStartDiveThisFrame = FALSE;
	u8 uPreMeleeSlot = 0xff;
	pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_PRE_MELEE, &uPreMeleeSlot);

	//
	// periodically position self for a dive bomb
	//
	switch(_uDiveRunState)
	{
		case DIVERUN_STATE_NONE:
			if (!_pWD->pBot->IsInAir())
			{
				if (pBrain->GetInfreqCycleRandom() < 0.1+0.9f*_pWD->fCourage &&
					pT->m_fTimeWithLOS > 0.5f &&
					_pWD->fNowTime - _fLastDivePathFailTime > 5.0f &&
					_pWD->fNowTime - _fDiveRunStartedTime > 5.0f &&
					(pT->m_fEnemyMarkLastSmallGridChangeTime > _fLastDivePathFailTime ||
					pBrain->GetLastXYZChangeTime() > _fLastDivePathFailTime))
				{
					FASSERT(!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_PRE_MELEE));


					f32 fMinDiveRange = 30.0f;
					f32 fMaxDiveRange = 150.0f;

					//
					// find a spot to dive from
					//
					GraphVert* pV = aimain_pAIGraph->FindClosestLOSVert2DInDonut_AdjustWithinRange(_pWD->pMover->GetLoc(), 5.0f, _pWD->EnemyMarkTagPos, fMinDiveRange, fMaxDiveRange);
					if (pV && 
						pV->GetRad() > _pWD->pMover->GetRadiusXZ() &&
						pV->GetHeight() > _pWD->pMover->GetHeight())
					{
						CFVec3A GoalInAir = _pWD->EnemyMarkTagPos;
						GoalInAir = pV->GetLocation();
						GoalInAir.x += (pV->GetRad()-_pWD->pMover->GetRadiusXZ()) * fmath_RandomBipolarUnitFloat();
						GoalInAir.z += (pV->GetRad()-_pWD->pMover->GetRadiusXZ()) * fmath_RandomBipolarUnitFloat();
						GoalInAir.y += pV->GetHeight()-_pWD->pMover->GetHeight();

						pT->RequestSearch(GoalInAir, CGroundCombat::SEARCHREASON_PRE_MELEE,  10.0f);
						pT->ChangeTactic(CGroundCombat::TACTIC_NONE);//disable the range tactic now..
						_fDiveRunStartedTime = _pWD->fNowTime;
						_uDiveRunState = DIVERUN_STATE_WAIT_FOR_PATH;
					}
				}
			}
			break;
		case DIVERUN_STATE_WAIT_FOR_PATH:
		

			
			//if
			//		an SEARCHREASON_PRE_MELEE pt path fails || 
			//      bot is stuck
			//then
			//		clear the search slot
			//
			if (uPreMeleeSlot == 0xff ||
				pT->m_aSearchQuery[uPreMeleeSlot].HasFailed() || 
				pT->m_aSearchQuery[uPreMeleeSlot].WasCanceled())
			{
				_fLastDivePathFailTime = _pWD->uNowTime;
				bQuitDive = TRUE;
			}
			//if
			//		a SEARCHREASON_PRE_MELEE path is ready && 
			//      not already following it.
			//then
			//		follow it
			else if (!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_PRE_MELEE) &&
				pT->IsSearchDone(CGroundCombat::SEARCHREASON_PRE_MELEE, &uPreMeleeSlot))
			{
				pT->Attack_AssignPath(uPreMeleeSlot);
				_uDiveRunState = DIVERUN_STATE_ENROUTE;
			}
			break;
		case DIVERUN_STATE_ENROUTE:

			if (uPreMeleeSlot != 0xff)
			{

				//if    
				//		just completed a SEARCHREASON_PRE_MELEE path
				//then
				//		clear the search slot  and dive
				//
				GraphVert* pV = aimain_pAIGraph->GetVert(pJMover->GetLastGraphVertId());
				if (pT->IsSearchDone(CGroundCombat::SEARCHREASON_PRE_MELEE) &&
					!pT->IsFollowingPath() &&
					(!pV ||
					 (pV->m_fHeightClearance < pJMover->GetHeight()*2.0f) ||
					 (pJMover->GetLoc().y > (pV->GetLocation().y + pV->m_fHeightClearance - pJMover->GetHeight()-2.0f))))	   //as high as I can go in this "goal vert
				{
					pT->FreeRequestSlot(uPreMeleeSlot);

					if (pT->m_fTimeWithLOS > 0.15f)
					{
						bStartDiveThisFrame = TRUE;
						_uDiveRunState = DIVERUN_STATE_DIVING;
					}
					else
					{
						bQuitDive = TRUE;
					}
				}
				else
				{
					if (pV) 
					{
						if (pV->m_fHeightClearance > pJMover->GetHeight()*2.0f &&
							(!pT->IsFollowingPath() || 
							(pBrain->GetInfreqCycleRandom() < 0.1f+0.9f*_pWD->fCourage)))
						{
							pJMover->HoverTowardY(pV->m_Location.y+pV->m_fHeightClearance-pJMover->GetHeight());
							if (!_pWD->pDamagedByMem)
							{
								pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;//only good for this frame
							}
						}
					}

					if (pJMover->GetLoc().DistSqXZ(pT->m_aSearchQuery[uPreMeleeSlot].GetGoalLoc()) < 10.0f*10.0f)
					{
						pJMover->FaceToward(pT->m_aSearchQuery[uPreMeleeSlot].GetGoalLoc());
						pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;//only good for this frame
					}
				}
			}
			else
			{
				bQuitDive = TRUE;
			}

			//dive bomb if opportunity presents itself.
			//health is a factor
		/*		if (_pWD->pBot && 
				_pWD->pBot->IsInAir() &&
				pT->m_fTimeWithLOS > 0.5f &&
				_pWD->pMover->GetLoc().y > _pWD->EnemyMarkTagPos.y+ 5.0f &&
				_pWD->fDistToEnemyMarkXZ < 60.0f  &&
				!(pT->m_uAttackFlags & CGroundCombat::FLAG_BOT_WAS_DOING_SPECIAL_MOVE) &&
				pT->m_uMeleeTimeOut < _pWD->uNowTime)
			{
				if (_pWD->uNowTime != _pWD->uLastFrameNowTime)
				{	//dice roll once per sec
					u32 uDiveBombOdds = (u32) (100.0f*(1.0f - _pWD->pBot->ComputeUnitHealth()));
					u32 uDiveBombOddsFloor = 0;
					if (_pWD->fNowTime - pBrain->GetLastXYZChangeTime() > 0.0f)
					{
						uDiveBombOddsFloor += (u16) ((_pWD->fNowTime - pBrain->GetLastXYZChangeTime())*25.0f);  //each second of stationary increases floor by 50
					}
					uDiveBombOddsFloor += (u16) (((_pWD->pMover->GetLoc().y -(_pWD->EnemyMarkTagPos.y+ 5.0f))*0.1f)*15.0f); //every ten feet over head high is 15
					FMATH_CLAMP(uDiveBombOddsFloor, 0, 100);
					FMATH_CLAMPMIN(uDiveBombOdds, uDiveBombOddsFloor);

					if (fmath_RandomChoice(100) < uDiveBombOdds)
					{
						bStartDiveThisFrame = TRUE;
					}
				}
			}
	*/
			break;
		case DIVERUN_STATE_DIVING:
			if (!((CBotJumper*)_pWD->pBot)->IsDiving())
			{
				bQuitDive = TRUE;
			}
			break;
	}

	if (bStartDiveThisFrame)
	{
		//findfix: should lead based on dive time
		_pWD->pMover->m_Controls.AimAt( _pWD->EnemyMark );
		_pWD->pMover->m_Controls.Fire2();
		_fGlobalDiveNotify = _pWD->fNowTime;
	}

	if (bQuitDive)
	{
		if (uPreMeleeSlot != 0xff)
		{
			pT->FreeRequestSlot(uPreMeleeSlot);
		}
		_uDiveRunState = DIVERUN_STATE_NONE;
		pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);//re-enable the range tactic now..
	}
}


BOOL DoRules_ARS_Jumper0_Base(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();


	//
	//  When to switch strategy
	//



	//
	// strategy specif stuff
	//
	switch (_uCurStrategy)
	{
		case J_STRATEGY_RANGE:
		{
			_GARangeSetParams(	pT,
								1,		   //u8 uDodgeTimeMin,   
								0,		   //u8 uDodgeTimeMax,   
								1,		   //u8 uDodgeOutTimeMin,
								0,		   //u8 uDodgeOutTimeMax,
								1,		   //u8 uDamageDodgeTimeMin
								_kafJumperMinRange[_uCurStrategy],	   //f32 fRangeMin,      
								_kafJumperMaxRange[_uCurStrategy],	   //f32 fRangeMax
								CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED,
								0.0f);

			//
			// periodically hover
			//
			if (_fModeTimeout < aiutils_GetCurTimeSecs())
			{
				bHoverDecision = FALSE;
				if (_pWD->pBot && _pWD->pBot->TypeBits() & ENTITY_BIT_BOTJUMPER)
				{
					bHoverDecision = (fmath_RandomFloat() < 0.1f+0.9f*_pWD->fCourage);//Courage if 0.5 means odds are 0.6, which plays pretty nicely. Courage of 0 means odds of flying are 0.1f every time cyclerand changes.
				}
				_fModeTimeout = aiutils_GetTimeOutRandMinMax(3.0f, 8.0f);
			}

			if (bHoverDecision == TRUE)
			{
				CAIHoverBotMover* pJMover = (CAIHoverBotMover*) _pWD->pMover;
				if (pJMover->GetLastGraphVertId()) 
				{
					GraphVert* pV = aimain_pAIGraph->GetVert(pJMover->GetLastGraphVertId());
					if (pV->m_fHeightClearance > pJMover->GetHeight()*2.0f)
					{
						pJMover->HoverTowardY(pV->m_Location.y+pV->m_fHeightClearance-pJMover->GetHeight());
					}
				}
			}

			if (((CBotJumper*) _pWD->pBot)->IsDiving())
			{
				bHoverDecision = FALSE;
				_fModeTimeout = aiutils_GetTimeOutRandMinMax(1.0f, 2.0f);
			}

			//dive bomb if opportunity presents itself.
			//health is a factor
			if (_pWD->pBot && 
				_pWD->pBot->IsInAir() &&
				pT->m_fTimeWithLOS > 0.5f &&
				_pWD->pMover->GetLoc().y > _pWD->EnemyMarkTagPos.y+ 5.0f &&
				_pWD->fDistToEnemyMarkXZ < 60.0f  &&
				!(pT->m_uAttackFlags & CGroundCombat::FLAG_BOT_WAS_DOING_SPECIAL_MOVE) &&
				pT->m_uMeleeTimeOut < _pWD->uNowTime)
			{
				if (_pWD->uNowTime != _pWD->uLastFrameNowTime)
				{	//dice roll once per sec
					u32 uDiveBombOdds = (u32) (100.0f*(1.0f - _pWD->pBot->ComputeUnitHealth()));
					u32 uDiveBombOddsFloor = 0;
					if (_pWD->fNowTime - pBrain->GetLastXYZChangeTime() > 0.0f)
					{
						uDiveBombOddsFloor += (u16) ((_pWD->fNowTime - pBrain->GetLastXYZChangeTime())*25.0f);  //each second of stationary increases floor by 50
					}
					uDiveBombOddsFloor += (u16) (((_pWD->pMover->GetLoc().y -(_pWD->EnemyMarkTagPos.y+ 5.0f))*0.1f)*15.0f); //every ten feet over head high is 15
					FMATH_CLAMP(uDiveBombOddsFloor, 0, 100);
					FMATH_CLAMPMIN(uDiveBombOdds, uDiveBombOddsFloor);

					if (fmath_RandomChoice(100) < uDiveBombOdds)
					{
						//findfix: should lead based on dive time
						_pWD->pMover->m_Controls.AimAt( _pWD->EnemyMark);
						_pWD->pMover->m_Controls.Fire2();
						_fGlobalDiveNotify = _pWD->fNowTime;

						//might be good to clear the path, or mark dive origin a good place to return to..
					}
				}
			}
		}
		break;
		case J_STRATEGY_MELEE:
		{
			pT->GetBrain()->PushBaseSpeedSetting(100);
			//set range params
			if (pT->m_TacticFSM.GetActiveStateHandle() == CGroundCombat::TACTIC_RANGEATTACK)
			{
				_GARangeSetParams(	pT,
									1,		   //u8 uDodgeTimeMin,   
									0,		   //u8 uDodgeTimeMax,   
									1,		   //u8 uDodgeOutTimeMin,
									0,		   //u8 uDodgeOutTimeMax,
									1,		   //u8 uDamageDodgeTimeMin
									_kafJumperMinRange[_uCurStrategy],	   //f32 fRangeMin,      
									_kafJumperMaxRange[_uCurStrategy],	   //f32 fRangeMax
									CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED,
									0.0f);
			}

			//Periodically causes a dive run to occur, if there is enough headroom
			_HandleDiveRun(uControl, pParam1, pParam2);

			if (_uDiveRunState == DIVERUN_STATE_NONE)
			{
				//
				// periodically hover if distance from player is fairly far
				//
				if (_pWD->pBot &&
					(_pWD->pBot->TypeBits() & ENTITY_BIT_BOTJUMPER))
				{
					if (_pWD->fDistToEnemyMarkXZ > 10.0f+5.0f+pBrain->GetInfreqCycleRandom() &&
						pBrain->GetInfreqCycleRandom() < 0.1f+0.9f*_pWD->fCourage)	   //Courage if 0.5 means odds are 0.55, which plays pretty nicely. Courage of 0 means odds of flying are 0.1f every time cyclerand changes.
					{
						CAIHoverBotMover* pJMover = (CAIHoverBotMover*) _pWD->pMover;
						if (pJMover->GetLastGraphVertId()) 
						{
							GraphVert* pV = aimain_pAIGraph->GetVert(pJMover->GetLastGraphVertId());
							if (pV->m_fHeightClearance > pJMover->GetHeight()*2.0f)
							{
								f32 fY = pBrain->GetInfreqCycleRandom()*pV->m_fHeightClearance-pJMover->GetHeight();
								if (fY < pJMover->GetHeight())
								{
									fY = pJMover->GetHeight();
								}
								pJMover->HoverTowardY(pV->m_Location.y+fY);
							}
						}
					}

					//dive bomb if airborn
					//health is a factor
					if (pT->m_fTimeWithLOS > 0.25f &&
						_pWD->pMover->GetLoc().y > _pWD->EnemyMarkTagPos.y &&
						_pWD->fDistToEnemyMarkXZ < 200.0f  &&
						_pWD->pBot->IsInAir() &&
						! ((CBotJumper*) _pWD->pBot)->IsDiving())
					{
						if ( (_fTimeSinceGlobalDiveNotify > 0.0f && _fTimeSinceGlobalDiveNotify < 0.3f) ||
							_pWD->uNowTime != _pWD->uLastFrameNowTime)
						{	//dice roll once per sec
							u32 uDiveBombOdds = (u32) (100.0f*(1.0f - _pWD->pBot->ComputeUnitHealth()));
							u32 uDiveBombOddsFloor = 0;
							if (_pWD->fNowTime - pBrain->GetLastXYZChangeTime() > 0.0f)
							{
								uDiveBombOddsFloor += (u16) ((_pWD->fNowTime - pBrain->GetLastXYZChangeTime())*25.0f);  //each second of stationary increases floor by 50
							}
							uDiveBombOddsFloor += (u16) (((_pWD->pMover->GetLoc().y -(_pWD->EnemyMarkTagPos.y))*0.1f)*15.0f); //every ten feet over head high is 15
							uDiveBombOddsFloor += (u16) (5.0f*pT->GetBrain()->GetLastXYZChangeTime());
							if (_pWD->pBot->m_Velocity_WS.y < 2.0f)
							{
								uDiveBombOddsFloor += 25;
							}
							if ( _fTimeSinceGlobalDiveNotify > 0.0f && _fTimeSinceGlobalDiveNotify < 0.3f)
							{
								uDiveBombOddsFloor +=50;
							}
							FMATH_CLAMP(uDiveBombOddsFloor, 0, 100);
							FMATH_CLAMPMIN(uDiveBombOdds, uDiveBombOddsFloor);

							if (fmath_RandomChoice(100) < uDiveBombOdds)
							{

								CFVec3A TargetLeadingBonusVec = CFVec3A::m_Null;
								f32 fProjSpeed = 180;   //estimated jumper dive speed
								if (fProjSpeed > 0.0f)
								{
									//cheap leading code
									f32 fTimeOfTravel = _pWD->fDistToEnemyMark/fProjSpeed;
									TargetLeadingBonusVec = pT->m_LastKnownVel;
									TargetLeadingBonusVec.Mul(fTimeOfTravel*_pWD->fIntelligence);		//no intelligence means no leading
									TargetLeadingBonusVec.Add(_pWD->EnemyMark);
								}

								//findfix: should lead based on dive time
								_pWD->pMover->m_Controls.AimAt( TargetLeadingBonusVec);
								_pWD->pMover->m_Controls.Fire2();
								_fGlobalDiveNotify = _pWD->fNowTime;

								//might be good to clear the path, or mark dive origin a good place to return to..
							}
						}
					}
				}
			}

			//
			// extra slashing periodically and when acquires target
			//
			if (pT->m_pEnemy && 
				!_pWD->pBot->IsInAir() &&   //only slash when on ground
				_pWD->pMover->CanAimPrimary() && //either of my two arms is attached
				pT->m_fTimeWithLOS > 0.01f &&
				((pT->m_fTimeWithLOS < 0.5f) ||
				(_pWD->uLastFrameNowTime != _pWD->uNowTime &&
				 pBrain->GetInfreqCycleRandom() < 0.2f+0.3f*_pWD->fAggression)))
			{
				_pWD->pMover->m_Controls.Fire2();   //fire 2 is slash if on ground
			}

		}
		break;
	}


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


	//
	// slash when opportunity presents itself
	//
	if (pT->m_pEnemy && 
		!_pWD->pBot->IsInAir() &&   //only slash when on ground
		_pWD->pMover->CanAimPrimary() && //either of my two arms is attached
		pT->m_fTimeWithoutLOS < 2.0f &&
		_pWD->fDistToEnemyMark < fEnemyInMechMeleeRangeBoost+(f32) pT->m_uMeleeRad)
	{
		_pWD->pMover->m_Controls.Fire2();   //fire 2 is slash if on ground
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_Jumper0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_JumperAIBeginFrame(uControl, pParam1, pParam2);

	pT->ChangeRuleSetState(CGroundCombat::ARS_JUMPER0_BASE);

	_JumperAIEndFrame(uControl, pParam1, pParam2);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_Jumper0(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_JumperAIBeginFrame(uControl, pParam1, pParam2);

	pT->m_RuleSetStateFSM.Work();

	pT->DoGenericTauntWork();

	//
	//  Special RULE that happens no matter what the sub mode of rs0 is
	//
	if (_pWD->pBot && _pWD->pBot->IsWalkingDead())
	{
		pT->ChangeRuleSet(CAIBrain::ATTACKRULESET_0);	   //will automatically kick into hide/panic mode whenthis happens
	}

	_JumperAIEndFrame(uControl, pParam1, pParam2);
	return AIFSM_WORK_CB_RETURN;
}


void CGroundCombat::DoWeaponAimAndFireWork_Jumper(void)
{
	CGroundCombat* pT = this;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	CAIWeaponCtrl* pWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
	if (!pWeaponCtrl)
	{
		return;
	}
	pWeaponCtrl->Work();	 //call this once a frame, before asking any questions of the weaponnctrl obj


	if ((m_uAttackFlags & FLAG_PANIC_ON) ||
		(s_pWD->pBot && (s_pWD->pBot->IsHopping() || s_pWD->pBot->IsRolling())) ||
		(s_pWD->pEnemyBot && !s_pWD->pEnemyBot->IsTargetable()) ||
		s_pWD->pMover->IsTalkingWithAnim() ||
		(!m_pEnemy && m_pBrain->GetReactionState() != CAIBrain::REACTIONSTATE_ATTACK_IMPULSE) ||	 //don't blind fire, unless on impulse reactio state
		!s_pWD->pMover->CanFirePrimary() ) {
		return;
	}

	BOOL bFireThisFrame = pWeaponCtrl->IsBursting();
	if (!bFireThisFrame)
	{
		if (_uCurStrategy != J_STRATEGY_MELEE)   //don't use weapon timers during melee, only fire in response to damage
		{
			if (pT->m_fTimeWithoutLOS < 3.0f*_pWD->fAggression)
			{
				bFireThisFrame = pWeaponCtrl->IsTimeToStartBurst();
			}
		}
	}
	
	if (!bFireThisFrame)
	{
		if (_pWD->pDamagedByMem && _pWD->pDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
		{
			bFireThisFrame = TRUE;
		}
	}

	if (bFireThisFrame &&
		s_pWD->pMover->HasTargetLock())
	{
		pWeaponCtrl->NotifyFire();
		s_pWD->pMover->m_Controls.Fire();
	}

	BOOL bLaserAim = TRUE;

	if (pT->m_uAttackFlags & CGroundCombat::FLAG_BOT_WAS_DOING_SPECIAL_MOVE ||
		(s_pWD->pMover->m_Controls.GetEntityControl()->m_nButtons & CBotControl::BUTTONFLAG_JUMP2))
	{
		bLaserAim = FALSE;
	}
	// AIM Logic
	if (bLaserAim &&
		m_fTimeWithoutLOS < 10.0f )	//aim if target has been within los in the last ten seconds
	{
		if (m_fUpdateBadShotBonusVecTime  < aiutils_FTotalLoopSecs())
		{
			m_BadShotBonusUnit.x = fmath_RandomBipolarUnitFloat();
			m_BadShotBonusUnit.z = fmath_RandomBipolarUnitFloat();
			m_BadShotBonusUnit.y = fmath_RandomFloat(); // missing low,sucks//fmath_RandomBipolarUnitFloat();
			m_fUpdateBadShotBonusVecTime = aiutils_FTotalLoopSecs() + 0.2f+ fmath_RandomFloat();
		}

		f32 fAccuracy = pWeaponCtrl->GetAccuracy();

		if (s_pWD->fDistToEnemyMark < (m_pBrain->GetAIMover()->GetRadiusXZ()+5.0f) &&
			m_uAttackFlags & FLAG_HASTARGETLOS)
		{
			fAccuracy = 1.0f;
		}
		else
		{
			//modifiers of units accuracy
			if ( s_pWD->pMover->GetMoveSpeed() > 1.0f)  // reduce aim accuracy by up to 0.25f if shooter is moving.
			{
				fAccuracy-= 0.25f*(1.0f - fAccuracy);
				FMATH_CLAMPMIN(fAccuracy, 0.0f);
			}

			if (m_pEnemy && 
				(m_pEnemy->TypeBits() & ENTITY_BIT_BOT) &&
				((CBot*) m_pEnemy)->m_fSpeed_WS > 1.0f)  // reduce aim accuracy by up to 0.25f if target is moving.
			{
				fAccuracy-= 0.25f*(1.0f - fAccuracy);
				FMATH_CLAMPMIN(fAccuracy, 0.0f);
			}
		}
    		
		CFVec3A TargetLeadingBonusVec = CFVec3A::m_Null;
		if (((CBot*) s_pWD->pMover->GetEntity())->m_apWeapon[0])
		{
			f32 fProjSpeed = ((CBot*) s_pWD->pMover->GetEntity())->m_apWeapon[0]->GetProjectileSpeed();
			if (fProjSpeed > 0.0f)
			{
				//cheap leading code
				f32 fTimeOfTravel = s_pWD->fDistToEnemyMark/fProjSpeed;
				TargetLeadingBonusVec = m_LastKnownVel;
				TargetLeadingBonusVec.Mul(fTimeOfTravel*fAccuracy);		//no accuracy means no leading
			}
		}

		CFVec3A BadShotBonusVec;
		BadShotBonusVec = m_BadShotBonusUnit;
		f32 fBadShotBonusVecScaleXZ = 5.0f*(1.0f-fAccuracy);		 //potentially miss by six feet
		BadShotBonusVec.Mul(fBadShotBonusVecScaleXZ);	
		BadShotBonusVec.Add(s_pWD->EnemyMarkTagPos);// *(m_pEnemy->GetTagPoint(uTagToShootAt))); cheating!
		BadShotBonusVec.Add(TargetLeadingBonusVec);

		if (m_LastAim == CFVec3A::m_Null)
		{
			m_LastAim = BadShotBonusVec;
		}
		else
		{
			m_LastAim.Add(BadShotBonusVec);	  //m_LastAim approaches BadShotBunusVec.. note: fps dependent
			m_LastAim.Mul(0.5f);
		}

		FMATH_DEBUG_FCHECK(m_LastAim.x);
		FMATH_DEBUG_FCHECK(m_LastAim.y);
		FMATH_DEBUG_FCHECK(m_LastAim.z);

		s_pWD->pMover->m_Controls.AimAt(m_LastAim);
	}
}