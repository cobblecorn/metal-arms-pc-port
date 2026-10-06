//////////////////////////////////////////////////////////////////////////////////////
// gamesave.cpp - game load and save functionality.
//
// Author: Chris MacDonald
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
// 09/28/02 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "gamesave.h"
#include "fscriptsystem.h"
#include "FScriptTypes.h"
#include "fEventListener.h"
#include "fscriptinst.h"
#include "FPointPath1.h"
#include "FPointPath2.h"
#include "FQOTang1.h"
#include "FQuatTang2.h"
#include "FQuatTang3.h"
#include "FQuatComp.h"
#include "fcamshake.h"
#include "entity.h"
#include "fdebris.h"
#include "fclib.h"
#include "eproj.h"
#include "player.h"
#include "item.h"
#include "BarterSystem.h"
#include "botswarmer.h"
#include "eproj_cleaner.h"
#include "AI\AIEnviro.h"
#include "ai\aigroup.h"
#include "AI\AIEdgeLock.h"
#include "ai\aibtATABLE.H"
#include "fxempblast.h"
#include "AlarmSys.h"
#include "spawnsys.h"
#include "splat.h"
#include "FXStreamer.h"
#include "FXSlower.h"
#include "game.h"
#include "MAScriptTypes.h"
#include "ColiseumMiniGame.h"
#include "GeneralCorrosiveGame.h"
#include "MG_FinalBattle.h"
#include "collectable.h"
#include "SpyVsSpy.h"
#include "mg_HoldYourGround.h"
#include "weapon_recruiter.h"
#include "fdecal.h"
#include "SpaceDock.h"
#include "ZombieBossGame.h"
#include "esphere.h"
#include "MultiplayerMgr.h"
#include "PauseScreen.h"

#define _CHECKPOINT_TEXT_TIME		( 3.0f )	// time in seconds to display checpoint saved/restored text

enum {
	_TEXT_MODE_SAVE,
	_TEXT_MODE_RESTORE,
};


//------------------------------------------------------------------------------
// private checkpoint save & restore functionality
//------------------------------------------------------------------------------

static BOOL _bSaveCheckpoint;
static BOOL _bRestoreCheckpoint;
static s32 _nCheckpoint;
static BOOL _bPrintText;
#if FANG_WINGC
static BOOL _bCheckpointReviveDeferred;
#endif
static CFMtx43A _mtxCheckpointXform;
static f32 _fCheckpointRadius;

static f32 _fTextTimeRemaining;								// time remaining to display checkpoint text
static s32 _nTextMode;											// indicates which text phrase to print
static BOOL _bCheckpointSaved[ FCHECKPOINT_MAX_CHECKPOINTS ];	// remember that a checkpoint has been saved to.
static CFCheckPoint::ObjectDataHandle_t _hSaveData[ FCHECKPOINT_MAX_CHECKPOINTS ];	// handle for global data to save
static FTextAreaHandle_t	_hTextBoxCheckpoint;					// handle for checkpoint saved text
static CFCheckPoint::ObjectDataHandle_t _hPlayerPossessHandle[ FCHECKPOINT_MAX_CHECKPOINTS ];	// handles for saving player possession info

extern BOOL ai_bUnarmedPlayerRuleDisabled;

static void _checkpoint_ReOrientUser( void );

//------------------------------------------------------------------------------
// saves various global engine data
static void _SaveGlobalData( s32 nCheckpoint )
{
	_hSaveData[nCheckpoint] = CFCheckPoint::CreateObjectDataHandle();
	CFCheckPoint::SaveData( FLoop_nRealTotalLoopTicks );
	CFCheckPoint::SaveData( FLoop_nTotalLoopTicks );
	CFCheckPoint::SaveData( ai_bUnarmedPlayerRuleDisabled);

	// save the level parameters.
	// cjm moved here because level_CheckpointSave() "piggybacks" on to the data handle set above.
	level_CheckpointSave();

}

//------------------------------------------------------------------------------
// restores various global engine data
static void _RestoreGlobalData( s32 nCheckpoint )
{
	CFCheckPoint::SetObjectDataHandle( _hSaveData[nCheckpoint] );
	CFCheckPoint::LoadData( FLoop_nRealTotalLoopTicks );
	CFCheckPoint::LoadData( FLoop_nTotalLoopTicks );
	CFCheckPoint::LoadData( ai_bUnarmedPlayerRuleDisabled);

	// Restore the level parameters...
	// cjm moved here because level_CheckpointRestore() "piggybacks" on to the data handle set above.
	level_CheckpointRestore();
}

//------------------------------------------------------------------------------
// saves info about bot player is possessing
static void _SavePlayerPossession( s32 nCheckpoint )
{
	s32 nIndex;

	_hPlayerPossessHandle[nCheckpoint] = CFCheckPoint::CreateObjectDataHandle();

	for( nIndex = 0; nIndex < CPlayer::m_nPlayerCount; nIndex++ )
	{
		FASSERT( Player_aPlayer[ nIndex ].m_pEntityCurrent );
		CFCheckPoint::SaveData( (u32&) Player_aPlayer[ nIndex ].m_pEntityCurrent );
		CFCheckPoint::SaveData( Player_aPlayer[ nIndex ].m_pEntityCurrent->GetArmorModifier() );
	}
}

//------------------------------------------------------------------------------
// restores player to bot he was possessing at save time, if any.
static void _RestorePlayerPossession( s32 nCheckpoint )
{
	CEntity *pPlayerEntityCurrent[MAX_PLAYERS];
	f32 fPossessedArmorModifier[MAX_PLAYERS];
	s32 nIndex;

	CFCheckPoint::SetObjectDataHandle( _hPlayerPossessHandle[nCheckpoint] );

	for( nIndex = 0; nIndex < CPlayer::m_nPlayerCount; nIndex++ )
	{
		CFCheckPoint::LoadData( (u32&) pPlayerEntityCurrent[ nIndex ] );
		CFCheckPoint::LoadData( fPossessedArmorModifier[ nIndex ] );
	}

	// if player was possessing a bot at time of checkpoint save, put him back into that bot now.
	for( nIndex = 0; nIndex < CPlayer::m_nPlayerCount; nIndex++ )
	{
		if( pPlayerEntityCurrent[ nIndex ] != Player_aPlayer[ nIndex ].m_pEntityOrig )
		{
			CBot *pBot;

			FASSERT( pPlayerEntityCurrent[ nIndex ]->TypeBits() & ENTITY_BIT_BOT );
			if( pPlayerEntityCurrent[ nIndex ]->TypeBits() & ENTITY_BIT_BOT )
			{
				pBot = (CBot*) pPlayerEntityCurrent[ nIndex ];
				pBot->Possess( nIndex, fPossessedArmorModifier[ nIndex ] );
				pBot->Power_SetState( TRUE /*bPowerUp*/ );
				Player_aPlayer[ nIndex ].EnableEntityControl();

				if( Player_aPlayer[ nIndex ].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOTGLITCH )
				{
					CBotGlitch *pGlitchy;
					pGlitchy = (CBotGlitch*) Player_aPlayer[ nIndex ].m_pEntityOrig;
					pGlitchy->RespawnEffect_Kill();	// prevent respawn effect when operating a console
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
// script save and restore functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// saves the state of all script instances & event listeners
static BOOL _script_InstanceCheckpointSaveAll( s32 nCheckpoint )
{
	u32 nScriptInstances = CFScriptSystem::NumScriptInsts();
	u32 nListeners = CFScriptSystem::NumEventListeners();
	CFScriptInst *paScriptInstances = CFScriptSystem::GetScriptInstList();
	CFEventListener *paEventListeners = CFScriptSystem::GetEventListenerList();
	CFScriptInst *pInst;
	CFEventListener *pList;
	u32 nIndex;

	FASSERT( nCheckpoint < FSCRIPT_INST_MAX_STATE_SAVES );

	for( nIndex = 0; nIndex < nScriptInstances; nIndex++ )
	{
		pInst = &paScriptInstances[nIndex];

		// get a handle to the data we are about to save
		pInst->m_hStateSave[nCheckpoint] = CFCheckPoint::CreateObjectDataHandle();

		if( pInst->m_pDataArea )
		{
			// save the script instance data
			if( pInst->m_pScript->m_uDataAreaSize )
			{
				CFCheckPoint::SaveData( (void*) pInst->m_pDataArea, (u32) pInst->m_pScript->m_uDataAreaSize );
			}
		}

		CFCheckPoint::SaveData( pInst->m_nOnInitIdx );
		CFCheckPoint::SaveData( pInst->m_nOnEventIdx );
		CFCheckPoint::SaveData( pInst->m_nWorkIdx );
		CFCheckPoint::SaveData( pInst->m_nOnEndIdx );
		CFCheckPoint::SaveData( pInst->m_uEventFlags );
		CFCheckPoint::SaveData( pInst->m_uFlags );
		CFCheckPoint::SaveData( pInst->m_fDisabledCtdn );
	}

	for( nIndex = 0; nIndex < nListeners; nIndex++ )
	{
		pList = &paEventListeners[nIndex];

		// get a handle to the data we are about to save
		pList->m_hStateSave[nCheckpoint] = CFCheckPoint::CreateObjectDataHandle();

		// save data for this listener
		CFCheckPoint::SaveData( pList->m_bDisabled );
		CFCheckPoint::SaveData( pList->m_uEventFlags );
		CFCheckPoint::SaveData( pList->m_uUserData );
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// restores the state of all script instances & event listeners
static BOOL _script_InstanceCheckpointRestoreAll( s32 nCheckpoint )
{
	u32 nScriptInstances = CFScriptSystem::NumScriptInsts();
	u32 nListeners = CFScriptSystem::NumEventListeners();
	CFScriptInst *paScriptInstances = CFScriptSystem::GetScriptInstList();
	CFEventListener *paEventListeners = CFScriptSystem::GetEventListenerList();
	CFScriptInst *pInst;
	CFEventListener *pList;
	u32 nIndex;

	FASSERT( nCheckpoint < FSCRIPT_INST_MAX_STATE_SAVES );

	for( nIndex = 0; nIndex < nScriptInstances; nIndex++ )
	{
		pInst = &paScriptInstances[nIndex];

		// set the data handle for this script instance
		CFCheckPoint::SetObjectDataHandle( pInst->m_hStateSave[nCheckpoint] );

		if( pInst->m_pDataArea )
		{
			// load the script instance data
			if( pInst->m_pScript->m_uDataAreaSize )
			{
				CFCheckPoint::LoadData( (void*) pInst->m_pDataArea, (u32) pInst->m_pScript->m_uDataAreaSize );
			}
		}

		CFCheckPoint::LoadData( pInst->m_nOnInitIdx );
		CFCheckPoint::LoadData( pInst->m_nOnEventIdx );
		CFCheckPoint::LoadData( pInst->m_nWorkIdx );
		CFCheckPoint::LoadData( pInst->m_nOnEndIdx );
		CFCheckPoint::LoadData( pInst->m_uEventFlags );
		CFCheckPoint::LoadData( pInst->m_uFlags );
		CFCheckPoint::LoadData( pInst->m_fDisabledCtdn );
	}

	for( nIndex = 0; nIndex < nListeners; nIndex++ )
	{
		pList = &paEventListeners[nIndex];

		// set the data handle for this listener
		CFCheckPoint::SetObjectDataHandle( pList->m_hStateSave[nCheckpoint] );

		CFCheckPoint::LoadData( pList->m_bDisabled );
		CFCheckPoint::LoadData( pList->m_uEventFlags );
		CFCheckPoint::LoadData( pList->m_uUserData );
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// save various pooled objects used by script system.
s32 ghMathCPHandle[ FCHECKPOINT_MAX_CHECKPOINTS ];
static BOOL _math_SavePools( s32 nCheckpoint )
{
	FASSERT( nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS );
	ghMathCPHandle[ nCheckpoint ] = CFCheckPoint::CreateObjectDataHandle();

	CFVec3A *pCFVec3 = CFST_CFVec3Wrapper::GetPool();
	FASSERT( pCFVec3 );
	s32 nCFVec3 = CFST_CFVec3Wrapper::GetPoolNumEntries();
	FASSERT( nCFVec3 > 0 );
	CFCheckPoint::SaveData( (void*) pCFVec3, sizeof( CFVec3A ) * nCFVec3 );

	CFV3OPointPath1 *pPoint1 = CFV3OPointPath1::GetPool();
	FASSERT( pPoint1 );
	s32 nPoint1 = CFV3OPointPath1::GetPoolNumEntries();
	FASSERT( nPoint1 );
	CFCheckPoint::SaveData( (void*) pPoint1, sizeof( CFV3OPointPath1 ) * nPoint1 );

	CFV3OPointPath2 *pPoint2 = CFV3OPointPath2::GetPool();
	FASSERT( pPoint2 );
	s32 nPoint2 = CFV3OPointPath2::GetPoolNumEntries();
	FASSERT( nPoint2 );
	CFCheckPoint::SaveData( (void*) pPoint2, sizeof( CFV3OPointPath2 ) * nPoint2 );

	CFQOTang1 *pTang1 = CFQOTang1::GetPool();
	FASSERT( pTang1 );
	s32 nTang1 = CFQOTang1::GetPoolNumEntries();
	FASSERT( nTang1 > 0 );
	CFCheckPoint::SaveData( (void*) pTang1, sizeof( CFQOTang1 ) * nTang1 );

	CFQuatTang2 *pTang2 = CFQuatTang2::GetPool();
	FASSERT( pTang2 );
	s32 nTang2 = CFQuatTang2::GetPoolNumEntries();
	FASSERT( nTang2 > 0 );
	CFCheckPoint::SaveData( (void*) pTang2, sizeof( CFQuatTang2 ) * nTang2 );

	CFQuatTang3 *pTang3 = CFQuatTang3::GetPool();
	FASSERT( pTang3 );
	s32 nTang3 = CFQuatTang3::GetPoolNumEntries();
	FASSERT( nTang3 > 0 );
	CFCheckPoint::SaveData( (void*) pTang3, sizeof( CFQuatTang3 ) * nTang3 );

	CFQuatComp *pComp = CFQuatComp::GetPool();
	FASSERT( pComp );
	s32 nComp = CFQuatComp::GetPoolNumEntries();
	FASSERT( nComp > 0 );
	CFCheckPoint::SaveData( (void*) pComp, sizeof( CFQuatComp ) * nComp );

	//save the Script Timers
	CMAST_Timer::CheckPointSave();

	return TRUE;
}

//------------------------------------------------------------------------------
// restore various pooled objects used by script system.
static BOOL _math_RestorePools( s32 nCheckpoint )
{
	s32 nIndex;

	FASSERT( nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS );
	CFCheckPoint::SetObjectDataHandle( ghMathCPHandle[ nCheckpoint ] );

	CFVec3A *pCFVec3 = CFST_CFVec3Wrapper::GetPool();
	FASSERT( pCFVec3 );
	s32 nCFVec3 = CFST_CFVec3Wrapper::GetPoolNumEntries();
	FASSERT( nCFVec3 > 0 );
	CFCheckPoint::LoadData( (void*) pCFVec3, sizeof( CFVec3A ) * nCFVec3 );

	CFV3OPointPath1 *pPoint1 = CFV3OPointPath1::GetPool();
	FASSERT( pPoint1 );
	s32 nPoint1 = CFV3OPointPath1::GetPoolNumEntries();
	FASSERT( nPoint1 );
	CFCheckPoint::LoadData( (void*) pPoint1, sizeof( CFV3OPointPath1 ) * nPoint1 );
	for( nIndex = 0; nIndex < nPoint1; nIndex++ )
	{
		if( ( !pPoint1[ nIndex ].IsWorking() || pPoint1[ nIndex ].IsDone() ) && CFMotionObj::IsInWorkList( pPoint1[ nIndex ] ) )
		{
			CFMotionObj::RemoveFromWorkList( &pPoint1[ nIndex ] );
		}
		else if( pPoint1[ nIndex ].IsWorking() && !pPoint1[ nIndex ].IsDone() && !CFMotionObj::IsInWorkList( pPoint1[ nIndex ] ) )
		{
			CFMotionObj::AddToWorkList( &pPoint1[ nIndex ] );
		}
	}

	CFV3OPointPath2 *pPoint2 = CFV3OPointPath2::GetPool();
	FASSERT( pPoint2 );
	s32 nPoint2 = CFV3OPointPath2::GetPoolNumEntries();
	FASSERT( nPoint2 );
	CFCheckPoint::LoadData( (void*) pPoint2, sizeof( CFV3OPointPath2 ) * nPoint2 );
	for( nIndex = 0; nIndex < nPoint2; nIndex++ )
	{
		if( ( !pPoint2[ nIndex ].IsWorking() || pPoint2[ nIndex ].IsDone() ) && CFMotionObj::IsInWorkList( pPoint2[ nIndex ] ) )
		{
			CFMotionObj::RemoveFromWorkList( &pPoint2[ nIndex ] );
		}
		else if( pPoint2[ nIndex ].IsWorking() && !pPoint2[ nIndex ].IsDone() && !CFMotionObj::IsInWorkList( pPoint2[ nIndex ] ) )
		{
			CFMotionObj::AddToWorkList( &pPoint2[ nIndex ] );
		}
	}

	CFQOTang1 *pTang1 = CFQOTang1::GetPool();
	FASSERT( pTang1 );
	s32 nTang1 = CFQOTang1::GetPoolNumEntries();
	FASSERT( nTang1 > 0 );
	CFCheckPoint::LoadData( (void*) pTang1, sizeof( CFQOTang1 ) * nTang1 );
	for( nIndex = 0; nIndex < nTang1; nIndex++ )
	{
		if( ( !pTang1[ nIndex ].IsWorking() || pTang1[ nIndex ].IsDone() ) && CFMotionObj::IsInWorkList( pTang1[ nIndex ] ) )
		{
			CFMotionObj::RemoveFromWorkList( &pTang1[ nIndex ] );
		}
		else if( pTang1[ nIndex ].IsWorking() && !pTang1[ nIndex ].IsDone() && !CFMotionObj::IsInWorkList( pTang1[ nIndex ] ) )
		{
			CFMotionObj::AddToWorkList( &pTang1[ nIndex ] );
		}
	}

	CFQuatTang2 *pTang2 = CFQuatTang2::GetPool();
	FASSERT( pTang2 );
	s32 nTang2 = CFQuatTang2::GetPoolNumEntries();
	FASSERT( nTang2 > 0 );
	CFCheckPoint::LoadData( (void*) pTang2, sizeof( CFQuatTang2 ) * nTang2 );
	for( nIndex = 0; nIndex < nTang2; nIndex++ )
	{
		if( ( !pTang2[ nIndex ].IsWorking() || pTang2[ nIndex ].IsDone() ) && CFMotionObj::IsInWorkList( pTang2[ nIndex ] ) )
		{
			CFMotionObj::RemoveFromWorkList( &pTang2[ nIndex ] );
		}
		else if( pTang2[ nIndex ].IsWorking() && !pTang2[ nIndex ].IsDone() && !CFMotionObj::IsInWorkList( pTang2[ nIndex ] ) )
		{
			CFMotionObj::AddToWorkList( &pTang2[ nIndex ] );
		}
	}

	CFQuatTang3 *pTang3 = CFQuatTang3::GetPool();
	FASSERT( pTang3 );
	s32 nTang3 = CFQuatTang3::GetPoolNumEntries();
	FASSERT( nTang3 > 0 );
	CFCheckPoint::LoadData( (void*) pTang3, sizeof( CFQuatTang3 ) * nTang3 );
	for( nIndex = 0; nIndex < nTang3; nIndex++ )
	{
		if( ( !pTang3[ nIndex ].IsWorking() || pTang3[ nIndex ].IsDone() ) && CFMotionObj::IsInWorkList( pTang3[ nIndex ] ) )
		{
			CFMotionObj::RemoveFromWorkList( &pTang3[ nIndex ] );
		}
		else if( pTang3[ nIndex ].IsWorking() && !pTang3[ nIndex ].IsDone() && !CFMotionObj::IsInWorkList( pTang3[ nIndex ] ) )
		{
			CFMotionObj::AddToWorkList( &pTang3[ nIndex ] );
		}
	}

	CFQuatComp *pComp = CFQuatComp::GetPool();
	FASSERT( pComp );
	s32 nComp = CFQuatComp::GetPoolNumEntries();
	FASSERT( nComp > 0 );
	CFCheckPoint::LoadData( (void*) pComp, sizeof( CFQuatComp ) * nComp );
	for( nIndex = 0; nIndex < nComp; nIndex++ )
	{
		if( ( !pComp[ nIndex ].IsWorking() || pComp[ nIndex ].IsDone() ) && CFMotionObj::IsInWorkList( pComp[ nIndex ] ) )
		{
			CFMotionObj::RemoveFromWorkList( &pComp[ nIndex ] );
		}
		else if( pComp[ nIndex ].IsWorking() && !pComp[ nIndex ].IsDone() && !CFMotionObj::IsInWorkList( pComp[ nIndex ] ) )
		{
			CFMotionObj::AddToWorkList( &pComp[ nIndex ] );
		}
	}

	//restore the Script Timers
	CMAST_Timer::CheckPointRestore();

	return TRUE;
}

//------------------------------------------------------------------------------
// saves to a checkpoint
static BOOL _checkpoint_Save( s32 nCheckpoint, BOOL bPrintText )
{
	BOOL bResult = FALSE;
	BOOL bFlushResult;

	FASSERT( nCheckpoint >= 0 && nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS );

	// set the checkpoint
	if( !CFCheckPoint::SetCheckPoint( nCheckpoint ) )
	{
		return FALSE;
	}

	// set checkpoint system state to "save"
	if( !CFCheckPoint::SetSaveState() )
	{
		return FALSE;
	}

	// A new save replaces the prior stream contents, so invalidate its saved flag first.
	_bCheckpointSaved[ nCheckpoint ] = FALSE;

	_SaveGlobalData( nCheckpoint );
	_SavePlayerPossession( nCheckpoint );

	CGeneralCorrosiveGame::Save( nCheckpoint );
	CMGFinalBattle::Save( nCheckpoint );

	// save entities
	bResult = CEntity::CheckpointSaveAll( nCheckpoint );

	if( bResult )
	{
		bResult = _math_SavePools( nCheckpoint );
	}

	if( bResult )
	{
		// save scripts
		bResult = _script_InstanceCheckpointSaveAll( nCheckpoint );
	}

	AlarmSys_CheckpointSave();

	bartersystem_CheckpointSave();

	CCollectable::CheckpointSaveGlobal( nCheckpoint );
	CSpaceDock::CheckpointSave(nCheckpoint);


	bFlushResult = CFCheckPoint::DoneSaving();
	if( !bFlushResult )
	{
		bResult = FALSE;
	}

	if( bResult )
	{
		// remember that we saved to this checkpoint
		_bCheckpointSaved[ nCheckpoint ] = TRUE;

		if( bPrintText )
		{
			// cause checkpoint text message to be displayed
			_fTextTimeRemaining = _CHECKPOINT_TEXT_TIME;
			_nTextMode = _TEXT_MODE_SAVE;
		}
	}

	FASSERT( bResult || !bFlushResult );
	return bResult;
}


//------------------------------------------------------------------------------
// restores to a checkpoint
static BOOL _checkpoint_Restore( s32 nCheckpoint, BOOL bPrintText )
{
	s32 nIndex;

	BOOL bResult = FALSE;
    
	FASSERT( nCheckpoint >= 0 && nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS );

	// set the checkpoint
	if( !CFCheckPoint::SetCheckPoint( nCheckpoint ) )
	{
		return FALSE;
	}

	// set checkpoint system state to "load"
	if( !CFCheckPoint::SetLoadState() )
	{
		return FALSE;
	}

#if FANG_WINGC
	// Arrivals belong to the current attempt. Keeping them across a team restore
	// could release a level exit before both players traversed the restored route.
	CEntity::CoopTripwireReset();
#endif
#if defined(MA_PC_INPUT)
	game_PcResetWeaponSelect();
#endif

	// set player back to the original bot before calling restore.
	FASSERT( CPlayer::m_nPlayerCount <= MAX_PLAYERS );
	for( nIndex = 0; nIndex < CPlayer::m_nPlayerCount; nIndex++ )
	{
		Player_aPlayer[ nIndex ].ReturnToOriginalBot();
	}

	for ( nIndex = 0; nIndex < CPlayer::m_nPlayerCount; nIndex++)
	{
		Player_aPlayer[ nIndex ].m_Hud.CheckpointRestore();
	}

	CWeaponRecruiter::UnrecruitAllBots();

	CFXStreamerEmitter::KillAllStreamers();

	CFDecal::KillAll();

	// Get the eprojs put away before restoring entities...
	CEProjPool::ReturnAllProjectilesToFreePoolForAllPools();


	// load global data
	_RestoreGlobalData( nCheckpoint );

	// restore the alarm system before the entities, many of the alarm system elements
	// use bots from a pool and need to have them in the world to properly clean up
	AlarmSys_CheckpointRestore();

	// Restore the mini game states if the mini game is currently running.
	CZombieBossGame::Restore();
	CColiseumMiniGame::Restore();
	CGeneralCorrosiveGame::Restore( nCheckpoint );
	CMGFinalBattle::Restore( nCheckpoint );
	mg_HoldYourGround_Restore();

	// restore entities & scripts
	CEntity::CheckpointRestoreAll( nCheckpoint );

	bartersystem_CheckpointRestore();

	CCollectable::CheckpointRestoreGlobal( nCheckpoint );
	CSpaceDock::CheckpointRestore(nCheckpoint);


	//just incase
//	ResLock_CheckpointRestore();

	bResult = _math_RestorePools( nCheckpoint );

	if( bResult )
	{
		bResult = _script_InstanceCheckpointRestoreAll( nCheckpoint );
	}

	// Make sure all of the streaming data is in memory before we render a frame
	fvis_BlockOnStreamingData();

	CFCheckPoint::DoneLoading();

	CFDebrisSpawner::KillAll();
	CFDebris::KillAll( FALSE );
	//fexplosion_KillAll();

	//CSplat::KillAll();

	CSpawnSys::CheckpointRestore();
	CEProj_Cleaner::ClearTargetingCleaners();
	AIEnviro_KillAllSounds();
	AIBTA_ResetGlobalTimers();
	CFXEMPBlast::KillAll();
	CFXSlower::KillAll();
	CBotSwarmer::CheckpointRestoreSystem();
	CSpawnSys::CheckpointRestore();
	aigroup_CheckpointRestore();
	fcamera_StopEffectsOnAllCameras();
	// restore the alarm system before the entities, many of the alarm system elements
	// use bots from a pool and need to have them in the world to properly clean up
	AlarmSys_CheckpointRestore();
		
	// Restor the Spy Vs. Spy Mini-game if we are running it
	CSpyVsSpy::CheckpointRestore( nCheckpoint );
	
	// restore the alarm system before the entities, many of the alarm system elements
	// use bots from a pool and need to have them in the world to properly clean up
	AlarmSys_CheckpointRestorePostamble();

	// ensure that HUD mode is set for all players (that are not driving vehicles).
	s32 nPlayerNum;
	for( nPlayerNum = 0; nPlayerNum < CPlayer::m_nPlayerCount; nPlayerNum++ )
	{
		if( Player_aPlayer[ nPlayerNum ].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT )
		{
			CBot *pBot = (CBot*) Player_aPlayer[ nPlayerNum ].m_pEntityCurrent;
			if( !pBot->m_pDrivingVehicle )
			{
				CHud2* pHud = CHud2::GetHudForPlayer( nPlayerNum );
				pBot->SetHudMode( pHud, pBot->m_pBotDef );
			}
		}
	}

	_RestorePlayerPossession( nCheckpoint );

	if( bResult && bPrintText )
	{
		// cause checkpoint text message to be displayed
		_fTextTimeRemaining = _CHECKPOINT_TEXT_TIME;
		_nTextMode = _TEXT_MODE_RESTORE;
	}

	//Commented out by RAF 
	//The level_CheckpointRestore will restart any playing streams...
	//level_RestartAllStreams();

	FASSERT( bResult );
	return bResult;
}

//------------------------------------------------------------------------------
// If we have been requested to do so, re-orient and position the user bot
// after restoring from a checkpoint.
static void _checkpoint_ReOrientUser( void )
{
	// Should never be able to get here in multiplayer
	FASSERT( MultiplayerMgr.IsSinglePlayer() );

	// Get the player bot
	CBot* pBot = (CBot*)Player_aPlayer[0].m_pEntityCurrent;
	FASSERT( pBot && (pBot->TypeBits() & ENTITY_BIT_BOT) );

	// If the player is in a mech, what do we do??? For now, just bail...
	if( pBot->GetCurMech() )
		return;

	// OK, looks like we are OK. Now move the player
	pBot->Relocate_RotXlatFromUnitMtx_WS( &_mtxCheckpointXform );
}


//------------------------------------------------------------------------------
// Public interface for checkpoint system.
//------------------------------------------------------------------------------

BOOL checkpoint_InitSystem( void ) {
	FTextArea_t TextArea;

	// Set up "Checkpoint Saved" text box...
	ftext_SetToDefaults( &TextArea );
	TextArea.bVisible = FALSE;
	TextArea.fUpperLeftX = 0.05f;
	TextArea.fLowerRightX = 0.3f;
	TextArea.fUpperLeftY = 0.2f;
	TextArea.fLowerRightY = 0.24f;
	TextArea.fNumberOfLines = 2;
	TextArea.fBorderThicknessX = 0.0f;
	TextArea.fBorderThicknessY = 0.0f;
	TextArea.oColorForeground.SetColor( 1.0f, 1.0f, 0.0f );
	TextArea.oColorForeground.SetAlpha( 0.6f );
	TextArea.oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.5f );
	TextArea.oColorBorder.Set( 0.1f, 0.1f, 1.0f, 1.0f );
	TextArea.ohFont = '1';
	TextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	_hTextBoxCheckpoint = ftext_Create( &TextArea );

	_fTextTimeRemaining = 0.0f;
	_nTextMode = _TEXT_MODE_SAVE;

	_fCheckpointRadius = -1.0f;

	return TRUE;
}


void checkpoint_UninitSystem( void ) {
	_fTextTimeRemaining = 0.0f;
}


//------------------------------------------------------------------------------
// inits checkpoints for a new level
BOOL checkpoint_LevelInit( void )
{
	s32 nIndex;

	for( nIndex = 0; nIndex < FCHECKPOINT_MAX_CHECKPOINTS; nIndex++ )
	{
		_bCheckpointSaved[ nIndex ] = FALSE;
		_hSaveData[ nIndex ] = CFCheckPoint::NULL_HANDLE;
	}

	_bSaveCheckpoint = FALSE;
	_bRestoreCheckpoint = FALSE;
#if FANG_WINGC
	_bCheckpointReviveDeferred = FALSE;
#endif
	_fCheckpointRadius = -1.0f;
	return TRUE;
}

//------------------------------------------------------------------------------
// Returns TRUE if the checkpoint has been saved.  Useful for determining
// to which checkpoints the game can be restored.
BOOL checkpoint_Saved( s32 nCheckpoint )
{
	FASSERT( nCheckpoint >= 0 && nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS );

	return _bCheckpointSaved[ nCheckpoint ];
}

//------------------------------------------------------------------------------
// Marks a checkpoint as being unsaved.  Useful for level restarts, so that
// mid-level checkpoints can be invalidated.
void checkpoint_SetUnsaved( s32 nCheckpoint )
{
	FASSERT( nCheckpoint >= 0 && nCheckpoint < FCHECKPOINT_MAX_CHECKPOINTS );

	_bCheckpointSaved[ nCheckpoint ] = FALSE;
}

//------------------------------------------------------------------------------
// periodic work function for checkpoints.  Initiates save or restore
// requested by checkpoint_Save() and checkpoint_Restore() functions
void checkpoint_Work( void )
{
	if( _bSaveCheckpoint && !_bRestoreCheckpoint )
	{
#if FANG_WINGC
		// A trigger may fire while the surviving player is airborne. Do not save a corpse position;
		// keep this request pending until every downed partner can revive on the playable route.
		if( _nCheckpoint > 0 && !CPlayer::CoopReviveForCheckpoint() ) {
			if( !_bCheckpointReviveDeferred ) {
				DEVPRINTF( "Co-op: checkpoint slot=%d pending safe partner revival.\n", _nCheckpoint );
				_bCheckpointReviveDeferred = TRUE;
			}
			return;
		}
		if( _bCheckpointReviveDeferred ) {
			DEVPRINTF( "Co-op: checkpoint slot=%d revival ready; saving.\n", _nCheckpoint );
		}
		_bCheckpointReviveDeferred = FALSE;
#endif
		_bSaveCheckpoint = FALSE;
		_checkpoint_Save( _nCheckpoint, _bPrintText );
	}
	else if( _bRestoreCheckpoint )
	{
		// A team wipe/restart takes precedence over a checkpoint still waiting for safe placement.
		_bSaveCheckpoint = FALSE;
		_bRestoreCheckpoint = FALSE;
#if FANG_WINGC
		_bCheckpointReviveDeferred = FALSE;
#endif
		_checkpoint_Restore( _nCheckpoint, _bPrintText );

		////////////// If a radius has been specified, we need to reorient the user and 
		////////////// place his buddies
		////////////if( _fCheckpointRadius > 0.0f )
		////////////{
		////////////	_checkpoint_ReOrientUser();
		////////////}
	}
}

//------------------------------------------------------------------------------
// Global function to initiate a checkpoint save which includes a
// sphere defining the user's location and a safe radius around the
// user in which we can move his buddies.
// Save will actually occur at end of current frame.
BOOL checkpoint_Save( s32 nCheckpoint, BOOL bPrintText, CESphere* pESphere )
{
#if FANG_WINGC
	DEVPRINTF( "Port: checkpoint save requested: slot=%d notify=%d sphere=%d.\n", nCheckpoint, bPrintText, pESphere != NULL );
#endif
	_bSaveCheckpoint = TRUE;
	_nCheckpoint = nCheckpoint;
	_bPrintText = bPrintText;

	// Save off the starting transform and the radius.
	if( pESphere )
	{
		_mtxCheckpointXform = *pESphere->MtxToWorld();
		_fCheckpointRadius = pESphere->Sphere_WS()->m_fRadius;
	}
	else
	{
		// Radius of -1 indicates no user position was specified
		_fCheckpointRadius = -1.0f;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// Global function to initiate a checkpoint restore.
// Restore will actually occur at end of current frame.
BOOL checkpoint_Restore( s32 nCheckpoint, BOOL bPrintText, cchar *pszReason )
{
#if FANG_WINGC
	DEVPRINTF( "Port: checkpoint restore requested: slot=%d saved=%d notify=%d reason='%s' pause=%d current=%d.\n",
		nCheckpoint, checkpoint_Saved( nCheckpoint ), bPrintText, pszReason ? pszReason : "unspecified",
		CPauseScreen::IsActive(), CPlayer::m_nCurrent );
	for( s32 i=0; i<CPlayer::m_nPlayerCount; ++i ) {
		CEntity *pEntity = Player_aPlayer[i].m_pEntityCurrent;
		CBot *pBot = pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOT) ? (CBot *)pEntity : NULL;
		const CFVec3 *pPos = pBot && pBot->m_pWorldMesh ? &pBot->MtxToWorld()->m_vPos.v3 : NULL;
		DEVPRINTF( "Port: checkpoint restore player=%d entity='%s' world=%d dead=%d air=%d pos=(%.1f,%.1f,%.1f).\n",
			i+1, pEntity && pEntity->Name() ? pEntity->Name() : "none", pEntity ? pEntity->IsInWorld() : FALSE,
			pBot ? pBot->IsDeadOrDying() : FALSE, pBot ? pBot->IsInAir() : FALSE,
			pPos ? pPos->x : 0.f, pPos ? pPos->y : 0.f, pPos ? pPos->z : 0.f );
	}
#endif
	_bRestoreCheckpoint = TRUE;
	_nCheckpoint = nCheckpoint;
	_bPrintText = bPrintText;

	return TRUE;
}



//------------------------------------------------------------------------------
void checkpoint_Draw( void )
{
	if( _fTextTimeRemaining > 0.0f )
	{
		_fTextTimeRemaining -= FLoop_fPreviousLoopSecs;
		if( _fTextTimeRemaining < 0.0f )
		{
			_fTextTimeRemaining = 0.0f;
		}

		if( _nTextMode == _TEXT_MODE_SAVE )
		{
			ftext_PrintString( _hTextBoxCheckpoint, Game_apwszPhrases[ GAMEPHRASE_CHECKPOINT_SAVED ] );
		}
		else
		{
			ftext_PrintString( _hTextBoxCheckpoint, Game_apwszPhrases[ GAMEPHRASE_RESTORED_TO_CHECKPOINT ] );
		}
	}
}





//------------------------------------------------------------------------------
// Player Profile Functionality
//------------------------------------------------------------------------------

// inits the profile data to the defaults
BOOL CPlayerProfile::InitData() {

	InitNewProfile( FALSE );
	
	m_SaveInfo.nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
	m_SaveInfo.nStorageDeviceID = FSTORAGE_DEVICE_ID_NONE;

	m_nControllerIndex = 0;

	return TRUE;
}

// loads the named profile into the object instance.
BOOL CPlayerProfile::LoadFromCard() {
	FStorage_Error_e nError;
	
	// make sure that our save info and memory card is ready
	if( !IsDesiredCardReady( TRUE, FALSE ) ) {
		// already printed out a message
		return FALSE;
	}

	// read the profile in
    nError = fstorage_ReadProfile( m_SaveInfo.nStorageDeviceID,
		m_SaveInfo.wszProfileName,
		0,
		&m_Data,
		sizeof( GameSave_ProfileData_t ) );
	if( nError != FSTORAGE_ERROR_NONE ) {
		DEVPRINTF( "CPlayerProfile::LoadProfile: Trouble reading the profile from the card.\n" );
		return FALSE;	
	}

	// verify the profile header
	if( m_Data.Header.nFileVersion != PROFILE_VERSION_NUMBER ||
		m_Data.Header.nSignature != PROFILE_SIGNATURE ||
		m_Data.Header.nProfileBytes != sizeof( GameSave_ProfileData_t ) ) {
		DEVPRINTF( "CPlayerProfile::LoadProfile: The data in the header does not match what is required. Invalid profile.\n" );
		return FALSE;	
	}

	// generate a CRC of the data contained in the rest of the profile
	u8 *pnData = (u8 *)&m_Data;
	pnData += sizeof( GameSave_ProfileHeader_t );
	u32 nCRC = fmath_Crc32( 0, pnData, sizeof( GameSave_ProfileData_t ) - sizeof( GameSave_ProfileHeader_t ) );
	if( nCRC != m_Data.Header.nProfileCRC ) {
		DEVPRINTF( "CPlayerProfile::LoadProfile: The CRC of the file doesn't match the one stored in the profile header, Invalid profile.\n" );
		return FALSE;	
	}

	FASSERT( !IsVirtual() );
	
	// since we just read the profile, we don't need to save it till someone alters it
	m_SaveInfo.nFlags = GAMESAVE_SAVE_INFO_FLAGS_NONE;
	
    return TRUE;
}

// checks the memcard specified in m_SaveInfo for errors.
BOOL CPlayerProfile::IsDesiredCardReady( BOOL bCheckForProfiles, BOOL bCheckForNewProfileRoom ) {
	const FStorage_DeviceInfo_t *pDeviceInfo;
	s32 nStringLen; 

	// make sure that we know what device to read from
	if( m_SaveInfo.nStorageDeviceID == FSTORAGE_DEVICE_ID_NONE ) {
		DEVPRINTF( "CPlayerProfile::IsDesiredCardReady: SaveInfo not setup properly.\n" );
		return FALSE;
	}
	nStringLen = fclib_wcslen( m_SaveInfo.wszProfileName );
	if( nStringLen <= 0 || nStringLen > PROFILE_NAME_MAX_LENGTH ) {
		DEVPRINTF( "CPlayerProfile::IsDesiredCardReady: The profile string name is not setup properly, it is %d characters long.\n", nStringLen );
		return FALSE;
	}

	// see if the device is ready to be read from
	pDeviceInfo = fstorage_GetDeviceInfo( m_SaveInfo.nStorageDeviceID );
	if( !pDeviceInfo ||
		((pDeviceInfo->uStatus & FSTORAGE_DEVICE_READY_FOR_USE) != FSTORAGE_DEVICE_READY_FOR_USE ) ) {
		
		DEVPRINTF( "CPlayerProfile::IsDesiredCardReady: The mem card is not ready to be read from.\n" );
		return FALSE;
	}

	if( bCheckForProfiles && (pDeviceInfo->uNumProfiles == 0) ) {
		DEVPRINTF( "CPlayerProfile::IsDesiredCardReady: The mem card specified has no profiles on it.\n" );
		return FALSE;
	}

	if( bCheckForNewProfileRoom && (pDeviceInfo->uBytesAvailable < sizeof( GameSave_ProfileData_t )) ) {
		DEVPRINTF( "CPlayerProfile::IsDesiredCardReady: The mem card specified doesn't have room for a new profile on it.\n" );
		return FALSE;
	}

	return TRUE;
}

// saves current profile
BOOL CPlayerProfile::SaveToCard() {
	FStorage_Error_e nError;
	u8 *pnData;

	FASSERT( !IsVirtual() );

	if( (m_SaveInfo.nFlags & GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING) == 0 ) {
		// we don't need to save this profile
		DEVPRINTF( "CPlayerProfile::SaveToCard: The profile doesn't need to be saved, it isn't flagged as such.\n" );
		return TRUE;
	}
	
	// make sure that our save info and memory card is ready
	if( !IsDesiredCardReady( FALSE, (m_SaveInfo.nFlags & GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW)  ) ) {
		// already printed out a message
		return FALSE;
	}

	// fill in the profile header
	m_Data.Header.nFileVersion = PROFILE_VERSION_NUMBER;
	m_Data.Header.nSignature = PROFILE_SIGNATURE;
	m_Data.Header.nProfileBytes = sizeof( GameSave_ProfileData_t );
	// generate a CRC of the profile data
	pnData = (u8 *)&m_Data;
	pnData += sizeof( GameSave_ProfileHeader_t );
	m_Data.Header.nProfileCRC = fmath_Crc32( 0, pnData, sizeof( GameSave_ProfileData_t ) - sizeof( GameSave_ProfileHeader_t ) );

    if( m_SaveInfo.nFlags & GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW ) {
		// we need to create a new profile first
		nError = fstorage_CreateProfile( m_SaveInfo.nStorageDeviceID,
			m_SaveInfo.wszProfileName,
			sizeof( GameSave_ProfileData_t ) );
		if( nError != FSTORAGE_ERROR_NONE ) {
			DEVPRINTF( "CPlayerProfile::SaveToCard: Trouble creating a new profile.\n" );
			return FALSE;
		}
		m_SaveInfo.nFlags &= ~GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW;
	}

	// save the data out
	nError = fstorage_WriteProfile( m_SaveInfo.nStorageDeviceID, 
		m_SaveInfo.wszProfileName,
		0,
		&m_Data,
		sizeof( GameSave_ProfileData_t ) );
	if( nError != FSTORAGE_ERROR_NONE ) {
		DEVPRINTF( "CPlayerProfile::SaveToCard: Trouble writing a profile to disk.\n" );
		return FALSE;
	}

	m_SaveInfo.nFlags &= ~GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;

	return TRUE;
}

// deletes the profile, but doesn't affect the data in memory.
BOOL CPlayerProfile::DeleteFromCard() {
	FStorage_Error_e nError;

	// make sure that our save info and memory card is ready
	if( !IsDesiredCardReady( TRUE, FALSE ) ) {
		// already printed out a message
		return FALSE;
	}

	nError = fstorage_DeleteProfile( m_SaveInfo.nStorageDeviceID, m_SaveInfo.wszProfileName );
	if( nError != FSTORAGE_ERROR_NONE ) {
		DEVPRINTF( "CPlayerProfile::DeleteFromCard: Trouble deleting a profile.\n" );
		return FALSE;
	}

	return TRUE;
}


// checks to see whether the profile exists on the card and whether it's ok
BOOL CPlayerProfile::IsOnCard( void ) {
    FASSERT( !IsVirtual() );
	FStorage_Error_e nError;

	if( !IsDesiredCardReady( FALSE, FALSE ) ) {
		return FALSE;
	}

	nError = fstorage_ValidateProfile( m_SaveInfo.nStorageDeviceID, m_SaveInfo.wszProfileName, FALSE );

	return nError == FSTORAGE_ERROR_NONE;
}


// this function will NOT read the profile from the card before deleting it.
// this function will also save the current profile in memory to the new name and
// update the SaveInfo with the new name.
BOOL CPlayerProfile::RenameProfile( cwchar *pwszNewProfileName ) {
	FASSERT( !IsVirtual() );

	wchar wszOldName[FSTORAGE_MAX_NAME_LEN];

	if( !(m_SaveInfo.nFlags & GAMESAVE_SAVE_INFO_FLAGS_RENAME) ) {
		// we don't need to rename this profile
		DEVPRINTF( "CPlayerProfile::RenameProfile: The profile doesn't need to be renamed, it isn't flagged as such.\n" );
		return TRUE;
	}

	// delete the old profile
	if( !DeleteFromCard() ) {
		// already logged the error
		return FALSE;
	}

	// save off a copy of our old name
	fclib_wcsncpy( wszOldName, m_SaveInfo.wszProfileName, FSTORAGE_MAX_NAME_LEN );

	// copy the new name into the old name slot
	fclib_wcsncpy( m_SaveInfo.wszProfileName, pwszNewProfileName, FSTORAGE_MAX_NAME_LEN );

	m_SaveInfo.nFlags &= ~GAMESAVE_SAVE_INFO_FLAGS_RENAME;
	m_SaveInfo.nFlags |= (GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING | GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW);

	// save the profile in memory
	if( !SaveToCard() ) {
		// already logged the error
		
		// couldn't rename, go back to what we had before
		fclib_wcsncpy( m_SaveInfo.wszProfileName, wszOldName, FSTORAGE_MAX_NAME_LEN );
		m_SaveInfo.nFlags |= (GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING | GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW);
        SaveToCard();

		return FALSE;
	}

	return TRUE;	
}

// inits a profile with all of the default values as if the user has just created a new profile
void CPlayerProfile::InitNewProfile( BOOL bVirtualProfile/*=FALSE*/ ) {
	
	// zero out the profile
	fang_MemZero( &m_Data, sizeof( GameSave_ProfileData_t ) );

	// general data
	m_Data.nFlags = GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING;
	if( bVirtualProfile ) {
		m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE;
	}	
	m_Data.fUnitVibrationIntensity = 0.8f;
	m_Data.fUnitLookSensitivity = 0.5f;
	m_Data.nControllerConfigIndex = 0;
	m_Data.fUnitSoundLevel = 0.75f;
	m_Data.fUnitMusicLevel = 0.75f;

	m_Data.nTotalSecretChipsGathered = 0;
	m_Data.nNumMPLevelsUnlocked = 0;

	ResetToBeginning();

	// multi player data
	m_Data.nColorIndex = GAMESAVE_MP_COLORS_YELLOW;
	m_Data.nMPRulesCount = 0;

	// Note: the name we are giving here should never show up; we give it
	// this name so that we will know if a bug occurs and it does show up
	for( s32 i = 0; i < MP_RULE_SET_MAX; i++ ) {
		SetMultiPlayerRulesToDefault( &m_Data.aMultiPlayerRules[i], L"XXXX" );
	}
}

// set the multiplayer rules to the default settings.
void CPlayerProfile::SetMultiPlayerRulesToDefault( GameSave_MPRules_t *pRules, cwchar *pwszName ) {

	// In case pwszName points inside the rules (which it will sometimes)
	// make a copy of it.
	wchar wszName[META_GAME_NAME_LEN];
	if( pwszName ) {
		fclib_wcsncpy( wszName, pwszName, META_GAME_NAME_LEN - 1 );
	}

	// zero everything out to be sure that nothing is uninitialized
	fang_MemZero( pRules, sizeof( GameSave_MPRules_t ) );

	// Now copy the name back in
	if( pwszName ) {
		fclib_wcsncpy( pRules->wszGameName, wszName, META_GAME_NAME_LEN - 1 );
	}

	pRules->nGameType = GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH;

	pRules->nFlags = GAMESAVE_RULE_FLAGS_MARKERS_ON |
					 GAMESAVE_RULE_FLAGS_RADAR_ON |
					 GAMESAVE_RULE_FLAGS_POSSESSABLE_BOTS |
					 GAMESAVE_RULE_FLAGS_FRIENDLY_FIRE_DAMAGE;

	pRules->nDMKillsPerRound = 0;
	pRules->nTimeLimit = 15;

	pRules->nLimitPrimaryWeapon = GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT;
	pRules->nLimitSecondaryWeapon = GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT;
	pRules->nLimitVehicles = GAMESAVE_VEHICLE_LIMIT_ALL_VEHICLES;

	pRules->nTimeBetweenHillSwaps = 0;	
}

// resets a profile to the 1st level in the single player game, but
// will not reset any setting changes in the profile
void CPlayerProfile::ResetToBeginning() {
	u32 i;
	CInventory TempInv;
	CMemCardInventory TempMCInv;

	TempInv.SetToInitial();
	CMemCardInventory::SaveInventory( &TempMCInv, &TempInv );

	// general data
	m_Data.nCurrentLevel = 0;
	m_Data.nDifficulty = GAMESAVE_DIFFICULTY_NORMAL;
	m_Data.nFlags &= ~( GAMESAVE_PROFILE_FLAGS_100_PERCENT_FINISHED |
						GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER | 
						GAMESAVE_PROFILE_FLAGS_VISITED_BARTER_DRIODS );
	
	for( i=0; i < LEVEL_SINGLE_PLAYER_COUNT; i++ ) {
        m_Data.aLevelProgress[i].fTimeToComplete = 0.0f;
		m_Data.aLevelProgress[i].nSecretChipsCollected = 0;
		m_Data.aLevelProgress[i].nBonusSecretChipEarned = 0;
		m_Data.aLevelProgress[i].nWashersCollected = 0;
		m_Data.aLevelProgress[i].nEnemiesKilled = 0;
		
		m_Data.aLevelProgress[i].Inventory = TempMCInv;
	}
}

// has this player visited the barter droids before?
BOOL CPlayerProfile::HasVisitedBarterDroidsBefore() {
	if( m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_VISITED_BARTER_DRIODS ) {
		return TRUE;
	}
	return FALSE;
}

// mark this player as having seen the barter droids before
void CPlayerProfile::VisitedBarterDroids() {

#if FANG_WINGC
	// co-op session profiles are virtual but saved by coopsave_SaveSession()
	if( !IsVirtual() || coopsave_IsSessionActive() ) {
#else
	if( !IsVirtual() ) {
#endif
		// update the flag
		m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_VISITED_BARTER_DRIODS;
	}
}

// is this profile virtual (meaning don't save to memory card)
BOOL CPlayerProfile::IsVirtual() const {

	return (m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE);
}

// counts up the earned secret chips on the completed levels
u32 CPlayerProfile::CountEarnedSecretChips( void ) const {
	u32 i, nCount = 0;

	for( i=0; i < m_Data.nCurrentLevel; i++ ) {
		nCount += m_Data.aLevelProgress[i].nBonusSecretChipEarned;
		nCount += m_Data.aLevelProgress[i].nSecretChipsCollected;
	}
	return nCount;
}

//------------------------------------------------------------------------------
// player profile interface functions
//------------------------------------------------------------------------------

static CPlayerProfile *_apPlayerProfile[MAX_PLAYERS];

//------------------------------------------------------------------------------
// inits the system on startup
BOOL playerprofile_InitSystem( void ) {

	for( s32 nIndex = 0; nIndex < MAX_PLAYERS; nIndex++ ) {
		_apPlayerProfile[nIndex] = NULL;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// shuts down the system
void playerprofile_UninitSystem( void ) {

	for( s32 nIndex = 0; nIndex < MAX_PLAYERS; nIndex++ ) {
		_apPlayerProfile[nIndex] = NULL;
	}
}

//------------------------------------------------------------------------------
// sets the player profile object for a particular player
void playerprofile_Set( s32 nPlayer, CPlayerProfile *pProfile ) {
	FASSERT( nPlayer >= 0  && nPlayer < MAX_PLAYERS );

	_apPlayerProfile[nPlayer] = pProfile;
}

//------------------------------------------------------------------------------
// gets a pointer to the profile object for a particular player
CPlayerProfile *playerprofile_Get( s32 nPlayer ) {
	FASSERT( nPlayer >= 0  && nPlayer < MAX_PLAYERS );

	return _apPlayerProfile[nPlayer];
}

#if FANG_WINGC
//------------------------------------------------------------------------------
// PC local co-op campaign saves (see gamesave.h)

#define _COOPSAVE_SIGNATURE		( 0x434F4F50 )	// 'COOP'
#define _COOPSAVE_VERSION		( 1 )
#define _COOPSAVE_MAX_RECORDS	( 8 )			// player 1 and up to 7 partners

// one player of the campaign
typedef struct {
	wchar wszName[FSTORAGE_MAX_NAME_LEN];	// profile name, empty when the record is unused
	u32 nLastSave;							// the campaign's nSaveCount when this player last saved
	GameSave_ProfileData_t Data;			// level progress and per-level inventory
} _CoopRecord_t;

// the whole file
typedef struct {
	u32 nSignature;
	u32 nVersion;
	u32 nRecordBytes;		// sizeof( _CoopRecord_t ): a different profile layout is not read
	u32 nCRC;				// of everything after this field
	u32 nSaveCount;
	u32 nNumRecords;		// records in use, record 0 is player 1's
	_CoopRecord_t aRecords[_COOPSAVE_MAX_RECORDS];
} _CoopFile_t;

#define _COOPSAVE_CRC_OFFSET	( 4 * sizeof( u32 ) )

static _CoopFile_t _CoopFile;
static _CoopRecord_t _aCoopScratch[MAX_PLAYERS];		// players without a saved profile, not kept
static CPlayerProfile _CoopScratchProfile;				// for ResetToBeginning()
static wchar _wszCoopOwner[FSTORAGE_MAX_NAME_LEN];
static BOOL _bCoopCanSave = FALSE;
static BOOL _bCoopSession = FALSE;
static BOOL _bCoopSensitivityDirty = FALSE;
static u32 _nCoopSessionPlayers = 0;
static _CoopRecord_t *_apCoopSessionRecord[MAX_PLAYERS];
static CPlayerProfile *_apCoopSessionProfile[MAX_PLAYERS];

static u32 _CoopFileCRC( void ) {
	return fmath_Crc32( 0, (u8 *)&_CoopFile + _COOPSAVE_CRC_OFFSET, sizeof( _CoopFile_t ) - _COOPSAVE_CRC_OFFSET );
}

// A new profile's progress, keeping pData's settings.
static void _CoopResetProgress( GameSave_ProfileData_t *pData ) {
	_CoopScratchProfile.m_Data = *pData;
	_CoopScratchProfile.ResetToBeginning();
	*pData = _CoopScratchProfile.m_Data;
}

static void _CoopClearStats( GameSave_ProfileData_t *pData ) {
	for( u32 i=0; i < LEVEL_SINGLE_PLAYER_COUNT; i++ ) {
		pData->aLevelProgress[i].fTimeToComplete = 0.0f;
		pData->aLevelProgress[i].nSecretChipsCollected = 0;
		pData->aLevelProgress[i].nBonusSecretChipEarned = 0;
		pData->aLevelProgress[i].nWashersCollected = 0;
		pData->aLevelProgress[i].nEnemiesKilled = 0;
	}
}

BOOL coopsave_Load( cwchar *pwszOwner ) {
	coopsave_EndSession();

	_bCoopCanSave = ( pwszOwner && pwszOwner[0] );
	_wszCoopOwner[0] = 0;
	if( _bCoopCanSave ) {
		fclib_wcsncpy( _wszCoopOwner, pwszOwner, FSTORAGE_MAX_NAME_LEN - 1 );
		_wszCoopOwner[FSTORAGE_MAX_NAME_LEN - 1] = 0;

		u32 nBytes = 0;
		if( fstorage_PcCoopRead( _wszCoopOwner, &_CoopFile, sizeof( _CoopFile_t ), &nBytes ) ) {
			if( nBytes == sizeof( _CoopFile_t ) &&
				_CoopFile.nSignature == _COOPSAVE_SIGNATURE &&
				_CoopFile.nVersion == _COOPSAVE_VERSION &&
				_CoopFile.nRecordBytes == sizeof( _CoopRecord_t ) &&
				_CoopFile.nNumRecords >= 1 && _CoopFile.nNumRecords <= _COOPSAVE_MAX_RECORDS &&
				_CoopFile.nCRC == _CoopFileCRC() ) {
				DEVPRINTF( "Co-op save: loaded '%ls' (level %u, %u players on record).\n",
					_wszCoopOwner, (u32)_CoopFile.aRecords[0].Data.nCurrentLevel, _CoopFile.nNumRecords );
				return TRUE;
			}
			DEVPRINTF( "Co-op save: '%ls' is from another build or damaged; starting a new campaign.\n", _wszCoopOwner );
		}
	}

	// a new campaign: player 1's record only
	fang_MemZero( &_CoopFile, sizeof( _CoopFile_t ) );
	_CoopFile.nSignature = _COOPSAVE_SIGNATURE;
	_CoopFile.nVersion = _COOPSAVE_VERSION;
	_CoopFile.nRecordBytes = sizeof( _CoopRecord_t );
	_CoopFile.nNumRecords = 1;
	fclib_wcsncpy( _CoopFile.aRecords[0].wszName, _wszCoopOwner, FSTORAGE_MAX_NAME_LEN - 1 );
	_CoopScratchProfile.InitNewProfile( FALSE );
	_CoopFile.aRecords[0].Data = _CoopScratchProfile.m_Data;
	if( _bCoopCanSave ) {
		DEVPRINTF( "Co-op save: no campaign saved for '%ls' yet.\n", _wszCoopOwner );
	} else {
		DEVPRINTF( "Co-op save: player 1 has no saved profile; progress is not saved.\n" );
	}
	return FALSE;
}

BOOL coopsave_CanSave( void ) {
	return _bCoopCanSave;
}

GameSave_ProfileData_t *coopsave_GetOwnerData( void ) {
	return &_CoopFile.aRecords[0].Data;
}

void coopsave_BeginSession( const CPlayerProfile *const *papPersonal, CPlayerProfile *const *papSession, u32 nPlayers, BOOL bNewGame ) {
	u32 i, p;
	BOOL abClaimed[_COOPSAVE_MAX_RECORDS];

	FASSERT( nPlayers >= 1 && nPlayers <= MAX_PLAYERS );

	_CoopRecord_t *pOwner = &_CoopFile.aRecords[0];
	pOwner->Data.nFlags &= ~GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE;
	if( bNewGame ) {
		// a new game for the whole team
		for( i=1; i < _CoopFile.nNumRecords; i++ ) {
			_CoopResetProgress( &_CoopFile.aRecords[i].Data );
		}
	}
	for( i=0; i < _COOPSAVE_MAX_RECORDS; i++ ) {
		abClaimed[i] = FALSE;
	}

	for( p=0; p < nPlayers; p++ ) {
		const CPlayerProfile *pPersonal = papPersonal[p];
		cwchar *pwszName = pPersonal->m_SaveInfo.wszProfileName;
		const BOOL bSavedProfile = !pPersonal->IsVirtual() && pPersonal->m_SaveInfo.nStorageDeviceID != FSTORAGE_DEVICE_ID_NONE && pwszName[0];
		_CoopRecord_t *pRecord = NULL;
		BOOL bNewRecord = FALSE;

		if( p == 0 ) {
			pRecord = pOwner;
			abClaimed[0] = TRUE;
		} else if( bSavedProfile && _bCoopCanSave ) {
			// this player's record, by profile name
			for( i=1; i < _CoopFile.nNumRecords; i++ ) {
				if( !abClaimed[i] && !fclib_wcscmp( _CoopFile.aRecords[i].wszName, pwszName ) ) {
					pRecord = &_CoopFile.aRecords[i];
					abClaimed[i] = TRUE;
					break;
				}
			}
			if( !pRecord ) {
				// a new partner: a free record, else the one saved longest ago
				u32 nPick = _CoopFile.nNumRecords;
				if( nPick < _COOPSAVE_MAX_RECORDS ) {
					_CoopFile.nNumRecords++;
				} else {
					nPick = 0;
					for( i=1; i < _COOPSAVE_MAX_RECORDS; i++ ) {
						if( !abClaimed[i] && (!nPick || _CoopFile.aRecords[i].nLastSave < _CoopFile.aRecords[nPick].nLastSave) ) {
							nPick = i;
						}
					}
				}
				if( nPick ) {
					pRecord = &_CoopFile.aRecords[nPick];
					abClaimed[nPick] = TRUE;
					fang_MemZero( pRecord, sizeof( _CoopRecord_t ) );
					fclib_wcsncpy( pRecord->wszName, pwszName, FSTORAGE_MAX_NAME_LEN - 1 );
					bNewRecord = TRUE;
				}
			}
		}
		if( !pRecord ) {
			// no saved profile: played from player 1's progress, not kept
			pRecord = &_aCoopScratch[p];
			fang_MemZero( pRecord, sizeof( _CoopRecord_t ) );
			bNewRecord = TRUE;
		}

		GameSave_ProfileData_t *pData = &pRecord->Data;
		if( bNewRecord ) {
			*pData = pOwner->Data;
			_CoopClearStats( pData );
			DEVPRINTF( "Co-op save: player %u (%ls) starts from player 1's progress%s.\n", p + 1, pwszName,
				pRecord == &_aCoopScratch[p] ? " (no saved profile, not kept)" : "" );
		} else if( p > 0 ) {
			if( pData->nCurrentLevel < pOwner->Data.nCurrentLevel ) {
				// missed levels: player 1's inventory for them
				for( i = (u32)pData->nCurrentLevel + 1; i <= pOwner->Data.nCurrentLevel && i < LEVEL_SINGLE_PLAYER_COUNT; i++ ) {
					pData->aLevelProgress[i].Inventory = pOwner->Data.aLevelProgress[i].Inventory;
				}
				DEVPRINTF( "Co-op save: player %u (%ls) catches up from level %u to %u.\n", p + 1, pwszName,
					(u32)pData->nCurrentLevel, (u32)pOwner->Data.nCurrentLevel );
			} else {
				DEVPRINTF( "Co-op save: player %u (%ls) continues with their own inventory.\n", p + 1, pwszName );
			}
		}
		if( p > 0 ) {
			const u32 nCampaignFlags = GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER | GAMESAVE_PROFILE_FLAGS_100_PERCENT_FINISHED;
			pData->nCurrentLevel = pOwner->Data.nCurrentLevel;
			pData->nDifficulty = pOwner->Data.nDifficulty;
			pData->nFlags = (pData->nFlags & ~nCampaignFlags) | (pOwner->Data.nFlags & nCampaignFlags);
		}

		// the session profile: the record's progress with the player's own settings
		CPlayerProfile *pSession = papSession[p];
		const u32 nSettingFlags = GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG | GAMESAVE_PROFILE_FLAGS_AUTO_CENTER |
			GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING | GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT;
		const GameSave_ProfileData_t Personal = pPersonal->m_Data;
		pSession->m_Data = *pData;
		pSession->m_Data.nFlags = (pData->nFlags & ~nSettingFlags) | (Personal.nFlags & nSettingFlags) | GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE;
		pSession->m_Data.nControllerConfigIndex = Personal.nControllerConfigIndex;
		pSession->m_Data.nColorIndex = Personal.nColorIndex;
		pSession->m_Data.fUnitVibrationIntensity = Personal.fUnitVibrationIntensity;
		// Co-op records distinguish players even when they chose the same personal
		// profile. Keep their co-op sensitivity instead of resetting it every boot.
		pSession->m_Data.fUnitLookSensitivity = pRecord->nLastSave &&
			pData->fUnitLookSensitivity >= 0.0f && pData->fUnitLookSensitivity <= 1.0f ?
			pData->fUnitLookSensitivity : Personal.fUnitLookSensitivity;
		pSession->m_Data.fUnitSoundLevel = Personal.fUnitSoundLevel;
		pSession->m_Data.fUnitMusicLevel = Personal.fUnitMusicLevel;
		wchar wszName[FSTORAGE_MAX_NAME_LEN];
		fclib_wcsncpy( wszName, pwszName, FSTORAGE_MAX_NAME_LEN - 1 );
		wszName[FSTORAGE_MAX_NAME_LEN - 1] = 0;
		fang_MemZero( &pSession->m_SaveInfo, sizeof( GameSave_SaveInfo_t ) );
		pSession->m_SaveInfo.nStorageDeviceID = FSTORAGE_DEVICE_ID_NONE;
		fclib_wcsncpy( pSession->m_SaveInfo.wszProfileName, wszName, FSTORAGE_MAX_NAME_LEN - 1 );

		_apCoopSessionRecord[p] = pRecord;
		_apCoopSessionProfile[p] = pSession;
	}

	_nCoopSessionPlayers = nPlayers;
	_bCoopSession = TRUE;
}

BOOL coopsave_SaveLookSensitivity( const CPlayerProfile *pSession ) {
	if( !_bCoopSession || !_bCoopCanSave || !pSession ) return FALSE;
	for( u32 p=0; p<_nCoopSessionPlayers; ++p ) {
		if( _apCoopSessionProfile[p] != pSession ) continue;
		_CoopRecord_t *pRecord = _apCoopSessionRecord[p];
		// Players who joined without a saved profile are intentionally temporary.
		if( pRecord == &_aCoopScratch[p] ) return TRUE;
		const f32 fSensitivity = pSession->m_Data.fUnitLookSensitivity;
		if( !(fSensitivity >= 0.0f && fSensitivity <= 1.0f) ) return FALSE;
		if( !pRecord->nLastSave || pRecord->Data.fUnitLookSensitivity != fSensitivity ) {
			pRecord->Data.fUnitLookSensitivity = fSensitivity;
			pRecord->nLastSave = ++_CoopFile.nSaveCount;
			_bCoopSensitivityDirty = TRUE;
		}
		if( !_bCoopSensitivityDirty ) return TRUE;
		_CoopFile.nCRC = _CoopFileCRC();
		if( !fstorage_PcCoopWrite( _wszCoopOwner, &_CoopFile, sizeof( _CoopFile_t ) ) ) {
			DEVPRINTF( "Co-op settings: could not save look sensitivity for '%ls'.\n", _wszCoopOwner );
			return FALSE;
		}
		_bCoopSensitivityDirty = FALSE;
		DEVPRINTF( "Co-op settings: saved player %u look sensitivity %.3f.\n", p+1, fSensitivity );
		return TRUE;
	}
	return FALSE;
}

BOOL coopsave_SaveSession( void ) {
	u32 p;

	if( !_bCoopSession ) {
		return FALSE;
	}

	_CoopFile.nSaveCount++;
	for( p=0; p < _nCoopSessionPlayers; p++ ) {
		_CoopRecord_t *pRecord = _apCoopSessionRecord[p];
		pRecord->Data = _apCoopSessionProfile[p]->m_Data;
		pRecord->Data.nFlags &= ~GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE;
		pRecord->nLastSave = _CoopFile.nSaveCount;
	}

	if( !_bCoopCanSave ) {
		return TRUE;
	}

	_CoopFile.nCRC = _CoopFileCRC();
	if( !fstorage_PcCoopWrite( _wszCoopOwner, &_CoopFile, sizeof( _CoopFile_t ) ) ) {
		DEVPRINTF( "Co-op save: could not write '%ls'.\n", _wszCoopOwner );
		return FALSE;
	}
	DEVPRINTF( "Co-op save: saved '%ls' at level %u for %u players.\n", _wszCoopOwner, (u32)_CoopFile.aRecords[0].Data.nCurrentLevel, _nCoopSessionPlayers );
	_bCoopSensitivityDirty = FALSE;
	return TRUE;
}

void coopsave_EndSession( void ) {
	_bCoopSession = FALSE;
	_bCoopSensitivityDirty = FALSE;
	_nCoopSessionPlayers = 0;
}

BOOL coopsave_IsSessionActive( void ) {
	return _bCoopSession;
}
#endif




