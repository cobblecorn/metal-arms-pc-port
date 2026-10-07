//////////////////////////////////////////////////////////////////////////////////////
// SpyVsSpy.cpp - 
//
// Author: Nathan Miller
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/20/03 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fpad.h"
#include "fanim.h"
#include "floop.h"
#include "faudio.h"
#include "fsound.h"
#include "fworld.h"
#include "fcamanim.h"
#include "fresload.h"
#include "fmeshpool.h"
#include "sas_user.h"

#include "hud2.h"
#include "Door.h"
#include "game.h"
#include "Player.h"
#include "entity.h"
#include "fdebris.h"
#include "botpart.h"
#include "espline.h"
#include "esphere.h"
#include "gamecam.h"
#include "SpyVsSpy.h"
#include "botgrunt.h"
#include "meshtypes.h"
#include "botglitch.h"
#include "CamManual.h"
#include "CamBot.h"
#include "explosion.h"
#include "meshentity.h"
#include "BotTalkData.h"
#include "Collectable.h"
#include "SplineActor.h"
#include "FXMeshBuild.h"
#include "SpyVsSpyDDR.h"
#include "TalkSystem2.h"
#include "botscientist.h"
#include "eparticlepool.h"
#include "multiplayerMgr.h"

#include "AI/AIApi.h"
#include "AI/AIBrain.h"
#include "AI/AIMover.h"
#include "AI/AIBrainman.h"
#include "AI/AIBTATable.h"

#define _SPY_VS_SPY_SOUNDS				( "Level_18" )
#define _SPY_VS_SPY_DIALOG				( "Level_18D" )
#define _SPY_VS_SPY_TALKS				( "Level_18DC" )
#define _SPY_VS_SPY_SOUND_CSV			( "snd_spyvspy" )
#define _SPY_VS_SPY_CONFIG				( "spy_cfg" )

#define _SPY_VS_SPY_STREAM_START		( "AS_18o_010$" )
#define _SPY_VS_SPY_STREAM_SEARCH		( "AS_18i_030$" )
#define _SPY_VS_SPY_STREAM_PROGRAM		( "AS_18i_320$" )
#define _SPY_VS_SPY_AMBIENT_STREAM		( "ser_genint" )
#define _SPY_VS_SPY_MUSIC_VOL			( 1.0f )

#define _BIG_RADIUS						( 1000.0f )

// Animations are run at 30fps
#define _OO_ANIM_FPS					( 1.0f / 30.0f )

// Forward declarations
class CCraneStage;


static const char *_apszBTRNames[] = {
	"***_back",	
	"***_Cbac",	
	"***_frwr",	
	"***_Cfrw",	
	"***_steL",	
	"***_CstL",	
	"***_steR",	
	"***_CstR",	
	"***_rotR",	
	"***_CroR",	
	"***_rotL",	
	"***_CroL",	
	"***_jump",
	"***_Cjum",
	"***_flip",
	"***_Cfli",

	"***_defc",	
	"***_hmmm",	
	"***_impo",	
	"***_inva",	
	"***_read",	
	"***_test",	
	"***_thon",	
	"***_what",
};

extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );

#define _STAGE1_CRANE_HEAD				"conv_claw01"	// Head claw
#define _STAGE1_CRANE_TORSO				"conv_claw02"	// Torso claw
#define _STAGE1_CRANE_LEGS				"conv_claw03"	// Feet claw
#define _STAGE1_DISPENSER_HEAD			"conv_disp01"	// Head dispenser
#define _STAGE1_DISPENSER_TORSO			"conv_disp02"	// Torso dispenser
#define _STAGE1_DISPENSER_LEGS			"conv_disp03"	// Feet dispenser
#define _STAGE1_CRANE_DEST_POINTS		"partsspline"	// Dest points for the cranes
#define _STAGE1_CONVEYOR_HEAD_POINTS	"headspline"
#define _STAGE1_CONVEYOR_TORSO_POINTS	"torsospline"
#define _STAGE1_CONVEYOR_LEGS_POINTS	"legsspline"
#define _STAGE1_CRANE_BONE_NAME			"cranedummy"
#define _STAGE1_HEAD_MESH				"gocff_mhead"
#define _STAGE1_TORSO_MESH				"gocff_mtrso"
#define _STAGE1_LEGS_MESH				"gocff_mlegs"
#define _STAGE1_CAMERA_SPLINE			"headcamspline"
#define _STAGE1_CRANE_CAMERA			"headcam"
#define _STAGE1_HEAD_CONVEYOR			"head_conv"
#define _STAGE1_CONSOLE					"main_console"
#define _STAGE1_DISPENSER_SMOKE			"bigsteamy3"
#define _STAGE1_DISPENSER_EXPLOSION		"RocketL1"
#define _STAGE1_PART_EXPLOSION			"MervBoom"
#define _STAGE1_HEAD_CLIP				"BigBird"
#define _STAGE1_TORSO_CLIP				"CookieMonster"
#define _STAGE1_CRANE_BIG_RADIUS		( 30.0f )
#define _STAGE1_CONVEYOR_PART_MOVE_TIME ( 1.0f / 10.0f )
#define _STAGE1_BIG_GLITCH_RADIUS		( 1000.0f )
#define _STAGE1_NUM_EMITTERS			( 3 )
#define	_STAGE1_PART_MAX				( 5 )
#define _STAGE1_GRAB_SPHERE_SIZE		( 6.0f )
#define _STAGE1_SHAKE_UNIT_INTENSITY	( 0.25f )
#define _STAGE1_SHAKE_DURATION			( 0.25f )
#define _STAGE1_DISPENSER_TEXTURE_ID	( 8 )
#define _STAGE1_CONVEYOR_TEXTURE_ID		( 8 )
#define _STAGE1_FOV_FRAC				( 0.5f )
#define _STAGE1_CAMERA_STOP_TIME		( 11.0f )

enum {
	MACHINE_NONE = -1,
	MACHINE_HEAD = 0,
	MACHINE_TORSO,
	MACHINE_LEGS,
	MACHINE_COUNT,
};


BOOL CEmitterFader::m_bAlive = FALSE;
CEmitterFader::FaderEntry_t CEmitterFader::m_Faders[ _MAX_FADERS ];

void CEmitterFader::Setup( void ) {
	m_bAlive = FALSE;

	for( u32 i = 0; i < _MAX_FADERS; ++i ) {
		m_Faders[i].bUsed = FALSE;
	}
}

void CEmitterFader::Reset( void ) { 
	for( u32 i = 0; i < _MAX_FADERS; ++i ) {
		if( m_Faders[i].bUsed && m_Faders[i].bKillWhenDone ) {
			m_Faders[i].pEmitter->Destroy();
			m_Faders[i].pEmitter = NULL;
		}

		m_Faders[i].bUsed = FALSE;
	}
}

void CEmitterFader::Work( void ) {
	if( !m_bAlive ) {
		return;
	}

	f32 fUnit;
	BOOL bAlive = FALSE;
	FaderEntry_t *pFader;

	for( u32 i = 0; i < _MAX_FADERS; ++i ) {
		pFader = &m_Faders[i];

		if( pFader->bUsed ) {
			pFader->fTime += FLoop_fPreviousLoopSecs;
			fUnit = ( pFader->fTime * pFader->fTimeOOTotal );
			FMATH_CLAMP( fUnit, 0.0f, 1.0f );

			if( pFader->pPos ) { 
				pFader->pEmitter->SetPosition( pFader->pPos );
			}

			pFader->pEmitter->SetVolume( FMATH_FPOT( fUnit, pFader->fVolumeStart, pFader->fVolumeEnd ) );

			if( fUnit >= 1.0f ) {
				if( pFader->bInAndOut ) {
					// Fade in and out
					f32 fTempVol = pFader->fVolumeEnd;

					pFader->bInAndOut = FALSE;

					pFader->fTime = 0.0f;
					pFader->fTimeOOTotal = fmath_Inv( pFader->fTimeTotal - fmath_Inv( pFader->fTimeOOTotal ) );
					pFader->fVolumeEnd = pFader->fVolumeStart;
					pFader->fVolumeStart = fTempVol;

					bAlive = TRUE;
				} else {
					// Normal fade in / out case
					if( m_Faders[i].bKillWhenDone ) {
						m_Faders[i].pEmitter->Destroy();
					}

					m_Faders[i].bUsed = FALSE;
				}
			} else {
				bAlive = TRUE;
			}
		}
	}

	m_bAlive = bAlive;
}

void CEmitterFader::Fade(	CFAudioEmitter *pEmitter,
							const f32 &fStartVol,
							const f32 &fEndVol,
							const f32 &fTimeToFade,
							BOOL bDestroyWhenDone/* = FALSE*/,
							const CFVec3A *pPos/* = NULL*/ ) {
	if( fStartVol < 0.0f ) {
		goto _ExitWithError;
	}

	if( fEndVol < 0.0f ) {
		goto _ExitWithError;
	}

	if( fTimeToFade < 0.0f ) {
		goto _ExitWithError;
	}

	if( !pEmitter ) {
		goto _ExitWithError;
	}

	FaderEntry_t *pFader = _GetFader();

	if( pFader == NULL ) {
		goto _ExitWithError;
	}

	pFader->bUsed = TRUE;
	pFader->bKillWhenDone = bDestroyWhenDone;
	pFader->bInAndOut = FALSE;
	pFader->fVolumeStart = fStartVol;
	pFader->fVolumeEnd = fEndVol;
	pFader->fTime = 0.0f;
	pFader->fTimeTotal = fTimeToFade;
	pFader->fTimeOOTotal = fmath_Inv( fTimeToFade );
	pFader->pEmitter = pEmitter;
	pFader->pEmitter->SetVolume( fStartVol );
	pFader->pPos = pPos;

	m_bAlive = TRUE;

	return;

_ExitWithError:
	if( pEmitter && bDestroyWhenDone ) {
		pEmitter->Destroy();
	}
}

void CEmitterFader::FadeInAndOut(	CFAudioEmitter *pEmitter,
									const f32 &fStartVol,
									const f32 &fEndVol,
									const f32 &fTimeToFade,
									const f32 &fUnitMaxVol, // Unit time to hit max volume at
									BOOL bDestroyWhenDone/* = FALSE*/,
									const CFVec3A *pPos/* = NULL*/ ) {
	if( fStartVol < 0.0f ) {
		goto _ExitWithError;
	}

	if( fEndVol < 0.0f ) {
		goto _ExitWithError;
	}

	if( fTimeToFade < 0.0f ) {
		goto _ExitWithError;
	}

	if( !pEmitter ) {
		goto _ExitWithError;
	}

	if( fUnitMaxVol < 0.0f || fUnitMaxVol > 1.0f ) {
		goto _ExitWithError;
	}

	FaderEntry_t *pFader = _GetFader();

	if( pFader == NULL ) {
		goto _ExitWithError;
	}

	pFader->bUsed = TRUE;
	pFader->bKillWhenDone = bDestroyWhenDone;
	pFader->bInAndOut = TRUE;
	pFader->fVolumeStart = fStartVol;
	pFader->fVolumeEnd = fEndVol;
	pFader->fTime = 0.0f;
	pFader->fTimeTotal = fTimeToFade;
	pFader->fTimeOOTotal = fmath_Inv( fTimeToFade * fUnitMaxVol );
	pFader->pEmitter = pEmitter;
	pFader->pEmitter->SetVolume( fStartVol );
	pFader->pPos = pPos;

	m_bAlive = TRUE;

	return;

_ExitWithError:
	if( pEmitter && bDestroyWhenDone ) {
		pEmitter->Destroy();
	}
}

CEmitterFader::FaderEntry_t *CEmitterFader::_GetFader( void ) {
	for( u32 i = 0; i < _MAX_FADERS; ++i ) {
		if( !m_Faders[i].bUsed ) {
			return &m_Faders[i];
		}
	}

	return NULL;
}


//
//
//
// CSpyVsSpySoundCenter
//
//
//
static const cchar *_apszSounds[ CSpyVsSpySoundCenter::SOUND_NUM_SOUNDS ] = {
	"CraneMoveLoop",
	"CraneMoveStart",
	"CraneMoveStop",
	"CranePinchMiss",
	"CranePinchHit",
	"CraneArmStart",
	"CraneArmLoop",

	"ConveyorLoop",
	"ConveyorSuccessDing",

	"DispenserDing",
	"DispenserWork",

	"GurneyMove",
	"GurneyArmMoveUp",

	"ElevatorDing",

	"LockerOpen",
	"LockerClose",
	"BuildArms",

	"CommandLeft",
	"CommandRight",
	"CommandForward",
	"CommandBackward",
	"CommandRotateRight",
	"CommandRotateLeft",
	"NoCommandLeft",
	"NoCommandRight",
	"NoCommandForward",
	"NoCommandBackward",
	"NoCommandRotateRight",
	"NoCommandRotateLeft",

	"BadBotWarning",
	"DefectiveBot",
	"InspectBot",
	"OpenLockerWarning",
	"ImpostorBot",
	"TestingReady",
	"TestingStart",

	"1_FinalSpy",
	"1_UpForTonight",
	"1_DroidHunt",
	"1_Fun",
	"2_LeaveBox",
	"2_DroidHunt",
	"2_Bash",
	"2_Drink",

	"Scratch",
	"BuildArmSparks",
	"HeadCrane",
	"ProgramLoop",
	"PartClatter",
	"CheckSound",

	"SearchAmbient",
	"ProgramAmbient",

	"DeathMachine",
	"IdleMachine",
	"BuildMachineRumble",
};

CFSoundGroup **CSpyVsSpySoundCenter::m_apGroups = NULL;

BOOL CSpyVsSpySoundCenter::Load( void ) {
	FASSERT( !m_apGroups );

	FResFrame_t Frame = fres_GetFrame();

	m_apGroups = (CFSoundGroup **) fres_AllocAndZero( sizeof( CFSoundGroup *) * CSpyVsSpySoundCenter::SOUND_NUM_SOUNDS );

	if( !m_apGroups ) {
		DEVPRINTF( "CSpyVsSpySoundCenter::Load(): Failed to allocate space for sound group pointers.\n" );
		goto _ExitWithError;
	}

	for( u32 i = 0; i < CSpyVsSpySoundCenter::SOUND_NUM_SOUNDS; ++i ) {
		m_apGroups[ i ] = CFSoundGroup::RegisterGroup( _apszSounds[i] );
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( Frame );

	return FALSE;
}

void CSpyVsSpySoundCenter::Unload( void ) {
	m_apGroups = NULL;
}

//
//
//
// CCam
//
//
//
FCLASS_ALIGN_PREFIX class CCam {
public:
	CCam() { _Reset(); }
	virtual ~CCam() { }

protected:
	virtual void _Reset( void );
	
	void _SetStartToCurrent( void );

public:
	f32 m_fUnitTime;
	f32 m_fStartFOV;
	f32 m_fEndFOV;
	f32 m_fCurrentFOV;

	CFVec3A m_StartPoint;
	CFVec3A m_EndPoint;
	CFVec3A m_CurrentPoint;

	CFQuatA m_EndQuat;
	CFQuatA m_StartQuat;
	CFQuatA m_CurrentQuat;

	CCamManualInfo m_Camera;

	FCLASS_STACKMEM_ALIGN( CCam );
} FCLASS_ALIGN_SUFFIX;

void CCam::_Reset( void ) {
	m_fUnitTime = 0.0f;

	m_fStartFOV = 0.0f;
	m_StartPoint.Zero();
	m_StartQuat.Identity();

	m_fEndFOV = 0.0f;
	m_EndPoint.Zero();
	m_EndQuat.Identity();

	m_fCurrentFOV = 0.0f;
	m_CurrentPoint.Zero();
	m_CurrentQuat.Zero();
}

void CCam::_SetStartToCurrent( void ) {
	m_fStartFOV = m_fCurrentFOV;
	m_StartPoint = m_CurrentPoint;
	m_StartQuat = m_CurrentQuat;
}

//
//
//
// CStage1Cam
//
//
//
#define _CLAW_CAM_TIMESCALE		( 0.5f )
#define _CLAW_CAM_INTERP_DELAY	( 1.0f )
#define _STAGE1_CAM_TIME_DROP_PART ( 16.3f )

FCLASS_ALIGN_PREFIX class CStage1Cam : public CCam {
public:
	CStage1Cam();
	virtual ~CStage1Cam() { }

	FINLINE BOOL IsCraneCamFinished( void ) { FASSERT( m_pCamCraneAnimInst ); return m_pCamCraneAnimInst->GetTotalTime() == m_pCamCraneAnimInst->GetTime(); }
	FINLINE BOOL CanDropPart( void ) { return m_pCamCraneAnimInst->GetTime() >= _STAGE1_CAM_TIME_DROP_PART; }
	FINLINE BOOL IsDropping( void ) { return m_eCamState == CAMERA_STATE_DROPPING; }

	BOOL Load( void );
	void Unload( void );
	void Restore( void );
	void Setup( CBot *pBot, const CFVec3A *pLookPoint );
	void Work( void );

	void StartMoveToEnd( void );

protected:
	virtual void _Reset( void );

protected:
	typedef enum {
		CAMERA_STATE_INTERP_TO_CAM,
		CAMERA_STATE_PLAYER,
		CAMERA_STATE_DROPPING,
	} CameraState_e;
public:
	CBot *m_pBot;
	CameraState_e m_eCamState;

	CFCamAnimInst *m_pCamCraneAnimInst;

	FCLASS_STACKMEM_ALIGN( CStage1Cam );
} FCLASS_ALIGN_SUFFIX;

CStage1Cam::CStage1Cam() {
	_Reset();
	m_pCamCraneAnimInst = NULL;
}

void CStage1Cam::_Reset( void ) {
	CCam::_Reset();

	m_pBot = NULL;
	m_eCamState = CAMERA_STATE_INTERP_TO_CAM;
}

BOOL CStage1Cam::Load( void ) {
	m_pCamCraneAnimInst = CFCamAnimInst::Load( _STAGE1_CRANE_CAMERA );

	if( !m_pCamCraneAnimInst ) {
		DEVPRINTF( "CStage1Cam::Load(): Failed to load camera '%s'.\n", _STAGE1_CRANE_CAMERA );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	Unload();
	return FALSE;
}

void CStage1Cam::Unload( void ) {
	if( m_pCamCraneAnimInst ) {
		fdelete( m_pCamCraneAnimInst );
		m_pCamCraneAnimInst = NULL;
	}

	_Reset();
}

void CStage1Cam::Restore( void ) {
	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( m_pBot->m_nPossessionPlayerIndex ), m_pBot );

	m_pBot = NULL;
	m_eCamState = CAMERA_STATE_INTERP_TO_CAM;

	_Reset();
}

#define _CLAW_CAM_START_FRAME ( 50.0f )

void CStage1Cam::Setup( CBot *pBot, const CFVec3A *pLookPoint ) {
	FASSERT( pBot );
	FASSERT( pLookPoint );
	FASSERT( m_pCamCraneAnimInst );

	m_pBot = pBot;

	CFCamera* pCam = fcamera_GetCameraByIndex( pBot->m_nPossessionPlayerIndex );

	m_fUnitTime = 0.0f;
	m_StartPoint = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
	m_StartQuat.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
	pCam->GetFOV( &m_fStartFOV );

	m_pCamCraneAnimInst->UpdateTime( _CLAW_CAM_START_FRAME * _OO_ANIM_FPS );
	m_pCamCraneAnimInst->ComputeFrame( m_fEndFOV, m_EndQuat, m_EndPoint.v3 );

	m_fCurrentFOV = m_fStartFOV;
	m_CurrentPoint = m_StartPoint;
	m_CurrentQuat = m_StartQuat;

	m_Camera.m_pvecPos = &m_CurrentPoint;
	m_Camera.m_pqQuat = &m_CurrentQuat;
	m_Camera.m_fHalfFOV = m_fCurrentFOV;

	gamecam_SwitchPlayerToManualCamera( PLAYER_CAM( pBot->m_nPossessionPlayerIndex ), &m_Camera );

	m_eCamState = CAMERA_STATE_INTERP_TO_CAM;

	CFAudioEmitter *pEmitter;

	pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_HEAD_CRANE ), TRUE );

	if( pEmitter ) {
		pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
	}

	CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, _STAGE1_CAMERA_STOP_TIME, 0.5f, TRUE );

	m_CurrentQuat.BuildMtx( CSpyVsSpy::m_pCommonData->m_Listener );
	CSpyVsSpy::m_pCommonData->m_Listener.m_vPos = m_CurrentPoint;

	game_SetListenerOrientationCallback( &CSpyVsSpy::ListenerOrientationCallback );
}

void CStage1Cam::Work( void ) {
	f32 fUnitLin;

	switch( m_eCamState ) {
		case CAMERA_STATE_INTERP_TO_CAM:
			m_fUnitTime += ( 0.5f * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fUnitTime, 1.0f );

			fUnitLin = fmath_UnitLinearToSCurve( m_fUnitTime );

			m_pCamCraneAnimInst->DeltaTime( FLoop_fPreviousLoopSecs, TRUE );
			m_pCamCraneAnimInst->ComputeFrame( m_fEndFOV, m_EndQuat, m_EndPoint.v3 );

			CSpyVsSpy::CameraInterp( m_fUnitTime, m_fStartFOV, m_StartPoint, m_StartQuat,
				m_fEndFOV, m_EndPoint, m_EndQuat, 
				&m_fCurrentFOV, &m_CurrentPoint, &m_CurrentQuat );

			m_Camera.m_fHalfFOV = m_fCurrentFOV;

			if( m_pCamCraneAnimInst->GetTime() >= _STAGE1_CAMERA_STOP_TIME ) {
				m_fUnitTime = 0.0f;
				m_eCamState = CAMERA_STATE_PLAYER;
				_SetStartToCurrent();

				m_fEndFOV = m_fCurrentFOV * _STAGE1_FOV_FRAC;
			}
		break;
		case CAMERA_STATE_PLAYER: {
			m_fUnitTime += ( 0.5f * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fUnitTime, 1.0f );

			CSpyVsSpy::CameraInterp( m_fUnitTime, m_fStartFOV, m_StartPoint, m_StartQuat,
				m_fEndFOV, m_EndPoint, m_EndQuat, 
				&m_fCurrentFOV, &m_CurrentPoint, &m_CurrentQuat );

			m_Camera.m_fHalfFOV = m_fCurrentFOV;

			CFVec3A CamToPlayer;

			CamToPlayer.Sub( m_pBot->MtxToWorld()->m_vPos, m_CurrentPoint );

			if( CamToPlayer.MagSq() > FMATH_POS_EPSILON ) {
				CFQuatA PlayerQuat;

				CamToPlayer.Unitize();
				CFMtx43A::m_Temp.UnitMtxFromUnitVec( &CamToPlayer );
				PlayerQuat.BuildQuat( CFMtx43A::m_Temp );
				m_CurrentQuat.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fUnitTime ), m_StartQuat, PlayerQuat );
			}
		}
		break;
		case CAMERA_STATE_DROPPING: {
			m_fUnitTime += ( 0.5f * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fUnitTime, 1.0f );

			m_pCamCraneAnimInst->DeltaTime( FLoop_fPreviousLoopSecs, TRUE );
			m_pCamCraneAnimInst->ComputeFrame( m_fEndFOV, m_EndQuat, m_EndPoint.v3 );

			CSpyVsSpy::CameraInterp( m_fUnitTime, m_fStartFOV, m_StartPoint, m_StartQuat,
				m_fEndFOV, m_EndPoint, m_EndQuat, 
				&m_fCurrentFOV, &m_CurrentPoint, &m_CurrentQuat );

			m_Camera.m_fHalfFOV = m_fCurrentFOV;

			if( m_pCamCraneAnimInst->GetTotalTime() == m_pCamCraneAnimInst->GetTime() ) {
				if( game_GetListenerOrientationCallback() == CSpyVsSpy::ListenerOrientationCallback ) {
					game_SetListenerOrientationCallback( NULL );
				}
			}
		}
		break;
	}

	m_CurrentQuat.BuildMtx( CSpyVsSpy::m_pCommonData->m_Listener );
	CSpyVsSpy::m_pCommonData->m_Listener.m_vPos = m_CurrentPoint;
}

void CStage1Cam::StartMoveToEnd( void ) {
	m_eCamState = CAMERA_STATE_DROPPING;
	m_fUnitTime = 0.0f;

	_SetStartToCurrent();

	CSpyVsSpy::GetGlitch()->ShakeCamera( _STAGE1_SHAKE_UNIT_INTENSITY, _STAGE1_SHAKE_DURATION );

	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_HEAD_CRANE ), TRUE );

	if( pEmitter ) {
		pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		CEmitterFader::Fade( pEmitter, 1.0f, 1.0f, m_pCamCraneAnimInst->GetTotalTime() - _STAGE1_CAMERA_STOP_TIME, TRUE );
	}
}


//
//
//
// CConveyorPart
//
//
//
FCLASS_ALIGN_PREFIX class CConveyorPart {
public:
	CConveyorPart()				{ }
	virtual ~CConveyorPart()	{ }

	void Reset( void );

public:
	BOOL8 m_bAlive;
	BOOL8 _PAD[3];

	f32 m_fUnitLerp;
	CFWorldMeshItem *m_pMesh;

	FCLASS_STACKMEM_ALIGN( CConveyorPart );
} FCLASS_ALIGN_SUFFIX;

void CConveyorPart::Reset( void ) {
	m_bAlive = FALSE;
	
	m_fUnitLerp = 0.0f;

	m_pMesh = NULL;
}

//
//
//
// CConveyor
//
//
//
FCLASS_ALIGN_PREFIX class CConveyor {
public:
	CConveyor()				{ _ClearDataMembers(); }
	virtual ~CConveyor()	{ }

	void Reset( void );
	BOOL Setup( cchar *pszSplineName, CFMeshPool *pMeshPool );
	void Restore( void );
	void Cleanup( void );
	void Work( void );
	
	void PlacePartOnConveyor( const u32 &uGlitchPart = -1 );

protected:
	void _ClearDataMembers( void );

	BOOL m_bPullGlitch;
	u32 m_uPartType;

	CFMeshPool *m_pMeshPool; // Pool we use, don't own it

	CESpline *m_pSpline;
	const CFVec3A *m_pPtStart;
	const CFVec3A *m_pPtEnd;

	CFAudioEmitter *m_apEmitter[ _STAGE1_NUM_EMITTERS ];

	CConveyorPart m_aParts[_STAGE1_PART_MAX];

	FCLASS_STACKMEM_ALIGN( CConveyorPart );
} FCLASS_ALIGN_SUFFIX;

void CConveyor::Reset( void ) {
	for( u32 i = 0; i < _STAGE1_NUM_EMITTERS; ++i ) {
		if( m_apEmitter[ i ] ) {
			m_apEmitter[ i ]->Stop( FALSE );
			m_apEmitter[ i ]->Destroy();
			m_apEmitter[ i ] = NULL;
		}
	}

	_ClearDataMembers();
}

BOOL CConveyor::Setup( cchar *pszSplineName, CFMeshPool *pMeshPool ) {
	FASSERT( pszSplineName );
	FASSERT( pMeshPool );

	CEntity *pEntity;
	CFVec3A Lerp;
	f32 fVal;

	Reset();

	// Setup the conveyor splines
	pEntity = CEntity::FindInWorld( pszSplineName );
	if( pEntity == NULL ) {
		DEVPRINTF( "CConveyor::_SetupConveyor(): Failed to find entity '%s'.\n", pszSplineName );
		goto _ExitWithError;
	}

	if( !( pEntity->TypeBits() & ENTITY_BIT_SPLINE ) ) {
		DEVPRINTF( "CConveyor::_SetupConveyor(): Entity '%s' was not of expected type.\n", pszSplineName );
		goto _ExitWithError;
	}

	m_pSpline = (CESpline *) pEntity;

	if( m_pSpline->PointCount() < 2 ) {
		DEVPRINTF( "CConveyor::_SetupConveyor(): Too few points in spline '%s'.  Need at least 2.\n", pszSplineName );
		goto _ExitWithError;
	}

	m_pMeshPool = pMeshPool;

	m_pPtStart = &m_pSpline->PointArray()[0];
	m_pPtEnd = &m_pSpline->PointArray()[1];

	for( u32 i = 0; i < _STAGE1_NUM_EMITTERS; ++i ) {
		fVal = (f32) ( i + 1 ) * fmath_Inv( (f32) ( _STAGE1_NUM_EMITTERS + 1 ) );
        Lerp.Lerp( fVal, *m_pPtStart, *m_pPtEnd );

		m_apEmitter[ i ] = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CONVEYOR_LOOP ), FALSE, &Lerp );
	
		if( m_apEmitter[ i ] ) {
			m_apEmitter[ i ]->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		}
	}

	m_bPullGlitch = FALSE;
	m_uPartType = -1;

	return TRUE;

_ExitWithError:
	Reset();

	return FALSE;
}

void CConveyor::Restore( void ) {
	CFVec3A Lerp;
	f32 fVal;

	for( u32 i = 0; i < _STAGE1_NUM_EMITTERS; ++i ) {
		fVal = (f32) ( i + 1 ) * fmath_Inv( (f32) ( _STAGE1_NUM_EMITTERS + 1 ) );
        Lerp.Lerp( fVal, *m_pPtStart, *m_pPtEnd );

		if( m_apEmitter[ i ] ) {
			m_apEmitter[ i ]->Stop( FALSE );
			m_apEmitter[ i ]->Destroy();
		}

		m_apEmitter[ i ] = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CONVEYOR_LOOP ), FALSE, &Lerp );
	
		if( m_apEmitter[ i ] ) {
			m_apEmitter[ i ]->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		}
	}

	for( u32 i = 0; i < _STAGE1_PART_MAX; ++i ) {
		if( m_aParts[i].m_bAlive ) {
            m_pMeshPool->ReturnToFreePool( m_aParts[i].m_pMesh );
			m_aParts[i].m_bAlive = FALSE;
		}
	}

	m_bPullGlitch = FALSE;
	m_uPartType = -1;
}

void CConveyor::Cleanup( void ) {
	for( u32 i = 0; i < _STAGE1_NUM_EMITTERS; ++i ) {
		if( m_apEmitter[ i ] ) {
			m_apEmitter[ i ]->Stop( FALSE );
			m_apEmitter[ i ]->Destroy();
		}
	}
}

void CConveyor::Work( void ) {
	CConveyorPart *pPart;
	CFVec3A LerpPoint;

	for( u32 i = 0; i < _STAGE1_PART_MAX; ++i ) {
		if( m_aParts[i].m_bAlive ) {
			pPart = &m_aParts[i];

			FASSERT( pPart->m_pMesh );

			pPart->m_fUnitLerp += ( _STAGE1_CONVEYOR_PART_MOVE_TIME * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( pPart->m_fUnitLerp, 1.0f );

			if( pPart->m_fUnitLerp == 1.0f ) {
				if( !m_bPullGlitch ) {
					m_pMeshPool->ReturnToFreePool( pPart->m_pMesh );
				}
				pPart->m_bAlive = FALSE;
			} else {
				LerpPoint.Lerp( pPart->m_fUnitLerp, *m_pPtStart, *m_pPtEnd );

				if( !m_bPullGlitch ) {
					CFMtx43A::m_Temp.Identity();
					CFMtx43A::m_Temp.m_vPos = LerpPoint;
		
					pPart->m_pMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, 1.0f );
					pPart->m_pMesh->UpdateTracker();
				} else {
					switch( m_uPartType ) {
						case MACHINE_HEAD:
							CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos = LerpPoint;
							CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos.Add( CFVec3A::m_UnitAxisY );
						break;
						case MACHINE_TORSO:
							CSpyVsSpy::m_pCommonData->m_TorsoMtx.m_vPos = LerpPoint;
							CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos.Add( CFVec3A::m_UnitAxisY );
						break;
						case MACHINE_LEGS:
							CSpyVsSpy::m_pCommonData->m_LegsMtx.m_vPos = LerpPoint;
							CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos.Add( CFVec3A::m_UnitAxisY );
						break;
					}
				}
			}
		}
	}
}

void CConveyor::PlacePartOnConveyor( const u32 &uGlitchPart ) {
	if( uGlitchPart != -1 ) {
		m_uPartType = uGlitchPart;
		m_bPullGlitch = TRUE;
	}

	u32 i;
	for( i = 0; i < _STAGE1_PART_MAX; ++i ) {
		// Find a free spot
		if( !m_aParts[i].m_bAlive ) {
			CConveyorPart *pPart = &m_aParts[i];

			pPart->m_bAlive = TRUE;
			pPart->m_fUnitLerp = 0.0f;

			if( !m_bPullGlitch ) {
				pPart->m_pMesh = m_pMeshPool->GetFromFreePool();
			
				FASSERT( pPart->m_pMesh );

				// Put it in the world
				CFMtx43A::m_Temp.Identity();
				CFMtx43A::m_Temp.m_vPos = *m_pPtStart;

				pPart->m_pMesh->AddToWorld();
				pPart->m_pMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, 1.0f );
				pPart->m_pMesh->UpdateTracker();
			}

			break;
		}
	}

	FASSERT( i < _STAGE1_PART_MAX );
}

void CConveyor::_ClearDataMembers( void ) {
	u32 i;

	m_pMeshPool = NULL;
	m_pSpline = NULL;

	for( i = 0; i < _STAGE1_PART_MAX; ++i ) {
		m_aParts[i].Reset();
	}

	for( i = 0; i < _STAGE1_NUM_EMITTERS; ++i ) {
		m_apEmitter[ i ] = NULL;
	}

	m_pPtStart = NULL;
	m_pPtEnd = NULL;

	m_bPullGlitch = FALSE;
	m_uPartType = -1;
}

//
//
//
// CMachine
//
//
//
FCLASS_ALIGN_PREFIX class CMachine {
public:
	CMachine()			{ }
	virtual ~CMachine() { }

	FINLINE BOOL HasPartInArm( void ) { return m_bHavePart; } //{ return m_pPullPart != NULL; }
	FINLINE BOOL JustGrabbedHead( void ) { return m_bGotHead; }
	FINLINE BOOL GetReadyToDrop( void ) { return m_bReadyToDrop && ( m_fAnimTime >= m_fAnimTimes[m_uType][m_uAnimTimeIndex] ); }
	FINLINE CMeshEntity *GetCrane( void ) { return m_pCrane; }

	void Reset( void );
	void Unload( void );
	void Restore( void );
	BOOL Setup( u32 uType, cchar *pszCraneName, cchar *pszDispenserName, const CFVec3A &DestPoint, cchar *pszConveyorSplineName, cchar *pszConveyorMeshName );
	void Cleanup( void );

	void Work( BOOL bCanDrop, BOOL bPartDispenserDestroyed );

	void StartMoveToEnd( void );

protected:
	BOOL _SetupMeshPool( cchar *pszMeshName );

	void _DispenserEnable( BOOL bEnable );

	void _StateSetGrabPart( void );
	void _StateSetMoveUp( void );
	void _StateSetMoveOver( void );
	void _StateSetPlacePart( void );
	void _StateSetMoveBack( void );

	void _StateHandleGrabPart( void );
	void _StateHandleLift( void );
	void _StateHandleMove( BOOL bCanDrop );
	void _StateHandlePlacePart( void );
	void _StateHandleMoveBack( void );

protected:
	typedef enum {
		MACHINE_STATE_DEAD,
		MACHINE_STATE_GRAB_PART,
		MACHINE_STATE_MOVE_UP,
		MACHINE_STATE_MOVE_OVER,
		MACHINE_STATE_PLACE_PART,
		MACHINE_START_MOVE_BACK,
	} MachineState_e;

	enum {
		MACHINE_ANIM_GRAB_TIME = 0,
		MACHINE_ANIM_UP_TIME,
		MACHINE_ANIM_END_MOVE,
		MACHINE_ANIM_PART_DOWN,
		MACHINE_ANIM_NUM_ANIMS,
	};

	static f32 m_fAnimTimes[MACHINE_COUNT][MACHINE_ANIM_NUM_ANIMS];
	static FParticle_DefHandle_t m_hSmokeHandle;
	static FExplosion_GroupHandle_t m_hExplosionHandle;
	static FExplosion_GroupHandle_t m_hPartExplosionHandle;

	u32 m_uType;
	MachineState_e m_eState;

	BOOL8 m_bReadyToDrop;
	BOOL8 m_bDispenserDestroyed;
	BOOL8 m_bHavePart; // Use this bool so that we can treat the Glitch head as a part
	BOOL8 m_bGotHead;
	BOOL m_bDontPause;

	f32 m_fAnimTime;
	f32 m_fAnimTotalTime;
	f32 m_fAnimDir;
	f32 m_fUnitInterp;
	
	u32 m_uAnimTimeIndex;

	CFVec3A *m_pCraneAttachBone;

	CFMeshPool *m_pMeshPool;
	CMeshEntity *m_pCrane;
	CMeshEntity *m_pDispenser;
	CFWorldMeshItem *m_pPullPart;

	FMeshTexLayerHandle_t m_hTrackTexLayer;
	CFAudioEmitter *m_pEmitter;

	CConveyor m_Conveyor;

	FParticle_EmitterHandle_t m_hEmitterHandle;
	
	FCLASS_STACKMEM_ALIGN( CMachine );
} FCLASS_ALIGN_SUFFIX;

f32 CMachine::m_fAnimTimes[MACHINE_COUNT][MACHINE_ANIM_NUM_ANIMS] = {
	// Head
	{ 
		59.0f  * _OO_ANIM_FPS, 
		98.0f  * _OO_ANIM_FPS, 
		490.0f * _OO_ANIM_FPS,
		519.0f * _OO_ANIM_FPS, 
	},

	// Torso
	{ 
		59.0f  * _OO_ANIM_FPS, 
		98.0f  * _OO_ANIM_FPS, 
		150.0f * _OO_ANIM_FPS,
		204.0f * _OO_ANIM_FPS, 
	},

	// Legs
	{ 
		59.0f  * _OO_ANIM_FPS, 
		98.0f  * _OO_ANIM_FPS, 
		150.0f * _OO_ANIM_FPS,
		204.0f * _OO_ANIM_FPS, 
	},
};

FParticle_DefHandle_t CMachine::m_hSmokeHandle = FPARTICLE_INVALID_HANDLE;
FExplosion_GroupHandle_t CMachine::m_hExplosionHandle = FEXPLOSION_INVALID_HANDLE;
FExplosion_GroupHandle_t CMachine::m_hPartExplosionHandle = FEXPLOSION_INVALID_HANDLE;

void CMachine::Reset( void ) {
	m_uType = MACHINE_NONE;

	m_bReadyToDrop = FALSE;
	m_bDispenserDestroyed = FALSE;
	m_bHavePart = FALSE;
	m_bGotHead = FALSE;
	m_bDontPause = FALSE;

	m_eState = MACHINE_STATE_DEAD;

	m_pMeshPool = NULL;

	m_fUnitInterp = 0.0f;
	m_pCraneAttachBone = NULL;
	m_pPullPart = NULL;

	m_pCrane = NULL;
	
	m_pDispenser = NULL;
	m_hTrackTexLayer = NULL;

	m_fAnimTime = 0.0f;
	m_fAnimTotalTime = 0.0f;
	m_fAnimDir = 1.0f;
	m_uAnimTimeIndex = MACHINE_ANIM_GRAB_TIME;

	m_pEmitter = NULL;

	m_Conveyor.Reset();

	m_hEmitterHandle = FPARTICLE_INVALID_HANDLE;
}

void CMachine::Unload( void ) {
	if( m_pMeshPool != NULL ) {
		CFMeshPool::DestroyPool( m_pMeshPool );
	}
	m_pMeshPool = NULL;

	if( m_pEmitter ) {
		m_pEmitter->Destroy();
		m_pEmitter = NULL;
	}

	if( m_hEmitterHandle != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hEmitterHandle );
	}

	Reset();

	m_hSmokeHandle = FPARTICLE_INVALID_HANDLE;
	m_hExplosionHandle = FEXPLOSION_INVALID_HANDLE;
	m_hPartExplosionHandle = FEXPLOSION_INVALID_HANDLE;
}

void CMachine::Restore( void ) {
	CFSphere CollSphere;

	m_fAnimTime = 0.0f;
	m_fAnimDir = 1.0f;

	m_bReadyToDrop = FALSE;
	m_bDispenserDestroyed = FALSE;
	m_bHavePart = FALSE;
	m_bGotHead = FALSE;
	m_bDontPause = FALSE;

	m_fUnitInterp = 0.0f;
	m_pPullPart = NULL;
	
	m_pCrane->UserAnim_SetClampMode( TRUE );
	m_pCrane->UserAnim_UpdateUnitTime( 0.0f );
	m_pCrane->UserAnim_Pause( FALSE );

	// Setup the dispenser
	_DispenserEnable( TRUE );

	// Play its work sound
	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DISPENSER_WORK ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );

	_StateSetGrabPart();

	// Make the sphere larger since it is to small
	CollSphere.m_Pos.x = m_pCrane->MtxToWorld()->m_vPos.x;
	CollSphere.m_Pos.y = m_pCrane->MtxToWorld()->m_vPos.y;
	CollSphere.m_Pos.z = m_pCrane->MtxToWorld()->m_vPos.z;
	CollSphere.m_fRadius = _BIG_RADIUS;
	m_pCrane->GetMeshInst()->MoveTracker( CollSphere );

	m_Conveyor.Restore();

	if( m_hEmitterHandle != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hEmitterHandle );
	}
}

BOOL CMachine::Setup( u32 uType, cchar *pszCraneName, cchar *pszDispenserName, const CFVec3A &DestPoint, cchar *pszConveyorSplineName, cchar *pszConveyorMeshName ) {
	FASSERT( uType != MACHINE_NONE );

	CEntity *pEntity;
	CFSphere CollSphere;
	s32 sBoneIndex;

	Reset();

	m_uType = uType;

	pEntity = CEntity::FindInWorld( pszCraneName );
	if( pEntity == NULL ) {
		DEVPRINTF( "CMachine::Setup(): Failed to find entity '%s'.\n", pszCraneName );
		goto _ExitWithError;
	}

	if( !( pEntity->TypeBits() & ENTITY_BIT_MESHENTITY ) ) {
		DEVPRINTF( "CMachine::Setup(): Entity '%s' was not the right type.\n", pszCraneName );
		goto _ExitWithError;
	}

	m_pCrane = (CMeshEntity *) pEntity;

	sBoneIndex = m_pCrane->GetMeshInst()->FindBone( _STAGE1_CRANE_BONE_NAME );

	if( sBoneIndex == -1 ) {
		DEVPRINTF( "CMachine::Setup():  Unable to find bone '%s' in '%s'.\n", _STAGE1_CRANE_BONE_NAME, pszCraneName );
		goto _ExitWithError;
	}

	m_pCraneAttachBone = &m_pCrane->GetMeshInst()->GetBoneMtxPalette()[sBoneIndex]->m_vPos;
	m_pCrane->UserAnim_SetSpeedMult( 1.0f );
	m_pCrane->SetCollisionFlag( FALSE ); // Don't want to collide with them, goes whacky

	m_fAnimTotalTime = m_pCrane->UserAnim_GetCurrentInst()->GetTotalTime();


	pEntity = CEntity::FindInWorld( pszDispenserName );
	if( pEntity == NULL ) {
		DEVPRINTF( "CMachine::Setup(): Failed to find entity '%s'.\n", pszDispenserName );
		goto _ExitWithError;
	}

	if( !( pEntity->TypeBits() & ENTITY_BIT_MESHENTITY ) ) {
		DEVPRINTF( "CMachine::Setup(): Entity '%s' was not the right type.\n", pszDispenserName );
		goto _ExitWithError;
	}

	m_pDispenser = (CMeshEntity *) pEntity;
	m_hTrackTexLayer = m_pDispenser->GetMesh()->GetTexLayerHandle( _STAGE1_DISPENSER_TEXTURE_ID );

	if( m_hTrackTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) {
		DEVPRINTF( "CMachine::Setup():  Error getting tex layer handle for track, ID %d\n", _STAGE1_DISPENSER_TEXTURE_ID );
	} else {
		m_pDispenser->GetMesh()->AnimTC_ApplyToShader( m_hTrackTexLayer, TRUE );
	}

	if( m_hSmokeHandle == FPARTICLE_INVALID_HANDLE ) {
		m_hSmokeHandle = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _STAGE1_DISPENSER_SMOKE );

		if( m_hSmokeHandle == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CMachine::Setup(): Failed to load particle '%s'.\n", _STAGE1_DISPENSER_SMOKE );
			goto _ExitWithError;
		}
	}

	if( m_hExplosionHandle == FEXPLOSION_INVALID_HANDLE ) {
		m_hExplosionHandle = CExplosion2::GetExplosionGroup( _STAGE1_DISPENSER_EXPLOSION );

		if( m_hExplosionHandle == FEXPLOSION_INVALID_HANDLE ) {
			DEVPRINTF( "CMachine::Setup(): Failed to find explosion group '%s'.\n", _STAGE1_DISPENSER_EXPLOSION );
			goto _ExitWithError;
		}
	}

	if( m_hPartExplosionHandle == FEXPLOSION_INVALID_HANDLE ) {
		m_hPartExplosionHandle = CExplosion2::GetExplosionGroup( _STAGE1_PART_EXPLOSION );

		if( m_hPartExplosionHandle == FEXPLOSION_INVALID_HANDLE ) {
			DEVPRINTF( "CMachine::Setup(): Failed to find explosion group '%s'.\n", _STAGE1_PART_EXPLOSION );
			goto _ExitWithError;
		}
	}

	// We will re-start on our own
	m_pCrane->UserAnim_SetClampMode( TRUE );
	m_pCrane->UserAnim_UpdateUnitTime( 0.0f );
	m_pCrane->UserAnim_Pause( FALSE );

	// Setup the dispenser
	_DispenserEnable( TRUE );
	
	// Play its work sound
	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DISPENSER_WORK ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );

	_StateSetGrabPart();

	// Make the sphere larger since it is to small
	CollSphere.m_Pos.x = m_pCrane->MtxToWorld()->m_vPos.x;
	CollSphere.m_Pos.y = m_pCrane->MtxToWorld()->m_vPos.y;
	CollSphere.m_Pos.z = m_pCrane->MtxToWorld()->m_vPos.z;
	CollSphere.m_fRadius = _BIG_RADIUS;
	m_pCrane->GetMeshInst()->MoveTracker( CollSphere );

	if( !_SetupMeshPool( pszConveyorMeshName ) ) {
		goto _ExitWithError;
	}

	if( !m_Conveyor.Setup( pszConveyorSplineName, m_pMeshPool ) ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	Reset();

	return FALSE;
}

void CMachine::Cleanup( void ) {
	m_Conveyor.Cleanup();
}

void CMachine::Work( BOOL bCanDrop, BOOL bPartDispenserDestroyed ) {
	if( m_eState == MACHINE_STATE_DEAD ) {
        // Don't do anything right now
		return;
	}

	m_fAnimTime = m_pCrane->UserAnim_GetCurrentInst()->GetTime();

	if( !m_bDispenserDestroyed && bPartDispenserDestroyed ) {
		CFVec3A Position;
		FExplosionSpawnParams_t SpawnParams;
		FExplosion_SpawnerHandle_t hSpawner;

		m_bDispenserDestroyed = bPartDispenserDestroyed;

		_DispenserEnable( FALSE );

		// If we have a part when we got destroyed, just drop it
		if( m_pPullPart ) {
			//_StateSetPlacePart();
            m_bReadyToDrop = FALSE;
			m_bHavePart = FALSE;

			_StateSetMoveBack();

			m_pMeshPool->ReturnToFreePool( m_pPullPart );
			m_pPullPart = NULL;

			SpawnParams.InitToDefaults();
			SpawnParams.Pos_WS = *m_pCraneAttachBone;

			hSpawner = CExplosion2::GetExplosionSpawner();

			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				CExplosion2::SpawnExplosion( hSpawner, m_hPartExplosionHandle, &SpawnParams );
			}
		}

		Position = CFVec3A::m_UnitAxisY;
		Position.Mul( 2.0f );
		Position.Add( m_pDispenser->MtxToWorld()->m_vPos );

		m_hEmitterHandle = fparticle_SpawnEmitter( m_hSmokeHandle, Position.v3 );

		SpawnParams.InitToDefaults();
		SpawnParams.Pos_WS = Position;

		hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			CExplosion2::SpawnExplosion( hSpawner, m_hExplosionHandle, &SpawnParams );
		}
	}

	m_bGotHead = FALSE;

	switch( m_eState ) {
		case MACHINE_STATE_GRAB_PART:
			_StateHandleGrabPart();
		break;
		case MACHINE_STATE_MOVE_UP:
			_StateHandleLift();
		break;
		case MACHINE_STATE_MOVE_OVER:
			_StateHandleMove( bCanDrop );
		break;
		case MACHINE_STATE_PLACE_PART:
			_StateHandlePlacePart();
		break;
		case MACHINE_START_MOVE_BACK:
			_StateHandleMoveBack();
		break;
	}

	if( m_pEmitter ) {
		m_pEmitter->SetPosition( m_pCraneAttachBone );
	}

	// Move the part
	if( m_pPullPart || m_bHavePart ) {
		if( m_pPullPart ) {
			CFMtx43A::m_Temp.Identity();
			CFMtx43A::m_Temp.m_vPos = *m_pCraneAttachBone;

//			if( m_uType == MACHINE_LEGS || m_uType == MACHINE_TORSO ) {
				CFMtx43A::m_Temp = FMath_aRotMtx43A[ FMTX_ROTTYPE_180Y ];
				CFMtx43A::m_Temp.m_vPos = *m_pCraneAttachBone;
				CFMtx43A::m_Temp.m_vPos.Add( m_pPullPart->m_BoundSphere_MS.m_Pos );
//			}

			
			m_pPullPart->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, 1.0f );
			m_pPullPart->UpdateTracker();
		} else if( m_bHavePart ) {
			CFSphere CollSphere;
			CFVec3A Temp;
			CFQuatA Quat;
			CFQuatA Identity;

			Identity.Identity();

			m_fUnitInterp += ( 4.0f * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fUnitInterp, 1.0f );

			CollSphere.m_Pos.x = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.x;
			CollSphere.m_Pos.y = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.y;
			CollSphere.m_Pos.z = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.z;
			CollSphere.m_fRadius = _STAGE1_BIG_GLITCH_RADIUS;

			CSpyVsSpy::GetGlitch()->GetMesh()->MoveTracker( CollSphere );

			if( m_uType == MACHINE_HEAD ) {
				Identity.BuildQuat( FMath_aRotMtx43A[ FMTX_ROTTYPE_180Y ] );

				Temp.Lerp( m_fUnitInterp, CSpyVsSpy::m_pCommonData->m_HeadStartPos, *m_pCraneAttachBone );
				Quat.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fUnitInterp ), CSpyVsSpy::m_pCommonData->m_HeadStartQuat, Identity );

				Quat.BuildMtx( CSpyVsSpy::m_pCommonData->m_HeadMtx );
				CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos = Temp;

				CSpyVsSpy::m_pCommonData->m_bHaveHead = TRUE;
			} else if( m_uType == MACHINE_TORSO ) {
				Temp.Lerp( m_fUnitInterp, CSpyVsSpy::m_pCommonData->m_TorsoStartPos, *m_pCraneAttachBone );
				Quat.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fUnitInterp ), CSpyVsSpy::m_pCommonData->m_TorsoStartQuat, Identity );

				Quat.BuildMtx( CSpyVsSpy::m_pCommonData->m_TorsoMtx );
				CSpyVsSpy::m_pCommonData->m_TorsoMtx.m_vPos = Temp;

				CSpyVsSpy::m_pCommonData->m_bHaveTorso = TRUE;
			} else if( m_uType == MACHINE_LEGS ) {
				Temp.Lerp( m_fUnitInterp, CSpyVsSpy::m_pCommonData->m_LegsStartPos, *m_pCraneAttachBone );
				Quat.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fUnitInterp ), CSpyVsSpy::m_pCommonData->m_LegsStartQuat, Identity );

				Quat.BuildMtx( CSpyVsSpy::m_pCommonData->m_LegsMtx );
				CSpyVsSpy::m_pCommonData->m_LegsMtx.m_vPos = Temp;

				CSpyVsSpy::m_pCommonData->m_bHaveLegs = TRUE;
			}
		}
	}

	if( m_pDispenser->UserAnim_GetCurrentInst()->GetUnitTime() >= 1.0f && !m_pDispenser->UserAnim_IsPaused() ) {
		CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DISPENSER_DING ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );
		m_pDispenser->UserAnim_Pause( TRUE );
	}

	m_Conveyor.Work();

	// So the head machine will stay in view
	if( m_uType == MACHINE_HEAD ) {
		CFSphere CollSphere;

		CollSphere.m_Pos.x = m_pCrane->MtxToWorld()->m_vPos.x;
		CollSphere.m_Pos.y = m_pCrane->MtxToWorld()->m_vPos.y;
		CollSphere.m_Pos.z = m_pCrane->MtxToWorld()->m_vPos.z;
		CollSphere.m_fRadius = _BIG_RADIUS;
		m_pCrane->GetMeshInst()->MoveTracker( CollSphere );
	}
}

void CMachine::StartMoveToEnd( void ) {
//	m_pCrane->UserAnim_Pause( FALSE ); // So we don't run away.
//	m_eState = MACHINE_STATE_MOVE_OVER;
//	m_bDontPause = TRUE;
	m_pCrane->RemoveFromWorld();
}

BOOL CMachine::_SetupMeshPool( cchar *pszMeshName ) {
	u32 i;
	FMesh_t *pMesh;
	FMeshInit_t  MeshInit;
	CFWorldMeshItem *pWorldMesh;
	CFWorldMeshItem **paMeshes = NULL;
	FResFrame_t Frame = fres_GetFrame();
	FMemFrame_t FrameMem = fmem_GetFrame();

	// Try to create the mesh pool
	m_pMeshPool = CFMeshPool::CreatePool( _STAGE1_PART_MAX );

	if( m_pMeshPool == NULL ) {
		DEVPRINTF( "CMachine::_SetupMeshPool(): Unable to create mesh pool.\n" );
		goto _ExitWithError;
	}

	// Try to load the mesh 
	pMesh = (FMesh_t *) fresload_Load( FMESH_RESTYPE, pszMeshName );

	if( pMesh == NULL ) {
		DEVPRINTF( "CMachine::_SetupMeshPool(): Could not find mesh '%s'\n", pszMeshName );
		goto _ExitWithError;
	}

	// Array of temp mesh pointers
	paMeshes = (CFWorldMeshItem **) fmem_Alloc( sizeof( CFWorldMeshItem * ) * _STAGE1_PART_MAX );

	if( paMeshes == NULL ) {
		DEVPRINTF( "CMachine::_SetupMeshPool(): Not enough memory for array of CFWorldMeshes.\n" );
		goto _ExitWithError;
	}

	MeshInit.pMesh		= pMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist	= FMATH_MAX_FLOAT;
	MeshInit.Mtx.Identity();

	for( i = 0; i < _STAGE1_PART_MAX; ++i ) {
		pWorldMesh = m_pMeshPool->GetFromFreePool();

		// We should ALWAYS have a valid item
		FASSERT( pWorldMesh );

		pWorldMesh->Init( &MeshInit );
		pWorldMesh->m_nUser = MESHTYPES_UNKNOWN;//MESHTYPES_ENTITY;
		pWorldMesh->m_pUser = NULL;//this;
		pWorldMesh->SetUserTypeBits( 0 );
		pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_DONT_DRAW;
		pWorldMesh->m_nFlags |= FMESHINST_FLAG_NOCOLLIDE;
		pWorldMesh->SetCollisionFlag( FALSE );
		pWorldMesh->SetLineOfSightFlag( FALSE );
		pWorldMesh->RemoveFromWorld();

		paMeshes[i] = pWorldMesh;
	}

	for( i = 0; i < _STAGE1_PART_MAX; ++i ) {
		m_pMeshPool->ReturnToFreePool( paMeshes[ i ] );
	}

	fmem_ReleaseFrame( FrameMem );

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( Frame );
	fmem_ReleaseFrame( FrameMem );

	return FALSE;
}

void CMachine::_DispenserEnable( BOOL bEnable ) {
	// If the system is dead and we try to enable, just ignore it
	if( m_bDispenserDestroyed && bEnable ) {
		return;
	}

	m_pDispenser->UserAnim_SetSpeedMult( 1.0f );
	m_pDispenser->UserAnim_SetClampMode( TRUE );
	m_pDispenser->UserAnim_UpdateUnitTime( 0.0f );

	if( bEnable ) {
		m_pDispenser->UserAnim_Pause( FALSE );
		m_pDispenser->GetMesh()->AnimTC_ApplyToShader( m_hTrackTexLayer, TRUE );
	} else {
		m_pDispenser->UserAnim_Pause( TRUE );
		m_pDispenser->GetMesh()->AnimTC_ApplyToShader( m_hTrackTexLayer, FALSE );
	}
}

void CMachine::_StateSetGrabPart( void ) {
	m_fAnimDir = 1.0f;
	m_bReadyToDrop = FALSE;
	m_eState = MACHINE_STATE_GRAB_PART;
	m_uAnimTimeIndex = MACHINE_ANIM_GRAB_TIME;
	
	m_pCrane->UserAnim_SetSpeedMult( 1.0f );

	// Play the crane start sound
	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_ARM_START ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

	// Start the crane loop
	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_ARM_LOOP ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );
	CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, m_fAnimTimes[ m_uType ][ m_uAnimTimeIndex ], 0.5f, TRUE );
}

void CMachine::_StateSetMoveUp( void ) {
	u32 uPrevIndex = m_uAnimTimeIndex;
	f32 fTime;

	m_eState = MACHINE_STATE_MOVE_UP;
	m_uAnimTimeIndex = MACHINE_ANIM_UP_TIME;
	m_fAnimDir = 1.0f;

	fTime = m_fAnimTimes[ m_uType ][ m_uAnimTimeIndex ] - m_fAnimTimes[ m_uType ][ uPrevIndex ];

	// Start the crane loop
	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_ARM_LOOP ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

	if( pEmitter ) {
		pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, fTime, 0.5f, TRUE );
	}
}

void CMachine::_StateSetMoveOver( void ) {
	u32 uPrevIndex = m_uAnimTimeIndex;
	f32 fTime;

	m_fAnimDir = 1.0f;
	m_eState = MACHINE_STATE_MOVE_OVER;
	m_uAnimTimeIndex = MACHINE_ANIM_END_MOVE;

	fTime = m_fAnimTimes[ m_uType ][ m_uAnimTimeIndex ] - m_fAnimTimes[ m_uType ][ uPrevIndex ];

	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_START ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

	// Start the crane loop
	if( m_uType != MACHINE_HEAD ) {
		CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_LOOP );

		CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pCrane->MtxToWorld()->m_vPos );

		if( pEmitter ) {
			pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
			CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, fTime, 0.5f, TRUE );
		}
	} else {
		if( m_bHavePart && m_pPullPart ) {
			CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_LOOP );

			if( m_pEmitter ) {
				m_pEmitter->Destroy();
				m_pEmitter = NULL;
			}

			m_pEmitter = CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, m_pCraneAttachBone );
			CEmitterFader::Fade( m_pEmitter, 0.0f, 1.0f, 2.0f, FALSE, m_pCraneAttachBone );
		}
	}
}


void CMachine::_StateSetPlacePart( void ) {
	u32 uPrevIndex = m_uAnimTimeIndex;
	f32 fTime;

	m_fAnimDir = 1.0f;
	m_uAnimTimeIndex = MACHINE_ANIM_PART_DOWN;
	m_eState = MACHINE_STATE_PLACE_PART;

	fTime = m_fAnimTimes[ m_uType ][ m_uAnimTimeIndex ] - m_fAnimTimes[ m_uType ][ uPrevIndex ];

	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_STOP ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

	// Start the crane loop
	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_ARM_LOOP ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

	if( pEmitter ) {
		pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, fTime, 0.5f, TRUE );
	}
}

void CMachine::_StateSetMoveBack( void ) {
	u32 uPrevIndex = m_uAnimTimeIndex;
	f32 fTime;

	m_eState = MACHINE_START_MOVE_BACK;
	m_fAnimDir = 1.0f;

	fTime = m_fAnimTotalTime - m_fAnimTimes[ m_uType ][ uPrevIndex ];

	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_START ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

	// Start the crane loop
	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_LOOP ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

	if( pEmitter ) {
		pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, fTime, 0.5f, TRUE );
	}
}

void CMachine::_StateHandleGrabPart( void ) {
	if( m_fAnimDir > 0.0f ) {
		if( m_fAnimTime >= m_fAnimTimes[m_uType][m_uAnimTimeIndex] ) {
			// Only get a part on our arm if the dispensers aren't destroyed
			if( !m_bDispenserDestroyed ) {
				// Put the dispenser into its original state, we have the part now
				_DispenserEnable( FALSE );

				// Grab a part
				FASSERT( m_pPullPart == NULL );

				m_pPullPart = m_pMeshPool->GetFromFreePool();

				FASSERT( m_pPullPart );
				
				CFMtx43A::m_Temp.Identity();	
				CFMtx43A::m_Temp = FMath_aRotMtx43A[ FMTX_ROTTYPE_180Y ];
				CFMtx43A::m_Temp.m_vPos = *m_pCraneAttachBone;
				CFMtx43A::m_Temp.m_vPos.Add( m_pPullPart->m_BoundSphere_MS.m_Pos );

				m_pPullPart->AddToWorld();
				m_pPullPart->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, 1.0f );
				m_pPullPart->UpdateTracker();

				m_bHavePart = TRUE;
				m_fUnitInterp = 0.0f;

				_StateSetMoveUp();

				// Play a grab part sound
				CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_PINCH_HIT ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );
			} else {
				if( CSpyVsSpy::GetGlitch()->IsOnGroundInPieces() ) {
					f32 fDist2 = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.DistSq( *m_pCraneAttachBone );
					
					// The dispenser system has been destroyed, see if we can grab a part
					if( m_uType == MACHINE_HEAD && !m_bHavePart ) {
						if( fDist2 <= FMATH_SQUARE( _STAGE1_GRAB_SPHERE_SIZE ) ) {
							CSpyVsSpy::m_pCommonData->m_bHaveHead = TRUE;
							m_bHavePart = TRUE;
							m_bGotHead = TRUE;
						}
					} else if( m_uType == MACHINE_TORSO && !m_bHavePart ) {
						if( CSpyVsSpy::m_pCommonData->m_bHaveHead ) {
							// Only grab torso once we have a head
							if( fDist2 <= FMATH_SQUARE( _STAGE1_GRAB_SPHERE_SIZE ) ) {
								CSpyVsSpy::m_pCommonData->m_bHaveTorso = TRUE;
								m_bHavePart = TRUE;
							}
						}
					} else if( m_uType == MACHINE_LEGS && !m_bHavePart ) {
						if( CSpyVsSpy::m_pCommonData->m_bHaveHead && CSpyVsSpy::m_pCommonData->m_bHaveTorso ) {
							if( fDist2 <= FMATH_SQUARE( _STAGE1_GRAB_SPHERE_SIZE ) ) {
								CSpyVsSpy::m_pCommonData->m_bHaveLegs = TRUE;
								m_bHavePart = TRUE;
							}
						}
					}
				}

				if( !m_bHavePart ) {
					// We don't have a part, so we need to animate backwards to try again
					m_eState = MACHINE_STATE_GRAB_PART;
					m_uAnimTimeIndex = MACHINE_ANIM_GRAB_TIME;
					m_fAnimDir = -1.0f;
					
					m_pCrane->UserAnim_SetSpeedMult( -1.0f );

					// Start the loop for the machine arm
					CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_ARM_LOOP ), FALSE, &m_pCrane->MtxToWorld()->m_vPos );

					if( pEmitter ) {
						pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
						CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, m_fAnimTimes[ m_uType ][ MACHINE_ANIM_GRAB_TIME ], 0.5f, TRUE );
					}

					// Play a part miss sound
					CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_PINCH_MISS ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );
				} else {
					// We've got a part
					_StateSetMoveUp();

					// Play a grab part sound
					CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_PINCH_HIT ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );
				}
			}
		}
	} else {
		// We are animating backwards
		if( m_fAnimTime <= 0.0f ) {
			// Start animating forward again
			_StateSetGrabPart();
		}
	}
}

void CMachine::_StateHandleLift( void ) {
	if( m_fAnimTime >= m_fAnimTimes[m_uType][m_uAnimTimeIndex] ) {
		_StateSetMoveOver();
	}
}

void CMachine::_StateHandleMove( BOOL bCanDrop ) {
	if( m_uType == MACHINE_HEAD && !m_pPullPart ) {
		if( m_fAnimTime >= 11.0f && !m_bDontPause ) {
			m_pCrane->UserAnim_Pause( TRUE ); // So we don't run away.
			m_eState = MACHINE_STATE_DEAD;

			// No more parts
			_DispenserEnable( FALSE );

			CSpyVsSpy::GetGlitch()->ShakeCamera( _STAGE1_SHAKE_UNIT_INTENSITY, _STAGE1_SHAKE_DURATION );

			return;
		}
	}

	if( m_fAnimTime >= m_fAnimTimes[m_uType][m_uAnimTimeIndex] ) {
		if( m_bHavePart && bCanDrop ) {
			_StateSetPlacePart();

			m_bReadyToDrop = FALSE;
		} else {
			if( m_bHavePart ) {
				m_bReadyToDrop = TRUE;
			}

			// We can't go down, so stand still
			m_fAnimTime = m_fAnimTimes[m_uType][m_uAnimTimeIndex];
			m_pCrane->UserAnim_UpdateTime( m_fAnimTime );
		}
	}
}

void CMachine::_StateHandlePlacePart( void ) {
	if( m_fAnimTime >= m_fAnimTimes[m_uType][m_uAnimTimeIndex] ) {
		_StateSetMoveBack();

		m_bHavePart = FALSE;

		if( m_pPullPart ) {
			m_pMeshPool->ReturnToFreePool( m_pPullPart );
			m_pPullPart = NULL;

			m_Conveyor.PlacePartOnConveyor();

			//m_pDispenser->UserAnim_UpdateUnitTime( 0.0f );
			//m_pDispenser->UserAnim_Pause( FALSE );
			_DispenserEnable( TRUE );

			// Play its work sound
			CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DISPENSER_WORK ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );
		} else if( m_bHavePart ) { 
			m_Conveyor.PlacePartOnConveyor( m_uType );

			_DispenserEnable( TRUE );

			// Play its work sound
			CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DISPENSER_WORK ), FALSE, &m_pDispenser->MtxToWorld()->m_vPos );
		} else {
			// If we don't have a part mesh, just lock the machines where they are
			m_eState = MACHINE_STATE_DEAD;
		}
	}
}

void CMachine::_StateHandleMoveBack( void ) {
	if( m_fAnimTime >= m_fAnimTotalTime ) {
		m_fAnimTime = 0.0f;
		m_pCrane->UserAnim_UpdateTime( 0.0f );
		_StateSetGrabPart();

		if( m_uType == MACHINE_HEAD ) {
			CEmitterFader::Fade( m_pEmitter, 1.0f, 0.0f, 0.25f, TRUE );
			m_pEmitter = NULL;
		}
	}
}




//
//
//
// CCraneStage - The first part of Spy vs. Spy, the crane stage
//
//
//
FCLASS_ALIGN_PREFIX class CCraneStage : public CSpyVsSpyStage { 
//----------------------------------------------------------------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CCraneStage();
	virtual ~CCraneStage() { }

	BOOL Load( LevelEvent_e eEvent );
	void Unload( void );
	void Restore( void );
	void SwitchTo( void );
	void Cleanup( void );
	void Work( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private definitions
//----------------------------------------------------------------------------------------------------------------------------------
private:


//----------------------------------------------------------------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );
	void _ClearMachines( void );
	void _EnableConveyorScroll( BOOL bEnable );

//----------------------------------------------------------------------------------------------------------------------------------
// Private data
//----------------------------------------------------------------------------------------------------------------------------------
private:
	BOOL8 m_bWaitTalk;
	BOOL8 m_bCraneSystemDestroyed;
	BOOL8 m_bUseCam;
	BOOL8 _PAD;

	CMachine m_aMachines[MACHINE_COUNT];
	CStage1Cam m_LookCamera;

	CMeshEntity *m_pHeadConveyor;
	CMeshEntity *m_pConsole;
	CMeshEntity *m_pHeadClip;
	CMeshEntity *m_pTorsoClip;

	FParticle_DefHandle_t m_hSmokeHandle;

	FCLASS_STACKMEM_ALIGN( CCraneStage );
} FCLASS_ALIGN_SUFFIX;

CCraneStage::CCraneStage() { 
	_ClearDataMembers();
}

BOOL CCraneStage::Load( LevelEvent_e eEvent ) { 
	if( eEvent != LEVEL_EVENT_POST_WORLD_LOAD ) {
		return TRUE;
	}

	cchar *pszCraneName;
	cchar *pszDispenserName;
	cchar *pszSplineName;
	cchar *pszMeshName;
	CMachine *pMachine;
	CESpline *pDestSpline;
	CEntity *pEntity;

	// Find the dest point spline for the cranes
	pEntity = CEntity::FindInWorld( _STAGE1_CRANE_DEST_POINTS );
	if( pEntity == NULL ) {
		DEVPRINTF( "CCraneStage::Load(): Entity '%s' was not found.\n", _STAGE1_CRANE_DEST_POINTS );
		goto _ExitWithError;
	}
	
	if( !( pEntity->TypeBits() & ENTITY_BIT_SPLINE ) ) {
		DEVPRINTF( "CCraneStage::Load(): Entity '%s' was not of expected type.\n", _STAGE1_CRANE_DEST_POINTS );
		goto _ExitWithError;
	}

	pDestSpline = (CESpline *) pEntity;
	if( pDestSpline->PointCount() != 3 ) {
		DEVPRINTF( "CCraneStage::Load(): Entity '%s' did not have 3 points.\n", _STAGE1_CRANE_DEST_POINTS );
		goto _ExitWithError;
	}

	// Find the dest point spline for the cranes
	pEntity = CEntity::FindInWorld( _STAGE1_HEAD_CONVEYOR );
	if( pEntity == NULL ) {
		DEVPRINTF( "CCraneStage::Load(): Entity '%s' was not found.\n", _STAGE1_HEAD_CONVEYOR );
		goto _ExitWithError;
	}
	
	if( !( pEntity->TypeBits() & ENTITY_BIT_MESHENTITY ) ) {
		DEVPRINTF( "CCraneStage::Load(): Entity '%s' was not of expected type.\n", _STAGE1_HEAD_CONVEYOR );
		goto _ExitWithError;
	}

	m_pHeadConveyor = (CMeshEntity *) pEntity;

	// Find the console
	pEntity = CEntity::FindInWorld( _STAGE1_CONSOLE );
	if( pEntity == NULL ) {
		DEVPRINTF( "CCraneStage::Load(): Entity '%s' was not found.\n", _STAGE1_CONSOLE );
		goto _ExitWithError;
	}
	
	if( !( pEntity->TypeBits() & ENTITY_BIT_MESHENTITY ) ) {
		DEVPRINTF( "CCraneStage::Load(): Entity '%s' was not of expected type.\n", _STAGE1_CONSOLE );
		goto _ExitWithError;
	}

	m_pConsole = (CMeshEntity *) pEntity;

	_EnableConveyorScroll( TRUE );


	// Setup our machines, the dispenser / crane pairs
	for( u32 i = 0; i < MACHINE_COUNT; ++i ) {
		switch( i ) {
			case MACHINE_HEAD:
				pszCraneName = _STAGE1_CRANE_HEAD;
				pszDispenserName = _STAGE1_DISPENSER_HEAD;
				pszSplineName = _STAGE1_CONVEYOR_HEAD_POINTS;
				pszMeshName = _STAGE1_HEAD_MESH;
			break;
			case MACHINE_TORSO:
				pszCraneName = _STAGE1_CRANE_TORSO;
				pszDispenserName = _STAGE1_DISPENSER_TORSO;
				//pszSplineName = _STAGE1_CONVEYOR_TORSO_POINTS;
				pszSplineName = _STAGE1_CONVEYOR_LEGS_POINTS;
				pszMeshName = _STAGE1_TORSO_MESH;
			break;
			case MACHINE_LEGS:
				pszCraneName = _STAGE1_CRANE_LEGS;
				pszDispenserName = _STAGE1_DISPENSER_LEGS;
				//pszSplineName = _STAGE1_CONVEYOR_LEGS_POINTS;
				pszSplineName = _STAGE1_CONVEYOR_TORSO_POINTS;
				pszMeshName = _STAGE1_LEGS_MESH;
			break;
		}

		pMachine = &m_aMachines[i];

		if( !pMachine->Setup( i, pszCraneName, pszDispenserName, pDestSpline->PointArray()[i], pszSplineName, pszMeshName ) ) {
			goto _ExitWithError;
		}
	}

	if( !m_LookCamera.Load() ) {
		DEVPRINTF( "CCraneStage::Load(): Failed to load look camera.\n" );
		goto _ExitWithError;
	}

	if( !( m_pHeadClip = (CMeshEntity *) CSpyVsSpy::FindEntity( _STAGE1_HEAD_CLIP, ENTITY_BIT_MESHENTITY ) ) ) {
		goto _ExitWithError;
	}

	if( !( m_pTorsoClip = (CMeshEntity *) CSpyVsSpy::FindEntity( _STAGE1_TORSO_CLIP, ENTITY_BIT_MESHENTITY ) ) ) {
		goto _ExitWithError;
	}

	_SetFinished( FALSE );

	return TRUE;

_ExitWithError:
	Unload();
	_ClearDataMembers();

	return FALSE;
}

void CCraneStage::Unload( void ) { 
	for( u32 i = 0; i < MACHINE_COUNT; ++i ) {
		m_aMachines[i].Unload();
	}

	m_LookCamera.Unload();

	_ClearDataMembers();
}

void CCraneStage::Restore( void ) {
	for( u32 i = 0; i < MACHINE_COUNT; ++i ) {
		m_aMachines[i].Restore();
	}

	if( m_bUseCam ) {
		m_LookCamera.Restore();
		m_bUseCam = FALSE;
	}

	m_bWaitTalk = TRUE;
	CSpyVsSpy::TurnOffHUD( FALSE );
	
	m_bCraneSystemDestroyed = FALSE;

	SwitchTo();
	_SetFinished( FALSE );
}

void CCraneStage::SwitchTo( void ) {
	m_pHeadClip->AddToWorld();
	m_pTorsoClip->AddToWorld();
	CSpyVsSpy::TurnOffHUD( FALSE );
}

void CCraneStage::Cleanup( void ) {
 	for( u32 i = 0; i < MACHINE_COUNT; ++i ) {
		m_aMachines[ i ].Cleanup();
	}
}

void CCraneStage::Work( void ) { 
	u32 i;
	BOOL bReadyToDrop = TRUE;
	BOOL bCanDrop = TRUE;
	BOOL bAllHaveParts = TRUE;

	if( m_bWaitTalk ) {
		CSpyVsSpy::StreamStart( _SPY_VS_SPY_STREAM_START, TRUE, TRUE );
		m_bWaitTalk = FALSE;
		return;
	}

 	for( i = 0; i < MACHINE_COUNT; ++i ) {
		// Once we are in the camera, head machine is finished doing things.  
		if( m_bUseCam && i == MACHINE_HEAD ) {
			continue;
		}

		if( !m_aMachines[i].HasPartInArm() ) {
			bAllHaveParts = FALSE;
		} else {
			switch( i ) {
				case MACHINE_TORSO:
					if( m_pTorsoClip->IsInWorld() && m_bUseCam ) {
						m_pTorsoClip->RemoveFromWorld();
					}
				break;
			}	
		}

		if( !m_aMachines[ i ].GetReadyToDrop() ) {
			bCanDrop = FALSE;
		}
	}

	bReadyToDrop = bAllHaveParts && bCanDrop;

	if( m_pConsole->GetCurrentMeshIndex() != 0 ) {
		m_bCraneSystemDestroyed = TRUE;
	}

	if( m_bUseCam ) {
		m_LookCamera.Work();

		if( bReadyToDrop && !m_LookCamera.IsDropping() ) {
			m_aMachines[ MACHINE_HEAD ].StartMoveToEnd();
			m_LookCamera.StartMoveToEnd();
			_EnableConveyorScroll( FALSE );
		}

		if( m_LookCamera.IsCraneCamFinished() ) {
			_EnableConveyorScroll( TRUE );
			_SetFinished( TRUE );

			CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->SetDrawFlags( CHud2::DRAW_GLITCH_HUD );
			CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->EnableDrawFlags( CHud2::DRAW_GLITCH_HUD );
			CSpyVsSpy::TurnOffHUD( TRUE );
		}

		// This is done so that we can drop the other body parts at about the time when the camera animation drops
		if( m_LookCamera.CanDropPart() ) {
			bReadyToDrop = TRUE;
		} else {
			bReadyToDrop = FALSE;
		}
	}

	for( i = 0; i < MACHINE_COUNT; ++i ) {
		m_aMachines[i].Work( bReadyToDrop, m_bCraneSystemDestroyed );

		if( m_aMachines[i].JustGrabbedHead() ) {
			m_LookCamera.Setup( CSpyVsSpy::GetGlitch(), &CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
			m_bUseCam = TRUE;
			m_pHeadClip->RemoveFromWorld();

			CInventory *pInv = CSpyVsSpy::GetGlitch()->m_pInventory;
			CItemInst *pPrimary = pInv->IsWeaponInInventory( "Empty Primary" );
			CItemInst *pSecondary = pInv->IsWeaponInInventory( "Wrench" );

			pInv->SetCurWeapon( INV_INDEX_PRIMARY, pPrimary, FALSE, TRUE );
			pInv->SetCurWeapon( INV_INDEX_SECONDARY, pSecondary, FALSE, TRUE );

			//CSpyVsSpy::TurnOffHUD( TRUE );
			CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->DisableDrawFlags( CHud2::DRAW_GLITCH_HUD );
			CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->EnableDrawFlags( ( CHud2::DRAW_LEFT_AMMOBOX | CHud2::DRAW_LEFT_WEAPONBOX ) );
			CSpyVsSpy::GetGlitch()->ShakeCamera( _STAGE1_SHAKE_UNIT_INTENSITY, _STAGE1_SHAKE_DURATION );
		}
	}

//	if( m_bUseCam ) {
//		m_LookCamera.Work();
//
//		if( bReadyToDrop ) {
//			m_LookCamera.StartMoveToEnd();
//			_EnableConveyorScroll( FALSE );
//		}
//
//		if( m_LookCamera.IsCraneCamFinished() ) {
//			_EnableConveyorScroll( TRUE );
//			_SetFinished( TRUE );
//		}
//	}
}

void CCraneStage::_ClearDataMembers( void ) { 
	m_bWaitTalk = TRUE;
	m_bCraneSystemDestroyed = FALSE;
	m_bUseCam = FALSE;
	m_LookCamera.Unload();
	
	m_pHeadConveyor = NULL;
	m_pConsole = NULL;
	m_pHeadClip = NULL;
	m_pTorsoClip = NULL;

	_ClearMachines();
}

void CCraneStage::_ClearMachines( void ) {
	for( u32 i = 0; i < MACHINE_COUNT; ++i ) {
		m_aMachines[i].Reset();
	}
}

void CCraneStage::_EnableConveyorScroll( BOOL bEnable ) {
	FASSERT( m_pHeadConveyor );

	FMeshTexLayerHandle_t hTexLayer = m_pHeadConveyor->GetMesh()->GetTexLayerHandle( _STAGE1_CONVEYOR_TEXTURE_ID );

	if( hTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) {
		return;
	}

	m_pHeadConveyor->GetMesh()->AnimTC_ApplyToShader( hTexLayer, bEnable );
}

#define _STAGE2_GURNEY_NAME							( "gurney01" )
#define _STAGE2_GURNEY_SPLINE						( "gurneyspline01" )
#define _STAGE2_PART_BONE_NAME						( "part_bone" )
#define _STAGE2_CONVEYOR_CAMERA						( "convcam" )
#define _STAGE2_PART_LEGS							( "glegs" )
#define _STAGE2_PART_TORSO							( "gtorso" )
#define _STAGE2_DROP_START_NAME						( "Build_Drop" )
#define _STAGE2_DROP_END_NAME						( "Build_Land" )
#define _STAGE2_ASSEMBLY_MACHINE					( "GlitchBuilder" )
#define _STAGE2_WATCH_POINT							( "watch_glitch" )
#define _STAGE2_SPARK_PARTICLES						( "s_spkshower" )

#define _STAGE2_BAD_COUNT_MAX						( 5 )
#define _STAGE2_MIN_LOOK_ANGLE						( 20.0f )
#define _STAGE2_MAX_LOOK_ANGLE						( 60.0f )
// !!nate - tweak this
#define _STAGE2_GURNEY_FRAC_DOOR_TIME				( 0.54f )
#define _STAGE2_GURNEY_MOVE_TIME					( 20.0f )
#define _STAGE2_GURNEY_MOVE_DOOR_TIME				( _STAGE2_GURNEY_MOVE_TIME * _STAGE2_GURNEY_FRAC_DOOR_TIME )
#define _STAGE2_CONVEYOR_PART_START_TIME			( 21.0f )
#define _STAGE2_SNAP_BACK_RATE						( 2.0f )
#define _STAGE2_BUILD_TIME							( 4.0f )
#define _STAGE2_OVERLAP_TIME						( 0.2f )
#define _STAGE2_PART_FALL_TIME						( 0.5f )
#define _STAGE2_BUILDER_SECS_BEFORE_HAND_CONTACT	( 35.0f * _OO_ANIM_FPS ) // !!Nate - used to be 35
#define _STAGE2_BUILDER_SECS_OF_ARM_STOW			( ( 120.0f - 85.0f ) * _OO_ANIM_FPS )
#define _STAGE2_FREEZE_TIME							( 9.33f )
#define _STAGE2_MACHINE_TIMESCALE					( 2.0f )
#define _STAGE2_SPARK_TIME_MIN						( 0.1f )
#define _STAGE2_SPARK_TIME_MAX						( 0.3f )

//
//
//
// CStage2Cam
//
//
//
FCLASS_ALIGN_PREFIX class CStage2Cam : public CCam {
public:
	CStage2Cam();
	virtual ~CStage2Cam() { }

	FINLINE BOOL CanStartParts( void ) { FASSERT( m_pCamConveyorAnimInst ); return ( m_pCamConveyorAnimInst->GetTime() >= _STAGE2_CONVEYOR_PART_START_TIME ); }
	FINLINE BOOL IsConveyorFinished( void ) {  FASSERT( m_pCamConveyorAnimInst ); return ( m_pCamConveyorAnimInst->GetTime() == m_pCamConveyorAnimInst->GetTotalTime() ); }
	FINLINE BOOL IsInGurneyMode( void ) { return m_eCamState == CAMERA_STATE_GURNEY; }
	FINLINE BOOL InGlitch( void ) { return m_eCamState == CAMERA_STATE_TO_GLITCH && m_fUnitTime == 1.0f; }
	FINLINE const f32 &GetUnitTime( void ) { return m_fUnitTime; }
	FINLINE CFCamAnimInst *GetCamAnimInst( void ) { return m_pCamConveyorAnimInst; }

	BOOL Load( void );
	void Unload( void );
	void Restore( void );
	void Setup( CBot *pBot );
	void Work( void );

	void SetGurneyMode( CFVec3A *pCamPos );
	void FreezeCamera( const CFMtx43A *pMtx );
	void CamToGlitch( void );
	void SwitchBackToGlitch( void );
	void MoveToEntity( CEntity *pEntity );

protected:
	virtual void _Reset( void );
	void _ModifyQuatWithPlayerLook( CFQuatA *pQuat, BOOL bMod );
	void _HandleOnConveyor( void );
	void _HandleOnGurney( void );
	void _Interp( void );
	
protected:
	typedef enum {
		CAMERA_STATE_FROZEN,
		CAMERA_STATE_MOVE_CONVEYOR,
		CAMERA_STATE_GURNEY,
		CAMERA_STATE_TO_GLITCH,
	} CameraState_e;
public:
	BOOL m_bModControls;
	CameraState_e m_eCamState;

	f32 m_fTimeScale;
	f32 m_fPlayerYaw;
	f32 m_fPlayerPitch;

	CFVec3A *m_pChasePoint;
	CFCamAnimInst *m_pCamConveyorAnimInst;

	FCLASS_STACKMEM_ALIGN( CStage2Cam );
} FCLASS_ALIGN_SUFFIX;

CStage2Cam::CStage2Cam() {
	_Reset(); 
	m_pCamConveyorAnimInst = NULL;
}

BOOL CStage2Cam::Load( void ) {
	m_pCamConveyorAnimInst = CFCamAnimInst::Load( _STAGE2_CONVEYOR_CAMERA );

	if( !m_pCamConveyorAnimInst ) {
		DEVPRINTF( "CStage2Cam::Load(): Failed to load camera '%s'.\n", _STAGE2_CONVEYOR_CAMERA );
		goto _ExitWithError;
	}

	m_bModControls = TRUE;

	return TRUE;

_ExitWithError:
	Unload();
	return FALSE;
}

void CStage2Cam::Unload( void ) {
	if( m_pCamConveyorAnimInst ) {
		fdelete( m_pCamConveyorAnimInst );
		m_pCamConveyorAnimInst = NULL;
	}

	_Reset();
}

void CStage2Cam::Restore( void ) {
//	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );

	m_eCamState = CAMERA_STATE_MOVE_CONVEYOR;

	m_bModControls = TRUE;

	m_fPlayerYaw = 0.0f;
	m_fPlayerPitch = 0.0f;
	m_fTimeScale = 1.0f;

	_Reset();

	m_pCamConveyorAnimInst->UpdateTime( 0.0f );
	m_pCamConveyorAnimInst->ComputeFrame( m_fCurrentFOV, m_CurrentQuat, m_CurrentPoint.v3 );

	m_fStartFOV = m_fCurrentFOV;
	m_StartQuat = m_CurrentQuat;
	m_StartPoint = m_CurrentPoint;

	m_fCurrentFOV = m_fStartFOV;
	m_CurrentPoint = m_StartPoint;
	m_CurrentQuat = m_StartQuat;
}

void CStage2Cam::Setup( CBot *pBot ) {
	FASSERT( pBot );
	FASSERT( m_pCamConveyorAnimInst );

	CFCamera* pCam = fcamera_GetCameraByIndex( pBot->m_nPossessionPlayerIndex );

	m_fUnitTime = 0.0f;
	m_StartPoint = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
	m_StartQuat.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
	pCam->GetFOV( &m_fStartFOV );

	m_pCamConveyorAnimInst->UpdateTime( 0.0f );
//	m_pCamConveyorAnimInst->ComputeFrame( m_fCurrentFOV, m_CurrentQuat, m_CurrentPoint.v3 );
//
//	m_fStartFOV = m_fCurrentFOV;
//	m_StartQuat = m_CurrentQuat;
//	m_StartPoint = m_CurrentPoint;

	m_fCurrentFOV = m_fStartFOV;
	m_CurrentPoint = m_StartPoint;
	m_CurrentQuat = m_StartQuat;

	m_Camera.m_pvecPos = &m_CurrentPoint;
	m_Camera.m_pqQuat = &m_CurrentQuat;
	m_Camera.m_fHalfFOV = m_fCurrentFOV;

	gamecam_SwitchPlayerToManualCamera( PLAYER_CAM( pBot->m_nPossessionPlayerIndex ), &m_Camera );

	m_eCamState = CAMERA_STATE_MOVE_CONVEYOR;

	m_pChasePoint = NULL;
	m_fTimeScale = 1.0f;
}

void CStage2Cam::Work( void ) {
	switch( m_eCamState ) {
		case CAMERA_STATE_FROZEN:
			m_fUnitTime += ( m_fTimeScale * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fUnitTime, 1.0f );

			_Interp();
		break;
		case CAMERA_STATE_MOVE_CONVEYOR:
			_HandleOnConveyor();
		break;
		case CAMERA_STATE_GURNEY:
			_HandleOnGurney();
		break;
		case CAMERA_STATE_TO_GLITCH:
			m_fUnitTime += ( m_fTimeScale * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fUnitTime, 1.0f );

			_Interp();
		break;
	}
}

void CStage2Cam::SetGurneyMode( CFVec3A *pCamPos ) {
	m_eCamState = CAMERA_STATE_GURNEY;

	m_pChasePoint = pCamPos;
	m_fUnitTime = 0.0f;

	m_StartPoint = m_EndPoint = m_CurrentPoint;
    m_fStartFOV = m_fEndFOV = m_fCurrentFOV;
	m_StartQuat = m_EndQuat = m_CurrentQuat;

	m_Camera.m_fHalfFOV = m_fCurrentFOV;
}

void CStage2Cam::FreezeCamera( const CFMtx43A *pMtx ) {
	m_eCamState = CAMERA_STATE_FROZEN;
	m_fUnitTime = 0.0f;
	m_fTimeScale = 0.25f;

	m_StartQuat = m_CurrentQuat;
	m_StartPoint = m_EndPoint = m_CurrentPoint;
    m_fStartFOV = m_fEndFOV = m_fCurrentFOV;

	m_EndQuat.BuildQuat( *pMtx );
}

void CStage2Cam::CamToGlitch( void ) {
	if( m_eCamState != CAMERA_STATE_TO_GLITCH ) {
		m_StartQuat = m_CurrentQuat;
		m_StartPoint = m_CurrentPoint;
		m_fStartFOV = m_fCurrentFOV;

		m_fUnitTime = 0.0f;
	}

	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );
	fcamera_Work();

	CFCamera* pCam = fcamera_GetCameraByIndex( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );

	m_EndPoint = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
	m_EndQuat.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
	pCam->GetFOV( &m_fEndFOV );

	gamecam_SwitchPlayerToManualCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), &m_Camera );

	m_eCamState = CAMERA_STATE_TO_GLITCH;
}

void CStage2Cam::SwitchBackToGlitch( void ) {
	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );
}

void CStage2Cam::MoveToEntity( CEntity *pEntity ) {
	CFQuatA Quat;
	CFVec3A Temp;

	m_fUnitTime = 0.0f;

	Temp = CFVec3A::m_UnitAxisY;
	Temp.Sub( CFVec3A::m_UnitAxisX );
	Temp.Mul( 4.5f );

	m_EndPoint.Add( pEntity->MtxToWorld()->m_vPos, Temp );

	Temp = pEntity->MtxToWorld()->m_vUp;
	Temp.Mul( -1.0f );
	CFMtx43A::m_Temp.UnitMtxFromUnitVec( &Temp );

	m_EndQuat.BuildQuat( CFMtx43A::m_Temp );
	m_fEndFOV = m_fCurrentFOV;

	_SetStartToCurrent();

	m_eCamState = CAMERA_STATE_FROZEN;
	m_fTimeScale = 0.25f;
}

void CStage2Cam::_Reset( void ) {
	CCam::_Reset();

	m_eCamState = CAMERA_STATE_MOVE_CONVEYOR;

	m_fPlayerYaw = 0.0f;
	m_fPlayerPitch = 0.0f;
	
	m_pChasePoint = NULL;
	
	m_bModControls = TRUE;
}

void CStage2Cam::_ModifyQuatWithPlayerLook( CFQuatA *pQuat, BOOL bMod ) {
	FASSERT( pQuat );

	CFMtx43A TempMtx;
	f32 fYaw, fPitch;
	CHumanControl *pHumanControl;
	CPlayer *pPlayer;
	f32 fControls_AimDown;

	pQuat->BuildMtx( CFMtx43A::m_Temp );

	fYaw = fmath_Atan( CFMtx43A::m_Temp.m_vFront.x, CFMtx43A::m_Temp.m_vFront.z );
	fPitch = fmath_Atan( fmath_Sqrt( 1.0f - CFMtx43A::m_Temp.m_vFront.y * CFMtx43A::m_Temp.m_vFront.y ), CFMtx43A::m_Temp.m_vFront.y ) - FMATH_HALF_PI;

	if( bMod ) {
		FASSERT( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex >= 0 );

		pHumanControl = (CHumanControl *) CSpyVsSpy::GetGlitch()->Controls();
		pPlayer = &Player_aPlayer[ CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ];
		fControls_AimDown = pHumanControl->m_fAimDown;

		if( pHumanControl->m_fRotateCW != 0.0f ) {
			m_fPlayerYaw += 0.25f * pHumanControl->m_fRotateCW * FLoop_fPreviousLoopSecs * FMATH_2PI;
			//FMATH_CLAMP( m_fCamYaw, FMATH_DEG2RAD( -20.0f ), FMATH_DEG2RAD( 20.0f ) );
		}

		if( fControls_AimDown != 0.0f ) {
			m_fPlayerPitch += 0.25f * fControls_AimDown * FLoop_fPreviousLoopSecs * FMATH_2PI;
			FMATH_CLAMP( m_fPlayerPitch, FMATH_DEG2RAD( -_STAGE2_MAX_LOOK_ANGLE ), FMATH_DEG2RAD( _STAGE2_MIN_LOOK_ANGLE ) );
		}

		if( m_fPlayerYaw > FMATH_2PI ) {
			while( m_fPlayerYaw > FMATH_2PI ) {
				m_fPlayerYaw -= FMATH_2PI;
			}
		} else if( m_fPlayerYaw < -FMATH_2PI ) {
			while( m_fPlayerYaw > FMATH_2PI ) {
				m_fPlayerYaw += FMATH_2PI;
			}
		}

		if( m_fPlayerPitch > FMATH_2PI ) {
			while( m_fPlayerPitch > FMATH_2PI ) {
				m_fPlayerPitch -= FMATH_2PI;
			}
		} else if( m_fPlayerPitch < -FMATH_2PI ) {
			while( m_fPlayerPitch < FMATH_2PI ) {
				m_fPlayerPitch += FMATH_2PI;
			}
		}
	} else {
		// Move the view back to the camera animation
		if( m_fPlayerYaw < 0.0f ) {
			m_fPlayerYaw += ( _STAGE2_SNAP_BACK_RATE * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fPlayerYaw, 0.0f );
		} else if( m_fPlayerYaw > 0.0f ) {
			m_fPlayerYaw -= ( _STAGE2_SNAP_BACK_RATE * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMIN( m_fPlayerYaw, 0.0f );
		}

		if( m_fPlayerPitch < 0.0f ) {
			m_fPlayerPitch += ( _STAGE2_SNAP_BACK_RATE * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fPlayerPitch, 0.0f );
		} else if( m_fPlayerPitch > 0.0f ) {
			m_fPlayerPitch -= ( _STAGE2_SNAP_BACK_RATE * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMIN( m_fPlayerPitch, 0.0f );
		}
	}

	fPitch += m_fPlayerPitch;

	TempMtx.Identity();
	TempMtx.RotateX( fPitch );

	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.RotateY( fYaw + m_fPlayerYaw );
	CFMtx43A::m_Temp.Mul( TempMtx );

	pQuat->BuildQuat( CFMtx43A::m_Temp );
}

void CStage2Cam::_HandleOnConveyor( void ) {
	m_fUnitTime += ( 2.0f * FLoop_fPreviousLoopSecs );
	FMATH_CLAMPMAX( m_fUnitTime, 1.0f );

	m_pCamConveyorAnimInst->DeltaTime( FLoop_fPreviousLoopSecs, TRUE );
	m_pCamConveyorAnimInst->ComputeFrame( m_fEndFOV, m_EndQuat, m_EndPoint.v3 );

	_Interp();
	
	// !!Nate - #define this time or get a frame count
	if( m_pCamConveyorAnimInst->GetTime() >= _STAGE2_FREEZE_TIME && m_bModControls ) {
		m_bModControls = FALSE;
	}

	_ModifyQuatWithPlayerLook( &m_CurrentQuat, m_bModControls );

	// Pull the other parts along with us
	CFVec3A HeadMoveAmount;

	HeadMoveAmount.Sub( m_CurrentPoint, CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos );
	HeadMoveAmount.y = 0.0f;

	if( m_bModControls ) {
		CSpyVsSpy::m_pCommonData->m_TorsoMtx.m_vPos.Add( HeadMoveAmount );
		CSpyVsSpy::m_pCommonData->m_LegsMtx.m_vPos.Add( HeadMoveAmount );
		CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos.Add( HeadMoveAmount );

		// So the head is always below us and not drawn
		CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos.y = -999999.0f;
	}

	CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &m_CurrentPoint );
}

void CStage2Cam::_HandleOnGurney( void ) {
	if( m_pChasePoint ) {
		CFVec3A BumpUp = CFVec3A::m_UnitAxisY;

		BumpUp.Mul( 2.0f );

		m_EndPoint = *m_pChasePoint;
		m_EndPoint.Add( BumpUp );
	}
	
	m_fUnitTime += ( 2.0f * FLoop_fPreviousLoopSecs );
	FMATH_CLAMPMAX( m_fUnitTime, 1.0f );
	
	_Interp();

	_ModifyQuatWithPlayerLook( &m_CurrentQuat, TRUE );
}


void CStage2Cam::_Interp( void ) {
	f32 fUnitLin = fmath_UnitLinearToSCurve( m_fUnitTime );

	m_CurrentPoint.Lerp( fUnitLin, m_StartPoint, m_EndPoint );
	m_fCurrentFOV = FMATH_FPOT( fUnitLin, m_fStartFOV, m_fEndFOV );
	m_CurrentQuat.ReceiveSlerpOf( fUnitLin, m_StartQuat, m_EndQuat );

	m_Camera.m_fHalfFOV = m_fCurrentFOV;
}

//
//
//
// CMoveStage - The second part of Spy vs. Spy, the moving stage.
//
//
//
FCLASS_ALIGN_PREFIX class CMoveStage : public CSpyVsSpyStage { 
//----------------------------------------------------------------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CMoveStage();
	virtual ~CMoveStage() { }

	BOOL Load( LevelEvent_e eEvent );
	void Unload( void );
	void Restore( void );
	void Work( void );
	void SwitchTo( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private definitions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	typedef enum {
		STATE_NONE,
		STATE_ON_CONVEYOR,
		STATE_ON_GURNEY,
		STATE_WATCH,
		STATE_PIECES_FALL,
		STATE_MACHINE_WORK,
		STATE_CAM_TO_WATCH,
		STATE_BUILD,
		STATE_CAM_TO_GLITCH,
	} StageState_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );
	void _ResetStates( void );
	void _HandleControls( void );
	void _PlaceGlitchIntoWorld( void );
	void _GurneyRunAway( void );

	void _ArmStartAnimation( void );
	void _ArmUpdateAnimation( const f32 &fUnitDropTime );
	void _ArmEndAnimation( void );

	void _SetupMoverParts( void );
	void _StartGlitchBuild( void );
	void _BuildWork( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private data
//----------------------------------------------------------------------------------------------------------------------------------
private:
	StageState_e m_eStageState;

	CStage2Cam m_StageCamera;

	BOOL m_bLockControls;

	f32 m_fRotInterp;
	CFVec3A *m_pPartBone;
	CMeshEntity *m_pGurney;
	CHorizontalSplineActor *m_pSplineActor;

	// Start positions for pull parts
	CFVec3A m_StartLegsPos;
	CFVec3A m_StartTorsoPos;
	CFVec3A m_StartHeadPos;

	CFVec3A m_HeadOffset;
	CFVec3A m_LegsOffset;
	CFVec3A m_TorsoOffset;

	u32 m_uBadCount;

	CMeshEntity *m_pLegs;
	CMeshEntity *m_pTorso;

	CFXMeshBuildPartMgr *m_pPartBuilder;

	f32 m_fPartFallTime;

	CFVec3A m_DropStart;
	CFVec3A m_DropEnd;

	f32 m_fArmAnim_SecsTillFirstContact;
	f32 m_fArmAnim_SecsRegPartDrop;
	f32 m_fArmAnim_PercentOfAnim1stContact;
	f32 m_fArmAnim_PercentOfAnimPartDrop;
	f32 m_fArmAnim_SecsOfArmStow;
	// vars based one the actual bot being dispensed and the above ArmAnim values
	f32 m_fBotArm_UnitSecsTillFirstContact;
	f32 m_fBotArm_OODropNormalizer;
	f32 m_fSecsTillNextSpark;
	f32 m_fSecsBetweenPartLandings;
	FParticle_DefHandle_t m_hSparkParticleDef;

	CEntity *m_pWatchPoint;
	CMeshEntity *m_pBuildMachine;

	CDoorEntity *m_pDoor1;
	CDoorEntity *m_pDoor2;

	CFAudioEmitter *m_pMachineEmitter;
	FCLASS_STACKMEM_ALIGN( CMoveStage );
} FCLASS_ALIGN_SUFFIX;

CMoveStage::CMoveStage() { 
	_ClearDataMembers();
}

BOOL CMoveStage::Load( LevelEvent_e eEvent ) { 
	if( eEvent == LEVEL_EVENT_POST_WORLD_LOAD ) {
		CEntity *pEntity;

		// Find the gurney
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_GURNEY_NAME, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}
		m_pGurney = (CMeshEntity *) pEntity;

		if( m_pGurney->UserAnim_GetCount() != 2 ) {
			DEVPRINTF( "CCraneStage::Load(): Gurney does not have the right number of animations.\n" );
			goto _ExitWithError;
		}

		s32 sBoneIndex = m_pGurney->GetMesh()->FindBone( _STAGE2_PART_BONE_NAME );

		if( sBoneIndex == -1 ) {
			DEVPRINTF( "CCraneStage::Load(): Could not find bone '%s' in '%s'.\n", _STAGE2_PART_BONE_NAME, _STAGE2_GURNEY_NAME );
			goto _ExitWithError;
		}

		m_pPartBone = &m_pGurney->GetMesh()->GetBoneMtxPalette()[ sBoneIndex ]->m_vPos;

		// Find the legs
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_PART_LEGS, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}
		m_pLegs = (CMeshEntity *) pEntity;

		// Find the torso
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_PART_TORSO, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}
		m_pTorso = (CMeshEntity *) pEntity;

		// Find the build machine
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_ASSEMBLY_MACHINE, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}
		m_pBuildMachine = (CMeshEntity *) pEntity;

		if( m_pBuildMachine->UserAnim_GetCount() != 2 ) {
			DEVPRINTF( "CCraneStage::Load(): Entity '%s' does not have 2 animations.\n", _STAGE2_ASSEMBLY_MACHINE );
			goto _ExitWithError;
		}

		// Get the watch point
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_WATCH_POINT, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
		m_pWatchPoint = pEntity;

		// Find the drop points
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_DROP_START_NAME, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
		m_DropStart = pEntity->MtxToWorld()->m_vPos;

		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_DROP_END_NAME, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
		m_DropEnd = pEntity->MtxToWorld()->m_vPos;

		// Find the doors we need
		if( !( pEntity = CSpyVsSpy::FindEntity( "gurneydoor01", ENTITY_BIT_DOOR ) ) ) {
			goto _ExitWithError;
		}
		m_pDoor1 = (CDoorEntity *) pEntity;

		
		if( !( pEntity = CSpyVsSpy::FindEntity( "gurneydoor02", ENTITY_BIT_DOOR ) ) ) {
			goto _ExitWithError;
		}
		m_pDoor2 = (CDoorEntity *) pEntity;

		// Load the spark particles
		m_hSparkParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _STAGE2_SPARK_PARTICLES );

		if( m_hSparkParticleDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CMoveStage::Load(): Failed to load particle '%s'.\n", _STAGE2_SPARK_PARTICLES );
			goto _ExitWithError;
		}

		if( !( pEntity = (CESpline *) CSpyVsSpy::FindEntity( _STAGE1_CONVEYOR_TORSO_POINTS, ENTITY_BIT_SPLINE ) ) ) {
			goto _ExitWithError;
		}
		m_StartLegsPos = ((CESpline *) pEntity )->PointArray()[ 0 ];

		if( !( pEntity = (CESpline *) CSpyVsSpy::FindEntity( _STAGE1_CONVEYOR_LEGS_POINTS, ENTITY_BIT_SPLINE ) ) ) {
			goto _ExitWithError;
		}
		m_StartTorsoPos = ((CESpline *) pEntity )->PointArray()[ 0 ];

		if( !( pEntity = (CESpline *) CSpyVsSpy::FindEntity( _STAGE1_CONVEYOR_HEAD_POINTS, ENTITY_BIT_SPLINE ) ) ) {
			goto _ExitWithError;
		}
		m_StartHeadPos = ((CESpline *) pEntity )->PointArray()[ 0 ];

		CFVec3A BumpUp = CFVec3A::m_UnitAxisY;

		BumpUp.Mul( 0.5f );

		m_StartHeadPos.Add( BumpUp );
		m_StartTorsoPos.Add( BumpUp );
		m_StartLegsPos.Add( BumpUp );
	} else if( eEvent == LEVEL_EVENT_PRE_ENTITY_FIXUP ) {
        // Setup the gurney spline
		m_pSplineActor = CHorizontalSplineActor::Acquire();

		if( !m_pSplineActor ) {
			DEVPRINTF( "CCraneStage::Load(): Could not get spline actor.\n" );
			goto _ExitWithError;
		}

		if( !m_pSplineActor->Init( _STAGE2_GURNEY_SPLINE, _STAGE2_GURNEY_MOVE_TIME, NULL/*m_pGurney*/, TRUE ) ) {
			DEVPRINTF( "CCraneStage::Load(): Could not get spline actor to init.\n" );
			goto _ExitWithError;
		}

		m_pSplineActor->Stop();

		_ResetStates();

		if( !m_StageCamera.Load() ) {
			goto _ExitWithError;
		}

		_SetFinished( FALSE );

		m_eStageState = STATE_ON_CONVEYOR;
		m_bLockControls = TRUE;
	}

	return TRUE;

_ExitWithError:
	Unload();
	_ClearDataMembers();

	return FALSE;
}

void CMoveStage::Unload( void ) { 
	_ClearDataMembers();
	m_StageCamera.Unload();
}

void CMoveStage::Restore( void ) {
	_ResetStates();

	SwitchTo();

	m_StageCamera.Restore();
	_SetFinished( FALSE );
}

void CMoveStage::Work( void ) { 
	FASSERT( m_pLegs );
	FASSERT( m_pTorso );

	if( m_eStageState == STATE_NONE ) {
		return;
	}

	_HandleControls();

	m_StageCamera.Work();

	switch( m_eStageState ) {
		case STATE_ON_CONVEYOR:
			if( m_StageCamera.CanStartParts() && m_pLegs->UserAnim_IsPaused() ) {
				// Start the animations of the parts falling down
				m_pLegs->UserAnim_Pause( FALSE );
				m_pTorso->UserAnim_Pause( FALSE );
			}

			// In this case everything is in its desination position, so setup the offsets
			if( m_StageCamera.IsConveyorFinished() ) {
				_SetupMoverParts();
			}
		break;
		case STATE_ON_GURNEY: {
			f32 fTime = m_pSplineActor->GetTimePos();

			if( fTime >= 3.0f ) {
				if( m_pDoor1->GetUnitPos() == 0.0f ) {
					m_pDoor1->ForceGotoPos( 1, CDoorEntity::GOTOREASON_UNKNOWN );
				}
			}

			if( fTime >= 9.25f ) {
				if( m_pDoor2->GetUnitPos() == 0.0f ) {
					m_pDoor2->ForceGotoPos( 1, CDoorEntity::GOTOREASON_UNKNOWN );
				}
			}

			m_pSplineActor->GetMatrix( &CFMtx43A::m_Temp );
			m_pGurney->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );

			CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &CFMtx43A::m_Temp.m_vPos );

			// Stop at the door to watch
			if( fTime >= _STAGE2_GURNEY_MOVE_DOOR_TIME ) {
				// !!nate - temp direction
				CFMtx43A::m_Temp.Identity();
				CFMtx43A::m_Temp.m_vFront.Set( 1.0f, 0.0f, 0.0f );
				CFMtx43A::m_Temp.m_vUp.Set( 0.0f, 1.0f, 0.0f );
				CFMtx43A::m_Temp.m_vRight.Set( 0.0f, 0.0f, -1.0f );

				m_StageCamera.FreezeCamera( &CFMtx43A::m_Temp/*m_pGurney->MtxToWorld()*/ );
				m_eStageState = STATE_WATCH;

				m_bLockControls = TRUE;

				CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &m_pGurney->MtxToWorld()->m_vPos );
			}
		break;
		}
		case STATE_WATCH:
			m_pSplineActor->GetMatrix( &CFMtx43A::m_Temp );
			m_pGurney->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );

			// Dump the parts
			if( m_pSplineActor->IsDone() && m_pGurney->UserAnim_GetCurrentIndex() == 0 ) {
				m_pGurney->UserAnim_Select( 1 );
				m_pGurney->UserAnim_SetClampMode( TRUE );
				m_pGurney->UserAnim_SetSpeedMult( 1.0f );
				m_pGurney->UserAnim_UpdateTime( 0.0f );
				m_pGurney->UserAnim_Pause( FALSE );

				_PlaceGlitchIntoWorld();

				CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_BUILD_MACHINE_RUMBLE );
				
				m_pMachineEmitter = CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pBuildMachine->MtxToWorld()->m_vPos );
				CEmitterFader::Fade( m_pMachineEmitter, 0.0f, 1.0f, 3.0f );
			}

			if( m_pGurney->UserAnim_GetCurrentIndex() == 1 && 
				m_pGurney->UserAnim_GetCurrentInst()->GetUnitTime() >= 1.0f ) {
				m_eStageState = STATE_PIECES_FALL;

				m_pTorso->DetachFromParent();
				m_pLegs->DetachFromParent();

				m_fPartFallTime = _STAGE2_PART_FALL_TIME;

				CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_PART_CLATTER ) );
			}
		break;
		case STATE_PIECES_FALL: {
			CFVec3A Vel( m_pGurney->MtxToWorld()->m_vFront );
			CFVec3A Temp;

			Vel.Sub( CFVec3A::m_UnitAxisY );
			Vel.Unitize();
			Vel.Mul( 16.0f * FLoop_fPreviousLoopSecs);

			Temp = m_pTorso->MtxToWorld()->m_vPos;
			Temp.Add( Vel );
			m_pTorso->Relocate_Xlat_WS( &Temp );

			Temp = m_pLegs->MtxToWorld()->m_vPos;
			Temp.Add( Vel );
			m_pLegs->Relocate_Xlat_WS( &Temp );

			m_fPartFallTime -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fPartFallTime, 0.0f );

			if( m_fPartFallTime == 0.0f ) {
				m_fRotInterp = 0.0f;
				m_eStageState = STATE_MACHINE_WORK;
				m_pBuildMachine->UserAnim_Pause( FALSE );
				m_fSecsTillNextSpark = 1.0f;//fmath_RandomFloatRange( _STAGE2_SPARK_TIME_MIN, _STAGE2_SPARK_TIME_MAX );

				CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_ARMS_WORK ) ),
					0.0f, 1.0f, m_pBuildMachine->UserAnim_GetCurrentInst()->GetTotalTime() * fmath_Inv( _STAGE2_MACHINE_TIMESCALE ), 0.5f, TRUE );

				CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_GURNEY_MOVE ), FALSE, &m_pGurney->MtxToWorld()->m_vPos );

				CEmitterFader::Fade( pEmitter, 1.0f, 1.0f, _STAGE2_GURNEY_MOVE_TIME, TRUE, &m_pGurney->MtxToWorld()->m_vPos );
			}
		}
		break;
		case STATE_MACHINE_WORK:{
			f32 fUnitTime = m_pBuildMachine->UserAnim_GetCurrentInst()->GetUnitTime();

			_GurneyRunAway();

			m_fSecsTillNextSpark -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fSecsTillNextSpark, 0.0f );

			CFVec3A SparkPos = CFVec3A::m_UnitAxisY;

			if( m_fSecsTillNextSpark == 0.0f && fUnitTime < 0.8f ) {
				s32 sNum = fmath_RandomRange( 1, 4 );
				CFVec3A SparkPos;

				for( s32 i = 0; i < sNum; ++i ) {
					SparkPos = CFVec3A::m_UnitAxisY;
					SparkPos.Mul( 20.0f );
					SparkPos.Add( m_pBuildMachine->MtxToWorld()->m_vPos );
					SparkPos.x += fmath_RandomFloatRange( -10.0f, 10.0f );
					SparkPos.z += fmath_RandomFloatRange( -10.0f, 10.0f );
	
					fparticle_SpawnEmitter( m_hSparkParticleDef, SparkPos.v3, NULL, 1.0f );
					m_fSecsTillNextSpark = fmath_RandomFloatRange( _STAGE2_SPARK_TIME_MIN, _STAGE2_SPARK_TIME_MAX );
				}
			}

			if( fUnitTime >= 1.0f ) {
				m_StageCamera.MoveToEntity( m_pWatchPoint );
				m_eStageState = STATE_CAM_TO_WATCH;

				m_pLegs->RemoveFromWorld( TRUE );
				m_pTorso->RemoveFromWorld( TRUE );
				m_fSecsTillNextSpark = 0.0f;
			}
		}break;
		case STATE_CAM_TO_WATCH:{
			f32 fUnitTime = m_StageCamera.GetUnitTime();

			_GurneyRunAway();

			if( fUnitTime >= 1.0f ) {
				m_eStageState = STATE_BUILD;
			} else if( fUnitTime >= 0.5f ) {
				if( !( Player_aPlayer[0].m_uPlayerFlags & CPlayer::PF_DONT_CALL_WORK ) ) {
					_StartGlitchBuild();
				} else {
					_BuildWork();
				}
			}
		}
		break;
		case STATE_BUILD:
			_BuildWork();
		break;
		case STATE_CAM_TO_GLITCH:
			_GurneyRunAway();
			_BuildWork();

			if( m_StageCamera.InGlitch() && !m_pPartBuilder ) {
				m_StageCamera.SwitchBackToGlitch();
				m_eStageState = STATE_NONE;
			
				Player_aPlayer[0].m_uPlayerFlags &= ~CPlayer::PF_DONT_CALL_WORK;
				
				m_pGurney->RemoveFromWorld( TRUE );
				_SetFinished( TRUE );

				m_bLockControls = FALSE;

				CSpyVsSpy::GetGlitch()->DontIgnoreBotVBotCollision();
			}
		break;
/*		case STATE_CAM_TO_WATCH:
			_GurneyRunAway();

			if( m_StageCamera.GetUnitTime() >= 1.0f ) {
				Player_aPlayer[0].m_uPlayerFlags |= CPlayer::PF_DONT_CALL_WORK;

				CSpyVsSpy::GetGlitch()->RelocateAllChildren( FALSE );	
				//CSpyVsSpy::GetGlitch()->DrawEnable( TRUE, TRUE );

				m_pPartBuilder = CFXMeshBuildPartMgr::InitMgr( m_DropStart, CSpyVsSpy::GetGlitch(), CSpyVsSpy::GetGlitch()->m_pWorldMesh, _STAGE2_BUILD_TIME, _STAGE2_OVERLAP_TIME );

				m_eStageState = STATE_BUILD;
				
				_ArmStartAnimation();

				// Lock the doors so we can't go back
				m_pDoor1->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );
				m_pDoor2->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );
				m_pDoor1->SetLockState( TRUE );
				m_pDoor2->SetLockState( TRUE );

				CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CONVEYOR_LOOP ) );
				CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, 3.0f + ( m_pBuildMachine->UserAnim_GetCurrentInst()->GetTotalTime() * fmath_Inv( _STAGE2_MACHINE_TIMESCALE ) ), 0.1f, TRUE );
			}
		break;
		case STATE_BUILD:
			_GurneyRunAway();

			if( !CSpyVsSpy::GetGlitch()->IsDrawEnabled() ) {
				CSpyVsSpy::GetGlitch()->DrawEnable( TRUE, TRUE );
			}

			if( m_pPartBuilder ) {
				f32 fUnitPercentBuilt;

				// We are all done with this stage
				if( m_pPartBuilder->Work( fUnitPercentBuilt ) ) {
					m_pPartBuilder = NULL;

					m_StageCamera.CamToGlitch();
					m_eStageState = STATE_CAM_TO_GLITCH;
				}
			
				_ArmUpdateAnimation( fUnitPercentBuilt );

				CSpyVsSpy::GetGlitch()->RelocateAllChildren( FALSE );	
			}
		break;
		case STATE_CAM_TO_GLITCH:
			_GurneyRunAway();

			if( m_StageCamera.InGlitch() ) {
				m_StageCamera.SwitchBackToGlitch();

				m_eStageState = STATE_NONE;
			
				Player_aPlayer[0].m_uPlayerFlags &= ~CPlayer::PF_DONT_CALL_WORK;

				m_pGurney->RemoveFromWorld( TRUE );

				_SetFinished( TRUE );

				m_bLockControls = FALSE;

				CSpyVsSpy::GetGlitch()->DontIgnoreBotVBotCollision();
			}
		break;*/
	}
}

void CMoveStage::SwitchTo( void ) {
	CSpyVsSpy::TurnOffHUD( TRUE );

	m_StageCamera.Setup( CSpyVsSpy::GetGlitch() );

	CSpyVsSpy::m_pCommonData->m_bHaveHead = TRUE;
	CSpyVsSpy::m_pCommonData->m_bHaveLegs = TRUE;
	CSpyVsSpy::m_pCommonData->m_bHaveTorso = TRUE;

	CSpyVsSpy::GetGlitch()->DrawEnable( TRUE, TRUE );
	CSpyVsSpy::GetGlitch()->ImmobilizeBot();
	CSpyVsSpy::GetGlitch()->IgnoreBotVBotCollision();

	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CONVEYOR_LOOP ), TRUE, &CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );

	if( pEmitter ) {
		pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		CEmitterFader::FadeInAndOut( pEmitter,  1.0f, 1.0f, m_StageCamera.GetCamAnimInst()->GetTotalTime(), 0.5f, TRUE );
	}

	CSpyVsSpy::m_pCommonData->m_TorsoMtx.m_vPos = m_StartTorsoPos;
	CSpyVsSpy::m_pCommonData->m_LegsMtx.m_vPos = m_StartLegsPos;
	CSpyVsSpy::m_pCommonData->m_HeadMtx.m_vPos = m_StartHeadPos;

	CInventory *pInv = CSpyVsSpy::GetGlitch()->m_pInventory;
	CItemInst *pPrimary = pInv->IsWeaponInInventory( "Empty Primary" );
	CItemInst *pSecondary = pInv->IsWeaponInInventory( "Empty Secondary" );

	pInv->SetCurWeapon( INV_INDEX_PRIMARY, pPrimary, FALSE, TRUE );
	pInv->SetCurWeapon( INV_INDEX_SECONDARY, pSecondary, FALSE, TRUE );

	if( m_pMachineEmitter ) {
		m_pMachineEmitter->Destroy();
		m_pMachineEmitter = NULL;
	}

	_SetFinished( FALSE );
}

void CMoveStage::_ClearDataMembers( void ) { 
	m_pGurney = NULL;
	m_pSplineActor = NULL;

	m_uBadCount = 0;

	m_pLegs = NULL;
	m_pTorso = NULL;

	m_bLockControls = FALSE;

	m_fPartFallTime = 0.0f;

	m_fArmAnim_SecsTillFirstContact = 0.0f;
	m_fArmAnim_SecsRegPartDrop = 0.0f;
	m_fArmAnim_PercentOfAnim1stContact = 0.0f;
	m_fArmAnim_PercentOfAnimPartDrop = 0.0f;
	m_fArmAnim_SecsOfArmStow = 0.0f;

	m_fBotArm_UnitSecsTillFirstContact = 0.0f;
	m_fBotArm_OODropNormalizer = 0.0f;
	m_fSecsTillNextSpark = 0.0f;
	m_fSecsBetweenPartLandings = 0.0f;

	m_pPartBuilder = NULL;

	m_pDoor1 = NULL;
	m_pDoor2 = NULL;

	m_StartLegsPos.Zero();
	m_StartTorsoPos.Zero();
	m_StartHeadPos.Zero();

	m_fRotInterp = 0.0f;

	m_pMachineEmitter = NULL;
}

void CMoveStage::_ResetStates( void ) {
	m_pSplineActor->Reset( FALSE );
	m_pSplineActor->SetTimePos( 0.0f );

	m_pGurney->UserAnim_Select( 0 );
	m_pGurney->UserAnim_SetClampMode( TRUE );
	m_pGurney->UserAnim_SetSpeedMult( 1.0f );
	m_pGurney->UserAnim_UpdateUnitTime( 0.0f );
	m_pGurney->UserAnim_Pause( TRUE );
	m_pGurney->UserAnim_SetFlip( CMeshEntity::ANIMFLIP_OFF );

	// Since the torso and legs are added as children
	m_pGurney->DetachAllChildren();

	m_pTorso->AddToWorld();
	m_pTorso->UserAnim_SetClampMode( TRUE );
	m_pTorso->UserAnim_SetSpeedMult( 1.0f );
	m_pTorso->UserAnim_UpdateTime( 0.0f );
	m_pTorso->UserAnim_Pause( TRUE );

	m_pLegs->AddToWorld();
	m_pLegs->UserAnim_SetClampMode( TRUE );
	m_pLegs->UserAnim_SetSpeedMult( 1.0f );
	m_pLegs->UserAnim_UpdateTime( 0.0f );
	m_pLegs->UserAnim_Pause( TRUE );

	m_pBuildMachine->UserAnim_Select( 0 );
	m_pBuildMachine->UserAnim_SetFlip( CMeshEntity::ANIMFLIP_OFF );
	m_pBuildMachine->UserAnim_SetClampMode( TRUE );
	m_pBuildMachine->UserAnim_SetSpeedMult( _STAGE2_MACHINE_TIMESCALE );
	m_pBuildMachine->UserAnim_UpdateTime( 0.0f );
	m_pBuildMachine->UserAnim_Pause( TRUE );

	CFSphere CollSphere;

	CollSphere.m_Pos.x = m_pBuildMachine->MtxToWorld()->m_vPos.x;
	CollSphere.m_Pos.y = m_pBuildMachine->MtxToWorld()->m_vPos.y;
	CollSphere.m_Pos.z = m_pBuildMachine->MtxToWorld()->m_vPos.z;
	CollSphere.m_fRadius = _STAGE1_BIG_GLITCH_RADIUS;

	m_pBuildMachine->GetMesh()->MoveTracker( CollSphere );
	
	CSpyVsSpy::GetGlitch()->RelocateAllChildren( FALSE );	

//	m_pLegs->Attach_ToParent_WS( m_pGurney, _STAGE2_PART_BONE_NAME, FALSE );
//	m_pTorso->Attach_ToParent_WS( m_pGurney, _STAGE2_PART_BONE_NAME, FALSE );

//	m_pGurney->UserAnim_Select( 0 );
//	m_pGurney->UserAnim_Pause( TRUE );

	m_pTorso->SetCollisionFlag( FALSE );
	m_pLegs->SetCollisionFlag( FALSE );

	m_eStageState = STATE_ON_CONVEYOR;

	//m_bLockControls = FALSE;
	m_bLockControls = TRUE;

	if( m_pPartBuilder ) {
		if( !m_pPartBuilder->IsMgrAvailable() ) {
			m_pPartBuilder->ForceFinish();
		}
	}

	Player_aPlayer[0].m_uPlayerFlags &= ~CPlayer::PF_DONT_CALL_WORK;

	m_fArmAnim_SecsOfArmStow = _STAGE2_BUILDER_SECS_OF_ARM_STOW;
	m_fArmAnim_SecsTillFirstContact = 0.29f * m_pBuildMachine->UserAnim_GetCurrentInst()->GetTotalTime();//_STAGE2_BUILDER_SECS_BEFORE_HAND_CONTACT;
	m_fArmAnim_SecsRegPartDrop = m_pBuildMachine->UserAnim_GetCurrentInst()->GetTotalTime() - m_fArmAnim_SecsOfArmStow - m_fArmAnim_SecsTillFirstContact;
	m_fArmAnim_PercentOfAnim1stContact = 0.29f;//m_fArmAnim_SecsTillFirstContact * m_pBuildMachine->UserAnim_GetCurrentInst()->GetOOTotalTime();
	m_fArmAnim_PercentOfAnimPartDrop = m_fArmAnim_SecsRegPartDrop * m_pBuildMachine->UserAnim_GetCurrentInst()->GetOOTotalTime();

	m_pDoor1->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );
	m_pDoor2->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );

	m_fRotInterp = 0.0f;
}

void CMoveStage::_HandleControls( void ) {
	CHumanControl * pHumanControl = (CHumanControl *) CSpyVsSpy::GetGlitch()->Controls();
	CPlayer *pPlayer = &Player_aPlayer[ CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ];

	// If they try to move
	if( pHumanControl->m_fForward != 0.0f || pHumanControl->m_fStrafeRight != 0.0f ) {
		++m_uBadCount;
	}

	// If they try to fire
	if( pHumanControl->m_fFire1 != 0.0f || pHumanControl->m_fFire2 != 0.0f ) {
		++m_uBadCount;
	}

	// If they try to select something
	if( pHumanControl->m_nPadFlagsSelect1 != 0 || pHumanControl->m_nPadFlagsSelect2 != 0 ) {
		++m_uBadCount;
	}

	// If they try and action, jump or a mele attack
	if( pHumanControl->m_nPadFlagsAction != 0 || pHumanControl->m_nPadFlagsJump != 0 || pHumanControl->m_nPadFlagsMelee != 0 ) {
		++m_uBadCount;
	}

	if( m_bLockControls ) {
		m_uBadCount = 0;
	}

/*	pHumanControl->m_fForward = 0.0f;
	pHumanControl->m_fStrafeRight = 0.0f;
	pHumanControl->m_fFire1 = 0.0f;
	pHumanControl->m_fFire2 = 0.0f;
	pHumanControl->m_nPadFlagsSelect1 = 0;
	pHumanControl->m_nPadFlagsSelect2 = 0;
	pHumanControl->m_nPadFlagsAction = 0;
	pHumanControl->m_nPadFlagsJump = 0;
	pHumanControl->m_nPadFlagsMelee = 0;*/

	if( m_uBadCount >= _STAGE2_BAD_COUNT_MAX ) {
		m_uBadCount = 0;
	}
}

void CMoveStage::_PlaceGlitchIntoWorld( void ) {
	// No longer use our callback
	CSpyVsSpy::m_pCommonData->m_bHaveHead = FALSE;
	CSpyVsSpy::m_pCommonData->m_bHaveLegs = FALSE;
	CSpyVsSpy::m_pCommonData->m_bHaveTorso = FALSE;

	CFSphere CollSphere;

	// !!Nate - need to remember his original radius somehere, globally so we can share it
	CollSphere.m_Pos.x = m_DropEnd.x;
	CollSphere.m_Pos.y = m_DropEnd.y;
	CollSphere.m_Pos.z = m_DropEnd.z;
	CollSphere.m_fRadius = _STAGE1_BIG_GLITCH_RADIUS;

	CSpyVsSpy::GetGlitch()->GetMesh()->MoveTracker( CollSphere );
	CSpyVsSpy::GetGlitch()->MtxToWorld()->Identity();
	CSpyVsSpy::GetGlitch()->MtxToWorld()->RotateY( FMATH_PI );
	CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos = m_DropEnd;
	CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &m_DropEnd );
	
	CSpyVsSpy::GetGlitch()->ResetToNeutral();
	CSpyVsSpy::TurnOffHUD( TRUE ); // This is done since ResetToNeutral() turns on the reticle
	CSpyVsSpy::GetGlitch()->RelocateAllChildren( FALSE );	
	CSpyVsSpy::GetGlitch()->DrawEnable( FALSE, TRUE );
}

void CMoveStage::_GurneyRunAway( void ) {
	f32 fTime = m_pSplineActor->GetTimePos();
	f32 fAnimTime = m_pGurney->UserAnim_GetCurrentInst()->GetTime();
	CFMtx43A RotMtx;

	fAnimTime -= ( 2.0f * FLoop_fPreviousLoopSecs );
	fTime -= ( 1.5f * FLoop_fPreviousLoopSecs );
	m_fRotInterp += FLoop_fPreviousLoopSecs;
	
	FMATH_CLAMPMIN( fTime, 0.0f );
	FMATH_CLAMPMIN( fAnimTime, 0.0f );
	FMATH_CLAMPMAX( m_fRotInterp, 1.0f );

	m_pSplineActor->SetTimePos( fTime );
	m_pGurney->UserAnim_UpdateTime( fAnimTime );

	m_pSplineActor->GetMatrix( &CFMtx43A::m_Temp );
	RotMtx.Identity();
	RotMtx.RotateY( FMATH_FPOT( m_fRotInterp, 0.0f, FMATH_PI ) );

	CFMtx43A::m_Temp.Mul( RotMtx );
	m_pGurney->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );
}

// Taken from BotDispenser.cpp
void CMoveStage::_ArmStartAnimation( void ) {
	f32 fSecsBetweenParts, fSecsPerPart;

	m_pBuildMachine->UserAnim_Select( 1 );
	m_pBuildMachine->UserAnim_UpdateUnitTime( 0.0f );
	m_pBuildMachine->UserAnim_Pause( FALSE );
	m_pBuildMachine->UserAnim_SetSpeedMult( 1.0f );

	CFXMeshBuildPartMgr::CalculateDropSecs( _STAGE2_BUILD_TIME,
		_STAGE2_OVERLAP_TIME,
		(f32) m_pPartBuilder->GetTotalPartCount(),
		fSecsBetweenParts,
		fSecsPerPart );

	CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_BUILD_ARM_SPARKS ) ),
					0.0f, 1.0f, _STAGE2_BUILD_TIME, 0.2f, TRUE );

	CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_ARMS_WORK ), TRUE ),
		0.0f, 1.0f, _STAGE2_BUILD_TIME, 0.2f, TRUE );

//	CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_ARMS_WORK ), FALSE, &CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos ), 
//		0.0f, 1.0f, _STAGE2_BUILD_TIME, 0.1f, TRUE );

	m_fBotArm_UnitSecsTillFirstContact = fmath_Div( fSecsPerPart, _STAGE2_BUILD_TIME );
	m_fBotArm_OODropNormalizer = fmath_Inv( 1.0f - m_fBotArm_UnitSecsTillFirstContact );

	m_fSecsTillNextSpark = fSecsPerPart + ( fSecsBetweenParts * 2.0f ); // * 2.0f to slow down the sparks
	m_fSecsBetweenPartLandings = fSecsBetweenParts; // * 2.0f to slow down the sparks
}

void CMoveStage::_ArmUpdateAnimation( const f32 &fUnitDropTime ) {
	f32 fUnitAnimTime;

	if( fUnitDropTime >= m_fBotArm_UnitSecsTillFirstContact ) {
		// playing the rest of the drop animations
//		if( fUnitDropTime < 1.0f ) {
//            ToggleArmSound( TRUE );				
//		}
		fUnitAnimTime = fUnitDropTime - m_fBotArm_UnitSecsTillFirstContact;
		fUnitAnimTime *= m_fBotArm_OODropNormalizer;
		fUnitAnimTime *= m_fArmAnim_PercentOfAnimPartDrop;
		fUnitAnimTime += m_fArmAnim_PercentOfAnim1stContact;
	} else {
		// waiting for the 1st piece to finish dropping
		fUnitAnimTime = fmath_Div( fUnitDropTime, m_fBotArm_UnitSecsTillFirstContact );
		fUnitAnimTime *= m_fArmAnim_PercentOfAnim1stContact;
	}

	FMATH_CLAMP( fUnitAnimTime, 0.0f, ( m_fArmAnim_PercentOfAnim1stContact + m_fArmAnim_PercentOfAnimPartDrop ) );
	m_pBuildMachine->UserAnim_UpdateUnitTime( fUnitAnimTime );

	m_fSecsTillNextSpark -= FLoop_fPreviousLoopSecs;
	
	if( m_fSecsTillNextSpark < 0.0f && fUnitAnimTime < m_fArmAnim_PercentOfAnimPartDrop ) {
		if( m_hSparkParticleDef != FPARTICLE_INVALID_HANDLE ) {
			CFVec3A Pos = CFVec3A::m_UnitAxisY;
			f32 fHeight;

			fHeight = FMATH_FPOT( fUnitDropTime, 0.0f, CSpyVsSpy::GetGlitch()->m_fCollCylinderHeight_WS );
			Pos.Mul( fHeight );
			Pos.Add( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );

			fparticle_SpawnEmitter( m_hSparkParticleDef, Pos.v3, NULL, 1.0f );
		}

		m_fSecsTillNextSpark += ( m_fSecsBetweenPartLandings * 2.0f ); // * 2.0f to slow down the sparks
	}
}

void CMoveStage::_ArmEndAnimation( void ) {

}

void CMoveStage::_SetupMoverParts( void ) {
	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_PART_CLATTER ) );

	m_pLegs->UserAnim_Pause( TRUE );
	m_pTorso->UserAnim_Pause( TRUE );

	m_pSplineActor->Start();

	m_pLegs->Attach_ToParent_WS( m_pGurney, _STAGE2_PART_BONE_NAME, FALSE );
	m_pTorso->Attach_ToParent_WS( m_pGurney, _STAGE2_PART_BONE_NAME, FALSE );

	m_pGurney->UserAnim_Pause( FALSE );
	m_pGurney->UserAnim_SetClampMode( TRUE );
	m_StageCamera.SetGurneyMode( m_pPartBone );

	m_eStageState = STATE_ON_GURNEY;

	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_GURNEY_ARM_MOVE_UP ), FALSE, &m_pGurney->MtxToWorld()->m_vPos );

	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_GURNEY_MOVE ), TRUE );

	if( pEmitter ) {
		pEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
		CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, _STAGE2_GURNEY_MOVE_DOOR_TIME, 0.25f, TRUE );
	}
}

void CMoveStage::_StartGlitchBuild( void ) {
	Player_aPlayer[0].m_uPlayerFlags |= CPlayer::PF_DONT_CALL_WORK;

	m_pBuildMachine->SetCollisionFlag( FALSE );
	CSpyVsSpy::GetGlitch()->m_pWorldMesh->SetCollisionFlag( FALSE );
	CSpyVsSpy::GetGlitch()->RelocateAllChildren( FALSE );	

	m_pPartBuilder = CFXMeshBuildPartMgr::InitMgr( m_DropStart, CSpyVsSpy::GetGlitch(), CSpyVsSpy::GetGlitch()->m_pWorldMesh, _STAGE2_BUILD_TIME, _STAGE2_OVERLAP_TIME );

	_ArmStartAnimation();

	// Lock the doors so we can't go back
	m_pDoor1->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );
	m_pDoor2->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );
	m_pDoor1->SetLockState( TRUE );
	m_pDoor2->SetLockState( TRUE );

	CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CONVEYOR_LOOP ) );
	CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, 3.0f + ( m_pBuildMachine->UserAnim_GetCurrentInst()->GetTotalTime() * fmath_Inv( _STAGE2_MACHINE_TIMESCALE ) ), 0.1f, TRUE );
}

void CMoveStage::_BuildWork( void ) {
	if( !CSpyVsSpy::GetGlitch()->IsDrawEnabled() ) {
		CSpyVsSpy::GetGlitch()->DrawEnable( TRUE, TRUE );
	}

	if( m_pPartBuilder ) {
		f32 fUnitPercentBuilt;

		// We are all done with this stage
		if( m_pPartBuilder->Work( fUnitPercentBuilt ) ) {
			m_pPartBuilder = NULL;
			CSpyVsSpy::GetGlitch()->m_pWorldMesh->SetCollisionFlag( TRUE );
			m_pBuildMachine->SetCollisionFlag( TRUE );
		}
	
		_ArmUpdateAnimation( fUnitPercentBuilt );
		CSpyVsSpy::GetGlitch()->RelocateAllChildren( FALSE );	

		if( ( fUnitPercentBuilt >= 0.4f ) && ( m_eStageState != STATE_CAM_TO_GLITCH ) ) {
			m_StageCamera.CamToGlitch();
			m_eStageState = STATE_CAM_TO_GLITCH;
		}

		if( ( fUnitPercentBuilt >= 0.8f ) && m_pMachineEmitter ) {
			CEmitterFader::Fade( m_pMachineEmitter, 1.0f, 0.0f, 0.25f, TRUE );
			m_pMachineEmitter = NULL;
		}
	}
}

#define _STAGE3_EXPLOSION_NAME						( "MervSpawn" )
#define _STAGE3_GUY									( "evil_nate" )
#define _STAGE3_LOWER_SPLINE						( "nates_spline2" )
#define _STAGE3_UPPER_SPLINE						( "nates_spline" )
#define _STAGE3_DOOR								( "nates_evildoor01" )
#define _STAGE3_EXIT_DOOR							( "olstoppy" )
#define _STAGE3_LOOK_OUT_POINT						( "pickup1" )
#define _STAGE3_CRANE_NAME							( "ddrcrane" )
#define _STAGE3_CRANE_BONE							( "cranedummy" )
#define _STAGE3_LOCKER_ANIM_RATEMUL					( 5.0f )
#define _STAGE3_JUNK_COUNT_MIN						( 10 )
#define _STAGE3_JUNK_COUNT_MAX						( 15 )
#define _STAGE3_LOCKER_OPEN_TIME					( 3.0f )
#define _STAGE3_FIRST_FLOOR_SPLINE_TIME				( 10.0f )
#define _STAGE3_OO_FIRST_FLOOR_SPLINE_TIME			( 1.0f / _STAGE3_FIRST_FLOOR_SPLINE_TIME )
#define _STAGE3_SECOND_FLOOR_SPLINE_TIME			( 5.0f )
#define _STAGE3_REST_TIME_BOT						( 3.0f )
#define _STAGE3_REST_TIME_LOCKER					( 2.0f )
#define _STAGE3_REST_TIME_CONSOLE					( 5.0f )
#define _STAGE3_REST_TIME_ELEVATOR_FIRST_FLOOR		( 4.0f )
#define _STAGE3_REST_TIME_ELEVATOR_SECOND_FLOOR_MIN	( 3.0f )
#define _STAGE3_REST_TIME_ELEVATOR_SECOND_FLOOR_MAX ( 5.0f )
#define _STAGE3_MIN_DOT_SIZE						( 0.85f )
#define _STAGE3_SAFE_RADIUS							( 4.0f )
#define _STAGE3_DEATH_TIME							( 1.0f ) // How long we can be considered dead until we really die
#define _STAGE3_CLOSE_ENOUGH						( 6.0f )
#define _STAGE3_GRAB_TIME							( 38.0f * _OO_ANIM_FPS )
#define _STAGE3_LET_GO_TIME							( 80.0f * _OO_ANIM_FPS )
#define _STAGE3_DELAY_CHASE_UNIT					( -1.0f )
#define _STAGE3_TIME_TILL_CHASE						( 4.0f )
#define _STAGE3_MIN_OPEN_TRY						( 4 )
// If you change this, change it in DDR
#define _STAGE3_CRANE_TIMESCALE						( 0.25f )
#define _STAGE3_OO_CRANE_TIMESCALE					( 1.0f / _STAGE3_CRANE_TIMESCALE )
#define _STAGE3_LOCKERID							( 8 )
#define _STAGE3_LOCKER_GREEN						( 0 )
#define _STAGE3_LOCKER_RED							( 1 )
#define _STAGE3_GOTO_TIMEOUT						( 10.0f )
#define _STAGE3_AMBIENT_VOL							( 0.5f )
#define _STAGE3_IDLE_MACHINE_VOL					( 0.5f )

// If you change these, make sure to set the values to NULL below
#define _STAGE3_NUM_JUNK_TYPES		( 7 )
static cchar *_apszJunkTypes[] = {
	"JunkMed",
	"MechMed",
	"GruntChunks",
	"GlassMed",
	"GlitchChunks",
	"ZomChunks",
	"PredGuts",
};

//
//
//
// CLocker
//
//
//
FCLASS_ALIGN_PREFIX class CLocker {
public:
	CLocker()			{ }
	virtual ~CLocker()	{ }

	FINLINE BOOL IsOpen( void )				{ return m_pMesh->UserAnim_GetCurrentInst()->GetTime() > 0.0f; }
	FINLINE CFMtx43A *GetMtx( void )		{ return m_pMesh->MtxToWorld(); }
	FINLINE void SetHaveChip( BOOL bChip )	{ m_eLockerType = bChip ? LOCKER_CHIP : LOCKER_JUNK; }
	FINLINE void SetNotAllowedChip( void )	{ m_bCanHaveChip = FALSE; }
	FINLINE BOOL CanHaveChip( void )		{ return m_bCanHaveChip; }
	FINLINE CMeshEntity *GetMesh( void )	{ return m_pMesh; }

	BOOL Load( u32 uNdx, BOOL bHaveChip );
	void Restore( void );
	void Unload( void );

	void Work( CHumanControl *pControl, u32 &uNumTriedOpen );

	void ForceClose( void );

public:
	typedef enum {
		LOCKER_NONE = 0,
		LOCKER_CHIP,
		LOCKER_JUNK,
	} LockerType_e;

protected:
	void _ClearDataMembers( void );
	void _OpenLocker( u32 uNumOpenTries );
	void _CloseLocker( void );

	void _SpawnChip( void );
	void _SpawnExplosion( void );
	void _SpawnDebris( void );
	
protected:
	BOOL8 m_bCanHaveChip;
	BOOL8 m_bBeenOpened;
	BOOL8 _PAD[2];

	LockerType_e m_eLockerType;
	CMeshEntity *m_pMesh;

	FMeshTexLayerHandle_t m_hLightTexLayer;

	static FExplosion_GroupHandle_t m_hExplosionHandle;
	static CFDebrisGroup *m_apJunkDebrisGroups[ _STAGE3_NUM_JUNK_TYPES ];

	FCLASS_STACKMEM_ALIGN( CLocker );
} FCLASS_ALIGN_SUFFIX;

FExplosion_GroupHandle_t CLocker::m_hExplosionHandle = FEXPLOSION_INVALID_HANDLE;
CFDebrisGroup *CLocker::m_apJunkDebrisGroups[ _STAGE3_NUM_JUNK_TYPES ] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };

BOOL CLocker::Load( u32 uNdx, BOOL bHaveChip ) {
	CEntity *pEntity;
	char aBuff[64];

	_ClearDataMembers();

	sprintf( aBuff, "locker%02d", uNdx );

	pEntity = CEntity::FindInWorld( aBuff );

	if( !pEntity ) {
		DEVPRINTF( "CLocker::Load(): Failed to find '%s'.\n", aBuff );
		goto _ExitWithError;
	}

	if( !( pEntity->TypeBits() & ENTITY_BIT_MESHENTITY ) ) {
		DEVPRINTF( "CLocker::Load(): Entity '%s' is not a mesh entity.\n", aBuff );
		goto _ExitWithError;
	}

	m_pMesh = (CMeshEntity *) pEntity;

	if( bHaveChip ) {
		m_eLockerType = LOCKER_CHIP;
//		CCollectable::NotifyCollectableUsedInWorld( COLLECTABLE_CHIP );
	} else {
		m_eLockerType = LOCKER_JUNK;

		// ALways try to load since we always use
		if( m_hExplosionHandle == FEXPLOSION_INVALID_HANDLE ) {
			m_hExplosionHandle = CExplosion2::GetExplosionGroup( _STAGE3_EXPLOSION_NAME );
		}

		if( !m_apJunkDebrisGroups[0] ) {
			for( u32 i = 0; i < _STAGE3_NUM_JUNK_TYPES; ++i ) {
				m_apJunkDebrisGroups[ i ] = CFDebrisGroup::Find( _apszJunkTypes[ i ] );
			}
		}
	}

	m_hLightTexLayer = m_pMesh->GetMesh()->GetTexLayerHandle( _STAGE3_LOCKERID );

	if( m_hLightTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) {
		DEVPRINTF( "CLocker::Load():  Error getting tex layer handle for locker, ID %d\n", _STAGE3_LOCKERID );
	} else {
		m_pMesh->GetMesh()->TexFlip_AnimateFlip( m_hLightTexLayer, FALSE );
		m_pMesh->GetMesh()->TexFlip_SetFlipPage( m_hLightTexLayer, _STAGE3_LOCKER_GREEN );
	}

	Restore();

	return TRUE;

_ExitWithError:
	return FALSE;
}

// Put everything back into its initial state
void CLocker::Restore( void ) {
	FASSERT( m_pMesh );

	m_bBeenOpened = FALSE;
	m_bCanHaveChip = TRUE;

	m_pMesh->UserAnim_Pause( TRUE );
	m_pMesh->UserAnim_SetSpeedMult( _STAGE3_LOCKER_ANIM_RATEMUL );
	m_pMesh->UserAnim_UpdateUnitTime( 0.0f );
	m_pMesh->UserAnim_SetClampMode( TRUE );

	m_pMesh->GetMesh()->TexFlip_SetFlipPage( m_hLightTexLayer, _STAGE3_LOCKER_GREEN );

	m_eLockerType = LOCKER_JUNK;
}

void CLocker::Unload( void ) {
	m_hExplosionHandle = FEXPLOSION_INVALID_HANDLE;
	
	for( u32 i = 0; i < _STAGE3_NUM_JUNK_TYPES; ++i ) {
		m_apJunkDebrisGroups[ i ] = NULL;
	}
}

void CLocker::Work( CHumanControl *pControl, u32 &uNumTriedOpen ) {
	FASSERT( pControl );

	CBot *pInteractor = CSpyVsSpy::GetGlitch();
#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() ) {
		for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
			CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
			CHumanControl *pHuman = &Player_aPlayer[n].m_HumanControl;
			if( pBot && Player_aPlayer[n].HasEntityControl() && !pBot->IsDeadOrDying() &&
				(pHuman->m_nPadFlagsAction & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT) &&
				m_pMesh->GetBoundingSphere_WS().IsIntersecting( pBot->m_pWorldMesh->GetBoundingSphere() ) ) {
				pInteractor = pBot; pControl = pHuman; break;
			}
		}
	}
#endif
	// Only check for intersection when we have an action
	if( ( pControl->m_nPadFlagsAction & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT ) ) {
		CFVec3A Front = m_pMesh->MtxToWorld()->m_vFront;
		Front.Mul( -1.0f );

		if( pInteractor->MtxToWorld()->m_vFront.Dot( Front ) > 0.0f ) {
			return;
		}

		if( !m_pMesh->GetBoundingSphere_WS().IsIntersecting( pInteractor->m_pWorldMesh->GetBoundingSphere() ) ) {
			return;
		}

		if( !IsOpen() && m_pMesh->UserAnim_GetCurrentInst()->GetUnitTime() == 0.0f ) {
			_OpenLocker( uNumTriedOpen );

			if( !m_bBeenOpened && uNumTriedOpen < _STAGE3_MIN_OPEN_TRY ) {
				// Only count ones we havne't opened yet
				m_bBeenOpened = TRUE;
				++uNumTriedOpen;
			}
			return;
		} 
		
		if( IsOpen() && m_pMesh->UserAnim_GetCurrentInst()->GetUnitTime() >= 1.0f ) {
			_CloseLocker();
			return;
		}
	}
}

void CLocker::ForceClose( void ) {
	_CloseLocker();
}

void CLocker::_ClearDataMembers( void ) {
	m_pMesh = NULL;
	m_eLockerType = LOCKER_NONE;
	m_hLightTexLayer = FMESH_TEXLAYERHANDLE_INVALID;
	m_bCanHaveChip = TRUE;
	m_bBeenOpened = FALSE;
}

void CLocker::_OpenLocker( u32 uNumOpenTries ) {
	m_pMesh->UserAnim_Pause( FALSE );
	m_pMesh->UserAnim_SetSpeedMult( _STAGE3_LOCKER_ANIM_RATEMUL );

	if( uNumOpenTries < _STAGE3_MIN_OPEN_TRY ) {
		if( m_bCanHaveChip ) {
			SetNotAllowedChip();
			_SpawnExplosion();
			_SpawnDebris();
		}
	} else {
		switch( m_eLockerType ) {
			case LOCKER_CHIP:
				_SpawnChip();
			break;
			case LOCKER_JUNK:
				_SpawnExplosion();
				_SpawnDebris();
			break;
		}
	}

	m_eLockerType = LOCKER_NONE;

	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_LOCKER_OPEN ), FALSE, &m_pMesh->MtxToWorld()->m_vPos );
	m_pMesh->GetMesh()->TexFlip_SetFlipPage( m_hLightTexLayer, _STAGE3_LOCKER_RED );
}

void CLocker::_CloseLocker( void ) {
	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_LOCKER_CLOSE ), FALSE, &m_pMesh->MtxToWorld()->m_vPos );
	m_pMesh->UserAnim_SetSpeedMult( -_STAGE3_LOCKER_ANIM_RATEMUL );

	m_pMesh->GetMesh()->TexFlip_SetFlipPage( m_hLightTexLayer, _STAGE3_LOCKER_GREEN );
}

void CLocker::_SpawnChip( void ) {
	CFMtx43A::m_Temp.Identity();
	
	CFMtx43A::m_Temp.m_vPos = m_pMesh->MtxToWorld()->m_vFront;
	CFMtx43A::m_Temp.m_vPos.Mul( -2.5f );
	CFMtx43A::m_Temp.m_vPos.Add( m_pMesh->GetBoundingSphere_WS().m_Pos );

	CFVec3A Vel;

	Vel = m_pMesh->MtxToWorld()->m_vFront;
	Vel.Mul( -fmath_RandomFloatRange( 5.0f, 15.0f ) );

	CCollectable::PlaceIntoWorld( COLLECTABLE_CHIP, &CFMtx43A::m_Temp, &Vel );
}

void CLocker::_SpawnExplosion( void ) {
	if( m_hExplosionHandle != FEXPLOSION_INVALID_HANDLE ) {
		FExplosion_SpawnerHandle_t hSpawn = CExplosion2::GetExplosionSpawner();

		if( hSpawn != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();
			
			SpawnParams.uFlags |= FEXPLOSION_SPAWN_NO_SOUND;

			SpawnParams.Pos_WS.v3 = m_pMesh->GetBoundingSphere_WS().m_Pos;
			SpawnParams.Pos_WS.Sub( m_pMesh->MtxToWorld()->m_vFront );
			SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;

			CExplosion2::SpawnExplosion( hSpawn, m_hExplosionHandle, &SpawnParams );
		}
	}
}

void CLocker::_SpawnDebris( void ) {
	CFDebrisGroup *pGroup = NULL;

	pGroup = m_apJunkDebrisGroups[ fmath_RandomRange( 0, _STAGE3_NUM_JUNK_TYPES - 1 ) ];

	if( !pGroup ) {
		return;
	}

	CFDebrisSpawner DebrisSpawner;
	DebrisSpawner.InitToDefaults();

	DebrisSpawner.m_Mtx.m_vPos.v3 = m_pMesh->MtxToWorld()->m_vFront.v3;
	DebrisSpawner.m_Mtx.m_vPos.Mul( -1.5f );
	DebrisSpawner.m_Mtx.m_vPos.Add( m_pMesh->GetBoundingSphere_WS().m_Pos );

	DebrisSpawner.m_Mtx.m_vZ = m_pMesh->MtxToWorld()->m_vFront.v3;
	DebrisSpawner.m_Mtx.m_vZ.Mul( -1.0f );

	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
	DebrisSpawner.m_pDebrisGroup = pGroup;
	DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
	DebrisSpawner.m_fMinSpeed = 5.0f;
	DebrisSpawner.m_fMaxSpeed = 15.0f;
	DebrisSpawner.m_fUnitDirSpread = 1.0f;
	DebrisSpawner.m_fScaleMul = 1.0f;
	DebrisSpawner.m_fGravityMul = 1.0f;
	DebrisSpawner.m_fRotSpeedMul = 1.0f;
	DebrisSpawner.m_nMinDebrisCount = _STAGE3_JUNK_COUNT_MIN;
	DebrisSpawner.m_nMaxDebrisCount = _STAGE3_JUNK_COUNT_MAX;
	DebrisSpawner.m_pFcnCallback = NULL;

	CGColl::SpawnDebris( &DebrisSpawner );
}

//
//
//
// CSearchStage - 
//
//
//
FCLASS_ALIGN_PREFIX class CSearchStage : public CSpyVsSpyStage { 
//----------------------------------------------------------------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CSearchStage();
	virtual ~CSearchStage() { }

	BOOL Load( LevelEvent_e eEvent );
	void Unload( void );
	void Restore( void );
	void Work( void );
	void SwitchTo( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private definitions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum {
		NUM_LOCKERS = 18,
	};

	typedef enum {
		GUY_STATE_NONE,
		GUY_STATE_WAIT_TALK,
		GUY_STATE_WAIT_FIRST_WORK,
		GUY_STATE_WALK_AWAY,			// Walking away from the player
		GUY_STATE_WALK_CONSOLE,			// Walking to his console
		GUY_STATE_WALK_AWAY_CONSOLE,	// Walking away from his console
		GUY_STATE_WALK_BACK,			// Walking back to the player
		GUY_STATE_RESTING,				// Guy is resting, how nice
		GUY_STATE_DONE,					// Guy is done inspecting and walking about
		GUY_STATE_CRANE,				// Crane grabbing Glitch
		GUY_STATE_CLOSE_LOCKER,			// Guy is going to close a locker
		GUY_STATE_FAULTY_GLITCH,		// Guy walks to Glitch and kills him
		GUY_STATE_FAKE_DDR,
		GUY_STATE_MOVE_CAM_BACK,
		GUY_STATE_KILL_GLITCH,
	} GuyState_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );
	void _SetGuyState( GuyState_e eState );
	void _SetGuyRestState( GuyState_e eNextState, f32 fTime );
	void _WorkGuyState( u32 uNumLockersOpen );
	void _CheckDeath( void );
	void _KillGlitch( void );
	void _CloseInspectionLockers( void );

#if FANG_WINGC
public:
 BOOL CoopIndividualViews() const { return m_bCoopEscort; }
 BOOL PortTestEscort(u32 test);
 BOOL PortTestInspection(u32 test);
private:
 BOOL m_bCoopEscort;
 void _CoopEscort(BOOL start);
 GuyState_e _InspectionFailureState(GuyState_e state);
#endif
	void _CameraChaseWork( void );
	void _CameraSpinWork( void );

	const CFVec3A &_GetStartPoint( CESpline *pSpline );
	const CFVec3A &_GetEndPoint( CESpline *pSpline );

	BOOL _IsAtGotoPoint( const f32 &fRad );

//----------------------------------------------------------------------------------------------------------------------------------
// Private data
//----------------------------------------------------------------------------------------------------------------------------------
private:
	CFVec3A m_SpawnPoint;
	CFVec3A m_SafeDirection;

	u32 m_uNumLockersTried;

	CHumanControl *m_pHumanControl;
	CLocker m_Lockers[NUM_LOCKERS];

	f32 m_fRestTime;
	f32 m_fDeathTimer;
	f32 m_fGotoTimer;
	CBotScientist *m_pGuy;
	GuyState_e m_eGuyState;
	GuyState_e m_eGuyStatePrev;
	GuyState_e m_eGuyStateNext;

	CESpline *m_pSplineFirstFloor;
	CESpline *m_pSplineSecondFloor;

	CDoorEntity *m_pDoor;
	CDoorEntity *m_pExitDoor;

	CFVec3A m_GotoPoint;
	CFVec3A m_EndOfStagePoint;

	BOOL m_bWarnedAboutLocker;

	CMeshEntity *m_pCrane;
	CMeshEntity *m_pBuildMachine;

	CCamSimpleInfo m_Camera;

	BOOL8 m_bChaseCam; // USed for the chase cam and the DDR cam
	BOOL8 m_bGotoGlitch;
	BOOL8 m_bChipPlaced;
	BOOL8 m_bSpinCam;

	f32 m_fUnitChase;
	f32 m_fTimeTillChase;
	f32 m_fCamTimeScale;
	
	CFVec3A m_ChasePos;
	CFVec3A m_StartPos;
	CFQuatA m_ChaseQuat;
	CFQuatA m_StartQuat;

	CFAudioEmitter *m_pAmbientEmitter;
	CFAudioEmitter *m_pIdleMachine;
	FCLASS_STACKMEM_ALIGN( CSearchStage );
} FCLASS_ALIGN_SUFFIX;

CSearchStage::CSearchStage() { 
	_ClearDataMembers();
}

BOOL CSearchStage::Load( LevelEvent_e eEvent ) { 
	if( eEvent == LEVEL_EVENT_POST_WORLD_LOAD ) {
		_ClearDataMembers();

		for( u32 i = 0; i < NUM_LOCKERS; ++i ) {
			if( !m_Lockers[i].Load( i + 1, FALSE ) ) { 
				goto _ExitWithError;
			}
		}

		m_uNumLockersTried = 0;

		CEntity *pEntity = NULL;

		// Find the drop point so we know where to put Glitch
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_DROP_END_NAME ) ) ) {
			goto _ExitWithError;
		}

		m_SpawnPoint.Set( pEntity->MtxToWorld()->m_vPos );

		m_eGuyState = GUY_STATE_WAIT_TALK;
		m_eGuyStatePrev = GUY_STATE_NONE;

		// Find the door
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE3_DOOR, ENTITY_BIT_DOOR ) ) ) {
			goto _ExitWithError;
		}

		m_pDoor = (CDoorEntity *) pEntity;

		// Find the exit doors
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE3_EXIT_DOOR, ENTITY_BIT_DOOR ) ) ) {
			goto _ExitWithError;
		}

		m_pExitDoor = (CDoorEntity *) pEntity;
		//m_pExitDoor->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );
		m_pExitDoor->SnapToPos( CDoorEntity::DOORSTATE_ZERO );

		// Find the splines
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE3_LOWER_SPLINE, ENTITY_BIT_SPLINE ) ) ) { 
			goto _ExitWithError;
		}

		m_pSplineFirstFloor = (CESpline *) pEntity;

		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE3_UPPER_SPLINE, ENTITY_BIT_SPLINE ) ) ) { 
			goto _ExitWithError;
		}

		m_pSplineSecondFloor = (CESpline *) pEntity;

		// End of stage point
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE3_LOOK_OUT_POINT, ENTITY_BIT_SPHERE )  ) ) {
			goto _ExitWithError;
		}

		m_EndOfStagePoint = pEntity->MtxToWorld()->m_vPos;

		// Find the crane in the world
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE3_CRANE_NAME, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}

		m_pCrane = (CMeshEntity *) pEntity;

		// Find the build machine
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE2_ASSEMBLY_MACHINE, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}

		m_pBuildMachine = (CMeshEntity *) pEntity;
	} else if( eEvent == LEVEL_EVENT_PRE_ENTITY_FIXUP ) {
		_SetFinished( FALSE );

		FASSERT( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex >= 0 );
		m_pHumanControl = (CHumanControl *) CSpyVsSpy::GetGlitch()->Controls();

		CEntity *pEntity;

		// Find the tech. guy
		if( !( pEntity = CSpyVsSpy::FindEntity( _STAGE3_GUY, ENTITY_BIT_BOTGRUNT ) ) ) {
			goto _ExitWithError;
		}

		m_pGuy = (CBotScientist *) pEntity;
		CSpyVsSpy::MakeBotDumb( m_pGuy, 1.0f );
		m_pGuy->AIBrain()->GetAIMover()->m_uMoverFlags &= ~( CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS | CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE );
	}

	return TRUE;

_ExitWithError:
	Unload();
	_ClearDataMembers();

	return FALSE;
}

void CSearchStage::Unload( void ) { 
	if( m_pAmbientEmitter ) {
		m_pAmbientEmitter->Destroy();
		m_pAmbientEmitter = NULL;
	}

	if( m_pIdleMachine ) {
		m_pIdleMachine->Destroy();
		m_pIdleMachine = NULL;
	}

	for( u32 i = 0; i < NUM_LOCKERS; ++i ) {
		m_Lockers[i].Unload();
	}

	_ClearDataMembers();
}

void CSearchStage::Restore( void ) {
	if( m_pAmbientEmitter ) {
		m_pAmbientEmitter->Destroy();
		m_pAmbientEmitter = NULL;
	}

	if( m_pIdleMachine ) {
		m_pIdleMachine->Destroy();
		m_pIdleMachine = NULL;
	}

	SwitchTo();

	for( u32 i = 0; i < NUM_LOCKERS; ++i ) {
		m_Lockers[i].Restore();
	}

	_SetFinished( FALSE );
}

void CSearchStage::Work( void ) { 
	u32 uNumOpen = 0;

	for( u32 i = 0; i < NUM_LOCKERS; ++i ) {
		m_Lockers[i].Work( m_pHumanControl, m_uNumLockersTried );

		if( m_Lockers[i].IsOpen() ) {
			++uNumOpen;
		}

		if( !m_bChipPlaced && m_uNumLockersTried == _STAGE3_MIN_OPEN_TRY ) {
			u32 uRand = (u32) fmath_RandomRange( 0, NUM_LOCKERS - 1 );
			u32 uMaxTries = 100;

			while( uMaxTries ) {
				if( m_Lockers[ uRand ].CanHaveChip() ) {
					m_Lockers[ uRand ].SetHaveChip( TRUE );
					break;
				}

				--uMaxTries;
			}

			// Just in case we really fail badly
			if( !uMaxTries ) {
				m_Lockers[ 0 ].SetHaveChip( TRUE );
			}

			m_bChipPlaced = TRUE;
		}
	}

	_WorkGuyState( uNumOpen );
	_CheckDeath();

	if( m_bChaseCam && m_eGuyState == GUY_STATE_CRANE ) {
		_CameraChaseWork();
	}

	if( m_bSpinCam ) {
		_CameraSpinWork();
	}
}

void CSearchStage::SwitchTo( void ) {
#if FANG_WINGC
 _CoopEscort(FALSE);
 // Rebuilding relocates P1; start its scripted inspection with a neutral,
 // owning view even if a partner was aiming upward in the preceding scene.
 if(MultiplayerMgr.IsLocalCoop()) {
  CSpyVsSpy::GetGlitch()->m_fMountPitch_WS=0;

  fcamera_GetCameraByIndex(0)->SetViewSource(NULL);
 }
#endif
	CFSphere CollSphere;

	// !!Nate - need to remember his original radius somehere, globally so we can share it
	CollSphere.m_Pos.x = m_SpawnPoint.x;
	CollSphere.m_Pos.y = m_SpawnPoint.y;
	CollSphere.m_Pos.z = m_SpawnPoint.z;
	CollSphere.m_fRadius = _STAGE1_BIG_GLITCH_RADIUS;

	CSpyVsSpy::GetGlitch()->GetMesh()->MoveTracker( CollSphere );
	CSpyVsSpy::GetGlitch()->MtxToWorld()->Identity();
	CSpyVsSpy::GetGlitch()->MtxToWorld()->RotateY( FMATH_PI );
	CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos = m_SpawnPoint;
	CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &m_SpawnPoint );

	CSpyVsSpy::GetGlitch()->MobilizeBot();

	m_SafeDirection = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vFront;

	_SetGuyState( GUY_STATE_WAIT_TALK/*GUY_STATE_WAIT_FIRST_WORK*/ );

#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() ) {
		m_pGuy->IgnoreBotVBotCollision();
		m_pGuy->AIBrain()->GetAIMover()->m_uMoverFlags &= ~CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;
		m_pGuy->AIBrain()->GetAIMover()->m_uMoverFlags |= CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS;
	} else
#endif
	{
		m_pGuy->DontIgnoreBotVBotCollision();
		m_pGuy->AIBrain()->GetAIMover()->m_uMoverFlags &= ~( CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS | CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE );
	}
    ai_TurnOffPerceptor( m_pGuy->AIBrain(), AI_PERCEPTOR_EYES );
	ai_TurnOffPerceptor( m_pGuy->AIBrain(), AI_PERCEPTOR_EARS );
	ai_TurnOffPerceptor( m_pGuy->AIBrain(), AI_PERCEPTOR_TOUCH );
	ai_TurnOffPerceptor( m_pGuy->AIBrain(), AI_PERCEPTOR_RADIO );

	m_pDoor->m_uBehaviorCtrlFlags |= CDoorEntity::DOOR_BEHAVIOR_CTRL_AUTOCLOSE;

	CSpyVsSpy::GetGlitch()->DrawEnable( TRUE, TRUE );

	// !!Nate - need to put these back at the end of the stage
	CSpyVsSpy::GetGlitch()->SetBotFlag_DontUseIdleAnimations();

	m_pCrane->UserAnim_Pause( TRUE );
	m_pCrane->UserAnim_UpdateTime( 0.0f );
	m_pCrane->UserAnim_Select( 0 );
	m_pCrane->UserAnim_SetClampMode( TRUE );
	m_pCrane->UserAnim_SetSpeedMult( _STAGE3_CRANE_TIMESCALE );
	m_pCrane->SetCollisionFlag( FALSE );

	m_bChaseCam = FALSE;
	m_bGotoGlitch = FALSE;
	m_bSpinCam = FALSE;
	m_bChipPlaced = FALSE;
	m_uNumLockersTried = 0;

	m_fCamTimeScale = 1.0f;

	m_bWarnedAboutLocker = FALSE;

	// Trick the HUD
	CHud2 *pHud = CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );

	// Hack so we can draw the chip that we find
	CSpyVsSpy::TurnOffHUD( FALSE );
	// Turn them all off except for batteries
	pHud->DisableDrawFlags( CHud2::DRAW_GLITCH_HUD );
	pHud->EnableDrawFlags( CHud2::DRAW_ITEMS );
	CSpyVsSpy::GetGlitch()->ReticleEnable( FALSE );

	CSpyVsSpy::AttackDisable( TRUE );

	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );

	CInventory *pInv = CSpyVsSpy::GetGlitch()->m_pInventory;
	CItemInst *pPrimary = pInv->IsWeaponInInventory( "Empty Primary" );
	CItemInst *pSecondary = pInv->IsWeaponInInventory( "Wrench" );

	pInv->SetCurWeapon( INV_INDEX_PRIMARY, pPrimary, FALSE, TRUE );
	pInv->SetCurWeapon( INV_INDEX_SECONDARY, pSecondary, FALSE, TRUE );
#if FANG_WINGC
	CSpyVsSpy::CoopSyncStage( "Empty Primary", "Wrench", TRUE );
#endif

	_SetFinished( FALSE );
}

void CSearchStage::_ClearDataMembers( void ) {
#if FANG_WINGC
 m_bCoopEscort=FALSE;
#endif
	m_SpawnPoint.Zero();
	m_SafeDirection.Zero();

	m_uNumLockersTried = 0;

	m_pGuy = NULL;
	m_pSplineFirstFloor = NULL;
	m_pSplineSecondFloor = NULL;

	m_eGuyState = GUY_STATE_WALK_AWAY;
	m_eGuyStateNext = GUY_STATE_WALK_CONSOLE;
	m_eGuyStatePrev = GUY_STATE_NONE;

	m_fRestTime = -1.0f;

	m_GotoPoint.Zero();

	m_bWarnedAboutLocker = FALSE;

	m_bChaseCam = FALSE;
	m_bGotoGlitch = FALSE;
	m_bSpinCam = FALSE;
	m_fUnitChase = 0.0f;
	m_fTimeTillChase = 0.0f;
	m_fCamTimeScale = 1.0f;
	m_pCrane = NULL;
	m_ChasePos.Zero();
	m_ChaseQuat.Identity();
	m_StartPos.Zero();
	m_StartQuat.Identity();

	m_bChipPlaced = FALSE;

	m_fGotoTimer = 0.0f;

	m_pAmbientEmitter = NULL;
	m_pIdleMachine = NULL;
}

#if FANG_WINGC
CSearchStage::GuyState_e CSearchStage::_InspectionFailureState(GuyState_e state) {
 if(!MultiplayerMgr.IsLocalCoop() || (state!=GUY_STATE_FAULTY_GLITCH && state!=GUY_STATE_KILL_GLITCH)) return state;
 // Several open lockers and repeat violations bypass _CheckDeath. Handle
 // every inspection failure here, before taking control or enabling combat.
 for(u32 n=0;n<NUM_LOCKERS;++n) if(m_Lockers[n].IsOpen()) return GUY_STATE_CLOSE_LOCKER;
 return GUY_STATE_WALK_AWAY;
}
#endif

void CSearchStage::_CloseInspectionLockers( void ) {
 for(u32 n=0;n<NUM_LOCKERS;++n) {
  if(!m_Lockers[n].IsOpen()) continue;
  m_Lockers[n].ForceClose();
  CFSoundGroup::PlaySound(CSpyVsSpySoundCenter::GetGroup(CSpyVsSpySoundCenter::SOUND_LOCKER_CLOSE));
#if FANG_WINGC
  if(MultiplayerMgr.IsLocalCoop()) continue;
#endif
  break; // Retail solo warning closes its one offending locker.
 }
}

void CSearchStage::_SetGuyState( GuyState_e eState ) {
#if FANG_WINGC
 const GuyState_e safeState=_InspectionFailureState(eState);
 if(safeState!=eState) {
  DEVPRINTF("Co-op: factory inspection failure %d becomes warning/retry %d; chip search retained.\n",eState,safeState);
  eState=safeState;
  if(eState==GUY_STATE_WALK_AWAY) {
   m_bSpinCam=FALSE;
   m_pBuildMachine->DrawEnable(TRUE,TRUE);
   gamecam_SwitchPlayerTo3rdPersonCamera(PLAYER_CAM(0),CSpyVsSpy::GetGlitch());
   CSpyVsSpy::TakeControlFromPlayer(FALSE);
  }
 }
#endif
	switch( eState ) {
		case GUY_STATE_WALK_AWAY:
			m_GotoPoint = _GetEndPoint( m_pSplineFirstFloor );
			ai_AssignGoal_GotoWithLookAt( m_pGuy->AIBrain(), m_GotoPoint, CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos, 
				CSpyVsSpy::GetGlitch()->Guid(), 25, 5, 100, 0 );
		break;
		case GUY_STATE_WALK_BACK:
			m_GotoPoint = _GetStartPoint( m_pSplineFirstFloor );

			// We walk back to Glitch if we are at a locker, don't relocate 
			if( m_eGuyStatePrev != GUY_STATE_CLOSE_LOCKER ) {
				CFMtx43A::m_Temp = *m_pGuy->MtxToWorld();
				CFMtx43A::m_Temp.m_vPos.Zero();
				CFMtx43A::m_Temp.Mul( FMath_aRotMtx43A[ FMTX_ROTTYPE_180Y ] );
				CFMtx43A::m_Temp.m_vPos = _GetEndPoint( m_pSplineFirstFloor );

				m_pGuy->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );
				m_pDoor->SetLockState( FALSE );
			} else {
				m_fCamTimeScale = -0.5f;
			}

			ai_AssignGoal_GotoWithLookAt( m_pGuy->AIBrain(), m_GotoPoint, CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos, 
				CSpyVsSpy::GetGlitch()->Guid(), 5, 15, 100, 0 );
		break;
		case GUY_STATE_WALK_CONSOLE:
			m_GotoPoint = _GetEndPoint( m_pSplineSecondFloor );
			m_pGuy->Relocate_Xlat_WS( &_GetStartPoint( m_pSplineSecondFloor ) );
			ai_AssignGoal_Goto( m_pGuy->AIBrain(), m_GotoPoint, 5, 100, 0 );	
		break;
		case GUY_STATE_WALK_AWAY_CONSOLE:
			m_GotoPoint = _GetStartPoint( m_pSplineSecondFloor );
			ai_AssignGoal_Goto( m_pGuy->AIBrain(), m_GotoPoint, 5, 100, 0 );	
		break;
		case GUY_STATE_CLOSE_LOCKER: {
			u32 i;

			for( i = 0; i < NUM_LOCKERS; ++i ) {
				if( m_Lockers[i].IsOpen() ) {
					m_GotoPoint = m_Lockers[i].GetMtx()->m_vPos;
					m_GotoPoint.Sub( m_Lockers[i].GetMtx()->m_vFront );
					m_GotoPoint.Sub( m_Lockers[i].GetMtx()->m_vFront );
					m_GotoPoint.Add( m_Lockers[i].GetMtx()->m_vUp );
					ai_AssignGoal_Goto( m_pGuy->AIBrain(), m_GotoPoint, 10, 100, 0 );	
					break;
				}
			}

			FASSERT( i != NUM_LOCKERS );

			CFCamera *pCam = fcamera_GetCameraByIndex( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );
			CFVec3A CamToLocker;
			f32 fPlaneD;

			m_StartQuat.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
			m_StartPos = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;

			CamToLocker.Sub( m_Lockers[i].GetMtx()->m_vPos, pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos );
			CFMtx43A::m_Temp.UnitMtxFromNonUnitVec( &CamToLocker );
		
			m_pBuildMachine->DrawEnable( FALSE, TRUE );

			m_bSpinCam = TRUE;
			m_fCamTimeScale = 0.5f;
			m_fUnitChase = 0.0f;

			m_ChaseQuat.BuildQuat( CFMtx43A::m_Temp );
			m_ChasePos = CFVec3A::m_UnitAxisY;
			m_ChasePos.Mul( 8.0f );

			m_Lockers[ i ].GetMesh()->AppendTrackerSkipList();;

			fPlaneD = -m_StartPos.Dot( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vFront );

			// The locker is in front of the camera
			if( ( m_Lockers[i].GetMtx()->m_vPos.Dot( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vFront ) + fPlaneD ) >= 0.0f ) {
				CFVec3A Diff;

				Diff.Sub( _GetStartPoint( m_pSplineFirstFloor ), m_StartPos );
				Diff.y = 0.0f;

				m_ChasePos.Add( Diff );
			}
			
			m_ChasePos.Add( m_StartPos );

			m_Camera.m_Quat_WS = m_StartQuat;
			m_Camera.m_Pos_WS = m_StartPos;

			// Do this to trick the HUD
			CSpyVsSpy::TurnOffHUD( TRUE, TRUE );
			//CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->DisableDrawFlags( CHud2::DRAW_GLITCH_HUD );
			CSpyVsSpy::TakeControlFromPlayer( TRUE );
			CSpyVsSpy::TurnOffHUD( FALSE, TRUE );

			gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), &m_Camera );
		}
		break;
		case GUY_STATE_DONE: {
			CFVec3A DestPosGuy = CFVec3A::m_UnitAxisZ;

			m_GotoPoint = CFVec3A::m_UnitAxisX;
			m_GotoPoint.Mul( 8.0f );
			m_GotoPoint.Add( m_EndOfStagePoint );

			DestPosGuy.Mul( -6.0f );
			DestPosGuy.Add( m_GotoPoint );

			m_pExitDoor->SnapToPos( CDoorEntity::DOORSTATE_ONE );
			m_pGuy->IgnoreBotVBotCollision();
			m_pGuy->AIBrain()->GetAIMover()->m_uMoverFlags |= ( CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS | CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE );
			ai_AssignGoal_Goto( CSpyVsSpy::GetGlitch()->AIBrain(), m_GotoPoint, 1, 90, 0 );
#if FANG_WINGC
   _CoopEscort(TRUE);
#endif
			ai_AssignGoal_Goto( m_pGuy->AIBrain(), DestPosGuy, 1, 100, 0 );
		}
		break;
		case GUY_STATE_CRANE:{
#if FANG_WINGC
   _CoopEscort(FALSE);
#endif
			m_pCrane->UserAnim_UpdateTime( 0.0f );
			m_pCrane->UserAnim_Select( 0 );
			m_pCrane->UserAnim_Pause( FALSE );

			CSpyVsSpy::GetGlitch()->ImmobilizeBot();
						
			CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_LOOP );
			CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pCrane->MtxToWorld()->m_vPos ),
				0.0f, 1.0f, 
				m_pCrane->UserAnim_GetCurrentInst()->GetTotalTime() * _STAGE3_OO_CRANE_TIMESCALE, 0.1f, TRUE );
		}
		break;
		case GUY_STATE_FAULTY_GLITCH:
			m_GotoPoint = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos;
			ai_AssignGoal_GotoWithLookAt( m_pGuy->AIBrain(), m_GotoPoint, CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 25, 15, 100, 0 );
			CSpyVsSpy::TurnOffHUD( TRUE, TRUE );
			CSpyVsSpy::TakeControlFromPlayer( TRUE );
			CSpyVsSpy::TurnOffHUD( FALSE, TRUE );
		break;
		case GUY_STATE_NONE:

		break;
		case GUY_STATE_KILL_GLITCH:
//			CSpyVsSpy::TakeControlFromPlayer( FALSE );
			_KillGlitch();
		break;
		case GUY_STATE_WAIT_FIRST_WORK:
			ai_AssignGoal_GotoWithLookAt( m_pGuy->AIBrain(), _GetStartPoint( m_pSplineFirstFloor ), CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 0, 3, 100, 0 );
		break;
		case GUY_STATE_FAKE_DDR:{
			CDDRStage *pDDR = (CDDRStage *) CSpyVsSpy::m_pGame->m_pDanceStage;
			
			pDDR->m_uFlags |= CDDRStage::FLAG_SPECIAL_WATCH_MODE;
			pDDR->SwitchTo();

			m_bChaseCam = FALSE;

			CFCamera* pCam = fcamera_GetCameraByIndex( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );
			CFVec3A Dir;

			Dir.Set( 0.60694289f, -0.52893627f, 0.59316665f );
			CFMtx43A::m_Temp.Identity();
			CFMtx43A::m_Temp.UnitMtxFromUnitVec( &Dir );

			m_fUnitChase = 0.0f;

			m_Camera.m_Pos_WS = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
			m_Camera.m_Quat_WS.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
				
			m_ChasePos.Set( 262.71231f, -1.5636727f, 165.83513f );
			m_ChaseQuat.BuildQuat( CFMtx43A::m_Temp );

			m_StartPos = m_Camera.m_Pos_WS;
			m_StartQuat = m_Camera.m_Quat_WS;

			gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), &m_Camera );
			CSpyVsSpy::TakeControlFromPlayer( TRUE );
		}
		break;
		case GUY_STATE_MOVE_CAM_BACK:{
			m_fUnitChase = 0.0f;

			m_StartPos = m_Camera.m_Pos_WS;
			m_StartQuat = m_Camera.m_Quat_WS;

			CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &m_EndOfStagePoint );

			gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );
			fcamera_Work();

			CFCamera* pCam = fcamera_GetCameraByIndex( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );

			m_ChasePos = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
			m_ChaseQuat.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
		
			gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), &m_Camera );
			CSpyVsSpy::TakeControlFromPlayer( TRUE );

			//ai_AssignGoal_Goto( CSpyVsSpy::GetGlitch()->AIBrain(), m_EndOfStagePoint, 1, 80, 0 );
		}
		break;
	}

	m_fGotoTimer = _STAGE3_GOTO_TIMEOUT;

	m_eGuyStatePrev = m_eGuyState;
	m_eGuyState = eState;
}

#if FANG_WINGC
void CSearchStage::_CoopEscort(BOOL start) {
 if(!MultiplayerMgr.IsLocalCoop() || (!start && !m_bCoopEscort)) return;
 m_bCoopEscort=start;
 for(s32 n=1;n<CPlayer::m_nPlayerCount;++n) {
  CPlayer &player=Player_aPlayer[n];
  CBot *bot=(CBot *)player.m_pEntityOrig;
  if(!bot || !bot->IsInWorld() || bot->IsDeadOrDying()) continue;
  if(start) {
   player.ZeroControls(); player.DisableEntityControl();
   bot->IgnoreBotVBotCollision(); bot->MobilizeBot();
   bot->m_fMountPitch_WS=0;
   CFMtx43A facing=*CSpyVsSpy::GetGlitch()->MtxToWorld();
   facing.m_vPos=bot->MtxToWorld()->m_vPos;
   bot->Relocate_RotXlatFromUnitMtx_WS(&facing);
   aibrainman_Activate(bot->AIBrain());
   CFVec3A destination=m_GotoPoint,back;
   back.Mul(CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vFront,-4.0f*n);
   destination.Add(back);
   ai_AssignGoal_GotoWithLookAt(bot->AIBrain(),destination,CFVec3A::m_Null,m_pGuy->Guid(),50,2,90,0);
   gamecam_SwitchPlayerTo3rdPersonCamera(PLAYER_CAM(n),bot);
   fcamera_GetCameraByIndex(n)->SetViewSource(NULL);
  } else {
   aibrainman_Deactivate(bot->AIBrain());
   player.EnableEntityControl(); aibrainman_ConfigurePlayerBotBrain(bot->AIBrain(),n);
   bot->DontIgnoreBotVBotCollision(); player.ZeroControls();
  }
 }
 DEVPRINTF("Co-op: factory escort %s for partners with individual cameras.\n",start?"starts":"ends");
}

BOOL CSearchStage::PortTestInspection(u32 test) {
 if(test==0 || test==2) {
  // Reproduce the actual inspection return with five open lockers. Repeat
  // with the warning already consumed, the second authored lethal branch.
  m_bWarnedAboutLocker=(test==2);
  for(u32 n=8;n<13;++n) {
   CMeshEntity *locker=m_Lockers[n].GetMesh();
   locker->UserAnim_UpdateUnitTime(1.0f);
   locker->UserAnim_SetSpeedMult(_STAGE3_LOCKER_ANIM_RATEMUL);
   locker->UserAnim_Pause(FALSE); // Match _OpenLocker, not Restore's paused animation.
  }
  _SetGuyState(GUY_STATE_WALK_BACK);
  m_fGotoTimer=0;
  _WorkGuyState(5);
  return m_eGuyState==GUY_STATE_CLOSE_LOCKER;
 }
 if(test==1) {
  for(u32 n=0;n<NUM_LOCKERS;++n) if(m_Lockers[n].IsOpen()) return FALSE;
  return m_eGuyState!=GUY_STATE_FAULTY_GLITCH && m_eGuyState!=GUY_STATE_KILL_GLITCH && !m_bSpinCam;
 }
 if(test==3) {
  _SetGuyState(GUY_STATE_KILL_GLITCH);
  return m_eGuyState==GUY_STATE_WALK_AWAY;
 }
 if(test==4) {
  _SetGuyState(GUY_STATE_WALK_BACK);m_fGotoTimer=0;_WorkGuyState(0);
  return m_eGuyState==GUY_STATE_RESTING && m_eGuyStateNext==GUY_STATE_DONE;
 }
 return FALSE;
}

BOOL CSearchStage::PortTestEscort(u32 test) {
 if(test==0) {_SetGuyState(GUY_STATE_DONE);return m_bCoopEscort;}
 if(test==1) return m_eGuyState==GUY_STATE_FAKE_DDR;
 if(test==2) {_CoopEscort(FALSE);return !m_bCoopEscort;}
 return FALSE;
}
#endif

void CSearchStage::_SetGuyRestState( GuyState_e eNextState, f32 fTime ) {
	m_fRestTime = fTime;
	m_eGuyStatePrev = m_eGuyState;
	m_eGuyState = GUY_STATE_RESTING;
	m_eGuyStateNext = eNextState;
}

static CItemInst *_SpyTeamChip() {
	CItemInst *pItem = CSpyVsSpy::GetGlitch()->m_pInventory->IsItemInInventory("chip");
#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() && (!pItem || pItem->m_nClipAmmo < 1) ) {
		for( s32 n = 1; n < CPlayer::m_nPlayerCount; ++n ) {
			CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
			if( !pBot || !pBot->m_pInventory ) continue;
			CItemInst *pOther = pBot->m_pInventory->IsItemInInventory("chip");
			if( pOther && pOther->m_nClipAmmo >= 1 ) return pOther;
		}
	}
#endif
	return pItem;
}

void CSearchStage::_WorkGuyState( u32 uNumLockersOpen ) {
	CFVec3A TempPos;
	CFQuatA QDest, QRes;
	BOOL bAtGotoPoint = FALSE;

	if( m_pGuy->AIBrain() && 
		!m_pGuy->AIBrain()->IsGoalChangePending() &&
		m_pGuy->AIBrain()->GetCurThought() != CAIBrain::TT_GOTO ) {
		bAtGotoPoint = TRUE;
	}
//	bAtGotoPoint = _IsAtGotoPoint( _STAGE3_CLOSE_ENOUGH );

	m_fGotoTimer -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fGotoTimer, 0.0f );

	// If we timeout while going to a goto position, force the at goto point to TRUE
	if( m_fGotoTimer == 0.0f ) {
		bAtGotoPoint = TRUE;
	}

#if !FANG_PRODUCTION_BUILD && SAS_ACTIVE_USER == SAS_USER_NATHAN
//	fdraw_DevSphere( &m_GotoPoint.v3, 8.0f );
#endif

	switch( m_eGuyState ) {
		case GUY_STATE_WAIT_TALK:
			CSpyVsSpy::StreamStart( _SPY_VS_SPY_STREAM_SEARCH, TRUE, TRUE );
			_SetGuyState( GUY_STATE_WAIT_FIRST_WORK );
		break;
		case GUY_STATE_WAIT_FIRST_WORK:{
#if FANG_WINGC
			// The factory only transports P1's parts. Rejoin the team after Shhh's
			// post-assembly transmission, before the guide starts the next puzzle.
			if( MultiplayerMgr.IsLocalCoop() ) {
				CPlayer::CoopPlaceStartingPartners( TRUE, 0 );
				if( !CPlayer::CoopReviveForCheckpoint() ) break;
				CPlayer::CoopPlaceStartingPartners( TRUE, 0 );
				DEVPRINTF( "Spy factory: partners regrouped after post-assembly transmission.\n" );
			}
#endif

			m_pAmbientEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_SEARCH_AMBIENT ), TRUE, NULL, 0.0f );
			CEmitterFader::Fade( m_pAmbientEmitter, 0.0f, _STAGE3_AMBIENT_VOL, 2.0f );

			CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_IDLE_MACHINE );
			CFVec3A SoundPos = CFVec3A::m_UnitAxisY;
	
			SoundPos.Mul( 5.0f );
			SoundPos.Add( m_SpawnPoint );

			m_pIdleMachine = CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &SoundPos, _STAGE3_IDLE_MACHINE_VOL );

			CEmitterFader::Fade( m_pIdleMachine, 0.0f, _STAGE3_IDLE_MACHINE_VOL, 3.0f );
			_SetGuyState( GUY_STATE_WALK_AWAY );
		}break;
		case GUY_STATE_RESTING:
			m_fRestTime -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fRestTime, 0.0f );

			if( m_fRestTime == 0.0f ) {
				_SetGuyState( m_eGuyStateNext );
			}
		break;
		case GUY_STATE_WALK_AWAY:
			if( bAtGotoPoint ) {
				// Close the door and lock it
				m_pDoor->ForceGotoPos( 0, CDoorEntity::GOTOREASON_UNKNOWN );
				m_pDoor->SetLockState( TRUE );

				// Play a ding, the wait time is the time he is in the elevator
				CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_ELEVATOR_DING ), TRUE );
				_SetGuyRestState( GUY_STATE_WALK_CONSOLE, _STAGE3_REST_TIME_ELEVATOR_FIRST_FLOOR );
			}
		break;
		case GUY_STATE_WALK_BACK:
			if( bAtGotoPoint ) {
				BOOL bWarned = FALSE;

				if( uNumLockersOpen ) {
					if( uNumLockersOpen == 1 ) {
						// We haven't been warned about an open locker yet, let him slide and play
						if( !m_bWarnedAboutLocker ) {
							m_bWarnedAboutLocker = TRUE;
							_SetGuyState( GUY_STATE_CLOSE_LOCKER );
						} else {
							_SetGuyState( GUY_STATE_FAULTY_GLITCH );
						}
					} else {
						_SetGuyState( GUY_STATE_FAULTY_GLITCH );
					}

					bWarned = TRUE;
				}

				if( !bWarned ) {
					CItemInst *pItem = _SpyTeamChip();
					BOOL bHasAll = FALSE;

					if( pItem ) {
						if( pItem->m_nClipAmmo >= 1 ) {
							CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_TESTING_READY );
							f32 fIdle = fsndfx_GetDuration( CFSoundGroup::GetSoundHandle( pGroup, 0 ) );
							
							// This is because the HUD is on, but we aren't drawing anything
							CSpyVsSpy::TurnOffHUD( TRUE, TRUE );
							CSpyVsSpy::TakeControlFromPlayer( TRUE );
							CSpyVsSpy::TurnOffHUD( FALSE, TRUE );
							
							CSpyVsSpy::PlayBotTalk( m_pGuy, CSpyVsSpy::BTR_READY );

							_SetGuyRestState( GUY_STATE_DONE, fIdle );
							bHasAll = TRUE;
							CEmitterFader::Fade( m_pAmbientEmitter, _STAGE3_AMBIENT_VOL, 0.0f, 3.0f, TRUE );
							CEmitterFader::Fade( m_pIdleMachine, _STAGE3_IDLE_MACHINE_VOL, 0.0f, 3.0f, TRUE );
						}
					}
					
					if( !bHasAll ) {
						CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CHECK_SOUND ), TRUE );
						CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_INSPECT_BOT ), TRUE );
						_SetGuyRestState( GUY_STATE_WALK_AWAY, _STAGE3_REST_TIME_BOT );	
					}
				}
			} 
		break;
		case GUY_STATE_WALK_CONSOLE:
			if( bAtGotoPoint ) {
				_SetGuyRestState( GUY_STATE_WALK_AWAY_CONSOLE, _STAGE3_REST_TIME_CONSOLE );
			}
		break;
		case GUY_STATE_WALK_AWAY_CONSOLE:
			if( bAtGotoPoint ) {
				f32 fRandomTime = fmath_RandomFloatRange( _STAGE3_REST_TIME_ELEVATOR_SECOND_FLOOR_MIN, _STAGE3_REST_TIME_ELEVATOR_SECOND_FLOOR_MAX );
				// Play a ding, the wait time is the time he is in the elevator
				CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_ELEVATOR_DING ), TRUE );
				_SetGuyRestState( GUY_STATE_WALK_BACK, fRandomTime );
			} 
		break;
		case GUY_STATE_CLOSE_LOCKER:
			if( bAtGotoPoint ) {
				_CloseInspectionLockers();

				CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_OPEN_LOCKER_WARNING );
				f32 fIdle;
				
				fIdle = FMATH_MAX( fsndfx_GetDuration( CFSoundGroup::GetSoundHandle( pGroup, 0 ) ) * 0.70f, _STAGE3_REST_TIME_BOT );

				CFSoundGroup::PlaySound( pGroup, TRUE );

				// If we were told to look at a locker, go back to look at Glitch
				_SetGuyRestState( GUY_STATE_WALK_BACK, fIdle );	

				ai_AssignGoal_FaceIt( m_pGuy->AIBrain(), CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 0, 10, 0 );
			}
		break;
		case GUY_STATE_DONE: {
			BOOL bThere = FALSE;

			if( CSpyVsSpy::GetGlitch()->AIBrain() && 
				!CSpyVsSpy::GetGlitch()->AIBrain()->IsGoalChangePending() &&
				CSpyVsSpy::GetGlitch()->AIBrain()->GetCurThought() != CAIBrain::TT_GOTO ) {
				bThere = TRUE;
			}

			if( bThere ) {
				CSpyVsSpy::TakeControlFromPlayer( FALSE );
				_SetGuyState( GUY_STATE_FAKE_DDR ); 
			}
		}
		break;
		case GUY_STATE_FAKE_DDR:{
			f32 fFOV = 0.0f;
			CDDRStage *pDDR = (CDDRStage *) CSpyVsSpy::m_pGame->m_pDanceStage;

			m_fUnitChase += ( FLoop_fPreviousLoopSecs * 0.5f );
			FMATH_CLAMPMAX( m_fUnitChase, 1.0f );

			if( !pDDR->IsFinished() ) {
				if( pDDR->m_eStageState == CDDRStage::STATE_GLITCH_GETS_KILLED && !m_bChaseCam ) {
					m_bChaseCam = TRUE;
					m_fUnitChase = 0.0f;
					m_StartPos = m_ChasePos;
					m_StartQuat = m_ChaseQuat;
				}

				if( m_bChaseCam ) {
					CFMtx43A::m_Temp.m_vPos.Sub( pDDR->m_pKillMiner->MtxToWorld()->m_vPos, m_StartPos );
					CFMtx43A::m_Temp.UnitMtxFromNonUnitVec( &CFMtx43A::m_Temp.m_vPos );
					m_ChaseQuat.BuildQuat( CFMtx43A::m_Temp );
				}

				CSpyVsSpy::CameraInterp( m_fUnitChase, 
								fFOV, m_StartPos, m_StartQuat,
								fFOV, m_ChasePos, m_ChaseQuat,
								&fFOV, &m_Camera.m_Pos_WS, &m_Camera.m_Quat_WS );

				CSpyVsSpy::m_pGame->m_pDanceStage->Work();
			} else {
				FMATH_CLEARBITMASK( pDDR->m_uFlags, CDDRStage::FLAG_SPECIAL_BITS );

				if( m_pCrane->UserAnim_GetCurrentInst()->GetUnitTime() >= 0.999f ) {
                    _SetGuyState( GUY_STATE_MOVE_CAM_BACK );
				}
			}
		}
		break;
		case GUY_STATE_MOVE_CAM_BACK:{
			f32 fFOV = 0.0f;

			m_fUnitChase += ( FLoop_fPreviousLoopSecs * 0.5f );
			FMATH_CLAMPMAX( m_fUnitChase, 1.0f );

			CSpyVsSpy::CameraInterp( m_fUnitChase, 
				fFOV, m_StartPos, m_StartQuat,
				fFOV, m_ChasePos, m_ChaseQuat,
				&fFOV, &m_Camera.m_Pos_WS, &m_Camera.m_Quat_WS );

			if( m_fUnitChase == 1.0f ) {
				gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );

				_SetGuyState( GUY_STATE_CRANE );	
			}
		}
		break;
		case GUY_STATE_CRANE: {
			CFSphere Sphere;

			// Blow up the bounding sphere, yet again
			Sphere.m_Pos = m_pCrane->MtxToWorld()->m_vPos.v3;
			Sphere.m_fRadius = _BIG_RADIUS;
			m_pCrane->GetMesh()->MoveTracker( Sphere );

			f32 fTime = m_pCrane->UserAnim_GetCurrentInst()->GetTime();

			if( fTime >= m_pCrane->UserAnim_GetCurrentInst()->GetTotalTime() ) {
				_SetFinished( TRUE );

				// Put the HUD back to where it was
				CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->EnableDrawFlags( CHud2::DRAW_GLITCH_HUD );
				//CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->DisableDrawFlags( CHud2::DRAW_RADAR );
				CSpyVsSpy::GetGlitch()->ReticleEnable( TRUE );
				CSpyVsSpy::TurnOffHUD( TRUE );
			} else if( fTime >= _STAGE3_LET_GO_TIME && CSpyVsSpy::GetGlitch()->GetParent() ) {
				m_pCrane->DetachAllChildren();
				CSpyVsSpy::GetGlitch()->MobilizeBot();
				//CSpyVsSpy::TakeControlFromPlayer( FALSE );
				
				m_pCrane->UserAnim_SetSpeedMult( 1.0f );
			} else if( fTime >= _STAGE3_GRAB_TIME && fTime < _STAGE3_LET_GO_TIME && !CSpyVsSpy::GetGlitch()->GetParent() ) {
				CFVec3A SnapPoint;
				s32 sBoneIndex = m_pCrane->GetMeshInst()->FindBone( _STAGE1_CRANE_BONE_NAME );
				CFCamera* pCam = fcamera_GetCameraByIndex( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );

				if( sBoneIndex == -1 ) {
					FASSERT_NOW;
				}

				SnapPoint = m_pCrane->GetMeshInst()->GetBoneMtxPalette()[ sBoneIndex ]->m_vPos;
				SnapPoint.y = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.y;

				CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &SnapPoint );
				CSpyVsSpy::GetGlitch()->ImmobilizeBot();
				CSpyVsSpy::GetGlitch()->Attach_ToParent_WithGlue_WS( m_pCrane, _STAGE3_CRANE_BONE );

				m_bChaseCam = TRUE;
				m_bGotoGlitch = FALSE;
				m_fUnitChase = _STAGE3_DELAY_CHASE_UNIT; // delay a bit
				m_fTimeTillChase = _STAGE3_TIME_TILL_CHASE;

				m_Camera.m_Pos_WS = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
				m_Camera.m_Quat_WS.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
				
				m_ChasePos = m_Camera.m_Pos_WS;
				m_ChaseQuat = m_Camera.m_Quat_WS;
				m_StartPos = m_ChasePos;
				m_StartQuat = m_ChaseQuat;

				gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), &m_Camera );
				CSpyVsSpy::TakeControlFromPlayer( TRUE );

				CSpyVsSpy::GetGlitch()->ShakeCamera( 0.5f, 0.75f );
				CSpyVsSpy::TurnOffHUD( TRUE );				

				CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_PINCH_HIT ), FALSE, &CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
				((CDDRStage *) CSpyVsSpy::m_pGame->m_pDanceStage)->TossSparksFromBot( CSpyVsSpy::GetGlitch() );
			}

		}
		break;
		case GUY_STATE_FAULTY_GLITCH:
			// Make Glitch look at him
			CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->BeginFrame();
			ai_AssignGoal_FaceIt( CSpyVsSpy::GetGlitch()->AIBrain(), CFVec3A::m_Null, m_pGuy->Guid(), 100, 0, 0 );
			CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->EndFrame();

			if( _IsAtGotoPoint( 13.0f ) || m_fGotoTimer == 0.0f ) {
				CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_IMPOSTOR );
				CFSoundInfo *pInfo = CFSoundGroup::GetRandomSoundInfo( pGroup );
				f32 fIdle = fsndfx_GetDuration( pInfo->m_hSound ) + 1.0f;

				//CFSoundGroup::PlaySound( pGroup, TRUE );

				_SetGuyRestState( GUY_STATE_KILL_GLITCH, fIdle );

				if( !fclib_stricmp( "BRGT_impost", pInfo->m_pszSoundTagName ) ) {
					CSpyVsSpy::PlayBotTalk( m_pGuy, CSpyVsSpy::BTR_IMPOSTER );
				} else if( !fclib_stricmp( "BRGT_invade", pInfo->m_pszSoundTagName ) ) {
					CSpyVsSpy::PlayBotTalk( m_pGuy, CSpyVsSpy::BTR_INVADER );
				}
			}
		break;
		case GUY_STATE_NONE:

		break;
		case GUY_STATE_KILL_GLITCH:
			//m_pGuy->AIBrain()->GetAIMover()->m_Controls.Fire();
			//m_pGuy->AIBrain()->GetAIMover()->m_Controls.AimAt( *CSpyVsSpy::GetGlitch()->GetTagPoint( 0 ) );

			if( CSpyVsSpy::GetGlitch()->IsDeadOrDying() ) {
				CSpyVsSpy::TakeControlFromPlayer( FALSE );
				_SetGuyState( GUY_STATE_NONE );
			}
		break;
		default:
			FASSERT_NOW;
		break;
	}
}

void CSearchStage::_CheckDeath( void ) {
#if FANG_WINGC
	// The inspection's lethal solo failure path cannot recover while a co-op
	// partner is alive, and partner collisions can move P1 outside its tiny zone.
	// Keep the guide scripted instead of entering the hostile/kill-only state.
	if( MultiplayerMgr.IsLocalCoop() ) return;
#endif

	BOOL bDead = FALSE;
	BOOL bController = FALSE;

	GuyState_e eTestState;

	if( m_eGuyState == GUY_STATE_RESTING ) {
		switch( m_eGuyStateNext ) {
			case GUY_STATE_WALK_AWAY:
				eTestState = GUY_STATE_WALK_BACK;
			break;
			case GUY_STATE_WALK_BACK:
				eTestState = GUY_STATE_WALK_AWAY_CONSOLE;
			break;
			case GUY_STATE_WALK_AWAY_CONSOLE:
				eTestState = GUY_STATE_WALK_CONSOLE;
			break;
			case GUY_STATE_WALK_CONSOLE:
				eTestState = GUY_STATE_WALK_CONSOLE;//GUY_STATE_WALK_AWAY;
			break;
		}	
	} else {
		eTestState = m_eGuyState;
	}
	
	f32 fDot = m_SafeDirection.Dot( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vFront );

	if( ( eTestState == GUY_STATE_WALK_AWAY ) || ( eTestState == GUY_STATE_WALK_BACK ) || ( eTestState == GUY_STATE_CLOSE_LOCKER ) ) {
//		if( m_pHumanControl->m_fFire1 != 0.0f || m_pHumanControl->m_fFire2 != 0.0f ) {
//			bDead = TRUE;
//			bController = TRUE;
//		}

		if( m_pHumanControl->m_nPadFlagsJump != 0 || m_pHumanControl->m_nPadFlagsMelee != 0 ) {
			bDead = TRUE;
			bController = TRUE;
		}

		if( fDot < _STAGE3_MIN_DOT_SIZE ) { 
			bDead = TRUE;
		} 

		if( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.DistSq( m_SpawnPoint ) > FMATH_SQUARE( _STAGE3_SAFE_RADIUS ) ) {
			bDead = TRUE;
		}

		if( bDead ) {
			m_fDeathTimer -= FLoop_fPreviousLoopSecs * ( bController ? 3.0f : 1.0f );
			FMATH_CLAMPMIN( m_fDeathTimer, 0.0f );

			if( m_fDeathTimer == 0.0f ) {
				// We're really dead this time
				_SetGuyState( GUY_STATE_FAULTY_GLITCH );
			}
		} else {
			m_fDeathTimer = _STAGE3_DEATH_TIME;
		}
	} else { 
		m_fDeathTimer = _STAGE3_DEATH_TIME;
	}
}

// If we get here, we need to kill Glitch and stop everything else
void CSearchStage::_KillGlitch( void ) {
//	CSpyVsSpy::TakeControlFromPlayer( FALSE );
//	CSpyVsSpy::GetGlitch()->Die( TRUE, FALSE );
	CAIBrain *pBrain = m_pGuy->AIBrain();

	CSpyVsSpy::MakeBotSmart( m_pGuy );

	pBrain->AttribStack_Push( CAIBrain::ATTRIB_AGGRESSION, 100 );
	pBrain->AttribStack_Push( CAIBrain::ATTRIB_INTELLIGENCE, 100 );
	pBrain->AttribStack_Push( CAIBrain::ATTRIB_COURAGE, 100 );

	if( pBrain->GetAIMover()->GetWeaponCtrlPtr( 0 ) ) {
		CAIWeaponCtrl *pControl = pBrain->GetAIMover()->GetWeaponCtrlPtr( 0 );

		pControl->m_fBurstDelay = 0.0f;
		pControl->m_fFireDelay = 30.0f;
		pControl->m_fAccuracy = 1.0f;
		pControl->m_uNumFiresPerBurst = 255;
		pControl->m_uNumFiresPerBurstBonus = 255;
		pControl->m_fBurstDelayBonus = 0.0f;
		pControl->m_fFireDelayBonus = 0.0f;
	}

	ai_AssignGoal_Attack( pBrain, CSpyVsSpy::GetGlitch()->Guid(), 0 );

	CSpyVsSpy::GetGlitch()->SetNormHealth( 0.2f );
}

void CSearchStage::_CameraChaseWork( void ) {
	f32 fUnitS;
	CFVec3A Pos, Dir;
	CFQuatA Quat;

	m_fUnitChase += ( FLoop_fPreviousLoopSecs * 0.5f );
	FMATH_CLAMPMAX( m_fUnitChase, 1.0f );

	m_fTimeTillChase -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fTimeTillChase, 0.0f );

	if( m_fTimeTillChase == 0.0f && !m_bGotoGlitch ) {
		m_fUnitChase = 0.0f;
		m_StartPos = m_Camera.m_Pos_WS;
		m_StartQuat = m_Camera.m_Quat_WS;

		m_bGotoGlitch = TRUE;
	}

	if( m_fUnitChase < 0.0f ) {
		return;
	}

	if( !m_bGotoGlitch ) {
		Dir.Sub( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos, m_StartPos );
		CFMtx43A::m_Temp.UnitMtxFromNonUnitVec( &Dir );
		m_ChaseQuat.BuildQuat( CFMtx43A::m_Temp );
	} else {
		gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );
		fcamera_Work();

		CFCamera* pCam = fcamera_GetCameraByIndex( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );

		m_ChasePos = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
		m_ChaseQuat.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
	
		gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), &m_Camera );
		CSpyVsSpy::TakeControlFromPlayer( TRUE );
	}

	fUnitS = fmath_UnitLinearToSCurve( m_fUnitChase );
	Pos.Lerp( fUnitS, m_StartPos, m_ChasePos );
	Quat.ReceiveSlerpOf( fUnitS, m_StartQuat, m_ChaseQuat );
    
	m_Camera.m_Pos_WS = Pos.v3;
	m_Camera.m_Quat_WS = Quat;
}

void CSearchStage::_CameraSpinWork( void ) {
	m_fUnitChase += ( FLoop_fPreviousLoopSecs * m_fCamTimeScale );

	if( m_fUnitChase < 0.0f ) {
		m_fUnitChase = 0.0f;
		gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );
		CSpyVsSpy::TakeControlFromPlayer( FALSE );
		m_bSpinCam = FALSE;

		m_pBuildMachine->DrawEnable( TRUE, TRUE );
	} else if( m_fUnitChase > 1.0f ) {
		m_fUnitChase = 1.0f;
	}
	
	f32 fFov = 0.0f; // not used

	CSpyVsSpy::CameraInterp( m_fUnitChase, fFov, m_StartPos, m_StartQuat, fFov, m_ChasePos, m_ChaseQuat, &fFov, &m_Camera.m_Pos_WS, &m_Camera.m_Quat_WS );
}

const CFVec3A &CSearchStage::_GetStartPoint( CESpline *pSpline ) {
	FASSERT( pSpline );
	return pSpline->PointArray()[ 0 ];
}

const CFVec3A &CSearchStage::_GetEndPoint( CESpline *pSpline ) {
	FASSERT( pSpline );
	return pSpline->PointArray()[ pSpline->PointCount() - 1 ];
}

BOOL CSearchStage::_IsAtGotoPoint( const f32 &fRad ) {
	return m_GotoPoint.DistSqXZ( m_pGuy->MtxToWorld()->m_vPos ) <= FMATH_SQUARE( fRad );
}

#define _BATTLE_NUM_GUYS		( 6 )
#define _BATTLE_GUY_1			( "Pete" )
#define _BATTLE_GUY_2			( "Jimmy" )
#define _BATTLE_GUY_3			( "Joey" )
#define _BATTLE_GUY_4			( "Mike" )
#define _BATTLE_GUY_5			( "Fred" )
#define _BATTLE_GUY_6			( "Tony" )
#define _BATTLE_START_POINT		( "startglitch" )
#define _BATTLE_ESCORT1			( "escort01" )
#define _BATTLE_ESCORT2			( "escort02" )
#define _BATTLE_DOOR			( "battledoor" )
#define _BATTLE_REPROGRAM_POINT ( "reprogram" )
#define _BATTLE_DOOR_DELAY		( 2.0f )

//
//
//
// CBattleStage - 
//
//
//
FCLASS_ALIGN_PREFIX class CBattleStage : public CSpyVsSpyStage { 
//----------------------------------------------------------------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBattleStage();
	virtual ~CBattleStage() { }

	BOOL Load( LevelEvent_e eEvent );
	void Unload( void );
	void Restore( void );
	void Work( void );
	void SwitchTo( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private definitions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	typedef enum {
		STATE_BATTLE,
		STATE_WALK_TO_DOOR,
		STATE_RUN_TO_PROGRAM,
	} BattleState_e;
//----------------------------------------------------------------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );
	void _SetupFighterBot( CBot *pBot );
	void _MakeEscortDumb( CBot *pBot );

	void _BattleWork( void );
	void _WalkToDoorWork( void );
//	void _RunToProgramWork( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private data
//----------------------------------------------------------------------------------------------------------------------------------
private:
	BattleState_e m_eStageState;

	CESphere *m_pGlitchStartPoint;
	CBotMiner *m_paMiners[ _BATTLE_NUM_GUYS ];

	CBotGrunt *m_pEscortGrunt1;
	CBotGrunt *m_pEscortGrunt2;
	CDoorEntity *m_pBattleDoor;

	f32 m_fDoorDelay;
	CFVec3A m_ProgramPoint;

	FCLASS_STACKMEM_ALIGN( CBattleStage );
} FCLASS_ALIGN_SUFFIX;

CBattleStage::CBattleStage() { 
	_ClearDataMembers();
}

BOOL CBattleStage::Load( LevelEvent_e eEvent ) { 
	if( eEvent == LEVEL_EVENT_POST_WORLD_LOAD ) {
		cchar *pszGuyName = NULL;
		CEntity *pEntity;

		// Find all of the miners we will kill
		for( u32 i = 0; i < _BATTLE_NUM_GUYS; ++i ) {
			switch( i + 1 ) {
				case 1: pszGuyName = _BATTLE_GUY_1; break;
				case 2: pszGuyName = _BATTLE_GUY_2; break;
				case 3: pszGuyName = _BATTLE_GUY_3; break;
				case 4: pszGuyName = _BATTLE_GUY_4; break;
				case 5: pszGuyName = _BATTLE_GUY_5; break;
				case 6: pszGuyName = _BATTLE_GUY_6; break;
			}

			pEntity = CSpyVsSpy::FindEntity( pszGuyName, ENTITY_BIT_BOTMINER );

			if( !pEntity ) {
				goto _ExitWithError;
			}

			m_paMiners[ i ] = (CBotMiner *) pEntity;
		}

		// Find the Glitch start point
		if( !( pEntity = CSpyVsSpy::FindEntity( _BATTLE_START_POINT, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
		m_pGlitchStartPoint = (CESphere *) pEntity;

		// Find the escorts
#if FANG_WINGC
		// The retail level was reworked after this source: it has no escorts or "reprogram"
		// point (main.dol names neither). Missing escorts are skipped.
		m_pEscortGrunt1 = (CBotGrunt *) CSpyVsSpy::FindEntity( _BATTLE_ESCORT1, ENTITY_BIT_BOTGRUNT, FALSE );
		m_pEscortGrunt2 = (CBotGrunt *) CSpyVsSpy::FindEntity( _BATTLE_ESCORT2, ENTITY_BIT_BOTGRUNT, FALSE );
#else
		if( !( pEntity = CSpyVsSpy::FindEntity( _BATTLE_ESCORT1, ENTITY_BIT_BOTGRUNT ) ) ) {
			goto _ExitWithError;
		}
		m_pEscortGrunt1 = (CBotGrunt *) pEntity;

		if( !( pEntity = CSpyVsSpy::FindEntity( _BATTLE_ESCORT2, ENTITY_BIT_BOTGRUNT ) ) ) {
			goto _ExitWithError;
		}
		m_pEscortGrunt2 = (CBotGrunt *) pEntity;
#endif

		// Find the battle door
		if( !( pEntity = CSpyVsSpy::FindEntity( _BATTLE_DOOR, ENTITY_BIT_DOOR ) ) ) {
			goto _ExitWithError;
		}
		m_pBattleDoor = (CDoorEntity *) pEntity;

		// Find the reprogram point that we will run to
#if FANG_WINGC
		// Only the retired run-to-program code used this point.
		if( !( pEntity = CSpyVsSpy::FindEntity( _BATTLE_REPROGRAM_POINT, ENTITY_BIT_SPHERE, FALSE ) ) ) {
			pEntity = m_pGlitchStartPoint;
		}
#else
		if( !( pEntity = CSpyVsSpy::FindEntity( _BATTLE_REPROGRAM_POINT, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
#endif

		m_ProgramPoint = pEntity->MtxToWorld()->m_vPos;

	} else if( eEvent == LEVEL_EVENT_PRE_ENTITY_FIXUP ) {

	}

	return TRUE;

_ExitWithError:
	Unload();
	_ClearDataMembers();

	return FALSE;
}

void CBattleStage::Unload( void ) { 
	_ClearDataMembers();
}

void CBattleStage::Restore( void ) {
	SwitchTo();
}

void CBattleStage::Work( void ) { 
	switch( m_eStageState ) {
		case STATE_BATTLE:
			_BattleWork();
		break;
		case STATE_WALK_TO_DOOR:
			_WalkToDoorWork();
		break;
//		case STATE_RUN_TO_PROGRAM:
//			_RunToProgramWork();
//		break;
	}
}

void CBattleStage::SwitchTo( void ) {
	for( u32 i = 0; i < _BATTLE_NUM_GUYS; ++i ) {
		_SetupFighterBot( m_paMiners[ i ] );
	}

	if( m_pEscortGrunt1 ) {
		_MakeEscortDumb( (CBot *) m_pEscortGrunt1 );
	}
	if( m_pEscortGrunt2 ) {
		_MakeEscortDumb( (CBot *) m_pEscortGrunt2 );
	}

	// Put him in the world
	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp = FMath_aRotMtx43A[ FMTX_ROTTYPE_90Y ];
	CFMtx43A::m_Temp.Transpose33();
	CFMtx43A::m_Temp.m_vPos = m_pGlitchStartPoint->MtxToWorld()->m_vPos;
	CSpyVsSpy::GetGlitch()->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );

	m_eStageState = STATE_BATTLE;
	_SetFinished( FALSE );

	CSpyVsSpy::TurnOffHUD( FALSE, 0, TRUE );
	CSpyVsSpy::AttackDisable( FALSE );
	CSpyVsSpy::TakeControlFromPlayer( FALSE );

	CInventory *pInv = CSpyVsSpy::GetGlitch()->m_pInventory;
	CItemInst *pPrimary = pInv->IsWeaponInInventory( "RLauncher L1" );
	CItemInst *pSecondary = pInv->IsWeaponInInventory( "Empty Secondary" );

	pInv->SetCurWeapon( INV_INDEX_PRIMARY, pPrimary, FALSE, TRUE );
	pInv->SetCurWeapon( INV_INDEX_SECONDARY, pSecondary, FALSE, TRUE );
#if FANG_WINGC
	for( s32 n = 0; MultiplayerMgr.IsLocalCoop() && n < CPlayer::m_nPlayerCount; ++n ) {
		CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
		if( pBot ) { pBot->DontIgnoreBotVBotCollision(); pBot->m_vecCamPosAdj.Zero(); pBot->m_vecCamLookAdj.Zero(); }
		CHud2::GetHudForPlayer(n)->EnableDrawFlags(CHud2::DRAW_GLITCH_HUD);
		GameCamType_e type;
		CCamBot *pCamera = (CCamBot *)gamecam_GetCameraManByIndex(PLAYER_CAM(n),&type);
		if(type==GAME_CAM_TYPE_ROBOT_3RD) pCamera->SetAlwaysOverride(FALSE);
	}
	CSpyVsSpy::CoopSyncStage( "RLauncher L1", "Empty Secondary", TRUE );
#endif

	_SetFinished( FALSE );
}

void CBattleStage::_ClearDataMembers( void ) { 
	for( u32 i = 0; i < _BATTLE_NUM_GUYS; ++i ) {
		m_paMiners[ i ] = NULL;
	}

	m_pGlitchStartPoint = NULL;
	
	m_pEscortGrunt1 = NULL;
	m_pEscortGrunt2 = NULL;
    m_pBattleDoor = NULL;

	m_fDoorDelay = 0.0f;
	m_ProgramPoint.Zero();

	m_eStageState = STATE_BATTLE;
}

void CBattleStage::_SetupFighterBot( CBot *pBot ) {
	CAIBrain *pBrain = pBot->AIBrain();

	ai_SetRace( pBrain, AIRACE_MIL );

	pBrain->SetFlag_AttackEnemy();
	pBrain->SetFlag_AttackNeutral();
	pBrain->SetFlag_AttackFriendly();
	pBrain->SetFlag_AttackPlayer();
	pBrain->SetFlag_AttackNPC();

	ai_AssignJob_Wander( pBrain, 0.0f, TRUE, 0, 0, 0, 0, 0 );

	pBrain->SetBonusEyeScanDistForPlayersUnalert( 0.0f );
	pBrain->SetBonusEyeScanDistForPlayersAlert( 0.0f );
}

void CBattleStage::_MakeEscortDumb( CBot *pBot ) {
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_EYES );
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_EARS );
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_TOUCH );
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_RADIO );

	// Do this so our guy doesn't look around like a moron
	ai_AssignJob_Wait( pBot->AIBrain(), FALSE, CFVec3A::m_Null, 0, 0 );

	pBot->AIBrain()->GetAIMover()->m_uMoverFlags |= CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS;
	pBot->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( 1.0f );

	pBot->IgnoreBotVBotCollision();
	pBot->EnableCameraCollision( FALSE );
	pBot->SetInvincible( TRUE );

	ai_AssignGoal_FaceIt( pBot->AIBrain(), CFVec3A::m_NegDoubleVec, CSpyVsSpy::GetGlitch()->Guid(), 100, 0, -1 );
}

void CBattleStage::_BattleWork( void ) {
	u32 i;

	for( i = 0; i < _BATTLE_NUM_GUYS; ++i ) {
		if( !m_paMiners[ i ]->IsDead() ) {
			break;
		}
	}

	if( i == _BATTLE_NUM_GUYS ) {
		m_eStageState = STATE_WALK_TO_DOOR;

		CSpyVsSpy::TurnOffHUD( TRUE );

		CInventory *pInv = CSpyVsSpy::GetGlitch()->m_pInventory;
		CItemInst *pPrimary = pInv->IsWeaponInInventory( "Empty Primary" );
		CItemInst *pSecondary = pInv->IsWeaponInInventory( "Empty Secondary" );

		pInv->SetCurWeapon( INV_INDEX_PRIMARY, pPrimary, FALSE, TRUE );
		pInv->SetCurWeapon( INV_INDEX_SECONDARY, pSecondary, FALSE, TRUE );

		CSpyVsSpy::CoopSyncStage( "Empty Primary", "Empty Secondary", FALSE );

		// Open up the door so we know where to go.
		m_pBattleDoor->ForceGotoPos( 1, CDoorEntity::GOTOREASON_UNKNOWN );
	}
}

#define _BATTLE_DOOR_DIST_XZ_SQ ( 15.0f * 15.0f )

void CBattleStage::_WalkToDoorWork( void ) {
	if( m_fDoorDelay == 0.0f ) {
		CFSphere Sphere;
		
		Sphere.m_Pos = m_pBattleDoor->MtxToWorld()->m_vRight.v3;
		Sphere.m_Pos.Mul( -8.0f );
		Sphere.m_Pos.Add( CFVec3A::m_UnitAxisY.v3 );
		Sphere.m_Pos.Add( CFVec3A::m_UnitAxisY.v3 );
		Sphere.m_Pos.Add( m_pBattleDoor->MtxToWorld()->m_vPos.v3 );
		Sphere.m_fRadius = 4.0f;

		if( Sphere.IsIntersecting( CSpyVsSpy::GetGlitch()->m_pWorldMesh->GetBoundingSphere() ) ) {
			CSpyVsSpy::TakeControlFromPlayer( TRUE );
			m_fDoorDelay = _BATTLE_DOOR_DELAY;

			// The door is now forced open when you win.
//			m_pBattleDoor->ForceGotoPos( 1, CDoorEntity::GOTOREASON_UNKNOWN );

			if( m_pEscortGrunt1 ) {
				ai_AssignGoal_FaceIt( CSpyVsSpy::GetGlitch()->AIBrain(), CFVec3A::m_Null, m_pEscortGrunt1->Guid(), 100, 0, 0 );
			}
		}
	} else {
		m_fDoorDelay -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fDoorDelay, 0.0f );

		if( m_fDoorDelay == 0.0f ){
			_SetFinished( TRUE );
			CSpyVsSpy::TakeControlFromPlayer( FALSE );

//			m_eStageState = STATE_RUN_TO_PROGRAM;
//
//			CFVec3A DestPoint, BumpUp;
//
//			BumpUp.Mul( CFVec3A::m_UnitAxisY, 5.0f );
//
//			DestPoint = CFVec3A::m_UnitAxisX;
//			DestPoint.Mul( 10.0f );
//			DestPoint.Add( m_ProgramPoint );
//
//			ai_AssignGoal_Goto( m_pEscortGrunt1->AIBrain(), DestPoint, 1, 100 );
//
//			DestPoint = CFVec3A::m_UnitAxisX;
//			DestPoint.Mul( -10.0f );
//			DestPoint.Add( BumpUp );
//			DestPoint.Add( m_ProgramPoint );
//
//			ai_AssignGoal_Goto( m_pEscortGrunt2->AIBrain(), DestPoint, 1, 100 );
//			ai_AssignGoal_Goto( CSpyVsSpy::GetGlitch()->AIBrain(), m_ProgramPoint, 1, 90 );
		}
	}
}

//void CBattleStage::_RunToProgramWork( void ) {
//	f32 fDistXZSq = m_ProgramPoint.DistSqXZ( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
//
//	if( fDistXZSq <= ( 4.0f * 4.0f ) ) {
//		_SetFinished( TRUE );
//	}
//}


#define _PROGRAM_SPAWN_POINT			( "reprogram" )
#define _PROGRAM_CRATE_NAME				( "funnyname" )
#define _PROGRAM_ARM_NAME				( "ali" )
#define _PROGRAM_BOX_STOP				( "boxstop" )
//#define _PROGRAM_PARTICLES				( "spy_reprog" )
#define _PROGRAM_REPROGRAM_TIME			( 8.0f )
#define _PROGRAM_REPROGRAM_TIME_EXTRA	( 2.0f )
#define _PROGRAM_OO_REPROGRAM_TIME		( 1.0f / _PROGRAM_REPROGRAM_TIME )
#define _PROGRAM_RUMBLE_TIME			( 0.75f )
#define _PROGRAM_BACK_UP_TIME			( 30.0f * _OO_ANIM_FPS )
#define _PROGRAM_MOVE_TIME				( 1 / 3.0f ) // 1 / seconds
#define _PROGRAM_BOT1					( "Bert" )
#define _PROGRAM_BOT2					( "Ernie" )
#define _PROGRAM_AMBIENT_VOL			( 0.5f )
#define _PROGRAM_RADIUS_SIZE			( 5.0f )

//
//
//
// CProgramStage - 
//
//
//
FCLASS_ALIGN_PREFIX class CProgramStage : public CSpyVsSpyStage { 
//----------------------------------------------------------------------------------------------------------------------------------
// Public functions
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CProgramStage();
	BOOL Contains(CEntity *body) const {return m_bPackUp && m_pCrate && body && body->GetParent()==m_pCrate;}
	virtual ~CProgramStage() { }

	BOOL Load( LevelEvent_e eEvent );
	void Unload( void );
	void Restore( void );
	void Work( void );
	void SwitchTo( void );
	void Cleanup( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private definitions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	typedef enum {
//		STATE_WAIT_FIRST_WORK,
		STATE_IDLE,
//		STATE_INTRO,
//		STATE_LOOK,
//		STATE_PROGRAM,
		STATE_BOX,
		STATE_BOX_MOVE,
		STATE_BOX_STOPPED,
	} ProgramState_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Private functions
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );

//	void _LookWork( void );
//	void _ProgramWork( void );
	void _CrateWork( void );
	void _CrateMove( void );

	void _DialogWork( void );
//	void _ParticleWork( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private data
//----------------------------------------------------------------------------------------------------------------------------------
private:
	BOOL8 m_bPackUp;
	BOOL8 m_bDialogStarted;
	u8 m_uDialogIndex;
	BOOL8 _PAD;

	ProgramState_e m_eStageState;

	f32 m_fProgramTime;
	f32 m_fTimeTillNextRumble;
	f32 m_fUnitInterp;
//	f32 m_fParticleAngle;

	CFVec3A m_SpawnPoint;
	CFVec3A m_CrateStart;
	CFVec3A m_CrateDest;
	CMeshEntity *m_pCrate;
	CMeshEntity *m_pArms;

//	FParticle_DefHandle_t m_hProgramParticleDef;
//	CEParticle *m_pProgramParticle;

	f32 m_fDialogIdle;

	CFAudioEmitter *m_pAmbientEmitter;

	static u32 m_auDialogSounds[];

	FCLASS_STACKMEM_ALIGN( CProgramStage );
} FCLASS_ALIGN_SUFFIX;

#define _PROGRAM_NUM_DIALOG_ITEMS ( 8 )

u32 CProgramStage::m_auDialogSounds[] = {
	CSpyVsSpySoundCenter::SOUND_1_FINALSPY,
	CSpyVsSpySoundCenter::SOUND_2_LEAVEBOX,

	CSpyVsSpySoundCenter::SOUND_1_UPFORTONIGHT,
	CSpyVsSpySoundCenter::SOUND_2_DROIDHUNT,

	CSpyVsSpySoundCenter::SOUND_1_DROIDHUNT,
	CSpyVsSpySoundCenter::SOUND_2_BASH,

	CSpyVsSpySoundCenter::SOUND_1_FUN,
	CSpyVsSpySoundCenter::SOUND_2_DRINK,
};

CProgramStage::CProgramStage() { 
	_ClearDataMembers();
}

BOOL CProgramStage::Load( LevelEvent_e eEvent ) { 
	if( eEvent == LEVEL_EVENT_POST_WORLD_LOAD ) {
		CEntity *pEntity;

		// Find the spawn point
#if FANG_WINGC
		// The retail level has no "reprogram" sphere; use Glitch's start point instead.
		if( !( pEntity = CSpyVsSpy::FindEntity( _PROGRAM_SPAWN_POINT, ENTITY_BIT_SPHERE, FALSE ) ) &&
			!( pEntity = CSpyVsSpy::FindEntity( _BATTLE_START_POINT, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
#else
		if( !( pEntity = CSpyVsSpy::FindEntity( _PROGRAM_SPAWN_POINT, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
#endif
		m_SpawnPoint = pEntity->MtxToWorld()->m_vPos;

		// Find the crate
		if( !( pEntity = CSpyVsSpy::FindEntity( _PROGRAM_CRATE_NAME, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}
		m_pCrate = (CMeshEntity *) pEntity;

		// Find the arms
		if( !( pEntity = CSpyVsSpy::FindEntity( _PROGRAM_ARM_NAME, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}
		m_pArms = (CMeshEntity *) pEntity;

		// Find the crate dest
		if( !( pEntity = CSpyVsSpy::FindEntity( _PROGRAM_BOX_STOP, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}
		m_CrateDest = pEntity->MtxToWorld()->m_vPos;

		// Try to load our program particles
//		m_hProgramParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _PROGRAM_PARTICLES );
//		
//		if( m_hProgramParticleDef == FPARTICLE_INVALID_HANDLE ) {
//			DEVPRINTF( "CProgramStage::Load(): Could not find particle definition '%s'.\n", _PROGRAM_PARTICLES );
//			goto _ExitWithError;
//		}

		// Find the two bots that talk
		if( !( pEntity = CSpyVsSpy::FindEntity( _PROGRAM_BOT1, ENTITY_BIT_BOT ) ) ) {
			goto _ExitWithError;
		}

		CBot *pBot = (CBot *) pEntity;

		pBot->AIBrain()->SetFlag_AttackPlayer();
//		pBot->AIBrain()->SetFlag_AttackFriendly();

		if( !( pEntity = CSpyVsSpy::FindEntity( _PROGRAM_BOT2, ENTITY_BIT_BOT ) ) ) {
			goto _ExitWithError;
		}

		pBot = (CBot *) pEntity;

		pBot->AIBrain()->SetFlag_AttackPlayer();
//		pBot->AIBrain()->SetFlag_AttackFriendly();
	} else if( eEvent == LEVEL_EVENT_PRE_ENTITY_FIXUP ) {

	}

	return TRUE;

_ExitWithError:
	Unload();
	_ClearDataMembers();

	return FALSE;
}

void CProgramStage::Unload( void ) { 
//	if( m_pAmbientEmitter ) {
//		m_pAmbientEmitter->Destroy();
//		m_pAmbientEmitter = NULL;
//	}

	_ClearDataMembers();
}

void CProgramStage::Restore( void ) {
//	if( m_pAmbientEmitter ) {
//		m_pAmbientEmitter->Destroy();
//		m_pAmbientEmitter = NULL;
//	}

	SwitchTo();

	CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &m_SpawnPoint );
	CSpyVsSpy::CoopSyncStage( "Empty Primary", "Empty Secondary", TRUE );
}

void CProgramStage::Work( void ) { 
	switch( m_eStageState ) {
//		case STATE_WAIT_FIRST_WORK:
//			m_eStageState = STATE_INTRO;
//
//			CSpyVsSpy::StreamStart( _SPY_VS_SPY_STREAM_PROGRAM, TRUE, TRUE );
//		break;
//		case STATE_INTRO:
//			CSpyVsSpy::TakeControlFromPlayer( TRUE );
//			m_eStageState = STATE_LOOK;
//
//			m_pProgramParticle = eparticlepool_GetEmitter();
//
//			if( m_pProgramParticle ) {
//				m_pProgramParticle->StartEmission( m_hProgramParticleDef, 1.0f, _PROGRAM_REPROGRAM_TIME );
//				m_pProgramParticle->Relocate_RotXlatFromUnitMtx_WS( CSpyVsSpy::GetGlitch()->MtxToWorld() );
//			}
//			
//			m_pAmbientEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_PROGRAM_AMBIENT ), TRUE, NULL, 0.0f );
//			CEmitterFader::Fade( m_pAmbientEmitter, 0.0f, _PROGRAM_AMBIENT_VOL, 2.0f );
//
//		break;
//		case STATE_LOOK:
//			_LookWork();
//		break;
//		case STATE_PROGRAM:
//			_ProgramWork();
//		break;	
		case STATE_BOX:
			_CrateWork();
		break;
		case STATE_BOX_MOVE:
			_CrateMove();
		break;
		case STATE_IDLE:
#if FANG_WINGC
			if(m_pCrate->NormHealth()<=0 || !m_pCrate->IsInWorld()) {
				CBot *lead=CSpyVsSpy::GetGlitch();
				if(lead->GetParent()==m_pCrate) {
					m_pCrate->SetCollisionFlag(FALSE);
					if(CPlayer::RecoverFromStoryCarrier(lead)) {lead->DetachFromParent();lead->MobilizeBot();}
				}
				if(CSpyVsSpy::CoopPackingWaiting() && lead->GetParent()!=m_pCrate && !lead->IsInAir()) {
					CSpyVsSpy::CoopSyncStage(NULL,NULL,TRUE);
					// Empty weapons suppressed reticles during packing; release that override
					// after safe breakout so newly equipped weapons can draw crosshairs.
					CSpyVsSpy::TurnOffHUD(FALSE);
					DEVPRINTF("Factory packing: partner wait ended after box exit.\n");
				}
			}
#endif
		break;
	}

	_DialogWork();
}

void CProgramStage::SwitchTo( void ) {
//	m_eStageState = STATE_WAIT_FIRST_WORK;
	m_eStageState = STATE_BOX;

	m_fProgramTime = 0.0f;
	m_fTimeTillNextRumble = 0.0f;

#if FANG_WINGC
	// Retail funnyname disables animation driving in its builder. Its clock
	// otherwise stays at the initial pose forever, blocking packing completion.
	m_pCrate->DriveMeshWithAnim( TRUE );
	m_pCrate->AlwaysWork( TRUE );
	m_pArms->DriveMeshWithAnim( TRUE );
	m_pArms->AlwaysWork( TRUE );
#endif
	m_pCrate->UserAnim_Pause( TRUE );
	m_pCrate->UserAnim_SetSpeedMult( 1.0f );
	m_pCrate->UserAnim_SetClampMode( TRUE );
	m_pCrate->UserAnim_UpdateTime( _PROGRAM_BACK_UP_TIME );

	m_pArms->UserAnim_Pause( TRUE );
	m_pArms->UserAnim_SetSpeedMult( 1.0f );
	m_pArms->UserAnim_SetClampMode( TRUE );
	m_pArms->UserAnim_UpdateTime( _PROGRAM_BACK_UP_TIME );

//	if( m_pProgramParticle ) {
//		m_pProgramParticle->StopEmission( TRUE );
//		m_pProgramParticle = NULL;
//	}

	m_bDialogStarted = FALSE;
	m_uDialogIndex = 0;
	m_fDialogIdle = 0.0f;

	m_bPackUp = FALSE;

	m_pCrate->SetInvincible( TRUE );

	CSpyVsSpy::AttackDisable( TRUE );
	CSpyVsSpy::TurnOffHUD( TRUE );
	CSpyVsSpy::CoopSyncStage( "Empty Primary", "Empty Secondary", TRUE );

//	m_fParticleAngle = 0.0f;

	_SetFinished( FALSE );

	CFVec3A Pos = CFVec3A::m_UnitAxisY;

	Pos.Mul( 15.0f );
	Pos.Add( m_SpawnPoint );

	m_pAmbientEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_PROGRAM_AMBIENT ), FALSE, &Pos, 0.0f );
	CEmitterFader::Fade( m_pAmbientEmitter, 0.0f, _PROGRAM_AMBIENT_VOL, 2.0f );
}


void CProgramStage::Cleanup( void ) {
	if( m_pAmbientEmitter ) {
		m_pAmbientEmitter->Destroy();
		m_pAmbientEmitter = NULL;
	}
}

void CProgramStage::_ClearDataMembers( void ) { 
//	m_pProgramParticle = NULL;
//	m_eStageState = STATE_WAIT_FIRST_WORK;
	m_eStageState = STATE_BOX;

	m_fProgramTime = 0.0f;
	m_fTimeTillNextRumble = 0.0f;
	m_fUnitInterp = 0.0f;

	m_pCrate = NULL;
	m_pArms = NULL;
	m_bPackUp = FALSE;

	m_bDialogStarted = FALSE;
	m_uDialogIndex = 0;
	m_fDialogIdle = 0.0f;

	m_pAmbientEmitter = NULL;

//	m_fParticleAngle = 0.0f;
}

//void CProgramStage::_LookWork( void ) {
//	CFVec3A LookPoint;
//
//	LookPoint = FMath_aRotMtx43A[ FMTX_ROTTYPE_90Y ].m_vFront;
//	LookPoint.Mul( 100.0f );
//	LookPoint.Add( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
//
//	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->BeginFrame();
//
//	if( CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->FaceToward( LookPoint ) ) {
//		m_eStageState = STATE_PROGRAM;
//		m_fTimeTillNextRumble = _PROGRAM_RUMBLE_TIME;
//
//		CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_PROGRAM_LOOP ) );
//
//		CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, _PROGRAM_REPROGRAM_TIME, 0.5f, TRUE );
//	}
//
//	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->EndFrame();
//
//	_ParticleWork();
//}
//
//void CProgramStage::_ProgramWork( void ) {
//	m_fProgramTime += FLoop_fPreviousLoopSecs;
//	FMATH_CLAMPMAX( m_fProgramTime, ( _PROGRAM_REPROGRAM_TIME + _PROGRAM_REPROGRAM_TIME_EXTRA ) );
//
//	if( m_fProgramTime == ( _PROGRAM_REPROGRAM_TIME + _PROGRAM_REPROGRAM_TIME_EXTRA ) ) {
//		CSpyVsSpy::TakeControlFromPlayer( FALSE );
//		m_eStageState = STATE_BOX;
//
//		ai_SetRace( CSpyVsSpy::GetGlitch()->AIBrain(), AIRACE_MIL );
//
//		if( m_pProgramParticle ) {
//			m_pProgramParticle->StopEmission( TRUE );
//			m_pProgramParticle = NULL;
//		}
//	}
//
//	m_fTimeTillNextRumble -= FLoop_fPreviousLoopSecs;
//	FMATH_CLAMPMIN( m_fTimeTillNextRumble, 0.0f );
//
//	if( m_fTimeTillNextRumble == 0.0f && m_fProgramTime <= _PROGRAM_REPROGRAM_TIME ) {
//		m_fTimeTillNextRumble = _PROGRAM_RUMBLE_TIME;
//		CSpyVsSpy::GetGlitch()->ShakeCamera( _STAGE1_SHAKE_UNIT_INTENSITY, _STAGE1_SHAKE_DURATION );
//	}
//
//	_ParticleWork();
//}

void CProgramStage::_CrateWork( void ) {
	if( !m_bPackUp ) {
		f32 fDistSq = m_pCrate->MtxToWorld()->m_vPos.DistSq( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );

		if( fDistSq <= 1.5f ) {
			m_bPackUp = TRUE;
#if FANG_WINGC
			// The animated walls are not a stable moving floor. Carry the story
			// body explicitly until the box's authored destruction detaches it.
			CSpyVsSpy::GetGlitch()->Attach_ToParent_WithGlue_WS(m_pCrate);
			for( s32 n=1; MultiplayerMgr.IsLocalCoop() && n<CPlayer::m_nPlayerCount; ++n ) {
				CPlayer *pPlayer=&Player_aPlayer[n];
				CBot *pBot=(CBot *)pPlayer->m_pEntityOrig;
				if(!pBot) continue;
				pPlayer->ZeroControls(); pPlayer->m_HumanControl.Zero();
				pPlayer->DisableEntityControl();
				pBot->ZeroVelocity(); pBot->ImmobilizeBot();
				pBot->DrawEnable(FALSE,TRUE);
			}
			DEVPRINTF("Factory packing: closing the box; partners held until safe exit.\n");
#endif
			m_pCrate->UserAnim_Pause( FALSE );
			m_pArms->UserAnim_Pause( FALSE );
			CSpyVsSpy::TakeControlFromPlayer( TRUE );

			CEmitterFader::Fade( m_pAmbientEmitter, _PROGRAM_AMBIENT_VOL, 0.0f, 2.0f, TRUE );

			CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_START );
			CFSoundGroup::PlaySound( pGroup );
		}
	} else {
		if( m_pCrate->UserAnim_GetCurrentInst()->GetUnitTime() >= 0.99f ) {
			m_eStageState = STATE_BOX_MOVE;
			m_fUnitInterp = 0.0f;
			m_CrateStart = m_pCrate->MtxToWorld()->m_vPos;
#if FANG_WINGC
			DEVPRINTF("Factory packing: box closed; starting conveyor.\n");
#endif

			CFAudioEmitter *pEmitter = CFSoundGroup::AllocAndPlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CONVEYOR_LOOP ) );
			CEmitterFader::FadeInAndOut( pEmitter, 0.0f, 1.0f, fmath_Inv( _PROGRAM_MOVE_TIME ) + 2.0f, 0.1f, TRUE );

			CSpyVsSpy::GetGlitch()->ShakeCamera( 0.5f, 0.75f );
		}
	}
}

void CProgramStage::_CrateMove( void ) {
	m_fUnitInterp += ( FLoop_fPreviousLoopSecs * _PROGRAM_MOVE_TIME );
	FMATH_CLAMP( m_fUnitInterp, 0.0f, 1.0f );

	CFVec3A LerpPos;

	LerpPos.Lerp( m_fUnitInterp, m_CrateStart, m_CrateDest );
	m_pCrate->Relocate_Xlat_WS( &LerpPos );

	if( m_fUnitInterp == 1.0f ) {
		m_eStageState = STATE_IDLE;
		m_bDialogStarted = TRUE;
#if FANG_WINGC
		DEVPRINTF("Factory packing: delivery complete; P1 restored, partners watching until safe exit.\n");
#endif

		CSpyVsSpy::TakeControlFromPlayer( FALSE );
		CSpyVsSpy::AttackDisable( FALSE );
		CSpyVsSpy::TurnOffHUD( FALSE );
		CSpyVsSpy::CoopSyncStage( "Empty Primary", "Empty Secondary", FALSE );
#if FANG_WINGC
		// Partners watch the boxed story actor until he exits onto checked ground.
		for(s32 n=1;MultiplayerMgr.IsLocalCoop() && n<CPlayer::m_nPlayerCount;++n)
			CHud2::GetHudForPlayer(n)->SetDrawEnabled(FALSE);
#endif
		CSpyVsSpy::GetGlitch()->ShakeCamera( 0.5f, 0.75f );

		CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_STOP );
		CFSoundGroup::PlaySound( pGroup );
	}
}

void CProgramStage::_DialogWork( void ) {
	if( !m_bDialogStarted ) {
		return;
	}

	if( m_fDialogIdle == 0.0f ) {
		if( m_uDialogIndex >= _PROGRAM_NUM_DIALOG_ITEMS ) {
			m_bDialogStarted = FALSE;
			m_pCrate->SetInvincible( FALSE );
			return;
		}

		CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( m_auDialogSounds[ m_uDialogIndex ] );

		if( !pGroup ) {
			return;
		}

		m_fDialogIdle = fsndfx_GetDuration( CFSoundGroup::GetSoundHandle( pGroup, 0 ) ) + 0.5f;

		CFSoundGroup::PlaySound( pGroup, TRUE );

		++m_uDialogIndex;
	} else {
		m_fDialogIdle -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fDialogIdle, 0.0f );
	}
}

//void CProgramStage::_ParticleWork( void ) {
//	if( !m_pProgramParticle ) {
//		return;
//	}
//
//	m_fParticleAngle += ( FLoop_fPreviousLoopSecs * FMATH_PI );
//
//	if( m_fParticleAngle > FMATH_2PI ) {
//		m_fParticleAngle -= FMATH_2PI;
//	}
//
//	CFVec3A Dir;
//
//	Dir = FMath_aRotMtx43A[ FMTX_ROTTYPE_90Y ].m_vFront;
//	Dir.RotateY( m_fParticleAngle );
//	Dir.Mul( _PROGRAM_RADIUS_SIZE );
//	Dir.Add( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
//	Dir.y += 0.25f;
//	m_pProgramParticle->Relocate_Xlat_WS( &Dir );
//}



//
//
//
// CSpyVsSpy
//
//
//
BOOL CSpyVsSpy::m_bLevelInitted = FALSE;
CSpyVsSpy *CSpyVsSpy::m_pGame = NULL;

//CBotGlitch *CSpyVsSpy::m_pGlitch = NULL;
//
//BOOL8 CSpyVsSpy::m_bHaveHead = FALSE;
//BOOL8 CSpyVsSpy::m_bHaveTorso = FALSE;
//BOOL8 CSpyVsSpy::m_bHaveLegs = FALSE;
//BOOL8 CSpyVsSpy::m_bStreamBlock = FALSE;
//BOOL8 CSpyVsSpy::m_bHaveMessage = FALSE;
//BOOL8 CSpyVsSpy::m_bHudState = TRUE;
//
//CFQuatA CSpyVsSpy::m_HeadStartQuat;
//CFQuatA CSpyVsSpy::m_TorsoStartQuat;
//CFQuatA CSpyVsSpy::m_LegsStartQuat;
//CFVec3A CSpyVsSpy::m_HeadStartPos;
//CFVec3A CSpyVsSpy::m_TorsoStartPos;
//CFVec3A CSpyVsSpy::m_LegsStartPos;
//CFMtx43A CSpyVsSpy::m_HeadMtx;
//CFMtx43A CSpyVsSpy::m_TorsoMtx;
//CFMtx43A CSpyVsSpy::m_LegsMtx;
//CFMtx43A CSpyVsSpy::m_Listener;

SpyVsSpyConfigValues_t *CSpyVsSpy::m_pConfigValues = NULL;
CSpyVsSpy::SpyCommonData_t *CSpyVsSpy::m_pCommonData = NULL;

BOOL CSpyVsSpy::LoadLevel( LevelEvent_e eEvent ) {
	FResFrame_t Frame = fres_GetFrame();

	if( eEvent == LEVEL_EVENT_POST_WORLD_LOAD ) {
		FASSERT( !m_bLevelInitted );

		m_pCommonData = (SpyCommonData_t *) fres_AllocAndZero( sizeof( SpyCommonData_t ) );
		
		if( !m_pCommonData ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for common data.\n" );
			goto _ExitWithError;
		}

		m_pConfigValues = (SpyVsSpyConfigValues_t *) fres_AllocAndZero( sizeof( SpyVsSpyConfigValues_t ) );

		if( !m_pConfigValues ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for config values.\n" );
			goto _ExitWithError;
		}

		m_pCommonData->m_pGlitch = NULL;

		m_pCommonData->m_bHaveHead = FALSE;
		m_pCommonData->m_bHaveTorso = FALSE;
		m_pCommonData->m_bHaveLegs = FALSE;

		m_pCommonData->m_HeadStartPos.Zero();
		m_pCommonData->m_TorsoStartPos.Zero();
		m_pCommonData->m_LegsStartPos.Zero();

		m_pCommonData->m_HeadStartQuat.Identity();
		m_pCommonData->m_TorsoStartQuat.Identity();
		m_pCommonData->m_LegsStartQuat.Identity();

		m_pCommonData->m_HeadMtx.Identity();
		m_pCommonData->m_TorsoMtx.Identity();
		m_pCommonData->m_LegsMtx.Identity();

		m_bLevelInitted = TRUE;

		if( !fresload_Load( FSNDFX_RESTYPE, _SPY_VS_SPY_SOUNDS ) ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Could not load sound effect bank '%s'\n", _SPY_VS_SPY_SOUNDS );
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SPY_VS_SPY_DIALOG ) ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Could not load sound effect bank '%s'\n", _SPY_VS_SPY_DIALOG );
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SPY_VS_SPY_TALKS ) ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Could not load sound effect bank '%s'\n", _SPY_VS_SPY_TALKS );
		}

		if( !_LoadLevelCSV() ) {
			goto _ExitWithError;
		}

		if( !CSpyVsSpySoundCenter::Load() ) {
			goto _ExitWithError;
		}

		CEmitterFader::Setup();

		m_pGame = fnew CSpyVsSpy;

		if( m_pGame == NULL ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for CSpyVsSpy.\n" );
			goto _ExitWithError;
		}

		if( !m_pGame->_Load( eEvent ) ) {
			goto _ExitWithError;
		}

		m_pCommonData->m_HeadStartQuat.Identity();
		m_pCommonData->m_TorsoStartQuat.Identity();
		m_pCommonData->m_LegsStartQuat.Identity();

		m_pCommonData->m_HeadStartPos.Zero();
		m_pCommonData->m_TorsoStartPos.Zero();
		m_pCommonData->m_LegsStartPos.Zero();

		m_pCommonData->m_HeadMtx.Identity();
		m_pCommonData->m_TorsoMtx.Identity();
		m_pCommonData->m_LegsMtx.Identity();

//		if( game_GetListenerOrientationCallback() ) {
//			DEVPRINTF( "CSpyVsSpy::LoadLeve(): game_GetListenerOrientationCallback() returned a non-NULL value.\n" );
//			goto _ExitWithError;
//		}

		m_pCommonData->m_Listener.Identity();

	} else if( eEvent == LEVEL_EVENT_PRE_ENTITY_FIXUP ) {
		if( !m_pGame->_Load( eEvent ) ) {
			goto _ExitWithError;
		}

		CFXMeshBuildPartMgr::SubmitInitNeeds( 60, 2 );

		CCollectable::NotifyCollectableUsedInWorld( COLLECTABLE_CHIP );
	}

	return TRUE;

_ExitWithError:
	UnloadLevel();

	fres_ReleaseFrame( Frame );

	return FALSE;
}

static CHud2 *_SpyTransmissionHud( void ) {
	const s32 nPlayer = MultiplayerMgr.IsLocalCoop() ? 0 : CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex;
	return CHud2::GetHudForPlayer( nPlayer );
}

static void _SpyTransmissionStop( void ) {
	if( MultiplayerMgr.IsLocalCoop() ) CHud2::TransmissionShared_Stop( TRUE );
	else _SpyTransmissionHud()->TransmissionMsg_Stop();
}

void CSpyVsSpy::UnloadLevel( void ) {
	if( !m_bLevelInitted ) {
		return;
	}

	if( CSpyVsSpy::GetGlitch() && ( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex != -1 ) && 
		!_SpyTransmissionHud()->TransmissionMsg_IsDonePlaying() ) {
		_SpyTransmissionStop();
	}

	CSpyVsSpySoundCenter::Unload();
	CEmitterFader::Reset();

	m_pGame->_Unload();

	fdelete( m_pGame );
	m_pGame = NULL;

	m_bLevelInitted =  FALSE;

	m_pCommonData->m_pGlitch = NULL;

	m_pCommonData->m_bHaveHead = FALSE;
	m_pCommonData->m_bHaveTorso = FALSE;
	m_pCommonData->m_bHaveLegs = FALSE;

	if( game_GetListenerOrientationCallback() == &ListenerOrientationCallback ) {
		game_SetListenerOrientationCallback( NULL );		
	}

	m_pCommonData = NULL;
	m_pConfigValues = NULL;
}

void CSpyVsSpy::Work( void ) {
	FASSERT( m_bLevelInitted );
	FASSERT( m_pGame );

	if( m_pCommonData->m_bHaveMessage ) {
		if( !_SpyTransmissionHud()->TransmissionMsg_IsDonePlaying() ) {
			if( m_pCommonData->m_bStreamBlock ) {
				return;
			}
		} else {
			m_pCommonData->m_bHaveMessage = FALSE;

			// If we were blocking, let them run around again
			if( m_pCommonData->m_bStreamBlock ) {
				CSpyVsSpy::TakeControlFromPlayer( FALSE );
			}

			// Do this so the letterbox has a chance to recover
			return;
		}
	}

	m_pGame->_Work();
}

void CSpyVsSpy::SetupGlitchPointer( CBotGlitch *pGlitch ) {
	FASSERT( pGlitch );
	m_pCommonData->m_pGlitch = pGlitch;
}

void CSpyVsSpy::CheckpointRestore( s32 nCheckpoint ) {
	if( !m_bLevelInitted ) {
		return;
	}

	// Make sure this happens before the restore since we start the message in the restore
	if( !_SpyTransmissionHud()->TransmissionMsg_IsDonePlaying() ) {
		_SpyTransmissionStop();
	}

	CEmitterFader::Reset();

	m_pCommonData->m_bStreamBlock = FALSE;
	m_pCommonData->m_bHaveMessage = FALSE;

	m_pCommonData->m_bHaveHead = FALSE;
	m_pCommonData->m_bHaveTorso = FALSE;
	m_pCommonData->m_bHaveLegs = FALSE;

	m_pGame->_CheckpointRestore( nCheckpoint );
}

void CSpyVsSpy::StreamStart( cchar *pszName, BOOL bBlock /*= FALSE */, BOOL bCanSkip/* = FALSE*/ ) {
	// !!Nate - this is temp since we have an extra Restore() at level load
	if( !_SpyTransmissionHud()->TransmissionMsg_IsDonePlaying() ) {
		return;
	}

	const BOOL bStarted = MultiplayerMgr.IsLocalCoop() ?
		CHud2::TransmissionShared_Start( CHud2::TRANSMISSION_AUTHOR_AGENT_SHHH, pszName, 1.0f, TRUE ) :
		_SpyTransmissionHud()->TransmissionMsg_Start( CHud2::TRANSMISSION_AUTHOR_AGENT_SHHH, pszName, 1.0f, TRUE );
	if( !bStarted ) {
		DEVPRINTF( "CSpyVsSpy::StreamStart(): Failed to start stream '%s'.\n", pszName );
		return;
	}

	m_pCommonData->m_bStreamBlock = bBlock;

	if( m_pCommonData->m_bStreamBlock ) {
		CSpyVsSpy::TakeControlFromPlayer( TRUE, bCanSkip );
	}

	m_pCommonData->m_bHaveMessage = TRUE;
}

f32 CSpyVsSpy::GetIdleTime( f32 fUnitTime ) {
//	f32 fCommand = FMATH_FPOT( fUnitTime, m_pConfigValues->fDDRCommandTimeMax, m_pConfigValues->fDDRCommandTimeMin );
//	f32 fMove = FMATH_FPOT( fUnitTime, m_pConfigValues->fDDRMoveTimeMax, m_pConfigValues->fDDRMoveTimeMin );
//
//	return fCommand - fMove;
	return GetCommandTime( fUnitTime ) - GetMoveTime( fUnitTime );
}

f32 CSpyVsSpy::GetCommandTime( f32 fUnitTime ) {
	return FMATH_FPOT( fUnitTime, m_pConfigValues->fDDRCommandTimeMax, m_pConfigValues->fDDRCommandTimeMin );
}

f32 CSpyVsSpy::GetMoveTime( f32 fUnitTime ) {
	//return GetCommandTime( fUnitTime ) - GetIdleTime( fUnitTime );
	return FMATH_FPOT( fUnitTime, m_pConfigValues->fDDRMoveTimeMax, m_pConfigValues->fDDRMoveTimeMin );
}

void CSpyVsSpy::TurnOffHUD( BOOL bOff, BOOL bNoReticle, BOOL bNoWeaponSel ) {
	s32 nCount = 1;
#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() ) nCount = CPlayer::m_nPlayerCount;
#endif
	for( s32 n = 0; n < nCount; ++n ) {
		CBot *pBot = n ? (CBot *)Player_aPlayer[n].m_pEntityOrig : GetGlitch();
		if( !pBot ) continue;
		CHud2 *pHud = CHud2::GetHudForPlayer( pBot->m_nPossessionPlayerIndex );
		pHud->SetDrawEnabled( !bOff );
		pHud->SetWSEnable( !bOff && !bNoWeaponSel );
		if( !bNoReticle ) pBot->ReticleEnable( !bOff );
	}
	m_pCommonData->m_bHudState = bOff;
}

void CSpyVsSpy::AttackDisable( BOOL bOff ) {
	s32 nCount = 1;
#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() ) nCount = CPlayer::m_nPlayerCount;
#endif
	for( s32 n = 0; n < nCount; ++n ) {
		CBot *pBot = n ? (CBot *)Player_aPlayer[n].m_pEntityOrig : GetGlitch();
		if( !pBot ) continue;
		if( bOff ) pBot->SetBotFlag_DontAllowFire();
		else pBot->ClearBotFlag_DontAllowFire();
	}
}

#if FANG_WINGC
void CSpyVsSpy::CoopSyncStage( cchar *pszPrimary, cchar *pszSecondary, BOOL bRegroup ) {
	if( !MultiplayerMgr.IsLocalCoop() ) return;
	if( bRegroup ) {
		CPlayer::CoopPlaceStartingPartners( TRUE, 0 );
		CPlayer::CoopReviveForCheckpoint();
		CPlayer::CoopPlaceStartingPartners( TRUE, 0 );
	}
	CHud2 *pLeadHud = CHud2::GetHudForPlayer( 0 );
	const u32 leadEnabled = pLeadHud->DrawFlagsEnabled( CHud2::DRAW_GLITCH_HUD );
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
		CPlayer *pPlayer = &Player_aPlayer[n];
		CBot *pBot = (CBot *)pPlayer->m_pEntityOrig;
		if( !pBot || !pBot->m_pInventory ) continue;
		CHud2 *pHud = CHud2::GetHudForPlayer( n );
		pHud->EnableDrawFlags( CHud2::DRAW_GLITCH_HUD );
		pHud->DisableDrawFlags( CHud2::DRAW_GLITCH_HUD & ~leadEnabled );
		pHud->SetDrawEnabled( pLeadHud->IsDrawEnabled() );
		pHud->SetWSEnable( pLeadHud->IsWSEnabled() );
		cchar *aNames[2] = { pszPrimary, pszSecondary };
		for( s32 slot = 0; slot < 2; ++slot ) {
			if( !aNames[slot] ) continue;
			CItemInst *pItem = pBot->m_pInventory->IsWeaponInInventory( aNames[slot] );
			if( !pItem ) {
				CCollectable::GiveWeaponToPlayer( pBot, aNames[slot] );
				pItem = pBot->m_pInventory->IsWeaponInInventory( aNames[slot] );
			}
			if( pItem ) pBot->m_pInventory->SetCurWeapon( slot ? INV_INDEX_SECONDARY : INV_INDEX_PRIMARY, pItem, FALSE, TRUE );
		}
		if( pszPrimary && !fclib_stricmp( pszPrimary, "Empty Primary" ) ) pBot->ReticleEnable( FALSE );
		if( n && bRegroup && !pBot->IsDeadOrDying() ) {
			pBot->MobilizeBot();
			pBot->DrawEnable( TRUE, TRUE );
			pPlayer->EnableEntityControl();
			aibrainman_ConfigurePlayerBotBrain( pBot->AIBrain(), n );
			ai_SetRace( pBot->AIBrain(), ai_GetRace( GetGlitch()->AIBrain() ) );
			pBot->m_vecCamPosAdj.Zero();
			pBot->m_vecCamLookAdj.Zero();
			gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(n), pBot );
		}
	}
	DEVPRINTF( "Co-op: factory stage synchronizes %d players (%s / %s, regroup %d).\n",
		CPlayer::m_nPlayerCount, pszPrimary ? pszPrimary : "unchanged", pszSecondary ? pszSecondary : "unchanged", bRegroup );
}

BOOL CSpyVsSpy::CoopIndividualViews() {
 if(!IsFactoryActive() || !MultiplayerMgr.IsLocalCoop()) return FALSE;
 if(m_pGame->m_eCurrentStage==STAGE_SEARCH)
  return ((CSearchStage *)m_pGame->m_pSearchStage)->CoopIndividualViews();
 return FALSE;
}

void CSpyVsSpy::ConstrainPackingControls(CEntity *body, CHumanControl *controls) {
	if(!controls || !IsFactoryActive() || m_pGame->m_eCurrentStage!=STAGE_PROGRAM || body!=GetGlitch()) return;
	if(!((CProgramStage *)m_pGame->m_pProgramStage)->Contains(body)) return;
	// The carried actor may aim/select/fire to break out, but must not jump or
	// walk through animated walls before the authored box exit. Solo also needs this.
	controls->m_fForward=controls->m_fStrafeRight=controls->m_fCrossDown=0;
	controls->m_nPadFlagsJump=0;
}

BOOL CSpyVsSpy::CoopPackingWaiting() {
	if(!IsFactoryActive() || !MultiplayerMgr.IsLocalCoop() || m_pGame->m_eCurrentStage!=STAGE_PROGRAM) return FALSE;
	CBot *partner=(CBot *)Player_aPlayer[1].m_pEntityOrig;
	return partner && !partner->IsDrawEnabled() && !Player_aPlayer[1].HasEntityControl();
}

BOOL CSpyVsSpy::PortTestEscort(u32 test) {
 char mode[32];GetEnvironmentVariableA("MA_PORT_TEST_COOP_POLISH",mode,sizeof(mode));
 if(fclib_stricmp(mode,"spy-escort") || !IsFactoryActive()) return FALSE;
 return ((CSearchStage *)m_pGame->m_pSearchStage)->PortTestEscort(test);
}

BOOL CSpyVsSpy::PortTestInspection(u32 test) {
 char mode[32];GetEnvironmentVariableA("MA_PORT_TEST_COOP_POLISH",mode,sizeof(mode));
 if(fclib_stricmp(mode,"spy-inspection") || !IsFactoryActive()) return FALSE;
 return ((CSearchStage *)m_pGame->m_pSearchStage)->PortTestInspection(test);
}

BOOL CSpyVsSpy::PortTestDance( u32 nTest ) {
	if( !IsFactoryActive() ) return FALSE;
	return ((CDDRStage *)m_pGame->m_pDanceStage)->PortTestParty( nTest );
}

BOOL CSpyVsSpy::PortTestStage( u32 nStage ) {
	char mode[32];
	GetEnvironmentVariableA( "MA_PORT_TEST_COOP_POLISH", mode, sizeof(mode) );
	if( (fclib_stricmp(mode,"spy-party") && fclib_stricmp(mode,"spy-escort") && fclib_stricmp(mode,"spy-packing") && fclib_stricmp(mode,"spy-resume") && fclib_stricmp(mode,"spy-inspection") && fclib_stricmp(mode,"spy-box-resume")) || !IsFactoryActive() ) return FALSE;
	if( nStage == 0 ) { m_pGame->m_eCurrentStage = STAGE_DANCE; m_pGame->m_pCurrentStage = m_pGame->m_pDanceStage; }
	else if( nStage == 1 ) { m_pGame->m_eCurrentStage = STAGE_BATTLE; m_pGame->m_pCurrentStage = m_pGame->m_pBattleStage; }
	else if(nStage==2) {m_pGame->m_eCurrentStage=STAGE_SEARCH;m_pGame->m_pCurrentStage=m_pGame->m_pSearchStage;}
	else if(nStage==3) {m_pGame->m_eCurrentStage=STAGE_PROGRAM;m_pGame->m_pCurrentStage=m_pGame->m_pProgramStage;}
	else return FALSE;
	m_pCommonData->m_bHaveMessage = FALSE;
	_SpyTransmissionStop();
	if(nStage==1) {
		// Explicit opt-in test/resume needs the loadout normally awarded by training.
		for(s32 n=0;n<CPlayer::m_nPlayerCount;++n) {
			CBot *bot=(CBot *)Player_aPlayer[n].m_pEntityOrig;
			if(!bot) continue;
			CCollectable::GiveWeaponToPlayer(bot,"RLauncher L1",1);
			CCollectable::GiveWeaponToPlayer(bot,"Empty Secondary",1);
		}
	}
	m_pGame->m_pCurrentStage->Restore();
	return TRUE;
}
#else
void CSpyVsSpy::CoopSyncStage( cchar *, cchar *, BOOL ) {}
#endif

CEntity *CSpyVsSpy::FindEntity( cchar *pszName, const u64 &uTypeBits/* = 0*/, BOOL bRequired/* = TRUE*/ ) {
	CEntity *pEntity;

	// Find the gurney
	pEntity = CEntity::FindInWorld( pszName );
	if( pEntity == NULL ) {
		DEVPRINTF( bRequired ? "CSpyVsSpy::_FindMeshEntity(): Entity '%s' was not found.\n" : "CSpyVsSpy: optional entity '%s' is not in this level.\n", pszName );
		return NULL;
	}
	
	if( !uTypeBits ) {
		return pEntity;
	}

	if( !( pEntity->TypeBits() & uTypeBits ) ) {
		DEVPRINTF( "CSpyVsSpy::_FindMeshEntity(): Entity '%s' was not of expected type.\n", pszName );
		return NULL;
	}

	return pEntity;
}

void CSpyVsSpy::TakeControlFromPlayer( BOOL bTake, BOOL bCanSkip/* = FALSE*/ ) {
	CPlayer *pPlayer = CPlayer::m_pCurrent;
#if FANG_WINGC
	// The factory stages operate on P1's story body. In co-op the last worked
	// player can be P2-P4, so return human control to the same body as the AI.
	if( MultiplayerMgr.IsLocalCoop() ) pPlayer = &Player_aPlayer[0];
	DEVPRINTF("Port: spy factory %s control for player %d.\n",
		bTake ? "takes" : "restores", pPlayer->m_nPlayerIndex + 1);
#endif
	if( !bTake ) {
		// Give control back to the player
		aibrainman_Deactivate( Player_aPlayer[ 0 ].m_pEntityCurrent->AIBrain() );
		pPlayer->EnableEntityControl();
		aibrainman_ConfigurePlayerBotBrain( Player_aPlayer[ 0 ].m_pEntityCurrent->AIBrain(), 0 );
		game_LeaveLetterbox( FALSE );
	} else {
		game_LeaveLetterbox( FALSE );
		game_EnterLetterbox( NULL, FALSE, bCanSkip );
		pPlayer->DisableEntityControl();
		aibrainman_Activate( Player_aPlayer[0].m_pEntityCurrent->AIBrain() );
	}
}

void CSpyVsSpy::CameraInterp( const f32 &fUnitTime,
						const f32 &fStartFOV, const CFVec3A &StartPos, const CFQuatA &StartQuat,
						const f32 &fEndFOV, const CFVec3A &EndPos, const CFQuatA &EndQuat,
						f32 *fCurrFOV, CFVec3A *CurrPos, CFQuatA *CurrQuat ) {

	f32 fUnitLin = fmath_UnitLinearToSCurve( fUnitTime );

	CurrPos->Lerp( fUnitLin, StartPos, EndPos );
	*fCurrFOV = FMATH_FPOT( fUnitLin, fStartFOV, fEndFOV );
	CurrQuat->ReceiveSlerpOf( fUnitLin, StartQuat, EndQuat );
}

void CSpyVsSpy::MakeBotDumb( CBot *pBot, f32 fSpeedPercent ) {
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_EYES );
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_EARS );
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_TOUCH );
	ai_TurnOffPerceptor( pBot->AIBrain(), AI_PERCEPTOR_RADIO );

	// Do this so our guy doesn't look around like a moron
	ai_AssignJob_Wait( pBot->AIBrain(), FALSE, CFVec3A::m_Null, 0, 0 );

	pBot->AIBrain()->GetAIMover()->m_uMoverFlags |= ( CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS | CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE );
	pBot->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( fSpeedPercent );

	pBot->IgnoreBotVBotCollision();
	pBot->EnableCameraCollision( FALSE );
}

void CSpyVsSpy::MakeBotSmart( CBot *pBot ) {
	ai_TurnOnPerceptor( pBot->AIBrain(), AI_PERCEPTOR_EYES );
	ai_TurnOnPerceptor( pBot->AIBrain(), AI_PERCEPTOR_EARS );
	ai_TurnOnPerceptor( pBot->AIBrain(), AI_PERCEPTOR_TOUCH );
	ai_TurnOnPerceptor( pBot->AIBrain(), AI_PERCEPTOR_RADIO );

	pBot->AIBrain()->GetAIMover()->m_uMoverFlags &= ~( CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS | CAIMover::MOVERFLAG_DISABLE_OBJECT_AVOIDANCE );
	pBot->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( 1.0f );

	pBot->DontIgnoreBotVBotCollision();
	pBot->EnableCameraCollision( FALSE );
}

#define _BOT_TALK_TIMESCALE ( 0.70f )

void CSpyVsSpy::PlayBotTalk( CBot *pBot, u32 uType, f32 *fTime/* = NULL*/ ) {
	FASSERT( pBot );
	FASSERT( uType < BTR_COUNT );

	cchar *pszBTAName;

	pszBTAName = AIBTA_EventToBTAName( _apszBTRNames[ uType ], pBot->AIBrain() );

	if( pszBTAName ) {
		if( ai_IsTalking( pBot->AIBrain() ) ) {
			ai_TalkModeEnd( pBot->AIBrain() );
		}

		if( ai_TalkModeBegin( pBot->AIBrain(), AIBrain_TalkModeCB, (void *) pszBTAName, pBot, 30, FALSE, FALSE, 0) ) {
			if( fTime ) {
				CBotTalkData *pData = CTalkSystem2::GetTalkDataByFileName( pszBTAName );

				*fTime = pData->m_fTotalTime * _BOT_TALK_TIMESCALE;
			}
		} else {
			if( fTime ) {
				*fTime = 0.0f;
			}
		}
	}
}

void CSpyVsSpy::ListenerOrientationCallback( u32 uPlayerIndex ) {
	// Every local co-op listener follows the shared crane camera during this
	// sequence. The retail zero-only assertion rejected the second listener.
	FASSERT( !uPlayerIndex || (MultiplayerMgr.IsLocalCoop() && uPlayerIndex < (u32)CPlayer::m_nPlayerCount) );
	FASSERT( MultiplayerMgr.IsSinglePlayer() );

	CFXfm xfmListener;

	xfmListener.BuildFromMtx( m_pCommonData->m_Listener );
	faudio_SetListenerOrientation( uPlayerIndex, &xfmListener );
}

BOOL CSpyVsSpy::_Load( LevelEvent_e eEvent ) {
	if( eEvent == LEVEL_EVENT_POST_WORLD_LOAD ) {
		// Setup the crane stage
		m_pCraneStage = (CSpyVsSpyStage *) fnew CCraneStage;

		if( m_pCraneStage == NULL ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for crane stage.\n" );
			goto _ExitWithError;
		}
	
		if( !m_pCraneStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

	
		// Setup the move stage
		m_pMoveStage = (CSpyVsSpyStage *) fnew CMoveStage;

		if( m_pMoveStage == NULL ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for move stage.\n" );
			goto _ExitWithError;
		}
	
		if( !m_pMoveStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		// Setup the search stage
		m_pSearchStage = (CSpyVsSpyStage *) fnew CSearchStage;

		if( m_pSearchStage == NULL ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for move stage.\n" );
			goto _ExitWithError;
		}

		if( !m_pSearchStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		// Setup the dancing stage
		m_pDanceStage = (CSpyVsSpyStage *) fnew CDDRStage;

		if( m_pDanceStage == NULL ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for dance stage.\n" );
			goto _ExitWithError;
		}

		if( !m_pDanceStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		// Setup the battle stage
		m_pBattleStage = (CSpyVsSpyStage *) fnew CBattleStage;

		if( m_pBattleStage == NULL ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for battle stage.\n" );
			goto _ExitWithError;
		}

		if( !m_pBattleStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}


		// Setup the program stage
		m_pProgramStage = fnew CProgramStage;

		if( m_pProgramStage == NULL ) {
			DEVPRINTF( "CSpyVsSpy::LoadLevel(): Failed to allocate memory for program stage.\n" );
			goto _ExitWithError;
		}

		if( !m_pProgramStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

//		level_SetCurStreamVolume( 1.0f );

		// !!Nate
		m_eCurrentStage = STAGE_CRANE;
		m_pCurrentStage = m_pCraneStage;
//		m_eCurrentStage = STAGE_MOVE;
//		m_pCurrentStage = m_pMoveStage;
//		m_eCurrentStage = STAGE_SEARCH;
//		m_pCurrentStage = m_pSearchStage;
//		m_eCurrentStage = STAGE_DANCE;
//		m_pCurrentStage = m_pDanceStage;
//		m_eCurrentStage = STAGE_BATTLE;
//		m_pCurrentStage = m_pBattleStage;
//		m_eCurrentStage = STAGE_PROGRAM;
//		m_pCurrentStage = m_pProgramStage;
	} else if( eEvent == LEVEL_EVENT_PRE_ENTITY_FIXUP ) {
		if( !m_pCraneStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		if( !m_pMoveStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		if( !m_pSearchStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		if( !m_pDanceStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		if( !m_pBattleStage->Load( eEvent ) ) { 
			goto _ExitWithError;
		}

		if( !m_pProgramStage->Load( eEvent ) ) {
			goto _ExitWithError;
		}

		// !!nate - Temp for dev stuff
		//m_pCurrentStage->Restore();

		m_pCurrentStage->SwitchTo();
	}

	return TRUE;

_ExitWithError:
	_Unload();
	return FALSE;
}

void CSpyVsSpy::_Unload( void ) {
	if( m_pCraneStage ) {
		m_pCraneStage->Unload();

        fdelete( m_pCraneStage );
		m_pCraneStage = NULL;
	}

	if( m_pMoveStage ) {
		m_pMoveStage->Unload();

        fdelete( m_pMoveStage );
		m_pMoveStage = NULL;
	}

	if( m_pSearchStage ) {
		m_pSearchStage->Unload();

		fdelete( m_pSearchStage );
		m_pSearchStage = NULL;
	}

	if( m_pDanceStage ) {
		m_pDanceStage->Unload();

		fdelete( m_pDanceStage );
		m_pDanceStage = NULL;
	}

	if( m_pBattleStage ) {
		m_pBattleStage->Unload();

		fdelete( m_pBattleStage );
		m_pBattleStage = NULL;
	}
	
	if( m_pProgramStage ) {
		m_pProgramStage->Unload();

		fdelete( m_pProgramStage );
		m_pProgramStage = NULL;
	}
}

void CSpyVsSpy::_Work( void ) {
	FASSERT( GetGlitch() );

	CEmitterFader::Work();

	m_pCurrentStage->Work();

	if( m_pCurrentStage->IsFinished() ) {
		switch( m_eCurrentStage ) {
			case STAGE_CRANE:
				m_pCurrentStage = m_pMoveStage;
				m_eCurrentStage = STAGE_MOVE;
			break;
			case STAGE_MOVE:
				m_pCurrentStage = m_pSearchStage;
				m_eCurrentStage = STAGE_SEARCH;
			break;	
			case STAGE_SEARCH:
				m_pCurrentStage = m_pDanceStage;
				m_eCurrentStage = STAGE_DANCE;
			break;
			case STAGE_DANCE:
				m_pCurrentStage = m_pBattleStage;
				m_eCurrentStage = STAGE_BATTLE;
			break;
			case STAGE_BATTLE:
				m_pCurrentStage = m_pProgramStage;
				m_eCurrentStage = STAGE_PROGRAM;
			break;	
		}

		m_pCurrentStage->SwitchTo();
	}
}


void CSpyVsSpy::_ClearDataMembers( void ) {
	m_eCurrentStage = STAGE_CRANE;
	m_pCurrentStage = NULL;

	m_pCraneStage = NULL;
	m_pMoveStage = NULL;
	m_pSearchStage = NULL;
	m_pDanceStage = NULL;
	m_pBattleStage = NULL;
	m_pProgramStage = NULL;
}

void CSpyVsSpy::_CheckpointRestore( s32 nCheckpoint ) {
	if( m_pCommonData->m_bHaveMessage ) {
		if( _SpyTransmissionHud()->TransmissionMsg_IsDonePlaying() ) {
			_SpyTransmissionStop();
		}

		m_pCommonData->m_bHaveMessage = FALSE;
	}

//	if( nCheckpoint == 1 ) {
		m_pCurrentStage->Restore();
//	} else {
//		m_eCurrentStage = STAGE_CRANE;
//		m_pCurrentStage = m_pCraneStage;
//		m_pCurrentStage->Restore();
//	}
}

const FGameData_TableEntry_t _aUserPropVocab[] = {
	// f32 fDDRCommandTimeMin;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// f32 fDDRCommandTimeMax;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// f32 fDDRMoveTimeMin;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// f32 fDDRMoveTimeMax;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// f32 fDDRAlignFailTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// f32 fDDRPositionFailTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// f32 fDDRLookFailTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// f32 fDDRMoveDistance;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000,

	// f32 fDDRLagDistance;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1000,
	F32_DATATABLE_0,

	// f32 fDDRBumpUpDistance;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X  | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// u32 uDDRMoveTimeMin
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

BOOL CSpyVsSpy::_LoadLevelCSV( void ) {
	FResFrame_t Frame = fres_GetFrame();
	FGameDataFileHandle_t hHandle;

	hHandle = (FGameDataFileHandle_t) fresload_Load( FGAMEDATA_RESTYPE, _SPY_VS_SPY_CONFIG );

	if( hHandle == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CSpyVsSpy::_LoadLevelCSV(): Failed to load '%s'.\n", _SPY_VS_SPY_CONFIG );
		goto _ExitWithError;
	}

#if FANG_WINGC
	// Retail spy_cfg has no "info" table. Its "ddr" table holds records of this same 11-field
	// vocabulary (four in the retail data). The retail game loads up to 30 of them and gives each
	// DDR stage the record its level data names; this port uses the first record throughout.
	cchar *pszTableName = "ddr";
#else
	cchar *pszTableName = "info";
#endif
	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( hHandle, pszTableName );

	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CSpyVsSpy::_LoadLevelCSV(): Could not find \"%s\" table in CSV.\n", pszTableName );
		goto _ExitWithError;
	}

	// Found table...
	if( !fgamedata_GetTableData( hTable, _aUserPropVocab, m_pConfigValues, sizeof( SpyVsSpyConfigValues_t ) ) ) {
		DEVPRINTF( "CSpyVsSpy::_LoadLevelCSV(): Failed to load table.\n" );
		goto _ExitWithError;		
	}

	if( m_pConfigValues->fDDRCommandTimeMin > m_pConfigValues->fDDRCommandTimeMax ) {
		DEVPRINTF( "CSpyVsSpy::_LoadLevelCSV(): Invalid move time range (%f to %f).\n", m_pConfigValues->fDDRCommandTimeMin, m_pConfigValues->fDDRCommandTimeMax );
		goto _ExitWithError;				
	}

	if( m_pConfigValues->fDDRMoveTimeMin > m_pConfigValues->fDDRMoveTimeMax ) {
		DEVPRINTF( "CSpyVsSpy::_LoadLevelCSV(): Invalid move time range (%f to %f).\n", m_pConfigValues->fDDRMoveTimeMin, m_pConfigValues->fDDRMoveTimeMax );
		goto _ExitWithError;				
	}

	if( m_pConfigValues->fDDRMoveTimeMin >= m_pConfigValues->fDDRCommandTimeMin ) {
		m_pConfigValues->fDDRCommandTimeMin += 0.25f;
	}

	if( m_pConfigValues->fDDRMoveTimeMax >= m_pConfigValues->fDDRCommandTimeMax ) {
		m_pConfigValues->fDDRCommandTimeMax += 0.25f;
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( Frame );

	return FALSE;
}
