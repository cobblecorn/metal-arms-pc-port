//////////////////////////////////////////////////////////////////////////////////////
// gamecam.cpp - 
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
// 01/28/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "gamecam.h"
#include "gamepad.h"
#include "ftext.h"
#include "player.h"
#include "SplitScreen.h"

// list cameraman include files here
#include "CamBot.h"
#include "CamDebug.h"
#include "CamManual.h"
#include "CamSimple.h"
#include "CamCutscene.h"
#include "botglitch.h"

//====================
// private definitions

//////////////////////////////////////////////////////////////////////////////////////
// THESE DEFINES ARE KEY, PLEASE UPDATE WHEN ADDING NEW CAMERAMEN
#define _NUM_BOT_CAMERAMEN			GAME_CAM_MAX_COUNT
#if CAM_DEBUG_ENABLE
	#define _NUM_DEBUG_CAMERAMEN		1
#else
	#define _NUM_DEBUG_CAMERAMEN		0	
#endif
#define _NUM_MANUAL_CAMERAMEN		GAME_CAM_MAX_COUNT
#define _NUM_SIMPLE_CAMERAMEN		GAME_CAM_MAX_COUNT
#define _NUM_CUTSCENE_CAMERAMEN		1
//////////////////////////////////////////////////////////////////////////////////////

#define _TOTAL_NUM_CAMERAS			( GAME_CAM_MAX_COUNT + _NUM_DEBUG_CAMERAMEN )
#define _DEBUG_CAMERA_INDEX			( _TOTAL_NUM_CAMERAS - 1 )

#define _REQUIRED_FIELDS_PER_BOT		( sizeof( CamBotInfo_t ) / 4 )
static cchar *_pszCSVFileName = "b_cam_data";
static cchar *_pszTableName = "bot_cam_data";



////////////////////////////////////////////////////
// ATTENTION PROGRAMMERS: IF YOU ARE LOOKING TO 
// TWEAK SOME BOT'S CAMERA SETTINGS, YOU WILL NEED
// TO CHECK OUT AND EDIT THE CSV FILE NAMED:
// B_CAM_DATA.CSV
////////////////////////////////////////////////////



//=================
// public variables

//==================
// private variables

static BOOL8 _bInitOK = FALSE;
static BOOL8 _bWaitForDebugButtonsToBeUp = TRUE;
static BOOL8 _bWaitForStartButtonToBeUp = TRUE;
static s8 _nPlayerToMonitorForDebugging = GAMECAM_DONT_MONITOR_CONTROLS;
static u8 _nControllerPortToMonitor = 0;
static GameCam_DebugModes_e _nDebugMode = GAMECAM_DEBUG_MODES_NOT_DEBUGGING;
static CCamBot *_paBotCameramen = NULL;
#if CAM_DEBUG_ENABLE
	static CCamDebug *_paDebugCameramen = NULL;
	static CFCameraMan *_pCamManBeingDebugged = NULL;
#endif
static GameCamPlayer_e _nActivePlayer;
static GameCamType_e _anCurrentCameraType[_TOTAL_NUM_CAMERAS];
static CCamManual *_paManualCameramen = NULL;
static CCamSimple *_paSimpleCameramen = NULL;
static CCamCutscene *_paCutsceneCameramen = NULL;
static CFCameraMan *_apCameraMen[GAME_CAM_MAX_COUNT];

static CamBotInfo_t *_paCamBotInfos = NULL;

//===================
// private prototypes

static void _ShowPlayerCameras( BOOL bShow );
static void _DoNotUpdateBot( BOOL bEnable );
static void _GetOutOfDebugCam( void );
static BOOL _CheckForEnterDebugMode();

//=================
// public functions

// Call once at bootup, this function will create GAME_CAM_MAX_COUNT players cameras.
// This function will initialize the camera system, so the viewport system must
// be initialized before calling.
BOOL gamecam_InitSystem( void ) {
	u32 i, nNumFields, nNumBots, nIndex;
	FResFrame_t ResFrame;
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;

	FASSERT( !_bInitOK );

	ResFrame = fres_GetFrame();
	hMemFrame = fmem_GetFrame();

	/////////////////////////////////////////////
	// load the wrapper csv file into temp memory
	hFile = fgamedata_LoadFileToFMem( _pszCSVFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "gamecam_InitSystem() : Could not load the bot cam csv file '%s'.\n", _pszCSVFileName );
		goto _ExitSetupWithError;
	}
	// find the table
	hTable = fgamedata_GetFirstTableHandle( hFile, _pszTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "gamecam_InitSystem() : Could not find the bot cam table named '%s'.\n", _pszTableName );
		goto _ExitSetupWithError;
	}
	// get the number of fields in the table
	nNumFields = fgamedata_GetNumFields( hTable );
	nNumBots = nNumFields / _REQUIRED_FIELDS_PER_BOT;
	if( (nNumBots * _REQUIRED_FIELDS_PER_BOT) != nNumFields || (nNumBots == 0) ) {
		DEVPRINTF( "gamecam_InitSystem() : The bot cam table '%s' didn't contain %d entries per bot as expected.\n", _pszTableName, _REQUIRED_FIELDS_PER_BOT );
		goto _ExitSetupWithError;
	}
	// warn the user about too many or too few bots
	FASSERT( nNumBots == BOTCLASS_COUNT );
	if( nNumBots < BOTCLASS_COUNT ) {
		DEVPRINTF( "gamecam_InitSystem() : The bot cam csv file '%s' contains info for %d bots, but there are %d, defaulting unsupplied bots to glitch settings.\n", _pszCSVFileName, nNumBots, BOTCLASS_COUNT );
	} else if( nNumBots > BOTCLASS_COUNT ) {
		DEVPRINTF( "gamecam_InitSystem() : The bot cam csv file '%s' contains info for %d bots, but the code only has %d, ignoring the extra bots.\n", _pszCSVFileName, nNumBots, BOTCLASS_COUNT );
		nNumBots = BOTCLASS_COUNT;
	}
	// allocate an array to hold the bot cam data
	_paCamBotInfos = (CamBotInfo_t *)fres_Alloc( sizeof( CamBotInfo_t ) * BOTCLASS_COUNT );
	if( !_paCamBotInfos ) {
		DEVPRINTF( "gamecam_InitSystem() : Could not allocate memory to hold the bot cam csv data.\n" );
		goto _ExitSetupWithError;
	}
	// fill in table entry structs
	FGameData_TableEntry_t DegreeEntry, FloatEntry;
	
	DegreeEntry.nFlags = (FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS);
	DegreeEntry.nBytesForData = sizeof( f32 );

	FloatEntry.nFlags = (FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_FLOAT_X);
	FloatEntry.nBytesForData = sizeof( f32 );

	// load bot cam data that resides in the csv file
	nIndex = 0;
	for( i=0; i < nNumBots; i++ ) {
		if( !fgamedata_GetFieldFromTable( hTable, nIndex++, &DegreeEntry, &_paCamBotInfos[i].fHalfFOV ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &DegreeEntry, &_paCamBotInfos[i].fHalfFOV_MP ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].TargetPos_MS.x ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].TargetPos_MS.y ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].TargetPos_MS.z ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].Pos_MS.x ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].Pos_MS.y ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].Pos_MS.z ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].fPosScale_MP ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].fLookupYOffset ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_paCamBotInfos[i].fClosestDistToTarget ) ) {
			DEVPRINTF( "gamecam_InitSystem() : Could not properly read the bot cam csv data.\n" );
			goto _ExitSetupWithError;
		}
	}
	// init any bots that weren't in the file
	for( i=nNumBots; i < BOTCLASS_COUNT; i++ ) {
		_paCamBotInfos[i] = _paCamBotInfos[0];
	}
#if FANG_WINGC
	// Port diagnostic: the bot camera table read from the retail CSV.
	for( i=0; Fang_bPortDiag && i < 2 && i < nNumBots; i++ ) {
		DEVPRINTF( "PORT-CAM bot %u: fov=%.3f/%.3f target=(%.2f,%.2f,%.2f) pos=(%.2f,%.2f,%.2f) scaleMP=%.2f lookupY=%.2f closest=%.2f (%u bots, %u fields)\n",
			i, _paCamBotInfos[i].fHalfFOV, _paCamBotInfos[i].fHalfFOV_MP,
			_paCamBotInfos[i].TargetPos_MS.x, _paCamBotInfos[i].TargetPos_MS.y, _paCamBotInfos[i].TargetPos_MS.z,
			_paCamBotInfos[i].Pos_MS.x, _paCamBotInfos[i].Pos_MS.y, _paCamBotInfos[i].Pos_MS.z,
			_paCamBotInfos[i].fPosScale_MP, _paCamBotInfos[i].fLookupYOffset, _paCamBotInfos[i].fClosestDistToTarget,
			nNumBots, nNumFields );
	}
#endif

	///////////////////////////////////////
	// set our vars to their default values
	_nPlayerToMonitorForDebugging = GAMECAM_DONT_MONITOR_CONTROLS;
	_nControllerPortToMonitor = 0;
	_nDebugMode = GAMECAM_DEBUG_MODES_NOT_DEBUGGING;
	_bWaitForDebugButtonsToBeUp = TRUE;
	_bWaitForStartButtonToBeUp = TRUE;
	_paBotCameramen = NULL;
#if CAM_DEBUG_ENABLE
	_paDebugCameramen = NULL;
	_pCamManBeingDebugged = NULL;
#endif
	_paManualCameramen = NULL;
	_paSimpleCameramen = NULL;
	_paCutsceneCameramen = NULL;
	_nActivePlayer = GAME_CAM_PLAYER_1;
	for( i=0; i < _TOTAL_NUM_CAMERAS; i++ ) {
		_anCurrentCameraType[i] = GAME_CAM_TYPE_NOT_IN_USE;
	}

	for( i=0; i<GAME_CAM_MAX_COUNT; ++i ) {
		_apCameraMen[i] = NULL;
	}

	////////////////////
	// call system inits
	if( !CCamBot::InitSystem() ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not initialize the bot cam system.\n" );	
		goto _ExitSetupWithError;
	}
	if( !CCamManual::InitSystem() ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not initialize the manual cam system.\n" );	
		goto _ExitSetupWithError;
	}
	if( !CCamSimple::InitSystem() ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not initialize the simple cam system.\n" );	
		goto _ExitSetupWithError;
	}
	if( !CCamCutscene::InitSystem() ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not initialize the cutscene cam system.\n" );	
		goto _ExitSetupWithError;
	}

	////////////////////
	// allocate cameras
	if( !fcamera_InitSystem( _TOTAL_NUM_CAMERAS ) ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not intialize the camera system for %d cameras.\n", _TOTAL_NUM_CAMERAS );	
		goto _ExitSetupWithError;
	}

	/////////////////////
	// allocate cameramen
	_paBotCameramen = fnew CCamBot[_NUM_BOT_CAMERAMEN];
	if( !_paBotCameramen ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not allocate %d bot cameramen.\n", _NUM_BOT_CAMERAMEN );
		goto _ExitSetupWithError;
	}
#if CAM_DEBUG_ENABLE
	_paDebugCameramen = fnew CCamDebug[_NUM_DEBUG_CAMERAMEN];
	if( !_paDebugCameramen ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not allocate %d debug cameramen.\n", _NUM_DEBUG_CAMERAMEN );
		goto _ExitSetupWithError;
	}	
#endif
	_paManualCameramen = fnew CCamManual[_NUM_MANUAL_CAMERAMEN];
	if( !_paManualCameramen ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not allocate %d manual cameramen.\n", _NUM_MANUAL_CAMERAMEN );
		goto _ExitSetupWithError;
	}
	_paSimpleCameramen = fnew CCamSimple[_NUM_SIMPLE_CAMERAMEN];
	if( !_paSimpleCameramen ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not allocate %d simple cameramen.\n", _NUM_SIMPLE_CAMERAMEN );
		goto _ExitSetupWithError;
	}
	_paCutsceneCameramen = fnew CCamCutscene[_NUM_CUTSCENE_CAMERAMEN];
	if( !_paCutsceneCameramen ) {
		DEVPRINTF( "gamecam_InitSystem(): Could not allocate %d cutscene cameramen.\n", _NUM_CUTSCENE_CAMERAMEN );
		goto _ExitSetupWithError;
	}

	// mark the system as up and ready to be used
	_bInitOK = TRUE;

	// release our fmem frame so that the temp csv file is unloaded
	fmem_ReleaseFrame( hMemFrame );

	return TRUE;

_ExitSetupWithError:
	gamecam_UninitSystem();
	fres_ReleaseFrame( ResFrame );
	fmem_ReleaseFrame( hMemFrame );
	return FALSE;
}

void gamecam_UninitSystem( void ) {
	u32 i;

	_nPlayerToMonitorForDebugging = GAMECAM_DONT_MONITOR_CONTROLS;
	_nControllerPortToMonitor = 0;
	_nDebugMode = GAMECAM_DEBUG_MODES_NOT_DEBUGGING;
	_bWaitForDebugButtonsToBeUp = TRUE;
	_bWaitForStartButtonToBeUp = TRUE;
	if( _paBotCameramen ) {
		fdelete_array( _paBotCameramen );
		_paBotCameramen = NULL;
	}
#if CAM_DEBUG_ENABLE
	if( _paDebugCameramen ) {
		fdelete_array( _paDebugCameramen );
		_paDebugCameramen = NULL;
	}
	_pCamManBeingDebugged = NULL;
#endif
	if( _paManualCameramen ) {
		fdelete_array( _paManualCameramen );
		_paManualCameramen = NULL;
	}
	if( _paSimpleCameramen ) {
		fdelete_array( _paSimpleCameramen );
		_paSimpleCameramen = NULL;
	}
	if( _paCutsceneCameramen ) {
		fdelete_array( _paCutsceneCameramen );
		_paCutsceneCameramen = NULL;
	}
	_nActivePlayer = GAME_CAM_PLAYER_1;
	for( i=0; i < _TOTAL_NUM_CAMERAS; i++ ) {
		_anCurrentCameraType[i] = GAME_CAM_TYPE_NOT_IN_USE;
	}
	
	if( _bInitOK ) {
		fcamera_UninitSystem();
		_bInitOK = FALSE;
	}

	_paCamBotInfos = NULL;
	// uninit the bot cam system
	CCamBot::UninitSystem();
}

// Call after each level has been loaded.  This function will scan the loaded
// world and init needed level specific classes.
// Calling this function will also unattach all cameramen from their cameras.
BOOL gamecam_InitLevelCameras( s32 nAllowDebuggingByPlayer/*=GAMECAM_DONT_MONITOR_CONTROLS*/,
							  u32 nControllerPortOfPlayer/*=0*/ ) {

	FASSERT( _bInitOK );

	SetPlayerToAllowDebugging( nAllowDebuggingByPlayer, nControllerPortOfPlayer );

	// init any placed scenes that reside in the level



	return TRUE;
}

void gamecam_ExitAnyDebugCam( void ) {
	FASSERT( _bInitOK );

	// if we were in debug mode, restore the regular camera.
	_GetOutOfDebugCam();
}

// Call at the end of each level.  This call will return the gamecam system back
// to how the state was before the last call to gamecam_InitLevelCameras().
// Calling this function will also unattach all cameramen from their cameras.
void gamecam_PostLevelCleanup( void ) {
	u32 i;

	FASSERT( _bInitOK );

	_nPlayerToMonitorForDebugging = GAMECAM_DONT_MONITOR_CONTROLS;
	_nControllerPortToMonitor = 0;
	_nDebugMode = GAMECAM_DEBUG_MODES_NOT_DEBUGGING;
	_bWaitForDebugButtonsToBeUp = TRUE;
	_bWaitForStartButtonToBeUp = TRUE;
	_nActivePlayer = GAME_CAM_PLAYER_1;
	for( i=0; i < _TOTAL_NUM_CAMERAS; i++ ) {
		_anCurrentCameraType[i] = GAME_CAM_TYPE_NOT_IN_USE;
		fcamera_SetCameraManByIndex( i, NULL );
		fcamera_GetCameraByIndex( i )->Reset( i );
	}
	for( i=0; i < _NUM_BOT_CAMERAMEN; i++ ) {
		_paBotCameramen[i].Reset();	
	}
	for( i=0; i < _NUM_MANUAL_CAMERAMEN; i++ ) {
		_paManualCameramen[i].Reset();
	}
	for( i=0; i < _NUM_SIMPLE_CAMERAMEN; i++ ) {
		_paSimpleCameramen[i].Reset();
	}
	for( i=0; i < _NUM_CUTSCENE_CAMERAMEN; i++ ) {
		_paCutsceneCameramen[i].Reset();
	}
#if CAM_DEBUG_ENABLE
	_pCamManBeingDebugged = NULL;
#endif

	for( i=0; i<GAME_CAM_MAX_COUNT; ++i ) {
		_apCameraMen[i] = NULL;
	}

	// clear any placed scenes that reside in the level
}

void SetPlayerToAllowDebugging( s32 nAllowDebuggingByPlayer/*=GAMECAM_DONT_MONITOR_CONTROLS*/,
							   u32 nControllerPortOfPlayer/*=0*/ ) {

	FASSERT( _bInitOK );

	FMATH_CLAMP( nAllowDebuggingByPlayer, GAMECAM_DONT_MONITOR_CONTROLS, GAME_CAM_MAX_COUNT );
	FMATH_CLAMP( nControllerPortOfPlayer, 0, MAX_PLAYERS );

	_nPlayerToMonitorForDebugging = nAllowDebuggingByPlayer;
	_nControllerPortToMonitor = nControllerPortOfPlayer;
	
	// if we were in debug mode, restore the regular camera.
	_GetOutOfDebugCam();	
}

// Call during a level to assign a certain bot to a player.  Different cameramen
// handle different camera types, thus the different functions.
void gamecam_SwitchPlayerTo3rdPersonCamera( GameCamPlayer_e nPlayer, CBot *pBot ) {

	FASSERT( _bInitOK );
	FASSERT( pBot );
	FASSERT( nPlayer < _NUM_BOT_CAMERAMEN );

	_GetOutOfDebugCam();
	
	fcamera_SetCameraManByIndex( nPlayer, &_paBotCameramen[nPlayer] );
	_paBotCameramen[nPlayer].Init( pBot, &_paCamBotInfos[ pBot->m_pBotDef->m_nClass ] );

	_apCameraMen[nPlayer] = &_paBotCameramen[nPlayer];
}

void gamecam_SwitchPlayerTo1stPersonCamera( GameCamPlayer_e nPlayer, CBot *pBot ) {

	FASSERT( _bInitOK );
	FASSERT( pBot );
	FASSERT( nPlayer < _NUM_BOT_CAMERAMEN );

	_GetOutOfDebugCam();
	
	fcamera_SetCameraManByIndex( nPlayer, &_paBotCameramen[nPlayer] );
	_paBotCameramen[nPlayer].Init( pBot, &_paCamBotInfos[ pBot->m_pBotDef->m_nClass ] );
	_paBotCameramen[nPlayer].ChangeCameraMode( CCamBot::CAM_BOT_3RD_PERSON );

	_apCameraMen[nPlayer] = &_paBotCameramen[nPlayer];
}

void gamecam_SwitchPlayerToManualCamera( GameCamPlayer_e nPlayer, CCamManualInfo *pCamInfo ) {
	
	FASSERT( _bInitOK );
	FASSERT( pCamInfo );
	FASSERT( nPlayer < _NUM_MANUAL_CAMERAMEN );
	FASSERT( nPlayer >= GAME_CAM_PLAYER_1);

	_GetOutOfDebugCam();
	
	fcamera_SetCameraManByIndex( nPlayer, &_paManualCameramen[nPlayer] );
	_paManualCameramen[nPlayer].Init( pCamInfo );

	_apCameraMen[nPlayer] = &_paManualCameramen[nPlayer];
}

void gamecam_SwitchPlayerToSimpleCamera( GameCamPlayer_e nPlayer, CCamSimpleInfo *pCamInfo ) {

	FASSERT( _bInitOK );
	FASSERT( pCamInfo );
	FASSERT( nPlayer < _NUM_SIMPLE_CAMERAMEN );

	_GetOutOfDebugCam();
	
	fcamera_SetCameraManByIndex( nPlayer, &_paSimpleCameramen[nPlayer] );
	_paSimpleCameramen[nPlayer].Init( pCamInfo );

	_apCameraMen[nPlayer] = &_paSimpleCameramen[nPlayer];
}


void gamecam_SwitchPlayerToCutsceneCamera( GameCamPlayer_e nPlayer, CCamCutsceneInfo *pCamInfo ) {
	
	FASSERT( _bInitOK );
	FASSERT( pCamInfo );

	//WE SHOULD NEVER CALL THIS FUNCTION IN ANY OTHER MODE OTHER
	//THAN SINGLE PLAYER!
	FASSERT( nPlayer == GAME_CAM_PLAYER_1);

	_GetOutOfDebugCam();
	
	fcamera_SetCameraManByIndex( nPlayer, &_paCutsceneCameramen[0] );
	_paCutsceneCameramen[0].Init( pCamInfo );

	_apCameraMen[nPlayer] = &_paCutsceneCameramen[0];
}


// Call to make a certain player ACTIVE.  This will change what camera
// gamecam_GetActiveCamera() returns.
BOOL gamecam_SetActiveCamera( GameCamPlayer_e nPlayer ) {
	
	FASSERT( _bInitOK );
	FASSERT( nPlayer < GAME_CAM_MAX_COUNT );

	if( fcamera_GetCameraManByIndex( nPlayer ) ) {
		// nPlayer is current being driven by a cameraman
		_nActivePlayer = nPlayer;
	} else {
		return FALSE;
	}

	return TRUE;
}

CFCamera *gamecam_GetActiveCamera( void ) {

	FASSERT( _bInitOK );

	return fcamera_GetCameraByIndex( _nActivePlayer );
}

CFCameraMan *gamecam_GetCameraManByIndex( GameCamPlayer_e nPlayer, GameCamType_e *pnCamTypeOut/*=NULL*/ ) {
	FASSERT( _bInitOK );
	FASSERT( nPlayer < GAME_CAM_MAX_COUNT );

	if( pnCamTypeOut ) {
		if( _apCameraMen[nPlayer] == &_paBotCameramen[nPlayer] ) {
			if( _paBotCameramen[nPlayer].GetCamBotMode() == CCamBot::CAM_BOT_3RD_PERSON ) {
				*pnCamTypeOut = GAME_CAM_TYPE_ROBOT_3RD;
			} else if( _paBotCameramen[nPlayer].GetCamBotMode() == CCamBot::CAM_BOT_1ST_PERSON ) {
				*pnCamTypeOut = GAME_CAM_TYPE_ROBOT_1ST;
			} else {
				FASSERT_NOW;
			}
		} else if( _apCameraMen[nPlayer] == &_paManualCameramen[nPlayer] ) {
			*pnCamTypeOut = GAME_CAM_TYPE_MANUAL;
		} else if( _apCameraMen[nPlayer] == &_paSimpleCameramen[nPlayer] ) {
			*pnCamTypeOut = GAME_CAM_TYPE_SIMPLE;
		} else if( _apCameraMen[nPlayer] == &_paCutsceneCameramen[0] ) {
			*pnCamTypeOut = GAME_CAM_TYPE_CUTSCENE;
		} else {
			FASSERT_NOW;
		}
	}

	return _apCameraMen[nPlayer];
}

// Call to find out if system is in debug mode.
BOOL gamecam_IsInDebugMode( void ) {

	FASSERT( _bInitOK );

	return (_nDebugMode != GAMECAM_DEBUG_MODES_NOT_DEBUGGING);
}

// Returns the camera that is being debugged right now, NULL if not in debug mode.
CFCamera *gamecam_GetCameraBeingDebugged( GameCam_DebugModes_e *pnMode/*=NULL*/ ) {

	FASSERT( _bInitOK );
#if CAM_DEBUG_ENABLE
	if( _nDebugMode != GAMECAM_DEBUG_MODES_NOT_DEBUGGING ) {
		if( pnMode ) {
			*pnMode = _nDebugMode;
		}
		return _paDebugCameramen->GetCameraBeingDebugged();
	}
#endif
	return NULL;
}


static BOOL _CheckForEnterDebugMode() {
#if CAM_DEBUG_ENABLE
	if( _nPlayerToMonitorForDebugging == GAMECAM_DONT_MONITOR_CONTROLS ) {
		return FALSE;
	}

	CFCamera *pActivePlayerCam = fcamera_GetCameraByIndex( _nPlayerToMonitorForDebugging );
	CFCameraMan *pActivePlayerCamMan = fcamera_GetCameraManByIndex( _nPlayerToMonitorForDebugging );

	FASSERT( _bInitOK );
	FASSERT( _nDebugMode == GAMECAM_DEBUG_MODES_NOT_DEBUGGING );

	///////////////////////////////////////////
	// check the keys that we are interested in
	BOOL bEnterDebugKeysDown = FALSE;
	BOOL bToggleModesKeyDown = FALSE;
	BOOL bFire1Button = ( Gamepad_aapSample[_nControllerPortToMonitor][GAMEPAD_MAIN_FIRE_PRIMARY]->uLatches & FPAD_LATCH_ON );
	BOOL bFire2Button = ( Gamepad_aapSample[_nControllerPortToMonitor][GAMEPAD_MAIN_FIRE_SECONDARY]->uLatches & FPAD_LATCH_ON );
	BOOL bSelectFire2Button = ( Gamepad_aapSample[_nControllerPortToMonitor][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & FPAD_LATCH_ON );

	// check the debug mode button combination
	if( _bWaitForDebugButtonsToBeUp ) {
		if( !bFire1Button && !bFire2Button && !bSelectFire2Button ) {
			_bWaitForDebugButtonsToBeUp = FALSE;
		}
	} else if( bFire1Button && bFire2Button && bSelectFire2Button ) {
		bEnterDebugKeysDown = TRUE;
		_bWaitForDebugButtonsToBeUp = TRUE;
	}
	// check the start button button
	if( !_bWaitForDebugButtonsToBeUp && !bEnterDebugKeysDown ) {
		if( _bWaitForStartButtonToBeUp ) {
			if( !bSelectFire2Button ) {
				_bWaitForStartButtonToBeUp = FALSE;
			}
		} else if( bSelectFire2Button ) {
			bToggleModesKeyDown = TRUE;
			_bWaitForStartButtonToBeUp = TRUE;
		}
	}

	//////////////////////////////////////
	// see if we should go into debug mode
	if( bEnterDebugKeysDown && pActivePlayerCam ) {
		// switch into debug mode...
		_nDebugMode = GAMECAM_DEBUG_MODES_FREE_CAM;
		DEVPRINTF( "*** Entering Debug Cam Mode ***\n" );

		_pCamManBeingDebugged = pActivePlayerCamMan;

		fcamera_SetCameraManByIndex( _DEBUG_CAMERA_INDEX, _paDebugCameramen );
		_paDebugCameramen->Init( pActivePlayerCam, _nControllerPortToMonitor );

		// Show all the player cameras
		_ShowPlayerCameras( TRUE );

		if( _pCamManBeingDebugged ) {
			// lock the cameraman that is being debugged
			_pCamManBeingDebugged->FreezeShot( TRUE );
		}
		_DoNotUpdateBot( TRUE );
		return TRUE;
	}
#endif
	return FALSE;
}


// Work() should be called once per frame, but should be called after the bots have been 
// put into their final location this frame.  Once the Work() call completes, all cameras 
// will be in their final location for this frame.
void gamecam_Work() {
	
	FASSERT( _bInitOK );

#if CAM_DEBUG_ENABLE
	if( _nPlayerToMonitorForDebugging != GAMECAM_DONT_MONITOR_CONTROLS ) {
		// debug camera is allowed
		CFCamera *pActivePlayerCam = fcamera_GetCameraByIndex( _nPlayerToMonitorForDebugging );
		CFCameraMan *pActivePlayerCamMan = fcamera_GetCameraManByIndex( _nPlayerToMonitorForDebugging );
		BOOL bDoDebugCamWork = TRUE;

		if( _nDebugMode == GAMECAM_DEBUG_MODES_NOT_DEBUGGING ) {
			if( _CheckForEnterDebugMode() ) {
				bDoDebugCamWork = FALSE;
			}
		}

		// Only do all this stuff if we're in debug mode.
		if( bDoDebugCamWork && 
			(_nDebugMode != GAMECAM_DEBUG_MODES_NOT_DEBUGGING) ) {
			///////////////////////////////////////////
			// check the keys that we are interested in
			BOOL bEnterDebugKeysDown = FALSE;
			BOOL bToggleModesKeyDown = FALSE;
			BOOL bFire1Button = ( Gamepad_aapSample[_nControllerPortToMonitor][GAMEPAD_MAIN_FIRE_PRIMARY]->uLatches & FPAD_LATCH_ON );
			BOOL bFire2Button = ( Gamepad_aapSample[_nControllerPortToMonitor][GAMEPAD_MAIN_FIRE_SECONDARY]->uLatches & FPAD_LATCH_ON );
			BOOL bSelectFire2Button = ( Gamepad_aapSample[_nControllerPortToMonitor][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches & FPAD_LATCH_ON );

			// check the debug mode button combination
			if( _bWaitForDebugButtonsToBeUp ) {
				if( !bFire1Button && !bFire2Button && !bSelectFire2Button ) {
					_bWaitForDebugButtonsToBeUp = FALSE;
				}
			} else if( bFire1Button && bFire2Button && bSelectFire2Button ) {
				bEnterDebugKeysDown = TRUE;
				_bWaitForDebugButtonsToBeUp = TRUE;
			}
			// check the start button button
			if( !_bWaitForDebugButtonsToBeUp && !bEnterDebugKeysDown ) {
				if( _bWaitForStartButtonToBeUp ) {
					if( !bSelectFire2Button ) {
						_bWaitForStartButtonToBeUp = FALSE;
					}
				} else if( bSelectFire2Button ) {
					bToggleModesKeyDown = TRUE;
					_bWaitForStartButtonToBeUp = TRUE;
				}
			}

			// see if we should leave debug mode
			if( bEnterDebugKeysDown ) {
				// switch out of debug mode...
				_GetOutOfDebugCam();
				
			} else {
				// see if we should switch debug modes...
				if( bToggleModesKeyDown ) {
					// switch debug modes
					switch( _nDebugMode ) {

					default:
					case GAMECAM_DEBUG_MODES_FREE_CAM:
						// move to GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM mode
						_nDebugMode = GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM;
						// draw the bot's camera icon and allow the bot to move around
						_ShowPlayerCameras(TRUE);
						_DoNotUpdateBot( FALSE );
						if( _pCamManBeingDebugged ) {
							// unlock the cameraman that is being debugged
							_pCamManBeingDebugged->FreezeShot( FALSE );
						}
						// lock the debug camera and don't draw it's icon
						_paDebugCameramen->FreezeShot( TRUE );
						fcamera_ToggleCameraIconDraw( _paDebugCameramen->GetAssignedCamera()->GetID(), FALSE );
						break;

					case GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM:
						// move to GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM2 mode
						_nDebugMode = GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM2;
						// don't draw the bot's camera icon and allow the bot to move around
						_ShowPlayerCameras(FALSE);
						_DoNotUpdateBot( FALSE );
						if( _pCamManBeingDebugged ) {
							// unlock the cameraman that is being debugged
							_pCamManBeingDebugged->FreezeShot( FALSE );
						}
						// lock the debug camera and draw it's icon
						_paDebugCameramen->FreezeShot( TRUE );
						fcamera_ToggleCameraIconDraw( _paDebugCameramen->GetAssignedCamera()->GetID(), TRUE );		
						break;

					case GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM2:
						// move to GAMECAM_DEBUG_MODES_LOCKED_MOVING_LOOKAT_CAM mode
						_nDebugMode = GAMECAM_DEBUG_MODES_LOCKED_MOVING_LOOKAT_CAM;
						// don't draw the bot's camera icon and allow the bot to move around
						_ShowPlayerCameras(TRUE);
						_DoNotUpdateBot( FALSE );
						if( _pCamManBeingDebugged ) {
							// unlock the cameraman that is being debugged
							_pCamManBeingDebugged->FreezeShot( FALSE );
						}
						// don't lock the debug camera and don't draw it's icon
						_paDebugCameramen->FreezeShot( FALSE );
						fcamera_ToggleCameraIconDraw( _paDebugCameramen->GetAssignedCamera()->GetID(), FALSE );
						_paDebugCameramen->ForceModeChange( CCamDebug::CAM_DEBUG_MATCHYAW );
						_paDebugCameramen->EnableModeChanges( FALSE );
						_paDebugCameramen->EnableUserMovement( FALSE );
						break;

					case GAMECAM_DEBUG_MODES_LOCKED_MOVING_LOOKAT_CAM:
						// move to GAMECAM_DEBUG_MODES_FREE_CAM mode
						_nDebugMode = GAMECAM_DEBUG_MODES_FREE_CAM;
						// draw the bot's camera icon and don't allow it to move
						_ShowPlayerCameras(TRUE);
						_DoNotUpdateBot( TRUE );
						if( _pCamManBeingDebugged ) {
							// lock the cameraman that is being debugged
							_pCamManBeingDebugged->FreezeShot( TRUE );
						}
						// don't lock the debug camera and don't draw an icon
						_paDebugCameramen->FreezeShot( FALSE );
						fcamera_ToggleCameraIconDraw( _paDebugCameramen->GetAssignedCamera()->GetID(), FALSE );
						_paDebugCameramen->EnableModeChanges( TRUE );
						_paDebugCameramen->EnableUserMovement( TRUE );
						_paDebugCameramen->ForceModeChange( CCamDebug::CAM_DEBUG_FREE );
						break;
					}
				}
			}
		}
	}
#endif
	// must do the camera work every frame
	fcamera_Work();

#if FANG_WINGC
	{
		// Port diagnostic: active camera type and world-space position.
		static u32 _nFrame = 0;
		if( (_nFrame++ % 240) == 60 && Fang_bPortDiag ) {
			CFCamera *pCam = fcamera_GetCameraByIndex( _nActivePlayer );
			const CFXfm *pXfm = pCam ? pCam->GetFinalXfm() : NULL;
			DEVPRINTF( "PORT-CAMWORK player=%d type=%d debugmode=%d monitor=%d pos=(%.1f,%.1f,%.1f) fwd=(%.2f,%.2f,%.2f)\n",
				(int)_nActivePlayer, (int)_anCurrentCameraType[_nActivePlayer], (int)_nDebugMode, (int)_nPlayerToMonitorForDebugging,
				pXfm ? pXfm->m_MtxF.m_vPos.x : 0.0f, pXfm ? pXfm->m_MtxF.m_vPos.y : 0.0f, pXfm ? pXfm->m_MtxF.m_vPos.z : 0.0f,
				pXfm ? pXfm->m_MtxF.m_vFront.x : 0.0f, pXfm ? pXfm->m_MtxF.m_vFront.y : 0.0f, pXfm ? pXfm->m_MtxF.m_vFront.z : 0.0f );
		}
	}
#endif
}

// This function will set the active viewport and initialize the camera stack with the 
// settings contained in the ACTIVE camera (unless in debug mode, then the debug mode
// camera will be used instead).  This call should occur before any 3D rendering
// takes place, but anytime after the current frame's Work() call.
BOOL gamecam_SetViewportAndInitCamStack() {

	FASSERT( _bInitOK );
	
	CFCamera *pCurrentCamera;
	GameCam_DebugModes_e nDebugMode = ( _nActivePlayer == _nPlayerToMonitorForDebugging ) ? _nDebugMode : GAMECAM_DEBUG_MODES_NOT_DEBUGGING;

	switch( nDebugMode ) {
	default:
	case GAMECAM_DEBUG_MODES_NOT_DEBUGGING:
	case GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM2:
		pCurrentCamera = fcamera_GetCameraByIndex( _nActivePlayer );		
		break;
#if CAM_DEBUG_ENABLE
	case GAMECAM_DEBUG_MODES_FREE_CAM:
	case GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM:
	case GAMECAM_DEBUG_MODES_LOCKED_MOVING_LOOKAT_CAM:
		pCurrentCamera = fcamera_GetCameraByIndex( _DEBUG_CAMERA_INDEX );
		break;
#endif
	}
	
	if( !pCurrentCamera ) {
		return FALSE;
	}
	pCurrentCamera->SetupCameraAndViewport();

	return TRUE;
}

// Draw() should be called as one of the last 3D draw functions in the gameloop.  Most of
// the time Draw() will simply return, but sometimes (in debug mode) certain information be
// drawn into the scene.
void gamecam_Draw() {

	FASSERT( _bInitOK );

	if( _nActivePlayer != _nPlayerToMonitorForDebugging ) {
		// the active player is not the player that could be in debug mode
		return;
	}
#if CAM_DEBUG_ENABLE
	switch( _nDebugMode ) {
	default:
	case GAMECAM_DEBUG_MODES_NOT_DEBUGGING:
		break;

	case GAMECAM_DEBUG_MODES_FREE_CAM:
		if( _pCamManBeingDebugged ) {
			_pCamManBeingDebugged->Draw();
		}
		fcamera_Draw();

		switch( _paDebugCameramen->GetCurrentMode() )
		case CCamDebug::CAM_DEBUG_FREE: {
			ftext_DebugPrintString( 0.6f, 0.06f, "~w1~s0.73Debug Cam1: Free" );	
			break;

		case CCamDebug::CAM_DEBUG_LOOKAT:
			ftext_DebugPrintString( 0.6f, 0.06f, "~w1~s0.73Debug Cam1: Lookat" );	
			CFVec3A *pvecCamPos = &FXfm_pView->m_MtxR.m_vPos;
			ftext_DebugPrintf( 0.55f, 0.08f, "~w1~s1.00<%5.3f, %5.3f, %5.3f>", pvecCamPos->x, pvecCamPos->y, pvecCamPos->z );
			break;
		}		
		break;

	case GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM:
		if( _pCamManBeingDebugged ) {
			_pCamManBeingDebugged->Draw();
		}
		fcamera_Draw();
		ftext_DebugPrintString( 0.6f, 0.06f, "~w1~s0.73Debug Cam2: Security" );	
		break;

	case GAMECAM_DEBUG_MODES_LOCKED_MOVING_LOOKAT_CAM:
		fcamera_Draw();
		ftext_DebugPrintString( 0.6f, 0.06f, "~w1~s0.73Debug Cam4: Attached Lookat" );			
		break;

	case GAMECAM_DEBUG_MODES_LOCKED_DEBUG_CAM2:
		fcamera_Draw();
		ftext_DebugPrintString( 0.6f, 0.06f, "~w1~s0.73Debug Cam3: Regular" );
		break;
	}	
	if( _nDebugMode != GAMECAM_DEBUG_MODES_NOT_DEBUGGING ) {
		CFVec3A *pvecCamPos = &FXfm_pView->m_MtxR.m_vPos;
		ftext_DebugPrintf( 0.55f, 0.08f, "~w1~s1.00<%5.3f, %5.3f, %5.3f>", pvecCamPos->x, pvecCamPos->y, pvecCamPos->z );
	}
#endif
}


void gamecam_DoCamBotTransition( const gamecam_CamBotTransInfo_t *pTransCamInfo, CCamManualInfo *pCamManInfo, gamecam_CamBotTransResult_t *pResult,
								 gamecam_CamBotTransResult_t *pBot1Result, gamecam_CamBotTransResult_t *pBot2Result ) {
	FASSERT( pTransCamInfo->uPlayer < MAX_PLAYERS );
	FASSERT_UNIT_FLOAT( pTransCamInfo->fUnitInterp );
	FASSERT( pTransCamInfo->pBot1 != NULL );
	FASSERT( pTransCamInfo->pBot2 != NULL );
	FASSERT( pResult != NULL );
	FASSERT( pBot1Result != NULL );
	FASSERT( pBot2Result != NULL );

	CFCamera *pCamera;
	
	// get data for Bot 1
	gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)pTransCamInfo->uPlayer, pTransCamInfo->pBot1 );
	fcamera_Work();
	pCamera = fcamera_GetCameraByIndex( pTransCamInfo->uPlayer );
	pBot1Result->mtxCamera = pCamera->GetFinalXfm()->m_MtxR;
	pBot1Result->qCamera.BuildQuat( pBot1Result->mtxCamera );
	pCamera->GetFOV( &pBot1Result->fFOV );
	pBot1Result->fFOV = splitscreen_UnmapFOV( pTransCamInfo->uPlayer, pBot1Result->fFOV );
	
	// get data for Bot 2
	gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)pTransCamInfo->uPlayer, pTransCamInfo->pBot2 );
	fcamera_Work();
	pCamera = fcamera_GetCameraByIndex( pTransCamInfo->uPlayer );
	pBot2Result->mtxCamera = pCamera->GetFinalXfm()->m_MtxR;
	pBot2Result->qCamera.BuildQuat( pBot2Result->mtxCamera );
	pCamera->GetFOV( &pBot2Result->fFOV );
	pBot2Result->fFOV = splitscreen_UnmapFOV( pTransCamInfo->uPlayer, pBot2Result->fFOV );

	// perform the interpolation
	pResult->fFOV = FMATH_FPOT( pTransCamInfo->fUnitInterp, pBot1Result->fFOV, pBot2Result->fFOV );
	pResult->qCamera.ReceiveSlerpOf( pTransCamInfo->fUnitInterp, pBot1Result->qCamera, pBot2Result->qCamera );
	pResult->qCamera.BuildMtx( pResult->mtxCamera );
	pResult->mtxCamera.m_vPos.x = FMATH_FPOT( pTransCamInfo->fUnitInterp, pBot1Result->mtxCamera.m_vPos.x, pBot2Result->mtxCamera.m_vPos.x );
	pResult->mtxCamera.m_vPos.y = FMATH_FPOT( pTransCamInfo->fUnitInterp, pBot1Result->mtxCamera.m_vPos.y, pBot2Result->mtxCamera.m_vPos.y );
	pResult->mtxCamera.m_vPos.z = FMATH_FPOT( pTransCamInfo->fUnitInterp, pBot1Result->mtxCamera.m_vPos.z, pBot2Result->mtxCamera.m_vPos.z );

	// set up man cam
	pCamManInfo->m_fHalfFOV		= pResult->fFOV;
	if( pCamManInfo->m_pmtxMtx ) {
		*pCamManInfo->m_pmtxMtx	= pResult->mtxCamera;
	} else {
		FASSERT( pCamManInfo->m_pqQuat );
		FASSERT( pCamManInfo->m_pvecPos );

		*pCamManInfo->m_pqQuat	= pResult->qCamera;
		*pCamManInfo->m_pvecPos	= pResult->mtxCamera.m_vPos;
	}

	gamecam_SwitchPlayerToManualCamera( (GameCamPlayer_e)pTransCamInfo->uPlayer, pCamManInfo );	
}


//==================
// private functions

static void _ShowPlayerCameras( BOOL bShow ) {
	// in debug camera we want to display everyone's camera icon
	for( int i = 0; i < CPlayer::m_nPlayerCount; i++ ) {
		fcamera_ToggleCameraIconDraw( i, bShow );
	}
}

static void _DoNotUpdateBot( BOOL bDoNotUpdateBot ) {
	// in debug camera we only want to disable the player bot begin debugged
	if( _nPlayerToMonitorForDebugging == GAMECAM_DONT_MONITOR_CONTROLS ) {
		return;
	}
	if( Player_aPlayer[_nPlayerToMonitorForDebugging].m_pEntityCurrent->IsCreated() ) {
		Player_aPlayer[_nPlayerToMonitorForDebugging].m_pEntityCurrent->EnableWork( !bDoNotUpdateBot );
	}
}

static void _GetOutOfDebugCam( void ) {

	if( _nDebugMode == GAMECAM_DEBUG_MODES_NOT_DEBUGGING ) {
		return;
	}

	_nDebugMode = GAMECAM_DEBUG_MODES_NOT_DEBUGGING;
	DEVPRINTF( "*** Exiting Debug Cam Mode ***\n" );
#if CAM_DEBUG_ENABLE
	_ShowPlayerCameras(FALSE);
    fcamera_ToggleCameraIconDraw( _paDebugCameramen->GetAssignedCamera()->GetID(), FALSE );
	fcamera_SetCameraManByIndex( _DEBUG_CAMERA_INDEX, NULL );
	_paDebugCameramen->Reset();	
	if( _pCamManBeingDebugged ) {
		// unlock the cameraman that is being debugged
		_pCamManBeingDebugged->FreezeShot( FALSE );
		_pCamManBeingDebugged = NULL;
	}
	_DoNotUpdateBot( FALSE );

	_bWaitForDebugButtonsToBeUp = TRUE;
	_bWaitForStartButtonToBeUp = TRUE;
#endif
}



