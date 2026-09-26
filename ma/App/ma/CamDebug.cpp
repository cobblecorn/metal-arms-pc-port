//////////////////////////////////////////////////////////////////////////////////////
// CamDebug.cpp - a debug camera (looks at another camera) or just a free camera (no collision) 
//
// Author: Michael Starich   
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
// 01/31/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "CamDebug.h"
#include "fmath.h"
#include "floop.h"
#include "gamepad.h"
#include "level.h"
#include "player.h"
#include "entity.h"
#include "bot.h"
#include "botglitch.h"

#if CAM_DEBUG_ENABLE

//====================
// private definitions 

#define _STARTING_X_OFFSET		0.0f
#define _STARTING_Y_OFFSET		2.0f
#define _STARTING_Z_OFFSET		-5.0f
#define _STARTING_FOV			FMATH_QUARTER_PI

#define _MAX_XLAT_SPEED			( 0.85f * 215.0f )
#define _MAX_ROT_SPEED			( FMATH_DEG2RAD( 1.60f * 100.0f ) )
#define _MAX_ZOOM_SPEED			( 0.65f * 115.0f )
#define _MIN_LENGTH				2.0f
#define _MAX_PITCH				( FMATH_DEG2RAD( 89.95f ) )

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions

//==================
// private functions

CCamDebug::CCamDebug() : CFCameraMan() {
	Reset();
}

CCamDebug::~CCamDebug() {
}

BOOL CCamDebug::Init( CFCamera *pTarget, u32 nControlPortIndex ) {

	// do some parameter error checking
	if( !m_pCamera ) {
		DEVPRINTF( "CCamDebug::Init() : No camera assigned, call AssignCamera() before Init().\n" );
		return FALSE;
	}
	if( !pTarget ) {
		DEVPRINTF( "CCamDebug::Init() : Debug cameras require a target camera to init.\n" );
		return FALSE;
	}
	if( nControlPortIndex >= GAMEPAD_MAX_PORT_COUNT ) {
		DEVPRINTF( "CCamDebug::Init() : Debug cameras require a valid controller port index to init.\n" );
		return FALSE;
	}	
	// setup our starting mode
	m_nMode = CAM_DEBUG_FREE;
	m_pTargetCam = pTarget;
	m_nControlPort = nControlPortIndex;
	m_bSwitchModesButtonDown = FALSE;
	m_bFreezeShot = FALSE;
	m_bAllowModeChanges = TRUE;
	m_bAllowMovement = TRUE;

	m_bFreeModeValid = FALSE;
	m_bLookAtModeValid = FALSE;

	// default our camera to our starting offset
	SetFreeModeDefaults();

	// see if this level has specified near/far planes in the data file
	if( Level_hLevelDataFile != FGAMEDATA_INVALID_FILE_HANDLE ) {
		f32 fNearZ, fFarZ, *pfNewValue;
		FGameDataTableHandle_t hTable;
		FGameData_VarType_e nDataType;
		BOOL bAdjustClipPlanes = FALSE;

		// grab the default clip planes
		m_pCamera->GetClipPlanes( &fNearZ, &fFarZ );

		// see if we have a near plane table 
		hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "near_plane" );
		if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
			pfNewValue = (f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
			if( nDataType == FGAMEDATA_VAR_TYPE_FLOAT ) {
				fNearZ = *pfNewValue;
				bAdjustClipPlanes = TRUE;
			}											    
		}

		// see if we have a far plane table
		hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "far_plane" );
		if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
			pfNewValue = (f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
			if( nDataType == FGAMEDATA_VAR_TYPE_FLOAT ) {
				fFarZ = *pfNewValue;
				bAdjustClipPlanes = TRUE;
			}											    
		}

		if( bAdjustClipPlanes ) {
			// set our clip planes
			m_pCamera->SetClipPlanes( fNearZ, fFarZ );
		}

		// must be done last in this function
		m_pCamera->MoveViewport( pTarget->GetViewport() );
	}

	return TRUE;
}

void CCamDebug::SetFreeModeDefaults() {

	CFVec3 PosMS( _STARTING_X_OFFSET, _STARTING_Y_OFFSET, _STARTING_Z_OFFSET );
	m_pTargetCam->GetXfmWithoutShake()->TransformPointR( m_FreePos_WS, PosMS );
	m_pTargetCam->InvalidateCacheKeys();
	m_pTargetCam->GetHeadingPitchRoll( &m_fFreeHeading, &m_fFreePitch, NULL );
	
	m_pCameraData->m_Xfm.BuildInvRotYXZ_XlatFromPoint( m_fFreeHeading, m_fFreePitch, 0.0f,
													   m_FreePos_WS.x, m_FreePos_WS.y, m_FreePos_WS.z );
	m_pCamera->SetFOV( _STARTING_FOV );

	m_pCamera->InvalidateCacheKeys();

	m_bFreeModeValid = TRUE;
}

void CCamDebug::SetLookAtModeDefaults() {
	
	CFVec3 PosMS( _STARTING_X_OFFSET, _STARTING_Y_OFFSET, _STARTING_Z_OFFSET );
	
	m_fLookAtLength = PosMS.Mag();
	m_pTargetCam->GetXfmWithoutShake()->TransformPointR( m_LookAtPos_WS, PosMS );
	m_pTargetCam->InvalidateCacheKeys();
	m_pTargetCam->GetHeadingPitchRoll( &m_fLookAtHeading, &m_fLookAtPitch, NULL );
	
	m_pCameraData->m_Xfm.BuildLookat( m_LookAtPos_WS, *m_pTargetCam->GetPos() );
	m_pCamera->SetFOV( _STARTING_FOV );

	m_pCamera->InvalidateCacheKeys();

	m_bLookAtModeValid = TRUE;
}

void CCamDebug::Work() {
	CFVec3 PosMS, Perp;
	CFQuat Quat;
	CFMtx43 Mtx43;

	if( !m_pTargetCam ) {
		return;
	}

	if( m_bFreezeShot ) {
		// don't do anything...we are locked
		return;
	}

	// compute our max speeds based on our framerate
	f32 fMaxXlat = (m_bAllowMovement) ? _MAX_XLAT_SPEED * FLoop_fPreviousLoopSecs : 0.0f;
	f32 fMaxRot = (m_bAllowMovement) ? _MAX_ROT_SPEED * FLoop_fPreviousLoopSecs : 0.0f;
	f32 fMaxZoom = (m_bAllowMovement) ? _MAX_ZOOM_SPEED * FLoop_fPreviousLoopSecs : 0.0f;
	
	// grab the controls that we are interested in
	f32 fForward = Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState;
	f32 fAimDown = Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState;
	f32 fStrafeRight = Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState;
	f32 fRotateCW = Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_ROTATE_LEFT_RIGHT]->fCurrentState;

	// square all of the analog stick controls before using
	fForward = SafeAnalogSquare( fForward );
	fAimDown = SafeAnalogSquare( fAimDown );
	fStrafeRight = SafeAnalogSquare( fStrafeRight );
	fRotateCW = SafeAnalogSquare( fRotateCW );

	BOOL bActionButton = ( Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_ACTION]->uLatches & FPAD_LATCH_ON );
	BOOL bJumpButton = ( Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_JUMP]->uLatches & FPAD_LATCH_ON );
	BOOL bFire1Button = ( Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_FIRE_PRIMARY]->fCurrentState > 0.0f );
	f32 fFire1 = Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_FIRE_PRIMARY]->fCurrentState;
	BOOL bFire2Button = ( Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_FIRE_SECONDARY]->fCurrentState > 0.0f );
	f32 fFire2 = Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_FIRE_SECONDARY]->fCurrentState;
	BOOL bSelectFire1 = ( Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches & FPAD_LATCH_ON );
	BOOL bSelectFire2 = ( Gamepad_aapSample[m_nControlPort][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & FPAD_LATCH_ON );
	
	////////////////////////////////
	// see if we should switch modes
	if( !m_bSwitchModesButtonDown && (fStrafeRight < -0.6f) && bJumpButton && bSelectFire1 ) {
		m_bSwitchModesButtonDown = TRUE;
		m_nMode++;
		SwitchModes( m_nMode );

		// don't move this frame
		fMaxXlat = 0.0f;
		fMaxRot = 0.0f;
		fMaxZoom = 0.0f;
	} else if( (fStrafeRight > -0.6f) && !bJumpButton && !bSelectFire1 ) {
		m_bSwitchModesButtonDown = FALSE;
	}

	//////////////////////////////
	// move around the environment
	switch( m_nMode ) {
	default:
	case CAM_DEBUG_FREE:

		if( bActionButton ) {
			//CPlayer::m_pCurrent->m_pEntityCurrent->RemoveFromWorld();
			CPlayer::m_pCurrent->m_pEntityCurrent->Relocate_RotXlatFromUnitMtx_WS(&m_pCamera->GetFinalXfm()->m_MtxR);
			((CBot *)CPlayer::m_pCurrent->m_pEntityCurrent)->ZeroVelocity();
			CPlayer::m_pCurrent->m_pEntityCurrent->Work();
			if( CPlayer::m_pCurrent->m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
				CBotGlitch *pGlitchy = (CBotGlitch*) CPlayer::m_pCurrent->m_pEntityCurrent;
				pGlitchy->SnapAntenna();	// snap antenna to new position
			}
		} else 	if( !(bSelectFire1 && !bJumpButton && (fStrafeRight > 0.6f) ) ) {
			// pitch the camera
			m_fFreePitch += (fAimDown * fMaxRot);
			FMATH_CLAMP( m_fFreePitch, -_MAX_PITCH, _MAX_PITCH );
			// change the heading of the camera
			m_fFreeHeading += (fRotateCW * fMaxRot);
			// move the camera through the world
			PosMS.x = fStrafeRight * fMaxXlat;
			if( bFire1Button && bFire2Button ) {
				// both up & down are pressed, don't move up or down
				PosMS.y = 0.0f;
			} else {
				if( bFire2Button ) {
					//FANG_PLATFORM_CONSOLE
					PosMS.y = -fMaxXlat * fFire2;
				} else if( bFire1Button ) {
					PosMS.y = fMaxXlat * fFire1;
				} else {
					PosMS.y = 0.0f;
				}
			}
			PosMS.z = fForward * fMaxXlat;
			m_pCameraData->m_Xfm.TransformPointR( m_FreePos_WS, PosMS );
			// build the final xfm
			m_pCameraData->m_Xfm.BuildInvRotYXZ_XlatFromPoint( m_fFreeHeading, m_fFreePitch, 0.0f,
															   m_FreePos_WS.x, m_FreePos_WS.y, m_FreePos_WS.z );
			m_pCamera->InvalidateCacheKeys();
		} else {
			// restore the default settings
			SetFreeModeDefaults();
		}				
		break;
	case CAM_DEBUG_LOOKAT:
		if( !(bSelectFire1 && !bJumpButton && (fStrafeRight > 0.6f) ) ) {
			// zoom in or out of the point
			if( !(bFire1Button && bFire2Button) ) {
				// both up & down are not pressed, zoom up or down
				if( bFire2Button ) {
					m_fLookAtLength += fMaxZoom * fFire2;
				} else if( bFire1Button ) {
					m_fLookAtLength -= fMaxZoom * fFire1;
					FMATH_CLAMPMIN( m_fLookAtLength, _MIN_LENGTH );
				}
			}

			// pitch the camera
			m_fLookAtPitch += (fForward * fMaxRot);
			FMATH_CLAMP( m_fLookAtPitch, -_MAX_PITCH, _MAX_PITCH );
			// change the heading of the camera
			m_fLookAtHeading += (fStrafeRight * fMaxRot);

			PosMS.Set( 0.0f, 0.0f, -1.0f );
			PosMS.RotateY( m_fLookAtHeading );
			Perp.Set( PosMS.z, 0.0f, -PosMS.x );

			Quat.BuildQuat( Perp, -m_fLookAtPitch );

			Quat.BuildMtx( Mtx43.m33 );
			Mtx43.m_vPos.Zero();
			Mtx43.m_vPos = *m_pTargetCam->GetPos();

			PosMS *= m_fLookAtLength;
			m_LookAtPos_WS = Mtx43.MultPoint( PosMS ); 

			// build the final xfm
			m_pCameraData->m_Xfm.BuildLookat( m_LookAtPos_WS, *m_pTargetCam->GetPos() );
			m_pCamera->InvalidateCacheKeys();
		} else {
			// restore the default settings
			SetLookAtModeDefaults();
		}
		break;
	case CAM_DEBUG_MATCHYAW:

			f32 fDy = m_pCameraData->m_Xfm.m_MtxR.m_vPos.y - Player_aPlayer[0].m_pEntityCurrent->MtxToWorld()->m_vPos.v3.y;
			PosMS.Set( 0.0f, 0.0f, -m_fLookAtLength );
			PosMS.y = fDy;
			f32 fYaw = fmath_Atan(m_pTargetCam->GetFinalXfm()->m_MtxR.m_vFront.x, m_pTargetCam->GetFinalXfm()->m_MtxR.m_vFront.z);
			PosMS.RotateY(fYaw);
			PosMS.Add(Player_aPlayer[0].m_pEntityCurrent->MtxToWorld()->m_vPos.v3);
			m_pCameraData->m_Xfm.BuildLookat( PosMS, Player_aPlayer[0].m_pEntityCurrent->MtxToWorld()->m_vPos.v3 );
			m_pCamera->InvalidateCacheKeys();
		break;
	}
}

void CCamDebug::Reset() {
	m_nMode = CAM_DEBUG_FREE;
	m_pTargetCam = NULL;
	m_nControlPort = 0;
	m_bSwitchModesButtonDown = FALSE;
	m_bFreeModeValid = FALSE;
	m_bLookAtModeValid = FALSE;
	m_fFreeHeading = 0.0f;
	m_fFreePitch = 0.0f;
	m_fLookAtHeading = 0.0f;
	m_fLookAtPitch = 0.0f;
	m_fLookAtLength = _MIN_LENGTH;
	m_bFreezeShot = FALSE;
	m_bAllowModeChanges = TRUE;
	m_bAllowMovement = TRUE;
}

CFCamera *CCamDebug::GetCameraBeingDebugged() {
	return m_pTargetCam;
}

void CCamDebug::EnableModeChanges( BOOL bEnable ) {
	m_bAllowModeChanges = bEnable;
}

void CCamDebug::EnableUserMovement( BOOL bEnable ) {
	m_bAllowMovement = bEnable;
}

void CCamDebug::SwitchModes( u32 nNewMode ) {
	CFVec3 PosMS;
	const CFVec3 *pPosWS;
	CFVec3 UnitPos;
	
	pPosWS = m_pCamera->GetPos();

	m_nMode = nNewMode;
	switch( m_nMode ) {
	default:
		m_nMode = CAM_DEBUG_FREE; 
	case CAM_DEBUG_FREE:
		// use the last good free cam vars
		m_pCameraData->m_Xfm.BuildInvRotYXZ_XlatFromPoint( m_fFreeHeading, m_fFreePitch, 0.0f,
														   m_FreePos_WS.x, m_FreePos_WS.y, m_FreePos_WS.z );
		m_pCamera->InvalidateCacheKeys();
		break;

	case CAM_DEBUG_LOOKAT:
	case CAM_DEBUG_MATCHYAW:
		// calculate the distance to the target camera
		if( !m_bLookAtModeValid ) {
 			// use the current pos of the debug cam to construct the lookat vars
			m_LookAtPos_WS = *pPosWS;
			PosMS = m_LookAtPos_WS - *m_pTargetCam->GetPos();
			m_fLookAtLength = PosMS.Mag();
			FMATH_CLAMPMIN( m_fLookAtLength, _MIN_LENGTH );
			
			m_pCameraData->m_Xfm.BuildLookat( m_LookAtPos_WS, *m_pTargetCam->GetPos() );
			m_pCamera->InvalidateCacheKeys();
			m_pCamera->GetHeadingPitchRoll( &m_fLookAtHeading, NULL, NULL );

			UnitPos = pPosWS->Unit();
			m_fLookAtPitch = UnitPos.y;		// SER: Used to be UnitPos.Dot( CFVec3::m_UnitAxisY )
			m_fLookAtPitch *= _MAX_PITCH;

			m_bLookAtModeValid = TRUE;
		} else {
			// we have valid lookat vars, use them
			m_pCameraData->m_Xfm.BuildLookat( m_LookAtPos_WS, *m_pTargetCam->GetPos() );
			m_pCamera->InvalidateCacheKeys();
		}
		break;
		break;
	}
}

void CCamDebug::ForceModeChange( u32 nNewMode ) {
	
	if( !m_pTargetCam || !m_bAllowModeChanges ) {
		return;
	}

	m_bSwitchModesButtonDown = TRUE;
	SwitchModes( nNewMode );		
}

f32 CCamDebug::SafeAnalogSquare( f32 fVal ) {
	f32 fSquaredVal;

	fSquaredVal = fVal * fVal;
	if( fVal < 0.0f ) {
		fSquaredVal *= -1.0f;
	}
	return fSquaredVal;
}

#endif