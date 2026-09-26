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
#include "../BotProbe.h"
#include "../BotPred.h"
#include "AIPath.h"
#include "../BotSnarq.h"

static const f32 kfOOHun = 1.0f/100.0f;
static const f32 _kfPredRocketSafetyRange = 20.0f;

static BOOL _StuffLowPrecisionVecInMedMem(	CAIBrain* pBrain, const CFVec3A &tmp, u8 uMemoryId, u8 uHowLong)				
{
	CAIMemoryMedium* pMem = (CAIMemoryMedium*) pBrain->GetKnowledge().RememberThis(uMemoryId, uHowLong);
	if (pMem)
	{
		u8 *pBuff = (u8*) &(pMem->m_uSmallData);
		pBuff = pMem->PokeS16(pBuff, (s16)tmp.x);	
		pBuff = pMem->PokeS16(pBuff, (s16)tmp.y);				
		pBuff = pMem->PokeS16(pBuff, (s16)tmp.z);				
		FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryMedium));
		return TRUE;
	}
	return FALSE;
}


static BOOL _ExtractLowPrecisionVecInMedMem(CAIBrain* pBrain, 
											u8 uMemoryId,
											CFVec3A *pTmp)				
{

	CAIMemoryLarge* pMem;
	if (pBrain->GetKnowledge().CanRememberAny(uMemoryId, (CAIMemorySmall**) &pMem))
	{
		u8 *pBuff = (u8*) &(pMem->m_uSmallData);
		s16 tmp;
		pBuff = pMem->PeekS16(pBuff, &tmp);
		pTmp->x = tmp;
		pBuff = pMem->PeekS16(pBuff, &tmp);
		pTmp->y = tmp;
		pBuff = pMem->PeekS16(pBuff, &tmp);
		pTmp->z = tmp;
		FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryMedium));
		return TRUE;
	}
	return FALSE;
}


void CGroundCombat::DoWeaponAimAndFireWork_Snarq(void)
{
	BOOL bDoNormalAim = TRUE;

	m_uAttackFlags &= ~FLAG_TORSO_NO_WORK;
	if (m_pEnemy && s_pWD->pBot->m_fSpeedXZ_WS > 5.0f &&
		IsFollowingPath(TRUE, SEARCHREASON_STRAFE))
	{
		if (s_pWD->EnemyMark.y+s_pWD->pEnemyBot->m_fCollCylinderHeight_WS < s_pWD->pMover->GetLoc().y - 10.0f)
		{
			//enemy is well below us.
			bDoNormalAim = FALSE;
			CFVec3A AimDir = s_pWD->pBot->m_UnitVelocityXZ_WS;
			CFVec3A wing;
			wing.Cross(CFVec3A::m_UnitAxisY, AimDir);

			CFQuatA Quat;

			Quat.BuildQuat(wing, FMATH_DEG2RAD(60));//30.0f-GetBrain()->GetInfreqCycleRandom()*10.0f));//s_pWD->pBot->m_pBotInfo_MountAim->fAllowableLockAngleCos
			Quat.BuildMtx( CFMtx43A::m_Temp );
			CFMtx43A::m_Temp.MulDir(AimDir);
			m_LastAim = AimDir;


			m_LastAim.Mul(s_pWD->fDistToEnemyMark+ (s_pWD->fDistToEnemyMark*0.25f - 2.0f*(s_pWD->fDistToEnemyMark*0.25f*GetBrain()->GetInfreqCycleRandom())));
  			m_LastAim.Add(s_pWD->pMover->GetLoc());


			CAIWeaponCtrl* pWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
			if (pWeaponCtrl)
			{
				pWeaponCtrl->NotifyFire();
				s_pWD->pMover->m_Controls.Fire();
			}

			s_pWD->pMover->m_Controls.AimAt(m_LastAim);
			m_uAttackFlags |= FLAG_TORSO_NO_WORK;
			s_pWD->pMover->FaceToward(m_LastAim);
		}
	}
	DoWeaponAimAndFireWork_Generic(-1.0f, bDoNormalAim, FALSE);
}


void CGroundCombat::DoWeaponAimAndFireWork_Predator(void)
{
	if (s_pWD->pMover->IsTalkingWithAnim()) {
		return;
	}

	CAIWeaponCtrl* pPrimaryWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
	CAIWeaponCtrl* pSecondaryWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(1);
	if (!(pPrimaryWeaponCtrl && pSecondaryWeaponCtrl) )
	{
		return;
	}

	if (!(s_pWD->pMover->CanFirePrimary() || s_pWD->pMover->CanFireSecondary()))
	{
		return;  //this predator has neither primary or secondary
	}

	pPrimaryWeaponCtrl->Work();	 //call this once a frame, before asking any questions of the weaponnctrl obj
	pSecondaryWeaponCtrl->Work();

	f32 _fMaxBlindReturnPrimaryFireRange = 10.0f;

	BOOL bFirePrimary = FALSE;
	BOOL bFireSecondary = FALSE;
	u32 bAimRocket = FALSE;
	if (pPrimaryWeaponCtrl->IsBursting())
	{
		bFirePrimary = TRUE;
	}
	if (pSecondaryWeaponCtrl->IsBursting())
	{
		bFireSecondary = TRUE;
	}
	else if (s_pWD->pMover->HasTargetLock() &&
			((this->m_fTimeWithLOS > 0.15f+m_pBrain->GetInfreqCycleRandom()*0.5f) ||
			(s_pWD->pDamagedByMem && (s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs < 1) && s_pWD->fDistToEnemyMark < _fMaxBlindReturnPrimaryFireRange)))
	{
		bFirePrimary = TRUE;
	}
	else if (m_uFireOddsTimeOut < s_pWD->uNowTime)
	{
		f32 fTimeSinceLOS = m_fTimeWithoutLOS;
		m_uFireOddsTimeOut = s_pWD->uNowTime + 1;
		if (fTimeSinceLOS > 2.0f && 
			(m_uAttackFlags & FLAG_NOPATH)) // HasTargetLock means that the weapon will fire angle is within some internal tolerance to be considered a "lock" circa 20degrees
		{
			u32 uFireSecondaryOdds = 0;
			if (s_pWD->pDamagedByMem && (s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs < 1) && s_pWD->fDistToEnemyMark < _fMaxBlindReturnPrimaryFireRange)
			{
				uFireSecondaryOdds = 97;
			}
			else
			{	//no los
				uFireSecondaryOdds = 5;
				if (fTimeSinceLOS < 5.0f)
				{
					uFireSecondaryOdds += (u16) ((f32)80*fTimeSinceLOS/5.0f);
				}
			}
			if (fmath_RandomChoice(100) < uFireSecondaryOdds)
			{
				bFireSecondary = TRUE;
			}
		}
		else if (!m_pEnemy)
		{
			if (m_uAttackFlags & FLAG_CANTFINDENEMY &&
				m_pBrain->GetWorldAddRandom() < 0.77f)
			{
				if (s_pWD->uNowTime - (u16)m_fAttackThoughtInitTime < 2)
				{
					bFirePrimary = TRUE;
				}
			}

			if (!bFirePrimary &&
				s_pWD->fAggression > 0.75f &&
				s_pWD->pDamagedByMem &&
				s_pWD->uNowTime < m_uBlindResponseToDamageTimeOut && 
				(s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs< 2) )
			{
				bFirePrimary = TRUE;
			}
		}
		else if (m_pEnemy &&
				!(m_pEnemy->TypeBits() & ENTITY_BIT_BOT))
		{
			bFirePrimary = TRUE;
		}
	}

	if (s_pWD->pBot && (s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTPRED))
	{
		u32 uNumDangling, uNumIntact;
		if (((CBotPred*) s_pWD->pBot)->GetLimbsStatus(&uNumIntact, &uNumDangling))
		{
			u32 uNumGone = 4 - (uNumDangling + uNumIntact);
			if (uNumDangling + uNumGone > 2)
			{
				bFireSecondary = bFirePrimary;
				bFirePrimary = FALSE;
			}
		}
	}

	if (!bFireSecondary &&
		(bFirePrimary || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
		s_pWD->pMover->CanFirePrimary()	 &&
		pPrimaryWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY))
	{  
		pPrimaryWeaponCtrl->NotifyFire();
		s_pWD->pMover->m_Controls.Fire();
	}
	else if (	(bFireSecondary || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
				pSecondaryWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY) &&
				s_pWD->pMover->CanFireSecondary() && 
				m_pEnemy )
	{  
		pSecondaryWeaponCtrl->NotifyFire();

		CFVec3A RocketSafetyEndPt;
		RocketSafetyEndPt.Sub(this->m_LastAim, s_pWD->pMover->GetEyePos());
		if (RocketSafetyEndPt.SafeUnitAndMag(RocketSafetyEndPt) > 0.0f)
		{
			RocketSafetyEndPt.Mul(_kfPredRocketSafetyRange);
			RocketSafetyEndPt.Add(s_pWD->pMover->GetEyePos());
			if (!aiutils_IsLOSObstructed_IgnoreBots(s_pWD->pMover->GetEyePos(),  RocketSafetyEndPt, s_pWD->pBot))
			{
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
	if (this->m_fTimeWithoutLOS < 0.5f || pSecondaryWeaponCtrl->IsBursting())
	{	//predator stops aiming if no los
//		DoLagWeaponAiming(0);
//		DoLeadAimWithError(7.0f, pPrimaryWeaponCtrl->GetAccuracy(), s_pWD->EnemyMark);



		CAIWeaponCtrl* pChainWeaponCtrl = pPrimaryWeaponCtrl;
		if (!pChainWeaponCtrl)
		{
			return;
		}

		f32 fAccuracy = 1.0f;
		fAccuracy *= pChainWeaponCtrl->GetAccuracy();
	//	fAccuracy = 1.0f;
		if (m_uAttackFlags & FLAG_HASTARGETLOS &&
			s_pWD->fDistToEnemyMark < s_pWD->pMover->GetRadiusXZ()+5.0f)
		{
			fAccuracy = 1.0f;
		}
		else
		{
			if ( ((CBot*) s_pWD->pMover->GetEntity())->m_fSpeed_WS > 1.0f)  // reduce aim accuracy by up to 0.5f if shooter is moving.
			{
				fAccuracy-= 0.25f*(1.0f - fAccuracy);
				FMATH_CLAMPMIN(fAccuracy, 0.0f);
			}

			if (m_pEnemy && ((CBot*) m_pEnemy)->m_fSpeed_WS > 1.0f)  // reduce aim accuracy by up to 0.5f if target is moving.
			{
				fAccuracy-= 0.25f*(1.0f - fAccuracy);
				FMATH_CLAMPMIN(fAccuracy, 0.0f);
			}

		}

		CFVec3A NewAimPt;
		if (m_LastAim == CFVec3A::m_Null || this->m_fTimeWithLOS < 0.01f || !pPrimaryWeaponCtrl->IsBursting())
		{
//			m_LastAim = s_pWD->pBot->MtxToWorld()->m_vFront;
//			m_LastAim.Mul(s_pWD->pMover->GetRadiusXZ());
//			m_LastAim.Add(s_pWD->pMover->GetLoc());
			m_LastAim = s_pWD->EnemyMark;
		}
		else
		{
			f32 fLerp = 1.0f - fmath_Div(pChainWeaponCtrl->m_fFireTimeOut - s_pWD->fNowTime, pChainWeaponCtrl->m_fFireTimeOut - pChainWeaponCtrl->m_fBurstStartTime);
			f32 fTotalDist = s_pWD->fDistToEnemyMark - (2.0f*s_pWD->pMover->GetRadiusXZ());
			FMATH_CLAMP_MIN0( fTotalDist );

			f32 fSpreadFactor = fmath_Div( this->m_fTimeWithLOS, 10.0f );
			FMATH_CLAMP_UNIT_FLOAT( fSpreadFactor );

			f32 fSpread = fTotalDist * (1.0f - fSpreadFactor);

			m_LastAim = s_pWD->DeltaToEnemyMarkUnit;
			m_LastAim.Mul(fSpread*2.0f*fLerp);
			

			CFVec3A MinVec = s_pWD->DeltaToEnemyMarkUnit;
			MinVec.Mul( -fSpread );

			//MinVec.Mul(2.0f*s_pWD->pMover->GetRadiusXZ());

			m_LastAim.Add(MinVec);	  //m_LastAim approaches BadShotBunusVec.. findfix: fps dependent
			//m_LastAim.Add(s_pWD->pMover->GetLoc());
			m_LastAim.Add(s_pWD->EnemyMarkTagPos);
		}

		FMATH_DEBUG_FCHECK(m_LastAim.x);
		FMATH_DEBUG_FCHECK(m_LastAim.y);
		FMATH_DEBUG_FCHECK(m_LastAim.z);

		s_pWD->pMover->m_Controls.AimAt(m_LastAim);
	}
}

//shield				   off,  on
f32 _kfPredMinRange[2] = {20.0f, 20.0f};
f32 _kfPredMaxRange[2] = {200.0f, 50.0f};
f32 _kfPredEraptSmartDelayTimeBase = 3.5f;
BOOL InRules_ARS_Pred0_Base(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

	pT->ChangeTactic(CGroundCombat::TACTIC_RANGEATTACK);
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GARangeSetParams(	pT,
						3,		   //u8 uDodgeTimeMin,   
						6,		   //u8 uDodgeTimeMax,   
						2,		   //u8 uDodgeOutTimeMin,
						4,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						pBrain->GetAIMover()->GetRadiusXZ()+_kfPredMinRange[bShieldOn],	   //f32 fRangeMin,      
						pBrain->GetAIMover()->GetRadiusXZ()+_kfPredMaxRange[bShieldOn],	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_SMART | CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED,
						0.5f+_pWD->fAggression*_kfPredEraptSmartDelayTimeBase);

	//try to make botsnarq move a lot if he is in sight, or getting damaged
	if (_pWD->pBot->TypeBits() & ENTITY_BIT_BOTSNARQ)
	{
		if (pT->m_fTimeWithLOS > 0.1f || pT->m_fDamageTakenPerSec > 0.1f)
		{
			pT->m_AttackPtSearch.m_uRepeatTimeMin = 0;
			pT->m_AttackPtSearch.m_uRepeatTimeMax = 0;
		}
		else
		{
			pT->m_AttackPtSearch.m_uRepeatTimeMin = 1;
			pT->m_AttackPtSearch.m_uRepeatTimeMax = 5;
		}
	}
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Pred0_Base(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//give range attack time
	u16 uPredInvestigateMarkAfterTime = (u16)pT->m_fEraptLosIsLostWaitTime + 3 + (u16) 2*(pBrain->GetWorldAddRandom() < 0.5f) - (u16) ((f32)2*pBrain->GetAttrib(CAIBrain::ATTRIB_AGGRESSION)*kfOOHun);
	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

	//when pred0 is in base state, tactic should always be RANGE

	f32 fDeltaY = _pWD->EnemyMark.y - _pWD->pMover->GetLoc().y;
	f32 fAbsDeltaY = FMATH_FABS(fDeltaY);

	f32 fFunnerMinXZRange = fmath_Div(fAbsDeltaY, 0.57f);	//tan 30   //_pWD->fDistToEnemyMarkXZ;
	//adjust range times depending on whether the pred sheild is on.
	_GARangeSetParams(	pT,
						3,		   //u8 uDodgeTimeMin,   
						6,		   //u8 uDodgeTimeMax,   
						2,		   //u8 uDodgeOutTimeMin,
						4,		   //u8 uDodgeOutTimeMax,
						1,			//u8 uDamageDodgeTimeMin
						fFunnerMinXZRange,//pBrain->GetAIMover()->GetRadiusXZ()+_kfPredMinRange[bShieldOn],	   //f32 fRangeMin,      
						pBrain->GetAIMover()->GetRadiusXZ()+_kfPredMaxRange[bShieldOn],	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_SMART | CGroundCombat::ERAPTUSEAGECTRL_TIMER_BASED,
						0.5f+_pWD->fAggression*_kfPredEraptSmartDelayTimeBase);
	//always keep the y range setup based on 
	f32 fDeltaYFromEnemy = _pWD->pMover->GetLoc().y - _pWD->EnemyMarkTagPos.y;
	if (fDeltaYFromEnemy > 0.0f)
	{
		if (pBrain->GetInfreqCycleRandom() < 0.5f)
		{
			pT->m_AttackPtSearch.m_nRangeYInc = -10;
		}
		else
		{
			pT->m_AttackPtSearch.m_nRangeYInc = 10;
		}

		pT->m_AttackPtSearch.m_uRangeYMin = 10;
		if (fDeltaYFromEnemy < 255.0f)
		{
			pT->m_AttackPtSearch.m_uRangeYMax = (u8) fDeltaYFromEnemy;
		}
		else
		{
			pT->m_AttackPtSearch.m_uRangeYMax = 255;
		}

	}
	else
	{
		pT->m_AttackPtSearch.m_uRangeYMin = 10;
		pT->m_AttackPtSearch.m_uRangeYMax = 20;
		pT->m_AttackPtSearch.m_nRangeYInc = +10;
	}

	//if
	//	 Haven't seen the enemy for some time (aggression is a factor) &&
	//   currently am more than _kfInvestigateMarkMinDist feet from enemymark  (prevents rule from firing repeatedly)
	//then
	//   Investigate the mark
	//

	//
/*
	if (pT->m_fTimeWithoutLOS > (f32) uPredInvestigateMarkAfterTime &&
		!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS) &&  //
		(pT->m_uRangeTacticSmartEraptDecisionTimeOut > 0 && (pT->m_AttackPtSearch.m_uFailureCount > 5 || (pT->m_uRangeTacticSmartEraptDecisionTimeOut < _pWD->uNowTime))) &&
		_pWD->fDistToEnemyMark >= _kfInvestigateMarkMinDist+_pWD->pMover->GetRadiusXZ())
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_PRED0_INVESTIGATEMARK);
	}
*/


	if ((_pWD->pBot->TypeBits() & ENTITY_BIT_BOTSNARQ) &&
		pT->m_fTimeWithoutLOS < 1.0f &&
		_pWD->uLastFrameNowTime != _pWD->uNowTime)
	{
		CBotSnarq* pBotSnarq = ((CBotSnarq*) _pWD->pBot);

		CFVec3A LastFailLoc;
		if (!_ExtractLowPrecisionVecInMedMem(pBrain, MEMORY_FAILED_STRAFE, &LastFailLoc) ||
			LastFailLoc.DistSq(_pWD->pMover->GetLoc()) > 3.0f)
		{
			pT->ChangeRuleSetState(CGroundCombat::ARS_PRED0_STRAFE);
		}

		if (pBotSnarq->m_fSpeedXZ_WS > 8.0f)
		{
			
			f32 fVelDotFront = pBotSnarq->m_VelocityXZ_WS.Dot(_pWD->pMover->GetTorsoLookAtXZ());
			if (FMATH_FABS(fVelDotFront) < 0.7f)
			{
				CFVec3A RightVel;
				f32 fTmp;
				fTmp = RightVel.x;
				RightVel.x = pBotSnarq->m_VelocityXZ_WS.z;
				RightVel.y = 0.0f;
				RightVel.z = -fTmp;
				BOOL bRight = (RightVel.Dot(_pWD->pMover->GetTorsoLookAtXZ()) > 0.0f);
				pBotSnarq->StartRoll(1.0f, bRight);
			}
			else if (fVelDotFront > 0.9f && pBotSnarq->m_fSpeed_WS > 40.0f)
			{
				pBotSnarq->StartRoll(1.0f, (FVid_nFrameCounter & 1));
			}
		}

	}


//	if (!_pWD->pMover->CanAimPrimary())
//	{
//		pT->m_uAttackFlags |= CGroundCombat::FLAG_PANIC_ON | CGroundCombat::FLAG_TORSO_NO_WORK;
//		pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
//
//}

	return AIFSM_WORK_CB_RETURN;
}

enum
{
	STRAFERUN_NONE,
	STRAFERUN_INITPATH,
	STRAFERUN_WAITFORPATH,
	STRAFERUN_ENROUTE
};

u8 _uStrafeRunState = STRAFERUN_NONE;
u8 _uStrafeRunPathNum = 0;
CFVec3A _SrafeRunDirXZ;
CFVec3A _SrafeRunFlyByMark;
u8 _uStrafeInitDistXZ; 
static BOOL _StrafeEndFrame(CAIBrain* pBrain)
{
	CAIMemoryLarge* pMem;
	if (pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_STRAFERUNDATA, (CAIMemorySmall**) &pMem) ||
		(pMem = (CAIMemoryLarge*) pBrain->GetKnowledge().RememberThis(MEMORY_LARGE_STRAFERUNDATA, 2)))
	{
		FASSERT(pMem);
		pMem->ResetTimer(2);
		u8 *pBuff = (u8*) &(pMem->m_uSmallData);
		pBuff = pMem->PokeU8U8(pBuff, _uStrafeRunState, _uStrafeRunPathNum);
		pBuff = pMem->PokeU8U8(pBuff, _uStrafeInitDistXZ, 0);
		pBuff = pMem->PokeF32(pBuff, _SrafeRunDirXZ.x);					
		pBuff = pMem->PokeF32(pBuff, _SrafeRunDirXZ.y);						
		pBuff = pMem->PokeF32(pBuff, _SrafeRunDirXZ.z);						
		pBuff = pMem->PokeF32(pBuff, _SrafeRunFlyByMark.x);					
		pBuff = pMem->PokeF32(pBuff, _SrafeRunFlyByMark.y);						
		pBuff = pMem->PokeF32(pBuff, _SrafeRunFlyByMark.z);						
		FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryLarge));
		return TRUE;
	}
	return FALSE;
}


static BOOL _StrafeBeginFrame(CAIBrain* pBrain )
{
	CAIMemoryLarge* pMem;
	if (pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_STRAFERUNDATA, (CAIMemorySmall**) &pMem))
	{
		FASSERT(pMem);
		pMem->ResetTimer(2);
		u8 *pBuff = (u8*) &(pMem->m_uSmallData);
		pBuff = pMem->PeekU8U8(pBuff, &_uStrafeRunState, &_uStrafeRunPathNum);
		pBuff = pMem->PeekU8U8(pBuff, &_uStrafeInitDistXZ, NULL);
		pBuff = pMem->PeekF32(pBuff, &(_SrafeRunDirXZ.x));					
		pBuff = pMem->PeekF32(pBuff, &(_SrafeRunDirXZ.y));						
		pBuff = pMem->PeekF32(pBuff, &(_SrafeRunDirXZ.z));						
		pBuff = pMem->PeekF32(pBuff, &(_SrafeRunFlyByMark.x));					
		pBuff = pMem->PeekF32(pBuff, &(_SrafeRunFlyByMark.y));						
		pBuff = pMem->PeekF32(pBuff, &(_SrafeRunFlyByMark.z));						
		FASSERT(pBuff < (u8*) pMem + sizeof(CAIMemoryLarge));
		return TRUE;
	}
	return FALSE;
}




BOOL InRules_ARS_Pred0_Strafe(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	_uStrafeRunPathNum = 0;
	_uStrafeRunState = STRAFERUN_INITPATH;

	pT->GetBrain()->PushBaseSpeedSetting(100);
	//nothing, leave tactic that was running running, until we have a strafe path

	_StrafeEndFrame(pBrain);

	return AIFSM_INIT_CB_DONE;
}

BOOL DoRules_ARS_Pred0_Strafe(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	CFVec3A Tmp;
	BOOL bQuitStrafe = FALSE;
	u8 uStrafeSlot = 0xff;

	_StrafeBeginFrame(pBrain);

	pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_STRAFE, &uStrafeSlot);

	//
	// periodically position self for a dive bomb
	//
	switch(_uStrafeRunState)
	{
		case STRAFERUN_INITPATH:

			if (_uStrafeRunPathNum == 0)
			{
				_SrafeRunFlyByMark = _pWD->EnemyMark;
				_SrafeRunDirXZ = _pWD->DeltaToEnemyMarkUnit;
				_SrafeRunDirXZ.y = 0.0f;
				if (_SrafeRunDirXZ.SafeUnitAndMag(_SrafeRunDirXZ) <= 0.0f)
				{
					_SrafeRunDirXZ = _pWD->pMover->GetTorsoLookAtXZ();
				}
				_uStrafeInitDistXZ = (u8) _pWD->fDistToEnemyMarkXZ;
			}

			if (uStrafeSlot != 0xff)
			{
				pT->FreeRequestSlot(uStrafeSlot);
				uStrafeSlot = 0xff;
			}

			Tmp = _SrafeRunDirXZ;
			Tmp.Mul(_pWD->pMover->GetRadiusXZ()+25.0f);
			Tmp.Add(_pWD->pMover->GetLoc());
			Tmp.y = _pWD->EnemyMarkTagPos.y + 0.5f*fmath_Abs(_pWD->EnemyMark.y - _pWD->pMover->GetLoc().y);

			pT->RequestSearch(Tmp, CGroundCombat::SEARCHREASON_STRAFE, _pWD->pMover->GetRadius()*2.0f+ fmath_Abs(_pWD->EnemyMark.y - _pWD->pMover->GetLoc().y));
			_uStrafeRunState = STRAFERUN_WAITFORPATH;

/*
			switch (_uStrafeRunPathNum)
			{
			case 0:
				Tmp = _pWD->DeltaToEnemyMarkUnit;
				Tmp.Mul(40.0f);
				Tmp.Add(_pWD->EnemyMarkTagPos);
				Tmp.y = _pWD->EnemyMarkTagPos.y + 0.5f*fmath_Abs(_pWD->EnemyMark.y - _pWD->pMover->GetLoc().y);

				pT->RequestSearch(Tmp, CGroundCombat::SEARCHREASON_STRAFE, _pWD->pMover->GetRadius()*2.0f+ fmath_Abs(_pWD->EnemyMark.y - _pWD->pMover->GetLoc().y));
				_uStrafeRunState = STRAFERUN_WAITFORPATH;

				break;
			case 1:
				Tmp = _pWD->pMover->GetTorsoLookAtXZ();
				Tmp.Mul(40.0f);
				Tmp.Add(_pWD->pMover->GetLoc());

				pT->RequestSearch(Tmp, CGroundCombat::SEARCHREASON_STRAFE, 10.0f);
				_uStrafeRunState = STRAFERUN_WAITFORPATH;

				break;
			default:
			bQuitStrafe = TRUE;
			}
  */
			break;
		case STRAFERUN_WAITFORPATH:
			
			//if
			//		an SEARCHREASON_STRAFE pt path fails || 
			//      bot is stuck
			//then
			//		clear the search slot
			//
			if (uStrafeSlot == 0xff ||
				pT->m_aSearchQuery[uStrafeSlot].HasFailed() || 
				pT->m_aSearchQuery[uStrafeSlot].WasCanceled())
			{
				bQuitStrafe = TRUE;
			}
			//if
			//		a SEARCHREASON_STRAFE path is ready && 
			//      not already following it.
			//then
			//		follow it
			else if (pT->IsSearchDone(CGroundCombat::SEARCHREASON_STRAFE, &uStrafeSlot))
			{
//				pT->m_aSearchQuery[uStrafeSlot].GetResults()->GetAIPath()->DontStopAtEnd();

				if (pT->m_TacticFSM.GetActiveStateHandle() != CGroundCombat::TACTIC_NONE)
				{
					pT->ChangeTactic(CGroundCombat::TACTIC_NONE);
				}
				pT->Attack_AssignPath(uStrafeSlot);
				_uStrafeRunState = STRAFERUN_ENROUTE;
			}
			break;
		case STRAFERUN_ENROUTE:

			if (uStrafeSlot != 0xff)
			{
				f32 fCloseEnoughtDist = (_pWD->pMover->GetRadius()+5.0f);
				if ((pT->m_bPathJustCompleted &&
					pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_STRAFE) ||
					(pT->IsFollowingPath() &&
					 pT->m_aSearchQuery[uStrafeSlot].GetGoalLoc().DistSq(_pWD->pMover->GetLoc()) < fCloseEnoughtDist*fCloseEnoughtDist)
					)
				{
					_uStrafeRunState = STRAFERUN_INITPATH;
					_uStrafeRunPathNum++; 				
				}
				else if (!pT->IsFollowingPath())
				{
					pT->Attack_AssignPath(uStrafeSlot);
				}
			}
			else if (_pWD->pMover->IsStuck(2.0f))
			{
				bQuitStrafe = TRUE;
			}
			break;
	}

/*	f32 fFlyPastByDist = 40.0f;
	CFVec3A DeltaToSTrafeMarkUnitXZ;
	DeltaToSTrafeMarkUnitXZ.Sub(_SrafeRunFlyByMark, _pWD->GetLoc());

	if (!bQuitStrafe &&
		(_pWD->fDistToEnemyMarkXZ > fFlyPastByDist && _SrafeRunDirXZ.Dot(DeltaToEnemyMarkUnitXZ) < -1.0f) &&
		!pT->IsFollowingPath() || 
		(uStrafeSlot != 0xff && pT->m_aSearchQuery[uStrafeSlot].GetGoalLoc().DistSqXZ(_pWD->EnemyMark) > 40))
	{
		bQuitStrafe = TRUE;
	}

  */

	if (bQuitStrafe)
	{
		if (uStrafeSlot != 0xff)
		{
			pT->FreeRequestSlot(uStrafeSlot);
		}
		_uStrafeRunState = STRAFERUN_NONE;
		_uStrafeRunPathNum = 0;
		pT->ChangeRuleSetState(CGroundCombat::ARS_PRED0_BASE);
		pBrain->GetKnowledge().ForgetAllId(MEMORY_FAILED_STRAFE);

		_StuffLowPrecisionVecInMedMem(pBrain, _pWD->pMover->GetLoc(), MEMORY_FAILED_STRAFE, 2);
	}

	_StrafeEndFrame(pBrain);

	return AIFSM_WORK_CB_RETURN;
}


BOOL OutRules_ARS_Pred0_Strafe(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	u8 uStrafeSlot;
	
	if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_STRAFE, &uStrafeSlot))
	{
		pT->FreeRequestSlot(uStrafeSlot);
	}
	_uStrafeRunState = STRAFERUN_NONE;
	_uStrafeRunPathNum = 0;	

	pT->GetBrain()->PopBaseSpeedSetting();
	return AIFSM_UNINIT_CB_DONE;
}


BOOL InRules_ARS_Pred0_InvestigateMark(u32 uControlParam, void *pParam1, void *pvParam2)
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
						2.0f,	   //f32 fRangeMin,      
						4.0f,	   //f32 fRangeMax
						CGroundCombat::ERAPTUSEAGECTRL_NEVER,
						0.0f);	   

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Pred0_InvestigateMark(u32 uControlParam, void* pParam1, void* pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

	//if
	//   seen the enemy since pT tactic began.
	//then
	//   go to base state
	if ((f32)(_pWD->uNowTime - pT->m_uTacticInitTime) < pT->m_fTimeWithoutLOS)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_PRED0_BASE);
	}
	//if
	//	 close to the mark but don't see enemy or
	//	 been stuck for some time
	//then
	//   go to search state
	else if ((_pWD->fDistToEnemyMark < _kfInvestigateMarkMinDist) ||
			 !pBrain->HasMovedOneFootSince(_kuInvestigateMarkFailureTime))
	{
/* search disable for pred right now findfix
		//kick into search mode, then eventually back to our job if no enemy can be found
		pT->ChangeRuleSetState(CGroundCombat::ARS_PRED0_SEARCH);
*/
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_Pred0_Search(u32 uControl, void* pParam1, void* pParam2)
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


BOOL DoRules_ARS_Pred0_Search(u32 uControl, void* pParam1, void* pParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

	//
	//If 
	//		sees the enemy
	//then
	//		go back to base state
	//
	if (pT->m_fTimeWithoutLOS < 0.7f)
	{
		pT->ChangeRuleSetState(CGroundCombat::ARS_PRED0_BASE);
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InitRuleSet_Pred0(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeRuleSetState(CGroundCombat::ARS_PRED0_BASE);

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_Pred0(u32 uControlParam, void *pParam1, void *pvParam2)
{
	CGroundCombat* pT = (CGroundCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	BOOL bShieldOn = aiutils_IsShieldActive(_pWD->pMover->GetEntity());

#ifdef _DEBUG
	FSMStateHandle uState = pT->m_RuleSetStateFSM.GetActiveStateHandle();
	cchar* pszState = pT->m_RuleSetStateFSM.GetActiveStateName();
	switch (uState)
	{
		case FSMSTATEHANDLE_INVALID:
			break;
		case CGroundCombat::ARS_PRED0_BASE:
			break;
		case CGroundCombat::ARS_PRED0_INVESTIGATEMARK:
			break;
		case CGroundCombat::ARS_PRED0_SEARCH:
			break;
		case CGroundCombat::ARS_PRED0_STRAFE:
			break;
		default:
			FASSERT(pT->m_RuleSetStateFSM.IsStateChangePending());
			break;
	}
#endif

	//
	//  work sub states
	//
	pT->m_RuleSetStateFSM.Work();

	pT->DoGenericTauntWork();

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

