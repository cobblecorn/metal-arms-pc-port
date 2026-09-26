//////////////////////////////////////////////////////////////////////////////////////
// wpr_languageselect.cpp - The language selection module / menu.
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
// 05/20/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fsysinfo.h"
#include "wpr_languageselect.h"
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

//====================
// private definitions

#define _NUM_CONTROLLERS				( 4 )
#define _SAVE_OFF_CONTROLLER_MAPPINGS	FALSE
#define _LANGUAGE_SELECT_CSVNAME		"langselect"
#define _REFERENCE_CHAR_TABLE_NAME		"ReferenceChar"
#define _LANG_SELECT_MODE_TABLE_NAME	"LangSelectMode"

//Menu Mode Defines
#define _MENU_DATA_TABLE_NAME			"MenuData"
#define _NUM_MENU_DATA_TABLE_ENTRIES	5
#define _MENU_ITEMS_TABLE_NAME			"MenuItems"
#define _NUM_ITEMS_FIELDS_PER_ENTRY		10

//Fang Sysinfo Language Mode Defines
#define _FANG_LANGUAGE_TABLE_NAME		"FangLangTable"
#define _NUM_FANG_FIELDS_PER_LANG_ENTRY	3

typedef enum  {
	_LANGSELECT_DISABLED = 0,
	_LANGSELECT_MENU_MODE = 1,
	_LANGSELECT_FANG_MODE = 2,
	_LANGSELECT_LOADING_MODE = 3,
} _LangSelectMode_e;

typedef struct {
	f32 fUpperLeftX; // These 2 coordinates are In TEXEL SPACE!
	f32 fUpperLeftY;
	f32 fLowerRightX;
	f32 fLowerRightY;
	char cLangChar;
	char cAudioLangChar;
	s8 nLeftSelectionID;
	s8 nRightSelectionID;
	s8 nUpSelectionID;
	s8 nDownSelectionID;
} _MenuLangEntry_t;



typedef struct {
	CFTexInst *pBackgroundTexInst;
	CFTexInst *pLanguagesTexInst;
	CFTexInst *pHighlightTexInst;
	f32 fLanguagesTGATextureHeight;

	u32 nNumMenuItems;
	_MenuLangEntry_t *paLangEntries; 

	u32 nCurrentSelection;		// Which language is currently selected;
	char cReferenceChar;

} _MenuLangData_t;

typedef struct {
	u32 nXBLangID;			// The XBOX language ID
	char cLangChar;			// The App Language character to be used for everything BUT audio banks
	char cAudioLangChar;	// The app language character to be used for AUDIO BANKS
} _FangLangEntry_t;

typedef struct {
	u32 nTableEntries;
	_FangLangEntry_t *paLangEntries;
	char cReferenceChar;
} _FangLangData_t;

//=================
// public variables

//==================
// private variables

static BOOL _bSystemInitialized = FALSE;
static _LangSelectMode_e _eLangSelectMode;

static FViewport_t *_pViewportOrtho3D;
static FResFrame_t _ResFrame;
static _MenuLangData_t *_pMenuLangData;
static _FangLangData_t *_pFangLangData;
static BOOL _bPrevGovernorState;
static u32 _nLoadingFrameCounter;
#if _SAVE_OFF_CONTROLLER_MAPPINGS
static GamepadMap_e _nPrevGamePadMap[ _NUM_CONTROLLERS ];
#endif

//===================
// private prototypes

static BOOL _Init( void );
static BOOL _Work( void );
static BOOL _MenuModeWork( void );
static BOOL _FangModeWork( void );
static BOOL _Draw( void );
static BOOL _UnloadResources( BOOL bLoadFontsAndGamephrase );
static BOOL _LoadLanguageSelectCSV( void );
static BOOL _ParseEnableLangSelectTable( FGameDataFileHandle_t hFile );
static char _ParseReferenceCharTable( FGameDataFileHandle_t hFile );
static BOOL _LoadMenuLanguageData( FGameDataFileHandle_t hFile );
static BOOL _LoadFangLanguageData( FGameDataFileHandle_t hFile );
static BOOL _ParseMenuDataTable( FGameDataFileHandle_t hFile );
static BOOL _ParseMenuItemsTable( FGameDataFileHandle_t hFile );
static BOOL _SwitchToLoadingMode( void );

//=================
// public functions

BOOL wpr_languageselect_InitSystem( void ) {

	_bSystemInitialized = TRUE;
	_eLangSelectMode = _LANGSELECT_DISABLED;

	return TRUE;
}

void wpr_languageselect_UninitSystem( void ) {

	_bSystemInitialized = FALSE;
}

void wpr_languageselect_Start( void ) {
	FASSERT( _bSystemInitialized );

	gameloop_SetLoopHandlers( _Work, _Draw, _Init );
}


//==================
// private functions

static BOOL _Init( void ) {

	u32 i;

	_pMenuLangData = NULL;
	_pFangLangData = NULL;

	// setup the controller mapping
	for( i=0; i < _NUM_CONTROLLERS; i++ ) {
#if _SAVE_OFF_CONTROLLER_MAPPINGS
		_nPrevGamePadMap[ i ] = gamepad_GetMapping( i );
#endif
		gamepad_SetMapping( i, GAMEPAD_MAP_MENU );
	}

	// grab memory frames...
	_ResFrame = fres_GetFrame();

	// Load the internationalization csv file
	if( !_LoadLanguageSelectCSV() ) {
		_UnloadResources( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}

	_pViewportOrtho3D = fviewport_Create();
	if( !_pViewportOrtho3D ) {
		_UnloadResources( FALSE );
		launcher_LoadFailure();
		return TRUE;
	}
	fviewport_InitOrtho3D( _pViewportOrtho3D, 0.1f, 100.0f );

	_bPrevGovernorState = floop_IsGovernorEnabled();
	floop_EnableGovernor( TRUE );

	return TRUE;
}

static BOOL _Work( void ) {

	FASSERT( _bSystemInitialized );

	switch ( _eLangSelectMode ) {
		case _LANGSELECT_DISABLED:
			if( !_SwitchToLoadingMode() ) {
				launcher_LoadFailure();
			}
			break;

		case _LANGSELECT_MENU_MODE:
			_MenuModeWork();
			break;

		case _LANGSELECT_FANG_MODE:
			_FangModeWork();
			break;

		case _LANGSELECT_LOADING_MODE:
			// Display 3 frames of loading then move on...
			_nLoadingFrameCounter++;
			if( _nLoadingFrameCounter == 3 ) {
				// Load the rest of the assets and move on...
				if( !game_InitLocalizedResources() ) { // Call this after we have unloaded, so the resources stay in memory!
					launcher_LoadFailure();
				} else {
					wpr_startupoptions_Start(); // Fire up the startup options menu
				}
			}
			break;

		default:
			FASSERT_NOW; // Should not get here
	}

	return TRUE;
}

static BOOL _MenuModeWork( void ) {

	u32 i;

	FASSERT( _pMenuLangData );

	gamepad_Sample();

	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
		if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_ACCEPT ]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) {
			// Set the language ID here...
			_MenuLangEntry_t *pLangEntry = &_pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ];
			if( _pMenuLangData->cReferenceChar != pLangEntry->cLangChar ) {
				// Set the localization data
				ffile_SetLanguageChars( _pMenuLangData->cReferenceChar, pLangEntry->cLangChar, pLangEntry->cAudioLangChar );
			}

			if( !_SwitchToLoadingMode() ) {
				launcher_LoadFailure();
				return TRUE;
			}
		}
	}

	// Loop through all the controllers and see if one is commanding a change...
	for( i=0; i<_NUM_CONTROLLERS; i++ ) {
	// see if we should change the current selection (up/down)
		if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_LEFT_ANALOG_Y ]->uLatches &
			FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

			if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_LEFT_ANALOG_Y ]->fCurrentState > 0.1f ) {
				// Selection up
				if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nUpSelectionID != -1 ) {
					// This is a valid selection
					_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nUpSelectionID;
				}
			} else {
				// Selection down
				if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nDownSelectionID != -1 ) {
					// This is a valid selection
					_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nDownSelectionID;
				}
			}
		} else if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_DPAD_Y ]->uLatches &
				FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
			if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_DPAD_Y ]->fCurrentState > 0.1f ) {
				// Selection up
				if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nUpSelectionID != -1 ) {
					// This is a valid selection
					_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nUpSelectionID;
				}
			} else {
				// Selection down
				if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nDownSelectionID != -1 ) {
					// This is a valid selection
					_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nDownSelectionID;
				}
			}
		} else {
			// see if we should change the current selection (left/right)
			if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_LEFT_ANALOG_X ]->uLatches &
				FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {

				if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_LEFT_ANALOG_X ]->fCurrentState > 0.1f ) {
					// Selection right
					if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nRightSelectionID != -1 ) {
						// This is a valid selection
						_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nRightSelectionID;
					}
				} else {
					// Selection left
					if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nLeftSelectionID != -1 ) {
						// This is a valid selection
						_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nLeftSelectionID;
					}
				}		
			} else if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_DPAD_X ]->uLatches &
					FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) {
				if( Gamepad_aapSample[ i ][ GAMEPAD_MENU_DPAD_X ]->fCurrentState > 0.1f ) {
					// Selection right
					if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nRightSelectionID != -1 ) {
						// This is a valid selection
						_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nRightSelectionID;
					}
				} else {
					// Selection left
					if( _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nLeftSelectionID != -1 ) {
						// This is a valid selection
						_pMenuLangData->nCurrentSelection = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].nLeftSelectionID;
					}
				}
			}
		}
	}
	return TRUE;
}


static BOOL _FangModeWork( void ) {

	u32 i;

	FASSERT( _pFangLangData );

	// Get the Fang selected language
	u32 nCaps = fsysinfo_GetCapabilities();

	//Now, run through the Fang language table and see if we have found our language ID
	for( i = 0; i < _pFangLangData->nTableEntries; i++ ) {
		_FangLangEntry_t *pLangEntry = &_pFangLangData->paLangEntries[ i ];
		if( pLangEntry->nXBLangID & nCaps ) {
			// Set the language ID here...
			if( _pFangLangData->cReferenceChar != pLangEntry->cLangChar ) {
				// Set the localization data
				ffile_SetLanguageChars( _pFangLangData->cReferenceChar, pLangEntry->cLangChar, pLangEntry->cAudioLangChar );
			}
			break;
		}
	}
	
	
	if( !_SwitchToLoadingMode() ) {
		launcher_LoadFailure();
	}
	return TRUE;
}


static BOOL _Draw( void ) {

	if(	_eLangSelectMode == _LANGSELECT_LOADING_MODE ) {
		// Just call the wrapper loading draw function here...
		launcher_DrawLoadingScreen();
		return TRUE;
	};

	// If we are here, then we are drawing a menu...
	FASSERT( _eLangSelectMode == _LANGSELECT_MENU_MODE );

	CFVec2 Lower, Upper;
	FDrawVtx_t aVtx[4];// used by all fdraw functions

	FASSERT( _bSystemInitialized );

	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
	

	FViewport_t *pPrevViewport = fviewport_SetActive( _pViewportOrtho3D );
	FXfm_Identity.InitStackWithView();


	// Draw fdraw objects
	frenderer_Push( FRENDERER_DRAW, NULL );

	// Draw the background texture map fullscreen left to right... the top
	// and bottom will clip
	wpr_drawutils_DrawTextureToScreen( TRUE,
						_pMenuLangData->pBackgroundTexInst,
						0.00f, 0.0f,
						2.0,
						_pViewportOrtho3D->Res.x * (1.0f/640.0f),
						_pViewportOrtho3D->HalfRes.x,
						_pViewportOrtho3D->HalfRes.x ); // This ensures it draws fullscreen

	// Now draw the Flags TGA to the screen, based on the size the CSV defines
	wpr_drawutils_DrawTextureToScreen( TRUE,
						_pMenuLangData->pLanguagesTexInst,
						0.00f, 0.0f,
						_pMenuLangData->fLanguagesTGATextureHeight,//2.3f,
						_pViewportOrtho3D->Res.x * (1.0f/640.0f),
						_pViewportOrtho3D->HalfRes.x,
						_pViewportOrtho3D->HalfRes.y );

	// Now, draw the highlight
	FTexDef_t *pLanguagesTexDef = _pMenuLangData->pLanguagesTexInst->GetTexDef();
	f32 fUX, fUY, fLX, fLY;
	fUX = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].fUpperLeftX / pLanguagesTexDef->TexInfo.nTexelsAcross;
	fUY = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].fUpperLeftY / pLanguagesTexDef->TexInfo.nTexelsDown;
	fLX = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].fLowerRightX / pLanguagesTexDef->TexInfo.nTexelsAcross;
	fLY = _pMenuLangData->paLangEntries[ _pMenuLangData->nCurrentSelection ].fLowerRightY / pLanguagesTexDef->TexInfo.nTexelsDown;

	//Now, convert these coordinates to orthographic coordinates
	wpr_drawutils_ConvertTextCoordsToOrthoCoords( fUX, fUY, Upper.x, Upper.y, _pViewportOrtho3D->HalfRes.y, _pViewportOrtho3D->HalfRes.y ); 
	wpr_drawutils_ConvertTextCoordsToOrthoCoords( fLX, fLY, Lower.x, Lower.y, _pViewportOrtho3D->HalfRes.y, _pViewportOrtho3D->HalfRes.y ); 

	//Now, scale the ortho coordinates by half the texture height
	Upper *= ( _pMenuLangData->fLanguagesTGATextureHeight * 0.5f * _pViewportOrtho3D->HalfRes.y );
	Lower *= ( _pMenuLangData->fLanguagesTGATextureHeight * 0.5f * _pViewportOrtho3D->HalfRes.y );

	// Now, we have the orthographic coordinates of the region of the selected box... draw our highlight around it!

	// Next, Figure out the coordinates to use for the highlight selection here....
	aVtx[0].ST.Set( 0.0f, 1.0f );
	aVtx[1].ST.Set( 0.0f, 0.0f );
	aVtx[2].ST.Set( 1.0f, 1.0f );
	aVtx[3].ST.Set( 1.0f, 0.0f );

	CFColorRGBA Color;
	Color.Set( 1.0f, 1.0f, 1.0f, 1.0f );

	aVtx[0].Pos_MS.Set( Lower.x, Lower.y, 1.0f ); 
	aVtx[0].ColorRGBA = Color;
		
	aVtx[1].Pos_MS.Set( Lower.x, Upper.y, 1.0f );						
	aVtx[1].ColorRGBA = Color;

	aVtx[2].Pos_MS.Set( Upper.x, Lower.y, 1.0f );
	aVtx[2].ColorRGBA = Color;

	aVtx[3].Pos_MS.Set( Upper.x, Upper.y, 1.0f );
	aVtx[3].ColorRGBA = Color;

	fdraw_SetTexture( _pMenuLangData->pHighlightTexInst );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );

	// Pop the fdraw renderer off the rendering stack 
	frenderer_Pop();
	
	// Restore the viewport
	fviewport_SetActive( pPrevViewport );

	return TRUE;
}


static BOOL _SwitchToLoadingMode( void ) {

	// First, unload all of our loaded resources
	// and load the bare bones localized resources
	if( !_UnloadResources( TRUE ) ) {
		return FALSE;
	}

	_eLangSelectMode = _LANGSELECT_LOADING_MODE;
	_nLoadingFrameCounter = 0;

	return TRUE;
}


static BOOL _UnloadResources( BOOL bLoadFontsAndGamephrase ) {

	if( _pMenuLangData ) {
		// Release any GC Specific resources here
		if( _pMenuLangData->pBackgroundTexInst ) {
			fdelete( _pMenuLangData->pBackgroundTexInst );
			_pMenuLangData->pBackgroundTexInst = NULL;		
		}

		if( _pMenuLangData->pLanguagesTexInst ) {
			fdelete( _pMenuLangData->pLanguagesTexInst );
			_pMenuLangData->pLanguagesTexInst = NULL;		
		}

		if( _pMenuLangData->pHighlightTexInst ) {
			fdelete( _pMenuLangData->pHighlightTexInst );
			_pMenuLangData->pHighlightTexInst = NULL;		
		}

		_pMenuLangData = NULL;
	}

	if( _pFangLangData ) {
		// Release any XBOX specific resources here
		_pFangLangData = NULL;
	}

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

	// finally load the fonts and gamephrases if we need to...
	if( bLoadFontsAndGamephrase ) {
		// Now that a local has been determined, load the rest of the local specific systems
		if( !game_LoadLocalizedPhraseTableAndFonts() ) {
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL _LoadLanguageSelectCSV( void ) {
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;

	// First, open up the file
	// grab an fres and fmem frame
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the game csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _LANGUAGE_SELECT_CSVNAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "wpr_languageselect : _LoadLanguageSelectCSV() : Could not load the language selection csv file '%s'.\n", _LANGUAGE_SELECT_CSVNAME );
		goto _ExitWithError;
	}

	if( !_ParseEnableLangSelectTable( hFile ) ) {
		goto _ExitWithError;
	}

	if( _eLangSelectMode == _LANGSELECT_MENU_MODE ) {
		if( !_LoadMenuLanguageData( hFile ) ) {
			goto _ExitWithError;
		}
	} else if ( _eLangSelectMode == _LANGSELECT_FANG_MODE ) {
		if( !_LoadFangLanguageData( hFile ) ) {
			goto _ExitWithError;
		}
	}

	fmem_ReleaseFrame( hMemFrame );
	return TRUE;

_ExitWithError:

	fmem_ReleaseFrame( hMemFrame );
	return FALSE;
}


static BOOL _ParseEnableLangSelectTable( FGameDataFileHandle_t hFile ) {
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nFieldsInTable;

	// Determine if the language selection system is even supposed to be active...
	hTable = fgamedata_GetFirstTableHandle( hFile, _LANG_SELECT_MODE_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_languageselect : _ParseEnableLangSelectTable() : Could not find the Enable language selection table named '%s'.\n", _LANG_SELECT_MODE_TABLE_NAME );
		return FALSE;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nFieldsInTable = fgamedata_GetNumFields( hTable );
	if( nFieldsInTable != 1 ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "wpr_languageselect : _ParseEnableLangSelectTable() : Error! -- Expecting 1 field in the table %s.  Found %d fields in table instead.\n",
			       _LANG_SELECT_MODE_TABLE_NAME, nFieldsInTable );
		return FALSE;
	}

	f32 fActive = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 0 , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_languageselect : _ParseEnableLangSelectTable() : Field 0 in table %s is not a FLOAT format!\n", _LANG_SELECT_MODE_TABLE_NAME );
		return FALSE;
	}

	// Now, verify that fActive is either 0.0 or 1.0, or 2.0f
	if( fActive == 0.0f ) {
		_eLangSelectMode = _LANGSELECT_DISABLED;
	} else if( fActive == 1.0f ) {
		_eLangSelectMode = _LANGSELECT_MENU_MODE;
	} else if( fActive == 2.0f ) {
		_eLangSelectMode = _LANGSELECT_FANG_MODE;
	} else {
		DEVPRINTF("wpr_languageselect : _ParseEnableLangSelectTable() : Field 0 in table %s is not 0, 1, or 2!\n", _LANG_SELECT_MODE_TABLE_NAME );
		return FALSE;
	}

	return TRUE;
}



static char _ParseReferenceCharTable( FGameDataFileHandle_t hFile ) {
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nFieldsInTable;
	cchar* pszReferenceChar;

	hTable = fgamedata_GetFirstTableHandle( hFile, _REFERENCE_CHAR_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_languageselect : _ParseReferenceCharTable() : Could not find the Reference Character table named '%s'.\n", _REFERENCE_CHAR_TABLE_NAME );
		return 0;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nFieldsInTable = fgamedata_GetNumFields( hTable );
	if( nFieldsInTable != 1 ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "wpr_languageselect : _ParseReferenceCharTable() : Error! -- Expecting 1 field in the table %s.  Found %d fields in table instead.\n",
			       _REFERENCE_CHAR_TABLE_NAME, nFieldsInTable );
		return 0;
	}

	pszReferenceChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("wpr_languageselect : _ParseReferenceCharTable() : The Field 0 in table %s is not a STRING format!\n", _REFERENCE_CHAR_TABLE_NAME );
		return 0;
	}

	// Now, verify that pszReferenceChar is only 1 character long
	if( fclib_strlen( pszReferenceChar ) != 1 ) {
		DEVPRINTF("wpr_languageselect : _ParseReferenceCharTable() : String Field 0 in table %s is not one character only in length!\n", _REFERENCE_CHAR_TABLE_NAME );
		return 0;
	}

	return pszReferenceChar[ 0 ];
}


static BOOL _LoadMenuLanguageData( FGameDataFileHandle_t hFile ) {

	// Create the GC language structure here...
	_pMenuLangData = ( _MenuLangData_t* )fres_AllocAndZero( sizeof( _MenuLangData_t ) );
	if( !_pMenuLangData ) {
		DEVPRINTF( "wpr_languageselect : _LoadMenuLanguageData() : Could not allocate _pMenuLangData!\n" );
		return FALSE;
	}

	// Load the reference character
	_pMenuLangData->cReferenceChar = _ParseReferenceCharTable( hFile );
	if( !_pMenuLangData->cReferenceChar ) {
		return FALSE;
	}


	// Parse the menu data table
	if( !_ParseMenuDataTable( hFile ) ) {
		return FALSE;
	}

	// parse the GCMenuItems table
	if( !_ParseMenuItemsTable( hFile ) ) {
		return FALSE;
	}
	return TRUE;
}


static BOOL _ParseMenuDataTable( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	cchar* pszBackgroundTGAName;
	cchar* pszLanguagesTGAName;
	cchar* pszHighlightTGAName;

	// Now, find the GC Menu data table
	hTable = fgamedata_GetFirstTableHandle( hFile, _MENU_DATA_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_languageselect : _LoadMenuLanguageData() : Could not find the GC menu data table named '%s'.\n", _MENU_DATA_TABLE_NAME );
		return FALSE;
	}

	// verify the number of fields in the table 
	if( fgamedata_GetNumFields( hTable ) != _NUM_MENU_DATA_TABLE_ENTRIES ) {
		DEVPRINTF( "wpr_languageselect : _LoadMenuLanguageData() : Expecting exactly %d fields in the %s table.\n", _NUM_MENU_DATA_TABLE_ENTRIES, _MENU_DATA_TABLE_NAME );
		return FALSE;
	}

	// Get the Background texture TGA name from the table.
	pszBackgroundTGAName = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("wpr_languageselect : _LoadMenuLanguageData() : The Background TGA texture name (field 0) in table %s is not a STRING format!\n", _MENU_DATA_TABLE_NAME );
		return FALSE;
	}

	// Get the Languages texture TGA name from the table.
	pszLanguagesTGAName = (cchar *)fgamedata_GetPtrToFieldData( hTable, 1, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("wpr_languageselect : _LoadMenuLanguageData() : The Languages TGA texture name (field 1) in table %s is not a STRING format!\n", _MENU_DATA_TABLE_NAME );
		return FALSE;
	}

	// Get the Highlight texture TGA name from the table.
	pszHighlightTGAName = (cchar *)fgamedata_GetPtrToFieldData( hTable, 2, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("wpr_languageselect : _LoadMenuLanguageData() : The Highlight TGA texture name (field 2) in table %s is not a STRING format!\n", _MENU_DATA_TABLE_NAME );
		return FALSE;
	}

	// Get the Languages TGA RenderHeight value
	_pMenuLangData->fLanguagesTGATextureHeight = *( f32* ) fgamedata_GetPtrToFieldData( hTable, 3 , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_languageselect : _ParseMenuDataTable() : Field 3 in table %s is not a FLOAT format!\n", _MENU_DATA_TABLE_NAME );
		return FALSE;
	}

	// Get the Default Selection value
	_pMenuLangData->nCurrentSelection = ( u32 ) *( f32* ) fgamedata_GetPtrToFieldData( hTable, 4 , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
		DEVPRINTF("wpr_languageselect : _ParseMenuDataTable : Field 4 in table %s is not a FLOAT format!\n", _MENU_DATA_TABLE_NAME );
		return FALSE;
	}

	// lastly, create the texture assets
	_pMenuLangData->pBackgroundTexInst = fnew CFTexInst;
	if( !_pMenuLangData->pBackgroundTexInst ) {
		DEVPRINTF("wpr_languageselect : _ParseMenuDataTable : Could not create a Background CFTexInst!\n" );
		return FALSE;
	}
	_pMenuLangData->pBackgroundTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszBackgroundTGAName ) );	

	_pMenuLangData->pLanguagesTexInst = fnew CFTexInst;
	if( !_pMenuLangData->pLanguagesTexInst ) {
		DEVPRINTF("wpr_languageselect : _ParseMenuDataTable : Could not create a Languages CFTexInst!\n" );
		return FALSE;
	}
	_pMenuLangData->pLanguagesTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszLanguagesTGAName ) );	

	_pMenuLangData->pHighlightTexInst = fnew CFTexInst;
	if( !_pMenuLangData->pHighlightTexInst ) {
		DEVPRINTF("wpr_languageselect : _ParseMenuDataTable : Could not create a Highlight CFTexInst!\n" );
		return FALSE;
	}
	_pMenuLangData->pHighlightTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, pszHighlightTGAName ) );	

	return TRUE;
}

static BOOL _ParseMenuItemsTable( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 i;
	cchar *pszLanguageChar;
	cchar *pszAudioLanguageChar;

	// Now, find the GC Menu items table
	hTable = fgamedata_GetFirstTableHandle( hFile, _MENU_ITEMS_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_languageselect : _ParseMenuItemsTable : Could not find the GC menu data table named '%s'.\n", _MENU_ITEMS_TABLE_NAME );
		return FALSE;
	}

	// verify the number of fields in the table 
	u32 nFieldCount = fgamedata_GetNumFields( hTable );
	if( nFieldCount % _NUM_ITEMS_FIELDS_PER_ENTRY ) {
		DEVPRINTF( "wpr_languageselect : _ParseMenuItemsTable : Expecting exactly %d fields per menu entry in the %s table.\n", _NUM_ITEMS_FIELDS_PER_ENTRY, _MENU_ITEMS_TABLE_NAME );
		return FALSE;
	}

	// Figure out the number of menu entries...
	u32 nNumEntries = nFieldCount / _NUM_ITEMS_FIELDS_PER_ENTRY;
	_pMenuLangData->nNumMenuItems = nNumEntries;

	// Allocate memory for the menu items...
	_pMenuLangData->paLangEntries = ( _MenuLangEntry_t* )fres_AllocAndZero( nNumEntries * sizeof( _MenuLangEntry_t ) );
	if( !_pMenuLangData->paLangEntries ) {
		DEVPRINTF( "wpr_languageselect : _ParseMenuItemsTable : Could not allocate _pMenuLangData->paLangEntries!\n" );
		return FALSE;
	}


	// Now load the individual menu entries...
	for( i = 0; i < nNumEntries; i++ ) {
		_MenuLangEntry_t *pEntry = &_pMenuLangData->paLangEntries[ i ];

		// Get the Upper left corner X value
		pEntry->fUpperLeftX = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 0 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 0, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}

		// Get the Upper left corner Y value
		pEntry->fUpperLeftY = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 1 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 1, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}

		// Get the Lower right corner X value
		pEntry->fLowerRightX = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 2 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 2, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}

		// Get the Lower right corner Y value
		pEntry->fLowerRightY = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 3 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 3, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}

		// Load the language specific character
		pszLanguageChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 4, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a STRING format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 4, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}
		pEntry->cLangChar = pszLanguageChar[ 0 ];

		// Load the audio language specific character
		pszAudioLanguageChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 5, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a STRING format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 5, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}
		pEntry->cAudioLangChar = pszAudioLanguageChar[ 0 ];

		// Load the stick left selection ID
		pEntry->nLeftSelectionID = ( s32 ) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 6 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 6, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}

		// Load the stick Right selection ID
		pEntry->nRightSelectionID = ( s32 ) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 7 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 7, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}

		// Load the stick Up selection ID
		pEntry->nUpSelectionID = ( s32 ) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 8 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 8, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}

		// Load the stick Down selection ID
		pEntry->nDownSelectionID = ( s32 ) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_ITEMS_FIELDS_PER_ENTRY + 9 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _ParseMenuItemsTable() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_ITEMS_FIELDS_PER_ENTRY + 9, _MENU_ITEMS_TABLE_NAME );
			return FALSE;
		}
	}

	return TRUE;
}


static BOOL _LoadFangLanguageData( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 i;
	cchar *pszLanguageChar;
	cchar *pszAudioLanguageChar;

	// Create the Fang language structure here...
	_pFangLangData = ( _FangLangData_t* )fres_AllocAndZero( sizeof( _FangLangData_t ) );
	if( !_pFangLangData ) {
		DEVPRINTF( "wpr_languageselect : _LoadXBLanguageData() : Could not allocate _pFangLangData!\n" );
		return FALSE;
	}

	// Load the reference character
	_pFangLangData->cReferenceChar = _ParseReferenceCharTable( hFile );
	if( !_pFangLangData->cReferenceChar ) {
		return FALSE;
	}

	// Now, load the language table...
	hTable = fgamedata_GetFirstTableHandle( hFile, _FANG_LANGUAGE_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "wpr_languageselect : _LoadXBLanguageData() : Could not find the XBox language data table named '%s'.\n", _FANG_LANGUAGE_TABLE_NAME );
		return FALSE;
	}

	// verify the number of fields in the table 
	u32 nFieldCount = fgamedata_GetNumFields( hTable );
	if( nFieldCount % _NUM_FANG_FIELDS_PER_LANG_ENTRY ) {
		DEVPRINTF( "wpr_languageselect : _LoadXBLanguageData() : Expecting exactly %d fields per menu entry in the %s table.\n", _NUM_FANG_FIELDS_PER_LANG_ENTRY, _FANG_LANGUAGE_TABLE_NAME );
		return FALSE;
	}

	// Figure out the number of menu entries...
	u32 nNumEntries = nFieldCount / _NUM_FANG_FIELDS_PER_LANG_ENTRY;
	_pFangLangData->nTableEntries = nNumEntries;

	// Allocate memory for the table entries.
	_pFangLangData->paLangEntries = ( _FangLangEntry_t* )fres_AllocAndZero( nNumEntries * sizeof( _FangLangEntry_t ) );
	if( !_pFangLangData->paLangEntries ) {
		DEVPRINTF( "wpr_languageselect : _LoadXBLanguageData() : Could not allocate _pFangLangData->paLangEntries!\n" );
		return FALSE;
	}

	// Now load the individual table entries...
	u32 uTempVal;
	for( i = 0; i < nNumEntries; i++ ) {
		_FangLangEntry_t *pEntry = &_pFangLangData->paLangEntries[ i ];

		// Get the Upper left corner X value
		uTempVal = ( u32 ) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_FANG_FIELDS_PER_LANG_ENTRY + 0 , nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT) {
			DEVPRINTF("wpr_languageselect : _LoadXBLanguageData() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_FANG_FIELDS_PER_LANG_ENTRY + 0, _FANG_LANGUAGE_TABLE_NAME );
			return FALSE;
		}
		pEntry->nXBLangID = ( 1 << uTempVal ); 

		// Load the language specific character
		pszLanguageChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_FANG_FIELDS_PER_LANG_ENTRY + 1, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING) {
			DEVPRINTF("wpr_languageselect : _LoadXBLanguageData() : Field %d in table %s is not a STRING format!\n", _NUM_FANG_FIELDS_PER_LANG_ENTRY + 1, _FANG_LANGUAGE_TABLE_NAME );
			return FALSE;
		}
		pEntry->cLangChar = pszLanguageChar[ 0 ];

		// Load the audio language specific character
		pszAudioLanguageChar = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_FANG_FIELDS_PER_LANG_ENTRY + 2, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING) {
			DEVPRINTF("wpr_languageselect : _LoadXBLanguageData() : Field %d in table %s is not a STRING format!\n", _NUM_FANG_FIELDS_PER_LANG_ENTRY + 2, _FANG_LANGUAGE_TABLE_NAME );
			return FALSE;
		}
		pEntry->cAudioLangChar = pszAudioLanguageChar[ 0 ];
	}

	return TRUE;
}
