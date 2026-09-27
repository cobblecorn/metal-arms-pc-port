//////////////////////////////////////////////////////////////////////////////////////
// Launcher.cpp - 
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
// 01/22/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "launcher.h"
#include "PickLevel.h"
#include "fversion.h"
#include "fmath.h"
#include "gamepad.h"
#include "gameloop.h"
#include "fviewport.h"
#include "ftext.h"
#include "game.h"
#include "level.h"
#include "player.h"
#include "gamesave.h"
#include "fperf.h"
#include "wpr_system.h"
#include "e3menu.h"
#include "wpr_languageselect.h"

//====================
// private definitions

typedef enum {
	_DEV_MENU = 0,
	_RETAIL_MENUS,
#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
	_E3_MENUS,
#endif

	_SELECTION_COUNT
} _Selections_e;

typedef enum {
	_MENU_TYPE_BOOTUP = 0,
	_MENU_TYPE_PICKLEVEL,
	_MENU_TYPE_E3,
	_MENU_TYPE_WRAPPERS,
	_MENU_TYPE_LANGUAGESELECT,

	_MENU_TYPE_COUNT
} _MenuType_e;

static cchar *_pszSelectedTextBlinkAndColor = "~B5~C90502099";
static cchar *_pszUnSelectedTextBlinkAndColor = "~B0~C30303085";

//=================
// public variables

extern BOOL Launcher_bDemoLaunched = FALSE;


//==================
// private variables

static u8 _nCurState;// one of LauncherFrom_e
static s8 _nCurSelection;// one of _Selections_e values
static BOOL8 _bSystemOK = FALSE;
static BOOL8 _bLoadTestMode = FALSE;
static GameInitInfo_t *_pCurrentGameInit = NULL;

static GamepadMap_e _anOldMappings[GAMEPAD_MAX_PORT_COUNT];
static BOOL8 _bMenuControllerMappingSet = FALSE;
static BOOL8 _bWrapperBootupParameter;

// Desktop port: development builds boot into the retail front end unless this is set (-dev-menu).
BOOL Launcher_bShowDevBootMenu = FALSE;
static BOOL8 _bLaunchRealWrappersFromLoadingScreen;
static u8 _nFrameCounter;

static wchar _wszLoadHeading[64];

#if !FANG_PRODUCTION_BUILD
	static BOOL8 _bNoHudCodeEntered;
	static BOOL8 _bFixed30FPSEntered;
	static BOOL8 _bCameraPOVEntered;

	static BOOL8 _bNoHudCodeEntered_Latch;
	static BOOL8 _bFixed30FPSEntered_Latch;
	static BOOL8 _bCameraPOVEntered_Latch;
#endif

static FResFrame_t _ResFrameBootupMenu;

//===================
// private prototypes

static BOOL _InitBootupMenu( void );
static BOOL _BootupWork( void );
static BOOL _BootupDraw( void );
static void _BootupCleanup( void );
static void _SetControllerMapping( BOOL bMenuMapping );
static BOOL _InitLoadNextSinglePlayerLevel( void ); 
static BOOL _ReloadCurrentLevel( void );

static BOOL _StartQuickSkipLevel( BOOL bLoadTestMode );
static BOOL _StartPickLevel( BOOL bLoadTestMode );
static BOOL _StartE3Level( void );
static BOOL _StartWrapperLevel( BOOL bLoadTestMode, BOOL bPlayLoadScreen );
static cchar *_FindLevelNameFromID( u32 nLevelID, u32 *pnLevelNum=NULL );

static void _ScheduleMenu( _MenuType_e nMenuType, BOOL bFromBootup=FALSE );

static BOOL _LoadFailureWork( void );
static BOOL _LoadFailureDraw( void );

static BOOL _WrapperLoadWork( void );
static BOOL _WrapperLoadDraw( void );

//=================
// public functions

BOOL launcher_InitSystem( void ) {
	
	FASSERT( !_bSystemOK );

	_nCurState = LAUNCHER_FROM_BOOTUP;
	_bLoadTestMode = FALSE;
	_nCurSelection = 0;
	_bLoadTestMode = FALSE;
	_pCurrentGameInit = NULL;
	_bMenuControllerMappingSet = FALSE;
	_bSystemOK = TRUE;

	return TRUE;
}

void launcher_UninitSystem( void ) {
	
	if( _bSystemOK ) {
		_bSystemOK = FALSE;
	}
}

BOOL launcher_EnterMenus( LauncherFrom_e nFrom ) {

	FASSERT( _bSystemOK );
	FASSERT( nFrom < LAUNCHER_FROM_COUNT );

	switch( nFrom ) {

	case LAUNCHER_FROM_BOOTUP:
#if LAUNCHER_GO_DIRECTLY_TO_RETAIL_WRAPPERS
		// go directly to the wrappers
		//_ScheduleMenu( _MENU_TYPE_WRAPPERS, TRUE );
		_ScheduleMenu( _MENU_TYPE_LANGUAGESELECT, TRUE );
#elif LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
		// load the game fonts...
		if( !game_LoadLocalizedPhraseTableAndFonts() ) {
			return FALSE;
		}
		if( !game_InitLocalizedResources() ) {
			return FALSE;
		}
		// go directly to the e3 menu (it will handle the demo version too)
		_ScheduleMenu( _MENU_TYPE_E3 );
#else
	#if defined(MA_PC_INPUT)
		if( !Launcher_bShowDevBootMenu ) {
			// As the shipped game does: straight to the retail front end.
			_ScheduleMenu( _MENU_TYPE_LANGUAGESELECT, TRUE );
			break;
		}
	#endif
		// development mode, offer the choice of boot modes
		_nCurState = nFrom;
		_ScheduleMenu( _MENU_TYPE_BOOTUP );
#endif
		break;

	case LAUNCHER_FROM_GAME:
		// go back to whatever menu was used to lauch the game
		switch( _nCurState ) {

		case LAUNCHER_FROM_QUICK_SKIP:
			// since the passed in level finished, go ahead and start the pick level menu
		case LAUNCHER_FROM_PICK_LEVEL:
			// return to the picklevel menu
			_ScheduleMenu( _MENU_TYPE_PICKLEVEL );
			break;
		case LAUNCHER_FROM_E3:
			// return to the E3 menu
			_ScheduleMenu( _MENU_TYPE_E3 );
			break;
		case LAUNCHER_FROM_WRAPPERS:
			// return to the wrappers
			_ScheduleMenu( _MENU_TYPE_WRAPPERS, FALSE );
			break;
		default:
			// we should never get here
			FASSERT_NOW;
			return FALSE;
		}
		break;

	case LAUNCHER_FROM_STARTUP_OPTIONS:
		_ScheduleMenu( _MENU_TYPE_WRAPPERS, TRUE );
		break;

	default:
	case LAUNCHER_FROM_PICK_LEVEL:		
	case LAUNCHER_FROM_WRAPPERS:
		// should never happen, this wouldn't make sense
		FASSERT_NOW;
		return FALSE;
		break;	
	}	

	return TRUE;
}

BOOL launcher_StartGame( LauncherFrom_e nFrom, BOOL bLoadTestMode/*=FALSE*/ ) {
	
	FASSERT( _bSystemOK );

	switch( nFrom ) {
	
	case LAUNCHER_FROM_QUICK_SKIP:
		// start a level directly (only really used under the windows build)
		_nCurState = nFrom;
		_SetControllerMapping( FALSE );
		return _StartQuickSkipLevel( FALSE );
		break;

	case LAUNCHER_FROM_PICK_LEVEL:
		_nCurState = nFrom;
		// restore the controller mappings
		_SetControllerMapping( FALSE );
		return _StartPickLevel( bLoadTestMode );
		break;

	case LAUNCHER_FROM_E3:
		_nCurState = nFrom;
		// restore the controller mappings
		_SetControllerMapping( FALSE );
		if( !_StartE3Level() ) {
			// couldn't load a level, go to the load failure message, but don't let the caller know that we failed (we will handle the error here)
			launcher_LoadFailure();			
		}
		return TRUE;
		break;
		
	case LAUNCHER_FROM_WRAPPERS:
		_nCurState = nFrom;
		// restore the controller mappings
		_SetControllerMapping( FALSE );
		_pCurrentGameInit = (GameInitInfo_t *)wpr_system_GetInitInfo();
		if( !_StartWrapperLevel( bLoadTestMode, TRUE ) ) {
			// couldn't load a level, go to the load failure message, but don't let the caller know that we failed (we will handle the error here)
			launcher_LoadFailure();			
		}
		return TRUE;
		break;


	case LAUNCHER_FROM_STARTUP_OPTIONS:
	default:
		FASSERT_NOW;
		break;
	}

	return FALSE;
}

// called from the game's work function to let us
// know that the current level's goals have been 
// met and we should proceed to the next level
void launcher_EndOfGameDecisions( LauncherDecision_e nDecision ) {

	FASSERT( _bSystemOK );
	FASSERT( nDecision < LAUNCHER_DECISION_COUNT );

	switch( _nCurState ) {

	default:
	case LAUNCHER_FROM_GAME:
	case LAUNCHER_FROM_BOOTUP:
		// this doesn't make sense at all
		FASSERT_NOW;
		break;

	case LAUNCHER_FROM_PICK_LEVEL:
	case LAUNCHER_FROM_QUICK_SKIP:
		// since this isn't really playing the game, go back to the pick level screen
		_ScheduleMenu( _MENU_TYPE_PICKLEVEL );
		break;

	case LAUNCHER_FROM_E3:
		// return to the e3 menu
		_ScheduleMenu( _MENU_TYPE_E3 );
		break;

	case LAUNCHER_FROM_WRAPPERS:
		// this is where we have some decisions to make
		if( nDecision == LAUNCHER_DECISION_LOAD_NEXT ) {
			// move ahead to the next level
			if( _pCurrentGameInit->bSinglePlayer ) {
				// single player mode
							
				// this is the last level in the series
				if( _pCurrentGameInit->bReplayingALevel ||
					_pCurrentGameInit->nLevelToPlay == (LEVEL_SINGLE_PLAYER_COUNT-1) ) {
					// we finished replaying a level or finished the game

					// record the stats and return to the wrappers
					_ScheduleMenu( _MENU_TYPE_WRAPPERS, FALSE );
				} else {
					// we need to load the next level in the chain
					gameloop_SetLoopHandlers( NULL, NULL, _InitLoadNextSinglePlayerLevel );					
				}
			} else {
				// multiplayer mode
				_ScheduleMenu( _MENU_TYPE_WRAPPERS, FALSE );
			}
		} else {
			gameloop_SetLoopHandlers( NULL, NULL, _ReloadCurrentLevel );
		}
		break;
	}
}

BOOL launcher_IsInLoadTestMode( void ) {
	return _bLoadTestMode;
}

BOOL launcher_LaunchedFromWrappers( void ) {
	if( _nCurState == LAUNCHER_FROM_WRAPPERS ) {
		return TRUE;
	}
	return FALSE;
}

void launcher_LoadFailure( void ) {
	// it will never leave this state
	DEVPRINTF( "launcher_LoadFailure() : There was a load failure, entering the failure screen.\n" );
    gameloop_SetLoopHandlers( _LoadFailureWork, _LoadFailureDraw, NULL );
}

//==================
// private functions

static BOOL _InitBootupMenu( void ) {
#if !FANG_PRODUCTION_BUILD	
	_bNoHudCodeEntered = FALSE;
	_bFixed30FPSEntered = FALSE;
	_bCameraPOVEntered = FALSE;

	_bNoHudCodeEntered_Latch = FALSE;
	_bFixed30FPSEntered_Latch = FALSE;
	_bCameraPOVEntered_Latch = FALSE;
#endif

	_ResFrameBootupMenu = fres_GetFrame();

	// Load the needed fonts...
	if( FTEXT_ERROR == ftext_Load( '1', "tfharialw$" ) ) {
		DEVPRINTF( "Could not load the \"tfharialw$\" font.", "Launcher Bootup Menu: Text System Problem" );
		goto _ExitWithError;
	}
	if( FTEXT_ERROR == ftext_Load( '3', "tfhtych26$" ) ) {
		DEVPRINTF( "Could not load the \"tfhtych26$\" font.", "Launcher Bootup Menu: Text System Problem" );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:

	fres_ReleaseFrame( _ResFrameBootupMenu );
	return FALSE;
}

static void _BootupCleanup( void ) {

	// Release the frame that has the fonts loaded...
	fres_ReleaseFrame( _ResFrameBootupMenu );
}

static BOOL _BootupWork( void ) {

	gamepad_Sample();
#if !FANG_PRODUCTION_BUILD
	// see if the user wants to turn off the hud and overlaid debug graphics
	if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState > 0.1 &&
		Gamepad_aapSample[0][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f &&
		Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_SIDE]->uLatches & FPAD_LATCH_ON ) {

		if( !_bNoHudCodeEntered_Latch ) {
			// the code has been entered
			_bNoHudCodeEntered ^= TRUE;
			_bNoHudCodeEntered_Latch = TRUE;
		}
	} else {
		_bNoHudCodeEntered_Latch = FALSE;
	}

	// see if user wants to lock the frame rate at 30 FPS (usually for capturing frames)
	if( Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_SIDE]->uLatches & FPAD_LATCH_ON &&
		Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_TOP]->uLatches & FPAD_LATCH_ON &&
		Gamepad_aapSample[0][GAMEPAD_MENU_BACK]->uLatches & FPAD_LATCH_ON ) {

		if( !_bFixed30FPSEntered_Latch ) {
			// the code has been entered
			_bFixed30FPSEntered ^= TRUE;
			_bFixed30FPSEntered_Latch = TRUE;
		}
	} else {
		_bFixed30FPSEntered_Latch = FALSE;
	}

	// see if the user wants to tie debug camera POV to the camera or glitch's camera
	if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState > 0.1 &&
		Gamepad_aapSample[0][GAMEPAD_MENU_RIGHT_ANALOG_Y]->fCurrentState > 0.1f &&
		Gamepad_aapSample[0][GAMEPAD_MENU_DPAD_Y]->fCurrentState < -0.1f ) {

		if( !_bCameraPOVEntered_Latch ) {
			// the code has been entered
			_bCameraPOVEntered ^= TRUE;
			_bCameraPOVEntered_Latch = TRUE;
		}
	} else {
		_bCameraPOVEntered_Latch = FALSE;
	}
#endif
	// see if a selection has been made
	if( Gamepad_aapSample[0][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		// a selection has been made

		// put a 1/30 sec delay to ensure that the controller values will be updated
		CFTimer Timer;
		Timer.Reset();
		while( Timer.SampleSeconds() < 0.30f ) {
			fmath_SetRandomSeed( Timer.SampleTicks() );
			fmath_RandomFloat();
		}
#if !FANG_PRODUCTION_BUILD
		// apply the codes now
		if( _bNoHudCodeEntered ) {
			// turn off overlaid graphics
			gameloop_EnableOverlayDrawElements( FALSE, FALSE );
		}
		if( _bFixed30FPSEntered ) {
			// fix the FPS at 30
			gameloop_SetAnimPlaybackRate( GAMELOOP_ANIM_PLAYBACK_30FPS );
		}
		if( _bCameraPOVEntered ) {
			Game_bCameraPOVVisibility = TRUE;
		}
#endif
		// boot to the proper mode
		switch( _nCurSelection ) {
		case _DEV_MENU:

			// Cleanup the boot menu fonts
			_BootupCleanup();

			// launch into development bootup mode
			// load the game fonts...

			if( !game_LoadLocalizedPhraseTableAndFonts() ) {
				return FALSE;
			}
			if( !game_InitLocalizedResources() ) {
				return FALSE;
			}
			_ScheduleMenu( _MENU_TYPE_PICKLEVEL );
			break;
		case _RETAIL_MENUS:
			// launch the retail bootup mode

			// Cleanup the boot menu fonts
			_BootupCleanup();

//			_ScheduleMenu( _MENU_TYPE_WRAPPERS, TRUE );
			_ScheduleMenu( _MENU_TYPE_LANGUAGESELECT, TRUE );
			break;
#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
		case _E3_MENUS:

			// Cleanup the boot menu fonts
			_BootupCleanup();

			// launch into development bootup mode
			// load the game fonts...
			if( !game_LoadLocalizedPhraseTableAndFonts() ) {
				return FALSE;
			}
			if( !game_InitLocalizedResources() ) {
				return FALSE;
			}

			// launch into the E3 bootup mode
			_ScheduleMenu( _MENU_TYPE_E3 );
			break;
#endif
		}
		return TRUE;
	}	

	// see if we should change the current selection (up/down)
	if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_ANALOG_Y]->uLatches &
		FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_ANALOG_Y]->fCurrentState > 0.1f ) {
			_nCurSelection--;			
		} else {
			_nCurSelection++;
		}
	} else if( Gamepad_aapSample[0][GAMEPAD_MENU_DPAD_Y]->uLatches &
			   FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
		if( Gamepad_aapSample[0][GAMEPAD_MENU_DPAD_Y]->fCurrentState > 0.1f ) {
			_nCurSelection--;
		} else {
			_nCurSelection++;
		}
	}
	FMATH_CLAMP( _nCurSelection, 0, (_SELECTION_COUNT-1) );

	return TRUE;
}

static BOOL _BootupDraw( void ) {

	fviewport_SetActive( NULL );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.03f, 1.0f, 0 );

	// draw the heading
	ftext_Printf( 0.5f, 0.075f, "~f3~w1~ac~C90502099~s1.50Metal Arms" ); 
	ftext_Printf( 0.5f, 0.115f, "~f3~w1~ac~C30207599~s0.90Glitch in the System" );
	ftext_Printf( 0.5f, 0.25f, "~f3~w1~ac~s1.25Choose Bootup Mode" );
	ftext_Printf( 0.5f, 0.28f, "~f1~w0~ac~s1.25~C30207599--------------------------------------------------------" );

	// draw the selections			
	ftext_Printf( 0.5f, 0.35f, "%s~f1~w0~ac~s1.35%s", 
		(_nCurSelection == 0) ? _pszSelectedTextBlinkAndColor : _pszUnSelectedTextBlinkAndColor, 
		"Development Mode" );
	ftext_Printf( 0.5f, 0.42f, "%s~f1~w0~ac~s1.35%s", 
		(_nCurSelection == 1) ? _pszSelectedTextBlinkAndColor : _pszUnSelectedTextBlinkAndColor,
		"Retail Mode" );
#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
	if( Launcher_bDemoLaunched ) {
		ftext_Printf( 0.5f, 0.49f, "%s~f1~w0~ac~s1.35%s", 
			(_nCurSelection == 2) ? _pszSelectedTextBlinkAndColor : _pszUnSelectedTextBlinkAndColor,
			"Demo Disk Mode" );	
	} else {
		ftext_Printf( 0.5f, 0.49f, "%s~f1~w0~ac~s1.35%s", 
			(_nCurSelection == 2) ? _pszSelectedTextBlinkAndColor : _pszUnSelectedTextBlinkAndColor,
			"E3 Mode" );
	}
#endif
	// draw the version number in the lower right corner of the screen
	ftext_Printf( 0.68f, 0.67f, "~f1~C92929299~w1~al~s0.69Ver %d.%d.%d", fversion_GetToolMajorVer(), 
																		 fversion_GetToolMinorVer(),
																		 fversion_GetToolSubVer() );
#if !FANG_PRODUCTION_BUILD
	if( _bNoHudCodeEntered || _bFixed30FPSEntered || _bCameraPOVEntered ) {
		// a code has been entered, print which ones
		ftext_Printf( 0.08f, 0.62f, "~f1~w0~aL~s0.80~C30207599Codes Entered: %s, %s, %s", 
			_bNoHudCodeEntered ? "No Hud" : "--", 
			_bFixed30FPSEntered ? "Fixed 30 FPS" : "--",
			_bCameraPOVEntered ? "Camera POV" : "--" );
	}
#endif
	return TRUE;
}

static void _SetControllerMapping( BOOL bMenuMapping ) {
	u32 i;

	if( bMenuMapping ) {
		if( !_bMenuControllerMappingSet ) {
			for( i=0; i < GAMEPAD_MAX_PORT_COUNT; i++ ) {
				_anOldMappings[i] = gamepad_SetMapping( i, GAMEPAD_MAP_MENU );
			}
			_bMenuControllerMappingSet = TRUE;
		}
	} else {
		if( _bMenuControllerMappingSet ) {
			for( i=0; i < GAMEPAD_MAX_PORT_COUNT; i++ ) {
				gamepad_SetMapping( i, _anOldMappings[i] );
			}
			_bMenuControllerMappingSet = FALSE;
		}
	}
}

static BOOL _InitLoadNextSinglePlayerLevel( void ) {
	cchar *pszLevelName;

	FASSERT( _nCurState == LAUNCHER_FROM_WRAPPERS );

	gameloop_ShowFPS( FALSE );
	
	_pCurrentGameInit->nLevelToPlay++;

	pszLevelName = _FindLevelNameFromID( _pCurrentGameInit->nLevelToPlay );
	FASSERT( pszLevelName );

	// unload the current level
	game_UnloadLevel();
	
	_pCurrentGameInit->pwszLevelDisplayHeading = _wszLoadHeading;
	wpr_system_CreateLoadHeading( _pCurrentGameInit->nLevelToPlay, _wszLoadHeading, 64 );
	
	// load the selected level...
	if( !game_LoadLevel( pszLevelName, TRUE, _pCurrentGameInit ) ) {
		launcher_LoadFailure();
		return TRUE;//FALSE;
	}

	gameloop_ShowFPS( TRUE );
	
	return TRUE;
}

static BOOL _ReloadCurrentLevel( void ) {
	cchar *pszLevelName;

	FASSERT( _nCurState == LAUNCHER_FROM_WRAPPERS );

	gameloop_ShowFPS( FALSE );
	
	pszLevelName = _FindLevelNameFromID( _pCurrentGameInit->nLevelToPlay );
	FASSERT( pszLevelName );

	// unload the current level
	game_UnloadLevel();
	
	// load the selected level...
	if( !game_LoadLevel( pszLevelName, TRUE, _pCurrentGameInit ) ) {
		launcher_LoadFailure();
		return TRUE;//FALSE;
	}

	gameloop_ShowFPS( TRUE );
	if( Gameloop_bDrawDebugInfo ) {
		FPerf_nDisplayPerfType = FPERF_TYPE_FRAMERATE;
	}
	
	return TRUE;
}

static BOOL _StartQuickSkipLevel( BOOL bLoadTestMode ) {
	u32 i;
	cchar *pszLevelName;
	static GameInitInfo_t _CampaignCoopInit;

	_bLoadTestMode = FALSE;
	pszLevelName = gameloop_GetSkipLevelName();

	// since we are quick launching, NULL out the player profile array
	for( i=0; i < MAX_PLAYERS; i++ ) {
		playerprofile_Set( i, NULL );
	}
	
	if( gameloop_GetSkipLevelUseMissionData() ) {
		for( s32 index=0; index<Level_nCount; ++index ) {
			const Level_t &mission = Level_aInfo[index];
			if( mission.pszWorldResName && !fclib_stricmp( mission.pszWorldResName, pszLevelName ) &&
				mission.nLevel >= 0 && mission.nLevel < LEVEL_SINGLE_PLAYER_COUNT ) {
				DEVPRINTF( "Mission launch: %s, world=%s, data=%s, materials=%s.\n",
					mission.pszTitle, mission.pszWorldResName, mission.pszCSVFile, mission.pszMaterialCSVFile );
				const u8 nCampaignPlayers = gameloop_GetQuickLaunchCampaignPlayers();
				const GameInitInfo_t *pGameInit = NULL;
				if( nCampaignPlayers > 1 ) {
					// Experimental PC path: keep campaign rules and virtual/no-save profiles while the
					// existing per-player split-screen, HUD, and controller setup initializes each slot.
					memset( &_CampaignCoopInit, 0, sizeof( _CampaignCoopInit ) );
					_CampaignCoopInit.nNumPlayers = nCampaignPlayers;
					_CampaignCoopInit.bSinglePlayer = TRUE;
					_CampaignCoopInit.bNewGame = TRUE;
					_CampaignCoopInit.nLevelToPlay = (u8)mission.nLevel;
					_CampaignCoopInit.nDifficultyLevel = GAMESAVE_DIFFICULTY_NORMAL;
					pGameInit = &_CampaignCoopInit;
					DEVPRINTF( "Campaign co-op prototype: %u local players, campaign rules, no profile saves.\n",
						nCampaignPlayers );
				}
				if( !game_LoadLevel( mission.pszTitle, FALSE, pGameInit ) ) return FALSE;
				gameloop_ShowFPS( TRUE );
				return TRUE;
			}
		}
		DEVPRINTF( "Mission launch: no registered single-player mission for world '%s'.\n", pszLevelName );
		return FALSE;
	}

	// load the selected level...
	if( !game_LoadGenericDebugLevel( pszLevelName ) ) {
		return FALSE;
	}
	gameloop_ShowFPS( TRUE );

	return TRUE;
}

static BOOL _StartPickLevel( BOOL bLoadTestMode ) {
#if LAUNCHER_INCLUDE_DEV_MENU
	cchar *pszLevelName;

	_bLoadTestMode = bLoadTestMode;
	pszLevelName = picklevel_GetNameOfSelectedLevel();

	// end the picklevel system
	picklevel_End();

	picklevel_MeasureLoadTime( TRUE );

	// load the selected level...
	if( !game_LoadLevel( pszLevelName, FALSE, NULL ) ) {
		_SetControllerMapping( TRUE );
		return FALSE;
	}

	picklevel_MeasureLoadTime( FALSE );

	gameloop_ShowFPS( TRUE );
#endif	
	return TRUE;
}

static BOOL _StartE3Level( void ) {
#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
	cchar *pszLevelName;

	_bLoadTestMode = FALSE;
	pszLevelName = e3menu_GetNameOfSelectedLevel();
	_pCurrentGameInit = (GameInitInfo_t *)e3menu_GetInitInfo();
	_pCurrentGameInit->pwszLevelDisplayHeading = NULL;

	// end the e3 menu system
	e3menu_End();

	// load the selected level...
	if( !game_LoadLevel( pszLevelName, TRUE, _pCurrentGameInit ) ) {
		_SetControllerMapping( TRUE );
		return FALSE;
	}
	gameloop_ShowFPS( TRUE );
#endif
	return TRUE;
}

static BOOL _StartWrapperLevel( BOOL bLoadTestMode, BOOL bPlayLoadScreen ) {
	cchar *pszLevelName;	

	_bLoadTestMode = bLoadTestMode;

	// find the level to load
	pszLevelName = _FindLevelNameFromID( _pCurrentGameInit->nLevelToPlay );
	FASSERT( pszLevelName );
	_pCurrentGameInit->pwszLevelDisplayHeading = _wszLoadHeading;
	
	wpr_system_CreateLoadHeading( _pCurrentGameInit->nLevelToPlay, _wszLoadHeading, 64 );

	// end the wrapper system
	wpr_system_End();

	// load the selected level...
	if( !game_LoadLevel( pszLevelName, bPlayLoadScreen, _pCurrentGameInit ) ) {
		_SetControllerMapping( TRUE );
		return FALSE;
	}	
	gameloop_ShowFPS( TRUE );
	
	return TRUE;
}

static cchar *_FindLevelNameFromID( u32 nLevelID, u32 *pnLevelNum/*=NULL*/ ) {
	u32 i;

	FASSERT( Level_nCount > 0 );

	for( i=0; i < (u32)Level_nCount; i++ ) {
		if( Level_aInfo[i].nLevel == nLevelID ) {
			if( pnLevelNum ) {
				*pnLevelNum = i;
			}
			return Level_aInfo[i].pszTitle; 
		}
	}
	return NULL;
}

static void _ScheduleMenu( _MenuType_e nMenuType, BOOL bFromBootup/*=FALSE*/ ) {

	gameloop_ShowFPS( FALSE );
	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;

	// hookup the controls to the menu state
	// save the state of the controller mappings as we set the mapping to menu mode
	_SetControllerMapping( TRUE );

	switch( nMenuType ) {
		
	case _MENU_TYPE_BOOTUP:
		gameloop_SetLoopHandlers( _BootupWork, _BootupDraw, _InitBootupMenu );
		break;

	case _MENU_TYPE_PICKLEVEL:
#if LAUNCHER_INCLUDE_DEV_MENU	
		picklevel_Start();
#endif
		break;

	case _MENU_TYPE_E3:
#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS		
		// draw the loading... screen and then start up the real game wrappers
		_bWrapperBootupParameter = bFromBootup;
		_bLaunchRealWrappersFromLoadingScreen = FALSE;
		_nFrameCounter = 0;
		gameloop_SetLoopHandlers( _WrapperLoadWork, _WrapperLoadDraw, NULL );	
		break;
#endif
	case _MENU_TYPE_WRAPPERS:
		// draw the loading... screen and then start up the real game wrappers
		_bWrapperBootupParameter = bFromBootup;
		_bLaunchRealWrappersFromLoadingScreen = TRUE;
		_nFrameCounter = 0;
		gameloop_SetLoopHandlers( _WrapperLoadWork, _WrapperLoadDraw, NULL );	
		break;

	case _MENU_TYPE_LANGUAGESELECT:
		wpr_languageselect_Start();
		break;
	}
}

static BOOL _LoadFailureWork( void ) {
	return TRUE;
}

static BOOL _LoadFailureDraw( void ) {

	fviewport_SetActive( NULL );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.03f, 1.0f, 0 );

	if( Game_apwszPhrases[GAMEPHRASE_BAD_GAME_DISK] ) {
        wchar wszString[128];
		_snwprintf( wszString, 128, Game_apwszPhrases[GAMEPHRASE_BAD_GAME_DISK], '\n', '\n' );
		ftext_Printf( 0.5f, 0.35f, L"~C90502099~f1~w0~ac~s1.35%ls", wszString );
	} else {
		// the string isn't loaded hardcode one -- The fonts are not loaded either, so use the system font
		ftext_Printf( 0.5f, 0.35f, L"~C90502099~w0~ac~s1.35%ls", L"There is a problem with the\ndisc you are using.\nIt may be dirty or damaged." );
	}

	return TRUE;
}

static BOOL _WrapperLoadWork( void ) {
	if( _nFrameCounter == 0 ) {
		// dump the currently loaded game
		game_UnloadLevel();
	}

	_nFrameCounter++;

	if( _nFrameCounter > 2 ) {
		if( _bLaunchRealWrappersFromLoadingScreen ) {
            wpr_system_Start( _bWrapperBootupParameter );
		} else {
#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS
			e3menu_Start();
#endif
		}
	}

	return TRUE;
}

static BOOL _WrapperLoadDraw( void ) {
	
	launcher_DrawLoadingScreen();	

	return TRUE;
}

void launcher_DrawLoadingScreen( void ) {
	fviewport_SetActive( NULL );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.03f, 1.0f, 0 );

	launcher_DrawLoadingText();
}

void launcher_DrawLoadingText( void ) {
	if( Game_apwszPhrases[GAMEPHRASE_LOADING] ) {
		ftext_Printf( 0.10f, 0.65f, L"~C70707099~f1~w0~aL~s1.10%ls...", Game_apwszPhrases[GAMEPHRASE_LOADING] );
	} else {
		ftext_Printf( 0.10f, 0.65f, L"~C70707099~f1~w0~aL~s1.10Loading..." );
	}
}
