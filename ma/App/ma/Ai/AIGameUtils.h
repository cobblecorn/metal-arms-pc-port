//////////////////////////////////////////////////////////////////////////////////////
// AIGameUtils.h - 
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
// 05/02/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIGAMEUTILS_H_
#define _AIGAMEUTILS_H_ 1

#include "fmath.h"

class CAIGraph;
class CEntity;
class CBot;
class CAIBrain;
class CFColorRGBA;


BOOL aiutils_InitSystem(void);
void aiutils_UninitSystem(void);

CEntity* aiutils_GetPlayerEntity(s32 nPlayerIndex);
BOOL aiutils_GetPlayerLoc(s32 PlayerIndex, CFVec3A* pReturnLoc);
s32 aiutils_GetNumPlayers(void);
#if FANG_WINGC
BOOL aiutils_CanScriptRecruitWastelandZombie(CBot* pBot, CBot* pRecruiter, const CFVec3A* pEpicenter);
void aiutils_EnsureFreedWastelandZombieFollowsPlayer(CEntity* pEntity);
#endif
BOOL aiutils_IsFriendly(CEntity* pEntity, CEntity* pOtherEntity, f32* pfSuspiciousOfFriendship = NULL);  //Is this entity my friend, and if he is wearing a disguise, how suspicious should I be of it?
BOOL aiutils_IsPlayer(CEntity* pEntity, u16* pnIndex = NULL);
BOOL aiutils_GetPlayerIndex(CEntity* pEntity, u16* pnIndex =NULL);
BOOL aiutils_IsDeadOrDying(CEntity* pEntity);
BOOL aiutils_HasShield(CEntity* pEntity);
BOOL aiutils_IsShieldActive(CEntity* pEntity);
u8 aiutils_GetNumWeapons(CEntity* pEntity);
u8 aiutils_GetNumMeleeAttacks(CEntity* pEntity);
BOOL aiutils_RandomGraphLocMaxRange(s16 nWithinRoom, const CFVec3A& Origin, f32 fWithinRad, CFVec3A * pReturnLoc);
BOOL aiutils_RandomGraphLocMinMaxRange(s16 nWithinRoom, const CFVec3A& Origin, f32 fWithinRad, CFVec3A * pReturnLoc);
BOOL aiutils_FloorHeightAt(const CFVec3A& Pt_WS, f32 *pfHeight, f32 fMaxAbsVertOffset);
BOOL aiutils_FloorElevationAt(const CFVec3A& Pt_WS,	f32 *pfElevation, f32 fDropDist = 10.0f, f32 fMaxPotentialYBump = 5.0f);
const CFVec3A & aiutils_GetEntityLoc(CEntity* pEntity);
f32 aiutils_GetEntityRadXZ(CEntity* pEntity);
f32 aiutils_GetEntityDist(CEntity* pEntityA, CEntity* pEntityB);
f32 aiutils_GetEntityDist2(CEntity* pEntityA, CEntity* pEntityB);
f32 aiutils_GetEntityDistXZ(CEntity* pEntityA, CEntity* pEntityB);
f32 aiutils_GetEntityDistXZ2(CEntity* pEntityA, CEntity* pEntityB);
f32 aiutils_GetMaxBotSpeedFlatGround(CBot* pBot);
f32 aiutils_FTotalLoopSecs(void);
f32 aiutils_GetCurTimeSecs(void);
u16 aiutils_GetLastTimeSecsU16(void);
u16 aiutils_GetCurTimeSecsU16(void);
f32 aiutils_AITickToSecs(u16 uAITick);
u16 aiutils_SecsToAITick(f32 fSecs);
u16 aiutils_GetCurAITick(void);
BOOL aiutils_IsLOSObstructed_IgnoreBots(const CFVec3A &rRayStart, const CFVec3A &rRayEnd, CEntity* pSrcEntityHint);	//Hint can be NULL
#if !FANG_PRODUCTION_BUILD
	extern s32 _nNumRaysDebugTracking;
	extern CFVec3A _aRayOriginDebugTracking[];
	extern CFVec3A _aRayDestDebugTracking[];
	extern u32 _aRayDestDebugData[];

	void _aiutils_DebugTrackRay(const CFVec3A& origin, const CFVec3A& dest, s32 nColor );
	#define aiutils_DebugTrackRay( origin, dest, nColor ) _aiutils_DebugTrackRay( origin, dest, nColor )
	void _aiutils_DebugTrackRay(const CFVec3& origin, const CFVec3& dest, s32 nColor);
	#define aiutils_DebugTrackRay( origin, dest, nColor) _aiutils_DebugTrackRay( origin, dest, nColor )
#else
	#define aiutils_DebugTrackRay( origin, dest, nColor )
	#define aiutils_DebugTrackRay( origin, dest, nColor)
#endif

BOOL aiutils_GetCamPos(CFVec3A* pCamPos);
BOOL aiutils_GetCamLookAt(CFVec3A* pCamLookAt);
BOOL aiutils_CanEntityFly(CEntity* pEntity);
f32 aiutils_GetBotAimPitch(CBot* pBot);
BOOL aiutils_IsCBot(CEntity* pEntity);
BOOL aiutils_IsCBotBrain(CAIBrain* pBrain);
u16 aiutils_GetTimeOutRandMinMax(u8 uMinTime, u8 uMaxTime);
f32 aiutils_GetTimeOutRandMinMax(f32 fMinTime, f32 fMaxTime);
BOOL aiutils_IsArmed(CEntity* pEntity);	  //NOTE! (specialrules apply for unarmed player. See _ISArmed() function.))
BOOL aiutils_BrainLocToScreenPos(CAIBrain* pBrain, CFVec3A *pScreenPos);
BOOL aiutils_WSToScreenPos(const CFVec3A & Pt_WS, CFVec3A *pScreenPos);
BOOL aiutils_IsInBoundsOfWorld(CEntity* pEntity);
BOOL aiutils_IsInBoundsOfWorld(const CFVec3A& Pt_WS);
s32 aiutils_FindClosestTo(const CFVec3A& to, const CFVec3A	*paPts, s32 nNumPts);
s32 aiutils_FindRandomBitIndex(u8 uVal);

CBot* aiutils_GetMechOperator(CBot* pMech);
BOOL aiutils_IsMech(CEntity* pEntity);
BOOL aiutils_IsMechSeatObstructed(CBot* pMech, u16 uSeatId, CEntity* pExceptBy = NULL);
BOOL aiutils_GetCurMech(CEntity* pEntity, CEntity** ppMech = NULL);

// Used to figure nice parabolas with variable airtime, and arc depending on velocity
f32	 aiutils_ComputeTrajectoryFromXZVel(CFVec3A& rVelocityVec, const CFVec3A& rStartPos,const CFVec3A& rEndPos,f32 fXZVelocity, f32 fGravity=32.0f);

// Used to figure nice parabolas with constant airtime
f32	 aiutils_ComputeTrajectoryFromAirTime(CFVec3A& rVelocityVec, const CFVec3A& rStartPos,const CFVec3A& rEndPos,f32 fAirTime, f32 fGravity=32.0f);

extern u32	*aiutils_anGraphVertSortBuff;
extern u32	*aiutils_anGraphVertEnroute;

enum
{
	AICOLOR_OFF	= 	0,
	AICOLOR_RED,
	AICOLOR_GREEN,
	AICOLOR_BLUE,
	AICOLOR_YELLOW,
	AICOLOR_WHITE,
	AICOLOR_MAGENTA,
	AICOLOR_CYAN,
	NUM_AICOLORS
};
extern CFColorRGBA* aiutils_paDebugColors;


BOOL aiutils_DoesSphereIntersectAnyBotsBesidesSkipList( const CFSphere &Sphere_WS, u32 uSkipListCount = 0);


#endif


