//////////////////////////////////////////////////////////////////////////////////////
// SpyVsSpyDDR.cpp - 
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
// 04/21/03 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fsound.h"
#include "fresload.h"

#include "CamBot.h"
#include "Player.h"
#include "entity.h"
#include "espline.h"
#include "gamecam.h"
#include "gamepad.h"
#include "botglitch.h"
#include "botgrunt.h"
#include "CamSimple.h"
#include "MeshEntity.h"
#include "SpyVsSpyDDR.h"
#include "multiplayerMgr.h"
#include "botpart.h"

#include "AI/AIApi.h"
#include "AI/AIBrain.h"
#include "AI/AIMover.h"
#include "AI/AIBrainman.h"
#include "AI/AIGameUtils.h"

#define _BIG_RADIUS ( 1000.0f )

#define _DDR_CSV_NAME				( "spy_ddr.csv" )
#define _DDR_MUSIC					( "Col_4_loop2" )
#define _DDR_MINER_NAME1			( "evan" )
#define _DDR_MINER_NAME2			( "dave" ) // The guy that gets killed
#define _DDR_MINER_NAME3			( "matt" )
#define _DDR_GRUNT_NAME				( "win4" )
#define _DDR_HOLO_MESH				( "gowr_holog" )
#define _DDR_TO_DOORS_SPLINE		( "fromddr" )
#define _DDR_DOOR_POS1				( "droidgo1" )
#define _DDR_DOOR_POS2				( "droidgo2" )
#define _DDR_DOOR_POS3				( "droidgo3" )
#define _DDR_CRANE					( "ddrcrane" )
#define _DDR_CRANE_BONE_NAME		( "cranedummy" )
#define _DDR_BATTLE_START_POINT		( "startglitch" )
#define _DDR_CRANE_GRAB_POINT		( "pickup2" )
#define _DDR_STREAM					( "AS_18i_100$" )
#define _DDR_SPARK_PARTICLES		( "s_spkshower" )
#define _DDR_CRUSHER				( "olcrushy" )
#define _DDR_MAX_FAKE_MOVES			( 4 )
#define _DDR_IDLE_TIME				( 4.0f )
#define _DDR_MUSIC_VOL				( 0.35f )
#define _DDR_ROTATE_AMOUNT			( FMATH_HALF_PI )
#define _DDR_PROBABILITY_MOVE		( 0.5f )
#define _DDR_MAX_MESH_ALPHA			( 0.6f )
#define _DDR_MAX_MESSUPS			( 2 )
#define _DDR_MESSAGE_TIME_SCALE		( 0.70f )
#define _DDR_JUMP_BUMP				( 4.0f )
#define _DDR_DLB_JUMP_BUMP			( 7.0f )
#define _DDR_JUMP_FORGIVE_TIME		( 0.5f )
#define _DDR_CRANE_TIMESCALE		( 0.25f )
#define _DDR_OO_CRANE_TIMESCALE		( 1.0f / _DDR_CRANE_TIMESCALE )
#define _DDR_CRANE_DEATH_UNIT_TIME	( 0.49f )

/*
=============
CSV Commands
=============
	L - Move Left
	R - Move Right
	F - Move Forward
	B - Move Backward
	J - Single Jump
	D - Double Jump

	*R - Rotate Right
	*L - Rotate Left

	? before means optional
*/
static u32 _ClassifyDanceMove( cchar *pszString ) {
	u32 uLen = fclib_strlen( pszString );

	if( pszString == NULL || !uLen || uLen > 3 ) {
		DEVPRINTF( "CDDRStage::_ClassifyDanceMove(): Bad dance move string (%s).\n", pszString );
		return MOVE_TYPE_INVALID;
	}

	BOOL bMayBeRotate = FALSE;

	switch( pszString[0] ) {
		case 'l':
		case 'L':
			return MOVE_TYPE_LEFT;
		break;
		case 'r':
		case 'R':
			return MOVE_TYPE_RIGHT;
		break;
		case 'f':
		case 'F':
			return MOVE_TYPE_FORWARD;
		break;
		case 'b':
		case 'B':
			return MOVE_TYPE_BACKWARD;
		break;
		case 'j':
		case 'J':
			return MOVE_TYPE_JUMP;
		break;
		case 'd':
		case 'D':
			return MOVE_TYPE_DOUBLE_JUMP;
		break;
		case '*':
			bMayBeRotate = TRUE;
		break;
		case '?': {
			u32 uType = _ClassifyDanceMove( pszString + 1 );

			if( uType == MOVE_TYPE_INVALID ) {
				return uType;
			}

			return MOVE_TYPE_COUNT + uType;
		break;
		}
	}

	if( bMayBeRotate ) {
		switch( pszString[1] ) {
			case 'r':
			case 'R':
				return MOVE_TYPE_ROTATE_RIGHT;
			break;
			case 'l':
			case 'L':
				return MOVE_TYPE_ROTATE_LEFT;
			break;
		}
	}

	DEVPRINTF( "CDDRStage::_ClassifyDanceMove(): Bad dance move string (%s).\n", pszString );
	return MOVE_TYPE_INVALID;
}



//
//
//
// CDanceMoveTable
//
//
//
BOOL CDanceMoveTable::Setup( u32 uNumGroups ) {
	FMemFrame_t MemFrame = fmem_GetFrame();

	m_uNumMoveGroups = uNumGroups;
	m_pauNumMovesPerGroup = (u32 *) fres_AllocAndZero( sizeof( u32 ) * uNumGroups );

	if( !m_pauNumMovesPerGroup ) {
		DEVPRINTF( "CDDRStage::_LoadCSV(): Unable to allocate memory for moves per group count.\n" );
		goto _ExitWithError;
	}

	m_paeMoves = (u32 **) fres_AllocAndZero( sizeof( u32 *) * uNumGroups );

	if( !m_paeMoves ) {
		DEVPRINTF( "CDDRStage::_LoadCSV(): Unable to allocate memory for dance move pointers.\n" );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( MemFrame );
	Reset();

	return FALSE;
}

void CDanceMoveTable::Reset( void ) {
	m_uNumMoveGroups = 0;
	m_pauNumMovesPerGroup = NULL;
	m_paeMoves = NULL;
}

//
//
//
// CDanceMoveWalker
//
//
//
void CDanceMoveWalker::Reset( void ) {
	m_uMoveGroup = 0;
	m_uCurrentMove = 0;
	m_uNumFakeMoves = 0;
}


//
//
//
// CDDRStage
//
//
//
CDDRStage::CDDRStage() {

}

BOOL CDDRStage::Load( LevelEvent_e eEvent ) {
	FResFrame_t MemFrame = fres_GetFrame();

	if( eEvent == LEVEL_EVENT_POST_WORLD_LOAD ) {
		_ClearDataMembers();

		if( !_LoadCSV() ) {
			goto _ExitWithError;
		}

		if( !( m_paMiners[ 0 ] = (CBotMiner *) CSpyVsSpy::FindEntity( _DDR_MINER_NAME1, ENTITY_BIT_BOTMINER ) ) ) {
			goto _ExitWithError;
		}

		if( !( m_pKillMiner = (CBotMiner *) CSpyVsSpy::FindEntity( _DDR_MINER_NAME2, ENTITY_BIT_BOTMINER ) ) ) {
			goto _ExitWithError;
		}

		if( !( m_paMiners[ 1 ] = (CBotMiner *) CSpyVsSpy::FindEntity( _DDR_MINER_NAME3, ENTITY_BIT_BOTMINER ) ) ) {
			goto _ExitWithError;
		}

		// These should never fail
		FASSERT( m_paMiners[ 0 ]->AIBrain() );
		FASSERT( m_paMiners[ 1 ]->AIBrain() );
		FASSERT( m_paMiners[ 0 ]->AIBrain()->GetAIMover() );
		FASSERT( m_paMiners[ 1 ]->AIBrain()->GetAIMover() );

		// Do this since we consider ourself at the dest already when we start
		f32 fFloor;

		if( !aiutils_FloorHeightAt( m_paMiners[ 0 ]->MtxToWorld()->m_vPos, &fFloor, 5.0f ) ) {
			DEVPRINTF( "CDDRStage::Load(): aiutils_FloorHeightAt() failed.\n" );
			goto _ExitWithError;
		}

		m_DestPoints[ 0 ] = m_paMiners[ 0 ]->MtxToWorld()->m_vPos;
		m_DestPoints[ 0 ].y = fFloor;
		m_DestPoints[ 1 ] = m_paMiners[ 1 ]->MtxToWorld()->m_vPos;
		m_DestPoints[ 1 ].y = fFloor;
		m_GlitchDestPoint = m_pKillMiner->MtxToWorld()->m_vPos;
		m_GlitchDestPoint.y = fFloor;

		m_StartPoints[ 0 ] = m_DestPoints[ 0 ];
		m_StartPoints[ 1 ] = m_DestPoints[ 1 ];
		m_GlitchStartPoint = m_GlitchDestPoint;

		m_StartMatrix = *m_pKillMiner->MtxToWorld();//*m_paMiners[ 0 ]->MtxToWorld();
		m_DirMatrix = *m_pKillMiner->MtxToWorld();
		m_DirMatrix.m_vPos.Zero();

		_RelocateBots();

		// Load the holo mesh
		FMeshInit_t MeshInit;
		FMesh_t *pMesh;

		pMesh = (FMesh_t *) fresload_Load( FMESH_RESTYPE, _DDR_HOLO_MESH );

		if( pMesh == NULL ) {
			DEVPRINTF( "CDDRStage::Load():  Unable to load mesh %s\n", _DDR_HOLO_MESH );
			goto _ExitWithError;
		}

		MeshInit.fCullDist = 200.0f;
		MeshInit.nFlags = 0;
		MeshInit.pMesh = pMesh;
		MeshInit.Mtx.Identity();

		m_pHoloMesh = fnew CFWorldMesh();

		if( !m_pHoloMesh ) {
			DEVPRINTF( "CDDRStage::Load():  Unable to allocate world mesh.\n" );
			goto _ExitWithError;
		}

		m_pHoloMesh->Init( &MeshInit );
		m_pHoloMesh->RemoveFromWorld();
		m_pHoloMesh->SetCollisionFlag( FALSE );
		m_pHoloMesh->m_nUser = NULL;
		m_pHoloMesh->m_pUser = NULL;
		m_pHoloMesh->SetUserTypeBits( 0 );
		m_pHoloMesh->UpdateTracker();
		m_pHoloMesh->SetCullDirection( FMESH_CULLDIR_NONE );
#if FANG_WINGC
		if( MultiplayerMgr.IsLocalCoop() ) {
			// Players are initialized after this resource load; player count is
			// still zero here. Reserve all partner guides, show only active slots.
			for( s32 n = 0; n < 3; ++n ) {
				m_apCoopHolo[n] = fnew CFWorldMesh();
				if( !m_apCoopHolo[n] ) goto _ExitWithError;
				m_apCoopHolo[n]->Init( &MeshInit );
				m_apCoopHolo[n]->RemoveFromWorld();
				m_apCoopHolo[n]->SetCollisionFlag( FALSE );
				m_apCoopHolo[n]->SetUserTypeBits( 0 );
				m_apCoopHolo[n]->SetCullDirection( FMESH_CULLDIR_NONE );
			}
		}
#endif

		// Find the command grunt
		m_pCommandGrunt = (CBotGrunt * ) CSpyVsSpy::FindEntity( _DDR_GRUNT_NAME, ENTITY_BIT_BOTGRUNT );

		if( !m_pCommandGrunt ) {
			goto _ExitWithError;
		}

		f32 fY = 0;

		if( !aiutils_FloorHeightAt( m_pCommandGrunt->MtxToWorld()->m_vPos, &fY, 20.0f ) ) {
			goto _ExitWithError;
		}

		m_CommandGruntStart = m_pCommandGrunt->MtxToWorld()->m_vPos;
		m_CommandGruntStart.y = fY + 1.0f;

		CEntity *pEntity;

		// Find the door points
		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_DOOR_POS1 ) ) ) {
			goto _ExitWithError;
		}
		m_DoorDest1 = pEntity->MtxToWorld()->m_vPos;

		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_DOOR_POS2 ) ) ) {
			goto _ExitWithError;
		}
		m_DoorDest2 = pEntity->MtxToWorld()->m_vPos;

		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_DOOR_POS3 ) ) ) {
			goto _ExitWithError;
		}
		m_DoorDest3 = pEntity->MtxToWorld()->m_vPos;

		m_DoorDest1.Add( CFVec3A::m_UnitAxisY );
		m_DoorDest2.Add( CFVec3A::m_UnitAxisY );
		m_DoorDest3.Add( CFVec3A::m_UnitAxisY );

		// Find the running spline
		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_TO_DOORS_SPLINE, ENTITY_BIT_SPLINE ) ) ) {
			goto _ExitWithError;
		}

		m_pRunSpline = (CESpline *) pEntity;

		// Find the crane
		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_CRANE, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}

		m_pDDRCrane = (CMeshEntity *) pEntity;

		if( m_pDDRCrane->UserAnim_GetCount() != 2 ) {
			DEVPRINTF( "CDDRStage::Load(): Crane does not have two animations.\n" );
			goto _ExitWithError;
		}

		s32 sBoneIndex = m_pDDRCrane->GetMeshInst()->FindBone( _DDR_CRANE_BONE_NAME );

		if( sBoneIndex == -1 ) {
			DEVPRINTF( "CDDRStage::Load(): Unable to find bone '%s' in '%s'.\n", _DDR_CRANE_BONE_NAME, _DDR_CRANE );
			goto _ExitWithError;
		}

		m_pCraneBone = &m_pDDRCrane->GetMeshInst()->GetBoneMtxPalette()[ sBoneIndex ]->m_vPos;

		// Find the drop point for Glitch
		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_BATTLE_START_POINT, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}

		m_BattleStartPoint = pEntity->MtxToWorld()->m_vPos;

		// Put things in a state so we don't get our grunt killed
		// Make our AI guys stupid
		CSpyVsSpy::MakeBotDumb( m_pKillMiner, 0.45f );
		CSpyVsSpy::MakeBotDumb( m_paMiners[ 0 ], 0.45f );
		CSpyVsSpy::MakeBotDumb( m_paMiners[ 1 ], 0.45f );
		CSpyVsSpy::MakeBotDumb( m_pCommandGrunt, 1.0f );

		_RelocateBots();

		m_pCommandGrunt->SetInvincible( TRUE );
		m_pCommandGrunt->SetTargetable( FALSE );

		m_paMiners[ 0 ]->SetInvincible( TRUE );
		m_paMiners[ 1 ]->SetInvincible( TRUE );

		// Find the crane grab point
		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_CRANE_GRAB_POINT, ENTITY_BIT_SPHERE ) ) ) {
			goto _ExitWithError;
		}

		m_CraneGrabPoint = pEntity->MtxToWorld()->m_vPos;
		aiutils_FloorHeightAt( m_CraneGrabPoint, &m_CraneGrabPoint.y, 20.0f );
	
		m_pTVTexture = ftex_CreateRenderTarget( 128, 128, FTEX_RENDERTARGET_FMT_C24_A8_D24_S8, "TVs", TRUE, 0, NULL, FTEX_2D, TRUE );
	
		if ( m_pTVTexture ) {
			ftex_AddRenderTarget( m_pTVTexture, _TVCallback, TRUE, 30, FALSE, FALSE, this, TRUE, FALSE);
			ftex_ActivateRenderTarget( m_pTVTexture, FALSE );
		}

		char szName[5];

		for( u32 i = 0; i < NUM_TVS; ++i ) {
			sprintf( szName, "tv0%d", i + 1 );

			CMeshEntity *pTV = (CMeshEntity *)CEntity::FindInWorld( szName );

			if( pTV ) {
				FMeshTexLayerHandle_t hHandle = pTV->GetMeshInst()->GetTexLayerHandle( 0 );

				if( m_pTVTexture ) {
					pTV->GetMeshInst()->TexFlip_SetSingleTexture( hHandle, m_pTVTexture );
				}
			}
		}

		// Load the spark particles
		m_hSparkParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _DDR_SPARK_PARTICLES );

		if( m_hSparkParticleDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CDDRStage::Load(): Failed to load particle '%s'.\n", _DDR_SPARK_PARTICLES );
			goto _ExitWithError;
		}

		// Find ze-crusher
		if( !( pEntity = CSpyVsSpy::FindEntity( _DDR_CRUSHER, ENTITY_BIT_MESHENTITY ) ) ) {
			goto _ExitWithError;
		}

		m_pCrusher = (CMeshEntity *) pEntity;

	} else if( eEvent == LEVEL_EVENT_POST_ENTITY_FIXUP ) {
		CFVec3A DestPos = CFVec3A::m_UnitAxisZ;

		DestPos.Mul( -20.0f );
		DestPos.Add( m_CommandGruntStart );

		ai_AssignGoal_GotoWithLookAt( m_pCommandGrunt->AIBrain(), DestPos, CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 50, 1, 100 );
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( MemFrame );
	Unload();
	return FALSE;
}

void CDDRStage::Unload( void ) {
#if FANG_WINGC
	for( s32 n = 0; n < 3; ++n ) {
		if( m_apCoopHolo[n] ) { m_apCoopHolo[n]->RemoveFromWorld(); fdelete(m_apCoopHolo[n]); m_apCoopHolo[n] = NULL; }
	}
#endif
	if( m_pCrusherEmitter ) {
		m_pCrusherEmitter->Destroy();
		m_pCrusherEmitter = NULL;
	}

	_ClearDataMembers();
}

void CDDRStage::Restore( void ) {
	SwitchTo();

	CFMtx43A::m_Temp = m_DirMatrix;
	CFMtx43A::m_Temp.m_vPos = m_GlitchStartPoint;

	CSpyVsSpy::GetGlitch()->Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp );
}

void CDDRStage::Work( void ) {
#if FANG_WINGC
	BOOL bCoopCheckedThisFrame = FALSE;
	if( _CoopActive() && !IsFinished() ) {
		for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
			CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
			if( pBot && pBot->IsDeadOrDying() ) m_bCoopRetryPending = TRUE;
		}
		if( m_bCoopRetryPending ) {
			for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
				CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
				if( pBot && pBot->IsDeadOrDying() && !pBot->IsDead() ) return;
			}
			for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
				CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
				if( !pBot ) continue;
				pBot->DetachFromParent(); pBot->MobilizeBot();
				Player_aPlayer[n].Resurrect();
				if( pBot->GetPartMgr() && pBot->GetPartMgr()->IsCreated() ) pBot->GetPartMgr()->ResetAllToIntact();
				Player_aPlayer[n].CoopClearRespawnWait();
				Player_aPlayer[n].ZeroControls();
			}
			SwitchTo();
			m_eStageState = STATE_WAIT_WORK; // Retry the game without replaying Shhh's long briefing.
			DEVPRINTF( "Co-op: factory instructor retries with the whole team.\n" );
			return;
		}
	}
#endif
	if( IsFinished() ) {
		return;
	}

#if FANG_WINGC
	if( _CoopActive() ) CSpyVsSpy::TurnOffHUD( TRUE ); else
#endif
	if( CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->IsDrawEnabled() )
		CHud2::GetHudForPlayer( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex )->SetDrawEnabled( FALSE );
#if FANG_WINGC
	if( _CoopActive() ) for( s32 n = 1; n < CPlayer::m_nPlayerCount; ++n ) {
		CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
		if( pBot ) pBot->m_vecCamPosAdj = CSpyVsSpy::GetGlitch()->m_vecCamPosAdj;
	}
#endif

	if( !_IsInSpecialMode() ) {
		if( !( m_uFlags & FLAG_STOP_CAM_ANIM ) ) {
			CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.z -= ( FLoop_fPreviousLoopSecs * 5.0f );
			CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.y += ( FLoop_fPreviousLoopSecs * 5.0f );
			FMATH_CLAMPMIN( CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.z, CSpyVsSpy::m_pConfigValues->fDDRLagDistance );
			FMATH_CLAMPMAX( CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.y, CSpyVsSpy::m_pConfigValues->fDDRBumpUpDistance );

			if( ( CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.z == CSpyVsSpy::m_pConfigValues->fDDRLagDistance ) &&
				( CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.y == CSpyVsSpy::m_pConfigValues->fDDRBumpUpDistance ) ) {
				FMATH_SETBITMASK( m_uFlags, FLAG_STOP_CAM_ANIM );
			}
		}
	}

	switch( m_eStageState ) {
		case STATE_WAIT_DIALOG:
			level_StopAllStreams();
			CSpyVsSpy::StreamStart( _DDR_STREAM, TRUE, TRUE );
			m_eStageState = STATE_WAIT_WORK;

			return;
		break;
		case STATE_WAIT_WORK: {
			CSpyVsSpy::TakeControlFromPlayer( TRUE );
			CSpyVsSpy::TurnOffHUD( TRUE );
			// Fire off the intro sound clip
			CSpyVsSpy::PlayBotTalk( m_pCommandGrunt, CSpyVsSpy::BTR_TEST );
			
			level_PlayMusic( _DDR_MUSIC, _DDR_MUSIC_VOL, TRUE );

			m_eStageState = STATE_WALK_GLITCH_BACK;

			ai_AssignGoal_GotoWithLookAt( m_pCommandGrunt->AIBrain(), m_CommandGruntStart, CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 50, 1, 100 );

			return;
		}
		break;
		case STATE_GIVE_COMMAND:
			_IssueCommand();
		break;
		case STATE_EXECUTE_COMMAND:
			_ExecuteCommand();
		break;
		case STATE_WAIT_FINISH_COMMAND:
#if FANG_WINGC
			// Consume this frame's input before judging the deadline, once only.
			if( _CoopActive() ) {
				_CoopCheckPlayers();
				bCoopCheckedThisFrame = TRUE;
				if( m_eStageState != STATE_WAIT_FINISH_COMMAND || m_bCoopRetryPending ) break;
			}
#endif
			_CheckBotsDone();
		break;
		case STATE_WALK_GLITCH_BACK:
			_CheckBotsDone();
		break;
		case STATE_IDLE_TIME:
			m_fIdleTimer -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fIdleTimer, 0.0f );

			if( m_fIdleTimer < 0.5f && ( m_uFlags & FLAG_DELAY_MESH ) ) {
				FMATH_CLEARBITMASK( m_uFlags, FLAG_DELAY_MESH );
				m_pHoloMesh->AddToWorld();
				m_pHoloMesh->UpdateTracker();
			}

			if( m_fIdleTimer == 0.0f ) {
				if( ( m_uFlags & FLAG_WAIT_EXECUTE ) ) {
					m_eStageState = STATE_EXECUTE_COMMAND;
				} else {
					m_eStageState = STATE_GIVE_COMMAND;
				}
			}
		break;
		case STATE_CHECK_GLITCH:
			_CheckGlitchWork();
		break;
		case STATE_RUN_TO_DOORS:
			_RunToDoorsWork();
		break;
		case STATE_GRAB_GLITCH:
			_CraneWork();
		break;
		case STATE_GLITCH_GETS_KILLED:
			m_fIdleTimer -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fIdleTimer, 0.0f );

			_CheckBotsDone();

			if( m_fIdleTimer == 0.0f ) {
				_SetStateGrabGlitch();
			}
		break;
		case STATE_DDR_DONE:

		break;
	}

	m_paMiners[ 0 ]->OverrideHipSlackThisFrame();
	m_paMiners[ 1 ]->OverrideHipSlackThisFrame();

	if( _IsInSpecialMode() ) {
		m_pKillMiner->OverrideHipSlackThisFrame();
	}

	switch( m_eStageState ) {
		case STATE_WALK_GLITCH_BACK:
		case STATE_CHECK_GLITCH:
		case STATE_GRAB_GLITCH:
		case STATE_RUN_TO_DOORS:

		break;
		default:
			if( !_IsInSpecialMode() ) {
#if FANG_WINGC
				if( _CoopActive() ) { if( !bCoopCheckedThisFrame ) _CoopCheckPlayers(); } else
#endif
				_CheckAlignment( CSpyVsSpy::GetGlitch() );
			} else {
				_CheckAlignment( m_pKillMiner );
			}
		break;
	}

	_MeshAlphaWork();
#if FANG_WINGC
	_CoopHologramsWork();
#endif
}

void CDDRStage::SwitchTo( void ) {
	// !!Nate
	// Temp till we get a spawn point
//	CFVec3A Temp = CFVec3A::m_UnitAxisY;
//	Temp.Add( m_pKillMiner->MtxToWorld()->m_vPos );
//	CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &Temp );

//	m_uFlags = 0;
	FMATH_CLEARBITMASK( m_uFlags, FLAG_ALL_BITS );

	// Make our AI guys stupid
	CSpyVsSpy::MakeBotDumb( m_pKillMiner, 0.45f );
	CSpyVsSpy::MakeBotDumb( m_paMiners[ 0 ], 0.45f );
	CSpyVsSpy::MakeBotDumb( m_paMiners[ 1 ], 0.45f );
	CSpyVsSpy::MakeBotDumb( m_pCommandGrunt, 1.0f );

	m_fTimeInterp = 0.0f;
	m_fIdleTimer = 0.0f;
	m_fMoveTimeIdle = CSpyVsSpy::GetIdleTime( m_fTimeInterp );
	m_fGlitchMoveTimer = 0.0f;
	m_uBadMoveCount = 0;

	CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_TESTING_START );
	f32 fIdle = fsndfx_GetDuration( CFSoundGroup::GetSoundHandle( pGroup, 0 ) );

	// Don't play the sound here since we have the streaming audio kicking in
//	CFSoundGroup::PlaySound( pGroup, TRUE );

	if( _IsInSpecialMode() ) {
		m_eStageState = STATE_IDLE_TIME;
		m_fIdleTimer = 3.0f;
	} else {
		m_eStageState = STATE_WAIT_DIALOG;
		m_fIdleTimer = fIdle;// + 3.0f; // Start with a long idle time
		m_pKillMiner->Die();
	}
	
	m_DirMatrix = m_StartMatrix;//*m_paMiners[ 0 ]->MtxToWorld();

	m_DestPoints[ 0 ] = m_StartPoints[ 0 ];
	m_DestPoints[ 1 ] = m_StartPoints[ 1 ];
	m_GlitchDestPoint = m_GlitchStartPoint;

	_InitWalker();
	_RecalcLookPoint();
	_RelocateBots();

	ai_AssignGoal_FaceIt( m_pCommandGrunt->AIBrain(), CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 50, 0, -1 );

	m_pCommandGrunt->SetInvincible( TRUE );
	m_pCommandGrunt->SetTargetable( FALSE );

	m_paMiners[ 0 ]->SetInvincible( TRUE );
	m_paMiners[ 1 ]->SetInvincible( TRUE );

	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( 0.8f );

	m_uGruntSplinePoint = 0;

	m_pDDRCrane->SetCollisionFlag( FALSE );
	m_pDDRCrane->UserAnim_SetSpeedMult( 0.25f );
	m_pDDRCrane->UserAnim_Pause( TRUE );
	m_pDDRCrane->UserAnim_SetClampMode( TRUE );
	m_pDDRCrane->UserAnim_Select( 1 );
	m_pDDRCrane->UserAnim_UpdateUnitTime( 0.0f );

	// !!Nate - make sure to turn this back on after the stage is over
	CSpyVsSpy::GetGlitch()->IgnoreBotVBotCollision();
	CSpyVsSpy::GetGlitch()->SetBotFlag_DontUseIdleAnimations();
	CSpyVsSpy::TurnOffHUD( TRUE );
	CSpyVsSpy::AttackDisable( TRUE );

	if( !_IsInSpecialMode() ) {
		// Don't do this here, we want to skip the cutscene
		//CSpyVsSpy::TakeControlFromPlayer( TRUE );

		GameCamType_e nCamType;
		CCamBot *pBotCam;

		gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), CSpyVsSpy::GetGlitch() );
		pBotCam = (CCamBot *) gamecam_GetCameraManByIndex( GAME_CAM_PLAYER_1, &nCamType );

		if( nCamType == GAME_CAM_TYPE_ROBOT_3RD ) {
			CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.Zero();
			CSpyVsSpy::GetGlitch()->m_vecCamLookAdj.Zero();

			pBotCam->SetAlwaysOverride( TRUE );
		}

		// Make the command grunt stand off to the side a bit
		CFVec3A DestPos = CFVec3A::m_UnitAxisZ;

		DestPos.Mul( -20.0f );
		DestPos.Add( m_CommandGruntStart );
	
		ai_AssignGoal_GotoWithLookAt( m_pCommandGrunt->AIBrain(), DestPos, CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 50, 1, 100 );
		FMATH_CLEARBITMASK( m_uFlags, FLAG_SPECIAL_BITS );
	}

	m_GotoPoint.Zero();

	ftex_ActivateRenderTarget( m_pTVTexture, TRUE );

	m_pCrusher->UserAnim_UpdateTime( 0.0f );
	m_pCrusher->UserAnim_Pause( TRUE );
	
	if( m_pCrusherEmitter ) {
		m_pCrusherEmitter->Destroy();
		m_pCrusherEmitter = NULL;
	}

#if FANG_WINGC
	if( _CoopActive() ) {
		m_bCoopRetryPending = FALSE;
		_CoopPlacePlayers();
		CSpyVsSpy::CoopSyncStage( "Empty Primary", "Empty Secondary", FALSE );
		_CoopCommandState( FALSE );
	}
#endif
	_SetFinished( FALSE );
}

void CDDRStage::TossSparksFromBot( CBot *pBot ) {
	CFVec3A Temp;
	CFVec3A Pos;

	Pos = CFVec3A::m_UnitAxisY;
	Pos.Mul( pBot->m_fCollCylinderHeight_WS * 0.5f );
	Pos.Add( pBot->MtxToWorld()->m_vPos );

	for( u32 i = 0; i < 4; ++i ) {
		Temp = Pos;
		Temp.x += fmath_RandomBipolarUnitFloat() * 2.0f;
		Temp.y += fmath_RandomBipolarUnitFloat() * 2.0f;
		Temp.z += fmath_RandomBipolarUnitFloat() * 2.0f;
		fparticle_SpawnEmitter( m_hSparkParticleDef, Temp.v3, NULL, 1.0f );
	}
}

void CDDRStage::_ClearDataMembers( void ) {
#if FANG_WINGC
	m_bCoopRetryPending = FALSE;
	for( s32 n = 0; n < 4; ++n ) { m_aCoopDance[n].offset.Zero(); m_aCoopDance[n].flags = 0; m_aCoopDance[n].align = m_aCoopDance[n].position = m_aCoopDance[n].look = m_aCoopDance[n].move = 0; }
	for( s32 n = 0; n < 3; ++n ) m_apCoopHolo[n] = NULL;
#endif
	m_uFlags = 0;

	m_DanceTable.Reset();
	m_DanceMoveWalker.Reset();

	m_DirMatrix.Identity();
	m_LookPoint.Zero();

	m_pKillMiner = NULL;
	m_paMiners[ 0 ] = NULL;
	m_paMiners[ 1 ] = NULL;
	m_DestPoints[ 0 ].Zero();
	m_DestPoints[ 1 ].Zero();

	m_eStageState = STATE_GIVE_COMMAND;

	m_fIdleTimer = 0.0f;
	m_fTimeInterp = 0.0f;
	m_fMoveTimeIdle = CSpyVsSpy::GetIdleTime( m_fTimeInterp );

	m_fGlitchMoveTimer = 0.0f;
	m_GlitchStartPoint.Zero();
	m_GlitchDestPoint.Zero();
	m_StartMatrix.Identity();

	m_fHoloMeshAlpha = 0.0f;
	m_fHoloMeshAlphaFadeDir = 0.0f;
	m_pHoloMesh = NULL;

	m_pCommandGrunt = NULL;
	m_uBadMoveCount = 0;
	m_CommandGruntStart.Zero();

	m_DoorDest1.Zero();
	m_DoorDest2.Zero();
	m_DoorDest3.Zero();

	m_uGruntSplinePoint = 0;
	m_pRunSpline = NULL;

	m_pDDRCrane = NULL;
	m_pCraneBone = NULL;

	m_fUnitCamInterp = 0.0f;
	m_StartQuat.Identity();

	m_BattleStartPoint.Zero();
	m_CraneGrabPoint.Zero();

	m_fAlignFailTimer = 0.0f;
	m_fPositionFailTimer = 0.0f;
	m_fLookFailTimer = 0.0f;

	m_GotoPoint.Zero();

	m_pTVTexture = NULL;
	m_pViewport = NULL;

	m_pCrusher = NULL;
	m_pCrusherEmitter = NULL;
}

// Puts the left and right bots _DDR_MOVE_DISTANCE from the center bot
void CDDRStage::_RelocateBots( void ) {
	CFVec3A StartPoint = m_GlitchStartPoint;//m_pKillMiner->MtxToWorld()->m_vPos;
	CFVec3A Right = m_StartMatrix.m_vRight;//m_pKillMiner->MtxToWorld()->m_vRight;
	CFVec3A Dest;

	Dest.Mul( Right, -CSpyVsSpy::m_pConfigValues->fDDRMoveDistance ).Add( StartPoint );
	m_paMiners[ 0 ]->Relocate_Xlat_WS( &Dest );

	Dest.Mul( Right, CSpyVsSpy::m_pConfigValues->fDDRMoveDistance ).Add( StartPoint );
	m_paMiners[ 1 ]->Relocate_Xlat_WS( &Dest );

	if( _IsInSpecialMode() ) {
		m_pKillMiner->Relocate_Xlat_WS( &StartPoint );
	}
}

void CDDRStage::_InitWalker( void ) {
	m_DanceMoveWalker.m_uCurrentMove = 0;

	if( !_IsInSpecialMode() ) {
		m_DanceMoveWalker.m_uMoveGroup = (u32) fmath_RandomRange( 0, m_DanceTable.m_uNumMoveGroups - 2 ); // -2 since we always ignore the last dance move set
	} else {
		// If in special mode we ALWAYS play the last dance in the list
		m_DanceMoveWalker.m_uMoveGroup = m_DanceTable.m_uNumMoveGroups - 1;
	}
	m_DanceMoveWalker.m_uNumFakeMoves = 0;
	m_fTimeInterp = 0.0f;
}

void CDDRStage::_IssueCommand( void ) {
	if( m_DanceMoveWalker.m_uCurrentMove == m_DanceTable.GetNumMoves( m_DanceMoveWalker.m_uMoveGroup ) ) {
		// We are done
		if( !_IsInSpecialMode() ) {
			_RunToDoors();
		} 

		return;
	}
	
	if( _IsInSpecialMode() ) {
		if( ( m_DanceTable.GetNumMoves( m_DanceMoveWalker.m_uMoveGroup ) - m_DanceMoveWalker.m_uCurrentMove ) <= 1 ) {
			FMATH_SETBITMASK( m_uFlags, FLAG_SPECIAL_FAIL_COMMANDS );
		}
	}

	u32 uType = m_DanceTable.GetDanceMove( m_DanceMoveWalker.m_uMoveGroup, m_DanceMoveWalker.m_uCurrentMove );
	BOOL bFake = FALSE;

	FASSERT( uType != MOVE_TYPE_INVALID );

	if( m_DanceTable.GetNumMoves( m_DanceMoveWalker.m_uMoveGroup ) > 1 ) {
		s32 sMaxMoves = m_DanceTable.GetNumMoves( m_DanceMoveWalker.m_uMoveGroup ) - 1 - CSpyVsSpy::m_pConfigValues->uDDRMoveTimeMin;

		FMATH_CLAMPMIN( sMaxMoves, 1 );

		m_fTimeInterp = (f32) ( m_DanceMoveWalker.m_uCurrentMove * fmath_Inv( (f32) sMaxMoves ) ); 
		FMATH_CLAMP_UNIT_FLOAT( m_fTimeInterp );
	} else {
		m_fTimeInterp = 1.0f;
	}

	FMATH_CLEARBITMASK( m_uFlags, ( FLAG_FAKE_COMMAND | FLAG_JUMP_OK ) );

	if( uType >= MOVE_TYPE_COUNT ) {
		uType -= MOVE_TYPE_COUNT;

		// We have an optional move
//		if( !fmath_RandomChance( _DDR_PROBABILITY_MOVE ) && ( m_DanceMoveWalker.m_uNumFakeMoves < _DDR_MAX_FAKE_MOVES ) ) {
			// Let's play a fake
			FMATH_SETBITMASK( m_uFlags, FLAG_FAKE_COMMAND );
			FMATH_SETBITMASK( m_uFlags, FLAG_WAIT_EXECUTE );
			_FakeMove( uType, TRUE );
#if FANG_WINGC
			_CoopCommandState( FALSE );
#endif

			return;
//		}
	}

	FMATH_SETBITMASK( m_uFlags, FLAG_WAIT_EXECUTE ); // We are waiting to execute this command

	// Play the sound
	u32 uOffset = uType - MOVE_TYPE_LEFT;
	u32 uNdx;

	switch( uType ) {
		case MOVE_TYPE_LEFT:
			uNdx = CSpyVsSpy::BTR_CLEFT;
		break;
		case MOVE_TYPE_RIGHT:
			uNdx = CSpyVsSpy::BTR_CRIGHT;
		break;
		case MOVE_TYPE_FORWARD:
			uNdx = CSpyVsSpy::BTR_CFORWARD;
		break;
		case MOVE_TYPE_BACKWARD:
			uNdx = CSpyVsSpy::BTR_CBACK;
		break;
		case MOVE_TYPE_ROTATE_LEFT:
			uNdx = CSpyVsSpy::BTR_CROTL;
		break;
		case MOVE_TYPE_ROTATE_RIGHT:
			uNdx = CSpyVsSpy::BTR_CROTR;
		break;
		case MOVE_TYPE_JUMP:
			uNdx = CSpyVsSpy::BTR_CJUMP;
			FMATH_SETBITMASK( m_uFlags, FLAG_JUMP1_IN_QUEUE );
		break;
		case MOVE_TYPE_DOUBLE_JUMP:
			uNdx = CSpyVsSpy::BTR_CDJUMP;
			FMATH_SETBITMASK( m_uFlags, FLAG_JUMP2_IN_QUEUE );
		break;
	}

	m_eStageState = STATE_IDLE_TIME;
	CSpyVsSpy::PlayBotTalk( m_pCommandGrunt, uNdx, &m_fIdleTimer );
#if FANG_WINGC
	_CoopCommandState( FALSE );
#endif
}

// We never get here with a fake command, if we do, bad
void CDDRStage::_ExecuteCommand( void ) {
	u32 uType = m_DanceTable.GetDanceMove( m_DanceMoveWalker.m_uMoveGroup, m_DanceMoveWalker.m_uCurrentMove );
	
	FASSERT( uType != MOVE_TYPE_INVALID );

	FMATH_CLEARBITMASK( m_uFlags, ( FLAG_HAVE_JUMP | FLAG_JUMP1 | FLAG_JUMP2 | FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE ) );

	if( !( m_uFlags & FLAG_FAKE_COMMAND ) ) {
		BOOL bJump = FALSE;
		BOOL bDbl = FALSE;

		if( uType >= MOVE_TYPE_COUNT ) {
			uType -= MOVE_TYPE_COUNT;
		}

		switch( uType ) {
			case MOVE_TYPE_LEFT:
			case MOVE_TYPE_RIGHT:
			case MOVE_TYPE_FORWARD:
			case MOVE_TYPE_BACKWARD:
				_TellAIToMove( uType );
			break;
			case MOVE_TYPE_ROTATE_RIGHT:
			case MOVE_TYPE_ROTATE_LEFT:
				_RotateDirections( uType );
			break;
			case MOVE_TYPE_JUMP:
			case MOVE_TYPE_DOUBLE_JUMP:
				if( uType == MOVE_TYPE_JUMP ) {
					bJump = TRUE;
				} else {
					bDbl = TRUE;
				}

				_IssueJump( uType );
			break;
			default:
				FASSERT_NOW;
			break;
		}

		++m_DanceMoveWalker.m_uCurrentMove;
		_MeshPlaceIntoWorld( NULL, bJump, bDbl );
	} else {
		// We got a fake one
		_FakeMove( ( uType - MOVE_TYPE_COUNT ) );
	}

	FMATH_CLEARBITMASK( m_uFlags, FLAG_WAIT_EXECUTE );

	m_eStageState = STATE_WAIT_FINISH_COMMAND;
	m_fIdleTimer = CSpyVsSpy::GetCommandTime( m_fTimeInterp );
#if FANG_WINGC
	// The response window must include the same extra movement grace used by
	// the validator, followed by the retail settling interval.
	if( _CoopActive() ) m_fIdleTimer += 0.6f;
#endif
	m_fAlignFailTimer = 0.0f;
	m_fPositionFailTimer = 0.0f;
	m_fLookFailTimer = 0.0f;
#if FANG_WINGC
	_CoopCommandState( TRUE );
#endif
}

void CDDRStage::_TellAIToMove( u32 uType, CFMtx43A *pMtx/* = NULL*/ ) {
	CFVec3A MoveDir;

	switch( uType ) {
		case MOVE_TYPE_LEFT:
			MoveDir = m_DirMatrix.m_vRight;
			MoveDir.Mul( -1.0f );
		break;
		case MOVE_TYPE_RIGHT:
			MoveDir = m_DirMatrix.m_vRight;
		break;
		case MOVE_TYPE_FORWARD:
			MoveDir = m_DirMatrix.m_vFront;
		break;
		case MOVE_TYPE_BACKWARD:
			MoveDir = m_DirMatrix.m_vFront;
			MoveDir.Mul( -1.0f );
		break;
		default:
			FASSERT_NOW;
		break;
	}

	MoveDir.Mul( CSpyVsSpy::m_pConfigValues->fDDRMoveDistance );

	if( !pMtx ) {
		for( u32 i = 0; i < NUM_MINERS; ++i ) {
			m_DestPoints[ i ].Add( MoveDir );
		}

		// Setup where Glitch will need to go
		m_GlitchDestPoint.Add( MoveDir );
		m_fGlitchMoveTimer = 0.0f;

		FMATH_SETBITMASK( m_uFlags, FLAG_MOVE_COMMAND );
	} else {
		pMtx->Identity();
		*pMtx = m_DirMatrix;
		pMtx->m_vPos = m_GlitchDestPoint;
		pMtx->m_vPos.Add( MoveDir );
	}
}

void CDDRStage::_RotateDirections( u32 uType, CFMtx43A *pMtx/* = NULL*/ ) {
	CFMtx43A RotMtx;

	RotMtx.Identity();

	if( uType == MOVE_TYPE_ROTATE_RIGHT ) {
		RotMtx.RotateY( FMATH_HALF_PI );
	} else {
		RotMtx.RotateY( -FMATH_HALF_PI );
	}

	if( !pMtx ) {
		m_DirMatrix.Mul( RotMtx );

		_RecalcLookPoint();

		//m_GlitchMoveStartPoint = m_GlitchDestPoint;
		m_fGlitchMoveTimer = 0.0f;

		FMATH_CLEARBITMASK( m_uFlags, FLAG_MOVE_COMMAND );
	} else {
		*pMtx = m_DirMatrix;
		pMtx->Mul( RotMtx );
		pMtx->m_vPos = m_GlitchDestPoint;
	}
}

void CDDRStage::_IssueJump( u32 uType ) {
	FMATH_SETBITMASK( m_uFlags, FLAG_HAVE_JUMP );

	for( u32 i = 0; i < NUM_MINERS; ++i ) {
		CAIBrain *pBrain = m_paMiners[ i ]->AIBrain();
		
		if( uType == MOVE_TYPE_JUMP ) {
			FMATH_SETBITMASK( m_uFlags, FLAG_JUMP1 );
			pBrain->GetAIMover()->m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_FORCE_JUMP1;
		} else {
			FMATH_SETBITMASK( m_uFlags, FLAG_JUMP2 );
			pBrain->GetAIMover()->m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_FORCE_JUMP2;
		}
	}

	if( _IsInSpecialMode() ) {
		u32 uFlag = CBotControl::FLAG_FORCE_JUMP1;

		if( uType != MOVE_TYPE_JUMP ) {
			uFlag = CBotControl::FLAG_FORCE_JUMP2;
		}
		
		m_pKillMiner->AIBrain()->GetAIMover()->m_Controls.GetEntityControl()->m_nFlags |= uFlag;
	}

	m_fGlitchMoveTimer = 0.0f;
}

#define _DDR_MAX_MISS_DIST_SQXZ ( 6.0f )
//#define _DDR_MAX_OUT_OF_ALIGN_TIME ( 0.5f )
#define _DDR_LOOK_DOT_MIN ( 0.79f )

#define _DDR_MIN_HORZ_DOT ( 0.96f )
#define _DDR_MIN_HORZ_DOT_SECONDARY ( 0.90f )

void CDDRStage::_FakeMove( u32 uFake, BOOL bAudioOnly /*= FALSE*/ ) {
	if( !bAudioOnly ) {
		++m_DanceMoveWalker.m_uNumFakeMoves;
		++m_DanceMoveWalker.m_uCurrentMove;
	} else {
		u32 uOffset = uFake - MOVE_TYPE_LEFT;
		u32 uNdx;

		switch( uFake ) {
			case MOVE_TYPE_LEFT:
				uNdx = CSpyVsSpy::BTR_LEFT;
			break;
			case MOVE_TYPE_RIGHT:
				uNdx = CSpyVsSpy::BTR_RIGHT;
			break;
			case MOVE_TYPE_FORWARD:
				uNdx = CSpyVsSpy::BTR_FORWARD;
			break;
			case MOVE_TYPE_BACKWARD:
				uNdx = CSpyVsSpy::BTR_BACK;
			break;
			case MOVE_TYPE_ROTATE_LEFT:
				uNdx = CSpyVsSpy::BTR_ROTL;
			break;
			case MOVE_TYPE_ROTATE_RIGHT:
				uNdx = CSpyVsSpy::BTR_ROTR;
			break;
			case MOVE_TYPE_JUMP:
				uNdx = CSpyVsSpy::BTR_JUMP;
			break;
			case MOVE_TYPE_DOUBLE_JUMP:
				uNdx = CSpyVsSpy::BTR_DJUMP;
			break;
		}

		m_eStageState = STATE_IDLE_TIME;
		CSpyVsSpy::PlayBotTalk( m_pCommandGrunt, uNdx, &m_fIdleTimer );

		BOOL bJump = FALSE;
		BOOL bDbl = FALSE;

		switch( uFake ) {
			case MOVE_TYPE_LEFT:
			case MOVE_TYPE_RIGHT:
			case MOVE_TYPE_FORWARD:
			case MOVE_TYPE_BACKWARD:
				_TellAIToMove( uFake, &CFMtx43A::m_Temp );
			break;
			case MOVE_TYPE_ROTATE_RIGHT:
			case MOVE_TYPE_ROTATE_LEFT:
				_RotateDirections( uFake, &CFMtx43A::m_Temp );
			break;
			case MOVE_TYPE_JUMP:
			case MOVE_TYPE_DOUBLE_JUMP:
				if( uFake == MOVE_TYPE_JUMP ) {
					bJump = TRUE;
				} else {
					bDbl = TRUE;
				}
				CFMtx43A::m_Temp = m_DirMatrix;
				CFMtx43A::m_Temp.m_vPos = m_GlitchDestPoint;
			break;
		}

		FMATH_SETBITMASK( m_uFlags, FLAG_DELAY_MESH );
		_MeshPlaceIntoWorld( &CFMtx43A::m_Temp, bJump, bDbl, TRUE );
	}
}

#define _DDR_GOOD_ENOUGH_DIST_SQXZ ( 2.0f * 2.0f )

void CDDRStage::_CheckBotsDone( void ) {
	u32 uNumDone = 0;

	for( u32 i = 0; i < NUM_MINERS; ++i ) {
		m_paMiners[ i ]->AIBrain()->GetAIMover()->BeginFrame();

		if( m_paMiners[ i ]->AIBrain()->GetAIMover()->MoveToward( m_DestPoints[ i ] ) ) {
			++uNumDone;
		} else {
			// !!Nate - makes them not run around a lot, but they don't stay in a good line
			if( m_paMiners[ i ]->MtxToWorld()->m_vPos.DistSqXZ( m_DestPoints[ i ] ) <= _DDR_GOOD_ENOUGH_DIST_SQXZ ) {
				++uNumDone;
			}
		}

		if( m_paMiners[ i ]->AIBrain()->GetAIMover()->FaceToward( m_LookPoint ) ) {
			++uNumDone;
		}

		m_paMiners[ i ]->AIBrain()->GetAIMover()->EndFrame();
	}

	if( !_IsInSpecialMode() ) {
		if( ( m_eStageState == STATE_WALK_GLITCH_BACK )  || ( m_eStageState == STATE_GLITCH_GETS_KILLED ) ) {
			CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->BeginFrame();

			if( CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->MoveToward( m_GlitchDestPoint ) ) {
				++uNumDone;
			}

			if( CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->FaceToward( m_LookPoint ) ) {
				++uNumDone;
			}

			CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->EndFrame();

			if( uNumDone == 6 ) {
				// We got to our destination
				// Put stuff back
				if( m_eStageState != STATE_GLITCH_GETS_KILLED ) {
					CSpyVsSpy::TakeControlFromPlayer( FALSE );
#if FANG_WINGC
					if( _CoopActive() ) { _CoopPlacePlayers(); _CoopCommandState( FALSE ); }
#endif
				}

				uNumDone = 4;
			} else {
				uNumDone = 5; // Just one above what we test so we never think we are done when we aren't
			}

		}
	} else {
		if( !( m_uFlags & FLAG_SPECIAL_FAIL_COMMANDS ) || 
			( m_eStageState == STATE_WALK_GLITCH_BACK ) || 
			( m_eStageState == STATE_GLITCH_GETS_KILLED ) ) {
			m_pKillMiner->AIBrain()->GetAIMover()->BeginFrame();

			if( m_pKillMiner->AIBrain()->GetAIMover()->MoveToward( m_GlitchDestPoint ) ) {
				++uNumDone;
			}

			if( m_pKillMiner->AIBrain()->GetAIMover()->FaceToward( m_LookPoint ) ) {
				++uNumDone;
			}

			m_pKillMiner->AIBrain()->GetAIMover()->EndFrame();
		}

		if( uNumDone == 6 ) {
			// We got to our destination
			uNumDone = 4;
		} else {
			uNumDone = 5; // Just one above what we test so we never think we are done when we aren't
		}
	}
	
	if( uNumDone == 4 ) {
		if( m_eStageState == STATE_WAIT_FINISH_COMMAND ) {
			m_fIdleTimer -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fIdleTimer, 0.0f );

			if( m_fIdleTimer == 0.0f ) {
#if FANG_WINGC
				// Timed tolerances must not let the next command erase an unmet
				// target. Judge each player's completed command before advancing.
				if( _CoopActive() && !_CoopFinishCommand() ) return;
#endif
				m_eStageState = STATE_IDLE_TIME;
			}

		} else if( m_eStageState != STATE_GLITCH_GETS_KILLED ) {
			m_eStageState = STATE_IDLE_TIME;

			if( m_fIdleTimer == 0.0f ) {
				m_fIdleTimer = CSpyVsSpy::GetIdleTime( m_fTimeInterp );
			}
		}
	}
}

void CDDRStage::_RecalcLookPoint( void ) {
	CFVec3A Up = CFVec3A::m_UnitAxisY;

	Up.Mul( CSpyVsSpy::GetGlitch()->m_fCollCylinderHeight_WS );
	m_LookPoint = m_DirMatrix.m_vFront;
	m_LookPoint.Mul( 100.0f );
	//m_LookPoint.Add( m_StartPoints[ 0 ] ); // Just use one of the start points, doens't matter
	m_LookPoint.Add( m_CommandGruntStart );
	m_LookPoint.Add( Up );
}

void CDDRStage::_CheckAlignment( CBot *pBot ) {
	if( m_eStageState == STATE_GLITCH_GETS_KILLED ) {
		return;
	}

	// If we failing commands in special mode, don't bother to check any of this
	if( _IsInSpecialMode() && !( m_uFlags & FLAG_SPECIAL_FAIL_COMMANDS ) ) {
		return;
	}

	f32 fMoveTime;
	f32 fFrontDot = 0.0f;
	f32 fHorzDot = 0.0f;
	BOOL bDie = FALSE;
 f32 maxMissDistanceSq=_DDR_MAX_MISS_DIST_SQXZ;
 f32 minAlignmentDot=_DDR_MIN_HORZ_DOT, minLookDot=_DDR_LOOK_DOT_MIN;
 f32 alignTime=CSpyVsSpy::m_pConfigValues->fDDRAlignFailTime;
 f32 positionTime=CSpyVsSpy::m_pConfigValues->fDDRPositionFailTime;
 f32 lookTime=CSpyVsSpy::m_pConfigValues->fDDRLookFailTime;
 f32 moveGrace=0;
#if FANG_WINGC
 if(_CoopActive()) {
  maxMissDistanceSq=12.25f; // 3.5 feet, independently around each player's marker.
  minAlignmentDot=.80f; minLookDot=.60f;
  alignTime+=.75f; positionTime+=.75f; lookTime+=.75f; moveGrace=.6f;
 }
#endif
	CFVec3A Bot0ToGlitch;
	CFVec3A GlitchDir;
	CHumanControl *pControl = (CHumanControl *) pBot->Controls();
	CFVec3A TestPosition = pBot->MtxToWorld()->m_vPos;
#if FANG_WINGC
	if( _CoopActive() ) TestPosition.Sub( m_aCoopDance[pBot->m_nPossessionPlayerIndex].offset );
#endif

	//
	// Testing for jumps
	//
	if( !_IsInSpecialMode() ) {
		// The player jumped.  Was it at the right time?
		if( ( pControl->m_nPadFlagsJump & GAMEPAD_BUTTON_1ST_PRESS_MASK ) ) {
			BOOL bWaitJump = ( m_uFlags & FLAG_JUMP1_IN_QUEUE ) || ( m_uFlags & FLAG_JUMP2_IN_QUEUE );

			if( ( m_uFlags & FLAG_FAKE_COMMAND ) ) {
				bWaitJump = FALSE;
			}

			if( bWaitJump ) {
				// If we are expecting a jumping move, give the player a bit of slack
				BOOL bLetThemSlide = FALSE;

				if( m_fIdleTimer <= _DDR_JUMP_FORGIVE_TIME ) {
					bLetThemSlide = TRUE;
					FMATH_SETBITMASK( m_uFlags, FLAG_HAVE_JUMP );

					if( ( m_uFlags & FLAG_JUMP1_IN_QUEUE ) ) {
						FMATH_SETBITMASK( m_uFlags, FLAG_JUMP1 );
					} else {
						FMATH_SETBITMASK( m_uFlags, FLAG_JUMP2 );
					}

					//FMATH_CLEARBITMASK( m_uFlags, ( FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE ) );
				}

				if( !bLetThemSlide ) {
					// Do this since we take control away when we die and the bitmask doesn't
					// seem to get cleared when we do this.
					bDie = TRUE;
					goto _QuickDie;
				}
			}

			// We have a jump command or are waiting on one
			if( ( m_uFlags & FLAG_HAVE_JUMP ) ) {
				// Jump1 command
				if( ( m_uFlags & FLAG_JUMP1 ) ) {
					FMATH_SETBITMASK( m_uFlags, FLAG_JUMP_OK );
					FMATH_CLEARBITMASK( m_uFlags, ( FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE ) );

					// Don't want a double jump while we single jump
					if( pBot->IsInAir() && !pBot->GetBotFlag_CanDoubleJump() ) {
						bDie = TRUE;
						goto _QuickDie;
					}
				}

				// Jump2 command
				if( ( m_uFlags & FLAG_JUMP2 ) ) {
					if( !pBot->GetBotFlag_CanDoubleJump() ) {
						FMATH_SETBITMASK( m_uFlags, FLAG_JUMP_OK );
						FMATH_CLEARBITMASK( m_uFlags, ( FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE ) );
					}
				}
			} else {
				// We jumped and it was at the wrong time( we don't have a queued jump )
				bDie = TRUE;
				goto _QuickDie;
			}
		}
	}

#if FANG_WINGC
 // The spoken command announces a future target. Do not punish players for
 // starting their turn/step during the voice line, against the previous target.
 if(_CoopActive() && (m_uFlags & FLAG_WAIT_EXECUTE)) {
  if(bDie) goto _QuickDie;
  return;
 }
#endif
	//
	// Angle test
	//
//	Bot0ToGlitch.Sub( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos, m_DestPoints[ 0 ]/*m_paMiners[ 0 ]->MtxToWorld()->m_vPos*/ );
	Bot0ToGlitch.Sub( TestPosition, m_DestPoints[ 0 ] );
	Bot0ToGlitch.y = 0.0f;

	if( Bot0ToGlitch.MagSq() > FMATH_POS_EPSILON ) {
		Bot0ToGlitch.Unitize();
	
		fHorzDot = m_StartMatrix.m_vRight.Dot( Bot0ToGlitch );

		if( fHorzDot < minAlignmentDot ) {
			m_fAlignFailTimer += FLoop_fPreviousLoopSecs;
		} else {
			m_fAlignFailTimer -= ( 4.0f * FLoop_fPreviousLoopSecs );
		}
	} else {
		m_fAlignFailTimer += FLoop_fPreviousLoopSecs;
	}

	FMATH_CLAMP( m_fAlignFailTimer, 0.0f, alignTime );
	if( m_fAlignFailTimer == alignTime ) {
		bDie = TRUE;
	}

	//
	// If we have passed the max time for moving, check the distance
	//
	m_fGlitchMoveTimer += FLoop_fPreviousLoopSecs;
	fMoveTime = CSpyVsSpy::GetMoveTime( m_fTimeInterp ) + moveGrace;
	FMATH_CLAMPMAX( m_fGlitchMoveTimer, fMoveTime );

	// Time is up, we need to be in our dest position
	if( m_fGlitchMoveTimer == fMoveTime ) {
		//f32 fDistSqXZ = m_GlitchDestPoint.DistSqXZ( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
		f32 fDistSqXZ = m_GlitchDestPoint.DistSqXZ( TestPosition );

		if( fDistSqXZ > maxMissDistanceSq ) {
			m_fPositionFailTimer += FLoop_fPreviousLoopSecs;;
		} else {
			m_fPositionFailTimer -= ( 4.0f * FLoop_fPreviousLoopSecs );
		
			// If we happen to be in our dest position, give a bit of slack with alignment
			if( bDie && fHorzDot >= _DDR_MIN_HORZ_DOT_SECONDARY ) {
				m_fAlignFailTimer = 0.0f;
				bDie = FALSE;
			}
		}
		FMATH_CLAMP( m_fPositionFailTimer, 0.0f, positionTime );
		if( m_fPositionFailTimer == positionTime ) {
			bDie = TRUE;
		}

		if( ( m_uFlags & FLAG_HAVE_JUMP ) && !( m_uFlags & FLAG_JUMP_OK ) ) {
			if( !( m_uFlags & ( FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE ) ) ) {
				if( ( m_uFlags & FLAG_JUMP1 ) || ( m_uFlags & FLAG_JUMP2 ) ) {
					bDie = TRUE;
				}
			}
		}

#if FANG_WINGC
		// Keep each player's requirement until the command completion check.
		if( !_CoopActive() )
#endif
		FMATH_CLEARBITMASK( m_uFlags, ( FLAG_HAVE_JUMP | FLAG_JUMP1 | FLAG_JUMP2 ) );
	}


	//
	// Check the look direction
	//
//	GlitchDir = CSpyVsSpy::GetGlitch()->m_MountUnitFrontXZ_WS;
	GlitchDir = pBot->m_MountUnitFrontXZ_WS;
	GlitchDir.y = 0.0f;

	fFrontDot = m_paMiners[ 0 ]->MtxToWorld()->m_vFront.Dot( GlitchDir );
	
	if( fFrontDot < minLookDot ) {
		m_fLookFailTimer += FLoop_fPreviousLoopSecs;
	} else {
		m_fLookFailTimer -= ( 4.0f * FLoop_fPreviousLoopSecs );
	}

	// If they try to rotate the wrong way don't let them get away with it
	if( fFrontDot < 0.0f ) {
#if FANG_WINGC
  if(!_CoopActive())
#endif
		bDie = TRUE;
	}


	FMATH_CLAMP( m_fLookFailTimer, 0.0f, lookTime );
	if( m_fLookFailTimer == lookTime ) {
		bDie = TRUE;
	}

_QuickDie:
	if( bDie ) {
#if FANG_WINGC
  if(_CoopActive()) DEVPRINTF("Co-op: instructor rejects P%d state=%d flags=%u move=%.2f align=%.2f position=%.2f look=%.2f targetDist=%.2f.\n",
   pBot->m_nPossessionPlayerIndex+1,m_eStageState,m_uFlags,m_fGlitchMoveTimer,m_fAlignFailTimer,m_fPositionFailTimer,m_fLookFailTimer,
   m_GlitchDestPoint.DistSqXZ(TestPosition));
#endif
		pControl->m_nPadFlagsJump &= ~GAMEPAD_BUTTON_1ST_PRESS_MASK;

		if( m_uBadMoveCount < _DDR_MAX_MESSUPS && !_IsInSpecialMode() ) {
			_CheckOnGlitch();
		} else {
			_SetGlitchGetsKilled();
		}
	}
}

void CDDRStage::_MeshAlphaWork( void ) {
	if( !m_pHoloMesh->IsAddedToWorld() ) {
		return;
	}

	m_fHoloMeshAlpha += ( m_fHoloMeshAlphaFadeDir * FLoop_fPreviousLoopSecs );
	FMATH_CLAMP( m_fHoloMeshAlpha, 0.0f, _DDR_MAX_MESH_ALPHA );

	if( m_fHoloMeshAlpha == _DDR_MAX_MESH_ALPHA ) {
		m_fHoloMeshAlphaFadeDir = -m_fHoloMeshAlphaFadeDir;
	}

	m_pHoloMesh->SetMeshAlpha( m_fHoloMeshAlpha );

	if( m_fHoloMeshAlpha == 0.0f ) {
		m_pHoloMesh->RemoveFromWorld();
	}

	m_pHoloMesh->SetMeshTint( 0.0f, 1.0f, 0.0f );
}

void CDDRStage::_MeshPlaceIntoWorld( CFMtx43A *pMtx /*= NULL*/, BOOL bJump/* = FALSE*/, BOOL bDblJump/* = FALSE*/, BOOL bDelay/* = FALSE*/ ) {
	CFMtx43A Temp;
	CFVec3A BumpUp;

	BumpUp = CFVec3A::m_UnitAxisY;
	BumpUp.Mul( CSpyVsSpy::GetGlitch()->m_fCollCylinderHeight_WS * 0.65f );

	if( bJump || bDblJump ) {
		CFVec3A Add = CFVec3A::m_UnitAxisY;

		Add.Mul( bJump ? _DDR_JUMP_BUMP : _DDR_DLB_JUMP_BUMP );
		BumpUp.Add( Add );
	}

	if( pMtx ) {
		Temp = *pMtx;
		Temp.m_vPos.Add( BumpUp );
	} else {
		Temp = m_DirMatrix;
		Temp.m_vPos = m_GlitchDestPoint;
		Temp.m_vPos.Add( BumpUp );
	}

	m_pHoloMesh->m_Xfm.BuildFromMtx( Temp, 1.0f );
#if FANG_WINGC
	if( _CoopActive() && bDelay ) m_pHoloMesh->RemoveFromWorld();
#endif

	if( !bDelay ) {
		m_pHoloMesh->AddToWorld();
		m_pHoloMesh->UpdateTracker();
	}

	m_fHoloMeshAlpha = _DDR_MAX_MESH_ALPHA;
	m_fHoloMeshAlphaFadeDir = -1.0f;//-fmath_Inv( CSpyVsSpy::GetMoveTime( m_fTimeInterp ) );
}

void CDDRStage::_CheckOnGlitch( void ) {
	// Stop his talk
	if( ai_IsTalking( m_pCommandGrunt->AIBrain() ) ) {
		ai_TalkModeEnd( m_pCommandGrunt->AIBrain() );
	}

	m_eStageState = STATE_CHECK_GLITCH;

	// Don't let them run away from us
	if( !_IsInSpecialMode() ) {
		CSpyVsSpy::TakeControlFromPlayer( TRUE );
	}

	// Make sure you set this
	m_fIdleTimer = 0.0f;

	// Run to Glitch
	m_GotoPoint = m_pCommandGrunt->MtxToWorld()->m_vPos;

	if( !_IsInSpecialMode() ) {
		m_GotoPoint.Sub( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
	} else {
		m_GotoPoint.Sub( m_pKillMiner->MtxToWorld()->m_vPos );
	}
	
	if( m_GotoPoint.MagSq() > FMATH_POS_EPSILON ) {
		m_GotoPoint.Unitize();
	} else {
		m_GotoPoint.Zero();
	}	

	m_GotoPoint.Mul( 10.0f );
	
	if( !_IsInSpecialMode() ) {
		m_GotoPoint.Add( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );
	} else {
		m_GotoPoint.Add( m_pKillMiner->MtxToWorld()->m_vPos );
	}

	m_GotoPoint.Add( CFVec3A::m_UnitAxisY );

	u32 uGUID = !_IsInSpecialMode() ? CSpyVsSpy::GetGlitch()->Guid() : m_pKillMiner->Guid();

	ai_AssignGoal_GotoWithLookAt( 
		m_pCommandGrunt->AIBrain(), 
		m_GotoPoint,
		CFVec3A::m_Null,
		uGUID,
		50,
		15,
		100 );

	FMATH_CLEARBITMASK( m_uFlags, FLAG_ALL_BITS );

	CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_SCRATCH ) );

	level_FadeMusicVolume( 0.1f, 2.0f );
}

void CDDRStage::_CheckGlitchWork( void ) {
	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->BeginFrame();
	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->FaceToward( m_pCommandGrunt->MtxToWorld()->m_vPos );
	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->EndFrame();

	if( m_fIdleTimer == 0.0f ) {
		// This happens before the below chunk, the idle is for walking the guy back
		if( m_pCommandGrunt->MtxToWorld()->m_vPos.DistSqXZ( m_GotoPoint ) <= ( 15.0f * 15.0f ) ) {
			CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_BAD_BOT_WARNING );
			CFSoundInfo *pInfo = CFSoundGroup::GetRandomSoundInfo( pGroup );
			u32 uNdx;

			m_fIdleTimer = fsndfx_GetDuration( pInfo->m_hSound ) + 2.0f;

			if( !fclib_stricmp( "BRGT_what" , pInfo->m_pszSoundTagName ) ) {
				uNdx = CSpyVsSpy::BTR_WHAT;
			} else if( !fclib_stricmp( "BRGT_hmm"  , pInfo->m_pszSoundTagName ) ) {
				uNdx = CSpyVsSpy::BTR_HMM;
			} else if( !fclib_stricmp( "BRGT_thison" , pInfo->m_pszSoundTagName ) ) {
				uNdx = CSpyVsSpy::BTR_THISON;
			}

			CSpyVsSpy::PlayBotTalk( m_pCommandGrunt, uNdx );

			// Make sure there are no jump flags
			CHumanControl *pControl = (CHumanControl *) CSpyVsSpy::GetGlitch()->Controls();
			pControl->m_nPadFlagsJump &= ~GAMEPAD_BUTTON_1ST_PRESS_MASK;

			level_FadeMusicVolume( _DDR_MUSIC_VOL, 2.0f );
		}
	} else {
		m_fIdleTimer -= FLoop_fPreviousLoopSecs;

		if( m_fIdleTimer < 0.0f ) {
			m_fIdleTimer = 0.0f;
			_GlitchMessedUp();
		}
	}
}

void CDDRStage::_GlitchMessedUp( void ) {
	m_eStageState = STATE_WALK_GLITCH_BACK;
	
	// Make them start over
	m_DanceMoveWalker.m_uCurrentMove = 0;
	m_DanceMoveWalker.m_uNumFakeMoves = 0;

	if( !m_uBadMoveCount ) {
		_InitWalker();
	}

	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( 0.8f );

	// Make everyone go back to their start positions
	m_DestPoints[ 0 ] = m_StartPoints[ 0 ];
	m_DestPoints[ 1 ] = m_StartPoints[ 1 ];
	m_GlitchDestPoint = m_GlitchStartPoint;

	m_DirMatrix = m_StartMatrix;

	_RecalcLookPoint();

	++m_uBadMoveCount;

	m_fMoveTimeIdle = CSpyVsSpy::GetIdleTime( m_fTimeInterp );

	ai_AssignGoal_GotoWithLookAt( m_pCommandGrunt->AIBrain(), m_CommandGruntStart, CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 50, 1, 100 );
}

void CDDRStage::_SetGlitchGetsKilled( void ) {
#if FANG_WINGC
	if( _CoopActive() ) { m_bCoopRetryPending = TRUE; return; }
#endif
	CFVec3A GruntDest;

	if( m_eStageState == STATE_GLITCH_GETS_KILLED ) {
		FASSERT_NOW;
	}

	m_eStageState = STATE_GLITCH_GETS_KILLED;

	// Make everyone go back to their start positions
	m_DestPoints[ 0 ] = m_StartPoints[ 0 ];
	m_DestPoints[ 1 ] = m_StartPoints[ 1 ];

	// Point where the crane will grab us
	m_GlitchDestPoint = m_CraneGrabPoint;

	GruntDest = CFVec3A::m_UnitAxisX;
	GruntDest.Add( CFVec3A::m_UnitAxisZ );
	GruntDest.Mul( 4.0f );
	GruntDest.Add( m_CraneGrabPoint );

	ai_AssignGoal_GotoWithLookAt( m_pCommandGrunt->AIBrain(), GruntDest, CFVec3A::m_Null, CSpyVsSpy::GetGlitch()->Guid(), 1, 5, 80, 0 );
	m_DirMatrix = m_StartMatrix;

	_RecalcLookPoint();

	if( !_IsInSpecialMode() ) {
		CSpyVsSpy::TakeControlFromPlayer( TRUE );
	}

	CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DEFECTIVE_BOT );
	m_fIdleTimer = fsndfx_GetDuration( CFSoundGroup::GetSoundHandle( pGroup, 0 ) ) + 2.0f;
	CSpyVsSpy::PlayBotTalk( m_pCommandGrunt, CSpyVsSpy::BTR_DEFECT, &m_fIdleTimer );
}

void CDDRStage::_SetStateGrabGlitch( void ) {
	m_pDDRCrane->UserAnim_Select( 1 );
	m_pDDRCrane->UserAnim_Pause( FALSE );
	m_pDDRCrane->UserAnim_UpdateUnitTime( 0.0f );
	m_eStageState = STATE_GRAB_GLITCH;

	if( !_IsInSpecialMode() ) {
		CFCamera* pCam = fcamera_GetCameraByIndex( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex );

		m_Camera.m_Pos_WS = pCam->GetOwnXfmWithoutShake()->m_MtxR.m_vPos;
		m_Camera.m_Quat_WS.BuildQuat( pCam->GetOwnXfmWithoutShake()->m_MtxR );
		m_StartQuat = m_Camera.m_Quat_WS;
		m_fUnitCamInterp = 0.0f;

		gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM( CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ), &m_Camera );
	}

	CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_LOOP );
	CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pDDRCrane->MtxToWorld()->m_vPos ),
		0.0f, 1.0f, 
		m_pDDRCrane->UserAnim_GetCurrentInst()->GetTotalTime() * _DDR_OO_CRANE_TIMESCALE * _DDR_CRANE_DEATH_UNIT_TIME, 0.1f, TRUE );

}

void CDDRStage::_CraneWork( void ) {
	CFSphere Sphere;

	// Blow up the bounding sphere, yet again
	Sphere.m_Pos = m_pDDRCrane->MtxToWorld()->m_vPos.v3;
	Sphere.m_fRadius = _BIG_RADIUS;
	m_pDDRCrane->GetMesh()->MoveTracker( Sphere );

	if( !_IsInSpecialMode() ) {
		if( m_fUnitCamInterp == 0.0f ) {
			if( m_pDDRCrane->UserAnim_GetCurrentInst()->GetUnitTime() >= 0.15f ) {
				CFVec3A Pos = CFVec3A::m_UnitAxisY;

				Pos.Mul( 0.75f );
				Pos.Add( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos );

				CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &Pos );
				CSpyVsSpy::GetGlitch()->ImmobilizeBot();
				CSpyVsSpy::GetGlitch()->Attach_ToParent_WithGlue_WS( m_pDDRCrane, _DDR_CRANE_BONE_NAME );
				CSpyVsSpy::GetGlitch()->ShakeCamera( 0.5f, 0.75f );
				m_fUnitCamInterp = FLoop_fPreviousLoopSecs;

				TossSparksFromBot( CSpyVsSpy::GetGlitch() );

				CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DEATH_MACHINE );
				m_pCrusherEmitter = CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pDDRCrane->MtxToWorld()->m_vPos );
				CEmitterFader::Fade( m_pCrusherEmitter, 0.75f, 1.0f, 1.0f );

				m_pCrusher->UserAnim_Pause( FALSE );

				CFSoundGroup::PlaySound( CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_PINCH_HIT ) );
			}
		} else {
			CFVec3A ToGlitch;
			CFQuatA CurrentQuat;
			f32 fUnitTime;

			ToGlitch.Sub( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos, m_Camera.m_Pos_WS );
			CFMtx43A::m_Temp.UnitMtxFromNonUnitVec( &ToGlitch );

			CurrentQuat.BuildQuat( CFMtx43A::m_Temp );

			m_Camera.m_Quat_WS.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fUnitCamInterp ), m_StartQuat, CurrentQuat );

			m_fUnitCamInterp += ( 0.5f * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMAX( m_fUnitCamInterp, 1.0f );

			fUnitTime = m_pDDRCrane->UserAnim_GetCurrentInst()->GetUnitTime();

			if( fUnitTime >= 1.0f ) {
				m_pCrusher->UserAnim_UpdateTime( 0.0f );
				m_pCrusher->UserAnim_Pause( TRUE );

				CEmitterFader::Fade( m_pCrusherEmitter, 1.0f, 0.0f, 1.0f );
				m_pCrusherEmitter = NULL;
			} else if( fUnitTime >= _DDR_CRANE_DEATH_UNIT_TIME ) {
				if( !CSpyVsSpy::GetGlitch()->IsDeadOrDying() ) {
					CSpyVsSpy::GetGlitch()->MobilizeBot();
					CSpyVsSpy::GetGlitch()->DetachFromParent();			
					CSpyVsSpy::GetGlitch()->Die( TRUE );

					// Give control back to the player
					CSpyVsSpy::TakeControlFromPlayer( FALSE );

					f32 fTime = m_pDDRCrane->UserAnim_GetCurrentInst()->GetTotalTime() -
						( m_pDDRCrane->UserAnim_GetCurrentInst()->GetTotalTime() * _DDR_OO_CRANE_TIMESCALE * _DDR_CRANE_DEATH_UNIT_TIME );
					CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_LOOP );

					CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pDDRCrane->MtxToWorld()->m_vPos ),
						0.0f, 1.0f, 
						fTime, 0.1f, TRUE );
				}

			} 
		}
	} else {
		f32 fTime = m_pDDRCrane->UserAnim_GetCurrentInst()->GetUnitTime();

		if( fTime >= 0.15f && !m_pKillMiner->IsImmobile() ) {
			CFVec3A Pos = CFVec3A::m_UnitAxisY;

			Pos.Mul( 0.75f );
			Pos.Add( m_pKillMiner->MtxToWorld()->m_vPos );

			m_pKillMiner->Relocate_Xlat_WS( &Pos );
			m_pKillMiner->ImmobilizeBot();
			m_pKillMiner->Attach_ToParent_WithGlue_WS( m_pDDRCrane, _DDR_CRANE_BONE_NAME );

			TossSparksFromBot( m_pKillMiner );

			CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_DEATH_MACHINE );
			m_pCrusherEmitter = CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pDDRCrane->MtxToWorld()->m_vPos );
			CEmitterFader::Fade( m_pCrusherEmitter, 0.0f, 1.0f, 1.0f );

			m_pCrusher->UserAnim_Pause( FALSE );
		}

		f32 fUnitTime = m_pDDRCrane->UserAnim_GetCurrentInst()->GetUnitTime();

		if( fUnitTime >= 1.0f ) {
			m_pCrusher->UserAnim_UpdateTime( 0.0f );
			m_pCrusher->UserAnim_Pause( TRUE );

			CEmitterFader::Fade( m_pCrusherEmitter, 1.0f, 0.0f, 1.0f );
			m_pCrusherEmitter = NULL;
		} else if( fTime >= _DDR_CRANE_DEATH_UNIT_TIME ) {
			if( !m_pKillMiner->IsDeadOrDying() ) {
				m_pKillMiner->MobilizeBot();
				m_pKillMiner->DetachFromParent();			
				m_pKillMiner->Die( TRUE );

				f32 fTime = m_pDDRCrane->UserAnim_GetCurrentInst()->GetTotalTime() -
					( m_pDDRCrane->UserAnim_GetCurrentInst()->GetTotalTime() * _DDR_OO_CRANE_TIMESCALE * _DDR_CRANE_DEATH_UNIT_TIME );
				CFSoundGroup *pGroup = CSpyVsSpySoundCenter::GetGroup( CSpyVsSpySoundCenter::SOUND_CRANE_MOVE_LOOP );

				CEmitterFader::FadeInAndOut( CFSoundGroup::AllocAndPlaySound( pGroup, FALSE, &m_pDDRCrane->MtxToWorld()->m_vPos ),
					0.0f, 1.0f, 
					fTime, 0.1f, TRUE );

				_SetFinished( TRUE );
			}
		}
	}
}

void CDDRStage::_RunToDoors( void ) {
	CSpyVsSpy::TakeControlFromPlayer( TRUE );

	CSpyVsSpy::GetGlitch()->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( 0.8f );
	m_paMiners[ 0 ]->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( 1.0f );
	m_paMiners[ 1 ]->AIBrain()->GetAIMover()->SetForwardGoalSpeedPctCapable( 0.9f );

	m_fIdleTimer = 2.0f;
	m_eStageState = STATE_RUN_TO_DOORS;

	ai_AssignGoal_Goto( CSpyVsSpy::GetGlitch()->AIBrain(), m_DoorDest1, 2, 80, 0 );
	ai_AssignGoal_Goto( m_paMiners[ 0 ]->AIBrain(), m_DoorDest2, 2, 100, 0 );
	ai_AssignGoal_Goto( m_paMiners[ 1 ]->AIBrain(), m_DoorDest3, 2, 100, 0 );
}

#define _DDR_DOOR_RAD_SQ ( 7.0f * 7.0f )

void CDDRStage::_RunToDoorsWork( void ) {
	u32 uDone = 0;

	CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.z += ( FLoop_fPreviousLoopSecs * 5.0f );
	CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.y -= ( FLoop_fPreviousLoopSecs * 5.0f );
	FMATH_CLAMPMAX( CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.z, 0.0f );
	FMATH_CLAMPMIN( CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.y, 0.0f );

	if( CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.z == 0.0f && CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.y == 0.0f ) {
		GameCamType_e nCamType;
		CCamBot *pBotCam = (CCamBot *) gamecam_GetCameraManByIndex( GAME_CAM_PLAYER_1, &nCamType );

		if( nCamType == GAME_CAM_TYPE_ROBOT_3RD ) {
			CSpyVsSpy::GetGlitch()->m_vecCamPosAdj.Zero();
			pBotCam->SetAlwaysOverride( FALSE );
		}
	}

	if( m_DoorDest1.DistSqXZ( CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos ) <= _DDR_DOOR_RAD_SQ ) {
		++uDone;
	}

	if( m_DoorDest2.DistSqXZ( m_paMiners[ 0 ]->MtxToWorld()->m_vPos ) <= _DDR_DOOR_RAD_SQ ) {
		++uDone;
	}

	if( m_DoorDest3.DistSqXZ( m_paMiners[ 1 ]->MtxToWorld()->m_vPos ) <= _DDR_DOOR_RAD_SQ ) {
		++uDone;
	}

	if( uDone == 3 ) {
		CPlayer *pPlayer = &Player_aPlayer[ CSpyVsSpy::GetGlitch()->m_nPossessionPlayerIndex ];

		if( !pPlayer->IsViewFading() ) {
			CFColorRGBA StartColorRGBA, EndColorRGBA;
		
			StartColorRGBA.TransparentWhite();
			EndColorRGBA.OpaqueWhite();

			pPlayer->StartViewFade( &StartColorRGBA, &EndColorRGBA, 3.0f );
#if FANG_WINGC
				if( _CoopActive() ) for( s32 n = 1; n < CPlayer::m_nPlayerCount; ++n ) Player_aPlayer[n].StartViewFade( &StartColorRGBA, &EndColorRGBA, 3.0f );
#endif
		} else {
			if( pPlayer->GetViewFadeUnitProgress() >= 1.0f ) {
				CFColorRGBA StartColorRGBA, EndColorRGBA;
			
				StartColorRGBA.OpaqueWhite();
				EndColorRGBA.TransparentBlack();

				pPlayer->StartViewFade( &StartColorRGBA, &EndColorRGBA, 3.0f );
#if FANG_WINGC
				if( _CoopActive() ) for( s32 n = 1; n < CPlayer::m_nPlayerCount; ++n ) Player_aPlayer[n].StartViewFade( &StartColorRGBA, &EndColorRGBA, 3.0f );
#endif

				// Restore control back to the player
				CSpyVsSpy::TakeControlFromPlayer( FALSE );

				CSpyVsSpy::GetGlitch()->Relocate_Xlat_WS( &m_BattleStartPoint );
				CSpyVsSpy::GetGlitch()->DontIgnoreBotVBotCollision();

				_SetFinished( TRUE );

				ftex_ActivateRenderTarget( m_pTVTexture, FALSE );
				level_StopMusic();
			}	
		}
	}
}

void CDDRStage::_TVCallback( void *pUserData ) {
	FASSERT( pUserData );

	static CFXfm m_VXfm;
	static CFVec3A Pos;
	CDDRStage *pDDR = (CDDRStage *) pUserData;

	if( !pDDR->m_pViewport ) {
		pDDR->m_pViewport = fviewport_Create();
	}

	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	fviewport_InitPersp( pDDR->m_pViewport, FMATH_DEG2RAD( 15.0f ), 1.0f, 1000.0f );
	fviewport_SetActive( pDDR->m_pViewport );

	Pos.Set( 332.0f, -18.0f, 182.0f );

	if( !((CDDRStage * ) CSpyVsSpy::m_pGame->m_pDanceStage)->_IsInSpecialMode() ) {
		m_VXfm.BuildLookat( Pos.v3, CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.v3 );
	} else {
		m_VXfm.BuildLookat( Pos.v3, ((CDDRStage * ) CSpyVsSpy::m_pGame->m_pDanceStage)->m_pKillMiner->MtxToWorld()->m_vPos.v3 );
	}

	m_VXfm.InitStackWithView();

	fvis_Draw( NULL, FALSE, 0 );
}

BOOL CDDRStage::_LoadCSV( void ) {
	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	FMemFrame_t MemFrame = fmem_GetFrame();
	u32 uNumTables = 0;

	hFile = fgamedata_LoadFileToFMem( _DDR_CSV_NAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CDDRStage::_LoadCSV(): Could not load DDR CSV file '%s.csv'.\n", _DDR_CSV_NAME );
		goto _ExitWithError;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		++uNumTables;
	}

	if( !uNumTables ) {
		DEVPRINTF( "CDDRStage::_LoadCSV(): No dances defined in '%s.csv'.", _DDR_CSV_NAME );
		goto _ExitWithError;
	}

	if( !m_DanceTable.Setup( uNumTables ) ) {
		goto _ExitWithError;
	}

	uNumTables = 0;
	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		if( !_LoadFromGameData( hTable, uNumTables ) ) {
			DEVPRINTF( "CDDRStage::_LoadCSV(): Trouble parsing table named '%s'.\n", fgamedata_GetTableName( hTable ) );
			goto _ExitWithError;
		}

		++uNumTables;
	}

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );

	return FALSE;
}

BOOL CDDRStage::_LoadFromGameData( FGameDataTableHandle_t hTable, u32 uTableNum ) {
	u32 i, uNumFields;
	cchar *pszString;
	FGameData_TableEntry_t StringEntry;	
	FResFrame_t MemFrame = fres_GetFrame();

	StringEntry.nFlags = (FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_ONLY);
	StringEntry.nBytesForData = sizeof( char * );

	uNumFields = fgamedata_GetNumFields( hTable );

	if( uNumFields <= 0 ) {
		DEVPRINTF( "CDDRStage::_LoadFromGameData(): No fields in table.\n" );
		goto _ExitWithError;
	}

	FASSERT( m_DanceTable.m_paeMoves[ uTableNum ] == NULL );

	m_DanceTable.m_paeMoves[ uTableNum ] = (u32 *) fres_Alloc( sizeof( u32 ) * uNumFields );

	if( !m_DanceTable.m_paeMoves[ uTableNum ] ) {
		DEVPRINTF( "CDDRStage::_LoadFromGameData(): Failed to allocate memory for dance moves.\n" );
		goto _ExitWithError;
	}

	for( i = 0; i < uNumFields; ++i ) {
		m_DanceTable.m_paeMoves[ uTableNum ][ i ] = MOVE_TYPE_INVALID;
	}

	m_DanceTable.m_pauNumMovesPerGroup[ uTableNum ] = uNumFields;

	u32 eMove;

	for( i = 0; i < uNumFields; ++i ) {
		if( !fgamedata_GetFieldFromTable( hTable, i, &StringEntry, &pszString ) ) {
			DEVPRINTF( "CDDRStage::_LoadFromGameData(): Trouble reading dance string data.\n" );
			goto _ExitWithError;
		}

		eMove = _ClassifyDanceMove( pszString );

		if( eMove == MOVE_TYPE_INVALID ) {
			goto _ExitWithError;
		}

		m_DanceTable.m_paeMoves[ uTableNum ][ i ] = eMove;

	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( MemFrame );
	return FALSE;
}

#if !FANG_PRODUCTION_BUILD && SAS_ACTIVE_USER == SAS_USER_NATHAN
static cchar *_Classify( u32 eType ) {
	if( eType >= MOVE_TYPE_COUNT ) {
		eType -= MOVE_TYPE_COUNT;
	}

	switch( eType ) {
		case MOVE_TYPE_LEFT:
			return "LEFT";
		break;
		case MOVE_TYPE_RIGHT:
			return "RIGHT";
		break;
		case MOVE_TYPE_FORWARD:
			return "FORWARD";
		break;
		case MOVE_TYPE_BACKWARD:
			return "BACKWARD";
		break;
		case MOVE_TYPE_ROTATE_RIGHT:
			return "ROT RIGHT";
		break;
		case MOVE_TYPE_ROTATE_LEFT:
			return "ROT LEFT";
		break;
	}

	return "WHAT THE HELL";
}
void CDDRStage::_DrawDebug( void ) {
	f32 fX = 0.65f;
	f32 fY = 0.1f;
	f32 fInc = 0.03f;

	for( u32 i = 0; i < m_DanceTable.GetNumMoves( m_DanceMoveWalker.m_uMoveGroup ); ++i ) {
		cchar *pszName = _Classify( m_DanceTable.GetDanceMove( m_DanceMoveWalker.m_uMoveGroup, i ) );

		if( ( m_DanceMoveWalker.m_uCurrentMove - 1 ) == i /*&& ( m_eStageState == STATE_WAIT_FINISH_COMMAND )*/ ) {
			ftext_DebugPrintf( fX, fY, "~FA~S1.20~C00990099~w1%s",  pszName );
		} else {
			ftext_DebugPrintf( fX, fY, "~FA~S1.20~w1%s",  pszName );
		}
		fY += fInc;
	}
}

#define _STEPS ( 4 )
#define _DIST ( _DDR_MOVE_DISTANCE )

void CDDRStage::_DrawDebugGrid( void ) {
	/*s32 i, j;
	CFVec3A DirX, DirY;
	CFVec3A Pos;

	for( i = -( _STEPS - 1 ); i < _STEPS; ++i ) {
		DirX.Set( m_StartMatrix.m_vRight );
		DirX.Mul( (f32) i * _DIST );

		for( j = -( _STEPS - 1 ); j < _STEPS; ++j ) {
			DirY.Set( m_StartMatrix.m_vFront );
			DirY.Mul( (f32) j * _DIST );

			Pos.Set( m_StartMatrix.m_vPos );
			Pos.y = CSpyVsSpy::GetGlitch()->MtxToWorld()->m_vPos.y;
			Pos.Add( DirX );
			Pos.Add( DirY );

			fdraw_DevSphere( &Pos.v3, fmath_Sqrt( _DDR_MAX_MISS_DIST_SQXZ ), &FColor_MotifBlue, 2, 2 );
		}
	}*/
}

#endif
#if FANG_WINGC
BOOL CDDRStage::_CoopActive() { return MultiplayerMgr.IsLocalCoop() && !_IsInSpecialMode(); }

void CDDRStage::_CoopPlacePlayers() {
	if( !_CoopActive() ) return;
	const CFVec3A cameraAdjust = CSpyVsSpy::GetGlitch()->m_vecCamPosAdj;
	static const f32 offsets[4][2] = { {0,0}, {0,-4}, {0,-8}, {0,-12} };
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
		CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
		if( !pBot || pBot->IsDeadOrDying() ) continue;
		CFVec3A side, back;
		side.Mul( m_StartMatrix.m_vRight, offsets[n][0] );
		back.Mul( m_StartMatrix.m_vFront, offsets[n][1] );
		m_aCoopDance[n].offset = side; m_aCoopDance[n].offset.Add(back);
		CFMtx43A matrix = m_StartMatrix;
		matrix.m_vPos = m_GlitchStartPoint; matrix.m_vPos.Add(m_aCoopDance[n].offset);
		pBot->DetachFromParent(); pBot->MobilizeBot(); pBot->DrawEnable(TRUE,TRUE);
		pBot->Relocate_RotXlatFromUnitMtx_WS(&matrix);
		pBot->ZeroVelocity();
		pBot->IgnoreBotVBotCollision();
		pBot->m_vecCamPosAdj = cameraAdjust; pBot->m_vecCamLookAdj.Zero();
		gamecam_SwitchPlayerTo3rdPersonCamera(PLAYER_CAM(n),pBot);
		GameCamType_e type;
		CCamBot *pCam = (CCamBot *)gamecam_GetCameraManByIndex(PLAYER_CAM(n),&type);
		if(type==GAME_CAM_TYPE_ROBOT_3RD) pCam->SetAlwaysOverride(TRUE);
		Player_aPlayer[n].ZeroControls();
		Player_aPlayer[n].m_HumanControl.Zero();
		if(n) { Player_aPlayer[n].EnableEntityControl(); aibrainman_ConfigurePlayerBotBrain(pBot->AIBrain(),n); }
	}
}

void CDDRStage::_CoopCommandState( BOOL bExecute ) {
	if( !_CoopActive() ) return;
	const u32 jumps = FLAG_HAVE_JUMP | FLAG_JUMP1 | FLAG_JUMP2 | FLAG_JUMP_OK | FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE;
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
		CoopDanceState &state = m_aCoopDance[n];
		const u32 earlyJump = bExecute ? (state.flags & FLAG_JUMP_OK) : 0;
		state.flags = (m_uFlags & jumps & ~FLAG_JUMP_OK) | earlyJump;
		state.align = state.position = state.look = 0;
		state.move = m_fGlitchMoveTimer;
	}
}

void CDDRStage::_CoopCheckPlayers() {
	if( m_eStageState == STATE_WAIT_DIALOG || m_eStageState == STATE_WAIT_WORK || m_bCoopRetryPending ) return;
	const u32 jumps = FLAG_HAVE_JUMP | FLAG_JUMP1 | FLAG_JUMP2 | FLAG_JUMP_OK | FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE;
	const u32 sharedFlags = m_uFlags;
	const StageState_e previous = m_eStageState;
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
		CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
		if(!pBot || pBot->IsDeadOrDying() || !Player_aPlayer[n].HasEntityControl()) continue;
		CoopDanceState &state = m_aCoopDance[n];
		m_uFlags = (sharedFlags & ~jumps) | state.flags;
		m_fGlitchMoveTimer=state.move; m_fAlignFailTimer=state.align; m_fPositionFailTimer=state.position; m_fLookFailTimer=state.look;
		_CheckAlignment(pBot);
		state.flags=m_uFlags & jumps; state.move=m_fGlitchMoveTimer;
		state.align=m_fAlignFailTimer; state.position=m_fPositionFailTimer; state.look=m_fLookFailTimer;
		if(m_eStageState!=previous || m_bCoopRetryPending) {
   m_uFlags=(m_uFlags & ~jumps) | m_aCoopDance[0].flags;
   m_fGlitchMoveTimer=m_aCoopDance[0].move;
   return;
  }
	}
	m_uFlags=(sharedFlags & ~jumps) | m_aCoopDance[0].flags; m_fGlitchMoveTimer=m_aCoopDance[0].move;
}

BOOL CDDRStage::_CoopFinishCommand() {
	// Endpoint checks are independent of the accumulating failure timers. Those
	// timers forgive brief mistakes; they cannot approve a skipped instruction.
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
		CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityOrig;
		if( !pBot || pBot->IsDeadOrDying() || !Player_aPlayer[n].HasEntityControl() ) {
			m_bCoopRetryPending = TRUE;
			return FALSE;
		}
		CFVec3A position = pBot->MtxToWorld()->m_vPos;
		position.Sub( m_aCoopDance[n].offset );
		const f32 distanceSq = position.DistSqXZ( m_GlitchDestPoint );
		const f32 facing = m_DirMatrix.m_vFront.Dot( pBot->m_MountUnitFrontXZ_WS );
		const u32 jumpFlags = m_aCoopDance[n].flags;
		const BOOL jumpMissed = (jumpFlags & FLAG_HAVE_JUMP) && !(jumpFlags & FLAG_JUMP_OK);
		// Bound tolerance below a whole authored step, even with another config.
		const f32 radius = FMATH_MIN( 3.5f, CSpyVsSpy::m_pConfigValues->fDDRMoveDistance * 0.65f );
		if( distanceSq > radius * radius || facing < 0.60f || jumpMissed ) {
			DEVPRINTF( "Co-op: instructor incomplete P%d distanceSq=%.2f facing=%.2f jumpMissed=%d.\n",
				n+1, distanceSq, facing, jumpMissed );
			if( m_uBadMoveCount < _DDR_MAX_MESSUPS ) _CheckOnGlitch();
			else _SetGlitchGetsKilled();
			return FALSE;
		}
	}
	// A completed jump must not authorize an unrelated jump during the next
	// announcement. Keep requirements through this deadline, then retire them.
	const u32 jumps = FLAG_HAVE_JUMP | FLAG_JUMP1 | FLAG_JUMP2 | FLAG_JUMP_OK | FLAG_JUMP1_IN_QUEUE | FLAG_JUMP2_IN_QUEUE;
	FMATH_CLEARBITMASK( m_uFlags, jumps );
	for( s32 n=0; n<CPlayer::m_nPlayerCount; ++n ) m_aCoopDance[n].flags = 0;
	return TRUE;
}

void CDDRStage::_CoopHologramsWork() {
	for(s32 n=0;n<3;++n) {
		CFWorldMesh *pMesh=m_apCoopHolo[n]; if(!pMesh) continue;
		if(!_CoopActive() || n+1>=CPlayer::m_nPlayerCount || !m_pHoloMesh->IsAddedToWorld()) {pMesh->RemoveFromWorld();continue;}
		// World meshes use the forward model-to-world transform.
		CFMtx43A matrix=m_pHoloMesh->m_Xfm.m_MtxF;
		matrix.m_vPos.Add(m_aCoopDance[n+1].offset);
		pMesh->m_Xfm.BuildFromMtx(matrix,1.0f);
		pMesh->SetMeshAlpha(m_fHoloMeshAlpha);
		static const f32 colors[3][3]={{0,1,1},{1,.6f,0},{1,0,1}};
		pMesh->SetMeshTint(colors[n][0],colors[n][1],colors[n][2]);
		pMesh->AddToWorld(); pMesh->UpdateTracker();
	}
}

BOOL CDDRStage::PortTestParty( u32 nTest ) {
	char mode[32]; GetEnvironmentVariableA("MA_PORT_TEST_COOP_POLISH",mode,sizeof(mode));
	if(fclib_stricmp(mode,"spy-party") || !_CoopActive()) return FALSE;
	if(nTest==11 || nTest==14 || nTest==15) {
		PortTestParty(2);
		m_DestPoints[1]=m_StartPoints[1]; m_uBadMoveCount=0;
		m_DanceMoveWalker.m_uMoveGroup=0; m_DanceMoveWalker.m_uCurrentMove=0;
		m_fTimeInterp=1.0f;
		_ExecuteCommand(); // Actual first retail command: six-foot step left.
		for(s32 n=0;n<CPlayer::m_nPlayerCount;++n) {
			Player_aPlayer[n].ZeroControls();
			Player_aPlayer[n].m_HumanControl.m_nPadFlagsJump=0;
			if(nTest==15 || (nTest==14 && n==0)) {
				CFMtx43A matrix=m_DirMatrix; matrix.m_vPos=m_GlitchDestPoint;
				matrix.m_vPos.Add(m_aCoopDance[n].offset);
				if(nTest==15) {CFVec3A miss;miss.Mul(m_StartMatrix.m_vRight,2.8f);matrix.m_vPos.Add(miss);}
				((CBot *)Player_aPlayer[n].m_pEntityOrig)->Relocate_RotXlatFromUnitMtx_WS(&matrix);
			}
		}
		DEVPRINTF("COOP-TEST command fixture: case=%u window=%.2f targetStepSq=%.2f.\n",nTest,m_fIdleTimer,m_GlitchDestPoint.DistSqXZ(m_GlitchStartPoint));
		return m_GlitchDestPoint.DistSqXZ(m_GlitchStartPoint)>30 && m_fIdleTimer>1.8f;
	}
	if(nTest==12) {
		if(!m_pHoloMesh->IsAddedToWorld() || m_fHoloMeshAlpha<=0.0f) {DEVPRINTF("COOP-TEST holo main: added=%d alpha=%.2f state=%d.\n",m_pHoloMesh->IsAddedToWorld(),m_fHoloMeshAlpha,m_eStageState);return FALSE;}
		for(s32 n=1;n<CPlayer::m_nPlayerCount;++n) {
			CFWorldMesh *mesh=m_apCoopHolo[n-1];
			if(!mesh || !mesh->IsAddedToWorld()) {DEVPRINTF("COOP-TEST holo partner %d: allocated=%d added=%d.\n",n+1,mesh!=NULL,mesh ? mesh->IsAddedToWorld() : FALSE);return FALSE;}
			CFVec3A expected=m_pHoloMesh->m_Xfm.m_MtxF.m_vPos;expected.Add(m_aCoopDance[n].offset);
			if(mesh->m_Xfm.m_MtxF.m_vPos.DistSq(expected)>.01f || mesh->m_Xfm.m_MtxF.m_vPos.DistSq(m_pHoloMesh->m_Xfm.m_MtxF.m_vPos)<1) {DEVPRINTF("COOP-TEST holo partner %d: errorSq=%.2f separationSq=%.2f offsetSq=%.2f.\n",n+1,mesh->m_Xfm.m_MtxF.m_vPos.DistSq(expected),mesh->m_Xfm.m_MtxF.m_vPos.DistSq(m_pHoloMesh->m_Xfm.m_MtxF.m_vPos),m_aCoopDance[n].offset.MagSq());return FALSE;}
		}
		return TRUE;
	}
	if(nTest==17) {
		if(m_pHoloMesh->IsAddedToWorld()) return FALSE;
		for(s32 n=1;n<CPlayer::m_nPlayerCount;++n)
			if(m_apCoopHolo[n-1] && m_apCoopHolo[n-1]->IsAddedToWorld()) return FALSE;
		return m_eStageState==STATE_WAIT_FINISH_COMMAND && m_fIdleTimer>0;
	}
	if(nTest==13) return m_eStageState==STATE_CHECK_GLITCH;
	if(nTest==16) return m_eStageState==STATE_IDLE_TIME && !(m_uFlags & FLAG_WAIT_EXECUTE);
	if(nTest==0) { _SetGlitchGetsKilled(); return m_bCoopRetryPending; }
	if(nTest==1) return m_eStageState==STATE_WALK_GLITCH_BACK || m_eStageState==STATE_IDLE_TIME;
	if(nTest==3) {
		CBot *pBot=(CBot *)Player_aPlayer[1].m_pEntityOrig;
		CFMtx43A matrix=*pBot->MtxToWorld(); CFVec3A position=matrix.m_vPos; matrix.RotateY(FMATH_PI); matrix.m_vPos=position;
		pBot->Relocate_RotXlatFromUnitMtx_WS(&matrix);
  const f32 oldFrame=FLoop_fPreviousLoopSecs;
  FLoop_fPreviousLoopSecs=.1f;
  for(int n=0;n<60 && m_eStageState!=STATE_CHECK_GLITCH;++n) _CoopCheckPlayers();
  FLoop_fPreviousLoopSecs=oldFrame;
		return m_eStageState==STATE_CHECK_GLITCH;
	}
	if(nTest==4 || nTest==5) {
		if(nTest==4) {
			m_uFlags=FLAG_HAVE_JUMP|FLAG_JUMP1; _CoopCommandState(FALSE);
			m_eStageState=STATE_WAIT_FINISH_COMMAND;
			Player_aPlayer[0].m_HumanControl.m_nPadFlagsJump=GAMEPAD_BUTTON_1ST_PRESS_MASK;
			Player_aPlayer[1].m_HumanControl.m_nPadFlagsJump=0;
		} else {
			Player_aPlayer[0].m_HumanControl.m_nPadFlagsJump=0;
			Player_aPlayer[1].m_HumanControl.m_nPadFlagsJump=GAMEPAD_BUTTON_1ST_PRESS_MASK;
		}
		_CoopCheckPlayers();
		const BOOL good=(m_aCoopDance[0].flags & FLAG_JUMP_OK) &&
			((m_aCoopDance[1].flags & FLAG_JUMP_OK)!=0)==(nTest==5);
		Player_aPlayer[0].ZeroControls(); Player_aPlayer[1].ZeroControls();
		return good;
	}
 if(nTest==8 || nTest==9 || nTest==10) {
  m_uFlags=0; _CoopCommandState(FALSE);
  m_eStageState=STATE_WAIT_FINISH_COMMAND;
  CBot *bot=(CBot *)Player_aPlayer[1].m_pEntityOrig;
  CFVec3A position=bot->MtxToWorld()->m_vPos,side;
  side.Mul(m_StartMatrix.m_vRight,nTest==10?8.0f:2.8f); position.Add(side);
  bot->Relocate_Xlat_WS(&position);
  if(nTest==9) {m_uFlags|=FLAG_WAIT_EXECUTE;CFMtx43A facing=*bot->MtxToWorld();CFVec3A position=facing.m_vPos;facing.RotateY(FMATH_PI);facing.m_vPos=position;bot->Relocate_RotXlatFromUnitMtx_WS(&facing);}
  const f32 oldFrame=FLoop_fPreviousLoopSecs;FLoop_fPreviousLoopSecs=.1f;
  for(int n=0;n<60 && m_eStageState==STATE_WAIT_FINISH_COMMAND;++n) _CoopCheckPlayers();
  FLoop_fPreviousLoopSecs=oldFrame;
  return nTest==10 ? m_eStageState==STATE_CHECK_GLITCH : (m_eStageState==STATE_WAIT_FINISH_COMMAND && !m_bCoopRetryPending);
 }
	if(nTest==6) { _RunToDoors(); return m_eStageState==STATE_RUN_TO_DOORS; }
	if(nTest==7) return IsFinished();
	if(nTest==2) {
		// Run the production validator independently in each lane with correct position/facing.
		m_eStageState=STATE_WAIT_FINISH_COMMAND; m_fIdleTimer=2;
		m_uFlags=0; m_DirMatrix=m_StartMatrix; m_GlitchDestPoint=m_GlitchStartPoint;
		m_DestPoints[0]=m_StartPoints[0]; m_fGlitchMoveTimer=0;
		for(s32 n=0;n<CPlayer::m_nPlayerCount;++n) {m_aCoopDance[n].flags=0;m_aCoopDance[n].align=m_aCoopDance[n].position=m_aCoopDance[n].look=m_aCoopDance[n].move=0;}
		CSpyVsSpy::TakeControlFromPlayer(FALSE); _CoopPlacePlayers();
		_CoopCheckPlayers();
		return m_eStageState==STATE_WAIT_FINISH_COMMAND && !m_bCoopRetryPending;
	}
	return FALSE;
}
#endif
