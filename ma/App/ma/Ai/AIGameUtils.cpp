#include "fang.h"
#include "floop.h"
#include "fviewport.h"
#include "fxfm.h"
#include "fworld_coll.h"
#include "AIGameUtils.h"
#include "AIGraph.h"
#include "AiMain.h"
#include "AIRooms.h"
#include "AIBrain.h"
#include "AIMover.h"
#include "Apenew.h"
#include "AIPath.h"
#include "AINodepools.h"
#include "AIFsm.h"
#include "AiApi.h"
#include "AIBrainMems.h"
#include "ftl.h"
#include "../bot.h"
#include "../Botglitch.h"
#include "../Bottitan.h"  //shield
#include "../eshield.h"
#include "../MeshEntity.h"
#include "../meshtypes.h"
#include "../MultiplayerMgr.h"
#include "../player.h"
#include "../level.h"
#include "../site_botweapon.h"
#include "../vehiclerat.h"

static BOOL _Multiplayer_IsFriendly(CBot* pSelfBot, CBot* pTestBot);

#define FRIEND			0xff	 //friends no matter what, even damage by one to the other is considered accidental
#define ENEMY			0x00	 //enemies no matter what, on sight, they will fight if both are armed.
#define FRIEND_UNTIL	0x0f	 //friends until provoked (usually by damage from)


u8 _aauRaceRelationsGrid[NUM_AIRACES][NUM_AIRACES] =
{
//						BOTRACE_MIL		BOTRACE_DROID	BOTRACE_ZOMBIE	BOTRACE_AMBIENT
/*BOTRACE_MIL*/				FRIEND,			ENEMY,			ENEMY,			FRIEND_UNTIL,		
/*BOTRACE_DROID	*/			ENEMY,			FRIEND,			ENEMY,			FRIEND_UNTIL,		
/*BOTRACE_ZOMBIE*/			ENEMY,			ENEMY,			FRIEND,			FRIEND_UNTIL,		
/*BOTRACE_AMBIENT*/			FRIEND_UNTIL,	FRIEND_UNTIL,	FRIEND_UNTIL,	FRIEND_UNTIL,
};

u32 *aiutils_anGraphVertSortBuff = NULL;
u32 *aiutils_anGraphVertEnroute = NULL;

CFColorRGBA aiutils_aDebugColors[NUM_AICOLORS];
CFColorRGBA *aiutils_paDebugColors = NULL;

BOOL aiutils_InitSystem(void)
{
	aiutils_anGraphVertEnroute = (u32*) fres_AlignedAllocAndZero(4*aimain_pAIGraph->GetNumVerts(), 16);
	if (!aiutils_anGraphVertEnroute)
	{
		goto _ExitAIMain_InitSysWithError;
	}

	aiutils_anGraphVertSortBuff = (u32*) fres_AlignedAllocAndZero(4*aimain_pAIGraph->GetNumVerts(), 16);
	if (!aiutils_anGraphVertSortBuff)
	{
		goto _ExitAIMain_InitSysWithError;
	}

	CFSM::InitSystem(aimain_pNodePool);

	aiutils_aDebugColors[AICOLOR_OFF] = FColor_MotifWhite;
	aiutils_aDebugColors[AICOLOR_RED] = FColor_MotifRed;
	aiutils_aDebugColors[AICOLOR_GREEN] =  FColor_MotifGreen;
	aiutils_aDebugColors[AICOLOR_BLUE] =  FColor_MotifBlue;
	aiutils_aDebugColors[AICOLOR_YELLOW].SetColor(1.0f, 1.0f, 0.0f);
	aiutils_aDebugColors[AICOLOR_WHITE] = FColor_MotifWhite;
	aiutils_aDebugColors[AICOLOR_MAGENTA].SetColor(1.0f, 0.0f, 1.0f);
	aiutils_aDebugColors[AICOLOR_CYAN].SetColor(0.0f, 1.0f, 1.0f);

	aiutils_paDebugColors = aiutils_aDebugColors;
	return TRUE;

_ExitAIMain_InitSysWithError:
	return FALSE;

}


void aiutils_UninitSystem(void)
{
	CFSM::UninitSystem();

	if (aiutils_anGraphVertEnroute)
	{
		//	fres_Free(aiutils_anGraphVertEnroute);
		aiutils_anGraphVertEnroute = NULL;
	}

	if (aiutils_anGraphVertEnroute)
	{
		//	fres_Free(aiutils_anGraphVertSortBuff);
		aiutils_anGraphVertSortBuff = NULL;
	}
}



BOOL aiutils_GetPlayerLoc(s32 PlayerIndex, CFVec3A* pReturnLoc)
{
	CEntity* pPlayerEntity = Player_aPlayer[PlayerIndex].m_pEntityCurrent;  //findfix: need a way to attach a dummy brain to the player robot for obstacle avoidance	
	FASSERT(pPlayerEntity->TypeBits() & ENTITY_BIT_BOT);
	if (pReturnLoc)
	{
		*pReturnLoc = pPlayerEntity->MtxToWorld()->m_vPos;
	}
	return TRUE;
}


BOOL aiutils_RandomGraphLocMaxRange(s16 nWithinRoom, const CFVec3A& Origin, f32 fWithinRad, CFVec3A * pWanderLoc)
{
	CAIGraph* pGraph = aimain_pAIGraph;
	BOOL bClampToRoom = FALSE;
	if (nWithinRoom >=0)
	{
		bClampToRoom = TRUE;
	}

	//count verts options first
	s32 nOptions = 0;
	for (u16 i = 1; i < pGraph->GetNumVerts(); i++)
	{
		GraphVert* pV = pGraph->GetVert(i);
		if (pV == NULL || pV->m_nVertSlotStatus == VERT_SLOT_FREE)// || pV->m_nProps & VERTPROP_JUMP_VERT)
		{
			continue;
		}
		if (bClampToRoom &&	airooms_IsVertInRoom(pGraph->GetVertId(pV)) != (u16) nWithinRoom)
		{
			continue; //only look for open verts in this room
		}
		
		if (pV->GetLocation().DistSq(Origin) > fWithinRad*fWithinRad)
		{
			continue;  //only find verts who's center is withing wander Radius
		}
		aiutils_anGraphVertSortBuff[nOptions] = i;
		nOptions++;
	}

	if (nOptions > 0)
	{
		u16 nPick = (u16) aiutils_anGraphVertSortBuff[fmath_RandomInt16() % nOptions];
		GraphVert* pV = pGraph->GetVert(nPick);

		CFVec3A Loc;
		Loc.x = fmath_RandomBipolarUnitFloat()*pV->GetRad();
		Loc.y = 0.0f;
		Loc.z = fmath_RandomBipolarUnitFloat()*pV->GetRad();
		Loc.Add(pV->GetLocation());
		if (Loc.DistSqXZ(Origin) > fWithinRad*fWithinRad)
		{
			CFVec3A tmp;
			tmp.Sub(Origin, pV->GetLocation());
			tmp.y = 0.0f;
			tmp.Unitize();
			tmp.Mul(fmath_RandomFloatRange(0.001f, pV->GetRad()));
			Loc.Add(tmp, pV->GetLocation());
		}
		f32 fFloor = 0.0f;
		if (aiutils_FloorHeightAt(Loc, &fFloor, Loc.Dist(Origin)))
		{
			if (pWanderLoc)
			{
				*pWanderLoc = Loc;
				pWanderLoc->y = fFloor;
			}
		}
		return TRUE;
	}

	return FALSE;
}


BOOL aiutils_RandomGraphLocMinMaxRange(s16 nWithinRoom, const CFVec3A& Origin, f32 fWithinRad, CFVec3A * pReturnLoc)
{
	CAIGraph* pGraph = aimain_pAIGraph;
	return FALSE;
}


//first shoots ray down to look for floor, then up.
BOOL aiutils_FloorHeightAt(const CFVec3A& Pt_WS, f32 *pfHeight, f32 fMaxAbsVertOffset)
{
	BOOL bFoundFloor = FALSE;

	CFCollInfo rayColl;
	rayColl.nCollTestType =	FMESH_COLLTESTTYPE_RAY;
	rayColl.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	rayColl.bFindClosestImpactOnly = FALSE;
	rayColl.nCollMask = FCOLL_MASK_CHECK_ALL;
	rayColl.nResultsLOD = FCOLL_LOD_HIGHEST;
	rayColl.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	rayColl.bCullBacksideCollisions = TRUE;
	rayColl.Ray.Init( Pt_WS.x, Pt_WS.y+fMaxAbsVertOffset, Pt_WS.z, Pt_WS.x, Pt_WS.y-fMaxAbsVertOffset, Pt_WS.z );

	if( fworld_CollideWithWorldTris(&rayColl) && FColl_nImpactCount > 0 )
	{
		fcoll_Sort( FALSE );	//pgm: Hi Steve, Do I need this?

		FCollImpact_t *pImpact = FColl_apSortedImpactBuf[0];
		if (pImpact)
		{
			*pfHeight = pImpact->ImpactPoint.y;
			bFoundFloor = TRUE;
		}
	}

	aiutils_DebugTrackRay(rayColl.Ray.vStart_WS, rayColl.Ray.vEnd_WS, bFoundFloor);

	return bFoundFloor;  //no floor found
}

//first shoots ray down to look for floor, then up.
BOOL aiutils_FloorElevationAt(const CFVec3A& Pt_WS,
								f32 *pfElevation,
								f32 fDropDist /*= 10.0f*/,
								f32 fMaxPotentialYBump /*= 5.0f*/)
{
	BOOL bFoundFloor = FALSE;

	f32 fYBumpSoFar = 0.0f;
	f32 fYBumpDelta =  fMaxPotentialYBump*0.5f;
	u32 uRayTest = 0;
	CFVec3A Head = Pt_WS;
	CFVec3A Tail = Pt_WS;
	Tail.y += fDropDist;

	while (!bFoundFloor && fYBumpSoFar <= fMaxPotentialYBump)
	{
		CFCollInfo rayColl;
		rayColl.nCollTestType =	FMESH_COLLTESTTYPE_RAY;
		rayColl.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
		rayColl.bFindClosestImpactOnly = FALSE;
		rayColl.nCollMask = FCOLL_MASK_CHECK_ALL;
		rayColl.nResultsLOD = FCOLL_LOD_HIGHEST;
		rayColl.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
		rayColl.bCullBacksideCollisions = TRUE;
		rayColl.Ray.Init( Head.x, Head.y, Head.z, Tail.x, Tail.y, Tail.z );

		if( fworld_CollideWithWorldTris(&rayColl) && FColl_nImpactCount > 0 )
		{
			fcoll_Sort( FALSE );	//pgm: Hi Steve, Do I need this?

			FCollImpact_t *pImpact = FColl_apSortedImpactBuf[0];
			if (pImpact)
			{
				*pfElevation = pImpact->ImpactPoint.y;
				bFoundFloor = TRUE;
			}
		}

		if (!bFoundFloor)
		{
			Head.y += fYBumpDelta;
			if (uRayTest==0)
			{
				Tail = Pt_WS;
			}
			fYBumpSoFar+=  fYBumpDelta;
		}
		uRayTest++;

		aiutils_DebugTrackRay(rayColl.Ray.vStart_WS, rayColl.Ray.vEnd_WS, bFoundFloor);
	}
	
	return bFoundFloor;  //no floor found
}



//
// Is Line Of Sight Obstructed
// Note!  Bot's are invisible 
//
BOOL aiutils_IsLOSObstructed_IgnoreBots(const CFVec3A &rRayStart, const CFVec3A &rRayEnd, CEntity* pSrcEntityHint /* NULL*/) 
{
	CFWorldTracker* pHint = NULL;
	if (pSrcEntityHint && pSrcEntityHint->TypeBits() & ENTITY_BIT_BOT)
	{
		pHint = ((CBot*) pSrcEntityHint)->m_pWorldMesh;
	}
	if (fworld_IsLineOfSightObstructed( &rRayStart,							//  const CFVec3A *pRayStart_WS, 
									&rRayEnd,							//  const CFVec3A *pRayEnd_WS, 
									0,									//  u32 nTrackerSkipCount/*=0*/, 
									NULL,								//  const CFWorldTracker * const * ppTrackerSkipList/*=NULL*/, 
									pHint,								//  const CFWorldTracker *pStartHint/*=NULL*/,
									~ENTITY_BITS_ALLBOTBITS))			//  u64 nUserTypeBitMask/*=0xffffffffffffffff*/,
	{																	//  u16 nCollisionMask/*=FCOLL_MASK_CHECK_ALL*/,
																		//  u8 nCollisionLOD/*=FCOLL_LOD_HIGHEST*/,
																		//  u16 *pAccumTriMasks/*=NULL*/ )
	  
		return TRUE;
	}
	return FALSE;  
}



const CFVec3A & aiutils_GetEntityLoc(CEntity* pEntity)
{
	return pEntity->MtxToWorld()->m_vPos;
}


f32 aiutils_GetEntityRadXZ(CEntity* pEntity)
{
	f32 fRad = 2.0f;
	if (pEntity->TypeBits() & ENTITY_BIT_BOT)
	{
		if (pEntity->AIBrain() && pEntity->AIBrain()->GetFormation())
		{
			fRad = ((CBot*) pEntity)->m_fCollCylinderRadius_WS+5.0f;
		}
		else
		{
			fRad = ((CBot*) pEntity)->m_fCollCylinderRadius_WS;
		}
	}
	else if (pEntity->TypeBits() & ENTITY_BIT_MESHENTITY)
	{
		if (((CMeshEntity*) pEntity)->GetMeshInst())
		{
			if (((CMeshEntity*) pEntity)->GetMeshInst()->m_BoundSphere_MS.m_fRadius > 18.0f)
			{
				fRad = 18.0f;
			}
		}
	}

	return fRad;
}


f32 aiutils_GetMaxBotSpeedFlatGround(CBot* pBot)
{
	return pBot->GetMaxFlatSurfaceSpeed();  // Maximum speed on a normal flat surface
}


f32 aiutils_GetEntityDist(CEntity* pEntityA, CEntity* pEntityB)
{
	return aiutils_GetEntityLoc(pEntityA).Dist(aiutils_GetEntityLoc(pEntityB));
}


f32 aiutils_GetEntityDist2(CEntity* pEntityA, CEntity* pEntityB)
{
	return aiutils_GetEntityLoc(pEntityA).DistSq(aiutils_GetEntityLoc(pEntityB));
}


f32 aiutils_GetEntityDistXZ(CEntity* pEntityA, CEntity* pEntityB)
{
	return aiutils_GetEntityLoc(pEntityA).DistXZ(aiutils_GetEntityLoc(pEntityB));
}


f32 aiutils_GetEntityDistXZ2(CEntity* pEntityA, CEntity* pEntityB)
{
	return aiutils_GetEntityLoc(pEntityA).DistSqXZ(aiutils_GetEntityLoc(pEntityB));
}


#if !FANG_PRODUCTION_BUILD

s32 _nNumRaysDebugTracking = 0;
CFVec3A _aRayOriginDebugTracking[500];
CFVec3A _aRayDestDebugTracking[500];
u32 _aRayDestDebugData[500];

void _aiutils_DebugTrackRay(const CFVec3A& origin, const CFVec3A& dest, s32 nColor)
{
	if (_nNumRaysDebugTracking < 500)
	{
		_aRayOriginDebugTracking[_nNumRaysDebugTracking] = origin;
		_aRayDestDebugTracking[_nNumRaysDebugTracking] = dest;
		_aRayDestDebugData[_nNumRaysDebugTracking] = nColor;
		_nNumRaysDebugTracking++;
	}
}

void _aiutils_DebugTrackRay(const CFVec3& origin, const CFVec3& dest, s32 nColor)
{
	if (_nNumRaysDebugTracking < 500)
	{
		_aRayOriginDebugTracking[_nNumRaysDebugTracking].Set(origin);
		_aRayDestDebugTracking[_nNumRaysDebugTracking].Set(dest);
		_aRayDestDebugData[_nNumRaysDebugTracking] = nColor;
		_nNumRaysDebugTracking++;
	}
}
#endif // !FANG_PRODUCTION_BIULD


f32 aiutils_FTotalLoopSecs(void)
{
	return FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
}


f32 aiutils_GetCurTimeSecs(void)
{
	return aiutils_FTotalLoopSecs();
}


u16 aiutils_GetCurTimeSecsU16(void)
{
	return (u16) FLoop_nTotalLoopSecs;
}


u16 aiutils_GetLastTimeSecsU16(void)
{
	return (u16) (aiutils_FTotalLoopSecs()-FLoop_fPreviousLoopSecs);
}


f32	aiutils_AITickToSecs(u16 uAITick)
{
	return (f32) uAITick*2.0f;
}


u16	aiutils_SecsToAITick(f32 fSecs)
{
	return (u16) (fSecs*0.5f);
}


u16	aiutils_GetCurAITick(void)
{
	return aiutils_SecsToAITick(aiutils_FTotalLoopSecs());
}

u16 aiutils_GetTimeOutRandMinMax(u8 uMinTime, u8 uMaxTime)
{
	u16 uTimeOut = aiutils_GetCurTimeSecsU16() + uMinTime;
	if (uMaxTime > uMinTime)
	{
		uTimeOut += fmath_RandomChoice(uMaxTime - uMinTime);
	}
	return uTimeOut;
}


f32 aiutils_GetTimeOutRandMinMax(f32 fMinTime, f32 fMaxTime)
{
	f32 fTimeOut = aiutils_GetCurTimeSecs() + fMinTime;
	if (fMaxTime > fMinTime)
	{
		fTimeOut += fmath_RandomFloat() * (fMaxTime - fMinTime);
	}
	return fTimeOut;
	}


s32 aiutils_GetNumPlayers(void)
{
	return CPlayer::m_nPlayerCount;
}


#if FANG_WINGC
static BOOL _IsWastelandCageOccupant( CEntity *pEntity ) {
	if( Level_nLoadedIndex < 0 || fclib_stricmp( Level_aInfo[Level_nLoadedIndex].pszWorldResName, "WEWJjourn01" ) ||
		!pEntity || !(pEntity->TypeBits() & ENTITY_BIT_BOTZOM) || !pEntity->Name() ) return FALSE;
	return !fclib_stricmp( pEntity->Name(), "pipezom1" ) || !fclib_stricmp( pEntity->Name(), "pipezom2" );
}

static BOOL _IsCagedWastelandZombie( CEntity *pEntity ) {
	return _IsWastelandCageOccupant( pEntity ) && !((CBot *)pEntity)->Recruit_IsRecruited();
}

BOOL aiutils_CanScriptRecruitWastelandZombie( CBot *pBot, CBot *pRecruiter, const CFVec3A *pEpicenter ) {
	// ZombieBots have neither the recruitable-class flag nor an open data port. The
	// authored cage-release Bot_Recruit calls must bypass those two ordinary restrictions.
	// A grenade supplies an epicenter, so it retains the normal recruitment rules.
	return !pEpicenter && MultiplayerMgr.IsSinglePlayer() && _IsWastelandCageOccupant( pBot ) &&
		pBot->IsCreated() && pBot->IsInWorld() && !pBot->IsDeadOrDying() &&
		!pBot->Recruit_IsRecruited() && pBot->m_nOwnerPlayerIndex < 0 &&
		pBot->m_nPossessionPlayerIndex < 0 && !pBot->DataPort_IsBeingShocked() && !pBot->DataPort_IsReserved() &&
		!pBot->Recruit_IsInstanceForbidden() &&
		pRecruiter && pRecruiter != pBot && pRecruiter->IsCreated() && pRecruiter->IsInWorld() &&
		!pRecruiter->IsDeadOrDying() && pRecruiter->m_nPossessionPlayerIndex >= 0;
}

void aiutils_EnsureFreedWastelandZombieFollowsPlayer( CEntity *pEntity ) {
	if( !MultiplayerMgr.IsSinglePlayer() || !_IsWastelandCageOccupant( pEntity ) ) return;
	CBot *pBot = (CBot *)pEntity;
	// The retail cage-destruction script recruits each bot. Wait for its shock to finish;
	// friendliness alone must never recruit a still-caged bot or override human possession.
	if( !pBot->IsCreated() || !pBot->IsInWorld() || pBot->IsDeadOrDying() ||
		!pBot->Recruit_IsRecruited() || pBot->DataPort_IsBeingShocked() ||
		pBot->m_nPossessionPlayerIndex >= 0 ) return;
	CAIBrain *pBrain = pBot->AIBrain();
	if( !pBrain || !pBrain->GetFlag_Active() ) return;

	// Prefer Glitch/P1, with a surviving partner while P1 is dead. Following the current
	// controlled body also keeps the allies useful during normal possession.
	for( s32 i=0; i<CPlayer::m_nPlayerCount && i<MAX_PLAYERS; ++i ) {
		CEntity *pPlayerEntity = Player_aPlayer[i].m_pEntityCurrent;
		if( !pPlayerEntity || !(pPlayerEntity->TypeBits() & ENTITY_BIT_BOT) || pPlayerEntity == pEntity ) continue;
		CBot *pPlayerBot = (CBot *)pPlayerEntity;
		CAIBrain *pLeader = pPlayerBot->AIBrain();
		if( !pPlayerBot->IsCreated() || !pPlayerBot->IsInWorld() || pPlayerBot->IsDeadOrDying() ||
			!pLeader || !pLeader->GetFlag_Active() ) continue;
		if( pBrain->GetLeader() == pLeader && pBrain->GetCurFollowerThoughtPtr() ) return;
		// Retry after a checkpoint/failed thought without restarting a healthy formation.
		if( pBrain->GetLeader() == pLeader ) pBrain->StopFollowing();
		pBrain->SetFlag_Buddy_Ctrl_Auto();
		if( pBrain->AssignLeader( pLeader ) ) {
			DEVPRINTF( "Port: freed cage ally '%s' follows player %d.\n", pBot->Name(), i+1 );
		}
		return;
	}
}
#endif

BOOL aiutils_IsFriendly(CEntity* pSelfEntity, CEntity* pEntityToTest, f32 *pfSuspiciousOfFriendship)
{
	if (pfSuspiciousOfFriendship)
	{
		*pfSuspiciousOfFriendship = 0.0f;
	}

	if (pSelfEntity && pEntityToTest)
	{
#if FANG_WINGC
		if( _IsCagedWastelandZombie( pSelfEntity ) || _IsCagedWastelandZombie( pEntityToTest ) ) return TRUE;
#endif
		// Special rules for multiplayer
		if (MultiplayerMgr.IsMultiplayer())
		{
			if ((pEntityToTest->TypeBits() & ENTITY_BIT_BOT) && (pSelfEntity->TypeBits() & ENTITY_BIT_BOT))
				return _Multiplayer_IsFriendly((CBot*)pSelfEntity, (CBot*)pEntityToTest);
			else
				return TRUE;	// If they are not both bots, just say friendly
		}

		// special rule for setting player's reticle color
		//
		// If 
		//   the player is asking about friendship, and that player is in a possessed bot
		// then 
		//    every entity will be considered enemy
		// s
		if( pSelfEntity && pEntityToTest && aiutils_IsPlayer( pSelfEntity ) && 
			(pSelfEntity->TypeBits() & ENTITY_BIT_BOT) &&
			(((CBot*)pSelfEntity)->m_nOwnerPlayerIndex < 0) )	 // assuming if it passed that last check it's a bot
		{		
			return FALSE;   //Enemy
		}

		//
		// if either entity is unarmed, then there is a friendly relationship.
		// NOTE! (specialrules apply for unarmed player. See aiutils_IsArmed() function.))
		if (!aiutils_IsArmed(pSelfEntity) || !aiutils_IsArmed(pEntityToTest))
		{
			return TRUE; //Friend
		}


		//
		//	Brains vs Brains
		//
		if (pSelfEntity->AIBrain() && pEntityToTest->AIBrain())
		{  
			f32 fDisguisedToMe = 0.0f;
			
			u8 uRace = pSelfEntity->AIBrain()->GetRace();
			u8 uOtherRace = pEntityToTest->AIBrain()->GetRace();
			
			if (uOtherRace != AIRACE_DROID &&
				(pEntityToTest->TypeBits() & ENTITY_BIT_BOT) &&
				(pSelfEntity->TypeBits() & ENTITY_BIT_BOT) &&
				((CBot*) pEntityToTest)->m_nPossessionPlayerIndex > -1 &&
				(((CBot*) pSelfEntity)->GetDisguisedBot(&fDisguisedToMe) == pEntityToTest) &&
				fDisguisedToMe <= 0.0f)	  
			{
				uOtherRace = AIRACE_DROID;				   //Note. All non-disguised player bots are good!  A disguise only makes good seem evil.
			}


			BOOL bFriend = FALSE;
			if (_aauRaceRelationsGrid[uRace][uOtherRace] == FRIEND)
			{
				bFriend = TRUE;
			}
			else if (_aauRaceRelationsGrid[uRace][uOtherRace] == FRIEND_UNTIL)
			{	//must see if this bot has done anything bad.
				bFriend = TRUE;

				CAIMemoryLarge* pDamageDealtMem = NULL;
				if (pEntityToTest->AIBrain()->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGE_DEALT, (CAIMemorySmall**) &pDamageDealtMem))
				{
					while (bFriend && pDamageDealtMem)
					{
						if (pDamageDealtMem->m_pEntity &&
							(pDamageDealtMem->m_pEntity == pEntityToTest ||
								(pDamageDealtMem->m_pEntity->AIBrain() && 
								_aauRaceRelationsGrid[uRace][pDamageDealtMem->m_pEntity->AIBrain()->GetRace()] != ENEMY)))
						{
							if (uRace == BOTRACE_AMBIENT && uOtherRace == BOTRACE_AMBIENT)
							{
								bFriend = !(pDamageDealtMem->m_pEntity == pSelfEntity);
							}
							else
							{
								bFriend = FALSE;
							}
							break;
						}
						pDamageDealtMem = (CAIMemoryLarge*) pEntityToTest->AIBrain()->GetKnowledge().GetNextOldestOfId(pDamageDealtMem);
					}
				}
			}

			if (pSelfEntity->TypeBits() & ENTITY_BIT_BOT &&
				pEntityToTest->TypeBits() & ENTITY_BIT_BOT &&
				(((CBot*)pSelfEntity)->GetCurMech() == pEntityToTest || ((CBot*)pEntityToTest)->GetCurMech() == pSelfEntity))
			{	  //
				bFriend = TRUE;
			}

			//
			//	Some Friendship is based on a disguise
			//	 check it now.
			if (bFriend &&
				uOtherRace != AIRACE_DROID &&				  //droids can never be disguised
				(pEntityToTest->TypeBits() & ENTITY_BIT_BOT) &&
				((CBot*) pEntityToTest)->m_nPossessionPlayerIndex > -1 &&
				fDisguisedToMe > 0.0f)
			{
				if (pfSuspiciousOfFriendship)
				{
					*pfSuspiciousOfFriendship = 1.0f - fDisguisedToMe;
				}

				if (fDisguisedToMe <= 0.0f)
				{
					bFriend = FALSE;
				}
			}
			return bFriend;
		}
		else
		{
			//non-brained comparisons
			if ( (pSelfEntity->TypeBits() & ENTITY_BIT_BOT) && (pEntityToTest->TypeBits() & ENTITY_BIT_BOT) )
			{
				f32 fDisguisedToMe = 0.0f;
				u8 uMyRace = ( pSelfEntity->AIBrain() ? pSelfEntity->AIBrain()->GetRace() : ((CBot*)pSelfEntity)->m_pBotDef->m_nRace );
				u8 uTestRace = ( pEntityToTest->AIBrain() ? pEntityToTest->AIBrain()->GetRace() : ((CBot*)pEntityToTest)->m_pBotDef->m_nRace );
				
				if (uTestRace != AIRACE_DROID &&
					((CBot*) pEntityToTest)->m_nPossessionPlayerIndex > -1 &&
					(((CBot*) pSelfEntity)->GetDisguisedBot(&fDisguisedToMe) == pEntityToTest) &&
					fDisguisedToMe <= 0.0f)	  
				{
					uTestRace = AIRACE_DROID;		//Note. All non-disguised player bots are good!  A disguise only makes good seem evil.
				}
				return (_aauRaceRelationsGrid[uMyRace][uTestRace] != ENEMY);
			}
		}
	}
	return TRUE;
}


static BOOL _IsWeaponSlotArmed(CBot* pBot, u32 uSlot)
{
	BOOL bArmed = TRUE;

	for (u32 i = 0; i < (1+pBot->m_anNumDupWeapons[uSlot]) && bArmed; i++)
	{
		CWeapon* pWeapon = NULL;
		if (i == 0)
		{
			pWeapon = pBot->m_apWeapon[uSlot];
		}
		else
		{
			pWeapon = pBot->m_aapDupWeapons[uSlot][i-1];
			FASSERT(pWeapon);
		}


		if ((!pWeapon || (((CEntity*) pWeapon)->TypeBits() & ENTITY_BIT_WEAPONHAND)) ||
			(((CEntity*) pWeapon)->TypeBits() & (ENTITY_BIT_WEAPONGREN | ENTITY_BIT_WEAPONMORTAR | ENTITY_BIT_WEAPONCLEANER | ENTITY_BIT_WEAPONEMP | ENTITY_BIT_MAGMABOMB) && !pBot->IsThrowing()))
		{
			bArmed = FALSE;
		}
	}
	return bArmed;
}


BOOL aiutils_IsArmed(CEntity* pEntity)
{
	CBot* pBot = NULL;

	if (pEntity && pEntity->TypeBits() & ENTITY_BIT_BOT)
	{
		pBot = (CBot*) pEntity;
	}

	if (pBot &&
		!(pBot->TypeBits() & ENTITY_BIT_BOTPROBE) &&	 //Probes are always considered armed.
		!(pBot->TypeBits() & ENTITY_BIT_BOTJUMPER) &&	 //Jumpers are always considered armed.
		!(pEntity->TypeBits() & ENTITY_BIT_SITEWEAPON) && // all siteweapons are considered armed and dangerous
		!(pBot->TypeBits() & ENTITY_BIT_BOTGLITCH && ((CBotGlitch*)pBot)->IsAttackingWithMelee()) &&  //glitch is armed if currenly attacking with melee
		!(pBot->TypeBits() & ENTITY_BIT_VEHICLESENTINEL && ((CVehicle*) pBot)->GetDriverBot()) &&	 //sentinels are always armed if they have a driver.  This is only necessary because they're weapons are not pointed to by CBot::m_apWeapon....
		!(pBot->TypeBits() & ENTITY_BIT_BOTZOM) && //zombies are considered to be armed.
		!(pBot->m_nPossessionPlayerIndex > -1 && !ai_IsUnarmedPlayerRuleEnabled()) &&
		!(pBot->AIBrain() && pBot->AIBrain()->GetFlag_NeverUnarmed()) &&
		!((CBot*)pEntity)->GetCurMech() &&
		!_IsWeaponSlotArmed((CBot*) pEntity, 0) &&
		!_IsWeaponSlotArmed((CBot*) pEntity, 1))  //driving a vehicle is considered being armed
	{
		return FALSE;
	}
	return TRUE;
}


CEntity* aiutils_GetPlayerEntity(s32 nPlayerIndex)
{	
	FASSERT(nPlayerIndex >=0 && nPlayerIndex < CPlayer::m_nPlayerCount);
	return Player_aPlayer[nPlayerIndex].m_pEntityCurrent;
}


BOOL aiutils_IsPlayer(CEntity* pEntity, u16* pnIndex /*=NULL*/)
{
	if (pEntity &&
		(pEntity->TypeBits() & ENTITY_BIT_BOT) && 
		((CBot*) pEntity)->m_nPossessionPlayerIndex >=0)
	{
		if (pnIndex)
		{
			*pnIndex = (u16) ((CBot*) pEntity)->m_nPossessionPlayerIndex;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL aiutils_GetPlayerIndex(CEntity* pEntity, u16* pnIndex /*=NULL*/)
{
	if (pEntity &&
		(pEntity->TypeBits() & ENTITY_BIT_BOT) && 
		((CBot*) pEntity)->m_nPossessionPlayerIndex >=0)
	{
		if (pnIndex)
		{
			*pnIndex = (u16) ((CBot*) pEntity)->m_nPossessionPlayerIndex;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL aiutils_IsDeadOrDying(CEntity* pEntity)
{
	return (pEntity &&
			(pEntity->TypeBits() & ENTITY_BIT_BOT) && 
			((CBot*) pEntity)->IsDeadOrDying());
}


BOOL aiutils_GetCamPos(CFVec3A* pCamPos)
{
	FViewport_t * pView = fviewport_GetActive( );
	if (pView)
	{
		if (pCamPos)
		{
			(*pCamPos).Set(FXfm_pView->m_MtxR.m_vPos);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL aiutils_GetCamLookAt(CFVec3A* pCamLookAt)
{
	FViewport_t * pView = fviewport_GetActive( );
	if (pView && FXfm_pView)
	{
		if (pCamLookAt)
		{
			CFVec3A tmpAxisZ = CFVec3A::m_UnitAxisZ;
			(*pCamLookAt) = FXfm_pView->m_MtxR.MulDir(tmpAxisZ);
			(*pCamLookAt).Unitize();
			return TRUE;
		}
	}
	return FALSE;
}


BOOL aiutils_CanEntityFly(CEntity* pEntity)
{
	return (pEntity && 
			pEntity->AIBrain() &&
			pEntity->AIBrain()->GetAIMover() &&
			pEntity->AIBrain()->GetAIMover()->CanFly());
}


f32 aiutils_GetBotAimPitch(CBot* pBot)
{
	return pBot->m_fMountPitch_WS;
}


BOOL aiutils_IsCBot(CEntity* pEntity)
{
	return pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOT);
}


BOOL aiutils_IsCBotBrain(CAIBrain* pBrain)
{
	return (pBrain &&
			pBrain->GetAIMover() &&
			pBrain->GetAIMover()->GetEntity() &&
			pBrain->GetAIMover()->GetEntity() &&
			(pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT));
}


BOOL aiutils_GetCurMech(CEntity* pEntity, CEntity** ppMech /* =NULL*/)
{
	if (pEntity &&
		(pEntity->TypeBits() & ENTITY_BIT_BOT) &&
		((CBot*)pEntity)->GetCurMech())
	{
		if (ppMech)
		{
			*ppMech = ((CBot*)pEntity)->GetCurMech();
		}
		return TRUE;
	}
	
	// this code is just incase somehow the GetCurMech call gets jacked.
	if (pEntity &&
		pEntity->GetParent() &&
		(((pEntity->GetParent()->TypeBits() & ENTITY_BIT_VEHICLE) &&
		 ((CVehicle*) pEntity->GetParent())->GetDriverBot() == pEntity) ||
		 ((pEntity->GetParent()->TypeBits() & ENTITY_BIT_SITEWEAPON) &&
		 ((CBotSiteWeapon*) pEntity->GetParent())->GetDriverBot() == pEntity)))
	{
		if (ppMech)
		{
			*ppMech = pEntity->GetParent();
		}
		return TRUE;
	}
	return FALSE;
}


BOOL aiutils_HasShield(CEntity* pEntity)
{
	BOOL bShield = FALSE;
	if (pEntity &&
		pEntity->TypeBits() & ENTITY_BIT_BOTTITAN &&
		((CBotTitan*) pEntity)->GetShield())
	{
		bShield = TRUE;
	}
	return bShield;
}


BOOL aiutils_IsShieldActive(CEntity* pEntity)
{
	BOOL bShieldOn = FALSE;
	if (pEntity &&
		pEntity->TypeBits() & ENTITY_BIT_BOTTITAN &&
		((CBotTitan*) pEntity)->GetShield() &&
		((CBotTitan*) pEntity)->GetShield()->IsEnabled() && 
		((CBotTitan*) pEntity)->GetShield()->NormHealth() > 0.0f)
	{
		bShieldOn = TRUE;
	}
	return bShieldOn;
}


u8 aiutils_GetNumWeapons(CEntity* pEntity)
{
	u8 uNumWeaponCtrls = 0;
	if (pEntity && pEntity->AIBrain())
	{
		uNumWeaponCtrls = pEntity->AIBrain()->GetNumWeapons();
	}
	return uNumWeaponCtrls;
}


BOOL aiutils_BrainLocToScreenPos(CAIBrain* pBrain, CFVec3A *pScreenPos)
{
	//3d Brain to ScreenPos text

	if (pBrain)
	{
		CFVec3A Pt_WS = pBrain->GetAIMover()->GetLoc();
		Pt_WS.y +=   pBrain->GetAIMover()->GetHeight();
		return aiutils_WSToScreenPos(Pt_WS, pScreenPos);
	}
	return FALSE;
}


BOOL aiutils_WSToScreenPos(const CFVec3A &Pt_WS, CFVec3A *pScreenPos)
{
	//3d Brain to ScreenPos text
	BOOL bInView = FALSE;
	FViewport_t * pView = fviewport_GetActive();

	CFVec3A ScreenPos;
	ScreenPos.z = -1.0f;

	CFVec3A Point_VS = Pt_WS;
	if (FXfm_pView)
	{
		FXfm_pView->m_MtxF.MulPoint(Point_VS);
		if (Point_VS.z > 0.0f)
		{
			fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &(FXfm_pView->m_MtxF), &(Pt_WS), &ScreenPos);
			if (ScreenPos.z >0.0f && ScreenPos.x < 1.0f && ScreenPos.x > -1.0f && ScreenPos.y < 1.0f && ScreenPos.y > -1.0f)
			{
				ScreenPos.x = 0.5f+0.5f*ScreenPos.x;
				ScreenPos.y = (0.375f -ScreenPos.y*0.375f);
				bInView = TRUE;
			}
		}
	}

	if (bInView && pScreenPos)
	{
		*pScreenPos = ScreenPos;
	}
	return bInView;
}


BOOL aiutils_IsInBoundsOfWorld(const CFVec3A& Pt_WS)
{
	return (Pt_WS.x > FVis_pVisData->CellTree.paNodes[FVis_pVisData->CellTree.nFirstNodeIndex].AABB.vMinXYZ.x &&
			Pt_WS.x < FVis_pVisData->CellTree.paNodes[FVis_pVisData->CellTree.nFirstNodeIndex].AABB.vMaxXYZ.x &&
			Pt_WS.y > FVis_pVisData->CellTree.paNodes[FVis_pVisData->CellTree.nFirstNodeIndex].AABB.vMinXYZ.y &&
			Pt_WS.y < FVis_pVisData->CellTree.paNodes[FVis_pVisData->CellTree.nFirstNodeIndex].AABB.vMaxXYZ.y &&
			Pt_WS.z > FVis_pVisData->CellTree.paNodes[FVis_pVisData->CellTree.nFirstNodeIndex].AABB.vMinXYZ.z &&
			Pt_WS.z < FVis_pVisData->CellTree.paNodes[FVis_pVisData->CellTree.nFirstNodeIndex].AABB.vMaxXYZ.z);

}

BOOL aiutils_IsInBoundsOfWorld(CEntity* pEntity)
{
	return (pEntity &&
			aiutils_IsInBoundsOfWorld(pEntity->MtxToWorld()->m_vPos));
}



s32 aiutils_FindClosestTo(const CFVec3A& to, const CFVec3A	*paPts, s32 nNumPts)
{
	f32 fClosestDistSq = 0.0f;
	s32 nClosest = -1;

	for (s32 i = 0; i < nNumPts; i++)
	{
		f32 fDistSq = to.DistSq(paPts[i]);
		if (nClosest || fDistSq < fClosestDistSq)
		{
			nClosest = i;
			fClosestDistSq = fDistSq ;
		}
	}

	return nClosest;
}

s32 aiutils_FindRandomBitIndex(u8 uVal)
{
	s32 nWinner = -1;
	u8 auEval[8];
	s32 nEvals = 0;

	if (uVal <3)
	{
		return ((s32) uVal) - 1;
	}
	for (u32 i = 0; i < 8; i++)
	{
		if (uVal & (1<<i))
		{
			auEval[nEvals++] = i;
		}
	}
	return auEval[fmath_RandomChoice(nEvals)];
}

// Used to figure nice parabolas with variable airtime, and arc depending on velocity
f32	 aiutils_ComputeTrajectoryFromXZVel(CFVec3A& rVelocityVec, const CFVec3A& rStartPos,const CFVec3A& rEndPos,f32 fXZVelocity, f32 fGravity)
{
	// Figure out how far away the target is
	CFVec3A vPosToTargetXZ;
	vPosToTargetXZ.x = rEndPos.x - rStartPos.x;
	vPosToTargetXZ.y = 0.0f;
	vPosToTargetXZ.z = rEndPos.z - rStartPos.z;

	f32 fMagSqXZ = vPosToTargetXZ.MagSqXZ();
	if (fMagSqXZ < 0.001f)
	{
		DEVPRINTF("aiutils_ComputeTrajectoryFromXZVel() bad imput values");
		return 0.0f;
	}
	
	CFVec3A vUnitPosToTargetXZ;
	f32 fInvDistXZ = fmath_InvSqrt( fMagSqXZ );
	vUnitPosToTargetXZ.x = vPosToTargetXZ.x *fInvDistXZ;
	vUnitPosToTargetXZ.y = 0.f;
	vUnitPosToTargetXZ.z = vPosToTargetXZ.z *fInvDistXZ;
	f32 fDistXZ = fmath_Inv(fInvDistXZ);
	// now we know distance and direction to target;

	// give our XZ velocity, that implies the airtime
	f32 fOOTimeInAir = fXZVelocity * fInvDistXZ;
	f32 fTimeInAir = fmath_Inv(fOOTimeInAir);

	// compute the velocity vector of a projectile given the travel time
	rVelocityVec.x = fXZVelocity * vUnitPosToTargetXZ.x;
	rVelocityVec.z = fXZVelocity * vUnitPosToTargetXZ.z;
	rVelocityVec.y = (fTimeInAir * fTimeInAir * 0.5f * fGravity - (rStartPos.y - rEndPos.y)) * fOOTimeInAir;

	return fTimeInAir;
}

// Used to figure nice parabolas with constant airtime
f32	 aiutils_ComputeTrajectoryFromAirTime(CFVec3A& rVelocityVec, const CFVec3A& rStartPos,const CFVec3A& rEndPos,f32 fTimeInAir, f32 fGravity)
{
	// Figure out how far away the target is
	CFVec3A vPosToTargetXZ;
	vPosToTargetXZ.x = rEndPos.x - rStartPos.x;
	vPosToTargetXZ.y = 0.0f;
	vPosToTargetXZ.z = rEndPos.z - rStartPos.z;

	f32 fMagSqXZ = vPosToTargetXZ.MagSqXZ();
	if (fMagSqXZ < 0.001f)
	{
		DEVPRINTF("aiutils_ComputeTrajectoryFromAirTime() bad imput values");
		return 0.0f;
	}
	
	CFVec3A vUnitPosToTargetXZ;
	f32 fInvDistXZ = fmath_InvSqrt( fMagSqXZ );
	vUnitPosToTargetXZ.x = vPosToTargetXZ.x *fInvDistXZ;
	vUnitPosToTargetXZ.y = 0.f;
	vUnitPosToTargetXZ.z = vPosToTargetXZ.z *fInvDistXZ;
	f32 fDistXZ = fmath_Inv(fInvDistXZ);
	// now we know distance and direction to target;

	// give our airtime, that implies the XZ velocity
	f32 fOOTimeInAir = fmath_Inv(fTimeInAir);
	f32 fXZVelocity  = fDistXZ * fOOTimeInAir;

	// compute the velocity vector of a projectile given the travel time
	rVelocityVec.x = fXZVelocity * vUnitPosToTargetXZ.x;
	rVelocityVec.z = fXZVelocity * vUnitPosToTargetXZ.z;
	rVelocityVec.y = (fTimeInAir * fTimeInAir * 0.5f * fGravity - (rStartPos.y - rEndPos.y)) * fOOTimeInAir;
	
	return fTimeInAir;
}


CBot* aiutils_GetMechOperator(CBot* pMech)
{
	CBot* pOperator = NULL;
	if (pMech)
	{
		if (pMech->TypeBits() & ENTITY_BIT_VEHICLE)
		{
			pOperator = ((CVehicle*) pMech)->GetDriverBot();
		}
		else if (pMech->TypeBits() & ENTITY_BIT_SITEWEAPON && (((CBot*) pMech)->IsPillBox() || ((CBot*) pMech)->IsRatGun()) )
		{
			pOperator = ((CBotSiteWeapon*) pMech)->GetDriverBot();
		}
	}
	return pOperator;
}


BOOL aiutils_IsMech(CEntity* pEntity)
{
	return (pEntity->TypeBits() & ENTITY_BIT_VEHICLE ||	
			(pEntity->TypeBits() & ENTITY_BIT_SITEWEAPON && (((CBot*) pEntity)->IsPillBox() || ((CBot*) pEntity)->IsRatGun()) ));
}

BOOL aiutils_IsMechSeatObstructed(CBot* pMech, u16 uSeatId, CEntity* pExceptBy /*= NULL*/)
{
	CBot* pOperator = NULL;
	if (pMech)
	{
		pOperator = aiutils_GetMechOperator(pMech);
		if (pOperator == pExceptBy)
		{
			pOperator = NULL;
		}
		
		if (!pOperator &&
			pMech->TypeBits() & ENTITY_BIT_VEHICLE)
		{
			pOperator = ((CVehicle*) pMech)->IsStationObstructed(CVehicle::STATION_DRIVER);
	
			if (pOperator == pExceptBy)
			{
				pOperator = NULL;
			}
		}
		else if (!pOperator &&
				 pMech->TypeBits() & ENTITY_BIT_SITEWEAPON &&
				 (((CBot*) pMech)->IsPillBox() || ((CBot*) pMech)->IsRatGun()) )
		{
			for (s32 i = 0; i < CPlayer::m_nPlayerCount && !pOperator; i++)
			{
				if (Player_aPlayer[i].m_pEntityCurrent &&
					Player_aPlayer[i].m_pEntityCurrent->IsInWorld() &&
					Player_aPlayer[i].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT &&
					!((CBot*) Player_aPlayer[i].m_pEntityCurrent)->IsDeadOrDying() &&
					Player_aPlayer[i].m_pEntityCurrent->MtxToWorld()->m_vPos.DistSq(pMech->MtxToWorld()->m_vPos) < ((pMech->m_fCollCylinderRadius_WS + 5.0f)*(pMech->m_fCollCylinderRadius_WS + 5.0f)))
				{
					pOperator = (CBot*) Player_aPlayer[i].m_pEntityCurrent;

					if (pOperator == pExceptBy)
					{
						pOperator = NULL;
					}
				}
			}
		}
	}
	return pOperator != NULL;
}

// returning FALSE aborts the collision test.
CFSphere aiutils_BotInSphere;
BOOL aiutils_BotInSphereTrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	if( !pTracker->IsCollisionFlagSet() ) {
		// Not collidable...
		return TRUE;
	}

	u32 i;

	for( i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not an entity...
		return TRUE;
	}

	if( !(((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT) ) {
		// Not a bot...
		return TRUE;
	}

	CBot *pBot = (CBot *)pTracker->m_pUser;

	if( pBot->m_pBotInfo_Gen == NULL ) {
		// Not collision sphere info...
		return TRUE;
	}
/*
	if( pBot->TypeBits() & ENTITY_BIT_BOTZOM ) {
		// This is a ZombieBot...

		CBotZom *pBotZom = (CBotZom *)pBot;

		if( pBotZom->m_nZomMode == ZOMMODE_IN_PIECES ) {
			// Ignore collision with ZombieBots that are in pieces...
			return TRUE;
		}
	}
  */
	CFSphere HisCollSphere_WS;
	HisCollSphere_WS.m_Pos.x = pBot->m_pBotInfo_Gen->fCollSphere1X_MS + pBot->MtxToWorld()->m_vPos.x;
	HisCollSphere_WS.m_Pos.y = pBot->ComputeCollSphereY() + pBot->MtxToWorld()->m_vPos.y - 1.0f;
	HisCollSphere_WS.m_Pos.z = pBot->m_pBotInfo_Gen->fCollSphere1Z_MS + pBot->MtxToWorld()->m_vPos.z;
	HisCollSphere_WS.m_fRadius = pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;

	if( !aiutils_BotInSphere.IsIntersecting( HisCollSphere_WS ) ) {
		// No collision...
		return TRUE;
	}

	// This bot's collision sphere intersects ours...

	return FALSE;
}


// Returns TRUE if another bot's collision sphere occupies the same space as ours.
BOOL aiutils_DoesSphereIntersectAnyBotsBesidesSkipList( const CFSphere &Sphere_WS, u32 uSkipListCount /*NULL*/ )
{
	FWorld_nTrackerSkipListCount = uSkipListCount;
	CFWorldUser UserTracker;
	UserTracker.MoveTracker( Sphere_WS );
	aiutils_BotInSphere = Sphere_WS;
	BOOL bFoundAny = FALSE;
	bFoundAny = !UserTracker.FindIntersectingTrackers( aiutils_BotInSphereTrackerCollisionCallback, FWORLD_TRACKERTYPE_MESH );

	return bFoundAny;
}

static BOOL _Multiplayer_IsFriendly(CBot* pSelfBot, CBot* pTestBot)
{
	// If both bots are player controlled, then ignore recruitment and use
	// only original teams.
	if (pSelfBot->IsPlayerBot() && pTestBot->IsPlayerBot())
	{
		if (MultiplayerMgr.IsTeamPlay())
			return (Player_aPlayer[pSelfBot->m_nPossessionPlayerIndex].m_nTeamNum == Player_aPlayer[pTestBot->m_nPossessionPlayerIndex].m_nTeamNum);
		else
			return FALSE;
	}

	// If they are both on the same team, no need to check further
	if (pSelfBot->IsSameTeam(pTestBot))
		return TRUE;

	// Different teams, so check further

	// Powered down bots are friendly
	if (pTestBot->Power_IsPoweredDown())
		return TRUE;

	// Check things which take drivers and which are enemies only
	// when occupied by an enemy
	if (pTestBot->TypeBits() & ENTITY_BIT_VEHICLE)
	{
		CBot* pDriverBot = ((CVehicle*)pTestBot)->GetDriverBot();
		return (pDriverBot == NULL) ? TRUE : pSelfBot->IsSameTeam(pDriverBot);
	}
	else if (pTestBot->IsPillBox() || pTestBot->IsRatGun())
	{
		CBot* pDriverBot = ((CBotSiteWeapon*)pTestBot)->GetDriverBot();
		return (pDriverBot == NULL) ? TRUE : pSelfBot->IsSameTeam(pDriverBot);
	}

	// Must be a bad guy...
	return FALSE;
}
