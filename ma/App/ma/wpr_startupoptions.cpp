//////////////////////////////////////////////////////////////////////////////////////
// wpr_startupoptions.cpp - The game startup options / menu.
//
// Author: Russell A. Foushee   
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
// 05/28/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fsysinfo.h"
#include "wpr_startupoptions.h"
#include "gameloop.h"
#include "ffile.h"
#include "floop.h"
#include "fresload.h"
#include "frenderer.h"
#include "fviewport.h"
#include "fxfm.h"
#include "launcher.h"
#include "fperf.h"
#include "wpr_drawutils.h"
#include "gamepad.h"
#include "game.h"
#include "pausescreen.h"

//====================
// private definitions

#define _NUM_CONTROLLERS				( 4 )
#define _SAVE_OFF_CONTROLLER_MAPPINGS	FALSE
#define _NUM_MENU_ITEMS					2
#define _STARTUP_OPTIONS_CSVNAME		"startopts$"
#define _ASSETS_TABLE_NAME				"assets"
#define _MENUITEMS_TABLE_NAME			"MenuItems"
#define _NUM_MENU_FIELDS_PER_ENTRY		12
#define _SELECTIONREGION_TABLE_NAME		"SelectionRegion"
#define _REGIONLOCATION_TABLE_NAME		"RegionLocations"
#define _BUTTONDATA_TABLE_NAME			"ButtonData"
#define _NUM_BUTTONDATA_FIELDS			6
#define _QUESTION_COLOR					L"70757885"
#define _OPTION_COLOR					L"70757899"
#define _RESOLUTION_DELAY_TIME			2.5f
#define _BLACKSCREEN_DELAY_FRAMECOUNT	3.0f

typedef struct {
	BOOL bDisplayItem;
	f32 fYRegionTopPos; // The top position of the region for this menu item
	wchar awszQuestion[2][256];
	wchar aawszResolution[2][2][256]; // This is the array that stores the question resolution (2 resolution options, 2 lines each)
	wchar awszOptions[2][64];
	u32 nSelectedOptionID;
	f32 fYQuestionPos; // The screen Y offset the question into the selection region
	f32 fYOptionPos;   // The screen Y offset of the currently selection option in the selection region
	f32 fYResolutionPos; // The screen Y offset the resolution string into the selection region
} _StartupMenuItem_t;

typedef struct {
	f32 fButtonPosX;
	f32 fButtonPosY;
	f32 fButtonWidth;
	f32 fButtonHeight;
	f32 fButtonYTextAdj;
	wchar wszButtonTextStyle[64];
} _StartupButtonData_t;

typedef enum {
	_STARTUPOPTIONS_SELECTION_MODE,		// The user is selecting the proper actions
	_STARTUPOPTIONS_RESOLUTION_MODE,	// The selected options are being displayed to the user
	_STARTUPOPTIONS_BLACKSCREEN_MODE,	// The screen is drawn black for a very short amount of time
} _StartupOptionsMode_e;

//=================
// public variables

//==================
// private variables

static BOOL _bSystemInitialized = FALSE;
static BOOL _bBButtonPressedAtStartup;
static FViewport_t *_pViewportOrtho3D;
static FResFrame_t _ResFrame;
static BOOL _bPrevGovernorState;
static CFTexInst *_pBackgroundTexInst;
static CFTexInst *_pHighlightTexInst;
static CFTexInst *_pButtonTexInst;
static _StartupMenuItem_t _aMenuItems[ _NUM_MENU_ITEMS ];
static u32 _nItemsToDisplay;
static f32 _afRegionLocations[ 3 ];
static f32 _fRegionWidth;
static f32 _fRegionHeight;
static u32 _nCurrentSelection;
static _StartupOptionsMode_e _eCurrentMode;
static f32 _fDelayTimer;

// Button Variables
static _StartupButtonData_t *_pButtonData;

#if _SAVE_OFF_CONTROLLER_MAPPINGS
static GamepadMap_e _nPrevGamePadMap[ _NUM_CONTROLLERS ];
#endif

//===================
// private prototypes

static BOOL _Init( void );
static BOOL _Work( void );
static BOOL _Draw( void );
static void _SelectionModeWork( void );
static void _ResolutionModeWork( void );
static void _BlackScreenModeWork( void );
static void _SelectionModeDraw( void );
static void _ResolutionModeDraw( void );
static void _DrawBackgroundAndHighlight( BOOL bDrawHighlight );
static void _DrawButton( void );
static BOOL _UnloadAndExit( BOOL bLoadWrappers );
static BOOL _LoadStartupOptionsCSV( void );
static BOOL _ParseAssetsTable( FGameDataFileHandle_t hFile );
static BOOL _ParseMenuTable( FGameDataFileHandle_t hFile );
static BOOL _ParseSelectionRegionTable( FGameDataFileHandle_t hFile );
static BOOL _ParseRegionLocationTable( FGameDataFileHandle_t hFile );
static BOOL _ParseButtonData( FGameDataFileHandle_t hFile );
static void _SwitchToMode( _StartupOptionsMode_e eNewMode );

//=================
// public functions

BOOL wpr_startupoptions_InitSystem( void ) {

	_bSystemInitialized = TRUE;

	_bBButtonPressedAtStartup = FALSE;

	_pBackgroundTexInst = NULL;
	_pHighlightTexInst = NULL;
	_pButtonTexInst = NULL;
	_nItemsToDisplay = 0;

	// Clear the menu options array
	fang_MemZero( _aMenuItems, _NUM_MENU_ITEMS * sizeof( _StartupMenuItem_t ) );

	_eCurrentMode = _STARTUPOPTIONS_SELECTION_MODE;

	return TRUE;
}

void wpr_startupoptions_UninitSystem( void ) {

	_bSystemInitialized = FALSE;
}

void wpr_startupoptions_CaptureAppStartButtons() {

#if FANG_PLATFORM_GC

	u32 i;

	// setup the controller mapping
	for( i=0; i < _NUM_CONTROLLERS; i++ ) {
#if _SAVE_OFF_CONTROLLER_MAPPINGS
		_nPrevGamePadMap[ i ] = gamepad_GetMapping( i );
#endif
		gamepad_SetMapping( i, GAMEPAD_MAP_MENU );
	}

	gamepad_Sample();

	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
		if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_BACK ]->uLatches & FPAD_LATCH_ON ) {
			_bBButtonPressedAtStartup = TRUE;
		}
	}

#if _SAVE_OFF_CONTROLLER_MAPPINGS
	// restore previous gamepad mapping
	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
		gamepad_SetMapping( i, _nPrevGamePadMap[ i ] );
	}
#endif

#endif

}



void wpr_startupoptions_Start( void ) {
	FASSERT( _bSystemInitialized );

	gameloop_SetLoopHandlers( _Work, _Draw, _Init );
}


//==================
// private functions

static BOOL _Init( void ) {

	u32 i;

	// setup the controller mapping
	for( i=0; i < _NUM_CONTROLLERS; i++ ) {
#if _SAVE_OFF_CONTROLLER_MAPPINGS
		_nPrevGamePadMap[ i ] = gamepad_GetMapping( i );
#endif
		gamepad_SetMapping( i, GAMEPAD_MAP_MENU );
	}

	// grab memory frames...
	_ResFrame = fres_GetFrame();

	_pViewportOrtho3D = fviewport_Create();
	if( !_pViewportOrtho3D ) {
		_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	fviewport_InitOrtho3D( _pViewportOrtho3D, 0.1f, 100.0f );

	_bPrevGovernorState = floop_IsGovernorEnabled();
	floop_EnableGovernor( TRUE );

	_pBackgroundTexInst = NULL;
	_pHighlightTexInst = NULL;
	_pButtonTexInst = NULL;

	_nItemsToDisplay = 0;

#if FANG_PLATFORM_GC
	// The XBOX has a dashboard to set this stuff up, so we can just use that.
	// The gamecube on the other hand... well... they are kinda beevises when it comes to this stuff
	if( !_LoadStartupOptionsCSV() ) {
		_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}

	// Load the controller button textures ( for the accept button )
	_pButtonTexInst = fnew CFTexInst;
	if( !_pButtonTexInst ) {
		DEVPRINTF("wpr_startupoptions : _Init() : Could not create a button CFTexInst!\n" );
		_UnloadAndExit( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	_pButtonTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, CPauseScreen_pszControlsTex ) );	
	_pButtonTexInst->ClearFlag( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T | CFTexInst::FLAG_WRAP_U );

	// Now, examine the fsysinfo structures and determine what we need to do...
	u32 uCaps = fsysinfo_GetCapabilities();

	// Check to see if we should display the PAL selection option
	if( ( uCaps & FSYSINFO_VIDEO_PAL_60HZ ) || ( _bBButtonPressedAtStartup && ( uCaps & FSYSINFO_VIDEO_PAL ) ) ) {
		// We have pal mode, so we definately want to display the 50/60 hz option...
		_aMenuItems[ 0 ].bDisplayItem = TRUE;
		_aMenuItems[ 0 ].fYRegionTopPos = _afRegionLocations[ 2 ];
		_nItemsToDisplay++;
		_nCurrentSelection = 0;
	}

	if( uCaps & FSYSINFO_AUDIO_STEREO ) {
		// Ask the surround question
		_aMenuItems[ 1 ].bDisplayItem = TRUE;
		if( _aMenuItems[ 0 ].bDisplayItem ) {
			_aMenuItems[ 0 ].fYRegionTopPos = _afRegionLocations[ 0 ];
			_aMenuItems[ 1 ].fYRegionTopPos = _afRegionLocations[ 1 ];
		} else {
			_aMenuItems[ 1 ].fYRegionTopPos = _afRegionLocations[ 2 ];
			_nCurrentSelection = 1;
		}

		_nItemsToDisplay++;
	}

#endif

	return TRUE;
}

static void _SwitchToMode( _StartupOptionsMode_e eNewMode ) {

	if( eNewMode != _eCurrentMode ) {
		if( eNewMode == _STARTUPOPTIONS_SELECTION_MODE ) {
		} else if ( eNewMode == _STARTUPOPTIONS_RESOLUTION_MODE ) {
			_fDelayTimer = 0.0f;
		} else if ( eNewMode == _STARTUPOPTIONS_BLACKSCREEN_MODE ) {
			_fDelayTimer = 0.0f;
		}
	}
		
	_eCurrentMode = eNewMode;
}

static BOOL _Work( void ) {

	FASSERT( _bSystemInitialized );

	if( ( _eCurrentMode == _STARTUPOPTIONS_SELECTION_MODE ) && ( _nItemsToDisplay == 0 ) ) {
		_SwitchToMode( _STARTUPOPTIONS_BLACKSCREEN_MODE );
		return TRUE;
	}

	switch( _eCurrentMode ) {
		case _STARTUPOPTIONS_SELECTION_MODE:
			_SelectionModeWork();
			break;
		
		case _STARTUPOPTIONS_RESOLUTION_MODE:
			_ResolutionModeWork();
			break;

		case _STARTUPOPTIONS_BLACKSCREEN_MODE:
			_BlackScreenModeWork();
			break;

		default:
			FASSERT_NOW;
	}

	return TRUE;
}

static void _SelectionModeWork( void ) {

	u32 i;

	// Gamecube needs some special technology
	gamepad_Sample();

	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
		if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_ACCEPT ]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			// Selections are done -- Apply them
#if FANG_PLATFORM_GC
			// See if we should apply the PAL 60 mode
			if( _aMenuItems[ 0 ].bDisplayItem && ( _aMenuItems[ 0 ].nSelectedOptionID == 0 ) ) {
				FVidWin_t NewWinParams = FVid_Win;
				NewWinParams.VidMode.nFlags |= FVID_MODEFLAG_PAL_60HZ;
				fvid_ResetWindow( &NewWinParams );
			}
			
			// See if we should apply the surround sound mode
			if( _aMenuItems[ 1 ].bDisplayItem && ( _aMenuItems[ 1 ].nSelectedOptionID == 0 ) ) {
				faudio_EnableSurround_GC( TRUE );
			}
#endif
			_SwitchToMode( _STARTUPOPTIONS_RESOLUTION_MODE );
			return;			
		}
	}

	// Loop through all the controllers and see if one is commanding a change...
	BOOL bSelectionUp = FALSE;
	BOOL bSelectionDown = FALSE;
	BOOL bSelectionLeftRight = FALSE;
	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
	// see if we should change the current selection (up/down)
		if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_LEFT_ANALOG_Y ]->uLatches &
			FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
			if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_LEFT_ANALOG_Y ]->fCurrentState > 0.1f ) {
				// Selection up
				bSelectionUp = TRUE;
			} else {
				// Selection Down
				bSelectionDown = TRUE;
			}
		} else if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_DPAD_Y ]->uLatches &
				FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
			if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_DPAD_Y ]->fCurrentState > 0.1f ) {
				// Selection up
				bSelectionUp = TRUE;
			} else {
				// Selection down
				bSelectionDown = TRUE;
			}
		} else {
			// see if we should change the current selection (left/right)
			if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_LEFT_ANALOG_X ]->uLatches &
				FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

				bSelectionLeftRight = TRUE;
			} else if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_DPAD_X ]->uLatches &
					FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
				bSelectionLeftRight = TRUE;
			}
		}
	}
	if( bSelectionUp && ( _nItemsToDisplay > 1 ) && ( _nCurrentSelection > 0 ) ) {
		_nCurrentSelection--;
	}
	if( bSelectionDown && ( _nItemsToDisplay > 1 ) && ( _nCurrentSelection < ( _NUM_MENU_ITEMS - 1 ) ) ) {
		_nCurrentSelection++;
	}

	if( bSelectionLeftRight ) {
		_aMenuItems[ _nCurrentSelection ].nSelectedOptionID = !_aMenuItems[ _nCurrentSelection ].nSelectedOptionID;
	}
}


static void _ResolutionModeWork( void ) {
	_fDelayTimer += FLoop_fPreviousLoopSecs;
	if( _fDelayTimer >= _RESOLUTION_DELAY_TIME ) {
		_SwitchToMode( _STARTUPOPTIONS_BLACKSCREEN_MODE );
	}
}


static void _BlackScreenModeWork( void ) {
	_fDelayTimer += 1.0f;
	if( _fDelayTimer >= _BLACKSCREEN_DELAY_FRAMECOUNT ) {
		_UnloadAndExit( TRUE );
	}
}


static BOOL _Draw( void ) {

	FASSERT( _bSystemInitialized );

	if( _eCurrentMode == _STARTUPOPTIONS_BLACKSCREEN_MODE ) {
		// Draw the black loading screen for a couple of frames
		launcher_DrawLoadingScreen();
		return TRUE;
	}

	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
	
	FViewport_t *pPrevViewport = fviewport_SetActive( _pViewportOrtho3D );
	FXfm_Identity.InitStackWithView();

	// Draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );

	switch( _eCurrentMode ) {
		case _STARTUPOPTIONS_SELECTION_MODE:
			_SelectionModeDraw();
			break;
		
		case _STARTUPOPTIONS_RESOLUTION_MODE:
			_ResolutionModeDraw();
			break;

		case _STARTUPOPTIONS_BLACKSCREEN_MODE:
		default:
			FASSERT_NOW;
	}

	// Pop the fdraw renderer off the rendering stack 
	frenderer_Pop();
	
	// Restore the viewport
	fviewport_SetActive( pPrevViewport );

	return TRUE;
}


static void _SelectionModeDraw( void ) {

	u32 i;

	FASSERT( _pBackgroundTexInst );
	FASSERT( _pHighlightTexInst );
	FASSERT( _pButtonTexInst );

	// Draw the background texture map fullscreen left to right... the top
	// and bottom will clip
	wpr_drawutils_DrawTextureToScreen( TRUE,
						_pBackgroundTexInst,
						0.00f, 0.0f,
						2.0,
						_pViewportOrtho3D->Res.x * (1.0f/640.0f),
						_pViewportOrtho3D->HalfRes.x,
						_pViewportOrtho3D->HalfRes.x ); // This ensures it draws fullscreen

	_DrawBackgroundAndHighlight( _nItemsToDisplay > 1 );

	// Now, run through the menu items here and draw the appropriate text...
	for( i = 0; i < _NUM_MENU_ITEMS; i++ ) {
		if( _aMenuItems[ i ].bDisplayItem ) {
			// Draw the question here....
			ftext_Printf( 0.5f, _aMenuItems[ i ].fYRegionTopPos + _aMenuItems[ i ].fYQuestionPos,
				L"~f1~C%ls~w0~aC~s1.50%ls\n%ls",
				_QUESTION_COLOR,
				_aMenuItems[ i ].awszQuestion[ 0 ],
				_aMenuItems[ i ].awszQuestion[ 1 ]);

			if( i != _nCurrentSelection ) {
				// Draw the current selection here....
				ftext_Printf( 0.5f, _aMenuItems[ i ].fYRegionTopPos + _aMenuItems[ i ].fYOptionPos,
					L"~f1~C%ls~w0~aC~s1.50%ls",
					WprDataTypes_pwszWhiteTextColor, //_OPTION_COLOR,
					_aMenuItems[ i ].awszOptions[ _aMenuItems[ i ].nSelectedOptionID ] );
			} else {
				// lets do some blinking text with arrows...
				ftext_Printf( 0.5f, _aMenuItems[ i ].fYRegionTopPos + _aMenuItems[ i ].fYOptionPos,
					L"~f1"		// Font
					L"~aC"		// Alignment
					L"~w0"		// Not Fixed width
					L"~s1.50"	// Scale
					L"~C%ls"	// Arrow Color (Set to Blue!)
					L"{"		// The left Arrow
					L"~C%ls"	// Text Color
					L"~B9"		// Set to Blink
					L"%ls"		// The Text itself
					L"~B0"		// No blink
					L"~C%ls"	// Set back to Arrow Color
					L"}",		// The Right Arrow
					WprDataTypes_pwszBlueTextColor, 
					WprDataTypes_pwszWhiteTextColor,
					_aMenuItems[ i ].awszOptions[ _aMenuItems[ i ].nSelectedOptionID ],
					WprDataTypes_pwszBlueTextColor );
			}
		}
	}

	_DrawButton();
}



static void _ResolutionModeDraw( void ) {

	u32 i;

	FASSERT( _pBackgroundTexInst );

	// Draw the background texture map fullscreen left to right... the top
	// and bottom will clip
	wpr_drawutils_DrawTextureToScreen( TRUE,
						_pBackgroundTexInst,
						0.00f, 0.0f,
						2.0,
						_pViewportOrtho3D->Res.x * (1.0f/640.0f),
						_pViewportOrtho3D->HalfRes.x,
						_pViewportOrtho3D->HalfRes.x ); // This ensures it draws fullscreen

	_DrawBackgroundAndHighlight( FALSE );

	// Now, run through the menu items here and draw the appropriate text...
	for( i = 0; i < _NUM_MENU_ITEMS; i++ ) {
		if( _aMenuItems[ i ].bDisplayItem ) {
			// Draw the resolution here....
			ftext_Printf( 0.5f, _aMenuItems[ i ].fYRegionTopPos + _aMenuItems[ i ].fYResolutionPos,
				L"~f1~C%ls~w0~aC~s1.50%ls\n%ls",
				_QUESTION_COLOR,
				_aMenuItems[ i ].aawszResolution[ _aMenuItems[ i ].nSelectedOptionID ][ 0 ],
				_aMenuItems[ i ].aawszResolution[ _aMenuItems[ i ].nSelectedOptionID ][ 1 ] );
		}
	}
}


static void _DrawBackgroundAndHighlight( BOOL bDrawHighlight ) {
	CFVec2 Lower, Upper;
	FDrawVtx_t aVtx[4];// used by all fdraw functions
	u32 i;

	for( i = 0; i < _NUM_MENU_ITEMS; i++ ) {
		if( _aMenuItems[ i ].bDisplayItem ) {
			// Next, draw a highlight and a background around the currently selected menu item
			f32 fUpperX = 0.5f - _fRegionWidth * 0.5f;
			f32 fLowerX = 0.5f + _fRegionWidth * 0.5f;
			f32 fUpperY = _aMenuItems[ i ].fYRegionTopPos;
			f32 fLowerY = fUpperY + _fRegionHeight;

			wpr_drawutils_ConvertTextCoordsToOrthoCoords( fUpperX, fUpperY, Upper.x, Upper.y, _pViewportOrtho3D->HalfRes.x, _pViewportOrtho3D->HalfRes.y ); 
			wpr_drawutils_ConvertTextCoordsToOrthoCoords( fLowerX, fLowerY, Lower.x, Lower.y, _pViewportOrtho3D->HalfRes.x, _pViewportOrtho3D->HalfRes.y ); 

			//Now, multiply the ortho coordinates by the half screen resolution!
			Upper.x *= _pViewportOrtho3D->HalfRes.x;
			Lower.x *= _pViewportOrtho3D->HalfRes.x;
			Upper.y *= _pViewportOrtho3D->HalfRes.y;
			Lower.y *= _pViewportOrtho3D->HalfRes.y;

			// Now, we have the orthographic coordinates of the region of the selected box... draw our highlight around it!
			aVtx[0].ST.Set( 0.0f, 1.0f );
			aVtx[1].ST.Set( 0.0f, 0.0f );
			aVtx[2].ST.Set( 1.0f, 1.0f );
			aVtx[3].ST.Set( 1.0f, 0.0f );

			CFColorRGBA Color;
			Color.Set( 0.0f, 0.0f, 0.0f, 0.25f );

			aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 1.0f ); 
			aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 1.0f );						
			aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 1.0f );
			aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 1.0f );

			aVtx[0].ColorRGBA = Color;
			aVtx[1].ColorRGBA = Color;
			aVtx[2].ColorRGBA = Color;
			aVtx[3].ColorRGBA = Color;

			fdraw_SetTexture( NULL );
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );

			// Only draw a highlight if this is the highlighed item, and there
			// is more than one question on the screen
			if( bDrawHighlight && ( i == _nCurrentSelection ) ) {
				// Draw a highlight border around this object as well

				CFColorRGBA TexColor;
				TexColor.Set( 1.0f, 1.0f, 1.0f, 1.0f );
				aVtx[0].ColorRGBA = TexColor;
				aVtx[1].ColorRGBA = TexColor;
				aVtx[2].ColorRGBA = TexColor;
				aVtx[3].ColorRGBA = TexColor;

				fdraw_SetTexture( _pHighlightTexInst );
				fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );
			}
		}
	}
}

// Assumes the fdraw renderer is set up, with an orthographic viewport
void _DrawButton( void ) {
	f32 fButtonSizeX, fButtonSizeY;
	u32 i;
	FDrawVtx_t aVtx[4];// used by all fdraw functions
	CFVec2 Lower, Upper;

	fButtonSizeX = _pButtonData->fButtonWidth;
	fButtonSizeY = _pButtonData->fButtonHeight;

	fdraw_SetTexture( _pButtonTexInst );

	f32 fUpperX = _pButtonData->fButtonPosX;
	f32 fLowerX = _pButtonData->fButtonPosX + fButtonSizeX;
	f32 fUpperY = _pButtonData->fButtonPosY;
	f32 fLowerY = _pButtonData->fButtonPosY + fButtonSizeY;

	wpr_drawutils_ConvertTextCoordsToOrthoCoords( fUpperX, fUpperY, Upper.x, Upper.y, _pViewportOrtho3D->HalfRes.x, _pViewportOrtho3D->HalfRes.y ); 
	wpr_drawutils_ConvertTextCoordsToOrthoCoords( fLowerX, fLowerY, Lower.x, Lower.y, _pViewportOrtho3D->HalfRes.x, _pViewportOrtho3D->HalfRes.y ); 

	aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 1.0f ); 
	aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 1.0f );						
	aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 1.0f );
	aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 1.0f );

	aVtx[0].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].y );
	aVtx[1].ST.Set( CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].y );
	aVtx[2].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].y );
	aVtx[3].ST.Set( CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].x, CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].y );

	CFColorRGBA TexColor;
	TexColor.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	aVtx[0].ColorRGBA = TexColor;
	aVtx[1].ColorRGBA = TexColor;
	aVtx[2].ColorRGBA = TexColor;
	aVtx[3].ColorRGBA = TexColor;

	for( i=0; i<4; i++ ) {
		aVtx[i].Pos_MS.x *= _pViewportOrtho3D->HalfRes.x;
		aVtx[i].Pos_MS.y *= _pViewportOrtho3D->HalfRes.y;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );

	// Now draw the text
	ftext_Printf( _pButtonData->fButtonPosX + fButtonSizeX, _pButtonData->fButtonPosY + _pButtonData->fButtonYTextAdj, _pButtonData->wszButtonTextStyle, L"Accept" );
}


static BOOL _UnloadAndExit( BOOL bLoadWrappers ) {

	if( _pBackgroundTexInst ) {
		fdelete( _pBackgroundTexInst );
		_pBackgroundTexInst = NULL;		
	}

	if( _pHighlightTexInst ) {
		fdelete( _pHighlightTexInst );
		_pHighlightTexInst = NULL;		
	}

	if( _pButtonTexInst ) {
		fdelete( _pButtonTexInst );
		_pButtonTexInst = NULL;		
	}

	_pButtonData = NULL;

	// release the mem frames
	fres_ReleaseFrame( _ResFrame );

	floop_EnableGovernor( _bPrevGovernorState );

	_pViewportOrtho3D = NULL;

	gameloop_ShowFPS( TRUE );

#if _SAVE_OFF_CONTROLLER_MAPPINGS
	// restore previous gamepad mapping
	u32 i;
	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
		gamepad_SetMapping( i, _nPrevGamePadMap[ i ] );
	}
#endif

	// finally move on to the retail wrapper mode
	if( bLoadWrappers ) {
		launcher_EnterMenus( LAUNCHER_FROM_STARTUP_OPTIONS );
	}
	return TRUE;
}

static BOOL _LoadStartupOptionsCSV( void ) {
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;

	// First, open up the file
	// grab an fres and fmem frame
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _STARTUP_OPTIONS_CSVNAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "wpr_startupoptions : _LoadStartupOptionsCSV() : Could not load the startup options csv file '%s'.\n", _STARTUP_OPTIONS_CSVNAME );
		goto _ExitWithError;
	}

	if( !_ParseAssetsTable( hFile ) ) {
		goto _ExitWithError;
	}

	if( !_ParseMenuTable( hFile ) ) {
		goto _ExitWithError;
	}

	if( !_ParseSelectionRegionTable( hFile ) ) {
		goto _ExitWithError;
	}

	if( !_ParseRegionLocationTable( hFile ) ) {
		goto _ExitWithError;
	}

	if( !_ParseButtonData( hFile ) ) {
		goto _ExitWithError;
	}

	fmem_ReleaseFrame( hMemFrame );
	return TRUE;

_ExitWithError:

	fmem_ReleaseFrame( hMemFrame );
	return FALSE;
}

static BOOL _ParseAssetsTable( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nFieldsInTable;
	cchar *pszBackgroundTGA;
	cchar *pszHighlightTGA;

	hTable = fgamedata_GetFirstTableHandle( hFile, _ASSETS_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_startupoptions : _ParseAssetsTable() : Could not find the assets table named '%s'.\n", _ASSETS_TABLE_NAME );
		return FALSE;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nFieldsInTable = fgamedata_GetNumFields( hTable );
	if( nFieldsInTable != 2 ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "wpr_startupoptions : _ParseAssetsTable() : Error! -- Expecting 2 field in the table %s.  Found %d fields in table instead.\n",
			       _ASSETS_TABLE_NAME, nFieldsInTable );
		return FALSE;
	}

	pszBackgroundTGA = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("wpr_startupoptions : _ParseAssetsTable() : The Field 0 in table %s is not a STRING format!\n", _ASSETS_TABLE_NAME );
		return FALSE;
	}

	pszHighlightTGA = (cchar *)fgamedata_GetPtrToFieldData( hTable, 1, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("wpr_startupoptions : _ParseAssetsTable() : The Field 1 in table %s is not a STRING format!\n", _ASSETS_TABLE_NAME );
		return FALSE;
	}

	// Now, load the assets
	// lastly, create the texture assets
	_pBackgroundTexInst = fnew CFTexInst;
	if( !_pBackgroundTexInst ) {
		DEVPRINTF("wpr_startupoptions : _ParseAssetsTable() : Could not create a Background CFTexInst!\n" );
		return FALSE;
	}
	_pBackgroundTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszBackgroundTGA ) );	

	_pHighlightTexInst = fnew CFTexInst;
	if( !_pHighlightTexInst ) {
		DEVPRINTF("wpr_startupoptions : _ParseAssetsTable() : Could not create a Highlight CFTexInst!\n" );
		return FALSE;
	}
	_pHighlightTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszHighlightTGA ) );	

	return TRUE;
}

static BOOL _ParseMenuTable( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 i;
	cwchar *pwszItem;

	// Now, load the language table...
	hTable = fgamedata_GetFirstTableHandle( hFile, _MENUITEMS_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_startupoptions : _ParseMenuTable() : Could not find the menu items table named '%s'.\n", _MENUITEMS_TABLE_NAME );
		return FALSE;
	}

	// verify the number of fields in the table 
	u32 nFieldCount = fgamedata_GetNumFields( hTable );
	if( nFieldCount % _NUM_MENU_FIELDS_PER_ENTRY ) {
		DEVPRINTF( "wpr_startupoptions : _ParseMenuTable() : Expecting exactly %d fields per menu entry in the %s table.\n", _NUM_MENU_FIELDS_PER_ENTRY, _MENUITEMS_TABLE_NAME );
		return FALSE;
	}

	// Figure out the number of menu entries...
	u32 nNumEntries = nFieldCount / _NUM_MENU_FIELDS_PER_ENTRY;
	if( nNumEntries != _NUM_MENU_ITEMS ) {
		DEVPRINTF( "wpr_startupoptions : _ParseMenuTable() : Expecting exactly %d item groups in the %s table.\n", _NUM_MENU_ITEMS, _MENUITEMS_TABLE_NAME );
		return FALSE;
	}

	// Now load the individual menu entries...
	for( i = 0; i < nNumEntries; i++ ) {
		_StartupMenuItem_t *pEntry = &_aMenuItems[ i ];

		// Load question line 1
		pwszItem = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 0, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a WIDESTRING format!\n", i * _NUM_MENU_FIELDS_PER_ENTRY + 0, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}
		fclib_wcscpy( pEntry->awszQuestion[ 0 ], pwszItem );

		// Load the question line 2
		pwszItem = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 1, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a WIDESTRING format!\n", i * _NUM_MENU_FIELDS_PER_ENTRY + 1, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}
		fclib_wcscpy( pEntry->awszQuestion[ 1 ], pwszItem );

		// Now load option A
		pwszItem = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 2, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a WIDESTRING format!\n", i * _NUM_MENU_FIELDS_PER_ENTRY + 2, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}
		fclib_wcscpy( pEntry->awszOptions[ 0 ], pwszItem );

		// Now load option B
		pwszItem = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 3, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a WIDESTRING format!\n", i * _NUM_MENU_FIELDS_PER_ENTRY + 3, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}
		fclib_wcscpy( pEntry->awszOptions[ 1 ], pwszItem );

		// Now load the default selected item
		u32 uSelectedID = ( u32 ) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 4, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a FLOAT format!\n",  i * _NUM_MENU_FIELDS_PER_ENTRY + 4, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}
		pEntry->nSelectedOptionID = uSelectedID;

		// Now load the Y question text region offset 
		pEntry->fYQuestionPos = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 5, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a FLOAT format!\n",  i * _NUM_MENU_FIELDS_PER_ENTRY + 5, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}

		// Now load the Y question text region offset 
		pEntry->fYOptionPos = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 6, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a FLOAT format!\n",  i * _NUM_MENU_FIELDS_PER_ENTRY + 6, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}

		// Load the Option resolution Lines
		for( u32 j = 0; j < 4; j++ ) {
			pwszItem = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 7 + j, nDataType );
			if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING) {
				DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a WIDESTRING format!\n", i * _NUM_MENU_FIELDS_PER_ENTRY + 7 + j, _MENUITEMS_TABLE_NAME );
				return FALSE;
			}
			fclib_wcscpy( pEntry->aawszResolution[ j/2 ][ j%2 ] , pwszItem );
		}

		// Now load the Y resolution text region offset 
		pEntry->fYResolutionPos = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_MENU_FIELDS_PER_ENTRY + 11, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_startupoptions : _ParseMenuTable() : Field %d in table %s is not a FLOAT format!\n",  i * _NUM_MENU_FIELDS_PER_ENTRY + 11, _MENUITEMS_TABLE_NAME );
			return FALSE;
		}
	}

	return TRUE;
}

static BOOL _ParseSelectionRegionTable( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nFieldsInTable;

	hTable = fgamedata_GetFirstTableHandle( hFile, _SELECTIONREGION_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_startupoptions : _ParseSelectionRegionTable() : Could not find the selection region table named '%s'.\n", _SELECTIONREGION_TABLE_NAME );
		return FALSE;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nFieldsInTable = fgamedata_GetNumFields( hTable );
	if( nFieldsInTable != 2 ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "wpr_startupoptions : _ParseSelectionRegionTable() : Error! -- Expecting 2 fields in the table %s.  Found %d fields in table instead.\n",
			       _SELECTIONREGION_TABLE_NAME, nFieldsInTable );
		return FALSE;
	}

	// Now, read in the region bounds
	_fRegionWidth = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_startupoptions : _ParseSelectionRegionTable() : Field 0 in table %s is not a FLOAT format!\n",  _SELECTIONREGION_TABLE_NAME );
		return FALSE;
	}

	_fRegionHeight = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 1, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_startupoptions : _ParseSelectionRegionTable() : Field 1 in table %s is not a FLOAT format!\n",  _SELECTIONREGION_TABLE_NAME );
		return FALSE;
	}

	return TRUE;
}

static BOOL _ParseRegionLocationTable( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nFieldsInTable;

	hTable = fgamedata_GetFirstTableHandle( hFile, _REGIONLOCATION_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_startupoptions : _ParseRegionLocationTable() : Could not find the region location table named '%s'.\n", _REGIONLOCATION_TABLE_NAME );
		return FALSE;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nFieldsInTable = fgamedata_GetNumFields( hTable );
	if( nFieldsInTable != 3 ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "wpr_startupoptions : _ParseRegionLocationTable() : Error! -- Expecting 3 fields in the table %s.  Found %d fields in table instead.\n",
			       _REGIONLOCATION_TABLE_NAME, nFieldsInTable );
		return FALSE;
	}

	// Now, read in the three y region locations
	u32 i;
	for( i = 0; i < 3; i++ ) {
		_afRegionLocations[ i ] = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_startupoptions : _ParseRegionLocationTable() : Field %d in table %s is not a FLOAT format!\n",  i, _REGIONLOCATION_TABLE_NAME );
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL _ParseButtonData( FGameDataFileHandle_t hFile ) {
	
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nFieldsInTable;

	hTable = fgamedata_GetFirstTableHandle( hFile, _BUTTONDATA_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_startupoptions : _ParseButtonData() : Could not find the Button Data table named '%s'.\n", _BUTTONDATA_TABLE_NAME );
		return FALSE;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nFieldsInTable = fgamedata_GetNumFields( hTable );
	if( nFieldsInTable != _NUM_BUTTONDATA_FIELDS ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "wpr_startupoptions : _ParseButtonData() : Error! -- Expecting %d fields in the table %s.  Found %d fields in table instead.\n",
			       _NUM_BUTTONDATA_FIELDS, _BUTTONDATA_TABLE_NAME, nFieldsInTable );
		return FALSE;
	}

	// Allocate the Button Data structure
	_pButtonData = ( _StartupButtonData_t* )fres_AllocAndZero( sizeof( _StartupButtonData_t ) );
	if( !_pButtonData ) {
		DEVPRINTF( "wpr_startupoptions : _ParseButtonData() : Could not allocate _pButtonData!\n" );
		return FALSE;
	}

	// Now, fill that puppy up!
	_pButtonData->fButtonPosX = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_startupoptions : _ParseButtonData() : Field 0 in table %s is not a FLOAT format!\n",  _BUTTONDATA_TABLE_NAME );
		return FALSE;
	}

	_pButtonData->fButtonPosY = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 1, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_startupoptions : _ParseButtonData() : Field 1 in table %s is not a FLOAT format!\n",  _BUTTONDATA_TABLE_NAME );
		return FALSE;
	}

	_pButtonData->fButtonWidth = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 2, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_startupoptions : _ParseButtonData() : Field 2 in table %s is not a FLOAT format!\n",  _BUTTONDATA_TABLE_NAME );
		return FALSE;
	}

	_pButtonData->fButtonHeight = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 3, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_startupoptions : _ParseButtonData() : Field 3 in table %s is not a FLOAT format!\n",  _BUTTONDATA_TABLE_NAME );
		return FALSE;
	}

	cwchar *pwszItem = (cwchar *)fgamedata_GetPtrToFieldData( hTable, 4, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING) {
		DEVPRINTF("wpr_startupoptions : _ParseButtonData() : Field 4 in table %s is not a WIDESTRING format!\n",  _BUTTONDATA_TABLE_NAME );
		return FALSE;
	}
	fclib_wcscpy( _pButtonData->wszButtonTextStyle, pwszItem );

	_pButtonData->fButtonYTextAdj = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 5, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_startupoptions : _ParseButtonData() : Field 5 in table %s is not a FLOAT format!\n",  _BUTTONDATA_TABLE_NAME );
		return FALSE;
	}

	return TRUE;
}