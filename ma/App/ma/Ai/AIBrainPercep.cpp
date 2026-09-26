//////////////////////////////////////////////////////////////////////////////////////
// AIBrainPercep.cpp - 
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
// 11/13/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fworld_coll.h"
#include "AIBrain.h"
#include "AIBrainMems.h"
#include "AIMover.h"
#include "AIBrainMan.h"
#include "AIGroup.h"
#include "AIGameUtils.h"
#include "../protrack.h"
#include "../player.h"
#include "../damage.h"
#include "AIThoughtsGeneric.h" //CGenericWait.. findfix: why again?
#include "../meshtypes.h"

#define NPC_BOT_SEENMEM_TIME 5

//
//	   Sound Recording Algo
//
//	   Each bot only every remembers one sound at a time.
//	   Only newer sounds will bump the currently remembered sound if there is one.
//	   SoundCollisionCB should never be called for sounds
//        that have been created by the listener
//
//		  If a sound is recorder, return TRUE to the callback,
//           to indicate there is no need for anymore sounddetection
//          this frame
//           

static CAIMemoryLarge* _pHeardMemCBData = NULL;
static const u32 _kuRememberSoundsFor = 2;
void _SoundCollisionCBInit(void)
{
	_pHeardMemCBData = NULL; //set this to NULL.  It is static data used by _SoundCollisionCB
}

f32 fDebugDisplayF32 = 0.0f;


//static
BOOL CAIBrain::_SoundCollisionCB(CAISound* pSound, void* pData)
{
	CAIBrain* pThisBrain = (CAIBrain*) pData;

	FASSERT(pSound);
	BOOL bNewSound = FALSE;

	if (!_pHeardMemCBData &&
		!pThisBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_HEARD, (CAIMemorySmall**) &_pHeardMemCBData))
	{
		_pHeardMemCBData = pThisBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_HEARD, _kuRememberSoundsFor);
		bNewSound = TRUE;
	}
	else
	{
		if (_pHeardMemCBData)
		{
			CAISound* pHearMemSound = AIEnviro_SoundHandleToPtr(_pHeardMemCBData->m_uMediumData);
			if (!pHearMemSound ||
				pSound->m_fWhen > pHearMemSound->m_fWhen ||
				(pSound == pHearMemSound && pSound->m_uSoundCtrlFlags & AISOUNDCTRL_MODIFIED_SOUND))
			{
				bNewSound = TRUE;
				_pHeardMemCBData->ResetTimer(_kuRememberSoundsFor);
			}
		}
	}

	if (_pHeardMemCBData && bNewSound)
	{
		_pHeardMemCBData->m_pEntity = pSound->m_pEntity;
		_pHeardMemCBData->m_Loc = pThisBrain->GetLoc().v3;
		_pHeardMemCBData->m_EntityLoc = pSound->m_Origin.v3;
		_pHeardMemCBData->m_uMediumData = (u32) pSound->m_uHandle;
	}
	return bNewSound;	//TRUE means stop iterating please!
}


BOOL CAIBrain::DoVisCheckAndVerifyLOSWithEntity(CEntity* pOtherEntity, 	CAIMemoryMedium* pVisCheckMem /*= NULL*/)
{
	CAIBrain* pThisBrain = this;
	CFVec3A LookAtLoc;
	BOOL bCanSeeTarg = FALSE;
	CAIMover* pMover = pThisBrain->GetAIMover();
	CFVec3A EyeLoc = pThisBrain->GetAIMover()->GetEyePos();
	BOOL bOtherIsPlayer = (pOtherEntity->TypeBits() & ENTITY_BIT_BOT) && ( ((CBot*) pOtherEntity)->m_nPossessionPlayerIndex != -1);
	u8 uCurTag = 0;


	if (bOtherIsPlayer && pVisCheckMem)
	{
		uCurTag = pVisCheckMem->m_uSmallData & 0x00ff;
		LookAtLoc = *(pOtherEntity->GetTagPoint(uCurTag));
	} 
	else
	{
		LookAtLoc = *(pOtherEntity->GetTagPoint(0));
	}

	
	//Verify LOS
	PROTRACK_BEGINBLOCK("EyeRays");
	FWorld_nTrackerSkipListCount = 0;
	
	//The other bots trackers to ignore in the LOS test.
	pOtherEntity->AppendTrackerSkipList();	
	CEntity* pMech = NULL;
	aiutils_GetCurMech(pOtherEntity, &pMech);
	if (pMech)
	{
	   pMech->AppendTrackerSkipList();
	}

	//My trackers to ignore in the LOS test.
	pMover->GetEntity()->AppendTrackerSkipList();
	pMech  = NULL;
	aiutils_GetCurMech(pMover->GetEntity(), &pMech);
	if (pMech)
	{
	   pMech->AppendTrackerSkipList();
	}
	if (!fworld_IsLineOfSightObstructed( &EyeLoc, &LookAtLoc, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, ((CBot*) pMover->GetEntity())->m_pWorldMesh))
	{
		//we have a hit
		bCanSeeTarg = TRUE;
	}
	aiutils_DebugTrackRay(EyeLoc, LookAtLoc, bCanSeeTarg);

	if (pVisCheckMem)
	{
		u16 uTagPtStats = pVisCheckMem->m_uSmallData & 0xff00;
		uTagPtStats &= ~(1<<(8+uCurTag));
		uTagPtStats |= bCanSeeTarg*(1<<(8+uCurTag));
		pVisCheckMem->m_uSmallData &=~0xff00;
		pVisCheckMem->m_uSmallData |= uTagPtStats&0xff00;
	}

	PROTRACK_ENDBLOCK();//"EyeRay");

	return bCanSeeTarg;
}


BOOL CAIBrain::IntersectsVisionCone(const CFVec3A& Pt_WS, f32 fOtherRad, f32 fVisionRange, f32 *pfDot, f32 *pfDist)
{
	f32 fRadSum = fOtherRad + fVisionRange;
	BOOL bCanSeeTarg = FALSE;
	CAIMemoryLarge* pSeenMemory = NULL;
	CFVec3A LookVec = GetAIMover()->GetEyeLookAt();
	CFVec3A EyeLoc = GetAIMover()->GetEyePos();
	CFVec3A DeltaToTarg = Pt_WS;
	DeltaToTarg.Sub(EyeLoc);
	f32 fDistToTarg2 = DeltaToTarg.MagSq();

	if (fDistToTarg2 > 0.0f && 
		fDistToTarg2 < fRadSum*fRadSum &&		  //within 
		(GetEyeCosHalfFOV() < 0.0f || DeltaToTarg.Dot(LookVec) > 0.0f))	  //in front of me
	{
		f32 fDistToTarg = fmath_Sqrt(fDistToTarg2);
		DeltaToTarg.Mul(1.0f/fDistToTarg);
		f32 fDot = DeltaToTarg.Dot(LookVec);
		if (fDot > GetEyeCosHalfFOV())
		{
			if (pfDot)
			{
				*pfDot = fDot;
			}
			if (pfDist)
			{
				*pfDist = fDistToTarg;
			}
			return TRUE;//in field of View
		}
	}
	return FALSE;
}


//static
static CAISound* _pClosestProjectileThisFrame = NULL;
static f32 _fClosestProjectileThisFrameDistSq = 0.0f;
void CAIBrain::_BeginVisibleEntityScan(void)
{
	_pClosestProjectileThisFrame = NULL;
	_fClosestProjectileThisFrameDistSq = 0.0f;
}


BOOL CAIBrain::_CanSeeVisibleEntity_CB(CAISound* pSound, void* pData)
{
	CAIBrain* pThisBrain = (CAIBrain*) pData;
	f32 fDistSq = 0.0f;
	if (pSound->m_uSoundType == AISOUNDTYPE_PROJECTILE &&
		pThisBrain->IntersectsVisionCone(pSound->m_Origin, pSound->m_fAudibleRadius, pThisBrain->GetEyeScanDist()) &&
		!_pClosestProjectileThisFrame || (fDistSq = pSound->m_Origin.DistSq(pThisBrain->GetAIMover()->GetLoc())) < _fClosestProjectileThisFrameDistSq)
	{
		_pClosestProjectileThisFrame = pSound;
		_fClosestProjectileThisFrameDistSq = fDistSq;
	}
	else if (pSound->m_pEntity)
	{
		pThisBrain->CanSeeIt(pSound->m_pEntity, pSound->m_fAudibleRadius);
	}

	return FALSE; //TRUE means stop iterating please!
}


void CAIBrain::_EndVisibleEntityScan(CAIBrain* pThisBrain)
{
	//Verify LOS
	if (_pClosestProjectileThisFrame)
	{
		PROTRACK_BEGINBLOCK("EyeRays");
		if (!aiutils_IsLOSObstructed_IgnoreBots( pThisBrain->GetAIMover()->GetEyePos(), _pClosestProjectileThisFrame->m_Origin, pThisBrain->GetAIMover()->GetEntity()))
		{
			//we have a hit
			aiutils_DebugTrackRay(pThisBrain->GetAIMover()->GetEyePos(), _pClosestProjectileThisFrame->m_Origin, TRUE);
		}
		else
		{
			aiutils_DebugTrackRay(pThisBrain->GetAIMover()->GetEyePos(), _pClosestProjectileThisFrame->m_Origin, FALSE);
			_pClosestProjectileThisFrame = NULL;
		}
		PROTRACK_ENDBLOCK();//"EyeRay");
	}

	if (_pClosestProjectileThisFrame)
	{	//we have a hit
		CAIMemoryLarge* pSeenMemory = NULL;
		if (pThisBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_SEEN_PROJECTILE, (CAIMemorySmall **) &pSeenMemory))
		{
	//		pSeenMemory->ResetTimer(2);
			pSeenMemory->m_uWhenTimeSecs = aiutils_GetCurTimeSecsU16();
		}
		else
		{
			pSeenMemory = pThisBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_SEEN_PROJECTILE, 1);
		}

		if (pSeenMemory)
		{
			pSeenMemory->m_Loc = pThisBrain->GetLoc().v3;
			pSeenMemory->m_EntityLoc = _pClosestProjectileThisFrame->m_Origin.v3;
			pSeenMemory->m_pEntity = (_pClosestProjectileThisFrame)->m_pEntity;
		}
	}
	_pClosestProjectileThisFrame = NULL;
}


void CAIBrain::CanSeeIt(CEntity* pEntity, f32 fRadius)
{
	CAIBrain* pThisBrain = this;
	CAIMemoryLarge* pSeenMemory = NULL;
	BOOL bCanSeeTarg = FALSE;

	CFVec3A OtherLoc = pEntity->MtxToWorld()->m_vPos;
	f32 fOtherRadius = fRadius;
	if (fOtherRadius < 0.0f)
	{
		fOtherRadius = 1.0f;
	}

	f32 fScanDist = pThisBrain->GetEyeScanDist();
	if (pThisBrain->GetLeader() && 
		(pThisBrain->GetLeader()->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) &&
		(((CBot*) pThisBrain->GetLeader()->GetAIMover()->GetEntity())->m_nPossessionPlayerIndex != -1))
	{
		fScanDist += pThisBrain->GetBonusEyeScanDistForPlayers();
	}

	pThisBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, pEntity, &pSeenMemory);

	BOOL bOtherHasMovedRecently = FALSE;
	if (pEntity->AIBrain())
	{
		bOtherHasMovedRecently = pEntity->AIBrain()->HasMovedOneFootSince(1.0f);
	}
	else if (pEntity->TypeBits() & ENTITY_BIT_BOT)
	{
		bOtherHasMovedRecently = TRUE;//((CBot*) pEntity)->m_fSpeed_WS > 0.0f;  would use speed, but need override for 
	}
	else 
	{
		bOtherHasMovedRecently = TRUE;//findfix: optimize here 
	}

	if (pThisBrain->m_fBrainAlertUnit < 0.3f &&			//some optimization code to reduce number of raycasts between non-moving npc bots that aren't in the action
		!pThisBrain->HasMovedOneFootSince(1.0f) && 
		!bOtherHasMovedRecently &&
		pThisBrain->GetAIMover()->m_Controls.GetEntityControl()->m_fAimDown  == 0.0f &&
		pThisBrain->GetAIMover()->m_Controls.GetEntityControl()->m_fRotateCW == 0.0f)
	{
		if (!pSeenMemory)
		{
			bCanSeeTarg = FALSE;
		}
		else if (pSeenMemory && (pSeenMemory->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY))
		{
			bCanSeeTarg = TRUE;
		}
	}
	else
	{
		bCanSeeTarg = (	pThisBrain->IntersectsVisionCone(OtherLoc, fOtherRadius, fScanDist) &&
						pThisBrain->DoVisCheckAndVerifyLOSWithEntity(pEntity));
	}

	if (bCanSeeTarg)
	{
		if (pSeenMemory)
		{
			pSeenMemory->ResetTimer(5);
		}
		else
		{
			pSeenMemory = pThisBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_SEEN, NPC_BOT_SEENMEM_TIME);
		}

		if (pSeenMemory)
		{
			pSeenMemory->m_Loc = pThisBrain->GetLoc().v3;
			pSeenMemory->m_EntityLoc = OtherLoc.v3;
			pSeenMemory->m_pEntity = pEntity;
		}
	}

}

//static
BOOL CAIBrain::_CanSeeNPCBrain_CB(CAIBrain* pOtherBrain, void* pData)
{
	CAIBrain* pThisBrain = (CAIBrain*) pData;
	CAIMover* pMover = pThisBrain->GetAIMover();
	CAIMover* pOtherMover = pOtherBrain->GetAIMover();

	if (pOtherMover == pMover ||
		((pOtherMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT) && ( ((CBot*) pOtherMover->GetEntity())->m_nPossessionPlayerIndex != -1)))
	{
		return AIBRAINMAN_CONTINUE_ITERATION;	 //skip it
	}
	
	pThisBrain->CanSeeIt(pOtherMover->GetEntity(), pOtherMover->GetRadiusXZ());

	return AIBRAINMAN_CONTINUE_ITERATION;
}


//static
BOOL CAIBrain::_CanSeePlayer(CAIBrain* pThisBrain, CEntity* pPlayerEntity)
{
	CAIMover* pMover = pThisBrain->GetAIMover();
	CAIMemoryLarge* pSeenMemory = NULL;
	CAIMemoryMedium* pVisCheckMem = NULL;
	BOOL bCanSeeTarg = FALSE;
	u8 uCurTag;
	f32 fPeripheralDot = 0.0f;
	f32 fDistToObj = 0.0f;

	BOOL bOtherIsPlayer = (pPlayerEntity->TypeBits() & ENTITY_BIT_BOT) && (((CBot*) pPlayerEntity)->m_nPossessionPlayerIndex != -1);
	FASSERT(bOtherIsPlayer);

	CFVec3A OtherLoc = pPlayerEntity->MtxToWorld()->m_vPos;
	f32 fOtherRad = ((CBot*) pPlayerEntity)->m_fCollCylinderRadius_WS;
	f32 fScanDist = pThisBrain->GetEyeScanDist() + pThisBrain->GetBonusEyeScanDistForPlayers() * bOtherIsPlayer;

	if (pThisBrain->IntersectsVisionCone(OtherLoc, fOtherRad, fScanDist, &fPeripheralDot, &fDistToObj))
	{
		//maintain a vis-check in memory that pertains to this entity I'm looking at.
		if (bOtherIsPlayer && pPlayerEntity->AIBrain())
		{
			CNiIterator<CAIMemoryMedium*> Search = pThisBrain->GetKnowledge().SearchMedium();
			while (Search.IsValid())
			{
				CAIMemoryMedium* pMem = Search.Get();
				if (pMem->m_uMemoryId == MEMORY_MEDIUM_VISCHECK &&
					pMem->m_uMediumData == pPlayerEntity->Guid())
				{
					pVisCheckMem = pMem;
					pVisCheckMem->m_uHowLongSecs++; //make the vischeck memory stay around one more frame
					break;	//found the vis-check, quit searching
				}
				Search.Next();
			}

			if (!pVisCheckMem)
			{
				pVisCheckMem = pThisBrain->GetKnowledge().RememberMedium(MEMORY_MEDIUM_VISCHECK, 1, CAIMemorySmall::CONTROL_FLAG_FRAMECOUNT); //remember one frame at a time
				if (pVisCheckMem)
				{
					pVisCheckMem->m_uMediumData = pPlayerEntity->Guid();
					pVisCheckMem->m_fMediumData = 0.0f;  //Initial Focus is zero
					pVisCheckMem->m_uSmallData = 0;
				}
			}
		}

		bCanSeeTarg = pThisBrain->DoVisCheckAndVerifyLOSWithEntity(pPlayerEntity, pVisCheckMem);
	}

	if (bCanSeeTarg)
	{
		if (pThisBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, pPlayerEntity, &pSeenMemory))
		{
			pSeenMemory->ResetTimer(5);
			pSeenMemory->m_uWhenTimeSecs = aiutils_GetCurTimeSecsU16();
		}
		else
		{
			pSeenMemory = pThisBrain->GetKnowledge().RememberLarge(MEMORY_LARGE_SEEN, 5);
		}

		if (pSeenMemory)
		{
			pSeenMemory->m_Loc = pThisBrain->GetLoc().v3;
			pSeenMemory->m_EntityLoc = OtherLoc.v3;
			pSeenMemory->m_pEntity = pPlayerEntity;
		}
	}

	if (pVisCheckMem)
	{
		//Update VisCheck 

		//Advance and store the TagPtCounter inside of uSmallData
		u32 uNumTags = pPlayerEntity->GetTagPointCount();
		uCurTag = pVisCheckMem->m_uSmallData & 0x00ff;
		u8 uTagPtCounter = uCurTag +1;
		if (uTagPtCounter >= uNumTags)
		{
			uTagPtCounter = 0;
		}
		pVisCheckMem->m_uSmallData &= 0xff00;
		pVisCheckMem->m_uSmallData |= uTagPtCounter & 0x00ff;

		//Get the number of visible tags
		u32 i;
		u8 uNumVisibleTags = 0;
		for ( i = 0; i < uNumTags; i++)
		{
			uNumVisibleTags += !!(pVisCheckMem->m_uSmallData & (1<<(8+i)));
		}

		f32 fFocus = pVisCheckMem->m_fMediumData;
		if (uNumVisibleTags > 0)
		{
			//Focus
			CBot* pBot = (CBot*) pPlayerEntity;
			f32 fFocusTimePerfectScore		= 0.5f;
			f32 fYPeripheralScore			= 1.0f;

			f32 fMotionWeight				= 0.100f;
			f32 fSizeWeight					= 0.425f;
			f32 fXZPeripheralWeight			= 0.325f;
			f32 fVisRayWeight				= 0.100f;  //Is the player up against a wall or anything
			f32 fBrightnessWeight			= 0.050f;
			//
			//Factor the movement of the obj (pgm: this could be made to be relative movement ie, left-right is harder to track that front-back)
			//
			f32 fMotionScore = 0.0f;
			if (pBot->m_fMaxFlatSurfaceSpeed_WS > 0.0f)
			{
				fMotionScore = pBot->m_fSpeed_WS / pBot->m_fMaxFlatSurfaceSpeed_WS;
			}
			FMATH_CLAMP(fMotionScore, 0.0f, 1.0f);

			//
			// size of obj relative to how far away and amount of obj that is occluded
			//
			f32 fVisibleRad = pBot->m_fCollCylinderHeight_WS*2.0f*pBot->m_fCollCylinderRadius_WS;	//Area of collision Cylinder
			f32 fPctOfTagsVisible = (f32)uNumVisibleTags / uNumTags;
			fVisibleRad *= 0.65f+0.35f*fPctOfTagsVisible;	 //scale the visible Rad by the amount of the body that is actually visible
			f32 fFreeFeet = 60.0f;
			f32 fRadScale = ((fScanDist-fFreeFeet) - (fDistToObj-fFreeFeet))/(fScanDist-fFreeFeet);
			FMATH_CLAMP(fRadScale, 0.0f, 1.0f);
			fVisibleRad *= fRadScale;

			f32 fSizeScore = fVisibleRad * (1.0f/16.0f);	   //10 is just a measuring stick used to balance the perspective scaling.  (but happens to be blinks collcyl area)
			FMATH_CLAMP(fSizeScore, 0.0f, 1.0f);
			
			//
			// Peripheral vision
			//

			// Horizontal
			CFVec3A DeltaToTargXZ = *pPlayerEntity->GetTagPoint(0);
			DeltaToTargXZ.Sub(pThisBrain->GetAIMover()->GetEyePos());
			DeltaToTargXZ.y = 0.0f;
			f32 fXZMag = DeltaToTargXZ.SafeUnitAndMag(DeltaToTargXZ);
			CFVec3A EyeLookXZ = pThisBrain->GetAIMover()->GetEyeLookAt();
			EyeLookXZ.y = 0.0f;
			EyeLookXZ.SafeUnitAndMag(EyeLookXZ);
			f32 fXZPeripheralDot = 1.0f;
			if (fXZMag > 0.0f)
			{
				fXZPeripheralDot = DeltaToTargXZ.Dot(EyeLookXZ);
			}
			if (fXZPeripheralDot > 0.85f)  // cos (30 degrees)
			{
				fXZPeripheralDot  = 1.0f;
			}
			f32 fXZPeriferalDistanceDampening = fSizeScore;
			f32 fXZPeripheralScore = (fXZPeripheralDot*fXZPeripheralDot*fXZPeripheralDot)*fXZPeriferalDistanceDampening;
			FMATH_CLAMP(fXZPeripheralScore, 0.0f, 1.0f);

			//Vertical
			f32 fCurPitch = 0.0f;
			f32 fGazeY = pThisBrain->GetAIMover()->GetEyeLookAt().y;
			FMATH_CLAMP(fGazeY, -1.0f, 1.0f);  //because safeunitandmag
			fCurPitch = fmath_Atan( fmath_Sqrt( 1.0f - fGazeY*fGazeY ), fGazeY ) - FMATH_HALF_PI;


			f32 fPitchToDest = 0.0f;
			CFVec3A DeltaToDestUnit;
			DeltaToDestUnit.Sub(*(pPlayerEntity->GetTagPoint(0)), pThisBrain->GetAIMover()->GetEyePos());
			f32 fDist = DeltaToDestUnit.SafeUnitAndMag(DeltaToDestUnit);
			if (fDist > FMATH_POS_EPSILON)
			{
				FMATH_CLAMP(DeltaToDestUnit.y, -1.0f, 1.0f);  //because safeunitandmag
				fPitchToDest = fmath_Atan( fmath_Sqrt( 1.0f - DeltaToDestUnit.y*DeltaToDestUnit.y ), DeltaToDestUnit.y ) - FMATH_HALF_PI;
			}
			else
			{
				fPitchToDest = 0.0f;
			}

			f32 fPitchDelta = fPitchToDest - fCurPitch;
			FMATH_CLAMP(fPitchDelta, -FMATH_HALF_PI, FMATH_HALF_PI);
			f32 fPitchMinPenalty = -0.25;
			if (fPitchDelta < fPitchMinPenalty)
			{
				fYPeripheralScore = (-FMATH_HALF_PI - fPitchDelta) / (-FMATH_HALF_PI - fPitchMinPenalty);
				fYPeripheralScore*=fYPeripheralScore;
			}
			else
			{
				fYPeripheralScore  = 1.0f;
			}

			//
			// Out in the open Score
			//	   based on 8 cached los tests from the players #0 tag point
			f32 fVisRayScore = 0.0f;
			u8* puaVisRays = aigroup_GetVisRaysAt(pPlayerEntity);
			if (puaVisRays)
			{
				FASSERT(puaVisRays);
				if (!CGenericWait::s_bVisRaysInitialized)
				{
					CGenericWait::InitializeVisRays();
				}

				f32 fBestDot = -1.0f;
				u8 uBestRay = 0;
				u8 uPosDotCounter = 0;
				for (i = 0; i < CGenericWait::NUM_VISRAYS && (uPosDotCounter < (CGenericWait::NUM_VISRAYS>>1)+1); i++)
				{
					f32 fRayDot = CGenericWait::s_aVisRay[i].Dot(DeltaToTargXZ);
					if (fRayDot > fBestDot)
					{
						fBestDot = fRayDot;
						uBestRay = (u8)i;
					}
					if (fRayDot > 0.0f)
					{
						uPosDotCounter++;
					}
				}

				fVisRayScore = 0.4f*(puaVisRays[uBestRay]!=CGenericWait::VISRAY_FAILED);
				fVisRayScore += 0.2f* (puaVisRays[(uBestRay+1)&0x7]!=CGenericWait::VISRAY_FAILED);
				fVisRayScore += 0.2f* (puaVisRays[(uBestRay+2)&0x7]!=CGenericWait::VISRAY_FAILED);
				fVisRayScore += 0.1f* (puaVisRays[(uBestRay+CGenericWait::NUM_VISRAYS-1)&0x7]!=CGenericWait::VISRAY_FAILED);
				fVisRayScore += 0.1f* (puaVisRays[(uBestRay+CGenericWait::NUM_VISRAYS-2)&0x7]!=CGenericWait::VISRAY_FAILED);
				FMATH_CLAMP(fVisRayScore, 0.0f, 1.0f);
			}
			else
			{
				fVisRayScore = 1.0f;
			}
			

			f32 fBrightnessScore = aigroup_GetLightAt(pPlayerEntity);
			FMATH_CLAMP(fBrightnessScore, 0.0f, 1.0f);

			//
			// Sum all factors
			//
			f32 fFocusScoreThisFrame = fMotionWeight*fMotionScore +
										fSizeWeight*fSizeScore +
										fXZPeripheralWeight*fXZPeripheralScore +
										fVisRayWeight*fVisRayScore +
										fBrightnessWeight*fBrightnessScore;
			
			fFocusScoreThisFrame*=fYPeripheralScore; //Scales whole formula

			fFocusScoreThisFrame*=1.15f;	//flatten the top of the whole function, so that not every factors must be perfect for a perfect score
			FMATH_CLAMP(fFocusScoreThisFrame, 0.0f, 1.0f);

			if (fFocusScoreThisFrame > 0.0f)
			{
				fDebugDisplayF32 = fYPeripheralScore;//fFocusScoreThisFrame;//fFocusTimePerfectScore/fFocusScoreThisFrame;
				fFocus += FLoop_fPreviousLoopSecs/fFocusTimePerfectScore*fFocusScoreThisFrame;
			}
			else
			{
				fDebugDisplayF32 = 0.0f;
				fFocus += 0.0f;
			}
			FMATH_CLAMP(fFocus, 0.0f, 1.0f);
		}
		else
		{
			fDebugDisplayF32 = -1.0f;
			fFocus = 0.0f;
		}
		pVisCheckMem->m_fMediumData = fFocus;
	}

	return AIBRAINMAN_CONTINUE_ITERATION;
}


void CAIBrain::DoPerceptionWork(void)
{
	CAIMover* pMover = GetAIMover();
	u8 uPerceptorsCtrl = GetAttrib(ATTRIB_PERCEPTORCTRL);
	BOOL bIsPlayerBrain = (pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT) && ( ((CBot*) pMover->GetEntity())->m_nPossessionPlayerIndex != -1);


	if (!(uPerceptorsCtrl & (PCFLAG_EARS_ON | PCFLAG_EYES_ON)))
	{
		return;  //perception turned completely off
	}

	//sleeping bots have closed Eyes
	if (GetAIMover()->IsSleeping() ||										
		(!GetAIMover()->CanSee() && this->GetInfreqCycleRandom() < 0.3f))	//some bots that can't see. And even those can still see from time to time.
	{
		uPerceptorsCtrl &= ~PCFLAG_EYES_ON;
	}
//	uPerceptorsCtrl &= ~PCFLAG_EYES_ON;
//	uPerceptorsCtrl &= ~PCFLAG_EARS_ON;

	//
	// Ears!
	//
	if (uPerceptorsCtrl & PCFLAG_EARS_ON && IsHearingLODActive())
	{
		_SoundCollisionCBInit();
		AIEnviro_DetectSoundCollisions( m_pMover->GetEarLoc(),					//Ear Loc
										m_pMover->GetEntity(),					//Listener
										GetHearingMag(),						//Scale Sound Radii
										_SoundCollisionCB,						//CB Func
										(void*) this);							//CB param
	}

	//
	// Eyes!
	//
	if (uPerceptorsCtrl & PCFLAG_EYES_ON)
	{
		if ((((u32) GetGUID() & 1) == (FVid_nFrameCounter & 1)))
		{  
			//only look for Projectiles and Other bots every other frame
			
			//Look for projectils that we can see
			_BeginVisibleEntityScan();
			AIEnviro_IterateAllAIVisibleEntities(CAIBrain::_CanSeeVisibleEntity_CB, this);
			_EndVisibleEntityScan(this);
		

			//look for any other NPC brains
			aibrainman_IterateActiveBrains(CAIBrain::_CanSeeNPCBrain_CB, this);
		}
		else
		{
			//go through all seen memories entities and update
			//as if we had actually done the perception and received 
			//identical results as last frame

			//seen any entities besides player bots
			CNiIterator<CAIMemoryLarge*> it;
			CEntity* pOtherEntity = NULL;
			it.Reset();
			CAIMemoryLarge* pSeenMem = NULL;
			while (	pSeenMem = GetKnowledge().NextMemory(MEMORY_LARGE_SEEN, &it))
			{
				pOtherEntity = pSeenMem->m_pEntity;
				if (pOtherEntity &&
					pOtherEntity->IsInWorld() &&
					(!(pOtherEntity->TypeBits() & ENTITY_BIT_BOT) || !((CBot*) pOtherEntity)->IsDeadOrDying()) &&
					!aiutils_IsPlayer(pOtherEntity))
				{
					if (pSeenMem->m_uControlFlags & pSeenMem->CONTROL_FLAG_NEW_MEMORY)
					{
						pSeenMem->ResetTimer(NPC_BOT_SEENMEM_TIME);
						pSeenMem->m_Loc = GetLoc().v3;
						pSeenMem->m_EntityLoc = pOtherEntity->MtxToWorld()->m_vPos.v3;
					}
				}
			}


		}

		//now look for other players
		if (!bIsPlayerBrain)
		{
			for (s32 i = 0; i < CPlayer::m_nPlayerCount; i++)
			{
				if (Player_aPlayer[i].m_pEntityCurrent &&
					Player_aPlayer[i].m_pEntityCurrent->IsInWorld() &&
					!((CBot*)Player_aPlayer[i].m_pEntityCurrent)->IsDeadOrDying())
				{
					CEntity* pPlayerBot = Player_aPlayer[i].m_pEntityCurrent;

					//very special note
					//this if statement controls who bots see as their enemy when glitch jumps
					//into a mech.
					//for all mechs, except the pillbox (in which glitch is exposed)
					//the enemy will become the mech.
					if (((CBot*) Player_aPlayer[i].m_pEntityCurrent)->GetCurMech() &&
						!(((CBot*) Player_aPlayer[i].m_pEntityCurrent)->GetCurMech()->TypeBits() & ENTITY_BIT_SITEWEAPON))	// a ver
					{
						pPlayerBot = ((CBot*) Player_aPlayer[i].m_pEntityCurrent)->GetCurMech();
					}
					CAIBrain::_CanSeePlayer(this, pPlayerBot);
				}
			}
		}

	}
}


void CAIBrain::NotifyActionEnable(CEntity* pEntity)
{
	CAIMemorySmall* pMem;
	if (GetKnowledge().CanRememberAny(MEMORY_SMALL_PLAYERPROMPTED, &pMem))
	{
		GetKnowledge().ForgetThis(pMem);
	}
	u16 nPlayerIndex = 0;
	if (aiutils_IsPlayer(pEntity, &nPlayerIndex))
	{
		pMem = GetKnowledge().RememberSmall(MEMORY_SMALL_PLAYERPROMPTED, 5);
		if (pMem)
		{
			pMem->m_uSmallData = nPlayerIndex;
		}
	}
}

void CAIBrain::NotifyDamaged(const CDamageResult *pDamageResult)
{
	FASSERT(pDamageResult);
	u8 uPerceptorsCtrl = GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL);
	if (uPerceptorsCtrl & CAIBrain::PCFLAG_TOUCH_ON)
	{
		CAIMemoryLarge* pDamagedByMem = NULL;
		if (GetKnowledge().CanRememberEntity(MEMORY_LARGE_DAMAGED_BY, pDamageResult->m_pDamageData->m_Damager.pBot, &pDamagedByMem))
		{
			pDamagedByMem->ResetTimer(pDamagedByMem->m_uHowLongSecs);
		}
		else
		{
			pDamagedByMem = GetKnowledge().RememberLarge(MEMORY_LARGE_DAMAGED_BY, 4);
			pDamagedByMem->m_uMediumData = 0;							//how much damage this memory?
			pDamagedByMem->m_fMediumData = aiutils_GetCurTimeSecs();		//Time at which damage was first taken
		}

		if (pDamagedByMem)
		{
			//update my memory about damaging this guy
			pDamagedByMem->m_pEntity = pDamageResult->m_pDamageData->m_Damager.pBot;

			//If Damage is ambient, then m_uSmallData = TRUE
			pDamagedByMem->m_uSmallData = pDamageResult->m_pDamageData->m_nDamageLocale; //ambient, impact, blast
			if (pDamageResult->m_pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_AMBIENT)
			{
				pDamagedByMem->m_Loc = -pDamageResult->m_pDamageData->m_AttackUnitDir_WS.v3;
			}
			else
			{
				pDamagedByMem->m_Loc = CFVec3A::m_Ones.v3;
			}


			//specifics about what kind of damage, how much, etc

			//where did the shot Hit
			pDamagedByMem->m_EntityLoc = pDamageResult->m_pDamageData->m_ImpactPoint_WS.v3;
			
			//How much damage this frame?
			pDamagedByMem->m_fLargeData = pDamageResult->m_fDeltaHitpoints;

			//How much damage by this guy ever?  (store in a u32, so mul by 100, then round)?
			pDamagedByMem->m_uMediumData += (u32) (pDamageResult->m_fDeltaHitpoints*s_fDamagedByTotalDamageMultiplier);
		}
	}
}


void CAIBrain::NotifyDamageDealt(const CDamageResult *pDamageResult)
{
	FASSERT(pDamageResult);
	CAIMemoryLarge* pDamageDealtMem = NULL;
	if (GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGE_DEALT, (CAIMemorySmall**) &pDamageDealtMem))
	{
		//update my memory about damaging this guy
		pDamageDealtMem->ResetTimer(pDamageDealtMem->m_uHowLongSecs);
	}
	else
	{
		pDamageDealtMem = GetKnowledge().RememberLarge(MEMORY_LARGE_DAMAGE_DEALT, 5);
	}

	if( pDamageDealtMem )
	{
		pDamageDealtMem->m_pEntity = pDamageResult->m_pDamageData->m_Damager.pBot;
		pDamageDealtMem->m_Loc = GetLoc().v3;
		
		//where did the shot probably come from?
		pDamageDealtMem->m_EntityLoc = pDamageResult->m_pDamageData->m_ImpactPoint_WS.v3;

		pDamageDealtMem->m_fLargeData = pDamageResult->m_fDeltaHitpoints;
	}
}


void CAIBrain::NotifyRammedBy(CEntity* pDamager)
{
	if (pDamager)
	{
		u8 uPerceptorsCtrl = GetAttrib(CAIBrain::ATTRIB_PERCEPTORCTRL);
		if (uPerceptorsCtrl & CAIBrain::PCFLAG_TOUCH_ON)
		{
			CAIMemoryLarge* pMem;
			if (GetKnowledge().CanRememberAny(MEMORY_LARGE_COLLIDED, (CAIMemorySmall**) &pMem))
			{
				//update my memory about getting bumped into
				pMem->ResetTimer(pMem->m_uHowLongSecs);
				pMem->m_pEntity = pDamager;
				pMem->m_Loc = GetLoc().v3;
				pMem->m_EntityLoc = pDamager->MtxToWorld()->m_vPos.v3;
				if (pDamager->AIBrain())
				{
					pMem->m_EntityLoc.y += pDamager->AIBrain()->GetAIMover()->GetHeight();
				}
			}
			else
			{
				if (pMem = GetKnowledge().RememberLarge(MEMORY_LARGE_COLLIDED, 2))
				{
					//specifics about about getting bumped into
					pMem->m_pEntity= pDamager;   //who did it
					pMem->m_EntityLoc = pDamager->MtxToWorld()->m_vPos.v3;
					pMem->m_Loc = GetLoc().v3;
					if (pDamager->AIBrain())
					{
						pMem->m_EntityLoc.y += pDamager->AIBrain()->GetAIMover()->GetHeight();
					}
				}
			}
		}
	}
}

