//////////////////////////////////////////////////////////////////////////////////////
// RoboBuddy.cpp - Class for Blink's robo buddy.
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
// 07/09/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "RoboBuddy.h"
#include "fworld_coll.h"

#include "meshentity.h"
#include "bot.h"
#include "meshtypes.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

static cchar *_pszMesh = "GRDFbuddy00";

static cchar *_apszBoneList[] = { "Prop" };
static cchar *_pszFireBone = "Fire_Primary";

//static const CFVec3A _afRoboPos[2] = { CFVec3A(3.0f, 3.0f, 0.7f), CFVec3A(3.0f, 3.0f, 0.7f) };
static const u32 _uRoboPosCnt = 5;
static const f32 _afRoboOfsX[_uRoboPosCnt] = { 3.0f, 3.0f, -0.5f, 3.0f, -0.25f };
static const f32 _afRoboOfsY[_uRoboPosCnt] = { 3.0f, 5.0f, 5.0f, 3.0f,  5.0f };
static const f32 _afRoboOfsZ[_uRoboPosCnt] = { 0.7f, 0.0f, 0.4f, -1.0f, -0.5f };

CFSphere *CRoboBuddy::s_poSphere;
BOOL CRoboBuddy::s_bHitMesh;
CBot *CRoboBuddy::s_pCurOwnerBot;

//===================
// private prototypes

//=================
// public functions

CRoboBuddy::CRoboBuddy()
{
	_Reset();
}



CRoboBuddy::~CRoboBuddy()
{
	Destroy();
}



BOOL CRoboBuddy::Create()
{
	FResFrame_t hResFrame = fres_GetFrame();

	//////////////////////////////////////////////////////////////////////
	// Get our mesh entity set up.
	m_pMeshEntity = fnew CMeshEntity;
	if(m_pMeshEntity == NULL)
	{
		DEVPRINTF("CRoboBuddy::Create() : Out of memory.\n");
		goto _ExitWithError;
	}

	m_pMeshEntity->Create(_pszMesh, NULL, "RoboBuddy");
	m_pMeshEntity->RemoveFromWorld(TRUE);

	m_nFireBoneIdx = m_pMeshEntity->GetMeshInst()->FindBone(_pszFireBone);
	if(m_nFireBoneIdx == -1)
	{
		DEVPRINTF("CRoboBuddy::Create() : Could not find bone '%s'.\n", _pszFireBone);
		return(FALSE);
	}

		//////////////////////////////////////////////////////////////////////
		// Get the animation set up.

		// Create a manual matrix animation object.
		m_pManMtx = fnew CFAnimManMtx;
		if(m_pManMtx == NULL)
		{
			DEVPRINTF("CRoboBuddy::Create() : Could not create CFAnimManMtx.\n");
			goto _ExitWithError;
		}
		if(!m_pManMtx->Create(1, _apszBoneList))
		{
			DEVPRINTF("CRoboBuddy::Create() : Could not Create() CFAnimManMtx.\n");
			goto _ExitWithError;
		}

		// Create the mesh rest animation object.
		m_pRestMtx = fnew CFAnimMeshRest;
		if(m_pRestMtx == NULL)
		{
			DEVPRINTF("CRoboBuddy::Create() : Could not create CFAnimMeshRest.\n");
			goto _ExitWithError;
		}
		if(!m_pRestMtx->Create(m_pMeshEntity->GetMeshInst()->m_pMesh))
		{
			DEVPRINTF("CRoboBuddy::Create() : Could not Create() CFAnimMeshRest.\n");
			goto _ExitWithError;
		}

		// Create an animation combiner object.
		m_pAnimCombiner = fnew CFAnimCombiner;
		if(m_pAnimCombiner == NULL)
		{
			DEVPRINTF("CRoboBuddy::Create() : Could not create CFAnimCombiner.\n");
			goto _ExitWithError;
		}
		m_pAnimCombiner->CreateSummer(m_pMeshEntity->GetMeshInst());
		m_pAnimCombiner->AttachToTap(0, m_pRestMtx);
		m_pAnimCombiner->AttachToTap(1, m_pManMtx);

		m_pMeshEntity->DriveMeshWithAnim(TRUE);
		m_pMeshEntity->UserAnim_Select(-1);
		m_pMeshEntity->SetExternalCombiner(0, m_pAnimCombiner);
		//
		//////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////

	return(TRUE);

_ExitWithError:
	fres_ReleaseFrame(hResFrame);
	return(FALSE);
}



void CRoboBuddy::Destroy()
{
	fdelete(m_pMeshEntity);
	fdelete(m_pAnimCombiner);
	fdelete(m_pRestMtx);
	fdelete(m_pManMtx);
	_Reset();
}



BOOL CRoboBuddy::StartDeploy()
{
	FASSERT(m_pOwnerBot != NULL);

	m_pMeshEntity->Relocate_RotXlatFromUnitMtx_WS(m_pOwnerBot->MtxToWorld());
	m_pMeshEntity->AddToWorld();

	m_pMeshEntity->SetCollisionFlag(FALSE);
	m_vecVel.Zero();
	m_fPropThetaY = 0.0f;

	m_fThetaX = 0.0f;
	m_fOmegaX = 0.0f;
	m_fThetaY = 0.0f;
	m_fOmegaY = 0.0f;

//	m_fSecsSinceLastMove = 0.0f;

	m_fUnitEffectEnd = 0.0f;

	m_eCurState = STATE_DEPLOYED;

	m_uLastHoverPos = 0;

	return(TRUE);
}



void CRoboBuddy::Work(CFVec3A &vecTargetPos)
{
	switch(m_eCurState)
	{
		case STATE_STOWED:
		{
			break;
		}
		case STATE_DEPLOYING:
		{
			break;
		}
		case STATE_DEPLOYED:
		{
			//////////////////////////////////////////////////////////////////////
			// Figure out our goal position (based on collisions with the world).
			CFVec3A vecGoal;
			u32 uCurPosCnt;
			u32 uCurPosOfs = 0;
			u32 uCurPosIdx = m_uLastHoverPos;
			s32 nFirstEmptySlot = -1;

			if((FVid_nFrameCounter & 1) == 1)
			{
				for(uCurPosCnt = 0; uCurPosCnt < _uRoboPosCnt; ++uCurPosCnt)
				{
					uCurPosIdx = uCurPosCnt + uCurPosOfs;
					if(uCurPosIdx >= _uRoboPosCnt)
					{
						uCurPosIdx -= _uRoboPosCnt;
					}

					_CalcPosFromOfs(vecGoal, uCurPosIdx);

					CFSphere oSphere;
					oSphere.m_fRadius = 1.8f;
					oSphere.m_Pos = vecGoal.v3;

					CFCollInfo oCollInfo;
					oCollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
					oCollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
					oCollInfo.bFindClosestImpactOnly = FALSE;
					oCollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
					oCollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
					oCollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
					oCollInfo.pSphere_WS = &oSphere;

					CFTrackerCollideSpheresInfo oSphereInfo;
					oSphereInfo.pCallback = _SphereCollideCallback;
					oSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
					oSphereInfo.bIgnoreCollisionFlag = FALSE;
					oSphereInfo.pMasterSphere_WS = &oSphere;
					oSphereInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
					oSphereInfo.nTrackerSkipCount = 0;
					oSphereInfo.ppTrackerSkipList = NULL;

					s_poSphere = &oSphere;
					s_bHitMesh = FALSE;
					s_pCurOwnerBot = m_pOwnerBot;

					// Let's evaluate whether or not this is a good point.
					fworld_CollideWithTrackers(&oSphereInfo);
					if(!s_bHitMesh && !fworld_CollideWithWorldTris(&oCollInfo))
					{
						// At least we're not colliding here.  Let's record that fact if it's the first one.
						if(nFirstEmptySlot == -1)
						{
							nFirstEmptySlot = (s32)(uCurPosIdx);
						}

						// Let's check if we have line of sight.
						FWorld_nTrackerSkipListCount = 0;
						m_pMeshEntity->AppendTrackerSkipList();

						FCollImpact_t oCollImpact;

						if(!fworld_FindClosestImpactPointToRayStart(&oCollImpact, &vecGoal, &vecTargetPos, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList))
						{
							// No collision was found.  There probably not aiming at anything, but that's cool.
							break;
						}
						else
						{
							// Okay, we collided with something, let's see if it was close to our target point.
							CFVec3A vecTargetMinusImpact = vecTargetPos;
							vecTargetMinusImpact.Sub( oCollImpact.ImpactPoint );
							if(vecTargetMinusImpact.MagSq() <= 9.0f)
							{
								// They're close.
								break;
							}
						}
					}
					else
					{
						int x = 3;
					}
				}
			}
			else
			{
				_CalcPosFromOfs(vecGoal, m_uLastHoverPos);
			}

			if(uCurPosIdx == _uRoboPosCnt)
			{
				// We didn't find a 'perfect' point, so let's check to see if there's at least one
				if(nFirstEmptySlot != -1)
				{
					_CalcPosFromOfs(vecGoal, (u32)(nFirstEmptySlot));
					m_uLastHoverPos = (u32)(nFirstEmptySlot);
				}
				else
				{
					_CalcPosFromOfs(vecGoal, 0);
					m_uLastHoverPos = 0;
				}
				// Disable firing here.
			}
			else
			{
				int x = 3;
				m_uLastHoverPos = uCurPosIdx;
				// vecGoal already has a point for us from the calculations above.
				// Enable firing here.
			}
			//
			//////////////////////////////////////////////////////////////////////

			vecGoal.x += (0.20f + 1.50f * m_fUnitEffectEnd) * fmath_RandomBipolarUnitFloat();
			vecGoal.y += (0.20f + 1.50f * m_fUnitEffectEnd) * fmath_RandomBipolarUnitFloat();
			vecGoal.z += (0.20f + 1.50f * m_fUnitEffectEnd) * fmath_RandomBipolarUnitFloat();

			_DoMovement(vecGoal);
			_PropWork();

			break;
		}
		case STATE_STOWING:
		{
			m_fStateTimer += FLoop_fPreviousLoopSecs;
			if(m_fStateTimer >= 10.0f)
			{
				m_eCurState = STATE_STOWED;
				m_pMeshEntity->RemoveFromWorld();
			}
			else
			{
				CFVec3A vecGoal;
				vecGoal = m_pOwnerBot->MtxToWorld()->m_vPos;
				CFVec3A vecTemp;
				vecTemp = m_pOwnerBot->MtxToWorld()->m_vY;
				vecTemp.Mul(10.0f * m_fStateTimer * m_fStateTimer + 5.0f);
				vecGoal.Add(vecTemp);
				vecTemp = m_pOwnerBot->MtxToWorld()->m_vZ;
				vecTemp.Mul(16.0f * m_fStateTimer + 2.0f);
				vecGoal.Add(vecTemp);

				_DoMovement(vecGoal);
				_PropWork();
			}
			break;
		}
		default:
		{
			FASSERT_NOW;
			break;
		}
	}
}



void CRoboBuddy::SetUnitEffectEnd(f32 fUnitEffectEnd)
{
	m_fUnitEffectEnd = fUnitEffectEnd;
}



void CRoboBuddy::StartStow()
{
	if( m_eCurState != STATE_STOWED ) {
		m_eCurState = STATE_STOWING;
		m_fStateTimer = 0.0f;
	}
}



CFVec3A *CRoboBuddy::GetFirePos()
{
	if(m_eCurState == STATE_DEPLOYED)
	{
		CFMtx43A **apBoneMtx = m_pMeshEntity->GetMeshInst()->GetBoneMtxPalette();
		return(&apBoneMtx[m_nFireBoneIdx]->m_vPos);
	}
	else
	{
		return(NULL);
	}
}

//==================
// private functions

void CRoboBuddy::_Reset()
{
	m_pMeshEntity = NULL;
	m_pAnimCombiner = NULL;
	m_pRestMtx = NULL;
	m_pManMtx = NULL;

	m_eCurState = STATE_STOWED;
	
	// Justin wasn't initializing all of his variables...again :-(
	m_pOwnerBot = NULL;
	m_fThetaX = 0.0f;
	m_fOmegaX = 0.0f;
	m_fThetaY = 0.0f;
	m_fOmegaY = 0.0f;
	m_vecVel.Zero();
	m_fPropThetaY = 0.0f;
	m_pMeshEntity = NULL;
	m_uLastHoverPos = 0;
	m_nFireBoneIdx = -1;
	m_fStateTimer = 0.0f;
}



void CRoboBuddy::_DoMovement(const CFVec3A &vecGoal)
{
	static const f32 _fRestoreF = 0.35f * 1.9f * 1.9f * 1.2f * 1.2f * 1.5f * 60.0f;
	static const f32 _fDampF = -2.00f * 2.0f * 1.8f * 0.9f;
	static const f32 _fRepelF = -2.40f * 2.0f * 3.0f * 0.50f * 60.0f;

	//////////////////////////////////////////////////////////////////////
	// Calculate the new position for the mesh.
		//////////////////////////////////////////////////////////////////////
		// Calculate the restoring force (to the goal position).
		CFVec3A vecForceRestore = vecGoal;
		vecForceRestore.Sub(m_pMeshEntity->MtxToWorld()->m_vPos);
		vecForceRestore.Mul(_fRestoreF);
		if(m_uLastHoverPos != 0)
		{
			vecForceRestore.Mul(0.7f);
		}
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Calculate the dampening force.
		CFVec3A vecForceDamp = m_vecVel;
		vecForceDamp.Mul(_fDampF);
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Calculate the repelling force (keeps it from colliding with Blink).
		// JUSTIN: Redo this distance calculation.
		CFVec3A vecForceRepel = m_pOwnerBot->MtxToWorld()->m_vPos;
		// JUSTIN: This probably isn't correct, but it's not really wrong either.
		vecForceRepel.v3 += m_pOwnerBot->m_pWorldMesh->m_BoundSphere_MS.m_Pos;
		vecForceRepel.y += 1.0f;
		vecForceRepel.Sub(m_pMeshEntity->MtxToWorld()->m_vPos);
		f32 fDistSq = vecForceRepel.MagSq();
		if(fDistSq <= 9.0f)
		{
			vecForceRepel.Mul(_fRepelF / fDistSq);
		}
		else
		{
			vecForceRepel.Zero();
		}
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Integrate the forces to get the impulses.
		CFVec3A vecImpulse = vecForceRestore;
		vecImpulse.Add(vecForceDamp);
		vecImpulse.Add(vecForceRepel);
		vecImpulse.Mul(FLoop_fPreviousLoopSecs);
		m_vecVel.Add(vecImpulse);
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Integrate the velocity to get the displacement.
		CFVec3A vecDisplace = m_vecVel;
		vecDisplace.Mul(FLoop_fPreviousLoopSecs);
		//
		//////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// 

	FASSERT(m_fThetaY >= 0.0f);
	FASSERT(m_fThetaY < FMATH_2PI);

	f32 fDelThetaY = m_fThetaY - m_pOwnerBot->m_fMountYaw_WS;
	while(fDelThetaY < -FMATH_PI)
	{
		fDelThetaY += FMATH_2PI;
	}
	while(fDelThetaY > FMATH_PI)
	{
		fDelThetaY -= FMATH_2PI;
	}

	f32 fRestore = -11.0f * 3.0f * 1.2f * fDelThetaY;
	f32 fDamp = -2.0f * 1.8f * 1.2f * (m_fOmegaY);

	m_fOmegaY += (fRestore + fDamp) * FLoop_fPreviousLoopSecs;
	m_fThetaY += (m_fOmegaY) * FLoop_fPreviousLoopSecs;

	if(m_fThetaY < 0.0f)
	{
		m_fThetaY += FMATH_2PI;
	}
	else if(m_fThetaY >= FMATH_2PI)
	{
		m_fThetaY -= FMATH_2PI;
	}
	FASSERT(m_fThetaY >= 0.0f);
	FASSERT(m_fThetaY < FMATH_2PI+FMATH_POS_EPSILON);

	CFVec3A vecPos = m_pMeshEntity->MtxToWorld()->m_vPos;
	vecPos.Add(vecDisplace);

	CFMtx43A mtxOri;
	mtxOri.Identity();

	// Do a bank based on our lateral movement.
	f32 fThetaZ = -vecDisplace.Dot(m_pMeshEntity->MtxToWorld()->m_vRight);
	FMATH_CLAMPMIN(fThetaZ, -FMATH_PI / 7.0f);
	FMATH_CLAMPMAX(fThetaZ, FMATH_PI / 7.0f);
	mtxOri.RotateZ(fThetaZ);

	mtxOri.RotateY(m_fThetaY);

	mtxOri.m_vPos = vecPos;

	m_pMeshEntity->Relocate_RotXlatFromUnitMtx_WS(&mtxOri);
	//
	//////////////////////////////////////////////////////////////////////
}



void CRoboBuddy::_PropWork()
{
	//////////////////////////////////////////////////////////////////////
	// Rotate the propeller.
	CFMtx43A mtxTemp;

	mtxTemp.Identity();
	f32 fTemp = m_vecVel.Mag();
	m_fPropThetaY += (FLoop_fPreviousLoopSecs * (1.5f * fTemp + 11.0f + fmath_RandomFloatRange(0.0f, 5.0f)));
	mtxTemp.RotateY(m_fPropThetaY);
	m_pManMtx->UpdateFrame(0, mtxTemp);
	//
	//////////////////////////////////////////////////////////////////////
}


void CRoboBuddy::_CalcPosFromOfs(CFVec3A &vecOut, u32 uPtIdx)
{
	CFVec3A vecRight = m_pOwnerBot->MtxToWorld()->m_vRight;
	vecRight.Mul(_afRoboOfsX[uPtIdx]);
	CFVec3A vecUp = m_pOwnerBot->MtxToWorld()->m_vUp;
	vecUp.Mul(_afRoboOfsY[uPtIdx]);
	CFVec3A vecFront = m_pOwnerBot->MtxToWorld()->m_vFront;
	vecFront.Mul(_afRoboOfsZ[uPtIdx]);

	vecOut = m_pOwnerBot->MtxToWorld()->m_vPos;
	vecOut.Add(vecRight);
	vecOut.Add(vecUp);
	vecOut.Add(vecFront);
}



BOOL CRoboBuddy::_SphereCollideCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex)
{
	CFWorldMesh *pWM = (CFWorldMesh *)(pTracker);
	CFCollInfo oCollInfo;
	oCollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
	oCollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	oCollInfo.bFindClosestImpactOnly = FALSE;
	oCollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
	oCollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	oCollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	oCollInfo.pSphere_WS = s_poSphere;

	if((pWM != s_pCurOwnerBot->m_pWorldMesh) && (pWM->CollideWithMeshTris(&oCollInfo)))
	{
		s_bHitMesh = TRUE;
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}