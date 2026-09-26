
//////////////////////////////////////////////////////////////////////////////////////
// AIBrain.cpp - 
//
// Author: Pat MacKellar 
//
//
//	Note!
//  This class should only know about CEntity and CAIMover!!! All unit specific interface should go into the
//  approprate member of the CAIMover hierarchy....
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
// 04/15/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdraw.h"
#include "FCheckPoint.h"
#include "AIBrain.h"
#include "AIBrainMems.h"
#include "AIMover.h"
#include "AINodePools.h"
#include "ApeNew.h"
#include "AIGameUtils.h"
#include "AIFormations.h"
#include "AiApi.h"
#include "AIEdgeLock.h"
#include "../Entity.h" 
#include "../protrack.h"
#include "../BotTalkInst.h"

//====================
// private definitions

//=================
// public variables
BOOL CAIBrain::m_bGlobalBlindDefAndDumb = FALSE;
BOOL CAIBrain::m_bGlobalIgnoreMode = FALSE;
f32 CAIBrain::s_fDamagedByTotalDamageMultiplier = 1000.0f;

//==================
// private variables
CNiBank<CPendingThought>* CAIBrain::s_pPendingThoughtBank = NULL;
CPendingThought *  CNiIterator<CPendingThought *>::s_ReturnError;

void				*aibrain_pPostNodePoolMem = NULL;
FLinkRoot_t			aibrain_PostNodePool;
FLinkRoot_t			*aibrain_pPostNodePool = NULL;

//===================
// private prototypes

//=================
// private functions

//==================
// public functions


CAIBrain* CNiIterator<CAIBrain *>::s_ReturnError;

static cchar*  _pszHUH = "HUH?";
static cchar*  _pszInvalid = "TT_INVALID";
static cchar*  _pszOff = "TT_OFF";
cchar* CAIBrain::s_aszThoughtTypesStrings[] = 
{
// Job Thought Types
"WAIT",
"WANDER",
"PATROL",
// Goal Thought Types
"FOLLOW",
"ATTACK",
"SEARCH",
"GOTO",
"FACEIT",
"TALK",
};


cchar* CAIBrain::ThoughtTypeToString(s16 sThoughtType)
{
	if (sThoughtType >=0 && sThoughtType < NUM_THOUGHT_TYPES)
	{
		return s_aszThoughtTypesStrings[sThoughtType];
	}
	else
	{
		switch (sThoughtType)
		{
		case  TT_INVALID:
			return _pszInvalid;
		case TT_OFF:
			return _pszOff;
		}
	}
	return _pszHUH;
}


cchar* CAIBrain::s_apszBrainReactionStateStrings[]   =
{
	"RESTA:NONE",
	"RESTA:GREN_DODGE",
	"RESTA:GREN_DODGE_2",
	"RESTA:FOLLOW_SUSPICIOUS",
	"RESTA:ATTACK_IMPULSE",
	"RESTA:ATTACK",
	"RESTA:VEHICLE_DODGE",
	"RESTA:VEHICLE_DODGE_2",
	"RESTA:GOSEE_SOUND",
	"RESTA:GOSEE_SOUND_2"
};


static u16 _uBrainGUIDCounter = 1;

CAIBrain::CAIBrain(void)
:	m_pLeader(NULL),
	m_pCurThought(NULL),
	m_nJobThought(TT_INVALID),
	m_nGoalThought(TT_INVALID),
	m_nCurThought(TT_INVALID),
	m_nLastThought(TT_INVALID),
	m_uBrainGUID(0),
	m_uAttackRuleset(0),
	m_uRace(AIRACE_DROID),
	m_pJobParamPack(NULL),
	m_pCurTalk(NULL),
	m_pCurReact(NULL),
	m_pFormation(NULL),
	m_pCurFollowerThought(NULL),
	m_uLODFlags(0),
	m_uLODOverrideTimeLeft(0),
	m_fSoundMarkTime(0.0f),
	m_fSightMarkTime(0.0f),
	m_uSightMarkFlags(SIGHT_MARK_FLAG_NONE),
	m_uSoundMarkFlags(SOUND_MARK_FLAG_NONE),
	m_uBrainFlags(BRAINFLAG_ATTACKS_ENEMY | BRAINFLAG_ATTACKS_PLAYERS),
	m_fInfreqCycleRandom(0.0f),
	m_fWorldAddRandom(0.0f),
	m_fLastXYZChange(-5.0f),
	m_uLastX(0),
	m_uLastY(0),
	m_uLastZ(0),
	m_uStartleMin(10),
	m_uStartleMax(30),
	m_uOddsOfIgnoringPlayerAtEyeScanDist(70),
	m_uInnerRadiusForIgnorePlayerOdds(45),		// From this m_uIgnorePlayerOddInnerRadius to m_fEyeScanDist+m_fBonusEyeScanDistForPlayers, Odds of ignoring 0% -m_uOddsOfIgnoringPlayerAtEyeScanDist% will be squared distance interpolation
	m_uOddsOfIgnoringNearbyGrenades(0),			// When a greande sound is heard, these are the odds that a grenade reaction will NOT be done
	m_uOddsOfNotGoingToSeeSounds(0),
	m_uAttackNotifyRadius(40),					// How large a radius will the bot notify other bots at the time attack begins
	m_uMechUseageRadius(0),
	m_uMechUseageCtrl(MECH_USEAGE_CTRL_NEVER),
	m_uMechUseageTypeFlags(MECH_USEAGE_TYPE_FLAG_NONE),
	m_uMechLockSeatId(0),
	m_pMechLock(NULL),
	m_fBrainAlertUnit(0.0f),
	m_uHearBTATimeOut(0),
	m_uSeeBTATimeOut(0),
	m_uOuchBTATimeOut(0),
	m_uStartleTimeOut(0),
	m_uGoSeeSoundTimeOut(0),
	m_uDontGoSeeSoundsTimeOut(0),
	m_uEyeReactBlockerTimeOut(0),
	m_uDodgeVehicleReactBlockerTimeOut(0),
	m_uIgnoreNearbyGrenadeTimeOut(0),
	m_uDodgeGrenadeTimeOut(0),
	m_uNumWeapons(0),
	m_uWakeupRadius(0xff),
	m_uMeleeRad(0),
	m_uMeleeTimeMin(0),
	m_uMeleeTimeMax(0),
	m_pszStringTableCommGroupName(NULL),
	m_uJumpDist(0xffff)
{
	m_FollowerList.SetNodePool(aibrain_pPostNodePool);
	m_Knowledge.SetPtrListNodePoolUseage(aimain_pNodePool);
	m_PendingTalkList.SetNodePool(aimain_pNodePool);
	m_PendingReactList.SetNodePool(aimain_pNodePool);
	m_PendingThoughtList.SetNodePool(aimain_pNodePool);

	AttribStack_Init();

	m_PercepUnalert.m_fHearingMag = 1.0f;
	m_PercepUnalert.m_fBonusEyeScanDistForPlayers = 200.0f;
	m_PercepUnalert.m_fEyeCosHalfFOV = 0.30f;
	m_PercepUnalert.m_fEyeScanDist = 50.0f;

	m_PercepAlert.m_fHearingMag = 1.0f;
	m_PercepAlert.m_fBonusEyeScanDistForPlayers = 200.0f;
	m_PercepAlert.m_fEyeCosHalfFOV = 0.30f;
	m_PercepAlert.m_fEyeScanDist = 50.0f;
}


CAIBrain::~CAIBrain(void)
{
	RemoveFromWorld();
	FASSERT(m_PendingThoughtList.Size()==0);
	FASSERT(m_PendingTalkList.Size()==0);
	FASSERT(m_PendingReactList.Size()==0);
}


void CAIBrain::Create(CAIMover *pMover)
{
	m_pMover = pMover;
	m_nCurThought = TT_INVALID;
	for (s32 i = 0; i < CAIBrain::NUM_THOUGHT_TYPES; i++)
	{
		m_apThoughtBankAccessFunc[i] = NULL;
	}
	m_uBrainGUID = _uBrainGUIDCounter++;
	AttribStack_Init();
	m_uLODFlags = 0;
	m_uLODOverrideTimeLeft = 0;
	m_fSoundMarkTime = 0.0f;
	m_fSightMarkTime = 0.0f;
	m_uSightMarkFlags = 0;
	m_uReactionState = REACTIONSTATE_NONE;
	m_uSuspicionBTAPlayed = 0;
	m_uSuspicionLevel = 0;
}


void CAIBrain::AddToWorld(void)
{
	FASSERT(m_pMover);
	FASSERT(m_nCurThought == TT_INVALID);
	m_fWorldAddRandom = fmath_RandomUnitFloatLessThan1();
	m_fLastXYZChange = aiutils_GetCurTimeSecs();

	m_pMover->AddToWorld();

}

void CAIBrain::CleanupActiveThoughts(void)
{
	if (m_pCurThought)
	{
		m_pCurThought->Cleanup();

		FASSERT(m_apThoughtBankAccessFunc[m_nCurThought]);
		(*(m_apThoughtBankAccessFunc[m_nCurThought]))(m_pCurThought);	   //give back
		m_pCurThought = NULL;
		m_nCurThought = TT_INVALID;
	}

	if (m_pCurTalk)
	{
		m_pCurTalk->Cleanup();
		FASSERT(m_apThoughtBankAccessFunc[TT_TALK]);
		(*(m_apThoughtBankAccessFunc[TT_TALK]))(m_pCurTalk);	   //give back
		m_pCurTalk = NULL;
	}

	if (m_pCurReact)
	{
		u8 uThoughtType = m_pCurReact->GetThoughtType();
		m_pCurReact->Cleanup();
		FASSERT(uThoughtType >= 0 && uThoughtType < NUM_THOUGHT_TYPES && m_apThoughtBankAccessFunc[uThoughtType]);
		(*(m_apThoughtBankAccessFunc[uThoughtType]))(m_pCurReact);	   //give back
		m_pCurReact = NULL;
	}

	if (m_pCurFollowerThought)
	{
		m_pCurFollowerThought->Cleanup();
		FASSERT(m_apThoughtBankAccessFunc[TT_FOLLOW]);
		(*(m_apThoughtBankAccessFunc[TT_FOLLOW]))(m_pCurFollowerThought);	   //give back
		m_pCurFollowerThought = NULL;
	}

}

void CAIBrain::ClearBrain(void)
{
   CleanupActiveThoughts();

	if (GetMechLock())
	{
		ResLock_FreeMechSeatLock(m_pMechLock, m_uMechLockSeatId, GetGUID());
		FASSERT(!ResLock_HasMechSeatLocked((u32) GetGUID()));
		ClearMechLock();
	}
	else
	{
		FASSERT(!ResLock_HasMechSeatLocked((u32) GetGUID()));
	}
	FASSERT(!ResLock_HasEdgeLocked((u32) GetGUID()));

	CNiIterator<CPendingThought*> it = m_PendingThoughtList.Begin();
	FASSERT(s_pPendingThoughtBank);
	while (it.IsValid())
	{
		s_pPendingThoughtBank->Recycle(it.Get());
		it.Next();
	}
	m_PendingThoughtList.RemoveAll();

	it = m_PendingTalkList.Begin();
	FASSERT(s_pPendingThoughtBank);	  //not a typo. Talk lists use the thought bank.
	while (it.IsValid())
	{
		s_pPendingThoughtBank->Recycle(it.Get());
		it.Next();
	}
	m_PendingTalkList.RemoveAll();
	
	it = m_PendingReactList.Begin();
	FASSERT(s_pPendingThoughtBank);	  //not a typo. React lists use the thought bank.
	while (it.IsValid())
	{
		s_pPendingThoughtBank->Recycle(it.Get());
		it.Next();
	}
	m_PendingReactList.RemoveAll();

	m_Knowledge.ForgetAll();

	m_nLastThought = m_nCurThought;
	m_pJobParamPack = NULL;
	m_nJobThought = TT_INVALID;
	m_nCurThought = TT_INVALID;
	m_uBrainFlags = BRAINFLAG_ATTACKS_ENEMY | BRAINFLAG_ATTACKS_PLAYERS;
	m_fHeadScan = 0.0f;
	m_fSoundMarkTime = 0.0f;
	m_uSoundMarkFlags = SOUND_MARK_FLAG_NONE;
	m_fSightMarkTime = 0.0f;
	m_uSightMarkFlags = SIGHT_MARK_FLAG_NONE;
	m_SightMark1.Zero();
	m_SightMark2.Zero();
	m_uSuspicionBTAPlayed = 0;

	m_fLastXYZChange = -5.0f;
	m_uLastX = 0;
	m_uLastY = 0;
	m_uLastZ = 0;
	m_VolSlackAtLastNapJerk = 0.0f;
	m_fVolSlackTimeOut = 0.0f;
	m_uHearBTATimeOut = 0;
	m_uSeeBTATimeOut = 0;
	m_uOuchBTATimeOut = 0;
	m_uStartleTimeOut = 0;  //time at which bot will startle again
	m_uGoSeeSoundTimeOut = 0;
	m_uDontGoSeeSoundsTimeOut = 0;
	m_uEyeReactBlockerTimeOut = 0;
	m_uDodgeVehicleReactBlockerTimeOut = 0;
	m_uIgnoreNearbyGrenadeTimeOut = 0;
	m_uDodgeGrenadeTimeOut = 0;
	AttribStack_Reset();
	m_uLODFlags = m_uLODFlags & (LOD_OVERRIDE_ALWAYS | LOD_ALLOW_OVERRIDE_WHEN_CONVENIENT);
	m_uLODOverrideTimeLeft = 0;
	m_uReactionState = REACTIONSTATE_NONE;
	m_uSuspicionLevel = 0;
	m_fBrainAlertUnit = 0.0f;


	while (m_FollowerList.Size())
	{
		RemoveFollower(m_FollowerList.Begin().Get().m_pFollower);
	}
	
	if (m_pLeader)
	{
		StopFollowing();
		FASSERT(m_pLeader == NULL);
	}


}


void CAIBrain::RemoveFromWorld(void)
{
	ClearBrain();
	

	m_pMover->RemoveFromWorld();
}


BOOL CAIBrain::IsGoalChangePending(void)
{
	if (m_PendingThoughtList.Size() > 0)
	{
		CNiIterator<CPendingThought*> it = m_PendingThoughtList.Begin();
		while (it.IsValid())
		{
			if (IsGoal(it.Get()->m_nThoughtType))
			{
				return TRUE;
			}
			it.Next();
		}
	}
	return FALSE;
}


BOOL CAIBrain::AddPendingThought(s8 nThoughtType, CAIMemorySmall* pParamPack)
{
	CPendingThought* pPending = s_pPendingThoughtBank->Get();
	FASSERT(pPending);
	if (pPending)
	{
		pPending->m_uThoughtFlags = CAIThought::THOUGHTFLAG_NONE;
		pPending->m_nThoughtType = nThoughtType;
		pPending->m_pParamPack = pParamPack;
		m_PendingThoughtList.PushTail(pPending);
	}

	return pPending!=NULL;
}

BOOL CAIBrain::AddPendingReact(s8 nThoughtType, CAIMemorySmall* pParamPack)
{
	CPendingThought* pPending = s_pPendingThoughtBank->Get();
	FASSERT(pPending);
	if (pPending)
	{
		pPending->m_uThoughtFlags = CAIThought::THOUGHTFLAG_NONE;;
		pPending->m_nThoughtType = nThoughtType;
		pPending->m_pParamPack = pParamPack;
		m_PendingReactList.PushTail(pPending);
	}

	return pPending!=NULL;
}


BOOL CAIBrain::AddPendingTalk(CAIMemorySmall* pParamPack)
{
	CPendingThought* pPending = s_pPendingThoughtBank->Get();
	FASSERT(pPending);
	if (pPending)
	{
		pPending->m_uThoughtFlags = CAIThought::THOUGHTFLAG_NONE;
		pPending->m_nThoughtType = TT_TALK;
		pPending->m_pParamPack = pParamPack;
		m_PendingTalkList.PushTail(pPending);
	}

	return pPending!=NULL;
}


BOOL CAIBrain::AssignJob(s8 nThoughtType, CAIMemorySmall* pParamPack)
{
	FASSERT(IsJob(nThoughtType));
	return IsThoughtConnected(nThoughtType) && AddPendingThought(nThoughtType, pParamPack);
}


BOOL CAIBrain::AssignGoal(s8 nThoughtType, CAIMemorySmall* pParamPack)
{
	FASSERT(IsGoal(nThoughtType));
	return IsThoughtConnected(nThoughtType) && AddPendingThought(nThoughtType, pParamPack);
}


BOOL CAIBrain::AssignReact(s8 nThoughtType, CAIMemorySmall* pParamPack)
{
	return IsThoughtConnected(nThoughtType) && AddPendingReact(nThoughtType, pParamPack);
}


BOOL CAIBrain::AssignTalk(CAIMemorySmall* pParamPack)
{
	return IsThoughtConnected(TT_TALK) && AddPendingTalk(pParamPack) ;
}


BOOL CAIBrain::ForceFailureOnAllThoughtsExceptJob(void)
{
	BOOL bDidIt = FALSE;
	if (GetReactPtr())
	{
		GetReactPtr()->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;
		bDidIt = TRUE;
	}
	
	if (GetTalkPtr())
	{
		GetTalkPtr()->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;
		bDidIt = TRUE;
	}

	if (GetCurThoughtPtr() && IsGoal(GetCurThought()))
	{
		GetCurThoughtPtr()->m_uThoughtFlags |= CAIThought::THOUGHTFLAG_GOAL_FAILED;
		bDidIt = TRUE;
	}

	//to make sure I get atleast one more work to clean this mess up.
	m_uBrainFlags |= LOD_OVERRIDE_FOR_ONE_WORK;	//findfix: hopefully nothing unwanted happens in that one frame might need to add immediate thought a goal cleanup funcs
	return bDidIt;
}


void CAIBrain::StopFollowing(void)
{
	if (m_pLeader)
	{
		m_pLeader->RemoveFollower(this);
		m_pLeader = NULL;
		if (m_pCurFollowerThought)
		{
			m_pCurFollowerThought->Cleanup();
			(*(m_apThoughtBankAccessFunc[TT_FOLLOW]))(m_pCurFollowerThought);	   //give back the follow thought
			m_pCurFollowerThought = NULL;
		}
	}
}


BOOL CAIBrain::CanFollow(CAIBrain* pNewLeaderBrain)
{
	BOOL bCan = FALSE;
	if (!GetLeader() || (pNewLeaderBrain == GetLeader()))
	{
		if (GetRace() == pNewLeaderBrain->GetRace() &&
			  (GetFlag_Buddy_Ctrl_Auto() || GetFlag_Buddy_Ctrl_OnAction()))
		{
			bCan = TRUE;
		}
		else
		{
			CBot* pThisBot = (GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) ? (CBot*) GetAIMover()->GetEntity(): NULL;
			CBot* pLeaderBot = (pNewLeaderBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) ? (CBot*) pNewLeaderBrain->GetAIMover()->GetEntity() : NULL;

			if (pThisBot && pLeaderBot)
			{
				if (pThisBot->Recruit_GetRecruiter() >-1 && pThisBot->Recruit_GetRecruiter() == pLeaderBot->m_nPossessionPlayerIndex )
				{
					bCan = TRUE;
				}
			}
		}
	}
	return bCan;
}


void CAIBrain::BootUnhappyFollowers(void)
{
	CNiIterator<CAIPost> it = m_FollowerList.Begin();
	while (it.IsValid())
	{
		if (!it.Get().m_pFollower->CanFollow(this))
		{
			it.Get().m_pFollower->StopFollowing();
			it = m_FollowerList.Begin(); //restart the scan, list has changed
		}
		else
		{
			it.Next();
		}
	}

}


BOOL CAIBrain::AssignLeader(CAIBrain* pNewLeader)
{
	if (IsFollowing() && pNewLeader == this->GetLeader())
	{
		return TRUE;
	}


	if (IsFollowing())
	{
		StopFollowing();
	}
	
	if (pNewLeader &&
		pNewLeader->CanLead(this) &&
		m_pCurFollowerThought == NULL &&
		m_apThoughtBankAccessFunc[TT_FOLLOW])
	{
		if (pNewLeader->AssignFollower(this))
		{
			//this follower is getting a new leader

			m_pLeader = pNewLeader;
			FASSERT(m_pCurFollowerThought == NULL);
			FASSERT((m_apThoughtBankAccessFunc[TT_FOLLOW]));
			m_pCurFollowerThought = (*(m_apThoughtBankAccessFunc[TT_FOLLOW]))(NULL);
			if (m_pCurFollowerThought)
			{
				m_pCurFollowerThought->Init(this, CAIThought::THOUGHTFLAG_NONE, TT_FOLLOW);
			}
			else
			{
				m_pLeader->RemoveFollower(this);
			}
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CAIBrain::AssignFollower(CAIBrain* pNewFollower)
{
	if (!pNewFollower)
	{
		return FALSE;
	}

	CNiIterator<CAIPost> it = m_FollowerList.Begin();
	while (it.IsValid() && (it.Get().m_pFollower != pNewFollower))
	{
		it.Next();
	}

	if (!it.IsValid())
	{  
		//this leader is getting a new follower!
		m_FollowerList.PushHead(CAIPost(pNewFollower));

		if (!m_pFormation)
		{
			m_pFormation = AIFormations_Get();
			m_pFormation->Init(CAIFormation::FORMATIONTYPE_GLITCHBUDDIES, this);
		}

		if (m_pFormation)
		{
			m_pFormation->Join(pNewFollower);
		}


		return TRUE;
	}
	return FALSE;
}


BOOL CAIBrain::RemoveFollower(CAIBrain* pFollower)
{
	BOOL bDidit = FALSE;
	CNiIterator<CAIPost> it = m_FollowerList.Begin();
	while (it.IsValid())
	{
		if (it.Get().m_pFollower == pFollower)
		{
			it.RemoveThenBack();
			bDidit = TRUE;
			break;
		}
		it.Next();
	}


	if (m_pFormation)
	{
		m_pFormation->Quit(pFollower);
		if (m_FollowerList.Size() == 0)
		{
			AIFormations_Recycle(m_pFormation);
			m_pFormation = NULL;
		}
	}

	return bDidit;
}


void CAIBrain::DoKnowledgeWork(void)
{
	m_Knowledge.Work();
}


void CAIBrain::DoThoughtFSMWork(void)
{
	CAIThought *pOldThought = NULL;
	CAIThought *pOldTalk = NULL;
	CAIThought *pOldReact = NULL;

	//
	//	Process changes to brains current talk state
	//
	if (m_pCurTalk && (m_pCurTalk->IsFailed() || m_pCurTalk->IsComplete()))
	{
		pOldTalk = m_pCurTalk;
		if (pOldTalk->GetParamPack())
		{
			GetKnowledge().ForgetThis(pOldTalk->GetParamPack());
			pOldTalk->ClearParamPack();
		}

		pOldTalk->Cleanup();
		m_pCurTalk = NULL;
	}

	if (m_PendingTalkList.Size())
	{
		if (!pOldTalk) //check this incase we already cleard out the cur talk this frame because it failed or completed
		{
			pOldTalk = m_pCurTalk;
			if (pOldTalk)
			{
				if (pOldTalk->GetParamPack())
				{
					GetKnowledge().ForgetThis(pOldTalk->GetParamPack());
					pOldTalk->ClearParamPack();
				}
				pOldTalk->Cleanup();
				m_pCurTalk = NULL;
			}
		}
		CPendingThought* pPendingTalk = m_PendingTalkList.PopHead();
		if (pPendingTalk->m_nThoughtType && m_apThoughtBankAccessFunc[pPendingTalk->m_nThoughtType])
		{
			m_pCurTalk = (*(m_apThoughtBankAccessFunc[pPendingTalk->m_nThoughtType]))(NULL);
			if (m_pCurTalk)
			{
				m_pCurTalk->Init(this, CAIThought::THOUGHTFLAG_NONE, CAIBrain::TT_TALK, pPendingTalk->m_pParamPack);
			}
		}
		s_pPendingThoughtBank->Recycle(pPendingTalk);
		pPendingTalk = NULL;
	}

	//
	//	Process changes to brains current react state
	//
	if (m_pCurReact && (m_pCurReact->IsFailed() || m_pCurReact->IsComplete()))
	{
		pOldReact = m_pCurReact;
		if (pOldReact->GetParamPack())
		{
			GetKnowledge().ForgetThis(pOldReact->GetParamPack());
			pOldReact->ClearParamPack();
		}

		pOldReact->Cleanup();
		m_pCurReact = NULL;
	}

	if (m_PendingReactList.Size())
	{
		if (!pOldReact) //check this incase we already cleard out the cur React this frame because it failed or completed
		{
			pOldReact = m_pCurReact;
			if (pOldReact)
			{
				if (pOldReact->GetParamPack())
				{
					GetKnowledge().ForgetThis(pOldReact->GetParamPack());
					pOldReact->ClearParamPack();
				}
				pOldReact->Cleanup();
				m_pCurReact = NULL;
			}
		}
		CPendingThought* pPendingReact = m_PendingReactList.PopHead();
		if (pPendingReact->m_nThoughtType && m_apThoughtBankAccessFunc[pPendingReact->m_nThoughtType])
		{
			m_pCurReact = (*(m_apThoughtBankAccessFunc[pPendingReact->m_nThoughtType]))(NULL);
			if (m_pCurReact)
			{
				m_pCurReact->Init(this, CAIThought::THOUGHTFLAG_NONE, pPendingReact->m_nThoughtType, pPendingReact->m_pParamPack);
			}
		}
		s_pPendingThoughtBank->Recycle(pPendingReact);
		pPendingReact = NULL;
	}
	
	//
	//	Process changes to brains current follow state
	//
	if (m_pCurFollowerThought && (m_pCurFollowerThought->IsFailed() || m_pCurFollowerThought->IsComplete()))
	{
		StopFollowing();
	}


	//
	//	Process changes to our current job/goal state
	//
	if (IsThoughtChangePending())
	{
		CPendingThought* pPending = PopPendingThought();
		FASSERT(pPending);
		if (m_nCurThought >= 0 && IsJob(pPending->m_nThoughtType) && IsGoal(m_nCurThought) && !(m_pCurThought->IsComplete() || m_pCurThought->IsFailed()))
		{  //no need to wait, just switch Job thought while brain is currently working on a goal
			if (m_pJobParamPack && m_pJobParamPack != pPending->m_pParamPack)
			{  //might need to clean this out though
				GetKnowledge().ForgetThis(m_pJobParamPack);
			}
			m_nJobThought = pPending->m_nThoughtType;
			m_pJobParamPack = pPending->m_pParamPack;
			if (pPending->m_pParamPack)
			{
				FASSERT(GetKnowledge().IsInMemory(pPending->m_pParamPack));
			}
			s_pPendingThoughtBank->Recycle(pPending);
			pPending = PopPendingThought();
		}

		if (pPending)
		{
			pOldThought = m_pCurThought;
			if (pOldThought)
			{
				if (pOldThought->GetParamPack() && pOldThought->GetParamPack() != m_pJobParamPack)
				{
					//this thought's param pack can be removed from memory now
					//except for when this thought is the background job thought.
					//then, let it stay in.
					GetKnowledge().ForgetThis(pOldThought->GetParamPack());
				}
				pOldThought->Cleanup();

				m_pCurThought = NULL;
			}
			m_nLastThought = m_nCurThought;
			m_nCurThought = TT_INVALID;

			if (pPending->m_nThoughtType != TT_INVALID && m_apThoughtBankAccessFunc[pPending->m_nThoughtType])
			{
				m_nCurThought = pPending->m_nThoughtType;
				m_pCurThought = (*(m_apThoughtBankAccessFunc[m_nCurThought]))(NULL);

				//special stuff for only when the job thought changes
				if (IsJob(pPending->m_nThoughtType))
				{
					m_nJobThought = m_nCurThought;

					//store pointer to the parampack for the job thought
					if (m_pJobParamPack != pPending->m_pParamPack)
					{
						if (m_pJobParamPack)
						{
							GetKnowledge().ForgetThis(m_pJobParamPack);
							m_pJobParamPack = NULL;
						}
						m_pJobParamPack = pPending->m_pParamPack;
					}
				}
				if (pPending->m_pParamPack)
				{
					FASSERT(m_Knowledge.IsInMemory(pPending->m_pParamPack));	//why would someone be telling me to look in my memory tofind the param pack, but it not be there?
				}
				if (m_pCurThought)
				{
					m_pCurThought->Init(this, pPending->m_uThoughtFlags, pPending->m_nThoughtType, pPending->m_pParamPack);
				}
				else
				{
					m_nCurThought = TT_INVALID;
				}
				s_pPendingThoughtBank->Recycle(pPending);
				pPending = NULL;
			}
			else
			{
				if (pPending->m_pParamPack)
				{
					m_Knowledge.ForgetThis(pPending->m_pParamPack);
				}
				s_pPendingThoughtBank->Recycle(pPending);
				pPending = NULL;
			}
		}
	}

	if (m_pCurReact)
	{
		m_pCurReact->Work();
	}
	else if (m_pCurFollowerThought)
	{
		m_pCurFollowerThought->Work();	  //follow thought may or may not execute a goal thought if it feels the situation is right.
	}
	else if (m_pCurThought)
	{									  //do goal, or job
		m_pCurThought->Work();
	}
	
	if (m_pCurTalk)
	{
		m_pCurTalk->Work();
	}

	if (pOldThought)
	{
		(*(m_apThoughtBankAccessFunc[m_nLastThought]))(pOldThought); //recycle
	}

	if (pOldTalk)
	{
		(*(m_apThoughtBankAccessFunc[TT_TALK]))(pOldTalk); //recycle
	}

	if (pOldReact)
	{
		ThoughtBank_AccessFunc* pFunc = aithought_ThoughtClassTypeToAccessFunc(pOldReact->GetClassInterface());
		FASSERT(pFunc);
		(*pFunc)(pOldReact); //recycle
	}
}


void CAIBrain::UpdateInfreqCycleRandom(void)
{
	if ((aiutils_GetCurTimeSecsU16() & 5) == 5)
	{
		if (!(m_uBrainFlags & BRAINFLAG_NEW_INFREQ_RAND_TIMER)) 
		{
			m_fInfreqCycleRandom = fmath_RandomUnitFloatLessThan1();
			m_uBrainFlags |= BRAINFLAG_NEW_INFREQ_RAND_TIMER;
		}
	}
	else
	{
		m_uBrainFlags &= ~BRAINFLAG_NEW_INFREQ_RAND_TIMER;
	}


	
}

void CAIBrain::Work(void)
{
	DoUpdateMovementTrackingWork();

	CBot* pBot = NULL;
	if ((GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT))
	{
		pBot = (CBot*) GetAIMover()->GetEntity();
	}

	BOOL bBotLayerShutDown = FALSE;
	if ((pBot->IsDeadOrDying() && !pBot->IsWalkingDead()) ||
		(aiutils_IsMech(pBot) && !aiutils_GetMechOperator(pBot)))
	{
		bBotLayerShutDown = TRUE;
		CleanupActiveThoughts();	 //no need to be thinking about anything.
	}
	else if (!pBot->Power_IsPoweredUp())	 //no mech needs brain work (especially perception), unless it has a driver
	{
		bBotLayerShutDown = TRUE;
	}

	if (!bBotLayerShutDown)
	{	//no thought work for those that are dead or not fully powered up!
		PROTRACK_BEGINBLOCK("PERCEP");
		DoPerceptionWork();
		PROTRACK_ENDBLOCK();//"PERCEP");
	}
	
	DoKnowledgeWork();
	
	if (!bBotLayerShutDown)
	{	//no thought work for those that are dead or not fully powered up!
		PROTRACK_BEGINBLOCK("THOUGHT");
		DoDecisionWork_PreThought();
		DoThoughtFSMWork();

		DoDecisionWork_PostThought();
		PROTRACK_ENDBLOCK();//"THOUGHT");
	}

	if (m_pFormation)
	{  //give my formation a chance to work this frame
		m_pFormation->Work();
	}

	//might lose your lock if someone else gains access
	if (GetMechLock())
	{
		CBot* pMechOperator = aiutils_GetMechOperator(GetMechLock());
		if (pMechOperator && pMechOperator != m_pMover->GetEntity())
		{
			ResLock_FreeMechSeatLock(m_pMechLock, m_uMechLockSeatId, GetGUID());
			FASSERT(!ResLock_HasMechSeatLocked((u32) GetGUID()));
			ClearMechLock();
		}
		else if (!pMechOperator && aiutils_IsMechSeatObstructed(GetMechLock(), m_uMechLockSeatId, m_pMover->GetEntity()))
		{
			ResLock_FreeMechSeatLock(m_pMechLock, m_uMechLockSeatId, GetGUID());
			FASSERT(!ResLock_HasMechSeatLocked((u32) GetGUID()));
			ClearMechLock();
		}

	}

/*	if (m_pMover->GetCurMech() && GetMechLock() != m_pMover->GetCurMech())
	{
		FASSERT(0);
	}
  */
	if (GetAIMover()->IsDeadOrDying() && GetMechLock())
	{
		ResLock_FreeMechSeatLock(m_pMechLock, m_uMechLockSeatId, GetGUID());
		FASSERT(!ResLock_HasMechSeatLocked((u32) GetGUID()));
		ClearMechLock();
	}

	UpdateInfreqCycleRandom();

}


void CAIBrain::DebugRender(f32 fScreenX, f32 fScreenY)
{
	GetKnowledge().DebugRender(fScreenX, fScreenY);
}


const CFVec3A& CAIBrain::GetLoc(void)
{
	return m_pMover->GetLoc();
}


BOOL CAIBrain::AttribStack_Push(u8 uAttrib, u8 uValue)
{
	if (m_auAttribStackDepth[uAttrib] < ATTRIB_STACK_NUM_LEVELS-1)
	{
		m_auAttribStackDepth[uAttrib]++;
		m_aauAttribStack[uAttrib][m_auAttribStackDepth[uAttrib]] = uValue;
	    return TRUE;
	}
	return FALSE;
}


BOOL CAIBrain::AttribStack_Pop(u8 uAttrib)
{
	if (m_auAttribStackDepth[uAttrib] > 0)	//must always have something on the stack
	{
		m_aauAttribStack[uAttrib][m_auAttribStackDepth[uAttrib]] = 0xfa;  //bad value
		m_auAttribStackDepth[uAttrib]--;
	    return TRUE;
	}
	return FALSE;
}


void CAIBrain::AttribStack_Init(void)
{
	fang_MemZero(m_auAttribStackDepth, sizeof(u8)*NUM_ATTRIBS);
	fang_MemZero(m_aauAttribStack, sizeof(u8)*NUM_ATTRIBS*ATTRIB_STACK_NUM_LEVELS);		//All attribs default to zero, except.....
	SetAttrib(ATTRIB_PERCEPTORCTRL, 0xff);	//except..ATTRIB_PERCEPTORCTRL..... (ALL PERCEPTORS default to on and active)
}

void CAIBrain::AttribStack_Reset(void)
{
	fang_MemZero(m_auAttribStackDepth, sizeof(u8)*NUM_ATTRIBS);		//attribs at stack depth zero will not be reset. Assume those are "configured into the brain"
}

void CAIBrain::SetBaseSpeed(u8 uBaseSpeedPct)
{
	FMATH_CLAMP(uBaseSpeedPct, 0, 100);
	m_aauAttribStack[ATTRIB_SPEED][0] = uBaseSpeedPct;
	GetAIMover()->SetForwardGoalSpeedPctCapable((f32) GetAttrib(ATTRIB_SPEED)/100.0f);
}


void CAIBrain::PushBaseSpeedSetting(u8 uNewBaseSpeedPct)
{
	FMATH_CLAMP(uNewBaseSpeedPct, 0, 100);
	AttribStack_Push(ATTRIB_SPEED, uNewBaseSpeedPct);
	GetAIMover()->SetForwardGoalSpeedPctCapable((f32) GetAttrib(ATTRIB_SPEED)/100.0f);
}


void CAIBrain::PopBaseSpeedSetting(void)
{
	if (AttribStack_Pop(ATTRIB_SPEED))
	{
		GetAIMover()->SetForwardGoalSpeedPctCapable((f32) GetAttrib(ATTRIB_SPEED)/100.0f);
	}
}


u8 CAIBrain::GetPerceptionCtrl(void)
{
	u8 uCtrlBits = m_aauAttribStack[ATTRIB_PERCEPTORCTRL][m_auAttribStackDepth[ATTRIB_PERCEPTORCTRL]];
	if (m_bGlobalIgnoreMode)
	{
		uCtrlBits &= ~(PCFLAG_EYES_INUSE | PCFLAG_EARS_INUSE | PCFLAG_EARS_INUSE | PCFLAG_RADIO_INUSE);
	}
	if (m_bGlobalBlindDefAndDumb)
	{
		uCtrlBits &= ~(PCFLAG_EYES_ON | PCFLAG_EARS_ON | PCFLAG_EARS_ON | PCFLAG_RADIO_ON);
	}
	return uCtrlBits;
}



// checkpoint data save
BOOL CAIBrain::CheckpointSave( void )
{
	BOOL bBool;

	//---------------------------------------------
	CFCheckPoint::SaveData( m_uBrainGUID );
	CFCheckPoint::SaveData( (void*) m_aauAttribStack, NUM_ATTRIBS * ATTRIB_STACK_NUM_LEVELS );
	CFCheckPoint::SaveData( m_uRace);
	CFCheckPoint::SaveData(m_uBrainFlags & BRTAINFLAG_ONESTOSAVE);   //not all brainflags are saved and restored. some are just too situational
	//---------------------------------------------

	//---------------------------------------------
	// save attack specs
	CAIMemoryLarge* pParamPack;
	GetKnowledge().CanRememberAny(MEMORY_LARGE_ATTACKSPECS,(CAIMemorySmall**) &pParamPack);
	if( pParamPack )
	{
		bBool = TRUE;
		CFCheckPoint::SaveData( bBool );
		CFCheckPoint::SaveData( pParamPack, sizeof( CAIMemoryLarge ) );	// it's a large even though m_pJobParamPack points to small
	}
	else
	{
		bBool = FALSE;
		CFCheckPoint::SaveData( bBool );
	}
	//---------------------------------------------

	//---------------------------------------------
	// save current job
	s16 nJobThought = m_nJobThought;
	pParamPack = (CAIMemoryLarge*) m_pJobParamPack;

	if (!pParamPack && GetPendingThought() && IsJob(GetPendingThought()->m_nThoughtType))
	{
		pParamPack = (CAIMemoryLarge*) GetPendingThought()->m_pParamPack;
		nJobThought = GetPendingThought()->m_nThoughtType;
	}
	if( pParamPack )
	{
		bBool = TRUE;
		CFCheckPoint::SaveData( bBool );
		CFCheckPoint::SaveData( pParamPack,  CAIKnowledge::GetSizeBytes( pParamPack));
	}
	else
	{
		bBool = FALSE;
		CFCheckPoint::SaveData( bBool );
	}

	s16 tmp = nJobThought;
	CFCheckPoint::SaveData( tmp );
	//---------------------------------------------

	//---------------------------------------------
	// save goal
	if( m_nCurThought == TT_GOTO && GetCurThoughtPtr() && GetCurThoughtPtr()->GetParamPack() )
	{
		bBool = TRUE;
		CFCheckPoint::SaveData( bBool );
		CFCheckPoint::SaveData( GetCurThoughtPtr()->GetParamPack(), CAIKnowledge::GetSizeBytes(GetCurThoughtPtr()->GetParamPack()) );
	}
	else
	{
		bBool = FALSE;
		CFCheckPoint::SaveData( bBool );
	}
	//---------------------------------------------

	//---------------------------------------------
	// save goal
	if( IsReacting() && m_pCurReact && m_pCurReact->GetThoughtType()==TT_GOTO && m_pCurReact->GetParamPack())
	{
		bBool = TRUE;
		CFCheckPoint::SaveData( bBool );
		CFCheckPoint::SaveData( m_pCurReact->GetParamPack(), CAIKnowledge::GetSizeBytes(m_pCurReact->GetParamPack()) );
	}
	else
	{
		bBool = FALSE;
		CFCheckPoint::SaveData( bBool );
	}
	//---------------------------------------------

	// save entity ptrs
	u32 uPlayerLeaderGUID = 0;
	u32 uMechLockGUID = 0;
	u32 uMechGUID = 0;


	if (GetLeader() && aiutils_IsPlayer(GetLeader()->GetAIMover()->GetEntity()))
	{
		uPlayerLeaderGUID = GetLeader()->GetAIMover()->GetEntity()->Guid();
	}

	if (GetMechLock())
	{
		uMechLockGUID = GetMechLock()->Guid();
	}

	if (m_pMover->GetCurMech())
	{
		uMechGUID = m_pMover->GetCurMech()->Guid();
	}

	CFCheckPoint::SaveData( uPlayerLeaderGUID );
	CFCheckPoint::SaveData( uMechLockGUID );
	CFCheckPoint::SaveData( uMechGUID );

	GetKnowledge().CheckpointSave();
	return TRUE;
}

void CAIBrain::CheckpointRestore( void )
{
	BOOL bHasAttackSpecs;
	BOOL bHasJobParamPack;
	BOOL bHasGotoGoal;
	BOOL bHasReactGotoGoal;
	CEntity *pEntity;

	ClearBrain();

	pEntity = m_pMover->GetEntity();

	//---------------------------------------------
	CFCheckPoint::LoadData( m_uBrainGUID );
	CFCheckPoint::LoadData( (void*) m_aauAttribStack, NUM_ATTRIBS * ATTRIB_STACK_NUM_LEVELS );
	CFCheckPoint::LoadData(m_uRace);
	CFCheckPoint::LoadData(m_uBrainFlags);  //if the flag wasn't saved, it wasn't needed.

	//---------------------------------------------

	//---------------------------------------------
	// load attack specs
	CFCheckPoint::LoadData( bHasAttackSpecs );
	if( bHasAttackSpecs )
	{
		CAIMemoryLarge* pParamPack;
		pParamPack = GetKnowledge().RememberLarge(MEMORY_LARGE_ATTACKSPECS, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
		if( pParamPack )
		{
			CFCheckPoint::LoadData( pParamPack, sizeof( CAIMemoryLarge ) );	// it's a large even though m_pJobParamPack points to small
		}
		else
		{
			FASSERT( FALSE );
		}
	}
	//---------------------------------------------

	//---------------------------------------------
	// restore job
	CFCheckPoint::LoadData( bHasJobParamPack );
	if( bHasJobParamPack )
	{
		m_pJobParamPack = GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
		FASSERT( m_pJobParamPack );
		CFCheckPoint::LoadData( m_pJobParamPack, sizeof( CAIMemoryLarge ) );	// it's a large even though m_pJobParamPack points to small
	}
	else
	{
		FASSERT( m_pJobParamPack == NULL );
	}
	s16 tmp = m_nJobThought;
	CFCheckPoint::LoadData( tmp);
	m_nJobThought = (s8)tmp;
	if (m_pJobParamPack && IsJob(m_nJobThought))
	{
		if (!AddPendingThought( m_nJobThought, m_pJobParamPack ))
		{
			GetKnowledge().ForgetThis(m_pJobParamPack);
			m_pJobParamPack = NULL;
		}
	}
	//---------------------------------------------


	//---------------------------------------------
	// restore goto goal
	CFCheckPoint::LoadData( bHasGotoGoal );
	if( bHasGotoGoal )
	{
		CAIMemoryLarge *pMem = GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
		FASSERT( pMem );
		CFCheckPoint::LoadData( pMem, sizeof( CAIMemoryLarge ) );	// it's a large even though m_pJobParamPack points to small
		if (!AddPendingThought( TT_GOTO, pMem ))
		{
			GetKnowledge().ForgetThis(pMem);
		}
	}
	//---------------------------------------------

	//---------------------------------------------
	// restore react goto goal  (goto with atttackor something)
	CFCheckPoint::LoadData( bHasReactGotoGoal );
	if( bHasReactGotoGoal )
	{
		CAIMemoryLarge *pMem = GetKnowledge().RememberLarge(MEMORY_LARGE_PARAMPACK, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);
		FASSERT( pMem );
		CFCheckPoint::LoadData( pMem, sizeof( CAIMemoryLarge ) );	// it's a large even though m_pJobParamPack points to small
		if (!AddPendingReact( TT_GOTO, pMem ))
		{
			GetKnowledge().ForgetThis(pMem);
		}
	}
	//---------------------------------------------


	//entity ptrs to be fixed up in postamble.
	CFCheckPoint::LoadData( m_uPlayerLeaderGUID );
	CFCheckPoint::LoadData( m_uMechLockGUID );
	CFCheckPoint::LoadData( m_uMechGUID );

	GetKnowledge().CheckpointRestore();

	// clear active flag so AIBrain::AddToWorld will be happy
	ClearFlag_Active();
}

void CAIBrain::CheckpointRestorePostamble(void)
{
	if (m_uPlayerLeaderGUID)
	{
		CEntity* pLeaderToBe = CEntity::Find(m_uPlayerLeaderGUID);
		FASSERT(pLeaderToBe && pLeaderToBe->AIBrain());

		//Loop throug all my leaders followers 
		CNiIterator<CAIPost> it = pLeaderToBe->AIBrain()->m_FollowerList.Begin();
		while (it.IsValid())
		{
			CAIBrain* pFollower = it.Get().m_pFollower;
			CEntity* pFollowerEntity = pFollower->GetAIMover()->GetEntity();
			it.Next();
		}


		//I am my leaders follower.
		AssignLeader(pLeaderToBe->AIBrain());

	}

	if (m_uMechLockGUID)
	{
		CEntity* pMechLockToBe = CEntity::Find(m_uMechLockGUID);
		FASSERT(pMechLockToBe && pMechLockToBe->AIBrain());
		if (pMechLockToBe && ResLock_RequestMechSeatLock(FALSE, (CBot*) pMechLockToBe, 0, GetGUID()))
		{
			SetMechLock((CBot*) pMechLockToBe, 0);
		}
	}

	if (m_uMechGUID)
	{
	//	CEntity* pOldMech = CEntity::Find(m_uMechGUID);
	//	FASSERT(pOldMech && pOldMech == m_pMover->GetCurMech());
	}

}


//
// Statics
//

BOOL CAIBrain::InitLevel(FLinkRoot_t* pGlobalPtrNodePool)
{
	m_bGlobalIgnoreMode = FALSE;

	u32 uSizeof = sizeof(s_apszBrainReactionStateStrings);
	FASSERT(sizeof(s_apszBrainReactionStateStrings) == sizeof(cchar*) * NUM_REACTIONSTATES);
	FASSERT(sizeof(s_aszThoughtTypesStrings) == sizeof(cchar*) * NUM_THOUGHT_TYPES);

	FASSERT(s_pPendingThoughtBank == NULL);
	s_pPendingThoughtBank = APE_NEW CNiBank<CPendingThought>(pGlobalPtrNodePool, AIBRAIN_DEFAULT_GLOBAL_PENDINGTHOUGHT_POOL_COUNT, APE_NEW CPendingThought[AIBRAIN_DEFAULT_GLOBAL_PENDINGTHOUGHT_POOL_COUNT]);

	_uBrainGUIDCounter = 1;

	// initialize the CAIPost Node Pool for all m_follower lists to share
	FASSERT(aibrain_pPostNodePoolMem == NULL);
	aibrain_pPostNodePoolMem = fres_Alloc(AIBRAIN_DEFAULT_GLOBAL_FORMATIONPOST_POOL_COUNT * sizeof(CNiNode<CAIPost>) );
	if (aibrain_pPostNodePoolMem)
	{
		flinklist_InitRoot(&aibrain_PostNodePool, 0);
		flinklist_InitPool(&aibrain_PostNodePool, aibrain_pPostNodePoolMem, sizeof(CNiNode<CAIPost>), AIBRAIN_DEFAULT_GLOBAL_FORMATIONPOST_POOL_COUNT);
		aibrain_pPostNodePool = &aibrain_PostNodePool;
	}

	return s_pPendingThoughtBank && aibrain_pPostNodePoolMem;
}


void CAIBrain::UninitLevel(void)
{
	APE_DELETE(s_pPendingThoughtBank); s_pPendingThoughtBank = NULL;

	if (aibrain_pPostNodePoolMem)
	{
		//	fres_Free(aibrain_pPostNodePoolMem);
		aibrain_pPostNodePoolMem = NULL;
	}
	aibrain_pPostNodePool = NULL;

}


void CAIBrain::DoUpdateMovementTrackingWork(void)
{
	m_uBrainFlags &=~ BRAINFLAG_JUST_STOPPED;

	CEntity* pEntity = (CBot*) GetAIMover()->GetEntity();
	if (pEntity->TypeBits() & ENTITY_BIT_BOT)
	{
		if (((CBot*) pEntity)->m_fSpeed_WS > 0.1f)
		{
			if (m_uLastX != (u32) pEntity->MtxToWorld()->m_vPos.x || 
				m_uLastY != (u32) pEntity->MtxToWorld()->m_vPos.y ||
				m_uLastZ != (u32) pEntity->MtxToWorld()->m_vPos.z)
			{
				m_fLastXYZChange = aiutils_GetCurTimeSecs();
				m_uLastX = (u32) pEntity->MtxToWorld()->m_vPos.x;
				m_uLastY = (u32) pEntity->MtxToWorld()->m_vPos.y;
				m_uLastZ = (u32) pEntity->MtxToWorld()->m_vPos.z;
			}
			m_uBrainFlags |= BRAINFLAG_WAS_MOVING;
		}
		else
		{
			if (m_uBrainFlags & BRAINFLAG_WAS_MOVING)
			{
				m_uBrainFlags |= BRAINFLAG_JUST_STOPPED;
			}
			m_uBrainFlags &= ~BRAINFLAG_WAS_MOVING;
		}
	}

}


BOOL CAIBrain::HasMovedOneFootSince(f32 fTimeSecs)
{
	return (aiutils_GetCurTimeSecs() - m_fLastXYZChange < fTimeSecs);
}


BOOL CAIBrain::ShouldStartle(void)
{
	return (m_uStartleMax >= m_uStartleMin && m_uStartleTimeOut < aiutils_GetCurTimeSecsU16());
}


void CAIBrain::SetStartleTimeOut(void)
{
	m_uStartleTimeOut = aiutils_GetCurTimeSecsU16() + m_uStartleMin;
	if (m_uStartleMax > m_uStartleMin )
	{
		m_uStartleTimeOut += (u16) fmath_RandomChoice(m_uStartleMax - m_uStartleMin);
	}
}


u16 CAIBrain::GetAllQuiteTime(void)
{
	f32 fMostRecent = m_fSoundMarkTime;
	if (m_fSightMarkTime > fMostRecent)
	{
		fMostRecent = m_fSightMarkTime;
	}
	return (u16) fMostRecent;
}


s32	CAIBrain::GetFollowerRank(u32* puOutOfHowMany)
{
	if (GetLeader())
	{
		CNiIterator<CAIPost> it = GetLeader()->m_FollowerList.Begin();
		u16 uRank = 0;
		while (it.IsValid())
		{
			if (it.Get().m_pFollower == this)
			{
				return (s32) uRank;
			}
			uRank++;
			it.Next();
		}
		if (puOutOfHowMany)
		{
			*puOutOfHowMany = GetLeader()->GetNumFollowers();
		}
	}
	return -1;
}


CAIPost* CAIBrain::GetFollowerPost(void)
{
	if (GetLeader())
	{
		CNiIterator<CAIPost> it = GetLeader()->m_FollowerList.Begin();
		while (it.IsValid())
		{
			if (it.Get().m_pFollower == this)
			{
				CAIPost* pPost = NULL;

				it.Get(&pPost);
				return pPost;
			}
			it.Next();
		}
	}
	return NULL;
}

void CAIBrain::ClearMechLock(void)
{
//	FASSERT(!m_pMover->GetCurMech());
	m_pMechLock = NULL;
}


void CAIBrain::SetMechLock(CBot* pMech, u8 uSeatId)
{
	FASSERT(!m_pMechLock); 
	m_pMechLock = pMech;
	m_uMechLockSeatId = uSeatId;
}



BOOL CAIBrain::IsTalking(void)
{
	return HasTalkThought() || m_PendingTalkList.Size() > 0 || GetAIMover()->IsTalkingWithAnim() || GetAIMover()->IsTalkingWithOnlyAudio();
}