//////////////////////////////////////////////////////////////////////////////////////
// AIApi.cpp - 
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
// 05/10/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AiApi.h"
#include "AIBrainUtils.h"
#include "AIBrain.h"
#include "AIBrainMems.h"
#include "AIBrainman.h"
#include "AIMover.h"
#include "AIThoughtsGround.h"
#include "AIgameutils.h"
#include "../Entity.h"
#include "../GString.h"
#include "../TalkSystem2.h"
#include "../Bottalkdata.h"

BOOL ai_bUnarmedPlayerRuleDisabled = TRUE;


BOOL ai_AssignJob_Wait(	CAIBrain* pBrain,
						BOOL bSpecifyLookAt,		// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						const CFVec3A& LookAtLoc,	// A position to look at (Ignored if uLookAtObjGUID > 0)	
						u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uLookAroundChance)		// (0-100) Chance that obj will look around every now and again.

{
	if (!pBrain )
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
	{
		return FALSE;
	}

	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
	if (pParamPack)
	{
		StuffWaitParams(pParamPack,
						bSpecifyLookAt,
						LookAtLoc,
						uLookAtObjGUID,
						uLookAroundChance);
		if (pBrain->AssignJob(CAIBrain::TT_WAIT, pParamPack))
		{
			return TRUE;
		}
		else
		{
			pBrain->GetKnowledge().ForgetThis(pParamPack);
		}
	}
	return FALSE;
}


BOOL ai_AssignJob_Wander(	CAIBrain* pBrain,
							f32 fRadius,			// (0.0 - Inf)	Radius (in feet) to stay within while wandering
							BOOL bStayInRoom,		// (0/1)		Use radius, but always stay in the original room
							u8 uSpeedPctMin,		// (0-100)		Min Pct of this unit ability that it should travel at
							u8 uSpeedPctMax,		// (0-100)		Max             "      "
							u8 uSociable,			// (0-100)		How likely they are to stop and talk to other bots
							u8 uCuriosity,			// (0-100)		How likely they are to stop and look around at pretty things
							u8 uTakeBreakChance,	// (1-100)		a one in ?? chance that bot will take a break every now and then.
							cchar *pszPathName)	// String table Name of espline entity that contains the path points
{
	if (!pBrain )//|| !pBrain->GetAIMover()->GetEntity()->IsInWorld()) can't do this check because guys arent in world at creation time 
	{
		return FALSE;
	}

	FASSERT((!pszPathName) || (gstring_Main.FindString(pszPathName)));	 //must have a good pszpathname
	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

	if (pParamPack)
	{
		StuffWanderParams(	pParamPack,
							fRadius,			
							bStayInRoom,		
							uSpeedPctMin,	
							uSpeedPctMax,	
							uSociable,			
							uCuriosity,			
							uTakeBreakChance,
							pszPathName);	

		if (pBrain->AssignJob(CAIBrain::TT_WANDER, pParamPack))
		{
			return TRUE;
		}
		else
		{
		   pBrain->GetKnowledge().ForgetThis(pParamPack);
		}

	}
	return FALSE;
}


//
// Patrol
//	- Unit moves along a path, potentially doing something special in a custom behavior ZONE
//
BOOL ai_AssignJob_Patrol(	CAIBrain *pBrain,
							cchar *pszPathName,				// String table Name of espline entity that contains the path points
							u8 uPatrolSpeed,				// (0-100)	Pct of speed patrolling unit should travel at
							u8 uDirection,					// (0,1)	0= Clockwise, 1 = CounterClockWise
							u8 uNumIgnoreZones,				// how many elements in the next parameter
							cchar** paszIgonoreZones)		// array of names of patrol zones to ignore
{
	if (!pBrain )
	{
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
		return FALSE;
	}

	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

	if (pParamPack)
	{
		StuffPatrolParams(	pParamPack,
							pszPathName,		
							uPatrolSpeed,		
							uDirection,			
							uNumIgnoreZones,	
							paszIgonoreZones);
		if (pBrain->AssignJob(CAIBrain::TT_PATROL, pParamPack))
		{
			return TRUE;
		}
		else
		{
		   pBrain->GetKnowledge().ForgetThis(pParamPack);
		}
	}
	return FALSE;
}


BOOL ai_AssignGoal_Goto(CAIBrain* pBrain,
						const CFVec3A& GotoLoc,			// Where
						u8 uFudgeDest/* = 0*/,			// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
						u8 uSpeedPct/* = 0*/,			// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
						u16	uGotoFlags/* = 0*/)			// GOTOFLAG_*
{
	if (!pBrain )
	{
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
		return FALSE;
	}

//	uGotoFlags |= GOTOFLAG_RETURN_FIRE_OK;
	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

	if (pParamPack)
	{
		StuffGotoParams(pParamPack,
						GotoLoc,
						uFudgeDest,
						uSpeedPct,
						uGotoFlags);

		if (uGotoFlags & GOTOFLAG_RETURN_FIRE_OK)
		{
			if (pBrain->AssignReact(CAIBrain::TT_GOTO, pParamPack))
			{
				return TRUE;
			}
		}
		else
		{
			if (pBrain->AssignGoal(CAIBrain::TT_GOTO, pParamPack))
			{
				return TRUE;
			}
		}
		pBrain->GetKnowledge().ForgetThis(pParamPack);
	}
	return FALSE;
}


//
// Goto With LookAt
//
//	 - Unit will goto the specified location using pathfinder and object avoidance
//   - while looking at a something
//
BOOL ai_AssignGoal_GotoWithLookAt(	CAIBrain* pBrain,
									const CFVec3A& GotoLoc,			// Where
									const CFVec3A& LookAtLoc,		// A pt in world space to aim torso and head at.(only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
									u32	uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means none).
									u8 uPctToLookAt,				// (0-100) Percent to Goal that look at order will turn on  
									u8 uFudgeDest/* = 0*/,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
									u8 uSpeedPct/* = 0*/,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
									u16	uGotoFlags/* = 0*/)			// GOTOFLAG_*
{
	if (!pBrain )
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
	{
		return FALSE;
	}
	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

//	uGotoFlags |= GOTOFLAG_RETURN_FIRE_OK;

	if (pParamPack)
	{
		StuffGotoWithLookAtParams(pParamPack,
									GotoLoc,					// Where
									LookAtLoc,					// A pt in world space to aim torso and head at.(only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
									uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means none).
									uPctToLookAt,				// (0-100) Percent to Goal that look at order will turn on  
									uFudgeDest,			// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
									uSpeedPct ,			// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
									uGotoFlags);		// GOTOFLAG_*

		if (uGotoFlags & GOTOFLAG_RETURN_FIRE_OK)
		{
			if (pBrain->AssignReact(CAIBrain::TT_GOTO, pParamPack))
			{
				return TRUE;
			}
		}
		else
		{
			if (pBrain->AssignGoal(CAIBrain::TT_GOTO, pParamPack))
			{
				return TRUE;
			}
		}
		pBrain->GetKnowledge().ForgetThis(pParamPack);
	}
	return FALSE;
}


BOOL ai_AssignGoal_Attack(	CAIBrain* pBrain,
							u32	uEnemyGUID,
							s16 nCompletionEvent)
{
	if (!pBrain )
	{
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
		return FALSE;
	}
	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

	if (pParamPack)
	{
		StuffAttackParams(pParamPack,
						  uEnemyGUID,
						  nCompletionEvent);

		if (pBrain->AssignGoal(CAIBrain::TT_COMBAT, pParamPack))
		{
			return TRUE;
		}
		else
		{
		   pBrain->GetKnowledge().ForgetThis(pParamPack);
		}
	}
	return FALSE;
}


BOOL ai_AssignGoal_FaceIt(	CAIBrain* pBrain,
							const CFVec3A& LookAtLoc,	// A position to look at (Ignored if uLookAtObjGUID > 0)	
							u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
							u8 uTrack,					// (0-100) How hard to try to track LookAt it if moving
							u8 uTimeOut,				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
							s16 nCompletionEvent)		// A scripting event to call apon completion or failure
{
	if (!pBrain )
	{
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
		return FALSE;
	}
	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
	if (pParamPack)
	{
		StuffFaceItParams(	pParamPack,
							LookAtLoc,
							uLookAtObjGUID,
							uTrack,
							uTimeOut);
		if (pBrain->AssignGoal(CAIBrain::TT_FACEIT, pParamPack))
		{
			return TRUE;
		}
		else
		{
			pBrain->GetKnowledge().ForgetThis(pParamPack);
		}
	}
	return FALSE;
}

BOOL ai_FollowLeader(	CAIBrain* pBrain,
						u32	uLeaderGUID,			// The GUID of an entity to follow
						s16 nCompletionEvent)		// A scripting event to call apon completion or failure
{
	if (!pBrain )
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
	{
		return FALSE;
	}

	CAIBrain* pLeader = NULL;
	if (uLeaderGUID)
	{
		CEntity* pLeaderEntity = CEntity::Find(uLeaderGUID);
		pLeader = pLeaderEntity->AIBrain();
	}
	pBrain->AssignLeader(pLeader);	//pLeader = NULL to indicate don't follow anybody

	return FALSE;
}

//Some defaults for Coliseum.
BOOL ai_WeaponChanged( CAIBrain *pBrain, CBotBuilder::NPCWeapon_e eWpn )
{
	if (!pBrain)
		//can't do this switch because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
	{
		return FALSE;
	}

	CAIWeaponCtrl *pWpnCtrl = pBrain->GetAIMover()->GetWeaponCtrlPtr(0);

	switch (eWpn)
	{
		case CBotBuilder::_NPC_WEAPON_LASER:
				pWpnCtrl->m_fAccuracy = 1.0f;
				pWpnCtrl->m_fBurstDelay = 0.0f;
				pWpnCtrl->m_fBurstDelayBonus = 0.0f;
				pWpnCtrl->m_fFireDelay = 0.0f;
				pWpnCtrl->m_fFireDelayBonus = 0.0f;
				pWpnCtrl->m_uNumFiresPerBurst = 100;
				pWpnCtrl->m_uNumFiresPerBurstBonus = 7;

				pWpnCtrl->m_uCtrlFlags |= CAIWeaponCtrl::CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
			break;
		case CBotBuilder::_NPC_WEAPON_SPEW:
				pWpnCtrl->m_fAccuracy = 1.0f;
				pWpnCtrl->m_fBurstDelay = 0.0f;
				pWpnCtrl->m_fBurstDelayBonus = 0.0f;
				//pWpnCtrl->m_fFireDelay = 0.01f;
				//pWpnCtrl->m_fFireDelayBonus = 3.032f;
				pWpnCtrl->m_fFireDelay = 0.0f;
				pWpnCtrl->m_fFireDelayBonus = 0.0f;
				pWpnCtrl->m_uNumFiresPerBurst = 100;
				pWpnCtrl->m_uNumFiresPerBurstBonus = 3;

				pWpnCtrl->m_uCtrlFlags |= CAIWeaponCtrl::CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
			break;
		case CBotBuilder::_NPC_WEAPON_FLAMER:
				pWpnCtrl->m_fAccuracy = 1.0f;
				pWpnCtrl->m_fBurstDelay = 0.0f;
				pWpnCtrl->m_fBurstDelayBonus = 0.0f;
				//pWpnCtrl->m_fFireDelay = 2.0f;
				//pWpnCtrl->m_fFireDelayBonus = 2.0f;
				pWpnCtrl->m_fFireDelay = 0.0f;
				pWpnCtrl->m_fFireDelayBonus = 0.0f;
				pWpnCtrl->m_uNumFiresPerBurst = 3;
				pWpnCtrl->m_uNumFiresPerBurstBonus = 7;

				pWpnCtrl->m_uCtrlFlags |= CAIWeaponCtrl::CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
			break;
		case CBotBuilder::_NPC_WEAPON_BLASTER:
				pWpnCtrl->m_fAccuracy = 1.0f;
				pWpnCtrl->m_fBurstDelay = 0.0f;
				pWpnCtrl->m_fBurstDelayBonus = 0.0f;
				pWpnCtrl->m_fFireDelay = 0.05f;
				pWpnCtrl->m_fFireDelayBonus = 0.1f;
				pWpnCtrl->m_uNumFiresPerBurst = 48;
				pWpnCtrl->m_uNumFiresPerBurstBonus = 3;

				pWpnCtrl->m_uCtrlFlags |= CAIWeaponCtrl::CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
			break;
		case CBotBuilder::_NPC_WEAPON_RIVET:
				pWpnCtrl->m_fAccuracy = 1.0f;
				pWpnCtrl->m_fBurstDelay = 0.0f;
				pWpnCtrl->m_fBurstDelayBonus = 0.0f;
				pWpnCtrl->m_fFireDelay = 0.01f;
				pWpnCtrl->m_fFireDelayBonus = 0.02f;
				pWpnCtrl->m_uNumFiresPerBurst = 10;
				pWpnCtrl->m_uNumFiresPerBurstBonus = 7;

				pWpnCtrl->m_uCtrlFlags |= CAIWeaponCtrl::CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
			break;
		default:
			pWpnCtrl->m_fAccuracy = 1.0f;
			pWpnCtrl->m_fBurstDelay = 0.0f;
			pWpnCtrl->m_fBurstDelayBonus = 0.0f;
			//pWpnCtrl->m_fFireDelay = 0.15f;
			//pWpnCtrl->m_fFireDelayBonus = 0.05f;
			pWpnCtrl->m_fFireDelay = 0.0f;
			pWpnCtrl->m_fFireDelayBonus = 0.0f;
			pWpnCtrl->m_uNumFiresPerBurst = 3;
			pWpnCtrl->m_uNumFiresPerBurstBonus = 7;
			break;
	}

	return TRUE;
}

BOOL ai_TalkModeBegin(CAIBrain* pBrain,
					  TalkModeCallback pFunc,
					  void *pvData1,
					  void *pvData2,
					  u8 uSecs,
					  BOOL bNeedtoStop,
					  BOOL bNeedToFace,
					  u32 uLookAtObjGUID)
{
	if ( !pBrain )
		//can't do this check because guys arent in world at creation time || !pBrain->GetAIMover()->GetEntity()->IsInWorld())
	{
		return FALSE;
	}
	CAIMemoryLarge* pParamPack = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);


	if (!bNeedtoStop && pFunc)
	{
		CBotTalkData *pData = CTalkSystem2::GetTalkDataByFileName((cchar*)pvData1);
		if (pData && pData->DrivesLowerBody())
		{
			bNeedtoStop = TRUE;
		}
	}


	if (pParamPack)
	{
		StuffTalkToParams(	pParamPack,
							uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
							uSecs,			// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
							bNeedtoStop,
							(void *)pFunc,
							pvData1,
							pvData2);

		if (pBrain->AssignTalk(pParamPack))
		{
			return TRUE;
		}
		else
		{
		   pBrain->GetKnowledge().ForgetThis(pParamPack);
		}
	}
	return FALSE;
}


BOOL ai_TalkModeEnd(CAIBrain* pBrain)
{
	if (pBrain && pBrain->HasTalkThought() && pBrain->GetTalkPtr())
	{
		return pBrain->GetTalkPtr()->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_COMPLETE;
	}
	return FALSE;
}


BOOL ai_IsTalking(CAIBrain* pBrain)
{
	if (pBrain)
	{
		return pBrain->HasTalkThought() || pBrain->GetAIMover()->IsTalkingWithAnim();
			;
	}
	return FALSE;
}


//////////////////////////////////////////////////  Perception Control ///////////////////////////////////////////

u8 _auPercepOnFlagMapping[4] =
{
	CAIBrain::PCFLAG_EYES_ON,
	CAIBrain::PCFLAG_EARS_ON,
	CAIBrain::PCFLAG_TOUCH_ON,
	CAIBrain::PCFLAG_RADIO_ON
};

u8 _auPercepInuseFlagMapping[4] =
{
	CAIBrain::PCFLAG_EYES_INUSE,
	CAIBrain::PCFLAG_EARS_INUSE,
	CAIBrain::PCFLAG_TOUCH_INUSE,
	CAIBrain::PCFLAG_RADIO_INUSE,
};


void ai_TurnOnPerceptor(CAIBrain* pBrain, u8 uPerceptorId)
{
	FASSERT(sizeof(_auPercepInuseFlagMapping)/sizeof(u8) == AI_NUM_PERCEPTORS);
	FASSERT(sizeof(_auPercepOnFlagMapping)/sizeof(u8) == AI_NUM_PERCEPTORS);

	if (pBrain)
	{
		FASSERT(uPerceptorId>=0 && uPerceptorId < AI_NUM_PERCEPTORS);
		pBrain->SetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL, pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL) | _auPercepOnFlagMapping[uPerceptorId]);
	}
}


void ai_TurnOffPerceptor(CAIBrain* pBrain, u8 uPerceptorId)
{

	if (pBrain)
	{
		FASSERT(uPerceptorId>=0 && uPerceptorId < AI_NUM_PERCEPTORS);
		pBrain->SetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL, pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL) & ~_auPercepOnFlagMapping[uPerceptorId]);
	}
}


void ai_UsePerceptor(CAIBrain* pBrain, u8 uPerceptorId)
{
	if (pBrain)
	{
		FASSERT(uPerceptorId>=0 && uPerceptorId < AI_NUM_PERCEPTORS);
		pBrain->SetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL, pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL) | _auPercepInuseFlagMapping[uPerceptorId]);
	}
}


void ai_IgnorePerceptor(CAIBrain* pBrain, u8 uPerceptorId)
{
	if (pBrain)
	{
		FASSERT(uPerceptorId>=0 && uPerceptorId < AI_NUM_PERCEPTORS);
		pBrain->SetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL, pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL) & ~_auPercepInuseFlagMapping[uPerceptorId]);
	}
}


BOOL ai_IsPerceptorOn(CAIBrain* pBrain, u8 uPerceptorId)
{
	if (pBrain)
	{
		FASSERT(uPerceptorId>=0 && uPerceptorId < AI_NUM_PERCEPTORS);
		return pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL) & _auPercepOnFlagMapping[uPerceptorId];
	}
	return FALSE;
}


BOOL ai_IsPerceptorInUse(CAIBrain* pBrain, u8 uPerceptorId)
{
	if (pBrain)
	{
		FASSERT(uPerceptorId>=0 && uPerceptorId < AI_NUM_PERCEPTORS);
		return pBrain->GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL) & _auPercepInuseFlagMapping[uPerceptorId];
	}
	return FALSE;
}


f32 ai_GetBrainAlertUnit(CAIBrain* pBrain)
{
	if( pBrain )
	{
		return pBrain->GetBrainAlertUnit();
	}
	else
	{
		return 0.0f;
	}
}

//////////////////////////////////////////////////  Brain Attributes ///////////////////////////////////////////////
u8 ai_GetRace(CAIBrain* pBrain)
{
	if (pBrain)
	{
		return pBrain->GetRace();
	}
	return AIRACE_DROID	;
}


void ai_SetRace(CAIBrain* pBrain, u8 uAIRace)	  // uAIRace is from enum above
{
	if (pBrain)
	{
		FASSERT(uAIRace >=0 && uAIRace < NUM_AIRACES);
		pBrain->SetRace(uAIRace);
	}
}


void ai_SetBaseSpeed(	CAIBrain* pBrain,
						u8 uSpeedPct)	// (0-100) base speed	(0 means stop!)
{
	if (pBrain)
	{
		pBrain->SetBaseSpeed(uSpeedPct);
	}
}


void ai_ClearAttackSpecs(CAIBrain* pBrain,
						  BOOL bResetCurrentAttack)		// if brain is currently on attack, do you want to reset the attack to use default attack specs?		
{
	if (pBrain)
	{
		pBrain->GetKnowledge().ForgetAllId(MEMORY_LARGE_ATTACKSPECS);
		if (bResetCurrentAttack && pBrain->GetCurThought() == CAIBrain::TT_COMBAT)
		{
			//findfix: it would be ideal to have a way to re-init the attack order, withought 
			//gouging it entirely.
			ai_AssignGoal_Goto(pBrain, pBrain->GetAIMover()->GetLoc(), 100, -1, FALSE);
		}
	}
}


void ai_SetAttackSpecs( CAIBrain* pBrain,
						u8 uNormalRuleSet,			// which ruleset the brain will use when attacking
						u8 uAlternateRuleSet,			// an alternate one
						u8 uAlternateRuleSetOdds,		// (0-100) pct chance that the alternate will be used
						u16 uAttackMaxDistFromOrigin,		// Limit the distance traveled from location at the time attack mode turns on
						u16 uStayPutDist,					// stay local until enemy is atleast this close
						u8 uAttackSpeedMin,					// (0-100) how fast to move while in attack mode (min)
						u8 uAttackSpeedMax,					// (0-100) how fast to move while in attack mode (max)
						BOOL bResetCurrentAttack)		// if brain is currently on attack, do you want to reset the attack to use default attack specs?		
{
	if (pBrain)
	{
		pBrain->GetKnowledge().ForgetAllId(MEMORY_LARGE_ATTACKSPECS);

		CAIMemoryLarge* pAttackSpec = pBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_ATTACKSPECS, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
		if (pAttackSpec)
		{
			StuffAttackSpecs(	pAttackSpec,
								uNormalRuleSet,	 // which ruleset the brain will use when attacking
								uAlternateRuleSet,
								uAlternateRuleSetOdds, //(0-100) pct chance that the alternate will be used
								uAttackSpeedMin,  //(0-100)
								uAttackSpeedMax,
								uAttackSpeedMax,
								uStayPutDist,
								0xff);
		}
		
		if (bResetCurrentAttack && pBrain->GetCurThought() == CAIBrain::TT_COMBAT)
		{
			//findfix: it would be ideal to have a way to re-init the attack order, withought 
			//gouging it entirely.
			ai_AssignGoal_Goto(pBrain, pBrain->GetAIMover()->GetLoc(), 100, -1, FALSE);
		}
	}
}


void ai_NotifyCutSceneBegin(void)
{
	CAIBrain::m_bGlobalIgnoreMode = TRUE;
	aibrainman_StopAttacking();
}


void ai_NotifyCutSceneEnd(void)
{
	CAIBrain::m_bGlobalIgnoreMode = FALSE;
}


void ai_EnableUnarmedPlayerRule(void)
{
	ai_bUnarmedPlayerRuleDisabled = FALSE;

}


void ai_DisableUnarmedPlayerRule(void)
{
	ai_bUnarmedPlayerRuleDisabled = TRUE;
}

															
BOOL ai_IsUnarmedPlayerRuleEnabled(void)
{
	return !ai_bUnarmedPlayerRuleDisabled;
}


CEntity* ai_GetLastEnemySeen( CAIBrain* pBrain,
							  CFVec3A* pLastSeenLoc,
							  BOOL *pbNewlySeen /*= NULL*/,
							  u16 *puWhenLastSeenSec/* =NULL*/) 
{
	CEntity* pIntruder = NULL;

	if (pBrain &&
		pBrain->GetCurThoughtPtr() &&
		pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CSCOUTALERT))
	{
		CScoutAlert* pAlertThought = (CScoutAlert*) pBrain->GetCurThoughtPtr();
		if (pAlertThought->m_pIntruder)
		{
			CAIMemoryLarge* pSeenMem = NULL;
			if (pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, pAlertThought->m_pIntruder, &pSeenMem))
			{
				if (pLastSeenLoc)
				{
					pLastSeenLoc->v3 = pSeenMem->m_EntityLoc;
				}
				pIntruder = pAlertThought->m_pIntruder;
			}
		}
	}
	
	if (!pIntruder &&
		pBrain &&
		pBrain->GetCurThoughtPtr() &&
		pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT))
	{
		CGroundCombat* pCombatThought = (CGroundCombat*) pBrain->GetCurThoughtPtr();
		if (pCombatThought->m_pEnemy && pCombatThought->m_pEnemy->IsInWorld() && !pCombatThought->m_pEnemy->IsMarkedForWorldRemove())
		{
			if (pCombatThought->m_fTimeWithoutLOS < 1.0f)
			{
				if (pLastSeenLoc)
				{
				   *pLastSeenLoc = pCombatThought->m_LastEnemyMark;
				}
			   pIntruder = pCombatThought->m_pEnemy;
			}
			else
			{
				CAIMemoryLarge* pSeenMem = NULL;
				if (pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, pCombatThought->m_pEnemy, &pSeenMem))
				{
					if (pLastSeenLoc)
					{
						pLastSeenLoc->v3 = pSeenMem->m_EntityLoc;
					}
					pIntruder = pCombatThought->m_pEnemy;
				}
			}
		}
	}
	
	if (!pIntruder)
	{
		CNiIterator<CAIMemoryLarge*> Search = pBrain->GetKnowledge().SearchLarge();
		while (Search.IsValid())
		{
			CAIMemoryLarge* pMem = Search.Get();
			if (pMem->m_uMemoryId == MEMORY_LARGE_SEEN &&
				!aiutils_IsFriendly(pBrain->GetAIMover()->GetEntity(), pMem->m_pEntity))
			{
				if (pLastSeenLoc)
				{
					pLastSeenLoc->v3 = pMem->m_EntityLoc;
				}
				pIntruder = pMem->m_pEntity;

				if (pbNewlySeen)
				{
					*pbNewlySeen = !!(pMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY);
				}

				if (puWhenLastSeenSec) 
				{
					*puWhenLastSeenSec = pMem->m_uWhenTimeSecs;
				}

				break;	//found the vis-check, quit searching
			}
			Search.Next();
		}
	}

	return pIntruder;
}


BOOL ai_IsAttacking(CAIBrain* pBrain)
{
	BOOL bAttacking = FALSE;
	if (pBrain &&
		(pBrain->IsGoalChangePending() ||
		pBrain->GetCurThought() == CAIBrain::TT_COMBAT))
	{
		bAttacking = TRUE;
	}
	return bAttacking;
}


CBot* ai_GetMechLock(CAIBrain* pBrain)
{
	return pBrain->GetMechLock();
}

//
//	AI Buddy Ctrl
//
void ai_ChangeBuddyCtrl(CAIBrain* pBrain, u8 uBuddyCtrl)	  //change buddy ctrl.... will break formation or connect up follow leader if appropriate
{


	switch (uBuddyCtrl)
	{
		case AI_BUDDY_CTRL_NEVER:
			pBrain->ClearFlag_Buddy_Ctrl_Auto();
			pBrain->ClearFlag_Buddy_Ctrl_OnAction();
			pBrain->AssignLeader(NULL);
			break;
		case AI_BUDDY_CTRL_AUTO:
			pBrain->SetFlag_Buddy_Ctrl_Auto();
			break;
		case AI_BUDDY_CTRL_ONACTION:
			pBrain->SetFlag_Buddy_Ctrl_OnAction();
			break;
	}
}

void ai_ResetToJob(CAIBrain* pBrain)
{
	if (pBrain)
	{
		pBrain->ForceFailureOnAllThoughtsExceptJob();
	}
}
