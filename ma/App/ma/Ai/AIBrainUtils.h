//////////////////////////////////////////////////////////////////////////////////////
// AIBrainUtils.h - 
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
// 07/03/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIBRAINUTILS_H_
#define _AIBRAINUTILS_H_ 1

#include "fmath.h"
class CAIBrain;
class CEntity;
class CAIMemorySmall;
class CAIMemoryMedium;
class CAIMemoryLarge;

///////////////////////////////////////////////////////////////////////////////////
//
//  More Helper functions (only needed when not using the Assign_ functions above)
//

void StuffWaitParams(	CAIMemoryLarge* pParamPack,
						BOOL bSpecifyLookAt,		// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						const CFVec3A& LookAtLoc,	// A position to look at (Ignored if LookAtObjGUID > 0)	
						u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uLookAroundChance);		// (0-100) Chance that obj will look around every now and again.

u8* StuffWanderParams(	CAIMemoryLarge* pParamPack,
						f32 fRadius,				// (0.0 - Inf)	Radius (in feet) to stay within while wandering
						BOOL bStayInRoom,			// (0/1)		Use radius, but always stay in the original room
						u8 uSpeedPctMin,			// (0-100)		Min Pct of this unit ability that it should travel at
						u8 uSpeedPctMax,			// (0-100)		Max             "      "
						u8 uSociable,				// (0-100)		How likely they are to stop and talk to other bots
						u8 uCuriosity,				// (0-100)		How likely they are to stop and look around at pretty things
						u8 uTakeBreakChance, 		// (1-100)		a one in ?? chance that bot will take a break every now and then.
						cchar *pszPathName);		// String table Name of espline entity that contains the path points


//
// Patrol
//	- Unit moves along a path, potentially doing something special in a custom behavior ZONE
void StuffPatrolParams(	CAIMemoryLarge* pParamPack,
						cchar *pszPathName,				// String table Name of espline entity that contains the path points
						u8 uPatrolSpeed,				// (0-100)	Pct of speed patrolling unit should travel at
						u8 uDirection,					// (0,1)	0= Clockwise, 1 = CounterClockWise
						u8 uNumIgnoreZones,				// how many elements in the next parameter
						cchar** paszIgonoreZones);		// array of names of patrol zones to ignore

u8* StuffGotoWithLookAtParams(	CAIMemoryLarge* pParamPack,
						const CFVec3A& GotoLoc,			// Where
						const CFVec3A& LookAtLoc,		// A pt in world space to aim torso and head at.(only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
						u32	uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means none).
						u8 uPctToLookAt,				// (0-100) Percent to Goal that look at order will turn on  
						u8 uFudgeDest = 0,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
						u8 uSpeedPct = 0,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
						u16	uGotoFlags = 0);			// GOTOFLAG_*

u8* StuffGotoParams(	CAIMemoryLarge* pParamPack,
						const CFVec3A& GotoLoc,			// Where
						u8 uFudgeDest = 0,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
						u8 uSpeedPct = 0,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
						u16	uGotoFlags = 0);			// GOTOFLAG_*

u8* ExtractGotoParams(	CAIMemoryLarge* pParamPack,
						CFVec3A* pGotoLoc,		   	// Where
						CFVec3A *pLookAtLoc,	   	// A pt in world space to aim torso and head at.(only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
						u32	*puLookAtObjGUID,	   	// The GUID of an entity to keep looking at (0 means none).
						u8	*puPctToLookAt,		   	// (0-100) Percent to Goal that look at order will turn on  
						u8 *puFudgeDest,		   	// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
						u8 *puSpeedPct,			   	// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
						u16	*puGotoFlags);		   	// GOTOFLAG_*
						

u8* StuffAvoidParams(	CAIMemoryLarge* pParamPack,
						const CFVec3A& GotoLoc,		// Where
						u8 uSpeedPct,				// Pct speed this unit should travel at
						BOOL bSpecifyLookAt,		// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						const CFVec3A& LookAtLoc,	// A position to look at (Ignored if LookAtObjGUID > 0)	
						u32	LookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uStartingLookAtImp,		// (0-100) importance that the bot look towards the target at the beginning of the goto
						u8 uEndingLookAtImp,		// (0-100) importance that the bot look towards the target at the end of the goto
						u32 uAvoidFlags,
						s16 nCompletionEvent);		// Some scripting event to call apon completion or failure

// 
// Attack
//
void StuffAttackParams(	CAIMemoryLarge* pParamPack,
						u32	uEnemyGUID,
						s16 nCompletionEvent);

void StuffAttackSpecs(	CAIMemoryLarge* pAttackSpecs,
						u8 uNormalRuleSet,	 // which ruleset the brain will use when attacking
						u8 uAlternateRuleSet,
						u8 uAlternateRuleSetOdds, //(0-100) pct chance that the alternate will be used
						u8 uAttackSpeedMin,  //(0-100)
						u8 uAttackSpeedMax,	//(0-100)
						u16 uMaxDistFromOrigin,
						u16 uStayPutDist,
						u8 uMeleeRad
						);


//
// Face It
//
void StuffFaceItParams(	CAIMemoryLarge* pParamPack,
						const CFVec3A& LookAtLoc,	// A position to look at (Ignored if LookAtObjGUID > 0)	
						u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uTrack,					// (0-100) How hard to try to track LookAt it if moving
						u8 uTimeOut);				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)

//
// Talk To
//
void StuffTalkToParams(	CAIMemoryLarge* pParamPack,
						u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uTimeOut,				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
						BOOL bStopFirst,
						void *pTalkModeFunc,
						void *pvData1,
						void *pvData2);

////////////////////////////////////////////////////////////////////

void ExtractWaitParams(	CAIMemoryLarge* pParamPack,
						BOOL  *pbSpecifyLookAt,		// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						CFVec3A *pLookAtLoc,		// A position to look at (Ignored if LookAtObjGUID > 0)	
						u32	*puLookAtObjGUID,		// The GUID of an entity to keep looking at (0 means ignore)
						u8 *puLookAroundChance);	// (0-100) Chance that obj will look around every now and again.

void ExtractWanderParams(	CAIMemoryLarge* pParamPack,
							f32 *pfRadius,		
							BOOL *pbStayInRoom,	
							u8 *puSpeedPctMin,	
							u8 *puSpeedPctMax,	
							u8 *puSociable,	
							u8 *puCuriosity,		
							u8 *puTakeBreakChance,
							cchar **ppszPathName);	// String table Name of espline entity that contains the path points

void ExtractPatrolParams(	CAIMemoryLarge* ppParamPack,
							cchar **ppszPathName,			// String table Name of espline entity that contains the path points
							u8 *puPatrolSpeed,				// (0-100)	Pct of speed patrolling unit should travel at
							u8 *puDirection,				// (0,1)	0= Clockwise, 1 = CounterClockWise
							u8 *puNumIgnoreZones,			// how many elements in the next parameter
							cchar*** ppaszIgonoreZones);	// array of names of patrol zones to ignore

u8* ExtractAvoidParams(	CAIMemoryLarge* pParamPack,
						CFVec3A *pGotoLoc,				// Where
						u8 *puSpeedPct,					// Pct speed this unit should travel at
						BOOL *pbSpecifyLookAt,			// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						CFVec3A *pLookAtLoc,			// A position to look at (Ignored if LookAtObjGUID > 0)	
						u32	*pLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8	*puStartingLookAtImp,		// (0-100) importance that the bot look towards the target at the beginning of the goto
						u8	*puEndingLookAtImp,			// (0-100) importance that the bot look towards the target at the end of the goto
						u32 *puAvoidFlags,
						s16 *pnCompletionEvent);		// Some scripting event to call apon completion or failure

void ExtractAttackParams(	CAIMemoryLarge* pParamPack,
							u32 *puEnemyGUID,
							s16 *pnCompletionEvent);

void ExtractFaceItParams(	CAIMemoryLarge* pParamPack,
							CFVec3A* pLookAtLoc,	
							u32* puLookAtObjGUID,		// The GUID of an entity to keep looking at (0 means ignore)
							u8* puTrack,				// (0-100) How hard to try to track LookAt it if moving
							u8* puTimeOut);				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)

void ExtractTalkToParams(	CAIMemoryLarge* pParamPack,
							u32	*puLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
							u8 *puTimeOut,				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
							BOOL *pbStopFirst,
							void **ppTalkModeFunc,
							void **ppvData1,
							void **ppvData2);

void ExtractAttackSpecs(	CAIMemoryLarge* pAttackSpecs,
							u8 *puNormalRuleSet,	 // which ruleset the brain will use when attacking
							u8 *puAlternateRuleSet,
							u8 *puAlternateRuleSetOdds, //(0-100) pct chance that the alternate will be used
							u8 *puAttackSpeedMin,  //(0-100)
							u8 *puAttackSpeedMax,	//(0-100)
							u16 *puMaxDistFromOrigin,
							u16 *puStayPutDist,
							u8 *puMeleeRad
							);

u8 ExtractGreetingLevel(CAIMemorySmall* pGreetMemory, u32 nPlayerId);
u8 StuffIncGreetingLevel(CAIMemorySmall* pGreetMemory, u32 nPlayerId);
void StuffGreetingLevel(CAIMemorySmall* pGreetMemory, u32 nPlayerId, u8 nCount);

void StuffSphereMarker(CAIMemoryMedium* pReactBlockMem, const CFVec3A& BotLocXZ, const CFVec3A& StimLocXZ, u8 uStimType, u8 uInvalidateDist);
void ExtractSphereMarker(CAIMemoryMedium* pReactBlockMem, CFVec3A* pBotLocXZ, CFVec3A* pStimLocXZ, u8* puStimType, u8 *puInvalidateDist);
BOOL DoesSeenMemMatchSphereMarker(CAIMemoryMedium* pReactBlockMem, CAIMemoryLarge* pSeenMem);


#endif

