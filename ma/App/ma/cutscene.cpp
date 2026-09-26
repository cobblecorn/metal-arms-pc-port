//////////////////////////////////////////////////////////////////////////////////////
// cutscene.cpp - in-game cutscene movie player.
//
// Author: Russell Foushee     
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
// 01/15/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "cutscene.h"
#include "gameloop.h"
#include "floop.h"
#include "fmovie2.h"
#include "fviewport.h"
#include "FScriptSystem.h"
#include "faudio.h"
#include "gamepad.h"
#include "Player.h"
#include "fclib.h"
#include "game.h"

// cutscene Stats structure
#define _MAX_MOVIE_CHARS			( 32 )

typedef struct {
	char szFilename[ _MAX_MOVIE_CHARS ];
	u32 uNumTimesPlayed;
} _cutsceneStruct_t;


//private variables
static BOOL _bInitialized = FALSE;
static BOOL _bStarted;
static BOOL _bAllowEarlyTermination;
static BOOL _hCurrentCutscene;

//controller Mappings
static GamepadMap_e _nOldMappings;


static GameloopFcn_t *_pPreviousFcnWork;		// Work function (NULL=none)
static GameloopFcn_t *_pPreviousFcnDraw;		// Draw function (NULL=none)

//private prototypes
static BOOL _Work( void );
static BOOL _Draw( void );

BOOL cutscene_InitSystem( void ) {

	_bStarted = FALSE;
	_bInitialized = TRUE;
	_hCurrentCutscene = CUTSCENE_INVALID_HANDLE;

	return TRUE;
}

void cutscene_UninitSystem( void ) {
	cutscene_End();
	_bInitialized = FALSE;
}


cutscene_Handle_t cutscene_AcquireHandle( cchar* pszMovieFilename, BOOL bPrependPlatformPrefix ) {
	FASSERT( _bInitialized = TRUE );

	//first, create the full movie name
	char szFullFilename[ 64 ];
	szFullFilename[ 0 ] = NULL;
	if( bPrependPlatformPrefix ) {
#if FANG_PLATFORM_GC
		fclib_strcpy( szFullFilename, "GC_" );
#else
		fclib_strcpy( szFullFilename, "XB_" );
#endif
	}
	fclib_strcat( szFullFilename, pszMovieFilename );

	//next, determine if this filename even exists...
	if( !fmovie2_MovieExists( szFullFilename ) ) {
		DEVPRINTF("[ cutscene_AcquireHandle ] -- Movie %s does not exist", szFullFilename);
		return CUTSCENE_INVALID_HANDLE;
	}

	if( fclib_strlen( szFullFilename ) > 31 ) {
		DEVPRINTF("[ cutscene_AcquireHandle ] -- Movie name too large!");
		return CUTSCENE_INVALID_HANDLE;
	}

	//allocate a cutscene struct
	_cutsceneStruct_t *pStruct = ( _cutsceneStruct_t* ) fres_AllocAndZero( sizeof( _cutsceneStruct_t ) );
	if( !pStruct ) {
		DEVPRINTF("[ cutscene_AcquireHandle ] -- Could not allocate memory for a cutscene handle\n");
		return CUTSCENE_INVALID_HANDLE;
	}
	fclib_strcpy( pStruct->szFilename, szFullFilename );

	return ( cutscene_Handle_t ) pStruct;
}



BOOL cutscene_Start( cutscene_Handle_t hCutscene, f32 fNormalizedVolume ){

	FASSERT( _bInitialized = TRUE );
	FASSERT( _bStarted == FALSE );

	if( hCutscene == CUTSCENE_INVALID_HANDLE ) {
		DEVPRINTF("[ cutscene_Start ] -- Passed in an invalid cutscene handle!\n");
		return FALSE;
	}

	for( s32 i = 0; i < CPlayer::m_nPlayerCount; ++i ) {
		fcamera_GetCameraByIndex( i )->StunCamera( -1.0f );
	}

	_hCurrentCutscene = hCutscene;

	_cutsceneStruct_t *pCutsceneStruct = ( _cutsceneStruct_t* ) _hCurrentCutscene;

#if !FANG_PRODUCTION_BUILD
	_bAllowEarlyTermination = TRUE;
#else
	_bAllowEarlyTermination = ( pCutsceneStruct->uNumTimesPlayed > 0 || game_IsCurrentGameAReplay() );
#endif

	fmovie2_Play( pCutsceneStruct->szFilename, fNormalizedVolume );
	if( fmovie2_GetStatus() == FMOVIE2_STATUS_PLAYING ){
		//we have successfully loaded the movies, so lets hijack the
		//game loop while we play the movies

		pCutsceneStruct->uNumTimesPlayed++;

		//first, save off the previous loop handlers so we can restore them
		//at movie termination time.
		gameloop_GetLoopHandlers( &_pPreviousFcnWork, &_pPreviousFcnDraw, NULL );

		// set the gameloop handlers to be _Work() and _Draw()
		gameloop_SetLoopHandlers( _Work, _Draw, NULL );

		//Pause all the sounds and streams for the duration of the movie playing...
		CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_5 );
		CFAudioStream::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_5 );

		// save the state of the controller mappings as we set the mapping to menu mode
		_nOldMappings = gamepad_SetMapping( CPlayer::m_pCurrent->m_nControllerIndex, GAMEPAD_MAP_MENU );

		floop_PauseGame( TRUE );

		_bStarted = TRUE;
	}

	return TRUE;
}


void cutscene_End( void ){

	if( _bStarted ) {

		fmovie2_Unload(); //terminate any movie that may currently be playing

		//trigger a cutscene termination event here (if we need to)...
		s32 nMovieEventID = CFScriptSystem::GetEventNumFromName("movie");
		if( nMovieEventID == -1 ) {
			DEVPRINTF("[ cutscene_InitSystem ] -- NO MOVIE EVENT ID FOUND!!!\n");
		}
		if( ( nMovieEventID != -1 ) && ( _hCurrentCutscene != CUTSCENE_INVALID_HANDLE ) ) {
			CFScriptSystem::TriggerEvent( nMovieEventID, _hCurrentCutscene, 0, 0 );
		}

		//Unpause all the sounds and streams that may have been affected by the cutscene...
		CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_NONE );
		CFAudioStream::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_NONE );

		// restore the controller mappings
		gamepad_SetMapping( CPlayer::m_pCurrent->m_nControllerIndex, _nOldMappings );

		//restore the original loop handlers
		gameloop_SetLoopHandlers( _pPreviousFcnWork, _pPreviousFcnDraw, NULL );

		floop_PauseGame( FALSE );
		
		_hCurrentCutscene = CUTSCENE_INVALID_HANDLE;
		_bStarted = FALSE;
	}
}


static BOOL _Work( void ) {

	BOOL bTerminateCutscene = FALSE;

	if( fmovie2_GetStatus() != FMOVIE2_STATUS_PLAYING ) {
		//movie playing has terminated
		bTerminateCutscene = TRUE;
	}

	if( _bAllowEarlyTermination ) {
		//query the controller here for early termination issues...
		gamepad_Sample();

		if( Gamepad_aapSample[CPlayer::m_pCurrent->m_nControllerIndex][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			bTerminateCutscene = TRUE;
		}
		if( Gamepad_aapSample[CPlayer::m_pCurrent->m_nControllerIndex][GAMEPAD_MENU_START]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			bTerminateCutscene = TRUE;
		}
	}

	if( bTerminateCutscene ) {
		cutscene_End();
	}

	return TRUE;
}



static BOOL _Draw( void ) {

	fviewport_SetActive( NULL );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.00f, 1.0f, 0 );

	fmovie2_Draw();

	return TRUE;
}
