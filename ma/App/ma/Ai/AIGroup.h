//////////////////////////////////////////////////////////////////////////////////////
// AIGroup.h - 
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
// 08/14/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIGROUP_H_
#define _AIGROUP_H_ 1


#include "fmath.h"

class CAIBrain;
class CEntity;
class CGraphPoi;
class CBot;
class CBotSiteWeapon;
class CVehicle;

BOOL aigroup_InitSystem(void);
void aigroup_UninitSystem(void);
BOOL aigroup_InitLevel(void);
void aigroup_UninitLevel(void);
void aigroup_Work(void);
void aigroup_CheckpointRestore(void);

void aigroup_RegisterAttack(CAIBrain* pBrain);
void aigroup_UnregisterAttack(CAIBrain* pBrain);

u16 aigroup_CountEnemiesOf(CEntity* pEnemy);	//How many brains are currently attacking this enemy?
u16 aigroup_GetRankAmongAllies(CEntity* pCommonEnemy, CBot* pWhom, u64 uFriendsBitMask);	//How many allies with uFriendsBitMask are currently attacking this enemy? Which rank is pWhom?
u16 aigroup_CountAlliesNearPt(CEntity* pOfWho, const CFVec3A& NearPt_WS, f32 fWithinDist3D);
u16 aigroup_CountEnemiesNearPt(CEntity* pOfWho, const CFVec3A& NearPt_WS, f32 fWithinDist3D);
BOOL aigroup_ShareKnowledgeOfEnemyLoc(CEntity* pEnemy, CFVec3A* pNearPt_WS);
u8* aigroup_GetVisRaysAt(CEntity* pEntity);
f32 aigroup_GetLightAt(CEntity* pEntity);
u16 aigroup_IsPoiLocked(CGraphPoi* pPoi);
u16 aigroup_RequestPoiLock(CGraphPoi* pPoi);
u16 aigroup_FreePoiLock(CGraphPoi* pPoi);
BOOL aigroup_GetSuspicionInfo(CEntity* pOfEntity, CAIBrain** ppGroupLeader, u32* puMemberCount, f32*pfMaxSuspicion);

// Special functions to make AI aware of Non-AI-Bots that are on attack
//	  
//	  should be called by the non-when a No-AI-Bot Begins or Ends its attack
//
void aigroup_BotSiteWeaponRegisterAttack(CBotSiteWeapon* pSite);		
void aigroup_BotSiteWeaponUNegisterAttack(CBotSiteWeapon* pSite);

void aigroup_RegisterAsMech(CBot* pSite);
void aigroup_UnregisterAsMech(CBot* pSite);

CVehicle* aigroup_FindNearestVehicle( CAIBrain* pDriverBrain, u64 uVehicleLeafBit, f32 fRadius );
CVehicle* aigroup_FindPlayerVehicle( u64 uVehicleLeafBit );
CBot* aigroup_FindNearestSite( const CFVec3A& Pt_WS, f32 fRadius );

CBot* aigroup_FindOpenPillBox(CAIBrain* pBrain, f32 fWithin);
CBot* aigroup_FindOpenSentinel(CAIBrain* pBrain, f32 fWithin);
CBot* aigroup_FindOpenLoader(CAIBrain* pBrain, f32 fWithin);
CBot* aigroup_FindOpenRat(CAIBrain* pBrain, f32 fWithin);
CBot* aigroup_FindOpenRatGun(CAIBrain* pBrain, f32 fWithin);

void aigroup_RegisterForAIAvoidence(CEntity* pEntity);
void aigroup_UnRegisterForAIAvoidence(CEntity* pEntity);

typedef BOOL IterateEntityCBFunc(CEntity* pEntity, void* pData);
#define AIGROUP_STOP_ENTITY_ITERATION 1
#define AIGROUP_CONTINUE_ENTITY_ITERATION 0
void aigroup_IterateAvoidanceEntityList(IterateEntityCBFunc* pCBFunc, void* pData);
u32 aigroup_GetAvoidanceEntityListSize(void);
extern const f32 _PLAYERBRAIN_VISRAYSCAN_LOS_DIST;

#endif

