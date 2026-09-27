//////////////////////////////////////////////////////////////////////////////////////
// PickLevel.cpp - 
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
// 04/29/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "PickLevel.h"

#if LAUNCHER_INCLUDE_DEV_MENU

#include "level.h"
#include "fres.h"
#include "gameloop.h"
#include "gamepad.h"
#include "fviewport.h"
#include "ftext.h"
#include "game.h"
#include "fversion.h"
#include "faudio.h"
#include "fsndfx.h"
#include "floop.h"
#include "fperf.h"
#include "fclib.h"
#include "wpr_system.h"
#include "difficulty.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif


//====================
// private definitions

#define _MAX_ENTRIES_PER_COL		14

#define _OPTION_CHANGED_SOUND		"VolumeTest"

typedef enum { 
	_MODE_PICK_SINGLE_PLAYER_LEVEL = 0,
	_MODE_PICK_MULTI_PLAYER_LEVEL,
	_MODE_LEVEL_LOAD_ERROR,

	_MODE_COUNT
} _Modes_e;

typedef struct {
	u32 nNumEntries;
	Level_t *pFirstEntry;
	cchar *pszDisplayTitle;
} _MenuItem_t;

//=================
// public variables

//==================
// private variables

static BOOL8 _bSystemInitialized = FALSE;
static BOOL8 _bNeedToUnload;
static u8 _nCurrentMode;
static u8 _nModeBeforeError;
static s8 _nCurSubItem;
static u8 _nLoadTestMode;// 0 is off, otherwise we in test mode and counting frames till we auto select
static BOOL8 _bLoadLevelNextFrame;
static BOOL8 _bPrevGovernorState;
static BOOL8 _bStressTest;
static u8 _nStressTestStartIndex;
static s8 _nCurSelection;
static s8 _nNumSinglePlayerSelections;
static s8 _nNumMultiPlayerSelections;
static cchar *_pszLevelToLoad;
static FResFrame_t _ResFrame;
static FMemFrame_t _MemFrame;
static CFAudioStream *_pAudioStream;
static FSndFx_FxHandle_t _hClick;
static FSndFx_FxHandle_t _hSelected;
static _MenuItem_t *_paSinglePlayerMenuItems;// points to _nNumSinglePlayerSelections
static _MenuItem_t *_paMultiPlayerMenuItems;// points to _nNumMultiPlayerSelections
static f32 _fLastLoadSecs;
static f32 _fLongestLoadSecs;
static f32 _fAverageLoadSecs;
static f32 _fNumLoads;
static f32 _fTotalLoadSecs;
static cchar *_pszLongestLevelLoad;
static u32 _nDifficultyLevel;
static FSndFx_FxHandle_t _hSoundDevOptionChanged;

//===================
// private prototypes

static BOOL _InitPickLevel( void );
static BOOL _Work( void );
static BOOL _PickLevelWork( void );
static BOOL _Draw( void );
static void _ResetVars( void );
static void _StartLoadTestMode( void );

//=================
// public functions

BOOL picklevel_InitSystem( void ) {
	_bSystemInitialized = TRUE;

	_ResetVars();	// Compiler Bug Warning: Note that _ResetVars() must be called after the _bSystemInitialized line above or else the compiler won't call _ResetVars()

	return TRUE;
}

void picklevel_UninitSystem( void ) {

	if( _bSystemInitialized ) {
		picklevel_End();
	}
	_ResetVars();

	_bSystemInitialized = FALSE;
}

BOOL picklevel_Start( void ) {
	FASSERT( _bSystemInitialized );

	_pszLevelToLoad = NULL;
	_paSinglePlayerMenuItems = NULL;
	_paMultiPlayerMenuItems = NULL;
	_bLoadLevelNextFrame = FALSE;

	gameloop_SetLoopHandlers( _Work, _Draw, _InitPickLevel );
	
	return TRUE;
}

void picklevel_End( void ) {
	FASSERT( _bSystemInitialized );

	if( !_bNeedToUnload ) {
		return;
	}
	_bNeedToUnload = FALSE;

	fres_ReleaseFrame( _ResFrame );
	fmem_ReleaseFrame( _MemFrame );

	if( _pAudioStream ) {
		_pAudioStream->Destroy();
		_pAudioStream = NULL;
	}
	_paSinglePlayerMenuItems = NULL;
	_paMultiPlayerMenuItems = NULL;

	floop_EnableGovernor( _bPrevGovernorState );
}

cchar *picklevel_GetNameOfSelectedLevel( void ) {
	FASSERT( _bSystemInitialized );

	return _pszLevelToLoad;
}

void picklevel_MeasureLoadTime( BOOL bStart ) {
	
	if( bStart ) {
		ftimer_Clock_Reset();
	} else {
		_fLastLoadSecs = ftimer_Clock_GetSeconds();
		if( _fLastLoadSecs > _fLongestLoadSecs ) {
			_fLongestLoadSecs = _fLastLoadSecs;
			_pszLongestLevelLoad = _pszLevelToLoad;
		}
		_fNumLoads += 1.0f;
		_fTotalLoadSecs += _fLastLoadSecs;
		_fAverageLoadSecs = fmath_Div( _fTotalLoadSecs, _fNumLoads );
	}
}

//==================
// private functions

static BOOL _InitPickLevel( void ) {

	_pAudioStream = NULL;
	_hClick = fsndfx_GetFxHandle( "Hud Click" );
	_hSelected = fsndfx_GetFxHandle( "Hud Select" );
	_hSoundDevOptionChanged = fsndfx_GetFxHandle( _OPTION_CHANGED_SOUND );

	// Init difficulty only if this is the first time this function has been called...
	if( _nDifficultyLevel == CDifficulty::DIFFICULTY_COUNT ) {
		_nDifficultyLevel = CDifficulty::DEFAULT_DIFFICULTY_LEVEL;
	}

	// clear out any level that may be loaded
	game_UnloadLevel();

	// grab memory frames...
	_ResFrame = fres_GetFrame();
	_MemFrame = fmem_GetFrame();

	_bNeedToUnload = TRUE;

	// allocate an array to hold all of the different levels
	_paSinglePlayerMenuItems = (_MenuItem_t *)fres_AllocAndZero( sizeof( _MenuItem_t ) * Level_nCount-1 );
	if( !_paSinglePlayerMenuItems ) {
		// should never happen
		FASSERT_NOW;
		return FALSE;
	}
	// fill in the allocated table
	s32 i;
	u32 nIndex;
	cchar *pszTitle;
	_nNumSinglePlayerSelections = 0;
	_nNumMultiPlayerSelections = 0;
	for( i=1; i < Level_nCount; i++ ) {
		if( Level_aInfo[i].nLevel < LEVEL_SINGLE_PLAYER_COUNT ) {
			FASSERT( _nNumMultiPlayerSelections == 0 );// don't intermix the multi and single player levels
			// put this level under the single player menu
			if( _nNumSinglePlayerSelections == 0 ) {
				// create an entry to start
				_paSinglePlayerMenuItems[_nNumSinglePlayerSelections].nNumEntries = 1;
				_paSinglePlayerMenuItems[_nNumSinglePlayerSelections].pFirstEntry = &Level_aInfo[i];
				pszTitle = fclib_strchr( Level_aInfo[i].pszTitle, ' ' );
				if( !pszTitle ) {
					_paSinglePlayerMenuItems[_nNumSinglePlayerSelections].pszDisplayTitle = Level_aInfo[i].pszTitle;
				} else {
					_paSinglePlayerMenuItems[_nNumSinglePlayerSelections].pszDisplayTitle = ++pszTitle;
				}
				_nNumSinglePlayerSelections++;
			} else {
				// see if the title is that same as the current one
				nIndex = _nNumSinglePlayerSelections - 1;
				pszTitle = fclib_strchr( Level_aInfo[i].pszTitle, ' ' );
				if( !pszTitle ){
					pszTitle = Level_aInfo[i].pszTitle;
				} else {
					pszTitle++;
				}
				if( fclib_stricmp( pszTitle, _paSinglePlayerMenuItems[nIndex].pszDisplayTitle ) == 0 ) {
					// this is a sub level of the current selection
					_paSinglePlayerMenuItems[nIndex].nNumEntries++;
				} else {
					// create a new entry
					_paSinglePlayerMenuItems[_nNumSinglePlayerSelections].nNumEntries = 1;
					_paSinglePlayerMenuItems[_nNumSinglePlayerSelections].pFirstEntry = &Level_aInfo[i];
					_paSinglePlayerMenuItems[_nNumSinglePlayerSelections].pszDisplayTitle = pszTitle;
					_nNumSinglePlayerSelections++;
				}
			}
		} else {
			if( !_nNumMultiPlayerSelections ) {
				// start the multiplayer list here (at the end of the single player list)
				_paMultiPlayerMenuItems = &_paSinglePlayerMenuItems[_nNumSinglePlayerSelections];
			}
            _paMultiPlayerMenuItems[_nNumMultiPlayerSelections].nNumEntries = 1;
			_paMultiPlayerMenuItems[_nNumMultiPlayerSelections].pFirstEntry = &Level_aInfo[i];
			pszTitle = fclib_strchr( Level_aInfo[i].pszTitle, ' ' );
			if( !pszTitle ) {
				_paMultiPlayerMenuItems[_nNumMultiPlayerSelections].pszDisplayTitle = Level_aInfo[i].pszTitle;
			} else {
				_paMultiPlayerMenuItems[_nNumMultiPlayerSelections].pszDisplayTitle = ++pszTitle;
			}
			_nNumMultiPlayerSelections++;
		}
	}
	FASSERT( _nNumSinglePlayerSelections );

    if( _nLoadTestMode ) {
		// we are in the load test mode
		_nLoadTestMode = 3;// to allow 1 frame to be drawn before auto selecting the next level

		if( !_bStressTest ) {
			// not a load stress test, advance to the next sub item and then to the next items
			if( _nCurrentMode == _MODE_PICK_SINGLE_PLAYER_LEVEL ) {
				_nCurSubItem++;
				if( _nCurSubItem >= (s8)_paSinglePlayerMenuItems[_nCurSelection].nNumEntries ) {
					// move to the next selection
					_nCurSelection++;
					_nCurSubItem = 0;
					if( _nCurSelection >= _nNumSinglePlayerSelections ) {
						// end the load test mode
						_nCurSelection = 0;
						_nLoadTestMode = 0;
					}
				}
			} else {
				// move to the next selection
				_nCurSelection++;
				_nCurSubItem = 0;
				if( _nCurSelection >= _nNumMultiPlayerSelections ) {
					// end the load test mode
					_nCurSelection = 0;
					_nLoadTestMode = 0;
				}
			}
		} else {
			// in stress tests, just randomly pick selections...forever
			if( _nCurrentMode == _MODE_PICK_SINGLE_PLAYER_LEVEL ) {
				_nCurSelection = fmath_RandomChoice( _nNumSinglePlayerSelections - _nStressTestStartIndex );
				_nCurSelection += _nStressTestStartIndex;
                _nCurSubItem = fmath_RandomChoice( _paSinglePlayerMenuItems[_nCurSelection].nNumEntries );
			} else {
                _nCurSelection = fmath_RandomChoice( _nNumMultiPlayerSelections - _nStressTestStartIndex );
				_nCurSubItem = 0;
			}
		}
	}
		
	// clear any pending text prints
	ftext_ClearAllPending();

	_bPrevGovernorState = floop_IsGovernorEnabled();
	floop_EnableGovernor( TRUE );

	_bLoadLevelNextFrame = FALSE;

	return TRUE;
}

static BOOL _Work( void ) {

	FASSERT( _bSystemInitialized );

	gamepad_Sample();

	// start the music
	if( _pAudioStream &&
		_pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
		_pAudioStream->SetVolume( 0.25f );		
		_pAudioStream->Play( 0 );
	}

	// see if should exit the game
#if FANG_ENABLE_DEV_FEATURES
	if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState > 0.1 &&
		Gamepad_aapSample[0][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.1f &&
		Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_SIDE]->uLatches & FPAD_LATCH_ON ) {
		// exit the gameloop
		gameloop_ScheduleExit();
		return FALSE;
	}
#endif

	switch( _nCurrentMode ) {
	
	case _MODE_PICK_MULTI_PLAYER_LEVEL:	
	case _MODE_PICK_SINGLE_PLAYER_LEVEL:
		_PickLevelWork();	
		break;

	case _MODE_LEVEL_LOAD_ERROR:
		if( Gamepad_aapSample[0][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			_nCurrentMode = _nModeBeforeError;
		}
		break;
		
	default:
		FASSERT_NOW;
		break;
	}
		
	return TRUE;	
}

static BOOL _PickLevelWork( void ) {
	s32 nNumSelections;
	_MenuItem_t *paItems;

	// setup our ptrs depending on what mode we are in
	if( _nCurrentMode == _MODE_PICK_SINGLE_PLAYER_LEVEL ) {
		nNumSelections = _nNumSinglePlayerSelections;
		paItems = _paSinglePlayerMenuItems;
	} else {
		nNumSelections = _nNumMultiPlayerSelections;
		paItems = _paMultiPlayerMenuItems;
	}

	if( _bLoadLevelNextFrame ) {
		CDifficulty::SetLevel( _nDifficultyLevel );

		// we have been requested to load a level, do so now
		if( !launcher_StartGame( LAUNCHER_FROM_PICK_LEVEL, _nLoadTestMode ) ) {
			// setup an error message and go back into the menus
			gameloop_SetLoopHandlers( _Work, _Draw, _InitPickLevel );
			_nModeBeforeError = _nCurrentMode;
			_nCurrentMode = _MODE_LEVEL_LOAD_ERROR;
			_InitPickLevel();
			_nLoadTestMode = 0;
		}
		_bLoadLevelNextFrame = FALSE;
		return TRUE;
	}
	
	if( _nLoadTestMode > 1 ) {
		// just waiting a few frames till we auto select the next level
		_nLoadTestMode--;
		return TRUE;
	}

	// see if a load test selection has been made
	if( _nLoadTestMode == 1 ||
		(Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_TOP]->fCurrentState > 0.5f && Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_SIDE]->fCurrentState > 0.5f) ) {
		// record the currently selected level name
		fsndfx_Play2D( _hSelected );
		_pszLevelToLoad = paItems[_nCurSelection].pFirstEntry[_nCurSubItem].pszTitle;
		_StartLoadTestMode();
		return TRUE;		
	}

	// see if a load stress test was requested
	if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState > 0.5f &&
		Gamepad_aapSample[0][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.5f &&
		Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_TOP]->fCurrentState > 0.5f ) {
		// record the currently selected level name
		fsndfx_Play2D( _hSelected );
		_pszLevelToLoad = paItems[_nCurSelection].pFirstEntry[_nCurSubItem].pszTitle;
		_StartLoadTestMode();
		if( !_bStressTest ) {
            _bStressTest = TRUE;
			_nStressTestStartIndex = _nCurSelection;
		}
		return TRUE;
	}

	if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_SHOULDER]->fCurrentState == 0.0f &&
		Gamepad_aapSample[0][GAMEPAD_MENU_RIGHT_SHOULDER]->fCurrentState > 0.5f &&
		(Gamepad_aapSample[0][GAMEPAD_MENU_BUTTON_SIDE]->uLatches & (FPAD_LATCH_ON | FPAD_LATCH_CHANGED)) == (FPAD_LATCH_ON | FPAD_LATCH_CHANGED) ) {

		if( ++_nDifficultyLevel >= CDifficulty::DIFFICULTY_COUNT ) {
			_nDifficultyLevel = 0;
		}

		fsndfx_Play2D( _hSoundDevOptionChanged, 1.0f, 0.9f + (0.1f * (f32)_nDifficultyLevel) );

		return TRUE;
	}

	// allow the user to switch between single and multi player menus
	if( Gamepad_aapSample[0][GAMEPAD_MENU_BACK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		if( _nCurrentMode == _MODE_PICK_SINGLE_PLAYER_LEVEL ) {
			_nCurrentMode = _MODE_PICK_MULTI_PLAYER_LEVEL;
		} else {
			_nCurrentMode = _MODE_PICK_SINGLE_PLAYER_LEVEL;
		}
		// default to the first item in the list
		_nCurSelection = 0;
		_nCurSubItem = 0;
        return TRUE;
	}
	
	// see if a selection has been made
	if( Gamepad_aapSample[0][GAMEPAD_MENU_ACCEPT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
		// record the currently selected level name
		fsndfx_Play2D( _hSelected );
		_pszLevelToLoad = paItems[_nCurSelection].pFirstEntry[_nCurSubItem].pszTitle;
		_bLoadLevelNextFrame = TRUE;		
		return TRUE;
	}	

	s32 nOldSelection = _nCurSelection;
	s32 nOldSubItem = _nCurSubItem;

	// see if we should change the current selection (up/down)
	if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_ANALOG_Y]->uLatches &
		FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

		if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_ANALOG_Y]->fCurrentState > 0.1f ) {
			_nCurSelection--;			
		} else {
			_nCurSelection++;
		}
		_nCurSubItem = 0;
	} else if( Gamepad_aapSample[0][GAMEPAD_MENU_DPAD_Y]->uLatches &
			   FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
		if( Gamepad_aapSample[0][GAMEPAD_MENU_DPAD_Y]->fCurrentState > 0.1f ) {
			_nCurSelection--;
		} else {
			_nCurSelection++;
		}
		_nCurSubItem = 0;
	} else {
		// see if we should change the current selection (left/right)
		if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_ANALOG_X]->uLatches &
			FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

			if( Gamepad_aapSample[0][GAMEPAD_MENU_LEFT_ANALOG_X]->fCurrentState > 0.1f ) {
				_nCurSubItem++;
				if( _nCurSubItem >= (s32)paItems[_nCurSelection].nNumEntries ) {
					_nCurSelection += _MAX_ENTRIES_PER_COL;
					_nCurSubItem = 0;
				}
			} else {
				_nCurSubItem--;
				if( _nCurSubItem < 0 ) {
					_nCurSelection -= _MAX_ENTRIES_PER_COL;
				}
			}		
		} else if( Gamepad_aapSample[0][GAMEPAD_MENU_DPAD_X]->uLatches &
				   FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
			if( Gamepad_aapSample[0][GAMEPAD_MENU_DPAD_X]->fCurrentState > 0.1f ) {
				_nCurSubItem++;
				if( _nCurSubItem >= (s32)paItems[_nCurSelection].nNumEntries ) {
					_nCurSelection += _MAX_ENTRIES_PER_COL;
					_nCurSubItem = 0;
				}
			} else {
				_nCurSubItem--;
				if( _nCurSubItem < 0 ) {
					_nCurSelection -= _MAX_ENTRIES_PER_COL;
				}
			}
		}
		if( _nCurSelection < 0 ) {
			_nCurSelection += _MAX_ENTRIES_PER_COL;
			_nCurSubItem = 0;
		} else if( _nCurSelection >= nNumSelections ) {
			_nCurSelection -= _MAX_ENTRIES_PER_COL;
			_nCurSubItem = paItems[_nCurSelection].nNumEntries - 1;
		}
		if( _nCurSubItem < 0 ) {
			_nCurSubItem = 0;
		}
	}

	if( _nCurSelection < 0 ) {
		_nCurSelection = nNumSelections-1;
		_nCurSubItem = 0;
	} else if( _nCurSelection >= nNumSelections ) {
		_nCurSelection = 0;
		_nCurSubItem = 0;
	}

	if( nOldSelection != _nCurSelection ||
		nOldSubItem != _nCurSubItem ) {
		fsndfx_Play2D( _hClick );
	}

	return TRUE;
}

static BOOL _Draw( void ) {

	FASSERT( _bSystemInitialized );

	fviewport_SetActive( NULL );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.03f, 1.0f, 0 );

	ftext_ClearAllPending();

	switch( _nCurrentMode ) {
		
	case _MODE_LEVEL_LOAD_ERROR:
#if defined(MA_PC_INPUT)
		{
			const PcPromptStyle nStyle = pcinput_PromptStyleForPort( pcinput_KeyboardPort() );
			cwchar *pwszAccept = nStyle == PCINPUT_PROMPT_STYLE_KEYBOARD ? L"Space" :
				( nStyle == PCINPUT_PROMPT_STYLE_PLAYSTATION ? L"Cross" : L"A" );
			ftext_Printf( 0.5f, 0.20f, "~f0~C92929299~w1~ac~s0.85Error: Could not load\n\n%s.\n\nPress %ls to continue.", _pszLevelToLoad, pwszAccept );
		}
#else
		ftext_Printf( 0.5f, 0.20f, "~f0~C92929299~w1~ac~s0.85Error: Could not load\n\n%s.\n\nPress 'A' to continue.", _pszLevelToLoad );
#endif
		break;

	case _MODE_PICK_MULTI_PLAYER_LEVEL:
	case _MODE_PICK_SINGLE_PLAYER_LEVEL:
		{	
			s32 nNumSelections;
			_MenuItem_t *paItems;
			f32 fFirstColumnX;
			f32 fFirstColumnY;
			if( _nCurrentMode == _MODE_PICK_SINGLE_PLAYER_LEVEL ) {
				nNumSelections = _nNumSinglePlayerSelections;
				paItems = _paSinglePlayerMenuItems;
                ftext_Printf( 0.5f, 0.075f, "~f3~w1~ac~s1.05Pick SP Level" );
				fFirstColumnX = 0.07f;
				fFirstColumnY = 0.12f;
			} else {
				nNumSelections = _nNumMultiPlayerSelections;
				paItems = _paMultiPlayerMenuItems;
                ftext_Printf( 0.5f, 0.075f, "~f3~w1~ac~s1.05Pick MP Level" );
				fFirstColumnX = 0.25f;
				fFirstColumnY = 0.12f;
			}

			s32 i, j;
			f32 fY, fX;
			for( i=0; i < nNumSelections; i++ ) {
				if( i < _MAX_ENTRIES_PER_COL ) { 
					fY = fFirstColumnY + (i * 0.0325f);
					fX = fFirstColumnX;
				} else {
					fY = fFirstColumnY + ((i-_MAX_ENTRIES_PER_COL) * 0.0325f);
					fX = 0.52f;
				}
				if( _nCurSelection == (s32)i ) {
					// this is the current selection
					if( _nLoadTestMode ) {
						// don't flash the current selection since we are in auto select mode
						ftext_Printf( fX, fY, "~C90502099~f0~w1~al~s0.66%s", paItems[i].pszDisplayTitle );
					} else {
						// flash the current selection since we are in auto selecting test mode
						ftext_Printf( fX, fY, "~B5~C90502099~f0~w1~al~s0.66%s", paItems[i].pszDisplayTitle );
					}
				} else {
					if( paItems[i].pFirstEntry->nLevel == LEVEL_DEVELOPMENT_LEVEL ) {
						ftext_Printf( fX, fY, "~f0~C75159299~w1~al~s0.66%s", paItems[i].pszDisplayTitle );
					} else {
                        ftext_Printf( fX, fY, "~f0~C92929299~w1~al~s0.66%s", paItems[i].pszDisplayTitle );
					}
				}
				// draw the subitem numbers
				if( _nCurrentMode == _MODE_PICK_SINGLE_PLAYER_LEVEL ) {
					fX += 0.335f;
					for( j=0; j < (s32)paItems[i].nNumEntries; j++ ) {
						if( _nCurSelection == (s32)i && j == _nCurSubItem ) {
							if( _nLoadTestMode ) {
								// don't flash the current selection since we are in auto select mode	
								ftext_Printf( fX, fY, "~C90502099~f0~w1~al~s0.66%d ", j+1 );
							} else {
								// flash the current selection since we are in auto selecting test mode
								ftext_Printf( fX, fY, "~B5~C90502099~f0~w1~al~s0.66%d ", j+1 );
							}
						} else {
							if( paItems[i].pFirstEntry->nLevel == LEVEL_DEVELOPMENT_LEVEL ) {
								ftext_Printf( fX, fY, "~f0~C75159299~w1~al~s0.66%d ", j+1 );		
							} else {
								ftext_Printf( fX, fY, "~f0~C92929299~w1~al~s0.66%d ", j+1 );
							}
						}
						fX += 0.015f;
					}				
				}
			}
		}
		if( _fNumLoads > 0.0f ) {
			ftext_Printf( 0.45f, 0.58f, "~f1~C25305585~w0~aL~s0.65Last = %.2f secs\n"
										"Longest = '%s' %.2f\n"
										"%d loads avg %.2f secs", _fLastLoadSecs,
																  _pszLongestLevelLoad, _fLongestLoadSecs,
																  (u32)_fNumLoads, _fAverageLoadSecs );
		}

		// draw instructions
#if !FANG_PRODUCTION_BUILD
		ftext_Printf( 0.07f, 0.58f, "~f1~C25305585~w0~aL~s0.65A = Reg Load\n"
									"~C55303585X+Y = Load Test\n"
									"~C25305585B = %s Menus\n"
									"~C55303585Y+Triggers = Stress Test\n"
									"~C25305585X+RightTrig = Difficulty (%s)",
									(_nCurrentMode == _MODE_PICK_MULTI_PLAYER_LEVEL) ? "SP" : "MP", CDifficulty::DevMode_GetEnglishNameForLevel( _nDifficultyLevel ) );
#else
		ftext_Printf( 0.07f, 0.58f, "~f1~C25305585~w0~aL~s0.65A = Reg Load\n"
									"~C55303585X+Y = Load Test\n"
									"~C25305585B = %s Menus\n"
									"~C55303585Y+Triggers = Stress Test\n"
									"~C25305585X+RTrig = Diff (%d)",
									(_nCurrentMode == _MODE_PICK_MULTI_PLAYER_LEVEL) ? "SP" : "MP", _nDifficultyLevel );
#endif
		break;

	default:
		FASSERT_NOW;
		break;
	}

	// draw the version number in the lower right corner of the screen
	ftext_Printf( 0.68f, 0.67f, "~f1~C92929299~w1~al~s0.69Ver %d.%d.%d", fversion_GetToolMajorVer(), 
																		 fversion_GetToolMinorVer(),
																		 fversion_GetToolSubVer() );

	return TRUE;
}

static void _ResetVars( void ) {
	_bNeedToUnload = FALSE;
	_nCurrentMode = _MODE_PICK_SINGLE_PLAYER_LEVEL;
	_nModeBeforeError = _MODE_PICK_SINGLE_PLAYER_LEVEL;
	_nCurSubItem = 0;
	_nLoadTestMode = 0;
	_bLoadLevelNextFrame = FALSE;
	_pszLevelToLoad = NULL;
	_pAudioStream = NULL;
	_nCurSelection = 0;
	_nNumSinglePlayerSelections = 0;
	_nNumMultiPlayerSelections = 0;
	_paSinglePlayerMenuItems = NULL;
	_paMultiPlayerMenuItems = NULL;

	_fLastLoadSecs = 0.0f;
    _fLongestLoadSecs = 0.0f;
	_fAverageLoadSecs = 0.0f;
    _fNumLoads = 0.0f;
	_fTotalLoadSecs = 0.0f;
	_pszLongestLevelLoad = NULL;

	_bStressTest = FALSE;
	_nStressTestStartIndex = 0;

	// Set difficulty level to an invalid value so we'll know that it needs to be initialized...
	_nDifficultyLevel = CDifficulty::DIFFICULTY_COUNT;
}

static void _StartLoadTestMode( void ) {

	if( _nLoadTestMode == 0 ) {
		_fLastLoadSecs = 0.0f;
		_fLongestLoadSecs = 0.0f;
		_fAverageLoadSecs = 0.0f;
		_fNumLoads = 0.0f;
		_pszLongestLevelLoad = NULL;
		_fTotalLoadSecs = 0.0f;
	}

	_nLoadTestMode = 1;
	_bLoadLevelNextFrame = TRUE;
}

#else
BOOL picklevel_InitSystem( void ) {
	return TRUE;
}
void picklevel_UninitSystem( void ) {
}

#endif
