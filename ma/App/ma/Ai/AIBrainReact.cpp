//////////////////////////////////////////////////////////////////////////////////////
// AIBrainReact.cpp - 
//
// Author: Pat MacKellar 
//
// More CAIBrain implementation
//
////////////////////////////////////////////////////////////////////////////////////
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
// 07/2/02 MacKellar   Created.
// 11/13/02 MacKellar	Moved perception code to AIBrainPercep.cpp
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdraw.h"
#include "AIBrain.h"
#include "AIBrainMems.h"
#include "AIMover.h"
#include "AINodePools.h"
#include "ApeNew.h"
#include "AIBotMover.h"  //findfix: remove this when entity is completed
#include "AIBrainman.h"	 //findfix: remove this when entity is completed
#include "AIGameUtils.h"
#include "AiApi.h"
#include "AIEnviro.h"
#include "AIBTATable.h"
#include "AIBrainUtils.h"
#include "AIGroup.h"
#include "AIThoughtsGeneric.h" //follow, etc
#include "../Alarmsys.h"
#include "../Bot.h"   // findfix: shouldn't be here just for peeking into aimover class
#include "../BotTalkInst.h"
#include "../Player.h"
#include "../Protrack.h"
#include "../TalkSystem2.h"
#include "../vehicle.h"

BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );
void AIBrain_TalkEndCB( CBotTalkInst *pTalkInst, CBot *pBot );

u8 _kuSecsBetweenReactBlockOdds = 10;
u8 _kuReactBlockDist = 10; //feet
u8 kuTalkRequestTimeOutSecs = 1;
u8 _kuSecsBetweenHearBTAPlays = 0;
u8 _kuSecsBetweenSeeBTAPlays = 10;
u8 _kuSecsBetweenOuchBTAPlays = 10;
u16 kuMinAllQuietForStartle = 30;
u8 _kuGrenadeDodgeTimeOutSecs = 7;
u8 _kuIgnoreNearbyGrenadeTimeOutSecsSecs = 5;
u8 _kuGoSeeSoundsTimeOutSecs = 7;
u8 _kuIgnoreNearbyGoSeeSoundsTimeOutSecs = 5;
u8 _kuVehicleDodgeReactBlockTimeOutSecs = 3;
f32 kfPersonalSpaceSq = 5.0f*5.0f;
f32 _kfGrenadeDodgeDist = 10.0f;
f32 _fQuickTurnMarkReactionDistSq = 40.0f*40.0f;
s32 kauPersonalSpaceGreetingTimeSpacing[CAIBrain::NUM_GREETINGLEVELS] = 
{
	0, 2, 5, 10
};

f32 kafGreetingLevelDistancesSq[CAIBrain::NUM_GREETINGLEVELS] = 
{
	25.0f*25.0f, kfPersonalSpaceSq, kfPersonalSpaceSq, kfPersonalSpaceSq
};
f32 _kfOOAlertBlendOutTime = 1.0f/10.0f;

static char* _szSuspBTACatNames[4] = 
{
	"P*F_SUS1",
	"P*F_SUS2",
	"P*F_SUS3",
	"P*F_SUS4"
};

static char* _szGreetBTACatNames[CAIBrain::NUM_GREETINGLEVELS] = 
{
	"P*F_GRE1",
	"P*F_GRE2",
	"P*F_GRE3",
	"P*F_GRE4"
};

static char* _szBuddyBTACatNames[CAIBrain::NUM_GREETINGLEVELS] = 
{
	"P*F_JOIN",
	"P*F_JOIN",
	"P*F_JOIN",
	"P*F_JOIN"
};


enum e_Reactions
{
	REACTBLOCK_ATTACK = 0,
	REACTBLOCK_LOOKAT,
	REACTBLOCK_GREET,
};

void CAIBrain::OverrideGreetingLevel(s32 nPlayerId, u8 uGreetingLevel)
{
	if (nPlayerId <0 || nPlayerId>=4)
	{
		return;
	}
	CAIMemorySmall* pGreetMemory = NULL;
	if (!GetKnowledge().CanRememberAny(MEMORY_SMALL_GREETEDPLAYER, &pGreetMemory) || //never greeted any players before
		(pGreetMemory && !ExtractGreetingLevel(pGreetMemory, nPlayerId)))		//never greeted THIS player before
	{  //I have never greeted him before type of greeting
		if (!pGreetMemory)
		{
			pGreetMemory = GetKnowledge().RememberSmall(MEMORY_SMALL_GREETEDPLAYER, 300);  //secs
		}
		if (pGreetMemory)
		{
			pGreetMemory->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_NOTIMER;
			StuffGreetingLevel(pGreetMemory, nPlayerId, uGreetingLevel);	 //which player
		}
	}
}


BOOL CAIBrain::Action_Greet(CAIMemorySmall* pGreetMemory, CEntity* pOtherEntity, u8 uGreetingLevel)
{
	cchar *pszBTAName = NULL;

	if (m_pMover->IsSleeping())
	{
		FASSERT(0); //How could this happen? eyes should be closed while sleeping
		m_pMover->WakeUp();
	}

	if (uGreetingLevel >= GREETINGLEVEL_0 &&
		uGreetingLevel< NUM_GREETINGLEVELS &&
		!IsTalking())
	{
		char** _apaszBTACatNames[2] = {_szGreetBTACatNames, _szBuddyBTACatNames};
		BOOL bBuddy = (GetLeader() && GetLeader()->GetAIMover()->GetEntity() == pOtherEntity);
		if (pszBTAName = AIBTA_EventToBTAName(_apaszBTACatNames[bBuddy][uGreetingLevel], this))
		{
			ai_TalkModeBegin(this,
								AIBrain_TalkModeCB,
								(void*) pszBTAName,
								GetAIMover()->GetEntity(),
								kuTalkRequestTimeOutSecs,
								uGreetingLevel==0, //Stop first if greetlevel == 0
								TRUE,  //LOOK AT
								pOtherEntity->Guid());

			return TRUE;
		}
	}
	return FALSE;
}


BOOL CAIBrain::Action_Attack(CEntity* pOtherEntity)
{

	CEntity* pEnemy = NULL;
	if (!pOtherEntity ||
		!pOtherEntity->AIBrain() ||
		(pOtherEntity->AIBrain() != GetLeader() &&			 //never attack your leader
		 (!pOtherEntity->AIBrain()->GetLeader() ||
		  pOtherEntity->AIBrain()->GetLeader() != GetLeader())))

	{
		pEnemy = pOtherEntity;
		if (pEnemy && pEnemy->TypeBits() & ENTITY_BIT_BOT && ((CBot*) pEnemy)->IsDeadOrDying())
		{
			pEnemy = NULL;
		}
	}

	if ((pEnemy && ChangeReactionState(REACTIONSTATE_ATTACK)) ||
		!pEnemy && ChangeReactionState(REACTIONSTATE_ATTACK_IMPULSE))
	{
		if ( m_pMover->IsSleeping())
		{
			m_pMover->WakeUp();
		}
		ai_AssignGoal_Attack(	this,
								pEnemy ? pEnemy->Guid() : 0,
								0);						// Some scripting event to call apon completion or failure
		return TRUE;
	}
	return FALSE;
}


BOOL CAIBrain::Action_DodgeGrenade(const CFVec3A& WhereLoc)
{
	if (!((m_uReactionState == REACTIONSTATE_GRENADE_DODGE)|| (m_uReactionState == REACTIONSTATE_GRENADE_DODGE_2)) )
	{
		if (m_uIgnoreNearbyGrenadeTimeOut < aiutils_GetCurTimeSecsU16())
		{
			if (m_uOddsOfIgnoringNearbyGrenades && (u16) fmath_RandomChoice(100) < m_uOddsOfIgnoringNearbyGrenades )
			{
				//don't start grenade dodge state due to Odds of ignoring grenades
				m_uIgnoreNearbyGrenadeTimeOut = aiutils_GetCurTimeSecsU16()+_kuIgnoreNearbyGrenadeTimeOutSecsSecs;
			}
			else
			{
				if (ChangeReactionState(REACTIONSTATE_GRENADE_DODGE))
				{
					m_uDodgeGrenadeTimeOut = aiutils_GetCurTimeSecsU16()+_kuGrenadeDodgeTimeOutSecs;
				}
			}
		}
	}

	if (m_uReactionState == REACTIONSTATE_GRENADE_DODGE)
	{
		if ( m_pMover->IsSleeping())
		{
			m_pMover->WakeUp();
		}

		CFVec3A DeltaPos;
		f32 fDeltaDist = 0.0f;
		DeltaPos.Sub(WhereLoc, GetAIMover()->GetLoc());
		if (1)//DeltaPos.Dot(GetAIMover()->GetEntity()->MtxToWorld()->m_vFront) < 0.0f)	  //if it is in front of me.
		{
			fDeltaDist = DeltaPos.SafeUnitAndMag(DeltaPos);
			if (fDeltaDist > 0.0f)
			{

				DeltaPos.Mul(50.0f-fDeltaDist);
				DeltaPos.Mul(-1.0f);
				DeltaPos.Add(GetAIMover()->GetLoc());


				CAIMemoryLarge* pParamPack = GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

				if (pParamPack)
				{
					f32 fFudgeDist = 3.0f + GetAIMover()->GetRadiusXZ();
					FMATH_CLAMP(fFudgeDist, 0.0f, 255.0f);
					StuffGotoParams( pParamPack,
									DeltaPos,												//const CFVec3A& GotoLoc,		// Where
									(u8) fFudgeDist,					//u8 uFudgeDest,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
									100,													//u8 uSpeedPct,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
									GOTOFLAG_FORCE_ROLL*(fmath_RandomChoice(100) < 40));	// GOTOFLAG_*

					if (!AssignReact(CAIBrain::TT_GOTO, pParamPack))
					{
					   GetKnowledge().ForgetThis(pParamPack);
					}

					cchar* pszBTAName = NULL;
					if (!IsTalking() && (pszBTAName = AIBTA_EventToBTAName("N*E_RUNS", this)))
					{
						ai_TalkModeBegin(this,
										AIBrain_TalkModeCB,
										(void*) pszBTAName,
										GetAIMover()->GetEntity(),
										kuTalkRequestTimeOutSecs,
										FALSE, //STOP/DON'T STOP
										FALSE, //LOOK/DON'T LOOK
										0);
					}
					m_uReactionState = REACTIONSTATE_GRENADE_DODGE_2;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}



BOOL CAIBrain::Action_DodgeVehicle(CAISound* pSound)
{
	if (!((m_uReactionState == REACTIONSTATE_DODGE_VEHICLE) || (m_uReactionState == REACTIONSTATE_DODGE_VEHICLE_2 )))
	{
		if (m_uDodgeVehicleReactBlockerTimeOut < aiutils_GetCurTimeSecsU16())
		{
			if (ChangeReactionState(REACTIONSTATE_DODGE_VEHICLE))
			{
				m_uDodgeVehicleReactBlockerTimeOut = aiutils_GetCurTimeSecsU16()+_kuVehicleDodgeReactBlockTimeOutSecs;
			}
		}
	}

	if (m_uReactionState == REACTIONSTATE_DODGE_VEHICLE)
	{
		if ( m_pMover->IsSleeping())
		{
			m_pMover->WakeUp();
		}

		CEntity* pDodgeThis = NULL;

		if (m_pMover->GetEntity()->TypeBits() & ENTITY_BIT_VEHICLE ||
			m_pMover->GetCurMech())
		{					   //some things just should dodge vehicles
			return FALSE;
		}

		if (pSound->m_uSoundType == AISOUNDTYPE_BOT &&
			pSound->m_pEntity &&
			pSound->m_pEntity->GetParent() &&
			(pSound->m_pEntity->GetParent()->TypeBits() & ENTITY_BIT_VEHICLE))
		{
			pDodgeThis = (CVehicle*) pSound->m_pEntity->GetParent();
		}
		else if (pSound->m_uSoundType == AISOUNDTYPE_DODGEME && pSound->m_pEntity)
		{
			pDodgeThis = pSound->m_pEntity;
		}

		if (pDodgeThis)
		{
			CFVec3A DeltaPosXZ;
			f32 fDeltaDist = 0.0f;
			CFVec3A FrontVec_XZ = pDodgeThis->MtxToWorld()->m_vFront;
			FrontVec_XZ.y = 0.0f;
			CFVec3A RightVec_XZ;
			if (FrontVec_XZ.SafeUnitAndMag(FrontVec_XZ) > 0.0f)
			{
				CFVec3A RightVec_XZ;
				RightVec_XZ.x = FrontVec_XZ.z;
				RightVec_XZ.y = 0.0f;
				RightVec_XZ.z = -FrontVec_XZ.x;

				DeltaPosXZ.Sub(GetAIMover()->GetLoc(), pDodgeThis->MtxToWorld()->m_vPos);
				if (RightVec_XZ.Dot(DeltaPosXZ) > 0.0f)
				{
					DeltaPosXZ = RightVec_XZ;
				}
				else
				{
					DeltaPosXZ = RightVec_XZ;
					DeltaPosXZ.Mul(-1.0f);
				}
				DeltaPosXZ.Mul(18.0f);	   //f32 should get distance based on object I'm avoiding, but 18 works for grunts-vs-rat.
				DeltaPosXZ.Add(GetAIMover()->GetLoc());

				f32 fFloorHeight = 0.0f;
				if (aiutils_FloorHeightAt(DeltaPosXZ, &fFloorHeight, 10.0f))
				{
					DeltaPosXZ.y = fFloorHeight+aigraph_kfSurfaceOffset;
				}
				CAIMemoryLarge* pParamPack = GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

				if (pParamPack)
				{
					f32 fFudgeDist = 3.0f + GetAIMover()->GetRadiusXZ();
					FMATH_CLAMP(fFudgeDist, 0.0f, 255.0f);
					StuffGotoParams( pParamPack,
									DeltaPosXZ,								//const CFVec3A& GotoLoc,		// Where
									(u8) fFudgeDist,						//u8 uFudgeDest,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
									100,									//u8 uSpeedPct,					// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
									GOTOFLAG_FORCE_ROLL);					// GOTOFLAG_*


					if (!AssignReact(CAIBrain::TT_GOTO, pParamPack))
					{
						GetKnowledge().ForgetThis(pParamPack);
					}
					m_uReactionState = REACTIONSTATE_DODGE_VEHICLE_2;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


BOOL CAIBrain::Action_GoSeeSound(CAISound* pSound)
{
	if (!((m_uReactionState == REACTIONSTATE_GOSEE_SOUND)|| (m_uReactionState == REACTIONSTATE_GOSEE_SOUND_2)) )
	{
		if (pSound->m_uSoundType == AISOUNDTYPE_ALARM)
		{
			if (ChangeReactionState(REACTIONSTATE_GOSEE_SOUND))
			{
				m_uGoSeeSoundTimeOut = aiutils_GetCurTimeSecsU16()+_kuGoSeeSoundsTimeOutSecs;   //when to give up
			}

		}
		else if (m_uDontGoSeeSoundsTimeOut < aiutils_GetCurTimeSecsU16())
		{
			if (m_uOddsOfNotGoingToSeeSounds && (u16) fmath_RandomChoice(100) < m_uOddsOfNotGoingToSeeSounds )
			{
				//don't start reaction due to Odds of ignoring grenades
				m_uDontGoSeeSoundsTimeOut = aiutils_GetCurTimeSecsU16()+_kuIgnoreNearbyGoSeeSoundsTimeOutSecs;
			}
			else
			{
				if (ChangeReactionState(REACTIONSTATE_GOSEE_SOUND))
				{
					m_uGoSeeSoundTimeOut = aiutils_GetCurTimeSecsU16()+_kuGoSeeSoundsTimeOutSecs;

					if ( m_pMover->IsSleeping())
					{
						m_pMover->WakeUp();
					}
				}
			}
		}
	}


	//
	// Just invoked this action
	//
	if (m_uReactionState == REACTIONSTATE_GOSEE_SOUND)
	{
		if (GetAIMover()->GetLoc().DistSq(pSound->m_Origin) < GetAIMover()->GetRadius() * 40.0f * GetAIMover()->GetRadius() * 40.0f)
		{
			CAIMemoryLarge* pParamPack = GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

			if (pParamPack)
			{
				StuffGotoParams(pParamPack,
								pSound->m_Origin,   // Where                                                                                    
								3+(u8) GetAIMover()->GetRadiusXZ(),					// u8 uFudgeDest = 0,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
								0,					// u8 uSpeedPct = 0,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
								GOTOFLAG_USE_JOB_REACT_RULES);					// u16	uGotoFlags = 0);			// GOTOFLAG_*

				if (!AssignReact(CAIBrain::TT_GOTO, pParamPack))
				{
					GetKnowledge().ForgetThis(pParamPack);
				}

				m_uReactionState = REACTIONSTATE_GOSEE_SOUND_2;
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CAIBrain::Action_InvestigateSound(CAIMemoryLarge* pHeardMem,  CAISound* pSound)
{
	CFVec3A WhereLoc;
	WhereLoc.Set(pHeardMem->m_EntityLoc);
	if (!pSound)
	{
		return FALSE;
	}
	cchar* pszBTAName = NULL;

	if (m_pMover->IsSleeping())
	{
		f32 fSoundMakersRadius = 0.0f;
		if (pHeardMem->m_pEntity &&
			pHeardMem->m_pEntity->AIBrain())
		{
			fSoundMakersRadius = pHeardMem->m_pEntity->AIBrain()->GetAIMover()->GetRadiusXZ();
		}

		//
		// because some sleeping bots have a distance check, regardless of how loud the sound is.
		//
		if (GetWakeupRadius() == 0xff ||	//no distance limit
			pSound->m_Origin.DistSq(m_pMover->GetLoc()) < (GetWakeupRadius()+fSoundMakersRadius)*(GetWakeupRadius()+fSoundMakersRadius))
		{

			//
			//  Is the sound loud enough to make me wakeup outright, or just napjerk a little
			//
			f32 fDistToSound  = pSound->m_Origin.Dist(GetAIMover()->GetLoc());
			f32 fVolSlack = pSound->m_fAudibleRadius - fDistToSound;
			if (pSound->m_fAudibleRadius < 19.1f && fVolSlack < 10.0f)
			{   //not too loud, just napjerk
				if (m_fVolSlackTimeOut < aiutils_GetCurTimeSecs())
				{
					m_VolSlackAtLastNapJerk = 0.0f;
				}

				if (m_fVolSlackTimeOut > aiutils_GetCurTimeSecs() &&
					fVolSlack > m_VolSlackAtLastNapJerk)
				{
					m_pMover->WakeUp();
				}
				else if (m_fVolSlackTimeOut < aiutils_GetCurTimeSecs() ||
						fVolSlack > m_VolSlackAtLastNapJerk )
				{
					m_VolSlackAtLastNapJerk = fVolSlack+1.0f;
					m_fVolSlackTimeOut = aiutils_GetCurTimeSecs()+1.5f;
					if (m_pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
					{
						((CBot*) m_pMover->GetEntity())->NapJerk(TRUE);		 //TRUE means, bot can wakeup if it wants to.
					}
					SetSoundMark(pSound);

					cchar* pszBTAName = NULL;
					if (!IsTalking() &&
						(pszBTAName = AIBTA_EventToBTAName("N*E_NAPJ", this)))
					{
						ai_TalkModeBegin(this,
										AIBrain_TalkModeCB,
										(void*) pszBTAName,
										GetAIMover()->GetEntity(),
										kuTalkRequestTimeOutSecs,
										FALSE, //DON"T STOP
										FALSE,// LOOK
										0);	 //don't track another entity
					}
				}
			}
			else
			{	//that was a loud sound.  No need to napJerk,
				m_pMover->WakeUp();

				if (pSound->m_uSoundType == AISOUNDTYPE_RAGE &&
					pSound->m_pEntity &&
					pSound->m_pEntity->AIBrain())
				{
					CFVec3A CauseOfRage = pSound->m_Origin;
					if (ai_GetLastEnemySeen(pSound->m_pEntity->AIBrain(), &CauseOfRage))
					{
						SetSightMark_2Stage(pSound->m_Origin, CauseOfRage, SIGHT_MARK_FLAG_BOT);
					}
					else
					{
						SetSightMark_1Stage(pSound->m_Origin, SIGHT_MARK_FLAG_BOT);
					}
				}
				else
				{
					if (!!IsTalking() && 
						m_uHearBTATimeOut < aiutils_GetCurTimeSecsU16() &&
						(pszBTAName = AIBTA_EventToBTAName("N*E_HEAR", this)))
					{
						if (ai_TalkModeBegin(this,
										AIBrain_TalkModeCB,
										(void*) pszBTAName,
										GetAIMover()->GetEntity(),
										kuTalkRequestTimeOutSecs,
										FALSE, //DON"T STOP
										FALSE,// LOOK
										0))	 //don't track another entity
						{
							m_uHearBTATimeOut = aiutils_GetCurTimeSecsU16()+_kuSecsBetweenHearBTAPlays;
						}
					}
					SetSoundMark(pSound);
				}
			}
		}
	}
	else
	{   //not sleeping
		if (pSound->m_uSoundType == AISOUNDTYPE_RAGE &&
			pSound->m_pEntity)
		{
			CFVec3A CauseOfRage = pSound->m_Origin;
			if (ai_GetLastEnemySeen(pSound->m_pEntity->AIBrain(), &CauseOfRage))
			{
				SetSightMark_2Stage(pSound->m_Origin, CauseOfRage, SIGHT_MARK_FLAG_BOT);
			}
			else
			{
				SetSightMark_1Stage(pSound->m_Origin, SIGHT_MARK_FLAG_BOT);
			}

			pHeardMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
		}
		else
		{
			if (!IsTalking() && 
				this->m_fBrainAlertUnit < 0.1f && 
				m_uHearBTATimeOut < aiutils_GetCurTimeSecsU16() &&
				(pszBTAName = AIBTA_EventToBTAName("N*E_HEAR", this)))
			{
				if (ai_TalkModeBegin(this,
								AIBrain_TalkModeCB,
								(void*) pszBTAName,
								GetAIMover()->GetEntity(),
								kuTalkRequestTimeOutSecs,
								FALSE, //DON"T STOP
								FALSE,// LOOK
								0))	 //don't track another entity
				{
					m_uHearBTATimeOut = aiutils_GetCurTimeSecsU16()+_kuSecsBetweenHearBTAPlays;
				}

			}

			if ((pSound->m_uSoundType == AISOUNDTYPE_BOT && GetAIMover()->m_pEntity->TypeBits() & ENTITY_BIT_BOTZOM) ||
				pSound->m_uSoundType == AISOUNDTYPE_BOTFOOD || 
				(pSound->m_uSoundType == AISOUNDTYPE_ALARM && m_pMover->GetEntity()->HasAlarmSysUse() ))
			{	//go checkout the sound?
				BOOL bGoSee = TRUE;
				if (pSound->m_uSoundType == AISOUNDTYPE_ALARM)
				{
					CAlarm* pAlarm = CAlarmNet::FindAlarm(pSound->m_pEntity);
					if (pAlarm->m_uAlarmNetId == AlarmSys_GetAutoBotNetId(((CBot*)m_pMover->GetEntity())))
					{
						bGoSee = FALSE;
					}
				}
				
				if (bGoSee)
				{
					Action_GoSeeSound(pSound);
					pHeardMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
				}
			}

			
			//any sound except that of my leader will at least set the sound mark
			if (!(pSound->m_uSoundType == AISOUNDTYPE_BOT && GetLeader() && pSound->m_pEntity == GetLeader()->GetAIMover()->GetEntity())) 
			{
				SetSoundMark(pSound);
			}
		}

		return TRUE;//hmmmmmm
	}

	return FALSE;
}


BOOL CAIBrain::Rules_Job_Generic_OnSeenFriendlyPlayer(CAIMemoryLarge* pSeenMem)
{
	BOOL bActionTakenThisFrame = FALSE;
	CEntity* pOtherEntity = pSeenMem->m_pEntity;
	s32 nPlayerId = ((CBot*)pOtherEntity)->m_nPossessionPlayerIndex;
	CAIMemoryLarge* pVisCheckMem = NULL;
	GetKnowledge().CanRememberAny(MEMORY_MEDIUM_VISCHECK, (CAIMemorySmall**)&pVisCheckMem);
	CAIMemorySmall* pGreetMemory = NULL;
	GetKnowledge().CanRememberAny(MEMORY_SMALL_GREETEDPLAYER, &pGreetMemory);
	u8 uGreetingLevel = GREETINGLEVEL_0;
	if (pGreetMemory)
	{
		uGreetingLevel =  ExtractGreetingLevel(pGreetMemory, nPlayerId);
	}

	if ((!pVisCheckMem || pVisCheckMem->m_fMediumData > 0.25f) &&
		!pGreetMemory &&
		(!pOtherEntity || !pOtherEntity->AIBrain() || pOtherEntity->AIBrain() != GetLeader()))
	{
		CFVec3A tmp;
		tmp.Set(pSeenMem->m_EntityLoc);
		tmp.y+=((CBot*)pOtherEntity)->m_fCollCylinderHeight_WS;
		SetSightMark_1Stage(CFVec3A(tmp), SIGHT_MARK_FLAG_BOT);
	}

	if (!pVisCheckMem || pVisCheckMem->m_fMediumData >= 0.99f)
	{
		FASSERT(pOtherEntity->TypeBits() & ENTITY_BIT_BOT);
		if (m_uBrainFlags & BRAINFLAG_ATTACKS_FRIENDLY &&
			m_uBrainFlags & BRAINFLAG_ATTACKS_PLAYERS)
		{	//Attack
			bActionTakenThisFrame = Action_Attack(pOtherEntity);
		}
		else if (pOtherEntity->AIBrain() &&
				!GetLeader() &&
				 GetRace() == pOtherEntity->AIBrain()->GetRace()&&
				 GetFlag_Buddy_Ctrl_Auto()) //join with friendly players
		{
			bActionTakenThisFrame = AssignLeader(pOtherEntity->AIBrain());
		}
		else if (uGreetingLevel < NUM_GREETINGLEVELS)
		{	//Greet?
/*		note: pgm removed all greeting stuff.  I think it was written before buddies were even a glimmer in their papa's eye.
			FASSERT( nPlayerId >=0 && nPlayerId < 4);
			f32 fDistToEntitySq = aiutils_GetEntityDist2(GetAIMover()->GetEntity(), pOtherEntity);

			if (!IsTalking())
			{
				if (!pGreetMemory)
				{
					pGreetMemory = GetKnowledge().RememberSmall(MEMORY_SMALL_GREETEDPLAYER, 300);  //secs
					if (pGreetMemory)
					{
						StuffGreetingLevel(pGreetMemory, nPlayerId, 0);
					}
				}

				if (fDistToEntitySq < kafGreetingLevelDistancesSq[uGreetingLevel])
				{
					//
					// if
					//		 Have never greeted,
					// then
					//       Greet
					// else	if
					//		 Have greeted < than max times
					// then
					//       Greet
					if (uGreetingLevel == 0)		
					{  //I have never greeted this bot before
						if (pGreetMemory)
						{
							bActionTakenThisFrame = Action_Greet(pGreetMemory, pOtherEntity, uGreetingLevel);

							if (bActionTakenThisFrame)
							{
								StuffIncGreetingLevel(pGreetMemory, ((CBot*) pOtherEntity)->m_nPossessionPlayerIndex);
							}
						}
					}
					else if (pGreetMemory &&
							((aiutils_GetCurTimeSecsU16() - pGreetMemory->m_uWhenTimeSecs) >  kauPersonalSpaceGreetingTimeSpacing[uGreetingLevel]))
					{  
						pGreetMemory->m_uWhenTimeSecs = aiutils_GetCurTimeSecsU16();
						bActionTakenThisFrame = Action_Greet(pGreetMemory, pOtherEntity, uGreetingLevel);
						if (bActionTakenThisFrame)
						{
							StuffIncGreetingLevel(pGreetMemory, ((CBot*) pOtherEntity)->m_nPossessionPlayerIndex);
						}
					}
				}
			}
*/
		}
	}

	return bActionTakenThisFrame;
}


BOOL CAIBrain::Rules_Job_Generic_OnSeenEnemyPlayer(CAIMemoryLarge* pSeenMem)
{
	BOOL bActionTakenThisFrame = FALSE;
	CAIMemoryLarge* pVisCheckMem = NULL;
	GetKnowledge().CanRememberAny(MEMORY_MEDIUM_VISCHECK, (CAIMemorySmall**)&pVisCheckMem);
	CAIMemoryMedium* pReactBlockerMem = NULL;
	GetKnowledge().CanRememberAny(MEMORY_MEDIUM_SPHEREMARKER, (CAIMemorySmall**)&pReactBlockerMem);
	

	//  note.
	//	this code related to a Reaction Block, is an attempt to have a bot 
	//  decide to ignore something for a certain
	//  amount of time as long as neither his position, or the position of the "thing" changes too much
	BOOL bIgnore = FALSE;
	if (pReactBlockerMem && DoesSeenMemMatchSphereMarker(pReactBlockerMem, pSeenMem))
	{
		bIgnore = TRUE;
	}
	else if (!pReactBlockerMem &&
			m_uEyeReactBlockerTimeOut < aiutils_GetCurTimeSecsU16() &&
			m_uOddsOfIgnoringPlayerAtEyeScanDist)
	{
		CFVec3A ELoc(pSeenMem->m_EntityLoc);
		f32 fDist = ELoc.Dist(GetLoc());
		f32 fEyeScanDist = GetEyeScanDist()+GetBonusEyeScanDistForPlayers();
		if (fDist > (f32) m_uInnerRadiusForIgnorePlayerOdds)
		{
			f32 fDistLerp = 0.0f;

			if (fEyeScanDist > (f32) m_uInnerRadiusForIgnorePlayerOdds+1.0f)
			{
				fDistLerp = (fDist - (f32) m_uInnerRadiusForIgnorePlayerOdds) / (fEyeScanDist - (f32) m_uInnerRadiusForIgnorePlayerOdds);
			}
			else
			{
				fDistLerp = 1.0f;
			}
			u8 uOddsOfIgnoring = (u8)(m_uOddsOfIgnoringPlayerAtEyeScanDist*fDistLerp);
			if (fmath_RandomChoice(100) < uOddsOfIgnoring)
			{
				bIgnore = TRUE;
				pReactBlockerMem = GetKnowledge().RememberMedium(MEMORY_MEDIUM_SPHEREMARKER, _kuSecsBetweenReactBlockOdds);
				if (pReactBlockerMem)
				{
					StuffSphereMarker(pReactBlockerMem, GetLoc(), pSeenMem->m_EntityLoc, REACTBLOCK_ATTACK, _kuReactBlockDist);
				}
			}
		}
		m_uEyeReactBlockerTimeOut = aiutils_GetCurTimeSecsU16()+_kuSecsBetweenReactBlockOdds;
	}

	if (!bIgnore)
	{
		if (m_uBrainFlags & BRAINFLAG_ATTACKS_ENEMY &&
			m_uBrainFlags & BRAINFLAG_ATTACKS_PLAYERS &&
			(!pVisCheckMem || pVisCheckMem->m_fMediumData > 0.99f))
		{
			bActionTakenThisFrame = Action_Attack(pSeenMem->m_pEntity);
		}
		if (pVisCheckMem && pVisCheckMem->m_fMediumData >= 0.25f)
		{
			CFVec3A	LookAtLoc;
	//		u8 uCurTag = pVisCheckMem->m_uSmallData & 0x00ff;
			u8 uCurTag = 0;
			LookAtLoc = *(pSeenMem->m_pEntity->GetTagPoint(uCurTag));
			SetSightMark_1Stage(LookAtLoc, SIGHT_MARK_FLAG_BOT);
		}
	}

	return bActionTakenThisFrame;

}


BOOL CAIBrain::Rules_Job_Generic_OnSeenSuspiciousPlayer(CAIMemoryLarge* pSeenMem, f32 *pfSuspicion)
{
	CAIBrain* pSuspicionLeader = NULL;
	f32 fMaxSuspicion = 0.0f;
	cchar* pszBTAName =NULL;
	u8 uSuspicionLevel = 0;
	BOOL bActionTakenThisFrame = FALSE;
	CAIMemoryLarge* pVisCheckMem = NULL;
	GetKnowledge().CanRememberAny(MEMORY_MEDIUM_VISCHECK, (CAIMemorySmall**)&pVisCheckMem);
	CAIMemoryMedium* pReactBlockerMem = NULL;
	GetKnowledge().CanRememberAny(MEMORY_MEDIUM_SPHEREMARKER, (CAIMemorySmall**)&pReactBlockerMem);
	BOOL bIgnore = FALSE;
	CFVec3A	LookAtLoc;
	
	//  note.
	//	this code related to a Reaction Block, is an attempt to have a bot 
	//  decide to ignore something for a certain
	//  amount of time as long as neither his position, or the position of the "thing" changes too much
	if (pReactBlockerMem && DoesSeenMemMatchSphereMarker(pReactBlockerMem, pSeenMem))
	{
		bIgnore = TRUE;
	}
	else if (!pReactBlockerMem &&
			m_uEyeReactBlockerTimeOut < aiutils_GetCurTimeSecsU16() &&
			m_uOddsOfIgnoringPlayerAtEyeScanDist)
	{
		CFVec3A ELoc(pSeenMem->m_EntityLoc);
		f32 fDist = ELoc.Dist(GetLoc());
		if (fDist > (f32) m_uInnerRadiusForIgnorePlayerOdds)
		{
			f32 fDistLerp = (fDist- (f32) m_uInnerRadiusForIgnorePlayerOdds)/(GetEyeScanDist()+GetBonusEyeScanDistForPlayers()-(f32) m_uInnerRadiusForIgnorePlayerOdds);
			u8 uOddsOfIgnoring = (u8)(m_uOddsOfIgnoringPlayerAtEyeScanDist*fDistLerp);
			if (fmath_RandomChoice(100) < uOddsOfIgnoring)
			{
				bIgnore = TRUE;
				pReactBlockerMem = GetKnowledge().RememberMedium(MEMORY_MEDIUM_SPHEREMARKER, _kuSecsBetweenReactBlockOdds);
				if (pReactBlockerMem)
				{
					StuffSphereMarker(pReactBlockerMem, GetLoc(), pSeenMem->m_EntityLoc, REACTBLOCK_ATTACK, _kuReactBlockDist);
				}
			}
		}
		m_uEyeReactBlockerTimeOut = aiutils_GetCurTimeSecsU16()+_kuSecsBetweenReactBlockOdds;
	}

	if (!bIgnore)
	{
		if (aigroup_GetSuspicionInfo(pSeenMem->m_pEntity, &pSuspicionLeader, NULL, &fMaxSuspicion) &&
			fMaxSuspicion > *pfSuspicion)
		{
			*pfSuspicion = fMaxSuspicion;
		}
		
		if (pVisCheckMem && pVisCheckMem->m_fMediumData >= 0.25f)
		{
			if (*pfSuspicion < 0.1f)
			{
			   //ignore
				uSuspicionLevel = 0;
			}
			else if (*pfSuspicion < 0.25f)
			{
				u8 uCurTag = 0;//pVisCheckMem->m_uSmallData & 0x00ff;
				LookAtLoc = *(pSeenMem->m_pEntity->GetTagPoint(uCurTag));
				SetSightMark_1Stage(LookAtLoc, SIGHT_MARK_FLAG_BOT);
				uSuspicionLevel = 1;
			}
			else if (*pfSuspicion < 0.4f)
			{
				u8 uCurTag = 0;//pVisCheckMem->m_uSmallData & 0x00ff;
				LookAtLoc = *(pSeenMem->m_pEntity->GetTagPoint(uCurTag));
				SetSightMark_1Stage(LookAtLoc, SIGHT_MARK_FLAG_BOT | SIGHT_MARK_FLAG_STOP_AND_LOOK);
				uSuspicionLevel = 2;
			}
			else if (*pfSuspicion < 1.0f)
			{
				u8 uCurTag = 0;//pVisCheckMem->m_uSmallData & 0x00ff;
				LookAtLoc = *(pSeenMem->m_pEntity->GetTagPoint(uCurTag));
				SetSightMark_1Stage(LookAtLoc, SIGHT_MARK_FLAG_BOT);

				if (GetReactionState() != REACTIONSTATE_FOLLOW_SUSPICIOUS &&
					ChangeReactionState(REACTIONSTATE_FOLLOW_SUSPICIOUS))
				{
					ai_FollowLeader(this, pSeenMem->m_pEntity->Guid(), -1);
					if (m_pCurFollowerThought && m_pCurFollowerThought->SupportsClassInterface(CLASS_CGENERICFOLLOW))
					{
						((CGenericFollow*) m_pCurFollowerThought)->SetLook_AtLeader();
						bActionTakenThisFrame = TRUE;
					}
				}
				if (*pfSuspicion > 0.9f)
				{
					uSuspicionLevel = 3;
				}
				else
				{
					uSuspicionLevel = 2;
				}
			}

			if (uSuspicionLevel < m_uSuspicionLevel)
			{
				m_uSuspicionBTAPlayed = 0;
			}
			m_uSuspicionLevel = uSuspicionLevel;

			if (!IsTalking() && 
				m_uSuspicionBTAPlayed <= m_uSuspicionLevel &&
				pSuspicionLeader == this)
			{
				FASSERT(uSuspicionLevel >= 0 && uSuspicionLevel < 4);
				pszBTAName = AIBTA_EventToBTAName(_szSuspBTACatNames[uSuspicionLevel], this);
				if (pszBTAName)
				{
					ai_TalkModeBegin(this,
									AIBrain_TalkModeCB,
									(void*) pszBTAName,
									GetAIMover()->GetEntity(),
									1,//kuAttackMOdeTalkRequestTimeOutSecs,
									FALSE, //STOP
									TRUE, //LOOK
									pSeenMem->m_pEntity->Guid());
					m_uSuspicionBTAPlayed = uSuspicionLevel+1;
				}
			}
		}
	}
	return bActionTakenThisFrame;
}


BOOL CAIBrain::ChangeReactionState(u32 uReactionState)
{
	BOOL bOKtoChange = TRUE;

	//
	// Can we change to that state right now?
	//
	switch (uReactionState)
	{
		case REACTIONSTATE_FOLLOW_SUSPICIOUS:
			if (uReactionState == REACTIONSTATE_GRENADE_DODGE || uReactionState == REACTIONSTATE_GRENADE_DODGE_2)
			{
				bOKtoChange = FALSE;
			}
			break;

		case REACTIONSTATE_ATTACK_IMPULSE:
			{
				//if current reactionstate == attack, then no changing.
				if (GetCurThought() == TT_COMBAT && (m_uReactionState == REACTIONSTATE_ATTACK || m_uReactionState == REACTIONSTATE_ATTACK_IMPULSE))
				{
					bOKtoChange = FALSE;
				}
			}
			break;
		case REACTIONSTATE_ATTACK:
			{
				//if current reactionstate == attack, then no changing.
				if (GetCurThought() == TT_COMBAT && m_uReactionState == REACTIONSTATE_ATTACK)
				{
					bOKtoChange = FALSE;
				}
			}
			break;
	}

	if (bOKtoChange)
	{
		//
		//  Cleanup old state
		//
		switch (m_uReactionState)
		{
			case REACTIONSTATE_FOLLOW_SUSPICIOUS:
				if (IsFollowing())
				{
					StopFollowing();
				}
				break;
			case REACTIONSTATE_ATTACK:
				{

				}
				break;
		}
		
		// switch
		m_uReactionState = uReactionState;

		//
		//  Init new state
		//
		switch (m_uReactionState)
		{
			case REACTIONSTATE_FOLLOW_SUSPICIOUS:
				break;
			case REACTIONSTATE_ATTACK:
			case REACTIONSTATE_ATTACK_IMPULSE:
				m_fBrainAlertUnit = 1.0f;
				break;
		}
	}

	return bOKtoChange;
}



BOOL CAIBrain::Rules_Job_Generic_OnSeenFriendlyNPC(CAIMemoryLarge* pSeenMem)
{
	BOOL bDidIt = FALSE;
	if (m_uBrainFlags & BRAINFLAG_ATTACKS_FRIENDLY &&
		m_uBrainFlags & BRAINFLAG_ATTACKS_NPC)
	{
		bDidIt = Action_Attack(pSeenMem->m_pEntity);
	}
	return bDidIt;
}


BOOL CAIBrain::ShouldFollowerAttack(CAIBrain* pLeaderBrain, CEntity* pEnemy)
{
   BOOL bOkToAttack;
	//O.k to attack?
//	if (GetKnowledge().CanRememberEntity(MEMORY_LARGE_DAMAGED_BY, pEnemy) ||
//		pLeaderBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_DAMAGE_DEALT, pEnemy) ||
//		pLeaderBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_DAMAGED_BY, pEnemy))
	if (GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY) ||
		pLeaderBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGE_DEALT) ||
		pLeaderBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY) ||
		((pLeaderBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) && (aiutils_GetCurTimeSecs() - ((CBot*)pLeaderBrain->GetAIMover()->GetEntity())->GetLastFireTime() < 1.0f)))
	{
		bOkToAttack = TRUE;
	}
	else
	{
		bOkToAttack = FALSE;
	}

	return bOkToAttack;

}

BOOL CAIBrain::Rules_Job_Generic_OnSeenEnemyNPC(CAIMemoryLarge* pSeenMem)
{
	BOOL bDidIt = FALSE;
	if (m_uBrainFlags & BRAINFLAG_ATTACKS_ENEMY &&
		m_uBrainFlags & BRAINFLAG_ATTACKS_NPC)
	{
		BOOL bOkToAttack;
		CAIBrain* pLeaderBrain = pLeaderBrain = GetLeader();
		if (pLeaderBrain)		 //rules of engagement for followers
		{
			bOkToAttack = ShouldFollowerAttack(pLeaderBrain, pSeenMem->m_pEntity);
		}
		else
		{
			bOkToAttack = TRUE;
		}

		if (bOkToAttack)
		{
			bDidIt = Action_Attack(pSeenMem->m_pEntity);
		}
	}
	return bDidIt;
}

	
BOOL CAIBrain::Rules_Job_Generic_OnHeard(CAIMemoryLarge* pHeardMem)
{
	CAISound* pSound =  AIEnviro_SoundHandleToPtr(pHeardMem->m_uMediumData);
	if (!pSound)
	{
		return FALSE;
	}


	CFVec3A WhereLoc;
	WhereLoc.Set(pHeardMem->m_EntityLoc);

	if (!(pHeardMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED))
	{
		//
		// if 
		//    Sound was a grenade sound
		// then
		//    move away from it
		// else
		//    look in the direction of the sound
		if (pSound->m_uSoundType == AISOUNDTYPE_GRENADE)
		{	
			if (Action_DodgeGrenade(pSound->m_Origin))
			{
				return TRUE;
			}
			else
			{
				SetSoundMark(pSound);
				return FALSE;
			}
		}
		else if ((pSound->m_uSoundType == AISOUNDTYPE_BOT &&
				pSound->m_pEntity &&
				pSound->m_pEntity->GetParent() &&
				(pSound->m_pEntity->GetParent()->TypeBits() & ENTITY_BIT_VEHICLE)) ||
				(pSound->m_uSoundType == AISOUNDTYPE_DODGEME))
		{	
			if (Action_DodgeVehicle(pSound))
			{
				return TRUE;
			}
			else
			{
				SetSoundMark(pSound);
				return FALSE;
			}
		}
		else
		{	
			return Action_InvestigateSound(pHeardMem,  pSound);
		}
	}
	return FALSE;
}

BOOL CAIBrain::PlayBTAOnTakeDamage(CAIMemoryLarge* pDamagedByMem)
{
	BOOL bDidIt = FALSE;
	//
	//   Taking damage might cause ai to play an OUCH BTA
	//
	cchar* pszBTAName = NULL;

	if (!IsTalking() && 
		m_uOuchBTATimeOut < aiutils_GetCurTimeSecsU16())
	{
		char szEvent[9];
		fclib_strcpy(szEvent,"***_OUCH");


		if (pDamagedByMem->m_pEntity)
		{
			if (aiutils_IsPlayer(pDamagedByMem->m_pEntity))
			{
				szEvent[0] = 'P';
			}
			else
			{
				szEvent[0] = 'N';
			}

			if (aiutils_IsFriendly(GetAIMover()->GetEntity(), pDamagedByMem->m_pEntity))
			{
				szEvent[2] = 'F';
			}
			else
			{
				szEvent[2] = 'E';
			}
		}

		if (pszBTAName = AIBTA_EventToBTAName(szEvent, this))
		{
			if (ai_TalkModeBegin(this,
							AIBrain_TalkModeCB,
							(void*) pszBTAName,
							GetAIMover()->GetEntity(),
							kuTalkRequestTimeOutSecs,
							FALSE, //DON"T STOP
							(FVid_nFrameCounter & 1), //RANDOM LOOK or DON"T LOOK
							0))
			{
				bDidIt = TRUE;
				m_uOuchBTATimeOut = aiutils_GetCurTimeSecsU16()+_kuSecsBetweenOuchBTAPlays;
			}
		}
	}
	return bDidIt;
}


BOOL CAIBrain::Rules_Job_Generic_OnTakeDamage(CAIMemoryLarge* pDamagedByMem)
{
	BOOL bDidIt = FALSE;

	//
	//   Taking damage always wakes ai up.
	//
	if (m_pMover->IsSleeping())
	{
		m_pMover->WakeUp();
	}

	PlayBTAOnTakeDamage(pDamagedByMem);

	//
	// if damage was not Ambient, set sightmark_2stage
	//
	if (pDamagedByMem->m_uSmallData == FALSE)		//FALSE means damage was from an impact, or nearby blast
	{
		CFVec3 Where2 = pDamagedByMem->m_Loc;		//Loc contains the direction that damage came from
		Where2 *= GetAIMover()->GetRadiusXZ()*2.0f;
		Where2 += pDamagedByMem->m_EntityLoc;
		SetSightMark_1Stage(Where2, SIGHT_MARK_FLAG_BOT);
	}

	//
	// Taking damage might cause an AI to go on attack.
	//

	CEntity* pWhoDidIt = NULL;
	CAIMemoryLarge* pSeenMem = NULL;
	if (pDamagedByMem->m_pEntity &&
		m_Knowledge.CanRememberEntity(MEMORY_LARGE_SEEN, pDamagedByMem->m_pEntity, &pSeenMem) &&
		(aiutils_GetCurTimeSecsU16() - pSeenMem->m_uWhenTimeSecs < 2)   &&
		!aiutils_IsFriendly(GetAIMover()->GetEntity(), pDamagedByMem->m_pEntity))
	{
		pWhoDidIt = pDamagedByMem->m_pEntity;
	}
	bDidIt = Action_Attack(pWhoDidIt);

	return bDidIt;
}


BOOL CAIBrain::Rules_Job_Generic_OnPrompted(CAIMemorySmall* pSmallMem)
{
	BOOL bActionTaken = FALSE;
	CEntity* pHeWhoActionedMe = NULL;
	if (Player_aPlayer[pSmallMem->m_uSmallData].m_pEntityCurrent)
	{
		pHeWhoActionedMe = Player_aPlayer[pSmallMem->m_uSmallData].m_pEntityCurrent;
		if (pHeWhoActionedMe)
		{
			SetSightMark_1Stage(*(pHeWhoActionedMe->GetTagPoint(0)), SIGHT_MARK_FLAG_BOT);
			
			if (!GetLeader() &&
				GetFlag_Buddy_Ctrl_OnAction() &&
				pHeWhoActionedMe &&
				pHeWhoActionedMe->AIBrain() && 
				GetRace() == pHeWhoActionedMe->AIBrain()->GetRace())
			{
				bActionTaken = AssignLeader(pHeWhoActionedMe->AIBrain());
			}
			else if (GetLeader() && pHeWhoActionedMe->AIBrain() == GetLeader())
			{
				StopFollowing();
				ClearFlag_Buddy_Ctrl_Auto();
				SetFlag_Buddy_Ctrl_OnAction();
				bActionTaken = TRUE;

				if (!IsTalking())
				{
					cchar* pszBTAName = NULL;
					if (pszBTAName = AIBTA_EventToBTAName("P*F_NOPE", this))
					{
						ai_TalkModeBegin(this,
											AIBrain_TalkModeCB,
											(void*) pszBTAName,
											GetAIMover()->GetEntity(),
											1,
											TRUE,  //STOP/NOSTOP
											TRUE,  //LOOK AT
											pHeWhoActionedMe->Guid());
					}
				}
			}
		}
	}

	if (bActionTaken)
	{
		pSmallMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
	}
	return bActionTaken = TRUE;
}


void CAIBrain::Rules_Job_Generic_EnviroChangeRules(void)
{
	//
	// On Player Perceived Logic
	//
	CEntity* pOtherEntity = NULL;
	CAIMemoryLarge* pCollided = NULL;
	CAIMemoryLarge* pMem = NULL;
	u8 uPerceptorsCtrl = GetAttrib(ATTRIB_PERCEPTORCTRL);
	BOOL bActionTakenThisFrame = FALSE;
	CNiIterator<CAIMemoryLarge*> it;
	
	//
	// scan memory for stimuli based on changes in the environment
	//

	//seen any player bots ?
	it.Reset();
	while (	!bActionTakenThisFrame &&
			(uPerceptorsCtrl & PCFLAG_EYES_INUSE) &&
			(pMem = GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_SEEN, &it)))
	{
		pOtherEntity = pMem->m_pEntity;
		if (pOtherEntity &&
			pOtherEntity->IsInWorld() &&
			(!(pOtherEntity->TypeBits() & ENTITY_BIT_BOT) || !((CBot*) pOtherEntity)->IsDeadOrDying()) &&
			aiutils_IsPlayer(pOtherEntity))
		{
			f32 fSuspicion = 0.0f;
			f32 fGroupSuspicion = 0.0f;
			if (aiutils_IsFriendly(GetAIMover()->GetEntity(), pOtherEntity, &fSuspicion))
			{	//is it my friend
				if (fSuspicion > 0.0f)
				{
					fGroupSuspicion = fSuspicion;
					if (fSuspicion < 0.99f &&
						((aiutils_GetCurTimeSecs() - ((CBot*) pOtherEntity)->GetLastFireTime() < 1.0f) ||
						 (((CBot*) pOtherEntity)->GetCurMech() && (aiutils_GetCurTimeSecs() - ((CBot*) pOtherEntity)->GetCurMech()->GetLastFireTime() < 1.0f)))) 

					{
						fGroupSuspicion = 0.99f;
					}
					bActionTakenThisFrame = Rules_Job_Generic_OnSeenSuspiciousPlayer(pMem, &fGroupSuspicion);	 //OnSeenSuspicious can override this!
				}
				else
				{	
					bActionTakenThisFrame = Rules_Job_Generic_OnSeenFriendlyPlayer(pMem);
				}
				
				if (//fSuspicion < 0.9f &&	//test code toprevent suspicion level from automatically dropping
					GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT &&
					pOtherEntity->TypeBits() & ENTITY_BIT_BOT &&
					((CBot*) pOtherEntity)->m_nPossessionPlayerIndex > -1 &&
					Player_aPlayer[((CBot*) pOtherEntity)->m_nPossessionPlayerIndex].m_pEntityOrig != pOtherEntity)
				{
					f32 fDelta = (1.0f-fSuspicion)-(1.0f-fGroupSuspicion);
					((CBot*) GetAIMover()->GetEntity())->NotifyPossessedBotSeen(((CBot*) pOtherEntity), fDelta);
				}

			}
			else  
			{	 //enemy player seen
				bActionTakenThisFrame = Rules_Job_Generic_OnSeenEnemyPlayer(pMem);
			}

			//every stimuli gets one shot to be reacted to.
			pMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
		}
	}

	//seen any entities besides player bots
	it.Reset();
	while (	!bActionTakenThisFrame &&
			(uPerceptorsCtrl & PCFLAG_EYES_INUSE) &&
			(pMem = GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_SEEN, &it)))
	{
		pOtherEntity = pMem->m_pEntity;
		if (pOtherEntity &&
			pOtherEntity->IsInWorld() &&
			(!(pOtherEntity->TypeBits() & ENTITY_BIT_BOT) || !((CBot*) pOtherEntity)->IsDeadOrDying()) &&
			!aiutils_IsPlayer(pOtherEntity))
		{
			if (aiutils_IsFriendly(GetAIMover()->GetEntity(), pOtherEntity))
			{	//and he is my friend
				bActionTakenThisFrame = Rules_Job_Generic_OnSeenFriendlyNPC(pMem);
			}
			else  
			{	 //enemy player seen
				bActionTakenThisFrame = Rules_Job_Generic_OnSeenEnemyNPC(pMem);
			}

			//every stimuli gets one shot to be reacted to.
			pMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
		}
	}

	// Been Damaged?
	it.Reset();
	while (	!bActionTakenThisFrame &&
			(uPerceptorsCtrl & PCFLAG_TOUCH_INUSE) &&
			(pMem = GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_DAMAGED_BY, &it)))
	{
		bActionTakenThisFrame  = Rules_Job_Generic_OnTakeDamage(pMem);

		//every stimuli gets one shot to be reacted to.
		pMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
	}

	// Heard any Bots?
	it.Reset();
	while (	!bActionTakenThisFrame &&
			(uPerceptorsCtrl & PCFLAG_EARS_INUSE) &&
			(pMem = GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_HEARD, &it)))
	{
		pOtherEntity = pMem->m_pEntity;	 //may be null for a sound
		CAISound* pSound = AIEnviro_SoundHandleToPtr(pMem->m_uMediumData);
		if (pSound && pSound->m_uSoundType == AISOUNDTYPE_BOT)
		{
			if (aiutils_IsFriendly(GetAIMover()->GetEntity(), pOtherEntity))
			{	//and he is my friend
				bActionTakenThisFrame = Rules_Job_Generic_OnHeard(pMem);
			}
			else  
			{	 //enemy player seen
				bActionTakenThisFrame = Rules_Job_Generic_OnHeard(pMem);
			}

			//every stimuli gets one shot to be reacted to.
			if (bActionTakenThisFrame)
			{
				pMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
			}
		}
	}

	// Seen any projectiles?
	it.Reset();
	while (	!bActionTakenThisFrame &&
			(uPerceptorsCtrl & PCFLAG_EYES_INUSE) &&
			(pMem = GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_SEEN_PROJECTILE, &it)) &&
			!(pMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED)
			)
	{
		if (!IsSightMarkValid() || !(m_uSightMarkFlags & SIGHT_MARK_FLAG_BOT))
		{
			CFVec3A tmp;
			tmp.Set(pMem->m_EntityLoc);
			SetSightMark_2Stage(tmp, *(pMem->m_pEntity->GetTagPoint(0)), SIGHT_MARK_FLAG_PROJECTILE);

			pMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
		}
	}

	// Heard anything besides bots
	it.Reset();
	while (	!bActionTakenThisFrame &&
			(uPerceptorsCtrl & PCFLAG_EARS_INUSE) &&
			(pMem = GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_HEARD, &it)))
	{
		CAISound* pSound = AIEnviro_SoundHandleToPtr((AISoundHandle) pMem->m_uMediumData);
		if (pSound && pSound->m_uSoundType != AISOUNDTYPE_BOT)
		{
			bActionTakenThisFrame = Rules_Job_Generic_OnHeard(pMem);
			//every stimuli gets one shot to be reacted to.
			if (bActionTakenThisFrame)
			{
				pMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
			}
		}
	}

	//   
	//	 Anybody click action on me?
	//
	CAIMemorySmall* pSmallMem = NULL;
	if (!bActionTakenThisFrame	&&
		GetKnowledge().CanRememberAny(MEMORY_SMALL_PLAYERPROMPTED, &pSmallMem) &&
		(pSmallMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY) &&
		!(pSmallMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED))
	{
		FASSERT(pSmallMem->m_uSmallData >= 0 && pSmallMem->m_uSmallData < CPlayer::m_nPlayerCount);

		bActionTakenThisFrame = Rules_Job_Generic_OnPrompted(pSmallMem);
	}

	//
	// Anybody smack me?
	//
	if (!bActionTakenThisFrame	&&
		GetKnowledge().CanRememberAny(MEMORY_LARGE_COLLIDED, (CAIMemorySmall**) &pMem) &&
		pMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
	{
		if (m_pMover->IsSleeping())
		{
			m_pMover->WakeUp();
		}

		CFVec3A tmp;
		tmp.Set(pMem->m_EntityLoc);
		SetSightMark_1Stage(tmp, SIGHT_MARK_FLAG_BOT);
	}

	//
	//  What is happening to my leader 
	//
	if (!bActionTakenThisFrame)
	{
		CAIBrain* pLeaderBrain = GetLeader();
		if (pLeaderBrain)
		{
			CAIMemoryLarge* pDamagedByMem = NULL;
			if (pLeaderBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall**) &pDamagedByMem) &&
				pDamagedByMem->m_pEntity &&
				!aiutils_IsFriendly(this->GetAIMover()->GetEntity(), pDamagedByMem->m_pEntity))
			{
				bActionTakenThisFrame = Action_Attack(pDamagedByMem->m_pEntity);
			}
		}
	}


}


void CAIBrain::Rules_Job_Generic(void)
{
	Rules_Job_Generic_EnviroChangeRules();	//Reactions to stimuli

	//someday, there could be planning type rules that kick off not just because something new was detected in the environement
}


void CAIBrain::DoBehaviorRules(void)
{
	if (GetAttrib(ATTRIB_PERCEPTORCTRL) & (PCFLAG_EYES_INUSE |PCFLAG_EARS_INUSE| PCFLAG_TOUCH_INUSE | PCFLAG_RADIO_INUSE))
	{
		if ((!IsGoal(m_nCurThought) ||
				GetCurThoughtPtr()->m_uThoughtFlags & CAIThought::THOUGHTFLAG_USE_JOB_REACT_RULES))
		{
			Rules_Job_Generic();
		}
		else if (GetLeader())
		{
			//
			//	 Anybody click action on me?
			//
			CAIMemorySmall* pSmallMem = NULL;
			if (GetKnowledge().CanRememberAny(MEMORY_SMALL_PLAYERPROMPTED, &pSmallMem) &&
				pSmallMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY)
			{
				FASSERT(pSmallMem->m_uSmallData >= 0 && pSmallMem->m_uSmallData < CPlayer::m_nPlayerCount);
				Rules_Job_Generic_OnPrompted(pSmallMem);
			}
		}
	}
}


void CAIBrain::DoDecisionWork_PreThought(void)
{
	//
	// If goal ends, go back to work on your job
	//
	if (IsGoal(m_nCurThought))
	{	//currently doing a goal
		FASSERT(m_pCurThought);
		if (!IsThoughtChangePending() && 	 //there's nothing waiting to come through, and I'm out of goals, so go back to working on the job
			(m_pCurThought->IsComplete() ||	m_pCurThought->IsFailed()))
		{
			AddPendingThought(m_nJobThought, m_pJobParamPack);
		}
	}
	
	DoBehaviorRules();
}

#define HEAD_SCANRADMAX 0.5f

void CAIBrain::DoDecisionWork_PostThought(void)
{
	
	//Reaction Work
	switch (GetReactionState())
	{
		case REACTIONSTATE_FOLLOW_SUSPICIOUS:
			if (m_pCurFollowerThought && m_pCurFollowerThought->SupportsClassInterface(CLASS_CGENERICFOLLOW))
			{
				if (!GetLeader() ||
					(GetLeader()->GetAIMover()->GetEntity() != ((CBot*) GetAIMover()->GetEntity())->GetDisguisedBot()) ||
					(GetLeader() && !aiutils_IsPlayer(GetLeader()->GetAIMover()->GetEntity())))
				{
					this->StopFollowing();
					ChangeReactionState(REACTIONSTATE_NONE);
				}
			}
			break;
		case REACTIONSTATE_GRENADE_DODGE:
		case REACTIONSTATE_GRENADE_DODGE_2:
			{
				//special thing to kick us out of grenade dodge state if we don't hear an explosion
				BOOL bDodgeDone = FALSE;
				if (m_uDodgeGrenadeTimeOut < aiutils_GetCurTimeSecsU16())
				{
					bDodgeDone = TRUE;
				}
				else
				{
					// Heard anything Explosions that might signify the grenade going off
					CAIMemoryLarge* pMem = NULL;
					u8 uPerceptorsCtrl = GetAttrib(ATTRIB_PERCEPTORCTRL);
					CNiIterator<CAIMemoryLarge*> it;
					while (	(uPerceptorsCtrl & PCFLAG_EARS_INUSE) &&
							(pMem = GetKnowledge().NextUnAcknowledgeMemory(MEMORY_LARGE_HEARD, &it)))
					{
						CAISound* pSound = AIEnviro_SoundHandleToPtr((AISoundHandle) pMem->m_uMediumData);
						if (pSound && pSound->m_uSoundType == AISOUNDTYPE_EXPLOSION)
						{
							bDodgeDone = TRUE;
							break;
						}
					}
				}

				if (bDodgeDone)
				{
					ChangeReactionState(REACTIONSTATE_NONE);

					//hmmm, is this wrong, think it should be killing the react goto, not the curhtouhg gogot
					if (GetCurThought()==TT_GOTO && GetCurThoughtPtr()) 
					{
						GetCurThoughtPtr()->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;
					}
				}
			}
			break;
		case REACTIONSTATE_ATTACK:
			if (GetCurThought() != TT_COMBAT)
			{
				ChangeReactionState(REACTIONSTATE_NONE);
			}
			break;
		case REACTIONSTATE_ATTACK_IMPULSE:
			if (GetCurThought() != TT_COMBAT)
			{
				ChangeReactionState(REACTIONSTATE_NONE);
			}
			break;
		case REACTIONSTATE_DODGE_VEHICLE:
		case REACTIONSTATE_DODGE_VEHICLE_2:
			{
				//special thing to kick us out of grenade dodge state if we don't hear an explosion
				if (!GetReactPtr()) 
				{
					ChangeReactionState(REACTIONSTATE_NONE);
				}

			}
			break;
		case REACTIONSTATE_GOSEE_SOUND:
		case REACTIONSTATE_GOSEE_SOUND_2:
			//special thing to kick us out of gosee state if we get there or time out
			BOOL bDone = FALSE;
			if (m_uGoSeeSoundTimeOut < aiutils_GetCurTimeSecsU16())
			{
				bDone = TRUE;
				
				if (GetReactPtr() && GetReactPtr()->GetClassInterface() == CLASS_CGENERICGOTO) 
				{
					GetReactPtr()->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;
				}
			}
			else
			{
				
				if (!GetReactPtr() || GetReactPtr()->GetClassInterface() != CLASS_CGENERICGOTO) 
				{
					bDone = TRUE;
				}
			}

			if (bDone)
			{
				ChangeReactionState(REACTIONSTATE_NONE);

			}
			break;
	}


	//Sight Mark Work
	if (IsSightMarkValid())
	{
		if ((m_uSightMarkFlags & SIGHT_MARK_FLAG_TWO_STAGE) &&
			!(m_uSightMarkFlags & SIGHT_MARK_FLAG_SECOND_STAGE))
		{
			CFVec3A Delta1;
			Delta1.Sub(m_SightMark1, GetAIMover()->GetEyePos());
			Delta1.y = 0.0f;
			if (Delta1.SafeUnitAndMag(Delta1) > 0.0f)
			{
				if (Delta1.Dot(GetAIMover()->GetTorsoLookAtXZ()) > 0.9f)
				{	//second mark is now on the same side as mark one, so might as well look on
					m_uSightMarkFlags |= SIGHT_MARK_FLAG_SECOND_STAGE;
				}
			}
		}
		else if ((m_uSightMarkFlags & SIGHT_MARK_FLAG_TWO_STAGE) &&
				 (m_uSightMarkFlags & SIGHT_MARK_FLAG_SECOND_STAGE))
		{
			if (aiutils_GetCurTimeSecs() - m_fSightMarkTime > 4.0f)
			{
				ClearSightMark();
			}
			CFVec3A Delta1;
			Delta1.Sub(m_SightMark2, GetAIMover()->GetEyePos());
			Delta1.y = 0.0f;
			if (Delta1.SafeUnitAndMag(Delta1) > 0.0f)
			{
				if (Delta1.Dot(GetAIMover()->GetTorsoLookAtXZ()) > 0.9f)
				{	//second mark is now on the same side as mark one, so might as well look on
					ClearSightMark();
				}
			}
		}
		else if (aiutils_GetCurTimeSecs() - m_fSightMarkTime > 2.0f)
		{
			ClearSightMark();
		}
	}

	//Sound Mark Work
	if (IsSoundMarkValid())
	{
		if (aiutils_GetCurTimeSecs()-m_fSoundMarkTime > 2.0f)
		{
			ClearSoundMark();
		}
	}
	
	BOOL bLookingAtSomething = GetAIMover()->m_uMoverFlags & CAIMover::MOVERFLAG_RECEIVED_EXTERNAL_HEADLOOK_GOAL;

	if (!bLookingAtSomething && 
		IsSightMarkValid() &&
		!(IsSoundMarkValid() && m_SoundMark.m_uSoundType == AISOUNDTYPE_GRENADE) &&
		(!IsGoal(m_nCurThought) ||
		 (GetCurThoughtPtr() && !(GetCurThoughtPtr()->m_uThoughtFlags & CAIThought::THOUGHTFLAG_GOAL_HEAD_PRIORITY)) ||
		 (GetCurThoughtPtr() && GetCurThoughtPtr()->m_uThoughtFlags & CAIThought::THOUGHTFLAG_USE_JOB_REACT_RULES)) &&
		1)//!GetAIMover()->IsTalking())
	{
		f32 fForwardDot = 1.0f;
		BOOL bAligned = FALSE;

		CFVec3A SightMarkLookAt;
		GetSightMark(&SightMarkLookAt);

		if (!GetAIMover()->IsFollowingPath() ||
			GetAIMover()->IsStopped() ||
			(m_uSightMarkFlags & SIGHT_MARK_FLAG_STOP_AND_LOOK))
		{
			//
			// bot is not moving
			//

			//special sight mark says I'm stopped because of this sight mark
			if (m_uSightMarkFlags & SIGHT_MARK_FLAG_STOP_AND_LOOK)
			{
				GetAIMover()->MoveToward(GetAIMover()->GetLoc());
			}


			//
			// look
			//
			bAligned = GetAIMover()->FaceToward(SightMarkLookAt, &fForwardDot);
			if (m_uSightMarkFlags & SIGHT_MARK_FLAG_QUICK_TURN)
			{
				if (!bAligned)
				{
					GetAIMover()->m_Controls.QuickTurn();
				}
				else
				{
					m_uSightMarkFlags &=~ SIGHT_MARK_FLAG_QUICK_TURN;
				}
			}
		}
		else
		{
			//
			// Bot is moving, so only look if convenient
			//
			
			CFVec3A Delta;
			Delta.Sub(SightMarkLookAt, GetAIMover()->GetLoc());
			Delta.y = 0.0f;
			if (Delta.Dot(GetAIMover()->GetMoveDirXZ()) > 0.0f ||
				aiutils_GetCurTimeSecs() - m_fSightMarkTime < 1.0f)
			{	
				
				//sight mark is in the direction I'm moving, so might as well look
				GetAIMover()->FaceToward(SightMarkLookAt, &fForwardDot);

				if (m_uSightMarkFlags & SIGHT_MARK_FLAG_QUICK_TURN)
				{
					if (!bAligned)
					{
						GetAIMover()->m_Controls.QuickTurn();
					}
					else
					{
						m_uSightMarkFlags &=~SIGHT_MARK_FLAG_QUICK_TURN;
					}
				}
			}
		}
	}
	else if (!bLookingAtSomething && 
		IsSoundMarkValid() &&
		(!IsGoal(m_nCurThought) ||
		 (GetCurThoughtPtr() && !(GetCurThoughtPtr()->m_uThoughtFlags & CAIThought::THOUGHTFLAG_GOAL_HEAD_PRIORITY)) ||
		 (GetCurThoughtPtr() && GetCurThoughtPtr()->m_uThoughtFlags & CAIThought::THOUGHTFLAG_USE_JOB_REACT_RULES)) &&
		1)//!GetAIMover()->IsTalking())
	{
		if (!GetKnowledge().CanRememberAny(MEMORY_LARGE_HEARD) )
		{
			ClearSoundMark();
		}
		else
		{
			if (!GetAIMover()->IsFollowingPath() ||
				GetAIMover()->IsStopped())
			{
				GetAIMover()->FaceToward(GetSoundMarkLoc());
			}
			else
			{
				CFVec3A Delta;
				Delta.Sub(m_SoundMark.m_Origin, GetAIMover()->GetLoc());
				Delta.y = 0.0f;
				if (Delta.Dot(GetAIMover()->GetMoveDirXZ()) > 0.0f ||
					aiutils_GetCurTimeSecs() - m_fSightMarkTime < 1.0f)
				{	
					
					//sound mark is in the direction I'm moving, so might as well look
					GetAIMover()->FaceToward(GetSoundMarkLoc());
				}
			}
		}
	}
	else if (GetFlag_Scan())
	{
		DoScanHeadWork();
	}


	if (this->GetReactionState()==REACTIONSTATE_NONE)
	{
		m_fBrainAlertUnit -= FLoop_fPreviousLoopSecs * _kfOOAlertBlendOutTime;
		FMATH_CLAMPMIN(m_fBrainAlertUnit, 0.0f);
	}

}


void CAIBrain::DoScanHeadWork(void)	
{
	if (GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT && GetAIMover()->CanSee())
	{
		CBot* pBot = (CBot*) GetAIMover()->GetEntity();

		f32 fScanRadDir = 1.0f-(2.0f*(GetFlag_ScanDir()!=0));
		m_fHeadScan += fScanRadDir*FLoop_fPreviousLoopSecs*0.75f;
		if (m_fHeadScan > HEAD_SCANRADMAX || m_fHeadScan < -HEAD_SCANRADMAX)
		{
			ToggleFlag_ScanDir();
			FMATH_CLAMP(m_fHeadScan, -HEAD_SCANRADMAX, HEAD_SCANRADMAX);

			CFVec3A AtXZ = GetAIMover()->GetTorsoLookAtXZ();
			AtXZ.RotateY(m_fHeadScan);
			AtXZ.Mul(8.0f);
			AtXZ.Add(*(pBot->m_pApproxEyePoint_WS));

			GetAIMover()->SetHeadLookGoal_WS(AtXZ);
		}
		else
		{
			GetAIMover()->SetHeadLookGoal_WS(pBot->GetHeadLookLocation());
		}
	}

}



void CAIBrain::SetSoundMark(CAISound* pSound, u8 uSoundMarkFlags)
{
	if (pSound)
	{
		BoostBrainAlert(1.0f);

		m_SoundMark = *pSound;	//copy the whole structure
		m_uSoundMarkFlags = SOUND_MARK_FLAG_VALID | uSoundMarkFlags;

		if (!(((CBot*) GetAIMover()->GetEntity())->IsSleeping() || ((CBot*) GetAIMover()->GetEntity())->IsWaking()))
		{
			if (m_SoundMark.m_Origin.DistSq(GetAIMover()->GetLoc()) < _fQuickTurnMarkReactionDistSq)
			{
				CFVec3A Delta;
				Delta.Sub(m_SoundMark.m_Origin, GetAIMover()->GetLoc());
				Delta.y = 0.0f;
				if (Delta.Dot(GetAIMover()->GetTorsoLookAtXZ()) < 0.0f)
				{	//behind me, so do a quick turn!
					m_uSoundMarkFlags |= SOUND_MARK_FLAG_QUICK_TURN;
					Delta.SafeUnitAndMag(Delta);
					GetAIMover()->m_Controls.InitQuickTurn(Delta.Dot(GetAIMover()->GetTorsoLookAtXZ()));
					if (ShouldStartle() &&
						aiutils_GetCurTimeSecs() - GetAllQuiteTime() > kuMinAllQuietForStartle)
					{
						GetAIMover()->m_Controls.Startle();		   //Startle Anim should be used when quick turning
						GetAIMover()->m_Controls.SetFlag_Slam_RotateCW_Average();
					}
				}
			}
		}
		m_fSoundMarkTime = aiutils_GetCurTimeSecs();
	}
	//Since we just reacted to something, reset my start timeout, I should be startleable for some amount of time
	SetStartleTimeOut();
}


const CFVec3A& CAIBrain::GetSoundMarkLoc(void)
{
	FASSERT( m_uSoundMarkFlags & SOUND_MARK_FLAG_VALID);
	return m_SoundMark.m_Origin;
}


void CAIBrain::ClearSoundMark(void)
{
	m_uSoundMarkFlags = SOUND_MARK_FLAG_NONE;
}


void CAIBrain::SetSightMark_1Stage(const CFVec3A& loc1, u8 uSightMarkFlags)
{
	if (m_uSightMarkFlags & SIGHT_MARK_FLAG_PROJECTILE &&
		IsSightMarkValid())
	{	//ignore new projectile. current sight mark has higher priority
		return;
	}
	BoostBrainAlert(1.0f);

	m_uSightMarkFlags = SIGHT_MARK_FLAG_VALID | uSightMarkFlags;
	m_SightMark1 = loc1;
	m_SightMark2.Zero();	
	m_fSightMarkTime = aiutils_GetCurTimeSecs();


	if (!(((CBot*) GetAIMover()->GetEntity())->IsSleeping() || ((CBot*) GetAIMover()->GetEntity())->IsWaking()))
	{
		if (m_SightMark1.DistSq(GetAIMover()->GetLoc()) < _fQuickTurnMarkReactionDistSq)
		{
			CFVec3A Delta;
			Delta.Sub(m_SightMark1, GetAIMover()->GetLoc());
			Delta.y = 0.0f;
			if (Delta.Dot(GetAIMover()->GetTorsoLookAtXZ()) < 0.0f)
			{	//behind me, so do a quick turn!
				m_uSightMarkFlags |= SIGHT_MARK_FLAG_QUICK_TURN;
				Delta.SafeUnitAndMag(Delta);
				GetAIMover()->m_Controls.InitQuickTurn(Delta.Dot(GetAIMover()->GetTorsoLookAtXZ()));

				if (ShouldStartle())
				{
					GetAIMover()->m_Controls.Startle();		   //Startle Anim should be used when quick turning
					GetAIMover()->m_Controls.SetFlag_Slam_RotateCW_Average();
				}
			}
		}
	}

	//Since we just reacted to something, reset my start timeout, I should be startleable for some amount of time
	SetStartleTimeOut();
}


void CAIBrain::SetSightMark_2Stage(const CFVec3A& loc1, const CFVec3A& loc2, u8 uSightMarkFlags)
{
	if (m_uSightMarkFlags & SIGHT_MARK_FLAG_PROJECTILE &&
		m_uSightMarkFlags & uSightMarkFlags)
	{	//ignore sight mark setting. old one is better
		return;
	}

	BoostBrainAlert(1.0f);

	m_uSightMarkFlags = SIGHT_MARK_FLAG_VALID | uSightMarkFlags | SIGHT_MARK_FLAG_TWO_STAGE;
	m_SightMark1 = loc1;
	m_SightMark2 = loc2;	
	m_fSightMarkTime = aiutils_GetCurTimeSecs();

	CFVec3A Delta;
	CFVec3A tmp;
	GetSightMark(&tmp);
	Delta.Sub(tmp, GetAIMover()->GetLoc());
	Delta.y = 0.0f;
	if (Delta.Dot(GetAIMover()->GetTorsoLookAtXZ()) < 0.0f)
	{	//behind me, so do a quick turn!
		m_uSightMarkFlags |= SIGHT_MARK_FLAG_QUICK_TURN;
		Delta.SafeUnitAndMag(Delta);
		GetAIMover()->m_Controls.InitQuickTurn(Delta.Dot(GetAIMover()->GetTorsoLookAtXZ()));
		if (ShouldStartle())
		{
			GetAIMover()->m_Controls.Startle();		   //Startle Anim should be used when quick turning
			GetAIMover()->m_Controls.SetFlag_Slam_RotateCW_Average();
		}
		GetAIMover()->m_Controls.SetFlag_Slam_RotateCW_Average();
	}
	//Since we just reacted to something, reset my start timeout, I should be startleable for some amount of time
	SetStartleTimeOut();
}


void CAIBrain::GetSightMark(CFVec3A* pMarkPos )
{
	FASSERT((m_uSightMarkFlags & SIGHT_MARK_FLAG_VALID));
	if (m_uSightMarkFlags & SIGHT_MARK_FLAG_SECOND_STAGE) 
	{
		CFVec3A Delta1;
		Delta1.Sub(m_SightMark2, m_SightMark1);
		if (Delta1.SafeUnitAndMag(Delta1) > 0.0f)
		{
			Delta1.Mul((aiutils_GetCurTimeSecs() - this->m_fSightMarkTime)*30.0f);	//20 feet per second
			Delta1.Add(m_SightMark1);
			*pMarkPos=Delta1;
			return;
		}
	}
	*pMarkPos=m_SightMark1;
	return;
}


void CAIBrain::ClearSightMark(void)
{
	m_uSightMarkFlags = SIGHT_MARK_FLAG_NONE;
}


// Called by the AI system when it has put the bot into a state in which the talking can start.
BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 )
{
	cchar* pszBTAName = (cchar*) pvData1;
	CBot* pBot = (CBot *)(pvData2) ;

	BOOL bDidIt = FALSE;
	if (uTalkModeCBControl == TMCB_READY)
	{	//the only thing this CB cares about is the READY
		CAIBrain* pBrain = NULL;

		if (pBot && (pBrain = pBot->AIBrain()))
		{
			CBotTalkInst *pInst = CTalkSystem2::BTIPool_Get(pszBTAName);

			if( pInst && ( pBot->TypeBits() & (ENTITY_BIT_BOTCORROSIVE | ENTITY_BIT_BOTPRED) ) ) {
				// Set the radius to a very large on for corrosive...
				pInst->SetAudioRadius( 1000.0f );
			}

			if( pInst && (bDidIt = CTalkSystem2::SubmitTalkRequest( pInst, pBot )))
			{
				// The request went through.
				FASSERT(pInst);

				pInst->SetTalkDoneCB(AIBrain_TalkEndCB);	 //hey inst, if you stop, for whatever reason, please call this CB
			}
			else if (pInst)
			{
				CTalkSystem2::BTIPool_Recycle(pInst);
			}
		}
	}
	return bDidIt;
}


// Called by a CTalkInst when a talking instance has completed.
void AIBrain_TalkEndCB( CBotTalkInst *pTalkInst, CBot *pBot )
{
	CTalkSystem2::BTIPool_Recycle(pTalkInst);
	ai_TalkModeEnd(pBot->AIBrain());
}