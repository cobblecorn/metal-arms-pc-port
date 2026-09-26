//////////////////////////////////////////////////////////////////////////////////////
// LaserBeam.cpp - Laser beam system for Mettle Arms.
//
// Author: Justin Link
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
// 06/13/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "LaserBeam.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fdraw.h"
#include "fvtxpool.h"
#include "meshtypes.h"
#include "floop.h"
#include "fcamera.h"
#include "fresload.h"
#include "gamecam.h"
#include "potmark.h"
#include "bot.h"
#include "damage.h"
#include "weapon.h"
#include "player.h"



#define _LASERBEAM_DECAL_NAME		( "LaserL3" )

BOOL CLaserBeam::m_bSystemInitted = FALSE;
CLaserBeam *CLaserBeam::m_pLastCreated = NULL;
//cchar *CLaserBeam::m_pszTexName = "circ_2";
cchar *CLaserBeam::m_pszTexName = "tfdmlaser01";//"circ_2_a";
cchar *CLaserBeam::m_pszMuzzleFlash = "tfdmlaser02";
CFTexInst CLaserBeam::m_texBeam, CLaserBeam::m_texMuzzleFlash;
FDecalDefHandle_t CLaserBeam::m_hDecalDef = FDECALDEF_INVALID_HANDLE;



// Static function.
BOOL CLaserBeam::InitSystem()
{
	m_bSystemInitted = TRUE;

	m_texBeam.SetTexDef((FTexDef_t *)(fresload_Load(FTEX_RESNAME, m_pszTexName)));
	m_texMuzzleFlash.SetTexDef((FTexDef_t *)(fresload_Load(FTEX_RESNAME, m_pszMuzzleFlash)));

	//////////////////////////////////////////////////////////////////////
	// Set the texture to wrap.
	u32 uTexFlags;
	uTexFlags = m_texBeam.GetFlags();
	uTexFlags |= (CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T | CFTexInst::FLAG_WRAP_U);
	m_texBeam.SetFlags(uTexFlags);
	//
	//////////////////////////////////////////////////////////////////////

	m_hDecalDef = CFDecal::GetDefByName( _LASERBEAM_DECAL_NAME );
	if( m_hDecalDef == FDECALDEF_INVALID_HANDLE ) {
		DEVPRINTF( "CLaserBeam::InitSystem(): Unable to get decal handle\n" );
	}

	m_pLastCreated = NULL;

	return(TRUE);
}



// Static function.
void CLaserBeam::UninitSystem()
{
	m_pLastCreated = NULL;

	m_bSystemInitted = FALSE;
}



// Static function.
BOOL CLaserBeam::InitLevel()
{
	return(TRUE);
}



// Static function.
void CLaserBeam::UninitLevel()
{
#if 0
	//////////////////////////////////////////////////////////////////////
	// Step through our list and delete any laser beams that we might have
	//   given out.
	CLaserBeam *pPrev;
	CLaserBeam *pCur = m_pLastCreated;

	while(pCur != NULL)
	{
		pPrev = pCur->m_pPrev;
		fdelete(pCur);
		pCur = pPrev;
	}
#endif
	//
	//////////////////////////////////////////////////////////////////////

	m_pLastCreated = NULL;
}



// Static function.
void CLaserBeam::Work()
{
	//////////////////////////////////////////////////////////////////////
	// Step through our list and call the draw functions on each laser beam.
	CLaserBeam *pCur = m_pLastCreated;

	while(pCur != NULL)
	{
		pCur->_Work();
		pCur = pCur->m_pPrev;
	}
	//
	//////////////////////////////////////////////////////////////////////
}



// Static function.
void CLaserBeam::Draw()
{
//	frenderer_Push(FRENDERER_DRAW, NULL);

		fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
		fdraw_Depth_EnableWriting(FALSE);

		//////////////////////////////////////////////////////////////////////
		// Step through our list and call the draw functions on each laser beam.
		CLaserBeam *pCur = m_pLastCreated;

		while(pCur != NULL)
		{
			pCur->_Draw();
			pCur = pCur->m_pPrev;
		}
		//
		//////////////////////////////////////////////////////////////////////

//	frenderer_Pop();
}



// Static function.
CLaserBeam *CLaserBeam::GetLaserBeam( void )
{
	CLaserBeam *pLB = fnew CLaserBeam;
	if(pLB != NULL)
	{
		pLB->m_pPrev = m_pLastCreated;
		m_pLastCreated = pLB;
		pLB->m_nLoopTickSnapshotWhenLastDamageIssued = 0;
	}
	else
	{
		DEVPRINTF("CLaserBeam::GetLaserBeam() : Could not create any more laser beams.\n");
	}

	return(pLB);
}



// 
void CLaserBeam::SetStartPosAndDir(const CFVec3A &vecPos, const CFVec3A &vecUnitDir, CWeapon *pOwnerWeapon, const CDamageProfile *pDamageProfile)
{
	//////////////////////////////////////////////////////////////////////
	// Check to see if we are updating based on an old position or not.
//	if((m_uFlags & LASERBEAMFLAG_DRAWNLASTFRAME) != 0)
//	{
//		m_vecEndPosVel.Set(0.0f, 0.0f, 0.0f);
//	}
//	else
	{
		// This is where our laser would hit in an ideal world (we might not hit here due to something
		//   being in the way or due to the 'physics' on the laser beam).
		m_vecEndPosIdeal = vecUnitDir;
		m_vecEndPosIdeal.Mul(200.0f);
		m_vecEndPosIdeal.Add(vecPos);

		// Pretend like we didn't hit anybody.  We'll fix that it we did.
		m_pImpactEntity = NULL;
		m_bHitSomething = FALSE;

		FWorld_nTrackerSkipListCount = 0;
		if(fworld_FindClosestImpactPointToRayStart(&m_Impact, &vecPos, &m_vecEndPosIdeal, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList))
		{
			m_bHitSomething = TRUE;

			// We will adjust the ideal end point due to a collision with something.
			m_vecEndPosIdeal.Set(m_Impact.ImpactPoint);

			m_pImpactWorldMesh = (CFWorldMesh *)(m_Impact.pTag);
			// See if the world mesh that we found is an entity...
			if((m_pImpactWorldMesh != NULL) && (m_pImpactWorldMesh->m_nUser == MESHTYPES_ENTITY))
			{
				m_pImpactEntity = (CEntity *)(m_pImpactWorldMesh->m_pUser);
			}
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	m_vecStartPos = vecPos;
	m_vecStartUnitDir = vecUnitDir;

	m_pOwnerWeapon = pOwnerWeapon;
	m_pDamageProfile = pDamageProfile;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Mark that we do want to be drawn next frame.
	FMATH_SETBITMASK(m_uFlags, LASERBEAMFLAG_DRAWNEXTFRAME);
	//
	//////////////////////////////////////////////////////////////////////
}



CLaserBeam::CLaserBeam()
{
	m_uFlags = 0;
	m_pImpactEntity = NULL;
	m_fTexScroll = 0.0f;
}



CLaserBeam::~CLaserBeam()
{
}



void CLaserBeam::_Work()
{
	if((m_uFlags & LASERBEAMFLAG_DRAWNEXTFRAME) == 0)
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////
	// Find the real position that we should be shooting at.
	if((m_uFlags & LASERBEAMFLAG_DRAWNLASTFRAME) == 0)
	{
		// They weren't firing last frame, so we'll start off where we're pointing.
		m_vecEndPosVel.Set(0.0f, 0.0f, 0.0f);
		m_vecEndPosLast = m_vecEndPosIdeal;
	}
	else
	{
		CFVec3A vecOldBeam = m_vecStartPos;
		vecOldBeam.Sub(m_vecEndPosLast);

		CFVec3A vecNewBeam = m_vecStartPos;
		vecNewBeam.Sub(m_vecEndPosIdeal);

		f32 fOldBeamLen = vecOldBeam.Mag();
		f32 fNewBeamLen = vecNewBeam.Mag();
		f32 fDelBeamLen =  fOldBeamLen - fNewBeamLen;

//		if(FMATH_FABS(fDelBeamLen) <= 50.0f)
		if(FMATH_FABS((fOldBeamLen - fNewBeamLen) / (fOldBeamLen)) < 0.2f)
		{
			CFVec3A vecDelEndPos = m_vecEndPosIdeal;
			vecDelEndPos.Sub(m_vecEndPosLast);

			CFVec3A vecEndPosAcc;
			vecEndPosAcc = vecDelEndPos;
			vecEndPosAcc.Mul(300.0f);

			CFVec3A vecDamp;
			vecDamp = m_vecEndPosVel;
			vecDamp.Mul(-15.0f);

			vecEndPosAcc.Add(vecDamp);

			// Integrate the acceleration to get the change in velocity.
			vecEndPosAcc.Mul(FLoop_fPreviousLoopSecs);
			m_vecEndPosVel.Add(vecEndPosAcc);

			// Integrate the velocity to get the change in position.
			CFVec3A vecDisplace = m_vecEndPosVel;
			vecDisplace.Mul(FLoop_fPreviousLoopSecs);
			m_vecEndPosLast.Add(vecDisplace);
		}
		else
		{
//			DEVPRINTF("Too much movement, cutting short.\n");
			m_vecEndPosVel.Set(0.0f, 0.0f, 0.0f);
			m_vecEndPosLast = m_vecEndPosIdeal;
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	if(m_bHitSomething)
	{
		CFDecal::Create( m_hDecalDef, &m_Impact );
		//if((m_pImpactEntity == NULL) || ((m_pImpactEntity->TypeBits() & ENTITY_BIT_BOT) == 0))
		//{
		//	potmark_NewPotmark(&m_vecEndPosLast, &m_Impact.UnitFaceNormal, 1.0f, POTMARKTYPE_LASER2);
		//}

		if((FLoop_nTotalLoopTicks & 3) == 2)
		{
//			fpart_SpawnParticleEmitter(FPART_TYPE_SPARK_BURST, 0.2f, m_Impact.ImpactPoint, &m_Impact.UnitFaceNormal, TRUE );
//			fpart_SpawnParticleEmitter(FPART_TYPE_SMOKE_PUFF, 0.2f, m_Impact.ImpactPoint, &m_Impact.UnitFaceNormal);
		}

		if( (FLoop_nTotalLoopTicks >= (m_nLoopTickSnapshotWhenLastDamageIssued + (u64)(m_pDamageProfile->m_fDamagePeriod * FLoop_fTicksPerSec)) )
			|| (FLoop_nTotalLoopTicks < m_nLoopTickSnapshotWhenLastDamageIssued) )
		{
			m_nLoopTickSnapshotWhenLastDamageIssued = FLoop_nTotalLoopTicks;

			if(m_pImpactEntity != NULL)
			{
				// Get an empty damage form...
				CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

				if( pDamageForm )
				{
					// Fill out the form...

					pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
					pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
					pDamageForm->m_pDamageProfile = m_pDamageProfile;
					pDamageForm->m_Damager.pWeapon = m_pOwnerWeapon;
					pDamageForm->m_Damager.pBot = m_pOwnerWeapon->GetOwner();
					pDamageForm->m_Damager.nDamagerPlayerIndex = m_pOwnerWeapon->GetOwner() ? m_pOwnerWeapon->GetOwner()->m_nPossessionPlayerIndex : -1;
					pDamageForm->m_pDamageeEntity = m_pImpactEntity;
					pDamageForm->InitTriDataFromCollImpact( m_pImpactWorldMesh, &m_Impact, &m_vecStartUnitDir );

					CDamage::SubmitDamageForm( pDamageForm );
				}
			}
		}
	}

#if 0
		if(m_pImpactEntity != NULL)
		{
			CDamageInfo oDamageInfo;
			oDamageInfo.m_eDamagerClass = DAMAGERCLASS_LASER;
			oDamageInfo.m_fUnitBulletDamage = 0.0f;
			oDamageInfo.m_fUnitExplosionDamage = 0.0f;
			oDamageInfo.m_fUnitFlameDamage = 0.10f;
			oDamageInfo.m_pEDamagee = m_pImpactEntity;
			oDamageInfo.m_pEDamager = NULL;					// This may get set to something, but it doesn't have to.
			oDamageInfo.m_pEDamagerOwner = m_pOwner;
			oDamageInfo.m_vecDir_WS.Set(0.0f, 1.0f, 0.0f);	// TODO: This should get set to be something.
			oDamageInfo.m_vecNormal_WS.Set(m_Impact.UnitFaceNormal);
			oDamageInfo.m_vecPos_WS.Set(m_Impact.ImpactPoint);
			oDamageInfo.m_nBoneIdx = m_Impact.nBoneIndex;

			CDamageSystem::SubmitDamageRequest(&oDamageInfo);
		}
#endif
}



void CLaserBeam::_Eval(CFVec3A *avecCoeff, f32 fInput, CFVec3A &vecOut)
{
	CFVec3A vecTemp;

	vecOut = avecCoeff[0];

	vecTemp = avecCoeff[1];
	vecTemp.Mul(fInput);

	vecOut.Add(vecTemp);

	vecTemp = avecCoeff[2];
	vecTemp.Mul(fInput);
	vecTemp.Mul(fInput);

	vecOut.Add(vecTemp);

	vecTemp = avecCoeff[3];
	vecTemp.Mul(fInput);
	vecTemp.Mul(fInput);
	vecTemp.Mul(fInput);

	vecOut.Add(vecTemp);
}



void CLaserBeam::_Draw()
{
	if((m_uFlags & LASERBEAMFLAG_DRAWNEXTFRAME) == 0)
	{
		m_uFlags &= ~(LASERBEAMFLAG_DRAWNLASTFRAME);
		return;
	}
/*
	CFXfm xfmSphere;
	CFColorRGBA rgbaSphere;
	rgbaSphere.OpaqueRed();

	xfmSphere.BuildTranslation(m_vecEndPosLast.v3);
	xfmSphere.PushModel();
	fdraw_FacetedWireSphere(&CFVec3::m_Null, 1.0f, 2, 2, &rgbaSphere);
	CFXfm::PopModel();

	xfmSphere.BuildTranslation(m_vecEndPosIdeal.v3);
	xfmSphere.PushModel();
	fdraw_FacetedWireSphere(&CFVec3::m_Null, 1.0f, 2, 2, &rgbaSphere);
	CFXfm::PopModel();
*/

	//////////////////////////////////////////////////////////////////////
	// Calculate the important distances.
	f32 fDistTotal, fOODistTotal, fOODistTotalSq, fOODistTotalCu;
//	CFVec3A vecStartMinusEnd = m_vecStartPos;
//	vecStartMinusEnd.Sub(m_vecEndPosLast);
	CFVec3A vecEndMinusStart = m_vecEndPosLast;
	vecEndMinusStart.Sub(m_vecStartPos);
//	fDistTotal = vecStartMinusEnd.Mag();
	fDistTotal = vecEndMinusStart.Mag();

	// They would have to be moving *really* fast for this to FASSERT.
	FASSERT(fDistTotal <= 500.0f);

	fOODistTotal = 1.0f / fDistTotal;
	fOODistTotalSq = fOODistTotal * fOODistTotal;
	fOODistTotalCu = fOODistTotalSq * fOODistTotal;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Calculate our coefficients.
	CFVec3A avecCoeff[4];

	// These coefficients are for a cubic beam.
	avecCoeff[0] = m_vecStartPos;
	avecCoeff[1] = m_vecStartUnitDir;

	CFVec3A vecDiffPos = m_vecEndPosLast;
	vecDiffPos.Sub(m_vecStartPos);

//	CFVec3A vecUnitStartMinusEnd;
//	vecUnitStartMinusEnd = vecStartMinusEnd;
//	vecUnitStartMinusEnd.Mul(fOODistTotal);
	CFVec3A vecUnitEndMinusStart;
	vecUnitEndMinusStart = vecEndMinusStart;
	vecUnitEndMinusStart.Mul(fOODistTotal);

	CFVec3A vecEndDir;
//	vecEndDir.ReceiveReflection(m_vecStartUnitDir, vecUnitStartMinusEnd);
	vecEndDir.ReceiveReflection(m_vecStartUnitDir, vecUnitEndMinusStart);
	FASSERT(FMATH_FABS(m_vecStartUnitDir.MagSq() - vecEndDir.MagSq()) < 0.01f);

	CFVec3A vecSumDir1 = m_vecStartUnitDir;
	vecSumDir1.Add(vecEndDir);

	CFVec3A vecSumDir2 = m_vecStartUnitDir;
	vecSumDir2.Mul(2.0f);
	vecSumDir2.Add(vecEndDir);

	CFVec3A vecTemp;

	avecCoeff[2] = vecDiffPos;
	avecCoeff[2].Mul(fOODistTotalSq * 3.0f);
	vecTemp = vecSumDir2;
	vecTemp.Mul(fOODistTotal);
	avecCoeff[2].Sub(vecTemp);

	avecCoeff[3] = vecSumDir1;
	avecCoeff[3].Mul(fOODistTotalSq);
	vecTemp = vecDiffPos;
	vecTemp.Mul(fOODistTotalCu * 2.0f);
	avecCoeff[3].Sub(vecTemp);

/*
	// These coefficients are for a quadratic beam.
	avecCoeff[0] = m_vecStartPos;
	avecCoeff[1] = m_vecStartUnitDir;
	avecCoeff[2] = m_vecStartUnitDir;
	avecCoeff[2].Mul(-fDistTotal);
	avecCoeff[2].Add(m_vecEndPosLast);
	avecCoeff[2].Sub(m_vecStartPos);
	avecCoeff[2].Mul(fOODistTotalSq);
	avecCoeff[3].Set(0.0f, 0.0f, 0.0f);*/

	// These coefficients are for a straight beam.
/*	avecCoeff[0] = m_vecStartPos;
	avecCoeff[1] = m_vecEndPosLast;
	avecCoeff[1].Sub(m_vecStartPos);
	avecCoeff[1].Unitize();
	avecCoeff[2].Set(0.0f, 0.0f, 0.0f);
	avecCoeff[3].Set(0.0f, 0.0f, 0.0f)*/
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Get a 'frame of reference' for the beam (basically we need an up
	//   and a right vector).
	CFCamera *pCamera = gamecam_GetActiveCamera();
	FASSERT(pCamera != NULL);
	CFMtx43A mtxBeam = pCamera->GetFinalXfm()->m_MtxR;
	CFVec3A vecPos = m_vecStartPos;
	CFVec3A vecFront = mtxBeam.m_vFront;
	CFVec3A vecRight = mtxBeam.m_vRight;
	CFVec3A vecUp = mtxBeam.m_vUp;

	CFVec3A vecOfs = vecRight;
	vecOfs.Mul(0.80f/*0.28f*/);

	CFVec3A vecJitter;
	vecJitter.Zero();
	//
	//////////////////////////////////////////////////////////////////////

	FDrawVtx_t *pVtxArray = fvtxpool_GetArray(800);

	//////////////////////////////////////////////////////////////////////
	// Calculate the coordinates for and draw the beam.

	// Holds the start and end positions for the current line segment.
	CFVec3A avecPt[2];

	f32 fStepSize;
//	if(fDistTotal <= 25.0f)
//	{
		fStepSize = fDistTotal * (1.0f / 20.0f);
//	}
//	else
//	{
//		fStepSize = 10.0f;
//	}

	u32 uCurVtxArryIdx;

	f32 fCurPos = 0.0f;
	_Eval(avecCoeff, 0.0f, avecPt[0]);

	m_fTexScroll += FLoop_fPreviousLoopSecs * 10.0f;
	if(m_fTexScroll >= 1.0f)
	{
		m_fTexScroll -= 1.0f;
	}

	f32 fTStepSize = fStepSize * (1.0f / 10.0f);
	f32 afT[2];
	afT[0] = fTStepSize + m_fTexScroll;
	afT[1] = m_fTexScroll;
	//
	//////////////////////////////////////////////////////////////////////

	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);
	fdraw_SetTexture(&m_texBeam);

	//////////////////////////////////////////////////////////////////////
	// Draw the beam.
	BOOL bCollided = FALSE;
	uCurVtxArryIdx = 0;
	while((fCurPos <= fDistTotal) && (!bCollided))
	{
		fCurPos += fStepSize;
		_Eval(avecCoeff, fCurPos, avecPt[1]);
		// JUSTIN: Check for collision.

		// Lower right.
		pVtxArray[uCurVtxArryIdx].ColorRGBA.OpaqueWhite();
		pVtxArray[uCurVtxArryIdx].Pos_MS = avecPt[0].v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecOfs.v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecJitter.v3;
//		pVtxArray[uCurVtxArryIdx++].ST.Set(1.0f, 1.0f + m_fTexScroll);
		pVtxArray[uCurVtxArryIdx++].ST.Set(1.0f, afT[0]);

		// Lower left.
		pVtxArray[uCurVtxArryIdx].ColorRGBA.OpaqueWhite();
		pVtxArray[uCurVtxArryIdx].Pos_MS = avecPt[0].v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS -= vecOfs.v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecJitter.v3;
//		pVtxArray[uCurVtxArryIdx++].ST.Set(0.0f, 1.0f + m_fTexScroll);
		pVtxArray[uCurVtxArryIdx++].ST.Set(0.0f, afT[0]);

		// Upper right.
		pVtxArray[uCurVtxArryIdx].ColorRGBA.OpaqueWhite();
		pVtxArray[uCurVtxArryIdx].Pos_MS = avecPt[1].v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecOfs.v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecJitter.v3;
//		pVtxArray[uCurVtxArryIdx++].ST.Set(1.0f, 0.0f + m_fTexScroll);
		pVtxArray[uCurVtxArryIdx++].ST.Set(1.0f, afT[1]);

		// Lower left.
		pVtxArray[uCurVtxArryIdx].ColorRGBA.OpaqueWhite();
		pVtxArray[uCurVtxArryIdx].Pos_MS = avecPt[0].v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS -= vecOfs.v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecJitter.v3;
//		pVtxArray[uCurVtxArryIdx++].ST.Set(0.0f, 1.0f + m_fTexScroll);
		pVtxArray[uCurVtxArryIdx++].ST.Set(0.0f, afT[0]);

		// Upper right.
		pVtxArray[uCurVtxArryIdx].ColorRGBA.OpaqueWhite();
		pVtxArray[uCurVtxArryIdx].Pos_MS = avecPt[1].v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecOfs.v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecJitter.v3;
//		pVtxArray[uCurVtxArryIdx++].ST.Set(1.0f, 0.0f + m_fTexScroll);
		pVtxArray[uCurVtxArryIdx++].ST.Set(1.0f, afT[1]);

		// Upper left.
		pVtxArray[uCurVtxArryIdx].ColorRGBA.OpaqueWhite();
		pVtxArray[uCurVtxArryIdx].Pos_MS = avecPt[1].v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS -= vecOfs.v3;
		pVtxArray[uCurVtxArryIdx].Pos_MS += vecJitter.v3;
//		pVtxArray[uCurVtxArryIdx++].ST.Set(0.0f, 0.0f + m_fTexScroll);
		pVtxArray[uCurVtxArryIdx++].ST.Set(0.0f, afT[1]);

		FASSERT(uCurVtxArryIdx < 800);

		avecPt[0] = avecPt[1];

		afT[0] -= fTStepSize;
		afT[1] -= fTStepSize;
	}
	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, pVtxArray, uCurVtxArryIdx);
	//
	//////////////////////////////////////////////////////////////////////

	fvtxpool_ReturnArray(pVtxArray);

	//////////////////////////////////////////////////////////////////////
	// Mark that we were drawn this frame, and not to draw next frame unless
	//   we hear otherwise.
	if (CPlayer::m_nCurrent == (CPlayer::m_nPlayerCount - 1)) {
		FMATH_SETBITMASK(m_uFlags, LASERBEAMFLAG_DRAWNLASTFRAME);
		FMATH_CLEARBITMASK(m_uFlags, LASERBEAMFLAG_DRAWNEXTFRAME);
	}
	//
	//////////////////////////////////////////////////////////////////////
}
