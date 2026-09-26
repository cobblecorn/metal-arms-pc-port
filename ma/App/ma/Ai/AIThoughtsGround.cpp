#include "fang.h"
#include "flinklist.h"
#include "fScriptSystem.h"
#include "AIThoughtsGround.h"
#include "AIThoughtsGeneric.h"
#include "AICorrosive.h"
#include "AIZombieBoss.h"
#include "AIMain.h"
#include "AIBrainman.h"
#include "AiBrainMems.h"
#include "AIBrain.h"
#include "AIMover.h"
#include "AIBotMover.h"
#include "AIGraph.h"
#include "AINodePools.h"
#include "AIRooms.h"
#include "ApeNew.h"
#include "AiApi.h"
#include "AiBrainUtils.h"
#include "AIGameUtils.h"
#include "AIPatrolPath.h"
#include "AIEnviro.h"
#include "AIBTAtable.h"
#include "AIGroup.h"
#include "AIFSM.h"
#include "AIEdgeLock.h"
#include "AITactics.h"
#include "AIRulesGround.h"

#include "../alarmsys.h"
#include "../Bot.h"
#include "../BotEliteGuard.h"
#include "../BotProbe.h"
#include "../botscout.h"
#include "../BotTalkInst.h"
#include "../BotTalkData.h"
#include "../BotJumper.h"
#include "../BotGrunt.h"
#include "../Bottitan.h"
#include "../Botmozer.h"
#include "../Botscientist.h"
#include "../botzom.h"
#include "../Entity.h"
#include "../espline.h"
#include "../GString.h"
#include "../meshentity.h"
#include "../Player.h"
#include "../Site_BotWeapon.h"
#include "../Vehicle.h"
#include "../Weapon.h"
#include "../VehicleLoader.h"

extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );


u8 kuAttackMOdeTalkRequestTimeOutSecs = 1;
f32 kfSentinelRocketSafetyRange = 15.0f;
s32 maimain_nAIJobEvent = 0;
static const u8 _kuMinDodgeDamageTime = 1;
static const f32 kfOOHun = 1.0f/100.0f;





//
//  Ground Search
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
///////////////////////
void CGroundSearch::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /*= NULL*/)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);

	m_pLookAtMgr = NULL;
	m_uSearchFlags = 0;
	
	m_pWander = (CGenericWander*) CGenericWander::BankAccess(NULL);
	if (m_pWander)
	{
		//no param pack, since we'll configure it as we see fit right now
		m_pWander->Init(m_pBrain, uThoughtFlags, NULL);	 
		m_pWander->m_fWanderRadius = pBrain->GetAIMover()->GetRadiusXZ()*10.0f;							//wander radius will expand with time
		m_pWander->m_uWanderFlags &= ~CGenericWander::WANDER_IGNORERADIUSLIMIT;
	}

	m_InvestigatePt.Zero();
	m_fInvestigateTimeOut  =0.0f;
	m_uSearchState = SEARCHSTATE_WANDER;

	m_pBrain->PushBaseSpeedSetting(20);	 //50% speed while searching. lower if alert
}


void CGroundSearch::Cleanup(void)
{
	if (m_pWander)
	{
		m_pWander->Cleanup();
		CGenericWander::BankAccess(m_pWander);
		m_pWander = NULL;
	}

	if (m_pLookAtMgr && (m_uSearchFlags & SEARCHFLAG_RECYCLE_LOOKATMGR))
	{
		CGenericWait::BankAccess(m_pLookAtMgr);
		m_pLookAtMgr = NULL;
	}
	else if (m_pLookAtMgr)
	{
	}
	
	CAIThought::Cleanup();
	m_pBrain->PopBaseSpeedSetting();
	m_pBrain->ClearFlag_Scan();
}


void CGroundSearch::Investigate(const CFVec3A& Pt_WS)
{
   	m_InvestigatePt = Pt_WS;
	
	m_fInvestigateTimeOut = 0.0f; //we're moving toward the investigate pt now.

	if (m_uSearchState == SEARCHSTATE_WANDER)
	{
		m_uSearchState = SEARCHSTATE_INVESTIGATE_PF;

		u16 uSearchFlags = CSearchQuery::SEARCHPARAM_NEED_AIPATH | CSearchQuery::SEARCHPARAM_FUDGE_DEST | CSearchQuery::SEARCHPARAM_FLOOR | CSearchQuery::SEARCHPARAM_NO_SWITCHES | CSearchQuery::SEARCHPARAM_NO_LIFTS;
		aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
		if (m_SearchQuery.GetResults())
		{
			m_SearchQuery.GetResults()->Clear();
		}


		m_SearchQuery.Clear();
		m_SearchQuery.SetDefaultSearchParams(m_pBrain->GetAIMover()->GetLoc(),
												Pt_WS,
												m_pBrain->GetAIMover()->GetRadiusXZ(), //w
												m_pBrain->GetAIMover()->GetMinHeight(), //h
												&m_SearchResults,
												uSearchFlags,
												(u32) m_pBrain);
		aimain_pGraphSearcher->SubmitQuery(&m_SearchQuery);
		m_SearchQuery.SetFudgeDestDist(7.0);
		m_SearchQuery.SetMaxJumpDist(GetBrain()->GetMaxJumpDist());
	}

}

BOOL _bSearchUseHeadScan = 0;
BOOL _bSearchUseTorsoScan = 1;

void CGroundSearch::SetLookAtMgr(CGenericWait* pLookAtMgr)
{
	if (m_pLookAtMgr && (m_uSearchFlags & SEARCHFLAG_RECYCLE_LOOKATMGR))
	{
		CGenericWait::BankAccess(m_pLookAtMgr);
		m_pLookAtMgr = NULL;
	}
	m_pLookAtMgr = pLookAtMgr;
}


void CGroundSearch::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();

	if (m_uSearchState == SEARCHSTATE_WANDER)
	{
		if (m_pWander)
		{
			m_pWander->Work();
			if (m_pWander->m_fWanderRadius < pMover->GetRadiusXZ()*10.0f+100.0f)
			{
				m_pWander->m_fWanderRadius+=FLoop_fPreviousLoopSecs; //half foot per second
			}
			else
			{
				m_pWander->m_fWanderRadius  = pMover->GetRadiusXZ()*10.0f+100.0f;
			}

			if (m_pLookAtMgr)
			{
				if (!m_pLookAtMgr->IsVisRayScanPaused())
				{
					 //Alert Mode
					pMover->m_Controls.Alert();		//Alert doesn't stick, so dude will only be in alert while in state_base
				}

			}
		}
	}
	else if (m_uSearchState == SEARCHSTATE_INVESTIGATE_PF)
	{
		if (m_SearchQuery.IsDone())
		{
			m_SearchQuery.SetHasBeenDone();
			m_pBrain->GetAIMover()->AssignPath(m_SearchQuery.GetResults()->GetAIPath());
			m_uSearchState = SEARCHSTATE_INVESTIGATE_ENROUTE;
		}
		else if (m_SearchQuery.HasFailed())
		{
			m_uSearchState = SEARCHSTATE_WANDER;
		}
	}
	else if (m_uSearchState == SEARCHSTATE_INVESTIGATE_ENROUTE)
	{
		if (m_pBrain->GetAIMover()->FollowPath())
		{
			m_uSearchState = SEARCHSTATE_WANDER;

			if (m_pWander)
			{
				m_pWander->Cleanup();
				m_pWander->Init(m_pBrain, this->m_uThoughtFlags, NULL);
				m_pWander->m_fWanderRadius = 20.0f;							//wander radius will expand with time
				m_pWander->m_uWanderFlags &= ~CGenericWander::WANDER_IGNORERADIUSLIMIT;
			} 
		}
	}

//	m_pBrain->SetFlag_Scan();
	if (m_pLookAtMgr && _bSearchUseTorsoScan)
	{
		m_pLookAtMgr->m_uWaitFlags |= CGenericWait::WAIT_AUTOSCAN_TORSO;
		m_pLookAtMgr->m_uPausedVisRayScanMin = 10;
		m_pLookAtMgr->m_uPausedVisRayScanMax = 15;
	}

}




//
//  Pill Box Wait
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
///////////////////////

void CPillWait::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /*= NULL*/)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
}


void CPillWait::Cleanup(void)
{
	CAIThought::Cleanup();
}


void CPillWait::Work(void)
{
	//every so often, look for somebody who might want to be driving
	CBotSiteWeapon* pSite = (CBotSiteWeapon*) m_pBrain->GetAIMover()->GetEntity();
/*	if (!pSite->GetDriverBot())
	{
		CAIMemoryLarge* pSeenMem = NULL;
		if (m_pBrain->GetKnowledge().CanRemember(MEMORY_LARGE_SEEN,

	}
*/

}


//
//  Scout Bot alert state
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
///////////////////////
#define _SWITCH_HEIGHT		( 3.0f )

cchar* apszScoutStages[] =
{
	"SS_SEE_TARGET",
	"SS_SCANFOR_TARGET",
	"SS_SURPRISED",
	"SS_FIND_PATH",
	"SS_GOTO",
	"SS_SWITCH",
	"SS_NIRVANA",
};

void CScoutAlert::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /*= NULL*/)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	m_pGotoThought = (CGenericGoto*) CGenericGoto::BankAccess(NULL);
	m_pIntruder = NULL;
	m_uAlarmGotoFailCount = 0;
	if (!m_pGotoThought)
	{
		m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;
	}

	FASSERT(sizeof(apszScoutStages)/sizeof(cchar*) == NUM_SCOUTSTAGES);

	u32 uTargetGUID;
	s16 sEventId;
	ExtractAttackParams((CAIMemoryLarge*) pParamPack,	&uTargetGUID,	&sEventId);
	if (uTargetGUID)
	{
		m_pIntruder = CEntity::Find(uTargetGUID);
	}

	u8 uNormalRuleSet;					// which ruleset the brain will use when attacking
	u8 uAlternateRuleSet;
	u8 uAlternateRuleSetOdds;			// (0-100) pct chance that the alternate will be used
	u8 uAttackSpeedMin;					// (0-100)
	u8 uAttackSpeedMax;					// (0-100)
	u8 uAttackSpeed;
	u16 uMaxDistFromOrigin;
	u16 uStayPutDist;
	u8 uMeleeRad;
	CAIMemoryLarge *pAttackSpecsParamPack = NULL;
	if( m_pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_ATTACKSPECS,(CAIMemorySmall**) &pAttackSpecsParamPack) )
	{
		ExtractAttackSpecs(	pAttackSpecsParamPack,
							&uNormalRuleSet,
							&uAlternateRuleSet,
							&uAlternateRuleSetOdds,
							&uAttackSpeedMin,
							&uAttackSpeedMax,
							&uMaxDistFromOrigin,
							&uStayPutDist,
							&uMeleeRad);

		uAttackSpeed = uAttackSpeedMin + (u8) fmath_RandomChoice(uAttackSpeedMax - uAttackSpeedMin);
		m_pBrain->PushBaseSpeedSetting(uAttackSpeed);
	}
	else
	{
		m_pBrain->PushBaseSpeedSetting(100);
	}
	m_uNirvanaTimeOut = 0;

	m_uScoutStage = SCOUTSTAGE_SCANFOR_TARGET;
	m_uGoalTimeOut = aiutils_GetTimeOutRandMinMax((u8)1, (u8)3);
	if (m_pIntruder)
	{
		if (GetBrain()->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, m_pIntruder, NULL))
		{
			m_uScoutStage = SCOUTSTAGE_SEE_TARGET;
		}
	}

}


void CScoutAlert::Cleanup(void)
{
	CAIThought::Cleanup();

	m_pBrain->PopBaseSpeedSetting();

	if (m_pGotoThought)
	{
		m_pGotoThought->Cleanup();
		CGenericGoto::BankAccess(m_pGotoThought);
		m_pGotoThought = NULL;
	}
}

BOOL CScoutAlert::HasLOSToSwitch(CBot* pBot)
{
	CFVec3A GotoLoc;
	GotoLoc = m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vFront;
	GotoLoc.Mul(m_pBrain->GetAIMover()->GetRadiusXZ());
	GotoLoc.y += _SWITCH_HEIGHT;
	GotoLoc.Add(m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos);
	
	return !aiutils_IsLOSObstructed_IgnoreBots(GetBrain()->GetAIMover()->GetEyePos(), GotoLoc, pBot);

}

void CScoutAlert::Work(void)
{
	CAIBrain* pBrain = GetBrain();
	CAIMover* pMover = GetBrain()->GetAIMover();
	FASSERT((pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT));
	CBot *pBot = (CBot*)pBrain->GetAIMover()->GetEntity();
	CBotScout *pBotScout = (CBotScout *) pBrain->GetAIMover()->GetEntity();	 //o.k to be null so that other bots can use this AI, even if they don't have the fancy scout beam effect 
	BOOL bCheckedLOSThisFrame = FALSE;
	BOOL bHasLOSThisFrame = FALSE;

	//exit conditions
	if (m_pIntruder && (!m_pIntruder->IsInWorld() || ((m_pIntruder->TypeBits() & ENTITY_BIT_BOT) && ((CBot*)m_pIntruder)->IsDeadOrDying()) ))
	{
		//don't know why I am on attack, go back to job
		this->m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;
		return;
	}

	m_uThoughtFlags &= ~CAIThought::THOUGHTFLAG_USE_JOB_REACT_RULES;
	switch (m_uScoutStage)
	{
		case SCOUTSTAGE_SEE_TARGET:
			if (pBotScout)
			{
				f32 fDist = 0.0f;
				if (m_pIntruder)
				{	
					fDist = m_pIntruder->MtxToWorld()->m_vPos.Dist(pBot->MtxToWorld()->m_vPos);
				}

				pBotScout->TargetSighted(fDist);	//causes a suprise hop, then head retracts permanently
			}
			m_uScoutStage = SCOUTSTAGE_SURPRISED;
			break;
		case SCOUTSTAGE_SCANFOR_TARGET:
			{
				if (m_pIntruder && GetBrain()->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, m_pIntruder, NULL))
				{
					m_uScoutStage = SCOUTSTAGE_SEE_TARGET;
				}
				else if (!m_pIntruder)
				{	//no intruder
					
					CAIMemoryLarge* pDamagedMem = NULL;
					if (GetBrain()->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall**) &pDamagedMem))
					{
						if (pDamagedMem->m_uSmallData == CDamageForm::DAMAGE_LOCALE_IMPACT)
						{
							m_uScoutStage = SCOUTSTAGE_SEE_TARGET;
							m_pIntruder = pDamagedMem->m_pEntity;
						}
						else if (pDamagedMem->m_uSmallData == CDamageForm::DAMAGE_LOCALE_BLAST && pDamagedMem->m_pEntity)
						{
							if (pDamagedMem->m_pEntity->MtxToWorld()->m_vPos.DistSq(pMover->GetLoc()) < 100.0f*100.0f)
							{
								if (aiutils_IsLOSObstructed_IgnoreBots(pMover->GetEyePos(), *(pDamagedMem->m_pEntity->GetTagPoint(0)), pBot))
								{
									m_uScoutStage = SCOUTSTAGE_SEE_TARGET;
									m_pIntruder = pDamagedMem->m_pEntity;
								}
							}
							else
							{

							}
						}
					}

					if (m_uScoutStage == SCOUTSTAGE_SCANFOR_TARGET)
					{
						m_uThoughtFlags |= CAIThought::THOUGHTFLAG_USE_JOB_REACT_RULES;	 //tell reaction system to pick up on enemies just like normal
					}

					CFVec3A GoTo;
					GoTo = pMover->GetEyeLookAt();
					f32 fTmp = GoTo.x;
					GoTo.x = GoTo.z;
					GoTo.z = -fTmp;
					GoTo.Mul(3.0f);
					GoTo.Add(pMover->GetLoc());

					if (pBotScout)
					{
						//scout doesn't respond to facetoward,
						//so use movetoward to move in a circle
						//
						pMover->MoveToward(GoTo);
					}
					else
					{
						pMover->MoveToward(pMover->GetLoc());
						pMover->FaceToward(GoTo);
					}
					if (m_uGoalTimeOut < aiutils_GetCurTimeSecsU16())
					{
						m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;
					}
				}
			}
			break;
		case SCOUTSTAGE_SURPRISED:
			if( pBotScout)
			{
				if (!pBotScout->IsSurprised() )  //wait for anim
				{
					m_uScoutStage = SCOUTSTAGE_FIND_PATH;
				}
			}
			else
			{
				m_uScoutStage = SCOUTSTAGE_FIND_PATH;
			}
			break;

		case SCOUTSTAGE_FIND_PATH:
			{
				m_pSwitch = NULL;
				if (m_uAlarmGotoFailCount < CAlarmSwitch::m_AlarmSwitchList.nCount)
				{
					CAlarmSwitch* pSwitch = (CAlarmSwitch*) flinklist_GetHead(&CAlarmSwitch::m_AlarmSwitchList);
					f32 fBestDistSq = 0.0f;
					while (pSwitch)
					{
						f32 fDistSq = pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos.DistSq(m_pBrain->GetAIMover()->GetLoc());
						if (pSwitch->IsWorking() && 
							(!m_pSwitch ||
								fDistSq < fBestDistSq))
						{
							m_pSwitch = pSwitch;
							fBestDistSq = fDistSq;
						}
						pSwitch = (CAlarmSwitch*) flinklist_GetNext(&CAlarmSwitch::m_AlarmSwitchList, pSwitch);
					}
				}

				if (m_pSwitch)
				{
					CFVec3A GotoLoc;
					GotoLoc = m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vFront;
					GotoLoc.Mul(m_pBrain->GetAIMover()->GetRadiusXZ() + 3.0f);
					GotoLoc.Add(m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos);
					m_pGotoThought->Init(	m_pBrain,
											THOUGHTFLAG_NONE,
											CAIBrain::TT_INVALID,   //not being used as a brain current thought
											GotoLoc,
											10 );	   //fudge dist

					m_uScoutStage = SCOUTSTAGE_GOTO;
				}
				else
				{
					m_uAlarmGotoFailCount = CAlarmSwitch::m_AlarmSwitchList.nCount;
				}
			}
			break;
		case SCOUTSTAGE_GOTO:
			{
				
				m_pGotoThought->Work();
				if (!m_pSwitch || !m_pSwitch->IsWorking() || !m_pGotoThought)
				{
					m_uScoutStage = SCOUTSTAGE_FIND_PATH;
					m_uAlarmGotoFailCount++;
				}
				else if (m_pGotoThought->GetThoughtFlags() & THOUGHTFLAG_GOAL_FAILED)
				{
					m_uScoutStage = SCOUTSTAGE_FIND_PATH;
					m_uAlarmGotoFailCount++;
				}
				else 
				{
					if (m_pGotoThought->GetThoughtFlags() & THOUGHTFLAG_GOAL_COMPLETE)
					{
						m_uScoutStage = SCOUTSTAGE_SWITCH;
					}
					else
					{
						//if close to the switch,
						//can see it, and
						//am stopped, then just 
						//consider self to be close enough
						if (pMover->IsStopped() && 
							pBot->MtxToWorld()->m_vPos.DistSq(m_pGotoThought->m_GoalLoc) < (3.0f+pMover->GetRadiusXZ())*(3.0f+pMover->GetRadiusXZ()))
						{
							bCheckedLOSThisFrame = TRUE;
							bHasLOSThisFrame = HasLOSToSwitch(pBot);
							m_uScoutStage = SCOUTSTAGE_SWITCH;
						}
					}
					
					//findfix: remove this when scout gets special effects
	//#if !FANG_PRODUCTION_BUILD
	//					if (m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos.DistSq(m_pBrain->GetAIMover()->GetLoc()) < (m_pBrain->GetAIMover()->GetRadiusXZ()+ 5.0f)*(m_pBrain->GetAIMover()->GetRadiusXZ() + 5.0f))
	//					{
	//						_aiutils_DebugTrackRay(m_pBrain->GetAIMover()->GetLoc(), m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos, 2);
	//					}
	//#endif
	//
				}
			}

			break;
		case SCOUTSTAGE_SWITCH:
			{
				//little thing so that the goto thought we are using gives back it's resources
				if (m_pGotoThought->m_uStage == CGenericGoto::GOTOSTAGE_EN_ROUTE)
				{
					m_pGotoThought->m_uStage = CGenericGoto::GOTOSTAGE_CLOSEENOUGH;
				}


				CAlarmNet* pNet = NULL;
				if (m_pSwitch)
				{
					pNet = CAlarmNet::FindAlarmNet(m_pSwitch->m_uAlarmNetId);
				}

				if (pNet && pNet->m_uNetState == CAlarmNet::NETSTATE_OFF)
				{
					if (m_pSwitch && m_pSwitch->m_pAlarmSwitchEntity)
					{
						CFVec3A vDelta;
						f32 fDist;

						if (m_pSwitch->m_pAlarmSwitchEntity->GetMesh())
						{
							vDelta.Set( m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos );
							vDelta.Sub( pBot->MtxToWorld()->m_vPos );
							fDist = vDelta.Mag();
				
							if( fDist > ( m_pSwitch->m_pAlarmSwitchEntity->GetMesh()->GetBoundingSphere().m_fRadius + ACTION_BUTTON_TEST_RADIUS ) * 0.9f )
							{
								// scout has moved too far away, try to get closer
								m_uScoutStage = SCOUTSTAGE_FIND_PATH;
							}
						}
					}

					// hold down the alarm button so "download" time advances
					m_pBrain->GetAIMover()->m_Controls.Action();

				}
				else
				{
					m_uScoutStage = SCOUTSTAGE_NIRVANA;
					m_uNirvanaTimeOut = 0;//aiutils_GetTimeOutRandMinMax((u8)4, (u8)20);
				}

				GetBrain()->GetAIMover()->MoveToward(GetBrain()->GetAIMover()->GetLoc());
				if (m_pSwitch && m_pSwitch->m_pAlarmSwitchEntity)
				{
					CFVec3A LookAt_WS;
					LookAt_WS = m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos;
					LookAt_WS.y +=_SWITCH_HEIGHT; 
					GetBrain()->GetAIMover()->FaceToward(LookAt_WS);
				}
			}

			break;
		case SCOUTSTAGE_NIRVANA:
			if (m_pGotoThought)
			{
				m_pGotoThought->Cleanup();
				CGenericGoto::BankAccess(m_pGotoThought);
				m_pGotoThought = NULL;
			}

			if (m_uNirvanaTimeOut < aiutils_GetCurTimeSecsU16() ||
				(m_pIntruder && m_pIntruder->MtxToWorld()->m_vPos.DistSq(pMover->GetLoc()) < (20.0f+pMover->GetRadiusXZ())*(20.0f+pMover->GetRadiusXZ())))
			{
				if (pBotScout)
				{
					pBotScout->SetTickRate(1.0f);
				}
				ai_AssignJob_Wander(GetBrain(), 200.0f, TRUE, 30, 40, 0, 0, 0, NULL);
				GetBrain()->ClearFlag_AttackPlayer();
				GetBrain()->ClearFlag_AttackNPC();
				ai_TurnOffPerceptor(GetBrain(), AI_PERCEPTOR_EYES );
				ai_TurnOffPerceptor(GetBrain(), AI_PERCEPTOR_EARS );
				ai_TurnOffPerceptor(GetBrain(), AI_PERCEPTOR_TOUCH);
				ai_TurnOffPerceptor(GetBrain(), AI_PERCEPTOR_RADIO);

				m_uThoughtFlags |= THOUGHTFLAG_GOAL_COMPLETE;
			}
			break;

	}

	// draw scout beam effects whenever los is obtained to the switch
	if (m_uScoutStage > SCOUTSTAGE_FIND_PATH &&
		m_pSwitch &&
		m_pSwitch->m_pAlarmSwitchEntity && 
		pBotScout)
	{
	
		if (m_uScoutStage == SCOUTSTAGE_NIRVANA ||
			m_uScoutStage == SCOUTSTAGE_SWITCH ||
			((bCheckedLOSThisFrame && bHasLOSThisFrame) || HasLOSToSwitch(pBot)))
		{
			CFVec3A vBeamEnd;
			vBeamEnd.Set( m_pSwitch->m_pAlarmSwitchEntity->MtxToWorld()->m_vPos );
			vBeamEnd.y += _SWITCH_HEIGHT;
			

			f32 fLerp = 0.0f;


			if (m_uScoutStage == SCOUTSTAGE_SWITCH &&
				m_pSwitch &&
				m_pSwitch->m_fPressButtonTime > 0.0f)
			{

				//						CFVec3A ScreenPos;
				//						if (aiutils_BrainLocToScreenPos(m_pBrain, &ScreenPos))
				//						{
				//							ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w1 UPLOADING %f", 100.0f - 100.0f*				//						}
				fLerp = ((f32)m_pSwitch->m_fPressButtonTimeLeft) / m_pSwitch->m_fPressButtonTime;
			}
			pBotScout->ActivateBeam( vBeamEnd, (m_uScoutStage == SCOUTSTAGE_SWITCH), fLerp );


		}
	}

	//don't forget about the intruder
	if (m_pIntruder)
	{
		CAIMemoryLarge* pSeenMem = NULL;
		if (m_pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, m_pIntruder, &pSeenMem))
		{
			pSeenMem->ResetTimer(3);
		}
	}
}


//
//
//  Ground Attack
//
//
//
static u16 _kuDefaultLocalAttackSearchRange = 3;
static u16 _kuDefaultLocalAttackSearchRangeInc = 3;

void CGroundCombatWorkContext::Init(CGroundCombat* pT)
{
	pMover = pT->GetBrain()->GetAIMover();
	EnemyMark.Zero();
	EnemyMarkTagPos.Zero();
	pSeenMem = NULL;
	pHeardMem = NULL;
	pDamagedByMem = NULL;
	pDamageDealtMem = NULL;
	bHasLOSThisFrame = FALSE;
	pEnemyDamagedByMem = NULL;
	uLastFrameNowTime = aiutils_GetLastTimeSecsU16();
	uNowTime = aiutils_GetCurTimeSecsU16();
	fNowTime = aiutils_GetCurTimeSecs();
	fAlignment = 1.0f;
	bLostTheEnemy = FALSE;
	bSensesDanger = FALSE;
	fDistToEnemyMark = 0.0f;
	fDistToEnemyMarkXZ = 0.0f;
	fAggression = pT->GetBrain()->GetAttrib(CAIBrain::ATTRIB_AGGRESSION)*kfOOHun;
	fIntelligence = pT->GetBrain()->GetAttrib(CAIBrain::ATTRIB_INTELLIGENCE)*kfOOHun;
	fCourage = pT->GetBrain()->GetAttrib(CAIBrain::ATTRIB_COURAGE)*kfOOHun;
	pEnemyPoi = NULL;
	fDistToEnemyPoi = 0.0f;
	pEnemyBot = NULL;

	if (pT->m_pEnemy && pT->m_pEnemy->TypeBits() & ENTITY_BIT_BOT)
	{
		pEnemyBot = (CBot*) pT->m_pEnemy;
	}
	if (pMover->GetEntity() && pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
	{
		pBot = (CBot*) pMover->GetEntity();
	}

	if (pT->m_pAttackSpecsParamPack)
	{
		ExtractAttackSpecs(	pT->m_pAttackSpecsParamPack,
							&uNormalRuleSet,
							&uAlternateRuleSet,
							&uAlternateRuleSetOdds,
							&uAttackSpeedMin,
							&uAttackSpeedMax,
							&uMaxDistFromOrigin,
							&uStayPutDist,
							&(pT->m_uMeleeRad));
	}
	else
	{ //defaults if no paramspec exists
		uMaxDistFromOrigin = 0xffff;
		uNormalRuleSet = CAIBrain::ATTACKRULESET_0;
		uAlternateRuleSet = CAIBrain::ATTACKRULESET_NONE;
		uAlternateRuleSetOdds = 0;
		uAttackSpeedMin = 85;
		uAttackSpeedMax = 100;
		uStayPutDist = 0xffff;
	}

	if (pT->m_uAttackFlags & CGroundCombat::FLAG_3DBOT)
	{
		pFindEnemyRelPos_Func = &CGroundCombat::FindEnemyRelPos_3D;
		pFindCurRelPos_Func = &CGroundCombat::FindCurRelPos_3D;
		pFindLocalPoi_Func = &CGroundCombat::FindLocalPoi_3D;
		pFindCoverPos_Func = &CGroundCombat::FindCoverPos_3D;
		pFindRetreatPos_Func = &CGroundCombat::FindRetreatPos_3D;
		pFindRangeAcquirePos_Func = &CGroundCombat::FindRangeAcquirePos_3D;
		pFindAdvancePos_Func = &CGroundCombat::FindAdvancePos_3D;
	}
	else
	{
		pFindEnemyRelPos_Func = &CGroundCombat::FindEnemyRelPos_Ground;
		pFindCurRelPos_Func = &CGroundCombat::FindCurRelPos_Ground;
		pFindLocalPoi_Func = &CGroundCombat::FindLocalPoi_Ground;
		pFindCoverPos_Func = &CGroundCombat::FindCoverPos_Ground;
		pFindRetreatPos_Func = &CGroundCombat::FindRetreatPos_Ground;
		pFindRangeAcquirePos_Func = &CGroundCombat::FindRangeAcquirePos_Ground;
		pFindAdvancePos_Func = &CGroundCombat::FindAdvancePos_Ground;
	}
}

/*static */CGroundCombatWorkContext *CGroundCombat::s_pWD =NULL;
static CGroundCombatWorkContext WD;

CFSMStateDef s_aTacticFSMStateDefs[] = 
{										
	CFSMStateDef("TACTIC_NONE",			_GANoneInit,			_GANoneWork,			_GANoneUninit,			CGroundCombat::TACTIC_NONE,				0,	NULL),
	CFSMStateDef("TACTIC_STANDGROUND",	_GAStandGroundInit,		_GAStandGroundWork,		_GAStandGroundUninit,	CGroundCombat::TACTIC_STANDGROUND,		0,	NULL),
	CFSMStateDef("TACTIC_RANGEATTACK",	_GARangeInit,			_GARangeWork,			_GARangeUninit,			CGroundCombat::TACTIC_RANGEATTACK,		0,	NULL),
	CFSMStateDef("TACTIC_CIRCLESTRAFE",	_GACircleInit,			_GACircleWork,			_GACircleUninit,		CGroundCombat::TACTIC_CIRCLESTRAFE,		0,	NULL),
	CFSMStateDef("TACTIC_PEEKABOO",		_GAPeekABooInit,		_GAPeekABooWork,		_GAPeekABooUninit,		CGroundCombat::TACTIC_PEEKABOO,			0,	NULL),
	CFSMStateDef("TACTIC_SEARCH",		_GASearchInit,			_GASearchWork,			_GASearchUninit,		CGroundCombat::TACTIC_SEARCH,			0,	NULL),
	CFSMStateDef("TACTIC_RANGE3D",		_GARange3DInit,			_GARange3DWork,			_GARange3DUninit,		CGroundCombat::TACTIC_RANGE3D,			0,	NULL),
};
	

CFSMStateDef s_aRuleSetFSMStateDefs[] = 
{	
	CFSMStateDef("ATTACKRULESET_0",				InitRuleSet_0,				DoRuleSet_0,			NULL,	CAIBrain::ATTACKRULESET_0,					0,	NULL),
	CFSMStateDef("ATTACKRULESET_1",				InitRuleSet_1,				DoRuleSet_1,			NULL,	CAIBrain::ATTACKRULESET_1,					0,	NULL),
	CFSMStateDef("ATTACKRULESET_NPC",			InitRuleSet_NPC,			DoRuleSet_NPC,			NULL,	CAIBrain::ATTACKRULESET_NPC,				0,	NULL),
	CFSMStateDef("ATTACKRULESET_TITAN0",		InitRuleSet_Titan0,			DoRuleSet_Titan0,		NULL,	CAIBrain::ATTACKRULESET_TITAN0,				0,	NULL),
	CFSMStateDef("ATTACKRULESET_PRED0",			InitRuleSet_Pred0,			DoRuleSet_Pred0,		NULL,	CAIBrain::ATTACKRULESET_PRED0,				0,	NULL),
	CFSMStateDef("ATTACKRULESET_PROBE0",		InitRuleSet_Probe0,			DoRuleSet_Probe0,		NULL,	CAIBrain::ATTACKRULESET_PROBE0,				0,	NULL),
	CFSMStateDef("ATTACKRULESET_RAT0",			InitRuleSet_Rat0,			DoRuleSet_Rat0,			NULL,	CAIBrain::ATTACKRULESET_RAT0,				0,	NULL),
	CFSMStateDef("ATTACKRULESET_ELITEGUARD0",	InitRuleSet_EliteGuard0,	DoRuleSet_EliteGuard0,	NULL,	CAIBrain::ATTACKRULESET_ELITEGUARD0,		0,	NULL),
	CFSMStateDef("ATTACKRULESET_ZOMBIE0",		InitRuleSet_Zombie0,		DoRuleSet_Zombie0,		NULL,	CAIBrain::ATTACKRULESET_ZOMBIE0,			0,	NULL),
	CFSMStateDef("ATTACKRULESET_CORROSIVE0",	InitRuleSet_Corrosive0,		DoRuleSet_Corrosive0,	NULL,	CAIBrain::ATTACKRULESET_CORROSIVE0,			0,	NULL),
	CFSMStateDef("ATTACKRULESET_ZOMBIEBOSS0",	InitRuleSet_ZombieBoss0,	DoRuleSet_ZombieBoss0,	NULL,	CAIBrain::ATTACKRULESET_ZOMBIEBOSS0,		0,	NULL),
	CFSMStateDef("ATTACKRULESET_JUMPER0",		InitRuleSet_Jumper0,		DoRuleSet_Jumper0,		NULL,	CAIBrain::ATTACKRULESET_JUMPER0,			0,	NULL),
	CFSMStateDef("ATTACKRULESET_JUMPER1",		InitRuleSet_Jumper0,/*ok*/	DoRuleSet_Jumper0,/*ok*/NULL,	CAIBrain::ATTACKRULESET_JUMPER1,			0,	NULL),
	CFSMStateDef("ATTACKRULESET_SCIENTIST",		InitRuleSet_Scientist0,		DoRuleSet_Scientist0,	NULL,	CAIBrain::ATTACKRULESET_SCIENTIST0,			0,	NULL),
	CFSMStateDef("ATTACKRULESET_NONE",			NULL,						NULL,					NULL,	CAIBrain::ATTACKRULESET_NONE,				0,	NULL)
};


CFSMStateDef s_aRuleSetStateFSMStateDefs[] = 
{
	CFSMStateDef("ARS_0_BASE",					InRules_ARS_0_Base,				DoRules_ARS_0_Base,							NULL,		CGroundCombat::ARS_0_BASE,						0,	NULL),
	CFSMStateDef("ARS_0_HIDE",					InRules_ARS_0_Hide,				DoRules_ARS_0_Hide,							NULL,		CGroundCombat::ARS_0_HIDE,						0,	NULL),
	CFSMStateDef("ARS_0_HIDEFROMDAMAGE",		InRules_ARS_0_HideFromDamage,	DoRules_ARS_0_HideFromDamage,				NULL,		CGroundCombat::ARS_0_HIDEFROMDAMAGE,			0,	NULL),
	CFSMStateDef("ARS_0_INVMARK",				InRules_ARS_0_InvestigateMark,	DoRules_ARS_0_InvestigateMark,				NULL,		CGroundCombat::ARS_0_INVESTIGATEMARK,			0,	NULL),
	CFSMStateDef("ARS_0_CHALLENGE",				InRules_ARS_0_Challenge,		DoRules_ARS_0_Challenge,					NULL,		CGroundCombat::ARS_0_CHALLENGE,					0,	NULL),
	CFSMStateDef("ARS_0_SEARCH",				InRules_ARS_0_Search,			DoRules_ARS_0_Search,						NULL,		CGroundCombat::ARS_0_SEARCH,					0,	NULL),
	CFSMStateDef("ARS_0_SITEGUNNER",			InRules_ARS_0_SiteGunner,		DoRules_ARS_0_SiteGunner,					NULL,		CGroundCombat::ARS_0_SITEGUNNER,				0,	NULL),
	CFSMStateDef("ARS_0_PASSENGER",				NULL					,		DoRules_ARS_0_Passenger,					NULL,		CGroundCombat::ARS_0_PASSENGER,					0,	NULL),
	CFSMStateDef("ARS_0_DRIVER",				InRules_ARS_0_Driver,			DoRules_ARS_0_Driver,						NULL,		CGroundCombat::ARS_0_DRIVER,					0,	NULL),
//////////										
	CFSMStateDef("ARS_1_BASE",					NULL,							NULL,										NULL,		CGroundCombat::ARS_1_BASE,						0,	NULL),
/////////
	CFSMStateDef("ARS_NPC_BASE",					InRules_ARS_NPC_Base,						DoRules_ARS_NPC_Base,						NULL,		CGroundCombat::ARS_NPC_BASE,					0,	NULL),
	CFSMStateDef("ARS_NPC_INVMARK",					InRules_ARS_NPC_InvestigateMark,			DoRules_ARS_NPC_InvestigateMark,			NULL,		CGroundCombat::ARS_NPC_INVESTIGATEMARK,			0,	NULL),
//////////
	CFSMStateDef("ARS_TITAN0_BASE",					InRules_ARS_Titan0_Base,					DoRules_ARS_Titan0_Base,					NULL,		CGroundCombat::ARS_TITAN0_BASE,					0,	NULL),
	CFSMStateDef("ARS_TITAN0_INVESTIGATEMARK",		InRules_ARS_Titan0_InvestigateMark,			DoRules_ARS_Titan0_InvestigateMark,			NULL,		CGroundCombat::ARS_TITAN0_INVESTIGATEMARK,		0,	NULL),
	CFSMStateDef("ARS_TITAN0_SEARCH",				InRules_ARS_Titan0_Search,					DoRules_ARS_Titan0_Search,					NULL,		CGroundCombat::ARS_TITAN0_SEARCH,				0,	NULL),
/////////
	CFSMStateDef("ARS_PRED0_BASE",					InRules_ARS_Pred0_Base,						DoRules_ARS_Pred0_Base,						NULL,		CGroundCombat::ARS_PRED0_BASE,					0,	NULL),
	CFSMStateDef("ARS_PRED0_INVESTIGATEMARK",		InRules_ARS_Pred0_InvestigateMark,			DoRules_ARS_Pred0_InvestigateMark,			NULL,		CGroundCombat::ARS_PRED0_INVESTIGATEMARK,		0,	NULL),
	CFSMStateDef("ARS_PRED0_SEARCH",				InRules_ARS_Pred0_Search,					DoRules_ARS_Pred0_Search,					NULL,		CGroundCombat::ARS_PRED0_SEARCH,				0,	NULL),
	CFSMStateDef("ARS_PRED0_STRAFE",				InRules_ARS_Pred0_Strafe,					DoRules_ARS_Pred0_Strafe,					NULL,		CGroundCombat::ARS_PRED0_STRAFE,				0,	NULL),
/////////
	CFSMStateDef("ARS_PROBE0_BASE",					NULL,										NULL,										NULL,		CGroundCombat::ARS_PROBE0_BASE,					0,	NULL),
//////////
	CFSMStateDef("ARS_RAT0_BASE",					NULL,										NULL,										NULL,		CGroundCombat::ARS_RAT0_BASE,					0,	NULL),
//////////
	CFSMStateDef("ARS_ELITEGUARD0_BASE",			InRules_ARS_EliteGuard0_Base,				DoRules_ARS_EliteGuard0_Base,				NULL,		CGroundCombat::ARS_ELITEGUARD0_BASE,			0,	NULL),
	CFSMStateDef("ARS_ELITEGUARD0_HIDEFROMDAMAGE",	InRules_ARS_EliteGuard0_HideFromDamage,		DoRules_ARS_EliteGuard0_HideFromDamage,		NULL,		CGroundCombat::ARS_ELITEGUARD0_HIDEFROMDAMAGE,	0,	NULL),
	CFSMStateDef("ARS_ELITEGUARD0_USECOVER",		InRules_ARS_EliteGuard0_UseCover,			DoRules_ARS_EliteGuard0_UseCover,			NULL,		CGroundCombat::ARS_ELITEGUARD0_USECOVER,		0,	NULL),
	CFSMStateDef("ARS_ELITEGUARD0_WATCHDOG",		InRules_ARS_EliteGuard0_WatchDog,			DoRules_ARS_EliteGuard0_WatchDog,			NULL,		CGroundCombat::ARS_ELITEGUARD0_WATCHDOG,		0,	NULL),
	CFSMStateDef("ARS_ELITEGUARD0_INVESTIGATEMARK",	InRules_ARS_EliteGuard0_InvestigateMark,	DoRules_ARS_EliteGuard0_InvestigateMark,	NULL,		CGroundCombat::ARS_ELITEGUARD0_INVESTIGATEMARK,	0,	NULL),
	CFSMStateDef("ARS_ELITEGUARD0_SEARCH",			InRules_ARS_EliteGuard0_Search,				DoRules_ARS_EliteGuard0_Search,				NULL,		CGroundCombat::ARS_ELITEGUARD0_SEARCH,			0,	NULL),
/////////
	CFSMStateDef("ARS_ZOMBIE0_BASE",				InRules_ARS_Zombie0_Base,					DoRules_ARS_Zombie0_Base,					NULL,		CGroundCombat::ARS_ZOMBIE0_BASE,				0,	NULL),
	CFSMStateDef("ARS_ZOMBIE0_QUITTER",				InRules_ARS_Zombie0_Quitter,				DoRules_ARS_Zombie0_Quitter,				OutRules_ARS_Zombie0_Quitter,		CGroundCombat::ARS_ZOMBIE0_QUITTER,				0,	NULL),
	CFSMStateDef("ARS_ZOMBIE0_HIDEFROMDAMAGE",		InRules_ARS_Zombie0_HideFromDamage,			DoRules_ARS_Zombie0_HideFromDamage,			NULL,		CGroundCombat::ARS_ZOMBIE0_HIDEFROMDAMAGE,		0,	NULL),
	CFSMStateDef("ARS_ZOMBIE0_SEARCH",				InRules_ARS_Zombie0_Search,					DoRules_ARS_Zombie0_Search,					NULL,		CGroundCombat::ARS_ZOMBIE0_SEARCH,				0,	NULL),
/////////
	CFSMStateDef("ARS_CORROSIVE0_BASE",				InRules_ARS_Corrosive0_Base,				DoRules_ARS_Corrosive0_Base,				OutRules_ARS_Corrosive0_Base,		CGroundCombat::ARS_CORROSIVE0_BASE,				0,	NULL),
	CFSMStateDef("ARS_CORROSIVE0_INVESTIGATEMARK",	InRules_ARS_Corrosive0_InvestigateMark,		DoRules_ARS_Corrosive0_InvestigateMark,		NULL,		CGroundCombat::ARS_CORROSIVE0_INVESTIGATEMARK,	0,	NULL),
	CFSMStateDef("ARS_CORROSIVE0_SEARCH",			InRules_ARS_Corrosive0_Search,				DoRules_ARS_Corrosive0_Search,				NULL,		CGroundCombat::ARS_CORROSIVE0_SEARCH,			0,	NULL),
	CFSMStateDef("ARS_CORROSIVE0_ATTACKLOCATION",	InRules_ARS_Corrosive0_AttackLocation,		DoRules_ARS_Corrosive0_AttackLocation,		NULL,		CGroundCombat::ARS_CORROSIVE0_ATTACKLOCATION,	0,	NULL),
	CFSMStateDef("ARS_CORROSIVE0_FINGERFLICK",		InRules_ARS_Corrosive0_FingerFlick,			DoRules_ARS_Corrosive0_FingerFlick,			NULL,		CGroundCombat::ARS_CORROSIVE0_FINGERFLICK,		0,	NULL),
	CFSMStateDef("ARS_CORROSIVE0_PEERHERE",			InRules_ARS_Corrosive0_PeerHere,			DoRules_ARS_Corrosive0_PeerHere,			NULL,		CGroundCombat::ARS_CORROSIVE0_PEERHERE,			0,	NULL),
	CFSMStateDef("ARS_CORROSIVE0_GETENEMYOFFME",	InRules_ARS_Corrosive0_GetEnemyOffMe,		DoRules_ARS_Corrosive0_GetEnemyOffMe,		NULL,		CGroundCombat::ARS_CORROSIVE0_GETENEMYOFFME,	0,	NULL),
/////////
	CFSMStateDef("ZBOSS_SEARCH_RULE",				InitRule_ZombieBoss_Search,					RuleWork_ZombieBoss_Search,					NULL,		CGroundCombat::ARS_ZOMBIEBOSS_SEARCH,			0,	NULL),	
	CFSMStateDef("ZBOSS_GRAB_RULE",					InitRule_ZombieBoss_Grab,					RuleWork_ZombieBoss_Grab,					NULL,		CGroundCombat::ARS_ZOMBIEBOSS_GRAB,				0,	NULL),
	CFSMStateDef("ZBOSS_SMASH_RULE",				InitRule_ZombieBoss_Smash,					RuleWork_ZombieBoss_Smash,					NULL,		CGroundCombat::ARS_ZOMBIEBOSS_SMASH,			0,	NULL),
	CFSMStateDef("ZBOSS_FISH_RULE",					InitRule_ZombieBoss_Fish,					RuleWork_ZombieBoss_Fish,					NULL,		CGroundCombat::ARS_ZOMBIEBOSS_FISH,				0,	NULL),
	CFSMStateDef("ZBOSS_LURCH_RULE",				InitRule_ZombieBoss_Lurch,					RuleWork_ZombieBoss_Lurch,					NULL,		CGroundCombat::ARS_ZOMBIEBOSS_FISH,				0,	NULL),
	CFSMStateDef("ZBOSS_GOTO_RULE",					InitRule_ZombieBoss_Goto,					RuleWork_ZombieBoss_Goto,					NULL,		CGroundCombat::ARS_ZOMBIEBOSS_GOTO,				0,	NULL),
	


/////////
	CFSMStateDef("ARS_JUMPER0_BASE",				InRules_ARS_Jumper0_Base,					DoRules_ARS_Jumper0_Base,					NULL,		CGroundCombat::ARS_JUMPER0_BASE,				0,	NULL),
	CFSMStateDef("ARS_SCIENTIST0_BASE",				InRules_ARS_Scientist0_Base,				DoRules_ARS_Scientist0_Base,				NULL,		CGroundCombat::ARS_SCIENTIST0_BASE,				0,	NULL),
};												



CGroundCombat::CGroundCombat(void)
 : CAIThought()
{
	FASSERT(sizeof(s_aTacticFSMStateDefs) / sizeof(CFSMStateDef) == CGroundCombat::NUM_TACTICS);	
	FASSERT(sizeof(s_aRuleSetFSMStateDefs) / sizeof(CFSMStateDef) == CAIBrain::NUM_ATTACKRULESETS);
	FASSERT(sizeof(s_aRuleSetStateFSMStateDefs) / sizeof(CFSMStateDef) == CGroundCombat::ARS_NUM_TOTALSTATES);

	m_pLookAtMgr = NULL;
}


CGroundCombat::~CGroundCombat(void)
{
}

f32 _GetDistSqOffOfLOF(CAIBrain *pShooter, CAIBrain* pFriend, const CFVec3A& LineOfFire, f32 fDistToCheck)
{
	FASSERT(fDistToCheck > 0.0f);

	CFVec3A Tmp;
	CFVec3A FriendVec;
	FriendVec.Sub(pFriend->GetAIMover()->GetLoc(), pShooter->GetAIMover()->GetLoc());
	f32 fDot = FriendVec.Dot(LineOfFire);
	if (fDot > 0.0f && fDot < fDistToCheck)
	{
		Tmp = LineOfFire;
		Tmp.Mul(fDot);
		f32 fDistSqOffLOF = FriendVec.DistSq(Tmp);
		return fDistSqOffLOF;
	}
	return FMATH_MAX_FLOAT;
}


BOOL _IsFriendBlockingWay(CGroundCombat * pT)
{
	BOOL bBlocked = FALSE;
	CAIBrain* pShooterBrain = pT->GetBrain();
	CBot* pSelfBot = WD.pBot;

	if (pSelfBot && pShooterBrain)
	{
		CFVec3A LOF;
		f32 fDistToCheck;
		if (pT->m_pEnemy && pT->m_pEnemy->AIBrain())
		{
			LOF.Sub(pT->m_pEnemy->AIBrain()->GetAIMover()->GetLoc(), pShooterBrain->GetAIMover()->GetLoc());
			fDistToCheck = LOF.SafeUnitAndMag(LOF);
		}
		else
		{
			LOF = pSelfBot->MtxToWorld()->m_vFront;
			fDistToCheck = pSelfBot->m_fCollCylinderRadius_WS + 30.0f;
		}

		if (fDistToCheck > 1.0f)
		{
			CAIMemoryLarge* pSeen = NULL;
			if (pShooterBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_SEEN, (CAIMemorySmall**)&pSeen))
			{
				while (pSeen)
				{
					if (pSeen->m_pEntity &&
						pSeen->m_pEntity != pT->m_pEnemy &&		 //works even when m_pEnemy is NULL
						pSeen->m_pEntity->AIBrain() &&
						(pT->s_pWD->uNowTime - pSeen->m_uWhenTimeSecs) < 2 &&
						!(pSelfBot && pSeen->m_pEntity->TypeBits() & ENTITY_BIT_BOT &&	pSelfBot->GetCurMech() == pSeen->m_pEntity || ((CBot*)pSeen->m_pEntity)->GetCurMech() == pSelfBot) &&
						aiutils_IsFriendly(pShooterBrain->GetAIMover()->GetEntity(), pSeen->m_pEntity) &&
						_GetDistSqOffOfLOF(pShooterBrain, pSeen->m_pEntity->AIBrain(), LOF, fDistToCheck) < (pSeen->m_pEntity->AIBrain()->GetAIMover()->GetRadiusXZ()*pSeen->m_pEntity->AIBrain()->GetAIMover()->GetRadiusXZ()))
					{
						bBlocked = TRUE;
						break;
					}

					pSeen = (CAIMemoryLarge*) pShooterBrain->GetKnowledge().GetNextOldestOfId(pSeen);
				}
			}
		}
	}

	return bBlocked;
}

void CGroundCombat::ChangeEnemy(CEntity* pNewEnemy)
{
	//yikes, didn't want this here, but..
	if (pNewEnemy &&
		aiutils_IsMech(pNewEnemy) &&
		((CBot*) pNewEnemy)->IsPillBox())
	{
		pNewEnemy = ((CBotSiteWeapon*) pNewEnemy)->GetDriverBot();
	}

	m_pEnemy = pNewEnemy;
	m_pLastKnownEnemyVehicle = NULL;
	m_nPossessionPlayerIndexOfEnemy = -1;
	if (m_pEnemy)
	{
		if (m_pEnemy->AIBrain())
		{
			m_uEnemyRace = m_pEnemy->AIBrain()->GetRace();
		}

		BOOL bHasSeenEnemy = FALSE;
		if (m_pEnemy->TypeBits() & ENTITY_BIT_BOT)
		{
			m_nPossessionPlayerIndexOfEnemy = ((CBot*) m_pEnemy)->m_nPossessionPlayerIndex;

			if (m_pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, m_pEnemy))
			{
				m_pLastKnownEnemyVehicle = ((CBot*) m_pEnemy)->GetCurMech();
			}
		}


		//flags related to awareness of my enemy
		CFVec3A NewEnemyMark;
		if (m_pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, m_pEnemy))
		{
			m_uAttackFlags |= FLAG_HASEVERSEENENEMY;
			NewEnemyMark = m_pEnemy->MtxToWorld()->m_vPos;
		}
		else
		{
			m_uAttackFlags &= ~FLAG_HASEVERSEENENEMY;
			NewEnemyMark = m_pEnemy->MtxToWorld()->m_vPos;	   //findfix: potential cheating..... might want to fix this to less cheap
		}
		m_uAttackFlags &= ~FLAG_LASTKNOWNSAFEPOS_VALID;
		//vbls related to awareness of my enemy
		m_pLastKnownEnemyVehicle = NULL;
		m_LastAim = NewEnemyMark;
		m_fTimeWithoutLOS  = 0.0f;	 //since we're pretending we have seen our enemy, let's assume we just did.
		m_fTimeWithLOS = 0.0f;
		m_uLosStatusChangeCounter = 0;
		m_LastEnemyMark = NewEnemyMark;
		m_LastKnownVel = CFVec3A::m_Null;
		m_LastKnownSafePos = m_pBrain->GetLoc();
		m_EnemyMarkAtLastKnownSafeLoc = NewEnemyMark;
	}
}

extern void _SoundCollisionCBInit(void);
static BOOL _NotifyCommGroup_ActiveBrainCB(CAIBrain* pBrain, void* pData)
{
	CAISound *pRageSound = (CAISound*) pData;
	CAIBrain *pBrainOfRageSoundMaker = pRageSound->m_pEntity->AIBrain();
	if (pBrainOfRageSoundMaker != pBrain &&
		pBrain->GetCommGroupName() &&
		pBrainOfRageSoundMaker->GetCommGroupName() == pBrain->GetCommGroupName())	  //Note! these cchar*'s are from string table, so it is o.k. to compare ptrs
	{
		_SoundCollisionCBInit();
		CAIBrain::_SoundCollisionCB(pRageSound, pBrain);
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}

void CGroundCombat::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /* = NULL*/)
{
	s_pWD = NULL;
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	CAIMover* pMover = m_pBrain->GetAIMover();
	m_pEnemy = NULL;
	m_uEnemyRace = 0;
	u32 uTargetGUID = 0;
	s16 sEventId = 0;
	u32 i;
	ExtractAttackParams((CAIMemoryLarge*) pParamPack, &uTargetGUID,	&sEventId);
	ChangeEnemy(CEntity::Find(uTargetGUID));

	if (!m_pEnemy)
	{
		m_uThoughtFlags |= THOUGHTFLAG_USE_JOB_REACT_RULES;
	}
	FASSERT(!m_pLookAtMgr);


	//
	//   Range Tactic Vbls
	//
	m_fRangeMin = 10;
	m_fRangeMax = 50;
	m_uRangeTactic_EraptUseageCtrl = ERAPTUSEAGECTRL_NEVER;
	m_uRangePathFailureCount = 0;
	m_uRangeTacticSmartEraptDecisionTimeOut = 0;
	m_fEraptLosIsLostWaitTime = 1.0f;
	

	m_BadShotBonusUnit.Zero();
	m_uAttackFlags = FLAG_NOPATH;

	if (aiutils_CanEntityFly(pMover->GetEntity()))
	{
		m_uAttackFlags |= FLAG_3DBOT;
	}

	m_uLostEnemyTime = aiutils_GetCurTimeSecsU16();
	m_bPathJustCompleted = FALSE;
	if (!(GetBrain()->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLERAT))		//rats don't need look at mgrs
	{
		m_pLookAtMgr = (CGenericWait*) CGenericWait::BankAccess(NULL);
		if (m_pLookAtMgr)
		{
			m_pLookAtMgr->Init(pBrain, THOUGHTFLAG_NONE, CAIBrain::TT_INVALID);
			m_pLookAtMgr->m_uWaitFlags |= CGenericWait::WAIT_VISRAYS_ENABLED;
		}
	}
	m_LastAim.Zero();
	m_LastEnemyMark.Zero();
	m_LastKnownVel.Zero();
	m_pSearchTactic = NULL;
	m_pLockedPoi = NULL;
	m_uDodgeState = DODGESTATE_IN;
	m_uDodgePathFailureCount = 0;
	m_uDamageDodgeTimeOut = 0;
	m_uDamageDodgeTimeMin = 1;


	for (i = 0; i < NUM_PATH_REQUEST_SLOTS; i++)
	{
		m_anSearchReason[i] = SEARCHREASON_INVALID;
	}

	m_nCurPathRequestSlot = CGroundCombat::SEARCHREASON_INVALID;

	m_fUpdateBadShotBonusVecTime = 0.0f;
	m_fTimeWithoutLOS = 30.0f;	   //until we have seen our enemy, we will pretent we haven't seen it for 30 seconds
	m_fTimeWithLOS = 0.0f;
	m_uLosStatusChangeCounter = 0;
	m_fTimeWithTargetLock = 0.0f;
	m_fTimeWithoutTargetLock = 30.0f;
	m_uLastPeekABooSupriseTime = 0;
	m_uDamageSec = 0;
	m_uLastPeekABooBailOut = 0;
	m_fDamageDealtPerSec = 0.0f;
	m_fDamageTakenPerSec = 0.0f; //::attack_init
	m_uTacticInitTime = aiutils_GetCurTimeSecsU16();
	m_uRuleSetStateInitTime = aiutils_GetCurTimeSecsU16();

	m_uLastPanicOnDelayTimeOut = 0;
	m_uLastAlertOnDelayTimeOut = 0;
	m_uBlindResponseToDamageTimeOut = aiutils_GetCurTimeSecsU16() + 3;	//it will be at least 3 seconds before the enemy mount is set in response to damage while blind 
	m_uEnemyEvalTimeOut = aiutils_GetCurTimeSecsU16() + 1;
	m_uAttackAfterKillingEnemyTimeOut = 0;

	//
	//   Peekaboo data
	//
	m_uAttackFromCoverTimeOut = 0; //this means take cover asap!
	m_uAttackFromCoverTimeMin = 3;
	m_uAttackFromCoverTimeMax = 8;

	m_fAttackThoughtInitTime = aiutils_GetCurTimeSecs();

	//for doing an attack pt search  
	m_AttackPtSearch.ClearData();

	//for doing a cover pt search 
	m_CoverPtSearch.m_fCoverSearchAngle = 0.0f;
	m_CoverPtSearch.m_fCoverSearchRange	= 10.0f;
	m_CoverPtSearch.m_uCoverPtFailureCount = 0;

	//melee attack timing and radius
	m_uMeleeTimeOut = 0;
	m_pBrain->GetMeleeConfig(&m_uMeleeRad, &m_uMeleeTimeMin, &m_uMeleeTimeMax);


	//weapons
	m_uFireOddsTimeOut = 0;

	//
	//   standground tactic vbls
	//
	m_uDodgeTimeMin = 2;
	m_uDodgeTimeMax = 4;
	m_uDodgeOutTimeMin = 2;
	m_uDodgeOutTimeMax = 4;

	m_pAttackSpecsParamPack = NULL;
	m_pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_ATTACKSPECS,(CAIMemorySmall**) &m_pAttackSpecsParamPack);
	m_pSubThought = NULL;

	m_uTauntTimeMin = 5;
	m_uTauntTimeMax	= 10;
	m_uTauntTimeOut = 0;
	if (m_uTauntTimeMax > m_uTauntTimeMin)
	{
		m_uTauntTimeOut =  aiutils_GetCurTimeSecsU16() + (u16) (m_uTauntTimeMin + (u8) fmath_RandomChoice(m_uTauntTimeMax - m_uTauntTimeMin));
	}

	//Data that should be not part of this class, but stored in ARS specific parampacks
	m_uARS_0_flags = 0;


	m_AttackOrigin = m_pBrain->GetLoc();

//	if (m_pBrain->IsTalking())
//	{
//		ai_TalkModeEnd(m_pBrain);
//	}

	CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("ai"), AI_EVENTDATA1_ATTACK_INIT, (u32)(this->m_pBrain->GetAIMover()->GetEntity()), (u32) m_pEnemy);

	//Note: Ok to do this even without an enemy!
	AISoundHandle uRageHandle = AIEnviro_AddSound(m_pBrain->GetLoc(), (f32) m_pBrain->GetAttackNotifyRadius(), 5.0f, AISOUNDTYPE_RAGE, 0, m_pBrain->GetAIMover()->GetEntity());
	if (m_pBrain->GetCommGroupName())
	{
		CAISound* pAIRageSound = AIEnviro_SoundHandleToPtr(uRageHandle);
		if (pAIRageSound)
		{
			aibrainman_IterateActiveBrains(_NotifyCommGroup_ActiveBrainCB, pAIRageSound);
		}
	}

	AttackWorkBeginFrame();

	if (m_pEnemy && 
		!(	s_pWD->pSeenMem ||
			s_pWD->pHeardMem ||
			s_pWD->pDamagedByMem))
	{
		//how did I ever know to attack this guy?
		//must have been told do so, so I'll assume
		//that I know where he is!
		CFVec3A NewEnemyMark;
		NewEnemyMark = m_pEnemy->MtxToWorld()->m_vPos;	   //findfix: potential cheating..... might want to fix this to less cheap

		m_uAttackFlags &= ~FLAG_LASTKNOWNSAFEPOS_VALID;
		//vbls related to awareness of my enemy
		m_pLastKnownEnemyVehicle = NULL;
		m_LastAim = NewEnemyMark;
		m_fTimeWithoutLOS  = 0.0f;	 //since we're pretending we have seen our enemy, let's assume we just did.
		m_fTimeWithLOS = 0.0f;
		m_uLosStatusChangeCounter = 0;
		m_LastEnemyMark = NewEnemyMark;
		m_LastKnownVel = CFVec3A::m_Null;
		m_LastKnownSafePos = m_pBrain->GetLoc();
		m_EnemyMarkAtLastKnownSafeLoc = NewEnemyMark;
	}



	m_uAttackMoveSpeed = s_pWD->uAttackSpeedMin + (u8) fmath_RandomChoice(s_pWD->uAttackSpeedMax-s_pWD->uAttackSpeedMin);
	m_pBrain->PushBaseSpeedSetting(m_uAttackMoveSpeed);

	InitRuleSet();

	AttackWorkEndFrame();
}


void CGroundCombat::Cleanup(void)
{
	s_pWD = &WD;
	s_pWD->Init(this);

	CAIMover* pMover = m_pBrain->GetAIMover();
	CAIThought::Cleanup(); //stop bot and clear controls

	if (m_pLookAtMgr)
	{
		CGenericWait::BankAccess(m_pLookAtMgr);
		m_pLookAtMgr = NULL;
	}

	for (u32 i = 0; i < NUM_PATH_REQUEST_SLOTS; i++)
	{
		FreeRequestSlot(i);

		//will not leave mover following any path that combat order requested
	}

	ReleaseLockedPoi();

	m_TacticFSM.RemovePendingAndActiveStates(TRUE);
	m_RuleSetFSM.RemovePendingAndActiveStates(TRUE);
	m_RuleSetStateFSM.RemovePendingAndActiveStates(TRUE);

	m_pBrain->PopBaseSpeedSetting();
	
	//BTAPLAY
	if (m_uAttackFlags & FLAG_HASEVERSEENENEMY &&
		m_pBrain->GetAIMover()->GetEntity()->IsInWorld() &&
		!m_pBrain->GetAIMover()->GetEntity()->IsMarkedForWorldRemove() &&
		(m_pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) &&
		!(((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsDeadOrDying()) &&
		!m_pBrain->IsThoughtChangePending() &&
		m_pBrain->GetAIMover()->GetEntity()->NormHealth() > 0.05f &&
		m_pEnemy &&
		(m_pEnemy->TypeBits() & ENTITY_BIT_BOT) &&
		m_nPossessionPlayerIndexOfEnemy >=0 &&
		(!((CBot*)m_pEnemy)->IsInWorld() ||
		 ((CBot*)m_pEnemy)->IsMarkedForWorldRemove() ||
		 ((CBot*)m_pEnemy)->IsDeadOrDying()))
	{
		cchar* pszBTAName =NULL;
		if (m_uThoughtFlags & THOUGHTFLAG_GOAL_FAILED)
		{
			pszBTAName = "P*E_SRCH";
			CombatMode_PlayBTA(pszBTAName, TRUE, FALSE);
		}
		else 
		{
			CFVec3A tmp = CFVec3A::m_Null;
			if (m_pEnemy)
			{
				tmp = m_pEnemy->MtxToWorld()->m_vFront;
				if (m_pEnemy->TypeBits() & ENTITY_BIT_BOT)
				{
					tmp.Mul(((CBot*)m_pEnemy)->m_fCollCylinderRadius_WS);
				}
				else
				{
					tmp.Mul(5.0f);
				}
			}
			tmp.Add(this->m_LastEnemyMark);


			ai_AssignGoal_GotoWithLookAt(m_pBrain, tmp, this->m_LastEnemyMark, 0, 90, 5, 65, GOTOFLAG_VICT_BTA_AT_END);
		}
	}

	
	FASSERT(!m_pSearchTactic);
	FASSERT(!this->m_pSubThought);

	s_pWD = NULL;
}


void CGroundCombat::CombatMode_PlayBTA(cchar* pszBTAEvent, BOOL bStopFirst, BOOL bLookFirst)
{
	CAIBrain* pBrain = m_pBrain;

	if (!pBrain->IsTalking())
	{
 		cchar* pszBTAName = AIBTA_EventToBTAName(pszBTAEvent, pBrain);
		if (pszBTAName)
		{
			ai_TalkModeBegin(pBrain,
							AIBrain_TalkModeCB,
							(void*) pszBTAName,
							pBrain->GetAIMover()->GetEntity(),
							kuAttackMOdeTalkRequestTimeOutSecs,
							bStopFirst, //DON"T STOP
							bLookFirst, //DON"T LOOK
							0);
		}
	}
}


void CGroundCombat::ChangeTactic(FSMStateHandle uTactic)
{
	FASSERT(uTactic >= 0 && uTactic  < NUM_TACTICS);
	CGroundCombat* pThis = this;
	m_TacticFSM.ChangeState(s_aTacticFSMStateDefs[uTactic],
							&uTactic,
							NULL,
							(void**) &pThis);
}


void CGroundCombat::ChangeRuleSet(FSMStateHandle uRuleSet)
{
	FASSERT(uRuleSet >= 0 && uRuleSet  <CAIBrain::NUM_ATTACKRULESETS);
	CGroundCombat* pThis = this;
	m_uRuleSet = (u8) uRuleSet;
	m_RuleSetFSM.ChangeState(s_aRuleSetFSMStateDefs[uRuleSet],
							&uRuleSet,
							NULL,
							(void**) &pThis);
}


void CGroundCombat::ChangeRuleSetState(FSMStateHandle uRuleSetState)
{
	FASSERT(uRuleSetState >= 0 && uRuleSetState  < ARS_NUM_TOTALSTATES);
	CGroundCombat* pThis = this;
	m_RuleSetStateFSM.ChangeState(s_aRuleSetStateFSMStateDefs[uRuleSetState],
									&uRuleSetState,
									NULL,
									(void**) &pThis);
	m_uRuleSetStateInitTime = aiutils_GetCurTimeSecsU16();
}

const cchar* _kpszBadTacticLable = "NO_TACTIC";
cchar* CGroundCombat::GetTacticName(void)
{
	cchar* pszRVal = m_TacticFSM.GetActiveStateName();
	if (!pszRVal)
	{
		pszRVal = _kpszBadTacticLable;
	}
	return pszRVal;
}


const cchar* _kpszBadRuleSetLable = "NO_RULESET";
cchar* CGroundCombat::GetRuleSetName(void)
{
	cchar* pszRVal = m_RuleSetFSM.GetActiveStateName();
	if (!pszRVal)
	{
		pszRVal = _kpszBadRuleSetLable;
	}
	return pszRVal;
}


const cchar* _kpszBadRuleStateLable = "NO_RULESET_STATE";
cchar* CGroundCombat::GetRuleSetStateName(void)
{
	cchar* pszRVal = m_RuleSetStateFSM.GetActiveStateName();
	if (!pszRVal)
	{
		pszRVal = _kpszBadRuleStateLable;
	}
	return pszRVal;
}

//
//  udir 
//	 0=left
//   1=right
//   2=toward
//   3=away
//
BOOL CGroundCombat::FindCurRelPos_Ground(CFVec3A *pAttackPt,
										 const CFVec3A& EnemyPos,
										 const CFVec3A& CurPos,
										 f32 fLocalDist,
										 f32 fYOff,
										 u32 uLocalDir,
										 BOOL bCheckLOSToEnemy,
										 f32 fMinDistToEnemy,
										 f32 fMaxDistToEnemy)
{
	//Circle Strafe
	CFVec3A VecToEnemy;
	CFVec3A LocalDelta;
	LocalDelta.Zero();
	CFVec3A BumpPos;
	CFVec3A BumpPosAtGoal;
	f32 fDistToTargXZ;
	BOOL bHasLOS = FALSE;
	f32 fYBump = 3.0f; //findfix: configure this per bot (y offset of the los test, relative to pts passed in

	VecToEnemy.Sub(EnemyPos, CurPos);
	VecToEnemy.y = 0.0f;

	FASSERT(pAttackPt);

	fDistToTargXZ = VecToEnemy.SafeUnitAndMagXZ(VecToEnemy);
	if (fDistToTargXZ > 0.0f)
	{
		f32 fNewDistToTargXZ;
		if (uLocalDir == RELATIVE_DIR_LEFT || uLocalDir == RELATIVE_DIR_RIGHT)
		{
			//90 degree rotate about Y
			LocalDelta.z = -VecToEnemy.x;
			LocalDelta.x = VecToEnemy.z;
			LocalDelta.Mul(fLocalDist * (1.0f - (2*uLocalDir)));

			fNewDistToTargXZ = fDistToTargXZ;
		}
		else
		{	//dir = toward or away
			if (uLocalDir == RELATIVE_DIR_TOWARD)
			{
				fNewDistToTargXZ  = fDistToTargXZ - fLocalDist;
			}
			else if (uLocalDir == RELATIVE_DIR_AWAY)
			{
				fNewDistToTargXZ  = fDistToTargXZ + fLocalDist;
			}

			FMATH_CLAMPMIN(fLocalDist, 0.0f);
			LocalDelta = VecToEnemy;
			LocalDelta.Mul(fLocalDist * (1.0f - (2*(uLocalDir-2))));
		}

		if (fMaxDistToEnemy >= 0.0f && fMinDistToEnemy >= 0.0f)
		{
			f32 fAdjustDist = 0.0f;
			if (fNewDistToTargXZ < fMinDistToEnemy)
			{ //move back
				fAdjustDist = fMinDistToEnemy-fNewDistToTargXZ;
			}
			if (fNewDistToTargXZ > fMaxDistToEnemy)
			{
				fAdjustDist = fNewDistToTargXZ-fMaxDistToEnemy;
			}
			VecToEnemy.Mul(fAdjustDist);
			LocalDelta.Add(VecToEnemy);
		}

		pAttackPt->Add(LocalDelta, CurPos);
		pAttackPt->y += fYBump;

		if (bCheckLOSToEnemy)
		{
			BumpPos = *pAttackPt;
			BumpPosAtGoal = EnemyPos;
			BumpPosAtGoal.y += fYBump;
			bHasLOS = !aiutils_IsLOSObstructed_IgnoreBots( BumpPos, BumpPosAtGoal, s_pWD->pBot);
			aiutils_DebugTrackRay(BumpPos, BumpPosAtGoal, bHasLOS);
			return bHasLOS;
		}
		return TRUE;
	}

	return FALSE;
}


//
//  udir 
//	 0=left
//   1=right
//   2=toward
//   3=away

BOOL CGroundCombat::FindEnemyRelPos_Ground(CFVec3A *pAttackPt,
										   const CFVec3A& EnemyPos,
										   const CFVec3A& CurPos,
										   f32 fRelDist,
										   f32 fYOff,
										   u32 uRelDir,
										   BOOL bCheckLOSToEnemy)
{
	CFVec3A VecToEnemy;
	CFVec3A BumpPos;
	CFVec3A BumpPosAtGoal;
	f32 fTmp;
	f32 fDistToTarg;
	BOOL bHasLOS = FALSE;
	f32 fYBump = 3.0f; //findfix: configure this per bot (y offset of the los test, relative to pts passed in
	f32 fMinAttackPosDistToTarg = 3.0f;	 //findfix: configure this per bot

	VecToEnemy.Sub(EnemyPos, CurPos);
	VecToEnemy.y = 0.0f;

	FASSERT(pAttackPt);

	fDistToTarg = VecToEnemy.SafeUnitAndMagXZ(VecToEnemy);
	if (fDistToTarg > 0.0f)
	{
		if (fRelDist < fMinAttackPosDistToTarg)
		{
			fRelDist = fMinAttackPosDistToTarg;
		}

		if (uRelDir == RELATIVE_DIR_LEFT || uRelDir == RELATIVE_DIR_RIGHT)
		{
			//2D rotate
			fTmp = VecToEnemy.z;
			VecToEnemy.z = -VecToEnemy.x;
			VecToEnemy.x = fTmp;
			
			VecToEnemy.Mul(fRelDist * (1.0f - (2*uRelDir)));
		}
		else
		{	 //toward / away
			VecToEnemy.Mul(fRelDist * (1.0f - (2*(RELATIVE_DIR_AWAY-uRelDir))));
		}
		pAttackPt->Add(VecToEnemy, EnemyPos);
		pAttackPt->y += fYBump;

		if (bCheckLOSToEnemy)
		{
			BumpPos = *pAttackPt;
			BumpPosAtGoal = EnemyPos;
			BumpPosAtGoal.y += fYBump;
			bHasLOS = !aiutils_IsLOSObstructed_IgnoreBots( BumpPos, BumpPosAtGoal, s_pWD->pEnemyBot);
			aiutils_DebugTrackRay(BumpPos, BumpPosAtGoal, bHasLOS);
			return bHasLOS;
		}
		return TRUE;
	}

	return FALSE;
}


BOOL CGroundCombat::FindCoverPos_Ground(CFVec3A *pResult,
										const CFVec3A& EnemyPos,
										const CFVec3A& CurPos,
										f32 fXZRot,
										f32 fYOff,
										f32 fTestDist)
{
	CFVec3A TestPos;
	CFVec3A VecToEnemy;
	f32 fDistToEnemy;
	CFVec3A vStart, vEnd;
	BOOL bLOS = TRUE;
	u32 uCount = 0;
	f32 fLOSHeightBump = m_pBrain->GetAIMover()->GetHeight()*0.6f;

	VecToEnemy.Sub(EnemyPos, CurPos);
	VecToEnemy.y = 0.0f;

	fDistToEnemy = VecToEnemy.SafeUnitAndMagXZ(VecToEnemy);
	if (fDistToEnemy > 0.0f)
	{
		while (bLOS && uCount < 2)
		{
			uCount++;

			fXZRot *= (1.0f - (f32)uCount*2.0f);		  //check both pos and neg rotations of the cover vec in one function call
			TestPos = VecToEnemy;
			TestPos.RotateY(fXZRot);
			TestPos.Mul(fTestDist);
			TestPos.Add(CurPos);
			f32 fFloor = 0.0f;
			if (aiutils_FloorHeightAt(TestPos, &fFloor, (fTestDist > 10.0f) ? 10.0f : fTestDist))
			{

				CGraphPoi* pNearbyPoi = aimain_pAIGraph->FindClosestPoi(TestPos, 10.0f, 0);
				if (pNearbyPoi && !pNearbyPoi->IsLocVisible(aimain_pAIGraph, EnemyPos))
				{
				   //found a nearby poi that looks like it would make good cover, adjust the TestPos
					pNearbyPoi->GetLocation(aimain_pAIGraph, &TestPos);
				}

				vEnd.Set( TestPos );
				vEnd.y += fLOSHeightBump;
				vStart.Set( EnemyPos );
				vStart.y += fLOSHeightBump;
				bLOS = !aiutils_IsLOSObstructed_IgnoreBots(vStart,  vEnd, s_pWD->pEnemyBot);
    			aiutils_DebugTrackRay(vStart, vEnd, bLOS);
				if (!bLOS && pResult)
				{
					*pResult = TestPos;
				}
			}
		}
		return !bLOS;
	}
	return FALSE;
}


BOOL CGroundCombat::FindRetreatPos_Ground(CFVec3A *pResult,
										  const CFVec3A& From,
										  const CFVec3A& LookAt,
										  f32 fDist,
										  f32 fYOff)
{
	pResult->Set(LookAt);
	pResult->Mul(-fDist);
	pResult->Add(From);
	f32 fFloor = 0.0f;
	if (aiutils_FloorHeightAt(*pResult, &fFloor, fDist))
	{
		(*pResult).y = fFloor;
		return TRUE;
	}
	return FALSE;
}


BOOL CGroundCombat::FindAdvancePos_Ground(CFVec3A *pResult,
										  const CFVec3A& From,
										  const CFVec3A& LookAt,
										  f32 fDist,
										  f32 fYOff)
{
	pResult->Set(LookAt);
	pResult->Mul(fDist);
	pResult->Add(From);
	f32 fFloor = 0.0f;
	if (aiutils_FloorHeightAt(*pResult, &fFloor, fDist))
	{
		(*pResult).y = fFloor;
		return TRUE;
	}
	return FALSE;
}


BOOL CGroundCombat::FindRangeAcquirePos_Ground(CFVec3A *pResult, 
											   const CFVec3A& EnemyPos,
											   f32 fYOff)
{
	*pResult = EnemyPos;
	return TRUE;
}


BOOL CGroundCombat::FindLocalPoi_Ground(CFVec3A *pResult, CGraphPoi** pPoiToLockIfUsed, const CFVec3A& EnemyTagPos, const CFVec3A& CurPos,  f32 fMinRad, f32 fMaxRad, f32 fYOff, BOOL bCheckLOSToEnemy)
{																																									  
	BOOL bHasLOS = FALSE;
	CGraphPoi* pPoi = aimain_pAIGraph->FindClosestPoi(CurPos, fMaxRad, 0, CGraphPoi::POIFLAG_OFFENSIVE_HINT); //won't return locked poi
	if (pPoi)
	{
		if (pPoi->IsLocVisible(aimain_pAIGraph, EnemyTagPos))
		{
			pPoi->GetLocation(aimain_pAIGraph, pResult);
			if (bCheckLOSToEnemy)
			{
				CFVec3A BumpPos;
				f32 fLOSHeightBump = m_pBrain->GetAIMover()->GetHeight()*0.6f;
				BumpPos = *pResult;
				BumpPos.y += fLOSHeightBump;

				bHasLOS = !aiutils_IsLOSObstructed_IgnoreBots( BumpPos, EnemyTagPos, s_pWD->pEnemyBot);
			}
		}
		else
		{
			//nearby point, but not good angle, so pass
			int i= 2;
		}
	}

	if (pPoiToLockIfUsed)
	{
		*pPoiToLockIfUsed = pPoi;
	}
	
	return (pPoi && !bCheckLOSToEnemy) || bHasLOS;
}



//
// class CAttackPtSearch
//

void CAttackPtSearch::ClearData(void)
{
	m_uRepeatTimeMin = 0;
	m_uRepeatTimeMax = 0;
	m_nDirInc = 0;
	m_uDirMin = 0;
	m_uDirMax = 0;
	m_nRangeXZInc = 0;
	m_uRangeXZMax = 0;
	m_uRangeXZMin = 0;
	m_nRangeYInc = 0;
	m_uRangeYMax = 0;
	m_uRangeYMin = 0;
	m_uSearchType = 0;

	m_uRepeatTimeOut = 0;
	m_nCurRangeXZ = 0;
	m_nCurRangeY = 0;
	m_nCurDir = 0;
	m_uFailureCount = 0;
	m_uSucceedCount = 0;
}

void CAttackPtSearch::SetAttackPtSearchParams(	u8 uRepeatTimeMin,
											u8 uRepeatTimeMax,
											s8 nDirInc,
											u8 uDirMin,
											u8 uDirMax,
											s8 nRangeXZInc,
											u8 uRangeXZMin,
											u8 uRangeXZMax,
											s8 nRangeYInc,
											u8 uRangeYMin,
											u8 uRangeYMax)
{
	m_uRepeatTimeMin = uRepeatTimeMin;
	m_uRepeatTimeMax = uRepeatTimeMax;
	m_nDirInc = nDirInc;
	m_uDirMin = uDirMin;
	m_uDirMax = uDirMax;
	m_nRangeXZInc = nRangeXZInc;
	m_uRangeXZMax = uRangeXZMax;
	m_uRangeXZMin = uRangeXZMin;
	m_nRangeYInc = nRangeYInc;
	m_uRangeYMax = uRangeYMax;
	m_uRangeYMin = uRangeYMin;
}


BOOL CAttackPtSearch::IsTimeForNewSearch(void)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	//m_uRepeatTimeMax < m_uRepeatTimeMin) means NEVER pick a new attack loc
	return ((m_uRepeatTimeMax >= m_uRepeatTimeMin) && (_pWD->uNowTime > m_uRepeatTimeOut)); 
}


void CAttackPtSearch::InitLocal(void)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	f32 fRange;
	m_nCurDir = 0;
	m_uFailureCount = 0;
	m_uSucceedCount = 0;

	m_uSearchType = fmath_RandomChoice(CAttackPtSearch::NUM_ATTACKPT_SEARCHTYPES);
	if (m_uSearchType == CAttackPtSearch::ATTACKPT_SEARCHTYPE_LOCAL_HOPTO)
	{
		fRange = ((CBot*) _pWD->pMover->GetEntity())->GetApproxHopDist();
		FMATH_CLAMP(fRange, 0.0f, 32767.0f);
		m_nCurRangeXZ = (s16) fRange;
	}
	else if (m_uSearchType == CAttackPtSearch::ATTACKPT_SEARCHTYPE_LOCAL_ROLLTO)
	{
		fRange = ((CBot*) _pWD->pMover->GetEntity())->GetApproxRollDist();
		FMATH_CLAMP(fRange, 0.0f, 32767.0f);
		m_nCurRangeXZ = (s16) fRange;
	}
	else
	{
		m_nCurRangeXZ = _kuDefaultLocalAttackSearchRange;
	}

	m_uRepeatTimeOut = aiutils_GetTimeOutRandMinMax(m_uRepeatTimeMin, m_uRepeatTimeMax);
}


void CAttackPtSearch::AdvanceLocal(BOOL bLastSearchSuccessful)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;

	if (bLastSearchSuccessful)
	{
		InitLocal();
	}
	else
	{
		m_uFailureCount++;
		m_nCurDir++;
		
		if (m_uSearchType == CAttackPtSearch::ATTACKPT_SEARCHTYPE_LOCAL_HOPTO ||
			m_uSearchType == CAttackPtSearch::ATTACKPT_SEARCHTYPE_LOCAL_ROLLTO)
		{
			while (m_nCurDir == CGroundCombat::RELATIVE_DIR_TOWARD ||
					m_nCurDir == CGroundCombat::RELATIVE_DIR_AWAY)
			{
				m_nCurDir++;
			}
		}

		if (m_nCurDir >= CGroundCombat::NUM_RELATIVE_DIRS)
		{
			m_nCurDir = 0;

			if (m_uSearchType == CAttackPtSearch::ATTACKPT_SEARCHTYPE_LOCAL_HOPTO ||
				m_uSearchType == CAttackPtSearch::ATTACKPT_SEARCHTYPE_LOCAL_ROLLTO)
			{
			   	m_nCurRangeXZ = _kuDefaultLocalAttackSearchRange;
			}
			else
			{
				m_nCurRangeXZ += _kuDefaultLocalAttackSearchRangeInc;
			}
		}
	}
}


void CAttackPtSearch::InitEnemyRel(void)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	if (m_nDirInc > 0)
	{
		m_nCurDir = m_uDirMin;
	}
	else
	{
		m_nCurDir = m_uDirMax;
	}
	
	if (m_nRangeXZInc > 0)
	{
		m_nCurRangeXZ = m_uRangeXZMin;
	}
	else
	{
		m_nCurRangeXZ = m_uRangeXZMax;
	}
	if (m_nRangeYInc > 0)
	{
		m_nCurRangeY = m_uRangeYMin;
	}
	else
	{
		m_nCurRangeY = m_uRangeYMax;
	}
	m_uFailureCount = 0;
	m_uSucceedCount = 0;

	m_uRepeatTimeOut = aiutils_GetTimeOutRandMinMax(m_uRepeatTimeMin, m_uRepeatTimeMax);
}


void CAttackPtSearch::AdvanceEnemyRel(BOOL bLastSearchSuccessful)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	if (bLastSearchSuccessful)
	{
		m_nCurDir = (u8) fmath_RandomChoice(CGroundCombat::NUM_RELATIVE_DIRS);
		if (m_nRangeXZInc > 0)
		{
			m_nCurRangeXZ = m_uRangeXZMin;
		}
		else
		{
			m_nCurRangeXZ = m_uRangeXZMax;
		}
		if (m_nRangeYInc > 0)
		{
			m_nCurRangeY = m_uRangeYMin;
		}
		else
		{
			m_nCurRangeY = m_uRangeYMax;
		}
		m_uFailureCount = 0;
		m_uSucceedCount++;

		//reset the new enemy-relative-attack-pt selection timer
		m_uRepeatTimeOut = aiutils_GetTimeOutRandMinMax(m_uRepeatTimeMin, m_uRepeatTimeMax);
	}
	else
	{
		m_uFailureCount++;
		m_nCurDir += m_nDirInc;

		if (m_nDirInc > 0 && m_nCurDir > m_uDirMax)
		{
			m_nCurDir = m_uDirMin;
			m_nCurRangeXZ += m_nRangeXZInc;
		}
		else if (m_nDirInc < 0 && m_nCurDir < m_uDirMin)
		{
			m_nCurDir = m_uDirMax;
			m_nCurRangeXZ += m_nRangeXZInc;
		}

		if (m_nRangeXZInc > 0 && m_nCurRangeXZ > m_uRangeXZMax)
		{
			m_nCurRangeXZ = m_uRangeXZMin;
			m_nCurRangeY += m_nRangeYInc;
		}
		else if (m_nRangeXZInc < 0 && m_nCurRangeXZ < m_uRangeXZMin)
		{
			m_nCurRangeXZ = m_uRangeXZMax;
			m_nCurRangeY += m_nRangeYInc;
		}

		if (m_nRangeYInc > 0 && m_nCurRangeY > m_uRangeYMax)
		{
			m_nCurRangeY = m_uRangeYMin;
		}
		else if (m_nRangeYInc < 0 && m_nCurRangeY < m_uRangeYMin)
		{
			m_nCurRangeY = m_uRangeYMax;
		}
	}
}

f32 _kfEnemyRelAttackPtSearchFudgeDist[2] = {5.0f, 100.0f};
void _EnemyRelAttackPtWork(CGroundCombat* pT, BOOL bOkToRequestEnemyRelAttackPtPaths, BOOL bAutoSuccess, BOOL *pbNewPtReached /*= NULL*/)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	u8 uAttackPtSlot =0;
	CFVec3A AttackPt;
	if (pbNewPtReached)
	{
		*pbNewPtReached = FALSE;
	}

	//if
	//		Client has o.k'd new attack pts.
	//      Enough time has passed since last successfull attack pt change && 
	//		No Path ready to a new attack pt
	//      
	//then
	//
	//		try to find a new Enemy Relative attack pt
	//
	if (bOkToRequestEnemyRelAttackPtPaths &&
		pT->m_AttackPtSearch.IsTimeForNewSearch() &&
		pT->m_pEnemy &&
		!pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS))
	{
		if ((pT->*(_pWD->pFindEnemyRelPos_Func))(	&AttackPt,
												_pWD->EnemyMarkTagPos,
												_pWD->pMover->GetLoc(),
												pT->m_AttackPtSearch.m_nCurRangeXZ,
												pT->m_AttackPtSearch.m_nCurRangeY,
												pT->m_AttackPtSearch.m_nCurDir%CGroundCombat::NUM_RELATIVE_DIRS,
												CHECK_LOS_TO_ENEMY_MARK) &&
			AttackPt.DistSqXZ(_pWD->pMover->GetLoc()) > 10.0f*10.0f)
		{
			pT->RequestSearch(AttackPt, CGroundCombat::SEARCHREASON_NEWATTACKPOS, _kfEnemyRelAttackPtSearchFudgeDist[!!(pT->m_uAttackFlags & CGroundCombat::FLAG_3DBOT)]);
 			if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
			{
				//special path that should avoid the mark en-route to new attack point
				pT->m_aSearchQuery[uAttackPtSlot].SetAvoidPt(_pWD->EnemyMark, 10.5f);
				pT->m_aSearchQuery[uAttackPtSlot].SetParams(pT->m_aSearchQuery[uAttackPtSlot].GetParams() | CSearchQuery::SEARCHPARAM_AVOIDPT_PATH);
			}
		}
		else
		{
			pT->m_AttackPtSearch.AdvanceEnemyRel(CAttackPtSearch::BOOL_LAST_SEARCH_FAILED);
		}
	}

	//if
	//		an attack pt path fails || 
	//      bot is stuck
	//then
	//		reset clear the search slot
	//		reset the new attack pt selection vbls
	//
	if (pT->IsSearchFailed(CGroundCombat::SEARCHREASON_NEWATTACKPOS) ||
		(_pWD->pMover->IsStuck(2.2f))
		)
	{
		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			pT->FreeRequestSlot(uAttackPtSlot);
		}

		pT->m_AttackPtSearch.AdvanceEnemyRel(CAttackPtSearch::BOOL_LAST_SEARCH_FAILED);
	}

	//if    
	//		just completed an attackpt path
	//then
	//      reset the attack path frequency timers and such
	//
	if (pT->m_bPathJustCompleted &&
		pT->m_nLastPathReason == CGroundCombat::SEARCHREASON_NEWATTACKPOS)
	{
		if (pbNewPtReached)
		{
			*pbNewPtReached = TRUE;
		}

//pgm: note! this can't always be called inside this func. Must be called from client since 
//      there are several conditions (los to player for example) for when a search can be considered a success
		if (bAutoSuccess)
		{
			pT->m_AttackPtSearch.AdvanceEnemyRel(CAttackPtSearch::BOOL_LAST_SEARCH_SUCCEEDED);
		}
		else
		{
			pT->m_AttackPtSearch.AdvanceEnemyRel(CAttackPtSearch::BOOL_LAST_SEARCH_FAILED);
		}

		if (pT->FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			pT->FreeRequestSlot(uAttackPtSlot);
		}
	}

	//if
	//		bOkToRequestEnemyRelAttackPtPaths
	//		an attack pt path is ready && 
	//      not currently following attack pt
	//then
	//		follow the new tc path

	if (!pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS))
	{
		if (pT->IsSearchDone(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			if (bOkToRequestEnemyRelAttackPtPaths )
			{
				pT->Attack_AssignPath(uAttackPtSlot);
			}
			else
			{
				pT->FreeRequestSlot(uAttackPtSlot);
			}
		}
	}
}


void _InitCoverPtSearch(CCoverPtSearch* pS, BOOL bPanic)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	pS->m_uCoverPtFailureCount = 0;
	pS->m_fCoverSearchAngle = FMATH_HALF_PI;

	if (bPanic)
	{
		pS->m_fCoverSearchRange = 60.0f;	 //when in panic, farther distances are checked first, and range is decreased while scanning
	}
	else
	{
		pS->m_fCoverSearchRange = 10.0f;
	}
}


void _AdvanceCoverPtSearch(CCoverPtSearch* pS, BOOL bLastSearchSuccessful, BOOL bPanic)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	if (bLastSearchSuccessful)
	{
		pS->m_uCoverPtFailureCount = 0;

		if (bPanic)
		{
			pS->m_fCoverSearchRange = 60.0f;	 //when in panic, farther distances are checked first, and range is decreased while scanning
		}
		else
		{
			pS->m_fCoverSearchRange = 10.0f;
		}
	
	}
	else
	{
		pS->m_uCoverPtFailureCount++;
		f32 fCoverSearchAngleStep = FMATH_HALF_PI*0.25f;
		if (pS->m_fCoverSearchAngle + fCoverSearchAngleStep > FMATH_PI)		  //sweeping from half-pi to pi, because cover pt search funcs test both sides of arc every frame
		{
			pS->m_fCoverSearchAngle = FMATH_HALF_PI;

			if (bPanic)
			{
			   pS->m_fCoverSearchRange -= 5.0f;
			}
			else
			{
			   pS->m_fCoverSearchRange += 5.0f;
			}
		}
		else
		{
		   pS->m_fCoverSearchAngle += fCoverSearchAngleStep;
		}
		
		if (pS->m_fCoverSearchRange > 75.0f)
		{
			pS->m_fCoverSearchRange = 5.0f;
		}

		if (pS->m_fCoverSearchRange < 0.0f)
		{
			pS->m_fCoverSearchRange = 70.0f;
		}
	}
}




BOOL CGroundCombat::IsLastSafePosUsefull(const CFVec3A& EnemyPos)
{
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	
	if (m_EnemyMarkAtLastKnownSafeLoc.DistSq(EnemyPos) < 3.0f*3.0f)
	{
		return TRUE;
	}

	return FALSE;
}


//
// Where is Enemy
//	  - the whole point of this, is to
//      prioritize the current stimuli and
//		if possible, return the mount position of where we currently believe
//      the player is
BOOL CGroundCombat::WhereIsEnemy(CAIMemoryLarge* pSeenMem,
								 CAIMemoryLarge* pHeardMem,
								 CAIMemoryLarge* pDamagedByMem,
								 CFVec3A* pEnemyMountPos,
								 BOOL *pbHasLOSThisFrame)
{
	u16 uNowTime = aiutils_GetCurTimeSecsU16();
	BOOL bEnemyMountPosSet = TRUE;
	CAIMover* pMover = m_pBrain->GetAIMover();

	FASSERT(pEnemyMountPos && pbHasLOSThisFrame);
	*pbHasLOSThisFrame = FALSE;

	if (pSeenMem)
	{
		u16 uTimeSinceSeen = uNowTime - pSeenMem->m_uWhenTimeSecs;

		//if this is a new memory, we have los right now
		if (pSeenMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
		{	//WE've got LOS with the target.
			*pbHasLOSThisFrame = TRUE;

			m_pLastKnownEnemyVehicle = NULL;
			aiutils_GetCurMech(m_pEnemy, &m_pLastKnownEnemyVehicle);
			if (m_pLastKnownEnemyVehicle && ((CBot*)m_pLastKnownEnemyVehicle)->IsPillBox())
			{   //don't target the vehicle if it is a pillbox
				m_pLastKnownEnemyVehicle = NULL;
			}
		}
		
		if (m_pLastKnownEnemyVehicle)
		{   //If I know the player is in a vehicle, then set the mountpos to be the vehicle pos
			pEnemyMountPos->Set(m_pLastKnownEnemyVehicle->MtxToWorld()->m_vPos);
		}
		else
		{
			if (uTimeSinceSeen  < 3)  //findfix: configure this number of seconds before prediction begins, or set it dynamically, I guess
			{
				pEnemyMountPos->Set(pSeenMem->m_EntityLoc);  //enemy position is where I last saw him!

				if (!(pSeenMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY))
				{
					if (pHeardMem && (pHeardMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY))
					{
						pEnemyMountPos->Set(pHeardMem->m_EntityLoc);  //enemy position is where I last saw him!
					}
					else if (m_fTimeWithoutLOS < 3.0f)	//safety
					{
						//code for taking a guess where he might be?
						f32 fExtrapolatePosScale = 0.25f;
						pEnemyMountPos->Set(m_LastKnownVel);
						pEnemyMountPos->Mul(m_fTimeWithoutLOS);		 //longer it has been, farther we extrapolate the last know velocity
						pEnemyMountPos->Mul(fExtrapolatePosScale);
						pEnemyMountPos->v3 += pSeenMem->m_EntityLoc;
					}
				}
				else if (	(pSeenMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY) &&
							(m_pEnemy && m_pEnemy->TypeBits() & ENTITY_BIT_BOT) &&
							FLoop_fPreviousLoopSecs > 0.0f)
				{
					m_LastKnownVel.Sub(((CBot*) m_pEnemy)->MtxToWorld()->m_vPos, ((CBot*) m_pEnemy)->m_MountPrevPos_WS);
					m_LastKnownVel.Mul(FLoop_fPreviousLoopOOSecs);
				}
			}
			else
			{
				if (pDamagedByMem && pSeenMem->m_uWhenTimeSecs < pDamagedByMem->m_uWhenTimeSecs)
				{  //just got hit by something, pretend enemy pos is in the direction of where I was hit
					pEnemyMountPos->Set(pDamagedByMem->m_EntityLoc);
				}
				else if (pHeardMem && pSeenMem->m_uWhenTimeSecs < pHeardMem->m_uWhenTimeSecs)
				{	//just heard something, pretend enemy pos is in the direction of where I was hit
					pEnemyMountPos->Set(pHeardMem->m_EntityLoc);
				}
				else
				{	//best guess is that enemy position is where I last saw him!
					pEnemyMountPos->Set(pSeenMem->m_EntityLoc);  
				}
			}

			if (!(pSeenMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY))
			{	//We don't have los with the target
				m_LastKnownSafePos = pMover->GetLoc();
				m_LastKnownSafePos.y+=1.0f;
				m_EnemyMarkAtLastKnownSafeLoc.Set(*pEnemyMountPos);
				m_EnemyMarkAtLastKnownSafeLoc.y+=1.0f;
			}
		}
	}
	else if (	pHeardMem &&
				s_pWD->uNowTime - pHeardMem->m_uWhenTimeSecs <2 )
	{
		pEnemyMountPos->Set(pHeardMem->m_EntityLoc);

		if (!s_pWD->pMover->CanSee())
		{
			*pbHasLOSThisFrame = TRUE; //run off of ears
		}
	}
	else if (	pDamagedByMem &&
				(s_pWD->uNowTime - pDamagedByMem->m_uWhenTimeSecs < 1 ) &&
				pDamagedByMem->m_uWhenTimeSecs > m_uBlindResponseToDamageTimeOut)
	{
		pEnemyMountPos->Set(pDamagedByMem->m_EntityLoc);
		m_uBlindResponseToDamageTimeOut = s_pWD->uNowTime + 3;

		if (!s_pWD->pMover->CanSee())
		{
			*pbHasLOSThisFrame = TRUE; //run off of ears
		}

	}
	else if (m_pEnemy && !(m_pEnemy->TypeBits() & ENTITY_BIT_BOT))
	{
		//cheat when attacking non-bots
		*pEnemyMountPos = m_pEnemy->MtxToWorld()->m_vPos;
		*pbHasLOSThisFrame = TRUE; //run off of ears
		//no Idea where the enemy is
		bEnemyMountPosSet = TRUE;
	}
	else if( m_pEnemy &&
			 pMover->m_pEntity->GetParent() && 
			 pMover->m_pEntity->GetParent()->TypeBits() & ENTITY_BIT_SITEWEAPON &&
			 ((CBot*)pMover->m_pEntity->GetParent())->m_pBotDef->m_nSubClass  == BOTSUBCLASS_SITEWEAPON_RATGUN )
	{
		*pEnemyMountPos = m_pEnemy->MtxToWorld()->m_vPos;
		*pbHasLOSThisFrame = TRUE;
		bEnemyMountPosSet = TRUE;
	}
	else
	{
		*pEnemyMountPos = this->m_LastEnemyMark;
		//no Idea where the enemy is
		bEnemyMountPosSet = FALSE;
	}


	return bEnemyMountPosSet;
}


void CGroundCombat::ResetCommonTacticVbls(void)
{
	m_TargPosAtLastTacticReset = s_pWD->EnemyMark;
	m_TacticOrigin = s_pWD->pMover->GetLoc();
	m_uTacticInitTime = s_pWD->uNowTime;
}


f32 CGroundCombat::GetEnemyMoveDistSqThisTactic(void)
{
	if (s_pWD)
	{
		return s_pWD->EnemyMark.DistSq(m_TargPosAtLastTacticReset);
	}
	return 0.0f;
}


f32 CGroundCombat::GetMoveDistSqThisTactic(void)
{
	if (s_pWD)
	{
		return s_pWD->pMover->GetLoc().DistSq(m_TacticOrigin);
	}
	return 0.0f;
}


void CGroundCombat::Attack_AssignPath(u8 uSlot, u32 uAssignPathFlags /*= ASSIGNPATHFLAG_NONE*/)
{
	if (m_uAttackFlags & FLAG_FOLLOW_PATHS_WITH_PRECISION)
	{
		uAssignPathFlags |= CAIMover::ASSIGNPATHFLAG_PRECISION_GOAL;
	}
	s_pWD->pMover->AssignPath(m_aSearchResults[uSlot].GetAIPath(), uAssignPathFlags);
	m_nCurPathRequestSlot = uSlot;
	m_uAttackFlags &= ~FLAG_NOPATH;
}


void CGroundCombat::ResetDodge(void)
{
	u8 uDodgeOutSlot = 0;
	u8 uDodgeInSlot = 0;
	m_uDodgeState = DODGESTATE_IN;
	m_DodgeInLoc = m_pBrain->GetAIMover()->GetLoc();

	if (s_pWD->uNowTime >= m_uDodgeTimeOut)
	{
		m_uDodgeTimeOut = aiutils_GetTimeOutRandMinMax(m_uDodgeTimeMin, m_uDodgeTimeMax);
	}

	if (s_pWD->uNowTime >= m_uDodgeOutTimeOut)
	{
		m_uDodgeOutTimeOut = aiutils_GetTimeOutRandMinMax(m_uDodgeOutTimeMin, m_uDodgeOutTimeMax);
	}


	if (FindRequestSlotInUse(SEARCHREASON_DODGEOUT, &uDodgeOutSlot))
	{
		FreeRequestSlot(uDodgeOutSlot);
	}
	if (FindRequestSlotInUse(SEARCHREASON_DODGEIN, &uDodgeInSlot))
	{
		FreeRequestSlot(uDodgeInSlot);
	}

}


void CGroundCombat::DoDodgeWork(BOOL bOkToRequestDodgePaths)
{
	u8 uDodgeOutSlot = 0;
	u8 uDodgeInSlot = 0;
	BOOL bDodgeTimeOutOverride = FALSE;
	BOOL bDodgeDamage = FALSE;

	//if
	//		taken damage recently &&
	//      enough time has passed since last damage dodge
	//then
	//      force a dodge
	if (s_pWD->pDamagedByMem &&
		((s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs) < 1))
	{
		bDodgeTimeOutOverride = (m_uDamageDodgeTimeOut < s_pWD->uNowTime);
		bDodgeDamage = TRUE;   //remember to set the m_uDamageDodgeTimeOut if this override actually causes a dodge path to be taken
	} 

	if (!bDodgeTimeOutOverride &&
		m_uDodgeTimeMax < m_uDodgeTimeMin)	 //Idle Dodges Disabled
	{
		return; 
	}

	//if
	//		player is in my personal space, and I can see him ||
	//		the whole attack state is really new
	//then
	//		start a dodge 
	//
	if (!bDodgeTimeOutOverride &&
		((s_pWD->fDistToEnemyMark < 6.0f && (m_fTimeWithoutLOS < 0.35f)) ||
		(aiutils_FTotalLoopSecs() - m_fAttackThoughtInitTime < 0.25f)	))
	{
		bDodgeTimeOutOverride = TRUE;
	}


	if (bDodgeTimeOutOverride  && bDodgeDamage && m_uDodgeState == DODGESTATE_OUT
		&& (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTPRED))
	{
		ResetDodge();
	}

	//if
	//		state == dodge-in				 &&
	//		not working or following a d-o-p &&
	//		client says it is an O.K. time to try to dodge
	//then
	//		request a dodgeout path
	if ((m_uDodgeState == DODGESTATE_IN) &&
		!FindRequestSlotInUse(SEARCHREASON_DODGEOUT) &&
		(bDodgeTimeOutOverride || s_pWD->uNowTime >= m_uDodgeTimeOut) &&
		(bOkToRequestDodgePaths || (bDodgeTimeOutOverride && bDodgeDamage)))


	{
		BOOL bGoodPoint = FALSE;
		CFVec3A DodgePt;


		//pick a dodge direction
		{
			f32 fDodgeDist = 1.0f+s_pWD->pMover->GetRadiusXZ();
			if (m_pBrain->GetInfreqCycleRandom() > 0.5f)
			{
				fDodgeDist*=2.0f;
			}
			if (m_uAttackFlags & FLAG_3DBOT && 
				m_fTimeWithLOS > 0.0f &&
				this->m_fDamageTakenPerSec < 0.001f && 
				m_pBrain->GetInfreqCycleRandom() < 0.8f && 
				(s_pWD->EnemyMark.y < s_pWD->pMover->GetLoc().y))
			{
				bGoodPoint = TRUE;
				DodgePt = s_pWD->pMover->GetLoc();
				f32 fDropDist = s_pWD->pMover->GetRadiusXZ();
				if (fDropDist < s_pWD->pMover->GetRadius())
				{
					fDropDist =  s_pWD->pMover->GetRadius();
				}
				fDropDist*=2.0f;
				DodgePt.y -=fDropDist;
			}
			else
			{
				u8 uDodgeDir = s_pWD->uNowTime & 1;
				uDodgeDir = this->m_uDodgePathFailureCount &3;
				if (uDodgeDir ==3)
				{	
					uDodgeDir = 4;
				}
				bGoodPoint = (this->*(s_pWD->pFindCurRelPos_Func))(&DodgePt,
																	s_pWD->EnemyMark,
																	s_pWD->pMover->GetLoc(),
																	fDodgeDist,
																	0.0f,
																	uDodgeDir,
																	DONT_CHECK_LOS_TO_ENEMY_MARK,
																	-1.0f,
																	-1.0f);
			}
		}
		if ( bGoodPoint )
		{
			RequestSearch(DodgePt, SEARCHREASON_DODGEOUT, 2.0f);
		}
	}


	//if 
	//		state == DODGESTATE_OUT					&&
	//		no d-i-p in use, or in progress			&&
	//		client says it is an O.K. time to try to dodge
	//then
	//		request a d-i-p
	if (m_uDodgeState == DODGESTATE_OUT &&
		!FindRequestSlotInUse(SEARCHREASON_DODGEIN) &&
		bOkToRequestDodgePaths)
	{
		RequestSearch(m_DodgeInLoc, SEARCHREASON_DODGEIN, 2.0f);
	}



	//if 
	//		just completed a d-o-p
	//then
	//		uDodgeState = DODGESTATE_OUT
	if (m_bPathJustCompleted &&
		m_nLastPathReason == SEARCHREASON_DODGEOUT)
	{
		m_uDodgeState = DODGESTATE_OUT;
		
		m_uDodgeOutTimeOut = aiutils_GetTimeOutRandMinMax(m_uDodgeOutTimeMin, m_uDodgeOutTimeMax);

		//normally, d.o.p - d.i.p combo packs are preserved until the enemy or player moves a certain amount.
		//But, there are some odds that the combo pack will be thrown out, even thought they could be used again.
		if (m_pBrain->GetInfreqCycleRandom() > 0.5f &&
			FindRequestSlotInUse(SEARCHREASON_DODGEOUT, &uDodgeOutSlot))
		{
			FreeRequestSlot(uDodgeOutSlot);
			if (FindRequestSlotInUse(SEARCHREASON_DODGEIN, &uDodgeInSlot))
			{
				FreeRequestSlot(uDodgeInSlot);
			}
		}
	}

	
	//if 
	//		state == dodge-in				&&
	//		not following any path			&&
	//		a d-o-p is ready				&&
	//		time to dodge out
	//then
	//		follow it
	if ((m_uDodgeState == DODGESTATE_IN) &&
		!IsFollowingPath() &&
  		(bDodgeTimeOutOverride || s_pWD->uNowTime >= m_uDodgeTimeOut) &&
		IsSearchDone(SEARCHREASON_DODGEOUT, &uDodgeOutSlot))
	{
		u32 uAssignPathFlags = 0;
		//code to test hopping by doing it during dodgeOut moves
		m_uDodgePathFailureCount = 0;
		if (bDodgeDamage && s_pWD->pBot && (s_pWD->pBot->IsAlertOn() || !s_pWD->pBot->IsCapableofAlert()))
		{
			uAssignPathFlags |= CAIMover::ASSIGNPATHFLAG_CANHOP;
		}
		Attack_AssignPath(uDodgeOutSlot, uAssignPathFlags);

		//reset our timer on the time in, now that we made it in
		m_uDodgeTimeOut = aiutils_GetTimeOutRandMinMax(m_uDodgeTimeMin, m_uDodgeTimeMax);

		//If we're dodging damage, mark the time
		if (bDodgeDamage)
		{
			m_uDamageDodgeTimeOut = m_uDamageDodgeTimeMin+s_pWD->uNowTime;
		}
	}

	//if
	//		a d-o-p fails || 
	//		a d-i-p fails  ||
	//		something besides dodge has moved us 
	//then
	//		reset the dodgestate
	f32 fDodgeTacticResetDistSq = s_pWD->pMover->GetRadiusXZ()+3.0f;
	fDodgeTacticResetDistSq *= fDodgeTacticResetDistSq;
	f32 fDistFromDodgeInSq = s_pWD->pMover->GetLoc().DistSq(m_DodgeInLoc);
	if (IsSearchFailed(SEARCHREASON_DODGEOUT) ||
		IsSearchFailed(SEARCHREASON_DODGEIN) ||
		(m_uDodgeState == DODGESTATE_IN  && !IsFollowingPath() && fDistFromDodgeInSq > fDodgeTacticResetDistSq) ||
		(fDistFromDodgeInSq >  (s_pWD->pMover->GetRadiusXZ()*4.0f)*(s_pWD->pMover->GetRadiusXZ()*4.0f)) ||
		s_pWD->pMover->IsStuck(3.0f) ||
		(m_bPathJustCompleted && m_nLastPathReason != SEARCHREASON_DODGEOUT && m_nLastPathReason != SEARCHREASON_DODGEIN))
	{
		m_uDodgePathFailureCount++;
		ResetDodge();
	}
	
	//if
	//		just completed a d-i-p
	//then
	//		uDodgeState = DODGESTATE_IN
	if (m_bPathJustCompleted &&
		m_nLastPathReason == SEARCHREASON_DODGEIN )
	{
		//reset our timer on the time in, now that we made it in
		m_uDodgeTimeOut = aiutils_GetTimeOutRandMinMax(m_uDodgeTimeMin, m_uDodgeTimeMax);
		
		m_uDodgeState = DODGESTATE_IN;
		if (m_pLookAtMgr)
		{
			m_pLookAtMgr->ResetVisRayScan();		  //should do this whenever we're gonna be stopped for awhile
		}
	}

	//if 
	//		at dodge-out
	//		not currently following a path
	//		a d-i-p is ready
	//		dodge-out timer is up

	//then				  
	//		follow it
	if (m_uDodgeState == DODGESTATE_OUT &&
		!IsFollowingPath() &&
		(bDodgeTimeOutOverride || s_pWD->uNowTime >= m_uDodgeOutTimeOut) &&
		IsSearchDone(SEARCHREASON_DODGEIN, &uDodgeInSlot))
	{
		u32 uAssignPathFlags = 0;
		//code to test hopping by doing it during dodgeOut moves
		if (bDodgeDamage && s_pWD->pBot && (s_pWD->pBot->IsAlertOn() || !s_pWD->pBot->IsCapableofAlert()))
		{
			uAssignPathFlags |= CAIMover::ASSIGNPATHFLAG_CANHOP;
		}
		m_uDodgePathFailureCount = 0;
		Attack_AssignPath(uDodgeInSlot, uAssignPathFlags);

		//just decide to start dodging in, reset our timer on this, in case we don't ever make it 
		m_uDodgeOutTimeOut = aiutils_GetTimeOutRandMinMax(m_uDodgeOutTimeMin, m_uDodgeOutTimeMax);

		//If we're dodging damage, mark the time
		if (bDodgeDamage)
		{
			m_uDamageDodgeTimeOut = m_uDamageDodgeTimeMin + s_pWD->uNowTime;
		}
	}
}




s8 CGroundCombat::FindFreeRequestSlot(void)
{
	for (s8 i = 0; i < NUM_PATH_REQUEST_SLOTS; i++)
	{
		if (m_anSearchReason[i] == SEARCHREASON_INVALID)
		{
			return i;
		}
	}
	return -1; 
}


BOOL CGroundCombat::FindRequestSlotInUse(s8 nReason, u8* puSlot /* = NULL */)	 //could be pending, searching, done, failed, or in use
{
	for (u8 i = 0; i < NUM_PATH_REQUEST_SLOTS; i++)
	{
		if (m_anSearchReason[i] == nReason)
		{
			if (puSlot)
			{
				*puSlot = i;
			}
			return TRUE;
		}
	}
	return FALSE; 
}


void CGroundCombat::FreeRequestSlot(s32 nSlot)
{
	FASSERT(nSlot >=0 && nSlot < NUM_PATH_REQUEST_SLOTS);

	if (m_nCurPathRequestSlot == nSlot)
	{  //just in case!
		m_pBrain->GetAIMover()->AssignPath(NULL);
		m_nCurPathRequestSlot = SEARCHREASON_INVALID;
		m_uAttackFlags |= FLAG_NOPATH;
	}

	aimain_pGraphSearcher->CancelSearch(&m_aSearchQuery[nSlot]); //safe even if not in progress
	m_aSearchQuery[nSlot].Clear();
	m_aSearchResults[nSlot].Clear();
	m_anSearchReason[nSlot] = SEARCHREASON_INVALID;
}	  


BOOL CGroundCombat::IsSearchDone(s8 nReason, u8* puSlot)
{
	for (u8 uSlot = 0; uSlot < NUM_PATH_REQUEST_SLOTS; uSlot++)
	{
		if (m_anSearchReason[uSlot] == nReason && m_aSearchQuery[uSlot].IsDone())
		{
			if (puSlot)
			{
				*puSlot = uSlot;
			}
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CGroundCombat::IsSearchFailed(s8 nReason, u8*puSlot /*= NULL*/)
{
	for (u8 uSlot = 0; uSlot < NUM_PATH_REQUEST_SLOTS; uSlot++)
	{
		if (m_anSearchReason[uSlot] == nReason && (m_aSearchQuery[uSlot].HasFailed() || m_aSearchQuery[uSlot].WasCanceled()))
		{
			if (puSlot)
			{
				*puSlot = uSlot;
			}
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CGroundCombat::IsFollowingPath(BOOL bCheckReason /*= FALSE*/, s8 uReason /*= 0*/)
{
	if (m_nCurPathRequestSlot != SEARCHREASON_INVALID)
	{
		if (bCheckReason && m_anSearchReason[m_nCurPathRequestSlot] != uReason)
		{
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CGroundCombat::RequestSearch(const CFVec3A &WhereTo, s8 nReason, f32 fFudgeDestDist)
{
	if (m_uAttackFlags & FLAG_COMBAT_CANT_MOVE_BOT)
	{   //sorry, the combat thought is not the thought that is responsible for moving the bot around right now
		return FALSE;
	}

	if (s_pWD->uMaxDistFromOrigin != 0xffff && WhereTo.DistSqXZ(m_AttackOrigin) > (f32) s_pWD->uMaxDistFromOrigin*s_pWD->uMaxDistFromOrigin)
	{
		return FALSE;  //Attack Parameters prevent movement that far from the origin.
	}

	if (!s_pWD->pMover->CanMove())
	{
		return FALSE;
	}
	FASSERT(!this->FindRequestSlotInUse(nReason));
	s8 nSlot = FindFreeRequestSlot();

	if (nSlot > SEARCHREASON_INVALID)
	{
		u16 uSearchParams = CSearchQuery::SEARCHPARAM_NEED_AIPATH | CSearchQuery::SEARCHPARAM_NO_SWITCHES | CSearchQuery::SEARCHPARAM_NO_LIFTS;
		if (m_uAttackFlags & FLAG_3DBOT)
		{
			uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
		}
		else
		{
			uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;
		}

		if (!s_pWD->pMover->CanJump())
		{
			uSearchParams &=~ CSearchQuery::SEARCHPARAM_NO_JUMPS;
		}

		aimain_pGraphSearcher->CancelSearch(&m_aSearchQuery[nSlot]); //safe even if not in progress
		if (m_aSearchQuery[nSlot].GetResults())
		{
			m_aSearchQuery[nSlot].GetResults()->Clear();
		}


		m_aSearchQuery[nSlot].Clear();
		CFVec3A WhereFrom;
		s_pWD->pMover->GetPathFindingOrigin(&WhereFrom);
		m_aSearchQuery[nSlot].SetDefaultSearchParams(WhereFrom,
														WhereTo,
														s_pWD->pMover->GetRadiusXZ(), //w
														s_pWD->pMover->GetMinHeight(), //h
														&m_aSearchResults[nSlot],
														uSearchParams,
														(u32) m_pBrain);
		aimain_pGraphSearcher->SubmitQuery(&(m_aSearchQuery[nSlot]));
		if (fFudgeDestDist > 0.001f)
		{
			m_aSearchQuery[nSlot].SetFudgeDestDist(fFudgeDestDist);
		}
		m_aSearchQuery[nSlot].SetMaxJumpDist(GetBrain()->GetMaxJumpDist());
		m_anSearchReason[nSlot] = nReason;

		return TRUE;
	}
	return FALSE;
}

void CGroundCombat::ReleaseLockedPoi(void)
{
	if (m_pLockedPoi)
	{
		if (aimain_pGraphAccess)
		{
			aimain_pGraphAccess->UnLockPoi(aimain_pAIGraph->GetPoiId(m_pLockedPoi));
		}
		m_pLockedPoi = NULL;
	}
}


void CGroundCombat::LockPoi(CGraphPoi* pPoi)
{
	ReleaseLockedPoi();
	if (aimain_pGraphAccess && pPoi)
	{
		aimain_pGraphAccess->LockPoi(aimain_pAIGraph->GetPoiId(pPoi));
		m_pLockedPoi = pPoi;
	}
}



void CGroundCombat::InitRuleSet(void)
{
	//
	//	Rule Set comes from aibrain
	//
	u8 uInitialRuleSet =  CAIBrain::ATTACKRULESET_0;
	if (GetBrain()->GetMechLock() ||
		(GetBrain()->GetMechUseageCtrl() == CAIBrain::MECH_USEAGE_CTRL_INIT && !m_pBrain->GetFlag_MechUseageCtrlInitFailed()))
	{
		uInitialRuleSet = CAIBrain::ATTACKRULESET_0;	   //ATTACKRULESET_0 is the only one that can deal with guys using vehicles.
	}
	else
	{
		uInitialRuleSet = m_pBrain->GetAttackRuleSet();


		//
		// ATTACKRULESET_NPC is an automatic custom replacement for ATTACKRULESET_0
		// for when NPC bots fight NPC bots
		//
		if (uInitialRuleSet == CAIBrain::ATTACKRULESET_0 &&
			m_pEnemy && 
			 (!(m_pEnemy->TypeBits() & ENTITY_BIT_BOT) || (((CBot*)m_pEnemy)->m_nPossessionPlayerIndex == -1)))
		{
			uInitialRuleSet = CAIBrain::ATTACKRULESET_NPC; 
		}

	}

	ChangeRuleSet(uInitialRuleSet);
}


void CGroundCombat::DoRuleSet(void)
{
	m_RuleSetFSM.Work();
}



#define MAX_NUM_POT_ENEMY_VAL 5

CEntity* _EvaluatePotentialEnemies(CAIBrain* pBrain, CEntity** paKnowEnemies, u32 uNumKnownEnemies)
{
	u16 uNowTime = aiutils_GetCurTimeSecsU16();
	s32 i;

	CEntity* apAIPotEnemy[MAX_NUM_POT_ENEMY_VAL];
	f32 afAIPotEnemyDistSq[MAX_NUM_POT_ENEMY_VAL];
	f32 afAIPotEnemyDamagedByTotal[MAX_NUM_POT_ENEMY_VAL];
	s32 anAIPotEnemyLastTimeSeen[MAX_NUM_POT_ENEMY_VAL];
	f32 afScore[MAX_NUM_POT_ENEMY_VAL];
/*
	u32 auAIPotEnemDistSort[MAX_NUM_POT_ENEMY_VAL];
	f32 afAIPotEnemyDamageTimes[MAX_NUM_POT_ENEMY_VAL];
	f32 afAIPotEnemyDamageThreat[MAX_NUM_POT_ENEMY_VAL];
	f32 afAIPotEnemyHealthLeft[MAX_NUM_MAX_NUM_POT_ENEMY_VALENEMY_VAL];
*/

	if (!pBrain)
	{
		return FALSE;
	}
		

	s32 nNumPots = 0;

	//
	//  Find all the enemies that I have seen in last two seconds
	//
	CAIMemoryLarge* pSeenMem = NULL;
	if (pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_SEEN, (CAIMemorySmall**)&pSeenMem))
	{
		while (pSeenMem && nNumPots < MAX_NUM_POT_ENEMY_VAL)
		{
			if (pSeenMem->m_pEntity &&
				(uNowTime - pSeenMem->m_uWhenTimeSecs) < 2 &&
				!aiutils_IsFriendly(pBrain->GetAIMover()->GetEntity(), pSeenMem->m_pEntity))
			{
				apAIPotEnemy[nNumPots] = pSeenMem->m_pEntity;
				anAIPotEnemyLastTimeSeen[nNumPots] = pSeenMem->m_uWhenTimeSecs;
				afAIPotEnemyDamagedByTotal[nNumPots] = 0.0f;
				nNumPots++;
			}

			pSeenMem = (CAIMemoryLarge*) pBrain->GetKnowledge().GetNextOldestOfId(pSeenMem);
		}
	}
	
	//
	//  Find all the enemies that have damaged me.
	//
	CAIMemoryLarge* pDamagedByMem = NULL;
	if (nNumPots < MAX_NUM_POT_ENEMY_VAL &&
		pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall**) &pDamagedByMem))
	{
		while (pDamagedByMem && nNumPots < MAX_NUM_POT_ENEMY_VAL)
		{
			if (pDamagedByMem->m_pEntity &&
				!aiutils_IsFriendly(pBrain->GetAIMover()->GetEntity(), pDamagedByMem->m_pEntity))
			{
				
				for (i = 0; i < nNumPots; i++)
				{
					if (apAIPotEnemy[i] == pDamagedByMem->m_pEntity)
					{
						break;
					}
				}
				if (i == nNumPots)
				{
					apAIPotEnemy[nNumPots] = pDamagedByMem->m_pEntity;
					anAIPotEnemyLastTimeSeen[nNumPots] = -30;
					nNumPots++;
				}
				afAIPotEnemyDamagedByTotal[i] = fmath_Div(((f32)pDamagedByMem->m_uMediumData), CAIBrain::s_fDamagedByTotalDamageMultiplier);
			}

			pDamagedByMem = (CAIMemoryLarge*) pBrain->GetKnowledge().GetNextOldestOfId(pDamagedByMem);
		}
	}

	u32 ii;
	s32 j;
	for (ii = 0; ii < uNumKnownEnemies && nNumPots < MAX_NUM_POT_ENEMY_VAL; ii++)
	{
		for (j = 0; j < nNumPots; j++)
		{
			if (paKnowEnemies[ii] && 
				paKnowEnemies[ii] == apAIPotEnemy[j])
			{
				break;
			}
		}

		if (paKnowEnemies[ii] && j == nNumPots)
		{
			apAIPotEnemy[nNumPots] = paKnowEnemies[ii];
			anAIPotEnemyLastTimeSeen[nNumPots] = -30;
			afAIPotEnemyDamagedByTotal[nNumPots] = 0.0f;
			nNumPots++;
		}

	}

	s32  uClosest = -1;
	f32 fClosestDistSq = FMATH_MAX_FLOAT;
	f32 fFarthestDistSq = 0.0f;
	s32 uFarthest = -1;
	s32 uMostDamagedBy = -1;
	f32 fMostDamageAmount = 0.0f;

	//
	// Calc Distance Squared
	// Remember closest, farthest, most Damage By
	//
	for (i = 0; i < nNumPots; i++)
	{
		afAIPotEnemyDistSq[i] = pBrain->GetAIMover()->GetLoc().DistSq(apAIPotEnemy[i]->MtxToWorld()->m_vPos);
		if (afAIPotEnemyDistSq[i] < fClosestDistSq)
		{
			uClosest = i;
			fClosestDistSq  = afAIPotEnemyDistSq[i];
		}
		if (afAIPotEnemyDistSq[i] > fFarthestDistSq)
		{
			uFarthest = i;
			fFarthestDistSq = afAIPotEnemyDistSq[i];
		}

		if (afAIPotEnemyDamagedByTotal[i] >	fMostDamageAmount)
		{
			uMostDamagedBy = i;
			fMostDamageAmount = afAIPotEnemyDamagedByTotal[i];
		}
	}

	for (i = 0; i < nNumPots; i++)
	{
		f32 fTimeSinceSeenScore = (f32) uNowTime - anAIPotEnemyLastTimeSeen[i];
		if (fTimeSinceSeenScore > 10.0f)
		{
			fTimeSinceSeenScore = 10.0f;
		}
		f32 fDamagedByScore = 0.0f;
		if (fMostDamageAmount > 0.0f)
		{
			fDamagedByScore = fmath_Div(afAIPotEnemyDamagedByTotal[i], fMostDamageAmount);
		}

		if ( FMATH_FABS(afAIPotEnemyDistSq[i] - fFarthestDistSq) > 0.00001f )
		{
			afScore[i] = (1.0f - fmath_Div(afAIPotEnemyDistSq[i], fFarthestDistSq)) +
						fDamagedByScore +
						(10.0f-fTimeSinceSeenScore)*0.1f +
						apAIPotEnemy[i]->ComputeUnitHealth() +
						-0.5f*(!apAIPotEnemy[i]->IsTargetable());
		}
		else
		{
			afScore[i] = 1.0f + 
						fDamagedByScore +
						(10.0f-fTimeSinceSeenScore)*0.1f +
						apAIPotEnemy[i]->ComputeUnitHealth() +
						-0.5f*(!apAIPotEnemy[i]->IsTargetable());
		}
	}

	s32 nBest = -1;
	f32 fBestScore = 0.0f;
	for (i = 0; i < nNumPots; i++)
	{
		if (afScore[i] > fBestScore)
		{
			fBestScore = afScore[i];
			nBest = i;
		}
	}


	if (nBest > -1)
	{
		return apAIPotEnemy[nBest];
	}

	return NULL;
}



//
// GatherAttackWorkContext
// Setup Context for this Attack
//
void CGroundCombat::AttackWorkBeginFrame(void)
{
	u8 uPerceptorsCtrl = 0;
	s_pWD = &WD;
	s_pWD->Init(this);

	uPerceptorsCtrl = m_pBrain->GetPerceptionCtrl();
	
	m_uAttackFlags &= ~FLAGS_TO_CLEAR_EVERY_FRAME;

	if (m_pLastKnownEnemyVehicle &&
		(!(m_pLastKnownEnemyVehicle->IsInWorld()) || 
		  ((m_pLastKnownEnemyVehicle->TypeBits() & ENTITY_BIT_BOT) &&
		  ((CBot*) m_pLastKnownEnemyVehicle)->IsDeadOrDying())))
	{
		m_pLastKnownEnemyVehicle = NULL;
	}


	if (m_uEnemyEvalTimeOut < s_pWD->uNowTime)
	{
		CEntity* pBestEnemy = NULL;
		pBestEnemy = _EvaluatePotentialEnemies(m_pBrain, &m_pEnemy, 1);

		if (pBestEnemy && pBestEnemy != m_pEnemy )
		{
			if (m_pEnemy)
			{

				BOOL bOkToAttack = TRUE;
				CAIBrain* pLeaderBrain = GetBrain()->GetLeader();
				if (pLeaderBrain)		 //rules of engagement for followers
				{
					bOkToAttack = GetBrain()->ShouldFollowerAttack(pLeaderBrain, pBestEnemy);
				}

				if (bOkToAttack)
				{
					//
					// NOTE!!!!!! don't automatically pick up an enemies if never had one
					// this is because the reaction system will do it for you by re-issuing attack orders
					//
					ChangeEnemy(pBestEnemy);

					//cause all vbls this frame to be re-evaluated since we changed enemies this frame
					s_pWD->Init(this);

					//causes a reset on current rulset, rulesetstate, and tactic 
					InitRuleSet();
				}
			}


		}
		m_uEnemyEvalTimeOut = s_pWD->uNowTime + 1;
	}

	if (uPerceptorsCtrl & CAIBrain::PCFLAG_TOUCH_INUSE)
	{
		m_pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGE_DEALT, (CAIMemorySmall **) &s_pWD->pDamageDealtMem);
		m_pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall **) &s_pWD->pDamagedByMem);
	}
	if (uPerceptorsCtrl & CAIBrain::PCFLAG_EYES_INUSE)
	{
		m_pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, m_pEnemy, &s_pWD->pSeenMem);
		if (m_pEnemy && s_pWD->pSeenMem)
		{
			m_uAttackFlags |= FLAG_HASEVERSEENENEMY;
		}
	}
	if (uPerceptorsCtrl & CAIBrain::PCFLAG_EARS_INUSE)
	{	//
		// possible cheater, I mean, depends on whether the player's sound is distinguishable amongst all other sounds
		//
		m_pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_HEARD, m_pEnemy, &s_pWD->pHeardMem);
	}
	if (m_pEnemy && m_pEnemy->AIBrain())
	{	//cheater  Look inside my target's brain and get a pointer to his last DamageTaken memory
		m_pEnemy->AIBrain()->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall **) &s_pWD->pEnemyDamagedByMem);
	}

	//Track Damage Dealt and Damage Taken Per Second
	if (m_uDamageSec < s_pWD->uNowTime)
	{
	   m_uDamageSec = s_pWD->uNowTime;
	   m_fDamageTakenPerSec = 0.0f;	  //reset every second
	   m_fDamageDealtPerSec = 0.0f;
	   m_fEnemyDamageTakenPerSec = 0.0f;
	}

	if (s_pWD->pDamagedByMem && s_pWD->pDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
	{
		m_fDamageTakenPerSec += s_pWD->pDamagedByMem->m_fLargeData;	   //found damage memory
	}
	if (s_pWD->pDamageDealtMem && s_pWD->pDamageDealtMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
	{
		m_fDamageDealtPerSec += s_pWD->pDamageDealtMem->m_fLargeData;
	}
	if (s_pWD->pEnemyDamagedByMem)
	{
		if (m_pEnemy && m_pEnemy->AIBrain())
		{	//cheater  Look inside my target's brain and get a pointer to his last DamageTaken memory
			CAIMemoryLarge* pEnemyDamagedByMem = NULL;
			m_pEnemy->AIBrain()->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall **) &pEnemyDamagedByMem);
			while (pEnemyDamagedByMem && pEnemyDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
			{
				m_fEnemyDamageTakenPerSec += s_pWD->pEnemyDamagedByMem->m_fLargeData;
				pEnemyDamagedByMem = (CAIMemoryLarge*)m_pEnemy->AIBrain()->GetKnowledge().GetNextOldestOfId(pEnemyDamagedByMem);
			}
		}
	}
	if (s_pWD->pHeardMem && (s_pWD->pHeardMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY))
	{
		if (m_pLookAtMgr)
		{
        	m_pLookAtMgr->ForceLookAtWhileScanning(CFVec3A(s_pWD->pHeardMem->m_EntityLoc.x, s_pWD->pHeardMem->m_EntityLoc.y, s_pWD->pHeardMem->m_EntityLoc.z));
		}
	}
	if (s_pWD->pDamagedByMem && (s_pWD->pDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY))
	{
		if (m_pLookAtMgr)
		{
			m_pLookAtMgr->ForceLookAtWhileScanning(CFVec3A(s_pWD->pDamagedByMem->m_EntityLoc.x, s_pWD->pDamagedByMem->m_EntityLoc.y, s_pWD->pDamagedByMem->m_EntityLoc.z));
		}
	}

	//special case in which we have damaged ourselves.
	if (s_pWD->pDamagedByMem && s_pWD->pDamagedByMem->m_pEntity == s_pWD->pMover->GetEntity())
	{  //setting this to null will prevent further reactions to the damage
		// but keep the m_fDamageTakenPerSec up to date
		s_pWD->pDamagedByMem = NULL;
	}

	s_pWD->bLostTheEnemy = !WhereIsEnemy(	s_pWD->pSeenMem,	s_pWD->pHeardMem,	s_pWD->pDamagedByMem,	&s_pWD->EnemyMark, &s_pWD->bHasLOSThisFrame);

	s_pWD->DeltaToEnemyMarkUnit.Sub(s_pWD->EnemyMark, s_pWD->pMover->GetLoc());
	if ((s_pWD->fDistToEnemyMark = s_pWD->DeltaToEnemyMarkUnit.SafeUnitAndMag(s_pWD->DeltaToEnemyMarkUnit)) < 0.0f)
	{
		s_pWD->fDistToEnemyMark = 0.0f;
		s_pWD->DeltaToEnemyMarkUnit = s_pWD->pMover->GetTorsoLookAtXZ();
	}

	s_pWD->fDistToEnemyMarkXZ = s_pWD->EnemyMark.DistXZ(s_pWD->pMover->GetLoc());

	if (s_pWD->bHasLOSThisFrame)
	{
		if (m_fTimeWithoutLOS > 0.3f)
		{
			m_uLosStatusChangeCounter++;
		}
		m_fTimeWithLOS += FLoop_fPreviousLoopSecs;
		m_fTimeWithoutLOS = 0.0f;		  //got los, so time without it is 0.0f
	}
	else
	{
		if (m_fTimeWithLOS > 0.3f)
		{
			m_uLosStatusChangeCounter++;
		}
		m_fTimeWithoutLOS += FLoop_fPreviousLoopSecs;	   //just grows and grows
		m_fTimeWithLOS = 0.0f;
	}

	if (s_pWD->pBot &&
		s_pWD->pBot->HasTargetLock())
	{
		m_fTimeWithTargetLock+=FLoop_fPreviousLoopSecs;
		m_fTimeWithoutTargetLock = 0.0f;
	}
	else
	{
		m_fTimeWithoutTargetLock+=FLoop_fPreviousLoopSecs;
		m_fTimeWithTargetLock=0.0f;
	}

	//divide world into a 2x2x2 grid, track movement on this grid
	if ((u32)(m_LastEnemyMark.x*0.5f) != (u32) (s_pWD->EnemyMark.x*0.5f) || 
		(u32)(m_LastEnemyMark.y*0.25f) != (u32) (s_pWD->EnemyMark.y*0.25f) ||
		(u32)(m_LastEnemyMark.z*0.5f) != (u32) (s_pWD->EnemyMark.z*0.5f))
	{
		m_fEnemyMarkLastSmallGridChangeTime = aiutils_GetCurTimeSecs();
	}
	m_LastEnemyMark = s_pWD->EnemyMark;

	if (!(m_uAttackFlags & CGroundCombat::FLAG_STAYPUTBARRIER_BROKEN) &&
		s_pWD->uStayPutDist != 0xffff &&
		((u16)s_pWD->fDistToEnemyMark < s_pWD->uStayPutDist))
	{
		m_uAttackFlags |= CGroundCombat::FLAG_STAYPUTBARRIER_BROKEN;
	}
	
	u32 uTagToShootAt = 0; //findfix: should pick the one that was actually verified as having los with
	if (m_pEnemy && m_pEnemy->GetTagPoint(uTagToShootAt))
	{
		s_pWD->EnemyMarkTagPos.Sub(*(m_pEnemy->GetTagPoint(uTagToShootAt)), aiutils_GetEntityLoc(m_pEnemy));

		f32 fRange = s_pWD->pMover->GetRadius()+20.0f;
		if (s_pWD->fDistToEnemyMark > fRange)
		{
			s_pWD->EnemyMarkTagPos.Mul(0.5f);
		}
		else
		{
			s_pWD->EnemyMarkTagPos.Mul(0.5f+0.5f*(1.0f - s_pWD->fDistToEnemyMark/fRange));
		}

	}
	s_pWD->EnemyMarkTagPos.Add(s_pWD->EnemyMark);

	if (s_pWD->bLostTheEnemy)
	{	//no idea where the enemy is.....
		if (!(m_uAttackFlags & FLAG_CANTFINDENEMY))
		{
			m_uAttackFlags |= FLAG_CANTFINDENEMY;
			m_uLostEnemyTime = s_pWD->uNowTime;
		}
	}
	else
	{
		m_uAttackFlags &= ~FLAG_CANTFINDENEMY;
	}

	if (s_pWD->bHasLOSThisFrame)
	{
		m_uAttackFlags |= FLAG_HASTARGETLOS;
	}

	s_pWD->bSensesDanger = (s_pWD->pDamagedByMem && ((s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs) < 2));  //findfix: should be a calculated danger factor

	//Is there a poi near the Target
	if (m_fTimeWithoutLOS < 1.5f && ((s_pWD->uNowTime & 1) == (m_pBrain->GetGUID() & 1)))
	{
		s_pWD->pEnemyPoi = aimain_pAIGraph->FindClosestPoi(s_pWD->EnemyMarkTagPos, 5.0f, 0);
		if (s_pWD->pEnemyPoi)
		{
			CFVec3A PoiLoc;
			s_pWD->pEnemyPoi->GetLocation(aimain_pAIGraph, &PoiLoc);
			s_pWD->fDistToEnemyPoi = PoiLoc.Dist(s_pWD->EnemyMarkTagPos);
		}
	}

	if ((m_uAttackFlags & FLAG_BOT_WAS_DOING_SPECIAL_MOVE))
	{
		if (s_pWD->pDamageDealtMem && (s_pWD->pDamageDealtMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY) && s_pWD->pDamageDealtMem->m_pEntity == s_pWD->pMover->GetEntity())
		{
			m_uAttackFlags |= FLAG_DID_DAMAGE_DURING_SPECIAL_MOVE;
		}
		else if (s_pWD->pBot &&
			 	 s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTPROBE &&
				((CBotProbe*)s_pWD->pBot)->IsLeeching(NULL))
		{	//need special case for if I grabbed him, but didn't do damage for some reason.
			m_uAttackFlags |= FLAG_DID_DAMAGE_DURING_SPECIAL_MOVE;
		}
	}
}


void CGroundCombat::DoWeaponAimAndFireWork()
{
	//
	// 	Weapon firing and aim
	//
	if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTTITAN)
	{
		DoWeaponAimAndFireWork_Titan();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTELITEGUARD)
	{
		DoWeaponAimAndFireWork_EliteGuard();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTZOM)
	{
		//none, melee trigger is pulled inside of botzom ruleset
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTPRED)
	{
		DoWeaponAimAndFireWork_Predator();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTPROBE)
	{
		DoWeaponAimAndFireWork_Probe();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTCORROSIVE)
	{
		DoWeaponAimAndFireWork_Corrosive();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTZOMBIEBOSS)
	{
		DoWeaponAimAndFireWork_ZombieBoss();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTSCIENTIST)
	{
		DoWeaponAimAndFireWork_Scientist();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLESENTINEL)
	{
		DoWeaponAimAndFireWork_Sentinel();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLERAT)
	{
		DoWeaponAimAndFireWork_Rat();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLELOADER)
	{
		DoWeaponAimAndFireWork_Loader();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTJUMPER)
	{
		DoWeaponAimAndFireWork_Jumper();
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTSNARQ)
	{
		DoWeaponAimAndFireWork_Snarq();
	}
	else
	{
		if (m_RuleSetStateFSM.GetActiveStateHandle() == ARS_0_SITEGUNNER &&
			((CBot*) s_pWD->pMover->GetEntity())->GetCurMech() &&
			(((CBot*) s_pWD->pMover->GetEntity())->GetCurMech()->TypeBits() & ENTITY_BIT_SITEWEAPON))
		{
			if (((CBot*) s_pWD->pMover->GetEntity())->GetCurMech()->IsPillBox())
			{
				DoWeaponAimAndFireWork_SiteGunner();
			}
			else if (((CBot*) s_pWD->pMover->GetEntity())->GetCurMech()->IsRatGun())
			{
				DoWeaponAimAndFireWork_RatGun();
			}
			else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
			{
				DoWeaponAimAndFireWork_Grunt();
			}
			else
			{
				DoWeaponAimAndFireWork_Generic();
			}
		}
		else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
		{
			DoWeaponAimAndFireWork_Grunt();
		}
		else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
		{
			DoWeaponAimAndFireWork_Grunt();
		}
		else
		{
			DoWeaponAimAndFireWork_Generic();
		}
	}
}

void CGroundCombat::AttackWorkEndFrame(void)
{
	BOOL bWasDoingSpecialMove = m_uAttackFlags & FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
	
	m_uAttackFlags &= ~FLAG_BOT_WAS_DOING_SPECIAL_MOVE;

	//as bots get implemented that have a special move, set the	  FLAG_BOT_WAS_DOING_SPECIAL_MOVE now
	if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTTITAN)
	{
		m_uAttackFlags |= ((CBotTitan*) s_pWD->pMover->GetEntity())->IsStomping()*FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTPROBE)
	{
		CBotProbe* pProbe = (CBotProbe*) s_pWD->pMover->GetEntity();
		if (pProbe->IsAttackLeechEnabled() || pProbe->IsAttackSpinEnabled())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLELOADER)
	{
		CVehicleLoader* pProbe = (CVehicleLoader*) s_pWD->pMover->GetEntity();
		if (pProbe->IsClawInUse())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTJUMPER)
	{
		CBotJumper* pJumper = (CBotJumper*) s_pWD->pBot;
		if (pJumper->IsSlashing())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
		else if (pJumper->IsDiving())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTMOZER)
	{
		CBotMozer* pMozer = (CBotMozer*) s_pWD->pBot;
		if (pMozer->IsSwinging())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTELITEGUARD)
	{
		CBotEliteGuard* pEG = (CBotEliteGuard*) s_pWD->pBot;
		if (pEG->IsSwinging())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTSCIENTIST)
	{
		CBotScientist* pTit = (CBotScientist*) s_pWD->pBot;
		if (pTit->IsAttacking())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
	}
	else if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTZOM)
	{
		CBotZom* pZom = (CBotZom*) s_pWD->pBot;
		if (pZom->IsSlashing())
		{
			m_uAttackFlags |= FLAG_BOT_WAS_DOING_SPECIAL_MOVE;
		}
	}


	if (!(m_uAttackFlags & FLAG_BOT_WAS_DOING_SPECIAL_MOVE))
	{
		if (bWasDoingSpecialMove)
		{
			if (m_uAttackFlags & FLAG_DID_DAMAGE_DURING_SPECIAL_MOVE)
			{   //It was me that did damage to him.
				m_uAttackFlags |= FLAG_JUST_DID_SUCCESSFUL_MELEE;
			}
			else
			{
				m_uAttackFlags |= FLAG_JUST_DID_FAILED_MELEE;
			}
		}
		else
		{
			m_uAttackFlags &= ~(FLAG_JUST_DID_FAILED_MELEE | FLAG_JUST_DID_SUCCESSFUL_MELEE);
		}
		m_uAttackFlags &= ~FLAG_DID_DAMAGE_DURING_SPECIAL_MOVE;
	}
	s_pWD = NULL;
}


void CGroundCombat::DoGenericEnviroReactRules(BOOL bDodgeGrenades, BOOL bDodgeVehicles, BOOL bDamageBTAReaction)
{
	CGroundCombat* pT = this;
	CGroundCombatWorkContext* _pWD = s_pWD;
	CAIBrain* pBrain = GetBrain();

	//	 if
	//		Not currently doing the react rules in AIBrain_React code &&
	//		Not already reacting to something &&
	//		Ears and Eyes On
	//		
	//   then
	//			scan for sounds
	//			and do grenade dodge if necessary
	//
	if (!(pT->m_uThoughtFlags & CAIThought::THOUGHTFLAG_USE_JOB_REACT_RULES))
	{
		u8 uPerceptorsCtrl = pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL);

		if (!pBrain->IsReacting() &&
			(pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL) & (CAIBrain::PCFLAG_EYES_INUSE | CAIBrain::PCFLAG_EARS_INUSE)))
		{
			CAIMemoryLarge* pMem = NULL;
			CNiIterator<CAIMemoryLarge*> it;
			while (	(uPerceptorsCtrl & CAIBrain::PCFLAG_EARS_INUSE) &&
					(pMem = pBrain->GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_HEARD, &it)))
			{
				CAISound* pSound = AIEnviro_SoundHandleToPtr((AISoundHandle) pMem->m_uMediumData);
				if (pSound)
				{
					if (pSound->m_uSoundType == AISOUNDTYPE_GRENADE)
					{
						if (bDodgeGrenades)
						{
							if (!pBrain->Action_DodgeGrenade(pSound->m_Origin))
							{
								pBrain->SetSoundMark(pSound);
							}
						}
						break;
					}
					else if (pSound->m_uSoundType == AISOUNDTYPE_BOT &&
						pSound->m_pEntity &&
						pSound->m_pEntity->GetParent() &&
						((pSound->m_pEntity->GetParent()->TypeBits() & ENTITY_BIT_VEHICLESENTINEL) ||
						(pSound->m_pEntity->GetParent()->TypeBits() & ENTITY_BIT_VEHICLERAT)))
					{	
						if (bDodgeVehicles)
						{
							if (!pBrain->Action_DodgeVehicle(pSound))
							{
								pBrain->SetSoundMark(pSound);
							}
						}
						break;
					}
				}
			}
		}

		if (bDamageBTAReaction && (uPerceptorsCtrl & CAIBrain::PCFLAG_TOUCH_INUSE))
		{
			if (_pWD->pDamagedByMem && !(_pWD->pDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED))
			{
				pT->GetBrain()->PlayBTAOnTakeDamage(_pWD->pDamagedByMem);
				_pWD->pDamagedByMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
			}
		}

		//
		//	 Anybody click action on me?
		//
		if (!pT->GetBrain()->GetLeader())
		{
			CAIMemorySmall* pSmallMem = NULL;
			if (pT->GetBrain()->GetKnowledge().CanRememberAny(MEMORY_SMALL_PLAYERPROMPTED, &pSmallMem) &&
				(pSmallMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY) &&
				!(pSmallMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED))
			{
				FASSERT(pSmallMem->m_uSmallData >= 0 && pSmallMem->m_uSmallData < CPlayer::m_nPlayerCount);
				pT->GetBrain()->Rules_Job_Generic_OnPrompted(pSmallMem);
			}
		}
	}
}

//
//  Control the Torso and Head Direction
//
void CGroundCombat::DoTorsoAndHeadWork(void)
{
	if (m_pBrain->IsReacting() && m_pBrain->GetReactionState() != CAIBrain::REACTIONSTATE_ATTACK)
	{
		 //let reaction superseed the combat torso an dhead controls
		return;
	}



	if (m_uAttackFlags & FLAG_TORSO_NO_WORK)
	{
	}
	else if ((m_uAttackFlags & (FLAG_TORSO_DIR_ENEMY | FLAG_TORSO_SCAN | FLAG_TORSO_DIR_FORWARD)))
	{

		//
		// if 
		//     on path that is not a dodge path
		// then
		//		let pathfollowing set look-at
		// else if 
		//		no-sign of player for some time,
		// then
		//    scan the area
		// else
		//	  look at the EnemyMark
		if (m_uAttackFlags & FLAG_TORSO_DIR_ENEMY)
		{	//look at the enemy mark
			s_pWD->pMover->FaceToward(s_pWD->EnemyMarkTagPos, &s_pWD->fAlignment);
		}
		else if (m_uAttackFlags & FLAG_TORSO_SCAN)
		{	//search mode.
			if (m_pLookAtMgr)
			{
    			m_pLookAtMgr->Work();		//scan mode, but should look at the mark occasionally.
									//if mark is blocked, look as close to it as possible
			}
		}
		else if (m_uAttackFlags & FLAG_TORSO_DIR_FORWARD)
		{	
			if (IsFollowingPath())
			{	//"AIBotMover::FollowPath() sets the look at, don't override that
			}
			else if (m_fTimeWithoutLOS > 1.35f &&
					 (!s_pWD->pHeardMem || (s_pWD->pHeardMem && s_pWD->uNowTime - s_pWD->pHeardMem->m_uWhenTimeSecs > 3.0f)))	   //probably this should e built into the lookatmgr
			{	//scan mode, but should look at the mark occasionally.
				//if mark is blocked, look as close to it as possible
				if (m_pLookAtMgr)
				{
					m_pLookAtMgr->Work();		
				}
			}
			else
			{	//look at the EnemyMarkTagPoss

				s_pWD->pMover->FaceToward(s_pWD->EnemyMarkTagPos, &s_pWD->fAlignment);
			}
		}
		else
		{
			FASSERT(0);
		}
	}
	else
	{
		if (m_fTimeWithoutLOS > 1.35f)
		{
			if (IsFollowingPath())
			{	//"AIBotMover::FollowPath() sets the look at, don't override that
			}
			else
			{
				if (m_pLookAtMgr)
				{
					m_pLookAtMgr->Work();		//scan mode, but should look at the mark occasionally.
										//if mark is blocked, look as close to it as possible
				}
			}
		}
		else
		{
			s_pWD->pMover->FaceToward(s_pWD->EnemyMarkTagPos, &s_pWD->fAlignment);
		}
	}

	if (m_uAttackFlags & FLAG_HEAD_NO_WORK)
	{
		//nothing since either ruleset, rulesetstate or tactic says so.
	}
	else if (m_uAttackFlags & FLAG_HEAD_SCAN)
	{
		m_pBrain->DoScanHeadWork();
	}
}


void CGroundCombat::DoWeaponAimAndFireWork_Generic(f32 fCheckLosFromMuzzleDist /* 0.0f */, BOOL bDoAimLogic/* = TRUE*/, BOOL bCheckForFriendsBlockingWay/* = TRUE */ )
{
	u8 uWeaponId = 0;
	CAIWeaponCtrl* pWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(uWeaponId);

	if (!pWeaponCtrl)
	{
		return;
	}

	pWeaponCtrl->Work();	 //call this once a frame, before asking any questions of the weaponnctrl obo

	if ((m_uAttackFlags & FLAG_PANIC_ON) ||
		(s_pWD->pBot && (s_pWD->pBot->IsHopping() || s_pWD->pBot->IsRolling())) ||
		(s_pWD->pEnemyBot && !s_pWD->pEnemyBot->IsTargetable()) ||
		s_pWD->pMover->IsTalkingWithAnim() ||
		(!m_pEnemy && m_pBrain->GetReactionState() != CAIBrain::REACTIONSTATE_ATTACK_IMPULSE) ||	 //don't blind fire, unless on impulse reactio state
		!s_pWD->pMover->CanFirePrimary() ) {
		return;
	}

	BOOL bNormalFire = FALSE;

	if (pWeaponCtrl->IsBursting())
	{
		bNormalFire = TRUE;
	}
	
	//if 
	//	 recently had LOS	&&
	//   BotLayer weapon system returned that a lock is met, which means the bullet will go within some degrees of where I told it to
	//then
	//   calculate some odds that a shot will be fired
	if (m_pEnemy &&
		(!pWeaponCtrl->IsBursting() || pWeaponCtrl->CanAbortBursts()) &&
		m_fTimeWithoutLOS < 1.0f && 
		s_pWD->pMover->HasTargetLock()) // HasTargetLock means that the weapon will fire. Angle is within some internal tolerance to be considered a "lock" circa 20degrees
	{
		u32 uFireOdds = 100;
		if (m_uAttackFlags & FLAG_HASTARGETLOS)
		{
			uFireOdds = 90;
		}
		else
		{
			uFireOdds = 5;
			if (m_fTimeWithoutLOS < 3.0f)
			{
				uFireOdds += (u16) (80.0f*((3.0f-m_fTimeWithoutLOS)*0.33f));
			}
		}

		if (pWeaponCtrl->GetCtrlFlag_StopAndShoot() &&
			!(m_uAttackFlags & FLAG_TRIGGEROVERRIDE_STOPSHOOT) &&
			s_pWD->fDistToEnemyMark > 10.0f &&
			s_pWD->pMover->GetMoveSpeed() > 1.0f)
		{	
			uFireOdds = 0;
		}

		//being farther away from target decreases the desire to take a shot
		f32 fMaxDistRelatedDampening = 0.25f;	  //findfix: adjust these for different units possibly
		f32 fDistRelatedDampeningFeetThreshold = 50.0f;
		fDistRelatedDampeningFeetThreshold += fDistRelatedDampeningFeetThreshold*s_pWD->fAggression; //how far away
		f32 fDistRelatedDampening = fMaxDistRelatedDampening;
		if (uFireOdds && s_pWD->fDistToEnemyMark < fDistRelatedDampeningFeetThreshold)
		{
			fDistRelatedDampening = fMaxDistRelatedDampening*s_pWD->fDistToEnemyMark/fDistRelatedDampeningFeetThreshold;
			fDistRelatedDampening += (1.0f - fDistRelatedDampening)*s_pWD->fAggression;
		}
		uFireOdds = (u8) (uFireOdds*fDistRelatedDampening);

		if (fmath_RandomChoice(100) < uFireOdds)
		{
			bNormalFire = TRUE;
		}
		else
		{
			bNormalFire = FALSE;
		}
	}
	else if (!m_pEnemy)
	{
		if (s_pWD->pBot && s_pWD->pBot->NormHealth() < 1.0f)
		{
			if (m_uAttackFlags & FLAG_CANTFINDENEMY &&
				m_pBrain->GetWorldAddRandom() < 0.77f)
			{
				if (s_pWD->uNowTime - (u16)m_fAttackThoughtInitTime < 2)
				{
					bNormalFire = TRUE;
				}
			}

			if (!bNormalFire &&
				s_pWD->fAggression > 0.75f &&
				s_pWD->pDamagedByMem &&
				s_pWD->uNowTime < m_uBlindResponseToDamageTimeOut && 
				(s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs< 2) )
			{
				bNormalFire = TRUE;
			}
		}
	}
	else if (m_pEnemy && !s_pWD->pEnemyBot)  //enemy is not a bot
	{
		bNormalFire = TRUE;
	}

	if ((bNormalFire || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
		pWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY))
	{  

		BOOL bBlockedShot = FALSE;
		
		if (bCheckForFriendsBlockingWay)
		{
			bBlockedShot = _IsFriendBlockingWay(this);
		}
		
		//some weapons are so deadly, we need to try to make sure they will go at least a certain distance
		if (!bBlockedShot &&
			!pWeaponCtrl->IsBursting() && //only do this check when a burst is about to begin.  for performance reasons, hopefully bursts with deadly weapons aren't that long.
			fCheckLosFromMuzzleDist > 0.01f &&
			s_pWD->pBot)
		{
			CFVec3A MuzzlePt;
			s_pWD->pBot->ComputeApproxMuzzlePoint_WS(&MuzzlePt);

			CFVec3A RocketSafetyEndPt;
			if (m_LastAim == CFVec3A::m_Null)
			{
				//confused?  why would we have lock, but not have m_LastAim?
				//Let's assume bot will shoot forward
				RocketSafetyEndPt = s_pWD->pMover->GetTorsoLookAt();
			}
			else
			{
				RocketSafetyEndPt.Sub(m_LastAim, MuzzlePt);
			}
			if (RocketSafetyEndPt.SafeUnitAndMag(RocketSafetyEndPt) > 0.0f)
			{
				RocketSafetyEndPt.Mul(fCheckLosFromMuzzleDist+s_pWD->pMover->GetRadiusXZ());
				RocketSafetyEndPt.Add(MuzzlePt);
				if (!aiutils_IsLOSObstructed_IgnoreBots(MuzzlePt, RocketSafetyEndPt, s_pWD->pBot))
				{
					aiutils_DebugTrackRay(MuzzlePt, RocketSafetyEndPt, TRUE);
				}
				else
				{
					bBlockedShot  = TRUE;
					aiutils_DebugTrackRay(MuzzlePt, RocketSafetyEndPt, FALSE);
				}
			}
		}

		if (!bBlockedShot)
		{
			pWeaponCtrl->NotifyFire();
			s_pWD->pMover->m_Controls.Fire();
		}
	}

	//
	// aim
	//
	if (bDoAimLogic && m_fTimeWithoutLOS < 10.0f)	//aim if target has been within los in the last ten seconds
	{
		DoLeadAimWithError(5.0f, pWeaponCtrl->GetAccuracy(), s_pWD->EnemyMarkTagPos);
	}
}


void CGroundCombat::DoWeaponAimAndFireWork_Grunt(void)
{
	CAIWeaponCtrl* pWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
	if (pWeaponCtrl)
	{
		
		//
		// When and where to fire primary weapon
		//
		BOOL bDeadlyWeapon = FALSE;
		f32 fFireSafetyDist = -1.0f;
		if (s_pWD->pBot &&
			s_pWD->pBot->m_apWeapon[0] &&
			s_pWD->pBot->m_apWeapon[0]->TypeBits() & (ENTITY_BIT_WEAPONROCKET | ENTITY_BIT_WEAPONRIVET))
		{
			if (!m_pEnemy)
			{
				bDeadlyWeapon = TRUE;
			}
			else
			{

				fFireSafetyDist= 10.0f;
				FMATH_CLAMPMAX(fFireSafetyDist, s_pWD->fDistToEnemyMark);
				if (!s_pWD->pMover->CanAimPrimary())
				{
					if (aigroup_CountAlliesNearPt(s_pWD->pBot, s_pWD->pMover->GetLoc(), 20.0f)>1 || 
						(s_pWD->pBot->m_fSpeed_WS < 5.0f && s_pWD->fDistToEnemyMark > 15.0f))
					{
						bDeadlyWeapon = TRUE;
					}
					else
					{
						if (s_pWD->fDistToEnemyMark > 15.0f)
						{
							bDeadlyWeapon = TRUE;
						}
					}
				}
			}
		}

		if (!bDeadlyWeapon)
		{
			DoWeaponAimAndFireWork_Generic(fFireSafetyDist);
		}

		//
		// Throw grenades if enemy is within los 
		//
		CAIMemoryMedium* pNumGrenadesMem = NULL;
		if (GetBrain()->GetKnowledge().CanRememberAny(MEMORY_MEDIUM_NUM_GRENADES, (CAIMemorySmall**) &pNumGrenadesMem) &&
			pNumGrenadesMem->m_uSmallData > 0)
		{
			BOOL bGrenadeThisFrame = FALSE;

			if (!pWeaponCtrl->IsBursting() &&
				s_pWD->pEnemyBot &&
				s_pWD->fDistToEnemyMark > 10.0f &&
				s_pWD->fDistToEnemyMark < 100.0f &&
				((CBotGrunt*)s_pWD->pBot)->IsFingerOnTrigger2() &&
				(s_pWD->pMover->CanAimSecondary() || aigroup_CountEnemiesOf(m_pEnemy) < 2) &&
				!s_pWD->pBot->IsThrowing())
			{

				if (aigroup_CountAlliesNearPt(s_pWD->pMover->GetEntity(), s_pWD->EnemyMark, 10.0f) < 1)
				{
					if ((aiutils_GetCurTimeSecs() - m_fAttackThoughtInitTime < 0.25))
					{
						if ((pNumGrenadesMem->m_uMediumData & AI_GRENADE_USE_FIRST_CONTACT))
						{
							bGrenadeThisFrame = TRUE;
						}
					}
					else if (pNumGrenadesMem->m_uMediumData & AI_GRENADE_USE_ENEMYINSIGHT)
					{
						if (s_pWD->uNowTime != s_pWD->uLastFrameNowTime && 
							m_fTimeWithLOS > GetBrain()->GetInfreqCycleRandom()*1.0f+4.0f*(1.0f-s_pWD->fAggression) &&
							!GetBrain()->GetKnowledge().CanRememberAny(MEMORY_SMALL_GRENADETOSS))
						{
							bGrenadeThisFrame = TRUE;
							GetBrain()->GetKnowledge().RememberThis(MEMORY_SMALL_GRENADETOSS, 3+(u32)(GetBrain()->GetInfreqCycleRandom()*5.0f));
						}
					}
					
					if (!bGrenadeThisFrame && pNumGrenadesMem->m_uMediumData & AI_GRENADE_USE_ENEMY_EVASIVE)
					{
						if (s_pWD->pMover->CanSee() &&
							m_uLosStatusChangeCounter > 2+3*GetBrain()->GetWorldAddRandom() &&
							m_fTimeWithLOS < 0.25 && m_fTimeWithoutLOS < 0.25)
						{
							bGrenadeThisFrame = TRUE;
						}

					}

					if (!bGrenadeThisFrame)
					{
						if (pNumGrenadesMem->m_uMediumData & AI_GRENADE_USE_SELF_DEFENSE)
						{
							if (m_TacticFSM.GetActiveStateHandle() == TACTIC_PEEKABOO &&
								m_fDamageDealtPerSec > 0.2f &&
								this->m_fTimeWithLOS > 0.02f)
							{
								bGrenadeThisFrame = TRUE;
							}
						}
					}
					if (!bGrenadeThisFrame)
					{
						if (pNumGrenadesMem->m_uMediumData & AI_GRENADE_USE_RETURN_GRENADE_FIRE)
						{
							CAIMemoryLarge* pSeenMem = NULL;
							if (m_fTimeWithoutLOS < 1.0f &&
								GetBrain()->GetKnowledge().CanRememberAny(MEMORY_LARGE_SEEN_PROJECTILE, (CAIMemorySmall**) &pSeenMem))
							{
								bGrenadeThisFrame = TRUE;
							}
						}
					}
				}

				if ( bGrenadeThisFrame ) 
				{
					f32 fGrenadeThrowSpeed = 50.0f;

					if (s_pWD->fDistToEnemyMark > 80.0f && 
						GetBrain()->GetWorldAddRandom() > 0.4f &&
						!IsFollowingPath())
					{
						s_pWD->pMover->m_Controls.JumpHigh();
					}
					
					s_pWD->pMover->m_Controls.Fire2();
					s_pWD->pMover->m_Controls.AimAt(s_pWD->EnemyMark);
					pNumGrenadesMem->m_uSmallData--;
				}
			}
		}
	}
}



void CGroundCombat::DoWeaponAimAndFireWork_SiteGunner(void)
{
	if (m_fTimeWithoutLOS > 1.35f &&
		(s_pWD->pEnemyBot && !s_pWD->pEnemyBot->IsTargetable()))	   //probably this should e built into the lookatmgr
	{
		return;	  //no hard-aiming the pill.  because we should be scanning or something

	}

	CBot* pSite = NULL;
	if (((CBot*) s_pWD->pMover->GetEntity())->GetCurMech())
	{
		pSite = ((CBot*) s_pWD->pMover->GetEntity())->GetCurMech();
	}
	
	if( !pSite ||
		!pSite->AIBrain() ||
		(m_uAttackFlags & FLAG_PANIC_ON) ||
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsHopping() || ((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsRolling()) {
		return;
	}
	CAIBrain* pSiteBrain = pSite->AIBrain();;
	CAIMover* pSiteMover = pSiteBrain->GetAIMover();
	CAIWeaponCtrl* pWeaponCtrl = pSiteMover->GetWeaponCtrlPtr(0);

	if (!pWeaponCtrl)
	{
		return;
	}

	pWeaponCtrl->Work();	 //call this once a frame, before asking any questions of the weaponnctrl obo

	BOOL bNormalFire = FALSE;

	//if 
	//	 recently had LOS	&&
	//   BotLayer weapon system returned that a lock is met, which means the bullet will go within some degrees of where I told it to
	//then
	//   calculate some odds that a shot will be fired
	if (pWeaponCtrl->IsBursting())
	{
		bNormalFire = TRUE;
	}
	if (m_fTimeWithoutLOS < 1.0f && 
		1)//s_pWD->pMover->HasTargetLock()) // HasTargetLock means that the weapon will fire angle is within some internal tolerance to be considered a "lock" circa 20degrees
	{
		u32 uFireOdds = 100;
		if (m_uAttackFlags & FLAG_HASTARGETLOS)
		{
			uFireOdds = 90;
		}
		else
		{
			uFireOdds = 5;
			if (m_fTimeWithoutLOS < 3.0f)
			{
				uFireOdds += (u16) ((f32)80*m_fTimeWithoutLOS/3.0f);
			}
		}

		if (pWeaponCtrl->GetCtrlFlag_StopAndShoot() &&
			!(m_uAttackFlags & FLAG_TRIGGEROVERRIDE_STOPSHOOT) &&
			s_pWD->fDistToEnemyMark > 10.0f &&
			s_pWD->pMover->GetMoveSpeed() > 1.0f)
		{	
			uFireOdds = 0;
		}

		//being farther away from target decreases the desire to take a shot
		f32 fMaxDistRelatedDampening = 0.25f;	  //findfix: adjust these for different units possibly
		f32 fDistRelatedDampeningFeetThreshold = 50.0f;
		if (((CBot*) s_pWD->pMover->GetEntity())->GetCurMech()->IsRatGun())
		{
			fDistRelatedDampeningFeetThreshold = 150.0f;
		}
		fDistRelatedDampeningFeetThreshold += fDistRelatedDampeningFeetThreshold*s_pWD->fAggression; //how far away
		f32 fDistRelatedDampening = fMaxDistRelatedDampening;
		if (uFireOdds && s_pWD->fDistToEnemyMark < fDistRelatedDampeningFeetThreshold)
		{
			fDistRelatedDampening = fMaxDistRelatedDampening*s_pWD->fDistToEnemyMark/fDistRelatedDampeningFeetThreshold;
			fDistRelatedDampening += (1.0f - fDistRelatedDampening)*s_pWD->fAggression;
		}
		uFireOdds = (u8) (uFireOdds*fDistRelatedDampening);

		if (fmath_RandomChoice(100) < uFireOdds)
		{
			bNormalFire = TRUE;
		}
		else
		{
			bNormalFire = FALSE;
		}
	}
	else if (!m_pEnemy)
	{
		if (m_uAttackFlags & FLAG_CANTFINDENEMY &&
			m_pBrain->GetWorldAddRandom() < 0.77f)
		{
			if (s_pWD->uNowTime - (u16)m_fAttackThoughtInitTime < 2)
			{
				bNormalFire = TRUE;
			}
		}

		if (!bNormalFire &&
			s_pWD->fAggression > 0.75f &&
			s_pWD->pDamagedByMem &&
			s_pWD->uNowTime < m_uBlindResponseToDamageTimeOut && 
			(s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs< 2) )
		{
				bNormalFire = TRUE;
		}
	}
	else if (	m_pEnemy &&
				!(m_pEnemy->TypeBits() & ENTITY_BIT_BOT))
	{
		bNormalFire = TRUE;
	}

	BOOL bIsTalking = s_pWD->pMover->IsTalkingWithAnim();
	if ((bNormalFire || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
		pWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY) &&
		!bIsTalking)
	{  
		pWeaponCtrl->NotifyFire();
		s_pWD->pMover->m_Controls.Fire();
	}

	//AIM!
	if (m_fUpdateBadShotBonusVecTime  < aiutils_FTotalLoopSecs())
	{
		m_BadShotBonusUnit.x = fmath_RandomBipolarUnitFloat();
		m_BadShotBonusUnit.z = fmath_RandomBipolarUnitFloat();
		m_BadShotBonusUnit.y = fmath_RandomFloat(); // missing low,sucks//fmath_RandomBipolarUnitFloat();
		m_fUpdateBadShotBonusVecTime = aiutils_FTotalLoopSecs() + 0.2f+ fmath_RandomFloat();
	}

	f32 fAccuracy = 1.0f;
	fAccuracy *= pWeaponCtrl->GetAccuracy();
//	fAccuracy = 1.0f;
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

	if (s_pWD->fDistToEnemyMark < 5.0f &&
		m_uAttackFlags & FLAG_HASTARGETLOS)
	{
	   fAccuracy = 1.0f;
	}
	
	CFVec3A TargetLeadingBonusVec =  CFVec3A::m_Null;
	if (s_pWD->pBot->m_apWeapon[0])
	{	//can't lead if I can't find out projectile speed
		f32 fProjSpeed = ((CBot*) s_pWD->pMover->GetEntity())->m_apWeapon[0]->GetProjectileSpeed();
		if (fProjSpeed > 0.0f)
		{
			//cheap leading code
			f32 fTimeOfTravel = s_pWD->fDistToEnemyMark/fProjSpeed;
			TargetLeadingBonusVec = m_LastKnownVel;
			TargetLeadingBonusVec.Mul(fTimeOfTravel*fAccuracy);		//no accuracy means no leading
		}
	}

	if (m_LastAim == CFVec3A::m_Null)
	{
		m_LastAim = s_pWD->EnemyMark;
	}
	else
	{
		CFVec3A NewAim = s_pWD->EnemyMarkTagPos;
		f32 fPillAimLagPct = 0.1f+0.5f*fAccuracy;
		NewAim.Mul(fPillAimLagPct);
		m_LastAim.Mul(1.0f-fPillAimLagPct);
		m_LastAim.Add(NewAim);	  //m_LastAim approaches BadShotBunusVec.. findfix: fps dependent
	}

	FMATH_DEBUG_FCHECK(m_LastAim.x);
	FMATH_DEBUG_FCHECK(m_LastAim.y);
	FMATH_DEBUG_FCHECK(m_LastAim.z);

	s_pWD->pMover->m_Controls.AimAt(m_LastAim);
}


void CGroundCombat::DoWeaponAimAndFireWork_RatGun(void)
{
	DoWeaponAimAndFireWork_SiteGunner();
}


void CGroundCombat::DoWeaponAimAndFireWork_Sentinel(void)
{
	if ((m_uAttackFlags & FLAG_PANIC_ON) ||
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsHopping() ||
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsRolling())
	{
		return;
	}

	CAIWeaponCtrl* pChainWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(1);
	CAIWeaponCtrl* pCannonWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
	if (!(pChainWeaponCtrl && pCannonWeaponCtrl) )
	{
		return;
	}

	f32 _fIdealChainGunRangeMin = 15.0f;
	f32 _fIdealChainGunRangeMax = 90.0f;

	if (!s_pWD->pMover->CanFirePrimary() && !s_pWD->pMover->CanFireSecondary())
	{
		return;  //this titan has neither primary or secondary
	}


	pChainWeaponCtrl->Work();	 //call this once a frame, before asking any questions of the weaponnctrl obj
	pCannonWeaponCtrl->Work();

	BOOL bFireChain = FALSE;
	BOOL bFireCannon = FALSE;
	BOOL bCanFireRocketInNearFuture = TRUE;
	u32 bAimRocket = FALSE;
	if (pChainWeaponCtrl->IsBursting())
	{
		bFireChain = TRUE;
	}
	else if (s_pWD->fDistToEnemyMark >= _fIdealChainGunRangeMax)
	{
		bFireCannon = TRUE;
		bAimRocket = TRUE;
	}
	else if ((m_uFireOddsTimeOut < s_pWD->uNowTime) &&
			 ((s_pWD->fDistToEnemyMark > _fIdealChainGunRangeMin && s_pWD->fDistToEnemyMark < _fIdealChainGunRangeMax) ||
			 !bCanFireRocketInNearFuture))
	{
		m_uFireOddsTimeOut = s_pWD->uNowTime +1;

		if (m_fTimeWithoutLOS < 1.0f && 
			s_pWD->pMover->HasTargetLock()) // HasTargetLock means that the weapon will fire and the angle is within some internal tolerance to be considered a "lock" circa 20degrees
		{
			u32 uFireOdds = 100;
			if (m_uAttackFlags & FLAG_HASTARGETLOS)
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

	if (s_pWD->pMover->CanFirePrimary() &&
		(bFireChain || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
		pChainWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY))
	{  
		pChainWeaponCtrl->NotifyFire();
		s_pWD->pMover->m_Controls.Fire2();
	}

	if (s_pWD->pMover->CanFireSecondary() &&
		(bFireCannon || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
		pCannonWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY) &&
		s_pWD->pMover->HasTargetLock() &&
		m_pEnemy )
	{  
		pCannonWeaponCtrl->NotifyFire();

		CFVec3A RocketSafetyEndPt;
		RocketSafetyEndPt.Sub(this->m_LastAim, s_pWD->pMover->GetEyePos());
		if (RocketSafetyEndPt.SafeUnitAndMag(RocketSafetyEndPt) > 0.0f)
		{
			RocketSafetyEndPt.Mul(kfSentinelRocketSafetyRange);
			RocketSafetyEndPt.Add(s_pWD->pMover->GetEyePos());
			if (!aiutils_IsLOSObstructed_IgnoreBots(s_pWD->pMover->GetEyePos(), RocketSafetyEndPt, s_pWD->pBot))
			{
				s_pWD->pMover->m_Controls.Fire();
				aiutils_DebugTrackRay(s_pWD->pMover->GetEyePos(), RocketSafetyEndPt, TRUE);
			}
			else
			{
				aiutils_DebugTrackRay(s_pWD->pMover->GetEyePos(), RocketSafetyEndPt, FALSE);
			}
		}
	}

	DoLagWeaponAiming(0);
}

void CGroundCombat::DoWeaponAimAndFireWork_Rat(void)
{
	//nothing to do.  Rat driver doesn't operate guns
}


void CGroundCombat::DoWeaponAimAndFireWork_Loader(void)
{
	if ((m_uAttackFlags & FLAG_PANIC_ON) ||
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsHopping() ||
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->IsRolling())
	{
		return;
	}

	CAIWeaponCtrl* pClawWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(0);
	CAIWeaponCtrl* pLGunWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(1);
	if (!(pLGunWeaponCtrl && pClawWeaponCtrl) )
	{
		return;
	}

	f32 _fIdealLGunGunRangeMin = 5.0f;
	f32 _fIdealLGunGunRangeMax = 150.0f;

	if (!(s_pWD->pMover->CanJump() || !s_pWD->pMover->CanMove()))
	{
		_fIdealLGunGunRangeMin = s_pWD->pMover->GetRadiusXZ();	   //this might chafe a bit with titan since he can't aim that close up..
		return;  //this titan has neither primary or secondary
	}


	pLGunWeaponCtrl->Work();	 //call this once a frame, before asking any questions of the weaponnctrl obj
	pClawWeaponCtrl->Work();

	BOOL bFireLGun = FALSE;
	BOOL bFirePinch = FALSE;
	BOOL bCanFireRocketInNearFuture = TRUE;
	u32 bAimRocket = FALSE;
	if (pLGunWeaponCtrl->IsBursting())
	{
		bFireLGun = TRUE;
	}
	else if (s_pWD->fDistToEnemyMark <m_uMeleeRad)
	{
		bFirePinch = TRUE;
		bAimRocket = TRUE;
	}
	else if ((m_uFireOddsTimeOut < s_pWD->uNowTime) &&
			 ((s_pWD->fDistToEnemyMark > _fIdealLGunGunRangeMin && s_pWD->fDistToEnemyMark < _fIdealLGunGunRangeMax) ||
			 !bCanFireRocketInNearFuture))
	{
		m_uFireOddsTimeOut = s_pWD->uNowTime +1;

		if (m_fTimeWithoutLOS < 1.0f && 
			s_pWD->pMover->HasTargetLock()) // HasTargetLock means that the weapon will fire and the angle is within some internal tolerance to be considered a "lock" circa 20degrees
		{
			u32 uFireOdds = 100;
			if (m_uAttackFlags & FLAG_HASTARGETLOS)
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
			f32 fFireOddsDampeningInnerRad = _fIdealLGunGunRangeMin;
			f32 fFireOddsDampeningOutterRad = _fIdealLGunGunRangeMax;
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
				bFireLGun = TRUE;
			}
		}
		else if (!m_pEnemy)
		{
			if (m_uAttackFlags & FLAG_CANTFINDENEMY &&
				m_pBrain->GetWorldAddRandom() < 0.77f)
			{
				if (s_pWD->uNowTime - (u16)m_fAttackThoughtInitTime < 2)
				{
					bFireLGun = TRUE;
				}
			}

			if (!bFireLGun &&
				s_pWD->fAggression > 0.75f &&
				s_pWD->pDamagedByMem &&
				s_pWD->uNowTime < m_uBlindResponseToDamageTimeOut && 
				(s_pWD->uNowTime - s_pWD->pDamagedByMem->m_uWhenTimeSecs< 2) )
			{
				bFireLGun = TRUE;
			}
		}
		else if (m_pEnemy &&
				!(m_pEnemy->TypeBits() & ENTITY_BIT_BOT))
		{
			bFireLGun = TRUE;
		}
	}

	if (s_pWD->pMover->CanFirePrimary() &&
		(bFireLGun || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
		pLGunWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY))
	{  
		pLGunWeaponCtrl->NotifyFire();
		s_pWD->pMover->m_Controls.Fire2();
	}

	if (s_pWD->pMover->CanFireSecondary() &&
		(bFirePinch || (m_uAttackFlags & FLAG_TRIGGERHAPPY)) &&
		pClawWeaponCtrl->IsReady(m_uAttackFlags & FLAG_TRIGGERHAPPY) &&
		s_pWD->pMover->HasTargetLock() &&
		m_pEnemy )
	{  
		pClawWeaponCtrl->NotifyFire();
		s_pWD->pMover->m_Controls.Fire();
	}

	DoLagWeaponAiming(0);
}


void CGroundCombat::DoLeadAimWithError(f32 fMaxErrorDist, f32 fBaseAccuracy, const CFVec3A& ErrorOrigin)
{
	if (m_fUpdateBadShotBonusVecTime  < aiutils_FTotalLoopSecs())
	{
		m_BadShotBonusUnit.x = fmath_RandomBipolarUnitFloat();
		m_BadShotBonusUnit.z = fmath_RandomBipolarUnitFloat();
		m_BadShotBonusUnit.y = fmath_RandomFloat(); // missing low,sucks//fmath_RandomBipolarUnitFloat();
		m_fUpdateBadShotBonusVecTime = aiutils_FTotalLoopSecs() + 0.2f+ fmath_RandomFloat();
	}

	f32 fAccuracy = fBaseAccuracy;
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
	
	f32 fProjSpeed = -1.0f;
	CFVec3A TargetLeadingBonusVec = CFVec3A::m_Null;
	if (s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTELITEGUARD)
	{
		fProjSpeed = ((CBotEliteGuard*) s_pWD->pBot)->GetApproxPrimaryWeaponSpeed();
	}
	else if (((CBot*) s_pWD->pMover->GetEntity())->m_apWeapon[0])
	{
		f32 fProjSpeed = ((CBot*) s_pWD->pMover->GetEntity())->m_apWeapon[0]->GetProjectileSpeed();
	}

	if (fProjSpeed > 0.0f)
	{
		//cheap leading code
		f32 fTimeOfTravel = s_pWD->fDistToEnemyMark/fProjSpeed;
		TargetLeadingBonusVec = m_LastKnownVel;
		TargetLeadingBonusVec.Mul(fTimeOfTravel*fAccuracy);		//no accuracy means no leading
	}

	CFVec3A BadShotBonusVec;
	BadShotBonusVec = m_BadShotBonusUnit;
	f32 fBadShotBonusVecScaleXZ = fMaxErrorDist*(1.0f-fAccuracy);		 //potentially miss by six feet
	BadShotBonusVec.Mul(fBadShotBonusVecScaleXZ);	
	BadShotBonusVec.Add(ErrorOrigin);// *(m_pEnemy->GetTagPoint(uTagToShootAt))); cheating!
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


void CGroundCombat::DoLagWeaponAiming(u8 uWeaponId)
{
	CAIWeaponCtrl* pChainWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(uWeaponId);
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
	if (m_LastAim == CFVec3A::m_Null)
	{
		m_LastAim = s_pWD->EnemyMark;
	}
	else
	{
		f32 fLag = 0.0f;
		if (m_fTimeWithLOS < 2.5f)
		{
			fLag = fmath_Div(2.5f-m_fTimeWithLOS, 2.5f);
		}
		else
		{
			fLag = 0.0f;
		}

		f32 fLerp = fLag*(0.02f+0.18f*fLag+0.2f*fAccuracy);
		CFVec3A NewAim = s_pWD->EnemyMarkTagPos;
		NewAim.Mul(1.0f-fLerp);
		m_LastAim.Mul(fLerp);
		m_LastAim.Add(NewAim);	  //m_LastAim approaches BadShotBunusVec.. findfix: fps dependent
	}

	FMATH_DEBUG_FCHECK(m_LastAim.x);
	FMATH_DEBUG_FCHECK(m_LastAim.y);
	FMATH_DEBUG_FCHECK(m_LastAim.z);

	s_pWD->pMover->m_Controls.AimAt(m_LastAim);
}


void CGroundCombat::Titan_RocketAiming()
{
	CAIWeaponCtrl* pRocketWeaponCtrl = s_pWD->pMover->GetWeaponCtrlPtr(1);
	if (!pRocketWeaponCtrl)
	{
		return;
	}

	if (m_fUpdateBadShotBonusVecTime  < aiutils_FTotalLoopSecs())
	{
		m_BadShotBonusUnit.x = fmath_RandomBipolarUnitFloat();
		m_BadShotBonusUnit.z = fmath_RandomBipolarUnitFloat();
		m_BadShotBonusUnit.y = fmath_RandomBipolarUnitFloat();   //missing low, isn't too bad
		m_fUpdateBadShotBonusVecTime = aiutils_FTotalLoopSecs() + 0.2f + fmath_RandomFloat();
	}

	f32 fAccuracy = 1.0f;
	fAccuracy *= pRocketWeaponCtrl->GetAccuracy();
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
	
	CFVec3A TargetLeadingBonusVec;
	u32 uTitanRocketSlot = 1;
	f32 fProjSpeed = 0.0f;
	if (s_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTTITAN)
	{
		fProjSpeed = ((CBotTitan*)s_pWD->pMover->GetEntity())->GetRocketTravelSpeed();
	}
	if (fProjSpeed > 0.0f)
	{
		//cheap leading code
		f32 fTimeOfTravel = 0.1f+s_pWD->fDistToEnemyMark/fProjSpeed;   //the .1 is due to an internal rocket fire delay.  findfix: should come from titan structure somewhere
		TargetLeadingBonusVec = m_LastKnownVel;
		TargetLeadingBonusVec.Mul(fTimeOfTravel);		//no accuracy means no leading
	}

	CFVec3A BadShotBonusVec;
	BadShotBonusVec = m_BadShotBonusUnit;
	f32 fBadShotBonusVecScaleXZ = 5.0f*(1.0f-fAccuracy);		 //potentially miss by six feet
	if (!(m_uAttackFlags & FLAG_HASTARGETLOS))
	{
		fBadShotBonusVecScaleXZ += m_fTimeWithoutLOS;	//every second, increase the error in rocket aim
	}
	BadShotBonusVec.Mul(fBadShotBonusVecScaleXZ);	

	if (GetBrain()->GetAttrib(CAIBrain::ATTRIB_INTELLIGENCE) == 100)
	{
		BadShotBonusVec.Add(s_pWD->EnemyMark);  //Titan aims at the feet since there is splash damage
	}
	else
	{
		BadShotBonusVec.Add(s_pWD->EnemyMarkTagPos);  //Titan aims at the feet since there is splash damage
	}
	BadShotBonusVec.Add(TargetLeadingBonusVec);

	if (m_LastAim == CFVec3A::m_Null)
	{
		m_LastAim = BadShotBonusVec;
	}
	else
	{
		m_LastAim.Add(BadShotBonusVec);	  //m_LastAim approaches BadShotBunusVec.. findfix: fps dependent
		m_LastAim.Mul(0.5f);
	}

	FMATH_DEBUG_FCHECK(m_LastAim.x);
	FMATH_DEBUG_FCHECK(m_LastAim.y);
	FMATH_DEBUG_FCHECK(m_LastAim.z);

	s_pWD->pMover->m_Controls.AimAt(m_LastAim);
}


void CGroundCombat::DoFollowPathWork(void)
{

	if (m_uAttackFlags & FLAG_COMBAT_CANT_MOVE_BOT)
	{   //sorry, the combat thought is not the thought that is responsible for moving the bot around right now
		return;
	}

	if (m_uAttackFlags & FLAG_NOPATH)
	{
		//stay put, just incase I don't give you a path
		if (m_uAttackFlags & FLAG_PANIC_ON)
		{	//crazy mode means never stop slamming the controller, even if up against a wall
			CFVec3A tmp;
			tmp = s_pWD->pMover->GetTorsoLookAtXZ();
			tmp.Mul(s_pWD->pMover->GetRadiusXZ()*5.0f);
			tmp.Add(s_pWD->pMover->GetLoc());
			s_pWD->pMover->MoveToward(tmp);
			if (s_pWD->pBot->m_fSpeed_WS < 4.0f && !s_pWD->pMover->IsStuck(1.0f))
			{
				tmp.x = s_pWD->pMover->GetTorsoLookAtXZ().z;
				tmp.y = 0.0f;
				tmp.z = -s_pWD->pMover->GetTorsoLookAtXZ().x;
				if (GetBrain()->GetWorldAddRandom() > 0.5f)
				{
					tmp.Mul(-1.0f);
				}
				tmp.Mul(s_pWD->pMover->GetRadiusXZ()*5.0f);
				tmp.Add(s_pWD->pMover->GetLoc());

				s_pWD->pMover->FaceToward(tmp);

			}
		}
		else if (!s_pWD->pMover->IsFollowingPath() && !(m_uAttackFlags & FLAG_MANUAL_MOVE_CONTROL))
		{
			s_pWD->pMover->MoveToward(s_pWD->pMover->GetLoc());
		}
		m_bPathJustCompleted = FALSE;
	}
	else
	{
		BOOL bFollowPathDone = s_pWD->pMover->FollowPath();	//returns true if there is no path, or the path has been completed!

		if (bFollowPathDone && !(m_uAttackFlags & FLAG_NOPATH))
		{
			m_bPathJustCompleted = TRUE;
			m_nLastPathReason = m_anSearchReason[m_nCurPathRequestSlot];
		}
		else
		{
			m_bPathJustCompleted = FALSE;
		}
		if (bFollowPathDone)
		{
			m_nCurPathRequestSlot = SEARCHREASON_INVALID;
			m_uAttackFlags |= FLAG_NOPATH;
			s_pWD->pMover->AssignPath(NULL);
		}
		else
		{
			m_uAttackFlags &= ~FLAG_NOPATH;
		}
	}
}


BOOL CGroundCombat::DoEnemyRelatedExitConditions(void)
{
	BOOL bFailure = FALSE;
	BOOL bSuccess = FALSE;
	BOOL bExit = FALSE;

	if (!m_pEnemy && 
		(s_pWD->uNowTime - m_uLostEnemyTime > 5))
	{
		bFailure = TRUE;
	}

	if (m_pEnemy &&	!m_pEnemy->IsInWorld())
	{
		m_uThoughtFlags |= THOUGHTFLAG_GOAL_COMPLETE; //must have killed him
	}

	if (s_pWD->pEnemyBot && s_pWD->pEnemyBot->IsPlayerOutOfBody())
	{
		//findfix: would like to add knowlege function that removes all memories related to a specific entity
		bSuccess = TRUE;
	}

	if (s_pWD->pEnemyBot && this->m_nPossessionPlayerIndexOfEnemy != s_pWD->pEnemyBot->m_nPossessionPlayerIndex)
	{
		//findfix: would like to add knowlege function that removes all memories related to a specific entity

		bSuccess = TRUE;
	}

	if (s_pWD->pEnemyBot && aiutils_IsMech(s_pWD->pEnemyBot) && !aiutils_GetMechOperator(s_pWD->pEnemyBot))
	{
		bSuccess = TRUE;
	}

	if (m_pEnemy && m_pEnemy->AIBrain() && m_uEnemyRace != m_pEnemy->AIBrain()->GetRace())
	{
		//enemy has changed race... let's give up on attacking him. 
		//if he is still my enemy, then let AIBrain reaction system re-trigger attack mode.
		//if he isn't then don't
		bSuccess = TRUE;
	}

	if (m_pEnemy &&
		(( s_pWD->pEnemyBot && ( s_pWD->pEnemyBot->m_nPossessionPlayerIndex == -1 ) && aiutils_IsDeadOrDying(m_pEnemy)) ||
		( s_pWD->pEnemyBot && ( s_pWD->pEnemyBot->m_nPossessionPlayerIndex >= 0 ) && s_pWD->pEnemyBot->IsDead() ) ) )
	{
		bSuccess = TRUE;
	}


	if (bFailure)
	{
		m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;	 //couldn't find that guy!
		bExit = TRUE;
	}
	else if (bSuccess)
	{
		m_uThoughtFlags |= THOUGHTFLAG_GOAL_COMPLETE; //must have killed him
		bExit = TRUE;
	}

	if (bExit)
	{
		int i = 3;
		i++;
	}
	return bExit;
}


void CGroundCombat::Work(void)
{
	//
	//Gather Attack Work Context
	//
	AttackWorkBeginFrame();

	//
	// A few exit conditions related to our enemy
	//
	if (!( m_uAttackFlags & FLAG_DISABLE_ENEMY_RELATED_EXIT_CONDITIONS) &&
		!m_RuleSetFSM.IsStateChangePending())
	{
		if (DoEnemyRelatedExitConditions())
		{
			return; //get out now, since attack is going away anyway
		}
	}

	//pump the paths in progress slots
	for (u32 uSlot = 0; uSlot < NUM_PATH_REQUEST_SLOTS; uSlot++)
	{
		if (m_aSearchQuery[uSlot].IsDone() && !m_aSearchQuery[uSlot].HasBeenDone())
		{
			m_aSearchQuery[uSlot].SetHasBeenDone();
		}
		else if (m_aSearchQuery[uSlot].HasFailed())
		{
			uSlot = uSlot;
		}
	}

	//
	// Do current tactic work
	//
#ifdef _DEBUG
	FSMStateHandle uTactic = m_TacticFSM.GetActiveStateHandle();
	cchar * pszTacticName = m_TacticFSM.GetActiveStateName();
	switch (uTactic)
	{
	case FSMSTATEHANDLE_INVALID:
			break;
		case CGroundCombat::TACTIC_NONE:
			break;
		case CGroundCombat::TACTIC_STANDGROUND:
			break;
		case CGroundCombat::TACTIC_RANGEATTACK:
			break;
		case CGroundCombat::TACTIC_CIRCLESTRAFE:
			break;
		case CGroundCombat::TACTIC_PEEKABOO:
			break;
		case CGroundCombat::TACTIC_SEARCH:
			break;
		case CGroundCombat::TACTIC_RANGE3D:
			break;
		default:
			FASSERT(0);
			break;
	}
#endif
	m_TacticFSM.Work((void*) &WD);

	//
	// If Mover is following a path, it is our job to call CAIMover::FollowPath
	//
	DoFollowPathWork();

	//
	// Make Decisions
	//		   NOTE!  RuleSet work funcs should call their own rulesetstate fsm's work
	//
	DoRuleSet();

#if !FANG_PRODUCTION_BUILD
	f32 fTmp4 =10.0f;		//testing the FPu, making sure it works.
	fTmp4-=10.0f;
#endif

	//
	//  Control the Torso and Head Direction
	//
	DoTorsoAndHeadWork();

#if !FANG_PRODUCTION_BUILD
	f32 fTmp3 =10.0f;		//testing the FPu, makeing sure it works.
	fTmp3-=10.0f;
#endif			

	//
	// Aim and fire weapon
	//
	DoWeaponAimAndFireWork();

#if !FANG_PRODUCTION_BUILD
	f32 fTmp =10.0f;		//testing the FPu, makeing sure it works.
	fTmp-=10.0f;
#endif


	//
	//	End Combat for this attack work
	//
	AttackWorkEndFrame();
}





//Position is Enemy Relative in the XZ, but CurRel in the Y
BOOL CGroundCombat::FindEnemyRelPos_3D(CFVec3A *pResult,
									   const CFVec3A& EnemyPos,
									   const CFVec3A& CurPos,
									   f32 fXZDist,
									   f32 fYOff,
									   u32 uRelDir,
									   BOOL bCheckLOSToEnemy)
{
	CFVec3A VecToEnemyXZ;
	CFVec3A BumpPos;
	CFVec3A BumpPosAtGoal;
	f32 fTmp;
	f32 fDistToTarg;
	BOOL bHasLOS = FALSE;
	f32 fYBump = 3.0f;
	f32 fMinAttackPosDistToTarg = 3.0f;	 //findfix: configure this per bot

	VecToEnemyXZ.Sub(EnemyPos, CurPos);
	f32 fDY = VecToEnemyXZ.y;
	VecToEnemyXZ.y = 0.0f;

	FASSERT(pResult);

	fDistToTarg = VecToEnemyXZ.SafeUnitAndMagXZ(VecToEnemyXZ);
	if (fDistToTarg > 0.0f)
	{
		if (fXZDist < fMinAttackPosDistToTarg && FMATH_FABS(fDY) < fMinAttackPosDistToTarg)
		{
			fXZDist = fMinAttackPosDistToTarg;
		}

		if (uRelDir == RELATIVE_DIR_LEFT || uRelDir == RELATIVE_DIR_RIGHT)
		{
			//2D rotate
			fTmp = VecToEnemyXZ.z;
			VecToEnemyXZ.z = -VecToEnemyXZ.x;
			VecToEnemyXZ.x = fTmp;
			
			VecToEnemyXZ.Mul(fXZDist * (1.0f - (2*uRelDir)));
		}
		else
		{	 //toward / away
			VecToEnemyXZ.Mul(fXZDist * (1.0f - (2*(RELATIVE_DIR_AWAY-uRelDir))));
		}
		pResult->Add(VecToEnemyXZ, EnemyPos);
		pResult->y += fYBump + fYOff;

		if (bCheckLOSToEnemy)
		{
			BumpPos = *pResult;
			BumpPosAtGoal = EnemyPos;
			bHasLOS = !aiutils_IsLOSObstructed_IgnoreBots( BumpPos, BumpPosAtGoal, s_pWD->pEnemyBot);
			aiutils_DebugTrackRay(BumpPos, BumpPosAtGoal, bHasLOS);
			return bHasLOS;
		}
		return TRUE;
	}

	return FALSE;
}


BOOL CGroundCombat::FindCurRelPos_3D(CFVec3A *pResult,
									 const CFVec3A& EnemyPos,
									 const CFVec3A& CurPos,
									 f32 fLocalDist,
									 f32 fYOff,
									 u32 uLocalDir,
									 BOOL bCheckLOSToEnemy,
									 f32 fMinDistToEnemy,
									 f32 fMaxDistToEnemy)
{
	CFVec3A BumpPosAtGoal;
	BOOL bHasLOS = FALSE;
	if ((uLocalDir == RELATIVE_DIR_TOWARD || uLocalDir == RELATIVE_DIR_AWAY) || m_pBrain->GetInfreqCycleRandom() > 0.5f)
	{
		CFVec3A VecToEnemy;
		CFVec3A LocalDelta;
		LocalDelta.Zero();
		f32 fDistToTargXZ;

		VecToEnemy.Sub(EnemyPos, CurPos);
		VecToEnemy.y = 0.0f;

		FASSERT(pResult);

		fDistToTargXZ = VecToEnemy.SafeUnitAndMagXZ(VecToEnemy);
		if (fDistToTargXZ > 0.0f)
		{
			f32 fNewDistToTargXZ;
			if (uLocalDir == RELATIVE_DIR_LEFT || uLocalDir == RELATIVE_DIR_RIGHT)
			{
				//90 degree rotate about Y
				LocalDelta.z = -VecToEnemy.x;
				LocalDelta.x = VecToEnemy.z;
				LocalDelta.Mul(fLocalDist * (1.0f - (2*uLocalDir)));

				fNewDistToTargXZ = fDistToTargXZ;
			}
			else
			{	//dir = toward or away
				if (uLocalDir == RELATIVE_DIR_TOWARD)
				{
					fNewDistToTargXZ  = fDistToTargXZ - fLocalDist;
				}
				else if (uLocalDir == RELATIVE_DIR_AWAY)
				{
					fNewDistToTargXZ  = fDistToTargXZ + fLocalDist;
				}

				FMATH_CLAMPMIN(fLocalDist, 0.0f);
				LocalDelta = VecToEnemy;
				LocalDelta.Mul(fLocalDist * (1.0f - (2*(uLocalDir-2))));
			}

			if (fMaxDistToEnemy >= 0.0f && fMinDistToEnemy >= 0.0f)
			{
				f32 fAdjustDist = 0.0f;
				if (fNewDistToTargXZ < fMinDistToEnemy)
				{ //move back
					fAdjustDist = fMinDistToEnemy-fNewDistToTargXZ;
				}
				if (fNewDistToTargXZ > fMaxDistToEnemy)
				{
					fAdjustDist = fNewDistToTargXZ-fMaxDistToEnemy;
				}
				VecToEnemy.Mul(fAdjustDist);
				LocalDelta.Add(VecToEnemy);
			}

			pResult->Add(LocalDelta, CurPos);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		*pResult = CurPos;
		pResult->y += (fLocalDist * (1.0f - (2*uLocalDir)));
	}

	if (bCheckLOSToEnemy)
	{
		BumpPosAtGoal = EnemyPos;
		BumpPosAtGoal.y += 2.0f;
		bHasLOS = !aiutils_IsLOSObstructed_IgnoreBots( *pResult, BumpPosAtGoal, s_pWD->pBot);
		aiutils_DebugTrackRay(*pResult, BumpPosAtGoal, bHasLOS);
		return bHasLOS;
	}
	return TRUE;
}


BOOL CGroundCombat::FindLocalPoi_3D(CFVec3A *pResult,
									CGraphPoi** pPoiToLockIfUsed,
									const CFVec3A& EnemyPos,
									const CFVec3A& CurPos,
									f32 fMinRad,
									f32 fMaxRad,
									f32 fYOff,
									BOOL bCheckLOSToEnemy)
{
	return FALSE;
}


BOOL CGroundCombat::FindCoverPos_3D(CFVec3A *pResult,
									const CFVec3A& EnemyPos,
									const CFVec3A& CurPos,
									f32 fXZRot,
									f32 fYOff,
									f32 fTestDist)
{
	CFVec3A TestPos;
	CFVec3A DeltaToEnemyUnitXZ;
	f32 fDistToEnemyXZ;
	CFVec3A vStart, vEnd;
	BOOL bLOS = TRUE;
	u32 uCount = 0;
	CFVec3A EnemyPosBump = EnemyPos;
	EnemyPosBump.y += 2.0f;

	DeltaToEnemyUnitXZ.Sub(EnemyPosBump, CurPos);
	DeltaToEnemyUnitXZ.y = 0.0f;
	fDistToEnemyXZ = DeltaToEnemyUnitXZ.SafeUnitAndMag(DeltaToEnemyUnitXZ);
	if (fDistToEnemyXZ > 0.0f)
	{
		while (bLOS && uCount < 2)
		{
			uCount++;

			fXZRot *= (1.0f - (f32)((uCount+s_pWD->uNowTime)&1)*2.0f);		  //check both pos and neg rotations of the cover vec in one function call
			TestPos = DeltaToEnemyUnitXZ;
			TestPos.RotateY(fXZRot);
			TestPos.Mul(fTestDist);
			TestPos.Add(CurPos);
			bLOS = !aiutils_IsLOSObstructed_IgnoreBots(EnemyPosBump,  TestPos, s_pWD->pEnemyBot);   //If I were to move to TestPos, would enemy be able to see me?
    		aiutils_DebugTrackRay(EnemyPosBump, TestPos, bLOS);   //If I were to move to TestPos, would enemy be able to see me?
			if (!bLOS && pResult)
			{
				*pResult = TestPos;
			}
		}
		return !bLOS;
	}
	return FALSE;
}


BOOL CGroundCombat::FindRetreatPos_3D(CFVec3A *pResult,
									  const CFVec3A& From,
									  const CFVec3A& DeltaToEnemyUnit,
									  f32 fDist,
									  f32 fYoff)
{
	CFVec3A DeltaToEnemyUnitXZ;
	f32 fDistToEnemyXZ;

	DeltaToEnemyUnitXZ = DeltaToEnemyUnit;
	DeltaToEnemyUnitXZ.y = 0.0f;
	fDistToEnemyXZ = DeltaToEnemyUnitXZ.SafeUnitAndMag(DeltaToEnemyUnitXZ);

	if (fDistToEnemyXZ < 0.0f)
	{
		DeltaToEnemyUnitXZ = CFVec3A::m_UnitAxisX;
		fDistToEnemyXZ = 1.0f;
	}

	pResult->Set(DeltaToEnemyUnitXZ);
	pResult->Mul(-fDist);
	pResult->Add(From);

	return TRUE;
}


BOOL CGroundCombat::FindAdvancePos_3D(CFVec3A *pResult,
									  const CFVec3A& From,
									  const CFVec3A& DeltaToEnemyUnit,
									  f32 fDist,
									  f32 fYoff)
{

	pResult->Set(DeltaToEnemyUnit);
	pResult->Mul(fDist);
	pResult->Add(From);
	return TRUE;
}

BOOL CGroundCombat::FindRangeAcquirePos_3D(CFVec3A *pResult,
										   const CFVec3A& EnemyPos,
										   f32 fYOff)
{
	*pResult = EnemyPos;
	(*pResult).y = s_pWD->pMover->GetLoc().y;
	return TRUE;
}

void CGroundCombat::DoGenericTauntWork(void)
{
	CGroundCombatWorkContext* _pWD = s_pWD;
	CAIBrain* pBrain = GetBrain();

	cchar* pszBTAName = NULL;
	u32 uAllies = aigroup_CountEnemiesOf(m_pEnemy);

	// 	  just went on attack  and
	//	  I'm not the first one on pT attack and
	//	  there is a player n enemy
	//	  and can see enemy
    //	  haven't taken any damage
	f32 fTimeSinceNewAttack = aiutils_GetCurTimeSecs() - m_fAttackThoughtInitTime;
	if (	!pBrain->IsTalking() &&
			(fTimeSinceNewAttack > 0.25f && fTimeSinceNewAttack < 0.5f) &&
			s_pWD->pEnemyBot && 
			s_pWD->pEnemyBot->m_nPossessionPlayerIndex != -1 &&
			(m_uAttackFlags & CGroundCombat::FLAG_HASTARGETLOS) &&
			uAllies > 1 && 
			!s_pWD->pDamagedByMem &&
			(pszBTAName = AIBTA_EventToBTAName("P*E_SEES", pBrain)))
	{
		if (ai_TalkModeBegin(pBrain,
						AIBrain_TalkModeCB,
						(void*) pszBTAName,
						pBrain->GetAIMover()->GetEntity(),
						kuAttackMOdeTalkRequestTimeOutSecs,
						TRUE, //STOP
						FALSE, //DON"T LOOK
						0))
		{
			//did it
			int i =2;
		}
	} else if (!pBrain->IsTalking() &&
		!(m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON)  &&
		(AIBTA_GroupLaughTime(pBrain) || (m_uTauntTimeOut < _pWD->uNowTime)) &&
		((m_fDamageDealtPerSec > 0.10f && m_fDamageTakenPerSec < 0.3f) || (m_fDamageTakenPerSec < 0.4f && m_fEnemyDamageTakenPerSec > 0.10f)) &&   //taunt
		s_pWD->pEnemyBot && 
		s_pWD->pEnemyBot->m_nPossessionPlayerIndex != -1 &&
		(m_fTimeWithoutLOS < 0.7f))
	{
		//lay down a taunt, wer're kicking butt and not taking much damage
		if (s_pWD->fDistToEnemyMark > 0.0f &&
			s_pWD->fDistToEnemyMark < (50.0f+s_pWD->pMover->GetRadiusXZ()) &&
			m_pEnemy->MtxToWorld()->m_vFront.Dot(s_pWD->DeltaToEnemyMarkUnit) < -0.75f)
		{
			pszBTAName = AIBTA_EventToBTAName("P*E_TAUN", pBrain);
		}

		if (!pszBTAName)
		{
			pszBTAName = AIBTA_EventToBTAName("P*E_LAFF", pBrain);
		}

		if (pszBTAName)
		{
			if (ai_TalkModeBegin(pBrain,
							AIBrain_TalkModeCB,
							(void*) pszBTAName,
							pBrain->GetAIMover()->GetEntity(),
							kuAttackMOdeTalkRequestTimeOutSecs,
							TRUE, //STOP
							FALSE, //DON"T LOOK
							0))
			{
				if (m_uTauntTimeMax > m_uTauntTimeMin)
				{
					m_uTauntTimeOut =  aiutils_GetCurTimeSecsU16() + (u16) (m_uTauntTimeMin + (u8) fmath_RandomChoice(m_uTauntTimeMax - m_uTauntTimeMin));
				}
			}
		}
		else
		{
			m_uTauntTimeOut =  aiutils_GetCurTimeSecsU16() + (u16) (1 + (u8) fmath_RandomChoice(2));
		}
	}
	else if (	m_fDamageTakenPerSec >= 0.1f &&  //stops taunting
				pBrain->GetAIMover()->IsTalkingWithAnim() &&
				_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT &&
				((CBot*) _pWD->pMover->GetEntity())->m_pActiveTalkInst &&
				AIBTA_IsBTATaunt(((CBot*) _pWD->pMover->m_pEntity)->m_pActiveTalkInst->GetTalkData()->m_pszSourceFileName))
	{
		ai_TalkModeEnd(pBrain);
	}
}



// heart is a baloon, shoots up throught the stoney ground, there is no room no space to rent in this town, your out of luck, and the reason that you had to care, the traffic is stuck, and your not moving anywhere,

CNiBank<CGroundSearch>* CGroundSearch::s_pAllocBank = NULL;
CNiBank<CPillWait>* CPillWait::s_pAllocBank = NULL;
CNiBank<CGroundCombat>* CGroundCombat::s_pAllocBank = NULL;
CNiBank<CScoutAlert>* CScoutAlert::s_pAllocBank = NULL;


BOOL CGroundSearch::InitBank(s32 nHowMany)
{
	if (!CGroundSearch::s_pAllocBank)
	{
		CGroundSearch::s_pAllocBank = APE_NEW CNiBank<CGroundSearch>(aimain_pNodePool, nHowMany, APE_NEW CGroundSearch[nHowMany]);	 //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CGroundSearch::s_pAllocBank != NULL;
}


void CGroundSearch::CleanupBank(void)
{
	APE_DELETE( CGroundSearch::s_pAllocBank); CGroundSearch::s_pAllocBank = NULL;
}


CAIThought* CGroundSearch::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CGroundSearch::s_pAllocBank->Recycle((CGroundSearch *) pRecycle);
	}
	else
	{
		return CGroundSearch::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CGroundSearch::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGROUNDSEARCH) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGroundSearch::GetClassInterface(void)
{
	return CLASS_CGROUNDSEARCH;
}


BOOL CPillWait::InitBank(s32 nHowMany)
{
	if (!CPillWait::s_pAllocBank)
	{
		CPillWait::s_pAllocBank = APE_NEW CNiBank<CPillWait>(aimain_pNodePool, nHowMany, APE_NEW CPillWait[nHowMany]);	  //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CPillWait::s_pAllocBank != NULL;
}


void CPillWait::CleanupBank(void)
{
	APE_DELETE( CPillWait::s_pAllocBank); CPillWait::s_pAllocBank = NULL;
}


CAIThought* CPillWait::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CPillWait::s_pAllocBank->Recycle((CPillWait *) pRecycle);
	}
	else
	{
		return CPillWait::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CPillWait::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CPILLWAIT) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CPillWait::GetClassInterface(void)
{
	return CLASS_CPILLWAIT;
}


BOOL CGroundCombat::InitBank(s32 nHowMany)
{
	if (!CGroundCombat::s_pAllocBank)
	{
		CGroundCombat::s_pAllocBank = APE_NEW CNiBank<CGroundCombat>(aimain_pNodePool, nHowMany, APE_NEW CGroundCombat[nHowMany]);		//note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}

	return CGroundCombat::s_pAllocBank != NULL;
}


void CGroundCombat::CleanupBank(void)
{

	APE_DELETE( CGroundCombat::s_pAllocBank); CGroundCombat::s_pAllocBank = NULL;
}


CAIThought* CGroundCombat::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CGroundCombat::s_pAllocBank->Recycle((CGroundCombat *) pRecycle);
	}
	else
	{
		return CGroundCombat::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CGroundCombat::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGROUNDCOMBAT) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGroundCombat::GetClassInterface(void)
{
	return CLASS_CGROUNDCOMBAT;
}


BOOL CScoutAlert::InitBank(s32 nHowMany)
{
	if (!CScoutAlert::s_pAllocBank)
	{
		CScoutAlert::s_pAllocBank = APE_NEW CNiBank<CScoutAlert>(aimain_pNodePool, nHowMany, APE_NEW CScoutAlert[nHowMany]);	  //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CScoutAlert::s_pAllocBank != NULL;
}


void CScoutAlert::CleanupBank(void)
{
	APE_DELETE( CScoutAlert::s_pAllocBank); CScoutAlert::s_pAllocBank = NULL;
}


CAIThought* CScoutAlert::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CScoutAlert::s_pAllocBank->Recycle((CScoutAlert *) pRecycle);
	}
	else
	{
		return CScoutAlert::s_pAllocBank->Get();
	}
	return NULL;
}

BOOL CScoutAlert::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CSCOUTALERT) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CScoutAlert::GetClassInterface(void)
{
	return CLASS_CSCOUTALERT;
}


//
//
//	  Necessary functions for pooling. sorry about it.
//
//
CNiBank<CCorrosiveCombat>* CCorrosiveCombat::s_pAllocBank = NULL;
BOOL CCorrosiveCombat::InitBank(s32 nHowMany)
{
	if (!CCorrosiveCombat::s_pAllocBank)
	{
		CCorrosiveCombat::s_pAllocBank = APE_NEW CNiBank<CCorrosiveCombat>(aimain_pNodePool, nHowMany, APE_NEW CCorrosiveCombat[nHowMany]);	  //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CCorrosiveCombat::s_pAllocBank != NULL;
}


void CCorrosiveCombat::CleanupBank(void)
{
	APE_DELETE( CCorrosiveCombat::s_pAllocBank); CCorrosiveCombat::s_pAllocBank = NULL;
}


CAIThought* CCorrosiveCombat::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CCorrosiveCombat::s_pAllocBank->Recycle((CCorrosiveCombat *) pRecycle);
	}
	else
	{
		return CCorrosiveCombat::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CCorrosiveCombat::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CCORROSIVECOMBAT) || CGroundCombat::SupportsClassInterface(nInterfaceId);
}


s32 CCorrosiveCombat::GetClassInterface(void)
{
	return CLASS_CCORROSIVECOMBAT;
}

//
//
//	  Necessary functions for pooling. sorry about it.
//
//
CNiBank<CZombieBossCombat>* CZombieBossCombat::s_pAllocBank = NULL;
BOOL CZombieBossCombat::InitBank(s32 nHowMany)
{
	if (!CZombieBossCombat::s_pAllocBank)
	{
		CZombieBossCombat::s_pAllocBank = APE_NEW CNiBank<CZombieBossCombat>(aimain_pNodePool, nHowMany, APE_NEW CZombieBossCombat[nHowMany]);	  //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CZombieBossCombat::s_pAllocBank != NULL;
}


void CZombieBossCombat::CleanupBank(void)
{
	APE_DELETE( CZombieBossCombat::s_pAllocBank); CZombieBossCombat::s_pAllocBank = NULL;
}


CAIThought* CZombieBossCombat::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CZombieBossCombat::s_pAllocBank->Recycle((CZombieBossCombat *) pRecycle);
	}
	else
	{
		return CZombieBossCombat::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CZombieBossCombat::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CZOMBIEBOSSCOMBAT) || CGroundCombat::SupportsClassInterface(nInterfaceId);
}


s32 CZombieBossCombat::GetClassInterface(void)
{
	return CLASS_CCORROSIVECOMBAT;
}

