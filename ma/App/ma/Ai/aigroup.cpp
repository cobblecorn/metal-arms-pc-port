//////////////////////////////////////////////////////////////////////////////////////
// aigroup.cpp - 
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
// 09/15/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "aigroup.h"
#include "ftl.h"
#include "fscriptsystem.h"
#include "AIBrain.h"
#include "AIBrainman.h"
#include "AiBrainMems.h"
#include "AIGameUtils.h"
#include "AIMover.h"
#include "AIThoughtsGeneric.h"
#include "AIThoughtsGround.h"
#include "AI/AiApi.h"
#include "AI/AIMain.h"
#include "Ai\AINodePools.h"
#include "AI\AIEdgeLock.h"

#include "../Entity.h"
#include "../Player.h"
#include "../Site_BotWeapon.h"		  //needed since there is no cbot access to the driver of a pill
#include "../Vehicle.h"				  //needed since there is no cbot access to the driver of a vehicle


FCLASS_ALIGN_PREFIX class CGroupAttackContext
{
public:
	CGroupAttackContext(void);
	~CGroupAttackContext(void);

	void ClearData(void);
	void Work(void);
	u32 m_uLastUpdateFrame;
	u16 m_uAttackerCount;
	u16 m_uLocalAllies;

	FCLASS_STACKMEM_ALIGN(CGroupAttackContext);
} FCLASS_ALIGN_SUFFIX;

CGroupAttackContext::CGroupAttackContext(void)
{
	ClearData();
}


CGroupAttackContext::~CGroupAttackContext(void)
{
}


void CGroupAttackContext::ClearData(void)
{
	m_uAttackerCount = 0;
	m_uLastUpdateFrame = 0;
	m_uLocalAllies = 0;
}


void CGroupAttackContext::Work(void)
{
	if (m_uAttackerCount && (FVid_nFrameCounter - m_uLastUpdateFrame) > 60)
	{
		m_uAttackerCount = 0;
	}
}



//
// CGroupVisQueryContext
//   - Vis Query Information
//
//
FCLASS_ALIGN_PREFIX class CGroupVisQueryContext
{
public:
	CGroupVisQueryContext(void);
	~CGroupVisQueryContext(void);

	void ClearData(void);
	void Work(u32 uPlayerId);

	f32 CalcLightValueAt(u32 uPlayerId);


	CFVec3A m_MountAtLastQueryTime;
	CGenericWait m_LookAtMgr;
	u32 m_uLastLightQueryFrame;
	f32 m_fLightValueAt;
} FCLASS_ALIGN_SUFFIX;

const f32 _PLAYERBRAIN_VISRAYSCAN_LOS_DIST = 3.0f;


CGroupVisQueryContext::CGroupVisQueryContext(void)
{
	ClearData();
}


CGroupVisQueryContext::~CGroupVisQueryContext(void)
{
}


void CGroupVisQueryContext::ClearData(void)
{
	m_MountAtLastQueryTime.Zero();
	m_uLastLightQueryFrame = 0;
	m_fLightValueAt = 0.0f;
}


f32 CGroupVisQueryContext::CalcLightValueAt(u32 uPlayerId)
{
	CAIBrain* pPlayerBrain = NULL;
	if (!(Player_aPlayer[uPlayerId].m_pEntityCurrent &&
		(pPlayerBrain = Player_aPlayer[uPlayerId].m_pEntityCurrent->AIBrain())))
	{
		return 1.0f;
	}

	if (m_uLastLightQueryFrame < FVid_nFrameCounter)
	{
		m_uLastLightQueryFrame = FVid_nFrameCounter;
		if (pPlayerBrain->HasMovedOneFootSince(0.5f))
		{
			m_MountAtLastQueryTime = pPlayerBrain->GetAIMover()->GetLoc();
			CFColorRGB LocalColor;
			if (fworld_LightPoint(&m_MountAtLastQueryTime, &LocalColor, TRUE))
			{
				m_fLightValueAt = LocalColor.CalcIntensity();
			}
			else
			{
				m_fLightValueAt = 1.0f;

			}
		}
	}

	return m_fLightValueAt;

}


void CGroupVisQueryContext::Work(u32 uPlayerId)
{
	if (!(Player_aPlayer[uPlayerId].m_pEntityCurrent &&
		Player_aPlayer[uPlayerId].m_pEntityCurrent->AIBrain()))
	{
		return;
	}

	if (!m_LookAtMgr.GetBrain() && Player_aPlayer[uPlayerId].m_pEntityCurrent->AIBrain())
	{
		m_LookAtMgr.Init(Player_aPlayer[uPlayerId].m_pEntityCurrent->AIBrain(), CAIThought::THOUGHTFLAG_NONE, CAIBrain::TT_INVALID);
		m_LookAtMgr.m_uWaitFlags |= CGenericWait::WAIT_VISRAYS_ENABLED;
		//retest a vis-ray every four frames
		m_LookAtMgr.m_uWaitFlags |= CGenericWait::WAIT_TIMER_UPDATE_VISRAYS;
		m_LookAtMgr.m_uLOSFrameDelay = 4;
	}
	if (Player_aPlayer[uPlayerId].m_pEntityCurrent->AIBrain()->HasMovedOneFootSince(1.0f))
	{
		m_LookAtMgr.UpdateVisRays(*(Player_aPlayer[uPlayerId].m_pEntityCurrent->GetTagPoint(0)), Player_aPlayer[uPlayerId].m_pEntityCurrent->AIBrain()->GetAIMover()->GetRadiusXZ()+_PLAYERBRAIN_VISRAYSCAN_LOS_DIST);
	}
}



FCLASS_ALIGN_PREFIX class CSuspicionGroupContext
{
public:
	CSuspicionGroupContext(void);
	~CSuspicionGroupContext(void);

	void ClearData(void);
	void Work(void);

	u32 m_uLastUpdateFrame;
	f32 m_fMaxSuspicion;
	u16 m_uMemberCount;
	CAIBrain* m_pLeader;			//Leader is that bot which is closest, of all bots that are supsicios, to the player bot in question

	FCLASS_STACKMEM_ALIGN(CSuspicionGroupContext);
} FCLASS_ALIGN_SUFFIX;


CSuspicionGroupContext::CSuspicionGroupContext(void)
{
	ClearData();
}


CSuspicionGroupContext::~CSuspicionGroupContext(void)
{
}


void CSuspicionGroupContext::ClearData(void)
{
	m_uMemberCount = 0;
	m_uLastUpdateFrame = 0;
	m_pLeader = NULL;
	m_fMaxSuspicion = 0.0f;
}


void CSuspicionGroupContext::Work(void)
{
	if (m_uMemberCount && (FVid_nFrameCounter - m_uLastUpdateFrame) > 60)	   //eh?   Kill it after 60 frames (findfix: could be one sec, could be 4, or 10.)
	{
		m_uMemberCount = 0;
		m_pLeader = NULL;
	}
}




//
//	Group Level Queries
//
//

CGroupAttackContext _aGAContext[MAX_PLAYERS];
CGroupVisQueryContext _aGVQContext[MAX_PLAYERS];
CSuspicionGroupContext _aGSContext[MAX_PLAYERS];
CNiList<CBot*> _ActiveMechList;				 //List of all Mech that have registered with the AI system as a operatable thing
CBot* CNiIterator<CBot*>::s_ReturnError;
CNiList<CEntity*> _AvoidanceList;
BOOL _bSystemInitialized = FALSE;
BOOL aigroup_InitSystem(void)
{
	FASSERT(_AvoidanceList.Size()==0);
	FASSERT(_ActiveMechList.Size() == 0);
	_ActiveMechList.SetNodePool(aimain_pNodePool);
	_AvoidanceList.SetNodePool(aimain_pNodePool);
	_bSystemInitialized = TRUE;
	return TRUE;
}


void aigroup_UninitSystem(void)
{
	_ActiveMechList.RemoveAll();
	_AvoidanceList.RemoveAll();
	_bSystemInitialized = FALSE;
}


BOOL aigroup_InitLevel(void)
{
	u32 i = 0; 
	for (i=0; i < (u32)CPlayer::m_nPlayerCount; i++)
	{
		_aGAContext[i].ClearData();
		_aGVQContext[i].ClearData();
		_aGSContext[i].ClearData();
	}

	FASSERT(_AvoidanceList.Size() == 0);
	return TRUE;
}


void aigroup_CheckpointRestore(void)
{
	u32 i = 0; 
	for (i=0; i < (u32)CPlayer::m_nPlayerCount; i++)
	{
		_aGAContext[i].ClearData();
		_aGVQContext[i].ClearData();
		_aGSContext[i].ClearData();
	}
}

void aigroup_UninitLevel(void)
{
	_AvoidanceList.RemoveAll();
}


void aigroup_Work(void)
{
	u32 i = 0; 
	for (i=0; i < (u32)CPlayer::m_nPlayerCount; i++)
	{
		_aGAContext[i].Work();
	}
	for (i=0; i < (u32)CPlayer::m_nPlayerCount; i++)
	{
		_aGVQContext[i].Work(i);
	}
	
	for (i=0; i < (u32)CPlayer::m_nPlayerCount; i++)
	{
		_aGSContext[i].Work();
	}

	CNiIterator<CEntity*> it = _AvoidanceList.Begin();
	while (it.IsValid())
	{
		if (!it.Get()->IsInWorld() ||
			it.Get()->IsMarkedForWorldRemove())
		{
			it.RemoveThenNext();
		}
		else
		{
			it.Next();
		}
	}
}


FCLASS_ALIGN_PREFIX struct CountBrainsVsEntityCBData
{
	CFVec3A Pt_WS;
	CEntity* pOfEntity;
	f32 fWithinRangeSq;
	u16 uCount;
} FCLASS_ALIGN_SUFFIX;


static BOOL _CountAttackersOf_ActiveBrainCB(CAIBrain* pBrain, void* pData)
{
	CountBrainsVsEntityCBData *pCountData = (CountBrainsVsEntityCBData*) pData;

	if (pBrain->GetCurThought() == CAIBrain::TT_COMBAT && pCountData->pOfEntity == ((CGroundCombat*)pBrain->GetCurThoughtPtr())->m_pEnemy)
	{
		pCountData->uCount++;
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}


u16 _CountAttackersOf(CEntity* pEntity)
{
	CountBrainsVsEntityCBData CountData;
	CountData.pOfEntity = pEntity;
	CountData.uCount = 0;
	aibrainman_IterateActiveBrains(_CountAttackersOf_ActiveBrainCB, &CountData);
	return CountData.uCount;
}


u16 aigroup_CountEnemiesOf(CEntity* pEntity)
{
	u16 uPlayerId = 0;
	u16 uCount = 0;
	if (aiutils_GetPlayerIndex(pEntity, &uPlayerId))
	{
		if (_aGAContext[uPlayerId].m_uLastUpdateFrame != FVid_nFrameCounter)
		{
			u16 uOldAttackerCount = _aGAContext[uPlayerId].m_uAttackerCount;
		   _aGAContext[uPlayerId].m_uAttackerCount = _CountAttackersOf(pEntity);
		   _aGAContext[uPlayerId].m_uLastUpdateFrame = FVid_nFrameCounter;

		   if (uOldAttackerCount && _aGAContext[uPlayerId].m_uAttackerCount == 0)
		   {
				CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("ai"), AI_EVENTDATA1_NO_ATTACKERS, 0, uPlayerId);
		   }
		}
		uCount = _aGAContext[uPlayerId].m_uAttackerCount;
	}
	else
	{
		uCount = _CountAttackersOf(pEntity);
	}

	return uCount;
}

static BOOL _CountAlliesof_ActiveBrainCB(CAIBrain* pBrain, void* pData)
{
	CountBrainsVsEntityCBData *pCountData = (CountBrainsVsEntityCBData*) pData;

	if ((pBrain->GetAIMover()->GetLoc().DistSq(pCountData->Pt_WS) < pCountData->fWithinRangeSq) &&
		aiutils_IsFriendly(pBrain->GetAIMover()->GetEntity(), pCountData->pOfEntity))
	{
		pCountData->uCount++;
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}


static BOOL _CountEnemiesof_ActiveBrainCB(CAIBrain* pBrain, void* pData)
{
	CountBrainsVsEntityCBData *pCountData = (CountBrainsVsEntityCBData*) pData;

	if ((pBrain->GetAIMover()->GetLoc().DistSq(pCountData->Pt_WS) < pCountData->fWithinRangeSq) &&
		!aiutils_IsFriendly(pBrain->GetAIMover()->GetEntity(), pCountData->pOfEntity))
	{
		pCountData->uCount++;
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}

u16 aigroup_CountAlliesNearPt(CEntity* pOfWho, const CFVec3A& NearPt_WS, f32 fWithinDist3D)
{
	CountBrainsVsEntityCBData CountData;

	CountData.pOfEntity = pOfWho;
	CountData.uCount = 0;
	CountData.Pt_WS = NearPt_WS;
	CountData.fWithinRangeSq = fWithinDist3D*fWithinDist3D;

	aibrainman_IterateActiveBrains(_CountAlliesof_ActiveBrainCB, &CountData);
	return CountData.uCount;
}

u16 aigroup_CountEnemiesNearPt(CEntity* pOfWho, const CFVec3A& NearPt_WS, f32 fWithinDist3D)
{
	CountBrainsVsEntityCBData CountData;

	CountData.pOfEntity = pOfWho;
	CountData.uCount = 0;
	CountData.Pt_WS = NearPt_WS;
	CountData.fWithinRangeSq = fWithinDist3D*fWithinDist3D;

	aibrainman_IterateActiveBrains(_CountEnemiesof_ActiveBrainCB, &CountData);
	return CountData.uCount;
}

//How many allies with uFriendsBitMask are currently attacking this enemy? Which rank is pWhom?
#define MAX_NUM_TO_RANK 32
FCLASS_ALIGN_PREFIX struct CRankWhoCBData
{
	CEntity* pOfEntity;
	CEntity* pRankWho;
	u64 uFriendsBitMask;
	u16 uRank;

} FCLASS_ALIGN_SUFFIX;

static	CAIBrain* _apRankThese[MAX_NUM_TO_RANK];

static BOOL _GetRankOfWho_ActiveBrainCB(CAIBrain* pBrain, void* pData)
{
	CRankWhoCBData *pCBData = (CRankWhoCBData*) pData;

	if (pBrain->GetCurThought() == CAIBrain::TT_COMBAT &&
		pCBData->pOfEntity == ((CGroundCombat*)pBrain->GetCurThoughtPtr())->m_pEnemy &&
		((CGroundCombat*)pBrain->GetCurThoughtPtr())->m_fTimeWithoutLOS < 1.0f &&
		(pBrain->GetAIMover()->GetEntity()->TypeBits() & pCBData->uFriendsBitMask))
	{
		_apRankThese[pCBData->uRank] = pBrain;
		pCBData->uRank++;
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}



u16 aigroup_GetRankAmongAllies(CEntity* pCommonEnemy, CBot* pWhom, u64 uFriendsBitMask)
{
	if (pCommonEnemy)
	{
		u32 i;
		CRankWhoCBData CBData;
		
		CBData.pOfEntity = pCommonEnemy;
		CBData.pRankWho	= pWhom;
		CBData.uFriendsBitMask = uFriendsBitMask;
		CBData.uRank = 0;
		aibrainman_IterateActiveBrains(_GetRankOfWho_ActiveBrainCB, &CBData);

		f32 fClosestSq = FMATH_MAX_FLOAT;
		CAIBrain* pClosest = NULL;
		for (i = 0; i < CBData.uRank; i++)
		{
			f32 fDistDq = pCommonEnemy->MtxToWorld()->m_vPos.DistSq(_apRankThese[i]->GetAIMover()->GetLoc());
			if (fDistDq < fClosestSq)
			{
				pClosest = _apRankThese[i];
				fClosestSq = fDistDq;
			}
		}

		if (pClosest && pClosest == pWhom->AIBrain())
		{
			return 0;
		}

		u16 uDistanceRank = 1;
		for (i = 0; i < CBData.uRank ; i++)
		{
			if (pWhom->AIBrain() ==	 _apRankThese[i])
			{
				return uDistanceRank;
			}

			if (_apRankThese[i] != pClosest)
			{
				uDistanceRank++;
			}
		}
	}

	return 0;
}


FCLASS_ALIGN_PREFIX struct CSuspicionGroupCBData
{
	CSuspicionGroupCBData(void) 
		: pOfEntity(NULL),
		  uCount(0),
		  fMaxSuspicion(0.0f),
		  fMinDistSq(0.0f),
		  pLeader(NULL)
	{
	}

    CFVec3A Pt_WS;
	CEntity* pOfEntity;
	CAIBrain* pLeader;
	f32 fMinDistSq;
	f32 fMaxSuspicion;
	u16 uCount;
} FCLASS_ALIGN_SUFFIX;


static BOOL _GetSuspicionOf_ActiveBrainCB(CAIBrain* pBrain, void* pData)
{
	CSuspicionGroupCBData *pCBData = (CSuspicionGroupCBData*) pData;

	f32 fDisguise = 0.0f;

	if (((CBot*) pBrain->GetAIMover()->GetEntity())->GetDisguisedBot(&fDisguise))
	{
		f32 fDistToOfEntitySq = pBrain->GetAIMover()->GetLoc().DistSq(pCBData->pOfEntity->MtxToWorld()->m_vPos);
		if (pCBData->uCount == 0 ||
			pCBData->fMaxSuspicion < (1.0f-fDisguise))
		{
		   pCBData->fMaxSuspicion = 1.0f -fDisguise;
		}
		if (pCBData->uCount == 0 ||
			pCBData->fMinDistSq > fDistToOfEntitySq)
		{
		   pCBData->fMinDistSq = fDistToOfEntitySq;
		   pCBData->pLeader	= pBrain;
		}
		pCBData->uCount++;
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}


static u32 _GetSuspicionLevel(CSuspicionGroupCBData* pCBData)
{
	aibrainman_IterateActiveBrains(_GetSuspicionOf_ActiveBrainCB, pCBData);
	return pCBData->uCount;
}


BOOL aigroup_GetSuspicionInfo(CEntity* pOfEntity, CAIBrain** ppGroupLeader, u32* puMemberCount, f32*pfMaxSuspicion)
{
	CSuspicionGroupCBData CBData;
	CBData.pOfEntity = pOfEntity;

	u16 uPlayerId = 0;
	u16 uCount = 0;
	if (aiutils_GetPlayerIndex(pOfEntity, &uPlayerId))
	{
		if (_aGSContext[uPlayerId].m_uLastUpdateFrame != FVid_nFrameCounter)
		{
			if (_GetSuspicionLevel(&CBData))
			{
				_aGSContext[uPlayerId].m_uLastUpdateFrame = FVid_nFrameCounter;
				_aGSContext[uPlayerId].m_pLeader = CBData.pLeader;
				_aGSContext[uPlayerId].m_uMemberCount = CBData.uCount;
				_aGSContext[uPlayerId].m_fMaxSuspicion = CBData.fMaxSuspicion;
			}
		}
		
		uCount = _aGSContext[uPlayerId].m_uMemberCount;
		if (uCount)
		{
			if (ppGroupLeader)
			{
				*ppGroupLeader = _aGSContext[uPlayerId].m_pLeader;
			}
			if (puMemberCount)
			{
				*puMemberCount = _aGSContext[uPlayerId].m_uMemberCount;
			}
			if (pfMaxSuspicion)
			{
				*pfMaxSuspicion = _aGSContext[uPlayerId].m_fMaxSuspicion;
			}
		}
	}
	else
	{
		uCount = _GetSuspicionLevel(&CBData);
		if (uCount)
		{
			if (ppGroupLeader)
			{
				*ppGroupLeader = CBData.pLeader;
			}
			if (puMemberCount)
			{
				*puMemberCount = CBData.uCount;
			}
			if (pfMaxSuspicion)
			{
				*pfMaxSuspicion = CBData.fMaxSuspicion;
			}
		}
	}
	return uCount !=0;
}


static BOOL _GetAnyHavingSeenEntity_ActiveBrainCB(CAIBrain* pBrain, void* pData)
{
	CSuspicionGroupCBData *pCBData = (CSuspicionGroupCBData*) pData;

	f32 fDisguise = 0.0f;

	CAIMemoryLarge* pSeenMem = NULL;
	if (pBrain->GetAIMover()->GetEntity() != pCBData->pOfEntity &&
		pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, pCBData->pOfEntity, &pSeenMem))
	{
		pCBData->pLeader = pBrain;
		pCBData->Pt_WS = pSeenMem->m_EntityLoc;
		return AIBRAINMAN_STOP_ITERATION;
	}
	return AIBRAINMAN_CONTINUE_ITERATION;
}


BOOL aigroup_ShareKnowledgeOfEnemyLoc(CEntity* pEnemy, CFVec3A* pNearPt_WS)
{
	CSuspicionGroupCBData Context;
	Context.pOfEntity = pEnemy;


	BOOL bFound = FALSE;
	aibrainman_IterateActiveBrains(_GetAnyHavingSeenEntity_ActiveBrainCB, &Context);
	if (Context.pLeader)
	{
		if (pNearPt_WS)
		{
			*pNearPt_WS = Context.Pt_WS;
		}

		//found one
		bFound = TRUE;
	}

	return bFound;
}






u8* aigroup_GetVisRaysAt(CEntity* pEntity)
{
	u16 uPlayerId = 0;
	u8* paVisRays = NULL;
	if (aiutils_GetPlayerIndex(pEntity, &uPlayerId) && pEntity->AIBrain())
	{
		 
		paVisRays = _aGVQContext[uPlayerId].m_LookAtMgr.m_auScannerStatus;
	}

	return paVisRays;
}


f32 aigroup_GetLightAt(CEntity* pEntity)
{
	u16 uPlayerId = 0;
	f32 fLightAt = 1.0f;
	if (aiutils_GetPlayerIndex(pEntity, &uPlayerId))
	{
		fLightAt = _aGVQContext[uPlayerId].CalcLightValueAt(uPlayerId);
	}

	return fLightAt;
}


void aigroup_RegisterAsMech(CBot* pBot)
{
	if (_bSystemInitialized && pBot && !_ActiveMechList.Find(pBot))
	{
		_ActiveMechList.PushHead(pBot);
	}
}


void aigroup_UnregisterAsMech(CBot* pBot)
{
	_ActiveMechList.Remove(pBot);
}


CBot* aigroup_FindOpenPillBox(CAIBrain* pDriverBrain, f32 fWithin)
{
	f32 fWithinRadSq = fWithin*fWithin;
	CNiIterator<CBot*> it = _ActiveMechList.Begin();
	while (it.IsValid())
	{
		CBot* pBot = it.Get();
		if (!pBot->IsDeadOrDying() &&
			pBot->IsPillBox() &&
			pBot->MtxToWorld()->m_vPos.DistSq(pDriverBrain->GetAIMover()->GetLoc()) < fWithinRadSq &&
			!((CBotSiteWeapon*) pBot)->GetDriverBot() &&
			!aiutils_IsMechSeatObstructed(pBot, 0, pDriverBrain->GetAIMover()->GetEntity()) &&
			((CBotSiteWeapon*) pBot)->BotIsEligibleToOperatePillbox((CBot*) pDriverBrain->GetAIMover()->GetEntity()) &&
			!ResLock_IsMechSeatLocked(pBot, 0))
		{
			return pBot;
		}
		it.Next();
	}
	return NULL;
}


CBot* aigroup_FindOpenRatGun(CAIBrain* pDriverBrain, f32 fWithin)
{
	f32 fWithinRadSq = fWithin*fWithin;
	CNiIterator<CBot*> it = _ActiveMechList.Begin();
	while (it.IsValid())
	{
		CBot* pBot = it.Get();
		if (!pBot->IsDeadOrDying() &&
			pBot->IsRatGun() &&
			pBot->MtxToWorld()->m_vPos.DistSq(pDriverBrain->GetAIMover()->GetLoc()) < fWithinRadSq &&
			!((CBotSiteWeapon*) pBot)->GetDriverBot() &&
			(!((CBotSiteWeapon*) pBot)->GetParent() || !(((CBotSiteWeapon*) pBot)->GetParent()->TypeBits() & ENTITY_BIT_VEHICLERAT) || aiutils_IsFriendly(pDriverBrain->GetAIMover()->GetEntity(), pBot->GetParent())) &&
			!aiutils_IsMechSeatObstructed(pBot, 0, pDriverBrain->GetAIMover()->GetEntity()) &&
			((CBotSiteWeapon*) pBot)->BotIsEligibleToOperatePillbox((CBot*) pDriverBrain->GetAIMover()->GetEntity()) &&
			!ResLock_IsMechSeatLocked(pBot, 0))
		{
			return pBot;
		}
		it.Next();
	}
	return NULL;
}

// private vehicle find, excludes occupied or locked vehicles
CBot* _aigroup_FindOpenVehicle( CAIBrain *pDriverBrain, u64 uVehicleLeafBit, f32 fRadius )
{
	CBot *pBestBot = NULL;
	f32 fBestDistSq = fRadius * fRadius;
	f32 fVehicleDistSq;

	// loop through all suitable vehicle bots, looking for one closest to pDriverBrain
	CNiIterator<CBot*> it = _ActiveMechList.Begin();
	while (it.IsValid())
	{
		CBot* pVehicle = it.Get();
		u32 uWhoGUID = 0;
		if (!pVehicle->IsDeadOrDying() &&
			( pVehicle->TypeBits() & uVehicleLeafBit ) &&
			!((CVehicle*) pVehicle)->GetDriverBot() &&
			(pVehicle->m_fSpeedXZ_WS < 5.0f || FVid_nFrameCounter < 3) &&
			!aiutils_IsMechSeatObstructed(pVehicle, 0, pDriverBrain->GetAIMover()->GetEntity()) &&
			(!ResLock_IsMechSeatLocked(pVehicle, 0, &uWhoGUID) || uWhoGUID == pDriverBrain->GetGUID()))
		{
			fVehicleDistSq = pVehicle->MtxToWorld()->m_vPos.DistSq( pDriverBrain->GetAIMover()->GetLoc() );

			// see if new bot found is within current best distance (or within radius squared if first bot found)
			if( fVehicleDistSq < fBestDistSq )
			{
				fBestDistSq = fVehicleDistSq;	// reset best distance to closer distance
				pBestBot = pVehicle;				// remember this bot
			}
		}
		it.Next();
	}

	return pBestBot;
}

// basic find, no check for anyone driving or locked.
CVehicle* aigroup_FindNearestVehicle( CAIBrain* pDriverBrain, u64 uVehicleLeafBit, f32 fRadius )
{
	CBot *pBestBot = NULL;
	f32 fBestDistSq = fRadius * fRadius;
	f32 fVehicleDistSq;

	// loop through all suitable vehicle bots, looking for one closest to vLocation
	CNiIterator<CBot*> it = _ActiveMechList.Begin();
	while (it.IsValid())
	{
		CBot* pVehicle = it.Get();
		if (pVehicle->TypeBits() & ENTITY_BIT_VEHICLE && 
			!pVehicle->IsDeadOrDying() &&
			pVehicle != (CBot*) pDriverBrain->GetAIMover()->GetEntity() &&
			( pVehicle->TypeBits() & uVehicleLeafBit ) )
		{
			fVehicleDistSq = pVehicle->MtxToWorld()->m_vPos.DistSq( pDriverBrain->GetAIMover()->GetLoc() );

			// see if new bot found is within current best distance (or within radius squared if first bot found)
			if( fVehicleDistSq < fBestDistSq )
			{
				fBestDistSq = fVehicleDistSq;	// reset best distance to closer distance
				pBestBot = pVehicle;				// remember this bot
			}
		}
		it.Next();
	}

	FASSERT(!pBestBot || pBestBot->TypeBits() & ENTITY_BIT_VEHICLE);
	return (CVehicle*)pBestBot;
}


CBot* aigroup_FindNearestSite( const CFVec3A& Pt_WS, f32 fRadius )
{
	CBot *pBestBot = NULL;
	f32 fBestDistSq = fRadius * fRadius;
	f32 fDistSq;

	// loop through all suitable vehicle bots, looking for one closest to vLocation
	CNiIterator<CBot*> it = _ActiveMechList.Begin();
	while (it.IsValid())
	{
		CBot* pSite = it.Get();
		if (pSite->TypeBits() & ENTITY_BIT_SITEWEAPON)
		{
			fDistSq = pSite->MtxToWorld()->m_vPos.DistSq( Pt_WS );

			// see if new bot found is within current best distance (or within radius squared if first bot found)
			if( fDistSq < fBestDistSq )
			{
				fBestDistSq = fDistSq;	// reset best distance to closer distance
				pBestBot = pSite;				// remember this bot
			}
		}
		it.Next();
	}

	FASSERT(!pBestBot || pBestBot->TypeBits() & ENTITY_BIT_SITEWEAPON);
	return pBestBot;
}



// returns pointer to first player-driven Vehicle found, or NULL if none.
CVehicle* aigroup_FindPlayerVehicle( u64 uVehicleLeafBit )
{
	// loop through all suitable vehicle bots
	CNiIterator<CBot*> it = _ActiveMechList.Begin();
	while (it.IsValid())
	{
		CBot* pBot = it.Get();
		if ((pBot->TypeBits() & ENTITY_BIT_VEHICLE) && ( pBot->TypeBits() & uVehicleLeafBit ) )
		{
			CVehicle *pVehicle = (CVehicle*) pBot;
			if ((pVehicle->GetDriverBot() && pVehicle->GetDriverBot()->IsPlayerBot()) ||
				(pVehicle->GetGunnerBot() && pVehicle->GetGunnerBot()->IsPlayerBot()) )
			{
				return (CVehicle*) pVehicle;
			}
		}
		it.Next();
	}

	return NULL;
}


CBot* aigroup_FindOpenSentinel(CAIBrain* pDriverBrain, f32 fWithin)
{

	if (pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGLITCH ||
		pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTMINER ||
		pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
	{
		return _aigroup_FindOpenVehicle( pDriverBrain, ENTITY_BIT_VEHICLESENTINEL, fWithin );
	}

	return NULL;
}


CBot* aigroup_FindOpenRat(CAIBrain* pDriverBrain, f32 fWithin)
{
	if (pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGLITCH ||
		pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTMINER ||
		pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
	{
		return _aigroup_FindOpenVehicle( pDriverBrain, ENTITY_BIT_VEHICLERAT, fWithin );
	}
	return NULL;
}


CBot* aigroup_FindOpenLoader(CAIBrain* pDriverBrain, f32 fWithin)
{

	if (pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGLITCH ||
		pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTMINER ||
		pDriverBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
	{
		return _aigroup_FindOpenVehicle( pDriverBrain, ENTITY_BIT_VEHICLELOADER, fWithin );
	}
	return NULL;
}


void aigroup_RegisterForAIAvoidence(CEntity* pEntity)
{
	if (_bSystemInitialized && pEntity)
	{
		_AvoidanceList.PushHead(pEntity);
	}
}


void aigroup_UnRegisterForAIAvoidence(CEntity* pEntity)
{
	_AvoidanceList.Remove(pEntity);
}

void aigroup_IterateAvoidanceEntityList(IterateEntityCBFunc* pCBFunc, void* pData)
{
	CNiIterator<CEntity*> it = _AvoidanceList.Begin();
	while (it.IsValid())
	{
		CEntity* pEntity = it.Get();
		if (pEntity->IsInWorld())
		{
			if (pCBFunc(pEntity, pData) == AIGROUP_STOP_ENTITY_ITERATION)		//If CB returns TRUE, it means ABORT!
			{
				return;
			}
		}
		it.Next();
	}

}


u32 aigroup_GetAvoidanceEntityListSize(void)
{
	return _AvoidanceList.Size();
}

