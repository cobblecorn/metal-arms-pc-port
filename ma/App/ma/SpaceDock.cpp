//////////////////////////////////////////////////////////////////////////////////////
// SpaceDock.cpp - 
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
// 08/01/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "SpaceDock.h"

#include "FScriptSystem.h"
#include "fanim.h"

#include "gamepad.h"

#include "meshentity.h"
#include "ESwitch.h"

#include "gamesave.h"
#include "player.h"
#include "gamecam.h"
#include "game.h"
#include "bot.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

BOOL CSpaceDock::s_bSystemInitted = FALSE;

CMeshEntity *CSpaceDock::s_pEMagnet = NULL;
CESwitch *CSpaceDock::s_pESwitch = NULL;
CMeshEntity *CSpaceDock::s_apFence[5];
CEntity *CSpaceDock::s_pCamPos = NULL;
//CEntity *CSpaceDock::s_pETrigger = NULL;

u32 CSpaceDock::s_hEventListener;

CCamManualInfo CSpaceDock::s_oCamInfo;
CFMtx43A CSpaceDock::s_mtxCam;

CSpaceDock::State_e CSpaceDock::s_eCurState;

CFAnimManMtx *CSpaceDock::s_pManMtx;
CFAnimMeshRest *CSpaceDock::s_pRestMtx;
CFAnimCombiner *CSpaceDock::s_pAnimCombiner;
CFVec2 CSpaceDock::s_vecCraneDisp;
CFVec2 CSpaceDock::s_vecCraneVel;
f32 CSpaceDock::s_fMagnetDispY;
f32 CSpaceDock::s_fMagnetVelY;

cchar *CSpaceDock::s_apszCraneBoneNames[CRANEBONE_COUNT] = { "Magnet", "CrossBeam", "Slider" };
s32 CSpaceDock::s_anCraneBoneIndices[CRANEBONE_COUNT];

f32 CSpaceDock::s_fStateTimer;
f32 CSpaceDock::s_fFenceTimer;
BOOL CSpaceDock::s_bFenceToggleState;

static const CFVec2 _vecCranePosMin(-21.7f, -90.0f);
static const CFVec2 _vecCranePosMax(50.0f, 70.0f);
static const f32 _fMaxMagnetDispY = -20.0f;
static const CFVec3A _avecFencePos[2] = { CFVec3A(45.0f, 10.4f, 178.9f), CFVec3A(98.3f, 10.4f, 178.9f) };
static const cchar *_apszFenceNames[] = { "fencestart", "fenceleftmid", "fenceleftup", "fencerightmid", "fencerightup" };
static const f32 _fLowerTime = 1.20f;
static const f32 _fPauseTime = 0.5f;
static const f32 _fFencePullTime = 2.0f;
static const f32 _fRaiseTime = 1.10f;
static const f32 _fEndPause = 1.0f;

static const f32 _fOOLowerTime = 1.0f / _fLowerTime;
static const f32 _fOORaiseTime = 1.0f / _fRaiseTime;

CFCheckPoint::ObjectDataHandle_t SpaceDoc_hSaveData[ FCHECKPOINT_MAX_CHECKPOINTS ];

//===================
// private prototypes

//=================
// public functions

BOOL CSpaceDock::InitLevel( LevelEvent_e eEvent )
{
	//////////////////////////////////////////////////////////////////////
	// Glitch turn around test
//	do
//	{
//		s32 nTriggerEvent = CFScriptSystem::GetEventNumFromName("tripwire");
//		if(nTriggerEvent == -1)
//		{
//			DEVPRINTF("CSpaceDock::InitLevel() : Could not find tripwire event.\n");
//			break;
//		}
//
//		u64 uEventFlags = 1 << nTriggerEvent;
//
//		if( !CFScriptSystem::RegisterEventListener( _TurnAroundCallback, NULL, &uEventFlags, (u32) nTriggerEvent ) )
//		{
//			DEVPRINTF("CSpaceDock::InitLevel() : Could not register turn around listener.\n" );
//			break;
//		}
//
//		s_pETrigger = (CEntity*) (CEntity::FindInWorld("airlock_trigger"));
//		if(s_pETrigger == NULL)
//		{
//			DEVPRINTF("CSpaceDock::InitLevel() : Could not find airlock_trigger.\n");
//			break;
//		}
//
//	}
//	while( 0 );

	if(eEvent != LEVEL_EVENT_POST_WORLD_LOAD)
	{
		return TRUE;
	}
	
	//////////////////////////////////////////////////////////////////////
	// Get things set up with the event system.
	s32 nSwitchEvent = CFScriptSystem::GetEventNumFromName("switch");
	if(nSwitchEvent == -1)
	{
		DEVPRINTF("CSpaceDock::InitLevel() : Could not find switch event.\n");
		return(TRUE);
	}

	u64 uEventFlags = 1 << nSwitchEvent;
	if(!CFScriptSystem::RegisterEventListener(_EventCallback, &s_hEventListener, &uEventFlags))
	{
		DEVPRINTF("CSpaceDock::InitLevel() : Could not register event listener.\n");
		return(TRUE);
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Find the entities that we are interested in.
	s_pEMagnet = (CMeshEntity *)(CEntity::FindInWorld("crane"));
	if(s_pEMagnet == NULL)
	{
		DEVPRINTF("CSpaceDock::InitLevel() : Could not find crane.\n");
		return(TRUE);
	}

	s_pESwitch = (CESwitch *)(CEntity::FindInWorld("craneswitch"));
	if(s_pESwitch == NULL)
	{
		DEVPRINTF("CSpaceDock::InitLevel() : Could not find crane switch.\n");
		return(TRUE);
	}

	//////////////////////////////////////////////////////////////////////
	// Find all of the versions of the fence in the world.
	u32 uCurFenceIdx;
	for(uCurFenceIdx = 0; uCurFenceIdx < 5; ++uCurFenceIdx)
	{
		s_apFence[uCurFenceIdx] = (CMeshEntity *)(CEntity::Find(_apszFenceNames[uCurFenceIdx]));
		if(s_apFence[uCurFenceIdx] == NULL)
		{
			DEVPRINTF("CSpaceDock::InitLevel() : Could not find entity named '%s' in world.\n", _apszFenceNames[uCurFenceIdx]);
			return(TRUE);
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	s_pCamPos = CEntity::Find("cranecampos");
	if(s_pCamPos == NULL)
	{
		DEVPRINTF("CSpaceDock::InitLevel() : Could not find camera position for magnet puzzle.\n");
		return(TRUE);
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Find the indices of the bones that we are interested in.
	u32 uCraneBoneIdx;
	CFMeshInst *pMI = s_pEMagnet->GetMeshInst();
	for(uCraneBoneIdx = 0; uCraneBoneIdx < CRANEBONE_COUNT; ++uCraneBoneIdx)
	{
		s_anCraneBoneIndices[uCraneBoneIdx] = pMI->FindBone(s_apszCraneBoneNames[uCraneBoneIdx]);
		if(s_anCraneBoneIndices[uCraneBoneIdx] == -1)
		{
			DEVPRINTF("CSpaceDock::InitLevel() : Could not find bone '%s' on crane.\n", s_apszCraneBoneNames[uCraneBoneIdx]);
			return(TRUE);
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Set up our camera info.
	s_oCamInfo.m_fHalfFOV = FMATH_PI / 8.0f;
	s_oCamInfo.m_pmtxMtx = &s_mtxCam;
	s_oCamInfo.m_pPosObj = NULL;
	s_oCamInfo.m_pqQuat = NULL;
	s_oCamInfo.m_pQuatObj = NULL;
	s_oCamInfo.m_pvecPos = NULL;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
		//////////////////////////////////////////////////////////////////////
		// Create a manual matrix animation object.
		s_pManMtx = fnew CFAnimManMtx;
		if(s_pManMtx == NULL)
		{
			DEVPRINTF("CSpaceDock::InitLevel() : Could not create CFAnimManMtx.\n");
			return(TRUE);
		}
		if(!s_pManMtx->Create(CRANEBONE_COUNT, s_apszCraneBoneNames))
		{
			DEVPRINTF("CSpaceDock::InitLevel() : Could not Create() CFAnimManMtx.\n");
			return(TRUE);
		}
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Create the mesh rest animation object.
		s_pRestMtx = fnew CFAnimMeshRest;
		if(s_pRestMtx == NULL)
		{
			DEVPRINTF("CSpaceDock::InitLevel() : Could not create CFAnimMeshRest.\n");
			return(TRUE);
		}
		if(!s_pRestMtx->Create(s_pEMagnet->GetMeshInst()->m_pMesh))
		{
			DEVPRINTF("CSpaceDock::InitLevel() : Could not Create() CFAnimMeshRest.\n");
			return(TRUE);
		}
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Create an animation combiner object.
		s_pAnimCombiner = fnew CFAnimCombiner;
		if(s_pAnimCombiner == NULL)
		{
			DEVPRINTF("CSpaceDock::InitLevel() : Could not create CFAnimCombiner.\n");
			return(TRUE);
		}
		s_pAnimCombiner->CreateSummer(s_pEMagnet->GetMeshInst());
		s_pAnimCombiner->AttachToTap(0, s_pRestMtx);
		s_pAnimCombiner->AttachToTap(1, s_pManMtx);
		//
		//////////////////////////////////////////////////////////////////////

		s_pEMagnet->DriveMeshWithAnim(TRUE);
		s_pEMagnet->UserAnim_Select(-1);
		s_pEMagnet->SetExternalCombiner(0, s_pAnimCombiner);
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	s_eCurState = STATE_PUZZLEUNSOLVED;
	//
	//////////////////////////////////////////////////////////////////////

	s_bSystemInitted = TRUE;

	s_vecCraneDisp.Zero();
	s_vecCraneVel.Zero();
	s_fMagnetDispY = 0.0f;
	s_fMagnetVelY = 0.0f;

	return(TRUE);
}



void CSpaceDock::UninitLevel()
{
	if(!s_bSystemInitted)
	{
		return;
	}

	fdelete(s_pAnimCombiner);
	s_pAnimCombiner = NULL;

	fdelete(s_pRestMtx);
	s_pRestMtx = NULL;

	fdelete(s_pManMtx);
	s_pManMtx = NULL;
}


void CSpaceDock::CheckpointSave(s32 nCheckpointID)
{
	SpaceDoc_hSaveData[ nCheckpointID ] = CFCheckPoint::CreateObjectDataHandle();
	u32 uTmp = (u32)s_eCurState;
	CFCheckPoint::SaveData(uTmp);
}


void CSpaceDock::CheckpointRestore(s32 nCheckpointID)
{
	CFCheckPoint::SetObjectDataHandle( SpaceDoc_hSaveData[ nCheckpointID ] );
	u32 uTmp;
	CFCheckPoint::LoadData( uTmp);
	s_eCurState = (CSpaceDock::State_e) uTmp;
}


void CSpaceDock::Work()
{
	if(!s_bSystemInitted)
	{
		return;
	}

	if((s_eCurState == STATE_PUZZLEUNSOLVED) || (s_eCurState == STATE_PUZZLESOLVED))
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////
	// Check the controls.
	u32 nControlIndex = Player_aPlayer[0].m_nControllerIndex;
	f32 fCrossBar = Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState;
	f32 fSlider = Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState;
	BOOL bActivate = Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_JUMP]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT;
	BOOL bCancel = Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// State transitions.
	switch(s_eCurState)
	{
		case STATE_OPERATINGCRANE:
		{
			if(bCancel && s_vecCraneVel.Mag2() < 16.0f)
			{
				// Don't let them out unless they're mostly stopped.
				_LeaveCraneCam();

				BOOL bRetVal = game_LeaveMiniGameMode();
				FASSERT(bRetVal);

				s_vecCraneVel.Zero();

				s_eCurState = STATE_PUZZLEUNSOLVED;
			}
			else if(bActivate && s_vecCraneVel.Mag2() < 16.0f)
			{
				// Don't let them activate the magnet unless they're mostly stopped.
				s_vecCraneVel.Zero();
				s_fStateTimer = _fLowerTime;
				s_eCurState = STATE_LOWERINGCRANE;
			}
			break;
		}
		case STATE_LOWERINGCRANE:
		{
			s_fStateTimer -= FLoop_fPreviousLoopSecs;
			if(s_fStateTimer <= 0.0f)
			{
				s_fStateTimer = _fPauseTime;
				s_eCurState = STATE_PAUSEDATBOTTOM;
			}
			break;
		}
		case STATE_PAUSEDATBOTTOM:
		{
			s_fStateTimer -= FLoop_fPreviousLoopSecs;
			if(s_fStateTimer <= 0.0f)
			{
				// The magnet is in the down position, let's see if we're close enough to
				// either of the fences.
				CFVec3A vecMagnetPos = s_pEMagnet->GetMeshInst()->GetBoneMtxPalette()[s_anCraneBoneIndices[CRANEBONE_MAGNET]]->m_vPos;

				if(vecMagnetPos.DistSqXZ(_avecFencePos[0]) <= 250.0f)
				{
					s_fFenceTimer = 0.05f;
					s_fStateTimer = _fFencePullTime;
					s_bFenceToggleState = TRUE;

//					fpart_SpawnParticleEmitter(FPART_TYPE_SPARK_BURST, 1.0f, _avecFencePos[0], &CFVec3A::m_UnitAxisY, TRUE);
					s_eCurState = STATE_RAISINGFENCE1;
				}
				else if(vecMagnetPos.DistSqXZ(_avecFencePos[1]) <= 250.0f)
				{
					s_fFenceTimer = 0.05f;
					s_fStateTimer = _fFencePullTime;
					s_bFenceToggleState = TRUE;

//					BOOL bRetVal = fpart_SpawnParticleEmitter(FPART_TYPE_SPARK_BURST, 1.0f, _avecFencePos[1], &CFVec3A::m_UnitAxisY, TRUE);
//					FASSERT(bRetVal);
					s_eCurState = STATE_RAISINGFENCE2;
				}
				else
				{
					s_fStateTimer = _fRaiseTime;
					s_eCurState = STATE_RAISINGCRANE;
				}
			}
			break;
		}
		case STATE_RAISINGCRANE:
		{
			s_fStateTimer -= FLoop_fPreviousLoopSecs;
			if(s_fStateTimer <= 0.0f)
			{
				s_eCurState = STATE_OPERATINGCRANE;
			}
			break;
		}
		case STATE_RAISINGFENCE1:
		{
			s_fStateTimer -= FLoop_fPreviousLoopSecs;
			if(s_fStateTimer <= 0.0f)
			{
				s_apFence[0]->RemoveFromWorld();
				s_apFence[1]->RemoveFromWorld();
				s_apFence[2]->AddToWorld();

				s_fStateTimer = _fEndPause;

				s_eCurState = STATE_PAUSEDATEND;
			}
			break;
		}
		case STATE_RAISINGFENCE2:
		{
			s_fStateTimer -= FLoop_fPreviousLoopSecs;
			if(s_fStateTimer <= 0.0f)
			{
				s_apFence[0]->RemoveFromWorld();
				s_apFence[3]->RemoveFromWorld();
				s_apFence[4]->AddToWorld();

				s_fStateTimer = _fEndPause;

				s_eCurState = STATE_PAUSEDATEND;
			}
			break;
		}
		case STATE_PAUSEDATEND:
		{
			s_fStateTimer -= FLoop_fPreviousLoopSecs;
			if(s_fStateTimer <= 0.0f)
			{
				_LeaveCraneCam();

				BOOL bRetVal = game_LeaveMiniGameMode();
				FASSERT(bRetVal);

				s_vecCraneVel.Zero();

				s_eCurState = STATE_PUZZLESOLVED;
			}

			if(bCancel)
			{
				_LeaveCraneCam();

				BOOL bRetVal = game_LeaveMiniGameMode();
				FASSERT(bRetVal);

				s_vecCraneVel.Zero();

				s_eCurState = STATE_PUZZLESOLVED;
			}

			break;
		}
		default:
		{
			FASSERT_NOW;
			break;
		}
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// State work.
	switch(s_eCurState)
	{
		case STATE_OPERATINGCRANE:
		{
			//////////////////////////////////////////////////////////////////////
			// Update the velocity with the applied forces.
			s_vecCraneVel.x += ((-50.0f * fSlider) - (2.0f * s_vecCraneVel.x)) * FLoop_fPreviousLoopSecs;
			s_vecCraneVel.y += ((50.0f * fCrossBar) - (2.0f * s_vecCraneVel.y)) * FLoop_fPreviousLoopSecs;
			//
			//////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////
			// Update the position.
			s_vecCraneDisp.x += s_vecCraneVel.x * FLoop_fPreviousLoopSecs;
			s_vecCraneDisp.y += s_vecCraneVel.y * FLoop_fPreviousLoopSecs;
			//
			//////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////
			// Make sure it stays in the 'playing area'.
			if(s_vecCraneDisp.x < _vecCranePosMin.x)
			{
				s_vecCraneDisp.x = 2.0f * _vecCranePosMin.x - s_vecCraneDisp.x;
				s_vecCraneVel.x *= -0.8f;
			}
			else if(s_vecCraneDisp.x > _vecCranePosMax.x)
			{
				s_vecCraneDisp.x = 2.0f * _vecCranePosMax.x - s_vecCraneDisp.x;
				s_vecCraneVel.x *= -0.8f;
			}

			if(s_vecCraneDisp.y < _vecCranePosMin.y)
			{
				s_vecCraneDisp.y = 2.0f * _vecCranePosMin.y - s_vecCraneDisp.y;
				s_vecCraneVel.y *= -0.8f;
			}
			else if(s_vecCraneDisp.y > _vecCranePosMax.y)
			{
				s_vecCraneDisp.y = 2.0f * _vecCranePosMax.y - s_vecCraneDisp.y;
				s_vecCraneVel.y *= -0.8f;
			}
			//
			//////////////////////////////////////////////////////////////////////
			break;
		}
		case STATE_LOWERINGCRANE:
		{
			f32 fUnitPos = s_fStateTimer * _fOOLowerTime;
			s_fMagnetDispY = _fMaxMagnetDispY * fmath_UnitLinearToSCurve(1.0f - fUnitPos);

			break;
		}
		case STATE_RAISINGCRANE:
		{
			f32 fUnitPos = s_fStateTimer * _fOORaiseTime;
			s_fMagnetDispY = _fMaxMagnetDispY * fmath_UnitLinearToSCurve(fUnitPos);

			break;
		}
		case STATE_RAISINGFENCE1:
		{
			s_fFenceTimer -= FLoop_fPreviousLoopSecs;
			if(s_fFenceTimer <= 0.0f)
			{
				s_fFenceTimer = 0.03f;
				if(s_bFenceToggleState)
				{
					s_apFence[0]->RemoveFromWorld();
					s_apFence[1]->AddToWorld();
				}
				else
				{
					s_apFence[0]->AddToWorld();
					s_apFence[1]->RemoveFromWorld();
				}
				s_bFenceToggleState = !s_bFenceToggleState;
			}

			break;
		}
		case STATE_RAISINGFENCE2:
		{
			s_fFenceTimer -= FLoop_fPreviousLoopSecs;
			if(s_fFenceTimer <= 0.0f)
			{
				s_fFenceTimer = 0.03f;
				if(s_bFenceToggleState)
				{
					s_apFence[0]->RemoveFromWorld();
					s_apFence[3]->AddToWorld();
				}
				else
				{
//					s_apFence[0]->AddToWorld();
					s_apFence[3]->RemoveFromWorld();
				}
				s_bFenceToggleState = !s_bFenceToggleState;
			}

			break;
		}
		case STATE_PAUSEDATBOTTOM:
		case STATE_PAUSEDATEND:
		case STATE_PUZZLESOLVED:
		case STATE_PUZZLEUNSOLVED:
		{
			break;
		}
		default:
		{
			FASSERT_NOW;
			break;
		}
	}

	_UpdateCraneBones();
	_CalcCraneCam();
}



void CSpaceDock::Draw()
{
	if(!s_bSystemInitted)
	{
		return;
	}
}



//==================
// private functions

void CSpaceDock::_EventCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3)
{
	if(!s_bSystemInitted)
	{
		return;
	}

	CEntity *pE = (CEntity *)(uEventData2);
	FASSERT(pE != NULL);
	if(pE == s_pESwitch)
	{
		switch(s_eCurState)
		{
			case STATE_PUZZLEUNSOLVED:
			{
				BOOL bRetVal = game_EnterMiniGameMode();
				FASSERT(bRetVal);

				// 
				_InitCraneCam();

				s_eCurState = STATE_OPERATINGCRANE;

				break;
			}
			case STATE_PUZZLESOLVED:
			{
				break;
			}
			default:
			{
				// Who's pressing this switch?  Nobody should be.
				FASSERT_NOW;
				break;
			}
		}
	}
}



void CSpaceDock::_InitCraneCam()
{
	_CalcCraneCam();

	// Switch to our special cam.
	gamecam_SwitchPlayerToManualCamera(GAME_CAM_PLAYER_1, &s_oCamInfo);
}



void CSpaceDock::_CalcCraneCam()
{
	// Get the position and orientation from the crane.
/*	CFMtx43A mtxRotate;
	mtxRotate.Identity();
	mtxRotate.SetRotationYXZ(-FMATH_PI * 0.10f, 0.1f, 0.0f);

	CFMeshInst *pMI = s_pEMagnet->GetMeshInst();
	s_mtxCam = *pMI->GetBoneMtxPalette()[s_anCraneBoneIndices[CRANEBONE_SLIDER]];

	s_mtxCam.Mul(mtxRotate);
	s_mtxCam.m_vPos.y += -30.0f;
	s_mtxCam.m_vPos.x += 60.0f * 0.8f;
	s_mtxCam.m_vPos.z -= 180.0f * 0.8f;*/

	CFMeshInst *pMI = s_pEMagnet->GetMeshInst();

	// Calculate the direction in which we want to be looking.
	CFVec3A vecLook;
	vecLook = pMI->GetBoneMtxPalette()[s_anCraneBoneIndices[CRANEBONE_SLIDER]]->m_vPos;
	vecLook.y -= 60.0f;
	vecLook.Sub(s_pCamPos->MtxToWorld()->m_vPos);
	vecLook.Unitize();

	// Calculate the 'right' vector.
	CFVec3A vecRight;
	vecRight.x = vecLook.z;
	vecRight.y = 0.0f;
	vecRight.z = -vecLook.x;
	vecRight.Unitize();

	// Calculate the new 'up' vector.
	CFVec3A vecUp;
	vecUp.Cross(vecLook, vecRight);

	s_mtxCam.m_vX = vecRight;
	s_mtxCam.m_vY = vecUp;
	s_mtxCam.m_vZ = vecLook;
	s_mtxCam.m_vP.Set(s_pCamPos->MtxToWorld()->m_vPos);
}



void CSpaceDock::_LeaveCraneCam()
{
	gamecam_SwitchPlayerTo3rdPersonCamera(GAME_CAM_PLAYER_1, (CBot *)(CPlayer::m_pCurrent->m_pEntityCurrent));
}



void CSpaceDock::_UpdateCraneBones()
{
	CFMtx43A mtxTemp;

	//////////////////////////////////////////////////////////////////////
	// Adjust the magnet on the crossbar.
	mtxTemp.Identity();
	mtxTemp.m_vPos.x = s_vecCraneDisp.x;
	s_pManMtx->UpdateFrame(CRANEBONE_CROSSBEAM, mtxTemp);

	mtxTemp.Identity();
	mtxTemp.m_vPos.x = s_vecCraneDisp.y;
	s_pManMtx->UpdateFrame(CRANEBONE_SLIDER, mtxTemp);

	mtxTemp.Identity();
	mtxTemp.m_vPos.y = s_fMagnetDispY;
	s_pManMtx->UpdateFrame(CRANEBONE_MAGNET, mtxTemp);
	//
	//////////////////////////////////////////////////////////////////////
}

/*
// Glitch turn around test callback
void CSpaceDock::_TurnAroundCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3)
{
	CEntity *pTriggerEntity = (CEntity *)(uEventData2);
	if( pTriggerEntity == NULL ) return;

	if(pTriggerEntity == s_pETrigger )
	{
		CEntity *pBotEntity = (CEntity*) uEventData3;

		if( pBotEntity == NULL ) return;

		if (pBotEntity->TypeBits() & ENTITY_BIT_BOT)
		{
			CBot* pBot = (CBot*) pBotEntity;

			if( pBot->IsReverseFacingMode() )
			{
				pBot->EndReverseFacingMode();
			}
			else
			{
				pBot->StartReverseFacingMode();
			}
		}
	}
}
  */
