//////////////////////////////////////////////////////////////////////////////////////
// AIBrainUtils.cpp - 
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
#include "fang.h"
#include "AIBrainUtils.h"
#include "AIBrain.h"
#include "AIMover.h"
#include "../Entity.h"

void StuffWaitParams(	CAIMemoryLarge* pParamPack,
						BOOL bSpecifyLookAt,		// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						const CFVec3A& LookAtLoc,	// A position to look at	
						u32 uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uLookAroundChance)		// (0-100)		

{
	if (pParamPack)
	{
		u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PokeU8U8(pBuff, bSpecifyLookAt, uLookAroundChance);		//2
		pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.x);							//4
		pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.y);							//4
		pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.z);							//4
		pBuff = pParamPack->PokeU32(pBuff, uLookAtObjGUID);							//4
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
}


void ExtractWaitParams(	CAIMemoryLarge* pParamPack,
						BOOL *pbSpecifyLookAt,		// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						CFVec3A *pLookAtLoc,	// A position to look at (Ignored if uLookAtObjGUID > 0)	
						u32	*puLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 *puLookAroundChance)		// (0-100) Chance that obj will look around every now and again.
{
	if (pParamPack)
	{  
		u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
		u8 SpecifyLookAt;
		pBuff = pParamPack->PeekU8U8(pBuff, &SpecifyLookAt, puLookAroundChance);	//2
		pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->x));						//4
		pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->y));						//4
		pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->z));						//4
		pBuff = pParamPack->PeekU32(pBuff, puLookAtObjGUID);						//4
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
		*pbSpecifyLookAt = SpecifyLookAt;
	}
}


u8* StuffWanderParams(	CAIMemoryLarge* pParamPack,
						f32 fRadius,			// (0.0 - Inf)	Radius (in feet) to stay within while wandering
						BOOL bStayInRoom,		// (0/1)		Use radius, but always stay in the original room
						u8 uSpeedPctMin,		// (0-100)		Min Pct of this unit ability that it should travel at
						u8 uSpeedPctMax,		// (0-100)		Max             "      "
						u8 uSociable,			// (0-100)		How likely they are to stop and talk to other bots
						u8 uCuriosity,			// (0-100)		How likely they are to stop and look around at pretty things
						u8 uTakeBreakChance,	// (1-100)		a one in ?? chance that bot will take a break every now and then.
						cchar *pszPathName)	// String table Name of espline entity that contains the path points
{
	u8* pBuff = NULL;
	if (pParamPack)
	{	//stuff params into a large memory module
		pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PokeF32(pBuff, fRadius);						//4
		pBuff = pParamPack->PokeU8U8(pBuff, bStayInRoom, uTakeBreakChance);	//2
		pBuff = pParamPack->PokeU8U8(pBuff, uSpeedPctMin, uSpeedPctMax);	//2
		pBuff = pParamPack->PokeU8U8(pBuff, uSociable, uCuriosity);			//2
		pBuff = pParamPack->PokeU32(pBuff, (u32) pszPathName);				//4
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
	return pBuff;
}


void ExtractWanderParams(	CAIMemoryLarge* pParamPack,
							f32 *pfRadius,			// (0.0 - Inf)	Radius (in feet) to stay within while wandering
							BOOL *pbStayInRoom,		// (0/1)		Use radius, but always stay in the original room
							u8 *puSpeedPctMin,		// (0-100)		Min Pct of this unit ability that it should travel at
							u8 *puSpeedPctMax,		// (0-100)		Max             "      "
							u8 *puSociable,			// (0-100)		How likely they are to stop and talk to other bots
							u8 *puCuriosity,		// (0-100)		How likely they are to stop and look around at pretty things
							u8 *puTakeBreakChance,	// (1-100)		a one in ?? chance that bot will take a break every now and then.
							cchar **ppszPathName)	// String table Name of espline entity that contains the path points
{
	if (pParamPack)
	{ //Extract params into a large memory module
		u8 tmp1;
		u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PeekF32(pBuff, pfRadius);
		pBuff = pParamPack->PeekU8U8(pBuff, &tmp1, puTakeBreakChance);
		*pbStayInRoom = tmp1;
		pBuff = pParamPack->PeekU8U8(pBuff, puSpeedPctMin, puSpeedPctMax);
		pBuff = pParamPack->PeekU8U8(pBuff, puSociable, puCuriosity);		  
		pBuff = pParamPack->PeekU32(pBuff, (u32*) ppszPathName);			//4
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
}


void StuffPatrolParams(	CAIMemoryLarge* pParamPack,
						cchar *pszPathName,				// String table Name of espline entity that contains the path points
						u8 uPatrolSpeed,				// (0-100)	Pct of speed patrolling unit should travel at
						u8 uDirection,					// (0,1)	0= Clockwise, 1 = CounterClockWise
						u8 uNumIgnoreZones,				// how many elements in the next parameter
						cchar** paszIgonoreZones)		// array of names of patrol zones to ignore
{
	if (pParamPack)
	{
		u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PokeU32(pBuff, (u32) pszPathName);			//4
		pBuff = pParamPack->PokeU32(pBuff, (u32) paszIgonoreZones);		//4
		pBuff = pParamPack->PokeU8U8(pBuff, uPatrolSpeed, uDirection);	//2
		pBuff = pParamPack->PokeU8U8(pBuff, uNumIgnoreZones, 0);		//2
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
}


void ExtractPatrolParams(	CAIMemoryLarge* pParamPack,
							cchar **ppszPathName,				// String table Name of espline entity that contains the path points
							u8 *puPatrolSpeed,					// (0-100)	Pct of speed patrolling unit should travel at
							u8 *puDirection,					// (0,1)	0= Clockwise, 1 = CounterClockWise
							u8 *puNumIgnoreZones,				// how many elements in the next parameter
							cchar*** ppaszIgonoreZones)			// array of names of patrol zones to ignore
{
	if (pParamPack)
	{
		u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PeekU32(pBuff, (u32*) ppszPathName);			//4
		pBuff = pParamPack->PeekU32(pBuff, (u32*) ppaszIgonoreZones);		//4
		pBuff = pParamPack->PeekU8U8(pBuff, puPatrolSpeed, puDirection);	//2
		pBuff = pParamPack->PeekU8U8(pBuff, puNumIgnoreZones, NULL);		//2
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
}



u8* StuffGotoParams(	CAIMemoryLarge* pParamPack,
						const CFVec3A& GotoLoc,			// Where
						u8 uFudgeDest /* = 0 */,		// how much (in feet) the path finder can fudge the destination
						u8 uSpeedPct /* = 0 */,			// Pct speed this unit should travel at
						u16	uGotoFlags/* = 0 */)		// CGenericGoto::GOTOFLAG_*
						
{
	u8* pBuff = NULL;
	if (pParamPack)
	{ //Extract params from a large memory module
		pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PokeF32(pBuff, GotoLoc.x);			//4
		pBuff = pParamPack->PokeF32(pBuff, GotoLoc.y);			//4
		pBuff = pParamPack->PokeF32(pBuff, GotoLoc.z);			//4
		pBuff = pParamPack->PokeF32(pBuff, 0.0f);				//4
		pBuff = pParamPack->PokeF32(pBuff, 0.0f);				//4
		pBuff = pParamPack->PokeF32(pBuff, 0.0f);				//4
		pBuff = pParamPack->PokeU32(pBuff, 0);		//4
		pBuff = pParamPack->PokeU8U8(pBuff, 0, uFudgeDest);		//2
		pBuff = pParamPack->PokeU8U8(pBuff, uSpeedPct, 0);  //2
		pBuff = pParamPack->PokeU16(pBuff, uGotoFlags);
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
	return pBuff;
}

u8* StuffGotoWithLookAtParams(	CAIMemoryLarge* pParamPack,
						const CFVec3A& GotoLoc,			// Where
						const CFVec3A& LookAtLoc,		// A position to look at (Ignored if uLookAtObjGUID > 0)	
						u32	uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means ignore)
						u8 uPctToLookAt,			
						u8 uFudgeDest /* = 0 */,		// how much (in feet) the path finder can fudge the destination
						u8 uSpeedPct /* = 0 */,			// Pct speed this unit should travel at
						u16	uGotoFlags/* = 0 */)		// CGenericGoto::GOTOFLAG_*
						
{
	u8* pBuff = NULL;
	if (pParamPack)
	{ //Extract params from a large memory module
		pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PokeF32(pBuff, GotoLoc.x);			//4
		pBuff = pParamPack->PokeF32(pBuff, GotoLoc.y);			//4
		pBuff = pParamPack->PokeF32(pBuff, GotoLoc.z);			//4
		pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.x);		//4
		pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.y);		//4
		pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.z);		//4
		pBuff = pParamPack->PokeU32(pBuff, uLookAtObjGUID);		//4
		pBuff = pParamPack->PokeU8U8(pBuff, uPctToLookAt, uFudgeDest);		//2
		pBuff = pParamPack->PokeU8U8(pBuff, uSpeedPct, 0);  //2
		pBuff = pParamPack->PokeU16(pBuff, uGotoFlags);
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
	return pBuff;
}

u8* ExtractGotoParams(	CAIMemoryLarge* pParamPack,
						CFVec3A *pGotoLoc,			// Where
						CFVec3A *pLookAtLoc,		// A position to look at (Ignored if uLookAtObjGUID > 0)	
						u32	*puLookAtObjGUID,		// The GUID of an entity to keep looking at (0 means ignore)
						u8	*puPctToLookAt,			// 
						u8 *puFudgeDest,
						u8 *puSpeedPct,				// Pct speed this unit should travel at
						u16	*puGotoFlags)			// CGenericGoto::GOTOFLAG_*
						
{
	u8* pBuff = NULL;
	if (pParamPack)
	{ //Extract params from a large memory module
		pBuff = (u8*) &(pParamPack->m_uSmallData);
		pBuff = pParamPack->PeekF32(pBuff, &(pGotoLoc->x));			//4
		pBuff = pParamPack->PeekF32(pBuff, &(pGotoLoc->y));			//4
		pBuff = pParamPack->PeekF32(pBuff, &(pGotoLoc->z));			//4
		pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->x));		//4
		pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->y));		//4
		pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->z));		//4
		pBuff = pParamPack->PeekU32(pBuff, puLookAtObjGUID);		//4
		pBuff = pParamPack->PeekU8U8(pBuff, puPctToLookAt, puFudgeDest);		//2
		pBuff = pParamPack->PeekU8U8(pBuff, puSpeedPct, NULL);  //2
		pBuff = pParamPack->PeekU16(pBuff, puGotoFlags);
		FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
	}
	return pBuff;
}



void StuffAttackParams(CAIMemoryLarge* pParamPack,
						u32	uEnemyGUID,
						s16 nCompletionEvent)
{
	if (pParamPack)
	{ //stuff params into a large memory module
		pParamPack->m_uMediumData = uEnemyGUID;
	}
}




void ExtractAttackParams(CAIMemoryLarge* pParamPack,
						 u32 *puEnemyGUID,
						 s16 *pnCompletionEvent)
{
	FASSERT(pParamPack);
	if (puEnemyGUID)
	{
		*puEnemyGUID = pParamPack->m_uMediumData;
	}
	if (pnCompletionEvent)
	{
		*pnCompletionEvent = -1;
	}
}


void StuffFaceItParams(	CAIMemoryLarge* pParamPack,
						const CFVec3A& LookAtLoc,	// A position to look at (Ignored if uLookAtObjGUID > 0)	
						u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uTrack,					// (0-100) How hard to try to track LookAt it if moving
						u8 uTimeOut)				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
{
	FASSERT(pParamPack);
	u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
	pBuff = pParamPack->PokeU8U8(pBuff, uTrack, uTimeOut);  //2
	pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.x);		  //4
	pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.y);		  //4
	pBuff = pParamPack->PokeF32(pBuff, LookAtLoc.z);		  //4
	pBuff = pParamPack->PokeU32(pBuff, uLookAtObjGUID);		  //4
	FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
}


void ExtractFaceItParams(	CAIMemoryLarge* pParamPack,
							CFVec3A* pLookAtLoc,	
							u32* puLookAtObjGUID,		// The GUID of an entity to keep looking at (0 means ignore)
							u8* puTrack,				// (0-100) How hard to try to track LookAt it if moving
							u8* puTimeOut)				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
{
	FASSERT(pParamPack && pLookAtLoc && puLookAtObjGUID && puTrack && puTimeOut);
	u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
	pBuff = pParamPack->PeekU8U8(pBuff, puTrack, puTimeOut); //2
	pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->x));					//4
	pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->y));					//4
	pBuff = pParamPack->PeekF32(pBuff, &(pLookAtLoc->z));					//4
	pBuff = pParamPack->PeekU32(pBuff, puLookAtObjGUID);					//4
	FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
}


void StuffTalkToParams(	CAIMemoryLarge* pParamPack,
						u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uTimeOut,				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
						BOOL bStopFirst,
						void *pTalkModeFunc,
						void *pvData1,
						void *pvData2)
{
	u8 uStopFirst = (u8) bStopFirst;
	FASSERT(pParamPack);
	u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
	pBuff = pParamPack->PokeU32(pBuff, uLookAtObjGUID);			 //4
	pBuff = pParamPack->PokeU32(pBuff, (u32)pTalkModeFunc);		 //4
	pBuff = pParamPack->PokeU32(pBuff, (u32)pvData1);			 //4
	pBuff = pParamPack->PokeU32(pBuff, (u32)pvData2);			 //4
	pBuff = pParamPack->PokeU8U8(pBuff, uStopFirst, uTimeOut);	 //2
	FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
}


void ExtractTalkToParams(	CAIMemoryLarge* pParamPack,
							u32	*puLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
							u8 *puTimeOut,				// (0-255) Even if the thing is there, quit after this number of seconds (0 means never quit)
							BOOL *pbStopFirst,
							void **ppTalkModeFunc,
							void **ppvData1,
							void **ppvData2)
{
	u8 bStopFirst;
	FASSERT(pParamPack && puLookAtObjGUID && puTimeOut && pbStopFirst && ppTalkModeFunc && ppvData1 && ppvData2);
	u8 *pBuff = (u8*) &(pParamPack->m_uSmallData);
	pBuff = pParamPack->PeekU32(pBuff, puLookAtObjGUID);			//4
	pBuff = pParamPack->PeekU32(pBuff, (u32*)ppTalkModeFunc);		//4
	pBuff = pParamPack->PeekU32(pBuff, (u32*)ppvData1);				//4
	pBuff = pParamPack->PeekU32(pBuff, (u32*)ppvData2);				//4
	pBuff = pParamPack->PeekU8U8(pBuff, &bStopFirst, puTimeOut);	//2
	*pbStopFirst = bStopFirst;
	FASSERT(pBuff < (u8*) pParamPack + sizeof(CAIMemoryLarge));
}



void StuffAttackSpecs(	CAIMemoryLarge* pAttackSpecs,
						u8 uNormalRuleSet,				// which ruleset the brain will use when attacking
						u8 uAlternateRuleSet,
						u8 uAlternateRuleSetOdds,		//(0-100) pct chance that the alternate will be used
						u8 uAttackSpeedMin,				//(0-100)
						u8 uAttackSpeedMax,				//(0-100)
						u16 uMaxDistFromOrigin,
						u16 uStayPutDist,
						u8 uMeleeRad
						)
{
	FASSERT(pAttackSpecs);
	u8 *pBuff = (u8*) &(pAttackSpecs->m_uSmallData);
	pBuff = pAttackSpecs->PokeU8U8(pBuff, uNormalRuleSet, uAlternateRuleSet);		//2
	pBuff = pAttackSpecs->PokeU8U8(pBuff, uAlternateRuleSetOdds, uMeleeRad);		//2
	pBuff = pAttackSpecs->PokeU8U8(pBuff, uAttackSpeedMin, uAttackSpeedMax);		//2
	pBuff = pAttackSpecs->PokeU16(pBuff, uMaxDistFromOrigin);						//2
	pBuff = pAttackSpecs->PokeU16(pBuff, uStayPutDist);							//2
	FASSERT(pBuff < (u8*) pAttackSpecs + sizeof(CAIMemoryLarge));
}


void ExtractAttackSpecs(	CAIMemoryLarge* pAttackSpecs,
							u8 *puNormalRuleSet,	 // which ruleset the brain will use when attacking
							u8 *puAlternateRuleSet,
							u8 *puAlternateRuleSetOdds, //(0-100) pct chance that the alternate will be used
							u8 *puAttackSpeedMin,  //(0-100)
							u8 *puAttackSpeedMax,	//(0-100)
							u16 *puMaxDistFromOrigin,
							u16 *puStayPutDist,
							u8 *puMeleeRad
							)
{
	FASSERT(pAttackSpecs && puNormalRuleSet && puAlternateRuleSet && puAlternateRuleSetOdds && puAttackSpeedMin && puAttackSpeedMax && puMaxDistFromOrigin && puStayPutDist);
	u8 *pBuff = (u8*) &(pAttackSpecs->m_uSmallData);
	pBuff = pAttackSpecs->PeekU8U8(pBuff, puNormalRuleSet, puAlternateRuleSet);		//2
	pBuff = pAttackSpecs->PeekU8U8(pBuff, puAlternateRuleSetOdds, puMeleeRad);		//2
	pBuff = pAttackSpecs->PeekU8U8(pBuff, puAttackSpeedMin, puAttackSpeedMax);		//2
	pBuff = pAttackSpecs->PeekU16(pBuff, puMaxDistFromOrigin);						//2
	pBuff = pAttackSpecs->PeekU16(pBuff, puStayPutDist);								//2
	FASSERT(pBuff < (u8*) pAttackSpecs + sizeof(CAIMemoryLarge));
}


u8 ExtractGreetingLevel(CAIMemorySmall* pGreetMemory, u32 nPlayerId)
{
	return ((pGreetMemory->m_uSmallData & (0xf << (nPlayerId*4))) >> nPlayerId*4);
}


u8 StuffIncGreetingLevel(CAIMemorySmall* pGreetMemory, u32 nPlayerId)
{
	u8 uCount = ExtractGreetingLevel(pGreetMemory, nPlayerId);
	if (uCount < 0xf)
	{
		pGreetMemory->m_uSmallData += (1 << (nPlayerId*4));
		uCount++;
	}
	return uCount;
}


void StuffGreetingLevel(CAIMemorySmall* pGreetMemory, u32 nPlayerId, u8 nCount)
{
	pGreetMemory->m_uSmallData &= ~(0xf << (nPlayerId*4));
	if (nCount < 0xf)
	{
		pGreetMemory->m_uSmallData |= (nCount << (nPlayerId*4));
	}
}


void StuffSphereMarker(CAIMemoryMedium* pStimMem, const CFVec3A& BotLoc, const CFVec3A& StimLoc, u8 uStimType, u8 uInvalidateDist)
{
	FASSERT(pStimMem);
	u8 *pBuff = (u8*) &(pStimMem->m_uSmallData);
	pBuff = pStimMem->PokeS16(pBuff, (s16) BotLoc.x );				//2
	pBuff = pStimMem->PokeS16(pBuff, (s16) BotLoc.z );				//2
	pBuff = pStimMem->PokeS16(pBuff, (s16) StimLoc.x );				//2
	pBuff = pStimMem->PokeS16(pBuff, (s16) StimLoc.z );				//2
	pBuff = pStimMem->PokeU8U8(pBuff, uStimType, uInvalidateDist);	//2
	FASSERT(pBuff <= (u8*) pStimMem + sizeof(CAIMemoryMedium));

}


void ExtractSphereMarker(CAIMemoryMedium* pStimMem, CFVec3A* pBotLocXZ, CFVec3A* pStimLocXZ, u8* puStimType, u8 *puInvalidateDist)
{
	FASSERT(pStimMem && pBotLocXZ && pStimLocXZ && puStimType && puInvalidateDist);
	pBotLocXZ->y = 0.0f;
	pStimLocXZ->y = 0.0f;
	u8 *pBuff = (u8*) &(pStimMem->m_uSmallData);
	s16 tmp;
	pBuff = pStimMem->PeekS16(pBuff, &tmp);		pBotLocXZ->x = (f32) tmp;	//2
	pBuff = pStimMem->PeekS16(pBuff, &tmp);		pBotLocXZ->z = (f32) tmp;	//2
	pBuff = pStimMem->PeekS16(pBuff, &tmp);		pStimLocXZ->x = (f32) tmp;	//2
	pBuff = pStimMem->PeekS16(pBuff, &tmp);		pStimLocXZ->z = (f32) tmp;	//2
	pBuff = pStimMem->PeekU8U8(pBuff, puStimType, puInvalidateDist);	//2
	FASSERT(pBuff <= (u8*) pStimMem + sizeof(CAIMemoryMedium));
}


BOOL DoesSeenMemMatchSphereMarker(CAIMemoryMedium* pReactBlockMem, CAIMemoryLarge* pSeenMem)
{
	CFVec3A BlockLoc;
	CFVec3A BlockFromLoc;
	CFVec3A SeenAtLoc(pSeenMem->m_EntityLoc);
	CFVec3A SeenFromLoc(pSeenMem->m_Loc);
	u8 uBlockStimType;
	u8 uBlockDist;
	ExtractSphereMarker(pReactBlockMem, &BlockFromLoc, &BlockLoc, &uBlockStimType, &uBlockDist);

	return ((SeenAtLoc.DistSqXZ(BlockLoc) < (f32) (uBlockDist*uBlockDist)) &&	   //neither end has moved more than blockDist
			(SeenFromLoc.DistSqXZ(BlockFromLoc) < (f32) (uBlockDist*uBlockDist)));
}
